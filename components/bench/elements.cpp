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

	$Id:: elements.cpp 2333 2009-11-09 12:52:11Z benjmitch     $
	$Rev:: 2333                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-09 12:52:11 +0000 (Mon, 09 Nov 2009)       $
________________________________________________________________

*/



////////////////	COMPONENT INFO

//	component information
#define COMPONENT_CLASS_CPP client_brahms_bench_elements_0
#define COMPONENT_CLASS_STRING "client/brahms/bench/elements"
#define COMPONENT_FLAGS (F_NOT_RATE_CHANGER)

//	include common header
#include "../process.h"

//	STL
#include <algorithm>
using namespace std;

#include "scalable.h"


#ifdef USE_FLOAT
const TYPE TYPE_STATE = TYPE_DOUBLE;
#else
const TYPE TYPE_STATE = TYPE_UINT32;
#endif


//	process
class COMPONENT_CLASS_CPP : public Process
{
public:
	Symbol event(Event* event);

private:

	//	pars
	PARS					pars;

	//	state
	STATE					state;
	bool					listened;

	//	ports
	numeric::Input input;
	numeric::Output output;
};

Symbol COMPONENT_CLASS_CPP::event(Event* event)
{
	switch(event->type)
	{
		case EVENT_STATE_SET:
		{
			EventStateSet* data = (EventStateSet*) event->data;
			XMLNode xmlNode(data->state);
			DataMLNode nodeState(&xmlNode);

			//	get pars
			pars.P = nodeState.getField("P").getUINT32();
			pars.E = nodeState.getField("E").getUINT32();
			pars.O = nodeState.getField("O").getUINT32();
			pars.N = nodeState.getField("N").getUINT32();


/*
			if (pars.E%(WRAP))
				berr << "E%WRAP != 0";
				*/

			//	init state
			UINT32 p = nodeState.getField("p").getUINT32();
			state.init(pars, p);
			return C_OK;
		}

		case EVENT_INIT_PRECONNECT:
		{
			//	validate connectivity
			if (iif.getNumberOfPorts() != 1) berr << "expected 1 inputs";
			return C_OK;
		}

		case EVENT_INIT_CONNECT:
		{
			if (event->flags & F_LAST_CALL)
			{
				//	validate input
				input.attach(hComponent, 0);
				input.validateStructure(TYPE_STATE | TYPE_REAL, Dims(1).cdims());
			}

			if (event->flags & F_FIRST_CALL)
			{
				//	create output
				output.setName("out");
				output.create(hComponent);
				output.setStructure(TYPE_STATE | TYPE_REAL, Dims(1).cdims());
			}

			return C_OK;
		}

		case EVENT_INIT_POSTCONNECT:
		{
			//	check if port is listened
			listened = oif.getPortFlags(output.getPort()) & F_LISTENED;
			return C_OK;
		}

		case EVENT_RUN_SERVICE:
		{
			//	access input
			STATE_TYPE in = *((STATE_TYPE*) input.getContent());

			//	step state
			STATE_TYPE out = *state.step(in);

			//	access output
			*((STATE_TYPE*) output.getContent()) = out;

			//	ok
			return C_OK;
		}
	}

	//	not processed
	return S_NULL;
}

#include "brahms-1199.h"

