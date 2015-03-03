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

	$Id:: resample_numeric.cpp 2339 2009-11-09 22:28:32Z benjm#$
	$Rev:: 2339                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-09 22:28:32 +0000 (Mon, 09 Nov 2009)       $
________________________________________________________________

*/



////////////////	COMPONENT INFO

//	component information
#define COMPONENT_CLASS_STRING "std/2009/resample/numeric"
#define COMPONENT_CLASS_CPP std_2009_resample_numeric_0
#define COMPONENT_FLAGS (0)

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
	//
	//	"order" (resampling order):
	//		0: output is always most recent input (zero-order hold)
	//		1: output is linear piecewise reconstruction of input (linear interpolation)
	UINT32					order;					//	order of resampling - see below
	bool					zero;					//	if true, set undefined outputs to zero

	//	other
	DOUBLE					t_conv;

	//	stream
	struct Stream
	{
		Stream()
		{
			//	(!output.due()) indicates "not yet initialised"
			type = 0;
			numEls = 0;
			t0 = 0.0;
			t1 = 0.0;
		}

		//	port handles
		numeric::Input	input;
		numeric::Output	output;

		//	data structure
		TYPE				type;
		UINT64				numEls;
		UINT64				numBytes;

		//	last two input data
		VBYTE				in0;
		VBYTE				in1;

		//	and the times they were seen
		DOUBLE				t0;
		DOUBLE				t1;

		//	buffer pointers
		BYTE*				p_in0;
		BYTE*				p_in1;
	};

	//	streams
	vector<Stream>			streams;
};






////////////////	OPERATION METHODS

template <class T> void resample(T* p0, DOUBLE t0, T* p1, DOUBLE t1, T* p, DOUBLE t, UINT64 N, bool zero)
{
	if (zero && !t0)
	{
		for (UINT64 e=0; e<N; e++)
		{
			p[e] = 0.0;
		}
	}

	else
	{
		//	this produces NaNs automatically when the output is undefined (but might be slow at doing that?)
		for (UINT64 e=0; e<N; e++)
		{
			DOUBLE s0 = p0[e];
			DOUBLE s1 = p1[e];
			DOUBLE fac = (t - t0) / (t0 - t1);
			p[e] = s1 + fac * (s0 - s1);
		}
	}
}

Symbol COMPONENT_CLASS_CPP::event(Event* event)
{


	switch(event->type)
	{

		case EVENT_RUN_SERVICE:
		{
			//	time now
			DOUBLE t = time->now * t_conv;

			//	for each stream
			for (UINT32 s=0; s<streams.size(); s++)
			{
				//	check for input, and load if present
				if (streams[s].input.due())
				{
					//	flip buffers
					BYTE* temp = streams[s].p_in1;
					streams[s].p_in1 = streams[s].p_in0;
					streams[s].p_in0 = temp;

					//	load zeroth buffer
					memcpy(streams[s].p_in0, streams[s].input.getContent(), streams[s].numBytes);

					//	fix times
					streams[s].t1 = streams[s].t0;
					streams[s].t0 = t;
				}

				//	check for output, and calculate if present
				if (streams[s].output.due())
				{
					//	access
					void* p_output = streams[s].output.getContent();

					//	first order
					if (order)
					{
						//	prepare
						DOUBLE t0 = streams[s].t0;
						DOUBLE t1 = streams[s].t1;

						//	call template based on known type
						switch(streams[s].type)
						{
							case TYPE_DOUBLE: resample((DOUBLE*)streams[s].p_in0, t0, (DOUBLE*)streams[s].p_in1, t1, (DOUBLE*)p_output, t, streams[s].numEls, zero); break;
							case TYPE_SINGLE: resample((SINGLE*)streams[s].p_in0, t0, (SINGLE*)streams[s].p_in1, t1, (SINGLE*)p_output, t, streams[s].numEls, zero); break;
							case TYPE_UINT64: resample((UINT64*)streams[s].p_in0, t0, (UINT64*)streams[s].p_in1, t1, (UINT64*)p_output, t, streams[s].numEls, zero); break;
							case TYPE_UINT32: resample((UINT32*)streams[s].p_in0, t0, (UINT32*)streams[s].p_in1, t1, (UINT32*)p_output, t, streams[s].numEls, zero); break;
							case TYPE_UINT16: resample((UINT16*)streams[s].p_in0, t0, (UINT16*)streams[s].p_in1, t1, (UINT16*)p_output, t, streams[s].numEls, zero); break;
							case TYPE_UINT8: resample((UINT8*)streams[s].p_in0, t0, (UINT8*)streams[s].p_in1, t1, (UINT8*)p_output, t, streams[s].numEls, zero); break;
							case TYPE_INT64: resample((INT64*)streams[s].p_in0, t0, (INT64*)streams[s].p_in1, t1, (INT64*)p_output, t, streams[s].numEls, zero); break;
							case TYPE_INT32: resample((INT32*)streams[s].p_in0, t0, (INT32*)streams[s].p_in1, t1, (INT32*)p_output, t, streams[s].numEls, zero); break;
							case TYPE_INT16: resample((INT16*)streams[s].p_in0, t0, (INT16*)streams[s].p_in1, t1, (INT16*)p_output, t, streams[s].numEls, zero); break;
							case TYPE_INT8: resample((INT8*)streams[s].p_in0, t0, (INT8*)streams[s].p_in1, t1, (INT8*)p_output, t, streams[s].numEls, zero); break;
							default: berr << "resample/numeric not coded for this data type";
						}

					}

					//	zero order
					else
					{
						//	unload zeroth buffer
						memcpy(p_output, streams[s].p_in0, streams[s].numBytes);
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

			//	get parameter
			if (nodeState.hasField("order"))
				order = nodeState.getField("order").getUINT32();
			else order = 0;
			if (order > 1) berr << "invalid parameter \"order\" (must be one or zero)";

			//	get parameter
			if (nodeState.hasField("zero"))
				zero = nodeState.getField("zero").getBOOL();
			else zero = true;

			//	ok
			return C_OK;
		}

		case EVENT_INIT_PRECONNECT:
		{
			//	one stream for each input port
			streams.resize(iif.getNumberOfPorts());

			//	attach
			for (UINT32 s=0; s<streams.size(); s++)
				streams[s].input.attach(hComponent, s);

			//	ok
			return C_OK;
		}

		case EVENT_INIT_POSTCONNECT:
		{
			//	other prep
			t_conv =((DOUBLE)time->baseSampleRate.den) / ((DOUBLE)time->baseSampleRate.num);

			//	ok
			return C_OK;
		}

		case EVENT_INIT_CONNECT:
		{
			//	for each stream
			for (UINT32 s=0; s<streams.size(); s++)
			{
				//	only handle each stream once
				if (streams[s].input.due() && !streams[s].input.seen())
				{
					//	validate and get structure
					const numeric::Structure* structure = streams[s].input.getStructure();

					//	store information
					streams[s].type = structure->typeElement;
					streams[s].numEls = structure->numberOfElementsTotal;
					streams[s].numBytes = structure->numberOfBytesTotal;

					//	resize buffers
					if (streams[s].numBytes)
					{
						streams[s].in0.resize(streams[s].numBytes);
						streams[s].in1.resize(streams[s].numBytes);

						streams[s].p_in0 = &streams[s].in0[0];
						streams[s].p_in1 = &streams[s].in1[0];
					}

					//	create output as copy of input
					streams[s].output.setSampleRate(time->sampleRate);
					streams[s].output.setName(streams[s].input.getName());
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
