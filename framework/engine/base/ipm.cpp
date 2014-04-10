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

	$Id:: ipm.cpp 2283 2009-11-02 00:22:31Z benjmitch          $
	$Rev:: 2283                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-02 00:22:31 +0000 (Mon, 02 Nov 2009)       $
________________________________________________________________

*/

#include "ipm.h"

namespace brahms
{
	namespace base
	{

////////////////	TRANSLATE IPM TAG

		string TranslateIPMTAG(UINT8 tag)
		{
			switch(tag)
			{
				case IPMTAG_HELLO: return "IPMTAG_HELLO";
				case IPMTAG_GOODBYE: return "IPMTAG_GOODBYE";
				case IPMTAG_ERROR: return "IPMTAG_ERROR";
				case IPMTAG_CANCEL: return "IPMTAG_CANCEL";
				case IPMTAG_SYNC: return "IPMTAG_SYNC";
				case IPMTAG_KEEPALIVE: return "IPMTAG_KEEPALIVE";
				case IPMTAG_ANNOUNCEOUTPUT: return "IPMTAG_ANNOUNCEOUTPUT";
				case IPMTAG_FINDOUTPUT: return "IPMTAG_FINDOUTPUT";
				case IPMTAG_OUTPUTFOUND: return "IPMTAG_OUTPUTFOUND";
				case IPMTAG_OUTPUTNOTFOUND: return "IPMTAG_OUTPUTNOTFOUND";
				case IPMTAG_ENDPHASE: return "IPMTAG_ENDPHASE";
				case IPMTAG_PUSHRATES: return "IPMTAG_PUSHRATES";
				case IPMTAG_PUSHBASERATE: return "IPMTAG_PUSHBASERATE";
				case IPMTAG_PUSHDATA: return "IPMTAG_PUSHDATA";
				case IPMTAG_USEDDATA: return "IPMTAG_USEDDATA";
				case IPMTAG_QUERYBUFFER: return "IPMTAG_QUERYBUFFER";
				case IPMTAG_UNDEFINED: return "IPMTAG_UNDEFINED";
			}

			return "IPMTAG_<unknown>";
		}



////////////////	IPM MESSAGE CLASS

		/*
			Note that we don't use std::vector, here, though we could. This is
			so we can be absolutely sure when memory allocations are done, and
			keep them to a minimum for our application (i.e. buffers never
			make a reallocation to make the buffer smaller).
		*/

		//	construct static (no internal memory block, used for PUSHDATA messages only)
		IPM::IPM(MessageAwayFunction callback, void* parentPort, UINT32 index, UINT32 to)
		{
			//	not a pool member
			m_pool = NULL;

			//	no internal data
			m_data = NULL;
			m_block = NULL;
			m_reserved = 0;
			m_size = 0;

			//	has a callback
			m_callback = callback;
			m_parentPort = parentPort;
			m_index = index;

			//	routing
			m_to = to;
		}

		//	construct in pool (with internal memory block)
		IPM::IPM(IPMPool* pool)
		{
			//	member of this pool
			m_pool = pool;

			//	internal data
			m_reserved = sizeof(IPM_HEADER);
			m_size = m_reserved;
			m_data = m_block = new BYTE[m_reserved];

			//	no callback
			m_callback = NULL;
			m_parentPort = NULL;
			m_index = 0;

			//	no routing
			m_to = VOICE_UNDEFINED;

			//	clear
			clear();
		}

		void IPM::clear()
		{
			//	default size
			resize(sizeof(IPM_HEADER));

			//	initialise header with default values
			IPM_HEADER& p_header = header();

			//	A
			p_header.sig = IPM_SIGNATURE;
			p_header.from = IPMVOICE_UNDEFINED;
			p_header.tag = IPMTAG_UNDEFINED;
			p_header.fmt = IPMFMT_UNCOMPRESSED;

			//	B
			p_header.order = IPM_UNDEFINED;
			p_header.msgStreamID = IPM_UNDEFINED;

			//	C
			p_header.bytesAfterHeaderUncompressed = 0;
			p_header.bytesAfterHeaderCompressed = 0;

			//	D
			p_header.D = 0;
		}


		IPM::~IPM()
		{
			if (m_block) delete m_block;
		}

		UINT32 IPM::size()
		{
			return m_size;
		}

		void IPM::resize(UINT32 p_size)
		{
			//	error if no internal block
			if (!m_block) ferr << E_INTERNAL << "cannot resize external IPM";

			//	no action if currently of this size
			if (m_size == p_size) return;

			//	reserve more bytes if necessary (these buffers *never* get smaller)
			if (p_size > m_reserved)
			{
				//	get new memory block
				BYTE* new_m_data = new BYTE[p_size];

				//	copy old contents into it
				memcpy(new_m_data, m_data, m_size);

				//	discard old block
				delete m_block;

				//	and install new one
				m_data = m_block = new_m_data;

				//	record new reserved size
				m_reserved = p_size;
			}

			//	TODO: remove this! noone needs these zeroed! ...zero new bytes
			if (p_size > m_size)
				memset(m_data + m_size, 0, p_size - m_size);

			//	in any case, set the new size
			m_size = p_size;
		}

		void IPM::resize____AND_LEAVE_CONTENTS_CORRUPTED____(UINT32 p_size)
		{
			//	error if no internal block
			if (!m_block) ferr << E_INTERNAL << "cannot resize external IPM";

			//	no action if currently of this size
			if (m_size == p_size) return;

			//	in any case, set the new size
			m_size = p_size;

			//	reserve more if necessary. these buffers *never* get smaller, saving time on reallocation, but this
			//	means we must use them for similarly-sized things, else we take a memory usage hit. we do this. a more
			//	significant optimisation is that we don't contract to maintain the buffer's contents during this call.
			//	this is why the function is so named, to make this abundantly clear :).
			if (m_size > m_reserved)
			{
				delete m_block;
				m_data = m_block = new BYTE[m_size];
				m_reserved = m_size;
			}
		}

		BYTE* IPM::stream(UINT32 offset)
		{
			//	return pointer into stream at given offset
			return m_data + offset;
		}

		IPM_HEADER& IPM::header()
		{
			//	return reference to header (always present)
			return *((IPM_HEADER*)m_data);
		}

		BYTE* IPM::body(UINT32 offset)
		{
			//	return pointer into body at given offset
			offset += sizeof(IPM_HEADER);
			return m_data + offset;
		}

		UINT32 IPM::uncompressedSize()
		{
			return sizeof(IPM_HEADER) + header().bytesAfterHeaderUncompressed;
		}

		UINT32 IPM::compressedSize()
		{
			return sizeof(IPM_HEADER) + header().bytesAfterHeaderCompressed;
		}

		void IPM::appendString(const string& s)
		{
			//	append string, note NULL is included
			UINT32 offset = size();
			resize(size() + s.length() + 1);
			memcpy(m_data + offset, s.c_str(), s.length() + 1);

			//	update header
			header().bytesAfterHeaderUncompressed = size() - sizeof(IPM_HEADER);
		}

		void IPM::appendBytes(BYTE* data, UINT32 count)
		{
			//	ignore zero bytes case
			if (!count) return;

			//	append bytes
			UINT32 offset = size();
			resize(offset + count);
			memcpy(m_data + offset, data, count);

			//	update header
			header().bytesAfterHeaderUncompressed = size() - sizeof(IPM_HEADER);
		}

		void IPM::setExternalStream(BYTE* stream)
		{
			//	error if internal block
			if (m_block) ferr << E_INTERNAL << "m_block not NULL in setExternalStream()";

			//	set stream
			m_data = stream;
		}

		void* IPM::getParentPort()
		{
			return m_parentPort;
		}

		UINT32 IPM::getIndex()
		{
			return m_index;
		}

		UINT32 IPM::getTo()
		{
			return m_to;
		}

		void IPM::setTo(UINT32 to)
		{
			m_to = to;
		}

		void IPM::release()
		{
			//	if has a callback, fire that
			if (m_callback)
			{
				m_callback(this);
				return;
			}

			//	if from a pool, return to pool
			if (m_pool)
			{
				m_pool->put(this);
				return;
			}

			//	otherwise, i'm frankly at a loss
			ferr << E_INTERNAL << "m_callback and m_pool both NULL in IPM::release()";
		}



////////////////	NAMESPACE

	}
}




