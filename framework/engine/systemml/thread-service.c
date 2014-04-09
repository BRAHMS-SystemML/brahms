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

	$Id:: thread-service.c 2278 2009-11-01 21:23:08Z benjmitch $
	$Rev:: 2278                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-01 21:23:08 +0000 (Sun, 01 Nov 2009)       $
________________________________________________________________

*/




	//	performance
#ifdef USE_SLOW_VERSION
	INT64 t_tot = rdtsc();
	INT64 t_lock = 0; // service process
	INT64 t_step = 0, t_pre_step = 0, t_post_step = 0; // service data
	INT64 t_unlock_rd = 0, t_unlock_wr = 0; // housekeeping
	INT64 t1 = 0, t2 = 0;
#endif

	const bool* globalStop = engineData.core.condition.get_p(brahms::base::COND_END_RUN_PHASE);

	//	EVENT_RUN_SERVICE
	Event serviceEvent;
	serviceEvent.type = EVENT_RUN_SERVICE;
	serviceEvent.flags = 0;
	serviceEvent.object = NULL;
	serviceEvent.data = NULL;



////////////////	SET INITIAL PROCESS SERVICE TIMES

	/*

		All processes have had their "now" set to either 0 (if the system
		is unprogressed) or non-zero (if the system is in progression).
		Here, we advance the now timer to the end of the execution if the
		process has no inputs or outputs, so that it doesn't get in the
		way in our timing calculations, below. We'll catch it up when we
		get to the end. This is because processes that have no i's or o's
		actually receive no service calls, so we've nothing to do.

	*/

	for (UINT32 p=0; p<processes.size(); p++)
	{
		//	get reference to process object
		brahms::systemml::Process* process = processes[p];

		//	count i/o
		UINT32 ni = process->iif.getPortCount();
		UINT32 no = process->oif.getPortCount();

		//	set time
		if (!(ni+no))
			process->componentTime.now = time_executionStop;
	}








#define REPORT_EXIT_MAIN_LOOP(reason) { tout << "WORKER MAIN LOOP EXIT: " << reason << D_VERB; }
#define EXIT_MAIN_LOOP(reason) { REPORT_EXIT_MAIN_LOOP(reason); break; }

//	return smaller of two values (just making absolutely sure we know what this does by defining it ourselves - some cross-platform issues...)
#define SMALLEROF(a, b) (a<b ? a : b)



////////////////	ENTER MAIN LOOP

	//	report
	tout << "WORKER MAIN LOOP ENTER" << D_VERB;

	//	cache event handler for each process
	for (UINT32 p=0; p<processes.size(); p++)
		processes[p]->eventHandler = processes[p]->module->getHandler();


	//	loop over time
	while(true)
	{



	////////////////	GET EARLIEST SERVICE TIME AMONGST PROCESSES

		//	failing all else, our next time for servicing processes will be the execution stop
		BaseSamples time_nextThreadService = time_executionStop;

		//	get next Thread Service Time (next service time for the whole thread,
		//	the earliest service time amongst all processes wrapped in this Thread).
		for (UINT32 p=0; p<processes.size(); p++)
		{
			//	check its next sample time
			brahms::systemml::Process* process = processes[p];
			time_nextThreadService = SMALLEROF(time_nextThreadService, process->componentTime.now);
		}

		//	lay it in to thread's "now" time
		time_now = time_nextThreadService;

		//tout << "thread service t = " << time_now << D_INFO;



	////////////////	STOP IF REACHED REQUESTED STOP TIME

		//	if reached execution stop time
		if (time_now >= time_executionStop)
			EXIT_MAIN_LOOP("REACHED EXECUTION STOP");



	////////////////	SERVICE EACH PROCESS THAT NEEDS SERVICING

		//	for each process
		for (UINT32 p=0; p<processes.size(); p++)
		{

			brahms::systemml::Process* processBeingServiced = processes[p];



		////////////////	MARK ACTIVE BETWEEN EVERY PROCESS EVENT

			signalActive();



			////	IGNORE IF NOT DUE

			//	check if due to be serviced
			if (time_now != processBeingServiced->componentTime.now) continue;

			//	break on cancel
			if (*globalStop)
			{
				EXIT_MAIN_LOOP("global stop (1)");
			}



#ifdef USE_SLOW_VERSION
	if (ShowServicePhaseTiming)
	{
		t1 = rdtsc();
	}
#endif



#ifdef DEBUG_LOCKS
		tout << "ACQUIRE LOCKS @ t = " << time_now << D_INFO;
#endif

			////	ACQUIRE LOCKS

			if (processBeingServiced->readAndWriteLock(time_now, &tout) == C_CANCEL)
			{
				EXIT_MAIN_LOOP("C_CANCEL whilst locking ports");
			}

			//	break on cancel
			if (*globalStop)
			{
				EXIT_MAIN_LOOP("global stop (2)");
			}

#ifdef DEBUG_LOCKS
		tout << "ACQUIRE LOCKS (OK)" << D_INFO;
#endif





#ifdef USE_SLOW_VERSION
	if (ShowServicePhaseTiming)
	{
		t2 = rdtsc();
		t_lock += t2 - t1;
		t1 = t2;
	}
#endif



			////	SERVICE PROCESS

	#ifdef USE_SLOW_VERSION
			//	report service entry
			if (DetailLevelMax)
				tout << processBeingServiced->getObjectName() << "->service(" << processBeingServiced->componentTime.now << ")" << D_FULL;
	#endif

			//	set event flags and data (type already set, object set below)
			serviceEvent.flags = processBeingServiced->flags;

			//	set event object
			serviceEvent.object = processBeingServiced->object;

			//	store error information
			processBeingFired = processBeingServiced;

	#ifdef USE_SLOW_VERSION
			//	timer
			DOUBLE t0 = 0.0;
			if (TimeRunPhase)
				t0 = threadTimer.elapsed();
	#endif

#ifdef USE_SLOW_VERSION
	if (ShowServicePhaseTiming)
	{
		t2 = rdtsc();
		t_pre_step += t2 - t1;
		t1 = t2;
	}
#endif

			//	fire EVENT_RUN_SERVICE
			Symbol err = processBeingServiced->eventHandler(&serviceEvent);

#ifdef USE_SLOW_VERSION
	if (ShowServicePhaseTiming)
	{
		t2 = rdtsc();
		t_step += t2 - t1;
		t1 = t2;
	}
#endif

	#ifdef USE_SLOW_VERSION
			//	timer
			if (TimeRunPhase)
				processBeingServiced->irtWallclock.run += threadTimer.elapsed() - t0;
	#endif

			processBeingFired = NULL;

	#ifdef USE_SLOW_VERSION
			//	report service entry
			if (DetailLevelMax)
				tout << "(event returned)" << D_FULL;
	#endif













			//	catch error and cancel
			if (err != C_OK)
			{
				//	process name
				processBeingFired = processBeingServiced;
				string processName = processBeingServiced->getObjectName();
				string trace = "whilst firing \"EVENT_RUN_SERVICE\" on \"" + processName + "\"";

				//	catch errors
				if (S_ERROR(err))
				{
					brahms::error::Error e = brahms::error::globalErrorRegister.pull(err);
					e.trace(trace);
					throw e;
				}

				//	otherwise
				switch(err)
				{
					case S_NULL:
					{
						ferr << E_NOT_COMPLIANT << "no response from process whilst handling required event";
					}

					case C_STOP_USER:
					case C_STOP_EXTERNAL:
					case C_STOP_CONDITION:
					case C_STOP_THEREFOREIAM:
					{
						//	signal local
						engineData.core.condition.set(brahms::base::COND_LOCAL_CANCEL);

						//	exit
						string reason = brahms::base::symbol2string(err);
						reason += (" from \"" + processName + "\"");
						EXIT_MAIN_LOOP(reason);
					}

					default:
					{
						ferr << E_NOT_COMPLIANT << "unrecognised response from process (0x" << hex << err << ")";
					}
				}
			}



#ifdef USE_SLOW_VERSION
	if (ShowServicePhaseTiming)
	{
		t2 = rdtsc();
		t_post_step += t2 - t1;
		t1 = t2;
	}
#endif







#ifdef DEBUG_LOCKS
		tout << "RELEASE READ LOCKS" << D_INFO;
#endif

			////	RELEASE READ LOCKS

			BaseSamples time_nextProcessService = processBeingServiced->readRelease(time_now, time_executionStop);

			//	break on cancel
			if (*globalStop)
			{
				EXIT_MAIN_LOOP("global stop (3)");
			}

#ifdef DEBUG_LOCKS
		tout << "RELEASE READ LOCKS (OK)" << D_INFO;
#endif



/*

	First, we release all read locks on our inputs. Then, for each output in turn, we
	propagate the output (fire EVENT_LOG_SERVICE and/or send it into the comms layer)
	and then release the write lock. I'm not entirely clear why we do this in two separate
	phases, but I think it's important. It might have somehting to do with the possibility
	that one of our outputs goes directly to one of our inputs. I'm really not sure.

*/



#ifdef USE_SLOW_VERSION
	if (ShowServicePhaseTiming)
	{
		t2 = rdtsc();
		t_unlock_rd += t2 - t1;
		t1 = t2;
	}
#endif

#ifdef DEBUG_LOCKS
		tout << "RELEASE WRITE LOCKS" << D_INFO;
#endif

			////	RELEASE WRITE LOCKS (AND PROPAGATE OUTPUTS)

			//	also advances process clock
#ifdef USE_SLOW_VERSION
			processBeingServiced->writeRelease(time_now, time_nextProcessService, DetailLevelMax ? &tout : NULL);
#else
			processBeingServiced->writeRelease(time_now, time_nextProcessService, NULL);
#endif

			//	break on cancel
			if (*globalStop)
			{
				EXIT_MAIN_LOOP("global stop (4)");
			}

#ifdef DEBUG_LOCKS
		tout << "RELEASE WRITE LOCKS (OK)" << D_INFO;
#endif







#ifdef USE_SLOW_VERSION
	if (ShowServicePhaseTiming)
	{
		t2 = rdtsc();
		t_unlock_wr += t2 - t1;
		t1 = t2;
	}
#endif

		} // for each process



	////////////////	CHECK FOR VOICE-GLOBAL STOP CONDITION

		//	if a process or the user has asked us to cancel, or an error has occurred
		if (*globalStop)
			EXIT_MAIN_LOOP("CONDITION STOP");

	} // while(true)






	//	report
	string s = "TIME NOW = " + n2s(time_now);
	REPORT_EXIT_MAIN_LOOP(s);



////////////////	REPORT SERVICE PHASE TIMING

#ifdef USE_SLOW_VERSION

	if (ShowServicePhaseTiming)
	{
		tout << "thread " << getThreadIdentifier() << " performance data:" << D_INFO;
		t_tot = rdtsc() - t_tot;
//		tout.precision(1);
//		tout.setf(ios::fixed, ios::floatfield);
		tout << ios::fixed << ios::floatfield;
		tout << "lock        % " << ((double)t_lock)/((double)t_tot)*100.0 << " (" << t_lock << ")" << D_INFO;
		tout << "pre step    % " << ((double)t_pre_step)/((double)t_tot)*100.0 << " (" << t_pre_step << ")" << D_INFO;
		tout << "step        % " << ((double)t_step)/((double)t_tot)*100.0 << " (" << t_step << ")" << D_INFO;
		tout << "post step   % " << ((double)t_post_step)/((double)t_tot)*100.0 << " (" << t_post_step << ")" << D_INFO;
		tout << "unl rd      % " << ((double)t_unlock_rd)/((double)t_tot)*100 << " (" << t_unlock_rd << ")" << D_INFO;
		tout << "unl wr      % " << ((double)t_unlock_wr)/((double)t_tot)*100 << " (" << t_unlock_wr << ")" << D_INFO;
		UINT64 t_lost = t_tot - t_lock - t_pre_step - t_step - t_post_step - t_unlock_rd - t_unlock_wr;
		tout << "lost        % " << ((double)t_lost)/((double)t_tot)*100 << " (" << t_lost << ")" << D_INFO;
		DOUBLE el = ((DOUBLE)t_tot) / 100000.0;
		tout << "elapsed: " << el << D_INFO;
	}

#endif



