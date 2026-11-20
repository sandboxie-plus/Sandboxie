#pragma once
#include <QWidget>
#include <QDateTime>

class QTreeWidget;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class QGroupBox;

//
// CMcpSessionWindow - one window for the whole agent session.
//
// This replaces a string of popup dialogs, and the reason is not tidiness. A
// modal box that appears on its own, with no history and no context, is
// something people dismiss by reflex - and the thing being dismissed here is
// permission to change their machine. A window that is already open, shows what
// has happened so far, and puts the current question at the bottom gives the
// decision somewhere to sit.
//
// It opens when an agent connects and stays open. Everything the agent does
// lands in the history, so the question at the bottom is read in the context of
// the five things that came before it.
//
// Non-modal on purpose: the user has to be able to look at their boxes, the
// trace, or the agent's chat window while deciding.
//
class CMcpSessionWindow : public QWidget
{
	Q_OBJECT

public:
	CMcpSessionWindow(QWidget* parent = NULL);

	enum EKind
	{
		eInfo = 0,		// connected, suspended, released
		eRead,			// something the agent looked at - muted, but recorded
		eChange,		// configuration actually written
		eAction,		// something was done - a snapshot, a terminate, a command
		eAllowed,		// a confirmation the user accepted
		eRefused,		// one they declined
		eWarning,
	};

	// bDetailInLog: whether Detail belongs in the exported action log as well
	// as in the window. False for anything whose detail is a document rather
	// than a fact about what happened - the composed report, above all, which
	// otherwise ends up quoted inside itself.
	void			AddEntry(EKind Kind, const QString& Text, const QString& Detail = QString(),
						bool bDetailInLog = true);

	// The history as plain text, for the report. This is the half of the
	// account that nobody has to be trusted for: it is what Sandboxie-Plus
	// itself saw happen, in order, including what the user allowed and
	// refused. The agent's narrative explains; this is the evidence.
	QString			GetActionLog() const;

	// The per-session override of how much gets asked. Starts at the default
	// from the settings and is not written back - a choice made in the middle
	// of one conversation should not quietly become the policy for every
	// future one.
	//
	// bNewAgent resets it to that default, which is what a new client
	// connecting must do: an allowance granted to one agent is not inherited
	// by the next one to turn up. Without it the setting is only re-read, so
	// an administrator tightening the ceiling takes effect without throwing
	// away a choice the user already made.
	void			RefreshConsentMode(bool bNewAgent = false);
	void			SetStatus(const QString& Text, const QString& Client = QString());

	// The pairing code, shown here rather than in a box of its own. Identity is
	// the resolved program path, and is what the "do not ask again" offer would
	// remember - so the offer appears only when we actually established one.
	void			ShowCode(const QString& Code, int ValidSeconds, const QString& Identity = QString());
	void			ClearCode();
	bool			IsRememberChecked() const;

	// Puts a question at the bottom and waits for an answer. Returns true only
	// on an explicit yes; closing the window counts as a refusal.
	//
	// Blocks in a nested event loop, which is what keeps the agent waiting on
	// the user rather than being told "asked, check back later".
	bool			AskPrompt(const QString& Title, const QString& Body, const QString& ConfirmText);
	bool			IsPrompting() const		{ return m_bPrompting; }

	// Brings it up without asking anything - used when a session starts.
	void			Appear();

	// The report the agent has written so far, kept here rather than only on
	// the session, because the session dies with the client - and the client
	// dying is exactly the case this exists for. Somebody closes the agent's
	// console halfway through and walks off; what it had worked out by then is
	// still worth sending, and nobody was ever asked before this.
	//
	// Every report_compose overwrites it, so the offer is always the agent's
	// latest understanding rather than its first guess.
	void			SetPendingReport(const QString& Client, const QString& Text,
						bool bAttachIni, bool bAttachTrace, bool bOutcomeConfirmed);
	void			ClearPendingReport();

	// Forgets that the user said no, which only a genuinely new agent should
	// do: within one session a decline is final, because an agent that now
	// composes after every change would otherwise re-arm the question every
	// time the window is closed.
	// A new agent: the questions this window asks start over, and a decline by
	// the last one is not inherited by this one.
	void			NoteSessionStarted();

	// The agent was authorised, so from here on there is a session worth
	// confirming the end of. Before it, closing this window is somebody
	// deciding not to let an agent in - which is an answer in itself and does
	// not deserve to be second-guessed.
	void			NoteAuthorised();

	// Back to the state of a window nobody has used: no history, no code, no
	// write-up, nothing remembered about who was asked what. Called when the
	// agent connection closes, whoever closed it, because the next session is
	// a different conversation and the leftovers of the last one are at best
	// confusing and at worst a report attributed to the wrong agent.
	void			ResetAll();
	bool			HasPendingReport() const	{ return !m_PendingReport.isEmpty(); }

signals:
	void			Answered();

protected:
	virtual void	closeEvent(QCloseEvent* pEvent);

private slots:
	void			OnAllow();
	void			OnFinish();
	void			OnCancel();
	void			OnModeChanged(int Index);
	void			OnRefuse();

private:
	void			Answer(bool bAllowed);
	void			ShowIdle();
	void			OfferPendingReport();
	// What to do about a window the user is closing.
	enum EEnd
	{
		eStayOpen = 0,
		eCloseKeepServer,
		eCloseAndStop,
	};
	EEnd			AskHowToEnd();
	void			UpdateFinish();
	// False to stay open.
	bool			CloseSession();

	QLabel*			m_pStatus;
	class QComboBox* m_pMode;
	QLabel*			m_pModeNote;
	// The code gets its own panel rather than one long label: only the digits
	// want the oversized face, and a sentence set in it wraps badly.
	class QFrame*	 m_pCodePanel;
	QLabel*			m_pCode;
	QLabel*			m_pCodeNote;
	class QCheckBox* m_pRemember;
	QTreeWidget*	m_pHistory;

	QGroupBox*		m_pPromptBox;
	QLabel*			m_pPromptTitle;
	QPlainTextEdit*	m_pPromptBody;
	QPushButton*	m_pAllow;
	QPushButton*	m_pRefuse;

	// How a session ends. Shown while nobody has established whether the
	// problem is actually gone, which is the case these exist for: the agent
	// has stopped doing things and the run needs an ending.
	QPushButton*	m_pFinish;
	QPushButton*	m_pCancel;
	QLabel*			m_pIdle;

	bool			m_bPrompting;
	bool			m_bAnswer;

	// Bumped per prompt, so the arming timer of an answered prompt cannot arm
	// the Allow button of the next one.
	int				m_PromptGeneration;

	QString			m_PendingClient;
	bool			m_bOutcomeConfirmed = false;
	bool			m_bHadAccess = false;
	QString			m_PendingReport;
	bool			m_bPendingIni = false;
	bool			m_bPendingTrace = false;
};
