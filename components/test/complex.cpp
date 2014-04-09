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

	$Id:: index.cpp 2426 2009-12-10 21:10:50Z benjmitch        $
	$Rev:: 2426                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-12-10 21:10:50 +0000 (Thu, 10 Dec 2009)       $
________________________________________________________________

*/



/*

	this test process produces complex data in both adjacent and
	interleaved form, for testing automatic translation in
	std/data/numeric

*/


////////////////	COMPONENT INFO

//	component information
#define COMPONENT_CLASS_STRING "client/brahms/test/complex"
#define COMPONENT_CLASS_CPP client_brahms_test_complex_0
#define COMPONENT_FLAGS F_NOT_RATE_CHANGER

//	include common header
#include "../process.h"



////////////////	COMPONENT CLASS

class COMPONENT_CLASS_CPP : public Process
{

public:

	//	framework event function
	Symbol event(Event* event);

private:

	//	output
	numeric::Output outputA, outputI;

};



////////////////	EVENT FUNCTION

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
			//	create output
			outputA.setName("adjacent");
			outputA.create(hComponent);
			outputA.setStructure(TYPE_DOUBLE | TYPE_COMPLEX | TYPE_CPXFMT_ADJACENT, Dims(2, 3).cdims());

			//	create output
			outputI.setName("interleaved");
			outputI.create(hComponent);
			outputI.setStructure(TYPE_DOUBLE | TYPE_COMPLEX | TYPE_CPXFMT_INTERLEAVED, Dims(2, 3).cdims());

			//	ok
			return C_OK;
		}

		case EVENT_RUN_SERVICE:
		{
			//	access output
			DOUBLE* pA = (DOUBLE*) outputA.getContent();
			DOUBLE* pI = (DOUBLE*) outputI.getContent();

			//	put incrementing numbers in
			UINT32 sz = 6;
			for (UINT32 i=0; i<sz; i++)
			{
				DOUBLE valR = i;
				DOUBLE valI = i + sz;

				pA[i] = valR;
				pA[i+sz] = valI;

				pI[i*2] = valR;
				pI[i*2+1] = valI;
			}

			//	ok
			return C_OK;
		}

	}

	//	not serviced
	return S_NULL;
}







//	include overlay (a second time)
#include "brahms-1199.h"

