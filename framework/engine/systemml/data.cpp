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

	$Id:: data.cpp 2406 2009-11-19 20:53:19Z benjmitch         $
	$Rev:: 2406                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-19 20:53:19 +0000 (Thu, 19 Nov 2009)       $
________________________________________________________________

*/


#include "systemml.h"


namespace brahms
{
	namespace systemml
	{




		Data::Data(
			string name,
			EngineData& engineData,
			brahms::output::Source* tout,
			string className,
			UINT16 release,
			SampleRate sampleRate
			)
			: Component(name, engineData, tout, className, release, CT_DATA)
		{
			setSampleRate(sampleRate);
			parentPort = NULL;
		}

		Data::~Data()
		{
		}

		Data* Data::duplicate(brahms::output::Source* tout)
		{
			//	load if not already loaded
			if (!module) ferr << E_INTERNAL << "duplicate() before load()";

			Data* newObject = NULL;

			try
			{
				//	copy this object to a new object
				newObject = new Data(*this);

				/*

					Copy Ctor for Component copies "module" from rhs,
					so no need to call load() on new object.

				*/

				//	ask existing component to duplicate to create new component

				//	event data
				EventModuleCreate data;
				____CLEAR(data);
				data.hComponent = newObject->getObjectHandle();
				data.data = &newObject->componentData;

				//	event
				brahms::EventEx event(
						EVENT_MODULE_DUPLICATE,
						0,
						this,
						&data,
						true,
						tout
					);
				event.fire();

				//	check it instantiated ok
				if (!event.object || event.object == object)
					ferr << E_NOT_COMPLIANT << "\"" + desiredClassName + "\" failed to instantiate component";

				//	check info is valid right now, to avoid seg faults later
				if (!data.info) ferr << E_NOT_COMPLIANT << "\"" << desiredClassName << "\" did not return ComponentInfo";
				if (!data.info->cls) ferr << E_NOT_COMPLIANT << "\"" << desiredClassName << "\" did not set \"cls\" in ComponentInfo";
				if (data.info->type == CT_NULL) ferr << E_NOT_COMPLIANT << "\"" << desiredClassName << "\" did not set \"type\" correctly in ComponentInfo";
				if (!data.info->componentVersion) ferr << E_NOT_COMPLIANT << "\"" << desiredClassName << "\" did not set \"componentVersion\" in ComponentInfo";
				if (!data.info->additional) ferr << E_NOT_COMPLIANT << "\"" << desiredClassName << "\" did not set \"additional\" in ComponentInfo";
				if (!data.info->libraries) ferr << E_NOT_COMPLIANT << "\"" << desiredClassName << "\" did not set \"libraries\" in ComponentInfo";

				//	store newly created object and component info
				newObject->object = event.object;
				newObject->componentInfo = data.info;

				//	ok
				return newObject;
			}

			catch(...)
			{
				if (newObject)
					delete newObject;
				throw;
			}
		}



	}
}

