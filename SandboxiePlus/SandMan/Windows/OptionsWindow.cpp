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
	QSize sizeFromContents(ContentsType type, const QStyleOption* option,
		const QSize& size, const QWidget* widget) const {
		QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
		if (type == QStyle::CT_TabBarTab) {
			s.transpose();
			s.setHeight(s.height() * 15 / 10);
		}
		return s;
	}

	void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
		if (element == CE_TabBarTabLabel) {
			if (const QStyleOptionTab* tab = qstyleoption_cast<const QStyleOptionTab*>(option)) {
				QStyleOptionTab opt(*tab);
				opt.shape = QTabBar::RoundedNorth;
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
	

	QWidget* centralWidget = new QWidget();
	ui.setupUi(centralWidget);
	this->setCentralWidget(centralWidget);
	this->setWindowTitle(tr("Sandboxie Plus - '%1' Options").arg(Name));

	ui.tabs->setTabPosition(QTabWidget::West);
	ui.tabs->tabBar()->setStyle(new CustomTabStyle());

	if (m_Template)
	{
		ui.tabGeneral->setEnabled(false);
		ui.tabStart->setEnabled(false);
		ui.tabRestrictions->setEnabled(false);
		ui.tabInternet->setEnabled(false);
		ui.tabAdvanced->setEnabled(false);
		ui.tabTemplates->setEnabled(false);

		for (int i = 0; i < ui.tabs->count(); i++) 
			ui.tabs->setTabEnabled(i, ui.tabs->widget(i)->isEnabled());

		ui.tabs->setCurrentIndex(ui.tabs->indexOf(ui.tabAccess));

		ui.chkShowForceTmpl->setEnabled(false);
		ui.chkShowStopTmpl->setEnabled(false);
		ui.chkShowAccessTmpl->setEnabled(false);
	}

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
	connect(ui.txtCopyLimit, SIGNAL(textChanged(const QString&)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkNoCopyWarn, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
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
	connect(ui.chkRestrictStart, SIGNAL(clicked(bool)), this, SLOT(OnRestrictStart()));
	connect(ui.btnAddStartProg, SIGNAL(pressed()), this, SLOT(OnAddStartProg()));
	connect(ui.btnDelStartProg, SIGNAL(pressed()), this, SLOT(OnDelStartProg()));
	connect(ui.chkStartBlockMsg, SIGNAL(clicked(bool)), this, SLOT(OnStartChanged()));
	//

	// Restrictions
	connect(ui.chkBlockShare, SIGNAL(clicked(bool)), this, SLOT(OnRestrictionChanged()));
	connect(ui.chkDropRights, SIGNAL(clicked(bool)), this, SLOT(OnRestrictionChanged()));
	connect(ui.chkNoDefaultCOM, SIGNAL(clicked(bool)), this, SLOT(OnRestrictionChanged()));
	connect(ui.chkProtectSCM, SIGNAL(clicked(bool)), this, SLOT(OnRestrictionChanged()));
	connect(ui.chkProtectRpcSs, SIGNAL(clicked(bool)), this, SLOT(OnRestrictionChanged()));
	connect(ui.chkProtectSystem, SIGNAL(clicked(bool)), this, SLOT(OnRestrictionChanged()));
	//

	// INet
	connect(ui.chkBlockINet, SIGNAL(clicked(bool)), this, SLOT(OnBlockINet()));
	connect(ui.btnAddINetProg, SIGNAL(pressed()), this, SLOT(OnAddINetProg()));
	connect(ui.btnDelINetProg, SIGNAL(pressed()), this, SLOT(OnDelINetProg()));
	connect(ui.chkINetBlockMsg, SIGNAL(clicked(bool)), this, SLOT(OnINetBlockChanged()));
	//

	// Access
	connect(ui.btnAddFile, SIGNAL(pressed()), this, SLOT(OnAddFile()));
	connect(ui.btnAddKey, SIGNAL(pressed()), this, SLOT(OnAddKey()));
	connect(ui.btnAddIPC, SIGNAL(pressed()), this, SLOT(OnAddIPC()));
	connect(ui.btnAddClsId, SIGNAL(pressed()), this, SLOT(OnAddClsId()));
	connect(ui.btnAddCOM, SIGNAL(pressed()), this, SLOT(OnAddCOM()));
	// todo: add priority by order 
	ui.btnMoveUp->setVisible(false);
	ui.btnMoveDown->setVisible(false);
	connect(ui.chkShowAccessTmpl, SIGNAL(clicked(bool)), this, SLOT(OnShowAccessTmpl()));
	connect(ui.btnDelAccess, SIGNAL(pressed()), this, SLOT(OnDelAccess()));

	connect(ui.treeAccess, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(OnAccessItemClicked(QTreeWidgetItem*, int)));
	connect(ui.treeAccess, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnAccessItemDoubleClicked(QTreeWidgetItem*, int)));
	//

	// Advanced
	ui.cmbEmptyCmd->addItem("%SystemRoot%\\System32\\cmd.exe /c RMDIR /s /q \"%SANDBOX%\"");

	connect(ui.chkProtectBox, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkAutoEmpty, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.cmbEmptyCmd, SIGNAL(currentTextChanged(const QString&)), this, SLOT(OnAdvancedChanged()));
	connect(ui.btnAddUser, SIGNAL(pressed()), this, SLOT(OnAddUser()));
	connect(ui.btnDelUser, SIGNAL(pressed()), this, SLOT(OnDelUser()));
	//

	// Templates
	connect(ui.cmbCategories, SIGNAL(currentIndexChanged(int)), this, SLOT(OnFilterTemplates()));
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

	restoreGeometry(theConf->GetBlob("OptionsWindow/Window_Geometry"));
}

COptionsWindow::~COptionsWindow()
{
	theConf->SetBlob("OptionsWindow/Window_Geometry",saveGeometry());
}

void COptionsWindow::closeEvent(QCloseEvent *e)
{
	this->deleteLater();
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

		ui.txtCopyLimit->setText(QString::number(m_pBox->GetNum("CopyLimitKb", 80 * 1024)));
		ui.chkNoCopyWarn->setChecked(!m_pBox->GetBool("CopyLimitSilent", false));
	
		m_GeneralChanged = false;
	}

	LoadGroups();

	LoadForced();

	LoadStop();

	{
		ui.chkStartBlockMsg->setChecked(m_pBox->GetBool("NotifyStartRunAccessDenied", true));
	
		m_StartChanged = false;
	}

	{
		ui.chkBlockShare->setChecked(m_pBox->GetBool("BlockNetworkFiles", true));
		ui.chkDropRights->setChecked(m_pBox->GetBool("DropAdminRights", false));
		ui.chkNoDefaultCOM->setChecked(!m_pBox->GetBool("OpenDefaultClsid", true));
		ui.chkProtectSCM->setChecked(!m_pBox->GetBool("UnrestrictedSCM", false));
		ui.chkProtectRpcSs->setChecked(m_pBox->GetBool("ProtectRpcSs", false));
		ui.chkProtectSystem->setChecked(!m_pBox->GetBool("ExposeBoxedSystem", false));

		m_RestrictionChanged = false;
	}

	{
		ui.chkINetBlockMsg->setChecked(m_pBox->GetBool("NotifyInternetAccessDenied", true));
	
		m_INetBlockChanged = false;
	}

	LoadAccessList();

	{
		ui.chkProtectBox->setChecked(m_pBox->GetBool("NeverDelete", false));
		ui.chkAutoEmpty->setChecked(m_pBox->GetBool("AutoDelete", false));
		ui.cmbEmptyCmd->setCurrentText(m_pBox->GetText("DeleteCommand", ""));

		QStringList Users = m_pBox->GetText("Enabled").split(",");
		ui.lstUsers->clear();
		if (Users.count() > 1)
			ui.lstUsers->addItems(Users.mid(1));

		m_AdvancedChanged = false;
	}

	{
		LoadTemplates();
		m_TemplatesChanged = false;
	}
}

void COptionsWindow::SaveConfig()
{
	if (m_GeneralChanged)
	{
		m_pBox->SetText("BoxNameTitle", ui.cmbBoxIndicator->currentData().toString());

		QStringList BorderCfg;
		BorderCfg.append(QString("#%1,%2,%3").arg(m_BorderColor.blue(), 2, 16, QChar('0')).arg(m_BorderColor.green(), 2, 16, QChar('0')).arg(m_BorderColor.red(), 2, 16, QChar('0')));
		BorderCfg.append(ui.cmbBoxBorder->currentData().toString());
		//BorderCfg.append(5) // width
		m_pBox->SetText("BorderColor", BorderCfg.join(","));

		m_pBox->SetNum("CopyLimitKb", ui.txtCopyLimit->text().toInt());
		m_pBox->SetBool("CopyLimitSilent", ui.chkNoCopyWarn->isChecked());

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

	if (m_RestrictionChanged)
	{
		m_pBox->SetBool("BlockNetworkFiles", ui.chkBlockShare->isChecked());
		m_pBox->SetBool("DropAdminRights", ui.chkDropRights->isChecked());
		m_pBox->SetBool("OpenDefaultClsid", ui.chkNoDefaultCOM->isChecked());
		m_pBox->SetBool("UnrestrictedSCM", ui.chkProtectSCM->isChecked());
		m_pBox->SetBool("ProtectRpcSs", ui.chkProtectRpcSs->isChecked());
		m_pBox->SetBool("ExposeBoxedSystem", ui.chkProtectSystem->isChecked());

		m_RestrictionChanged = false;
	}

	if (m_INetBlockChanged)
	{
		m_pBox->SetBool("NotifyInternetAccessDenied", ui.chkINetBlockMsg->isChecked());

		m_INetBlockChanged = false;
	}

	if (m_AccessChanged)
		SaveAccessList();

	if (m_AdvancedChanged)
	{
		m_pBox->SetBool("NeverDelete", ui.chkProtectBox->isChecked());
		m_pBox->SetBool("AutoDelete", ui.chkAutoEmpty->isChecked());
		m_pBox->SetText("DeleteCommand", ui.cmbEmptyCmd->currentText());

		QStringList Users;
		for (int i = 0; i < ui.lstUsers->count(); i++)
			Users.append(ui.lstUsers->item(i)->text());
		m_pBox->SetText("Enabled", Users.count() > 0 ? "y," + Users.join(",") : "y");

		m_AdvancedChanged = false;
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

	ui.lblCopyLimit->setText(tr("kilobytes (%1)").arg(FormatSize(ui.txtCopyLimit->text().toInt() * 1024)));
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

void COptionsWindow::SetProgramItem(QString Program, QTreeWidgetItem* pItem, int Column)
{
	pItem->setData(Column, Qt::UserRole, Program);
	if (Program.left(1) == "<")
		Program = tr("Group: %1").arg(Program.mid(1, Program.length() - 2));
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
		for (int i = 1; i < Entries.count(); i++) 
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

	m_pBox->UpdateTextList("ProcessGroup", ProcessGroups);

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
		QMessageBox::warning(this, "SandboxiePlus", tr("Please sellect group first."));
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

	m_pBox->UpdateTextList("ForceProcess", ForceProcess);
	m_pBox->UpdateTextList("ForceFolder", ForceFolder);

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
	QString Value = QFileDialog::getExistingDirectory(this, tr("Select Directory"));
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
	for (int i = 0; i < ui.treeForced->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeForced->topLevelItem(i);
		int Type = pItem->data(0, Qt::UserRole).toInt();
		if (Type == -1)
			continue; // entry from template
		switch (Type)
		{
		case 1:	LingerProcess.append(pItem->data(1, Qt::UserRole).toString()); break;
		case 2: LeaderProcess.append(pItem->data(1, Qt::UserRole).toString()); break;
		}
	}

	m_pBox->UpdateTextList("LingerProcess", LingerProcess);
	m_pBox->UpdateTextList("LeaderProcess", LeaderProcess);

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
	bool Enable = ui.chkRestrictStart->isChecked();
	if (Enable)
		SetAccessEntry(eIPC, "!<StartRunAccess>", eClosed, "*");
	else
		DelAccessEntry(eIPC, "!<StartRunAccess>", eClosed, "*");
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

	case eOpenClsid:		return "OpenClsid";
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
	case eOpenPipePath:		Type = eFile;	Mode = eFull;	break;
	case eClosedFilePath:	Type = eFile;	Mode = eClosed;	break;
	case eReadFilePath:		Type = eFile;	Mode = eReadOnly; break;
	case eWriteFilePath:	Type = eFile;	Mode = eWriteOnly; break;

	case eOpenKeyPath:		Type = eKey;	Mode = eDirect;	break;
	case eClosedKeyPath:	Type = eKey;	Mode = eClosed;	break;
	case eReadKeyPath:		Type = eKey;	Mode = eReadOnly; break;
	case eWriteKeyPath:		Type = eKey;	Mode = eWriteOnly; break;

	case eOpenIpcPath:		Type = eIPC;	Mode = eDirect;	break;
	case eClosedIpcPath:	Type = eIPC;	Mode = eClosed;	break;

	case eOpenWinClass:		Type = eWndCls;	Mode = eDirect;	break;

	case eOpenClsid:		Type = eClsId;	Mode = eDirect;	break;

	default:				return;
	}

	QStringList Values = Value.split(",");
	if (Values.count() >= 2) 
		AddAccessEntry(Type, Mode, Values[0], Values[1], Template);
	else // all programs
		AddAccessEntry(Type, Mode, "", Values[0], Template);
}

QString COptionsWindow::GetAccessModeStr(EAccessMode Mode)
{
	switch (Mode)
	{
	case eDirect:		return "Direct";
	case eFull:			return "Full";
	case eClosed:		return "Closed";
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
	case eWndCls:		return "Wnd Class";
	case eClsId:		return "COM Object";
	}
	return "Unknown";
}

void COptionsWindow::AddAccessEntry(EAccessType	Type, EAccessMode Mode, QString Program, const QString& Path, const QString& Template)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem();

	pItem->setText(0, GetAccessTypeStr(Type) + (Template.isEmpty() ? "" : " (" + Template + ")"));
	pItem->setData(0, Qt::UserRole, !Template.isEmpty() ? -1 : (int)Type);

	pItem->setData(1, Qt::UserRole, Program);
	if (Program.isEmpty())
		Program = tr("All Programs");
	bool Not = Program.left(1) == "!";
	if (Not)
		Program.remove(0, 1);
	if (Program.left(1) == "<")
		Program = tr("Group: %1").arg(Program.mid(1, Program.length() - 2));
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
		case eFull:			return "OpenPipePath";
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
	case eWndCls:
		switch (Mode)
		{
		case eDirect:		return "OpenWinClass";
		}
		break;
	case eClsId:
		switch (Mode)
		{
		case eDirect:		return "OpenClsid";
		}
		break;
	}
	return "Unknown";
}

void COptionsWindow::SaveAccessList()
{
	QMultiMap<QString, QString> AccessMap;
	for (int i = 0; i < ui.treeAccess->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeAccess->topLevelItem(i);
		int Type = pItem->data(0, Qt::UserRole).toInt();
		if (Type == -1)
			continue; // entry from template
		int Mode = pItem->data(2, Qt::UserRole).toInt();
		QString Program = pItem->data(1, Qt::UserRole).toString();
		QString Value = pItem->data(3, Qt::UserRole).toString();
		if (Program.isEmpty())
			Value.prepend(Program + ",");
		AccessMap.insertMulti(MakeAccessStr((EAccessType)Type, (EAccessMode)Mode), Value);
	}

	foreach(const QString& Key, AccessMap.uniqueKeys())
		m_pBox->UpdateTextList(Key, AccessMap.values(Key));
	
	m_AccessChanged = false;
}

void COptionsWindow::OnAccessItemClicked(QTreeWidgetItem* pItem, int Column)
{
	if (Column != 0)
		return;

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
	if(Index != -1)
		Program = pCombo->itemData(Index, Qt::UserRole).toString();

	pItem->setText(1, (pNot->isChecked() ? "NOT " : "") + pCombo->currentText());
	pItem->setData(1, Qt::UserRole, (pNot->isChecked() ? "!" : "") + Program);
	pItem->setText(2, GetAccessModeStr((EAccessMode)pMode->currentData().toInt()));
	pItem->setData(2, Qt::UserRole, pMode->currentData());
	pItem->setText(3, pPath->text());
	pItem->setData(3, Qt::UserRole, pPath->text());

	ui.treeAccess->setItemWidget(pItem, 1, NULL);
	ui.treeAccess->setItemWidget(pItem, 2, NULL);
	ui.treeAccess->setItemWidget(pItem, 3, NULL);

	m_AccessChanged = true;
}

QList<COptionsWindow::EAccessMode> COptionsWindow::GetAccessModes(EAccessType Type)
{
	switch (Type)
	{
	case eFile:			return QList<EAccessMode>() << eDirect << eFull << eClosed << eReadOnly << eWriteOnly;
	case eKey:			return QList<EAccessMode>() << eDirect << eClosed << eReadOnly << eWriteOnly;
	case eIPC:			return QList<EAccessMode>() << eDirect << eClosed;
	}
	return QList<EAccessMode>() << eDirect;
}

void COptionsWindow::OnAccessItemDoubleClicked(QTreeWidgetItem* pItem, int Column)
{
	if (Column == 0)
		return;

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
	
	// todo: add recently ran programs or programs from other configs

	for (int i = 0; i < ui.treeGroups->topLevelItemCount(); i++) {
		QTreeWidgetItem* pItem = ui.treeGroups->topLevelItem(i);
		pCombo->addItem(tr("Group: %1").arg(pItem->text(0)), "<" + pItem->text(0) + ">");
	}

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

void COptionsWindow::OnAdvancedChanged()
{
	m_AdvancedChanged = true;

	ui.chkAutoEmpty->setEnabled(!ui.chkProtectBox->isChecked());
	ui.cmbEmptyCmd->setEnabled(!ui.chkProtectBox->isChecked());
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
			Title = m_pBox->GetAPI()->GetSbieMessage(MsgNum, Title.mid(End+2));
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

	for (QMultiMap<QString, QPair<QString, QString>>::iterator I = m_AllTemplates.begin(); I != m_AllTemplates.end(); ++I)
	{
		if (!Category.isEmpty() && I.key().compare(Category, Qt::CaseInsensitive) != 0)
			continue;

		QString Name = I.value().first.mid(9);

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
		QMessageBox::warning(this, "SandboxiePlus", tr("This template is enabled globally to configure it use the global options."));
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
	m_pBox->UpdateTextList("Template", m_BoxTemplates);

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
			ui.chkRestrictStart->setChecked(GetAccessEntry(eIPC, "!<StartRunAccess>", eClosed, "*") != NULL);
			CopyGroupToList("<StartRunAccess>", ui.treeStart);
		}
		else if (ui.tabs->currentWidget() == ui.tabInternet)
		{
			ui.chkBlockINet->setChecked(GetAccessEntry(eFile, "!<InternetAccess>", eClosed, "InternetAccessDevices") != NULL);
			CopyGroupToList("<InternetAccess>", ui.treeINet);
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

	// Note: an incremental update would be more elegat but it would change the entry order in the ini,
	//			hence its better for the user to fully rebuild the section each time.
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