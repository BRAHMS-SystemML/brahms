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

	$Id:: enginedata.h 2410 2009-11-20 10:18:18Z benjmitch     $
	$Rev:: 2410                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-20 10:18:18 +0000 (Fri, 20 Nov 2009)       $
________________________________________________________________

*/



#ifndef INCLUDED_ENGINE_ENGINEDATA
#define INCLUDED_ENGINE_ENGINEDATA

#ifndef BRAHMS_BUILDING_ENGINE
#define BRAHMS_BUILDING_ENGINE
#endif
#include "brahms-client.h"
#include "base/os.h"
#include "support/execution.h"
#include "support/loader.h"
#include "support/environment.h"
#include "base/ipm.h"
#include "base/core.h"

////////////////	NAMESPACE

namespace brahms
{



////////////////	ENGINE GLOBAL DATA

	/*

		This is the engine-global data, that is shared by all parts
		of the engine.

	*/

	struct EngineData
	{
		EngineData(const CreateEngine* p_createEngine)
			:
			environment(core),
			pool("EngineData")
		{
			//	copy create engine
			this->core.createEngine = *p_createEngine;

			flag_allowCreatePort = false;

			systemInfo.engineVersion = VERSION_ENGINE;
		}

		//	flags
		bool flag_allowCreatePort;

		//	engine-global system information object
		ExecutionInfo systemInfo;

		//	timer - NOTE this is for the exclusive use
		//	of the caller thread (the thread making
		//	calls on the engine)
		brahms::os::Timer systemTimer;

		//	objects
		brahms::Execution execution;
		brahms::Loader loader;
		brahms::Environment environment;

		//	IPM pool
		brahms::base::IPMPool pool;

		//	core
		brahms::base::Core core;

		//	log filename
		string logFilename;

		//	exec pars for component
		brahms::xml::XMLNode execParsComponent;
	};




	namespace comms
	{

	////////////////	COMMS LAYER

		class Comms
		{

		public:
			Comms(brahms::EngineData& engineData);
			~Comms();

			//	up and down
			void init();
			void term();

			//	structure
			void addRoutingEntry(UINT32 remoteVoiceIndex, UINT32 msgStreamID, brahms::channel::PushDataHandler pushDataHandler, void* pushDataHandlerArgument);
			void stopRouting(brahms::output::Source& fout);

			//	push
			void push(brahms::base::IPM* ipm, brahms::output::Source& tout);

			//	pull (if next due message doesn't match tag, an error is raised)
			Symbol pull(UINT32 remoteVoiceIndex, brahms::base::IPM*& ipm, UINT8 tag, brahms::output::Source& tout, UINT32 timeout, bool throwOnTimeout);

			//	miscellaneous
			void flush(brahms::output::Source& tout);
			string getInfo();
			void signalPeers(UINT8 signal);
			void synchronize(brahms::output::Source& tout);


		////	STATE

			brahms::EngineData& engineData;
			vector<brahms::channel::Channel*> channels; // one channel to each other Voice (and a NULL in there for the self-referential channel)

			bool signalledPeers;
			brahms::os::Mutex signalledPeersMutex;

		};



////////////////	NAMESPACE

	}
}



////////////////	INCLUSION GUARD

#endif
