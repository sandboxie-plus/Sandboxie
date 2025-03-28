#include "stdafx.h"

#include "NewBoxWizard.h"
#include "../MiscHelpers/Common/Common.h"
#include "../Windows/SettingsWindow.h"
#include "../SandMan.h"
#include "Helpers/WinAdmin.h"
#include <QButtonGroup>
#include "../QSbieAPI/SbieUtils.h"
#include "../Views/SbieView.h"
#include "../MiscHelpers/Common/CheckableMessageBox.h"
#include "../Windows/BoxImageWindow.h"
#include "../AddonManager.h"


CNewBoxWizard::CNewBoxWizard(bool bAlowTemp, QWidget *parent)
    : QWizard(parent)
{
    setPage(Page_Type, new CBoxTypePage(bAlowTemp));
    setPage(Page_Files, new CFilesPage);
    setPage(Page_Isolation, new CIsolationPage);
    setPage(Page_Advanced, new CAdvancedPage);
    setPage(Page_Summary, new CSummaryPage);

    m_bAdvanced = theConf->GetBool("Options/AdvancedBoxWizard", false);

    setWizardStyle(ModernStyle);
    //setOption(HaveHelpButton, true);
    setPixmap(QWizard::LogoPixmap, QPixmap(":/SandMan.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    connect(this, &QWizard::helpRequested, this, &CNewBoxWizard::showHelp);

    setWindowTitle(tr("New Box Wizard"));

    setMinimumWidth(600 * theConf->GetInt("Options/FontScaling", 100) / 100);
}

void CNewBoxWizard::showHelp()
{

}

QString CNewBoxWizard::CreateNewBox(bool bAlowTemp, QWidget* pParent)
{
	CNewBoxWizard wizard(bAlowTemp, pParent);
    if (!theGUI->SafeExec(&wizard))
        return QString();

    QString BoxName = wizard.field("boxName").toString();
	BoxName.replace(" ", "_");
	return BoxName;
}

SB_STATUS CNewBoxWizard::TryToCreateBox()
{
    QString BoxName = field("boxName").toString();
    BoxName.replace(" ", "_");
	int BoxType = field("boxType").toInt();
#ifndef USE_COMBO
    bool BlackBox = field("blackBox").toBool();
#else
    bool BlackBox = CSandBoxPlus::ePrivate;
#endif

    QString Password;
    quint64 ImageSize = 0;
    if (BlackBox) {
        CBoxImageWindow window(CBoxImageWindow::eNew, this);
        if (theGUI->SafeExec(&window) == 1) {
            Password = window.GetPassword();
            ImageSize = window.GetImageSize();

            if (theConf->GetBool("Options/WarnBoxCrypto", true)) {
                bool State = false;
                if(CCheckableMessageBox::question(this, "Sandboxie-Plus",
                    tr("This sandbox content will be placed in an encrypted container file, "
                        "please note that any corruption of the container's header will render all its content permanently inaccessible. "
                        "Corruption can occur as a result of a BSOD, a storage hardware failure, or a malicious application overwriting random files. "
                        "This feature is provided under a strict <b>No Backup No Mercy</b> policy, YOU the user are responsible for the data you put into an encrypted box. "
                        "<br /><br />"
                        "IF YOU AGREE TO TAKE FULL RESPONSIBILITY FOR YOUR DATA PRESS [YES], OTHERWISE PRESS [NO].")
                    , tr("Don't show this message again."), &State, QDialogButtonBox::Yes | QDialogButtonBox::No, QDialogButtonBox::No, QMessageBox::Warning) != QDialogButtonBox::Yes)
                        return SB_ERR(SB_Canceled);

                if (State)
                    theConf->SetValue("Options/WarnBoxCrypto", false);
            }

        }
        else
            return SB_ERR(SB_Canceled);
    }

	SB_STATUS Status = theAPI->CreateBox(BoxName, true);

	if (!Status.IsError())
	{
		CSandBoxPtr pBox = theAPI->GetBoxByName(BoxName);

        // SharedTemplate
        QElapsedTimer timer;
        timer.start();

        int SharedTemplateIndex = field("sharedTemplateIndex").toInt();
        const QString templateName = (SharedTemplateIndex == 0)
            ? QString("SharedTemplate")
            : QString("SharedTemplate_%1").arg(SharedTemplateIndex);
        const QString templateFullName = "Template_Local_" + templateName;
        const QString templateSettings = theAPI->SbieIniGetEx(templateFullName, "");
        const QStringList templateSettingsLines = templateSettings.split(QRegularExpression(QStringLiteral("[\r\n]")), Qt::SkipEmptyParts);
        const QString templateComment = tr("Add your settings after this line.");
        const QString templateTitle = (SharedTemplateIndex == 0)
            ? tr("Shared Template")
            : tr("Shared Template") + " " + QString::number(SharedTemplateIndex);
        const QString boxSettings = theAPI->SbieIniGetEx(BoxName, "");
        const QStringList boxSettingsLines = boxSettings.split(QRegularExpression(QStringLiteral("[\r\n]")), Qt::SkipEmptyParts);
        const QStringList SPECIAL_SETTINGS = { "BorderColor", "BoxIcon", "ConfigLevel", "CopyLimitKb" };

        bool disableWizardSettings = templateSettings.contains(QRegularExpression(QStringLiteral("[\r\n]#DisableWizardSettings=y[\r\n]")));
        bool removeDefaultAll = templateSettings.contains(QRegularExpression(QStringLiteral("[\r\n]#RemoveDefaultAll=y[\r\n]")));
        bool removeDefaultRecovers = templateSettings.contains(QRegularExpression(QStringLiteral("[\r\n]#RemoveDefaultRecovers=y[\r\n]")));
        bool removeDefaultTemplates = templateSettings.contains(QRegularExpression(QStringLiteral("[\r\n]#RemoveDefaultTemplates=y[\r\n]")));

        int sharedTemplateMode = field("sharedTemplate").toInt();

        // Create base template
        if (templateSettings.isEmpty() && sharedTemplateMode != 0) {
            const QString templateBase = QStringLiteral("Tmpl.Title=%1\nTmpl.Class=Local\n%3=n\n%4=n\n%5=n\n%6=n\nTmpl.Comment=%2")
                .arg(templateTitle, templateComment, "#DisableWizardSettings", "#RemoveDefaultAll", "#RemoveDefaultRecovers", "#RemoveDefaultTemplates");
            theAPI->SbieIniSet(templateFullName, "", templateBase);
        }

        switch (sharedTemplateMode)
        {
        case 1:
        case 2:
        case 3:
            // Remove default settings
            if (removeDefaultRecovers || removeDefaultAll) {
                pBox->DelValue("RecoverFolder");
            }
            if (removeDefaultTemplates || removeDefaultAll) {
                pBox->DelValue("Template");
            }
            if (removeDefaultAll) {
                for (const QString& bLine : boxSettingsLines) {
                    int bParts = bLine.indexOf("=", Qt::SkipEmptyParts);
                    if (bParts != -1) {
                        const QString bKey = bLine.mid(0, bParts).trimmed();
                        const QString bValue = bLine.mid(bParts + 1).trimmed();
                        if (bKey.compare("Enabled", Qt::CaseInsensitive) != 0 && bKey.compare("ConfigLevel") != 0) { // Do not remove Enabled and ConfigLevel
                            pBox->DelValue(bKey, bValue);
                        }
                    }
                }
            }
            break;
        default:
            // Default case
            break;
        }
        if (sharedTemplateMode == 1) { // Insert as template
            const QString insertValue = templateFullName.mid(9); // Template_
            pBox->AppendText("Template", insertValue);
        }
        else if (sharedTemplateMode == 2) { // Append template settings to configuration
            for (const QString& tLine : templateSettingsLines) {
                int tParts = tLine.indexOf("=", Qt::SkipEmptyParts);
                if (tParts == -1) {
                    continue; // Skip lines that don't have at least one '=' character.
                }
                const QString tKey = tLine.mid(0, tParts).trimmed();
                const QString tValue = tLine.mid(tParts + 1).trimmed();
                if (tKey.compare("Enabled", Qt::CaseInsensitive) == 0 || tKey.startsWith("Tmpl.") || tKey.startsWith("#") || tKey.endsWith("Disabled")) {
                    continue; // Skip lines that start or end with one of these
                }

                if (tValue.compare("y", Qt::CaseInsensitive) == 0 || tValue.compare("n", Qt::CaseInsensitive) == 0 || SPECIAL_SETTINGS.contains(tKey)) {
                    pBox->SetText(tKey, tValue);
                }
                else {
                    pBox->AppendText(tKey, tValue);
                }
            }
        }
        qDebug().noquote().nospace() << templateName << " (Mode = " << sharedTemplateMode << ") operation took " << timer.elapsed() << " ms";
        //
        if (!disableWizardSettings || sharedTemplateMode == 0) {
		    switch (BoxType)
		    {
		    	case CSandBoxPlus::eHardenedPlus:
                    pBox->SetBool("UsePrivacyMode", true);
		    	case CSandBoxPlus::eHardened:
		    		pBox->SetBool("UseSecurityMode", true);
		    		break;

		    	case CSandBoxPlus::eDefaultPlus:
                    pBox->SetBool("UsePrivacyMode", true);
		    	case CSandBoxPlus::eDefault:
		    		break;

		    	case CSandBoxPlus::eAppBoxPlus:
                    pBox->SetBool("UsePrivacyMode", true);
		    	case CSandBoxPlus::eAppBox:
		    		pBox->SetBool("NoSecurityIsolation", true);
		    		//pBox->AppendText("Template", "NoUACProxy"); // proxy is always needed for exes in the box
		    		pBox->AppendText("Template", "RpcPortBindingsExt");
		    		break;
		    }

            if (BlackBox) {
                pBox->SetBool("UseFileImage", true);
                pBox->SetBool("ConfidentialBox", true);
            }

		    QRgb rgb = theGUI->GetBoxColor(BoxType);
		    pBox->SetText("BorderColor", QString("#%1%2%3").arg(qBlue(rgb), 2, 16, QChar('0')).arg(qGreen(rgb), 2, 16, QChar('0')).arg(qRed(rgb), 2, 16, QChar('0')) + ",ttl");


            QString Location = field("boxLocation").toString();
            if (!Location.isEmpty()) {
                pBox->SetText("FileRootPath", Location);
                theAPI->UpdateBoxPaths(pBox.data());
            }

            if (field("boxVersion").toInt() == 1) {
                pBox->SetBool("UseFileDeleteV2", true);
		    	pBox->SetBool("UseRegDeleteV2", true);
            }
            if(!field("separateUser").toBool())
                pBox->SetBool("SeparateUserFolders", false);
            if(field("useVolumeSN").toBool())
                pBox->SetBool("UseVolumeSerialNumbers", true);

            if (field("autoRemove").toBool()) {
                pBox->SetBool("AutoDelete", true);
                pBox->SetBool("AutoRemove", true);
            }
            else if(field("autoDelete").toBool())
                pBox->SetBool("AutoDelete", true);
            if(field("autoRecover").toBool())
                pBox->SetBool("AutoRecover", true);

            if (field("blockNetwork").toInt() == 1) { // device based
                //pBox->AppendText("AllowNetworkAccess", "<BlockNetAccess>,n");
                pBox->AppendText("ClosedFilePath", "!<InternetAccess>,InternetAccessDevices");
                //pBox->AppendText("ClosedFilePath", "<BlockNetDevices>,InternetAccessDevices");
            }
            else if (field("blockNetwork").toInt() == 2) { // using WFP
                pBox->AppendText("AllowNetworkAccess", "!<InternetAccess>,n");
                //pBox->AppendText("AllowNetworkAccess", "<BlockNetAccess>,n");
                //pBox->AppendText("ClosedFilePath", "<BlockNetDevices>,InternetAccessDevices");
            }
            pBox->SetBool("BlockNetworkFiles", !field("shareAccess").toBool());

            bool bAllowNetwork = field("blockNetwork").toInt() == 0;
            if (field("promptAccess").toBool() && !bAllowNetwork)
                pBox->SetBool("PromptForInternetAccess", true);

            bool bHardened = (BoxType == CSandBoxPlus::eHardenedPlus || BoxType == CSandBoxPlus::eHardened);
            bool bAppBox = (BoxType == CSandBoxPlus::eAppBoxPlus || BoxType == CSandBoxPlus::eAppBox);
            bool bDropAdmin = field("dropAdmin").toBool();
            if (field("dropAdmin").toBool() && !bHardened)
                pBox->SetBool("DropAdminRights", true);

            if (field("fakeAdmin").toBool())
                pBox->SetBool("FakeAdminRights", true);

            if(field("msiServer").toBool() && !bDropAdmin && !bHardened)
                pBox->SetBool("MsiInstallerExemptions", true);

            if(field("boxToken").toBool() && !bAppBox)
                pBox->SetBool("SandboxieLogon", true);

            if(field("imagesProtection").toBool())
                pBox->SetBool("ProtectHostImages", true);

            if (field("coverBoxedWindows").toBool())
                pBox->SetBool("CoverBoxedWindows", true);

            if (!Password.isEmpty())
                pBox->ImBoxCreate(ImageSize / 1024, Password);

            if (field("boxVersion").toInt() == 1) {
                if (theConf->GetBool("Options/WarnDeleteV2", true)) {
                    bool State = false;
                    CCheckableMessageBox::question(this, "Sandboxie-Plus",
                        tr("The new sandbox has been created using the new <a href=\"https://sandboxie-plus.com/go.php?to=sbie-delete-v2\">Virtualization Scheme Version 2</a>, if you experience any unexpected issues with this box,"
                            " please switch to the Virtualization Scheme to Version 1 and report the issue,"
                            " the option to change this preset can be found in the Box Options in the Box Structure group.")
                        , tr("Don't show this message again."), &State, QDialogButtonBox::Ok, QDialogButtonBox::Ok, QMessageBox::Information);

                    if (State)
                        theConf->SetValue("Options/WarnDeleteV2", false);
                }
            }
        }
    }

    return Status;
}

QString CNewBoxWizard::GetDefaultLocation()
{
    QString DefaultPath = theAPI->GetGlobalSettings()->GetText("FileRootPath", "\\??\\%SystemDrive%\\Sandbox\\%USER%\\%SANDBOX%", false, false);
    // HACK HACK: globally %SANDBOX% evaluates to GlobalSettings
    DefaultPath.replace("\\GlobalSettings", "\\" + field("boxName").toString().replace(" ", "_"));
    return theAPI->Nt2DosPath(DefaultPath);
}


//////////////////////////////////////////////////////////////////////////////////////////
// CBoxTypePage
// 

CBoxTypePage::CBoxTypePage(bool bAlowTemp, QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Create new Sandbox"));
    QPixmap Logo = QPixmap(theGUI->m_DarkTheme ? ":/SideLogoDM.png" : ":/SideLogo.png");
    int Scaling = theConf->GetInt("Options/FontScaling", 100);
    if(Scaling !=  100) Logo = Logo.scaled(Logo.width() * Scaling / 100, Logo.height() * Scaling / 100);
    setPixmap(QWizard::WatermarkPixmap, Logo);

    m_bInstant = theConf->GetBool("Options/InstantBoxWizard", false);

    int row = 0;
    QGridLayout *layout = new QGridLayout;
#ifndef USE_COMBO
    layout->setSpacing(2);
    QLabel* pTopLabel = new QLabel(tr("A sandbox isolates your host system from processes running within the box, "
        "it prevents them from making permanent changes to other programs and data in your computer. "));
#else
    QLabel* pTopLabel = new QLabel(tr("A sandbox isolates your host system from processes running within the box, "
        "it prevents them from making permanent changes to other programs and data in your computer. "
        "The level of isolation impacts your security as well as the compatibility with applications, "
        "hence there will be a different level of isolation depending on the selected Box Type. "
        "Sandboxie can also protect your personal data from being accessed by processes running under its supervision."));
#endif
    pTopLabel->setWordWrap(true);
    layout->addWidget(pTopLabel, row++ , 0, 1, 3);

    layout->addItem(new QSpacerItem(0, 3), row++, 0);

    layout->addWidget(new QLabel(tr("Enter box name:")), row++, 0);

    m_pBoxName = new QLineEdit();
    m_pBoxName->setMaxLength(32); // BOXNAME_COUNT
    m_pBoxName->setText(theAPI->MkNewName("New Box"));
    m_pBoxName->setFocus();
    layout->addWidget(m_pBoxName, row++, 1, 1, 2);
    registerField("boxName", m_pBoxName);


    /*QLabel* pMore = new QLabel(tr("<a href=\"more\">More Types</a>"));
    pMore->setAlignment(Qt::AlignRight);
    connect(pMore, SIGNAL(linkActivated(const QString&)), this, SLOT(SnowMore()));
    layout->addWidget(pMore, row, 2);*/

    layout->addWidget(new QLabel(tr("Select box type:")), row++, 0);

#ifndef USE_COMBO
    m_TypeGroup = new QButtonGroup();

    auto AddBoxType = [&](const QString& label, int Type, const QString& tip = QString(), bool bCheck = false) {
        QAbstractButton* pC = bCheck ? (QAbstractButton*)new QCheckBox() : (QAbstractButton*)new QRadioButton();
        if (theGUI->m_DarkTheme) {
            QPalette palette = QApplication::palette();
            palette.setColor(QPalette::Base, Qt::white);
            palette.setColor(QPalette::Text, Qt::black);
            pC->setPalette(palette);
        }
        pC->setToolTip(tip);
        if(!bCheck) m_TypeGroup->addButton((QRadioButton*)pC, Type);
        QHBoxLayout* pLayout = new QHBoxLayout();
        pLayout->setContentsMargins(0,0,0,0);
        pLayout->setSpacing(4);
        pC->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
        pLayout->addWidget(pC);
        QLabel* pLabel = new QLabel(label);
        pLabel->setToolTip(tip);
        pLayout->addWidget(pLabel);
        connect(pLabel, SIGNAL(linkActivated(const QString&)), theGUI, SLOT(OpenUrl(const QString&)));
        QWidget* pW = new QWidget();
        pW->setLayout(pLayout);
        layout->addWidget(pW, row, 1, 1, 2);
        if (Type != -1) {
            QLabel* pIcon = new QLabel();
            pIcon->setPixmap(theGUI->GetBoxIcon(Type).pixmap(16, 16));
            pIcon->setAlignment(Qt::AlignRight);
            pIcon->setContentsMargins(0, 2, 4, 0);
            pIcon->setToolTip(tip);
            layout->addWidget(pIcon, row, 0);
        }
        row++;
        //return qMakePair(pW, pIcon);
        return pC;
    };

    AddBoxType(tr("<a href=\"sbie://docs/security-mode\">Security Hardened</a> Sandbox with <a href=\"sbie://docs/privacy-mode\">Data Protection</a>"), (int)CSandBoxPlus::eHardenedPlus, 
        tr("This box type offers the highest level of protection by significantly reducing the attack surface exposed to sandboxed processes. \n"
            "It strictly limits access to user data, allowing processes within this box to only access C:\\Windows and C:\\Program Files directories. \n"
            "The entire user profile remains hidden, ensuring maximum security."));
    AddBoxType(tr("<a href=\"sbie://docs/security-mode\">Security Hardened</a> Sandbox"), (int)CSandBoxPlus::eHardened, 
        tr("This box type offers the highest level of protection by significantly reducing the attack surface exposed to sandboxed processes."));
    AddBoxType(tr("Sandbox with <a href=\"sbie://docs/privacy-mode\">Data Protection</a>"), (int)CSandBoxPlus::eDefaultPlus, 
        tr("In this box type, sandboxed processes are prevented from accessing any personal user files or data. The focus is on protecting user data, and as such, \n"
            "only C:\\Windows and C:\\Program Files directories are accessible to processes running within this sandbox. This ensures that personal files remain secure."));
    AddBoxType(tr("Standard Sandbox"), (int)CSandBoxPlus::eDefault, 
        tr("This box type offers the default behavior of Sandboxie classic. It provides users with a familiar and reliable sandboxing scheme. \n"
            "Applications can be run within this sandbox, ensuring they operate within a controlled and isolated space."));
    AddBoxType(tr("<a href=\"sbie://docs/compartment-mode\">Application Compartment</a> Box with <a href=\"sbie://docs/privacy-mode\">Data Protection</a>"), (int)CSandBoxPlus::eAppBoxPlus, 
        tr("This box type prioritizes compatibility while still providing a good level of isolation. It is designed for running trusted applications within separate compartments. \n"
            "While the level of isolation is reduced compared to other box types, it offers improved compatibility with a wide range of applications, ensuring smooth operation within the sandboxed environment."));
    AddBoxType(tr("<a href=\"sbie://docs/compartment-mode\">Application Compartment</a> Box"), (int)CSandBoxPlus::eAppBox, 
        tr("This box type prioritizes compatibility while still providing a good level of isolation. It is designed for running trusted applications within separate compartments. \n"
            "While the level of isolation is reduced compared to other box types, it offers improved compatibility with a wide range of applications, ensuring smooth operation within the sandboxed environment."));

    QWidget* pGap = new QWidget();
    pGap->setMinimumHeight(4);
    layout->addWidget(pGap, row++, 1, 1, 2);

    //AddBoxType(tr("<a href=\"sbie://docs/boxencryption\">Encrypted</a> <a href=\"sbie://docs/black-box\">Confidential</a> Box"), (int)CSandBoxPlus::ePrivate,
    QWidget* pBlackBox = AddBoxType(tr("<a href=\"sbie://docs/boxencryption\">Encrypt</a> Box content and set <a href=\"sbie://docs/black-box\">Confidential</a>"), (int)CSandBoxPlus::ePrivate,
        tr("In this box type the sandbox uses an encrypted disk image as its root folder. This provides an additional layer of privacy and security. \n"
            "Access to the virtual disk when mounted is restricted to programs running within the sandbox. Sandboxie prevents other processes on the host system from accessing the sandboxed processes. \n"
            "This ensures the utmost level of privacy and data protection within the confidential sandbox environment."), true);

    connect(m_TypeGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SIGNAL(typeChanged()));
    registerField("boxType", this, "currentType", "typeChanged");
    connect(m_TypeGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(OnBoxTypChanged()));

    connect(pBlackBox, SIGNAL(toggled(bool)), this, SIGNAL(typeChanged()));
    registerField("blackBox", pBlackBox);
    connect(pBlackBox, SIGNAL(toggled(bool)), this, SLOT(OnBoxTypChanged()));


    //QCheckBox* pMore = new QCheckBox(tr("Show More Types"));
    //layout->addWidget(pMore, 4, 3);
    //connect(pMore, &QCheckBox::toggled, [=](bool bValue) {
    //          ...
    //    });
#else
    bool bAll = true;

    m_pBoxType = new QComboBox();
	m_pBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eHardenedPlus), tr("Hardened Sandbox with Data Protection"), (int)CSandBoxPlus::eHardenedPlus);
	if (bAll) m_pBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eHardened), tr("Security Hardened Sandbox"), (int)CSandBoxPlus::eHardened);
	m_pBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eDefaultPlus), tr("Sandbox with Data Protection"), (int)CSandBoxPlus::eDefaultPlus);
	m_pBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eDefault), tr("Standard Isolation Sandbox (Default)"), (int)CSandBoxPlus::eDefault);
	//m_pBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eInsecure), tr("INSECURE Configuration (please change)"), (int)CSandBoxPlus::eInsecure);
	if (bAll) m_pBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eAppBoxPlus), tr("Application Compartment with Data Protection"), (int)CSandBoxPlus::eAppBoxPlus);
	m_pBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eAppBox), tr("Application Compartment Box"), (int)CSandBoxPlus::eAppBox);
    m_pBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::ePrivate), tr("Confidential Encrypted Box"), (int)CSandBoxPlus::ePrivate);
    connect(m_pBoxType, SIGNAL(currentIndexChanged(int)), this, SLOT(OnBoxTypChanged()));
    layout->addWidget(m_pBoxType, row++, 1, 1, 2);
    registerField("boxType", m_pBoxType, "currentData", "currentIndexChanged");

    m_pInfoLabel = new QLabel();
    m_pInfoLabel->setWordWrap(true);
    //m_pInfoLabel->setOpenExternalLinks(true);
    connect(m_pInfoLabel, SIGNAL(linkActivated(const QString&)), theGUI, SLOT(OpenUrl(const QString&)));
    layout->addWidget(m_pInfoLabel, row++, 0, 1, 3);

    m_pBoxType->setCurrentIndex(3); // default
#endif

    QWidget* pSpacer = new QWidget();
	pSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(pSpacer, row++, 1);

    QCheckBox* pTemp = new QCheckBox(tr("Remove after use"));
    pTemp->setToolTip(tr("After the last process in the box terminates, all data in the box will be deleted and the box itself will be removed."));
    layout->addWidget(pTemp, row, 0, 1, 2);
    pTemp->setVisible(bAlowTemp);
    registerField("autoRemove", pTemp);

    m_pAdvanced = new QCheckBox(tr("Configure advanced options"));
    if (theGUI->m_DarkTheme) {
        QPalette palette = QApplication::palette();
        palette.setColor(QPalette::Base, Qt::white);
        palette.setColor(QPalette::Text, Qt::black);
        m_pAdvanced->setPalette(palette);
    }
    layout->addWidget(m_pAdvanced, row++, 2, 1, 1);
    connect(m_pAdvanced, SIGNAL(toggled(bool)), this, SLOT(OnAdvanced()));

    setLayout(layout);
}


void CBoxTypePage::initializePage()
{
    m_pAdvanced->setChecked(((CNewBoxWizard*)wizard())->m_bAdvanced);
}

void CBoxTypePage::setCurrentType(int type) 
{
    if(m_TypeGroup->buttons().count() < type)
        m_TypeGroup->button(type)->setChecked(true);
}

int CBoxTypePage::currentType() 
{ 
    return m_TypeGroup->checkedId(); 
}

void CBoxTypePage::OnBoxTypChanged()
{
#ifndef USE_COMBO
    int BoxType = m_TypeGroup->checkedId();
    bool BlackBox = field("blackBox").toBool();
#else
	int BoxType = m_pBoxType->currentData().toInt();
    bool BlackBox = CSandBoxPlus::ePrivate;

	m_pInfoLabel->setText(theGUI->GetBoxDescription(BoxType));
#endif

    if(BoxType != CSandBoxPlus::eDefault || BlackBox)
		theGUI->CheckCertificate(this, BlackBox ? 1 : 0);

    emit completeChanged();
}

void CBoxTypePage::OnAdvanced()
{
    ((CNewBoxWizard*)wizard())->m_bAdvanced = m_pAdvanced->isChecked();
    if (m_bInstant)
    {
        QString BoxName = m_pBoxName->text();
#ifdef USE_COMBO
        int BoxType = m_pBoxType->currentIndex();
#endif

        wizard()->restart();

        m_pBoxName->setText(BoxName);
#ifdef USE_COMBO
        m_pBoxType->setCurrentIndex(BoxType);
#endif
    }
}

int CBoxTypePage::nextId() const
{
    if (!m_pAdvanced->isChecked()) {
        if(m_bInstant)
            return -1;
        return CNewBoxWizard::Page_Summary;
    }
    return CNewBoxWizard::Page_Files;
}

bool CBoxTypePage::isComplete() const
{
#ifndef USE_COMBO
    if (m_TypeGroup->checkedId() == -1)
        return false;
#endif
    return true;
}

bool CBoxTypePage::validatePage()
{
    QString BoxName = field("boxName").toString();
    if (!theGUI->GetBoxView()->TestNameAndWarn(BoxName))
        return false;

#ifndef USE_COMBO
    int BoxType = m_TypeGroup->checkedId();
    bool BlackBox = field("blackBox").toBool();
#else
	int BoxType = m_pBoxType->currentData().toInt();
    bool BlackBox = (BoxType == CSandBoxPlus::ePrivate || BoxType == CSandBoxPlus::ePrivatePlus);
#endif

    if (BlackBox && !theGUI->IsImDiskReady()) {
        theGUI->GetAddonManager()->TryInstallAddon("ImDisk", this, tr("To use encrypted boxes you need to install the ImDisk driver, do you want to download and install it?"));
        return false;
    }

    if (m_bInstant && !m_pAdvanced->isChecked())
        return !((CNewBoxWizard*)wizard())->TryToCreateBox().IsError();

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// CFilesPage
// 

CFilesPage::CFilesPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Sandbox location and behavior"));
    setSubTitle(tr("On this page the sandbox location and its behavior can be customized.\nYou can use %USER% to save each users sandbox to an own folder."));

    int row = 0;
    QGridLayout *layout = new QGridLayout;

    QLabel* pFileLabel = new QLabel(tr("Sandboxed Files"), this);
	QFont fnt = pFileLabel->font();
	fnt.setBold(true);
	//fnt.setWeight(QFont::DemiBold);
	pFileLabel->setFont(fnt);
    layout->addWidget(pFileLabel, row++, 0);
    layout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 2, 1, 1);


    // Location
    QLineEdit* pDummy = new QLineEdit();
    pDummy->setVisible(false);
    layout->addWidget(pDummy, row, 0);
    registerField("boxLocation", pDummy);

	QHBoxLayout* pLayout = new QHBoxLayout();
	pLayout->setContentsMargins(0,0,0,0);
	m_pBoxLocation = new QComboBox();
    m_pBoxLocation->setEditable(true);
	pLayout->addWidget(m_pBoxLocation);
	QPushButton* pButton = new QPushButton("...");
	pButton->setMaximumWidth(25);
    connect(pButton, &QPushButton::clicked, [&]() {
        QString FilePath = QFileDialog::getExistingDirectory(this, tr("Select Directory"));
	    if (!FilePath.isEmpty())
		    this->m_pBoxLocation->setCurrentText(FilePath.replace("/", "\\"));
    });
	pLayout->addWidget(pButton);
    layout->addLayout(pLayout, row++, 1, 1, 3);
    //

    QLabel* pVersionLbl = new QLabel(tr("Virtualization scheme"), this);
    layout->addWidget(pVersionLbl, row, 1);

    QComboBox* pVersion = new QComboBox();
    pVersion->addItem(tr("Version 1"));
	pVersion->addItem(tr("Version 2"));
    layout->addWidget(pVersion, row++, 2);
    pVersion->setCurrentIndex(theConf->GetInt("BoxDefaults/BoxScheme", 2) - 1); // V2 default
    layout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 3, 1, 1);
    registerField("boxVersion", pVersion);

    QCheckBox* pUserFolders = new QCheckBox(tr("Separate user folders"));
    pUserFolders->setChecked(theConf->GetBool("BoxDefaults/SeparateUser", true));
    layout->addWidget(pUserFolders, row++, 2, 1, 2);
    registerField("separateUser", pUserFolders);

    QCheckBox* pUseVolumeSN = new QCheckBox(tr("Use volume serial numbers for drives"));
    pUseVolumeSN->setChecked(theConf->GetBool("BoxDefaults/UseVolumeSN", false));
    layout->addWidget(pUseVolumeSN, row++, 2, 1, 2);
    registerField("useVolumeSN", pUseVolumeSN);

    QCheckBox* pAutoDelete = new QCheckBox(tr("Auto delete content when last process terminates"));
    pAutoDelete->setChecked(theConf->GetBool("BoxDefaults/AutoDelete", false));
    layout->addWidget(pAutoDelete, row++, 1, 1, 3);
    if (field("autoRemove").toBool())
        pAutoDelete->setEnabled(false);
    registerField("autoDelete", pAutoDelete);

    QCheckBox* pAutoRecover = new QCheckBox(tr("Enable Immediate Recovery of files from recovery locations"));
    pAutoRecover->setChecked(theConf->GetBool("BoxDefaults/AutoRecover", true));
    layout->addWidget(pAutoRecover, row++, 1, 1, 3);
    registerField("autoRecover", pAutoRecover);


    setLayout(layout);


	int size = 16.0;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	size *= (QApplication::desktop()->logicalDpiX() / 96.0); // todo Qt6
#endif
    AddIconToLabel(pFileLabel, CSandMan::GetIcon("Folder").pixmap(size,size));
}

int CFilesPage::nextId() const
{
    return CNewBoxWizard::Page_Isolation;
}

void CFilesPage::initializePage()
{
    m_pBoxLocation->clear();
    QString Location = theAPI->GetGlobalSettings()->GetText("FileRootPath", "\\??\\%SystemDrive%\\Sandbox\\%USER%\\%SANDBOX%");
    m_pBoxLocation->addItem(Location/*.replace("%SANDBOX%", field("boxName").toString())*/);
    QStringList StdLocations = QStringList() 
        << "\\??\\%SystemDrive%\\Sandbox\\%USER%\\%SANDBOX%" 
        << "\\??\\%SystemDrive%\\Sandbox\\%SANDBOX%" 
        << "\\??\\%SystemDrive%\\Users\\%USER%\\Sandbox\\%SANDBOX%";
    foreach(auto StdLocation, StdLocations) {
        if (StdLocation != Location)
            m_pBoxLocation->addItem(StdLocation);
    }
}

bool CFilesPage::validatePage()
{
    QString Location = m_pBoxLocation->currentText();
    if (Location == m_pBoxLocation->itemText(0))
        wizard()->setField("boxLocation", "");
    else {
        int offset = Location.left(4) == "\\??\\" ? 4 : 0;
        if (Location.length() < offset + 4) {
            QMessageBox::critical(this, "Sandboxie-Plus", tr("A sandbox cannot be located at the root of a partition, please select a folder."));
            return false;
        }
        if (Location.left(2) == "\\\\") {
            QMessageBox::critical(this, "Sandboxie-Plus", tr("A sandbox cannot be located on a network share, please select a local folder."));
            return false;
        }
        if (Location.mid(offset + 2).contains(QRegularExpression("[ <>:\"/\\|?*\\[\\]]"))){
            QMessageBox::critical(this, "Sandboxie-Plus", tr("The selected box location is not a valid path."));
            return false;
        }
        QDir Dir(Location);
        if (Dir.exists() && !Dir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty()) {
            if(QMessageBox::warning(this, "Sandboxie-Plus", tr("The selected box location exists and is not empty, it is recommended to pick a new or empty folder. "
                "Are you sure you want to use an existing folder?"), QDialogButtonBox::Yes, QDialogButtonBox::No) != QDialogButtonBox::Yes)
                return false;
        }
        if (Location.mid(offset, 13).compare("%SystemDrive%") != 0 && !QDir().exists(Location.mid(offset, 3))) {
            QMessageBox::critical(this, "Sandboxie-Plus", tr("The selected box location is not placed on a currently available drive."));
            return false;
        }
        wizard()->setField("boxLocation", Location);
    }
    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// CIsolationPage
// 

CIsolationPage::CIsolationPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Sandbox Isolation options"));
    setSubTitle(tr("On this page sandbox isolation options can be configured."));

    int row = 0;
    QGridLayout *layout = new QGridLayout;

    QLabel* pNetLabel = new QLabel(tr("Network Access"), this);
    QFont fnt = pNetLabel->font();
	fnt.setBold(true);
	//fnt.setWeight(QFont::DemiBold);
    pNetLabel->setFont(fnt);
    layout->addWidget(pNetLabel, row++, 0);

    QComboBox* pNetAccess = new QComboBox();
    pNetAccess->addItem(tr("Allow network/internet access"));
	pNetAccess->addItem(tr("Block network/internet by denying access to Network devices"));
	if (theGUI->IsWFPEnabled()) 
        pNetAccess->addItem(tr("Block network/internet using Windows Filtering Platform"));
    pNetAccess->setCurrentIndex(theConf->GetInt("BoxDefaults/BlockNetwork", 0));
    layout->addWidget(pNetAccess, row++, 1, 1, 3);
    connect(pNetAccess, SIGNAL(currentIndexChanged(int)), this, SLOT(OnBlockNetworkChanged(int)));
    registerField("blockNetwork", pNetAccess);

    m_pShareAccess = new QCheckBox(tr("Allow access to network files and folders"));
    m_pShareAccess->setToolTip(tr("This option is not recommended for Hardened boxes"));
    m_pShareAccess->setChecked(theConf->GetBool("BoxDefaults/ShareAccess", false));
    layout->addWidget(m_pShareAccess, row++, 1, 1, 3);
    registerField("shareAccess", m_pShareAccess);

    m_pPromptAccess = new QCheckBox(tr("Prompt user whether to allow an exemption from the blockade"));
    m_pPromptAccess->setChecked(theConf->GetBool("BoxDefaults/PromptAccess", false));
    layout->addWidget(m_pPromptAccess, row++, 1, 1, 3);
    registerField("promptAccess", m_pPromptAccess);


    QLabel* pAdminLabel = new QLabel(tr("Admin Options"), this);
    pAdminLabel->setFont(fnt);
    layout->addWidget(pAdminLabel, row++, 0);

    m_pDropAdmin = new QCheckBox(tr("Drop rights from Administrators and Power Users groups"));
    m_pDropAdmin->setChecked(theConf->GetBool("BoxDefaults/DropAdmin", false));
    layout->addWidget(m_pDropAdmin, row++, 1, 1, 3);
    connect(m_pDropAdmin, &QCheckBox::stateChanged, this, &CIsolationPage::OnDropAdminChanged);
    registerField("dropAdmin", m_pDropAdmin);

    QCheckBox* pFakeAdmin = new QCheckBox(tr("Make applications think they are running elevated"));
    pFakeAdmin->setChecked(theConf->GetBool("BoxDefaults/FakeAdmin", false));
    layout->addWidget(pFakeAdmin, row++, 1, 1, 3);
    registerField("fakeAdmin", pFakeAdmin);

    m_pMSIServer = new QCheckBox(tr("Allow MSIServer to run with a sandboxed system token"));
    m_pMSIServer->setToolTip(tr("This option is not recommended for Hardened boxes"));
    if (!theConf->GetBool("BoxDefaults/DropAdmin", false))
        m_pMSIServer->setChecked(theConf->GetBool("BoxDefaults/MsiExemptions", false));
    layout->addWidget(m_pMSIServer, row++, 1, 1, 3);
    registerField("msiServer", m_pMSIServer);

    QLabel* pBoxLabel = new QLabel(tr("Box Options"), this);
    pBoxLabel->setFont(fnt);
    layout->addWidget(pBoxLabel, row++, 0);

    m_pBoxToken = new QCheckBox(tr("Use a Sandboxie login instead of an anonymous token"));
    m_pBoxToken->setToolTip(tr("Using a custom Sandboxie Token allows to isolate individual sandboxes from each other better, and it shows in the user column of task managers the name of the box a process belongs to. Some 3rd party security solutions may however have problems with custom tokens."));
    m_pBoxToken->setChecked(theConf->GetBool("BoxDefaults/BoxToken", false));
    layout->addWidget(m_pBoxToken, row++, 1, 1, 3);
    registerField("boxToken", m_pBoxToken);

    setLayout(layout);

	int size = 16.0;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	size *= (QApplication::desktop()->logicalDpiX() / 96.0); // todo Qt6
#endif
    AddIconToLabel(pNetLabel, CSandMan::GetIcon("Network").pixmap(size,size));
    AddIconToLabel(pAdminLabel, CSandMan::GetIcon("Shield9").pixmap(size,size));
    AddIconToLabel(pBoxLabel, CSandMan::GetIcon("Sandbox").pixmap(size,size));
}

int CIsolationPage::nextId() const
{
    return CNewBoxWizard::Page_Advanced;
}

void CIsolationPage::initializePage()
{
    int BoxType = wizard()->field("boxType").toInt();

    bool bHardened = (BoxType == CSandBoxPlus::eHardenedPlus || BoxType == CSandBoxPlus::eHardened);
    bool bDropAdmin = field("dropAdmin").toBool();
    m_pMSIServer->setEnabled(!bHardened && !bDropAdmin);
    m_pShareAccess->setEnabled(!bHardened);
    m_pDropAdmin->setEnabled(!bHardened);
    m_pDropAdmin->setChecked(bDropAdmin || bHardened);

    bool bAppBox = (BoxType == CSandBoxPlus::eAppBoxPlus || BoxType == CSandBoxPlus::eAppBox);
    bool bBoxToken = field("boxToken").toBool();
    m_pBoxToken->setEnabled(!bAppBox);
    m_pBoxToken->setChecked(!bAppBox && bBoxToken);

    bool bAllowNetwork = field("blockNetwork").toInt() == 0;
    bool bPromptAccess = field("promptAccess").toBool();
    m_pPromptAccess->setEnabled(!bAllowNetwork);
    m_pPromptAccess->setChecked(!bAllowNetwork && bPromptAccess);
}

bool CIsolationPage::validatePage()
{
    return true;
}

void CIsolationPage::OnDropAdminChanged(int state) {
    // If m_pDropAdmin is checked, disable m_pMSIServer
    if (state == Qt::Checked) {
        m_pMSIServer->setEnabled(false);
        m_pMSIServer->setChecked(false);
    }
    else {
        // If m_pDropAdmin is unchecked, enable m_pMSIServer
        m_pMSIServer->setEnabled(true);
        m_pMSIServer->setChecked(theConf->GetBool("BoxDefaults/MsiExemptions", false));
    }
}

void CIsolationPage::OnBlockNetworkChanged(int index) {
    if (index == 0) {
        // If network access is allowed, disable m_pPromptAccess
        m_pPromptAccess->setEnabled(false);
        m_pPromptAccess->setChecked(false);
    }
    else {
        // If network access is blocked, enable m_pPromptAccess
        m_pPromptAccess->setEnabled(true);
        m_pPromptAccess->setChecked(theConf->GetBool("BoxDefaults/PromptAccess", false));
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// CAdvancedPage
// 

CAdvancedPage::CAdvancedPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Advanced Sandbox options"));
    setSubTitle(tr("On this page advanced sandbox options can be configured."));

    int row = 0;
    QGridLayout *layout = new QGridLayout;

    QLabel* pBoxLabel = new QLabel(tr("Advanced Options"), this);
    QFont fnt = pBoxLabel->font();
	fnt.setBold(true);
	//fnt.setWeight(QFont::DemiBold);
    pBoxLabel->setFont(fnt);
    layout->addWidget(pBoxLabel, row++, 0);

    QCheckBox* pImageProtection = new QCheckBox(tr("Prevent sandboxed programs on the host from loading sandboxed DLLs"));
    pImageProtection->setToolTip(tr("This feature may reduce compatibility as it also prevents box located processes from writing to host located ones and even starting them."));
    pImageProtection->setChecked(theConf->GetBool("BoxDefaults/ImagesProtection", false));
    pImageProtection->setEnabled(g_CertInfo.active);
    layout->addWidget(pImageProtection, row++, 1, 1, 3);
    registerField("imagesProtection", pImageProtection);

	QCheckBox* pWindowCover = new QCheckBox(tr("Prevent sandboxed windows from being captured"));
	pWindowCover->setToolTip(tr("This feature can cause a decline in the user experience because it also prevents normal screenshots."));
	pWindowCover->setChecked(theConf->GetBool("BoxDefaults/CoverBoxedWindows", false));
	layout->addWidget(pWindowCover, row++, 1, 1, 3);
	registerField("coverBoxedWindows", pWindowCover);

	QString SharedTemplateName = tr("Shared Template");
	QLabel* pSharedTemplateLbl = new QLabel(tr("Shared template mode"), this);
    pSharedTemplateLbl->setToolTip(tr("This setting adds a local template or its settings to the sandbox configuration so that the settings in that template are shared between sandboxes."
        "\nHowever, if 'use as a template' option is selected as the sharing mode, some settings may not be reflected in the user interface."
        "\nTo change the template's settings, simply locate the '%1' template in the App Templates list under Sandbox Options, then double-click on it to edit it."
        "\nTo disable this template for a sandbox, simply uncheck it in the template list.").arg(SharedTemplateName));
    layout->addWidget(pSharedTemplateLbl, row, 1);

    QString SharedTemplateTip0 = tr("This option does not add any settings to the box configuration and does not remove the default box settings based on the removal settings within the template.");
    QString SharedTemplateTip1 = tr("This option adds the shared template to the box configuration as a local template and may also remove the default box settings based on the removal settings within the template.");
    QString SharedTemplateTip2 = tr("This option adds the settings from the shared template to the box configuration and may also remove the default box settings based on the removal settings within the template.");
    QString SharedTemplateTip3 = tr("This option does not add any settings to the box configuration, but may remove the default box settings based on the removal settings within the template.");
    m_pSharedTemplate = new QComboBox();
    m_pSharedTemplate->addItem(tr("Disabled"));
    m_pSharedTemplate->setItemData(0, SharedTemplateTip0, Qt::ToolTipRole);
    m_pSharedTemplate->addItem(tr("Use as a template"));
    m_pSharedTemplate->setItemData(1, SharedTemplateTip1, Qt::ToolTipRole);
    m_pSharedTemplate->addItem(tr("Append to the configuration"));
    m_pSharedTemplate->setItemData(2, SharedTemplateTip2, Qt::ToolTipRole);
    m_pSharedTemplate->addItem(tr("Remove defaults if set"));
    m_pSharedTemplate->setItemData(3, SharedTemplateTip3, Qt::ToolTipRole);
    layout->addWidget(m_pSharedTemplate, row++, 2);
    m_pSharedTemplate->setCurrentIndex(theConf->GetInt("BoxDefaults/SharedTemplate", 0));
    layout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 4, 1, 1);
    registerField("sharedTemplate", m_pSharedTemplate);

    QLabel* pSharedTemplateIndexLbl = new QLabel(tr("Shared template selection"), this);
    layout->addWidget(pSharedTemplateIndexLbl, row, 1);

    m_pSharedTemplateIndex = new QComboBox();
    QStringList templateOptions;
    QStringList templateToolTips;

    for (int i = 0; i <= 9; ++i) {
        QString templateName = (i == 0)
            ? QString("SharedTemplate")
            : QString("SharedTemplate_%1").arg(i);
        QString templateFullName = "Template_Local_" + templateName;
        QString templateTitle = theAPI->SbieIniGetEx(templateFullName, "Tmpl.Title");

        // Determine the template name, including the index if not zero
        QString templateNameCustom = templateTitle.isEmpty()
            ? (i == 0 ? SharedTemplateName : SharedTemplateName + " " + QString::number(i))
            : templateTitle;

        templateOptions << templateNameCustom;

        // Set tooltip text using the combined template name
        QString toolTipText = tr("This option specifies the template to be used in shared template mode. (%1)").arg(templateFullName);
        templateToolTips << toolTipText;
    }

    // Set options to the combobox
    m_pSharedTemplateIndex->addItems(templateOptions);

    // Set tooltips for each item
    for (int i = 0; i < templateOptions.size(); ++i) {
        m_pSharedTemplateIndex->setItemData(i, templateToolTips[i], Qt::ToolTipRole);
    }

    layout->addWidget(m_pSharedTemplateIndex, row++, 2);
    m_pSharedTemplateIndex->setCurrentIndex(theConf->GetInt("BoxDefaults/SharedTemplateIndex", 0));
    registerField("sharedTemplateIndex", m_pSharedTemplateIndex);


    setLayout(layout);

    // Connect the combo box signal to the slot
    connect(m_pSharedTemplate, qOverload<int>(&QComboBox::currentIndexChanged),
        this, &CAdvancedPage::OnSharedTemplateIndexChanged);

    // Initial call to set the state based on the default index
    OnSharedTemplateIndexChanged(m_pSharedTemplate->currentIndex());


	int size = 16.0;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	size *= (QApplication::desktop()->logicalDpiX() / 96.0); // todo Qt6
#endif
    AddIconToLabel(pBoxLabel, CSandMan::GetIcon("Advanced").pixmap(size,size));
}

void CAdvancedPage::OnSharedTemplateIndexChanged(int index)
{
    if (m_pSharedTemplateIndex) {
        bool enable = (index != 0);
        m_pSharedTemplateIndex->setEnabled(enable);
    }
}

int CAdvancedPage::nextId() const
{
    return CNewBoxWizard::Page_Summary;
}

void CAdvancedPage::initializePage()
{
}

bool CAdvancedPage::validatePage()
{
    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// CSummaryPage
// 

CSummaryPage::CSummaryPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Create the new Sandbox"));
    QPixmap Logo = QPixmap(theGUI->m_DarkTheme ? ":/SideLogoDM.png" : ":/SideLogo.png");
    int Scaling = theConf->GetInt("Options/FontScaling", 100);
    if(Scaling !=  100) Logo = Logo.scaled(Logo.width() * Scaling / 100, Logo.height() * Scaling / 100);
    setPixmap(QWizard::WatermarkPixmap, Logo);

    int row = 0;
    QGridLayout *layout = new QGridLayout;

    QLabel* pLabel = new QLabel;
    pLabel->setWordWrap(true);
    pLabel->setText(tr("Almost complete, click Finish to create a new sandbox and conclude the wizard."));
    layout->addWidget(pLabel, row++ , 0, 1, 3);


    m_pSummary = new QTextEdit();
    m_pSummary->setReadOnly(true);
    m_pSummary->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(m_pSummary, row++ , 0, 1, 3);

    m_pSetDefault = new QCheckBox(tr("Save options as new defaults"));
    layout->addWidget(m_pSetDefault, row++, 2);

    //QWidget* pSpacer = new QWidget();
    //pSpacer->setMinimumHeight(16);
    //layout->addWidget(pSpacer);

    QWidget* pSpacer = new QWidget();
	pSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(pSpacer, row++, 1);

    m_pSetInstant = new QCheckBox(tr("Skip this summary page when advanced options are not set"));
    m_pSetInstant->setChecked(theConf->GetBool("Options/InstantBoxWizard", false));
    layout->addWidget(m_pSetInstant, row++, 1, 1, 2);

    setLayout(layout);
}

int CSummaryPage::nextId() const
{
    return -1;
}

void CSummaryPage::initializePage()
{
    m_pSummary->setText(theGUI->GetBoxDescription(wizard()->field("boxType").toInt()));

    QString Location = field("boxLocation").toString();
    if (Location.isEmpty())
        Location = ((CNewBoxWizard*)wizard())->GetDefaultLocation();
    m_pSummary->append(tr("\nThis Sandbox will be saved to: %1").arg(Location));

    if (field("autoRemove").toBool()) 
        m_pSummary->append(tr("\nThis box's content will be DISCARDED when it's closed, and the box will be removed."));
    else if (field("autoDelete").toBool())
        m_pSummary->append(tr("\nThis box will DISCARD its content when its closed, its suitable only for temporary data."));
    if (field("blockNetwork").toInt())
        m_pSummary->append(tr("\nProcesses in this box will not be able to access the internet or the local network, this ensures all accessed data to stay confidential."));
    if (field("msiServer").toBool())
        m_pSummary->append(tr("\nThis box will run the MSIServer (*.msi installer service) with a system token, this improves the compatibility but reduces the security isolation."));
    else if(field("fakeAdmin").toBool())
        m_pSummary->append(tr("\nProcesses in this box will think they are run with administrative privileges, without actually having them, hence installers can be used even in a security hardened box."));
    if(field("boxToken").toBool())
        m_pSummary->append(tr("\nProcesses in this box will be running with a custom process token indicating the sandbox they belong to."));


    m_pSetDefault->setVisible(((CNewBoxWizard*)wizard())->m_bAdvanced);
}

bool CSummaryPage::validatePage()
{
    if (m_pSetDefault->isChecked())
    {
        theConf->SetValue("BoxDefaults/BoxScheme", field("boxVersion").toInt() + 1);
        theConf->SetValue("BoxDefaults/SeparateUser", field("separateUser").toBool());
        theConf->SetValue("BoxDefaults/UseVolumeSN", field("useVolumeSN").toBool());


        theConf->SetValue("BoxDefaults/AutoDelete", field("autoDelete").toBool());
        theConf->SetValue("BoxDefaults/AutoRecover", field("autoRecover").toBool());

        theConf->SetValue("BoxDefaults/BlockNetwork", field("blockNetwork").toInt());
        theConf->SetValue("BoxDefaults/ShareAccess", field("shareAccess").toBool());
        theConf->SetValue("BoxDefaults/PromptAccess", field("promptAccess").toBool());

        theConf->SetValue("BoxDefaults/DropAdmin", field("dropAdmin").toBool());
        theConf->SetValue("BoxDefaults/FakeAdmin", field("fakeAdmin").toBool());
        theConf->SetValue("BoxDefaults/MsiExemptions", field("msiServer").toBool());

        theConf->SetValue("BoxDefaults/BoxToken", field("boxToken").toBool());
        theConf->SetValue("BoxDefaults/ImagesProtection", field("imagesProtection").toBool());
        theConf->SetValue("BoxDefaults/CoverBoxedWindows", field("coverBoxedWindows").toBool());
        theConf->SetValue("BoxDefaults/SharedTemplate", field("sharedTemplate").toInt());
        theConf->SetValue("BoxDefaults/SharedTemplateIndex", field("sharedTemplateIndex").toInt());
    }

    theConf->SetValue("Options/InstantBoxWizard", m_pSetInstant->isChecked());
    theConf->SetValue("Options/AdvancedBoxWizard", ((CNewBoxWizard*)wizard())->m_bAdvanced);

    SB_STATUS Status = ((CNewBoxWizard*)wizard())->TryToCreateBox();
    if (Status.IsError()) {
        if(Status.GetMsgCode() != SB_Canceled)
            QMessageBox::critical(this, "Sandboxie-Plus", tr("Failed to create new box: %1").arg(theGUI->FormatError(Status)));
        return false;
    }
    return true;
}
