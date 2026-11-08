#include "stdafx.h"
#include "McpSubmitDialog.h"
#include "../SandMan.h"
#include "../Boxes/BoxReport.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QTextEdit>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QGroupBox>
#include <QScrollBar>
#include <QMessageBox>
#include <QStyle>

// The line the radio buttons write into the report. A marker rather than a
// position, so toggling between the two answers replaces it instead of
// stacking up contradictions - and so the user can delete it by hand like any
// other line, because the text in the box is what gets sent.
static const char* const kOutcomeLine = "Outcome as confirmed by the user: ";

CMcpSubmitDialog::CMcpSubmitDialog(const QString& Client, const QString& Text,
		const QString& ActionLog, bool bAttachIni, bool bAttachTrace, bool bAskOutcome,
		QWidget* parent)
 : QDialog(parent)
{
	m_pFixed = NULL;
	m_pBroken = NULL;
	setWindowTitle(tr("Send this to the Sandboxie-Plus project?"));
	// Wide enough to read a report in rather than scroll one through.
	resize(720, 560);

	QVBoxLayout* pLayout = new QVBoxLayout(this);

	// Encouraging, and honest about why. The reason to send is not that the
	// project wants data - it is that a result nobody reports has to be
	// rediscovered by the next person with the same problem.
	QLabel* pIntro = new QLabel(Client.isEmpty()
		? tr("The agent wrote this up while it worked.")
		: tr("%1 wrote this up while it worked.").arg(Client), this);
	pIntro->setWordWrap(true);
	QFont Bold = pIntro->font();
	Bold.setBold(true);
	pIntro->setFont(Bold);
	pLayout->addWidget(pIntro);

	QLabel* pWhy = new QLabel(tr(
		"Sending it is how what happened here reaches the people who maintain Sandboxie. A fix "
		"that worked tells them what to recommend to the next person with the same problem - and "
		"that is the report they almost never get. One that did not work tells them what is still "
		"broken.\n\n"
		"Nothing is sent unless you choose to, and nothing goes without you reading it. Edit or "
		"delete anything below first - the second half was written by the agent and can contain "
		"anything it saw."), this);
	pWhy->setWordWrap(true);
	pLayout->addWidget(pWhy);

	m_pReport = new QTextEdit(this);

	// The log goes in here rather than being attached, because it is small,
	// it is the sequence of what actually happened, and a maintainer reading
	// the narrative needs it on the same page. It was missing entirely: the
	// report said what the agent believed it had done, with nothing to check
	// that against.
	//
	// Added here rather than at compose time so it reaches up to this moment,
	// and so the user reads the real thing before it goes - the text in this
	// box is what gets sent.
	QString Full = Text;
	if (!ActionLog.isEmpty()) {
		// Headings in English like the rest of the report body. The entries
		// themselves are in the user's language, as the wizard's reports have
		// always been.
		if (!Full.endsWith("\n"))
			Full += "\n";
		Full += "\n================ WHAT SANDBOXIE-PLUS RECORDED ================\n";
		Full += "(observed and timed by Sandboxie-Plus, not written by the agent)\n";
		Full += ActionLog;
	}
	m_pReport->setPlainText(Full);
	m_pReport->setPlaceholderText(tr("Detailed issue description"));
	pLayout->addWidget(m_pReport, 1);

	if (bAskOutcome) {
		// Unanswered on purpose: neither is preselected, because a preselected
		// answer is the agent's guess wearing the user's name. Leaving both
		// clear sends the report with the outcome marked as unconfirmed, which
		// is the truth.
		QGroupBox* pOutcome = new QGroupBox(tr("Is the problem actually gone?"), this);
		QVBoxLayout* pOutcomeBox = new QVBoxLayout(pOutcome);

		QLabel* pAsk = new QLabel(tr("The agent did not establish this, and it is the single "
			"most useful line in the report - it is what tells the project whether the fix it is "
			"reading about actually works. Answering is optional."), pOutcome);
		pAsk->setWordWrap(true);
		pOutcomeBox->addWidget(pAsk);

		m_pFixed = new QRadioButton(tr("Yes - the problem is fixed"), pOutcome);
		m_pBroken = new QRadioButton(tr("No - it is still there"), pOutcome);
		pOutcomeBox->addWidget(m_pFixed);
		pOutcomeBox->addWidget(m_pBroken);
		pLayout->addWidget(pOutcome);

		connect(m_pFixed, &QRadioButton::toggled, this, &CMcpSubmitDialog::OnOutcomeChanged);
		connect(m_pBroken, &QRadioButton::toggled, this, &CMcpSubmitDialog::OnOutcomeChanged);
	}

	QGridLayout* pOptions = new QGridLayout();
	pOptions->setSpacing(2);

	m_pAttachIni = new QCheckBox(tr("Attach Sandboxie.ini"), this);
	m_pAttachIni->setChecked(bAttachIni);
	pOptions->addWidget(m_pAttachIni, 0, 0);

	m_pAttachLog = new QCheckBox(tr("Attach Logs"), this);
	m_pAttachLog->setTristate(true);
	// What the agent asked for: the trace is the larger and more revealing of
	// the two, so it is only pre-checked when the agent said it needed it.
	m_pAttachLog->setCheckState(bAttachTrace ? Qt::Checked : Qt::PartiallyChecked);
	pOptions->addWidget(m_pAttachLog, 0, 1);

	m_pMail = new QLineEdit(this);
	m_pMail->setText(theConf->GetString("User/eMail"));
	m_pMail->setPlaceholderText(tr("Email address"));
	pOptions->addWidget(m_pMail, 1, 0, 1, 2);
	pLayout->addLayout(pOptions);

	// Deliberately the same pair as the agent window ends with: the refusal on
	// the left with the red cross, the one to press on the right with the green
	// tick, at the same size. Only the words change between the two windows, so
	// the second one needs no reading to be understood.
	QHBoxLayout* pButtons = new QHBoxLayout();

	QPushButton* pDecline = new QPushButton(
		style()->standardIcon(QStyle::SP_DialogCancelButton), tr("Don't Send"), this);
	m_pSend = new QPushButton(
		style()->standardIcon(QStyle::SP_DialogApplyButton), tr("Send Report"), this);
	pDecline->setIconSize(QSize(24, 24));
	m_pSend->setIconSize(QSize(24, 24));
	pDecline->setMinimumHeight(36);
	m_pSend->setMinimumHeight(36);
	// The default, because it is the outcome worth having - but it is one
	// keystroke either way, and nothing here presses it for the user.
	m_pSend->setDefault(true);

	pButtons->addWidget(pDecline);
	pButtons->addStretch();
	pButtons->addWidget(m_pSend);
	pLayout->addLayout(pButtons);

	connect(m_pSend, &QPushButton::clicked, this, &CMcpSubmitDialog::OnSend);
	connect(pDecline, &QPushButton::clicked, this, &QDialog::reject);

	// Every control that changes what travels with the report refreshes the
	// summary of what travels with the report.
	connect(m_pAttachIni, &QCheckBox::stateChanged, this, &CMcpSubmitDialog::UpdateAttachmentHint);
	connect(m_pAttachLog, &QCheckBox::stateChanged, this, &CMcpSubmitDialog::UpdateAttachmentHint);
	connect(m_pMail, &QLineEdit::textChanged, this, &CMcpSubmitDialog::UpdateAttachmentHint);
	UpdateAttachmentHint();
}

void CMcpSubmitDialog::OnSend()
{
	CBoxReport::SSubmission Request;
	// What is on screen, not what was passed in: if the user cut a line out of
	// the report, the line is gone.
	Request.Text = m_pReport->toPlainText();
	Request.AttachLog = CBoxReport::AttachLogFrom(m_pAttachLog->checkState());
	Request.bAttachIni = m_pAttachIni->isChecked();
	Request.Mail = m_pMail->text().trimmed();
	CBoxReport::RememberMail(Request.Mail);

	// Dead while the upload runs, so a second click cannot send it twice. It
	// comes back if the tracker refuses, because then trying again - usually
	// without the log - is exactly the right move.
	m_pSend->setEnabled(false);

	if (!CBoxReport::Submit(this, Request, [this](bool bOk) {
			if (bOk) accept();
			else m_pSend->setEnabled(true);
		})) {
		m_pSend->setEnabled(true);
		QMessageBox::critical(this, "Sandboxie-Plus", tr("The report could not be sent."));
	}
}

void CMcpSubmitDialog::OnOutcomeChanged()
{
	if (!m_pFixed || !m_pBroken)
		return;

	// Written into the text the user is looking at rather than bolted on at
	// send time. The rule everywhere in this feature is that what is sent is
	// what was on screen, and a line added after the last look would break it.
	QStringList Lines = m_pReport->toPlainText().split(QChar('\n'));
	for (int i = Lines.count() - 1; i >= 0; i--) {
		if (Lines[i].startsWith(kOutcomeLine))
			Lines.removeAt(i);
	}
	while (!Lines.isEmpty() && Lines.last().trimmed().isEmpty())
		Lines.removeLast();

	if (m_pFixed->isChecked() || m_pBroken->isChecked()) {
		Lines.append(QString());
		// English, like the rest of the report body: a maintainer reads these,
		// and they arrive from every locale there is.
		Lines.append(QString(kOutcomeLine) + (m_pFixed->isChecked() ? "fixed" : "not_fixed"));
	}

	// Keeps the view where it was: the line lands at the end of a long
	// document and jumping there on every click would be seasickness.
	const int Scroll = m_pReport->verticalScrollBar()->value();
	m_pReport->setPlainText(Lines.join(QChar('\n')));
	m_pReport->verticalScrollBar()->setValue(Scroll);
}

void CMcpSubmitDialog::UpdateAttachmentHint()
{
	// Both lists, always: what will go and what could have gone. A checkbox
	// labelled "Attach Logs" tells nobody which logs, how many there are, or
	// that there is a second one it is not sending - and this is the one
	// decision in the feature that cannot be taken back.
	const int AttachLog = CBoxReport::AttachLogFrom(m_pAttachLog->checkState());

	QStringList With, Without;
	(AttachLog >= 1 ? With : Without).append(tr("the Sandboxie-Plus message log"));
	(AttachLog >= 2 ? With : Without).append(tr("the resource trace - everything every "
		"sandboxed program touched while it was on"));
	(m_pAttachIni->isChecked() ? With : Without).append(tr("your whole Sandboxie.ini, which "
		"describes every sandbox you have"));

	const QString Mail = m_pMail->text().trimmed();
	if (Mail.isEmpty())
		Without.append(tr("any contact address, so nobody can reply to you about it"));
	else
		With.append(tr("your address %1, so a maintainer can reply").arg(Mail));

	QString Tip = tr("Selecting the partially checked state sends the message log but not the "
		"resource trace. Both can be read in the main window before you send.") + "\n\n";

	Tip += tr("Sent with the report:");
	Tip += With.isEmpty()
		? "\n   - " + tr("nothing but the text above")
		: "\n   - " + With.join("\n   - ");
	if (!Without.isEmpty())
		Tip += "\n\n" + tr("Left out:") + "\n   - " + Without.join("\n   - ");

	// On all three, because any of them is where the question "what am I
	// actually sending" gets asked.
	m_pAttachIni->setToolTip(Tip);
	m_pAttachLog->setToolTip(Tip);
	m_pMail->setToolTip(Tip);
	m_pSend->setToolTip(Tip);
}

