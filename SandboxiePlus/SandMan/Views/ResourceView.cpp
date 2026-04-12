#include "stdafx.h"
#include "ResourceView.h"
#include "..\SandMan.h"
#include "..\SbiePlusAPI.h"
#include "..\..\MiscHelpers\Common\Common.h"

#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")

////////////////////////////////////////////////////////////////////////////////////////
// CResourceView

CResourceView::CResourceView(QWidget* parent) : QWidget(parent)
{
	m_pMainLayout = new QVBoxLayout();
	m_pMainLayout->setContentsMargins(0, 0, 0, 0);
	this->setLayout(m_pMainLayout);

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

	// Set reasonable column widths
	m_pTree->setColumnWidth(0, 250);	// Name
	m_pTree->setColumnWidth(1, 60);		// PID
	m_pTree->setColumnWidth(2, 70);		// CPU
	m_pTree->setColumnWidth(3, 100);	// Working Set
	m_pTree->setColumnWidth(4, 100);	// Private Bytes
	m_pTree->setColumnWidth(5, 100);	// Status

	m_pMainLayout->addWidget(m_pTree);

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
	}

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

	// Update tree - try to preserve expansion state
	QSet<QString> expandedBoxes;
	for (int i = 0; i < m_pTree->topLevelItemCount(); i++) {
		QTreeWidgetItem* pItem = m_pTree->topLevelItem(i);
		if (pItem->isExpanded())
			expandedBoxes.insert(pItem->data(0, Qt::UserRole).toString());
	}

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
		QTreeWidgetItem* pBoxItem = new QTreeWidgetItem();

		// Display name
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
		}
		else {
			pBoxItem->setText(2, "-");
			pBoxItem->setText(3, "-");
			pBoxItem->setText(4, "-");
			pBoxItem->setText(5, box.DiskSize > 0 ? tr("Inactive") : tr("Empty"));
		}

		// Add child processes
		for (const SProcessStats& proc : box.Processes)
		{
			QTreeWidgetItem* pProcItem = new QTreeWidgetItem(pBoxItem);
			pProcItem->setText(0, proc.ProcessName);
			pProcItem->setText(1, QString::number(proc.ProcessId));
			pProcItem->setText(2, QString("%1%").arg(proc.CpuUsage, 0, 'f', 1));
			pProcItem->setText(3, FormatSize(proc.WorkingSetSize));
			pProcItem->setText(4, FormatSize(proc.PrivateBytes));
			pProcItem->setText(5, tr("Running"));

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

		m_pTree->addTopLevelItem(pBoxItem);

		// Restore expansion state; auto-expand active boxes
		if (box.ProcessCount > 0 || expandedBoxes.contains(box.BoxName))
			pBoxItem->setExpanded(true);
	}
}

void CResourceView::Clear()
{
	m_pTree->clear();
	m_LastTimes.clear();
}
