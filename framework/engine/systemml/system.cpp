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

	$Id:: system.cpp 2451 2010-01-25 16:48:58Z benjmitch       $
	$Rev:: 2451                                                $
	$Author:: benjmitch                                        $
	$Date:: 2010-01-25 16:48:58 +0000 (Mon, 25 Jan 2010)       $
________________________________________________________________

*/



#include "systemml.h"

using namespace brahms::output;
using namespace brahms::math;
using namespace brahms::comms;

namespace brahms
{
	namespace systemml
	{




	////////////////	SYSTEM

		System::System(EngineData& p_engineData, Comms& p_comms) : engineData(p_engineData), comms(p_comms)
		{
			____CLEAR(systemTime);
		}

		System::~System()
		{
		}

		void System::terminate(brahms::output::Source& tout)
		{
			/*
				NB: we must delete the system only after all threads that might operate
				on it have terminated. this means the W threads, of course, but also any
				S thread may fire a callback on a port having delivered a PUSHDATA msg.
				
				therefore, this function must not be called until all W have been stopped,
				and all S have been stopped (or, alternatively, they've been queue-flushed).
			*/

			try
			{
				//	delete remote objects
				for (UINT32 n=0; n<inputPortRemotes.size(); n++)
					delete inputPortRemotes[n];
				inputPortRemotes.clear();

				//	delete remote objects
				for (UINT32 n=0; n<outputPortRemotes.size(); n++)
					delete outputPortRemotes[n];
				outputPortRemotes.clear();

				//	delete processes
				for (UINT32 n=0; n<processes.size(); n++)
					delete processes[n];
				processes.clear();

				//	delete exposes
				for (UINT32 n=0; n<exposes.size(); n++)
					delete exposes[n];
				exposes.clear();

				//	delete links
				for (UINT32 n=0; n<links.size(); n++)
					delete links[n];
				links.clear();
			}

			catch(brahms::error::Error e)
			{
				e.trace("whilst calling terminate() on System");
				engineData.core.caller.storeError(e, tout);
			}
		}

		void System::load(string path, brahms::Execution& execution, brahms::Loader& loader)
		{

			brahms::output::Source& fout(engineData.core.caller.tout);

		////////////////	PARSE XML

			fout << "System File" << D_VERB;
			ifstream fileSystem(path.c_str());
			if (!fileSystem) ferr << E_OS << "error opening \"" << path << "\"";
			
			try
			{
				nodeSystem.parse(fileSystem);
			}
			CATCH_TRACE_RETHROW("parsing \"" + path + "\"")

			fileSystem.close();

			//	report
			string systemTitle;
			if (nodeSystem.hasChild("Title")) systemTitle = nodeSystem.getChild("Title")->nodeText();
			else systemTitle = "Untitled";
			fout << "loaded system \"" << systemTitle << "\"" << D_VERB;

		////////////////	PARSE SYSTEMML

			//	parse system
			parse(&nodeSystem, "");

		////////////////	RESOLVE LINKS/EXPOSES

			//	dump exposes
			fout << "Parsed Exposes" << D_VERB;
			for (UINT32 e=0; e<exposes.size(); e++)
				fout << exposes[e]->what << " <-- " << exposes[e]->as << D_FULL;

			//	resolve exposes
			fout << "Resolved Exposes" << D_VERB;
			for (UINT32 l=0; l<links.size(); l++)
			{
				//	extract
				Link* link = links[l];

				//	resolve src
				VSTRING src;
				src.push_back(link->getSrc());
				src = resolveExposes(src);

				//	resolve dst (may expand to multiple dst's)
				VSTRING dst;
				dst.push_back(link->getDst());
				dst = resolveExposes(dst);

				//	check
				if (src.size() != 1)
					ferr << E_INTERNAL << "processing link Src (size is " << src.size() << ")";

				//	create one link for each src/dst pair...
				//	one already exists, but may need to have its data overwritten
				for (UINT32 d=0; d<dst.size(); d++)
				{
					//	usually, reference the first
					Link* newLink = link;

					//	but create if necessary as copy of first
					if (d)
					{
						newLink = new Link(*link);
						links.push_back(newLink);
					}

					//	update data in link
					newLink->setSrc(src[0]);
					newLink->setDst(dst[d]);

					//	keep a tally of zero-lag links, because these will be needed to
					//	work out in what order the processes need to be executed
					if (!newLink->getLag())
					{
						//	find the src process
						UINT32 src;
						for (src=0; src<processes.size(); src++)
							if (newLink->getSrcProcessName() == processes[src]->getName()) break;
						if (src == processes.size())
							ferr << E_SYSTEM_FILE << "Link Src process \"" << newLink->getSrcProcessName() << "\" not found";

						//	find the dst process
						UINT32 dst;
						for (dst=0; dst<processes.size(); dst++)
							if (newLink->getDstProcessName() == processes[dst]->getName()) break;
						if (dst == processes.size())
							ferr << E_SYSTEM_FILE << "Link Dst process \"" << newLink->getDstProcessName() << "\" not found";

						//	store this
						processes[dst]->addZeroLagSource(src);
					}
				}
			}



		////////////////	ORDER PROCESSES SO THAT ALL ZERO-LAG LINKS ARE FROM i TO j WITH i < j
					////	(FAILURE TO DO THIS INDICATES AN ARITHMETIC LOOP)

			//	report
			fout << "Reorder Processes" << D_VERB;

			//	take copy of disordered processes
			vector<Process*> disordered = processes;
			processes.clear();

			//	until complete
			while(true)
			{
				//	find an acceptable process - that is, a process where all its zero-lag
				//	sources have already been "used" (source entries are NULL in "disordered")
				UINT32 p;
				for (p=0; p<disordered.size(); p++)
				{
					//	if already used, move on - can't use again
					if (!disordered[p]) continue;

					//	check if any of its zero-lag sources are unused, and if so move on
					bool moveon = false;
					Process* pp = ((Process*)disordered[p]);
					UINT32 zc = pp->getZeroLagSources().size();
					for (UINT32 z=0; z<zc; z++)
					{
						//	source index
						UINT32 src = pp->getZeroLagSources()[z];

						//	check
						if (disordered[src])
						{
							moveon = true;
							break;
						}
					}
					if (moveon) continue;

					//	ok, we can use this process now
					processes.push_back(disordered[p]);
					disordered[p] = NULL;
					break;
				}

				//	if one was found, go round again
				if (p < disordered.size()) continue;

				//	if we've finished, break while loop
				if (processes.size() == disordered.size()) break;

				//	if not, that indicates an arithmetic loop (no suitable process was found)
				string involved;
				for (UINT32 src=0; src<disordered.size(); src++)
				{
					if (!disordered[src]) continue;
					if (involved.length()) involved += ", ";
					involved += "\"" + disordered[src]->getName() + "\"";
				}
				ferr << E_CIRCULAR_REFERENCE << "one (or more) loops involving " << involved;
			}

		////////////////	SCHEDULE SYSTEM

			//	report
			fout << "Schedule System" << D_VERB;

			//	take copy of unscheduled processes
			vector<Process*> unscheduled = processes;
			processes.clear();

			//	for each process, find the affinity group it belongs
			//	to, if any, and add it to that group
			vector<INT32> processGroup;
			for (UINT32 w=0; w<unscheduled.size(); w++)
			{
				//	best match so far
				string bestmatch;
				INT32 bestgroup = -1;

				//	for each group
				for (UINT32 g=0; g<execution.groups.size(); g++)
				{
					brahms::Group grp = execution.groups[g];
					for (UINT32 i=0; i<grp.ident.size(); i++)
					{
						string ident = grp.ident[i];
						UINT32 L = ident.length();
						if (L <= bestmatch.length())
							continue;
						if (unscheduled[w]->getName().substr(0, L) == ident)
						{
							bestmatch = ident;
							bestgroup = g;
						}
					}
				}

				//	add it
				processGroup.push_back(bestgroup);
				//fout << "process " << unscheduled[w]->name << " on " << bestgroup << D_VERB;
			}

			//	now go through the groups and count the members of each
			for (UINT32 w=0; w<unscheduled.size(); w++)
			{
				if (processGroup[w] != -1)
					execution.groups[processGroup[w]].members++;
			}

			//	count how many processes are already assigned to voices (because some
			//	groups might have been scheduled already by explicit voice affinity)
			vector<UINT32> assignedProcessCount;
			UINT32 groupsScheduled = 0;
			assignedProcessCount.resize(engineData.core.getVoiceCount());
			for (UINT32 v=0; v<engineData.core.getVoiceCount(); v++)
				assignedProcessCount[v] = 0;
			for (UINT32 g=0; g<execution.groups.size(); g++)
			{
				if (execution.groups[g].voice != VOICE_UNDEFINED)
				{
					INT32 v = execution.groups[g].voice;
					assignedProcessCount[v] += execution.groups[g].members;
					groupsScheduled++;
				}
			}

			//	now schedule any affinity groups that were not explicitly scheduled, largest first
			while(groupsScheduled < execution.groups.size())
			{
				//	find the largest unscheduled group
				UINT32 largestsize = 0;
				INT32 nextGroupToSchedule = -1;
				for (UINT32 g=0; g<execution.groups.size(); g++)
				{
					//	is unscheduled?
					if (execution.groups[g].voice == VOICE_UNDEFINED)
					{
						//	is larger than our current best bet?
						if (execution.groups[g].members > largestsize)
						{
							largestsize = execution.groups[g].members;
							nextGroupToSchedule = g;
						}
					}
				}

				//	if we didn't find one, we've only got empty groups left
				if (nextGroupToSchedule == -1)
				{
					//	warn
					fout << "some affinity groups had no processes assigned to them - either they matched no processes, or the processes were more specifically matched in other groups" << D_WARN;

					//	ok
					break;
				}

				//	find the voice with the smallest process count
				UINT32 smallestsize = 0xFFFFFFFF;
				VoiceIndex voice = VOICE_UNDEFINED;
				for (VoiceIndex v=0; v<engineData.core.getVoiceCount(); v++)
				{
					if (assignedProcessCount[v] < smallestsize)
					{
						smallestsize = assignedProcessCount[v];
						voice = v;
					}
				}

				//	unlikely
				if (voice == VOICE_UNDEFINED)
					ferr << E_INTERNAL << "scheduler error";

				//	schedule this group on that voice
				execution.groups[nextGroupToSchedule].voice = voice;
				assignedProcessCount[voice] += execution.groups[nextGroupToSchedule].members;
				groupsScheduled++;

				//	report
				if (voice == engineData.core.getVoiceIndex())
				{
					fout << "Computing Affinity Group " << nextGroupToSchedule << " here:" << D_VERB;
					for (UINT32 w=0; w<unscheduled.size(); w++)
					{
						if (processGroup[w] == nextGroupToSchedule)
							fout << unscheduled[w]->getName() << D_FULL;
					}
				}
				else fout << "Affinity Group " << nextGroupToSchedule << " on Voice " << unitIndex(voice) << D_VERB;
			}

			//	now, schedule each process either by its affinity group
			//	or by round-robin
			fout << "Computing Other Processes:" << D_VERB;
			for (UINT32 w=0; w<unscheduled.size(); w++)
			{
				INT32 group = processGroup[w];
				VoiceIndex voice = VOICE_UNDEFINED;
				bool processIsNotInAffinityGroup = false;

				//	check for scheduling by affinity group
				if (group != -1)
				{
					voice = execution.groups[group].voice;
				}

				//	else schedule by round-robin
				else
				{
					UINT32 smallestsize = 0xFFFFFFFF;
					for (UINT32 v=0; v<engineData.core.getVoiceCount(); v++)
					{
						if (assignedProcessCount[v] < smallestsize)
						{
							smallestsize = assignedProcessCount[v];
							voice = v;
							processIsNotInAffinityGroup = true;
						}
					}
				}

				//	check
				if (voice == VOICE_UNDEFINED)
					ferr << E_INTERNAL << "scheduler error";

				//	schedule
				if (processIsNotInAffinityGroup) assignedProcessCount[voice]++;
				if (voice == engineData.core.getVoiceIndex())
				{
					//	store it for processing
					processes.push_back(unscheduled[w]);

					//	report it if it's scheduled by round-robin
					if (processIsNotInAffinityGroup) fout << unscheduled[w]->getName() << D_FULL;
				}
				else
				{
					//	mark that we're computing it elsewhere
					processesOnOtherNodes.push_back(unscheduled[w]->getName());

					//	and delete it
					delete unscheduled[w];
				}
				unscheduled[w] = NULL;
			}

			//	report on scheduling results
			fout << "(list ends)" << D_FULL;
			for (UINT32 v=0; v<engineData.core.getVoiceCount(); v++)
				fout << "Voice " << unitIndex(v) << " will compute " << assignedProcessCount[v] << " processes" << D_VERB;



		////////////////	MAKE LINKS

			//	report
			fout << "Make Links" << D_VERB;

			//	for each link
			for (UINT32 l=0; l<links.size(); l++)
			{
				//	dst process name
				string dstName = links[l]->getDstProcessName();

				//	find the object amongst processes
				UINT32 p;
				for (p=0; p<processes.size(); p++)
					if (dstName == processes[p]->getName()) break;

				//	if link dst is not on this voice, let's check it is on another one...
				if (p == processes.size())
				{
					for (p=0; p<processesOnOtherNodes.size(); p++)
					{
						if (processesOnOtherNodes[p] == dstName)
							break;
					}

					//	if not, throw
					if (p == processesOnOtherNodes.size())
						ferr << E_SYSTEM_FILE << "link dst process \"" << dstName << "\" not found in system";
				}

				//	if it is, link to it from the dst process object
				else
				{
					processes[p]->addLink(links[l]);
					fout << links[l]->getSrc() << " ==> " << links[l]->getDst() << D_VERB;
				}
			}

		////////////////	SET RANDOM SEEDS

			//	report
			fout << "Derive Implicit Seeds" << D_VERB;



		////////////////	LOAD COMPONENT MODULES

			/*

				Why?

				So that we have ModuleInfo available for the next section.
				The processes are not instantiated, however, until the
				worker thread is running - thus, all their events are called
				in the same thread context (at least with dynamic load
				balancing switched off).
			*/

			//	report
			fout << "Load Component Modules" << D_VERB;

			//	for each process
			for (UINT32 p=0; p<processes.size(); p++)
			{
				//	report
				fout << processes[p]->getName() << " (" << processes[p]->getClassName() << ")" << D_VERB;

				//	load
				processes[p]->load(fout);
			}
		}

		UINT32 System::getProcessCount()
		{
			return processes.size();
		}

		Process* System::getProcessByIndex(UINT32 index)
		{
			if (index >= processes.size())
				ferr << E_INTERNAL << "index out of range";

			return processes[index];
		}

		void System::listMissingInputs()
		{
			brahms::output::Source& fout(engineData.core.caller.tout);

			fout << "Deadlock - Available Outputs:" << D_INFO_UNHIDEABLE;

			//	list outputs available on peer voices
			for (UINT32 v=0; v<peerVoices.size(); v++)
			{
				if (v == ((UINT32)engineData.core.getVoiceIndex()))
				{
					fout << "Voice " << v << " (this Voice)" << D_INFO_UNHIDEABLE;

					//	for each process
					for(UINT32 p=0; p<processes.size(); p++)
					{
						Process* process = processes[p];
						process->oif.listAvailableOutputs(fout);
					}

					//	ok
					continue;
				}

				PeerVoice& voice = peerVoices[v];
				stringstream ss;
				fout << "Voice " << v << D_INFO_UNHIDEABLE;

				for (UINT32 o=0; o<voice.outputs.size(); o++)
					fout << voice.outputs[o] << D_INFO_UNHIDEABLE;
			}

			fout << "Deadlock - Missing Inputs:" << D_INFO_UNHIDEABLE;

			UINT32 count = 0;
			for(UINT32 p=0; p<processes.size(); p++)
			{
				//	get process
				Process* process = processes[p];

				//	list each set
				count += process->iif.listMissingInputs(process->getObjectName().c_str(), fout);
			}

			if (!count) fout << "(none - deadlock occurred in another voice)" << D_INFO;
		}

		PassResult System::localPass(brahms::thread::Workers& workers)
		{
			brahms::output::Source& fout(engineData.core.caller.tout);

			//	prepare return result
			PassResult prLocal = { false, true };

			//	for each process
			for(UINT32 p=0; p<processes.size(); p++)
			{
				//	report
				Process* process = processes[p];
				fout << "passing \"" + process->getObjectName() + "\"" << D_FULL;

				//	get set of all unseen inputs
				vector<InputPortLocal*> unseenInputs = process->getUnseenInputs();
				fout << "unseen inputs: " << unseenInputs.size() << D_FULL;

				//	if process has seen all its inputs, and received
				//	at least one CONNECT call, move on
				if (!unseenInputs.size() && !process->EVENT_INIT_CONNECT_firstCall)
					continue;

				//	otherwise, get as many inputs as possible
				UINT32 numInputsNewlyConnected = 0;

				//	for each input to this process
				if (unseenInputs.size())
					fout << "collecting inputs for process..." << D_FULL;

				for (UINT32 i=0; i<unseenInputs.size(); i++)
				{
					//	find the required input amongst the outputs of other processes
					OutputPort* output = findOutput(unseenInputs[i]->link->src);

					//	if we got one, connect it
					if (output)
					{
						fout << "found \"" << string(unseenInputs[i]->link->src) << "\"" << D_FULL;

						//	connect the output port to the input port
						//	and return the current data object in the output port
						UINT32 lag = unseenInputs[i]->link->lag;
						output->connectInput(unseenInputs[i], lag, fout);

						//	ok, we're done for this input since we've found it
						numInputsNewlyConnected++;
					}
				}



		////////////////	RUN ITS INIT PHASE_CONNECT

				//	respect flag F_NEEDS_ALL_INPUTS
				if (
					(process->componentInfo->flags & F_NEEDS_ALL_INPUTS) &&
					(numInputsNewlyConnected != unseenInputs.size())
					)
				{
					fout << "... F_NEEDS_ALL_INPUTS" << D_FULL;
					prLocal.finished = false;
					continue;
				}

				//	prepare event
				brahms::EventEx event(
					EVENT_INIT_CONNECT,
					0,
					process,
					NULL
				);

				//	prepare flags
				if (process->EVENT_INIT_CONNECT_firstCall)
				{
					process->EVENT_INIT_CONNECT_firstCall = false;
					event.flags |= F_FIRST_CALL;
				}
				if (numInputsNewlyConnected == unseenInputs.size())
				{
					event.flags |= F_LAST_CALL;
				}

				//	measure the number of datas in this process's output interface
				UINT32 outputPortsBefore = process->oif.getPortCount();

				//	let thread call event
				engineData.flag_allowCreatePort = true;
				workers.fireSingleEvent(event);
				engineData.flag_allowCreatePort = false;

				//	measure the number of datas in this process's output interface
				UINT32 outputPortsAfter = process->oif.getPortCount();

				//	any new datas in this process's output interface have been newly created
				UINT32 outputPortsCreated = outputPortsAfter - outputPortsBefore;

				//	if some new outputs were created
				if (outputPortsCreated)
				{
					fout << "process created " << outputPortsCreated << " new outputs" << D_VERB;

					//	mark progress
					prLocal.progress = true;

					//	get all output ports
					vector<OutputPort*> outputs = process->getAllOutputPorts();

					//	for each output port
					for (UINT32 o=0; o<outputs.size(); o++)
					{
						//	extract
						OutputPort* port = outputs[o];
						Data* data = port->getDueData(); // is placed on dueData in port constructor

						//	ignore old ones
						if (!port->newlyCreated) continue;

						//	report
						fout << "new output created \"" << port->getObjectName() << "\"" << D_VERB;

						//	convert its sample rate to lowest terms
						data->componentTime.sampleRate = toLowestTerms(data->componentTime.sampleRate);

						//	if unnamed, name it
						if (!port->getObjectName().length())
						{
							string name = "out" + brahms::text::n2s(o+1, 4);
							fout << "renaming to \"" << name << "\"" << D_FULL;
							port->setObjectName(name);
						}

						//	now, whether user or framework named the port, construct the
						//	full name of the data object (at this point, the data object
						//	name is the empty string)
						if (!port->parentSet) ferr << E_INTERNAL;
						string dataName = process->getObjectName() + ">>" + port->parentSet->getObjectName() + ">" + port->getObjectName();
						data->setObjectName(dataName);

						/*

							**** NOTES on naming: ****

								The system, process and data are named by their SystemML Identifiers (relative to the system root):

								item			example name

								system			mysys/mysubsys
								process			mysys/mysubsys/myprocess
								data			mysys/mysubsys/myprocess>>>myport
								data			mysys/mysubsys/myprocess>>myset>myport

								All ports, however, input and output, are named simply

								item			example name

								input port		myport
								output port		myport

								EXCEPT: remote OutputPort's (output ports created to
								represent outputs that are physically located on another
								voice) are given the name of their Data object (e.g.
								mysys/mysubsys/myprocess>>>myport). this is ok, since we
								don't search them in the normal way, but search them linearly
								looking for an explicit data name. so it works. but it's
								perhaps a bit weird?

						*/

						//	set state of zeroth buffer (the one the process just
						//	created) and all other buffers will be copied from this
						//	one so they will have the same state
						/*

							The structure of the data object has already been set by the
							creating process. We want to be able to write the front buffer
							object contents from StateML, but without changing
							structure. Therefore, we record structure string
							before and after use, allowing us to confirm that it doesn't
							change.

						*/
						fout << "setting data object initial state" << D_FULL;

						//	before
						EventGenericStructure eas = {0};
						brahms::EventEx event(EVENT_GENERIC_STRUCTURE_GET, 0, data, &eas, true, &fout);
						event.fire();
						string structureBefore = eas.structure;
						if (!structureBefore.length())
							ferr << E_NOT_COMPLIANT << "object did not support EVENT_GENERIC_STRUCTURE_GET";

						//	call data object to set its state
						EventStateSet ess;
						____CLEAR(ess);
						ess.flags = F_ZERO;
						brahms::EventEx event2(EVENT_STATE_SET, 0, data, &ess, true, &fout);
						event2.fire();

						//	after
						event.fire();
						string structureAfter = eas.structure;
						if (structureBefore != structureAfter)
							ferr << E_NOT_COMPLIANT << "call to EVENT_STATE_SET during init changed structure of data object: probably indicates invalid content tag in Link";

						//	announce
						announceOutput(dataName);
					}
				}

				//	if not finished, mark
				if (numInputsNewlyConnected != unseenInputs.size())
				{
					fout << "(not all inputs have yet been seen - more to do)" << D_FULL;
					prLocal.finished = false;
				}

			}	// each process

			//	signal remote Voices we're done with their services
			//	message contents are two 0/1 characters representing finished/progress state
			//	of this Voice at the end of its local pass for this global pass of connect phase

			//	send ENDPHASE to all Voices except ourselves
			fout << "sending ENDPHASE" << D_VERB;
			for (VoiceIndex remoteVoiceIndex=0; remoteVoiceIndex<engineData.core.getVoiceCount(); remoteVoiceIndex++)
			{
				//	not to ourselves
				if (remoteVoiceIndex == engineData.core.getVoiceIndex()) continue;

				//	construct message
				brahms::base::IPM* ipms = engineData.pool.get(brahms::base::IPMTAG_ENDPHASE, remoteVoiceIndex);
				ipms->header().endPhaseData.finished = prLocal.finished;
				ipms->header().endPhaseData.progress = prLocal.progress;

				//	send it
				comms.push(ipms, fout);
			}

			//	ok
			return prLocal;
		}

		void System::connectPhaseServe(UINT32 remoteVoiceIndex, bool &globalFinished, bool &globalProgress)
		{
			brahms::output::Source& fout(engineData.core.caller.tout);

			while(true)
			{
				//	get message from client
				brahms::base::IPM* ipmr = NULL;
				comms.pull(remoteVoiceIndex, ipmr, brahms::base::IPMTAG_UNDEFINED, fout, brahms::channel::COMMS_TIMEOUT_DEFAULT, true);

				//	switch on message we were sent
				switch (ipmr->header().tag)
				{
					case brahms::base::IPMTAG_ENDPHASE:
					{
						//	extract remote connection's finished/progress states and update our local global states with respect to these
						if (!ipmr->header().endPhaseData.finished) globalFinished = false;
						if (ipmr->header().endPhaseData.progress) globalProgress = true;
						fout << "received ENDPHASE" << D_VERB;

						//	release message
						ipmr->release();

						//	close server
						return;
					}

					case brahms::base::IPMTAG_ANNOUNCEOUTPUT:
					{
						string outputName = (const char*)ipmr->body();
						fout << "received IPMTAG_ANNOUNCEOUTPUT: \"" << outputName << "\"" << D_VERB;

						//	store
						peerVoices[remoteVoiceIndex].outputs.push_back(outputName);

						//	release message
						ipmr->release();

						//	ok
						break;
					}

					case brahms::base::IPMTAG_FINDOUTPUT:
					{
						//	look for requested inlet in our local table
						string outputName = (const char*)ipmr->body();
						fout << "received IPMTAG_FINDOUTPUT (" << outputName << ")" << D_VERB;

						//	release message
						ipmr->release();

						//	NOTE: we only want to find a local one - if we have it, but it's not local,
						//	some other voice is better placed than us to supply it!
						Identifier outputNameIdentifier;
						outputNameIdentifier.parse(outputName.c_str(), ST_OUTPUT_PORT, "argument of IPMTAG_FINDOUTPUT");
						OutputPort* port = findOutputLocally(outputNameIdentifier);
						
						//	if found
						if (port)
						{
							/*	DOCUMENTATION: PUSHDATA_IDENTICAL_TO_ALL_SENDERS

								Search for the tag PUSHDATA_IDENTICAL_TO_ALL_SENDERS in port.cpp for full
								details. Here, we implement the strategy of making sure multiple
								InputPortRemote's attached to the same output get the same msgStreamID.
								Note that this is still guaranteed to be "unique" at the remote voice,
								since only one of these InputPortRemote's is connected to each remote voice.

								(UNSORTED NOTES...)

								The IPMTAG_OUTPUTFOUND returns with a "msgStreamID" value
								which is the index of the Inlet to which the newly
								created outlet is attached in this voice. This value is
								unique to this Inlet in this voice, and, thus, to the
								PUSHDATA connection on the channel it will be sent over
								at runtime.

								At the other end, when the message is received, a new
								InletRemote is created. PUSHDATA messages, at runtime,
								will be given the same "msgStreamID" value as this message,
								so the receiving voice can associate the msgStreamID value
								with a pointer to the newly created InletRemote.

								Then, when the receiving thread gets PUSHDATA messages,
								it can use the msgStreamID value to look up the pointer
								to the Inlet in its msgStreamID table, and therefore push
								the messages directly into the proper Inlet.

								HOWEVER!

								There may be multiple target peers that receive the same
								stream from this voice. See notes in port.cpp as to why
								they *must* accept the same msgStreamID. Therefore, if the
								stream is already being sent to another voice, we use
								the *same* msgStreamID in the later connection too...

								THIS IS STILL BAD, in that the target will maintain
								a sparse array (not that bad) and that we will make
								a separate call to EVENT_GET_CONTENT for every attached
								peer voice (this is potentially quite bad, though not
								that bad for our supplied data classes because they don't
								do a copy operation as part of that EVENT).

							//	message stream ID need only be unique between us and the
							//	peer voice who is asking for this, but this algorithm is
							//	easy, at least. TODO: it's probably not good though, because
							//	the receiving voice will keep a sparse array as a routing
							//	table, potentially wasteful with large systems. better to
							//	index per inter-voice channel rather than per system.

							*/

							//	old code works unless multiple InputPortRemote's attach to the same output
							UINT32 msgStreamID = inputPortRemotes.size();

							//	subtlety introduced to fix that problem (see documentation above)
							for (UINT32 i=0; i<inputPortRemotes.size(); i++)
							{
								//	if an existing port has the same name we were seeking...
								if (inputPortRemotes[i]->getObjectName() == outputName)
								{
									//	then just steal the msgStreamID from it!
									msgStreamID = inputPortRemotes[i]->msgStreamID;
									break;
								}
							}

							//	report
							fout << "(found)" << D_VERB;
							fout << "serializing..." << D_FULL;



							/*	DOCUMENTATION: INTERTHREAD_SIGNALLING
					
								We do syncing across an inlet/outlet pair if the two objects are running in
								different threads. Remote outlets are given thread INDEX_NOT_SET so that they
								must be different from all phsyical inlets on the local node, and thus will
								be synced.
							*/

							//	create a new "remote input"
							InputPortRemote* input = new InputPortRemote(
								outputName,
								engineData,
								comms.channels[remoteVoiceIndex],
								msgStreamID,
								remoteVoiceIndex
								);
							inputPortRemotes.push_back(input);

							//	report
//							____INFO("new InputPortRemote");

							//	connect to output port (zero lag for these, always)
							port->connectInput(input, 0, fout);
							port->connectRemoteInput(input);

							//	get zeroth data object, for reference data
							Data* data = port->getZerothData();
							const ModuleInfo* moduleInfo = data->module->getInfo();
							const ComponentInfo* componentInfo = data->getComponentInfo();

							//	get wrapped data object, and have it serialize with structure for passing on to remote process
							brahms::xml::XMLNode nodeSerial("Serial");
							brahms::xml::XMLNode* nodeSpec = nodeSerial.appendChild(new brahms::xml::XMLNode("Spec"));
							nodeSpec->appendChild(new brahms::xml::XMLNode("Class", componentInfo->cls));
							nodeSpec->appendChild(new brahms::xml::XMLNode("Binding", brahms::text::n2s(moduleInfo->binding).c_str()));
							nodeSpec->appendChild(new brahms::xml::XMLNode("Release", brahms::text::n2s(componentInfo->componentVersion->release).c_str()));

							//	serialize base class state (Component)
							brahms::xml::XMLNode* nodeComponent = nodeSerial.appendChild(new brahms::xml::XMLNode("Component"));
							nodeComponent->appendChild(new brahms::xml::XMLNode("Name", data->getObjectName().c_str()));
							
							//	serialize base class state (Data)
							ComponentTime* time = &data->componentTime;
							brahms::xml::XMLNode* nodeData = nodeSerial.appendChild(new brahms::xml::XMLNode("Data"));
							nodeData->appendChild(new brahms::xml::XMLNode("sampleRateNum", brahms::text::n2s(time->sampleRate.num).c_str()));
							nodeData->appendChild(new brahms::xml::XMLNode("sampleRateDen", brahms::text::n2s(time->sampleRate.den).c_str()));

							//	serialize derived class state (SystemML Object)
							EventStateGet esg;
							____CLEAR(esg);
							esg.precision = PRECISION_NOT_SET;
							brahms::EventEx event(
								EVENT_STATE_GET,
								0,
								data,
								&esg,
								true,
								&fout
							);
							event.fire();

							if (!esg.state)
								ferr << E_NOT_COMPLIANT << "data object \"" << data->getObjectName() << "\" did not return state in EVENT_STATE_GET";
							brahms::xml::XMLNode* xnode = objectRegister.resolveXMLNode(esg.state);
							if (xnode->nodeName() != string(""))
								ferr << E_NOT_COMPLIANT << "data object \"" << data->getObjectName() << "\" set state node name in EVENT_STATE_GET";
							xnode->nodeName("State");
							nodeSerial.appendChild(xnode);

							//	serialize it
							stringstream ss;
							nodeSerial.serialize(ss);
							string sss = ss.str();

							//	respond that we have it, and pass serialized object to remote process
							brahms::base::IPM* ipms = engineData.pool.get(brahms::base::IPMTAG_OUTPUTFOUND, remoteVoiceIndex);
							ipms->header().msgStreamID = msgStreamID;
							ipms->appendString(sss);

							//	send it
							comms.push(ipms, fout);
							fout << "(pushed IPMTAG_OUTPUTFOUND)" << D_FULL;

							//	report
							fout << "OK" << D_VERB;
						}

						//	if not found
						else
						{
							//	respond that we don't have it
							fout << "(not found)" << D_VERB;
							brahms::base::IPM* ipms = engineData.pool.get(brahms::base::IPMTAG_OUTPUTNOTFOUND, remoteVoiceIndex);
							comms.push(ipms, fout);
							fout << "(pushed IPMTAG_OUTPUTNOTFOUND)" << D_VERB;
						}

						//	ok
						break;
					}

					default:
					{
						//	we don't understand!
						UINT8 tag = ipmr->header().tag;

						//	release message
						ipmr->release();

						//	badness
						ferr << E_COMMS << "unexpected message received in connectPhaseServe() (" << tag << ")";
					}
				}
			}
		}

		OutputPort* System::findOutputLocally(Identifier& outputName)
		{
			//	look for it locally
			for (UINT32 p=0; p<processes.size(); p++)
			{
				if (processes[p]->getObjectName() == outputName.processName)
					return processes[p]->findOutput(outputName);
			}

			//	not found
			return NULL;
		}

		OutputPort* System::findOutput(Identifier& outputName)
		{
			brahms::output::Source& fout(engineData.core.caller.tout);

			//	look for it locally
			OutputPort* localPort = findOutputLocally(outputName);
			if (localPort) return localPort;

			//	look for it amongst already-connected remote outputs
			string dataName = string(outputName);
			for (UINT32 i=0; i<outputPortRemotes.size(); i++)
			{
				if (outputPortRemotes[i]->getObjectName() == dataName)
					return outputPortRemotes[i];
			}

			//	ask peers if they have it, and connect a stream if so
			for (UINT32 remoteVoiceIndex=0; remoteVoiceIndex<engineData.core.getVoiceCount(); remoteVoiceIndex++)
			{
				//	skip ourselves
				if (remoteVoiceIndex == ((UINT32)engineData.core.getVoiceIndex())) continue;

				//	only ask remote voice for this object if our register (created
				//	by servicing IPMTAG_ANNOUNCEOUTPUT messages) says that they
				//	have it (otherwise, the call would be a waste of time)
				bool available = false;
				PeerVoice& peerVoice = peerVoices[remoteVoiceIndex];
				for (UINT32 o=0; o<peerVoice.outputs.size(); o++)
				{
					if (peerVoice.outputs[o] == dataName)
					{
						available = true;
						break;
					}
				}
				if (!available) continue;

				//	ask remote Voice for this data object
				brahms::base::IPM* ipms = engineData.pool.get(brahms::base::IPMTAG_FINDOUTPUT, remoteVoiceIndex);
				ipms->appendString(dataName);
				comms.push(ipms, fout);
				fout << "sent IPMTAG_FINDOUTPUT" << D_VERB;

				//	get response
				brahms::base::IPM* ipmr;
				comms.pull(remoteVoiceIndex, ipmr, brahms::base::IPMTAG_UNDEFINED, fout, brahms::channel::COMMS_TIMEOUT_DEFAULT, true);

				//	if it had it, we need to negotiate a connection now - eek!
				if (ipmr->header().tag == brahms::base::IPMTAG_OUTPUTFOUND)
				{
					fout << "recv IPMTAG_OUTPUTFOUND" << D_VERB;

					//	message content will be a serialized (with structure) data object
					brahms::xml::XMLNode nodeSerial;
					nodeSerial.parse((const char*)ipmr->body());
					brahms::xml::XMLNode* nodeSpec = nodeSerial.getChild("Spec");
					string className = nodeSpec->getChild("Class")->nodeText();
					string language = nodeSpec->getChild("Binding")->nodeText();
					string release = nodeSpec->getChild("Release")->nodeText();
					UINT16 urelease = atof(release.c_str());

					//	unserialize base class state (Component)
					brahms::xml::XMLNode* nodeComponent = nodeSerial.getChild("Component");
					string name = nodeComponent->getChild("Name")->nodeText();

					//	unserialize base class state (Data)
					brahms::xml::XMLNode* nodeData = nodeSerial.getChild("Data");
					SampleRate sampleRate;
					sampleRate.num = atof(nodeData->getChild("sampleRateNum")->nodeText());
					sampleRate.den = atof(nodeData->getChild("sampleRateDen")->nodeText());

					//	instantiate the data object
					Data* data = new Data(name.c_str(), engineData, &fout, className, urelease, sampleRate);

					//	load component module
					data->load(fout);

					//	instantiate the data component
					data->instantiate(&fout);

					//	unserialize derived class state (SystemML Object)
					EventStateSet ess;
					____CLEAR(ess);
					ess.state = nodeSerial.getChild("State")->getObjectHandle();
					brahms::EventEx event(
						EVENT_STATE_SET,
						0,
						data,
						&ess,
						true,
						&fout
					);
					event.fire();

					//	create new port
					/* SEE NOTES on naming */
					outputPortRemotes.push_back(new OutputPort(name.c_str(), engineData, data, NULL));

					//	report
//					____INFO("new OutputPort(Remote)\n");

					//	set port due data to zeroth buffer so that it's available for sml_getPortData() before run phase begins
					outputPortRemotes.back()->setDueData(data);

					/*	DOCUMENTATION: INTERTHREAD_SIGNALLING

						we do syncing across an inlet/outlet pair if the two
						objects are running in different threads. remote OutputPorts
						have no "parentSet", so they are always known to be running
						in different threads from any worker thread, and are thus
						not given redundant thread-locking.
					*/

					//	add entry to channel's routing table
					comms.addRoutingEntry(remoteVoiceIndex, ipmr->header().msgStreamID, pushdataHandler, outputPortRemotes.back());

					//	report
					fout << "OK" << D_FULL;

					//	release
					ipmr->release();

					//	ok, we can return it now
					return outputPortRemotes.back();
				}

				else if (ipmr->header().tag == brahms::base::IPMTAG_OUTPUTNOTFOUND)
				{
					//	release
					ipmr->release();

					//	no action, not yet found
					fout << "recv IPMTAG_OUTPUTNOTFOUND" << D_VERB;
				}

				else
				{
					//	release
					ipmr->release();

					//	badness
					ferr
						<< E_COMMS
						<< "invalid response received to IPMTAG_FINDOUTPUT ("
						<< brahms::base::TranslateIPMTAG(ipmr->header().tag) << ")"
						;
				}
			}

			//	accept defeat
			return NULL;
		}

		void System::announceOutput(string name)
		{
			brahms::output::Source& fout(engineData.core.caller.tout);

			//	tell peers that named output is now available on this voice
			//	ask peers if they have it, and connect a stream if so
			for (UINT32 remoteVoiceIndex=0; remoteVoiceIndex<engineData.core.getVoiceCount(); remoteVoiceIndex++)
			{
				//	skip ourselves
				if (remoteVoiceIndex == ((UINT32)engineData.core.getVoiceIndex())) continue;

				//	ask remote Voice for this data object
				brahms::base::IPM* ipms = engineData.pool.get(brahms::base::IPMTAG_ANNOUNCEOUTPUT, remoteVoiceIndex);
				ipms->appendString(name);
				comms.push(ipms, fout);
				fout << "sent IPMTAG_ANNOUNCEOUTPUT: \"" << name << "\"" << D_VERB;
			}
		}

		string parentSystem(string pathToSubSystem)
		{
			//	if non-empty, just remove the trailing slash
			if (pathToSubSystem.length())
				return pathToSubSystem.substr(0, pathToSubSystem.length() - 1);

			//	if empty, return something inaccessible so the expose never gets fired
			return "$inaccessible$";
		}

		vector<OutputPort*> System::getAllOutputPorts()
		{
			vector<OutputPort*> ret;

			for (UINT32 p=0; p<processes.size(); p++)
			{
				vector<OutputPort*>& ports = processes[p]->oif.ports;
				for (UINT32 op=0; op<ports.size(); op++)
					ret.push_back(ports[op]);
			}

			return ret;
		}

		void System::parse(brahms::xml::XMLNode* systemML, string pathToSubSystem)
		{
			brahms::output::Source& fout(engineData.core.caller.tout);

			//	get children of passed tags
			const brahms::xml::XMLNodeList* children = systemML->childNodes();
			UINT32 count = children->size();

			//	parse each in turn
			for(UINT32 c=0; c<count; c++)
			{
				//	extract child
				brahms::xml::XMLNode* child = children->at(c);
				string childTagName(child->nodeName());

				//	ignore some sub tags that we don't currently use
				if (childTagName == "Name") continue;
				if (childTagName == "Title") continue;

				//	handle "System"
				if (childTagName == "System")
				{
					//	get sub-system name
					string subSystemName = child->getChild("Name")->nodeText();

					//	do sub-system
					parse(child, pathToSubSystem + subSystemName + "/");
					continue;
				}

				//	is "Process"?
				if (childTagName == "Process")
				{
					//	extract object name
					Identifier processIdentifier;
					string processName = child->getChild("Name")->nodeText();
					processIdentifier.parse(processName.c_str(), ST_PROCESS, ("name of process"));
					if (processIdentifier.hasPath) ferr << E_SYSTEM_FILE << "process name \"" << processName << "\" is illegal (should not contain slashes)";
					processName = pathToSubSystem + processName;
					processIdentifier.parse(processName.c_str(), ST_PROCESS, ("name of process"));

					//	extract class name
					string className = child->getChild("Class")->nodeText();

					//	extract sample rate
					SampleRate sampleRate = parseSampleRate(child->getChild("Time")->getChild("SampleRate")->nodeText());

					//	store seed if supplied
					VUINT32 seed;
					if (child->hasChild("Seed"))
					{
						string s = child->getChild("Seed")->nodeText();
						while(true)
						{
							if (!s.length()) break;

							size_t pos = s.find(" ");
							if (pos == string::npos)
							{
								DOUBLE d = brahms::text::s2n(s);
								if (d == brahms::text::S2N_FAILED)
									ferr << E_SYSTEM_FILE << "failed to interpret Seed value";
								UINT32 u = d;
								stringstream ss;
								ss << u;
								if (ss.str() != s)
									ferr << E_SYSTEM_FILE << "failed to interpret Seed value";
								seed.push_back(u);
								break;
							}

							string v = s.substr(0, pos);
							DOUBLE d = brahms::text::s2n(v);
							if (d == brahms::text::S2N_FAILED)
								ferr << E_SYSTEM_FILE << "failed to interpret Seed value";
							UINT32 u = d;
							stringstream ss;
							ss << u;
							if (ss.str() != v)
								ferr << E_SYSTEM_FILE << "failed to interpret Seed value";
							seed.push_back(u);
							
							s = s.substr(pos+1);
							while(s.length() && s[0] == ' ')
								s = s.substr(1);
						}
					}

					if (!seed.size())
					{
						//	use exec file seed, if there is one
						seed = engineData.execution.seed;
						if (seed.size())
						{
							//	increment
							UINT32 MinUniqueSeedsPerProcess = engineData.core.execPars.getu("MinUniqueSeedsPerProcess");
							UINT32 indexInSystem = processes.size();
							seed[0] += (indexInSystem+1) * MinUniqueSeedsPerProcess;
							if (seed[0] < 0x80000000) ferr << E_SEED_OVERFLOW << "whilst generating execution seeds";
						}
					}

					//	create new process
					processes.push_back(new Process(processName, engineData, &fout, className, sampleRate, child, seed, processes.size()));

					//	ok
					continue;
				}

				//	is "Link"?
				if (childTagName == "Link")
				{
					//	extract object name
					//	this tag is not actually required, so we won't throw an error if it's absent
					string name;
					if (child->hasChild("name"))
						name = child->getChild("Name")->nodeText();

					//	extract data
					string src = child->getChild("Src")->nodeText();
					string dst = child->getChild("Dst")->nodeText();
					string lag = child->getChild("Lag")->nodeText();

					//	create new link
					links.push_back(new Link(name, pathToSubSystem, src, dst, lag));

					//	ok
					continue;
				}

				//	is "Expose"?
				if (childTagName == "Expose")
				{
					//	parse Name
					string exposeName;
					if (child->hasChild("name"))
						exposeName = child->getChild("Name")->nodeText();

					//	parse What and As as SystemML identifiers
					Identifier exposeWhat;
					exposeWhat.parse(child->getChild("What")->nodeText(), ST_NULL, child->getChild("What")->nodeName());
					Identifier exposeAs;
					exposeAs.parse(child->getChild("As")->nodeText(), ST_NULL, child->getChild("As")->nodeName());

					//	check not null
					if (exposeWhat.type == ST_NULL || exposeAs.type == ST_NULL)
						ferr << E_SYSTEM_FILE << "expose is invalid (unrecognised What or As)";

					//	As cannot has path
					if (exposeAs.hasPath)
						ferr << E_SYSTEM_FILE << "expose As cannot have slashes (must be exposed directly on the sub-system)";

					//	What must be a set or a port
					if (exposeWhat.type == ST_PROCESS)
					{
						if (!exposeName.length()) exposeName = "<unnamed>";
						string name = pathToSubSystem + exposeName;
						ferr << E_SYSTEM_FILE << "processes cannot be exposed (in Expose \"" << name << "\")";
					}

					//	strings
					string what = pathToSubSystem + string(exposeWhat);
					string as;

					//	handle each case separately, rather than trying to be clever!
					switch (exposeWhat.type)
					{
						case ST_INPUT_PORT:
						case ST_OUTPUT_PORT:
						{
							//	can be a process name (interpreted as port name)
							if (exposeAs.type == ST_PROCESS)
							{
								exposeAs.portName = exposeAs.processName;
								exposeAs.processName = "";
								exposeAs.type = exposeWhat.type;
							}

							//	can be a output port
							else if (exposeAs.type == exposeWhat.type)
							{
								//	can be in the form "set>port"
								if (exposeAs.processName.length() && !exposeAs.setName.length())
								{
									exposeAs.setName = exposeAs.processName;
									exposeAs.processName = "";
								}

								//	can be in the form ">>set>port" (leave it alone in this case)
							}

							else
							{
								ferr << E_SYSTEM_FILE << "invalid expose (output port must be exposed as output port)";
							}

							//	ok
							as = parentSystem(pathToSubSystem) + string(exposeAs);
							break;
						}

						case ST_INPUT_SET:
						case ST_OUTPUT_SET:
						{
							switch(exposeAs.type)
							{
								//	can be a process name (interpreted as set name)
								case ST_PROCESS:
								{
									exposeAs.setName = exposeAs.processName;
									exposeAs.processName = "";
									exposeAs.type = exposeWhat.type;
									break;
								}

								default:
									ferr << E_SYSTEM_FILE << "invalid expose (set must be exposed as set)";
							}

							//	ok
							as = parentSystem(pathToSubSystem) + string(exposeAs);
							break;
						}
						
						default:
							ferr << E_INTERNAL;
					}

					//	add expose to alias list
					if (what == as)
					{
						//	to avoid infinite loops, we don't add aliases that are to
						//	themselves (these are valid logically in SystemML)
						fout << "not exposing " << what << " as " << as << " (redundant)" << D_FULL;
					}
					else
					{
						//	if there are two exposes that are identical, we don't add
						//	the later one - this avoids problems when we resolve them later on
						bool alreadyExists = false;
						for (UINT32 a=0; a<exposes.size(); a++)
						{
							if (exposes[a]->what == what && exposes[a]->as == as)
							{
								alreadyExists = true;
								break;
							}
						}

						//	ok
						if (!alreadyExists)
							exposes.push_back(new Expose(what, as));
					}

					//	ok
					continue;
				}

				//	unrecognised
				fout << "SystemML parse warning: ignored unrecognised tag \"" + childTagName + "\"" << D_VERB;
			}
		}

		VSTRING System::resolveExposes(VSTRING identifiers)
		{
			brahms::output::Source& fout(engineData.core.caller.tout);

			//	if we are resolving dsts of a link, we may resolve
			//	any input item to multiple output items

			//	start with no outputs and assume no resolves made
			VSTRING outputs;
			bool resolvedSome = false;

			//	resolve each input in turn
			for (UINT32 i=0; i<identifiers.size(); i++)
			{
				//	extract input and assume this item not resolved
				string input = identifiers[i];
				bool resolvedThis = false;

				//	find it in alias table
				for (UINT32 a=0; a<exposes.size(); a++)
				{
					if (input == exposes[a]->as)
					{
						outputs.push_back(exposes[a]->what);
						resolvedSome = true;
						resolvedThis = true;

						fout << exposes[a]->as << " --> " << exposes[a]->what << D_FULL;
					}

					//	exposed sets can match only the beginning of the name, since
					//	we may have a set exposed as path<<set< and be trying to match
					//	path<<set<particularInputName

					/*

						CHANGE: 08/12/2009

						this code wasn't working, because we seem to have changed the way all
						identifiers are represented so that they *all* have a double chevron.

						instead: if the expose references a port, it must end in a port name.
						if it ends in a chevron, and since only sets and ports can be exposed,
						it *must* specify a set name. in that case only, we can allow substring
						matching.

					else if (exposes[a]->as.find(">>") != string::npos || exposes[a]->as.find("<<") != string::npos)

					*/

					else
					{
						size_t L = exposes[a]->as.length();
						char last = exposes[a]->as.at(L-1);
						if (last == '>' || last == '<')
						{
							//	match only as far as the alias specifies
							if (input.substr(0, L) == exposes[a]->as)
							{
								outputs.push_back(exposes[a]->what + input.substr(L));
								resolvedSome = true;
								resolvedThis = true;

								fout << exposes[a]->as << " --> " << exposes[a]->what << D_FULL;
							}
						}
					}
				}

				//	if this item not resolved, push it back as it stands
				if (!resolvedThis)
					outputs.push_back(input);
			}

			//	if any items resolved, recurse to handle multiple exposure (e.g. A exposed as B, B exposed as C)
			if (resolvedSome) return resolveExposes(outputs);

			//	otherwise, just return
			return outputs;
		}

		void System::getAllSampleRates(vector<SampleRate>& sampleRates)
		{
			for (UINT32 p=0; p<processes.size(); p++)
				processes[p]->getAllSampleRates(sampleRates);
		}

		void System::progress(brahms::output::Source& source)
		{
			for (UINT32 p=0; p<processes.size(); p++)
				processes[p]->progress(source);
		}

		void System::setDueDatas(brahms::output::Source& source)
		{
			for (UINT32 p=0; p<processes.size(); p++)
				processes[p]->setDueDatas(source);
		}

		void System::initInterThreadLocks(brahms::output::Source& source)
		{
			for (UINT32 p=0; p<processes.size(); p++)
				processes[p]->initInterThreadLocks(source);
		}

		void System::finalizeAllComponentTimes(SampleRate baseSampleRate, BaseSamples executionStop)
		{
			for (UINT32 p=0; p<processes.size(); p++)
				processes[p]->finalizeAllComponentTimes(baseSampleRate, executionStop);

			for (UINT32 o=0; o<outputPortRemotes.size(); o++)
				outputPortRemotes[o]->finalizeAllComponentTimes(baseSampleRate, executionStop);
		}

		void System::endInitPhase(brahms::output::Source& fout)
		{
			//	collect output ports
			vector<OutputPort*> oports;

			//	for each remote input
			for (UINT32 i=0; i<inputPortRemotes.size(); i++)
			{
				//	get attached output
				OutputPort* oport = inputPortRemotes[i]->connectedOutputPort;

				//	if not listed, add it
				for (UINT32 p=0; p<oports.size(); p++)
				{
					if (oports[p] == oport)
					{
						oport = NULL;
						break;
					}
				}
		
				//	list it
				if (oport) oports.push_back(oport);
			}

			//	for each output port, have it handle the next bit
			for (UINT32 p=0; p<oports.size(); p++)
			{
				//	send
				oports[p]->endInitPhase(fout);
			}
		}

		inline BaseSamples secondsToBaseSamples(DOUBLE t, DOUBLE base_T)
		{
			//	special case, -1.0 becomes BASE_SAMPLES_INF (indicates no end to a window)
			if (t == -1.0) return BASE_SAMPLES_INF;

			//	convert
			DOUBLE base_samples = t / base_T;

			//	now, we could just do a floor() operation on the resulting base sample count, but the
			//	trouble is that since the time instant is specified in floating point, it may be very
			//	very slightly under at this point. we don't want this to result in an incorrect time
			//	instant, so if the value is *just* under the base sample boundary, we poke it up.
			BaseSamples base_samples_floor = floor(base_samples);
			DOUBLE dif = base_samples - ((DOUBLE)base_samples_floor);
			if (dif > 0.999) base_samples++;

			//	ok
			return base_samples;
		}

		vector<LogOriginBaseSamples> originToBaseSamples(vector<LogOriginSeconds> originsf, BaseSamples samplePeriod, SampleRate baseSampleRate)
		{
			DOUBLE base_T = sampleRateToPeriod(baseSampleRate);

			vector<LogOriginBaseSamples> origins;
			origins.resize(originsf.size());

			for (UINT32 o=0; o<originsf.size(); o++)
			{
				//	get ref to next origins
				LogOriginBaseSamples& origin(origins[o]);
				LogOriginSeconds& originf(originsf[o]);

				//	convert
				origin.t0 = secondsToBaseSamples(originf.repeat.t0, base_T);
				origin.T  = secondsToBaseSamples(originf.repeat.T , base_T);
				origin.t1 = secondsToBaseSamples(originf.repeat.t1, base_T);

				//cout << "origin: " << origin.t0 << ":" << origin.T << ":" << origin.t1 << endl;

				//	and convert each window
				for (UINT32 w=0; w<originf.windows.size(); w++)
				{
					LogWindowBaseSamples lwbs;
					lwbs.t0 = secondsToBaseSamples(originf.windows[w].t0, base_T);
					lwbs.t1 = secondsToBaseSamples(originf.windows[w].t1, base_T);
					origin.windows.push_back(lwbs);

					//cout << "window: " << lwbs.t0 << ":" << lwbs.t1 << endl;
				}
			}

			return origins;
		}

		string precisionToString(INT32 precision)
		{
			if (precision == PRECISION_NOT_SET) return "<precision not set>";
			if (precision == PRECISION_DO_NOT_LOG) return "<precision do not log>";
			return brahms::text::n2s(precision);
		}

		void System::startLogs(brahms::thread::Workers& workers)
		{
			brahms::output::Source& fout(engineData.core.caller.tout);

			UINT32 subfiles = 0;

			//	keep a list of unlogged items
			VSTRING unloggedItems;

			//	for each process
			for(UINT32 p=0; p<processes.size(); p++)
			{
				//	get process
				Process* process = processes[p];
				const ComponentInfo* processInfo = process->getComponentInfo();
				string processName = process->getObjectName();

				//	initially, assume default log mode
				INT32 precision = engineData.execution.defaultLogMode.precision;
				bool encapsulated = engineData.execution.defaultLogMode.encapsulated;
				bool recurse = engineData.execution.defaultLogMode.recurse;
				vector<LogOriginSeconds> origins = engineData.execution.defaultLogMode.origins;

				//	find best match amongst specific log modes
				INT32 bestMatch = -1;
				UINT32 bestMatchLength = 0;
				for (UINT32 n=0; n<engineData.execution.logRules.size(); n++)
				{
					const LogRule& logRule = engineData.execution.logRules[n];

					if (logRule.name.length() < bestMatchLength) continue;
					if (logRule.name.length() > processName.length()) continue;
					if (logRule.name != processName.substr(0, logRule.name.length())) continue;

					//	if no recursion, only take exact match
					if (!logRule.mode.recurse && logRule.name.length() != processName.length()) continue;

					//	logRule name must specify a sub-system, process, or output, in which
					//	case the next character in the matched string must be "/", ">" or NULL
					if (logRule.name.length() != processName.length())
					{
						char c = processName[logRule.name.length()];
						if (c != '/' && c != '>')
						{
							//	no error - just ignore, and the user will get a warning
							continue;
						}
					}

					//	match
					bestMatch = n;
					bestMatchLength = logRule.name.length();
				}

				//	if any found, update log mode
				if (bestMatchLength)
				{
					LogRule& logRule = engineData.execution.logRules[bestMatch];
					logRule.logged.push_back(processName + " (Process)"); // mark that this rule caused this item to log, for audit
					precision = logRule.mode.precision;
					encapsulated = logRule.mode.encapsulated;
					recurse = logRule.mode.recurse;
					origins = logRule.mode.origins;
				}
				else unloggedItems.push_back(processName + " (Process)");

				//	if logging, advise the process component
				if (precision != PRECISION_DO_NOT_LOG)
				{
					//	prepare filename
					subfiles++;
					process->setSuggestedOutputFilename(engineData.execution.fileReport + "." + brahms::text::n2s(subfiles, 4));

					//	prepare event data
					process->logEventData.precision = precision;
					process->logEventData.flags = encapsulated ? F_ENCAPSULATED : 0;

					//	fire EVENT_LOG_INIT
					process->logEvent.type = EVENT_LOG_INIT;
					workers.fireSingleEvent(process->logEvent);
				}

				//	loop through its inputs checking compliance
				vector<InputPortLocal*> iports = process->getAllInputPorts();
				for (UINT32 i=0; i<iports.size(); i++)
				{
					//	get data
					InputPortLocal* port = iports[i];
					Data* data = port->getZerothData();

					//	if F_INPUTS_SAME_RATE, check input rate matches process rate
					if (processInfo->flags & F_INPUTS_SAME_RATE)
					{
						if (process->componentTime.samplePeriod != data->componentTime.samplePeriod)
							ferr << E_INPUT_RATE_MISMATCH << "process \"" << processName
								<< "\" specifies F_INPUTS_SAME_RATE but input \""
								<< data->getObjectName() << "\" does not share its sample rate ("
								<< sampleRateToRate(process->componentTime.sampleRate) << " != "
								<< sampleRateToRate(data->componentTime.sampleRate) << ")";
					}
				}

				//	loop through its outputs checking compliance and advising its outputs to log
				vector<OutputPort*> oports = process->getAllOutputPorts();
				for (UINT32 o=0; o<oports.size(); o++)
				{
					//	get data
					OutputPort* port = oports[o];
					Data* data = port->getZerothData();
					string dataName = data->getObjectName();

					//	if F_OUTPUTS_SAME_RATE, check output rate matches process rate
					if (processInfo->flags & F_OUTPUTS_SAME_RATE)
					{
						if (process->componentTime.samplePeriod != data->componentTime.samplePeriod)
							ferr << E_OUTPUT_RATE_MISMATCH << "process \"" << processName
								<< "\" specifies F_OUTPUTS_SAME_RATE but output \""
								<< dataName << "\" does not share its sample rate ("
								<< sampleRateToRate(process->componentTime.sampleRate) << " != "
								<< sampleRateToRate(data->componentTime.sampleRate) << ")";
					}

					//	initially, assume default log mode
					INT32 precision = engineData.execution.defaultLogMode.precision;
					bool encapsulated = engineData.execution.defaultLogMode.encapsulated;
					vector<LogOriginSeconds> origins = engineData.execution.defaultLogMode.origins;
					bool recurse = engineData.execution.defaultLogMode.recurse;

					//	find best match amongst specific log modes
					INT32 bestMatch = -1;
					UINT32 bestMatchLength = 0;
					for (UINT32 n=0; n<engineData.execution.logRules.size(); n++)
					{
						const LogRule& logRule = engineData.execution.logRules[n];

						if (logRule.name.length() < bestMatchLength) continue;
						if (logRule.name.length() > dataName.length()) continue;
						if (logRule.name != dataName.substr(0, logRule.name.length())) continue;

						//	if no recursion, only take exact match
						if (!logRule.mode.recurse && logRule.name.length() != dataName.length()) continue;

						//	log must specify a sub-system, process, or output, in which
						//	case the next character in the matched string must be "/", ">" or NULL
						if (logRule.name.length() != dataName.length())
						{
							char c = dataName[logRule.name.length()];
							if (c != '/' && c != '>')
							{
								//	no error - just ignore, and the user will get a warning
								continue;
							}
						}

						//	match
						bestMatch = n;
						bestMatchLength = logRule.name.length();
					}

					//	if any found, update log mode
					if (bestMatchLength)
					{
						LogRule& logRule = engineData.execution.logRules[bestMatch];

						logRule.logged.push_back(dataName); // mark that this rule caused this item to log, for audit
						precision = logRule.mode.precision;
						encapsulated = logRule.mode.encapsulated;
						origins = logRule.mode.origins;
						recurse = logRule.mode.recurse;
					}
					else unloggedItems.push_back(dataName);

					//	store log information in Data
					port->logEventData.precision = precision;
					port->origins = originToBaseSamples(origins, data->componentTime.samplePeriod, data->componentTime.baseSampleRate);

					//	if logging, advise the data component
					if (precision != PRECISION_DO_NOT_LOG)
					{
						//	start log of port
						port->startLog(
							encapsulated,
							engineData.execution.fileReport + "." + brahms::text::n2s(subfiles, 4),
							&fout
						);

						//	increment subfiles
						subfiles++;
					}
				}
			}

			//	if requested, show what was logged, what not, and why
//			{ FOUT_SECTION("Logging Report")

				for (UINT32 n=0; n<engineData.execution.logRules.size(); n++)
				{
					const LogRule& logRule = engineData.execution.logRules[n];

					string encap = logRule.mode.encapsulated ? "encapsulated" : "unencapsulated";
					string recur = logRule.mode.recurse ? "recurse" : "no recurse";
					fout << "\"" << logRule.name << "\" (precision " << precisionToString(logRule.mode.precision) << ", " << encap << ", " << recur << "):" << D_VERB;
					for (UINT32 l=0; l<logRule.logged.size(); l++)
						fout << logRule.logged[l] << D_FULL;
					if (!logRule.logged.size())
						fout << "(no matches)" << D_FULL;
				}
				if (engineData.execution.defaultLogMode.precision == PRECISION_DO_NOT_LOG)
				{
					fout << "\"default logging rule\" (do not log):" << D_VERB;
				}
				else
				{
					string encap = engineData.execution.defaultLogMode.encapsulated ? "encapsulated" : "unencapsulated";
					string recur = engineData.execution.defaultLogMode.recurse ? "recurse" : "no recurse";
					fout << "\"default logging rule\" (precision " << precisionToString(engineData.execution.defaultLogMode.precision) << ", " << encap << ", " << recur << "):" << D_VERB;
				}
				for (UINT32 n=0; n<unloggedItems.size(); n++)
					fout << unloggedItems[n] << D_FULL;
				if (unloggedItems.size() == 0) fout << "(none)" << D_FULL;

				//	if we are running Solo, check we've got no log requests left that weren't used, and warn if so
				//	(it's less easy to do this if running Concerto, because any voice may be supposed to log the
				//	item, and we won't know in this voice if another voice instantiated the process what outputs
				//	it chose to generate - we could do this through inter-voice communication but what a bore)
				if (engineData.core.getVoiceCount() == 1)
				{
					for (UINT32 n=0; n<engineData.execution.logRules.size(); n++)
					{
						const LogRule& logRule = engineData.execution.logRules[n];
						if (!logRule.logged.size())
							fout << "user requested to log \"" << logRule.name << "\", but nothing matching was found" << D_WARN;
					}
				}

//			}

		}



		Symbol pushdataHandler(void* arg, BYTE* stream, UINT32 count)
		{
			//	route to the appropriate OutputPortRemote
			brahms::systemml::OutputPort* port = (brahms::systemml::OutputPort*) arg;

			//	lock for write
			Symbol result = port->writeLock(0);

			//	if cancelled, abandon this data
			if (result == C_CANCEL)
				return C_CANCEL;

			//	get data object
			Data* data = port->getDueData();

			//	set content (unserialize)
			EventContent ec;
			ec.stream = stream;
			ec.bytes = count;
			brahms::EventEx event(
				EVENT_CONTENT_SET,
				0,
				data,
				&ec,
				true,
				NULL
			);
			event.fire();

			//	release
			port->writeRelease(0, NULL, 0);

			//	ok
			return C_OK;
		}




	}
}


