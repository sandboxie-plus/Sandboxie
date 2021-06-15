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

private slots:
	void				UpdateFilters();

protected:
	CTraceModel*		m_pTraceModel;
	CTraceFilterProxyModel* m_pSortProxy;

	QToolBar*			m_pTraceToolBar;
	QAction*			m_pTraceTree;
	QCheckBox*			m_pOnlyCurrent;
	QComboBox*			m_pTracePid;
	QComboBox*			m_pTraceTid;

};