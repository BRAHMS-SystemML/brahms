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

#ifndef _CHANNEL_SOCKETS_H_
#define _CHANNEL_SOCKETS_H_

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

//////////////// THREAD PROCEDURE DECLARATIONS
/*
 These static thread procedures take one argument, the
 ProtocolChannel object, and simply call the member
 function thread procedure on that object.
*/

void SenderThreadProc(void* arg);
void ReceiverThreadProc(void* arg);

//////////////// COMMS PROTOCOL
const UINT32 CONNECTION_ATTEMPT_INTERVAL_MS = 25;

//////// PROTOCOL CHANNEL
struct ProtocolChannel
{
    // sockets
    // all send/recv goes on this one
    OS_SOCKET dataSocket;
    // this one is the local socket used by the server only
    OS_SOCKET serverListenSocket;

    // channel data
    bool server;
    UINT32 localPort;
    UINT32 remotePort;

    // engine data
    brahms::base::Core& core;
    ChannelInitData channelInitData;

    // cached parameters
    struct
    {
        UINT32 SocketsTimeout;
        UINT32 TimeoutThreadTerm;
        UINT32 PushDataMaxItems;
        UINT32 PushDataMaxBytes;
        UINT32 PushDataWaitStep;
        INT32 localVoiceIndex;
    } pars;

    // buffer pool
    IPMPool channelSlushPool;

    //////////////// CHANNEL
    ProtocolChannel(ChannelInitData channelInitData, brahms::base::Core& p_core);
    void flush(brahms::output::Source& tout);
    void listen();
    void open(brahms::output::Source& fout);
    void terminate(brahms::output::Source& fout);
    void stopRouting(brahms::output::Source& fout);
    void audit(ChannelAuditData& data);

    //////////////// SENDER
    void MemberSenderThreadProc();

#define SocketsKeepAliveInterval 1000

    struct Sender
    {
        Sender(INT32 remoteVoiceIndex, brahms::base::Core& core);
        void audit(ChannelAuditData& data);
        void terminate(brahms::output::Source& tout);

        // message queue
        IPM_FIFO q;
        
        // sender thread
        brahms::thread::Thread thread;

        // sender audit data
        ChannelSimplexData simplex;

        // other data
        UINT8 format;
        UINT32 IntervoiceCompression;
        bool flushed;

        // estimated contents of remote buffer for each link
        brahms::os::Mutex auditsMutex;
        vector<QueueAuditDataX> audits;
    } sender;
    
    UINT32 push(IPM* ipm, brahms::output::Source* tout);


    //////////////// RECEIVER
    void MemberReceiverThreadProc();

#define SOCKETS_PULL_WAITSTEP 100 // have to come back every now and then to check for waited > SocketsTimeout; this is usually measured in seconds, so anything sub-second will do here since it only gets hit in an error condition so it's not a performance concern

#define EXTRA_WAIT_FOR_RECEIVER 5000
    struct Receiver
    {
        Receiver(INT32 remoteVoiceIndex, brahms::base::Core& core);
        void audit(ChannelAuditData& data);
        void terminate(brahms::output::Source& fout);
        void stopRouting(brahms::output::Source& fout);
        bool active();

        // channel timeout watchdog timer
        brahms::os::Timer watchdog;

        // message queue
        IPM_FIFO q;

        // receiver thread
        brahms::thread::Thread thread;

        // receiver audit data
        ChannelSimplexData simplex;

        // deliverers array is sparse, and is its own routing table (that is,
        // the nth entry is the deliverer that is targeted by PUSHDATA messages
        // with a msgStreamID of n)
        vector<Deliverer*> deliverers;

        // flag that a message has come in
        bool messageReceived;
    } receiver;

    Symbol pull(IPM*& ipm, brahms::output::Source& tout);
    void addRoutingEntry(UINT32 msgStreamID, PushDataHandler pushDataHandler, void* pushDataHandlerArgument);
};

#endif // _CHANNEL_SOCKETS_H_
