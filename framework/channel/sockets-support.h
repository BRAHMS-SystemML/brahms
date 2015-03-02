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
#include "channel.h" // CommsInitData
using brahms::channel::CommsInitData;

string h2s(UINT64 val, UINT32 width = 0)
{
	stringstream ss;
	ss << std::hex << std::uppercase << val;
	if (!width) return ss.str();
	string ret = ss.str();
	while (ret.length() < width) ret = "0" + ret;
	return ret;
}


#ifdef __WIN__

	#include "winsock2.h"

	typedef SOCKET OS_SOCKET;
	typedef SOCKADDR OS_SOCKADDR;

	#define OS_INVALID_SOCKET INVALID_SOCKET
	#define OS_SOCKET_ERROR SOCKET_ERROR
	#define OS_LASTERROR WSAGetLastError()
	#define OS_WOULDBLOCK WSAEWOULDBLOCK
	#define OS_CONNREFUSED WSAECONNREFUSED
	#define OS_INPROGRESS WSAEINPROGRESS
	#define OS_ALREADY WSAEALREADY
	#define OS_ISCONN WSAEISCONN

	#define os_closesocket closesocket

	//	Human-readable socket errors
	string socketsErrorString(unsigned int err)
	{
		switch(err)
		{
			case WSANOTINITIALISED: return "WSANOTINITIALISED, not initialised yet";
			case WSAENETDOWN: return "WSAENETDOWN, network subsystem failed";
			case WSAEADDRINUSE: return "WSAEADDRINUSE, socket local address in use";
			case WSAEINTR: return "WSAEINTR, blocking call cancelled by client";
			case WSAEINPROGRESS: return "WSAEINPROGRESS, blocking call in progress";
			case WSAEALREADY: return "WSAEALREADY, non-blocking call already in progress";
			case WSAEADDRNOTAVAIL: return "WSAEADDRNOTAVAIL, remote address not valid";
			case WSAEAFNOSUPPORT: return "WSAEAFNOSUPPORT, address cannot be used with this socket";
			case WSAECONNREFUSED: return "WSAECONNREFUSED, connection refused";
			case WSAEFAULT: return "WSAEFAULT, argument wrong format";
			case WSAEINVAL: return "WSAEINVAL, parameter is a listening socket";
			case WSAEISCONN: return "WSAEISCONN, socket is already connected";
			case WSAENETUNREACH: return "WSAENETUNREACH, remote host unreachable";
			case WSAEHOSTUNREACH: return "WSAEHOSTUNREACH, operation to unreachable remote host";
			case WSAENOBUFS: return "WSAENOBUFS, no buffer space available";
			case WSAENOTSOCK: return "WSAENOTSOCK, operation on non-socket";
			case WSAETIMEDOUT: return "WSAETIMEDOUT, connection timed out";
			case WSAEWOULDBLOCK: return "WSAEWOULDBLOCK, operation would block";
			case WSAEACCES: return "WSAEACCES, requires SO_BROADCAST";

			case WSAECONNABORTED: return "WSAECONNABORTED, connection aborted";
			case WSAECONNRESET: return "WSAECONNRESET, connection reset by peer";
			default: return h2s(err);
		}
	}

	void os_nonblock(OS_SOCKET socket)
	{
		unsigned long noBlock = 1;
		if ( ioctlsocket(socket, FIONBIO, &noBlock) == OS_SOCKET_ERROR )
			ferr << E_OS << "failed ioctl (" + socketsErrorString(OS_LASTERROR) + ")";
	}

	void os_block(OS_SOCKET socket)
	{
		unsigned long noBlock = 0;
		if ( ioctlsocket(socket, FIONBIO, &noBlock) == OS_SOCKET_ERROR )
			ferr << E_OS << "failed ioctl (" + socketsErrorString(OS_LASTERROR) + ")";
	}

	void os_enablenagle(OS_SOCKET socket, bool enable)
	{
		BOOL disable = !enable;
		if (setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (const char*) &disable, sizeof(BOOL)))
			ferr << E_OS << "failed setsockopt()";
	}

#endif

#ifdef __NIX__

	#include "sys/socket.h"
	#include "sys/types.h"
	#include "netinet/in.h"
	#include "arpa/inet.h"
	#include "string.h"
	#include "unistd.h"
	#include "fcntl.h"
	#include "netinet/tcp.h"
	#include "errno.h"

	typedef int OS_SOCKET;
	typedef const struct sockaddr OS_SOCKADDR;

	#define OS_INVALID_SOCKET -1
	#define OS_SOCKET_ERROR -1
	#define OS_LASTERROR errno
	#define OS_WOULDBLOCK EWOULDBLOCK
	#define OS_CONNREFUSED ECONNREFUSED
	#define OS_INPROGRESS EINPROGRESS
	#define OS_ALREADY EALREADY
	#define OS_ISCONN EISCONN

	#define os_closesocket close

	//	Human-readable socket errors
	string socketsErrorString(unsigned int err)
	{
		switch(err)
		{
			case EALREADY: return "EALREADY: " + string(strerror(err));
			case EINPROGRESS: return "EINPROGRESS: " + string(strerror(err));
			case ETIMEDOUT: return "ETIMEDOUT: " + string(strerror(err));
		}
		return strerror(err);
	}

	void os_nonblock(OS_SOCKET socket)
	{
		int flags = fcntl(socket, F_GETFL, 0);
		if (
			flags == OS_SOCKET_ERROR ||
			fcntl(socket, F_SETFL, flags | O_NONBLOCK) == OS_SOCKET_ERROR
			)
		ferr << E_OS << "failed fcntl (" + socketsErrorString(OS_LASTERROR) + ")";
	}

	void os_block(OS_SOCKET socket)
	{
		int flags = fcntl(socket, F_GETFL, 0);
		if (
			flags == OS_SOCKET_ERROR ||
			fcntl(socket, F_SETFL, flags & (~O_NONBLOCK)) == OS_SOCKET_ERROR
			)
		ferr << E_OS << "failed fcntl (" + socketsErrorString(OS_LASTERROR) + ")";
	}

	void os_enablenagle(OS_SOCKET socket, bool enable)
	{
		//fout << "nagle enable " << enable << D_VERB;
		int disable = enable ? 0 : 1;
		if (setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (const char*) &disable, sizeof(int)))
			ferr << E_OS << "failed setsockopt";
	}

#endif





////////////////	GET LISTEN PORT NUMBER FROM SALIENT DATA

UINT32 listenPort(UINT32 SocketsBasePort, UINT32 voiceCount, UINT32 serverIndex, UINT32 clientIndex)
{
	/*	Voice 1 listens on N-1 ports, starting with voice 2
		Voice 2 listens on N-2 ports, starting with voice 3
		...
		Every Voice listens to the voices above it (and connects to the voices below it) */

	UINT32 port = 0;
	for (UINT32 s=0; s<serverIndex; s++)
		port += voiceCount - s - 1;
	return SocketsBasePort + port + clientIndex - serverIndex - 1;
}




////////////////	COMMS LAYER OBJECT

/*

	One (global) comms layer object exists, which brings up and down
	the comms layer (sockets layer) at startup and shutdown.

*/

class CommsLayer
{

public:

	CommsLayer()
	{
		core = NULL;
	}

	~CommsLayer()
	{
		if (core)
		{
#ifdef __WIN__

			//	terminate winsock
			WSACleanup();

#endif

		}
	}

	CommsInitData init(brahms::base::Core& p_core)
	{
		if (!core)
		{

#ifdef __WIN__

			WSADATA wsaData;
			int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
			if ( iResult != NO_ERROR )
				ferr << E_OS << "failed to init winsock (" + socketsErrorString(WSAGetLastError()) + ")";

#endif

			//	store
			core = &p_core;
		}

		//	ok
		CommsInitData ret;
		ret.voiceIndex = VOICE_UNDEFINED;
		ret.voiceCount = 0;
		return ret;
	}

	//	is initialized?
	bool isinit()
	{
		return core != NULL;
	}

	//	engine data (non-NULL indicates we've initialised)
	brahms::base::Core* core;
}
commsLayer;


