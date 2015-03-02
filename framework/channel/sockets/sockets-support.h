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

	$Id:: sockets-support.cpp 2366 2009-11-13 00:46:58Z benjmi#$
	$Rev:: 2366                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-13 00:46:58 +0000 (Fri, 13 Nov 2009)       $
________________________________________________________________

*/

#ifndef _CHANNEL_SOCKETS_SUPPORT_H_
#define _CHANNEL_SOCKETS_SUPPORT_H_

// Ensure __NIX__ and __WIN__ etc are set up
#ifndef BRAHMS_BUILDING_ENGINE
#define BRAHMS_BUILDING_ENGINE
#endif
#include "brahms-client.h"

#include <string>
using std::string;
#include <sstream>
using std::stringstream;
#include "base/brahms_error.h" // ferr
#include "base/constants.h" // E_OS etc
#include "base/text.h" // h2s
using brahms::text::h2s;
#include "channel.h" // CommsInitData
using brahms::channel::CommsInitData;

// identical to implementation in base/text.cpp modulo some std:: tags.
#ifdef CANT_USE_ENGINE_IMPL_OF_THIS
string h2s(UINT64 val, UINT32 width = 0);
#endif

#ifdef __WIN__

# include "winsock2.h"

typedef SOCKET OS_SOCKET;
typedef SOCKADDR OS_SOCKADDR;

# define OS_INVALID_SOCKET INVALID_SOCKET
# define OS_SOCKET_ERROR SOCKET_ERROR
# define OS_LASTERROR WSAGetLastError()
# define OS_WOULDBLOCK WSAEWOULDBLOCK
# define OS_CONNREFUSED WSAECONNREFUSED
# define OS_INPROGRESS WSAEINPROGRESS
# define OS_ALREADY WSAEALREADY
# define OS_ISCONN WSAEISCONN

# define os_closesocket closesocket

#endif // __WIN__

#ifdef __NIX__

# include "sys/socket.h"
# include "sys/types.h"
# include "netinet/in.h"
# include "arpa/inet.h"
# include "string.h"
# include "unistd.h"
# include "fcntl.h"
# include "netinet/tcp.h"
# include "errno.h"

typedef int OS_SOCKET;
typedef const struct sockaddr OS_SOCKADDR;

# define OS_INVALID_SOCKET -1
# define OS_SOCKET_ERROR -1
# define OS_LASTERROR errno
# define OS_WOULDBLOCK EWOULDBLOCK
# define OS_CONNREFUSED ECONNREFUSED
# define OS_INPROGRESS EINPROGRESS
# define OS_ALREADY EALREADY
# define OS_ISCONN EISCONN

# define os_closesocket close

#endif // __NIX__

//	Human-readable socket errors
string socketsErrorString(unsigned int err);

// There are separate Windows/Unix implementations of these
void os_nonblock(OS_SOCKET socket);
void os_block(OS_SOCKET socket);
void os_enablenagle(OS_SOCKET socket, bool enable);

////////////////	GET LISTEN PORT NUMBER FROM SALIENT DATA
UINT32 listenPort(UINT32 SocketsBasePort, UINT32 voiceCount, UINT32 serverIndex, UINT32 clientIndex);

/*
  COMMS LAYER OBJECT

  One (global) comms layer object exists, which brings up and down the
  comms layer (sockets layer) at startup and shutdown.
*/
class CommsLayer
{
public:
    CommsLayer();
    ~CommsLayer();

    CommsInitData init(brahms::base::Core& p_core);
    //	is initialized?
    bool isinit();

    //	engine data (non-NULL indicates we've initialised)
    brahms::base::Core* core;
};

extern CommsLayer commsLayer;

#endif // _CHANNEL_SOCKETS_SUPPORT_H_
