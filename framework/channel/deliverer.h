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

	$Id:: deliverer.cpp 2329 2009-11-09 00:13:25Z benjmitch    $
	$Rev:: 2329                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-09 00:13:25 +0000 (Mon, 09 Nov 2009)       $
________________________________________________________________

*/

#ifndef _CHANNEL_DELIVERER_H_
#define _CHANNEL_DELIVERER_H_

#include "brahms-client.h"
#include <string>
using std::string;
#include <sstream>
using std::ostringstream;
#include <stdexcept>
using std::exception;
#include "base/thread.h"
#include "base/brahms_error.h"
#include "channel.h"
using namespace brahms::channel;
#include "base/ipm.h"
using namespace brahms::base;
#include "fifo.h"
#include "base/output.h"
using namespace brahms::output;

////////////////	DELIVERER CLASS

//const UINT32 DELIVERY_PERIOD_SMOOTHING_L = 64;

void DelivererProc(void* arg);

string uint32_n2s(UINT32 n);

struct Deliverer : public brahms::thread::Thread
{
    Deliverer(
        PushDataHandler p_pushDataHandler,
        void* p_pushDataHandlerArgument,
        UINT32 delivererIndex,
        ChannelInitData channelInitData,
        brahms::base::Core& p_core
	);
    void start(UINT32 TimeoutThreadTerm);
    void push(IPM* msg);
    QueueAuditData queueAudit();
    void audit(ChannelAuditData& data);
    void terminate(brahms::output::Source& fout);
    void ThreadProc();
    IPM* getIPMFromPool();

private:
    brahms::base::Core& core;

    IPMPool delivererIPMPool;

    PushDataHandler pushDataHandler;
    void* pushDataHandlerArgument;

    IPM_FIFO q;

    UINT32 order;
    bool stop;
};

#endif // _CHANNEL_DELIVERER_H_
