#pragma once

#include <QtWidgets/QMainWindow>

#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/TreeViewEx.h"
#include "../MiscHelpers/Common/PanelView.h"
#include "../MiscHelpers/Common/ProgressDialog.h"
#include "Models/ResMonModel.h"
#include "Models/ApiMonModel.h"
#include <QTranslator>

#define VERSION_MJR		0
#define VERSION_MIN 	3
#define VERSION_REV 	5
#define VERSION_UPD 	0


//#include "../QSbieAPI/SbieAPI.h"
#include "SbiePlusAPI.h"

class CSbieView;
class CApiLog;
class CBoxBorder;

class CSandMan : public QMainWindow
{
	Q_OBJECT

public:
	CSandMan(QWidget *parent = Q_NULLPTR);
	virtual ~CSandMan();

	CProgressDialog*	GetProgressDialog() { return m_pProgressDialog; }

	static QString		GetVersion();

	static void			CheckResults(QList<SB_STATUS> Results);

protected:
	SB_STATUS			ConnectSbie();
	SB_STATUS			ConnectSbieImpl();
	SB_STATUS			DisconnectSbie();
	SB_STATUS			StopSbie(bool andRemove = false);

	bool				IsFullyPortable();

	void				closeEvent(QCloseEvent *e);
	void				timerEvent(QTimerEvent* pEvent);
	int					m_uTimerID;
	bool				m_bConnectPending;
	bool				m_bStopPending;
	CBoxBorder*			m_pBoxBorder;

	CApiLog*			m_ApiLog;
	

public slots:
	void				OnMessage(const QString&);

	void				OnStatusChanged();
	void				OnLogMessage(const QString& Message, bool bNotify = false);

	void				OnNotAuthorized(bool bLoginRequired, bool& bRetry);

	void				UpdateSettings();

private slots:
	void				OnSelectionChanged();

	void				OnMenuHover(QAction* action);

	void				OnNewBox();
	void				OnEmptyAll();
	void				OnMaintenance();

	void				OnCleanUp();
	void				OnSetKeep();

	void				OnSettings();
	void				OnEditIni();
	void				OnReloadIni();
	void				OnSetMonitoring();
	void				OnSetLogging();

	void				OnExit();
	void				OnAbout();

	void				OnSysTray(QSystemTrayIcon::ActivationReason Reason);

private:
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
	QMenu*				m_pCleanUpMenu;
	QAction*			m_pCleanUpProcesses;
	QAction*			m_pCleanUpMsgLog;
	QAction*			m_pCleanUpResLog;
	QAction*			m_pCleanUpApiLog;
	QToolButton*		m_pCleanUpButton;
	QAction*			m_pKeepTerminated;

	QMenu*				m_pMenuOptions;
	QAction*			m_pMenuSettings;
	QAction*			m_pEditIni;
	QAction*			m_pReloadIni;
	QAction*			m_pEnableMonitoring;
	QAction*			m_pEnableLogging;

	QMenu*				m_pMenuHelp;
	QAction*			m_pAbout;
	QAction*			m_pSupport;
	QAction*			m_pAboutQt;

	QSystemTrayIcon*	m_pTrayIcon;
	QMenu*				m_pTrayMenu;
	bool				m_bIconEmpty;

	bool				m_bExit;

	CProgressDialog*	m_pProgressDialog;

	void				SetDarkTheme(bool bDark);
	QString				m_DefaultStyle;
	QPalette			m_DefaultPalett;

	void				LoadLanguage();
	QTranslator			m_Translator;
	QByteArray			m_Translation;
};

extern CSandMan* theGUI;