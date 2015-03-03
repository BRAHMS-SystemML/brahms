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




////////////////	COMPONENT INFO

//	component information
#define COMPONENT_CLASS_STRING "dev/std/index/numeric"
#define COMPONENT_CLASS_CPP dev_std_index_numeric_0
#define COMPONENT_FLAGS (F_NOT_RATE_CHANGER | F_NEEDS_ALL_INPUTS)

//	include common header
#include "components/process.h"



////////////////	COMPONENT CLASS

enum DimType
{
	DT_NORMAL = 0,
	DT_ALL,
	DT_SQUEEZE
};

struct Input
{
	Symbol hInput;

	UINT64 bytesperblock; // number of input bytes that belong in a contiguous block in output
	UINT64 offset; // offset into output of first block's destination
};

class COMPONENT_CLASS_CPP : public Process
{

public:

	//	framework event function
	Symbol event(Event* event);

private:

	//	input
	numeric::Input input;

	//	output
	numeric::Output output;

	//	pars
	vector< vector<UINT32> > index;
	vector<DimType> dimType;

	//	state
	Dims dims;
	Dims inputDims;
	TYPE inputType;

	//	pre-calculated
	vector<UINT32> src;

};



////////////////	DO INDEX TEMPLATE

template <class T> void doIndex(T* output, T* input, const VUINT32& src)
{
	for (UINT32 i=0; i<src.size(); i++)
		output[i] = input[src[i]];
}

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

			//	index
			DataMLNode nodeIndices = nodeState.getField("index").validate(TYPE_CELL, Dims(1, DIM_ANY));
			UINT32 N = nodeIndices.getNumberOfElementsReal();
			for (UINT32 n=0; n<N; n++)
			{
				DataMLNode nodeIndex = nodeIndices.getCell(n);
				VUINT32 range;

				switch(nodeIndex.getElementType())
				{
					case TYPE_CHAR16:
					{
						//	":" means "all"
						string s = nodeIndex.getSTRING();
						if (s != ":")
							berr << "string entry in \"index\" must be \":\"";
						dimType.push_back(DT_ALL);
						break;
					}

					case TYPE_CELL:
					{
						//	cell wrapping indicates that dim should be "squeezed" from output
						nodeIndex = nodeIndex.validate(Dims(1)).getCell(0);
						range.push_back(nodeIndex.getUINT32());
						dimType.push_back(DT_SQUEEZE);
						break;
					}

					default:
					{
						range = nodeIndex.getArrayUINT32();
						dimType.push_back(DT_NORMAL);
						break;
					}
				}

				index.push_back(range);
				dims.push_back(range.size());
			}

			//	ok
			return C_OK;
		}

		case EVENT_INIT_PRECONNECT:
		{
			//	validate input count
			if (iif.getNumberOfPorts() != 1)
				berr << "expects exactly one input";

			//	ok
			return C_OK;
		}

		case EVENT_INIT_CONNECT:
		{
			//	access input
			input.attach(hComponent, 0);

			//	get structure
			const numeric::Structure* s = input.getStructure();
			inputDims = s->dims;
			inputType = s->typeElement;

			//	check we got the right number of indices
			if (inputDims.size() != index.size())
				berr << "\"index\" must specify indexing for as many dimensions as the input has (" << inputDims.size() << ")";

			//	handle implicit indexing
			for (UINT32 d=0; d<index.size(); d++)
			{
				switch(dimType[d])
				{
					case DT_ALL:
						//	index[d] will be empty
						for (UINT32 i=1; i<=inputDims[d]; i++)
							index[d].push_back(i);
						dims[d] = inputDims[d];
						break;

					default:
					{
						//	no action
					}
				}
			}

			//	validate index ranges
			for (UINT32 d=0; d<index.size(); d++)
			{
				for (UINT32 i=0; i<index[d].size(); i++)
				{
					UINT32 n = index[d][i];
					if (n < 1 || n > inputDims[d])
						berr << "\"index\" out of range";
				}
			}

			//	squeeze dims
			Dims squeezedDims;
			for (UINT32 d=0; d<index.size(); d++)
			{
				if (dimType[d] == DT_SQUEEZE)
				{
					if (dims[d] != 1)
						berr << E_INTERNAL << "squeezed dimension was not scalar";
				}

				else
				{
					squeezedDims.push_back(dims[d]);
				}
			}

			//	create output
			output.setName(input.getName());
			output.create(hComponent);
			output.setStructure(s->type, squeezedDims.cdims());

			//	pre-calculate src indices (dst indices are contiguous)
			if (dims.getNumberOfElements())
			{
				//	calculate strides
				VUINT32 strides;
				UINT32 stride = 1;
				for (UINT32 d=0; d<dims.size(); d++)
				{
					strides.push_back(stride);
					stride *= inputDims[d];
				}

				Dims ind;
				for (UINT32 d=0; d<dims.size(); d++)
					ind.push_back(0);

				while (true)
				{
					//	calculate src index
					UINT32 i = 0;
					for (UINT32 d=0; d<dims.size(); d++)
						i += (index[d][ind[d]] - 1) * strides[d]; // convert one-based to zero-based before use
					src.push_back(i);

					//	advance
					bool done = false;
					for (UINT32 d=0; d<dims.size(); d++)
					{
						ind[d]++;
						if (ind[d] < ((INT32)index[d].size()))
							break;
						ind[d] = 0;
						if (d == (dims.size() - 1))
							done = true;
					}

					//	if finished, break
					if (done) break;
				}
			}

			//	ok
			return C_OK;
		}

		case EVENT_RUN_SERVICE:
		{
			//	access output
			void* p_output = output.getContent();

			//	access input
			const void* p_input = input.getContent();

			//	call inline template function
			switch(inputType)
			{
				case TYPE_DOUBLE:
					doIndex((DOUBLE*)p_output, (DOUBLE*)p_input, src);
					break;

				case TYPE_SINGLE:
					doIndex((SINGLE*)p_output, (SINGLE*)p_input, src);
					break;

				case TYPE_UINT64:
					doIndex((UINT64*)p_output, (UINT64*)p_input, src);
					break;

				case TYPE_UINT32:
					doIndex((UINT32*)p_output, (UINT32*)p_input, src);
					break;

				case TYPE_UINT16:
					doIndex((UINT16*)p_output, (UINT16*)p_input, src);
					break;

				case TYPE_UINT8:
					doIndex((UINT8*)p_output, (UINT8*)p_input, src);
					break;

				case TYPE_INT64:
					doIndex((INT64*)p_output, (INT64*)p_input, src);
					break;

				case TYPE_INT32:
					doIndex((INT32*)p_output, (INT32*)p_input, src);
					break;

				case TYPE_INT16:
					doIndex((INT16*)p_output, (INT16*)p_input, src);
					break;

				case TYPE_INT8:
					doIndex((INT8*)p_output, (INT8*)p_input, src);
					break;

				default:
					berr << "unhandled numeric type " << getElementTypeString(inputType);
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
