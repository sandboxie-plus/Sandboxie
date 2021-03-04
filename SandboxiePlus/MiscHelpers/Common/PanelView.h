#pragma once

#include "../mischelpers_global.h"

class MISCHELPERS_EXPORT CPanelView : public QWidget
{
	Q_OBJECT
public:
	CPanelView(QWidget *parent = 0);
	virtual ~CPanelView();

	static void					SetSimpleFormat(bool bSimple) { m_SimpleFormat = bSimple; }
	static void					SetMaxCellWidth(int iMaxWidth) { m_MaxCellWidth = iMaxWidth; }
	static void					SetCellSeparator(const QString& Sep) { m_CellSeparator = Sep; }

	static QString				m_CopyCell;
	static QString				m_CopyRow;
	static QString				m_CopyPanel;

protected slots:
	virtual void				OnMenu(const QPoint& Point);

	virtual void				OnCopyCell();
	virtual void				OnCopyRow();
	virtual void				OnCopyPanel();


	virtual QTreeView*			GetView() = 0;
	virtual QAbstractItemModel* GetModel() = 0;
	virtual QModelIndex			MapToSource(const QModelIndex& Model) { return Model; }
	static QModelIndexList		MapToSource(QModelIndexList Indexes, QSortFilterProxyModel* pProxy)	{ 
		for (int i = 0; i < Indexes.count(); i++)
			Indexes[i] = pProxy->mapToSource(Indexes[i]);
		return Indexes;
	}

	virtual void				AddPanelItemsToMenu(bool bAddSeparator = true);

	virtual void				ForceColumn(int column, bool bSet = true) { if (bSet) m_ForcedColumns.insert(column); else m_ForcedColumns.remove(column); }

	virtual QStringList			CopyHeader();
	virtual QStringList			CopyRow(const QModelIndex& ModelIndex, int Level = 0);
	virtual void				RecursiveCopyPanel(const QModelIndex& ModelIndex, QList<QStringList>& Rows, int Level = 0);

protected:
	void						FormatAndCopy(QList<QStringList> Rows, bool Headder = true);

	QMenu*						m_pMenu;

	QAction*					m_pCopyCell;
	QAction*					m_pCopyRow;
	QAction*					m_pCopyPanel;

	//bool						m_CopyAll;
	QSet<int>					m_ForcedColumns;
	static bool					m_SimpleFormat;
	static int					m_MaxCellWidth;
	static QString				m_CellSeparator;
};

template <class T>
class CPanelWidget : public CPanelView
{
public:
	CPanelWidget(QWidget *parent = 0) : CPanelView(parent)
	{
		m_pMainLayout = new QVBoxLayout();
		m_pMainLayout->setMargin(0);
		this->setLayout(m_pMainLayout);

		m_pTreeList = new T();
		m_pTreeList->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(m_pTreeList, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(OnMenu(const QPoint &)));
		m_pMainLayout->addWidget(m_pTreeList);
		m_pTreeList->setMinimumHeight(50);
		AddPanelItemsToMenu();

		m_pLastAction = m_pMenu->actions()[0];
	}

	virtual QMenu*				GetMenu()	{ return m_pMenu; }
	virtual void				AddAction(QAction* pAction) { m_pMenu->insertAction(m_pLastAction, pAction); }

	virtual T*					GetTree()	{ return m_pTreeList; }
	virtual QTreeView*			GetView()	{ return m_pTreeList; }
	virtual QAbstractItemModel* GetModel()	{ return m_pTreeList->model(); }

	virtual QVBoxLayout*		GetLayout()	{ return m_pMainLayout; }

protected:
	QVBoxLayout*			m_pMainLayout;

	T*						m_pTreeList;

	QAction*				m_pLastAction;
};

#include "TreeWidgetEx.h"
#include "Finder.h"

class MISCHELPERS_EXPORT CPanelWidgetEx : public CPanelWidget<QTreeWidgetEx>
{
	Q_OBJECT

public:
	CPanelWidgetEx(QWidget *parent = 0) : CPanelWidget<QTreeWidgetEx>(parent) 
	{
		m_pFinder = new CFinder(NULL, this, false);
		m_pMainLayout->addWidget(m_pFinder);
		QObject::connect(m_pFinder, SIGNAL(SetFilter(const QRegExp&, bool, int)), this, SLOT(SetFilter(const QRegExp&, bool, int)));
	}

	static void ApplyFilter(QTreeWidgetEx* pTree, QTreeWidgetItem* pItem, const QRegExp& Exp/*, bool bHighLight = false, int Col = -1*/)
	{
		for (int j = 0; j < pTree->columnCount(); j++)
			pItem->setBackground(j, !Exp.isEmpty() && pItem->text(j).contains(Exp) ? Qt::yellow : Qt::white);

		for (int i = 0; i < pItem->childCount(); i++)
		{
			ApplyFilter(pTree, pItem->child(i), Exp/*, bHighLight, Col*/);
		}
	}

	static void ApplyFilter(QTreeWidgetEx* pTree, const QRegExp& Exp/*, bool bHighLight = false, int Col = -1*/)
	{
		for (int i = 0; i < pTree->topLevelItemCount(); i++)
			ApplyFilter(pTree, pTree->topLevelItem(i), Exp/*, bHighLight, Col*/);
	}

private slots:
	void SetFilter(const QRegExp& Exp, bool bHighLight = false, int Col = -1) // -1 = any
	{
		ApplyFilter(m_pTreeList, Exp);
	}

private:
	
	CFinder*				m_pFinder;
};

#include "TreeViewEx.h"
#include "SortFilterProxyModel.h"

class CPanelViewEx: public CPanelWidget<QTreeViewEx>
{
public:
	CPanelViewEx(QAbstractItemModel* pModel, QWidget *parent = 0) : CPanelWidget<QTreeViewEx>(parent)
	{
		m_pModel = pModel;

		m_pSortProxy = new CSortFilterProxyModel(false, this);
		m_pSortProxy->setSortRole(Qt::EditRole);
		m_pSortProxy->setSourceModel(m_pModel);
		m_pSortProxy->setDynamicSortFilter(true);

		m_pTreeList->setModel(m_pSortProxy);
		((CSortFilterProxyModel*)m_pSortProxy)->setView(m_pTreeList);
		

		m_pTreeList->setSelectionMode(QAbstractItemView::ExtendedSelection);
#ifdef WIN32
		QStyle* pStyle = QStyleFactory::create("windows");
		m_pTreeList->setStyle(pStyle);
#endif
		m_pTreeList->setExpandsOnDoubleClick(false);
		m_pTreeList->setSortingEnabled(true);

		m_pTreeList->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(m_pTreeList, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnMenu(const QPoint &)));

		m_pTreeList->setColumnReset(1);
		//connect(m_pTreeList, SIGNAL(ResetColumns()), m_pTreeList, SLOT(OnResetColumns()));
		//connect(m_pBoxTree, SIGNAL(ColumnChanged(int, bool)), this, SLOT(OnColumnsChanged()));

		m_pMainLayout->addWidget(CFinder::AddFinder(m_pTreeList, m_pSortProxy));
	}

protected:
	QAbstractItemModel*		m_pModel;
	QSortFilterProxyModel*	m_pSortProxy;
};