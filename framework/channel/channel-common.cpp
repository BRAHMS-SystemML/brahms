#include "channel-common.h"

// Common channel code, whether mpich2 or sockets.
brahms::channel::ChannelSimplexData::ChannelSimplexData()
{
    uncompressed = 0;
    compressed = 0;
    queue = 0;
}
