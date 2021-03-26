#include "stdafx.h"
#include "TraceView.h"
#include "..\SandMan.h"
#include "../QSbieAPI/SbieAPI.h"
#include "..\Models\TraceModel.h"
#include "..\..\MiscHelpers\Common\Common.h"

class CTraceFilterProxyModel : public CSortFilterProxyModel
{
public:
	CTraceFilterProxyModel(QObject* parrent = 0) : CSortFilterProxyModel(false, parrent)
	{
		m_FilterPid = 0;
		m_FilterTid = 0;
	}

	bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
	{
		CTraceModel* pTraceModel = (CTraceModel*)sourceModel();

		QModelIndex index = pTraceModel->index(source_row, 0, source_parent);
		//CTraceEntryPtr pEntry = pTraceModel->GetEntry(index);
		//if (pEntry.data() == NULL) 
		{
			QVariant Id = pTraceModel->GetItemID(index);
			StrPair typeId = Split2(Id.toString(), "_");

			if (m_FilterPid != 0 && typeId.first == "pid") {
				if (m_FilterPid != typeId.second.toUInt())
					return false;
			}

			if (m_FilterTid != 0 && typeId.first == "tid") {
				if (m_FilterTid != typeId.second.toUInt())
					return false;
			}
		}

		return CSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
	}

	quint32		m_FilterPid;
	quint32		m_FilterTid;
};

CTraceView::CTraceView(QWidget* parent) : CPanelWidget<QTreeViewEx>(parent)
{
	//m_pTreeList->setItemDelegate(theGUI->GetItemDelegate());

	m_pTreeList->setSelectionMode(QAbstractItemView::ExtendedSelection);

	m_pTraceToolBar = new QToolBar();
	m_pTraceTree = m_pTraceToolBar->addAction(CSandMan::GetIcon("Tree"), tr("Show as task tree"), this, SLOT(OnSetTree()));
	m_pTraceTree->setCheckable(true);
	m_pTraceTree->setChecked(theConf->GetBool("Options/UseLogTree"));
	m_pTraceToolBar->addSeparator();
	m_pTraceToolBar->layout()->setSpacing(3);
	m_pTraceToolBar->addWidget(new QLabel(tr("PID:")));

	m_pTracePid = new QComboBox();
	m_pTracePid->addItem(tr("[All]"), 0);
	m_pTracePid->setMinimumWidth(225);
	connect(m_pTracePid, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSetPidFilter()));
	m_pTraceToolBar->addWidget(m_pTracePid);
	m_pTraceToolBar->addWidget(new QLabel(tr("TID:")));

	m_pTraceTid = new QComboBox();
	m_pTraceTid->addItem(tr("[All]"), 0);
	m_pTraceTid->setMinimumWidth(75);
	connect(m_pTraceTid, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSetTidFilter()));
	m_pTraceToolBar->addWidget(m_pTraceTid);

	m_pMainLayout->setSpacing(0);

	m_pMainLayout->insertWidget(0, m_pTraceToolBar);

	m_pTraceModel = new CTraceModel();
	m_pTraceModel->SetTree(m_pTraceTree->isChecked());
	connect(m_pTraceModel, SIGNAL(NewBranche()), this, SLOT(UpdateFilters()));

	m_pSortProxy = new CTraceFilterProxyModel(this);
	m_pSortProxy->setSortRole(Qt::EditRole);
	m_pSortProxy->setSourceModel(m_pTraceModel);
	m_pSortProxy->setDynamicSortFilter(true);

	m_pTreeList->setModel(m_pSortProxy);
	m_pSortProxy->setView(m_pTreeList);


	m_pTreeList->setSelectionMode(QAbstractItemView::ExtendedSelection);
#ifdef WIN32
	QStyle* pStyle = QStyleFactory::create("windows");
	m_pTreeList->setStyle(pStyle);
#endif
	m_pTreeList->setExpandsOnDoubleClick(false);
	m_pTreeList->setSortingEnabled(true);

	m_pTreeList->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pTreeList, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnMenu(const QPoint&)));

	m_pTreeList->setColumnReset(1);
	//connect(m_pTreeList, SIGNAL(ResetColumns()), m_pTreeList, SLOT(OnResetColumns()));
	//connect(m_pBoxTree, SIGNAL(ColumnChanged(int, bool)), this, SLOT(OnColumnsChanged()));

	m_pMainLayout->addWidget(CFinder::AddFinder(m_pTreeList, m_pSortProxy));


	QByteArray Columns = theConf->GetBlob("MainWindow/TraceLog_Columns");
	if (!Columns.isEmpty())
		((QTreeViewEx*)GetView())->OnResetColumns();
	else
		((QTreeViewEx*)GetView())->restoreState(Columns);
}

CTraceView::~CTraceView()
{
	theConf->SetBlob("MainWindow/TraceLog_Columns", GetView()->header()->saveState());
}

void CTraceView::Refresh()
{
	QList<CTraceEntryPtr> ResourceLog = theAPI->GetTrace();
	//m_pTraceModel->Sync(ResourceLog, Pids);
	QList<QVariant> Added = m_pTraceModel->Sync(ResourceLog);

	if (m_pTraceModel->IsTree())
	{
		QTimer::singleShot(100, this, [this, Added]() {
			CSortFilterProxyModel* pSortProxy = (CSortFilterProxyModel*)GetModel();
			foreach(const QVariant ID, Added) {
				m_pTreeList->expand(pSortProxy->mapFromSource(m_pTraceModel->FindIndex(ID)));
			}
		});
	}
}

void CTraceView::Clear()
{
	m_pTracePid->clear();
	m_pTracePid->addItem(tr("[All]"), 0);

	m_pTraceTid->clear();
	m_pTraceTid->addItem(tr("[All]"), 0);

	theAPI->ClearTrace();
	m_pTraceModel->Clear();
}

void CTraceView::OnSetTree()
{
	m_pTraceModel->SetTree(m_pTraceTree->isChecked());
	m_pTraceModel->Clear();
	theConf->SetValue("Options/UseLogTree", m_pTraceTree->isChecked());
}

void CTraceView::UpdateFilters()
{
	quint32 cur_pid = m_pTracePid->currentData().toUInt();

	QMap<quint32, CTraceModel::SProgInfo> pids = m_pTraceModel->GetPids();
	foreach(quint32 pid, pids.uniqueKeys()) {
		CTraceModel::SProgInfo& Info = pids[pid];

		if(m_pTracePid->findData(pid) == -1)
			m_pTracePid->addItem(tr("%1 (%2)").arg(Info.Name).arg(pid), pid);

		if (cur_pid != 0 && cur_pid != pid)
			continue;

		foreach(quint32 tid, Info.Threads) {
			if (m_pTraceTid->findData(tid) == -1)
				m_pTraceTid->addItem(tr("%1").arg(tid), tid);
		}
	}
}

void CTraceView::OnSetPidFilter()
{
	m_pSortProxy->m_FilterPid = m_pTracePid->currentData().toUInt();
	m_pSortProxy->m_FilterTid = 0;

	QTimer::singleShot(100, this, [this]() {

		m_pTraceTid->clear();
		m_pTraceTid->addItem(tr("[All]"), 0);

		UpdateFilters();
	});

	m_pSortProxy->setFilterKeyColumn(m_pSortProxy->filterKeyColumn());
	m_pTreeList->expandAll();
}

void CTraceView::OnSetTidFilter()
{
	m_pSortProxy->m_FilterTid = m_pTraceTid->currentData().toUInt();

	m_pSortProxy->setFilterKeyColumn(m_pSortProxy->filterKeyColumn());
	m_pTreeList->expandAll();
}