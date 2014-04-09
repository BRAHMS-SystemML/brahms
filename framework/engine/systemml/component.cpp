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

	$Id:: component.cpp 2406 2009-11-19 20:53:19Z benjmitch    $
	$Rev:: 2406                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-19 20:53:19 +0000 (Thu, 19 Nov 2009)       $
________________________________________________________________

*/




#include "systemml.h"


using namespace brahms::output;


namespace brahms
{
	namespace systemml
	{

		Component::Component(
			string p_name,
			EngineData& p_engineData,
			brahms::output::Source* p_tout,
			string p_desiredClassName,
			UINT16 p_desiredRelease,
			ComponentType p_type
		)
			:
			RegisteredObject(p_type, p_name),
			engineData(p_engineData),
			desiredClassName(p_desiredClassName),
			desiredRelease(p_desiredRelease),
			tout(p_tout)
		{
			////////////////////////////////////////////////////////////////////////
			//	ZEROED IN CONSTRUCTOR
			____CLEAR(componentTime);
			componentInfo = NULL;
			object = NULL;
			module = NULL;
			flags = 0;

			////////////////////////////////////////////////////////////////////////
			//	INITIALISED IN CONSTRUCTOR
			componentData.name = getObjectName().c_str();
			componentData.time = &componentTime;
		}

		Component::Component(const Component& copy)
			:
			RegisteredObject(copy.getObjectType(), copy.getObjectName()),
			engineData(copy.engineData),
			desiredClassName(copy.desiredClassName),
			desiredRelease(copy.desiredRelease)
		{
			//	can only copy if already instantiated
			if (!copy.module)
				ferr << E_INTERNAL << "Component() copy ctor before instantiate()";

			////////////////////////////////////////////////////////////////////////
			//	COPIED IN CONSTRUCTOR
			componentTime = copy.componentTime;
			componentInfo = copy.componentInfo;
			object = NULL;				/* NOTE the object field is not set by copy ctor, since it needs to be created anew through EVENT_MODULE_DUPLICATE */
			module = copy.module;
			flags = copy.flags;
			tout = NULL; // always start with a null tout (only data components are copied, anyway)

			////////////////////////////////////////////////////////////////////////
			//	INITIALISED IN CONSTRUCTOR
			componentData.name = getObjectName().c_str();
			componentData.time = &componentTime;
		}

		void Component::load(brahms::output::Source& tout)
		{
			/*

				WHY is load() a separate call at all? Because during System
				instantiation, we create a Process object for each process
				in the system. However, if we're running Concerto, some of
				these Processes will actually be computed in other voices.
				These, we will destroy before we ever load them.

				Conversely, we only instantiate Data and Utility that we *are*
				going to use, so those code branches call load immediately
				after operator new.

			*/

			//	check
			if (module) ferr << E_INTERNAL << "load() called twice";

			//	load release 0 (in future, we'll use ICI to infer the release)
			componentCreateS.componentClass = desiredClassName;
			module = engineData.loader.loadComponent(componentCreateS, tout, &engineData.systemInfo, desiredRelease);
		}

		const char* ComponentTypeString(ObjectType ot)
		{
			switch(ot)
			{
				case CT_DATA: return "Data";
				case CT_PROCESS: return "Process";
				case CT_UTILITY: return "Utility";
				default: return "<Unknown Component Type>";
			}
		}

		ObjectType Component::instantiate(brahms::output::Source* tout)
		{
			//	load if not already loaded
			if (!module) ferr << E_INTERNAL << "instantiate() before load()";
			if (object) ferr << E_INTERNAL << "instantiate() called twice";

			//	event data
			EventModuleCreateBindings data;
			____CLEAR(data);
			data.hComponent = getObjectHandle();
			data.data = &componentData;

			//	data about the wrapped function, only known about and used by bindings
			data.wrapped.namespaceRootPath = componentCreateS.namespaceRootPath.c_str();
			data.wrapped.componentClass = desiredClassName.c_str();
			data.wrapped.releasePath = componentCreateS.releasePath.c_str();
			data.wrapped.moduleFilename = componentCreateS.moduleFilename.c_str();

			//	event
			brahms::EventEx event(
					EVENT_MODULE_CREATE,
					0,
					this,
					&data,
					true,
					tout
				);
			event.fire();

			//	check it instantiated ok
			if (!event.object)
				ferr << E_NOT_COMPLIANT << "\"" << desiredClassName << "\" failed to instantiate component";

			//	store newly created object and component info
			object = event.object;
			componentInfo = data.info;

			//	check info is valid right now, to avoid seg faults later
			if (!data.info) ferr << E_NOT_COMPLIANT << "\"" << desiredClassName << "\" did not return ComponentInfo";
			if (!data.info->cls) ferr << E_NOT_COMPLIANT << "\"" << desiredClassName << "\" did not set \"cls\" in ComponentInfo";
			ObjectType ret = data.info->type;

			//	check type is right
			if ((getObjectType() != CT_COMPONENT) && (ret != getObjectType()))
			{
				ferr << E_WRONG_COMPONENT_TYPE << "instantiated \"" << desiredClassName << "\" expecting a "
					<< ComponentTypeString(getObjectType()) << ", but it was a " << ComponentTypeString(ret);
			}

			if (!data.info->componentVersion) ferr << E_NOT_COMPLIANT << "\"" << desiredClassName << "\" did not set \"componentVersion\" in ComponentInfo";
			if (!data.info->additional) ferr << E_NOT_COMPLIANT << "\"" << desiredClassName << "\" did not set \"additional\" in ComponentInfo";
			if (!data.info->libraries) ferr << E_NOT_COMPLIANT << "\"" << desiredClassName << "\" did not set \"libraries\" in ComponentInfo";

			//	spec must match release (but we don't know or care about the developer revision)
			bool correctSpec =
				data.info->cls == desiredClassName &&
				data.info->componentVersion->release == desiredRelease;
			if (!correctSpec)
			{
				//	check for case-sensitive error
				if (data.info->cls != desiredClassName && brahms::text::strimatch(data.info->cls, desiredClassName) == 0)
					(*tout) << "requested component is case-insensitive match for instantiated component: have you got your cases right?" << D_WARN;
				ferr << E_NOT_COMPLIANT << "supplied component (" << brahms::text::info2string(data.info)
					<< ") does not support requested component (" << desiredClassName << "-" << desiredRelease << ")";
			}

			//	ok
			return ret;
		}

		void Component::destroy(brahms::output::Source* tout)
		{
			//	load if not already loaded
			//if (!module) ferr << E_INTERNAL << "destroy() before load()";

			//	IGNORE THE ABOVE - the module may never have loaded, that's no reason to panic

			//if (!object) ferr << E_INTERNAL << "destroy() before instantiate() (or called twice)";
			//	only destroy if it was successfully instantiated
			if (object)
			{
				//	fire event
				brahms::EventEx event(
					EVENT_MODULE_DESTROY,
					0,
					this,
					NULL,
					false,
					tout
				);
				event.fire();

				//	mark object as destroyed
				object = NULL;
			}
		}

		Component::~Component()
		{
			if (object)
				____WARN("component dtor called when object has not been destroy()ed");
		}

		void Component::setName(string p_name)
		{
			setObjectName(p_name);
			componentData.name = getObjectName().c_str();
		}

		void Component::setSampleRate(SampleRate p_sampleRate)
		{
			componentTime.sampleRate = p_sampleRate;
		}

		SampleRate Component::getSampleRate()
		{
			return componentTime.sampleRate;
		}

		string Component::getName()
		{
			return getObjectName();
		}

		string Component::getClassName()
		{
			return desiredClassName;
		}

		UINT16 Component::getRelease()
		{
			return desiredRelease;
		}

		void* Component::getObject()
		{
			return object;
		}

		void Component::setFlag(UINT32 flag)
		{
			flags |= flag;
		}

		void Component::finalizeComponentTime(SampleRate baseSampleRate, BaseSamples executionStop)
		{
			//	get rate
			SampleRate rsr = componentTime.sampleRate;

			//	BSR has a numerator >= all SRs and a denominator <= all SRs (see find() algorithm)
			//	which two facts allow us to the division succinctly
			UINT64 divisor = (baseSampleRate.num / rsr.num) * (rsr.den / baseSampleRate.den);

			//	lay in to component
			componentTime.baseSampleRate = baseSampleRate;
			componentTime.executionStop = executionStop;
			componentTime.samplePeriod = divisor;
			componentTime.now = 0;

			//	fout
			(engineData.core.caller.tout) << getObjectName() << ": "
				<< "(" << brahms::text::sampleRateToString(baseSampleRate) << ")Hz / " << divisor << " = "
				<< "(" << brahms::text::sampleRateToString(componentTime.sampleRate) << ")Hz" << D_VERB;
		}





	}
}
