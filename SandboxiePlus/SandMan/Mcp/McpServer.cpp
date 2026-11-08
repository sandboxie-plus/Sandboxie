#include "stdafx.h"
#include "McpServer.h"
#include "McpTools.h"
#include "McpDocs.h"
#include "McpKnowledge.h"
#include "McpSession.h"
#include "McpStream.h"
#include "McpAudit.h"
#include "McpConsent.h"
#include "McpOperate.h"
#include "McpReport.h"
#include "McpSessionWindow.h"
#include <QTimer>
#include "../SandMan.h"
#include "../SbiePlusAPI.h"
#include "../../MiscHelpers/Common/HttpServer/HttpSocket.h"
#include <QJsonDocument>
#include <QJsonArray>

const char* const CMcpServer::kEndpointPath = "/mcp";

//
// How a call is named in the session window. The arguments are summarised, not
// dumped: a box name is what makes "box_get_config" mean something, and a page
// of JSON in a list row means nothing.
//
static QString DescribeCall(const QString& Name, const QJsonObject& Arguments)
{
	QStringList Parts;
	foreach(const QString& Key, QStringList() << "box" << "setting" << "query" << "page" << "command") {
		const QJsonValue Value = Arguments.value(Key);
		if (Value.isString() && !Value.toString().isEmpty())
			Parts.append(Value.toString());
	}
	return Parts.isEmpty() ? Name : QString("%1  %2").arg(Name).arg(Parts.join(", "));
}

// The first line of a tool result, which the mutating tools write as a summary
// of what they did.
static QString ResultHeadline(const QJsonObject& Result)
{
	const QJsonArray Content = Result.value("content").toArray();
	if (Content.isEmpty())
		return QString();
	return Content.first().toObject().value("text").toString().section(QChar(10), 0, 0).trimmed();
}

//
// CHttpServer logs through a virtual; route it into SandMan's log rather than
// qWarning so a listener problem is visible where users look.
//
class CMcpHttpServer : public CHttpServer
{
public:
	CMcpHttpServer(const QString& Name, CMcpServer* pOwner)
	 : CHttpServer(Name, pOwner), m_pOwner(pOwner) {}

	virtual void LogLine(int Level, const QString& Message)
	{
		m_pOwner->Log(Level, Message);
	}

protected:
	CMcpServer* m_pOwner;
};

CMcpServer::CMcpServer(QObject* parent)
 : QObject(parent)
{
	m_pHttp = NULL;
	m_pDocs = NULL;
	m_pKnowledge = NULL;
	m_pSessions = new CMcpSessionManager(this);
	m_pAudit = new CMcpAudit();
	m_pConsent = new CMcpConsent(this);
	connect(m_pConsent, &CMcpConsent::Decided, this, &CMcpServer::OnConsentDecided);
	connect(m_pSessions, &CMcpSessionManager::PairingSucceeded, this, &CMcpServer::OnSessionPaired);
	m_pPumpTimer = new QTimer(this);
	// Fast enough that a queued event is not perceptibly delayed, slow enough
	// to be free: the loop walks a handful of sockets.
	m_pPumpTimer->setInterval(200);
	connect(m_pPumpTimer, &QTimer::timeout, this, &CMcpServer::OnPump);
	m_pIdleTimer = new QTimer(this);
	// One minute is plenty: the timeout is an hour, so the worst case is a
	// session that lingers a minute past it.
	m_pIdleTimer->setInterval(60 * 1000);
	connect(m_pIdleTimer, &QTimer::timeout, m_pSessions, &CMcpSessionManager::ExpireIdle);
	connect(m_pIdleTimer, &QTimer::timeout, this, [this]() { m_pSessions->DropDeparted(); });
	connect(m_pSessions, &CMcpSessionManager::SessionDeparted, this,
		[this](const QString& ClientName, bool bHadShadows) {
			m_pAudit->Record("session", "client gone, session released", QJsonObject{
				{ "client", ClientName }, { "hadShadows", bHadShadows } });
			if (bHadShadows)
				Log(CHttpServer::eLogDebug, tr("released a session whose client exited, discarding its test copies"));

			// The window stays open after the agent goes, because the history in
			// it is the answer to "what did that thing just do to my machine".
			const QString Who = ClientName.isEmpty() ? tr("the agent") : ClientName;
			CMcpSessionWindow* pWindow = theGUI->GetMcpWindow();
			pWindow->SetStatus(tr("disconnected"), Who);
			pWindow->ClearCode();
			pWindow->AddEntry(CMcpSessionWindow::eInfo, bHadShadows
				? tr("%1 exited; its test copies were discarded").arg(Who)
				: tr("%1 exited").arg(Who));
		});
}

CMcpServer::~CMcpServer()
{
	Stop();
	delete m_pAudit;
	m_pAudit = NULL;
}

void CMcpServer::OnPump()
{
	if (m_pHttp)
		m_pHttp->Process();
}

void CMcpServer::OnStreamDestroyed(QObject* pStream)
{
	// Keyed by value, so this removes it whichever session it belonged to.
	for (auto I = m_Listeners.begin(); I != m_Listeners.end(); ) {
		if (I.value() == pStream)
			I = m_Listeners.erase(I);
		else
			++I;
	}
}

bool CMcpServer::Start(const QString& Endpoint)
{
	Stop();

	// Checked before anything is opened. A name the protecting template does
	// not cover silently removes a layer of the isolation described in the
	// plan, and arming anyway would make that claim untrue without telling
	// anybody.
	QString Reason;
	if (!McpProtocol::IsEndpointNameSafe(Endpoint, &Reason)) {
		Log(CHttpServer::eLogError, tr("refusing to listen on '%1': %2").arg(Endpoint).arg(Reason));
		return false;
	}

	// The CHttpServer constructor listens, and reports a failure through the
	// virtual LogLine - which during base-class construction still resolves to
	// the base implementation, so that particular line goes to qWarning rather
	// than our log. Hence the explicit check and message here; do not "fix" it
	// by moving the logging into the override.
	m_pHttp = new CMcpHttpServer(Endpoint, this);
	if (!m_pHttp->IsListening()) {
		Log(CHttpServer::eLogError, tr("MCP server could not listen on '%1'").arg(Endpoint));
		Stop();
		return false;
	}

	// The documentation is the one capability that works with Sandboxie stopped,
	// so it is mounted regardless of driver state.
	if (!m_pDocs) {
		m_pDocs = new CMcpDocs();
		if (m_pDocs->Open())
			Log(CHttpServer::eLogDebug, tr("documentation pack mounted: %1 pages from %2")
				.arg(m_pDocs->GetPageCount()).arg(m_pDocs->GetPackPath()));
		else
			Log(CHttpServer::eLogWarning, tr("no documentation pack found - docs lookups will be unavailable"));
	}
	McpTools::SetDocs(m_pDocs);

	// The knowledge base rides along with the pack, and like the pack it does
	// not need the driver: knowing what a problem usually is does not require
	// Sandboxie to be running. Reloaded on every arm so an entry dropped into
	// kb/ is picked up without restarting SandMan - which is the whole point
	// of the loose directory.
	if (!m_pKnowledge)
		m_pKnowledge = new CMcpKnowledge();
	m_pKnowledge->Load(m_pDocs);
	McpTools::SetKnowledge(m_pKnowledge);
	if (m_pKnowledge->GetCount())
		Log(CHttpServer::eLogDebug, tr("knowledge base: %1 entries from %2")
			.arg(m_pKnowledge->GetCount()).arg(m_pKnowledge->GetSources().join(", ")));
	else
		Log(CHttpServer::eLogDebug, tr("no troubleshooting knowledge base found"));
	// Without a way to ask, nothing may change: McpOperate disables itself when
	// this is NULL, which is also what Stop leaves behind.
	McpOperate::SetConsent(m_pConsent);
	McpReport::SetConsent(m_pConsent);
	m_pIdleTimer->start();
	m_pPumpTimer->start();

	if (m_pAudit->Open())
		Log(CHttpServer::eLogDebug, tr("audit log: %1").arg(m_pAudit->GetPath()));
	m_pAudit->Record("server", "armed", QJsonObject{ { "endpoint", Endpoint } });

	m_pHttp->SetServerName("SandMan");
	// Generous, per the design: an agent may think for minutes between calls.
	m_pHttp->SetKeepAlive(60 * 60);
	m_pHttp->RegisterHandler(this, kEndpointPath);
	// Clients configured with a bare pipe name and no path land on "/".
	m_pHttp->RegisterHandler(this, "");

	m_Endpoint = Endpoint;
	Log(CHttpServer::eLogDebug, tr("MCP server listening on '%1%2'").arg(Endpoint).arg(kEndpointPath));
	return true;
}

void CMcpServer::Stop()
{
	m_Pending.clear();
	if (m_pIdleTimer)
		m_pIdleTimer->stop();
	if (m_pPumpTimer)
		m_pPumpTimer->stop();
	// Close the streams before the sessions they belong to.
	foreach(CMcpStream* pStream, m_Listeners.values())
		pStream->EndStream();
	m_Listeners.clear();
	if (m_pAudit) {
		m_pAudit->Record("server", "disarmed");
		m_pAudit->Close();
	}
	// Disarming is an explicit act by the user, so the sessions go with it -
	// unlike an idle timeout, which only suspends.
	if (m_pSessions)
		m_pSessions->Clear();
	McpTools::SetDocs(NULL);
	McpOperate::SetConsent(NULL);
	McpReport::SetConsent(NULL);
	if (m_pDocs) {
		delete m_pDocs;
		m_pDocs = NULL;
	}
	if (m_pHttp) {
		m_pHttp->deleteLater();
		m_pHttp = NULL;
	}
	m_Endpoint.clear();
}

bool CMcpServer::IsListening() const
{
	return m_pHttp && m_pHttp->IsListening();
}

void CMcpServer::Log(int Level, const QString& Message)
{
	emit LogMessage(Level, Message);
}

//
// The gate. Never infer box membership from an image name or path - both are
// attacker-controlled - and never from the process cache in SandMan, which lags
// the driver and would read a just-started boxed process as unboxed.
//
//
// Whether a connection has to present a code before it may look at this
// machine. The user may waive it; an administrator may refuse to let them,
// from the file the user cannot rewrite.
//
bool CMcpServer::IsPairingRequired()
{
	if (theAPI && theAPI->IsConnected()
		&& theAPI->GetGlobalSettings()->GetBool("AiAgentRequirePairing", false))
		return true;
	return theConf->GetBool("Options/McpRequirePairing", true);
}

bool CMcpServer::CheckPeer(CHttpSocket* pRequest, QString* pReason)
{
	const quint32 Pid = pRequest->GetPeerProcessId();
	if (!Pid) {
		*pReason = tr("the connecting process could not be identified");
		return false;
	}

	QString BoxName;
	switch (theAPI->QueryProcessBox(Pid, &BoxName))
	{
		case CSbieAPI::eNotBoxed:
			return true;

		case CSbieAPI::eBoxed:
			*pReason = tr("process %1 is sandboxed, in box '%2'").arg(Pid).arg(BoxName);
			return false;

		default:
			// The driver query failed. Refusing is the only safe reading.
			*pReason = tr("box membership of process %1 could not be established").arg(Pid);
			return false;
	}
}

void CMcpServer::HandleRequest(CHttpSocket* pRequest)
{
	QString Reason;
	if (!CheckPeer(pRequest, &Reason)) {
		Log(CHttpServer::eLogWarning, tr("refused an MCP connection: %1").arg(Reason));
		m_pAudit->Record("refused", Reason);
		SendHttpError(pRequest, 403, tr("Refused: %1").arg(Reason));
		return;
	}

	// DNS rebinding: an Origin that is present and not ours is refused. A
	// request with no Origin is a native client rather than a browser.
	const QString Origin = pRequest->GetHeader("Origin");
	if (!Origin.isEmpty() && !Origin.startsWith("http://localhost") && !Origin.startsWith("http://127.0.0.1")) {
		Log(CHttpServer::eLogWarning, tr("refused an MCP connection from origin '%1'").arg(Origin));
		SendHttpError(pRequest, 403, tr("Origin not allowed"));
		return;
	}

	// This revision removed the GET stream and protocol sessions, so a GET or
	// DELETE is answered 405 rather than pretending to support them.
	if (!pRequest->IsPost()) {
		SendHttpError(pRequest, 405, tr("The MCP endpoint accepts POST only"));
		return;
	}

	// HandleRequest fires on the header; the body has not arrived yet. Wait for
	// it, then dispatch.
	m_Pending.insert(pRequest);
	connect(pRequest, &QIODevice::readChannelFinished, this, &CMcpServer::OnBodyComplete, Qt::UniqueConnection);
}

void CMcpServer::ReleaseRequest(CHttpSocket* pRequest)
{
	m_Pending.remove(pRequest);
}

void CMcpServer::OnBodyComplete()
{
	CHttpSocket* pRequest = qobject_cast<CHttpSocket*>(sender());
	if (!pRequest || !m_Pending.contains(pRequest))
		return;
	m_Pending.remove(pRequest);

	Dispatch(pRequest, pRequest->readAll());
}

void CMcpServer::Dispatch(CHttpSocket* pRequest, const QByteArray& Body)
{
	McpProtocol::SRequest Request = McpProtocol::ParseRequest(Body);
	if (!Request.bValid) {
		Log(CHttpServer::eLogDebug, tr("rejected an MCP message: %1").arg(Request.Error));
		SendJson(pRequest, McpProtocol::MakeError(QJsonValue(), Request.ErrorCode, Request.Error),
			McpProtocol::HttpStatusForError(Request.ErrorCode));
		return;
	}

	// Mirrored headers must agree with the body, or an intermediary routing on
	// one and a server acting on the other disagree about what was asked.
	QString HeaderError;
	if (!McpProtocol::ValidateHeaders(Request,
			pRequest->GetHeader("MCP-Protocol-Version"),
			pRequest->GetHeader("Mcp-Method"),
			pRequest->GetHeader("Mcp-Name"), &HeaderError)) {
		SendJson(pRequest, McpProtocol::MakeError(Request.Id, McpProtocol::eHeaderMismatch, HeaderError), 400);
		return;
	}

	// A declared version we do not implement gets the shape the spec requires,
	// so the client can pick one from the list and retry. An absent version is
	// a legacy caller and is handled by the initialize path instead.
	if (!Request.ProtocolVersion.isEmpty() && !McpProtocol::IsSupportedVersion(Request.ProtocolVersion)) {
		SendJson(pRequest, McpProtocol::MakeUnsupportedVersionError(Request.Id, Request.ProtocolVersion), 400);
		return;
	}

	if (Request.bNotification) {
		// Nothing in the core protocol needs a reply here, and the spec wants an
		// empty 202 rather than a JSON-RPC response.
		pRequest->SendResponse(202, 0);
		return;
	}

	// The session is keyed on the verified peer process, not on anything in the
	// message: this revision has no session header, and a header would be worth
	// less than a pid the driver vouched for anyway.
	const quint32 PeerPid = pRequest->GetPeerProcessId();
	// Resolved on every request, from the pid, by us - never taken from the
	// client. Doing it per request rather than once is what lets GetOrCreate
	// notice that a pid now belongs to a different program.
	const QString Identity = CMcpClients::ResolveIdentity(PeerPid);
	CMcpSession* pSession = m_pSessions->GetOrCreate(PeerPid, pRequest->GetHeader("User-Agent"), Identity);

	{
		// A program the user chose to remember skips the code - and that
		// decision is audited every time it is used, because an authorisation
		// nobody re-confirms should at least be visible.
		if (!pSession->IsPaired() && CMcpClients::IsRemembered(pSession->GetIdentity())) {
			m_pSessions->PairRememberedFor(pSession);
			m_pAudit->Record("pair", "authorised from the remembered list", QJsonObject{
				{ "identity", pSession->GetIdentity() }, { "pid", (double)PeerPid } });
		}

		// Or the user has decided the code is not worth it on this machine.
		//
		// Worth being clear about what this gives up, because it is not nothing:
		// the code is not there to decide whether the agent is trustworthy, it is
		// there to decide WHICH of the many programs running as this user is the
		// one being let in. Waiving it means any unboxed program that finds the
		// pipe is treated as the agent. The pipe is still restricted to this
		// user by its descriptor, sandboxed callers are still refused by the
		// driver check, and changes are still governed by the consent mode - so
		// this opens the reading, not the writing.
		//
		// An administrator can take the choice away; the agent never can.
		if (!pSession->IsPaired() && !IsPairingRequired()) {
			m_pSessions->PairRememberedFor(pSession);
			m_pAudit->Record("pair", "authorised without a code, as configured", QJsonObject{
				{ "identity", pSession->GetIdentity() }, { "pid", (double)PeerPid } });
			if (theGUI)
				theGUI->GetMcpWindow()->AddEntry(CMcpSessionWindow::eWarning,
					tr("authorised without a code, because asking for one is turned off"),
					pSession->GetIdentity());
		}
	}

	pSession->Touch();

	int ErrorCode = 0;
	QString ErrorMessage;
	QJsonObject Result;

	if (Request.Method == "server/discover")
		Result = OnDiscover(Request);
	else if (Request.Method == "initialize")
		Result = OnInitialize(Request);
	else if (Request.Method == "ping")
		Result = QJsonObject();
	else if (Request.Method == "tools/list")
		Result = OnToolsList(Request, pSession);
	else if (Request.Method == "tools/call") {
		const QString ToolName = Request.Params.value("name").toString();
		m_pAudit->Record("tool", ToolName, QJsonObject{
			{ "identity", pSession->GetIdentity() },
			{ "client", pSession->GetClientName() },
			{ "pid", (double)pSession->GetPeerPid() },
			{ "paired", pSession->IsPaired() } });

		// Every call lands in the session window. Reads are dimmed there rather
		// than hidden: "what did it look at" is a fair question, and the answer
		// should not live only in a file nobody opens.
		const bool bMutating = McpOperate::Exists(ToolName)
			|| (McpReport::Exists(ToolName) && ToolName != "report_compose");
		CMcpSessionWindow* pWindow = theGUI->GetMcpWindow();
		pWindow->AddEntry(CMcpSessionWindow::eRead,
			tr("asked for %1").arg(DescribeCall(ToolName, Request.Params.value("arguments").toObject())));

		Result = OnToolsCall(Request, pSession, &ErrorCode, &ErrorMessage);

		// Something was actually done, so whatever the agent has written up is
		// now behind. The reminder rides on the answer to the call that made it
		// so, because that is the one moment it is certain to be read:
		// instructions handed over at connect time are read once and drift out
		// of attention, and the first run in the field proved it - the agent
		// fixed the problem, said so in its own chat window, and composed
		// nothing, so the user was never offered anything and the project heard
		// nothing about a fix that worked.
		//
		// McpOperate rather than bMutating: that set is the things that change
		// the machine. Reminding an agent to write a report on the result of
		// submitting one would be nonsense.
		if (McpOperate::Exists(ToolName) && !Result.value("isError").toBool()) {
			pSession->NoteUnreported();
			McpTools::AppendNote(Result, pSession->HasDraftReport()
				? tr("Your write-up is now out of date - this is not in it. Call report_compose "
					"again so that what the user is offered describes what you have actually done.")
				: tr("You have changed something on this machine and nothing is written up yet. "
					"Call report_compose now, with what you know so far - the symptom and what you "
					"have ruled out is already worth having - and again as you learn more. The user "
					"is offered whatever you composed last, and they may close the agent window at "
					"any moment."));
		}

		// Only what actually happened, and only once it has: a line written
		// before the call would claim a change that consent may yet refuse.
		if (bMutating && !Result.value("isError").toBool()) {
			const QString Headline = ResultHeadline(Result);
			if (!Headline.isEmpty())
				pWindow->AddEntry(CMcpSessionWindow::eAction, Headline);
		}
	}
	else if (Request.Method == "subscriptions/listen") {
		// Answers with a stream rather than an object and returns early: the
		// response goes out on the stream, not through SendJson.
		OnSubscriptionsListen(pRequest, Request, pSession);
		return;
	}
	else {
		ErrorCode = McpProtocol::eMethodNotFound;
		ErrorMessage = tr("Method not found: %1").arg(Request.Method);
	}

	if (ErrorCode) {
		SendJson(pRequest, McpProtocol::MakeError(Request.Id, ErrorCode, ErrorMessage),
			McpProtocol::HttpStatusForError(ErrorCode));
		return;
	}

	SendJson(pRequest, McpProtocol::MakeResult(Request.Id, Result));
}

QJsonObject CMcpServer::ServerInfo() const
{
	QJsonObject Info;
	Info["name"] = "sandboxie-plus";
	Info["title"] = "Sandboxie-Plus";
	Info["version"] = theGUI ? theGUI->GetVersion() : QString("unknown");
	return Info;
}

QJsonObject CMcpServer::Capabilities() const
{
	QJsonObject Tools;
	// True: pairing emits notifications/tools/list_changed, on the
	// subscriptions/listen stream the bridge keeps open. Nothing needs it any
	// more - the list no longer changes - but we do send it, so declaring it is
	// not a claim we fail to keep.
	Tools["listChanged"] = true;

	QJsonObject Caps;
	Caps["tools"] = Tools;
	return Caps;
}

QString CMcpServer::Instructions() const
{
	return tr(
		"This server exposes the local Sandboxie-Plus installation on this machine.\n"
		"\n"
		"You can read the documentation and the settings schema straight away - those tools "
		"need no authorisation, because they return what Sandboxie ships rather than anything "
		"about this machine.\n"
		"\n"
		"To look at the machine itself - which sandboxes exist, what is running, what was "
		"denied - authorise first. Call 'pair' with no arguments, and Sandboxie-Plus shows the "
		"user a six-digit code; ask the user for that code and call 'pair' again with it. Do not "
		"guess the code.\n"
		"\n"
		"Every tool is listed from the start, including the ones you cannot use yet: those "
		"refuse until you have paired, and say so when they do. Nothing appears or disappears "
		"from the list, so there is no need to refresh it - and no point asking the user to "
		"reconnect.\n"
		"\n"
		"One standing rule, and it is not optional: keep a written report going at all times, "
		"with report_compose. Compose one as soon as you have a symptom and a suspicion - "
		"before you have a cause, let alone a fix - and compose again after every single thing "
		"you change, run or rule out. It replaces the previous one, so there is no cost to "
		"doing it often and no partial state to manage.\n"
		"\n"
		"And find out whether it worked. When the user tells you their problem is gone - or that "
		"it is not - compose again with confirmed_by_user set, which records that the verdict is "
		"theirs rather than yours. If they have not said, ask: 'is it working now?' is worth one "
		"turn, and it is the single most valuable line in the report. Until that flag is set "
		"Sandboxie-Plus assumes the run is unfinished: it shows the user buttons to end it "
		"themselves, and asks them the question itself before anything is sent.\n"
		"\n"
		"Treat it as part of the work rather than as paperwork at the end. Sandboxie-Plus keeps "
		"your latest write-up and offers it to the user when they close the agent window, which "
		"they may do at any moment - and if you have composed nothing, nothing is offered and "
		"everything you worked out is lost to the project. Finishing a run having fixed "
		"something and written nothing up is a failed run, however pleased the user is.\n"
		"\n"
		"Then read before acting: sandboxie_version first, because settings and fixes differ "
		"between builds, then box_list, then box_get_config for the box in question. Note that "
		"box_get_config shows only what is set on that box - values inherited from templates "
		"and global defaults are not listed, so an absent setting is not necessarily inactive.\n"
		"\n"
		"Two separate sources of truth about settings, and they disagree about most of the "
		"surface. config_schema is the authority on whether a setting exists, what its syntax "
		"is, and which version added or removed it. The bundled documentation (docs_search, "
		"docs_get, docs_for_setting) explains what a setting means, but covers only about two "
		"in five of them. If the documentation says nothing about a setting, that is not "
		"evidence the setting does not exist - check config_schema before telling the user it "
		"is not real. Conversely, never invent a setting name: the driver ignores an unknown "
		"name silently instead of reporting an error, so a typo looks exactly like a setting "
		"that had no effect.\n"
		"\n"
		"When something does not work in a sandbox, the order that pays off is: "
#ifdef SBIE_HAS_DIAG_EVENTS
		"box_processes to see whether it is even running, box_diagnostics for what "
		"Sandboxie itself objected to, then trace_query filtered to that box for the "
		"accesses that were denied. If tracing is off, trace_query says so - ask the user "
#else
		"box_processes to see whether it is even running, then log_query for what "
		"Sandboxie itself reported, then trace_query filtered to that box for the "
		"accesses that were denied. If tracing is off, trace_query says so - ask the user "
#endif
		"to enable it and reproduce the problem rather than reasoning from an empty "
		"trace.\n"
		"\n"
		"Boxes usually have an alias, and that is what the user calls them. Refer to a box "
		"the way box_list shows in displayName; either the name or the alias is accepted "
		"as input.\n"
		"\n"
		"Paths and account names in diagnostic output are replaced with tokens such as "
		"%User% and %Computer%. That is deliberate - do not ask the user to undo it, and "
		"do not treat a token as a literal path.\n"
		"\n"
		"Some tools change things, and every one of them shows the user exactly what "
		"will change and waits for them to approve it. You cannot change anything "
		"silently, so do not try to minimise what you are asking for - a clear "
		"explanation gets approved and a vague one does not.\n"
		"\n"
		"For a fix you are unsure about, test it first: shadow_begin clones the "
		"configuration to a disposable copy that shares the real file set, shadow_set "
		"changes the copy, and then shadow_commit copies the differences back - one "
		"confirmation for the whole fix - or shadow_discard throws it away. A fix you "
		"have actually tried is worth far more than one you believe in. Note that only "
		"configuration is disposable: files written while testing land in the real "
		"sandbox, and a discard does not undo them.\n"
		"\n"
		"For a change you are confident about, or an everyday request such as forcing a "
		"program into a sandbox, use box_apply_changes directly. Run config_validate "
		"first either way.\n"
		"\n"
		"If you are about to change files rather than settings, offer snapshot_create "
		"first - it is near-instant and makes the change undoable - but it only works "
		"while nothing is running in the sandbox, so it has to come before you start "
		"anything.\n"
		"\n"
		"To see whether a fix actually worked, box_run starts a program inside a "
		"sandbox. The user is shown the command line exactly as you write it and has "
		"to agree before anything runs, so write what you mean. Note that it runs "
		"against the sandbox's current settings: a change staged with shadow_set is "
		"not in effect, so commit it first if the run is meant to test it.\n"
		"\n"
		"Nothing here deletes a sandbox or its contents, deliberately.\n"
		"\n"
		"Keep a running write-up as you go, with report_compose, rather than saving it "
		"for the end. Compose as soon as you know something worth reporting - the "
		"symptom and what you have ruled out is already worth having - and compose "
		"again whenever you learn more. Each call replaces the last.\n"
		"\n"
		"The reason is not bookkeeping. Sandboxie-Plus keeps your latest write-up and "
		"offers it to the user when they close the agent window, which they may do at "
		"any moment - your console closing does not wait for you to finish a thought. "
		"If you have composed nothing, nothing is offered and the whole run is lost to "
		"the project. A rough write-up that gets sent beats a polished one that never "
		"existed.\n"
		"\n"
		"Compose on a success as readily as on a failure. A failure report tells the "
		"project what is still broken; a success report tells it what the fix was, and "
		"that is the one it almost never receives.\n"
		"\n"
		"You do not send it - that is the user's, and you have no tool for it. When you "
		"are done, tell them to press Finish, the green tick at the bottom of the "
		"Sandboxie-Plus agent window: it opens your report for them to read, correct and "
		"send to the project's own tracker, which needs no account of any kind. Say once "
		"that sending it is worth doing and why, then leave it with them - they may say "
		"no, and that is a normal answer.\n"
		"\n"
		"Fill in what you ruled out, not only what you concluded. A cause is made "
		"identifiable by what it is not, and that is the field that turns one report "
		"into something that recognises the same fault on somebody else's machine.");
}

QJsonObject CMcpServer::OnDiscover(const McpProtocol::SRequest& Request)
{
	Q_UNUSED(Request);

	QJsonArray Versions;
	foreach(const QString& Version, McpProtocol::SupportedVersions())
		Versions.append(Version);

	QJsonObject Result;
	Result["protocolVersions"] = Versions;
	Result["serverInfo"] = ServerInfo();
	Result["capabilities"] = Capabilities();
	Result["instructions"] = Instructions();
	return Result;
}

QJsonObject CMcpServer::OnInitialize(const McpProtocol::SRequest& Request)
{
	// The legacy path. Echo the version of the client when we support it,
	// otherwise answer with our newest legacy revision - a legacy client has no
	// fall-forward mechanism, so handing it a modern-only version would leave it
	// with nothing to do.
	QString Version = Request.Params.value("protocolVersion").toString();
	if (!McpProtocol::IsSupportedVersion(Version) || McpProtocol::IsModernVersion(Version))
		Version = McpProtocol::kVersionLegacyNewest;

	QJsonObject Result;
	Result["protocolVersion"] = Version;
	Result["serverInfo"] = ServerInfo();
	Result["capabilities"] = Capabilities();
	Result["instructions"] = Instructions();
	return Result;
}

QJsonObject CMcpServer::PairToolDescription()
{
	QJsonObject Code;
	Code["type"] = "string";
	Code["description"] = QObject::tr(
		"The six-digit code shown in Sandboxie-Plus. Omit it on the first call and "
		"Sandboxie-Plus will display one; then ask the user to read it to you.");

	QJsonObject Properties;
	Properties["code"] = Code;

	QJsonObject Schema;
	Schema["type"] = "object";
	Schema["properties"] = Properties;
	// Deliberately not required: calling with no code is how a code is
	// requested, which is the first half of the exchange.

	QJsonObject Tool;
	Tool["name"] = "pair";
	Tool["title"] = QObject::tr("Connect to Sandboxie-Plus");
	Tool["description"] = QObject::tr(
		"Authorises this connection to look at this machine. Call with no arguments and "
		"Sandboxie-Plus shows a six-digit code on the user's screen; ask the user for it and "
		"call again with code set.\n\n"
		"You do not need this to read the documentation or the settings schema - those work "
		"now. You do need it before anything that reads the user's sandboxes, and before "
		"anything that changes them. Do not guess the code: a handful of wrong attempts "
		"invalidates it and a new one has to be requested.");
	Tool["inputSchema"] = Schema;
	return Tool;
}

QJsonObject CMcpServer::OnToolsList(const McpProtocol::SRequest& Request, CMcpSession* pSession)
{
	Q_UNUSED(Request);
	Q_UNUSED(pSession);

	// The same list whether this session is paired or not.
	//
	// Two earlier designs stood here. The first had the user choose a tier up
	// front, which is a question asked before anyone knows what the agent will
	// need, and is therefore answered by picking the most permissive option so
	// that things work. The second showed one tool before pairing and
	// everything after - the "bounded pre-auth state and tools/list_changed
	// reveal".
	//
	// Neither was where the security lived. It lives on the call: OnToolsCall
	// refuses anything but the documentation until a code displayed on the
	// user's screen has been read back, every change needs a confirmation of
	// its own, and the listener does not exist until a human arms it. Three
	// levels of access, none chosen in advance, each asked for at the moment it
	// means something. Hiding the names on top of that decided only when the
	// agent learned the tools existed.
	//
	// And it decided it badly. A tool list changes only when the client
	// refetches it, and a client refetches only on
	// notifications/tools/list_changed - the message clients most often get
	// wrong, with a trail of regressions in the reference client alone, and one
	// no agent can ask for on its own behalf. Miss it and the session is over
	// before it starts: paired, with the tools still invisible, and nothing the
	// user or the agent can do but reconnect and hope. That cost a full test
	// run, with the agent correctly reporting it had no way forward.
	//
	// So nothing depends on a notification arriving. An unpaired caller can
	// read the names and schemas, which is published API surface - the same
	// reasoning that makes the documentation itself public. What it cannot do
	// is call one: it gets a result telling it to pair, which teaches the next
	// step better than an absence it would have to infer.
	QJsonArray Tools;
	// Listed even once paired, so the list really is constant. Calling it
	// again then is harmless - it says so and stops.
	Tools.append(PairToolDescription());
	foreach(const QJsonValue& Value, McpTools::Describe())
		Tools.append(Value);
	foreach(const QJsonValue& Value, McpOperate::Describe())
		Tools.append(Value);
	foreach(const QJsonValue& Value, McpReport::Describe())
		Tools.append(Value);

	QJsonObject Result;
	Result["tools"] = Tools;
	return Result;
}

QJsonObject CMcpServer::OnToolsCall(const McpProtocol::SRequest& Request, CMcpSession* pSession, int* pErrorCode, QString* pErrorMessage)
{
	const QString Name = Request.Params.value("name").toString();
	const QJsonObject Arguments = Request.Params.value("arguments").toObject();

	if (Name == "pair")
		return CallPair(Arguments, pSession);

	if (!pSession->IsPaired() && !McpTools::IsPublic(Name)) {
		// A tool result rather than a protocol error: the agent should read
		// this, pair, and carry on, which it cannot do if the turn is lost.
		return McpTools::MakeErrorResult(tr(
			"That needs authorisation: it reads this machine rather than the documentation. "
			"Call the pair tool, read the code Sandboxie-Plus displays to the user, and submit "
			"it. The documentation and schema tools work without it."));
	}

	// The mutating tools are routed separately because they need the session:
	// shadows belong to one, and a change has to be attributable to whoever
	// asked for it.
	if (McpOperate::Exists(Name))
		return McpOperate::Call(Name, Arguments, pSession);

	// Reporting needs the session too: the composed draft lives there, so
	// report_submit sends what was shown rather than what is passed in.
	if (McpReport::Exists(Name))
		return McpReport::Call(Name, Arguments, pSession);

	if (!McpTools::Exists(Name)) {
		*pErrorCode = McpProtocol::eInvalidParams;
		*pErrorMessage = tr("Unknown tool: %1").arg(Name);
		return QJsonObject();
	}

	return McpTools::Call(Name, Arguments);
}

QJsonObject CMcpServer::CallPair(const QJsonObject& Arguments, CMcpSession* pSession)
{
	if (pSession->IsPaired())
		return McpTools::MakeTextResult(tr("This connection is already authorised."));

	const QString Code = Arguments.value("code").toString();

	if (Code.trimmed().isEmpty()) {
		// First half: mint and display. The code goes to the screen, never over
		// the wire - an agent that could read it would not be proving anything.
		m_pSessions->MintCodeFor(pSession);
		return McpTools::MakeTextResult(tr(
			"Sandboxie-Plus is now showing a six-digit code. Ask the user to read it to you, "
			"then call pair again with that code. It is valid for five minutes."));
	}

	switch (m_pSessions->SubmitCodeFor(pSession, Code))
	{
		case CMcpSession::ePairOk:
			m_pAudit->Record("pair", "authorised by code", QJsonObject{
				{ "identity", pSession->GetIdentity() }, { "pid", (double)pSession->GetPeerPid() } });
			Log(CHttpServer::eLogWarning, tr("an agent was authorised: %1 (process %2)")
				.arg(pSession->GetClientName().isEmpty() ? tr("unidentified client") : pSession->GetClientName())
				.arg(pSession->GetPeerPid()));
			// No instruction to refresh anything: the tools were listed all
			// along, and refreshing a list is the client runtime's job rather
			// than something a model can call. An earlier version of this said
			// "call tools/list again", and produced an agent that dutifully
			// told the user to reconnect.
			return McpTools::MakeTextResult(tr(
				"Authorised. Every tool you can see will now answer, including the ones that "
				"were refusing. Changes still need the user to confirm them one at a time."));

		case CMcpSession::ePairNoCode:
			return McpTools::MakeErrorResult(tr(
				"No code has been requested. Call pair with no arguments first."));

		case CMcpSession::ePairExpired:
			return McpTools::MakeErrorResult(tr(
				"That code has expired. Call pair with no arguments to request a new one."));

		case CMcpSession::ePairTooManyAttempts:
			return McpTools::MakeErrorResult(tr(
				"Too many incorrect attempts; the code has been invalidated. Call pair with "
				"no arguments to request a new one, and ask the user to read it again."));

		default:
			return McpTools::MakeErrorResult(tr(
				"That code is not correct. Ask the user to read it again from the Sandboxie-Plus window."));
	}
}

CMcpStream* CMcpServer::BeginSse(CHttpSocket* pRequest)
{
	CMcpStream* pStream = new CMcpStream(pRequest);

	pRequest->SetHeader("Content-Type", "text/event-stream");
	pRequest->SetHeader("Cache-Control", "no-cache");
	// Tells a reverse proxy not to buffer the response; without it events can
	// sit in an intermediary until enough accumulate.
	pRequest->SetHeader("X-Accel-Buffering", "no");

	// A sequential response buffer is what selects chunked encoding, which is
	// what makes an open-ended response possible at all.
	pRequest->SetResponseBuffer(pStream);
	pRequest->SendResponse(200);
	return pStream;
}

void CMcpServer::NotifySession(const CMcpSession* pSession, const QString& Method, const QJsonObject& Params)
{
	QJsonObject Message;
	Message["jsonrpc"] = "2.0";
	Message["method"] = Method;
	if (!Params.isEmpty())
		Message["params"] = Params;
	const QByteArray Json = QJsonDocument(Message).toJson(QJsonDocument::Compact);

	foreach(CMcpStream* pStream, m_Listeners.values(pSession)) {
		if (!pStream->IsEnded())
			pStream->SendMessage(Json);
	}
	OnPump();
}

void CMcpServer::OnConsentDecided(const QString& What, bool bAllowed, const QString& Detail)
{
	// Both outcomes recorded. The refusals are the ones worth being able to
	// point at afterwards.
	m_pAudit->Record("consent", bAllowed ? "allowed" : "refused", QJsonObject{
		{ "what", What }, { "detail", Detail } });
	Log(CHttpServer::eLogWarning, bAllowed
		? tr("the user allowed: %1").arg(What)
		: tr("the user refused: %1 (%2)").arg(What).arg(Detail));
}

void CMcpServer::OnSessionPaired(const QString& ClientName)
{
	Q_UNUSED(ClientName);
	// The names did not change - the list is constant now - but what they will
	// do did: every tool that was refusing is now callable. A client that
	// refetches on this loses nothing, and for a client that surfaces tool
	// state to its user this is the only announcement that the session became
	// authorised.
	//
	// Nothing depends on it arriving, which is the point. It used to be the
	// only way the tools could ever appear.
	foreach(const CMcpSession* pSession, m_Listeners.uniqueKeys()) {
		if (pSession->IsPaired())
			NotifySession(pSession, "notifications/tools/list_changed");
	}
}

void CMcpServer::OnSubscriptionsListen(CHttpSocket* pRequest, const McpProtocol::SRequest& Request, CMcpSession* pSession)
{
	CMcpStream* pStream = BeginSse(pRequest);
	m_Listeners.insert(pSession, pStream);
	connect(pStream, &QObject::destroyed, this, &CMcpServer::OnStreamDestroyed);

	// Acknowledge on the stream itself, so the client knows it is live.
	QJsonObject Ack;
	Ack["jsonrpc"] = "2.0";
	Ack["method"] = "notifications/subscriptions/acknowledged";
	pStream->SendMessage(QJsonDocument(Ack).toJson(QJsonDocument::Compact));

	// The response to the request that opened the stream. Unlike a normal
	// request the stream does not close after it - that is the point.
	pStream->SendMessage(McpProtocol::MakeResult(Request.Id, QJsonObject()));

	m_pAudit->Record("stream", "listen stream opened", QJsonObject{
		{ "client", pSession->GetClientName() }, { "pid", (double)pSession->GetPeerPid() } });
	OnPump();
}

void CMcpServer::SendJson(CHttpSocket* pRequest, const QByteArray& Json, int Code)
{
	// Body first: SendResponse prepends the header and derives Content-Length
	// from what is already buffered.
	pRequest->SetHeader("Content-Type", "application/json");
	pRequest->write(Json);
	pRequest->SendResponse(Code);
}

void CMcpServer::SendHttpError(CHttpSocket* pRequest, int Code, const QString& Message)
{
	pRequest->RespondWithError(Code, Message);
}
