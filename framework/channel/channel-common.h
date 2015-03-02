#ifndef BRAHMS_BUILDING_ENGINE
#define BRAHMS_BUILDING_ENGINE
#endif
#include "brahms-client.h" // Ensure types are set up (INT32 etc)
#include "base/ipm.h" // ChannelPoolData
#include <string>
using std::string;

namespace brahms
{
    namespace channel
    {
        enum Protocol
        {
            PROTOCOL_NULL = 0,
            // note that the protocols are listed in preference order - earlier
            // ones will be selected, if available, during reading of the Execution
            // File
            //
            // note, also, that they are |'d during the parsing process, so they
            // must be bit-exclusive (1, 2, 4, etc.)
            PROTOCOL_MPI = 1,
            PROTOCOL_SOCKETS = 2
        };

        // INITIALISATION DATA

        // comms init data
        struct CommsInitData
        {
            INT32 voiceIndex;
            INT32 voiceCount;
        };

        // channel init data
        struct ChannelInitData
        {
            // contact data
            UINT32 remoteVoiceIndex;
            string remoteAddress;
        };

        // AUDIT DATA

        // channel simplex data
        struct ChannelSimplexData
        {
            ChannelSimplexData();
            UINT64 uncompressed;
            UINT64 compressed;
            UINT32 queue;
        };

        // channel audit data
        struct ChannelAuditData
        {
            // send and recv
            ChannelSimplexData send;
            ChannelSimplexData recv;

            // pool
            brahms::base::ChannelPoolData pool;
        };

        // use default timeout rather than anything specific
        const UINT32 COMMS_TIMEOUT_DEFAULT = 0x80000001;

        // push data handler function prototype
        typedef Symbol (*PushDataHandler)(void* arg, BYTE* stream, UINT32 count);
    }
}
