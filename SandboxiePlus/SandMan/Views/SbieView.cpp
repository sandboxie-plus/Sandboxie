#include "stdafx.h"
#include "SbieView.h"
#include "..\SandMan.h"
#include "../QSbieAPI/SbieAPI.h"
#include "../../MiscHelpers/Common/SortFilterProxyModel.h"
#include "../../MiscHelpers/Common/Settings.h"
#include "../Windows/OptionsWindow.h"

CSbieView::CSbieView(QWidget* parent) : CPanelView(parent)
{
	m_pMainLayout = new QVBoxLayout();
	m_pMainLayout->setMargin(0);
	this->setLayout(m_pMainLayout);

	m_pSbieModel = new CSbieModel();
	m_pSbieModel->SetTree(true);
	m_pSbieModel->SetUseIcons(true);

	m_pSortProxy = new CSortFilterProxyModel(false, this);
	m_pSortProxy->setSortRole(Qt::EditRole);
	m_pSortProxy->setSourceModel(m_pSbieModel);
	m_pSortProxy->setDynamicSortFilter(true);


	// SbieTree
	m_pSbieTree = new QTreeViewEx();
	//m_pSbieTree->setItemDelegate(theGUI->GetItemDelegate());

	m_pSbieTree->setModel(m_pSortProxy);

	m_pSbieTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_pSbieTree->setSortingEnabled(true);

	m_pSbieTree->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pSbieTree, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnMenu(const QPoint &)));
	connect(m_pSbieTree, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(OnDoubleClicked(const QModelIndex&)));
	connect(m_pSbieTree->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), SLOT(ProcessSelection(QItemSelection, QItemSelection)));

	//connect(theGUI, SIGNAL(ReloadPanels()), m_pSbieModel, SLOT(Clear()));

	m_pMainLayout->addWidget(m_pSbieTree);
	// 

	m_pMainLayout->addWidget(new CFinder(m_pSortProxy, this));


	connect(m_pSbieModel, SIGNAL(ToolTipCallback(const QVariant&, QString&)), this, SLOT(OnToolTipCallback(const QVariant&, QString&)), Qt::DirectConnection);

	m_pMenuRun = m_pMenu->addMenu(tr("Run"));
		m_pMenuRunAny = m_pMenuRun->addAction(tr("Run Program"), this, SLOT(OnSandBoxAction()));
		m_pMenuRunBrowser = m_pMenuRun->addAction(tr("Run Web Browser"), this, SLOT(OnSandBoxAction()));
		m_pMenuRunExplorer = m_pMenuRun->addAction(tr("Run Explorer"), this, SLOT(OnSandBoxAction()));
		m_pMenuRunCmd = m_pMenuRun->addAction(tr("Run Cmd.exe"), this, SLOT(OnSandBoxAction()));
	m_pMenuEmptyBox = m_pMenu->addAction(tr("Terminate All Programs"), this, SLOT(OnSandBoxAction()));
	m_pMenu->addSeparator();
	m_pMenuCleanUp = m_pMenu->addAction(tr("Delete Content"), this, SLOT(OnSandBoxAction()));
	m_pMenu->addSeparator();
	m_pMenuPresets = m_pMenu->addMenu(tr("Sandbox Presets"));
		m_pMenuPresetsLogApi = m_pMenuPresets->addAction(tr("Enable API Call logging"), this, SLOT(OnSandBoxAction()));
		m_pMenuPresetsLogApi->setCheckable(true);
		m_pMenuPresetsINet = m_pMenuPresets->addAction(tr("Block Internet Access"), this, SLOT(OnSandBoxAction()));
		m_pMenuPresetsINet->setCheckable(true);
		m_pMenuPresetsShares = m_pMenuPresets->addAction(tr("Allow Network Shares"), this, SLOT(OnSandBoxAction()));
		m_pMenuPresetsShares->setCheckable(true);
		m_pMenuPresetsNoAdmin = m_pMenuPresets->addAction(tr("Drop Admin Rights"), this, SLOT(OnSandBoxAction()));
		m_pMenuPresetsNoAdmin->setCheckable(true);
	m_pMenuOptions = m_pMenu->addAction(tr("Sandbox Options"), this, SLOT(OnSandBoxAction()));
	m_pMenuRename = m_pMenu->addAction(tr("Rename Sandbox"), this, SLOT(OnSandBoxAction()));
	m_pMenuRemove = m_pMenu->addAction(tr("Remove Sandbox"), this, SLOT(OnSandBoxAction()));
	m_iMenuBox = m_pMenu->actions().count();

	m_pMenuTerminate = m_pMenu->addAction(tr("Terminate"), this, SLOT(OnProcessAction()));
	m_pMenuTerminate->setShortcut(QKeySequence::Delete);
	m_pMenuTerminate->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	this->addAction(m_pMenuTerminate);
	m_pMenuSuspend = m_pMenu->addAction(tr("Suspend"), this, SLOT(OnProcessAction()));
	m_pMenuResume = m_pMenu->addAction(tr("Resume"), this, SLOT(OnProcessAction()));
	m_iMenuProc = m_pMenu->actions().count();

	QByteArray Columns = theConf->GetBlob("GUI/BoxTree_Columns");
	if (Columns.isEmpty())
		m_pSbieTree->OnResetColumns();
	else
		m_pSbieTree->restoreState(Columns);

	//m_pMenu = new QMenu();
	AddPanelItemsToMenu();
}

CSbieView::~CSbieView()
{
	theConf->SetBlob("GUI/BoxTree_Columns", m_pSbieTree->saveState());
}

void CSbieView::Refresh()
{
	QList<QVariant> Added = m_pSbieModel->Sync(theAPI->GetAllBoxes());

	if (m_pSbieModel->IsTree())
	{
		QTimer::singleShot(100, this, [this, Added]() {
			foreach(const QVariant ID, Added) {
				m_pSbieTree->expand(m_pSortProxy->mapFromSource(m_pSbieModel->FindIndex(ID)));
			}
		});
	}
}

void CSbieView::OnToolTipCallback(const QVariant& ID, QString& ToolTip)
{
	if (ID.type() == QVariant::String)
	{
		QString BoxName = ID.toString();
		CSandBoxPtr pBox = theAPI->GetBoxByName(BoxName);
		CSandBoxPlus* pBoxEx = qobject_cast<CSandBoxPlus*>(pBox.data());
		if (!pBoxEx)
			return;

		// todo more info

		ToolTip = BoxName + "\n";
		ToolTip += tr("    File root: %1\n").arg(pBoxEx->GetFileRoot());
		ToolTip += tr("    Registry root: %1\n").arg(pBoxEx->GetRegRoot());
		ToolTip += tr("    IPC root: %1\n").arg(pBoxEx->GetIpcRoot());
		
		ToolTip += tr("Options:\n    ");
		ToolTip += pBoxEx->GetStatusStr().replace(", ", "\n    ");
	}
	else if (quint64 ProcessId = ID.toULongLong())
	{
		// todo proc info
	}
}

void CSbieView::OnMenu(const QPoint& Point)
{
	CSandBoxPtr pBox;
	CBoxedProcessPtr pProcess;
	int iProcessCount = 0;
	int iSandBoxeCount = 0;
	int iSuspendedCount = 0;
	foreach(const QModelIndex& Index, m_pSbieTree->selectedRows())
	{
		QModelIndex ModelIndex = m_pSortProxy->mapToSource(Index);
		pProcess = m_pSbieModel->GetProcess(ModelIndex);
		if (pProcess)
		{
			iProcessCount++;
			if (pProcess->IsSuspended())
				iSuspendedCount++;
		}
		else
		{
			pBox = m_pSbieModel->GetSandBox(ModelIndex);
			if (pBox)
				iSandBoxeCount++;
		}
	}

	QList<QAction*> MenuActions = m_pMenu->actions();

	for (int i = 0; i < m_iMenuBox; i++)
		MenuActions[i]->setVisible(iSandBoxeCount > 0 && iProcessCount == 0);
	m_pMenuRun->setEnabled(iSandBoxeCount == 1);
	m_pMenuRename->setEnabled(iSandBoxeCount == 1);

	m_pMenuPresets->setEnabled(iSandBoxeCount == 1);
	m_pMenuPresetsLogApi->setChecked(pBox && pBox.objectCast<CSandBoxPlus>()->HasLogApi());
	m_pMenuPresetsINet->setChecked(pBox && pBox.objectCast<CSandBoxPlus>()->IsINetBlocked());
	m_pMenuPresetsShares->setChecked(pBox && pBox.objectCast<CSandBoxPlus>()->HasSharesAccess());
	m_pMenuPresetsNoAdmin->setChecked(pBox && pBox.objectCast<CSandBoxPlus>()->IsDropRights());

	m_pMenuOptions->setEnabled(iSandBoxeCount == 1);

	for (int i = m_iMenuBox; i < m_iMenuProc; i++)
		MenuActions[i]->setVisible(iProcessCount > 0 && iSandBoxeCount == 0);
	m_pMenuSuspend->setEnabled(iProcessCount > iSuspendedCount);
	m_pMenuResume->setEnabled(iSuspendedCount > 0);

	CPanelView::OnMenu(Point);
}

void CSbieView::OnSandBoxAction()
{
	QList<SB_STATUS> Results;

	QAction* Action = qobject_cast<QAction*>(sender());
	QList<CSandBoxPtr> SandBoxes = CSbieView::GetSelectedBoxes();
	if (SandBoxes.isEmpty())
		return;
	if (Action == m_pMenuRunAny)
	{
		/*QString Command = ShowRunDialog(SandBoxes.first()->GetName());
		if(!Command.isEmpty())
			SandBoxes.first()->RunCommand(Command);*/

		Results.append(SandBoxes.first()->RunStart("run_dialog"));
	}
	else if (Action == m_pMenuRunBrowser)
		Results.append(SandBoxes.first()->RunStart("default_browser"));
	else if (Action == m_pMenuRunExplorer)
		Results.append(SandBoxes.first()->RunCommand("explorer.exe /e,::{20D04FE0-3AEA-1069-A2D8-08002B30309D}"));
	else if (Action == m_pMenuRunCmd)
		Results.append(SandBoxes.first()->RunCommand("cmd.exe"));
	else if (Action == m_pMenuPresetsLogApi)
		SandBoxes.first().objectCast<CSandBoxPlus>()->SetLogApi(m_pMenuPresetsLogApi->isChecked());
	else if (Action == m_pMenuPresetsINet)
		SandBoxes.first().objectCast<CSandBoxPlus>()->SetINetBlock(m_pMenuPresetsINet->isChecked());
	else if (Action == m_pMenuPresetsShares)
		SandBoxes.first().objectCast<CSandBoxPlus>()->SetAllowShares(m_pMenuPresetsShares->isChecked());
	else if (Action == m_pMenuPresetsNoAdmin)
		SandBoxes.first().objectCast<CSandBoxPlus>()->SetDropRights(m_pMenuPresetsNoAdmin->isChecked());
	else if (Action == m_pMenuOptions)
	{
		COptionsWindow* pOptionsWindow = new COptionsWindow(SandBoxes.first(), SandBoxes.first()->GetName(), this);
		pOptionsWindow->show();
	}
	else if (Action == m_pMenuRename)
	{
		QString OldValue = SandBoxes.first()->GetName().replace("_", " ");
		QString Value = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter a new name for the Sandbox."), QLineEdit::Normal, OldValue);
		if (Value.isEmpty() || Value == OldValue)
			return;
		Results.append((SandBoxes.first()->RenameBox(Value)));
	}
	else if (Action == m_pMenuRemove)
	{
		if (QMessageBox("Sandboxie-Plus", tr("Do you really want remove the selected sandbox(es)?"), QMessageBox::Warning, QMessageBox::Yes, QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton).exec() != QMessageBox::Yes)
			return;

		foreach(const CSandBoxPtr& pBox, SandBoxes)
			Results.append(pBox->RemoveBox());
	}
	else if (Action == m_pMenuCleanUp)
	{
		if (QMessageBox("Sandboxie-Plus", tr("Do you really want delete the content of the selected sandbox(es)?"), QMessageBox::Warning, QMessageBox::Yes, QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton).exec() != QMessageBox::Yes)
			return;

		theGUI->GetProgressDialog()->show();

		m_BoxesToClean = 0;
		foreach(const CSandBoxPtr& pBox, SandBoxes)
		{
			m_BoxesToClean++;
			Results.append(pBox->CleanBox());
			connect(pBox.data(), SIGNAL(BoxCleaned()), this, SLOT(OnBoxCleaned()));
		}
	}
	else if (Action == m_pMenuEmptyBox)
	{
		foreach(const CSandBoxPtr& pBox, SandBoxes)
			Results.append(pBox->TerminateAll());
	}

	CSandMan::CheckResults(Results);
}

void CSbieView::OnBoxCleaned()
{
	disconnect(sender(), SIGNAL(BoxCleaned()), this, SLOT(OnBoxCleaned()));

	if(--m_BoxesToClean <= 0)
		theGUI->GetProgressDialog()->hide();
}

void CSbieView::OnProcessAction()
{
	QList<SB_STATUS> Results;

	QAction* Action = qobject_cast<QAction*>(sender());
	if (Action == m_pMenuTerminate)
	{
		if (QMessageBox("Sandboxie-Plus", tr("Do you want to %1 the selected process(es)").arg(((QAction*)sender())->text().toLower())
			, QMessageBox::Question, QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape, QMessageBox::NoButton).exec() != QMessageBox::Yes)
			return;
	}

	foreach(const CBoxedProcessPtr& pProcess, CSbieView::GetSelectedProcesses())
	{
		if (Action == m_pMenuTerminate)
			Results.append(pProcess->Terminate());
		else if (Action == m_pMenuSuspend)
			Results.append(pProcess->SetSuspend(true));
		else if (Action == m_pMenuResume)
			Results.append(pProcess->SetSuspend(false));
	}

	CSandMan::CheckResults(Results);
}

void CSbieView::OnDoubleClicked(const QModelIndex& index)
{
	QModelIndex ModelIndex = m_pSortProxy->mapToSource(index);
	CSandBoxPtr pBox = m_pSbieModel->GetSandBox(ModelIndex);
	if (pBox.isNull())
		return;

	COptionsWindow* pOptionsWindow = new COptionsWindow(pBox, pBox->GetName(), this);
	pOptionsWindow->show();
}

void CSbieView::ProcessSelection(const QItemSelection& selected, const QItemSelection& deselected)
{
	if (selected.empty())
		return;

	QItemSelectionModel* selectionModel = m_pSbieTree->selectionModel();

	QItemSelection selection = selectionModel->selection();
	QModelIndex root_parent = m_pSbieTree->currentIndex().parent();
	while (root_parent.isValid() && root_parent.parent().isValid())
		root_parent = root_parent.parent();

	QItemSelection invalid;
	foreach(const QModelIndex& index, selection.indexes())
	{
		QModelIndex parent = index.parent();
		while (parent.isValid() && parent.parent().isValid())
			parent = parent.parent();

		if (parent != root_parent)
			invalid.select(index, index);
	}
	selectionModel->select(invalid, QItemSelectionModel::Deselect);
}

QList<CSandBoxPtr> CSbieView::GetSelectedBoxes()
{
	QList<CSandBoxPtr> List;
	foreach(const QModelIndex& Index, m_pSbieTree->selectedRows())
	{
		QModelIndex ModelIndex = m_pSortProxy->mapToSource(Index);
		CSandBoxPtr pBox = m_pSbieModel->GetSandBox(ModelIndex);
		if (!pBox)
			continue;
		List.append(pBox);
	}
	return  List;
}

QList<CBoxedProcessPtr> CSbieView::GetSelectedProcesses()
{
	QList<CBoxedProcessPtr> List;
	foreach(const QModelIndex& Index, m_pSbieTree->selectedRows())
	{
		QModelIndex ModelIndex = m_pSortProxy->mapToSource(Index);
		CBoxedProcessPtr pProcess = m_pSbieModel->GetProcess(ModelIndex);
		if (!pProcess)
			return QList < CBoxedProcessPtr>();
		List.append(pProcess);
	}
	return  List;
}