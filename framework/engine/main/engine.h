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

	$Id:: engine.h 2382 2009-11-16 18:12:18Z benjmitch         $
	$Rev:: 2382                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-16 18:12:18 +0000 (Mon, 16 Nov 2009)       $
________________________________________________________________

*/



#ifndef INCLUDED_ENGINE_ENGINE
#define INCLUDED_ENGINE_ENGINE

#include "support/register.h"
#include "base/brahms_time.h"
#include "systemml/system.h"
#include "systemml/thread.h"
#include "enginedata.h"

#undef fout
#define fout (engineData.core.caller.tout)

namespace brahms
{
	class Engine : public RegisteredObject
	{
	public:
		Engine(CreateEngine* createEngine);
		~Engine();

		//	bring up and down
		void up();
		bool down(UINT32* messageCount);

		//	non-document operations
		void walksub(WalkLevel level, const string& path, string subpath, string indent);
		void walk(WalkLevel level);

		//	document operations
		void open();
		void execute();
		void close();

	private:
		//	send monitor event
		MonitorEvent monitorEvent;
		Symbol monitorSendEvent(Symbol eventType);
		Symbol monitorSendPhase(ExecPhase phase, string msg);
		Symbol monitorSendOperation(string msg, bool initPhase = false);
		Symbol monitorSendProgress(DOUBLE progressMin, DOUBLE progressMax);

		//	engine data
		brahms::time::TIME_IRT irtWallclock;
		brahms::time::TIME_IRT_CPU irtCPU;
		string t_openDocument;

	public:

		//	data shared amongst components
		brahms::EngineData engineData;

	private:

		//	engine components
		brahms::thread::Workers workers;
		brahms::systemml::System system;
		brahms::comms::Comms comms;

	};

	class LogSection
	{
	public:
		LogSection(brahms::output::Sink& p_sink, const string& key)
			: sink(p_sink)
		{
			sink.section(key);
		}

		~LogSection()
		{
			sink.section("");
		}

	private:
		brahms::output::Sink& sink;
	};

	#define FOUT_SECTION(key) LogSection fout_section(engineData.core.sink, key);
}

#endif // INCLUDED_ENGINE_ENGINE
