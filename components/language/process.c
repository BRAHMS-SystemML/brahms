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

	$Id:: process.c 2436 2009-12-12 19:14:48Z benjmitch        $
	$Rev:: 2436                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-12-12 19:14:48 +0000 (Sat, 12 Dec 2009)       $
________________________________________________________________

*/




////////////////	COMPONENT INFO

#define COMPONENT_CLASS_STRING "client/brahms/language/1266"
#define COMPONENT_CLASS_CPP client_brahms_language_1266_0
#define COMPONENT_RELEASE __REL__
#define COMPONENT_REVISION __REV__
#define COMPONENT_ADDITIONAL "Author=Ben Mitch\n" "URL=http://brahms.sourceforge.net\n"
#define COMPONENT_FLAGS ( F_NOT_RATE_CHANGER | F_NEEDS_ALL_INPUTS )

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
	//	TO-DO: Design this structure to hold the state data for your process
	UINT32 count;
	DOUBLE noise;
	DOUBLE accum;
	Symbol hProcess;
	Symbol hInputSet;
	Symbol hOutputSet;
	Symbol hInput;
	Symbol hOutput;
	Symbol hRNG;
	UINT32 suppress_output;
	struct HandledEvent utility;
	struct HandledEvent input;
	struct HandledEvent output;
};

Symbol quick_errorMessage(Symbol hComponent, Symbol error, const char* msg, UINT32 flags)
{
	struct EventErrorMessage data;
	struct EngineEvent event;

	data.error = error;
	data.msg = msg;
	data.flags = flags;
	
	event.hCaller = hComponent;
	event.flags = 0;
	event.type = ENGINE_EVENT_ERROR_MESSAGE;
	event.data = &data;
	return brahms_engineEvent(&event);
}



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
			Symbol node = S_NULL;
			Symbol err = S_NULL;
			const char* nodetext = NULL;
			struct EventStateSet* ess = (struct EventStateSet*) event->data;
			struct EventCreateUtility cu;
			struct EventGetRandomSeed grs;
			struct std_2009_util_rng_0_Seed seed;
			UINT32 useed;
			DOUBLE temp;
			struct EventGetSet data;
			struct EngineEvent engineEvent;
			struct EventOutputMessage msg;
			


////////////////	SAY HELLO

			msg.flags = 0;
			msg.level = D_INFO;
			msg.msg = "Hello from C";

			engineEvent.hCaller = object->hProcess;
			engineEvent.flags = 0;
			engineEvent.type = ENGINE_EVENT_OUTPUT_MESSAGE;
			engineEvent.data = (void*) &msg;

			err = brahms_engineEvent(&engineEvent);
			if (S_ERROR(err)) return err;



////////////////	ACCESS SETS

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

			//	set parameters (there's no direct support for DataML in C, so we have to do it manually)
			node = xml_getChild(ess->state, "m", 0);
			if (S_ERROR(node)) return node;
			err = xml_getNodeText(node, &nodetext);
			if (S_ERROR(err)) return err;
			sscanf(nodetext, "%i", &object->count);

			node = xml_getChild(ess->state, "m", 1);
			if (S_ERROR(node)) return node;
			err = xml_getNodeText(node, &nodetext);
			if (S_ERROR(err)) return err;
			sscanf(nodetext, "%lf", &object->noise);

			node = xml_getChild(ess->state, "m", 2);
			if (S_ERROR(node)) return node;
			err = xml_getNodeText(node, &nodetext);
			if (S_ERROR(err)) return err;
			sscanf(nodetext, "%lf", &temp);
			object->suppress_output = temp;



////////////////	INIT STATE

			//	set state
			object->accum = 0.0;



////////////////	GET RNG

			//	obtain RNG utility object
			____CLEAR(cu);
			cu.hUtility = S_NULL;
			cu.spec.cls = "std/2009/util/rng";
			cu.spec.release = 0;
			cu.name = NULL;
			cu.handledEvent = &object->utility;

			engineEvent.hCaller = object->hProcess;
			engineEvent.flags = 0;
			engineEvent.type = ENGINE_EVENT_CREATE_UTILITY;
			engineEvent.data = (void*) &cu;

			object->hRNG = brahms_engineEvent(&engineEvent);
			if (S_ERROR(object->hRNG)) return object->hRNG;

			//	initialise it
			object->utility.event.type = std_2009_util_rng_0_EVENT_UTILITY_SELECT;
			object->utility.event.flags = 0;
			object->utility.event.data = "MT2000.normal";
			err = object->utility.handler(&object->utility.event);
			if (S_ERROR(err)) return err;

			//	get random seed
			____CLEAR(grs);
			grs.count = 1;
			grs.seed = &useed;

			engineEvent.hCaller = object->hProcess;
			engineEvent.flags = 0;
			engineEvent.type = ENGINE_EVENT_GET_RANDOM_SEED;
			engineEvent.data = (void*) &grs;

			err = brahms_engineEvent(&engineEvent);
			if (S_ERROR(err)) return err;

			//	seed RNG
			____CLEAR(seed);
			seed.seed = grs.seed;
			seed.count = grs.count;
			object->utility.event.type = std_2009_util_rng_0_EVENT_UTILITY_SEED;
			object->utility.event.flags = 0;
			object->utility.event.data = &seed;
			err = object->utility.handler(&object->utility.event);
			if (S_ERROR(err)) return err;

			//	ok
			return C_OK;
		}

		case EVENT_INIT_CONNECT:
		{
			//	See the documentation for this event, but in short it is
			//	called repeatedly until you have seen all your inputs. On
			//	each call, you are expected to create as many outputs as
			//	possible. You may check the flags F_FIRST_CALL and
			//	F_LAST_CALL to pick out those particular calls in the
			//	repetition. On the last call, all our inputs are available.
			//	However, since this process sets F_NEEDS_ALL_INPUTS, it
			//	will only receive one call to this event.

			struct std_2009_data_numeric_0_Structure struc;
			INT64 scount;
			Symbol err;
			struct EventAddPort data;
			struct EventGetPort gpdata;
			struct EventGetSetInfo setInfo;
			struct EngineEvent engineEvent;

			//	expect one input
			setInfo.hSet = object->hInputSet;
			setInfo.flags = 0;
			setInfo.name = NULL;
			setInfo.portCount = 0;

			engineEvent.hCaller = object->hProcess;
			engineEvent.flags = 0;
			engineEvent.type = ENGINE_EVENT_GET_SET_INFO;
			engineEvent.data = (void*) &setInfo;

			err = brahms_engineEvent(&engineEvent);
			if (S_ERROR(err)) return err;

			if (setInfo.portCount != 1)
				return quick_errorMessage(object->hProcess, E_INVALID_INPUT, "expects one input", 0);

			//	get input
			gpdata.hSet = object->hInputSet;
			gpdata.flags = 0;
			gpdata.name = NULL;
			gpdata.index = 0;
			gpdata.handledEvent = &object->input;
			gpdata.spec.cls = "std/2009/data/numeric";
			gpdata.spec.release = 0;
			
			engineEvent.hCaller = object->hProcess;
			engineEvent.flags = 0;
			engineEvent.type = ENGINE_EVENT_GET_PORT;
			engineEvent.data = &gpdata;
			object->hInput = brahms_engineEvent(&engineEvent);

			if (S_ERROR(object->hInput)) return object->hInput;

			//	validate structure
			scount = 1;
			struc.type = TYPE_DOUBLE | TYPE_REAL;
			struc.dims.dims = &scount;
			struc.dims.count = 1;
			object->input.event.type = std_2009_data_numeric_0_EVENT_DATA_VALIDATE_STRUCTURE;
			object->input.event.flags = 0;
			object->input.event.data = &struc;
			err = object->input.handler(&object->input.event);
			if (S_ERROR(err)) return err;

			//	create one output
			____CLEAR(data);
			data.index = INDEX_UNDEFINED;
			data.hSet = object->hOutputSet;
			data.flags = 0;
			data.spec.cls = "std/2009/data/numeric";
			data.spec.release = 0;
			data.name = "out";
			data.hPortToCopy = S_NULL;
			data.transportProtocol = C_TRANSPORT_PERIODIC;
			data.handledEvent = &object->output;

			engineEvent.hCaller = object->hProcess;
			engineEvent.flags = 0;
			engineEvent.type = ENGINE_EVENT_ADD_PORT;
			engineEvent.data = (void*) &data;

			object->hOutput = brahms_engineEvent(&engineEvent);
			if (S_ERROR(object->hOutput)) return object->hOutput;

			//	set structure
			scount = object->count;
			struc.type = TYPE_DOUBLE | TYPE_REAL;
			struc.dims.dims = &scount;
			struc.dims.count = 1;
			object->output.event.type = std_2009_data_numeric_0_EVENT_DATA_STRUCTURE_SET;
			object->output.event.flags = 0;
			object->output.event.data = &struc;
			err = object->output.handler(&object->output.event);
			if (S_ERROR(err)) return err;

			//	ok
			return C_OK;
		}

		case EVENT_RUN_SERVICE:
		{
			//	variables
			Symbol err;
			struct std_2009_data_numeric_0_Content content;
			DOUBLE in;
			DOUBLE* out;
			UINT32 i;
			struct std_2009_util_rng_0_Fill fill;

			content.flags = 0;
			content.type = TYPE_CPXFMT_ADJACENT | TYPE_ORDER_COLUMN_MAJOR;

			//	retrieve input (SERVICE input interface)
			object->input.event.type = std_2009_data_numeric_0_EVENT_DATA_CONTENT_GET;
			object->input.event.flags = 0;
			object->input.event.data = &content;
			err = object->input.handler(&object->input.event);
			if (S_ERROR(err)) return err;

			//	don't really need to check this, since it's enforced given the above.
			//	in particular, structure of data objects cannot change after EVENT_INIT_CONNECT.
			if (content.bytes != sizeof(DOUBLE)) return E_REALITY_INVERSION;
			in = *((DOUBLE*)content.real);

			//	process
			object->accum = 0.9 * object->accum + in;

			//	and of course, we have to SERVICE our output interface
			object->output.event.type = std_2009_data_numeric_0_EVENT_DATA_CONTENT_GET;
			object->output.event.flags = 0;
			object->output.event.data = &content;
			err = object->output.handler(&object->output.event);
			if (S_ERROR(err)) return err;

			//	don't really need to check this, since it's enforced given the above.
			//	in particular, structure of data objects cannot change after EVENT_INIT_CONNECT.
			if (content.bytes != (object->count * sizeof(DOUBLE))) return E_REALITY_INVERSION;
			out = (DOUBLE*)content.real;

			//	do random bit
			____CLEAR(fill);
			fill.dst = out;
			fill.count = object->count;
			fill.type = TYPE_DOUBLE;
			fill.gain = object->noise;
			object->utility.event.type = std_2009_util_rng_0_EVENT_UTILITY_FILL;
			object->utility.event.flags = 0;
			object->utility.event.data = &fill;
			err = object->utility.handler(&object->utility.event);
			if (S_ERROR(err)) return err;

			//	add accum
			for (i=0; i<object->count; i++)
				out[i] += object->accum;

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


