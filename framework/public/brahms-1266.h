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

	$Id:: brahms-1266.h 2428 2009-12-11 15:10:37Z benjmitch    $
	$Rev:: 2428                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-12-11 15:10:37 +0000 (Fri, 11 Dec 2009)       $
________________________________________________________________

*/


/*

	This is a C overlay interface for the BRAHMS Component Interface.

	Revision: __REV__

	The BRAHMS Component Interface is a set of C structures and functions.
	Overlay interfaces are C bases from which you can build your own
	components using C techniques.

*/



////////////////////////////////////////////////////////////////
//
//	Half of this file gets processed the first time it is
//	included, and half the second time. On subsequent includes,
//	nothing is processed.
//

//	set on second pass
#ifdef INCLUDED_BRAHMS_C_1
#define INCLUDED_BRAHMS_C_2
#endif

//	set on first pass
#define INCLUDED_BRAHMS_C_1

//
////////////////////////////////////////////////////////////////






////////////////////////////////////////////////////////////////
//
//	The first section, the H section, gets processed the
//	*first* time this file is included.
//

#ifdef INCLUDED_BRAHMS_C_1
#ifndef INCLUDED_BRAHMS_C_2

//
////////////////////////////////////////////////////////////////



#include "BrahmsConfig.h"
#define __BINDING__ 1266
#define __REV__ VERSION_BRAHMS_REV

#define BRAHMS_BUILD_TARGET __BINDING__
#define BRAHMS_BINDING_NAME __BINDING__
#define BRAHMS_BINDING_REVISION __REV__

#define ____CLEAR(what) memset(&what, 0, sizeof(what))
#include "brahms-component.h"


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




////////////////////////////////////////////////////////////////
//
//	To simplify process authoring, the most common options
//	can be included simply by defining this symbol
//

#ifdef OVERLAY_QUICKSTART_PROCESS

//	include interfaces for other components we use
#include "data_numeric.h"
#include "data_spikes.h"
#include "util_rng.h"

#endif

//
////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////
//
//	The first section, the H section, gets processed the
//	*first* time this file is included.
//

#endif
#endif

//
////////////////////////////////////////////////////////////////










////////////////////////////////////////////////////////////////
//
//	The second section, the C section, gets processed the
//	*second* time this file is included.
//

#ifdef INCLUDED_BRAHMS_CPP_2

//
////////////////////////////////////////////////////////////////







////////////////////////////////////////////////////////////////
//
//	The second section, the C section, gets processed the
//	*second* time this file is included.
//

#endif

//
////////////////////////////////////////////////////////////////
