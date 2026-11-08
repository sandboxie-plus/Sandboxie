#include "stdafx.h"
#include "McpConnectDialog.h"
#include "McpSession.h"
#include "../SandMan.h"
#include <QListWidget>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>
#include <QDesktopServices>

CMcpConnectDialog::CMcpConnectDialog(QWidget* parent)
 : QDialog(parent)
{
	setWindowTitle(tr("Connect an AI Agent"));
	resize(640, 520);

	QVBoxLayout* pLayout = new QVBoxLayout(this);

	QLabel* pIntro = new QLabel(tr(
		"An AI agent can read your sandbox configuration, the bundled documentation and "
		"diagnostic data, and help you work out why something does not run in a sandbox.\n\n"
		"It is off until you turn it on, it is not reachable from inside a sandbox, and it has "
		"to be authorised with a code shown on your screen before it can do anything."), this);
	pIntro->setWordWrap(true);
	pLayout->addWidget(pIntro);

	m_pStatus = new QLabel(this);
	m_pStatus->setWordWrap(true);
	pLayout->addWidget(m_pStatus);

	pLayout->addWidget(new QLabel(tr("Which client do you want to use?"), this));
	m_pList = new QListWidget(this);
	pLayout->addWidget(m_pList, 1);

	m_pExplain = new QLabel(this);
	m_pExplain->setWordWrap(true);
	m_pExplain->setTextInteractionFlags(Qt::TextSelectableByMouse);
	pLayout->addWidget(m_pExplain);

	m_pSnippet = new QPlainTextEdit(this);
	m_pSnippet->setReadOnly(true);
	m_pSnippet->setMaximumHeight(130);
	m_pSnippet->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	pLayout->addWidget(m_pSnippet);

	QHBoxLayout* pButtons = new QHBoxLayout();
	m_pConfigure = new QPushButton(tr("Configure It For Me"), this);
	m_pLaunch = new QPushButton(tr("Start It For Me"), this);
	m_pCopy = new QPushButton(tr("Copy"), this);
	m_pToggle = new QPushButton(this);
	m_pForget = new QPushButton(this);
	pButtons->addWidget(m_pConfigure);
	pButtons->addWidget(m_pLaunch);
	pButtons->addWidget(m_pCopy);
	pButtons->addStretch();
	pButtons->addWidget(m_pForget);
	pButtons->addWidget(m_pToggle);
	pLayout->addLayout(pButtons);

	QDialogButtonBox* pBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
	connect(pBox, &QDialogButtonBox::rejected, this, &QDialog::accept);
	pLayout->addWidget(pBox);

	connect(m_pList, &QListWidget::currentRowChanged, this, &CMcpConnectDialog::OnClientChanged);
	connect(m_pConfigure, &QPushButton::clicked, this, &CMcpConnectDialog::OnConfigure);
	connect(m_pLaunch, &QPushButton::clicked, this, &CMcpConnectDialog::OnLaunch);
	connect(m_pCopy, &QPushButton::clicked, this, &CMcpConnectDialog::OnCopy);
	connect(m_pToggle, &QPushButton::clicked, this, &CMcpConnectDialog::OnToggleServer);
	connect(m_pForget, &QPushButton::clicked, this, &CMcpConnectDialog::OnForgetRemembered);

	Refresh();
	if (m_pList->count())
		m_pList->setCurrentRow(0);
}

void CMcpConnectDialog::Refresh()
{
	const int Row = m_pList->currentRow();

	m_Clients = CMcpClientSetup::Detect();

	CMcpClientSetup::SortFoundFirst(m_Clients);

	m_pList->clear();
	foreach(const CMcpClientSetup::SClient& Client, m_Clients) {
		QString Label = Client.Name;
		if (CMcpClientSetup::IsConfigured(Client))
			Label += tr("   (found, already configured)");
		else if (Client.bDetected && !Client.bFallback)
			Label += tr("   (found)");
		else if (!Client.bDetected)
			Label += tr("   (not found)");
		m_pList->addItem(Label);
	}
	if (Row >= 0 && Row < m_pList->count())
		m_pList->setCurrentRow(Row);

	const bool bRunning = theGUI->IsMcpServerRunning();
	m_pToggle->setText(bRunning ? tr("Turn Off") : tr("Turn On"));
	m_pToggle->setEnabled(CMcpClientSetup::IsShimAvailable());

	m_pStatus->setText(bRunning
		? tr("The connection is ON. A client can connect now.")
		: tr("The connection is OFF. Turn it on here, or let the client ask for it - "
			 "Sandboxie-Plus will offer when something tries to connect."));

	const int Remembered = CMcpClients::GetRemembered().count();
	m_pForget->setVisible(Remembered > 0);
	m_pForget->setText(tr("Forget %1 Remembered Program(s)", "", Remembered).arg(Remembered));

	if (!CMcpClientSetup::IsShimAvailable()) {
		m_pStatus->setText(tr("SbieMcp.exe was not found next to SandMan, so this installation "
			"cannot accept agent connections."));
	}

	OnClientChanged();
}

CMcpClientSetup::SClient CMcpConnectDialog::CurrentClient() const
{
	const int Row = m_pList->currentRow();
	if (Row < 0 || Row >= m_Clients.count())
		return CMcpClientSetup::SClient();
	return m_Clients.at(Row);
}

void CMcpConnectDialog::OnClientChanged()
{
	const CMcpClientSetup::SClient Client = CurrentClient();
	const bool bHaveShim = CMcpClientSetup::IsShimAvailable();

	switch (Client.Kind)
	{
		case CMcpClientSetup::eWriteJson:
			m_pExplain->setText(tr("Configuration file:\n%1\n\n"
				"Your other servers are kept; only the Sandboxie entry is added or updated, and "
				"the file is backed up first.").arg(Client.ConfigPath));
			m_pSnippet->setPlainText(CMcpClientSetup::GetSnippet());
			m_pConfigure->setVisible(true);
			m_pConfigure->setEnabled(bHaveShim);
			m_pConfigure->setText(CMcpClientSetup::IsConfigured(Client) ? tr("Update It For Me") : tr("Configure It For Me"));
			break;

		case CMcpClientSetup::eCommand:
			m_pExplain->setText(Client.bCanLaunch
				? tr("%1\n\nOr let Sandboxie-Plus start it for you, in a folder of its own:\n%2\n\n"
					"The configuration written there reaches that one session and nothing else "
					"you do with it.").arg(Client.Note).arg(CMcpClientSetup::GetAgentFolder())
				: Client.Note);
			m_pSnippet->setPlainText(Client.Command);
			m_pConfigure->setVisible(false);
			break;

		default:
			m_pExplain->setText(Client.Note);
			m_pSnippet->setPlainText(CMcpClientSetup::GetSnippet());
			m_pConfigure->setVisible(false);
			break;
	}

	m_pLaunch->setVisible(Client.bCanLaunch);
	m_pLaunch->setEnabled(bHaveShim);

	m_pCopy->setEnabled(bHaveShim && !m_pSnippet->toPlainText().isEmpty());
}

void CMcpConnectDialog::OnConfigure()
{
	const CMcpClientSetup::SClient Client = CurrentClient();

	QString Error, Backup;
	if (!CMcpClientSetup::WriteConfig(Client, &Error, &Backup)) {
		QMessageBox::warning(this, "Sandboxie-Plus", Error);
		return;
	}

	QString Message = tr("%1 is configured. Restart it if it is running, so it picks up the change.").arg(Client.Name);
	if (!Backup.isEmpty())
		Message += tr("\n\nThe previous configuration was kept as:\n%1").arg(Backup);
	QMessageBox::information(this, "Sandboxie-Plus", Message);

	theGUI->OnLogMessage(tr("MCP: wrote the agent configuration for %1").arg(Client.Name));
	Refresh();
}

void CMcpConnectDialog::OnLaunch()
{
	const CMcpClientSetup::SClient Client = CurrentClient();

	// Started, so it has somewhere to connect to. Doing this silently is the
	// right call here: the user asked for the client to be launched, and
	// handing them a terminal that cannot reach us would be a worse surprise
	// than turning the listener on.
	if (!theGUI->IsMcpServerRunning())
		theGUI->StartMcpServer();

	QString Error, Folder;
	if (!CMcpClientSetup::LaunchClaudeCode(&Error, &Folder)) {
		QMessageBox::warning(this, "Sandboxie-Plus", Error);
		Refresh();
		return;
	}

	theGUI->OnLogMessage(tr("MCP: started %1 in %2").arg(Client.Name).arg(Folder));

	// Done with: the agent is starting and the session window is what the user
	// watches next. Leaving this dialog in front of it would make them close a
	// window they had finished with to see the one that matters.
	accept();
}

void CMcpConnectDialog::OnCopy()
{
	QApplication::clipboard()->setText(m_pSnippet->toPlainText());
	m_pCopy->setText(tr("Copied"));
	QTimer::singleShot(1500, this, [this]() { m_pCopy->setText(tr("Copy")); });
}

void CMcpConnectDialog::OnToggleServer()
{
	theGUI->OnToggleMcpServer();
	Refresh();
}

void CMcpConnectDialog::OnForgetRemembered()
{
	const QStringList Remembered = CMcpClients::GetRemembered();
	if (QMessageBox::question(this, "Sandboxie-Plus",
			tr("Forget these programs? Each will have to be authorised with a code again.\n\n%1")
				.arg(Remembered.join("\n")),
			QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
		return;

	CMcpClients::ForgetAll();
	theGUI->OnLogMessage(tr("MCP: the remembered agent programs were forgotten"));
	Refresh();
}
