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

	$Id:: system.h 2299 2009-11-03 22:44:59Z benjmitch         $
	$Rev:: 2299                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-03 22:44:59 +0000 (Tue, 03 Nov 2009)       $
________________________________________________________________

*/

#ifndef _SYSTEMML_SYSTEM_H_
#define _SYSTEMML_SYSTEM_H_ 1

#include <vector>
using std::vector;
#include <string>
using std::string;
#include "main/enginedata.h"
using brahms::EngineData;
#include "systemml/thread.h"
#include "systemml/identifier.h"
using brahms::systemml::Identifier;
#include "systemml/link.h"
using brahms::systemml::Link;
#include "systemml/systemml.h"
using brahms::systemml::Expose;
#include "support/execution.h"
#include "base/output.h"


namespace brahms
{
	namespace systemml
	{

	////////////////	SYSTEM

		struct PassResult
		{
			bool progress;
			bool finished;
		};

		struct PeerVoice
		{
			vector<string> outputs;
		};

		class System
		{

		public:

			System(EngineData& engineData, brahms::comms::Comms& comms);
			~System();

			//	interface
			void load(string path, brahms::Execution& execution, brahms::Loader& loader);

			//	accessors
			UINT32 getProcessCount();

			Process* getProcessByIndex(UINT32 index);

			//	system time
			MonitorTime systemTime;

			void listMissingInputs();

			//	local pass
			PassResult localPass(brahms::thread::Workers& workers);

			//	remote pass
			void connectPhaseServe(UINT32 remoteVoiceIndex, bool &globalFinished, bool &globalProgress);

			//	find an output that has the name expected by the passed input port
			OutputPort* findOutputLocally(Identifier& outputName);
			OutputPort* findOutput(Identifier& outputName);
			void announceOutput(string name);

			//	operations
			void terminate(brahms::output::Source& tout);

			//	get all sample rates (process and data)
			void getAllSampleRates(vector<SampleRate>& sampleRates);

			void finalizeAllComponentTimes(SampleRate baseSampleRate, BaseSamples executionStop);
			void progress(brahms::output::Source& fout);
			void setDueDatas(brahms::output::Source& fout);
			void initInterThreadLocks(brahms::output::Source& fout);

			void startLogs(brahms::thread::Workers& workers);

			//	get all output ports
			vector<OutputPort*> getAllOutputPorts();



		public:

			//	helpers
			void parse(brahms::xml::XMLNode* systemML, string pathToSubSystem);
			VSTRING resolveExposes(VSTRING identifiers);

			//	system data
			vector<Process*> processes;
			vector<Link*> links;
			vector<Expose*> exposes;

			//	list of processes on other nodes (only for validation)
			vector<string> processesOnOtherNodes;

			//	System File (is maintained so it can be updated and re-serialized)
			brahms::xml::XMLNode nodeSystem;

			//	reference to engine data
			EngineData& engineData;
			brahms::comms::Comms& comms;

			//	peer voice data
			vector<PeerVoice> peerVoices;


		////	I/Ps and O/Ps

			/*

				Local inputs and outputs are owned by a Set, by an Interface, by a
				Process, by this System object. Remote inputs and outputs are objects
				created in abstraction, that look identical to the partner port they
				are connected to, but behave internally so as to propagate data between
				peer voices. They do not have an associated Set or Process (the Process
				with which they are "associated" is on a remote voice).

			*/

			vector<InputPortRemote*> inputPortRemotes;
			vector<OutputPort*> outputPortRemotes;

			//	end init phase completes initialisation of data objects
			void endInitPhase(brahms::output::Source& fout);



		};



	////////////////	PUSHDATA HANDLER directs PUSHDATA messages into a "remote" OutputPort

		Symbol pushdataHandler(void* arg, BYTE* stream, UINT32 count);


	}
}

#endif // _SYSTEMML_SYSTEM_H_
