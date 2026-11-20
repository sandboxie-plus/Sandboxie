#include "stdafx.h"
#include "HttpTransport.h"

#include <QAbstractSocket>
#include <QLocalSocket>

#ifdef Q_OS_WIN
// winsock2 before windows.h, or windows.h drags in the legacy winsock.h and
// every socket type collides. Same order as SandMan/Helpers/WinHelper.cpp.
#include <winsock2.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>
#include <windows.h>

// Declared here rather than in the project files: this is the only TU that
// needs them, and it keeps both build systems working without six per-config
// edits to the vcxproj. Same convention as SandMan/Helpers/StorageInfo.cpp.
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#endif

namespace HttpTransport
{

#ifdef Q_OS_WIN

//
// A loopback TCP connection is (client ip:port) <-> (server ip:port). The
// client process owns the socket whose *local* endpoint is what we see as the
// peer, so the table row to match is the one whose local endpoint equals the
// peer's and whose remote endpoint equals ours. Matching on only one of the two
// would pick the wrong row whenever a process holds several connections to us.
//
static quint32 GetTcpPeerPid(const QAbstractSocket* pSocket)
{
	const QHostAddress Peer = pSocket->peerAddress();
	const quint16 PeerPort = pSocket->peerPort();
	const quint16 OurPort = pSocket->localPort();
	if (PeerPort == 0 || OurPort == 0)
		return 0;

	const bool bV6 = (Peer.protocol() == QAbstractSocket::IPv6Protocol);
	const ULONG Family = bV6 ? AF_INET6 : AF_INET;

	quint32 Pid = 0;
	ULONG Size = 0;
	if (GetExtendedTcpTable(NULL, &Size, FALSE, Family, TCP_TABLE_OWNER_PID_ALL, 0) != ERROR_INSUFFICIENT_BUFFER)
		return 0;

	QByteArray Buffer(Size, 0);
	if (GetExtendedTcpTable(Buffer.data(), &Size, FALSE, Family, TCP_TABLE_OWNER_PID_ALL, 0) != NO_ERROR)
		return 0;

	if (!bV6)
	{
		const quint32 PeerV4 = Peer.toIPv4Address();
		MIB_TCPTABLE_OWNER_PID* pTable = (MIB_TCPTABLE_OWNER_PID*)Buffer.data();
		for (DWORD i = 0; i < pTable->dwNumEntries; i++)
		{
			const MIB_TCPROW_OWNER_PID& Row = pTable->table[i];
			if (ntohs((u_short)Row.dwLocalPort) != PeerPort || ntohs((u_short)Row.dwRemotePort) != OurPort)
				continue;
			if (ntohl(Row.dwLocalAddr) != PeerV4)
				continue;
			Pid = Row.dwOwningPid;
			break;
		}
	}
	else
	{
		Q_IPV6ADDR PeerV6 = Peer.toIPv6Address();
		MIB_TCP6TABLE_OWNER_PID* pTable = (MIB_TCP6TABLE_OWNER_PID*)Buffer.data();
		for (DWORD i = 0; i < pTable->dwNumEntries; i++)
		{
			const MIB_TCP6ROW_OWNER_PID& Row = pTable->table[i];
			if (ntohs((u_short)Row.dwLocalPort) != PeerPort || ntohs((u_short)Row.dwRemotePort) != OurPort)
				continue;
			if (memcmp(Row.ucLocalAddr, PeerV6.c, 16) != 0)
				continue;
			Pid = Row.dwOwningPid;
			break;
		}
	}
	return Pid;
}

#endif // Q_OS_WIN

quint32 GetPeerProcessId(const QIODevice* pDevice)
{
#ifdef Q_OS_WIN
	// QLocalSocket is a named pipe on Windows and its socketDescriptor() is the
	// pipe HANDLE, despite the Qt documentation describing it as a Winsock
	// handle. Verified empirically: GetNamedPipeClientProcessId on the
	// server-side socket returns the connecting process. Note this must be the
	// handle the *server* accepted on - the client's own handle answers about
	// itself.
	if (const QLocalSocket* pSocket = qobject_cast<const QLocalSocket*>(pDevice))
	{
		const qintptr Desc = pSocket->socketDescriptor();
		if (Desc == -1)
			return 0;
		ULONG Pid = 0;
		if (!GetNamedPipeClientProcessId((HANDLE)Desc, &Pid))
			return 0;
		return (quint32)Pid;
	}

	if (const QAbstractSocket* pSocket = qobject_cast<const QAbstractSocket*>(pDevice))
		return GetTcpPeerPid(pSocket);
#else
	Q_UNUSED(pDevice);
#endif
	return 0;
}

}
