#include "stdafx.h"
#include "SandMan.h"
#include "../MiscHelpers/Common/Common.h"
#include "../MiscHelpers/Common/ExitDialog.h"
#include "../MiscHelpers/Common/SortFilterProxyModel.h"
#include "Views/SbieView.h"
#include "../MiscHelpers/Common/CheckableMessageBox.h"
#include <QWinEventNotifier>
#include "ApiLog.h"
#include "./Dialogs/MultiErrorDialog.h"
#include "../QSbieAPI/SbieUtils.h"

CSbiePlusAPI* theAPI = NULL;

#if defined(Q_OS_WIN)
#include <wtypes.h>
#include <QAbstractNativeEventFilter>
#include <dbt.h>

//BOOLEAN OnWM_Notify(NMHDR *Header, LRESULT *Result);

class CNativeEventFilter : public QAbstractNativeEventFilter
{
public:
	virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *result)
	{
		if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG")
		{
			MSG *msg = static_cast<MSG *>(message);

			//if(msg->message != 275 && msg->message != 1025)
			//	qDebug() << msg->message;

			if (msg->message == WM_NOTIFY)
			{
				//LRESULT ret;
				//if (OnWM_Notify((NMHDR*)msg->lParam, &ret))
				//	*result = ret;
				return true;
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
		}
		return false;
	}
};

HWND MainWndHandle = NULL;
#endif

CSandMan* theGUI = NULL;

CSandMan::CSandMan(QWidget *parent)
	: QMainWindow(parent)
{
#if defined(Q_OS_WIN)
	MainWndHandle = (HWND)QWidget::winId();

	QApplication::instance()->installNativeEventFilter(new CNativeEventFilter);
#endif

	theGUI = this;

	m_bExit = false;

	theAPI = new CSbiePlusAPI(this);
	connect(theAPI, SIGNAL(StatusChanged()), this, SLOT(OnStatusChanged()));

	QString appTitle = tr("Sandboxie-Plus v%1").arg(GetVersion());
	this->setWindowTitle(appTitle);

	m_ApiLog = NULL;

	m_bConnectPending = false;
	m_bStopPending = false;

	m_pMainWidget = new QWidget();
	m_pMainLayout = new QVBoxLayout(m_pMainWidget);
	m_pMainLayout->setMargin(2);
	m_pMainLayout->setSpacing(0);
	this->setCentralWidget(m_pMainWidget);

	m_pToolBar = new QToolBar();
	m_pMainLayout->insertWidget(0, m_pToolBar);


	m_pLogSplitter = new QSplitter();
	m_pLogSplitter->setOrientation(Qt::Vertical);
	m_pMainLayout->addWidget(m_pLogSplitter);

	m_pPanelSplitter = new QSplitter();
	m_pPanelSplitter->setOrientation(Qt::Horizontal);
	m_pLogSplitter->addWidget(m_pPanelSplitter);

	/*
	// Box Tree
	m_pBoxModel = new CSbieModel();
	m_pBoxModel->SetTree(true);
	m_pBoxModel->SetUseIcons(true);

	m_pSortProxy = new CSortFilterProxyModel(false, this);
	m_pSortProxy->setSortRole(Qt::EditRole);
	m_pSortProxy->setSourceModel(m_pBoxModel);
	m_pSortProxy->setDynamicSortFilter(true);

	m_pBoxTree = new QTreeViewEx();
	//m_pBoxTree->setItemDelegate(theGUI->GetItemDelegate());

	m_pBoxTree->setModel(m_pSortProxy);

	m_pBoxTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
#ifdef WIN32
	QStyle* pStyle = QStyleFactory::create("windows");
	m_pBoxTree->setStyle(pStyle);
#endif
	m_pBoxTree->setSortingEnabled(true);

	m_pBoxTree->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pBoxTree, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnMenu(const QPoint &)));

	//connect(theGUI, SIGNAL(ReloadPanels()), m_pWindowModel, SLOT(Clear()));

	m_pBoxTree->setColumnReset(2);
	connect(m_pBoxTree, SIGNAL(ResetColumns()), this, SLOT(OnResetColumns()));
	connect(m_pBoxTree, SIGNAL(ColumnChanged(int, bool)), this, SLOT(OnColumnsChanged()));

	//m_pSplitter->addWidget(CFinder::AddFinder(m_pBoxTree, m_pSortProxy));
	//m_pSplitter->setCollapsible(0, false);
	// 

	//connect(m_pBoxTree, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnItemSelected(const QModelIndex&)));
	//connect(m_pBoxTree->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(OnItemSelected(QModelIndex)));

	m_pPanelSplitter->addWidget(m_pBoxTree);
	*/

	m_pBoxView = new CSbieView();
	m_pPanelSplitter->addWidget(m_pBoxView);

	connect(m_pBoxView->GetTree()->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(OnSelectionChanged()));

	//m_pPanelSplitter->addWidget();

	m_pLogTabs = new QTabWidget();
	m_pLogSplitter->addWidget(m_pLogTabs);

	// Message Log
	m_pMessageLog = new CPanelWidgetEx();

	//m_pMessageLog->GetView()->setItemDelegate(theGUI->GetItemDelegate());
	((QTreeWidgetEx*)m_pMessageLog->GetView())->setHeaderLabels(tr("Time|Message").split("|"));

	m_pMessageLog->GetView()->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_pMessageLog->GetView()->setSortingEnabled(false);

	
	m_pLogTabs->addTab(m_pMessageLog, tr("Sbie Messages"));
	//

	// Res Log
	m_pResMonModel = new CResMonModel();
	//m_pResMonModel->SetUseIcons(true);

	m_pResourceLog = new CPanelViewImpl<CResMonModel>(m_pResMonModel);

	//m_pResourceLog->GetView()->setItemDelegate(theGUI->GetItemDelegate());

	m_pResourceLog->GetView()->setSelectionMode(QAbstractItemView::ExtendedSelection);

	m_pLogTabs->addTab(m_pResourceLog, tr("Resource Monitor"));
	//

	// Api Log
	m_pApiLog = new CPanelWidgetEx();

	//m_pApiLog->GetView()->setItemDelegate(theGUI->GetItemDelegate());
	((QTreeWidgetEx*)m_pApiLog->GetView())->setHeaderLabels(tr("Time|Entry").split("|"));

	m_pApiLog->GetView()->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_pApiLog->GetView()->setSortingEnabled(false);

	m_pLogTabs->addTab(m_pApiLog, tr("Api Call Log"));
	m_pApiLog->setEnabled(false);
	//

	connect(menuBar(), SIGNAL(hovered(QAction*)), this, SLOT(OnMenuHover(QAction*)));

	m_pMenuFile = menuBar()->addMenu(tr("&Sandbox"));
		m_pNew = m_pMenuFile->addAction(QIcon(":/Actions/NewBox"), tr("Create New Box"), this, SLOT(OnNewBox()));
		m_pMenuFile->addSeparator();
		m_pEmptyAll = m_pMenuFile->addAction(QIcon(":/Actions/EmptyAll"), tr("Terminate All Processes"), this, SLOT(OnEmptyAll()));
		m_pMenuFile->addSeparator();
		m_pMaintenance = m_pMenuFile->addMenu(QIcon(":/Actions/Maintenance"), tr("&Maintenance"));
			m_pConnect = m_pMaintenance->addAction(QIcon(":/Actions/Connect"), tr("Connect"), this, SLOT(OnMaintenance()));
			m_pDisconnect = m_pMaintenance->addAction(QIcon(":/Actions/Disconnect"), tr("Disconnect"), this, SLOT(OnMaintenance()));
			m_pMaintenance->addSeparator();
			m_pStopAll = m_pMaintenance->addAction(QIcon(":/Actions/Stop"), tr("Stop All"), this, SLOT(OnMaintenance()));
			m_pMaintenance->addSeparator();
			m_pMaintenanceItems = m_pMaintenance->addMenu(QIcon(":/Actions/Advanced"), tr("&Advanced"));
				m_pInstallDrv = m_pMaintenanceItems->addAction(tr("Install Driver"), this, SLOT(OnMaintenance()));
				m_pStartDrv = m_pMaintenanceItems->addAction(tr("Start Driver"), this, SLOT(OnMaintenance()));
				m_pStopDrv = m_pMaintenanceItems->addAction(tr("Stop Driver"), this, SLOT(OnMaintenance()));
				m_pUninstallDrv = m_pMaintenanceItems->addAction(tr("Uninstall Driver"), this, SLOT(OnMaintenance()));
				m_pMaintenanceItems->addSeparator();
				m_pInstallSvc = m_pMaintenanceItems->addAction(tr("Install Service"), this, SLOT(OnMaintenance()));
				m_pStartSvc = m_pMaintenanceItems->addAction(tr("Start Service"), this, SLOT(OnMaintenance()));
				m_pStopSvc = m_pMaintenanceItems->addAction(tr("Stop Service"), this, SLOT(OnMaintenance()));
				m_pUninstallSvc = m_pMaintenanceItems->addAction(tr("Uninstall Service"), this, SLOT(OnMaintenance()));
				
		m_pMenuFile->addSeparator();
		m_pExit = m_pMenuFile->addAction(QIcon(":/Actions/Exit"), tr("Exit"), this, SLOT(OnExit()));


	m_pMenuView = menuBar()->addMenu(tr("&View"));
		m_pCleanUpMenu = m_pMenuView->addMenu(QIcon(":/Actions/Clean"), tr("Clean Up"));
			m_pCleanUpProcesses = m_pCleanUpMenu->addAction(tr("Cleanup Processes"), this, SLOT(OnCleanUp()));
			m_pCleanUpMenu->addSeparator();
			m_pCleanUpMsgLog = m_pCleanUpMenu->addAction(tr("Cleanup Message Log"), this, SLOT(OnCleanUp()));
			m_pCleanUpResLog = m_pCleanUpMenu->addAction(tr("Cleanup Resource Log"), this, SLOT(OnCleanUp()));
			m_pCleanUpApiLog = m_pCleanUpMenu->addAction(tr("Cleanup Api Call Log"), this, SLOT(OnCleanUp()));

		m_pKeepTerminated = m_pMenuView->addAction(QIcon(":/Actions/Keep"), tr("Keep terminated"), this, SLOT(OnSetKeep()));
		m_pKeepTerminated->setCheckable(true);

	m_pMenuOptions = menuBar()->addMenu(tr("&Options"));
		m_pEditIni = m_pMenuOptions->addAction(QIcon(":/Actions/EditIni"), tr("Edit ini file"), this, SLOT(OnEditIni()));
		m_pReloadIni = m_pMenuOptions->addAction(QIcon(":/Actions/ReloadIni"), tr("Reload ini file"), this, SLOT(OnReloadIni()));
		m_pMenuOptions->addSeparator();
		m_pEnableMonitoring = m_pMenuOptions->addAction(QIcon(":/Actions/SetLogging"), tr("Resource Logging"), this, SLOT(OnSetMonitoring()));
		m_pEnableMonitoring->setCheckable(true);
		m_pMenuOptions->addSeparator();
		m_pEnableLogging = m_pMenuOptions->addAction(QIcon(":/Actions/LogAPI"), tr("API Call Logging"), this, SLOT(OnSetLogging()));
		m_pEnableLogging->setCheckable(true);
		

	m_pMenuHelp = menuBar()->addMenu(tr("&Help"));
		m_pSupport = m_pMenuHelp->addAction(tr("Support Sandboxie-Plus on Patreon"), this, SLOT(OnAbout()));
		m_pMenuHelp->addSeparator();
		m_pAboutQt = m_pMenuHelp->addAction(tr("About the Qt Framework"), this, SLOT(OnAbout()));
		//m_pMenuHelp->addSeparator();
		m_pAbout = m_pMenuHelp->addAction(QIcon(":/SandMan.png"), tr("About Sandboxie-Plus"), this, SLOT(OnAbout()));



	//m_pToolBar->addAction(m_pMenuNew);
	//m_pToolBar->addAction(m_pMenuEmptyAll);
	//m_pToolBar->addSeparator();
	m_pToolBar->addAction(m_pKeepTerminated);
	//m_pToolBar->addAction(m_pCleanUp);

	m_pCleanUpButton = new QToolButton();
	m_pCleanUpButton->setIcon(MakeActionIcon(":/Actions/Clean"));
	m_pCleanUpButton->setToolTip(tr("Cleanup"));
	m_pCleanUpButton->setPopupMode(QToolButton::MenuButtonPopup);
	m_pCleanUpButton->setMenu(m_pCleanUpMenu);
	//QObject::connect(m_pCleanUpButton, SIGNAL(triggered(QAction*)), , SLOT());
	QObject::connect(m_pCleanUpButton, SIGNAL(pressed()), this, SLOT(OnCleanUp()));
	m_pToolBar->addWidget(m_pCleanUpButton);


	m_pToolBar->addSeparator();
	m_pToolBar->addAction(m_pEditIni);
	m_pToolBar->addSeparator();
	m_pToolBar->addAction(m_pEnableMonitoring);
	m_pToolBar->addAction(m_pEnableLogging);
	m_pToolBar->addSeparator();
	

	/*QWidget* pSpacer = new QWidget();
	pSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_pToolBar->addWidget(pSpacer);

	//m_pToolBar->addAction(m_pMenuElevate);

	m_pToolBar->addSeparator();
	m_pToolBar->addWidget(new QLabel("        "));
	QLabel* pSupport = new QLabel("<a href=\"https://www.patreon.com/DavidXanatos\">Support Sandboxie-Plus on Patreon</a>");
	pSupport->setTextInteractionFlags(Qt::TextBrowserInteraction);
	connect(pSupport, SIGNAL(linkActivated(const QString&)), this, SLOT(OnAbout()));
	m_pToolBar->addWidget(pSupport);
	m_pToolBar->addWidget(new QLabel("        "));*/



	QIcon Icon;
	Icon.addFile(":/SandMan.png");
	m_pTrayIcon = new QSystemTrayIcon(Icon, this);
	m_pTrayIcon->setToolTip("Sandboxie-Plus");
	connect(m_pTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(OnSysTray(QSystemTrayIcon::ActivationReason)));
	m_bIconEmpty = false;

	m_pTrayMenu = new QMenu();
	m_pTrayMenu->addSeparator();
	m_pTrayMenu->addAction(m_pExit);

	bool bAutoRun = QApplication::arguments().contains("-autorun");

	m_pTrayIcon->show(); // Note: qt bug; without a first show hide does not work :/
	if(!bAutoRun && !theConf->GetBool("SysTray/Show", true))
		m_pTrayIcon->hide();

	restoreGeometry(theConf->GetBlob("MainWindow/Window_Geometry"));
	//m_pBoxTree->restoreState(theConf->GetBlob("GUI/BoxTree_Columns"));
	m_pMessageLog->GetView()->header()->restoreState(theConf->GetBlob("GUI/LogList_Columns"));
	QByteArray Columns = theConf->GetBlob("GUI/ResMonList_Columns");
	if (!Columns.isEmpty())
		((QTreeViewEx*)m_pResourceLog->GetView())->OnResetColumns();
	else
		((QTreeViewEx*)m_pResourceLog->GetView())->restoreState(Columns);
	m_pApiLog->GetView()->header()->restoreState(theConf->GetBlob("GUI/ApiLogList_Columns"));
	m_pLogSplitter->restoreState(theConf->GetBlob("MainWindow/Log_Splitter"));
	m_pPanelSplitter->restoreState(theConf->GetBlob("MainWindow/Panel_Splitter"));
	m_pLogTabs->setCurrentIndex(theConf->GetInt("GUI/LogTab", 0));
	
	if (theConf->GetBool("Options/NoStatusBar", false))
		statusBar()->hide();
	else if (theConf->GetBool("Options/NoSizeGrip", false))
		statusBar()->setSizeGripEnabled(false);

	bool bIsMonitoring = theAPI->IsMonitoring();
	m_pResourceLog->setEnabled(bIsMonitoring);
	m_pEnableMonitoring->setChecked(bIsMonitoring);

	m_pKeepTerminated->setChecked(theConf->GetBool("Options/KeepTerminated"));

	m_pProgressDialog = new CProgressDialog("Maintenance operation progress...", this);
	m_pProgressDialog->setWindowModality(Qt::ApplicationModal);

	connect(theAPI, SIGNAL(LogMessage(const QString&)), this, SLOT(OnLogMessage(const QString&)));

	if (CSbieUtils::IsRunning(CSbieUtils::eAll) || theConf->GetBool("Options/StartIfStopped", true))
		ConnectSbie();

	m_uTimerID = startTimer(250);
}

CSandMan::~CSandMan()
{
	if(m_pEnableMonitoring->isChecked())
		theAPI->EnableMonitor(false);

	killTimer(m_uTimerID);

	m_pTrayIcon->hide();

	theConf->SetBlob("MainWindow/Window_Geometry", saveGeometry());
	//theConf->SetBlob("GUI/BoxTree_Columns", m_pBoxTree->saveState());
	theConf->SetBlob("GUI/LogList_Columns", m_pMessageLog->GetView()->header()->saveState());
	theConf->SetBlob("GUI/ResMonList_Columns", m_pResourceLog->GetView()->header()->saveState());
	theConf->SetBlob("GUI/ApiLogList_Columns", m_pApiLog->GetView()->header()->saveState());
	theConf->SetBlob("MainWindow/Log_Splitter", m_pLogSplitter->saveState());
	theConf->SetBlob("MainWindow/Panel_Splitter", m_pPanelSplitter->saveState());
	theConf->SetValue("GUI/LogTab", m_pLogTabs->currentIndex());

	theAPI = NULL;

	theGUI = NULL;
}

void CSandMan::OnExit()
{
	m_bExit = true;
	close();
}

void CSandMan::closeEvent(QCloseEvent *e)
{
	if (!m_bExit)
	{
		QString OnClose = theConf->GetString("Options/OnClose", "ToTray");
		if (m_pTrayIcon->isVisible() && OnClose.compare("ToTray", Qt::CaseInsensitive) == 0)
		{
			hide();

			e->ignore();
			return;
		}
		else if(OnClose.compare("Prompt", Qt::CaseInsensitive) == 0)
		{
			CExitDialog ExitDialog(tr("Do you want to close Sandboxie Manager?"));
			if (!ExitDialog.exec())
			{
				e->ignore();
				return;
			}
		}
	}

	if (IsFullyPortable() && theAPI->IsConnected())
	{
		int PortableStop = theConf->GetInt("Options/PortableStop", -1);
		if (PortableStop == -1)
		{
			bool State = false;
			PortableStop = CCheckableMessageBox::question(this, "Sandboxie-Plus", tr("Sandboxie-Plus was running in portable mode, now it has to clean up the created services, this will prompt for administrative privileges.")
				, tr("Don't show this message again."), &State, QDialogButtonBox::Ok | QDialogButtonBox::Cancel, QDialogButtonBox::Ok, QMessageBox::Information) == QDialogButtonBox::Ok ? 1 : 0;

			if (!PortableStop)
			{
				e->ignore();
				return;
			}

			if (State)
				theConf->SetValue("Options/PortableStop", PortableStop);
		}

		StopSbie(true);
	}

	QApplication::quit();
}

bool CSandMan::IsFullyPortable()
{
	QString SbiePath = theAPI->GetSbiePath();
	QString AppPath = QApplication::applicationDirPath().replace("/", "\\");
	return (theConf->IsPortable() && SbiePath.compare(AppPath, Qt::CaseInsensitive) == 0);
}

void CSandMan::OnMessage(const QString& Message)
{
	if (Message == "ShowWnd")
	{
		if (!isVisible())
			show();
		setWindowState(Qt::WindowActive);
		SetForegroundWindow(MainWndHandle);
	}
	else if (Message.left(6) == "Status")
	{
		QString Status = Message.mid(7);
		if (Status != "OK")
		{
			if(m_bStopPending)
				QMessageBox::warning(NULL, tr("Sandboxie-Plus - Error"), tr("Failed to stop all sandboxie components"));
			else if(m_bConnectPending)
				QMessageBox::warning(NULL, tr("Sandboxie-Plus - Error"), tr("Failed to start required sandboxie components"));

			OnLogMessage(tr("Maintenance operation %1").arg(Status));
			CheckResults(QList<SB_STATUS>() << SB_ERR(Status));
		}
		else
		{
			OnLogMessage(tr("Maintenance operation Successful"));
			if (m_bConnectPending)
				theAPI->Connect(true);
		}
		m_pProgressDialog->hide();
		m_bConnectPending = false;
		m_bStopPending = false;
	}
}

void CSandMan::timerEvent(QTimerEvent* pEvent)
{
	if (pEvent->timerId() != m_uTimerID)
		return;

	if (theAPI->IsConnected())
	{
		theAPI->ReloadBoxes();
		theAPI->UpdateProcesses(m_pKeepTerminated->isChecked());
	}

	m_pBoxView->Refresh();

	if (m_bIconEmpty != (theAPI->TotalProcesses() == 0))
	{
		m_bIconEmpty = (theAPI->TotalProcesses() == 0);
		QIcon Icon;
		Icon.addFile(m_bIconEmpty ? ":/SandMan2.png" : ":/SandMan.png");
		m_pTrayIcon->setIcon(Icon);
	}

	/*QList<QVariant> Added = m_pBoxModel->Sync(theAPI->GetAllBoxes());

	if (m_pBoxModel->IsTree())
	{
		QTimer::singleShot(100, this, [this, Added]() {
			foreach(const QVariant ID, Added) {
				m_pBoxTree->expand(m_pSortProxy->mapFromSource(m_pBoxModel->FindIndex(ID)));
			}
		});
	}*/

	OnSelectionChanged();
}

void CSandMan::OnSelectionChanged()
{
	QList<CBoxedProcessPtr>	Processes = m_pBoxView->GetSelectedProcesses();
	/*if (Processes.isEmpty())
	{
		QList<CSandBoxPtr>Boxes = m_pBoxView->GetSelectedBoxes();
		foreach(const CSandBoxPtr& pBox, Boxes)
			Processes.append(pBox->GetProcessList().values());
	}*/

	QSet<quint64> Pids;
	foreach(const CBoxedProcessPtr& pProcess, Processes)
	{
		Pids.insert(pProcess->GetProcessId());
	}

	QList<CResLogEntryPtr> ResourceLog = theAPI->GetResLog();

	m_pResMonModel->Sync(ResourceLog, Pids);
}

void CSandMan::OnStatusChanged()
{
	QString appTitle = tr("Sandboxie-Plus v%1").arg(GetVersion());
	if (theAPI->IsConnected())
	{
		appTitle.append(tr("   -   Driver: v%1").arg(theAPI->GetVersion()));
		//appTitle.append(tr(" - %1").arg(theAPI->GetIniPath()));

		if (theAPI->GetAllBoxes().count() == 0) {
			OnLogMessage(tr("No sandboxes found; creating: %1").arg("DefaultBox"));
			theAPI->CreateBox("DefaultBox");
		}

		if(IsFullyPortable())
			appTitle.append(tr("   -   Portable"));

		OnLogMessage(tr("Sbie Directory: %1").arg(theAPI->GetSbiePath()));
		OnLogMessage(tr("Loaded Config: %1").arg(theAPI->GetIniPath()));

		if (theConf->GetBool("Options/WatchIni", true))
			theAPI->WatchIni(true);
	}
	else
	{
		appTitle.append(tr("   -   Driver NOT connected").arg(theAPI->GetVersion()));

		theAPI->WatchIni(false);
	}
	this->setWindowTitle(appTitle);
}

void CSandMan::OnMenuHover(QAction* action)
{
	//if (!menuBar()->actions().contains(action))
	//	return; // ignore sub menus


	if (menuBar()->actions().at(0) == action)
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
}

void CSandMan::OnLogMessage(const QString& Message)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem(); // Time|Message
	pItem->setText(0, QDateTime::currentDateTime().toString("hh:mm:ss.zzz"));
	pItem->setText(1, Message);
	m_pMessageLog->GetTree()->addTopLevelItem(pItem);

	m_pMessageLog->GetView()->verticalScrollBar()->setValue(m_pMessageLog->GetView()->verticalScrollBar()->maximum());

	statusBar()->showMessage(Message);
}

void CSandMan::OnApiLogEntry(const QString& Message)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem(); // Time|Message
	pItem->setText(0, QDateTime::currentDateTime().toString("hh:mm:ss.zzz"));
	pItem->setText(1, Message);
	m_pApiLog->GetTree()->addTopLevelItem(pItem);

	m_pApiLog->GetView()->verticalScrollBar()->setValue(m_pApiLog->GetView()->verticalScrollBar()->maximum());
}
/*
void CSandMan::OnResetColumns()
{
	for (int i = 0; i < m_pBoxModel->columnCount(); i++)
		m_pBoxTree->SetColumnHidden(i, false);
}

void CSandMan::OnColumnsChanged()
{
	m_pBoxModel->Sync(theAPI->GetAllBoxes());
}

void CSandMan::OnMenu(const QPoint& Point)
{
}*/

void CSandMan::OnNewBox()
{
	QString Value = QInputDialog::getText(this, "Sandboxie-Plus", "Please enter a name for the new Sandbox.", QLineEdit::Normal, "NewBox");
	if (Value.isEmpty())
		return;
	theAPI->CreateBox(Value);
}

void CSandMan::OnEmptyAll()
{
	theAPI->TerminateAll();
}

SB_STATUS CSandMan::ConnectSbie()
{
	SB_STATUS Status;

	if (!CSbieUtils::IsRunning(CSbieUtils::eAll)) 
	{
		if (!CSbieUtils::IsInstalled(CSbieUtils::eAll))
		{
			int PortableStart = theConf->GetInt("Options/PortableStart", -1);
			if (PortableStart == -1)
			{
				bool State = false;
				PortableStart = CCheckableMessageBox::question(this, "Sandboxie-Plus", tr("Sandboxie-Plus was started in portable mode and it needs to create nececery services, this will prompt for administrative privileges.")
					, tr("Don't show this message again."), &State, QDialogButtonBox::Ok | QDialogButtonBox::Cancel, QDialogButtonBox::Ok, QMessageBox::Information) == QDialogButtonBox::Ok ? 1 : 0;

				if (State)
					theConf->SetValue("Options/PortableStart", PortableStart);
			}

			if(!PortableStart)
				return SB_OK;
		}

		Status = CSbieUtils::Start(CSbieUtils::eAll);
	}

	if (Status.GetStatus() == OP_ASYNC)
		m_bConnectPending = true;
	else if (!Status.IsError())
		Status = theAPI->Connect(true);

	return Status;
}

SB_STATUS CSandMan::DisconnectSbie()
{
	return theAPI->Disconnect();
}

SB_STATUS CSandMan::StopSbie(bool andRemove)
{
	SB_STATUS Status;

	if (theAPI->IsConnected()) {
		Status = theAPI->TerminateAll();
		theAPI->Disconnect();
	}
	if (!Status.IsError()) {
		if(andRemove)
			Status = CSbieUtils::Uninstall(CSbieUtils::eAll); // it stops it first ofcause
		else
			Status = CSbieUtils::Stop(CSbieUtils::eAll);
		if (Status.GetStatus() == OP_ASYNC)
			m_bStopPending = true;
	}

	return Status;
}

void CSandMan::OnMaintenance()
{
	SB_STATUS Status;
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
	else if(sender() == m_pStartSvc)
		Status = CSbieUtils::Start(CSbieUtils::eService);
	else if(sender() == m_pStopSvc)
		Status = CSbieUtils::Stop(CSbieUtils::eService);
	else if (sender() == m_pUninstallSvc)
		Status = CSbieUtils::Uninstall(CSbieUtils::eService);

	if (Status.GetStatus() == OP_ASYNC) {
		statusBar()->showMessage(tr("Executing maintenance operation, please wait..."));
		m_pProgressDialog->show();
		return;
	}

	CheckResults(QList<SB_STATUS>() << Status);
}

void CSandMan::OnCleanUp()
{
	if (sender() == m_pCleanUpMsgLog || sender() == m_pCleanUpButton)
		m_pMessageLog->GetTree()->clear();
	
	if (sender() == m_pCleanUpResLog || sender() == m_pCleanUpButton)
		theAPI->ClearResLog();
	
	if (sender() == m_pCleanUpApiLog || sender() == m_pCleanUpButton)
		m_pApiLog->GetTree()->clear();
	
	if (sender() == m_pCleanUpProcesses || sender() == m_pCleanUpButton)
		theAPI->UpdateProcesses(false);
}

void CSandMan::OnSetKeep()
{
	theConf->SetValue("Options/KeepTerminated", m_pKeepTerminated->isChecked());
}

void CSandMan::OnEditIni()
{
	if (theConf->GetBool("Options/NoEditInfo", true))
	{
		bool State = false;
		CCheckableMessageBox::question(this, "Sandboxie-Plus", tr("The changes will be applyed automatically as soon as the editor is closed.")
			, tr("Don't show this message again."), &State, QDialogButtonBox::Ok, QDialogButtonBox::Ok, QMessageBox::Information);

		if (State)
			theConf->SetValue("Options/NoEditInfo", false);
	}

	wstring IniPath = theAPI->GetIniPath().toStdWString();

	SHELLEXECUTEINFO si = { 0 };
	si.cbSize = sizeof(SHELLEXECUTEINFO);
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.hwnd = NULL;
	si.lpVerb = L"runas";
	si.lpFile = L"notepad.exe";
	si.lpParameters = IniPath.c_str();
	si.lpDirectory = NULL;
	si.nShow = SW_SHOW;
	si.hInstApp = NULL;
	ShellExecuteEx(&si);
	//WaitForSingleObject(si.hProcess, INFINITE);
	//CloseHandle(si.hProcess);

	if (theConf->GetBool("Options/WatchIni", true))
		return; // if the ini s watched dont double reload
	
	QWinEventNotifier* processFinishedNotifier = new QWinEventNotifier(si.hProcess);
	processFinishedNotifier->setEnabled(true);
	connect(processFinishedNotifier, &QWinEventNotifier::activated, this, [processFinishedNotifier, this, si]() {
		processFinishedNotifier->setEnabled(false);
		processFinishedNotifier->deleteLater();
		this->OnReloadIni();
		CloseHandle(si.hProcess);
	});
}

void CSandMan::OnReloadIni()
{
	theAPI->ReloadConfig();
}

void CSandMan::OnSetMonitoring()
{
	theAPI->EnableMonitor(m_pEnableMonitoring->isChecked());
	m_pResourceLog->setEnabled(m_pEnableMonitoring->isChecked());
}

void CSandMan::OnSetLogging()
{
	if (m_pEnableLogging->isChecked())
	{
		if (theConf->GetBool("Options/ApiLogInfo", true))
		{
			QString Message = tr("To use API logging you must first set up the LogApiDll from https://github.com/sandboxie-plus/LogApiDll with one or more sand boxes.\n"
				"Please download the latest release and set it up with the sandboxie.ini as instructed in the README.md of the project.");

			bool State = false;
			CCheckableMessageBox::question(this, "Sandboxie-Plus", Message
				, tr("Don't show this message again."), &State, QDialogButtonBox::Ok, QDialogButtonBox::Ok, QMessageBox::Information);

			if (State)
				theConf->SetValue("Options/ApiLogInfo", false);
		}

		if (!m_ApiLog) {
			m_ApiLog = new CApiLog();
			connect(m_ApiLog, SIGNAL(ApiLogEntry(const QString&)), this, SLOT(OnApiLogEntry(const QString&)));
			m_pApiLog->setEnabled(true);
		}
	}
	else
	{
		if (m_ApiLog) {
			m_pApiLog->setEnabled(false);
			m_ApiLog->deleteLater();
			m_ApiLog = NULL;
		}
	}
}

void CSandMan::CheckResults(QList<SB_STATUS> Results)
{
	for (QList<SB_STATUS>::iterator I = Results.begin(); I != Results.end(); )
	{
		if (!I->IsError())
			I = Results.erase(I);
		else
			I++;
	}

	if (Results.count() == 1)
		QMessageBox::warning(NULL, tr("Sandboxie-Plus - Error"), Results[0].GetText());
	else if (Results.count() > 1)
	{
		CMultiErrorDialog Dialog(tr("Operation failed for %1 item(s).").arg(Results.size()), Results);
		Dialog.exec();
	}
}

void CSandMan::OnSysTray(QSystemTrayIcon::ActivationReason Reason)
{
	static bool TriggerSet = false;
	static bool NullifyTrigger = false;
	switch(Reason)
	{
		case QSystemTrayIcon::Context:
			m_pTrayMenu->popup(QCursor::pos());	
			break;
		case QSystemTrayIcon::DoubleClick:
			if (isVisible())
			{
				if(TriggerSet)
					NullifyTrigger = true;
				hide();
				break;
			}
			show();
		case QSystemTrayIcon::Trigger:
#ifdef WIN32
			if (isVisible() && !TriggerSet)
			{
				TriggerSet = true;
				QTimer::singleShot(100, [this]() { 
					TriggerSet = false;
					if (NullifyTrigger) {
						NullifyTrigger = false;
						return;
					}
					setWindowState(Qt::WindowActive);
					SetForegroundWindow(MainWndHandle);
				} );
			}
#endif
			break;
	}
}


QString CSandMan::GetVersion()
{
	QString Version = QString::number(VERSION_MJR) + "." + QString::number(VERSION_MIN) //.rightJustified(2, '0')
#if VERSION_REV > 0
		+ "." + QString::number(VERSION_REV)
#endif
#if VERSION_UPD > 0
		+ QString('a' + VERSION_UPD - 1)
#endif
		;
	return Version;
}

void CSandMan::OnAbout()
{
	if (sender() == m_pAbout)
	{
		QString AboutCaption = tr(
			"<h3>About Sandboxie-Plus</h3>"
			"<p>Version %1</p>"
			"<p>by DavidXanatos</p>"
			"<p>Copyright (c) 2020</p>"
		).arg(GetVersion());
		QString AboutText = tr(
			"<p>Sandboxie-Plus is a powerfull sandboxing and application virtualization tool. Based on the well known <a href=\"https://www.sandboxie.com\">Sandboxie</a>.</p>"
			"<p></p>"
			"<p>Visit <a href=\"https://github.com/Sandboxie-Plus\">Sandboxie-Plus on github</a> for more information.</p>"
			"<p></p>"
			"<p></p>"
			"<p></p>"
			"<p>Icons from <a href=\"https://icons8.com\">icons8.com</a></p>"
			"<p></p>"
		);
		QMessageBox *msgBox = new QMessageBox(this);
		msgBox->setAttribute(Qt::WA_DeleteOnClose);
		msgBox->setWindowTitle(tr("About Sandboxie-Plus"));
		msgBox->setText(AboutCaption);
		msgBox->setInformativeText(AboutText);

		QIcon ico(QLatin1String(":/SandMan.png"));
		msgBox->setIconPixmap(ico.pixmap(128, 128));

		msgBox->exec();
	}
	else if (sender() == m_pAboutQt)
		QMessageBox::aboutQt(this);
	else
		QDesktopServices::openUrl(QUrl("https://www.patreon.com/DavidXanatos"));
}

//////////////////////////////////////////////////////////////////////////////////////////
//
/*
#include <windows.h>
#include <shellapi.h>

#define RFF_NOBROWSE 0x0001
#define RFF_NODEFAULT 0x0002
#define RFF_CALCDIRECTORY 0x0004
#define RFF_NOLABEL 0x0008
#define RFF_NOSEPARATEMEM 0x0020
#define RFF_OPTRUNAS 0x0040

#define RFN_VALIDATE (-510)
#define RFN_LIMITEDRUNAS (-511)

#define RF_OK 0x0000
#define RF_CANCEL 0x0001
#define RF_RETRY 0x0002

typedef struct _NMRUNFILEDLGW
{
	NMHDR hdr;
	PWSTR lpszFile;
	PWSTR lpszDirectory;
	UINT ShowCmd;
} NMRUNFILEDLGW, *LPNMRUNFILEDLGW, *PNMRUNFILEDLGW;

QString g_RunDialogCommand;

BOOLEAN OnWM_Notify(NMHDR *Header, LRESULT *Result)
{
	LPNMRUNFILEDLGW runFileDlg = (LPNMRUNFILEDLGW)Header;
	if (Header->code == RFN_VALIDATE)
	{
		g_RunDialogCommand = QString::fromWCharArray(runFileDlg->lpszFile);

		*Result = RF_CANCEL;
		return TRUE;
	}
	//else if (Header->code == RFN_LIMITEDRUNAS)
	//{
	//
	//}
	return FALSE;
}

extern "C"
{
	NTSYSCALLAPI NTSTATUS NTAPI LdrGetProcedureAddress(IN PVOID DllHandle, IN VOID* ProcedureName OPTIONAL, IN ULONG ProcedureNumber OPTIONAL, OUT PVOID *ProcedureAddress, IN BOOLEAN RunInitRoutines);
	//NTSTATUS(NTAPI *LdrGetProcedureAddress)(HMODULE ModuleHandle, PANSI_STRING FunctionName, WORD Oridinal, PVOID *FunctionAddress);
}

BOOLEAN NTAPI ShowRunFileDialog(HWND WindowHandle, HICON WindowIcon, LPCWSTR WorkingDirectory, LPCWSTR WindowTitle, LPCWSTR WindowDescription, ULONG Flags)
{
	typedef BOOL(WINAPI *RunFileDlg_I)(HWND hwndOwner, HICON hIcon, LPCWSTR lpszDirectory, LPCWSTR lpszTitle, LPCWSTR lpszDescription, ULONG uFlags);

	BOOLEAN result = FALSE;

	if (HMODULE shell32Handle = LoadLibrary(L"shell32.dll"))
	{
		RunFileDlg_I dialog = NULL;
		if (LdrGetProcedureAddress(shell32Handle, NULL, 61, (void**)&dialog, TRUE) == 0)
			result = !!dialog(WindowHandle, WindowIcon, WorkingDirectory, WindowTitle, WindowDescription, Flags);

		FreeLibrary(shell32Handle);
	}

	return result;
}

QString ShowRunDialog(const QString& BoxName)
{
	g_RunDialogCommand.clear();
	wstring boxName = BoxName.toStdWString();
	ShowRunFileDialog(MainWndHandle, NULL, NULL, boxName.c_str(), L"Enter the path of a program that will be created in a sandbox.", 0); // RFF_OPTRUNAS);
	return g_RunDialogCommand;
}
*/