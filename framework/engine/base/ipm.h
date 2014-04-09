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

	$Id:: ipm.h 2363 2009-11-12 23:58:51Z benjmitch            $
	$Rev:: 2363                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-12 23:58:51 +0000 (Thu, 12 Nov 2009)       $
________________________________________________________________

*/



#ifndef INCLUDED_BRAHMS_FRAMEWORK_BASE_IPM
#define INCLUDED_BRAHMS_FRAMEWORK_BASE_IPM



////////////////	NAMESPACE

namespace brahms
{
	namespace base
	{



		
////////////////	IPM CONSTANTS

		const UINT32 IPM_SIGNATURE			= 0x004D5049;		// "IPM\0" (Intel)

		//	comms maintenance messages
		const UINT8 IPMTAG_HELLO			= 0x11;			//	first message sent/received on all channels should be this
		const UINT8 IPMTAG_GOODBYE			= 0x12;			//	last message sent/received on all channels should be this
		const UINT8 IPMTAG_ERROR			= 0x13;			//	when a voice errors, it sends this to all other voices so that they know how to go down
		const UINT8 IPMTAG_CANCEL			= 0x14;			//	when a voice errors, it sends this to all other voices so that they know how to go down
		const UINT8 IPMTAG_SYNC				= 0x15;			//	exchanged to synchronise voices at a point in operation
		const UINT8 IPMTAG_KEEPALIVE		= 0x16;			//	sent by some comms layers to keep open a connection that is not passing any data
		const UINT8 IPMTAG_FLUSH			= 0x17;			//	used privately in some channel implementations to effect a flush of the send queue

		//	messages used during connect phase
		const UINT8 IPMTAG_ANNOUNCEOUTPUT	= 0x21;			//	notify peers that the specified output is now available from sender
		const UINT8 IPMTAG_FINDOUTPUT		= 0x22;			//	used to ask remote voice if they have a specified inlet (data is SystemML identifier of required inlet)
		const UINT8 IPMTAG_OUTPUTFOUND		= 0x23;			//	answer from remote voice that it does (data is serialized data object)
		const UINT8 IPMTAG_OUTPUTNOTFOUND	= 0x24;			//	answer from remote voice that it does not (no data)
			/* NOTE! now that we send IPMTAG_ANNOUNCEOUTPUT, peers know what each other has available, so
				IPMTAG_OUTPUTNOTFOUND should *never* get sent! */
		const UINT8 IPMTAG_ENDPHASE			= 0x25;			//	used to advise all remote voices that we are finished with our inlet requests for this pass

		//	messages used during baserate negotiation
		const UINT8 IPMTAG_PUSHRATES		= 0x31;			//	push all of our requested sample rates to the master (zeroth) voice
		const UINT8 IPMTAG_PUSHBASERATE		= 0x32;			//	push back the calculated system-wide baserate from the master voice to the other voices

		//	reporting limit
		const UINT8 IPMTAG_MAX_D_VERB		= 0x40;

		//	messages used during run-time data propagation
		const UINT8 IPMTAG_PUSHDATA			= 0x41;			//	push the serialized CONTENT of a data object between voices
		const UINT8 IPMTAG_USEDDATA			= 0x42;			//	sent back from dst voice to indicate that the receive buffer has freed up by some amount
		const UINT8 IPMTAG_QUERYBUFFER		= 0x43;			//	sent in place of PUSHDATA to elicit a USEDDATA, but avoiding filling the buffer any further

		//	undefined constants
		const UINT8 IPMTAG_UNDEFINED		= 0xFF;			//	undefined tag marker
		const UINT16 IPMVOICE_UNDEFINED		= 0xFFFF;
		const UINT32 IPM_UNDEFINED			= 0xFFFFFFFF;

		//	formats
		const UINT8	IPMFMT_UNCOMPRESSED		= 0x00;
		const UINT8	IPMFMT_DEFLATE			= 0x01;

		string TranslateIPMTAG(UINT8 tag);



////////////////	IPM HEADER

		struct QueueAuditData
		{
			void clear()
			{
				bytes = 0;
				items = 0;
			}

			UINT32 bytes;
			UINT32 items;
		};

		struct EndPhaseData
		{
			UINT32 finished;
			UINT32 progress;
		};

		struct IPM_HEADER
		{
			//	64 A (common)
			UINT32	sig;		//	signature tag so that we can assert message start point
			UINT16	from;		//	index of sending Voice
			UINT8	tag;		//	message tag (basic message content indicator, e.g. IPMTAG_OK)
			UINT8	fmt;		//	data format (uncompressed, deflate, etc.)

			//	64 B (specific)
			UINT32	order;		//	for IPMTAG_PUSHDATA, delivery order from the Outlet at the send end
			UINT32	msgStreamID;//	for IPMTAG_PUSHDATA, message stream identifier, used to find target at recv end (see notes below)

			//	64 C (size)
			UINT32  bytesAfterHeaderUncompressed;	//	bytes after header, uncompressed, may be zero
			UINT32  bytesAfterHeaderCompressed;		//	bytes after header, compressed, may be zero

			//	64 D (general)
			union
			{
				UINT64 D;
				QueueAuditData audit;
				EndPhaseData endPhaseData;
			};
		};



////////////////	IPM MESSAGE CLASS

		class IPM;
		class IPMPool;
		typedef void (*MessageAwayFunction)(IPM* ipm);

		struct IPM
		{
			IPM(MessageAwayFunction callback, void* parentPort, UINT32 index, UINT32 to);
			IPM(IPMPool* pool);
			~IPM();

			void clear();
			UINT32 size();
			void resize(UINT32 p_size);
			void resize____AND_LEAVE_CONTENTS_CORRUPTED____(UINT32 p_size);
			BYTE* stream(UINT32 offset = 0);
			IPM_HEADER& header();
			BYTE* body(UINT32 offset = 0);
			UINT32 uncompressedSize();
			UINT32 compressedSize();
			void appendString(const string& s);
			void appendBytes(BYTE* data, UINT32 count);
			void setExternalStream(BYTE* stream);
			void* getParentPort();
			UINT32 getIndex();
			UINT32 getTo();
			void setTo(UINT32 to);
			void release();

		private:

			//	parent pool
			IPMPool* m_pool;

			//	data location (equals m_block if we're storing internally)
			BYTE* m_data;

			//	memory block
			BYTE* m_block;

			//	memory block size
			UINT32 m_reserved;

			//	number of currently valid bytes (m_size <= m_reserved)
			UINT32 m_size;

			//	callback
			MessageAwayFunction m_callback;

			//	parent
			void* m_parentPort;

			//	index
			UINT32 m_index;

			//	destination
			UINT32 m_to;
		};



////////////////	IPM POOL

		/*	DOCUMENTATION: IPM_POOL

			At run-time, most messages pulled in are PUSHDATA, and it is these
			that we want to be processed afap. We therefore want to process
			them without performing an additional buffer copy. Therefore, we
			want to recv() them into a buffer that can be passed directly to
			the deliverer thread. We could do this by creating a new() buffer
			for every recv() and allowing the deliverer thread to call delete()
			on it when it has finished delivering it. But...

			This would require a constant new/delete cycle going on, which would
			be a performance hit, when in fact we only need a few buffers being
			passed back and forth between the receiver and the deliverer. Thus,
			we maintain a buffer pool.

			When the receiver needs to receive a message, it uses a buffer from
			the pool, if one exists, or creates a new one. It passes this to
			the deliverer. When the deliverer is done with it, it puts it back
			into the pool. Thus, the size of the pool will increase only as
			much as required, and we avoid constant memory allocation and
			deallocation.

			Since we're using the pool for this, we also use it for storing
			messages in the receive queue, ready to be pull()ed out. That is,
			messages other than PUSHDATA.
			
			Follow this documentation tag to find the points where messages are
			get() from pools and put() back in.

			If pools come up short (not all IPMs are checked back in at shutdown)
			then they send a message to stderr. If this happens, recompile with this
			symbol defined to track which messages don't end up coming back.
		*/

		//#define DEBUG_SHORT_POOLS

		//	channel pool data
		struct ChannelPoolData
		{
			ChannelPoolData()
			{
				inuse = 0;
				total = 0;
			}

			UINT32 inuse;
			UINT32 total;
		};

		struct IPMPool
		{
			IPMPool(string poolName)
			{
				total = 0;
				m_poolName = poolName;
			}

			~IPMPool()
			{
#ifdef DEBUG_SHORT_POOLS
				____WARN("DEBUG_SHORT_POOLS is on (this will marginally slow pool performance)");
#endif
				//	assert
				if (pool.size() != total)
				{
					____WARN("pool " << this << " \"" << m_poolName << "\" short (" << pool.size() << " of " << total << " IPMs present)");

#ifdef DEBUG_SHORT_POOLS
					for (list<IPM*>::iterator i=pool_checkedOut.begin(); i!=pool_checkedOut.end(); i++)
						____WARN("    MISSING: " << brahms::base::TranslateIPMTAG((*i)->header().tag) << " (" << (*i) << ")");
#endif
				}

				while(!pool.empty())
				{
					IPM* ipm = pool.top();
					pool.pop();
					delete ipm;
				}	
			}

			IPM* get(UINT8 tag, UINT32 to)
			{
				//	this overload of this function clears the message, as if
				//	it had been newly created, so that the caller can assume
				//	it contains default values to start off with.
				IPM* ipm = get();

				//	clear
				ipm->clear();

				//	fill in values
				ipm->header().tag = tag;
				ipm->setTo(to);

				//	ok
				return ipm;
			}

			IPM* get()
			{
				//	the no-arguments overload of this function returns the
				//	buffer as it comes, with whatever content and at whatever
				//	size. this is suitable for the receiver, who will immediately
				//	resize it and fill it from the comms layer.

				//	protect pool
				brahms::os::MutexLocker locker(mutex);

				//	if pool is empty, add one new IPM to it
				if (pool.empty())
				{
					//	update total number of IPMs
					total++;

					//	add new IPM to pool
					pool.push(new IPM(this));
				}

				//	get IPM from pool
				IPM* ipm = pool.top();
				pool.pop();

#ifdef DEBUG_SHORT_POOLS
				pool_checkedOut.push_back(ipm);
#endif

				//	and return that
				return ipm;
			}

			void audit(ChannelPoolData& data)
			{
				//	protect pool
				brahms::os::MutexLocker locker(mutex);

				//	do audit
				data.inuse += (total - pool.size());
				data.total += total;
			}

		private:

			friend class IPM;

			//	put is private, so can only be called through returnToPool()
			void put(IPM* ipm)
			{
				//	protect pool
				brahms::os::MutexLocker locker(mutex);

				//	ignore if NULL
				if (!ipm) return;

				//	if pool is large enough, discard IPM
				if (pool.size() >= 1000)
				{
					//	update total number of IPMs
					total--;

					//	discard IPM
					delete ipm;
				}

				//	else return IPM to pool
				else pool.push(ipm);

#ifdef DEBUG_SHORT_POOLS
				for (list<IPM*>::iterator i=pool_checkedOut.begin(); i!=pool_checkedOut.end(); i++)
				{
					if (*i == ipm)
					{
						pool_checkedOut.erase(i);
						break;
					}
				}
#endif
			}

			//	pool
			stack<IPM*> pool;

			//	number of IPMs that exist somewhere,
			//	whether checked out or held in the stack
			UINT32 total;		

			//	mutex to protect pool
			brahms::os::Mutex mutex;

			//	name for debugging
			string m_poolName;

#ifdef DEBUG_SHORT_POOLS
			//	debug
			list<IPM*> pool_checkedOut;
#endif
		};



////////////////	NAMESPACE
		
	}
}



////////////////	INCLUSION GUARD

#endif


