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

	$Id:: listened.cpp 2333 2009-11-09 12:52:11Z benjmitch     $
	$Rev:: 2333                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-09 12:52:11 +0000 (Mon, 09 Nov 2009)       $
________________________________________________________________

*/



////////////////	COMPONENT INFO

//	component information
#define COMPONENT_CLASS_STRING "client/brahms/test/listened"
#define COMPONENT_CLASS_CPP client_brahms_test_listened_0
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

	//	output ports
	numeric::Output outputA;
	numeric::Output outputB;

	//	my state variables
	bool listenedA;
	bool listenedB;
	bool publishAnyway;
	UINT32 N;
};




////////////////	OPERATION METHODS

COMPONENT_CLASS_CPP::COMPONENT_CLASS_CPP()
{
}

Symbol COMPONENT_CLASS_CPP::event(Event* event)
{
	switch(event->type)
	{
		case EVENT_STATE_SET:
		{
			//	extract DataML
			EventStateSet* data = (EventStateSet*) event->data;
			XMLNode xmlNode(data->state);
			DataMLNode nodePars(&xmlNode);

			//	extract fields
			publishAnyway = nodePars.getField("publishAnyway").getBOOL();
			N = nodePars.getField("N").getUINT32();

			//	ok
			return C_OK;
		}

		case EVENT_INIT_CONNECT:
		{
			//	on first call
			if (event->flags & F_FIRST_CALL)
			{
				//	create one output, real scalar DOUBLE
				outputA.setName("A");
				outputA.create(hComponent);
				outputA.setStructure(TYPE_REAL | TYPE_DOUBLE, Dims(N, N).cdims());

				//	create one output, real scalar DOUBLE
				outputB.setName("B");
				outputB.create(hComponent);
				outputB.setStructure(TYPE_REAL | TYPE_DOUBLE, Dims(N, N).cdims());
			}

			//	ok
			return C_OK;
		}

		case EVENT_INIT_POSTCONNECT:
		{
			//	get listened state of both ports
			listenedA = outputA.getFlags() & F_LISTENED;
			listenedB = outputB.getFlags() & F_LISTENED;
			bout << "listenedA == " << listenedA << D_INFO;
			bout << "listenedB == " << listenedB << D_INFO;

			//	ok
			return C_OK;
		}

		case EVENT_RUN_SERVICE:
		{
			//	do work for each listened port
			for (UINT32 p=0; p<2; p++)
			{
				DOUBLE* p_data = NULL;

				if (p)
				{
					//	only do the work if somebody's listening...
					if (!listenedB) continue;

					//	access content
					p_data = (DOUBLE*) outputB.getContent();
				}

				else
				{
					//	only do the work if somebody's listening...
					if (!listenedA) continue;

					//	access content
					p_data = (DOUBLE*) outputA.getContent();
				}

				DOUBLE start = 42.0;

				for (UINT32 r=0; r<1000; r++)
				{
					for (UINT32 m=0; m<N; m++)
					{
						for (UINT32 n=0; n<N; n++)
						{
							p_data[m * N + n] = start;
							start *= 0.99999;
							start += 1.0;
						}
					}
				}
			}

			//	ok
			return C_OK;
		}
	}

	//	not serviced
	return S_NULL;
}


#include "brahms-1199.h"
