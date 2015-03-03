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

	$Id:: util_rng.h 2436 2009-12-12 19:14:48Z benjmitch       $
	$Rev:: 2436                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-12-12 19:14:48 +0000 (Sat, 12 Dec 2009)       $
________________________________________________________________

*/

// The old build scheme would replace ____CLASS_US____ using the
// makefiles, but I prefer to state these here in the code.

#ifndef INCLUDED_COMPONENT_std_2009_util_rng_0
#define INCLUDED_COMPONENT_std_2009_util_rng_0





/*

	EVENT_UTILITY_SELECT (const char* gen)
		"gen" is a string specifying the generator that this object should
		implement. Syntax is "<generator>,<distribution>". Currently supported
		generators are "MT2000" (normal, exponential, uniform).

	EVENT_UTILITY_SEED (Seed* seed)
		"seed" holds the UINT32 vector seed with which to seed the RNG. Note
		that the seed may not completely specify the generator's state. For
		set/get state, see the appropriate events.

	EVENT_UTILITY_FILL (Fill* fill)
		"fill" is a structure specifying what should be filled, and with what
		type of data (FLOAT32, DOUBLE).

	EVENT_UTILITY_GET (DOUBLE* value)
		A pointer to a single value from the RNG is returned in "value". The
		pointer is valid until the next call to the object, and should be
		dereferenced immediately.

*/








#ifndef __cplusplus

/*

	C Interface

*/

//	Structures

struct std_2009_util_rng_0_Seed
{
	const UINT32* seed;
	UINT32 count;
	UINT32 flags;
};

struct std_2009_util_rng_0_Fill
{
	void* dst;
	UINT64 count;
	TYPE type;
	DOUBLE gain;
	DOUBLE offset;
	UINT32 flags;
};



//	Event Types

#define std_2009_util_rng_0_EVENT_UTILITY_SELECT (C_BASE_USER_EVENT + 1)
#define std_2009_util_rng_0_EVENT_UTILITY_SEED (C_BASE_USER_EVENT + 2)
#define std_2009_util_rng_0_EVENT_UTILITY_FILL (C_BASE_USER_EVENT + 3)
#define std_2009_util_rng_0_EVENT_UTILITY_GET (C_BASE_USER_EVENT + 4)





#else

/*

	C++ Interface

*/

//	need string class for some API calls
#include <string>
#include <vector>

namespace std_2009_util_rng_0
{

	struct Seed
	{
		const UINT32* seed;
		UINT32 count;
		UINT32 flags;
	};

	struct Fill
	{
		void* dst;
		UINT64 count;
		brahms::TYPE type;
		DOUBLE gain;
		DOUBLE offset;
		UINT32 flags;
	};

	const brahms::Symbol EVENT_UTILITY_SELECT = C_BASE_USER_EVENT + 1;
	const brahms::Symbol EVENT_UTILITY_SEED = C_BASE_USER_EVENT + 2;
	const brahms::Symbol EVENT_UTILITY_FILL = C_BASE_USER_EVENT + 3;
	const brahms::Symbol EVENT_UTILITY_GET = C_BASE_USER_EVENT + 4;

	void raiseError(brahms::Symbol code, const char* msg)
	{
		brahms::EventErrorMessage eem;
		eem.error = code;
		eem.msg = msg;
		eem.flags = 0;

		brahms::EngineEvent event;
		event.hCaller = 0;
		event.flags = 0;
		event.type = ENGINE_EVENT_ERROR_MESSAGE;
		event.data = &eem;
		throw brahms::brahms_engineEvent(&event);
	}

	brahms::Symbol fireHandledEvent(brahms::HandledEvent* hev)
	{
		if (!hev) raiseError(E_NULL_ARG, "NULL argument in fireHandledEvent()");
		if (!hev->handler) raiseError(E_NULL_ARG, "NULL \"handler\" in fireHandledEvent()");
		if (!hev->event.object) raiseError(E_NULL_ARG, "NULL \"object\" in fireHandledEvent()");

		brahms::Symbol result = hev->handler(&hev->event);
		if (S_ERROR(result)) throw result;
		return result;
	}



////////////////	INDIRECT INTERFACE

	struct Utility
	{

		Utility()
		{
			hUtility = S_NULL;
			hev.handler = 0;
			hev.event.type = EVENT_NULL;
			hev.event.flags = 0;
			hev.event.object = 0;
			hev.event.data = 0;
		}

		void fire()
		{
			//	fire event
			std_2009_util_rng_0::fireHandledEvent(&hev);
		}

		void check()
		{
			//	check
			if (!hUtility) raiseError(E_INTERFACE_MISUSE, "initialise Utility using create() before using other functions");
		}

		brahms::Symbol getUtility()
		{
			check();

			//	get data
			return hUtility;
		}

////////////////	CREATE

		void setName(const char* name)
		{
			//	check
			if (hUtility) raiseError(E_INTERFACE_MISUSE, "must setName() before create()");

			//	store
			m_utilityName = name;
		}

		void create(brahms::Symbol hComponent)
		{
			//	create utility
			brahms::EventCreateUtility data;
			data.flags = 0;
			data.hUtility = S_NULL;
			data.spec.cls = "std/2009/util/rng";
			data.spec.release = 0;
			data.name = m_utilityName.length() ? m_utilityName.c_str() : NULL;
			data.handledEvent = &hev;

			brahms::EngineEvent event;
			event.hCaller = hComponent;
			event.flags = 0;
			event.type = ENGINE_EVENT_CREATE_UTILITY;
			event.data = (void*) &data;

			brahms::Symbol result = brahms_engineEvent(&event);
			if (S_ERROR(result)) throw result;

			hUtility = data.hUtility;
		}

		void select(const char* generator)
		{
			//	handled event
			hev.event.type = EVENT_UTILITY_SELECT;
			hev.event.data = (void*)generator;

			//	fire
			fire();
		}

		void seed(const std::vector<UINT32>& seed)
		{
			//	seed
			Seed cseed = {0};
			cseed.count = seed.size();
			cseed.seed = cseed.count ? &seed[0] : NULL;

			//	handled event
			hev.event.type = EVENT_UTILITY_SEED;
			hev.event.data = (void*)&cseed;

			//	fire
			fire();
		}

		void seed(const UINT32* seed, UINT32 count)
		{
			//	seed
			Seed cseed = {0};
			cseed.count = count;
			cseed.seed = seed;

			//	handled event
			hev.event.type = EVENT_UTILITY_SEED;
			hev.event.data = (void*)&cseed;

			//	fire
			fire();
		}

		void fill(DOUBLE* dst, UINT64 count, DOUBLE gain = 1.0, DOUBLE offset = 0.0)
		{
			//	fill
			Fill fill = {0};
			fill.dst = dst;
			fill.count = count;
			fill.type = TYPE_FLOAT64;
			fill.gain = gain;
			fill.offset = offset;

			//	handled event
			hev.event.type = EVENT_UTILITY_FILL;
			hev.event.data = (void*)&fill;

			//	fire
			fire();
		}

		void fill(FLOAT32* dst, UINT64 count, FLOAT32 gain = 1.0, FLOAT32 offset = 0.0)
		{
			//	fill
			Fill fill = {0};
			fill.dst = dst;
			fill.count = count;
			fill.type = TYPE_FLOAT32;
			fill.gain = gain;
			fill.offset = offset;

			//	handled event
			hev.event.type = EVENT_UTILITY_FILL;
			hev.event.data = (void*)&fill;

			//	fire
			fire();
		}

		DOUBLE get()
		{
			//	handled event
			hev.event.type = EVENT_UTILITY_GET;
			hev.event.data = 0;

			//	fire
			fire();

			//	get
			return *((DOUBLE*)hev.event.data);
		}

////////////////	DATA

		brahms::Symbol hUtility;
		brahms::HandledEvent hev;
		std::string m_utilityName;

	};

}





////////////////	C / C++

#endif






////////////////	INCLUSION GUARD

#endif
