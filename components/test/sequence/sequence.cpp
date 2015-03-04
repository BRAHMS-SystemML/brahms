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

	$Id: sequence.cpp 2316 2009-11-07 20:38:30Z benjmitch $
	$Rev: 2316 $
	$Author: benjmitch $
	$Date: 2009-11-07 20:38:30 +0000 (Sat, 07 Nov 2009) $
________________________________________________________________

*/



////////////////	COMPONENT INFO

//	component information
#define COMPONENT_CLASS_STRING "client/brahms/test/sequence"
#define COMPONENT_CLASS_CPP client_brahms_test_sequence_0
#define COMPONENT_FLAGS (0)

#include <unistd.h>

//	include common header
#include "components/process.h"



////////////////	COMPONENT CLASS (DERIVES FROM Process)

class COMPONENT_CLASS_CPP : public Process
{

public:

	//	framework event function
	Symbol event(Event* event);

private:

	/*

		Private member data of the process goes here.
		You can use this to store your parameters
		and process state between calls to event().

	*/

	UINT32					state;
	UINT32					tmax;

	rng::Utility random;

	numeric::Input input;
	numeric::Output output;

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

Symbol COMPONENT_CLASS_CPP::event(Event* event)
{


	switch(event->type)
	{
		case EVENT_RUN_SERVICE:
		{

			//	access inputs
			if (input.due())
			{
				UINT32* inputContents = (UINT32*) input.getContent();
				state += inputContents[0];
			}

			//	processing
			os_sleep((UINT32)(random.get() * tmax));

			//	access outputs
			if (time->now % time->samplePeriod == 0)
			{
				//	i am now guaranteed to have an output in "output" that is "due"
				UINT32* outputContents = (UINT32*) output.getContent();
				outputContents[0] = time->now;
				outputContents[1] = state;
			}

			//	ok
			return C_OK;
		}



		case EVENT_STATE_SET:
		{
			//	extract DataML
			EventStateSet* data = (EventStateSet*) event->data;
			XMLNode xmlNode(data->state);
			DataMLNode nodeState(&xmlNode);

			state = nodeState.getField("state").getUINT32();//, true, 0);
			tmax = nodeState.getField("tmax").getUINT32();//, true, 1000);

			//	create and seed RNG
			VUINT32 seed = getRandomSeed(1);
			random.create(hComponent);
			random.select("MT2000.uniform");
			random.seed(&seed[0], 1);

			//	ok
			return C_OK;
		}



		case EVENT_INIT_CONNECT:
		{
			//	validate connectivity
			if (iif.getNumberOfPorts() != 1) berr << "expected 1 input";

			//	on first call, create output
			if (event->flags & F_FIRST_CALL)
			{
				output.setName("out");
				output.create(hComponent);
				output.setStructure(TYPE_UINT32 | TYPE_REAL, Dims(2).cdims());
			}

			//	on last call, validate input
			if (event->flags & F_LAST_CALL)
			{
				input.attach(hComponent, 0);
				input.validateStructure(TYPE_UINT32, Dims(2).cdims());
			}

			//	ok
			return C_OK;
		}




	}

	//	not serviced
	return S_NULL;


}






#include "brahms-1199.h"
