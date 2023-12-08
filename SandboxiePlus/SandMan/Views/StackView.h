#pragma once

#include "../../MiscHelpers/Common/PanelView.h"
#include "../../MiscHelpers/Common/TreeviewEx.h"
#include "../Models/SbieModel.h"
#include "../Models/TraceModel.h"
#include "../Models/MonitorModel.h"
#include "../../MiscHelpers/Common/SortFilterProxyModel.h"


class CFinder;

class CStackView : public CPanelView
{
	Q_OBJECT
public:
	CStackView(QWidget *parent = 0);
	virtual ~CStackView();

public slots:
	void					Clear()			{ m_pStackList->clear(); }
	void					Invalidate();
	void					ShowStack(const QVector<quint64>& Stack, const CBoxedProcessPtr& pProcess);

	//void					OnMenu(const QPoint &point);

	void					SetFilter(const QRegularExpression& Exp, bool bHighLight = false, int Col = -1); // -1 = any
	void					SetFilter(const QString& Exp, int iOptions = 0, int Col = -1); // -1 = any

protected:
	//virtual void				OnMenu(const QPoint& Point);
	virtual QTreeView*			GetView()	{ return m_pStackList; }
	virtual QAbstractItemModel* GetModel()	{ return m_pStackList->model(); }

private:
	enum EStackColumns
	{
		eStack = 0,
		eSymbol,
		eCount
	};

	QVBoxLayout*			m_pMainLayout;

	bool					m_bIsInvalid;
	QTreeWidgetEx*			m_pStackList;

	CFinder*				m_pFinder;

	//QMenu*					m_pMenu;
};
