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

	$Id:: cat.cpp 2427 2009-12-11 04:57:31Z benjmitch          $
	$Rev:: 2427                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-12-11 04:57:31 +0000 (Fri, 11 Dec 2009)       $
________________________________________________________________

*/




////////////////	COMPONENT INFO

//	component information
#define COMPONENT_CLASS_STRING "dev/std/cat/numeric"
#define COMPONENT_CLASS_CPP dev_std_cat_numeric_0
#define COMPONENT_FLAGS (F_NOT_RATE_CHANGER | F_NEEDS_ALL_INPUTS)

//	include common header
#include "components/process.h"



////////////////	COMPONENT CLASS

struct Input
{
	numeric::Input input;

	UINT64 bytesperblock; // number of input bytes that belong in a contiguous block in output
	UINT64 offset; // offset into output of first block's destination
};

class COMPONENT_CLASS_CPP : public Process
{

public:

	//	framework event function
	Symbol event(Event* event);

private:

	//	inputs
	vector<Input> inputs;

	//	output
	numeric::Output output;

	//	parameters
	UINT32 dim;

	//	common to all inputs
	UINT64 blockcount; // number of blocks of making up each input
	UINT64 stride; // additional offset for each subsequent block

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

			//	dim
			dim = nodeState.getField("dim").getUINT32();
			if (!dim) berr << "concatenation dimension cannot be zero - specify dimension in one-based form";
			if (dim > 100) berr << "concatenation dimension is greater than 100 - this seems unwise";

			//	but convert to zero-based for use
			dim--;

			//	ok
			return C_OK;
		}

		case EVENT_INIT_PRECONNECT:
		{
			//	validate input count
			if (iif.getNumberOfPorts() < 1)
				berr << "expects at least one input";

			//	ok
			return C_OK;
		}

		case EVENT_INIT_CONNECT:
		{
			//	we specify F_NEEDS_ALL_INPUTS so we only get one call
			inputs.resize(iif.getNumberOfPorts());
			TYPE type = TYPE_UNSPECIFIED;
			Dims dims;
			UINT64 offset = 0;

			//	for each input
			for (UINT32 i=0; i<inputs.size(); i++)
			{
				//	get and validate input class
				inputs[i].input.attach(hComponent, i);

				//	get structure
				const numeric::Structure* s = inputs[i].input.getStructure();
				Dims idims = s->dims;

				//	first input defines dims and type
				if (!i)
				{
					dims = s->dims;
					type = s->type;

					//	check
					if (dim >= dims.size())
						berr << "concatenation dimension out of range in first input";

					//	number of blocks is equal to the dimensions from "dim" on, so that
					//	blockcount * bytesperblock is the number of bytes at each input
					blockcount = 1;
					for (UINT32 d=dim+1; d<dims.size(); d++) blockcount *= dims[d];
				}

				//	other inputs contribute to output dimension in the "dim" index
				else
				{
					dims[dim] += idims[dim];
				}

				//	validate numeric type
				if ((s->type & TYPE_ARRAY_MASK) != (type & TYPE_ARRAY_MASK))
					berr << "all inputs must be of the same numeric type";

				//	must have right number of dims
				if (s->dims.count != dims.size())
					berr << "inputs must all have the same number of dimensions";

				//	all dims must match, except that specified by dim
				for (UINT32 d=0; d<dims.size(); d++)
				{
					//	validate dimension
					if (d != dim)
					{
						if (idims[d] != dims[d])
							berr << "inputs must match in size in all dimensions except the concatenation dimension";
					}
				}

				//	block size at input is equal to the block size of the dimensions
				//	up to and including the cat dimension
				UINT64 numEls = 1;
				for (UINT32 d=0; d<=dim; d++) numEls *= idims[d];
				inputs[i].bytesperblock = numEls * TYPE_BYTES(type);

				//	offset into output we can tot up as we go
				inputs[i].offset = offset;
				offset += inputs[i].bytesperblock;
			}

			//	stride at output is equal to the block size of the dimensions
			//	up to and including the cat dimension
			UINT64 numEls = 1;
			for (UINT32 d=0; d<=dim; d++) numEls *= dims[d];
			stride = numEls * TYPE_BYTES(type);

			//	create output
			output.setName("out");
			output.create(hComponent);
			output.setStructure(type, dims.cdims());

			//	ok
			return C_OK;
		}

		case EVENT_RUN_SERVICE:
		{
			//	access output
			BYTE* p_output = (BYTE*) output.getContent();

			//	for each input
			for (UINT32 i=0; i<inputs.size(); i++)
			{
				//	access input
				BYTE* p_input = (BYTE*) inputs[i].input.getContent();

				//	copy blocks into output
				for (UINT32 b=0; b<blockcount; b++)
				{
					memcpy(
						p_output + inputs[i].offset + b * stride,
						p_input + b * inputs[i].bytesperblock,
						inputs[i].bytesperblock
					);
				}
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
