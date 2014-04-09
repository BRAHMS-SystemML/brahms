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

	$Id:: operation.cpp 2316 2009-11-07 20:38:30Z benjmitch    $
	$Rev:: 2316                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-07 20:38:30 +0000 (Sat, 07 Nov 2009)       $
________________________________________________________________

*/





////////////////	COMPONENT INFO

#define COMPONENT_CLASS_STRING "client/brahms/bench/operation"
#define COMPONENT_CLASS_CPP client_brahms_bench_operation_0
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

	struct
	{
		UINT32			f;			//	floating-point math
		UINT32			i;			//	integer math
		UINT32			m;			//	memory writes
		UINT32			n;			//	working set size
	}
	scale;

	//	workspace
	VDOUBLE				mem;
	DOUBLE				acc;

	//	ports
	numeric::Input input;
	numeric::Output output;

};





////////////////	OPERATION METHODS

Symbol COMPONENT_CLASS_CPP::event(Event* event)
{
	switch(event->type)
	{

		case EVENT_RUN_SERVICE:
		{


		////	FLOATING-POINT

			//	limit
			if (acc > 10.0) acc /= 10.0;

			//	operation
			UINT32 op = scale.f;
			while(op--)
			{
				//	rise
				acc *= 1.000001;
			}




		////	INTEGER

			//	operation
			UINT32 val = acc;
			op = scale.i / 2;
			while(op--)
			{
				val += 13;
				val /= 2;
			}
			if (val & 1) acc += 1.0;
			else acc += 2.0;





		////	MEMORY

			//	operation
			if (scale.m && scale.n)
			{
				//	step size must walk working set
				UINT32 s = ((DOUBLE)scale.n) / ((DOUBLE)scale.m);
				if (!s) s = 1;

				DOUBLE* p = &mem[0];
				UINT32 o = 0;
				for (UINT32 m=0; m<scale.m; m++)
				{
					p[o] = acc;
					o += s;
					o = o % scale.n;
				}
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

			//	extract fields
			scale.f = nodeState.getField("f").getUINT32();
			scale.i = nodeState.getField("i").getUINT32();
			scale.m = nodeState.getField("m").getUINT32();
			scale.n = nodeState.getField("n").getUINT32();

			//	prepare memory
			mem.resize(scale.n);
			acc = 1.0;

			//	ok
			return C_OK;

		}



		case EVENT_INIT_CONNECT:
		{

			//	on last call
			if (event->flags & F_LAST_CALL)
			{
				//	validate input
				input.attach(hComponent, "out");
				input.validateStructure(TYPE_DOUBLE | TYPE_REAL, Dims(1).cdims());
			}

			//	on first call
			if (event->flags & F_FIRST_CALL)
			{
				//	create output
				output.setName("out");
				output.create(hComponent);
				output.setStructure(TYPE_DOUBLE | TYPE_REAL, Dims(1).cdims());
			}

			//	ok
			return C_OK;

		}



	}


	return S_NULL;



}



#include "brahms-1199.h"


