#pragma once

#include <QtWidgets/QMainWindow>

#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/TreeViewEx.h"
#include "../MiscHelpers/Common/PanelView.h"
#include "../MiscHelpers/Common/ProgressDialog.h"
#include "../MiscHelpers/Common/NetworkAccessManager.h"
#include "Models/ResMonModel.h"
#include "Models/ApiMonModel.h"
#include <QTranslator>
#include "Windows/PopUpWindow.h"

#define VERSION_MJR		0
#define VERSION_MIN 	5
#define VERSION_REV 	3
#define VERSION_UPD 	1


//#include "../QSbieAPI/SbieAPI.h"
#include "SbiePlusAPI.h"

class CSbieView;
class CApiLog;
class CBoxBorder;
class CSbieTemplates;

class CSandMan : public QMainWindow
{
	Q_OBJECT

public:
	CSandMan(QWidget *parent = Q_NULLPTR);
	virtual ~CSandMan();

	CSbieTemplates*		GetCompat() { return m_SbieTemplates; }

	static QString		GetVersion();

	SB_PROGRESS			RecoverFiles(const QList<QPair<QString, QString>>& FileList, int Action = 0);

	void				AddAsyncOp(const CSbieProgressPtr& pProgress);
	static QString		FormatError(const SB_STATUS& Error);
	static void			CheckResults(QList<SB_STATUS> Results);

	static QIcon		GetIcon(const QString& Name);

protected:
	SB_STATUS			ConnectSbie();
	SB_STATUS			ConnectSbieImpl();
	SB_STATUS			DisconnectSbie();
	SB_STATUS			StopSbie(bool andRemove = false);

	static void			RecoverFilesAsync(const CSbieProgressPtr& pProgress, const QList<QPair<QString, QString>>& FileList, int Action = 0);

	bool				IsFullyPortable();

	void				closeEvent(QCloseEvent *e);
	void				timerEvent(QTimerEvent* pEvent);
	int					m_uTimerID;
	bool				m_bConnectPending;
	bool				m_bStopPending;
	CBoxBorder*			m_pBoxBorder;
	CSbieTemplates*		m_SbieTemplates;

	CApiLog*			m_ApiLog;
	
	QMap<CSbieProgress*, CSbieProgressPtr> m_pAsyncProgress;

	CNetworkAccessManager*	m_RequestManager;
	CSbieProgressPtr	m_pUpdateProgress;

public slots:
	void				OnMessage(const QString&);

	void				OnStatusChanged();
	void				OnLogMessage(const QString& Message, bool bNotify = false);
	void				OnLogSbieMessage(quint32 MsgCode, const QStringList& MsgData, quint32 ProcessId);

	void				OnNotAuthorized(bool bLoginRequired, bool& bRetry);

	void				OnQueuedRequest(quint32 ClientPid, quint32 ClientTid, quint32 RequestId, const QVariantMap& Data);
	void				OnFileToRecover(const QString& BoxName, const QString& FilePath, quint32 ProcessId);

	void				OpenRecovery(const QString& BoxName);

	void				UpdateSettings();
	void				OnIniReloaded();

	void				OnAsyncFinished();
	void				OnAsyncFinished(CSbieProgress* pProgress);
	void				OnAsyncMessage(const QString& Text);
	void				OnAsyncProgress(int Progress);
	void				OnCancelAsync();

	void				OnBoxClosed(const QString& BoxName);

	void				CheckForUpdates(bool bManual = true);

	void				OpenUrl(const QUrl& url);

private slots:
	void				OnSelectionChanged();

	void				OnMenuHover(QAction* action);

	void				OnNewBox();
	void				OnEmptyAll();
	void				OnDisableForce();
	void				OnDisableForce2();
	void				OnMaintenance();

	void				OnViewMode(QAction* action);
	void				OnCleanUp();
	void				OnSetKeep();

	void				OnSettings();
	void				OnResetMsgs();
	void				OnEditIni();
	void				OnReloadIni();
	void				OnSetMonitoring();
	void				OnSetLogging();

	void				OnExit();
	void				OnHelp();
	void				OnAbout();

	void				OnShowHide();
	void				OnSysTray(QSystemTrayIcon::ActivationReason Reason);

	void				OnUpdateCheck();
	void				OnUpdateProgress(qint64 bytes, qint64 bytesTotal);
	void				OnUpdateDownload();

private:
	void				CreateMenus();
	void				CreateToolBar();

	void				SetViewMode(bool bAdvanced);

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
	CPanelViewEx*		m_pResourceLog;
	CResMonModel*		m_pResMonModel;
	CPanelViewEx*		m_pApiCallLog;
	CApiMonModel*		m_pApiMonModel;


	QMenu*				m_pMenuFile;
	QAction*			m_pNew;
	QAction*			m_pEmptyAll;
	QAction*			m_pDisableForce;
	QAction*			m_pDisableForce2;
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
	QAction*			m_pExit;

	QMenu*				m_pMenuView;
	QActionGroup*		m_pViewMode;
	int					m_iMenuViewPos;
	QMenu*				m_pCleanUpMenu;
	QAction*			m_pCleanUpProcesses;
	QAction*			m_pCleanUpMsgLog;
	QAction*			m_pCleanUpResLog;
	QAction*			m_pCleanUpApiLog;
	QToolButton*		m_pCleanUpButton;
	QAction*			m_pKeepTerminated;

	QMenu*				m_pMenuOptions;
	QAction*			m_pMenuSettings;
	QAction*			m_pMenuResetMsgs;
	QAction*			m_pEditIni;
	QAction*			m_pReloadIni;
	QAction*			m_pEnableMonitoring;
	QAction*			m_pEnableLogging;

	QMenu*				m_pMenuHelp;
	QAction*			m_pSupport;
	QAction*			m_pForum;
	QAction*			m_pManual;
	QAction*			m_pUpdate;
	QAction*			m_pAbout;
	QAction*			m_pAboutQt;

	QSystemTrayIcon*	m_pTrayIcon;
	QMenu*				m_pTrayMenu;
	bool				m_bIconEmpty;
	bool				m_bIconDisabled;

	bool				m_bExit;

	CProgressDialog*	m_pProgressDialog;
	CPopUpWindow*		m_pPopUpWindow;

	void				SetDarkTheme(bool bDark);
	QString				m_DefaultStyle;
	QPalette			m_DefaultPalett;

	void				LoadLanguage();
	QTranslator			m_Translator;
	QByteArray			m_Translation;
public:
	quint32				m_LanguageId;
};

extern CSandMan* theGUI;