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

	$Id:: random_numeric.cpp 2428 2009-12-11 15:10:37Z benjmit#$
	$Rev:: 2428                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-12-11 15:10:37 +0000 (Fri, 11 Dec 2009)       $
________________________________________________________________

*/



////////////////	COMPONENT INFO

//	component information
#define COMPONENT_CLASS_STRING "std/2009/random/numeric"
#define COMPONENT_CLASS_CPP std_2009_random_numeric_0
#define COMPONENT_ADDITIONAL "Author=Ben Mitch\n" "URL=http://brahms.sourceforge.net\n"

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

	//	parameters
	Dims					dims;
	VDOUBLE					pars;
	bool					complex;
	UINT32					numEls;

	//	workspace
	rng::Utility random;

	//	handles
	numeric::Output output;
	string outputName;
};





////////////////	OPERATION METHODS

Symbol COMPONENT_CLASS_CPP::event(Event* event)
{


	switch(event->type)
	{

		case EVENT_RUN_SERVICE:
		{
			//	access output
			DOUBLE* p = (DOUBLE*) output.getContent();

			//	generate random numbers directly into output object
			random.fill(p, numEls, pars[1], pars[0]);

			//	ok
			return C_OK;
		}


		case EVENT_STATE_SET:
		{
			//	extract DataML
			EventStateSet* data = (EventStateSet*) event->data;
			XMLNode xmlNode(data->state);
			DataMLNode nodeState(&xmlNode);

			//	extract dims
			dims = nodeState.getField("dims").getArrayDims();
			numEls = 1;
			if (dims.size())
			{
				for (UINT32 d=0; d<dims.size(); d++)
					numEls *= dims[d];
			}
			else numEls = 0;

			//	extract distribution parameters
			string dist = nodeState.getField("dist").getSTRING();
			pars = nodeState.getField("pars").getArrayDOUBLE();

			//	for now, just make sure it's 2!
			if (pars.size() != 2) berr << "expected 2 parameters in ini.pars";

			//	parameter conversion for uniform
			//	[min max] passed in becomes [offset gain] for generator
			if (dist == "uniform") pars[1] = pars[1] - pars[0];

			//	extract complex flag
			if (nodeState.hasField("complex"))
				complex = nodeState.getField("complex").getBOOL();
			else complex = false;
			if (complex) numEls *= 2;

/*
	I used this code for testing the random seeding system in 0.7.2

			//	get three seeds
			for (int n=0; n<3; n++)
			{
				VUINT32 seed = getRandomSeed(4);
				for (int i=0; i<seed.size(); i++)
				{
					stringstream ss;
					ss << hex << seed[i];
					string s = ss.str();
					while (s.length() < 8) s = "0" + s;
					bout << s << " ";
				}
				bout << D_INFO;
			}
*/

			//	create and seed RNG
			VUINT32 seed = getRandomSeed(1);
			string specification = "MT2000." + dist;
			random.create(hComponent);
			random.select(specification.c_str());
			random.seed(&seed[0], 1);

			//	output name
			if (nodeState.hasField("outputName"))
				outputName = nodeState.getField("outputName").getSTRING();
			else outputName = "out";

			//	ok
			return C_OK;
		}


		case EVENT_INIT_PRECONNECT:
		{
			//	make sure we got no inputs
			if (iif.getNumberOfPorts())
				berr << "expects no inputs";

			//	ok
			return C_OK;
		}

		case EVENT_INIT_CONNECT:
		{
			//	instantiate output
			output.setName(outputName.c_str());
			output.create(hComponent);
			output.setStructure(TYPE_DOUBLE | (complex ? TYPE_COMPLEX : TYPE_REAL), dims.cdims());

			//	ok
			return C_OK;
		}



	}


	//	not serviced
	return S_NULL;

}



#include "brahms-1199.h"
