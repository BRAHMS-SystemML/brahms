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

	$Id:: thread.h 2241 2009-10-29 23:11:54Z benjmitch         $
	$Rev:: 2241                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-10-29 23:11:54 +0000 (Thu, 29 Oct 2009)       $
________________________________________________________________

*/




#ifndef INCLUDED_ENGINE_SYSTEMML_THREAD
#define INCLUDED_ENGINE_SYSTEMML_THREAD




////////////////	NAMESPACE

namespace brahms
{

	namespace thread
	{



	////////////////	RUN PHASE TIMER

		struct RunPhaseTimer
		{
			RunPhaseTimer();
			void start(UINT32 totalThreads);
			void report();
			DOUBLE read();

			DOUBLE runPhaseBracketTime;
			UINT32 threadsLeftToReport;
			brahms::os::Mutex mutex;
			brahms::os::Timer timer;
		};



	////////////////	WORKER THREAD

		class WorkerThread : public Thread
		{

			friend class Workers;

		public:

			//	tors
			WorkerThread(INT32 threadIndex, brahms::output::Sink* sink, UINT32 TimeoutThreadHang, UINT32 TimeoutThreadTerm, EngineData& engineData, RunPhaseTimer& runPhaseTimer);
			~WorkerThread();

			//	operations
			void load(brahms::systemml::Process* process);
			void waitForIdle();
			void doRunPhase(BaseSamples executionStop);

			bool hasHung();

			/*

				fireCommonEvent()
					fire an event on *all* processes in this thread

				fireSingleEvent()
					fire an event on just one particular process in this thread

			*/
			void fireCommonEvent(Symbol eventType);
			void fireSingleEvent(const EventEx& event);

			//	thread procedure
			void threadProc();

			//	timing
			void endRunPhase();

			//	signals
			void signalActive();

			//	terminate
			void terminate(brahms::output::Source& fout);

		private:

			//	signal to terminate
			bool stop;

			//	timing
			RunPhaseTimer& runPhaseTimer;

			//	thread errors
			void worker_error(brahms::error::Error& e, brahms::output::Source* source = NULL); /* only supply source if calling from another thread! */

			vector<brahms::systemml::Process*> processes;

			//	flags
			bool flag_runPhaseFinished;

			//	data passed from control-side to execution-side
			Symbol commonEventType;
			EventEx singleEventEvent;

			//	framework-thread signalling
			brahms::os::Signal signalIdle;
			brahms::os::Signal signalRelease;


			//	execution time
			BaseSamples time_now;
			BaseSamples time_stop;

			//	process that is currently firing (for error catching)
			brahms::systemml::Process* processBeingFired;

			//	synchronize
			bool Synchronize();

			//	event functions
			void FireSingleEvent();
			void FireCommonEvent();

			//	service loop
			void LockForWrite();
			void Service();

			//	pars
			UINT32 TimeoutThreadHang;
			UINT32 TimeoutThreadTerm;

			//	reference to engine data
			brahms::EngineData& engineData;

	////////////////	UNSORTED

			//	monitoring
			struct Monitor
			{
				Monitor()
				{
					current = 0;
					last = 0;
					tLastSawActivity = 0.0;
				}

				UINT64				current;
				UINT64				last;
				DOUBLE				tLastSawActivity;
			}
			monitor;

		};



	////////////////	WORKER MANAGER

		enum WorkersState
		{
			WS_ACTIVE = 0,
			WS_TERMINATED
		};

		enum RunPhaseStatus
		{
			RPS_FINISHED = 0,
			RPS_ACTIVE,
			RPS_HUNG,
			RPS_ERROR
		};

		class Workers
		{

		public:

			//	tors
			Workers(EngineData& engineData);
			~Workers();

			//	interface
			WorkerThread* create(brahms::output::Sink* sink);
			WorkerThread* getThread(UINT32 index);
			UINT32 getThreadCount();
			void doRunPhase(BaseSamples executionStop);
			RunPhaseStatus runPhaseStatus(UINT64 TimeoutThreadHang);

			//	terminate
			void terminate(brahms::output::Source& fout);

			void fireSingleEvent(const EventEx& event);

			void markExecutionProgress(BaseSamples* tmin, BaseSamples* tmax);

			brahms::time::TIME_IRT_CPU getCPUTime(UINT32 t);

			vector<brahms::systemml::Process*> getProcesses(UINT32 t);

			DOUBLE getRunPhaseTime();

		private:

			//	run phase timer
			RunPhaseTimer runPhaseTimer;

			//	state
			WorkersState state;

			//	list of worker threads
			vector<WorkerThread*> threads;

			//	reference to engine data
			EngineData& engineData;

		};



////////////////	NAMESPACE

	}
}



////////////////	INCLUSION GUARD

#endif
