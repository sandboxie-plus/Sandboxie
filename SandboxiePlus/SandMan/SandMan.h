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
class CFileView;
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

	static QIcon		GetIcon(const QString& Name, int iAction = 1);

	bool				IsFullyPortable();

	bool				IsShowHidden() { return m_pShowHidden && m_pShowHidden->isChecked(); }
	bool				KeepTerminated() { return m_pKeepTerminated && m_pKeepTerminated->isChecked(); }
	bool				ShowAllSessions() { return m_pShowAllSessions && m_pShowAllSessions->isChecked(); }
	bool				IsSilentMode();
	bool				IsDisableRecovery() {return IsSilentMode() || m_pDisableRecovery && m_pDisableRecovery->isChecked();}
	bool				IsDisableMessages() {return IsSilentMode() || m_pDisableMessages && m_pDisableMessages->isChecked();}
	CSbieView*			GetBoxView() { return m_pBoxView; }
	CFileView*			GetFileView() { return m_pFileView; }

	bool				RunSandboxed(const QStringList& Commands, QString BoxName = QString(), const QString& WrkDir = QString());

	QIcon				GetBoxIcon(int boxType, bool inUse = false);// , bool inBusy = false);
	QRgb				GetBoxColor(int boxType) { return m_BoxColors[boxType]; }
	QIcon				GetColorIcon(QColor boxColor, bool inUse = false/*, bool bOut = false*/);
	QIcon				MakeIconBusy(const QIcon& Icon, int Index = 0);
	QIcon				MakeIconRecycle(const QIcon& Icon);
	QString				GetBoxDescription(int boxType);
	
	bool				CheckCertificate(QWidget* pWidget);

	void				UpdateTheme();
	void				UpdateTitleTheme(const HWND& hwnd);

	void				UpdateCertState();

signals:
	void				CertUpdated();

protected:
	friend class COnlineUpdater;
	SB_RESULT(void*)	ConnectSbie();
	SB_STATUS			ConnectSbieImpl();
	SB_STATUS			DisconnectSbie();
	SB_RESULT(void*)	StopSbie(bool andRemove = false);

	static void			RecoverFilesAsync(const CSbieProgressPtr& pProgress, const QString& BoxName, const QList<QPair<QString, QString>>& FileList, int Action = 0);

	QIcon				GetTrayIcon(bool isConnected = true);
	QString				GetTrayText(bool isConnected = true);

	void				CheckSupport();

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

	//struct SBoxIcon {
	//	QIcon Empty;
	//	QIcon InUse;
	//	//QIcon Busy;
	//};
	//QMap<int, SBoxIcon> m_BoxIcons;

	class UGlobalHotkeys* m_pHotkeyManager;

public slots:
	void				OnBoxSelected();

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
	void				OnBoxClosed(const CSandBoxPtr& pBox);

	void				OnStartMenuChanged();


	void				OpenUrl(const QString& url) { OpenUrl(QUrl(url)); }
	void				OpenUrl(const QUrl& url);

	int					ShowQuestion(const QString& question, const QString& checkBoxText, bool* checkBoxSetting, int buttons, int defaultButton, int type);
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
	void				OnEmptyAll();
	void				OnWndFinder();
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
	void				OnRecoveryLog();

	void				OnSettings();
	void				OnResetMsgs();
	void				OnResetGUI();
	void				OnEditIni();
	void				OnReloadIni();
	void				OnMonitoring();

	void				CheckForUpdates(bool bManual = true);

	void				OnExit();
	void				OnHelp();
	void				OnAbout();

	void				OnShowHide();
	void				OnSysTray(QSystemTrayIcon::ActivationReason Reason);

	void				SetUITheme();
	void				SetTitleTheme(const HWND& hwnd);

	void				AddLogMessage(const QString& Message);
	void				AddFileRecovered(const QString& BoxName, const QString& FilePath);

	void				commitData(QSessionManager& manager);

private:

	void				CreateUI();

	void				CreateMenus(bool bAdvanced);
	void				CreateOldMenus();
	void				CreateMaintenanceMenu();
	void				CreateViewBaseMenu();
	void				CreateHelpMenu(bool bAdvanced);
	void				CreateToolBar();
	void				CreateLabel();
	void				CreateView(int iViewMode);
	void				CreateTrayIcon();
	void				CreateTrayMenu();
	void				CreateBoxMenu(QMenu* pMenu, int iOffset = 0, int iSysTrayFilter = 0);

	void				HandleMaintenance(SB_RESULT(void*) Status);

	void				LoadState(bool bFull = true);
	void				StoreState();

	void				UpdateState();

	void				EnumBoxLinks(QMap<QString, QMap<QString, QString> >& BoxLinks, const QString& Prefix, const QString& Folder, bool bWithSubDirs = true);
	void				CleanupShortcutPath(const QString& Path);
	void				DeleteShortcut(const QString& Path);
	void				CleanUpStartMenu(QMap<QString, QMap<QString, QString> >& BoxLinks);

	QWidget*			m_pMainWidget;
	QVBoxLayout*		m_pMainLayout;

	QToolBar*			m_pToolBar;
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
	QAction*			m_pRunBoxed;
	QAction*			m_pNewBox;
	QAction*			m_pNewGroup;
	QAction*			m_pImportBox;
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
	QAction*			m_pMenuBrowse;
	QAction*			m_pRefreshAll;
	QMenu*				m_pCleanUpMenu;
	QAction*			m_pCleanUpProcesses;
	QAction*			m_pCleanUpMsgLog;
	QAction*			m_pCleanUpTrace;
	QAction*			m_pCleanUpRecovery;
	QToolButton*		m_pCleanUpButton;
	QAction*			m_pKeepTerminated;
	QAction*			m_pShowAllSessions;
	QAction*			m_pArrangeGroups;

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
	QAction*			m_pContribution;
	QAction*			m_pForum;
	QAction*			m_pManual;
	QAction*			m_pUpdate;
	QAction*			m_pAbout;
	QAction*			m_pAboutQt;

	QLabel*				m_pDisabledForce;
	QLabel*				m_pDisabledRecovery;
	QLabel*				m_pDisabledMessages;

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

	bool				m_StartMenuUpdatePending;

	bool				m_ThemeUpdatePending;
	QString				m_DefaultStyle;
	QPalette			m_DefaultPalett;
	double				m_DefaultFontSize;
	QPalette			m_DarkPalett;

	void				LoadLanguage();
	void				LoadLanguage(const QString& Lang, const QString& Module, int Index);
	QTranslator			m_Translator[2];

public:
	class COnlineUpdater*m_pUpdater;

	quint32				m_LanguageId;
	bool				m_DarkTheme;
	bool				m_FusionTheme;
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

extern CSandMan* theGUI;
