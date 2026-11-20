#pragma once
#include <QIODevice>
#include <QByteArray>

//
// CMcpStream - one Server-Sent Events response stream.
//
// CHttpSocket streams by reading from a response buffer device, and its contract
// is specific (see TrySendBuffer): a **sequential** device gets chunked
// transfer encoding, every read becomes one chunk, a read of 0 means "nothing
// right now, keep the stream open", and a read of -1 is what ends it and emits
// the terminating chunk. This class is that device.
//
// Nothing drives it on its own: after queueing an event the owner has to pump
// the server so the socket picks it up. CMcpServer runs a timer for that.
//
class CMcpStream : public QIODevice
{
	Q_OBJECT

public:
	CMcpStream(QObject* parent = NULL);

	// Always sequential. This is what selects chunked encoding, and it is also
	// why the socket never asks whether we are at the end - only a read of -1
	// closes the stream.
	virtual bool	isSequential() const	{ return true; }

	// One JSON-RPC message as one SSE event.
	void			SendMessage(const QByteArray& Json);

	// An SSE comment, which carries no data and exists to keep intermediaries
	// and client idle timers from closing a quiet stream.
	void			SendKeepAlive();

	// No further events. The socket emits the terminating chunk once it has
	// drained whatever is still queued.
	void			EndStream();
	bool			IsEnded() const			{ return m_bEnded; }

	virtual qint64	bytesAvailable() const;

signals:
	// The client went away. Closing the stream is the cancellation signal on
	// this transport, so an owner with work in flight should stop it.
	void			Cancelled();

protected:
	virtual qint64	readData(char* pData, qint64 MaxLen);
	// Read-only: a response stream is not somewhere the client writes.
	virtual qint64	writeData(const char* pData, qint64 Len);

private:
	QByteArray		m_Buffer;
	bool			m_bEnded;
};
