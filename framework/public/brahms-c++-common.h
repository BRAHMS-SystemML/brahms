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

	$Id:: brahms-c++-common.h 2437 2009-12-13 19:06:12Z benjmi#$
	$Rev:: 2437                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-12-13 19:06:12 +0000 (Sun, 13 Dec 2009)       $
________________________________________________________________

*/


/*

	This is a C++ overlay interface for the BRAHMS Component Interface.

	Binding: __BINDING__
	Revision: __REV__

	The BRAHMS Component Interface is a set of C structures and functions.
	Overlay interfaces are C++ bases from which you can build your own
	components using C++ techniques. They considerably simplify component
	authoring, provided you are a fan of C++.

*/




////////////////////////////////////////////////////////////////
//
//	Half of this file gets processed the first time it is
//	included, and half the second time. On subsequent includes,
//	nothing is processed.
//
////////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////////
//
//	The first section, the H section, gets processed the
//	*first* time this file is included.
//

#ifndef INCLUDED_OVERLAY_1
#define INCLUDED_OVERLAY_1

//
////////////////////////////////////////////////////////////////











#ifndef BRAHMS_BUILD_TARGET
#define BRAHMS_BUILD_TARGET __BINDING__
#endif

#define BRAHMS_BINDING_NAME __BINDING__
#define BRAHMS_BINDING_REVISION __REV__

#define ____CLEAR(what) memset(&what, 0, sizeof(what))













//  legacy define __LINUX__ means __GLN__
#ifdef __LINUX__
#ifndef __GLN__
#define __GLN__
#endif
#endif

//  legacy define __WINDOWS__ means __WIN__
#ifdef __WINDOWS__
#ifndef __WIN__
#define __WIN__
#endif
#endif

//  BRAHMS C Component Interface
#include "brahms-component.h"





////////////////////////////////////////////////////////////////
//
//	To simplify process authoring, the most common options
//	can be included simply by defining this symbol
//

#ifdef OVERLAY_QUICKSTART_PROCESS

//	component type
#define COMPONENT_PROCESS

//	bring in namespaces
using namespace brahms;

#endif

//
////////////////////////////////////////////////////////////////


//	identify component type
#ifdef COMPONENT_PROCESS
#define COMPONENT_PROCESS_OR_DATA
#define COMPONENT_TYPE CT_PROCESS
#endif

#ifdef COMPONENT_DATA
#define COMPONENT_PROCESS_OR_DATA
#define COMPONENT_TYPE CT_DATA
#endif

#ifdef COMPONENT_UTILITY
#define COMPONENT_TYPE CT_UTILITY
#endif

//	assume process if not specified
#ifndef COMPONENT_TYPE
#define COMPONENT_PROCESS
#define COMPONENT_PROCESS_OR_DATA
#define COMPONENT_TYPE CT_PROCESS
#endif

//  C++ standard library
#include <string>
#include <vector>
#include <exception>
#include <sstream>
#include <fstream>
#include <limits>
#include <numeric>

//	C standard library
#include <cmath>
#include <ctime>
#include <cctype>
#include <cstring>

//	helper function
std::string ____N2S(UINT32 n)
{
	std::ostringstream s;
	s << n;
	return s.str();
}

//	helper function
std::string ____NOPATH(std::string path)
{
	size_t pos1 = path.rfind("/");
	size_t pos2 = path.rfind("\\");
	if (pos1 != std::string::npos && pos2 != std::string::npos)
	{
		if (pos1 > pos2) path = path.substr(pos1+1);
		else path = path.substr(pos2+1);
	}
	else if (pos1 != std::string::npos) path = path.substr(pos1+1);
	else if (pos2 != std::string::npos) path = path.substr(pos2+1);
	return path;
}

//	helper function
std::string ____TRACE_STRING(const char* function, const char* file, UINT32 line)
{
	std::string ret = "in ";
	ret += function;
	ret += "() at ";
	ret += ____NOPATH(file);
	ret += ":" + ____N2S(line);
	return ret;
}

//  synonyms for standard numeric types
typedef UINT8                   BYTE;



namespace brahms
{



////////////////    ENGINE VERSION STRUCTURE

	const FrameworkVersion VERSION_ENGINE = {
		VERSION_ENGINE_MAJ,
		VERSION_ENGINE_MIN,
		VERSION_ENGINE_REL,
		VERSION_ENGINE_REV
	};



////////////////    VECTOR TYPES

	typedef std::vector<DOUBLE>     VDOUBLE;
	typedef std::vector<SINGLE>     VSINGLE;
	typedef std::vector<UINT64>     VUINT64;
	typedef std::vector<UINT32>     VUINT32;
	typedef std::vector<UINT16>     VUINT16;
	typedef std::vector<UINT8>      VUINT8;
	typedef std::vector<INT64>      VINT64;
	typedef std::vector<INT32>      VINT32;
	typedef std::vector<INT16>      VINT16;
	typedef std::vector<INT8>       VINT8;
	typedef std::vector<CHAR64>     VCHAR64;
	typedef std::vector<CHAR32>     VCHAR32;
	typedef std::vector<CHAR16>     VCHAR16;
	typedef std::vector<CHAR8>      VCHAR8;
	typedef std::vector<BOOL64>     VBOOL64;
	typedef std::vector<BOOL32>     VBOOL32;
	typedef std::vector<BOOL16>     VBOOL16;
	typedef std::vector<BOOL8>      VBOOL8;
	typedef std::vector<bool>       Vbool;
	typedef std::vector<BYTE>       VBYTE;
	typedef std::vector<std::string> VSTRING;



////////////////    HANDY CONVERSION FUNCTION FOR SAMPLE RATE

	inline DOUBLE sampleRateToRate(const SampleRate& sr)
	{
		return ((DOUBLE)sr.num) / ((DOUBLE)sr.den);
	}

	inline DOUBLE sampleRateToPeriod(const SampleRate& sr)
	{
		return ((DOUBLE)sr.den) / ((DOUBLE)sr.num);
	}



////////////////	ERROR BUILDING CLASS WITH STREAM INTERFACE

	class ErrorBuilder
	{

	public:

		//	ctor
		ErrorBuilder()
		{
			code = S_NULL;
		}

		//	copy ctor (copy is used to invoke throwing)
		ErrorBuilder(const ErrorBuilder& src)
		{
			code = src.code;
			if (code == S_NULL) code = E_USER;
			EventErrorMessage data;
			data.error = code;
			std::string msg = src.msg.str();
			data.msg = msg.c_str();
			data.flags = 0;
			
			EngineEvent event;
			event.hCaller = 0;
			event.flags = 0;
			event.type = ENGINE_EVENT_ERROR_MESSAGE;
			event.data = &data;
			data.error = brahms_engineEvent(&event);

			std::string trace = src.trace.str();
			data.msg = trace.c_str();
			data.flags = F_DEBUGTRACE;

			throw brahms_engineEvent(&event);
		}

		//	handle symbol
		ErrorBuilder& operator<<(Symbol data)
		{
			if (code == S_NULL) code = data;
			else msg << data;
			return (*this);
		}

		//	handle data
		template<class T> ErrorBuilder& operator<<(const T& data)
		{
			if (code == S_NULL) code = E_USER;
			msg << data;
			return (*this);
		}

		//	handle function
		template<class T> ErrorBuilder& operator<<(T& (*data)(T&))
		{
			if (code == S_NULL) code = E_USER;
			msg << data;
			return (*this);
		}

		//	handle trace
		ErrorBuilder& at(const char* function, const char* file, int line)
		{
			trace << ____TRACE_STRING(function, file, line);
			return (*this);
		}

		//	members
		Symbol code;
		std::stringstream msg, trace;

	};



////////////////	ERROR RAISING MACRO

#define berr brahms::ErrorBuilder unusedInstanceInvokesCopyCtor = brahms::ErrorBuilder().at(__FUNCTION__, __FILE__, __LINE__)



////////////////    DIMENSION CLASS

	struct Dims : public VINT64
	{

		//  "explicit" http://www.devx.com/tips/Tip/12493
		//  prevents the syntax:
		//      Dims d;
		//      d = 7;
		//  (which may be undesirable, it's certainly a bit weird)
		//  use instead:
		//      Dims d(7);
		//      Dims e = Dims(7);

		explicit Dims(INT64 d1 = DIM_ABSENT, INT64 d2 = DIM_ABSENT, INT64 d3 = DIM_ABSENT, INT64 d4 = DIM_ABSENT)
		{
			if (d1 != DIM_ABSENT) push_back(d1);
			if (d2 != DIM_ABSENT) push_back(d2);
			if (d3 != DIM_ABSENT) push_back(d3);
			if (d4 != DIM_ABSENT) push_back(d4);
		}

		Dims(const VINT64& stl)
		{
			for (UINT32 d=0; d<stl.size(); d++)
				push_back(stl[d]);
		}

		Dims(const Dimensions& dims)
		{
			for(UINT32 d=0; d<dims.count; d++)
				push_back(dims.dims[d]);
		}

		Dims& operator=(const VINT64& stl)
		{
			clear();
			for (UINT32 d=0; d<stl.size(); d++)
				push_back(stl[d]);
			return *this;
		}

		Dims& operator=(const Dimensions& dims)
		{
			clear();
			for(UINT32 d=0; d<dims.count; d++)
				push_back(dims.dims[d]);
			return *this;
		}

		bool operator==(const Dims& comp)
		{
			if (size() != comp.size()) return false;
			for(UINT32 d=0; d<size(); d++)
				if ((*this)[d] != comp[d]) return false;
			return true;
		}

		bool operator!=(const Dims& comp)
		{
			if (size() != comp.size()) return true;
			for(UINT32 d=0; d<size(); d++)
				if ((*this)[d] != comp[d]) return true;
			return false;
		}

		Dimensions cdims() const
		{
			Dimensions d;
			d.dims = size() ? &((*this)[0]) : NULL;
			d.count = size();
			return d;
		}

		operator std::string() const
		{
			std::ostringstream ret;
			for (UINT32 d=0; d<size(); d++)
			{
				if (d) ret << " ";
				ret << at(d);
			}
			return ret.str();
		}

		std::string commaString() const
		{
			std::ostringstream ret;
			for (UINT32 d=0; d<size(); d++)
			{
				if (d) ret << ","; // comma-separated
				ret << at(d);
			}
			return ret.str();
		}

		UINT64 getNumberOfElements() const
		{
			if (!size()) return 0;
			UINT64 numberOfElements = 1;
			for (UINT32 d=0; d<size(); d++)
				numberOfElements *= at(d);
			return numberOfElements;
		}

	};



////////////////    API ASSERT SUCCESS FUNCTION (THROW IF ERROR, ELSE PASS UNCHANGED)

	inline Symbol ____SUCCESS(Symbol result)
	{
		if (S_ERROR(result)) throw result;
		return result;
	}



////////////////    OUTPUT CLASS

	//	enum detail level allows us to treat it differently in ComponentOut
	#undef D_NONE
	#undef D_WARN
	#undef D_INFO
	#undef D_VERB

	enum EnumDetailLevel
	{
		D_NONE =    0x00,       //  normal levels
		D_WARN =    0x20,
		D_INFO =    0x40,
		D_VERB =    0x60,

		//	legacy (all resolve to D_VERB)
		D_1 =       0x60,
		D_2 =       0x60,
		D_3 =       0x60,
		D_4 =       0x60,
		D_5 =       0x60
	};

	class ComponentOut
	{

	public:

		ComponentOut()
		{
			hComponent = S_NULL;
		}

		void initialize(Symbol hComponent)
		{
			this->hComponent = hComponent;
		}

		void operator<<(EnumDetailLevel level)
		{
			EventOutputMessage data;
			std::string s = event.str();
			event.str("");
			data.msg = s.c_str();
			data.flags = 0;
			data.level = level;

			EngineEvent event;
			event.hCaller = hComponent;
			event.flags = 0;
			event.type = ENGINE_EVENT_OUTPUT_MESSAGE;
			event.data = &data;

			____SUCCESS(brahms_engineEvent(&event));
		}

		template<class T> ComponentOut& operator<<(const T& data)
		{
			event << data;
			return *this;
		}

		template<class T> ComponentOut& operator<<(T& (*data)(T&))
		{
			event << data;
			return *this;
		}

	private:

		std::ostringstream event;
		Symbol hComponent;

	};






////////////////	MODULE-LEVEL DATA

	//	system info (set in EVENT_MODULE_INIT)
	const ExecutionInfo* executionInfo;

	//	LEGACY
	#define systemInfo executionInfo



////////////////    SAFE FIRE EVENT

#ifdef BRAHMS_HIGH_PERFORMANCE

	Symbol fireHandledEvent(HandledEvent* hev)
	{
		Symbol result = ____SUCCESS(hev->handler(&hev->event));
	}

#else

	Symbol fireHandledEvent(HandledEvent* hev)
	{
		if (!hev)
			berr << E_NULL_ARG << "NULL argument in fireHandledEvent()";

		if (!hev->handler)
			berr << E_NULL_ARG << "NULL \"handler\" in fireHandledEvent()";

		return ____SUCCESS(hev->handler(&hev->event));
	}

#endif



////////////////	XML OVERLAY

	class XMLNode
	{

	public:

		XMLNode()
		{
			element = ____SUCCESS(xml_createElement(NULL));
		}

		XMLNode(const char* name)
		{
			element = ____SUCCESS(xml_createElement(name));
		}

		XMLNode(const char* name, const char* text)
		{
			element = ____SUCCESS(xml_createElement(name));
			____SUCCESS(xml_setNodeText(element, text));
		}

		XMLNode(Symbol node)
		{
			this->element = node;
		}

		~XMLNode()
		{
			for (UINT32 c=0; c<children.size(); c++)
				delete children[c];
		}

		XMLNode* getChild(const char* name, UINT32 index = 0)
		{
			Symbol child = xml_getChild(element, name, index);
			if (S_ERROR(child))
			{
				if (child == E_NOT_FOUND)
				{
					//	add message (isn't added on API)
					const char* nodeName = NULL;
					Symbol result = xml_getNodeName(element, &nodeName);
					if (S_ERROR(result)) nodeName = "<error-getting-node-name>";

					berr << E_NOT_FOUND << "looking for child \"" << name << "\" on node \"" << nodeName << "\"";
					if (index) berr << E_NOT_FOUND << "looking for child \"" << name << "\" (" << index << ") on node \"" << nodeName << "\"";
				}

				throw child;
			}
			XMLNode* node = new XMLNode(child);
			children.push_back(node);
			return node;
		}

		XMLNode* getChildOrNull(const char* name, UINT32 index = 0)
		{
			Symbol child = xml_getChild(element, name, index);
			if (child == E_NOT_FOUND) return NULL;
			if (S_ERROR(child)) throw child;
			XMLNode* node = new XMLNode(child);
			children.push_back(node);
			return node;
		}

		XMLNode* appendChild(XMLNode* child)
		{
			____SUCCESS(xml_appendChild(element, child->element));
			return child;
		}

		const char* getAttribute(const char* name)
		{
			const char* value = NULL;
			Symbol result = xml_getAttribute(element, name, &value);
			if (S_ERROR(result))
			{
				if (result == E_NOT_FOUND)
				{
					//	add message (isn't added on API)
					const char* nodeName = NULL;
					result = xml_getNodeName(element, &nodeName);
					if (S_ERROR(result)) nodeName = "<error-getting-node-name>";
					berr << E_NOT_FOUND << "looking for attribute \"" << name << "\" on node \"" << nodeName << "\"";
				}

				throw result;
			}
			return value;
		}

		const char* getAttributeOrNull(const char* name)
		{
			const char* value = NULL;
			Symbol result = xml_getAttribute(element, name, &value);
			if (result == E_NOT_FOUND) return NULL;
			if (S_ERROR(result)) throw result;
			return value;
		}

		void setAttribute(const char* key, const char* value)
		{
			____SUCCESS(xml_setAttribute(element, key, value));
		}

		const char* nodeName()
		{
			const char* name = NULL;
			____SUCCESS(xml_getNodeName(element, &name));
			return name;
		}

		const char* nodeText()
		{
			const char* text = NULL;
			____SUCCESS(xml_getNodeText(element, &text));
			return text;
		}

		void nodeName(const char* name)
		{
			____SUCCESS(xml_setNodeName(element, name));
		}

		void nodeText(const char* text)
		{
			____SUCCESS(xml_setNodeText(element, text));
		}

		std::vector<XMLNode> childNodes()
		{
			std::vector<Symbol> children;
			children.resize(1);
			UINT32 count = 1;

			//	first call (with one element of space available)
			Symbol result = ____SUCCESS(xml_childNodes(element, &children[0], &count));

			//	second call (with the correct number available)
			if (result == E_OVERFLOW)
			{
				children.resize(count);
				____SUCCESS(xml_childNodes(element, &children[0], &count));
			}

			std::vector<XMLNode> nodes;
			nodes.resize(count); // count always holds the correct count at this point
			for (UINT32 n=0; n<count; n++)
				nodes[n].element = children[n];
			return nodes;
		}

		void clear()
		{
			____SUCCESS(xml_clearNode(element));
		}

		Symbol element;
		std::vector<XMLNode*> children;

	};



////////////////    SYSTEMML INTERFACE CLASS

#ifdef COMPONENT_PROCESS

	std::string translateSetName(std::string name)
	{
		if (name.length()) return name;
		else return "<default set>";
	}

	struct SystemMLInterface
	{
		SystemMLInterface()
		{
			hObject = S_NULL;
			flags = 0;
			defaultSet = S_NULL;
		}

		void initialize(Symbol hObject, UINT32 flags)
		{
			this->hObject = hObject;
			this->flags = flags;
			getDefaultSet();
		}

		UINT32 getNumberOfPorts(Symbol hSet)
		{
#if __BINDING__ == 1065
			//	1065 has old DEFAULT_SET semantics, but we don't want them in 1199
			if (hSet == -1) hSet = getDefaultSet();
#endif
			EventGetSetInfo info;
			info.hSet = hSet;
			info.flags = 0;
			info.name = NULL;
			info.portCount = 0;

			EngineEvent event;
			event.hCaller = hObject;
			event.flags = 0;
			event.type = ENGINE_EVENT_GET_SET_INFO;
			event.data = (void*) &info;

			____SUCCESS(brahms::brahms_engineEvent(&event));
			return info.portCount;
		}

		UINT32 getNumberOfPorts()
		{
			return getNumberOfPorts(getDefaultSet());
		}

		Symbol getSet(const char* name)
		{
			EventGetSet data;
			data.flags = flags;
			data.name = name;

			EngineEvent event;
			event.hCaller = hObject;
			event.flags = 0;
			event.type = ENGINE_EVENT_GET_SET;
			event.data = &data;

			return ____SUCCESS(brahms::brahms_engineEvent(&event));
		}

		Symbol addPort(Symbol hSet, const char* cls, UINT16 release)
		{
#if __BINDING__ == 1065
			//	1065 has old DEFAULT_SET semantics, but we don't want them in 1199
			if (hSet == -1) hSet = getDefaultSet();
#endif
			EventAddPort data;
			data.hSet = hSet;
			data.flags = 0;
			data.spec.cls = cls;
			data.spec.release = release;
			data.hPortToCopy = S_NULL;
			data.name = NULL;
			data.index = INDEX_UNDEFINED;
			data.transportProtocol = C_TRANSPORT_PERIODIC;
			data.transportData = NULL;
			data.handledEvent = NULL;

			EngineEvent event;
			event.hCaller = hObject;
			event.flags = 0;
			event.type = ENGINE_EVENT_ADD_PORT;
			event.data = &data;

			return ____SUCCESS(brahms::brahms_engineEvent(&event));
		}

		Symbol addPort(Symbol hSet, Symbol copyThis)
		{
#if __BINDING__ == 1065
			//	1065 has old DEFAULT_SET semantics, but we don't want them in 1199
			if (hSet == -1) hSet = getDefaultSet();
#endif
			EventAddPort data;
			data.flags = 0;
			data.hSet = hSet;
			data.spec.cls = NULL;
			data.spec.release = 0;
			data.hPortToCopy = copyThis;
			data.name = NULL;
			data.index = INDEX_UNDEFINED;
			data.transportProtocol = C_TRANSPORT_PERIODIC;
			data.transportData = NULL;
			data.handledEvent = NULL;

			EngineEvent event;
			event.hCaller = hObject;
			event.flags = 0;
			event.type = ENGINE_EVENT_ADD_PORT;
			event.data = &data;

			return ____SUCCESS(brahms::brahms_engineEvent(&event));
		}



#ifndef BRAHMS_NO_LEGACY_SUPPORT

		Symbol getPort(Symbol hSet, UINT32 portIndex)
		{
#if __BINDING__ == 1065
			//	1065 has old DEFAULT_SET semantics, but we don't want them in 1199
			if (hSet == -1) hSet = getDefaultSet();
#endif
			EventGetPort data;
			data.hSet = hSet;
			data.flags = 0;
			data.handledEvent = NULL;
			data.spec.cls = NULL;
			data.spec.release = 0;
			data.name = NULL;
			data.index = portIndex;

			//	use legacy call because it allows us to get an output port as well as an input port
			EngineEvent event;
			event.hCaller = hObject;
			event.flags = 0;
			event.type = ENGINE_EVENT_GET_PORT_ON_SET;
			event.data = &data;

			return ____SUCCESS(brahms::brahms_engineEvent(&event));
		}

		Symbol getPort(Symbol hSet, std::string name)
		{
#if __BINDING__ == 1065
			//	1065 has old DEFAULT_SET semantics, but we don't want them in 1199
			if (hSet == -1) hSet = getDefaultSet();
#endif
			EventGetPort data;
			data.hSet = hSet;
			data.flags = 0;
			data.handledEvent = NULL;
			data.spec.cls = NULL;
			data.spec.release = 0;
			data.name = name.c_str();
			data.index = INDEX_UNDEFINED;

			//	use legacy call because it allows us to get an output port as well as an input port
			EngineEvent event;
			event.hCaller = hObject;
			event.flags = 0;
			event.type = ENGINE_EVENT_GET_PORT_ON_SET;
			event.data = &data;
			
			Symbol handle = ____SUCCESS(brahms::brahms_engineEvent(&event));

#if __BINDING__ != 1065
			//	1065 has old semantics here, too
			if (handle == S_NULL)
			{
				EventGetSetInfo info;
				info.hSet = hSet;
				info.flags = 0;
				info.name = NULL;
				info.portCount = 0;

				EngineEvent event;
				event.hCaller = hObject;
				event.flags = 0;
				event.type = ENGINE_EVENT_GET_SET_INFO;
				event.data = (void*) &info;

				Symbol ret = brahms_engineEvent(&event);
				if (S_ERROR(ret)) info.name = "<could not get name>";
				berr << E_NOT_FOUND << "port \"" << name << "\" not found on set \"" << translateSetName(info.name) << "\"";
			}
#endif

			return handle;
		}

#endif



#ifdef BRAHMS_BUILDING_BINDINGS

		//	used by bindings only...
		Symbol addPortHEV(Symbol hSet, const char* cls, UINT16 release, HandledEvent* handledEvent)
		{
#if __BINDING__ == 1065
			//	1065 has old DEFAULT_SET semantics, but we don't want them in 1199
			if (hSet == -1) hSet = getDefaultSet();
#endif
			EventAddPort data;
			data.hSet = hSet;
			data.flags = 0;
			data.spec.cls = cls;
			data.spec.release = release;
			data.hPortToCopy = S_NULL;
			data.name = NULL;
			data.index = INDEX_UNDEFINED;
			data.transportProtocol = C_TRANSPORT_PERIODIC;
			data.transportData = NULL;
			data.handledEvent = handledEvent;

			EngineEvent event;
			event.hCaller = hObject;
			event.flags = 0;
			event.type = ENGINE_EVENT_ADD_PORT;
			event.data = &data;

			return ____SUCCESS(brahms::brahms_engineEvent(&event));
		}

		//	used by bindings only...
		Symbol addPortHEV(Symbol hSet, Symbol copyThis, HandledEvent* handledEvent)
		{
#if __BINDING__ == 1065
			//	1065 has old DEFAULT_SET semantics, but we don't want them in 1199
			if (hSet == -1) hSet = getDefaultSet();
#endif
			EventAddPort data;
			data.flags = 0;
			data.hSet = hSet;
			data.spec.cls = NULL;
			data.spec.release = 0;
			data.hPortToCopy = copyThis;
			data.name = NULL;
			data.index = INDEX_UNDEFINED;
			data.transportProtocol = C_TRANSPORT_PERIODIC;
			data.transportData = NULL;
			data.handledEvent = handledEvent;

			EngineEvent event;
			event.hCaller = hObject;
			event.flags = 0;
			event.type = ENGINE_EVENT_ADD_PORT;
			event.data = &data;

			return ____SUCCESS(brahms::brahms_engineEvent(&event));
		}

		//	used by bindings only...
		Symbol getPortHEV(Symbol hSet, UINT32 portIndex, HandledEvent* handledEvent)
		{
#if __BINDING__ == 1065
			//	1065 has old DEFAULT_SET semantics, but we don't want them in 1199
			if (hSet == -1) hSet = getDefaultSet();
#endif
			EventGetPort data;
			data.hSet = hSet;
			data.flags = 0;
			data.handledEvent = handledEvent;
			data.spec.cls = NULL;
			data.spec.release = 0;
			data.name = NULL;
			data.index = portIndex;

			EngineEvent event;
			event.hCaller = hObject;
			event.flags = 0;
			event.type = ENGINE_EVENT_GET_PORT;
			event.data = &data;
			
			return ____SUCCESS(brahms::brahms_engineEvent(&event));
		}

		//	used by bindings only...
		Symbol getPortHEV(Symbol hSet, std::string name, HandledEvent* handledEvent)
		{
#if __BINDING__ == 1065
			//	1065 has old DEFAULT_SET semantics, but we don't want them in 1199
			if (hSet == -1) hSet = getDefaultSet();
#endif
			EventGetPort data;
			data.hSet = hSet;
			data.flags = 0;
			data.handledEvent = handledEvent;
			data.spec.cls = NULL;
			data.spec.release = 0;
			data.name = name.c_str();
			data.index = INDEX_UNDEFINED;

			EngineEvent event;
			event.hCaller = hObject;
			event.flags = 0;
			event.type = ENGINE_EVENT_GET_PORT;
			event.data = &data;
			
			Symbol handle = ____SUCCESS(brahms::brahms_engineEvent(&event));

#if __BINDING__ != 1065
			//	1065 has old semantics here, too
			if (handle == S_NULL)
			{
				EventGetSetInfo info;
				info.hSet = hSet;
				info.flags = 0;
				info.name = NULL;
				info.portCount = 0;

				EngineEvent event;
				event.hCaller = hObject;
				event.flags = 0;
				event.type = ENGINE_EVENT_GET_SET_INFO;
				event.data = (void*) &info;

				Symbol ret = brahms_engineEvent(&event);
				if (S_ERROR(ret)) info.name = "<could not get name>";
				berr << E_NOT_FOUND << "port \"" << name << "\" not found on set \"" << translateSetName(info.name) << "\"";
			}
#endif

			return handle;
		}

#endif // #ifdef BRAHMS_BUILDING_BINDINGS

		Symbol getPortOrNull(Symbol hSet, std::string name)
		{
#if __BINDING__ == 1065
			//	1065 has old DEFAULT_SET semantics, but we don't want them in 1199
			if (hSet == -1) hSet = getDefaultSet();
#endif
			EventGetPort data;
			data.hSet = hSet;
			data.flags = 0;
			data.handledEvent = NULL;
			data.name = name.c_str();
			data.index = INDEX_UNDEFINED;
			
			EngineEvent event;
			event.hCaller = hObject;
			event.flags = 0;
			event.type = ENGINE_EVENT_GET_PORT;
			event.data = &data;
			
			return ____SUCCESS(brahms::brahms_engineEvent(&event));
		}

		Symbol addPort(const char* cls, UINT16 release)
		{
			return addPort(getDefaultSet(), cls, release);
		}

		Symbol addPort(Symbol copyThis)
		{
			return addPort(getDefaultSet(), copyThis);
		}

		void setPortName(Symbol hPort, const char* portName)
		{
			//	make legacy call
			brahms::EngineEvent event;
			event.hCaller = hPort;
			event.flags = 0;
			event.type = ENGINE_EVENT_SET_PORT_NAME;
			event.data = (void*) portName;
			
			____SUCCESS(brahms::brahms_engineEvent(&event));
		}

#ifndef BRAHMS_NO_LEGACY_SUPPORT

		Symbol getPort(UINT32 portIndex)
		{
			return getPort(getDefaultSet(), portIndex);
		}

		Symbol getPort(std::string name)
		{
			return getPort(getDefaultSet(), name);
		}

		Symbol getPortOrNull(std::string name)
		{
			return getPortOrNull(getDefaultSet(), name);
		}

		Symbol getData(Symbol hPort)
		{
			//	make legacy call
			Symbol hData;
			brahms::EngineEvent event;
			event.hCaller = hPort;
			event.flags = 0;
			event.type = ENGINE_EVENT_DATA_FROM_PORT;
			event.data = (void*) &hData;
			
			____SUCCESS(brahms::brahms_engineEvent(&event));

			//	throw if empty
			if (hData == S_NULL) berr << E_PORT_EMPTY;
			return hData;
		}

		Symbol getDataOrNull(Symbol hPort)
		{
			//	make legacy call
			Symbol hData;
			brahms::EngineEvent event;
			event.hCaller = hPort;
			event.flags = 0;
			event.type = ENGINE_EVENT_DATA_FROM_PORT;
			event.data = (void*) &hData;
			
			____SUCCESS(brahms::brahms_engineEvent(&event));

			//	ok even if empty
			return hData;
		}

#endif

		void setPortSampleRate(Symbol hPort, SampleRate sampleRate)
		{
			//	make legacy call
			brahms::EngineEvent event;
			event.hCaller = hPort;
			event.flags = 0;
			event.type = ENGINE_EVENT_SET_PORT_SAMPLE_RATE;
			event.data = (void*) &sampleRate;
			
			____SUCCESS(brahms::brahms_engineEvent(&event));
		}

		Symbol getDefaultSet()
		{
			if (defaultSet == S_NULL) defaultSet = getSet("");
			return defaultSet;
		}

		const char* getPortName(Symbol hPort)
		{
			EventGetPortInfo info;
			info.hPort = hPort;
			info.flags = 0;
			info.name = NULL;
			info.componentInfo = NULL;

			EngineEvent event;
			event.hCaller = hObject;
			event.flags = 0;
			event.type = ENGINE_EVENT_GET_PORT_INFO;
			event.data = (void*) &info;

			____SUCCESS(brahms::brahms_engineEvent(&event));
			return info.name;
		}

		const char* getSetName(Symbol hSet)
		{
			EventGetSetInfo info;
			info.hSet = hSet;
			info.flags = 0;
			info.name = NULL;
			info.portCount = 0;

			EngineEvent event;
			event.hCaller = hObject;
			event.flags = 0;
			event.type = ENGINE_EVENT_GET_SET_INFO;
			event.data = (void*) &info;

			____SUCCESS(brahms::brahms_engineEvent(&event));
			return info.name;
		}

		UINT32 getPortFlags(Symbol hPort)
		{
			EventGetPortInfo info;
			info.hPort = hPort;
			info.flags = 0;
			info.name = NULL;
			info.componentInfo = NULL;

			EngineEvent event;
			event.hCaller = hObject;
			event.flags = 0;
			event.type = ENGINE_EVENT_GET_PORT_INFO;
			event.data = (void*) &info;

			____SUCCESS(brahms::brahms_engineEvent(&event));
			return info.flags;
		}

	public:

		Symbol hObject;
		UINT32 flags;
		Symbol defaultSet;

	};

#endif



////////////////    CONTRACT (SUCCEED OR THROW) API WRAPPERS

	inline const char* getSymbolString(Symbol symbol)
	{
		EventGetSymbolString gss;
		gss.flags = 0;
		gss.symbol = symbol;
		gss.result = NULL;

		EngineEvent event;
		event.hCaller = 0;
		event.flags = 0;
		event.type = ENGINE_EVENT_GET_SYMBOL_STRING;
		event.data = (void*) &gss;

		____SUCCESS(brahms_engineEvent(&event));
		return gss.result;
	}

	inline const char* getTypeString(TYPE type)
	{
		EventGetTypeString gets;
		gets.flags = 0;
		gets.type = type;
		gets.result = NULL;

		EngineEvent event;
		event.hCaller = 0;
		event.flags = 0;
		event.type = ENGINE_EVENT_GET_TYPE_STRING;
		event.data = (void*) &gets;

		____SUCCESS(brahms_engineEvent(&event));
		return gets.result;
	}

	inline const char* getElementTypeString(TYPE type)
	{
		return getTypeString(type);
	}



////////////////	DATAML OVERLAY

	/*

		DataMLNode is implemented as a purely functional overlay on
		XMLNode (the three items of data are actually just caches
		of data that exists on the XMLNode anyway).

	*/

	//	version
	#define DATAML_VERSION "5"

	//const UINT32 F_ENCAPSULATED	= 0x00000001; // defined in Component Interface
	const UINT32 F_STRICT			= 0x00000002;
	const UINT32 F_REAL				= 0x00000004;

	//	private flags
	const UINT32 F_STRING_FMT		= 0x80000000;
	const UINT32 F_BINARY_FILE_FMT	= 0x40000000;

	#define ENSURE_STRUCT { if ((cache.type & TYPE_ELEMENT_MASK) != TYPE_STRUCT) berr << E_DATAML << "expected " << getNodeNoun() << " to be a struct node"; }
	#define ENSURE_CELL { if ((cache.type & TYPE_ELEMENT_MASK) != TYPE_CELL) berr << E_DATAML << "expected " << getNodeNoun() << " to be a cell node"; }




	const UINT64 SUB_MAX_UINT64 = (UINT64)1000000000000000000ULL;

	//	in the setArray interface, we assume TYPE_CPXFMT_ADJACENT | TYPE_ORDER_COLUMN_MAJOR
	const TYPE TYPE_DATAML_SETARRARY_ASSUMED = TYPE_CPXFMT_ADJACENT | TYPE_ORDER_COLUMN_MAJOR;


	class DataMLNode
	{

	public:

		//	wrapped XMLNode
		XMLNode*		xmlNode;

		//	cached data from the XMLNode, calculated in the ctor
		struct
		{
			TYPE		type;
			Dims		dims;
			UINT64		numberOfRealElements;
		}
		cache;

		//	mode data (implementation hidden)
		UINT32			flags;
		UINT32			prec;

		//	node name
		std::string		name;



	////////////////////////////////////////////////////////////////
	////	PRIVATE FUNCTIONS

	private:

		std::string getNodeNoun() const
		{
			return "DataML node \"" + name + "\"";
		}

		std::string ordinal(UINT32 n)
		{
			std::ostringstream ret;
			ret << n;
			if ((n%100)>10 && (n%100)<20) ret << "th";
			else
			{
				switch (n % 10)
				{
				case 1: ret << "st"; break;
				case 2: ret << "nd"; break;
				case 3: ret << "rd"; break;
				default: ret << "th";
				}
			}
			return ret.str();
		}

		//	subroutine for nulling
		void clear()
		{
			xmlNode = NULL;
			cache.type = TYPE_UNSPECIFIED;
			cache.numberOfRealElements = 0;
			flags = F_ENCAPSULATED | F_REAL;
			prec = PRECISION_NOT_SET;
		}






	////////////////////////////////////////////////////////////////
	////	CONSTRUCTOR

	public:

		//	create unassigned DataMLNode interface
		DataMLNode()
		{
			//	the interface can be created without being associated with an XML node. in that
			//	case, "xmlNode" will be NULL.
			clear();
		}

		//	new interface - pass by pointer (still works!)
		DataMLNode(XMLNode* xmlNode)
		{
			//	the interface can be assigned to a specific XML node on creation. the only other
			//	way is by assignment using "operator=", below.
			clear();
			assign(xmlNode);
		}

		//	new interface - pass by reference
		DataMLNode(XMLNode& xmlNode)
		{
			//	the interface can be assigned to a specific XML node on creation. the only other
			//	way is by assignment using "operator=", below.
			clear();
			assign(&xmlNode);
		}

		//	can use the compiler assigned version of this function, because the interface will be copied correctly
		//
		//	DataMLNode& operator=(const DataMLNode& xmlNode)
		//	{
		//	}

		//	but we also allow DataMLNode = XMLNode
		DataMLNode& operator=(XMLNode& xmlNode)
		{
			//	call assign
			assign(&xmlNode);
			
			//	ok
			return *this;
		}

		//	subroutine for actually doing the assignment
		void assign(XMLNode* xmlNode)
		{
			//	store XML node
			this->xmlNode = xmlNode;
			if (!xmlNode) berr << E_DATAML << "cannot construct a DataMLNode from a NULL XMLNode";
			name = xmlNode->nodeName();

			//	get node text
			const char* text = xmlNode->nodeText();

			//	assume not binary file fmt, unless specified
			flags &= (~F_BINARY_FILE_FMT);
			const char* storage = xmlNode->getAttributeOrNull("s");
			if (storage)
			{
				if (storage == std::string("b"))
					flags |= F_BINARY_FILE_FMT;
				else berr << E_DATAML << "malformed storage modifier \"" << storage << "\" during parse of " << getNodeNoun();
			}

			//	get data type
			/*
				DataML type is...

				1) if explicitly specified using c="type"
				2) otherwise, c="string"
			*/
			const char* dataml = xmlNode->getAttributeOrNull("c");

			if (dataml)
			{
				std::string attrDataML(dataml);
				char c = 0;
				TYPE cpx = TYPE_UNSPECIFIED;

				switch (attrDataML.length())
				{
					case 1:
					{
						//	just a single class character (see specification)
						c = attrDataML[0];
						cpx = TYPE_REAL | TYPE_CPXFMT_ADJACENT; // TYPE_CPXFMT_ADJACENT is not used, but is required in a full TYPE constant
						break;
					}

					case 2:
					{
						//	a class character, and an appended 'x' indicates complex
						c = attrDataML[0];
						if (attrDataML[1] != 'x' && attrDataML[1] != 'y')
							berr << E_DATAML << "malformed class \"" << attrDataML << "\" during parse of " << getNodeNoun();
						cpx = TYPE_COMPLEX | ( (attrDataML[1] == 'x') ? TYPE_CPXFMT_ADJACENT : TYPE_CPXFMT_INTERLEAVED );
						break;
					}

					default:
					{
						berr << E_DATAML << "malformed class \"" << attrDataML << "\" during parse of " << getNodeNoun();
					}
				}

				switch(c)
				{
					case 'd': cache.type = TYPE_DOUBLE; break;
					case 'f': cache.type = TYPE_SINGLE; break;

					case 'v': cache.type = TYPE_UINT64; break;
					case 'u': cache.type = TYPE_UINT32; break;
					case 't': cache.type = TYPE_UINT16; break;
					case 's': cache.type = TYPE_UINT8; break;

					case 'p': cache.type = TYPE_INT64; break;
					case 'o': cache.type = TYPE_INT32; break;
					case 'n': cache.type = TYPE_INT16; break;
					case 'm': cache.type = TYPE_INT8; break;

					case 'l': cache.type = TYPE_BOOL8; break;
					case 'c': cache.type = TYPE_CHAR16; break;

					case 'y': cache.type = TYPE_CELL; break;
					case 'z': cache.type = TYPE_STRUCT; break;

					default: berr << E_DATAML << "malformed class \"" << attrDataML << "\" during parse of " << getNodeNoun();
				}

				//	bitwise or with complexity
				cache.type |= cpx;
			}

			else
			{
				//	type is simple string
				cache.type = TYPE_CHAR16;
				flags |= F_STRING_FMT; // so we know to read it differently!

				//	this isn't used, but should be included for consistency
				cache.type |= TYPE_CPXFMT_ADJACENT;
			}

			//	assume column-major, for now
			cache.type |= TYPE_ORDER_COLUMN_MAJOR;

			//	extract dimensions
			const char* attrDims = xmlNode->getAttributeOrNull("b");
			if (attrDims)
			{
				cache.dims.clear();

				while(*attrDims)
				{
					//	advance to find space or null
					const char* e = attrDims;
					while(*e && *e != ' ') e++;

					//	scan value
					UINT32 dim;
					if (!(std::istringstream(attrDims) >> dim))
						berr << E_DATAML << "malformed dims \"" << attrDims << "\" during parse of " << getNodeNoun();
					cache.dims.push_back(dim);

					//	null ends it
					if (!*e) break;

					//	otherwise, was a space, so advance
					attrDims = e + 1;
				}
			}
			else if (flags & F_STRING_FMT)
			{
				//	dims are given by the actual content string
				cache.dims = Dims(1, std::string(text).length());
			}
			else cache.dims = Dims(1); // absent dims attribute means scalar in DataML

			//	set number of elements
			if (cache.dims.size())
			{
				cache.numberOfRealElements = cache.dims.getNumberOfElements();
			}
			else cache.numberOfRealElements = 0;
		}





	////////////////////////////////////////////////////////////////
	////	MODE CHANGES

		DataMLNode& set(UINT32 flag)
		{
			//	must be only recognised user flags
			UINT32 userFlags = F_STRICT | F_REAL | F_ENCAPSULATED;
			if (flag != (flag & userFlags)) berr << E_DATAML << "invalid flag";

			flags |= flag;
			return *this;
		}

		DataMLNode& unset(UINT32 flag)
		{
			//	must be only recognised user flags
			UINT32 userFlags = F_STRICT | F_REAL | F_ENCAPSULATED;
			if (flag != (flag & userFlags)) berr << E_DATAML << "invalid flag";

			flags &= (~flag);
			return *this;
		}

		DataMLNode& precision(INT32 prec)
		{
			if ((prec < 0 || prec > 31) && (prec != PRECISION_NOT_SET))
				berr << E_DATAML << "invalid precision";

			this->prec = prec;
			return *this;
		}






	////////////////////////////////////////////////////////////////
	////	HELPERS

#define CHECK_ASSIGNED if (!xmlNode) berr << E_DATAML << "use of DataMLNode interface before assignment to a physical XML node"

		void setRootTags()
		{
			CHECK_ASSIGNED;

			//	root of a DataML document should declare itself as such
			xmlNode->setAttribute("Format", "DataML");
			xmlNode->setAttribute("Version", DATAML_VERSION);
			if (prec != PRECISION_NOT_SET)
				xmlNode->setAttribute("Precision", ____N2S(prec).c_str());
			xmlNode->setAttribute("AuthTool", ("BRAHMS Binding " + ____N2S(BRAHMS_BINDING_NAME)).c_str());
			xmlNode->setAttribute("AuthToolVersion", ____N2S(BRAHMS_BINDING_REVISION).c_str());
		}





	////////////////////////////////////////////////////////////////
	////	STRUCTURE ACCESSORS

		TYPE getType() const
		{
			CHECK_ASSIGNED;

			return cache.type;
		}

		TYPE getElementType() const
		{
			CHECK_ASSIGNED;

			return cache.type & TYPE_ELEMENT_MASK;
		}

		bool getComplexity() const
		{
			CHECK_ASSIGNED;

			//	handy shortcut function that returns complexity as a straight boolean
			return (cache.type & TYPE_COMPLEX_MASK) == TYPE_COMPLEX;
		}

		Dims getDims() const
		{
			CHECK_ASSIGNED;

			return cache.dims;
		}

		UINT32 getBytesPerElement() const
		{
			CHECK_ASSIGNED;

			return TYPE_BYTES(cache.type);
		}

		UINT64 getNumberOfElementsReal() const
		{
			CHECK_ASSIGNED;

			return cache.numberOfRealElements;
		}

		UINT64 getNumberOfElementsTotal() const
		{
			CHECK_ASSIGNED;

			return cache.numberOfRealElements * TYPE_COMPLEX_MULT(cache.type);
		}

		UINT64 getNumberOfBytesReal() const
		{
			CHECK_ASSIGNED;

			return cache.numberOfRealElements * TYPE_BYTES(cache.type);
		}

		UINT64 getNumberOfBytesTotal() const
		{
			CHECK_ASSIGNED;

			return cache.numberOfRealElements * TYPE_COMPLEX_MULT(cache.type) * TYPE_BYTES(cache.type);
		}





	////////////////////////////////////////////////////////////////
	////	STRUCTURE VALIDATION

		DataMLNode& validate(TYPE type, const Dims& dims)
		{
			CHECK_ASSIGNED;

			//	verify dims
			validate (dims);

			//	verify type
			validate(type);

			//	return this
			return *this;
		}

		DataMLNode& validate(const Dims& p_reqDims)
		{
			CHECK_ASSIGNED;

			//	copy dims
			Dims reqDims = p_reqDims;

			//	remove ellipsis if present
			bool ellipsis = (reqDims.size() && reqDims.at(reqDims.size() - 1) == DIM_ELLIPSIS);
			if (ellipsis) reqDims.pop_back();

			//	get copy of our dimensions
			Dims copyOfOurDims = cache.dims;

			//	expand trailing scalar dimensions
			while (copyOfOurDims.size() < reqDims.size() && (reqDims.at(copyOfOurDims.size()) == 1 || reqDims.at(copyOfOurDims.size()) == DIM_NONZERO || reqDims.at(copyOfOurDims.size()) == DIM_ANY))
				copyOfOurDims.push_back(1);

			//	check size
			if (copyOfOurDims.size() != reqDims.size())
			{
				if (ellipsis)
				{
					if (copyOfOurDims.size() < reqDims.size())
						berr << E_DATAML << "expected " << getNodeNoun() << " to have at least "
							<< reqDims.size() << " dimensions";
				}
				else
				{
					berr << E_DATAML << "expected " << getNodeNoun() << " to have "
						<< reqDims.size() << " dimensions";
				}
			}

			//	verify dims
			for (UINT32 d=0; d<reqDims.size(); d++)
			{
				if (reqDims.at(d) >= 0)
				{
					if (reqDims.at(d) != copyOfOurDims.at(d))
					{
						berr << E_DATAML << "expected " << getNodeNoun() << " to have "
							<< ordinal(d+1) << " dimension of " << reqDims.at(d);
					}
				}

				else if (reqDims.at(d) == DIM_NONZERO)
				{
					if (copyOfOurDims.at(d) == 0)
					{
						berr << E_DATAML << "expected " << getNodeNoun() << " to have " +
						ordinal(d+1) + " dimension non-zero";
					}
				}

				else if (reqDims.at(d) == DIM_ANY) { /* no check */ }

				else
				{
					berr << E_DATAML << "Dims array invalid, found value \"" << reqDims.at(d) << "\"";
				}
			}

			//	return this
			return *this;
		}

		DataMLNode& validate(TYPE type)
		{
			CHECK_ASSIGNED;

			//	verify element type
			if (type & TYPE_ELEMENT_MASK)
			{
				if ((cache.type & TYPE_ELEMENT_MASK) != (type & TYPE_ELEMENT_MASK))
				{
					berr << getNodeNoun() <<
						" is not \"" << getElementTypeString(type) << "\" type";
				}
			}

			//	check complexity
			if ((type & TYPE_COMPLEX_MASK) && ((type & TYPE_COMPLEX_MASK) != (cache.type & TYPE_COMPLEX_MASK)))
			{
				switch (type & TYPE_COMPLEX_MASK)
				{
					case TYPE_REAL:
					{
						berr << getNodeNoun() << "should be real";
					}

					case TYPE_COMPLEX:
					{
						berr << getNodeNoun() << "should be complex";
					}

					default: berr << E_INTERNAL << "type constant invalid";
				}
			}

			//	if TYPE_COMPLEX, unset F_REAL flag, if set
			if (type & TYPE_COMPLEX) unset(F_REAL);

			//	check complex storage format
			if ((type & TYPE_CPXFMT_MASK) && ((type & TYPE_CPXFMT_MASK) != (cache.type & TYPE_CPXFMT_MASK)))
			{
				switch (type & TYPE_CPXFMT_MASK)
				{
					case TYPE_CPXFMT_ADJACENT:
					{
						berr << getNodeNoun() << "should be complex-adjacent format";
					}

					case TYPE_CPXFMT_INTERLEAVED:
					{
						berr << getNodeNoun() << "should be complex-interleaved format";
					}

					default: berr << E_INTERNAL << "type constant invalid";
				}
			}

			//	verify order storage format
			if ((type & TYPE_ORDER_MASK) && ((type & TYPE_ORDER_MASK) != (cache.type & TYPE_ORDER_MASK)))
			{
				switch (type & TYPE_ORDER_MASK)
				{
					case TYPE_ORDER_COLUMN_MAJOR:
					{
						berr << getNodeNoun() << "should be column-major";
					}

					case TYPE_ORDER_ROW_MAJOR:
					{
						berr << getNodeNoun() << "should be row-major";
					}
				}
			}

			//	return this
			return *this;
		}







	////////////////////////////////////////////////////////////////
	////	STRUCT ARRAY INTERFACE

		bool isStruct() const
		{
			CHECK_ASSIGNED;

			return ((cache.type & TYPE_ELEMENT_MASK) == TYPE_STRUCT);
		}

		bool hasField(const char* tagName) const
		{
			CHECK_ASSIGNED;
			ENSURE_STRUCT;

			std::string fns = ";" + std::string(xmlNode->getAttribute("a"));
			size_t pos = fns.find(";" + std::string(tagName) + ";");
			return pos != std::string::npos;
		}

		VSTRING getFieldNames() const
		{
			CHECK_ASSIGNED;
			ENSURE_STRUCT;

			//	get field names as a VSTRING
			std::string fns = xmlNode->getAttribute("a");
			VSTRING fieldNames;
			while(fns.length())
			{
				size_t pos = fns.find(";");
				if (pos == std::string::npos) berr << E_DATAML << "error in \"fieldnames\" attribute";
				fieldNames.push_back(fns.substr(0, pos));
				fns = fns.substr(pos+1);
			}
			return fieldNames;
		}

		DataMLNode getField(const char* tagName, UINT32 index = 0) const
		{
			CHECK_ASSIGNED;
			ENSURE_STRUCT;

			//	convert field name into index
			VSTRING fn = getFieldNames();
			UINT32 nf = fn.size();
			UINT32 n = 0;
			for (n=0; n<nf; n++)
				if (fn[n] == tagName) break;
			if (n == nf) berr << E_DATAML << "field \"" << tagName << "\" not found in " << getNodeNoun();

			//	check index is valid
			if (index >= cache.numberOfRealElements) berr << E_DATAML << "index out of range in " << getNodeNoun();

			//	merge these indices and pull out the XML tag
			UINT32 mindex = index * nf + n;
			XMLNode* xmlFieldNode = xmlNode->getChildOrNull("m", mindex);
			if (!xmlFieldNode) berr << E_INTERNAL;

			//	return an interface to that node
			DataMLNode ret(xmlFieldNode);

			//	which is named appropriately
			if (index) ret.name = name + "(" + ____N2S(index + 1) + ")." + tagName;
			else ret.name = name + "." + tagName;

			//	ok
			return ret;
		}

		DataMLNode addField(const char* tagName)
		{
			CHECK_ASSIGNED;
			ENSURE_STRUCT;

			//	for now, we only allow this if the structure is scalar, cos it's just easier
			if (cache.numberOfRealElements != 1) berr << E_DATAML << "cannot add field to non-scalar struct " << getNodeNoun();

			//	check it's not already present
			VSTRING fn = getFieldNames();
			for (UINT32 n=0; n<fn.size(); n++)
				if (fn[n] == tagName) berr << E_DATAML << "field \"" << tagName << "\" already exists in " << getNodeNoun();

			//	update struct field names attribute
			fn.push_back(tagName);
			std::string sfn;
			for (UINT32 f=0; f<fn.size(); f++)
				sfn += fn[f] + ";";
			xmlNode->setAttribute("a", sfn.c_str());

			//	add new child
			XMLNode* newChild = xmlNode->appendChild(new XMLNode("m"));

			//	return interface to new child
			DataMLNode ret(newChild);

			//	which is named appropriately
			ret.name = name + "." + tagName;

			//	ok
			return ret;
		}

		void becomeStruct(const Dims& dims, std::vector<std::string>& fieldNames)
		{
			CHECK_ASSIGNED;

			//	set cache data
			cache.dims = dims;
			cache.numberOfRealElements = dims.getNumberOfElements();
			cache.type = TYPE_STRUCT;

			//	lay in new data
			xmlNode->clear();
			if (cache.numberOfRealElements != 1) xmlNode->setAttribute("b", std::string(dims).c_str());
			xmlNode->setAttribute("c", "z");
			std::string fn;
			for (UINT32 f=0; f<fieldNames.size(); f++)
				fn += fieldNames[f] + ";";
			xmlNode->setAttribute("a", fn.c_str());

			//	generate children
			UINT32 count = cache.numberOfRealElements * fieldNames.size();
			for (UINT32 c=0; c<count; c++)
			{
				//	BUG FOUND BY valgrind!
				//	OLD CODE: this leaks memory, since the XMLNode object is never destroyed. I could
				//	have fixed this by adding a delete line, but i figured safer just to be explicit...
				//xmlNode->appendChild(new XMLNode("m"));

				Symbol childElement = ____SUCCESS(xml_createElement("m"));
				____SUCCESS(xml_appendChild(xmlNode->element, childElement));
			}
		}





	////////////////////////////////////////////////////////////////
	////	CELL ARRAY INTERFACE

		bool isCellArray() const
		{
			CHECK_ASSIGNED;

			return ((cache.type & TYPE_ELEMENT_MASK) == TYPE_CELL);
		}

		DataMLNode getCell(UINT32 index) const
		{
			CHECK_ASSIGNED;

			ENSURE_CELL;
			if (index >= cache.numberOfRealElements) berr << E_DATAML << "index out of range";
			XMLNode* xmlCellNode = xmlNode->getChildOrNull(NULL, index);
			if (!xmlCellNode) berr << E_INTERNAL << "node not present";

			DataMLNode ret(xmlCellNode);
			ret.name = name + "{" + ____N2S(index + 1) + "}";
			return ret;
		}

		void becomeCell(const Dims& dims)
		{
			CHECK_ASSIGNED;

			//	set cache data, in case someone reads it back
			cache.dims = dims;
			cache.numberOfRealElements = dims.getNumberOfElements();
			cache.type = TYPE_CELL;

			//	lay new data into wrapped XML node (clear its state completely, losing any children it may have had)
			xmlNode->clear();
			if (cache.numberOfRealElements != 1) xmlNode->setAttribute("b", std::string(dims).c_str());
			xmlNode->setAttribute("c", "y");

			//	generate children
			for (UINT32 c=0; c<cache.numberOfRealElements; c++)
				xmlNode->appendChild(new XMLNode("m"));
		}





	////////////////////////////////////////////////////////////////
	////	STRING INTERFACE

		std::string getSTRING()
		{
			CHECK_ASSIGNED;

			//	verify
			bool isChar = (cache.type & TYPE_ELEMENT_MASK) == TYPE_CHAR16;
			bool isRow = cache.dims.size() == 2 && cache.dims[0] == 1;
			bool isEmpty = getNumberOfElementsTotal() == 0;
			if (!isChar || (!isRow && !isEmpty))
				berr << getNodeNoun() << " is not a string (row vector char array)";

			//	read char16 data
			VCHAR16 v;
			if (flags & F_STRING_FMT)
			{
				//	string is stored in raw text format, so the read protocol is different
				v.resize(cache.dims.at(1));
				const char* text = xmlNode->nodeText();
				for (UINT32 c=0; c<cache.dims.at(1); c++) v[c] = text[c];
			}
			else getArray(v);

			//	convert and return it (only handles ascii)
			std::string ret;
			for (UINT32 c=0; c<v.size(); c++)
			{
				CHAR16 ch = v[c];
				if (ch >=128) berr << E_DATAML << "string in " << getNodeNoun() << " contains out-of-range characters (access it as a CHAR16 array instead)";
				ret.push_back((char)ch);
			}
			return ret;
		}

		void setSTRING(const char* text)
		{
			CHECK_ASSIGNED;

			//	set cache data, in case someone reads it back
			UINT32 L = 0;
			while(text[L]) L++;
			cache.dims = Dims(1, L);
			cache.numberOfRealElements = L;
			cache.type = TYPE_CHAR16 | TYPE_REAL;

			//	assume ensure real, not strict, and precision not set
			flags |= F_STRING_FMT;

			//	lay new data into wrapped XML node (clear its state completely, losing any children it may have had)
			xmlNode->clear();
			xmlNode->nodeText(text);
		}





	////////////////////////////////////////////////////////////////
	////	RAW INTERFACE (INC. FUNCTIONS FOR READ AND WRITE TO TEXT)

		void parse_error(const char* p_text, const char* whilst) const
		{
			UINT32 N = 0;
			while(p_text[N] != ' ' && p_text[N]) N++;
			std::string offender(p_text, N);
			berr << E_DATAML << "whilst parsing " << whilst << ", encountered " << offender;
		}

		template <class T> const char* textToNumericData(T* p_arr, const char* p_text, UINT64 N, const char* nodeName) const
		{
			//	prepare type-specific infinity and nan
			T inf = 0;
			T nan = 0;
			if (std::numeric_limits<T>::has_infinity)
				inf = std::numeric_limits<T>::infinity();
			if (std::numeric_limits<T>::has_quiet_NaN)
				nan = std::numeric_limits<T>::quiet_NaN();

			//	prepare working text pointer
			const char* p_next = p_text;

			//	do the parse and fill
			for (UINT64 n=0; n<N; n++)
			{
				/*

				OLD CODE USES atof()

				//	scan to first space
				if (!*p_next) berr << E_DATAML << "overflow during parse of node \"" << nodeName
					<< "\" (" << i << " of " << N << ")";
				p_next = p_text;
				while(*p_next && *p_next != ' ') p_next++;

				//	scan a number
				if ((p_next - p_text) == 3 && p_text[0] == 'I' && p_text[1] == 'n' && p_text[2] == 'f')
					p_arr[i] = inf;
				else if ((p_next - p_text) == 4 && p_text[0] == '-' && p_text[1] == 'I' && p_text[2] == 'n' && p_text[3] == 'f')
					p_arr[i] = ninf;
				else if ((p_next - p_text) == 3 && p_text[0] == 'N' && p_text[1] == 'a' && p_text[2] == 'N')
					p_arr[i] = nan;
				else if ((p_next - p_text) == 4 && p_text[0] == '-' && p_text[1] == 'N' && p_text[2] == 'a' && p_text[3] == 'N')
					p_arr[i] = nnan;
				else p_arr[i] = (T)atof(p_text);

				//	advance
				p_text = p_next + 1;

				NEW CODE USES custom parsing, that is faster because we don't have
				to handle so many cases as atof() tries to...

				*/

				double neg = 1.0;
				double val = 0.0;

				//	collect -ve
				if (*p_next == '-')
				{
					neg = -1.0;
					p_next++;
				}

				//	collect +ve
				else if (*p_next == '+')
				{
					p_next++;
				}

				//	collect Inf
				if (*p_next == 'I')
				{
					p_next++;
					if (*p_next != 'n')
						parse_error(p_text, "Inf");
					p_next++;
					if (*p_next != 'f')
						parse_error(p_text, "Inf");
					p_next++;
					if (*p_next) // could be last, so no trailing space
					{
						if (*p_next != ' ')
							parse_error(p_text, "Inf");
						p_next++;
					}
					p_text = p_next;
					p_arr[n] = (T)(neg * inf);
					continue;
				}

				//	collect NaN
				if (*p_next == 'N')
				{
					p_next++;
					if (*p_next != 'a')
						parse_error(p_text, "NaN");
					p_next++;
					if (*p_next != 'N')
						parse_error(p_text, "NaN");
					p_next++;
					if (*p_next) // could be last, so no trailing space
					{
						if (*p_next != ' ')
							parse_error(p_text, "NaN");
						p_next++;
					}
					p_text = p_next;
					p_arr[n] = (T)(neg * nan);
					continue;
				}

				//	collect integer
				if (*p_next >= '0' && *p_next <= '9')
				{
					UINT64 num = *p_next - '0';
					p_next++;

					//	have to stop collecting before the UINT64 container overflows: we can't represent additional precision anyway
					while (*p_next >= '0' && *p_next <= '9')
					{
						if (num <= SUB_MAX_UINT64)
						{
							num *= 10;
							num += *p_next - '0';
						}
						p_next++;
					}

					val = num;
				}

				//	collect fraction
				if (*p_next == '.')
				{
					p_next++;
					if (!(*p_next >= '0' && *p_next <= '9'))
						parse_error(p_text, "fraction");

					UINT64 num = *p_next - '0';
					p_next++;
					double divisor = 10.0;

					//	have to stop collecting before the UINT64 container overflows: we can't represent additional precision anyway
					while (*p_next >= '0' && *p_next <= '9')
					{
						if (num <= SUB_MAX_UINT64)
						{
							num *= 10;
							num += *p_next - '0';
							divisor *= 10.0;
						}
						p_next++;
					}

					val += ((double)num) / divisor;
				}

				//	collect exponent
				if (*p_next == 'E' || *p_next == 'e')
				{
					p_next++;
					double eneg = 1.0;

					if (*p_next == '-')
					{
						eneg = -1.0;
						p_next++;
					}

					else if (*p_next == '+')
					{
						p_next++;
					}

					if (!(*p_next >= '0' && *p_next <= '9'))
						parse_error(p_text, "exponent");

					UINT64 num = *p_next - '0';
					p_next++;

					while (*p_next >= '0' && *p_next <= '9')
					{
						num *= 10;
						num += *p_next - '0';
						p_next++;
					}

					val *= std::pow(10.0, eneg * ((double)num));
				}

				//	skip final space
				if (*p_next != ' ')
				{
					//	allowed to be missing after final number...
					if (n != (N - 1))
						parse_error(p_text, "trailing whitespace");
				}
				else p_next++;

				//	load output array and advance p_text
				p_arr[n] = (T)(val * neg);
				p_text = p_next;
			}

			//	return where we got to in the text
			return p_text;
		}

		void getRaw(BYTE* p_real, BYTE* p_imag = NULL) const
		{
			CHECK_ASSIGNED;

			if (p_imag && (cache.type & TYPE_REAL))
				berr << E_DATAML << "attempt to read complex data from a non-complex array";

			if (!p_imag && (cache.type & TYPE_COMPLEX))
				berr << E_DATAML << "attempt to read non-complex data from a complex array";

			//	handle special case
			if (flags & F_STRING_FMT)
			{
				//	string is stored in raw text format, so the read protocol is different
				const char* text = xmlNode->nodeText();
				CHAR16* p_text = (CHAR16*)p_real;
				for (UINT32 c=0; c<cache.dims.at(1); c++) p_text[c] = text[c];
				return;
			}

			//	handle special case
			if (flags & F_BINARY_FILE_FMT)
			{
				//	string is filename of binary file that holds actual data
				std::string filename = xmlNode->nodeText();
				UINT64 N = getNumberOfBytesTotal();

				//	if filename is relative, read it from the SupplementaryFilePath
#ifdef __WIN__
				if (filename.length() >= 2 && filename.substr(1, 1) == ":")
#endif
#ifdef __NIX__
				if (filename.length() >= 1 && filename.substr(0, 1) == "/")
#endif
				{
					//	absolute pathname - leave it be
				}
				else
				{
					XMLNode pars(executionInfo->executionParameters);
					filename = pars.getChild("SupplementaryFilePath")->nodeText() + ("/" + filename);
				}

				//	open it and check the size
				std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
				if (!file) berr << E_DATAML << "error opening \"" << filename << "\"";
				file.seekg( 0, std::ios::end );
				UINT64 fileBytes = file.tellg();
				file.seekg( 0 );
				if (N != fileBytes)
				{
					file.close();
					berr << E_DATAML << "file \"" << filename << "\" should be " << N << " bytes (was " << fileBytes << ")";
				}

				//	fill it - the data will be interleaved real/complex/real/complex
				//	at the granularity of the last dimension, so we need to know
				//	what the last dimension is, and what the product of the other
				//	dimensions is, for the chunk size
				if (cache.dims.size())
				{
					UINT32 lastDim = cache.dims.back();
					UINT32 chunkBytes = TYPE_BYTES(cache.type);
					for (UINT32 d=0; d<(cache.dims.size()-1); d++)
						chunkBytes *= cache.dims[d];
					if (chunkBytes && lastDim)
					{
						for (UINT32 chunk=0; chunk<lastDim; chunk++)
						{
							if (p_real)
							{
								file.read((char*)p_real, chunkBytes);
								p_real += chunkBytes;
							}
							if (p_imag)
							{
								file.read((char*)p_imag, chunkBytes);
								p_imag += chunkBytes;
							}
						}
					}
				}

				//	close it
				file.close();

				//	ok
				return;
			}

			//	only valid for numeric types
			TYPE format = cache.type & TYPE_FORMAT_MASK;
			if ( ! (format == TYPE_FORMAT_BOOL || format == TYPE_FORMAT_UINT || format == TYPE_FORMAT_INT || format == TYPE_FORMAT_FLOAT || format == TYPE_FORMAT_CHAR) )
				berr << E_DATAML << "cannot fill from DataML node with data format \"" << getElementTypeString(cache.type) << "\"";

			//	get vital statistics
//			UINT32 bytesPerElement = TYPE_BYTES(cache.type);
			UINT64 N = getNumberOfElementsReal();

			//	prepare DOUBLE-type infinity and nan
//			DOUBLE inf = std::numeric_limits<DOUBLE>::infinity();
	//		DOUBLE ninf = -1.0 * inf;
	//		DOUBLE nan = std::numeric_limits<DOUBLE>::quiet_NaN();
	//		DOUBLE nnan = -1.0 * nan;

			//	get pointer to text in XML node
			const char* p_text = xmlNode->nodeText();

			//	get element type
			TYPE elementType = cache.type & TYPE_ELEMENT_MASK;
			UINT32 nPhases = cache.type & TYPE_COMPLEX ? 2 : 1;

			//	do real then imag numbers
			for (UINT32 phase=0; phase<nPhases; phase++)
			{
				//	select real or imag as target (if imag is NULL, *all* data goes into first pointer, including imag)
				BYTE* p_arr = phase == 1 ? (p_imag ? p_imag : p_real + getNumberOfBytesReal()) : p_real;

				//	farm out filling to the template
				switch(elementType)
				{
				case TYPE_SINGLE: p_text = textToNumericData((SINGLE*)p_arr, p_text, N, xmlNode->nodeName()); break;
				case TYPE_DOUBLE: p_text = textToNumericData((DOUBLE*)p_arr, p_text, N, xmlNode->nodeName()); break;
				case TYPE_UINT64: p_text = textToNumericData((UINT64*)p_arr, p_text, N, xmlNode->nodeName()); break;
				case TYPE_UINT32: p_text = textToNumericData((UINT32*)p_arr, p_text, N, xmlNode->nodeName()); break;
				case TYPE_UINT16: p_text = textToNumericData((UINT16*)p_arr, p_text, N, xmlNode->nodeName()); break;
				case TYPE_UINT8:  p_text = textToNumericData((UINT8 *)p_arr, p_text, N, xmlNode->nodeName()); break;
				case TYPE_INT64:  p_text = textToNumericData((INT64 *)p_arr, p_text, N, xmlNode->nodeName()); break;
				case TYPE_INT32:  p_text = textToNumericData((INT32 *)p_arr, p_text, N, xmlNode->nodeName()); break;
				case TYPE_INT16:  p_text = textToNumericData((INT16 *)p_arr, p_text, N, xmlNode->nodeName()); break;
				case TYPE_INT8:   p_text = textToNumericData((INT8  *)p_arr, p_text, N, xmlNode->nodeName()); break;
				case TYPE_BOOL8:  p_text = textToNumericData((BOOL8 *)p_arr, p_text, N, xmlNode->nodeName()); break;
				default:
					berr << E_DATAML << "case not coded";
				}
			}

			//	assert end condition
			if (*p_text) berr << E_DATAML << "content scan error during parse of " << getNodeNoun() << " (no NULL at scan complete)";
		}

		template <class T> void numericDataToText(T* p1, T* p2, std::string& str, UINT64 numEls, const char* fmt, INT32 prec = PRECISION_NOT_SET)
		{
			//	prepare buffer
			const UINT32 BUFFERSIZE = 16384;
			char buffer[BUFFERSIZE];
			buffer[0] = 0; // necessary!
			char* p_buffer = buffer;

			//	prepare fmt
			std::string sfmtx = "%";
			if (prec == PRECISION_NOT_SET)
			{
				if (fmt[0] == 'g') // is a float format, have to force max precision
					sfmtx += ".20";
			}
			else
			{
				std::stringstream event;
				event << prec;
				sfmtx += "." + event.str();
			}
			sfmtx += fmt;
			sfmtx += " ";
			const char* fmtx = sfmtx.c_str();

			//	for each
			for (UINT32 i=0; i<2; i++)
			{
				//	initialise pointers
				T* p_array = i ? p2 : p1;
				T* p_stop = p_array + numEls;

				//	loop over array
				while(p_array < p_stop)
				{
					//	add to buffer and advance buffer pointer
		#ifdef _MSC_VER
					//	new security in CRT
					sprintf_s(p_buffer, 30, fmtx, *p_array);
		#else
					sprintf(p_buffer, fmtx, *p_array);
		#endif
					while(*p_buffer) p_buffer++;

					// flush buffer if approaching full
					if ((p_buffer - buffer) > ((int)(BUFFERSIZE - 32)))
					{
						str += buffer;
						p_buffer = buffer;
						*p_buffer = 0;
					}

					//	advance to next array element
					p_array++;
				}

				//	if no complex, don't do second loop
				if (!p2) break;
			}

			//	flush buffer one last time
			str += buffer;

			//	remove trailing space
			if (str.size()) str.erase(str.end() - 1);

			//	convert IEEE representations to DataML representations
			size_t from = 0;
			while(true)
			{
				size_t next = str.find("1.#INF ", from);
				if (next == std::string::npos) break;
				str.replace(next, 7, "Inf    ");
			}
			while(true)
			{
				size_t next = str.find("1.#QNAN ", from);
				if (next == std::string::npos) break;
				str.replace(next, 8, "NaN     ");
			}

			//	-1.#IND (indeterminate) is achieved if we try to pass in a NaN through
			//	a binary file. i have no idea why this comes out differently, but one
			//	comes direct from matlab's binary representation, and the other comes
			//	via matlab's sprintf and c++'s atof, so something is lost in translation.
			while(true)
			{
				size_t next = str.find("-1.#IND ", from);
				if (next == std::string::npos) break;
				str.replace(next, 8, "NaN     ");
			}
		}

		const char* datamlString(TYPE type)
		{
			switch(type & TYPE_ELEMENT_MASK)
			{
				case TYPE_DOUBLE: return "d";
				case TYPE_SINGLE: return "f";
				case TYPE_UINT64: return "v";
				case TYPE_UINT32: return "u";
				case TYPE_UINT16: return "t";
				case TYPE_UINT8 : return "s";
				case TYPE_INT64 : return "p";
				case TYPE_INT32 : return "o";
				case TYPE_INT16 : return "n";
				case TYPE_INT8  : return "m";
				case TYPE_BOOL8 : return "l";
				default: berr << E_DATAML << "invalid DataML type \"" << std::hex << type << "\"";
			}
			
			return NULL;
		}

		void setArrayStructure(DataMLNode* node, TYPE type, const Dims& dims)
		{
			CHECK_ASSIGNED;

			//	clear node
			node->xmlNode->clear();

			//	set cache data, in case someone reads it back
			node->cache.type = type;
			node->cache.dims = dims;
			node->cache.numberOfRealElements = dims.getNumberOfElements();

			//	set class attribute
			switch (type & (TYPE_COMPLEX_MASK | TYPE_CPXFMT_MASK))
			{
				case TYPE_REAL | TYPE_CPXFMT_ADJACENT:
				case TYPE_REAL | TYPE_CPXFMT_INTERLEAVED:
					xmlNode->setAttribute("c", datamlString(type));
					break;

				case TYPE_COMPLEX | TYPE_CPXFMT_ADJACENT:
					xmlNode->setAttribute("c", (datamlString(type) + std::string("x")).c_str());
					break;

				case TYPE_COMPLEX | TYPE_CPXFMT_INTERLEAVED:
					xmlNode->setAttribute("c", (datamlString(type) + std::string("y")).c_str());
					break;

				default:
					berr << E_DATAML << "must specify complexity";
			}

			//	set dims attribute
			if (node->cache.numberOfRealElements != 1)
			{
				std::string b(dims);
				node->xmlNode->setAttribute("b", b.c_str());
			}
		}

		void setRaw(const Dims& dims, TYPE type, const void* p_real, const void* p_imag)
		{
			//	validate data is supplied
			if (dims.getNumberOfElements())
			{
				if (!p_real) berr << E_DATAML << "must supply real data";
				if ((!(type & TYPE_COMPLEX)) && p_imag) berr << E_DATAML << "must not supply imaginary data";
			}

			setArrayStructure(this, type, dims);

			std::string tempNodeTextString;

			//	data must either be split between the two pointers, or all in p_real
			UINT32 mult = ((type & TYPE_COMPLEX) && !p_imag) ? 2 : 1;

			//	doesn't make sense for it to be split if it's interleaved
			if (p_imag && (cache.type & TYPE_CPXFMT_INTERLEAVED))
				berr << E_DATAML << "interleaved data supplied in two blocks is not correct usage";

			switch(type & TYPE_ELEMENT_MASK)
			{
				case TYPE_DOUBLE:
					numericDataToText((DOUBLE*)p_real, (DOUBLE*)p_imag, tempNodeTextString, cache.numberOfRealElements * mult, "g", prec);
					break;
				case TYPE_SINGLE:
					numericDataToText((SINGLE*)p_real, (SINGLE*)p_imag, tempNodeTextString, cache.numberOfRealElements * mult, "g", prec);
					break;
				case TYPE_UINT64:
					numericDataToText((UINT64*)p_real, (UINT64*)p_imag, tempNodeTextString, cache.numberOfRealElements * mult, "llu");
					break;
				case TYPE_UINT32:
					numericDataToText((UINT32*)p_real, (UINT32*)p_imag, tempNodeTextString, cache.numberOfRealElements * mult, "u");
					break;
				case TYPE_UINT16:
					numericDataToText((UINT16*)p_real, (UINT16*)p_imag, tempNodeTextString, cache.numberOfRealElements * mult, "u");
					break;
				case TYPE_UINT8:
					numericDataToText((UINT8*)p_real, (UINT8*)p_imag, tempNodeTextString, cache.numberOfRealElements * mult, "u");
					break;
				case TYPE_INT64:
					numericDataToText((INT64*)p_real, (INT64*)p_imag, tempNodeTextString, cache.numberOfRealElements * mult, "lld");
					break;
				case TYPE_INT32:
					numericDataToText((INT32*)p_real, (INT32*)p_imag, tempNodeTextString, cache.numberOfRealElements * mult, "d");
					break;
				case TYPE_INT16:
					numericDataToText((INT16*)p_real, (INT16*)p_imag, tempNodeTextString, cache.numberOfRealElements * mult, "d");
					break;
				case TYPE_INT8:
					numericDataToText((INT8*)p_real, (INT8*)p_imag, tempNodeTextString, cache.numberOfRealElements * mult, "d");
					break;
				case TYPE_BOOL8:
					numericDataToText((BOOL8*)p_real, (BOOL8*)p_imag, tempNodeTextString, cache.numberOfRealElements * mult, "u");
					break;
				default:
					berr << E_DATAML << "case not coded";
			}

			xmlNode->nodeText(tempNodeTextString.c_str());
		}








	////////////////////////////////////////////////////////////////
	////	SET ARRAY INTERFACE

		void setArray(const Dims& dims, const VDOUBLE& src)
		{
			TYPE type = TYPE_DOUBLE | TYPE_DATAML_SETARRARY_ASSUMED;
			UINT64 av = src.size();
			UINT64 nr = dims.getNumberOfElements();
			if (av == nr) setRaw(dims, type | TYPE_REAL, av ? &src[0] : NULL, NULL);
			else if (av == (nr * 2)) setRaw(dims, type | TYPE_COMPLEX, av ? &src[0] : NULL, av ? &src[nr] : NULL);
			else berr << E_INVALID_ARG << "setArray()";
		}

		void setArray(const Dims& dims, const VSINGLE& src)
		{
			TYPE type = TYPE_SINGLE | TYPE_DATAML_SETARRARY_ASSUMED;
			UINT64 av = src.size();
			UINT64 nr = dims.getNumberOfElements();
			if (av == nr) setRaw(dims, type | TYPE_REAL, av ? &src[0] : NULL, NULL);
			else if (av == (nr * 2)) setRaw(dims, type | TYPE_COMPLEX, av ? &src[0] : NULL, av ? &src[nr] : NULL);
			else berr << E_INVALID_ARG << "setArray()";
		}

		void setArray(const Dims& dims, const VUINT64& src)
		{
			TYPE type = TYPE_UINT64 | TYPE_DATAML_SETARRARY_ASSUMED;
			UINT64 av = src.size();
			UINT64 nr = dims.getNumberOfElements();
			if (av == nr) setRaw(dims, type | TYPE_REAL, av ? &src[0] : NULL, NULL);
			else if (av == (nr * 2)) setRaw(dims, type | TYPE_COMPLEX, av ? &src[0] : NULL, av ? &src[nr] : NULL);
			else berr << E_INVALID_ARG << "setArray()";
		}

		void setArray(const Dims& dims, const VUINT32& src)
		{
			TYPE type = TYPE_UINT32 | TYPE_DATAML_SETARRARY_ASSUMED;
			UINT64 av = src.size();
			UINT64 nr = dims.getNumberOfElements();
			if (av == nr) setRaw(dims, type | TYPE_REAL, av ? &src[0] : NULL, NULL);
			else if (av == (nr * 2)) setRaw(dims, type | TYPE_COMPLEX, av ? &src[0] : NULL, av ? &src[nr] : NULL);
			else berr << E_INVALID_ARG << "setArray()";
		}

		void setArray(const Dims& dims, const VUINT16& src)
		{
			TYPE type = TYPE_UINT16 | TYPE_DATAML_SETARRARY_ASSUMED;
			UINT64 av = src.size();
			UINT64 nr = dims.getNumberOfElements();
			if (av == nr) setRaw(dims, type | TYPE_REAL, av ? &src[0] : NULL, NULL);
			else if (av == (nr * 2)) setRaw(dims, type | TYPE_COMPLEX, av ? &src[0] : NULL, av ? &src[nr] : NULL);
			else berr << E_INVALID_ARG << "setArray()";
		}

		void setArray(const Dims& dims, const VUINT8& src)
		{
			TYPE type = TYPE_UINT8 | TYPE_DATAML_SETARRARY_ASSUMED;
			UINT64 av = src.size();
			UINT64 nr = dims.getNumberOfElements();
			if (av == nr) setRaw(dims, type | TYPE_REAL, av ? &src[0] : NULL, NULL);
			else if (av == (nr * 2)) setRaw(dims, type | TYPE_COMPLEX, av ? &src[0] : NULL, av ? &src[nr] : NULL);
			else berr << E_INVALID_ARG << "setArray()";
		}

		void setArray(const Dims& dims, const VINT64& src)
		{
			TYPE type = TYPE_INT64 | TYPE_DATAML_SETARRARY_ASSUMED;
			UINT64 av = src.size();
			UINT64 nr = dims.getNumberOfElements();
			if (av == nr) setRaw(dims, type | TYPE_REAL, av ? &src[0] : NULL, NULL);
			else if (av == (nr * 2)) setRaw(dims, type | TYPE_COMPLEX, av ? &src[0] : NULL, av ? &src[nr] : NULL);
			else berr << E_INVALID_ARG << "setArray()";
		}

		void setArray(const Dims& dims, const VINT32& src)
		{
			TYPE type = TYPE_INT32 | TYPE_DATAML_SETARRARY_ASSUMED;
			UINT64 av = src.size();
			UINT64 nr = dims.getNumberOfElements();
			if (av == nr) setRaw(dims, type | TYPE_REAL, av ? &src[0] : NULL, NULL);
			else if (av == (nr * 2)) setRaw(dims, type | TYPE_COMPLEX, av ? &src[0] : NULL, av ? &src[nr] : NULL);
			else berr << E_INVALID_ARG << "setArray()";
		}

		void setArray(const Dims& dims, const VINT16& src)
		{
			TYPE type = TYPE_INT16 | TYPE_DATAML_SETARRARY_ASSUMED;
			UINT64 av = src.size();
			UINT64 nr = dims.getNumberOfElements();
			if (av == nr) setRaw(dims, type | TYPE_REAL, av ? &src[0] : NULL, NULL);
			else if (av == (nr * 2)) setRaw(dims, type | TYPE_COMPLEX, av ? &src[0] : NULL, av ? &src[nr] : NULL);
			else berr << E_INVALID_ARG << "setArray()";
		}

		void setArray(const Dims& dims, const VINT8& src)
		{
			TYPE type = TYPE_INT8 | TYPE_DATAML_SETARRARY_ASSUMED;
			UINT64 av = src.size();
			UINT64 nr = dims.getNumberOfElements();
			if (av == nr) setRaw(dims, type | TYPE_REAL, av ? &src[0] : NULL, NULL);
			else if (av == (nr * 2)) setRaw(dims, type | TYPE_COMPLEX, av ? &src[0] : NULL, av ? &src[nr] : NULL);
			else berr << E_INVALID_ARG << "setArray()";
		}

		void setArray(const Dims& dims, const Vbool& src)
		{
			TYPE type = TYPE_BOOL8 | TYPE_DATAML_SETARRARY_ASSUMED;
			UINT64 av = src.size();
			UINT64 nr = dims.getNumberOfElements();
			VUINT8 temp;
			temp.insert(temp.begin(), src.begin(), src.end());
			if (av == nr) setRaw(dims, type | TYPE_REAL, av ? &temp[0] : NULL, NULL);
			else if (av == (nr * 2)) setRaw(dims, type | TYPE_COMPLEX, av ? &temp[0] : NULL, av ? &temp[nr] : NULL);
			else berr << E_INVALID_ARG << "setArray()";
		}







	////////////////////////////////////////////////////////////////
	////	LARGE NUMERIC ARRAY INTERFACE

		/*

			this interface will fill the passed array,
			so you need to reserve enough memory: getNumberOfBytesTotal() bytes! if
			p_imag is NULL and there is imaginary data, it is all filled in to p_real,
			so watch out for that! if p_imag is non-NULL and there is no imaginary data
			we throw an exception.

		*/

		//	true if type specifies an acceptable element storage format
		inline bool TYPE_IS_ARRAY(TYPE type)
		{
			return (type & TYPE_FORMAT_MASK) && (type & TYPE_FORMAT_MASK) <= TYPE_FORMAT_CHAR;
		}

		TYPE runTimeType(DOUBLE val) { return TYPE_DOUBLE; }
		TYPE runTimeType(SINGLE val) { return TYPE_SINGLE; }
		TYPE runTimeType(UINT64 val) { return TYPE_UINT64; }
		TYPE runTimeType(UINT32 val) { return TYPE_UINT32; }
		TYPE runTimeType(UINT16 val) { return TYPE_UINT16; }
		TYPE runTimeType(UINT8  val) { return TYPE_UINT8 ; }
		TYPE runTimeType(INT64  val) { return TYPE_INT64; }
		TYPE runTimeType(INT32  val) { return TYPE_INT32; }
		TYPE runTimeType(INT16  val) { return TYPE_INT16; }
		TYPE runTimeType(INT8   val) { return TYPE_INT8 ; }
		TYPE runTimeType(bool   val) { return TYPE_BOOL8 ; }

		template <class T> std::vector<T>& templateGetArray(DataMLNode* node, std::vector<T>& ret)
		{
			CHECK_ASSIGNED;

			//	validate real assumption
			if (node->flags & F_REAL) node->validate(TYPE_REAL);

			//	get type constant for requested type
			T val = 0;
			TYPE type = runTimeType(val);

			//	not ok to continue if it's not an array type
			if (!TYPE_IS_ARRAY(node->cache.type))
				berr << node->getNodeNoun() << " is not an array node in getArray()";

			//	resize return object
			UINT64 N = node->getNumberOfElementsTotal();
			ret.resize(N);

			//	if no elements, we're done
			if (!N) return ret;

			//	if it's the right element type, just call getRaw()
			if (type == (node->cache.type & TYPE_ELEMENT_MASK))
			{
				//	get pointers to data
				T* p_real = &ret[0];
				T* p_imag = node->cache.type & TYPE_COMPLEX ? &ret[node->cache.numberOfRealElements] : NULL;

				//	run fill
				node->getRaw((BYTE*)p_real, (BYTE*)p_imag);
			}

			//	otherwise, try to do a compliant conversion
			else
			{
				//	if compliant has not been allowed...
				if (node->flags & F_STRICT)
					node->validate(type); // this will generate the appropriate error message

				//	do native fill
				switch(node->cache.type & TYPE_ELEMENT_MASK)
				{
					case TYPE_DOUBLE:
					{
						VDOUBLE temp;
						temp.resize(N);
						DOUBLE* p_real = &temp[0];
						DOUBLE* p_imag = node->cache.type & TYPE_COMPLEX ? &temp[node->cache.numberOfRealElements] : NULL;
						node->getRaw((BYTE*)p_real, (BYTE*)p_imag);
						for (UINT64 n=0; n<N; n++)
						{
							ret[n] = (T)temp[n];
							if (((DOUBLE)ret[n]) != temp[n])
								berr << node->getNodeNoun() <<
									" is not \"" << getElementTypeString(type) <<
									"\" type (and was not compliant)";
						}
						break;
					}
					case TYPE_SINGLE:
					{
						VSINGLE temp;
						temp.resize(N);
						SINGLE* p_real = &temp[0];
						SINGLE* p_imag = node->cache.type & TYPE_COMPLEX ? &temp[node->cache.numberOfRealElements] : NULL;
						node->getRaw((BYTE*)p_real, (BYTE*)p_imag);
						for (UINT64 n=0; n<N; n++)
						{
							ret[n] = (T)temp[n];
							if (((SINGLE)ret[n]) != temp[n])
								berr << node->getNodeNoun() <<
									" is not \"" << getElementTypeString(type) <<
									"\" type (and was not compliant)";
						}
						break;
					}
					case TYPE_UINT64:
					{
						VUINT64 temp;
						temp.resize(N);
						UINT64* p_real = &temp[0];
						UINT64* p_imag = node->cache.type & TYPE_COMPLEX ? &temp[node->cache.numberOfRealElements] : NULL;
						node->getRaw((BYTE*)p_real, (BYTE*)p_imag);
						for (UINT64 n=0; n<N; n++)
						{
							ret[n] = (T)temp[n];
							if (((UINT64)ret[n]) != temp[n])
								berr << node->getNodeNoun() <<
									" is not \"" << getElementTypeString(type) <<
									"\" type (and was not compliant)";
						}
						break;
					}
					case TYPE_UINT32:
					{
						VUINT32 temp;
						temp.resize(N);
						UINT32* p_real = &temp[0];
						UINT32* p_imag = node->cache.type & TYPE_COMPLEX ? &temp[node->cache.numberOfRealElements] : NULL;
						node->getRaw((BYTE*)p_real, (BYTE*)p_imag);
						for (UINT64 n=0; n<N; n++)
						{
							ret[n] = (T)temp[n];
							if (((UINT32)ret[n]) != temp[n])
								berr << node->getNodeNoun() <<
									" is not \"" << getElementTypeString(type) <<
									"\" type (and was not compliant)";
						}
						break;
					}
					case TYPE_UINT16:
					{
						VUINT16 temp;
						temp.resize(N);
						UINT16* p_real = &temp[0];
						UINT16* p_imag = node->cache.type & TYPE_COMPLEX ? &temp[node->cache.numberOfRealElements] : NULL;
						node->getRaw((BYTE*)p_real, (BYTE*)p_imag);
						for (UINT64 n=0; n<N; n++)
						{
							ret[n] = (T)temp[n];
							if (((UINT16)ret[n]) != temp[n])
								berr << node->getNodeNoun() <<
									" is not \"" << getElementTypeString(type) <<
									"\" type (and was not compliant)";
						}
						break;
					}
					case TYPE_UINT8:
					{
						VUINT8 temp;
						temp.resize(N);
						UINT8* p_real = &temp[0];
						UINT8* p_imag = node->cache.type & TYPE_COMPLEX ? &temp[node->cache.numberOfRealElements] : NULL;
						node->getRaw((BYTE*)p_real, (BYTE*)p_imag);
						for (UINT64 n=0; n<N; n++)
						{
							ret[n] = (T)temp[n];
							if (((UINT8)ret[n]) != temp[n])
								berr << node->getNodeNoun() <<
									" is not \"" << getElementTypeString(type) <<
									"\" type (and was not compliant)";
						}
						break;
					}
					case TYPE_INT64:
					{
						VINT64 temp;
						temp.resize(N);
						INT64* p_real = &temp[0];
						INT64* p_imag = node->cache.type & TYPE_COMPLEX ? &temp[node->cache.numberOfRealElements] : NULL;
						node->getRaw((BYTE*)p_real, (BYTE*)p_imag);
						for (UINT64 n=0; n<N; n++)
						{
							ret[n] = (T)temp[n];
							if (((INT64)ret[n]) != temp[n])
								berr << node->getNodeNoun() <<
									" is not \"" << getElementTypeString(type) <<
									"\" type (and was not compliant)";
						}
						break;
					}
					case TYPE_INT32:
					{
						VINT32 temp;
						temp.resize(N);
						INT32* p_real = &temp[0];
						INT32* p_imag = node->cache.type & TYPE_COMPLEX ? &temp[node->cache.numberOfRealElements] : NULL;
						node->getRaw((BYTE*)p_real, (BYTE*)p_imag);
						for (UINT64 n=0; n<N; n++)
						{
							ret[n] = (T)temp[n];
							if (((INT32)ret[n]) != temp[n])
								berr << node->getNodeNoun() <<
									" is not \"" << getElementTypeString(type) <<
									"\" type (and was not compliant)";
						}
						break;
					}
					case TYPE_INT16:
					{
						VINT16 temp;
						temp.resize(N);
						INT16* p_real = &temp[0];
						INT16* p_imag = node->cache.type & TYPE_COMPLEX ? &temp[node->cache.numberOfRealElements] : NULL;
						node->getRaw((BYTE*)p_real, (BYTE*)p_imag);
						for (UINT64 n=0; n<N; n++)
						{
							ret[n] = (T)temp[n];
							if (((INT16)ret[n]) != temp[n])
								berr << node->getNodeNoun() <<
									" is not \"" << getElementTypeString(type) <<
									"\" type (and was not compliant)";
						}
						break;
					}
					case TYPE_INT8:
					{
						VINT8 temp;
						temp.resize(N);
						INT8* p_real = &temp[0];
						INT8* p_imag = node->cache.type & TYPE_COMPLEX ? &temp[node->cache.numberOfRealElements] : NULL;
						node->getRaw((BYTE*)p_real, (BYTE*)p_imag);
						for (UINT64 n=0; n<N; n++)
						{
							ret[n] = (T)temp[n];
							if (((INT8)ret[n]) != temp[n])
								berr << node->getNodeNoun() <<
									" is not \"" << getElementTypeString(type) <<
									"\" type (and was not compliant)";
						}
						break;
					}
					case TYPE_BOOL8:
					{
						VBOOL8 temp;
						temp.resize(N);
						BOOL8* p_real = &temp[0];
						BOOL8* p_imag = node->cache.type & TYPE_COMPLEX ? &temp[node->cache.numberOfRealElements] : NULL;
						node->getRaw((BYTE*)p_real, (BYTE*)p_imag);
						for (UINT64 n=0; n<N; n++)
						{
							ret[n] = (T)temp[n];
							if (((BOOL8)ret[n]) != temp[n])
								berr << node->getNodeNoun() <<
									" is not \"" << getElementTypeString(type) <<
									"\" type (and was not compliant)";
						}
						break;
					}
					case TYPE_CHAR16:
					{
						berr << node->getNodeNoun() <<
							" is not \"" << getElementTypeString(type) <<
							"\" type (and character data cannot be automatically converted to numeric data)";
					}
					default:
					{
						berr << E_INTERNAL << "unexpected node type 0x" << std::hex << node->cache.type;
					}
				}
			}

			//	ok
			return ret;
		}

		VDOUBLE& getArray(VDOUBLE& dst)
		{
			return templateGetArray(this, dst);
		}

		VSINGLE& getArray(VSINGLE& dst)
		{
			return templateGetArray(this, dst);
		}

		VUINT64& getArray(VUINT64& dst)
		{
			return templateGetArray(this, dst);
		}

		VUINT32& getArray(VUINT32& dst)
		{
			return templateGetArray(this, dst);
		}

		VUINT16& getArray(VUINT16& dst)
		{
			return templateGetArray(this, dst);
		}

		VUINT8& getArray(VUINT8& dst)
		{
			return templateGetArray(this, dst);
		}

		VINT64& getArray(VINT64& dst)
		{
			return templateGetArray(this, dst);
		}

		VINT32& getArray(VINT32& dst)
		{
			return templateGetArray(this, dst);
		}

		VINT16& getArray(VINT16& dst)
		{
			return templateGetArray(this, dst);
		}

		VINT8& getArray(VINT8& dst)
		{
			return templateGetArray(this, dst);
		}

		Dims& getArray(Dims& dst)
		{
			VINT64 temp;
			templateGetArray(this, temp);
			dst = temp;
			return dst;
		}

		Vbool& getArray(Vbool& dst)
		{
		/*

		In my opinion the problem can be caused by the fact that STL offers a special
		implementation of std::vector< bool >, different from other std::vector< T >,
		based on storing boolean value in a single bit. Therefore you cannot refer these
		bits using "T &".

		In order to solve the problem, try another definition of your function:

			typename std::vector<T>::reference Matrix<T>::element(int row, int column)

			{

			. . .

			}

		Otherwise consider using of a vector of char or short instead of bool.

		I hope this makes sense.

		*/
			VBOOL8 temp;
			templateGetArray(this, temp);
			dst.resize(temp.size());
			for (UINT64 n=0; n<temp.size(); n++)
				dst[n] = temp[n];
			return dst;
		}





	////////////////////////////////////////////////////////////////
	////	SMALL NUMERIC ARRAY INTERFACE

		/*

			This interface creates the STL objects and returns them, causing
			an implicit copy. Therefore, operation will be appreciably slowed
			if the objects are large. For large objects, you can use the large
			array interface, which has marginally less comfortable syntax:

			SMALL: val = getArray...
			LARGE: vector val; getArray(val, ...

		*/

		VDOUBLE getArrayDOUBLE()
		{
			VDOUBLE ret;
			return getArray(ret);
		}

		VSINGLE getArraySINGLE()
		{
			VSINGLE ret;
			return getArray(ret);
		}

		VUINT64 getArrayUINT64()
		{
			VUINT64 ret;
			return getArray(ret);
		}

		VUINT32 getArrayUINT32()
		{
			VUINT32 ret;
			return getArray(ret);
		}

		VUINT16 getArrayUINT16()
		{
			VUINT16 ret;
			return getArray(ret);
		}

		VUINT8 getArrayUINT8()
		{
			VUINT8 ret;
			return getArray(ret);
		}

		VINT64 getArrayINT64()
		{
			VINT64 ret;
			return getArray(ret);
		}

		VINT32 getArrayINT32()
		{
			VINT32 ret;
			return getArray(ret);
		}

		VINT16 getArrayINT16()
		{
			VINT16 ret;
			return getArray(ret);
		}

		VINT8 getArrayINT8()
		{
			VINT8 ret;
			return getArray(ret);
		}

		Vbool getArrayBOOL()
		{
			Vbool ret;
			return getArray(ret);
		}

		Dims getArrayDims()
		{
			//	always validate REAL
			validate(TYPE_REAL);

			//	check array dimensions are agreeable
			if (cache.dims.size() < 1 || cache.dims.size() > 2 || cache.dims.at(0) != 1)
				berr << E_DATAML << "invalid Dims matrix (should be row vector)";

			//	convert INT64 to Dims on return
			Dims ret;
			ret = getArrayINT64();
			return ret;
		}




	////////////////////////////////////////////////////////////////
	////	SCALAR NUMERIC INTERFACE

		DOUBLE getDOUBLE()
		{
			//	validate real and scalar
			validate(TYPE_REAL, Dims(1).cdims());

			//	use more complex routine to obtain data
			VDOUBLE v = getArrayDOUBLE();
			return v[0];
		}

		UINT32 getUINT32()
		{
			//	validate real and scalar
			validate(TYPE_REAL, Dims(1).cdims());

			//	use more complex routine to obtain data
			VUINT32 v = getArrayUINT32();
			return v[0];
		}

		INT32 getINT32()
		{
			//	validate real and scalar
			validate(TYPE_REAL, Dims(1).cdims());

			//	use more complex routine to obtain data
			VINT32 v = getArrayINT32();
			return v[0];
		}

		bool getBOOL()
		{
			//	validate real and scalar
			validate(TYPE_REAL, Dims(1).cdims());

			//	use more complex routine to obtain data
			Vbool v = getArrayBOOL();
			return v[0];
		}





	////////////////////////////////////////////////////////////////
	////	BINARY NUMERIC INTERFACE

		//	set numeric data in binary file form, where the caller has already written the binary file
		void setBinaryFile(const Dims& dims, TYPE type, const char* filename)
		{
			setArrayStructure(this, type, dims);

			xmlNode->setAttribute("s", "b");

			//	if the fully-specified pathname has the path of the report file on it,
			//	just write out the relative path name, for ease of moving the report data around
			XMLNode pars(executionInfo->executionParameters);
			std::string reportpath = pars.getChild("SupplementaryFilePath")->nodeText();
			
			std::string sfilename = filename;
			if (sfilename.length() > reportpath.length() && sfilename.substr(0, reportpath.length()) == reportpath)
			{
				sfilename = sfilename.substr(reportpath.length());
				if (sfilename.length() && (sfilename.substr(0,1) == "/" || sfilename.substr(0,1) == "\\"))
					sfilename = sfilename.substr(1);
				xmlNode->nodeText(sfilename.c_str());
			}
			else xmlNode->nodeText(filename);
		}





	//	end DataMLNode class

	};


//	end namespace brahms

}








/* In order to provide legacy support for these interfaces,
we have to include the headers, here. This does no harm if
they are not used (except to slow compilation marginally). */

#ifndef BRAHMS_NO_LEGACY_SUPPORT

//	import brahms namespace
using namespace brahms;

//	include interfaces for standard data/util components
#include "std/2009/data/numeric/brahms/0/data.h"
#include "std/2009/data/spikes/brahms/0/data.h"
#include "std/2009/util/rng/brahms/0/utility.h"

#endif



////////////////////////////////////////////////////////////////
//
//	To simplify process authoring, the most common options
//	can be included simply by defining this symbol
//

#ifdef OVERLAY_QUICKSTART_PROCESS

//	component version
const struct ComponentVersion COMPONENT_VERSION =
{
	COMPONENT_RELEASE,
	COMPONENT_REVISION
};

#endif

//
////////////////////////////////////////////////////////////////



