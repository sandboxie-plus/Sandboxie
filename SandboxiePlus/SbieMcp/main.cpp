//
// SbieMcp - the stdio bridge to SandMan's MCP endpoint.
//
// Every MCP client that matters today speaks stdio: it launches a server as a
// subprocess and exchanges newline-delimited JSON-RPC over stdin and stdout.
// SandMan serves MCP over a named pipe instead, because a pipe inherits the
// kernel IPC default-deny that keeps sandboxed processes out and needs no
// listening socket. This program is the join between the two.
//
// It is deliberately thin. It does not interpret MCP, implement tools, or hold
// state: it frames each message as an HTTP POST, derives the headers the
// transport requires from the message itself, and copies the answer back. Every
// decision that matters - what is allowed, what needs consent, who is asking -
// belongs to SandMan, which can see the user.
//
// Useless without SandMan, hence it lives here rather than in SandboxieTools.
//
#include <QCoreApplication>
#include <QLocalSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QElapsedTimer>
#include <QProcess>
#include <QCommandLineParser>
#include <QDir>
#include <QMutex>
#include <QMutexLocker>
#include <QAtomicInt>
#include <QThread>
#include <stdio.h>

#ifdef Q_OS_WIN
#include <windows.h>
#include <tlhelp32.h>
#include <fcntl.h>
#include <io.h>
#endif

static const char* const kDefaultPipe = "SbieMcp";
static const char* const kEndpointPath = "/mcp";

static bool g_bVerbose = false;

// Set when stdin closes. The notification stream reconnects on its own, and
// this is how it learns not to.
static QAtomicInt g_bStopping(0);

static void LogLine(const QString& Message)
{
	// stderr is the only channel the stdio transport leaves free for logging:
	// anything on stdout that is not a JSON-RPC message breaks the client.
	if (g_bVerbose)
		fprintf(stderr, "[SbieMcp] %s\n", qUtf8Printable(Message));
}

static void Fatal(const QString& Message)
{
	fprintf(stderr, "[SbieMcp] %s\n", qUtf8Printable(Message));
}

//
// Reads one line from stdin, blocking. Returns false at end of input.
//
// Deliberately not QFile-on-stdin with a notifier: the exchange is strictly
// request then response, so a blocking read is both correct and simpler than an
// event loop that would only ever have one thing to do.
//
static bool ReadLine(QByteArray* pLine)
{
	pLine->clear();
	for (;;)
	{
		const int Char = fgetc(stdin);
		if (Char == EOF)
			return !pLine->isEmpty();
		if (Char == '\n')
			return true;
		if (Char != '\r')
			pLine->append((char)Char);
	}
}

// A long-lived stream is served from its own thread, so stdout has more than
// one writer and a half-written line would corrupt the client's framing.
static QMutex g_OutMutex;

static void WriteMessage(const QByteArray& Json)
{
	// One message per line, no embedded newlines - the stdio framing.
	QByteArray Line = Json;
	Line.replace('\r', "").replace('\n', "");
	Line.append('\n');

	QMutexLocker Lock(&g_OutMutex);
	fwrite(Line.constData(), 1, Line.size(), stdout);
	fflush(stdout);
}

//
// Header values must be plain ASCII. A name that is not gets the base64
// sentinel the spec defines, which the server decodes before comparing it to
// the body.
//
static QByteArray EncodeHeaderValue(const QString& Value)
{
	bool bSafe = !Value.isEmpty();
	foreach(const QChar& Char, Value) {
		const ushort Code = Char.unicode();
		if (Code < 0x21 || Code > 0x7E) { bSafe = false; break; }
	}
	// A plain value that happens to look like the sentinel must be encoded too,
	// or the server would decode something that was never encoded.
	if (bSafe && Value.startsWith("=?base64?") && Value.endsWith("?="))
		bSafe = false;

	if (bSafe)
		return Value.toLatin1();
	return "=?base64?" + Value.toUtf8().toBase64() + "?=";
}

//
// The MCP client is our parent: it launched this bridge as a subprocess, which
// is what the stdio transport is. Naming it in the activation prompt is the
// difference between "something wants to connect" and "Claude Code wants to
// connect", and the user can only answer the second one sensibly.
//
// For display only. It is read from the process list, so it is not evidence of
// anything - the box check in SandMan is what actually decides.
//
static QString ParentProcessName()
{
#ifdef Q_OS_WIN
	DWORD ParentPid = 0;
	HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (Snapshot == INVALID_HANDLE_VALUE)
		return QString();

	PROCESSENTRY32W Entry;
	Entry.dwSize = sizeof(Entry);
	const DWORD Us = GetCurrentProcessId();
	if (Process32FirstW(Snapshot, &Entry)) {
		do {
			if (Entry.th32ProcessID == Us) { ParentPid = Entry.th32ParentProcessID; break; }
		} while (Process32NextW(Snapshot, &Entry));
	}

	QString Name;
	if (ParentPid) {
		Entry.dwSize = sizeof(Entry);
		if (Process32FirstW(Snapshot, &Entry)) {
			do {
				if (Entry.th32ProcessID == ParentPid) {
					Name = QString::fromWCharArray(Entry.szExeFile);
					break;
				}
			} while (Process32NextW(Snapshot, &Entry));
		}
	}
	CloseHandle(Snapshot);

	// A colon would be read as the separator in the op string, and a newline or
	// a quote has no business in a message box.
	Name.remove(':').remove('"').remove('\r').remove('\n');
	return Name.trimmed();
#else
	return QString();
#endif
}

static QList<QByteArray> ExtractSseMessages(const QByteArray& Body);
static QList<QByteArray> TakeCompleteSseEvents(QByteArray* pBuffer);
static QByteArray MakeTransportErrorFwd(const QJsonValue& Id, const QString& Message);

struct SHttpResponse
{
	int			Code = 0;
	QString		ContentType;
	QByteArray	Body;
	bool		bOk = false;
	// The reply was an SSE stream and has already been written to stdout, so
	// the caller must not write it again.
	bool		bStreamed = false;
	QString		Error;
};

class CBridge
{
public:
	CBridge(const QString& PipeName) : m_PipeName(PipeName) {}

	bool Connect(int TimeoutMs)
	{
		if (m_Socket.state() == QLocalSocket::ConnectedState)
			return true;

		// Retried rather than waited on, because there is nothing to wait for:
		// a pipe that does not exist yet fails instantly with "Invalid name"
		// (which is how Qt reports ERROR_FILE_NOT_FOUND), and
		// waitForConnected returns at once rather than honouring the timeout.
		//
		// That mattered: the long timeout here is the window in which a user
		// reads the connection prompt and clicks Allow. Spending it in a
		// single instant attempt meant the bridge told the client we were not
		// accepting connections while the prompt was still on screen - the
		// first-run path, failing for the people least able to tell why.
		QElapsedTimer Timer; Timer.start();
		for (;;) {
			m_Socket.abort();
			m_Socket.connectToServer(m_PipeName);
			// A short wait per attempt: a pipe that exists answers at once, and
			// one that does not is a sleep between retries either way.
			if (m_Socket.waitForConnected(qMin(TimeoutMs, 250))) {
				LogLine(QString("connected to %1").arg(m_PipeName));
				return true;
			}
			m_LastError = m_Socket.errorString();
			if (Timer.elapsed() >= TimeoutMs)
				return false;
			QThread::msleep(100);
			if (g_bStopping.loadAcquire())
				return false;
		}
	}

	bool IsConnected() const { return m_Socket.state() == QLocalSocket::ConnectedState; }
	QString LastError() const { return m_LastError; }

	SHttpResponse Post(const QJsonObject& Message, const QByteArray& RawBody);

private:
	void Emit(const QByteArray& Json) const;
	bool ReadAtLeast(QByteArray* pBuffer, int Bytes, int TimeoutMs);
	bool ReadUntil(QByteArray* pBuffer, const QByteArray& Marker, int TimeoutMs);

	QString			m_PipeName;
	QLocalSocket	m_Socket;
	QString			m_LastError;
	// Long enough for a person to read a confirmation and decide, short enough
	// to eventually give up if SandMan has stopped answering.
	int				m_HeaderTimeoutMs = 10 * 60 * 1000;
	// A request-scoped stream should not be waited on forever, but a listen
	// stream is supposed to stay quiet for long stretches.
	int				m_StreamTimeoutMs = 60000;
	// On our own notification stream, where the client asked for nothing and
	// so must be told nothing except what the server announced.
	bool			m_bNotificationsOnly = false;

public:
	void			SetStreamTimeout(int Ms)	{ m_StreamTimeoutMs = Ms; }
	void			SetNotificationsOnly(bool bOn)	{ m_bNotificationsOnly = bOn; }
};

//
// Everything the server sends on a stream goes through here on its way to
// stdout.
//
// On a stream the client opened that is a straight copy. On the one this
// bridge opens for itself it is a filter: that stream also carries an
// acknowledgement and a response to the request that opened it, and both
// would reach the client as answers to something it never sent - which a
// client is entitled to treat as a protocol violation by us.
//
void CBridge::Emit(const QByteArray& Json) const
{
	if (m_bNotificationsOnly) {
		const QJsonObject Message = QJsonDocument::fromJson(Json).object();
		// A notification is a method with no id. Anything with an id is an
		// answer to us.
		const bool bNotification = !Message.contains("id") && Message.contains("method");
		// And the one notification that is also ours rather than the
		// client's: the stream saying it is live.
		const bool bOurs = Message.value("method").toString()
			== "notifications/subscriptions/acknowledged";
		if (!bNotification || bOurs) {
			LogLine(QString("kept off stdout: %1").arg(QString::fromUtf8(Json.left(160))));
			return;
		}
		LogLine(QString("forwarding %1").arg(Message.value("method").toString()));
	}
	WriteMessage(Json);
}

bool CBridge::ReadUntil(QByteArray* pBuffer, const QByteArray& Marker, int TimeoutMs)
{
	QElapsedTimer Timer; Timer.start();
	while (!pBuffer->contains(Marker)) {
		if (Timer.elapsed() > TimeoutMs) { m_LastError = "timed out"; return false; }
		if (m_Socket.waitForReadyRead(200))
			pBuffer->append(m_Socket.readAll());
		else if (m_Socket.state() != QLocalSocket::ConnectedState) {
			m_LastError = "the connection closed"; return false;
		}
	}
	return true;
}

bool CBridge::ReadAtLeast(QByteArray* pBuffer, int Bytes, int TimeoutMs)
{
	QElapsedTimer Timer; Timer.start();
	while (pBuffer->size() < Bytes) {
		if (Timer.elapsed() > TimeoutMs) { m_LastError = "timed out"; return false; }
		if (m_Socket.waitForReadyRead(200))
			pBuffer->append(m_Socket.readAll());
		else if (m_Socket.state() != QLocalSocket::ConnectedState) {
			m_LastError = "the connection closed"; return false;
		}
	}
	return true;
}

SHttpResponse CBridge::Post(const QJsonObject& Message, const QByteArray& RawBody)
{
	SHttpResponse Response;

	const QString Method = Message.value("method").toString();
	const QJsonObject Params = Message.value("params").toObject();
	const QJsonObject Meta = Params.value("_meta").toObject();
	const QString ProtocolVersion = Meta.value("io.modelcontextprotocol/protocolVersion").toString();

	QString Name;
	if (Params.contains("name"))
		Name = Params.value("name").toString();
	else if (Params.contains("uri"))
		Name = Params.value("uri").toString();

	QByteArray Request;
	Request += "POST " + QByteArray(kEndpointPath) + " HTTP/1.1\r\n";
	Request += "Host: localhost\r\n";
	Request += "Accept: application/json, text/event-stream\r\n";
	// Who is on the other end of this bridge. The server can resolve our image
	// path by itself and does not trust this for anything - it is for the name
	// shown in the session window, which should say "claude.exe" rather than
	// "an unidentified program".
	{
		const QString Parent = ParentProcessName();
		Request += "User-Agent: SbieMcp";
		if (!Parent.isEmpty())
			Request += " (" + EncodeHeaderValue(Parent) + ")";
		Request += "\r\n";
	}
	Request += "Content-Type: application/json\r\n";
	// Mirror only what the body actually declares. A modern message carries its
	// version in _meta and the header must match it; a legacy initialize
	// declares none, and inventing one here would be a header that disagrees
	// with the body - exactly what the server is required to reject.
	if (!ProtocolVersion.isEmpty())
		Request += "MCP-Protocol-Version: " + ProtocolVersion.toLatin1() + "\r\n";
	if (!Method.isEmpty())
		Request += "Mcp-Method: " + Method.toLatin1() + "\r\n";
	if (!Name.isEmpty())
		Request += "Mcp-Name: " + EncodeHeaderValue(Name) + "\r\n";
	Request += "Content-Length: " + QByteArray::number(RawBody.size()) + "\r\n";
	Request += "\r\n";
	Request += RawBody;

	if (m_Socket.write(Request) != Request.size()) {
		Response.Error = "the request could not be written";
		return Response;
	}
	m_Socket.waitForBytesWritten(5000);

	QByteArray Buffer;
	// Generous on purpose, and fifteen seconds was wrong. A tool that changes
	// anything puts a confirmation on the screen and the server holds the request
	// until the user answers, which routinely takes longer than a request-shaped
	// timeout. Giving up early told the agent the call had failed while the dialog
	// was still open - so a user who then clicked Allow applied a change the agent
	// had already been told did not happen.
	if (!ReadUntil(&Buffer, "\r\n\r\n", m_HeaderTimeoutMs)) {
		Response.Error = "no response header: " + m_LastError;
		return Response;
	}

	const int HeaderEnd = Buffer.indexOf("\r\n\r\n");
	const QByteArray Header = Buffer.left(HeaderEnd);
	Buffer.remove(0, HeaderEnd + 4);

	const QList<QByteArray> Lines = Header.split('\n');
	if (Lines.isEmpty()) { Response.Error = "empty response header"; return Response; }

	const QList<QByteArray> Status = Lines.first().simplified().split(' ');
	if (Status.count() >= 2)
		Response.Code = Status.at(1).toInt();

	qint64 ContentLength = -1;
	bool bChunked = false;
	for (int i = 1; i < Lines.count(); i++) {
		const QByteArray Line = Lines.at(i).trimmed();
		const int Colon = Line.indexOf(':');
		if (Colon == -1) continue;
		const QByteArray Key = Line.left(Colon).trimmed().toLower();
		const QByteArray Value = Line.mid(Colon + 1).trimmed();
		if (Key == "content-length") ContentLength = Value.toLongLong();
		else if (Key == "content-type") Response.ContentType = QString::fromLatin1(Value);
		else if (Key == "transfer-encoding" && Value.toLower().contains("chunked")) bChunked = true;
	}

	if (bChunked)
	{
		// Chunked is how an SSE stream arrives, and the events are written out
		// as each chunk lands rather than after the stream closes. Two reasons,
		// both learned the hard way: a progress notification is worthless if it
		// only appears once the work is finished, and a subscriptions/listen
		// stream never closes at all - buffering it would hang the bridge and
		// every request behind it.
		QByteArray Pending;
		for (;;) {
			if (!ReadUntil(&Buffer, "\r\n", m_StreamTimeoutMs)) { Response.Error = "truncated chunked body"; return Response; }
			const int Eol = Buffer.indexOf("\r\n");
			const qint64 Size = Buffer.left(Eol).trimmed().toLongLong(NULL, 16);
			Buffer.remove(0, Eol + 2);
			if (Size == 0)
				break;
			if (!ReadAtLeast(&Buffer, (int)(Size + 2), m_StreamTimeoutMs)) { Response.Error = "truncated chunk"; return Response; }
			Pending += Buffer.left((int)Size);
			Buffer.remove(0, (int)(Size + 2));

			// Whole events only: an event ends at a blank line, and half of one
			// is not something the client can be handed.
			foreach(const QByteArray& Event, TakeCompleteSseEvents(&Pending))
				Emit(Event);
			Response.bStreamed = true;
		}
		// Anything left without its terminating blank line still counts once
		// the stream has ended.
		foreach(const QByteArray& Event, ExtractSseMessages(Pending))
			Emit(Event);
	}
	else if (ContentLength > 0)
	{
		if (!ReadAtLeast(&Buffer, (int)ContentLength, 30000)) { Response.Error = "truncated body: " + m_LastError; return Response; }
		Response.Body = Buffer.left((int)ContentLength);
	}

	Response.bOk = true;
	return Response;
}

//
// Pulls every *complete* event out of a growing buffer, leaving any partial one
// behind. An event is terminated by a blank line.
//
static QList<QByteArray> TakeCompleteSseEvents(QByteArray* pBuffer)
{
	QList<QByteArray> Messages;
	for (;;) {
		// Both spellings of the event terminator: this end writes bare newlines,
		// but an intermediary may normalise them to CRLF.
		int End = pBuffer->indexOf("\n\n");
		int Skip = 2;
		const int CrEnd = pBuffer->indexOf("\r\n\r\n");
		if (CrEnd != -1 && (End == -1 || CrEnd < End)) { End = CrEnd; Skip = 4; }
		if (End == -1)
			break;

		const QByteArray Event = pBuffer->left(End);
		pBuffer->remove(0, End + Skip);
		Messages += ExtractSseMessages(Event);
	}
	return Messages;
}

//
// An SSE body carries one or more JSON-RPC messages in data: fields. Each event
// may span several data: lines, which are joined with newlines per the SSE
// specification.
//
static QList<QByteArray> ExtractSseMessages(const QByteArray& Body)
{
	QList<QByteArray> Messages;
	QByteArray Current;
	foreach(const QByteArray& Raw, Body.split('\n'))
	{
		const QByteArray Line = Raw.endsWith('\r') ? Raw.left(Raw.size() - 1) : Raw;
		if (Line.isEmpty()) {
			if (!Current.isEmpty()) { Messages.append(Current); Current.clear(); }
			continue;
		}
		if (Line.startsWith(':'))
			continue;	// a comment, used as a keep-alive
		if (Line.startsWith("data:")) {
			QByteArray Data = Line.mid(5);
			if (Data.startsWith(' ')) Data.remove(0, 1);
			if (!Current.isEmpty()) Current.append('\n');
			Current += Data;
		}
	}
	if (!Current.isEmpty())
		Messages.append(Current);
	return Messages;
}

//
// A long-lived stream gets its own connection and its own thread.
//
// subscriptions/listen is answered with a response stream that is *supposed*
// never to end: it stays open and delivers change notifications. Serving it on
// the main connection would park the request/response loop on it forever, so
// every later request would go unanswered - which is exactly what happened
// before this existed.
//
class CListenThread : public QThread
{
public:
	// bOurs: the bridge opened this one for itself rather than relaying a
	// request from the client. It is then filtered, retried, and silent about
	// its own failures, because there is no client request to fail.
	CListenThread(const QString& PipeName, const QJsonObject& Message, const QByteArray& RawBody,
			bool bOurs = false)
	 : m_PipeName(PipeName), m_Message(Message), m_RawBody(RawBody), m_bOurs(bOurs) {}

protected:
	virtual void run()
	{
		// Ours is reopened when it drops, because SandMan restarting or the
		// listener being turned off and on again would otherwise leave the
		// client deaf to every later change for the rest of the session - the
		// same silent failure this stream exists to prevent, just later.
		for (;;) {
			CBridge Bridge(m_PipeName);
			Bridge.SetNotificationsOnly(m_bOurs);

			if (!Bridge.Connect(5000)) {
				if (!m_bOurs) {
					WriteMessage(MakeTransportErrorFwd(m_Message.value("id"),
						QString("the bridge could not open a second connection for the stream: %1").arg(Bridge.LastError())));
					return;
				}
				LogLine(QString("the notification stream could not connect: %1").arg(Bridge.LastError()));
			}
			else {
				// No deadline worth enforcing: a quiet listen stream is the normal
				// case, and the keep-alive comments are what prove it is still alive.
				Bridge.SetStreamTimeout(0x7FFFFFFF);

				const SHttpResponse Response = Bridge.Post(m_Message, m_RawBody);
				if (!Response.bOk)
					LogLine(QString("the listen stream ended: %1").arg(Response.Error));
				else
					LogLine("the listen stream closed");
			}

			// A stream the client asked for is its business, not ours: it gets
			// one attempt and the client decides what to do about it.
			if (!m_bOurs)
				return;
			if (g_bStopping.loadAcquire())
				return;
			// Slow enough that a server which is gone for good is not hammered.
			QThread::msleep(2000);
			if (g_bStopping.loadAcquire())
				return;
		}
	}

private:
	QString		m_PipeName;
	QJsonObject	m_Message;
	QByteArray	m_RawBody;
	bool		m_bOurs = false;
};

static QByteArray MakeTransportError(const QJsonValue& Id, const QString& Message)
{
	QJsonObject Error;
	Error["code"] = -32603;	// internal error: the bridge failed, not the server
	Error["message"] = Message;

	QJsonObject Out;
	Out["jsonrpc"] = "2.0";
	Out["id"] = Id;
	Out["error"] = Error;
	return QJsonDocument(Out).toJson(QJsonDocument::Compact);
}

static QByteArray MakeTransportErrorFwd(const QJsonValue& Id, const QString& Message)
{
	return MakeTransportError(Id, Message);
}

int main(int argc, char** argv)
{
	QCoreApplication App(argc, argv);
	App.setApplicationName("SbieMcp");

#ifdef Q_OS_WIN
	// Binary mode, or the CRT rewrites \n into \r\n on stdout and the client
	// sees malformed framing.
	_setmode(_fileno(stdout), _O_BINARY);
	_setmode(_fileno(stdin), _O_BINARY);
#endif

	QCommandLineParser Parser;
	Parser.setApplicationDescription(
		"Bridges an MCP client that speaks stdio to the Sandboxie-Plus MCP endpoint,\n"
		"which SandMan serves over a named pipe.");
	Parser.addHelpOption();
	QCommandLineOption PipeOption(QStringList() << "pipe", "Pipe name to connect to.", "name", kDefaultPipe);
	QCommandLineOption VerboseOption(QStringList() << "verbose", "Log bridge activity to stderr.");
	QCommandLineOption NoActivateOption(QStringList() << "no-activate",
		"Fail immediately when nothing is listening, instead of asking SandMan to offer the user a prompt.");
	Parser.addOption(PipeOption);
	Parser.addOption(VerboseOption);
	Parser.addOption(NoActivateOption);
	Parser.process(App);

	g_bVerbose = Parser.isSet(VerboseOption);
	const QString PipeName = Parser.value(PipeOption);
	const bool bNoActivate = Parser.isSet(NoActivateOption);

	CBridge Bridge(PipeName);

	// Connect lazily: a client may launch this process long before it sends
	// anything, and failing at startup would be reported as "the server
	// crashed" rather than "nobody armed the connection".
	bool bWarned = false;
	// One per process. Opened on the first connection rather than at startup,
	// because there may be nothing listening yet.
	bool bListening = false;

	QByteArray Line;
	while (ReadLine(&Line))
	{
		if (Line.trimmed().isEmpty())
			continue;

		QJsonParseError ParseError;
		const QJsonDocument Doc = QJsonDocument::fromJson(Line, &ParseError);
		const QJsonObject Message = Doc.object();
		const QJsonValue Id = Message.value("id");
		const bool bNotification = !Message.contains("id");

		if (ParseError.error != QJsonParseError::NoError || !Doc.isObject()) {
			// Do not forward something we could not parse; the headers the
			// transport requires are derived from the body.
			if (!bNotification)
				WriteMessage(MakeTransportError(Id, QString("the bridge could not parse the message: %1").arg(ParseError.errorString())));
			continue;
		}

		if (!Bridge.Connect(3000))
		{
			if (!bNoActivate && !bWarned) {
				// The doorbell: ask the running SandMan to offer the user a
				// prompt. Rate-limited to once per process, so a client that
				// retries in a loop cannot spam prompts.
				bWarned = true;
				const QString Client = ParentProcessName();
				LogLine(QString("nothing is listening - asking SandMan to offer the connection to %1")
					.arg(Client.isEmpty() ? QString("an unidentified client") : Client));
				// Beside us, by path, rather than "whatever SandMan.exe resolves
				// to". We are installed next to it, while the search order also
				// takes in the working directory - which belongs to the client and
				// is nowhere near SandMan. Getting no prompt at all is the failure
				// this produced, and the silence was the worst part of it: the
				// bridge went on to wait two minutes for a decision nobody had been
				// asked to make.
				const QString SandMan = QCoreApplication::applicationDirPath() + "/SandMan.exe";
				const QStringList Args = QStringList() << "-op"
					<< (Client.isEmpty() ? QString("McpActivate") : QString("McpActivate:%1").arg(Client));
				if (!QProcess::startDetached(SandMan, Args)) {
					LogLine(QString("SandMan could not be started from %1 - nobody will be asked").arg(SandMan));
					// And no waiting for an answer to a question that was never
					// put: the error goes back now, with the two minutes unspent.
				}
				else {
					// The user has to read a prompt and decide, so wait well past
					// the usual connect timeout before giving up on them.
					Bridge.Connect(120000);
				}
			}
		}

		if (!Bridge.IsConnected()) {
			const QString Why = QString(
				"Sandboxie-Plus is not accepting agent connections. In SandMan, turn on "
				"Options > AI Agent Connection (MCP), then try again. (%1: %2)")
				.arg(PipeName).arg(Bridge.LastError());
			if (!bNotification)
				WriteMessage(MakeTransportError(Id, Why));
			else
				LogLine(Why);
			continue;
		}

		// The stream that carries the server's notifications, opened by us.
		//
		// This is what makes the tool list grow at the moment pairing succeeds.
		// The server announces that with notifications/tools/list_changed, and a
		// notification can only travel on a stream somebody opened - but no
		// stdio client opens one, because in the transport they were written
		// for, a server simply writes to stdout whenever it likes. Nobody was
		// asking, so nobody was told: a client that paired kept the tools it
		// had seen before pairing for the rest of the session, and the agent
		// was left telling the user to reconnect.
		//
		// The bridge is where that belongs. It owns the difference between the
		// two transports; the client should not have to know about a revision
		// it has never heard of.
		//
		// A client new enough to open its own gets two streams and hears each
		// change twice. Refetching a tool list twice is harmless, and is the
		// cheaper of the two mistakes: the other one is silence.
		if (!bListening) {
			bListening = true;
			QJsonObject Listen;
			Listen["jsonrpc"] = "2.0";
			// Distinctive on purpose. It never reaches the client, so if it ever
			// turns up in a log it should be obvious who sent it.
			Listen["id"] = "sbiemcp-internal-listen";
			Listen["method"] = "subscriptions/listen";
			const QByteArray Raw = QJsonDocument(Listen).toJson(QJsonDocument::Compact);

			CListenThread* pThread = new CListenThread(PipeName, Listen, Raw, true);
			QObject::connect(pThread, &QThread::finished, pThread, &QObject::deleteLater);
			pThread->start();
			LogLine("opened a notification stream of our own");
		}

		if (Message.value("method").toString() == "subscriptions/listen") {
			// Handed off, and the loop goes straight back to stdin.
			CListenThread* pThread = new CListenThread(PipeName, Message, Line);
			QObject::connect(pThread, &QThread::finished, pThread, &QObject::deleteLater);
			pThread->start();
			LogLine("subscriptions/listen handed to its own connection");
			continue;
		}

		const SHttpResponse Response = Bridge.Post(Message, Line);
		if (!Response.bOk) {
			if (!bNotification)
				WriteMessage(MakeTransportError(Id, QString("the bridge failed: %1").arg(Response.Error)));
			else
				LogLine(Response.Error);
			continue;
		}

		LogLine(QString("%1 -> %2 (%3 bytes)").arg(Message.value("method").toString()).arg(Response.Code).arg(Response.Body.size()));

		if (Response.bStreamed) {
			// The events went out as they arrived; there is nothing left to send.
			continue;
		}

		if (Response.Code == 202 || Response.Body.isEmpty()) {
			// An accepted notification has no reply, and stdio has nothing to
			// carry an HTTP status on its own.
			continue;
		}

		if (Response.ContentType.contains("text/event-stream")) {
			// A stream that carried no complete event, which is unusual but not
			// an error.
			foreach(const QByteArray& Event, ExtractSseMessages(Response.Body))
				WriteMessage(Event);
			continue;
		}

		// A JSON-RPC error body is a valid response and goes through unchanged:
		// the HTTP status is transport detail the client neither sees nor needs.
		WriteMessage(Response.Body);
	}

	// So the notification stream stops reconnecting behind us.
	g_bStopping.storeRelease(1);
	LogLine("input closed, exiting");
	return 0;
}
