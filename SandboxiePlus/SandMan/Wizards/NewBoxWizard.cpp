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


CNewBoxWizard::CNewBoxWizard(bool bAlowTemp, QWidget *parent)
    : QWizard(parent)
{
    setPage(Page_Type, new CBoxTypePage(bAlowTemp));
    setPage(Page_Files, new CFilesPage);
    setPage(Page_Advanced, new CAdvancedPage);
    setPage(Page_Summary, new CSummaryPage);
    
    m_bAdvanced = false;

    setWizardStyle(ModernStyle);
    //setOption(HaveHelpButton, true);
    setPixmap(QWizard::LogoPixmap, QPixmap(":/SandMan.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    connect(this, &QWizard::helpRequested, this, &CNewBoxWizard::showHelp);

    setWindowTitle(tr("New Box Wizard"));
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

	SB_STATUS Status = theAPI->CreateBox(BoxName, true);

	if (!Status.IsError())
	{
		CSandBoxPtr pBox = theAPI->GetBoxByName(BoxName);

		switch (BoxType)
		{
			case CSandBoxPlus::eHardenedPlus:
			case CSandBoxPlus::eHardened:
				pBox->SetBool("UseSecurityMode", true);
				if(BoxType == CSandBoxPlus::eHardenedPlus)
                    pBox->SetBool("UsePrivacyMode", true);
				break;
			case CSandBoxPlus::eDefaultPlus:
			case CSandBoxPlus::eDefault:
				pBox->SetBool("UseSecurityMode", false);
				if(BoxType == CSandBoxPlus::eDefaultPlus)
				    pBox->SetBool("UsePrivacyMode", true);
				break;
			case CSandBoxPlus::eAppBoxPlus:
			case CSandBoxPlus::eAppBox:
				pBox->SetBool("NoSecurityIsolation", true);
                if(BoxType == CSandBoxPlus::eAppBoxPlus)
				    pBox->SetBool("UsePrivacyMode", true);
				//pBox->InsertText("Template", "NoUACProxy"); // proxy is always needed for exes in the box
				pBox->InsertText("Template", "RpcPortBindingsExt");
				break;
		}

		QRgb rgb = theGUI->GetBoxColor(BoxType);
		pBox->SetText("BorderColor", QString("#%1%2%3").arg(qBlue(rgb), 2, 16, QChar('0')).arg(qGreen(rgb), 2, 16, QChar('0')).arg(qRed(rgb), 2, 16, QChar('0')) + ",ttl");


        QString Location = field("boxLocation").toString();
        if (!Location.isEmpty())
            pBox->SetText("FileRootPath", Location);

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
            //pBox->InsertText("AllowNetworkAccess", "<BlockNetAccess>,n");
            pBox->InsertText("ClosedFilePath", "!<InternetAccess>,InternetAccessDevices");
            //pBox->InsertText("ClosedFilePath", "<BlockNetDevices>,InternetAccessDevices");
        }
        else if (field("blockNetwork").toInt() == 2) { // using WFP
            pBox->InsertText("AllowNetworkAccess", "!<InternetAccess>,n");
            //pBox->InsertText("AllowNetworkAccess", "<BlockNetAccess>,n");
            //pBox->InsertText("ClosedFilePath", "<BlockNetDevices>,InternetAccessDevices");
        }
        pBox->SetBool("BlockNetworkFiles", !field("shareAccess").toBool());

        if(field("fakeAdmin").toBool())
            pBox->SetBool("FakeAdminRights", true);
        if(field("msiServer").toBool())
            pBox->SetBool("MsiInstallerExemptions", true);

        if(field("boxToken").toBool())
            pBox->SetBool("SandboxieLogon", true);

        if(field("imagesProtection").toBool())
            pBox->SetBool("ProtectHostImages", true);

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
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/SideLogo.png"));

    m_bInstant = theConf->GetBool("Options/InstantBoxWizard", false);

    int row = 0;
    QGridLayout *layout = new QGridLayout;
    QLabel* pTopLabel = new QLabel(tr("A sandbox isolates your host system from processes running within the box, "
        "it prevents them from making permanent changes to other programs and data in your computer. "
        "The level of isolation impacts your security as well as the compatibility with applications, "
        "hence there will be a different level of isolation depending on the selected Box Type. "
        "Sandboxie can also protect your personal data from being accessed by processes running under its supervision."));
    pTopLabel->setWordWrap(true);
    layout->addWidget(pTopLabel, row++ , 0, 1, 3);


    layout->addWidget(new QLabel(tr("Enter box name:")), row++, 0);

    m_pBoxName = new QLineEdit();
    m_pBoxName->setMaxLength(32);
	QMap<QString, CSandBoxPtr> Boxes = theAPI->GetAllBoxes();
	for (int i=0;; i++) {
		QString NewName = "New Box";
		if (i > 0) NewName.append(" " + QString::number(i));
		if (Boxes.contains(NewName.toLower().replace(" ", "_")))
			continue;
		m_pBoxName->setText(NewName);
		break;
	}
    m_pBoxName->setFocus();
    layout->addWidget(m_pBoxName, row++, 1, 1, 2);
    registerField("boxName", m_pBoxName);


    layout->addWidget(new QLabel(tr("Select box type:")), row++, 0);

    m_pBoxType = new QComboBox();
	m_pBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eHardenedPlus), tr("Hardened Sandbox with Data Protection"), (int)CSandBoxPlus::eHardenedPlus);
	m_pBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eHardened), tr("Security Hardened Sandbox"), (int)CSandBoxPlus::eHardened);
	m_pBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eDefaultPlus), tr("Sandbox with Data Protection"), (int)CSandBoxPlus::eDefaultPlus);
	m_pBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eDefault), tr("Standard Isolation Sandbox (Default)"), (int)CSandBoxPlus::eDefault);
	//m_pBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eInsecure), tr("INSECURE Configuration (please change)"), (int)CSandBoxPlus::eInsecure);
	m_pBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eAppBoxPlus), tr("Application Compartment with Data Protection"), (int)CSandBoxPlus::eAppBoxPlus);
	m_pBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eAppBox), tr("Application Compartment (NO Isolation)"), (int)CSandBoxPlus::eAppBox);
    connect(m_pBoxType, SIGNAL(currentIndexChanged(int)), this, SLOT(OnBoxTypChanged()));
    layout->addWidget(m_pBoxType, row++, 1, 1, 2);
    registerField("boxType", m_pBoxType);

    m_pInfoLabel = new QLabel();
    m_pInfoLabel->setWordWrap(true);
    //m_pInfoLabel->setOpenExternalLinks(true);
    connect(m_pInfoLabel, SIGNAL(linkActivated(const QString&)), theGUI, SLOT(OpenUrl(const QString&)));
    layout->addWidget(m_pInfoLabel, row++, 0, 1, 3);

    m_pBoxType->setCurrentIndex(3); // default


    QWidget* pSpacer = new QWidget();
	pSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(pSpacer, row++, 1);

    QCheckBox* pTemp = new QCheckBox(tr("Remove after use"));
    pTemp->setToolTip(tr("After the last process in the box terminates, all data in the box will be deleted and the box itself will be removed."));
    layout->addWidget(pTemp, row, 0, 1, 2);
    pTemp->setVisible(bAlowTemp);
    registerField("autoRemove", pTemp);

    m_pAdvanced = new QCheckBox(tr("Configure advanced options"));
    m_pAdvanced->setChecked(theConf->GetBool("Options/AdvancedBoxWizard", false));
    layout->addWidget(m_pAdvanced, row++, 2);
    connect(m_pAdvanced, SIGNAL(toggled(bool)), this, SLOT(OnAdvanced()));

    setLayout(layout);
}

void CBoxTypePage::OnBoxTypChanged()
{
	int BoxType = m_pBoxType->currentData().toInt();

	m_pInfoLabel->setText(theGUI->GetBoxDescription(BoxType));

	if(BoxType != CSandBoxPlus::eDefault)
		theGUI->CheckCertificate(this);
}

void CBoxTypePage::OnAdvanced()
{
    ((CNewBoxWizard*)wizard())->m_bAdvanced = m_pAdvanced->isChecked();
    if (m_bInstant)
    {
        QString BoxName = m_pBoxName->text();
        int BoxType = m_pBoxType->currentIndex();

        wizard()->restart();

        m_pBoxName->setText(BoxName);
        m_pBoxType->setCurrentIndex(BoxType);
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
    return true;
}

bool CBoxTypePage::validatePage()
{
    QString BoxName = field("boxName").toString();
    if (!theGUI->GetBoxView()->TestNameAndWarn(BoxName))
        return false;

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
    return CNewBoxWizard::Page_Advanced;
}
    
void CFilesPage::initializePage()
{
    m_pBoxLocation->clear();
    QString Location = theAPI->GetGlobalSettings()->GetText("FileRootPath", "\\??\\%SystemDrive%\\Sandbox\\%USER%\\%SANDBOX%");
    m_pBoxLocation->addItem(Location/*.replace("%SANDBOX%", field("boxName").toString())*/);
}

bool CFilesPage::validatePage()
{
    QString Location = m_pBoxLocation->currentText();
    if (Location == m_pBoxLocation->itemText(0))
        wizard()->setField("boxLocation", "");
    else {
        if (Location.mid(2).contains(QRegularExpression("[ <>:\"/\\|?*\\[\\]]"))){
            QMessageBox::critical(this, "Sandboxie-Plus", tr("The selected box location is not a valid path."));
            return false;
        }
        QDir Dir(Location);
        if (Dir.exists() && !Dir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty()) {
            if(QMessageBox::warning(this, "Sandboxie-Plus", tr("The selected box location exists and is not empty, it is recommended to pick a new or empty folder. "
                "Are you sure you want to use an existing folder?"), QDialogButtonBox::Yes, QDialogButtonBox::No) != QDialogButtonBox::Yes)
                return false;
        }
        if (!QDir().exists(Location.left(3))) {
            QMessageBox::critical(this, "Sandboxie-Plus", tr("The selected box location is not placed on a currently available drive."));
            return false;
        }
        wizard()->setField("boxLocation", Location);
    }
    return true;
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
    registerField("blockNetwork", pNetAccess);

    m_pShareAccess = new QCheckBox(tr("Allow access to network files and folders"));
    m_pShareAccess->setToolTip(tr("This option is not recommended for Hardened boxes"));
    m_pShareAccess->setChecked(theConf->GetBool("BoxDefaults/ShareAccess", false));
    layout->addWidget(m_pShareAccess, row++, 1, 1, 3);
    registerField("shareAccess", m_pShareAccess);


    QLabel* pAdminLabel = new QLabel(tr("Admin Options"), this);
    pAdminLabel->setFont(fnt);
    layout->addWidget(pAdminLabel, row++, 0);

    QCheckBox* pFakeAdmin = new QCheckBox(tr("Make applications think they are running elevated"));
    pFakeAdmin->setChecked(theConf->GetBool("BoxDefaults/FakeAdmin", false));
    layout->addWidget(pFakeAdmin, row++, 1, 1, 3);
    registerField("fakeAdmin", pFakeAdmin);

    m_pMSIServer = new QCheckBox(tr("Allow MSIServer to run with a sandboxed system token"));
    m_pMSIServer->setToolTip(tr("This option is not recommended for Hardened boxes"));
    m_pMSIServer->setChecked(theConf->GetBool("BoxDefaults/MsiExemptions", false));
    layout->addWidget(m_pMSIServer, row++, 1, 1, 3);
    registerField("msiServer", m_pMSIServer);

    QLabel* pBoxLabel = new QLabel(tr("Box Options"), this);
    pBoxLabel->setFont(fnt);
    layout->addWidget(pBoxLabel, row++, 0);

    QCheckBox* pBoxToken = new QCheckBox(tr("Use a Sandboxie login instead of an anonymous token"));
    pBoxToken->setToolTip(tr("Using a custom Sandboxie Token allows to isolate individual sandboxes from each other better, and it shows in the user column of task managers the name of the box a process belongs to. Some 3rd party security solutions may however have problems with custom tokens."));
    pBoxToken->setChecked(theConf->GetBool("BoxDefaults/BoxToken", false));
    layout->addWidget(pBoxToken, row++, 1, 1, 3);
    registerField("boxToken", pBoxToken);

    QCheckBox* pImageProtection = new QCheckBox(tr("Prevent sandboxes programs installed on host from loading dll's from the sandbox"));
    pImageProtection->setToolTip(tr("This feature may reduce compatibility as it also prevents box located processes from writing to host located ones and even starting them."));
    pImageProtection->setChecked(theConf->GetBool("BoxDefaults/ImagesProtection", false));
    pImageProtection->setEnabled(g_CertInfo.valid);
    layout->addWidget(pImageProtection, row++, 1, 1, 3);
    registerField("imagesProtection", pImageProtection);

    setLayout(layout);


	int size = 16.0;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	size *= (QApplication::desktop()->logicalDpiX() / 96.0); // todo Qt6
#endif
    AddIconToLabel(pNetLabel, CSandMan::GetIcon("Network").pixmap(size,size));
    AddIconToLabel(pAdminLabel, CSandMan::GetIcon("Shield9").pixmap(size,size));
    AddIconToLabel(pBoxLabel, CSandMan::GetIcon("Sandbox").pixmap(size,size));
}

int CAdvancedPage::nextId() const
{
    return CNewBoxWizard::Page_Summary;
}
    
void CAdvancedPage::initializePage()
{
    int BoxType = wizard()->field("boxType").toInt();
    bool bHardened = (BoxType == CSandBoxPlus::eHardenedPlus || BoxType == CSandBoxPlus::eHardened);
    m_pMSIServer->setEnabled(!bHardened);
    m_pShareAccess->setEnabled(!bHardened);
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
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/SideLogo.png"));

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

    m_pSetInstant = new QCheckBox(tr("Don't show the summary page in future (unless advanced options were set)"));
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

        theConf->SetValue("BoxDefaults/FakeAdmin", field("fakeAdmin").toBool());
        theConf->SetValue("BoxDefaults/MsiExemptions", field("msiServer").toBool());

        theConf->SetValue("BoxDefaults/BoxToken", field("boxToken").toBool());
        theConf->SetValue("BoxDefaults/ImagesProtection", field("imagesProtection").toBool());
    }

    theConf->SetValue("Options/InstantBoxWizard", m_pSetInstant->isChecked());
    theConf->SetValue("Options/AdvancedBoxWizard", ((CNewBoxWizard*)wizard())->m_bAdvanced);

    SB_STATUS Status = ((CNewBoxWizard*)wizard())->TryToCreateBox();
    if (Status.IsError()) {
        QMessageBox::critical(this, "Sandboxie-Plus", tr("Failed to create new box: %1").arg(theGUI->FormatError(Status)));
        return false;
    }
    return true;
}
