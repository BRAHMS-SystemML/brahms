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

	$Id:: data_numeric.cpp 2437 2009-12-13 19:06:12Z benjmitch $
	$Rev:: 2437                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-12-13 19:06:12 +0000 (Sun, 13 Dec 2009)       $
________________________________________________________________

*/



////////////////	COMPONENT INFO

//	component information
#define COMPONENT_CLASS_STRING "std/2009/data/numeric"
#define COMPONENT_CLASS_CPP class__std_2009_data_numeric_0
#define COMPONENT_FLAGS (0)

//	common
#include "../data.h"
#include "mutex.cpp"

#include <iostream>
using namespace std;

//	include header
#include "std/2009/data/numeric/brahms/0/data.h"
namespace numeric = std_2009_data_numeric_0;



////////////////	LAYOUT CONVERSION BUFFER

struct ConversionBuffer
{
	VBYTE data;
	BYTE* p_data;
	bool assigned;
	bool converted;
	Mutex mutex;

	ConversionBuffer()
	{
		p_data = NULL;
		assigned = false;
		converted = false;
	}

	void assign(UINT32 bytes)
	{
		MutexLocker locker(mutex);
		if (!assigned)
		{
			data.resize(bytes);
			p_data = bytes ? &data[0] : NULL;
			assigned = true;
		}
	
//		cout << "assign()" << endl;
	}

	BYTE* get()
	{
//		cout << "get()" << endl;

		return p_data;
	}
};

struct ConversionBuffers
{
	ConversionBuffer ac;
	ConversionBuffer ar;
	ConversionBuffer ic;
	ConversionBuffer ir;

	void invalidate()
	{
		ac.converted = false;
		ar.converted = false;
		ic.converted = false;
		ir.converted = false;

//		cout << "invalidate()" << endl;
	}
};



////////////////	COMPONENT CLASS

class COMPONENT_CLASS_CPP : public Data
{



////////////////	'STRUCTORS

public:

	COMPONENT_CLASS_CPP();
	~COMPONENT_CLASS_CPP();
	COMPONENT_CLASS_CPP(const COMPONENT_CLASS_CPP& src);



////////////////	OVERRIDES


	Symbol event(Event* event);



////////////////	INTERFACE

public:

	//	source init
	void						setStructure(const numeric::Structure* structure);

	//	sink validate
	void						validate(TYPE type, Dims dims);
	void						validate(TYPE type);



////////////////	GETS AND SETS ARE INLINE FOR SPEED

	//	source set
	void setState(const void* p_real, const void* p_imag, UINT64 bytes);



	TYPE typeFromString(const string& typeString);
	TYPE complexityFromString(const string& complexityString);
	Dims dimsFromString(string dims);

	void getReadBuffer(TYPE type, const void*& p_real, const void*& p_imag, UINT64& bytes);

	void NullFile();
	void OpenFile(const char* filename);
	void CloseFile();
	void WriteFile(const BYTE* buffer, UINT64 bytes);


////////////////	STATE

private:

	//	structure
	Dims						dims;
	numeric::Structure			structure;
	string						eas_str;

	//	state
	UINT32 contentHeaderBytes;
	void resizeState();
	VBYTE						headerAndState;				//	real and imaginary alongside (imaginary is there only if we are complex)

	//	layout conversion buffers (interleaved/adjacent, column-major/row-major)
	ConversionBuffers conversion;
	
	//	genertic stuff
	EventGenericContent			p_state;					//	pointers are NULL if corresponding state is empty
	EventGenericForm			eaf;

	//	log
	//
	//	if logging complex adjacent, we log the real and imag to separate
	//	logs, so we can produce a properly formed adjacent data block at exit, with
	//	real/imag as the trailing dimension.
	//
	//	if logging complex interleaved, we log both to the
	//	same log, so we can produce a properly formed interleaved data block at exit,
	//	with real/imag as the leading dimension
	VBYTE log1, log2;



	/*

		Am trialling the replacement of C++ output stream ofstream with an old
		style C file, so that i can set O_SYNC on it, which disables OS-level
		file buffering, avoiding alex's bug of long hangs whilst the OS does a
		sync every now and then.

		Hmmm... no need to set O_SYNC, and the performance is better than C++
		file all round. I'll try a bigger file write (several gig).

		Yup, that seems to work ok too. Ok, so perhaps we'll go with C-style
		i/o for now, since it's better behaved. I'll check if it works similarly
		well on windows.

		Yuh, works fine also on windows. I may as well change the data/spikes
		class for the same reason I guess.

	*/

//#define USE_CPP_FILE_IO

#ifdef USE_CPP_FILE_IO
	ofstream					outfile;
#else
	FILE*						outfid;
#endif

	//	parameters
	bool Encapsulated;
	bool WouldHaveBeenUnencapsulated;
	bool BufferToMemory;

	//	temp
	string outputFilename;

};




void explode(const string delim, const string& data, VSTRING& ret)
{
	//	return vector of strings
	UINT32 offset = 0;

	//	split data by delim
	while(offset <= data.length())
	{
		//	find occurrence of delim
		string::size_type p = data.find(delim, offset);

		//	if not found, just add what's left
		if (p == string::npos)
		{
			ret.push_back(data.substr(offset));
			break;
		}

		//	otherwise, add first bit and continue with other bit
		ret.push_back(data.substr(offset,p-offset));
		offset = p + 1;
	}
}





////////////////	INLINE ACCESSORS FOR SPEED

inline void COMPONENT_CLASS_CPP::setState(const void* q_real, const void* q_imag, UINT64 bytes)
{
	//	if zero size, nothing to do
	if (!structure.numberOfBytesTotal) return;

	//	check size is correct, if supplied
	if (bytes && (bytes != structure.numberOfBytesReal)) berr << "wrong number of bytes supplied";

	//	if no real supplied (we know now that it's non-zero size)
	if (!q_real) berr << "no real data supplied";

	//	if both are supplied, we better be complex...
	if (q_imag && (!structure.complex)) berr << "complex data supplied but object not complex";

	//	if both are supplied, data is coming in two halves
	if (q_imag)
	{
		if (structure.type & TYPE_CPXFMT_INTERLEAVED)
			berr <<" complex (interleaved) data should be supplied in a single block";

		memcpy((void*)p_state.real, q_real, structure.numberOfBytesReal);
		memcpy((void*)p_state.imag, q_imag, structure.numberOfBytesReal);
	}

	//	otherwise, data is coming all at once, either just real, or real and imag
	else
	{
		memcpy((void*)p_state.real, q_real, structure.numberOfBytesTotal);
	}
}







TYPE COMPONENT_CLASS_CPP::typeFromString(const string& typeString)
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
	berr << "invalid structure string \"" << typeString << "\"";
	return TYPE_UNSPECIFIED;
}

TYPE COMPONENT_CLASS_CPP::complexityFromString(const string& complexityString)
{
	if (complexityString == "REAL") return TYPE_REAL;
	if (complexityString == "COMPLEX") return TYPE_COMPLEX;
	berr << "invalid structure string \"" << complexityString << "\"";
	return TYPE_UNSPECIFIED;
}

Dims COMPONENT_CLASS_CPP::dimsFromString(string sdims)
{
	Dims dims;
	string sd;

	while (sdims.length())
	{
		size_t p = sdims.find(",");
		if (p == string::npos)
		{
			sd = sdims;
			sdims = "";
		}
		else
		{
			sd = sdims.substr(0, p);
			sdims = sdims.substr(p+1);
		}

		UINT32 d;
		if (!(istringstream(sd) >> d))
			berr << "invalid structure string";
		dims.push_back(d);
	}

	return dims;
}


void COMPONENT_CLASS_CPP::getReadBuffer(TYPE type, const void*& p_real, const void*& p_imag, UINT64& bytes)
{
	//	check not row-major
	if ((type & TYPE_ORDER_MASK) == TYPE_ORDER_ROW_MAJOR)
		berr << "only column-major is currently supported";

	//	check for conditions where we can just return the write buffer
	if (
		(structure.type & TYPE_REAL) // data has no imaginary part
		||
		((type & TYPE_CPXFMT_MASK) == TYPE_CPXFMT_UNSPECIFIED) // caller will accept either complex format
		||
		(((type & TYPE_CPXFMT_MASK) == TYPE_CPXFMT_INTERLEAVED) && ((structure.type & TYPE_CPXFMT_MASK) == TYPE_CPXFMT_INTERLEAVED)) // caller insists, but write buffer matches requested format
		||
		(((type & TYPE_CPXFMT_MASK) == TYPE_CPXFMT_ADJACENT) && ((structure.type & TYPE_CPXFMT_MASK) == TYPE_CPXFMT_ADJACENT)) // caller insists, but write buffer matches requested format
		)
	{
		//	correct format already, or leave in whatever format
		p_real = p_state.real;
		p_imag = p_state.imag;
		bytes = p_state.bytes;
		return;
	}
	
	//	if we get to here, we are needing to do a conversion. currently, either
	//	interleaved --> adjacent, or vice versa, since we only support column-major
	//	adding the conversion for row-major to this function is all that needs
	//	doing to add row-major support. plus taking out a couple of asserts() that
	//	the caller hasn't asked for row major (in setStructure() , i think).

	/*

		NOTES ON MULTITHREADING

		this event may be fired from multiple worker threads hosting multiple reader processes
		of this data object, and these calls may come concurrently. if we're not doing a conversion,
		we just return a pointer to the memory block, and they can all read simultaneously if they
		like, since nothing is being changed. if we are doing a conversion, the picture is similar
		(they all get a pointer to a memory block, and can all read simultaneously), but we have to
		prepare the memory block (write it) before returning the first read pointer.

		thus, we have to do thread-safety (mutexing) whilst we're actually preparing the block. the
		most common case is that there is only one reader. unfortunately, we can't know this (though
		i suppose we could add this information, perhaps have it sent in by the framework during
		EVENT_INIT_COMPLETE? TODO but note we would have to make sure all data objects in the ring
		buffer got this info) so we have to lock the mutex even in that case. the readers may
		be racing to obtain a read, and we have to ensure that only one of the threads actually
		prepares the block, rather than doing it multiple times. therefore, we use the flag
		"converted", which is set false when the object is written in EVENT_DATA_CONTENT_SET or
		EVENT_DATA_CONTENT_GET with F_GET_FOR_WRITE or EVENT_GENERIC_CONTENT_SET.

		when a reader comes through, it checks the state of "converted". if false, it locks the
		mutex, then checks again. if still false, it performs the conversion, then sets "converted"
		to true. this means that readers will usually get away without locking the mutex at all if
		they come through after another reader has already done the conversion (which will generally
		be very fast). worst case is that they come in whilst conversion is under way - in this case
		they will see "converted" false, block on lock mutex until conversion is complete, lock
		the mutex, see "converted" true, release the mutex. this is a shame, but i think its the
		unavoidable cost of doing the conversion once for multiple readers. if it turns out that it
		is more of a performance hit doing this inter-thread synchronisation than doing a conversion
		separately in each thread, then the advice can be changed so that the reader should accept the
		data in its native format, and performs the conversion locally if required. TODO then is a
		performance comparison of these two approaches for different sized data blocks (and on different
		platforms).

		incidentally, we similarly have to take precautions when assigning the conversionBuffer. we
		could assign the conversion buffer earlier than EVENT_DATA_CONTENT_GET, but then it would always
		exist, even if it wasn't going to be used. this could be a big memory hit in some cases.

		note also that the write-phase and read-phase are guaranteed to be distinct (that is, reads
		and write can never be concurrent). therefore, we don't need to be concerned with thread-safety
		here, and also we can just invalidate() all the buffers when a write comes in, safe in the
		knowledge that noone is currently reading.

	*/

	//	choose the buffer we need to use
	ConversionBuffer* buffer = NULL;
	switch (type & (TYPE_CPXFMT_MASK | TYPE_ORDER_MASK))
	{
		case TYPE_CPXFMT_ADJACENT | TYPE_ORDER_COLUMN_MAJOR:
		{
			buffer = &conversion.ac;
			break;
		}

		case TYPE_CPXFMT_INTERLEAVED | TYPE_ORDER_COLUMN_MAJOR:
		{
			buffer = &conversion.ic;
			break;
		}

		default: berr << E_INTERNAL;
	}

	//	otherwise, we need to do a conversion
	if (!buffer->assigned) // quick check does not require a mutex lock, since this boolean only ever goes from false to true once, and never back, during its lifetime
		buffer->assign(structure.numberOfBytesTotal);

	//	pointer to buffer
	BYTE* p_conv_real = buffer->get();
	BYTE* p_conv_imag = structure.numberOfBytesTotal ? p_conv_real + structure.numberOfBytesReal  : NULL;

	if ((type & TYPE_CPXFMT_MASK) == TYPE_CPXFMT_ADJACENT) // given above, this implies interleaved storage
	{
		//	point at conversion buffer
		p_real = p_conv_real;
		p_imag = p_conv_imag;

		if (!buffer->converted) // check once quickly without locking the mutex
		{
			//	mutex lock
			MutexLocker locker(buffer->mutex);

			if (!buffer->converted) // check again once we have the mutex
			{
				switch(structure.type & TYPE_ELEMENT_MASK)
				{
					case TYPE_DOUBLE:
					case TYPE_UINT64:
					case TYPE_INT64:
					{
						//	anything 64-bit
						UINT64* p_dst_r = (UINT64*) p_conv_real;
						UINT64* p_dst_i = (UINT64*) p_conv_imag;
						UINT64* p_src_r = (UINT64*) p_state.real;
						for (UINT64 e=0; e<structure.numberOfElementsReal; e++)
						{
							p_dst_r[e] = p_src_r[e*2];
							p_dst_i[e] = p_src_r[e*2+1];
						}
						break;
					}

					case TYPE_SINGLE:
					case TYPE_UINT32:
					case TYPE_INT32:
					{
						//	anything 32-bit
						UINT32* p_dst_r = (UINT32*) p_conv_real;
						UINT32* p_dst_i = (UINT32*) p_conv_imag;
						UINT32* p_src_r = (UINT32*) p_state.real;
						for (UINT64 e=0; e<structure.numberOfElementsReal; e++)
						{
							p_dst_r[e] = p_src_r[e*2];
							p_dst_i[e] = p_src_r[e*2+1];
						}
						break;
					}

					case TYPE_UINT16:
					case TYPE_INT16:
					{
						//	anything 16-bit
						UINT16* p_dst_r = (UINT16*) p_conv_real;
						UINT16* p_dst_i = (UINT16*) p_conv_imag;
						UINT16* p_src_r = (UINT16*) p_state.real;
						for (UINT64 e=0; e<structure.numberOfElementsReal; e++)
						{
							p_dst_r[e] = p_src_r[e*2];
							p_dst_i[e] = p_src_r[e*2+1];
						}
						break;
					}

					case TYPE_UINT8:
					case TYPE_INT8:
					case TYPE_BOOL8:
					{
						//	anything 8-bit
						UINT8* p_dst_r = (UINT8*) p_conv_real;
						UINT8* p_dst_i = (UINT8*) p_conv_imag;
						UINT8* p_src_r = (UINT8*) p_state.real;
						for (UINT64 e=0; e<structure.numberOfElementsReal; e++)
						{
							p_dst_r[e] = p_src_r[e*2];
							p_dst_i[e] = p_src_r[e*2+1];
						}
						break;
					}
				}

				buffer->converted = true;
			}
		}
	}

	else if ((type & TYPE_CPXFMT_MASK) == TYPE_CPXFMT_INTERLEAVED) // given above, this implies adjacent storage
	{
		//	point at conversion buffer
		p_real = p_conv_real;
		p_imag = NULL; // for interleaved data, imag is always returned NULL, since it's meaningless

		if (!buffer->converted) // check once quickly without locking the mutex
		{
			//	mutex lock
			MutexLocker locker(buffer->mutex);

			if (!buffer->converted) // check again once we have the mutex
			{
				switch(structure.type & TYPE_ELEMENT_MASK)
				{
					case TYPE_DOUBLE:
					case TYPE_UINT64:
					case TYPE_INT64:
					{
						//	anything 64-bit
						UINT64* p_dst_r = (UINT64*) p_conv_real;
						UINT64* p_src_r = (UINT64*) p_state.real;
						UINT64* p_src_i = (UINT64*) p_state.imag;
						for (UINT64 e=0; e<structure.numberOfElementsReal; e++)
						{
							p_dst_r[e*2] = p_src_r[e];
							p_dst_r[e*2+1] = p_src_i[e];

//							cout << ((DOUBLE*)p_dst_r)[e*2+1] << endl;
						}
						break;
					}

					case TYPE_SINGLE:
					case TYPE_UINT32:
					case TYPE_INT32:
					{
						//	anything 32-bit
						UINT32* p_dst_r = (UINT32*) p_conv_real;
						UINT32* p_src_r = (UINT32*) p_state.real;
						UINT32* p_src_i = (UINT32*) p_state.imag;
						for (UINT64 e=0; e<structure.numberOfElementsReal; e++)
						{
							p_dst_r[e*2] = p_src_r[e];
							p_dst_r[e*2+1] = p_src_i[e];
						}
						break;
					}

					case TYPE_UINT16:
					case TYPE_INT16:
					{
						//	anything 16-bit
						UINT16* p_dst_r = (UINT16*) p_conv_real;
						UINT16* p_src_r = (UINT16*) p_state.real;
						UINT16* p_src_i = (UINT16*) p_state.imag;
						for (UINT64 e=0; e<structure.numberOfElementsReal; e++)
						{
							p_dst_r[e*2] = p_src_r[e];
							p_dst_r[e*2+1] = p_src_i[e];
						}
						break;
					}

					case TYPE_UINT8:
					case TYPE_INT8:
					case TYPE_BOOL8:
					{
						//	anything 8-bit
						UINT8* p_dst_r = (UINT8*) p_conv_real;
						UINT8* p_src_r = (UINT8*) p_state.real;
						UINT8* p_src_i = (UINT8*) p_state.imag;
						for (UINT64 e=0; e<structure.numberOfElementsReal; e++)
						{
							p_dst_r[e*2] = p_src_r[e];
							p_dst_r[e*2+1] = p_src_i[e];
						}
						break;
					}
				}

				buffer->converted = true;
			}
		}
	}

	else berr << E_INTERNAL << "type == " << hex << type;

	//	conversion buffer bytes are same as write buffer bytes
	bytes = p_state.bytes;
}



////////////////	'STRUCTORS

COMPONENT_CLASS_CPP::COMPONENT_CLASS_CPP()
{
	contentHeaderBytes = 0;

	____CLEAR(structure);
	____CLEAR(p_state);

	//	give it a simple structure (empty double) so that these objects *always* have a valid structure of some sort
	numeric::Structure s;
	____CLEAR(s);
	s.type = TYPE_DOUBLE | TYPE_REAL | TYPE_CPXFMT_ADJACENT | TYPE_ORDER_COLUMN_MAJOR;
	Dims temp(0);
	s.dims = temp.cdims();
	setStructure(&s);

	//	null output file
	NullFile();
}

COMPONENT_CLASS_CPP::~COMPONENT_CLASS_CPP()
{
	CloseFile();
}



COMPONENT_CLASS_CPP::COMPONENT_CLASS_CPP(const COMPONENT_CLASS_CPP& src) : Data(src) // this last bit after the : tells the compiler to call the base class default copy constructor automatically
{
	//	the default copy constructor will just copy p_real, and clearly it has to point at
	//	our local state, not the src->state! note that "state = src.state" does not copy
	//	state.capacity(), either. doesn't matter here, but be careful!
	contentHeaderBytes = src.contentHeaderBytes;

	//	set structure does this quite safely!
	setStructure(&src.structure);

	//	we need also to copy our content
	if (p_state.real) memcpy((void*)p_state.real, src.p_state.real, structure.numberOfBytesTotal);
	//if (p_state.imag) memcpy((void*)p_state.imag, src.p_state.imag, structure.numberOfBytesReal);

	//	Don't copy outfile, because if it is active, *only* the 0th data object needs access to it
	NullFile();

	//	base class copy constructor has been called already, because we appended : Data(src) to the end of this definition
	//	see http://www.devx.com/tips/Tip/27939 for a brief tutorial
}




////////////////	HELPERS

void COMPONENT_CLASS_CPP::NullFile()
{
	//	null file
#ifdef USE_CPP_FILE_IO
	//	no action needed
#else
	//	null file descriptor
	outfid = NULL;
#endif
}

void COMPONENT_CLASS_CPP::OpenFile(const char* filename)
{
	//	open file
	outputFilename = filename;
#ifdef USE_CPP_FILE_IO
	outfile.open(filename, ios::out | ios::trunc | ios::binary);
	if (!outfile.is_open())
		berr << "failed to open file \"" << filename << "\"";
#else
	outfid = fopen(filename, "wb");
	if (!outfid)
		berr << "failed to open file \"" << filename << "\"";
#endif
}

void COMPONENT_CLASS_CPP::CloseFile()
{
	//	close file
#ifdef USE_CPP_FILE_IO
	if (outfile.is_open()) outfile.close();
#else
	if (outfid)
	{
		fclose(outfid);
		outfid = NULL;
	}
#endif
}

void COMPONENT_CLASS_CPP::WriteFile(const BYTE* buffer, UINT64 bytes)
{
	//	write file
#ifdef USE_CPP_FILE_IO
	outfile.write((char*)buffer, bytes);
	if (outfile.bad())
		berr << "failed to write file \"" << outputFilename << "\"";
#else
	if (fwrite(buffer, bytes, 1, outfid) != 1)
		berr << "failed to write file \"" << outputFilename << "\"";
#endif
}



////////////////	SYSTEM INTERFACE

Symbol COMPONENT_CLASS_CPP::event(Event* event)
{

	switch(event->type)
	{

		case EVENT_CONTENT_SET:
		{
			EventContent* ec = (EventContent*) event->data;

			//	just read state array
			if (structure.numberOfBytesTotal != ec->bytes)
				berr << "stream length incorrect in EVENT_CONTENT_SET (" << ec->bytes << " instead of " << structure.numberOfBytesTotal << ")";
			if (structure.numberOfBytesTotal) // avoids taking &state[0] if state is an empty vector, which is a seg fault in CL14
				memcpy((void*)p_state.real, ec->stream, ec->bytes);

			//	ok
			return C_OK;
		}



		case EVENT_CONTENT_GET:
		{
			EventContent* ec = (EventContent*) event->data;

			ec->stream = &headerAndState[0];
			ec->bytes = structure.numberOfBytesTotal;

			//	ok
			return C_OK;
		}



		case EVENT_LOG_SERVICE:
		{
			EventLog* el = (EventLog*) event->data;

			//	cast input
			COMPONENT_CLASS_CPP* src = (COMPONENT_CLASS_CPP*) el->source;

			//	to file
			if (!BufferToMemory)
			{
				//	this interleaves real & complex, so we'll have to uninterleave them when we read them

				//	write to output file
				WriteFile((const BYTE*)src->p_state.real, structure.numberOfBytesTotal);
			}

			//	to memory
			else
			{
				if (structure.numberOfBytesTotal)
				{
					switch (structure.type & (TYPE_COMPLEX_MASK | TYPE_CPXFMT_MASK))
					{
						case TYPE_REAL | TYPE_CPXFMT_ADJACENT:
						case TYPE_REAL | TYPE_CPXFMT_INTERLEAVED:
						{
							UINT32 offset = log1.size();
							log1.resize(offset + structure.numberOfBytesReal);
							memcpy(&log1[offset], src->p_state.real, structure.numberOfBytesReal);
							break;
						}

						case TYPE_COMPLEX | TYPE_CPXFMT_ADJACENT:
						{
							UINT32 offset = log1.size();
							log1.resize(offset + structure.numberOfBytesReal);
							memcpy(&log1[offset], src->p_state.real, structure.numberOfBytesReal);
							log2.resize(offset + structure.numberOfBytesReal);
							memcpy(&log2[offset], src->p_state.imag, structure.numberOfBytesReal);
							break;
						}

						case TYPE_COMPLEX | TYPE_CPXFMT_INTERLEAVED:
						{
							UINT32 offset = log1.size();
							log1.resize(offset + structure.numberOfBytesReal * 2);
							memcpy(&log1[offset], src->p_state.real, structure.numberOfBytesReal * 2);
							break;
						}

						default: berr << E_INTERNAL;
					}
				}
			}

			//	ok
			return C_OK;
		}






////////////////	GENERIC ACCESS INTERFACE

		case EVENT_GENERIC_STRUCTURE_GET:
		{
			EventGenericStructure* eas = (EventGenericStructure*) event->data;

			//	format: <type>/<REAL|COMPLEX_ADJACENT|COMPLEX_INTERLEAVED>/<comma-separated-dims>
			ostringstream ss;
			ss << getElementTypeString(structure.type);
			switch (structure.type & TYPE_COMPLEX_MASK)
			{
				case TYPE_REAL: ss << "/REAL/"; break;
				case TYPE_COMPLEX: ss << "/COMPLEX/"; break;
			}
			if (!(eas->type & TYPE_CPXFMT_MASK))
			{
				switch (structure.type & TYPE_CPXFMT_MASK)
				{
					case TYPE_CPXFMT_ADJACENT: ss << "CPXFMT_ADJACENT/"; break;
					case TYPE_CPXFMT_INTERLEAVED: ss << "CPXFMT_INTERLEAVED/"; break;
				}
			}
			if (!(eas->type & TYPE_ORDER_MASK))
			{
				switch (structure.type & TYPE_ORDER_MASK)
				{
					case TYPE_ORDER_COLUMN_MAJOR: ss << "COLUMN_MAJOR/"; break;
					case TYPE_ORDER_ROW_MAJOR: ss << "ROW_MAJOR/"; break;
				}
			}
			for (UINT32 d=0; d<structure.dims.count; d++)
			{
				if (d) ss << ",";
				ss << structure.dims.dims[d];
			}
			eas_str = ss.str();

			eas->structure = eas_str.c_str();

			//	ok
			return C_OK;
		}

		case EVENT_GENERIC_STRUCTURE_SET:
		{
			EventGenericStructure* eas = (EventGenericStructure*) event->data;

			//	format: <type>/<REAL|COMPLEX>/<COMPLEX_ADJACENT|COMPLEX_INTERLEAVED>/<COLUMN_MAJOR|ROW_MAJOR>/<comma-separated-dims>
			vector<string> parts;
			explode("/", eas->structure, parts);
			if (parts.size() < 3 || parts.size() > 5)
				berr << "invalid structure string \"" << eas->structure << "\" (wrong part count)";

			//	dims come last
			string sdims = parts.back();
			parts.pop_back();

			//	type and complexity first
			TYPE type = eas->type | typeFromString(parts[0]) | complexityFromString(parts[1]);

			//	complex storage format and ordering constants may be in string, or may be in "type"
			for (UINT32 p=2; p<parts.size(); p++)
			{
				if (parts[p] == "CPXFMT_ADJACENT") type |= TYPE_CPXFMT_ADJACENT;
				else if (parts[p] == "CPXFMT_INTERLEAVED") type |= TYPE_CPXFMT_INTERLEAVED;
				else if (parts[p] == "COLUMN_MAJOR") type |= TYPE_ORDER_COLUMN_MAJOR;
				else if (parts[p] == "ROW_MAJOR") type |= TYPE_ORDER_ROW_MAJOR;
				else berr << "invalid structure string \"" << eas->structure << "\" (unrecognised part \"" << parts[p] << "\")";
			}

			//	now we can call setStructure()
			numeric::Structure st;
			____CLEAR(st);
			st.type = type;
			Dims temp = dimsFromString(sdims);
			st.dims = temp.cdims();
			setStructure(&st);

			//	and return the converted data
			eas->type = structure.type;

			//	ok
			return C_OK;
		}

		case EVENT_GENERIC_FORM_ADVANCE:
		{
			EventGenericForm* egf = (EventGenericForm*) event->data;

			*egf = eaf;

			//	ok
			return C_OK;
		}

		case EVENT_GENERIC_CONTENT_GET:
		{
			EventGenericContent* egc = (EventGenericContent*) event->data;

			//	get pointer to block (convert if necessary)
			getReadBuffer(egc->type, egc->real, egc->imag, egc->bytes);

			//	ok
			return C_OK;
		}

		case EVENT_GENERIC_CONTENT_SET:
		{
			EventGenericContent* eac = (EventGenericContent*) event->data;

			//	mark that data has changed
			conversion.invalidate();

			//	delegate
			setState(eac->real, eac->imag, eac->bytes);

			//	ok
			return C_OK;
		}







		case numeric::EVENT_DATA_STRUCTURE_SET:
		{
			numeric::Structure* s = (numeric::Structure*) event->data;
			setStructure(s);

			//	ok
			return C_OK;
		}

		case numeric::EVENT_DATA_STRUCTURE_GET:
		{
			event->data = &structure;

			//	ok
			return C_OK;
		}

		case numeric::EVENT_DATA_VALIDATE_TYPE:
		{
			numeric::Structure* s = (numeric::Structure*) event->data;
			validate(s->type);

			//	ok
			return C_OK;
		}

		case numeric::EVENT_DATA_VALIDATE_STRUCTURE:
		{
			numeric::Structure* s = (numeric::Structure*) event->data;
			validate(s->type, s->dims);

			//	ok
			return C_OK;
		}

		case EVENT_INIT_COMPLETE:
		{
			EventInitComplete* eic = (EventInitComplete*) event->data;

			//	may be zero if we won't receive EVENT_CONTENT_GET, in which case no need to take action
			if (eic->contentHeaderBytes != 0)
			{
				//	store
				contentHeaderBytes = eic->contentHeaderBytes;
				
				//	get pointer to old content
				BYTE* old = (BYTE*) p_state.real;

				//	resize buffer
				resizeState();

				//	copy any spikes that may be present
				if (structure.numberOfBytesTotal)
					memmove((void*)p_state.real, old, structure.numberOfBytesTotal);

				//	and zero the header
				memset(&headerAndState[0], 0, contentHeaderBytes);
			}

			//	ok
			return C_OK;
		}

		case numeric::EVENT_DATA_CONTENT_SET:
		{
			numeric::Content* c = (numeric::Content*) event->data;

			//	mark that data has changed
			conversion.invalidate();

			//	delegate
			setState(c->real, c->imag, c->bytes);

			//	ok
			return C_OK;
		}

		case numeric::EVENT_DATA_CONTENT_GET:
		{
			numeric::Content* c = (numeric::Content*) event->data;

			//	if it's going to be written...
			if (c->flags & numeric::F_GET_FOR_WRITE)
			{
				//	...mark that data has changed
				conversion.invalidate();
			}

			//	get pointer to block (convert if necessary)
			getReadBuffer(c->type, c->real, c->imag, c->bytes);

			//	ok
			return C_OK;
		}

		case EVENT_STATE_SET:
		{
			EventStateSet* ess = (EventStateSet*) event->data;

			if (ess->flags & F_UNDEFINED)
			{
				//	set everything to NaNs
				//	switch on type
				switch(structure.type & TYPE_ELEMENT_MASK)
				{
					case TYPE_DOUBLE:
					{
						DOUBLE undef = numeric_limits<DOUBLE>::quiet_NaN();
						DOUBLE* p_content = (DOUBLE*) p_state.real;
						for (UINT32 i=0; i<structure.numberOfElementsTotal; i++) p_content[i] = undef;
						break;
					}

					case TYPE_SINGLE:
					{
						SINGLE undef = numeric_limits<SINGLE>::quiet_NaN();
						SINGLE* p_content = (SINGLE*) p_state.real;
						for (UINT32 i=0; i<structure.numberOfElementsTotal; i++) p_content[i] = undef;
						break;
					}

					//	other cases don't have real "undefined", so we use zero
					default:
						memset((void*)p_state.real, 0, structure.numberOfBytesTotal);
				}
			}

			else if (ess->flags & F_ZERO)
			{
				//	set everything to zero
				memset((void*)p_state.real, 0, structure.numberOfBytesTotal);
			}

			else
			{
				//	expect DataML node
				XMLNode xnode(ess->state);
				DataMLNode node(&xnode);

				//	get structure
				numeric::Structure rstructure;
				____CLEAR(rstructure);
//				cout << hex << node.getType() << endl;
				rstructure.type = node.getType();
				Dims temp = node.getDims();
				rstructure.dims = temp.cdims();
				setStructure(&rstructure);

				//	get state
				node.getRaw((BYTE*)p_state.real, (BYTE*)p_state.imag);
			}

			//	ok
			return C_OK;
		}



		case EVENT_STATE_GET:
		{
			EventStateGet* esg = (EventStateGet*) event->data;

			//	create <State> node
			XMLNode xmlNode;
			esg->state = xmlNode.element;

			//	wrap it in a DataMLNode interface
			DataMLNode state(&xmlNode);
			state.precision(esg->precision);

			//	write it
			state.setRaw(structure.dims, structure.type, p_state.real, p_state.imag);

			/*

				There's actually no need to announce to the world that we use DataML
				as our storage format, since nobody else has to read this (the data
				that other people might read is passed out in EVENT_LOG_TERM). Therefore,
				to save space in XML files, and amount of data passed over the network,
				we don't actually bother setting the root tags that indicate that the
				XML is DataML.

				If we *do* announce it, then the Data in Links becomes more easily
				accessible in paused systems, through bindings that understand DataML,
				because they can do the translation for the user.

			//	root tags
			state.setRootTags();

			*/

			//	ok
			return C_OK;
		}



		case EVENT_LOG_INIT:
		{
			EventLog* el = (EventLog*) event->data;

			//	calculate required storage
			UINT64 numSamples = time->executionStop / time->samplePeriod;
			UINT64 storedElements = structure.numberOfElementsTotal * numSamples;
			UINT64 bytesRequired = structure.numberOfBytesTotal * numSamples;

			//	storage mode
			BufferToMemory = executionInfo->BufferingPolicy == C_BUFFERING_ONLY_MEMORY;
			Encapsulated = storedElements <= 1000 || (el->flags & F_ENCAPSULATED);

			//	if this is true, we must store at max precision, since we would have done
			//	if we'd stored to binary file - if we don't do this, our precision depends
			//	on the number of elements stored (max in a binary file, as specified in a
			//	encapsulated file) which is confusing
			WouldHaveBeenUnencapsulated = !(el->flags & F_ENCAPSULATED);

			//	we *actually* buffer to memory either if we're running encap
			//	*or* if we're asked to buffer to memory and we're running !encap
			BufferToMemory = BufferToMemory || Encapsulated;

			//	if not encapsulated, open output file
			if (!BufferToMemory)
			{
				OpenFile(el->filename);
			}

			//	if buffering to memory, reserve memory
			if (BufferToMemory)
			{
				try
				{
					switch (structure.type & (TYPE_COMPLEX_MASK | TYPE_CPXFMT_MASK))
					{
						case TYPE_REAL | TYPE_CPXFMT_ADJACENT:
						case TYPE_REAL | TYPE_CPXFMT_INTERLEAVED:
						{
							log1.reserve(bytesRequired);
							break;
						}

						case TYPE_COMPLEX | TYPE_CPXFMT_ADJACENT:
						{
							log1.reserve(bytesRequired / 2);
							log2.reserve(bytesRequired / 2);
							break;
						}

						case TYPE_COMPLEX | TYPE_CPXFMT_INTERLEAVED:
						{
							log1.reserve(bytesRequired);
							break;
						}

						default: berr << E_INTERNAL;
					}
				}

				catch(std::exception e)
				{
					EventErrorMessage data;
					data.error = E_STD;
					data.msg = e.what();
					data.flags = 0;

					EngineEvent event;
					event.hCaller = hComponent;
					event.flags = 0;
					event.type = ENGINE_EVENT_ERROR_MESSAGE;
					event.data = &data;
					data.error = brahms_engineEvent(&event);

					string name = "<unknown>";
					if (componentData) name = componentData->name;
					string ss = ("whilst allocating memory for log of data object \"" + name + "\"");
					data.msg = ss.c_str();
					data.flags = F_TRACE;

					throw brahms_engineEvent(&event);
				}
			}

			//	ok
			return C_OK;
		}



		case EVENT_LOG_TERM:
		{
			EventLog* el = (EventLog*) event->data;

			//	create output node
			XMLNode xmlNode;
			el->result = xmlNode.element;

			//	wrap it in a DataMLNode interface
			DataMLNode nodeLog(&xmlNode);
			nodeLog.precision(el->precision);

			//	add log count as last dimension
			Dims dims = structure.dims;
			if (dims.size()) dims.push_back(el->count);

			//	to file
			if (!Encapsulated)
			{
				if (BufferToMemory)
				{
					//	must open & write the file now!
					OpenFile(el->filename);

					if (structure.numberOfBytesTotal)
					{
						//	check size is as expected
						if (el->count * structure.numberOfBytesTotal != (log1.size() + log2.size()))
							berr << "wrong byte count while storing";

						//	get pointers to real and imag logs
						const BYTE* p_log1 = (log1.size() ? &log1[0] : NULL);
						const BYTE* p_log2 = (log2.size() ? &log2[0] : NULL);

						//	get mult for log1
						UINT32 mult = (structure.type & TYPE_COMPLEX) ? 2 : 1;

						//	it has to come out interleaved, like if we'd written at run-time
						for (UINT32 s=0; s<el->count; s++)
						{
							WriteFile(p_log1, structure.numberOfBytesReal * mult);
							p_log1 += structure.numberOfBytesReal * mult;

							if (p_log2)
							{
								WriteFile(p_log2, structure.numberOfBytesReal);
								p_log2 += structure.numberOfBytesReal;
							}
						}
					}
				}

				//	already finished writing file
				CloseFile();

				//	lay it in to DataML node
				nodeLog.setBinaryFile(dims, structure.type, el->filename);
			}

			//	to memory
			else
			{
				//	if would have been encap, set max precision
				if (WouldHaveBeenUnencapsulated) nodeLog.precision(PRECISION_NOT_SET);

				//	check size is as expected
				if (el->count * structure.numberOfBytesTotal != (log1.size() + log2.size()))
					berr << "wrong byte count while storing";

				//	get pointers to real and imag logs
				const BYTE* p_log1 = log1.size() ? &log1[0] : NULL;
				const BYTE* p_log2 = log2.size() ? &log2[0] : NULL;

				//cout << componentData->name << ", " << log1.size() << ", " << log2.size() << endl;
				//cout << componentData->name << ", " << UINT64(p_log1) << ", " << UINT64(p_log2) << endl;

				//	write it
				nodeLog.setRaw(dims, structure.type, p_log1, p_log2);
			}

			//	write XML node
			nodeLog.setRootTags();

			//	ok
			return C_OK;
		}

	}

	return S_NULL;

}



////////////////	COMPONENT_CLASS_CPP INTERFACE

void COMPONENT_CLASS_CPP::resizeState()
{
	headerAndState.resize(contentHeaderBytes + structure.numberOfBytesTotal);

	//	set up fast pointers
	if (structure.numberOfBytesTotal)
	{
		p_state.real = &headerAndState[contentHeaderBytes];

		//	note that p_state.imag is NULL in TYPE_COMPLEX_INTERLEAVED case, since it's meaningless, so returning it to the client will only confuse
		p_state.imag = (structure.type & TYPE_COMPLEX) ? (&headerAndState[contentHeaderBytes + structure.numberOfBytesReal]) : NULL;
	}
	else
	{
		p_state.real = NULL;
		p_state.imag = NULL;
	}
	p_state.bytes = structure.numberOfBytesReal;
}

void COMPONENT_CLASS_CPP::setStructure(const numeric::Structure* pstructure)
{
	//	validate type float int uint bool
	switch(pstructure->type & TYPE_ELEMENT_MASK)
	{
		case TYPE_DOUBLE:
		case TYPE_SINGLE:
		case TYPE_UINT64:
		case TYPE_UINT32:
		case TYPE_UINT16:
		case TYPE_UINT8:
		case TYPE_INT64:
		case TYPE_INT32:
		case TYPE_INT16:
		case TYPE_INT8:
		case TYPE_BOOL8:
			break;
		default:
			berr << "invalid element type for a numeric array: must be float, int, uint or bool, and size must be 32/64 for floats, 8/16/32/64 for ints, and 8-bit for bool";
	}

	if (pstructure->type & (~TYPE_ARRAY_MASK))
		berr << "unrecognised bits set in type constant (type constant was 0x" << hex << pstructure->type << ")";

	if (!pstructure->dims.count)
		berr << "number of dimensions of array cannot be zero (though any individual dimension can)";

	//	type
	TYPE type = pstructure->type;

	//	complexity
	switch (type & TYPE_COMPLEX_MASK)
	{
		case TYPE_COMPLEX_UNSPECIFIED:
		{
			berr << "must specify complexity";
		}

		case TYPE_REAL:
		case TYPE_COMPLEX:
		{
			break;
		}

		default:
		{
			berr << "invalid complexity in TYPE constant";
		}
	}

	//	complex storage format
	switch (type & TYPE_CPXFMT_MASK)
	{
		case TYPE_CPXFMT_UNSPECIFIED:
		{
			//	default
			type |= TYPE_CPXFMT_ADJACENT;
			break;
		}

		case TYPE_CPXFMT_ADJACENT:
		case TYPE_CPXFMT_INTERLEAVED:
		{
			break;
		}

		default:
		{
			berr << "invalid complex storage format in TYPE constant";
		}
	}

	//	order storage format
	switch (type & TYPE_ORDER_MASK)
	{
		case TYPE_ORDER_UNSPECIFIED:
		{
			type |= TYPE_ORDER_COLUMN_MAJOR;
			break;
		}

		case TYPE_ORDER_COLUMN_MAJOR:
		{
			break;
		}

		case TYPE_ORDER_ROW_MAJOR:
		{
			berr << "row-major ordering not currently supported";
		}

		default:
		{
			berr << "invalid order storage format in TYPE constant";
		}
	}

	//	record structure
	dims = pstructure->dims;
	structure.type = type;
	structure.dims = dims.cdims();

	//	quick
	structure.typeElement = structure.type & TYPE_ELEMENT_MASK;
	structure.bytesPerElement = TYPE_BYTES(structure.type);

	//	calculate size variables
	structure.numberOfElementsReal = dims.getNumberOfElements();
	structure.numberOfElementsTotal = structure.numberOfElementsReal * TYPE_COMPLEX_MULT(structure.type);
	structure.numberOfBytesReal = structure.numberOfElementsReal * structure.bytesPerElement;
	structure.numberOfBytesTotal = structure.numberOfBytesReal * TYPE_COMPLEX_MULT(structure.type);

	//	calculate quick variables
	structure.complex = ((structure.type & TYPE_COMPLEX) != 0);
	structure.scalar = structure.numberOfElementsReal == 1;
	structure.realScalar = structure.numberOfElementsTotal == 1;

	//	resize state
	resizeState();

	//	set up numeric form: this is easy for us, cos we _are_ numeric!
	eaf.form = C_FORM_FIXED;
	eaf.type = structure.type;
	eaf.dims = structure.dims;
}






string n2s(UINT32 n)
{
	ostringstream s;
	s << n;
	return s.str();
}



std::string ordinal(UINT32 n)
{
	string ret = n2s(n);
	if (n>10 && n<20) return ret + "th";
	char c = ret[ret.length() - 1];
	if (c == '1') return ret + "st";
	if (c == '2') return ret + "nd";
	if (c == '3') return ret + "rd";
	return ret + "th";
}

void COMPONENT_CLASS_CPP::validate(TYPE type, Dims reqDims)
{
	validate(type);

	//	remove ellipsis if if present
	bool ellipsis = (reqDims.size() && reqDims.at(reqDims.size() - 1) == DIM_ELLIPSIS);
	if (ellipsis) reqDims.pop_back();

	//	get copy of our dimensions
	Dims thisDims = structure.dims;

	//	check size
	if (thisDims.size() != reqDims.size())
	{
		if (ellipsis)
		{
			if (thisDims.size() < reqDims.size())
				berr << "expected data to have at least " << reqDims.size() << " dimensions";
		}
		else
		{
			berr << "expected data to have " << reqDims.size() << " dimensions";
		}
	}

	//	verify dims
	for (UINT32 d=0; d<reqDims.size(); d++)
	{
		if (reqDims.at(d) >= 0)
		{
			if (reqDims.at(d) != thisDims.at(d))
			{
				berr << "expected data to have " +
				ordinal(d+1) + " dimension of " + n2s(reqDims.at(d));
			}
		}

		else if (reqDims.at(d) == DIM_NONZERO)
		{
			if (thisDims.at(d) == 0)
			{
				berr << "expected data to have " +
				ordinal(d+1) + " dimension non-zero";
			}
		}

		else if (reqDims.at(d) == DIM_ANY) { /* no check */ }

		else berr << "Dims array invalid, found value \"" << reqDims.at(d) << "\"";
	}
}

void COMPONENT_CLASS_CPP::validate(TYPE type)
{
	//	check element type
	if ((type & TYPE_ELEMENT_MASK) != TYPE_UNSPECIFIED)
	{
		if (structure.typeElement != (type & TYPE_ELEMENT_MASK))
			berr << "expected data to be of type " << getElementTypeString(type);
	}

	//	complexity
	switch (type & TYPE_COMPLEX_MASK)
	{
		case TYPE_COMPLEX_UNSPECIFIED:
		{
			//	no validate
			break;
		}

		case TYPE_REAL:
		{
			if ((structure.type & TYPE_COMPLEX_MASK) != TYPE_REAL)
				berr << "expected data to be real";
			break;
		}

		case TYPE_COMPLEX:
		{
			if ((structure.type & TYPE_COMPLEX_MASK) != TYPE_COMPLEX)
				berr << "expected data to be complex";
			break;
		}

		default:
		{
			berr << "invalid complexity in TYPE constant";
		}
	}

	//	complex storage format
	switch (type & TYPE_CPXFMT_MASK)
	{
		case TYPE_CPXFMT_UNSPECIFIED:
		{
			//	no validate
			break;
		}

		case TYPE_CPXFMT_ADJACENT:
		{
			if ((structure.type & TYPE_CPXFMT_MASK) != TYPE_CPXFMT_ADJACENT)
				berr << "expected data to be complex-adjacent format";
			break;
		}

		case TYPE_CPXFMT_INTERLEAVED:
		{
			if ((structure.type & TYPE_CPXFMT_MASK) != TYPE_CPXFMT_INTERLEAVED)
				berr << "expected data to be complex-interleaved format";
			break;
		}

		default:
		{
			berr << "invalid complex storage format in TYPE constant";
		}
	}

	//	order storage format
	switch (type & TYPE_ORDER_MASK)
	{
		case TYPE_ORDER_UNSPECIFIED:
		{
			//	no validate
			break;
		}

		case TYPE_ORDER_COLUMN_MAJOR:
		{
			if ((structure.type & TYPE_ORDER_MASK) != TYPE_ORDER_COLUMN_MAJOR)
				berr << "expected data to be column-major format";
			break;
		}

		case TYPE_ORDER_ROW_MAJOR:
		{
			if ((structure.type & TYPE_ORDER_MASK) != TYPE_ORDER_ROW_MAJOR)
				berr << "expected data to be row-major format";
			break;
		}

		default:
		{
			berr << "invalid order storage format in TYPE constant";
		}
	}
}



#include "brahms-1199.h"
