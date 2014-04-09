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

	$Id:: component.h 2406 2009-11-19 20:53:19Z benjmitch      $
	$Rev:: 2406                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-19 20:53:19 +0000 (Thu, 19 Nov 2009)       $
________________________________________________________________

*/





namespace brahms
{
	namespace thread
	{
		class WorkerThread;
	}

	namespace systemml
	{



	////////////////	COMPONENT

		class Component : public RegisteredObject
		{

			friend class System;
			friend class brahms::thread::WorkerThread;
			friend class EventEx;
			friend class brahms::systemml::OutputPort;
			friend class brahms::systemml::InputInterface;
			friend class brahms::systemml::OutputInterface;

		public:

			//	tors
			Component(
				string name,
				EngineData& engineData,
				brahms::output::Source* tout,
				string desiredClassName,
				UINT16 desiredRelease,
				ComponentType type
				);

			Component(const Component& copy);
			virtual ~Component();

			//	state interface
			void setName(string name);
			void setSampleRate(SampleRate sampleRate);
			string getName();
			string getClassName();
			UINT16 getRelease();
			SampleRate getSampleRate();
			void* getObject();

			//	load module and make module->moduleInfo available
			void load(brahms::output::Source& tout);

			//	load module if not already loaded, and call EVENT_MODULE_CREATE to create component
			ObjectType instantiate(brahms::output::Source* tout);

			//	EVENT_MODULE_DESTROY
			virtual void destroy(brahms::output::Source* tout);

			//	accessor
			const ComponentInfo* getComponentInfo()
			{
				return componentInfo;
			}

			//	accessor
			const ComponentData* getComponentData()
			{
				return &componentData;
			}

			void setFlag(UINT32 flag);

			void finalizeComponentTime(SampleRate baseSampleRate, BaseSamples executionStop);

			ComponentCreateS componentCreateS;


		public:

			////////////////////////////////////////////////////////////////////////
			//	SPECIFIED AS ARGUMENTS TO CONSTRUCTOR
			EngineData& engineData;
			string desiredClassName;					//	*DESIRED* class name (once instantiated, this is available through "module->moduleInfo")
			UINT16 desiredRelease;						//	*DESIRED* release (once instantiated, this is available through "module->moduleInfo")

			////////////////////////////////////////////////////////////////////////
			//	ZEROED IN CONSTRUCTOR
			ComponentTime componentTime;				//	initialised by framework in phase POST_CONNECT
			const ComponentInfo* componentInfo;			//	initialised in instantiate()
			void* object;								//	initialised in instantiate() or duplicate()
			brahms::module::Module* module;				//	initialised in instantiate()
			UINT32 flags;								//	zero, or F_LOCAL_ERROR, if component has thrown
			brahms::output::Source* tout;				//	parent thread's output source

			////////////////////////////////////////////////////////////////////////
			//	INITIALISED IN CONSTRUCTOR
			ComponentData componentData;				//	pointers to componentTime and componentInfo

		};





	}
}

