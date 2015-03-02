/*
 * sockets-support implementation
 */

#include "sockets-support.h"

#include <string>
using std::string;
#include <sstream>
using std::stringstream;

/*
 * Global commsLayer object.
 */
CommsLayer commsLayer;

// implemented also in engine (see base/text.h)
#ifdef CANT_USE_ENGINE_IMPL_OF_THIS
string h2s(UINT64 val, UINT32 width = 0);
{
    stringstream ss;
    ss << std::hex << std::uppercase << val;
    if (!width) return ss.str();
    string ret = ss.str();
    while (ret.length() < width) ret = "0" + ret;
    return ret;
}
#endif

#ifdef __WIN__

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

#endif // __WIN__

#ifdef __NIX__

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

#endif // __NIX__



// GET LISTEN PORT NUMBER FROM SALIENT DATA
UINT32 listenPort(UINT32 SocketsBasePort, UINT32 voiceCount, UINT32 serverIndex, UINT32 clientIndex)
{
// Voice 1 listens on N-1 ports, starting with voice 2
// Voice 2 listens on N-2 ports, starting with voice 3
// ...
// Every Voice listens to the voices above it (and connects to the voices below it) */

    UINT32 port = 0;
    for (UINT32 s=0; s<serverIndex; s++) {
        port += voiceCount - s - 1;
    }
    return SocketsBasePort + port + clientIndex - serverIndex - 1;
}

// CommsLayer implementation
//@{
CommsLayer::CommsLayer()
{
    core = NULL;
}

CommsLayer::~CommsLayer()
{
    if (core) {
#ifdef __WIN__
        // terminate winsock
        WSACleanup();
#endif
    }
}

CommsInitData
CommsLayer::init(brahms::base::Core& p_core)
{
    if (!core) {
#ifdef __WIN__
        WSADATA wsaData;
        int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
        if ( iResult != NO_ERROR ) {
            ferr << E_OS << "failed to init winsock (" + socketsErrorString(WSAGetLastError()) + ")";
        }
#endif

        // store
        core = &p_core;
    }

    // ok
    CommsInitData ret;
    ret.voiceIndex = VOICE_UNDEFINED;
    ret.voiceCount = 0;
    return ret;
}

// is initialized?
bool
CommsLayer::isinit()
{
    return core != NULL;
}
//@}
