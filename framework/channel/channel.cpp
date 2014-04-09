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

	$Id:: channel.cpp 2366 2009-11-13 00:46:58Z benjmitch      $
	$Rev:: 2366                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-13 00:46:58 +0000 (Fri, 13 Nov 2009)       $
________________________________________________________________

*/



/*

TODO: CHANNEL MUTEX

	Not entirely sure why this is used, but it protects push(), pull(),
	and close(). Thus, clients can't push() or pull() while the channel
	is closing. They also can't push() and pull() at the same time,
	which is probably a performance hit. This needs investigating.

*/




////////////////	INCLUDE SUPPORT

#define BRAHMS_BUILDING_CHANNEL

//	includes
#include "base/base.h"
#include "channel.h"
#include <queue>

//	import namespaces
using namespace brahms::channel;
using namespace brahms::base;
using namespace brahms::output;

//	safe IPM holder
struct SAFE_IPM
{
	SAFE_IPM()
	{
		m_ipm = NULL;
	}

	~SAFE_IPM()
	{
		release();
	}

	IPM*& ipm()
	{
		return m_ipm;
	}

	void release()
	{
		if (m_ipm)
		{
			IPM* temp = m_ipm;
			m_ipm = NULL;
			temp->release();
		}
	}

	IPM* retrieve()
	{
		IPM* temp = m_ipm;
		m_ipm = NULL; // we are relieved of this responsibility
		return temp;
	}

	IPM* m_ipm;
};

UINT32 unitIndex(UINT32 i)
{
	return i + 1;
}

struct QueueAuditDataX
{
	QueueAuditDataX()
	{
		queueAuditData.clear();
		queryBufferMsgsUnaccountedFor = 0;
	}

	QueueAuditData queueAuditData;

	//	increment when send QUERYBUFFER, decrement when receive USEDDATA
	UINT32 queryBufferMsgsUnaccountedFor;
};



////////////////	IPM DUMP

void ipm_dump(IPM* ipm, brahms::output::Source& tout, const char* name)
{
	tout << "--- IPM " << name << " ---" << D_INFO;
	tout << "sig: " << ((char*)&ipm->header().sig) << D_INFO;
	tout << "frm: " << dec << ipm->header().from << D_INFO;
	tout << "tag: " << brahms::base::TranslateIPMTAG(ipm->header().tag) << D_INFO;
	tout << "fmt: " << dec << ((UINT32)ipm->header().fmt) << D_INFO;
	tout << "order: " << dec << ipm->header().order << D_INFO;
	tout << "msgStreamID: " << dec << ipm->header().msgStreamID << D_INFO;
	tout << "bu: " << dec << ipm->header().bytesAfterHeaderUncompressed << D_INFO;
	tout << "bc: " << dec << ipm->header().bytesAfterHeaderCompressed << D_INFO;
	tout << "audit.bytes: " << dec << ipm->header().audit.bytes << D_INFO;
	tout << "audit.items: " << dec << ipm->header().audit.items << D_INFO;
	tout << "xxx IPM " << name << " xxx" << D_INFO;
}



////////////////	INCLUDE COMMON STUFF & COMMS PROTOCOL

#ifdef __WIN__

	void os_msleep(UINT32 ms)
	{
		Sleep(ms);
	}

#endif

#ifdef __NIX__

	void os_msleep(UINT32 ms)
	{
		usleep(ms * 1000);
	}

#endif


//#define REPORT_WAIT_STATES

#ifdef REPORT_WAIT_STATES
#define REPORT_THREAD_WAIT_STATE_IN(w) tout << "WAIT IN (" << w << ")" << D_INFO;
#define REPORT_THREAD_WAIT_STATE_OUT(w) tout << "WAIT OUT (" << w << ")" << D_INFO;
#else
#define REPORT_THREAD_WAIT_STATE_IN(w) ;
#define REPORT_THREAD_WAIT_STATE_OUT(w) ;
#endif

//	compress function, if available
CompressFunction* compressFunction = NULL;

//#include "compress.cpp"
#include "fifo.cpp"
#include "deliverer.cpp"

#ifdef __MPI__
#include "mpi.cpp"
#endif

#ifdef __SOCKETS__
#include "sockets.cpp"
#endif











////////////////	START NAMESPACE

namespace brahms
{





////////////////	CHANNEL CLASS

	Channel::~Channel()
	{
	}



////////////////	DERIVED CHANNEL CLASS

	enum ChannelState
	{
		CS_CLOSED = 0,
		CS_OPEN = 1
	};

	//	inter-process messages are sent back and forth over inter-process
	//	channels, which are full duplex links between processes, and take
	//	care of handshaking and resending internally
	class DerivedChannel : public Channel
	{

	public:

		//	creating the channel initializes resources and, in the case
		//	of TCP, starts listening for a connection from the peer.
		//	at this point, the channel is not open, it is just existing.
		//	channel destruction is a matter of releasing resources.
		DerivedChannel(ChannelInitData channelInitData, brahms::base::Core& core);

		//	open, close, and terminate
		//
		//	note that close() and terminate() have to be separate, so that
		//	GOODBYE is sent to all before we wait for GOODBYE from all.
		Symbol					open(brahms::output::Source& fout);
		Symbol					close(brahms::output::Source& fout);
		Symbol					terminate(brahms::output::Source& fout);
		void					stopRouting(brahms::output::Source& fout);

		//	these are the active functions, push and pull messages
		//	across the channel - see the functions themselves for
		//	details of what they can return
		UINT32					push(IPM* msg, brahms::output::Source* tout, bool ignoreIfNotOpen = false);
		Symbol					pull(IPM*& ipm, UINT8 tag, brahms::output::Source& tout, UINT32 timeout);
		Symbol					pullPrivate(IPM*& ipm, UINT8 tag, brahms::output::Source& tout, UINT32 timeout);
		
		//	flush
		void					flush(brahms::output::Source& tout);

		//	just returns audit data for display in the GUI
		bool					audit(ChannelAuditData& data);



	////////////////	DATA THAT NEEDS INITIALIZATION

		//	channel static state
		UINT32					SocketsTimeout;
		ChannelInitData			channelInitData;
		brahms::base::Core&		core;

		//	channel dynamic state
		ChannelState			state;
		UINT32					numberMessagesSent;
		UINT32					numberMessagesReceived;
		INT32					peerError;

		//	comms-layer-specific data
		ProtocolChannel			protocolChannel;



	////////////////	DATA THAT DOESN'T NEED INITIALIZATION (HAS OWN CTORS)

		//	timer
		brahms::os::Timer					watchdog;

		//	mutex for protecting access to all member data
		brahms::os::Mutex					channelMutex;

		//	routing table
		void addRoutingEntry(UINT32 msgStreamID, PushDataHandler pushDataHandler, void* pushDataHandlerArgument);

	};


	bool DerivedChannel::audit(ChannelAuditData& data)
	{
		protocolChannel.audit(data);
		return true; // continue
	}

	void DerivedChannel::addRoutingEntry(UINT32 msgStreamID, PushDataHandler pushDataHandler, void* pushDataHandlerArgument)
	{
		//	pass on to receiver
		protocolChannel.addRoutingEntry(msgStreamID, pushDataHandler, pushDataHandlerArgument);
	}



////////////////	CHANNEL

	DerivedChannel::DerivedChannel(ChannelInitData channelInitData, brahms::base::Core& p_core)
		:
		core(p_core),
		protocolChannel(channelInitData, core)
	{

		//	fout
		core.caller.tout << "creating channel to Voice " << unitIndex(channelInitData.remoteVoiceIndex) << D_VERB;



////////////////	INITIALIZE COMMON DATA

		//	channel static state
		SocketsTimeout = core.execPars.getu("SocketsTimeout");
		this->channelInitData = channelInitData;

		//	channel dynamic state
		state = CS_CLOSED;
		numberMessagesSent = 0;
		numberMessagesReceived = 0;
		peerError = -1;



////////////////	LISTEN ON CHANNEL

		protocolChannel.listen();



	}



////////////////	FLUSH FUNCTION

	void DerivedChannel::flush(brahms::output::Source& tout)
	{
		//	delegate
		protocolChannel.flush(tout);
	}
	
	

////////////////	PUSH FUNCTION

	UINT32 DerivedChannel::push(IPM* msg, brahms::output::Source* tout, bool ignoreIfNotOpen)
	{
		brahms::os::MutexLocker locker(channelMutex);

		//	get pointer to header
		IPM_HEADER* header = &msg->header();

		//if (tout) (*tout) << "push(" << brahms::base::TranslateIPMTAG(header->tag) << ")" << D_INFO;

		//	check state - HELLO messages are allowed through even when closed
		if (state == CS_OPEN || header->tag == IPMTAG_HELLO)
		{
			//	push the msg
			/*
				NB increment on numberMessagesSent as well as setting the MID needs mutex protection, else we could get out-of-order errors!
			*/

			//	tag message (note that this will overflow at 2^16 - unlikely, but still!)
			header->from = core.getVoiceIndex();

			//	pass to sender
			UINT32 msec = protocolChannel.push(msg, tout);

			//	keep track
			if (!msec) numberMessagesSent++;

			//	ok
			return msec;
		}
		else
		{
			//	release message
			msg->release();

			//	throw if appropriate
			if (!ignoreIfNotOpen)
				ferr << E_INTERNAL << "PUSH (" << brahms::base::TranslateIPMTAG(header->tag) << ") over channel that is not open";

			//	ok
			return 0;
		}

		//if (tout) (*tout) << "push done (" << brahms::base::TranslateIPMTAG(header->tag) << ")" << D_INFO;
	}



////////////////	PULL FUNCTION

	Symbol DerivedChannel::pull(IPM*& ipm, UINT8 tag, brahms::output::Source& tout, UINT32 timeout)
	{
		/*
			this is the clients' pull function, so it locks the mutex and
			checks that the channel is open
		*/
		brahms::os::MutexLocker locker(channelMutex);

		//	COMMS_TIMEOUT_DEFAULT equals use SocketsTimeout
		if (timeout == COMMS_TIMEOUT_DEFAULT)
			timeout = SocketsTimeout;

		//	check open
		if (state != CS_OPEN)
			ferr << E_INTERNAL << "pull() over channel that is not open";

		//	call sub
		return pullPrivate(ipm, tag, tout, timeout);
	}

	Symbol DerivedChannel::pullPrivate(IPM*& ipm, UINT8 tag, brahms::output::Source& tout, UINT32 timeout)
	{

		/*
			RETURN VALUES:

			S_NULL: no message pending (can only get this with timeout == 0)
			C_OK: message received
			E_COMMS_TIMEOUT: no message pending (can only get this with timeout != 0)
		*/

		//	recursive call to service timeout
		if (timeout)
		{
			//	prepare
			brahms::os::Timer watchdog;
			UINT32 quickDirtyCount = 0;

			//	pull loop
			while(true)
			{
				//	pull msg
				Symbol result = protocolChannel.pull(ipm, tout);
				if (result != C_OK) ferr << E_INTERNAL << "no message received";

				//	handle received
				if (result == C_OK) break;

				//	handle not received
				if (++quickDirtyCount == 100)
				{
					//	check for timeout
					if (watchdog.elapsedMS() >= timeout)
					{
						//	return code
						return E_COMMS_TIMEOUT;
					}

					//	restart
					quickDirtyCount = 0;
				}

				//	sleep and go round again
				brahms::os::msleep(1);
			}
		}

		//	single call because no timeout here
		else
		{
			//	pull msg
			Symbol result = protocolChannel.pull(ipm, tout);
			if (result != C_OK) ferr << E_INTERNAL << "no message received";
		}

		//	assert message tag and arg
		if (tag != IPMTAG_UNDEFINED && ipm->header().tag != tag)
			ferr << E_COMMS << "expecting " << brahms::base::TranslateIPMTAG(tag) << ", received " << brahms::base::TranslateIPMTAG(ipm->header().tag);

		//	ok
		return C_OK;
	}









////////////////	OPEN & CLOSE

	Symbol DerivedChannel::open(brahms::output::Source& fout)
	{
		//	fout
		fout << "opening channel to voice " << unitIndex(channelInitData.remoteVoiceIndex) << D_VERB;



////////////////	OPEN PROTOCOL LEVEL CHANNEL

		protocolChannel.open(fout);



////////////////	OPENING HANDSHAKE

		//	confirm channel is open with a handshake
		fout << "pushing IPMTAG_HELLO..." << D_FULL;
		IPM* ipms = protocolChannel.channelSlushPool.get(IPMTAG_HELLO, channelInitData.remoteVoiceIndex);

		//	push uses the mutex
		push(ipms, &fout);

		//	receive hello from remote Voice
		fout << "pulling IPMTAG_HELLO..." << D_FULL;
		IPM* ipmr;

		//	pullPrivate doesn't use the mutex
		Symbol result = pullPrivate(ipmr, IPMTAG_HELLO, fout, SocketsTimeout);
		if (!ipmr) ferr << E_INTERNAL;
		UINT8 tag = ipmr->header().tag;

		//	release
		ipmr->release();

		//	check if received
		if (result == E_COMMS_TIMEOUT)
			ferr << E_COMMS_TIMEOUT << "while waiting for IPMTAG_HELLO";

		//	confirm it is correct
		if (tag != IPMTAG_HELLO)
			ferr << E_COMMS << "expected IPMTAG_HELLO, received " << brahms::base::TranslateIPMTAG(tag);

		//	fout
		fout << "channel open!" << D_FULL;

		//	set state
		state = CS_OPEN;

		//	ok
		return C_OK;

	}

	Symbol DerivedChannel::close(brahms::output::Source& fout)
	{

////////////////	CLOSING HANDSHAKE

		try
		{
			//	report
			fout << "sending IPMTAG_GOODBYE (closing channel) to voice " << unitIndex(channelInitData.remoteVoiceIndex) << D_VERB;

			//	say GOODBYE to remote Voice
			IPM* ipms = protocolChannel.channelSlushPool.get(IPMTAG_GOODBYE, channelInitData.remoteVoiceIndex);
			push(ipms, &fout, true);

			//	close channel to push() and pull()
			state = CS_CLOSED;
		}

		catch(...)
		{
			//	close channel anyway...
			state = CS_CLOSED;

			//	rethrow
			throw;
		}

		//	ok
		return C_OK;
	}

	Symbol DerivedChannel::terminate(brahms::output::Source& fout)
	{
		//	terminate at protocol level
		protocolChannel.terminate(fout);

		//	ok
		return C_OK;
	}

	void DerivedChannel::stopRouting(brahms::output::Source& fout)
	{
		protocolChannel.stopRouting(fout);
	}









////////////////	EXPORTS

	namespace channel
	{



////////////////	COMMS INIT FUNCTION

		BRAHMS_CHANNEL_VIS CommsInitData commsInit(brahms::base::Core& core, CompressFunction* p_compressFunction)
		{
			//	store compressFunction, if supplied
			compressFunction = p_compressFunction;

			//	initialize comms layer
			return commsLayer.init(core);
		}


////////////////	CREATE CHANNEL FUNCTION

		BRAHMS_CHANNEL_VIS Channel* createChannel(brahms::base::Core& core, ChannelInitData channelInitData)
		{
			if (!commsLayer.isinit())
				ferr << E_INTERNAL << "must call commsInit() before createChannel()";
			return new DerivedChannel(channelInitData, core);
		}



	}
}


