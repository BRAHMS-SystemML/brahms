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

	$Id:: random_spikes.cpp 2419 2009-11-30 18:33:48Z benjmitc#$
	$Rev:: 2419                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-30 18:33:48 +0000 (Mon, 30 Nov 2009)       $
________________________________________________________________

*/



////////////////	COMPONENT INFO

//	component information
#define COMPONENT_CLASS_STRING "std/2009/random/spikes"
#define COMPONENT_CLASS_CPP std_2009_random_spikes_0
#define COMPONENT_FLAGS F_NOT_RATE_CHANGER

//	include common header
#include "../process.h"



////////////////	COMPONENT CLASS (DERIVES FROM Process)

class COMPONENT_CLASS_CPP : public Process
{

public:

	COMPONENT_CLASS_CPP();

	//	framework event function
	Symbol event(Event* event);

private:

	/*

		Private member data of the process goes here.
		You can use this to store your parameters
		and process state between calls to event().

	*/

	UINT32					num_streams;
	INT32					current_sample;

	struct SOURCESPIKES_GEN
	{
		double				min;
		double				std;
		VINT32				nextSpike;
		VDOUBLE				rand;
	}
	gen;

	rng::Utility random;
	spikes::Output output;
	string outputName;

};



////////////////	'STRUCTORS

COMPONENT_CLASS_CPP::COMPONENT_CLASS_CPP()
{
	current_sample = 0;
	num_streams = 0;
	memset(&gen,0,sizeof(gen));
}





////////////////	OPERATION METHODS


Symbol COMPONENT_CLASS_CPP::event(Event* event)
{


	switch(event->type)
	{

		case EVENT_RUN_SERVICE:
		{

			//	copy the next source data into our output
			VINT32 spikes;
			UINT32 s_used = 0;

			//	fire those that are due
			for (UINT32 s=0; s<num_streams; s++)
			{
				//	trigger spikes
				if (gen.nextSpike[s] == current_sample)
				{
					spikes.push_back(s);
					gen.nextSpike[s] = (INT32) (current_sample + gen.rand[s]);
					s_used = s + 1;
				}
			}

			//	generate ISIs
			if (s_used) random.fill(&gen.rand[0], s_used, gen.std, gen.min);

			//	pass on to output
			output.setContent(spikes.size() ? &spikes[0] : NULL, spikes.size());

			//	keep track of current sample
			current_sample++;

			//	ok
			return C_OK;

		}


		case EVENT_STATE_SET:
		{
			//	extract DataML
			EventStateSet* data = (EventStateSet*) event->data;
			XMLNode xmlNode(data->state);
			DataMLNode nodeState(&xmlNode);

			//	create and seed RNG
			VUINT32 seed = getRandomSeed(1);
			random.create(hComponent);
			random.select("MT2000.exponential");
			random.seed(&seed[0], 1);

			//	generator description...
			string isi = nodeState.getField("dist").getSTRING();

			//	exponential generator
			if (isi == "exponential")
			{
				num_streams = nodeState.getField("streams").getUINT32();
				VDOUBLE pars = nodeState.getField("pars").getArrayDOUBLE();
				if (pars.size() != 2) berr << "expected 2 parameters in \"pars\"";

				gen.min = pars[0] * sampleRateToRate(time->sampleRate);
				gen.std = pars[1] * sampleRateToRate(time->sampleRate);

				//	do initial ISI
				gen.nextSpike.resize(num_streams);
				gen.rand.resize(num_streams);
				random.fill(&gen.rand[0], num_streams, gen.std, gen.min);
				for (UINT32 s=0; s<num_streams; s++) gen.nextSpike[s] = (INT32) gen.rand[s];
			}

			//	else what is it?
			else berr << "unrecognised generator \"" + isi + "\"";

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
			output.setCapacity(num_streams);

			//	ok
			return C_OK;
		}



	}



	//	not serviced
	return S_NULL;


}




#include "brahms-1199.h"


