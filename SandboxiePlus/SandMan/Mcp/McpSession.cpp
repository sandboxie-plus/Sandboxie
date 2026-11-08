#include "stdafx.h"
#include "McpSession.h"
#include <QFileInfo>
#include <QRandomGenerator>
#include <QRegularExpression>
#include "../SandMan.h"
#include <QDir>
#include <windows.h>
#include <tlhelp32.h>

static const char* const kRememberedKey = "Options/McpRememberedClients";

// A pairing code is read off the screen and typed into a chat window, so it has
// to live long enough for a human to do that without being a standing invitation.
static const int kCodeValidSeconds = 5 * 60;
// Five guesses against a six-digit code is a one-in-two-hundred-thousand chance
// of a blind hit, and the code is replaced on exhaustion anyway.
static const int kMaxCodeAttempts = 5;

int CMcpSession::GetIdleTimeoutSeconds()
{
	return 60 * 60;
}

CMcpSession::CMcpSession(quint32 PeerPid, const QString& ClientName)
{
	m_PeerPid = PeerPid;
	m_ClientName = ClientName;
	m_State = eUnpaired;
	m_StartedAt = QDateTime::currentDateTime();
	m_LastSeenAt = m_StartedAt;
	m_CodeAttempts = 0;
}

void CMcpSession::ClearCode()
{
	m_Code.clear();
	m_CodeMintedAt = QDateTime();
	m_CodeAttempts = 0;
}

QString CMcpSession::MintCode()
{
	// From the system generator, not the default one: this is a credential for
	// the length of its life, however short that is.
	const quint32 Value = QRandomGenerator::system()->bounded(0, 1000000);
	m_Code = QString("%1").arg(Value, 6, 10, QChar('0'));
	m_CodeMintedAt = QDateTime::currentDateTime();
	m_CodeAttempts = 0;
	return m_Code;
}

bool CMcpSession::HasPendingCode() const
{
	if (m_Code.isEmpty() || !m_CodeMintedAt.isValid())
		return false;
	return m_CodeMintedAt.secsTo(QDateTime::currentDateTime()) <= kCodeValidSeconds;
}

CMcpSession::EPairResult CMcpSession::SubmitCode(const QString& Code)
{
	if (m_Code.isEmpty() || !m_CodeMintedAt.isValid())
		return ePairNoCode;

	if (m_CodeMintedAt.secsTo(QDateTime::currentDateTime()) > kCodeValidSeconds) {
		ClearCode();
		return ePairExpired;
	}

	if (m_CodeAttempts >= kMaxCodeAttempts) {
		// Burn the code rather than leaving a dead one pending: the next
		// attempt then says "ask for a new code" instead of "wrong again".
		ClearCode();
		return ePairTooManyAttempts;
	}

	m_CodeAttempts++;

	// Compared as digits, so whitespace or a stray separator the user included
	// when reading it out does not count as a wrong guess.
	QString Given = Code;
	Given.remove(QRegularExpression("[^0-9]"));
	if (Given != m_Code)
		return ePairWrongCode;

	ClearCode();
	m_State = ePaired;
	Touch();
	return ePairOk;
}

void CMcpSession::Touch()
{
	m_LastSeenAt = QDateTime::currentDateTime();
}

int CMcpSession::GetSecondsIdle() const
{
	return (int)m_LastSeenAt.secsTo(QDateTime::currentDateTime());
}

int CMcpSession::GetSecondsRemaining() const
{
	const int Left = GetIdleTimeoutSeconds() - GetSecondsIdle();
	return Left > 0 ? Left : 0;
}

bool CMcpSession::ExpireIfIdle()
{
	if (m_State != ePaired)
		return false;	// an unpaired session has nothing to lose
	if (GetSecondsIdle() < GetIdleTimeoutSeconds())
		return false;
	m_State = eSuspended;
	ClearCode();
	return true;
}

void CMcpSession::PairRemembered()
{
	m_State = ePaired;
	ClearCode();
	Touch();
}

void CMcpSession::Resume()
{
	// Deliberately back to eUnpaired and not straight to ePaired: resuming is
	// cheap for the agent but still requires the user to authenticate it, which
	// is the whole point of letting a session outlive its timeout.
	m_State = eUnpaired;
	ClearCode();
	Touch();
}


CMcpSessionManager::CMcpSessionManager(QObject* parent)
 : QObject(parent)
{
}

CMcpSessionManager::~CMcpSessionManager()
{
	Clear();
}

void CMcpSessionManager::Clear()
{
	foreach(CMcpSession* pSession, m_Sessions)
		delete pSession;
	m_Sessions.clear();
}

CMcpSession* CMcpSessionManager::Find(quint32 PeerPid) const
{
	return m_Sessions.value(PeerPid, NULL);
}

CMcpSession* CMcpSessionManager::GetOrCreate(quint32 PeerPid, const QString& ClientName, const QString& Identity)
{
	bool bNew = true;
	CMcpSession* pSession = m_Sessions.value(PeerPid, NULL);
	if (pSession) {
		// The pid matches, but is it the same program? A process id is reused,
		// and without this a new process landing on the pid of a departed
		// paired client would inherit its authorisation. A mismatch is treated
		// as a different client, not as the same one renamed.
		const bool bSameProgram = pSession->GetIdentity().isEmpty() || Identity.isEmpty()
			|| pSession->GetIdentity().compare(Identity, Qt::CaseInsensitive) == 0;
		const bool bBetterName = !ClientName.isEmpty() && pSession->GetClientName().isEmpty();

		if (bSameProgram && !bBetterName) {
			// A client that reconnects under the same pid keeps its session,
			// which is what makes a suspended one resumable.
			return pSession;
		}

		// Learning the name of a client we already knew is not a new session, and
		// announcing it as one would say the same client connected twice.
		bNew = !bSameProgram;

		// Destroying it discards its shadows, which is correct: they belonged to
		// whoever is no longer there.
		delete pSession;
		m_Sessions.remove(PeerPid);
	}

	// A client that sends no User-Agent still has a name: we resolved its
	// image path a moment ago in order to decide whether to trust it. Falling
	// back to the file name beats telling the user "an unidentified program"
	// about a program we identified well enough to let in.
	QString Name = ClientName;
	if (Name.isEmpty() && !Identity.isEmpty())
		Name = QFileInfo(Identity).fileName();

	pSession = new CMcpSession(PeerPid, Name);
	if (!Identity.isEmpty())
		pSession->SetIdentity(Identity);
	m_Sessions.insert(PeerPid, pSession);
	if (bNew)
		emit SessionStarted(ClientName, Identity);
	return pSession;
}

int CMcpSessionManager::DropDeparted()
{
	int Dropped = 0;
	foreach(quint32 Pid, m_Sessions.keys()) {
		// An identity that cannot be resolved means the process is gone:
		// ResolveIdentity opens the process to read its image path.
		if (!CMcpClients::ResolveIdentity(Pid).isEmpty())
			continue;

		CMcpSession* pSession = m_Sessions.take(Pid);
		const QString Name = pSession->GetClientName();
		const bool bHadShadows = pSession->HasShadows();
		delete pSession;	// discards its shadows
		Dropped++;
		emit SessionDeparted(Name, bHadShadows);
	}
	return Dropped;
}

void CMcpSessionManager::Remove(quint32 PeerPid)
{
	if (CMcpSession* pSession = m_Sessions.take(PeerPid))
		delete pSession;
}

void CMcpSessionManager::ExpireIdle()
{
	foreach(CMcpSession* pSession, m_Sessions) {
		if (pSession->ExpireIfIdle())
			emit SessionSuspended(pSession->GetClientName());
	}
}

QString CMcpSessionManager::MintCodeFor(CMcpSession* pSession)
{
	const QString Code = pSession->MintCode();
	emit PairingCodeMinted(pSession->GetClientName(), pSession->GetIdentity(), Code, kCodeValidSeconds);
	return Code;
}

CMcpSession::EPairResult CMcpSessionManager::SubmitCodeFor(CMcpSession* pSession, const QString& Code)
{
	const CMcpSession::EPairResult Result = pSession->SubmitCode(Code);
	if (Result == CMcpSession::ePairOk)
		emit PairingSucceeded(pSession->GetClientName());
	else {
		QString Reason;
		switch (Result) {
			case CMcpSession::ePairNoCode:			Reason = tr("no code had been requested"); break;
			case CMcpSession::ePairWrongCode:		Reason = tr("wrong code"); break;
			case CMcpSession::ePairExpired:			Reason = tr("the code had expired"); break;
			case CMcpSession::ePairTooManyAttempts:	Reason = tr("too many attempts"); break;
			default:								Reason = tr("refused"); break;
		}
		emit PairingFailed(pSession->GetClientName(), Reason);
	}
	return Result;
}


//
// CMcpClients
//

static QString ImagePathOf(DWORD Pid)
{
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, Pid);
	if (!hProcess)
		return QString();

	WCHAR Path[MAX_PATH * 2] = { 0 };
	DWORD Size = ARRAYSIZE(Path);
	QString Result;
	if (QueryFullProcessImageNameW(hProcess, 0, Path, &Size))
		Result = QDir::cleanPath(QString::fromWCharArray(Path, Size));
	CloseHandle(hProcess);
	return Result;
}

static DWORD ParentPidOf(DWORD Pid)
{
	HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (Snapshot == INVALID_HANDLE_VALUE)
		return 0;

	DWORD Parent = 0;
	PROCESSENTRY32W Entry;
	Entry.dwSize = sizeof(Entry);
	if (Process32FirstW(Snapshot, &Entry)) {
		do {
			if (Entry.th32ProcessID == Pid) { Parent = Entry.th32ParentProcessID; break; }
		} while (Process32NextW(Snapshot, &Entry));
	}
	CloseHandle(Snapshot);
	return Parent;
}

QString CMcpClients::ResolveIdentity(quint32 PeerPid)
{
	if (!PeerPid)
		return QString();

	const QString PeerPath = ImagePathOf((DWORD)PeerPid);
	if (PeerPath.isEmpty())
		return QString();

	// Only our own bridge earns the parent indirection, and only by full path.
	// Trusting the image *name* would let anything called SbieMcp.exe choose
	// which program it claims to be acting for.
	const QString OurShim = QDir::cleanPath(QApplication::applicationDirPath() + "/SbieMcp.exe");
	if (PeerPath.compare(OurShim, Qt::CaseInsensitive) != 0)
		return PeerPath;

	const DWORD ParentPid = ParentPidOf((DWORD)PeerPid);
	if (!ParentPid)
		return PeerPath;
	const QString ParentPath = ImagePathOf(ParentPid);
	return ParentPath.isEmpty() ? PeerPath : ParentPath;
}

QStringList CMcpClients::GetRemembered()
{
	return theConf->GetStringList(kRememberedKey);
}

bool CMcpClients::IsRemembered(const QString& Identity)
{
	if (Identity.isEmpty())
		return false;	// an identity we could not resolve is never remembered
	foreach(const QString& Known, GetRemembered()) {
		if (Known.compare(Identity, Qt::CaseInsensitive) == 0)
			return true;
	}
	return false;
}

void CMcpClients::Remember(const QString& Identity)
{
	if (Identity.isEmpty() || IsRemembered(Identity))
		return;
	QStringList List = GetRemembered();
	List.append(Identity);
	theConf->SetValue(kRememberedKey, List);
}

void CMcpClients::Forget(const QString& Identity)
{
	QStringList List = GetRemembered();
	for (int i = List.count() - 1; i >= 0; i--) {
		if (List.at(i).compare(Identity, Qt::CaseInsensitive) == 0)
			List.removeAt(i);
	}
	theConf->SetValue(kRememberedKey, List);
}

void CMcpClients::ForgetAll()
{
	theConf->SetValue(kRememberedKey, QStringList());
}

void CMcpSessionManager::PairRememberedFor(CMcpSession* pSession)
{
	pSession->PairRemembered();
	emit PairedFromMemory(pSession->GetClientName(), pSession->GetIdentity());
	emit PairingSucceeded(pSession->GetClientName());
}
