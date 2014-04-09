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

	$Id:: resample_spikes.cpp 2339 2009-11-09 22:28:32Z benjmi#$
	$Rev:: 2339                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-09 22:28:32 +0000 (Mon, 09 Nov 2009)       $
________________________________________________________________

*/



////////////////	COMPONENT INFO

//	component information
#define COMPONENT_CLASS_STRING "std/2009/resample/spikes"
#define COMPONENT_CLASS_CPP std_2009_resample_spikes_0
#define COMPONENT_FLAGS (0)

//	include common header
#include "../process.h"



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

	//	stream
	struct Stream
	{
		Stream()
		{
			p_buffer = NULL;
			capacity = 0;
		}

		//	parameters
		UINT32 capacity;

		//	output buffer
		VUINT8 buffer;
		UINT8* p_buffer;

		//	handles
		spikes::Input input;
		spikes::Output output;
	};

	//	streams
	vector<Stream> streams;

};






////////////////	OPERATION METHODS

Symbol COMPONENT_CLASS_CPP::event(Event* event)
{

	switch(event->type)
	{

		case EVENT_RUN_SERVICE:
		{
			//	for each stream
			for (UINT32 s=0; s<streams.size(); s++)
			{
				//	do nothing unless there is some capacity
				if (streams[s].capacity)
				{
					//	if due
					if (streams[s].input.due())
					{
						/*

							We do NOT need to clear the buffer before we start setting bits,
							since all set bits are cleared when they are sent out down below.

						*/

						INT32* p_input = NULL;
						UINT32 count = streams[s].input.getContent(p_input);

						for (UINT32 n=0; n<count; n++)
						{
							//	double spikes on one stream in our output time frame just get swallowed!
							streams[s].p_buffer[p_input[n]] = 1;
						}

					}

					//	if output is due, pass output buffer spikes into output
					if (time->now % time->samplePeriod == 0)
					{
						//	construct output
						VINT32 output_contents;
						for (UINT32 stream=0; stream<streams[s].capacity; stream++)
						{
							if (streams[s].p_buffer[stream])
							{
								output_contents.push_back(stream);
								streams[s].p_buffer[stream] = 0; // bit cleared (see above)
							}
						}

						//	fill output
						INT32* p = output_contents.size() ? &output_contents[0] : NULL;
						streams[s].output.setContent(p, output_contents.size());
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

			//	ok
			return C_OK;
		}

		case EVENT_INIT_PRECONNECT:
		{
			//	resize
			streams.resize(iif.getNumberOfPorts());

			//	attach
			for (UINT32 s=0; s<streams.size(); s++)
				streams[s].input.attach(hComponent, s);

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
					//	store capacity
					streams[s].capacity = streams[s].input.getCapacity();

					//	resize buffers
					if (streams[s].capacity)
					{
						streams[s].buffer.resize(streams[s].capacity);
						streams[s].p_buffer = &streams[s].buffer[0];
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


