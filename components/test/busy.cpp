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

	$Id:: busy.cpp 2359 2009-11-11 01:40:21Z benjmitch         $
	$Rev:: 2359                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-11 01:40:21 +0000 (Wed, 11 Nov 2009)       $
________________________________________________________________

*/


////////////////	COMPONENT INFO

//	component information
#define COMPONENT_CLASS_STRING "client/brahms/test/busy"
#define COMPONENT_CLASS_CPP client_brahms_test_busy_0
#define COMPONENT_FLAGS F_NOT_RATE_CHANGER

//	include common header
#include "../process.h"

#ifdef __WIN__
#include "windows.h"
#define msleep(msec) Sleep(msec)
#endif

#ifdef __NIX__
#define msleep(msec) usleep(msec*1000)
#endif



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

	numeric::Output output;

	//	my state variables
	bool			sleep;
	UINT32			msec;

};





////////////////	OPERATION METHODS

COMPONENT_CLASS_CPP::COMPONENT_CLASS_CPP()
{
	sleep = true;
}

Symbol COMPONENT_CLASS_CPP::event(Event* event)
{
	switch(event->type)
	{
		case EVENT_RUN_SERVICE:
		{
			if (sleep)
			{
				msleep(msec);
			}

			else
			{
				double A = 0.0;
				for (UINT32 a=0; a<msec; a++)
				{
					for (int b=0; b<40000; b++)
					{
						A = A + 1.0;
						A = A / 2.0;
					}
				}
				output.setContent(&A);
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
			sleep = nodePars.getField("sleep").getBOOL();
			msec = nodePars.getField("msec").getUINT32();
			if (msec < 1 || msec > 10000) berr << "invalid msec";

			//	ok
			return C_OK;
		}

		case EVENT_INIT_CONNECT:
		{
			if (event->flags & F_FIRST_CALL)
			{
				//	create one output
				output.setName("out");
				output.create(hComponent);
				output.setStructure(TYPE_DOUBLE | TYPE_REAL, Dims(1).cdims());
			}

			//	ok
			return C_OK;
		}
	}

	//	not serviced
	return S_NULL;
}


#include "brahms-1199.h"
