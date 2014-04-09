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

	$Id:: data_numeric.h 2436 2009-12-12 19:14:48Z benjmitch   $
	$Rev:: 2436                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-12-12 19:14:48 +0000 (Sat, 12 Dec 2009)       $
________________________________________________________________

*/




#ifndef INCLUDED_COMPONENT_____CLASS_US____
#define INCLUDED_COMPONENT_____CLASS_US____



/*

	data/numeric as an example of how to author a data component:

	- some comments on this subject in this file (and the cpp)
	are tagged "COMMENTARY".

*/



                                                                                                        





/*
	COMMENTARY: Data Component Interface
	-------------------------------------------
	
	the interface your data component provides takes the
	form of a number of events which can be passed over the same
	event interface that the framework uses to call the component.
	each event is defined as an event type (a constant) and an
	event data structure (which can be nothing, if no data is
	required) and, of course, a specification of how that event
	works.
*/




/*
	INTERFACE: Event Specification:
	-------------------------------------------

	EVENT_DATA_STRUCTURE_GET (Structure* st)
		"st" will be returned as a pointer to a Structure object which
		is completely filled in (i.e., derived data too). The pointer
		is valid only until the next event is fired on the object. "st"
		represents the object's structure.

	EVENT_DATA_STRUCTURE_SET (Structure* st)
		"st" should be a pointer to a Structure object which has the
		physical parts filled in. The object's structure is set from "st".

	EVENT_DATA_VALIDATE_STRUCTURE (Structure* st)
	EVENT_DATA_VALIDATE_TYPE (Structure* st)
		"st" should be a pointer to a Structure object which has the
		physical parts filled in. If any of the structure is not a
		match, the event returns an error code, which can be returned
		to the framework without modification, or annotated. In
		the case of EVENT_DATA_VALIDATE_TYPE, dimensions are not
		validated at all (allowing just the type to be checked).

	EVENT_DATA_CONTENT_GET (Content* eac)
		"eac" will be returned as a pointer to a Content object
		that is completely filled in. The pointer is valid only until the
		next event is fired on the object. "eac" represents the object's
		content.

	EVENT_DATA_CONTENT_SET (Content* eac)
		"eac" should hold pointers to enough data to fill the object's
		content. If the object has zero elements, one or both pointers
		may be 0. If the object has non-zero elements, eac->real must
		be non-0. If the object is complex, eac->imag may hold a
		pointer to imaginary data. If eac->imag is 0, all data will
		be copied from eac->real (so if the object is complex, there must
		be at least numberOfBytesTotal bytes available there). If eac->imag
		is non-0, numberOfBytesReal bytes are copied from each block.
*/




/*
	COMMENTARY: "C" and "C++" Interfaces
	-------------------------------------------

	your data object's native interface is in C though, as we
	will see below, it is recommended that you provide a wrapper
	for that interface in C++ for ease of use.
	
	both interfaces define the same data structures and event
	constants, but use different (language-tailored) naming
	conventions for these. the C++ interface provides, in addition,
	a C++ class that encapsulates your data component as a C++
	object with methods that can be called to access or modify it.
*/






/*
	INTERFACE: Namespace:
	-------------------------------------------

	if in C++, the interface falls within a namespace named for
	the component's class with slashes replaced by underscores,
	with _<release> as a suffix. your own data components should
	follow this convention to avoid conflicts. later extensions
	to your data object can, if necessary, provide further
	namespaces.
*/

#ifdef __cplusplus

//	need these classes for some API calls
#include <string>
#include <sstream>

namespace ____CLASS_US____
{
#endif



/*
	INTERFACE: Data Structures:
	-------------------------------------------
*/

#ifdef __cplusplus

struct Structure
{
	//	physical
	brahms::TYPE type;			//	full numeric type (includes complexity, shape, flags etc.)
	brahms::Dimensions dims;	//	real elements only; complexity does not affect this count

	//	derived
	brahms::TYPE typeElement;
#else

struct ____CLASS_US_____Structure
{
	//	physical
	TYPE type;					//	full numeric type (includes complexity, shape, flags etc.)
	struct Dimensions dims;		//	real elements only; complexity does not affect this count

	//	derived
	TYPE typeElement;
#endif

	UINT32 bytesPerElement;
	UINT64 numberOfElementsReal;
	UINT64 numberOfElementsTotal;
	UINT64 numberOfBytesReal;
	UINT64 numberOfBytesTotal;
	UINT8 complex;				//	boolean (derived from type)
	UINT8 scalar;				//	boolean (derived from dims)
	UINT8 realScalar;			//	boolean (derived from dims and type)
	UINT8 ____padding;

	//	event data
	UINT32 flags;
};

#ifdef __cplusplus
const UINT16 F_GET_FOR_WRITE = 0x0001; // you *must* pass this to EVENT_DATA_CONTENT_GET if you intend to write through the returned pointers

struct Content
{
	brahms::TYPE type;
#else
#define ____CLASS_US_____F_GET_FOR_WRITE 0x0001

struct ____CLASS_US_____Content
{
	TYPE type;
#endif
	const void* real;
	const void* imag;
	UINT64 bytes;

	//	event data
	UINT32 flags;
};



/*
	INTERFACE: Event Constants:
	-------------------------------------------
*/

#ifdef __cplusplus

const brahms::Symbol EVENT_DATA_STRUCTURE_GET = C_BASE_USER_EVENT + 1;
const brahms::Symbol EVENT_DATA_STRUCTURE_SET = C_BASE_USER_EVENT + 2;
const brahms::Symbol EVENT_DATA_VALIDATE_TYPE = C_BASE_USER_EVENT + 3;
const brahms::Symbol EVENT_DATA_VALIDATE_STRUCTURE = C_BASE_USER_EVENT + 4;
const brahms::Symbol EVENT_DATA_CONTENT_GET = C_BASE_USER_EVENT + 5;
const brahms::Symbol EVENT_DATA_CONTENT_SET = C_BASE_USER_EVENT + 6;

#else

#define ____CLASS_US_____EVENT_DATA_STRUCTURE_GET (C_BASE_USER_EVENT + 1)
#define ____CLASS_US_____EVENT_DATA_STRUCTURE_SET (C_BASE_USER_EVENT + 2)
#define ____CLASS_US_____EVENT_DATA_VALIDATE_TYPE (C_BASE_USER_EVENT + 3)
#define ____CLASS_US_____EVENT_DATA_VALIDATE_STRUCTURE (C_BASE_USER_EVENT + 4)
#define ____CLASS_US_____EVENT_DATA_CONTENT_GET (C_BASE_USER_EVENT + 5)
#define ____CLASS_US_____EVENT_DATA_CONTENT_SET (C_BASE_USER_EVENT + 6)

#endif



/*
	INTERFACE: C++ Wrapper Class:
	-------------------------------------------

	this is a helper function.
*/

#ifdef __cplusplus

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



/*
	INTERFACE: C++ Wrapper Class:
	-------------------------------------------

	this is the base class of the two physical accessors.
*/

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

	const Structure* getStructure()
	{
		check();

		//	handled event
		hev.event.type = EVENT_DATA_STRUCTURE_GET;
		hev.event.data = 0;

		//	mark
		m_seen = true;

		//	fire
		fire();

		//	structure
		return (Structure*) hev.event.data;
	}
};



/*
	INTERFACE: C++ Wrapper Class:
	-------------------------------------------

	this is the output accessor.
*/

struct Output : public Accessor
{

protected:

	____CLASS_US____::Content content;

public:

	Output()
	{
		content.type = TYPE_UNSPECIFIED;
		content.flags = F_GET_FOR_WRITE; // this is ignored in EVENT_DATA_CONTENT_SET, so we can use the same Content structure for GET and SET
	}

	void create(brahms::Symbol hComponent)
	{
		//	only once
		checknot();

		//	store
		this->hComponent = hComponent;

		//	add port
		brahms::EventAddPort apdata;
		apdata.index = INDEX_UNDEFINED;
		apdata.hSet = hSet ? hSet : hComponent; // hComponent means use the default set
		apdata.flags = 0;
		apdata.spec.cls = "std/2009/data/numeric";
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
		//	only once
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

	void setStructure(brahms::TYPE type, const brahms::Dimensions& dims)
	{
		check();

		//	structure
		Structure structure = {0};
		structure.type = type;
		structure.dims = dims;

		//	handled event
		hev.event.type = EVENT_DATA_STRUCTURE_SET;
		hev.event.data = (void*)&structure;

		//	fire
		fire();
	}

	void* getContent()
	{
		//	handled event
		hev.event.type = EVENT_DATA_CONTENT_GET;
		hev.event.data = &content;

		//	fire
		fire();

		//	content - have to cast non-const, naughty but avoids using two "Content" structures
		return (void*) content.real;
	}

	UINT64 getContent(void*& real, void*& imag)
	{
		//	prepare
		hev.event.type = EVENT_DATA_CONTENT_GET;
		hev.event.data = &content;

		//	fire
		fire();

		//	content - have to cast non-const, naughty but avoids using two "Content" structures
		real = (void*) content.real;
		imag = (void*) content.imag;
		return content.bytes;
	}

	void setContent(const void* real, const void* imag = 0, UINT32 bytes = 0)
	{
		//	content
		content.real = real;
		content.imag = imag;
		content.bytes = bytes;

		//	handled event
		hev.event.type = EVENT_DATA_CONTENT_SET;
		hev.event.data = &content;

		//	fire
		fire();
	}
};



/*
	INTERFACE: C++ Wrapper Class:
	-------------------------------------------

	this is the input accessor.
*/

struct Input : public Accessor
{

protected:

	____CLASS_US____::Content content;

public:

	Input()
	{
		content.type = TYPE_CPXFMT_ADJACENT | TYPE_ORDER_COLUMN_MAJOR;
		content.flags = 0;
	}

	void setReadFormat(brahms::TYPE type)
	{
		content.type = type;
	}

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
		getPort.spec.cls = "std/2009/data/numeric";
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
		getPort.spec.cls = "std/2009/data/numeric";
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

	void validateStructure(brahms::TYPE type, const brahms::Dimensions& dims)
	{
		//	structure
		Structure structure = {0};
		structure.type = type;
		structure.dims = dims;

		//	handled event
		hev.event.type = EVENT_DATA_VALIDATE_STRUCTURE;
		hev.event.data = (void*)&structure;

		//	mark
		m_seen = true;

		//	fire
		try
		{
			fire();
		}
		catch(brahms::Symbol e)
		{
			brahms::EventErrorMessage em;
			em.error = e;
			std::stringstream ss;
			ss << "whilst validating input \"" << getName() << "\"";
			std::string msg = ss.str();
			em.msg = msg.c_str();
			em.flags = F_TRACE;
			brahms::EngineEvent event;
			event.hCaller = hComponent;
			event.flags = 0;
			event.type = ENGINE_EVENT_ERROR_MESSAGE;
			event.data = &em;
			throw brahms::brahms_engineEvent(&event);
		}
	}

	void validateStructure(brahms::TYPE type)
	{
		//	structure
		Structure structure = {0};
		structure.type = type;

		//	handled event
		hev.event.type = EVENT_DATA_VALIDATE_TYPE;
		hev.event.data = (void*)&structure;

		//	mark
		m_seen = true;

		//	fire
		try
		{
			fire();
		}
		catch(brahms::Symbol e)
		{
			brahms::EventErrorMessage em;
			em.error = e;
			std::stringstream ss;
			ss << "whilst validating input \"" << getName() << "\"";
			std::string msg = ss.str();
			em.msg = msg.c_str();
			em.flags = F_TRACE;
			brahms::EngineEvent event;
			event.hCaller = hComponent;
			event.flags = 0;
			event.type = ENGINE_EVENT_ERROR_MESSAGE;
			event.data = &em;
			throw brahms::brahms_engineEvent(&event);
		}
	}

	const void* getContent()
	{
		//	handled event
		hev.event.type = EVENT_DATA_CONTENT_GET;
		hev.event.data = &content;

		//	fire
		fire();

		//	content
		return content.real;
	}

	UINT64 getContent(const void*& real, const void*& imag)
	{
		//	prepare
		hev.event.type = EVENT_DATA_CONTENT_GET;
		hev.event.data = &content;

		//	fire
		fire();

		//	content
		real = content.real;
		imag = content.imag;
		return content.bytes;
	}
};



/*
	INTERFACE: Namespace:
	-------------------------------------------
*/

}
#endif






////////////////	INCLUSION GUARD

#endif

