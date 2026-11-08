#pragma once

#include <QtWidgets/QMainWindow>

#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/TreeViewEx.h"
#include "../MiscHelpers/Common/PanelView.h"
#include "../MiscHelpers/Common/ProgressDialog.h"
#include <QTranslator>

#include "../version.h"


//#include "../QSbieAPI/SbieAPI.h"
#include "SbiePlusAPI.h"

class CPopUpWindow;
class CSbieView;
class CFileView;
class CBoxBorder;
class CSbieTemplatesEx;
class CMcpServer;
class CMcpSessionWindow;
class CTraceView;
class CScriptManager;
class CAddonManager;
class QLineEdit;

struct ToolBarAction {
	// Identifier of action stored in ini. Empty for separator.
	QString scriptName = "";

	// Not owned. Null for special cases.
	QAction* action;

	// Display name override for display in toolbar config menu. Empty if no override.
	QString nameOverride = "";
};

class CSandMan : public QMainWindow
{
	Q_OBJECT

public:
	CSandMan(QWidget *parent = Q_NULLPTR);
	virtual ~CSandMan();

	CSbieTemplatesEx*	GetCompat() { return m_SbieTemplates; }

	//
	// The one list of everything SandMan has received. The message view is a
	// rendering of it, not the place it lives, and the troubleshooting engine
	// and the MCP server read it rather than keeping copies - an event that
	// exists in only one consumer is an event the others cannot diagnose from.
	//
	struct SSbieMsg {
		enum EKind {
			eSbieMessage = 0,	// a message from the driver or the service
			eDiagnostic,		// a diagnostic event, 7001-7099
			eNotice,			// something SandMan itself reported
		};

		QDateTime TimeStamp;
		quint32 MsgCode;		// 0 for a notice
		QStringList MsgData;
		QString ProcessName;
		QString BoxName;
		EKind Kind = eSbieMessage;
	};

	// Bounded, because this takes diagnostic events and notices as well and a
	// long session would otherwise grow it without limit. The oldest go first;
	// the user can still clear it explicitly.
	static const int	m_MessageLogLimit = 20000;

	// Read-only, and deliberately not a copy: both the engine and the MCP
	// server read this list in place.
	const QVector<SSbieMsg>& GetMessageLog() const { return m_MessageLog; }

	// NULL until armed. The endpoint is configured, the arming is deliberate.
	CMcpServer*			GetMcpServer() { return m_pMcpServerImpl; }
	bool				StartMcpServer();
	void				StopMcpServer();
	bool				IsMcpServerRunning() const;

	// Keeps the menu entry and the toolbar button telling the truth: pressed
	// while a connection is open, and greyed where turning it on would do
	// nothing because the feature is switched off here or by an administrator.
	void				UpdateMcpAction();

	// The one window an agent session is watched from. Created on first use and
	// kept for the lifetime of SandMan, because its value is the history it
	// accumulates - everything the agent asked for and what was answered.
	CMcpSessionWindow*	GetMcpWindow();
	void				CheckCompat(QObject* receiver, const char* member);
	CScriptManager*		GetScripts() { return m_SbieScripts; }
	CAddonManager*		GetAddonManager() { return m_AddonManager; }

	static QString		GetVersion(bool bWithUpdates = false);
	static void			ShowMessageBox(QWidget* Widget, QMessageBox::Icon Icon, const QString& Message);

	bool				IsImDiskReady() const { return m_ImDiskReady; }

	bool				IsWFPEnabled() const;

	SB_PROGRESS			RecoverFiles(const QString& BoxName, const QList<QPair<QString, QString>>& FileList, QWidget* pParent, int Action = 0);
	static QStringList	GetFileCheckers(const CSandBoxPtr& pBox);
	SB_PROGRESS			CheckFiles(const QString& BoxName, const QStringList& Files);

	enum EDelMode {
		eCleanUp,
		eAuto,
		eForDelete
	};

	SB_STATUS			DeleteBoxContent(const CSandBoxPtr& pBox, EDelMode Mode, bool DeleteSnapshots = true);

	void				UpdateProcesses();

	SB_STATUS			AddAsyncOp(const CSbieProgressPtr& pProgress, bool bWait = false, const QString& InitialMsg = QString(), QWidget* pParent = NULL);
	static QString		FormatError(const SB_STATUS& Error);
	void				CheckResults(QList<SB_STATUS> Results, QWidget* pParent, bool bAsync = false);

	static QIcon		GetIcon(const QString& Name, int iAction = 1);
	static QString		GetBoxDisplayName(const CSandBoxPtr& pBox, CSandBoxPlus::EDisplayNameContext Context = CSandBoxPlus::eDisplayNormal);
	static QString		GetBoxDisplayName(const QString& BoxName, CSandBoxPlus::EDisplayNameContext Context = CSandBoxPlus::eDisplayNormal);

	bool				IsFullyPortable();

	bool				IsShowHidden() { return m_pShowHidden && m_pShowHidden->isChecked(); }
	bool				KeepTerminated();
	bool				IsAutoExpand() { return m_pAutoExpand && m_pAutoExpand->isChecked(); }
	bool				ShowAllSessions() { return m_pShowAllSessions && m_pShowAllSessions->isChecked(); }
	bool				IsSilentMode();
	bool				IsDisableRecovery() {return IsSilentMode() || m_pDisableRecovery && m_pDisableRecovery->isChecked();}
	bool				IsDisableMessages() {return IsSilentMode() || m_pDisableMessages && m_pDisableMessages->isChecked();}
	CSbieView*			GetBoxView() { return m_pBoxView; }
	CFileView*			GetFileView() { return m_pFileView; }

	QString				FormatSbieMessage(quint32 MsgCode, const QStringList& MsgData, QString ProcessName, QString* pLink = NULL);
	QString				MakeSbieMsgLink(quint32 MsgCode, const QStringList& MsgData, QString ProcessName);

	static void			SafeShow(QWidget* pWidget);
	int					SafeExec(QDialog* pDialog);

	bool				RunSandboxed(const QStringList& Commands, QString BoxName = QString(), const QString& WrkDir = QString(), bool bShowFCP = false);
	SB_RESULT(quint32)	RunStart(const QString& BoxName, const QString& Command, CSbieAPI::EStartFlags Flags = CSbieAPI::eStartDefault, const QString& WorkingDir = QString(), QProcess* pProcess = NULL);
	SB_STATUS			ImBoxMount(const CSandBoxPtr& pBox, bool bAutoUnmount = false);

	void				OpenSettings(const QString& Tab = QString());

	void				EditIni(const QString& IniPath, bool bPlus = false);

	void				UpdateDrives();
	void				UpdateForceUSB();

	QIcon				GetBoxIcon(int boxType, bool inUse = false);
	QRgb				GetBoxColor(int boxType) { return m_BoxColors[boxType]; }
	QIcon				GetColorIcon(QColor boxColor, bool inUse = false/*, bool bOut = false*/);
	QIcon				MakeIconBusy(const QIcon& Icon, int Index = 0);
	QIcon				IconAddOverlay(const QIcon& Icon, const QString& Name, int Size = 24);
	QString				GetBoxDescription(int boxType);
	
	bool				SetCertificate(const QByteArray& Certificate);
	bool				CheckCertificate(QWidget* pWidget, int iType = 0);

	bool				IsAlwaysOnTop() const;

	void				UpdateTheme();
	void				UpdateTitleTheme(const HWND& hwnd);

	SB_STATUS			ReloadCert(QWidget* pWidget = NULL);

	void				SaveMessageLog(QIODevice* pFile);

signals:
	void				DrivesChanged();

	void				CertUpdated();

	void				Closed();

protected:
	friend class COnlineUpdater;
	SB_RESULT(void*)	ConnectSbie();
	SB_STATUS			ConnectSbieImpl();
	SB_STATUS			DisconnectSbie();
	SB_RESULT(void*)	StopSbie(bool andRemove = false);

	static void			RecoverFilesAsync(QPair<const CSbieProgressPtr&,QWidget*> pParam, const QString& BoxName, const QList<QPair<QString, QString>>& FileList, const QStringList& Checkers, int Action = 0);
	static void			CheckFilesAsync(const CSbieProgressPtr& pProgress, const QString& BoxName, const QStringList &Files, const QStringList& Checkers);

	void				AddLogMessage(const QDateTime& TimeStamp, const QString& Message, const QString& Link = QString());
	void				AddLogMessageNow(const QDateTime& TimeStamp, const QString& Message, const QString& Link, int Count);
	void				ScheduleMessageLogFlush();

	QIcon				GetTrayIcon(bool isConnected = true, bool bSun = false);
	QString				GetTrayText(bool isConnected = true);

	void				CheckSupport();

	void				closeEvent(QCloseEvent* e);
	void				changeEvent(QEvent* e);

	void				dragEnterEvent(QDragEnterEvent* e);
	void				dropEvent(QDropEvent* e);

	void				timerEvent(QTimerEvent* pEvent);
	int					m_uTimerID;
	bool				m_bConnectPending;
	bool				m_bStopPending;
	CBoxBorder*			m_pBoxBorder;
	CSbieTemplatesEx*	m_SbieTemplates;
	CMcpServer*         m_pMcpServerImpl;
	// Rate limiting for the doorbell: a client that retries in a loop must not
	// be able to put a prompt on screen repeatedly, and a declined prompt is a
	// cooldown rather than an immediately re-askable state.
	QDateTime           m_McpPromptCooldown;
	int                 m_McpDenyStreak = 0;
	bool                m_bSweptShadows = false;
	bool                m_bMcpPrompting = false;
	CMcpSessionWindow*  m_pMcpWindow = NULL;
	QString             m_McpPendingIdentity;

	CScriptManager*		m_SbieScripts;
	CAddonManager*		m_AddonManager;

	QMap<CSbieProgress*, QPair<CSbieProgressPtr, QPointer<QWidget>>> m_pAsyncProgress;

	QMap<QString, QSet<QString>> m_MissingTemplates;

	QMap<int, QRgb> m_BoxColors;

	class UGlobalHotkeys* m_pHotkeyManager;

	bool				m_ImDiskReady;

	// The single place that appends, so the bound is enforced once.
	void				AppendMessageLog(const SSbieMsg& Msg);
	QVector<SSbieMsg>	m_MessageLog;

	struct SPendingMessageLogEntry {
		QDateTime TimeStamp;
		QString Message;
		QString Link;
		int Count;
	};
	QList<SPendingMessageLogEntry> m_PendingMessageLog;
	qint64				m_MessageLogPlainItemModeUntil;
	bool				m_MessageLogFlushPending;
	bool				m_FlushingMessageLog;

public slots:
	void				OnBoxSelected();

	void				OnMessage(const QString& MsgData);

	void				OnStatusChanged();
	void				OnLogMessage(const QString& Message, bool bNotify = false);

	// MCP: arming is a human action, and the listener does not exist until then.
	void				OnToggleMcpServer();
	void				OnMcpLogMessage(int Level, const QString& Message);

	// The doorbell: the stdio bridge found nobody listening and is asking us to
	// put the question to the user. ClientName is the bridge's parent process,
	// which is the actual MCP client, or empty when it could not be resolved.
	void				OnMcpActivate(const QString& ClientName);
	void				OnMcpHelp();
	void				OnMcpShowWindow();

	// The pairing exchange, from the user's side: SandMan shows the code, the
	// user reads it to the agent, the agent submits it.
	void				OnMcpPairingCode(const QString& ClientName, const QString& Identity, const QString& Code, int ValidSeconds);
	void				OnMcpPaired(const QString& ClientName);
	void				OnMcpPairingFailed(const QString& ClientName, const QString& Reason);
	void				OnMcpSuspended(const QString& ClientName);
	void				OnMcpSessionStarted(const QString& ClientName, const QString& Identity);
	void				OnLogSbieMessage(quint32 MsgCode, const QStringList& MsgData, quint32 ProcessId);

	void				OnNotAuthorized(bool bLoginRequired, bool& bRetry);

	void				OnQueuedRequest(quint32 ClientPid, quint32 ClientTid, quint32 RequestId, const QVariantMap& Data);
	void				OnFileToRecover(const QString& BoxName, const QString& FilePath, const QString& BoxPath, quint32 ProcessId);
	void				OnFileRecovered(const QString& BoxName, const QString& FilePath, const QString& BoxPath);

	bool				OpenRecovery(const CSandBoxPtr& pBox, bool& DeleteSnapshots, bool bCloseEmpty = false);
	class CRecoveryWindow* ShowRecovery(const CSandBoxPtr& pBox);

	void				TryFix(quint32 MsgCode, const QStringList& MsgData, const QString& ProcessName, const QString& BoxName);

	void				OpenCompat();

	void				UpdateSettings(bool bRebuildUI);
	void				RebuildUI();
	void				OnIniReloaded();

	void				SetupHotKeys();
	void				OnHotKey(size_t id);

	void				OnAsyncFinished();
	void				OnAsyncFinished(CSbieProgress* pProgress);
	void				OnAsyncMessage(const QString& Text);
	void				OnAsyncProgress(int Progress);
	void				OnCancelAsync();

	void				OnBoxAdded(const CSandBoxPtr& pBox);
	void				OnBoxOpened(const CSandBoxPtr& pBox);
	void				OnBoxClosed(const CSandBoxPtr& pBox);
	void				OnBoxCleaned(CSandBoxPlus* pBoxEx);

	void				OnStartMenuChanged();


	void				OpenUrl(const QString& url) { OpenUrl(QUrl(url)); }
	void				OpenUrl(QUrl url);

	int					ShowQuestion(const QString& question, const QString& checkBoxText, bool* checkBoxSetting, int buttons, int defaultButton, int type, QWidget* pParent);
	void				ShowMessage(const QString& message, int type);

	void				OnBoxMenu(const QPoint &);
	void				OnBoxDblClick(QTreeWidgetItem*);

	void				SyncStartMenu();
	void				ClearStartMenu();

	void				UpdateLabel();

private slots:

	void				OnMenuHover(QAction* action);
	void				OnBoxMenuHover(QAction* action);

	void				OnSandBoxAction();
	void				OnSettingsAction();
	void				OnPauseAll();
	void				OnEmptyAll();
	void				OnLockAll();
	void				OnWndFinder();
	void				OnBoxAssistant();
	void				OnDisableForce();
	void				OnDisableForce2();
	void				OnDisablePopUp();
	void				OnMaintenance();

	void				OnViewMode(QAction* action);
	void				OnAlwaysTop();
	void				OnView(QAction* action);
	void				OnRefresh();
	void				OnCleanUp();
	void				OnProcView();
	void				OnAutoExpand();
	void				OnRecoveryLog();

	void				OnSettings();
	void				OnResetMsgs();
	void				OnResetGUI();
	void				OnEditIni();
	void				OnReloadIni();
	void				OnMonitoring();

	void				OnSymbolStatus(const QString& Message);

	void				CheckForUpdates(bool bManual = true);

	void				OnRestartAsAdmin();

	void				OnExit();
	void				OnHelp();
	void				OnAbout();

	void				OnShowHide();
	void				OnTraySearch(const QString& Text);
	void				OnSysTray(QSystemTrayIcon::ActivationReason Reason);

	void				SetUITheme();
	void				SetTitleTheme(const HWND& hwnd);

    void				OnCertData(const QByteArray& Certificate, const QVariantMap& Params);

	void				AddLogMessage(const QString& Message);
	void				AddFileRecovered(const QString& BoxName, const QString& FilePath);
	void				OnFlushMessageLog();
	void				OnMessageLogDblClick(QTreeWidgetItem* pItem, int Column);

	void				commitData(QSessionManager& manager);

private:

	void				CreateUI();

	void				CreateMenus(bool bAdvanced);
	void				CreateOldMenus();
	void				CreateMaintenanceMenu();
	void				CreateViewBaseMenu();
	void				CreateHelpMenu(bool bAdvanced);
	void				CreateToolBar(bool bRebuild);
	void				CreateLabel();
	void				CreateView(int iViewMode);
	void				CreateTrayIcon();
	void				CreateTrayMenu();
	void				CreateBoxMenu(QMenu* pMenu, int iOffset = 0, int iSysTrayFilter = 0);

	void				HandleMaintenance(SB_RESULT(void*) Status);

	void				LoadState(bool bFull = true);
	void				StoreState();

	void				UpdateState();

	struct SBoxLink {
		QString RelPath; // key
		QString FullPath;
		QString Target;
	};

	void				EnumBoxLinks(QMap<QString, QMap<QString, SBoxLink> >& BoxLinks, const QString& Prefix, const QString& Folder, bool bWithSubDirs = true);
	void				CleanupShortcutPath(const QString& Path);
	void				DeleteShortcut(const QString& Path);
	void				CleanUpStartMenu(QMap<QString, QMap<QString, SBoxLink> >& BoxLinks);

	QSet<QString>		GetToolBarItemsConfig();
	void				SetToolBarItemsConfig(const QSet<QString>& items);
	QList<ToolBarAction> GetAvailableToolBarActions();
	void                CreateToolBarConfigMenu(const QList<ToolBarAction>& actions, const QSet<QString>& currentSet);
	void				OnToolBarMenuItemClicked(const QString& scriptName);
	void				OnResetToolBarMenuConfig();

	const QString       ToolBarConfigKey = "UIConfig/ToolBarItems";

	// per 1.9.3 menu. no whitespace!
	//
	// Only used when nothing is stored, so adding an entry here gives new
	// installations the button without rearranging a toolbar somebody already
	// arranged. Existing users pick it up from the toolbar's own context menu,
	// where it appears under its menu text.
	const QStringList	DefaultToolBarItems = QString(
						  "ReloadIni,EditIniMenu,Settings,McpServer,RunBoxed,NewBoxMenu,EnableMonitor,TerminateAll,CleanUpMenu,BrowseFiles,KeepTerminated"
						).split(',');

	QWidget*			m_pMainWidget;
	QVBoxLayout*		m_pMainLayout;

	QToolBar*			m_pToolBar;
	QMenu*				m_pToolBarContextMenu;
	QSplitter*			m_pPanelSplitter;
	QSplitter*			m_pLogSplitter;

	QStackedLayout*		m_pViewStack;
	QComboBox*			m_pBoxCombo;

	CSbieView*			m_pBoxView;
	CFileView*			m_pFileView;


	QTabWidget*			m_pLogTabs;
	CPanelWidgetEx*		m_pMessageLog;
	CTraceView*			m_pTraceView;
	CPanelWidgetEx*		m_pRecoveryLog;
	class CRecoveryLogWnd* m_pRecoveryLogWnd;

	QMenuBar*			m_pMenuBar;
	QHBoxLayout*		m_pMenuLayout;

	QMenu*				m_pMenuFile;
	QAction*			m_pRestart;
	QAction*			m_pRunBoxed;
	QAction*			m_pNewBox;
	QAction*			m_pNewGroup;
	QAction*			m_pImportBoxes;
	QAction*			m_pExportBoxes;
	QAction*			m_pPauseAll;
	QAction*			m_pEmptyAll;
	QAction*			m_pLockAll;
	QAction*			m_pWndFinder;
	QAction*			m_pDisableForce;
	QAction*			m_pDisableForce2;
	QAction*			m_pDisableRecovery;
	QAction*			m_pDisableMessages;
	QAction*			m_pDismissUpdate;
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
	QAction*			m_pImDiskCpl;
	QAction*			m_pUninstallAll;
	QAction*			m_pSetupWizard;
	QAction*			m_pExit;

	QMenu*				m_pMenuView;
	QActionGroup*		m_pViewMode;
	QAction*			m_pShowHidden;
	QAction*			m_pWndTopMost;
	QAction*			m_pMenuBrowse;
	QAction*			m_pRefreshAll;
	QMenu*				m_pCleanUpMenu;
	QAction*			m_pCleanUpProcesses;
	QAction*			m_pCleanUpMsgLog;
	QAction*			m_pCleanUpTrace;
	QAction*			m_pCleanUpRecovery;
	QToolButton*		m_pCleanUpButton;
	QToolButton*		m_pNewBoxButton;
	QToolButton*		m_pMcpButton;
	QToolButton*		m_pEditIniButton;
	//QToolButton*		m_pEditButton;
	QAction*			m_pKeepTerminated;
	QAction*			m_pAutoExpand;
	QAction*			m_pShowAllSessions;
	QAction*			m_pArrangeGroups;

	QMenu*				m_pMenuOptions;
	QAction*			m_pMenuSettings;
	QAction*			m_pMenuResetMsgs;
	QAction*			m_pMenuResetGUI;
	QAction*			m_pMcpServer;
	QAction*			m_pMcpHelp;
	QAction*			m_pMcpWindowAction;
	QAction*			m_pEditIni;
	QAction*			m_pEditIni2;
	QAction*			m_pEditIni3;
	QAction*			m_pReloadIni;
	QAction*			m_pEnableMonitoring;

	//QMenu*				m_pMenuTools;
	QAction*			m_pBoxAssistant;

	QAction*			m_pSeparator;
	QLabel*				m_pLabel;

	QMenu*				m_pMenuHelp;
	//QAction*			m_pSupport;
	QAction*			m_pContribution;
	QAction*			m_pForum;
	QAction*			m_pManual;
	QAction*			m_pUpdate;
	QAction*			m_pAbout;
	QAction*			m_pAboutQt;

	QLabel*				m_pTraceInfo;
	QLabel*				m_pDisabledForce;
	QLabel*				m_pDisabledRecovery;
	QLabel*				m_pDisabledMessages;
	QLabel*				m_pRamDiskInfo;
	QLabel*				m_pSummaryInfo;
	int					m_iRefreshTick;

	// for old menu
	QMenu*				m_pSandbox;


	QSystemTrayIcon*	m_pTrayIcon;
	QMenu*				m_pTrayMenu;
	QLineEdit*			m_pTraySearch;
	QWidgetAction*		m_pTrayList;
	QTreeWidget*		m_pTrayBoxes;
	int					m_iTrayPos;

	//QMenu*				m_pBoxMenu;
	bool				m_bIconEmpty;
	int					m_iIconDisabled;
	bool				m_bIconBusy;
	bool				m_bIconSun;
	int					m_iDeletingContent;

	bool				m_bOnTop;

	bool				m_bExit;

	CProgressDialog*	m_pProgressDialog;
	bool				m_pProgressModal;
	CPopUpWindow*		m_pPopUpWindow;

	bool				m_StartMenuUpdatePending;
	quint64				m_LastCheckInternetMs;
	bool				m_bHasInternet;
public:
	QMap<QString, QPair<QString, QIcon>> m_TrayIconCache; // boxName -> (configKey, icon)
	bool				m_ThemeUpdatePending;
	QString				m_DefaultStyle;
	QPalette			m_DefaultPalett;
	double				m_DefaultFontSize;
	QPalette			m_DarkPalett;

	void				LoadLanguage();
	void				LoadLanguage(const QString& Lang, const QString& Module, int Index);
	QTranslator			m_Translator[2];

	class COnlineUpdater*m_pUpdater;

	QString				m_Language;
	quint32				m_LanguageId;
	bool				m_DarkTheme;
};


class CRecoveryLogWnd : public QDialog
{
	Q_OBJECT

public:
	CRecoveryLogWnd(QWidget *parent = Q_NULLPTR);
	~CRecoveryLogWnd();

private slots:
	void		OnDblClick(QTreeWidgetItem* pItem);

signals:
	void		Closed();

protected:
	friend class CSandMan;
	void		closeEvent(QCloseEvent *e);

	CPanelWidgetEx* m_pRecoveryLog;
};

#include <QStyledItemDelegate>
class CTreeItemDelegate : public QStyledItemDelegate
{
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		QStyleOptionViewItem opt(option);
		opt.state &= ~QStyle::State_HasFocus;
		QStyledItemDelegate::paint(painter, opt, index);
	}
};

class CTreeItemDelegate2 : public CTreeItemDelegate
{
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		QSize size = QStyledItemDelegate::sizeHint(option, index);
		size.setHeight(32);
		return size;
	}
};

class CTrayBoxesItemDelegate : public QStyledItemDelegate
{
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		QSize size = QStyledItemDelegate::sizeHint(option, index);
		size.setHeight(qMax(size.height(), 20));
		return size;
	}

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		QStyleOptionViewItem opt(option);
		opt.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
		if ((opt.state & QStyle::State_MouseOver) != 0)
			opt.state |= QStyle::State_Selected;
		else if ((opt.state & QStyle::State_HasFocus) != 0 && m_Hold)
			opt.state |= QStyle::State_Selected;
		opt.state &= ~QStyle::State_HasFocus;
		QStyledItemDelegate::paint(painter, opt, index);
	}
public:
	static bool m_Hold;
};

extern CSandMan* theGUI;
