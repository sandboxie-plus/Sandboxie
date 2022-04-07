#pragma once

#include "../../MiscHelpers/Common/PanelView.h"
#include "../../MiscHelpers/Common/TreeviewEx.h"
#include "../Models/SbieModel.h"
#include "../Models/TraceModel.h"
#include "../Models/MonitorModel.h"

class CTraceFilterProxyModel;

class CTraceTree : public CPanelWidget<QTreeViewEx>
{
	Q_OBJECT
public:

	CTraceTree(QWidget* parent = 0);
	~CTraceTree();

	CTraceModel*		m_pTraceModel;

public slots:
	void				SetFilter(const QRegExp& Exp, bool bHighLight = false, int Column = -1) {
		emit FilterSet(Exp, bHighLight, Column);
	}
	void				SelectNext() {}

signals:
	void				FilterSet(const QRegExp& Exp, bool bHighLight = false, int Column = -1);
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
	CTraceView(QWidget* parent = 0);
	~CTraceView();

	void				Refresh();
	void				Clear();

	void				AddAction(QAction* pAction);

public slots:
	void				OnSetTree();
	void				OnSetMode();
	void				OnSetPidFilter();
	void				OnSetTidFilter();
	void				OnSetFilter();

private slots:
	void				UpdateFilters();
	void				SetFilter(const QRegExp& Exp, bool bHighLight = false, int Col = -1); // -1 = any

	void				SaveToFile();

protected:
	struct SProgInfo
	{
		QString Name;
		QSet<quint32> Threads;
	};

	QMap<quint32, SProgInfo>m_PidMap;
	quint64					m_LastID;
	int						m_LastCount;
	bool					m_bUpdatePending;
	QMap<QString, CMonitorEntryPtr> m_MonitorMap;

protected:
	friend int CTraceView__Filter(const CTraceEntryPtr& pEntry, void* params);

	bool				m_FullRefresh;

	QRegExp				m_FilterExp;
	bool				m_bHighLight;
	//int					m_FilterCol;
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
	QComboBox*			m_pTracePid;
	QComboBox*			m_pTraceTid;
	class QCheckList*	m_pTraceType;
	QComboBox*			m_pTraceStatus;
	QAction*			m_pAllBoxes;
	QAction*			m_pSaveToFile;

	QWidget*			m_pView;
	QStackedLayout*		m_pLayout;
};