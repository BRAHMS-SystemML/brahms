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

	$Id:: engine-execute.cpp 2282 2009-11-01 21:37:45Z benjmit#$
	$Rev:: 2282                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-01 21:37:45 +0000 (Sun, 01 Nov 2009)       $
________________________________________________________________

*/

#include "main/engine.h"
using brahms::Engine;
#include "base/output.h"
using namespace brahms::output;

void Engine::execute()
{
	FOUT_SECTION("Engine::execute()")

	monitorSendPhase(EP_EXECUTE, "Execute System");



////////////////	EVENT_BEGIN_RUNPHASE

	{ FOUT_SECTION("EVENT_BEGIN_RUNPHASE")

		//	**** SERIAL THREADING **** (see note higher up)

		//	monitor event
		monitorSendOperation("Sending EVENT_BEGIN_RUNPHASE...", true);

		//	release threads
		for (UINT32 t=0; t<workers.getThreadCount(); t++)
			workers.getThread(t)->fireCommonEvent(EVENT_BEGIN_RUNPHASE);

	}



	////////////////	GET EXEC PARS

		//	extract
		VUINT32 dat = engineData.environment.getulist("ThreadPollInterval", 3);
		DOUBLE ThreadPollIntervalFactor = 1.0 / ((DOUBLE)dat[0]);
		DOUBLE ThreadPollIntervalMin = dat[1] / 1000.0;
		DOUBLE ThreadPollIntervalMax = dat[2] / 1000.0;

		//	extract
		dat = engineData.environment.getulist("GUIUpdateInterval", 3);
		DOUBLE ProgbarUpdateIntervalFactor = 1.0 / ((DOUBLE)dat[0]);
		DOUBLE ProgbarUpdateIntervalMin = dat[1] / 1000.0;
		DOUBLE ProgbarUpdateIntervalMax = dat[2] / 1000.0;

		//	extract
		UINT64 TimeoutThreadHang = engineData.environment.getu("TimeoutThreadHang");
		bool ShowGUI = engineData.environment.getb("ShowGUI");



	////////////////	RELEASE THREADS

		{ FOUT_SECTION("Main Loop")

		//	for each threads, release to do run phase
		workers.doRunPhase(system.systemTime.executionStop);



	////////////////	RUNPHASE_RELEASE

		/*

			Both the "Thread Poll Interval" and the "Progbar Update Interval" are calculated
			on the fly as some fraction of the wallclock time, such that they are initially
			small and grow larger as the time spent grows larger. Both, in addition, have
			constraints to keep them in sensible ranges throughout.

		*/

		//	timers
		DOUBLE t_start = engineData.systemTimer.elapsed();
		DOUBLE t_now = 0.0;
		DOUBLE t_nextGUIUpdate = 0.0;

//			}



////////////////	RUNPHASE_LOOP

		//	run phase status
		brahms::thread::RunPhaseStatus rps = brahms::thread::RPS_ACTIVE;

		//	thread manager run loop
		fout << "enter main loop and release worker threads" << D_VERB;

		while(true)
		{
			//	thread poll interval
			DOUBLE ThreadPollInterval = t_now * ThreadPollIntervalFactor;
			ThreadPollInterval = max(ThreadPollInterval, ThreadPollIntervalMin);
			ThreadPollInterval = min(ThreadPollInterval, ThreadPollIntervalMax);
			UINT32 ThreadPollIntervalMS = ThreadPollInterval * 1000.0;
			brahms::os::msleep(ThreadPollIntervalMS);

			//	time now
			t_now = engineData.systemTimer.elapsed() - t_start;

			//	update progress bar
			if (ShowGUI && t_now > t_nextGUIUpdate)
			{
				//	calculate interval
				DOUBLE ProgbarUpdateInterval = t_now * ProgbarUpdateIntervalFactor;
				ProgbarUpdateInterval = max(ProgbarUpdateInterval, ProgbarUpdateIntervalMin);
				ProgbarUpdateInterval = min(ProgbarUpdateInterval, ProgbarUpdateIntervalMax);
				t_nextGUIUpdate = t_now + ProgbarUpdateInterval;

				//	get min and max times of threads/processes in system
				workers.markExecutionProgress(&system.systemTime.now, &system.systemTime.nowAdvance);

				//	get comms string from charlie
				string commsInfo = comms.getInfo();
				monitorEvent.message = commsInfo.c_str();

				//	callback event
				if (monitorSendEvent(EVENT_MONITOR_SERVICE) == C_CANCEL)
				{
					//	local cancel
					fout << "C_CANCEL returned from monitor callback" << D_VERB;
					engineData.core.condition.set(brahms::base::COND_LOCAL_CANCEL);
				}

				//	null
				monitorEvent.message = NULL;
			}

			//	check condition
			if (engineData.core.condition.get(brahms::base::COND_END_RUN_PHASE))
			{
				fout << "COND_END_RUN_PHASE met (exit main loop)" << D_VERB;
				break;
			}

			//	check worker threads state
			rps = workers.runPhaseStatus(TimeoutThreadHang);

			//	finished
			if (rps == brahms::thread::RPS_FINISHED)
			{
				fout << "SYSTEM MAIN LOOP EXIT: RPS_FINISHED" << D_VERB;
				break;
			}

			//	some hung?
			if (rps == brahms::thread::RPS_HUNG)
			{
				fout << "SYSTEM MAIN LOOP EXIT: RPS_HUNG" << D_VERB;
				break;
			}
		}

		fout << "exit main loop" << D_VERB;

	}


////////////////	PICK UP THREADS

	{ FOUT_SECTION("Postamble")

		//	propagate local cancel to peers
		if (engineData.core.condition.get(brahms::base::COND_LOCAL_CANCEL))
			comms.signalPeers(brahms::base::IPMTAG_CANCEL);

		//	propagate local error to peers
		if (engineData.core.condition.get(brahms::base::COND_LOCAL_ERROR))
			comms.signalPeers(brahms::base::IPMTAG_ERROR);

		//	wait for idle in worker threads
		fout << "wait for all worker threads to go idle..." << D_VERB;
		for (UINT32 t=0; t<workers.getThreadCount(); t++)
			workers.getThread(t)->waitForIdle();
		fout << "...OK" << D_VERB;

		//	finalize GUI
		monitorSendProgress(1.0, 1.0);
		monitorSendOperation("");

		//	signal monitor
		workers.markExecutionProgress(&system.systemTime.now, &system.systemTime.nowAdvance);

		//	wallclock time
		irtWallclock.run += workers.getRunPhaseTime();

		//	CPU time
		irtCPU.run = brahms::thread::getthreadtimes();

	}

}
