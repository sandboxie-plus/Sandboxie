#include "stdafx.h"
#include "SplitTreeView.h"

CSplitTreeView::CSplitTreeView(QAbstractItemModel* pModel, QWidget *parent) : QWidget(parent)
{
	m_pModel = pModel;

	m_pMainLayout = new QHBoxLayout();
	m_pMainLayout->setMargin(0);
	this->setLayout(m_pMainLayout);


	m_pSplitter = new QSplitter();
	m_pSplitter->setOrientation(Qt::Horizontal);
	m_pMainLayout->addWidget(m_pSplitter);
	
#ifdef WIN32
	QStyle* pStyle = QStyleFactory::create("windows");
#endif

	m_LockSellection = 0;

	// Tree
	m_pTree = new QTreeView();

	m_pOneModel = new COneColumnModel();
	m_pOneModel->setSourceModel(m_pModel);
	m_pTree->setModel(m_pOneModel);
	//m_pTree->setModel(m_pSortProxy);

	m_pTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
#ifdef WIN32
	m_pTree->setStyle(pStyle);
#endif
	//m_pTree->setSortingEnabled(true);
	m_pTree->setSortingEnabled(false);
	m_pTree->setUniformRowHeights(true);
	m_pTree->header()->setSortIndicatorShown(true);
	m_pTree->header()->setSectionsClickable(true);
	connect(m_pTree->header(), SIGNAL(sectionClicked(int)), this, SLOT(OnTreeCustomSortByColumn(int)));

	m_pTree->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pTree, SIGNAL(customContextMenuRequested( const QPoint& )), this, SIGNAL(MenuRequested(const QPoint &)));

	m_pSplitter->addWidget(m_pTree);
	//m_pSplitter->setCollapsible(0, false);
	m_pSplitter->setStretchFactor(0, 0);
	//


	// List
	// Note: It would be convinient to use QTreeViewEx here but qt does not scale well when there are too many columns
	//			hence we will add and remove columns at the model level directly.
	//			This way we can get out operational CPU usage to be quite comparable with TaskInfo na other advanced task managers
	//		 Plus there are to many columns to cram them into one simple context menu :-)
	//m_pList = new QTreeViewEx();
	m_pList = new QTreeView();

	m_pList->setModel(m_pModel);

	m_pList->setSelectionMode(QAbstractItemView::ExtendedSelection);
#ifdef WIN32
	m_pList->setStyle(pStyle);
#endif
	m_pList->setSortingEnabled(true);
	m_pList->setUniformRowHeights(true);
	connect(m_pList->header(), SIGNAL(sectionClicked(int)), this, SLOT(OnListCustomSortByColumn(int)));

	m_pList->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pList, SIGNAL(customContextMenuRequested( const QPoint& )), this, SIGNAL(MenuRequested(const QPoint &)));

	m_pSplitter->addWidget(m_pList);
	m_pSplitter->setCollapsible(1, false);
	m_pSplitter->setStretchFactor(1, 1);
	// 

	connect(m_pSplitter, SIGNAL(splitterMoved(int,int)), this, SLOT(OnSplitterMoved(int,int)));

	// Link selections
	//m_pTree->setSelectionModel(m_pList->selectionModel()); // this works only when booth views share the same data model

	connect(m_pTree->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(OnTreeSelectionChanged(QItemSelection,QItemSelection)));
	connect(m_pList->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(OnListSelectionChanged(QItemSelection,QItemSelection)));

	connect(m_pTree->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(OnTreeCurrentChanged(QModelIndex,QModelIndex)));
	connect(m_pList->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(OnListCurrentChanged(QModelIndex,QModelIndex)));

	m_bTreeEnabled = true;
	m_pList->setColumnHidden(0, true);

	// link expansion
	connect(m_pTree, SIGNAL(expanded(const QModelIndex)), this, SLOT(OnExpandTree(const QModelIndex)));
	connect(m_pTree, SIGNAL(collapsed(const QModelIndex)), this, SLOT(OnCollapseTree(const QModelIndex)));
	//connect(m_pList, SIGNAL(expanded(const QModelIndex)), this, SLOT(expand(const QModelIndex)));
	//connect(m_pList, SIGNAL(collapsed(const QModelIndex)), this, SLOT(collapse(const QModelIndex)));

	// link scrollbars
	m_pTree->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_pTree->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	m_pList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	connect(m_pList->verticalScrollBar(), SIGNAL(valueChanged(int)), m_pTree->verticalScrollBar(), SLOT(setValue(int)));
	connect(m_pTree->verticalScrollBar(), SIGNAL(valueChanged(int)), m_pList->verticalScrollBar(), SLOT(setValue(int)));

	connect(m_pTree, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnClickedTree(const QModelIndex&)));
	connect(m_pList, SIGNAL(clicked(const QModelIndex&)), this, SIGNAL(clicked(const QModelIndex&)));

	connect(m_pTree, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(OnDoubleClickedTree(const QModelIndex&)));
	connect(m_pList, SIGNAL(doubleClicked(const QModelIndex&)), this, SIGNAL(doubleClicked(const QModelIndex&)));

	//QTimer::singleShot(0, this, [this]() {
	//	emit TreeEnabled(m_bTreeEnabled);
	//});
}

CSplitTreeView::~CSplitTreeView()
{

}

int CSplitTreeView::GetTreeWidth() const
{
	QList<int> sizes = m_pSplitter->sizes();
	return sizes[0];
}

void CSplitTreeView::SetTreeWidth(int Width)
{
	QList<int> sizes = m_pSplitter->sizes();
	int total = sizes[0] + sizes[1];
	sizes[0] = Width;
	sizes[1] = total - Width;
}

void CSplitTreeView::OnSplitterMoved(int pos, int index)
{
	if (index != 1)
		return;

	//if ((pos > 0) == m_bTreeEnabled)
	//	return;
	//m_bTreeEnabled = (pos > 0);

	m_pList->setColumnHidden(0, pos > 0);

	//emit TreeEnabled(m_bTreeEnabled);
}

void CSplitTreeView::OnExpandTree(const QModelIndex& index)
{
	m_pList->expand(m_pOneModel->mapToSource(index));
}

void CSplitTreeView::OnCollapseTree(const QModelIndex& index)
{
	m_pList->collapse(m_pOneModel->mapToSource(index));
}

void CSplitTreeView::OnClickedTree(const QModelIndex& Index)
{
	emit clicked(m_pOneModel->mapToSource(Index));
}

void CSplitTreeView::OnDoubleClickedTree(const QModelIndex& Index)
{
	emit doubleClicked(m_pOneModel->mapToSource(Index));
}

void CSplitTreeView::expand(const QModelIndex &index) 
{ 
	m_pTree->expand(m_pOneModel->mapFromSource(index)); 
}

void CSplitTreeView::collapse(const QModelIndex &index) 
{ 
	m_pTree->collapse(m_pOneModel->mapFromSource(index)); 
}

void CSplitTreeView::OnTreeSelectionChanged(const QItemSelection& Selected, const QItemSelection& Deselected)
{
	if (m_LockSellection)
		return;
	m_LockSellection = 1;
	QItemSelection SelectedItems;
	foreach(const QModelIndex& Index, m_pTree->selectionModel()->selectedIndexes())
	{
		QModelIndex ModelIndex = m_pOneModel->mapToSource(Index);
		
		QModelIndex ModelL = m_pModel->index(ModelIndex.row(), 0, ModelIndex.parent());
		QModelIndex ModelR = m_pModel->index(ModelIndex.row(), m_pModel->columnCount()-1, ModelIndex.parent());
		
		SelectedItems.append(QItemSelectionRange(ModelL, ModelR));
	}
	m_pList->selectionModel()->select(SelectedItems, QItemSelectionModel::ClearAndSelect);
	/*
	foreach(const QModelIndex& Index, Selected.indexes())
	{
		QModelIndex ModelIndex = m_pOneModel->mapToSource(Index);
		
		QModelIndex ModelL = m_pModel->index(ModelIndex.row(), 0, ModelIndex.parent());
		QModelIndex ModelR = m_pModel->index(ModelIndex.row(), m_pModel->columnCount()-1, ModelIndex.parent());
		
		m_pList->selectionModel()->select(QItemSelection(ModelL, ModelR), QItemSelectionModel::Select);
	}
	foreach(const QModelIndex& Index, Deselected.indexes())
	{
		QModelIndex ModelIndex = m_pOneModel->mapToSource(Index);
		
		QModelIndex ModelL = m_pModel->index(ModelIndex.row(), 0, ModelIndex.parent());
		QModelIndex ModelR = m_pModel->index(ModelIndex.row(), m_pModel->columnCount()-1, ModelIndex.parent());
		
		m_pList->selectionModel()->select(QItemSelection(ModelL, ModelR), QItemSelectionModel::Deselect);
	}*/
	m_LockSellection = 0;
}

void CSplitTreeView::OnListSelectionChanged(const QItemSelection& Selected, const QItemSelection& Deselected)
{
	if (m_LockSellection != 2)
		emit selectionChanged(Selected, Deselected);

	if (m_LockSellection)
		return;
	m_LockSellection = 1;
	QItemSelection SelectedItems;
	foreach(const QModelIndex& Index, m_pList->selectionModel()->selectedIndexes())
	{
		QModelIndex ModelIndex = m_pOneModel->mapFromSource(Index);
		if (ModelIndex.column() != 0)
			continue;
		
		QModelIndex Model = m_pOneModel->index(ModelIndex.row(), 0, ModelIndex.parent());

		SelectedItems.append(QItemSelectionRange(Model));
	}
	m_pTree->selectionModel()->select(SelectedItems, QItemSelectionModel::ClearAndSelect);
	/*foreach(const QModelIndex& Index, Selected.indexes())
	{
		QModelIndex ModelIndex = m_pOneModel->mapFromSource(Index);
		if (ModelIndex.column() != 0)
			continue;
		m_pTree->selectionModel()->select(ModelIndex, QItemSelectionModel::Select);
	}
	foreach(const QModelIndex& Index, Deselected.indexes())
	{
		QModelIndex ModelIndex = m_pOneModel->mapFromSource(Index);
		if (ModelIndex.column() != 0)
			continue;
		m_pTree->selectionModel()->select(ModelIndex, QItemSelectionModel::Deselect);
	}*/
	m_LockSellection = 0;
}

void CSplitTreeView::OnTreeCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
	if (m_LockSellection)
		return;
	m_LockSellection = 2;
	int hPos = m_pList->horizontalScrollBar()->value(); // fix horizontalScrollBar position reset on selection
	m_pList->selectionModel()->setCurrentIndex(m_pOneModel->mapToSource(current), QItemSelectionModel::SelectCurrent);
	m_pList->horizontalScrollBar()->setValue(hPos);
	m_LockSellection = 0;
	emit currentChanged(m_pOneModel->mapToSource(current), m_pOneModel->mapToSource(previous));
}

void CSplitTreeView::OnListCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
	if (m_LockSellection)
		return;
	m_LockSellection = 2;
	m_pTree->selectionModel()->setCurrentIndex(m_pOneModel->mapFromSource(current), QItemSelectionModel::SelectCurrent);
	m_LockSellection = 0;
	emit currentChanged(current, previous);
}

QModelIndexList CSplitTreeView::selectedRows() const
{
	int Column = 0;
	for (int i = 0; i < m_pModel->columnCount(); i++)
	{
		if (!m_pList->isColumnHidden(i))
		{
			Column = i;
			break;
		}
	}

	QModelIndexList IndexList;
	foreach(const QModelIndex& Index, m_pList->selectionModel()->selectedIndexes())
	{
		if (Index.column() == Column)
			IndexList.append(Index);
	}
	return IndexList;
}

void CSplitTreeView::SetTree(bool bSet)
{
	m_bTreeEnabled = bSet;
	emit TreeEnabled(m_bTreeEnabled);
}

void CSplitTreeView::OnTreeCustomSortByColumn(int column)
{
	Qt::SortOrder order = m_pTree->header()->sortIndicatorOrder();
	if (order == Qt::AscendingOrder)
		SetTree(!m_bTreeEnabled);
	m_pList->sortByColumn(column, order);
	m_pTree->header()->setSortIndicatorShown(true);
}

void CSplitTreeView::OnListCustomSortByColumn(int column)
{
	Qt::SortOrder order = m_pList->header()->sortIndicatorOrder();
	if (column == 0)
		SetTree(!m_bTreeEnabled);
	else if (m_bTreeEnabled)
		SetTree(false);
	m_pTree->header()->setSortIndicatorShown(false);
}

static const qint32 CSplitTreeViewMagic = 'STVs';

QByteArray CSplitTreeView::saveState() const
{
    int version = 1;
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << qint32(CSplitTreeViewMagic);
    stream << qint32(version);
	stream << m_pList->header()->saveState();
	stream << m_pSplitter->saveState();
	stream << m_bTreeEnabled;
    
    return data;
}

bool CSplitTreeView::restoreState(const QByteArray &state)
{
    int version = 1;
    QByteArray sd = state;
    QDataStream stream(&sd, QIODevice::ReadOnly);

    qint32 marker;
    stream >> marker;
	qint32 v;
    stream >> v;
    if (marker != CSplitTreeViewMagic || v > version)
        return false;

	QByteArray header;
    stream >> header;
	m_pList->header()->restoreState(header);

	QByteArray splitter;
    stream >> splitter;
	m_pSplitter->restoreState(splitter);

    stream >> m_bTreeEnabled;
	emit TreeEnabled(m_bTreeEnabled);

	m_pTree->header()->setSortIndicatorShown(m_pList->header()->sortIndicatorSection() == 0);

	QList<int> sizes = m_pSplitter->sizes();
	for(int i=0; i < sizes.count(); i++)
		OnSplitterMoved(sizes[i], i);

    return true;
}
