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

	$Id:: data_spikes.h 2436 2009-12-12 19:14:48Z benjmitch    $
	$Rev:: 2436                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-12-12 19:14:48 +0000 (Sat, 12 Dec 2009)       $
________________________________________________________________

*/




#ifndef INCLUDED_COMPONENT_____CLASS_US____
#define INCLUDED_COMPONENT_____CLASS_US____





/*

	EVENT_DATA_STRUCTURE_GET (Dimensions* dims)
		On return, "dims" will hold the dimensions of the object.

	EVENT_DATA_STRUCTURE_SET (Dimensions* dims)
		Set the dimensions of the object from "dims".

	EVENT_DATA_CONTENT_GET (Spikes* sp)
		On return, "sp" will hold the content of the object.

	EVENT_DATA_CONTENT_SET (Spikes* sp)
		Set the content of the object from "sp".

*/






#ifndef __cplusplus

/*

	C Interface

*/



//	Structure

struct ____CLASS_US_____Spikes
{
	INT32* spikes;
	UINT32 count;
	UINT32 flags;
};



//	Event Types

#define ____CLASS_US_____EVENT_DATA_STRUCTURE_GET (C_BASE_USER_EVENT + 1)
#define ____CLASS_US_____EVENT_DATA_STRUCTURE_SET (C_BASE_USER_EVENT + 2)
#define ____CLASS_US_____EVENT_DATA_CONTENT_GET (C_BASE_USER_EVENT + 3)
#define ____CLASS_US_____EVENT_DATA_CONTENT_SET (C_BASE_USER_EVENT + 4)




#else

/*

	C++ Interface

*/

//	need string class for some API calls
#include <string>



namespace ____CLASS_US____
{

	struct Spikes
	{
		INT32* spikes;
		UINT32 count;
		UINT32 flags;
	};

	const brahms::Symbol EVENT_DATA_STRUCTURE_GET = C_BASE_USER_EVENT + 1;
	const brahms::Symbol EVENT_DATA_STRUCTURE_SET = C_BASE_USER_EVENT + 2;
	const brahms::Symbol EVENT_DATA_CONTENT_GET = C_BASE_USER_EVENT + 3;
	const brahms::Symbol EVENT_DATA_CONTENT_SET = C_BASE_USER_EVENT + 4;

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



////////////////	INDIRECT INTERFACE (SHARED)

	struct Accessor
	{

	protected:

		brahms::Symbol hComponent;
		brahms::Symbol hSet;
		brahms::Symbol hPort;
		brahms::HandledEvent hev;
		std::string m_portName;
		brahms::SampleRate sampleRate;
		bool m_seen;

	public:

		//	user tag
		UINT32 tag;

		Accessor()
		{
			hComponent = S_NULL;
			hSet = S_NULL;
			hPort = S_NULL;
			hev.handler = 0;
			hev.event.type = EVENT_NULL;
			hev.event.flags = 0;
			hev.event.object = 0;
			hev.event.data = 0;
			sampleRate.num = 0;
			sampleRate.den = 0;
			m_seen = false;
			tag = 0;
		}

		void copy(const Accessor& rhs)
		{
			hComponent = rhs.hComponent;
			hSet = rhs.hSet;
			hPort = rhs.hPort;
			hev = rhs.hev;
			m_portName = rhs.m_portName;
			sampleRate = rhs.sampleRate;
			m_seen = rhs.m_seen;
			tag = rhs.tag;
		}

////////////////	UPDATE ALL THE ABOVE TOGETHER (FIELDS, CONSTRUCT, COPY)

		Accessor(const Accessor& rhs)
		{
			if (rhs.hPort)
				throw "copy ctor cannot be called after create() or attach() (HandledEvent's would be lost)";
			copy(rhs);
		}

		Accessor& operator=(const Accessor& rhs)
		{
			if (this != &rhs)
			{
				if (rhs.hPort)
					throw "operator= cannot be called after create() or attach() (HandledEvent's would be lost)";
				copy(rhs);
			}

			return *this;
		}

		brahms::Symbol getPort() const
		{
			return hPort;
		}

		void fire()
		{
			//	fire event
			____CLASS_US____::fireHandledEvent(&hev);
		}

		void check() const
		{
			//	check
			if (!hPort) raiseError(E_INTERFACE_MISUSE, "initialise Accessor using create() or attach() before using other functions");
		}

		void checknot() const
		{
			//	check not
			if (hPort) raiseError(E_INTERFACE_MISUSE, "initialise Accessor using create() or attach() only once");
		}

		bool due() const
		{
			return hev.event.object != NULL;
		}

		bool seen(bool seenNow = false)
		{
			if (seenNow) m_seen = true;
			return m_seen;
		}

		const char* getName() const
		{
			check();

			//	get info
			brahms::EventGetPortInfo info;
			info.hPort = hPort;
			info.flags = 0;
			info.name = NULL;
			info.componentInfo = NULL;

			brahms::EngineEvent event;
			event.hCaller = hComponent;
			event.flags = 0;
			event.type = ENGINE_EVENT_GET_PORT_INFO;
			event.data = (void*) &info;

			brahms::Symbol result = brahms::brahms_engineEvent(&event);
			if (S_ERROR(result)) throw result;
			return info.name;
		}

		UINT32 getFlags() const
		{
			check();

			//	get info
			brahms::EventGetPortInfo info;
			info.hPort = hPort;
			info.flags = 0;
			info.name = NULL;
			info.componentInfo = NULL;

			brahms::EngineEvent event;
			event.hCaller = hComponent;
			event.flags = 0;
			event.type = ENGINE_EVENT_GET_PORT_INFO;
			event.data = (void*) &info;

			brahms::Symbol result = brahms::brahms_engineEvent(&event);
			if (S_ERROR(result)) throw result;
			return info.flags;
		}

		void selectSet(brahms::Symbol hSet)
		{
			//	check
			if (hPort) raiseError(E_INTERFACE_MISUSE, "must selectSet() before create() or attach()");

			//	store
			this->hSet = hSet;
		}

////////////////	GET STRUCTURE

		UINT32 getCapacity()
		{
			check();

			//	handled event
			hev.event.type = EVENT_DATA_STRUCTURE_GET;
			hev.event.data = 0;

			//	fire
			fire();

			//	mark
			m_seen = true;

			//	dimensions
			brahms::Dimensions dims = *((brahms::Dimensions*) hev.event.data);
			if (dims.count == 0) return 0;
			if (dims.count == 1) return dims.dims[0];
			UINT32 capacity = 1;
			for (UINT32 d=0; d<dims.count; d++) capacity *= dims.dims[d];
			return capacity;
		}

		brahms::Dimensions* getDimensions()
		{
			check();

			//	handled event
			hev.event.type = EVENT_DATA_STRUCTURE_GET;
			hev.event.data = 0;

			//	fire
			fire();

			//	mark
			m_seen = true;

			//	dimensions
			return (brahms::Dimensions*) hev.event.data;
		}
	};

////////////////	INDIRECT INTERFACE (OUTPUT)

	struct Output : public Accessor
	{

////////////////	CREATE

		void create(brahms::Symbol hComponent)
		{
			checknot();

			//	store
			this->hComponent = hComponent;

			//	add port
			brahms::EventAddPort apdata;
			apdata.index = INDEX_UNDEFINED;
			apdata.hSet = hSet ? hSet : hComponent; // hComponent means use the default set
			apdata.flags = 0;
			apdata.spec.cls = "std/2009/data/spikes";
			apdata.spec.release = 0;
			apdata.hPortToCopy = S_NULL;
			apdata.name = m_portName.length() ? m_portName.c_str() : NULL;
			apdata.transportProtocol = C_TRANSPORT_PERIODIC;
			brahms::TransportDataPeriodic periodic;
			periodic.sampleRate = sampleRate;
			apdata.transportData = (void*) &periodic;
			apdata.handledEvent = &hev;

			brahms::EngineEvent event;
			event.hCaller = hComponent;
			event.flags = 0;
			event.type = ENGINE_EVENT_ADD_PORT;
			event.data = &apdata;

			hPort = brahms::brahms_engineEvent(&event);
			if (S_ERROR(hPort)) throw hPort;
		}

		void create(brahms::Symbol hComponent, const Accessor& accessor)
		{
			checknot();

			//	store
			this->hComponent = hComponent;

			//	add port
			brahms::EventAddPort apdata;
			apdata.index = INDEX_UNDEFINED;
			apdata.hSet = hSet ? hSet : hComponent; // hComponent means use the default set
			apdata.flags = 0;
			apdata.spec.cls = NULL;
			apdata.spec.release = 0;
			apdata.hPortToCopy = accessor.getPort();
			apdata.name = m_portName.length() ? m_portName.c_str() : NULL;
			apdata.transportProtocol = C_TRANSPORT_PERIODIC;
			brahms::TransportDataPeriodic periodic;
			periodic.sampleRate = sampleRate;
			apdata.transportData = (void*) &periodic;
			apdata.handledEvent = &hev;

			brahms::EngineEvent event;
			event.hCaller = hComponent;
			event.flags = 0;
			event.type = ENGINE_EVENT_ADD_PORT;
			event.data = &apdata;

			hPort = brahms::brahms_engineEvent(&event);
			if (S_ERROR(hPort)) throw hPort;
		}

////////////////	SET STRUCTURE

		void setName(const char* name)
		{
			//	check
			if (hPort) raiseError(E_INTERFACE_MISUSE, "must setName() before create()");

			//	store
			m_portName = name;
		}

		void setSampleRate(brahms::SampleRate sampleRate)
		{
			//	check
			if (hPort) raiseError(E_INTERFACE_MISUSE, "must setSampleRate() before create()");

			//	store
			this->sampleRate = sampleRate;
		}

		void setCapacity(INT32 capacity)
		{
			check();

			//	data
			INT64 dim0 = capacity;
			brahms::Dimensions dims = {&dim0, 1};

			//	handled event
			hev.event.type = EVENT_DATA_STRUCTURE_SET;
			hev.event.data = (void*)&dims;

			//	fire
			fire();
		}

		void setDimensions(brahms::Dimensions dims)
		{
			check();

			//	handled event
			hev.event.type = EVENT_DATA_STRUCTURE_SET;
			hev.event.data = (void*)&dims;

			//	fire
			fire();
		}

////////////////	SET CONTENT OPTIONS (RUN-PHASE)

		void setContent(INT32* state, UINT32 count)
		{
			//	content
			Spikes spikes = {0};
			spikes.spikes = state;
			spikes.count = count;

			//	handled event
			hev.event.type = EVENT_DATA_CONTENT_SET;
			hev.event.data = &spikes;

			//	fire
			fire();
		}

	};



////////////////	INDIRECT INTERFACE (INPUT)

	struct Input : public Accessor
	{

////////////////	ATTACH

		Input& now()
		{
			//	throw if not due
			if (!due()) throw E_PORT_EMPTY;

			//	return object
			return *this;
		}

		Input& attach(brahms::Symbol hComponent, std::string name)
		{
			//	do pre attach
			attachSub(hComponent, name.c_str(), INDEX_UNDEFINED);

			//	return object
			return *this;
		}

		Input& attach(brahms::Symbol hComponent, UINT32 index)
		{
			//	do pre attach
			attachSub(hComponent, NULL, index);

			//	return object
			return *this;
		}

		void attachSub(brahms::Symbol hComponent, const char* name, UINT32 index)
		{
			//	only once
			checknot();

			//	store
			this->hComponent = hComponent;

			//	get port
			brahms::EventGetPort getPort;
			getPort.hSet = hSet ? hSet : hComponent; // hComponent means use the default set
			getPort.flags = 0;
			getPort.spec.cls = "std/2009/data/spikes";
			getPort.spec.release = 0;
			getPort.handledEvent = &hev;
			getPort.name = name;
			getPort.index = index;
			
			brahms::EngineEvent event;
			event.hCaller = hComponent;
			event.flags = 0;
			event.type = ENGINE_EVENT_GET_PORT;
			event.data = &getPort;
			hPort = brahms::brahms_engineEvent(&event);

			if (S_ERROR(hPort)) throw hPort;
		}

		bool tryAttach(brahms::Symbol hComponent, std::string name)
		{
			//	only once
			checknot();

			//	store
			this->hComponent = hComponent;

			//	get port
			brahms::EventGetPort getPort;
			getPort.hSet = hSet ? hSet : hComponent; // hComponent means use the default set
			getPort.flags = 0;
			getPort.spec.cls = "std/2009/data/spikes";
			getPort.spec.release = 0;
			getPort.handledEvent = &hev;
			getPort.name = name.c_str();
			getPort.index = INDEX_UNDEFINED;
			
			brahms::EngineEvent event;
			event.hCaller = hComponent;
			event.flags = 0;
			event.type = ENGINE_EVENT_GET_PORT;
			event.data = &getPort;
			hPort = brahms::brahms_engineEvent(&event);

			if (S_ERROR(hPort) == E_NOT_FOUND) return false;
			if (S_ERROR(hPort)) throw hPort;

			//	ok
			return true;
		}

////////////////	GET CONTENT OPTIONS (RUN-PHASE)

		UINT32 getContent(INT32*& state)
		{
			//	handled event
			hev.event.type = EVENT_DATA_CONTENT_GET;
			hev.event.data = 0;

			//	fire
			fire();

			//	content
			Spikes* spikes = (Spikes*) hev.event.data;
			state = spikes->spikes;
			return spikes->count;
		}


	};

}





////////////////	C / C++

#endif




////////////////	INCLUSION GUARD

#endif


