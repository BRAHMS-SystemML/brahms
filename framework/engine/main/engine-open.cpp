/*
________________________________________________________________

	This file is part of BRAHMS
	Copyright (C) 2007 Ben Mitchinson
	URL: http://brahms.sourceforge.net

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
________________________________________________________________

	Subversion Repository Information (automatically updated on commit)

	$Id:: engine-open.cpp 2382 2009-11-16 18:12:18Z benjmitch  $
	$Rev:: 2382                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-16 18:12:18 +0000 (Mon, 16 Nov 2009)       $
________________________________________________________________

*/

#include "main/engine.h"
using brahms::Engine;
#include "base/output.h"
using namespace brahms::output;
#include "base/text.h"
using brahms::text::n2s;
using brahms::text::s2n;
using brahms::text::S2N_FAILED;
using brahms::text::sampleRateToString;
#include "base/brahms_math.h"
using brahms::math::unitIndex;

void Engine::open()
{
	FOUT_SECTION("Engine::open()")

	monitorSendPhase(EP_OPEN, "Open System");



////////////////	PREAMBLE

	{ FOUT_SECTION("Preamble")

		//	CPU time
		irtCPU.prev = brahms::thread::getthreadtimes();

		//	set priority
		INT32 pri = engineData.environment.geti("Priority", -3, 3);
		brahms::os::setprocesspriority(pri);
		fout << "priority set to " << pri << D_VERB;

		//	since we disabled RDTSC (multi-core problems!), we're now using QPC() to do timing (utils) on windows
		//	and this is rather slow, so we warn if we're timing run phase
		if (engineData.environment.getb("TimeRunPhase"))
			fout << "\"TimeRunPhase == true\" may slow down your fine-grained executions" << D_WARN;

	}



////////////////	LOAD SYSTEM

	{ FOUT_SECTION("Load System")

		//	monitor event
		monitorSendOperation("Load System", true);

		//	mark time
		t_openDocument = brahms::os::getuctime();

		//	get path to system file from execution
		string path = engineData.execution.fileSysIn;

		//	have system object parse the file into an instantiated system
		system.load(path, engineData.execution, engineData.loader);

	}



////////////////	INITIAL THREAD ASSIGNMENT

	UINT32 processCount;
	vector<INT32> threadIndex;
	UINT32 numberOfThreadsUsed;

	{ FOUT_SECTION("Thread Assignment")

		//	monitor event
		monitorSendOperation("Thread Assignment", true);

		//	report
		fout << "Initial assignment..." << D_VERB;

		//	number of threads we've used so far
		numberOfThreadsUsed = 0;
		processCount = system.getProcessCount();

		//	max count
		UINT32 maxThreadCount = 0;

		//	thread assignment
		threadIndex.resize(processCount);

		//	no assignment necessary if there are no processes to assign
		if (processCount)
		{
			//	create as many as one thread per process, but the total thread count may be capped
			string mtc = engineData.environment.gets("MaxThreadCount");
			if (!mtc.length())
			{
				fout << "MaxThreadCount == <empty string> (unlimited)" << D_VERB;
			}
			else if (mtc.substr(0,1) == "x" || mtc.substr(0,1) == "X")
			{
				DOUBLE mult = s2n(mtc.substr(1));
				if (mult == S2N_FAILED)
					ferr << E_EXECUTION_PARAMETERS << "malformed \"MaxThreadCount\"";
				UINT32 umult = mult;
				if (umult != mult)
					ferr << E_EXECUTION_PARAMETERS << "malformed \"MaxThreadCount\"";
				if (umult)
				{
					fout << "MaxThreadCount == " << mtc << D_VERB;
					UINT32 numProcessors = brahms::os::getnumprocessors();
					if (numProcessors)
					{
						fout << numProcessors << " processor(s) detected" << D_VERB;
					}
					else
					{
						numProcessors = 1; // unknown, assume 1
						fout << "unable to detect number of processors (1 assumed)" << D_VERB;
					}
					maxThreadCount = numProcessors * umult;
					fout << "MaxThreadCount == " << numProcessors << mtc << " == " << maxThreadCount << D_VERB;
				}
				else fout << "MaxThreadCount == x0 (unlimited)" << D_VERB;
			}
			else
			{
				maxThreadCount = engineData.environment.getu("MaxThreadCount");
				fout << "MaxThreadCount == " << maxThreadCount;
				if (maxThreadCount == 0) fout << " (unlimited)";
				fout << D_VERB;
			}

			//	nominal thread count
			if (maxThreadCount) numberOfThreadsUsed = min(maxThreadCount, processCount);
			else numberOfThreadsUsed = processCount;

			//	nominal assignment
			for (UINT32 p=0; p<processCount; p++)
			{
				threadIndex[p] = p % numberOfThreadsUsed;
				fout << system.getProcessByIndex(p)->getObjectName() << " ==> thread " << unitIndex(threadIndex[p]) << D_VERB;
			}
		}

		//	number of threads may change if we have any F_NO_CONCURRENCY processes
		fout << "ProcessCount == " << processCount << D_VERB;
		fout << "ThreadCount == " << numberOfThreadsUsed << " (nominal)" << D_VERB;

		/*

			Thread Reassignment

			Processes that set F_NO_CONCURRENCY are not thread-safe, so we should execute
			all of them in the same processing thread. We do this by just moving any that
			are not in the same thread as the first, following thread assignment, into
			the first's thread. This gives us thread-safety, but may be short of optimal. Also,
			we emit a warning. To hide this warning, the user must explicitly do safe
			thread assignment for these processes.

		*/

		//	report
		fout << "Reassignment..." << D_VERB;

		//	for each process...
		for (UINT32 p=0; p<processCount; p++)
		{
			//	get process and module info
			brahms::systemml::Process* process_p = system.getProcessByIndex(p);
			const ModuleInfo* info_p = process_p->module->getInfo();

			//	check for flag
			if (!(info_p->flags & F_NO_CONCURRENCY)) continue;

			//	get executable class / SystemML class
			const char* executable_p = process_p->getClassName().c_str();
			switch (info_p->binding)
			{
				case 1258:
					executable_p = "/binding/1258";
					break;
				case 1262:
					executable_p = "/binding/1262";
					break;
			}

			//	for each previously listed process
			for (UINT32 q=0; q<p; q++)
			{
				//	get some information
				brahms::systemml::Process* process_q = system.getProcessByIndex(q);
				const ModuleInfo* info_q = process_q->module->getInfo();

				//	get executable class / SystemML class
				const char* executable_q = process_q->getClassName().c_str();
				switch (info_q->binding)
				{
					case 1258:
						executable_q = "/binding/1258";
						break;
					case 1262:
						executable_q = "/binding/1262";
						break;
				}

				//	if the same class, enforce them being in separate threads
				if (string(executable_p) == executable_q)
				{
					//	check if they've been assigned to different threads
					if (threadIndex[p] != threadIndex[q])
					{
						//	move the process into the earlier listed thread
						threadIndex[p] = threadIndex[q];
						fout << process_p->getObjectName() << " ==> thread " << unitIndex(threadIndex[p]) << D_VERB;

						/*fout << "W_PROCESS_NOT_THREAD_SAFE: moving process \"" << process_p->getObjectName()
							<< "\" into same thread as previous instance \"" << process_q->getObjectName() << "\"" << D_WARN;
							*/

						//	no need to check any further, we're done with process p
						break;
					}
				}
			}
		}

/*
		//	report
		for (UINT32 t=0; t<numberOfThreadsUsed; t++)
		{
			stringstream ss;
			ss << "Processes computed by Thread " << unitIndex(t);

			{ FOUT_SECTION(ss.str().c_str())

				bool assigned = false;
				for (UINT32 p=0; p<system.getProcessCount(); p++)
				{
					if (threadIndex[p] == t)
					{
						fout << system.getProcessByIndex(p)->getObjectName() << D_VERB;
						assigned = true;
					}
				}
				if (!assigned) fout << "(no assignments)" << D_VERB;

			}
		}
*/

	}



////////////////	CREATE THREADS

	{ FOUT_SECTION("Create Threads")

		//	load processes into threads
		for (UINT32 proposedThreadIndex=0; proposedThreadIndex<numberOfThreadsUsed; proposedThreadIndex++)
		{
			//	list all processes assigned to that thread
			vector<UINT32> processesAssigned;
			for (UINT32 p=0; p<system.getProcessCount(); p++)
			{
				if (threadIndex[p] == ((INT32)proposedThreadIndex))
					processesAssigned.push_back(p);
			}

			//	if no processes, move on (THEREFORE! actual thread index may not equal proposed thread index)
			if (!processesAssigned.size()) continue;

			//	create thread wrapper and connect its output stream and cancel state and set its index
			brahms::thread::WorkerThread* thread = workers.create(&engineData.core.sink);

			//	load processes into thread
			for (UINT32 i=0; i<processesAssigned.size(); i++)
				thread->load(system.getProcessByIndex(processesAssigned[i]));

			//	wait for idle
			thread->waitForIdle();
		}
	}



////////////////	INSTANTIATE

	{ FOUT_SECTION("Instantiate Processes")

		//	monitor event
		monitorSendOperation("Instantiating Processes...", true);

		/*

			**** SERIAL THREADING ****

			Currently, we do many of these events serially, even though
			they're executed in the worker threads. This avoids any
			thread-safety issues. I put them in parallel to start with,
			and things starting going wrong intermittently. Time will tell
			what are the problems, and where we need mutexes.

			Try, for instance, while(true) { brahms nested-exe.xml } - if
			you put all of these calls in parallel, you should get errors.
			May have to execute it 1000s of times to see anything, though.

			One solution is to make the whole API thread-safe simply by
			protecting each function with the mutex available in the API
			cpp. However, this may have performance issues, and is
			probably overkill. Therefore, I suggest that we slowly deal with
			thread-safety separately on each function, until they're all
			thread safe. At that point, the above test should be safe enough.

			Note that it's not just the component-side API that needs to be
			thread-safe, but the internal API of the engine as well, those
			functions used by the framework thread code itself.

		*/

		//	verbose
		fout << "SUB-PHASE: EVENT_MODULE_CREATE" << D_VERB;

		//	EVENT_MODULE_CREATE
		for (UINT32 t=0; t<workers.getThreadCount(); t++)
			workers.getThread(t)->fireCommonEvent(EVENT_MODULE_CREATE);

		//	verbose
		fout << "SUB-PHASE: EVENT_STATE_SET" << D_VERB;

		//	EVENT_STATE_SET
		for (UINT32 t=0; t<workers.getThreadCount(); t++)
			workers.getThread(t)->fireCommonEvent(EVENT_STATE_SET);
	}


////////////////	PRECONNECT

	{ FOUT_SECTION("EVENT_INIT_PRECONNECT")

		//	monitor event
		monitorSendOperation("EVENT_INIT_PRECONNECT", true);

		//	EVENT_INIT_PRECONNECT
		for (UINT32 t=0; t<workers.getThreadCount(); t++)
			workers.getThread(t)->fireCommonEvent(EVENT_INIT_PRECONNECT);
	}



////////////////	CONNECT

	{ FOUT_SECTION("EVENT_INIT_CONNECT")

		//	monitor event
		monitorSendOperation("EVENT_INIT_CONNECT", true);

		/*

		Passes are performed by Voices in order: pass 0 is by Voice 0, pass 1 is
		by Voice 1, etc. When all Voices have performed a pass, Voice 0 performs
		its next, which is pass <VoiceCount>, and so on. This process continues
		until all Voices have finished connect phase, or until deadlock is
		reached. Deadlock is defined as a pass being performed by all Voices
		without any single Voice making any progress.

		*/

		//	initialise state
		brahms::systemml::PassResult prGlobal = { false, true };
		UINT32 pass = 0;
		UINT32 globalPass = 0;
		system.peerVoices.resize(engineData.core.getVoiceCount());

		//	verbose
		fout << "SUB-PHASE: EVENT_INIT_CONNECT" << D_VERB;

		//	passes
		while (true)
		{
			//	report
			if (!pass)
				fout << "GLOBAL PASS " << globalPass << D_VERB;

			//	local or remote pass begins
			if (pass == ((UINT32)engineData.core.getVoiceIndex()))
			{
				//	report
				fout << "LOCAL PASS " << pass << D_VERB;

				//	monitor event
				string msg = "Connecting System:\n    Local Pass " + n2s(unitIndex(globalPass));
				monitorSendOperation(msg, true);

				//	perform local pass
				brahms::systemml::PassResult prLocal = system.localPass(workers);

				//	update global pass state
				prGlobal.progress |= prLocal.progress;
				prGlobal.finished &= prLocal.finished;
			}

			else
			{

				//	report
				fout << "REMOTE PASS " << pass << D_VERB;

				//	monitor event
				string msg = "Connecting System:\n    Serving Voice " + n2s(unitIndex(pass));
				monitorSendOperation(msg, true);

				//	act as server until released by Voice performing local pass
				system.connectPhaseServe(pass, prGlobal.finished, prGlobal.progress);
			}

			//	increment
			pass++;

			//	end of global pass?
			if (pass == engineData.core.getVoiceCount())
			{
				//	report
				fout << "GLOBAL PASS ENDS" << D_VERB;

				//	finished?
				if (prGlobal.finished) break;

				//	deadlock?
				if (!prGlobal.progress)
				{
					//	have system report state
					system.listMissingInputs();

					//	throw
					ferr << E_DEADLOCK << "see log for details";
				}

				//	maintain state
				pass = 0;
				globalPass++;
				prGlobal.progress = false;
				prGlobal.finished = true;
			}
		}

	}



////////////////	NEGOTIATE BASE RATE

	{ FOUT_SECTION("Base Rate")

		//	monitor event
		monitorSendOperation("Negotiating Base Rate...", true);

		/*

			The base rate is calculated as the lowest common multiple of all requested sample rates,
			or, if that is not representable, E_UNREPRESENTABLE is thrown.

			In solo, this is all done locally. In concerto, all the non-master voices send a list of
			all of their requested sample rates to the master (zeroth) voice. The master then calculates
			the base rate and sends it back to all of the non-master voices. All voices then calculate
			the period in base samples for each component.

		*/

		//	---- COLLATE LIST OF LOCAL SAMPLE RATES ----

		//	get list of all local sample rates (process and data)
		vector<SampleRate> allSampleRates;
		system.getAllSampleRates(allSampleRates);

		//	---- COLLATE SAMPLE RATE LISTS FROM ALL VOICES IN MASTER ----

		//	if Concerto
		if (engineData.core.getVoiceCount() > 1)
		{
			//	if i am the master
			if (engineData.core.getVoiceIndex() == VOICE_MASTER)
			{
				//	receive from non-masters
				fout << "receive sample rates from non-master Voices" << D_VERB;
				for (VoiceIndex remoteVoiceIndex=0; remoteVoiceIndex<engineData.core.getVoiceCount(); remoteVoiceIndex++)
				{
					//	not from self
					if (remoteVoiceIndex == VOICE_MASTER) continue;

					//	get sample rates from other Voices
					brahms::base::IPM* ipmr;
					comms.pull(remoteVoiceIndex, ipmr, brahms::base::IPMTAG_UNDEFINED, fout, brahms::channel::COMMS_TIMEOUT_DEFAULT, true);
					if (ipmr->header().tag != brahms::base::IPMTAG_PUSHRATES)
						ferr << E_COMMS << "expected IPMTAG_PUSHRATES at this point";

					//	add new rates to list
					double cnt = ipmr->header().bytesAfterHeaderUncompressed / sizeof(SampleRate);
					if (cnt != floor(cnt))
						ferr << E_COMMS << "invalid IPMTAG_PUSHRATES msg";
					int N = (int) cnt;
					fout << "received " << N << " sample rates from Voice " << remoteVoiceIndex << D_VERB;
					SampleRate* psr = (SampleRate*)ipmr->body();
					for (int n=0; n<N; n++)
						allSampleRates.push_back(psr[n]);

					//	release
					ipmr->release();
				}

				//	fout
				fout << "i now have a list of " + n2s(allSampleRates.size()) + " sample rates" << D_VERB;
			}

			//	if i am not the master
			else
			{
				//	push rates to master
				fout << "send " << allSampleRates.size() << " sample rates to master Voice" << D_VERB;
				brahms::base::IPM* ipms = engineData.pool.get(brahms::base::IPMTAG_PUSHRATES, VOICE_MASTER);
				ipms->appendBytes(allSampleRates.size() ? (BYTE*)&allSampleRates[0] : NULL, allSampleRates.size() * sizeof(SampleRate));
				comms.push(ipms, fout);
			}
		}



		//	---- CALCULATE THE BASE RATE IN MASTER ----

		brahms::local::BaseRate baseRate;

		if (engineData.core.getVoiceIndex() == VOICE_MASTER)
		{
			//	create base rate object and use it to find base rate
			fout << "calculate base sample rate..." << D_VERB;
			baseRate.findBaseRate(allSampleRates);
			baseRate.setSystemTime(&system.systemTime, engineData.execution.executionStop);
		}

		//	---- BROADCAST BASE RATE TO NON-MASTERS ----

		//	if Concerto
		if (engineData.core.getVoiceCount() > 1)
		{
			//	if we are the master
			if (engineData.core.getVoiceIndex() == VOICE_MASTER)
			{
				fout << "broadcast base sample rate" << D_VERB;
				for (VoiceIndex remoteVoiceIndex=0; remoteVoiceIndex<engineData.core.getVoiceCount(); remoteVoiceIndex++)
				{
					//	not to self
					if (remoteVoiceIndex == VOICE_MASTER) continue;

					//	send base rate to other Voice
					brahms::base::IPM* ipms = engineData.pool.get(brahms::base::IPMTAG_PUSHBASERATE, remoteVoiceIndex);
					ipms->appendBytes((BYTE*)&baseRate.baseSampleRate, sizeof(SampleRate));
					comms.push(ipms, fout);
				}
			}

			//	if we are not the master
			else
			{
				fout << "retrieve base rate from master" << D_VERB;
				brahms::base::IPM* ipmr;
				comms.pull(VOICE_MASTER, ipmr, brahms::base::IPMTAG_UNDEFINED, fout, brahms::channel::COMMS_TIMEOUT_DEFAULT, true);
				if (ipmr->header().tag != brahms::base::IPMTAG_PUSHBASERATE)
					ferr << E_COMMS << "expected IPMTAG_PUSHBASERATE at this point";
				if (ipmr->header().bytesAfterHeaderUncompressed != sizeof(SampleRate))
					ferr << E_COMMS << "invalid IPMTAG_PUSHBASERATE msg";
				baseRate.baseSampleRate = *((SampleRate*)ipmr->body());
				baseRate.setSystemTime(&system.systemTime, engineData.execution.executionStop);

				//	release
				ipmr->release();
			}
		}

		//	---- LAY IN BASE RATE RELATED TIMING DATA FOR ALL COMPONENTS ----

		//	report
		fout << "base sample rate = (" << sampleRateToString(baseRate.baseSampleRate) << ")Hz" << D_VERB;

		//	ask system to finalize component time for all components (process and data)
		system.finalizeAllComponentTimes(baseRate.baseSampleRate, system.systemTime.executionStop);

	}



////////////////	ADVANCE ALL PROCESSES TO THE TIME SPECIFIED IN THE SYSTEM FILE (IF ANY)

	{ FOUT_SECTION("Progress System")

		/*

			Usually, the System File implicitly specifies t=0 (by not specifying a time
			at all). But, if the system has been previously progressed, it may have a
			non-zero time. In this case, each <Process> tag will have time information
			that we must use now to set the time of each Process.

			In addition, any Data states must be laid in to the appropriate data objects,
			and the read/write locks of the associated Manifolds must be set correctly so
			that inter-thread comms begins with the right relationships between readers
			and writers.

		*/

		//	we have to do this first, so that all readBuffer/writeBuffer indices
		//	are correctly set (amongst other things)...
		system.progress(fout);

		//	...then we can do this, because the initial state of each lock
		//	depends only on whether the reader or the writer will reach it first
		system.initInterThreadLocks(fout);

		//	and we need to do it for any concerto locks as well, since we
		//	now use this as our default method to prepare the locks, even though
		//	in concerto we don't support system progression
		for (UINT32 p=0; p<system.outputPortRemotes.size(); p++)
			system.outputPortRemotes[p]->initInterThreadLocks(fout);

		//	set all due datas to NULL, now - they will be set to non-NULL
		//	when ip/op's are locked as the first thing done in thread service
		system.setDueDatas(fout);

	}



////////////////	START LOGS

	{ FOUT_SECTION("Start Logs")

		/*

			This must happen before EVENT_INIT_POSTCONNECT, so that F_LISTENED
			is correctly set before that event.

		*/

		//	monitor event
		monitorSendOperation("Starting Logs...", true);

		system.startLogs(workers);

	}



////////////////	EVENT_INIT_POSTCONNECT

	{ FOUT_SECTION("EVENT_INIT_POSTCONNECT")

		//	monitor event
		monitorSendOperation("EVENT_INIT_POSTCONNECT", true);

		//	**** SERIAL THREADING **** (see note higher up)

		//	EVENT_STATE_SET
		for (UINT32 t=0; t<workers.getThreadCount(); t++)
			workers.getThread(t)->fireCommonEvent(EVENT_INIT_POSTCONNECT);

	}



////////////////	END INIT PHASE

	//	send EVENT_INIT_COMPLETE to all data objects
	system.endInitPhase(fout);

	/*

		It's important that all peer voices synchronize about here,
		so that they have all completed all the above initialisation
		before we start sending them PUSHDATA messages. Other
		protections could be afforded, but this basic (and penalty-
		free) protection is easily implemented by the exchange of
		sync messages.

	*/

	comms.synchronize(fout);

	/* Time Init Phase */

	//	wallclock time
	irtWallclock.init += engineData.systemTimer.elapsed();

	//	CPU time
	irtCPU.init = brahms::thread::getthreadtimes();



}
