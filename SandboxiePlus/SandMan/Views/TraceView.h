#pragma once

#include "../../MiscHelpers/Common/PanelView.h"
#include "../../MiscHelpers/Common/TreeviewEx.h"
#include "../Models/SbieModel.h"
#include "../Models/TraceModel.h"
#include "../Models/MonitorModel.h"
#include "../../MiscHelpers/Common/SortFilterProxyModel.h"
#include "StackView.h"


class CTraceTree : public CPanelView
{
	Q_OBJECT
public:

	CTraceTree(QWidget* parent = 0);
	~CTraceTree();

	virtual QMenu*				GetMenu()	{ return m_pMenu; }

	virtual QTreeViewEx*		GetTree()	{ return m_pTreeList; }
	virtual QTreeView*			GetView()	{ return m_pTreeList; }
	virtual QAbstractItemModel* GetModel()	{ return m_pTreeList->model(); }

	CTraceModel*		m_pTraceModel;

public slots:
	void				SetFilter(const QString& Exp, int iOptions = 0, int Column = -1);

	void				ItemSelection(const QItemSelection& selected, const QItemSelection& deselected);

signals:
	void				FilterChanged();

protected:
	friend class CTraceView;

	QString				GetFilterExp() const { return m_FilterExp; }

	QVBoxLayout*		m_pMainLayout;
	QSplitter*			m_pSplitter;
	QTreeViewEx*		m_pTreeList;
	CStackView*			m_pStackView;

	//QRegularExpression	m_FilterExp;
	QString				m_FilterExp;
	bool				m_bHighLight;
	//int					m_FilterCol;
};

class CMonitorList : public CPanelWidget<QTreeViewEx>
{
public:

	CMonitorList(QWidget* parent = 0);
	~CMonitorList();

	CSortFilterProxyModel* m_pSortProxy;
	CMonitorModel*		m_pMonitorModel;
};

class CTraceView : public QWidget
{
	Q_OBJECT
public:
	CTraceView(bool bStandAlone, QWidget* parent = 0);
	~CTraceView();

	void				AddAction(QAction* pAction);

	void				SetEnabled(bool bSet);

public slots:
	void				Refresh();
	void				Clear();

	void				OnSetTree();
	void				OnObjTree();
	void				OnSetMode();
	void				OnSetPidFilter();
	void				OnSetTidFilter();
	void				OnSetFilter();
	void				OnShowStack();

private slots:
	void				UpdateFilters();
	void				OnFilterChanged();

	void				SaveToFile();

protected:
	void				timerEvent(QTimerEvent* pEvent);
	int					m_uTimerID;

	struct SProgInfo
	{
		QString Name;
		QSet<quint32> Threads;
	};

	QMap<quint32, SProgInfo>m_PidMap;
	quint64					m_LastID;
	int						m_LastCount;
	bool					m_bUpdatePending;
	QVector<CTraceEntryPtr> m_TraceList;
	QMap<QString, CMonitorEntryPtr> m_MonitorMap;

protected:
	bool				m_FullRefresh;

	quint32				m_FilterPid;
	quint32				m_FilterTid;
	QList<quint32>		m_FilterTypes;
	quint32				m_FilterStatus;
	void*				m_pCurrentBox;

	QVBoxLayout*		m_pMainLayout;

	CTraceTree*			m_pTrace;
	CMonitorList*		m_pMonitor;

	QToolBar*			m_pTraceToolBar;
	QAction*			m_pMonitorMode;
	QAction*			m_pTraceTree;
	QAction*			m_pObjectTree;
	QComboBox*			m_pTracePid;
	QComboBox*			m_pTraceTid;
	class QCheckList*	m_pTraceType;
	QComboBox*			m_pTraceStatus;
	QAction*			m_pAllBoxes;
	QAction*			m_pShowStack;
	QAction*			m_pSaveToFile;

	QWidget*			m_pView;
	QStackedLayout*		m_pLayout;
};


////////////////////////////////////////////////////////////////////////////////////////
// CTraceWindow

class CTraceWindow : public QDialog
{
	Q_OBJECT

public:
	CTraceWindow(QWidget *parent = Q_NULLPTR);
	~CTraceWindow();

signals:
	void		Closed();

protected:
	void		closeEvent(QCloseEvent *e);
};
