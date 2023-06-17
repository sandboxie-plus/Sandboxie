#include "stdafx.h"
#include "SandMan.h"
#include "../MiscHelpers/Common/Common.h"
#include "../MiscHelpers/Common/ExitDialog.h"
#include "../MiscHelpers/Common/SortFilterProxyModel.h"
#include "Views/SbieView.h"
#include "../MiscHelpers/Common/CheckableMessageBox.h"
#include <QWinEventNotifier>
#include "./Dialogs/MultiErrorDialog.h"
#include "../QSbieAPI/SbieUtils.h"
#include "../QSbieAPI/Sandboxie/BoxBorder.h"
#include "../QSbieAPI/Sandboxie/SbieTemplates.h"
#include "Windows/SettingsWindow.h"
#include "Windows/RecoveryWindow.h"
#include <QtConcurrent>
#include "../MiscHelpers/Common/SettingsWidgets.h"
#include "Windows/OptionsWindow.h"
#include <QProxyStyle>
#include "../MiscHelpers/Common/TreeItemModel.h"
#include "../MiscHelpers/Common/ListItemModel.h"
#include "Views/TraceView.h"
#include "Windows/SelectBoxWindow.h"
#include "../UGlobalHotkey/uglobalhotkeys.h"
#include "Wizards/SetupWizard.h"
#include "Helpers/WinAdmin.h"
#include "../MiscHelpers/Common/OtherFunctions.h"
#include "../MiscHelpers/Common/Common.h"
#include "Windows/SupportDialog.h"
#include "../MiscHelpers/Archive/Archive.h"
#include "../MiscHelpers/Archive/ArchiveFS.h"
#include "Views/FileView.h"
#include "OnlineUpdater.h"
#include "../MiscHelpers/Common/NeonEffect.h"
#include <QVariantAnimation>
#include <QSessionManager>
#include "Helpers/FullScreen.h"
#include "Helpers/WinHelper.h"
#include "../QSbieAPI/Helpers/DbgHelper.h"

CSbiePlusAPI* theAPI = NULL;

#include <wtypes.h>
#include <QAbstractNativeEventFilter>
#include <dbt.h>

class CNativeEventFilter : public QAbstractNativeEventFilter
{
public:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    virtual bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
#else
    virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *result)
#endif
	{
		if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG")
		{
			MSG *msg = static_cast<MSG *>(message);

			//if(msg->message != 275 && msg->message != 1025)
			//	qDebug() << msg->message;

			if (msg->message == WM_NOTIFY)
			{
				//return true;
			}
			else if (msg->message == WM_DEVICECHANGE)
			{
				if (msg->wParam == DBT_DEVICEARRIVAL // Drive letter added
				 || msg->wParam == DBT_DEVICEREMOVECOMPLETE) // Drive letter removed
				{
					/*DEV_BROADCAST_HDR* deviceBroadcast = (DEV_BROADCAST_HDR*)msg->lParam;
					if (deviceBroadcast->dbch_devicetype == DBT_DEVTYP_VOLUME) {
					}*/
					if (theAPI)
						theAPI->UpdateDriveLetters();
				}
				/*else if ((msg->wParam & 0xFF80) == 0xAA00 && msg->lParam == 'xobs') 
				{
					UCHAR driveNumber = (UCHAR)(msg->wParam & 0x1F);
					if (driveNumber < 26) {		
					}
				}
				else if (msg->wParam == DBT_DEVNODES_CHANGED) // hardware changed
				{
				}*/
			}
			else if (msg->message == WM_SETTINGCHANGE)
			{
				if (theGUI && theConf->GetInt("Options/UseDarkTheme", 2) == 2)
					theGUI->UpdateTheme();
			}
			else if (msg->message == WM_SHOWWINDOW && msg->wParam)
			{
				QWidget* pWidget = QWidget::find((WId)msg->hwnd);
				if (theGUI && pWidget && (pWidget->windowType() | Qt::Dialog) == Qt::Dialog)
					theGUI->UpdateTitleTheme(msg->hwnd);
			}
		}
		return false;
	}
};

HWND MainWndHandle = NULL;

CSandMan* theGUI = NULL;

extern QString g_PendingMessage;


CSandMan::CSandMan(QWidget *parent)
	: QMainWindow(parent)
{
#if defined(Q_OS_WIN)
	MainWndHandle = (HWND)QWidget::winId();

	QApplication::instance()->installNativeEventFilter(new CNativeEventFilter);
#endif

	CArchive::Init();

	theGUI = this;

	m_DarkTheme = false;
	m_FusionTheme = false;

	QDesktopServices::setUrlHandler("http", this, "OpenUrl");
	QDesktopServices::setUrlHandler("https", this, "OpenUrl");
	QDesktopServices::setUrlHandler("sbie", this, "OpenUrl");

	m_StartMenuUpdatePending = false;

	m_ThemeUpdatePending = false;
	m_DefaultStyle = QApplication::style()->objectName();
	m_DefaultPalett = QApplication::palette();
	m_DefaultFontSize = QApplication::font().pointSizeF();

	m_DarkPalett.setColor(QPalette::Light, QColor(96, 96, 96));
	m_DarkPalett.setColor(QPalette::Midlight, QColor(64, 64, 64));
	m_DarkPalett.setColor(QPalette::Mid, QColor(48, 48, 48));
	m_DarkPalett.setColor(QPalette::Dark, QColor(53, 53, 53));
	m_DarkPalett.setColor(QPalette::Shadow, QColor(25, 25, 25));
	m_DarkPalett.setColor(QPalette::Window, QColor(53, 53, 53));
	m_DarkPalett.setColor(QPalette::WindowText, Qt::white);
	m_DarkPalett.setColor(QPalette::Base, QColor(25, 25, 25));
	m_DarkPalett.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
	m_DarkPalett.setColor(QPalette::ToolTipBase, Qt::lightGray);
	m_DarkPalett.setColor(QPalette::ToolTipText, Qt::white);
	m_DarkPalett.setColor(QPalette::Text, Qt::white);
	m_DarkPalett.setColor(QPalette::Button, QColor(53, 53, 53));
	m_DarkPalett.setColor(QPalette::ButtonText, Qt::white);
	m_DarkPalett.setColor(QPalette::BrightText, Qt::red);
	m_DarkPalett.setColor(QPalette::Link, QColor(218, 130, 42));
	m_DarkPalett.setColor(QPalette::LinkVisited, QColor(218, 130, 42));
	m_DarkPalett.setColor(QPalette::Highlight, QColor(42, 130, 218));
	m_DarkPalett.setColor(QPalette::HighlightedText, Qt::black);
	m_DarkPalett.setColor(QPalette::PlaceholderText, QColor(96, 96, 96));
	m_DarkPalett.setColor(QPalette::Disabled, QPalette::WindowText, Qt::darkGray);
	m_DarkPalett.setColor(QPalette::Disabled, QPalette::Text, Qt::darkGray);
	m_DarkPalett.setColor(QPalette::Disabled, QPalette::Light, Qt::black);
	m_DarkPalett.setColor(QPalette::Disabled, QPalette::ButtonText, Qt::darkGray);

	LoadLanguage();

	if (!theConf->IsWritable()) {
		QMessageBox::critical(this, "Sandboxie-Plus", tr("WARNING: Sandboxie-Plus.ini in %1 cannot be written to, settings will not be saved.").arg(theConf->GetConfigDir()));
	}

	m_bExit = false;

	theAPI = new CSbiePlusAPI(this);
	connect(theAPI, SIGNAL(StatusChanged()), this, SLOT(OnStatusChanged()));

	connect(theAPI, SIGNAL(BoxAdded(const CSandBoxPtr&)), this, SLOT(OnBoxAdded(const CSandBoxPtr&)));
	connect(theAPI, SIGNAL(BoxClosed(const CSandBoxPtr&)), this, SLOT(OnBoxClosed(const CSandBoxPtr&)));
	connect(theAPI, SIGNAL(BoxCleaned(CSandBoxPlus*)), this, SLOT(OnBoxCleaned(CSandBoxPlus*)));

	QString appTitle = tr("Sandboxie-Plus v%1").arg(GetVersion());

	this->setWindowTitle(appTitle);

	setAcceptDrops(true);

	m_pBoxBorder = new CBoxBorder(theAPI, this);

	m_SbieTemplates = new CSbieTemplates(theAPI, this);


	m_bConnectPending = false;
	m_bStopPending = false;


	m_pUpdater = new COnlineUpdater(this);


	m_pMainWidget = new QWidget(this);

	m_pMenuBar = menuBar();
	connect(m_pMenuBar, SIGNAL(hovered(QAction*)), this, SLOT(OnMenuHover(QAction*)));

	QWidget* pMenuWidget = new QWidget(this);
	m_pMenuLayout = new QHBoxLayout(pMenuWidget);
	m_pMenuLayout->setContentsMargins(0, 0, 0, 0);
	//m_pMenuLayout->addWidget(m_pMenuBar);
	m_pMenuLayout->setMenuBar(m_pMenuBar);
	//m_pMenuLayout->addWidget(m_pLabel);
	//m_pMenuLayout->addStretch(10);
	setMenuWidget(pMenuWidget);

	CreateUI();
	setCentralWidget(m_pMainWidget);

	m_pTraceInfo = new QLabel();
	m_pDisabledForce = new QLabel();
	m_pDisabledRecovery = new QLabel();
	m_pDisabledMessages = new QLabel();
	statusBar()->addPermanentWidget(m_pTraceInfo);
	statusBar()->addPermanentWidget(m_pDisabledForce);
	statusBar()->addPermanentWidget(m_pDisabledRecovery);
	statusBar()->addPermanentWidget(m_pDisabledMessages);
	OnDisablePopUp(); // update statusbar



	m_pHotkeyManager = new UGlobalHotkeys(this);
	connect(m_pHotkeyManager, SIGNAL(activated(size_t)), SLOT(OnHotKey(size_t)));
	SetupHotKeys();

	//for (int i = 0; i < eMaxColor; i++) {
	//	m_BoxIcons[i].Empty = QIcon(QString(":/Boxes/Empty%1").arg(i));
	//	m_BoxIcons[i].InUse = QIcon(QString(":/Boxes/Full%1").arg(i));
	//}

	m_BoxColors[CSandBoxPlus::eHardenedPlus] = qRgb(238,35,4);
	m_BoxColors[CSandBoxPlus::eHardened] = qRgb(247,125,2);
	m_BoxColors[CSandBoxPlus::eDefaultPlus] = qRgb(1,133,248);
	m_BoxColors[CSandBoxPlus::eDefault] = qRgb(246,246,2);
	m_BoxColors[CSandBoxPlus::eAppBoxPlus] = qRgb(3,232,232);
	m_BoxColors[CSandBoxPlus::eAppBox] = qRgb(0,253,0);
	m_BoxColors[CSandBoxPlus::eInsecure] = qRgb(244,3,244);
	m_BoxColors[CSandBoxPlus::eOpen] = qRgb(255,255,255);

	CreateTrayIcon();

	LoadState();

	m_pProgressDialog = new CProgressDialog("");
	m_pProgressDialog->setWindowModality(Qt::ApplicationModal);
	connect(m_pProgressDialog, SIGNAL(Cancel()), this, SLOT(OnCancelAsync()));
	m_pProgressModal = false;

	m_pPopUpWindow = new CPopUpWindow();

	bool bAlwaysOnTop = theConf->GetBool("Options/AlwaysOnTop", false);
	this->setWindowFlag(Qt::WindowStaysOnTopHint, bAlwaysOnTop);
	m_pPopUpWindow->setWindowFlag(Qt::WindowStaysOnTopHint, bAlwaysOnTop);
	m_pProgressDialog->setWindowFlag(Qt::WindowStaysOnTopHint, bAlwaysOnTop);

	//connect(theAPI, SIGNAL(LogMessage(const QString&, bool)), this, SLOT(OnLogMessage(const QString&, bool)));
	connect(theAPI, SIGNAL(LogSbieMessage(quint32, const QStringList&, quint32)), this, SLOT(OnLogSbieMessage(quint32, const QStringList&, quint32)));
	connect(theAPI, SIGNAL(NotAuthorized(bool, bool&)), this, SLOT(OnNotAuthorized(bool, bool&)), Qt::DirectConnection);
	connect(theAPI, SIGNAL(QueuedRequest(quint32, quint32, quint32, const QVariantMap&)), this, SLOT(OnQueuedRequest(quint32, quint32, quint32, const QVariantMap&)), Qt::QueuedConnection);
	connect(theAPI, SIGNAL(FileToRecover(const QString&, const QString&, const QString&, quint32)), this, SLOT(OnFileToRecover(const QString&, const QString&, const QString&, quint32)), Qt::QueuedConnection);
	connect(theAPI, SIGNAL(ConfigReloaded()), this, SLOT(OnIniReloaded()));

    connect(qApp, &QGuiApplication::commitDataRequest, this, &CSandMan::commitData);

	m_uTimerID = startTimer(1000);

	bool bAutoRun = QApplication::arguments().contains("-autorun");
	if (!bAutoRun && g_PendingMessage.isEmpty())
		SafeShow(this);

	OnStatusChanged();
	if (CSbieUtils::IsRunning(CSbieUtils::eAll) || theConf->GetBool("Options/StartIfStopped", true))
	{
		SB_RESULT(void*) Status = ConnectSbie();
		HandleMaintenance(Status);
	}

	//qApp->setWindowIcon(GetIcon("IconEmptyDC", false));
}

CSandMan::~CSandMan()
{
	m_pPopUpWindow->close();
	delete m_pPopUpWindow;

	if(theAPI->IsMonitoring())
		theAPI->EnableMonitor(false);

	delete m_pBoxBorder;

	killTimer(m_uTimerID);

	m_pTrayIcon->show();
	m_pTrayIcon->hide();

	StoreState();

	theAPI = NULL;

	theGUI = NULL;
}

void CSandMan::LoadState(bool bFull)
{
	if (bFull) {
		setWindowState(Qt::WindowNoState);
		restoreGeometry(theConf->GetBlob("MainWindow/Window_Geometry"));
		restoreState(theConf->GetBlob("MainWindow/Window_State"));
	}

	//m_pBoxTree->restoreState(theConf->GetBlob("MainWindow/BoxTree_Columns"));
	if(m_pMessageLog) m_pMessageLog->GetView()->header()->restoreState(theConf->GetBlob("MainWindow/LogList_Columns"));
	if(m_pRecoveryLog) m_pRecoveryLog->GetView()->header()->restoreState(theConf->GetBlob("MainWindow/RecoveryLog_Columns"));
	if(m_pLogSplitter) m_pLogSplitter->restoreState(theConf->GetBlob("MainWindow/Log_Splitter"));
	if(m_pPanelSplitter) m_pPanelSplitter->restoreState(theConf->GetBlob("MainWindow/Panel_Splitter"));
	if(m_pLogTabs) m_pLogTabs->setCurrentIndex(theConf->GetInt("MainWindow/LogTab", 0));
}

void CSandMan::StoreState()
{
	theConf->SetBlob("MainWindow/Window_Geometry", saveGeometry());
	theConf->SetBlob("MainWindow/Window_State", saveState());

	//theConf->SetBlob("MainWindow/BoxTree_Columns", m_pBoxTree->saveState());
	if(m_pMessageLog) theConf->SetBlob("MainWindow/LogList_Columns", m_pMessageLog->GetView()->header()->saveState());
	if(m_pRecoveryLog) theConf->SetBlob("MainWindow/RecoveryLog_Columns", m_pRecoveryLog->GetView()->header()->saveState());
	if(m_pLogSplitter) theConf->SetBlob("MainWindow/Log_Splitter", m_pLogSplitter->saveState());
	if(m_pPanelSplitter) theConf->SetBlob("MainWindow/Panel_Splitter", m_pPanelSplitter->saveState());
	if(m_pLogTabs) theConf->SetValue("MainWindow/LogTab", m_pLogTabs->currentIndex());
}

QIcon CSandMan::GetIcon(const QString& Name, int iAction)
{
	int iNoIcons = theConf->GetInt("Options/NoIcons", 2);
	if (iNoIcons == 2)
		iNoIcons = theConf->GetInt("Options/ViewMode", 1) == 2 ? 1 : 0;
	if(iAction && iNoIcons)
		return QIcon();

	QString Path = QApplication::applicationDirPath() + "/Icons/" + Name + ".png";
	if(QFile::exists(Path))
		return QIcon(Path);
	return QIcon((iAction == 1 ? ":/Actions/" : ":/") + Name + ".png");
}

void CSandMan::CreateUI()
{
	SetUITheme();

	// Clear old ToolBar references.
	m_pNewBoxButton = nullptr;
	m_pCleanUpButton = nullptr;
	m_pEditIniButton = nullptr;	
	
	int iViewMode = theConf->GetInt("Options/ViewMode", 1);

	if(iViewMode == 2)
		CreateOldMenus();
	else
		CreateMenus(iViewMode == 1);

	m_pMainLayout = new QVBoxLayout(m_pMainWidget);
	m_pMainLayout->setContentsMargins(2,2,2,2);
	m_pMainLayout->setSpacing(0);

	if(iViewMode == 1)
		CreateToolBar(false);
	else {
		m_pSeparator = NULL;
		CreateLabel();
		m_pMenuLayout->addWidget(m_pLabel);
		UpdateLabel();
	}

	CreateView(iViewMode);

	if (iViewMode != 2) {
		m_pMenuBrowse->setChecked(theConf->GetBool("Options/ShowFilePanel", false));
		m_pFileView->setVisible(m_pMenuBrowse->isChecked());
	}

	foreach(QAction * pAction, m_pViewMode->actions())
		pAction->setChecked(pAction->data().toInt() == iViewMode);

	statusBar()->setVisible(iViewMode == 1);

	if(m_pKeepTerminated) m_pKeepTerminated->setChecked(theConf->GetBool("Options/KeepTerminated"));
	if(m_pShowAllSessions) m_pShowAllSessions->setChecked(theConf->GetBool("Options/ShowAllSessions"));

	m_pWndTopMost->setChecked(theConf->GetBool("Options/AlwaysOnTop", false));


	// pizza background
	int iUsePizza = theConf->GetInt("Options/UseBackground", 2);
	if (iUsePizza == 2)
		iUsePizza = theConf->GetInt("Options/ViewMode", 1) == 2 ? 1 : 0;
	if (iUsePizza) 
	{
		QPalette pizzaPalete = GetBoxView()->GetTree()->palette(); // QPalette pizzaPalete = QApplication::palette();
		SetPaleteTexture(pizzaPalete, QPalette::Base, QImage(":/Assets/background.png"));
		GetBoxView()->GetTree()->setPalette(pizzaPalete); // QApplication::setPalette(pizzaPalete);
		GetFileView()->GetTree()->setPalette(pizzaPalete); // QApplication::setPalette(pizzaPalete);
	}
}

void CSandMan::CreateMaintenanceMenu()
{
		m_pMaintenance = m_pMenuFile->addMenu(CSandMan::GetIcon("Maintenance"), tr("&Maintenance"));
			m_pConnect = m_pMaintenance->addAction(CSandMan::GetIcon("Connect"), tr("Connect"), this, SLOT(OnMaintenance()));
			m_pDisconnect = m_pMaintenance->addAction(CSandMan::GetIcon("Disconnect"), tr("Disconnect"), this, SLOT(OnMaintenance()));
			m_pMaintenance->addSeparator();
			m_pStopAll = m_pMaintenance->addAction(CSandMan::GetIcon("Stop"), tr("Stop All"), this, SLOT(OnMaintenance()));
			m_pMaintenanceItems = m_pMaintenance->addMenu(CSandMan::GetIcon("ManMaintenance"), tr("&Advanced"));
				m_pInstallDrv = m_pMaintenanceItems->addAction(tr("Install Driver"), this, SLOT(OnMaintenance()));
				m_pStartDrv = m_pMaintenanceItems->addAction(tr("Start Driver"), this, SLOT(OnMaintenance()));
				m_pStopDrv = m_pMaintenanceItems->addAction(tr("Stop Driver"), this, SLOT(OnMaintenance()));
				m_pUninstallDrv = m_pMaintenanceItems->addAction(tr("Uninstall Driver"), this, SLOT(OnMaintenance()));
				m_pMaintenanceItems->addSeparator();
				m_pInstallSvc = m_pMaintenanceItems->addAction(tr("Install Service"), this, SLOT(OnMaintenance()));
				m_pStartSvc = m_pMaintenanceItems->addAction(tr("Start Service"), this, SLOT(OnMaintenance()));
				m_pStopSvc = m_pMaintenanceItems->addAction(tr("Stop Service"), this, SLOT(OnMaintenance()));
				m_pUninstallSvc = m_pMaintenanceItems->addAction(tr("Uninstall Service"), this, SLOT(OnMaintenance()));
			m_pMaintenance->addSeparator();
			m_pSetupWizard = m_pMaintenance->addAction(CSandMan::GetIcon("Wizard"), tr("Setup Wizard"), this, SLOT(OnMaintenance()));
			//m_pUpdateCore = m_pMaintenance->addAction(CSandMan::GetIcon("Install"), tr("Update Core Files"), this, SLOT(OnMaintenance()));
			if(theGUI->IsFullyPortable())
				m_pUninstallAll = m_pMaintenance->addAction(CSandMan::GetIcon("Uninstall"), tr("Uninstall All"), this, SLOT(OnMaintenance()));
}

void CSandMan::CreateViewBaseMenu()
{
		m_pViewMode = new QActionGroup(m_pMenuView);
		MakeAction(m_pViewMode, m_pMenuView, tr("Simple View"), 0);
		MakeAction(m_pViewMode, m_pMenuView, tr("Advanced View"), 1);
		MakeAction(m_pViewMode, m_pMenuView, tr("Vintage View (like SbieCtrl)"), 2);
		connect(m_pViewMode, SIGNAL(triggered(QAction*)), this, SLOT(OnViewMode(QAction*)));

		m_pMenuView->addSeparator();
		m_pWndTopMost = m_pMenuView->addAction(tr("Always on Top"), this, SLOT(OnAlwaysTop()));
		m_pWndTopMost->setCheckable(true);
}

void CSandMan::CreateHelpMenu(bool bAdvanced)
{
	m_pMenuHelp = m_pMenuBar->addMenu(tr("&Help"));
		//m_pMenuHelp->addAction(tr("Support Sandboxie-Plus on Patreon"), this, SLOT(OnHelp()));
		//m_pSupport = m_pMenuHelp->addAction(tr("Support Sandboxie-Plus with Donations"), this, SLOT(OnHelp()));
		//if (!bAdvanced) {
		//	m_pMenuHelp->removeAction(m_pSupport);
		//	m_pMenuBar->addAction(m_pSupport);
		//}
		m_pContribution = m_pMenuHelp->addAction(CSandMan::GetIcon("Support"), tr("Contribute to Sandboxie-Plus"), this, SLOT(OnHelp()));
		m_pManual = m_pMenuHelp->addAction(tr("Online Documentation"), this, SLOT(OnHelp()));
		m_pForum = m_pMenuHelp->addAction(tr("Visit Support Forum"), this, SLOT(OnHelp()));
		m_pMenuHelp->addSeparator();
		m_pUpdate = m_pMenuHelp->addAction(CSandMan::GetIcon("Update"), tr("Check for Updates"), this, SLOT(CheckForUpdates()));
		m_pMenuHelp->addSeparator();
		m_pAboutQt = m_pMenuHelp->addAction(tr("About the Qt Framework"), this, SLOT(OnAbout()));
		m_pAbout = m_pMenuHelp->addAction(CSandMan::GetIcon("IconFull", 2), tr("About Sandboxie-Plus"), this, SLOT(OnAbout()));
}

void CSandMan::CreateMenus(bool bAdvanced)
{
	m_pMenuBar->clear();

	m_pMenuFile = m_pMenuBar->addMenu(tr("&Sandbox"));
		m_pNewBox = m_pMenuFile->addAction(CSandMan::GetIcon("NewBox"), tr("Create New Box"), this, SLOT(OnSandBoxAction()));
		m_pNewGroup = m_pMenuFile->addAction(CSandMan::GetIcon("Group"), tr("Create Box Group"), this, SLOT(OnSandBoxAction()));
		m_pImportBox = m_pMenuFile->addAction(CSandMan::GetIcon("UnPackBox"), tr("Import Box"), this, SLOT(OnSandBoxAction()));
		m_pMenuFile->addSeparator();
		m_pRunBoxed = m_pMenuFile->addAction(CSandMan::GetIcon("Run"), tr("Run Sandboxed"), this, SLOT(OnSandBoxAction()));
		m_pEmptyAll = m_pMenuFile->addAction(CSandMan::GetIcon("EmptyAll"), tr("Terminate All Processes"), this, SLOT(OnEmptyAll()));
		m_pMenuFile->addSeparator();
		m_pWndFinder = m_pMenuFile->addAction(CSandMan::GetIcon("finder"), tr("Is Window Sandboxed?"), this, SLOT(OnWndFinder()));

	if(bAdvanced || theGUI->IsFullyPortable())
		CreateMaintenanceMenu();
	else {
		m_pMaintenance = NULL;
			m_pConnect = NULL;
			m_pDisconnect = NULL;
			m_pMaintenanceItems = NULL;
			m_pInstallDrv = NULL;
			m_pStartDrv = NULL;
			m_pStopDrv = NULL;
			m_pUninstallDrv = NULL;
			m_pInstallSvc = NULL;
			m_pStartSvc = NULL;
			m_pStopSvc = NULL;
			m_pUninstallSvc = NULL;
			m_pStopAll = NULL;
			m_pUninstallAll = NULL;
			m_pSetupWizard = NULL;
			//m_pUpdateCore = NULL;
	}
				
		m_pMenuFile->addSeparator();
		m_pExit = m_pMenuFile->addAction(CSandMan::GetIcon("Exit"), tr("Exit"), this, SLOT(OnExit()));


	m_pMenuView = m_pMenuBar->addMenu(tr("&View"));

		CreateViewBaseMenu();

	if(bAdvanced) {
		m_pMenuView->addSeparator();

		m_pShowHidden = m_pMenuView->addAction(tr("Show Hidden Boxes"));
		m_pShowHidden->setCheckable(true);
		m_pShowAllSessions = m_pMenuView->addAction(tr("Show All Sessions"), this, SLOT(OnProcView()));
		m_pShowAllSessions->setCheckable(true);
	}
	else {
		m_pShowHidden = NULL;
		m_pShowAllSessions = NULL;
	}

		m_pMenuView->addSeparator();
		m_pMenuBrowse = m_pMenuView->addAction(CSandMan::GetIcon("Explore"), tr("Show File Panel"), this, SLOT(OnProcView()));
		m_pMenuBrowse->setCheckable(true);
		m_pMenuBrowse->setShortcut(QKeySequence("Ctrl+D"));
		m_pMenuBrowse->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		this->addAction(m_pMenuBrowse);

		m_pMenuView->addSeparator();

		m_pRefreshAll = m_pMenuView->addAction(CSandMan::GetIcon("Refresh"), tr("Refresh View"), this, SLOT(OnRefresh()));
		m_pRefreshAll->setShortcut(QKeySequence("F5"));
		m_pRefreshAll->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		this->addAction(m_pRefreshAll);

	if(bAdvanced) {
		m_pCleanUpMenu = m_pMenuView->addMenu(CSandMan::GetIcon("Clean"), tr("Clean Up"));
			m_pCleanUpProcesses = m_pCleanUpMenu->addAction(tr("Cleanup Processes"), this, SLOT(OnCleanUp()));
			m_pCleanUpMenu->addSeparator();
			m_pCleanUpMsgLog = m_pCleanUpMenu->addAction(tr("Cleanup Message Log"), this, SLOT(OnCleanUp()));
			m_pCleanUpTrace = m_pCleanUpMenu->addAction(tr("Cleanup Trace Log"), this, SLOT(OnCleanUp()));
			m_pCleanUpRecovery = m_pCleanUpMenu->addAction(tr("Cleanup Recovery Log"), this, SLOT(OnCleanUp()));

		m_pKeepTerminated = m_pMenuView->addAction(CSandMan::GetIcon("Keep"), tr("Keep terminated"), this, SLOT(OnProcView()));
		m_pKeepTerminated->setCheckable(true);
	}
	else {
		m_pCleanUpMenu = NULL;
			m_pCleanUpProcesses = NULL;
			m_pCleanUpMsgLog = NULL;
			m_pCleanUpTrace = NULL;
			m_pCleanUpRecovery = NULL;

		m_pKeepTerminated = NULL;
	}
		m_pMenuView->addSeparator();
		m_pEnableMonitoring = m_pMenuView->addAction(CSandMan::GetIcon("SetLogging"), tr("Trace Logging"), this, SLOT(OnMonitoring()));
	if (bAdvanced)
		m_pEnableMonitoring->setCheckable(true);
	if (!bAdvanced)
		m_pMenuView->addAction(CSandMan::GetIcon("Recover"), tr("Recovery Log"), this, SLOT(OnRecoveryLog()));
	

	m_pMenuOptions = m_pMenuBar->addMenu(tr("&Options"));
		m_pMenuSettings = m_pMenuOptions->addAction(CSandMan::GetIcon("Settings"), tr("Global Settings"), this, SLOT(OnSettings()));

		m_pMenuOptions->addSeparator();
		m_pDisableForce = m_pMenuOptions->addAction(CSandMan::GetIcon("PauseForce"), tr("Pause Forcing Programs"), this, SLOT(OnDisableForce()));
		m_pDisableForce->setCheckable(true);
		m_pDisableForce2 = new QAction(CSandMan::GetIcon("PauseForce"), tr("Pause Forcing Programs"));
		m_pDisableForce2->setCheckable(true);
		connect(m_pDisableForce2, SIGNAL(triggered()), this, SLOT(OnDisableForce2()));
	if(bAdvanced) {
		m_pDisableRecovery = m_pMenuOptions->addAction(GetIcon("DisableRecovery"), tr("Disable File Recovery"), this, SLOT(OnDisablePopUp()));
		m_pDisableRecovery->setCheckable(true);
		m_pDisableRecovery->setChecked(theConf->GetBool("UIConfig/DisabledRecovery", false));
		m_pDisableMessages = m_pMenuOptions->addAction(GetIcon("DisableMessagePopup"), tr("Disable Message Popup"), this, SLOT(OnDisablePopUp()));
		m_pDisableMessages->setCheckable(true);
		m_pDisableMessages->setChecked(theConf->GetBool("UIConfig/DisableMessages", false));
	}
	else {
		m_pDisableRecovery = NULL;
		m_pDisableMessages = NULL;
	}

		m_pMenuOptions->addSeparator();
		m_pEditIni = m_pMenuOptions->addAction(CSandMan::GetIcon("Editor"), tr("Edit Sandboxie.ini"), this, SLOT(OnEditIni()));
		m_pEditIni->setProperty("ini", "sbie");
		if (bAdvanced) {
			m_pEditIni2 = m_pMenuOptions->addAction(CSandMan::GetIcon("Editor2"), tr("Edit Templates.ini"), this, SLOT(OnEditIni()));
			m_pEditIni2->setProperty("ini", "tmpl");
			m_pEditIni3 = m_pMenuOptions->addAction(CSandMan::GetIcon("Editor4"), tr("Edit Sandboxie-Plus.ini"), this, SLOT(OnEditIni()));
			m_pEditIni3->setProperty("ini", "plus");
		} else
			m_pEditIni2 = m_pEditIni3 = NULL;
		m_pReloadIni = m_pMenuOptions->addAction(CSandMan::GetIcon("ReloadIni"), tr("Reload configuration"), this, SLOT(OnReloadIni()));

		m_pMenuOptions->addSeparator();
		m_pMenuResetMsgs = m_pMenuOptions->addAction(tr("Reset all hidden messages"), this, SLOT(OnResetMsgs()));
		m_pMenuResetGUI = m_pMenuOptions->addAction(tr("Reset all GUI options"), this, SLOT(OnResetGUI()));
		m_pMenuResetGUI->setShortcut(QKeySequence("Ctrl+Shift+R"));
		m_pMenuResetGUI->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		this->addAction(m_pMenuResetGUI);

	CreateHelpMenu(bAdvanced);

	// for old menu
	m_pSandbox = NULL;
}

void CSandMan::CreateOldMenus()
{
	m_pMenuBar->clear();

	m_pMenuFile = m_pMenuBar->addMenu(tr("&File"));
		m_pRunBoxed = m_pMenuFile->addAction(CSandMan::GetIcon("Run"), tr("Run Sandboxed"), this, SLOT(OnSandBoxAction()));
		m_pEmptyAll = m_pMenuFile->addAction(CSandMan::GetIcon("EmptyAll"), tr("Terminate All Processes"), this, SLOT(OnEmptyAll()));
		m_pDisableForce = m_pMenuFile->addAction(CSandMan::GetIcon("PauseForce"), tr("Pause Forcing Programs"), this, SLOT(OnDisableForce()));
		m_pDisableForce->setCheckable(true);
		m_pDisableForce2 = new QAction(CSandMan::GetIcon("PauseForce"), tr("Pause Forcing Programs"));
		m_pDisableForce2->setCheckable(true);
		connect(m_pDisableForce2, SIGNAL(triggered()), this, SLOT(OnDisableForce2()));
		//m_pDisableRecovery = m_pMenuFile->addAction(tr("Disable File Recovery"));
		//m_pDisableRecovery->setCheckable(true);
		m_pDisableRecovery = NULL;
		//m_pDisableMessages = m_pMenuFile->addAction(tr("Disable Message Popup"));
		//m_pDisableMessages->setCheckable(true);	
		m_pDisableMessages = NULL;
		m_pMenuFile->addSeparator();
		m_pWndFinder = m_pMenuFile->addAction(CSandMan::GetIcon("finder"), tr("Is Window Sandboxed?"), this, SLOT(OnWndFinder()));
		m_pEnableMonitoring = m_pMenuFile->addAction(CSandMan::GetIcon("SetLogging"), tr("Resource Access Monitor"), this, SLOT(OnMonitoring()));

		m_pMenuFile->addSeparator();

		if(theGUI->IsFullyPortable())
			CreateMaintenanceMenu();
		else {
			m_pMaintenance = NULL;
				m_pConnect = NULL;
				m_pDisconnect = NULL;
				m_pMaintenanceItems = NULL;
				m_pInstallDrv = NULL;
				m_pStartDrv = NULL;
				m_pStopDrv = NULL;
				m_pUninstallDrv = NULL;
				m_pInstallSvc = NULL;
				m_pStartSvc = NULL;
				m_pStopSvc = NULL;
				m_pUninstallSvc = NULL;
				m_pStopAll = NULL;
				m_pUninstallAll = NULL;
				m_pSetupWizard = NULL;
				//m_pUpdateCore = NULL;
		}
			
		m_pExit = m_pMenuFile->addAction(CSandMan::GetIcon("Exit"), tr("Exit"), this, SLOT(OnExit()));

	m_pMenuView = m_pMenuBar->addMenu(tr("&View"));

		CreateViewBaseMenu();

		m_pMenuView->addSeparator();
		QActionGroup* m_pView = new QActionGroup(m_pMenuView);
		MakeAction(m_pView, m_pMenuView, tr("Programs"), 0);
		MakeAction(m_pView, m_pMenuView, tr("Files and Folders"), 1);
		m_pView->actions().first()->setChecked(true);
		connect(m_pView, SIGNAL(triggered(QAction*)), this, SLOT(OnView(QAction*)));
		m_pMenuView->addSeparator();
		m_pMenuView->addAction(CSandMan::GetIcon("Recover"), tr("Recovery Log"), this, SLOT(OnRecoveryLog()));

		m_pMenuBrowse = NULL;
		//m_pMenuView->addSeparator();
		//m_pRefreshAll = m_pMenuView->addAction(CSandMan::GetIcon("Refresh"), tr("Refresh View"), this, SLOT(OnRefresh()));
		//m_pRefreshAll->setShortcut(QKeySequence("F5"));
		//m_pRefreshAll->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		//this->addAction(m_pRefreshAll);
		m_pRefreshAll = NULL;

		m_pCleanUpMenu = NULL;
			m_pCleanUpProcesses = NULL;
			m_pCleanUpMsgLog = NULL;
			m_pCleanUpTrace = NULL;
			m_pCleanUpRecovery = NULL;
		m_pKeepTerminated = NULL;

	m_pSandbox = m_pMenuBar->addMenu(tr("&Sandbox"));

		connect(m_pSandbox, SIGNAL(hovered(QAction*)), this, SLOT(OnBoxMenuHover(QAction*)));

		m_pSandbox->addSeparator();
		m_pNewBox = m_pSandbox->addAction(CSandMan::GetIcon("NewBox"), tr("Create New Sandbox"), this, SLOT(OnSandBoxAction()));
		m_pNewGroup = m_pSandbox->addAction(CSandMan::GetIcon("Group"), tr("Create New Group"), this, SLOT(OnSandBoxAction()));
		m_pImportBox = m_pSandbox->addAction(CSandMan::GetIcon("UnPackBox"), tr("Import Sandbox"), this, SLOT(OnSandBoxAction()));

		QAction* m_pSetContainer = m_pSandbox->addAction(CSandMan::GetIcon("Advanced"), tr("Set Container Folder"), this, SLOT(OnSettingsAction()));
		m_pSetContainer->setData(CSettingsWindow::eAdvanced);

		m_pArrangeGroups = m_pSandbox->addAction(tr("Set Layout and Groups"), this, SLOT(OnSettingsAction()));

		m_pShowHidden = m_pSandbox->addAction(tr("Reveal Hidden Boxes"));
		m_pShowHidden->setCheckable(true);
		//m_pShowAllSessions = m_pSandbox->addAction(tr("Show Boxes From All Sessions"), this, SLOT(OnProcView()));
		//m_pShowAllSessions->setCheckable(true);
		m_pShowAllSessions = NULL;

	m_pMenuOptions = m_pMenuBar->addMenu(tr("&Configure"));
		m_pMenuSettings = m_pMenuOptions->addAction(CSandMan::GetIcon("Settings"), tr("Global Settings"), this, SLOT(OnSettings()));
		m_pMenuOptions->addSeparator();

		QAction* m_pProgramAlert = m_pMenuOptions->addAction(CSandMan::GetIcon("Alarm"), tr("Program Alerts"), this, SLOT(OnSettingsAction()));
		m_pProgramAlert->setData(CSettingsWindow::eProgCtrl);
		QAction* m_pWindowsShell = m_pMenuOptions->addAction(CSandMan::GetIcon("Shell"), tr("Windows Shell Integration"), this, SLOT(OnSettingsAction()));
		m_pWindowsShell->setData(CSettingsWindow::eShell);
		QAction* m_pCompatibility = m_pMenuOptions->addAction(CSandMan::GetIcon("Compatibility"), tr("Software Compatibility"), this, SLOT(OnSettingsAction()));
		m_pCompatibility->setData(CSettingsWindow::eSoftCompat);

		m_pMenuResetMsgs = m_pMenuOptions->addAction(tr("Reset all hidden messages"), this, SLOT(OnResetMsgs()));
		m_pMenuResetGUI = m_pMenuOptions->addAction(tr("Reset all GUI options"), this, SLOT(OnResetGUI()));
		m_pMenuResetGUI->setShortcut(QKeySequence("Ctrl+Shift+R"));
		m_pMenuResetGUI->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		this->addAction(m_pMenuResetGUI);
		m_pMenuOptions->addSeparator();
		QAction* m_pConfigLock = m_pMenuOptions->addAction(CSandMan::GetIcon("Lock"), tr("Lock Configuration"), this, SLOT(OnSettingsAction()));
		m_pConfigLock->setData(CSettingsWindow::eConfigLock);
		m_pEditIni = m_pMenuOptions->addAction(CSandMan::GetIcon("Editor"), tr("Edit Sandboxie.ini"), this, SLOT(OnEditIni()));
		m_pEditIni->setProperty("ini", "sbie");
		m_pEditIni2 = m_pEditIni3 = NULL;
		m_pReloadIni = m_pMenuOptions->addAction(CSandMan::GetIcon("ReloadIni"), tr("Reload configuration"), this, SLOT(OnReloadIni()));

	CreateHelpMenu(false);

}

void CSandMan::OnView(QAction* pAction)
{
	int iView = pAction->data().toInt();
	if(m_pViewStack) m_pViewStack->setCurrentIndex(iView);

	if (iView == 1) { // files
		m_pBoxCombo->clear();
		foreach(const CSandBoxPtr & pBox, theAPI->GetAllBoxes())
			m_pBoxCombo->addItem(tr("Sandbox %1").arg(pBox->GetName().replace("_", "")), pBox->GetName());
		m_pBoxCombo->setCurrentIndex(m_pBoxCombo->findData(theAPI->GetGlobalSettings()->GetText("DefaultBox", "DefaultBox")));
	}
}

void CSandMan::SetToolBarItemsConfig(const QSet<QString>& items)
{
	QStringList list;
	for (auto item : items) list.append(item);
	theConf->SetValue(ToolBarConfigKey, list);
}

QSet<QString> CSandMan::GetToolBarItemsConfig()
{
	auto list = theConf->GetStringList(ToolBarConfigKey, DefaultToolBarItems);

	QSet<QString> validSet;

	for (auto item : GetAvailableToolBarActions()) {		
		if (!item.scriptName.isEmpty()) validSet.insert(item.scriptName);
	}

	// remove invalid and obsolete items
	QSet<QString> items;
	for (auto item : list) {
		auto trimmed = item.trimmed();
		if (validSet.contains(trimmed))
			items.insert(trimmed);
		else
			// m_pMessageLog exists, but UI does not
			// AddLogMessage(tr("Invalid toolbar item in sandboxie-plus.ini: %1").arg(item));
			;
	}

	return items;
}

QList<ToolBarAction> CSandMan::GetAvailableToolBarActions()
{
	// Assumes Advanced-Mode and (menu-)actions have been created.
	// Return items in toolbar display order

	return QList<ToolBarAction> {
			ToolBarAction{ "NewBoxMenu", nullptr, tr("New-Box Menu") },  //tr: Name of button in toolbar for showing actions new box, new group, import},
			ToolBarAction{ "NewBox", m_pNewBox },
			ToolBarAction{ "NewGroup", m_pNewGroup },
			ToolBarAction{ "ImportBox", m_pImportBox },
			ToolBarAction{ "", nullptr },        // separator
			ToolBarAction{ "RunBoxed", m_pRunBoxed },
			ToolBarAction{ "IsBoxed", m_pWndFinder },
			ToolBarAction{ "TerminateAll", m_pEmptyAll },
			ToolBarAction{ "", nullptr },        // separator
			ToolBarAction{ "CleanUpMenu", nullptr, tr("Cleanup") }, //tr: Name of button in toolbar for cleanup-all action
			ToolBarAction{ "KeepTerminated", m_pKeepTerminated },
			ToolBarAction{ "Refresh", m_pRefreshAll },
			ToolBarAction{ "", nullptr },        // separator
			ToolBarAction{ "BrowseFiles", m_pMenuBrowse },
			ToolBarAction{ "", nullptr },        // separator
			ToolBarAction{ "Settings", m_pMenuSettings },
			ToolBarAction{ "", nullptr },        // separator
			ToolBarAction{ "EditIniMenu", nullptr, tr("Edit-ini Menu") },  //tr: Name of button in toolbar for showing edit-ini files actions},
			ToolBarAction{ "EditIni", m_pEditIni },
			ToolBarAction{ "EditTemplates", m_pEditIni2 },
			ToolBarAction{ "EditPlusIni", m_pEditIni3 },
			ToolBarAction{ "ReloadIni", m_pReloadIni },
			ToolBarAction{ "", nullptr },        // separator
			ToolBarAction{ "EnableMonitor", m_pEnableMonitoring },
			ToolBarAction{ "", nullptr },        // separator
			ToolBarAction{ "DisableForce", m_pDisableForce2},
			ToolBarAction{ "DisableRecovery", m_pDisableRecovery },
			ToolBarAction{ "DisableMessages", m_pDisableMessages },
			ToolBarAction{ "", nullptr },        // separator
			ToolBarAction{ "Connect", m_pConnect },
			ToolBarAction{ "Disconnect", m_pDisconnect },
			ToolBarAction{ "StopAll", m_pStopAll },
			// ToolBarAction{"SetupWizard", m_pSetupWizard},
			// ToolBarAction{"UninstallAll", m_pUninstallAll}, // removed because not always valid in menu system
			ToolBarAction{ "", nullptr },        // separator
			ToolBarAction{ "CheckForUpdates", m_pUpdate },
			ToolBarAction{ "About", m_pAbout },
			ToolBarAction{ "", nullptr },        // separator
			ToolBarAction{ "Exit", m_pExit },
			ToolBarAction{ "", nullptr },        // separator
			ToolBarAction{ "Contribute", m_pContribution }
	};
}

void CSandMan::OnResetToolBarMenuConfig()
{
	theConf->SetValue(ToolBarConfigKey, DefaultToolBarItems);
	CreateToolBar(true);
}

void CSandMan::OnToolBarMenuItemClicked(const QString& scriptName)
{
	// Toggles content of config. Ignores menu item state. Menu is immediately rebuilt with toolbar update.
	auto items = GetToolBarItemsConfig();
	if (!items.remove(scriptName)) items.insert(scriptName);
	SetToolBarItemsConfig(items);
	CreateToolBar(true);
}

void CSandMan::CreateToolBarConfigMenu(const QList<ToolBarAction>& actions, const QSet<QString>& currentItems)
{
	auto m_pToolBarContextMenu = new QMenu(tr("Toolbar Items"), m_pToolBar);

	m_pToolBarContextMenu->addAction(tr("Reset Toolbar"), this, &CSandMan::OnResetToolBarMenuConfig);
	m_pToolBarContextMenu->addSeparator();

	for (auto sa : actions)
	{
		if (sa.scriptName == nullptr) {
			m_pToolBarContextMenu->addSeparator();
			continue;
		}

		QString text = sa.scriptName;
		if (!sa.nameOverride.isEmpty())
			text = sa.nameOverride;
		else if (sa.action)
			text = sa.action->text();  // tr: already localised
		else
			qDebug() << "ERROR: Missing display name for " << sa.scriptName;

		auto scriptName = sa.scriptName;
		auto menuAction = m_pToolBarContextMenu->addAction(text, this, [scriptName, this]() {
			OnToolBarMenuItemClicked(scriptName);
			}
		);
		menuAction->setCheckable(true);
		menuAction->setChecked(currentItems.contains(sa.scriptName));
	}

	m_pToolBar->setContextMenuPolicy(Qt::CustomContextMenu);	
	QObject::connect(m_pToolBar, &QToolBar::customContextMenuRequested, this,
		[m_pToolBarContextMenu, this](const QPoint& p) {
			m_pToolBarContextMenu->exec(mapToGlobal(p));
		}
	);
}

void CSandMan::CreateToolBar(bool rebuild)
{
	// Assumes UI is in Advanced-Mode and menus have been built.
	
	auto pOldToolBar = m_pToolBar;
	m_pToolBar = new QToolBar();
	m_pMainLayout->insertWidget(0, m_pToolBar);
	if (rebuild) {
		m_pLabel->deleteLater(); // should really be owned by m_pToolBar, not m_pMainWidget
		m_pMainLayout->removeWidget(pOldToolBar);
		pOldToolBar->deleteLater();
		m_pNewBoxButton = nullptr;  // deleted by pOldToolBar
		m_pEditIniButton = nullptr;
		m_pCleanUpButton = nullptr;
	}

	auto items = GetToolBarItemsConfig();
	auto scriptableActions = GetAvailableToolBarActions();
	CreateToolBarConfigMenu(scriptableActions, items);

	// Prevent leading, trailing, or consecutive separators
	bool needsSeparator = false; // true if we need to add a separator before the next action
	bool latestIsAction = false; // true if the most recent toolbar item is not a separator

	for (auto sa : scriptableActions)
	{
		if (sa.scriptName.isEmpty()) {
			// only trigger if we just added an action
			if (latestIsAction) needsSeparator = true;
			continue;
		}

		if (!items.contains(sa.scriptName)) continue;

		if (needsSeparator) {
			m_pToolBar->addSeparator();
			needsSeparator = false;
		}

		latestIsAction = true;

		if (sa.action)
		{
			m_pToolBar->addAction(sa.action);
		}
		else if (sa.scriptName == "CleanUpMenu")
		{
			auto but = new QToolButton();
			but->setIcon(CSandMan::GetIcon("Clean"));
			but->setToolTip(tr("Cleanup"));
			but->setText(tr("Cleanup"));
			but->setPopupMode(QToolButton::MenuButtonPopup);
			but->setMenu(m_pCleanUpMenu);		
			QObject::connect(but, SIGNAL(clicked(bool)), this, SLOT(OnCleanUp()));
			m_pCleanUpButton = but;
			m_pToolBar->addWidget(but);
		}
		else if (sa.scriptName == "NewBoxMenu")
		{
			auto but = new QToolButton();
			but->setIcon(CSandMan::GetIcon("NewBox"));
			but->setToolTip(tr("Create New Box"));
			but->setText(tr("Create New Box"));
			but->setPopupMode(QToolButton::MenuButtonPopup);
			auto menu = new QMenu(but);
			menu->addAction(m_pNewBox);
			menu->addAction(m_pNewGroup);
			menu->addAction(m_pImportBox);
			but->setMenu(menu);
			QObject::connect(but, &QToolButton::clicked, this, [this]() {GetBoxView()->AddNewBox();});
			m_pNewBoxButton = but;
			m_pToolBar->addWidget(but);
		}
		else if (sa.scriptName == "EditIniMenu")
		{
			auto but = new QToolButton();
			but->setIcon(CSandMan::GetIcon("Editor"));
			but->setToolTip(tr("Edit Sandboxie.ini"));
			but->setText(tr("Edit Sandboxie.ini"));
			but->setPopupMode(QToolButton::MenuButtonPopup);
			auto menu = new QMenu(but);
			menu->addAction(m_pEditIni);
			menu->addAction(m_pEditIni2);
			menu->addAction(m_pEditIni3);
			but->setMenu(menu);
			QObject::connect(but, &QToolButton::clicked, this, [this]() {OnEditIni();});
			m_pEditIniButton = but;
			m_pToolBar->addWidget(but);
		}
		else
		{
			qDebug() << "ERROR: You forgot to handle ToolBarAction scriptName " << sa.scriptName;
		}
	}

	// Contribute-Label

	QWidget* pSpacer = new QWidget();
	pSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_pToolBar->addWidget(pSpacer);

	//m_pToolBar->addAction(m_pMenuElevate);

	m_pSeparator = m_pToolBar->addSeparator();
	CreateLabel();
	m_pToolBar->addWidget(m_pLabel);
	UpdateLabel();
}

void CSandMan::CreateLabel()
{
	m_pLabel = new QLabel(m_pMainWidget);
	m_pLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	connect(m_pLabel, SIGNAL(linkActivated(const QString&)), this, SLOT(OpenUrl(const QString&)));

	m_pLabel->setAlignment(Qt::AlignCenter);
	m_pLabel->setContentsMargins(24, 0, 24, 0);

	QFont fnt = m_pLabel->font();
	fnt.setBold(true);
	//fnt.setWeight(QFont::DemiBold);
	m_pLabel->setFont(fnt);
}

void CSandMan::UpdateLabel()
{
	QString LabelText;
	QString LabelTip;

	if (!theConf->GetString("Updater/PendingUpdate").isEmpty())
	{
		QString FilePath = theConf->GetString("Updater/InstallerPath");
		if (!FilePath.isEmpty() && QFile::exists(FilePath)) {
			LabelText = tr("<a href=\"sbie://update/installer\" style=\"color: red;\">There is a new Sandboxie-Plus release %1 ready</a>").arg(theConf->GetString("Updater/InstallerVersion"));
			LabelTip = tr("Click to run installer");
		}
		else if (!theConf->GetString("Updater/UpdateVersion").isEmpty()){
			LabelText = tr("<a href=\"sbie://update/apply\" style=\"color: red;\">There is a new Sandboxie-Plus update %1 ready</a>").arg(theConf->GetString("Updater/UpdateVersion"));
			LabelTip = tr("Click to apply update");
		}
		else {
			LabelText = tr("<a href=\"sbie://update/check\" style=\"color: red;\">There is a new Sandboxie-Plus update v%1 available</a>").arg(theConf->GetString("Updater/PendingUpdate"));
			LabelTip = tr("Click to download update");
		}

		//auto neon = new CNeonEffect(10, 4, 180); // 140
		//m_pLabel->setGraphicsEffect(NULL);
	}
	else if (g_Certificate.isEmpty()) 
	{
		LabelText = tr("<a href=\"https://sandboxie-plus.com/go.php?to=patreon\">Support Sandboxie-Plus on Patreon</a>");
		LabelTip = tr("Click to open web browser");

		//auto neon = new CNeonEffect(10, 4, 240);
		auto neon = new CNeonEffect(10, 4);
		//neon->setGlowColor(Qt::green);
		neon->setHue(240);
		/*if(m_DarkTheme)
			neon->setColor(QColor(218, 130, 42));
		else
			neon->setColor(Qt::blue);*/
		m_pLabel->setGraphicsEffect(neon);

		/*auto glowAni = new QVariantAnimation(neon);
		glowAni->setDuration(10000);
		glowAni->setLoopCount(-1);
		glowAni->setStartValue(0);
		glowAni->setEndValue(360);
		glowAni->setEasingCurve(QEasingCurve::InQuad);
			connect(glowAni, &QVariantAnimation::valueChanged, [neon](const QVariant &value) {
				neon->setHue(value.toInt());
				qDebug() << value.toInt();
		});
		glowAni->start();*/

		/*auto glowAni = new QVariantAnimation(neon);
		glowAni->setDuration(3000);
		glowAni->setLoopCount(-1);
		glowAni->setStartValue(5);
		glowAni->setEndValue(20);
		glowAni->setEasingCurve(QEasingCurve::InQuad);
			connect(glowAni, &QVariantAnimation::valueChanged, [neon](const QVariant &value) {
				neon->setBlurRadius(value.toInt());
				qDebug() << value.toInt();
		});
		glowAni->start();*/

		/*auto glowAni = new QVariantAnimation(neon);
		glowAni->setDuration(3000);
		glowAni->setLoopCount(-1);
		glowAni->setStartValue(1);
		glowAni->setEndValue(20);
		glowAni->setEasingCurve(QEasingCurve::InQuad);
			connect(glowAni, &QVariantAnimation::valueChanged, [neon](const QVariant &value) {
				neon->setGlow(value.toInt());
				qDebug() << value.toInt();
		});
		glowAni->start();*/

		/*auto glowAni = new QVariantAnimation(neon);
		glowAni->setDuration(3000);
		glowAni->setLoopCount(-1);
		glowAni->setStartValue(5);
		glowAni->setEndValue(25);
		glowAni->setEasingCurve(QEasingCurve::InQuad);
			connect(glowAni, &QVariantAnimation::valueChanged, [neon](const QVariant &value) {
				int iValue = value.toInt();
				if (iValue >= 15)
					iValue = 30 - iValue;
				neon->setGlow(iValue);
				neon->setBlurRadius(iValue);
		});
		glowAni->start();*/

	}

	if(m_pSeparator) m_pSeparator->setVisible(!LabelText.isEmpty());
	m_pLabel->setVisible(!LabelText.isEmpty());
	m_pLabel->setText(LabelText);
	m_pLabel->setToolTip(LabelTip);
}

void CSandMan::CreateView(int iViewMode)
{
	m_pBoxView = new CSbieView();
	connect(m_pBoxView, SIGNAL(BoxSelected()), this, SLOT(OnBoxSelected()));
	m_pFileView = new CFileView();

	if (iViewMode != 1) {
		m_pRecoveryLogWnd = new CRecoveryLogWnd(m_pMainWidget);
		connect(this, SIGNAL(Closed()), m_pRecoveryLogWnd, SLOT(close()));
	}
	else
		m_pRecoveryLogWnd = NULL;

	if (iViewMode == 2) 
	{
		m_pViewStack = new QStackedLayout();
		m_pViewStack->addWidget(m_pBoxView);

		QWidget* pFileView = new QWidget();
		QGridLayout* pFileLayout = new QGridLayout(pFileView);
		pFileLayout->setContentsMargins(0,0,0,0);

		pFileLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 0, 1, 1);

		m_pBoxCombo = new QComboBox();
		connect(m_pBoxCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(OnBoxSelected()));
		pFileLayout->addWidget(m_pBoxCombo, 0, 1);

		pFileLayout->addWidget(m_pFileView, 1, 0, 1, 2);

		m_pViewStack->addWidget(pFileView);


		m_pMainLayout->addLayout(m_pViewStack);

		m_pPanelSplitter = NULL;
		m_pLogSplitter = NULL;

		m_pLogTabs = NULL;

		m_pMessageLog = NULL;
		m_pTraceView = NULL;
		m_pRecoveryLog = NULL;

		return;
	}

	m_pViewStack = NULL;
	m_pBoxCombo = NULL;

	if (iViewMode == 1) 
	{
		m_pLogSplitter = new QSplitter();
		m_pLogSplitter->setOrientation(Qt::Vertical);
		m_pMainLayout->addWidget(m_pLogSplitter);
	}

	m_pPanelSplitter = new QSplitter();
	m_pPanelSplitter->setOrientation(Qt::Horizontal);
	if (iViewMode == 1)
		m_pLogSplitter->addWidget(m_pPanelSplitter);
	else 
		m_pMainLayout->addWidget(m_pPanelSplitter);

	m_pPanelSplitter->addWidget(m_pBoxView);
	m_pPanelSplitter->addWidget(m_pFileView);

	m_pPanelSplitter->setCollapsible(0, false);
	//m_pPanelSplitter->setCollapsible(1, false);

	if (iViewMode == 1)
	{
		m_pLogTabs = new QTabWidget();
		m_pLogSplitter->addWidget(m_pLogTabs);

		// Message Log
		m_pMessageLog = new CPanelWidgetEx();
		m_pMessageLog->GetTree()->setItemDelegate(new CTreeItemDelegate());

		m_pMessageLog->GetTree()->setAlternatingRowColors(theConf->GetBool("Options/AltRowColors", false));
		
		//m_pMessageLog->GetView()->setItemDelegate(theGUI->GetItemDelegate());
		((QTreeWidgetEx*)m_pMessageLog->GetView())->setHeaderLabels(tr("Time|Message").split("|"));
		((QTreeWidgetEx*)m_pMessageLog->GetView())->setColumnFixed(1, true);

		m_pMessageLog->GetMenu()->insertAction(m_pMessageLog->GetMenu()->actions()[0], m_pCleanUpMsgLog);
		m_pMessageLog->GetMenu()->insertSeparator(m_pMessageLog->GetMenu()->actions()[0]);

		m_pMessageLog->GetView()->setSelectionMode(QAbstractItemView::ExtendedSelection);
		m_pMessageLog->GetView()->setSortingEnabled(false);

		m_pLogTabs->addTab(m_pMessageLog, tr("Sbie Messages"));
		//

		m_pTraceView = new CTraceView(false, this);

		m_pTraceView->AddAction(m_pCleanUpTrace);

		m_pLogTabs->addTab(m_pTraceView, tr("Trace Log"));


		// Recovery Log
		m_pRecoveryLog = new CPanelWidgetEx();
		m_pRecoveryLog->GetTree()->setItemDelegate(new CTreeItemDelegate());

		m_pRecoveryLog->GetTree()->setAlternatingRowColors(theConf->GetBool("Options/AltRowColors", false));

		//m_pRecoveryLog->GetView()->setItemDelegate(theGUI->GetItemDelegate());
		((QTreeWidgetEx*)m_pRecoveryLog->GetView())->setHeaderLabels(tr("Time|Box Name|File Path").split("|"));
		((QTreeWidgetEx*)m_pRecoveryLog->GetView())->setColumnFixed(1, true);
		((QTreeWidgetEx*)m_pRecoveryLog->GetView())->setColumnFixed(2, true);

		m_pRecoveryLog->GetMenu()->insertAction(m_pRecoveryLog->GetMenu()->actions()[0], m_pCleanUpRecovery);
		m_pRecoveryLog->GetMenu()->insertSeparator(m_pRecoveryLog->GetMenu()->actions()[0]);

		m_pRecoveryLog->GetView()->setSelectionMode(QAbstractItemView::ExtendedSelection);
		m_pRecoveryLog->GetView()->setSortingEnabled(false);

		m_pLogTabs->addTab(m_pRecoveryLog, tr("Recovery Log"));
		//
	}
	else {
		m_pLogSplitter = NULL;

		m_pLogTabs = NULL;

		m_pMessageLog = NULL;
		m_pTraceView = NULL;
		m_pRecoveryLog = NULL;
	}
}

void CSandMan::CheckForUpdates(bool bManual)
{
	m_pUpdater->CheckForUpdates(bManual);
}

#include "SandManTray.cpp"

void CSandMan::OnExit()
{
	m_bExit = true;
	close();
}

void CSandMan::closeEvent(QCloseEvent *e)
{
	if (!m_bExit)// && !theAPI->IsConnected())
	{
		if (m_pTrayIcon->isVisible())
		{
			StoreState();
			hide();

			if (theAPI->GetGlobalSettings()->GetBool("ForgetPassword", false))
				theAPI->ClearPassword();

			e->ignore();
			return;
		}
		else
		{
			CExitDialog ExitDialog(tr("Do you want to close Sandboxie Manager?"));
			if (!ExitDialog.exec())
			{
				e->ignore();
				return;
			}
		}
	}

	emit Closed();

	if (IsFullyPortable() && theAPI->IsConnected())
	{
		int PortableStop = theConf->GetInt("Options/PortableStop", -1);
		if (PortableStop == -1)
		{
			bool State = false;
			auto Ret = CCheckableMessageBox::question(this, "Sandboxie-Plus", tr("Sandboxie-Plus was running in portable mode, now it has to clean up the created services. This will prompt for administrative privileges.\n\nDo you want to do the clean up?")
				, tr("Don't show this message again."), &State, QDialogButtonBox::Yes | QDialogButtonBox::No | QDialogButtonBox::Cancel, QDialogButtonBox::Yes, QMessageBox::Question);

			if (Ret == QDialogButtonBox::Cancel)
			{
				e->ignore();
				return;
			}

			PortableStop = (Ret == QDialogButtonBox::Yes) ? 1 : 0;

			if (State)
				theConf->SetValue("Options/PortableStop", PortableStop);
		}

		if (PortableStop == 1) {
			SB_RESULT(void*) Status = StopSbie(true);
			// don't care for Status we quit here anyway
		}
	}

	QApplication::quit();
}

void CSandMan::commitData(QSessionManager& manager)
{
    //if (manager.allowsInteraction()) 
	//{
    //	manager.cancel();
	//	return;
    //} 

	m_pBoxView->SaveState();
	m_pFileView->SaveState();
	StoreState();
	theConf->Sync();
}

QIcon CSandMan::GetBoxIcon(int boxType, bool inUse)// , int iBusy)
{
	//EBoxColors color = eYellow;
	int iViewMode = theConf->GetInt("Options/ViewMode", 1);
	if (iViewMode != 2) {
		return GetColorIcon(m_BoxColors[boxType], inUse);
		/*switch (boxType) {
		case CSandBoxPlus::eHardenedPlus:		color = eRed; break;
		case CSandBoxPlus::eHardened:			color = eOrang; break;
		case CSandBoxPlus::eDefaultPlus:		color = eBlue; break;
		case CSandBoxPlus::eDefault:			color = eYellow; break;
		case CSandBoxPlus::eAppBoxPlus:			color = eCyan; break;
		case CSandBoxPlus::eAppBox:				color = eGreen; break;
		case CSandBoxPlus::eInsecure:			color = eMagenta; break;
		case CSandBoxPlus::eOpen:				color = eWhite; break;
		}*/
	}
	//if (inBusy)
	//	return m_BoxIcons[color].Busy;
	/*if (inUse)
		return m_BoxIcons[color].InUse;
	return m_BoxIcons[color].Empty;*/
	return GetColorIcon(m_BoxColors[CSandBoxPlus::eDefault], inUse);
}

QIcon CSandMan::GetColorIcon(QColor boxColor, bool inUse/*, bool bOut*/)
{
	static QPixmap Sand;
	if(Sand.isNull())
		Sand = QPixmap(":/Boxes/Sand");

	static QPixmap Frame;
	if(Frame.isNull())
		Frame = QPixmap(":/Boxes/Frame");

	static QPixmap Items;
	if(Items.isNull())
		Items = QPixmap(":/Boxes/Items");

	static QPixmap Out;
	if(Out.isNull())
		Out = QPixmap(":/Boxes/Out");

	QRgb rgb = boxColor.rgba();
	QImage MySand = Sand.toImage();
	//if (!bOut) {
	for (QRgb* c = (QRgb*)MySand.bits(); c != (QRgb*)(MySand.bits() + MySand.sizeInBytes()); c++) {
		if (*c == 0xFFFFFFFF)
			*c = rgb;
	}
	//}
	
	QPixmap result(32, 32);
	result.fill(Qt::transparent); // force alpha channel
	QPainter painter(&result);
	//if (bOut) {
	//	QImage MyOut = Out.toImage();
	//	for (QRgb* c = (QRgb*)MyOut.bits(); c != (QRgb*)(MyOut.bits() + MyOut.sizeInBytes()); c++) {
	//		*c = rgb;
	//	}
	//	painter.drawPixmap(0, 0, QPixmap::fromImage(MyOut));
	//}
	painter.drawPixmap(0, 0, QPixmap::fromImage(MySand));
	painter.drawPixmap(0, 0, Frame);
	if (inUse) 
	{	
		//rgb = change_hsv_c(rgb, -60, 2, 1); // yellow -> red

		my_rgb rgb1 = { (double)qRed(rgb), (double)qGreen(rgb), (double)qBlue(rgb) };
		my_hsv hsv = rgb2hsv(rgb1);

		if((hsv.h >= 30 && hsv.h < 150) || (hsv.h >= 210 && hsv.h < 330))		hsv.h -= 60;
		else if(hsv.h >= 150 && hsv.h < 210)									hsv.h += 120;
		else if((hsv.h >= 330 && hsv.h < 360) || (hsv.h >= 0 && hsv.h < 30))	hsv.h -= 240;

		if (hsv.h < 0) hsv.h += 360;
		else if (hsv.h >= 360) hsv.h -= 360;
		hsv.s = 1; // make the content always fully saturated

		my_rgb rgb2 = hsv2rgb(hsv);
		rgb = qRgb(rgb2.r, rgb2.g, rgb2.b);

		QImage MyItems = Items.toImage();
		for (QRgb* c = (QRgb*)MyItems.bits(); c != (QRgb*)(MyItems.bits() + MyItems.sizeInBytes()); c++) {
			if (*c == 0xFF000000)
				*c = rgb;
		}
		painter.drawPixmap(0, 0, QPixmap::fromImage(MyItems));
	}

	return QIcon(result);
}

QIcon CSandMan::MakeIconBusy(const QIcon& Icon, int Index)
{
	static QPixmap overlay;
	if(overlay.isNull())
		overlay = QPixmap(":/Boxes/Busy");

	QPixmap base = Icon.pixmap(32, 32);
	QPixmap result(base.width(), base.height());
	result.fill(Qt::transparent); // force alpha channel
	QPainter painter(&result);
	painter.drawPixmap(0, 0, base);

	QTransform  rm;
	rm.rotate(90 * (Index % 4));
	painter.drawPixmap(8, 8, overlay.transformed(rm));
	return QIcon(result);
}

QIcon CSandMan::IconAddOverlay(const QIcon& Icon, const QString& Name, int Size)
{
	QPixmap overlay = QPixmap(Name).scaled(Size, Size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	QPixmap base = Icon.pixmap(32, 32).scaled(32, 32, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	QPixmap result(base.width(), base.height());
	result.fill(Qt::transparent); // force alpha channel
	QPainter painter(&result);
	painter.drawPixmap(0, 0, base);

	painter.drawPixmap(32 - Size, 32 - Size, overlay);
	return QIcon(result);
}

QString CSandMan::GetBoxDescription(int boxType)
{
	QString Info;

	switch (boxType) {
	case CSandBoxPlus::eHardenedPlus:
	case CSandBoxPlus::eHardened:
		Info = tr("This box provides <a href=\"sbie://docs/security-mode\">enhanced security isolation</a>, it is suitable to test untrusted software.");
		break;
	case CSandBoxPlus::eDefaultPlus:
	case CSandBoxPlus::eDefault:
		Info = tr("This box provides standard isolation, it is suitable to run your software to enhance security.");	
		break;
	case CSandBoxPlus::eAppBoxPlus:
	case CSandBoxPlus::eAppBox:
		Info = tr("This box does not enforce isolation, it is intended to be used as an <a href=\"sbie://docs/compartment-mode\">application compartment</a> for software virtualization only.");
		break;
	}
	
	if(boxType == CSandBoxPlus::eHardenedPlus || boxType == CSandBoxPlus::eDefaultPlus || boxType == CSandBoxPlus::eAppBoxPlus)
		Info.append(tr("<br /><br />This box <a href=\"sbie://docs/privacy-mode\">prevents access to all user data</a> locations, except explicitly granted in the Resource Access options."));

	return Info;
}

bool CSandMan::IsFullyPortable()
{
	QString SbiePath = theAPI->GetSbiePath();
	QString IniPath = theAPI->GetIniPath();
	if (IniPath.indexOf(SbiePath, 0, Qt::CaseInsensitive) == 0)
		return true;
	return false;
}

bool CSandMan::IsSilentMode()
{
	if (!theConf->GetBool("Options/CheckSilentMode", true))
		return false;
	return IsFullScreenMode();
}

QWidget* g_GUIParent = NULL;

int CSandMan::SafeExec(QDialog* pDialog)
{
	QWidget* pPrevParent = g_GUIParent;
	g_GUIParent = pDialog;
	SafeShow(pDialog);
	int ret = pDialog->exec();
	g_GUIParent = pPrevParent;
	return ret;
}

void CSandMan::OnMessage(const QString& MsgData)
{
	QStringList Messages = MsgData.split("\n");
	QString Message = Messages[0];
	if (Message == "ShowWnd")
	{
		if (!isVisible())
			show();
		setWindowState(Qt::WindowActive);
		SetForegroundWindow(MainWndHandle);
	}
	else if (Message.left(4) == "Run:")
	{
		QString BoxName;
		QString CmdLine = Message.mid(4);

		if (CmdLine.contains("\\start.exe", Qt::CaseInsensitive)) {
			int pos = CmdLine.indexOf("/box:", 0, Qt::CaseInsensitive);
			int pos2 = CmdLine.indexOf(" ", pos);
			if (pos != -1 && pos2 != -1) {
				BoxName = CmdLine.mid(pos + 5, pos2 - (pos + 5));
				CmdLine = CmdLine.mid(pos2 + 1);
			}
		}

		QString WrkDir;
		for (int i = 1; i < Messages.length(); i++) {
			if (Messages[i].left(5) == "From:") {
				WrkDir = Messages[i].mid(5);
				break;
			}
		}

		CSupportDialog::CheckSupport(true);

		if (theConf->GetBool("Options/RunInDefaultBox", false) && (QGuiApplication::queryKeyboardModifiers() & Qt::ControlModifier) == 0) {
			theAPI->RunStart(theAPI->GetGlobalSettings()->GetText("DefaultBox", "DefaultBox"), CmdLine, false, WrkDir);
		}
		else
			RunSandboxed(QStringList(CmdLine), BoxName, WrkDir);
	}
	else if (Message.left(3) == "Op:")
	{
		QString Op = Message.mid(3);

		SB_RESULT(void*) Status;
		if (Op == "Connect")
			Status = ConnectSbie();
		else if (Op == "Disconnect")
			Status = DisconnectSbie();
		else if (Op == "Shutdown")
			Status = StopSbie();
		else if (Op == "EmptyAll")
			Status = theAPI->TerminateAll();
		else
			Status = SB_ERR(SB_Message, QVariantList () << (tr("Unknown operation '%1' requested via command line").arg(Op)));

		HandleMaintenance(Status);
	}
}

void CSandMan::dragEnterEvent(QDragEnterEvent* e)
{
	if (e->mimeData()->hasUrls()) {
		e->acceptProposedAction();
	}
}

bool CSandMan::RunSandboxed(const QStringList& Commands, QString BoxName, const QString& WrkDir)
{
	if (BoxName.isEmpty())
		BoxName = theAPI->GetGlobalSettings()->GetText("DefaultBox", "DefaultBox");
	CSelectBoxWindow* pSelectBoxWindow = new CSelectBoxWindow(Commands, BoxName, WrkDir);
	connect(this, SIGNAL(Closed()), pSelectBoxWindow, SLOT(close()));
	//pSelectBoxWindow->show();
	return SafeExec(pSelectBoxWindow) == 1;
}

void CSandMan::dropEvent(QDropEvent* e)
{
	QStringList Commands;
	foreach(const QUrl & url, e->mimeData()->urls()) {
		if (url.isLocalFile())
			Commands.append(url.toLocalFile().replace("/", "\\"));
	}

	QString BoxName;
	QList<CSandBoxPtr> Boxes = m_pBoxView->GetSelectedBoxes();
	if (Boxes.count() == 1)
		BoxName = Boxes.first()->GetName();

	QTimer::singleShot(0, this, [Commands, BoxName, this]() { RunSandboxed(Commands, BoxName); });
}

void CSandMan::timerEvent(QTimerEvent* pEvent)
{
	if (pEvent->timerId() != m_uTimerID)
		return;

	bool bForceProcessDisabled = false;
	bool bIconBusy = false;
	bool bConnected = false;

	if (theAPI->IsConnected())
	{
		SB_STATUS Status = theAPI->ReloadBoxes();

		theAPI->UpdateProcesses(KeepTerminated(), ShowAllSessions());

		bForceProcessDisabled = theAPI->AreForceProcessDisabled();
		m_pDisableForce->setChecked(bForceProcessDisabled);
		m_pDisableForce2->setChecked(bForceProcessDisabled);

		if (m_pTraceView) 
		{
			bool bIsMonitoring = theAPI->IsMonitoring();
			m_pEnableMonitoring->setChecked(bIsMonitoring);
			int iTraceCount = theAPI->GetTraceCount();
			if (!bIsMonitoring && iTraceCount > 0)
				bIsMonitoring = true; // don't disable the view as long as there are entries shown
			if (bIsMonitoring && m_pTraceView)
				m_pTraceInfo->setText(QString::number(iTraceCount));
			m_pTraceView->SetEnabled(bIsMonitoring);
		}

		QMap<quint32, CBoxedProcessPtr> Processes = theAPI->GetAllProcesses();
		int ActiveProcesses = 0;
		if (KeepTerminated()) {
			foreach(const CBoxedProcessPtr & Process, Processes) {
				if (!Process->IsTerminated())
					ActiveProcesses++;
			}
		}
		else 
			ActiveProcesses = Processes.count();


		if (theAPI->IsBusy() || m_iDeletingContent > 0)
			bIconBusy = true;

		if (m_iIconDisabled != (bForceProcessDisabled ? 1 : 0)) {
			QString Str1 = tr("No Force Process");
			m_pDisabledForce->setText(m_pDisableForce->isChecked() ? Str1 : QString(Str1.length(), ' '));
		}

		bool bUpdatePending = !theConf->GetString("Updater/PendingUpdate").isEmpty();
		if (m_pDismissUpdate) {
			if (bUpdatePending) {
				if (m_pDismissUpdate->isChecked())
					bUpdatePending = false;
				else if (!m_pDismissUpdate->isVisible())
					m_pDismissUpdate->setVisible(true);
			}
			else if (m_pDismissUpdate->isChecked())
				m_pDismissUpdate->setChecked(false);
			if (!bUpdatePending && m_pDismissUpdate->isVisible())
				m_pDismissUpdate->setVisible(false);
		}

		if (m_bIconEmpty != (ActiveProcesses == 0) || m_bIconBusy != bIconBusy || m_iIconDisabled != (bForceProcessDisabled ? 1 : 0) || bUpdatePending || m_bIconSun)
		{
			m_bIconEmpty = (ActiveProcesses == 0);
			m_bIconBusy = bIconBusy;
			m_iIconDisabled = (bForceProcessDisabled ? 1 : 0);

			m_bIconSun = bUpdatePending ? !m_bIconSun : false;
			m_pTrayIcon->setIcon(GetTrayIcon(true, m_bIconSun));
			m_pTrayIcon->setToolTip(GetTrayText());
		}
	}

	if (!isVisible() || windowState().testFlag(Qt::WindowMinimized))
		return;

	//QUERY_USER_NOTIFICATION_STATE NState; // todo
	//if (SHQueryUserNotificationState(&NState) == S_OK)
	//	;

	theAPI->UpdateWindowMap();

	m_pBoxView->Refresh();

	if(!IsSilentMode()) // do not check for updates when in presentation/game mode
		m_pUpdater->Process();

	if (!m_MissingTemplates.isEmpty())
	{
		if (m_MissingTemplates[0] == "") {
			m_MissingTemplates.clear();
			return;
		}

		int CleanupTemplates = theConf->GetInt("Options/AutoCleanupTemplates", -1);
		if (CleanupTemplates == -1)
		{
			bool State = false;
			CleanupTemplates = CCheckableMessageBox::question(this, "Sandboxie-Plus", tr("Some compatibility templates (%1) are missing, probably deleted, do you want to remove them from all boxes?")
				.arg(m_MissingTemplates.join(", "))
				, tr("Don't show this message again."), &State, QDialogButtonBox::Yes | QDialogButtonBox::No, QDialogButtonBox::Yes, QMessageBox::Information) == QDialogButtonBox::Yes ? 1 : 0;

			if (State)
				theConf->SetValue("Options/AutoCleanupTemplates", CleanupTemplates);
		}

		if (CleanupTemplates)
		{
			foreach(const QString& Template, m_MissingTemplates)
			{
				theAPI->GetGlobalSettings()->DelValue("Template", Template);
				foreach(const CSandBoxPtr& pBox, theAPI->GetAllBoxes())
					pBox->DelValue("Template", Template);
			}

			OnLogMessage(tr("Cleaned up removed templates..."));
		}
		m_MissingTemplates.clear();

		m_MissingTemplates.append("");
	}
}

void CSandMan::OnBoxSelected()
{
	CSandBoxPtr pBox;
	if (m_pPanelSplitter) {
		QList<CSandBoxPtr> boxes = m_pBoxView->GetSelectedBoxes();
		if (m_pPanelSplitter->sizes().at(1) > 0 && m_pFileView->isVisible() && boxes.count() == 1)
			pBox = boxes.first();
	}

	// for vintage mode
	if (m_pBoxCombo && m_pViewStack->currentIndex() == 1) {
		QString Name = m_pBoxCombo->currentData().toString();
		if (Name.isEmpty())
			Name = theAPI->GetGlobalSettings()->GetText("DefaultBox", "DefaultBox");
		pBox = theAPI->GetBoxByName(Name);
	}

	if (!pBox.isNull()) {
		if (!m_pFileView->isEnabled()) m_pFileView->setEnabled(true);
		if (pBox != m_pFileView->GetBox()) m_pFileView->SetBox(pBox);
	}
	else if (m_pFileView->isEnabled()) {
		m_pFileView->setEnabled(false);
		m_pFileView->SetBox(CSandBoxPtr());
	}
}

SB_STATUS CSandMan::DeleteBoxContent(const CSandBoxPtr& pBox, EDelMode Mode, bool DeleteSnapshots)
{
	SB_STATUS Ret = SB_OK;
	m_iDeletingContent++;

	if (Mode != eAuto) {
		Ret = pBox->TerminateAll();
		theAPI->UpdateProcesses(KeepTerminated(), ShowAllSessions());
		if (Ret.IsError())
			goto finish;
	}

	if (Mode != eForDelete) {
		foreach(const QString & Value, pBox->GetTextList("OnBoxDelete", true, false, true)) {
			QString Value2 = pBox->Expand(Value);
			CSbieProgressPtr pProgress = CSbieUtils::RunCommand(Value2, true);
			if (!pProgress.isNull()) {
				AddAsyncOp(pProgress, true, tr("Executing OnBoxDelete: %1").arg(Value2));
				if (pProgress->IsCanceled()) {
					Ret = CSbieStatus(SB_Canceled);
					goto finish;
				}
			}
		}
	}
	
	{
		SB_PROGRESS Status;
		if (Mode != eForDelete && !DeleteSnapshots && pBox->HasSnapshots()) { // in auto delete mode always return to last snapshot
			QString Current;
			QString Default = pBox->GetDefaultSnapshot(&Current);
			Status = pBox->SelectSnapshot(Mode == eAuto ? Current : Default);
		}
		else // if there are no snapshots just use the normal cleaning procedure
			Status = pBox->CleanBox();

		Ret = Status;
		if (Status.GetStatus() == OP_ASYNC) {
			Ret = AddAsyncOp(Status.GetValue(), true, tr("Auto Deleting %1 Content").arg(pBox->GetName()));
			OnBoxCleaned(qobject_cast<CSandBoxPlus*>(pBox.data()));
		}
	}

finish:
	m_iDeletingContent--;
	return Ret;
}

void CSandMan::OnBoxAdded(const CSandBoxPtr& pBox)
{
	connect(pBox.data(), SIGNAL(StartMenuChanged()), this, SLOT(OnStartMenuChanged()));
}

void CSandMan::EnumBoxLinks(QMap<QString, QMap<QString,SBoxLink> > &BoxLinks, const QString& Prefix, const QString& Folder, bool bWithSubDirs)
{
	QRegularExpression exp("/\\[[0-9Sa-zA-Z_]+\\] ");

	QStringList	Files = ListDir(Folder, QStringList() << "*.lnk" << "*.url" << "*.pif", bWithSubDirs);
	foreach(QString File, Files)
	{
		auto result = exp.match(File);
		if(!result.hasMatch())
			continue;

		int pos = result.capturedStart() + 1;
		int len = result.capturedLength() - 1;
		QString BoxName = File.mid(pos + 1, len - 3);

		SBoxLink BoxLink;
		BoxLink.RelPath = (Prefix + "/" + QString(File).remove(pos, len));
		BoxLink.FullPath = Folder + "/" + File;

		QVariantMap Link = ResolveShortcut(BoxLink.FullPath);
		BoxLink.Target = Link["Arguments"].toString().trimmed();
		if (BoxLink.Target.left(4) == "/box") {
			if(int pos = BoxLink.Target.indexOf(" ") + 1)
				BoxLink.Target = BoxLink.Target.mid(pos).trimmed();
		}

		BoxLinks[BoxName.toLower()].insert(BoxLink.RelPath.toLower(), BoxLink);
	}
}

void CSandMan::CleanupShortcutPath(const QString& Path)
{
	QRegularExpression exp("\\[[0-9Sa-zA-Z_]+\\] ");

	StrPair PathName = Split2(Path, "/", true);
	if (PathName.first.indexOf(exp) != -1) {
		if (ListDir(PathName.first).isEmpty())
		{
			QDir().rmdir(PathName.first);
			//qDebug() << "delete dir" << PathName.first;
			CleanupShortcutPath(PathName.first);
		}
	}
}

void CSandMan::DeleteShortcut(const QString& Path)
{
	QFile::remove(Path);
	//qDebug() << "delete link" << Path;
	CleanupShortcutPath(Path);
}

void CSandMan::CleanUpStartMenu(QMap<QString, QMap<QString, SBoxLink> >& BoxLinks)
{
	for (auto I = BoxLinks.begin(); I != BoxLinks.end(); ++I) {
		for (auto J = I->begin(); J != I->end(); ++J) {
			//qDebug() << "Delete Shortcut" << J.value();
			OnLogMessage(tr("Removed Shortcut: %1").arg(J.key()));
			DeleteShortcut(J->FullPath);
		}
	}
}

void CSandMan::ClearStartMenu()
{
	QMap<QString, QMap<QString, SBoxLink> > BoxLinks;
	EnumBoxLinks(BoxLinks, "Programs", QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation));
	EnumBoxLinks(BoxLinks, "Desktop", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation), false);

	CleanUpStartMenu(BoxLinks);
}

bool CSandMan__MatchLinkTarget(QString L, QString R)
{
	if (L == R)
		return true;
	if (L.left(1) == "\"" &&  L == "\"" + R + "\"")
		return true;
	return false;
}

void CSandMan::SyncStartMenu()
{
	m_StartMenuUpdatePending = false;

	int Mode = theConf->GetInt("Options/IntegrateStartMenu", 0);
	if (Mode == 0)
		return;

	QMap<QString, QMap<QString, SBoxLink> > BoxLinks;
	EnumBoxLinks(BoxLinks, "Programs", QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation));
	EnumBoxLinks(BoxLinks, "Desktop", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation), false);

	QMap<QString, CSandBoxPtr> Boxes = theAPI->GetAllBoxes();
	foreach(const CSandBoxPtr & pBox, Boxes) 
	{
		CSandBoxPlus* pBoxEx = (CSandBoxPlus*)pBox.data();

		QMap<QString, SBoxLink>& CurLinks = BoxLinks[pBoxEx->GetName().toLower()];

		foreach(const CSandBoxPlus::SLink & Link, pBoxEx->GetStartMenu())
		{
			QString Location;
			QString Prefix;
			StrPair LocPath = Split2(Link.Folder, "/");
			if (Mode == 2) // deep integration
			{
				if (LocPath.first == "Programs")
					Location = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
				else if (LocPath.first == "Desktop")
					Location = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
				else
					continue;
			}
			else //if(Mode == 1) // contained integration
			{
				Prefix = "Programs\\Sandboxie-Plus\\";
				Location = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + "\\Sandboxie-Plus";
				Location += "\\" + LocPath.first;
			}

			QString Folder;
			if (LocPath.second.isEmpty()) {
				Folder = Location;
				Location += "\\[" + pBoxEx->GetName() + "] ";
			} else {
				Location += "\\[" + pBoxEx->GetName() + "] " + LocPath.second + "\\";
				Folder = Location;
			}

			QString Key = QString(Prefix + Link.Folder + "\\" + Link.Name + ".lnk").replace("\\", "/").toLower();
			SBoxLink BoxLink = CurLinks.take(Key);
			if (BoxLink.FullPath.isEmpty() || !CSandMan__MatchLinkTarget(BoxLink.Target, Link.Target)) {
				//qDebug() << "CreateShortcut" << Location + Link.Name;
				if (!BoxLink.FullPath.isEmpty()) {
					QFile::remove(BoxLink.FullPath);
					OnLogMessage(tr("Updated Shortcut to: %1").arg(Key));
				} else
					OnLogMessage(tr("Added Shortcut to: %1").arg(Key));
				QDir().mkpath(Folder);
				CSbieUtils::CreateShortcut(theAPI, Location + Link.Name,
						Link.Name, pBoxEx->GetName(), Link.Target, Link.Icon.isEmpty() ? Link.Target : Link.Icon, Link.IconIndex);
			}
		}
	}

	CleanUpStartMenu(BoxLinks);
}

void CSandMan::OnStartMenuChanged()
{
	if (!m_StartMenuUpdatePending)
	{
		m_StartMenuUpdatePending = true;
		QTimer::singleShot(1000, this, SLOT(SyncStartMenu()));
	}
}

void CSandMan::OnBoxClosed(const CSandBoxPtr& pBox)
{
	if (!pBox->GetBool("NeverDelete", false))
	{
		if (pBox->GetBool("AutoDelete", false))
		{
			bool DeleteSnapshots = false;
			// if this box auto deletes first show the recovry dialog with the option to abort deletion
			if (!theGUI->OpenRecovery(pBox, DeleteSnapshots, true)) // unless no files are found than continue silently
				return;

			if (theConf->GetBool("Options/AutoBoxOpsNotify", false))
				OnLogMessage(tr("Auto deleting content of %1").arg(pBox->GetName()), true);

			if (theConf->GetBool("Options/UseAsyncBoxOps", false) || IsSilentMode())
			{
				auto pBoxEx = pBox.objectCast<CSandBoxPlus>();
				SB_STATUS Status = pBoxEx->DeleteContentAsync(DeleteSnapshots);
				CheckResults(QList<SB_STATUS>() << Status);
			}
			else
				DeleteBoxContent(pBox, eAuto, DeleteSnapshots);
		}
	}
}

void CSandMan::OnBoxCleaned(CSandBoxPlus* pBoxEx)
{
	if (pBoxEx->GetBool("AutoRemove", false)) 
	{
		if (theConf->GetBool("Options/AutoBoxOpsNotify", false))
			OnLogMessage(tr("Auto removing sandbox %1").arg(pBoxEx->GetName()), true);

		pBoxEx->RemoveBox();
		return;
	}
}

void CSandMan::OnStatusChanged()
{
	QString appTitle = tr("Sandboxie-Plus v%1").arg(GetVersion());
	if (theAPI->IsConnected())
	{
		bool bPortable = IsFullyPortable();

		QString SbiePath = theAPI->GetSbiePath();
		OnLogMessage(tr("%1 Directory: %2").arg(bPortable ? tr("Application") : tr("Installation")).arg(SbiePath));
		OnLogMessage(tr("Sandboxie-Plus Version: %1 (%2)").arg(GetVersion()).arg(theAPI->GetVersion()));
		OnLogMessage(tr("Current Config: %1").arg(theAPI->GetIniPath()));
		OnLogMessage(tr("Data Directory: %1").arg(QString(theConf->GetConfigDir()).replace("/","\\")));

		//statusBar()->showMessage(tr("Driver version: %1").arg(theAPI->GetVersion()));
		
		//appTitle.append(tr("   -   Driver: v%1").arg(theAPI->GetVersion()));
		if (bPortable)
		{
			//appTitle.append(tr("   -   Portable"));

			QString BoxPath = QDir::cleanPath(QApplication::applicationDirPath() + "/../Sandbox").replace("/", "\\");

			int PortableRootDir = theConf->GetInt("Options/PortableRootDir", 2);
			if (PortableRootDir == 2)
			{
				QString NtBoxRoot = theAPI->GetGlobalSettings()->GetText("FileRootPath", "\\??\\%SystemDrive%\\Sandbox\\%USER%\\%SANDBOX%", false, false).replace("GlobalSettings", "[BoxName]");

				bool State = false;
				PortableRootDir = CCheckableMessageBox::question(this, "Sandboxie-Plus", 
					tr("Sandboxie-Plus was started in portable mode, do you want to put the Sandbox folder into its parent directory?\nYes will choose: %1\nNo will choose: %2")
					.arg(BoxPath + "\\[BoxName]")
					.arg(theAPI->Nt2DosPath(NtBoxRoot))
					, tr("Don't show this message again."), &State, QDialogButtonBox::Yes | QDialogButtonBox::No, QDialogButtonBox::Yes, QMessageBox::Information) == QDialogButtonBox::Yes ? 1 : 0;

				if (State)
					theConf->SetValue("Options/PortableRootDir", PortableRootDir);
			}

			if (PortableRootDir)
				theAPI->GetGlobalSettings()->SetText("FileRootPath", BoxPath + "\\%SANDBOX%");
		}

		if (SbiePath.compare(QApplication::applicationDirPath().replace("/", "\\"), Qt::CaseInsensitive) == 0)
		{
			if (theAPI->GetUserSettings()->GetText("SbieCtrl_AutoStartAgent").isEmpty())
				theAPI->GetUserSettings()->SetText("SbieCtrl_AutoStartAgent", "SandMan.exe");

			QString cmd = CSbieUtils::GetContextMenuStartCmd();
			if (!cmd.isEmpty() && !cmd.contains("SandMan.exe", Qt::CaseInsensitive))
				CSettingsWindow__AddContextMenu();
		}

		m_pBoxView->Clear();

		OnIniReloaded();

		theAPI->WatchIni(true, theConf->GetBool("Options/WatchIni", true));

		if (!theAPI->ReloadCert().IsError())
			CSettingsWindow::LoadCertificate();
		else {
			g_Certificate.clear();

			QString CertPath = QCoreApplication::applicationDirPath() + "\\Certificate.dat";
			if(QFile::exists(CertPath)) // always delete invalid certificates
				WindowsMoveFile(CertPath.replace("/", "\\"), "");
		}
		UpdateCertState();

		uchar UsageFlags = 0;
		if (theAPI->GetSecureParam("UsageFlags", &UsageFlags, sizeof(UsageFlags))) {
			if (!g_CertInfo.business) {
				if ((UsageFlags & (2 | 1)) != 0) {
					if(g_CertInfo.valid)
						appTitle.append(tr(" for Personal use"));
					else
						appTitle.append(tr("   -   for Non-Commercial use ONLY"));
				}
			}
		}
		else { // migrate value form ini to registry // todo remove in later builds
			int BusinessUse = theConf->GetInt("Options/BusinessUse", 2);
			if (BusinessUse == 1) {
				UsageFlags = 1;
				theAPI->SetSecureParam("UsageFlags", &UsageFlags, sizeof(UsageFlags));
			}
		}
		
		g_FeatureFlags = theAPI->GetFeatureFlags();

		SB_STATUS Status = theAPI->ReloadBoxes(true);

		if (!Status.IsError()) {

			auto AllBoxes = theAPI->GetAllBoxes();

			m_pBoxView->ClearUserUIConfig(AllBoxes);

			foreach(const QString & Key, theConf->ListKeys("SizeCache")) {
				if (!AllBoxes.contains(Key.toLower()) || !theConf->GetBool("Options/WatchBoxSize", false))
					theConf->DelValue("SizeCache/" + Key);
			}

			QString DefaultBox = theAPI->GetGlobalSettings()->GetText("DefaultBox", "DefaultBox");
			if (!AllBoxes.contains(DefaultBox.toLower())) {
				OnLogMessage(tr("Default sandbox not found; creating: %1").arg(DefaultBox));
				theAPI->CreateBox(DefaultBox);
			}

			if (theConf->GetBool("Options/CleanUpOnStart", false)) {

				//
				// clean up Auto Delete boxes after reboot
				//

				theAPI->UpdateProcesses(false, ShowAllSessions());

				foreach(const CSandBoxPtr & pBox, AllBoxes) {
					if (pBox->GetActiveProcessCount() == 0)
						OnBoxClosed(pBox);
				}
			}
		}

		if (isVisible())
			CheckSupport();

		int WizardLevel = theConf->GetBool("Options/WizardLevel", 0);
		if (WizardLevel == 0) {
			if (!CSetupWizard::ShowWizard()) // if user canceled mark that and not show again
				theConf->SetValue("Options/WizardLevel", -1);
		}

		if (theConf->GetBool("Options/AutoRunSoftCompat", true))
		{
			if (m_SbieTemplates->RunCheck())
			{
				CSettingsWindow* pSettingsWindow = new CSettingsWindow(this);
				connect(pSettingsWindow, SIGNAL(OptionsChanged(bool)), this, SLOT(UpdateSettings(bool)));
				pSettingsWindow->showTab(CSettingsWindow::eSoftCompat);
			}
		}
	}
	else
	{
		appTitle.append(tr("   -   NOT connected"));

		m_pBoxView->Clear();

		theAPI->WatchIni(false);

		theAPI->StopMonitor();
	}

	this->setWindowTitle(appTitle);

	UpdateState();
}

void CSandMan::UpdateState()
{
	bool isConnected = theAPI->IsConnected();

	//m_pSupport->setVisible(g_Certificate.isEmpty());

	m_pTrayIcon->setIcon(GetTrayIcon(isConnected));
	m_pTrayIcon->setToolTip(GetTrayText(isConnected));
	m_bIconEmpty = true;
	m_iIconDisabled = -1;
	m_bIconBusy = false;
	m_bIconSun = false;

	m_pRunBoxed->setEnabled(isConnected);
	m_pNewBox->setEnabled(isConnected);
	m_pNewGroup->setEnabled(isConnected);
	m_pImportBox->setEnabled(isConnected);
	m_pEmptyAll->setEnabled(isConnected);
	m_pDisableForce->setEnabled(isConnected);
	m_pDisableForce2->setEnabled(isConnected);

	//m_pCleanUpMenu->setEnabled(isConnected);
	//m_pCleanUpButton->setEnabled(isConnected);
	//m_pKeepTerminated->setEnabled(isConnected);

	m_pEditIni->setEnabled(isConnected);
	if(m_pEditIni2) m_pEditIni2->setEnabled(isConnected);
	m_pReloadIni->setEnabled(isConnected);
	if(m_pEnableMonitoring) m_pEnableMonitoring->setEnabled(isConnected);

	if (m_pNewBoxButton) m_pNewBoxButton->setEnabled(isConnected);
	if (m_pEditIniButton) m_pEditIniButton->setEnabled(isConnected);
	if (m_pCleanUpButton) m_pCleanUpButton->setEnabled(isConnected);
}

void CSandMan::OnMenuHover(QAction* action)
{
	//if (!m_pMenuBar->actions().contains(action))
	//	return; // ignore sub menus


	if (m_pMenuBar->actions().at(0) == action && m_pMaintenance)
	{
		bool bConnected = theAPI->IsConnected();
		m_pConnect->setEnabled(!bConnected);
		m_pDisconnect->setEnabled(bConnected);

		m_pMaintenanceItems->setEnabled(!bConnected);

		bool DrvInstalled = CSbieUtils::IsInstalled(CSbieUtils::eDriver);
		bool DrvLoaded = CSbieUtils::IsRunning(CSbieUtils::eDriver);
		m_pInstallDrv->setEnabled(!DrvInstalled);
		m_pStartDrv->setEnabled(!DrvLoaded);
		m_pStopDrv->setEnabled(DrvLoaded);
		m_pUninstallDrv->setEnabled(DrvInstalled);

		bool SvcInstalled = CSbieUtils::IsInstalled(CSbieUtils::eService);
		bool SvcStarted = CSbieUtils::IsRunning(CSbieUtils::eService);
		m_pInstallSvc->setEnabled(!SvcInstalled);
		m_pStartSvc->setEnabled(!SvcStarted && DrvInstalled);
		m_pStopSvc->setEnabled(SvcStarted);
		m_pUninstallSvc->setEnabled(SvcInstalled);

		//m_pMenuStopAll - always enabled
	}

	if (m_pMenuBar->actions().at(2) == action && m_pSandbox)
		CreateBoxMenu(m_pSandbox);
}

void CSandMan::CheckSupport()
{
	if (CSupportDialog::CheckSupport())
		return;

	static bool ReminderShown = false;
	if (!ReminderShown && (g_CertInfo.expired || g_CertInfo.about_to_expire) && !theConf->GetBool("Options/NoSupportCheck", false)) 
	{
		ReminderShown = true;
		CSettingsWindow* pSettingsWindow = new CSettingsWindow(this);
		connect(pSettingsWindow, SIGNAL(OptionsChanged(bool)), this, SLOT(UpdateSettings(bool)));
		pSettingsWindow->showTab(CSettingsWindow::eSupport);
	}
}

#define HK_PANIC 1

void CSandMan::SetupHotKeys()
{
	m_pHotkeyManager->unregisterAllHotkeys();

	if (theConf->GetBool("Options/EnablePanicKey", false))
		m_pHotkeyManager->registerHotkey(theConf->GetString("Options/PanicKeySequence", "Shift+Pause"), HK_PANIC);
}

void CSandMan::OnHotKey(size_t id)
{
	switch (id)
	{
	case HK_PANIC: 
		theAPI->TerminateAll();
		break;
	}
}

void CSandMan::OnLogMessage(const QString& Message, bool bNotify)
{
	AddLogMessage(Message);

	if (bNotify) {
		statusBar()->showMessage(Message);
		m_pTrayIcon->showMessage("Sandboxie-Plus", Message);
	}
}

void CSandMan::AddLogMessage(const QString& Message)
{
	if (!m_pMessageLog)
		return;

	int last = m_pMessageLog->GetTree()->topLevelItemCount();
	if (last > 0) {
		QTreeWidgetItem* pItem = m_pMessageLog->GetTree()->topLevelItem(last-1);
		if (pItem->data(1, Qt::UserRole).toString() == Message) {
			int Count = pItem->data(0, Qt::UserRole).toInt();
			if (Count == 0)
				Count = 1;
			Count++;
			pItem->setData(0, Qt::UserRole, Count);
			QLabel* pLabel = (QLabel*)m_pMessageLog->GetTree()->itemWidget(pItem, 1);
			if(pLabel)
				pLabel->setText(Message + tr(" (%1)").arg(Count));
			else
				pItem->setText(1, Message + tr(" (%1)").arg(Count));
			return;
		}
	}

	QTreeWidgetItem* pItem = new QTreeWidgetItem(); // Time|Message
	pItem->setText(0, QDateTime::currentDateTime().toString("hh:mm:ss.zzz"));
	pItem->setData(1, Qt::UserRole, Message);
	m_pMessageLog->GetTree()->addTopLevelItem(pItem);
	if (Message.contains("</a>")) {
		QLabel* pLabel = new QLabel(Message);
		pLabel->setContentsMargins(3, 0, 0, 0);
		pLabel->setAutoFillBackground(true);
		connect(pLabel, SIGNAL(linkActivated(const QString&)), theGUI, SLOT(OpenUrl(const QString&)));
		m_pMessageLog->GetTree()->setItemWidget(pItem, 1, pLabel);

		QRegularExpression tagExp("<[^>]*>");
		QString TextMessage = Message;
		TextMessage.remove(tagExp);
		pItem->setText(1, TextMessage);
	}
	else
		pItem->setText(1, Message);
		

	m_pMessageLog->GetView()->verticalScrollBar()->setValue(m_pMessageLog->GetView()->verticalScrollBar()->maximum());
}

void CSandMan::OnLogSbieMessage(quint32 MsgCode, const QStringList& MsgData, quint32 ProcessId)
{
	if ((MsgCode & 0xFFFF) == 2198 ) // file migration progress
	{
		if (!IsDisableMessages() && theConf->GetBool("Options/ShowMigrationProgress", true))
			m_pPopUpWindow->ShowProgress(MsgCode, MsgData, ProcessId);
		return;
	}

	if ((MsgCode & 0xFFFF) == 1411) // removed/missing template
	{
		if(MsgData.size() >= 3 && !m_MissingTemplates.contains(MsgData[2]))
			m_MissingTemplates.append(MsgData[2]);
	}

	if ((MsgCode & 0xFFFF) == 6004) // certificate error
	{
		static quint64 iLastCertWarning = 0;
		if (iLastCertWarning + 60 < QDateTime::currentDateTime().toSecsSinceEpoch()) { // reset after 60 seconds
			iLastCertWarning = QDateTime::currentDateTime().toSecsSinceEpoch();
			
			QString Message;
			if (!MsgData[2].isEmpty())
				Message = tr("The program %1 started in box %2 will be terminated in 5 minutes because the box was configured to use features exclusively available to project supporters.").arg(MsgData[2]).arg(MsgData[1]);
			else 
				Message = tr("The box %1 is configured to use features exclusively available to project supporters, these presets will be ignored.").arg(MsgData[1]);
			Message.append(tr("<br /><a href=\"https://sandboxie-plus.com/go.php?to=sbie-get-cert\">Become a project supporter</a>, and receive a <a href=\"https://sandboxie-plus.com/go.php?to=sbie-cert\">supporter certificate</a>"));

			QMessageBox msgBox(this);
			msgBox.setTextFormat(Qt::RichText);
			msgBox.setIcon(QMessageBox::Critical);
			msgBox.setWindowTitle("Sandboxie-Plus");
			msgBox.setText(Message);
			msgBox.setStandardButtons(QMessageBox::Ok);
			msgBox.exec();
			/*msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			if (msgBox.exec() == QDialogButtonBox::Yes) {
				OpenUrl(QUrl("https://sandboxie-plus.com/go.php?to=sbie-get-cert"));
			}*/
			
			//bCertWarning = false;
		}
		// return;
	}

	QString Message;
	if (MsgCode != 0) {
		Message = theAPI->GetSbieMsgStr(MsgCode, m_LanguageId);
		Message.insert(8, "</a>");
		Message.prepend("<a href=\"https://sandboxie-plus.com/go.php?to=sbie-sbie" + QString::number(MsgCode & 0xFFFF) + "\">");
	}
	else if(MsgData.size() > 0)
		Message = MsgData[0];

	for (int i = 1; i < MsgData.size(); i++)
		Message = Message.arg(MsgData[i]);

	if (ProcessId != 4) // if it's not from the driver, add the pid
	{
		CBoxedProcessPtr pProcess = theAPI->GetProcessById(ProcessId);
		if(pProcess.isNull())
			Message.prepend(tr("PID %1: ").arg(ProcessId));
		else
			Message.prepend(tr("%1 (%2): ").arg(pProcess->GetProcessName()).arg(ProcessId));
	}

	OnLogMessage(Message);

	if ((MsgCode & 0xFFFF) == 6004) // certificate error
		return; // don't pop that one up

	if ((MsgCode & 0xFFFF) == 2111) // process open denided
		return; // don't pop that one up

	if(MsgCode != 0 && theConf->GetBool("Options/ShowNotifications", true) && !IsDisableMessages())
		m_pPopUpWindow->AddLogMessage(Message, MsgCode, MsgData, ProcessId);
}

bool CSandMan::CheckCertificate(QWidget* pWidget) 
{
	if (g_CertInfo.valid)
		return true;

	//if ((g_FeatureFlags & CSbieAPI::eSbieFeatureCert) == 0) {
	//	OnLogMessage(tr("The supporter certificate is expired"));
	//	return false;
	//}

	QMessageBox msgBox(pWidget);
	msgBox.setTextFormat(Qt::RichText);
	msgBox.setIcon(QMessageBox::Information);
	msgBox.setWindowTitle("Sandboxie-Plus");
	msgBox.setText(tr("The selected feature set is only available to project supporters. Processes started in a box with this feature set enabled without a supporter certificate will be terminated after 5 minutes.<br />"
		"<a href=\"https://sandboxie-plus.com/go.php?to=sbie-get-cert\">Become a project supporter</a>, and receive a <a href=\"https://sandboxie-plus.com/go.php?to=sbie-cert\">supporter certificate</a>"));
	msgBox.setStandardButtons(QMessageBox::Ok);
	msgBox.exec();
	/*msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	if (msgBox.exec() == QDialogButtonBox::Yes) {
		OpenUrl(QUrl("https://sandboxie-plus.com/go.php?to=sbie-get-cert"));
	}*/

	return false;
}

void CSandMan::UpdateCertState()
{
	g_CertInfo.State = theAPI->GetCertState();
	if (g_CertInfo.valid)
	{
		auto Args = GetArguments(g_Certificate, L'\n', L':');
		QString Type = Args.value("TYPE").toUpper();
		if (Type.contains("CONTRIBUTOR") || Type.contains("GREAT_PATREON") || Type.contains("HUGE"))
			g_CertInfo.insider = true;

		// behave as if there would be no certificate at all
		if (theConf->GetBool("Debug/IgnoreCertificate", false))
			g_CertInfo.State = 0;
		else
		{
			// simulate certificate being about to expire in 3 days from now
			if (theConf->GetBool("Debug/CertFakeAboutToExpire", false))
				g_CertInfo.expirers_in_sec = 3 * 24 * 3600;

			// simulate certificate having expired but being in the grace period
			if (theConf->GetBool("Debug/CertFakeGracePeriode", false))
				g_CertInfo.grace_period = 1;

			// simulate a subscription type certificate having expired 
			if (theConf->GetBool("Debug/CertFakeOld", false)) {
				g_CertInfo.valid = 0;
				g_CertInfo.expired = 1;
			}

			// simulate a perpetual use certificate being outside the update window
			if (theConf->GetBool("Debug/CertFakeExpired", false)) {
				// still valid
				g_CertInfo.expired = 1;
			}

			// simulate a perpetual use certificate being outside the update window
			// and having been applied to a version built after the update window has ended
			if (theConf->GetBool("Debug/CertFakeOutdated", false)) {
				g_CertInfo.valid = 0;
				g_CertInfo.expired = 1;
				g_CertInfo.outdated = 1;
			}

			// simulate this being a business certificate - only contributors and other insiders
			if (g_CertInfo.insider && theConf->GetBool("Debug/CertFakeBusiness", false))
				g_CertInfo.business = 1;

			// simulate this being a evaluation certificate
			if (theConf->GetBool("Debug/CertFakeEvaluation", false))
				g_CertInfo.evaluation = 1;
		}
	}

	if (g_CertInfo.evaluation)
	{
		if (g_CertInfo.expired)
			OnLogMessage(tr("The evaluation period has expired!!!"));
	}
	else
	{
		g_CertInfo.about_to_expire = g_CertInfo.expirers_in_sec > 0 && g_CertInfo.expirers_in_sec < (60 * 60 * 24 * 30);
		if (g_CertInfo.outdated)
			OnLogMessage(tr("The supporter certificate is not valid for this build, please get an updated certificate"));
		// outdated always implicates it is no longer valid
		else if (g_CertInfo.expired) // may be still valid for the current and older builds
			OnLogMessage(tr("The supporter certificate has expired%1, please get an updated certificate")
				.arg(!g_CertInfo.outdated ? tr(", but it remains valid for the current build") : ""));
		else if (g_CertInfo.about_to_expire)
			OnLogMessage(tr("The supporter certificate will expire in %1 days, please get an updated certificate").arg(g_CertInfo.expirers_in_sec / (60 * 60 * 24)));
	}

	emit CertUpdated();
}

void CSandMan::OnQueuedRequest(quint32 ClientPid, quint32 ClientTid, quint32 RequestId, const QVariantMap& Data)
{
	if (Data["id"].toInt() == 0) 
	{
		QVariantMap Ret;
		Ret["retval"] = (theAPI->IsStarting(ClientPid) || CSupportDialog::ShowDialog()) ? 1 : 0;
		theAPI->SendReplyData(RequestId, Ret);
		return;
	}
	m_pPopUpWindow->AddUserPrompt(RequestId, Data, ClientPid);
}

#include "SandManRecovery.cpp"

int CSandMan::ShowQuestion(const QString& question, const QString& checkBoxText, bool* checkBoxSetting, int buttons, int defaultButton, int type, QWidget* pParent)
{
	return CCheckableMessageBox::question(pParent, "Sandboxie-Plus", question, checkBoxText, checkBoxSetting, (QDialogButtonBox::StandardButtons)buttons, (QDialogButtonBox::StandardButton)defaultButton, (QMessageBox::Icon)type);
}

void CSandMan::ShowMessage(const QString& message, int type)
{
    QMessageBox mb(this);
    mb.setWindowTitle("Sandboxie-Plus");
    mb.setIconPixmap(QMessageBox::standardIcon((QMessageBox::Icon)type));
    mb.setText(message);
    mb.exec();
	QTimer::singleShot(10, [this]() {
		this->raise();
	});
}

void CSandMan::OnNotAuthorized(bool bLoginRequired, bool& bRetry)
{
	if (!bLoginRequired)
	{
		QMessageBox::warning(this, "Sandboxie-Plus", tr("Only Administrators can change the config."));
		return;
	}

	static bool LoginOpen = false;
	if (LoginOpen)
		return;
	LoginOpen = true;
	for (;;)
	{
		QString Value = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter the configuration password."), QLineEdit::Password);
		if (Value.isEmpty())
			break;
		SB_STATUS Status = theAPI->UnlockConfig(Value);
		if (!Status.IsError()) {
			bRetry = true;
			break;
		}
		QMessageBox::warning(this, "Sandboxie-Plus", tr("Login Failed: %1").arg(FormatError(Status)));
	}
	LoginOpen = false;
}

void CSandMan::OnBoxDblClick(QTreeWidgetItem* pItem)
{
	m_pBoxView->ShowOptions(pItem->data(0, Qt::UserRole).toString());
}

void CSandMan::OnSandBoxAction()
{
	QAction* pAction = qobject_cast<QAction*>(sender());

	if (pAction == m_pNewBox)
		GetBoxView()->AddNewBox();
	else if (pAction == m_pNewGroup)
		GetBoxView()->AddNewGroup();
	else if (pAction == m_pImportBox)
		GetBoxView()->ImportSandbox();
	else if (pAction == m_pRunBoxed)
		RunSandboxed(QStringList() << "run_dialog");
}

void CSandMan::OnEmptyAll()
{
 	if (theConf->GetInt("Options/WarnTerminateAll", -1) == -1)
	{
		bool State = false;
		if(CCheckableMessageBox::question(this, "Sandboxie-Plus", tr("Do you want to terminate all processes in all sandboxes?")
			, tr("Terminate all without asking"), &State, QDialogButtonBox::Yes | QDialogButtonBox::No, QDialogButtonBox::Yes, QMessageBox::Information) != QDialogButtonBox::Yes)
			return;

		if (State)
			theConf->SetValue("Options/WarnTerminateAll", 1);
	}

	theAPI->TerminateAll();
}

void CSandMan::OnDisableForce()
{
	bool Status = m_pDisableForce->isChecked();
	int Seconds = 0;
	if (Status)
	{
		int LastValue = theAPI->GetGlobalSettings()->GetNum("ForceDisableSeconds", 60);

		bool bOK = false;
		Seconds = QInputDialog::getInt(this, "Sandboxie-Plus", tr("Please enter the duration, in seconds, for disabling Forced Programs rules."), LastValue, 0, INT_MAX, 1, &bOK);
		if (!bOK)
			return;
	}
	theAPI->DisableForceProcess(Status, Seconds);
}

void CSandMan::OnDisableForce2()
{
	bool Status = m_pDisableForce2->isChecked();
	theAPI->DisableForceProcess(Status);
}

void CSandMan::OnDisablePopUp()
{
	QString Str2 = tr("No Recovery");
	if (m_pDisableRecovery) {
		m_pDisabledRecovery->setText(m_pDisableRecovery->isChecked() ? Str2 : QString(Str2.length(), ' '));
		theConf->SetValue("UIConfig/DisabledRecovery", m_pDisableRecovery->isChecked());
	}

	QString Str3 = tr("No Messages");
	if (m_pDisableMessages) {
		m_pDisabledMessages->setText(m_pDisableMessages->isChecked() ? Str3 : QString(Str3.length(), ' '));
		theConf->SetValue("UIConfig/DisableMessages", m_pDisableMessages->isChecked());
	}
}

SB_RESULT(void*) CSandMan::ConnectSbie()
{
	SB_RESULT(void*) Status;
	if (!CSbieUtils::IsRunning(CSbieUtils::eAll)) 
	{
		if (!CSbieUtils::IsInstalled(CSbieUtils::eAll))
		{
			int PortableStart = theConf->GetInt("Options/PortableStart", -1);
			if (PortableStart == -1)
			{
				bool State = false;
				PortableStart = CCheckableMessageBox::question(this, "Sandboxie-Plus", tr("Sandboxie-Plus was started in portable mode and it needs to create necessary services. This will prompt for administrative privileges.")
					, tr("Don't show this message again."), &State, QDialogButtonBox::Ok | QDialogButtonBox::Cancel, QDialogButtonBox::Ok, QMessageBox::Information) == QDialogButtonBox::Ok ? 1 : 0;

				if (State)
					theConf->SetValue("Options/PortableStart", PortableStart);
			}

			if(!PortableStart)
				return SB_OK;
		}

		Status = CSbieUtils::Start(CSbieUtils::eAll);
	}

	if (Status.GetStatus() == OP_ASYNC) {
		m_bConnectPending = true;
		return Status;
	}
	if (Status.IsError())
		return Status;

	return ConnectSbieImpl();
}

SB_STATUS CSandMan::ConnectSbieImpl()
{
	SB_STATUS Status = theAPI->Connect(g_PendingMessage.isEmpty(), theConf->GetBool("Options/UseInteractiveQueue", true));
	
	if (!g_PendingMessage.isEmpty()) {
		OnMessage(g_PendingMessage);
		PostQuitMessage(0);
	}

	if (Status.GetStatus() == 0xC0000038L /*STATUS_DEVICE_ALREADY_ATTACHED*/) {
		OnLogMessage(tr("CAUTION: Another agent (probably SbieCtrl.exe) is already managing this Sandboxie session, please close it first and reconnect to take over."));
		Status = SB_OK;
	}
	else if (Status.GetStatus() == 0xC000A000L /*STATUS_INVALID_SIGNATURE*/) {
		QMessageBox::critical(this, "Sandboxie-Plus", tr("<b>ERROR:</b> The Sandboxie-Plus Manager (SandMan.exe) does not have a valid signature (SandMan.exe.sig). Please download a trusted release from the <a href=\"https://sandboxie-plus.com/go.php?to=sbie-get\">official Download page</a>."));
		Status = SB_OK;
	}

	return Status;
}

SB_STATUS CSandMan::DisconnectSbie()
{
	return theAPI->Disconnect();
}

SB_RESULT(void*) CSandMan::StopSbie(bool andRemove)
{
	SB_RESULT(void*) Status;

	if (theAPI->IsConnected()) {
		Status = theAPI->TerminateAll();
		theAPI->Disconnect();
	}
	if (!Status.IsError()) {
		if(andRemove)
			Status = CSbieUtils::Uninstall(CSbieUtils::eAll); // it stops it first of course
		else
			Status = CSbieUtils::Stop(CSbieUtils::eAll);
		if (Status.GetStatus() == OP_ASYNC)
			m_bStopPending = true;
	}

	return Status;
}

void CSandMan::OnMaintenance()
{
	SB_RESULT(void*) Status;
	if (sender() == m_pConnect)
		Status = ConnectSbie();
	else if (sender() == m_pDisconnect)
		Status = DisconnectSbie();
	else if (sender() == m_pStopAll)
		Status = StopSbie();

	// advanced
	else if (sender() == m_pInstallDrv)
		Status = CSbieUtils::Install(CSbieUtils::eDriver);
	else if (sender() == m_pStartDrv)
		Status = CSbieUtils::Start(CSbieUtils::eDriver);
	else if (sender() == m_pStopDrv)
		Status = CSbieUtils::Stop(CSbieUtils::eDriver);
	else if (sender() == m_pUninstallDrv)
		Status = CSbieUtils::Uninstall(CSbieUtils::eDriver);

	else if (sender() == m_pInstallSvc)
		Status = CSbieUtils::Install(CSbieUtils::eService);
	else if (sender() == m_pStartSvc)
		Status = CSbieUtils::Start(CSbieUtils::eService);
	else if (sender() == m_pStopSvc)
		Status = CSbieUtils::Stop(CSbieUtils::eService);
	else if (sender() == m_pUninstallSvc)
		Status = CSbieUtils::Uninstall(CSbieUtils::eService);

	else if (sender() == m_pSetupWizard) {
		CSetupWizard::ShowWizard();
		return;
	}

	//else if (sender() == m_pUpdateCore) {
	//	// todo	
	//	return;
	//}

	// uninstall	
	else if (sender() == m_pUninstallAll) {

		Status = StopSbie(true);

		CSetupWizard::ShellUninstall();
	}

	HandleMaintenance(Status);
}

void CSandMan::HandleMaintenance(SB_RESULT(void*) Status)
{
	if (Status.GetStatus() == OP_ASYNC) {

		HANDLE hProcess = Status.GetValue();
		QWinEventNotifier* processFinishedNotifier = new QWinEventNotifier(hProcess);
		processFinishedNotifier->setEnabled(true);
		connect(processFinishedNotifier, &QWinEventNotifier::activated, this, [processFinishedNotifier, this, hProcess]() {
			processFinishedNotifier->setEnabled(false);
			processFinishedNotifier->deleteLater();
			
			DWORD dwStatus = 0;
			GetExitCodeProcess(hProcess, & dwStatus);

			if (dwStatus != 0)
			{
				if(m_bStopPending)
					QMessageBox::warning(this, tr("Sandboxie-Plus - Error"), tr("Failed to stop all Sandboxie components"));
				else if(m_bConnectPending)
					QMessageBox::warning(this, tr("Sandboxie-Plus - Error"), tr("Failed to start required Sandboxie components"));

				OnLogMessage(tr("Maintenance operation failed (%1)").arg((quint32)dwStatus));
				CheckResults(QList<SB_STATUS>() << SB_ERR(dwStatus));
			}
			else
			{
				OnLogMessage(tr("Maintenance operation completed"));
				if (m_bConnectPending) {

					QTimer::singleShot(1000, [this]() {
						SB_STATUS Status = this->ConnectSbieImpl();
						CheckResults(QList<SB_STATUS>() << Status);
						if (Status.IsError())
							theAPI->LoadEventLog();
					});
				}
			}
			m_pProgressDialog->hide();
			//statusBar()->showMessage(tr("Maintenance operation completed"), 3000);
			m_bConnectPending = false;
			m_bStopPending = false;

			CloseHandle(hProcess);
		});

		//statusBar()->showMessage(tr("Executing maintenance operation, please wait..."));
		m_pProgressDialog->OnStatusMessage(tr("Executing maintenance operation, please wait..."));
		SafeShow(m_pProgressDialog);

		return;
	}

	CheckResults(QList<SB_STATUS>() << Status);
}

void CSandMan::OnViewMode(QAction* pAction)
{
	int iViewMode = pAction->data().toInt();
	theConf->SetValue("Options/ViewMode", iViewMode);
	UpdateSettings(true);
}

void CSandMan::OnAlwaysTop()
{
	StoreState();
	bool bAlwaysOnTop = m_pWndTopMost->isChecked();
	theConf->SetValue("Options/AlwaysOnTop", bAlwaysOnTop);
	this->setWindowFlag(Qt::WindowStaysOnTopHint, bAlwaysOnTop);
	LoadState();
	SafeShow(this); // why is this needed?

	m_pPopUpWindow->setWindowFlag(Qt::WindowStaysOnTopHint, bAlwaysOnTop);
	m_pProgressDialog->setWindowFlag(Qt::WindowStaysOnTopHint, bAlwaysOnTop);
}

void CSandMan::OnRefresh()
{
	if (!theAPI->IsConnected())
		return;

	theAPI->ReloadBoxes(true);

	QMap<QString, CSandBoxPtr> Boxes = theAPI->GetAllBoxes();
	foreach(const CSandBoxPtr & pBox, Boxes) {
		pBox.objectCast<CSandBoxPlus>()->UpdateSize();
		if (theConf->GetBool("Options/ScanStartMenu", true))
			pBox.objectCast<CSandBoxPlus>()->ScanStartMenu();
	}
}

void CSandMan::OnCleanUp()
{
	if (sender() == m_pCleanUpMsgLog || sender() == m_pCleanUpButton)
		if(m_pMessageLog) m_pMessageLog->GetTree()->clear();
	
	if (sender() == m_pCleanUpTrace || sender() == m_pCleanUpButton)
		if (m_pTraceView) { 
			m_pTraceView->Clear(); 
			m_pTraceInfo->clear();
		}

	if (sender() == m_pCleanUpRecovery || sender() == m_pCleanUpButton)
		if(m_pRecoveryLog) m_pRecoveryLog->GetTree()->clear();
	
	if (sender() == m_pCleanUpProcesses || sender() == m_pCleanUpButton)
		theAPI->UpdateProcesses(false, ShowAllSessions());
}

void CSandMan::OnProcView()
{
	if(m_pKeepTerminated) theConf->SetValue("Options/KeepTerminated", m_pKeepTerminated->isChecked());
	if(m_pShowAllSessions) theConf->SetValue("Options/ShowAllSessions", m_pShowAllSessions->isChecked());

	if (m_pMenuBrowse) {
		theConf->SetValue("Options/ShowFilePanel", m_pMenuBrowse->isChecked());
		m_pFileView->setVisible(m_pMenuBrowse->isChecked());

		if (m_pMenuBrowse->isChecked()) {
			QTimer::singleShot(10, [&] {
				auto Sizes = m_pPanelSplitter->sizes();
				if (Sizes.at(1) == 0) {
					Sizes[1] = 100;
					m_pPanelSplitter->setSizes(Sizes);
				}
			});
		}
	}
}

void CSandMan::OnSettings()
{
	static CSettingsWindow* pSettingsWindow = NULL;
	if (pSettingsWindow == NULL) {
		pSettingsWindow = new CSettingsWindow();
		connect(this, SIGNAL(Closed()), pSettingsWindow, SLOT(close()));
		connect(pSettingsWindow, SIGNAL(OptionsChanged(bool)), this, SLOT(UpdateSettings(bool)));
		connect(pSettingsWindow, &CSettingsWindow::Closed, [this]() {
			pSettingsWindow = NULL;
			});
		SafeShow(pSettingsWindow);
	}
	else {
		pSettingsWindow->setWindowState((pSettingsWindow->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
		SetForegroundWindow((HWND)pSettingsWindow->winId());
	}
}

// for old menu
void CSandMan::OnSettingsAction()
{
	QAction* pAction = qobject_cast<QAction*>(sender());

	if (pAction == m_pArrangeGroups)
	{
		QMessageBox *msgBox = new QMessageBox(this);
		msgBox->setAttribute(Qt::WA_DeleteOnClose);
		msgBox->setWindowTitle("Sandboxie-Plus");
		msgBox->setText(tr("In the Plus UI, this functionality has been integrated into the main sandbox list view."));
		msgBox->setInformativeText(tr("Using the box/group context menu, you can move boxes and groups to other groups. You can also use drag and drop to move the items around. "
			"Alternatively, you can also use the arrow keys while holding ALT down to move items up and down within their group.<br />"
			"You can create new boxes and groups from the Sandbox menu."));
		QPixmap pic(":/Assets/LayoutAndGroups.png");
		msgBox->setIconPixmap(pic.scaled(pic.width() * 3/4, pic.height() * 3/4, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
		SafeExec(msgBox);
	}
	else
	{
		CSettingsWindow* pSettingsWindow = new CSettingsWindow(this);
		connect(pSettingsWindow, SIGNAL(OptionsChanged(bool)), this, SLOT(UpdateSettings(bool)));
		int Tab = pAction->data().toInt();
		pSettingsWindow->showTab(Tab, true);
	}
}

void CSandMan::UpdateSettings(bool bRebuildUI)
{
	if(m_pTrayBoxes) m_pTrayBoxes->clear(); // force refresh

	//GetBoxView()->UpdateRunMenu();

	SetupHotKeys();

	if (theConf->GetInt("Options/SysTrayIcon", 1))
		m_pTrayIcon->show();
	else
		m_pTrayIcon->hide();


	if (bRebuildUI) 
	{
		StoreState();

		RebuildUI();
	}
}

void CSandMan::RebuildUI()
{
	LoadLanguage();

	if(m_pRefreshAll) this->removeAction(m_pRefreshAll);
	if(m_pMenuBrowse) this->removeAction(m_pMenuBrowse);
	if(m_pMenuResetGUI) this->removeAction(m_pMenuResetGUI);

	m_pMainWidget->deleteLater();
	m_pMainWidget = new QWidget(this);
	setCentralWidget(m_pMainWidget);

	m_pLabel->deleteLater();

	CreateUI();

	m_pTrayMenu->deleteLater();
	CreateTrayMenu();

	LoadState(false);

	GetBoxView()->ReloadUserConfig();

	UpdateState();

	if(m_pTrayBoxes) m_pTrayBoxes->setStyle(QStyleFactory::create(m_DefaultStyle));
}

void CSandMan::OnResetMsgs()
{
	auto Ret = QMessageBox("Sandboxie-Plus", tr("Do you also want to reset hidden message boxes (yes), or only all log messages (no)?"),
		QMessageBox::Question, QMessageBox::Yes | QMessageBox::Default, QMessageBox::No, QMessageBox::Cancel | QMessageBox::Escape, this).exec();
	if (Ret == QMessageBox::Cancel)
		return;

	if (Ret == QMessageBox::Yes)
	{
		theConf->DelValue("Options/WarnDeleteV2");

		theConf->DelValue("Options/PortableStop");
		theConf->DelValue("Options/PortableStart");
		theConf->DelValue("Options/PortableRootDir");

		theConf->DelValue("Options/CheckForUpdates");

		theConf->DelValue("Options/NoEditInfo");
		theConf->DelValue("Options/NoEditWarn");

		theConf->DelValue("Options/BoxedExplorerInfo");
		theConf->DelValue("Options/ExplorerInfo");

		theConf->DelValue("Options/OpenUrlsSandboxed");

		theConf->DelValue("Options/AutoCleanupTemplates");
		theConf->DelValue("Options/WarnTerminateAll");
		theConf->DelValue("Options/WarnTerminate");

		theConf->DelValue("Options/InfoMkLink");

		theConf->DelValue("Options/WarnOpenCOM");
	}

	theAPI->GetUserSettings()->UpdateTextList("SbieCtrl_HideMessage", QStringList(), true);
	m_pPopUpWindow->ReloadHiddenMessages();
}

void CSandMan::OnResetGUI()
{
	foreach(const QString& Option, theConf->ListKeys("RecoveryWindow"))
		theConf->DelValue("RecoveryWindow/" + Option);

	foreach(const QString& Option, theConf->ListKeys("MainWindow"))
		theConf->DelValue("MainWindow/" + Option);

	foreach(const QString& Option, theConf->ListKeys("SettingsWindow"))
		theConf->DelValue("SettingsWindow/" + Option);

	foreach(const QString& Option, theConf->ListKeys("OptionsWindow"))
		theConf->DelValue("OptionsWindow/" + Option);

	theConf->DelValue("ErrorWindow/Window_Geometry");
	theConf->DelValue("PopUpWindow/Window_Geometry");
	theConf->DelValue("TraceWindow/Window_Geometry");
	theConf->DelValue("SelectBoxWindow/Window_Geometry");
	theConf->DelValue("SnapshotsWindow/Window_Geometry");
	theConf->DelValue("FileBrowserWindow/Window_Geometry");
	theConf->DelValue("RecoveryLogWindow/Window_Geometry");

//	theConf->SetValue("Options/DPIScaling", 1);
	theConf->SetValue("Options/FontScaling", 100);

	RebuildUI();
}

void CSandMan::OnEditIni()
{
	QString ini = ((QAction*)sender())->property("ini").toString();

	QString IniPath;
	bool bPlus;
	if (bPlus = (ini == "plus"))
	{
		IniPath = QString(theConf->GetConfigDir() + "/Sandboxie-Plus.ini").replace("/", "\\");
	}
	else if (ini == "tmpl")
	{
		IniPath = theAPI->GetSbiePath() + "\\Templates.ini";

		if (theConf->GetBool("Options/NoEditWarn", true)) {
			bool State = false;
			CCheckableMessageBox::question(this, "Sandboxie-Plus",
				tr("You are about to edit the Templates.ini, this is generally not recommended.\n"
					"This file is part of Sandboxie and all change done to it will be reverted next time Sandboxie is updated.")
				, tr("Don't show this message again."), &State, QDialogButtonBox::Ok, QDialogButtonBox::Ok, QMessageBox::Warning);

			if (State)
				theConf->SetValue("Options/NoEditWarn", false);
		}
	}
	else //if (ini == "sbie")
	{
		IniPath = theAPI->GetIniPath();

		if (theConf->GetBool("Options/NoEditInfo", true)) {
			bool State = false;
			CCheckableMessageBox::question(this, "Sandboxie-Plus",
				theConf->GetBool("Options/WatchIni", true)
				? tr("The changes will be applied automatically whenever the file gets saved.")
				: tr("The changes will be applied automatically as soon as the editor is closed.")
				, tr("Don't show this message again."), &State, QDialogButtonBox::Ok, QDialogButtonBox::Ok, QMessageBox::Information);

			if (State)
				theConf->SetValue("Options/NoEditInfo", false);
		}
	}

	bool bIsWritable = bPlus;
	if (!bIsWritable) {
		QFile File(IniPath);
		if (File.open(QFile::ReadWrite)) {
			bIsWritable = true;
			File.close();
		}
		// todo: warn user about file not being protected
	}

	std::wstring Editor = theConf->GetString("Options/Editor", "notepad.exe").toStdWString();
	std::wstring iniPath = L"\"" + IniPath.toStdWString() + L"\"";

	SHELLEXECUTEINFO si = { 0 };
	si.cbSize = sizeof(SHELLEXECUTEINFO);
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.hwnd = NULL;
	si.lpVerb = bIsWritable ? NULL : L"runas"; // plus ini does not require admin privileges
	si.lpFile = Editor.c_str();
	si.lpParameters = iniPath.c_str();
	si.lpDirectory = NULL;
	si.nShow = SW_SHOW;
	si.hInstApp = NULL;
	ShellExecuteEx(&si);
	//WaitForSingleObject(si.hProcess, INFINITE);
	//CloseHandle(si.hProcess);

	if (!bPlus && theConf->GetBool("Options/WatchIni", true))
		return; // if the ini is watched don't double reload
	
	QWinEventNotifier* processFinishedNotifier = new QWinEventNotifier(si.hProcess);
	processFinishedNotifier->setEnabled(true);
	connect(processFinishedNotifier, &QWinEventNotifier::activated, this, [processFinishedNotifier, this, si, bPlus]() {
		processFinishedNotifier->setEnabled(false);
		processFinishedNotifier->deleteLater();
		if (bPlus)
			theConf->Sync();
		else
			this->OnReloadIni();
		CloseHandle(si.hProcess);
	});
}

void CSandMan::OnReloadIni()
{
	theAPI->ReloadConfig(true);
}

void CSandMan::OnIniReloaded()
{
	OnLogSbieMessage(0, QStringList() << tr("Sandboxie config has been reloaded") << "" << "", 4);

	m_pBoxView->ReloadUserConfig();
	m_pPopUpWindow->ReloadHiddenMessages();

	g_FeatureFlags = theAPI->GetFeatureFlags();

	if (theAPI->GetGlobalSettings()->GetBool("MonitorStackTrace", false)) {
		QString SymPath = theConf->GetString("Options/Debug");
		if (SymPath.isEmpty()){
			SymPath = "SRV*C:\\Symbols*https://msdl.microsoft.com/download/symbols";
			theConf->SetValue("Options/Debug", SymPath);
		}
		CSymbolProvider::Instance()->SetSymPath(SymPath);
	}
}

void CSandMan::OnMonitoring()
{
	if (m_pTraceView)
	{
		theAPI->EnableMonitor(m_pEnableMonitoring->isChecked());

		if(m_pEnableMonitoring->isChecked() && !m_pToolBar->isVisible())
			m_pLogTabs->show();

		if(m_pEnableMonitoring->isChecked())
			m_pKeepTerminated->setChecked(true);
		else
			m_pKeepTerminated->setChecked(theConf->GetBool("Options/KeepTerminated"));
	}
	else
	{
		theAPI->EnableMonitor(true);

		static CTraceWindow* pTraceWindow = NULL;
		if (!pTraceWindow) {
			pTraceWindow = new CTraceWindow();
			connect(this, SIGNAL(Closed()), pTraceWindow, SLOT(close()));
			//pTraceWindow->setAttribute(Qt::WA_DeleteOnClose);
			connect(pTraceWindow, &CTraceWindow::Closed, [&]() {
				pTraceWindow = NULL;
			});
			SafeShow(pTraceWindow);
		}
	}
}

SB_STATUS CSandMan::AddAsyncOp(const CSbieProgressPtr& pProgress, bool bWait, const QString& InitialMsg)
{
	m_pAsyncProgress.insert(pProgress.data(), pProgress);
	connect(pProgress.data(), SIGNAL(Message(const QString&)), this, SLOT(OnAsyncMessage(const QString&)));
	connect(pProgress.data(), SIGNAL(Progress(int)), this, SLOT(OnAsyncProgress(int)));
	connect(pProgress.data(), SIGNAL(Finished()), this, SLOT(OnAsyncFinished()));

	m_pProgressDialog->OnStatusMessage(InitialMsg);
	if (bWait) {
		m_pProgressModal = true;
		m_pProgressDialog->exec(); // safe exec breaks the closing
		m_pProgressModal = false;
	}
	else
		SafeShow(m_pProgressDialog);

	if (pProgress->IsFinished()) // Note: since the operation runs asynchronously, it may have already finished, so we need to test for that
		OnAsyncFinished(pProgress.data());

	if (pProgress->IsCanceled())
		return CSbieStatus(SB_Canceled);
	return SB_OK;
}

void CSandMan::OnAsyncFinished()
{
	OnAsyncFinished(qobject_cast<CSbieProgress*>(sender()));
}

void CSandMan::OnAsyncFinished(CSbieProgress* pSender)
{
	CSbieProgressPtr pProgress = m_pAsyncProgress.take(pSender);
	if (pProgress.isNull())
		return;
	disconnect(pProgress.data() , SIGNAL(Finished()), this, SLOT(OnAsyncFinished()));

	SB_STATUS Status = pProgress->GetStatus();
	if(Status.IsError())
		CSandMan::CheckResults(QList<SB_STATUS>() << Status);

	if (m_pAsyncProgress.isEmpty()) {
		if(m_pProgressModal)
			m_pProgressDialog->close();
		else
			m_pProgressDialog->hide();
	}
}

void CSandMan::OnAsyncMessage(const QString& Text)
{
	m_pProgressDialog->OnStatusMessage(Text);
}

void CSandMan::OnAsyncProgress(int Progress)
{
	m_pProgressDialog->OnProgressMessage("", Progress);
}

void CSandMan::OnCancelAsync()
{
	foreach(const CSbieProgressPtr& pProgress, m_pAsyncProgress)
		pProgress->Cancel();
}

QString CSandMan::FormatError(const SB_STATUS& Error)
{
	//QString Text = Error.GetText();
	//if (!Text.isEmpty())
	//	return Text;

	QString Message;
	switch (Error.GetMsgCode())
	{
	case SB_Generic:		return tr("Error Status: 0x%1 (%2)").arg((quint32)Error.GetStatus(), 8, 16, QChar('0')).arg(
		(Error.GetArgs().isEmpty() || Error.GetArgs().first().toString().isEmpty()) ? tr("Unknown") : Error.GetArgs().first().toString().trimmed());
	case SB_Message:		Message = "%1"; break;
	case SB_NeedAdmin:		Message = tr("Administrator rights are required for this operation."); break;
	case SB_ExecFail:		Message = tr("Failed to execute: %1"); break;
	case SB_DriverFail:		Message = tr("Failed to connect to the driver"); break;
	case SB_ServiceFail:	Message = tr("Failed to communicate with Sandboxie Service: %1"); break;
	case SB_Incompatible:	Message = tr("An incompatible Sandboxie %1 was found. Compatible versions: %2"); break;
	case SB_PathFail:		Message = tr("Can't find Sandboxie installation path."); break;
	case SB_FailedCopyConf:	Message = tr("Failed to copy configuration from sandbox %1: %2"); break;
	case SB_AlreadyExists:  Message = tr("A sandbox of the name %1 already exists"); break;
	case SB_DeleteFailed:	Message = tr("Failed to delete sandbox %1: %2"); break;
	case SB_NameLenLimit:	Message = tr("The sandbox name can not be longer than 32 characters."); break;
	case SB_BadNameDev:		Message = tr("The sandbox name can not be a device name."); break;
	case SB_BadNameChar:	Message = tr("The sandbox name can contain only letters, digits and underscores which are displayed as spaces."); break;
	case SB_FailedKillAll:	Message = tr("Failed to terminate all processes"); break;
	case SB_DeleteProtect:	Message = tr("Delete protection is enabled for the sandbox"); break;
	case SB_DeleteNotEmpty:	Message = tr("All sandbox processes must be stopped before the box content can be deleted"); break;
	case SB_DeleteError:	Message = tr("Error deleting sandbox folder: %1"); break;
	//case SB_RemNotEmpty:	Message = tr("A sandbox must be emptied before it can be renamed."); break;
	case SB_DelNotEmpty:	Message = tr("A sandbox must be emptied before it can be deleted."); break;
	case SB_FailedMoveDir:	Message = tr("Failed to move directory '%1' to '%2'"); break;
	case SB_SnapIsRunning:	Message = tr("This Snapshot operation can not be performed while processes are still running in the box."); break;
	case SB_SnapMkDirFail:	Message = tr("Failed to create directory for new snapshot"); break;
	case SB_SnapCopyDatFail:Message = tr("Failed to copy box data files"); break;
	case SB_SnapNotFound:	Message = tr("Snapshot not found"); break;
	case SB_SnapMergeFail:	Message = tr("Error merging snapshot directories '%1' with '%2', the snapshot has not been fully merged."); break;
	case SB_SnapRmDirFail:	Message = tr("Failed to remove old snapshot directory '%1'"); break;
	case SB_SnapIsShared:	Message = tr("Can't remove a snapshot that is shared by multiple later snapshots"); break;
	case SB_SnapDelDatFail:	Message = tr("Failed to remove old box data files"); break;
	case SB_NotAuthorized:	Message = tr("You are not authorized to update configuration in section '%1'"); break;
	case SB_ConfigFailed:	Message = tr("Failed to set configuration setting %1 in section %2: %3"); break;
	case SB_SnapIsEmpty:	Message = tr("Can not create snapshot of an empty sandbox"); break;
	case SB_NameExists:		Message = tr("A sandbox with that name already exists"); break;
	case SB_PasswordBad:	Message = tr("The config password must not be longer than 64 characters"); break;
	case SB_Canceled:		Message = tr("The operation was canceled by the user"); break;

	case SBX_7zNotReady:	Message = tr("Import/Export not available, 7z.dll could not be loaded"); break;
	case SBX_7zCreateFailed: Message = tr("Failed to create the box archive"); break;
	case SBX_7zOpenFailed:	Message = tr("Failed to open the 7z archive"); break;
	case SBX_7zExtractFailed: Message = tr("Failed to unpack the box archive"); break;
	case SBX_NotBoxArchive:	Message = tr("The selected 7z file is NOT a box archive"); break;

	default:				return tr("Unknown Error Status: 0x%1").arg((quint32)Error.GetStatus(), 8, 16, QChar('0'));
	}

	foreach(const QVariant& Arg, Error.GetArgs())
		Message = Message.arg(Arg.toString()); // todo: make quint32 hex and so on

	return Message;
}

void CSandMan::CheckResults(QList<SB_STATUS> Results, bool bAsync)
{
	QStringList Errors;
	for (QList<SB_STATUS>::iterator I = Results.begin(); I != Results.end(); ++I) {
		if (I->IsError() && I->GetStatus() != OP_CANCELED)
			Errors.append(FormatError(*I));
	}

	if (bAsync) {
		foreach(const QString &Error, Errors)
			theGUI->OnLogMessage(Error, true);
	}
	else if (Errors.count() == 1)
		QMessageBox::warning(theGUI, tr("Sandboxie-Plus - Error"), Errors.first());
	else if (Errors.count() > 1) {
		CMultiErrorDialog Dialog(tr("Operation failed for %1 item(s).").arg(Errors.size()), Errors, theGUI);
		Dialog.exec();
	}
}

void CSandMan::OpenUrl(const QUrl& url)
{
	QString scheme = url.scheme();
	QString host = url.host();
	QString path = url.path();	
	QString query = url.query();

	if (scheme == "sbie") {
		if (path == "/check")
			m_pUpdater->CheckForUpdates(true);
		else if (path == "/installer")
			m_pUpdater->RunInstaller(false);
		else if (path == "/apply")
			m_pUpdater->ApplyUpdate(false);
		else if (path == "/cert")
			m_pUpdater->UpdateCert();
		else
			OpenUrl("https://sandboxie-plus.com/sandboxie" + path);
		return;
	}

	int iSandboxed = theConf->GetInt("Options/OpenUrlsSandboxed", 2);

	if (iSandboxed == 2)
	{
		bool bCheck = false;
		QString Message = tr("Do you want to open %1 in a sandboxed (yes) or unsandboxed (no) Web browser?").arg(url.toString());
		QDialogButtonBox::StandardButton Ret = CCheckableMessageBox::question(this, "Sandboxie-Plus", Message , tr("Remember choice for later."), 
			&bCheck, QDialogButtonBox::Yes | QDialogButtonBox::No | QDialogButtonBox::Cancel, QDialogButtonBox::Yes, QMessageBox::Question);
		if (Ret == QDialogButtonBox::Cancel) return;
		iSandboxed = Ret == QDialogButtonBox::Yes ? 1 : 0;
		if(bCheck) theConf->SetValue("Options/OpenUrlsSandboxed", iSandboxed);
	}

	if (iSandboxed) RunSandboxed(QStringList(url.toString()));
	else ShellExecute(MainWndHandle, NULL, url.toString().toStdWString().c_str(), NULL, NULL, SW_SHOWNORMAL);
}

bool CSandMan::IsWFPEnabled() const 
{ 
	return (g_FeatureFlags & CSbieAPI::eSbieFeatureWFP) != 0; 
}

QString CSandMan::GetVersion()
{
	QString Version = QString::number(VERSION_MJR) + "." + QString::number(VERSION_MIN) //.rightJustified(2, '0')
//#if VERSION_REV > 0 || VERSION_MJR == 0
		+ "." + QString::number(VERSION_REV)
//#endif
#if VERSION_UPD > 0
		+ QChar('a' + VERSION_UPD - 1)
#endif
		;
	return Version;
}

void CSandMan::SetUITheme()
{
	m_ThemeUpdatePending = false;


	bool bDark;
	int iDark = theConf->GetInt("Options/UseDarkTheme", 2);
	if (iDark == 2) {
		QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
		bDark = (settings.value("AppsUseLightTheme") == 0);
	} else
		bDark = (iDark == 1);
	
	if (bDark)
		QApplication::setPalette(m_DarkPalett);
	else
		QApplication::setPalette(m_DefaultPalett);
	m_DarkTheme = bDark;


	bool bFusion;
	int iFusion = theConf->GetInt("Options/UseFusionTheme", 2);
	if (iFusion == 2)
		bFusion = bDark;
	else 
		bFusion = (iFusion == 1);

	if (bFusion)
		QApplication::setStyle(QStyleFactory::create("Fusion"));
	else {
		int iViewMode = theConf->GetInt("Options/ViewMode", 1);
		QApplication::setStyle(QStyleFactory::create((bDark || iViewMode == 2) ? "Windows" : m_DefaultStyle));
	}
	m_FusionTheme = bFusion;


	CTreeItemModel::SetDarkMode(bDark);
	//CListItemModel::SetDarkMode(bDark); // not used
	CPopUpWindow::SetDarkMode(bDark);
	CPanelView::SetDarkMode(bDark);
	CFinder::SetDarkMode(bDark);


	QFont font = QApplication::font();
	double newFontSize = m_DefaultFontSize * theConf->GetInt("Options/FontScaling", 100) / 100.0;
	if (newFontSize != font.pointSizeF()) {
		font.setPointSizeF(newFontSize);
		QApplication::setFont(font);
	}

#if defined(Q_OS_WIN)
	foreach(QWidget * pWidget, QApplication::topLevelWidgets())
	{
		if (pWidget->isVisible())
			SetTitleTheme((HWND)pWidget->winId());
	}
#endif
}

void CSandMan::SetTitleTheme(const HWND& hwnd)
{
	static const int CurrentVersion = QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
												QSettings::NativeFormat).value("CurrentBuild").toInt();
	if (CurrentVersion < 17763) // Windows 10 1809 -
		return;
	
	HMODULE dwmapi = GetModuleHandle(L"dwmapi.dll");
	if (dwmapi)
	{
		typedef HRESULT(WINAPI* P_DwmSetWindowAttribute)(HWND, DWORD, LPCVOID, DWORD);
		P_DwmSetWindowAttribute pDwmSetWindowAttribute = reinterpret_cast<P_DwmSetWindowAttribute>(GetProcAddress(dwmapi, "DwmSetWindowAttribute"));
		if (pDwmSetWindowAttribute)
		{
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1
#define DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1 19
#endif
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
			BOOL bDark = m_DarkTheme;
			pDwmSetWindowAttribute(hwnd,
				CurrentVersion >= 18985 ? DWMWA_USE_IMMERSIVE_DARK_MODE : DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1,
				&bDark,
				sizeof(bDark));
		}
	}
}

void CSandMan::UpdateTheme()
{
	if (!m_ThemeUpdatePending)
	{
		m_ThemeUpdatePending = true;
		QTimer::singleShot(500, this, SLOT(SetUITheme()));
	}
}

void CSandMan::UpdateTitleTheme(const HWND& hwnd)
{
	SetTitleTheme(hwnd);
}

void CSandMan::LoadLanguage()
{
	QString Lang = theConf->GetString("Options/UiLanguage");
	if(Lang.isEmpty())
		Lang = QLocale::system().name();

	if (Lang.compare("native", Qt::CaseInsensitive) == 0)
		Lang.clear();

	m_LanguageId = LocaleNameToLCID(Lang.toStdWString().c_str(), 0);
	if (!m_LanguageId) 
		m_LanguageId = 1033; // default to English

	LoadLanguage(Lang, "sandman", 0);
	LoadLanguage(Lang, "qt", 1);

	QTreeViewEx::m_ResetColumns = tr("Reset Columns");
	CPanelView::m_CopyCell = tr("Copy Cell");
	CPanelView::m_CopyRow = tr("Copy Row");
	CPanelView::m_CopyPanel = tr("Copy Panel");
	CFinder::m_CaseInsensitive = tr("Case Sensitive");
	CFinder::m_RegExpStr = tr("RegExp");
	CFinder::m_Highlight = tr("Highlight");
	CFinder::m_CloseStr = tr("Close");
	CFinder::m_FindStr = tr("&Find ...");
	CFinder::m_AllColumns = tr("All columns");
}

void CSandMan::LoadLanguage(const QString& Lang, const QString& Module, int Index)
{
	qApp->removeTranslator(&m_Translator[Index]);

	if (Lang.isEmpty())
		return;
	
	QString LangAux = Lang; // Short version as fallback
	LangAux.truncate(LangAux.lastIndexOf('_'));

	C7zFileEngineHandler LangFS(QApplication::applicationDirPath() + "/translations.7z", "lang", this);

	QString LangDir;
	if (LangFS.IsOpen())
		LangDir = LangFS.Prefix() + "/";
	else
		LangDir = QApplication::applicationDirPath() + "/translations/";

	QString LangPath = LangDir + Module + "_";
	bool bAux = false;
	if (QFile::exists(LangPath + Lang + ".qm") || (bAux = QFile::exists(LangPath + LangAux + ".qm")))
	{
		if(m_Translator[Index].load(LangPath + (bAux ? LangAux : Lang) + ".qm", LangDir))
			qApp->installTranslator(&m_Translator[Index]);
	}
}

void CSandMan::OnHelp()
{
	//if (sender() == m_pSupport)
	//	QDesktopServices::openUrl(QUrl("https://sandboxie-plus.com/go.php?to=donate"));
	//else 
	if (sender() == m_pContribution)
		QDesktopServices::openUrl(QUrl("https://sandboxie-plus.com/go.php?to=sbie-contribute"));
	else if (sender() == m_pManual)
		QDesktopServices::openUrl(QUrl("https://sandboxie-plus.com/go.php?to=sbie-docs"));
	else if (sender() == m_pForum)
		QDesktopServices::openUrl(QUrl("https://sandboxie-plus.com/go.php?to=sbie-forum"));
	else
		QDesktopServices::openUrl(QUrl("https://sandboxie-plus.com/go.php?to=patreon"));
}

void CSandMan::OnAbout()
{
	if (sender() == m_pAbout)
	{
		QString AboutCaption = tr(
			"<h3>About Sandboxie-Plus</h3>"
			"<p>Version %1</p>"
			"<p>Copyright (c) 2020-2023 by DavidXanatos</p>"
		).arg(theGUI->GetVersion());

		QString CertInfo;
		if (!g_Certificate.isEmpty()) {
			CertInfo = tr("This copy of Sandboxie+ is certified for: %1").arg(GetArguments(g_Certificate, L'\n', L':').value("NAME"));
		} else {
			CertInfo = tr("Sandboxie+ is free for personal and non-commercial use.");
		}

		QString AboutText = tr(
			"Sandboxie-Plus is an open source continuation of Sandboxie.<br />"
			"Visit <a href=\"https://sandboxie-plus.com\">sandboxie-plus.com</a> for more information.<br />"
			"<br />"
			"%3<br />"
			"<br />"
			"Driver version: %1<br />"
			"Features: %2<br />"
			"<br />"
			"Icons from <a href=\"https://icons8.com\">icons8.com</a>"
		).arg(theAPI->GetVersion()).arg(theAPI->GetFeatureStr()).arg(CertInfo);

		QMessageBox *msgBox = new QMessageBox(this);
		msgBox->setAttribute(Qt::WA_DeleteOnClose);
		msgBox->setWindowTitle(tr("About Sandboxie-Plus"));
		msgBox->setText(AboutCaption);
		msgBox->setInformativeText(AboutText);

		QIcon ico(QLatin1String(":/SandMan.png"));
		msgBox->setIconPixmap(ico.pixmap(128, 128));

		SafeExec(msgBox);
	}
	else if (sender() == m_pAboutQt)
		QMessageBox::aboutQt(this);
}

// Make sure that QPlatformTheme strings won't be marked as vanished in all .ts files, even after running lupdate

static const char* platform_strings[] = {
QT_TRANSLATE_NOOP("QPlatformTheme", "OK"),
QT_TRANSLATE_NOOP("QPlatformTheme", "Apply"),
QT_TRANSLATE_NOOP("QPlatformTheme", "Cancel"),
QT_TRANSLATE_NOOP("QPlatformTheme", "&Yes"),
QT_TRANSLATE_NOOP("QPlatformTheme", "&No"),
};

// Make sure that CSandBox strings won't be marked as vanished in all .ts files, even after running lupdate

static const char* CSandBox_strings[] = {
QT_TRANSLATE_NOOP("CSandBox", "Waiting for folder: %1"),
QT_TRANSLATE_NOOP("CSandBox", "Deleting folder: %1"),
QT_TRANSLATE_NOOP("CSandBox", "Merging folders: %1 &gt;&gt; %2"),
QT_TRANSLATE_NOOP("CSandBox", "Finishing Snapshot Merge..."),
};


#include "SbieFindWnd.cpp"
