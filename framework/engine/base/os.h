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

	$Id:: os.h 2283 2009-11-02 00:22:31Z benjmitch             $
	$Rev:: 2283                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-02 00:22:31 +0000 (Mon, 02 Nov 2009)       $
________________________________________________________________

*/



#ifndef INCLUDED_BRAHMS_FRAMEWORK_BASE_OS
#define INCLUDED_BRAHMS_FRAMEWORK_BASE_OS



#ifdef __WIN__
#define _WIN32_IE 0x0500
#define _WIN32_WINNT 0x0500
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __NIX__
#include <pthread.h>
#endif

////////////////	NAMESPACE

namespace brahms
{
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

		const UINT32 SIGNAL_INFINITE_WAIT = 0;
		const UINT32 SIGNAL_WAIT_STEP = 100;

		struct Signal
		{
			//	initial state is "unset"
			Signal(UINT32 timeout, const bool* cancel);
			Signal(const Signal& src);
			~Signal() throw();

			//	interface
			void set();
			Symbol waitfor();

		private:

			//	parameters
			UINT32 timeout;
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



	////////////////	TIMER

		class Timer
		{

		public:

			Timer();
			~Timer();

			DOUBLE			reset();					//	return elapsed time, and reset timer
			DOUBLE			elapsed() const;			//	return elapsed time
			UINT32			elapsedMS() const;			//	return elapsed time in milliseconds (calls elapsed())
			DOUBLE			getTicksPerSec() const;		//	get number of ticks per second (resolution)

		private:

			INT64			t_start;					//	time timer was reset

		};



	////////////////	FUNCTIONS

		void msleep(UINT32 msec);



	}
}



////////////////	INCLUSION GUARD

#endif
