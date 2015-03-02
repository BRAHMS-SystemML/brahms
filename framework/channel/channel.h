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

	$Id:: channel.h 2363 2009-11-12 23:58:51Z benjmitch        $
	$Rev:: 2363                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-12 23:58:51 +0000 (Thu, 12 Nov 2009)       $
________________________________________________________________

*/

////////////////	INTER-PROCESS COMMUNICATIONS CHANNEL (CONCERTO ONLY)

#ifndef INCLUDED_BRAHMS_CHANNEL
#define INCLUDED_BRAHMS_CHANNEL

// Ensure __NIX__ and __WIN__ etc are set up
#ifndef BRAHMS_BUILDING_ENGINE
#define BRAHMS_BUILDING_ENGINE
#endif
#include "brahms-client.h"

#include "compress.h"
#include "base/ipm.h"
using brahms::base::QueueAuditData;
using brahms::base::IPM;
#include "base/core.h"
#include <string>
using std::string;
#include "channel-common.h"
#ifdef __NIX__
#include <unistd.h>
#endif

#ifdef BRAHMS_BUILDING_CHANNEL
#define BRAHMS_CHANNEL_VIS BRAHMS_DLL_EXPORT
#else
#define BRAHMS_CHANNEL_VIS BRAHMS_DLL_IMPORT
#endif

//#define REPORT_WAIT_STATES
#ifdef REPORT_WAIT_STATES
#define REPORT_THREAD_WAIT_STATE_IN(w) tout << "WAIT IN (" << w << ")" << D_INFO;
#define REPORT_THREAD_WAIT_STATE_OUT(w) tout << "WAIT OUT (" << w << ")" << D_INFO;
#else
#define REPORT_THREAD_WAIT_STATE_IN(w) ;
#define REPORT_THREAD_WAIT_STATE_OUT(w) ;
#endif

struct QueueAuditDataX
{
	QueueAuditDataX()
	{
		queueAuditData.clear();
		queryBufferMsgsUnaccountedFor = 0;
	}

	QueueAuditData queueAuditData;

	// increment when send QUERYBUFFER, decrement when receive USEDDATA
	UINT32 queryBufferMsgsUnaccountedFor;
};

/*
 * A global sleep function. Compiles using differing system code on
 * differing platforms. Wraps usleep() on Unix, Sleep() on Windows.
 */
void os_msleep(UINT32 ms);

// compress function, if available. typedeffed in
// compress/compress.h. Initialise to 0 in object code.
extern CompressFunction* compressFunction;

//	safe IPM holder
struct SAFE_IPM
{
    SAFE_IPM();
    ~SAFE_IPM();

    IPM*& ipm();
    void release();
    IPM* retrieve();

    IPM* m_ipm;
};

void ipm_dump(IPM* ipm, brahms::output::Source& tout, const char* name);

namespace brahms
{
	namespace channel
	{
		class Channel
		{
		public:
			virtual ~Channel();

			virtual Symbol open(brahms::output::Source& fout) = 0;
			virtual Symbol close(brahms::output::Source& fout) = 0;
			virtual Symbol terminate(brahms::output::Source& fout) = 0;

			//	during init and term, messages are sent and received in a pre-defined
			//	order, so we can push messages in *and* pull them out of the channel,
			//	knowing they will be there to be received.
			virtual UINT32 push(brahms::base::IPM* ipm, brahms::output::Source* tout, bool ignoreIfNotOpen = false) = 0;
			virtual Symbol pull(brahms::base::IPM*& ipm, UINT8 tag, brahms::output::Source& tout, UINT32 timeout) = 0;

			//	flush all messages in the send queue (this ensures that no PUSHDATA are
			//	there, which in turn ensures that no further callbacks will be generated
			//	from this channel object)
			virtual void flush(brahms::output::Source& tout) = 0;

			//	during run, however, incoming PUSHDATA messages come unexpectedly (we
			//	*could* work out when they are due, but we don't, because it would mean
			//	essentially computing the timing logic of all the other voices on the
			//	local voice, which would be a big waste of effort and a massive headfuck
			//	to boot). therefore, we instruct the channel, instead, to route these
			//	messages to a receiver function that we define. the "msgStreamID" value is
			//	stored in the header of the PUSHDATA message, and used at the receiving
			//	end to find the correct PushDataHandler to pass it to.
			virtual void addRoutingEntry(
				UINT32 msgStreamID,
				PushDataHandler pushDataHandler,
				void* pushDataHandlerArgument
				) = 0;

			//	when we're done with the deliverers and we want to make sure no more
			//	PUSHDATA are routed, we call this function to bring all deliverer threads
			//	to a halt
			virtual void stopRouting(brahms::output::Source& tout) = 0;

			//	audit the state of the channel, to give feedback to the user
			//	return true if the data has been filled in for all channels
			virtual bool audit(ChannelAuditData& data) = 0;
		};

		//	export prototypes
		typedef CommsInitData (CommsInitFunc) (brahms::base::Core&, CompressFunction* compressFunction);
		typedef Channel* (CreateChannelFunc) (brahms::base::Core&, brahms::channel::ChannelInitData);

		//	export declarations
		BRAHMS_CHANNEL_VIS CommsInitData commsInit(brahms::base::Core& core, CompressFunction* p_compressFunction);
		BRAHMS_CHANNEL_VIS Channel* createChannel(brahms::base::Core& core, ChannelInitData channelInitData);
	}
}

#endif // INCLUDED_BRAHMS_CHANNEL
