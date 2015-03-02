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

$Id:: sockets-sender.cpp 2366 2009-11-13 00:46:58Z benjmit#$
$Rev:: 2366                                                $
$Author:: benjmitch                                        $
$Date:: 2009-11-13 00:46:58 +0000 (Fri, 13 Nov 2009)       $
________________________________________________________________

*/

#include "sockets.h"

////////////////	SENDER PROCEDURE

void SenderThreadProc(void* arg)
{
	((ProtocolChannel*)arg)->MemberSenderThreadProc();
}

void ProtocolChannel::MemberSenderThreadProc()
{
	brahms::output::Source& tout(sender.thread.tout);

	try
	{
		//	auto-release IPM
		SAFE_IPM messageForDispatch;

		//	buffer for sending compressed messages
		VUINT8 buffer;

		//	send() watchdog timer
		brahms::os::Timer watchdog;

		//	loop until GOODBYE is received
		while (true)
		{
			//	pull message from queue (see WAIT STATES)
			REPORT_THREAD_WAIT_STATE_IN("pull()");
			Symbol result = sender.q.pull(messageForDispatch.ipm());
			REPORT_THREAD_WAIT_STATE_OUT("pull()");

			//	if timeout
			if (result == E_SYNC_TIMEOUT)
			{
				//	poke a KEEPALIVE into the queue
				IPM* ipms = channelSlushPool.get(IPMTAG_KEEPALIVE, channelInitData.remoteVoiceIndex);
				ipms->header().from = core.getVoiceIndex();
				sender.q.push(ipms);

				//	and go round again to fetch a message
				continue;
			}



	////////////////	A MESSAGE IS READY TO BE SENT

			//	get pointer to header
			IPM_HEADER* messageForDispatch_header = &messageForDispatch.ipm()->header();

			//	extract size
			UINT32 totalBytesToSend = messageForDispatch.ipm()->uncompressedSize();



	////////////////	FLUSH

			//	if IPMTAG_FLUSH, just set flushed and release the message without a send()
			if (messageForDispatch_header->tag == IPMTAG_FLUSH)
			{
				sender.flushed = true;
				messageForDispatch.release();
				continue;
			}





	////////////////	AUDIT


			//	audit
			sender.simplex.uncompressed += totalBytesToSend;



	////////////////	HANDLE ESTIMATE OF REMOTE BUFFER CONGESTION

			//	if it's IPMTAG_OUTPUTFOUND, we need to initialise a send auditor for that link
			if (messageForDispatch_header->tag == IPMTAG_OUTPUTFOUND)
			{
				brahms::os::MutexLocker locker(sender.auditsMutex);

				UINT32 msgStreamID = messageForDispatch_header->msgStreamID;
				if (sender.audits.size() <= msgStreamID) sender.audits.resize(msgStreamID + 1);
			}

			//	if it's IPMTAG_PUSHDATA, we need to audit it
			if (messageForDispatch_header->tag == IPMTAG_PUSHDATA)
			{
				brahms::os::MutexLocker locker(sender.auditsMutex);

				UINT32 msgStreamID = messageForDispatch_header->msgStreamID;
				sender.audits[msgStreamID].queueAuditData.items++;
				sender.audits[msgStreamID].queueAuditData.bytes += totalBytesToSend;

				//	see how full the buffer is
				DOUBLE fullness_items = ((DOUBLE)sender.audits[msgStreamID].queueAuditData.items) / ((DOUBLE)pars.PushDataMaxItems);
				DOUBLE fullness_bytes = ((DOUBLE)sender.audits[msgStreamID].queueAuditData.bytes) / ((DOUBLE)pars.PushDataMaxBytes);
				DOUBLE fullness = max(fullness_items, fullness_bytes);

				//	if we're getting full, and we're not already waiting for a USEDDATA message to come back
				if (
					(fullness >= 0.5)
					&&
					(!sender.audits[msgStreamID].queryBufferMsgsUnaccountedFor)
					)
				{
					sender.audits[msgStreamID].queryBufferMsgsUnaccountedFor++; // increment

					//	send message
					IPM* ipms = channelSlushPool.get(IPMTAG_QUERYBUFFER, channelInitData.remoteVoiceIndex);
					ipms->header().from = core.getVoiceIndex();
					ipms->header().msgStreamID = messageForDispatch_header->msgStreamID;
					sender.q.push(ipms);
				}
			}



	////////////////	HANDLE COMPRESSION

			//	compress, if compression is on and message is PUSHDATA
			if (sender.format == IPMFMT_DEFLATE && messageForDispatch_header->tag == IPMTAG_PUSHDATA)
			{
				//	reserve space in buffer and point stream at it (this formula for maximum space required is defined by zlib)
				UINT32 compressedSizeInBytes = (UINT32) (((DOUBLE)totalBytesToSend) * 1.01 + 64.0);
				if (buffer.size() < compressedSizeInBytes) buffer.resize(compressedSizeInBytes);

				//	get new pointers
				IPM_HEADER* compressedHeader = (IPM_HEADER*)&buffer[0];

				//	copy over header
				*compressedHeader = *messageForDispatch_header;

				//	compress data
				const char* err = compressFunction(
						messageForDispatch_header + 1,
						messageForDispatch_header->bytesAfterHeaderUncompressed,
						compressedHeader + 1,
						&compressedSizeInBytes,
						sender.IntervoiceCompression
					);
				if (err) ferr << E_INTERNAL << err;
				compressedHeader->bytesAfterHeaderCompressed = compressedSizeInBytes;

				//	mark format into header (cannot rely on byte count being different, might be coincidentally identical)
				compressedHeader->fmt = IPMFMT_DEFLATE;

				//	retarget the send
				messageForDispatch_header = compressedHeader;
				totalBytesToSend = sizeof(IPM_HEADER) + messageForDispatch_header->bytesAfterHeaderCompressed;
			}

			//	else, just update header
			else
			{
				//	compressed space is same as uncompressed
				messageForDispatch_header->bytesAfterHeaderCompressed = messageForDispatch_header->bytesAfterHeaderUncompressed;
			}



	////////////////	AUDIT

			//	audit
			sender.simplex.compressed += totalBytesToSend;



	////////////////	REPORT SEND

			//	report
			if (messageForDispatch_header->tag <= IPMTAG_MAX_D_VERB)
				tout << "sending " << brahms::base::TranslateIPMTAG(messageForDispatch_header->tag) << D_VERB;
			else
				tout << "sending " << brahms::base::TranslateIPMTAG(messageForDispatch_header->tag) << D_FULL;

/*
			if (messageForDispatch_header->tag == IPMTAG_ANNOUNCEOUTPUT)
			{
				string outputName = (const char*)messageForDispatch.ipm()->body();
				tout << "IPMTAG_ANNOUNCEOUTPUT: " << &messageForDispatch.ipm() << " " << outputName << D_INFO;
			}
			*/

	////////////////	SEND INTO COMMS LAYER

/*	Differences in partial writes to TCP sockets (contributed by James Knight) retrieved from: http://itamarst.org/writings/win32sockets.html
	In Unix, socket.send(buf) will buffer as much of buf as it has space for, and then return how much it accepted. This could be 0 or up to something around 128K. If you send some data and then some more, it will append to the previous buffer. In Windows, socket.send(buf) will either accept the entire buffer or raise ENOBUFS. Testing indicates that it will internally buffer any amount up to 50MB (this seems to be the total for either the process or the OS, I'm not sure). However, it will not incrementally accept more data to append to a socket's buffer until the big buffer has been completely emptied (seemingly down to the SO_SNDBUF length, which is 8192), but rather raises WSAEWOULDBLOCK instead. */

			//	send message into sockets layer
			BYTE* nextByteToSend = (BYTE*) messageForDispatch_header;
			UINT32 remainingBytesToSend = totalBytesToSend;

			//	send watchdog timer
			watchdog.reset();

			//	send message
			UINT32 quickDirtyCount = 0;
			while(remainingBytesToSend)
			{
				//	call send()
				REPORT_THREAD_WAIT_STATE_IN("send()");
				int bytesSent = send(dataSocket, (const char*)nextByteToSend, remainingBytesToSend, 0);
				REPORT_THREAD_WAIT_STATE_OUT("send()");

				//	check for error
				if (bytesSent == OS_SOCKET_ERROR)
				{
					//	if it returned WOULDBLOCK, we just need to wait a moment and try again
					if (OS_LASTERROR == OS_WOULDBLOCK)
					{
						//	only if there wasn't anything pending, must be that the
						//	buffer is full for some other reason - perhaps just backed
						//	up, so we do a yield to allow time for the send() buffer
						//	to clear...
						os_msleep(0);
						quickDirtyCount++;
						if (quickDirtyCount == 100)
						{
							//	in which case we also check for timeout
							if (watchdog.elapsedMS() >= pars.SocketsTimeout)
								ferr << E_COMMS_TIMEOUT << "in push()";

							//	restart
							quickDirtyCount = 0;
						}

						//	before trying to call send() again...
						continue;
					}

					//	any other error is just reported directly
					ferr << E_COMMS << "failed at send() (" << socketsErrorString(OS_LASTERROR) << ")";
				}

				//	otherwise, pointer advance
				nextByteToSend += bytesSent;
				remainingBytesToSend -= bytesSent;
			}

			//	check for goodbye
			bool goodbye = messageForDispatch_header->tag == IPMTAG_GOODBYE;

			//	release message (fire callback or return to pool)
			messageForDispatch.release();

			//	break on GOODBYE
			if (goodbye) break;
		}
	}

	//	trace exception
	catch(brahms::error::Error& e)
	{
		//	store error
		sender.thread.storeError(e, tout);
	}

	//	trace exception
	catch(exception se)
	{
		//	store error
		brahms::error::Error e(E_STD, se.what());
		sender.thread.storeError(e, tout);
	}

	//	trace exception
	catch(...)
	{
		//	store error
		brahms::error::Error e(E_UNRECOGNISED_EXCEPTION);
		sender.thread.storeError(e, tout);
	}
}
