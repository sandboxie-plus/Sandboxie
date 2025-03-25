#include "stdafx.h"
#include "TraceView.h"
#include "..\SandMan.h"
#include "..\AddonManager.h"
#include "../QSbieAPI/SbieAPI.h"
#include "..\Models\TraceModel.h"
#include "..\..\MiscHelpers\Common\Common.h"
#include "..\..\MiscHelpers\Common\CheckList.h"
#include "..\..\MiscHelpers\Common\CheckableComboBox.h"
#include "SbieView.h"
#include <QtConcurrent>

//class CTraceFilterProxyModel : public CSortFilterProxyModel
//{
//public:
//	CTraceFilterProxyModel(QObject* parent = 0) : CSortFilterProxyModel(parent)
//	{
//		m_FilterPid = 0;
//		m_FilterTid = 0;
//	}
//
//	bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
//	{
//		CTraceModel* pTraceModel = (CTraceModel*)sourceModel();
//
//		QModelIndex index = pTraceModel->index(source_row, 0, source_parent);
//		//CTraceEntryPtr pEntry = pTraceModel->GetEntry(index);
//		//if (pEntry.data() == NULL) 
//		{
//			QVariant Id = pTraceModel->GetItemID(index);
//			StrPair typeId = Split2(Id.toString(), "_");
//
//			if (m_FilterPid != 0 && typeId.first == "pid") {
//				if (m_FilterPid != typeId.second.toUInt())
//					return false;
//			}
//
//			if (m_FilterTid != 0 && typeId.first == "tid") {
//				if (m_FilterTid != typeId.second.toUInt())
//					return false;
//			}
//		}
//
//		return CSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
//	}
//
//	quint32		m_FilterPid;
//	quint32		m_FilterTid;
//};

////////////////////////////////////////////////////////////////////////////////////////
// CTraceTree

CTraceTree::CTraceTree(QWidget* parent) 
	: CPanelView(parent) 
{
	m_pMainLayout = new QVBoxLayout();
	m_pMainLayout->setContentsMargins(0,0,0,0);
	this->setLayout(m_pMainLayout);

	m_pSplitter = new QSplitter();
	m_pSplitter->setOrientation(Qt::Horizontal);
	m_pMainLayout->addWidget(m_pSplitter);

	m_pTreeList = new QTreeViewEx();
	m_pTreeList->setColumnFixed(0, true);
	m_pTreeList->setColumnFixed(1, true);
	m_pTreeList->setColumnFixed(2, true);
	m_pTreeList->setColumnFixed(3, true);
	m_pTreeList->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pTreeList, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(OnMenu(const QPoint &)));
	//m_pSplitter->addWidget(m_pTreeList);
	m_pTreeList->setMinimumHeight(50);
	AddPanelItemsToMenu();


	m_bHighLight = false;
	//m_FilterCol = -1;

	m_pTreeList->setAlternatingRowColors(theConf->GetBool("Options/AltRowColors", false));

	m_pTreeList->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_pTreeList->setUniformRowHeights(true); // critical for good performance with huge data sets

	m_pTraceModel = new CTraceModel(this);
	//connect(m_pTraceModel, SIGNAL(NewBranche()), this, SLOT(UpdateFilters()));

	//m_pSortProxy = new CTraceFilterProxyModel(this);
	//m_pSortProxy->setSortRole(Qt::EditRole);
	//m_pSortProxy->setSourceModel(m_pTraceModel);
	//m_pSortProxy->setDynamicSortFilter(true);

	//m_pTreeList->setModel(m_pSortProxy);
	//m_pSortProxy->setView(m_pTreeList);

	m_pTreeList->setModel(m_pTraceModel);

	QStyle* pStyle = QStyleFactory::create("windows");
	m_pTreeList->setStyle(pStyle);
	m_pTreeList->setItemDelegate(new CTreeItemDelegate());
	m_pTreeList->setExpandsOnDoubleClick(false);
	//m_pTreeList->setSortingEnabled(true);

	m_pTreeList->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pTreeList, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnMenu(const QPoint&)));

	m_pTreeList->setColumnReset(1);
	//connect(m_pTreeList, SIGNAL(ResetColumns()), m_pTreeList, SLOT(OnResetColumns()));
	//connect(m_pBoxTree, SIGNAL(ColumnChanged(int, bool)), this, SLOT(OnColumnsChanged()));

	//m_pMainLayout->addWidget(CFinder::AddFinder(m_pTreeList, m_pSortProxy));
	/*CFinder* pFinder = new CFinder(this, this, CFinder::eHighLightDefault);
	pFinder->SetTree(m_pTreeList);
	m_pMainLayout->addWidget(pFinder);*/

	CFinder* pFinder;
	//m_pMainLayout->addWidget(CFinder::AddFinder(m_pTreeList, this, CFinder::eHighLightDefault, &pFinder));
	m_pSplitter->addWidget(CFinder::AddFinder(m_pTreeList, this, CFinder::eHighLightDefault, &pFinder));
	pFinder->SetModel(m_pTraceModel);
	pFinder->SetAlwaysRaw();
	//QObject::connect(pFinder, SIGNAL(SelectNext()), this, SLOT(SelectNext()));


	m_pStackView = new CStackView();
	m_pSplitter->addWidget(m_pStackView);

	connect(m_pTreeList->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), SLOT(ItemSelection(QItemSelection, QItemSelection)));


	QByteArray Columns = theConf->GetBlob("MainWindow/TraceLog_Columns");
	if (!Columns.isEmpty())
		((QTreeViewEx*)GetView())->restoreState(Columns);

	QByteArray Split = theConf->GetBlob("MainWindow/TraceSplitter");
	if(!Split.isEmpty())
		m_pSplitter->restoreState(Split);
	//else { // by default collapse the details panel
	//	auto Sizes = m_pSplitter->sizes();
	//	Sizes[1] = 0;
	//	m_pSplitter->setSizes(Sizes);
	//}
}

CTraceTree::~CTraceTree() 
{
	theConf->SetBlob("MainWindow/TraceLog_Columns", GetView()->header()->saveState());
	theConf->SetBlob("MainWindow/TraceSplitter", m_pSplitter->saveState());
}

void CTraceTree::SetFilter(const QRegularExpression& Exp, int iOptions, int Column) 
{
	QString ExpStr = Exp.pattern();
	bool bReset = m_bHighLight != ((iOptions & CFinder::eHighLight) != 0) || (!m_bHighLight && m_FilterExp != ExpStr);

	//m_FilterExp = RegExp;
	m_FilterExp = ExpStr;
	m_bHighLight = (iOptions & CFinder::eHighLight) != 0;
	//m_FilterCol = Col;

	if(bReset)
		emit FilterChanged();
}

void CTraceTree::ItemSelection(const QItemSelection& selected, const QItemSelection& deselected)
{
	QItemSelectionModel* selectionModel = m_pTreeList->selectionModel();
	QItemSelection selection = selectionModel->selection();

	if (selection.indexes().isEmpty()) 
		return;

	CTraceEntryPtr pEntry = m_pTraceModel->GetEntry(selection.indexes().first());
	if (pEntry.data() == NULL)
		return;
	CBoxedProcessPtr pProcess = theAPI->GetProcessById(pEntry->GetProcessId());
	if(!pProcess.isNull())
		m_pStackView->ShowStack(pEntry->GetStack(), pProcess);
}



////////////////////////////////////////////////////////////////////////////////////////
// CMonitorList

CMonitorList::CMonitorList(QWidget* parent) 
	: CPanelWidgetTmpl<QTreeViewEx>(NULL, parent) 
{
	m_pTreeList->setAlternatingRowColors(theConf->GetBool("Options/AltRowColors", false));

	m_pTreeList->setSelectionMode(QAbstractItemView::ExtendedSelection);

	m_pMonitorModel = new CMonitorModel(this);
	//connect(m_pMonitorModel, SIGNAL(NewBranche()), this, SLOT(UpdateFilters()));

	m_pSortProxy = new CSortFilterProxyModel(this);
	m_pSortProxy->setSortRole(Qt::EditRole);
	m_pSortProxy->setSourceModel(m_pMonitorModel);
	m_pSortProxy->setDynamicSortFilter(true);

	m_pTreeList->setModel(m_pSortProxy);

	QStyle* pStyle = QStyleFactory::create("windows");
	m_pTreeList->setStyle(pStyle);
	m_pTreeList->setItemDelegate(new CTreeItemDelegate());
	
	m_pTreeList->setExpandsOnDoubleClick(false);
	m_pTreeList->setSortingEnabled(true);

	m_pTreeList->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pTreeList, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnMenu(const QPoint&)));

	m_pTreeList->setColumnFixed(0, true);
	m_pTreeList->setColumnFixed(1, true);
	m_pTreeList->setColumnFixed(2, true);
	m_pTreeList->setColumnReset(1);
	//connect(m_pTreeList, SIGNAL(ResetColumns()), m_pTreeList, SLOT(OnResetColumns()));
	//connect(m_pBoxTree, SIGNAL(ColumnChanged(int, bool)), this, SLOT(OnColumnsChanged()));

	m_pMainLayout->addWidget(CFinder::AddFinder(m_pTreeList, m_pSortProxy));


	QByteArray Columns = theConf->GetBlob("MainWindow/Monitor_Columns");
	if (!Columns.isEmpty())
		((QTreeViewEx*)GetView())->restoreState(Columns);
}

CMonitorList::~CMonitorList() 
{
	theConf->SetBlob("MainWindow/Monitor_Columns", GetView()->header()->saveState());
}


////////////////////////////////////////////////////////////////////////////////////////
// CTraceView


CTraceView::CTraceView(bool bStandAlone, QWidget* parent) : QWidget(parent)
{
	m_FullRefresh = true;

	m_LastID = 0;
	m_LastCount = 0;
	m_bUpdatePending = false;

	m_FilterTid = 0;
	m_FilterStatus = 0;

	m_pMainLayout = new QVBoxLayout();
	m_pMainLayout->setContentsMargins(0,0,0,0);
	this->setLayout(m_pMainLayout);

	m_pTraceToolBar = new QToolBar();

	m_pMonitorMode = m_pTraceToolBar->addAction(CSandMan::GetIcon("Monitor"), tr("Monitor mode"), this, SLOT(OnSetMode()));
	m_pMonitorMode->setCheckable(true);
	m_pMonitorMode->setChecked(theConf->GetBool("Options/UseMonitorMode", true));

	m_pTraceTree = m_pTraceToolBar->addAction(CSandMan::GetIcon("Tree"), tr("Show as task tree"), this, SLOT(OnSetTree()));
	m_pTraceTree->setCheckable(true);
	m_pTraceTree->setChecked(theConf->GetBool("Options/UseLogTree"));

	m_pObjectTree = m_pTraceToolBar->addAction(CSandMan::GetIcon("Objects"), tr("Show NT Object Tree"), this, SLOT(OnObjTree()));
	m_pObjectTree->setCheckable(true);
	m_pObjectTree->setChecked(theConf->GetBool("Options/UseObjectTree"));

	m_pTraceToolBar->addSeparator();
	m_pTraceToolBar->layout()->setSpacing(3);

	m_pTraceToolBar->addWidget(new QLabel(tr("PID:")));
	m_pTracePid = new CCheckableComboBox();
	m_pTracePid->m_SelectItems = tr("[All]");
	m_pTracePid->setMinimumWidth(300);
	connect(m_pTracePid->model(), SIGNAL(itemChanged(QStandardItem*)), this, SLOT(OnSetPidFilter(QStandardItem*)));
	m_pTraceToolBar->addWidget(m_pTracePid);

	m_pTraceToolBar->addWidget(new QLabel(tr("TID:")));
	m_pTraceTid = new QComboBox();
	m_pTraceTid->addItem(tr("[All]"), 0);
	m_pTraceTid->setMinimumWidth(75);
	m_pTraceTid->setEditable(true);
	connect(m_pTraceTid, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSetTidFilter()));
	m_pTraceToolBar->addWidget(m_pTraceTid);

	m_pTraceToolBar->addWidget(new QLabel(tr("Type:")));
	m_pTraceType = new QCheckList();
	//m_pTraceType->addItem(tr("[All]"), 0);
	m_pTraceType->setAllCheckedText(tr("[All]"));
	m_pTraceType->setNoneCheckedText(tr("[All]"));
	foreach(quint32 type, CTraceEntry::AllTypes()) 
		m_pTraceType->addCheckItem(CTraceEntry::GetTypeStr(type), type, Qt::Unchecked);
	m_pTraceType->setMinimumWidth(100);
	connect(m_pTraceType, SIGNAL(globalCheckStateChanged(int)), this, SLOT(OnSetFilter()));
	m_pTraceToolBar->addWidget(m_pTraceType);

	m_pTraceToolBar->addWidget(new QLabel(tr("Status:")));
	m_pTraceStatus = new QComboBox();
	m_pTraceStatus->addItem(tr("[All]"), 0);
	m_pTraceStatus->addItem(tr("Open"), 1);
	m_pTraceStatus->addItem(tr("Closed"), 2);
	m_pTraceStatus->addItem(tr("Trace"), 3);
	m_pTraceStatus->addItem(tr("Other"), 4);
	m_pTraceStatus->setMinimumWidth(75);
	connect(m_pTraceStatus, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSetFilter()));
	m_pTraceToolBar->addWidget(m_pTraceStatus);

	if (bStandAlone)
		m_pAllBoxes = NULL;
	else {
		m_pAllBoxes = m_pTraceToolBar->addAction(CSandMan::GetIcon("All"), tr("Show All Boxes"), this, SLOT(OnSetFilter()));
		m_pAllBoxes->setCheckable(true);
	}

	m_pShowStack = m_pTraceToolBar->addAction(CSandMan::GetIcon("Stack"), tr("Show Stack Trace"), this, SLOT(OnShowStack()));
	m_pShowStack->setCheckable(true);

	m_pTraceToolBar->addSeparator();

	m_pSaveToFile = m_pTraceToolBar->addAction(CSandMan::GetIcon("Save"), tr("Save to file"), this, SLOT(SaveToFile()));

	m_pMainLayout->setSpacing(0);

	m_pMainLayout->addWidget(m_pTraceToolBar);


	m_pView = new QWidget(this);
	m_pLayout = new QStackedLayout(m_pView);
	
	m_pTrace = new CTraceTree(this);
	m_pTrace->m_pTraceModel->SetTree(m_pTraceTree->isChecked());

	m_pTrace->m_pAutoScroll = new QAction(tr("Auto Scroll"));
	m_pTrace->m_pAutoScroll->setCheckable(true);
	m_pTrace->m_pAutoScroll->setChecked(theConf->GetBool("Options/TraceAutoScroll"));
	m_pTrace->GetMenu()->insertAction(m_pTrace->GetMenu()->actions()[0], m_pTrace->m_pAutoScroll);

	if (bStandAlone) {
		QAction* pAction = new QAction(tr("Cleanup Trace Log"));
		connect(pAction, SIGNAL(triggered()), this, SLOT(Clear()));
		m_pTrace->GetMenu()->insertAction(m_pTrace->GetMenu()->actions()[1], pAction);
	}

	m_pLayout->addWidget(m_pTrace);

	QObject::connect(m_pTrace, SIGNAL(FilterChanged()), this, SLOT(OnFilterChanged()));

	m_pMonitor = new CMonitorList(this);
	m_pMonitor->m_pMonitorModel->SetObjTree(m_pObjectTree->isChecked());
	m_pLayout->addWidget(m_pMonitor);

	m_pView->setLayout(m_pLayout);
	m_pMainLayout->addWidget(m_pView);

	OnSetMode();
	
	m_uTimerID = startTimer(1000);
}

CTraceView::~CTraceView()
{
	theConf->SetValue("Options/TraceAutoScroll", m_pTrace->m_pAutoScroll->isChecked());

	killTimer(m_uTimerID);
}

void CTraceView::timerEvent(QTimerEvent* pEvent)
{
	if (pEvent->timerId() != m_uTimerID)
		return;

	Refresh();
}

void CTraceView::SetEnabled(bool bSet)
{
	setEnabled(bSet);
	m_pShowStack->setChecked(theAPI->GetGlobalSettings()->GetBool("MonitorStackTrace", false));
	m_pTrace->m_pStackView->setVisible(m_pShowStack->isChecked());
}

void CTraceView::OnShowStack()
{
	if (m_pShowStack->isChecked() && !theGUI->GetAddonManager()->GetAddon("DbgHelp", CAddonManager::eInstalled).isNull())
        theGUI->GetAddonManager()->TryInstallAddon("DbgHelp", this, tr("To use the stack traces feature the DbgHelp.dll and SymSrv.dll are required, do you want to download and install them?"));
	theAPI->GetGlobalSettings()->SetBool("MonitorStackTrace", m_pShowStack->isChecked());
	m_pTrace->m_pStackView->setVisible(m_pShowStack->isChecked());
}

void CTraceView::Refresh()
{
	QList<CSandBoxPtr>Boxes;
	if(m_pAllBoxes && !m_pAllBoxes->isChecked())
		Boxes = theGUI->GetBoxView()->GetSelectedBoxes();
	
	if (m_pCurrentBox != (Boxes.count() == 1 ? Boxes.first().data() : NULL)) {
		m_pCurrentBox = Boxes.count() == 1 ? Boxes.first().data() : NULL;
		m_FullRefresh = true;
	}

	bool bMonitorMode = m_pMonitorMode->isChecked();

	if (m_FullRefresh) 
	{
		m_LastID = 0;
		m_LastCount = 0;
		m_PidMap.clear();

		quint64 start = GetCurCycle();
		m_pTrace->m_pTraceModel->Clear();
		qDebug() << "Clear took" << (GetCurCycle() - start) / 1000000.0 << "s";

		m_pMonitor->m_pMonitorModel->Clear();
		m_FullRefresh = false;
	}

	const QVector<CTraceEntryPtr> &ResourceLog = theAPI->GetTrace();

	bool bUpdateFilters = false;

	int i = 0;
	if (ResourceLog.count() >= m_LastCount && m_LastCount > 0)
	{
		i = m_LastCount - 1;
		if (m_LastID == ResourceLog.at(i)->GetUID())
			i++;
		else
			i = 0;
	}

	if (i == 0) {
		m_PidMap.clear();
		m_TraceList.clear();
		m_MonitorMap.clear();
	}

	if (m_LastCount == ResourceLog.count())
		return;

	//bool bHasFilter = !m_pTrace->m_FilterExp.pattern().isEmpty();
	bool bHasFilter = !m_pTrace->m_FilterExp.isEmpty();

	quint64 start = GetCurCycle();
	for (; i < ResourceLog.count(); i++)
	{
		const CTraceEntryPtr& pEntry = ResourceLog.at(i);

		SProgInfo& Info = m_PidMap[pEntry->GetProcessId()];
		if (Info.Name.isEmpty()) {
			Info.Name = pEntry->GetProcessName();
			bUpdateFilters = true;
		}
		if (!Info.Threads.contains(pEntry->GetThreadId())) {
			Info.Threads.insert(pEntry->GetThreadId());
			bUpdateFilters = true;
		}

		if (m_pCurrentBox != NULL && m_pCurrentBox != pEntry->GetBoxPtr())
			continue;

		quint32 pid = pEntry->GetProcessId();
		if (!((m_ShowPids.isEmpty() || m_ShowPids.contains(pid)) && !m_HidePids.contains(pid)))
			continue;

		if (m_FilterTid != 0 && m_FilterTid != pEntry->GetThreadId())
			continue;

		if (!m_FilterTypes.isEmpty() && !m_FilterTypes.contains(pEntry->GetType()))
			continue;

		if (bMonitorMode)
		{
			QString Name = pEntry->GetName();
			if (Name.isEmpty())
				Name = pEntry->GetMessage();
			CMonitorEntryPtr& pItem = m_MonitorMap[Name.toLower()];
			if (pItem.data() == NULL) {
				//if (Name.left(9).compare("\\REGISTRY", Qt::CaseInsensitive) == 0) {
				//	int pos = Name.indexOf("\\", 10);
				//	Name = Name.left(pos).toUpper() + Name.mid(pos);
				//}
				pItem = CMonitorEntryPtr(new CMonitorEntry(Name, pEntry->GetType()));
			}

			pItem->Merge(pEntry);
		}
		else
		{
			if (bHasFilter && !m_pTrace->m_bHighLight) {
				if (!pEntry->GetName().contains(m_pTrace->m_FilterExp, Qt::CaseInsensitive)
					&& !pEntry->GetMessage().contains(m_pTrace->m_FilterExp, Qt::CaseInsensitive)
					//&& !pEntry->GetTypeStr().contains(m_pTrace->m_FilterExp, Qt::CaseInsensitive) // don't filter on non static strings !!!
					//&& !pEntry->GetStautsStr().contains(m_pTrace->m_FilterExp, Qt::CaseInsensitive) // don't filter on non static strings !!!
					&& !pEntry->GetProcessName().contains(m_pTrace->m_FilterExp, Qt::CaseInsensitive))
						continue;
			}
	
			if (m_FilterStatus != 0) {
				if (pEntry->IsOpen()) {
					if (m_FilterStatus != 1) continue;
				} else if (pEntry->IsClosed()) {
					if (m_FilterStatus != 2) continue;
				} else if (pEntry->IsTrace()) {
					if (m_FilterStatus != 3) continue;
				} else {
					if (m_FilterStatus != 4) continue;
				}
			}

			m_TraceList.append(pEntry);
		}
	}
	qDebug() << "Filtering took" << (GetCurCycle() - start) / 1000000.0 << "s";

	m_LastCount = ResourceLog.count();
	if(m_LastCount)
		m_LastID = ResourceLog.last()->GetUID();

	if (bUpdateFilters && !m_bUpdatePending)
	{
		m_bUpdatePending = true;
		QTimer::singleShot(500, this, SLOT(UpdateFilters()));
	}


	if (bMonitorMode)
	{
		QList<QModelIndex> NewBranches = m_pMonitor->m_pMonitorModel->Sync(m_MonitorMap, this);

		if (m_pMonitor->m_pMonitorModel->IsObjTree())
		{
			QTimer::singleShot(10, this, [this, NewBranches]() {
				CSortFilterProxyModel* pSortProxy = m_pMonitor->m_pSortProxy;
				foreach(const QModelIndex& Index, NewBranches) {
					m_pMonitor->GetTree()->expand(pSortProxy->mapFromSource(Index));
				}
			});
		}
	}
	else
	{
		if (m_pTrace->m_bHighLight)
			m_pTrace->m_pTraceModel->SetHighLight(m_pTrace->m_FilterExp);
		else
			m_pTrace->m_pTraceModel->SetHighLight(QString());

		quint64 start = GetCurCycle();
		QList<QModelIndex> NewBranches = m_pTrace->m_pTraceModel->Sync(m_TraceList);
		qDebug() << "Sync took" << (GetCurCycle() - start) / 1000000.0 << "s";

		if (m_pTrace->m_pTraceModel->IsTree())
		{
			QTimer::singleShot(10, this, [this, NewBranches]() {
				quint64 start = GetCurCycle();
				foreach(const QModelIndex& Index, NewBranches)
					m_pTrace->GetTree()->expand(Index);
				qDebug() << "Expand took" << (GetCurCycle() - start) / 1000000.0 << "s";
			});
		}

		if(m_pTrace->m_pAutoScroll->isChecked())
			m_pTrace->m_pTreeList->scrollToBottom();
	}
}

void CTraceView::Clear()
{
	m_pTracePid->clear();
	m_ShowPids.clear();
	m_HidePids.clear();

	m_pTraceTid->clear();
	m_pTraceTid->addItem(tr("[All]"), 0);

	theAPI->ClearTrace();
	m_pTrace->m_pTraceModel->Clear(true);
	m_pMonitor->m_pMonitorModel->Clear();
}

void CTraceView::AddAction(QAction* pAction)
{
	m_pTrace->GetMenu()->insertAction(m_pTrace->GetMenu()->actions()[0], pAction);
	m_pTrace->GetMenu()->insertSeparator(m_pTrace->GetMenu()->actions()[0]);

	m_pMonitor->GetMenu()->insertAction(m_pMonitor->GetMenu()->actions()[0], pAction);
	m_pMonitor->GetMenu()->insertSeparator(m_pMonitor->GetMenu()->actions()[0]);
}

void CTraceView::OnSetTree()
{
	m_pTrace->m_pTraceModel->SetTree(m_pTraceTree->isChecked());

	//m_pTrace->m_pTraceModel->Clear();

	m_FullRefresh = true;
	Refresh();
	//m_pTrace->GetTree()->expandAll();

	theConf->SetValue("Options/UseLogTree", m_pTraceTree->isChecked());
}

void CTraceView::OnSetMode()
{
	if (m_pMonitorMode->isChecked())
		m_pLayout->setCurrentIndex(1); // monitor
	else
		m_pLayout->setCurrentIndex(0); // trace

	m_pTraceTree->setEnabled(!m_pMonitorMode->isChecked());
	m_pObjectTree->setEnabled(m_pMonitorMode->isChecked());
	m_pTraceStatus->setEnabled(!m_pMonitorMode->isChecked());
	m_pShowStack->setEnabled(!m_pMonitorMode->isChecked());

	m_FullRefresh = true;
	Refresh();

	theConf->SetValue("Options/UseMonitorMode", m_pMonitorMode->isChecked());
}

void CTraceView::OnObjTree()
{
	m_pMonitor->m_pMonitorModel->SetObjTree(m_pObjectTree->isChecked());

	//m_pMonitor->m_pMonitorModel->Clear();

	m_FullRefresh = true;
	Refresh();
	//m_pTrace->GetTree()->expandAll();

	theConf->SetValue("Options/UseObjectTree", m_pObjectTree->isChecked());
}

void CTraceView::UpdateFilters()
{
	m_bUpdatePending = false;

	QMap<quint32, SProgInfo> pids = m_PidMap;
	foreach(quint32 pid, pids.keys()) {
		SProgInfo& Info = pids[pid];

		if (m_pTracePid->findData(pid) == -1) {
			m_pTracePid->addItem(tr("%1 (%2)").arg(Info.Name).arg(pid), pid);
			QStandardItemModel *model = qobject_cast<QStandardItemModel *>(m_pTracePid->model());
			QStandardItem *item = model->item(m_pTracePid->count()-1);
			item->setCheckable(true);
			item->setCheckState(Qt::Unchecked); // Set default state
			item->setFlags(item->flags() | Qt::ItemIsUserTristate); // Enable tri-state
		}

		if ((m_ShowPids.isEmpty() || m_ShowPids.contains(pid)) && !m_HidePids.contains(pid))
			continue;

		foreach(quint32 tid, Info.Threads) {
			if (m_pTraceTid->findData(tid) == -1)
				m_pTraceTid->addItem(tr("%1").arg(tid), tid);
		}
	}
}

void CTraceView::OnFilterChanged()
{
	m_FullRefresh = true;
}

void CTraceView::OnSetPidFilter(QStandardItem* item)
{
	switch (item->checkState()) {
	case Qt::Checked:
		m_ShowPids.insert(item->data(Qt::UserRole).toUInt());
		m_HidePids.remove(item->data(Qt::UserRole).toUInt());
		break;
	case Qt::PartiallyChecked:
		m_ShowPids.remove(item->data(Qt::UserRole).toUInt());
		m_HidePids.insert(item->data(Qt::UserRole).toUInt());
		break;
	case Qt::Unchecked:
		m_ShowPids.remove(item->data(Qt::UserRole).toUInt());
		m_HidePids.remove(item->data(Qt::UserRole).toUInt());
		break;
	}
	
	m_FilterTid = 0;
	//m_pSortProxy->m_FilterPid = m_pTracePid->currentData().toUInt();
	//m_pSortProxy->m_FilterTid = 0;

	QTimer::singleShot(100, this, [this]() {

		m_pTraceTid->clear();
		m_pTraceTid->addItem(tr("[All]"), 0);

		UpdateFilters();
	});

	//m_pSortProxy->setFilterKeyColumn(m_pSortProxy->filterKeyColumn());
	m_FullRefresh = true;
	//if(!m_pMonitorMode->isChecked())
	//	m_pTrace->GetTree()->expandAll();
}

void CTraceView::OnSetTidFilter()
{
	//m_FilterTid = m_pTraceTid->currentData().toUInt();
	m_FilterTid = m_pTraceTid->currentText().toUInt();
	//m_pSortProxy->m_FilterTid = m_pTraceTid->currentData().toUInt();

	//m_pSortProxy->setFilterKeyColumn(m_pSortProxy->filterKeyColumn());
	m_FullRefresh = true;
	//if(!m_pMonitorMode->isChecked())
	//	m_pTrace->GetTree()->expandAll();
}


void CTraceView::OnSetFilter()
{
	m_FilterTypes.clear();
	for (int i = 0; i < m_pTraceType->count(); i++) {
		if (m_pTraceType->itemData(i, Qt::CheckStateRole).toInt() == Qt::Checked) {
			m_FilterTypes.append(m_pTraceType->itemData(i, Qt::UserRole + 1).toUInt());
		}
	}

	m_FilterStatus = m_pTraceStatus->currentData().toUInt();

	m_FullRefresh = true;
	//if(!m_pMonitorMode->isChecked())
	//	m_pTrace->GetTree()->expandAll();
}


void CTraceView::SaveToFile()
{
	QString Path = QFileDialog::getSaveFileName(this, tr("Save trace log to file"), "", QString("Log files (*.log)")).replace("/", "\\");
	if (Path.isEmpty())
		return;

	QFile File(Path);
	if (!File.open(QFile::WriteOnly)) {
		QMessageBox::critical(this, "Sandboxie-Plus", tr("Failed to open log file for writing"));
		return;
	}

	if (m_pMonitorMode->isChecked())
	{
		QList<QStringList> Rows = m_pMonitor->DumpPanel();
		foreach(const QStringList& Row, Rows)
			File.write(Row.join("\t").toLatin1() + "\n");
	}
	else
	{
		SaveToFile(&File);
	}

	File.close();
}

void CTraceView::SaveToFileAsync(const CSbieProgressPtr& pProgress, QVector<CTraceEntryPtr> ResourceLog, QIODevice* pFile)
{
	pProgress->ShowMessage(tr("Saving TraceLog..."));

	QByteArray Unknown = "Unknown";

	quint64 LastTimeStamp = 0;
	QByteArray LastTimeStampStr;
	for (int i = 0; i < ResourceLog.count() && !pProgress->IsCanceled(); i++)
	{
		if (i % 10000 == 0)
			pProgress->SetProgress(100 * i / ResourceLog.count());

		const CTraceEntryPtr& pEntry = ResourceLog.at(i);

		if (LastTimeStamp != pEntry->GetTimeStamp()) {
			LastTimeStamp = pEntry->GetTimeStamp();
			LastTimeStampStr = QDateTime::fromMSecsSinceEpoch(pEntry->GetTimeStamp()).toString("dd.MM.yyyy hh:mm:ss.zzz").toUtf8();
		}

		pFile->write(LastTimeStampStr);
		pFile->write("\t");
		QString Name = pEntry->GetProcessName();
		pFile->write(Name.isEmpty() ? Unknown : Name.toUtf8());
		pFile->write("\t");
		pFile->write(QByteArray::number(pEntry->GetProcessId()));
		pFile->write("\t");
		pFile->write(QByteArray::number(pEntry->GetThreadId()));
		pFile->write("\t");
		pFile->write(pEntry->GetTypeStr().toUtf8());
		pFile->write("\t");
		pFile->write(pEntry->GetStautsStr().toUtf8());
		pFile->write("\t");
		pFile->write(pEntry->GetName().toUtf8());
		pFile->write("\t");
		pFile->write(pEntry->GetMessage().toUtf8());
		pFile->write("\n");
	}

	pProgress->Finish(SB_OK);
}

bool CTraceView::SaveToFile(QIODevice* pFile)
{
	pFile->write("Timestamp\tProcess\tPID\tTID\tType\tStatus\tName\tMessage\n"); // don't translate log
	QVector<CTraceEntryPtr> ResourceLog = theAPI->GetTrace();
	CSbieProgressPtr pProgress = CSbieProgressPtr(new CSbieProgress());
	QtConcurrent::run(CTraceView::SaveToFileAsync, pProgress, ResourceLog, pFile);
	theGUI->AddAsyncOp(pProgress, true);
	return !pProgress->IsCanceled();
}


////////////////////////////////////////////////////////////////////////////////////////
// CTraceWindow

CTraceWindow::CTraceWindow(QWidget *parent)
	: QDialog(parent)
{
	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::CustomizeWindowHint;
	//flags &= ~Qt::WindowContextHelpButtonHint;
	//flags &= ~Qt::WindowSystemMenuHint;
	//flags &= ~Qt::WindowMinMaxButtonsHint;
	//flags |= Qt::WindowMinimizeButtonHint;
	//flags &= ~Qt::WindowCloseButtonHint;
	flags &= ~Qt::WindowContextHelpButtonHint;
	//flags &= ~Qt::WindowSystemMenuHint;
	setWindowFlags(flags);

	this->setWindowTitle(tr("Sandboxie-Plus - Trace Monitor"));

	this->setWindowFlag(Qt::WindowStaysOnTopHint, theGUI->IsAlwaysOnTop());

	QGridLayout* pLayout = new QGridLayout();
	pLayout->setContentsMargins(3,3,3,3);
	pLayout->addWidget(new CTraceView(true, this), 0, 0);
	this->setLayout(pLayout);

	restoreGeometry(theConf->GetBlob("TraceWindow/Window_Geometry"));
}

CTraceWindow::~CTraceWindow()
{
	theConf->SetBlob("TraceWindow/Window_Geometry", saveGeometry());

	if(!theAPI) theAPI->EnableMonitor(false);
}

void CTraceWindow::closeEvent(QCloseEvent *e)
{
	emit Closed();
	this->deleteLater();
}

