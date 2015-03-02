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

$Id:: sockets-receiver.cpp 2349 2009-11-10 16:17:47Z benjm#$
$Rev:: 2349                                                $
$Author:: benjmitch                                        $
$Date:: 2009-11-10 16:17:47 +0000 (Tue, 10 Nov 2009)       $
________________________________________________________________

*/

#include "sockets/sockets.h"

////////////////	RECEIVER PROCEDURE

void ReceiverThreadProc(void* arg)
{
	((ProtocolChannel*)arg)->MemberReceiverThreadProc();
}

UINT32 IPMTAG_USEDDATA_order = 0;

void ProtocolChannel::MemberReceiverThreadProc()
{
	brahms::output::Source& tout(receiver.thread.tout);
	bool receivedGoodbye = false;

	//	check
	UINT32 numMsgsRecv = 0;
	UINT32 numPushDataMsgsRecv = 0;
	UINT32 numUsedDataMsgsSentAfterPush = 0;
	UINT32 numUsedDataMsgsSentAfterQuery = 0;
	UINT32 numPartialReceiveOnPeek = 0;

	//	peek buffer
	IPM_HEADER peekHeader = {0};

	//	try
	try
	{
		//	auto-release IPMs
		SAFE_IPM messageBeingUncompressed;
		SAFE_IPM messageBeingReceived;

		//	loop until GOODBYE is sent
		while(true)
		{

////////////////	(a) pull one complete message into our local buffer

			//	recv() (see WAIT STATES)
			REPORT_THREAD_WAIT_STATE_IN("recv()");

			//	a partial receive during peek cannot be "continued", like a partial receive during
			//	not-peek, because the bytes aren't read. so if we get a partial receive, we just
			//	sleep fleetingly, and try again...
			while(true)
			{
				int result = recv(dataSocket, (char*)&peekHeader, sizeof(IPM_HEADER), MSG_PEEK);
				if (result == OS_SOCKET_ERROR)
					ferr << E_COMMS << "failed at recv() (" << socketsErrorString(OS_LASTERROR) << ")";
				if (result == sizeof(IPM_HEADER)) break;
				if (result > ((int)sizeof(IPM_HEADER)))
					ferr << E_INTERNAL << "result > sizeof(IPM_HEADER) at peek";
				if (result == 0)
					ferr << E_COMMS << "channel dropped (socket returned 0 from recv())";

				//	sleep fleetingly
				//tout << "sleep" << D_INFO;
				numPartialReceiveOnPeek++;
				brahms::os::msleep(1);
			}

			/*	DOCUMENTATION: IPM_POOL

				PUSHDATA messages are read directly into a buffer from the
				pool associated with their deliverer thread. Messages other
				than PUSHDATA are put into a buffer from the channel slush
				pool.

				Control messages are returned to the pool directly (see below).
				PUSHDATA messages are routed to a deliverer thread, and that
				thread will repool them. Other messages are placed in the receive
				queue, and the channel pull() function will repool them. Follow
				this documentation tag to find these places.
			*/

			//	if it's a PUSHDATA, get the deliverer that will handle it, and get the receive buffer from its pool
			Deliverer* deliverer = NULL;

			//	switch on header tag
			switch(peekHeader.tag)
			{
				case IPMTAG_PUSHDATA:
				{
					//	get deliverer
					if (peekHeader.msgStreamID >= receiver.deliverers.size())
						ferr << E_INTERNAL << "deliverers (routing table) overflow (0x" << hex << peekHeader.msgStreamID << " of " << dec << receiver.deliverers.size() << ")";
					deliverer = receiver.deliverers[peekHeader.msgStreamID];

					//	get buffer from deliverer's pool (these are appropriately sized for PUSHDATA through this link)
					messageBeingReceived.ipm() = deliverer->getIPMFromPool();

					//	ok
					break;
				}

				case IPMTAG_QUERYBUFFER:
				{
					//	get deliverer
					if (peekHeader.msgStreamID >= receiver.deliverers.size())
						ferr << E_INTERNAL << "deliverers (routing table) overflow (0x" << hex << peekHeader.msgStreamID << " of " << dec << receiver.deliverers.size() << ")";
					deliverer = receiver.deliverers[peekHeader.msgStreamID];

					//	deliberate drop-through...
				}

				default:
				{
					//	get buffer from channel slush pool
					messageBeingReceived.ipm() = channelSlushPool.get();

					//	ok
					break;
				}
			}

			//	audit
			numMsgsRecv++;
			receiver.messageReceived = true;

			//	get message size
			UINT32 totalBytesInMessageCompressed = sizeof(IPM_HEADER) + peekHeader.bytesAfterHeaderCompressed;
			UINT32 totalBytesInMessageUncompressed = sizeof(IPM_HEADER) + peekHeader.bytesAfterHeaderUncompressed;

			/*	DOCUMENTATION: POOL_BUFFER_RESIZE

				Note, that if we're running using compression, we'll be constantly resize()ing
				these buffers here, and below at POOL_BUFFER_RESIZE (again). They'll get
				smaller here (for the compressed message) and larger below (for the uncompressed
				message). This is why we've got our own implementation, IPM, instead of
				using std::vector<BYTE>, because we can make sure the implementation is efficient
				when used in this way.
			*/
			messageBeingReceived.ipm()->resize____AND_LEAVE_CONTENTS_CORRUPTED____(totalBytesInMessageCompressed);

			//	want to receive "messageBytes"
			UINT32 bytesReceivedIntoRecvBuffer = 0;
			while(bytesReceivedIntoRecvBuffer < totalBytesInMessageCompressed)
			{
				int result = recv(dataSocket, (char*) messageBeingReceived.ipm()->stream(bytesReceivedIntoRecvBuffer),
					totalBytesInMessageCompressed-bytesReceivedIntoRecvBuffer, 0);
				if (result == OS_SOCKET_ERROR)
					ferr << E_COMMS << "failed at recv() (" << socketsErrorString(OS_LASTERROR) << ")";
				bytesReceivedIntoRecvBuffer += result;
			}
			if (bytesReceivedIntoRecvBuffer != totalBytesInMessageCompressed) ferr << E_INTERNAL;

			REPORT_THREAD_WAIT_STATE_OUT("recv()");

			//	reset watchdog
			receiver.watchdog.reset();

////////////////	(b) handle that message

			//	get pointers
			IPM_HEADER& header = messageBeingReceived.ipm()->header();

			//	read message header
			if (header.sig != IPM_SIGNATURE)
			{
				//	dump header
				tout << "header.sig:                           0x" << hex << header.sig << D_INFO;
				tout << "header.tag:                           0x" << hex << (UINT32)header.tag << D_INFO;
				tout << "header.fmt:                           0x" << hex << (UINT32)header.fmt << D_INFO;
				tout << "header.from:                          0x" << hex << header.from << D_INFO;
				tout << "header.bytesAfterHeaderUncompressed:  0x" << hex << header.bytesAfterHeaderUncompressed << D_INFO;
				tout << "header.bytesAfterHeaderCompressed:    0x" << hex << header.bytesAfterHeaderCompressed << D_INFO;

				//	throw
				ferr << E_COMMS << "failed at assert sig (on msg with tag " << brahms::base::TranslateIPMTAG(header.tag) << ")";
			}

			//	assert routing
			if (header.from != channelInitData.remoteVoiceIndex)
				ferr << E_COMMS << "routing problem (" << header.from << " != " << channelInitData.remoteVoiceIndex << ")";

			//	audit
			receiver.simplex.compressed += totalBytesInMessageCompressed;
			receiver.simplex.uncompressed += totalBytesInMessageUncompressed;

			//	report
			if (header.tag <= IPMTAG_MAX_D_VERB)
				tout << "received " << brahms::base::TranslateIPMTAG(header.tag) << D_VERB;
			else
				tout << "received " << brahms::base::TranslateIPMTAG(header.tag) << D_FULL;

			//	handle control messages in this thread
			switch(header.tag)
			{
				case IPMTAG_GOODBYE:
				{
					//	return buffer to pool
					messageBeingReceived.release();

					receivedGoodbye = true;
					break;
				}

				case IPMTAG_CANCEL:
				{
					//	return buffer to pool
					messageBeingReceived.release();

					core.condition.set(brahms::base::COND_PEER_CANCEL);
					break;
				}

				case IPMTAG_USEDDATA:
				{
					//	subtract from audit for this link
					brahms::os::MutexLocker locker(sender.auditsMutex);

					//	assert
					UINT32 msgStreamID = header.msgStreamID;
					if (header.audit.bytes > sender.audits[msgStreamID].queueAuditData.bytes)
						ferr << E_INTERNAL << "USEDDATA audit arithmetic overflow";

					sender.audits[msgStreamID].queueAuditData.items -= header.audit.items;
					sender.audits[msgStreamID].queueAuditData.bytes -= header.audit.bytes;
					sender.audits[msgStreamID].queryBufferMsgsUnaccountedFor--;

					//	return buffer to pool
					messageBeingReceived.release();

					//	no further action
					break;
				}

				case IPMTAG_QUERYBUFFER:
				{
					//	do audit
					QueueAuditData audit = deliverer->queueAudit();

					//	if non-zero, poke a IPMTAG_USEDDATA into the queue going back
					if (audit.items)
					{
						IPM* ipms = channelSlushPool.get(IPMTAG_USEDDATA, channelInitData.remoteVoiceIndex);
						ipms->header().from = core.getVoiceIndex();
						ipms->header().order = IPMTAG_USEDDATA_order++;
						ipms->header().msgStreamID = header.msgStreamID;
						ipms->header().audit = audit;
						sender.q.push(ipms);

						numUsedDataMsgsSentAfterQuery++;
					}

					//	return buffer to pool
					messageBeingReceived.release();

					//	ok
					break;
				}

				case IPMTAG_PUSHDATA:
				{
					//	route it directly, not via the queue

					//	check
					numPushDataMsgsRecv++;

					//	if compressed
					if (header.fmt == IPMFMT_DEFLATE)
					{
						//	must have function
						if (!compressFunction) ferr << "compression module did not load, but peer voice sent compressed message; cannot continue";

						//	get another buffer from pool, to do the copy into
						messageBeingUncompressed.ipm() = deliverer->getIPMFromPool();

						/*	DOCUMENTATION: POOL_BUFFER_RESIZE */

						//	resize copy buffer
						messageBeingUncompressed.ipm()->resize____AND_LEAVE_CONTENTS_CORRUPTED____(totalBytesInMessageUncompressed);

						//	only inflate if non-empty
						if (header.bytesAfterHeaderUncompressed)
						{
							//	get pointer to data segment
							BYTE* data = messageBeingReceived.ipm()->body();

							//	inflate
							UINT32 dstBytes = header.bytesAfterHeaderUncompressed;
							const char* err = compressFunction(
								data,
								header.bytesAfterHeaderCompressed,
								messageBeingUncompressed.ipm()->body(),
								&dstBytes,
								-1
								);
							if (err) ferr << E_INTERNAL << err;

							//	assert
							if (dstBytes != header.bytesAfterHeaderUncompressed)
								ferr << E_COMMS << "size mismatch after decompression, " << dstBytes << " != " << header.bytesAfterHeaderUncompressed;
						}

						//	but always put in header
						messageBeingUncompressed.ipm()->header() = header;

						//	swap with original buffer (so that messageBeingReceived now contains the uncompressed message)
						swap(messageBeingUncompressed.ipm(), messageBeingReceived.ipm());

						//	return temp buffer
						messageBeingUncompressed.release();
					}

					//	push (and audit deliverer queue only if the sender has asked for this)
					deliverer->push(messageBeingReceived.retrieve());

					//	ok
					break;
				}

				case IPMTAG_ERROR:
				{
					core.condition.set(brahms::base::COND_PEER_ERROR);
					//	deliberate drop-through...
				}

				default:
				{
					//	add to queue
					receiver.q.push(messageBeingReceived.retrieve());

					//	ok
					break;
				}
			}

			//	break if GOODBYE
			if (receivedGoodbye) break;

		} // while(true) loop until received GOODBYE
	}

	//	trace exception
	catch(brahms::error::Error& e)
	{
		receiver.thread.storeError(e, tout);
	}

	//	trace exception
	catch(exception se)
	{
		brahms::error::Error e(E_STD, se.what());
		receiver.thread.storeError(e, tout);
	}

	//	trace exception
	catch(...)
	{
		brahms::error::Error e(E_UNRECOGNISED_EXCEPTION);
		receiver.thread.storeError(e, tout);
	}

	//	TODO REDUCED_USEDDATA_AMOUNT
	//
	//	TODO: it would be nice to reduce the number of USEDDATA sent back; we only have to
	//	send them back every few PUSHDATA messages, when the sender's estimate of the buffer
	//	fullness is half or more, since when the sender's estimate is less than that, there's
	//	no impact at the sender anyway. one way to do this is to send the sender's estimate
	//	of how full the buffer is with PUSHDATA, which could easily go in the header. however,
	//	we have the restriction that the PUSHDATA msgs to multiple receivers of the same
	//	output must be *identical* (search tag PUSHDATA_IDENTICAL_TO_ALL_SENDERS), and since
	//	the buffer estimates are per-receiver, they are not identical. therefore, this strategy
	//	won't currently work. needs more thought.

	tout << "numMsgsRecv = " << numMsgsRecv << D_VERB;
	tout << "numPushDataMsgsRecv = " << numPushDataMsgsRecv << D_VERB;
	tout << "numUsedDataMsgsSentAfterPush = " << numUsedDataMsgsSentAfterPush << D_VERB;
	tout << "numUsedDataMsgsSentAfterQuery = " << numUsedDataMsgsSentAfterQuery << D_VERB;
	tout << "numPartialReceiveOnPeek = " << numPartialReceiveOnPeek << D_VERB;
}
