/*
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
*/

// This is the second section, the CPP section, gets processed
// the *second* time this file is included. This is the
// "implementation" section.

#ifndef _BRAHMS_1199_IMPL_H_
#define _BRAHMS_1199_IMPL_H_

/*
  For native components (components not loaded through a language binding) the
  ComponentInfo structure is identical for all components instantiated by a
  module, so we can just maintain it at module level and return a pointer to
  it when requested (see EVENT_MODULE_CREATE/EVENT_MODULE_DESTROY in the cpp).
*/

// legacy
#ifdef MODULE_CT_TYPE
# error COMPONENT_TYPE is deprecated - use MODULE_CT_TYPE
#endif
#ifdef MODULE_COMPONENT_FLAGS
# error MODULE_COMPONENT_FLAGS is deprecated - use COMPONENT_FLAGS
#endif
#ifdef MODULE_MODULE_FLAGS
# error MODULE_MODULE_FLAGS is deprecated - use MODULE_FLAGS
#endif
#ifdef MODULE_COMPONENT_ADDITIONAL
# error MODULE_COMPONENT_ADDITIONAL is deprecated - use COMPONENT_ADDITIONAL
#endif

// engine version
const brahms::FrameworkVersion MODULE_VERSION_ENGINE = brahms::VERSION_ENGINE;

// component class
#ifndef COMPONENT_CLASS_CPP
# error must define COMPONENT_CLASS_CPP
#endif

// component class string
#ifndef COMPONENT_CLASS_STRING
# error must define COMPONENT_CLASS_STRING
#endif

// component type
#ifndef COMPONENT_TYPE
# error must define COMPONENT_TYPE
#endif

// component flags
#ifndef COMPONENT_FLAGS
# define COMPONENT_FLAGS 0
#endif

// component additional
#ifndef COMPONENT_ADDITIONAL
# define COMPONENT_ADDITIONAL ""
#endif

// module flags
#ifndef MODULE_FLAGS
# define MODULE_FLAGS 0
#endif

// component info
brahms::ComponentInfo COMPONENT_INFO =
{
    COMPONENT_CLASS_STRING,
    COMPONENT_TYPE,
    &COMPONENT_VERSION,
    COMPONENT_FLAGS,
    COMPONENT_ADDITIONAL,
    ""
};

// module info
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

// event handler
BRAHMS_DLL_EXPORT brahms::Symbol EventHandler(brahms::Event* event)
{
    brahms::Symbol result = S_NULL;

    try
    {

#ifdef MODULE_SHOW_EVENTS
        cout << "BRAHMS (INFO): " << brahms::getSymbolString(event->type)
             << " (ENTER) " << COMPONENT_CLASS_STRING << endl;
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
            // this covers all the events we know about, but also covers
            // any events we don't know about (events that are added later
            // to the specification). we just pass them on to the object,
            // if they are object-level events, but we check for an instance
            // before we pass them on, obviously! if they are module-level
            // events, and we don't recognise them, best we can do is ignore
            // them.

            if (event->type <= EVENT_MODULE_MAX)
            {
                // ignore, result is S_NULL (unhandled)
            }

            else
            {
                COMPONENT_CLASS_CPP* object = (COMPONENT_CLASS_CPP*) event->object;
                if (!object) return E_NO_INSTANCE;
                result = object->event(event);
            }

            // ok
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
    cout << "BRAHMS (INFO): " << brahms::getSymbolString(event->type)
         << " (LEAVE with result " << brahms::getSymbolString(result) << ") "
         << COMPONENT_CLASS_STRING << endl;
#endif

    return result;
}

#endif // _BRAHMS_1199_IMPL_H_
