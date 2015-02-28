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

	$Id:: engine-monitor.cpp 2282 2009-11-01 21:37:45Z benjmit#$
	$Rev:: 2282                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-01 21:37:45 +0000 (Sun, 01 Nov 2009)       $
________________________________________________________________

*/

#include "brahms-client.h"
#include "main/engine.h"

using brahms::Symbol;
using brahms::Engine;

Symbol Engine::monitorSendEvent(Symbol eventType)
{
	//	do nothing if no handler set
	if (!engineData.core.createEngine.handler) return S_NULL;

	//	update event data
	system.systemTime.wallclockTime = engineData.systemTimer.elapsed();

	//	construct and send event
	monitorEvent.type = eventType;
	return engineData.core.createEngine.handler(&monitorEvent);
}

Symbol Engine::monitorSendPhase(ExecPhase phase, string msg)
{
	//	do nothing if no handler set
	if (!engineData.core.createEngine.handler) return S_NULL;

	//	update event data
	system.systemTime.wallclockTime = engineData.systemTimer.elapsed();

	//	construct and send event
	monitorEvent.type = EVENT_MONITOR_PHASE;
	monitorEvent.phase = phase;
	monitorEvent.message = msg.c_str();
	Symbol ret = engineData.core.createEngine.handler(&monitorEvent);
	monitorEvent.message = NULL;
	return ret;
}

Symbol Engine::monitorSendOperation(string msg, bool initPhase)
{
	//	do nothing if no handler set
	if (!engineData.core.createEngine.handler) return S_NULL;

	//	update event data
	system.systemTime.wallclockTime = engineData.systemTimer.elapsed();

	//	construct and send event
	monitorEvent.type = EVENT_MONITOR_OPERATION;
	monitorEvent.message = msg.c_str();
	Symbol ret = engineData.core.createEngine.handler(&monitorEvent);
	monitorEvent.message = NULL;

	//	handle cancel during init by throwing an error
	if (initPhase && ret == C_CANCEL)
		ferr << E_ERROR << "execution cancelled before run-phase (by user)";

	//	ok
	return ret;
}

Symbol Engine::monitorSendProgress(DOUBLE progressMin, DOUBLE progressMax)
{
	//	do nothing if no handler set
	if (!engineData.core.createEngine.handler) return S_NULL;

	//	update event data
	system.systemTime.wallclockTime = engineData.systemTimer.elapsed();

	//	construct and send event
	monitorEvent.type = EVENT_MONITOR_PROGRESS;
	monitorEvent.progressMin = progressMin;
	monitorEvent.progressMax = progressMax;
	Symbol ret = engineData.core.createEngine.handler(&monitorEvent);
	return ret;
}
