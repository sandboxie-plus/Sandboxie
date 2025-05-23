#include "stdafx.h"

#include "SetupWizard.h"
#include "../MiscHelpers/Common/Common.h"
#include "../Windows/SettingsWindow.h"
#include "../SandMan.h"
#include "Helpers/WinAdmin.h"
#include <QButtonGroup>
#include "../QSbieAPI/SbieUtils.h"
#include "../OnlineUpdater.h"

CSetupWizard::CSetupWizard(int iOldLevel, QWidget *parent)
    : QWizard(parent)
{
    if (iOldLevel < SETUP_LVL_1)
        setPage(Page_Intro, new CIntroPage);
    if (iOldLevel < SETUP_LVL_3)
        setPage(Page_Certificate, new CCertificatePage(iOldLevel));
    if (iOldLevel < SETUP_LVL_1) {
        setPage(Page_UI, new CUIPage);
        setPage(Page_Shell, new CShellPage);
        //setPage(Page_WFP, new CWFPPage);
    }
    if (iOldLevel < SETUP_LVL_2)
        setPage(Page_Update, new CSBUpdate);
    setPage(Page_Finish, new CFinishPage);

    setWizardStyle(ModernStyle);
    //setOption(HaveHelpButton, true);
    setPixmap(QWizard::LogoPixmap, QPixmap(":/SandMan.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    connect(this, &QWizard::helpRequested, this, &CSetupWizard::showHelp);

    setWindowTitle(tr("Setup Wizard"));
}

void CSetupWizard::showHelp()
{
    static QString lastHelpMessage;

    QString message;

    switch (currentId()) {
    case Page_Intro:
        message = tr("The decision you make here will affect which page you get to see next.");
        break;
    default:
        message = tr("This help is likely not to be of any help.");
    }

    if (lastHelpMessage == message)
        message = tr("Sorry, I already gave all the help I could.");

    QMessageBox::information(this, tr("Setup Wizard Help"), message);

    lastHelpMessage = message;
}

bool CSetupWizard::ShowWizard(int iOldLevel)
{
    CSetupWizard wizard(iOldLevel, theGUI);
    if (!theGUI->SafeExec(&wizard))
        return false;
    
    if (iOldLevel < SETUP_LVL_1) {
        //bool useBusiness = wizard.field("useBusiness").toBool();
        //QString Certificate = wizard.field("useCertificate").toString();
        //bool isEvaluate = wizard.field("isEvaluate").toBool();

        if (wizard.field("useAdvanced").toBool())
            theConf->SetValue("Options/ViewMode", 1);
        else if (wizard.field("useSimple").toBool())
            theConf->SetValue("Options/ViewMode", 0);
        else if (wizard.field("useClassic").toBool())
            theConf->SetValue("Options/ViewMode", 2);

        if (wizard.field("useBrightMode").toInt())
            theConf->SetValue("Options/UseDarkTheme", 0);
        else if (wizard.field("useDarkMode").toInt())
            theConf->SetValue("Options/UseDarkTheme", 1);

        AutorunEnable(wizard.field("isAutoStart").toBool());

        if (wizard.field("useContecxtMenu").toBool())
            CSettingsWindow::AddContextMenu();

        if (wizard.field("useBrowserIcon").toBool())
            CSettingsWindow::AddBrowserIcon();

        //if (wizard.field("useWFP").toBool()) {
        //    theAPI->GetGlobalSettings()->SetBool("NetworkEnableWFP", true);
        //    theAPI->ReloadConfig(true);
        //}
    }

    if (iOldLevel < SETUP_LVL_2) {

        if (wizard.field("updateAll").toBool())
        {
            theConf->SetValue("Options/CheckForUpdates", 1);
            theConf->SetValue("Options/OnNewUpdate", "install");
            //theConf->SetValue("Options/CheckForTemplates", 1);
            theConf->SetValue("Options/CheckForIssues", 1);
            theConf->SetValue("Options/CheckForAddons", 1);
        }
        else
        {
            if(wizard.field("updateApp").toBool())
                theConf->SetValue("Options/CheckForUpdates", 1);
            if(wizard.field("applyHotfixes").toBool())
                theConf->SetValue("Options/OnNewUpdate", "install");
            //if(wizard.field("updateCompat").toBool())
            //    theConf->SetValue("Options/CheckForTemplates", 1);
            if(wizard.field("updateIssues").toBool())
                theConf->SetValue("Options/CheckForIssues", 1);
            if(wizard.field("updateAddons").toBool())
                theConf->SetValue("Options/CheckForAddons", 1);
        }

        if (wizard.field("updateAll").toBool() || wizard.field("updateApp").toBool()) {
            QString ReleaseChannel;
            if (wizard.field("channelStable").toBool())
                ReleaseChannel = "stable";
            else if (wizard.field("channelPreview").toBool())
                ReleaseChannel = "preview";
            else if (wizard.field("channelInsider").toBool())
                ReleaseChannel = "insider";
            if (!ReleaseChannel.isEmpty()) theConf->SetValue("Options/ReleaseChannel", ReleaseChannel);
        }
    }

    //if (wizard.field("isUpdate").toBool())
    //    theConf->SetValue("Options/CheckForUpdates", 1);

    if (iOldLevel < SETUP_LVL_1) 
    {
        if (wizard.field("editAdminOnly").toBool())
            theAPI->GetGlobalSettings()->SetText("EditAdminOnly", "y");
    }

    theConf->SetValue("Options/WizardLevel", SETUP_LVL_CURRENT);

    theGUI->UpdateSettings(true);
    
    return true;
}

void CSetupWizard::ShellUninstall()
{
    AutorunEnable(false);

	CSettingsWindow::RemoveContextMenu();
	CSbieUtils::RemoveContextMenu2();
	CSbieUtils::RemoveContextMenu3();
	CSbieUtils::RemoveContextMenu4();

    // todo: delete desktop browser shortcut and start menu integration
}

//////////////////////////////////////////////////////////////////////////////////////////
// CIntroPage
// 

CIntroPage::CIntroPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Introduction"));
    QPixmap Logo = QPixmap(theGUI->m_DarkTheme ? ":/SideLogoDM.png" : ":/SideLogo.png");
    int Scaling = theConf->GetInt("Options/FontScaling", 100);
    if(Scaling !=  100) Logo = Logo.scaled(Logo.width() * Scaling / 100, Logo.height() * Scaling / 100);
    setPixmap(QWizard::WatermarkPixmap, Logo);

    QVBoxLayout *layout = new QVBoxLayout;
    QLabel* pTopLabel = new QLabel(tr("Welcome to the Setup Wizard. This wizard will help you to configure your copy of <b>Sandboxie-Plus</b>. "
        "You can start this wizard at any time from the Sandbox->Maintenance menu if you do not wish to complete it now."));
    pTopLabel->setWordWrap(true);
    layout->addWidget(pTopLabel);

    QWidget* pSpace = new QWidget();
    pSpace->setMinimumHeight(16);
    layout->addWidget(pSpace);

    m_pLabel = new QLabel(tr("Select how you would like to use Sandboxie-Plus"));
    layout->addWidget(m_pLabel);

    m_pPersonal = new QRadioButton(tr("&Personally, for private non-commercial use"));
    layout->addWidget(m_pPersonal);
    connect(m_pPersonal, SIGNAL(toggled(bool)), this, SIGNAL(completeChanged()));
    registerField("usePersonal", m_pPersonal);

    m_pBusiness = new QRadioButton(tr("&Commercially, for business or enterprise use"));
    layout->addWidget(m_pBusiness);
    connect(m_pBusiness, SIGNAL(toggled(bool)), this, SIGNAL(completeChanged()));
    registerField("useBusiness", m_pBusiness);

    QLabel* pNote = new QLabel(tr("Note: this option is persistent"));
    layout->addWidget(pNote);

    uchar BusinessUse = 2;
    if (!g_Certificate.isEmpty())
        BusinessUse = CERT_IS_TYPE(g_CertInfo, eCertBusiness) ? 1 : 0;
    else {
        uchar UsageFlags = 0;
        if (theAPI->GetSecureParam("UsageFlags", &UsageFlags, sizeof(UsageFlags)))
            BusinessUse = (UsageFlags & 1) != 0 ? 1 : 0;
    }
    if (BusinessUse != 2) {
        m_pPersonal->setChecked(BusinessUse == 0);
        m_pBusiness->setChecked(BusinessUse == 1);
        if ((QApplication::keyboardModifiers() & Qt::ControlModifier) == 0) {
            m_pLabel->setEnabled(false);
            m_pPersonal->setEnabled(false);
            m_pBusiness->setEnabled(false);
        }
        pNote->setEnabled(false);
    }

    setLayout(layout);

    if (theGUI->m_DarkTheme) {
        QPalette palette = this->palette();
        palette.setColor(QPalette::Base, QColor(53, 53, 53));
        this->setPalette(palette);
    }
}

int CIntroPage::nextId() const
{
    if(g_Certificate.isEmpty())
        return CSetupWizard::Page_Certificate;
    return CSetupWizard::Page_UI;
}

bool CIntroPage::isComplete() const 
{
    if (m_pLabel->isEnabled() && !m_pPersonal->isChecked() && !m_pBusiness->isChecked())
        return false;
    return QWizardPage::isComplete();
}

//////////////////////////////////////////////////////////////////////////////////////////
// CCertificatePage
// 

CCertificatePage::CCertificatePage(int iOldLevel, QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Install your <b>Sandboxie-Plus</b> support certificate"));
    setSubTitle(tr("If you have a supporter certificate, please fill it into the field below."));
    
    if (iOldLevel < SETUP_LVL_1)
        m_NextPage = CSetupWizard::Page_UI;
    else if (iOldLevel < SETUP_LVL_3)
        m_NextPage = CSetupWizard::Page_Finish;

    QGridLayout *layout = new QGridLayout;

    m_pTopLabel = new QLabel();
    m_pTopLabel->setWordWrap(true);
    connect(m_pTopLabel, SIGNAL(linkActivated(const QString&)), theGUI, SLOT(OpenUrl(const QString&)));
    layout->addWidget(m_pTopLabel);

    m_pCertificate = new QPlainTextEdit();
    m_pCertificate->setMaximumSize(QSize(16777215, 73));
    m_pCertificate->setPlaceholderText(
		"NAME: User Name\n"
		"TYPE: ULTIMATE\n"
		"DATE: dd.mm.yyyy\n"
		"UPDATEKEY: 00000000000000000000000000000000\n"
		"SIGNATURE: AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=="
	);
    layout->addWidget(m_pCertificate);
    connect(m_pCertificate, SIGNAL(textChanged()), this, SIGNAL(completeChanged()));
    registerField("useCertificate", m_pCertificate, "plainText");
    
    layout->addWidget(new QLabel(tr("Retrieve certificate using Serial Number:")));

    m_pSerial = new QLineEdit();
    m_pSerial->setPlaceholderText("SBIE_-_____-_____-_____-_____");
    layout->addWidget(m_pSerial);

    m_pEvaluate = new QCheckBox(tr("Start evaluation without a certificate for a limited period of time."));
    if (CERT_IS_TYPE(g_CertInfo, eCertEvaluation)) {
        m_pEvaluate->setEnabled(false);
        m_pEvaluate->setChecked(true);
    }
    layout->addWidget(m_pEvaluate);
    connect(m_pEvaluate, SIGNAL(toggled(bool)), this, SIGNAL(completeChanged()));
    registerField("isEvaluate", m_pEvaluate);

    QLabel* pGetEvalCert = new QLabel(tr("<b><a href=\"_\"><font color='red'>Get a free evaluation certificate</font></a> and enjoy all premium features for %1 days.</b>").arg(EVAL_DAYS));
    pGetEvalCert->setToolTip(tr("You can request a free %1-day evaluation certificate up to %2 times per hardware ID.").arg(EVAL_DAYS).arg(EVAL_MAX));
    layout->addWidget(pGetEvalCert);
    connect(pGetEvalCert, &QLabel::linkActivated, this, [=]() {
        CSettingsWindow::StartEval(this, this, SLOT(OnCertData(const QByteArray&, const QVariantMap&)));
	});

    layout->addWidget(new QWidget());

    setLayout(layout);
}

void CCertificatePage::initializePage()
{
    m_pCertificate->setPlainText(g_Certificate);

    uchar UsageFlags = 0;
    theAPI->GetSecureParam("UsageFlags", &UsageFlags, sizeof(UsageFlags));

    if (field("useBusiness").toBool())
    {
        UsageFlags |= 1;
        UsageFlags &= ~2;
        theAPI->SetSecureParam("UsageFlags", &UsageFlags, sizeof(UsageFlags));

        m_pTopLabel->setText(
            tr("To use <b>Sandboxie-Plus</b> in a business setting, an appropriate <a href=\"https://sandboxie-plus.com/go.php?to=sbie-get-cert\">support certificate</a> for business use is required. "
            "If you do not yet have the required certificate(s), you can get those from the <a href=\"https://xanasoft.com/shop/\">xanasoft.com web shop</a>.")
        );

        m_pEvaluate->setVisible(true);
    }
    else 
    {
        if((UsageFlags & 1) != 0)
            UsageFlags |= 2;
        UsageFlags &= ~1;
        theAPI->SetSecureParam("UsageFlags", &UsageFlags, sizeof(UsageFlags));

        m_pTopLabel->setText(
            tr("<b>Sandboxie-Plus</b> provides additional features and box types exclusively to <u>project supporters</u>. "
                "Boxes like the Privacy Enhanced boxes <b><font color='red'>protect user data from illicit access</font></b> by the sandboxed programs. "
                "If you are not yet a supporter, then please consider <a href=\"https://sandboxie-plus.com/go.php?to=sbie-get-cert\">supporting the project</a> "
                "to ensure further development of Sandboxie and to receive a <a href=\"https://sandboxie-plus.com/go.php?to=sbie-cert\">supporter certificate</a>.")
        );

        m_pEvaluate->setVisible(false);
    }

    m_pSerial->setVisible(true);
    m_pSerial->clear();
}

int CCertificatePage::nextId() const
{
    return m_NextPage;
}

bool CCertificatePage::isComplete() const 
{
    if (field("useBusiness").toBool())
    {
        m_pCertificate->setEnabled(!(m_pEvaluate->isChecked() && m_pEvaluate->isEnabled()));
        if (m_pCertificate->toPlainText().isEmpty() && !(m_pEvaluate->isChecked() && m_pEvaluate->isEnabled()))
            return false;
    }
    return QWizardPage::isComplete();
}

void CCertificatePage::OnCertData(const QByteArray& Certificate, const QVariantMap& Params)
{
    if (!Certificate.isEmpty()) {
        m_pSerial->clear();
        m_pCertificate->setPlainText(Certificate);
        wizard()->next();
    }
    else {
        QString Message = tr("Failed to retrieve the certificate.");
        Message += tr("\nError: %1").arg(Params["error"].toString());
        QMessageBox::critical(this, "Sandboxie-Plus", Message);
    }
}

bool CCertificatePage::validatePage()
{
    if (m_pEvaluate->isChecked())
        return true;

    QByteArray Certificate = m_pCertificate->toPlainText().toUtf8();
    QString Serial = m_pSerial->text();

    if (!Serial.isEmpty()) {
        QVariantMap Params;
	    if(!Certificate.isEmpty())
		    Params["key"] = GetArguments(Certificate, L'\n', L':').value("UPDATEKEY");
        SB_PROGRESS Status = theGUI->m_pUpdater->GetSupportCert(Serial, this, SLOT(OnCertData(const QByteArray&, const QVariantMap&)), Params);
	    if (Status.GetStatus() == OP_ASYNC) {
		    theGUI->AddAsyncOp(Status.GetValue());
            Status.GetValue()->ShowMessage(tr("Retrieving certificate..."));
	    }
        return false;
    }

    if (!Certificate.isEmpty()) {
        if (Certificate != g_Certificate) {
            if (CSettingsWindow::ApplyCertificate(Certificate, this))
                return false;
        }
        if (CSettingsWindow::CertRefreshRequired()) {
            if (!CSettingsWindow::TryRefreshCert(this, this, SLOT(OnCertData(const QByteArray&, const QVariantMap&)))) {
                m_pCertificate->clear();
                theGUI->SetCertificate("");
            }
            return false;
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// CUIPage
// 

CUIPage::CUIPage(QWidget* parent)
    : QWizardPage(parent)
{
    setTitle(tr("Configure <b>Sandboxie-Plus</b> UI"));
    setSubTitle(tr("Select the user interface style you prefer."));

    QGridLayout* layout = new QGridLayout;

    m_pAdvanced = new QRadioButton(tr("&Advanced UI for experts"));
    m_pAdvanced->setChecked(theConf->GetInt("Options/ViewMode", 1) == 1);
    layout->addWidget(m_pAdvanced, 0, 0);
    registerField("useAdvanced", m_pAdvanced);

    m_pSimple = new QRadioButton(tr("&Simple UI for beginners"));
    m_pSimple->setChecked(theConf->GetInt("Options/ViewMode", 1) == 0);
    layout->addWidget(m_pSimple, 1, 0);
    registerField("useSimple", m_pSimple);

    m_pClassic = new QRadioButton(tr("&Vintage SbieCtrl.exe UI"));
    m_pClassic->setChecked(theConf->GetInt("Options/ViewMode", 1) == 2);
    layout->addWidget(m_pClassic, 2, 0);
    registerField("useClassic", m_pClassic);

    QButtonGroup *buttonGroup1 = new QButtonGroup();
    buttonGroup1->addButton(m_pAdvanced, 0);
    buttonGroup1->addButton(m_pSimple, 1);
    buttonGroup1->addButton(m_pClassic, 2);
    connect(buttonGroup1, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(UpdatePreview()));

    QLabel* pDummy = new QLabel();
    pDummy->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    layout->addWidget(pDummy, 0, 1, 6, 4);
    pDummy->setStyleSheet("QLabel { background-color : " + QApplication::palette().color(QPalette::Base).name() + "; }");

    m_pPreview = new QLabel();
    m_pPreview->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_pPreview->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    layout->addWidget(m_pPreview, 0, 1, 6, 4);

    QWidget* pSpacer = new QWidget();
	pSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(pSpacer, 3, 0);

    m_pBrightMode = new QRadioButton(tr("Use Bright Mode"));
    layout->addWidget(m_pBrightMode, 4, 0);
    registerField("useBrightMode", m_pBrightMode);

    m_pDarkMode = new QRadioButton(tr("Use Dark Mode"));
    layout->addWidget(m_pDarkMode, 5, 0);
    registerField("useDarkMode", m_pDarkMode);

    QButtonGroup *buttonGroup2 = new QButtonGroup();
    buttonGroup2->addButton(m_pBrightMode, 0);
    buttonGroup2->addButton(m_pDarkMode, 1);
    connect(buttonGroup2, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(UpdatePreview()));



    layout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 1, 1, 1);
    layout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 2, 1, 1);
    layout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 3, 1, 1);
    layout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 4, 1, 1);

    setLayout(layout);
}

void CUIPage::initializePage() 
{ 
    QTimer::singleShot(10, this, SLOT(UpdatePreview()));
}

void CUIPage::UpdatePreview()
{
    bool bDark;
    if (m_pDarkMode->isChecked())
        bDark = true;
    else if (m_pBrightMode->isChecked())
        bDark = false;
    else { // same as os
        QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
        bDark = (settings.value("AppsUseLightTheme") == 0);
    }

    QPixmap preview;
    if(m_pAdvanced->isChecked() && !bDark)
        preview = QPixmap::fromImage(QImage(":/Assets/Advanced.png"));
    else if(m_pAdvanced->isChecked() && bDark)
        preview =  QPixmap::fromImage(QImage(":/Assets/AdvancedD.png"));
    else if(m_pSimple->isChecked() && !bDark)
        preview = QPixmap::fromImage(QImage(":/Assets/Simple.png"));
    else if(m_pSimple->isChecked() && bDark)
        preview = QPixmap::fromImage(QImage(":/Assets/SimpleD.png"));
    else if(m_pClassic->isChecked() && !bDark)
        preview = QPixmap::fromImage(QImage(":/Assets/Classic.png"));
    else if(m_pClassic->isChecked() && bDark)
        preview = QPixmap::fromImage(QImage(":/Assets/ClassicD.png"));

    //QRect rect(0, 0, m_pPreview->width(), m_pPreview->height());
    //m_pPreview->setPixmap(preview.scaled(preview.width()*5/10, preview.height()*5/10, Qt::KeepAspectRatio, Qt::SmoothTransformation).copy(rect));
    //m_pPreview->setPixmap(preview.scaled(min(m_pPreview->width(), preview.width()), min(m_pPreview->height(), preview.height()), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    //m_pPreview->setPixmap(preview);
    //m_pPreview->setPixmap(preview.scaled(preview.width()*7/10, preview.height()*7/10, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_pPreview->setPixmap(preview.scaled(std::min(preview.width(), std::max(preview.width()*7/10, m_pPreview->width())),
        std::min(preview.height(), std::max(preview.height()*7/10, m_pPreview->height())), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

int CUIPage::nextId() const
{
    return CSetupWizard::Page_Shell;
}

//////////////////////////////////////////////////////////////////////////////////////////
// CShellPage
// 

CShellPage::CShellPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Configure <b>Sandboxie-Plus</b> shell integration"));
    setSubTitle(tr("Configure how Sandboxie-Plus should integrate with your system."));

    QVBoxLayout *layout = new QVBoxLayout;

    m_pAutoStart = new QCheckBox(tr("Start UI with Windows"));
    m_pAutoStart->setChecked(true);
    layout->addWidget(m_pAutoStart);
    registerField("isAutoStart", m_pAutoStart);

    m_pContecxtMenu = new QCheckBox(tr("Add 'Run Sandboxed' to the explorer context menu"));
    m_pContecxtMenu->setChecked(true);
    layout->addWidget(m_pContecxtMenu);
    registerField("useContecxtMenu", m_pContecxtMenu);

    m_pBrowserIcon = new QCheckBox(tr("Add desktop shortcut for starting Web browser under Sandboxie"));
    m_pBrowserIcon->setChecked(true);
    layout->addWidget(m_pBrowserIcon);
    registerField("useBrowserIcon", m_pBrowserIcon);

	m_pEditOnlyAdmin = new QCheckBox(tr("Only applications with admin rights can change configuration"));
	m_pEditOnlyAdmin->setChecked(false);
	connect(m_pEditOnlyAdmin,SIGNAL(clicked(bool)), this, SLOT(OnEditOnlyAdmin()));
	layout->addWidget(m_pEditOnlyAdmin);
	registerField("editAdminOnly", m_pEditOnlyAdmin);

    setLayout(layout);
}

void CShellPage::OnEditOnlyAdmin() 
{
    if (m_pEditOnlyAdmin->isChecked()) {
        if (QMessageBox::warning(this, tr("Warning"), tr("Enabling this option prevents changes to the Sandboxie.ini configuration from the user interface without admin rights. Be careful, as using Sandboxie Manager with normal user rights may result in a lockout. "
          "To make changes to the configuration, you must restart Sandboxie Manager as an admin by clicking 'Restart as Admin' in the 'Sandbox' menu in the main window."),
          QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel)
            m_pEditOnlyAdmin->setChecked(false);
    }
}

int CShellPage::nextId() const
{
    //return CSetupWizard::Page_WFP;
    return CSetupWizard::Page_Update;
    //return CSetupWizard::Page_Finish;
}

//////////////////////////////////////////////////////////////////////////////////////////
// CWFPPage
// 

/*CWFPPage::CWFPPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Configure <b>Sandboxie-Plus</b> network filtering"));
    setSubTitle(tr("Sandboxie can use the Windows Filtering Platform (WFP) to restrict network access."));

    QVBoxLayout *layout = new QVBoxLayout;

    QLabel* pLabel = new QLabel;
    pLabel->setWordWrap(true);
    pLabel->setText(tr("Using WFP allows Sandboxie to reliably enforce IP/Port based rules for network access. "
        "Unlike system level application firewalls, Sandboxie can use different rules in each box for the same application. "
        "If you already have a good and reliable application firewall and do not need per box rules, you can leave this option unchecked. "
        "Without WFP enabled, Sandboxie will still be able to reliably and entirely block processes from accessing the network. "
        "However, this can cause the process to crash, as the driver blocks the required network device endpoints. "
        "Even with WFP disabled, Sandboxie offers to set IP/Port based rules, however these will be applied in user mode only and not be enforced by the driver. "
        "Hence, without WFP enabled, an intentionally malicious process could bypass those rules, but not the entire network block."));
    layout->addWidget(pLabel);

    m_pUseWFP = new QCheckBox(tr("Enable Windows Filtering Platform (WFP) support"));
    m_pUseWFP->setChecked(theAPI->GetGlobalSettings()->GetBool("NetworkEnableWFP", false));
    layout->addWidget(m_pUseWFP);
    registerField("useWFP", m_pUseWFP);

    setLayout(layout);
}

int CWFPPage::nextId() const
{
    return CSetupWizard::Page_Finish;
}*/

//////////////////////////////////////////////////////////////////////////////////////////
// CSBUpdate
// 

CSBUpdate::CSBUpdate(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Configure <b>Sandboxie-Plus</b> updater"));
    setSubTitle(tr("Like with any other security product, it's important to keep your Sandboxie-Plus up to date."));

    QGridLayout *layout = new QGridLayout;
    layout->setSpacing(3);

    int row = 0;
    int rows = 4;

    m_pUpdate = new QCheckBox(tr("Regularly check for all updates to Sandboxie-Plus and optional components"));
    m_pUpdate->setToolTip(tr("Let Sandboxie regularly check for latest updates."));
    layout->addWidget(m_pUpdate, row++, 0, 1, rows);
    connect(m_pUpdate, &QCheckBox::toggled, this, &CSBUpdate::UpdateOptions);
    registerField("updateAll", m_pUpdate);

    QWidget* pSpacer = new QWidget();
    pSpacer->setMinimumHeight(16);
    pSpacer->setMaximumWidth(16);
    layout->addWidget(pSpacer, row+5, 0, 1, 1);

    m_pVersion = new QCheckBox(tr("Check for new Sandboxie-Plus versions:"));
    m_pVersion->setToolTip(tr("Check for new Sandboxie-Plus builds."));
    layout->addWidget(m_pVersion, row++, 1, 1, rows-2);
    //layout->addWidget(new QComboBox(), row-1, 3, 1, 1);
    connect(m_pVersion, &QCheckBox::toggled, this, &CSBUpdate::UpdateOptions);
    registerField("updateApp", m_pVersion);

    m_pChanelInfo = new QLabel(tr("Select in which update channel to look for new Sandboxie-Plus builds:"));
    m_pChanelInfo->setMinimumHeight(20);
    layout->addWidget(m_pChanelInfo, row++, 1, 1, rows-1);

    pSpacer = new QWidget();
    pSpacer->setMinimumHeight(16);
    pSpacer->setMaximumWidth(16);
    layout->addWidget(pSpacer, row, 1, 1, 1);

    m_pStable = new QRadioButton(tr("In the Stable Channel"));
    m_pStable->setToolTip(tr("The stable channel contains the latest stable GitHub releases."));
    layout->addWidget(m_pStable, row++, 2, 1, rows-2);
    registerField("channelStable", m_pStable);

    m_pPreview = new QRadioButton(tr("In the Preview Channel - with newest experimental changes"));
    m_pPreview->setToolTip(tr("The preview channel contains the latest GitHub pre-releases."));
    layout->addWidget(m_pPreview, row++, 2, 1, rows-2);
    registerField("channelPreview", m_pPreview);

    m_pInsider = new QRadioButton(tr("In the Insider Channel - exclusive features"));
    m_pInsider->setToolTip(tr("The Insider channel offers early access to new features and bugfixes that will eventually be released to the public, "
        "as well as all relevant improvements from the stable channel. \nUnlike the preview channel, it does not include untested, potentially breaking, "
        "or experimental changes that may not be ready for wider use."));
    layout->addWidget(m_pInsider, row, 2, 1, 1);
    registerField("channelInsider", m_pInsider);
    QLabel* pInsiderInfo = new QLabel(tr("More about the <a href=\"https://sandboxie-plus.com/go.php?to=sbie-insider\">Insider Channel</a>"));
    connect(pInsiderInfo, SIGNAL(linkActivated(const QString&)), theGUI, SLOT(OpenUrl(const QString&)));
    layout->addWidget(pInsiderInfo, row++, 3, 1, 1);

    //m_pTemplates = new QCheckBox(tr("Keep Compatibility Templates up to date"));
    //m_pTemplates->setToolTip(tr("Check for latest compatibility templates."));
    //layout->addWidget(m_pTemplates, row++, 1, 1, rows-1);
    //registerField("updateCompat", m_pTemplates);

    m_pHotfixes = new QCheckBox(tr("Keep Compatibility Templates up to date and apply hotfixes"));
    m_pHotfixes->setToolTip(tr("Check for latest compatibility templates and hotfixes."));
    layout->addWidget(m_pHotfixes, row++, 1, 1, rows-1);
    registerField("applyHotfixes", m_pHotfixes);

    m_pIssues = new QCheckBox(tr("Get the latest Scripts for the Troubleshooting Wizard"));
    m_pIssues->setToolTip(tr("Check for latest troubleshooting scripts for the troubleshooting wizard."));
    layout->addWidget(m_pIssues, row++, 1, 1, rows-1);
    registerField("updateIssues", m_pIssues);

    m_pAddons = new QCheckBox(tr("Keep the list of optional Add-on components up to date"));
    m_pAddons->setToolTip(tr("Check for latest available add-ons."));
    layout->addWidget(m_pAddons, row++, 1, 1, rows-1);
    registerField("updateAddons", m_pAddons);

    m_pUpdateInfo = new QLabel();
    m_pUpdateInfo->setWordWrap(true);
    //m_pUpdateInfo->setText(tr("Sandboxie-Plus applies many application restrictions which may occasionally lead to compatibility issues, necessitating mitigation efforts. "
    //    "Compatibility may be broken by new Windows updates and changes within sandboxed applications themselves. "
    //    "To ensure smooth operation, it is highly recommended to regularly check for Sandboxie-Plus updates, apply the latest compatibility templates, and keep troubleshooting scripts up-to-date."));
    m_pUpdateInfo->setText(tr("Sandboxie-Plus applies strict application restrictions, which can lead to compatibility issues. "
        "Stay updated with Sandboxie-Plus, including compatibility templates and troubleshooting, to ensure smooth operation amid Windows updates and application changes."));
    layout->addWidget(m_pUpdateInfo, row++, 0, 1, rows);

    layout->addItem(new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Expanding), row++, 0);

    m_pBottomLabel = new QLabel(tr("Access to the latest compatibility templates and the online troubleshooting database requires a valid <a href=\"https://sandboxie-plus.com/go.php?to=sbie-cert\">supporter certificate</a>."));
    connect(m_pBottomLabel, SIGNAL(linkActivated(const QString&)), theGUI, SLOT(OpenUrl(const QString&)));
    m_pBottomLabel->setWordWrap(true);
    layout->addWidget(m_pBottomLabel, row++, 0, 1, rows);

    setLayout(layout);
}

void CSBUpdate::initializePage()
{
    m_pUpdate->setChecked(true);
    m_pStable->setChecked(true);

    m_pBottomLabel->setVisible(!g_CertInfo.active || g_CertInfo.expired);

    UpdateOptions();
}

void CSBUpdate::UpdateOptions()
{
    m_pVersion->setVisible(!m_pUpdate->isChecked());
    //m_pTemplates->setVisible(!m_pUpdate->isChecked());
    m_pHotfixes->setVisible(!m_pUpdate->isChecked());
    m_pIssues->setVisible(!m_pUpdate->isChecked());
    m_pAddons->setVisible(!m_pUpdate->isChecked());
    m_pChanelInfo->setVisible(m_pUpdate->isChecked());
    m_pUpdateInfo->setVisible(m_pUpdate->isChecked());

    if (m_pUpdate->isChecked()) {
        m_pVersion->setChecked(true);
        //m_pTemplates->setChecked(true);
        m_pIssues->setChecked(true);
        m_pAddons->setChecked(true);
    }

    m_pStable->setEnabled(m_pVersion->isChecked());
    m_pPreview->setEnabled(m_pVersion->isChecked());
    m_pInsider->setEnabled(CERT_IS_INSIDER(g_CertInfo) && m_pVersion->isChecked());

    m_pHotfixes->setEnabled(m_pVersion->isChecked());
}

int CSBUpdate::nextId() const
{
    return CSetupWizard::Page_Finish;
}

//////////////////////////////////////////////////////////////////////////////////////////
// CFinishPage
// 

CFinishPage::CFinishPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Complete your configuration"));
    QPixmap Logo = QPixmap(theGUI->m_DarkTheme ? ":/SideLogoDM.png" : ":/SideLogo.png");
    int Scaling = theConf->GetInt("Options/FontScaling", 100);
    if(Scaling !=  100) Logo = Logo.scaled(Logo.width() * Scaling / 100, Logo.height() * Scaling / 100);
    setPixmap(QWizard::WatermarkPixmap, Logo);

    QVBoxLayout *layout = new QVBoxLayout;

    m_pLabel = new QLabel;
    m_pLabel->setWordWrap(true);
    m_pLabel->setText(tr("Almost complete, click Finish to apply all selected options and conclude the wizard."));
    layout->addWidget(m_pLabel);

    //QWidget* pSpacer = new QWidget();
    //pSpacer->setMinimumHeight(16);
    //layout->addWidget(pSpacer);
    
    //QLabel* pLabel = new QLabel;
    //pLabel->setWordWrap(true);
    //pLabel->setText(tr("Like with any other security product it's important to keep your Sandboxie-Plus up to date."));
    //layout->addWidget(pLabel);

    //m_pUpdate = new QCheckBox(tr("Keep Sandboxie-Plus up to date."));
    //m_pUpdate->setChecked(true);
    //layout->addWidget(m_pUpdate);
    //registerField("isUpdate", m_pUpdate);

    setLayout(layout);
}

int CFinishPage::nextId() const
{
    return -1;
}

void CFinishPage::initializePage()
{

}


//void ConclusionPage::setVisible(bool visible)
//{
//    QWizardPage::setVisible(visible);
//
//    if (visible) {
//        wizard()->setButtonText(QWizard::CustomButton1, tr("&Print"));
//        wizard()->setOption(QWizard::HaveCustomButton1, true);
//        connect(wizard(), &QWizard::customButtonClicked,
//                this, &ConclusionPage::printButtonClicked);
//    } else {
//        wizard()->setOption(QWizard::HaveCustomButton1, false);
//        disconnect(wizard(), &QWizard::customButtonClicked,
//                   this, &ConclusionPage::printButtonClicked);
//    }
//}
