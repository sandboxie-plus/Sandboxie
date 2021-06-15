#include "stdafx.h"
#include "SbieView.h"
#include "..\SandMan.h"
#include "../QSbieAPI/SbieAPI.h"
#include "../QSbieAPI/SbieUtils.h"
#include "../../MiscHelpers/Common/SortFilterProxyModel.h"
#include "../../MiscHelpers/Common/Settings.h"
#include "../../MiscHelpers/Common/Common.h"
#include "../Windows/OptionsWindow.h"
#include "../Windows/SnapshotsWindow.h"
#include <QFileIconProvider>
#include "../../MiscHelpers/Common/CheckableMessageBox.h"
#include "../Windows/RecoveryWindow.h"
#include "../Windows/NewBoxWindow.h"

#include "qt_windows.h"
#include "qwindowdefs_win.h"
#include <shellapi.h>

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
	m_pSbieTree->setExpandsOnDoubleClick(false);
	//m_pSbieTree->setItemDelegate(theGUI->GetItemDelegate());

	m_pSbieTree->setModel(m_pSortProxy);
	((CSortFilterProxyModel*)m_pSortProxy)->setView(m_pSbieTree);

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

	m_pNewBox = m_pMenu->addAction(CSandMan::GetIcon("NewBox"), tr("Create New Box"), this, SLOT(OnGroupAction()));
	m_pAddGroupe = m_pMenu->addAction(CSandMan::GetIcon("Group"), tr("Add Group"), this, SLOT(OnGroupAction()));
	m_pDelGroupe = m_pMenu->addAction(CSandMan::GetIcon("Remove"), tr("Remove Group"), this, SLOT(OnGroupAction()));
	m_iMenuTop = m_pMenu->actions().count();
	//m_pMenu->addSeparator();

	m_pMenuRun = m_pMenu->addMenu(CSandMan::GetIcon("Start"), tr("Run"));
		m_pMenuRunAny = m_pMenuRun->addAction(CSandMan::GetIcon("Run"), tr("Run Program"), this, SLOT(OnSandBoxAction()));
		m_pMenuRunMenu = m_pMenuRun->addAction(CSandMan::GetIcon("StartMenu"), tr("Run from Start Menu"), this, SLOT(OnSandBoxAction()));
		m_pMenuRunBrowser = m_pMenuRun->addAction(CSandMan::GetIcon("Internet"), tr("Run Web Browser"), this, SLOT(OnSandBoxAction()));
		m_pMenuRunMailer = m_pMenuRun->addAction(CSandMan::GetIcon("Email"), tr("Run eMail Client"), this, SLOT(OnSandBoxAction()));
		m_pMenuRunExplorer = m_pMenuRun->addAction(CSandMan::GetIcon("Explore"), tr("Run Explorer"), this, SLOT(OnSandBoxAction()));
		m_pMenuRunCmd = m_pMenuRun->addAction(CSandMan::GetIcon("Cmd"), tr("Run Cmd.exe"), this, SLOT(OnSandBoxAction()));
		m_pMenuRunCmdAdmin = m_pMenuRun->addAction(CSandMan::GetIcon("Cmd"), tr("Run Cmd.exe as Admin"), this, SLOT(OnSandBoxAction()));
		m_pMenuRun->addSeparator();
		m_iMenuRun = m_pMenuRun->actions().count();
	m_pMenuEmptyBox = m_pMenu->addAction(CSandMan::GetIcon("EmptyAll"), tr("Terminate All Programs"), this, SLOT(OnSandBoxAction()));
	m_pMenu->addSeparator();
	m_pMenuMkLink = m_pMenu->addAction(CSandMan::GetIcon("MkLink"), tr("Create Shortcut"), this, SLOT(OnSandBoxAction()));
	m_pMenu->addSeparator();
	m_pMenuExplore = m_pMenu->addAction(CSandMan::GetIcon("Explore"), tr("Explore Content"), this, SLOT(OnSandBoxAction()));
	m_pMenuSnapshots = m_pMenu->addAction(CSandMan::GetIcon("Snapshots"), tr("Snapshots Manager"), this, SLOT(OnSandBoxAction()));
	m_pMenuRecover = m_pMenu->addAction(CSandMan::GetIcon("Recover"), tr("Recover Files"), this, SLOT(OnSandBoxAction()));
	m_pMenuCleanUp = m_pMenu->addAction(CSandMan::GetIcon("Erase"), tr("Delete Content"), this, SLOT(OnSandBoxAction()));
	m_pMenu->addSeparator();
	m_pMenuPresets = m_pMenu->addMenu(CSandMan::GetIcon("Presets"), tr("Sandbox Presets"));
		
		m_pMenuPresetsAdmin = new QActionGroup(m_pMenuPresets);
		m_pMenuPresetsShowUAC = MakeAction(m_pMenuPresetsAdmin, m_pMenuPresets, tr("Ask for UAC Elevation"), 0);
		m_pMenuPresetsNoAdmin = MakeAction(m_pMenuPresetsAdmin, m_pMenuPresets, tr("Drop Admin Rights"), 1);
		m_pMenuPresetsFakeAdmin = MakeAction(m_pMenuPresetsAdmin, m_pMenuPresets, tr("Emulate Admin Rights"), 1 | 2);
		if (theAPI->IsRunningAsAdmin()) {
			m_pMenuPresetsNoAdmin->setEnabled(false);
			m_pMenuPresetsFakeAdmin->setEnabled(false);
		}
		connect(m_pMenuPresetsAdmin, SIGNAL(triggered(QAction*)), this, SLOT(OnSandBoxAction(QAction*)));

		m_pMenuPresets->addSeparator();
		m_pMenuPresetsINet = m_pMenuPresets->addAction(tr("Block Internet Access"), this, SLOT(OnSandBoxAction()));
		m_pMenuPresetsINet->setCheckable(true);
		m_pMenuPresetsShares = m_pMenuPresets->addAction(tr("Allow Network Shares"), this, SLOT(OnSandBoxAction()));
		m_pMenuPresetsShares->setCheckable(true);
	m_pMenuOptions = m_pMenu->addAction(CSandMan::GetIcon("Options"), tr("Sandbox Options"), this, SLOT(OnSandBoxAction()));
	m_pMenuRename = m_pMenu->addAction(CSandMan::GetIcon("Rename"), tr("Rename Sandbox"), this, SLOT(OnSandBoxAction()));
	m_iMoveTo = m_pMenu->actions().count();
	m_pMenuMoveTo = m_pMenu->addMenu(CSandMan::GetIcon("Group"), tr("Move to Group"));
	m_pMenuRemove = m_pMenu->addAction(CSandMan::GetIcon("Remove"), tr("Remove Sandbox"), this, SLOT(OnSandBoxAction()));
	m_iMenuBox = m_pMenu->actions().count();

	//UpdateRunMenu();

	m_pMenuTerminate = m_pMenu->addAction(CSandMan::GetIcon("Remove"), tr("Terminate"), this, SLOT(OnProcessAction()));
	m_pMenuTerminate->setShortcut(QKeySequence::Delete);
	m_pMenuLinkTo = m_pMenu->addAction(CSandMan::GetIcon("MkLink"), tr("Create Shortcut"), this, SLOT(OnProcessAction()));
	this->addAction(m_pMenuTerminate);
	m_pMenuTerminate->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	m_pMenuPreset = m_pMenu->addMenu(CSandMan::GetIcon("Presets"), tr("Preset"));
	m_pMenuPinToRun = m_pMenuPreset->addAction(tr("Pin to Run Menu"), this, SLOT(OnProcessAction()));
	m_pMenuPinToRun->setCheckable(true);
	m_pMenuBlackList = m_pMenuPreset->addAction(tr("Block and Terminate"), this, SLOT(OnProcessAction()));
	m_pMenuBlackList->setShortcut(QKeySequence("Shift+Del"));
	m_pMenuBlackList->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	this->addAction(m_pMenuBlackList);
	m_pMenuAllowInternet = m_pMenuPreset->addAction(tr("Allow internet access"), this, SLOT(OnProcessAction()));
	m_pMenuAllowInternet->setCheckable(true);
	m_pMenuMarkForced = m_pMenuPreset->addAction(tr("Force into this sandbox"), this, SLOT(OnProcessAction()));
	m_pMenuMarkForced->setCheckable(true);
	m_pMenuMarkLinger = m_pMenuPreset->addAction(tr("Set Linger Process"), this, SLOT(OnProcessAction()));
	m_pMenuMarkLinger->setCheckable(true);
	m_pMenuMarkLeader = m_pMenuPreset->addAction(tr("Set Leader Process"), this, SLOT(OnProcessAction()));
	m_pMenuMarkLeader->setCheckable(true);
	//m_pMenuSuspend = m_pMenu->addAction(tr("Suspend"), this, SLOT(OnProcessAction()));
	//m_pMenuResume = m_pMenu->addAction(tr("Resume"), this, SLOT(OnProcessAction()));
	m_iMenuProc = m_pMenu->actions().count();

	QByteArray Columns = theConf->GetBlob("MainWindow/BoxTree_Columns");
	if (Columns.isEmpty())
	{
		m_pSbieTree->OnResetColumns();
		m_pSbieTree->setColumnWidth(0, 300);
		m_pSbieTree->setColumnWidth(1, 70);
		m_pSbieTree->setColumnWidth(2, 70);
		m_pSbieTree->setColumnWidth(3, 70);
	}
	else
		m_pSbieTree->restoreState(Columns);

	//m_pMenu = new QMenu();
	AddPanelItemsToMenu();
}

CSbieView::~CSbieView()
{
	theConf->SetBlob("MainWindow/BoxTree_Columns", m_pSbieTree->saveState());
}

void CSbieView::Clear()
{
	m_Groups.clear();
	m_pSbieModel->Clear();
}

void CSbieView::Refresh()
{
	QList<QVariant> Added = m_pSbieModel->Sync(theAPI->GetAllBoxes(), m_Groups, theGUI->IsShowHidden());

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
	else if (quint32 ProcessId = ID.toUInt())
	{
		// todo proc info
	}
}

void CSbieView::OnMenu(const QPoint& Point)
{
	QList<QAction*> MenuActions = m_pMenu->actions();

	bool isConnected = theAPI->IsConnected();
	if (isConnected) {
		foreach(QAction * pAction, MenuActions) 
			pAction->setEnabled(true);
	}

	CSandBoxPtr pBox;
	CBoxedProcessPtr pProcess;
	int iProcessCount = 0;
	int iSandBoxeCount = 0;
	//int iSuspendedCount = 0;
	QModelIndexList Rows = m_pSbieTree->selectedRows();
	foreach(const QModelIndex& Index, Rows)
	{
		QModelIndex ModelIndex = m_pSortProxy->mapToSource(Index);
		
		pProcess = m_pSbieModel->GetProcess(ModelIndex);
		if (pProcess)
		{
			iProcessCount++;
			//if (pProcess->IsSuspended())
			//	iSuspendedCount++;
		}
		else
		{
			pBox = m_pSbieModel->GetSandBox(ModelIndex);
			if (pBox)
			{
				if (!pBox->IsEnabled())
					iSandBoxeCount = -1;
				else if (iSandBoxeCount != -1)
					iSandBoxeCount++;
			}
		}
	}


	for (int i = 0; i < m_iMenuTop; i++)
		MenuActions[i]->setVisible(iSandBoxeCount == 0 && iProcessCount == 0);
	m_pDelGroupe->setVisible(!Rows.isEmpty() && iSandBoxeCount == 0 && iProcessCount == 0);

	for (int i = m_iMenuTop; i < m_iMenuBox; i++)
		MenuActions[i]->setVisible(iSandBoxeCount != 0 && iProcessCount == 0);
	m_pMenuRun->setEnabled(iSandBoxeCount == 1);

	MenuActions[m_iMoveTo]->setVisible(!Rows.isEmpty() && iProcessCount == 0);

	if(iSandBoxeCount == 1)
		UpdateRunMenu(pBox);

	m_pMenuMkLink->setEnabled(iSandBoxeCount == 1);
	m_pMenuRename->setEnabled(iSandBoxeCount == 1);
	m_pMenuRecover->setEnabled(iSandBoxeCount == 1);

	m_pMenuPresets->setEnabled(iSandBoxeCount == 1);
	m_pMenuPresetsShowUAC->setChecked(pBox && !pBox->GetBool("DropAdminRights", false) && !pBox->GetBool("FakeAdminRights", false));
	m_pMenuPresetsNoAdmin->setChecked(pBox && pBox->GetBool("DropAdminRights", false) && !pBox->GetBool("FakeAdminRights", false));
	m_pMenuPresetsFakeAdmin->setChecked(pBox && pBox->GetBool("DropAdminRights", false) && pBox->GetBool("FakeAdminRights", false));
	m_pMenuPresetsINet->setChecked(pBox && pBox.objectCast<CSandBoxPlus>()->IsINetBlocked());
	m_pMenuPresetsShares->setChecked(pBox && pBox.objectCast<CSandBoxPlus>()->HasSharesAccess());

	m_pMenuExplore->setEnabled(iSandBoxeCount == 1);
	m_pMenuOptions->setEnabled(iSandBoxeCount == 1);
	m_pMenuSnapshots->setEnabled(iSandBoxeCount == 1);

	for (int i = m_iMenuBox; i < m_iMenuProc; i++)
		MenuActions[i]->setVisible(iProcessCount != 0 && iSandBoxeCount == 0);
	
	m_pMenuLinkTo->setEnabled(iProcessCount == 1);

	if (!pProcess.isNull()) {
		CSandBoxPlus* pBoxPlus = pProcess.objectCast<CSbieProcess>()->GetBox();
		QStringList RunOptions = pBoxPlus->GetTextList("RunCommand", true);

		QString FoundPin;
		QString FileName = pProcess->GetFileName();
		foreach(const QString& RunOption, RunOptions) {
			if (Split2(RunOption, "|").second.indexOf(FileName) == 0) {
				FoundPin = RunOption;
				break;
			}
		}
		if (FoundPin.isEmpty() && FileName.indexOf(pBoxPlus->GetFileRoot(), Qt::CaseInsensitive) == 0) {
			FileName.remove(0, pBoxPlus->GetFileRoot().length());
			foreach(const QString& RunOption, RunOptions) {
				if (Split2(RunOption, "|").second.indexOf(FileName) == 0) {
					FoundPin = RunOption;
					break;
				}
			}
		}

		m_pMenuPinToRun->setChecked(!FoundPin.isEmpty());
		m_pMenuPinToRun->setData(FoundPin);

		m_pMenuAllowInternet->setChecked(pProcess.objectCast<CSbieProcess>()->HasInternetAccess());

		m_pMenuMarkForced->setChecked(pProcess.objectCast<CSbieProcess>()->IsForcedProgram());

		int isLingering = pProcess.objectCast<CSbieProcess>()->IsLingeringProgram();
		m_pMenuMarkLinger->setChecked(isLingering != 0);
		m_pMenuMarkLinger->setEnabled(isLingering != 2);
		m_pMenuMarkLeader->setChecked(pProcess.objectCast<CSbieProcess>()->IsLeaderProgram());
	}

	//m_pMenuSuspend->setEnabled(iProcessCount > iSuspendedCount);
	//m_pMenuResume->setEnabled(iSuspendedCount > 0);

	if (!isConnected) {
		foreach(QAction * pAction, MenuActions)
			pAction->setEnabled(false);
	}

	CPanelView::OnMenu(Point);
}

int CSbieView__ParseGroup(const QString& Grouping, QMap<QString, QStringList>& m_Groups, const QString& Parent = "", int Index = 0)
{
	QRegExp RegExp("[,()]", Qt::CaseInsensitive, QRegExp::RegExp);
	for (; ; )
	{
		int pos = Grouping.indexOf(RegExp, Index);
		QString Name;
		if (pos == -1) {
			Name = Grouping.mid(Index);
			Index = Grouping.length();
		}
		else {
			Name = Grouping.mid(Index, pos - Index);
			Index = pos + 1;
		}
		if (!Name.isEmpty())
			m_Groups[Parent].append(Name);
		if (pos == -1)
			break;
		if (Grouping.at(pos) == "(")
		{
			m_Groups[Name] = QStringList();
			Index = CSbieView__ParseGroup(Grouping, m_Groups, Name, Index);
		}
		else if (Grouping.at(pos) == ")")
			break;
	}
	return Index;
}

void CSbieView::ReloadGroups()
{
	m_Groups.clear();

	QString Grouping = theAPI->GetUserSettings()->GetText("BoxDisplayOrder");

	CSbieView__ParseGroup(Grouping, m_Groups);

	UpdateGroupMenu();
}

void CSbieView::UpdateGroupMenu()
{
	// update move to menu
	foreach(QAction* pAction, m_pMenuMoveTo->actions())
		m_pMenuMoveTo->removeAction(pAction);
	foreach(const QString& Group, m_Groups.keys())
	{
		QAction* pAction = m_pMenuMoveTo->addAction(Group.isEmpty() ? tr("[None]") : Group, this, SLOT(OnGroupAction()));
		pAction->setData(Group);
	}
	m_pMenuMoveTo->setEnabled(m_Groups.keys().count() > 1);
}

QString CSbieView__SerializeGroup(QMap<QString, QStringList>& m_Groups, const QString& Parent = "")
{
	QStringList Grouping;
	foreach(const QString& Name, m_Groups[Parent])
	{
		if (m_Groups.contains(Name))
			Grouping.append(Name + "(" + CSbieView__SerializeGroup(m_Groups, Name) + ")");
		else
			Grouping.append(Name);
	}
	return Grouping.join(",");
}

QString CSbieView::FindParent(const QString& Name)
{
	for (auto I = m_Groups.begin(); I != m_Groups.end(); ++I)
	{
		if (I.value().contains(Name, Qt::CaseInsensitive))
			return I.key();
	}
	return QString();
}

bool CSbieView::IsParentOf(const QString& Name, const QString& Group)
{
	QString Parent = FindParent(Group);
	if (Parent == Name)
		return true;
	if (Parent.isEmpty())
		return false;
	return IsParentOf(Name, Parent);
}

void CSbieView::OnGroupAction()
{
	QAction* Action = qobject_cast<QAction*>(sender());

	if (Action == m_pNewBox)
	{
		QString Name = AddNewBox();
		if (Name.isEmpty())
			return;

		QModelIndex ModelIndex = m_pSortProxy->mapToSource(m_pSbieTree->currentIndex());
		QString Parent;
		if (m_pSbieModel->GetType(ModelIndex) == CSbieModel::eGroup)
			Parent = m_pSbieModel->GetID(ModelIndex).toString();

		m_Groups[Parent].append(Name);

		m_pSbieModel->Clear(); //todo improve that
	}
	else if (Action == m_pAddGroupe)
	{
		QString Name = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter a new group name"), QLineEdit::Normal);
		if (Name.isEmpty() || m_Groups.contains(Name))
			return;
		m_Groups[Name] = QStringList();

		QModelIndex ModelIndex = m_pSortProxy->mapToSource(m_pSbieTree->currentIndex());
		QString Parent;
		if (m_pSbieModel->GetType(ModelIndex) == CSbieModel::eGroup)
			Parent = m_pSbieModel->GetID(ModelIndex).toString();

		m_Groups[Parent].append(Name);
	}
	else if (Action == m_pDelGroupe)
	{
		if (QMessageBox("Sandboxie-Plus", tr("Do you really want to remove the selected group(s)?"), QMessageBox::Question, QMessageBox::Yes, QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton, this).exec() != QMessageBox::Yes)
			return;

		foreach(const QModelIndex& Index, m_pSbieTree->selectedRows())
		{
			QModelIndex ModelIndex = m_pSortProxy->mapToSource(Index);
			if (m_pSbieModel->GetType(ModelIndex) == CSbieModel::eGroup)
			{
				QString Group = m_pSbieModel->GetID(ModelIndex).toString();

				QStringList Items = m_Groups.take(Group); // remove groupe
		
				// remove from parents
				for (auto I = m_Groups.begin(); I != m_Groups.end(); ++I) {
					if (I.value().removeOne(Group)) {
						// move items to grand parent
						I.value().append(Items);
						break;
					}
				}
			}
		}

		m_pSbieModel->Clear(); //todo improve that, also move boxes to grant parent?
	}
	else // move to groupe
	{
		QString Group = Action->data().toString();
		
		foreach(const QModelIndex& Index, m_pSbieTree->selectedRows())
		{
			QModelIndex ModelIndex = m_pSortProxy->mapToSource(Index);
			QString Name;
			if (m_pSbieModel->GetType(ModelIndex) == CSbieModel::eGroup)
				Name = m_pSbieModel->GetID(ModelIndex).toString();
			else if (m_pSbieModel->GetType(ModelIndex) == CSbieModel::eBox)
				Name = m_pSbieModel->GetSandBox(ModelIndex)->GetName();
			
			if (Name.isEmpty())
				continue;

			if (Name == Group || IsParentOf(Name, Group)) {
				QMessageBox("Sandboxie-Plus", tr("A group can not be its own parent."), QMessageBox::Critical, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this).exec();
				continue;
			}

			// remove from old
			for (auto I = m_Groups.begin(); I != m_Groups.end(); ++I)
				I.value().removeAll(Name);

			// add to new
			m_Groups[Group].append(Name);
		}

		m_pSbieModel->Clear(); //todo improve that
	}

	StoreGroups();
	UpdateGroupMenu();
}

void CSbieView::StoreGroups()
{
	QMap<QString, CSandBoxPtr> Boxes = theAPI->GetAllBoxes();

	for (auto I = m_Groups.begin(); I != m_Groups.end(); ++I) {
		for (auto J = I->begin(); J != I->end(); ) {
			if (!Boxes.take(J->toLower()).isNull() || m_Groups.contains(*J)) ++J;
			else J = I->erase(J);
		}
	}

	QString Grouping = CSbieView__SerializeGroup(m_Groups);
	theAPI->GetUserSettings()->SetText("BoxDisplayOrder", Grouping);
}

QString CSbieView::AddNewBox()
{
	CNewBoxWindow NewBoxWindow(this);
	bool bAlwaysOnTop = theConf->GetBool("Options/AlwaysOnTop", false);
	NewBoxWindow.setWindowFlag(Qt::WindowStaysOnTopHint, bAlwaysOnTop);
	if (NewBoxWindow.exec() == 1)
	{
		theAPI->ReloadBoxes();
		Refresh();
		SelectBox(NewBoxWindow.m_Name);
		return NewBoxWindow.m_Name;
	}
	return QString();
}

void CSbieView::OnSandBoxAction()
{
	OnSandBoxAction(qobject_cast<QAction*>(sender()));
}

void CSbieView::OnSandBoxAction(QAction* Action)
{
	QList<SB_STATUS> Results;

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
	else if (Action == m_pMenuRunMenu)
		Results.append(SandBoxes.first()->RunStart("start_menu"));
	else if (Action == m_pMenuRunBrowser)
		Results.append(SandBoxes.first()->RunStart("default_browser"));
	else if (Action == m_pMenuRunMailer)
		Results.append(SandBoxes.first()->RunStart("mail_agent"));
	else if (Action == m_pMenuRunExplorer)
	{
		if (theConf->GetBool("Options/AdvancedView", true) == false && theConf->GetBool("Options/BoxedExplorerInfo", true))
		{
			bool State = false;
			CCheckableMessageBox::question(this, "Sandboxie-Plus",
				theAPI->GetSbieMsgStr(0x00000DCDL, theGUI->m_LanguageId) // MSG_3533
				, tr("Don't show this message again."), &State, QDialogButtonBox::Ok, QDialogButtonBox::Ok, QMessageBox::Information);

			if (State)
				theConf->SetValue("Options/BoxedExplorerInfo", false);
		}

		Results.append(SandBoxes.first()->RunStart("explorer.exe /e,::{20D04FE0-3AEA-1069-A2D8-08002B30309D}"));
	}
	else if (Action == m_pMenuRunCmd)
		Results.append(SandBoxes.first()->RunStart("cmd.exe"));
	else if (Action == m_pMenuRunCmdAdmin)
		Results.append(SandBoxes.first()->RunStart("cmd.exe", true));
	else if (Action == m_pMenuPresetsShowUAC)
	{
		SandBoxes.first()->SetBool("DropAdminRights", false);
		SandBoxes.first()->SetBool("FakeAdminRights", false);
	}
	else if (Action == m_pMenuPresetsNoAdmin)
	{
		SandBoxes.first()->SetBool("DropAdminRights", true);
		SandBoxes.first()->SetBool("FakeAdminRights", false);
	}
	else if (Action == m_pMenuPresetsFakeAdmin)
	{
		SandBoxes.first()->SetBool("DropAdminRights", true);
		SandBoxes.first()->SetBool("FakeAdminRights", true);
	}
	else if (Action == m_pMenuPresetsINet)
		SandBoxes.first().objectCast<CSandBoxPlus>()->SetINetBlock(m_pMenuPresetsINet->isChecked());
	else if (Action == m_pMenuPresetsShares)
		SandBoxes.first().objectCast<CSandBoxPlus>()->SetAllowShares(m_pMenuPresetsShares->isChecked());
	else if (Action == m_pMenuOptions)
	{
		OnDoubleClicked(m_pSbieTree->selectedRows().first());
	}
	else if (Action == m_pMenuExplore)
	{
		if (SandBoxes.first()->IsEmpty()) {
			QMessageBox("Sandboxie-Plus", tr("This Sandbox is empty."), QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this).exec();
			return;
		}

		if (theConf->GetBool("Options/AdvancedView", true) == false && theConf->GetBool("Options/ExplorerInfo", true))
		{
			bool State = false;
			CCheckableMessageBox::question(this, "Sandboxie-Plus",
				theAPI->GetSbieMsgStr(0x00000DCEL, theGUI->m_LanguageId) // MSG_3534
				, tr("Don't show this message again."), &State, QDialogButtonBox::Ok, QDialogButtonBox::Ok, QMessageBox::Information);

			if (State)
				theConf->SetValue("Options/ExplorerInfo", false);
		}

		::ShellExecute(NULL, NULL, SandBoxes.first()->GetFileRoot().toStdWString().c_str(), NULL, NULL, SW_SHOWNORMAL);
	}
	else if (Action == m_pMenuSnapshots)
	{
		CSandBoxPtr pBox = SandBoxes.first();

		static QMap<void*, CSnapshotsWindow*> SnapshotWindows;
		if (!SnapshotWindows.contains(pBox.data()))
		{
			CSnapshotsWindow* pSnapshotsWindow = new CSnapshotsWindow(SandBoxes.first(), this);
			SnapshotWindows.insert(pBox.data(), pSnapshotsWindow);
			connect(pSnapshotsWindow, &CSnapshotsWindow::Closed, [this, pBox]() {
				SnapshotWindows.remove(pBox.data());
			});
			pSnapshotsWindow->show();
		}
	}
	else if (Action == m_pMenuRename)
	{
		QString OldName = SandBoxes.first()->GetName();
		QString OldValue = OldName.replace("_", " ");
		QString Value = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter a new name for the Sandbox."), QLineEdit::Normal, OldValue);
		if (Value.isEmpty() || Value == OldValue)
			return;
		QString NewName = Value.replace(" ", "_");
		Results.append((SandBoxes.first()->RenameBox(NewName)));

		// update grouping
		for (auto I = m_Groups.begin(); I != m_Groups.end(); ++I) {
			int pos = I->indexOf(OldName);
			if (pos != -1)
				I->replace(pos, NewName);
		}

		StoreGroups();
	}
	else if (Action == m_pMenuRemove)
	{
		if (QMessageBox("Sandboxie-Plus", tr("Do you really want to remove the selected sandbox(es)?"), QMessageBox::Warning, QMessageBox::Yes, QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton, this).exec() != QMessageBox::Yes)
			return;

		foreach(const CSandBoxPtr & pBox, SandBoxes)
		{
			QString Name = pBox->GetName();
			SB_STATUS Status = pBox->RemoveBox();

			// update grouping
			if (!Status.IsError()) {
				for (auto I = m_Groups.begin(); I != m_Groups.end(); ++I) {
					I->removeAll(Name);
				}
			}

			Results.append(Status);
		}

		StoreGroups();
	}
	else if (Action == m_pMenuRecover)
	{
		emit RecoveryRequested(SandBoxes.first()->GetName());
	}
	else if (Action == m_pMenuCleanUp)
	{
		if (SandBoxes.count() == 1)
		{
			if (SandBoxes.first()->IsEmpty()) {
				QMessageBox("Sandboxie-Plus", tr("This Sandbox is already empty."), QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this).exec();
				return;
			}

			if (theConf->GetBool("Options/ShowRecovery", false))
			{
				CRecoveryWindow* pRecoveryWindow = new CRecoveryWindow(SandBoxes.first());
				pRecoveryWindow->FindFiles();
				if (pRecoveryWindow->exec() != 1)
					return;	
			}
			else if(QMessageBox("Sandboxie-Plus", tr("Do you want to delete the content of the selected sandbox?"), QMessageBox::Question, QMessageBox::Yes, QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton, this).exec() != QMessageBox::Yes)
				return;
		}
		else if (QMessageBox("Sandboxie-Plus", tr("Do you really want to delete the content of multiple sandboxes?"), QMessageBox::Warning, QMessageBox::Yes, QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton, this).exec() != QMessageBox::Yes)
			return;

		foreach(const CSandBoxPtr & pBox, SandBoxes)
		{
			SB_PROGRESS Status = pBox->CleanBox();
			if (Status.GetStatus() == OP_ASYNC)
				theGUI->AddAsyncOp(Status.GetValue());
			else if (Status.IsError())
				Results.append(Status);
		}	
	}
	else if (Action == m_pMenuEmptyBox)
	{
		if (QMessageBox("Sandboxie-Plus", tr("Do you want to terminate all processes in the selected sandbox(es)?"), QMessageBox::Question, QMessageBox::Yes, QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton, this).exec() != QMessageBox::Yes)
			return;

		foreach(const CSandBoxPtr& pBox, SandBoxes)
			Results.append(pBox->TerminateAll());
	}
	else if (Action == m_pMenuMkLink)
	{
		QString BoxName = SandBoxes.first()->GetName();
		QString LinkPath, IconPath, WorkDir;
		quint32 IconIndex;
		if (!CSbieUtils::GetStartMenuShortcut(theAPI, BoxName, LinkPath, IconPath, IconIndex, WorkDir))
			return;
		
		QString LinkName;
		int pos = LinkPath.lastIndexOf(L'\\');
		if (pos == -1)
			return;
		if (pos == 2 && LinkPath.length() == 3)
			LinkName = QObject::tr("Drive %1").arg(LinkPath.left(1));
		else {
			LinkName = LinkPath.mid(pos + 1);
			pos = LinkName.indexOf(QRegExp("[" + QRegExp::escape("\":;,*?.") + "]"));
			if (pos != -1)
				LinkName = LinkName.left(pos);
		}

		QString Path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation).replace("/", "\\");
		//Path = QFileDialog::getExistingDirectory(this, tr("Select Directory to create Shorcut in"), Path).replace("/", "\\");
		//if (Path.isEmpty())
		//	return;

		if (Path.right(1) != "\\")
			Path.append("\\");
		Path += "[" + BoxName + "] " + LinkName;

		Path = QFileDialog::getSaveFileName(this, tr("Create Shortcut to sandbox %1").arg(BoxName), Path, QString("Shortcut files (*.lnk)")).replace("/", "\\");
		if (Path.isEmpty())
			return;

		CSbieUtils::CreateShortcut(theAPI, Path, LinkName, BoxName, LinkPath, IconPath, IconIndex, WorkDir);
	}
	else // custom run menu command
	{
		QString Command = Action->data().toString();
		if (!Command.isEmpty())
		{
			if (Command.left(1) == "\\" && !SandBoxes.isEmpty())
				Command.prepend(SandBoxes.first()->GetFileRoot());
			Results.append(SandBoxes.first()->RunStart(Command));
		}
	}

	CSandMan::CheckResults(Results);
}

void CSbieView::OnProcessAction()
{
	QList<SB_STATUS> Results;

	QAction* Action = qobject_cast<QAction*>(sender());
	if (Action == m_pMenuTerminate || Action == m_pMenuBlackList)
	{
		if (QMessageBox("Sandboxie-Plus", tr("Do you want to %1 the selected process(es)?").arg(((QAction*)sender())->text().toLower())
			, QMessageBox::Question, QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape, QMessageBox::NoButton, this).exec() != QMessageBox::Yes)
			return;
	}

	foreach(const CBoxedProcessPtr& pProcess, CSbieView::GetSelectedProcesses())
	{
		if (Action == m_pMenuTerminate)
			Results.append(pProcess->Terminate());
		else if (Action == m_pMenuLinkTo)
		{
			QString BoxName = pProcess->GetBoxName();
			QString LinkName = pProcess->GetProcessName();
			QString LinkPath = pProcess->GetFileName();

			QString Path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation).replace("/", "\\");
			//Path = QFileDialog::getExistingDirectory(this, tr("Select Directory to create Shorcut in"), Path).replace("/", "\\");
			//if (Path.isEmpty())
			//	return;

			if (Path.right(1) != "\\")
				Path.append("\\");
			Path += "[" + BoxName + "] " + LinkName;

			Path = QFileDialog::getSaveFileName(this, tr("Create Shortcut to sandbox %1").arg(BoxName), Path, QString("Shortcut files (*.lnk)")).replace("/", "\\");
			if (Path.isEmpty())
				return;

			CSbieUtils::CreateShortcut(theAPI, Path, LinkName, BoxName, LinkPath, LinkPath);
		}
		else if (Action == m_pMenuPinToRun)
		{
			CSandBoxPlus* pBoxPlus = pProcess.objectCast<CSbieProcess>()->GetBox();

			if (m_pMenuPinToRun->isChecked())
			{
				QString FileName = pProcess->GetFileName();
				if (FileName.indexOf(pBoxPlus->GetFileRoot(), Qt::CaseInsensitive) == 0) {
					FileName.remove(0, pBoxPlus->GetFileRoot().length());
					if (FileName.at(0) != "\\")
						FileName.prepend("\\");
				}

				pBoxPlus->InsertText("RunCommand", pProcess->GetProcessName() + "|" + pProcess->GetFileName());
			}
			else if(!m_pMenuPinToRun->data().toString().isEmpty())
				pBoxPlus->DelValue("RunCommand", m_pMenuPinToRun->data().toString());
		}
		else if (Action == m_pMenuBlackList)
		{
			Results.append(pProcess->Terminate());
			pProcess.objectCast<CSbieProcess>()->BlockProgram();
		}
		else if (Action == m_pMenuAllowInternet)
		{
			if (!pProcess.objectCast<CSbieProcess>()->GetBox()->IsINetBlocked())
			{
				if (QMessageBox("Sandboxie-Plus", tr("This box does not have Internet restrictions in place, do you want to enable them?"), QMessageBox::Question, QMessageBox::Yes, QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton, this).exec() != QMessageBox::Yes)
					return;
				pProcess.objectCast<CSbieProcess>()->GetBox()->SetINetBlock(true);
			}
			pProcess.objectCast<CSbieProcess>()->SetInternetAccess(m_pMenuAllowInternet->isChecked());
		}
		else if (Action == m_pMenuMarkForced)
			pProcess.objectCast<CSbieProcess>()->SetForcedProgram(m_pMenuMarkForced->isChecked());
		else if (Action == m_pMenuMarkLinger)
			pProcess.objectCast<CSbieProcess>()->SetLingeringProgram(m_pMenuMarkLinger->isChecked());
		else if (Action == m_pMenuMarkLeader)
			pProcess.objectCast<CSbieProcess>()->SetLeaderProgram(m_pMenuMarkLeader->isChecked());
		/*else if (Action == m_pMenuSuspend)
			Results.append(pProcess->SetSuspend(true));
		else if (Action == m_pMenuResume)
			Results.append(pProcess->SetSuspend(false));*/
	}

	CSandMan::CheckResults(Results);
}

void CSbieView::OnDoubleClicked(const QModelIndex& index)
{
	QModelIndex ModelIndex = m_pSortProxy->mapToSource(index);
	CSandBoxPtr pBox = m_pSbieModel->GetSandBox(ModelIndex);
	if (pBox.isNull())
		return;

	if (!pBox->IsEnabled())
	{
		if (QMessageBox("Sandboxie-Plus", tr("This sandbox is disabled, do you want to enable it?"), QMessageBox::Question, QMessageBox::Yes, QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton, this).exec() != QMessageBox::Yes)
			return;
		pBox->SetText("Enabled", "y");
	}

	static QMap<void*, COptionsWindow*> OptionsWindows;
	if (!OptionsWindows.contains(pBox.data()))
	{
		COptionsWindow* pOptionsWindow = new COptionsWindow(pBox, pBox->GetName());
		OptionsWindows.insert(pBox.data(), pOptionsWindow);
		connect(pOptionsWindow, &COptionsWindow::Closed, [this, pBox]() {
			OptionsWindows.remove(pBox.data());
		});
		pOptionsWindow->show();
	}
}

void CSbieView::ProcessSelection(const QItemSelection& selected, const QItemSelection& deselected)
{
	if (selected.empty())
		return;

	QItemSelectionModel* selectionModel = m_pSbieTree->selectionModel();
	QItemSelection selection = selectionModel->selection();
	QItemSelection invalid;

	/*
	QModelIndex root_parent = m_pSbieTree->currentIndex().parent();
	while (root_parent.isValid() && root_parent.parent().isValid())
		root_parent = root_parent.parent();

	foreach(const QModelIndex& index, selection.indexes())
	{
		QModelIndex parent = index.parent();
		while (parent.isValid() && parent.parent().isValid())
			parent = parent.parent();

		if (parent != root_parent)
			invalid.select(index, index);
	}*/

	int Type = m_pSbieModel->GetType(m_pSortProxy->mapToSource(m_pSbieTree->currentIndex()));

	foreach(const QModelIndex& index, selection.indexes())
	{
		if (m_pSbieModel->GetType(m_pSortProxy->mapToSource(index)) != Type)
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

/*void CSbieView::UpdateRunMenu()
{
	while (m_iMenuRun < m_pMenuRun->actions().count())
		m_pMenuRun->removeAction(m_pMenuRun->actions().at(m_iMenuRun));

	QStringList RunOptions = theConf->ListKeys("RunOptions");
	foreach(const QString& RunOption, RunOptions)
	{
		StrPair NameCmd = Split2(theConf->GetString("RunOptions/" + RunOption), "|");

		QAction* pAction = m_pMenuRun->addAction(NameCmd.first, this, SLOT(OnSandBoxAction()));
		pAction->setData(NameCmd.second);
	}
}*/

void CSbieView::UpdateRunMenu(const CSandBoxPtr& pBox)
{
	while (m_iMenuRun < m_pMenuRun->actions().count())
		m_pMenuRun->removeAction(m_pMenuRun->actions().at(m_iMenuRun));

	QFileIconProvider IconProvider;

	QStringList RunOptions = pBox->GetTextList("RunCommand", true);
	foreach(const QString& RunOption, RunOptions) 
	{
		StrPair NameCmd = Split2(RunOption, "|");
		QAction* pAction = m_pMenuRun->addAction(NameCmd.first, this, SLOT(OnSandBoxAction()));

		QString Path = NameCmd.second;
		if (Path.left(1) == "\"") {
			int End = Path.indexOf("\"", 1);
			if (End != -1) Path = Path.mid(1, End - 1);
		}
		else {
			int End = Path.indexOf(" ");
			if (End != -1) Path.truncate(End);
		}

		if (Path.left(1) == "\\")
			Path.prepend(pBox->GetFileRoot());

		pAction->setIcon(IconProvider.icon(QFileInfo(Path)));

		pAction->setData(NameCmd.second);
	}
}

void CSbieView::SelectBox(const QString& Name)
{
	QModelIndex Index = m_pSbieModel->FindIndex(Name);
	QModelIndex ModelIndex = m_pSortProxy->mapFromSource(Index);

	QModelIndex ModelL = m_pSortProxy->index(ModelIndex.row(), 0, ModelIndex.parent());
	QModelIndex ModelR = m_pSortProxy->index(ModelIndex.row(), m_pSortProxy->columnCount() - 1, ModelIndex.parent());

	QItemSelection SelectedItems;
	SelectedItems.append(QItemSelectionRange(ModelL, ModelR));

	m_pSbieTree->scrollTo(ModelL);
	m_pSbieTree->selectionModel()->select(SelectedItems, QItemSelectionModel::ClearAndSelect);
}