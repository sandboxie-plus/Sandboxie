#include "stdafx.h"
#include "McpSessionWindow.h"
#include "../SandMan.h"
#include <QTreeWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QEventLoop>
#include <QCloseEvent>
#include <QStyle>
#include <QCheckBox>
#include <QApplication>
#include <QTimer>
#include <QFrame>
#include <QComboBox>
#include <QMessageBox>
#include "McpConsent.h"
#include "McpServer.h"
#include "McpSubmitDialog.h"

// Enough history to see the shape of a session without becoming its own
// scrolling problem.
static const int kMaxHistory = 500;

// How long Allow stays dead after a question appears. Long enough that a
// carried-over or reflex click cannot approve something unread, short enough
// that someone who has read it never waits.
static const int kArmingDelayMs = 900;

CMcpSessionWindow::CMcpSessionWindow(QWidget* parent)
 : QWidget(parent)
{
	m_bPrompting = false;
	m_bAnswer = false;
	m_PromptGeneration = 0;

	setWindowTitle(tr("AI Agent Session - Sandboxie-Plus"));
	setWindowFlag(Qt::Window);
	resize(720, 560);

	QVBoxLayout* pLayout = new QVBoxLayout(this);

	m_pStatus = new QLabel(this);
	m_pStatus->setWordWrap(true);
	pLayout->addWidget(m_pStatus);

	// How much this agent is asked, for this session only. The default comes
	// from the settings; changing it here does not change that default,
	// because a decision made in the middle of one conversation is about that
	// conversation.
	QHBoxLayout* pModeRow = new QHBoxLayout();
	pModeRow->addWidget(new QLabel(tr("For this session:"), this));
	m_pMode = new QComboBox(this);
	for (int i = CMcpConsent::eReadOnly; i <= CMcpConsent::eAskNever; i++) {
		m_pMode->addItem(CMcpConsent::ModeName((CMcpConsent::EMode)i), i);
		m_pMode->setItemData(m_pMode->count() - 1,
			CMcpConsent::ModeDescription((CMcpConsent::EMode)i), Qt::ToolTipRole);
	}
	pModeRow->addWidget(m_pMode, 1);
	pLayout->addLayout(pModeRow);

	m_pModeNote = new QLabel(this);
	m_pModeNote->setWordWrap(true);
	pLayout->addWidget(m_pModeNote);

	connect(m_pMode, SIGNAL(currentIndexChanged(int)), this, SLOT(OnModeChanged(int)));

	// The pairing code, as a panel of three parts rather than one sentence.
	// Only the digits get the oversized face - setting the whole line in it
	// made it wrap mid-phrase, which is both ugly and harder to read out.
	m_pCodePanel = new QFrame(this);
	m_pCodePanel->setFrameShape(QFrame::StyledPanel);
	m_pCodePanel->setVisible(false);
	QVBoxLayout* pCodeBox = new QVBoxLayout(m_pCodePanel);

	QLabel* pCodeIntro = new QLabel(tr("Give the agent this code:"), m_pCodePanel);
	pCodeBox->addWidget(pCodeIntro);

	m_pCode = new QLabel(m_pCodePanel);
	// No wrapping: six digits never need it, and a wrap here would break the
	// one thing on screen that has to be read character by character.
	m_pCode->setWordWrap(false);
	m_pCode->setTextInteractionFlags(Qt::TextSelectableByMouse);
	m_pCode->setAlignment(Qt::AlignCenter);
	QFont CodeFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	CodeFont.setPointSize(CodeFont.pointSize() + 10);
	CodeFont.setBold(true);
	// Spread out, because this is read aloud or copied by hand.
	CodeFont.setLetterSpacing(QFont::PercentageSpacing, 135);
	m_pCode->setFont(CodeFont);
	pCodeBox->addWidget(m_pCode);

	m_pCodeNote = new QLabel(m_pCodePanel);
	m_pCodeNote->setWordWrap(true);
	pCodeBox->addWidget(m_pCodeNote);

	m_pRemember = new QCheckBox(tr("Do not ask again for this program"), m_pCodePanel);
	m_pRemember->setVisible(false);
	pCodeBox->addWidget(m_pRemember);

	pLayout->addWidget(m_pCodePanel);

	pLayout->addWidget(new QLabel(tr("What this agent has done:"), this));
	m_pHistory = new QTreeWidget(this);
	m_pHistory->setRootIsDecorated(false);
	m_pHistory->setUniformRowHeights(true);
	m_pHistory->setHeaderLabels(QStringList() << tr("Time") << tr("What happened"));
	m_pHistory->header()->setStretchLastSection(true);
	m_pHistory->setColumnWidth(0, 70);
	pLayout->addWidget(m_pHistory, 1);

	// The question lives at the bottom, in the same place every time, so it is
	// never somewhere unexpected.
	m_pPromptBox = new QGroupBox(tr("Waiting for you"), this);
	QVBoxLayout* pPrompt = new QVBoxLayout(m_pPromptBox);

	m_pIdle = new QLabel(tr("Nothing is waiting. Once authorised the agent can read your "
		"sandboxes freely; anything it wants to change, start or stop will appear here "
		"first, and nothing happens until you answer."), m_pPromptBox);
	m_pIdle->setWordWrap(true);
	pPrompt->addWidget(m_pIdle);

	m_pPromptTitle = new QLabel(m_pPromptBox);
	m_pPromptTitle->setWordWrap(true);
	QFont TitleFont = m_pPromptTitle->font();
	TitleFont.setBold(true);
	m_pPromptTitle->setFont(TitleFont);
	m_pPromptTitle->setVisible(false);
	pPrompt->addWidget(m_pPromptTitle);

	m_pPromptBody = new QPlainTextEdit(m_pPromptBox);
	m_pPromptBody->setReadOnly(true);
	m_pPromptBody->setMaximumHeight(180);
	m_pPromptBody->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	m_pPromptBody->setVisible(false);
	pPrompt->addWidget(m_pPromptBody);

	QHBoxLayout* pButtons = new QHBoxLayout();
	// Icons so the answer can be found at a glance rather than read. Qt's
	// standard set means these match the platform instead of inventing a look.
	m_pAllow = new QPushButton(style()->standardIcon(QStyle::SP_DialogApplyButton), tr("Allow"), m_pPromptBox);
	m_pRefuse = new QPushButton(style()->standardIcon(QStyle::SP_DialogCancelButton), tr("Refuse"), m_pPromptBox);
	m_pAllow->setIconSize(QSize(24, 24));
	m_pRefuse->setIconSize(QSize(24, 24));
	m_pAllow->setMinimumHeight(36);
	m_pRefuse->setMinimumHeight(36);
	// Refuse is the default: a stray return key must not approve anything.
	m_pRefuse->setDefault(true);
	pButtons->addStretch();
	pButtons->addWidget(m_pAllow);
	pButtons->addWidget(m_pRefuse);
	pPrompt->addLayout(pButtons);

	m_pAllow->setVisible(false);
	m_pRefuse->setVisible(false);

	pLayout->addWidget(m_pPromptBox);

	// How the session ends, as two explicit choices rather than only a window
	// you close. Before this the end of a run had no shape at all: the agent
	// stopped talking and the user was left with a window and no idea that it
	// was holding anything worth sending.
	//
	// Cancel on the left and Finish on the right, in that order, because the
	// one on the right is the one to press and that is where the eye finishes.
	//
	// Always both, always visible. They were briefly hidden once the agent
	// recorded a confirmed result, on the theory that a run with a known
	// outcome needs no manual ending - which took the controls away at exactly
	// the moment the user had finished talking to the agent and wanted to wrap
	// up. A window whose buttons come and go cannot be learned.
	QHBoxLayout* pEndRow = new QHBoxLayout();
	m_pCancel = new QPushButton(style()->standardIcon(QStyle::SP_DialogCancelButton), tr("Cancel"), this);
	m_pFinish = new QPushButton(style()->standardIcon(QStyle::SP_DialogApplyButton), tr("Finish"), this);
	m_pCancel->setIconSize(QSize(24, 24));
	m_pFinish->setIconSize(QSize(24, 24));
	m_pCancel->setMinimumHeight(36);
	m_pFinish->setMinimumHeight(36);
	m_pCancel->setToolTip(tr("End this session without sending anything."));
	// Neither is the default. A stray return key belongs to the prompt above,
	// where Refuse already has it.
	m_pCancel->setAutoDefault(false);
	m_pFinish->setAutoDefault(false);
	pEndRow->addWidget(m_pCancel);
	pEndRow->addStretch();
	pEndRow->addWidget(m_pFinish);
	pLayout->addLayout(pEndRow);

	connect(m_pAllow, &QPushButton::clicked, this, &CMcpSessionWindow::OnAllow);
	connect(m_pRefuse, &QPushButton::clicked, this, &CMcpSessionWindow::OnRefuse);
	connect(m_pCancel, &QPushButton::clicked, this, &CMcpSessionWindow::OnCancel);
	connect(m_pFinish, &QPushButton::clicked, this, &CMcpSessionWindow::OnFinish);
	UpdateFinish();

	// SandMan shutting down while a question stands would leave an agent
	// waiting on a nested event loop inside a window that is going away. The
	// answer in that case is no - which is also the safe one, so a close that
	// is subsequently cancelled costs nothing but a repeated request.
	if (theGUI)
		connect(theGUI, &CSandMan::Closed, this, [this]() {
			if (m_bPrompting) {
				AddEntry(eRefused, tr("Sandboxie-Plus was closing, so the request was refused"));
				Answer(false);
			}
		});

	SetStatus(tr("No agent is connected."));
	RefreshConsentMode();
}

//
// Reads the default out of the settings and applies the administrator's
// ceiling. Called when the window is built and whenever a session starts, so
// a change to either is picked up without a restart.
//
void CMcpSessionWindow::RefreshConsentMode(bool bNewAgent)
{
	CMcpConsent* pConsent = theGUI && theGUI->GetMcpServer()
		? theGUI->GetMcpServer()->GetConsent() : NULL;

	int Default = theConf->GetInt("Options/McpConsentMode", CMcpConsent::eAskAlways);
	if (Default < CMcpConsent::eReadOnly || Default > CMcpConsent::eAskNever)
		Default = CMcpConsent::eAskAlways;

	// Keep what the user picked unless a different agent has turned up. An
	// earlier revision re-applied the default on every session and so threw
	// away a choice made moments before, which produced the worst possible
	// outcome: two identical requests in the same run, one asked about and
	// one not.
	if (!bNewAgent && m_pMode->currentIndex() >= 0)
		Default = m_pMode->currentData().toInt();

	const bool bRequired = CMcpConsent::IsConfirmationRequired();
	if (bRequired && Default > CMcpConsent::eAskAlways)
		Default = CMcpConsent::eAskAlways;

	// The looser halves are not merely clamped, they are taken away: a
	// disabled entry that explains itself beats one that silently does
	// nothing when picked.
	for (int i = 0; i < m_pMode->count(); i++) {
		const int Mode = m_pMode->itemData(i).toInt();
		const bool bAllowed = !bRequired || Mode <= CMcpConsent::eAskAlways;
		m_pMode->setItemData(i, bAllowed ? QVariant() : QVariant(0), Qt::UserRole - 1);
	}

	m_pMode->blockSignals(true);
	m_pMode->setCurrentIndex(m_pMode->findData(Default));
	m_pMode->blockSignals(false);

	if (pConsent)
		pConsent->SetMode((CMcpConsent::EMode)Default);

	m_pModeNote->setText(bRequired
		? tr("%1 Asking cannot be turned off on this machine: AiAgentRequireConfirmation is set "
			"in Sandboxie.ini.").arg(CMcpConsent::ModeDescription((CMcpConsent::EMode)Default))
		: CMcpConsent::ModeDescription((CMcpConsent::EMode)Default));

	// In the history when it changes under the user, because the one thing
	// worse than being asked too often is not knowing whether you will be.
	if (bNewAgent)
		AddEntry((CMcpConsent::EMode)Default >= CMcpConsent::eAskRisky ? eWarning : eInfo,
			tr("this session starts at: %1").arg(CMcpConsent::ModeName((CMcpConsent::EMode)Default)));
}

void CMcpSessionWindow::OnModeChanged(int Index)
{
	const CMcpConsent::EMode Mode = (CMcpConsent::EMode)m_pMode->itemData(Index).toInt();
	m_pModeNote->setText(CMcpConsent::ModeDescription(Mode));

	CMcpConsent* pConsent = theGUI && theGUI->GetMcpServer()
		? theGUI->GetMcpServer()->GetConsent() : NULL;
	if (pConsent)
		pConsent->SetMode(Mode);

	// In the history, because changing how much you are asked is itself one
	// of the more consequential things that happens in a session.
	AddEntry(Mode >= CMcpConsent::eAskRisky ? eWarning : eInfo,
		tr("you set this session to: %1").arg(CMcpConsent::ModeName(Mode)));
}

void CMcpSessionWindow::SetStatus(const QString& Text, const QString& Client)
{
	m_pStatus->setText(Client.isEmpty() ? Text : QString("%1 - %2").arg(Client).arg(Text));
}

QString CMcpSessionWindow::GetActionLog() const
{
	QString Log;
	for (int i = 0; i < m_pHistory->topLevelItemCount(); i++) {
		const QTreeWidgetItem* pItem = m_pHistory->topLevelItem(i);
		Log += pItem->text(0) + "  " + pItem->text(1) + "\n";

		// The detail is the tooltip in the window - the settings that changed,
		// the command line, the identity of the program. Indented under its
		// line rather than dropped, because that is usually the part a
		// maintainer wants.
		//
		// Except where the detail is a document. The write-up entries carry the
		// whole composed report as their tooltip, which is right in the window
		// and absurd here: the report ended up containing two copies of itself,
		// section headings and all, inside its own log of what happened. What
		// the agent did belongs here; what it wrote does not.
		if (!pItem->data(1, Qt::UserRole).toBool())
			continue;

		const QString Detail = pItem->toolTip(1).trimmed();
		if (Detail.isEmpty())
			continue;

		// And a ceiling even on the ones that are facts: a trace query or a
		// long diagnostic can run to hundreds of lines, and the log is meant to
		// be the shape of the session rather than its contents.
		const int kMaxDetailLines = 8;
		const QStringList Lines = Detail.split(QChar('\n'));
		for (int j = 0; j < Lines.count() && j < kMaxDetailLines; j++)
			Log += "      " + Lines.at(j) + "\n";
		if (Lines.count() > kMaxDetailLines)
			Log += QString("      ... and %1 more lines\n").arg(Lines.count() - kMaxDetailLines);
	}
	return Log;
}

void CMcpSessionWindow::AddEntry(EKind Kind, const QString& Text, const QString& Detail,
		bool bDetailInLog)
{
	QIcon Icon;
	switch (Kind) {
		case eRead:		Icon = style()->standardIcon(QStyle::SP_FileIcon); break;
		case eChange:	Icon = style()->standardIcon(QStyle::SP_FileDialogDetailedView); break;
		case eAction:	Icon = style()->standardIcon(QStyle::SP_MediaPlay); break;
		case eAllowed:	Icon = style()->standardIcon(QStyle::SP_DialogApplyButton); break;
		case eRefused:	Icon = style()->standardIcon(QStyle::SP_DialogCancelButton); break;
		case eWarning:	Icon = style()->standardIcon(QStyle::SP_MessageBoxWarning); break;
		default:		Icon = style()->standardIcon(QStyle::SP_MessageBoxInformation); break;
	}

	QTreeWidgetItem* pItem = new QTreeWidgetItem(m_pHistory);
	pItem->setText(0, QDateTime::currentDateTime().toString("HH:mm:ss"));
	pItem->setText(1, Text);
	pItem->setIcon(1, Icon);
	if (Kind == eRead) {
		// Reads are recorded but dimmed. Leaving them out would make the window
		// a confirmation log rather than an account of what the agent did;
		// giving them equal weight would bury the three lines that matter.
		QColor Dim = palette().color(QPalette::Disabled, QPalette::WindowText);
		pItem->setForeground(0, Dim);
		pItem->setForeground(1, Dim);
	}
	if (!Detail.isEmpty())
		pItem->setToolTip(1, Detail);
	pItem->setData(1, Qt::UserRole, bDetailInLog);

	while (m_pHistory->topLevelItemCount() > kMaxHistory)
		delete m_pHistory->takeTopLevelItem(0);

	m_pHistory->scrollToBottom();
}

void CMcpSessionWindow::ShowCode(const QString& Code, int ValidSeconds, const QString& Identity)
{
	// Grouped in threes for reading out loud. Harmless to copy that way too:
	// CMcpSession::SubmitCode strips separators before comparing.
	const QString Grouped = Code.length() == 6
		? QString("%1 %2").arg(Code.left(3)).arg(Code.mid(3)) : Code;
	m_pCode->setText(Grouped);

	const int Minutes = ValidSeconds / 60;
	m_pCodeNote->setText(tr("Valid for %1 minutes. Until it is entered the agent can do nothing. "
		"If you did not ask for this, ignore it.").arg(Minutes));

	m_pCodePanel->setVisible(true);

	// Offered only when the program could be identified - remembering an
	// identity we never established would remember nothing useful.
	m_pRemember->setChecked(false);
	m_pRemember->setVisible(!Identity.isEmpty());
	if (!Identity.isEmpty())
		m_pRemember->setToolTip(tr("Skips this code next time. Anything able to run from "
			"%1 will then be authorised without asking.").arg(Identity));

	Appear();
}

void CMcpSessionWindow::ClearCode()
{
	m_pCode->clear();
	m_pRemember->setVisible(false);
	m_pCodePanel->setVisible(false);
}

bool CMcpSessionWindow::IsRememberChecked() const
{
	return m_pRemember->isVisible() && m_pRemember->isChecked();
}

void CMcpSessionWindow::Appear()
{
	if (!isVisible())
		show();
	raise();
	activateWindow();

	// activateWindow() from a process that is not in the foreground is
	// unreliable on Windows, and the agent's own window is usually what has
	// focus. alert() flashes the taskbar button instead - asking for attention
	// rather than taking it, which is also the right manners for a window that
	// appears while someone is typing somewhere else.
	QApplication::alert(this, 0);
}

void CMcpSessionWindow::ShowIdle()
{
	m_pPromptTitle->setVisible(false);
	m_pPromptBody->setVisible(false);
	m_pAllow->setVisible(false);
	m_pRefuse->setVisible(false);
	m_pIdle->setVisible(true);
	m_pPromptBox->setTitle(tr("Waiting for you"));
}

bool CMcpSessionWindow::AskPrompt(const QString& Title, const QString& Body, const QString& ConfirmText)
{
	if (m_bPrompting)
		return false;	// one question at a time; the caller refuses rather than queueing

	m_bPrompting = true;
	m_bAnswer = false;

	m_pPromptBox->setTitle(tr("Waiting for you - please answer"));
	m_pPromptTitle->setText(Title);
	m_pPromptBody->setPlainText(Body);
	m_pAllow->setText(ConfirmText);
	m_pIdle->setVisible(false);
	m_pPromptTitle->setVisible(true);
	m_pPromptBody->setVisible(true);
	m_pAllow->setVisible(true);
	m_pRefuse->setVisible(true);
	m_pRefuse->setFocus();
	setWindowTitle(tr("AI Agent Session - waiting for you"));

	// Allow starts out dead for a moment, and this is the one piece of
	// deliberate friction in the window. The buttons are in the same place
	// every time, which is what makes them fast to use - and also what makes a
	// double-click on one request capable of approving the next one before it
	// has been read, since the second press lands on a button that has already
	// been replaced underneath the cursor. Refuse stays live throughout:
	// a stray click should be able to decline instantly, never to approve.
	const int Generation = ++m_PromptGeneration;
	m_pAllow->setEnabled(false);
	QTimer::singleShot(kArmingDelayMs, this, [this, Generation]() {
		if (m_bPrompting && m_PromptGeneration == Generation)
			m_pAllow->setEnabled(true);
	});

	Appear();

	// Nested loop, the same mechanism a modal dialog uses - the difference is
	// that this window is not modal, so the rest of SandMan stays usable while
	// the question stands.
	QEventLoop Loop;
	connect(this, &CMcpSessionWindow::Answered, &Loop, &QEventLoop::quit);
	Loop.exec();

	m_bPrompting = false;
	m_pAllow->setEnabled(true);
	setWindowTitle(tr("AI Agent Session - Sandboxie-Plus"));
	ShowIdle();
	return m_bAnswer;
}

void CMcpSessionWindow::Answer(bool bAllowed)
{
	if (!m_bPrompting)
		return;
	m_bAnswer = bAllowed;
	emit Answered();
}

void CMcpSessionWindow::OnAllow()	{ Answer(true); }
void CMcpSessionWindow::OnRefuse()	{ Answer(false); }

void CMcpSessionWindow::closeEvent(QCloseEvent* pEvent)
{
	// Always ignored: this window hides itself rather than being destroyed, so
	// the history survives until the connection closes.
	pEvent->ignore();

	// Not while the application is tearing down: a modal question during
	// shutdown is a hang waiting to happen, and the Closed handler has already
	// refused anything outstanding.
	if (QApplication::closingDown()) {
		hide();
		return;
	}

	CloseSession();
}

void CMcpSessionWindow::SetPendingReport(const QString& Client, const QString& Text,
		bool bAttachIni, bool bAttachTrace, bool bOutcomeConfirmed)
{
	const bool bFirst = m_PendingReport.isEmpty();

	m_PendingClient = Client;
	m_PendingReport = Text;
	m_bPendingIni = bAttachIni;
	m_bPendingTrace = bAttachTrace;

	// One way only: an agent that composed a confirmed outcome and then
	// composes again without the flag has not un-asked the user.
	if (bOutcomeConfirmed && !m_bOutcomeConfirmed) {
		m_bOutcomeConfirmed = true;
		AddEntry(eInfo, tr("recorded that you confirmed the result"));
	}

	// In the history either way, so "what did it write about me" is answerable
	// at any point rather than only when the offer appears.
	AddEntry(eInfo, bFirst
		? tr("wrote up what it has found - press Finish when you are done to read and send it")
		: tr("updated its write-up of what it has found"), Text, false);

	UpdateFinish();
}

void CMcpSessionWindow::ClearPendingReport()
{
	m_PendingClient.clear();
	m_PendingReport.clear();
	m_bPendingIni = false;
	m_bPendingTrace = false;
}

void CMcpSessionWindow::NoteSessionStarted()
{
	m_bOutcomeConfirmed = false;
	m_bHadAccess = false;
}

void CMcpSessionWindow::NoteAuthorised()
{
	m_bHadAccess = true;
}

void CMcpSessionWindow::UpdateFinish()
{
	// Dead until there is something to finish with. It used to be live always
	// and answer with a message box saying there was nothing written up, which
	// is a button that exists to tell you it does not work - a disabled button
	// says the same thing before the click rather than after it.
	//
	// The tooltip carries the why, because a greyed-out control with no
	// explanation is its own kind of dead end.
	const bool bReady = HasPendingReport();
	m_pFinish->setEnabled(bReady);
	m_pFinish->setToolTip(bReady
		? tr("Wrap up: read what the agent wrote and decide whether to send it.")
		: tr("The agent has not written anything up yet. Ask it to write up what it found - it "
			"has a tool for that - and this becomes available."));
}

void CMcpSessionWindow::ResetAll()
{
	// A question still on screen belongs to a session that no longer exists,
	// and the answer to a question nobody can hear is no.
	if (m_bPrompting) {
		AddEntry(eRefused, tr("the connection closed while a request was waiting, so it was refused"));
		Answer(false);
	}

	m_pHistory->clear();
	ClearCode();
	ClearPendingReport();
	UpdateFinish();
	m_bOutcomeConfirmed = false;
	m_bHadAccess = false;
	SetStatus(tr("No agent is connected."));
	ShowIdle();
}

void CMcpSessionWindow::OnFinish()
{
	// A question still standing has to be answered first: finishing while an
	// agent waits in a nested event loop would strand it there.
	if (m_bPrompting) {
		AddEntry(eRefused, tr("the session was finished while a request was waiting, so it was refused"));
		Answer(false);
	}

	// Cannot normally happen - the button is disabled without one - but a
	// shortcut or an accessibility tool can still reach a disabled button on
	// some platforms, and an empty submission window would be the result.
	if (!HasPendingReport())
		return;

	hide();
	OfferPendingReport();
	// The connection is left as it is. Finishing is about the report; whether
	// the agent may still reach this machine is a separate question, and the
	// user is asked it when they close the window or answers it with the
	// toolbar button.
}

void CMcpSessionWindow::OnCancel()
{
	// The same thing as the X, deliberately: two ways to say stop that behaved
	// differently would be two things to learn.
	CloseSession();
}

CMcpSessionWindow::EEnd CMcpSessionWindow::AskHowToEnd()
{
	// Only once there has been a session to end. An agent that connected and
	// was never let in has interacted with nobody: closing the window on it is
	// the refusal, and asking "are you sure" about a refusal gets the one
	// answer nobody wants to give twice.
	//
	// A question on screen counts whatever else is true: something is waiting
	// on an answer and closing decides it.
	//
	// When this is not asked, the connection is left exactly as it was. The
	// window is not the only way to start an agent, so closing it must not be a
	// hidden way to stop one.
	if (!m_bPrompting && !HasPendingReport() && !m_bHadAccess)
		return eCloseKeepServer;

	QMessageBox Box(this);
	Box.setWindowTitle("Sandboxie-Plus");
	Box.setIcon(QMessageBox::Question);
	Box.setText(tr("Close this window - and the agent connection with it?"));

	QString Detail;
	if (HasPendingReport())
		Detail = tr(
			"The agent has written up what it found, and closing does not send it. Finish instead "
			"- the green tick - to read it and decide; it is the only way what happened here "
			"reaches anyone else. Nothing is lost either way: the write-up is kept, and you can "
			"reopen this window and finish later.\n\n");
	else if (m_bPrompting)
		Detail = tr("A request is waiting for an answer, and closing refuses it.\n\n");

	// The part that is actually being asked, every time: what happens to the
	// connection. Spelled out rather than implied, because the two closes look
	// identical afterwards and only one of them can be undone by waiting.
	Detail += tr(
		"Disconnecting closes the agent connection as well: nothing can connect again until "
		"you turn it back on. Staying connected leaves it open, so the agent can carry on and "
		"this window comes back the next time it needs you.");
	Box.setInformativeText(Detail);

	QPushButton* pStop = Box.addButton(tr("Close and Disconnect"), QMessageBox::DestructiveRole);
	QPushButton* pClose = Box.addButton(tr("Close, Stay Connected"), QMessageBox::AcceptRole);
	QPushButton* pStay = Box.addButton(tr("Keep Open"), QMessageBox::RejectRole);
	// Keep Open by default: a stray return key must not disconnect an agent in
	// the middle of something.
	Box.setDefaultButton(pStay);
	Box.exec();

	if (Box.clickedButton() == pStop)
		return eCloseAndStop;
	if (Box.clickedButton() == pClose)
		return eCloseKeepServer;
	return eStayOpen;
}

bool CMcpSessionWindow::CloseSession()
{
	const EEnd How = AskHowToEnd();
	if (How == eStayOpen)
		return false;

	if (m_bPrompting) {
		// Closing the window is an answer, and the safe reading of it is no.
		AddEntry(eRefused, tr("the window was closed while a request was waiting, so it was refused"));
		Answer(false);
	}
	if (HasPendingReport())
		AddEntry(eInfo, tr("the window was closed, so the write-up was not sent"));

	// Hidden rather than destroyed: the history is worth keeping for the rest
	// of the session, and the next request brings it back - as does Finish, so
	// a write-up that was not sent now can still be sent later.
	hide();

	if (How == eCloseAndStop && theGUI && theGUI->IsMcpServerRunning()) {
		// Which resets this window, through StopMcpServer - so nothing of this
		// session is waiting for the next one.
		theGUI->StopMcpServer();
	}
	return true;
}

void CMcpSessionWindow::OfferPendingReport()
{
	if (m_PendingReport.isEmpty())
		return;

	// What was actually put in front of the user, remembered so that a draft
	// the agent writes while the dialog is open cannot be thrown away as
	// though it had been the one that was sent.
	const QString Shown = m_PendingReport;

	// Parented to the main window rather than to this one, which is hidden by
	// now and would give the dialog nowhere to sit. The dialog asks the outcome
	// itself when the agent never established it.
	CMcpSubmitDialog Dialog(m_PendingClient, Shown, GetActionLog(), m_bPendingIni,
		m_bPendingTrace, !m_bOutcomeConfirmed, theGUI);

	if (Dialog.exec() == QDialog::Accepted) {
		AddEntry(eAction, tr("the report was sent to the Sandboxie-Plus issue tracker"));

		// Spent now, and only this one: an agent that composed something newer
		// while the dialog was open keeps its newer draft.
		if (m_PendingReport == Shown) {
			ClearPendingReport();
			UpdateFinish();
		}
	}
	else {
		// Kept, which is the whole difference. It used to be discarded here, on
		// the reasoning that a user who said no should not be asked twice - but
		// nothing asks on its own any more: this dialog only appears because
		// somebody pressed Finish. Throwing the write-up away made "Don't Send"
		// mean "destroy it", which is not what it says, and left the window
		// showing a history of work with no way to report it.
		AddEntry(eInfo, tr("the report was not sent - press Finish again if you change your mind"));
	}
}
