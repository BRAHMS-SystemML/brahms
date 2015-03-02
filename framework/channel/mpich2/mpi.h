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

#ifndef _CHANNEL_MPICH2_MPI_H_
#define _CHANNEL_MPICH2_MPI_H_

// Ensure __NIX__ and __WIN__ etc are set up
#ifndef BRAHMS_BUILDING_ENGINE
#define BRAHMS_BUILDING_ENGINE
#endif
#include "brahms-client.h"

#include "deliverer.h"
//#include "base/brahms_math.h"
//using brahms::math::unitIndex;
//#include "compress.h"
#include <mpi/mpi.h>

using namespace brahms::channel;

////////////////	CHANNEL CLASS
struct ProtocolChannel
{
    ProtocolChannel(ChannelInitData p_channelInitData, brahms::base::Core& p_core);
    ~ProtocolChannel();

    void listen();
    void open(brahms::output::Source& fout);
    void terminate(brahms::output::Source& tout);
    void flush(brahms::output::Source& fout);
    void stopRouting(brahms::output::Source& tout);
    Symbol pull(IPM*& ipm, brahms::output::Source& tout);
    UINT32 push(IPM* msg, brahms::output::Source* tout);
    void audit(ChannelAuditData& data);

    //	add routing entry for PUSHDATA msgs
    void addRoutingEntry(UINT32 msgStreamID, PushDataHandler pushDataHandler, void* pushDataHandlerArgument);

    Deliverer* getDeliverer(UINT32 index);

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
    } pars;

private:

    //	engine data
    brahms::base::Core& core;

    //	addressing
    ChannelInitData channelInitData;

    //	audit
    ChannelAuditData cad;

    //	deliverers
    vector<Deliverer*> deliverers;

    //	first channel to attach does special behaviour during audit
    bool firstToAttach;
};

////////////////	COMMS LAYER CLASS
void CommsSendProc(void* arg);
void CommsRecvProc(void* arg);

class CommsLayer
{
public:
    CommsLayer();
    ~CommsLayer();

    CommsInitData init(brahms::base::Core& p_core);
    void openChannel(UINT32 remoteIndex);
    void closeChannel(UINT32 remoteIndex, brahms::output::Source& tout);
    void push(IPM* msg);
    void audit(ChannelAuditData& data);

////////////////	SENDER
public:
    struct Sender
    {
        Sender();
        void init(brahms::base::Core& core, CommsLayer* commsLayer);
        void terminate(brahms::output::Source& tout);
        //	message queue
        IPM_FIFO q;
        //	sender thread
        brahms::thread::Thread* thread;
        //	other data
        // UINT8 format;
        // UINT32 IntervoiceCompression;
        bool flushed;
        // audit
        ChannelSimplexData simplex;
        // engine data (set in init())
        brahms::base::Core* core;
    } sender;

    //	thread procedure
    void SendProc();

    void flush(brahms::output::Source& fout);

////////////////	RECEIVER
public:
    struct Receiver
    {
        Receiver();
        void init(brahms::base::Core& core, CommsLayer* commsLayer);
        void terminate(brahms::output::Source& tout);
        //	receiver thread
        brahms::thread::Thread* thread;
        //	audit
        ChannelSimplexData simplex;
        //	engine data (set in init())
        brahms::base::Core* core;
    } receiver;

    //	thread procedure
    void RecvProc();

    //	pool common to all channels
    IPMPool layerSlushPool;

    //	reference count of openChannel()/closeChannel()
    UINT32 numberChannelsOpen;

public:
    //	attach channel
    bool attachChannel(ProtocolChannel* channel, UINT32 remoteVoiceIndex);
    //	is initialized?
    bool isinit();
    //	attached channels
    vector<ProtocolChannel*> attachedChannels;

private:
    //	engine data (set in init())
    brahms::base::Core* core;
};

extern CommsLayer commsLayer;

#endif // _CHANNEL_MPICH2_MPI_H_
