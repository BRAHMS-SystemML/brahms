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

	$Id:: overhead.cpp 2316 2009-11-07 20:38:30Z benjmitch     $
	$Rev:: 2316                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-07 20:38:30 +0000 (Sat, 07 Nov 2009)       $
________________________________________________________________

*/





////////////////	COMPONENT INFO

#define COMPONENT_CLASS_STRING "client/brahms/bench/overhead"
#define COMPONENT_CLASS_CPP client_brahms_bench_overhead_0
#define COMPONENT_FLAGS F_NOT_RATE_CHANGER

//	include common header
#include "../process.h"



////////////////	COMPONENT CLASS (DERIVES FROM Process)

class COMPONENT_CLASS_CPP : public Process
{

public:

	//	framework event function
	Symbol event(Event* event);

private:

	//	ports
	vector<numeric::Input> inputs;
	numeric::Output output;

};



////////////////	EVENT

Symbol COMPONENT_CLASS_CPP::event(Event* event)
{
	switch(event->type)
	{
		case EVENT_STATE_SET:
		{
			//	extract DataML
			EventStateSet* data = (EventStateSet*) event->data;
			XMLNode xmlNode(data->state);
			DataMLNode nodeState(&xmlNode);

			//	ok
			return C_OK;
		}

		case EVENT_INIT_CONNECT:
		{
			//	on first call
			if (event->flags & F_FIRST_CALL)
			{
				//	create output
				output.setName("out");
				output.create(hComponent);
				output.setStructure(TYPE_DOUBLE | TYPE_REAL, Dims(1).cdims());
			}

			//	on last call
			if (event->flags & F_LAST_CALL)
			{
				//	validate input
				inputs.resize(iif.getNumberOfPorts());
				for (UINT32 i=0; i<inputs.size(); i++)
				{
					inputs[i].attach(hComponent, i);
					inputs[i].validateStructure(TYPE_DOUBLE | TYPE_REAL, Dims(1).cdims());
				}
			}

			//	ok
			return C_OK;
		}

		case EVENT_RUN_SERVICE:
		{
			//	access output
			DOUBLE* out = (DOUBLE*) output.getContent();
			*out = 0.0;

			//	access inputs
			for (UINT32 i=0; i<inputs.size(); i++)
			{
				DOUBLE* in = (DOUBLE*) inputs[i].getContent();
				*out += *in;
			}

			//	ok
			return C_OK;
		}

	}

	//	if we service the event, we return C_OK
	//	if we don't, we should return S_NULL to indicate that
	return S_NULL;
}







//	include overlay (a second time)
#include "brahms-1199.h"

