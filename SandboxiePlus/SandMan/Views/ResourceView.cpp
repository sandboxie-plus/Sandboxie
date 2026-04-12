#include "stdafx.h"
#include "ResourceView.h"
#include "..\SandMan.h"
#include "..\SbiePlusAPI.h"
#include "..\..\MiscHelpers\Common\Common.h"

#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")

#include <QHeaderView>
#include <QMessageBox>

////////////////////////////////////////////////////////////////////////////////////////
// CResourceView

CResourceView::CResourceView(QWidget* parent) : QWidget(parent)
{
	m_pMainLayout = new QVBoxLayout();
	m_pMainLayout->setContentsMargins(0, 0, 0, 0);
	m_pMainLayout->setSpacing(0);
	this->setLayout(m_pMainLayout);

	// === Toolbar ===
	m_pToolBar = new QToolBar();
	m_pToolBar->setIconSize(QSize(16, 16));

	m_pRefreshAction = m_pToolBar->addAction(CSandMan::GetIcon("Refresh"), tr("Refresh"), this, SLOT(Refresh()));
	m_pRefreshAction->setToolTip(tr("Refresh resource data now"));

	m_pToolBar->addSeparator();

	m_pTerminateAction = m_pToolBar->addAction(CSandMan::GetIcon("Remove"), tr("Terminate Process"), this, SLOT(OnTerminateProcess()));
	m_pTerminateAction->setToolTip(tr("Terminate the selected process"));
	m_pTerminateAction->setEnabled(false);

	m_pTerminateBoxAction = m_pToolBar->addAction(CSandMan::GetIcon("Stop"), tr("Terminate All in Box"), this, SLOT(OnTerminateBox()));
	m_pTerminateBoxAction->setToolTip(tr("Terminate all processes in the selected sandbox"));
	m_pTerminateBoxAction->setEnabled(false);

	m_pToolBar->addSeparator();

	m_pExpandAllAction = m_pToolBar->addAction(CSandMan::GetIcon("Expand"), tr("Expand All"), this, SLOT(OnExpandAll()));
	m_pCollapseAllAction = m_pToolBar->addAction(CSandMan::GetIcon("Collapse"), tr("Collapse All"), this, SLOT(OnCollapseAll()));

	// Spacer to push filter to the right
	QWidget* pSpacer = new QWidget();
	pSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	m_pToolBar->addWidget(pSpacer);

	// Filter
	QLabel* pFilterLabel = new QLabel(tr("Filter:"));
	pFilterLabel->setContentsMargins(4, 0, 4, 0);
	m_pToolBar->addWidget(pFilterLabel);

	m_pFilterEdit = new QLineEdit();
	m_pFilterEdit->setPlaceholderText(tr("Search sandbox or process..."));
	m_pFilterEdit->setMaximumWidth(200);
	m_pFilterEdit->setClearButtonEnabled(true);
	connect(m_pFilterEdit, &QLineEdit::textChanged, this, &CResourceView::OnFilterChanged);
	m_pToolBar->addWidget(m_pFilterEdit);

	m_pMainLayout->addWidget(m_pToolBar);

	// === Summary bar ===
	m_pSummaryLabel = new QLabel();
	m_pSummaryLabel->setContentsMargins(6, 2, 6, 2);
	m_pSummaryLabel->setStyleSheet("QLabel { background-color: palette(alternate-base); padding: 2px 6px; }");
	m_pMainLayout->addWidget(m_pSummaryLabel);

	// === Tree Widget ===
	m_pTree = new QTreeWidget();
	m_pTree->setHeaderLabels(QStringList() << tr("Sandbox / Process") << tr("PID") << tr("CPU") << tr("Working Set") << tr("Private Bytes") << tr("Status"));
	m_pTree->setColumnCount(6);
	m_pTree->setRootIsDecorated(true);
	m_pTree->setAlternatingRowColors(theConf->GetBool("Options/AltRowColors", false));
	m_pTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_pTree->setUniformRowHeights(true);
	m_pTree->setSortingEnabled(false);
	m_pTree->setItemDelegate(new CTreeItemDelegate());
	m_pTree->setExpandsOnDoubleClick(true);
	m_pTree->setContextMenuPolicy(Qt::CustomContextMenu);

	// Set reasonable column widths
	m_pTree->setColumnWidth(0, 250);	// Name
	m_pTree->setColumnWidth(1, 60);		// PID
	m_pTree->setColumnWidth(2, 70);		// CPU
	m_pTree->setColumnWidth(3, 100);	// Working Set
	m_pTree->setColumnWidth(4, 100);	// Private Bytes
	m_pTree->setColumnWidth(5, 100);	// Status

	connect(m_pTree, &QTreeWidget::customContextMenuRequested, this, &CResourceView::OnContextMenu);
	connect(m_pTree, &QTreeWidget::itemSelectionChanged, this, [this]() {
		QList<QTreeWidgetItem*> selectedItems = m_pTree->selectedItems();
		bool hasSelection = !selectedItems.isEmpty();
		bool hasProcessSelected = false;
		bool hasBoxSelected = false;

		for (auto* item : selectedItems) {
			if (item->parent() == nullptr)
				hasBoxSelected = true;
			else
				hasProcessSelected = true;
		}

		m_pTerminateAction->setEnabled(hasProcessSelected);
		m_pTerminateBoxAction->setEnabled(hasBoxSelected);
	});

	m_pMainLayout->addWidget(m_pTree);

	// === Context Menu ===
	m_pContextMenu = new QMenu(this);
	m_pContextMenu->addAction(m_pTerminateAction);
	m_pContextMenu->addAction(m_pTerminateBoxAction);
	m_pContextMenu->addSeparator();
	m_pContextMenu->addAction(m_pRefreshAction);
	m_pContextMenu->addSeparator();
	m_pContextMenu->addAction(m_pExpandAllAction);
	m_pContextMenu->addAction(m_pCollapseAllAction);

	// Refresh every 2 seconds
	m_uTimerID = startTimer(2000);
}

CResourceView::~CResourceView()
{
	killTimer(m_uTimerID);
}

void CResourceView::timerEvent(QTimerEvent* pEvent)
{
	if (pEvent->timerId() != m_uTimerID)
		return;

	// Only update when the tab is visible
	if (!isVisible())
		return;

	Refresh();
}

void CResourceView::OnContextMenu(const QPoint& pos)
{
	m_pContextMenu->popup(m_pTree->viewport()->mapToGlobal(pos));
}

void CResourceView::OnTerminateProcess()
{
	QList<QTreeWidgetItem*> selectedItems = m_pTree->selectedItems();
	if (selectedItems.isEmpty())
		return;

	// Collect selected process items (children only)
	QStringList procNames;
	for (auto* item : selectedItems) {
		if (item->parent() != nullptr) { // it's a process item
			procNames.append(QString("%1 (PID: %2)").arg(item->text(0)).arg(item->text(1)));
		}
	}

	if (procNames.isEmpty())
		return;

	if (QMessageBox::question(this, tr("Terminate Process"),
		tr("Are you sure you want to terminate %1 process(es)?").arg(procNames.count()) + "\n\n" + procNames.join("\n"),
		QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
		return;

	for (auto* item : selectedItems) {
		if (item->parent() == nullptr)
			continue; // skip boxes

		quint32 pid = item->text(1).toUInt();
		QString boxName = item->parent()->data(0, Qt::UserRole).toString();

		CSandBoxPtr pBox = theAPI->GetAllBoxes().value(boxName);
		if (pBox) {
			QMap<quint32, CBoxedProcessPtr> procs = pBox->GetProcessList();
			auto it = procs.find(pid);
			if (it != procs.end()) {
				it.value()->Terminate();
			}
		}
	}

	Refresh();
}

void CResourceView::OnTerminateBox()
{
	QList<QTreeWidgetItem*> selectedItems = m_pTree->selectedItems();
	if (selectedItems.isEmpty())
		return;

	QStringList boxNames;
	for (auto* item : selectedItems) {
		if (item->parent() == nullptr) { // it's a box item
			QString boxName = item->data(0, Qt::UserRole).toString();
			if (!boxNames.contains(boxName))
				boxNames.append(boxName);
		}
	}

	if (boxNames.isEmpty())
		return;

	if (QMessageBox::question(this, tr("Terminate Sandbox"),
		tr("Are you sure you want to terminate all processes in %1 sandbox(es)?").arg(boxNames.count()) + "\n\n" + boxNames.join("\n"),
		QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
		return;

	for (const QString& boxName : boxNames) {
		CSandBoxPtr pBox = theAPI->GetAllBoxes().value(boxName);
		if (pBox) {
			pBox->TerminateAll();
		}
	}

	Refresh();
}

void CResourceView::OnExpandAll()
{
	m_pTree->expandAll();
}

void CResourceView::OnCollapseAll()
{
	m_pTree->collapseAll();
}

void CResourceView::OnFilterChanged(const QString& text)
{
	Q_UNUSED(text);
	Refresh();
}

bool CResourceView::MatchesFilter(const QString& boxName, const QList<SProcessStats>& processes) const
{
	QString filter = m_pFilterEdit->text().trimmed();
	if (filter.isEmpty())
		return true;

	if (boxName.contains(filter, Qt::CaseInsensitive))
		return true;

	for (const auto& proc : processes) {
		if (proc.ProcessName.contains(filter, Qt::CaseInsensitive))
			return true;
		if (QString::number(proc.ProcessId).contains(filter))
			return true;
	}

	return false;
}

static QString FormatSize(quint64 size)
{
	if (size == 0)
		return "0 B";
	
	const char* units[] = { "B", "KB", "MB", "GB", "TB" };
	int unitIndex = 0;
	double dSize = (double)size;

	while (dSize >= 1024.0 && unitIndex < 4) {
		dSize /= 1024.0;
		unitIndex++;
	}

	if (unitIndex == 0)
		return QString("%1 B").arg(size);
	return QString("%1 %2").arg(dSize, 0, 'f', 1).arg(units[unitIndex]);
}

void CResourceView::UpdateProcessStats(SProcessStats& Stats)
{
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, Stats.ProcessId);
	if (hProcess == NULL)
		return;

	// Memory info
	PROCESS_MEMORY_COUNTERS_EX pmc;
	memset(&pmc, 0, sizeof(pmc));
	pmc.cb = sizeof(pmc);
	if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
		Stats.WorkingSetSize = pmc.WorkingSetSize;
		Stats.PrivateBytes = pmc.PrivateUsage;
	}

	// CPU time
	FILETIME ftCreation, ftExit, ftKernel, ftUser;
	if (GetProcessTimes(hProcess, &ftCreation, &ftExit, &ftKernel, &ftUser)) {
		Stats.KernelTime = ((quint64)ftKernel.dwHighDateTime << 32) | ftKernel.dwLowDateTime;
		Stats.UserTime = ((quint64)ftUser.dwHighDateTime << 32) | ftUser.dwLowDateTime;
		Stats.CpuUsage = CalcCpuUsage(Stats.ProcessId, Stats.KernelTime, Stats.UserTime);
	}

	CloseHandle(hProcess);
}

double CResourceView::CalcCpuUsage(quint32 pid, quint64 kernelTime, quint64 userTime)
{
	quint64 totalTime = kernelTime + userTime;
	quint64 now = GetTickCount64() * 10000ULL; // convert ms to 100-ns intervals

	auto it = m_LastTimes.find(pid);
	if (it == m_LastTimes.end()) {
		m_LastTimes[pid] = qMakePair(totalTime, now);
		return 0.0;
	}

	quint64 prevTotal = it->first;
	quint64 prevTimestamp = it->second;

	quint64 elapsedTime = now - prevTimestamp;
	if (elapsedTime == 0) {
		return 0.0;
	}

	quint64 cpuTimeDelta = totalTime - prevTotal;

	// Get number of logical processors
	static int nProcessors = 0;
	if (nProcessors == 0) {
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		nProcessors = si.dwNumberOfProcessors;
		if (nProcessors == 0) nProcessors = 1;
	}

	double cpuUsage = (double)cpuTimeDelta / (double)elapsedTime * 100.0 / nProcessors;
	if (cpuUsage > 100.0) cpuUsage = 100.0;
	if (cpuUsage < 0.0) cpuUsage = 0.0;

	m_LastTimes[pid] = qMakePair(totalTime, now);
	return cpuUsage;
}

void CResourceView::Refresh()
{
	if (!theAPI || !theAPI->IsConnected())
		return;

	// Build box stats
	QMap<QString, SBoxStats> BoxStatsMap;
	QMap<QString, CSandBoxPtr> Boxes = theAPI->GetAllBoxes();

	int totalBoxes = 0;
	int totalActiveBoxes = 0;
	int totalProcesses = 0;
	quint64 totalMemory = 0;

	for (auto I = Boxes.constBegin(); I != Boxes.constEnd(); ++I)
	{
		CSandBoxPtr pBox = I.value();
		if (!pBox->IsEnabled())
			continue;

		auto pBoxEx = pBox.objectCast<CSandBoxPlus>();
		if (!pBoxEx)
			continue;

		SBoxStats& BoxStats = BoxStatsMap[pBox->GetName()];
		BoxStats.BoxName = pBox->GetName();
		BoxStats.ProcessCount = 0;
		BoxStats.TotalWorkingSet = 0;
		BoxStats.TotalPrivateBytes = 0;
		BoxStats.TotalCpuUsage = 0.0;
		BoxStats.DiskSize = pBoxEx->GetSize();

		QMap<quint32, CBoxedProcessPtr> procs = pBox->GetProcessList();
		for (auto J = procs.constBegin(); J != procs.constEnd(); ++J)
		{
			CBoxedProcessPtr pProcess = J.value();
			if (pProcess->IsTerminated())
				continue;

			SProcessStats ProcStats;
			ProcStats.ProcessId = pProcess->GetProcessId();
			ProcStats.ProcessName = pProcess->GetProcessName();
			ProcStats.BoxName = pBox->GetName();
			ProcStats.WorkingSetSize = 0;
			ProcStats.PrivateBytes = 0;
			ProcStats.KernelTime = 0;
			ProcStats.UserTime = 0;
			ProcStats.CpuUsage = 0.0;

			UpdateProcessStats(ProcStats);

			BoxStats.ProcessCount++;
			BoxStats.TotalWorkingSet += ProcStats.WorkingSetSize;
			BoxStats.TotalPrivateBytes += ProcStats.PrivateBytes;
			BoxStats.TotalCpuUsage += ProcStats.CpuUsage;

			BoxStats.Processes.append(ProcStats);
		}

		totalBoxes++;
		if (BoxStats.ProcessCount > 0)
			totalActiveBoxes++;
		totalProcesses += BoxStats.ProcessCount;
		totalMemory += BoxStats.TotalWorkingSet;
	}

	// Update summary bar
	m_pSummaryLabel->setText(tr("Boxes: %1 (%2 active)  |  Processes: %3  |  Total Memory: %4")
		.arg(totalBoxes).arg(totalActiveBoxes).arg(totalProcesses).arg(FormatSize(totalMemory)));

	// Clean up stale CPU entries
	QSet<quint32> activePids;
	for (auto& box : BoxStatsMap) {
		for (auto& proc : box.Processes) {
			activePids.insert(proc.ProcessId);
		}
	}
	for (auto it = m_LastTimes.begin(); it != m_LastTimes.end(); ) {
		if (!activePids.contains(it.key()))
			it = m_LastTimes.erase(it);
		else
			++it;
	}

	// Update tree - try to preserve expansion, selection and scroll state
	QSet<QString> expandedBoxes;
	QSet<QString> selectedBoxes;
	QSet<quint32> selectedPids;

	for (int i = 0; i < m_pTree->topLevelItemCount(); i++) {
		QTreeWidgetItem* pItem = m_pTree->topLevelItem(i);
		QString boxName = pItem->data(0, Qt::UserRole).toString();
		if (pItem->isExpanded())
			expandedBoxes.insert(boxName);
		if (pItem->isSelected())
			selectedBoxes.insert(boxName);

		for (int j = 0; j < pItem->childCount(); j++) {
			QTreeWidgetItem* pChild = pItem->child(j);
			if (pChild->isSelected())
				selectedPids.insert(pChild->text(1).toUInt());
		}
	}
	int vScrollPos = m_pTree->verticalScrollBar()->value();

	m_pTree->clear();

	// Sort boxes: active ones first, then by name
	QList<SBoxStats> sortedBoxes = BoxStatsMap.values();
	std::sort(sortedBoxes.begin(), sortedBoxes.end(), [](const SBoxStats& a, const SBoxStats& b) {
		if (a.ProcessCount != b.ProcessCount) {
			if (a.ProcessCount == 0) return false;
			if (b.ProcessCount == 0) return true;
			return a.TotalCpuUsage > b.TotalCpuUsage;
		}
		return a.BoxName.compare(b.BoxName, Qt::CaseInsensitive) < 0;
	});

	for (const SBoxStats& box : sortedBoxes)
	{
		// Apply filter
		if (!MatchesFilter(box.BoxName, box.Processes))
			continue;

		QTreeWidgetItem* pBoxItem = new QTreeWidgetItem();

		QString displayName = box.BoxName;

		pBoxItem->setText(0, displayName);
		pBoxItem->setData(0, Qt::UserRole, box.BoxName);

		// Process count as PID column
		pBoxItem->setText(1, QString::number(box.ProcessCount));

		if (box.ProcessCount > 0) {
			pBoxItem->setText(2, QString("%1%").arg(box.TotalCpuUsage, 0, 'f', 1));
			pBoxItem->setText(3, FormatSize(box.TotalWorkingSet));
			pBoxItem->setText(4, FormatSize(box.TotalPrivateBytes));
			pBoxItem->setText(5, tr("Active"));

			// Bold font for active boxes
			QFont fnt = pBoxItem->font(0);
			fnt.setBold(true);
			pBoxItem->setFont(0, fnt);

			// Color-code high CPU usage (> 50% = red tint)
			if (box.TotalCpuUsage > 50.0) {
				pBoxItem->setForeground(2, QBrush(QColor(220, 50, 50)));
				QFont cpuFnt = pBoxItem->font(2);
				cpuFnt.setBold(true);
				pBoxItem->setFont(2, cpuFnt);
			} else if (box.TotalCpuUsage > 20.0) {
				pBoxItem->setForeground(2, QBrush(QColor(200, 140, 0)));
			}
		}
		else {
			pBoxItem->setText(2, "-");
			pBoxItem->setText(3, "-");
			pBoxItem->setText(4, "-");
			pBoxItem->setText(5, box.DiskSize > 0 ? tr("Inactive") : tr("Empty"));
		}

		// Add child processes
		QString filterText = m_pFilterEdit->text().trimmed();
		for (const SProcessStats& proc : box.Processes)
		{
			// If filtering, also filter individual processes
			if (!filterText.isEmpty()) {
				bool match = proc.ProcessName.contains(filterText, Qt::CaseInsensitive)
					|| QString::number(proc.ProcessId).contains(filterText)
					|| box.BoxName.contains(filterText, Qt::CaseInsensitive);
				if (!match)
					continue;
			}

			QTreeWidgetItem* pProcItem = new QTreeWidgetItem(pBoxItem);
			pProcItem->setText(0, proc.ProcessName);
			pProcItem->setText(1, QString::number(proc.ProcessId));
			pProcItem->setText(2, QString("%1%").arg(proc.CpuUsage, 0, 'f', 1));
			pProcItem->setText(3, FormatSize(proc.WorkingSetSize));
			pProcItem->setText(4, FormatSize(proc.PrivateBytes));
			pProcItem->setText(5, tr("Running"));

			// Color-code high CPU processes
			if (proc.CpuUsage > 50.0) {
				pProcItem->setForeground(2, QBrush(QColor(220, 50, 50)));
				QFont cpuFnt = pProcItem->font(2);
				cpuFnt.setBold(true);
				pProcItem->setFont(2, cpuFnt);
			} else if (proc.CpuUsage > 20.0) {
				pProcItem->setForeground(2, QBrush(QColor(200, 140, 0)));
			}

			// Align numeric columns right
			pProcItem->setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);
			pProcItem->setTextAlignment(2, Qt::AlignRight | Qt::AlignVCenter);
			pProcItem->setTextAlignment(3, Qt::AlignRight | Qt::AlignVCenter);
			pProcItem->setTextAlignment(4, Qt::AlignRight | Qt::AlignVCenter);
		}

		// Align box-level numeric columns right
		pBoxItem->setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);
		pBoxItem->setTextAlignment(2, Qt::AlignRight | Qt::AlignVCenter);
		pBoxItem->setTextAlignment(3, Qt::AlignRight | Qt::AlignVCenter);
		pBoxItem->setTextAlignment(4, Qt::AlignRight | Qt::AlignVCenter);

		if (selectedBoxes.contains(box.BoxName))
			pBoxItem->setSelected(true);

		m_pTree->addTopLevelItem(pBoxItem);

		// Restore expansion state; auto-expand active boxes
		if (box.ProcessCount > 0 || expandedBoxes.contains(box.BoxName))
			pBoxItem->setExpanded(true);
		
		// Restore specific child selection
		for (int i = 0; i < pBoxItem->childCount(); ++i) {
			QTreeWidgetItem* pChild = pBoxItem->child(i);
			if (selectedPids.contains(pChild->text(1).toUInt()))
				pChild->setSelected(true);
		}
	}

	// Restore scroll position
	m_pTree->verticalScrollBar()->setValue(vScrollPos);
}

void CResourceView::Clear()
{
	m_pTree->clear();
	m_LastTimes.clear();
	m_pSummaryLabel->setText("");
}
