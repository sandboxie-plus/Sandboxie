#pragma once

#include <QtWidgets/QMainWindow>

#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/TreeViewEx.h"
#include "../MiscHelpers/Common/PanelView.h"
#include "Models/ResMonModel.h"

#define VERSION_MJR		0
#define VERSION_MIN 	1
#define VERSION_REV 	0
#define VERSION_UPD 	0


#include "../QSbieAPI/SbieAPI.h"

class CSbieView;
class CApiLog;

class CSandMan : public QMainWindow
{
	Q_OBJECT

public:
	CSandMan(QWidget *parent = Q_NULLPTR);
	virtual ~CSandMan();

	static QString		GetVersion();

	static void			CheckResults(QList<SB_STATUS> Results);

protected:
	void				closeEvent(QCloseEvent *e);
	void				timerEvent(QTimerEvent* pEvent);
	int					m_uTimerID;

	CApiLog*			m_ApiLog;

public slots:
	void				OnLogMessage(const QString& Message);
	void				OnApiLogEntry(const QString& Message);

private slots:
	void				OnSelectionChanged();

	//void				OnResetColumns();
	//void				OnColumnsChanged();
	//void				OnMenu(const QPoint& Point);

	void				OnNewBox();
	void				OnEmptyAll();

	void				OnCleanUp();
	void				OnSetKeep();

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

	//QTreeViewEx*		m_pBoxTree;
	//CSbieModel*			m_pBoxModel;
	//QSortFilterProxyModel* m_pSortProxy;
	CSbieView*			m_pBoxView;


	QTabWidget*			m_pLogTabs;

	CPanelWidgetEx*		m_pMessageLog;
	CPanelViewImpl<CResMonModel>* m_pResourceLog;
	CResMonModel*		m_pResMonModel;
	CPanelWidgetEx*		m_pApiLog;


	QMenu*				m_pMenuFile;
	QAction*			m_pMenuNew;
	QAction*			m_pMenuEmptyAll;
	QAction*			m_pMenuExit;

	QMenu*				m_pMenuView;
	QAction*			m_pCleanUp;
	QAction*			m_pKeepTerminated;

	QMenu*				m_pMenuOptions;
	QAction*			m_pEditIni;
	QAction*			m_pReloadIni;
	QAction*			m_pEnableMonitoring;
	QAction*			m_pEnableLogging;

	QMenu*				m_pMenuHelp;
	QAction*			m_pMenuAbout;
	QAction*			m_pMenuSupport;
	QAction*			m_pMenuAboutQt;

	QSystemTrayIcon*	m_pTrayIcon;
	QMenu*				m_pTrayMenu;

	bool				m_bExit;
};
