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

$Id:: sockets.cpp 2366 2009-11-13 00:46:58Z benjmitch      $
$Rev:: 2366                                                $
$Author:: benjmitch                                        $
$Date:: 2009-11-13 00:46:58 +0000 (Fri, 13 Nov 2009)       $
________________________________________________________________

*/

// Ensure __NIX__ and __WIN__ etc are set up
#ifndef BRAHMS_BUILDING_ENGINE
#define BRAHMS_BUILDING_ENGINE
#endif
#include "brahms-client.h"

#include "sockets-support.h"
#include <string>
using std::string;
#include <sstream>
using std::stringstream;
#include "deliverer.h"
#include "base/brahms_math.h"
using brahms::math::unitIndex;
#include "compress.h"

////////////////	THREAD PROCEDURE DECLARATIONS
/*
	These static thread procedures take one argument, the
	ProtocolChannel object, and simply call the member
	function thread procedure on that object.
*/

void SenderThreadProc(void* arg);
void ReceiverThreadProc(void* arg);

////////////////	COMMS PROTOCOL
const UINT32 CONNECTION_ATTEMPT_INTERVAL_MS = 25;

////////	PROTOCOL CHANNEL
struct ProtocolChannel
{
	//	sockets
	OS_SOCKET				dataSocket;			//	all send/recv goes on this one
	OS_SOCKET				serverListenSocket;	//	this one is the local socket used by the server only

	//	channel data
	bool					server;
	UINT32					localPort;
	UINT32					remotePort;

	//	engine data
	brahms::base::Core&		core;
	ChannelInitData			channelInitData;

	//	cached parameters
	struct
	{
		UINT32 SocketsTimeout;
		UINT32 TimeoutThreadTerm;
		UINT32 PushDataMaxItems;
		UINT32 PushDataMaxBytes;
		UINT32 PushDataWaitStep;
		INT32 localVoiceIndex;
	}
	pars;


	//	buffer pool
	IPMPool channelSlushPool;



////////////////	CHANNEL

	ProtocolChannel(ChannelInitData channelInitData, brahms::base::Core& p_core)
		:
		core(p_core),
		channelSlushPool("channelSlushPool " + n2s(channelInitData.remoteVoiceIndex+1)),
		sender(channelInitData.remoteVoiceIndex, p_core),
		receiver(channelInitData.remoteVoiceIndex, p_core)
	{
		//	store stuff
		this->channelInitData = channelInitData;

		//	start with invalid sockets
		dataSocket = OS_INVALID_SOCKET;
		serverListenSocket = OS_INVALID_SOCKET;

		//	are we server or client?
		server = core.getVoiceIndex() < channelInitData.remoteVoiceIndex;

		//	cache parameters
		pars.SocketsTimeout = core.execPars.getu("SocketsTimeout");
		pars.TimeoutThreadTerm = core.execPars.getu("TimeoutThreadTerm");
		pars.PushDataMaxItems = core.execPars.getu("PushDataMaxItems");
		pars.PushDataMaxBytes = core.execPars.getu("PushDataMaxBytes");
		pars.PushDataWaitStep = core.execPars.getu("PushDataWaitStep");
		pars.localVoiceIndex = core.getVoiceIndex();

		//	choose port numbers
		UINT32 SocketsBasePort = core.execPars.getu("SocketsBasePort");
		localPort = listenPort(SocketsBasePort, core.getVoiceCount(), core.getVoiceIndex(), channelInitData.remoteVoiceIndex);
		remotePort = listenPort(SocketsBasePort, core.getVoiceCount(), channelInitData.remoteVoiceIndex, core.getVoiceIndex());

		//	misc
		sender.flushed = false;
	}

	void flush(brahms::output::Source& tout)
	{
		//	flush send queue (basically, of PUSHDATA msgs that may generate late callbacks)
		//	we can do this by placing a dummy message at the end
		//	of the queue and waiting for it to be sent, indicating
		//	that all real messages have been cleared
		IPM* ipms = channelSlushPool.get(IPMTAG_FLUSH, channelInitData.remoteVoiceIndex);
		ipms->header().from = core.getVoiceIndex();
		sender.q.push(ipms);

		//	wait for that to be registered
		while(!sender.flushed)
			os_msleep(1);
	}

	void listen()
	{
		//	if not server, do nowt
		if (!server) return;

		//	fout
		core.caller.tout << "starting listen on channel to Voice " << unitIndex(channelInitData.remoteVoiceIndex) << D_FULL;

		//	open listening socket (start server)
		sockaddr_in con;
		con.sin_family = AF_INET;
		con.sin_addr.s_addr = inet_addr( "0.0.0.0" );
		con.sin_port = htons( localPort );

		//	create socket
		if ((serverListenSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP )) == OS_INVALID_SOCKET )
			ferr << E_COMMS << "listen failed at create (" + socketsErrorString(OS_LASTERROR) + ")";

#ifdef __NIX__

		//	set socket option to reuse local addresses
		int optval = 1;
		if ( setsockopt(serverListenSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == OS_SOCKET_ERROR )
			ferr << E_COMMS << "listen failed at setsockopt (" + socketsErrorString(OS_LASTERROR) + ")";

#endif

		//	bind socket
		if ( bind( serverListenSocket, (OS_SOCKADDR*) &con, sizeof(con) ) == OS_SOCKET_ERROR)
			ferr << E_COMMS << "listen failed at bind (" + socketsErrorString(OS_LASTERROR) + ")";

		//	listen on socket
		if ( ::listen( serverListenSocket, 1 ) == OS_SOCKET_ERROR )
			ferr << E_COMMS << "listen failed at listen (" + socketsErrorString(OS_LASTERROR) + ")";

		//	mark the server socket as non-blocking so we can poll it
		//	rather than having it block when we call accept()
		os_nonblock(serverListenSocket);

		//	ok
		core.caller.tout << "listening!" << D_FULL;
	}

	void open(brahms::output::Source& fout)
	{
		//	connect() watchdog timer
		brahms::os::Timer watchdog;

		//	server accepts a connection
		if (server)
		{
			//	fout
			UINT32 port = localPort;
			stringstream ss;
			ss << channelInitData.remoteAddress << ":" << port;
			string addr = ss.str();
			core.caller.tout << "expecting connection from " << addr << "..." << D_FULL;

			//	accept pending connection from remote Voice
			while(true)
			{
				//	accept incoming connection
				if ((dataSocket = accept( serverListenSocket, NULL, NULL )) != OS_INVALID_SOCKET)
				{
					core.caller.tout << " CONNECTED (in " << watchdog.elapsedMS() << "mS)" << D_FULL;
					break;
				}

				//	otherwise, find out why
				int err = OS_LASTERROR;
				if (err != OS_WOULDBLOCK)
					ferr << E_COMMS << "failed at accept() (" + socketsErrorString(err) + ")";

				//	wait for remote Voice to try and connect
				core.caller.tout << "."; // "." means "no connection yet"
				os_msleep(CONNECTION_ATTEMPT_INTERVAL_MS);
				if (watchdog.elapsedMS() >= pars.SocketsTimeout)
					ferr << E_COMMS_TIMEOUT << "accepting connection from " << addr;
			}
		}

		//	client initiates a connection
		else
		{
			//	fout
			UINT32 port = remotePort;
			stringstream ss;
			ss << channelInitData.remoteAddress << ":" << port;
			string addr = ss.str();
			core.caller.tout << "attempting connection to " << addr << "..." << D_VERB;

			//	open socket
			if ((dataSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP )) == OS_INVALID_SOCKET )
				ferr << E_COMMS << "failed to create socket (" + socketsErrorString(OS_LASTERROR) + ")";

			//	address details
			sockaddr_in con;
			con.sin_family = AF_INET;
			con.sin_addr.s_addr = inet_addr( channelInitData.remoteAddress.c_str() );
			con.sin_port = htons( port );

			//	set push socket to non-blocking for connection phase
			os_nonblock(dataSocket);

			//	connect socket
			bool connected = false;

			while(true)
			{
				//	try to connect (asynchronously)
				UINT32 err = 0;
				if ( ::connect( dataSocket, (OS_SOCKADDR*) &con, sizeof(con) ) != OS_SOCKET_ERROR )
				{
					connected = true;
				}
				else
				{
					err = OS_LASTERROR;
					if (err == OS_ISCONN)
					{
						//	already connected (connection operation has completed)
						connected = true;
					}
				}

				//	if connected, report and break infinite loop
				if (connected)
				{
					core.caller.tout << " CONNECTED (in " << watchdog.elapsedMS() << "mS)" << D_VERB;
					break;
				}

				//	otherwise, handle error
				char status = '.'; // IN PROGRESS
				switch(err)
				{
					//	if would block, server may not be ready
					//	if connection refused, server may not be ready
					case OS_WOULDBLOCK:
					case OS_CONNREFUSED:

						status = 'R'; // REFUSED
						//	deliberate drop-through...

					//	most likely, the result was OS_EINPROGRESS, meaning
					//	the connection has been initiated but not yet completed
					case OS_INPROGRESS:
					case OS_ALREADY:

						//	timeout
						if (watchdog.elapsedMS() >= pars.SocketsTimeout)
							ferr << E_COMMS_TIMEOUT << "connecting to " << addr << " (" << socketsErrorString(err) << ")";

						//	else, wait a bit and try again
						core.caller.tout << status;
						os_msleep(CONNECTION_ATTEMPT_INTERVAL_MS);
						continue;

					//	other error
					default:
					{
						core.caller.tout << " FAILED" << D_VERB;
						ferr << E_COMMS << "failed at connect() (" << err << " = " << socketsErrorString(err) << ")";
					}
				}
			}
		}

		//	for remainder of operation, socket will have send() and recv()
		//	called on it from send/recv threads, so we can safely set it
		//	to blocking mode, simplifying the logic of using it
		os_block(dataSocket);

		//	handle NAGLE algorithm
		os_enablenagle(dataSocket, core.execPars.getu("SocketsUseNagle"));

		//	create threads
		sender.thread.start(pars.TimeoutThreadTerm, SenderThreadProc, this);
		receiver.thread.start(pars.TimeoutThreadTerm, ReceiverThreadProc, this);
	}

	void terminate(brahms::output::Source& fout)
	{
		fout << "terminate() called on channel to Voice " << unitIndex(channelInitData.remoteVoiceIndex) << D_VERB;

		sender.terminate(fout);
		receiver.terminate(fout);

		//	close socket
		if (dataSocket != OS_INVALID_SOCKET)
			::os_closesocket(dataSocket);
		dataSocket = OS_INVALID_SOCKET;

		//	close socket
		if (serverListenSocket != OS_INVALID_SOCKET)
			::os_closesocket(serverListenSocket);
		serverListenSocket = OS_INVALID_SOCKET;
	}

	void stopRouting(brahms::output::Source& fout)
	{
		receiver.stopRouting(fout);
	}

	void audit(ChannelAuditData& data)
	{
		sender.audit(data);
		receiver.audit(data);
		channelSlushPool.audit(data.pool);
	}



////////////////	SENDER

	void MemberSenderThreadProc();

	#define SocketsKeepAliveInterval 1000

	struct Sender
	{
		Sender(INT32 remoteVoiceIndex, brahms::base::Core& core)
			:
			q(SocketsKeepAliveInterval, NULL),
			thread(brahms::thread::TC_SENDER, remoteVoiceIndex, core)
		{
			memset(&simplex, 0, sizeof(simplex));

			//	init sender
			IntervoiceCompression = core.execPars.getu("IntervoiceCompression");
			if (IntervoiceCompression > 9) ferr << E_EXECUTION_PARAMETERS << "invalid IntervoiceCompression";
			format = IntervoiceCompression ? IPMFMT_DEFLATE : IPMFMT_UNCOMPRESSED;
			if (format == IPMFMT_DEFLATE && !compressFunction) ferr << "compression module did not load - must turn off IntervoiceCompression";
		}

		void audit(ChannelAuditData& data)
		{
			data.send.uncompressed += simplex.uncompressed;
			data.send.compressed += simplex.compressed;
			data.send.queue += q.size();
		}

		void terminate(brahms::output::Source& tout)
		{
			thread.terminate(tout);
			q.flush();
		}

		//	message queue
		IPM_FIFO q;

		//	sender thread
		brahms::thread::Thread thread;

		//	sender audit data
		ChannelSimplexData simplex;

		//	other data
		UINT8 format;
		UINT32 IntervoiceCompression;
		bool flushed;

		//	estimated contents of remote buffer for each link
		brahms::os::Mutex auditsMutex;
		vector<QueueAuditDataX> audits;
	}
	sender;

	UINT32 push(IPM* ipm, brahms::output::Source* tout)
	{
		if (sender.thread.flagState(brahms::thread::F_THREAD_ERROR))
		{
			ipm->release();
			ferr << E_COMMS << "channel dropped (" << sender.thread.getThreadIdentifier() << ")";
		}

		//	if this is PUSHDATA, and the receiving queue is congested, we
		//	hold up the calling thread here as necessary to keep it under
		//	control and avoid concerto loneranger
		IPM_HEADER& header = ipm->header();
		if (header.tag == IPMTAG_PUSHDATA)
		{
			UINT32 msgStreamID = header.msgStreamID;
			UINT32 items, bytes;

			brahms::os::MutexLocker locker(sender.auditsMutex);

			items = sender.audits[msgStreamID].queueAuditData.items;
			bytes = sender.audits[msgStreamID].queueAuditData.bytes;

			if (items > pars.PushDataMaxItems || bytes > pars.PushDataMaxBytes)
			{
				//	if COND_END_RUN_PHASE is set, ignore this condition, else we'll get stuck in a loop
				if (!(core.condition.get(brahms::base::COND_END_RUN_PHASE)))
				{
					IPM* ipms = channelSlushPool.get(IPMTAG_QUERYBUFFER, channelInitData.remoteVoiceIndex);
					ipms->header().from = core.getVoiceIndex();
					ipms->header().msgStreamID = header.msgStreamID;
					sender.q.push(ipms);
					sender.audits[msgStreamID].queryBufferMsgsUnaccountedFor++;

					//	tell caller to wait a bit
					return pars.PushDataWaitStep;
				}
			}
		}

		//	queue message
		sender.q.push(ipm);

		//	return 0 to indicate that message was delivered
		return 0;
	}



////////////////	RECEIVER

	void MemberReceiverThreadProc();

	#define SOCKETS_PULL_WAITSTEP 100 // have to come back every now and then to check for waited > SocketsTimeout; this is usually measured in seconds, so anything sub-second will do here since it only gets hit in an error condition so it's not a performance concern

	struct Receiver
	{
		Receiver(INT32 remoteVoiceIndex, brahms::base::Core& core)
			:
			q(SOCKETS_PULL_WAITSTEP, NULL),
			thread(brahms::thread::TC_RECEIVER, remoteVoiceIndex, core)
		{
			memset(&simplex, 0, sizeof(simplex));
			messageReceived = false;
		}

		void audit(ChannelAuditData& data)
		{
			data.recv.uncompressed += simplex.uncompressed;
			data.recv.compressed += simplex.compressed;
			data.recv.queue += q.size();

			for (UINT32 d=0; d<deliverers.size(); d++)
			{
				// some may be NULL (it's a sparse routing table)
				if (deliverers[d])
					deliverers[d]->audit(data);
			}
		}

		#define EXTRA_WAIT_FOR_RECEIVER 5000

		void terminate(brahms::output::Source& fout)
		{
			//	wait for thread to terminate or timeout
			while(true)
			{
				//	check for finished
				if (!thread.isActive())
				{
					//	terminate thread normally
					thread.terminate(fout);
					break;
				}

				//	check for timeout
				if (watchdog.elapsedMS() > EXTRA_WAIT_FOR_RECEIVER)
				{
					//	thread cannot recover now by receiving a message, we've already called terminate!
					//	TODO: we don't really need to check sockets like this, since it must either
					//	break the connection forcibly (recv() will return an error) or provide us with
					//	IPMTAG_GOODBYE to exit the thread. we keep this check in here purely whilst we're
					//	developing concerto, so that we get better info on what goes wrong if we've coded
					//	badly somewhere.
					thread.terminate(fout, EXTRA_WAIT_FOR_RECEIVER); // already waited EXTRA_WAIT_FOR_RECEIVER, so shave this off TimeoutThreadTerm
					break;
				}

				//	wait a mo!
				brahms::os::msleep(1);
			}

			//	stop routing (in case it hasn't been done explicitly)
			stopRouting(fout);

			//	delete deliverer threads
			for (UINT32 d=0; d<deliverers.size(); d++)
			{
				//	thread are terminated in stopRouting(), but we keep them existing
				//	just in case any late PUSHDATA messages come through; they can
				//	go harmlessly into the queue of the deliverer; now that the receiver
				//	is terminated, we can safely delete the deliverer objects.

				//	some may be NULL (it's a sparse routing table)
				if (deliverers[d])
					delete deliverers[d];
			}
			deliverers.clear();

			//	flush q
			q.flush();
		}

		void stopRouting(brahms::output::Source& fout)
		{
			for (UINT32 d=0; d<deliverers.size(); d++)
			{
				//	some may be NULL (it's a sparse routing table)
				if (deliverers[d])
					deliverers[d]->terminate(fout);
			}
		}

		bool active()
		{
			return thread.isActive();
		}

		//	channel timeout watchdog timer
		brahms::os::Timer watchdog;

		//	message queue
		IPM_FIFO q;

		//	receiver thread
		brahms::thread::Thread thread;

		//	receiver audit data
		ChannelSimplexData simplex;

		//	deliverers array is sparse, and is its own routing table (that is,
		//	the nth entry is the deliverer that is targeted by PUSHDATA messages
		//	with a msgStreamID of n)
		vector<Deliverer*> deliverers;

		//	flag that a message has come in
		bool messageReceived;
	}
	receiver;

	Symbol pull(IPM*& ipm, brahms::output::Source& tout)
	{
		//	get message
		UINT32 waited = 0;
		while(true)
		{
			Symbol result = receiver.q.pull(ipm);
			if (result != E_SYNC_TIMEOUT)
			{
				if (ipm->header().tag == IPMTAG_ERROR)
				{
					ipm->release();
					ferr << E_PEER_ERROR;
				}

				if (ipm->header().tag == IPMTAG_KEEPALIVE)
				{
					ipm->release();
					waited = 0;
					continue;
				}

				break;
			}

			//	check for any msg received
			if (receiver.messageReceived)
			{
				receiver.messageReceived = false;
				waited = 0;
			}

			waited += SOCKETS_PULL_WAITSTEP;
			if (waited >= pars.SocketsTimeout)
				ferr << E_COMMS_TIMEOUT << "no message received after " << waited << " milliseconds (" << receiver.thread.getThreadIdentifier() << ")";

			if (!receiver.active())
				ferr << E_COMMS << "channel dropped (" << receiver.thread.getThreadIdentifier() << ")";
		}

		//	ok
		return C_OK;
	}

	void addRoutingEntry(UINT32 msgStreamID, PushDataHandler pushDataHandler, void* pushDataHandlerArgument)
	{
		//	create new deliverer thread
		Deliverer* deliverer = new Deliverer(pushDataHandler, pushDataHandlerArgument, msgStreamID, channelInitData, core);

		//	add new entry to routing table (i.e. store deliverer object in sparse table)
		if (receiver.deliverers.size() <= msgStreamID)
		{
			receiver.deliverers.resize(msgStreamID + 1);
		}
		else
		{
			if (receiver.deliverers[msgStreamID])
				ferr << E_INTERNAL << "routing table location taken on addRoutingEntry()";
		}
		receiver.deliverers[msgStreamID] = deliverer;

		//	start new deliverer thread
		deliverer->start(pars.TimeoutThreadTerm);
	}

};
