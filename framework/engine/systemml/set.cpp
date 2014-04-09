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

	$Id:: set.cpp 2317 2009-11-07 22:10:20Z benjmitch          $
	$Rev:: 2317                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-07 22:10:20 +0000 (Sat, 07 Nov 2009)       $
________________________________________________________________

*/


#include "systemml.h"

namespace brahms
{
	namespace systemml
	{




	////////////////	SET

		Set::Set(string p_name, EngineData& p_engineData, ComponentType objectType, Process* p_process)
			: RegisteredObject(objectType, p_name), engineData(p_engineData), process(p_process)
		{
			flags = 0;
		}

		Set::~Set()
		{
			//	delete all ports
			for (UINT32 i=0; i<ports.size(); i++)
				delete ports[i];
		}

		UINT32 Set::getPortCount() const
		{
			return ports.size();
		}

		Port* Set::getPortByName(const char* name)
		{
			//	note this returns only the FIRST port in this set with this name (port names need not be unique on a set)
			for (UINT32 i=0; i<ports.size(); i++)
				if (ports[i]->getObjectName() == name) return ports[i];
			return NULL;
		}

		Port* Set::getPortByIndex(UINT32 index)
		{
			if (index >= ports.size())
				return NULL;
			return ports[index];
		}

		void Set::getSetInfo(EventGetSetInfo* info)
		{
			info->name = getObjectName().c_str();
			info->flags = flags;
			info->portCount = ports.size();
		}



	////////////////	INPUT SET

		InputSet::InputSet(string name, EngineData& engineData, Process* process)
			: Set(name, engineData, CT_INPUT_SET, process)
		{
		}

		InputPort* InputSet::addPort(Link* link)
		{
			/*

				The name of the port has been validated during
				parsing of the SystemML file, so we don't validate
				on this.

				An input Set can have multiple ports with the same
				name, so we don't validate on this.

			*/

			//	validate name
			string name = link->dst.portName;

			//	create new port
			ports.push_back(new InputPortLocal(name, engineData, link, this));

			//	add to parent interface
			process->iif.ports.push_back((InputPortLocal*)ports.back());

			//	return newly-added port
			return (InputPort*) ports.back();
		}



	////////////////	OUTPUT SET

		OutputSet::OutputSet(string name, EngineData& engineData, Process* process)
			: Set(name, engineData, CT_OUTPUT_SET, process)
		{
		}

		OutputPort* OutputSet::addPort(const char* className, UINT16 release)
		{
			//	validate
			if (!engineData.flag_allowCreatePort)
				ferr << E_SYSTEMML << "cannot create ports on this interface during this event";

			//	create the data object (set sample rate equal to that of the
			//	parent process - user code may change it immediately after
			//	through setSampleRate())
			Data* data = new Data("", engineData, NULL, className, release, process->getSampleRate());

			//	load component module
			data->load(*process->tout);

			//	instantiate the data component
			data->instantiate(process->tout);

			//	create new port
			ports.push_back(new OutputPort("", engineData, data, this));

			//	set port due data to zeroth buffer so that it's available for sml_getPortData() before run phase begins
			ports.back()->setDueData(data);

			//	add to parent interface
			process->oif.ports.push_back((OutputPort*)ports.back());

			//	ok
			return (OutputPort*) ports.back();
		}

		OutputPort* OutputSet::addPortByCopy(Data* rhs)
		{
			//	validate
			if (!engineData.flag_allowCreatePort)
				ferr << E_SYSTEMML << "cannot create ports on this interface during this event";

			//	create the data object as a copy of that passed
			Data* data = rhs->duplicate(process->tout);

			//	create new port
			ports.push_back(new OutputPort("", engineData, data, this));

			//	set port due data to zeroth buffer so that it's available for sml_getPortData() before run phase begins
			ports.back()->setDueData(data);

			//	add to parent interface
			process->oif.ports.push_back((OutputPort*)ports.back());

			//	ok
			return (OutputPort*) ports.back();
		}

		void OutputSet::getAllSampleRates(vector<SampleRate>& sampleRates)
		{
			for (UINT32 i=0; i<ports.size(); i++)
				sampleRates.push_back(ports[i]->getZerothData()->getSampleRate());
		}

		void OutputSet::finalizeAllComponentTimes(SampleRate baseSampleRate, BaseSamples executionStop)
		{
			for (UINT32 i=0; i<ports.size(); i++)
				((OutputPort*)ports[i])->finalizeAllComponentTimes(baseSampleRate, executionStop);
		}


		void OutputSet::listAvailableOutputs(brahms::output::Source& fout)
		{
			for (UINT32 i=0; i<ports.size(); i++)
				fout << ((OutputPort*)ports[i])->getDueData()->getObjectName() << brahms::output::D_INFO_UNHIDEABLE;
		}



	}

}

