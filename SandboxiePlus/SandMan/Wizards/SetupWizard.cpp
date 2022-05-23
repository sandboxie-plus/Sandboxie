#include "stdafx.h"

#include "SetupWizard.h"
#include "../MiscHelpers/Common/Common.h"
#include "../Windows/SettingsWindow.h"
#include "../SandMan.h"
#include "Helpers/WinAdmin.h"

QString emailRegExp = QStringLiteral(".+@.+");

CSetupWizard::CSetupWizard(QWidget *parent)
    : QWizard(parent)
{
    setPage(Page_Intro, new CIntroPage);
    setPage(Page_Certificate, new CCertificatePage);
    setPage(Page_Shell, new CShellPage);
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

bool CSetupWizard::ShowWizard()
{
    CSetupWizard wizard;
    if (!wizard.exec())
        return false;
    
    //bool useBusiness = wizard.field("useBusiness").toBool();
    //QString Certificate = wizard.field("useCertificate").toString();
    //bool isEvaluate = wizard.field("isEvaluate").toBool();

    AutorunEnable(wizard.field("isAutoStart").toBool());
    
    if (wizard.field("useContecxtMenu").toBool())
        CSettingsWindow__AddContextMenu();

    if (wizard.field("useBrowserIcon").toBool())
        CSettingsWindow__AddBrowserIcon();

    if (wizard.field("isUpdate").toBool()) {
        theConf->SetValue("Options/CheckForUpdates", 1);
	    theConf->SetValue("Options/DownloadUpdates", 1);
	    theConf->SetValue("Options/InstallUpdates", 1);
    }

    theConf->SetValue("Options/WizardLevel", 1);

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// CIntroPage
// 

CIntroPage::CIntroPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Introduction"));
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/SideLogo.png"));

    QVBoxLayout *layout = new QVBoxLayout;
    QLabel* pTopLabel = new QLabel(tr("Welcome to the Setup Wizard. This wizard will help you to configure your copy of <b>Sandboxie-Plus</b>. "
        "You can start this wizard at any time from the Sandbox->Maintenance menu if you do not wish to complete it now."));
    pTopLabel->setWordWrap(true);
    layout->addWidget(pTopLabel);

    QWidget* pSpace = new QWidget();
    pSpace->setMinimumHeight(16);
    layout->addWidget(pSpace);

    int BusinessUse = theConf->GetInt("Options/BusinessUse", 2);

    m_pLabel = new QLabel(tr("Select how you would like to use Sandboxie-Plus"));
    layout->addWidget(m_pLabel);

    m_pPersonalRadio = new QRadioButton(tr("&Personally, for private non-commercial use"));
    layout->addWidget(m_pPersonalRadio);
    connect(m_pPersonalRadio, SIGNAL(toggled(bool)), this, SIGNAL(completeChanged()));
    registerField("usePersonal", m_pPersonalRadio);

    m_pBusinessRadio = new QRadioButton(tr("&Commercially, for business or enterprise use"));
    layout->addWidget(m_pBusinessRadio);
    connect(m_pBusinessRadio, SIGNAL(toggled(bool)), this, SIGNAL(completeChanged()));
    registerField("useBusiness", m_pBusinessRadio);

    if (BusinessUse != 2) {
        m_pLabel->setEnabled(false);
        m_pPersonalRadio->setChecked(BusinessUse == 0);
        m_pPersonalRadio->setEnabled(false);
        m_pBusinessRadio->setChecked(BusinessUse == 1);
        m_pBusinessRadio->setEnabled(false);
    }

    setLayout(layout);
}

int CIntroPage::nextId() const
{
    if(g_Certificate.isEmpty())
        return CSetupWizard::Page_Certificate;
    return CSetupWizard::Page_Shell;
}

bool CIntroPage::isComplete() const 
{
    if (m_pLabel->isEnabled() && !m_pPersonalRadio->isChecked() && !m_pBusinessRadio->isChecked())
        return false;
    return QWizardPage::isComplete();
}

//////////////////////////////////////////////////////////////////////////////////////////
// CCertificatePage
// 

CCertificatePage::CCertificatePage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Install your <b>Sandboxie-Plus</b> support certificate"));
    setSubTitle(tr("If you have a supporter certificate, please fill it into the field below."));
    
    QGridLayout *layout = new QGridLayout;

    m_pTopLabel = new QLabel();
    m_pTopLabel->setWordWrap(true);
    connect(m_pTopLabel, SIGNAL(linkActivated(const QString&)), theGUI, SLOT(OpenUrl(const QString&)));
    layout->addWidget(m_pTopLabel);

    m_pCertificate = new QPlainTextEdit();
    m_pCertificate->setMaximumSize(QSize(16777215, 73));
    m_pCertificate->setPlaceholderText(
		"NAME: User Name\n"
		"LEVEL: ULTIMATE\n"
		"DATE: dd.mm.yyyy\n"
		"UPDATEKEY: 00000000000000000000000000000000\n"
		"SIGNATURE: AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=="
	);
    layout->addWidget(m_pCertificate);
    connect(m_pCertificate, SIGNAL(textChanged()), this, SIGNAL(completeChanged()));
    registerField("useCertificate", m_pCertificate, "plainText");
    
    m_pEvaluate = new QCheckBox(tr("Start evaluation without a certificate for a limited period of time."));
    layout->addWidget(m_pEvaluate);
    connect(m_pEvaluate, SIGNAL(toggled(bool)), this, SIGNAL(completeChanged()));
    registerField("isEvaluate", m_pEvaluate);

    layout->addWidget(new QWidget());

    setLayout(layout);
}

void CCertificatePage::initializePage()
{
    m_pCertificate->setPlainText(g_Certificate);

    if (field("useBusiness").toBool())
    {
        theConf->SetValue("Options/BusinessUse", 1);

        m_pTopLabel->setText(
            tr("To use <b>Sandboxie-Plus</b> in a business setting, an appropriate <a href=\"https://sandboxie-plus.com/go.php?to=sbie-get-cert\">support certificate</a> for business use is required. "
            "If you do not yet have the required certificate(s), you can get those from the <a href=\"https://xanasoft.com/shop/\">xanasoft.com web shop</a>.")
        );

        m_pEvaluate->setVisible(true);
    }
    else 
    {
        theConf->SetValue("Options/BusinessUse", 0);

        m_pTopLabel->setText(
            tr("<b>Sandboxie-Plus</b> provides additional features and box types exclusively to <u>project supporters</u>. "
                "Boxes like the Privacy Enhanced boxes <b><font color='red'>protect user data from illicit access</font></b> by the sandboxed programs. "
                "If you are not yet a supporter, then please consider <a href=\"https://sandboxie-plus.com/go.php?to=sbie-get-cert\">supporting the project</a> "
                "to ensure further development of Sandboxie and to receive a <a href=\"https://sandboxie-plus.com/go.php?to=sbie-cert\">supporter certificate</a>.")
        );

        m_pEvaluate->setVisible(false);
    }
}

int CCertificatePage::nextId() const
{
    return CSetupWizard::Page_Shell;
}

bool CCertificatePage::isComplete() const 
{
    if (field("useBusiness").toBool())
    {
        m_pCertificate->setEnabled(!m_pEvaluate->isChecked());
        if (m_pCertificate->toPlainText().isEmpty() && !m_pEvaluate->isChecked())
            return false;
    }
    return QWizardPage::isComplete();
}

bool CCertificatePage::validatePage()
{
    QByteArray Certificate = m_pCertificate->toPlainText().toLatin1();
    if (!m_pEvaluate->isChecked() && !Certificate.isEmpty() && g_Certificate != Certificate) {
		return CSettingsWindow::ApplyCertificate(Certificate, this);
    }
    return true;
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

    setLayout(layout);
}

int CShellPage::nextId() const
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
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/SideLogo.png"));

    QVBoxLayout *layout = new QVBoxLayout;

    m_pLabel = new QLabel;
    m_pLabel->setWordWrap(true);
    m_pLabel->setText(tr("Almost complete, click Finish to apply all selected options and conclude the wizard."));
    layout->addWidget(m_pLabel);

    QWidget* pSpacer = new QWidget();
    pSpacer->setMinimumHeight(16);
    layout->addWidget(pSpacer);
    
    //QLabel* pLabel = new QLabel;
    //pLabel->setWordWrap(true);
    //pLabel->setText(tr("Like with any other security product it's important to keep your Sandboxie-Plus up to date."));
    //layout->addWidget(pLabel);

    m_pUpdate = new QCheckBox(tr("Keep Sandboxie-Plus up to date."));
    m_pUpdate->setChecked(true);
    layout->addWidget(m_pUpdate);
    registerField("isUpdate", m_pUpdate);

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
