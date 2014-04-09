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

	$Id:: data.h 1645 2008-10-30 08:16:42Z benjmitch           $
	$Rev:: 1645                                                $
	$Author:: benjmitch                                        $
	$Date:: 2008-10-30 08:16:42 +0000 (Thu, 30 Oct 2008)       $
________________________________________________________________

*/




/*

	NOTES

	* We use <CLASS_US> to represent the class name of the
	component with slashes replaced with underscores, thus:

		<CLASS_US> = ____CLASS_US____

	* BRAHMS data objects offer an access API to process
	objects - it is this API that is defined in the
	accompanying header file (like this one). The framework
	never sees this API, since it does not need to access
	data object content.

	* Data objects offer their base interface over their 
	event() function, in C, so that they are accessible to
	all BRAHMS components. This interface will consist of
	a number of EVENT constants, and any data structures
	needed to pass data into and out of these events. All
	tokens on this interface should be prefixed with
	<CLASS_US> to ensure they are unique in the namespace of
	any including component.

	* It is strongly recommended that all data objects also
	offer a C++ version of this base interface, as below,
	simplifying their access from C++ components. In this
	case, all tokens should be placed in a namespace that is
	named <CLASS_US>.

	* It is not critical, but data objects should probably
	also provide an inclusion guard. To ensure uniqueness,
	the inclusion guard token should be, as used below:
	
		INCLUDED_COMPONENT_____CLASS_US____

*/



/*

	TEMPLATE

	* Here, we loosely use the analogy that we are building
	a data object that represents the state of a "vehicle".
	It has structure state (number of wheels, size of
	engine) and content state (speed, amount of gas).

*/






////////////////	INCLUSION GUARD

#ifndef INCLUDED_COMPONENT_____CLASS_US____
#define INCLUDED_COMPONENT_____CLASS_US____









#ifndef __cplusplus

/*

	C Interface

*/



//	Structures

struct ____CLASS_US_____VehicleStructure
{
	UINT32 numberOfWheels;
};

struct ____CLASS_US_____VehicleContent
{
	DOUBLE* wheelSpeed;
};



//	Event Types

//	The API should be implemented through event() calls (generally, though
//	there is nothing to prevent a particular data object offering additional
//	member functions to optimise access if that is appropriate). However, for
//	the sake of simplicity in the code of the process that is calling the
//	API, it is recommended that data objects also define wrapper functions
//	for these event calls, so that the events are made to look like strongly-
//	typed functions.

//	All event codes used by the API should be >= C_BASE_USER_EVENT. Since you
//	provide wrapper functions for these events, the event codes themselves
//	will not be needed outside of this header file.

#define ____CLASS_US_____EVENT_VEHICLE_GET_STRUCTURE ( C_BASE_USER_EVENT + 1 )
#define ____CLASS_US_____EVENT_VEHICLE_SET_STRUCTURE ( C_BASE_USER_EVENT + 2 )
#define ____CLASS_US_____EVENT_VEHICLE_GET_CONTENT ( C_BASE_USER_EVENT + 3 )
#define ____CLASS_US_____EVENT_VEHICLE_SET_CONTENT ( C_BASE_USER_EVENT + 4 )

//	Really this is just to illustrate, that there is no limit to the access
//	methods you can offer your client processes - it is whatever is appropriate
//	to your data object. BRAHMS never sees this interface, let alone calls it.
//	Here, we show an alternative way of accessing (some of) the content state.

#define ____CLASS_US_____EVENT_VEHICLE_GET_CONTENT_DIRECT ( C_BASE_USER_EVENT + 5 )
#define ____CLASS_US_____EVENT_VEHICLE_SET_CONTENT_DIRECT ( C_BASE_USER_EVENT + 6 )






#else

/*

	C++ Interface

*/



namespace ____CLASS_US____
{

	//	Structures

	struct VehicleStructure
	{
		UINT32 numberOfWheels;
	};

	struct VehicleContent
	{
		DOUBLE* wheelSpeed;
	};



	//	Event Types

	//	The API should be implemented through event() calls (generally, though
	//	there is nothing to prevent a particular data object offering additional
	//	member functions to optimise access if that is appropriate). However, for
	//	the sake of simplicity in the code of the process that is calling the
	//	API, it is recommended that data objects also define wrapper functions
	//	for these event calls, so that the events are made to look like strongly-
	//	typed functions.

	//	All event codes used by the API should be >= C_BASE_USER_EVENT. Since you
	//	provide wrapper functions for these events, the event codes themselves
	//	will not be needed outside of this header file.

	#define EVENT_GET_STRUCTURE ( C_BASE_USER_EVENT + 1 )
	#define EVENT_SET_STRUCTURE ( C_BASE_USER_EVENT + 2 )
	#define EVENT_GET_CONTENT ( C_BASE_USER_EVENT + 3 )
	#define EVENT_SET_CONTENT ( C_BASE_USER_EVENT + 4 )

	//	Really this is just to illustrate, that there is no limit to the access
	//	methods you can offer your client processes - it is whatever is appropriate
	//	to your data object. BRAHMS never sees this interface, let alone calls it.
	//	Here, we show an alternative way of accessing (some of) the content state.

	#define EVENT_GET_CONTENT_DIRECT ( C_BASE_USER_EVENT + 5 )
	#define EVENT_SET_CONTENT_DIRECT ( C_BASE_USER_EVENT + 6 )



	//	Inline Accessors

	inline TargetedEvent createTargetedEvent(Symbol hCaller, Symbol hTarget, Symbol type, void* data)
	{
		TargetedEvent tev;
		tev.hCaller = hCaller;
		tev.hTarget = hTarget;
		Symbol result = brahms_createTargetedEvent(&tev);
		if (S_ERROR(result)) berr << result;
		tev.event.type = type;
		tev.event.data = data;
		return tev;
	}

	inline VehicleStructure* get_structure(Symbol hCaller, Symbol hData)
	{
		TargetedEvent tev = createTargetedEvent(hCaller, hData, EVENT_GET_STRUCTURE, NULL);
		Symbol result = fireTargetedEvent(&tev);
		if(S_ERROR(result)) throw result;
		return (VehicleStructure*) tev.event.data;
	}

	inline void set_structure(Symbol hCaller, Symbol hData, VehicleStructure* structure)
	{
		TargetedEvent tev = createTargetedEvent(hCaller, hData, EVENT_SET_STRUCTURE, structure);
		Symbol result = fireTargetedEvent(&tev);
		if(S_ERROR(result)) throw result;
	}

	inline VehicleContent* get_content(Symbol hCaller, Symbol hData)
	{
		TargetedEvent tev = createTargetedEvent(hCaller, hData, EVENT_GET_CONTENT, NULL);
		Symbol result = fireTargetedEvent(&tev);
		if(S_ERROR(result)) throw result;
		return (VehicleContent*) tev.event.data;
	}

	inline void set_content(Symbol hCaller, Symbol hData, VehicleContent* content)
	{
		TargetedEvent tev = createTargetedEvent(hCaller, hData, EVENT_SET_CONTENT, content);
		Symbol result = fireTargetedEvent(&tev);
		if(S_ERROR(result)) throw result;
	}

	inline DOUBLE* get_content_direct(Symbol hCaller, Symbol hData)
	{
		TargetedEvent tev = createTargetedEvent(hCaller, hData, EVENT_GET_CONTENT_DIRECT, NULL);
		Symbol result = fireTargetedEvent(&tev);
		if(S_ERROR(result)) throw result;
		return ((VehicleContent*) tev.event.data)->wheelSpeed;
	}

	inline void set_content_direct(Symbol hCaller, Symbol hData, DOUBLE* content)
	{
		VehicleContent vcontent;
		vcontent.wheelSpeed = content;
		TargetedEvent tev = createTargetedEvent(hCaller, hData, EVENT_SET_CONTENT_DIRECT, &vcontent);
		Symbol result = fireTargetedEvent(&tev);
		if(S_ERROR(result)) throw result;
	}





}






#endif // C/C++




////////////////	INCLUSION GUARD

#endif


