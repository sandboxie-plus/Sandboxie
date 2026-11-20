#pragma once
#include <QObject>
#include <QSet>
#include "McpProtocol.h"
#include "../../MiscHelpers/Common/HttpServer/HttpServer.h"

class CMcpHttpServer;
class CMcpDocs;
class CMcpSession;
class CMcpSessionManager;
class CMcpStream;
class CMcpAudit;
class CMcpConsent;

//
// CMcpServer - the MCP endpoint, served over a named pipe by default.
//
// Dual-era (see McpProtocol.h): modern per-request callers and legacy
// initialize-handshake callers are both answered from one dispatcher.
//
// Not reachable from inside a box. The gate is layer 1 of the isolation
// design: the connecting process id comes from the transport
// (GetNamedPipeClientProcessId on a pipe, the TCP table otherwise) and is
// resolved against the *driver*, not any cache. Three outcomes, and only one
// of them is allowed through - a query that fails is refused, so a broken
// lookup cannot read as "not sandboxed".
//
class CMcpServer : public QObject, public CHttpHandler
{
	Q_OBJECT

public:
	CMcpServer(QObject* parent = NULL);
	~CMcpServer();

	// Endpoint is a pipe name ("SbieMcp") or an address:port. Returns false and
	// logs if it could not listen.
	bool			Start(const QString& Endpoint);
	void			Stop();
	bool			IsListening() const;
	QString			GetEndpoint() const		{ return m_Endpoint; }

	// The path the endpoint is served on, as a client would configure it.
	static const char* const kEndpointPath;

	// The sessions, so the GUI can show who is connected and relay a pairing
	// code. Never NULL once Start has succeeded.
	CMcpSessionManager*	GetSessions() const	{ return m_pSessions; }
	CMcpAudit*		GetAudit() const	{ return m_pAudit; }
	CMcpConsent*	GetConsent() const	{ return m_pConsent; }

	// False when the user has waived the pairing code and no administrator
	// has forbidden that.
	static bool		IsPairingRequired();

signals:
	void			LogMessage(int Level, const QString& Message);

protected:
	friend class CMcpHttpServer;

	virtual void	HandleRequest(CHttpSocket* pRequest);
	virtual void	ReleaseRequest(CHttpSocket* pRequest);

	void			Log(int Level, const QString& Message);

private slots:
	void			OnPump();
	void			OnStreamDestroyed(QObject* pStream);
	void			OnSessionPaired(const QString& ClientName);
	void			OnConsentDecided(const QString& What, bool bAllowed, const QString& Detail);

	// The body is complete - CHttpSocket emits this when the declared
	// Content-Length has arrived. HandleRequest fires on the header alone, so
	// the JSON is not readable yet at that point.
	void			OnBodyComplete();

private:
	// Returns false when the caller must be refused; Reason is for the log and
	// the response body.
	bool			CheckPeer(CHttpSocket* pRequest, QString* pReason);

	void			Dispatch(CHttpSocket* pRequest, const QByteArray& Body);
	void			SendJson(CHttpSocket* pRequest, const QByteArray& Json, int Code = 200);
	void			SendHttpError(CHttpSocket* pRequest, int Code, const QString& Message);

	// Method handlers. Each returns the JSON-RPC payload to send, or sets
	// pErrorCode and returns an empty object.
	QJsonObject		OnDiscover(const McpProtocol::SRequest& Request);
	QJsonObject		OnInitialize(const McpProtocol::SRequest& Request);
	QJsonObject		OnToolsList(const McpProtocol::SRequest& Request, CMcpSession* pSession);
	QJsonObject		OnToolsCall(const McpProtocol::SRequest& Request, CMcpSession* pSession, int* pErrorCode, QString* pErrorMessage);

	// The pair tool. Handled here rather than in McpTools because it is the one
	// tool whose effect is on the session itself.
	QJsonObject		CallPair(const QJsonObject& Arguments, CMcpSession* pSession);
	static QJsonObject	PairToolDescription();

	// Answers a request with an SSE stream instead of a single JSON object, and
	// returns the stream so the caller can keep writing to it. The socket owns
	// the stream from then on.
	CMcpStream*		BeginSse(CHttpSocket* pRequest);

	// subscriptions/listen: the response stream stays open and carries the
	// change notifications the client opted in to. This revision has no other
	// way to deliver one - the standalone GET stream is gone.
	void			OnSubscriptionsListen(CHttpSocket* pRequest, const McpProtocol::SRequest& Request, CMcpSession* pSession);

	// Delivers a notification to every listener belonging to a session.
	void			NotifySession(const CMcpSession* pSession, const QString& Method, const QJsonObject& Params = QJsonObject());

	QJsonObject		ServerInfo() const;
	QJsonObject		Capabilities() const;
	QString			Instructions() const;

	CMcpHttpServer*	m_pHttp;

	// Mounted once per armed session rather than per call: opening a 7z and
	// parsing a 160 KB index on every lookup would be absurd, and the pack does
	// not change underneath a running session.
	CMcpDocs*		m_pDocs;
	class CMcpKnowledge* m_pKnowledge;
	CMcpSessionManager*	m_pSessions;
	CMcpAudit*		m_pAudit;
	CMcpConsent*	m_pConsent;
	QTimer*			m_pIdleTimer;

	// Drives CHttpServer::Process, which is what moves queued stream events out
	// to the socket and reaps sockets whose keep-alive has lapsed. Nothing else
	// calls it, so without this a stream would only flush when the next request
	// happened to arrive.
	QTimer*			m_pPumpTimer;

	// Open listen streams, by the session that owns them.
	QMultiMap<const CMcpSession*, CMcpStream*>	m_Listeners;
	QString			m_Endpoint;

	// Sockets whose body we are still waiting for. Only used to avoid acting on
	// a socket that was released in the meantime.
	QSet<CHttpSocket*>	m_Pending;
};
