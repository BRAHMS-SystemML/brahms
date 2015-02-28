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

	$Id:: api.cpp 2376 2009-11-15 23:34:44Z benjmitch          $
	$Rev:: 2376                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-15 23:34:44 +0000 (Sun, 15 Nov 2009)       $
________________________________________________________________

*/



/*

	(References)

	GNU Compiler Predefined Macros
		http://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html

	MS Compiler Predefined Macros
		http://msdn.microsoft.com/en-us/library/b0084kay(VS.71).aspx

	C Standard Data Types
		http://www.space.unibe.ch/comp_doc/c_manual/C/CONCEPT/data_types.html

	MS Compiler Fixed Width Data Types
		http://msdn.microsoft.com/en-us/library/29dh1w7z(VS.80).aspx

*/

#include <iostream>
#include "systemml.h"


using namespace brahms::xml;
using namespace brahms::systemml;
using namespace brahms::os;
using namespace brahms::output;




//	we make many of the API calls non-reentrant, for safety's sake
//	this does not imply that it is necessary for any particular call
/*

	NOTE: if we ever allow more than one engine to be created, these
	will block! TODO

*/
Mutex componentAPIMutex;
Mutex engineAPIMutex;

//#define DEBUG_API
//#define USE_MUTEXES



struct APIFunctionScopeObject
{

	APIFunctionScopeObject(string p_function, Mutex& p_mutex)
#ifdef USE_MUTEXES
		: componentAPIMutexLocker(p_mutex)
#endif

	{
		function = p_function;
#ifdef DEBUG_API
		cerr << function << "\n\t{" << endl;
#endif
	}

	~APIFunctionScopeObject()
	{
#ifdef DEBUG_API
		cerr << "\t}" << endl;
#endif
	}

	string function;

#ifdef USE_MUTEXES
	MutexLocker componentAPIMutexLocker;
#endif

};




//#define CATCH_API_E catch(Symbol e) { string func = "engine:"; func += __FUNCTION__; func += "()"; return brahms::error::tracefunc(e, func); }

//	see notes at error.h about global error register
#define CATCH_API_S catch(const exception& se) { brahms::error::Error e(E_STD, se.what()); ____AT(e); return brahms::error::globalErrorRegister.push(e); }
#define CATCH_API_O catch(brahms::error::Error& e) { ____AT(e); return brahms::error::globalErrorRegister.push(e); }
#define CATCH_API_X catch(...) { brahms::error::Error e(E_UNRECOGNISED_EXCEPTION); ____AT(e); return brahms::error::globalErrorRegister.push(e); }
#define CATCH_API CATCH_API_O CATCH_API_S CATCH_API_X


#ifdef DEBUG_API

//	safe
#define COMPONENT_INTERFACE_FUNCTION_SCOPE APIFunctionScopeObject functionScopeObject(__FUNCTION__, componentAPIMutex);

#else

//	high performance
#define COMPONENT_INTERFACE_FUNCTION_SCOPE

#endif



////////////////    NOTES ON C/C++ CONSTANTS

	/*

		In C++, it's best to use "const <type>", but in C this results in
		memory allocation, so "#define" is preferred. To implement this
		without losing the semantics of this file, I've put in tokens for
		constants, and the makefile is used to convert these to the C
		syntax. The raw header includes tokens %XXXX, %[ and ]%. The converted
		header file contains formatted #define/const definitions instead.

	*/



////////////////    GENERAL API

	#define NOT_IMPLEMENTED { try { ferr << E_NOT_IMPLEMENTED << "in " << __FUNCTION__ << "()"; return S_NULL; } CATCH_API }
	#define ASSERT_NON_NULL(a) { if (!(a)) ferr << E_NULL_ARG << #a; }

	Symbol brahms_legacyEngineEvent(EngineEvent* eventData)
	{
		COMPONENT_INTERFACE_FUNCTION_SCOPE

		try
		{
			ASSERT_NON_NULL(eventData);

			//	switch on event type
			switch(eventData->type)
			{
				case ENGINE_EVENT_SET_PORT_NAME:
				{
					if (eventData->flags) ferr << E_INVALID_ARG << "one or more unrecognised flags was passed";
					const char* name = (const char*) eventData->data;
					ASSERT_NON_NULL(name);

					//	hCaller is port
					Symbol hPort = eventData->hCaller;
					OutputPort* port = objectRegister.resolveOutputPort(hPort);

					//	ok
					port->setName(name);
					return C_OK;
				}

				case ENGINE_EVENT_SET_PORT_SAMPLE_RATE:
				{
					if (eventData->flags) ferr << E_INVALID_ARG << "one or more unrecognised flags was passed";
					SampleRate* sampleRate = (SampleRate*) eventData->data;
					ASSERT_NON_NULL(sampleRate);

					//	hCaller is port
					Symbol hPort = eventData->hCaller;
					OutputPort* port = objectRegister.resolveOutputPort(hPort);

					//	ok
					port->setSampleRate(*sampleRate);
					return C_OK;
				}

/*
				case ENGINE_EVENT_PORT_FROM_DATA:
				{
					//	hCaller is data
					Symbol hData = eventData->hCaller;
					Data* dataObject = objectRegister.resolveData(hData);

					//	get port
					Symbol hPort = dataObject->parentPort->getObjectHandle();

					//	data points at somewhere to put it
					Symbol* pPort = (Symbol*) data->data;

					//	ok
					*pPort = hPort;
					break;
				}
*/

				case ENGINE_EVENT_DATA_FROM_PORT:
				{
					if (eventData->flags) ferr << E_INVALID_ARG << "one or more unrecognised flags was passed";
					Symbol* hData = (Symbol*) eventData->data;
					ASSERT_NON_NULL(hData);

					//	hCaller is port
					Symbol hPort = eventData->hCaller;
					Port* port = objectRegister.resolvePort(hPort);

					//	get data
					Data* dataObject = port->getDueData();
					*hData = S_NULL;
					if (dataObject) *hData = dataObject->getObjectHandle();

					//	ok
					return C_OK;
				}

				case ENGINE_EVENT_ASSERT_COMPONENT_SPEC:
				{
					if (eventData->flags) ferr << E_INVALID_ARG << "one or more unrecognised flags was passed";
					ComponentSpec* spec = (ComponentSpec*) eventData->data;
					ASSERT_NON_NULL(spec);

					//	hData is data
					Symbol hData = eventData->hCaller;
					Data* dataObject = objectRegister.resolveData(hData);

					//	assert
					const ComponentInfo* info = dataObject->getComponentInfo();
//						const ComponentData* data = dataObject->getComponentData();
					if (info->cls != std::string(spec->cls))
						ferr << E_INVALID_INPUT << "expected \"" << dataObject->getObjectName() << "\" to be of class \"" << spec->cls << "\"";
					if (info->componentVersion->release < spec->release)
						ferr << E_INVALID_INPUT << "expected \"" << dataObject->getObjectName() << "\" to be of release " << spec->release << " or greater";

					//	ok
					return C_OK;
				}

				case ENGINE_EVENT_FIRE_EVENT_ON_DATA:
				{
					if (eventData->flags) ferr << E_INVALID_ARG << "one or more unrecognised flags was passed";
					HandledEvent* handledEvent = (HandledEvent*) eventData->data;
					ASSERT_NON_NULL(handledEvent);

					//	hCaller is data
					Symbol hData = eventData->hCaller;
					Data* dataObject = objectRegister.resolveData(hData);

					//	get port
//					Port* port = dataObject->parentPort;

					//	attach
					handledEvent->handler = dataObject->module->getHandler();
					handledEvent->event.object = dataObject->object;

					//	assert
					if (!handledEvent->event.object)
						ferr << E_PORT_EMPTY << "port \"" << dataObject->getObjectName() << "\"";

					//	fire event
					Symbol response = handledEvent->handler(&handledEvent->event);
					if (S_ERROR(response)) throw brahms::error::globalErrorRegister.pull(response);

					//	ok
					return C_OK;
				}

				case ENGINE_EVENT_GET_PORT_ON_SET:
				{
					if (eventData->flags) ferr << E_INVALID_ARG << "one or more unrecognised flags was passed";
					EventGetPort* gp = (EventGetPort*) eventData->data;
					ASSERT_NON_NULL(gp);

					//	in legacy version of call, hSet *must* be an output or input set (rather than a process or input set, in the current version)
					Set* set = objectRegister.resolveSet(gp->hSet);

					//	validate
					if (gp->name && gp->index != INDEX_UNDEFINED)
						ferr << E_INVALID_ARG << "both name and index are defined";
					if (!gp->name && gp->index == INDEX_UNDEFINED)
						ferr << E_INVALID_ARG << "neither name nor index is defined";

					//	resolve port
					Port* port = NULL;

					//	by name
					if (gp->name)
					{
						//	get port
						port = set->getPortByName(gp->name);
						if (!port) ferr << E_NOT_FOUND << "port named \"" << gp->name << "\"";
					}

					//	by index
					if (gp->index != INDEX_UNDEFINED)
					{
						//	get port
						port = set->getPortByIndex(gp->index);
						if (!port) ferr << E_NOT_FOUND << "port with index " << gp->index;
					}

					//	return handle
					return port->getObjectHandle();
				}


				case ENGINE_EVENT_HANDLE_UTILITY_EVENT:
				{
					if (eventData->flags) ferr << E_INVALID_ARG << "one or more unrecognised flags was passed";
					EventCreateUtility* data = (EventCreateUtility*) eventData->data;
					if (data->flags) ferr << E_INVALID_ARG << "one or more unrecognised flags was passed";
					ASSERT_NON_NULL(data);

					//Process* process = objectRegister.resolveProcess(eventData->hCaller);

					/*
					this legacy event is used *only* by the legacy interface to std/util/rng,
					and obtains the handled event to a utility that has already been created.
					if that is handled differently, this can be removed.

					//	so... we don't create it (it will be passed in data->hUtility)
					Symbol hUtility = process->createUtility(data);
					if (S_ERROR(hUtility)) return hUtility;
					data->hUtility = hUtility;
					*/

					if (data->handledEvent)
					{
						Utility* utility = objectRegister.resolveUtility(data->hUtility);
						data->handledEvent->handler = utility->module->getHandler();
						data->handledEvent->event.object = utility->object;
					}

					return C_OK;
				}

				default:
				{
					ferr << E_ERROR << "unhandled engine event";
					return C_OK;
				}
			}

			//	ok
			ferr << E_INTERNAL << "fell out of switch";
		}
		CATCH_API;
	}

/*

	inline Symbol brahms_stillActive(Symbol hCaller)
	{
		struct EngineEvent event;
		event.hCaller = hCaller;
		event.flags = 0;
		event.type = ENGINE_EVENT_STILL_ACTIVE;
		event.data = 0;
		return brahms_engineEvent(&event);
	}

	inline Symbol brahms_getRandomSeed(Symbol hCaller, struct EventGetRandomSeed* data, UINT32 flags = 0)
	{
		struct EngineEvent event;
		event.hCaller = hCaller;
		event.flags = flags;
		event.type = ENGINE_EVENT_GET_RANDOM_SEED;
		event.data = (void*) data;
		return brahms_engineEvent(&event);
	}

	inline Symbol brahms_errorMessage(Symbol hCaller, struct EventErrorMessage* data, UINT32 flags = 0)
	{
		struct EngineEvent event;
		event.hCaller = hCaller;
		event.flags = flags;
		event.type = ENGINE_EVENT_ERROR_MESSAGE;
		event.data = (void*) data;
		return brahms_engineEvent(&event);
	}

	inline Symbol brahms_outputMessage(Symbol hCaller, struct EventOutputMessage* data, UINT32 flags = 0)
	{
		struct EngineEvent event;
		event.hCaller = hCaller;
		event.flags = flags;
		event.type = ENGINE_EVENT_OUTPUT_MESSAGE;
		event.data = (void*) data;
		return brahms_engineEvent(&event);
	}
*/

	BRAHMS_ENGINE_VIS Symbol brahms_engineEvent(EngineEvent* eventData)
	{
		/*
		UNDER CONSIDERATION

		%Symbol ENGINE_EVENT_READ_WALLCLOCK	          %[ C_BASE_ENGINE_EVENT + ]%
		%Symbol ENGINE_EVENT_GUI_ACQUIRE              %[ C_BASE_ENGINE_EVENT + ]%
		%Symbol ENGINE_EVENT_GUI_TASK                 %[ C_BASE_ENGINE_EVENT + ]%
		%Symbol ENGINE_EVENT_GUI_PROGRESS             %[ C_BASE_ENGINE_EVENT + ]%
		*/

		COMPONENT_INTERFACE_FUNCTION_SCOPE

		try
		{
			ASSERT_NON_NULL(eventData);

			//	switch on event type
			switch(eventData->type)
			{
				case ENGINE_EVENT_STILL_ACTIVE:
				{
					if (!eventData->hCaller) ferr << E_INVALID_ARG << "must supply hCaller";
					if (eventData->flags) ferr << E_INVALID_ARG << "one or more unrecognised flags was passed";

					Process* process = objectRegister.resolveProcess(eventData->hCaller);
					process->thread->signalActive();

					return C_OK;
				}

				case ENGINE_EVENT_CREATE_UTILITY:
				{
					if (!eventData->hCaller) ferr << E_INVALID_ARG << "must supply hCaller";
					if (eventData->flags) ferr << E_INVALID_ARG << "one or more unrecognised flags was passed";
					EventCreateUtility* data = (EventCreateUtility*) eventData->data;
					if (data->flags) ferr << E_INVALID_ARG << "one or more unrecognised flags was passed";
					ASSERT_NON_NULL(data);

					Process* process = objectRegister.resolveProcess(eventData->hCaller);
					Symbol hUtility = process->createUtility(data);
					if (S_ERROR(hUtility)) return hUtility;
					data->hUtility = hUtility;

					if (data->handledEvent)
					{
						Utility* utility = objectRegister.resolveUtility(data->hUtility);
						data->handledEvent->handler = utility->module->getHandler();
						data->handledEvent->event.object = utility->object;
					}

					return C_OK;
				}

				case ENGINE_EVENT_GET_RANDOM_SEED:
				{
					if (!eventData->hCaller) ferr << E_INVALID_ARG << "must supply hCaller";
					if (eventData->flags) ferr << E_INVALID_ARG << "one or more unrecognised flags was passed";
					EventGetRandomSeed* data = (EventGetRandomSeed*) eventData->data;
					if (data->flags) ferr << E_INVALID_ARG << "one or more unrecognised flags was passed";
					ASSERT_NON_NULL(data);

					Process* process = objectRegister.resolveProcess(eventData->hCaller);

					return process->__getRandomSeed(data);
				}

				case ENGINE_EVENT_ERROR_MESSAGE:
				{
					//	allowed to not supply this, for this event
					//if (!eventData->hCaller) ferr << E_INVALID_ARG << "must supply hCaller";

					if (eventData->flags) ferr << E_INVALID_ARG << "one or more unrecognised flags was passed";
					EventErrorMessage* data = (EventErrorMessage*) eventData->data;
					if (data->flags & ~(F_TRACE | F_DEBUGTRACE)) ferr << E_INVALID_ARG << "one or more unrecognised flags was passed";
					ASSERT_NON_NULL(data);

					//	if no message, return it unchanged
					if (!data->msg) return data->error;

					//	check it's an error
					if (!S_ERROR(data->error))
						ferr << E_INVALID_ARG << "symbol does not resolve to an error";

					//	if error has no GUID, then it's not a registered error, and we have to create it
					if (!(data->error & S_GUID_MASK))
					{
						brahms::error::Error e(data->error);
						data->error = brahms::error::globalErrorRegister.push(e);
					}

					//	do trace
					if (data->flags & F_TRACE)
						brahms::error::globalErrorRegister.borrow(data->error).trace(data->msg);
					else if (data->flags & F_DEBUGTRACE)
						brahms::error::globalErrorRegister.borrow(data->error).debugtrace(data->msg);
					else
					{
						if (brahms::error::globalErrorRegister.borrow(data->error).msg.length())
							brahms::error::globalErrorRegister.borrow(data->error).msg += ", ";
						brahms::error::globalErrorRegister.borrow(data->error).msg += data->msg;
					}

					//	we can return the error directly, since it is the requested augmented
					//	error. if an error occurs during this function, the caller will get
					//	that error instead, indicating that they are misusing this function.
					return data->error;
				}

				case ENGINE_EVENT_OUTPUT_MESSAGE:
				{
					if (!eventData->hCaller) ferr << E_INVALID_ARG << "must supply hCaller";
					if (eventData->flags) ferr << E_INVALID_ARG << "one or more unrecognised flags was passed";
					EventOutputMessage* data = (EventOutputMessage*) eventData->data;
					if (data->flags) ferr << E_INVALID_ARG << "one or more unrecognised flags was passed";
					ASSERT_NON_NULL(data);

					//	resolve object
					RegisteredObject* object = objectRegister.resolve(eventData->hCaller, CT_COMPONENT | CT_ENGINE);

					//	force level to some known value
					if (!(data->level == D_WARN || data->level == D_INFO || data->level == D_VERB || data->level == D_FULL))
						data->level = D_INFO;

					//	resolve detail level
					EnumDetailLevel dlevel = (EnumDetailLevel) data->level;

					//	find target
					switch (object->getObjectType())
					{
						/*
						if we want to let the engine client call this, we use this code:

						case CT_ENGINE:
						{
							Engine* engine = (Engine*) object;
							engine->engineData.core.caller.tout << data->msg << dlevel;
							break;
						}
						*/

						case CT_PROCESS:
						case CT_UTILITY:
						case CT_DATA:
						case CT_COMPONENT:
						{
							Component* component = (Component*) object;
							brahms::output::Source* tout = component->tout;
							if (tout) (*tout) << data->msg << dlevel;
							else
							{
								stringstream ss;
								ss << "(BRAHMS) COMPONENT MESSAGE LOST: " << data->msg << " (" << dlevel << ")\n";
								cerr << ss.str().c_str();
							}
							break;
						}

						default:
						{
							/*

								For now, messages from non-process components will raise.

								We need to handle messages from data/utility objects too! But that
								means knowing which thread they have been fired from. One way is to
								look at who calls them. Utilities are only ever fired by their owner
								process, so they can be assumed to be running in the same thread as
								that. Data are only called by the framework at run-time by the thread
								owning their writing process - but they can also generate messages
								when the reading process calls events on them, which would be in a
								different thread (potentially). Another way is to tag them in different
								phases (that is, when datas are locked into the read buffer of a process,
								they are tagged so that any messages they generate go to that process's
								thread, and vice versa when they are acting as a write buffer). Disadvantage
								is that it might be a performance hit, but small I suspect, something
								like:

									enum DataThreadContext
									{
										DTC_NULL = 0, // don't expect messages from this thread
										DTC_CALLER, // during init and term, we mostly only call from the framework (except during EVENT_INIT_CONNECT?)
										DTC_READER,
										DTC_WRITER
									}
							*/

							ferr << E_INTERNAL << "this type of caller cannot use ENGINE_EVENT_OUTPUT_MESSAGE";
						}
					}

					//	ok
					return C_OK;
				}

				case ENGINE_EVENT_GET_SYMBOL_STRING:
				{
					//	allowed to not supply this, for this event
					//if (!eventData->hCaller) ferr << E_INVALID_ARG << "must supply hCaller";

					if (eventData->flags) ferr << E_INVALID_ARG << "one or more unrecognised flags was passed";
					EventGetSymbolString* data = (EventGetSymbolString*) eventData->data;
					if (data->flags) ferr << E_INVALID_ARG << "one or more unrecognised flags was passed";
					ASSERT_NON_NULL(data);

					data->result = brahms::base::symbol2string(data->symbol);
					return C_OK;
				}

				case ENGINE_EVENT_GET_TYPE_STRING:
				{
					//	allowed to not supply this, for this event
					//if (!eventData->hCaller) ferr << E_INVALID_ARG << "must supply hCaller";

					if (eventData->flags) ferr << E_INVALID_ARG << "one or more unrecognised flags was passed";
					EventGetTypeString* data = (EventGetTypeString*) eventData->data;
					if (data->flags) ferr << E_INVALID_ARG << "one or more unrecognised flags was passed";
					ASSERT_NON_NULL(data);

					data->result = brahms::text::type2string(data->type);
					return C_OK;
				}

				case ENGINE_EVENT_GET_SET:
				{
					if (!eventData->hCaller) ferr << E_INVALID_ARG << "must supply hCaller";
					if (eventData->flags) ferr << E_INVALID_ARG << "one or more unrecognised flags was passed";
					EventGetSet* data = (EventGetSet*) eventData->data;
					ASSERT_NON_NULL(data);

					ASSERT_NON_NULL(data->name);
					if (data->flags & (~(F_IIF | F_OIF)))
						ferr << E_INVALID_ARG << "some flags were unrecognised";

					Process* process = objectRegister.resolveProcess(eventData->hCaller);

					//	get set
					Set* set = process->getSetByName(data->flags, data->name);
					if (!set) ferr << E_NOT_FOUND << "set \"" << data->name << "\" not found";

					//	return handle
					return set->getObjectHandle();
				}

				case ENGINE_EVENT_GET_SET_INFO:
				{
					if (!eventData->hCaller) ferr << E_INVALID_ARG << "must supply hCaller";
					if (eventData->flags) ferr << E_INVALID_ARG << "one or more unrecognised flags was passed";
					EventGetSetInfo* data = (EventGetSetInfo*) eventData->data;
					ASSERT_NON_NULL(data);

					//	resolve object
					Set* set = objectRegister.resolveSet(data->hSet);

					//	return result
					set->getSetInfo(data);

					return C_OK;
				}

				case ENGINE_EVENT_GET_PORT_INFO:
				{
					if (!eventData->hCaller) ferr << E_INVALID_ARG << "must supply hCaller";
					if (eventData->flags) ferr << E_INVALID_ARG << "one or more unrecognised flags was passed";
					EventGetPortInfo* data = (EventGetPortInfo*) eventData->data;
					ASSERT_NON_NULL(data);

					//	resolve object
					Port* port = objectRegister.resolvePort(data->hPort);

					//	return result
					port->getPortInfo(data);

					return C_OK;
				}

				case ENGINE_EVENT_GET_PORT:
				{
					if (!eventData->hCaller) ferr << E_INVALID_ARG << "must supply hCaller";
					if (eventData->flags) ferr << E_INVALID_ARG << "one or more unrecognised flags was passed";
					EventGetPort* data = (EventGetPort*) eventData->data;
					ASSERT_NON_NULL(data);

					//	idiom allows hSet to be hComponent, if default set is targeted
					RegisteredObject* object = objectRegister.resolve(data->hSet, CT_PROCESS | CT_INPUT_SET);
					InputSet* set = NULL;
					if (object->getObjectType() == CT_PROCESS)
					{
						Process* process = (Process*) object;
						set = (InputSet*) process->getSetByIndex(F_IIF, 0);
					}
					else
					{
						set = (InputSet*) object;
					}

					//	validate
					if (data->name && data->index != INDEX_UNDEFINED)
						ferr << E_INVALID_ARG << "both name and index are defined";
					if (!data->name && data->index == INDEX_UNDEFINED)
						ferr << E_INVALID_ARG << "neither name nor index is defined";

					//	resolve port
					Port* port = NULL;

					//	by name
					if (data->name)
					{
						//	get port
						port = set->getPortByName(data->name);
						if (!port) ferr << E_NOT_FOUND << "port named \"" << data->name << "\"";
					}

					//	by index
					if (data->index != INDEX_UNDEFINED)
					{
						//	get port
						port = set->getPortByIndex(data->index);
						if (!port) ferr << E_NOT_FOUND << "port with index " << data->index;
					}

					//	attach event
					if (data->handledEvent)
					{
						//	attach
						port->attach(data->handledEvent, data->flags, &data->spec);
					}

					//	return handle
					return port->getObjectHandle();
				}

				case ENGINE_EVENT_ADD_PORT:
				{
					if (!eventData->hCaller) ferr << E_INVALID_ARG << "must supply hCaller";
					if (eventData->flags) ferr << E_INVALID_ARG << "one or more unrecognised flags was passed";
					EventAddPort* data = (EventAddPort*) eventData->data;
					ASSERT_NON_NULL(data);

					//	index must (currently) be INDEX_UNDEFINED
					if (data->index != INDEX_UNDEFINED)
						ferr << E_INVALID_ARG << "index must (currently) be INDEX_UNDEFINED";

					//	idiom allows hSet to be hComponent, if default set is targeted
					RegisteredObject* object = objectRegister.resolve(data->hSet, CT_PROCESS | CT_OUTPUT_SET);
					OutputSet* set = NULL;
					if (object->getObjectType() == CT_PROCESS)
					{
						Process* process = (Process*) object;
						set = (OutputSet*) process->getSetByIndex(F_OIF, 0);
					}
					else set = (OutputSet*) object;

					//	port
					OutputPort* port = NULL;

					//	either cls is non-NULL
					if (data->spec.cls)
					{
						//	but not both
						if (data->hPortToCopy) ferr << E_INVALID_ARG << "cls and hPortToCopy are both non-NULL";

						//	return result
						port = set->addPort(data->spec.cls, data->spec.release);
					}

					//	or hData is non-NULL
					else if (data->hPortToCopy)
					{
						//	but not both
						if (data->spec.cls) ferr << E_INVALID_ARG << "cls and hPortToCopy are both non-NULL";

						//	resolve object
						Port* copyThisPort = objectRegister.resolvePort(data->hPortToCopy);

						//	resolve to data
						Data* copyThis = copyThisPort->getDueData();
						if (!copyThis) ferr << E_INTERNAL << "copy port when data not due";

						//	return result
						port = set->addPortByCopy(copyThis);
					}

					//	but not neither
					else ferr << E_NULL_ARG << "cls and hPortToCopy are both NULL";

					//	set name
					if (data->name)
						port->setName(data->name);

					//	check transport protocol
					if (data->transportProtocol != C_TRANSPORT_PERIODIC)
						ferr << E_NOT_IMPLEMENTED << "only periodic transport is currently supported";
					TransportDataPeriodic* transportData = (TransportDataPeriodic*) data->transportData;

					//	set sample rate
					if (transportData && transportData->sampleRate.num)
						port->setSampleRate(transportData->sampleRate);

					//	attach handled event, if supplied
					if (data->handledEvent)
					{
						//	attach event to port
						port->attach(data->handledEvent, data->flags);
					}

					//	ok
					return port->getObjectHandle();
				}

/*
				case ENGINE_EVENT_READ_WALLCLOCK:
				{
					ASSERT_NON_NULL(data->data);
					DOUBLE* p_time = (DOUBLE*)data->data;
					*p_time = process->engineData.systemTimer.elapsed();
					return C_OK;
				}
*/

/*
				case ENGINE_EVENT_GET_PROCESS_STATE:
				{
					...

					//	only relative name is passed, so we have to construct an absolute name
					string absoluteIdentifier = getComponentName(eventData->hCaller);
					size_t pos = absoluteIdentifier.rfind("/");
					if (pos == string::npos) absoluteIdentifier = data->identifier;
					else absoluteIdentifier = absoluteIdentifier.substr(0, pos+1) + data->identifier;

					//	find object with that absolute SystemML identifier
					for (UINT32 p=0; p<wrappers.size(); p++)
					{
						if (wrappers[p]->name == absoluteIdentifier)
						{
							//	check class is as expected
							if (wrappers[p]->className != data->cls)
							{
								return brahms::error::attach(
									E_MISMATCH,
									"component \"" + string(getComponentName(data->hCaller)) + "\" requested the StateML of process \"" + absoluteIdentifier + "\"; the latter was found but was not of the expected class \"" + data->cls + "\" (its class was \"" + wrappers[p]->className + "\")"
								);
							}

							//	did they want fresh data...?
							if (data->flags & F_FRESH)
							{
								//	fire EVENT_STATE_GET on process
								EventStateGet esg;
								____CLEAR(esg);
								esg.precision = PRECISION_NOT_SET;
								Event moduleEvent = createEvent(EVENT_STATE_GET, 0, wrappers[p]->hProcess, &esg);
								coreFireEvent(wrappers[p]->hProcess, &moduleEvent, true);
								if (!esg.state)
									ferr << E_NOT_COMPLIANT << "process \"" << absoluteIdentifier << "\" claimed to have serviced EVENT_STATE_GET, but responded incorrectly";
								return esg.state;
							}

							//	no, just pass the existing
							else
							{
								if (!wrappers[p]->nodeState) return brahms::error::attach(E_INTERNAL, "wrapper xml node was NULL");
								return brahms::xml::node2handle(wrappers[p]->nodeState);
							}
						}
					}

					//	error
					return brahms::error::attach(E_NOT_FOUND, "component \"" + string(getComponentName(data->hCaller)) + "\" requested the StateML of process \"" + absoluteIdentifier + "\"; the latter was not found (locally, i haven't coded remote access yet)");
				}
*/

				default:
				{
					return brahms_legacyEngineEvent(eventData);
				}
			}

			//	error
			ferr << E_INTERNAL << "fell out of switch";
		}
		CATCH_API
	}





////////////////    XML API

	/* W3C */

	BRAHMS_ENGINE_VIS Symbol xml_createElement(const char* name)
	{
		COMPONENT_INTERFACE_FUNCTION_SCOPE

		try
		{
			XMLNode* node = new XMLNode(name);
			return node->getObjectHandle();
		}
		CATCH_API
	}

	BRAHMS_ENGINE_VIS Symbol xml_setNodeName(Symbol node, const char* name)
	{
		COMPONENT_INTERFACE_FUNCTION_SCOPE

		try
		{
			XMLNode* xnode = objectRegister.resolveXMLNode(node);
			xnode->nodeName(name);
			return C_OK;
		}
		CATCH_API
	}

	BRAHMS_ENGINE_VIS Symbol xml_getNodeName(Symbol node, const char** name)
	{
		COMPONENT_INTERFACE_FUNCTION_SCOPE

		try
		{
			XMLNode* xnode = objectRegister.resolveXMLNode(node);
			*name = xnode->nodeName();
			return C_OK;
		}
		CATCH_API
	}

	BRAHMS_ENGINE_VIS Symbol xml_parentNode(Symbol node)
	{
		COMPONENT_INTERFACE_FUNCTION_SCOPE

		NOT_IMPLEMENTED
	}

	BRAHMS_ENGINE_VIS Symbol xml_childNodes(Symbol node, Symbol* children, UINT32* count)
	{
		COMPONENT_INTERFACE_FUNCTION_SCOPE

		NOT_IMPLEMENTED

/*
		try
		{
			XMLNode* xnode = objectRegister.resolveXMLNode(node);

			//	if not enough space offered, reply with how much space is needed
			UINT32 ccount = xnode->element.children.size();
			if (ccount > *count)
			{
				*count = ccount;
				return E_OVERFLOW;
			}

			//	fill it
			*count = ccount;
			for (UINT32 n=0; n<ccount; n++)
				children[n] = brahms::xml::node2handle(xnode->element.children[n]);
			return C_OK;
		}
		CATCH_API
		*/
	}

	BRAHMS_ENGINE_VIS Symbol xml_firstChild(Symbol node)
	{
		COMPONENT_INTERFACE_FUNCTION_SCOPE

		NOT_IMPLEMENTED
	}

	BRAHMS_ENGINE_VIS Symbol xml_lastChild(Symbol node)
	{
		COMPONENT_INTERFACE_FUNCTION_SCOPE

		NOT_IMPLEMENTED
	}

	BRAHMS_ENGINE_VIS Symbol xml_previousSibling(Symbol node)
	{
		COMPONENT_INTERFACE_FUNCTION_SCOPE

		NOT_IMPLEMENTED
	}

	BRAHMS_ENGINE_VIS Symbol xml_nextSibling(Symbol node)
	{
		COMPONENT_INTERFACE_FUNCTION_SCOPE

		NOT_IMPLEMENTED
	}

	BRAHMS_ENGINE_VIS Symbol xml_insertBefore(Symbol node, Symbol child, Symbol ref)
	{
		COMPONENT_INTERFACE_FUNCTION_SCOPE

		NOT_IMPLEMENTED
	}

	BRAHMS_ENGINE_VIS Symbol xml_replaceChild(Symbol node, Symbol child, Symbol old)
	{
		COMPONENT_INTERFACE_FUNCTION_SCOPE

		NOT_IMPLEMENTED
	}

	BRAHMS_ENGINE_VIS Symbol xml_removeChild(Symbol node, Symbol old)
	{
		COMPONENT_INTERFACE_FUNCTION_SCOPE

		NOT_IMPLEMENTED
	}

	BRAHMS_ENGINE_VIS Symbol xml_appendChild(Symbol node, Symbol child)
	{
		COMPONENT_INTERFACE_FUNCTION_SCOPE

		try
		{
			XMLNode* xnode = objectRegister.resolveXMLNode(node);
			XMLNode* ynode = objectRegister.resolveXMLNode(child);
			xnode->appendChild(ynode);
			return C_OK;
		}
		CATCH_API
	}

	BRAHMS_ENGINE_VIS Symbol xml_hasChildNodes(Symbol node)
	{
		COMPONENT_INTERFACE_FUNCTION_SCOPE

		NOT_IMPLEMENTED
	}

	BRAHMS_ENGINE_VIS Symbol xml_cloneNode(Symbol node, UINT32 deep)
	{
		COMPONENT_INTERFACE_FUNCTION_SCOPE

		NOT_IMPLEMENTED
	}

	BRAHMS_ENGINE_VIS Symbol xml_hasAttributes(Symbol node)
	{
		COMPONENT_INTERFACE_FUNCTION_SCOPE

		NOT_IMPLEMENTED
	}

	BRAHMS_ENGINE_VIS Symbol xml_isSameNode(Symbol node, Symbol other)
	{
		COMPONENT_INTERFACE_FUNCTION_SCOPE

		NOT_IMPLEMENTED
	}

	BRAHMS_ENGINE_VIS Symbol xml_isEqualNode(Symbol node, Symbol other)
	{
		COMPONENT_INTERFACE_FUNCTION_SCOPE

		NOT_IMPLEMENTED
	}

	BRAHMS_ENGINE_VIS Symbol xml_getAttribute(Symbol node, const char* name, const char** value)
	{
		COMPONENT_INTERFACE_FUNCTION_SCOPE

		try
		{
			XMLNode* xnode = objectRegister.resolveXMLNode(node);
			*value = xnode->getAttributeOrNull(name);
			if (*value) return C_OK;
			else return E_NOT_FOUND; // can't add a message, since we'll send lots of these in some cases without throwing an error, and the error register will overflow
		}
		CATCH_API
	}

	BRAHMS_ENGINE_VIS Symbol xml_setAttribute(Symbol node, const char* name, const char* value)
	{
		COMPONENT_INTERFACE_FUNCTION_SCOPE

		try
		{
			XMLNode* xnode = objectRegister.resolveXMLNode(node);
			xnode->setAttribute(name, value);
			return C_OK;
		}
		CATCH_API
	}

	BRAHMS_ENGINE_VIS Symbol xml_removeAttribute(Symbol node, const char* name)
	{
		COMPONENT_INTERFACE_FUNCTION_SCOPE

		NOT_IMPLEMENTED
	}

	BRAHMS_ENGINE_VIS Symbol xml_getElementsByTagName(Symbol node, const char* name, Symbol* children, UINT32* count)
	{
		COMPONENT_INTERFACE_FUNCTION_SCOPE

		NOT_IMPLEMENTED
	}

	BRAHMS_ENGINE_VIS Symbol xml_hasAttribute(Symbol node, const char* name)
	{
		COMPONENT_INTERFACE_FUNCTION_SCOPE

		NOT_IMPLEMENTED
	}


	/* Extensions */

	BRAHMS_ENGINE_VIS Symbol xml_setNodeText(Symbol node, const char* text)
	{
		COMPONENT_INTERFACE_FUNCTION_SCOPE

		try
		{
			XMLNode* xnode = objectRegister.resolveXMLNode(node);
			xnode->nodeText(text);
			return C_OK;
		}
		CATCH_API
	}

	BRAHMS_ENGINE_VIS Symbol xml_getNodeText(Symbol node, const char** text)
	{
		COMPONENT_INTERFACE_FUNCTION_SCOPE

		try
		{
			XMLNode* xnode = objectRegister.resolveXMLNode(node);
			*text = xnode->nodeText();
			return C_OK;
		}
		CATCH_API
	}

	BRAHMS_ENGINE_VIS Symbol xml_clearNode(Symbol node)
	{
		COMPONENT_INTERFACE_FUNCTION_SCOPE

		try
		{
			XMLNode* xnode = objectRegister.resolveXMLNode(node);
			xnode->clear();
			return C_OK;
		}
		CATCH_API
	}

	BRAHMS_ENGINE_VIS Symbol xml_getChild(Symbol node, const char* name, UINT32 index)
	{
		COMPONENT_INTERFACE_FUNCTION_SCOPE

		try
		{
			//	do job
			XMLNode* xnode = objectRegister.resolveXMLNode(node);
			XMLNode* child = xnode->getChildOrNull(name, index);
			if (!child) return E_NOT_FOUND; // can't add a message, since we'll send lots of these in some cases without throwing an error, and the error register will overflow

			//	return handle
			return child->getObjectHandle();
		}
		CATCH_API
	}





////////////////	ENGINE API

#include "api-engine.cpp"
