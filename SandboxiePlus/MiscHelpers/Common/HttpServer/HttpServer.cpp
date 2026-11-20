#include "stdafx.h"
#include "HttpServer.h"
#include "HttpSocket.h"
#include "../DebugHelpers.h"
#include <QDebug>
#include <QTcpSocket>
#include <QLocalSocket>

CHttpServer::CHttpServer(const QString& Name, QObject* qObject)
 : QObject(qObject)
{
	m_Port = 0;
	m_ServerName = "NeoServer";
	m_TransferBufferSize = 4 * 1024;
	m_KeepAlive = 115; // seconds

	// after the three above: AttachSocket hands them to every new socket
	Listen(Name);
}

CHttpServer::EEndpointKind CHttpServer::ParseEndpoint(const QString& Name, QHostAddress* pAddress, quint16* pPort, QString* pError)
{
	if(Name.isEmpty())
	{
		if(pError) *pError = tr("empty endpoint name");
		return eEndpointMalformed;
	}

	QString Host;
	QString Port;

	if(Name.startsWith('['))	// bracketed IPv6: [::1]:8080
	{
		int Close = Name.indexOf(']');
		if(Close == -1 || !Name.mid(Close + 1).startsWith(':'))
		{
			if(pError) *pError = tr("'%1' looks like a bracketed IPv6 endpoint but is not one").arg(Name);
			return eEndpointMalformed;	// '[' is not legal in a pipe name anyway
		}
		Host = Name.mid(1, Close - 1);
		Port = Name.mid(Close + 2);
	}
	else
	{
		int Sep = Name.lastIndexOf(':');
		if(Sep == -1)
			return eEndpointLocal;		// no port at all - a local socket name

		Host = Name.left(Sep);
		Port = Name.mid(Sep + 1);

		// A colon is legal in a pipe name, so only treat this as an endpoint
		// attempt when the tail actually looks like a port. Otherwise it is a
		// local name that happens to contain a colon.
		bool bNumeric = !Port.isEmpty();
		for(int i = 0; bNumeric && i < Port.length(); i++)
			bNumeric = Port.at(i).isDigit();
		if(!bNumeric)
			return eEndpointLocal;
	}

	bool bOk = false;
	uint PortNo = Port.toUInt(&bOk);
	if(!bOk || PortNo == 0 || PortNo > 65535)
	{
		if(pError) *pError = tr("'%1' is not a valid port").arg(Port);
		return eEndpointMalformed;
	}

	QHostAddress AddressValue;
	if(!AddressValue.setAddress(Host))
	{
		if(pError) *pError = tr("'%1' is not a valid IP address").arg(Host);
		return eEndpointMalformed;
	}

	// Deliberately literal, matching every other server's convention:
	// "0.0.0.0" binds every IPv4 interface, "[::]" every IPv6 one. setAddress
	// already yields AnyIPv4/AnyIPv6 for those, and QTcpServer handles both -
	// so no remapping to the dual-stack QHostAddress::Any, which would bind
	// more than the name asked for.

	if(pAddress) *pAddress = AddressValue;
	if(pPort) *pPort = (quint16)PortNo;
	return eEndpointNetwork;
}

bool CHttpServer::IsValidLocalName(const QString& Name, QString* pError)
{
	if(Name.isEmpty())
	{
		if(pError) *pError = tr("empty local socket name");
		return false;
	}

	// QLocalServer builds \\.\pipe\<name> on Windows, so the name itself must
	// not contain a separator and has to leave room for that prefix.
	if(Name.length() > 247)
	{
		if(pError) *pError = tr("local socket name is too long (%1 characters, 247 max)").arg(Name.length());
		return false;
	}
	if(Name.contains('\\') || Name.contains('/'))
	{
		if(pError) *pError = tr("local socket name must not contain a path separator");
		return false;
	}
	if(Name == "." || Name == "..")
	{
		if(pError) *pError = tr("'%1' is not a usable local socket name").arg(Name);
		return false;
	}
	foreach(const QChar& Char, Name)
	{
		if(Char.unicode() < 0x20 || Char.unicode() == 0x7F)
		{
			if(pError) *pError = tr("local socket name must not contain control characters");
			return false;
		}
	}
	return true;
}

bool CHttpServer::Listen(const QString& Name)
{
	QHostAddress Address;
	quint16 Port = 0;
	QString Error;

	switch(ParseEndpoint(Name, &Address, &Port, &Error))
	{
		case eEndpointNetwork:
			if(!Listen(Port, Address))
			{
				LogLine(eLogError, tr("Failed to listen on %1").arg(Name));
				return false;
			}
			return true;

		case eEndpointLocal:
			if(!IsValidLocalName(Name, &Error))
			{
				LogLine(eLogError, tr("Invalid local socket name '%1': %2").arg(Name).arg(Error));
				return false;
			}
			return ListenLocal(Name);

		default:
			// Rejected rather than silently opening a strangely named pipe.
			LogLine(eLogError, tr("Invalid endpoint '%1': %2").arg(Name).arg(Error));
			return false;
	}
}

void CHttpServer::LogLine(int Level, const QString& Message)
{
	if (Level == eLogError || Level == eLogWarning)
		qWarning("[CHttpServer] %s", Message.toUtf8().constData());
	else
		qDebug("[CHttpServer] %s", Message.toUtf8().constData());
}

bool CHttpServer::Listen(int Port, const QHostAddress& Address)
{
	QTcpServer* pListener = new QTcpServer(this);
	pListener->listen(Address, Port);
    if (!pListener->isListening()) 
	{
		delete pListener;
		return false;
	}
	connect(pListener, SIGNAL(newConnection()), this, SLOT(OnConnection()));

	m_Listners.insert(Port, pListener);
	if(!m_Port)
		m_Port = Port;	// the first TCP endpoint is the one GetHandler defaults to
	return true;
}

QHostAddress CHttpServer::ListenAddress(int Port) const
{
	QTcpServer* pListener = m_Listners.value(Port);
	return pListener ? pListener->serverAddress() : QHostAddress();
}

bool CHttpServer::ListenLocal(const QString& Name)
{
	if(m_LocalListners.contains(Name))
		return true;

	QLocalServer* pListener = new QLocalServer(this);
	// The caller owns the access decision: set SocketOptions before calling if
	// the socket must be restricted to one user.
	if(!pListener->listen(Name))
	{
		LogLine(eLogError, tr("Failed to listen on local socket %1: %2").arg(Name).arg(pListener->errorString()));
		delete pListener;
		return false;
	}
	connect(pListener, SIGNAL(newConnection()), this, SLOT(OnLocalConnection()));

	m_LocalListners.insert(Name, pListener);
	return true;
}

void CHttpServer::Process()
{
	foreach(CHttpSocket* pHttpSocket, m_Sockets.values())
	{
		switch(pHttpSocket->m_TransactionState)
		{
			case CHttpSocket::eWaiting:
				if(!pHttpSocket->KeepAlive())
					pHttpSocket->Drop();
				break;
			case CHttpSocket::eReading:
				if(pHttpSocket->m_pSocket->bytesAvailable() > 0)
				{
					pHttpSocket->TryReadSocket();
					HandleSocket(pHttpSocket);
				}
				break;
			case CHttpSocket::eWriting:
				pHttpSocket->TrySendBuffer();
				break;
		}
	}
}

void CHttpServer::RegisterHandler(CHttpHandler* pHandler, QString Path, quint16 LocalPort)
{
	ASSERT(!m_Handlers.contains(Path));
	m_Handlers.insert(Path, qMakePair(pHandler, LocalPort));
}

CHttpHandler* CHttpServer::GetHandler(QString Path, quint16 LocalPort)
{
	for(;;)
	{
		QPair<CHttpHandler*, quint16> Handler = m_Handlers.value(Path);
		if(Handler.first && ((Handler.second ? Handler.second : m_Port) == LocalPort))
			return Handler.first;

		int Pos = Path.lastIndexOf("/");
		if(Pos == -1)
			break;
		Path.truncate(Pos);
	}
	return NULL;
}

void CHttpServer::OnConnection()
{
	QTcpServer* pListener = qobject_cast<QTcpServer*>(sender());
	if(!pListener)
		return;
	while(QTcpSocket* pSocket = pListener->nextPendingConnection())
		AttachSocket(pSocket);
}

void CHttpServer::OnLocalConnection()
{
	QLocalServer* pListener = qobject_cast<QLocalServer*>(sender());
	if(!pListener)
		return;
	while(QLocalSocket* pSocket = pListener->nextPendingConnection())
		AttachSocket(pSocket);
}

// Both transports are wired identically - the signals are the same and the
// HTTP code only ever sees a QIODevice.
void CHttpServer::AttachSocket(QIODevice* pSocket)
{
	connect(pSocket, SIGNAL(readyRead()), this, SLOT(OnReadyRead()));
	connect(pSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(OnBytesWritten(qint64)));
	connect(pSocket, SIGNAL(disconnected()), this, SLOT(OnDisconnect()));
	HttpTransport::SetReadBufferSize(pSocket, m_TransferBufferSize);
	m_Sockets.insert(pSocket, new CHttpSocket(pSocket, m_KeepAlive, this));
}

void CHttpServer::OnReadyRead()
{
	QIODevice* pSocket = qobject_cast<QIODevice*>(sender());
	if(!pSocket || !m_Sockets.contains(pSocket))
		return;

	CHttpSocket* pHttpSocket = m_Sockets.value(pSocket);
	pHttpSocket->TryReadSocket();

	HandleSocket(pHttpSocket);
}

void CHttpServer::OnBytesWritten(qint64 bytes)
{
	Q_UNUSED(bytes); // the count is irrelevant - any progress just re-pumps the socket

	QIODevice* pSocket = qobject_cast<QIODevice*>(sender());
	if(!pSocket || !m_Sockets.contains(pSocket))
		return;

	CHttpSocket* pHttpSocket = m_Sockets.value(pSocket);
	if(pHttpSocket->m_TransactionState == CHttpSocket::eWriting)
		pHttpSocket->TrySendBuffer();
}

void CHttpServer::OnDisconnect()
{
	QIODevice* pSocket = qobject_cast<QIODevice*>(sender());
	if(!pSocket || !m_Sockets.contains(pSocket))
		return;

	CHttpSocket* pHttpSocket = m_Sockets.take(pSocket);
	if(CHttpHandler* pHandler = GetHandler(pHttpSocket->GetPath(), pHttpSocket->GetLocalPort()))
		pHandler->ReleaseRequest(pHttpSocket);
	pHttpSocket->deleteLater();
	pSocket->deleteLater();
}

void CHttpServer::HandleSocket(CHttpSocket* pHttpSocket)
{
	// m_TransactionState, not m_ResponseCode: comparing an HTTP status against
	// EState meant this could only fire for a response code of 2 or 4
	if(pHttpSocket->m_TransactionState == CHttpSocket::eHandling || pHttpSocket->m_TransactionState == CHttpSocket::eWriting)
	{
		LogLine(eLogError, tr("recived new request, befoure the previouse response was completly sent"));
		return;
	}

	if(pHttpSocket->m_RequestType == CHttpSocket::eNone) // are we still completing the header?
	{
		int End = pHttpSocket->m_RequestBuffer.indexOf("\r\n\r\n");
		if(End == -1) // hreader not yet complee
			return;

		ASSERT(pHttpSocket->m_TransactionState == CHttpSocket::eWaiting);
		pHttpSocket->m_TransactionState = CHttpSocket::eReading;

		//TRACE(L"Socket %d recived hreader %S", (int)pHttpSocket, pHttpSocket->m_RequestBuffer.left(End).data());
		QStringList RequestHeader = QString(pHttpSocket->m_RequestBuffer.left(End)).split('\n');
		pHttpSocket->m_RequestBuffer.remove(0,End+4);
		pHttpSocket->m_Downloaded -= End+4;

		if(int Code = pHttpSocket->HandleHeader(RequestHeader))
			pHttpSocket->RespondWithError(Code);
		else if(CHttpHandler* pHandler = GetHandler(pHttpSocket->GetPath(), pHttpSocket->GetLocalPort()))
			pHandler->HandleRequest(pHttpSocket);
		else
			pHttpSocket->RespondWithError(404, "No service abvailable for this this path");
	}

	if(pHttpSocket->m_TransactionState == CHttpSocket::eReading)
		pHttpSocket->HandleData();
	else if(pHttpSocket->m_TransactionState == CHttpSocket::eFailing)
		pHttpSocket->SendResponse();
}
