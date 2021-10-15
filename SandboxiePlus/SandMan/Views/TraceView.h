#pragma once

#include "../../MiscHelpers/Common/PanelView.h"
#include "../../MiscHelpers/Common/TreeviewEx.h"
#include "../Models/SbieModel.h"

class CTraceFilterProxyModel;
class CTraceModel;

class CTraceView : public CPanelWidget<QTreeViewEx>
{
	Q_OBJECT
public:
	CTraceView(QWidget* parent = 0);
	~CTraceView();

	void				Refresh();
	void				Clear();

public slots:
	void				OnSetTree();
	void				OnSetPidFilter();
	void				OnSetTidFilter();
	void				OnSetFilter();

private slots:
	void				UpdateFilters();
	void				SetFilter(const QRegExp& Exp, bool bHighLight = false, int Col = -1); // -1 = any
	void				SelectNext();

protected:
	friend int CTraceView__Filter(const CTraceEntryPtr& pEntry, void* params);
	CTraceModel*		m_pTraceModel;
	//CTraceFilterProxyModel* m_pSortProxy;
	bool				m_FullRefresh;

	QRegExp				m_FilterExp;
	bool				m_bHighLight;
	//int					m_FilterCol;
	quint32				m_FilterPid;
	quint32				m_FilterTid;
	quint32				m_FilterType;
	quint32				m_FilterStatus;
	void*				m_pCurrentBox;

	QToolBar*			m_pTraceToolBar;
	QAction*			m_pTraceTree;
	QCheckBox*			m_pAllBoxes;
	QComboBox*			m_pTracePid;
	QComboBox*			m_pTraceTid;
	QComboBox*			m_pTraceType;
	QComboBox*			m_pTraceStatus;

};