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

	$Id:: source_numeric.cpp 2428 2009-12-11 15:10:37Z benjmit#$
	$Rev:: 2428                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-12-11 15:10:37 +0000 (Fri, 11 Dec 2009)       $
________________________________________________________________

*/



////////////////	COMPONENT INFO

//	component information
#define COMPONENT_CLASS_STRING "std/2009/source/numeric"
#define COMPONENT_CLASS_CPP std_2009_source_numeric_0
#define COMPONENT_FLAGS F_NOT_RATE_CHANGER

//	include common header
#include "components/process.h"



////////////////	COMPONENT CLASS (DERIVES FROM Process)

class COMPONENT_CLASS_CPP : public Process
{

public:

	//	use ctor/dtor if required
	COMPONENT_CLASS_CPP();
	~COMPONENT_CLASS_CPP();

	//	framework event function
	Symbol event(Event* event);

private:

	/*

		Private member data of the process goes here.
		You can use this to store your parameters
		and process state between calls to event().

	*/

	//	source data structure
	struct SOURCE_STRUCTURE
	{
		SOURCE_STRUCTURE()
		{
			numericType = TYPE_UNSPECIFIED;
			complexity = false;
			numSamples = 0;

			bytesPerRealElement = 0;
			realElementsPerSample = 0;
			realBytesPerSample = 0;
			totalBytesPerSample = 0;
		}

		//	measured...
		TYPE				numericType;			//	numeric type
		bool				complexity;				//	true if source data is complex
		Dims				dims;					//	dimension (not including last dimension, time)
		INT64				numSamples;				//	available samples (i.e. last dimension, time)
		string				filename;				//	if in use

		//	inferred...
		UINT64				bytesPerRealElement;	//	size in bytes of a single (real) data element
		UINT64				realElementsPerSample;	//	number of elements per sample per channel (real/imag)
		UINT64				realBytesPerSample;		//	number of bytes per sample per channel (real/imag)
		UINT64				totalBytesPerSample;	//	total number of bytes per sample
		UINT64				totalBytesReal;
	}
	src;

	//	source use structure
	struct SOURCE_USE
	{
		SOURCE_USE()
		{
			start = 0;
			stop = 0;
			next = 0;
		}

		INT64				start;				//	first sample to use (defaults to zero)
		INT64				stop;				//	last sample to use plus one (defaults to N)
		INT64				next;				//	next sample to deliver
	}
	use;

	//	source data
	struct SOURCE_DATA
	{
		SOURCE_DATA()
		{
			fid = NULL;
			sourceIsAdjacent = false;
			real = NULL;
			imag = NULL;
		}

		//	if reading from a file, this is non-NULL (and the remaining members of this structure are NULL)
		FILE*				fid;

		//	if file is complex and in "adjacent" format, this is true
		bool				sourceIsAdjacent;

		//	if reading from our StateML, this is where we buffer the data...
		VBYTE				buffer;

		//	...and these pointers point at the start of the real and imag parts of the data in that buffer
		BYTE*				real;					//	will always be non-zero
		BYTE*				imag;					//	will be zero if source data is not complex
	}
	dat;

	//	behaviour flags
	struct BEHAVIOUR_FLAGS
	{
		BEHAVIOUR_FLAGS()
		{
			repeat = false;
			complexOutput = false;
		}

		//	flags
		bool					repeat;				//	true if we should repeat when we run out of data
		bool					complexOutput;		//	complexity of our output (same as source data, unless specified explicitly)
	}
	flags;

#ifdef SOURCE_NUMERIC_TIMING
	INT64 t1, t2;
#endif


	//	output accessor
	numeric::Output output;
	string outputName;
};






////////////////	'STRUCTORS

COMPONENT_CLASS_CPP::COMPONENT_CLASS_CPP()
{

#ifdef SOURCE_NUMERIC_TIMING
	t1 = 0;
	t2 = 0;
#endif
}

COMPONENT_CLASS_CPP::~COMPONENT_CLASS_CPP()
{
	if (dat.fid) fclose(dat.fid);
}

TYPE typeFromString(const string& typeString)
{
	if (typeString == "DOUBLE") return TYPE_DOUBLE;
	if (typeString == "SINGLE") return TYPE_SINGLE;
	if (typeString == "INT8") return TYPE_INT8;
	if (typeString == "INT16") return TYPE_INT16;
	if (typeString == "INT32") return TYPE_INT32;
	if (typeString == "INT64") return TYPE_INT64;
	if (typeString == "UINT8") return TYPE_UINT8;
	if (typeString == "UINT16") return TYPE_UINT16;
	if (typeString == "UINT32") return TYPE_UINT32;
	if (typeString == "UINT64") return TYPE_UINT64;
	if (typeString == "BOOL8") return TYPE_BOOL8;
	berr << "invalid type string \"" << typeString << "\"";
	return TYPE_UNSPECIFIED;
}

string n2s(UINT32 n)
{
	ostringstream s;
	s << n;
	return s.str();
}



////////////////	OPERATION METHODS

/*
inline bool TYPE_IS_FIXED(TYPE type)
{
	return (type & TYPE_FORMAT_MASK) == TYPE_FORMAT_FIXED;
}
*/

inline bool TYPE_IS_BOOLNUM(TYPE type)
{
	return (type & TYPE_FORMAT_MASK) && (type & TYPE_FORMAT_MASK) <= TYPE_FORMAT_BOOL;
}

#ifdef SOURCE_NUMERIC_TIMING
	//	RDTSC instruction for performance testing
	inline INT64 rdtsc()
	{
#ifdef __WIN__
		__asm
		{
			_emit 0x0F
			_emit 0x31
		}
#endif

#ifdef __NIX__
		INT64 x;
		__asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
		return x;
#endif
	}
#endif

Symbol COMPONENT_CLASS_CPP::event(Event* event)
{



	switch(event->type)
	{

		case EVENT_RUN_SERVICE:
		{
#ifdef SOURCE_NUMERIC_TIMING
			INT64 ta = rdtsc();
#endif

			//	check if we've run out of data
			if (use.next == use.stop)
			{
				if (!flags.repeat)
					berr << "source data has run out (set flag \"repeat\" to use data more than once)";

				//	reset counter
				use.next = use.start;
				if (use.next == use.stop) berr << "source data is empty (no samples supplied)";

				//	if reading from a file, reset the file pointer
				if (dat.fid)
					fseek(dat.fid, use.start * src.totalBytesPerSample, SEEK_SET);
			}

#ifdef SOURCE_NUMERIC_TIMING
			INT64 tb = rdtsc();
			t1 += (tb - ta);
#endif

			//	pointer to output data
			char* p_output = (char*) output.getContent();



////////////////	(FILE MODE) FILL OUTPUT

			#define FSEEK(p) { if (fseek(dat.fid, p, SEEK_SET)) berr << "failed fseek() on \"" << src.filename << "\""; }
			#define FADVANCE(b) { if (fseek(dat.fid, b, SEEK_CUR)) berr << "failed fseek() on \"" << src.filename << "\""; }
			#define FREAD(p, b) { if (fread(p, 1, b, dat.fid) != b) berr << "failed read on \"" << src.filename << "\""; }

			if (dat.fid)
			{
				//	read data from file
				if (dat.sourceIsAdjacent)
				{
					//	since we're having to jump about in the file anyway, we
					//	just ignore the current file pointer and set it explicitly
					//	on each cycle

					//	always write real
					FSEEK( use.next * src.realBytesPerSample );
					FREAD( p_output, src.realBytesPerSample );

					//	write imag if present in source and output
					if (src.complexity && flags.complexOutput)
					{
						FSEEK( src.totalBytesReal + use.next * src.realBytesPerSample );
						FREAD( p_output + src.realBytesPerSample, src.realBytesPerSample );
					}
				}

				else
				{
					//	always write real
					//	write imag if present in source and output
					if (src.complexity && flags.complexOutput)
					{
						//	no seek required, file pointer is kept up-to-date in interleaved mode
						FREAD( p_output, src.totalBytesPerSample );
					}

					else
					{
						//	no seek required, file pointer is kept up-to-date in interleaved mode
						FREAD( p_output, src.realBytesPerSample );

						//	if complex source, but we're not using it
						if (src.complexity)
						{
							//	must advance over the imaginary data that we're not using
							FADVANCE( src.realBytesPerSample );
						}
					}
				}
			}



////////////////	(BUFFER MODE) FILL OUTPUT

			else
			{
				//	always write real
				memcpy(p_output, dat.real + use.next * src.realBytesPerSample, src.realBytesPerSample);

				//	write imag if present in source and output
				if (src.complexity && flags.complexOutput)
					memcpy(p_output + src.realBytesPerSample, dat.imag + use.next * src.realBytesPerSample, src.realBytesPerSample);
			}



////////////////	(ALL MODES) ZERO IMAGINARY OUTPUT, IF NO IMAGINARY SOURCE

			//	if complex output, but non-complex source, must zero imaginary output
			if (flags.complexOutput && !src.complexity)
			{
				memset(p_output + src.realBytesPerSample, 0, src.realBytesPerSample);
			}



////////////////	ADVANCE

			//	increment counter
			use.next++;

#ifdef SOURCE_NUMERIC_TIMING
			INT64 tc = rdtsc();
			t2 += tc - tb;
#endif

			//	ok
			return C_OK;
		}



		case EVENT_STATE_SET:
		{
			//	extract DataML
			EventStateSet* data = (EventStateSet*) event->data;
			XMLNode xmlNode(data->state);
			DataMLNode nodeState(&xmlNode);

			//	get the type
			DataMLNode nodeData = nodeState.getField("data");
			TYPE nodeDataType = nodeData.getElementType();

			//	handle explicit numeric data
			if (TYPE_IS_BOOLNUM(nodeDataType))
			{
				src.numericType = nodeData.getElementType();
				src.bytesPerRealElement = nodeData.getBytesPerElement();
				src.dims = nodeData.getDims();
				src.complexity = nodeData.getComplexity();

				//	if data is not empty
				src.totalBytesReal = nodeData.getNumberOfBytesReal();
				if (src.totalBytesReal)
				{
					//	resize buffer
					UINT64 totalBytes = nodeData.getNumberOfBytesTotal();
					dat.buffer.resize(totalBytes);

					//	handle complex
					if (src.complexity)
					{
						dat.real = &dat.buffer[0];
						dat.imag = &dat.buffer[src.totalBytesReal];
						nodeData.getRaw(dat.real, dat.imag);
					}

					//	handle real
					else
					{
						dat.real = &dat.buffer[0];
						nodeData.getRaw(dat.real);
					}
				}

				//	if data is empty
				else
				{
					//	buffer remains of zero size, dat.real and dat.imag remain NULL
				}
			}

			//	handle binary file
			if (nodeDataType == TYPE_CHAR16)
			{
				string stype = nodeState.getField("type").getSTRING();
				src.numericType = typeFromString(stype);
				src.bytesPerRealElement = TYPE_BYTES(src.numericType);
				src.dims = nodeState.getField("dims").getArrayDims();

				//	open file
				src.filename = nodeData.getSTRING();
				dat.fid = fopen(src.filename.c_str(), "rb");
				if (!dat.fid) berr << "failed open \"" << src.filename << "\"";

				//	check its size (and infer complexity)
				if (fseek(dat.fid, 0, SEEK_END))
					berr << "failed fseek() on \"" << src.filename << "\"";
				UINT64 bytes = ftell(dat.fid);
				if (fseek(dat.fid, 0, SEEK_SET))
					berr << "failed fseek() on \"" << src.filename << "\"";
				src.totalBytesReal = src.dims.getNumberOfElements() * src.bytesPerRealElement;
				if (bytes == src.totalBytesReal) src.complexity = false;
				else if (bytes == (src.totalBytesReal * 2)) src.complexity = true;
				else berr << "unexpected file size \"" << src.filename << "\" (expected " << src.totalBytesReal << " or " << (src.totalBytesReal*2) << " bytes)";

				//	if complex, check for adjacent/interleaved
				if (src.complexity)
				{
					if (nodeState.hasField("sourceIsAdjacent"))
						dat.sourceIsAdjacent = nodeState.getField("sourceIsAdjacent").getBOOL();
					//	else leave format as "interleaved"
				}
			}

			//	validate data type
			if (src.numericType == TYPE_UNSPECIFIED)
				berr << "unsupported data type (nodeDataType == " << hex << nodeDataType << ")";

			//	check for explicit [number of dimensions in output stream] declaration
			//	and expand dims if necessary
			if (nodeState.hasField("ndims"))
			{
				UINT32 ndims = nodeState.getField("ndims").getUINT32();
				if (ndims > 100) berr << "more than 100 dimensions is a bit silly";
				if (src.dims.size() > (ndims + 1)) berr << "cannot specify ndims less than [dimension count of source data minus one]";
				while(src.dims.size() < (ndims+1)) src.dims.push_back(1);
			}

			//	dims must now have 2 dimensions at least (output size plus sample number)
			switch (src.dims.size())
			{
				case 0:
				{
					//	i'm not sure why this would ever happen, but we can't use this data
					//	anyway, so let's throw an error to be on the safe side
					berr << "input dimension is zero - please report this case to author";
					break;
				}

				case 1:
				{
					//	this is a common case, so let's assume the missing trailing dimension is
					//	unity, and emit a warning.
					src.dims.push_back(1);
					bout << "input data is only 1D - assuming trailing dimension (sample number) of unity" << D_VERB;
					break;
				}
			}

			//	peel off last dimension (sample) from dims
			src.numSamples = src.dims[src.dims.size() - 1];
			src.dims.resize(src.dims.size() - 1);

			//	calculate some stuff we'll need
			src.realElementsPerSample = 1;
			for (UINT32 d=0; d<src.dims.size(); d++) src.realElementsPerSample *= src.dims[d];
			if (!src.dims.size()) src.realElementsPerSample = 0;
			src.realBytesPerSample = src.realElementsPerSample * src.bytesPerRealElement;
			src.totalBytesPerSample = src.realBytesPerSample * (src.complexity ? 2 : 1);

			//	set start/stop
			use.start = 0;
			if (nodeState.hasField("start"))
				use.start = nodeState.getField("start").getUINT32();
			use.stop = src.numSamples;
			if (nodeState.hasField("stop"))
				use.stop = nodeState.getField("stop").getUINT32();
			if (use.stop > src.numSamples)
				berr << "invalid value for \"stop\" (out of range)";
			if (use.start >= use.stop)
				berr << "invalid values for \"start\" and \"stop\" (invalid range)";

			//	set next
			use.next = use.start; // default, at start
			if (nodeState.hasField("next"))
				use.next = nodeState.getField("next").getUINT32();
			if (use.next < use.start || use.next >= use.stop)
				berr << "invalid value for \"next\" (out of range)";

			//	if using a file, seek to sampleIn
			if (dat.fid)
				fseek(dat.fid, use.next * src.totalBytesPerSample, SEEK_SET);

			//	repeat flag
			if (nodeState.hasField("repeat"))
				flags.repeat = nodeState.getField("repeat").getBOOL();

			//	complex flag
			if (nodeState.hasField("complex"))
				flags.complexOutput = nodeState.getField("complex").getBOOL();
			else flags.complexOutput = src.complexity; // default

			//	we will generate a complex output if "complexity" is true, but
			//	we will only fill it if the source data is also complex!
			if (src.complexity && !flags.complexOutput)
				bout << "source/numeric: not using imaginary part of source data (output complexity set explicitly to false)" << D_VERB;

			//	output name
			if (nodeState.hasField("outputName"))
				outputName = nodeState.getField("outputName").getSTRING();
			else outputName = "out";

			//	ok
			return C_OK;
		}

		case EVENT_STATE_GET:
		{
			EventStateGet* esg = (EventStateGet*)event->data;

			//	get existing state node
			XMLNode xmlNode(esg->state);

			//	wrap it in a DataMLNode interface
			DataMLNode state(&xmlNode);
			state.precision(esg->precision);

			//	get field
			DataMLNode nodeNext;
			if (state.hasField("next"))
				nodeNext = state.getField("next");
			else
				nodeNext = state.addField("next");

			//	store field
			VUINT32 vnext;
			vnext.push_back(use.next);
			nodeNext.setArray(Dims(1), vnext);

			//	ok
			return C_OK;
		}

		case EVENT_INIT_CONNECT:
		{
			//	make sure we got no inputs
			if (iif.getNumberOfPorts())
				berr << "expects no inputs";

			//	instantiate output, and initialise it
			output.setName(outputName.c_str());
			output.create(hComponent);
			output.setStructure(src.numericType | (flags.complexOutput ? TYPE_COMPLEX : TYPE_REAL), src.dims.cdims());

			//	ok
			return C_OK;
		}

#ifdef SOURCE_NUMERIC_TIMING
		case EVENT_RUN_STOP:
		{
			bout << "t1: " << t1 << D_INFO;
			bout << "t2: " << t2 << D_INFO;
		}
#endif

	}


	//	not serviced
	return S_NULL;


}



#include "brahms-1199.h"
