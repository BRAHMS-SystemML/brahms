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

	$Id:: process.cpp 2316 2009-11-07 20:38:30Z benjmitch      $
	$Rev:: 2316                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-07 20:38:30 +0000 (Sat, 07 Nov 2009)       $
________________________________________________________________

*/





////////////////	COMPONENT INFO

#define COMPONENT_CLASS_STRING "client/brahms/language/1199"
#define COMPONENT_CLASS_CPP client_brahms_language_1199_0
#define COMPONENT_FLAGS F_NOT_RATE_CHANGER

//	include common header
#include "components/process.h"



////////////////	COMPONENT CLASS (DERIVES FROM Process)

class COMPONENT_CLASS_CPP : public Process
{

public:

	//	use ctor/dtor if required
	COMPONENT_CLASS_CPP() {}
	~COMPONENT_CLASS_CPP() {}

	//	framework event function
	Symbol event(Event* event);

private:

	//	parameters
	UINT32 count;
	DOUBLE noise;

	//	state
	DOUBLE accum;

	//	ports
	numeric::Input input;
	numeric::Output output;

	//	utilities
	rng::Utility random;

};



////////////////	EVENT

Symbol COMPONENT_CLASS_CPP::event(Event* event)
{
	switch(event->type)
	{
		case EVENT_STATE_SET:
		{
			bout << "Hello from C++" << D_INFO;

			//	extract DataML
			EventStateSet* data = (EventStateSet*) event->data;
			XMLNode xmlNode(data->state);
			DataMLNode nodeState(&xmlNode);

			//	extract parameters
			count = nodeState.getField("count").getUINT32();
			noise = nodeState.getField("noise").getDOUBLE();

			//	initialise state
			accum = 0.0;

			//	obtain a random number generator
			random.create(hComponent);

			//	initialise it
			random.select("MT2000.normal");
			VUINT32 seed = getRandomSeed(1);
			random.seed(&seed[0], 1);

			//	ok
			return C_OK;
		}

		case EVENT_INIT_CONNECT:
		{
			//	on first call
			if (event->flags & F_FIRST_CALL)
			{
				//	check expected number of inputs
				if (iif.getNumberOfPorts() != 1)
					berr << "expects one input";

				//	create output
				output.setName("out");
				output.create(hComponent);
				output.setStructure(TYPE_DOUBLE | TYPE_REAL, Dims(count).cdims());
			}

			//	on last call
			if (event->flags & F_LAST_CALL)
			{
				//	validate input
				input.attach(hComponent, 0);
				input.validateStructure(TYPE_DOUBLE | TYPE_REAL, Dims(1).cdims());
			}

			//	ok
			return C_OK;
		}

		case EVENT_RUN_SERVICE:
		{
			//	get input
			DOUBLE in = *((DOUBLE*) input.getContent());

			//	process
			accum = 0.9 * accum + in;

			//	set output
			DOUBLE* out = (DOUBLE*) output.getContent();
			random.fill(out, count, noise);
			for (UINT32 n=0; n<count; n++)
				out[n] += accum;

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
