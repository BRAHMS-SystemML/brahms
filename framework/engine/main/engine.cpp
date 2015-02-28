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

	$Id:: engine.cpp 2411 2009-11-20 12:09:27Z benjmitch       $
	$Rev:: 2411                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-20 12:09:27 +0000 (Fri, 20 Nov 2009)       $
________________________________________________________________

*/



#include "systemml/systemml.h"
#include "engine.h"
#include "comm.cpp"

using namespace brahms::output;
using namespace brahms::xml;
using namespace brahms::text;
using namespace brahms::math;

//#define FOUT_SECTION(key) Section fout_section(fout, key);



////////////////	NAMESPACE

namespace brahms
{



////////////////////////////////////////////////////////////////
////////////////
////////////////	CTOR & DTOR
////////////////
////////////////////////////////////////////////////////////////

	UINT32 engineCount = 0;

	void assertsz(UINT32 sz, UINT32 esz, string msg)
	{
		if (sz != esz)
			ferr << E_INTERNAL << "failed assert during engine init: sizeof(" << msg << ") == " << sz << " != " << esz;
	}

	Engine::Engine(CreateEngine* p_createEngine) :
		RegisteredObject(CT_ENGINE, "engine"),
		engineData(p_createEngine),
		workers(engineData),
		system(engineData, comms),
		comms(engineData)
	{
		//	get MPI rank (voice index) if necessary
		if (engineData.core.createEngine.voiceIndex == VOICE_FROM_MPI)
		{
			//	init MPI module
			brahms::channel::CommsInitFunc* commsInitFunc = NULL;
			brahms::channel::CreateChannelFunc* createChannelFunc = NULL;
			brahms::channel::CommsInitData commsInitData = comms::initChannelModule(engineData, brahms::channel::PROTOCOL_MPI, commsInitFunc, createChannelFunc);

			//	get voice from MPI rank
			engineData.core.createEngine.voiceIndex = commsInitData.voiceIndex;

			//	return it too
			p_createEngine->voiceIndex = engineData.core.createEngine.voiceIndex;
		}

		//	copy log filename (replace ((VOICE)) if present)
		if (engineData.core.createEngine.logFilename)
		{
			engineData.logFilename = engineData.core.createEngine.logFilename;
			size_t f = engineData.logFilename.find("((VOICE))");
			if (f != string::npos)
			{
				if (engineData.core.createEngine.voiceIndex == VOICE_UNDEFINED)
					ferr << E_VOICE_UNDEFINED << "voice undefined when first required";
				grep(engineData.logFilename, "((VOICE))", n2s(engineData.core.createEngine.voiceIndex + 1));
			}

			engineData.core.createEngine.logFilename = engineData.logFilename.c_str(); // point at local string, now
		}


		/*

			THIS DOESN'T WORK YET, because we haven't read the exe file, so we don't know the count is 1.
			instead, this is inferred later in finalize();

		//	infer voice 1 if voice count is 1 and voice index is undefined
		if (engineData.core.createEngine.voiceIndex == VOICE_UNDEFINED && engineData.core.createEngine.voiceCount == 1)
		{
			//	infer
			engineData.core.createEngine.voiceIndex = 0;

			//	return it too
			p_createEngine->voiceIndex = engineData.core.createEngine.voiceIndex;
		}

		*/

		//	increment engine count, if completed successfully
		engineCount++;
	}

	Engine::~Engine()
	{
		engineCount--;
	}






////////////////////////////////////////////////////////////////
////////////////
////////////////	UP
////////////////
////////////////////////////////////////////////////////////////

	void Engine::up()
	{
		assertsz(sizeof(UINT8), 1, "UINT8");
		assertsz(sizeof(UINT16), 2, "UINT16");
		assertsz(sizeof(UINT32), 4, "UINT32");
		assertsz(sizeof(UINT64), 8, "UINT64");
		assertsz(sizeof(INT8), 1, "INT8");
		assertsz(sizeof(INT16), 2, "INT16");
		assertsz(sizeof(INT32), 4, "INT32");
		assertsz(sizeof(INT64), 8, "INT64");
		assertsz(sizeof(UINTA), ARCH_BYTES, "UINTA");

		//	init sink
		engineData.core.sink.init(engineData.core.createEngine);

		//	create monitor event
		monitorEvent.type = S_NULL;
		monitorEvent.createEngine = &engineData.core.createEngine;
		monitorEvent.phase = EP_NULL;
		monitorEvent.message = NULL;
		monitorEvent.monitorTime = &system.systemTime;
		monitorEvent.progressMin = 0.0;
		monitorEvent.progressMax = 0.0;

		//	connect to sink
		engineData.core.caller.tout.connect(&engineData.core.sink, engineData.core.caller.getThreadIdentifier());

		{ FOUT_SECTION("Engine::up()")

			{ FOUT_SECTION("Prepare Environment")

				//	load install/machine/user-level environment
				{ FOUT_SECTION("Install-Level")
					engineData.environment.load(brahms::os::getenv("SYSTEMML_INSTALL_PATH") + "/BRAHMS/brahms.xml", true, fout);
				}
				{ FOUT_SECTION("Machine-Level")
					engineData.environment.load(brahms::os::getspecialfolder("appdata.machine") + "/brahms.xml", false, fout);
				}
				{ FOUT_SECTION("User-Level")
					engineData.environment.load(brahms::os::getspecialfolder("appdata.user") + "/brahms.xml", false, fout);
				}

				//	always have this, but will only be set to something non-empty if we're running an execution
				engineData.environment.add("SupplementaryFilePath");

				//	load execution file if there is one (we need the environment as early as possible)
				if (engineData.core.createEngine.executionFilename)
				{
					FOUT_SECTION("Execution-Level")

					if (!brahms::os::fileexists(engineData.core.createEngine.executionFilename))
						ferr << E_INVOCATION << "Execution File \"" << engineData.core.createEngine.executionFilename << "\" not found";

					//	load
					VoiceCount voiceCount = engineData.core.getVoiceCount();
					VoiceIndex voiceIndex = engineData.core.getVoiceIndex();
					engineData.execution.load(engineData.core.createEngine.executionFilename,
						&voiceCount, &voiceIndex, fout);
					engineData.core.setVoiceCount(voiceCount); //	TODO: wtf? might be changed by call to execution.load() but it's a bit weird, this
					engineData.core.setVoiceIndex(voiceIndex); //	TODO: wtf?

					//	set SupplementaryFilePath to System File (in) path to start with
					//	this will be changed later during log collection to the appropriate
					//	path (either System File (out) path during EVENT_STATE_GET or
					//	Report File path during EVENT_LOG_TERM)
					engineData.environment.set("SupplementaryFilePath", brahms::os::filenamepath(engineData.execution.fileSysIn), fout);

					//	load execution-level parameters
					engineData.environment.load(engineData.core.createEngine.executionFilename,
						engineData.execution.nodeExecution, fout);
				}

				//	invocation-level environment
				{
					FOUT_SECTION("Invocation-Level")

					if (engineData.core.createEngine.executionParameters)
					{
						//	are stored as "X=Y;X=Y;\0"
						const char* p = engineData.core.createEngine.executionParameters;

						//	dropout when NULL is reached
						while(*p)
						{
							//	p points at "X"
							const char* q = p; // point q at "X"
							while((*q) && (*q != ';')) q++; // point q at ";"
							if (!(*q)) ferr << E_INVOCATION << "malformed execution parameter string"; // check to avoid seg fault if wrong data supplied
							const char* e = p; // point e at "X"
							while((*e) && (*e != '=')) e++; // point e at "="
							if ((e==p) || (e>=(q-1))) ferr << E_INVOCATION << "malformed execution parameter string"; // check to avoid seg fault if wrong data supplied
							string key, val;
							key.assign(p, e-p);
							val.assign(e+1, q-e-1);
							engineData.environment.set(key, val, fout);
							p = q + 1; // point p at next "X" or at NULL
						}
					}
					else fout << "(none)" << D_VERB;
				}

				//	finalize environment
				{ FOUT_SECTION("Finalize Environment")

					engineData.environment.finalize(engineData.core.getVoiceCount(), engineData.core.getVoiceIndex(), fout);

					//	tell anyone that needs to know
					engineData.loader.init(engineData.environment);

				}

				//	store exec pars in ExecutionInfo for components
				engineData.systemInfo.executionParameters = engineData.execParsComponent.getObjectHandle();
				engineData.execParsComponent.nodeName("ComponentExecutionParameters");
				engineData.execParsComponent.appendChild(new XMLNode("SupplementaryFilePath", engineData.environment.gets("SupplementaryFilePath").c_str()));
				engineData.execParsComponent.appendChild(new XMLNode("VoiceID", engineData.environment.gets("VoiceID").c_str()));
				engineData.execParsComponent.appendChild(new XMLNode("WorkingDirectory", engineData.environment.gets("WorkingDirectory").c_str()));

				//engineData.WorkingDirectory = engineData.environment.gets("WorkingDirectory");
				//engineData.SupplementaryFilePath = engineData.environment.gets("SupplementaryFilePath");
				//engineData.systemInfo.SupplementaryFilePath = engineData.SupplementaryFilePath.c_str();
				//engineData.VoiceID = engineData.environment.gets("VoiceID");
				//engineData.systemInfo.VoiceID = engineData.VoiceID.c_str();

				//	parse BufferingPolicy
				string bp = engineData.environment.gets("BufferingPolicy");
				if (bp == "OnlyDisk") engineData.systemInfo.BufferingPolicy = C_BUFFERING_ONLY_DISK;
				else if (bp == "FavourDisk") engineData.systemInfo.BufferingPolicy = C_BUFFERING_FAVOUR_DISK;
				else if (bp == "Balanced") engineData.systemInfo.BufferingPolicy = C_BUFFERING_BALANCED;
				else if (bp == "FavourMemory") engineData.systemInfo.BufferingPolicy = C_BUFFERING_FAVOUR_MEMORY;
				else if (bp == "OnlyMemory") engineData.systemInfo.BufferingPolicy = C_BUFFERING_ONLY_MEMORY;
				else ferr << "invalid value of BufferingPolicy, \"" << bp << "\"";

			}

			{ FOUT_SECTION("Initialise Monitor")
				bool ShowGUI = engineData.environment.getb("ShowGUI");
				fout << "opening monitor (ShowGUI == " << ShowGUI << ")" << D_VERB;
				monitorSendEvent(EVENT_MONITOR_OPEN);
				if (ShowGUI) monitorSendEvent(EVENT_MONITOR_SHOW);
				fout << "monitor up" << D_VERB;
			}

			{ FOUT_SECTION("Initialise Communications")
				monitorSendOperation("Initialise Communications", true);

				comms.init();
			}

			//	report
			FrameworkVersion v = VERSION_ENGINE;
			fout
				<< "Engine is up! (version " << toString(v) <<  ", voice " << unitIndex(engineData.core.getVoiceIndex()) << "/" << engineData.core.getVoiceCount()
				<< ", on " << engineData.environment.gets("HostName") << ")" << D_VERB;

		}
	}






////////////////////////////////////////////////////////////////
////////////////
////////////////	DOWN
////////////////
////////////////////////////////////////////////////////////////

	bool Engine::down(UINT32* messageCount)
	{
		/*

			Each of these functions should catch its own errors, but to make
			sure we get *some* information back if an error gets thrown (at
			least, who threw it), we wrap the whole of down() in a try/catch.

		*/
		try
		{

			{ FOUT_SECTION("Engine::down()")

				{ FOUT_SECTION("Terminate Workers (Catch)")
					//	in case workers were not already terminated, we terminate them again here
					//	this can happen if an error occurs during open() or execute(), in which
					//	case close() is not called. TODO in the long term, a more formal approach
					//	will be preferred, here, where the "document", if open, is closed when
					//	we go down().
					monitorSendOperation("Terminate Workers");
					workers.terminate(fout);
				}

				{ FOUT_SECTION("Terminate Communications")
					if (engineData.core.condition.get(brahms::base::COND_LOCAL_ERROR))
						comms.signalPeers(brahms::base::IPMTAG_ERROR);

					monitorSendOperation("Terminate Communications");
					comms.term();
				}

				//	NB! Must terminate loader *last*, since other parts of the system
				//	use loaded modules (components, channels) and they need to not
				//	be unloaded until we're finished with them (else segfaults...)
				{ FOUT_SECTION("Terminate Loader")
					monitorSendOperation("Terminate Loader");
					engineData.loader.terminate(engineData.core.caller);
				}

				//	callback event
				{ FOUT_SECTION("Terminate Monitor")
					monitorSendOperation("Terminate Monitor");
					monitorSendEvent(EVENT_MONITOR_CLOSE);
				}

				fout << "Engine is down!" << D_VERB;
			}

			//	disconnect system source object
			fout << "disconnecting log..." << D_VERB;
			fout.disconnect();

			//	if there is one E_THREAD_ERROR, and at least one other error, we can remove that
			//	one, because it carries no additional information (it's just the way we use to
			//	bring the caller thread down hard when something goes wrong elsewhere)
			if (!(engineData.core.createEngine.engineFlags & F_MAXERR))
				engineData.core.errorStack.clearSpurious();

			//	terminate sink
			DataBurst db;
			db.voiceIndex = engineData.core.createEngine.voiceIndex;
			db.voiceCount = engineData.core.createEngine.voiceCount;
			db.workerCount = workers.getThreadCount();
			UINT32 messageCountTemp = engineData.core.sink.term(db, engineData.core.errorStack.getErrors());
			if (messageCount) *messageCount = messageCountTemp;

			//	return true if no error, else return false (errors will be detailed in log)
			return engineData.core.errorStack.getErrors().size() == 0;
		}

		catch(brahms::error::Error e)
		{
			____WARN("UNCAUGHT EXCEPTION IN " << __FUNCTION__ << ":\n" << e.format(brahms::FMT_TEXT, true));
		}

		catch(std::exception se)
		{
			brahms::error::Error e(E_STD, se.what());
			____WARN("UNCAUGHT EXCEPTION IN " << __FUNCTION__ << ":\n" << e.format(brahms::FMT_TEXT, true));
		}

		catch(...)
		{
			brahms::error::Error e(E_UNRECOGNISED_EXCEPTION);
			____WARN("UNCAUGHT EXCEPTION IN " << __FUNCTION__ << ":\n" << e.format(brahms::FMT_TEXT, true));
		}

		return false;
	}






#include "engine-open.cpp"
#include "engine-execute.cpp"
#include "engine-close.cpp"

#include "engine-walk.cpp"
#include "engine-monitor.cpp"




////////////////	ENGINE DESTRUCTION WARNING

	class EngineCheck
	{

	public:

		EngineCheck()
		{
		}

		~EngineCheck()
		{
			if (engineCount)
				____WARN("engine count == " << engineCount << " at engine unload");
		}

	}
	engineCheck;



////////////////	NAMESPACE

}


#include "api.cpp"
