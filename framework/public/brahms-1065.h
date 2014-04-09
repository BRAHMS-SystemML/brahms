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




//  in 1065, must include these here because we provide legacy calls for their interfaces
#if __BINDING__ == 1065
#ifdef COMPONENT_PROCESS
using namespace brahms;

namespace dev_std_data_numeric = std_2009_data_numeric_0;
namespace dev_std_data_spikes = std_2009_data_spikes_0;
namespace dev_std_util_rng = std_2009_util_rng_0;
#endif
#endif

//	start brahms namespace again
namespace brahms
{




////////////////    CONTRACT (SUCCEED OR THROW) API WRAPPERS

		inline VUINT32 getRandomSeed(Symbol hCaller, UINT32 count = 0)
		{
			VUINT32 seed;
			EventGetRandomSeed rseed;
			rseed.flags = 0;
			rseed.count = count;
			Symbol result = S_NULL;
			if (count)
			{
				seed.resize(count);
				rseed.seed = &seed[0];

				EngineEvent event;
				event.hCaller = hCaller;
				event.flags = 0;
				event.type = ENGINE_EVENT_GET_RANDOM_SEED;
				event.data = (void*) &rseed;
				result = brahms_engineEvent(&event);

				if (S_ERROR(result)) berr << result;
			}
			else
			{
				rseed.seed = NULL;

				EngineEvent event;
				event.hCaller = hCaller;
				event.flags = 0;
				event.type = ENGINE_EVENT_GET_RANDOM_SEED;
				event.data = (void*) &rseed;
				result = brahms_engineEvent(&event);
				if (S_ERROR(result)) berr << result;

				//	if zero, can have whatever we want - we choose 1
				if (!rseed.count) rseed.count = 1;
				seed.resize(rseed.count);
				rseed.seed = &seed[0];

				result = brahms_engineEvent(&event);
				if (S_ERROR(result)) berr << result;
			}
			return seed;
		}



////////////////    LEGACY FUNCTION FORM

	inline Symbol createUtility(Symbol hCaller, const char* cls, UINT16 release, const char* name = NULL)
	{
		EventCreateUtility data;
		data.flags = 0;
		data.hUtility = S_NULL;
		data.spec.cls = cls;
		data.spec.release = release;
		data.name = name;
		data.handledEvent = NULL;

		EngineEvent event;
		event.hCaller = hCaller;
		event.flags = 0;
		event.type = ENGINE_EVENT_CREATE_UTILITY;
		event.data = (void*) &data;

		Symbol result = brahms::brahms_engineEvent(&event);
		if (S_ERROR(result)) berr << result;

		return data.hUtility;
	}



////////////////    EVENT DATA

	struct EventState
	{
		UINT32 flags;
		VUINT32* seed;
		INT32 precision;
	};

	struct EventFunction
	{
		UINT32 handle;          //  how to call this function (OUT)

		union
		{
			//  used when getting function (i.e. in call to EVENT_FUNCTION_GET)
			struct
			{
				const char* name;           //  name of sought function (IN)
				UINT32 argumentCount;   //  number of arguments this function will expect (OUT)
			};

			//  used when calling function
			struct
			{
				std::vector<Argument>* args;
				UINT32 offendingArgument;   //  one-based index of offending argument if arguments were invalid (OUT)
			};
		};
	};

	struct EventAccess
	{
		const char* structure;
		Dimensions dims;
		const void* real;
		const void* imag;
		TYPE type;
		UINT64 bytes;
		UINT32 form;
	};




////////////////    EVENT CLASS

	struct Event1065
	{
		//  common event data
		Symbol type;                    //  event type (IN)
		Symbol response;            //  basic response code (OUT)
		UINT32 flags;                   //  flags (IN/OUT)
		XMLNode* xmlNode;               //  some XML node (IN/OUT)

		//  event-specific data
		union
		{
			EventState state;
			EventContent content;
			EventLog log;
			EventFunction function;
			EventAccess access;

			BYTE __force64[64];
		};
	};

	inline Event1065 newEvent(Symbol type)
	{
		Event1065 event;
		____CLEAR(event);
		event.type = type;
		return event;
	}



////////////////    COMPONENT CLASS

	struct Component
	{
		Component()
		{
			ptime = NULL;
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
			ptime = NULL;
			hComponent = S_NULL;
		}

		Component& operator=(const Component& src)
		{
			berr << E_INTERNAL << "Component() assignment";
		}

		virtual ~Component()
		{
		}

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
		
		void rng_select(Symbol hUtility, const char* generator)
		{
			std_2009_util_rng_0::select(hComponent, hUtility, generator);
		}

		void rng_seed(Symbol hUtility, UINT32* seed, UINT32 count)
		{
			std_2009_util_rng_0::seed(hComponent, hUtility, seed, count);
		}

		void rng_seed(Symbol hUtility, const VUINT32& seed)
		{
			std_2009_util_rng_0::seed(hComponent, hUtility, seed.size() ? &seed[0] : NULL, seed.size());
		}

		void rng_fill(Symbol hUtility, DOUBLE* dst, UINT64 count, DOUBLE gain = 1.0, DOUBLE offset = 0.0)
		{
			std_2009_util_rng_0::fill(hComponent, hUtility, dst, count, gain, offset);
		}

		void rng_fill(Symbol hUtility, SINGLE* dst, UINT64 count, SINGLE gain = 1.0, SINGLE offset = 0.0)
		{
			std_2009_util_rng_0::fill(hComponent, hUtility, dst, count, gain, offset);
		}

		DOUBLE rng_get(Symbol hUtility)
		{
			return std_2009_util_rng_0::get(hComponent, hUtility);
		}

/*
		THESE DON'T SEEM TO BE IN USE, SO THERE'S NO NEED TO SUPPLY THEM ON THIS LEGACY INTERFACE

		std::string rng_get_state(Symbol hUtility)
		{
			TargetedEvent tev = createTargetedEvent(hUtility, dev_std_util_rng::EVENT_GET_STATE, NULL);
			safeFireEvent(&tev);
			return (const char*) tev.event.data;
		}

		void rng_set_state(Symbol hUtility, const char* state)
		{
			TargetedEvent tev = createTargetedEvent(hUtility, dev_std_util_rng::EVENT_SET_STATE, (void*)state);
			safeFireEvent(&tev);
		}
*/

		std_2009_data_numeric_0::Structure* numeric_get_structure(Symbol hData)
		{
			return std_2009_data_numeric_0::get_structure(hComponent, hData);
		}

		void numeric_set_structure(Symbol hData, const std_2009_data_numeric_0::Structure& structure)
		{
			std_2009_data_numeric_0::set_structure(hComponent, hData, structure);
		}

		void numeric_set_structure(Symbol hData, TYPE type, const Dims& dims)
		{
			std_2009_data_numeric_0::set_structure(hComponent, hData, type, dims.cdims());
		}

		void numeric_validate(Symbol hData, TYPE type)
		{
			std_2009_data_numeric_0::validate(hComponent, hData, type);
		}

		void numeric_validate(Symbol hData, TYPE type, const Dims& dims)
		{
			std_2009_data_numeric_0::validate(hComponent, hData, type, dims.cdims());
		}

		void numeric_validate(Symbol hData, const std_2009_data_numeric_0::Structure& structure)
		{
			std_2009_data_numeric_0::validate(hComponent, hData, structure);
		}

		UINT64 numeric_get_content(Symbol hData, const void*& real, const void*& imag)
		{
			return std_2009_data_numeric_0::get_content(hComponent, hData, real, imag);
		}

		const void* numeric_get_content(Symbol hData)
		{
			return std_2009_data_numeric_0::get_content(hComponent, hData);
		}

		void numeric_set_content(Symbol hData, const void* real, const void* imag = NULL, UINT32 bytes = 0)
		{
			std_2009_data_numeric_0::set_content(hComponent, hData, real, imag, bytes);
		}

		Dimensions* spikes_get_dimensions(Symbol hData)
		{
			return std_2009_data_spikes_0::get_dimensions(hComponent, hData);
		}

		void spikes_set_dimensions(Symbol hData, Dimensions* dims)
		{
			std_2009_data_spikes_0::set_dimensions(hComponent, hData, dims);
		}

		UINT32 spikes_get_capacity(Symbol hData)
		{
			return std_2009_data_spikes_0::get_capacity(hComponent, hData);
		}

		void spikes_set_capacity(Symbol hData, UINT32 capacity)
		{
			std_2009_data_spikes_0::set_capacity(hComponent, hData, capacity);
		}

		UINT32 spikes_get_content(Symbol hData, INT32*& state)
		{
			return std_2009_data_spikes_0::get_content(hComponent, hData, state);
		}

		void spikes_set_content(Symbol hData, INT32* state, UINT32 count)
		{
			std_2009_data_spikes_0::set_content(hComponent, hData, state, count);
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
		
		void stillActive()
		{
			EngineEvent event;
			event.hCaller = hComponent;
			event.flags = 0;
			event.type = ENGINE_EVENT_STILL_ACTIVE;
			event.data = 0;
			
			____SUCCESS(brahms_engineEvent(&event));
		}

		virtual void event(Event1065& event) = 0;
		virtual void initialize(Symbol hComponent, const ComponentData* data) = 0;

		std::string processName()
		{
			return pdata->name;
		}

		const ComponentTime* ptime;
		const ComponentData* pdata;
		Symbol hComponent;
		ComponentOut bout;
		std::string error;
		XMLNode xmlNode;

#ifdef COMPONENT_DATA
		//  return objects (see .cpp)
		EventGenericStructure eas;
		EventGenericForm eaf;
		EventGenericContent eac;
		EventContent ec;
#endif
	};



////////////////    COMPONENT SUBCLASSES

	struct Process : public Component
	{
		Process()
		{
		}

		void initialize(Symbol hComponent, const ComponentData* data)
		{
			this->hComponent = hComponent;
			this->pdata = data;
			this->ptime = data->time;
			bout.initialize(hComponent);
			iif.initialize(hComponent, F_IIF);
			oif.initialize(hComponent, F_OIF);
		}

		SystemMLInterface iif;
		SystemMLInterface oif;
	};

	struct Data : public Component
	{
		Data()
		{
		}

		void initialize(Symbol hComponent, const ComponentData* data)
		{
			this->hComponent = hComponent;
			this->pdata = data;
			this->ptime = data->time;
			bout.initialize(hComponent);
		}
	};

	struct Utility : public Component
	{
		Utility()
		{
		}

		void initialize(Symbol hComponent, const ComponentData* data)
		{
			this->hComponent = hComponent;
			this->pdata = data;
			this->ptime = data->time;
			bout.initialize(hComponent);
		}
	};

	#define time (*ptime)



////////////////    LEGACY RESPONSE CODES

	const Symbol RESPONSE_NULL                      = S_NULL;
	const Symbol RESPONSE_OK                        = C_OK;
	const Symbol RESPONSE_STOP_USER_CANCEL          = C_STOP_USER;
	const Symbol RESPONSE_STOP_EXTERNAL_CANCEL      = C_STOP_EXTERNAL;
	const Symbol RESPONSE_STOP_CONDITION_REACHED    = C_STOP_CONDITION;
	const Symbol RESPONSE_STOP_THEREFOREIAM         = C_STOP_THEREFOREIAM;
	const Symbol RESPONSE_FUNCTION_NOT_FOUND        = E_FUNC_NOT_FOUND;
	const Symbol RESPONSE_BAD_ARG_COUNT             = E_BAD_ARG_COUNT;
	const Symbol RESPONSE_BAD_ARG_TYPE              = E_BAD_ARG_TYPE;
	const Symbol RESPONSE_BAD_ARG_SIZE              = E_BAD_ARG_SIZE;



////////////////    LEGACY DEFINES

/*
	struct VersionComponent : public ComponentVersion
	{
		VersionComponent(UINT16 language, UINT16 release, UINT16 revision)
		{
			this->language = language;
			this->release = release;
			this->revision = revision;
		}
	};
*/

	typedef std_2009_data_numeric_0::Structure ExtendedStructure;
	typedef std_2009_data_numeric_0::Structure Structure;

	/*

		"Event" is the event structure in the component interface, but we need it to
		mean something different for legacy (1065) code. Therefore, we
		define it to mean the 1065 event here. We undefine it at the
		top of 1065.cpp, so that it can be used in the normal way there.

	*/

	#define Event Event1065


	/*

		We define a bunch of other old constants, to keep 1065 stuff working,
		but we prefer not to have them in the long term.

	*/

#define VersionFramework FrameworkVersion
#define coreGetElementTypeString getElementTypeString
#define coreGetEventTypeString getSymbolString
#define coreCreateUtility(t, c, r) createUtility(hComponent, c, r, NULL)
#define SAMPLERATE SampleRate
#define BASESAMPLES BaseSamples
#define FLAG_ENCAPSULATED F_ENCAPSULATED
#define FLAG_FIRST_CALL F_FIRST_CALL
#define FLAG_LAST_CALL F_LAST_CALL
#define FLAG_ERROR F_GLOBAL_ERROR
#define FLAG_LOCAL_ERROR F_LOCAL_ERROR
#define FLAG_UNDEFINED F_UNDEFINED
#define FLAG_ZERO F_ZERO
#define FLAG_MODIFIED F_MODIFIED
#define FLAG_FRESH F_FRESH
#define FLAG_IS_LISTENED F_LISTENED
#define FLAG_NEEDS_ALL_INPUTS F_NEEDS_ALL_INPUTS
#define FLAG_INPUTS_SAME_RATE F_INPUTS_SAME_RATE
#define FLAG_OUTPUTS_SAME_RATE F_OUTPUTS_SAME_RATE
#define FLAG_NOT_RATE_CHANGER F_NOT_RATE_CHANGER
#define FLAG_NOT_THREAD_SAFE F_NO_CONCURRENCY
#define EVENT_BASE_PUBLIC C_BASE_USER_EVENT
#define EVENT_BASE_MODULE C_BASE_USER_EVENT;
#define EVENT_EXECPARS (C_BASE_LEGACY + 0x0001)
#define versionComponent COMPONENT_VERSION
#define VERSION_EXPORTED ( -2 )
#define DEFAULT_SET ( -1 )
#define HANDLE_INVALID S_NULL
#define TYPE_NOT_SET TYPE_UNSPECIFIED
#define EVENT_CALL_FUNCTION EVENT_FUNCTION_CALL
#define EVENT_GET_FUNCTION EVENT_FUNCTION_GET
#define MatMLNode DataMLNode




}   //  end namespace









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






////////////////////////////////////////////////////////////////
//
//	The second section, the CPP section, gets processed the
//	*second* time this file is included.
//

#endif
#endif

//
////////////////////////////////////////////////////////////////


