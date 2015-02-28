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

	$Id:: process.h 2316 2009-11-07 20:38:30Z benjmitch        $
	$Rev:: 2316                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-07 20:38:30 +0000 (Sat, 07 Nov 2009)       $
________________________________________________________________

*/

#ifndef _ENGINE_SYSTEMML_PROCESS_H_
#define _ENGINE_SYSTEMML_PROCESS_H_

namespace brahms
{
	namespace thread
	{
		class WorkerThread;
	}

	namespace systemml
	{



	////////////////	PROCESS

		enum ProcessRunState
		{
			PRS_STOP = 0,
			PRS_PAUSE,
			PRS_PLAY
		};

		class Process : public Component, public Loggable
		{

			friend class System;
			friend class brahms::systemml::InputSet;
			friend class brahms::systemml::OutputSet;
			friend class brahms::thread::WorkerThread;

		public:

			Process(
				string name,
				EngineData& engineData,
				brahms::output::Source* tout,
				string className,
				SampleRate sampleRate,
				brahms::xml::XMLNode* nodeProcess,
				const VUINT32& seed,
				UINT32 indexInSystem
			);

			~Process();

			void addZeroLagSource(UINT32 index);
			const VUINT32& getZeroLagSources();

			void addLink(Link* link);

			vector<InputPortLocal*> getUnseenInputs();


			//	find an output that has the name expected by the passed input port
			OutputPort* findOutput(Identifier& outputName);

			//	xml data
			brahms::xml::XMLNode* nodeProcess;

			//	structure IIF
			void structureInputInterface();

			void destroy(brahms::output::Source* tout);


			//	Component Interface
			Symbol __getRandomSeed(struct EventGetRandomSeed* data);
			//	Component Interface

			//	interface
			UINT32 getSetCount(UINT32 flags);
			Set* getSetByIndex(UINT32 flags, UINT32 index);
			Set* addSet(UINT32 flags, const char* name);
			Set* getSetByName(UINT32 flags, const char* name);


			void getAllSampleRates(vector<SampleRate>& sampleRates);

			void finalizeAllComponentTimes(SampleRate baseSampleRate, BaseSamples executionStop);

			void progress(brahms::output::Source& fout);
			void setDueDatas(brahms::output::Source& fout);
			void initInterThreadLocks(brahms::output::Source& fout);

			//	Run Phase read/write lock and release
			Symbol readAndWriteLock(BaseSamples now, brahms::output::Source* tout);
			BaseSamples readRelease(BaseSamples now, BaseSamples nextService);
			void writeRelease(BaseSamples now, BaseSamples nextService, brahms::output::Source* tout);



			vector<OutputPort*> getAllOutputPorts();
			vector<InputPortLocal*> getAllInputPorts();




			Symbol createUtility(EventCreateUtility* data);


			//	wallclock time
			brahms::time::TIME_IRT irtWallclock;



			EventHandlerFunction* eventHandler;

			//	handle to thread in which this process runs
			brahms::thread::WorkerThread* thread;

			UINT32 indexInSystem; // used for inferred seeds

		private:

			brahms::local::Seed seed;
			VUINT32 zeroLagSources;
			vector<Link*> links;
			bool EVENT_INIT_CONNECT_firstCall;
			brahms::EventEx serviceEvent;

			vector<Utility*> utilities;

			//	interfaces
			InputInterface iif;
			OutputInterface oif;


			//	housekeeping
			ProcessRunState state;

		};





	}
}

#endif // _ENGINE_SYSTEMML_PROCESS_H_
