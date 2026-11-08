#pragma once
#include <QDialog>

class QTextEdit;
class QCheckBox;
class QLineEdit;
class QLabel;
class QPushButton;
class QRadioButton;

//
// CMcpSubmitDialog - the offer to send what the agent found to the project.
//
// A window of its own rather than a page of the session window, and that is the
// point of it. The session window is where permission is granted while work is
// in progress; this is a separate decision, made after the work, about
// something leaving the machine. Putting it in the same frame as the Allow
// button would make it look like one more step of the same flow, and it is not:
// everything else the agent asks for stays on this computer.
//
// It is also what closes the hole this feature had. An agent that fixed
// something would say so in its own chat window and the knowledge died there:
// the user was never asked, so the one report the project almost never receives
// - the one that says what the fix was - was never offered. The session window
// now offers this when it closes, with whatever the agent had written by then.
//
// Modal, unlike the session window, because there is nothing to do alongside it
// but read the text. Accepted means the tracker took it; rejected means no.
//
class CMcpSubmitDialog : public QDialog
{
	Q_OBJECT

public:
	// Text is the agent's draft, verbatim; the user may edit or gut it before
	// sending, which is their right over anything that leaves their machine.
	// bAttachIni and bAttachTrace are what the agent asked for, and are no more
	// than a starting position for the checkboxes.
	//
	// bAskOutcome: nobody has established whether the problem is actually
	// gone. The agent is supposed to ask and record it, and when it has not,
	// this is the last chance to find out from the only person who knows -
	// which matters more than it sounds, because "fixed" and "not fixed" are
	// what make the corpus worth querying at all.
	// ActionLog is what Sandboxie-Plus recorded itself, appended to the report
	// as a section of its own. The agent writes the story; this is the part
	// that is not written by anybody.
	CMcpSubmitDialog(const QString& Client, const QString& Text, const QString& ActionLog,
				bool bAttachIni, bool bAttachTrace, bool bAskOutcome, QWidget* parent = NULL);

private slots:
	void			OnSend();
	void			OnOutcomeChanged();
	void			UpdateAttachmentHint();

private:
	QTextEdit*		m_pReport;
	QCheckBox*		m_pAttachIni;
	QCheckBox*		m_pAttachLog;
	QLineEdit*		m_pMail;
	QPushButton*	m_pSend;
	QRadioButton*	m_pFixed;
	QRadioButton*	m_pBroken;
};
