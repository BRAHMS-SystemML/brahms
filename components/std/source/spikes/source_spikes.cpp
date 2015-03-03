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

	$Id:: source_spikes.cpp 2419 2009-11-30 18:33:48Z benjmitc#$
	$Rev:: 2419                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-30 18:33:48 +0000 (Mon, 30 Nov 2009)       $
________________________________________________________________

*/



////////////////	COMPONENT INFO

//	component information
#define COMPONENT_CLASS_STRING "std/2009/source/spikes"
#define COMPONENT_CLASS_CPP std_2009_source_spikes_0
#define COMPONENT_FLAGS F_NOT_RATE_CHANGER

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

	//	buffers
	VINT32 tsMatrix;
	VINT32 outputSpikes;

	struct SOURCESPIKES_USER
	{
		//	streams
		INT32				s0, s1;
		INT32				n_streams;

		//	time
		INT32				t0, t1;
		INT32				t_next;

		//	ts matrix
		INT32*				ts;
		UINT32				n_spikes;
		UINT32				i_next;
	}
	user;

	//	pars
	bool repeat;
	UINT32 i_next_original;

	spikes::Output output;
	string outputName;

};






////////////////	OPERATION METHODS


Symbol COMPONENT_CLASS_CPP::event(Event* event)
{


	switch(event->type)
	{

		case EVENT_RUN_SERVICE:
		{
			//	clear
			outputSpikes.clear();

			//	any left?
			if (user.t_next == user.t1)
			{
				if (!repeat) berr << "source data has run out (set flag 'repeat' to use them more than once)";
				user.t_next = user.t0;
				user.i_next = i_next_original;
			}

			//	next spikes would be, if anywhere, at next_spike
			while(user.i_next < user.n_spikes)
			{
				if (user.ts[user.i_next*2] < user.t_next) berr << "source spikes not in chronological order, or fall outside specified window";
				if (user.ts[user.i_next*2] > user.t_next) break; // spike occurs at a later sample
				INT32 stream = user.ts[user.i_next*2+1] - user.s0; // offset into streams window

				//	ignore out-of-window spikes
				if (stream >= 0 && stream < user.n_streams)
					outputSpikes.push_back(stream);

				//	advance
				user.i_next++;
			}

			//	pass on to output
			output.setContent(outputSpikes.size() ? &outputSpikes[0] : NULL, outputSpikes.size());

			//	keep track of current sample
			user.t_next++;

			//	ok
			return C_OK;
		}

		case EVENT_STATE_SET:
		{
			//	extract DataML
			EventStateSet* data = (EventStateSet*) event->data;
			XMLNode xmlNode(data->state);
			DataMLNode nodeState(&xmlNode);

			//	spikes structure...
			DOUBLE fS = nodeState.getField("fS").getDOUBLE();
			if (fS != sampleRateToRate(time->sampleRate))
				berr << "spikes structure does not match process sample rate";

			//	produce whatever window is supplied, with first sample of window appearing at t=1
			VINT32 t = nodeState.getField("t").validate(Dims(1, 2)).getArrayINT32();
			user.t0 = t[0];
			user.t1 = t[1];
			user.t_next = user.t0;

			VINT32 s = nodeState.getField("s").validate(Dims(1, 2)).getArrayINT32();
			if (s[0] > s[1]) berr << "invalid spikes structure, s(1) > s(2)";
			user.s0 = s[0];
			user.s1 = s[1];
			user.n_streams = user.s1 - user.s0;

			tsMatrix = nodeState.getField("ts").validate(Dims(2, DIM_ANY)).getArrayINT32();
			user.n_spikes = tsMatrix.size() / 2;
			user.ts = user.n_spikes ? (INT32*)&tsMatrix[0] : NULL;
			user.i_next = 0;

			//	advance through any spikes that fall pre-window
			while(user.i_next < user.n_spikes)
			{
				if (user.ts[user.i_next*2] >= user.t_next) break;
				user.i_next++;
			}

			//	repeat
			if (nodeState.hasField("repeat"))
				repeat = nodeState.getField("repeat").getBOOL();
			else repeat = false;

			//	store current spike pointer in case we repeat
			i_next_original = user.i_next;

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
			output.setCapacity(user.n_streams);

			//	ok
			return C_OK;
		}


	}



	//	not serviced
	return S_NULL;

}



#include "brahms-1199.h"
