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

	$Id:: math_eproduct.cpp 2428 2009-12-11 15:10:37Z benjmitc#$
	$Rev:: 2428                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-12-11 15:10:37 +0000 (Fri, 11 Dec 2009)       $
________________________________________________________________

*/



////////////////	COMPONENT INFO

//	component information
#define COMPONENT_CLASS_STRING "std/2009/math/eproduct"
#define COMPONENT_CLASS_CPP std_2009_math_eproduct_0
#define COMPONENT_FLAGS F_NOT_RATE_CHANGER

//	include common header
#include "components/process.h"




////////////////	COMPONENT CLASS (DERIVES FROM Process)

class COMPONENT_CLASS_CPP : public Process
{

public:

	//	use ctor/dtor if required
	COMPONENT_CLASS_CPP();

	//	framework event function
	Symbol event(Event* event);

private:

	/*

		Private member data of the process goes here.
		You can use this to store your parameters
		and process state between calls to event().

	*/

	//	everything we know about the output
	UINT32					dataType;		//	data type (if not TYPE_NULL, is defined)

	bool					complex;		//	complexity flag (if false, not yet sure)
	bool					complexDefined;	//	true if complexity already defined

	Dims					dims;			//	dimensions
	bool					dimsDefined;	//	true if dimensions defined

	UINT32					numEls;			//	number of elements

	//	output buffer
	struct
	{
		VBYTE buffer;
		void* real;
		void* imag;
	}
	outputBuffer;

	//	handles
	vector<numeric::Input> inputs;
	numeric::Output output;
	string outputName;
};

COMPONENT_CLASS_CPP::COMPONENT_CLASS_CPP()
{
	dataType = TYPE_UNSPECIFIED;
	dimsDefined = false;
	complex = false;
	complexDefined = false;
	numEls = 0;
	outputBuffer.real = 0;
	outputBuffer.imag = 0;
}





////////////////	HELPERS

template <class T> void setUnity(T* real, T* imag, UINT32 N)
{
	for (UINT32 n=0; n<N; n++)
		real[n] = 1.0;

	if (imag)
		for (UINT32 n=0; n<N; n++)
			imag[n] = 0.0;
}

template <class T> void setValue(T* outReal, T* outImag, T* inReal, T* inImag, UINT32 no, UINT32 ni)
{
	//	scalar
	if (ni == 1)
	{
		//	complex output
		if (outImag)
		{
			//	complex input
			if (inImag)
			{
				for (UINT32 n=0; n<no; n++)
				{
					outReal[n] = inReal[0];
					outImag[n] = inImag[0];
				}
			}

			//	real input
			else
			{
				for (UINT32 n=0; n<no; n++)
				{
					outReal[n] = inReal[0];
					outImag[n] = 0.0;
				}
			}
		}

		//	real output
		else
		{
			//	complex input (this case can't happen)

			//	real input
			for (UINT32 n=0; n<no; n++)
				outReal[n] = inReal[0];
		}
	}

	//	non-scalar
	else
	{
		//	complex output
		if (outImag)
		{
			//	complex input
			if (inImag)
			{
				for (UINT32 n=0; n<no; n++)
				{
					outReal[n] = inReal[n];
					outImag[n] = inImag[n];
				}
			}

			//	real input
			else
			{
				for (UINT32 n=0; n<no; n++)
				{
					outReal[n] = inReal[n];
					outImag[n] = 0.0;
				}
			}
		}

		//	real output
		else
		{
			//	complex input (this case can't happen)

			//	real input
			for (UINT32 n=0; n<no; n++)
				outReal[n] = inReal[n];
		}
	}
}

template <class T> void mulValue(T* outReal, T* outImag, T* inReal, T* inImag, UINT32 no, UINT32 ni)
{
	//	scalar
	if (ni == 1)
	{
		//	complex output
		if (outImag)
		{
			//	complex input
			if (inImag)
			{
				for (UINT32 n=0; n<no; n++)
				{
					DOUBLE r = outReal[n] * inReal[0] - outImag[n] * inImag[0];
					DOUBLE i = outReal[n] * inImag[0] + outImag[n] * inReal[0];
					outReal[n] = r;
					outImag[n] = i;
				}
			}

			//	real input
			else
			{
				for (UINT32 n=0; n<no; n++)
				{
					outReal[n] *= inReal[0];
					outImag[n] *= inReal[0];
				}
			}
		}

		//	real output
		else
		{
			//	complex input (this case can't happen)

			//	real input
			for (UINT32 n=0; n<no; n++)
				outReal[n] *= inReal[0];
		}
	}

	//	non-scalar
	else
	{
		//	complex output
		if (outImag)
		{
			//	complex input
			if (inImag)
			{
				for (UINT32 n=0; n<no; n++)
				{
					DOUBLE r = outReal[n] * inReal[n] - outImag[n] * inImag[n];
					DOUBLE i = outReal[n] * inImag[n] + outImag[n] * inReal[n];
					outReal[n] = r;
					outImag[n] = i;
				}
			}

			//	real input
			else
			{
				for (UINT32 n=0; n<no; n++)
				{
					outReal[n] *= inReal[n];
					outImag[n] *= inReal[n];
				}
			}
		}

		//	real output
		else
		{
			//	complex input (this case can't happen)

			//	real input
			for (UINT32 n=0; n<no; n++)
				outReal[n] *= inReal[n];
		}
	}
}




////////////////	EVENT

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

			//	extract complex flag if supplied
			if (nodeState.hasField("complex"))
			{
				complex = nodeState.getField("complex").getBOOL();
				complexDefined = true;
			}

			//	extract dims if supplied
			if (nodeState.hasField("dims"))
			{
				dims = nodeState.getField("dims").getArrayDims();
				dimsDefined = true;
			}

			//	extract data type if supplied
			if (nodeState.hasField("type"))
			{
				string type = nodeState.getField("type").getSTRING();
				if (type == "DOUBLE") dataType = TYPE_DOUBLE;
				else if (type == "SINGLE") dataType = TYPE_SINGLE;
				else if (type == "FLOAT64") dataType = TYPE_FLOAT64;
				else if (type == "FLOAT32") dataType = TYPE_FLOAT32;
				else if (type == "UINT64") dataType = TYPE_UINT64;
				else if (type == "UINT32") dataType = TYPE_UINT32;
				else if (type == "UINT16") dataType = TYPE_UINT16;
				else if (type == "UINT8") dataType = TYPE_UINT8;
				else if (type == "INT64") dataType = TYPE_INT64;
				else if (type == "INT32") dataType = TYPE_INT32;
				else if (type == "INT16") dataType = TYPE_INT16;
				else if (type == "INT8") dataType = TYPE_INT8;
				else if (type == "BOOL64") dataType = TYPE_BOOL64;
				else if (type == "BOOL32") dataType = TYPE_BOOL32;
				else if (type == "BOOL16") dataType = TYPE_BOOL16;
				else if (type == "BOOL8") dataType = TYPE_BOOL8;
				else berr << "unrecognised specified type \"" + type + "\"";
			}

			//	output name
			if (nodeState.hasField("outputName"))
				outputName = nodeState.getField("outputName").getSTRING();
			else outputName = "out";

			//	ok
			return C_OK;
		}

		case EVENT_INIT_PRECONNECT:
		{
			//	if no inputs, output must be fully defined
			if (!iif.getNumberOfPorts())
			{
				if ((!complexDefined) || (!dimsDefined) || (dataType == TYPE_UNSPECIFIED))
					berr << "with no inputs, output must be specified completely (numeric type, complexity, and dimensions)";
			}

			//	attach inputs
			inputs.resize(iif.getNumberOfPorts());
			for (UINT32 i=0; i<iif.getNumberOfPorts(); i++)
				inputs[i].attach(hComponent, i);

			//	ok
			return C_OK;
		}

		case EVENT_INIT_CONNECT:
		{

////////////////	INFER OUTPUT STRUCTURE FROM INPUTS

			//	seen all inputs?
			bool seenAll = true;

			//	initialise only by examining inputs
			for (UINT32 i=0; i<iif.getNumberOfPorts(); i++)
			{
				//	if already handled, skip
				if (inputs[i].tag) continue;

				//	if not due, skip
				if (!inputs[i].due())
				{
					seenAll = false;
					continue;
				}

				//	due, so mark as seen
				inputs[i].tag = 1;

				//	validate type and get structure
				const numeric::Structure* structure = inputs[i].getStructure();

				//	input must match or define the data type
				if (dataType == TYPE_UNSPECIFIED)
				{
					//	define
					dataType = structure->typeElement;
				}
				else
				{
					//	already defined, must match
					inputs[i].validateStructure(dataType);
				}

				//	complexity must not exceed a complexity defined as "not complex" (this is only possible in the parameters)
				if (complexDefined && !complex && structure->complex) berr << "complex input precluded by parameters";

				//	interleaved unsupported
				if ((structure->type & TYPE_CPXFMT_MASK) == TYPE_CPXFMT_INTERLEAVED) berr << "complex interleaved input not currently supported";

				//	and any single complex input sets the output complexity to complex
//				bout << "port " << i << " comp " << hex << structure->typeComplexity << ", " << UINT32(structure->complex) << D_INFO;
				complex = complex || structure->complex;

				//	if this input is scalar, ignore it as far as dims are concerned
				if (structure->scalar) continue;

				//	input must match or define the dimensions
				Dims inputDims = structure->dims;
				if (!dimsDefined)
				{
					//	define dims
					dims = inputDims;
					dimsDefined = true;
				}

				else
				{
					//	match dims
					if (dims.size() != inputDims.size()) berr << "input sizes are not commensurate";
					for (UINT32 d=0; d<dims.size(); d++)
						if (dims[d] != inputDims[d]) berr << "input sizes are not commensurate";
				}
			}



////////////////	OTHER INFERENCES

			//	if all inputs were scalars, that defines dims as scalar
			if (seenAll && !dimsDefined)
			{
				//	define dims as scalar
				dims = Dims(1);
				dimsDefined = true;
			}

			//	if we've seen all inputs, or complex has been set, complexity is defined
			if (seenAll || complex)
			{
				complexDefined = true;
			}



////////////////	SOON AS READY, CREATE OUTPUT

			//	if data type and dims are defined, and complexity is true or we have seen all inputs, we
			//	can create the output (if we haven't done so already)
			if ((dataType != TYPE_UNSPECIFIED) && dimsDefined && complexDefined)
			{
				if (!outputBuffer.real)
				{
					//	create output
					output.setName(outputName.c_str());
					output.create(hComponent);
					output.setStructure(dataType | (complex ? TYPE_COMPLEX : TYPE_REAL), dims.cdims());

					//	get structure
					const numeric::Structure* structure = output.getStructure();

					//	set number of elements
					numEls = structure->numberOfElementsReal;

					//	resize output workspace
					outputBuffer.buffer.resize(structure->numberOfBytesTotal);

					//	get pointers to it
					outputBuffer.real = &outputBuffer.buffer[0];
					if (complex) outputBuffer.imag = &outputBuffer.buffer[structure->numberOfBytesReal];
				}
			}

			//	ok
			return C_OK;
		}

		case EVENT_INIT_POSTCONNECT:
		{
			//	check
			if (!outputBuffer.real) berr << E_INTERNAL << "output was not created";

			//	ok
			return C_OK;
		}

		case EVENT_RUN_SERVICE:
		{
			struct
			{
				const void* real;
				const void* imag;
			}
			input;

			const numeric::Structure* structure;



////////////////	IF NO INPUTS, OUTPUT IS REAL UNITY

			//	if no inputs
			if (!inputs.size())
			{
				//	switch on type
				switch(dataType)
				{
					case TYPE_DOUBLE:
						setUnity((DOUBLE*)outputBuffer.real, (DOUBLE*)outputBuffer.imag, numEls);
						break;
					case TYPE_SINGLE:
						setUnity((SINGLE*)outputBuffer.real, (SINGLE*)outputBuffer.imag, numEls);
						break;
					case TYPE_UINT64:
						setUnity((UINT64*)outputBuffer.real, (UINT64*)outputBuffer.imag, numEls);
						break;
					case TYPE_UINT32:
						setUnity((UINT32*)outputBuffer.real, (UINT32*)outputBuffer.imag, numEls);
						break;
					case TYPE_UINT16:
						setUnity((UINT16*)outputBuffer.real, (UINT16*)outputBuffer.imag, numEls);
						break;
					case TYPE_UINT8:
						setUnity((UINT8*)outputBuffer.real, (UINT8*)outputBuffer.imag, numEls);
						break;
					case TYPE_INT64:
						setUnity((INT64*)outputBuffer.real, (INT64*)outputBuffer.imag, numEls);
						break;
					case TYPE_INT32:
						setUnity((INT32*)outputBuffer.real, (INT32*)outputBuffer.imag, numEls);
						break;
					case TYPE_INT16:
						setUnity((INT16*)outputBuffer.real, (INT16*)outputBuffer.imag, numEls);
						break;
					case TYPE_INT8:
						setUnity((INT8*)outputBuffer.real, (INT8*)outputBuffer.imag, numEls);
						break;
					default:
						berr << "not coded yet for this data type";
				}
			}



////////////////	IF SOME INPUTS, OUTPUT IS FIRST INPUT

			//	if some inputs
			else
			{
				//	access first input
				structure = inputs[0].getStructure();
				inputs[0].getContent(input.real, input.imag);
				UINT32 iNumEls = structure->numberOfElementsReal;

				//	switch on type
				switch(dataType)
				{
					case TYPE_DOUBLE:
						setValue((DOUBLE*)outputBuffer.real, (DOUBLE*)outputBuffer.imag, (DOUBLE*)input.real, (DOUBLE*)input.imag, numEls, iNumEls);
						break;
					case TYPE_SINGLE:
						setValue((SINGLE*)outputBuffer.real, (SINGLE*)outputBuffer.imag, (SINGLE*)input.real, (SINGLE*)input.imag, numEls, iNumEls);
						break;
					case TYPE_UINT64:
						setValue((UINT64*)outputBuffer.real, (UINT64*)outputBuffer.imag, (UINT64*)input.real, (UINT64*)input.imag, numEls, iNumEls);
						break;
					case TYPE_UINT32:
						setValue((UINT32*)outputBuffer.real, (UINT32*)outputBuffer.imag, (UINT32*)input.real, (UINT32*)input.imag, numEls, iNumEls);
						break;
					case TYPE_UINT16:
						setValue((UINT16*)outputBuffer.real, (UINT16*)outputBuffer.imag, (UINT16*)input.real, (UINT16*)input.imag, numEls, iNumEls);
						break;
					case TYPE_UINT8:
						setValue((UINT8*)outputBuffer.real, (UINT8*)outputBuffer.imag, (UINT8*)input.real, (UINT8*)input.imag, numEls, iNumEls);
						break;
					case TYPE_INT64:
						setValue((INT64*)outputBuffer.real, (INT64*)outputBuffer.imag, (INT64*)input.real, (INT64*)input.imag, numEls, iNumEls);
						break;
					case TYPE_INT32:
						setValue((INT32*)outputBuffer.real, (INT32*)outputBuffer.imag, (INT32*)input.real, (INT32*)input.imag, numEls, iNumEls);
						break;
					case TYPE_INT16:
						setValue((INT16*)outputBuffer.real, (INT16*)outputBuffer.imag, (INT16*)input.real, (INT16*)input.imag, numEls, iNumEls);
						break;
					case TYPE_INT8:
						setValue((INT8*)outputBuffer.real, (INT8*)outputBuffer.imag, (INT8*)input.real, (INT8*)input.imag, numEls, iNumEls);
						break;
					default:
						berr << "not coded yet for this data type";
				}
			}



////////////////	FOR ANY FURTHER INPUTS, OUTPUT IS MULTIPLIED BY THEM

			//	for each further input, product it to the output
			for (UINT32 i=1; i<inputs.size(); i++)
			{
				//	access further input
				structure = inputs[i].getStructure();
				inputs[i].getContent(input.real, input.imag);
				UINT32 iNumEls = structure->numberOfElementsReal;

				//	switch on type
				switch(dataType)
				{
					case TYPE_DOUBLE:
						mulValue((DOUBLE*)outputBuffer.real, (DOUBLE*)outputBuffer.imag, (DOUBLE*)input.real, (DOUBLE*)input.imag, numEls, iNumEls);
						break;
					case TYPE_SINGLE:
						mulValue((SINGLE*)outputBuffer.real, (SINGLE*)outputBuffer.imag, (SINGLE*)input.real, (SINGLE*)input.imag, numEls, iNumEls);
						break;
					case TYPE_UINT64:
						mulValue((UINT64*)outputBuffer.real, (UINT64*)outputBuffer.imag, (UINT64*)input.real, (UINT64*)input.imag, numEls, iNumEls);
						break;
					case TYPE_UINT32:
						mulValue((UINT32*)outputBuffer.real, (UINT32*)outputBuffer.imag, (UINT32*)input.real, (UINT32*)input.imag, numEls, iNumEls);
						break;
					case TYPE_UINT16:
						mulValue((UINT16*)outputBuffer.real, (UINT16*)outputBuffer.imag, (UINT16*)input.real, (UINT16*)input.imag, numEls, iNumEls);
						break;
					case TYPE_UINT8:
						mulValue((UINT8*)outputBuffer.real, (UINT8*)outputBuffer.imag, (UINT8*)input.real, (UINT8*)input.imag, numEls, iNumEls);
						break;
					case TYPE_INT64:
						mulValue((INT64*)outputBuffer.real, (INT64*)outputBuffer.imag, (INT64*)input.real, (INT64*)input.imag, numEls, iNumEls);
						break;
					case TYPE_INT32:
						mulValue((INT32*)outputBuffer.real, (INT32*)outputBuffer.imag, (INT32*)input.real, (INT32*)input.imag, numEls, iNumEls);
						break;
					case TYPE_INT16:
						mulValue((INT16*)outputBuffer.real, (INT16*)outputBuffer.imag, (INT16*)input.real, (INT16*)input.imag, numEls, iNumEls);
						break;
					case TYPE_INT8:
						mulValue((INT8*)outputBuffer.real, (INT8*)outputBuffer.imag, (INT8*)input.real, (INT8*)input.imag, numEls, iNumEls);
						break;
					default:
						berr << "not coded yet for this data type";
				}
			}



////////////////	SEND OUTPUT

			//	store output
			output.setContent(&outputBuffer.buffer[0]);

			//	ok
			return C_OK;
		}

	}

	//	if we service the event, we return C_OK
	//	if we don't, we should return S_NULL to indicate that
	return S_NULL;
}







//	include overlay (a second time)
#include "brahms-1199.h"
