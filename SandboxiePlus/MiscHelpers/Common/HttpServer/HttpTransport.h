#pragma once

#include "../../mischelpers_global.h"
#include <QIODevice>
#include <QAbstractSocket>
#include <QLocalSocket>
#include <QHostAddress>

//
// CHttpServer serves both a QTcpServer and a QLocalServer, so CHttpSocket holds
// its connection as a plain QIODevice. QTcpSocket and QLocalSocket share nothing
// above QIODevice - QLocalSocket is not a QAbstractSocket - which covers read,
// write, bytesAvailable and bytesToWrite, i.e. almost everything the HTTP code
// does. The handful of calls that need more go through the helpers below, which
// qobject_cast to pick the concrete type.
//
// A failed cast here always means "this is the other transport", never an error:
// every helper has a defined answer for both, so callers never branch on type.
//

namespace HttpTransport
{
	inline bool IsLocal(const QIODevice* pDevice)
	{
		return qobject_cast<const QLocalSocket*>(pDevice) != NULL;
	}

	// Qt treats 0 as "unlimited", which would make the chunking loops in
	// TrySendBuffer spin on a zero bound - callers want a usable chunk size, so
	// hand back a default instead of 0.
	inline qint64 GetReadBufferSize(const QIODevice* pDevice, qint64 Default = 16 * 1024)
	{
		qint64 Size = 0;
		if (const QAbstractSocket* pSocket = qobject_cast<const QAbstractSocket*>(pDevice))
			Size = pSocket->readBufferSize();
		else if (const QLocalSocket* pSocket = qobject_cast<const QLocalSocket*>(pDevice))
			Size = pSocket->readBufferSize();
		return Size > 0 ? Size : Default;
	}

	inline void SetReadBufferSize(QIODevice* pDevice, qint64 Size)
	{
		if (QAbstractSocket* pSocket = qobject_cast<QAbstractSocket*>(pDevice))
			pSocket->setReadBufferSize(Size);
		else if (QLocalSocket* pSocket = qobject_cast<QLocalSocket*>(pDevice))
			pSocket->setReadBufferSize(Size);
	}

	// QLocalSocket spells this disconnectFromServer, hence the helper.
	inline void Disconnect(QIODevice* pDevice)
	{
		if (QAbstractSocket* pSocket = qobject_cast<QAbstractSocket*>(pDevice))
			pSocket->disconnectFromHost();
		else if (QLocalSocket* pSocket = qobject_cast<QLocalSocket*>(pDevice))
			pSocket->disconnectFromServer();
	}

	// Null for a local socket: a pipe has no peer address.
	inline QHostAddress GetPeerAddress(const QIODevice* pDevice)
	{
		if (const QAbstractSocket* pSocket = qobject_cast<const QAbstractSocket*>(pDevice))
			return pSocket->peerAddress();
		return QHostAddress();
	}

	// 0 for a local socket. CHttpServer::GetHandler already reads a registered
	// port of 0 as "whatever the server's own port is", so a handler registered
	// without a port keeps matching on the local transport.
	inline quint16 GetLocalPort(const QIODevice* pDevice)
	{
		if (const QAbstractSocket* pSocket = qobject_cast<const QAbstractSocket*>(pDevice))
			return pSocket->localPort();
		return 0;
	}

	// The process id behind the connection, or 0 when it cannot be established.
	//
	// 0 means "unknown", never "benign": a caller using this to gate access must
	// refuse on 0 rather than fall through. Out of line because the pipe and TCP
	// paths need windows.h and iphlpapi, which do not belong in every TU that
	// includes this header.
	quint32 MISCHELPERS_EXPORT GetPeerProcessId(const QIODevice* pDevice);

	// For log lines only - never parse this.
	inline QString GetPeerName(const QIODevice* pDevice)
	{
		if (const QAbstractSocket* pSocket = qobject_cast<const QAbstractSocket*>(pDevice))
			return pSocket->peerAddress().toString() + ":" + QString::number(pSocket->peerPort());
		if (const QLocalSocket* pSocket = qobject_cast<const QLocalSocket*>(pDevice))
			return "local:" + pSocket->serverName();
		return "unknown";
	}
}
