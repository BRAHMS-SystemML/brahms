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

	$Id:: os.h 2386 2009-11-17 19:15:06Z benjmitch             $
	$Rev:: 2386                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-17 19:15:06 +0000 (Tue, 17 Nov 2009)       $
________________________________________________________________

*/



#ifndef INCLUDED_OS
#define INCLUDED_OS



#ifdef __WIN__
#define _WIN32_IE 0x0500
#define _WIN32_WINNT 0x0500
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#include "base/thread.h"
using brahms::thread::ThreadProcRet;

////////////////	NAMESPACE

	namespace os
	{



	////////////////	MUTEX

		struct Mutex
		{
			//	tors
			Mutex();
			~Mutex() throw();

			//	interface
			void lock();
			void release();

		private:

	#ifdef __WIN__

			//	native state
			CRITICAL_SECTION cs;

	#endif

	#ifdef __NIX__

			//	native mutex
			pthread_mutex_t mutex;

	#endif

		};

		struct MutexLocker
		{
			MutexLocker(Mutex& p_mutex) : mutex(p_mutex)
			{
				mutex.lock();
			}

			~MutexLocker()
			{
				mutex.release();
			}

		private:

			Mutex& mutex;
		};



	////////////////	SIGNAL

		const int SIGNAL_INFINITE_WAIT = 0;
		const int SIGNAL_WAIT_STEP = 100;

		struct Signal
		{
			//	initial state is "unset"
			Signal(int timeout, const bool* cancel);
			Signal(const Signal& src);
			~Signal() throw();

			//	interface
			void set();
			int waitfor();

		private:

			//	parameters
			int timeout;
			const bool* cancel;

	#ifdef __WIN__

			//	state
			HANDLE				hSignal;

	#endif

	#ifdef __NIX__

			//	state
			pthread_mutex_t		mutex;
			pthread_cond_t		cond;
			bool				state;

	#endif

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



	////////////////	THREAD CLASS

		typedef void (*ThreadProc)(void*);

		struct Thread
		{
			friend ThreadProcRet OSThreadProc(ThreadProcArg arg);

			Thread(ThreadProc proc, void* arg);
			bool is_terminated()
			{
				return terminated;
			}

			bool terminated;
			ThreadProc proc;
			void* arg;
			OS_THREAD osThread;
		};



	////////////////	FUNCTIONS

		void msleep(int msec);



	}



////////////////	INCLUSION GUARD

#endif
