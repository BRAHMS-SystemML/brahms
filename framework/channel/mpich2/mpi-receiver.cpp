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

	$Id:: mpi-receiver.cpp 2321 2009-11-07 23:08:45Z benjmitch $
	$Rev:: 2321                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-07 23:08:45 +0000 (Sat, 07 Nov 2009)       $
________________________________________________________________

*/





void CommsRecvProc(void* arg)
{
	((CommsLayer*)arg)->RecvProc();
}

void CommsLayer::RecvProc()
{
	brahms::output::Source& tout = receiver.thread->tout;
	
	//	check
	UINT32 numUsedDataMsgsSent = 0;
	UINT32 numPushDataMsgsRecv = 0;

	//	cache some stuff locally
	INT32 localVoiceIndex = core->getVoiceIndex();

	//	start at -1, because 0 means we're done
	INT32 numberPeersAttached = -1;

	while(true)
	{
		//	probe pending message
		MPI::Status status;
		MPI::COMM_WORLD.Probe(MPI::ANY_SOURCE, MPI::ANY_TAG, status);
		UINT32 bytesPending = status.Get_count(MPI::BYTE);
		
		//	get tag (this is MPI_TAG_UB for non-PUSHDATA, or msgStreamID for PUSHDATA)
		UINT32 from = status.Get_source();
		UINT32 tag = status.Get_tag();

		/*	DOCUMENTATION: IPM_POOL
		
			PUSHDATA messages have tag == msgStreamID, and are read into a
			buffer from the appropriate deliverer's pool.
		
			All other messages have tag == MPI_TAG_UB, and are read into a
			buffer from the layer slush pool.
			
			Control messages are returned to the pool directly (see below).
			PUSHDATA messages are routed to a deliverer thread, and that
			thread will repool them. Other messages are placed in the receive
			queue, and the channel pull() function will repool them. Follow
			this documentation tag to find these places.
		*/
		
		//	assert
		if (from >= attachedChannels.size())
			ferr << E_INTERNAL << "channel out of range";

		//	get reference to channel object
		ProtocolChannel* channel = attachedChannels[from];
		
		//	assert
		if (!channel) ferr << E_INTERNAL << "channel is NULL";

		//	deliverer reference
		Deliverer* deliverer = NULL;
		
		//	get buffer from pool
		IPM* recvBuffer = NULL;
		if (tag == MPI_TAG_UB)
		{
			//	not PUSHDATA, use slush pool
			recvBuffer = layerSlushPool.get();
		}
		else
		{
			//	get deliverer from channel object
			deliverer = channel->getDeliverer(tag);
			
			//	PUSHDATA, use deliverer pool
			recvBuffer = deliverer->getIPMFromPool();
		}
		
		//	resize buffer
		recvBuffer->resize____AND_LEAVE_CONTENTS_CORRUPTED____(bytesPending);

		//	receive message
		MPI::COMM_WORLD.Recv(recvBuffer->stream(), bytesPending, MPI::BYTE, MPI::ANY_SOURCE, MPI::ANY_TAG);
		IPM_HEADER& header = recvBuffer->header();
		
		//	assert
		if (from != header.from)
			ferr << E_INTERNAL << "failed assert from != header.from";
		
		//	audit
		receiver.simplex.uncompressed += recvBuffer->uncompressedSize();
		receiver.simplex.compressed += recvBuffer->compressedSize();

		//	handle HELLO
		if (header.tag == IPMTAG_HELLO)
		{
			if (numberPeersAttached == -1) numberPeersAttached = 0;
			numberPeersAttached++;
		}
		
		//	handle GOODBYE
		if (header.tag == IPMTAG_GOODBYE)
		{
			numberPeersAttached--;
		}

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
				recvBuffer->release();
				
				//	no action, other than above to decrement our reference counter
				break;
			}

			case IPMTAG_CANCEL:
			{
				//	return buffer to pool
				recvBuffer->release();

				//	ok				
				core->condition.set(brahms::base::COND_PEER_CANCEL);
				break;
			}

			case IPMTAG_KEEPALIVE:
			{
				//	return buffer to pool
				recvBuffer->release();
				
				//	no action, other than implicitly to reset the watchdog timer and avoid this thread being killed
				break;
			}

			case IPMTAG_USEDDATA:
			{
				//	subtract from audit for this link
				brahms::os::MutexLocker locker(channel->auditsMutex);

				//	assert
				UINT32 msgStreamID = header.msgStreamID;
				if (header.audit.bytes > channel->audits[msgStreamID].queueAuditData.bytes)
					ferr << E_INTERNAL << "USEDDATA audit arithmetic overflow";

				channel->audits[msgStreamID].queueAuditData.items -= header.audit.items;
				channel->audits[msgStreamID].queueAuditData.bytes -= header.audit.bytes;
				channel->audits[msgStreamID].queryBufferMsgsUnaccountedFor--;

				//	return buffer to pool
				recvBuffer->release();
				
				//	no further action
				break;
			}

			case IPMTAG_QUERYBUFFER:
			{
				//	get deliverer from channel object
				deliverer = channel->getDeliverer(header.msgStreamID);
				
				// do audit by pushing NULL msg
				QueueAuditData audit = deliverer->queueAudit();

				//	if non-zero, poke a IPMTAG_USEDDATA into the queue going back
				if (audit.items)
				{
					IPM* ipms = layerSlushPool.get(IPMTAG_USEDDATA, header.from);
					ipms->header().from = localVoiceIndex;
					ipms->header().msgStreamID = header.msgStreamID;
					ipms->header().audit = audit;
					sender.q.push(ipms);

					numUsedDataMsgsSent++;
				}

				//	return buffer to pool
				recvBuffer->release();

				//	ok
				break;
			}

			case IPMTAG_PUSHDATA:
			{
				//	route it directly, not via the queue

				//	check
				numPushDataMsgsRecv++;

				//	get deliverer from channel object
				Deliverer* deliverer = channel->getDeliverer(header.msgStreamID);

				//	push (and audit deliverer queue)
				deliverer->push(recvBuffer);

				//	ok
				break;
			}

			case IPMTAG_ERROR:
			{
				//	set global error state
				core->condition.set(brahms::base::COND_PEER_ERROR);

				//	deliberate drop-through...
			}

			default:
			{
				//	add to queue
				channel->recvQueue.push(recvBuffer);

				//	ok
				break;
			}
		}

		//	escape
		if (!numberPeersAttached)
		{
			tout << "RecvProc() break: numberPeersAttached == 0" << D_VERB;
			break;
		}
	}

	//	check
	tout << "numPushDataMsgsRecv = " << numPushDataMsgsRecv << D_VERB;
	tout << "numUsedDataMsgsSent = " << numUsedDataMsgsSent << D_VERB;
}


