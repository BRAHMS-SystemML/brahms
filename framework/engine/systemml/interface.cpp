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

	$Id:: interface.cpp 2286 2009-11-02 02:29:40Z benjmitch    $
	$Rev:: 2286                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-02 02:29:40 +0000 (Mon, 02 Nov 2009)       $
________________________________________________________________

*/


#include "systemml.h"

using namespace brahms::math;


namespace brahms
{
	namespace systemml
	{




	////////////////	INTERFACE

		Interface::Interface(EngineData& p_engineData, Process* p_process)
			: engineData(p_engineData), process(p_process)
		{
		}

		Interface::~Interface()
		{
			//	delete all sets
			for (UINT32 i=0; i<sets.size(); i++)
				delete sets[i];
		}

		Set* Interface::addSet(const char* name)
		{
			if (getSetByName(name))
				ferr << E_INVALID_ARG << "set named \"" << prettySet(name) << "\" already exists on this interface";

			if (!validSetName(name))
				ferr << E_INVALID_ARG << "illegal set name \"" << prettySet(name) << "\"";

			//	create new set
			sets.push_back(create(name));

			//	return
			return sets.back();
		}

		UINT32 Interface::getSetCount()
		{
			return sets.size();
		}

		Set* Interface::getSetByIndex(UINT32 index)
		{
			if (index < sets.size())
				return sets[index];
			return NULL;
		}

		Set* Interface::getSetByName(const char* name)
		{
			if (!name) name = "";
			for (UINT32 i=0; i<sets.size(); i++)
				if (sets[i]->getObjectName() == name)
					return sets[i];
			return NULL;
		}

		UINT32 Interface::getPortCount() const
		{
			UINT32 count = 0;
			for (UINT32 i=0; i<sets.size(); i++)
				count += sets[i]->ports.size();
			return count;
		}

		UINT32 Interface::listMissingInputs(const char* processName, brahms::output::Source& fout)
		{
			//	first one we find, we'll output the process name
			bool first = true;
			UINT32 ret = 0;

			//	for each set
			for (UINT32 i=0; i<sets.size(); i++)
			{
				Set* set = sets[i];

				//	for each input
				for (UINT32 p=0; p<set->ports.size(); p++)
				{
					InputPortLocal* port = (InputPortLocal*) set->ports[p];
					if (!port->connectedOutputPort)
					{
						if (first) fout << "[required by process \"" << processName << "\"]" << brahms::output::D_INFO_UNHIDEABLE;
						fout << string(port->link->src) << brahms::output::D_INFO_UNHIDEABLE;
						first = false;
						ret++;
					}
				}
			}

			//	ok
			return ret;
		}



	////////////////	INPUT INTERFACE

		InputInterface::InputInterface(EngineData& engineData, Process* process)
			: Interface(engineData, process)
		{
			//	create default set
			sets.push_back(create(""));
		}

		Set* InputInterface::create(string name)
		{
			return new InputSet(name, engineData, process);
		}

		InputSet* InputInterface::getSetByName(const char* name)
		{
			if (!name) name = "";
			for (UINT32 i=0; i<sets.size(); i++)
				if (sets[i]->getObjectName() == name) return (InputSet*) sets[i];
			return NULL;
		}

		Symbol InputInterface::readLock(BaseSamples now, brahms::output::Source* tout)
		{
			for (UINT32 p=0; p<ports.size(); p++)
			{
				InputPortLocal* port = ports[p];
				if (port->readLock(now) == C_CANCEL) return C_CANCEL;
			}

			return C_OK;
		}

		BaseSamples InputInterface::readRelease(BaseSamples now, BaseSamples nextService)
		{
			//	assume next service time is at the end of the execution (never reached);
			//	true next service time of this interface is nearest service time of
			//	all ports on it

			for (UINT32 p=0; p<ports.size(); p++)
			{
				InputPortLocal* port = ports[p];
				nextService = port->readRelease(now, nextService);
			}

			return nextService;
		}





	////////////////	OUTPUT INTERFACE

		OutputInterface::OutputInterface(EngineData& engineData, Process* process)
			: Interface(engineData, process)
		{
			//	create default set
			sets.push_back(create(""));
		}

		Set* OutputInterface::create(string name)
		{
			return new OutputSet(name, engineData, process);
		}

		OutputSet* OutputInterface::getSetByName(const char* name)
		{
			if (!name) name = "";
			for (UINT32 i=0; i<sets.size(); i++)
				if (sets[i]->getObjectName() == name) return (OutputSet*) sets[i];
			return NULL;
		}

		//	find an output that has the name expected by the passed input port
		OutputPort* OutputInterface::findOutput(Identifier& outputName)
		{
			OutputSet* set = getSetByName(outputName.setName.c_str());
			if (!set) return NULL;
			return (OutputPort*) set->getPortByName(outputName.portName.c_str());
		}


		void OutputInterface::getAllSampleRates(vector<SampleRate>& sampleRates)
		{
			for (UINT32 i=0; i<sets.size(); i++)
				((OutputSet*)sets[i])->getAllSampleRates(sampleRates);
		}

		void OutputInterface::finalizeAllComponentTimes(SampleRate baseSampleRate, BaseSamples executionStop)
		{
			for (UINT32 i=0; i<sets.size(); i++)
				((OutputSet*)sets[i])->finalizeAllComponentTimes(baseSampleRate, executionStop);
		}

		void OutputInterface::listAvailableOutputs(brahms::output::Source& fout)
		{
			for (UINT32 i=0; i<sets.size(); i++)
				((OutputSet*)sets[i])->listAvailableOutputs(fout);
		}

		Symbol OutputInterface::writeLock(BaseSamples now)
		{
			for (UINT32 p=0; p<ports.size(); p++)
			{
				OutputPort* port = ports[p];
				if (port->writeLock(now) == C_CANCEL) return C_CANCEL;
			}

			return C_OK;
		}

		BaseSamples OutputInterface::writeRelease(BaseSamples now, BaseSamples nextService, brahms::output::Source* tout)
		{
			//	assume next service time is at the end of the execution (never reached);
			//	true next service time of this interface is nearest service time of
			//	all ports on it

			for (UINT32 p=0; p<ports.size(); p++)
			{
				OutputPort* port = ports[p];
				nextService = port->writeRelease(now, tout, nextService);

	/*
				ALTERNATIVELY, we can use the "due" information to help us, but i don't think it's any easier

				if (due)
				{
					stop = min(stop, now + );
				}
				else
				{
					stop = min(stop, NEXTBOUNDARY(port->getZerothData()->componentTime.now, port->getZerothData()->componentTime.samplePeriod);
				}
				*/
			}

			return nextService;
		}




	}
}


