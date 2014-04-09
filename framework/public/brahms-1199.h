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

	$Id:: brahms-1199.h 2413 2009-11-20 17:27:39Z benjmitch    $
	$Rev:: 2413                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-20 17:27:39 +0000 (Fri, 20 Nov 2009)       $
________________________________________________________________

*/




namespace brahms
{


////////////////	COMPONENT CLASS

	struct Component
	{
		Component()
		{
			time = NULL;
			componentData = NULL;
			hComponent = S_NULL;

			//	can perform 32/64 bit asserts here
			if (sizeof(UINT64) != 8) berr << "failed assert: sizeof(UINT64) == 8";
			if (sizeof(INT64) != 8) berr << "failed assert: sizeof(INT64) == 8";
			if (sizeof(UINT32) != 4) berr << "failed assert: sizeof(UINT32) == 4";
			if (sizeof(INT32) != 4) berr << "failed assert: sizeof(INT32) == 4";
			if (sizeof(UINT16) != 2) berr << "failed assert: sizeof(UINT16) == 2";
			if (sizeof(INT16) != 2) berr << "failed assert: sizeof(INT16) == 2";
			if (sizeof(INT16) != 2) berr << "failed assert: sizeof(INT16) == 2";
			if (sizeof(DOUBLE) != 8) berr << "failed assert: sizeof(DOUBLE) == 8";
		}

		Component(const Component& src)
		{
			time = NULL;
			hComponent = S_NULL;
		}

		Component& operator=(const Component& src)
		{
			berr << E_INTERNAL << "Component() assignment";
			return *this;
		}

		virtual ~Component()
		{
		}

		VUINT32 getRandomSeed(UINT32 count = 0)
		{
			VUINT32 seed;
			EventGetRandomSeed rseed;
			rseed.flags = 0;
			rseed.count = count;

			if (count)
			{
				seed.resize(count);
				rseed.seed = &seed[0];
				
				EngineEvent event;
				event.hCaller = hComponent;
				event.flags = 0;
				event.type = ENGINE_EVENT_GET_RANDOM_SEED;
				event.data = (void*) &rseed;

				____SUCCESS(brahms_engineEvent(&event));
			}
			else
			{
				rseed.seed = NULL;

				EngineEvent event;
				event.hCaller = hComponent;
				event.flags = 0;
				event.type = ENGINE_EVENT_GET_RANDOM_SEED;
				event.data = (void*) &rseed;
				
				____SUCCESS(brahms_engineEvent(&event));

				//	if zero, can have whatever we want - we choose 1
				if (!rseed.count) rseed.count = 1;
				seed.resize(rseed.count);
				rseed.seed = &seed[0];

				____SUCCESS(brahms_engineEvent(&event));
			}

			return seed;
		}

		ComponentData getComponentData(Symbol hSubject)
		{
			//	can still return our own
			if (hSubject == hComponent)
				return *componentData;

			//	otherwise that's an error
			berr << E_INTERNAL;
		}

//	TODO: should be removed in a future binding, since we now use the interface in utility.h to create a utility

		Symbol createUtility(const char* cls, UINT16 release, const char* name = NULL)
		{
			EventCreateUtility data;
			data.flags = 0;
			data.hUtility = S_NULL;
			data.spec.cls = cls;
			data.spec.release = release;
			data.name = name;
			data.handledEvent = NULL;

			EngineEvent event;
			event.hCaller = hComponent;
			event.flags = 0;
			event.type = ENGINE_EVENT_CREATE_UTILITY;
			event.data = (void*) &data;

			____SUCCESS(brahms_engineEvent(&event));

			return data.hUtility;
		}

		void stillActive()
		{
			EngineEvent event;
			event.hCaller = hComponent;
			event.flags = 0;
			event.type = ENGINE_EVENT_STILL_ACTIVE;
			event.data = 0;
			
			____SUCCESS(brahms_engineEvent(&event));
		}

		virtual Symbol event(Event* event) = 0;
		virtual void initialize(Symbol hComponent, const ComponentData* data) = 0;

		const ComponentTime* time;
		const ComponentData* componentData;
		Symbol hComponent;
		ComponentOut bout;



#ifndef BRAHMS_NO_LEGACY_SUPPORT

		void assertClass(Symbol hData, const char* cls, UINT16 release = 0)
		{
			//	data
			ComponentSpec spec;
			spec.cls = cls;
			spec.release = release;

			//	make legacy call
			brahms::EngineEvent event;
			event.hCaller = hData;
			event.flags = 0;
			event.type = ENGINE_EVENT_ASSERT_COMPONENT_SPEC;
			event.data = (void*) &spec;
			
			____SUCCESS(brahms::brahms_engineEvent(&event));
		}

		Symbol sendSignal(Symbol signal)
		{
			if (signal == ENGINE_EVENT_STILL_ACTIVE)
			{
				EngineEvent event;
				event.hCaller = hComponent;
				event.flags = 0;
				event.type = signal;
				event.data = 0;
				
				____SUCCESS(brahms_engineEvent(&event));
			}

			else
			{
				berr << E_INTERNAL << "signal not recognised";
			}

			return C_OK;
		}

#endif

	};



////////////////	COMPONENT SUBCLASSES

#ifdef COMPONENT_PROCESS

	struct Process : public Component
	{
		Process()
		{
		}

		void initialize(Symbol hComponent, const ComponentData* data)
		{
			this->hComponent = hComponent;
			this->time = data->time;
			this->componentData = data;
			bout.initialize(hComponent);
			iif.initialize(hComponent, F_IIF);
			oif.initialize(hComponent, F_OIF);
		}

		SystemMLInterface iif;
		SystemMLInterface oif;
	};

#endif

#ifdef COMPONENT_DATA

	struct Data : public Component
	{
		Data()
		{
		}

		void initialize(Symbol hComponent, const ComponentData* data)
		{
			this->hComponent = hComponent;
			this->time = data->time;
			this->componentData = data;
			bout.initialize(hComponent);
		}
	};

#endif

#ifdef COMPONENT_UTILITY

	struct Utility : public Component
	{
		Utility()
		{
		}

		void initialize(Symbol hComponent, const ComponentData* data)
		{
			this->hComponent = hComponent;
			this->time = data->time;
			this->componentData = data;
			bout.initialize(hComponent);
		}
	};

#endif




}








////////////////////////////////////////////////////////////////
//
//	The first section, the H section, gets processed the
//	*first* time this file is included.
//

#else

//
////////////////////////////////////////////////////////////////










////////////////////////////////////////////////////////////////
//
//	The second section, the CPP section, gets processed the
//	*second* time this file is included.
//

#ifndef INCLUDED_OVERLAY_2
#define INCLUDED_OVERLAY_2

//
////////////////////////////////////////////////////////////////









/*

	For native components (components not loaded through a language binding) the
	ComponentInfo structure is identical for all components instantiated by a
	module, so we can just maintain it at module level and return a pointer to
	it when requested (see EVENT_MODULE_CREATE/EVENT_MODULE_DESTROY in the cpp).

*/

//	legacy
#ifdef MODULE_CT_TYPE
#error COMPONENT_TYPE is deprecated - use MODULE_CT_TYPE
#endif
#ifdef MODULE_COMPONENT_FLAGS
#error MODULE_COMPONENT_FLAGS is deprecated - use COMPONENT_FLAGS
#endif
#ifdef MODULE_MODULE_FLAGS
#error MODULE_MODULE_FLAGS is deprecated - use MODULE_FLAGS
#endif
#ifdef MODULE_COMPONENT_ADDITIONAL
#error MODULE_COMPONENT_ADDITIONAL is deprecated - use COMPONENT_ADDITIONAL
#endif



//	engine version
const brahms::FrameworkVersion MODULE_VERSION_ENGINE = brahms::VERSION_ENGINE;

//	component class
#ifndef COMPONENT_CLASS_CPP
#error must define COMPONENT_CLASS_CPP
#endif

//	component class string
#ifndef COMPONENT_CLASS_STRING
#error must define COMPONENT_CLASS_STRING
#endif

//	component type
#ifndef COMPONENT_TYPE
#error must define COMPONENT_TYPE
#endif

//	component flags
#ifndef COMPONENT_FLAGS
#define COMPONENT_FLAGS 0
#endif

//	component additional
#ifndef COMPONENT_ADDITIONAL
#define COMPONENT_ADDITIONAL ""
#endif

//	module flags
#ifndef MODULE_FLAGS
#define MODULE_FLAGS 0
#endif

//	component info
brahms::ComponentInfo COMPONENT_INFO =
{
	COMPONENT_CLASS_STRING,
	COMPONENT_TYPE,
	&COMPONENT_VERSION,
	COMPONENT_FLAGS,
	COMPONENT_ADDITIONAL,
	""
};

//	module info
brahms::ModuleInfo MODULE_INFO =
{
	&MODULE_VERSION_ENGINE,
	ARCH_BITS,
	BRAHMS_BINDING_NAME,
	MODULE_FLAGS
};



#ifdef MODULE_SHOW_EVENTS
#include <iostream>
using namespace std;
#endif



//	event handler
BRAHMS_DLL_EXPORT brahms::Symbol EventHandler(brahms::Event* event)
{
	brahms::Symbol result = S_NULL;

	try
	{

#ifdef MODULE_SHOW_EVENTS
		cout << "BRAHMS (INFO): " << brahms::getSymbolString(event->type) << " (ENTER) " << COMPONENT_CLASS_STRING << endl;
#endif

		switch (event->type)
		{
			case EVENT_MODULE_QUERY:
			{
				brahms::EventModuleQuery* query = (brahms::EventModuleQuery*) event->data;
				query->interfaceID = N_BRAHMS_INTERFACE;
				result = C_OK;
				break;
			}

			case EVENT_MODULE_INIT:
			{
				brahms::EventModuleInit* init = (brahms::EventModuleInit*) event->data;
				executionInfo = init->executionInfo;
				init->moduleInfo = &MODULE_INFO;

#ifdef MODULE_INIT_AND_TERM
				result = moduleInit(init);
#else
				result = C_OK;
#endif

				break;
			}

			case EVENT_MODULE_TERM:
			{

#ifdef MODULE_INIT_AND_TERM
				brahms::EventModuleTerm* term = (brahms::EventModuleTerm*) event->data;
				result = moduleTerm(term);
#else
				result = C_OK;
#endif

				break;
			}

			case EVENT_MODULE_CREATE:
			{
				brahms::EventModuleCreate* emc = (brahms::EventModuleCreate*) event->data;
				emc->info = &COMPONENT_INFO;

				COMPONENT_CLASS_CPP* newObject = new COMPONENT_CLASS_CPP;
				event->object = newObject;

				newObject->initialize(emc->hComponent, emc->data);

				result = C_OK;
				break;
			}

			case EVENT_MODULE_DESTROY:
			{
				delete (COMPONENT_CLASS_CPP*) event->object;
				result = C_OK;
				break;
			}

			case EVENT_MODULE_DUPLICATE:
			{
				brahms::EventModuleCreate* emc = (brahms::EventModuleCreate*) event->data;
				emc->info = &COMPONENT_INFO;

				COMPONENT_CLASS_CPP* object = (COMPONENT_CLASS_CPP*) event->object;
				if (!object) return E_NO_INSTANCE;

				COMPONENT_CLASS_CPP* newObject = new COMPONENT_CLASS_CPP(*object);
				event->object = newObject;

				newObject->initialize(emc->hComponent, emc->data);

				result = C_OK;
				break;
			}

			default:
			{
				//	this covers all the events we know about, but also covers
				//	any events we don't know about (events that are added later
				//	to the specification). we just pass them on to the object,
				//	if they are object-level events, but we check for an instance
				//	before we pass them on, obviously! if they are module-level
				//	events, and we don't recognise them, best we can do is ignore
				//	them.

				if (event->type <= EVENT_MODULE_MAX)
				{
					//	ignore, result is S_NULL (unhandled)
				}

				else
				{
					COMPONENT_CLASS_CPP* object = (COMPONENT_CLASS_CPP*) event->object;
					if (!object) return E_NO_INSTANCE;
					result = object->event(event);
				}

				//	ok
				break;
			}
		}

	}

	catch(Symbol s)
	{
		result = s;
	}

	catch(const char* e)
	{
		brahms::EventErrorMessage data;
		data.error = E_USER;
		data.msg = e;
		data.flags = 0;

		EngineEvent event;
		event.hCaller = 0;
		event.flags = 0;
		event.type = ENGINE_EVENT_ERROR_MESSAGE;
		event.data = &data;
		result = brahms_engineEvent(&event);
	}

	catch(std::exception& e)
	{
		brahms::EventErrorMessage data;
		data.error = E_STD;
		data.msg = e.what();
		data.flags = 0;

		EngineEvent event;
		event.hCaller = 0;
		event.flags = 0;
		event.type = ENGINE_EVENT_ERROR_MESSAGE;
		event.data = &data;
		result = brahms_engineEvent(&event);
	}

	catch(...)
	{
		result = E_UNRECOGNISED_EXCEPTION;
	}

#ifdef MODULE_SHOW_EVENTS
	cout << "BRAHMS (INFO): " << brahms::getSymbolString(event->type) << " (LEAVE with result " << brahms::getSymbolString(result) << ") " << COMPONENT_CLASS_STRING << endl;
#endif

	return result;

}










////////////////////////////////////////////////////////////////
//
//	The second section, the CPP section, gets processed the
//	*second* time this file is included.
//

#endif
#endif

//
////////////////////////////////////////////////////////////////



