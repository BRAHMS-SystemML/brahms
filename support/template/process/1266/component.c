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

	$Id:: component.c 2410 2009-11-20 10:18:18Z benjmitch      $
	$Rev:: 2410                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-20 10:18:18 +0000 (Fri, 20 Nov 2009)       $
________________________________________________________________

*/

/* CUT HERE */

/*

	This is a newly created BRAHMS Process. It is a native
	process, and needs to be built before BRAHMS can run it.
	In Matlab, run "brahms_manager", select this component,
	and click on "Build". After doing this, you can build it
	in future using "brahms_utils".

*/



////////////////	COMPONENT INFO

//	component information
#define COMPONENT_CLASS_STRING __TEMPLATE_CLASS_STRING_C__
#define COMPONENT_RELEASE __TEMPLATE__RELEASE_INT__
#define COMPONENT_REVISION __TEMPLATE__REVISION_INT__
#define COMPONENT_ADDITIONAL __TEMPLATE__ADDITIONAL_C__
#define COMPONENT_FLAGS __TEMPLATE__FLAGS_C__

//	include the component interface overlay, quick process mode
//	(you must have defined the above to use this)
#define OVERLAY_QUICKSTART_PROCESS
#include "brahms-1266.h"

//	include C headers we use
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



////////////////	MSC SECURITY FIXES

#ifdef _MSC_VER
#define sprintf(a,b,c) sprintf_s(a,256,b,c)
#define sscanf sscanf_s
#endif



////////////////	PER-MODULE DATA

//	component version
const struct ComponentVersion COMPONENT_VERSION =
{
	COMPONENT_RELEASE,
	COMPONENT_REVISION
};

//	framework version
const struct FrameworkVersion MODULE_VERSION_ENGINE =
{
	VERSION_ENGINE_MAJ,
	VERSION_ENGINE_MIN,
	VERSION_ENGINE_REL,
	VERSION_ENGINE_REV
};

//	module info
const struct ModuleInfo MODULE_MODULE_INFO =
{
	&MODULE_VERSION_ENGINE,
	ARCH_BITS,
	BRAHMS_BINDING_NAME,
	0
};

//	component info
struct ComponentInfo COMPONENT_INFO =
{
	COMPONENT_CLASS_STRING,
	COMPONENT_TYPE,
	&COMPONENT_VERSION,
	COMPONENT_FLAGS,
	COMPONENT_ADDITIONAL,
	""
};

//	system info
const struct ExecutionInfo* executionInfo;



////////////////	COMPONENT DATA STRUCTURE

struct ProcessData
{

	/*

		Member data used by the binding goes here -
		don't change this.

	*/

	Symbol hProcess;



	/*

		Private member data of the process goes here.
		You can use this to store your parameters
		and process state between calls to event().

	*/

	Symbol hInputSet;
	Symbol hOutputSet;

};



////////////////	EVENT HANDLER

Symbol EventHandler(struct Event* event)
{
	//	pre-amble
	struct ProcessData* object = (struct ProcessData*) event->object;

	//	switch on event type
	switch (event->type)
	{
		case EVENT_MODULE_QUERY:
		{
			//	let the framework know that we offer the BRAHMS interface
			struct EventModuleQuery* query = (struct EventModuleQuery*) event->data;
			query->interfaceID = N_BRAHMS_INTERFACE;
			return C_OK;
		}

		case EVENT_MODULE_INIT:
		{
			//	extract event-specific data
			struct EventModuleInit* ei = (struct EventModuleInit*) event->data;

			//	retrieve system info
			executionInfo = ei->executionInfo;

			//	dispatch module info
			ei->moduleInfo = &MODULE_MODULE_INFO;

			//	ok
			return C_OK;
		}

		case EVENT_MODULE_CREATE:
		{
			//	extract event-specific data
			struct EventModuleCreate* emc = (struct EventModuleCreate*) event->data;

			//	dispatch component info
			emc->info = &COMPONENT_INFO;

			//	create new object
			event->object = malloc(sizeof(struct ProcessData));
			memset(event->object, 0, sizeof(struct ProcessData));

			//	TO-DO: do any initialisation of the new state data structure here
			((struct ProcessData*)event->object)->hProcess = emc->hComponent;

			//	ok
			return C_OK;
		}

		case EVENT_MODULE_DESTROY:
		{
			//	just delete the object
			free(object);

			//	ok
			return C_OK;
		}

		case EVENT_MODULE_DUPLICATE:
		{
			//	extract event-specific data
			struct EventModuleCreate* emc = (struct EventModuleCreate*) event->data;

			//	duplicate object (i.e. state data)
			struct ProcessData* newObject = 0;
			if (!object) return E_NO_INSTANCE;
			event->object = malloc(sizeof(struct ProcessData));

			//	TO-DO: do any initialisation of the new state data structure here
			newObject = (struct ProcessData*)event->object;
			*newObject = *object; // copy old object
			newObject->hProcess = emc->hComponent; // but set in new component handle

			//	dispatch component info
			emc->info = &COMPONENT_INFO;

			//	ok
			return C_OK;
		}

		case EVENT_STATE_SET:
		{
			//	extract event-specific data
			struct EventGetSet data;
			struct EventStateSet* ess = (struct EventStateSet*) event->data;
			struct EngineEvent engineEvent;

			//	get handle to default input and output sets
			data.flags = F_IIF;
			data.name = "";
			engineEvent.hCaller = object->hProcess;
			engineEvent.flags = 0;
			engineEvent.type = ENGINE_EVENT_GET_SET;
			engineEvent.data = (void*) &data;
			object->hInputSet = brahms_engineEvent(&engineEvent);
			if (S_ERROR(object->hInputSet)) return object->hInputSet;
			data.flags = F_OIF;
			data.name = "";
			engineEvent.hCaller = object->hProcess;
			engineEvent.flags = 0;
			engineEvent.type = ENGINE_EVENT_GET_SET;
			engineEvent.data = (void*) &data;
			object->hOutputSet = brahms_engineEvent(&engineEvent);
			if (S_ERROR(object->hOutputSet)) return object->hOutputSet;

			//	ok
			return C_OK;
		}

		case EVENT_INIT_CONNECT:
		{
			//	ok
			return C_OK;
		}

		case EVENT_RUN_SERVICE:
		{
			//	ok
			return C_OK;
		}
	}

	//	not processed
	return S_NULL;
}



////////////////	INCLUDES

//	include the overlay (yes, again!)
#include "brahms-1266.h"


