#pragma once

#include <QtWidgets/QMainWindow>

#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/TreeViewEx.h"
#include "../MiscHelpers/Common/PanelView.h"
#include "../MiscHelpers/Common/ProgressDialog.h"
#include "../MiscHelpers/Common/NetworkAccessManager.h"
#include <QTranslator>
#include "Windows/PopUpWindow.h"

#include "../version.h"


//#include "../QSbieAPI/SbieAPI.h"
#include "SbiePlusAPI.h"

class CSbieView;
class CBoxBorder;
class CSbieTemplates;
class CTraceView;


class CSandMan : public QMainWindow
{
	Q_OBJECT

public:
	CSandMan(QWidget *parent = Q_NULLPTR);
	virtual ~CSandMan();

	CSbieTemplates*		GetCompat() { return m_SbieTemplates; }

	static QString		GetVersion();

	SB_PROGRESS			RecoverFiles(const QString& BoxName, const QList<QPair<QString, QString>>& FileList, int Action = 0);

	enum EDelMode {
		eDefault,
		eAuto,
		eForDelete
	};

	SB_STATUS			DeleteBoxContent(const CSandBoxPtr& pBox, EDelMode Mode, bool DeleteShapshots = true);

	SB_STATUS			AddAsyncOp(const CSbieProgressPtr& pProgress, bool bWait = false, const QString& InitialMsg = QString());
	static QString		FormatError(const SB_STATUS& Error);
	static void			CheckResults(QList<SB_STATUS> Results);

	static QIcon		GetIcon(const QString& Name, bool bAction = true);

	bool				IsFullyPortable();

	bool				IsShowHidden() { return m_pShowHidden && m_pShowHidden->isChecked(); }
	bool				KeepTerminated() { return m_pKeepTerminated && m_pKeepTerminated->isChecked(); }
	bool				ShowAllSessions() { return m_pShowAllSessions && m_pShowAllSessions->isChecked(); }
	bool				IsDisableRecovery() {return m_pDisableRecovery && m_pDisableRecovery->isChecked();}
	bool				IsDisableMessages() {return m_pDisableMessages && m_pDisableMessages->isChecked();}
	CSbieView*			GetBoxView() { return m_pBoxView; }

	bool				RunSandboxed(const QStringList& Commands, const QString& BoxName, const QString& WrkDir = QString());

	QIcon				GetBoxIcon(int boxType, bool inUse = false);// , bool inBusy = false);
	QRgb				GetBoxColor(int boxType) { return m_BoxColors[boxType]; }
	QIcon				GetColorIcon(QColor boxColor, bool inUse = false);
	QIcon				MakeIconBusy(const QIcon& Icon, int Index = 0);
	QString				GetBoxDescription(int boxType);
	
	bool				CheckCertificate();

	void				UpdateTheme();

	void				InstallUpdate();

	void				UpdateCertState();
	void				UpdateCert();

signals:
	void				CertUpdated();

protected:
	SB_RESULT(void*)	ConnectSbie();
	SB_STATUS			ConnectSbieImpl();
	SB_STATUS			DisconnectSbie();
	SB_RESULT(void*)	StopSbie(bool andRemove = false);

	static void			RecoverFilesAsync(const CSbieProgressPtr& pProgress, const QString& BoxName, const QList<QPair<QString, QString>>& FileList, int Action = 0);

	QIcon				GetTrayIcon(bool isConnected = true);
	QString				GetTrayText(bool isConnected = true);

	void				closeEvent(QCloseEvent* e);

	void				dragEnterEvent(QDragEnterEvent* e);
	void				dropEvent(QDropEvent* e);

	void				timerEvent(QTimerEvent* pEvent);
	int					m_uTimerID;
	bool				m_bConnectPending;
	bool				m_bStopPending;
	CBoxBorder*			m_pBoxBorder;
	CSbieTemplates*		m_SbieTemplates;
	
	QMap<CSbieProgress*, CSbieProgressPtr> m_pAsyncProgress;

	CNetworkAccessManager*	m_RequestManager;
	CSbieProgressPtr	m_pUpdateProgress;

	QStringList			m_MissingTemplates;

	enum EBoxColors
	{
		eYellow = 0,
		eRed,
		eGreen,
		eBlue,
		eCyan,
		eMagenta,
		eOrang,
		eWhite,
		eMaxColor
	};

	QMap<int, QRgb> m_BoxColors;

	struct SBoxIcon {
		QIcon Empty;
		QIcon InUse;
		//QIcon Busy;
	};
	QMap<int, SBoxIcon> m_BoxIcons;

	class UGlobalHotkeys* m_pHotkeyManager;

public slots:
	void				OnMessage(const QString& MsgData);

	void				OnStatusChanged();
	void				OnLogMessage(const QString& Message, bool bNotify = false);
	void				OnLogSbieMessage(quint32 MsgCode, const QStringList& MsgData, quint32 ProcessId);

	void				OnNotAuthorized(bool bLoginRequired, bool& bRetry);

	void				OnQueuedRequest(quint32 ClientPid, quint32 ClientTid, quint32 RequestId, const QVariantMap& Data);
	void				OnFileToRecover(const QString& BoxName, const QString& FilePath, const QString& BoxPath, quint32 ProcessId);
	void				OnFileRecovered(const QString& BoxName, const QString& FilePath, const QString& BoxPath);

	bool				OpenRecovery(const CSandBoxPtr& pBox, bool& DeleteShapshots, bool bCloseEmpty = false);
	class CRecoveryWindow*	ShowRecovery(const CSandBoxPtr& pBox, bool bFind = true);

	void				UpdateSettings(bool bRebuildUI);
	void				OnIniReloaded();

	void				SetupHotKeys();
	void				OnHotKey(size_t id);

	void				OnAsyncFinished();
	void				OnAsyncFinished(CSbieProgress* pProgress);
	void				OnAsyncMessage(const QString& Text);
	void				OnAsyncProgress(int Progress);
	void				OnCancelAsync();

	void				OnBoxClosed(const CSandBoxPtr& pBox);

	void				CheckForUpdates(bool bManual = true);
	void				DownloadUpdates(const QString& DownloadUrl, bool bManual);

	void				OpenUrl(const QString& url) { OpenUrl(QUrl(url)); }
	void				OpenUrl(const QUrl& url);

	int					ShowQuestion(const QString& question, const QString& checkBoxText, bool* checkBoxSetting, int buttons, int defaultButton);

	void				OnBoxMenu(const QPoint &);
	void				OnBoxDblClick(QTreeWidgetItem*);

private slots:

	void				OnMenuHover(QAction* action);
	void				OnBoxMenuHover(QAction* action);

	void				OnSandBoxAction();
	void				OnEmptyAll();
	void				OnWndFinder();
	void				OnDisableForce();
	void				OnDisableForce2();
	void				OnMaintenance();

	void				OnViewMode(QAction* action);
	void				OnAlwaysTop();
	void				OnRefresh();
	void				OnCleanUp();
	void				OnProcView();

	void				OnSettings();
	void				OnResetMsgs();
	void				OnResetGUI();
	void				OnEditIni();
	void				OnReloadIni();
	void				OnMonitoring();

	void				OnExit();
	void				OnHelp();
	void				OnAbout();

	void				OnShowHide();
	void				OnSysTray(QSystemTrayIcon::ActivationReason Reason);

	void				OnUpdateCheck();
	void				OnUpdateProgress(qint64 bytes, qint64 bytesTotal);
	void				OnUpdateDownload();

	void				OnCertCheck();

	void				SetUITheme();

	void				AddLogMessage(const QString& Message);
	void				AddFileRecovered(const QString& BoxName, const QString& FilePath);

	void				UpdateLabel();

private:

	void				CreateUI();

	void				CreateMenus(bool bAdvanced);
	void				CreateOldMenus();
	void				CreateMaintenanceMenu();
	void				CreateViewBaseMenu();
	void				CreateHelpMenu(bool bAdvanced);
	void				CreateToolBar();
	void				CreateView(bool bAdvanced);
	void				CreateTrayIcon();
	void				CreateTrayMenu();
	void				CreateBoxMenu(QMenu* pMenu, int iOffset = 0, int iSysTrayFilter = 0);

	void				HandleMaintenance(SB_RESULT(void*) Status);

	void				LoadState();
	void				StoreState();

	QWidget*			m_pMainWidget;
	QVBoxLayout*		m_pMainLayout;

	QToolBar*			m_pToolBar;

	QSplitter*			m_pPanelSplitter;

	QSplitter*			m_pLogSplitter;

	CSbieView*			m_pBoxView;


	QTabWidget*			m_pLogTabs;

	CPanelWidgetEx*		m_pMessageLog;
	CTraceView*			m_pTraceView;
	CPanelWidgetEx*		m_pRecoveryLog;


	QMenu*				m_pMenuFile;
	QAction*			m_pNewBox;
	QAction*			m_pNewGroup;
	QAction*			m_pEmptyAll;
	QAction*			m_pWndFinder;
	QAction*			m_pDisableForce;
	QAction*			m_pDisableForce2;
	QAction*			m_pDisableRecovery;
	QAction*			m_pDisableMessages;
	QMenu*				m_pMaintenance;
	QAction*			m_pConnect;
	QAction*			m_pDisconnect;
	QMenu*				m_pMaintenanceItems;
	QAction*			m_pInstallDrv;
	QAction*			m_pStartDrv;
	QAction*			m_pStopDrv;
	QAction*			m_pUninstallDrv;
	QAction*			m_pInstallSvc;
	QAction*			m_pStartSvc;
	QAction*			m_pStopSvc;
	QAction*			m_pUninstallSvc;
	QAction*			m_pStopAll;
	QAction*			m_pUninstallAll;
	QAction*			m_pSetupWizard;
	QAction*			m_pExit;

	QMenu*				m_pMenuView;
	QActionGroup*		m_pViewMode;
	QAction*			m_pShowHidden;
	QAction*			m_pWndTopMost;
	QAction*			m_pRefreshAll;
	QMenu*				m_pCleanUpMenu;
	QAction*			m_pCleanUpProcesses;
	QAction*			m_pCleanUpMsgLog;
	QAction*			m_pCleanUpTrace;
	QAction*			m_pCleanUpRecovery;
	QToolButton*		m_pCleanUpButton;
	QAction*			m_pKeepTerminated;
	QAction*			m_pShowAllSessions;

	QMenu*				m_pMenuOptions;
	QAction*			m_pMenuSettings;
	QAction*			m_pMenuResetMsgs;
	QAction*			m_pMenuResetGUI;
	QAction*			m_pEditIni;
	QAction*			m_pReloadIni;
	QAction*			m_pEnableMonitoring;

	QAction*			m_pSeparator;
	QLabel*				m_pLabel;

	QMenu*				m_pMenuHelp;
	QAction*			m_pSupport;
	QAction*			m_pForum;
	QAction*			m_pManual;
	QAction*			m_pUpdate;
	QAction*			m_pAbout;
	QAction*			m_pAboutQt;


	// for old menu
	QMenu*				m_pSandbox;


	QSystemTrayIcon*	m_pTrayIcon;
	QMenu*				m_pTrayMenu;
	QWidgetAction*		m_pTrayList;
	QTreeWidget*		m_pTrayBoxes;
	int					m_iTrayPos;
	//QMenu*				m_pBoxMenu;
	bool				m_bIconEmpty;
	bool				m_bIconDisabled;
	bool				m_bIconBusy;
	int					m_iDeletingContent;

	bool				m_bExit;

	CProgressDialog*	m_pProgressDialog;
	bool				m_pProgressModal;
	CPopUpWindow*		m_pPopUpWindow;

	bool				m_ThemeUpdatePending;
	QString				m_DefaultStyle;
	QPalette			m_DefaultPalett;
	double				m_DefaultFontSize;

	void				LoadLanguage();
	void				LoadLanguage(const QString& Lang, const QString& Module, int Index);
	QTranslator			m_Translator[2];

public:
	quint32				m_LanguageId;
	bool				m_DarkTheme;
};

extern CSandMan* theGUI;
