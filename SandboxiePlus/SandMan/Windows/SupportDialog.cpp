#include "stdafx.h"
#include "SupportDialog.h"
#include "SandMan.h"
#include "../MiscHelpers/Common/Common.h"
#include "SettingsWindow.h"
#include <Windows.h>
#include "OnlineUpdater.h"

bool CSupportDialog::m_ReminderShown = false;

QDateTime CSupportDialog::GetSbieInstallationDate()
{
	time_t InstalDate = 0;
	theAPI->GetSecureParam("InstallationDate", &InstalDate, sizeof(InstalDate));

	time_t CurrentDate = QDateTime::currentDateTimeUtc().toSecsSinceEpoch();
	if (InstalDate == 0 || InstalDate > CurrentDate) {
		InstalDate = CurrentDate;
		theAPI->SetSecureParam("InstallationDate", &InstalDate, sizeof(InstalDate));
	}

	return QDateTime::fromSecsSinceEpoch(InstalDate);
}

bool CSupportDialog::IsBusinessUse()
{
	if (CERT_IS_TYPE(g_CertInfo, eCertBusiness))
		return true;
    uchar UsageFlags = 0;
	theAPI->GetSecureParam("UsageFlags", &UsageFlags, sizeof(UsageFlags));
	return (UsageFlags & 1) != 0;
}

bool CSupportDialog::CheckSupport(bool bOnRun)
{
	bool NoGo = false;

#ifdef INSIDER_BUILD
	if (g_CertInfo.active) {
		if (!CERT_IS_INSIDER(g_CertInfo)) {
			TArguments args = GetArguments(g_Certificate, L'\n', L':');
			if (args.value("TYPE").contains("PATREON")) {
				if (QMessageBox::question(NULL, "Sandboxie-Plus", tr("This Insider build requires a special certificate of type GREAT_PATREON, PERSONAL-HUGE, or CONTRIBUTOR.\n"
					"If you are a Great Supporter on Patreon already, Sandboxie can check online for an update of your certificate."), QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok) {
					theGUI->m_pUpdater->UpdateCert(true);
					if (CERT_IS_INSIDER(g_CertInfo))
						return false;
				}
			}
			else
				QMessageBox::warning(NULL, "Sandboxie-Plus", tr("This Insider build requires a special certificate of type GREAT_PATREON, PERSONAL-HUGE, or CONTRIBUTOR."));
		}
		else
			return false;
	}

	NoGo = true;
#else
	if (g_CertInfo.active)
		return false;

	QDateTime InstallDate = GetSbieInstallationDate();
	bool bOnARM64 = (g_FeatureFlags & CSbieAPI::eSbieFeatureARM64) != 0;

	QDateTime CurretnDate = QDateTime::currentDateTimeUtc();
	int Days = InstallDate.daysTo(CurretnDate);

	BYTE CertBlocked = 0;
	theAPI->GetSecureParam("CertBlocked", &CertBlocked, sizeof(CertBlocked));
	if (CertBlocked)
	{
		QString Message = tr("An attempt was made to use a blocked certificate on this system. This action violates the terms of use for the support certificate. "
			"You must now purchase a valid certificate, as the usage of the free version has been restricted.");

		CSupportDialog dialog(Message, true);
		if(dialog.exec() == QDialog::Rejected)
			PostQuitMessage(0);
		return true;
	}

	if (Days < 40)
		return false;

	if (IsBusinessUse())
	{
		if (g_CertInfo.expired)
			Days = -g_CertInfo.expirers_in_sec / (24 * 3600);

		NoGo = (Days > 60);

		if (!NoGo && m_ReminderShown && !bOnRun)
			return false;
	}
	else if (!bOnARM64 && !theAPI->GetGlobalSettings()->GetBool("AlwaysShowReminder"))
	{
		// Note: the old sandboxie showed a message after 30 days every 12 hours for 5 seconds
		
		int Interval = 30 * 24; // in hours

		USHORT ReminderShedule[2 * 11] = {
		//  days,	itnerval,	
			730,	     12,
			365,	 1 * 24,
			182,	 5 * 24,
			30,		30 * 24,
			0
		};
		USHORT CurReminderRevision = 1;

		USHORT ReminderRevision = 0;
		theAPI->GetSecureParam("ReminderRevision", &ReminderRevision, sizeof(ReminderRevision));
		if (ReminderRevision < CurReminderRevision) {
			theAPI->SetSecureParam("ReminderShedule", &ReminderShedule, sizeof(ReminderShedule));
			theAPI->SetSecureParam("ReminderRevision", &CurReminderRevision, sizeof(CurReminderRevision));
		}
		else if (ReminderRevision > CurReminderRevision)
			theAPI->GetSecureParam("ReminderShedule", &ReminderShedule, sizeof(ReminderShedule));

		for (USHORT* Cur = ReminderShedule; (ULONG_PTR)Cur < (ULONG_PTR)ReminderShedule + sizeof(ReminderShedule) && *Cur != 0; Cur += 2) {
			if (Days > Cur[0]) {
				if (Cur[1] < Interval) Interval = Cur[1];
				break;
			}
		}

		time_t LastReminder = 0;
		theAPI->GetSecureParam("LastReminder", &LastReminder, sizeof(LastReminder));
		if (LastReminder > 0 && LastReminder < CurretnDate.toSecsSinceEpoch()) {
			if (CurretnDate.toSecsSinceEpoch() - LastReminder < (time_t(Interval) * 3600))
				return false;
		}

		//ULONG ReminderConfig = 0;
		//theAPI->GetSecureParam("ReminderConfig", &ReminderConfig, sizeof(ReminderConfig));

		if ((rand() % 5) != 0)
			return false;
	}
	m_ReminderShown = true;
#endif

	if (!ShowDialog(NoGo))
		PostQuitMessage(0);

	return true;
}

extern int g_CertAmount;
int CountSeats();

bool CSupportDialog::ShowDialog(bool NoGo, int Wait)
{
	QDateTime InstallDate = GetSbieInstallationDate();

	int Days = InstallDate.daysTo(QDateTime::currentDateTimeUtc());

	QString Message;

#ifdef INSIDER_BUILD
	if (!CERT_IS_INSIDER(g_CertInfo))
	{
		Message += tr("This is a <a href=\"https://sandboxie-plus.com/go.php?to=sbie-insider\">exclusive Insider build</a> of Sandboxie-Plus it is only available to <a href=\"https://sandboxie-plus.com/go.php?to=patreon\">Patreon Supporters</a> on higher tiers as well as to project contributors and owners of a HUGE supporter certificate.");
	}
	else
#endif
	if (CountSeats() > g_CertAmount)
	{
		Message = tr("The installed supporter certificate allows for <b>%1 seats</b> to be active.<br /><br />").arg(g_CertAmount);

		Message += tr("<b>There seems to be however %1 Sandboxie-Plus instances on your network, <font color='red'>you need to obtain additional <a href=\"https://sandboxie-plus.com/go.php?to=sbie-obtain-cert&tip=more\">support certificates</a></font>.</b><br /><br />").arg(CountSeats());
	}
	else if (IsBusinessUse()) 
	{
		if (g_CertInfo.expired) {
			Days = -g_CertInfo.expirers_in_sec / (24 * 3600);

			Message = tr("The installed supporter certificate <b>has expired %1 days ago</b> and <a href=\"https://sandboxie-plus.com/go.php?to=sbie-renew-cert\">must be renewed</a>.<br /><br />").arg(Days);
		} else
			Message = tr("<b>You have installed Sandboxie-Plus more than %1 days ago.</b><br /><br />").arg(Days);

		Message += tr("<u>Commercial use of Sandboxie past the evaluation period</u>, requires a valid <a href=\"https://sandboxie-plus.com/go.php?to=sbie-obtain-cert\">support certificate</a>.").arg(Days);
	}
	else
	{
		bool bOnARM64 = (g_FeatureFlags & CSbieAPI::eSbieFeatureARM64) != 0;

		if (g_CertInfo.outdated)
			Message = tr("The installed supporter certificate is <b>outdated</b> and it is <u>not valid for<b> this version</b></u> of Sandboxie-Plus.<br /><br />");
		else if (g_CertInfo.expired)
			Message = tr("The installed supporter certificate is <b>expired</b> and <u>should be renewed</u>.<br /><br />");
		else
			Message = tr("<b>You have been using Sandboxie-Plus for more than %1 days now.</b><br /><br />").arg(Days);

		if (bOnARM64)
			Message += tr("Sandboxie on ARM64 requires a valid supporter certificate for continued use.<br /><br />");
		else
			Message += tr("Personal use of Sandboxie is free of charge on x86/x64, although some functionality is only available to project supporters.<br /><br />");

		if (g_CertInfo.expired)
			Message += tr("Please continue <a href=\"https://sandboxie-plus.com/go.php?to=sbie-renew-cert\">supporting the project</a> "
				"by renewing your <a href=\"https://sandboxie-plus.com/go.php?to=sbie-cert\">supporter certificate</a> and continue using the <b>enhanced functionality</b> in new builds.");
		else
			Message += tr("Sandboxie <u>without</u> a valid supporter certificate will sometimes <b><font color='red'>pause for a few seconds</font></b>. This pause allows you to consider <a href=\"https://sandboxie-plus.com/go.php?to=sbie-obtain-cert\">purchasing a supporter certificate</a> or <a href=\"https://sandboxie-plus.com/go.php?to=sbie-contribute\">earning one by contributing</a> to the project. <br /><br />"
				"A <a href=\"https://sandboxie-plus.com/go.php?to=sbie-cert\">supporter certificate</a> not just removes this reminder, but also enables <b>exclusive enhanced functionality</b> providing better security and compatibility.");
	}

	time_t LastReminder = QDateTime::currentDateTimeUtc().toSecsSinceEpoch();
	theAPI->SetSecureParam("LastReminder", &LastReminder, sizeof(LastReminder));

	CSupportDialog dialog(Message, NoGo, Wait);
	return dialog.exec() != QDialog::Rejected;
}

CSupportDialog::CSupportDialog(const QString& Message, bool NoGo, int Wait, QWidget *parent)
	: QDialog(parent)
{
	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::CustomizeWindowHint;
	//flags &= ~Qt::WindowContextHelpButtonHint;
	//flags &= ~Qt::WindowSystemMenuHint;
	//flags &= ~Qt::WindowMinMaxButtonsHint;
	//flags |= Qt::WindowMinimizeButtonHint;
	//flags &= ~Qt::WindowCloseButtonHint;
	flags &= ~Qt::WindowContextHelpButtonHint;
	//flags &= ~Qt::WindowSystemMenuHint;
	setWindowFlags(flags);

	//setWindowState(Qt::WindowActive);
	SetForegroundWindow((HWND)QWidget::winId());

	this->setWindowFlag(Qt::WindowStaysOnTopHint, theGUI->IsAlwaysOnTop());

	this->setWindowTitle(tr("Sandboxie-Plus - Support Reminder"));

	this->resize(420, 300);
	QVBoxLayout* verticalLayout = new QVBoxLayout(this);
	QGridLayout* gridLayout = new QGridLayout();

	QHBoxLayout* horizontalLayout = new QHBoxLayout();
	m_Buttons[0] = new QPushButton(this);
	horizontalLayout->addWidget(m_Buttons[0]);
	m_Buttons[1] = new QPushButton(this);
	horizontalLayout->addWidget(m_Buttons[1]);
	m_Buttons[2] = new QPushButton(this);
	horizontalLayout->addWidget(m_Buttons[2]);
	gridLayout->addLayout(horizontalLayout, 2, 0, 1, 1);

	QLabel* label = new QLabel(this);
	QSizePolicy sizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	sizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
	label->setSizePolicy(sizePolicy);
	label->setAlignment(Qt::AlignCenter);
	label->setWordWrap(true);
	label->setText(Message);
	connect(label, SIGNAL(linkActivated(const QString&)), theGUI, SLOT(OpenUrl(const QString&)));
	gridLayout->addWidget(label, 1, 0, 1, 1);

	QLabel* logo = new QLabel(this);
	logo->setFrameShape(QFrame::StyledPanel);
	logo->setFrameShadow(QFrame::Plain);
	//QPalette back = logo->palette();
	//SetPaleteTexture(back, QPalette::Window, QImage(":/Assets/sandboxie-back.png"));
	//logo->setPalette(back);
	//logo->setAutoFillBackground(true);
	logo->setStyleSheet("background-image: url(:/Assets/sandboxie-back.png);");
	logo->setPixmap(QPixmap::fromImage(QImage(":/Assets/sandboxie-logo.png")));
	logo->setAlignment(Qt::AlignCenter);
	gridLayout->addWidget(logo, 0, 0, 1, 1);

	verticalLayout->addLayout(gridLayout);


	for (int s = rand() % 3,  i = 0; i < 3; i++)
	{
		QPushButton* pButton = m_Buttons[(s + i) % 3];
		pButton->setAutoDefault(false);
		//pButton->setEnabled(false);
		pButton->setProperty("Action", (i == 1 && NoGo) ? 3 : i);
		connect(pButton, SIGNAL(clicked(bool)), this, SLOT(OnButton()));
	}

	m_CountDown = NoGo ? 0 : Wait;

	UpdateButtons();

	m_uTimerID = startTimer(1000);
}

CSupportDialog::~CSupportDialog()
{
	killTimer(m_uTimerID);
}

void CSupportDialog::timerEvent(QTimerEvent* pEvent)
{
	if (pEvent->timerId() != m_uTimerID)
		return;

	if (m_CountDown >= 0)
		UpdateButtons();
}

void CSupportDialog::UpdateButtons()
{
	if (m_CountDown-- > 0) 
		m_Buttons[1]->setText(tr("%1").arg(m_CountDown + 1));
	else 
	{
		for (int i = 0; i < 3; i++) {
			QPushButton* pButton = m_Buttons[i];
			//pButton->setEnabled(true);
			switch (pButton->property("Action").toInt()) {
			case 0:	pButton->setText(tr("Quit")); break;
			case 1:	pButton->setText(tr("Continue")); break;
			case 2: pButton->setText(tr("Get Certificate")); break;
			case 3: pButton->setText(tr("Enter Certificate")); break;
			}
		}
	}
}

void CSupportDialog::OnButton()
{
	int Action = ((QPushButton*)sender())->property("Action").toInt();

	if (Action == 3) {
		CSettingsWindow* pSettingsWindow = new CSettingsWindow(this);
		pSettingsWindow->showTab("Support", true);
		connect(pSettingsWindow, &CSettingsWindow::Closed, [this]() {
#ifdef INSIDER_BUILD
			if (g_CertInfo.active && !CERT_IS_INSIDER(g_CertInfo)) {
				TArguments args = GetArguments(g_Certificate, L'\n', L':');
				if (args.value("TYPE").contains("PATREON")) {
					theGUI->m_pUpdater->UpdateCert(true);
					if (CERT_IS_INSIDER(g_CertInfo)) {
						accept();
						return;
					}
				}

				QMessageBox::warning(this, "Sandboxie-Plus", tr("This Insider build requires a special certificate of type GREAT_PATREON, PERSONAL-HUGE, or CONTRIBUTOR."));
				return;
			}
#endif
			if (g_CertInfo.active)
				accept();
		});
	}
	else if (Action == 2) 
		return theGUI->OpenUrl(QUrl("https://sandboxie-plus.com/go.php?to=sbie-get-cert"));
	else if (Action == 1)
		accept();
	else
		reject();
}
