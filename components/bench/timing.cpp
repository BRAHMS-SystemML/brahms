
#ifdef __WIN__

#include "windows.h"

#endif

//	copied from brahms-component.h and helper-timing.{h,cpp}

#ifdef __GNUC__
#define __int8 char
#if ARCH_BITS == 64
#define __int64 long int
#else
#define __int64 long long int
#endif
#endif

typedef unsigned __int8 BYTE;
typedef signed __int64 INT64;

class Timer
{
public:
	Timer();
	double reset();
	double elapsed();

private:
	bool valid;
	INT64 t_start;
};

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
		gettimeofday(&tv, 0);
		INT64 ret = tv.tv_sec;
		ret *= 1000000;
		ret += tv.tv_usec;
		return ret;
	}

#endif

#ifdef __WIN__

	Timer::Timer()
	{
		if (!ticksPerSec) ticksPerSec = (double)qpf();
		valid = true;
		t_start = qpc();
	}

#endif

#ifdef __NIX__

	#include <sys/time.h>

	Timer::Timer()
	{
		if (!ticksPerSec) ticksPerSec = 1000000.0;
		valid = true;
		t_start = count();
	}

#endif

double Timer::reset()
{
	if (!valid) return 0.0;
	INT64 t_stop = count();
	double result = ((double)(t_stop-t_start)) / ticksPerSec;
	t_start = t_stop;
	return result;
}

double Timer::elapsed()
{
	if (!valid) return 0.0;
	INT64 t_stop = count();
	double result = ((double)(t_stop-t_start)) / ticksPerSec;
	return result;
}
