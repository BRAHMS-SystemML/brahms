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

	$Id:: thread.cpp 2354 2009-11-10 22:58:16Z benjmitch       $
	$Rev:: 2354                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-10 22:58:16 +0000 (Tue, 10 Nov 2009)       $
________________________________________________________________

*/




#include "systemml.h"

using namespace brahms::output;
using namespace brahms::text;
using namespace brahms::math;



namespace brahms
{
	namespace thread
	{


	////////////////
	////////////////
	////////////////	WORKER THREAD: VARIABLE CONTEXT
	////////////////
	////////////////

		/*

			The functions in this special section may be called in
			the context of the worker thread itself, or in the context
			of the caller thread (main thread).

		*/

		void WorkerThread::worker_error(brahms::error::Error& e, brahms::output::Source* source)
		{
			//	this function has to be careful, since it can be
			//	called both by the worker thread (most occurrences)
			//	or by the caller thread (if the worker thread has
			//	been detected hung).

			//	use thread's output object, unless overridden (one call
			//	uses this, when the thread has hung - last occurrence
			//	of "worker_error" in this file)
			if (!source) source = &tout;

			//	add error flags to processes
			brahms::systemml::Process* p = processBeingFired; // thread-safety, take a local copy
			if (p) p->setFlag(F_LOCAL_ERROR);
			for (UINT32 p=0; p<processes.size(); p++)
				processes[p]->setFlag(F_GLOBAL_ERROR);

			//	store error (and set error flag) - the storeError() function is thread-safe
			storeError(e, *source);
		}

		void WorkerThread::signalActive()
		{
			//	increment monitor (this is so close to thread safe as to make no real odds)
			monitor.current++;
		}







	////////////////
	////////////////
	////////////////	WORKER THREAD: CALLER CONTEXT
	////////////////
	////////////////

		/*

			The functions in this section are called in
			the context of the calling thread (main thread) only.

		*/

		void ThreadProcWorker(void* thread); // defined later in this file

		#define WAIT_FOR_IDLE_WAITSTEP 10

		WorkerThread::WorkerThread(INT32 threadIndex, brahms::output::Sink* sink, UINT32 TimeoutThreadHang, UINT32 TimeoutThreadTerm, EngineData& p_engineData, RunPhaseTimer& p_runPhaseTimer)
			:
			Thread(TC_WORKER, threadIndex, p_engineData.core),
			runPhaseTimer(p_runPhaseTimer),
			signalIdle(WAIT_FOR_IDLE_WAITSTEP, NULL),
			signalRelease(brahms::os::SIGNAL_INFINITE_WAIT, NULL),
			engineData(p_engineData)
		{
			this->TimeoutThreadHang = TimeoutThreadHang;
			this->TimeoutThreadTerm = TimeoutThreadTerm;
			flag_runPhaseFinished = false;
			time_now = 0;
			time_stop = 0;
			processBeingFired = NULL;
			stop = false;
		}

		WorkerThread::~WorkerThread()
		{
		}

		void WorkerThread::load(brahms::systemml::Process* process)
		{
			//	give process a pointer to this WorkerThread
			process->thread = this;

			//	give process a pointer to the thread's output source object so it can send output
			process->tout = &tout;

			//	store process in thread's array
			processes.push_back(process);
		}

		void WorkerThread::fireCommonEvent(Symbol eventType)
		{
			//	set cross-thread data
			commonEventType = eventType;
			singleEventEvent.type = S_NULL;

			//	signal thread active, so it doesn't timeout immediately
			signalActive();

			//	signal RELEASE to thread
			signalRelease.set();

			//	wait for idle (will throw if local thread error)
			waitForIdle();

			//	clear cross-thread data
			commonEventType = S_NULL;
		}

		void WorkerThread::fireSingleEvent(const EventEx& event)
		{
			//	set cross-thread data
			commonEventType = S_NULL;
			singleEventEvent = event;

			//	signal thread active, so it doesn't timeout immediately
			signalActive();

			//	signal RELEASE to thread
			signalRelease.set();

			//	wait for idle (will throw if local thread error)
			waitForIdle();

			//	clear cross-thread data
			singleEventEvent.type = S_NULL;
		}

		void WorkerThread::doRunPhase(BaseSamples executionStop)
		{
			//	store stop time
			time_stop = executionStop;

			//	set cross-thread data
			commonEventType = EVENT_RUN_PLAY;

			//	signal thread active, so it doesn't timeout immediately
			signalActive();

			//	signal RELEASE to thread
			signalRelease.set();
		}

		bool WorkerThread::hasHung()
		{
			//	if already detected
			if (flagState(F_THREAD_HUNG)) return true;

			//	get time since thread started runnings
			DOUBLE tElapsed = threadTimer.elapsed();

			//	if signalActive() has been called since we last
			//	checked, set tLastSawActivity as current time
			if (monitor.current != monitor.last)
			{
				monitor.last = monitor.current;
				monitor.tLastSawActivity = tElapsed;
			}

			//	now calculate time since we last saw activity (in milliseconds)
			UINT32 tSinceLastActive = (tElapsed - monitor.tLastSawActivity) * 1000.0;

			//	check for hang condition
			if (tSinceLastActive >= TimeoutThreadHang)
			{
				setFlag(F_THREAD_HUNG);
				brahms::error::Error e(E_THREAD_HUNG, "thread W" + n2s(unitIndex(getThreadIndex())));
				e.trace("time since active " + n2s(tSinceLastActive) + "ms (limit " + n2s(TimeoutThreadHang) + "ms)");
				e.trace("at baseSamples = " + n2s(time_now));

				//	use system source (see notes on CONTEXT)
				worker_error(e, &engineData.core.caller.tout);
				return true;
			}

			//	ok
			return false;
		}

		void WorkerThread::waitForIdle()
		{
			//	loop
			while(true)
			{
				//	break loop if signal set or cancel set
				Symbol w = signalIdle.waitfor();

				//	if OK or CANCEL, just return (should never see CANCEL since we passed NULL to waitfor())
				if (w == C_OK || w == C_CANCEL) return;

				//	if error, throw
				if (flagState(F_THREAD_ERROR))
				{
					//	use system source (see notes on CONTEXT)
					engineData.core.caller.tout << "F_THREAD_ERROR detected in " << getThreadIdentifier() << D_VERB_ERROR;

					//	throw
					ferr << E_THREAD_ERROR << "in waitForIdle()";
				}

				//	check for hang
				if (hasHung())
				{
					//	use system source (see notes on CONTEXT)
					engineData.core.caller.tout << "F_THREAD_HUNG detected in " << getThreadIdentifier() << D_VERB_ERROR;

					//	throw
					ferr << E_THREAD_HUNG << "in waitForIdle()";
				}

				//	else, go round again
			}
		}

		void WorkerThread::terminate(brahms::output::Source& fout)
		{
			//	set stop so the thread knows to terminate
			stop = true;

			//	but also set signal so that thread comes out of wait state asap
			signalRelease.set();

			//	then call the base class terminate routine
			Thread::terminate(fout);
		}








	////////////////
	////////////////
	////////////////	WORKER THREAD: SELF CONTEXT
	////////////////
	////////////////

		/*

			The functions in this section are called in
			the context of the worker thread itself.

		*/

		void ThreadProcWorker(void* thread)
		{
			((WorkerThread*)thread)->threadProc();
		}

		void WorkerThread::threadProc()
		{
			//	a thread proc may not except
			try
			{
				//	main loop (instructions from caller thread move us around in this loop)
				while(true)
				{

		////////////////	SYNCHRONIZE

					//	report
					tout << "thread " << getThreadIdentifier() << " IDLE..." << D_FULL;

					//	signal IDLE
					signalIdle.set();

					//	wait for RELEASE
					/*Symbol release = */signalRelease.waitfor();

					//	check for stop (terminate() was called)
					if (stop) break;

					//	otherwise, just continue

		////////////////	SYNCHRONIZE

					//	single event
					if (commonEventType == S_NULL)
					{
						FireSingleEvent();
					}

					//	run phase
					else if (commonEventType == EVENT_RUN_PLAY)
					{
						//	run phase
						commonEventType = EVENT_RUN_PLAY;
						FireCommonEvent();
						commonEventType = EVENT_RUN_RESUME;
						FireCommonEvent();
						Service();

						//	these functions catch their own errors...
						commonEventType = EVENT_RUN_PAUSE;
						FireCommonEvent();
						commonEventType = EVENT_RUN_STOP;
						FireCommonEvent();

						//	report ended run phase
						endRunPhase();

						//	...so we must check for them afterwards
						if (flagState(F_THREAD_ERROR))
						{
							tout << "ThreadMain exits (1)" << D_VERB;
							return;
						}
					}

					//	common event
					else
					{
						FireCommonEvent();
					}
				}

			}

			catch(brahms::error::Error& e)
			{
				____AT(e);
				worker_error(e);
			}

			catch(std::exception se)
			{
				brahms::error::Error e(E_STD, se.what());
				____AT(e);
				worker_error(e);
			}

			catch(...)
			{
				brahms::error::Error e(E_UNRECOGNISED_EXCEPTION);
				____AT(e);
				worker_error(e);
			}

			//	these functions catch their own errors, so they will
			//	execute on all processes regardless of how many processes throw errors
			//	also, they only fire if the process is in the right state, so they
			//	are safe to try even if we've not made it up as far as expected...
			commonEventType = EVENT_RUN_PAUSE;
			FireCommonEvent();
			commonEventType = EVENT_RUN_STOP;
			FireCommonEvent();

			//	report ended run phase
			endRunPhase();

			//	these functions catch their own errors, so they will
			//	execute on all processes regardless of how many processes throw errors
			commonEventType = EVENT_MODULE_DESTROY;
			FireCommonEvent();

			//	end
			markCPUTime(PHS_TERM);

			//	report
			//tout << "ThreadMain exits (2)" << D_VERB;
		}

		void WorkerThread::endRunPhase()
		{
			if (!flag_runPhaseFinished)
			{
				flag_runPhaseFinished = true;
				runPhaseTimer.report();
			}
		}

		//	RDTSC instruction for performance testing
		inline INT64 rdtsc()
		{
	#ifdef __WIN__
			__asm
			{
				_emit 0x0F
				_emit 0x31
			}
	#endif

	#ifdef __NIX__
			INT64 x;
			__asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
			return x;
	#endif
		}

		void WorkerThread::Service()
		{
			//TOUT_SECTION("Service()");
			//TOUT_SECTION("EVENT_RUN_SERVICE");

			brahms::error::Error e;

			try
			{
				//	pars
				bool TimeRunPhase = engineData.core.execPars.getu("TimeRunPhase");
				bool ShowServicePhaseTiming = engineData.core.execPars.getu("ShowServicePhaseTiming");
				bool DetailLevelMax = tout.getLevel() == D_FULL;
				bool UseSlowVersion = TimeRunPhase || ShowServicePhaseTiming || DetailLevelMax;

				BaseSamples time_executionStop = time_stop;

				/*
					For uber-efficiency, we actually run entirely different
					code depending on what measurements we want to make...
				*/

				//	switch on version
				if (!UseSlowVersion)
				{
					#include "thread-service.c"
				}
				else
				{
					tout << "using slow version of inner loop" << D_WARN;

					#define USE_SLOW_VERSION
					#include "thread-service.c"
				}
				return;
			}

			catch(brahms::error::Error& se)
			{
				e = se;
				e.trace("at baseSamples = " + n2s(time_now));
				____AT(e);
			}

			catch(std::exception se)
			{
				e.code = E_STD;
				e.msg = se.what();
				e.trace("at baseSamples = " + n2s(time_now));
				____AT(e);
			}

			catch(...)
			{
				e.code = E_UNRECOGNISED_EXCEPTION;
				e.trace("at baseSamples = " + n2s(time_now));
				____AT(e);
			}

			tout << LOG_ERROR_MARKER << " (in worker thread main loop)" << D_VERB;
			tout << e.format(brahms::FMT_TEXT, true) << D_VERB;
			tout << "MAIN LOOP: Exit (with exception) @ t=" << n2s(time_now) << D_VERB;

			throw e;
		}

		void WorkerThread::FireCommonEvent()
		{
//			stringstream ss;
			//ss << "FireCommonEvent(" << brahms::base::symbol2string(commonEventType) << ")";
//			ss << brahms::base::symbol2string(commonEventType);
//			TOUT_SECTION(ss.str().c_str());

			for (UINT32 p=0; p<processes.size(); p++)
			{

			////////	PREAMBLE

				//	timing
				double t0 = threadTimer.elapsed();

				//	extract some info
				brahms::systemml::Process* process = processes[p];
				const ModuleInfo* moduleInfo = process->module->getInfo();

				//	signal thread active
				signalActive();



			////////	FIRE EVENT

				switch(commonEventType)
				{
					case EVENT_MODULE_CREATE:
					{
						//	call Component
						process->instantiate(&tout);

						//	store object in logEvent
						process->logEvent.object = process->object;

						//	create sets
						brahms::module::Module* module = process->module;
						for (UINT32 s=0; s<module->inputSets.size(); s++)
							process->addSet(F_IIF, module->inputSets[s].c_str());
						for (UINT32 s=0; s<module->outputSets.size(); s++)
							process->addSet(F_OIF, module->outputSets[s].c_str());

						//	ok
						process->irtWallclock.init += threadTimer.elapsed() - t0;
						break;
					}

					case EVENT_STATE_SET:
					{
						//	event data
						EventStateSet data;
						____CLEAR(data);
						data.state = process->nodeProcess->getChild("State")->getObjectHandle();

						//	fire event
						processBeingFired = process;
						EventEx event(
							commonEventType,
							process->flags,
							process,
							&data,
							false,
							&tout
						);
						event.fire();
						processBeingFired = NULL;

						//	ok
						process->irtWallclock.init += threadTimer.elapsed() - t0;
						break;
					}

					case EVENT_INIT_PRECONNECT:
					{
						/*
						//	resize outlets array to match links size
						process->outlets.resize(process->links.size());
						*/

						//	structure IIF (create an input port for each incoming link)
						process->structureInputInterface();

						//	fire event
						processBeingFired = process;
						EventEx event(
							commonEventType,
							process->flags,
							process,
							NULL,
							false,
							&tout
						);
						event.fire();
						processBeingFired = NULL;

						//	ok
						process->irtWallclock.init += threadTimer.elapsed() - t0;
						break;
					}

					case EVENT_INIT_POSTCONNECT:
					{
						//	fire event
						processBeingFired = process;
						EventEx event(
							commonEventType,
							process->flags,
							process,
							NULL,
							false,
							&tout
						);
						event.fire();
						processBeingFired = NULL;

						//	ok
						process->irtWallclock.init += threadTimer.elapsed() - t0;
						break;
					}

					case EVENT_BEGIN_RUNPHASE:
					{
						//	mark timing
						markCPUTime(PHS_INIT);

						//	only fire this event on bindings
						if (!( (moduleInfo->binding == 1258) || (moduleInfo->binding == 1262) ))
							continue;

						//	fire event
						processBeingFired = process;
						EventEx event(
							commonEventType,
							process->flags,
							process,
							NULL,
							false,
							&tout
						);
						event.fire();
						processBeingFired = NULL;

						//	ok
						process->irtWallclock.init += threadTimer.elapsed() - t0;
						break;
					}

					case EVENT_RUN_PLAY:
					{
						//	check state
						if (process->state != brahms::systemml::PRS_STOP)
							ferr << E_INTERNAL << "process state not PRS_STOP on call to Play()";

						//	set state
						process->state = brahms::systemml::PRS_PAUSE;

						//	fire event
						processBeingFired = process;
						EventEx event(
							commonEventType,
							process->flags,
							process,
							NULL,
							false,
							&tout
						);
						event.fire();
						processBeingFired = NULL;

						//	ok
						process->irtWallclock.init += threadTimer.elapsed() - t0;
						break;
					}

					case EVENT_RUN_RESUME:
					{
						//	check state
						if (process->state != brahms::systemml::PRS_PAUSE)
							ferr << E_INTERNAL << "process state not PRS_PAUSE on call to Resume()";

						//	set state
						process->state = brahms::systemml::PRS_PLAY;

						//	fire event
						processBeingFired = process;
						EventEx event(
							commonEventType,
							process->flags,
							process,
							NULL,
							false,
							&tout
						);
						event.fire();
						processBeingFired = NULL;

						//	ok
						process->irtWallclock.init += threadTimer.elapsed() - t0;
						break;
					}

					case EVENT_RUN_PAUSE:
					{
						try
						{
							//	check state
							if (process->state == brahms::systemml::PRS_STOP)
								break; // nothing to do here

							//	check state
							if (process->state == brahms::systemml::PRS_PAUSE)
								break; // nothing to do here

							//	check state (could be anything)
							if (process->state != brahms::systemml::PRS_PLAY)
								ferr << E_INTERNAL << "process state not PRS_PLAY on call to Pause()";

							//	set state
							process->state = brahms::systemml::PRS_PAUSE;

							//	fire event
							processBeingFired = process;
							EventEx event(
								commonEventType,
								process->flags,
								process,
								NULL,
								false,
								&tout
							);
							event.fire();
							processBeingFired = NULL;
						}

						catch(brahms::error::Error& e)
						{
							____AT(e);
							worker_error(e);
						}

						catch(std::exception se)
						{
							brahms::error::Error e(E_STD, se.what());
							____AT(e);
							worker_error(e);
						}

						catch(...)
						{
							brahms::error::Error e(E_UNRECOGNISED_EXCEPTION);
							____AT(e);
							worker_error(e);
						}

						//	ok
						process->irtWallclock.term += threadTimer.elapsed() - t0;
						break;
					}

					case EVENT_RUN_STOP:
					{
						try
						{
							//	check state
							if (process->state == brahms::systemml::PRS_STOP)
								break; // nothing to do here

							//	check state
							if (process->state != brahms::systemml::PRS_PAUSE)
								ferr << E_INTERNAL << "process state not PRS_PAUSE on call to Stop()";

							//	set state
							process->state = brahms::systemml::PRS_STOP;

							//	fire event
							processBeingFired = process;
							EventEx event(
								commonEventType,
								process->flags,
								process,
								NULL,
								false,
								&tout
							);
							event.fire();
							processBeingFired = NULL;
						}

						catch(brahms::error::Error& e)
						{
							____AT(e);
							worker_error(e);
						}

						catch(std::exception se)
						{
							brahms::error::Error e(E_STD, se.what());
							____AT(e);
							worker_error(e);
						}

						catch(...)
						{
							brahms::error::Error e(E_UNRECOGNISED_EXCEPTION);
							____AT(e);
							worker_error(e);
						}

						//	ok
						process->irtWallclock.term += threadTimer.elapsed() - t0;
						break;
					}

					case EVENT_BEGIN_TERMPHASE:
					{
						//	mark timing
						markCPUTime(PHS_RUN);

						//	only fire this event on bindings
						if (!( (moduleInfo->binding == 1258) || (moduleInfo->binding == 1262) ))
							continue;

						//	fire event
						processBeingFired = process;
						EventEx event(
							commonEventType,
							process->flags,
							process,
							NULL,
							false,
							&tout
						);
						event.fire();
						processBeingFired = NULL;

						//	ok
						process->irtWallclock.term += threadTimer.elapsed() - t0;
						break;
					}

					case EVENT_STATE_GET:
					{
						//	old node state may be replaced
						brahms::xml::XMLNode* oldNodeState = process->nodeProcess->getChild("State");

						//	event data
						EventStateGet data;
						____CLEAR(data);
						data.state = oldNodeState->getObjectHandle();
						data.precision = PRECISION_NOT_SET; // maximum

						//	fire event
						processBeingFired = process;
						EventEx event(
							commonEventType,
							process->flags,
							process,
							&data,
							true,
							&tout
						);
						event.fire();
						processBeingFired = NULL;

						//	retrieve node
						brahms::xml::XMLNode* newNodeState = objectRegister.resolveXMLNode(data.state);

						//	which can be either modified...
						if (newNodeState == oldNodeState)
						{
							//	process either modified the existing node, or cleared it and started again
							//	nothing more to do
						}

						//	or overwritten with a new node
						else
						{
							//	process overwrote the existing state node, so we need to remove this from
							//	the SystemML document and append the new one instead (generally, it is
							//	much more efficient if the process doesn't do this: they can either update
							//	the existing node, or clear it and start again - both approaches save us doing
							//	an extra copy operation here...)
							process->nodeProcess->replaceChild(newNodeState, oldNodeState);

							//	delete the now-orphaned old state node
							delete oldNodeState;
						}

						//	ok
						process->irtWallclock.term += threadTimer.elapsed() - t0;
						break;
					}

					case EVENT_MODULE_DESTROY:
					{
						try
						{
							//	have object destroyed
							processBeingFired = process;
							process->destroy(&tout);
							processBeingFired = NULL;
						}

						catch(brahms::error::Error& e)
						{
							____AT(e);
							worker_error(e);
						}

						catch(std::exception se)
						{
							brahms::error::Error e(E_STD, se.what());
							____AT(e);
							worker_error(e);
						}

						catch(...)
						{
							brahms::error::Error e(E_UNRECOGNISED_EXCEPTION);
							____AT(e);
							worker_error(e);
						}

						//	timing
						process->irtWallclock.term += threadTimer.elapsed() - t0;
						break;
					}

					default:
					{
						ferr << E_INTERNAL << "invalid event (0x" << hex << commonEventType << ") in FireCommonEvent()";
					}
				}
			}
		}


		void WorkerThread::FireSingleEvent()
		{
//			stringstream ss;
			//ss << "FireSingleEvent(" << brahms::base::symbol2string(singleEventEvent.type) << ")";
//			ss << brahms::base::symbol2string(singleEventEvent.type);
//			TOUT_SECTION(ss.str().c_str());

			//	timing
			double t0 = threadTimer.elapsed();

			//	expect response
			singleEventEvent.requireResponse = singleEventEvent.type == EVENT_INIT_CONNECT;

			//	augment flags
			singleEventEvent.flags |= singleEventEvent.component->flags;

			//	set tout
			singleEventEvent.tout = &tout;

			//	fire event
			brahms::systemml::Process* process = (brahms::systemml::Process*) singleEventEvent.component;
			processBeingFired = process;
			singleEventEvent.fire();
			processBeingFired = NULL;

			//	switch on event type
			switch(singleEventEvent.type)
			{
				case EVENT_INIT_CONNECT:
					process->irtWallclock.init += threadTimer.elapsed() - t0;
					break;

				case EVENT_LOG_INIT:
					process->irtWallclock.init += threadTimer.elapsed() - t0;
					break;

				case EVENT_LOG_TERM:
					process->irtWallclock.term += threadTimer.elapsed() - t0;
					break;

				default:
					ferr << E_INTERNAL << "invalid event (0x" << hex << singleEventEvent.type << ") in FireSingleEvent()";
			}
		}








	////////////////
	////////////////
	////////////////	WORKER THREAD MANAGER, "WORKERS"
	////////////////
	////////////////

		Workers::Workers(EngineData& p_engineData)
			: state(WS_ACTIVE), engineData(p_engineData)
		{
		}

		Workers::~Workers()
		{
			//	delete all threads
			for (UINT32 t=0; t<threads.size(); t++)
				delete threads[t];
		}

		WorkerThread* Workers::create(brahms::output::Sink* sink)
		{
			//	check state
			if (state != WS_ACTIVE) ferr << E_INTERNAL << "state!=WS_ACTIVE";

			//	get environment data
			UINT32 TimeoutThreadHang = engineData.environment.getu("TimeoutThreadHang");
			UINT32 TimeoutThreadTerm = engineData.environment.getu("TimeoutThreadTerm");

			//	create thread
			INT32 threadIndex = threads.size();
			WorkerThread* thread = new WorkerThread(threadIndex, sink, TimeoutThreadHang, TimeoutThreadTerm, engineData, runPhaseTimer);
			threads.push_back(thread);
			thread->start(TimeoutThreadTerm, ThreadProcWorker, thread);
			return thread;
		}

		WorkerThread* Workers::getThread(UINT32 index)
		{
			//	check state
			if (state != WS_ACTIVE) ferr << E_INTERNAL << "state!=WS_ACTIVE";

			if (index >= threads.size())
				ferr << E_INTERNAL << "index out of range in " << __FUNCTION__;
			return threads[index];
		}

		UINT32 Workers::getThreadCount()
		{
			//	check state
//			if (state != WS_ACTIVE) ferr << E_INTERNAL << "state!=WS_ACTIVE";

			return threads.size();
		}

		void Workers::terminate(brahms::output::Source& fout)
		{
			//	check state
			if (state == WS_ACTIVE)
			{
				//	terminate all threads
				for (UINT32 t=0; t<threads.size(); t++)
				{
					try
					{
						threads[t]->terminate(fout);
					}
					catch(brahms::error::Error e)
					{
						e.trace("whilst calling terminate() on Workers");
						threads[t]->storeError(e, fout);
					}
				}
			}

			//	change state
			state = WS_TERMINATED;
		}

		void Workers::markExecutionProgress(BaseSamples* tmin, BaseSamples* tmax)
		{
			//	if nothing, no action
			if (!threads.size()) return;

			//	assume first thread for both min and max
			*tmin = threads[0]->time_now;
			*tmax = threads[0]->time_now;

			//	for each other thread
			for (UINT32 t=1; t<threads.size(); t++)
			{
				BaseSamples tt = threads[t]->time_now;
				if (tt <= *tmin) *tmin = tt;
				if (tt >= *tmax) *tmax = tt;
			}
		}

		void Workers::fireSingleEvent(const EventEx& event)
		{
			//	have appropriate thread fire it
			brahms::systemml::Process* process = (brahms::systemml::Process*) event.component;
			if (!process) ferr << E_INTERNAL << "process is NULL";
			process->thread->fireSingleEvent(event);
		}

		RunPhaseTimer::RunPhaseTimer()
		{
			runPhaseBracketTime = 0.0;
			threadsLeftToReport = 0;
		}

		void RunPhaseTimer::start(UINT32 totalThreads)
		{
			threadsLeftToReport = totalThreads;
			timer.reset();
		}

		void RunPhaseTimer::report()
		{
			threadsLeftToReport--;
			if (!threadsLeftToReport)
				runPhaseBracketTime = timer.elapsed();
		}

		DOUBLE RunPhaseTimer::read()
		{
			return runPhaseBracketTime;
		}

		DOUBLE Workers::getRunPhaseTime()
		{
			return runPhaseTimer.runPhaseBracketTime;
		}

		void Workers::doRunPhase(BaseSamples executionStop)
		{
			/*
				We time run phase here. We read the system timer when run phase
				starts. When each thread ends run phase it calls back, and if
				it is the last thread to call back, we read the system timer
				again, and subtract to get an accurate bracket run phase time.
			*/
			runPhaseTimer.start(threads.size());

			for (UINT32 t=0; t<threads.size(); t++)
				threads[t]->doRunPhase(executionStop);
		}

		RunPhaseStatus Workers::runPhaseStatus(UINT64 TimeoutThreadHang)
		{
			RunPhaseStatus ret = RPS_FINISHED;

			//	if all threads have terminated, we're done, and can drop out
			for (UINT32 t=0; t<threads.size(); t++)
			{
				//	if not finished run phase, continue waiting
				if (!threads[t]->flag_runPhaseFinished)
				{
					//	mark that at least one thread is still in run phase
					ret = RPS_ACTIVE;

					//	check for hang - pass system source, because this
					//	function is only ever called by the main thread
					if (threads[t]->hasHung())
						return RPS_HUNG;
				}
			}

			//	ok
			return ret;
		}


		brahms::time::TIME_IRT_CPU Workers::getCPUTime(UINT32 t)
		{
			return threads[t]->getCPUTime();
		}

		vector<brahms::systemml::Process*> Workers::getProcesses(UINT32 t)
		{
				return threads[t]->processes;
		}




////////////////	END NAMESPACE

	}
}


