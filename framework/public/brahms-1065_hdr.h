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

	        void operator=(const Component& src)
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
