#pragma once
#include <QJsonObject>
#include <QObject>
#include <QDateTime>
#include <QMap>
#include <QString>
#include "../Boxes/BoxShadows.h"

//
// CMcpSession - what a connected agent is allowed to do, and for how long.
//
// There is no protocol session to hang this on. The current MCP revision
// removed Mcp-Session-Id, so a session here is keyed on something better than a
// header the client could forge: the **verified peer process**, resolved from
// the transport and checked against the driver. The pipe connection supplies
// continuity, the peer pid supplies identity.
//
// Pairing runs in the direction that actually authenticates something. SandMan
// mints the code and shows it; the user reads it out of SandMan and gives it to
// the agent; the agent submits it. A code the agent generated would prove only
// that the agent can generate codes.
//
// Until a session is paired it sees exactly one tool. That is what makes the
// code worth submitting, and it is also the only mechanism the transport leaves
// available - there is no handshake to carry a token and no custom header a
// stock client would send.
//
class CMcpSession
{
public:
	enum EState
	{
		eUnpaired = 0,	// connected; only the pair tool is visible
		ePaired,		// the full tool surface
		eSuspended,		// timed out; resumable, but the user must re-authenticate
	};

	enum EPairResult
	{
		ePairOk = 0,
		ePairNoCode,			// nothing has been minted to compare against
		ePairWrongCode,
		ePairExpired,
		ePairTooManyAttempts,
	};

	CMcpSession(quint32 PeerPid, const QString& ClientName);

	// The shadows this session has open, created on first use. Owned here
	// because the destructor is what discards them: an abandoned session, a
	// crash, or a disarm all leave the user's boxes untouched without anyone
	// having to remember to clean up. Suspension deliberately does not destroy
	// the session, so an hour-long pause does not throw away the work.
	CBoxShadows&	Shadows()		{ return m_Shadows; }
	bool			HasShadows() const	{ return m_Shadows.HasShadows(); }

	// The composed report, held here between report_compose and
	// report_submit. It lives on the session rather than being passed back
	// in so that what gets submitted is what the user was shown - an agent
	// that could hand over a different body at send time would make the
	// review meaningless.
	void			SetDraftReport(const QString& Text, const QJsonObject& Envelope,
						bool bAttachIni, bool bAttachTrace)
						{ m_DraftReport = Text; m_DraftEnvelope = Envelope;
						  m_bDraftAttachIni = bAttachIni; m_bDraftAttachTrace = bAttachTrace;
						  m_Unreported = 0; }
	bool			DraftAttachesIni() const		{ return m_bDraftAttachIni; }
	bool			DraftAttachesTrace() const		{ return m_bDraftAttachTrace; }
	QString			GetDraftReport() const		{ return m_DraftReport; }
	QJsonObject		GetDraftEnvelope() const	{ return m_DraftEnvelope; }
	bool			HasDraftReport() const		{ return !m_DraftReport.isEmpty(); }

	// Material things done since the report was last written. The agent is
	// reminded on the result of the call that raised it, because a session
	// that fixes something and writes nothing up is a session the project
	// never hears about - which is exactly what the first real run did.
	void			NoteUnreported()			{ m_Unreported++; }
	int				UnreportedActions() const	{ return m_Unreported; }

	// Whether the user has said in so many words that it worked or did not.
	// Distinct from the outcome the agent composed: an agent is confident
	// about fixes that did not fix anything, and the person sitting in front
	// of the machine is the only authority on whether their problem is gone.
	void			SetOutcomeConfirmed(bool bOn)	{ m_bOutcomeConfirmed = bOn; }
	bool			IsOutcomeConfirmed() const	{ return m_bOutcomeConfirmed; }
	void			ClearDraftReport()			{ m_DraftReport.clear(); m_DraftEnvelope = QJsonObject();
											  m_bDraftAttachIni = false; m_bDraftAttachTrace = false; }

	// The full path of the program that is actually driving this - resolved by
	// us from the pid, never taken from the client. See
	// CMcpClients::ResolveIdentity for why the distinction matters.
	void			SetIdentity(const QString& Path)	{ m_Identity = Path; }
	QString			GetIdentity() const					{ return m_Identity; }

	// Pairs without a code because the user previously chose to remember this
	// program. Audited separately from a code pairing, because it is a weaker
	// claim and should be visible as one.
	void			PairRemembered();

	quint32			GetPeerPid() const		{ return m_PeerPid; }
	QString			GetClientName() const	{ return m_ClientName; }
	EState			GetState() const		{ return m_State; }
	bool			IsPaired() const		{ return m_State == ePaired; }

	QDateTime		GetStartedAt() const	{ return m_StartedAt; }
	QDateTime		GetLastSeenAt() const	{ return m_LastSeenAt; }

	// Mints a fresh six-digit code and starts its clock. Replaces any code
	// still pending, so a second request cannot be used to keep an old code
	// alive past its expiry.
	QString			MintCode();
	bool			HasPendingCode() const;
	QString			GetPendingCode() const	{ return m_Code; }

	EPairResult		SubmitCode(const QString& Code);

	// Records activity. A session that is being used does not time out.
	void			Touch();

	// Moves an idle session to eSuspended. Returns true when it changed state,
	// so the caller can log it once rather than every tick.
	bool			ExpireIfIdle();

	// Back to eUnpaired so the next pair attempt can revive it. Session state
	// that outlives the suspension - shadows, snapshots - is held by the owner,
	// not here, which is why suspension does not destroy anything.
	void			Resume();

	int				GetSecondsIdle() const;
	int				GetSecondsRemaining() const;

	// Generous on purpose: an agent may think for minutes between calls, and a
	// session that dies mid-investigation costs the user the whole context.
	static int		GetIdleTimeoutSeconds();

private:
	void			ClearCode();

	quint32			m_PeerPid;
	QString			m_ClientName;
	QString			m_Identity;
	CBoxShadows		m_Shadows;
	QString			m_DraftReport;
	QJsonObject		m_DraftEnvelope;
	bool			m_bDraftAttachIni = false;
	bool			m_bDraftAttachTrace = false;
	int				m_Unreported = 0;
	bool			m_bOutcomeConfirmed = false;
	EState			m_State;

	QDateTime		m_StartedAt;
	QDateTime		m_LastSeenAt;

	QString			m_Code;
	QDateTime		m_CodeMintedAt;
	int				m_CodeAttempts;
};


//
// CMcpSessionManager - one session per peer process.
//
// Emits rather than prompts: minting a code has to put something on screen, but
// that is the GUI's business and this has no business knowing how.
//
//
// CMcpClients - the programs the user chose not to be asked about again.
//
// Identity is the program's full path, resolved from the connecting pid by us.
// Be clear about what that is worth: it means anything able to run from that
// path inherits the authorisation, so it is a convenience for a trusted
// desktop, not a security boundary. It is opt-in per program, revocable, and
// every automatic pairing is written to the audit log so it is never invisible.
//
class CMcpClients
{
public:
	// The program behind a connection. The peer is normally our own stdio
	// bridge, which is one process per client launch, so the useful identity is
	// its parent - but the parent is only trusted when the peer is verifiably
	// our bridge, compared by full path against our own install directory. A
	// process that merely calls itself SbieMcp.exe does not get to nominate
	// what it is a child of.
	static QString	ResolveIdentity(quint32 PeerPid);

	static bool		IsRemembered(const QString& Identity);
	static void		Remember(const QString& Identity);
	static void		Forget(const QString& Identity);
	static QStringList	GetRemembered();
	static void		ForgetAll();
};


class CMcpSessionManager : public QObject
{
	Q_OBJECT

public:
	CMcpSessionManager(QObject* parent = NULL);
	~CMcpSessionManager();

	// The session for a peer, created unpaired if it is new. Never NULL.
	// Identity is supplied rather than looked up here so the comparison happens
	// on every request: a session is keyed on a process id, and a process id is
	// reused. Without the check, a new process that happened to get the pid of
	// a departed paired client would inherit its authorisation.
	CMcpSession*	GetOrCreate(quint32 PeerPid, const QString& ClientName, const QString& Identity);
	CMcpSession*	Find(quint32 PeerPid) const;

	void			Remove(quint32 PeerPid);
	void			Clear();

	int				GetCount() const	{ return m_Sessions.count(); }
	QList<CMcpSession*>	GetSessions() const	{ return m_Sessions.values(); }

	// Suspends whatever has gone idle. Call from a timer.
	void			ExpireIdle();

	// Releases sessions whose client process is gone. This is what discards
	// their shadows: suspension deliberately keeps them, but there is nothing
	// left to resume when the process has exited, and a shadow kept for a
	// client that will never come back is just a leak.
	int				DropDeparted();

signals:
	// A code was minted and must be shown to the user, who relays it to the
	// agent. The agent never sees it over the wire.
	void			PairingCodeMinted(const QString& ClientName, const QString& Identity, const QString& Code, int ValidSeconds);
	void			PairingSucceeded(const QString& ClientName);
	void			PairedFromMemory(const QString& ClientName, const QString& Identity);
	void			PairingFailed(const QString& ClientName, const QString& Reason);
	void			SessionSuspended(const QString& ClientName);
	void			SessionDeparted(const QString& ClientName, bool bHadShadows);
	// A client we had not seen before. The session window opens on this, so
	// the user has somewhere to watch from before the agent asks for anything.
	void			SessionStarted(const QString& ClientName, const QString& Identity);

public:
	// Routed through here so the GUI hears about it; the server calls these
	// rather than touching the session directly.
	QString			MintCodeFor(CMcpSession* pSession);

	// Pairs a session because its program is remembered, and reports it.
	void			PairRememberedFor(CMcpSession* pSession);
	CMcpSession::EPairResult	SubmitCodeFor(CMcpSession* pSession, const QString& Code);

private:
	QMap<quint32, CMcpSession*>	m_Sessions;
};
