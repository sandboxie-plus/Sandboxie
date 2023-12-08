#include "stdafx.h"

#include "BoxAssistant.h"
#include "../MiscHelpers/Common/Common.h"
#include "../MiscHelpers/Common/OtherFunctions.h"
#include "../Windows/SettingsWindow.h"
#include "../Windows/SelectBoxWindow.h"
#include "../SandMan.h"
#include "Helpers/WinAdmin.h"
#include <QButtonGroup>
#include <QListWidget>
#include "../QSbieAPI/SbieUtils.h"
#include "../Engine/BoxEngine.h"
#include "../Engine/SysObject.h"
#include "../Engine/ScriptManager.h"
#include "../MiscHelpers/Archive/Archive.h"
#include <QJsonDocument>
#include <QJsonObject>
#include "../MiscHelpers/Common/CheckableMessageBox.h"
#include <QHttpMultiPart>
#include "../Views/TraceView.h"
#include "../AddonManager.h"
#include "../MiscHelpers/Common/NetworkAccessManager.h"
#include "../CustomStyles.h"
#include "../OnlineUpdater.h"

CBoxAssistant::CBoxAssistant(QWidget *parent)
    : QWizard(parent)
{
    setWindowTitle(tr("Troubleshooting Wizard"));

    m_pEngine = NULL;
    m_bUseDebugger = false;
    m_pDebugger = NULL;

	QAction* pDbgAction = new QAction(tr("Toggle Debugger"));
	pDbgAction->setShortcut(QKeySequence("Ctrl+Shift+D"));
	connect(pDbgAction, SIGNAL(triggered()), this, SLOT(OnToggleDebugger()));
    addAction(pDbgAction);

    m_NextCounter = 0;

    setPage(Page_Begin, new CBeginPage);
    setPage(Page_Group, new CGroupPage);
    setPage(Page_List, new CListPage);
    setPage(Page_Run, new CRunPage);
    setPage(Page_Submit, new CSubmitPage);
    setPage(Page_Complete, new CCompletePage);
    
    setWizardStyle(ModernStyle);
    setPixmap(QWizard::LogoPixmap, QPixmap(":/SandMan.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    theGUI->GetScripts()->LoadIssues();
    connect(theGUI->GetScripts(), SIGNAL(IssuesUpdated), this, SLOT(IssuesUpdated()));
}

CBoxAssistant::~CBoxAssistant()
{
}

void CBoxAssistant::TryFix(quint32 MsgCode, const QStringList& MsgData, const QString& ProcessName, const QString& BoxName)
{
    QString Msg = QString("SBIE%1").arg(MsgCode & 0xFFFF);
    QMap<QString, QList<QVariantMap>> GroupedIssues = theGUI->GetScripts()->GetIssues();
    QVariantMap Issue;
    for (auto I = GroupedIssues.begin(); I != GroupedIssues.end(); ++I) {
        for(auto J = I->begin(); J != I->end(); ++J) {
            // Find message specific handler
            if (J->value("id").toString().compare(Msg, Qt::CaseInsensitive) == 0) {
                Issue = *J;
                break;
            }
            // fallback to generig message handler
            else if (Issue.isEmpty() && J->value("id").toString().compare("SBIEMSG", Qt::CaseInsensitive) == 0)
                Issue = *J;
        }
    }
    if (!Issue.isEmpty()) {
        PushIssue(Issue);
        m_Params["sbieMsg"] = theGUI->FormatSbieMessage(MsgCode, MsgData, ProcessName);
        m_Params["docLink"] = theGUI->MakeSbieMsgLink(MsgCode, MsgData, ProcessName);
        m_Params["msgCode"] = MsgCode & 0xFFFF;
        m_Params["msgData"] = QStringList(MsgData.mid(1));
        m_Params["processName"] = (!ProcessName.isEmpty() && ProcessName.left(4) != "PID:") ? ProcessName : QString();
        m_Params["boxName"] = BoxName;
        setStartId(Page_Run);
    }
}

void CBoxAssistant::OnIssuesUpdated()
{
    CBeginPage* pBegin = qobject_cast<CBeginPage*>(currentPage());
    if (pBegin)
        pBegin->initializePage();
}

void CBoxAssistant::OnToggleDebugger()
{
    m_bUseDebugger = !m_bUseDebugger;

    if (m_bUseDebugger && theGUI->GetAddonManager()->GetAddon("V4dbg", CAddonManager::eInstalled).isNull())
        theGUI->GetAddonManager()->TryInstallAddon("V4dbg", this, tr("To debug troubleshooting scripts you need the V4 Script Debugger add-on, do you want to download and install it?"));

    QString title = windowTitle();
    if (m_bUseDebugger)
        setWindowTitle(title + " - " + tr("Debugger Enabled"));
    else
        setWindowTitle(title.mid(0, title.indexOf(" - ")));
}

QList<QVariantMap> CBoxAssistant::GetIssues(const QVariantMap& Root) const 
{ 
    QMap<QString, QList<QVariantMap>> GroupedIssues = theGUI->GetScripts()->GetIssues();

    if (Root.contains("id"))
        return GroupedIssues.value(Root["id"].toString()); 

    QString Class = Root["class"].toString();
    QList<QVariantMap> AllIssues;
    for (auto I = GroupedIssues.begin(); I != GroupedIssues.end(); ++I) {
        for(auto J = I->begin(); J != I->end(); ++J) {
            if (J->value("type") == "issue" 
                && (Class.isEmpty() || J->value("class").toString().compare(Class, Qt::CaseInsensitive) == 0))
                AllIssues.append(*J);
        }
    }
    return AllIssues;
}

bool CBoxAssistant::StartEngine()
{
    QVariantMap Issue = CurrentIssue();

    QString Script = Issue["script"].toString();
    QString Name = Issue["id"].toString();

    if (!Script.isEmpty()) {
        m_pEngine = new CWizardEngine(this);

        connect(m_pEngine, SIGNAL(LogMessage(const QString&)), theGUI, SLOT(AddLogMessage(const QString&)));

        connect(m_pEngine, SIGNAL(BoxUsed(const CSandBoxPtr&)), this, SLOT(OnBoxUsed(const CSandBoxPtr&)));

        m_pEngine->AppendLog(QString("Starting troubleshooting script: %1").arg(Issue["id"].toString())); // no tr

        if (m_bUseDebugger) {
            QObject* pDebuggerBackend = m_pEngine->GetDebuggerBackend();
            if (pDebuggerBackend != NULL) {
                QObject* pDebuggerFrontend = newJSScriptDebuggerFrontendDynamic();

                QObject::connect(pDebuggerBackend, SIGNAL(sendResponse(QVariant)), pDebuggerFrontend, SLOT(processResponse(QVariant)), Qt::QueuedConnection);
                QObject::connect(pDebuggerFrontend, SIGNAL(sendRequest(QVariant)), pDebuggerBackend, SLOT(processRequest(QVariant)), Qt::QueuedConnection);

                m_pDebugger = newJSScriptDebuggerDynamic(pDebuggerFrontend);
                //connect(pDebugger, SIGNAL(detach()), this, ...);
                m_pDebugger->resize(1024, 640);
                m_pDebugger->restoreGeometry(theConf->GetBlob("DebuggerWindow/Window_Geometry"));
                m_pDebugger->show();
            }
            else {
                QMessageBox::critical(this, "Sandboxie-Plus", tr("V4ScriptDebuggerBackend could not be instantiated, probably V4ScriptDebugger.dll and or its dependencies are missing, script debugger could not be opened."));
            }
        }

        return m_pEngine->RunScript(Script, Name, m_Params);
    }
    return true;
}

void CBoxAssistant::KillEngine()
{
    m_pEngine->AppendLog(QString("Troubleshooting script terminated")); // no tr

    if (m_pDebugger) {

        QObject* pDebuggerBackend = m_pEngine->GetDebuggerBackend();
        QMetaObject::invokeMethod(pDebuggerBackend, "detach", Qt::DirectConnection);

        m_pDebugger->close();
        theConf->SetBlob("DebuggerWindow/Window_Geometry", m_pDebugger->saveGeometry());
        m_pDebugger->deleteLater();
        m_pDebugger = NULL;
    }

    delete m_pEngine;
    m_pEngine = NULL;
}

void CBoxAssistant::OnBoxUsed(const CSandBoxPtr& pBox)
{
    SUsedBox UsedBox;
    UsedBox.pBox = pBox;
    QDir Dir(pBox->GetFileRoot());
    foreach(const QFileInfo & Info, Dir.entryInfoList(QStringList() << "*.dmp", QDir::Files))
        UsedBox.OldDumps.append(Info.fileName());
    m_UsedBoxes.append(UsedBox);
}

void CBoxAssistant::accept()
{
    if (m_pEngine && currentId() != Page_Submit)
        m_pEngine->ApplyShadowChanges();
    QWizard::accept();
}

void CBoxAssistant::reject()
{
    if (m_pEngine && currentId() != Page_Submit) {
        if (theConf->GetInt("Options/WarnWizardOnClose", -1) == -1) {
            bool State = false;
            if (CCheckableMessageBox::question(this, "Sandboxie-Plus", tr("A troubleshooting procedure is in progress, canceling the wizard will abort it, this may leave the sandbox in an inconsistent state.")
                , tr("Don't ask in future"), &State, QDialogButtonBox::Ok | QDialogButtonBox::Cancel, QDialogButtonBox::Cancel) == QDialogButtonBox::Cancel)
                return;
            if (State)
                theConf->SetValue("Options/WarnWizardOnClose", 1);
        }
    }
    QWizard::reject();
}


//////////////////////////////////////////////////////////////////////////////////////////
// CBeginPage
// 

CBeginPage::CBeginPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Troubleshooting Wizard"));
    QPixmap Logo = QPixmap(theGUI->m_DarkTheme ? ":/SideLogoDM.png" : ":/SideLogo.png");
    int Scaling = theConf->GetInt("Options/FontScaling", 100);
    if(Scaling !=  100) Logo = Logo.scaled(Logo.width() * Scaling / 100, Logo.height() * Scaling / 100);
    setPixmap(QWizard::WatermarkPixmap, Logo);

    int row = 0;
    m_pLayout = new QGridLayout;
    QLabel* pTopLabel = new QLabel(tr("Welcome to the Troubleshooting Wizard for Sandboxie-Plus. "
        "This interactive assistant is designed to help you in resolving sandboxing issues."));
    pTopLabel->setWordWrap(true);
    m_pLayout->addWidget(pTopLabel, row++, 0, 1, 3);

    m_pLayout->addItem(new QSpacerItem(40, 10, QSizePolicy::Fixed, QSizePolicy::Fixed), row, 0);
    m_pLayout->addItem(new QSpacerItem(40, 10, QSizePolicy::Expanding, QSizePolicy::Fixed), row, 2);
    
    setLayout(m_pLayout);
}

void CBeginPage::initializePage() 
{
    foreach(QWidget * pWidget, m_pWidgets)
        delete pWidget;
    m_pWidgets.clear();

    int row = 2;

    auto AddIssue = [&](QVariantMap Issue) {
        QPushButton* pIssue = new QPushButton(theGUI->GetScripts()->Tr(Issue["name"].toString()));
        pIssue->setProperty("issue", Issue);
        connect(pIssue, SIGNAL(clicked(bool)), this, SLOT(OnCategory()));
        pIssue->setIcon(CSandMan::GetIcon(Issue["icon"].toString()));
        pIssue->setIconSize(QSize(32, 32));
        pIssue->setProperty("leftButton", true);
        pIssue->setStyle(new MyButtonStyle(pIssue->style()));
        m_pLayout->addWidget(pIssue, row++, 1);
        m_pWidgets.append(pIssue);
        return pIssue;
    };

    QVariantMap Root;
    Root["id"] = "root";
    foreach(auto Issue, ((CBoxAssistant*)wizard())->GetIssues(Root)) {
        if (((CBoxAssistant*)wizard())->GetIssues(Issue).isEmpty() && Issue["type"] != "issue")
            continue;
        AddIssue(Issue);
    }

    m_pLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Expanding), row++, 0);

    if (!g_CertInfo.active || g_CertInfo.expired) {
        QLabel* pBottomLabel = new QLabel(tr("With a valid <a href=\"https://sandboxie-plus.com/go.php?to=sbie-cert\">supporter certificate</a> the wizard would be even more powerful. "
            "It could access the <a href=\"https://sandboxie-plus.com/go.php?to=sbie-issue-db\">online solution database</a> to retrieve the latest troubleshooting instructions."));
        connect(pBottomLabel, SIGNAL(linkActivated(const QString&)), theGUI, SLOT(OpenUrl(const QString&)));
        pBottomLabel->setWordWrap(true);
        m_pLayout->addWidget(pBottomLabel, row++, 0, 1, 3);
        m_pWidgets.append(pBottomLabel);
    }
}

void CBeginPage::OnCategory()
{
    QVariantMap Issue = sender()->property("issue").toMap();
    ((CBoxAssistant*)wizard())->PushIssue(Issue);
    wizard()->next();
}

int CBeginPage::nextId() const
{
    QVariantMap Issue = ((CBoxAssistant*)wizard())->CurrentIssue();
    QString type = Issue["type"].toString();
    if (type == "issue")
        return CBoxAssistant::Page_Run;
    if (type == "list")
        return CBoxAssistant::Page_List;
    return CBoxAssistant::Page_Group;
}

bool CBeginPage::isComplete() const
{
    //return false;
     return true;
}

bool CBeginPage::validatePage() 
{
    if (((CBoxAssistant*)wizard())->CurrentIssue().isEmpty()) {
        QVariantMap Issue;
        Issue["type"] = "list";
        Issue["name"] = tr("Another issue");
        ((CBoxAssistant*)wizard())->PushIssue(Issue);
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// CGroupPage
// 

CGroupPage::CGroupPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Select issue from group"));
    QPixmap Logo = QPixmap(theGUI->m_DarkTheme ? ":/SideLogoDM.png" : ":/SideLogo.png");
    int Scaling = theConf->GetInt("Options/FontScaling", 100);
    if(Scaling !=  100) Logo = Logo.scaled(Logo.width() * Scaling / 100, Logo.height() * Scaling / 100);
    setPixmap(QWizard::WatermarkPixmap, Logo);

    int row = 0;
    m_pLayout = new QGridLayout;
    m_pLayout->setSpacing(2);
    m_pTopLabel = new QLabel(tr("Please specify the exact issue:"));
    m_pTopLabel->setWordWrap(true);
    m_pLayout->addWidget(m_pTopLabel, row++, 0, 1, 2);
    
    m_pGroup = new QButtonGroup();
    connect(m_pGroup, SIGNAL(idToggled(int, bool)), this, SIGNAL(completeChanged()));

    setLayout(m_pLayout);
}

void CGroupPage::initializePage()
{
    int row = 2;

    foreach(QWidget * pWidget, m_pWidgets)
        delete pWidget;
    m_pWidgets.clear();

    m_pLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Preferred), row++, 1);

    QVariantMap Group = ((CBoxAssistant*)wizard())->CurrentIssue();

    m_pTopLabel->setText(theGUI->GetScripts()->Tr(Group["description"].toString()));
   
    //QLabel* pCommon = new QLabel(tr("Common Issues:"));
    //m_pLayout->addWidget(pCommon, row++, 0);
    //m_pWidgets.append(pCommon);

    auto AddIssue = [&](QVariantMap Issue) {
        QRadioButton* pIssue = new QRadioButton(theGUI->GetScripts()->Tr(Issue["name"].toString()));
        pIssue->setToolTip(theGUI->GetScripts()->Tr(Issue["description"].toString()));
        pIssue->setProperty("issue", Issue);
        m_pGroup->addButton(pIssue);
        m_pLayout->addWidget(pIssue, row++, 1);
        m_pWidgets.append(pIssue);
    };

    foreach(auto Issue, ((CBoxAssistant*)wizard())->GetIssues(Group))
        AddIssue(Issue);

    if (!Group["class"].toString().isEmpty()) {
        QWidget* pSpacer = new QWidget();
	    pSpacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        m_pLayout->addWidget(pSpacer, row++, 0);
        m_pWidgets.append(pSpacer);

        //QLabel* pOther = new QLabel(tr("More Issues:"));
        //m_pLayout->addWidget(pOther, row++, 0);
        //m_pWidgets.append(pOther);

        QVariantMap Issue;
        Issue["type"] = "list";
        Issue["class"] = Group["class"];
        Issue["name"] = tr("Another issue");
        AddIssue(Issue);

        pSpacer = new QWidget();
	    pSpacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        m_pLayout->addWidget(pSpacer, row++, 0);
        m_pWidgets.append(pSpacer);
    }
}

void CGroupPage::cleanupPage()
{
    ((CBoxAssistant*)wizard())->PopIssue();

    QPointer<QWizard> w = wizard();
    QTimer::singleShot(10, [w]() {
        if (w && ((CBoxAssistant*)w.data())->m_NextCounter > 0)
            ((CBoxAssistant*)w.data())->removePage(CBoxAssistant::Page_Next + ((CBoxAssistant*)w.data())->m_NextCounter--);
        });
}

int CGroupPage::nextId() const
{
    if (QAbstractButton* pButton = m_pGroup->checkedButton()) {
        QVariantMap Issue = pButton->property("issue").toMap();
        QString type = Issue["type"].toString();
        if (type == "issue")
            return CBoxAssistant::Page_Run;
        if (type == "group")
            return CBoxAssistant::Page_Next + ((CBoxAssistant*)wizard())->m_NextCounter;
        if (type == "list")
            return CBoxAssistant::Page_List;
    }
    return CBoxAssistant::Page_Submit;
}

bool CGroupPage::isComplete() const
{
    return m_pGroup->checkedId() != -1;
}

bool CGroupPage::validatePage() 
{
    if (QAbstractButton* pButton = m_pGroup->checkedButton()) {
        QVariantMap Issue = pButton->property("issue").toMap();
        QString type = Issue["type"].toString();
        if (type == "group" || type == "list") 
            ((CBoxAssistant*)wizard())->setPage(CBoxAssistant::Page_Next + ++((CBoxAssistant*)wizard())->m_NextCounter, new CGroupPage());
        ((CBoxAssistant*)wizard())->PushIssue(Issue);
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// CListPage
// 

CListPage::CListPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Select issue from full list"));
    QPixmap Logo = QPixmap(theGUI->m_DarkTheme ? ":/SideLogoDM.png" : ":/SideLogo.png");
    int Scaling = theConf->GetInt("Options/FontScaling", 100);
    if(Scaling !=  100) Logo = Logo.scaled(Logo.width() * Scaling / 100, Logo.height() * Scaling / 100);
    setPixmap(QWizard::WatermarkPixmap, Logo);

    int row = 0;
    m_pLayout = new QGridLayout;
    m_pLayout->setSpacing(2);
    //QLabel* pTopLabel = new QLabel(tr("Please select an issue from the list"));
    //pTopLabel->setWordWrap(true);
    //m_pLayout->addWidget(pTopLabel, row++, 0, 1, 2);
    
    m_pFilter = new QLineEdit();
    m_pFilter->setPlaceholderText(tr("Search filter"));
    m_pLayout->addWidget(m_pFilter, row++, 0, 1, 2);
    connect(m_pFilter, SIGNAL(textChanged(const QString &)), this, SLOT(ApplyFilter()));

    m_pList = new QListWidget();
    m_pLayout->addWidget(m_pList, row++, 0, 1, 2);
    connect(m_pList, SIGNAL(itemClicked(QListWidgetItem *)), this, SIGNAL(completeChanged()));

    setLayout(m_pLayout);
}

void CListPage::ApplyFilter()
{
    static bool UpdatePending = false;
    if (!UpdatePending) {
        UpdatePending = true;
        QTimer::singleShot(100, [=]() {
            UpdatePending = false;
            LoadIssues();
        });
    }
}

void CListPage::LoadIssues()
{
    m_pList->clear();

    QVariantMap List = ((CBoxAssistant*)wizard())->CurrentIssue();

    int iAny = List.contains("id") ? 0 : 1;

    auto AddIssue = [&](QVariantMap Issue) {
        QListWidgetItem* pItem = new QListWidgetItem();
        pItem->setText(theGUI->GetScripts()->Tr(Issue["name"].toString()));
        pItem->setToolTip(theGUI->GetScripts()->Tr(Issue["description"].toString()));
        pItem->setData(Qt::UserRole, Issue);
        if (iAny != 1 && Issue["bold"].toBool()) {
            QFont font = pItem->font();
            font.setBold(true);
            pItem->setFont(font);
        }
        m_pList->addItem(pItem);
    };

    QString Filter = m_pFilter->text();

    foreach(auto Issue, ((CBoxAssistant*)wizard())->GetIssues(List)) {
        if (Filter.isEmpty()
          || theGUI->GetScripts()->Tr(Issue["name"].toString()).contains(Filter, Qt::CaseInsensitive)
          || theGUI->GetScripts()->Tr(Issue["description"].toString()).contains(Filter, Qt::CaseInsensitive))
            AddIssue(Issue);
    }

    if (iAny) {
        QVariantMap Issue;
        Issue["type"] = "submit";
        Issue["name"] = tr("None of the above");
        Issue["bold"] = true;
        iAny = 2;
        AddIssue(Issue);
    }
    else
        setTitle(theGUI->GetScripts()->Tr(List["name"].toString()));
}

void CListPage::initializePage()
{
    m_pFilter->clear();
    LoadIssues();
}

void CListPage::cleanupPage()
{
    ((CBoxAssistant*)wizard())->PopIssue();

    QPointer<QWizard> w = wizard();
    QTimer::singleShot(10, [w]() {
        if (w && ((CBoxAssistant*)w.data())->m_NextCounter > 0)
            ((CBoxAssistant*)w.data())->removePage(CBoxAssistant::Page_Next + ((CBoxAssistant*)w.data())->m_NextCounter--);
        });
}

int CListPage::nextId() const
{
    if (QListWidgetItem* pItem = m_pList->currentItem()) {
        QVariantMap Issue = pItem->data(Qt::UserRole).toMap();
        QString type = Issue["type"].toString();
        if (type == "issue")
            return CBoxAssistant::Page_Run;
        if (type == "group")
            return CBoxAssistant::Page_Next + ((CBoxAssistant*)wizard())->m_NextCounter;
        if (type == "list")
            return CBoxAssistant::Page_List;
    }
    return CBoxAssistant::Page_Submit;
}

bool CListPage::isComplete() const
{
    return !m_pList->selectedItems().isEmpty();
}

bool CListPage::validatePage()
{
    if (QListWidgetItem* pItem = m_pList->currentItem()) {
        QVariantMap Issue = pItem->data(Qt::UserRole).toMap();
        QString type = Issue["type"].toString();
        if (type == "group" || type == "list") 
            ((CBoxAssistant*)wizard())->setPage(CBoxAssistant::Page_Next + ++((CBoxAssistant*)wizard())->m_NextCounter, new CGroupPage());
        ((CBoxAssistant*)wizard())->PushIssue(Issue);
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// CRunPage
//  

CRunPage::CRunPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Troubleshooting ..."));
    setSubTitle(tr(""));

    m_pLayout = new QGridLayout;
    m_pTopLabel = new QLabel();
    connect(m_pTopLabel, SIGNAL(linkActivated(const QString&)), theGUI, SLOT(OpenUrl(const QString&)));
    m_pTopLabel->setWordWrap(true);
    m_pLayout->addWidget(m_pTopLabel, 0, 0, 1, 2);
    m_pForm = NULL;

    setLayout(m_pLayout);
}

void CRunPage::initializePage()
{
    QVariantMap Issue = ((CBoxAssistant*)wizard())->CurrentIssue();
    setTitle(theGUI->GetScripts()->Tr(Issue["name"].toString()));
    setSubTitle(theGUI->GetScripts()->Tr(Issue["description"].toString()));

    if(((CBoxAssistant*)wizard())->StartEngine()) {
        CWizardEngine* pEngine = ((CBoxAssistant*)wizard())->GetEngine();
        connect(pEngine, SIGNAL(StateChanged(int, const QString&)), this, SLOT(OnStateChanged(int, const QString&)));
    } else
        m_pTopLabel->setText(tr("This troubleshooting procedure could not be initialized. "
            "You can click on next to submit an issue report."));
}

void CRunPage::cleanupPage()
{
    ((CBoxAssistant*)wizard())->KillEngine();
}

void CRunPage::OnStateChanged(int state, const QString& Text)
{
    CWizardEngine* pEngine = ((CBoxAssistant*)wizard())->GetEngine();
    if (!pEngine || pEngine != sender())
        return;

    if (m_pForm) {
        delete m_pForm;
        m_pForm = NULL;
        m_pWidgets.clear();
    }

    bool bEnableNext = true;

    //qDebug() << "OnStateChanged" << state;
    switch (state)
    {
    case CBoxEngine::eRunning:
    case CBoxEngine::eRunningAsync:
        bEnableNext = false;
    case CBoxEngine::eReady:
        m_pTopLabel->setText(Text);
        break;
    case CBoxEngine::eQuery:
    {
        QVariantMap Query = pEngine->GetQuery();
        m_pTopLabel->setText(Query["text"].toString());

        if (Query["type"].toString().compare("form", Qt::CaseInsensitive) == 0) {

            m_pForm = new QWidget();
            QFormLayout* pForm = new QFormLayout(m_pForm);
            m_pLayout->addWidget(m_pForm, 1, 0, 1, 2);

            QVariantList Form = Query["form"].toList();
            foreach(const QVariant& vEntry, Form) {
                QVariantMap Entry = vEntry.toMap();
                QString type = Entry["type"].toString();
                QString name = theGUI->GetScripts()->Tr(Entry["name"].toString());
                QString hint = theGUI->GetScripts()->Tr(Entry["hint"].toString());
                QVariant value = Entry["value"].toString();
                QWidget* pWidget;
                if (type.compare("label", Qt::CaseInsensitive) == 0) {
                    pWidget = new QLabel(name);
                    pForm->addRow(pWidget);
                } 
                else if (type.compare("file", Qt::CaseInsensitive) == 0 || type.compare("folder", Qt::CaseInsensitive) == 0) {
                    CPathEdit* pPath = new CPathEdit(type.compare("folder", Qt::CaseInsensitive) == 0);
                    pWidget = pPath;
                    pPath->SetText(value.toString());
                    pForm->addRow(name, pPath);
                } 
                else if (type.compare("edit", Qt::CaseInsensitive) == 0) {
                    QLineEdit* pEdit = new QLineEdit();
                    pWidget = pEdit;
                    pEdit->setText(value.toString());
                    pEdit->setPlaceholderText(hint);
                    pForm->addRow(name, pEdit);
                } 
                else if (type.compare("check", Qt::CaseInsensitive) == 0) {
                    QCheckBox* pCheck = new QCheckBox(name);
                    pWidget = pCheck;
                    pCheck->setChecked(value.toBool());
                    pForm->addRow("", pCheck);
                } 
                else if (type.compare("radio", Qt::CaseInsensitive) == 0) {
                    QRadioButton* pRadio = new QRadioButton(name);
                    pWidget = pRadio;
                    pRadio->setChecked(value.toBool());
                    pForm->addRow("", pRadio);
                    
                    // todo: add mandatory flag for other fields
                    bEnableNext = false; // user must make a choice
                    connect(pRadio, SIGNAL(toggled(bool)), this, SLOT(CheckUserInput()));
                } 
                else if (type.compare("box", Qt::CaseInsensitive) == 0) {
                    QString Name = name;
                    //if(!Name.isEmpty()) pForm->addRow(new QLabel(Name));
                    CBoxPicker* pPicker = new CBoxPicker(value.toString());
                    pWidget = pPicker;
                    pForm->addRow(Name, pPicker);
                } 
                else if (type.compare("combo", Qt::CaseInsensitive) == 0) {
                    QComboBox* pCombo = new QComboBox();
                    pWidget = pCombo;
                    foreach(const QVariant & vItem, Entry["items"].toList()) {
                        if (vItem.type() == QVariant::Map) {
                            QVariantMap Item = vItem.toMap();
                            pCombo->addItem(theGUI->GetScripts()->Tr(Item["name"].toString()), Item["value"]);
                        } else
                            pCombo->addItem(vItem.toString());
                    }
                    pForm->addRow(name, pCombo);
                }
                pWidget->setToolTip(theGUI->GetScripts()->Tr(Entry["description"].toString()));
                if (Entry["disabled"].toBool()) pWidget->setDisabled(true);
                QString id = Entry["id"].toString();
                if (!id.isEmpty()) m_pWidgets.insert(id, pWidget);
            }
        }
        break;
    }
    case CBoxEngine::eError:
        m_pTopLabel->setText(tr("Something failed internally this troubleshooting procedure can not continue. "
            "You can click on next to submit an issue report.") + (pEngine ? tr("\n\nError: ") + Text : ""));
    case CBoxEngine::eCanceled:
        break;
    case CBoxEngine::eCompleted:
    case CBoxEngine::eSuccess:
    case CBoxEngine::eFailed:
        wizard()->next();
        break;
    }

    if(bEnableNext)
        wizard()->button(QWizard::NextButton)->setEnabled(true);
}

void CRunPage::CheckUserInput()
{
    wizard()->button(QWizard::NextButton)->setEnabled(true);
}

bool CRunPage::isComplete() const 
{
    return true;
}

int CRunPage::nextId() const
{
    CWizardEngine* pEngine = ((CBoxAssistant*)wizard())->GetEngine();
    if(!pEngine || !pEngine->WasSuccessfull())
        return CBoxAssistant::Page_Submit;
    return CBoxAssistant::Page_Complete;
}

bool CRunPage::validatePage()
{
    CWizardEngine* pEngine = ((CBoxAssistant*)wizard())->GetEngine();

    if (!pEngine || !(pEngine->HasQuery() || pEngine->IsReady())) {
        // disables back button on next page
        setCommitPage(true);
        return true;
    }

    // disable back button on the current page
    wizard()->button(QWizard::BackButton)->setEnabled(false);
    // disable next button, OnStateChanged will re-enable it
    wizard()->button(QWizard::NextButton)->setEnabled(false);

    if (pEngine->HasQuery()) {
        QVariantMap Reply;
        for (auto I = m_pWidgets.begin(); I != m_pWidgets.end(); ++I) {
            QVariant Value;
            if (CPathEdit* pPath = qobject_cast<CPathEdit*>(I.value()))
                Value = pPath->GetText();
            else if (QLineEdit* pEdit = qobject_cast<QLineEdit*>(I.value()))
                Value = pEdit->text();
            else if (QCheckBox* pCheck = qobject_cast<QCheckBox*>(I.value()))
                Value = pCheck->isChecked();
            else if (QRadioButton* pRadio = qobject_cast<QRadioButton*>(I.value()))
                Value = pRadio->isChecked();
            else if (CBoxPicker* pPicker = qobject_cast<CBoxPicker*>(I.value()))
                Value = pPicker->GetBoxName();
            else if (QComboBox* pCombo = qobject_cast<QComboBox*>(I.value())) {
                Value = pCombo->currentData();
                if (!Value.isValid())
                    Value = pCombo->currentIndex();
            }
            Reply[I.key()] = Value;
        }
        pEngine->SetResult(Reply);
    }
    else if (pEngine->IsReady()) {
        pEngine->Continue();
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// CSubmitPage
// 

CSubmitPage::CSubmitPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Submit Issue Report"));
    QPixmap Logo = QPixmap(theGUI->m_DarkTheme ? ":/SideLogoDM.png" : ":/SideLogo.png");
    int Scaling = theConf->GetInt("Options/FontScaling", 100);
    if(Scaling !=  100) Logo = Logo.scaled(Logo.width() * Scaling / 100, Logo.height() * Scaling / 100);
    setPixmap(QWizard::WatermarkPixmap, Logo);

    int row = 0;
    QGridLayout* pLayout = new QGridLayout;
    pLayout->setSpacing(2);
    m_pTopLabel = new QLabel();
    m_pTopLabel->setWordWrap(true);
    pLayout->addWidget(m_pTopLabel, row++, 0, 1, 3);

    m_pReport = new QTextEdit();
    m_pReport->setPlaceholderText(tr("Detailed issue description"));
    //m_pReport->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pLayout->addWidget(m_pReport, row++, 0, 1, 3);

    m_pAttachIni = new QCheckBox(tr("Attach Sandboxie.ini"));
    m_pAttachIni->setToolTip(tr("Sandboxing compatibility is reliant on the configuration, hence attaching the Sandboxie.ini file helps a lot with finding the issue."));
    pLayout->addWidget(m_pAttachIni, row, 0);

    m_pAttachLog = new QCheckBox(tr("Attach Logs"));
    m_pAttachLog->setTristate(true);
    m_pAttachLog->setToolTip(tr("Selecting partially checked state sends only the message log, but not the trace log.\nBefore sending, you can review the logs in the main window."));
    pLayout->addWidget(m_pAttachLog, row, 1);

    m_pAttachDmp = new QCheckBox(tr("Attach Crash Dumps"));
    m_pAttachDmp->setToolTip(tr("An application crashed during the troubleshooting procedure, attaching a crash dump can help with the debugging."));
    pLayout->addWidget(m_pAttachDmp, row, 2);

    m_pMail = new QLineEdit();
    m_pMail->setPlaceholderText(tr("Email address"));
    m_pMail->setToolTip(tr("You have the option to provide an email address to receive a notification once a solution for your issue has been identified."));
    pLayout->addWidget(m_pMail, ++row, 0, 1, 3);

    setLayout(pLayout);
}

void CSubmitPage::initializePage()
{
    QString Info = tr("We apologize for the inconvenience you are currently facing with Sandboxie-Plus. ");

    QString Report; // DO NOT TRANSLATE - Reports must be in English!

    CWizardEngine* pEngine = ((CBoxAssistant*)wizard())->GetEngine();
    if (pEngine) {
        if (pEngine->HasFailed() || pEngine->HasError())
            Info += tr("Unfortunately, the automated troubleshooting procedure failed. ");

        QVariantMap Issue = ((CBoxAssistant*)wizard())->CurrentIssue();
        Report = QString("Troubleshooting procedure '%1'").arg(Issue["id"].toString());
    }
    else {
        Info += tr("Regrettably, there is no automated troubleshooting procedure available for the specific issue you have described. ");

        Report = "[PLEASE ENTER YOUR ISSUE DESCRIPTION HERE]";
    }

    Info += tr("If you wish to submit an issue report, please review the report below and click 'Finish'.");

    m_pTopLabel->setText(Info);

    Report += "\n\nFurther information:\n";
    Report += "Sandboxie-Plus Version: " + theGUI->GetVersion() + "\n";
    Report += "Operating System Version: " + QSysInfo::kernelVersion() + "-" + QSysInfo::currentCpuArchitecture() + "\n";

    bool bNewDumps = false;
    foreach(auto & UsedBox, ((CBoxAssistant*)wizard())->m_UsedBoxes) {
        QDir Dir(UsedBox.pBox->GetFileRoot());
        foreach(const QFileInfo & Info, Dir.entryInfoList(QStringList() << "*.dmp", QDir::Files)) {
            if (!UsedBox.OldDumps.contains(Info.fileName())) {
                bNewDumps = true;
                break;
            }
        }
        Report += "Used Sandbox: " + UsedBox.pBox->GetName() + "\n";
    }

    if (pEngine) {
        QVariantMap Values = pEngine->GetReport();
        for (auto I = Values.begin(); I != Values.end(); ++I)
            Report += I.key() + ": " + I->toString() + "\n";
    }
    
    m_pReport->setText(Report);


    m_pAttachIni->setChecked(true);

    m_pAttachLog->setChecked(true);
    //bool bHasLog = !theAPI->GetTrace().isEmpty();
    //m_pAttachLog->setEnabled(bHasLog);
    //m_pAttachLog->setChecked(bHasLog);

    m_pAttachDmp->setEnabled(bNewDumps);
    m_pAttachDmp->setChecked(bNewDumps);
}

bool CSubmitPage::validatePage()
{
    QBuffer* pTraceLog = NULL;
    if (m_pAttachLog->checkState() == Qt::Checked) {
        pTraceLog = new QBuffer();
        pTraceLog->open(QIODevice::ReadWrite);
        if (!CTraceView::SaveToFile(pTraceLog)) {
            delete pTraceLog;
            return false;
        }
    }

    m_pUploadProgress = CSbieProgressPtr(new CSbieProgress());
	theGUI->AddAsyncOp(m_pUploadProgress, false, QString(), this);

    CNetworkAccessManager* pRequestManager = new CNetworkAccessManager(30 * 1000, this);

    QHttpMultiPart* pMultiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart Report;
    Report.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"report\""));
    Report.setBody(m_pReport->toPlainText().toUtf8());
    pMultiPart->append(Report);

    if (m_pAttachIni->isChecked()) {

        QFile* pSbieIni = new QFile(theAPI->GetIniPath(), pMultiPart);

        if (pSbieIni->open(QIODevice::ReadOnly)) {

            QHttpPart SbieIni;
            SbieIni.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
            SbieIni.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"sbieIni\"; filename=\"Sandboxie.ini\""));

            SbieIni.setBodyDevice(pSbieIni);
            pMultiPart->append(SbieIni);
        }
    }

    if (m_pAttachLog->isChecked()) {

        QBuffer* pSbieLogs = new QBuffer(pMultiPart);
        CArchive Archive("SbieTrace.7z", pSbieLogs);

        QMap<int, QIODevice*> Files;

        QBuffer* pMessageLog = new QBuffer();
        pMessageLog->open(QIODevice::ReadWrite);
        theGUI->SaveMessageLog(pMessageLog);
        int ArcIndex = Archive.AddFile("SbieMsg.log");
        pMessageLog->seek(0);
	    Files.insert(ArcIndex, pMessageLog);

        if (pTraceLog) {
            ArcIndex = Archive.AddFile("SbieTrace.log");
            pTraceLog->seek(0);
            Files.insert(ArcIndex, pTraceLog);
        }

        m_pUploadProgress->ShowMessage(tr("Compressing Logs"));
        SCompressParams Params;
        Params.iLevel = 9;
        Archive.Update(&Files, true, &Params);
        
        if (pSbieLogs->open(QIODevice::ReadOnly)) {

            QHttpPart SbieLogs;
            SbieLogs.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-strea"));
            SbieLogs.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"sbieLogs\"; filename=\"SbieLogs.7z\""));

            SbieLogs.setBodyDevice(pSbieLogs);
            pMultiPart->append(SbieLogs);
        }
    }

    if (m_pAttachDmp->isChecked()) {

        QBuffer* pSbieDumps = new QBuffer(pMultiPart);
        CArchive Archive("SbieDumps.7z", pSbieDumps);

        QMap<int, QIODevice*> Files;

        foreach(auto & UsedBox, ((CBoxAssistant*)wizard())->m_UsedBoxes) {
            QDir Dir(UsedBox.pBox->GetFileRoot());
            foreach(const QFileInfo & Info, Dir.entryInfoList(QStringList() << "*.dmp", QDir::Files)) {
                if (!UsedBox.OldDumps.contains(Info.fileName())) {
                    QFile* pCrashDump = new QFile(Info.filePath());
                    int ArcIndex = Archive.AddFile(Info.fileName());
                    Files.insert(ArcIndex, pCrashDump);
                }
            }
        }

        m_pUploadProgress->ShowMessage(tr("Compressing Dumps"));
        SCompressParams Params;
        Params.iLevel = 9;
        Archive.Update(&Files, true, &Params);

        if (pSbieDumps->open(QIODevice::ReadOnly)) {

            QHttpPart SbieDumps;
            SbieDumps.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-strea"));
            SbieDumps.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"sbieDumps\"; filename=\"SbieDumps.7z\""));

            SbieDumps.setBodyDevice(pSbieDumps);
            pMultiPart->append(SbieDumps);
        }
    }

    if (!m_pMail->text().isEmpty()) {
        QHttpPart eMail;
        eMail.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"email\""));
        eMail.setBody(m_pMail->text().toUtf8());
        pMultiPart->append(eMail);
    }

    quint64 RandID = COnlineUpdater::GetRandID();
    QHttpPart randId;
    randId.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"randId\""));
    randId.setBody(QString::number(RandID, 16).rightJustified(16, '0').toUpper().toUtf8());
    pMultiPart->append(randId);

    QUrl Url("https://sandboxie-plus.com/issues/submit.php");
    QNetworkRequest Request(Url);
	//Request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
	Request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
	//Request.setRawHeader("Accept-Encoding", "gzip");
    QNetworkReply* pReply = pRequestManager->post(Request, pMultiPart);
    pMultiPart->setParent(pReply);

	m_pUploadProgress->ShowMessage(tr("Submitting issue report..."));

    connect(pReply, &QNetworkReply::finished, this, [pReply, this]() {
        QByteArray Reply = pReply->readAll();
        pReply->deleteLater();

        m_pUploadProgress->Finish(SB_OK);
		m_pUploadProgress.clear();

        QVariantMap Result = QJsonDocument::fromJson(Reply).toVariant().toMap();
        if (!Result["success"].toBool()) {
            QMessageBox::critical(this, "Sandboxie-Plus", tr("Failed to submit issue report, error %1\nTry submitting without the log attached.").arg(Result["error"].toInt()));
            return;
        }

        QMessageBox::information(this, "Sandboxie-Plus", tr("Your issue report has been successfully submitted, thank you."));
        wizard()->close();
    });

    connect(pReply, &QNetworkReply::uploadProgress, this, [this](qint64 bytes, qint64 bytesTotal) {
        if (bytesTotal != 0 && !m_pUploadProgress.isNull())
		    m_pUploadProgress->Progress(100 * bytes / bytesTotal);
    });

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// CCompletePage
// 

CCompletePage::CCompletePage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Troubleshooting Completed"));
    QPixmap Logo = QPixmap(theGUI->m_DarkTheme ? ":/SideLogoDM.png" : ":/SideLogo.png");
    int Scaling = theConf->GetInt("Options/FontScaling", 100);
    if(Scaling !=  100) Logo = Logo.scaled(Logo.width() * Scaling / 100, Logo.height() * Scaling / 100);
    setPixmap(QWizard::WatermarkPixmap, Logo);

    QVBoxLayout *pLayout = new QVBoxLayout;

    m_pLabel = new QLabel;
    m_pLabel->setWordWrap(true);
    m_pLabel->setText(tr("Thank you for using the Troubleshooting Wizard for Sandboxie-Plus. We apologize for any inconvenience you experienced during the process. " 
        "If you have any additional questions or need further assistance, please don't hesitate to reach out. We're here to help. "
        "Thank you for your understanding and cooperation. \n\nYou can click Finish to close this wizard."));
    pLayout->addWidget(m_pLabel);

    // todo: report success optionally

    setLayout(pLayout);
}

void CCompletePage::initializePage()
{
    //wizard()->button(QWizard::CancelButton)->setEnabled(false);
}