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

	$Id:: process.cpp 2316 2009-11-07 20:38:30Z benjmitch      $
	$Rev:: 2316                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-07 20:38:30 +0000 (Sat, 07 Nov 2009)       $
________________________________________________________________

*/



#include "systemml.h"


using namespace brahms::math;


namespace brahms
{
	namespace systemml
	{




		Process::Process(
			string name,
			EngineData& engineData,
			brahms::output::Source* tout,
			string className,
			SampleRate sampleRate,
			brahms::xml::XMLNode* p_nodeProcess,
			const VUINT32& p_seed,
			UINT32 p_indexInSystem
		)
		:
			Component(name, engineData, tout, className, 0, CT_PROCESS),
			Loggable(this),
			iif(engineData, this),
			oif(engineData, this)
		{
			seed.set(p_seed);

			indexInSystem = p_indexInSystem;
			thread = NULL;

			//	force EVENT_INIT_CONNECT to be fired at least once, with F_FIRST_CALL
			EVENT_INIT_CONNECT_firstCall = true;

			setSampleRate(sampleRate);
			state = PRS_STOP;


			//	this is used as a cache by thread-service
			eventHandler = NULL;

			//	prepare XML data
			nodeProcess = p_nodeProcess;
		}

		Process::~Process()
		{
			for (UINT32 u=0; u<utilities.size(); u++)
				delete utilities[u];
		}

		void Process::destroy(brahms::output::Source* tout)
		{
			//	this override calls destroy() on child utilities, first
			for (UINT32 u=0; u<utilities.size(); u++)
				utilities[u]->destroy(tout);

			//	then on self
			Component::destroy(tout);
		}

		void Process::addZeroLagSource(UINT32 index)
		{
			zeroLagSources.push_back(index);
		}

		const VUINT32& Process::getZeroLagSources()
		{
			return zeroLagSources;
		}

		void Process::addLink(Link* link)
		{
			links.push_back(link);
		}

		Set* Process::addSet(UINT32 flags, const char* name)
		{
			if (flags & F_IIF) return iif.addSet(name);
			if (flags & F_OIF) return oif.addSet(name);
			ferr << E_INVALID_ARG << "flags must include F_IIF or F_OIF";

			//	avoid debug-mode warning...
			return 0;
		}

		UINT32 Process::getSetCount(UINT32 flags)
		{
			if (flags & F_IIF) return iif.getSetCount();
			if (flags & F_OIF) return oif.getSetCount();
			ferr << E_INVALID_ARG << "flags must include F_IIF or F_OIF";

			//	avoid debug-mode warning...
			return 0;
		}

		Set* Process::getSetByIndex(UINT32 flags, UINT32 index)
		{
			if (flags & F_IIF) return iif.getSetByIndex(index);
			if (flags & F_OIF) return oif.getSetByIndex(index);
			ferr << E_INVALID_ARG << "flags must include F_IIF or F_OIF";

			//	avoid debug-mode warning...
			return 0;
		}

		Set* Process::getSetByName(UINT32 flags, const char* name)
		{
			if (flags & F_IIF) return iif.getSetByName(name);
			if (flags & F_OIF) return oif.getSetByName(name);
			ferr << E_INVALID_ARG << "flags must include F_IIF or F_OIF";

			//	avoid debug-mode warning...
			return 0;
		}



		void Process::structureInputInterface()
		{
			/*

				The Links from the System File are stored in Process::links (a flat array)
				until the Process is instantiated (after we've concluded that it's going to
				be instantiated on our voice).

				Here, we resolve the flat list of links associated with the Process into
				Ports, one port for each link. The link objects are stored in the ports
				from this point on, so we clear them from the flat list in the Process.

			*/

			//	for each link
			for(UINT32 l=0; l<links.size(); l++)
			{
				//	get link
				Link* link = links[l];

				//	find target set
				InputSet* set = iif.getSetByName(link->dst.setName.c_str());
				if (!set) ferr << E_SYSTEM << "set called \"" << prettySet(link->dst.setName) << "\" not found on iif of \"" << getObjectName() << "\"";

				/*
				NO - INPUT SETS CAN HAVE MULTIPLE PORTS WITH THE SAME NAME

				//	find target port
				Port* port = set->getPortByName(link->dst.portName.c_str());
				if (port) ferr << E_SYSTEM << "port called \"" << link->dst.portName << "\" already present on set \"" << prettySet(link->dst.setName) << "\" on iif of \"" << getObjectName() << "\"";
				*/

				//	create port
				set->addPort(link);
			}

			//	clear links
			links.clear();
		}

		vector<InputPortLocal*> Process::getUnseenInputs()
		{
			vector<InputPortLocal*> result;

			//	for each set
			for (UINT32 s=0; s<iif.sets.size(); s++)
			{
				//	for each port
				for (UINT32 p=0; p<iif.sets[s]->ports.size(); p++)
				{
					//	if port has been connected, ignore it
					if (((InputPortLocal*)iif.sets[s]->ports[p])->connectedOutputPort)
						continue;

					//	else, add to result
					result.push_back((InputPortLocal*)iif.sets[s]->ports[p]);
				}
			}

			return result;
		}




		OutputPort* Process::findOutput(Identifier& outputName)
		{
			return oif.findOutput(outputName);
		}



		Symbol Process::__getRandomSeed(struct EventGetRandomSeed* data)
		{
			//	switch on which calling protocol
			if (data->count)
			{
				if (!data->seed) ferr << E_NULL_ARG << "seed";
				VUINT32 value = seed.get(data->count);
				for (UINT32 s=0; s<data->count; s++)
					data->seed[s] = value[s];
				return C_OK;
			}
			else
			{
				data->count = seed.available();
				return C_OK;
			}
		}



		void Process::getAllSampleRates(vector<SampleRate>& sampleRates)
		{
			//	process
			sampleRates.push_back(getSampleRate());

			//	data
			oif.getAllSampleRates(sampleRates);
		}

		void Process::setDueDatas(brahms::output::Source& fout)
		{
			/*

				Set them all to NULL, initially. They'll be set to non-NULL
				immediately before a call to EVENT_RUN_SERVICE, during i/p
				and o/p lock calls, if they are supposed to be non-NULL.

			*/

			vector<OutputPort*> outputPorts = getAllOutputPorts();
			for (UINT32 p=0; p<outputPorts.size(); p++)
			{
				//	extract
				OutputPort* outputPort = outputPorts[p];
				outputPort->setDueData(NULL);
			}
	
			vector<InputPortLocal*> inputPorts = getAllInputPorts();
			for (UINT32 p=0; p<inputPorts.size(); p++)
			{
				//	extract
				InputPortLocal* inputPort = inputPorts[p];
				inputPort->setDueData(NULL);
			}
		}

		void Process::progress(brahms::output::Source& fout)
		{

	
	/*

		NOTE TODO: when we make this work on Concerto, we'll have to
		set the indices in all the remote ports as well as we're
		setting them in the local ports, here. easiest to set up a
		test system and work out what to do empirically, i think.
		we'll also have to pay attention to the locks associated
		with these remote readers and writers.

	*/


			//	set time of process ("now", basically) according to progress already made
			brahms::xml::XMLNode* nodeTime = nodeProcess->getChild("Time");
			if (nodeTime->hasChild("BaseSampleRate"))
			{
				string s_baseSampleRate = nodeTime->getChild("BaseSampleRate")->nodeText();
				SampleRate baseSampleRate = parseSampleRate(s_baseSampleRate);

				//	error msg
				const char* advanceTime_err = "changing timing data of processes during execution pauses is not currently supported (but can be, ask for it to be implemented)";

				//	in principle, different parts of the system (different processes) may
				//	previously have been computed with different base sample rates (as part
				//	of separate systems/executions). and, in principle, we can handle this
				//	just fine, by finding a new base sample rate and computing all processes
				//	at that. we've already done this (found the new BSR), but i'm too lazy
				//	to code what we do here. basically, we have to recompute where this process
				//	is in time, using the "now" against its old BSR, and computing the now
				//	against its new BSR.
				if (baseSampleRate.num != componentTime.baseSampleRate.num || baseSampleRate.den != componentTime.baseSampleRate.den)
					ferr << E_NOT_IMPLEMENTED << advanceTime_err;

				//	so, the sample period must also agree with the computation from an earlier execution
				const char* c_sp = nodeTime->getChild("SamplePeriod")->nodeText();
				DOUBLE f = brahms::text::s2n(c_sp);
				BaseSamples bs_sp = f;
				string s = brahms::text::n2s(bs_sp);
				if (f == brahms::text::S2N_FAILED || (s != c_sp))
					ferr << E_SYSTEM_FILE << "failed to interpret \"SamplePeriod\"";
				if (bs_sp != componentTime.samplePeriod)
					ferr << E_NOT_IMPLEMENTED << advanceTime_err;

				//	ok, good - let's advance the timer if necessary
				const char* c_now = nodeTime->getChild("Now")->nodeText();
				f = brahms::text::s2n(c_now);
				BaseSamples bs_now = f;
				s = brahms::text::n2s(bs_now);
				if (f == brahms::text::S2N_FAILED || (s != c_now))
					ferr << E_SYSTEM_FILE << "failed to interpret \"Now\"";

				//	set it
				componentTime.now = bs_now;

				/*
					we now to set a few other things on each of the process's output ports.

						* the writeBuffer (index) must be set correctly.
						* N readBuffer (indexes) must be set correctly on attached input ports.
						* the read/write locks must be set appropriately, so that buffers that the
							writer will reach first are write-ready, whereas buffers that a reader
							will reach first are read-ready.
						* the output port, and each of the input ports, have to have their dueData
							member set correctly, either to point at a buffer, or at NULL if they
							are not currently due.
				*/

				//	for each output port
				vector<OutputPort*> outputPorts = getAllOutputPorts();
				for (UINT32 p=0; p<outputPorts.size(); p++)
				{
					//	extract
					OutputPort* outputPort = outputPorts[p];
					UINT32 numberOfBuffersInRing = outputPort->ring.size();

					//	get timing data
					ComponentTime portTime = outputPort->getDueData()->componentTime;

					//	calculate next port write time, which can be used to tell
					//	us which buffer will be written at the next write
					BaseSamples nextPortServiceTime = bs_now - (bs_now % portTime.samplePeriod); // earliest port service time that is now...
					if (nextPortServiceTime < bs_now) nextPortServiceTime += portTime.samplePeriod; //  ...or later than now, i.e. next write time

					//	now, how many writes *in total* have occurred since t=0
					UINT32 numberOfServices = nextPortServiceTime / portTime.samplePeriod;

					//	calculate which is the write buffer, based on this
					UINT32 equivalentNumberOfServices = numberOfServices % numberOfBuffersInRing; // this number of writes would have put us on the same write buffer

					//	then, write buffer is...
					INT32 writeBuffer = -equivalentNumberOfServices;
					if (writeBuffer < 0) writeBuffer += numberOfBuffersInRing;

					//	and we can lay that in
					outputPort->ring.writeBuffer = writeBuffer;
				}

				//	now, for each of our input ports, we can calculate which read
				//	buffer we should be on, and lay that in
				vector<InputPortLocal*> inputPorts = getAllInputPorts();
				for (UINT32 p=0; p<inputPorts.size(); p++)
				{
					//	extract
					InputPortLocal* inputPort = inputPorts[p];

					//	get attached output port
					OutputPort* outputPort = inputPort->connectedOutputPort;
					UINT32 numberOfBuffersInRing = outputPort->ring.size();
				
					//	get timing data
					ComponentTime portTime = outputPort->getDueData()->componentTime;

					//	calculate next port read time, which can be used to tell
					//	us which buffer will be read at the next read
					BaseSamples nextPortServiceTime = bs_now - (bs_now % portTime.samplePeriod); // earliest port service time that is now...
					if (nextPortServiceTime < bs_now) nextPortServiceTime += portTime.samplePeriod; //  ...or later than now, i.e. next read time

					//	now, how many reads *in total* have occurred since t=0
					UINT32 numberOfServices = nextPortServiceTime / portTime.samplePeriod;

					//	calculate which is the read buffer, based on this
					UINT32 equivalentNumberOfServices = numberOfServices % numberOfBuffersInRing; // this number of reads would have put us on the same read buffer

					//	then, read buffer is the lag (t=0 buffer) *minus* the number of moves we've made since then
					INT32 readBuffer = inputPort->lag - equivalentNumberOfServices;
					if (readBuffer < 0) readBuffer += numberOfBuffersInRing;

					//	and we can lay that in
					inputPort->readBuffer = readBuffer;
				}
			}

			//	set state of any data objects
			//	WE MUST DO THIS *AFTER* the above, so that "writeBuffer" is set correctly,
			//	because the objects in the SystemML file are referenced by "Lag" relative
			//	to the "current" write buffer (current at time of writing the file)
			if (nodeProcess->hasChild("Output"))
			{
				brahms::xml::XMLNode* nodeOutput = nodeProcess->getChild("Output");
				const brahms::xml::XMLNodeList* nodeDatas = nodeOutput->childNodes();
				for (UINT32 n=0; n<nodeDatas->size(); n++)
				{
					//	extract
					brahms::xml::XMLNode* nodeData = nodeDatas->at(n);
					string name = nodeData->getChild("Name")->nodeText();
					string slag = nodeData->getChild("Lag")->nodeText();
					UINT32 lag = brahms::text::s2n(slag);
					string slagb = brahms::text::n2s(lag);
					if (slag != slagb)
						ferr << E_SYSTEM_FILE << "failed to interpret \"Lag\" in Data tag";
					brahms::xml::XMLNode* nodeState = nodeData->getChild("State");

					//	find data object - we can use a dummy process name since
					//	we know we're in the right process already, and this allows
					//	us to use the standard "Identifier" parser
					Identifier id;
					id.parse(("dummy>>" + name).c_str(), ST_NULL, "identifier of Data state");
					OutputPort* port = findOutput(id);

					//	get data object
					Data* data = port->ring.getWriteBuffer(lag);

					//	call data object to set its state
					EventStateSet ess;
					____CLEAR(ess);
					ess.state = nodeState->getObjectHandle();
					brahms::EventEx event(EVENT_STATE_SET, 0, data, &ess, true, &fout);
					event.fire();
				}
			}
		}

		void Process::initInterThreadLocks(brahms::output::Source& fout)
		{
			//	each lock within the ring buffer of each output is set
			//	so that it is either read-ready or write-ready, depending
			//	on whether the reader or the writer is scheduled to reach
			//	it first. that is, *none* of the locks are locked, it's
			//	just to decide what they are ready-to-be-locked-for!

			//	we set all the output port buffers for the process, and
			//	since all output ports are associated with a process, this
			//	means we'll get 'em all.

			//	for each output port
			vector<OutputPort*> outputPorts = getAllOutputPorts();
			for (UINT32 p=0; p<outputPorts.size(); p++)
			{
				//	extract
				OutputPort* outputPort = outputPorts[p];
				outputPort->initInterThreadLocks(fout);
			}
		}

		void Process::finalizeAllComponentTimes(SampleRate baseSampleRate, BaseSamples executionStop)
		{
			//	finalize our own
			finalizeComponentTime(baseSampleRate, executionStop);

			//	and all data
			oif.finalizeAllComponentTimes(baseSampleRate, executionStop);
		}

		vector<InputPortLocal*> Process::getAllInputPorts()
		{
			return iif.ports;
		}

		vector<OutputPort*> Process::getAllOutputPorts()
		{
			return oif.ports;
		}

		Symbol Process::readAndWriteLock(BaseSamples now, brahms::output::Source* tout)
		{
			if (iif.readLock(now, tout) == C_CANCEL) return C_CANCEL;
			if (oif.writeLock(now) == C_CANCEL) return C_CANCEL;
			return C_OK;
		}

		BaseSamples Process::readRelease(BaseSamples now, BaseSamples nextService)
		{
			return iif.readRelease(now, nextService);
		}

		void Process::writeRelease(BaseSamples now, BaseSamples nextService, brahms::output::Source* tout)
		{
			nextService = oif.writeRelease(now, nextService, tout);
			componentTime.now = nextService;
		}

		Symbol Process::createUtility(EventCreateUtility* data)
		{
			//	get class name
			if (!data->spec.cls) ferr << E_INVALID_ARG << "EventCreateUtility::cls is NULL";
			string className = data->spec.cls;
			if (!className.length()) ferr << E_INVALID_ARG << "EventCreateUtility::cls is empty";

			//	utility name is <process name>:<utility name> or <process name>:<class name>
			string utilityName;
			if (data->name) utilityName = data->name;
			if (!utilityName.length()) utilityName = className;

			//	create
			utilities.push_back(new Utility(utilityName, engineData, tout, className, this));

			//	load
			utilities.back()->load(*tout);

			//	instantiate
			utilities.back()->instantiate(tout);

			//	ok
			return utilities.back()->getObjectHandle();
		}




	}
}

