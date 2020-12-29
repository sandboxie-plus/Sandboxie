#include "stdafx.h"
#include "OptionsWindow.h"
#include "SandMan.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/Common.h"
#include "../MiscHelpers/Common/ComboInputDialog.h"
#include "Helpers/WinAdmin.h"
#include <QProxyStyle>

class CustomTabStyle : public QProxyStyle {
public:
	CustomTabStyle(QStyle* style = 0) : QProxyStyle(style) {}

	QSize sizeFromContents(ContentsType type, const QStyleOption* option,
		const QSize& size, const QWidget* widget) const {
		QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
		if (type == QStyle::CT_TabBarTab) {
			s.transpose();
			if(theConf->GetBool("Options/DarkTheme", false))
				s.setHeight(s.height() * 13 / 10);
			else
				s.setHeight(s.height() * 15 / 10);
			s.setWidth(s.width() * 11 / 10); // for the the icon
		}
		return s;
	}

	void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
		if (element == CE_TabBarTabLabel) {
			if (const QStyleOptionTab* tab = qstyleoption_cast<const QStyleOptionTab*>(option)) {
				QStyleOptionTab opt(*tab);
				opt.shape = QTabBar::RoundedNorth;
				//opt.iconSize = QSize(32, 32);
				opt.iconSize = QSize(24, 24);
				QProxyStyle::drawControl(element, &opt, painter, widget);
				return;
			}
		}
		QProxyStyle::drawControl(element, option, painter, widget);
	}
};


COptionsWindow::COptionsWindow(const QSharedPointer<CSbieIni>& pBox, const QString& Name, QWidget *parent)
	: QMainWindow(parent)
{
	m_pBox = pBox;

	m_Template = pBox->GetName().left(9).compare("Template_", Qt::CaseInsensitive) == 0;
	bool ReadOnly = /*pBox->GetAPI()->IsConfigLocked() ||*/ (m_Template && pBox->GetName().mid(9, 6).compare("Local_", Qt::CaseInsensitive) != 0);
	
	QSharedPointer<CSandBoxPlus> pBoxPlus = m_pBox.objectCast<CSandBoxPlus>();
	if (!pBoxPlus.isNull())
		m_Programs = pBoxPlus->GetRecentPrograms();

	QWidget* centralWidget = new QWidget();
	ui.setupUi(centralWidget);
	this->setCentralWidget(centralWidget);
	this->setWindowTitle(tr("Sandboxie Plus - '%1' Options").arg(Name));

	ui.tabs->setTabPosition(QTabWidget::West);
	ui.tabs->tabBar()->setStyle(new CustomTabStyle(ui.tabs->tabBar()->style()));

	QStringList DebugOptions = theConf->ListKeys("DebugOptions");
	if(DebugOptions.isEmpty())
		ui.tabsAdvanced->removeTab(ui.tabsAdvanced->count() - 1);
	else
	{
		int RowCount = 0;
		foreach(const QString& DebugOption, DebugOptions)
		{
			QStringList ValueDescr = theConf->GetString("DebugOptions/" + DebugOption).split("|");

			QString Description = ValueDescr.size() >= 3 ? ValueDescr[2] : ValueDescr[0];
			int Column = 0; // use - to add up to 10 indents
			for (; Description[0] == "-" && Column < 10; Column++) Description.remove(0, 1);

			SDbgOpt DbgOption = { ValueDescr[0], ValueDescr.size() >= 2 ? ValueDescr[1] : "y" , false};

			QString Info = DbgOption.Name + "=" + DbgOption.Value;
			QCheckBox* pCheck = new QCheckBox(tr("%1 (%2)").arg(Description).arg(Info));
			//pCheck->setToolTip(Info);
			ui.dbgLayout->addWidget(pCheck, RowCount++, Column, 1, 10-Column);

			connect(pCheck, SIGNAL(clicked(bool)), this, SLOT(OnDebugChanged()));
			m_DebugOptions.insert(pCheck, DbgOption);
		}

		for(int i=0; i < 10; i++)
			ui.dbgLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding), RowCount, i);
		ui.dbgLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), RowCount, 10);
	}

	if (m_Template)
	{
		ui.tabGeneral->setEnabled(false);
		ui.tabStart->setEnabled(false);
		ui.tabInternet->setEnabled(false);
		ui.tabAdvanced->setEnabled(false);
		ui.tabTemplates->setEnabled(false);

		for (int i = 0; i < ui.tabs->count(); i++) 
			ui.tabs->setTabEnabled(i, ui.tabs->widget(i)->isEnabled());

		ui.tabs->setCurrentIndex(ui.tabs->indexOf(ui.tabAccess));

		ui.chkShowForceTmpl->setEnabled(false);
		ui.chkShowStopTmpl->setEnabled(false);
		ui.chkShowAccessTmpl->setEnabled(false);

		//ui.chkWithTemplates->setEnabled(false);
	}

	ui.tabs->setTabIcon(0, CSandMan::GetIcon("Box"));
	ui.tabs->setTabIcon(1, CSandMan::GetIcon("Group"));
	ui.tabs->setTabIcon(2, CSandMan::GetIcon("Force"));
	ui.tabs->setTabIcon(3, CSandMan::GetIcon("Stop"));
	ui.tabs->setTabIcon(4, CSandMan::GetIcon("Start"));
	ui.tabs->setTabIcon(5, CSandMan::GetIcon("Internet"));
	ui.tabs->setTabIcon(6, CSandMan::GetIcon("Wall"));
	ui.tabs->setTabIcon(7, CSandMan::GetIcon("Recover"));
	ui.tabs->setTabIcon(8, CSandMan::GetIcon("Advanced"));
	ui.tabs->setTabIcon(9, CSandMan::GetIcon("Template"));
	ui.tabs->setTabIcon(10, CSandMan::GetIcon("EditIni"));

	ui.tabs->setCurrentIndex(0);

	//connect(ui.chkWithTemplates, SIGNAL(clicked(bool)), this, SLOT(OnWithTemplates()));

	m_ConfigDirty = true;

	// General
	ui.cmbBoxIndicator->addItem(tr("Don't alter the window title"), "-");
	ui.cmbBoxIndicator->addItem(tr("Display [#] indicator only"), "n");
	ui.cmbBoxIndicator->addItem(tr("Display box name in title"), "y");

	ui.cmbBoxBorder->addItem(tr("Border disabled"), "off");
	ui.cmbBoxBorder->addItem(tr("Show only when title is in focus"), "ttl");
	ui.cmbBoxBorder->addItem(tr("Always show"), "on");

	connect(ui.cmbBoxIndicator, SIGNAL(currentIndexChanged(int)), this, SLOT(OnGeneralChanged()));
	connect(ui.cmbBoxBorder, SIGNAL(currentIndexChanged(int)), this, SLOT(OnGeneralChanged()));
	connect(ui.btnBorderColor, SIGNAL(pressed()), this, SLOT(OnPickColor()));
	connect(ui.spinBorderWidth, SIGNAL(valueChanged(int)), this, SLOT(OnGeneralChanged()));

	connect(ui.chkBlockNetShare, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkBlockNetParam, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkDropRights, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));

	connect(ui.txtCopyLimit, SIGNAL(textChanged(const QString&)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkCopyLimit, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkNoCopyWarn, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));

	connect(ui.chkProtectBox, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkAutoEmpty, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));

	connect(ui.btnAddExe, SIGNAL(clicked(bool)), this, SLOT(OnBrowsePath()));
	connect(ui.btnAddCmd, SIGNAL(clicked(bool)), this, SLOT(OnAddCommand()));
	connect(ui.btnDelCmd, SIGNAL(clicked(bool)), this, SLOT(OnDelCommand()));
	//

	// Groupes
	connect(ui.btnAddGroup, SIGNAL(pressed()), this, SLOT(OnAddGroup()));
	connect(ui.btnAddProg, SIGNAL(pressed()), this, SLOT(OnAddProg()));
	connect(ui.btnDelProg, SIGNAL(pressed()), this, SLOT(OnDelProg()));
	//

	// Force
	connect(ui.btnForceProg, SIGNAL(pressed()), this, SLOT(OnForceProg()));
	connect(ui.btnForceDir, SIGNAL(pressed()), this, SLOT(OnForceDir()));
	connect(ui.btnDelForce, SIGNAL(pressed()), this, SLOT(OnDelForce()));
	connect(ui.chkShowForceTmpl, SIGNAL(clicked(bool)), this, SLOT(OnShowForceTmpl()));
	//

	// Stop
	connect(ui.btnAddLingering, SIGNAL(pressed()), this, SLOT(OnAddLingering()));
	connect(ui.btnAddLeader, SIGNAL(pressed()), this, SLOT(OnAddLeader()));
	connect(ui.btnDelStopProg, SIGNAL(pressed()), this, SLOT(OnDelStopProg()));
	connect(ui.chkShowStopTmpl, SIGNAL(clicked(bool)), this, SLOT(OnShowStopTmpl()));
	//

	// Start
	connect(ui.radStartAll, SIGNAL(clicked(bool)), this, SLOT(OnRestrictStart()));
	connect(ui.radStartExcept, SIGNAL(clicked(bool)), this, SLOT(OnRestrictStart()));
	connect(ui.radStartSelected, SIGNAL(clicked(bool)), this, SLOT(OnRestrictStart()));
	connect(ui.btnAddStartProg, SIGNAL(pressed()), this, SLOT(OnAddStartProg()));
	connect(ui.btnDelStartProg, SIGNAL(pressed()), this, SLOT(OnDelStartProg()));
	connect(ui.chkStartBlockMsg, SIGNAL(clicked(bool)), this, SLOT(OnStartChanged()));
	//

	// INet
	connect(ui.chkBlockINet, SIGNAL(clicked(bool)), this, SLOT(OnBlockINet()));
	connect(ui.btnAddINetProg, SIGNAL(pressed()), this, SLOT(OnAddINetProg()));
	connect(ui.btnDelINetProg, SIGNAL(pressed()), this, SLOT(OnDelINetProg()));
	connect(ui.chkINetBlockPrompt, SIGNAL(clicked(bool)), this, SLOT(OnINetBlockChanged()));
	connect(ui.chkINetBlockMsg, SIGNAL(clicked(bool)), this, SLOT(OnINetBlockChanged()));
	//

	// Access
	connect(ui.btnAddFile, SIGNAL(pressed()), this, SLOT(OnAddFile()));
	QMenu* pFileBtnMenu = new QMenu(ui.btnAddFile);
	pFileBtnMenu->addAction(tr("Browse for File"), this, SLOT(OnBrowseFile()));
	pFileBtnMenu->addAction(tr("Browse for Folder"), this, SLOT(OnBrowseFolder()));
	ui.btnAddFile->setPopupMode(QToolButton::MenuButtonPopup);
	ui.btnAddFile->setMenu(pFileBtnMenu);
	connect(ui.btnAddKey, SIGNAL(pressed()), this, SLOT(OnAddKey()));
	connect(ui.btnAddIPC, SIGNAL(pressed()), this, SLOT(OnAddIPC()));
	connect(ui.btnAddWnd, SIGNAL(pressed()), this, SLOT(OnAddWnd()));
	connect(ui.btnAddCOM, SIGNAL(pressed()), this, SLOT(OnAddCOM()));
	// todo: add priority by order 
	ui.btnMoveUp->setVisible(false);
	ui.btnMoveDown->setVisible(false);
	connect(ui.chkShowAccessTmpl, SIGNAL(clicked(bool)), this, SLOT(OnShowAccessTmpl()));
	connect(ui.btnDelAccess, SIGNAL(pressed()), this, SLOT(OnDelAccess()));

	connect(ui.treeAccess, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(OnAccessItemClicked(QTreeWidgetItem*, int)));
	connect(ui.treeAccess, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnAccessItemDoubleClicked(QTreeWidgetItem*, int)));
	connect(ui.treeAccess, SIGNAL(itemSelectionChanged()), this, SLOT(OnAccessSelectionChanged()));
	//

	// Recovery
	connect(ui.chkAutoRecovery, SIGNAL(clicked(bool)), this, SLOT(OnRecoveryChanged()));
	connect(ui.btnAddRecovery, SIGNAL(pressed()), this, SLOT(OnAddRecFolder()));
	connect(ui.btnDelRecovery, SIGNAL(pressed()), this, SLOT(OnDelRecEntry()));
	connect(ui.btnAddRecIgnore, SIGNAL(pressed()), this, SLOT(OnAddRecIgnore()));
	connect(ui.btnAddRecIgnoreExt, SIGNAL(pressed()), this, SLOT(OnAddRecIgnoreExt()));
	connect(ui.chkShowRecoveryTmpl, SIGNAL(clicked(bool)), this, SLOT(OnShowRecoveryTmpl()));
	//

	// Advanced
	connect(ui.chkPreferExternalManifest, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));

	connect(ui.chkNoWindowRename, SIGNAL(clicked(bool)), this, SLOT(OnNoWindowRename()));

	connect(ui.chkProtectSCM, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkProtectRpcSs, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkProtectSystem, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));

	connect(ui.chkOpenCredentials, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkOpenProtectedStorage, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
		
	connect(ui.chkAddToJob, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));

	connect(ui.chkFileTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkPipeTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkKeyTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkIpcTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkGuiTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkComTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkDbgTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));

	connect(ui.chkHideOtherBoxes, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.btnAddProcess, SIGNAL(pressed()), this, SLOT(OnAddProcess()));
	connect(ui.btnDelProcess, SIGNAL(pressed()), this, SLOT(OnDelProcess()));

	connect(ui.btnAddUser, SIGNAL(pressed()), this, SLOT(OnAddUser()));
	connect(ui.btnDelUser, SIGNAL(pressed()), this, SLOT(OnDelUser()));
	connect(ui.chkMonitorAdminOnly, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	
	//

	// Templates
	connect(ui.cmbCategories, SIGNAL(currentIndexChanged(int)), this, SLOT(OnFilterTemplates()));
	connect(ui.txtTemplates, SIGNAL(textChanged(const QString&)), this, SLOT(OnFilterTemplates()));
	connect(ui.treeTemplates, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(OnTemplateClicked(QTreeWidgetItem*, int)));
	connect(ui.treeTemplates, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnTemplateDoubleClicked(QTreeWidgetItem*, int)));
	//

	connect(ui.tabs, SIGNAL(currentChanged(int)), this, SLOT(OnTab()));

	// edit
	connect(ui.btnEditIni, SIGNAL(pressed()), this, SLOT(OnEditIni()));
	connect(ui.btnSaveIni, SIGNAL(pressed()), this, SLOT(OnSaveIni()));
	connect(ui.btnCancelEdit, SIGNAL(pressed()), this, SLOT(OnCancelEdit()));
	//

	connect(ui.buttonBox->button(QDialogButtonBox::Ok), SIGNAL(pressed()), this, SLOT(accept()));
	connect(ui.buttonBox->button(QDialogButtonBox::Apply), SIGNAL(pressed()), this, SLOT(apply()));
	connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	if (ReadOnly) {
		ui.btnEditIni->setEnabled(false);
		ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
		ui.buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
	}

	OnTab(); // -> LoadConfig();

	ui.treeAccess->viewport()->installEventFilter(this);

	restoreGeometry(theConf->GetBlob("OptionsWindow/Window_Geometry"));

	QByteArray
	Columns = theConf->GetBlob("OptionsWindow/Run_Columns");
	if (!Columns.isEmpty()) ui.treeRun->header()->restoreState(Columns);
	Columns = theConf->GetBlob("OptionsWindow/Groups_Columns");
	if (!Columns.isEmpty()) ui.treeGroups->header()->restoreState(Columns);
	Columns = theConf->GetBlob("OptionsWindow/Forced_Columns");
	if (!Columns.isEmpty()) ui.treeForced->header()->restoreState(Columns);
	Columns = theConf->GetBlob("OptionsWindow/Stop_Columns");
	if (!Columns.isEmpty()) ui.treeStop->header()->restoreState(Columns);
	Columns = theConf->GetBlob("OptionsWindow/Start_Columns");
	if (!Columns.isEmpty()) ui.treeStart->header()->restoreState(Columns);
	Columns = theConf->GetBlob("OptionsWindow/INet_Columns");
	if (!Columns.isEmpty()) ui.treeINet->header()->restoreState(Columns);
	Columns = theConf->GetBlob("OptionsWindow/Access_Columns");
	if (!Columns.isEmpty()) ui.treeAccess->header()->restoreState(Columns);
	Columns = theConf->GetBlob("OptionsWindow/Recovery_Columns");
	if (!Columns.isEmpty()) ui.treeRecovery->header()->restoreState(Columns);
	Columns = theConf->GetBlob("OptionsWindow/Templates_Columns");
	if (!Columns.isEmpty()) ui.treeTemplates->header()->restoreState(Columns);
}

COptionsWindow::~COptionsWindow()
{
	theConf->SetBlob("OptionsWindow/Window_Geometry",saveGeometry());

	theConf->SetBlob("OptionsWindow/Run_Columns", ui.treeRun->header()->saveState());
	theConf->SetBlob("OptionsWindow/Groups_Columns", ui.treeGroups->header()->saveState());
	theConf->SetBlob("OptionsWindow/Forced_Columns", ui.treeForced->header()->saveState());
	theConf->SetBlob("OptionsWindow/Stop_Columns", ui.treeStop->header()->saveState());
	theConf->SetBlob("OptionsWindow/Start_Columns", ui.treeStart->header()->saveState());
	theConf->SetBlob("OptionsWindow/INet_Columns", ui.treeINet->header()->saveState());
	theConf->SetBlob("OptionsWindow/Access_Columns", ui.treeAccess->header()->saveState());
	theConf->SetBlob("OptionsWindow/Recovery_Columns", ui.treeRecovery->header()->saveState());
	theConf->SetBlob("OptionsWindow/Templates_Columns", ui.treeTemplates->header()->saveState());
}

void COptionsWindow::closeEvent(QCloseEvent *e)
{
	this->deleteLater();
}

bool COptionsWindow::eventFilter(QObject *source, QEvent *event)
{
	if (event->type() == QEvent::KeyPress && ((QKeyEvent*)event)->key() == Qt::Key_Escape && ((QKeyEvent*)event)->modifiers() == Qt::NoModifier)
		CloseAccessEdit(false);
	if (source == ui.treeAccess->viewport() && event->type() == QEvent::MouseButtonPress)
		CloseAccessEdit();
	return QMainWindow::eventFilter(source, event);
}

//void COptionsWindow::OnWithTemplates()
//{
//	m_Template = ui.chkWithTemplates->isChecked();
//	ui.buttonBox->setEnabled(!m_Template);
//	LoadConfig();
//}

void COptionsWindow::ReadAdvancedCheck(const QString& Name, QCheckBox* pCheck, const QString& Value)
{
	QString Data = m_pBox->GetText(Name, "");
	if (Data == Value)			pCheck->setCheckState(Qt::Checked);
	else if (Data.isEmpty())	pCheck->setCheckState(Qt::Unchecked);
	else						pCheck->setCheckState(Qt::PartiallyChecked);
}

void COptionsWindow::LoadConfig()
{
	m_ConfigDirty = false;

	{
		QString BoxNameTitle = m_pBox->GetText("BoxNameTitle", "n");
		ui.cmbBoxIndicator->setCurrentIndex(ui.cmbBoxIndicator->findData(BoxNameTitle.toLower()));

		QStringList BorderCfg = m_pBox->GetText("BorderColor").split(",");
		ui.cmbBoxBorder->setCurrentIndex(ui.cmbBoxBorder->findData(BorderCfg.size() >= 2 ? BorderCfg[1].toLower() : "on"));
		m_BorderColor = QColor("#" + BorderCfg[0].mid(5, 2) + BorderCfg[0].mid(3, 2) + BorderCfg[0].mid(1, 2));
		ui.btnBorderColor->setStyleSheet("background-color: " + m_BorderColor.name());
		int BorderWidth = BorderCfg.count() >= 3 ? BorderCfg[2].toInt() : 0;
		if (!BorderWidth) BorderWidth = 6;
		ui.spinBorderWidth->setValue(BorderWidth);

		ui.chkBlockNetShare->setChecked(m_pBox->GetBool("BlockNetworkFiles", true));
		ui.chkBlockNetParam->setChecked(m_pBox->GetBool("BlockNetParam", true));
		ui.chkDropRights->setChecked(m_pBox->GetBool("DropAdminRights", false));

		ui.treeRun->clear();
		foreach(const QString& Value, m_pBox->GetTextList("RunCommand", m_Template))
		{
			StrPair NameCmd = Split2(Value, "|");
			QTreeWidgetItem* pItem = new QTreeWidgetItem();
			AddRunItem(NameCmd.first, NameCmd.second);
		}


		int iLimit = m_pBox->GetNum("CopyLimitKb", 80 * 1024);
		ui.chkCopyLimit->setChecked(iLimit != -1);
		ui.txtCopyLimit->setText(QString::number(iLimit > 0 ? iLimit : 80 * 1024));
		ui.chkNoCopyWarn->setChecked(!m_pBox->GetBool("CopyLimitSilent", false));
	
		ui.chkProtectBox->setChecked(m_pBox->GetBool("NeverDelete", false));
		ui.chkAutoEmpty->setChecked(m_pBox->GetBool("AutoDelete", false));

		m_GeneralChanged = false;
	}

	LoadGroups();

	LoadForced();

	LoadStop();

	{
		ui.chkStartBlockMsg->setEnabled(!ui.radStartAll->isChecked());
		ui.chkStartBlockMsg->setChecked(m_pBox->GetBool("NotifyStartRunAccessDenied", true));
	
		m_StartChanged = false;
	}

	{
		ui.chkINetBlockPrompt->setEnabled(ui.chkBlockINet->isChecked());
		ui.chkINetBlockPrompt->setChecked(m_pBox->GetBool("PromptForInternetAccess", false));
		ui.chkINetBlockMsg->setEnabled(ui.chkBlockINet->isChecked());
		ui.chkINetBlockMsg->setChecked(m_pBox->GetBool("NotifyInternetAccessDenied", true));
	
		m_INetBlockChanged = false;
	}

	LoadAccessList();

	LoadRecoveryList();

	{
		ui.chkPreferExternalManifest->setChecked(m_pBox->GetBool("PreferExternalManifest", false));

		ui.chkProtectSCM->setChecked(!m_pBox->GetBool("UnrestrictedSCM", false));
		ui.chkProtectRpcSs->setChecked(m_pBox->GetBool("ProtectRpcSs", false));
		ui.chkProtectSystem->setChecked(!m_pBox->GetBool("ExposeBoxedSystem", false));

		ui.chkOpenProtectedStorage->setChecked(m_pBox->GetBool("OpenProtectedStorage", false));
		ui.chkOpenCredentials->setEnabled(!ui.chkOpenProtectedStorage->isChecked());
		ui.chkOpenCredentials->setChecked(m_pBox->GetBool("OpenCredentials", false));

		ui.chkAddToJob->setChecked(!m_pBox->GetBool("NoAddProcessToJob", false));

		ReadAdvancedCheck("FileTrace", ui.chkFileTrace, "*");
		ReadAdvancedCheck("PipeTrace", ui.chkPipeTrace, "*");
		ReadAdvancedCheck("KeyTrace", ui.chkKeyTrace, "*");
		ReadAdvancedCheck("IpcTrace", ui.chkIpcTrace, "*");
		ReadAdvancedCheck("GuiTrace", ui.chkGuiTrace, "*");
		ReadAdvancedCheck("ClsidTrace", ui.chkComTrace, "*");
		ui.chkDbgTrace->setChecked(m_pBox->GetBool("DebugTrace", false));

		ui.chkHideOtherBoxes->setChecked(m_pBox->GetBool("HideOtherBoxes", false));
		QStringList Processes = m_pBox->GetTextList("HideHostProcess", false);
		ui.lstProcesses->clear();
		ui.lstProcesses->addItems(Processes);


		QStringList Users = m_pBox->GetText("Enabled").split(",");
		ui.lstUsers->clear();
		if (Users.count() > 1)
			ui.lstUsers->addItems(Users.mid(1));
		ui.chkMonitorAdminOnly->setChecked(m_pBox->GetBool("MonitorAdminOnly", false));

		m_AdvancedChanged = false;
	}

	foreach(QCheckBox* pCheck, m_DebugOptions.keys()) 
	{
		SDbgOpt& DbgOption = m_DebugOptions[pCheck];
		DbgOption.Changed = false;
		ReadAdvancedCheck(DbgOption.Name, pCheck, DbgOption.Value);
	}

	{
		LoadTemplates();
		m_TemplatesChanged = false;
	}
}

void COptionsWindow::WriteAdvancedCheck(QCheckBox* pCheck, const QString& Name, const QString& Value)
{
	if (pCheck->checkState() == Qt::Checked)		m_pBox->SetText(Name, Value);
	else if (pCheck->checkState() == Qt::Unchecked) m_pBox->DelValue(Name);
}

void COptionsWindow::WriteAdvancedCheck(QCheckBox* pCheck, const QString& Name, const QString& OnValue, const QString& OffValue)
{
	//if (pCheck->checkState() == Qt::PartiallyChecked)
	//	return;

	if (pCheck->checkState() == Qt::Checked)
	{
		if(!OnValue.isEmpty())
			m_pBox->SetText(Name, OnValue);
		else
			m_pBox->DelValue(Name);
	}
	else if (pCheck->checkState() == Qt::Unchecked)
	{
		if (!OffValue.isEmpty())
			m_pBox->SetText(Name, OffValue);
		else
			m_pBox->DelValue(Name);
	}
}

void COptionsWindow::SaveConfig()
{
	if (m_GeneralChanged)
	{
		m_pBox->SetText("BoxNameTitle", ui.cmbBoxIndicator->currentData().toString());

		QStringList BorderCfg;
		BorderCfg.append(QString("#%1%2%3").arg(m_BorderColor.blue(), 2, 16, QChar('0')).arg(m_BorderColor.green(), 2, 16, QChar('0')).arg(m_BorderColor.red(), 2, 16, QChar('0')));
		BorderCfg.append(ui.cmbBoxBorder->currentData().toString());
		BorderCfg.append(QString::number(ui.spinBorderWidth->value()));
		m_pBox->SetText("BorderColor", BorderCfg.join(","));

		m_pBox->SetBool("BlockNetworkFiles", ui.chkBlockNetShare->isChecked());
		m_pBox->SetBool("BlockNetParam", ui.chkBlockNetParam->isChecked());
		m_pBox->SetBool("DropAdminRights", ui.chkDropRights->isChecked());

		QStringList RunCommands;
		for (int i = 0; i < ui.treeRun->topLevelItemCount(); i++) {
			QTreeWidgetItem* pItem = ui.treeRun->topLevelItem(i);
			RunCommands.append(pItem->text(0) + "|" + pItem->text(1));
		}
		m_pBox->UpdateTextList("RunCommand", RunCommands, m_Template);


		m_pBox->SetNum("CopyLimitKb", ui.chkCopyLimit->isChecked() ? ui.txtCopyLimit->text().toInt() : -1);
		m_pBox->SetBool("CopyLimitSilent", !ui.chkNoCopyWarn->isChecked());

		m_pBox->SetBool("NeverDelete", ui.chkProtectBox->isChecked());
		m_pBox->SetBool("AutoDelete", ui.chkAutoEmpty->isChecked());

		m_GeneralChanged = false;
	}

	if (m_GroupsChanged)
		SaveGroups();

	if (m_ForcedChanged)
		SaveForced();

	if (m_StopChanged)
		SaveStop();

	if (m_StartChanged)
	{
		m_pBox->SetBool("NotifyStartRunAccessDenied", ui.chkStartBlockMsg->isChecked());

		m_StartChanged = false;
	}

	if (m_INetBlockChanged)
	{
		m_pBox->SetBool("PromptForInternetAccess", ui.chkINetBlockPrompt->isChecked());
		m_pBox->SetBool("NotifyInternetAccessDenied", ui.chkINetBlockMsg->isChecked());

		m_INetBlockChanged = false;
	}

	if (m_AccessChanged)
		SaveAccessList();

	if (m_RecoveryChanged)
		SaveRecoveryList();

	if (m_AdvancedChanged)
	{
		if (ui.chkPreferExternalManifest->isChecked()) m_pBox->SetBool("PreferExternalManifest", true);
		else m_pBox->DelValue("PreferExternalManifest");

		WriteAdvancedCheck(ui.chkProtectSCM, "UnrestrictedSCM", "", "y");
		WriteAdvancedCheck(ui.chkProtectRpcSs, "ProtectRpcSs", "y", "");
		WriteAdvancedCheck(ui.chkProtectSystem, "ExposeBoxedSystem", "", "y");
		
		WriteAdvancedCheck(ui.chkOpenProtectedStorage, "OpenProtectedStorage", "y", "");
		WriteAdvancedCheck(ui.chkOpenCredentials, "OpenCredentials", "y", "");

		WriteAdvancedCheck(ui.chkAddToJob, "NoAddProcessToJob", "", "y");

		WriteAdvancedCheck(ui.chkFileTrace, "FileTrace", "*");
		WriteAdvancedCheck(ui.chkPipeTrace, "PipeTrace", "*");
		WriteAdvancedCheck(ui.chkKeyTrace, "KeyTrace", "*");
		WriteAdvancedCheck(ui.chkIpcTrace, "IpcTrace", "*");
		WriteAdvancedCheck(ui.chkGuiTrace, "GuiTrace", "*");
		WriteAdvancedCheck(ui.chkComTrace, "ClsidTrace", "*");
		WriteAdvancedCheck(ui.chkDbgTrace, "DebugTrace", "y");

		WriteAdvancedCheck(ui.chkHideOtherBoxes, "HideOtherBoxes");

		QStringList Processes;
		for (int i = 0; i < ui.lstProcesses->count(); i++)
			Processes.append(ui.lstProcesses->item(i)->text());
		m_pBox->UpdateTextList("HideHostProcess", Processes, false);

		QStringList Users;
		for (int i = 0; i < ui.lstUsers->count(); i++)
			Users.append(ui.lstUsers->item(i)->text());
		m_pBox->SetText("Enabled", Users.count() > 0 ? "y," + Users.join(",") : "y");
		WriteAdvancedCheck(ui.chkMonitorAdminOnly, "MonitorAdminOnly");

		m_AdvancedChanged = false;
	}

	foreach(QCheckBox* pCheck, m_DebugOptions.keys())
	{
		SDbgOpt& DbgOption = m_DebugOptions[pCheck];
		if (!DbgOption.Changed)
			continue;
		WriteAdvancedCheck(pCheck, DbgOption.Name, DbgOption.Value);
		DbgOption.Changed = false;
	}

	if (m_TemplatesChanged)
		SaveTemplates();
}

void COptionsWindow::apply()
{
	if (!ui.btnEditIni->isEnabled())
		SaveIniSection();
	else
		SaveConfig();

	LoadConfig();

	emit OptionsChanged();
}

void COptionsWindow::accept()
{
	apply();

	this->close();
}

void COptionsWindow::reject()
{
	this->close();
}

void COptionsWindow::OnGeneralChanged()
{
	m_GeneralChanged = true;

	ui.lblCopyLimit->setEnabled(ui.chkCopyLimit->isChecked());
	ui.txtCopyLimit->setEnabled(ui.chkCopyLimit->isChecked());
	ui.lblCopyLimit->setText(tr("kilobytes (%1)").arg(FormatSize(ui.txtCopyLimit->text().toULongLong() * 1024)));
	ui.chkNoCopyWarn->setEnabled(ui.chkCopyLimit->isChecked());

	ui.chkAutoEmpty->setEnabled(!ui.chkProtectBox->isChecked());
}

void COptionsWindow::OnPickColor()
{
	QColor color = QColorDialog::getColor(m_BorderColor, this, "Select color");
	if (!color.isValid())
		return;
	m_GeneralChanged = true;
	m_BorderColor = color;
	ui.btnBorderColor->setStyleSheet("background-color: " + m_BorderColor.name());
}

void COptionsWindow::OnBrowsePath()
{
	QString Value = QFileDialog::getOpenFileName(this, tr("Select Program"), "", tr("Executables (*.exe|*.cmd)")).replace("/", "\\");;
	if (Value.isEmpty())
		return;

	QString Name = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter a menu title"), QLineEdit::Normal);
	if (Name.isEmpty())
		return;

	AddRunItem(Name, Value);
	m_GeneralChanged = true;
}

void COptionsWindow::OnAddCommand()
{
	QString Value = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter a command"), QLineEdit::Normal);
	if (Value.isEmpty())
		return;

	QString Name = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter a menu title"), QLineEdit::Normal);
	if (Name.isEmpty())
		return;

	AddRunItem(Name, Value);
	m_GeneralChanged = true;
}

void COptionsWindow::AddRunItem(const QString& Name, const QString& Command)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem();
	pItem->setText(0, Name);
	pItem->setText(1, Command);
	pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
	ui.treeRun->addTopLevelItem(pItem);
}

void COptionsWindow::OnDelCommand()
{
	QTreeWidgetItem* pItem = ui.treeRun->currentItem();
	if (!pItem)
		return;

	delete pItem;
	m_GeneralChanged = true;
}

void COptionsWindow::SetProgramItem(QString Program, QTreeWidgetItem* pItem, int Column)
{
	pItem->setData(Column, Qt::UserRole, Program);
	if (Program.left(1) == "<")
		Program = tr("Group: %1").arg(Program.mid(1, Program.length() - 2));
	else
		m_Programs.insert(Program);
	pItem->setText(Column, Program);
}

void COptionsWindow::LoadGroups()
{
	m_TemplateGroups.clear();
	ui.treeGroups->clear();

	QMultiMap<QString, QStringList> GroupMap; // if we have a duplicate we want to know it
	QSet<QString> LocalGroups;

	QStringList ProcessGroups = m_pBox->GetTextList("ProcessGroup", m_Template);
	foreach(const QString& Group, ProcessGroups)
	{
		QStringList Entries = Group.split(",");
		QString GroupName = Entries.takeFirst();
		GroupMap.insertMulti(GroupName, Entries);
		LocalGroups.insert(GroupName);
	}

	foreach(const QString& Template, m_pBox->GetTemplates())
	{
		foreach(const QString& Group, m_pBox->GetTextListTmpl("ProcessGroup", Template))
		{
			m_TemplateGroups.insert(Group);
			QStringList Entries = Group.split(",");
			QString GroupName = Entries.takeFirst();
			if (LocalGroups.contains(GroupName))
				continue; // local group definitions overwrite template once
			GroupMap.insertMulti(GroupName, Entries);
		}
	}

	for(QMultiMap<QString, QStringList>::iterator I = GroupMap.begin(); I != GroupMap.end(); ++I)
	{
		QString GroupName = I.key();
		QStringList Entries = I.value();
		QTreeWidgetItem* pItem = new QTreeWidgetItem();
		pItem->setData(0, Qt::UserRole, GroupName);
		if (GroupName.length() > 2)
			GroupName = GroupName.mid(1, GroupName.length() - 2);
		pItem->setText(0, GroupName);
		for (int i = 0; i < Entries.count(); i++) 
		{
			QTreeWidgetItem* pSubItem = new QTreeWidgetItem();
			SetProgramItem(Entries[i], pSubItem, 0);
			pItem->addChild(pSubItem);
		}
		ui.treeGroups->addTopLevelItem(pItem);
	}
	ui.treeGroups->expandAll();

	m_GroupsChanged = false;
}

void COptionsWindow::SaveGroups()
{
	QStringList ProcessGroups;
	for (int i = 0; i < ui.treeGroups->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeGroups->topLevelItem(i);
		QString GroupName = pItem->data(0, Qt::UserRole).toString();
		QStringList Programs;
		for (int j = 0; j < pItem->childCount(); j++)
			Programs.append(pItem->child(j)->data(0, Qt::UserRole).toString());
		QString Group = GroupName + "," + Programs.join(",");
		if (m_TemplateGroups.contains(Group))
			continue; // don't save unchanged groups to local config
		ProcessGroups.append(Group);
	}

	m_pBox->UpdateTextList("ProcessGroup", ProcessGroups, m_Template);

	m_GroupsChanged = false;
}

void COptionsWindow::OnAddGroup()
{
	QString Value = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter a name for the new group"), QLineEdit::Normal, "NewGroup");
	if (Value.isEmpty())
		return;
	
	for (int i = 0; i < ui.treeGroups->topLevelItemCount(); i++) {
		QTreeWidgetItem* pItem = ui.treeGroups->topLevelItem(i);
		if (pItem->text(0).compare(Value, Qt::CaseInsensitive) == 0)
			return;
	}

	QTreeWidgetItem* pItem = new QTreeWidgetItem();
	pItem->setText(0, Value);
	pItem->setData(0, Qt::UserRole, "<" + Value + ">");
	ui.treeGroups->addTopLevelItem(pItem);

	m_GroupsChanged = true;
}

QString COptionsWindow::SelectProgram(bool bOrGroup)
{
	CComboInputDialog progDialog(this);
	progDialog.setText(tr("Enter program:"));
	progDialog.setEditable(true);

	if (bOrGroup)
	{
		for (int i = 0; i < ui.treeGroups->topLevelItemCount(); i++) {
			QTreeWidgetItem* pItem = ui.treeGroups->topLevelItem(i);
			progDialog.addItem(tr("Group: %1").arg(pItem->text(0)), pItem->data(0, Qt::UserRole).toString());
		}
	}

	foreach(const QString & Name, m_Programs)
		progDialog.addItem(Name, Name);

	progDialog.setValue("");

	if (!progDialog.exec())
		return QString();

	QString Program = progDialog.value();
	int Index = progDialog.findValue(Program);
	if (Index != -1)
		Program = progDialog.data().toString();

	return Program;
}

void COptionsWindow::OnAddProg()
{
	QTreeWidgetItem* pItem = ui.treeGroups->currentItem();
	while (pItem && pItem->parent())
		pItem = pItem->parent();

	if (!pItem)
	{
		QMessageBox::warning(this, "SandboxiePlus", tr("Please select group first."));
		return;
	}

	QString Value = SelectProgram();
	if (Value.isEmpty())
		return;

	QTreeWidgetItem* pSubItem = new QTreeWidgetItem();
	SetProgramItem(Value, pSubItem, 0);
	pItem->addChild(pSubItem);

	m_GroupsChanged = true;
}

void COptionsWindow::OnDelProg()
{
	QTreeWidgetItem* pItem = ui.treeGroups->currentItem();
	if (!pItem)
		return;

	delete pItem;

	m_GroupsChanged = true;
}

void COptionsWindow::CopyGroupToList(const QString& Groupe, QTreeWidget* pTree)
{
	pTree->clear();

	for (int i = 0; i < ui.treeGroups->topLevelItemCount(); i++) 
	{
		QTreeWidgetItem* pItem = ui.treeGroups->topLevelItem(i);
		if (pItem->data(0, Qt::UserRole).toString().compare(Groupe, Qt::CaseInsensitive) == 0)
		{
			for (int j = 0; j < pItem->childCount(); j++)
			{
				QString Value = pItem->child(j)->data(0, Qt::UserRole).toString();

				QTreeWidgetItem* pSubItem = new QTreeWidgetItem();
				SetProgramItem(Value, pSubItem, 0);
				pTree->addTopLevelItem(pSubItem);
			}
			break;
		}
	}
}

void COptionsWindow::LoadForced()
{
	ui.treeForced->clear();

	foreach(const QString& Value, m_pBox->GetTextList("ForceProcess", m_Template))
		AddForcedEntry(Value, 1);

	foreach(const QString& Value, m_pBox->GetTextList("ForceFolder", m_Template))
		AddForcedEntry(Value, 2);

	if (ui.chkShowForceTmpl->isChecked())
	{
		foreach(const QString& Template, m_pBox->GetTemplates())
		{
			foreach(const QString& Value, m_pBox->GetTextListTmpl("ForceProcess", Template))
				AddForcedEntry(Value, 1, Template);

			foreach(const QString& Value, m_pBox->GetTextListTmpl("ForceFolder", Template))
				AddForcedEntry(Value, 2, Template);
		}
	}

	m_ForcedChanged = false;
}

void COptionsWindow::AddForcedEntry(const QString& Name, int type, const QString& Template)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem();
	pItem->setText(0, (type == 1 ? tr("Process") : tr("Folder")) + (Template.isEmpty() ? "" : (" (" + Template + ")")));
	pItem->setData(0, Qt::UserRole, Template.isEmpty() ? type : -1);
	SetProgramItem(Name, pItem, 1);
	ui.treeForced->addTopLevelItem(pItem);
}

void COptionsWindow::SaveForced()
{
	QStringList ForceProcess;
	QStringList ForceFolder;
	for (int i = 0; i < ui.treeForced->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeForced->topLevelItem(i);
		int Type = pItem->data(0, Qt::UserRole).toInt();
		if (Type == -1)
			continue; // entry from template
		switch (Type)
		{
		case 1:	ForceProcess.append(pItem->data(1, Qt::UserRole).toString()); break;
		case 2: ForceFolder.append(pItem->data(1, Qt::UserRole).toString()); break;
		}
	}

	m_pBox->UpdateTextList("ForceProcess", ForceProcess, m_Template);
	m_pBox->UpdateTextList("ForceFolder", ForceFolder, m_Template);

	m_ForcedChanged = false;
}

void COptionsWindow::OnForceProg()
{
	QString Value = SelectProgram();
	if (Value.isEmpty())
		return;
	AddForcedEntry(Value, 1);
	m_ForcedChanged = true;
}

void COptionsWindow::OnForceDir()
{
	QString Value = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");
	if (Value.isEmpty())
		return;
	AddForcedEntry(Value, 2);
	m_ForcedChanged = true;
}

void COptionsWindow::OnDelForce()
{
	DeleteAccessEntry(ui.treeForced->currentItem());
	m_ForcedChanged = true;
}

void COptionsWindow::LoadStop()
{
	ui.treeStop->clear();

	foreach(const QString& Value, m_pBox->GetTextList("LingerProcess", m_Template))
		AddStopEntry(Value, 1);

	foreach(const QString& Value, m_pBox->GetTextList("LeaderProcess", m_Template))
		AddStopEntry(Value, 2);

	if (ui.chkShowStopTmpl->isChecked())
	{
		foreach(const QString& Template, m_pBox->GetTemplates())
		{
			foreach(const QString& Value, m_pBox->GetTextListTmpl("LingerProcess", Template))
				AddStopEntry(Value, 1, Template);

			foreach(const QString& Value, m_pBox->GetTextListTmpl("LeaderProcess", Template))
				AddStopEntry(Value, 2, Template);
		}
	}

	m_StopChanged = false;
}

void COptionsWindow::AddStopEntry(const QString& Name, int type, const QString& Template)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem();
	pItem->setText(0, (type == 1 ? tr("Lingerer") : tr("Leader")) + (Template.isEmpty() ? "" : (" (" + Template + ")")));
	pItem->setData(0, Qt::UserRole, Template.isEmpty() ? type : -1);
	SetProgramItem(Name, pItem, 1);
	ui.treeStop->addTopLevelItem(pItem);
}

void COptionsWindow::SaveStop()
{
	QStringList LingerProcess;
	QStringList LeaderProcess;
	for (int i = 0; i < ui.treeStop->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeStop->topLevelItem(i);
		int Type = pItem->data(0, Qt::UserRole).toInt();
		if (Type == -1)
			continue; // entry from template
		switch (Type)
		{
		case 1:	LingerProcess.append(pItem->data(1, Qt::UserRole).toString()); break;
		case 2: LeaderProcess.append(pItem->data(1, Qt::UserRole).toString()); break;
		}
	}

	m_pBox->UpdateTextList("LingerProcess", LingerProcess, m_Template);
	m_pBox->UpdateTextList("LeaderProcess", LeaderProcess, m_Template);

	m_StopChanged = false;
}

void COptionsWindow::OnAddLingering()
{
	QString Value = SelectProgram();
	if (Value.isEmpty())
		return;
	AddStopEntry(Value, 1);
	m_StopChanged = true;
}

void COptionsWindow::OnAddLeader()
{
	QString Value = SelectProgram();
	if (Value.isEmpty())
		return;
	AddStopEntry(Value, 2);
	m_StopChanged = true;
}

void COptionsWindow::OnDelStopProg()
{
	DeleteAccessEntry(ui.treeStop->currentItem());
	m_StopChanged = true;
}

void COptionsWindow::OnRestrictStart()
{
	// only selected
	bool Enable = ui.radStartSelected->isChecked();
	if (Enable)
		SetAccessEntry(eIPC, "!<StartRunAccess>", eClosed, "*");
	else
		DelAccessEntry(eIPC, "!<StartRunAccess>", eClosed, "*");

	// all except selected
	Enable = ui.radStartExcept->isChecked();
	if (Enable)
		SetAccessEntry(eIPC, "<StartRunAccess>", eClosed, "*");
	else
		DelAccessEntry(eIPC, "<StartRunAccess>", eClosed, "*");

	ui.chkStartBlockMsg->setEnabled(!ui.radStartAll->isChecked());
	//m_StartChanged = true;
}

void COptionsWindow::OnAddStartProg()
{
	AddProgToGroup(ui.treeStart, "<StartRunAccess>");
	//m_StartChanged = true;
}

void COptionsWindow::OnDelStartProg()
{
	DelProgFromGroup(ui.treeStart, "<StartRunAccess>");
	//m_StartChanged = true;
}

void COptionsWindow::OnBlockINet()
{
	bool Enable = ui.chkBlockINet->isChecked();
	ui.chkINetBlockPrompt->setEnabled(Enable);
	ui.chkINetBlockMsg->setEnabled(Enable);
	if (Enable)
		SetAccessEntry(eFile, "!<InternetAccess>", eClosed, "InternetAccessDevices");
	else
		DelAccessEntry(eFile, "!<InternetAccess>", eClosed, "InternetAccessDevices");
	//m_INetBlockChanged = true;
}

void COptionsWindow::OnAddINetProg()
{
	AddProgToGroup(ui.treeINet, "<InternetAccess>");
	//m_INetBlockChanged = true;
}

void COptionsWindow::OnDelINetProg()
{
	DelProgFromGroup(ui.treeINet, "<InternetAccess>");
	//m_INetBlockChanged = true;
}

void COptionsWindow::AddProgToGroup(QTreeWidget* pTree, const QString& Groupe)
{
	QString Value = SelectProgram();
	if (Value.isEmpty())
		return;

	QTreeWidgetItem* pItem = new QTreeWidgetItem();
	SetProgramItem(Value, pItem, 0);
	pTree->addTopLevelItem(pItem);

	AddProgToGroup(Value, Groupe);
}

void COptionsWindow::AddProgToGroup(const QString& Value, const QString& Groupe)
{
	QTreeWidgetItem* pGroupItem = NULL;
	for (int i = 0; i < ui.treeGroups->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pCurItem = ui.treeGroups->topLevelItem(i);
		if (pCurItem->data(0, Qt::UserRole).toString().compare(Groupe, Qt::CaseInsensitive) == 0)
		{
			pGroupItem = pCurItem;
			break;
		}
	}

	if (!pGroupItem)
	{
		pGroupItem = new QTreeWidgetItem();
		pGroupItem->setText(0, Groupe.mid(1, Groupe.length()-2));
		pGroupItem->setData(0, Qt::UserRole, Groupe);
		ui.treeGroups->addTopLevelItem(pGroupItem);
	}

	QTreeWidgetItem* pProgItem = new QTreeWidgetItem();
	SetProgramItem(Value, pProgItem, 0);
	pGroupItem->addChild(pProgItem);

	m_GroupsChanged = true;
}

void COptionsWindow::DelProgFromGroup(QTreeWidget* pTree, const QString& Groupe)
{
	QTreeWidgetItem* pItem = pTree->currentItem();
	if (!pItem)
		return;

	QString Value = pItem->data(0, Qt::UserRole).toString();

	delete pItem;

	for (int i = 0; i < ui.treeGroups->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pGroupItem = ui.treeGroups->topLevelItem(i);
		if (pGroupItem->data(0, Qt::UserRole).toString().compare(Groupe, Qt::CaseInsensitive) == 0)
		{
			for (int j = 0; j < pGroupItem->childCount(); j++)
			{
				QTreeWidgetItem* pProgItem = pGroupItem->child(j);
				if (pProgItem->data(0, Qt::UserRole).toString().compare(Value, Qt::CaseInsensitive) == 0)
				{
					delete pProgItem;
					m_GroupsChanged = true;
					break;
				}
			}
			break;
		}
	}
}

QTreeWidgetItem* COptionsWindow::GetAccessEntry(EAccessType Type, const QString& Program, EAccessMode Mode, const QString& Path)
{
	for (int i = 0; i < ui.treeAccess->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeAccess->topLevelItem(i);
		if (pItem->data(0, Qt::UserRole).toInt() == Type
		 && pItem->data(1, Qt::UserRole).toString().compare(Program, Qt::CaseInsensitive) == 0
		 && pItem->data(2, Qt::UserRole).toInt() == Mode
		 && pItem->data(3, Qt::UserRole).toString().compare(Path, Qt::CaseInsensitive) == 0)
			return pItem;
	}
	return NULL;
}

void COptionsWindow::SetAccessEntry(EAccessType Type, const QString& Program, EAccessMode Mode, const QString& Path)
{
	if (GetAccessEntry(Type, Program, Mode, Path) != NULL)
		return; // already set
	m_AccessChanged = true;
	AddAccessEntry(Type, Mode, Program, Path);
}

void COptionsWindow::DelAccessEntry(EAccessType Type, const QString& Program, EAccessMode Mode, const QString& Path)
{
	if(QTreeWidgetItem* pItem = GetAccessEntry(Type, Program, Mode, Path))
	{
		delete pItem;
		m_AccessChanged = true;
	}
}

QString COptionsWindow::AccessTypeToName(EAccessEntry Type)
{
	switch (Type)
	{
	case eOpenFilePath:		return "OpenFilePath";
	case eOpenPipePath:		return "OpenPipePath";
	case eClosedFilePath:	return "ClosedFilePath";
	case eReadFilePath:		return "ReadFilePath";
	case eWriteFilePath:	return "WriteFilePath";

	case eOpenKeyPath:		return "OpenKeyPath";
	case eClosedKeyPath:	return "ClosedKeyPath";
	case eReadKeyPath:		return "ReadKeyPath";
	case eWriteKeyPath:		return "WriteKeyPath";

	case eOpenIpcPath:		return "OpenIpcPath";
	case eClosedIpcPath:	return "ClosedIpcPath";

	case eOpenWinClass:		return "OpenWinClass";

	case eOpenCOM:			return "OpenClsid";
	case eClosedCOM:		return "ClosedClsid";
	case eClosedCOM_RT:		return "ClosedRT";
	}
	return "Unknown";
}

void COptionsWindow::LoadAccessList()
{
	ui.treeAccess->clear();

	for (int i = 0; i < eMaxAccessType; i++)
	{
		foreach(const QString& Value, m_pBox->GetTextList(AccessTypeToName((EAccessEntry)i), m_Template))
			ParseAndAddAccessEntry((EAccessEntry)i, Value);
	}

	if (ui.chkShowAccessTmpl->isChecked())
	{
		foreach(const QString& Template, m_pBox->GetTemplates())
		{
			for (int i = 0; i < eMaxAccessType; i++)
			{
				foreach(const QString& Value, m_pBox->GetTextListTmpl(AccessTypeToName((EAccessEntry)i), Template))
					ParseAndAddAccessEntry((EAccessEntry)i, Value, Template);
			}
		}
	}

	m_AccessChanged = false;
}

void COptionsWindow::ParseAndAddAccessEntry(EAccessEntry EntryType, const QString& Value, const QString& Template)
{
	EAccessType	Type;
	EAccessMode	Mode;
	switch (EntryType)
	{
	case eOpenFilePath:		Type = eFile;	Mode = eDirect;	break;
	case eOpenPipePath:		Type = eFile;	Mode = eDirectAll; break;
	case eClosedFilePath:	Type = eFile;	Mode = eClosed;	break;
	case eReadFilePath:		Type = eFile;	Mode = eReadOnly; break;
	case eWriteFilePath:	Type = eFile;	Mode = eWriteOnly; break;

	case eOpenKeyPath:		Type = eKey;	Mode = eDirect;	break;
	case eClosedKeyPath:	Type = eKey;	Mode = eClosed;	break;
	case eReadKeyPath:		Type = eKey;	Mode = eReadOnly; break;
	case eWriteKeyPath:		Type = eKey;	Mode = eWriteOnly; break;

	case eOpenIpcPath:		Type = eIPC;	Mode = eDirect;	break;
	case eClosedIpcPath:	Type = eIPC;	Mode = eClosed;	break;

	case eOpenWinClass:		Type = eWnd;	Mode = eDirect;	break;

	case eOpenCOM:			Type = eCOM;	Mode = eDirect;	break;
	case eClosedCOM:		Type = eCOM;	Mode = eClosed;	break;
	case eClosedCOM_RT:		Type = eCOM;	Mode = eClosedRT; break;

	default:				return;
	}

	//
	// Mind this special cases
	// OpenIpcPath=$:program.exe <- full access into the address space of a target process running outside the sandbox. 
	// OpenWinClass=$:program.exe <- permits to use the PostThreadMessage API to send a message directly to a thread running outside the sandbox. 
	// This form of the setting does not support wildcards.
	//

	QStringList Values = Value.split(",");
	if (Values.count() >= 2) 
		AddAccessEntry(Type, Mode, Values[0], Values[1], Template);
	else if (Values[0].left(2) == "$:") // special cases
		AddAccessEntry(Type, Mode, Values[0].mid(2), "$", Template);
	else // all programs
		AddAccessEntry(Type, Mode, "", Values[0], Template);
}

QString COptionsWindow::GetAccessModeStr(EAccessMode Mode)
{
	switch (Mode)
	{
	case eDirect:		return "Direct";
	case eDirectAll:	return "Direct All";
	case eClosed:		return "Closed";
	case eClosedRT:		return "Closed RT";
	case eReadOnly:		return "Read Only";
	case eWriteOnly:	return "Write Only";
	}
	return "Unknown";
}

QString COptionsWindow::GetAccessTypeStr(EAccessType Type)
{
	switch (Type)
	{
	case eFile:			return "File/Folder";
	case eKey:			return "Registry";
	case eIPC:			return "IPC Path";
	case eWnd:			return "Wnd Class";
	case eCOM:			return "COM Object";
	}
	return "Unknown";
}

void COptionsWindow::OnBrowseFile()
{
	QString Value = QFileDialog::getOpenFileName(this, tr("Select File"), "", tr("All Files (*.*)")).replace("/", "\\");
	if (Value.isEmpty())
		return;

	AddAccessEntry(eFile, eDirect, "", Value);
}

void COptionsWindow::OnBrowseFolder()
{
	QString Value = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");
	if (Value.isEmpty())
		return;

	AddAccessEntry(eFile, eDirect, "", Value);
}

void COptionsWindow::AddAccessEntry(EAccessType	Type, EAccessMode Mode, QString Program, const QString& Path, const QString& Template)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem();

	pItem->setText(0, GetAccessTypeStr(Type) + (Template.isEmpty() ? "" : " (" + Template + ")"));
	pItem->setData(0, Qt::UserRole, !Template.isEmpty() ? -1 : (int)Type);

	pItem->setData(1, Qt::UserRole, Program);
	bool bAll = Program.isEmpty();
	if (bAll)
		Program = tr("All Programs");
	bool Not = Program.left(1) == "!";
	if (Not)
		Program.remove(0, 1);
	if (Program.left(1) == "<")
		Program = tr("Group: %1").arg(Program.mid(1, Program.length() - 2));
	else if(!bAll)
		m_Programs.insert(Program);
	pItem->setText(1, (Not ? "NOT " : "") + Program);
	
	pItem->setText(2, GetAccessModeStr(Mode));
	pItem->setData(2, Qt::UserRole, (int)Mode);

	pItem->setText(3, Path);
	pItem->setData(3, Qt::UserRole, Path);

	ui.treeAccess->addTopLevelItem(pItem);
}

QString COptionsWindow::MakeAccessStr(EAccessType Type, EAccessMode Mode)
{
	switch (Type)
	{
	case eFile:
		switch (Mode)
		{
		case eDirect:		return "OpenFilePath";
		case eDirectAll:	return "OpenPipePath";
		case eClosed:		return "ClosedFilePath";
		case eReadOnly:		return "ReadFilePath";
		case eWriteOnly:	return "WriteFilePath";
		}
		break;
	case eKey:
		switch (Mode)
		{
		case eDirect:		return "OpenKeyPath";
		case eClosed:		return "ClosedKeyPath";
		case eReadOnly:		return "ReadKeyPath";
		case eWriteOnly:	return "WriteKeyPath";
		}
		break;
	case eIPC:
		switch (Mode)
		{
		case eDirect:		return "OpenIpcPath";
		case eClosed:		return "ClosedIpcPath";
		}
		break;
	case eWnd:
		switch (Mode)
		{
		case eDirect:		return "OpenWinClass";
		}
		break;
	case eCOM:
		switch (Mode)
		{
		case eDirect:		return "OpenClsid";
		case eClosed:		return "ClosedClsid";
		case eClosedRT:		return "ClosedRT";
		}
		break;
	}
	return "Unknown";
}

void COptionsWindow::OnAccessItemClicked(QTreeWidgetItem* pItem, int Column)
{
	if (Column != 0)
		return;

	CloseAccessEdit(pItem);
}

void COptionsWindow::CloseAccessEdit(bool bSave)
{
	for (int i = 0; i < ui.treeAccess->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeAccess->topLevelItem(i);
		CloseAccessEdit(pItem, bSave);
	}
}

void COptionsWindow::CloseAccessEdit(QTreeWidgetItem* pItem, bool bSave)
{
	QWidget* pProgram = ui.treeAccess->itemWidget(pItem, 1);
	if (!pProgram)
		return;

	QHBoxLayout* pLayout = (QHBoxLayout*)pProgram->layout();
	QToolButton* pNot = (QToolButton*)pLayout->itemAt(0)->widget();
	QComboBox* pCombo = (QComboBox*)pLayout->itemAt(1)->widget();

	QComboBox* pMode = (QComboBox*)ui.treeAccess->itemWidget(pItem, 2);
	QLineEdit* pPath = (QLineEdit*)ui.treeAccess->itemWidget(pItem, 3);

	QString Program = pCombo->currentText();
	int Index = pCombo->findText(Program);
	if (Index != -1)
		Program = pCombo->itemData(Index, Qt::UserRole).toString();
	if (!Program.isEmpty() && Program.left(1) != "<")
		m_Programs.insert(Program);

	if (bSave)
	{
		if (pItem->data(0, Qt::UserRole).toInt() == eCOM)
		{
			bool isGUID = pPath->text().length() == 38 && pPath->text().left(1) == "{" && pPath->text().right(1) == "}";
			switch (pMode->currentData().toInt())
			{
			case eDirect:
			case eClosed:
				if (!isGUID) {
					QMessageBox::critical(this, "SandboxiePlus", tr("COM objects must be specified by their GUID, like: {00000000-0000-0000-0000-000000000000}"));
					return;
				}
				break;
			case eClosedRT:
				if (isGUID) {
					QMessageBox::critical(this, "SandboxiePlus", tr("RT interfaces must be specified by their name."));
					return;
				}
				break;
			}
		}

		pItem->setText(1, (pNot->isChecked() ? "NOT " : "") + pCombo->currentText());
		pItem->setData(1, Qt::UserRole, (pNot->isChecked() ? "!" : "") + Program);
		pItem->setText(2, GetAccessModeStr((EAccessMode)pMode->currentData().toInt()));
		pItem->setData(2, Qt::UserRole, pMode->currentData());
		pItem->setText(3, pPath->text());
		pItem->setData(3, Qt::UserRole, pPath->text());
	}

	ui.treeAccess->setItemWidget(pItem, 1, NULL);
	ui.treeAccess->setItemWidget(pItem, 2, NULL);
	ui.treeAccess->setItemWidget(pItem, 3, NULL);

	m_AccessChanged = true;
}

QList<COptionsWindow::EAccessMode> COptionsWindow::GetAccessModes(EAccessType Type)
{
	switch (Type)
	{
	case eFile:			return QList<EAccessMode>() << eDirect << eDirectAll << eClosed << eReadOnly << eWriteOnly;
	case eKey:			return QList<EAccessMode>() << eDirect << eClosed << eReadOnly << eWriteOnly;
	case eIPC:			return QList<EAccessMode>() << eDirect << eClosed;
	case eWnd:			return QList<EAccessMode>() << eDirect;
	case eCOM:			return QList<EAccessMode>() << eDirect << eClosed << eClosedRT;
	}
	return QList<EAccessMode>();
}

void COptionsWindow::OnAccessItemDoubleClicked(QTreeWidgetItem* pItem, int Column)
{
	//if (Column == 0)
	//	return;

	int Type = pItem->data(0, Qt::UserRole).toInt();
	if (Type == -1) {
		QMessageBox::warning(this, "SandboxiePlus", tr("Template values can not be edited."));
		return;
	}

	QString Program = pItem->data(1, Qt::UserRole).toString();

	QWidget* pProgram = new QWidget();
	pProgram->setAutoFillBackground(true);
	QHBoxLayout* pLayout = new QHBoxLayout();
	pLayout->setMargin(0);
	pLayout->setSpacing(0);
	pProgram->setLayout(pLayout);
	QToolButton* pNot = new QToolButton(pProgram);
	pNot->setText("!");
	pNot->setCheckable(true);
	if (Program.left(1) == "!"){
		pNot->setChecked(true);
		Program.remove(0, 1);
	}
	pLayout->addWidget(pNot);
	QComboBox* pCombo = new QComboBox(pProgram);
	pCombo->addItem(tr("All Programs"), "");

	for (int i = 0; i < ui.treeGroups->topLevelItemCount(); i++) {
		QTreeWidgetItem* pItem = ui.treeGroups->topLevelItem(i);
		pCombo->addItem(tr("Group: %1").arg(pItem->text(0)), pItem->data(0, Qt::UserRole).toString());
	}

	foreach(const QString & Name, m_Programs)
		pCombo->addItem(Name, Name);

	pCombo->setEditable(true);
	int Index = pCombo->findData(Program);
	pCombo->setCurrentIndex(Index);
	if(Index == -1)
		pCombo->setCurrentText(Program);
	pLayout->addWidget(pCombo);

	ui.treeAccess->setItemWidget(pItem, 1, pProgram);

	QComboBox* pMode = new QComboBox();
	foreach(EAccessMode Mode, GetAccessModes((EAccessType)Type))
		pMode->addItem(GetAccessModeStr(Mode), (int)Mode);
	pMode->setCurrentIndex(pMode->findData(pItem->data(2, Qt::UserRole)));
	ui.treeAccess->setItemWidget(pItem, 2, pMode);

	QLineEdit* pPath = new QLineEdit();
	pPath->setText(pItem->data(3, Qt::UserRole).toString());
	ui.treeAccess->setItemWidget(pItem, 3, pPath);
}

void COptionsWindow::DeleteAccessEntry(QTreeWidgetItem* pItem)
{
	if (!pItem)
		return;

	if (pItem->data(0, Qt::UserRole).toInt() == -1) {
		QMessageBox::warning(this, "SandboxiePlus", tr("Template values can not be removed."));
		return;
	}

	delete pItem;
}

void COptionsWindow::OnDelAccess()
{
	DeleteAccessEntry(ui.treeAccess->currentItem());
	m_AccessChanged = true;
}


void COptionsWindow::SaveAccessList()
{
	QStringList Keys = QStringList() << "OpenFilePath" << "OpenPipePath" << "ClosedFilePath" << "ReadFilePath" << "WriteFilePath"
		<< "OpenKeyPath" << "ClosedKeyPath" << "ReadKeyPath" << "WriteKeyPath"
		<< "OpenIpcPath" << "ClosedIpcPath" << "OpenWinClass" << "OpenClsid" << "ClosedClsid" << "ClosedRT";

	QMap<QString, QList<QString>> AccessMap;
	for (int i = 0; i < ui.treeAccess->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeAccess->topLevelItem(i);
		int Type = pItem->data(0, Qt::UserRole).toInt();
		if (Type == -1)
			continue; // entry from template
		int Mode = pItem->data(2, Qt::UserRole).toInt();
		QString Program = pItem->data(1, Qt::UserRole).toString();
		QString Value = pItem->data(3, Qt::UserRole).toString();
		if (Value == "$") // special cases
			Value = "$:" + Program;
		else if (!Program.isEmpty())
			Value.prepend(Program + ",");
		AccessMap[MakeAccessStr((EAccessType)Type, (EAccessMode)Mode)].append(Value);
	}

	foreach(const QString& Key, Keys)
		m_pBox->UpdateTextList(Key, AccessMap[Key], m_Template);

	m_AccessChanged = false;
}

void COptionsWindow::LoadRecoveryList()
{
	ui.treeRecovery->clear();

	foreach(const QString& Value, m_pBox->GetTextList("RecoverFolder", m_Template))
		AddRecoveryEntry(Value, 1);

	foreach(const QString& Value, m_pBox->GetTextList("AutoRecoverIgnore", m_Template))
		AddRecoveryEntry(Value, 2);

	if (ui.chkShowRecoveryTmpl->isChecked())
	{
		foreach(const QString& Template, m_pBox->GetTemplates())
		{
			foreach(const QString& Value, m_pBox->GetTextListTmpl("RecoverFolder", Template))
				AddRecoveryEntry(Value, 1, Template);

			foreach(const QString& Value, m_pBox->GetTextListTmpl("AutoRecoverIgnore", Template))
				AddRecoveryEntry(Value, 2, Template);
		}
	}

	ui.chkAutoRecovery->setChecked(m_pBox->GetBool("AutoRecover", false));

	m_RecoveryChanged = false;
}

void COptionsWindow::AddRecoveryEntry(const QString& Name, int type, const QString& Template)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem();
	pItem->setText(0, (type == 1 ? tr("Folder") : tr("Exclusion")) + (Template.isEmpty() ? "" : (" (" + Template + ")")));
	pItem->setData(0, Qt::UserRole, Template.isEmpty() ? type : -1);
	pItem->setText(1, Name);
	pItem->setData(1, Qt::UserRole, Name);
	ui.treeRecovery->addTopLevelItem(pItem);
}

void COptionsWindow::SaveRecoveryList()
{
	QStringList RecoverFolder;
	QStringList AutoRecoverIgnore;
	for (int i = 0; i < ui.treeRecovery->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeRecovery->topLevelItem(i);
		int Type = pItem->data(0, Qt::UserRole).toInt();
		if (Type == -1)
			continue; // entry from template
		switch (Type)
		{
		case 1:	RecoverFolder.append(pItem->data(1, Qt::UserRole).toString()); break;
		case 2: AutoRecoverIgnore.append(pItem->data(1, Qt::UserRole).toString()); break;
		}
	}

	m_pBox->UpdateTextList("RecoverFolder", RecoverFolder, m_Template);
	m_pBox->UpdateTextList("AutoRecoverIgnore", AutoRecoverIgnore, m_Template);

	m_pBox->SetBool("AutoRecover", ui.chkAutoRecovery->isChecked());

	m_RecoveryChanged = false;
}

void COptionsWindow::OnAddRecFolder()
{
	QString Value = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");
	if (Value.isEmpty())
		return;

	AddRecoveryEntry(Value, 1);
	m_RecoveryChanged = true;
}

void COptionsWindow::OnAddRecIgnore()
{
	QString Value = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");
	if (Value.isEmpty())
		return;

	AddRecoveryEntry(Value, 2);
	m_RecoveryChanged = true;
}

void COptionsWindow::OnAddRecIgnoreExt()
{
	QString Value = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter a file extension to be excluded"), QLineEdit::Normal);
	if (Value.isEmpty())
		return;

	AddRecoveryEntry(Value, 2);
	m_RecoveryChanged = true;
}

void COptionsWindow::OnDelRecEntry()
{
	QTreeWidgetItem* pItem = ui.treeRecovery->currentItem();
	if (!pItem)
		return;

	if (pItem->data(0, Qt::UserRole).toInt() == -1) {
		QMessageBox::warning(this, "SandboxiePlus", tr("Template values can not be removed."));
		return;
	}

	delete pItem;
	m_RecoveryChanged = true;
}

void COptionsWindow::OnAdvancedChanged()
{
	ui.chkOpenCredentials->setEnabled(!ui.chkOpenProtectedStorage->isChecked());
	m_AdvancedChanged = true;
}

void COptionsWindow::OnNoWindowRename()
{
	if (ui.chkNoWindowRename->isChecked())
		SetAccessEntry(eWnd, "", eDirect, "#");
	else
		DelAccessEntry(eWnd, "", eDirect, "#");
	m_AdvancedChanged = true;
}

void COptionsWindow::OnDebugChanged()
{
	QCheckBox* pCheck = qobject_cast<QCheckBox*>(sender());
	m_DebugOptions[pCheck].Changed = true;
}

void COptionsWindow::OnAddProcess()
{
	QString Process = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter a program file name"));
	if (Process.isEmpty())
		return;

	ui.lstProcesses->addItem(Process);

	m_AdvancedChanged = true;
}

void COptionsWindow::OnDelProcess()
{
	foreach(QListWidgetItem* pItem, ui.lstProcesses->selectedItems())
		delete pItem;

	m_AdvancedChanged = true;
}

#include <wtypes.h>
#include <objsel.h>

void COptionsWindow::OnAddUser()
{
	QStringList Users;

	IDsObjectPicker *pObjectPicker = NULL;
	HRESULT hr = CoCreateInstance(CLSID_DsObjectPicker, NULL, CLSCTX_INPROC_SERVER, IID_IDsObjectPicker, (void **)&pObjectPicker);
	if (FAILED(hr))
		return;

	DSOP_SCOPE_INIT_INFO ScopeInit;
	memset(&ScopeInit, 0, sizeof(DSOP_SCOPE_INIT_INFO));
	ScopeInit.cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
	ScopeInit.flType = DSOP_SCOPE_TYPE_TARGET_COMPUTER | DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN | DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN;
	ScopeInit.flScope = DSOP_SCOPE_FLAG_STARTING_SCOPE | DSOP_SCOPE_FLAG_DEFAULT_FILTER_USERS | DSOP_SCOPE_FLAG_DEFAULT_FILTER_GROUPS;
	ScopeInit.FilterFlags.Uplevel.flBothModes = DSOP_FILTER_USERS | DSOP_FILTER_WELL_KNOWN_PRINCIPALS | DSOP_FILTER_BUILTIN_GROUPS
		| DSOP_FILTER_UNIVERSAL_GROUPS_SE | DSOP_FILTER_GLOBAL_GROUPS_SE | DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE;
	ScopeInit.FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_USERS | DSOP_DOWNLEVEL_FILTER_LOCAL_GROUPS | DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS;

	DSOP_INIT_INFO InitInfo;
	memset(&InitInfo, 0, sizeof(InitInfo));
	InitInfo.cbSize = sizeof(InitInfo);
	InitInfo.pwzTargetComputer = NULL;
	InitInfo.cDsScopeInfos = 1;
	InitInfo.aDsScopeInfos = &ScopeInit;
	InitInfo.flOptions = DSOP_FLAG_MULTISELECT;

	hr = pObjectPicker->Initialize(&InitInfo);

	if (SUCCEEDED(hr))
	{
		IDataObject *pDataObject = NULL;
		hr = pObjectPicker->InvokeDialog((HWND)this->winId(), &pDataObject);
		if (SUCCEEDED(hr) && pDataObject)
		{
			FORMATETC formatEtc;
			formatEtc.cfFormat = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DSOP_DS_SELECTION_LIST);
			formatEtc.ptd = NULL;
			formatEtc.dwAspect = DVASPECT_CONTENT;
			formatEtc.lindex = -1;
			formatEtc.tymed = TYMED_HGLOBAL;

			STGMEDIUM stgMedium;
			hr = pDataObject->GetData(&formatEtc, &stgMedium);
			if (SUCCEEDED(hr))
			{
				PDS_SELECTION_LIST pResults = (PDS_SELECTION_LIST)GlobalLock(stgMedium.hGlobal);
				if (pResults)
				{
					for (ULONG i = 0; i < pResults->cItems; i++)
						Users.append(QString::fromWCharArray(pResults->aDsSelection[i].pwzName));
					GlobalUnlock(stgMedium.hGlobal);
				}
			}
			pDataObject->Release();
		}
	}
	pObjectPicker->Release();


	if (Users.isEmpty())
		return;

	ui.lstUsers->addItems(Users);
}

void COptionsWindow::OnDelUser()
{
	foreach(QListWidgetItem* pItem, ui.lstUsers->selectedItems())
		delete pItem;
}

void COptionsWindow::LoadTemplates()
{
	m_AllTemplates.clear();
	ui.cmbCategories->clear();

	QStringList Templates;
	for (int index = 0; ; index++)
	{
		QString Value = m_pBox->GetAPI()->SbieIniGet("", "", index);
		if (Value.isNull())
			break;
		Templates.append(Value);
	}

	for (QStringList::iterator I = Templates.begin(); I != Templates.end();)
	{
		if (I->left(9).compare("Template_", Qt::CaseInsensitive) != 0 || *I == "Template_KnownConflicts") {
			I = Templates.erase(I);
			continue;
		}
	
		QString Name = *I++;
		QString Category = m_pBox->GetAPI()->SbieIniGet(Name, "Tmpl.Class", 0x40000000L); // CONF_GET_NO_GLOBAL);
		QString Title = m_pBox->GetAPI()->SbieIniGet(Name, "Tmpl.Title", 0x40000000L); // CONF_GET_NO_GLOBAL);

		if (Title.left(1) == "#")
		{
			int End = Title.mid(1).indexOf(",");
			if (End == -1) End = Title.length() - 1;
			int MsgNum = Title.mid(1, End).toInt();
			Title = theAPI->GetSbieMsgStr(MsgNum, theGUI->m_LanguageId).arg(Title.mid(End + 2)).arg("");
		}
		if (Title.isEmpty()) Title = Name;
		//else Title += " (" + Name + ")";
		if (Title == "-")
			continue; // skip separators

		m_AllTemplates.insertMulti(Category, qMakePair(Name, Title));
	}
	
	ui.cmbCategories->addItem(tr("All Categories"), "");
	ui.cmbCategories->setCurrentIndex(0);
	foreach(const QString& Category, m_AllTemplates.uniqueKeys())
	{
		if (Category.isEmpty()) 
			continue;
		ui.cmbCategories->addItem(Category, Category);
	}

	m_GlobalTemplates = m_pBox->GetAPI()->GetGlobalSettings()->GetTextList("Template", false);
	m_BoxTemplates = m_pBox->GetTextList("Template", false);

	ShowTemplates();
}

void COptionsWindow::ShowTemplates()
{
	ui.treeTemplates->clear();

	QString Category = ui.cmbCategories->currentData().toString();
	QString Filter = ui.txtTemplates->text();

	for (QMultiMap<QString, QPair<QString, QString>>::iterator I = m_AllTemplates.begin(); I != m_AllTemplates.end(); ++I)
	{
		if (!Category.isEmpty() && I.key().compare(Category, Qt::CaseInsensitive) != 0)
			continue;

		QString Name = I.value().first.mid(9);

		if (!Name.isEmpty() && Name.indexOf(Filter, 0, Qt::CaseInsensitive) == -1)
			continue;

		QTreeWidgetItem* pItem = new QTreeWidgetItem();
		pItem->setText(0, I.key());
		pItem->setData(1, Qt::UserRole, I.value().first);
		pItem->setText(1, I.value().second);
		//pItem->setFlags(pItem->flags() | Qt::ItemIsUserCheckable);
		if(m_GlobalTemplates.contains(Name))
			pItem->setCheckState(1, Qt::PartiallyChecked);
		else if (m_BoxTemplates.contains(Name))
			pItem->setCheckState(1, Qt::Checked);
		else
			pItem->setCheckState(1, Qt::Unchecked);
		ui.treeTemplates->addTopLevelItem(pItem);
	}
}

void COptionsWindow::OnTemplateClicked(QTreeWidgetItem* pItem, int Column)
{
	QString Name = pItem->data(1, Qt::UserRole).toString().mid(9);
	if (m_GlobalTemplates.contains(Name)) {
		QMessageBox::warning(this, "SandboxiePlus", tr("This template is enabled globally. To configure it, use the global options."));
		pItem->setCheckState(1, Qt::PartiallyChecked);
		return;
	}

	if (pItem->checkState(1) == Qt::Checked) {
		if (!m_BoxTemplates.contains(Name)) {
			m_BoxTemplates.append(Name);
			m_TemplatesChanged = true;
		}
	}
	else if (pItem->checkState(1) == Qt::Unchecked) {
		if (m_BoxTemplates.contains(Name)) {
			m_BoxTemplates.removeAll(Name);
			m_TemplatesChanged = true;
		}
	}
}

void COptionsWindow::OnTemplateDoubleClicked(QTreeWidgetItem* pItem, int Column)
{
	QSharedPointer<CSbieIni> pTemplate = QSharedPointer<CSbieIni>(new CSbieIni(pItem->data(1, Qt::UserRole).toString(), m_pBox->GetAPI()));

	COptionsWindow* pOptionsWindow = new COptionsWindow(pTemplate, pItem->text(1), this);
	pOptionsWindow->show();
}

void COptionsWindow::SaveTemplates()
{
	m_pBox->UpdateTextList("Template", m_BoxTemplates, m_Template);

	m_TemplatesChanged = false;
}

void COptionsWindow::OnTab()
{
	if (ui.tabs->currentWidget() == ui.tabEdit)
	{
		LoadIniSection();
		ui.txtIniSection->setReadOnly(true);
	}
	else 
	{
		if (m_ConfigDirty)
			LoadConfig();

		if (ui.tabs->currentWidget() == ui.tabStart)
		{
			if(GetAccessEntry(eIPC, "!<StartRunAccess>", eClosed, "*") != NULL)
				ui.radStartSelected->setChecked(true);
			else if (GetAccessEntry(eIPC, "<StartRunAccess>", eClosed, "*") != NULL)
				ui.radStartExcept->setChecked(true);
			else
				ui.radStartAll->setChecked(true);
			CopyGroupToList("<StartRunAccess>", ui.treeStart);
		}
		else if (ui.tabs->currentWidget() == ui.tabInternet)
		{
			ui.chkBlockINet->setChecked(GetAccessEntry(eFile, "!<InternetAccess>", eClosed, "InternetAccessDevices") != NULL);
			CopyGroupToList("<InternetAccess>", ui.treeINet);
		}
		else if (ui.tabs->currentWidget() == ui.tabAdvanced)
		{
			if (GetAccessEntry(eWnd, "", eDirect, "*") != NULL)
			{
				ui.chkNoWindowRename->setEnabled(false);
				ui.chkNoWindowRename->setChecked(true);
			}
			else
			{
				ui.chkNoWindowRename->setEnabled(true);
				ui.chkNoWindowRename->setChecked(GetAccessEntry(eWnd, "", eDirect, "#") != NULL);
			}
		}
	}
}

void COptionsWindow::SetIniEdit(bool bEnable)
{
	for (int i = 0; i < ui.tabs->count() - 1; i++) {
		bool Enabled = ui.tabs->widget(i)->isEnabled();
		ui.tabs->setTabEnabled(i, !bEnable && Enabled);
		ui.tabs->widget(i)->setEnabled(Enabled);
	}
	ui.btnSaveIni->setEnabled(bEnable);
	ui.btnCancelEdit->setEnabled(bEnable);
	ui.txtIniSection->setReadOnly(!bEnable);
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

void COptionsWindow::OnCancelEdit()
{
	SetIniEdit(false);
}

void COptionsWindow::LoadIniSection()
{
	QString Section;

	m_Settings = m_pBox->GetIniSection(NULL, m_Template);
	
	for (QList<QPair<QString, QString>>::const_iterator I = m_Settings.begin(); I != m_Settings.end(); ++I)
		Section += I->first + "=" + I->second + "\n";

	ui.txtIniSection->setPlainText(Section);
}

void COptionsWindow::SaveIniSection()
{
	m_ConfigDirty = true;

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

		m_pBox->InsertText(Settings.first, Settings.second);
	}

	//for (QList<QPair<QString, QString>>::const_iterator I = OldSettings.begin(); I != OldSettings.end(); ++I)
	//	m_pBox->DelValue(I->first, I->second);
	//
	//for (QList<QPair<QString, QString>>::const_iterator I = NewSettings.begin(); I != NewSettings.end(); ++I)
	//	m_pBox->InsertText(I->first, I->second);

	LoadIniSection();
}
