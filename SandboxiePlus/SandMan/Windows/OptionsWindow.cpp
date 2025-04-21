#include "stdafx.h"
#include "OptionsWindow.h"
#include "SandMan.h"
#include "SettingsWindow.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/Common.h"
#include "../MiscHelpers/Common/ComboInputDialog.h"
#include "../MiscHelpers/Common/SettingsWidgets.h"
#include "Helpers/WinAdmin.h"
#include "../Wizards/TemplateWizard.h"
#include "Helpers/TabOrder.h"
#include "../MiscHelpers/Common/CodeEdit.h"
#include "Helpers/IniHighlighter.h"


class NoEditDelegate : public QStyledItemDelegate {
public:
	NoEditDelegate(QObject* parent = 0) : QStyledItemDelegate(parent) {}

	virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
		return NULL;
	}
};

class QTreeWidgetHacker : public QTreeWidget
{
public:
	friend class ProgramsDelegate;
	//QModelIndex indexFromItem(const QTreeWidgetItem *item, int column = 0) const;
	//QTreeWidgetItem *itemFromIndex(const QModelIndex &index) const;
};


//////////////////////////////////////////////////////////////////////////
// ProgramsDelegate

class ProgramsDelegate : public QStyledItemDelegate {
public:
	ProgramsDelegate(COptionsWindow* pOptions, QTreeWidget* pTree, int Column, QObject* parent = 0) : QStyledItemDelegate(parent) {
		m_pOptions = pOptions; 
		m_pTree = pTree; 
		m_Column = (m_Group = (Column == -2)) ? -1 : Column;
	}

	virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
		QTreeWidgetItem* pItem = ((QTreeWidgetHacker*)m_pTree)->itemFromIndex(index);
		if (!pItem->data(index.column(), Qt::UserRole).isValid())
			return NULL;

		if(m_Group && !pItem->parent()) // for groups use simple edit
			return QStyledItemDelegate::createEditor(parent, option, index);

		if (m_Column == -1 || pItem->data(m_Column, Qt::UserRole).toInt() == COptionsWindow::eProcess) {
			QComboBox* pBox = new QComboBox(parent);
			pBox->setEditable(true);
			foreach(const QString Group, m_pOptions->GetCurrentGroups()) {
				QString GroupName = Group.mid(1, Group.length() - 2);
				pBox->addItem(tr("Group: %1").arg(GroupName), Group);
			}
			foreach(const QString & Name, m_pOptions->GetPrograms())
				pBox->addItem(Name, Name);

			connect(pBox->lineEdit(), &QLineEdit::textEdited, [pBox](const QString& text){
				/*if (pBox->currentIndex() != -1) {
					int pos = pBox->lineEdit()->cursorPosition();
					pBox->setCurrentIndex(-1);
					pBox->setCurrentText(text);
					pBox->lineEdit()->setCursorPosition(pos);
				}*/
				pBox->setProperty("value", text);
			});
			connect(pBox->lineEdit(), &QLineEdit::returnPressed, [pBox](){
				/*if (pBox->currentIndex() != -1) {
					int pos = pBox->lineEdit()->cursorPosition();
					pBox->setCurrentIndex(-1);
					pBox->setCurrentText(text);
					pBox->lineEdit()->setCursorPosition(pos);
				}*/
				pBox->setProperty("value", pBox->lineEdit()->text());
			});

			connect(pBox, qOverload<int>(&QComboBox::currentIndexChanged), [pBox](int index){
				if (index != -1) {
					QString Program = pBox->itemData(index).toString();
					pBox->setProperty("value", Program);
					pBox->lineEdit()->setReadOnly(Program.left(1) == "<");
				}
			});

			return pBox;
		}
		else if (pItem->data(0, Qt::UserRole).toInt() == COptionsWindow::ePath)
			return QStyledItemDelegate::createEditor(parent, option, index);
		else
			return NULL;
	}

	virtual void setEditorData(QWidget* editor, const QModelIndex& index) const {
		QComboBox* pBox = qobject_cast<QComboBox*>(editor);
		if (pBox) {
			QTreeWidgetItem* pItem = ((QTreeWidgetHacker*)m_pTree)->itemFromIndex(index);
			QString Program = pItem->data(index.column(), Qt::UserRole).toString();

			pBox->setProperty("value", Program);
			pBox->lineEdit()->setReadOnly(Program.left(1) == "<");

			int Index = pBox->findData(Program);
			pBox->setCurrentIndex(Index);
			if (Index == -1)
				pBox->setCurrentText(Program);
		}
		else
			QStyledItemDelegate::setEditorData(editor, index);
	}

	virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
		QTreeWidgetItem* pItem = ((QTreeWidgetHacker*)m_pTree)->itemFromIndex(index);

		QComboBox* pBox = qobject_cast<QComboBox*>(editor);
		if (pBox) {
			
			QString Value = pBox->property("value").toString();
			bool prev = m_pTree->blockSignals(true);
			pItem->setText(index.column(), pBox->currentText());
			m_pTree->blockSignals(prev);
			//QString Text = pBox->currentText();
			//QVariant Data = pBox->currentData();
			pItem->setData(index.column(), Qt::UserRole, Value);
		}

		QLineEdit* pEdit = qobject_cast<QLineEdit*>(editor);
		if (pEdit) {
			bool prev = m_pTree->blockSignals(true);
			pItem->setText(index.column(), pEdit->text());
			m_pTree->blockSignals(prev);
			QString Value = pEdit->text();
			if (m_Group) Value = "<" + Value + ">";
			pItem->setData(index.column(), Qt::UserRole, Value);
		}
	}

	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		QSize size = QStyledItemDelegate::sizeHint(option, index);
		if(size.height() < 20) size.setHeight(20); // ensure enough room for the combo box
		return size;
	}

protected:
	COptionsWindow* m_pOptions;
	QTreeWidget* m_pTree;
	int m_Column;
	bool m_Group;
};


//////////////////////////////////////////////////////////////////////////
// COptionsWindow

COptionsWindow::COptionsWindow(const QSharedPointer<CSbieIni>& pBox, const QString& Name, QWidget *parent)
	: CConfigDialog(parent)
{
	m_pBox = pBox;

	m_Template = pBox->GetName().left(9).compare("Template_", Qt::CaseInsensitive) == 0;
	bool ReadOnly = /*pBox->GetAPI()->IsConfigLocked() ||*/ (m_Template && pBox->GetName().mid(9, 6).compare("Local_", Qt::CaseInsensitive) != 0);
	
	m_HoldChange = false;

	m_ImageSize = 2ull*1024*1024*1024;

	QSharedPointer<CSandBoxPlus> pBoxPlus = m_pBox.objectCast<CSandBoxPlus>();
	if (!pBoxPlus.isNull())
		m_Programs = pBoxPlus->GetRecentPrograms();
	m_Programs.insert("program.exe");

	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::CustomizeWindowHint;
	//flags &= ~Qt::WindowContextHelpButtonHint;
	//flags &= ~Qt::WindowSystemMenuHint;
	//flags &= ~Qt::WindowMinMaxButtonsHint;
	flags |= Qt::WindowMinimizeButtonHint;
	//flags &= ~Qt::WindowCloseButtonHint;
	setWindowFlags(flags);

	this->setWindowFlag(Qt::WindowStaysOnTopHint, theGUI->IsAlwaysOnTop());

	ui.setupUi(this);
	this->setWindowTitle(tr("Sandboxie Plus - '%1' Options").arg(QString(Name).replace("_", " ")));

	ui.tabs->setTabPosition(QTabWidget::West);

	ui.tabs->setCurrentIndex(0);
	ui.tabs->setTabIcon(0, CSandMan::GetIcon("Config"));
	ui.tabs->setTabIcon(1, CSandMan::GetIcon("Security"));
	ui.tabs->setTabIcon(2, CSandMan::GetIcon("Group"));
	ui.tabs->setTabIcon(3, CSandMan::GetIcon("Control"));
	ui.tabs->setTabIcon(4, CSandMan::GetIcon("Stop"));
	ui.tabs->setTabIcon(5, CSandMan::GetIcon("Start"));
	ui.tabs->setTabIcon(6, CSandMan::GetIcon("Ampel"));
	ui.tabs->setTabIcon(7, CSandMan::GetIcon("Network"));
	ui.tabs->setTabIcon(8, CSandMan::GetIcon("Recover"));
	ui.tabs->setTabIcon(9, CSandMan::GetIcon("Settings"));
	ui.tabs->setTabIcon(10, CSandMan::GetIcon("Advanced"));
	ui.tabs->setTabIcon(11, CSandMan::GetIcon("Compatibility"));
	ui.tabs->setTabIcon(12, CSandMan::GetIcon("Editor"));

	ui.tabsGeneral->setCurrentIndex(0);
	ui.tabsGeneral->setTabIcon(0, CSandMan::GetIcon("Box"));
	ui.tabsGeneral->setTabIcon(1, CSandMan::GetIcon("Folder"));
	ui.tabsGeneral->setTabIcon(2, CSandMan::GetIcon("Move"));
	ui.tabsGeneral->setTabIcon(3, CSandMan::GetIcon("NoAccess"));
	ui.tabsGeneral->setTabIcon(4, CSandMan::GetIcon("EFence"));
	ui.tabsGeneral->setTabIcon(5, CSandMan::GetIcon("Run"));

	ui.tabsSecurity->setCurrentIndex(0);
	ui.tabsSecurity->setTabIcon(0, CSandMan::GetIcon("Shield7"));
	ui.tabsSecurity->setTabIcon(1, CSandMan::GetIcon("Fence"));
	ui.tabsSecurity->setTabIcon(2, CSandMan::GetIcon("Shield15"));
	ui.tabsSecurity->setTabIcon(3, CSandMan::GetIcon("Job"));
	ui.tabsSecurity->setTabIcon(4, CSandMan::GetIcon("Shield12"));

	ui.tabsForce->setCurrentIndex(0);
	ui.tabsForce->setTabIcon(0, CSandMan::GetIcon("Force"));
	ui.tabsForce->setTabIcon(1, CSandMan::GetIcon("Breakout"));

	ui.tabsStop->setCurrentIndex(0);
	ui.tabsStop->setTabIcon(0, CSandMan::GetIcon("Fail"));
	ui.tabsStop->setTabIcon(1, CSandMan::GetIcon("Pass"));
	ui.tabsStop->setTabIcon(2, CSandMan::GetIcon("Policy"));
		
	ui.tabsInternet->setCurrentIndex(0);
	ui.tabsInternet->setTabIcon(0, CSandMan::GetIcon("EthSocket2"));
	ui.tabsInternet->setTabIcon(1, CSandMan::GetIcon("Wall"));
	ui.tabsInternet->setTabIcon(2, CSandMan::GetIcon("DNS"));
	ui.tabsInternet->setTabIcon(3, CSandMan::GetIcon("Proxy"));
	ui.tabsInternet->setTabIcon(4, CSandMan::GetIcon("Network3"));

	ui.tabsAccess->setCurrentIndex(0);
	ui.tabsAccess->setTabIcon(0, CSandMan::GetIcon("Folder"));
	ui.tabsAccess->setTabIcon(1, CSandMan::GetIcon("RegEdit"));
	ui.tabsAccess->setTabIcon(2, CSandMan::GetIcon("Port"));
	ui.tabsAccess->setTabIcon(3, CSandMan::GetIcon("Window"));
	ui.tabsAccess->setTabIcon(4, CSandMan::GetIcon("Objects"));
	//ui.tabsAccess->setTabIcon(0, CSandMan::GetIcon("Rules"));
	ui.tabsAccess->setTabIcon(5, CSandMan::GetIcon("Policy"));

	ui.tabsRecovery->setCurrentIndex(0);
	ui.tabsRecovery->setTabIcon(0, CSandMan::GetIcon("QuickRecovery"));
	ui.tabsRecovery->setTabIcon(1, CSandMan::GetIcon("ImmidiateRecovery"));

	ui.tabsOther->setCurrentIndex(0);
	ui.tabsOther->setTabIcon(0, CSandMan::GetIcon("Presets"));
	ui.tabsOther->setTabIcon(1, CSandMan::GetIcon("Dll"));

	ui.tabsAdvanced->setCurrentIndex(0);
	ui.tabsAdvanced->setTabIcon(0, CSandMan::GetIcon("Presets"));
	ui.tabsAdvanced->setTabIcon(1, CSandMan::GetIcon("Trigger"));
	ui.tabsAdvanced->setTabIcon(2, CSandMan::GetIcon("Shield2"));
	ui.tabsAdvanced->setTabIcon(3, CSandMan::GetIcon("Anon"));
	ui.tabsAdvanced->setTabIcon(4, CSandMan::GetIcon("Users"));
	ui.tabsAdvanced->setTabIcon(5, CSandMan::GetIcon("SetLogging"));
	ui.tabsAdvanced->setTabIcon(6, CSandMan::GetIcon("Bug"));

	ui.tabsTemplates->setCurrentIndex(0);
	ui.tabsTemplates->setTabIcon(0, CSandMan::GetIcon("Template"));
	ui.tabsTemplates->setTabIcon(1, CSandMan::GetIcon("Explore"));
	ui.tabsTemplates->setTabIcon(2, CSandMan::GetIcon("Accessibility"));


	int iViewMode = theConf->GetInt("Options/ViewMode", 1);
	int iOptionLayout = theConf->GetInt("Options/NewConfigLayout", 2);
	if (iOptionLayout == 2)
		iOptionLayout = iViewMode != 2 ? 1 : 0;

	if ((QGuiApplication::queryKeyboardModifiers() & Qt::AltModifier) != 0)
		iOptionLayout = !iOptionLayout;

	QWidget* pDummy = new QWidget(this);
	pDummy->setVisible(false);

	// merge recovery tabs
	QWidget* pWidget3 = new QWidget();
	pWidget3->setLayout(ui.gridLayout_10);
	ui.gridLayout_24->addWidget(pWidget3, 1, 0);
	QWidget* pWidget4 = new QWidget();
	pWidget4->setLayout(ui.gridLayout_56);
	ui.gridLayout_24->addWidget(pWidget4, 2, 0);
	delete ui.tabsRecovery;
	ui.gridLayout_24->setContentsMargins(0, 0, 0, 0);

	// collect file options on a new files tab

	QWidget* pWidget = new QWidget();
	QGridLayout* pLayout = new QGridLayout(pWidget);

	QTabWidget* pTabWidget = new QTabWidget();
	pLayout->addWidget(pTabWidget, 0, 0);
	ui.tabs->insertTab(1, pWidget, tr("File Options"));
	ui.tabs->setTabIcon(1, CSandMan::GetIcon("Folder"));

	pTabWidget->addTab(ui.tabsGeneral->widget(1), ui.tabsGeneral->tabText(1));
	pTabWidget->setTabIcon(0, CSandMan::GetIcon("Files"));
	pTabWidget->addTab(ui.tabsGeneral->widget(1), ui.tabsGeneral->tabText(1));
	pTabWidget->setTabIcon(1, CSandMan::GetIcon("Move"));
	pTabWidget->addTab(ui.tabs->widget(9), ui.tabs->tabText(9));
	pTabWidget->setTabIcon(2, CSandMan::GetIcon("Recover"));
	//

	// re structure the UI a bit
	if (iOptionLayout == 1)
	{
		// merge stop tabs
		QWidget* pWidget1 = new QWidget();
		pWidget1->setLayout(ui.gridLayout_57);
		ui.gridLayout_17->addWidget(pWidget1, 1, 0);
		QWidget* pWidget2 = new QWidget();
		pWidget2->setLayout(ui.gridLayout_61);
		ui.gridLayout_17->addWidget(pWidget2, 2, 0);
		QWidget* pWidget3 = new QWidget();
		pWidget3->setLayout(ui.gridLayout_82);
		ui.gridLayout_82->setContentsMargins(3, 3, 3, 3);
		ui.verticalSpacer_40->changeSize(0, 0);
		ui.lblStopOpt->setVisible(false);
		ui.lblStopOpt->setProperty("hidden", true);
		ui.gridLayout_17->addWidget(pWidget3, 3, 0);
		delete ui.tabsStop;
		ui.gridLayout_17->setContentsMargins(0, 0, 0, 0);

		// move stop and restrictions to program tab
		ui.tabsForce->addTab(ui.tabs->widget(5), ui.tabs->tabText(5));
		ui.tabsForce->setTabIcon(2, CSandMan::GetIcon("Stop"));
		ui.tabsForce->addTab(ui.tabs->widget(5), ui.tabs->tabText(5));
		ui.tabsForce->setTabIcon(3, CSandMan::GetIcon("Start"));
		ui.gridLayout_19->setContentsMargins(3, 6, 3, 3);

		// move grouping to program tab
		ui.tabsForce->insertTab(0, ui.tabGroups, tr("Grouping"));
		ui.tabsForce->setTabIcon(0, CSandMan::GetIcon("Group"));
		ui.tabsForce->setCurrentIndex(0);
		ui.gridLayout_18->setContentsMargins(3, 6, 3, 3);
	}

	if (iViewMode != 1 && (QGuiApplication::queryKeyboardModifiers() & Qt::ControlModifier) == 0)
	{
		if (iOptionLayout == 1) {
			//ui.tabs->removeTab(7); // ini edit
			ui.tabAdvanced->setParent(pDummy); //ui.tabs->removeTab(5); // advanced
			//ui.tabsForce->removeTab(2); // breakout
		}
		else {
			//ui.tabs->removeTab(11); // ini edit
			ui.tabAdvanced->setParent(pDummy); //ui.tabs->removeTab(9); // advanced
			//ui.tabsForce->removeTab(1); // breakout
		}
		ui.tabPrivileges->setParent(pDummy); //ui.tabsSecurity->removeTab(3); // advanced security
		ui.tabIsolation->setParent(pDummy); //ui.tabsSecurity->removeTab(1); // security isolation
		//ui.tabsAccess->removeTab(5); // policy
		ui.treeOptions = NULL;
	}

	foreach(QTreeWidget* pTree, this->findChildren<QTreeWidget*>()) {
		if (pTree == ui.treeFolders) continue;
		pTree->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
		pTree->setMinimumHeight(50);
	}

	int size = 16.0;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	size *= (QApplication::desktop()->logicalDpiX() / 96.0); // todo Qt6
#endif
	AddIconToLabel(ui.lblAppearance, CSandMan::GetIcon("Design").pixmap(size,size));
	AddIconToLabel(ui.lblBoxType, CSandMan::GetIcon("Maintenance").pixmap(size,size));
	AddIconToLabel(ui.lblNotes, CSandMan::GetIcon("EditIni").pixmap(size,size));
	AddIconToLabel(ui.lblStructure, CSandMan::GetIcon("Structure").pixmap(size,size));
	AddIconToLabel(ui.lblMigration, CSandMan::GetIcon("Move").pixmap(size,size));
	AddIconToLabel(ui.lblDelete, CSandMan::GetIcon("Erase").pixmap(size,size));
	AddIconToLabel(ui.lblRawDisk, CSandMan::GetIcon("Disk").pixmap(size,size));
	AddIconToLabel(ui.lblJob, CSandMan::GetIcon("Job3").pixmap(size,size));
	AddIconToLabel(ui.lblLimit, CSandMan::GetIcon("Job2").pixmap(size,size));
	AddIconToLabel(ui.lblSecurity, CSandMan::GetIcon("Shield5").pixmap(size,size));
	AddIconToLabel(ui.lblElevation, CSandMan::GetIcon("Shield9").pixmap(size,size));
	AddIconToLabel(ui.lblACLs, CSandMan::GetIcon("Ampel").pixmap(size,size));
	AddIconToLabel(ui.lblBoxProtection, CSandMan::GetIcon("BoxConfig").pixmap(size,size));
	AddIconToLabel(ui.lblNetwork, CSandMan::GetIcon("Network").pixmap(size,size));
	AddIconToLabel(ui.lblBind, CSandMan::GetIcon("EthSocket2").pixmap(size,size));
	AddIconToLabel(ui.lblPrinting, CSandMan::GetIcon("Printer").pixmap(size,size));
	AddIconToLabel(ui.lblOther, CSandMan::GetIcon("NoAccess").pixmap(size,size));

	AddIconToLabel(ui.lblStopOpt, CSandMan::GetIcon("Stop").pixmap(size,size));

	AddIconToLabel(ui.lblPorts, CSandMan::GetIcon("Port").pixmap(size,size));

	AddIconToLabel(ui.lblMode, CSandMan::GetIcon("Anon").pixmap(size,size));
	AddIconToLabel(ui.lblPolicy, CSandMan::GetIcon("Policy").pixmap(size,size));

	AddIconToLabel(ui.lblCompatibility, CSandMan::GetIcon("Compatibility").pixmap(size,size));
	//AddIconToLabel(ui.lblComRpc, CSandMan::GetIcon("Objects").pixmap(size,size));

	AddIconToLabel(ui.lblPrivilege, CSandMan::GetIcon("Token").pixmap(size,size));
	AddIconToLabel(ui.lblToken, CSandMan::GetIcon("Sandbox").pixmap(size,size));
	AddIconToLabel(ui.lblIsolation, CSandMan::GetIcon("Fence").pixmap(size,size));
	AddIconToLabel(ui.lblDesktop, CSandMan::GetIcon("Monitor").pixmap(size,size));
	AddIconToLabel(ui.lblAccess, CSandMan::GetIcon("NoAccess").pixmap(size,size));
	AddIconToLabel(ui.lblProtection, CSandMan::GetIcon("EFence").pixmap(size,size));

	AddIconToLabel(ui.lblPrivacyProtection, CSandMan::GetIcon("Anon").pixmap(size,size));
	AddIconToLabel(ui.lblProcessHiding, CSandMan::GetIcon("Cmd").pixmap(size,size));

	AddIconToLabel(ui.lblMonitor, CSandMan::GetIcon("Monitor").pixmap(size,size));
	AddIconToLabel(ui.lblTracing, CSandMan::GetIcon("SetLogging").pixmap(size,size));


	if (theConf->GetBool("Options/AltRowColors", false)) {
		foreach(QTreeWidget* pTree, this->findChildren<QTreeWidget*>()) 
			pTree->setAlternatingRowColors(true);
	}

	m_pCodeEdit = new CCodeEdit(new CIniHighlighter(theGUI->m_DarkTheme));
	m_pCodeEdit->installEventFilter(this);
	ui.txtIniSection->parentWidget()->layout()->replaceWidget(ui.txtIniSection, m_pCodeEdit);
	delete ui.txtIniSection;
	ui.txtIniSection = NULL;
	connect(m_pCodeEdit, SIGNAL(textChanged()), this, SLOT(OnIniChanged()));

	CreateDebug();

	if (m_Template)
	{
		//ui.tabGeneral->setEnabled(false);
		//ui.tabStart->setEnabled(false);
		//ui.tabInternet->setEnabled(false);
		//ui.tabAdvanced->setEnabled(false);
		//ui.tabOther->setEnabled(false);
		//ui.tabTemplates->setEnabled(false);
		//
		//for (int i = 0; i < ui.tabs->count(); i++) 
		//	ui.tabs->setTabEnabled(i, ui.tabs->widget(i)->isEnabled());

		//ui.tabs->setCurrentIndex(ui.tabs->indexOf(ui.tabAccess));

		ui.chkShowGroupTmpl->setEnabled(false);
		ui.chkShowForceTmpl->setEnabled(false);
		ui.chkShowBreakoutTmpl->setEnabled(false);
		ui.chkShowStopTmpl->setEnabled(false);
		ui.chkShowLeaderTmpl->setEnabled(false);
		ui.chkShowStartTmpl->setEnabled(false);
		ui.chkShowFilesTmpl->setEnabled(false);
		ui.chkShowKeysTmpl->setEnabled(false);
		ui.chkShowIPCTmpl->setEnabled(false);
		ui.chkShowWndTmpl->setEnabled(false);
		ui.chkShowCOMTmpl->setEnabled(false);
		ui.chkShowNetFwTmpl->setEnabled(false);
		ui.chkShowRecoveryTmpl->setEnabled(false);
		ui.chkShowRecIgnoreTmpl->setEnabled(false);
		ui.chkShowTriggersTmpl->setEnabled(false);
		ui.chkShowHiddenProcTmpl->setEnabled(false);
		ui.chkShowHostProcTmpl->setEnabled(false);
		ui.chkShowOptionsTmpl->setEnabled(false);

		//ui.chkWithTemplates->setEnabled(false);
	}

	ui.tabs->setCurrentIndex(m_Template ? ui.tabs->count()-1 : 0);
	if(m_Template)
		OnTab();

	//connect(ui.chkWithTemplates, SIGNAL(clicked(bool)), this, SLOT(OnWithTemplates()));

	m_ConfigDirty = true;

	CreateGeneral();

	// Groups
	connect(ui.btnAddGroup, SIGNAL(clicked(bool)), this, SLOT(OnAddGroup()));
	connect(ui.btnAddProg, SIGNAL(clicked(bool)), this, SLOT(OnAddProg()));
	connect(ui.btnDelProg, SIGNAL(clicked(bool)), this, SLOT(OnDelProg()));
	connect(ui.chkShowGroupTmpl, SIGNAL(clicked(bool)), this, SLOT(OnShowGroupTmpl()));
	ui.treeGroups->setItemDelegateForColumn(0, new ProgramsDelegate(this, ui.treeGroups, -2, this));
	connect(ui.treeGroups, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(OnGroupsChanged(QTreeWidgetItem *, int)));
	//

	// Force
	connect(ui.btnForceProg, SIGNAL(clicked(bool)), this, SLOT(OnForceProg()));
	QMenu* pFileBtnMenu = new QMenu(ui.btnForceProg);
	pFileBtnMenu->addAction(tr("Browse for File"), this, SLOT(OnForceBrowseProg()));
	ui.btnForceProg->setPopupMode(QToolButton::MenuButtonPopup);
	ui.btnForceProg->setMenu(pFileBtnMenu);

	connect(ui.btnForceChild, SIGNAL(clicked(bool)), this, SLOT(OnForceChild()));
	pFileBtnMenu = new QMenu(ui.btnForceChild);
	pFileBtnMenu->addAction(tr("Browse for File"), this, SLOT(OnForceBrowseChild()));
	ui.btnForceChild->setPopupMode(QToolButton::MenuButtonPopup);
	ui.btnForceChild->setMenu(pFileBtnMenu);

	connect(ui.btnForceDir, SIGNAL(clicked(bool)), this, SLOT(OnForceDir()));
	connect(ui.btnDelForce, SIGNAL(clicked(bool)), this, SLOT(OnDelForce()));
	connect(ui.chkShowForceTmpl, SIGNAL(clicked(bool)), this, SLOT(OnShowForceTmpl()));
	//ui.treeForced->setEditTriggers(QAbstractItemView::DoubleClicked);
	ui.treeForced->setItemDelegateForColumn(0, new NoEditDelegate(this));
	ui.treeForced->setItemDelegateForColumn(1, new ProgramsDelegate(this, ui.treeForced, -1, this));
	connect(ui.treeForced, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(OnForcedChanged(QTreeWidgetItem *, int)));
	connect(ui.chkDisableForced, SIGNAL(clicked(bool)), this, SLOT(OnForcedChanged()));

	connect(ui.btnBreakoutProg, SIGNAL(clicked(bool)), this, SLOT(OnBreakoutProg()));
	QMenu* pFileBtnMenu2 = new QMenu(ui.btnBreakoutProg);
	pFileBtnMenu2->addAction(tr("Browse for File"), this, SLOT(OnBreakoutBrowse()));
	ui.btnBreakoutProg->setPopupMode(QToolButton::MenuButtonPopup);
	ui.btnBreakoutProg->setMenu(pFileBtnMenu2);
	connect(ui.btnBreakoutDir, SIGNAL(clicked(bool)), this, SLOT(OnBreakoutDir()));
	connect(ui.btnBreakoutDoc, SIGNAL(clicked(bool)), this, SLOT(OnBreakoutDoc()));
	connect(ui.btnDelBreakout, SIGNAL(clicked(bool)), this, SLOT(OnDelBreakout()));
	connect(ui.chkShowBreakoutTmpl, SIGNAL(clicked(bool)), this, SLOT(OnShowBreakoutTmpl()));
	//ui.treeBreakout->setEditTriggers(QAbstractItemView::DoubleClicked);
	ui.treeBreakout->setItemDelegateForColumn(0, new NoEditDelegate(this));
	ui.treeBreakout->setItemDelegateForColumn(1, new ProgramsDelegate(this, ui.treeBreakout, -1, this));
	connect(ui.treeBreakout, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(OnBreakoutChanged(QTreeWidgetItem *, int)));
	//

	// Stop
	connect(ui.btnAddLingering, SIGNAL(clicked(bool)), this, SLOT(OnAddLingering()));
	connect(ui.btnDelStopProg, SIGNAL(clicked(bool)), this, SLOT(OnDelStopProg()));
	connect(ui.chkShowStopTmpl, SIGNAL(clicked(bool)), this, SLOT(OnShowStopTmpl()));
	ui.treeStop->setItemDelegateForColumn(0, new ProgramsDelegate(this, ui.treeStop, -1, this));
	connect(ui.treeStop, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(OnStopChanged()));

	connect(ui.btnAddLeader, SIGNAL(clicked(bool)), this, SLOT(OnAddLeader()));
	connect(ui.btnDelLeader, SIGNAL(clicked(bool)), this, SLOT(OnDelLeader()));
	connect(ui.chkShowLeaderTmpl, SIGNAL(clicked(bool)), this, SLOT(OnShowLeaderTmpl()));
	ui.treeLeader->setItemDelegateForColumn(0, new ProgramsDelegate(this, ui.treeLeader, -1, this));
	connect(ui.treeLeader, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(OnStopChanged()));

	connect(ui.chkNoStopWnd, SIGNAL(clicked(bool)), this, SLOT(OnStopChanged()));
	connect(ui.chkLingerLeniency, SIGNAL(clicked(bool)), this, SLOT(OnStopChanged()));
	//

	// Start
	connect(ui.radStartAll, SIGNAL(clicked(bool)), this, SLOT(OnRestrictStart()));
	connect(ui.radStartExcept, SIGNAL(clicked(bool)), this, SLOT(OnRestrictStart()));
	connect(ui.radStartSelected, SIGNAL(clicked(bool)), this, SLOT(OnRestrictStart()));
	connect(ui.btnAddStartProg, SIGNAL(clicked(bool)), this, SLOT(OnAddStartProg()));
	connect(ui.btnDelStartProg, SIGNAL(clicked(bool)), this, SLOT(OnDelStartProg()));
	//connect(ui.chkShowStartTmpl, SIGNAL(clicked(bool)), this, SLOT(OnShowStartTmpl()));
	ui.chkShowStartTmpl->setVisible(false); // todo
	connect(ui.chkStartBlockMsg, SIGNAL(clicked(bool)), this, SLOT(OnStartChanged()));
	ui.treeStart->setItemDelegateForColumn(0, new ProgramsDelegate(this, ui.treeStart, -1, this));
	connect(ui.treeStart, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(OnStartChanged(QTreeWidgetItem *, int)));
	connect(ui.chkAlertBeforeStart, SIGNAL(clicked(bool)), this, SLOT(OnStartChanged()));
	//

	CreateNetwork();

	CreateAccess();

	// Recovery
	connect(ui.chkAutoRecovery, SIGNAL(clicked(bool)), this, SLOT(OnRecoveryChanged()));
	connect(ui.btnAddRecovery, SIGNAL(clicked(bool)), this, SLOT(OnAddRecFolder()));
	connect(ui.btnDelRecovery, SIGNAL(clicked(bool)), this, SLOT(OnDelRecEntry()));
	connect(ui.btnAddRecIgnore, SIGNAL(clicked(bool)), this, SLOT(OnAddRecIgnore()));
	connect(ui.btnAddRecIgnoreExt, SIGNAL(clicked(bool)), this, SLOT(OnAddRecIgnoreExt()));
	connect(ui.btnDelRecIgnore, SIGNAL(clicked(bool)), this, SLOT(OnDelRecIgnoreEntry()));
	connect(ui.chkShowRecoveryTmpl, SIGNAL(clicked(bool)), this, SLOT(OnShowRecoveryTmpl()));
	connect(ui.chkShowRecIgnoreTmpl, SIGNAL(clicked(bool)), this, SLOT(OnShowRecIgnoreTmpl()));
	//

	CreateAdvanced();

	// Templates
	connect(ui.cmbCategories, SIGNAL(currentIndexChanged(int)), this, SLOT(OnFilterTemplates()));
	connect(ui.txtTemplates, SIGNAL(textChanged(const QString&)), this, SLOT(OnFilterTemplates()));
	//connect(ui.treeTemplates, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(OnTemplateClicked(QTreeWidgetItem*, int)));
	connect(ui.treeTemplates, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(OnTemplateClicked(QTreeWidgetItem*, int)));
	connect(ui.treeTemplates, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnTemplateDoubleClicked(QTreeWidgetItem*, int)));
	connect(ui.btnAddTemplate, SIGNAL(clicked(bool)), this, SLOT(OnAddTemplates()));
	QMenu* pTmplBtnMenu = new QMenu(ui.btnAddTemplate);
	for(int i = 1; i < CTemplateWizard::TmplMax; i++)
		pTmplBtnMenu->addAction(tr("Add %1 Template").arg(CTemplateWizard::GetTemplateLabel((CTemplateWizard::ETemplateType)i)), this, SLOT(OnTemplateWizard()))->setData(i);
	ui.btnAddTemplate->setPopupMode(QToolButton::MenuButtonPopup);
	ui.btnAddTemplate->setMenu(pTmplBtnMenu);
	connect(ui.btnOpenTemplate, SIGNAL(clicked(bool)), this, SLOT(OnOpenTemplate()));
	connect(ui.btnDelTemplate, SIGNAL(clicked(bool)), this, SLOT(OnDelTemplates()));
	connect(ui.chkScreenReaders, SIGNAL(clicked(bool)), this, SLOT(OnScreenReaders()));
	//

	connect(ui.tabs, SIGNAL(currentChanged(int)), this, SLOT(OnTab()));

	// edit
	ApplyIniEditFont();

	connect(ui.btnEditIni, SIGNAL(clicked(bool)), this, SLOT(OnEditIni()));
	connect(ui.btnSaveIni, SIGNAL(clicked(bool)), this, SLOT(OnSaveIni()));
	connect(ui.btnCancelEdit, SIGNAL(clicked(bool)), this, SLOT(OnCancelEdit()));
	//connect(ui.txtIniSection, SIGNAL(textChanged()), this, SLOT(OnIniChanged()));

	connect(ui.buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked(bool)), this, SLOT(ok()));
	connect(ui.buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked(bool)), this, SLOT(apply()));
	connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(close()));

	if (ReadOnly)
	{
		ui.btnEditIni->setEnabled(false);
		ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	}

	if (theAPI->IsRunningAsAdmin())
	{
		ui.chkDropRights->setEnabled(false);
		ui.chkFakeElevation->setEnabled(false);
	}
	else
		ui.lblAdmin->setVisible(false);

	LoadConfig();

	m_pCurrentTab = ui.tabGeneral;
	UpdateCurrentTab();

	ui.buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
	
	ui.treeCopy->viewport()->installEventFilter(this);
	ui.treeINet->viewport()->installEventFilter(this);
	ui.treeNetFw->viewport()->installEventFilter(this);
	ui.treeFiles->viewport()->installEventFilter(this);
	ui.treeKeys->viewport()->installEventFilter(this);
	ui.treeIPC->viewport()->installEventFilter(this);
	ui.treeWnd->viewport()->installEventFilter(this);
	ui.treeCOM->viewport()->installEventFilter(this);
	//ui.treeAccess->viewport()->installEventFilter(this);
	if(ui.treeOptions) ui.treeOptions->viewport()->installEventFilter(this);
	this->installEventFilter(this); // prevent enter from closing the dialog

	restoreGeometry(theConf->GetBlob("OptionsWindow/Window_Geometry"));

	foreach(QTreeWidget * pTree, this->findChildren<QTreeWidget*>()) {
		QByteArray Columns = theConf->GetBlob("OptionsWindow/" + pTree->objectName() + "_Columns");
		if (!Columns.isEmpty()) 
			pTree->header()->restoreState(Columns);
	}

	if (theAPI->GetGlobalSettings()->GetBool("EditAdminOnly", false) && !IsAdminUser())
	{
		for (int I = 0; I < ui.tabs->count(); I++) {
			QGridLayout* pGrid = qobject_cast<QGridLayout*>(ui.tabs->widget(I)->layout());
			QTabWidget* pSubTabs = pGrid ? qobject_cast<QTabWidget*>(pGrid->itemAt(0)->widget()) : NULL;
			if (!pSubTabs) {
				ui.tabs->widget(I)->setEnabled(false);
			}
			else {
				for (int J = 0; J < pSubTabs->count(); J++) {
					pSubTabs->widget(J)->setEnabled(false);
				}
			}
		}
	}

	int iOptionTree = theConf->GetInt("Options/OptionTree", 2);
	if (iOptionTree == 2)
		iOptionTree = iViewMode == 2 ? 1 : 0;

	if (iOptionTree) {
		m_HoldChange = true;
		OnSetTree();
		m_HoldChange = false;
	}
	else {
		//this->setMinimumHeight(390);

		QWidget* pSearch = AddConfigSearch(ui.tabs);
		ui.horizontalLayout->insertWidget(0, pSearch);
		QTimer::singleShot(0, [this]() {
			m_pSearch->setMaximumWidth(m_pTabs->tabBar()->width());
		});

		QAction* pSetTree = new QAction();
		connect(pSetTree, SIGNAL(triggered()), this, SLOT(OnSetTree()));
		pSetTree->setShortcut(QKeySequence("Ctrl+Alt+T"));
		pSetTree->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		this->addAction(pSetTree);
	}
	m_pSearch->setPlaceholderText(tr("Search for options"));
	
	SetTabOrder(this);
}

void COptionsWindow::ApplyIniEditFont()
{
	QFont font; // defaults to application font
	auto fontName = theConf->GetString("UIConfig/IniFont", "").trimmed();
	if (!fontName.isEmpty()) {
		font.fromString(fontName); // ignore fromString() fail
		//ui.txtIniSection->setFont(font);
		m_pCodeEdit->SetFont(font);
	}
}

void COptionsWindow::OnSetTree()
{
	if (!ui.tabs) return;
	QWidget* pAltView = ConvertToTree(ui.tabs);
	ui.verticalLayout->replaceWidget(ui.tabs, pAltView);
	ui.tabs->deleteLater();
	ui.tabs = NULL;
}

void COptionsWindow::OnOptChanged() 
{
	if (m_HoldChange)
		return;
	ui.buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

COptionsWindow::~COptionsWindow()
{
	theConf->SetBlob("OptionsWindow/Window_Geometry",saveGeometry());

	foreach(QTreeWidget * pTree, this->findChildren<QTreeWidget*>()) 
		theConf->SetBlob("OptionsWindow/" + pTree->objectName() + "_Columns", pTree->header()->saveState());
}

void COptionsWindow::closeEvent(QCloseEvent *e)
{
	emit Closed();
	this->deleteLater();
}

bool COptionsWindow::eventFilter(QObject *source, QEvent *event)
{
	if (event->type() == QEvent::KeyPress && ((QKeyEvent*)event)->key() == Qt::Key_Escape 
		&& ((QKeyEvent*)event)->modifiers() == Qt::NoModifier
		&& source == m_pCodeEdit)
	{
		return true; // cancel event
	}

	if (event->type() == QEvent::KeyPress && ((QKeyEvent*)event)->key() == Qt::Key_Escape 
		&& ((QKeyEvent*)event)->modifiers() == Qt::NoModifier
		&& (source == ui.treeCopy->viewport()
			|| source == ui.treeINet->viewport() || source == ui.treeNetFw->viewport() 
			// || source == ui.treeAccess->viewport()
			|| source == ui.treeFiles->viewport() || source == ui.treeKeys->viewport() || source == ui.treeIPC->viewport() || source == ui.treeWnd->viewport() || source == ui.treeCOM->viewport() 
			|| (ui.treeOptions && source == ui.treeOptions->viewport())))
	{
		CloseCopyEdit(false);
		CloseINetEdit(false);
		CloseNetFwEdit(false);
		CloseAccessEdit(false);
		CloseOptionEdit(false);
        CloseNetProxyEdit(false);
		return true; // cancel event
	}

	if (event->type() == QEvent::KeyPress && (((QKeyEvent*)event)->key() == Qt::Key_Enter || ((QKeyEvent*)event)->key() == Qt::Key_Return) 
		&& (((QKeyEvent*)event)->modifiers() == Qt::NoModifier || ((QKeyEvent*)event)->modifiers() == Qt::KeypadModifier))
	{
		CloseCopyEdit(true);
		CloseINetEdit(true);
		CloseNetFwEdit(true);
		CloseAccessEdit(true);
		CloseOptionEdit(true);
        CloseNetProxyEdit(true);
		return true; // cancel event
	}
	
	if (source == ui.treeCopy->viewport() && event->type() == QEvent::MouseButtonPress)
	{
		CloseCopyEdit();
	}

	if (source == ui.treeINet->viewport() && event->type() == QEvent::MouseButtonPress)
	{
		CloseINetEdit();
	}

	if (source == ui.treeNetFw->viewport() && event->type() == QEvent::MouseButtonPress)
	{
		CloseNetFwEdit();
	}

    if (source == ui.treeProxy->viewport() && event->type() == QEvent::MouseButtonPress)
	{
		CloseNetProxyEdit();
	}

	if (//source == ui.treeAccess->viewport() 
		(source == ui.treeFiles->viewport() || source == ui.treeKeys->viewport() || source == ui.treeIPC->viewport() || source == ui.treeWnd->viewport() || source == ui.treeCOM->viewport())
		&& event->type() == QEvent::MouseButtonPress)
	{
		CloseAccessEdit();
	}


	if ((ui.treeOptions && source == ui.treeOptions->viewport()) && event->type() == QEvent::MouseButtonPress)
	{
		CloseOptionEdit();
	}

	return QDialog::eventFilter(source, event);
}

//void COptionsWindow::OnWithTemplates()
//{
//	m_Template = ui.chkWithTemplates->isChecked();
//	ui.buttonBox->setEnabled(!m_Template);
//	LoadConfig();
//}

void COptionsWindow::ReadAdvancedCheck(const QString& Name, QCheckBox* pCheck, const QString& Value)
{
	QString Data = m_pBox->GetText(Name);
	if (Data == Value)			pCheck->setCheckState(Qt::Checked);
	else if (Data.isEmpty())	pCheck->setCheckState(Qt::Unchecked);
	else						pCheck->setCheckState(Qt::PartiallyChecked);
}

int COptionsWindow__GetBoolConfig(const QString& Value)
{
	QString temp = Value.left(1).toLower();
	if (temp == "y")
		return 1;
	else if (temp == "n")
		return 0;
	return -1;
}

void COptionsWindow::ReadGlobalCheck(QCheckBox* pCheck, const QString& Setting, bool bDefault)
{
	int iLocal = COptionsWindow__GetBoolConfig(m_pBox->GetText(Setting));
	int iTemplate = COptionsWindow__GetBoolConfig(m_pBox->GetText(Setting, QString(), false, true, true));
	int iGlobal = COptionsWindow__GetBoolConfig(m_pBox->GetText(Setting, QString(), true));

	bool bTemplate = m_pBox->GetBool(Setting, bDefault, true, true);
	if (iLocal != -1) 
		pCheck->setChecked(iLocal == 1);
	else
		pCheck->setChecked(iTemplate != -1 ? iTemplate == 1 : iGlobal != -1 ? iGlobal == 1 : bDefault);
	QStringList Info;
	Info.append(tr("Box: %1").arg(iLocal == 1 ? "y" : "n"));
	if (iTemplate != -1)	Info.append(tr("Template: %1").arg(iTemplate == 1 ? "y" : "n"));
	if (iGlobal != -1)		Info.append(tr("Global: %1").arg(iGlobal == 1 ? "y" : "n"));
	Info.append(tr("Default: %1").arg(bDefault ? "y" : "n"));
	pCheck->setToolTip(Info.join("\r\n"));
}

void COptionsWindow::WriteGlobalCheck(QCheckBox* pCheck, const QString& Setting, bool bDefault)
{
	bool bLocal = pCheck->isChecked();
	bool bPreset = m_pBox->GetBool(Setting, bDefault, true, true);
	SB_STATUS Status;
	if(bPreset == bLocal)
		Status = m_pBox->DelValue(Setting);
	else 
		Status = m_pBox->SetText(Setting, bLocal ? "y" : "n");

	if (!Status)
		throw Status;
}

void COptionsWindow::LoadConfig()
{
	m_ConfigDirty = false;

	m_HoldChange = true;

	LoadTemplates();

	LoadGeneral();

	LoadGroups();

	LoadForced();

	LoadStop();

	LoadStart();

	LoadINetAccess();
	LoadNetFwRules();
	LoadDnsFilter();
	LoadNetProxy();
	LoadNetwork();

	LoadAccessList();

	LoadRecoveryList();

	LoadAdvanced();
	LoadDebug();
	
	UpdateBoxType();

	m_HoldChange = false;
}

void COptionsWindow::WriteAdvancedCheck(QCheckBox* pCheck, const QString& Name, const QString& Value)
{
	SB_STATUS Status;
	if (pCheck->checkState() == Qt::Checked)		
		Status = m_pBox->SetText(Name, Value);
	else if (pCheck->checkState() == Qt::Unchecked) 
		Status = m_pBox->DelValue(Name);
	
	if (!Status)
		throw Status;
}

void COptionsWindow::WriteAdvancedCheck(QCheckBox* pCheck, const QString& Name, const QString& OnValue, const QString& OffValue)
{
	//if (pCheck->checkState() == Qt::PartiallyChecked)
	//	return;

	if (!pCheck->isEnabled())
		return;

	QString StrValue;
	if (pCheck->checkState() == Qt::Checked)
	{
		if (!OnValue.isEmpty())
			StrValue = OnValue;
	}
	else if (pCheck->checkState() == Qt::Unchecked)
	{
		if (!OffValue.isEmpty())
			StrValue = OffValue;
	}

	QStringList Values = m_pBox->GetTextList(Name, false);
	foreach(const QString & CurValue, Values) {
		if (CurValue.contains(","))
			continue;
		if (!StrValue.isEmpty() && CurValue == StrValue)
			StrValue.clear();
		else
			m_pBox->DelValue(Name, CurValue);
	}

	if (!StrValue.isEmpty()) {
		SB_STATUS Status = m_pBox->AppendText(Name, StrValue);
		if (!Status)
			throw Status;
	}
}

void COptionsWindow::WriteText(const QString& Name, const QString& Value)
{
	SB_STATUS Status = m_pBox->SetText(Name, Value);
	if (!Status)
		throw Status;
}

void COptionsWindow::WriteTextList(const QString& Setting, const QStringList& List)
{
	SB_STATUS Status = m_pBox->UpdateTextList(Setting, List, m_Template);
	if (!Status)
		throw Status;
}

void COptionsWindow::WriteTextSafe(const QString& Name, const QString& Value)
{
	QStringList List = m_pBox->GetTextList(Name, false);

	// clear all non per process (name=program.exe,value) entries 
	for (int i = 0; i < List.count(); i++) {
		if (!List[i].contains(","))
			List.removeAt(i--);
	}

	// Prepend the global entry
	if (!Value.isEmpty()) List.append(Value);

	WriteTextList(Name, List);
}

QString COptionsWindow::ReadTextSafe(const QString& Name, const QString& Default)
{
	QStringList List = m_pBox->GetTextList(Name, false);

	for (int i = 0; i < List.count(); i++) {
		if (!List[i].contains(","))
			return List[i];
	}

	return Default;
}

void COptionsWindow::SaveConfig()
{
	bool UpdatePaths = false;

	m_pBox->SetRefreshOnChange(false);

	try
	{
		if (m_GeneralChanged)
			SaveGeneral();
		if (m_CopyRulesChanged)
			SaveCopyRules();

		if (m_GroupsChanged)
			SaveGroups();

		if (m_ForcedChanged)
			SaveForced();

		if (m_StopChanged)
			SaveStop();

		if (m_StartChanged)
			SaveStart();

		if (m_INetBlockChanged)
			SaveINetAccess();
		if (m_NetFwRulesChanged)
			SaveNetFwRules();
		if (m_DnsFilterChanged)
			SaveDnsFilter();
		if (m_NetProxyChanged)
			SaveNetProxy();
		if (m_NetworkChanged)
			SaveNetwork();

		if (m_AccessChanged) {
			SaveAccessList();
			UpdatePaths = true;
		}

		if (m_RecoveryChanged)
			SaveRecoveryList();

		if (m_AdvancedChanged)
			SaveAdvanced();
		SaveDebug();

		if (m_TemplatesChanged)
			SaveTemplates();

		if (m_FoldersChanged)
			SaveFolders();
	}
	catch (SB_STATUS Status)
	{
		theGUI->CheckResults(QList<SB_STATUS>() << Status, theGUI);
	}

	m_pBox->SetRefreshOnChange(true);
	m_pBox->GetAPI()->CommitIniChanges();

	if (UpdatePaths)
		TriggerPathReload();
}

bool COptionsWindow::apply()
{
	if (m_pBox->GetText("Enabled").isEmpty() && !(m_Template && m_pBox->GetName().mid(9, 6).compare("Local_", Qt::CaseInsensitive) == 0)) {
		QMessageBox::critical(this, "Sandboxie-Plus", tr("This sandbox has been deleted hence configuration can not be saved."));
		return false;
	}

	CloseINetEdit();
	CloseNetFwEdit();
	CloseAccessEdit();
	CloseOptionEdit();
    CloseNetProxyEdit();

	if (!ui.btnEditIni->isEnabled())
		SaveIniSection();
	else
	{
		if (m_GeneralChanged) {
			auto pBoxEx = m_pBox.objectCast<CSandBoxPlus>();
			if (ui.chkEncrypt->isChecked() && !QFile::exists(pBoxEx->GetBoxImagePath())) {
				if (m_Password.isEmpty())
					OnSetPassword();
				if (m_Password.isEmpty())
					return false;
				pBoxEx->ImBoxCreate(m_ImageSize / 1024, m_Password);
			}
		}

		SaveConfig();
	}

	LoadConfig();

	UpdateCurrentTab();

	//emit OptionsChanged();

	ui.buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);

	return true;
}

void COptionsWindow::ok()
{
	if(apply())
		close();
}

void COptionsWindow::reject()
{
	if (m_GeneralChanged
	 || m_CopyRulesChanged
	 || m_GroupsChanged
	 || m_ForcedChanged
	 || m_StopChanged
	 || m_StartChanged
	// ||  m_RestrictionChanged
	 || m_INetBlockChanged
	 || m_NetFwRulesChanged
	 || m_DnsFilterChanged
	 || m_NetProxyChanged
	 || m_AccessChanged
	 || m_TemplatesChanged
	 || m_FoldersChanged
	 || m_RecoveryChanged
	 || m_AdvancedChanged)
	{
		if (QMessageBox("Sandboxie-Plus", tr("Some changes haven't been saved yet, do you really want to close this options window?")
		, QMessageBox::Warning, QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape, QMessageBox::NoButton, this).exec() != QMessageBox::Yes)
			return;
	}

	this->close();
}

void COptionsWindow::showTab(const QString& Name)
{
	QWidget* pWidget = this->findChild<QWidget*>("tab" + Name);

	if (ui.tabs) {
		for (int i = 0; i < ui.tabs->count(); i++) {
			QGridLayout* pGrid = qobject_cast<QGridLayout*>(ui.tabs->widget(i)->layout());
			QTabWidget* pSubTabs = pGrid ? qobject_cast<QTabWidget*>(pGrid->itemAt(0)->widget()) : NULL;
			if(ui.tabs->widget(i) == pWidget)
				ui.tabs->setCurrentIndex(i);
			else if(pSubTabs) {
				for (int j = 0; j < pSubTabs->count(); j++) {
					if (pSubTabs->widget(j) == pWidget) {
						ui.tabs->setCurrentIndex(i);
						pSubTabs->setCurrentIndex(j);
					}
				}
			}
		}
	} 
	else
		m_pStack->setCurrentWidget(pWidget);

	CSandMan::SafeShow(this);
}

void COptionsWindow::SetProgramItem(QString Program, QTreeWidgetItem* pItem, int Column, const QString& Suffix, bool bList)
{
	pItem->setData(Column, Qt::UserRole, Program);
	if (Program.left(1) == "<")
		Program = tr("Group: %1").arg(Program.mid(1, Program.length() - 2));
	else if (Program.isEmpty() || Program == "*")
		Program = tr("All Programs");
	else if(bList)
		m_Programs.insert(Program);
	pItem->setText(Column, Program + Suffix);
}

QString COptionsWindow::SelectProgram(bool bOrGroup)
{
	CComboInputDialog progDialog(this);
	progDialog.setText(tr("Enter program:"));
	progDialog.setEditable(true);

	if (bOrGroup)
	{
		foreach(const QString Group, GetCurrentGroups()){
			QString GroupName = Group.mid(1, Group.length() - 2);
			progDialog.addItem(tr("Group: %1").arg(GroupName), Group);
		}
	}

	foreach(const QString & Name, m_Programs)
		progDialog.addItem(Name, Name);

	progDialog.setValue("");

	if (!progDialog.exec())
		return QString();

	// Note: pressing enter adds the value to the combo list !
	QString Program = progDialog.value(); 
	int Index = progDialog.findValue(Program);
	if (Index != -1 && progDialog.data().isValid())
		Program = progDialog.data().toString();

	return Program;
}

void COptionsWindow::OnTab(QWidget* pTab)
{
	m_pCurrentTab = pTab;

	if (pTab == ui.tabEdit)
	{
		LoadIniSection();
		//ui.txtIniSection->setReadOnly(true);
	}
	else 
	{
		if (m_ConfigDirty)
			LoadConfig();

		UpdateCurrentTab();
	}
}

void COptionsWindow::UpdateCurrentTab()
{
	if (m_pCurrentTab == ui.tabRestrictions || m_pCurrentTab == ui.tabOptions || m_pCurrentTab == ui.tabGeneral) 
	{
		ui.chkVmRead->setChecked(IsAccessEntrySet(eIPC, "", eReadOnly, "$:*"));
	}
	else if (m_pCurrentTab == ui.tabStart || m_pCurrentTab == ui.tabForce)
	{
		if (IsAccessEntrySet(eIPC, "!<StartRunAccess>", eClosed, "*"))
			ui.radStartSelected->setChecked(true);
		else if (IsAccessEntrySet(eIPC, "<StartRunAccess>", eClosed, "*"))
			ui.radStartExcept->setChecked(true);
		else
			ui.radStartAll->setChecked(true);
		ui.treeStart->clear();
		CopyGroupToList("<StartRunAccess>", ui.treeStart);
		CopyGroupToList("<StartRunAccessDisabled>", ui.treeStart, true);

		OnRestrictStart();
	}
	else if (m_pCurrentTab == ui.tabInternet || m_pCurrentTab == ui.tabINet || m_pCurrentTab == ui.tabNetConfig)
	{
		LoadBlockINet();
	}
	else if (m_pCurrentTab == ui.tabDNS || m_pCurrentTab == ui.tabNetProxy)
	{
		if (!m_HoldChange && !m_pCurrentTab->isEnabled())
			theGUI->CheckCertificate(this, 2);
	}
	else if (m_pCurrentTab == ui.tabCOM) {
		CheckOpenCOM();
	}
	else if (m_pCurrentTab == ui.tabWnd)
	{
		if (IsAccessEntrySet(eWnd, "", eOpen, "*"))
		{
			ui.chkNoWindowRename->setEnabled(false);
			ui.chkNoWindowRename->setChecked(true);
		}
		else
		{
			ui.chkNoWindowRename->setEnabled(true);
			ui.chkNoWindowRename->setChecked(IsAccessEntrySet(eWnd, "", eOpen, "#"));
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Raw section ini Editor
//

void COptionsWindow::SetIniEdit(bool bEnable)
{
	if (m_pTree) {
		m_pTree->setEnabled(!bEnable);
	}
	else {
		for (int i = 0; i < ui.tabs->count() - 1; i++) {
			bool Enabled = ui.tabs->widget(i)->isEnabled();
			ui.tabs->setTabEnabled(i, !bEnable && Enabled);
			ui.tabs->widget(i)->setEnabled(Enabled);
		}
	}
	ui.btnSaveIni->setEnabled(bEnable);
	ui.btnCancelEdit->setEnabled(bEnable);
	//ui.txtIniSection->setReadOnly(!bEnable);
	ui.btnEditIni->setEnabled(!bEnable);
}

void COptionsWindow::OnEditIni()
{
	SetIniEdit(true);
}

void COptionsWindow::OnSaveIni()
{
	SaveIniSection();
	SetIniEdit(false);
}

void COptionsWindow::OnIniChanged()
{
	if (m_HoldChange)
		return;
	if(ui.btnEditIni->isEnabled())
		SetIniEdit(true);
	ui.buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void COptionsWindow::OnCancelEdit()
{
	SetIniEdit(false);
	LoadIniSection();
}

void COptionsWindow::LoadIniSection()
{
	QString Section;

	// Note: the service only caches sandboxie.ini not templates.ini, hence for global templates we need to load the section through the driver
	if (m_Template && m_pBox->GetName().mid(9, 6).compare("Local_", Qt::CaseInsensitive) != 0)
	{
		m_Settings = m_pBox->GetIniSection(NULL, m_Template);

		for (QList<QPair<QString, QString>>::const_iterator I = m_Settings.begin(); I != m_Settings.end(); ++I)
			Section += I->first + "=" + I->second + "\n";
	}
	else
		Section = m_pBox->GetAPI()->SbieIniGetEx(m_pBox->GetName(), "");

	m_HoldChange = true;
	//ui.txtIniSection->setPlainText(Section);
	m_pCodeEdit->SetCode(Section);
	m_HoldChange = false;
}

void COptionsWindow::SaveIniSection()
{
	m_ConfigDirty = true;

	/*m_pBox->SetRefreshOnChange(false);

	// Note: an incremental update would be more elegant but it would change the entry order in the ini,
	//			hence it's better for the user to fully rebuild the section each time.
	//
	for (QList<QPair<QString, QString>>::const_iterator I = m_Settings.begin(); I != m_Settings.end(); ++I)
		m_pBox->DelValue(I->first, I->second);

	//QList<QPair<QString, QString>> NewSettings;
	//QList<QPair<QString, QString>> OldSettings = m_Settings;

	QStringList Section = SplitStr(ui.txtIniSection->toPlainText(), "\n");
	foreach(const QString& Line, Section)
	{
		if (Line.isEmpty())
			return;
		StrPair Settings = Split2(Line, "=");
		
		//if (!OldSettings.removeOne(Settings))
		//	NewSettings.append(Settings);

		m_pBox->AppendText(Settings.first, Settings.second);
	}

	//for (QList<QPair<QString, QString>>::const_iterator I = OldSettings.begin(); I != OldSettings.end(); ++I)
	//	m_pBox->DelValue(I->first, I->second);
	//
	//for (QList<QPair<QString, QString>>::const_iterator I = NewSettings.begin(); I != NewSettings.end(); ++I)
	//	m_pBox->AppendText(I->first, I->second);

	m_pBox->SetRefreshOnChange(true);
	m_pBox->GetAPI()->CommitIniChanges();*/

	//m_pBox->GetAPI()->SbieIniSet(m_pBox->GetName(), "", ui.txtIniSection->toPlainText());
	m_pBox->GetAPI()->SbieIniSet(m_pBox->GetName(), "", m_pCodeEdit->GetCode());

	//LoadIniSection();
}

#include "OptionsAccess.cpp"
#include "OptionsAdvanced.cpp"
#include "OptionsForce.cpp"
#include "OptionsGeneral.cpp"
#include "OptionsGrouping.cpp"
#include "OptionsNetwork.cpp"
#include "OptionsRecovery.cpp"
#include "OptionsStart.cpp"
#include "OptionsStop.cpp"
#include "OptionsTemplates.cpp"

#include <windows.h>

void COptionsWindow::TriggerPathReload()
{
	//
	// this message makes all boxes reload their path presets
	//

	DWORD bsm_app = BSM_APPLICATIONS;
	BroadcastSystemMessage(BSF_POSTMESSAGE, &bsm_app, WM_DEVICECHANGE, 'sb', 0);
}
