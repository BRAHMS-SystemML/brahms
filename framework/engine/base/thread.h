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

	$Id:: thread.h 2283 2009-11-02 00:22:31Z benjmitch         $
	$Rev:: 2283                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-02 00:22:31 +0000 (Mon, 02 Nov 2009)       $
________________________________________________________________

*/



#ifndef INCLUDED_BRAHMS_FRAMEWORK_BASE_THREAD
#define INCLUDED_BRAHMS_FRAMEWORK_BASE_THREAD



////////////////	NAMESPACE

namespace brahms { namespace base { class Core; } }

namespace brahms
{
	namespace thread
	{



////////////////	THREAD CLASS

		enum ThreadClass
		{
			TC_NULL = 0,
			TC_CALLER,
			TC_WORKER,
			TC_SENDER,
			TC_RECEIVER,
			TC_DELIVERER
		};



////////////////	THREAD STATE

		//	thread state
		enum ThreadState
		{
			TS_VIRGIN = 0,					//	default state
			TS_ACTIVE,						//	thread has been started (start() has been called)
			TS_FINISHED,					//	thread procedure has exited cleanly (without segfault)
			TS_START_FAILED,				//	thread procedure was not successfully started
			TS_TERMINATED					//	terminate() has been called
		};



////////////////	RUN PHASE

		enum RunPhase
		{
			PHS_PREV,
			PHS_INIT,
			PHS_RUN,
			PHS_TERM
		};



////////////////	THREAD PROTOTYPES

	#ifdef __WIN__

		typedef	UINT32 ThreadProcRet;
		typedef void* ThreadProcArg;
		typedef HANDLE OS_THREAD;

	#endif

	#ifdef __NIX__

		typedef void* ThreadProcRet;
		typedef void* ThreadProcArg;
		typedef pthread_t OS_THREAD;

	#endif



////////////////	THREAD BASE

		const UINT32 F_THREAD_ERROR =		0x00000001;
		const UINT32 F_THREAD_HUNG =		0x00000002;

		//	base support for thread (calling thread, not maintained by us, also has this, which is why it's separate)
		struct ThreadBase
		{
			//	tors
			ThreadBase(ThreadClass threadClass, UINT32 threadIndex, brahms::base::Core& core);

			//	flags
			bool flagState(const UINT32 flag);
			void setFlag(const UINT32 flag);

			//	id
			ThreadClass getThreadClass();
			INT32 getThreadIndex();
			const char* getThreadIdentifier();

			//	errors
			void storeError(brahms::error::Error& error, brahms::output::Source& tout);

		private:
		
			//	flags
			UINT32 flags;

			//	ref to core
			brahms::base::Core& core;

			//	id
			ThreadClass threadClass;
			INT32 threadIndex;
			string threadIdentifier;

		protected:

			//	use mutex to protect thread's data so that other
			//	threads can operate on this thread's data
			brahms::os::Mutex threadMutex;

		public:

			//	thread-private timer
			brahms::os::Timer threadTimer;

			//	thread-private output source
			brahms::output::Source tout;
		};



////////////////	THREAD CLASS

		typedef void (*ThreadProc)(void*);

		struct Thread : public ThreadBase
		{

			friend ThreadProcRet osThreadProc(ThreadProcArg arg);
			friend class Workers;

			//	tors
			Thread(ThreadClass threadClass, UINT32 threadIndex, brahms::base::Core& core);
			virtual ~Thread();

			//	control of the thread from outside
			void start(UINT32 TimeoutThreadTerm, ThreadProc threadProc, void* threadProcArg);	//	start thread running (state to TS_ACTIVE)
			bool isActive();						//	return true if thread has been start()ed but has not yet finished (i.e. if state is TS_ACTIVE)
			void terminate(brahms::output::Source& fout, UINT32 t_alreadyWaited = 0);	//	terminate gracefully or kill if necessary (state to TS_TERMINATED)

			//	timing
			void markCPUTime(RunPhase phs);		//	mark thread CPU times
			brahms::time::TIME_IRT_CPU getCPUTime()
			{
				return threadCPUTime;
			}

			//	parameters
			UINT32 TimeoutThreadTerm;

		private:

			//	thread mechanics
			ThreadState state;				//	indicates progression of thread through its life cycle
			ThreadProc threadProc;			//	address of the client's thread procedure
			void* threadProcArg;			//	argument to pass to the client's thread procedure

			//	timing
			brahms::time::TIME_IRT_CPU threadCPUTime;

			//	os-specific thread handle
			OS_THREAD osThread;

		};



////////////////	THREAD TIME

		brahms::time::TIME_CPU getthreadtimes();



	}
}


////////////////	INCLUSION GUARD

#endif


