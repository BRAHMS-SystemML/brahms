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

	$Id:: support.h 2361 2009-11-12 18:39:39Z benjmitch      $
	$Rev:: 2361                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-12 18:39:39 +0000 (Thu, 12 Nov 2009)       $
________________________________________________________________

*/


//	this file is included by all support components, and by clients
//	of support (bindings, engine, engine modules).


#ifndef INCLUDED_BRAHMS_SUPPORT
#define INCLUDED_BRAHMS_SUPPORT



////////////////	INCLUDE COMPONENT INTERFACE

//	include base
#include "base/base.h"
#include "channel/channel.h"

//	include engine
#include "brahms-client.h"

//	import namespace
using namespace brahms;



////////////////	NON-BINDINGS SECTION

/*
	Bindings (matlab, python, etc.) do not include any more of this,
	because it conflicts with stuff they include through 1199.
*/

#ifndef BRAHMS_BUILDING_BINDINGS



////////////////	CONSTANTS

const DOUBLE EFFECTIVELY_ZERO = 1e-15;

//	list of known languages, in order of preference
const UINT32 KNOWN_LANGUAGES[] = {1266, 1199, 1065, 1262, 1258};
const UINT32 KNOWN_LANGUAGES_COUNT = 5;
const UINT32 LANGUAGE_UNSPECIFIED = 0;

const char LOG_ERROR_MARKER[] = "#### CAUGHT ERROR HERE ####";
const INT32 INDEX_NOT_SET = 0xFFFFFFFF;

const FrameworkVersion VERSION_ENGINE = {
	VERSION_ENGINE_MAJ,
	VERSION_ENGINE_MIN,
	VERSION_ENGINE_REL,
	VERSION_ENGINE_REV
};



////////////////	MINIMAL NUMBER OF MACROS

#define ____AT(e)        { e.at(__FILE__, __LINE__); }



////////////////	FORWARD DECLARATION OF Engine SO WE CAN PASS IT TO THINGS

namespace brahms
{
	class Engine;
}



////////////////	HEADER LIST

//	basic functionality used throughout the support layer
#include "support/helpers.h"	//	helper functions i've written
#include "support/os.h"			//	platform-independent OS functionality

//	error/output are used throughout the support layer to provide support for exceptions and logging
#include "support/error.h"		//	exception class

//	global object register
#include "support/register.h"

//	components that use the object register
#include "support/xml.h"

//	other platform-independent stuff that uses some of the above
#include "support/module.h"		//	module class
#include "support/environment.h"
#include "support/loader.h"
#include "support/execution.h"




////////////////	NON-BINDINGS SECTION

#endif // #ifndef BRAHMS_BUILDING_BINDINGS



////////////////	INCLUSION GUARD

#endif
