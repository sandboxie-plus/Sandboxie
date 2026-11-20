#pragma once
#include "../../mischelpers_global.h"
#include <QHostAddress>
#include <QTcpServer>
#include <QLocalServer>
class CHttpSocket;
class CHttpServer;
class CHttpHandler;
#include "HttpHelper.h"
#include "HttpTransport.h"

class MISCHELPERS_EXPORT CHttpHandler
{
protected:
	friend class CHttpServer;
	friend class CHttpSocket;

	virtual void	HandleRequest(CHttpSocket* pRequest) = 0;
	virtual void	ReleaseRequest(CHttpSocket* pRequest) = 0;
};


class MISCHELPERS_EXPORT CHttpServer: public QObject
{
	Q_OBJECT

public:
	enum ELogLevel
	{
		eLogError = 0,
		eLogWarning,
		eLogDebug,
	};

	// One name covers both transports:
	//
	//   "1.2.3.4:8080"   a specific IPv4 interface, that port
	//   "0.0.0.0:8080"   any IPv4 interface
	//   "[::1]:8080"     a specific IPv6 interface (brackets required)
	//   "[::]:8080"      any IPv6 interface
	//   "Sandboxie_MCP"  a local socket - named pipe on Windows, unix socket
	//                    elsewhere
	//
	// Anything that is not an <address>:<port> pair is taken as a local socket
	// name and validated as one. A string that clearly *meant* to be an endpoint
	// but is malformed - a bad port, an unparseable address - is rejected rather
	// than silently becoming a pipe called "1.2.3.4:99999".
	//
	// Construction never throws: on failure the server simply is not listening,
	// the reason goes through LogLine, and IsListening() returns false.
	CHttpServer(const QString& Name, QObject* qObject = NULL);

	void			Process();

	bool			IsListening() const	{return !m_Listners.isEmpty() || !m_LocalListners.isEmpty();}
	int				GetPort()	{return m_Port;}	// 0 in local-socket mode

	// Explicit forms, for listening on more than one endpoint.
	bool			Listen(const QString& Name);	// same grammar as the constructor
	bool			Listen(int Port, const QHostAddress& Address = QHostAddress::Any);
	QHostAddress	ListenAddress(int Port) const;	// null if nothing listens there

	// Handlers registered without an explicit port keep matching here, because a
	// local socket reports port 0 and GetHandler reads a registered 0 as "the
	// server's own port" - which is also 0 in local-socket mode.
	bool			ListenLocal(const QString& Name);
	bool			IsListeningLocal(const QString& Name) const	{return m_LocalListners.contains(Name);}

	enum EEndpointKind
	{
		eEndpointNetwork = 0,	// parsed as <address>:<port>
		eEndpointLocal,			// no endpoint syntax present - a local socket name
		eEndpointMalformed,		// meant to be an endpoint, but is not a valid one
	};

	// Classify a name without listening on it. pError, when given, receives a
	// human-readable reason for eEndpointMalformed or an invalid local name.
	static EEndpointKind	ParseEndpoint(const QString& Name, QHostAddress* pAddress = NULL, quint16* pPort = NULL, QString* pError = NULL);
	static bool				IsValidLocalName(const QString& Name, QString* pError = NULL);

	// Overridden by the host to route these into its own log; the default
	// writes to qWarning/qDebug.
	virtual void	LogLine(int Level, const QString& Message);

	void			RegisterHandler(CHttpHandler* pHandler, QString Path = "", quint16 LocalPort = 0);

	// Goes out as the Server: header. Defaults to the historical value so
	// nothing that relied on it changes; hosts should set their own.
	void			SetServerName(const QString& Name) {m_ServerName = Name;}
	QString			GetServerName() const {return m_ServerName;}

	void			SetKeepAlive(quint32 KeepAlive) {m_KeepAlive = KeepAlive;}
	void			SetTransferBufferSize(quint64 TransferBufferSize) {m_TransferBufferSize = TransferBufferSize;}

private slots:
	void			OnConnection();
	void			OnLocalConnection();

	void			OnReadyRead();
	void			OnBytesWritten(qint64 bytes);
	void			OnDisconnect();

protected:
	friend class CHttpSocket;

	void			HandleSocket(CHttpSocket* pHttpSocket);
	CHttpHandler*	GetHandler(QString Path, quint16 LocalPort);
	void			AttachSocket(QIODevice* pSocket);

	// Keyed on QIODevice: the value is a QTcpSocket or a QLocalSocket and the
	// HTTP code does not care which.
	QMap<QIODevice*, CHttpSocket*>	m_Sockets;
	QMap<QString, QPair<CHttpHandler*, quint16> >	m_Handlers;

	QMap<int, QTcpServer*>		m_Listners;
	QMap<QString, QLocalServer*>	m_LocalListners;
	int				m_Port;

	QString			m_ServerName;
	quint64			m_TransferBufferSize;
	quint32			m_KeepAlive;
};
