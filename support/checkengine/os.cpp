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

	$Id:: os.cpp 2386 2009-11-17 19:15:06Z benjmitch           $
	$Rev:: 2386                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-17 19:15:06 +0000 (Tue, 17 Nov 2009)       $
________________________________________________________________

*/


#include "os.h"

#ifdef __NIX__
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#endif
	namespace os
	{

	////////////////	MUTEX

		/*

			We use a critical section on windows, is a little faster,
			and we don't need cross-process sync.

			NOTE that a windows CRITICAL_SECTION is *not* a mutex in
			the conventional sense - it can be locked as many times as
			required by the same thread, i.e. this code:

			Mutex mutex;
			mutex.lock();
			mutex.lock();
			mutex.release();
			mutex.release();
			print "OK";

			will work just fine. but it will prevent access by another
			thread.

		*/

	#ifdef __WIN__

		Mutex::Mutex()
		{
			InitializeCriticalSection(&cs);
		}

		Mutex::~Mutex() throw()
		{
			DeleteCriticalSection(&cs);
		}

		void Mutex::lock()
		{
			EnterCriticalSection(&cs);
		}

		void Mutex::release()
		{
			LeaveCriticalSection(&cs);
		}


	#endif

	#ifdef __NIX__

		Mutex::Mutex()
		{
			if (pthread_mutex_init(&mutex, NULL)) throw "pthread_mutex_init() failed";
		}

		Mutex::~Mutex() throw()
		{
			pthread_mutex_destroy(&mutex);
		}

		void Mutex::lock()
		{
			if (pthread_mutex_lock(&mutex)) throw "pthread_mutex_lock() failed";
		}

		void Mutex::release()
		{
			if (pthread_mutex_unlock(&mutex)) throw "pthread_mutex_unlock() failed";
		}

	#endif





	////////////////	SIGNAL

#ifdef __NIX__
		void addToTimeOfDay(int milliseconds, struct timespec* timespec)
		{
			struct timeval	tv;

			//      add milliseconds to time of day
			gettimeofday(&tv, NULL);
			tv.tv_sec += milliseconds / 1000;
			tv.tv_usec += (milliseconds % 1000) * 1000;

			//	simplify tv
			while (tv.tv_usec >= 1000000)
			{
					tv.tv_sec++;
					tv.tv_usec -= 1000000;
			}

			//      convert to timespec
			timespec->tv_sec = tv.tv_sec;
			timespec->tv_nsec = tv.tv_usec * 1000;
		}
#endif

		//	global false used by signals that don't have their own cancel
		bool Signal_global_false = false;

		Signal::Signal(int p_timeout, const bool* p_cancel)
		{

			//	store
			timeout = p_timeout;
			cancel = p_cancel ? p_cancel : &Signal_global_false;

		#ifdef __WIN__

			//	create signal, initially unset
			hSignal = CreateEvent(NULL, false, false, NULL);
			if (!hSignal) throw "CreateEvent() failed";

		#endif

		#ifdef __NIX__

			//	create signal, initially unset
			if (pthread_mutex_init(&mutex, NULL)) throw "pthread_mutex_init() failed";
			if (pthread_cond_init(&cond, NULL)) throw "pthread_cond_init() failed";

			//	set state
			state = false;

		#endif

		}

		Signal::Signal(const Signal& src)
		{
			//	too complicated - let's just not allow it
			throw "called copy ctor of Signal";
		}

		Signal::~Signal() throw()
		{

		#ifdef __WIN__

			//	destroy signal
			if (hSignal && !CloseHandle(hSignal))
				cerr << "CloseHandle() failed in ~Signal" << endl;

		#endif

		#ifdef __NIX__

			//	destroy signal
			{
				//	both mutex and cond destroy functions will fail if someone is
				//	currently waiting on the signal. since the waiter will not
				//	wait forever, we are happy to here wait until the waiter is
				//	finished (at least for some time).

				int t = 0;

				//	can't get this from global settings, because we might
				//	not have a settings object yet if we're going down early!
				//	(or rather it might not have any settings in it)
				int TimeoutDestroySyncObject = 1000;

				while (true)
				{
					//	destroy it
					int eno = pthread_mutex_destroy(&mutex);
					if (!eno) break;

					//	busy error
					if (eno == EBUSY)
					{
						//	wait and increment timer
						os::msleep(100);
						t += 100;

						//	check for long timeout
						if (t >= TimeoutDestroySyncObject)
						{
							break;
						}

						//	go round again
						continue;
					}

					//	other error
					break;
				}

				while (true)
				{
					//	destroy it
					int eno = pthread_cond_destroy(&cond);
					if (!eno) break;

					//	busy error
					if (eno == EBUSY)
					{
						//	wait and increment timer
						os::msleep(100);
						t += 100;

						//	check for long timeout
						if (t >= TimeoutDestroySyncObject)
						{
							break;
						}

						//	go round again
						continue;
					}

					//	other error
					break;
				}
			}

		#endif

		}

		void Signal::set()
		{

		#ifdef __WIN__

			//	set signal
			if (!SetEvent(hSignal)) throw "failed to set signal (SetEvent())";

		#endif

		#ifdef __NIX__

			//	acquire exclusive access to state
			if (pthread_mutex_lock(&mutex)) throw "failed to set signal (lock mutex)";

			//	set signal
			state = true;

			//	the other thread may be waiting for us to set the signal, so set it free
			//	pthread_cond_signal has no effect if waiting thread is not yet waiting
			if (pthread_cond_signal(&cond)) throw "failed to set signal (signal cond)";

			//	release exclusive access to state
			if (pthread_mutex_unlock(&mutex)) throw "failed to set signal (unlock mutex)";

		#endif

		}

		int Signal::waitfor()
		{

		#ifdef __WIN__

			//	multiple short waits, to allow cancel
			int waited = 0;

			//	enter loop
			while (true)
			{
				//	check cancel
				if (*cancel) return 0;

				//	check signal
				if (WaitForSingleObject(hSignal, SIGNAL_WAIT_STEP) == WAIT_OBJECT_0)
					return 1;

				//	handle timeout
				if (timeout != SIGNAL_INFINITE_WAIT)
				{
					//	advance timer
					waited += SIGNAL_WAIT_STEP;

					//	timeout
					if (waited >= timeout)
						return -1;
				}
			}

		#endif

		#ifdef __NIX__

			//	acquire exclusive access to state
			if (pthread_mutex_lock(&mutex)) throw "failed to waitfor (lock mutex)";

			//	multiple short waits, to allow cancel
			int waited = 0;

			//	if signal is not yet set, wait for it to be set, with timeout
			while (!state)
			{
				//	check cancel
				if (*cancel)
				{
					//	release exclusive access to state
					if (pthread_mutex_unlock(&mutex))
						throw "failed to waitfor (unlock mutex)";

					//	report cancel
					return 0;
				}

				//	calculate time to wait until
				struct timespec timespec;
				addToTimeOfDay(SIGNAL_WAIT_STEP, &timespec);

				//	wait
				pthread_cond_timedwait(&cond, &mutex, &timespec);
				if (state) break;

				//	advance timer and wait again
				waited += SIGNAL_WAIT_STEP;

				//	timeout
				if ((timeout != SIGNAL_INFINITE_WAIT) && (waited >= timeout))
				{
					//	release exclusive access to state
					if (pthread_mutex_unlock(&mutex))
						throw "failed to waitfor (unlock mutex)";

					//	report timeout
					return -1;
				}
			}

			//	unset signal
			state = false;

			//	release exclusive access to state
			if (pthread_mutex_unlock(&mutex)) throw "failed to waitfor (unlock mutex)";

			//	ok
			return 0;

		#endif

		}



////////////////	THREAD

		ThreadProcRet OSThreadProc(ThreadProcArg p_arg)
		{
			Thread* arg = (Thread*) p_arg;
			arg->proc(arg->arg);
			arg->terminated = true;
			return 0;
		}

		Thread::Thread(ThreadProc p_proc, void* p_arg)
		{
			terminated = false;
			proc = p_proc;
			arg = p_arg;

	#ifdef __WIN__

			if (!(osThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)&OSThreadProc, this, 0, 0 )))
				throw "failed start thread";

	#endif

	#ifdef __NIX__

			//	create thread attribute
			pthread_attr_t attr;
			if (pthread_attr_init(&attr))
				throw "failed to create thread attribute";

			//	set up attribute
			if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE))
			{
				pthread_attr_destroy(&attr);
				throw "failed to set thread attribute";
			}

			//	create thread (use default priority)
			if (pthread_create(&osThread, &attr, &OSThreadProc, this))
			{
				pthread_attr_destroy(&attr);
				throw "failed to create thread";
			}

			//	destroy thread attribute
			if (pthread_attr_destroy(&attr))
				throw "failed to destroy thread attribute";

	#endif

		}



////////////////	MSLEEP

		void msleep(int msec)
		{

		#ifdef __WIN__
			Sleep(msec);
		#endif

		#ifdef __NIX__
			usleep(msec * 1000);
		#endif

		}



////////////////	NAMESPACE

	}
