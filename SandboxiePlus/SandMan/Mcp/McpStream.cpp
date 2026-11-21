#include "stdafx.h"
#include "McpStream.h"

CMcpStream::CMcpStream(QObject* parent)
 : QIODevice(parent)
{
	m_bEnded = false;
	// Opened for reading only: the socket reads from us, nobody writes to us.
	QIODevice::open(QIODevice::ReadOnly);
}

void CMcpStream::SendMessage(const QByteArray& Json)
{
	if (m_bEnded)
		return;

	// An SSE data field cannot contain a newline - a bare newline would end the
	// field and a blank line would end the event, so a pretty-printed message
	// would be silently cut in half. JSON-RPC messages are compact here, but
	// strip defensively rather than rely on the producer.
	QByteArray Line = Json;
	Line.replace('\r', "").replace('\n', "");

	m_Buffer += "data: ";
	m_Buffer += Line;
	m_Buffer += "\n\n";
}

void CMcpStream::SendKeepAlive()
{
	if (m_bEnded)
		return;
	// Any line beginning with a colon is a comment the client must ignore.
	m_Buffer += ":\n\n";
}

void CMcpStream::EndStream()
{
	m_bEnded = true;
}

qint64 CMcpStream::bytesAvailable() const
{
	return m_Buffer.size() + QIODevice::bytesAvailable();
}

qint64 CMcpStream::readData(char* pData, qint64 MaxLen)
{
	if (m_Buffer.isEmpty()) {
		// 0 is "nothing right now" and leaves the stream open; -1 is what makes
		// the socket write the terminating chunk. Getting these the wrong way
		// round either truncates every quiet stream or never closes any.
		return m_bEnded ? -1 : 0;
	}

	const qint64 Size = qMin(MaxLen, (qint64)m_Buffer.size());
	memcpy(pData, m_Buffer.constData(), Size);
	m_Buffer.remove(0, (int)Size);
	return Size;
}

qint64 CMcpStream::writeData(const char* pData, qint64 Len)
{
	Q_UNUSED(pData);
	Q_UNUSED(Len);
	return -1;
}
