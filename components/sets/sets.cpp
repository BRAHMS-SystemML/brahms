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

	$Id:: sets.cpp 2419 2009-11-30 18:33:48Z benjmitch         $
	$Rev:: 2419                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-30 18:33:48 +0000 (Mon, 30 Nov 2009)       $
________________________________________________________________

*/



////////////////	COMPONENT INFO

//	component information
#define COMPONENT_CLASS_STRING "client/brahms/test/sets"
#define COMPONENT_CLASS_CPP client_brahms_test_sets_0
#define COMPONENT_RELEASE __REL__
#define COMPONENT_REVISION __REV__
#define COMPONENT_ADDITIONAL "Author=Ben Mitch\n" "URL=http://brahms.sourceforge.net\n"
#define COMPONENT_FLAGS (0)

//	in this file, we don't define OVERLAY_QUICKSTART_PROCESS, because
//	we use this file to test if the data/util headers compile ok
//	without importing the namespace "brahms". so, we have to add a
//	couple o' things manually...
#define COMPONENT_PROCESS

//	include the component interface overlay, NOT quick process mode
#include "brahms-1199.h"

//	manually, i said...
const struct brahms::ComponentVersion COMPONENT_VERSION =
{
	COMPONENT_RELEASE,
	COMPONENT_REVISION
};

//	manually import namespaces
#include "std/2009/data/numeric/brahms/0/data.h"
namespace numeric = std_2009_data_numeric_0;
#include "std/2009/data/spikes/brahms/0/data.h"
namespace spikes = std_2009_data_spikes_0;
#include "std/2009/util/rng/brahms/0/utility.h"
namespace rng = std_2009_util_rng_0;

//	now, we may as well import namespaces, since we got through the above OK
using namespace brahms;
using namespace std;



////////////////	COMPONENT CLASS (DERIVES FROM Process)

enum Operation
{
	OP_NULL = 0,
	OP_DOUBLE,
	OP_ROUND,
	OP_INVERT
};

class COMPONENT_CLASS_CPP : public Process
{

public:

	//	framework event function
	Symbol event(Event* event);

private:

	//	stream
	struct Stream
	{
		numeric::Input input;
		numeric::Output output;
		UINT32 elementCount;
		Operation operation;
	};

	//	streams
	vector<Stream> streams;

	//	sets
	Symbol hSetDoubleIn, hSetDoubleOut;
	Symbol hSetRoundIn, hSetRoundOut;
	Symbol hSetInvertIn, hSetInvertOut;
};






////////////////	OPERATION METHODS

Symbol COMPONENT_CLASS_CPP::event(Event* event)
{

	switch(event->type)
	{

		case EVENT_RUN_SERVICE:
		{
			//	each stream
			for (UINT32 s=0; s<streams.size(); s++)
			{
				//	access input
				DOUBLE* p = (DOUBLE*) streams[s].input.getContent();

				//	access output
				DOUBLE* q = (DOUBLE*) streams[s].output.getContent();

				//	access count
				UINT32 N = streams[s].elementCount;

				//	perform operation
				switch (streams[s].operation)
				{
					case OP_DOUBLE:
					{
						for(UINT32 n=0; n<N; n++)
							q[n] = 2.0 * p[n];
						break;
					}

					case OP_ROUND:
					{
						for(UINT32 n=0; n<N; n++)
							q[n] = floor(p[n] + 0.5);
						break;
					}

					case OP_INVERT:
					{
						for(UINT32 n=0; n<N; n++)
							q[n] = -p[n];
						break;
					}
					
					default:
					{
						berr << E_INTERNAL;
					}
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

			//	get handles to sets
			hSetDoubleIn = iif.getSet("double");
			hSetRoundIn = iif.getSet("round");
			hSetInvertIn = iif.getSet("invert");
			hSetDoubleOut = oif.getSet("double");
			hSetRoundOut = oif.getSet("round");
			hSetInvertOut = oif.getSet("invert");

			//	ok
			return C_OK;
		}

		case EVENT_INIT_PRECONNECT:
		{
			//	create streams
			UINT32 count = 
				iif.getNumberOfPorts(hSetDoubleIn)
				+
				iif.getNumberOfPorts(hSetRoundIn)
				+
				iif.getNumberOfPorts(hSetInvertIn)
				;
			streams.resize(count);
			UINT32 s = 0;

			//	attach streams
			count = iif.getNumberOfPorts(hSetDoubleIn);
			for (UINT32 c=0; c<count; c++)
			{
				Stream& stream = streams[s];
				stream.input.selectSet(hSetDoubleIn);
				stream.input.attach(hComponent, c);
				stream.output.selectSet(hSetDoubleOut);
				stream.operation = OP_DOUBLE;
				s++;
			}

			//	attach streams
			count = iif.getNumberOfPorts(hSetRoundIn);
			for (UINT32 c=0; c<count; c++)
			{
				Stream& stream = streams[s];
				stream.input.selectSet(hSetRoundIn);
				stream.input.attach(hComponent, c);
				stream.output.selectSet(hSetRoundOut);
				stream.operation = OP_ROUND;
				s++;
			}

			//	attach streams
			count = iif.getNumberOfPorts(hSetInvertIn);
			for (UINT32 c=0; c<count; c++)
			{
				Stream& stream = streams[s];
				stream.input.selectSet(hSetInvertIn);
				stream.input.attach(hComponent, c);
				stream.output.selectSet(hSetInvertOut);
				stream.operation = OP_INVERT;
				s++;
			}

			//	ok
			return C_OK;
		}

		case EVENT_INIT_CONNECT:
		{
			//	for each stream that is not yet created, create it
			for (UINT32 s=0; s<streams.size(); s++)
			{
				//	only handle each stream once
				if (streams[s].input.due() && !streams[s].input.seen())
				{
					//	get structure
					const numeric::Structure* structure = streams[s].input.getStructure();

					//	check it's a double type
					if (structure->typeElement != TYPE_DOUBLE)
						berr << "only double type inputs are accepted";

					//	store information
					streams[s].elementCount = structure->numberOfElementsTotal;

					//	create output as copy of input
					stringstream ss;
					ss << "a" << s;
					string st = ss.str();
					streams[s].output.setName(st.c_str());
					streams[s].output.create(hComponent, streams[s].input);
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

