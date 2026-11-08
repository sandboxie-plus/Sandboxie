#pragma once
#include <QObject>
#include <QString>
#include <QList>

//
// CMcpConsent - the one place a change gets approved.
//
// Every tool that alters anything goes through here, and the rules are the same
// for all of them:
//
//   * the user sees what will change, not a summary of it. A diff of current
//     against proposed, per setting, because "apply the recommended settings"
//     is not something anyone can meaningfully agree to
//   * the agent that asked is named. Approving a change for Claude Code is a
//     different decision from approving one for a program you do not recognise
//   * the default button is Refuse, and closing the window is a refusal
//   * one prompt is open at a time. A second request while the user is reading
//     the first is refused rather than stacked - a queue of pending approvals
//     is how people click Yes to the wrong thing
//
// The question is not a popup. It appears at the bottom of CMcpSessionWindow,
// which has been open since the agent connected and shows everything it has
// done so far - so the decision is read in context, rather than as a box that
// appeared on its own and is dismissed by reflex.
//
// The prompt runs inside the request that triggered it, so the agent waits on
// the user. That is intended: an asynchronous "I have asked, check back later"
// would let a model carry on as though it had been approved.
//
class CMcpConsent : public QObject
{
	Q_OBJECT

public:
	CMcpConsent(QObject* parent = NULL);

	struct SChange
	{
		QString Setting;
		QString Current;	// empty when the setting is not set yet
		QString Proposed;	// empty means delete
		bool	bKnown = true;	// in the settings schema at all
	};

	//
	// How much is asked, and why this is a setting rather than a fixed policy.
	//
	// A user who has given an agent a screen and a mouse can already do all of
	// this by clicking, so for them a confirmation is a button the agent
	// presses. Insisting on it anyway is the kind of friction that teaches
	// people to click Allow without reading, which costs them on the one
	// prompt that mattered.
	//
	// But the confirmation is not mainly protecting the user from their agent.
	// The diagnose tools return file paths, process names and command lines
	// produced by programs running in sandboxes - which is to say by exactly
	// the untrusted software Sandboxie exists to contain. A program in a box
	// can name a file so that it reads as an instruction, and that text
	// arrives in the agent's context as diagnostic data. Against a prompt
	// injection a clean typed API is *more* dangerous than screen-and-click:
	// the injected agent calls the tool once and it works, silently, instead
	// of visibly fumbling a GUI.
	//
	// So the default asks, and the user may decide otherwise - per session
	// from the agent window, or as a default in the settings. What never
	// changes with the mode is the recording: everything still lands in the
	// history and the audit log, because not being asked is not the same as
	// not being told.
	//
	enum EMode
	{
		eReadOnly = 0,	// refuse every change; the agent may look and nothing else
		eAskAlways,		// confirm anything that changes, starts, stops or sends
		eAskRisky,		// confirm only what cannot be undone, runs code, leaves the
						// machine, or opens a hole in a sandbox
		eAskNever,		// act, and record it
	};

	void			SetMode(EMode Mode)		{ m_Mode = Mode; }
	EMode			GetMode() const			{ return m_Mode; }

	// The administrator's ceiling, from Sandboxie.ini rather than the per-user
	// file: when confirmation is required the looser modes are unavailable,
	// whatever the user or the agent would prefer.
	static bool		IsConfirmationRequired();
	// The mode actually in force, which is GetMode() clamped by the above.
	EMode			EffectiveMode() const;

	static QString	ModeName(EMode Mode);
	static QString	ModeDescription(EMode Mode);

	// What a request would cost if it were wrong. The mode decides which of
	// these still stops to ask.
	enum ERisk
	{
		eReversible = 0,	// a setting that can be put back the way it was
		eOpensAccess,		// grants a sandbox reach it did not have
		eDestroys,			// throws away work or data
		eExecutes,			// runs a program
		eLeavesMachine,		// sends something off this computer
	};

	// A set of configuration changes, shown as a diff. Returns true only on an
	// explicit yes. The risk is derived from the settings themselves: one that
	// opens a sandbox up is not in the same class as one that closes it.
	bool			AskChanges(const QString& Client, const QString& BoxLabel,
						const QString& Rationale, const QList<SChange>& Changes);

	// Anything that is not a config diff - a rollback, a terminate, a new box.
	// Detail is shown verbatim, so it has to say what will actually happen,
	// including what is irreversible.
	bool			AskAction(const QString& Client, const QString& Title,
						const QString& Detail, const QString& ConfirmText,
						ERisk Risk = eDestroys);

	// True when the setting grants a sandbox access it did not have. These are
	// the changes a prompt injection would want, so they keep asking even in
	// the mode that otherwise does not.
	static bool		OpensAccess(const QString& Setting);

	// True while a prompt is on screen. The caller refuses rather than queueing.
	bool			IsPrompting() const	{ return m_bPrompting; }

signals:
	// For the audit log and the message log: every decision is recorded,
	// including the refusals, which are the ones worth being able to point at
	// later.
	void			Decided(const QString& What, bool bAllowed, const QString& Detail);

private:
	bool			Ask(const QString& Title, const QString& Body, const QString& ConfirmText,
						const QString& AuditWhat, ERisk Risk);

	bool			m_bPrompting = false;
	EMode			m_Mode = eAskAlways;
};
