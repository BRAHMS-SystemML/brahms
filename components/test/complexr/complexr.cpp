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

	this test process receives complex data in both adjacent and
	interleaved form, for testing automatic translation in
	std/data/numeric. whatever form it receives in, it generates
	output in the form specified in its state, and it asks
	data/numeric to do the conversion automagically.

*/


////////////////	COMPONENT INFO

//	component information
#define COMPONENT_CLASS_STRING "client/brahms/test/complexr"
#define COMPONENT_CLASS_CPP client_brahms_test_complexr_0
#define COMPONENT_FLAGS F_NOT_RATE_CHANGER

//	include common header
#include "components/process.h"
#include <iostream>
using namespace std;



////////////////	COMPONENT CLASS

class COMPONENT_CLASS_CPP : public Process
{

public:

	//	framework event function
	Symbol event(Event* event);

private:

	//	output
	numeric::Input input;
	numeric::Output output;

	//	mode
	bool interleaved;

	UINT64 bytes;
	TYPE typeComplexity;

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

			interleaved = nodeState.getField("interleaved").getBOOL();
			typeComplexity = TYPE_COMPLEX | (interleaved ? TYPE_CPXFMT_INTERLEAVED : TYPE_CPXFMT_ADJACENT);

			//	ok
			return C_OK;
		}

		case EVENT_INIT_CONNECT:
		{
			//	create output
			output.setName("out");
			output.create(hComponent);
			output.setStructure(TYPE_DOUBLE | typeComplexity, Dims(2, 3).cdims());

			//	attach input
			input.attach(hComponent, 0);

			//	check format
			const numeric::Structure* s = input.getStructure();
			bytes = s->numberOfBytesTotal;
//			cout << bytes << endl;

			//	set content format
			input.setReadFormat(typeComplexity | TYPE_ORDER_COLUMN_MAJOR);

			//	ok
			return C_OK;
		}

		case EVENT_RUN_SERVICE:
		{
			const DOUBLE* pi = (const DOUBLE*) input.getContent();
			DOUBLE* po = (DOUBLE*) output.getContent();

			memcpy(po, pi, bytes);

			//	ok
			return C_OK;
		}

	}

	//	not serviced
	return S_NULL;
}







//	include overlay (a second time)
#include "brahms-1199.h"
