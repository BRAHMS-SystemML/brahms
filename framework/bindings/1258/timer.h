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

	$Id:: timer.h 2419 2009-11-30 18:33:48Z benjmitch          $
	$Rev:: 2419                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-30 18:33:48 +0000 (Mon, 30 Nov 2009)       $
________________________________________________________________

*/



#ifdef __WIN__

	//	includes
	#define _WIN32_IE 0x0500
	#define _WIN32_WINNT 0x0500
	#define WIN32_LEAN_AND_MEAN
	#include "windows.h"

#endif


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




	////////////////	TIMER

		//	statics shared between all instances of the timer class
		double ticksPerSec = 0.0;
		const char* cacheFileName = "brahms-cached-ticks-per-sec";



	#ifdef __WIN__

		//	QueryPerformanceCounter is the backup on windows, if we don't have
		//	the RDTSC instruction, but it's also used to measure the CPU speed,
		//	so it's defined in both builds, using RDTSC or not.

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




	////////////////	BUILD-INDEPENDENT BITS

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
			return ticksPerSec; // mark we're not using RDTSC by the +ve sign
		}

		Timer::~Timer()
		{
			//	no action
		}



