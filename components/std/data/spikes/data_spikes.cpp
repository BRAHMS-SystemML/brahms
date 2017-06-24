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

	$Id:: data_spikes.cpp 2437 2009-12-13 19:06:12Z benjmitch  $
	$Rev:: 2437                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-12-13 19:06:12 +0000 (Sun, 13 Dec 2009)       $
________________________________________________________________

*/

/*
  since spikes doesn't currently write to file at run-time, we've
  disabled all the file writing, and it just always sends its ouput
  encapsulated. this may be troublesome when large numbers of spikes
  are generated, so we may have to fix this in a future release.
*/

////////////////	COMPONENT INFO

//	component information
#define COMPONENT_CLASS_STRING "std/2009/data/spikes"
#define COMPONENT_CLASS_CPP class__std_2009_data_spikes_0
#define COMPONENT_FLAGS (0)

//	common
#include "components/data.h"

//	include header
#include "data_spikes.h"
namespace spikes = std_2009_data_spikes_0;

//	include
#include <cstdlib> // atof() on linux
#include <string>
using std::string;
#include <sstream>
using std::ostringstream;

////////////////	COMPONENT CLASS
class COMPONENT_CLASS_CPP : public Data
{
////////////////	'STRUCTORS
public:
	COMPONENT_CLASS_CPP();
	~COMPONENT_CLASS_CPP();
	COMPONENT_CLASS_CPP(const COMPONENT_CLASS_CPP& src);

	Symbol event(Event* event);

////////////////	MEMBERS
private:

	//	state
	struct
	{
		//	header required by system, followed by list of stream indices that are in spike state
		//	size() of this vector is the capacity of the spikes object
		//	headerAndState =
		//	{
		//		0	header
		//		...
		//		p	state
		//		...
		//	}
		VBYTE		headerAndState;

		//	dimensions of spikes object (dims.getNumberOfElements(), then, is the "capacity" - the max number of spikes that can be stored)
		Dims		dims;
		Dimensions	cdims; // just points at "dims"
		UINT32		capacity; // just a cached copy of dims.getNumberOfElements(), set in setDimensions()

		//	spikes.spikes points at the offset in headerAndState where the spikes are stored (p, in above diagram)
		//	spikes.count is the number of spikes currently stored in this object (data in headerAndState beyond this count is irrelevant)
		spikes::Spikes spikes;
	}
	state;

#define CHRON_LOGGING
#ifdef CHRON_LOGGING
	//	log
	VINT32 log;
#else
	//	log
	VINT32 log_t;
	VINT32 log_n;
#endif

	//	structure set
	void					setDimensions(Dimensions cdims);

	//	buffer resize
	UINT32 contentHeaderBytes;
	void resizeBuffer();

	/*

		Am trialling the replacement of C++ output stream ofstream with an old
		style C file, so that i can set O_SYNC on it, which disables OS-level
		file buffering, avoiding alex's bug of long hangs whilst the OS does a
		sync every now and then.

		SEE NOTES IN data/numeric

		BUT!!! since data/spikes doesn't actually *do* run-time writing to file
		at the moment (rather, it stores the data in memory and writes it all at
		the end), there's no need to finish the implementation, here. it is, therefore,
		only partly done.

	*/
/*
#define USE_CPP_FILE_IO
#ifdef USE_CPP_FILE_IO
	ofstream					outfile;
#else
	FILE*						outfid;
#endif
*/

////////////////	PRE-PREPARED DATA READY FOR FRAMEWORK CALLS (CACHED, BASICALLY)

	//	EVENT_GENERIC_STRUCTURE_*
	struct EAS_DATA
	{
		string					str;
	}
	eas;

	//	EVENT_GENERIC_FORM_*
	struct EAF_DATA
	{
		Dims					dims;
	}
	eaf;

	//	EVENT_GENERIC_CONTENT_*
	struct EAC_DATA
	{
		EventGenericContent		data;
	}
	eac;

	BYTE* p_headerAndState;

};


void COMPONENT_CLASS_CPP::resizeBuffer()
{
	//	resize storage vector
	state.headerAndState.resize(contentHeaderBytes + state.capacity * sizeof(INT32));

	//	reset state pointer, and set count to zero
	state.spikes.spikes = (INT32*) (state.capacity ? &state.headerAndState[contentHeaderBytes] : NULL);
	state.spikes.count = 0;

	//	prepare ec data
	p_headerAndState = state.headerAndState.size() ? &state.headerAndState[0] : NULL; // point at beginning of header
}

void COMPONENT_CLASS_CPP::setDimensions(Dimensions cdims)
{

////////////////	UPDATE STATE

	//	set dims and capacity
	state.dims = cdims;
	state.cdims = state.dims.cdims();

        // state.capacity was set to the number of elements allocated
        // by default in a Dim struct and therefore in a VINT64, which
        // is just vector<INT64>. Now, as far as I understand, a
        // vector should resize itself as required, but this capacity
        // matters as it's used in resizeBuffer to resize some other,
        // associated memory storage structures.
        state.capacity = state.dims.getNumberOfElements() * 8;
        state.dims.reserve (state.capacity);
        printf ("state.capacity initially set to %d\n", state.capacity);

	//	resize storage vector
	resizeBuffer();


////////////////	PRE-CALCULATE (CACHE) DATA FOR RETURN TO FRAMEWORK REQUESTS

	//	prepare eas data
	eas.str = state.dims.commaString();

	//	prepare eaf data (dimensioned as a vector, not as a multi-dimensional array)
	eaf.dims.resize(1);
	eaf.dims[0] = state.capacity;

	//	prepare eac data
	eac.data.real = state.spikes.spikes;
	eac.data.imag = NULL;
	eac.data.bytes = 0;
}





//	compile information
#define MEMORY_PER_OBJECT 1048576







string n2s(DOUBLE n)
{
	ostringstream s;
	s << n;
	return s.str();
}

////////////////	'STRUCTORS

COMPONENT_CLASS_CPP::COMPONENT_CLASS_CPP()
{
	//	assume zero until we hear otherwise
	contentHeaderBytes = 0;

	Dims dims;
	dims.push_back(0);
	setDimensions(dims.cdims());


/*
	//	null output file
#ifdef USE_CPP_FILE_IO
	//	no action needed
#else
	outfid = NULL;
#endif
*/
}

COMPONENT_CLASS_CPP::~COMPONENT_CLASS_CPP()
{

	/*
	//	close output file
#ifdef USE_CPP_FILE_IO
	if (outfile.is_open()) outfile.close();
#else
	if (outfid)
	{
		fclose(outfid);
		outfid = NULL;
	}
#endif
	*/
}

COMPONENT_CLASS_CPP::COMPONENT_CLASS_CPP(const COMPONENT_CLASS_CPP& src) : Data(src) // this last bit after the : tells the compiler to call the base class default copy constructor automatically
{
	contentHeaderBytes = src.contentHeaderBytes;

	//	set dimensions from src to set up structure
	setDimensions(src.state.dims.cdims());

	//	then set count and memcpy spikes over (unlikely to be any at copy time, but let's do it anyway)
	state.spikes.count = src.state.spikes.count;
	memcpy(state.spikes.spikes, src.state.spikes.spikes, state.spikes.count * sizeof(INT32));

/*
	//	Don't copy outfile, because if it is active, *only* the 0th data object needs access to it
#ifdef USE_CPP_FILE_IO
	//	no action needed
#else
	//	null file descriptor
	outfid = NULL;
#endif
*/

	//	base class copy constructor has been called already, because we appended : Data(src) to the end of this definition
	//	see http://www.devx.com/tips/Tip/27939 for a brief tutorial
}



////////////////	SYSTEM INTERFACE

Symbol COMPONENT_CLASS_CPP::event(Event* event)
{

	switch(event->type)
	{

		case EVENT_CONTENT_GET:
		{
			EventContent* ec = (EventContent*) event->data;

			ec->stream = p_headerAndState;
			ec->bytes = state.spikes.count * sizeof(INT32);

			//	ok
			return C_OK;
		}



		case EVENT_CONTENT_SET:
		{
			EventContent* ec = (EventContent*) event->data;

			//	just read state array
			state.spikes.count = ec->bytes / sizeof(INT32);
			if ((state.spikes.count * 4 != ec->bytes) || (state.spikes.count > state.capacity))
				berr << "stream length incorrect in EVENT_CONTENT_SET (" << ec->bytes << " when capacity was " << state.capacity << ")";
			if (state.spikes.count)
				memcpy((BYTE*)state.spikes.spikes, ec->stream, ec->bytes);

			//	ok
			return C_OK;
		}



		case EVENT_LOG_SERVICE:
		{
			EventLog* el = (EventLog*) event->data;

			//	cast input
			COMPONENT_CLASS_CPP* src = (COMPONENT_CLASS_CPP*) el->source;

#ifdef CHRON_LOGGING
			//	to file
			if (!(el->flags & F_ENCAPSULATED))
			{
				if (src->state.spikes.count)
				{
					UINT32 C = src->state.spikes.count;
					UINT32 N0 = log.size();
					UINT32 N1 = N0 + C*2;
					log.resize(N1);
					if (src->state.spikes.count)
					{
						for (UINT32 s=0; s<C; s++)
						{
							log[N0+s*2] = el->count;
							log[N0+s*2+1] = src->state.spikes.spikes[s];
						}
					}
				}
			}

			//	to memory
			else
			{
				if (src->state.spikes.count)
				{
					UINT32 C = src->state.spikes.count;
					UINT32 N0 = log.size();
					UINT32 N1 = N0 + C*2;
					log.resize(N1);
					if (src->state.spikes.count)
					{
						for (UINT32 s=0; s<C; s++)
						{
							log[N0+s*2] = el->count;
							log[N0+s*2+1] = src->state.spikes.spikes[s];
						}
					}
				}
			}
#else
			//	to file
			if (!(el->flags & F_ENCAPSULATED))
			{
				if (src->state.spikes.count)
				{
					UINT32 M = log_n.size();
					UINT32 N = M + src->state.spikes.count;
					log_n.resize(N);
					log_t.resize(N);
					if (src->state.spikes.count)
						memcpy(&log_n[M], (BYTE*)src->state.spikes.spikes, src->state.spikes.count * sizeof(INT32));
					for (UINT32 n=M; n<N; n++) log_t[n] = el->count;
				}
			}

			//	to memory
			else
			{
				if (src->state.spikes.count)
				{
					UINT32 M = log_n.size();
					UINT32 N = M + src->state.spikes.count;
					log_n.resize(N);
					log_t.resize(N);
					if (src->state.spikes.count)
						memcpy(&log_n[M], (BYTE*)src->state.spikes.spikes, src->state.spikes.count * sizeof(INT32));
					for (UINT32 n=M; n<N; n++) log_t[n] = el->count;
				}
			}
#endif

			//	ok
			return C_OK;
		}



////////////////	GENERIC ACCESS INTERFACE

		case EVENT_GENERIC_STRUCTURE_GET:
		{
			EventGenericStructure* eas = (EventGenericStructure*) event->data;

			eas->structure = this->eas.str.c_str();

			//	ok
			return C_OK;
		}

		case EVENT_GENERIC_STRUCTURE_SET:
		{
			EventGenericStructure* eas = (EventGenericStructure*) event->data;

			//	format: <dim,dim,dim...>
			string fmt = eas->structure;
			Dims dims;
			while (true)
			{
				string::size_type p = fmt.find(",");
				if (p == string::npos)
				{
					dims.push_back(atof(fmt.c_str()));
					break;
				}

				string el = fmt.substr(0, p);
				fmt = fmt.substr(p+1);
				dims.push_back(atof(el.c_str()));
			}

			//	check
			if (dims.commaString() != eas->structure)
				berr << "invalid structure string \"" << eas->structure << "\"";

			//	do it
			setDimensions(dims.cdims());

			//	return type
			eas->type = TYPE_INT32 | TYPE_REAL | (eas->type & (TYPE_CPXFMT_MASK | TYPE_ORDER_MASK));

			//	ok
			return C_OK;
		}

		case EVENT_GENERIC_FORM_ADVANCE:
		{
			EventGenericForm* egf = (EventGenericForm*) event->data;

			eaf.dims[0] = state.capacity;

			egf->form = C_FORM_FIXED_BUT_LAST;
			egf->type = TYPE_INT32 | TYPE_REAL;
			egf->dims = eaf.dims.cdims();

			//	ok
			return C_OK;
		}

		case EVENT_GENERIC_FORM_CURRENT:
		{
			EventGenericForm* egf = (EventGenericForm*) event->data;

			eaf.dims[0] = state.spikes.count;

			egf->type = TYPE_INT32 | TYPE_REAL;
			egf->dims = eaf.dims.cdims();

			//	ok
			return C_OK;
		}

		case EVENT_GENERIC_CONTENT_GET:
		{
			EventGenericContent* egc = (EventGenericContent*) event->data;

			eac.data.bytes = state.spikes.count * sizeof(INT32);

			egc->real = eac.data.real;
			egc->imag = eac.data.imag;
			egc->bytes = eac.data.bytes;

			//	ok
			return C_OK;
		}

		case EVENT_GENERIC_CONTENT_SET:
		{
			EventGenericContent* eac = (EventGenericContent*) event->data;

			//	udata should be byte count
			//	pdata should be content bytes
			state.spikes.count = eac->bytes / sizeof(INT32);
			if ((state.spikes.count * sizeof(INT32)) != eac->bytes || (state.spikes.count > state.capacity))
				berr << "invalid set state (" << eac->bytes << " bytes supplied)";
			if (state.spikes.count)
			{
				if (!eac->real) berr << "invalid set state (no real data)";
				memcpy((BYTE*)state.spikes.spikes, eac->real, eac->bytes);
			}

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
				INT32* spikes = state.spikes.spikes;

				//	resize buffer
				resizeBuffer();

				//	copy any spikes that may be present
				if (state.spikes.count)
					memmove(state.spikes.spikes, spikes, state.spikes.count * sizeof(INT32));

				//	and zero the header
				memset(&state.headerAndState[0], 0, contentHeaderBytes);
			}

			//	ok
			return C_OK;
		}



////////////////	SPECIFIC ACCESS INTERFACE

		case spikes::EVENT_DATA_STRUCTURE_SET:
		{
			setDimensions(*((Dimensions*) event->data));

			//	ok
			return C_OK;
		}

		case spikes::EVENT_DATA_STRUCTURE_GET:
		{
			event->data = &state.cdims;

			//	ok
			return C_OK;
		}

		case spikes::EVENT_DATA_CONTENT_SET:
		{
			spikes::Spikes* spikes = (spikes::Spikes*) event->data;

			if (spikes->count > state.capacity) {
                            berr << "too many spikes supplied (" << spikes->count
                                 << " is more than previously set capacity of " << state.capacity << ")";
                        }
			state.spikes.count = spikes->count;
			if (spikes->count)
				memcpy((BYTE*)state.spikes.spikes, spikes->spikes, spikes->count * sizeof(INT32));

			//	ok
			return C_OK;
		}

		case spikes::EVENT_DATA_CONTENT_GET:
		{
			event->data = &state.spikes;

			//	ok
			return C_OK;
		}

		case EVENT_STATE_SET:
		{
			EventStateSet* ess = (EventStateSet*) event->data;

			if (ess->flags & F_UNDEFINED)
			{
				//	no meaningful undefined value, so we just set it empty
				state.spikes.count = 0;
			}

			else if (ess->flags & F_ZERO)
			{
				//	zero state is clearly no spikes
				state.spikes.count = 0;
			}

			else
			{
				//	expect DataML node
				XMLNode xnode(ess->state);

				//	get dimensions
				string fmt = xnode.getAttribute("Dims");

				//	format: <dim,dim,dim...>
				Dims dims;
				while (true)
				{
					string::size_type p = fmt.find(",");
					if (p == string::npos)
					{
						dims.push_back(atof(fmt.c_str()));
						break;
					}

					string el = fmt.substr(0, p);
					fmt = fmt.substr(p+1);
					dims.push_back(atof(el.c_str()));
				}

				//	check
				if (dims.commaString() != xnode.getAttribute("Dims"))
					berr << "invalid structure string \"" << xnode.getAttribute("Dims") << "\"";

				//	do it
				setDimensions(dims.cdims());

				//	set count
				state.spikes.count = (UINT32) atof(xnode.getAttribute("Count"));

				//	set state
				const char* text = xnode.nodeText();
				for (UINT32 n=0; n<state.spikes.count; n++)
				{
					if (!(*text >= '0' && *text <= '9')) berr << "invalid state in EVENT_STATE_SET";
					state.spikes.spikes[n] = (INT32) atof(text);
					while(*text && *text != ' ') text++;
					if (*text == ' ') text++;
				}
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

			//	we ignore precision

			//	generate s
			string ss;
			for (UINT32 s=0; s<state.spikes.count; s++)
				ss += n2s(((INT32*)state.spikes.spikes)[s]) + " ";
			if (ss.length()) ss.erase(ss.end() - 1);

			//	generate StateML
			xmlNode.setAttribute("Dims", state.dims.commaString().c_str());
			xmlNode.setAttribute("Count", n2s(state.spikes.count).c_str());
			xmlNode.nodeText(ss.c_str());

			//	ok
			return C_OK;
		}



		case EVENT_LOG_INIT:
		{
			EventLog* el = (EventLog*) event->data;

			if (!(el->flags & F_ENCAPSULATED))
			{
				/*
				//	open file
				outfile.open(el->filename, ios::out | ios::binary);
				if (!outfile.is_open()) berr << "failed to open file \"" + string(el->filename) + "\"";
				*/
			}

			else
			{
				//	reserve memory for storage
				//	how can we do this when we don't know how much spiking will occur?
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

			//	build output structure
			VSTRING fieldNames;
			fieldNames.push_back("version");
			fieldNames.push_back("meta");
			fieldNames.push_back("fS");
			fieldNames.push_back("dims");
			fieldNames.push_back("t");
			fieldNames.push_back("s");
			fieldNames.push_back("ts");
			nodeLog.becomeStruct(Dims(1), fieldNames);

			//	set basics
			nodeLog.getField("version").setArray(Dims(1), VDOUBLE(1, 1));
			fieldNames.clear();
			nodeLog.getField("meta").becomeStruct(Dims(1), fieldNames);
			nodeLog.getField("fS").setArray(Dims(1), VDOUBLE(1, sampleRateToRate(time->sampleRate)));

			//	dims
			nodeLog.getField("dims").setArray(Dims(1, state.dims.size()), state.dims);

			//	t
			VINT32 v;
			v.push_back(0);
			v.push_back(el->count);
			nodeLog.getField("t").setArray(Dims(1, 2), v);

			//	s
			v.clear();
			v.push_back(0);
			v.push_back(state.capacity);
			nodeLog.getField("s").setArray(Dims(1, 2), v);

			//	store ts
			nodeLog.getField("ts").setArray(Dims(2, log.size()/2), log);

			//	write XML node
			nodeLog.setRootTags();

			//	ok
			return C_OK;
		}


	}


	return S_NULL;

}


#include "brahms-1199.h"
