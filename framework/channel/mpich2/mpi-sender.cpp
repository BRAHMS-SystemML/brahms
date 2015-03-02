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

	$Id:: mpi-sender.cpp 2366 2009-11-13 00:46:58Z benjmitch   $
	$Rev:: 2366                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-13 00:46:58 +0000 (Fri, 13 Nov 2009)       $
________________________________________________________________

*/






void CommsSendProc(void* arg)
{
	((CommsLayer*)arg)->SendProc();
}

void CommsLayer::SendProc()
{
	bool warned_MPI_TAG_UB = false;
	
	brahms::output::Source& tout = sender.thread->tout;

	//	cache some stuff locally
	INT32 localVoiceIndex = core->getVoiceIndex();

	//	start at -1, because 0 means we're done
	INT32 numberPeersAttached = -1;

	while(true)
	{
		//	get msg from queue
		//	return value *must* be C_OK, since we didn't permit a timeout or a cancel, so no need to check it
		IPM* messageForDispatch = NULL;
		/*Symbol result =*/ sender.q.pull(messageForDispatch);
		IPM_HEADER* messageForDispatch_header = &messageForDispatch->header();



	////////////////	FLUSH

		//	if IPMTAG_FLUSH, just set flushed and release the message without a send()
		if (messageForDispatch_header->tag == IPMTAG_FLUSH)
		{
			sender.flushed = true;
			messageForDispatch->release();
			continue;
		}



	////////////////	EXTRACT

		//	extract
		UINT32 to_zeroIndex = messageForDispatch->getTo();
		UINT32 to_unitIndex = unitIndex(to_zeroIndex);
		if (to_zeroIndex >= attachedChannels.size())
			ferr << E_INTERNAL << "to_zeroIndex >= attachedChannels.size() ... ( " << to_zeroIndex << " >= " << attachedChannels.size() << " )";

		UINT64 totalBytesToSend = messageForDispatch->uncompressedSize();
		ProtocolChannel* channel = attachedChannels[to_zeroIndex];

		if (channel == NULL) ferr << E_INTERNAL << "attachedChannel[" << to_zeroIndex << "] was NULL";

		//	handle HELLO
		if (messageForDispatch_header->tag == IPMTAG_HELLO)
		{
			if (numberPeersAttached == -1) numberPeersAttached = 0;
			numberPeersAttached++;
		}
		
		//	handle GOODBYE
		if (messageForDispatch_header->tag == IPMTAG_GOODBYE)
		{
			numberPeersAttached--;
		}
		


	////////////////	HANDLE ESTIMATE OF REMOTE BUFFER CONGESTION

		//	if it's IPMTAG_OUTPUTFOUND, we need to initialise a send auditor for that link
		if (messageForDispatch_header->tag == IPMTAG_OUTPUTFOUND)
		{
			brahms::os::MutexLocker locker(channel->auditsMutex);

			UINT32 msgStreamID = messageForDispatch_header->msgStreamID;
			if (channel->audits.size() <= msgStreamID) channel->audits.resize(msgStreamID + 1);
		}

		//	if it's IPMTAG_PUSHDATA, we need to audit it
		if (messageForDispatch_header->tag == IPMTAG_PUSHDATA)
		{
			IPM* ipms = NULL;

			{
				brahms::os::MutexLocker locker(channel->auditsMutex);
	
				UINT32 msgStreamID = messageForDispatch_header->msgStreamID;
			
				channel->audits[msgStreamID].queueAuditData.items++;
				channel->audits[msgStreamID].queueAuditData.bytes += totalBytesToSend;
			
				//	see how full the buffer is
				DOUBLE fullness_items = ((DOUBLE)channel->audits[msgStreamID].queueAuditData.items) / ((DOUBLE)channel->pars.PushDataMaxItems);
				DOUBLE fullness_bytes = ((DOUBLE)channel->audits[msgStreamID].queueAuditData.bytes) / ((DOUBLE)channel->pars.PushDataMaxBytes);
				DOUBLE fullness = max(fullness_items, fullness_bytes);

				//	if we're getting full, and we're not already waiting for a USEDDATA message to come back
				if (
					(fullness >= 0.5)
					&&
					(!channel->audits[msgStreamID].queryBufferMsgsUnaccountedFor)
					)
				{
					channel->audits[msgStreamID].queryBufferMsgsUnaccountedFor++; // increment
					ipms = layerSlushPool.get(IPMTAG_QUERYBUFFER, to_zeroIndex);
					ipms->header().from = localVoiceIndex;
					ipms->header().msgStreamID = messageForDispatch_header->msgStreamID;
				}
			}

			if (ipms) sender.q.push(ipms);
		}



	////////////////	HANDLE COMPRESSION
		
		//	we don't do compression, but must set the compressed bytes value
		messageForDispatch_header->bytesAfterHeaderCompressed = messageForDispatch_header->bytesAfterHeaderUncompressed;



	////////////////	REPORT SEND
		
		//	report
		if (messageForDispatch_header->tag <= IPMTAG_MAX_D_VERB)
			tout << "sending " << brahms::base::TranslateIPMTAG(messageForDispatch_header->tag) << " to Voice " << to_unitIndex << D_VERB;
		else
			tout << "sending " << brahms::base::TranslateIPMTAG(messageForDispatch_header->tag) << " to Voice " << to_unitIndex << D_FULL;



	////////////////	SEND INTO COMMS LAYER
			
		//	in MPI, we use the tag to pre-warn the receiver of the msgStreamID of a PUSHDATA message
		UINT32 tag = MPI_TAG_UB; // not a PUSHDATA
		if (messageForDispatch_header->tag == IPMTAG_PUSHDATA)
		{
			tag = messageForDispatch_header->msgStreamID;
			if (tag >= MPI_TAG_UB)
			{
				//	warn
				if (!warned_MPI_TAG_UB)
				{
					warned_MPI_TAG_UB = true;
					____WARN("MPI_TAG_UB is " << MPI_TAG_UB << ", which limits performance when msgStreamID == " << tag);
				}
				
				//	send UB, the only effect of which is that the buffer at the other end will come from a less efficient pool
				tag = MPI_TAG_UB;
			}
		}

		//	send it
		MPI::COMM_WORLD.Send((char*)messageForDispatch->stream(), totalBytesToSend, MPI::BYTE, messageForDispatch->getTo(), tag);



	////////////////	AUDIT

		//	audit
		sender.simplex.uncompressed += messageForDispatch->uncompressedSize();
		sender.simplex.compressed += messageForDispatch->compressedSize();


		//	release message
		messageForDispatch->release();

		//	escape
		if (!numberPeersAttached)
		{
			tout << "SendProc() break: numberPeersAttached == 0" << D_VERB;
			break;
		}

		//	escape
		if (!numberPeersAttached) break;
	}
}


