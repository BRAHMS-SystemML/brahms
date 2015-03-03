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

	$Id:: brahms-1065.h 2442 2009-12-14 10:52:42Z benjmitch    $
	$Rev:: 2442                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-12-14 10:52:42 +0000 (Mon, 14 Dec 2009)       $
________________________________________________________________

*/

//	The second section, the CPP section, gets processed the
//	*second* time this file is included.
////////////////////////////////////////////////////////////////

#undef name
/*
	Revert "Event" to meaning the one defined in the component interface.
*/

#undef Event
#undef time



/*

	For native components (components not loaded through a language binding) the
	ComponentInfo structure is identical for all components instantiated by a
	module, so we can just maintain it at module level and return a pointer to
	it when requested (see EVENT_MODULE_CREATE/EVENT_MODULE_DESTROY in the cpp).

*/

//	legacy
#ifdef __CLASS_CPP__
#ifndef COMPONENT_CLASS_CPP
#define COMPONENT_CLASS_CPP __CLASS_CPP__
#endif
#endif
#ifdef MODULE_CLASS_STRING
#ifndef COMPONENT_CLASS_STRING
#define COMPONENT_CLASS_STRING MODULE_CLASS_STRING
#endif
#endif
#ifdef __CLASS_STRING__
#ifndef COMPONENT_CLASS_STRING
#define COMPONENT_CLASS_STRING __CLASS_STRING__
#endif
#endif
#ifdef MODULE_CT_TYPE
#define COMPONENT_TYPE MODULE_CT_TYPE
#endif
#ifdef MODULE_COMPONENT_FLAGS
#define COMPONENT_FLAGS MODULE_COMPONENT_FLAGS
#endif
#ifdef MODULE_COMPONENT_ADDITIONAL
#define COMPONENT_ADDITIONAL MODULE_COMPONENT_ADDITIONAL
#endif
#ifdef MODULE_MODULE_FLAGS
#define MODULE_FLAGS MODULE_MODULE_FLAGS
#endif
#define COMPONENT_VERSION MODULE_VERSION_COMPONENT




//	engine version
const FrameworkVersion MODULE_VERSION_ENGINE = VERSION_ENGINE;

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
ComponentInfo COMPONENT_INFO =
{
	COMPONENT_CLASS_STRING,
	COMPONENT_TYPE,
	&COMPONENT_VERSION,
	COMPONENT_FLAGS,
	COMPONENT_ADDITIONAL,
	""
};

//	module info
ModuleInfo MODULE_INFO =
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
BRAHMS_DLL_EXPORT Symbol EventHandler(Event* event)
{
	Symbol result = S_NULL;

#ifdef MODULE_SHOW_EVENTS
	cerr << "BRAHMS (INFO): " << getSymbolString(event->type) << " (ENTER) " << MODULE_SHOW_EVENTS << endl;
#endif

	try
	{

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
				EventModuleInit* init = (EventModuleInit*) event->data;
				executionInfo = init->executionInfo;
				init->moduleInfo = &MODULE_INFO;
				result = C_OK;
				break;
			}

			case EVENT_MODULE_TERM:
			{
				result = C_OK;
				break;
			}

			case EVENT_MODULE_CREATE:
			{
				EventModuleCreate* emc = (EventModuleCreate*) event->data;
				emc->info = &COMPONENT_INFO;

				COMPONENT_CLASS_CPP* object = new COMPONENT_CLASS_CPP;
				event->object = object;

				object->initialize(emc->hComponent, emc->data);

				Event1065 eevent = newEvent(EVENT_EXECPARS);
				object->xmlNode = XMLNode(executionInfo->executionParameters);
				eevent.xmlNode = &object->xmlNode;
				object->event(eevent);

				//	response may be NULL or OK, but we're still OK
				if (S_ERROR(eevent.response)) result = eevent.response;
				else result = C_OK;

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
				EventModuleCreate* emc = (EventModuleCreate*) event->data;
				emc->info = &COMPONENT_INFO;

				COMPONENT_CLASS_CPP* object = (COMPONENT_CLASS_CPP*) event->object;
				if (!object) return E_NO_INSTANCE;

				COMPONENT_CLASS_CPP* newObject = new COMPONENT_CLASS_CPP(*object);
				event->object = newObject;

				newObject->initialize(emc->hComponent, emc->data);

				Event1065 eevent = newEvent(EVENT_EXECPARS);
				object->xmlNode = XMLNode(executionInfo->executionParameters);
				eevent.xmlNode = &object->xmlNode;
				object->event(eevent);

				//	response may be NULL or OK, but we're still OK
				if (S_ERROR(eevent.response)) result = eevent.response;
				else result = C_OK;

				break;
			}

			default:
			{
				//	ignore unrecognised module-level events so that if new
				//	events are added to the specification, we don't fall over
				if (event->type <= EVENT_MODULE_MAX)
				{
					break;
				}

				COMPONENT_CLASS_CPP* object = (COMPONENT_CLASS_CPP*) event->object;
				if (!object)
				{
					result = E_NO_INSTANCE;
					break;
				}
				std::vector<Argument> cf_args;

				Event1065 eevent = newEvent(event->type);
				eevent.flags = event->flags;
				VUINT32 seed;

				switch(event->type)
				{
					case EVENT_STATE_SET:
					{
						EventStateSet* data = (EventStateSet*) event->data;
						object->xmlNode = XMLNode(data->state);
						eevent.xmlNode = &object->xmlNode;
						seed = getRandomSeed(object->hComponent, 1);
						eevent.state.seed = &seed;
						eevent.state.flags = data->flags;
						eevent.state.precision = PRECISION_NOT_SET;
						break;
					}

					case EVENT_STATE_GET:
					{
						EventStateGet* data = (EventStateGet*) event->data;
						if (data->state)
						{
							object->xmlNode = XMLNode(data->state);
							eevent.xmlNode = &object->xmlNode;
						}
						eevent.state.seed = NULL;
						eevent.state.flags = data->flags;
						eevent.state.precision = data->precision;
						break;
					}

#ifdef COMPONENT_DATA

					case EVENT_CONTENT_SET:
					{
						EventContent* data = (EventContent*) event->data;
						eevent.content = *data;
						break;
					}

					case EVENT_CONTENT_GET:
					{
						break;
					}

					case EVENT_GENERIC_STRUCTURE_GET:
					{
						break;
					}

					case EVENT_GENERIC_STRUCTURE_SET:
					{
						EventGenericStructure* data = (EventGenericStructure*) event->data;
						eevent.access.structure = data->structure;
						break;
					}

					case EVENT_GENERIC_FORM_ADVANCE:
					{
						break;
					}

					case EVENT_GENERIC_FORM_CURRENT:
					{
						break;
					}

					case EVENT_GENERIC_CONTENT_GET:
					{
						break;
					}

					case EVENT_GENERIC_CONTENT_SET:
					{
						EventGenericContent* data = (EventGenericContent*) event->data;
						eevent.access.real = data->real;
						eevent.access.imag = data->imag;
						eevent.access.bytes = data->bytes;
						break;
					}

					case EVENT_LOG_SERVICE:
					{
						EventLog* data = (EventLog*) event->data;
						eevent.log = *data;
						break;
					}

#endif

#ifdef COMPONENT_PROCESS_OR_DATA

					case EVENT_LOG_INIT:
					{
						EventLog* data = (EventLog*) event->data;
						eevent.log = *data;
						break;
					}

					case EVENT_LOG_TERM:
					{
						EventLog* data = (EventLog*) event->data;
						eevent.log = *data;
						break;
					}

#endif

#ifdef COMPONENT_PROCESS

					case EVENT_INIT_PRECONNECT:
					{
						break;
					}

					case EVENT_INIT_CONNECT:
					{
						break;
					}

					case EVENT_INIT_POSTCONNECT:
					{
						break;
					}

					case EVENT_RUN_PLAY:
					{
						break;
					}

					case EVENT_RUN_RESUME:
					{
						break;
					}

					case EVENT_RUN_SERVICE:
					{
						break;
					}

					case EVENT_RUN_PAUSE:
					{
						break;
					}

					case EVENT_RUN_STOP:
					{
						break;
					}

#endif

#ifdef COMPONENT_UTILITY

					case EVENT_FUNCTION_GET:
					{
						EventFunctionGet* data = (EventFunctionGet*) event->data;
						eevent.function.name = data->name;
						break;
					}

					case EVENT_FUNCTION_CALL:
					{
						EventFunctionCall* data = (EventFunctionCall*) event->data;
						eevent.function.handle = data->handle;
						eevent.function.args = &cf_args;
						cf_args.resize(data->count);
						for (UINT32 a=0; a<data->count; a++)
							cf_args[a] = *data->args[a];
						break;
					}

#endif

					default:
					{
						//	new events may be added in future, and these pre-compiled binaries
						//	should not fall over as a result. therefore, we ignore events
						//	we don't recognise
					}
				}

				//	fire object
				object->event(eevent);

				if (S_ERROR(eevent.response))
				{
					result = eevent.response;
					break;
				}

				switch(event->type)
				{
					case EVENT_STATE_SET:
					{
						break;
					}

					case EVENT_STATE_GET:
					{
						EventStateGet* data = (EventStateGet*) event->data;
						data->state = eevent.xmlNode->element;
						break;
					}

#ifdef COMPONENT_DATA

					case EVENT_CONTENT_SET:
					{
						EventContent* data = (EventContent*) event->data;
						break;
					}

					case EVENT_CONTENT_GET:
					{
						ec = eevent.content;
						event->data = &ec;
						break;
					}

					case EVENT_GENERIC_STRUCTURE_GET:
					{
						object->eas.structure = eevent.access.structure;
						event->data = &object->eas;
						break;
					}

					case EVENT_GENERIC_STRUCTURE_SET:
					{
						break;
					}

					case EVENT_GENERIC_FORM_ADVANCE:
					case EVENT_GENERIC_FORM_CURRENT:
					{
						object->eaf.form = eevent.access.form;
						object->eaf.type = eevent.access.type;
						object->eaf.dims = eevent.access.dims;
						event->data = &object->eaf;
						break;
					}

					case EVENT_GENERIC_CONTENT_GET:
					{
						object->eac.real = eevent.access.real;
						object->eac.imag = eevent.access.imag;
						object->eac.bytes = eevent.access.bytes;
						event->data = &object->eac;
						break;
					}

					case EVENT_GENERIC_CONTENT_SET:
					{
						break;
					}

					case EVENT_LOG_SERVICE:
					{
						break;
					}

#endif

#ifdef COMPONENT_PROCESS_OR_DATA

					case EVENT_LOG_INIT:
					{
						break;
					}

					case EVENT_LOG_TERM:
					{
						EventLog* data = (EventLog*) event->data;
						//*data = eevent.log;
						if (eevent.xmlNode) data->result = eevent.xmlNode->element;
						else data->result = S_NULL;
						break;
					}

#endif

#ifdef COMPONENT_PROCESS

					case EVENT_INIT_PRECONNECT:
					{
						break;
					}

					case EVENT_INIT_CONNECT:
					{
						break;
					}

					case EVENT_INIT_POSTCONNECT:
					{
						break;
					}

					case EVENT_RUN_PLAY:
					{
						break;
					}

					case EVENT_RUN_RESUME:
					{
						break;
					}

					case EVENT_RUN_SERVICE:
					{
						break;
					}

					case EVENT_RUN_PAUSE:
					{
						break;
					}

					case EVENT_RUN_STOP:
					{
						break;
					}

#endif

#ifdef COMPONENT_UTILITY

					case EVENT_FUNCTION_GET:
					{
						EventFunctionGet* data = (EventFunctionGet*) event->data;
						data->handle = eevent.function.handle;
						data->argumentModifyCount = eevent.function.argumentCount;
						break;
					}

					case EVENT_FUNCTION_CALL:
					{
						EventFunctionCall* data = (EventFunctionCall*) event->data;
						for (UINT32 a=0; a<data->count; a++)
							*data->args[a] = cf_args[a];
						data->count = eevent.function.offendingArgument;
						break;
					}

#endif

					default:
					{
						//	new events may be added in future, and these pre-compiled binaries
						//	should not fall over as a result. therefore, we ignore events
						//	we don't recognise
					}

				} // switch(event->type)

				result = eevent.response;
				break;

			} // default

		} // switch(event->type)

	} // try



	catch(Symbol s)
	{
		result = s;
	}

	catch(const char* e)
	{
		EventErrorMessage data;
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
		EventErrorMessage data;
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
	cerr << "BRAHMS (INFO): " << getSymbolString(event->type) << " (LEAVE with result " << getSymbolString(result) << ") " << MODULE_SHOW_EVENTS << endl;
#endif

	return result;

}
