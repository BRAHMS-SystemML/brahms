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

	$Id:: except.cpp 2449 2010-01-25 16:02:52Z benjmitch       $
	$Rev:: 2449                                                $
	$Author:: benjmitch                                        $
	$Date:: 2010-01-25 16:02:52 +0000 (Mon, 25 Jan 2010)       $
________________________________________________________________

*/



////////////////	COMPONENT INFO

//	component information
#define COMPONENT_CLASS_STRING "client/brahms/test/except"
#define COMPONENT_CLASS_CPP client_brahms_test_except_0
#define COMPONENT_FLAGS F_NOT_RATE_CHANGER

//	include common header
#include "components/process.h"



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
	Symbol			triggerEvent;
	double			triggerTime;

};





////////////////	OPERATION METHODS

COMPONENT_CLASS_CPP::COMPONENT_CLASS_CPP()
{
	triggerEvent = S_NULL;
	triggerTime = 0;
}

Symbol COMPONENT_CLASS_CPP::event(Event* event)
{
	bout << "except received event " << getSymbolString(event->type) << " (" << hex << event->type << " == " << triggerEvent << dec << ")" << D_VERB;

	//	except
	if (triggerEvent != EVENT_RUN_SERVICE && triggerEvent == event->type)
	{
		bout << "this was my trigger event - excepting now!" << D_VERB;
		berr << "excepting as requested during " << getSymbolString(event->type);
	}

	switch(event->type)
	{
		case EVENT_RUN_SERVICE:
		{
			if (triggerEvent == EVENT_RUN_SERVICE && (time->now / sampleRateToRate(time->baseSampleRate)) >= triggerTime)
			{
				bout << "this was my trigger event/time - excepting now!" << D_VERB;
				berr << "excepting as requested during " << getSymbolString(event->type) << " (base samples == " << time->now << ")";
			}

			//	ok
			return C_OK;
		}

		case EVENT_STATE_SET:
		{
			//	extract DataML
			EventStateSet* data = (EventStateSet*) event->data;
			XMLNode xmlNode(data->state);
			DataMLNode nodePars(&xmlNode);

			//	extract fields
			string event = nodePars.getField("event").set(F_STRICT).getSTRING();//, true, 0);
			triggerTime = nodePars.getField("time").getDOUBLE();//, true, 0.0);

			//	event
			triggerEvent = S_NULL;
			if (event == "EVENT_RUN_SERVICE") triggerEvent = EVENT_RUN_SERVICE;
			if (event == "EVENT_RUN_RESUME") triggerEvent = EVENT_RUN_RESUME;
			if (event == "EVENT_RUN_PLAY") triggerEvent = EVENT_RUN_PLAY;
			if (event == "") triggerEvent = C_OK; // don't throw
			if (triggerEvent == S_NULL) berr << "unrecognised event";

			//	ok
			return C_OK;
		}

		case EVENT_INIT_CONNECT:
		{
			//	ok
			return C_OK;
		}
	}

	//	not serviced
	return S_NULL;
}


#include "brahms-1199.h"
