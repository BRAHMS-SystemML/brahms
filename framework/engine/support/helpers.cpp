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

	$Id:: helpers.cpp 2278 2009-11-01 21:23:08Z benjmitch      $
	$Rev:: 2278                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-01 21:23:08 +0000 (Sun, 01 Nov 2009)       $
________________________________________________________________

*/



#include "support.h"



////////////////	SUPPORT FOR RDTSC

//	RDTSC instruction for unpredictable value
inline INT64 rdtsc()
{
#ifdef __WIN__
	__asm
	{
		_emit 0x0F
		_emit 0x31
	}
#endif

#ifdef __NIX__
	INT64 x;
	__asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
	return x;
#endif
}

#define FLIP(a, b) {BYTE c=a; a=b; b=c;}

inline UINT32 rdtsc_seed()
{
	UINT32 ret = 0;
	while(!ret) ret = rdtsc(); // avoid zero values
	
	//	flip bytes so that values are widely distributed across possible range, rather than localised depending on the current time
	BYTE* b = (BYTE*) &ret;
	FLIP(b[0], b[3]);
	FLIP(b[1], b[2]);

	return ret;
}



////////////////	NAMESPACE

namespace brahms
{
	namespace local
	{
		void Seed::set(const VUINT32& value)
		{
			//	validate
			if (value.size() > 1)
			{
				//	must not have zero entries
				for (UINT32 v=0; v<value.size(); v++)
				{
					if (value[v] == 0)
						throw E_SEED_INVALID;
				}
			}

			//	store
			this->value = value;
		}

		UINT32 Seed::available()
		{
			if (value.size() == 1 && value[0] == 0) return 0; // indefinite
			return value.size();
		}

		VUINT32 Seed::get(UINT32 count)
		{
			//ferr << E_INTERNAL << "oops";

			//	if empty
			if (!value.size())
			{
				//	treat as scalar zero
				VUINT32 ret;
				for (UINT32 c=0; c<count; c++)
					ret.push_back(rdtsc_seed());
				return ret;
			}

			//	else
			else
			{
				//	if zero
				if (value.size() == 1 && value[0] == 0)
				{
					//	scalar zero
					VUINT32 ret;
					for (UINT32 c=0; c<count; c++)
						ret.push_back(rdtsc_seed());
					return ret;
				}

				//	if not
				else
				{
					//	return value
					VUINT32 ret = value;

					//	increment
					value[0]++;

					//	check for overflow
					if (value[0] == 0)
						throw E_SEED_OVERFLOW;

					//	pad
					while(ret.size() < count)
						ret.push_back(0xAAAAAAAA);

					//	ok
					return ret;
				}
			}
		}

	////////////////	BASE RATE

		//	Least common multiple of a vector of fractions
		//	by Tak-Shing
		//
		//	By the associative law for LCM, we can calculate the LCM
		//	of n fractions iteratively:
		//	LCM(r[0], r[1], ...) = LCM(LCM(LCM(r[0], r[1]), r[2])...
		//
		//	See http://mathworld.wolfram.com/LeastCommonMultiple.html

		SampleRate BaseRate::findBaseRate(vector<SampleRate>& sampleRates)
		{
			//	extract
			UINT64 N = sampleRates.size();
			if (!N)
			{
				//	this means no processes in system, so BSR is irrelevant (we set it to 1Hz)
				baseSampleRate.num = 1;
				baseSampleRate.den = 1;
				return baseSampleRate;
			}

			//	convert all input sample rates to lowest terms
			//	done already: see note in baserate.h

			//	start with first rate
			baseSampleRate = sampleRates[0];

			//	do LCM search sequentially
			for (UINT32 i=0; i<N; i++)
			{
				//	we do zero as well, just so we can check the sample rates are valid...
				if (sampleRates[i].num == 0 || sampleRates[i].den == 0)
					ferr << E_INTERNAL << "sample rate " << i << " invalid (" << sampleRates[i].num << "/" << sampleRates[i].den << ")";

				//	mathworld: lcm(a, b) = a * b / gcd (a, b)
				//	we do this to the numerators in an order that minimizes chance of overflow

				//	this operation is always safe
				//	always safe to divide by its common divisor, must have an integral
				//	answer, and must be less than current baseSampleRate.num
				baseSampleRate.num /= brahms::math::gcd(baseSampleRate.num, sampleRates[i].num);

				//	this operation could overflow
				//	if it does, the corresponding division will give the wrong answer
				UINT64 result = baseSampleRate.num * sampleRates[i].num;
				if ((result / baseSampleRate.num) != sampleRates[i].num)
					ferr << E_UNREPRESENTABLE << "base sample rate too large";

				//	ok, no overflow
				baseSampleRate.num = result;

				//	this operation is always safe
				baseSampleRate.den = brahms::math::gcd(baseSampleRate.den, sampleRates[i].den);

				//	given the above, baseSampleRate is now still in lowest terms
				//	so we can just continue
			}

			//	ok
			return baseSampleRate;
		}

		//	other stuff, by mitch

		void BaseRate::setSystemTime(ComponentTime* stime, DOUBLE executionStop)
		{
			/*

				Execution stop time is specified as a float, but must be resolved to
				a base sample boundary. Execution is supposed to continue until the
				stop time is reached - that is, if the fS is 5Hz, and the stop time
				is 0.6, we should service three boundaries: t=0, 0.2 and 0.4, but *not*
				0.6. Here, we convert stop time from a float to a base sample number.
				Note, though, that naively, 0.7==>3, 0.61==>3, and 0.6==>3. In fact, the
				former two indicate that we *should* service the t=0.6 boundary, whereas
				the last indicates that we shouldn't. So we do a floor() conversion, but
				then if the stop time is above the back-conversion (e.g. 0.61 > 0.6) we
				increment the stop sample by one so that we actually *do* service that
				"0.6" boundary.

				In other words, when we're done, the stime->executionStop holds the first
				service boundary that we will *not* service.

			*/

			stime->baseSampleRate = baseSampleRate;
			DOUBLE stopInBaseSamples = floor(executionStop * sampleRateToRate(baseSampleRate));
			if (stopInBaseSamples > TEN_TO_THE_FIFTEEN)
				ferr << E_UNREPRESENTABLE << "execution stop time too large";

			//	we want to increment the integer stop time if the back conversion
			//	is not quite as large as the execution stop time - but, there will
			//	be floating-point inaccuracy, so we want to allow it to be some fraction
			//	of the sample period smaller without us changing anything... say, one
			//	millionth of the sample period is considered negligible
			DOUBLE backConversion = stopInBaseSamples / sampleRateToRate(baseSampleRate);
			DOUBLE discrepancy = executionStop - backConversion;
			DOUBLE discrepancyAsFractionOfPeriod = discrepancy * sampleRateToRate(baseSampleRate);

			if (discrepancyAsFractionOfPeriod > 1e-6) stopInBaseSamples++;
			stime->executionStop = stopInBaseSamples;
		}
	}
}


