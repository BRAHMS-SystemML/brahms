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

	$Id:: hang.cpp 2378 2009-11-16 00:47:59Z benjmitch         $
	$Rev:: 2378                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-16 00:47:59 +0000 (Mon, 16 Nov 2009)       $
________________________________________________________________

*/



////////////////	COMPONENT INFO

//	component information
#define COMPONENT_CLASS_STRING "client/brahms/test/hang"
#define COMPONENT_CLASS_CPP client_brahms_test_hang_0
#define COMPONENT_FLAGS F_NOT_RATE_CHANGER

#include <unistd.h>

//	include common header
#include "../process.h"



////////////////	COMPONENT CLASS (DERIVES FROM Process)

class COMPONENT_CLASS_CPP : public Process
{

public:

	//	use ctor/dtor if required
	COMPONENT_CLASS_CPP();

	//	framework event function
	Symbol event(Event* event);

private:

	/*

		Private member data of the process goes here.
		You can use this to store your parameters
		and process state between calls to event().

	*/

	//	my state variables
	Symbol triggerEvent;
	bool sendSignals;

};






////////////////	OS-SPECIFIC


#ifdef __WIN__

	#define _WIN32_IE 0x0500
	#define _WIN32_WINNT 0x0500
	#define WIN32_LEAN_AND_MEAN
	#include "windows.h"

	#define os_sleep Sleep

#endif

#ifdef __NIX__

	#define os_sleep(__milliseconds) usleep((__milliseconds) * 1000)

#endif



////////////////	OPERATION METHODS

COMPONENT_CLASS_CPP::COMPONENT_CLASS_CPP()
{
	triggerEvent = S_NULL;
	sendSignals = false;
}

Symbol COMPONENT_CLASS_CPP::event(Event* event)
{
	bout << "hang received event " << getSymbolString(event->type) << " (" << hex << event->type << " == " << triggerEvent << dec << ")" << D_VERB;

	//	except
	if (triggerEvent == event->type)
	{
		bout << "received trigger event " << getSymbolString(event->type) << " (hanging now...)" << D_INFO;
		
		//	hang loop
		while(true)
		{
			//	sleep
			os_sleep(100);

			//	signal
			if (sendSignals) stillActive();
		}
	}

	switch(event->type)
	{
		case EVENT_STATE_SET:
		{
			//	extract DataML
			EventStateSet* data = (EventStateSet*) event->data;
			XMLNode xmlNode(data->state);
			DataMLNode nodePars(&xmlNode);

			//	extract fields
			string event = nodePars.getField("event").set(F_STRICT).getSTRING();
			sendSignals = nodePars.getField("signals").getBOOL();

			//	event
			triggerEvent = S_NULL;
			if (event == "EVENT_RUN_SERVICE") triggerEvent = EVENT_RUN_SERVICE;
			if (event == "EVENT_RUN_RESUME") triggerEvent = EVENT_RUN_RESUME;
			if (event == "EVENT_RUN_PLAY") triggerEvent = EVENT_RUN_PLAY;
			if (event == "") triggerEvent = C_OK; // don't throw
			if (triggerEvent == S_NULL) berr << "unrecognised event";

			//	report
			if (sendSignals) bout << "hang sending signals" << D_VERB;
			else bout << "hang not sending signals" << D_VERB;

			//	ok
			return C_OK;
		}

		case EVENT_INIT_CONNECT:
		{
			//	ok
			return C_OK;
		}

		case EVENT_RUN_SERVICE:
		{
			//	ok
			return C_OK;
		}
	}

	//	not serviced
	return S_NULL;
}


#include "brahms-1199.h"
