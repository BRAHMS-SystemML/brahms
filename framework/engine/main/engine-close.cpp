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

	$Id:: engine-close.cpp 2386 2009-11-17 19:15:06Z benjmitch $
	$Rev:: 2386                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-17 19:15:06 +0000 (Tue, 17 Nov 2009)       $
________________________________________________________________

*/

#include "main/engine.h"
using brahms::Engine;
#include "base/output.h"
using namespace brahms::output;
#include "support/xml.h"
using brahms::xml::XMLNode;
#include "base/text.h"
using brahms::text::n2s;

void Engine::close()
{
	FOUT_SECTION("Engine::close()")

	monitorSendPhase(EP_CLOSE, "Closing System");

	//	reset timer
	DOUBLE t_bracket = engineData.systemTimer.reset();



////////////////	SYNCHRONISE WITH PEERS

	/*
		This is important because once we've all synchronised here
		we can stop the Deliverer threads safely, knowing that no
		more PUSHDATA messages will be forthcoming.
	*/

	{ FOUT_SECTION("Synchronize")
		monitorSendOperation("Synchronize");

		comms.synchronize(fout);
	}



////////////////	TERMINATE DELIVERERS

	{ FOUT_SECTION("Terminate Deliverers")
		monitorSendOperation("Terminate Deliverers");

		comms.stopRouting(fout);
	}



////////////////	EVENT_BEGIN_TERMPHASE

	{ FOUT_SECTION("EVENT_BEGIN_TERMPHASE")

		//	**** SERIAL THREADING **** (see note higher up)

		//	monitor event
		monitorSendOperation("Sending EVENT_BEGIN_TERMPHASE...");

		//	release threads
		for (UINT32 t=0; t<workers.getThreadCount(); t++)
			workers.getThread(t)->fireCommonEvent(EVENT_BEGIN_TERMPHASE);

	}



////////////////	PREPARE REPORT

	XMLNode* nodeLogs;

	{ FOUT_SECTION("Prepare Report")

		//	monitor event
		monitorSendOperation("Preparing Report File...");

		nodeLogs = engineData.execution.prepareReportFile(
			engineData.environment.gets("ExecutionID").c_str(),
			engineData.environment.gets("VoiceID").c_str(),
			engineData.core.getVoiceIndex(),
			engineData.core.getVoiceCount()
		);

	}












////////////////	EVENT_STATE_GET

	{ FOUT_SECTION("EVENT_STATE_GET")

		//	monitor event
		monitorSendOperation("Retrieving System State...");

		//	set supplementary file path appropriately
		engineData.environment.set("SupplementaryFilePath", brahms::os::filenamepath(engineData.execution.fileSysOut), fout);

		if (engineData.execution.fileSysOut.length())
		{
			//	we currently only handle this in Solo, because in Concerto
			//	we have to serialize the bit of the system on each voice, and
			//	then send all the XML to the master voice to put into the same
			//	single system file. this is not a big deal, really, but i'm in
			//	a hurry right now so i'll just prohibit it.
			if (engineData.core.getVoiceCount() > 1)
				ferr << E_NOT_IMPLEMENTED << "saving final state is only currently supported in Solo";

			//	for each thread
			for (UINT32 t=0; t<workers.getThreadCount(); t++)
				workers.getThread(t)->fireCommonEvent(EVENT_STATE_GET);

			//	for each process
			for (UINT32 p=0; p<system.getProcessCount(); p++)
			{
				//	get process
				brahms::systemml::Process* process = system.getProcessByIndex(p);

				//	add items to node <Time> to store current process timing state
				XMLNode* nodeProcess = process->nodeProcess;
				XMLNode* nodeTime = nodeProcess->getChild("Time");
				if (!nodeTime)
				{
					nodeTime = new XMLNode("Time");
					nodeProcess->insertBefore(nodeTime, NULL);
				}

				XMLNode* nodeBaseSampleRate = nodeTime->getChildOrNull("BaseSampleRate");
				if (!nodeBaseSampleRate)
					nodeBaseSampleRate = nodeTime->appendChild(new XMLNode("BaseSampleRate"));
				nodeBaseSampleRate->nodeText((n2s(process->componentTime.baseSampleRate.num) + "/" + n2s(process->componentTime.baseSampleRate.den)).c_str());

				XMLNode* nodeSamplePeriod = nodeTime->getChildOrNull("SamplePeriod");
				if (!nodeSamplePeriod)
					nodeSamplePeriod = nodeTime->appendChild(new XMLNode("SamplePeriod"));
				nodeSamplePeriod->nodeText(n2s(process->componentTime.samplePeriod).c_str());

				XMLNode* nodeNow = nodeTime->getChildOrNull("Now");
				if (!nodeNow)
					nodeNow = nodeTime->appendChild(new XMLNode("Now"));
				nodeNow->nodeText(n2s(process->componentTime.now).c_str());

				//	get process output ports
				vector<brahms::systemml::OutputPort*> ports = process->getAllOutputPorts();
				if (ports.size())
				{
					//	ensure <Output> exists
					XMLNode* nodeOutput;
					if (nodeProcess->hasChild("Output"))
					{
						nodeOutput = nodeProcess->getChild("Output");

						//	and must be empty, in case it had content when we started
						nodeOutput->clear();
					}
					else nodeOutput = nodeProcess->appendChild(new XMLNode("Output"));

					//	for each of its output ports
					for (UINT32 p=0; p<ports.size(); p++)
					{
						//	get port
						brahms::systemml::OutputPort* port = ports[p];

						//	for each buffer
						UINT32 bufferCount = port->ring.size();
						for (UINT32 lag=1; lag<=bufferCount; lag++)
						{
							//	get buffer that was written "lag" samples ago (note
							//	that the buffer written 0 samples ago has not, in
							//	fact, been written - it's the buffer due to be written
							//	as soon as the worker thread advances further)
							brahms::systemml::Data* data = port->ring.getWriteBuffer(lag);

							//	event data
							EventStateGet esg;
							____CLEAR(esg);
							esg.precision = PRECISION_NOT_SET; // maximum

							//	fire event
							EventEx event(
								EVENT_STATE_GET,
								0,
								data,
								&esg,
								true,
								&fout
							);
							event.fire();

							//	retrieve node
							brahms::xml::XMLNode* nodeState = objectRegister.resolveXMLNode(esg.state);

							//	node must not have been named
							nodeState->nodeName("State");

							//	create Data node
							XMLNode* nodeData = new XMLNode("Data");
							nodeData->appendChild(new XMLNode("Name", (port->parentSet->getObjectName() + ">" + port->getObjectName()).c_str()));
							nodeData->appendChild(new XMLNode("Lag", n2s(lag).c_str()));
							nodeData->appendChild(nodeState);

							//	add to system node
							nodeOutput->appendChild(nodeData);
						}
					}
				}
			}
		}
		else
		{
			fout << "not writing output System File (filename not supplied)" << D_FULL;
		}

	}



////////////////	EVENT_LOG_TERM

	{ FOUT_SECTION("EVENT_LOG_TERM")

		//	monitor event
		monitorSendOperation("Retrieving Logs...");

		//	set supplementary file path appropriately
		engineData.environment.set("SupplementaryFilePath", brahms::os::filenamepath(engineData.execution.fileReport), fout);

		//	for each process
		UINT32 P = system.getProcessCount();
		for (UINT32 p=0; p<P; p++)
		{
			//	get process
			brahms::systemml::Process* process = system.getProcessByIndex(p);

			//	record time taken to terminate this process
			DOUBLE tp0 = engineData.systemTimer.elapsed();

			//	fire EVENT_LOG_TERM if process is logging
			EventLog& pevent = process->logEventData;
			if (pevent.precision != PRECISION_DO_NOT_LOG)
			{
				//	prepare event
				//Event moduleEvent = createEvent(EVENT_LOG_TERM, 0, wrappers[p]->hProcess, &pevent);
				process->logEvent.type = EVENT_LOG_TERM;
				process->logEvent.tout = &fout;

				//	have thread fire it
				workers.fireSingleEvent(process->logEvent);

				//	might have returned an XML node (or might have nothing to contribute)
				if (pevent.result)
				{
					//	retrieve node
					XMLNode* nodeLog = objectRegister.resolveXMLNode(pevent.result);

					//	name node "Log"
					if (strlen(nodeLog->nodeName()))
						ferr << E_NOT_COMPLIANT << "component \"" << process->getObjectName()
							<< "\" named XML node in EVENT_LOG_TERM (framework will name this node)";
					nodeLog->nodeName("Log");

					//	create log entry (<Process> node) in Report File <Logs> node
					XMLNode* nodeProcess = nodeLogs->appendChild(new XMLNode("Process"));
					nodeProcess->appendChild(new XMLNode("Name", process->getObjectName().c_str()));
					nodeProcess->appendChild(new XMLNode("Class", process->getClassName().c_str()));
					nodeProcess->appendChild(new XMLNode("Client", "brahms"));
					nodeProcess->appendChild(new XMLNode("Implementation", "1199"));
					nodeProcess->appendChild(new XMLNode("Release", n2s(process->getRelease()).c_str()));
					nodeProcess->appendChild(new XMLNode("Encapsulated", pevent.flags & F_ENCAPSULATED ? "1" : "0"));
					nodeProcess->appendChild(nodeLog);
				}
			}

			//	for each output
			vector<brahms::systemml::OutputPort*> ports = process->getAllOutputPorts();
			for (UINT32 o=0; o<ports.size(); o++)
			{
				//	port
				brahms::systemml::OutputPort* port = ports[o];

				//	terminate data, and optionally add an output to output array
				//	this call is only made if if the object is storing (see brahms-component.h)
				if (port->logEventData.precision != PRECISION_DO_NOT_LOG) // this field marks that this object is storing (see EVENT_LOG_INIT)
				{
					//	set stuff: leave node obviously. also, leave steps since data
					//	object might want to use that. zero src to hide that since it's
					//	irrelevant at this stage, and set precision. flags are still
					//	set from initialisation.
					port->logEvent.type = EVENT_LOG_TERM;
					port->logEventData.source = NULL;
					port->logEvent.tout = &fout;

					//	fire event from caller thread
					port->logEvent.fire();

					//	should have returned an XML node
					if (!port->logEventData.result)
						ferr << E_NOT_COMPLIANT << "data component \"" << port->getObjectName() << "\" did not create an XML node during EVENT_LOG_TERM";

					//	retrieve node
					XMLNode* nodeLog = objectRegister.resolveXMLNode(port->logEventData.result);

					//	name node "Log"
					if (strlen(nodeLog->nodeName()))
						ferr << E_NOT_COMPLIANT << "component \"" << port->getObjectName()
							<< "\" named XML node in EVENT_LOG_TERM (framework will name this node)";
					nodeLog->nodeName("Log");

					//	create log entry (<Data> node) in Report File <Logs> node
					brahms::systemml::Data* data = port->getZerothData();
					XMLNode* nodeData = nodeLogs->appendChild(new XMLNode("Data"));

					//	parentSet should not be NULL
					if (!port->parentSet) ferr << E_INTERNAL;

					string name = process->getObjectName() + ">>" + port->parentSet->getObjectName() + ">" + port->getObjectName();
					nodeData->appendChild(new XMLNode("Name", name.c_str()));
					nodeData->appendChild(new XMLNode("Class", data->getClassName().c_str()));
					nodeData->appendChild(new XMLNode("Client", "brahms"));
					nodeData->appendChild(new XMLNode("Implementation", "1199"));
					nodeData->appendChild(new XMLNode("Release", n2s(data->getRelease()).c_str()));
					nodeData->appendChild(new XMLNode("Encapsulated", port->logEventData.flags & F_ENCAPSULATED ? "1" : "0"));
					nodeData->appendChild(nodeLog);
				}
			}

			//	record time taken to terminate this process
			process->irtWallclock.term += (engineData.systemTimer.elapsed() - tp0);

			//	monitor
			DOUBLE pr = ((DOUBLE)p+1)/((DOUBLE)P);
			monitorSendProgress(pr, pr);
		}
	}



////////////////	TERMPHASE_PAUSE

	if (engineData.core.createEngine.engineFlags & F_PAUSE_ON_EXIT)
	{
		fout << "Executing Pause..." << D_VERB;

		//	monitor event
		monitorSendOperation("Executing Pause...");

		brahms::os::msgbox(
			"PAUSE BEFORE EXIT WORKER THREADS\n\n"
			"You requested a pause before exiting worker threads (this is it). This gives\n"
			"you a chance to check the state of any attached engines that will close when\n"
			"the threads terminate. Press OK when you are done."
			);
	}



////////////////	CREATE REPORT NODE

	{ FOUT_SECTION("Construct Report")

		//	monitor event
		monitorSendOperation("Finalising Report File...");

		XMLNode* nodeEnvironment = engineData.execution.nodeReport.getChild("Environment");
		XMLNode* nodeClient = nodeEnvironment->getChild("Client");



////////////////	CREATE ENVIRONMENT NODE

		XMLNode* node = NULL;
		stringstream ss;

		//	time
		node = nodeEnvironment->appendChild(new XMLNode("Time"));
		string t = brahms::os::getuctime();
		node->appendChild(new XMLNode("Format", "UTC YYYY-MM-DD hh:mm:ss:xxx"));
		node->appendChild(new XMLNode("Open", t_openDocument.c_str()));
		node->appendChild(new XMLNode("Close", t.c_str()));

		//	os
		node = nodeEnvironment->appendChild(new XMLNode("OS"));
#ifdef __GLN__
		node->appendChild(new XMLNode("Type", "GLN"));
#endif
#ifdef __WIN__
		node->appendChild(new XMLNode("Type", "WIN"));
		OSVERSIONINFOEX version;
		version.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
		if (GetVersionEx((LPOSVERSIONINFO)&version))
		{
			ss << version.dwMajorVersion << "." << version.dwMinorVersion << "." << version.dwBuildNumber << ", SP" << version.wServicePackMajor << "." << version.wServicePackMinor << " (PlatformID " << version.dwPlatformId << ")";
			string s = ss.str();
			ss.str("");
			node->appendChild(new XMLNode("Version", s.c_str()));
		}
		else node->appendChild(new XMLNode("Version", ""));
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		switch(info.wProcessorArchitecture)
		{
			case PROCESSOR_ARCHITECTURE_AMD64: ss << "x64"; break;
			case PROCESSOR_ARCHITECTURE_IA64: ss << "IA64"; break;
			case PROCESSOR_ARCHITECTURE_INTEL: ss << "x86"; break;
			case PROCESSOR_ARCHITECTURE_UNKNOWN: ss << "Unknown"; break;
		}
		string s = ss.str();
		ss.str("");
		node->appendChild(new XMLNode("Architecture", s.c_str()));
#endif
#ifdef __OSX__
		node->appendChild(new XMLNode("Type", "OSX"));
#endif

#ifdef __WIN__
		//	platform
		node = nodeEnvironment->appendChild(new XMLNode("Platform"));
		ss << info.dwNumberOfProcessors;
		s = ss.str();
		ss.str("");
		node->appendChild(new XMLNode("ProcessorCount", s.c_str()));
#endif

		//	configuration
		/*XMLNode* nodeReportConfiguration = nodeClient->appendChild(new XMLNode("Configuration"));*/

		//	components
		/*XMLNode* nodeReportComponents = nodeClient->appendChild(new XMLNode("Components"));*/

		//	execution parameters
		/*XMLNode* nodeReportExecPars = */nodeClient->appendChild(new XMLNode(engineData.environment.nodeExecPars));



////////////////	CREATE PERFORMANCE NODE

		XMLNode* nodePerformance = nodeClient->appendChild(new XMLNode("Performance"));
		XMLNode* nodeTiming = nodePerformance->appendChild(new XMLNode("Timing"));
		nodeTiming->appendChild(new XMLNode("TimeRunPhase", engineData.environment.gets("TimeRunPhase").c_str()));



////////////////	ADD TIMING TO OUTPUT
			////	ALL TIMES ARE WRITTEN AT %.9f (nearest nanosecond)

		//	for each thread
		for (UINT32 t=0; t<workers.getThreadCount(); t++)
		{
			//	open "Thread" section
			XMLNode* nodeThread = nodeTiming->appendChild(new XMLNode("Thread"));

			//	fout thread index
			/*XMLNode* nodeIndex = */nodeThread->appendChild(new XMLNode("Index", n2s(t).c_str()));

			//	fout thread timing
			brahms::time::TIME_IRT_CPU irtcpu = workers.getCPUTime(t);
			string tdata = n2s(irtcpu.init.user) + " " + n2s(irtcpu.init.kernel) + " "
				+ n2s(irtcpu.run.user) + " " + n2s(irtcpu.run.kernel) + " "
				+ n2s(irtcpu.term.user) + " " + n2s(irtcpu.term.kernel);

			/*XMLNode* nodeIRTCPU = */nodeThread->appendChild(new XMLNode("IRTCPU", tdata.c_str()));

			//	for each wrapped process
			vector<brahms::systemml::Process*> processes = workers.getProcesses(t);
			for (UINT32 p=0; p<processes.size(); p++)
			{
				brahms::systemml::Process* process = processes[p];

				XMLNode* nodeProcess = nodeThread->appendChild(new XMLNode("Process"));
				nodeProcess->appendChild(new XMLNode("Name", process->getObjectName().c_str()));

				brahms::time::TIME_IRT* irt = &process->irtWallclock;
				string tdata = n2s(irt->init) + " " + n2s(irt->run) + " " + n2s(irt->term);

				/*XMLNode* nodeIRT = */nodeProcess->appendChild(new XMLNode("IRT", tdata.c_str()));
			}
		}

		//	open "Caller" section
		XMLNode* nodeCaller = nodeTiming->appendChild(new XMLNode("Caller"));

		//	for caller thread
		irtCPU.term = brahms::thread::getthreadtimes();

		//	finalize caller CPU timing
		irtCPU.finalize();
		brahms::time::TIME_IRT_CPU* irt = &irtCPU;
		string tdata = n2s(irt->init.user) + " " + n2s(irt->init.kernel) + " "
			+ n2s(irt->run.user) + " " + n2s(irt->run.kernel) + " "
			+ n2s(irt->term.user) + " " + n2s(irt->term.kernel);

		/*XMLNode* nodeIRTCPU = */nodeCaller->appendChild(new XMLNode("IRTCPU", tdata.c_str()));

		//	read bracket timer
		irtWallclock.term += engineData.systemTimer.elapsed();
		tdata = n2s(irtWallclock.init) + " " + n2s(irtWallclock.run) + " " + n2s(irtWallclock.term);

		/*XMLNode* nodeIRT = */nodeCaller->appendChild(new XMLNode("IRT", tdata.c_str()));

		//	add resolution information
		double tps = engineData.systemTimer.getTicksPerSec();
		bool rdtsc = false;
		if (tps < 0.0)
		{
			rdtsc = true;
			tps = -tps;
		}

		/*XMLNode* nodeTPS = */nodeTiming->appendChild(new XMLNode("TicksPerSec", n2s(tps).c_str()));
		/*XMLNode* nodeRDTSC = */nodeTiming->appendChild(new XMLNode("RDTSC", n2s(rdtsc ? 1 : 0).c_str()));

	}



////////////////	NOTES

	/*

		Calling system.terminate() deletes all of the objects in the system. We can't do
		this until all threads that may operate on these objects are no longer going to
		do so. This means all the W threads, of course, but also all S threads which may
		make callbacks following delivery of PUSHDATA messages. Obviously, we start by
		terminating the W threads. But then, we don't want to close comms yet, so instead
		we have each channel flush its send message queue at this point.

	*/

////////////////	TERMINATE WORKERS

	{ FOUT_SECTION("Terminate Workers")
		monitorSendOperation("Terminate Workers");

		workers.terminate(fout);
	}



////////////////	FLUSH COMMS (see NOTES above)

	comms.flush(fout);



////////////////	TERMINATE SYSTEM

	{ FOUT_SECTION("Terminate System")
		monitorSendOperation("Terminate System");

		system.terminate(fout);
	}



////////////////	WRITE OUTPUT FILES

	{ FOUT_SECTION("Write Files")

		//	see if we should serialize neatly
		UINT32 indent = engineData.environment.getb("TidyXML") ? 1 : 0;

		//	monitor event
		monitorSendOperation("Writing Output Files...");

		//	reset timer
		t_bracket = engineData.systemTimer.elapsed();

		//	write System File
		if (engineData.execution.fileSysOut.length())
		{
			//	write system state file
			fout << "writing system file..." << D_VERB;
			ofstream fileSysOut(engineData.execution.fileSysOut.c_str());
			if (!fileSysOut) ferr << E_OS << "error opening system out file \"" << engineData.execution.fileSysOut << "\"";
			fileSysOut << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n";
			system.nodeSystem.serialize(fileSysOut, indent);
			fileSysOut.close();
		}
		else fout << "not writing system file (not requested)" << D_VERB;

		//	write Report File
		fout << "writing report file..." << D_FULL;
		ofstream fileReport(engineData.execution.fileReport.c_str());
		if (!fileReport) ferr << E_OS << "error opening report file \"" << engineData.execution.fileReport << "\"";
		fileReport << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n";
		engineData.execution.nodeReport.serialize(fileReport, indent);
		fileReport.close();

		//	monitor event
		monitorSendOperation("All files written successfully");

	}



////////////////	FINISHED

	fout << (engineData.systemTimer.elapsed() - t_bracket) << " seconds of untimed operation whilst writing ExecutionML file" << D_VERB;

}
