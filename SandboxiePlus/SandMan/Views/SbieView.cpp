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
#include "../../MiscHelpers/Common/CheckableMessageBox.h"
#include "../Windows/RecoveryWindow.h"
#include "../Views/FileView.h"
#include "../Wizards/NewBoxWizard.h"
#include "../Helpers/WinHelper.h"
#include "../Windows/BoxImageWindow.h"
#include "../MiscHelpers/Archive/Archive.h"
#include "../Windows/SettingsWindow.h"
#include "../Windows/CompressDialog.h"
#include "../Windows/ExtractDialog.h"

#include "qt_windows.h"
#include "qwindowdefs_win.h"
#include <shellapi.h>

CSbieView::CSbieView(QWidget* parent) : CPanelView(parent)
{
	m_pMainLayout = new QVBoxLayout();
	m_pMainLayout->setContentsMargins(0,0,0,0);
	this->setLayout(m_pMainLayout);

	m_HoldExpand = false;

	m_pSbieModel = new CSbieModel(this);
	m_pSbieModel->SetTree(true);
	m_pSbieModel->SetUseIcons(true);

	m_pSortProxy = new CSortFilterProxyModel(this);
	m_pSortProxy->setSortRole(Qt::EditRole);
	m_pSortProxy->setSourceModel(m_pSbieModel);
	m_pSortProxy->setDynamicSortFilter(true);

	// SbieTree
	m_pSbieTree = new QTreeViewEx();
	m_pSbieTree->setColumnFixed(0, true);
	m_pSbieTree->setAlternatingRowColors(theConf->GetBool("Options/AltRowColors", false));
	m_pSbieTree->setExpandsOnDoubleClick(false);
	//m_pSbieTree->setItemDelegate(theGUI->GetItemDelegate());

	m_pSbieTree->setModel(m_pSortProxy);

	int iViewMode = theConf->GetInt("Options/ViewMode", 1);
	int iLargeIcons = theConf->GetInt("Options/LargeIcons", 2);
	if (iLargeIcons == 2)
		iLargeIcons = iViewMode == 2 ? 1 : 0;
	if (iLargeIcons) {
		m_pSbieModel->SetLargeIcons();
		m_pSbieTree->setIconSize(QSize(32, 32));
	}

	m_pSbieTree->setDragDropMode(QAbstractItemView::InternalMove);

	m_pSbieTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_pSbieTree->setSortingEnabled(true);
	//m_pSbieTree->setSortingEnabled(false);
	//m_pSbieTree->header()->setSortIndicatorShown(true);
	//m_pSbieTree->header()->setSectionsClickable(true);
	if(iViewMode != 2)
		connect(m_pSbieTree->header(), SIGNAL(sectionClicked(int)), this, SLOT(OnCustomSortByColumn(int)));

	QStyle* pStyle = QStyleFactory::create("windows");
	m_pSbieTree->setStyle(pStyle);
	m_pSbieTree->setItemDelegate(iLargeIcons ? new CTreeItemDelegate2() : new CTreeItemDelegate());

	m_pSbieTree->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pSbieTree, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnMenu(const QPoint &)));
	connect(m_pSbieTree, SIGNAL(pressed(const QModelIndex&)), this, SLOT(OnClicked(const QModelIndex&)));
	connect(m_pSbieTree, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(OnDoubleClicked(const QModelIndex&)));
	connect(m_pSbieTree->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), SLOT(ProcessSelection(QItemSelection, QItemSelection)));
	connect(m_pSbieTree, SIGNAL(expanded(const QModelIndex &)), this, SLOT(OnExpanded(const QModelIndex &)));
	connect(m_pSbieTree, SIGNAL(collapsed(const QModelIndex &)), this, SLOT(OnCollapsed(const QModelIndex &)));

	//connect(theGUI, SIGNAL(ReloadPanels()), m_pSbieModel, SLOT(Clear()));

	connect(m_pSbieModel, SIGNAL(MoveBox(const QString&, const QString&, int)), this, SLOT(OnMoveItem(const QString&, const QString&, int)));
	connect(m_pSbieModel, SIGNAL(MoveGroup(const QString&, const QString&, int)), this, SLOT(OnMoveItem(const QString&, const QString&, int)));

	m_pMainLayout->addWidget(m_pSbieTree);
	// 

	CFinder* pFinder = new CFinder(m_pSortProxy, this);
	m_pMainLayout->addWidget(pFinder);
	pFinder->SetTree(m_pSbieTree);
	QObject::connect(pFinder, SIGNAL(SetFilter(const QRegularExpression&, int, int)), this, SLOT(UpdateColapsed()));


	connect(m_pSbieModel, SIGNAL(ToolTipCallback(const QVariant&, QString&)), this, SLOT(OnToolTipCallback(const QVariant&, QString&)), Qt::DirectConnection);

	if(iViewMode == 2)
		CreateOldMenu();
	else
		CreateMenu();

	m_pCtxMenu = new QMenu();
	m_pCtxPinToRun = m_pCtxMenu->addAction(tr("Pin to Run Menu"), this, SLOT(OnMenuContextAction()));
	m_pCtxPinToRun->setCheckable(true);
	m_pCtxMkLink = m_pCtxMenu->addAction(CSandMan::GetIcon("MkLink"), tr("Create Shortcut"), this, SLOT(OnMenuContextAction()));

	m_pRemove = new QAction(this);
	m_pRemove->setShortcut(QKeySequence::Delete);
	m_pRemove->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	this->addAction(m_pRemove);
	connect(m_pRemove, SIGNAL(triggered()), this, SLOT(OnRemoveItem()));

	CreateGroupMenu();
	CreateTrayMenu();

	QByteArray Columns = theConf->GetBlob("MainWindow/BoxTree_Columns");
	if (Columns.isEmpty()) {
		m_pSbieTree->setColumnWidth(0, 300);
		m_pSbieTree->setColumnWidth(1, 70);
		m_pSbieTree->setColumnWidth(2, 70);
		m_pSbieTree->setColumnWidth(3, 70);
	} else
		m_pSbieTree->restoreState(Columns);
	if (theConf->GetBool("MainWindow/BoxTree_UseOrder", false) || iViewMode == 2)
		SetCustomOrder();

	//m_pMenu = new QMenu();
	AddPanelItemsToMenu();

	AddCopyMenu(m_pMenuBox);
	AddCopyMenu(m_pMenuProcess);
	AddCopyMenu(m_pMenuGroup);
}

CSbieView::~CSbieView()
{
	SaveState();
}

void CSbieView::SaveState()
{
	theConf->SetBlob("MainWindow/BoxTree_Columns", m_pSbieTree->saveState());
	//theConf->SetValue("MainWindow/BoxTree_UseOrder", m_pSortProxy->sortRole() == Qt::InitialSortOrderRole);
}

void CSbieView::Clear()
{
	m_Groups.clear();
	m_pSbieModel->Clear();
}

void CSbieView::CreateMenu()
{
	m_pNewBox = m_pMenu->addAction(CSandMan::GetIcon("NewBox"), tr("Create New Box"), this, SLOT(OnGroupAction()));
	m_pAddGroupe = m_pMenu->addAction(CSandMan::GetIcon("Group"), tr("Create Box Group"), this, SLOT(OnGroupAction()));
	m_pImportBox = m_pMenu->addAction(CSandMan::GetIcon("UnPackBox"), tr("Import Box"), this, SLOT(OnGroupAction()));
	m_pImportBox->setEnabled(CArchive::IsInit());


	m_pMenuBox = new QMenu();
	m_pStopAsync = m_pMenuBox->addAction(CSandMan::GetIcon("Stop"), tr("Stop Operations"), this, SLOT(OnSandBoxAction()));
	//m_pMenuBox->addSeparator();

	//m_pMenuRun = m_pMenuBox->addMenu(CSandMan::GetIcon("Start"), tr("Run"));
	m_pMenuRun = new CMenuEx(tr("Run"), m_pMenuBox);
	m_pMenuRun->setIcon(CSandMan::GetIcon("Start"));
	m_pMenuBox->addAction(m_pMenuRun->menuAction());
	connect(m_pMenuRun, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnMenuContextMenu(const QPoint&)));

		m_pMenuRunAny = m_pMenuRun->addAction(CSandMan::GetIcon("Run"), tr("Run Program"), this, SLOT(OnSandBoxAction()));
		m_pMenuRunMenu = m_pMenuRun->addAction(CSandMan::GetIcon("StartMenu"), tr("Run from Start Menu"), this, SLOT(OnSandBoxAction()));
		if (theConf->GetBool("Options/ScanStartMenu", true)) {
			m_pMenuRunStart = new QMenu();
			m_pMenuRunStart->addAction(CSandMan::GetIcon("StartMenu"), tr("(Host) Start Menu"), this, SLOT(OnSandBoxAction()));
			m_pMenuRunStart->addSeparator();
		}
		else
			m_pMenuRunStart = NULL;
		//m_pMenuRunTools->addSeparator();
		m_pMenuAutoRun = m_pMenuRun->addAction(CSandMan::GetIcon("ReloadIni"), tr("Execute Autorun Entries"), this, SLOT(OnSandBoxAction()));
		m_pMenuRunTools = m_pMenuRun->addMenu(CSandMan::GetIcon("Maintenance"), tr("Standard Applications"));
			m_pMenuRunBrowser = m_pMenuRunTools->addAction(CSandMan::GetIcon("Internet"), tr("Default Web Browser"), this, SLOT(OnSandBoxAction()));
			m_pMenuRunMailer = m_pMenuRunTools->addAction(CSandMan::GetIcon("Email"), tr("Default eMail Client"), this, SLOT(OnSandBoxAction()));

			m_pMenuRunExplorer = m_pMenuRunTools->addAction(CSandMan::GetIcon("Explore"), tr("Windows Explorer"), this, SLOT(OnSandBoxAction()));
			m_pMenuRunRegEdit = m_pMenuRunTools->addAction(CSandMan::GetIcon("RegEdit"), tr("Registry Editor"), this, SLOT(OnSandBoxAction()));
			m_pMenuRunAppWiz = m_pMenuRunTools->addAction(CSandMan::GetIcon("Software"), tr("Programs and Features"), this, SLOT(OnSandBoxAction()));
			
			m_pMenuRunTools->addSeparator();
			m_pMenuRunCmd = m_pMenuRunTools->addAction(CSandMan::GetIcon("Cmd"), tr("Command Prompt"), this, SLOT(OnSandBoxAction()));
			m_pMenuRunCmdAdmin = m_pMenuRunTools->addAction(CSandMan::GetIcon("Cmd"), tr("Command Prompt (as Admin)"), this, SLOT(OnSandBoxAction()));
#ifndef _WIN64
			if(CSbieAPI::IsWow64())
#endif
				m_pMenuRunCmd32 = m_pMenuRunTools->addAction(CSandMan::GetIcon("Cmd"), tr("Command Prompt (32-bit)"), this, SLOT(OnSandBoxAction()));
		m_pMenuRun->addSeparator();
		m_iMenuRun = m_pMenuRun->actions().count();
	m_pMenuEmptyBox = m_pMenuBox->addAction(CSandMan::GetIcon("EmptyAll"), tr("Terminate All Programs"), this, SLOT(OnSandBoxAction()));
	m_pMenuBox->addSeparator();
	m_pMenuMount = m_pMenuBox->addAction(CSandMan::GetIcon("LockOpen"), tr("Mount Box Image"), this, SLOT(OnSandBoxAction()));
	m_pMenuUnmount = m_pMenuBox->addAction(CSandMan::GetIcon("LockClosed"), tr("Unmount Box Image"), this, SLOT(OnSandBoxAction()));
	m_pMenuRecover = m_pMenuBox->addAction(CSandMan::GetIcon("Recover"), tr("Recover Files"), this, SLOT(OnSandBoxAction()));
	m_pMenuCleanUp = m_pMenuBox->addAction(CSandMan::GetIcon("Erase"), tr("Delete Content"), this, SLOT(OnSandBoxAction()));
	m_pMenuContent = m_pMenuBox->addMenu(CSandMan::GetIcon("Compatibility"), tr("Box Content"));
		m_pMenuBrowse = m_pMenuContent->addAction(CSandMan::GetIcon("Folder"), tr("Browse Files"), this, SLOT(OnSandBoxAction()));
		m_pMenuContent->addSeparator();
		m_pMenuRefresh = m_pMenuContent->addAction(CSandMan::GetIcon("Refresh"), tr("Refresh Info"), this, SLOT(OnSandBoxAction()));
		m_pMenuMkLink = m_pMenuContent->addAction(CSandMan::GetIcon("MkLink"), tr("Create Shortcut"), this, SLOT(OnSandBoxAction()));
		m_pMenuContent->addSeparator();
		m_pMenuExplore = m_pMenuContent->addAction(CSandMan::GetIcon("Explore"), tr("Explore Content"), this, SLOT(OnSandBoxAction()));
		m_pMenuRegEdit = m_pMenuContent->addAction(CSandMan::GetIcon("RegEdit"), tr("Open Registry"), this, SLOT(OnSandBoxAction()));
	m_pMenuSnapshots = m_pMenuBox->addAction(CSandMan::GetIcon("Snapshots"), tr("Snapshots Manager"), this, SLOT(OnSandBoxAction()));
	m_pMenuBox->addSeparator();
	m_pMenuOptions = m_pMenuBox->addAction(CSandMan::GetIcon("Options"), tr("Sandbox Options"), this, SLOT(OnSandBoxAction()));
	QFont f = m_pMenuOptions->font();
	f.setBold(true);
	m_pMenuOptions->setFont(f);

	m_pMenuPresets = m_pMenuBox->addMenu(CSandMan::GetIcon("Presets"), tr("Sandbox Presets"));
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

		m_pMenuPresets->addSeparator();
		m_pMenuPresetsRecovery = m_pMenuPresets->addAction(tr("Immediate Recovery"), this, SLOT(OnSandBoxAction()));
		m_pMenuPresetsRecovery->setCheckable(true);
		m_pMenuPresetsForce = m_pMenuPresets->addAction(tr("Disable Force Rules"), this, SLOT(OnSandBoxAction()));
		m_pMenuPresetsForce->setCheckable(true);
	
	m_pMenuTools = m_pMenuBox->addMenu(CSandMan::GetIcon("Maintenance"), tr("Sandbox Tools"));
		m_pMenuDuplicate = m_pMenuTools->addAction(CSandMan::GetIcon("Duplicate"), tr("Duplicate Box Config"), this, SLOT(OnSandBoxAction()));
		m_pMenuDuplicateEx = m_pMenuTools->addAction(CSandMan::GetIcon("Duplicate"), tr("Duplicate Box with Content"), this, SLOT(OnSandBoxAction()));
		m_pMenuExport = m_pMenuTools->addAction(CSandMan::GetIcon("PackBox"), tr("Export Box"), this, SLOT(OnSandBoxAction()));
		m_pMenuExport->setEnabled(CArchive::IsInit());

	m_pMenuRename = m_pMenuBox->addAction(CSandMan::GetIcon("Rename"), tr("Rename Sandbox"), this, SLOT(OnSandBoxAction()));
	m_pMenuMoveTo = m_pMenuBox->addMenu(CSandMan::GetIcon("Group"), tr("Move Sandbox"));
		m_pMenuMoveUp = m_pMenuMoveTo->addAction(CSandMan::GetIcon("Up"), tr("Move Up"), this, SLOT(OnGroupAction()));
		m_pMenuMoveUp->setShortcut(QKeySequence("Alt+Up"));
		m_pMenuMoveUp->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		this->addAction(m_pMenuMoveUp);
		//m_pMenuMoveBy = m_pMenuMoveTo->addAction(tr("Move to Position"), this, SLOT(OnGroupAction())); // does not seam that intuitive for users
		m_pMenuMoveDown = m_pMenuMoveTo->addAction(CSandMan::GetIcon("Down"), tr("Move Down"), this, SLOT(OnGroupAction()));
		m_pMenuMoveDown->setShortcut(QKeySequence("Alt+Down"));
		m_pMenuMoveDown->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		this->addAction(m_pMenuMoveDown);
		m_pMenuMoveTo->addSeparator();
	m_pMenuRemove = m_pMenuBox->addAction(CSandMan::GetIcon("Remove"), tr("Remove Sandbox"), this, SLOT(OnSandBoxAction()));


	// Process Menu
	m_pMenuProcess = new QMenu();
	m_pMenuTerminate = m_pMenuProcess->addAction(CSandMan::GetIcon("Remove"), tr("Terminate"), this, SLOT(OnProcessAction()));
	this->addAction(m_pMenuTerminate);
	m_pMenuLinkTo = m_pMenuProcess->addAction(CSandMan::GetIcon("MkLink"), tr("Create Shortcut"), this, SLOT(OnProcessAction()));
	m_pMenuPreset = m_pMenuProcess->addMenu(CSandMan::GetIcon("Presets"), tr("Preset"));
		m_pMenuPinToRun = m_pMenuPreset->addAction(tr("Pin to Run Menu"), this, SLOT(OnProcessAction()));
		m_pMenuPinToRun->setCheckable(true);
		m_pMenuBlackList = m_pMenuPreset->addAction(tr("Block and Terminate"), this, SLOT(OnProcessAction()));
		//m_pMenuBlackList->setShortcut(QKeySequence("Shift+Del"));
		//m_pMenuBlackList->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		this->addAction(m_pMenuBlackList);
		m_pMenuAllowInternet = m_pMenuPreset->addAction(tr("Allow internet access"), this, SLOT(OnProcessAction()));
		m_pMenuAllowInternet->setCheckable(true);
		m_pMenuMarkForced = m_pMenuPreset->addAction(tr("Force into this sandbox"), this, SLOT(OnProcessAction()));
		m_pMenuMarkForced->setCheckable(true);
		m_pMenuMarkLinger = m_pMenuPreset->addAction(tr("Set Linger Process"), this, SLOT(OnProcessAction()));
		m_pMenuMarkLinger->setCheckable(true);
		m_pMenuMarkLeader = m_pMenuPreset->addAction(tr("Set Leader Process"), this, SLOT(OnProcessAction()));
		m_pMenuMarkLeader->setCheckable(true);
	m_pMenuSuspend = m_pMenuProcess->addAction(tr("Suspend"), this, SLOT(OnProcessAction()));
	m_pMenuResume = m_pMenuProcess->addAction(tr("Resume"), this, SLOT(OnProcessAction()));
}

void CSbieView::CreateOldMenu()
{
	m_pNewBox = m_pMenu->addAction(CSandMan::GetIcon("NewBox"), tr("Create New Box"), this, SLOT(OnGroupAction()));
	m_pAddGroupe = m_pMenu->addAction(CSandMan::GetIcon("Group"), tr("Create Box Group"), this, SLOT(OnGroupAction()));
	m_pImportBox = m_pMenu->addAction(CSandMan::GetIcon("UnPackBox"), tr("Import Box"), this, SLOT(OnGroupAction()));
	m_pImportBox->setEnabled(CArchive::IsInit());
	

	m_pMenuBox = new QMenu();
	m_pStopAsync = m_pMenuBox->addAction(CSandMan::GetIcon("Stop"), tr("Stop Operations"), this, SLOT(OnSandBoxAction()));
	//m_pMenuBox->addSeparator();

	//m_pMenuRun = m_pMenuBox->addMenu(CSandMan::GetIcon("Start"), tr("Run Sandboxed"));
	m_pMenuRun = new CMenuEx(tr("Run"), m_pMenuBox);
	m_pMenuRun->setIcon(CSandMan::GetIcon("Start"));
	m_pMenuBox->addAction(m_pMenuRun->menuAction());
	connect(m_pMenuRun, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnMenuContextMenu(const QPoint&)));

		m_pMenuRunBrowser = m_pMenuRun->addAction(CSandMan::GetIcon("Internet"), tr("Run Web Browser"), this, SLOT(OnSandBoxAction()));
		m_pMenuRunMailer = m_pMenuRun->addAction(CSandMan::GetIcon("Email"), tr("Run eMail Reader"), this, SLOT(OnSandBoxAction()));
		m_pMenuRunAny = m_pMenuRun->addAction(CSandMan::GetIcon("Run"), tr("Run Any Program"), this, SLOT(OnSandBoxAction()));
		m_pMenuRunMenu = m_pMenuRun->addAction(CSandMan::GetIcon("StartMenu"), tr("Run From Start Menu"), this, SLOT(OnSandBoxAction()));
		if (theConf->GetBool("Options/ScanStartMenu", true)) {
			m_pMenuRunStart = new QMenu();
			m_pMenuRunStart->addAction(CSandMan::GetIcon("StartMenu"), tr("(Host) Start Menu"), this, SLOT(OnSandBoxAction()));
			m_pMenuRunStart->addSeparator();
		}
		else
			m_pMenuRunStart = NULL;
		m_pMenuRunExplorer = m_pMenuRun->addAction(CSandMan::GetIcon("Explore"), tr("Run Windows Explorer"), this, SLOT(OnSandBoxAction()));
		m_pMenuRunCmd = NULL;
		m_pMenuRunTools = NULL;
			m_pMenuRunCmdAdmin = NULL;
#ifdef _WIN64
			m_pMenuRunCmd32 = NULL;
#endif
			m_pMenuRunRegEdit = NULL;
			m_pMenuRunAppWiz = NULL;
			m_pMenuAutoRun = NULL;
		m_pMenuRun->addSeparator();
		m_iMenuRun = m_pMenuRun->actions().count();

	m_pMenuBox->addSeparator();
	m_pMenuEmptyBox = m_pMenuBox->addAction(CSandMan::GetIcon("EmptyAll"), tr("Terminate Programs"), this, SLOT(OnSandBoxAction()));
	m_pMenuMount = m_pMenuBox->addAction(CSandMan::GetIcon("LockOpen"), tr("Mount Box Image"), this, SLOT(OnSandBoxAction()));
	m_pMenuUnmount = m_pMenuBox->addAction(CSandMan::GetIcon("LockClosed"), tr("Unmount Box Image"), this, SLOT(OnSandBoxAction()));
	m_pMenuRecover = m_pMenuBox->addAction(CSandMan::GetIcon("Recover"), tr("Quick Recover"), this, SLOT(OnSandBoxAction()));
	m_pMenuCleanUp = m_pMenuBox->addAction(CSandMan::GetIcon("Erase"), tr("Delete Content"), this, SLOT(OnSandBoxAction()));
	m_pMenuExplore = m_pMenuBox->addAction(CSandMan::GetIcon("Explore"), tr("Explore Content"), this, SLOT(OnSandBoxAction()));

	m_pMenuBox->addSeparator();
	m_pMenuOptions = m_pMenuBox->addAction(CSandMan::GetIcon("Options"), tr("Sandbox Settings"), this, SLOT(OnSandBoxAction()));

	m_pMenuTools = m_pMenuBox->addMenu(CSandMan::GetIcon("Maintenance"), tr("Sandbox Tools"));
		m_pMenuBrowse = m_pMenuTools->addAction(CSandMan::GetIcon("Tree"), tr("Browse Content"), this, SLOT(OnSandBoxAction()));
		m_pMenuSnapshots = m_pMenuTools->addAction(CSandMan::GetIcon("Snapshots"), tr("Snapshots Manager"), this, SLOT(OnSandBoxAction()));

		m_pMenuTools->addSeparator();
		m_pMenuDuplicate = m_pMenuTools->addAction(CSandMan::GetIcon("Duplicate"), tr("Duplicate Sandbox Config"), this, SLOT(OnSandBoxAction()));
		m_pMenuDuplicateEx = m_pMenuTools->addAction(CSandMan::GetIcon("Duplicate"), tr("Duplicate Sandbox with Content"), this, SLOT(OnSandBoxAction()));
		m_pMenuExport = m_pMenuTools->addAction(CSandMan::GetIcon("PackBox"), tr("Export Sandbox"), this, SLOT(OnSandBoxAction()));
		m_pMenuExport->setEnabled(CArchive::IsInit());

		m_pMenuTools->addSeparator();
		m_pMenuRefresh = m_pMenuTools->addAction(CSandMan::GetIcon("Refresh"), tr("Refresh Info"), this, SLOT(OnSandBoxAction()));
		m_pMenuMkLink = m_pMenuTools->addAction(CSandMan::GetIcon("MkLink"), tr("Create Shortcut"), this, SLOT(OnSandBoxAction()));

	m_pMenuBox->addSeparator();
	m_pMenuRename = m_pMenuBox->addAction(CSandMan::GetIcon("Rename"), tr("Rename Sandbox"), this, SLOT(OnSandBoxAction()));
	m_pMenuMoveTo = m_pMenuBox->addMenu(CSandMan::GetIcon("Group"), tr("Move Sandbox"));
		m_pMenuMoveUp = m_pMenuMoveTo->addAction(CSandMan::GetIcon("Up"), tr("Move Up"), this, SLOT(OnGroupAction()));
		m_pMenuMoveUp->setShortcut(QKeySequence("Alt+Up"));
		m_pMenuMoveUp->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		this->addAction(m_pMenuMoveUp);
		//m_pMenuMoveBy = m_pMenuMoveTo->addAction(tr("Move to Position"), this, SLOT(OnGroupAction())); // does not seam that intuitive for users
		m_pMenuMoveDown = m_pMenuMoveTo->addAction(CSandMan::GetIcon("Down"), tr("Move Down"), this, SLOT(OnGroupAction()));
		m_pMenuMoveDown->setShortcut(QKeySequence("Alt+Down"));
		m_pMenuMoveDown->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		this->addAction(m_pMenuMoveDown);
		m_pMenuMoveTo->addSeparator();
	m_pMenuRemove = m_pMenuBox->addAction(CSandMan::GetIcon("Remove"), tr("Remove Sandbox"), this, SLOT(OnSandBoxAction()));


	
	m_pMenuContent = NULL;
		m_pMenuRegEdit = NULL;
	
	m_pMenuPresets = NULL;
		m_pMenuPresetsAdmin = NULL;
		m_pMenuPresetsShowUAC = NULL;
		m_pMenuPresetsNoAdmin = NULL;
		m_pMenuPresetsFakeAdmin = NULL;
		
		m_pMenuPresetsINet = NULL;
		m_pMenuPresetsShares = NULL;

		m_pMenuPresetsRecovery = NULL;
		m_pMenuPresetsForce = NULL;
		

	// Process Menu
	m_pMenuProcess = new QMenu();
	m_pMenuTerminate = m_pMenuProcess->addAction(CSandMan::GetIcon("Remove"), tr("Terminate"), this, SLOT(OnProcessAction()));
	this->addAction(m_pMenuTerminate);
	m_pMenuLinkTo = m_pMenuProcess->addAction(CSandMan::GetIcon("MkLink"), tr("Create Shortcut"), this, SLOT(OnProcessAction()));
	m_pMenuPreset = NULL;
		m_pMenuPinToRun = NULL;
		m_pMenuBlackList = NULL;
		m_pMenuAllowInternet = NULL;
		m_pMenuMarkForced = NULL;
		m_pMenuMarkLinger = NULL;
		m_pMenuMarkLeader = NULL;
	m_pMenuSuspend = NULL;
	m_pMenuResume = NULL;
}

void CSbieView::CreateGroupMenu()
{
	m_pMenuGroup = new QMenu();
	m_pMenuGroup->addAction(m_pNewBox);
	m_pMenuGroup->addAction(m_pAddGroupe);
	m_pMenuGroup->addAction(m_pImportBox);
	m_pMenuGroup->addSeparator();
	m_pRenGroupe = m_pMenuGroup->addAction(CSandMan::GetIcon("Rename"), tr("Rename Group"), this, SLOT(OnGroupAction()));
	m_pMenuGroup->addAction(CSandMan::GetIcon("Group"), tr("Move Group"))->setMenu(m_pMenuMoveTo);
	m_pDelGroupe = m_pMenuGroup->addAction(CSandMan::GetIcon("Remove"), tr("Remove Group"), this, SLOT(OnGroupAction()));
}

void CSbieView::CreateTrayMenu()
{
	m_pMenuTray = new QMenu();
	m_pMenuTray->addMenu(m_pMenuRun);
	m_pMenuTray->addAction(m_pMenuEmptyBox);
	m_pMenuTray->addSeparator();
	m_pMenuTray->addAction(m_pMenuBrowse);
	m_pMenuTray->addAction(m_pMenuExplore);
	m_pMenuTray->addAction(m_pMenuRegEdit);
	m_pMenuTray->addAction(m_pMenuSnapshots);
	m_pMenuTray->addAction(m_pMenuRecover);
	m_pMenuTray->addAction(m_pMenuCleanUp);
	m_pMenuTray->addSeparator();
	if (m_pMenuPresets) {
		m_pMenuTray->addAction(m_pMenuOptions);
		m_pMenuTray->addMenu(m_pMenuPresets);
	}
}

int CSbieView__ParseGroup(const QString& Grouping, QMap<QString, QStringList>& m_Groups, const QString& Parent = "", int Index = 0)
{
	QRegularExpression RegExp("[,()]", QRegularExpression::CaseInsensitiveOption);
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
		if (Grouping.at(pos) == '(')
		{
			m_Groups[Name] = QStringList();
			Index = CSbieView__ParseGroup(Grouping, m_Groups, Name, Index);
		}
		else if (Grouping.at(pos) == ')')
			break;
	}
	return Index;
}

QString CSbieView__SerializeGroup(QMap<QString, QStringList>& m_Groups, const QString& Parent = "", QSet<QString> Test = QSet<QString>())
{
	QStringList Grouping;
	foreach(const QString& Name, m_Groups[Parent])
	{
		if (Test.contains(Name))
			continue; // recursion, skil
		Test.insert(Name);
		if (m_Groups.contains(Name))
			Grouping.append(Name + "(" + CSbieView__SerializeGroup(m_Groups, Name, Test) + ")");
		else
			Grouping.append(Name);
	}
	return Grouping.join(",");
}

void CSbieView::Refresh()
{
	QList<QVariant> Added = m_pSbieModel->Sync(theAPI->GetAllBoxes(), m_Groups, theGUI->IsShowHidden());

	if (m_pSbieModel->IsTree())
	{
		QTimer::singleShot(10, this, [this, Added]() {
			foreach(const QVariant ID, Added) {

				QModelIndex ModelIndex = m_pSbieModel->FindIndex(ID);

				if (m_pSbieModel->GetType(ModelIndex) == CSbieModel::eProcess) {
					m_HoldExpand = true;
					m_pSbieTree->expand(m_pSortProxy->mapFromSource(ModelIndex));
					m_HoldExpand = false;
				}
				else 
				{
					QString Name;
					if (m_pSbieModel->GetType(ModelIndex) == CSbieModel::eGroup)
						Name = m_pSbieModel->GetID(ModelIndex).toString();
					else if (m_pSbieModel->GetType(ModelIndex) == CSbieModel::eBox)
						Name = m_pSbieModel->GetSandBox(ModelIndex)->GetName();

					if (!m_Collapsed.contains(Name)) {
						m_HoldExpand = true;
						m_pSbieTree->expand(m_pSortProxy->mapFromSource(ModelIndex));
						m_HoldExpand = false;
					}
				}
			}
		});
	}

	// add new boxes to the default group

	foreach(const QStringList &list, m_Groups) {
		foreach(const QString &str, list)
			Added.removeAll(str);
	}

	if (!Added.isEmpty()) {
		bool bChanged = false;
		foreach(const QVariant& ID, Added) {
			if (ID.type() == QVariant::String) {
				QString id = ID.toString();
				if (id.left(1) != "!") {
					bChanged = true;
					m_Groups[""].append(id);
				}
			}
		}

		if(bChanged)
			SaveBoxGrouping();
	}
}

void CSbieView::OnToolTipCallback(const QVariant& ID, QString& ToolTip)
{
	if (ID.type() == QVariant::String)
	{
		QString BoxName = ID.toString();
		CSandBoxPtr pBox = theAPI->GetBoxByName(BoxName);
		auto pBoxEx = pBox.objectCast<CSandBoxPlus>();
		if (!pBoxEx)
			return;

		// todo more info

		ToolTip = BoxName + "\n";
		ToolTip += tr("    File root: %1\n").arg(pBoxEx->GetFileRoot());
		ToolTip += tr("    Registry root: %1\n").arg(pBoxEx->GetRegRoot());
		ToolTip += tr("    IPC root: %1\n").arg(pBoxEx->GetIpcRoot());
		if(!pBoxEx->GetMountRoot().isEmpty())
			ToolTip += tr("    Disk root: %1\n").arg(pBoxEx->GetMountRoot());
		
		ToolTip += tr("Options:\n    ");
		ToolTip += pBoxEx->GetStatusStr().replace(", ", "\n    ");
	}
	else if (quint32 ProcessId = ID.toUInt())
	{
		// todo proc info
	}
}

void CSbieView::OnCustomSortByColumn(int column)
{
	Qt::SortOrder order = m_pSbieTree->header()->sortIndicatorOrder();
	//m_pSbieTree->sortByColumn(column, order);
	//m_pSbieTree->header()->setSortIndicatorShown(true);
	if (column == 0) {
		if (m_pSortProxy->sortRole() == Qt::InitialSortOrderRole) {
			m_pSortProxy->sort(0, Qt::AscendingOrder);
			m_pSortProxy->setSortRole(Qt::EditRole);
			theConf->SetValue("MainWindow/BoxTree_UseOrder", false);
			m_pSbieTree->header()->setSortIndicatorShown(true);
		} else if (order == Qt::DescendingOrder) {
			SetCustomOrder();
			theConf->SetValue("MainWindow/BoxTree_UseOrder", true);
		}
	}
	else {
		m_pSortProxy->setSortRole(Qt::EditRole);
		m_pSbieTree->header()->setSortIndicatorShown(true);
	}
}

bool CSbieView::UpdateMenu(bool bAdvanced, const CSandBoxPtr &pBox, int iSandBoxeCount, bool bBoxBusy, bool bBoxNotMounted)
{
	QList<QAction*> MenuActions = m_pMenu->actions();

	auto pBoxEx = pBox.objectCast<CSandBoxPlus>();

	m_pStopAsync->setVisible(bBoxBusy);

	m_pMenuMount->setVisible(bBoxNotMounted);
	m_pMenuMount->setEnabled(iSandBoxeCount == 1);

	if (bBoxBusy)
		iSandBoxeCount = 0;

	m_pMenuRun->setEnabled(iSandBoxeCount == 1);
	if(iSandBoxeCount == 1)
		UpdateRunMenu(pBox);

	m_pMenuRename->setEnabled(iSandBoxeCount == 1 && pBoxEx->GetMountRoot().isEmpty());

	m_pMenuOptions->setEnabled(iSandBoxeCount == 1);

	if (m_pMenuPresets) {
		m_pMenuPresets->setEnabled(iSandBoxeCount == 1);
		if (iSandBoxeCount == 1) {
			m_pMenuPresetsShowUAC->setChecked(pBox && !pBox->GetBool("DropAdminRights", false) && !pBox->GetBool("FakeAdminRights", false));
			m_pMenuPresetsNoAdmin->setChecked(pBox && pBox->GetBool("DropAdminRights", false) && !pBox->GetBool("FakeAdminRights", false));
			m_pMenuPresetsFakeAdmin->setChecked(pBox && pBox->GetBool("DropAdminRights", false) && pBox->GetBool("FakeAdminRights", false));
			m_pMenuPresetsINet->setChecked(pBox && pBox.objectCast<CSandBoxPlus>()->IsINetBlocked());
			m_pMenuPresetsShares->setChecked(pBox && pBox.objectCast<CSandBoxPlus>()->HasSharesAccess());
			m_pMenuPresetsRecovery->setChecked(pBox && pBox->GetBool("AutoRecover", false));
			m_pMenuPresetsForce->setChecked(pBox && pBox->GetBool("DisableForceRules", false));
		}
	}

	if (bBoxNotMounted)
		iSandBoxeCount = 0;

	m_pMenuMkLink->setEnabled(iSandBoxeCount == 1);
	m_pMenuTools->setEnabled(iSandBoxeCount == 1);
	m_pMenuUnmount->setVisible(pBoxEx && pBoxEx->UseImageFile() && !pBoxEx->GetMountRoot().isEmpty());
	m_pMenuRecover->setEnabled(iSandBoxeCount == 1);
	m_pMenuCleanUp->setEnabled(iSandBoxeCount > 0);
	if (m_pMenuContent) m_pMenuContent->setEnabled(iSandBoxeCount > 0);
	m_pMenuEmptyBox->setEnabled(iSandBoxeCount > 0);

	m_pMenuBrowse->setEnabled(iSandBoxeCount == 1);
	m_pMenuExplore->setEnabled(iSandBoxeCount == 1);
	if(m_pMenuRegEdit)m_pMenuRegEdit->setEnabled(iSandBoxeCount == 1);
	m_pMenuSnapshots->setEnabled(iSandBoxeCount == 1);

	m_pCopyCell->setVisible(bAdvanced);
	m_pCopyRow->setVisible(bAdvanced);
	m_pCopyPanel->setVisible(bAdvanced);

	return bBoxBusy == false;
}

void CSbieView::UpdateProcMenu(const CBoxedProcessPtr& pProcess, int iProcessCount, int iSuspendedCount)
{
	m_pMenuLinkTo->setEnabled(iProcessCount == 1);

	CSandBoxPlus* pBoxPlus = pProcess.objectCast<CSbieProcess>()->GetBox();
	QStringList RunOptions = pBoxPlus->GetTextList("RunCommand", true);

	QString FoundPin;
	QString FileName = pProcess->GetFileName();
	foreach(const QString& RunOption, RunOptions) {
		QVariantMap Entry = GetRunEntry(RunOption);
		QString CmdFile = pBoxPlus->GetCommandFile(Entry["Command"].toString());
		if(CmdFile.compare(FileName, Qt::CaseInsensitive) == 0) {
			FoundPin = RunOption;
			break;
		}
	}

	if (m_pMenuPreset) {
		m_pMenuPinToRun->setChecked(!FoundPin.isEmpty());
		m_pMenuPinToRun->setData(FoundPin);
		m_pMenuPinToRun->setProperty("WorkingDir", pProcess->GetWorkingDir());

		m_pMenuAllowInternet->setChecked(pProcess.objectCast<CSbieProcess>()->HasInternetAccess());

		m_pMenuMarkForced->setChecked(pProcess.objectCast<CSbieProcess>()->IsForcedProgram());

		int isLingering = pProcess.objectCast<CSbieProcess>()->IsLingeringProgram();
		m_pMenuMarkLinger->setChecked(isLingering != 0);
		m_pMenuMarkLinger->setEnabled(isLingering != 2);
		m_pMenuMarkLeader->setChecked(pProcess.objectCast<CSbieProcess>()->IsLeaderProgram());
	}

	if (m_pMenuSuspend) m_pMenuSuspend->setEnabled(iProcessCount > iSuspendedCount);
	if (m_pMenuResume) m_pMenuResume->setEnabled(iSuspendedCount > 0);
}

bool CSbieView::UpdateMenu()
{
	m_CurSandBoxes.clear();
	m_CurProcesses.clear();

	CSandBoxPtr pBox;
	bool bBoxBusy = false;
	bool bBoxNotMounted = false;
	CBoxedProcessPtr pProcess;
	int iProcessCount = 0;
	int iSandBoxeCount = 0;
	int iGroupe = 0;
	int iSuspendedCount = 0;
	QModelIndexList Rows = m_pSbieTree->selectedRows();
	foreach(const QModelIndex& Index, Rows)
	{
		QModelIndex ModelIndex = m_pSortProxy->mapToSource(Index);
		
		pProcess = m_pSbieModel->GetProcess(ModelIndex);
		if (pProcess)
		{
			m_CurProcesses.append(pProcess);
			iProcessCount++;
			if (pProcess->TestSuspended())
				iSuspendedCount++;
		}
		else
		{
			pBox = m_pSbieModel->GetSandBox(ModelIndex);
			if (pBox)
			{
				m_CurSandBoxes.append(pBox);

				if (!pBox->IsEnabled())
					iSandBoxeCount = -1;
				else if (iSandBoxeCount != -1)
					iSandBoxeCount++;

				auto pBoxEx = pBox.objectCast<CSandBoxPlus>();
				if(pBoxEx->IsBoxBusy())
					bBoxBusy = true;
				if (pBoxEx->UseImageFile() && pBoxEx->GetMountRoot().isEmpty())
					bBoxNotMounted = true;
			}
			else
				iGroupe++;
		}
	}

	bool bAdvanced = theConf->GetInt("Options/ViewMode", 1) == 1
		|| (QGuiApplication::queryKeyboardModifiers() & Qt::ControlModifier) != 0;

	m_pRenGroupe->setVisible(iGroupe == 1 && iSandBoxeCount == 0 && iProcessCount == 0);
	m_pDelGroupe->setVisible(iGroupe > 0 && iSandBoxeCount == 0 && iProcessCount == 0);

	if (!pProcess.isNull())
		UpdateProcMenu(pProcess, iProcessCount, iSuspendedCount);

	return UpdateMenu(bAdvanced, pBox, iSandBoxeCount, bBoxBusy, bBoxNotMounted);
}

void CSbieView::OnMenu(const QPoint& Point)
{
	if (!theAPI->IsConnected())
		return;

	UpdateMenu();

	UpdateCopyMenu();
	if (!m_CurProcesses.isEmpty())
		m_pMenuProcess->popup(QCursor::pos());	
	else if (!m_CurSandBoxes.isEmpty())
		m_pMenuBox->popup(QCursor::pos());
	else if (!GetSelectedGroups().isEmpty())
		m_pMenuGroup->popup(QCursor::pos());	
	else
		m_pMenu->popup(QCursor::pos());	
}

void CSbieView::UpdateMoveMenu()
{
    // update move-to menu

    // Clear existing entries that have data
    foreach (QAction* pAction, m_pMenuMoveTo->actions()) {
        if (!pAction->data().toString().isNull())
            m_pMenuMoveTo->removeAction(pAction);
    }

    // Build list of (groupKey, displayName)
    struct Item { QString key; QString name; };
    QList<Item> items;
    foreach (const QString& Group, m_Groups.keys()) {
        // Compute full hierarchical name
        QString temp = Group;
        QString fullName = Group;
        while (true) {
            QString parent = FindParent(temp);
            if (parent.isEmpty())
                break;
            temp = parent;
            fullName.prepend(parent + " > ");
        }
        items.append({ Group, fullName });
    }

    // Sort hierarchically: compare segment-by-segment
    std::sort(items.begin(), items.end(), [](const Item& a, const Item& b) {
        const QStringList sa = a.name.split(" > ");
        const QStringList sb = b.name.split(" > ");
        int n = qMin(sa.size(), sb.size());
        for (int i = 0; i < n; ++i) {
            int cmp = QString::compare(sa[i], sb[i], Qt::CaseInsensitive);
            if (cmp != 0)
                return cmp < 0;
        }
        // if one is prefix of the other, shorter one first
        return sa.size() < sb.size();
    });

    // Populate menu in sorted order
    for (const Item& item : items) {
        QString display = item.name.isEmpty() ? tr("[None]") : item.name;
        QAction* pAction = m_pMenuMoveTo->addAction(display, this, SLOT(OnGroupAction()));
        pAction->setData(item.key);
    }
    // Optionally enable only if more than one group
    // m_pMenuMoveTo->setEnabled(m_Groups.keys().count() > 1);
}

void CSbieView::RenameGroup(const QString OldName, const QString NewName)
{
	auto Group = m_Groups.take(OldName);
	m_Groups.insert(NewName, Group);

	RenameItem(OldName, NewName);
}

void CSbieView::RenameItem(const QString OldName, const QString NewName)
{
	quint64 Size = theConf->GetValue("SizeCache/" + OldName, -1).toLongLong();
	theConf->DelValue("SizeCache/" + OldName);
	if(Size != -1) theConf->SetValue("SizeCache/" + NewName, Size);

	if (m_Collapsed.remove(OldName))
		m_Collapsed.insert(NewName);

	for (auto I = m_Groups.begin(); I != m_Groups.end(); ++I)
	{
		if (I.value().removeOne(OldName))
			I.value().append(NewName);
	}
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

QStringList CSbieView::GetSelectedGroups(bool bAndBoxes)
{
	QStringList list;
	foreach(const QModelIndex & Index, m_pSbieTree->selectedRows())
	{
		QModelIndex ModelIndex = m_pSortProxy->mapToSource(Index);
		QString Name;
		if (m_pSbieModel->GetType(ModelIndex) == CSbieModel::eGroup)
			Name = m_pSbieModel->GetID(ModelIndex).toString();
		else if (bAndBoxes && m_pSbieModel->GetType(ModelIndex) == CSbieModel::eBox)
			Name = m_pSbieModel->GetSandBox(ModelIndex)->GetName();

		if (Name.isEmpty())
				continue;

		list.append(Name);
	}
	return list;
}

void CSbieView::OnGroupAction()
{
	OnGroupAction(qobject_cast<QAction*>(sender()));
}

void CSbieView::OnGroupAction(QAction* Action)
{
	if (Action == m_pNewBox || Action == m_pAddGroupe || Action == m_pImportBox)
	{
		QStringList List = GetSelectedGroups();

		QString Name = Action == m_pNewBox ? AddNewBox() : (Action == m_pImportBox ? ImportSandbox() : AddNewGroup());
		if (Name.isEmpty())
			return;

		if (List.isEmpty())
			return;
		m_Groups[""].removeAll(Name);
		m_Groups[List.first()].removeAll(Name);
		m_Groups[List.first()].append(Name);
	}
	else if (Action == m_pRenGroupe)
	{
		QStringList List = GetSelectedGroups();
		if (List.isEmpty())
			return;
		
		QString OldValue = List.first();

		QString Value = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter a new name for the Group."), QLineEdit::Normal, OldValue);
		if (Value.isEmpty() || Value == OldValue)
			return;
		if (!TestNameAndWarn(Value))
			return;

		RenameGroup(OldValue, Value);
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

				QStringList Items = m_Groups.take(Group); // remove group
		
				// remove from parents
				for (auto I = m_Groups.begin(); I != m_Groups.end(); ++I) {
					if (I.value().removeOne(Group)) {
						// move items to grand parent
						I.value().append(Items);
						break;
					}
				}
				m_Collapsed.remove(Group);
			}
		}
	}
	else if (Action == m_pMenuMoveUp /*|| Action == m_pMenuMoveBy*/ || Action == m_pMenuMoveDown)
	{
		if (!theConf->GetBool("MainWindow/BoxTree_UseOrder", false)) {
			SetCustomOrder();
			theConf->SetValue("MainWindow/BoxTree_UseOrder", true);
		}

		int Offset = 0;
		if (Action == m_pMenuMoveUp)
			Offset = -1;
		else if (Action == m_pMenuMoveDown)
			Offset = 1;
		else
			Offset = QInputDialog::getInt(this, "Sandboxie-Plus", tr("Move entries by (negative values move up, positive values move down):"), 0);
		if (Offset == 0)
			return;

		// todo: fix behaviour on multiple selection
		QMap<QString, QVector<int>> GroupPositions;
		bool bOutBounded = false;
		auto FindPosition = [this, Offset, &bOutBounded, &GroupPositions](const QString& Name) -> bool {
			foreach(const QString& Group, m_Groups.keys()) {
				int pos = m_Groups[Group].indexOf(Name);
				if (pos != -1) {
					if (pos + Offset >= 0 && pos + Offset < m_Groups[Group].count())
						GroupPositions[Group].append(pos);
					else
						bOutBounded = true;
					return true;
				}
			}
			return false;
		};
		foreach(const QString& Name, GetSelectedGroups(true)) {
			if (!FindPosition(Name)) {
				m_Groups[""].prepend(Name);
				FindPosition(Name);
			}
			if (bOutBounded)
				break;
		}
		if (bOutBounded)
			QApplication::beep();
		else {
			foreach(const QString& Group, GroupPositions.keys()) {
				std::sort(GroupPositions[Group].begin(), GroupPositions[Group].end(), [Offset](const int& a, const int& b) {
					return Offset > 0 && a > b || Offset < 0 && a < b;
					});
				foreach(const int ItemIndex, GroupPositions[Group]) {
					m_Groups[Group].swapItemsAt(ItemIndex + Offset, ItemIndex);
				}
			}
		}
	}
	else // move to group
	{
		QString Group = Action->data().toString();
		
		foreach(const QString& Name, GetSelectedGroups(true))
		{
			if (Name == Group || IsParentOf(Name, Group)) {
				QMessageBox("Sandboxie-Plus", tr("A group can not be its own parent."), QMessageBox::Critical, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this).exec();
				continue;
			}

			MoveItem(Name, Group);
		}
	}

	if (!(Action == m_pMenuMoveUp /*|| Action == m_pMenuMoveBy*/ || Action == m_pMenuMoveDown)) {
		m_pSbieModel->Clear(); //todo improve that
		Refresh();
	}

	UpdateMoveMenu();

	SaveBoxGrouping();
}

void CSbieView::SetCustomOrder()
{
	m_pSortProxy->sort(0, Qt::AscendingOrder);
	m_pSortProxy->setSortRole(Qt::InitialSortOrderRole);
	m_pSbieTree->header()->setSortIndicatorShown(false);
}

bool CSbieView::MoveItem(const QString& Name, const QString& To, int pos)
{
	QString From;

	// remove from old
	for (auto I = m_Groups.begin(); I != m_Groups.end(); ++I) {
		for (int i = 0; i < I.value().count(); i++) {
			if (I.value().at(i) == Name) {
				I.value().removeAt(i);
				From = I.key();
				if(From == To && i < pos)
					pos--;
				break;
			}
		}
	}

	// add to new
	if (pos < 0/* || pos > m_Groups[To].size()*/)
		m_Groups[To].append(Name);
	else
		m_Groups[To].insert(pos, Name);

	return From != To;
}

QString CSbieView::AddNewBox(bool bAlowTemp)
{
	QString BoxName = CNewBoxWizard::CreateNewBox(bAlowTemp, this);

	if (!BoxName.isEmpty()) {
		theAPI->ReloadBoxes();
		Refresh();
		SelectBox(BoxName);
	}
	return BoxName;
}

QString CSbieView::ImportSandbox()
{
	QString Path = QFileDialog::getOpenFileName(this, tr("Select file name"), "", tr("7-Zip Archive (*.7z);;Zip Archive (*.zip)"));
	if (Path.isEmpty())
		return "";

	QString Password;
	quint64 ImageSize = 0;
	
	CArchive Archive(Path);
	int Ret = Archive.Open();
	if (Ret == ERR_7Z_PASSWORD_REQUIRED) {
		for (;;) {
			CBoxImageWindow window(CBoxImageWindow::eImport, this);
			if (!theGUI->SafeExec(&window) == 1)
				return "";
			Archive.SetPassword(window.GetPassword());
			Ret = Archive.Open();
			if (Ret != ERR_7Z_OK) {
				QMessageBox::critical(this, "Sandboxie-Plus", tr("Failed to open archive, wrong password?"));
				continue;
			}
			Password = window.GetPassword();
			ImageSize = window.GetImageSize();
			break;
		}
	}
	if (Ret != ERR_7Z_OK) {
		QMessageBox::critical(this, "Sandboxie-Plus", tr("Failed to open archive (%1)!").arg(Ret));
		return "";
	}
	Archive.Close();

	StrPair PathName = Split2(Path, "/", true);
	StrPair NameEx = Split2(PathName.second, ".", true);
	QString Name = NameEx.first;
	
	CExtractDialog optWnd(Name, this);
	if(!Password.isEmpty())
		optWnd.ShowNoCrypt();
	if (!theGUI->SafeExec(&optWnd) == 1)
		return "";
	Name = optWnd.GetName();
	QString BoxRoot = optWnd.GetRoot();

	CSandBoxPtr pBox;
	SB_PROGRESS Status = theAPI->CreateBox(Name);
	if (!Status.IsError()) {
		pBox = theAPI->GetBoxByName(Name);
		if (pBox) {

			auto pBoxEx = pBox.objectCast<CSandBoxPlus>();

			if (!BoxRoot.isEmpty())
				pBox->SetFileRoot(BoxRoot);

			if (!Password.isEmpty() && !optWnd.IsNoCrypt()) {
				Status = pBoxEx->ImBoxCreate(ImageSize / 1024, Password);
				if (!Status.IsError())
					Status = pBoxEx->ImBoxMount(Password, true, true);
			}

			if (!Status.IsError())
				Status = pBoxEx->ImportBox(Path, Password);

			// always overwrite restored FileRootPath
			pBox->SetText("FileRootPath", BoxRoot);
		}
	}

	if (Status.GetStatus() == OP_ASYNC) {
		Status = theGUI->AddAsyncOp(Status.GetValue(), true, tr("Importing: %1").arg(Path));
		if (Status.IsError()) {
			theGUI->DeleteBoxContent(pBox, CSandMan::eForDelete);
			pBox->RemoveBox();
		}
	}
	else
		theGUI->CheckResults(QList<SB_STATUS>() << Status, this);

	return Name;
}

QString CSbieView::AddNewGroup()
{
	QString Name = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter a new group name"), QLineEdit::Normal);
	if (Name.isEmpty() || m_Groups.contains(Name))
		return "";
	if (!TestNameAndWarn(Name))
		return "";

	m_Groups[Name] = QStringList();

	QModelIndex ModelIndex = m_pSortProxy->mapToSource(m_pSbieTree->currentIndex());
	QString Parent;
	if (m_pSbieModel->GetType(ModelIndex) == CSbieModel::eGroup)
		Parent = m_pSbieModel->GetID(ModelIndex).toString();

	m_Groups[Parent].append(Name);

	UpdateMoveMenu();

	SaveBoxGrouping();

	return Name;
}

bool CSbieView::TestNameAndWarn(const QString& Name)
{
	if (Name.contains(QRegularExpression("[,()\r\n\t]")))
	{
		QMessageBox::critical(this, "Sandboxie-Plus", tr("The Sandbox name and Box Group name cannot use the ',()' symbol or control characters."));
		return false;
	}

	if (m_Groups.contains(Name)) {
		QMessageBox::critical(this, "Sandboxie-Plus", tr("This name is already used for a Box Group."));
		return false;
	}

	if (!theAPI->GetBoxByName(QString(Name).replace(" ", "_")).isNull()) {
		QMessageBox::critical(this, "Sandboxie-Plus", tr("This name is already used for a Sandbox."));
		return false;
	}

	return true;
}

void CSbieView::OnSandBoxAction()
{
	OnSandBoxAction(qobject_cast<QAction*>(sender()), m_CurSandBoxes);
}

void CSbieView::OnSandBoxAction(QAction* pAction)
{
	OnSandBoxAction(pAction, m_CurSandBoxes);
}

void CSbieView::OnSandBoxAction(QAction* Action, const QList<CSandBoxPtr>& SandBoxes)
{
	auto RenderSandboxNameList_ = [&,this](const QList<CSandBoxPtr>& SandBoxes, int max_displayed = 10) -> QString
	{
		QString name_list = "";
		for (int i = 0; i < SandBoxes.count() && i < max_displayed; i++)
		{
			if (i != 0) name_list.append("<br />");
			name_list.append(QString::fromWCharArray(L"\u2022 ")); // Unicode bullet
			name_list.append("<b>" + SandBoxes[i].objectCast<CSandBoxPlus>()->GetDisplayName() + "</b>");
		}
		if (SandBoxes.count() > max_displayed) 
		{
			name_list.append(tr("<br />"));
			name_list.append(QString::fromWCharArray(L"\u2022 ")); // Unicode bullet
			name_list.append(tr("... and %1 more").arg(SandBoxes.count() - max_displayed));
		}
			
		return name_list;
	};

	QList<SB_STATUS> Results;

	if (SandBoxes.isEmpty())
		return;
	if (Action == m_pStopAsync)
	{
		foreach(const CSandBoxPtr& pBox, SandBoxes)
		{
			auto pBoxEx = pBox.objectCast<CSandBoxPlus>();
			pBoxEx->OnCancelAsync();
		}
	}
	else if (Action == m_pMenuRunAny)
	{
		/*QString Command = ShowRunDialog(SandBoxes.first()->GetName());
		if(!Command.isEmpty())
			SandBoxes.first()->RunCommand(Command);*/

		Results.append(theGUI->RunStart(SandBoxes.first()->GetName(), "run_dialog"));
	}
	else if (Action == m_pMenuRunBrowser)
		Results.append(theGUI->RunStart(SandBoxes.first()->GetName(), "default_browser"));
	else if (Action == m_pMenuRunMailer)
		Results.append(theGUI->RunStart(SandBoxes.first()->GetName(), "mail_agent"));
	else if (Action == m_pMenuRunExplorer)
	{
		if (theConf->GetInt("Options/ViewMode", 1) != 1 && theConf->GetBool("Options/BoxedExplorerInfo", true))
		{
			bool State = false;
			CCheckableMessageBox::question(this, "Sandboxie-Plus",
				theAPI->GetSbieMsgStr(0x00000DCDL, theGUI->m_LanguageId) // MSG_3533
				, tr("Don't show this message again."), &State, QDialogButtonBox::Ok, QDialogButtonBox::Ok, QMessageBox::Information);

			if (State)
				theConf->SetValue("Options/BoxedExplorerInfo", false);
		}

		Results.append(theGUI->RunStart(SandBoxes.first()->GetName(), "explorer.exe /e,::{20D04FE0-3AEA-1069-A2D8-08002B30309D}"));
	}
	else if (Action == m_pMenuRunRegEdit)
		Results.append(theGUI->RunStart(SandBoxes.first()->GetName(), "regedit.exe"));
	else if (Action == m_pMenuRunAppWiz)
		Results.append(theGUI->RunStart(SandBoxes.first()->GetName(), "\"C:\\WINDOWS\\System32\\control.exe\" \"C:\\Windows\\System32\\appwiz.cpl\""));
	else if (Action == m_pMenuAutoRun)
		Results.append(theGUI->RunStart(SandBoxes.first()->GetName(), "auto_run"));
	else if (Action == m_pMenuRunCmd)
		Results.append(theGUI->RunStart(SandBoxes.first()->GetName(), "cmd.exe"));
	else if (Action == m_pMenuRunCmdAdmin)
		Results.append(theGUI->RunStart(SandBoxes.first()->GetName(), "cmd.exe", CSbieAPI::eStartElevated));
#ifdef _WIN64
	else if (Action == m_pMenuRunCmd32)
		Results.append(theGUI->RunStart(SandBoxes.first()->GetName(), "C:\\WINDOWS\\SysWOW64\\cmd.exe"));
#endif
	else if (Action == m_pMenuPresetsShowUAC)
	{
		SandBoxes.first()->SetBoolSafe("DropAdminRights", false);
		SandBoxes.first()->SetBoolSafe("FakeAdminRights", false);
	}
	else if (Action == m_pMenuPresetsNoAdmin)
	{
		SandBoxes.first()->SetBoolSafe("DropAdminRights", true);
		SandBoxes.first()->SetBoolSafe("FakeAdminRights", false);
	}
	else if (Action == m_pMenuPresetsFakeAdmin)
	{
		SandBoxes.first()->SetBoolSafe("DropAdminRights", true);
		SandBoxes.first()->SetBoolSafe("FakeAdminRights", true);
	}
	else if (Action == m_pMenuPresetsINet)
		SandBoxes.first().objectCast<CSandBoxPlus>()->SetINetBlock(m_pMenuPresetsINet->isChecked());
	else if (Action == m_pMenuPresetsShares)
		SandBoxes.first().objectCast<CSandBoxPlus>()->SetAllowShares(m_pMenuPresetsShares->isChecked());
	else if (Action == m_pMenuPresetsRecovery)
		m_pMenuPresetsRecovery->setChecked(SandBoxes.first()->SetBoolSafe("AutoRecover", m_pMenuPresetsRecovery->isChecked()));
	else if (Action == m_pMenuPresetsForce)
		m_pMenuPresetsForce->setChecked(SandBoxes.first()->SetBoolSafe("DisableForceRules", m_pMenuPresetsForce->isChecked()));
	else if (Action == m_pMenuOptions)
		ShowOptions(SandBoxes.first());
	else if (Action == m_pMenuBrowse)
		ShowBrowse(SandBoxes.first());
	else if (Action == m_pMenuRefresh)
	{
		foreach(const CSandBoxPtr& pBox, SandBoxes)
		{
			pBox.objectCast<CSandBoxPlus>()->UpdateSize();
			if (theConf->GetBool("Options/ScanStartMenu", true))
				pBox.objectCast<CSandBoxPlus>()->ScanStartMenu();
		}	
	}
	else if (Action == m_pMenuExplore)
	{
		if (SandBoxes.first()->IsEmpty()) {
			QMessageBox("Sandboxie-Plus", tr("This Sandbox is empty."), QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this).exec();
			return;
		}

		if (theConf->GetInt("Options/ViewMode", 1) != 1 && theConf->GetBool("Options/ExplorerInfo", true))
		{
			bool State = false;
			CCheckableMessageBox::question(this, "Sandboxie-Plus",
				theAPI->GetSbieMsgStr(0x00000DCEL, theGUI->m_LanguageId) // MSG_3534
				, tr("Don't show this message again."), &State, QDialogButtonBox::Ok, QDialogButtonBox::Ok, QMessageBox::Information);

			if (State)
				theConf->SetValue("Options/ExplorerInfo", false);
		}

		::ShellExecuteW(NULL, NULL, SandBoxes.first()->GetFileRoot().toStdWString().c_str(), NULL, NULL, SW_SHOWNORMAL);
	}
	else if (Action == m_pMenuRegEdit)
	{
		if (SandBoxes.first()->IsEmpty()) {
			QMessageBox("Sandboxie-Plus", tr("This Sandbox is empty."), QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this).exec();
			return;
		}

 		if (theConf->GetInt("Options/WarnOpenRegistry", -1) == -1)
		{
			bool State = false;
			if (CCheckableMessageBox::question(this, "Sandboxie-Plus", tr("WARNING: The opened registry editor is not sandboxed, please be careful and only do changes to the preselected sandbox locations.")
			  , tr("Don't show this warning in future"), &State, QDialogButtonBox::Ok | QDialogButtonBox::Cancel, QDialogButtonBox::Yes, QMessageBox::Information) != QDialogButtonBox::Ok)
				return;

			if (State)
				theConf->SetValue("Options/WarnOpenRegistry", 1);
		}

		std::wstring path = QCoreApplication::applicationFilePath().toStdWString();

		QStringList RegRoot = SandBoxes.first()->GetRegRoot().split("\\");
		while (RegRoot.first().isEmpty())
			RegRoot.removeFirst();
		RegRoot[0] = QString("Computer");
		if(RegRoot[1] == "USER")
			RegRoot[1] = QString("HKEY_USERS");
		else if(RegRoot[1] == "MACHINE")
			RegRoot[1] = QString("HKEY_LOCAL_MACHINE");
		std::wstring params = L"/OpenReg \"" + RegRoot.join("\\").toStdWString() + L"\"";
		if (SandBoxes.first()->GetActiveProcessCount() == 0)
			params += L" \"" + theAPI->GetStartPath().toStdWString() + L" /box:" + SandBoxes.first()->GetName().toStdWString() + L" mount_hive\"";

		SHELLEXECUTEINFOW shex;
		memset(&shex, 0, sizeof(shex));
		shex.cbSize = sizeof(shex);
		shex.fMask = SEE_MASK_FLAG_NO_UI;
		shex.hwnd = NULL;
		shex.lpFile = path.c_str();
		shex.lpParameters = params.c_str();
		shex.nShow = SW_SHOWNORMAL;
		shex.lpVerb = L"runas";

		ShellExecuteExW(&shex);
	}
	else if (Action == m_pMenuSnapshots)
	{
		CSandBoxPtr pBox = SandBoxes.first();

		static QMap<void*, CSnapshotsWindow*> SnapshotWindows;
		CSnapshotsWindow* pSnapshotsWindow = SnapshotWindows.value(pBox.data());
		if (pSnapshotsWindow == NULL) {
			pSnapshotsWindow = new CSnapshotsWindow(SandBoxes.first(), this);
			connect(theGUI, SIGNAL(Closed()), pSnapshotsWindow, SLOT(close()));
			SnapshotWindows.insert(pBox.data(), pSnapshotsWindow);
			connect(pSnapshotsWindow, &CSnapshotsWindow::Closed, [this, pBox]() {
				SnapshotWindows.remove(pBox.data());
			});
			CSandMan::SafeShow(pSnapshotsWindow);
		}
		else {
			pSnapshotsWindow->setWindowState((pSnapshotsWindow->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
			SetForegroundWindow((HWND)pSnapshotsWindow->winId());
		}
	}
	else if (Action == m_pMenuDuplicate || Action == m_pMenuDuplicateEx)
	{
		CSandBoxPtr pSrcBox = theAPI->GetBoxByName(SandBoxes.first()->GetName());
		if (!pSrcBox) return;

		QString OldValue = pSrcBox->GetName().replace("_", " ");
		QString Value = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter a new name for the duplicated Sandbox."), QLineEdit::Normal, tr("%1 Copy").arg(OldValue));
		if (Value.isEmpty() || Value == OldValue)
			return;
		
		QString Name = Value.replace(" ", "_");
		SB_STATUS Status = theAPI->CreateBox(Name, false);
		
		if (!Status.IsError())
		{
			QString Section; 
			Section = theAPI->SbieIniGetEx(pSrcBox->GetName(), "");
			Status = theAPI->SbieIniSet(Name, "", Section);
			theAPI->ReloadBoxes(true);
		}

		CSandBoxPtr pDestBox;
		if (!Status.IsError())
		{
			pDestBox = theAPI->GetBoxByName(Name);
			if(!pDestBox)
				Status = SB_ERR(SB_FailedCopyConf, QVariantList() << SandBoxes.first()->GetName() << tr("Not Created"));
		}

		if (Action == m_pMenuDuplicateEx && !Status.IsError())
		{
			auto pSrcBoxEx = pSrcBox.objectCast<CSandBoxPlus>();
			SB_PROGRESS Progress = pSrcBoxEx->CopyBox(pDestBox->GetFileRoot());

			if (Progress.GetStatus() == OP_ASYNC)
				Status = theGUI->AddAsyncOp(Progress.GetValue(), false, tr("Copying: %1").arg(Value));
			else
				Status = Progress;
		}
		
		Results.append(Status);
	}
	else if (Action == m_pMenuExport)
	{
		CSandBoxPtr pBox = SandBoxes.first();
		auto pBoxEx = pBox.objectCast<CSandBoxPlus>();

		CCompressDialog optWnd(this);
		if (pBoxEx->UseImageFile())
			optWnd.SetMustEncrypt();
		if (!theGUI->SafeExec(&optWnd) == 1)
			return;

		QString Password;
		if (optWnd.UseEncryption()) {
			CBoxImageWindow pwWnd(CBoxImageWindow::eExport, this);
			if (!theGUI->SafeExec(&pwWnd) == 1)
				return;
			Password = pwWnd.GetPassword();
		}

		QString Path = QFileDialog::getSaveFileName(this, tr("Select file name"), SandBoxes.first()->GetName() + optWnd.GetFormat(), tr("7-Zip Archive (*.7z);;Zip Archive (*.zip)"));
		if (Path.isEmpty())
			return;

		SB_PROGRESS Status = pBoxEx->ExportBox(Path, Password, optWnd.GetLevel(), optWnd.MakeSolid());
		if (Status.GetStatus() == OP_ASYNC)
			theGUI->AddAsyncOp(Status.GetValue(), false, tr("Exporting: %1").arg(Path));
		else
			Results.append(Status);
	}
	else if (Action == m_pMenuRename)
	{
		auto pBox = SandBoxes.first();
		QString OldValue = pBox->GetName().replace("_", " ");
		QString Alias = pBox->GetText("BoxAlias");
		bool bAlias = false;
		if (bAlias = !Alias.isEmpty())
			OldValue = Alias;
		bool bOk = false;
		QString Value = QInputDialog::getText(this, "Sandboxie-Plus", bAlias ? tr("Please enter a new alias for the Sandbox.") : tr("Please enter a new name for the Sandbox."), QLineEdit::Normal, OldValue, &bOk);
		if (!bOk || Value == OldValue)
			return;
		if (!Value.isEmpty() && !TestNameAndWarn(Value))
			return;

		bool bError = false;
		if (bAlias || (bError = CSbieAPI::ValidateName(QString(Value).replace(" ", "_")).IsError()))
		{
			if (!bAlias && QMessageBox::question(this, "Sandboxie-Plus", tr("The entered name is not valid, do you want to set it as an alias instead?"), QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes)
				return;
			if (Value.isEmpty()) pBox->DelValue("BoxAlias");
			else pBox->SetText("BoxAlias", Value);
			pBox->UpdateDetails();
		}
		else
		{
			SB_STATUS Status = pBox->RenameBox(Value.replace(" ", "_"));
			if (!Status.IsError())
			{
				RenameItem(OldValue.replace(" ", "_"), Value.replace(" ", "_"));
				if (theAPI->GetGlobalSettings()->GetText("DefaultBox", "DefaultBox").compare(OldValue.replace(" ", "_"), Qt::CaseInsensitive) == 0)
					theAPI->GetGlobalSettings()->SetText("DefaultBox", Value.replace(" ", "_"));
			}
			Results.append(Status);
		}

		SaveBoxGrouping();
	}
	else if (Action == m_pMenuMount)
	{
		Results.append(theGUI->ImBoxMount(SandBoxes.first()));
	}
	else if (Action == m_pMenuUnmount)
	{
		foreach(const CSandBoxPtr& pBox, SandBoxes) {
			auto pBoxEx = pBox.objectCast<CSandBoxPlus>();
			pBoxEx->TerminateAll();
			Results.append(pBox->ImBoxUnmount());
		}
	}
	else if (Action == m_pMenuRecover)
	{
		theGUI->ShowRecovery(SandBoxes.first());
	}
	else if (Action == m_pMenuRemove)
	{
		QString message = tr("Do you really want to remove the following sandbox(es)?<br /><br />%1<br /><br />Warning: The box content will also be deleted!")
			.arg(RenderSandboxNameList_(SandBoxes));
		if (QMessageBox("Sandboxie-Plus", message, QMessageBox::Warning, QMessageBox::Yes, QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton, this).exec() != QMessageBox::Yes)
			return;

		bool bChanged = false;
		foreach(const CSandBoxPtr& pBox, SandBoxes)
		{
			auto pBoxPlus = pBox.objectCast<CSandBoxPlus>();
			if (pBoxPlus->UseImageFile() && !pBoxPlus->GetMountRoot().isEmpty())
				pBoxPlus->ImBoxUnmount();

			SB_STATUS Status = SB_OK;

			if (!pBox->GetBool("IsShadow")) {
				if (pBox->GetBool("NeverRemove", false))
					Status = SB_ERR(SB_DeleteProtect);
				else {
					Status = theGUI->DeleteBoxContent(pBox, CSandMan::eForDelete);
					if (Status.GetMsgCode() == SB_Canceled)
						break;
				}
			}

			QString Name = pBox->GetName();
			if (!Status.IsError())
				Status = pBox->RemoveBox();

			if (!Status.IsError()) {
				theConf->DelValue("SizeCache/" + Name);
				m_Collapsed.remove(Name);
				for (auto I = m_Groups.begin(); I != m_Groups.end(); ++I)
				{
					if (I.value().removeOne(Name)) {
						bChanged = true;
						break;
					}
				}
			}

			Results.append(Status);
		}

		if(bChanged)
			SaveBoxGrouping();
	}
	else if (Action == m_pMenuCleanUp)
	{
		bool DeleteSnapshots = false;

		if (SandBoxes.count() == 1)
		{
			if (SandBoxes.first()->IsEmpty()) {
				QMessageBox("Sandboxie-Plus", tr("This Sandbox is already empty."), QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this).exec();
				return;
			}

			if (theConf->GetBool("Options/ShowRecovery", false))
			{
				// Use recovery dialog in place of the confirmation messagebox for box clean up
				if(!theGUI->OpenRecovery(SandBoxes.first(), DeleteSnapshots))
					return;
			}
			else
			{
				QString message = tr("Do you want to delete the content of the following sandbox?<br /><br />%1")
					.arg(RenderSandboxNameList_(SandBoxes));
				
				if (SandBoxes.first()->HasSnapshots())
				{
					if(CCheckableMessageBox::question(this, "Sandboxie-Plus", message
					, tr("Also delete all Snapshots"), &DeleteSnapshots, QDialogButtonBox::Yes | QDialogButtonBox::No, QDialogButtonBox::Yes) != QDialogButtonBox::Yes)
						return;
				}
				else
				{
					if(QMessageBox::question(this, "Sandboxie-Plus", message , QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes)
						return;
				}
			}
		}
		else
		{
			QString message = tr("Do you really want to delete the content of the following sandboxes?<br /><br />%1")
				.arg(RenderSandboxNameList_(SandBoxes));
			if(CCheckableMessageBox::question(this, "Sandboxie-Plus", message
				, tr("Also delete all Snapshots"), &DeleteSnapshots, QDialogButtonBox::Yes | QDialogButtonBox::No, QDialogButtonBox::Yes) != QDialogButtonBox::Yes)
				return;
		}

		foreach(const CSandBoxPtr& pBox, SandBoxes)
		{
			SB_STATUS Status = theGUI->DeleteBoxContent(pBox, CSandMan::eCleanUp, DeleteSnapshots);
			if (Status.GetMsgCode() == SB_Canceled)
				break;
			Results.append(Status);
		}	
	}
	else if (Action == m_pMenuEmptyBox)
	{
 		if (theConf->GetInt("Options/WarnTerminate", -1) == -1)
		{
			bool State = false;
			if(CCheckableMessageBox::question(this, "Sandboxie-Plus",  tr("Do you want to terminate all processes in the selected sandbox(es)?")
				, tr("Terminate without asking"), &State, QDialogButtonBox::Yes | QDialogButtonBox::No, QDialogButtonBox::Yes) != QDialogButtonBox::Yes)
				return;

			if (State)
				theConf->SetValue("Options/WarnTerminate", 1);
		}

		foreach(const CSandBoxPtr& pBox, SandBoxes)
			Results.append(pBox->TerminateAll());
	}
	else if (Action == m_pMenuMkLink)
	{
 		if (theConf->GetInt("Options/InfoMkLink", -1) == -1)
		{
			bool State = false;
			CCheckableMessageBox::question(this, "Sandboxie-Plus", tr("The Sandboxie Start Menu will now be displayed. Select an application from the menu, and Sandboxie will create a new "
				"shortcut icon on your real desktop, which you can use to invoke the selected application under the supervision of Sandboxie.")
				, tr("Don't show this message again."), &State, QDialogButtonBox::Ok, QDialogButtonBox::Ok, QMessageBox::Information);
			if (State)
				theConf->SetValue("Options/InfoMkLink", 1);
		}

		QString BoxName = SandBoxes.first()->GetName();
		QString LinkPath, IconPath, WorkDir;
		quint32 IconIndex;
		if (!CSbieUtils::GetStartMenuShortcut(theAPI, BoxName, LinkPath, IconPath, IconIndex, WorkDir))
			return;
		
		CreateShortcutEx(LinkPath, BoxName, "", IconPath, IconIndex, WorkDir);
	}
	else // custom run menu command
	{
		QString Command = Action->data().toString();
		QString WorkingDir = Action->property("WorkingDir").toString();
		if (Command.isEmpty())
			Results.append(theGUI->RunStart(SandBoxes.first()->GetName(), "start_menu", CSbieAPI::eStartDefault, WorkingDir));
		else {
			auto pBoxEx = SandBoxes.first().objectCast<CSandBoxPlus>();
			Results.append(theGUI->RunStart(SandBoxes.first()->GetName(), pBoxEx->GetFullCommand(Command), CSbieAPI::eStartDefault, pBoxEx->GetFullCommand(WorkingDir)));
		}
	}

	theGUI->CheckResults(Results, this);
}

bool CSbieView::CreateShortcutEx(const QString& LinkPath, const QString& BoxName, QString LinkName, const QString &IconPath, int IconIndex, const QString &WorkDir)
{
	if (LinkName.isEmpty()) {
		int pos = LinkPath.lastIndexOf(L'\\');
		if (pos == -1)
			return false;
		if (pos == 2 && LinkPath.length() == 3)
			LinkName = QObject::tr("Drive %1").arg(LinkPath.left(1));
		else {
			LinkName = LinkPath.mid(pos + 1);
			pos = LinkName.indexOf(QRegularExpression("[" + QRegularExpression::escape("\":;,*?.") + "]"));
			if (pos != -1)
				LinkName = LinkName.left(pos);
		}
	}

	QString Path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation).replace("/", "\\");
	//Path = QFileDialog::getExistingDirectory(this, tr("Select Directory to create Shortcut in"), Path).replace("/", "\\");
	//if (Path.isEmpty())
	//	return;

	if (Path.right(1) != "\\")
		Path.append("\\");
	Path += "[" + BoxName + "] " + LinkName;

	Path = QFileDialog::getSaveFileName(theGUI, tr("Create Shortcut to sandbox %1").arg(BoxName), Path, QString("Shortcut files (*.lnk)")).replace("/", "\\");
	if (Path.isEmpty())
		return false;

	QString StartExe = theAPI->GetSbiePath() + "\\SandMan.exe";
	return CSbieUtils::CreateShortcut(StartExe, Path, LinkName, BoxName, LinkPath, IconPath, IconIndex, WorkDir);
}

void CSbieView::OnProcessAction()
{
	OnProcessAction(qobject_cast<QAction*>(sender()), m_CurProcesses);
}

void CSbieView::OnProcessAction(QAction* Action, const QList<CBoxedProcessPtr>& Processes)
{
	QList<SB_STATUS> Results;

	if (Action == m_pMenuTerminate || Action == m_pMenuBlackList)
	{
 		if (theConf->GetInt("Options/WarnTerminate", -1) == -1)
		{
			if (Processes.isEmpty())
				return;

			bool State = false;
			if(CCheckableMessageBox::question(this, "Sandboxie-Plus", tr("Do you want to terminate %1?").arg(Processes.count() == 1 ? Processes[0]->GetProcessName() : tr("the selected processes"))
				, tr("Terminate without asking"), &State, QDialogButtonBox::Yes | QDialogButtonBox::No, QDialogButtonBox::Yes) != QDialogButtonBox::Yes)
				return;

			if (State)
				theConf->SetValue("Options/WarnTerminate", 1);
		}
	}

	foreach(const CBoxedProcessPtr& pProcess, Processes)
	{
		if (Action == m_pMenuTerminate)
			Results.append(pProcess->Terminate());
		else if (Action == m_pMenuLinkTo)
		{
			QString BoxName = pProcess->GetBoxName();
			QString LinkName = pProcess->GetProcessName();
			QString LinkPath = pProcess->GetFileName();
			QString WorkingDir = pProcess->GetWorkingDir();

			QString Path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation).replace("/", "\\");
			//Path = QFileDialog::getExistingDirectory(this, tr("Select Directory to create Shortcut in"), Path).replace("/", "\\");
			//if (Path.isEmpty())
			//	return;

			if (Path.right(1) != "\\")
				Path.append("\\");
			Path += "[" + BoxName + "] " + LinkName;

			Path = QFileDialog::getSaveFileName(this, tr("Create Shortcut to sandbox %1").arg(BoxName), Path, QString("Shortcut files (*.lnk)")).replace("/", "\\");
			if (Path.isEmpty())
				return;

			QString StartExe = theAPI->GetSbiePath() + "\\SandMan.exe";
			CSbieUtils::CreateShortcut(StartExe, Path, LinkName, BoxName, LinkPath, LinkPath, 0, WorkingDir);
		}
		else if (Action == m_pMenuPinToRun)
		{
			CSandBoxPlus* pBoxPlus = pProcess.objectCast<CSbieProcess>()->GetBox();
			if (m_pMenuPinToRun->isChecked())
			{
				QVariantMap Entry;
				Entry["Name"] = pProcess->GetProcessName();
				Entry["WorkingDir"] = pProcess->GetWorkingDir();
				Entry["Command"] = pBoxPlus->MakeBoxCommand(pProcess->GetFileName());
				pBoxPlus->AppendText("RunCommand", MakeRunEntry(Entry));
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
		else if (Action == m_pMenuSuspend)
			Results.append(pProcess->SetSuspended(true));
		else if (Action == m_pMenuResume)
			Results.append(pProcess->SetSuspended(false));
	}

	theGUI->CheckResults(Results, this);
}

void CSbieView::ShowOptions(const CSandBoxPtr& pBox)
{
	auto pBoxEx = pBox.objectCast<CSandBoxPlus>();
	if (pBoxEx->m_pOptionsWnd == NULL) {
		pBoxEx->m_pOptionsWnd = new COptionsWindow(pBox, pBoxEx->GetDisplayName());
		connect(theGUI, SIGNAL(Closed()), pBoxEx->m_pOptionsWnd, SLOT(close()));
		connect(pBoxEx->m_pOptionsWnd, &COptionsWindow::Closed, [pBoxEx]() {
			pBoxEx->m_pOptionsWnd = NULL;
		});
		CSandMan::SafeShow(pBoxEx->m_pOptionsWnd);
	}
	else {
		pBoxEx->m_pOptionsWnd->setWindowState((pBoxEx->m_pOptionsWnd->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
		SetForegroundWindow((HWND)pBoxEx->m_pOptionsWnd->winId());
	}
}

void CSbieView::ShowBrowse(const CSandBoxPtr& pBox)
{
	if (pBox->IsEmpty()) {
		QMessageBox("Sandboxie-Plus", tr("This Sandbox is empty."), QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this).exec();
		return;
	}

	static QMap<void*, CFileBrowserWindow*> FileBrowserWindows;
	CFileBrowserWindow* pFileBrowserWindow = FileBrowserWindows.value(pBox.data());
	if (pFileBrowserWindow == NULL) {
		pFileBrowserWindow = new CFileBrowserWindow(pBox);
		connect(theGUI, SIGNAL(Closed()), pFileBrowserWindow, SLOT(close()));
		FileBrowserWindows.insert(pBox.data(), pFileBrowserWindow);
		connect(pFileBrowserWindow, &CFileBrowserWindow::Closed, [this, pBox]() {
			FileBrowserWindows.remove(pBox.data());
		});
		CSandMan::SafeShow(pFileBrowserWindow);
	}
	else {
		pFileBrowserWindow->setWindowState((pFileBrowserWindow->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
		SetForegroundWindow((HWND)pFileBrowserWindow->winId());
	}
}

void CSbieView::OnDoubleClicked(const QModelIndex& index)
{
	QModelIndex ModelIndex = m_pSortProxy->mapToSource(index);
	CSandBoxPtr pBox = m_pSbieModel->GetSandBox(ModelIndex);

	if (index.column() == CSbieModel::ePath) {
		OnSandBoxAction(m_pMenuExplore, QList<CSandBoxPtr>() << pBox);
		return;
	}

	//if (index.column() != CSbieModel::eName)
	//	return;

	OnDoubleClicked(pBox);
}

void CSbieView::OnDoubleClicked(const CSandBoxPtr &pBox)
{
	if (pBox.isNull())
		return;

	if ((QGuiApplication::queryKeyboardModifiers() & Qt::ControlModifier) != 0) {
		ShowOptions(pBox);
		return;
	}


	if (!pBox->IsEnabled())
	{
		if (QMessageBox("Sandboxie-Plus", tr("This sandbox is currently disabled or restricted to specific groups or users. Would you like to allow access for everyone?"), QMessageBox::Question, QMessageBox::Yes, QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton, this).exec() != QMessageBox::Yes)
			pBox->SetText("Enabled", "y");// Fix #3999
	}
	
	QString Action = pBox->GetText("DblClickAction");
	if (Action.compare("!browse", Qt::CaseInsensitive) == 0)
		ShowBrowse(pBox);
	else if (Action.compare("!recovery", Qt::CaseInsensitive) == 0)
		theGUI->ShowRecovery(pBox);
	else if (Action.compare("!run", Qt::CaseInsensitive) == 0)
		pBox->RunStart("run_dialog");
	else if (!Action.isEmpty() && Action.left(1) != "!")
	{
		if (Action.left(1) == "\\")
			Action.prepend(pBox->GetFileRoot());
		pBox->RunStart(Action);
	}
	else
		ShowOptions(pBox);
}

void CSbieView::OnClicked(const QModelIndex& index)
{
	emit BoxSelected();
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

QMenu* CSbieView::GetMenuFolder(const QString& Folder, QMenu* pParent, QMap<QString, QMenu*>& Folders)
{
	QMenu* &pMenu = Folders[Folder];
	if (!pMenu)
	{
		QString Title;
		QStringList Names = Folder.split("/");
		StrPair Tmp = Split2(Folder, "/", true);
		if (!Tmp.second.isEmpty()) {
			pParent = GetMenuFolder(Tmp.first, pParent, Folders);
			Title = Tmp.second;
		}
		else
			Title = Tmp.first;

		pMenu = new CMenuEx(Title, pParent);
		pMenu->setIcon(m_IconProvider.icon(QFileIconProvider::Folder));
	    pParent->addAction(pMenu->menuAction());

		connect(pMenu, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnMenuContextMenu(const QPoint&)));
	}
	return pMenu;
}

void CSbieView::OnMenuContextMenu(const QPoint& point)
{
	QMenu* pMenu = (QMenu*)sender();
	QAction* pAction = pMenu->actionAt(point);
	if (!pAction) return;
	QString LinkTarget = pAction->data().toString();
	if (!LinkTarget.isEmpty()) {

		auto pBoxPlus = m_CurSandBoxes.first().objectCast<CSandBoxPlus>();
		QStringList RunOptions = pBoxPlus->GetTextList("RunCommand", true);

		QString FoundPin;
		QString Arguments;
		QString FileName = pBoxPlus->GetCommandFile(LinkTarget, &Arguments);
		foreach(const QString& RunOption, RunOptions) {
			QVariantMap Entry = GetRunEntry(RunOption);
			QString CurArgs;
			QString CmdFile = pBoxPlus->GetCommandFile(Entry["Command"].toString(), &CurArgs);
			if(CmdFile.compare(FileName, Qt::CaseInsensitive) == 0 && Arguments == CurArgs) {
				FoundPin = RunOption;
				break;
			}
		}

		m_pCtxPinToRun->setChecked(!FoundPin.isEmpty());
		if (FoundPin.isEmpty()) {
			QVariantMap Entry;
			Entry["Name"] = pAction->text();
			Entry["Icon"] = pAction->property("Icon").toString().replace(pBoxPlus->GetFileRoot(), "%BoxRoot%", Qt::CaseInsensitive) + "," + pAction->property("IconIndex").toString();
			Entry["WorkingDir"] = pBoxPlus->MakeBoxCommand(pAction->property("WorkingDir").toString());
			Entry["Command"] = pBoxPlus->MakeBoxCommand(LinkTarget);
			m_pCtxPinToRun->setData(MakeRunEntry(Entry));
		}
		else
			m_pCtxPinToRun->setData(FoundPin);

		m_pCtxMkLink->setData(pBoxPlus->GetFullCommand(LinkTarget));
		m_pCtxMkLink->setProperty("Name", pAction->text());
		m_pCtxMkLink->setProperty("Icon", pBoxPlus->GetFullCommand(pAction->property("Icon").toString()));
		m_pCtxMkLink->setProperty("IconIndex", pAction->property("IconIndex"));
		m_pCtxMkLink->setProperty("WorkingDir", pBoxPlus->GetFullCommand(pAction->property("WorkingDir").toString()));

		m_pCtxMenu->exec(QCursor::pos());
	}
}

void CSbieView::OnMenuContextAction()
{
	QAction* pAction = (QAction*)sender();

	auto pBoxPlus = m_CurSandBoxes.first().objectCast<CSandBoxPlus>();

	if (pAction == m_pCtxPinToRun)
	{
		QString Link = m_pCtxPinToRun->data().toString();
		if (!Link.isEmpty()) {
			if (m_pCtxPinToRun->isChecked())
				pBoxPlus->AppendText("RunCommand", Link);
			else
				pBoxPlus->DelValue("RunCommand", Link);
		}
	}
	else if (pAction == m_pCtxMkLink)
	{
		QString LinkTarget = m_pCtxMkLink->data().toString();
		QString LinkName = m_pCtxMkLink->property("Name").toString();
		QString Icon = m_pCtxMkLink->property("Icon").toString();
		int IconIndex = m_pCtxMkLink->property("IconIndex").toInt();
		QString WorkingDir = m_pCtxMkLink->property("WorkingDir").toString();
		QString BoxName = pBoxPlus->GetName();

		CreateShortcutEx(LinkTarget, BoxName, LinkName, Icon, IconIndex, WorkingDir);
	}
}

void CSbieView::UpdateStartMenu(CSandBoxPlus* pBoxEx)
{
	foreach(const CSandBoxPlus::SLink& Link, pBoxEx->GetStartMenu())
	{
		QMenu* pMenu = GetMenuFolder(Link.Folder, m_pMenuRunStart, m_MenuFolders);

		QAction* pAction = pMenu->addAction(Link.Name, this, SLOT(OnSandBoxAction()));
		QIcon Icon;
		if(Link.IconIndex == -1)
			Icon = theGUI->GetIcon("Internet");
		else if (!Link.Icon.isEmpty()) {
			if(QFile::exists(Link.Icon))
				Icon = LoadWindowsIcon(Link.Icon, Link.IconIndex);
			else 
				Icon = theGUI->GetIcon("File");
		}
		if (Icon.isNull()) Icon = m_IconProvider.icon(QFileInfo(Link.Target));
		pAction->setIcon(Icon);
		QString Command;
		if(Link.Target.contains(" "))
			Command = "\"" + Link.Target + "\"";
		else
			Command = Link.Target;
		if(!Link.Arguments.isEmpty())
			Command += " " + Link.Arguments;
		pAction->setData(Command);
		if(!Link.Icon.isEmpty()) pAction->setProperty("Icon", Link.Icon);
		pAction->setProperty("IconIndex", Link.IconIndex);
		pAction->setProperty("WorkingDir", Link.WorkDir);
	}
}

void CSbieView::UpdateRunMenu(const CSandBoxPtr& pBox)
{
	auto pBoxEx = pBox.objectCast<CSandBoxPlus>();

	while (m_iMenuRun < m_pMenuRun->actions().count())
		m_pMenuRun->removeAction(m_pMenuRun->actions().at(m_iMenuRun));
	while (!m_RunFolders.isEmpty())
		m_RunFolders.take(m_RunFolders.firstKey())->deleteLater();

	QStringList RunOptions = pBox->GetTextList("RunCommand", true, false, true);
	foreach(const QString& RunOption, RunOptions) 
	{
		QVariantMap Entry = GetRunEntry(RunOption);

		QMenu* pMenu;
		StrPair FolderName = Split2(Entry["Name"].toString(), "\\", true);
		if (FolderName.second.isEmpty()) {
			FolderName.second = FolderName.first;
			pMenu = m_pMenuRun;
		} else
			pMenu = GetMenuFolder(FolderName.first.replace("\\", "/"), m_pMenuRun, m_RunFolders);

		StrPair FileIndex = Split2(Entry["Icon"].toString(), ",", true);

		QString CmdFile = pBoxEx->GetCommandFile(Entry["Command"].toString());

		QString IconFile;
		int IconIndex ;
		if (FileIndex.second.isEmpty()) {
			IconFile = CmdFile;
			IconIndex = FileIndex.first.toInt();
		}
		else {
			if (FileIndex.first.isEmpty())
				IconFile = CmdFile;
			else
				IconFile = FileIndex.first.replace("%BoxRoot%", pBoxEx->GetFileRoot(), Qt::CaseInsensitive);
			IconIndex = FileIndex.second.toInt();
		}

		QAction* pAction = pMenu->addAction(FolderName.second, this, SLOT(OnSandBoxAction()));
		QIcon Icon;
		if(IconIndex == -1)
			Icon = theGUI->GetIcon("Internet");
		else if (!IconFile.isEmpty()) {
			if(QFile::exists(IconFile))
				Icon = LoadWindowsIcon(IconFile, IconIndex);
			else 
				Icon = theGUI->GetIcon("File");
		}
		if (Icon.isNull()) Icon = m_IconProvider.icon(QFileInfo(CmdFile));
		pAction->setIcon(Icon);
		pAction->setData(Entry["Command"].toString());
		pAction->setProperty("Icon", IconFile);
		pAction->setProperty("IconIndex", IconIndex);
		pAction->setProperty("WorkingDir", Entry["WorkingDir"]);
	}

	if (!m_pMenuRunStart)
		return;

	while (m_pMenuRunStart->actions().count() > 2)
		m_pMenuRunStart->removeAction(m_pMenuRunStart->actions().at(2));
	while (!m_MenuFolders.isEmpty())
		m_MenuFolders.take(m_MenuFolders.firstKey())->deleteLater();

	UpdateStartMenu(pBoxEx.data());
	
	if (m_pMenuRunStart->actions().count() > 2)
		m_pMenuRunMenu->setMenu(m_pMenuRunStart);
	else
		m_pMenuRunMenu->setMenu((QMenu*)NULL);
}

void CSbieView::SelectBox(const QString& Name)
{
	if(m_pSbieModel->Count() == 0)
		Refresh();

	QModelIndex Index = m_pSbieModel->FindIndex(Name);
	QModelIndex ModelIndex = m_pSortProxy->mapFromSource(Index);

	QModelIndex ModelL = m_pSortProxy->index(ModelIndex.row(), 0, ModelIndex.parent());
	QModelIndex ModelR = m_pSortProxy->index(ModelIndex.row(), m_pSortProxy->columnCount() - 1, ModelIndex.parent());

	QItemSelection SelectedItems;
	SelectedItems.append(QItemSelectionRange(ModelL, ModelR));

	m_pSbieTree->setCurrentIndex(ModelIndex);
	m_pSbieTree->scrollTo(ModelL);
	m_pSbieTree->selectionModel()->select(SelectedItems, QItemSelectionModel::ClearAndSelect);
}

void CSbieView::PopUpMenu(const QString& Name)
{
	//SelectBox(Name);
	CSandBoxPtr pBox = theAPI->GetBoxByName(Name);
	m_CurSandBoxes = QList<CSandBoxPtr>() << pBox;
	if (pBox.isNull() || !UpdateMenu(false, pBox)) return;
	m_pMenuTray->exec(QCursor::pos());
	//m_pMenuTray->popup(QCursor::pos());
	//OnMenu(QCursor::pos());
}

QMenu* CSbieView::GetMenu(const QString& Name)
{
	//SelectBox(Name);
	CSandBoxPtr pBox = theAPI->GetBoxByName(Name);
	m_CurSandBoxes = QList<CSandBoxPtr>() << pBox;
	if (pBox.isNull()) return NULL;
	UpdateMenu(false, pBox);
	return m_pMenuBox;
}

void CSbieView::ShowOptions(const QString& Name)
{
	QModelIndex Index = m_pSbieModel->FindIndex(Name);
	QModelIndex ModelIndex = m_pSortProxy->mapFromSource(Index);

	OnDoubleClicked(ModelIndex);
}

void CSbieView::ChangeExpand(const QModelIndex& index, bool bExpand)
{
	if (m_HoldExpand)
		return;

	QModelIndex ModelIndex = m_pSortProxy->mapToSource(index);

	if (m_pSbieModel->GetType(ModelIndex) == CSbieModel::eProcess)
		return;

	QString Name;
	if (m_pSbieModel->GetType(ModelIndex) == CSbieModel::eGroup)
		Name = m_pSbieModel->GetID(ModelIndex).toString();
	else if (m_pSbieModel->GetType(ModelIndex) == CSbieModel::eBox)
		Name = m_pSbieModel->GetSandBox(ModelIndex)->GetName();

	if(bExpand)
		m_Collapsed.remove(Name);
	else
		m_Collapsed.insert(Name);

	//QMap<QString, QStringList> Collapsed;
	//Collapsed.insert("", SetToList(m_Collapsed));
	//theAPI->GetUserSettings()->SetTextMap("CollapsedBoxes", Collapsed);

	QString Collapsed = SetToList(m_Collapsed).join(",");
	theConf->SetValue("UIConfig/BoxCollapsedView", Collapsed);
}

void CSbieView::UpdateColapsed()
{
	foreach(const QString& Group, m_Groups.keys())
	{
		if (!m_Collapsed.contains(Group)) {
			QModelIndex index = m_pSbieModel->FindGroupIndex(Group);
			if(index.isValid())
				m_pSbieTree->expand(m_pSortProxy->mapFromSource(index));
		}
	}
}

void CSbieView::ReloadUserConfig()
{
	if (!theAPI->IsConnected())
		return;

	m_Groups = theAPI->GetUserSettings()->GetTextMap("BoxGrouping");
	if (m_Groups.isEmpty()) { // try legacy entries
		QString Grouping = theConf->GetString("UIConfig/BoxDisplayOrder");
		if (Grouping.isEmpty())
			Grouping = theAPI->GetUserSettings()->GetText("BoxDisplayOrder");
		CSbieView__ParseGroup(Grouping, m_Groups);
	}

	UpdateMoveMenu();

	//QMap<QString, QStringList> Collapsed = theAPI->GetUserSettings()->GetTextMap("CollapsedBoxes");
	//m_Collapsed = ListToSet(Collapsed[""]);
	//if (m_Collapsed.isEmpty()) { // try legacy entries
		QString Collapsed = theConf->GetString("UIConfig/BoxCollapsedView");
		//if (Collapsed.isEmpty())
		//	Collapsed = theAPI->GetUserSettings()->GetText("BoxCollapsedView");
		m_Collapsed = ListToSet(SplitStr(Collapsed, ","));
	//}

	ClearUserUIConfig();
}

void CSbieView::ClearUserUIConfig(const QMap<QString, CSandBoxPtr> AllBoxes) 
{
	if (!AllBoxes.isEmpty())
	{
		for (auto I = m_Groups.begin(); I != m_Groups.end(); ++I) 
		{
			QStringList Temp = I.value();
			foreach(QString Name, I.value()) {
				if (AllBoxes.contains(Name.toLower()) || m_Groups.keys().contains(Name))
					continue;
				Temp.removeOne(Name);
			}
			I.value() = Temp;
		}
	}

	QSet<QString> Temp = m_Collapsed;
	foreach(QString Name, m_Collapsed)
	{
		if (m_Groups.end() == std::find_if(m_Groups.begin(), m_Groups.end(),
					  [Name](const QStringList& item)->int { return item.contains(Name); }))
			Temp.remove(Name);
	}
	m_Collapsed = Temp;
}

void CSbieView::SaveBoxGrouping()
{
	if (!theAPI->IsConnected())
		return;

	theAPI->GetUserSettings()->SetRefreshOnChange(false);

	auto Groups = m_Groups;
	// clean up non existing entries
	for (auto I = Groups.begin(); I != Groups.end(); ++I) {
		foreach(const QString &Name, I.value()) {
			if (theAPI->GetBoxByName(Name).isNull() && !Groups.contains(Name))
				I->removeAll(Name);
		}
	}
	theAPI->GetUserSettings()->SetTextMap("BoxGrouping", Groups);

	theAPI->GetUserSettings()->SetRefreshOnChange(true);
	theAPI->CommitIniChanges();
}

void CSbieView::OnMoveItem(const QString& Name, const QString& To, int row)
{
	QModelIndex index;
	if (!To.isEmpty()) { // only groups can be parents so add the group marker "!"
		QModelIndex index0 = m_pSbieModel->FindIndex("!" + To);
		index = m_pSbieModel->index(row, 0, index0);
	} else
		index = m_pSbieModel->index(row, 0);
	QModelIndex index2 = m_pSortProxy->mapFromSource(index);
	int row2 = index2.row();
	if (MoveItem(Name, To, row2)) {
		m_pSbieModel->Clear(); //todo improve that
		Refresh();
	}

	UpdateMoveMenu();

	SaveBoxGrouping();
}

void CSbieView::OnRemoveItem() 
{
	UpdateMenu();

	if (!m_CurProcesses.isEmpty())
		OnProcessAction(m_pMenuTerminate, m_CurProcesses);
	else if (!m_CurSandBoxes.isEmpty())
		OnSandBoxAction(m_pMenuRemove, m_CurSandBoxes);
	else if (!GetSelectedGroups().isEmpty())
		OnGroupAction(m_pDelGroupe);
}
