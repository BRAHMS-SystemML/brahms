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

	$Id:: brahms-c++-legacy.h 2443 2009-12-16 18:17:52Z benjmi#$
	$Rev:: 2443                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-12-16 18:17:52 +0000 (Wed, 16 Dec 2009)       $
________________________________________________________________

*/


/*

	Legacy material from data and util interfaces in Standard Library

*/



#ifndef BRAHMS_NO_LEGACY_SUPPORT

#define TYPE_DATA_MASK (TYPE_ELEMENT_MASK | TYPE_COMPLEX_MASK)

////////////////	CONSTANTS

const Symbol C_STILL_ACTIVE = ENGINE_EVENT_STILL_ACTIVE;

////////////////	DATA/NUMERIC

namespace std_2009_data_numeric_0
{

	Structure* get_structure(brahms::Symbol hCaller, brahms::Symbol hData)
	{
		//	handled event
		brahms::HandledEvent hev;
		hev.handler = 0;
		hev.event.type = EVENT_DATA_STRUCTURE_GET;
		hev.event.flags = 0;
		hev.event.object = 0;
		hev.event.data = 0;

		//	use legacy interface
		EngineEvent event;
		event.hCaller = hData;
		event.flags = 0;
		event.type = ENGINE_EVENT_FIRE_EVENT_ON_DATA;
		event.data = (void*) &hev;
		Symbol result = brahms_engineEvent(&event);
		if(S_ERROR(result)) throw result;

		//	structure
		return (Structure*) hev.event.data;
	}

	void set_structure(brahms::Symbol hCaller, brahms::Symbol hData, brahms::TYPE type, const brahms::Dimensions& dims)
	{
		//	structure
		Structure structure;
		structure.type = type;
		structure.dims = dims;

		//	handled event
		brahms::HandledEvent hev;
		hev.handler = 0;
		hev.event.type = EVENT_DATA_STRUCTURE_SET;
		hev.event.flags = 0;
		hev.event.object = 0;
		hev.event.data = (void*)&structure;

		//	use legacy interface
		EngineEvent event;
		event.hCaller = hData;
		event.flags = 0;
		event.type = ENGINE_EVENT_FIRE_EVENT_ON_DATA;
		event.data = (void*) &hev;
		Symbol result = brahms_engineEvent(&event);
		if(S_ERROR(result)) throw result;
	}

	void set_structure(brahms::Symbol hCaller, brahms::Symbol hData, const Structure& structure)
	{
		//	handled event
		brahms::HandledEvent hev;
		hev.handler = 0;
		hev.event.type = EVENT_DATA_STRUCTURE_SET;
		hev.event.flags = 0;
		hev.event.object = 0;
		hev.event.data = (void*)&structure;

		//	use legacy interface
		EngineEvent event;
		event.hCaller = hData;
		event.flags = 0;
		event.type = ENGINE_EVENT_FIRE_EVENT_ON_DATA;
		event.data = (void*) &hev;
		Symbol result = brahms_engineEvent(&event);
		if(S_ERROR(result)) throw result;
	}

	void validate(brahms::Symbol hCaller, brahms::Symbol hData, brahms::TYPE type)
	{
		//	structure
		Structure structure;
		structure.type = type;

		//	handled event
		brahms::HandledEvent hev;
		hev.handler = 0;
		hev.event.type = EVENT_DATA_VALIDATE_TYPE;
		hev.event.flags = 0;
		hev.event.object = 0;
		hev.event.data = (void*)&structure;

		//	use legacy interface
		EngineEvent event;
		event.hCaller = hData;
		event.flags = 0;
		event.type = ENGINE_EVENT_FIRE_EVENT_ON_DATA;
		event.data = (void*) &hev;
		Symbol result = brahms_engineEvent(&event);
		if(S_ERROR(result)) throw result;
	}

	void validate(brahms::Symbol hCaller, brahms::Symbol hData, brahms::TYPE type, const brahms::Dimensions& dims)
	{
		//	structure
		Structure structure;
		structure.type = type;
		structure.dims = dims;

		//	handled event
		brahms::HandledEvent hev;
		hev.handler = 0;
		hev.event.type = EVENT_DATA_VALIDATE_STRUCTURE;
		hev.event.flags = 0;
		hev.event.object = 0;
		hev.event.data = (void*)&structure;

		//	use legacy interface
		EngineEvent event;
		event.hCaller = hData;
		event.flags = 0;
		event.type = ENGINE_EVENT_FIRE_EVENT_ON_DATA;
		event.data = (void*) &hev;
		Symbol result = brahms_engineEvent(&event);
		if(S_ERROR(result)) throw result;
	}

	void validate(brahms::Symbol hCaller, brahms::Symbol hData, const Structure& structure)
	{
		//	handled event
		brahms::HandledEvent hev;
		hev.handler = 0;
		hev.event.type = EVENT_DATA_VALIDATE_STRUCTURE;
		hev.event.flags = 0;
		hev.event.object = 0;
		hev.event.data = (void*)&structure;

		//	use legacy interface
		EngineEvent event;
		event.hCaller = hData;
		event.flags = 0;
		event.type = ENGINE_EVENT_FIRE_EVENT_ON_DATA;
		event.data = (void*) &hev;
		Symbol result = brahms_engineEvent(&event);
		if(S_ERROR(result)) throw result;
	}

	UINT64 get_content(brahms::Symbol hCaller, brahms::Symbol hData, const void*& real, const void*& imag)
	{
		//	handled event
		brahms::HandledEvent hev;
		hev.handler = 0;
		hev.event.type = EVENT_DATA_CONTENT_GET;
		hev.event.flags = 0;
		hev.event.object = 0;
		hev.event.data = 0;

		//	use legacy interface
		EngineEvent event;
		event.hCaller = hData;
		event.flags = 0;
		event.type = ENGINE_EVENT_FIRE_EVENT_ON_DATA;
		event.data = (void*) &hev;
		Symbol result = brahms_engineEvent(&event);
		if(S_ERROR(result)) throw result;

		//	content
		Content* eac = (Content*) hev.event.data;
		real = eac->real;
		imag = eac->imag;
		return eac->bytes;
	}

	const void* get_content(brahms::Symbol hCaller, brahms::Symbol hData)
	{
		//	handled event
		brahms::HandledEvent hev;
		hev.handler = 0;
		hev.event.type = EVENT_DATA_CONTENT_GET;
		hev.event.flags = 0;
		hev.event.object = 0;
		hev.event.data = 0;

		//	use legacy interface
		EngineEvent event;
		event.hCaller = hData;
		event.flags = 0;
		event.type = ENGINE_EVENT_FIRE_EVENT_ON_DATA;
		event.data = (void*) &hev;
		Symbol result = brahms_engineEvent(&event);
		if(S_ERROR(result)) throw result;

		//	content
		Content* eac = (Content*) hev.event.data;
		return (const void*) eac->real;
	}

	void set_content(brahms::Symbol hCaller, brahms::Symbol hData, const void* real, const void* imag = 0, UINT32 bytes = 0)
	{
		//	content
		Content eac;
		eac.real = (void*)real;
		eac.imag = (void*)imag;
		eac.bytes = bytes;

		//	handled event
		brahms::HandledEvent hev;
		hev.handler = 0;
		hev.event.type = EVENT_DATA_CONTENT_SET;
		hev.event.flags = 0;
		hev.event.object = 0;
		hev.event.data = &eac;

		//	use legacy interface
		EngineEvent event;
		event.hCaller = hData;
		event.flags = 0;
		event.type = ENGINE_EVENT_FIRE_EVENT_ON_DATA;
		event.data = (void*) &hev;
		Symbol result = brahms_engineEvent(&event);
		if(S_ERROR(result)) throw result;
	}

}



////////////////	DIRECT INTERFACE

namespace std_2009_data_spikes_0
{

	brahms::Dimensions* get_dimensions(brahms::Symbol hCaller, brahms::Symbol hData)
	{
		//	handled event
		brahms::HandledEvent hev;
		hev.handler = 0;
		hev.event.type = EVENT_DATA_STRUCTURE_GET;
		hev.event.flags = 0;
		hev.event.object = 0;
		hev.event.data = 0;

		//	use legacy interface
		EngineEvent event;
		event.hCaller = hData;
		event.flags = 0;
		event.type = ENGINE_EVENT_FIRE_EVENT_ON_DATA;
		event.data = (void*) &hev;
		Symbol result = brahms_engineEvent(&event);
		if(S_ERROR(result)) throw result;

		//	dimensions
		return (brahms::Dimensions*) hev.event.data;
	}

	void set_dimensions(brahms::Symbol hCaller, brahms::Symbol hData, brahms::Dimensions* dims)
	{
		//	handled event
		brahms::HandledEvent hev;
		hev.handler = 0;
		hev.event.type = EVENT_DATA_STRUCTURE_SET;
		hev.event.flags = 0;
		hev.event.object = 0;
		hev.event.data = (void*)dims;

		//	use legacy interface
		EngineEvent event;
		event.hCaller = hData;
		event.flags = 0;
		event.type = ENGINE_EVENT_FIRE_EVENT_ON_DATA;
		event.data = (void*) &hev;
		Symbol result = brahms_engineEvent(&event);
		if(S_ERROR(result)) throw result;
	}

	UINT32 get_capacity(brahms::Symbol hCaller, brahms::Symbol hData)
	{
		//	handled event
		brahms::HandledEvent hev;
		hev.handler = 0;
		hev.event.type = EVENT_DATA_STRUCTURE_GET;
		hev.event.flags = 0;
		hev.event.object = 0;
		hev.event.data = 0;

		//	use legacy interface
		EngineEvent event;
		event.hCaller = hData;
		event.flags = 0;
		event.type = ENGINE_EVENT_FIRE_EVENT_ON_DATA;
		event.data = (void*) &hev;
		Symbol result = brahms_engineEvent(&event);
		if(S_ERROR(result)) throw result;

		//	dimensions
		brahms::Dimensions dims = *((brahms::Dimensions*) hev.event.data);
		if (dims.count == 0) return 0;
		if (dims.count == 1) return dims.dims[0];
		UINT32 capacity = 1;
		for (UINT32 d=0; d<dims.count; d++) capacity *= dims.dims[d];
		return capacity;
	}

	void set_capacity(brahms::Symbol hCaller, brahms::Symbol hData, UINT32 capacity)
	{
		//	dimensions
		INT64 dim0 = capacity;
		brahms::Dimensions dims = {&dim0, 1};

		//	handled event
		brahms::HandledEvent hev;
		hev.handler = 0;
		hev.event.type = EVENT_DATA_STRUCTURE_SET;
		hev.event.flags = 0;
		hev.event.object = 0;
		hev.event.data = (void*)&dims;

		//	use legacy interface
		EngineEvent event;
		event.hCaller = hData;
		event.flags = 0;
		event.type = ENGINE_EVENT_FIRE_EVENT_ON_DATA;
		event.data = (void*) &hev;
		Symbol result = brahms_engineEvent(&event);
		if(S_ERROR(result)) throw result;
	}

	UINT32 get_content(brahms::Symbol hCaller, brahms::Symbol hData, INT32*& state)
	{
		//	handled event
		brahms::HandledEvent hev;
		hev.handler = 0;
		hev.event.type = EVENT_DATA_CONTENT_GET;
		hev.event.flags = 0;
		hev.event.object = 0;
		hev.event.data = 0;

		//	use legacy interface
		EngineEvent event;
		event.hCaller = hData;
		event.flags = 0;
		event.type = ENGINE_EVENT_FIRE_EVENT_ON_DATA;
		event.data = (void*) &hev;
		Symbol result = brahms_engineEvent(&event);
		if(S_ERROR(result)) throw result;

		//	content
		Spikes* spikes = (Spikes*) hev.event.data;
		state = spikes->spikes;
		return spikes->count;
	}

	void set_content(brahms::Symbol hCaller, brahms::Symbol hData, INT32* state, UINT32 count)
	{
		//	content
		Spikes spikes;
		spikes.spikes = state;
		spikes.count = count;

		//	handled event
		brahms::HandledEvent hev;
		hev.handler = 0;
		hev.event.type = EVENT_DATA_CONTENT_SET;
		hev.event.flags = 0;
		hev.event.object = 0;
		hev.event.data = &spikes;

		//	use legacy interface
		EngineEvent event;
		event.hCaller = hData;
		event.flags = 0;
		event.type = ENGINE_EVENT_FIRE_EVENT_ON_DATA;
		event.data = (void*) &hev;
		Symbol result = brahms_engineEvent(&event);
		if(S_ERROR(result)) throw result;
	}

}



////////////////	DIRECT INTERFACE

namespace std_2009_util_rng_0
{

	inline Symbol brahms_getHandledEventForUtility(Symbol hCaller, struct EventCreateUtility* data)
	{
		struct EngineEvent event;
		event.hCaller = hCaller;
		event.flags = 0;
		event.type = ENGINE_EVENT_HANDLE_UTILITY_EVENT;
		event.data = (void*) data;
		return brahms_engineEvent(&event);
	}

	void select(brahms::Symbol hCaller, brahms::Symbol hUtility, const char* generator)
	{
		//	handled event
		brahms::HandledEvent hev;
		hev.handler = 0;
		hev.event.type = EVENT_UTILITY_SELECT;
		hev.event.flags = 0;
		hev.event.object = 0;
		hev.event.data = (void*)generator;

		//	prepare handled event
		EventCreateUtility data;
		data.flags = 0;
		data.hUtility = hUtility;
		data.spec.cls = NULL;
		data.spec.release = 0;
		data.name = 0;
		data.handledEvent = &hev;
		brahms::Symbol result = brahms_getHandledEventForUtility(hCaller, &data);
		if(S_ERROR(result)) throw result;

		//	fire event
		std_2009_util_rng_0::fireHandledEvent(&hev);
	}

	void seed(brahms::Symbol hCaller, brahms::Symbol hUtility, const UINT32* seed, UINT32 count)
	{
		//	seed
		Seed cseed;
		cseed.count = count;
		cseed.seed = seed;

		//	handled event
		brahms::HandledEvent hev;
		hev.handler = 0;
		hev.event.type = EVENT_UTILITY_SEED;
		hev.event.flags = 0;
		hev.event.object = 0;
		hev.event.data = (void*)&cseed;

		//	prepare handled event
		EventCreateUtility data;
		data.flags = 0;
		data.hUtility = hUtility;
		data.spec.cls = NULL;
		data.spec.release = 0;
		data.name = 0;
		data.handledEvent = &hev;
		brahms::Symbol result = brahms_getHandledEventForUtility(hCaller, &data);
		if(S_ERROR(result)) throw result;

		//	fire event
		std_2009_util_rng_0::fireHandledEvent(&hev);
	}

	void fill(brahms::Symbol hCaller, brahms::Symbol hUtility, DOUBLE* dst, UINT64 count, DOUBLE gain = 1.0, DOUBLE offset = 0.0)
	{
		//	fill
		Fill fill;
		fill.dst = dst;
		fill.count = count;
		fill.type = TYPE_FLOAT64;
		fill.gain = gain;
		fill.offset = offset;

		//	handled event
		brahms::HandledEvent hev;
		hev.handler = 0;
		hev.event.type = EVENT_UTILITY_FILL;
		hev.event.flags = 0;
		hev.event.object = 0;
		hev.event.data = (void*)&fill;

		//	prepare handled event
		EventCreateUtility data;
		data.flags = 0;
		data.hUtility = hUtility;
		data.spec.cls = NULL;
		data.spec.release = 0;
		data.name = 0;
		data.handledEvent = &hev;
		brahms::Symbol result = brahms_getHandledEventForUtility(hCaller, &data);
		if(S_ERROR(result)) throw result;

		//	fire event
		std_2009_util_rng_0::fireHandledEvent(&hev);
	}

	void fill(brahms::Symbol hCaller, brahms::Symbol hUtility, FLOAT32* dst, UINT64 count, FLOAT32 gain = 1.0, FLOAT32 offset = 0.0)
	{
		//	fill
		Fill fill;
		fill.dst = dst;
		fill.count = count;
		fill.type = TYPE_FLOAT32;
		fill.gain = gain;
		fill.offset = offset;

		//	handled event
		brahms::HandledEvent hev;
		hev.handler = 0;
		hev.event.type = EVENT_UTILITY_FILL;
		hev.event.flags = 0;
		hev.event.object = 0;
		hev.event.data = (void*)&fill;

		//	prepare handled event
		EventCreateUtility data;
		data.flags = 0;
		data.hUtility = hUtility;
		data.spec.cls = NULL;
		data.spec.release = 0;
		data.name = 0;
		data.handledEvent = &hev;
		brahms::Symbol result = brahms_getHandledEventForUtility(hCaller, &data);
		if(S_ERROR(result)) throw result;

		//	fire event
		std_2009_util_rng_0::fireHandledEvent(&hev);
	}

	DOUBLE get(brahms::Symbol hCaller, brahms::Symbol hUtility)
	{
		//	handled event
		brahms::HandledEvent hev;
		hev.handler = 0;
		hev.event.type = EVENT_UTILITY_GET;
		hev.event.flags = 0;
		hev.event.object = 0;
		hev.event.data = 0;

		//	prepare handled event
		EventCreateUtility data;
		data.flags = 0;
		data.hUtility = hUtility;
		data.spec.cls = NULL;
		data.spec.release = 0;
		data.name = 0;
		data.handledEvent = &hev;
		brahms::Symbol result = brahms_getHandledEventForUtility(hCaller, &data);
		if(S_ERROR(result)) throw result;

		//	fire event
		std_2009_util_rng_0::fireHandledEvent(&hev);

		//	get
		return *((DOUBLE*)hev.event.data);
	}

}

#endif


