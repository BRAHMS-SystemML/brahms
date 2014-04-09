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

	$Id:: os.cpp 2288 2009-11-02 13:49:31Z benjmitch           $
	$Rev:: 2288                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-02 13:49:31 +0000 (Mon, 02 Nov 2009)       $
________________________________________________________________

*/


#ifdef __NIX__

#include <sys/time.h>
#include <errno.h>

#endif
                            
                            
                            
namespace brahms
{
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
			if (pthread_mutex_init(&mutex, NULL)) ferr << E_OS << "pthread_mutex_init() failed";
		}

		Mutex::~Mutex() throw()
		{
			pthread_mutex_destroy(&mutex);
		}

		void Mutex::lock()
		{
			if (pthread_mutex_lock(&mutex)) ferr << E_OS << "pthread_mutex_lock() failed";
		}

		void Mutex::release()
		{
			if (pthread_mutex_unlock(&mutex)) ferr << E_OS << "pthread_mutex_unlock() failed";
		}

	#endif





	////////////////	SIGNAL

#ifdef __NIX__
		void addToTimeOfDay(UINT32 milliseconds, struct timespec* timespec)
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

		Signal::Signal(UINT32 p_timeout, const bool* p_cancel)
		{

			//	store
			timeout = p_timeout;
			cancel = p_cancel ? p_cancel : &Signal_global_false;

		#ifdef __WIN__

			//	create signal, initially unset
			hSignal = CreateEvent(NULL, false, false, NULL);
			if (!hSignal) ferr << E_OS << "CreateEvent() failed";

		#endif

		#ifdef __NIX__

			//	create signal, initially unset
			if (pthread_mutex_init(&mutex, NULL)) ferr << E_OS << "pthread_mutex_init() failed";
			if (pthread_cond_init(&cond, NULL)) ferr << E_OS << "pthread_cond_init() failed";

			//	set state
			state = false;

		#endif

		}

		Signal::Signal(const Signal& src)
		{
			//	too complicated - let's just not allow it
			ferr << E_INTERNAL << "called copy ctor of Signal";
		}

		Signal::~Signal() throw()
		{

		#ifdef __WIN__

			//	destroy signal
			if (hSignal && !CloseHandle(hSignal))
				____WARN("CloseHandle() failed in ~Signal");

		#endif

		#ifdef __NIX__

			//	destroy signal
			{
				//	both mutex and cond destroy functions will fail if someone is
				//	currently waiting on the signal. since the waiter will not
				//	wait forever, we are happy to here wait until the waiter is
				//	finished (at least for some time).

				UINT32 t = 0;

				//	can't get this from global settings, because we might
				//	not have a settings object yet if we're going down early!
				//	(or rather it might not have any settings in it)
				UINT32 TimeoutDestroySyncObject = 1000;

				while (true)
				{
					//	destroy it
					int eno = pthread_mutex_destroy(&mutex);
					if (!eno) break;

					//	busy error
					if (eno == EBUSY)
					{
						//	wait and increment timer
						brahms::os::msleep(100);
						t += 100;

						//	check for long timeout
						if (t >= TimeoutDestroySyncObject)
						{
							____WARN("TIMEOUT waiting to destroy mutex (" << TimeoutDestroySyncObject << " milliseconds) - this will happen if a thread is waiting on it(?)");
							break;
						}

						//	go round again
						continue;
					}

					//	other error
					____WARN("pthread_mutex_destroy() failed in ~Signal");
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
						brahms::os::msleep(100);
						t += 100;

						//	check for long timeout
						if (t >= TimeoutDestroySyncObject)
						{
							____WARN("TIMEOUT waiting to destroy cond (" << TimeoutDestroySyncObject << " milliseconds) - this will happen if a thread is waiting on it(?)");
							break;
						}

						//	go round again
						continue;
					}

					//	other error
					____WARN("pthread_cond_destroy() failed in ~Signal");
					break;
				}
			}

		#endif

		}

		void Signal::set()
		{

		#ifdef __WIN__

			//	set signal
			if (!SetEvent(hSignal)) ferr << E_OS << "failed to set signal (SetEvent())";

		#endif

		#ifdef __NIX__

			//	acquire exclusive access to state
			if (pthread_mutex_lock(&mutex)) ferr << E_OS << "failed to set signal (lock mutex)";

			//	set signal
			state = true;

			//	the other thread may be waiting for us to set the signal, so set it free
			//	pthread_cond_signal has no effect if waiting thread is not yet waiting
			if (pthread_cond_signal(&cond)) ferr << E_OS << "failed to set signal (signal cond)";

			//	release exclusive access to state
			if (pthread_mutex_unlock(&mutex)) ferr << E_OS << "failed to set signal (unlock mutex)";

		#endif

		}

		Symbol Signal::waitfor()
		{

		#ifdef __WIN__

			//	multiple short waits, to allow cancel
			UINT32 waited = 0;

			//	enter loop
			while (true)
			{
				//	check cancel
				if (*cancel) return C_CANCEL;

				//	check signal
				if (WaitForSingleObject(hSignal, SIGNAL_WAIT_STEP) == WAIT_OBJECT_0)
					return C_OK;

				//	handle timeout
				if (timeout != SIGNAL_INFINITE_WAIT)
				{
					//	advance timer
					waited += SIGNAL_WAIT_STEP;

					//	timeout
					if (waited >= timeout)
						return E_SYNC_TIMEOUT;
				}
			}

		#endif

		#ifdef __NIX__

			//	acquire exclusive access to state
			if (pthread_mutex_lock(&mutex)) ferr << E_OS << "failed to waitfor (lock mutex)";

			//	multiple short waits, to allow cancel
			UINT32 waited = 0;

			//	if signal is not yet set, wait for it to be set, with timeout
			while (!state)
			{
				//	check cancel
				if (*cancel)
				{
					//	release exclusive access to state
					if (pthread_mutex_unlock(&mutex))
						ferr << E_OS << "failed to waitfor (unlock mutex)";

					//	report cancel
					return C_CANCEL;
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
						ferr << E_OS << "failed to waitfor (unlock mutex)";

					//	report timeout
					return E_SYNC_TIMEOUT;
				}
			}

			//	unset signal
			state = false;

			//	release exclusive access to state
			if (pthread_mutex_unlock(&mutex)) ferr << E_OS << "failed to waitfor (unlock mutex)";

			//	ok
			return C_OK;

		#endif

		}



////////////////	TIMER

		/*	DOCUMENTATION: TIMER

			QueryPerformanceCounter() actually takes ages to call for some reason. A much quicker
			call is provided by the inline assembler in the function RDTSC(), dropping the overhead
			of timing processing. I hear that RDTSC() is Pentium (or late Athlon) specific, so it
			may not work on some platforms, but in any case it all goes wrong when running on multiple
			cores, because their RDTSC clocks do not agree. So we don't use it anymore. See version
			0.7.2 for the original code base.

			See http://en.wikipedia.org/wiki/RDTSC for basic information.
		*/

		//	statics shared between all instances of the timer class
		double ticksPerSec = 0.0;

	#ifdef __WIN__

		inline INT64 qpc()
		{
			LARGE_INTEGER li;
			QueryPerformanceCounter(&li);
			INT64 ret = li.HighPart;
			ret <<= 32;
			ret += li.LowPart;
			return ret;
		}

		inline INT64 qpf()
		{
			LARGE_INTEGER li;
			QueryPerformanceFrequency(&li);
			INT64 ret = li.HighPart;
			ret <<= 32;
			ret += li.LowPart;
			return ret;
		}

		inline INT64 count()
		{
			return qpc();
		}

	#endif

	#ifdef __NIX__

		#include <sys/time.h>

		inline INT64 count()
		{
			struct timeval tv;
			gettimeofday(&tv, NULL);
			INT64 ret = tv.tv_sec;
			ret *= 1000000;
			ret += tv.tv_usec;
			return ret;
		}

	#endif

		void initialiseTimers()
		{
			//	if done, skip
			if (ticksPerSec) return;

	#ifdef __WIN__
			//	ensure we've got frequency
			if (!ticksPerSec) ticksPerSec = (double)qpf();
	#else
			//	ensure we've got frequency (microseconds for this implementation)
			if (!ticksPerSec) ticksPerSec = 1000000.0;
	#endif

		}

		Timer::Timer()
		{
			//	init timers
			initialiseTimers();

			//	get start time
			t_start = count();
		}

		double Timer::reset()
		{
			if (!ticksPerSec) return 0.0;
			INT64 t_stop = count();
			double result = ((double)(t_stop-t_start)) / ticksPerSec;
			t_start = t_stop;
			return result;
		}

		double Timer::elapsed() const
		{
			if (!ticksPerSec) return 0.0;
			INT64 t_stop = count();
			double result = ((double)(t_stop-t_start)) / ticksPerSec;
			return result;
		}

		UINT32 Timer::elapsedMS() const
		{
			double t = elapsed();
			return (UINT32)(t * 1000);
		}

		double Timer::getTicksPerSec() const
		{
			return ticksPerSec;
		}

		Timer::~Timer()
		{
			//	no action
		}



////////////////	MSLEEP

		void msleep(UINT32 msec)
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
}




