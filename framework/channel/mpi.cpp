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

	$Id:: mpi.cpp 2366 2009-11-13 00:46:58Z benjmitch          $
	$Rev:: 2366                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-13 00:46:58 +0000 (Fri, 13 Nov 2009)       $
________________________________________________________________

*/





////////////////	PREAMBLE

#include "mpi-support.cpp"


using namespace brahms::channel;




////////////////	CHANNEL CLASS

	struct ProtocolChannel
	{
		ProtocolChannel(ChannelInitData p_channelInitData, brahms::base::Core& p_core);
		~ProtocolChannel();

		void listen() { } // no action in MPI
		void open(brahms::output::Source& fout);
		void terminate(brahms::output::Source& tout);
		void flush(brahms::output::Source& fout);

		void stopRouting(brahms::output::Source& tout)
		{
			//	terminate deliverers
			for (UINT32 d=0; d<deliverers.size(); d++)
			{
				if (deliverers[d])
					deliverers[d]->terminate(tout);
			}
		}

		Symbol pull(IPM*& ipm, brahms::output::Source& tout);
		UINT32 push(IPM* msg, brahms::output::Source* tout);

		void audit(ChannelAuditData& data);

		//	add routing entry for PUSHDATA msgs
		void addRoutingEntry(UINT32 msgStreamID, PushDataHandler pushDataHandler, void* pushDataHandlerArgument)
		{
			//	make available
			while(deliverers.size() <= msgStreamID)
				deliverers.push_back(NULL);
				
			//	assert not already
			if (deliverers[msgStreamID])
				ferr << E_INTERNAL << "deliverer exists (" << msgStreamID << ")";
		
			//	create new deliverer thread
			deliverers[msgStreamID] = new Deliverer(
				pushDataHandler, pushDataHandlerArgument,
				msgStreamID, channelInitData, core);

			//	start it
			deliverers[msgStreamID]->start(core.execPars.getu("TimeoutThreadTerm"));
		}

		Deliverer* getDeliverer(UINT32 index)
		{
			if (index >= deliverers.size())
				ferr << E_INTERNAL << "deliverers (routing table) overflow (0x" << hex << index << " of " << dec << deliverers.size() << ")";
			return deliverers[index];
		}

		//	message queue
		IPM_FIFO recvQueue;

		//	pool for this channel
		IPMPool channelSlushPool;

		//	estimated contents of remote buffer for each link
		brahms::os::Mutex auditsMutex;
		vector<QueueAuditDataX> audits;

		//	cached parameters
		struct
		{
			UINT32 PushDataMaxItems;
			UINT32 PushDataMaxBytes;
			UINT32 PushDataWaitStep;
		}
		pars;

	private:

		//	engine data
		brahms::base::Core&				core;

		//	addressing
		ChannelInitData			channelInitData;

		//	audit
		ChannelAuditData		cad;

		//	deliverers
		vector<Deliverer*>		deliverers;
		
		//	first channel to attach does special behaviour during audit
		bool firstToAttach;
	};



////////////////	COMMS LAYER CLASS

	void CommsSendProc(void* arg);
	void CommsRecvProc(void* arg);

	class CommsLayer
	{

	public:

		CommsLayer()
			:
			layerSlushPool("layerSlushPool")
		{
			//	not initialized
			core = NULL;

			//	reference count
			numberChannelsOpen = 0;
		}

		~CommsLayer()
		{
			//	finalize
			if (core)
			{
				MPI::Finalize();
			}
		}
		
		CommsInitData init(brahms::base::Core& p_core)
		{
			CommsInitData ret = {VOICE_UNDEFINED, 0};

			if (!core)
			{
				//	initialize
				int provided = MPI::Init_thread(MPI_THREAD_MULTIPLE);

				string sprovided;
				switch(provided)
				{
				case MPI_THREAD_SINGLE: sprovided = "MPI_THREAD_SINGLE"; break;
				case MPI_THREAD_FUNNELED: sprovided = "MPI_THREAD_FUNNELED"; break;
				case MPI_THREAD_SERIALIZED: sprovided = "MPI_THREAD_SERIALIZED"; break;
				case MPI_THREAD_MULTIPLE: sprovided = "MPI_THREAD_MULTIPLE"; break;
				}

				if (provided != MPI_THREAD_MULTIPLE)
				{
					MPI::Finalize();
					ferr << E_MPI << "insufficient MPI support (MPI_THREAD_MULTIPLE required, " << sprovided << " provided)";
					return ret;
				}

				//	store engine data
				core = &p_core;
			}

			//	ok
			ret.voiceIndex = MPI::COMM_WORLD.Get_rank();
			ret.voiceCount = MPI::COMM_WORLD.Get_size();
			return ret;
		}

		void openChannel(UINT32 remoteIndex)
		{
			//	fail if not initialized
			if (!core) ferr << __FUNCTION__ << "() before MPI initialized";

			//	first opened channel starts sender & receiver threads
			if (!numberChannelsOpen)
			{
				sender.init(*core, this);
				receiver.init(*core, this);
			}

			//	increment reference count
			numberChannelsOpen++;
		}

		void closeChannel(UINT32 remoteIndex, brahms::output::Source& tout)
		{
			//	fail if not initialized
			if (!core) ferr << __FUNCTION__ << "() before MPI initialized";

			//	detach
			numberChannelsOpen--;

			//	if last
			if (!numberChannelsOpen)
			{
				sender.terminate(tout);
				receiver.terminate(tout);
			}
		}

		void push(IPM* msg)
		{
			//	fail if not initialized
			if (!core) ferr << __FUNCTION__ << "() before MPI initialized";

			//	add to queue
			sender.q.push(msg);
		}

		void audit(ChannelAuditData& data)
		{
			//	fail if not initialized
			if (!core) ferr << __FUNCTION__ << "() before MPI initialized";

			//	fill in per-comms-layer data

			//	send
			data.send.uncompressed = sender.simplex.uncompressed;
			data.send.compressed = sender.simplex.compressed;
			data.send.queue = sender.q.size(); // one queue, all channels (=)

			//	recv
			data.recv.uncompressed = receiver.simplex.uncompressed;
			data.recv.compressed = receiver.simplex.compressed;

			//	pool
			layerSlushPool.audit(data.pool);
		}


	
////////////////	SENDER

	public:

		struct Sender
		{
			Sender()
				:
				q(brahms::os::SIGNAL_INFINITE_WAIT, NULL)
			{
				thread = NULL;
				core = NULL;
				flushed = false;
			}

			void init(brahms::base::Core& core, CommsLayer* commsLayer)
			{
				this->core = &core;
				UINT32 TimeoutThreadTerm = core.execPars.getu("TimeoutThreadTerm");
				thread = new brahms::thread::Thread(brahms::thread::TC_SENDER, 0, core);
				thread->start(TimeoutThreadTerm, CommsSendProc, commsLayer);
			}

			void terminate(brahms::output::Source& tout)
			{
				thread->terminate(tout);
				delete thread;
				thread = NULL;
			}

			//	message queue
			IPM_FIFO q;

			//	sender thread
			brahms::thread::Thread* thread;

			//	other data
//			UINT8 format;
//			UINT32 IntervoiceCompression;
			bool flushed;

			//	audit
			ChannelSimplexData simplex;

			//	engine data (set in init())
			brahms::base::Core* core;
		}
		sender;

		//	thread procedure
		void SendProc();

		void flush(brahms::output::Source& fout)
		{
			//	flush send queue (basically, of PUSHDATA msgs that may generate late callbacks)
			//	we can do this by placing a dummy message at the end
			//	of the queue and waiting for it to be sent, indicating
			//	that all real messages have been cleared
			IPM* ipms = layerSlushPool.get(IPMTAG_FLUSH, 0);
			ipms->header().from = 0;
			sender.q.push(ipms);
			
			//	wait for that to be registered
			while(!sender.flushed)
				os_msleep(1);
		}



////////////////	RECEIVER

	public:

		struct Receiver
		{
			Receiver()
			{
				thread = NULL;
				core = NULL;
			}

			void init(brahms::base::Core& core, CommsLayer* commsLayer)
			{
				this->core = &core;
				UINT32 TimeoutThreadTerm = core.execPars.getu("TimeoutThreadTerm");
				thread = new brahms::thread::Thread(brahms::thread::TC_RECEIVER, 0, core);
				thread->start(TimeoutThreadTerm, CommsRecvProc, commsLayer);
			}

			void terminate(brahms::output::Source& tout)
			{
				thread->terminate(tout);
				delete thread;
				thread = NULL;
			}
		
			//	receiver thread
			brahms::thread::Thread* thread;

			//	audit
			ChannelSimplexData simplex;

			//	engine data (set in init())
			brahms::base::Core* core;
		}
		receiver;

		//	thread procedure
		void RecvProc();

		//	pool common to all channels
		IPMPool layerSlushPool;

		//	reference count of openChannel()/closeChannel()
		UINT32 numberChannelsOpen;

	public:

		//	attach channel
		bool attachChannel(ProtocolChannel* channel, UINT32 remoteVoiceIndex)
		{
			bool firstToAttach = attachedChannels.size() == 0;
			
			while(attachedChannels.size() <= remoteVoiceIndex)
				attachedChannels.push_back(NULL);
			if (attachedChannels[remoteVoiceIndex])
				ferr << E_INTERNAL << "channel exists at attach";
			attachedChannels[remoteVoiceIndex] = channel;
			
			return firstToAttach;
		}

		//	is initialized?
		bool isinit()
		{
			return core != NULL;
		}

		//	attached channels
		vector<ProtocolChannel*> attachedChannels;

	private:

		//	engine data (set in init())
		brahms::base::Core* core;

	}
	commsLayer;






////////////////	CHANNEL IMPLEMENTATION

	ProtocolChannel::ProtocolChannel(ChannelInitData p_channelInitData, brahms::base::Core& p_core)
		:
		recvQueue(brahms::os::SIGNAL_INFINITE_WAIT, NULL),
		channelSlushPool("channelSlushPool " + n2s(channelInitData.remoteVoiceIndex+1)),
		core(p_core),
		channelInitData(p_channelInitData)
	{
		firstToAttach = commsLayer.attachChannel(this, channelInitData.remoteVoiceIndex);
		pars.PushDataMaxItems = core.execPars.getu("PushDataMaxItems");
		pars.PushDataMaxBytes = core.execPars.getu("PushDataMaxBytes");
		pars.PushDataWaitStep = core.execPars.getu("PushDataWaitStep");
	}

	ProtocolChannel::~ProtocolChannel()
	{
	}

	void ProtocolChannel::flush(brahms::output::Source& fout)
	{
		//	only we'll do the flush, since there's only one q across all channels
		if (firstToAttach)
			commsLayer.flush(fout);
	}

	void ProtocolChannel::open(brahms::output::Source& fout)
	{
		commsLayer.openChannel(channelInitData.remoteVoiceIndex);
	}

	void ProtocolChannel::terminate(brahms::output::Source& tout)
	{
		//	terminate deliverers
		for (UINT32 d=0; d<deliverers.size(); d++)
		{
			if (deliverers[d])
				deliverers[d]->terminate(tout);
		}

		//	then, detach from comms layer
		commsLayer.closeChannel(channelInitData.remoteVoiceIndex, tout);

		//	flush q
		recvQueue.flush();
	}

	Symbol ProtocolChannel::pull(IPM*& ipm, brahms::output::Source& tout)
	{
		/*
		if (receiver.thread.flagState(brahms::thread::F_THREAD_ERROR))
			ferr << E_COMMS << "channel dropped";
			*/

		//	get message
		/*Symbol result = */recvQueue.pull(ipm);

		//	handle IPMTAG_ERROR
		if (ipm->header().tag == IPMTAG_ERROR)
		{
			ipm->release();
			ferr << E_PEER_ERROR;
		}

		//	ok
		return C_OK;
	}

/*
	UINT32 ProtocolChannel::push(IPM* msg, brahms::output::Source* tout)
	{
		commsLayer.push(msg);

		//	NB: we don't currently do congestion management in the MPI build; i could
		//	have just borrowed it from the sockets build, but it strikes me this should
		//	be common code, so perhaps it needs to be migrated to channel.cpp.
		return 0;
	}
*/
	
	UINT32 ProtocolChannel::push(IPM* ipm, brahms::output::Source* tout)
	{
	/*
		if (sender.thread.flagState(brahms::thread::F_THREAD_ERROR))
		{
			ipm->release();
			ferr << E_COMMS << "channel dropped (" << sender.thread.getThreadIdentifier() << ")";
		}
		*/

		//	if this is PUSHDATA, and the receiving queue is congested, we
		//	hold up the calling thread here as necessary to keep it under
		//	control and avoid concerto loneranger
		IPM_HEADER& header = ipm->header();
		if (header.tag == IPMTAG_PUSHDATA)
		{
			UINT32 msgStreamID = header.msgStreamID;
			UINT32 items, bytes;

			brahms::os::MutexLocker locker(auditsMutex);

			items = audits[msgStreamID].queueAuditData.items;
			bytes = audits[msgStreamID].queueAuditData.bytes;

			if (items > pars.PushDataMaxItems || bytes > pars.PushDataMaxBytes)
			{
				//	if COND_END_RUN_PHASE is set, ignore this condition, else we'll get stuck in a loop
				if (!(core.condition.get(brahms::base::COND_END_RUN_PHASE)))
				{
					IPM* ipms = channelSlushPool.get(IPMTAG_QUERYBUFFER, channelInitData.remoteVoiceIndex);
					ipms->header().from = core.getVoiceIndex();
					ipms->header().msgStreamID = header.msgStreamID;
					commsLayer.push(ipms);
					audits[msgStreamID].queryBufferMsgsUnaccountedFor++;

					//	tell caller to wait a bit
					return pars.PushDataWaitStep;
				}
			}
		}

		//	queue message
		commsLayer.push(ipm);

		//	return 0 to indicate that message was delivered
		return 0;
	}

	void ProtocolChannel::audit(ChannelAuditData& data)
	{
		//	only audit comms layer from first channel, since the data in there is common to all channels
		if (firstToAttach)
			commsLayer.audit(data);

		//	now audit per-channel items
		data.recv.queue += recvQueue.size(); // one queue per channel (+=)
		
		//	deliverers
		for (UINT32 d=0; d<deliverers.size(); d++)
		{
			deliverers[d]->audit(data);
		}
	}




////////////////	SENDER THREAD PROC

	#include "mpi-sender.cpp"



////////////////	RECEIVER THREAD PROC

	#include "mpi-receiver.cpp"




