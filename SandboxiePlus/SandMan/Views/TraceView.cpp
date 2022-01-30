#include "stdafx.h"
#include "TraceView.h"
#include "..\SandMan.h"
#include "../QSbieAPI/SbieAPI.h"
#include "..\Models\TraceModel.h"
#include "..\..\MiscHelpers\Common\Common.h"
#include "..\..\MiscHelpers\Common\CheckList.h"
#include "SbieView.h"

//class CTraceFilterProxyModel : public CSortFilterProxyModel
//{
//public:
//	CTraceFilterProxyModel(QObject* parrent = 0) : CSortFilterProxyModel(false, parrent)
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

CTraceView::CTraceView(QWidget* parent) : CPanelWidget<QTreeViewEx>(parent)
{
	//m_pTreeList->setItemDelegate(theGUI->GetItemDelegate());

	m_FullRefresh = true;

	m_bHighLight = false;
	//m_FilterCol = -1;
	m_FilterPid = 0;
	m_FilterTid = 0;
	m_FilterStatus = 0;

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

	m_pAllBoxes = m_pTraceToolBar->addAction(CSandMan::GetIcon("All"), tr("Show All Boxes"), this, SLOT(OnSetFilter()));
	m_pAllBoxes->setCheckable(true);
	m_pAllBoxes->setChecked(theConf->GetBool("Options/UseLogTree"));

	m_pTraceToolBar->addSeparator();

	m_pSaveToFile = m_pTraceToolBar->addAction(CSandMan::GetIcon("Save"), tr("Save to file"), this, SLOT(SaveToFile()));

	m_pMainLayout->setSpacing(0);

	m_pMainLayout->insertWidget(0, m_pTraceToolBar);

	m_pTraceModel = new CTraceModel();
	m_pTraceModel->SetTree(m_pTraceTree->isChecked());
	connect(m_pTraceModel, SIGNAL(NewBranche()), this, SLOT(UpdateFilters()));

	//m_pSortProxy = new CTraceFilterProxyModel(this);
	//m_pSortProxy->setSortRole(Qt::EditRole);
	//m_pSortProxy->setSourceModel(m_pTraceModel);
	//m_pSortProxy->setDynamicSortFilter(true);

	//m_pTreeList->setModel(m_pSortProxy);
	//m_pSortProxy->setView(m_pTreeList);

	m_pTreeList->setModel(m_pTraceModel);


	m_pTreeList->setSelectionMode(QAbstractItemView::ExtendedSelection);
#ifdef WIN32
	QStyle* pStyle = QStyleFactory::create("windows");
	m_pTreeList->setStyle(pStyle);
#endif
	m_pTreeList->setExpandsOnDoubleClick(false);
	//m_pTreeList->setSortingEnabled(true);

	m_pTreeList->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pTreeList, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnMenu(const QPoint&)));

	m_pTreeList->setColumnReset(1);
	//connect(m_pTreeList, SIGNAL(ResetColumns()), m_pTreeList, SLOT(OnResetColumns()));
	//connect(m_pBoxTree, SIGNAL(ColumnChanged(int, bool)), this, SLOT(OnColumnsChanged()));

	//m_pMainLayout->addWidget(CFinder::AddFinder(m_pTreeList, m_pSortProxy));
	m_pMainLayout->addWidget(CFinder::AddFinder(m_pTreeList, this));


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

int CTraceView__Filter(const CTraceEntryPtr& pEntry, void* params)
{
	CTraceView* This = (CTraceView*)params;

	int Ret = 1;

	if (This->m_pCurrentBox != NULL && This->m_pCurrentBox != pEntry->GetBoxPtr())
		return 0;

	if (This->m_FilterExp.isValid()) {
		if (!pEntry->GetMessage().contains(This->m_FilterExp)
			&& !pEntry->GetTypeStr().contains(This->m_FilterExp)
			//&& !pEntry->GetStautsStr().contains(This->m_FilterExp)
			&& !pEntry->GetProcessName().contains(This->m_FilterExp))
			Ret = This->m_bHighLight ? 1 : 0;
		else
			Ret = This->m_bHighLight ? 2 : 1;
	}

	if (This->m_FilterPid != 0 && This->m_FilterPid != pEntry->GetProcessId())
		return 0;

	if (This->m_FilterTid != 0 && This->m_FilterTid != pEntry->GetThreadId())
		return 0;

	if (!This->m_FilterTypes.isEmpty() && !This->m_FilterTypes.contains(pEntry->GetType()))
		return 0;
	
	if (This->m_FilterStatus != 0) {
		if (pEntry->IsOpen()) {
			if(This->m_FilterStatus == 1) return Ret;
		} else if (pEntry->IsClosed()) {
			if (This->m_FilterStatus == 2) return Ret;
		} else if (pEntry->IsTrace()) {
			if(This->m_FilterStatus == 3) return Ret;
		} else
			if(This->m_FilterStatus == 4) return Ret;
		return 0;
	}

	return Ret;
}

void CTraceView::Refresh()
{
	QList<CSandBoxPtr>Boxes;
	if(!m_pAllBoxes->isChecked())
		Boxes = theGUI->GetBoxView()->GetSelectedBoxes();
	
	if (m_pCurrentBox != (Boxes.count() == 1 ? Boxes.first().data() : NULL)) {
		m_pCurrentBox = Boxes.count() == 1 ? Boxes.first().data() : NULL;
		m_FullRefresh = true;
	}

	if (m_FullRefresh) {
		m_pTraceModel->Clear();
		m_FullRefresh = false;
	}

	QVector<CTraceEntryPtr> ResourceLog = theAPI->GetTrace();
	QList<QVariant> Added = m_pTraceModel->Sync(ResourceLog, CTraceView__Filter, this);
	
	if (m_pTraceModel->IsTree())
	{
		QTimer::singleShot(100, this, [this, Added]() {
			//CSortFilterProxyModel* pSortProxy = (CSortFilterProxyModel*)GetModel();
			foreach(const QVariant ID, Added) {
			//	m_pTreeList->expand(pSortProxy->mapFromSource(m_pTraceModel->FindIndex(ID)));
				m_pTreeList->expand(m_pTraceModel->FindIndex(ID));
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

void CTraceView::SetFilter(const QRegExp& Exp, bool bHighLight, int Col)
{

	m_FilterExp = Exp;
	m_bHighLight = bHighLight;
	//m_FilterCol = Col;

	m_FullRefresh = true;
}

void CTraceView::SelectNext()
{
}

void CTraceView::OnSetPidFilter()
{
	m_FilterPid = m_pTracePid->currentData().toUInt();
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
	m_pTreeList->expandAll();
}

void CTraceView::OnSetTidFilter()
{
	m_FilterTid = m_pTraceTid->currentData().toUInt();
	//m_pSortProxy->m_FilterTid = m_pTraceTid->currentData().toUInt();

	//m_pSortProxy->setFilterKeyColumn(m_pSortProxy->filterKeyColumn());
	m_FullRefresh = true;
	m_pTreeList->expandAll();
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
	m_pTreeList->expandAll();
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

	QVector<CTraceEntryPtr> ResourceLog = theAPI->GetTrace();
	for (int i = 0; i < ResourceLog.count(); i++)
	{
		CTraceEntryPtr pEntry = ResourceLog.at(i);

		//int iFilter = CTraceView__Filter(pEntry, this);
		//if (!iFilter)
		//	continue;

		QStringList Line;
		Line.append(pEntry->GetTimeStamp().toString("hh:mm:ss.zzz"));
		QString Name = pEntry->GetProcessName();
		Line.append(Name.isEmpty() ? tr("Unknown") : Name);
		Line.append(QString("%1").arg(pEntry->GetProcessId()));
		Line.append(QString("%1").arg(pEntry->GetThreadId()));
		Line.append(pEntry->GetTypeStr());
		Line.append(pEntry->GetStautsStr());
		Line.append(pEntry->GetMessage());

		File.write(Line.join("\t").toLatin1() + "\n");
	}

	File.close();
}