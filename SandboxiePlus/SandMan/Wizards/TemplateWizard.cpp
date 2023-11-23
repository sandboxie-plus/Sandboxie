#include "stdafx.h"

#include "TemplateWizard.h"
#include "../MiscHelpers/Common/Common.h"
#include "../Windows/SettingsWindow.h"
#include "../SandMan.h"
#include "Helpers/WinAdmin.h"
#include <QButtonGroup>
#include "../QSbieAPI/SbieUtils.h"
#include "../Views/SbieView.h"
#include "../MiscHelpers/Common/CheckableMessageBox.h"


CTemplateWizard::CTemplateWizard(ETemplateType Type, QWidget *parent)
    : QWizard(parent)
{
    setPage(Page_TemplateType, new CTemplateTypePage);

    setPage(Page_BrowserType, new CBrowserTypePage);
    setPage(Page_BrowserPaths, new CBrowserPathsPage);
    setPage(Page_BrowserOptions, new CBrowserOptionsPage);

    setPage(Page_FinishTemplate, new CFinishTemplatePage);

    m_Type = Type;
    m_Hold = false;
    switch (Type) {
    case TmplWebBrowser: setStartId(Page_BrowserType); break;
    }

    setWizardStyle(ModernStyle);
    //setOption(HaveHelpButton, true);
    setPixmap(QWizard::LogoPixmap, QPixmap(":/SandMan.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    connect(this, &QWizard::helpRequested, this, &CTemplateWizard::showHelp);

    setWindowTitle(tr("Compatibility Template Wizard"));
}

void CTemplateWizard::showHelp()
{

}

QString CTemplateWizard::GetTemplateLabel(ETemplateType Type)
{
	switch (Type) {
    case TmplCustom: return tr("Custom");
	case TmplWebBrowser: return tr("Web Browser");
	default: return QString();
	}
}

bool CTemplateWizard::CreateNewTemplate(CSandBox* pBox, ETemplateType Type, QWidget* pParent)
{
	CTemplateWizard wizard(Type, pParent);
    if (!theGUI->SafeExec(&wizard))
        return false;
	
    if (Type == TmplCustom)
        Type = (ETemplateType)wizard.field("tmplType").toInt();
    QString Name = wizard.field("templateName").toString();
    QString Section = QString(Name).replace(" ", "_");

    switch (Type)
    {
        case TmplCustom: 
            // todo:
            break;
	    case TmplWebBrowser: 
        {
            CBrowserTypePage* pPage = (CBrowserTypePage*)wizard.page(CTemplateWizard::Page_BrowserType);
            
            QString BrowserPath = wizard.field("browserPath").toString();
            QString BrowserBinary = Split2(BrowserPath, "\\", true).second;
            QString ProfilePath = wizard.field("profilePath").toString();
            if (wizard.field("noProfile").toBool())
                ProfilePath.clear();

            {
                QSharedPointer<CSbieIni> pTemplate = QSharedPointer<CSbieIni>(new CSbieIni("Template_Local_" + Section + "_Force", theAPI));
	            pTemplate->SetText("Tmpl.Title", tr("Force %1 to run in this sandbox").arg(Name));
	            pTemplate->SetText("Tmpl.Class", "Local");
                pTemplate->AppendText("ForceProcess", BrowserBinary);

                if (wizard.field("forceBrowser").toBool() && pBox)
                        pBox->AppendText("Template", pTemplate->GetName().mid(9));
            }

            if (!ProfilePath.isEmpty())
            {
                QString AppData_Roaming = QProcessEnvironment::systemEnvironment().value("AppData");
                QString AppData_Local = QProcessEnvironment::systemEnvironment().value("LocalAppData");

                ProfilePath.replace(AppData_Roaming, "%AppData%");
                ProfilePath.replace(AppData_Local, "%Local AppData%");

                {
                    QSharedPointer<CSbieIni> pTemplate = QSharedPointer<CSbieIni>(new CSbieIni("Template_Local_" + Section + "_Profile_DirectAccess", theAPI));
                    pTemplate->SetText("Tmpl.Title", tr("Allow direct access to the entire %1 profile folder").arg(Name));
                    pTemplate->SetText("Tmpl.Class", "Local");
                    pTemplate->AppendText("OpenFilePath", BrowserBinary + "," + ProfilePath);

                    if (wizard.field("fullProfile").toBool() && pBox)
                        pBox->AppendText("Template", pTemplate->GetName().mid(9));
                }

                switch (pPage->m_BrowserType)
                {
                case CBrowserTypePage::Browser_Gecko:
                    {
                        QSharedPointer<CSbieIni> pTemplate = QSharedPointer<CSbieIni>(new CSbieIni("Template_Local_" + Section + "_Phishing_DirectAccess", theAPI));
                        pTemplate->SetText("Tmpl.Title", tr("Allow direct access to %1 phishing database").arg(Name));
                        pTemplate->SetText("Tmpl.Class", "Local");
                        pTemplate->AppendText("OpenFilePath", BrowserBinary + "," + ProfilePath + "*\\cert9.db");

                        QString ProfilePath2 = ProfilePath;
                        ProfilePath2.replace("%AppData%", "%Local AppData%");

                        pTemplate->AppendText("OpenFilePath", BrowserBinary + "," + ProfilePath2 + "*\\safebrowsing*");

                        if (wizard.field("phishingDb").toBool() && pBox)
                            pBox->AppendText("Template", pTemplate->GetName().mid(9));
                    }
                    {
                        QSharedPointer<CSbieIni> pTemplate = QSharedPointer<CSbieIni>(new CSbieIni("Template_Local_" + Section + "_Session_DirectAccess", theAPI));
                        pTemplate->SetText("Tmpl.Title", tr("Allow direct access to %1 session management").arg(Name));
                        pTemplate->SetText("Tmpl.Class", "Local");
                        pTemplate->AppendText("OpenFilePath", BrowserBinary + "," + ProfilePath + "*\\sessionstore.js*");

                        if (wizard.field("sessions").toBool() && pBox)
                            pBox->AppendText("Template", pTemplate->GetName().mid(9));
                    }
                    {
                        QSharedPointer<CSbieIni> pTemplate = QSharedPointer<CSbieIni>(new CSbieIni("Template_Local_" + Section + "_Passwords_DirectAccess", theAPI));
                        pTemplate->SetText("Tmpl.Title", tr("Allow direct access to %1 passwords").arg(Name));
                        pTemplate->SetText("Tmpl.Class", "Local");
                        pTemplate->AppendText("OpenFilePath", BrowserBinary + "," + ProfilePath + "*\\logins.json");
                        pTemplate->AppendText("OpenFilePath", BrowserBinary + "," + ProfilePath + "*\\key*.db");

                        if (wizard.field("passwords").toBool() && pBox)
                            pBox->AppendText("Template", pTemplate->GetName().mid(9));
                    }
                    {
                        QSharedPointer<CSbieIni> pTemplate = QSharedPointer<CSbieIni>(new CSbieIni("Template_Local_" + Section + "_Cookies_DirectAccess", theAPI));
                        pTemplate->SetText("Tmpl.Title", tr("Allow direct access to %1 cookies").arg(Name));
                        pTemplate->SetText("Tmpl.Class", "Local");
                        pTemplate->AppendText("OpenFilePath", BrowserBinary + "," + ProfilePath + "*\\cookies*");

                        if (wizard.field("cookies").toBool() && pBox)
                            pBox->AppendText("Template", pTemplate->GetName().mid(9));
                    }
                    {
                        QSharedPointer<CSbieIni> pTemplate = QSharedPointer<CSbieIni>(new CSbieIni("Template_Local_" + Section + "_Bookmarks_DirectAccess", theAPI));
                        pTemplate->SetText("Tmpl.Title", tr("Allow direct access to %1 bookmark and history database").arg(Name));
                        pTemplate->SetText("Tmpl.Class", "Local");
                        pTemplate->AppendText("OpenFilePath", BrowserBinary + "," + ProfilePath + "*\\bookmark*");
                        pTemplate->AppendText("OpenFilePath", BrowserBinary + "," + ProfilePath + "*\\places*");
                        pTemplate->AppendText("OpenFilePath", BrowserBinary + "," + ProfilePath + "*\\favicons.sqlite");

                        if (wizard.field("history").toBool() && pBox)
                            pBox->AppendText("Template", pTemplate->GetName().mid(9));
                    }
                    break;
                case CBrowserTypePage::Browser_Chromium:
                    {
                        QSharedPointer<CSbieIni> pTemplate = QSharedPointer<CSbieIni>(new CSbieIni("Template_Local_" + Section + "_Phishing_DirectAccess", theAPI));
                        pTemplate->SetText("Tmpl.Title", tr("Allow direct access to %1 phishing database").arg(Name));
                        pTemplate->SetText("Tmpl.Class", "Local");
                        pTemplate->AppendText("OpenFilePath", BrowserBinary + "," + ProfilePath + "\\Safe Browsing*");
                        pTemplate->AppendText("OpenFilePath", BrowserBinary + "," + ProfilePath + "\\CertificateRevocation");
                        pTemplate->AppendText("OpenFilePath", BrowserBinary + "," + ProfilePath + "\\SmartScreen"); // Microsoft Edge
                        pTemplate->AppendText("OpenFilePath", BrowserBinary + "," + ProfilePath + "\\Ad Blocking"); // Microsoft Edge

                        if (wizard.field("phishingDb").toBool() && pBox)
                            pBox->AppendText("Template", pTemplate->GetName().mid(9));
                    }
                    {
                        QSharedPointer<CSbieIni> pTemplate = QSharedPointer<CSbieIni>(new CSbieIni("Template_Local_" + Section + "_Sync_DirectAccess", theAPI));
                        pTemplate->SetText("Tmpl.Title", tr("Allow direct access to %1 sync data").arg(Name));
                        pTemplate->SetText("Tmpl.Class", "Local");
                        pTemplate->AppendText("OpenFilePath", BrowserBinary + "," + ProfilePath + "*\\Sync Data\\*");
                        pTemplate->AppendText("OpenFilePath", BrowserBinary + "," + ProfilePath + "*\\Sync Extension Settings\\*");

                        if (wizard.field("syncData").toBool() && pBox)
                            pBox->AppendText("Template", pTemplate->GetName().mid(9));
                    }
                    {
                        QSharedPointer<CSbieIni> pTemplate = QSharedPointer<CSbieIni>(new CSbieIni("Template_Local_" + Section + "_Preferences_DirectAccess", theAPI));
                        pTemplate->SetText("Tmpl.Title", tr("Allow direct access to %1 preferences").arg(Name));
                        pTemplate->SetText("Tmpl.Class", "Local");
                        pTemplate->AppendText("OpenFilePath", BrowserBinary + "," + ProfilePath + "*\\Preferences*");

                        if (wizard.field("preferences").toBool() && pBox)
                            pBox->AppendText("Template", pTemplate->GetName().mid(9));
                    }
                    {
                        QSharedPointer<CSbieIni> pTemplate = QSharedPointer<CSbieIni>(new CSbieIni("Template_Local_" + Section + "_Passwords_DirectAccess", theAPI));
                        pTemplate->SetText("Tmpl.Title", tr("Allow direct access to %1 passwords").arg(Name));
                        pTemplate->SetText("Tmpl.Class", "Local");
                        pTemplate->AppendText("OpenFilePath", BrowserBinary + "," + ProfilePath + "*\\Login Data*");

                        if (wizard.field("passwords").toBool() && pBox)
                            pBox->AppendText("Template", pTemplate->GetName().mid(9));
                    }
                    {
                        QSharedPointer<CSbieIni> pTemplate = QSharedPointer<CSbieIni>(new CSbieIni("Template_Local_" + Section + "_Cookies_DirectAccess", theAPI));
                        pTemplate->SetText("Tmpl.Title", tr("Allow direct access to %1 cookies").arg(Name));
                        pTemplate->SetText("Tmpl.Class", "Local");
                        pTemplate->AppendText("OpenFilePath", BrowserBinary + "," + ProfilePath + "*\\Network\\Cookies*");

                        if (wizard.field("cookies").toBool() && pBox)
                            pBox->AppendText("Template", pTemplate->GetName().mid(9));
                    }
                    {
                        QSharedPointer<CSbieIni> pTemplate = QSharedPointer<CSbieIni>(new CSbieIni("Template_Local_" + Section + "_Bookmarks_DirectAccess", theAPI));
                        pTemplate->SetText("Tmpl.Title", tr("Allow direct access to %1 bookmarks").arg(Name));
                        pTemplate->SetText("Tmpl.Class", "Local");
                        pTemplate->AppendText("OpenFilePath", BrowserBinary + "," + ProfilePath + "*\\Bookmarks*");
                        pTemplate->AppendText("OpenFilePath", BrowserBinary + "," + ProfilePath + "*\\Favicons*");

                        if (wizard.field("bookmarks").toBool() && pBox)
                            pBox->AppendText("Template", pTemplate->GetName().mid(9));
                    }
                    {
                        QSharedPointer<CSbieIni> pTemplate = QSharedPointer<CSbieIni>(new CSbieIni("Template_Local_" + Section + "_History_DirectAccess", theAPI));
                        pTemplate->SetText("Tmpl.Title", tr("Allow direct access to %1 bookmark and history database").arg(Name));
                        pTemplate->SetText("Tmpl.Class", "Local");
                        pTemplate->AppendText("OpenFilePath", BrowserBinary + "," + ProfilePath + "*\\Bookmarks*");
                        pTemplate->AppendText("OpenFilePath", BrowserBinary + "," + ProfilePath + "*\\Favicons*");
                        pTemplate->AppendText("OpenFilePath", BrowserBinary + "," + ProfilePath + "*\\*History*");
                        pTemplate->AppendText("OpenFilePath", BrowserBinary + "," + ProfilePath + "*\\Current *");
                        pTemplate->AppendText("OpenFilePath", BrowserBinary + "," + ProfilePath + "*\\Last *");
                        pTemplate->AppendText("OpenFilePath", BrowserBinary + "," + ProfilePath + "*\\Visited Links*");

                        if (wizard.field("history").toBool() && pBox)
                            pBox->AppendText("Template", pTemplate->GetName().mid(9));
                    }
                    break;
                }
            }


            break;
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// CTemplateTypePage
// 

CTemplateTypePage::CTemplateTypePage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Create new Template"));
    QPixmap Logo = QPixmap(theGUI->m_DarkTheme ? ":/SideLogoDM.png" : ":/SideLogo.png");
    int Scaling = theConf->GetInt("Options/FontScaling", 100);
    if(Scaling !=  100) Logo = Logo.scaled(Logo.width() * Scaling / 100, Logo.height() * Scaling / 100);
    setPixmap(QWizard::WatermarkPixmap, Logo);

    int row = 0;
    QGridLayout *layout = new QGridLayout;
    //QLabel* pTopLabel = new QLabel(tr(""));
    //pTopLabel->setWordWrap(true);
    //layout->addWidget(pTopLabel, row++ , 0, 1, 3);

    layout->addWidget(new QLabel(tr("Select template type:")), row++, 0);

    m_pTemplateType = new QComboBox();
    for (int i = 0; i < CTemplateWizard::TmplMax; i++)
        m_pTemplateType->addItem(tr("%1 template").arg(CTemplateWizard::GetTemplateLabel((CTemplateWizard::ETemplateType)i)));

    connect(m_pTemplateType, SIGNAL(currentIndexChanged(int)), this, SLOT(OnTypChanged()));
    layout->addWidget(m_pTemplateType, row++, 1, 1, 2);
    registerField("tmplType", m_pTemplateType);

    QLabel* pInfoLabel = new QLabel();
    pInfoLabel->setWordWrap(true);
    layout->addWidget(pInfoLabel, row++, 0, 1, 3);

    setLayout(layout);
}

void CTemplateTypePage::OnTypChanged()
{
    CTemplateWizard::ETemplateType Type = (CTemplateWizard::ETemplateType)field("tmplType").toInt();
    if (!((CTemplateWizard*)wizard())->m_Hold && ((CTemplateWizard*)wizard())->m_Type != Type) {
        ((CTemplateWizard*)wizard())->m_Type = Type;
        ((CTemplateWizard*)wizard())->m_Hold = true;
        wizard()->restart();
        ((CTemplateWizard*)wizard())->m_Hold = false;
    }
}

void CTemplateTypePage::initializePage()
{
    m_pTemplateType->setCurrentIndex(((CTemplateWizard*)wizard())->m_Type);
}

int CTemplateTypePage::nextId() const
{
    switch (field("tmplType").toInt()) {
    case CTemplateWizard::TmplWebBrowser: return CTemplateWizard::Page_BrowserPaths;
    default: return -1;
    }
}

bool CTemplateTypePage::isComplete() const
{
    return true;
}

bool CTemplateTypePage::validatePage()
{
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// CBrowserTypePage
// 

CBrowserTypePage::CBrowserTypePage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Create Web Browser Template"));
    setSubTitle(tr("Select your Web Browsers main executable, this will allow Sandboxie to identify the browser."));

    m_BrowserType = Browser_Other;

    int row = 0;
    QGridLayout *layout = new QGridLayout;
    //QLabel* pTopLabel = new QLabel(tr(""));
    //pTopLabel->setWordWrap(true);
    //layout->addWidget(pTopLabel, row++, 0, 1, 3);

    layout->addWidget(new QLabel(tr("Enter browser name:")), row++, 0);
    
    m_pName = new QLineEdit();
    m_pName->setMaxLength(32); // BOXNAME_COUNT
    m_pName->setMaximumWidth(150);
    layout->addWidget(m_pName, row++, 0, 1, 1);
    connect(m_pName, SIGNAL(textChanged(const QString&)), this, SLOT(OnNameChanged()));
    registerField("templateName", m_pName);

    m_NameState = 0;

    layout->addWidget(new QLabel(tr("Main executable (eg. firefox.exe, chrome.exe, msedge.exe, etc...):")), row++, 0);

 	CPathEdit* pImagePath = new CPathEdit(false, true);
	pImagePath->SetWindowsPaths();
    pImagePath->SetFilter(tr("Browser executable (*.exe)"));
	//pImagePath->SetDefault();
	//pImagePath->SetText();

    QStringList KnownPaths;

    // Firefox based
    KnownPaths.append("C:\\Program Files\\Mozilla Firefox\\firefox.exe");
    KnownPaths.append("C:\\Program Files\\Firefox Developer Edition\\firefox.exe");
    KnownPaths.append("C:\\Program Files\\Waterfox\\waterfox.exe");
    KnownPaths.append("C:\\Program Files\\LibreWolf\\librewolf.exe");
    KnownPaths.append("C:\\Program Files\\Pale Moon\\palemoon.exe");
    KnownPaths.append("C:\\Program Files\\SeaMonkey\\seamonkey.exe");

    // CHromium based
    QString AppData_Local = QProcessEnvironment::systemEnvironment().value("LocalAppData");
    KnownPaths.append("C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe");
    KnownPaths.append("C:\\Program Files\\Google\\Chrome Dev\\Application\\chrome.exe");
    KnownPaths.append("C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe");
    KnownPaths.append("C:\\Program Files (x86)\\Microsoft\\Edge Dev\\Application\\msedge.exe");
    KnownPaths.append("C:\\Program Files\\BraveSoftware\\Brave-Browser\\Application\\brave.exe");
    KnownPaths.append("C:\\Program Files\\SRWare Iron (64-Bit)\\iron.exe");
    KnownPaths.append("C:\\Program Files\\Comodo\\Dragon\\dragon.exe");
    KnownPaths.append(AppData_Local + "\\Chromium\\Application\\chrome.exe");
    KnownPaths.append(AppData_Local + "\\Programs\\Opera\\launcher.exe");
    KnownPaths.append(AppData_Local + "\\Vivaldi\\Application\\vivaldi.exe");
    KnownPaths.append(AppData_Local + "\\Opera Software\\Opera Neon\\Application\\neon.exe");
    KnownPaths.append(AppData_Local + "\\Maxthon\\Application\\Maxthon.exe");

    foreach(const QString& KnownPath, KnownPaths) {
        if(QFile::exists(KnownPath))
            pImagePath->GetCombo()->addItem(KnownPath);
    }
    pImagePath->GetCombo()->setCurrentText("");
    connect(pImagePath, SIGNAL(textChanged(const QString&)), this, SLOT(OnPathChanged()));

    layout->addWidget(pImagePath, row++, 0, 1, 3);
    m_pBrowserPath = pImagePath->GetEdit();
    registerField("browserPath", m_pBrowserPath);

    m_pInfoLabel = new QLabel();
    m_pInfoLabel->setWordWrap(true);
    layout->addWidget(m_pInfoLabel, row++, 0, 1, 3);

    setLayout(layout);
}

int CBrowserTypePage::TestFiles(const QString& ImageDir, const QList<STestFile>& Files)
{
    int TotalConfidence = 0;
    foreach(const STestFile & File, Files) {
        if (QFile::exists(ImageDir + "\\" + File.Name)) {
            TotalConfidence += File.Confidence;
        }
    }
    return TotalConfidence;
}

CBrowserTypePage::EBrowserType CBrowserTypePage::DetectBrowserType(const QString& ImagePath, int* pConfidence)
{
    int Confidence = 0;
    StrPair PathName = Split2(ImagePath, "\\", true);
    StrPair NameExt = Split2(PathName.second, ".", true);

    // check for Gecko based
    QList<STestFile> GeckoFiles = QList<STestFile>() 
        << STestFile{ "xul.dll", 50 } 
        << STestFile{ "mozglue.dll", 50 };
    Confidence = TestFiles(PathName.first, GeckoFiles);
    if (Confidence > 0) {
        if (pConfidence) *pConfidence = Confidence;
        return Browser_Gecko;
    }

    // check for Chromium based
    QList<STestFile> ChromiumFiles = QList<STestFile>() 
        << STestFile{ "chrome.dll", 40 } << STestFile{ NameExt.first + ".dll", 40 } // some releases rename this dll
        << STestFile{ "chrome_elf.dll", 40 } << STestFile{ NameExt.first + "_elf.dll", 40 } // some releases rename and that dll
        << STestFile{ "v8_context_snapshot.bin", 20 };
    QDir Dir(PathName.first);
    auto Dirs = Dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (int i = -1; i < Dirs.size(); i++) {
        Confidence = TestFiles(PathName.first + (i == -1 ? "" : ("\\" + Dirs[i].fileName())), ChromiumFiles);
        if (Confidence > 0) {
            if (pConfidence) *pConfidence = Confidence;
            return Browser_Chromium;
        }
    }

    return Browser_Other;
}

void CBrowserTypePage::OnPathChanged()
{
    QString Path = m_pBrowserPath->text();

    m_BrowserType = DetectBrowserType(Path);
    switch (m_BrowserType)
    {
    case Browser_Gecko:  
        m_pInfoLabel->setText(tr("The browser appears to be Gecko based, like Mozilla Firefox and its derivatives.")); 
        break;
    case Browser_Chromium: 
        m_pInfoLabel->setText(tr("The browser appears to be Chromium based, like Microsoft Edge or Google Chrome and its derivatives.")); 
        break;
    default:
        m_pInfoLabel->setText(tr("Browser could not be recognized, template cannot be created.")); 
    }

    if (m_pName->text().isEmpty() || m_NameState == 0) 
    {
        StrPair PathName = Split2(Path, "\\", true);
        StrPair NameExt = Split2(PathName.second, ".", true);
        StrPair PathParent = Split2(PathName.first, "\\", true);
        if (PathParent.second == "Application") 
            PathParent = Split2(PathParent.first, "\\", true);

        m_NameState = 2;
        m_pName->setText(PathParent.second);
        m_NameState = 0;
    }

    emit completeChanged();
}

void CBrowserTypePage::OnNameChanged()
{
    if (m_NameState == 2)
        return;
    m_NameState = 1;
    emit completeChanged();
}

void CBrowserTypePage::initializePage()
{
    //QTimer::singleShot(10, m_pBrowserPath, SLOT(setFocus()));
}

int CBrowserTypePage::nextId() const
{
    return CTemplateWizard::Page_BrowserPaths;
}

bool CBrowserTypePage::isComplete() const
{
    return m_BrowserType != Browser_Other && !field("templateName").toString().isEmpty();
}

bool CBrowserTypePage::validatePage()
{
    QString Section = field("templateName").toString().replace(" ", "_");
    QSharedPointer<CSbieIni> pTemplate = QSharedPointer<CSbieIni>(new CSbieIni("Template_Local_" + Section + "_Force", theAPI));
    if (!pTemplate->GetText("ForceProcess").isEmpty()) {
        QMessageBox::critical(this, "Sandboxie-Plus", tr("This browser name is already in use, please choose an other one."));
        return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// CBrowserPathsPage
// 

CBrowserPathsPage::CBrowserPathsPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Create Web Browser Template"));
    setSubTitle(tr("Configure your Web Browser's profile directories."));

    int row = 0;
    QGridLayout *layout = new QGridLayout;
    //QLabel* pTopLabel = new QLabel(tr(""));
    //pTopLabel->setWordWrap(true);
    //layout->addWidget(pTopLabel, row++, 0, 1, 3);

    layout->addWidget(new QLabel(tr("User profile(s) directory:")), row, 0);

    m_pProfileFilter = new QCheckBox(tr("Show also imperfect matches"));
    layout->addWidget(m_pProfileFilter, row++, 2);
    connect(m_pProfileFilter, SIGNAL(toggled(bool)), this, SLOT(OnProfilesChange()));

 	CPathEdit* pImagePath = new CPathEdit(true, true);
	pImagePath->SetWindowsPaths();
    pImagePath->SetFilter(tr("Browser Executable (*.exe)"));
	//pImagePath->SetDefault();
	//pImagePath->SetText();
    connect(pImagePath, SIGNAL(textChanged(const QString&)), this, SLOT(OnProfileChange()));

    layout->addWidget(pImagePath, row++, 0, 1, 3);
    m_pProfilePath = pImagePath->GetCombo();
    registerField("profilePath", m_pProfilePath->lineEdit());

    m_pInfoLabel = new QLabel();
    m_pInfoLabel->setWordWrap(true);
    layout->addWidget(m_pInfoLabel, row++, 0, 1, 3);

    QWidget* pSpacer = new QWidget();
	pSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(pSpacer, row++, 1);

    m_pNoProfile = new QCheckBox(tr("Continue without browser profile"));
    layout->addWidget(m_pNoProfile, row++, 2);
    connect(m_pNoProfile, SIGNAL(toggled(bool)), this, SLOT(OnNoProfile()));
    registerField("noProfile", m_pNoProfile);
    
    setLayout(layout);
}

void CBrowserPathsPage::OnNoProfile()
{
    m_pProfilePath->setEnabled(!m_pNoProfile->isChecked());
    m_pInfoLabel->setEnabled(!m_pNoProfile->isChecked());

    emit completeChanged();
}

bool CBrowserPathsPage::IsFirefoxProfile(const QString& Path)
{
    int FoundCount = 0;
    QStringList KnownFiles = QStringList() << "prefs.js" << "extensions.json" << "cookies.sqlite" << "storage.sqlite" << "protections.sqlite" 
        << "xulstore.json" << "places.sqlite" << "favicons.sqlite" << "permissions.sqlite" << "formhistory.sqlite" << "credentialstate.sqlite" << "extension-preferences.json";
    foreach(const QString & KnownFile, KnownFiles) {
        if (QFile::exists(Path + "\\" + KnownFile))
            FoundCount++;
    }
    return FoundCount >= KnownFiles.size() / 2;
}

QString CBrowserPathsPage::GetFirefoxProfiles(const QString& Path)
{
    if (QFile::exists(Path + "\\Profiles"))
        return Path + "\\Profiles";
    return QString();
}

bool CBrowserPathsPage::IsChromiumProfile(const QString& Path)
{
    int FoundCount = 0;
    QStringList KnownFiles = QStringList() << "Bookmarks" << "Favicons" << "History" << "Preferences" << "Shortcuts" << "Web Data"
        << "Code Cache" << "Extension Scripts" << "Extension State" << "Local Storage" << "Network" << "Session Storage";
    foreach(const QString & KnownFile, KnownFiles) {
        if (QFile::exists(Path + "\\" + KnownFile))
            FoundCount++;
    }
    return FoundCount >= KnownFiles.size() / 2;
}

QString CBrowserPathsPage::GetChromiumProfiles(const QString& Path)
{
    if (IsChromiumProfile(Path))
        return Path;
    if (QFile::exists(Path + "\\User Data"))
        return Path + "\\User Data";
    return QString();
}

int CBrowserPathsPage::SmartMatch(const QString& DirName, const QString& Name) 
{
    if (DirName.compare(Name, Qt::CaseInsensitive) == 0)
        return 100;
    if(DirName.simplified().remove(" ").compare(Name, Qt::CaseInsensitive) == 0)
        return 90;

    QStringList DirNameParts = DirName.split(QRegularExpression("[^A-Za-z0-9]"), Qt::SkipEmptyParts);
    foreach(const QString & DirNamePart, DirNameParts) {
        if (Name.contains(DirNamePart, Qt::CaseInsensitive))
            return 50;
    }

    QStringList NameParts = Name.split(QRegularExpression("[^A-Za-z0-9]"), Qt::SkipEmptyParts);
    foreach(const QString & NamePart, NameParts) {
        if (DirName.contains(NamePart, Qt::CaseInsensitive))
            return 50;
    }

    return 0;
}

QList<CBrowserPathsPage::SFoundFolder> CBrowserPathsPage::FindFolders(const QString& Root, const QString& Name, QString(*GetProfilePath)(const QString&), int Depth)
{
    QList<SFoundFolder> Folders;
    QDir Dir(Root);
    foreach(const QFileInfo& Info, Dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString DirName = Info.fileName();
        if (DirName.compare("Temp", Qt::CaseInsensitive) == 0)
            continue;
        if (int Match = SmartMatch(DirName, Name)) {
            QString ProfilePath = GetProfilePath(Root + "\\" + DirName);
            if (!ProfilePath.isEmpty())
                Folders.append(SFoundFolder{ ProfilePath, Match });
        }
        if (Depth > 0)
            Folders.append(FindFolders(Root + "\\" + DirName, Name, GetProfilePath, Depth - 1));
    }
    return Folders;
}

QList<CBrowserPathsPage::SFoundFolder> CBrowserPathsPage::FindFirefoxFolders(const QString& ImagePath)
{
    StrPair PathName = Split2(ImagePath, "\\", true);
    StrPair NameExt = Split2(PathName.second, ".", true);

    QString AppData_Roaming = QProcessEnvironment::systemEnvironment().value("AppData");

    return FindFolders(AppData_Roaming, NameExt.first, GetFirefoxProfiles);
}

QList<CBrowserPathsPage::SFoundFolder> CBrowserPathsPage::FindChromiumFolders(const QString& ImagePath)
{
    QList<CBrowserPathsPage::SFoundFolder> Folders;

    StrPair PathName = Split2(ImagePath, "\\", true);
    StrPair NameExt = Split2(PathName.second, ".", true);
    StrPair PathParent = Split2(PathName.first, "\\", true);

    QString AppData_Local = QProcessEnvironment::systemEnvironment().value("LocalAppData");

    // same directory, best match
    QString GetProfilePath = GetChromiumProfiles(PathParent.first);
    if (!GetProfilePath.isEmpty()) // "User Data" in the same parent as "Application"
        Folders.append(SFoundFolder{ PathParent.first + "\\User Data", 100 });
    GetProfilePath = GetChromiumProfiles(PathName.first);
    if (!GetProfilePath.isEmpty()) // "User Data" under "Application" like Maxthon
        Folders.append(SFoundFolder{ PathName.first + "\\User Data", 100 });

    if (Folders.isEmpty())
    {
        // find directories by image parent name, higher priority
        if (PathParent.second == "Application") 
        {
            PathParent = Split2(PathParent.first, "\\", true);

            Folders.append(FindFolders(AppData_Local, PathParent.second, GetChromiumProfiles));
        }
        // find directories by image name
        else 
        {
            //if (NameExt.first.compare("Chrome.exe", Qt::CaseInsensitive) == 0 && !PathParent.first.contains("\\Google\\")) {
            //}

            Folders.append(FindFolders(AppData_Local, NameExt.first, GetChromiumProfiles));

            if (Folders.isEmpty()) // like opera 
            {
                QString AppData_Roaming = QProcessEnvironment::systemEnvironment().value("AppData");

                Folders.append(FindFolders(AppData_Roaming, PathParent.second, GetChromiumProfiles));
            }
        }
    }

    return Folders;
}

void CBrowserPathsPage::initializePage()
{
    CBrowserTypePage* pPage = (CBrowserTypePage*)wizard()->page(CTemplateWizard::Page_BrowserType);

    switch (pPage->m_BrowserType)
    {
    case CBrowserTypePage::Browser_Gecko:
        setSubTitle(tr("Configure your Gecko based Browsers profile directories."));
        m_FoundFolders = FindFirefoxFolders(field("browserPath").toString());
        break;
    case CBrowserTypePage::Browser_Chromium:
        setSubTitle(tr("Configure your Chromium based Browsers profile directories."));
        m_FoundFolders = FindChromiumFolders(field("browserPath").toString());
        break;
    }

    OnProfilesChange();
}

void CBrowserPathsPage::OnProfilesChange()
{
    QList<SFoundFolder> Folders = m_FoundFolders;
    int MaxConfidence = 0;
    int MinConfidence = 0;
    if (!Folders.isEmpty()) 
    {
        auto min_max = std::minmax_element(Folders.begin(), Folders.end(), [](const SFoundFolder& s1, const SFoundFolder& s2) { return s1.Confidence < s2.Confidence; });
        MinConfidence = min_max.first->Confidence;
        MaxConfidence = min_max.second->Confidence;

        if (!m_pProfileFilter->isChecked()) {
            // filter the results with the highest confidence level only
            QList<SFoundFolder> BestFolders;
            std::copy_if(Folders.begin(), Folders.end(), std::inserter(BestFolders, BestFolders.end()), [MaxConfidence](const SFoundFolder& s) {return s.Confidence == MaxConfidence; });
            Folders = BestFolders;
        }
        else {
            // sort the items with the highest confidence first
            std::sort(Folders.begin(), Folders.end(), [](const SFoundFolder& s1, const SFoundFolder& s2) { return s1.Confidence > s2.Confidence; });
        }
    }

    m_pProfileFilter->setVisible(MinConfidence < 100 && MaxConfidence > 0);
    if (MaxConfidence != 100)
        m_pProfileFilter->setChecked(true);
    m_pProfileFilter->setEnabled(MaxConfidence >= 100);

    m_pProfilePath->clear();
    foreach(const SFoundFolder & Folder, Folders)
        m_pProfilePath->addItem(Folder.Path);
    if (m_pProfilePath->count() > 1) // require the user to make a choice
        m_pProfilePath->setCurrentText("");
    
    OnProfileChange();
}

bool CBrowserPathsPage::IsBrowserProfile(const QString& Path) const
{
    CBrowserTypePage* pPage = (CBrowserTypePage*)wizard()->page(CTemplateWizard::Page_BrowserType);

    switch (pPage->m_BrowserType)
    {
    case CBrowserTypePage::Browser_Gecko:
        return IsFirefoxProfile(Path);
    case CBrowserTypePage::Browser_Chromium:
        return IsChromiumProfile(Path);
    default:
        return false;
    }
}

bool CBrowserPathsPage::TestProfilePath(const QString& Path) const
{
    if (IsBrowserProfile(Path))
        return true;

    QDir Dir(Path);
    foreach(const QFileInfo& Info, Dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        if (IsBrowserProfile(Path + "\\" + Info.fileName()))
            return true;
    }

    return false;
}

void CBrowserPathsPage::OnProfileChange()
{
    QString CurPath = m_pProfilePath->currentText();
    if (CurPath.isEmpty())
    {
        if(m_FoundFolders.isEmpty())
            m_pInfoLabel->setText(tr(
                "No suitable folders have been found.\n"
                "Note: you need to run the browser unsandboxed for them to get created.\n"
                "Please browse to the correct user profile directory."));
        else
            m_pInfoLabel->setText(tr(
                "Please choose the correct user profile directory, if it is not listed you may need to browse to it."));
    }
    else if (TestProfilePath(CurPath))
    {
        if (m_pProfileFilter->isVisible() && m_pProfileFilter->isCheckable())
            m_pInfoLabel->setText(tr(
                "Please ensure the selected directory is correct, the wizard is not confident in all the presented options."));
        else
            m_pInfoLabel->setText(tr(
                "Please ensure the selected directory is correct."));
    }
    else
    {
        m_pInfoLabel->setText(tr(
            "This path does not look like a valid profile directory."));
    }

    emit completeChanged();
}

int CBrowserPathsPage::nextId() const
{
    return CTemplateWizard::Page_BrowserOptions;
}

bool CBrowserPathsPage::isComplete() const
{
    return TestProfilePath(m_pProfilePath->currentText()) || field("noProfile").toBool();
}

bool CBrowserPathsPage::validatePage()
{
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// CBrowserOptionsPage
// 

CBrowserOptionsPage::CBrowserOptionsPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Create Web Browser Template"));
    setSubTitle(tr("Configure web browser template options."));

    int row = 0;
    QGridLayout *layout = new QGridLayout;
    //QLabel* pTopLabel = new QLabel(tr(""));
    //pTopLabel->setWordWrap(true);
    //layout->addWidget(pTopLabel, row++, 0, 1, 3);

    layout->setSpacing(3);

    m_pForce = new QCheckBox(tr("Force the Web Browser to run in this sandbox"));
    layout->addWidget(m_pForce, row++, 0, 1, 3);
    registerField("forceBrowser", m_pForce);

    m_pProfile = new QCheckBox(tr("Allow direct access to the entire Web Browser profile folder"));
    layout->addWidget(m_pProfile, row++, 0, 1, 3);
    registerField("fullProfile", m_pProfile);

    m_pPhishingDb = new QCheckBox(tr("Allow direct access to Web Browser's phishing database"));
    layout->addWidget(m_pPhishingDb, row++, 0, 1, 3);
    registerField("phishingDb", m_pPhishingDb);

    m_pSessions = new QCheckBox(tr("Allow direct access to Web Browser's session management"));
    layout->addWidget(m_pSessions, row++, 0, 1, 3);
    registerField("sessions", m_pSessions);

    m_pSyncData = new QCheckBox(tr("Allow direct access to Web Browser's sync data"));
    layout->addWidget(m_pSyncData, row++, 0, 1, 3);
    registerField("syncData", m_pSyncData);

    m_pPreferences = new QCheckBox(tr("Allow direct access to Web Browser's preferences"));
    layout->addWidget(m_pPreferences, row++, 0, 1, 3);
    registerField("preferences", m_pPreferences);

    m_pPasswords = new QCheckBox(tr("Allow direct access to Web Browser's passwords"));
    layout->addWidget(m_pPasswords, row++, 0, 1, 3);
    registerField("passwords", m_pPasswords);

    m_pCookies = new QCheckBox(tr("Allow direct access to Web Browser's cookies"));
    layout->addWidget(m_pCookies, row++, 0, 1, 3);
    registerField("cookies", m_pCookies);

    m_pBookmarks = new QCheckBox(tr("Allow direct access to Web Browser's bookmarks"));
    layout->addWidget(m_pBookmarks, row++, 0, 1, 3);
    registerField("bookmarks", m_pBookmarks);

    m_pHistory = new QCheckBox(tr("Allow direct access to Web Browser's bookmark and history database"));
    layout->addWidget(m_pHistory, row++, 0, 1, 3);
    registerField("history", m_pHistory);

    setLayout(layout);
}

int CBrowserOptionsPage::nextId() const
{
    return CTemplateWizard::Page_FinishTemplate;
}
    
void CBrowserOptionsPage::initializePage()
{
    CBrowserTypePage* pPage = (CBrowserTypePage*)wizard()->page(CTemplateWizard::Page_BrowserType);

    m_pSessions->setVisible(pPage->m_BrowserType == CBrowserTypePage::Browser_Gecko);

    m_pSyncData->setVisible(pPage->m_BrowserType == CBrowserTypePage::Browser_Chromium);
    m_pPreferences->setVisible(pPage->m_BrowserType == CBrowserTypePage::Browser_Chromium);
    m_pBookmarks->setVisible(pPage->m_BrowserType == CBrowserTypePage::Browser_Chromium);

    m_pProfile->setEnabled(!field("noProfile").toBool());
    m_pPhishingDb->setEnabled(!field("noProfile").toBool());
    m_pSessions->setEnabled(!field("noProfile").toBool());
    m_pSyncData->setEnabled(!field("noProfile").toBool());
    m_pPreferences->setEnabled(!field("noProfile").toBool());
    m_pPasswords->setEnabled(!field("noProfile").toBool());
    m_pCookies->setEnabled(!field("noProfile").toBool());
    m_pBookmarks->setEnabled(!field("noProfile").toBool());
    m_pHistory->setEnabled(!field("noProfile").toBool());
}

bool CBrowserOptionsPage::validatePage()
{
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// CFinishTemplatePage
// 

CFinishTemplatePage::CFinishTemplatePage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Create Web Browser Template"));
    QPixmap Logo = QPixmap(theGUI->m_DarkTheme ? ":/SideLogoDM.png" : ":/SideLogo.png");
    int Scaling = theConf->GetInt("Options/FontScaling", 100);
    if(Scaling !=  100) Logo = Logo.scaled(Logo.width() * Scaling / 100, Logo.height() * Scaling / 100);
    setPixmap(QWizard::WatermarkPixmap, Logo);

    int row = 0;
    QGridLayout *layout = new QGridLayout;

    QLabel* pLabel = new QLabel;
    pLabel->setWordWrap(true);
    pLabel->setText(tr("Almost complete, click Finish to create a new  Web Browser Template and conclude the wizard."));
    layout->addWidget(pLabel, row++ , 0, 1, 3);

    m_pSummary = new QTextEdit();
    m_pSummary->setReadOnly(true);
    m_pSummary->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(m_pSummary, row++ , 0, 1, 3);
    
    setLayout(layout);
}

int CFinishTemplatePage::nextId() const
{
    return -1;
}

void CFinishTemplatePage::initializePage()
{
    QString Info = tr("Browser name: %1\n").arg(field("templateName").toString());

    CBrowserTypePage* pPage = (CBrowserTypePage*)wizard()->page(CTemplateWizard::Page_BrowserType);
    switch (pPage->m_BrowserType) {
    case CBrowserTypePage::Browser_Gecko: Info += tr("Browser Type: Gecko (Mozilla Firefox)\n"); break;
    case CBrowserTypePage::Browser_Chromium: Info += tr("Browser Type: Chromium (Google Chrome)\n"); break;
    }
    Info += tr("\n");
    Info += tr("Browser executable path: %1\n").arg(field("browserPath").toString());

    QString ProfilePath = field("profilePath").toString();

    QString AppData_Roaming = QProcessEnvironment::systemEnvironment().value("AppData");
    QString AppData_Local = QProcessEnvironment::systemEnvironment().value("LocalAppData");

    ProfilePath.replace(AppData_Roaming, "%AppData%");
    ProfilePath.replace(AppData_Local, "%Local AppData%");

    Info += tr("Browser profile path: %1\n").arg(ProfilePath);

    m_pSummary->setText(Info);
}

bool CFinishTemplatePage::validatePage()
{
    return true;
}