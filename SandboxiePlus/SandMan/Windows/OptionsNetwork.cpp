#include "stdafx.h"
#include "OptionsWindow.h"
#include "SandMan.h"
#include "SettingsWindow.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/Common.h"
#include "../MiscHelpers/Common/ComboInputDialog.h"
#include "../MiscHelpers/Common/SettingsWidgets.h"
#include "Helpers/WinAdmin.h"


void COptionsWindow::CreateNetwork()
{
	//connect(ui.chkBlockINet, SIGNAL(clicked(bool)), this, SLOT(OnBlockINet()));
	connect(ui.cmbBlockINet, SIGNAL(currentIndexChanged(int)), this, SLOT(OnBlockINet()));
	connect(ui.btnAddINetProg, SIGNAL(clicked(bool)), this, SLOT(OnAddINetProg()));
	connect(ui.btnDelINetProg, SIGNAL(clicked(bool)), this, SLOT(OnDelINetProg()));
	connect(ui.chkINetBlockPrompt, SIGNAL(clicked(bool)), this, SLOT(OnINetBlockChanged()));
	connect(ui.chkINetBlockMsg, SIGNAL(clicked(bool)), this, SLOT(OnINetBlockChanged()));

	connect(ui.treeINet, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnINetItemDoubleClicked(QTreeWidgetItem*, int)));
	connect(ui.treeINet, SIGNAL(itemSelectionChanged()), this, SLOT(OnINetSelectionChanged()));
	connect(ui.treeINet, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(OnINetChanged(QTreeWidgetItem *, int)));

	connect(ui.btnAddFwRule, SIGNAL(clicked(bool)), this, SLOT(OnAddNetFwRule()));
	connect(ui.btnDelFwRule, SIGNAL(clicked(bool)), this, SLOT(OnDelNetFwRule()));
	connect(ui.treeNetFw, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnNetFwItemDoubleClicked(QTreeWidgetItem*, int)));
	connect(ui.treeNetFw, SIGNAL(itemSelectionChanged()), this, SLOT(OnNetFwSelectionChanged()));
	connect(ui.treeNetFw, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(OnNetFwChanged(QTreeWidgetItem *, int)));

	connect(ui.chkShowNetFwTmpl, SIGNAL(clicked(bool)), this, SLOT(OnShowNetFwTmpl()));

	connect(ui.txtProgFwTest, SIGNAL(textChanged(const QString&)), this, SLOT(OnTestNetFwRule()));
	connect(ui.txtPortFwTest, SIGNAL(textChanged(const QString&)), this, SLOT(OnTestNetFwRule()));
	connect(ui.txtIPFwTest, SIGNAL(textChanged(const QString&)), this, SLOT(OnTestNetFwRule()));
	connect(ui.cmbProtFwTest, SIGNAL(currentIndexChanged(int)), this, SLOT(OnTestNetFwRule()));
	ui.cmbProtFwTest->addItem(tr("Any"), (int)eAny);
	ui.cmbProtFwTest->addItem(tr("TCP"), (int)eTcp);
	ui.cmbProtFwTest->addItem(tr("UDP"), (int)eUdp);
	ui.cmbProtFwTest->addItem(tr("ICMP"), (int)eIcmp);
	connect(ui.btnClearFwTest, SIGNAL(clicked(bool)), this, SLOT(OnClearNetFwTest()));
}

void COptionsWindow::LoadINetAccess()
{
	m_IsEnabledWFP = m_pBox->GetAPI()->GetGlobalSettings()->GetBool("NetworkEnableWFP", false);
	// check if we are blocking globally and if so adapt the behaviour accordingly
	m_WFPisBlocking = !m_pBox->GetAPI()->GetGlobalSettings()->GetBool("AllowNetworkAccess", true); 
	
	ui.lblNoWfp->setVisible(!m_IsEnabledWFP); // warn user that this is only user mode

	ui.cmbBlockINet->clear();
	ui.cmbBlockINet->addItem(tr("Allow access"), 0);
	if (m_IsEnabledWFP) ui.cmbBlockINet->addItem(tr("Block using Windows Filtering Platform"), 1);
	ui.cmbBlockINet->addItem(tr("Block by denying access to Network devices"), 2);

	m_INetBlockChanged = false;
}

void COptionsWindow::SaveINetAccess()
{
	int Mode = ui.cmbBlockINet->currentData().toInt();
	if (Mode == 1)				m_pBox->InsertText("AllowNetworkAccess", "!<InternetAccess>,n");
	else						m_pBox->DelValue("AllowNetworkAccess", "!<InternetAccess>,n");
	if (Mode != 0)				m_pBox->DelValue("AllowNetworkAccess", "y");
	else if (m_WFPisBlocking)	m_pBox->InsertText("AllowNetworkAccess", "y");

	WriteAdvancedCheck(ui.chkINetBlockPrompt, "PromptForInternetAccess", "y", "");
	WriteAdvancedCheck(ui.chkINetBlockMsg, "NotifyInternetAccessDenied", "", "n");

	m_INetBlockChanged = false;
}

int COptionsWindow::GroupToINetMode(const QString& Mode)
{
	if (Mode.compare("<InternetAccess>", Qt::CaseInsensitive) == 0)		return 0;
	if (Mode.compare("<BlockNetAccess>", Qt::CaseInsensitive) == 0)		return 1;
	if (Mode.compare("<BlockNetDevices>", Qt::CaseInsensitive) == 0)	return 2;
	if (Mode.compare("<InternetAccessDisabled>", Qt::CaseInsensitive) == 0)		return 0 | 0x10;
	if (Mode.compare("<BlockNetAccessDisabled>", Qt::CaseInsensitive) == 0)		return 1 | 0x10;
	if (Mode.compare("<BlockNetDevicesDisabled>", Qt::CaseInsensitive) == 0)	return 2 | 0x10;
	return -1;
}

QString COptionsWindow::INetModeToGroup(int Mode)
{
	switch (Mode)
	{
	case 0:				return "<InternetAccess>";
	case 1:				return "<BlockNetAccess>";
	case 2:				return "<BlockNetDevices>";
	case 0 | 0x10:		return "<InternetAccessDisabled>";
	case 1 | 0x10:		return "<BlockNetAccessDisabled>";
	case 2 | 0x10:		return "<BlockNetDevicesDisabled>";
	}
	return "";
}

void COptionsWindow::LoadBlockINet()
{
	if (GetAccessEntry(eFile, "!<InternetAccess>", eClosed, "InternetAccessDevices") != NULL)
		ui.cmbBlockINet->setCurrentIndex(ui.cmbBlockINet->findData(2));
	else if (m_IsEnabledWFP && (FindEntryInSettingList("AllowNetworkAccess", "!<InternetAccess>,n") 
		|| (m_WFPisBlocking && !FindEntryInSettingList("AllowNetworkAccess", "y"))))
		ui.cmbBlockINet->setCurrentIndex(ui.cmbBlockINet->findData(1));
	else
		ui.cmbBlockINet->setCurrentIndex(ui.cmbBlockINet->findData(0));

	//ui.chkBlockINet->setChecked();
	//ui.chkINetBlockPrompt->setEnabled(ui.chkBlockINet->isChecked());
	ui.chkINetBlockPrompt->setEnabled(ui.cmbBlockINet->currentIndex() != 0);
	ui.chkINetBlockPrompt->setChecked(m_pBox->GetBool("PromptForInternetAccess", false));
	//ui.chkINetBlockMsg->setEnabled(ui.chkBlockINet->isChecked());
	ui.chkINetBlockMsg->setEnabled(ui.cmbBlockINet->currentIndex() != 0);
	ui.chkINetBlockMsg->setChecked(m_pBox->GetBool("NotifyInternetAccessDenied", true));
	

	ui.treeINet->clear();
	for (int i = 0; i < ui.treeGroups->topLevelItemCount(); i++) 
	{
		QTreeWidgetItem* pGroupItem = ui.treeGroups->topLevelItem(i);
		int Mode = GroupToINetMode(pGroupItem->data(0, Qt::UserRole).toString());
		if (Mode == -1)
			continue;
		
		for (int j = 0; j < pGroupItem->childCount(); j++)
		{
			QString Value = pGroupItem->child(j)->data(0, Qt::UserRole).toString();

			QTreeWidgetItem* pItem = new QTreeWidgetItem();
			pItem->setCheckState(0, (Mode & 0x10) != 0 ? Qt::Unchecked : Qt::Checked);
			
			SetProgramItem(Value, pItem, 0);
	
			pItem->setData(1, Qt::UserRole, Mode);
			if (!m_IsEnabledWFP && Mode == 1) Mode = -1; // this mode is not available
			pItem->setText(1, GetINetModeStr(Mode));

			ui.treeINet->addTopLevelItem(pItem);
		}
	}
}

QString COptionsWindow::GetINetModeStr(int Mode)
{
	switch (Mode)
	{
	case 0:		return tr("Allow");
	case 1:		return tr("Block (WFP)"); // Windows Filtering Platform
	case 2:		return tr("Block (NDev)"); // Network Devices
	}
	return tr("");
}

void COptionsWindow::OnINetItemDoubleClicked(QTreeWidgetItem* pItem, int Column)
{
	QString Program = pItem->data(0, Qt::UserRole).toString();

	//QWidget* pProgram = new QWidget();
	//pProgram->setAutoFillBackground(true);
	//QHBoxLayout* pLayout = new QHBoxLayout();
	//pLayout->setContentsMargins(0,0,0,0);
	//pLayout->setSpacing(0);
	//pProgram->setLayout(pLayout);
	//QComboBox* pCombo = new QComboBox(pProgram);
	QComboBox* pCombo = new QComboBox();

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
	//pLayout->addWidget(pCombo);

	//ui.treeINet->setItemWidget(pItem, 0, pProgram);
	ui.treeINet->setItemWidget(pItem, 0, pCombo);

	QComboBox* pMode = new QComboBox();
	for (int i = 0; i < 3; i++) {
		if (!m_IsEnabledWFP && i == 1) continue; // this mode is not available
		pMode->addItem(GetINetModeStr(i), i);
	}
	pMode->setCurrentIndex(pMode->findData(pItem->data(1, Qt::UserRole)));
	ui.treeINet->setItemWidget(pItem, 1, pMode);
}

void COptionsWindow::OnINetChanged(QTreeWidgetItem* pItem, int Column)
{
	if (Column != 0)
		return;

	if (pItem->checkState(0) == Qt::Checked) {
		QString Program = pItem->data(0, Qt::UserRole).toString();
		int Mode = pItem->data(1, Qt::UserRole).toInt();
		Mode |= 0x10;
		if (DelProgramFromGroup(Program, INetModeToGroup(Mode))) {
			Mode &= ~0x10;
			AddProgramToGroup(Program, INetModeToGroup(Mode));
		}
	}
	else {
		QString Program = pItem->data(0, Qt::UserRole).toString();
		int Mode = pItem->data(1, Qt::UserRole).toInt();
		Mode &= ~0x10;
		if (DelProgramFromGroup(Program, INetModeToGroup(Mode))) {
			Mode |= 0x10;
			AddProgramToGroup(Program, INetModeToGroup(Mode));
		}
	}
}

void COptionsWindow::CloseINetEdit(bool bSave)
{
	for (int i = 0; i < ui.treeINet->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeINet->topLevelItem(i);
		CloseINetEdit(pItem, bSave);
	}
}

void COptionsWindow::CloseINetEdit(QTreeWidgetItem* pItem, bool bSave)
{
	QWidget* pProgram = ui.treeINet->itemWidget(pItem, 0);
	if (!pProgram)
		return;

	if (bSave)
	{
		//QHBoxLayout* pLayout = (QHBoxLayout*)pProgram->layout();
		//QComboBox* pCombo = (QComboBox*)pLayout->itemAt(0)->widget();
		QComboBox* pCombo = (QComboBox*)pProgram;

		QComboBox* pMode = (QComboBox*)ui.treeINet->itemWidget(pItem, 1);

		QString OldProgram = pItem->data(0, Qt::UserRole).toString();
		int OldMode = pItem->data(1, Qt::UserRole).toInt();
		if (pItem->checkState(0) == Qt::Unchecked)
			OldMode |= 0x10;
		DelProgramFromGroup(OldProgram, INetModeToGroup(OldMode));


		QString NewProgram = pCombo->currentText();
		int NewMode = pMode->currentData().toInt();
		if (pItem->checkState(0) == Qt::Unchecked)
			NewMode |= 0x10;
		AddProgramToGroup(NewProgram, INetModeToGroup(NewMode));


		SetProgramItem(NewProgram, pItem, 0);
	
		pItem->setText(1, GetINetModeStr(NewMode));
		pItem->setData(1, Qt::UserRole, NewMode);
	}

	ui.treeINet->setItemWidget(pItem, 0, NULL);
	ui.treeINet->setItemWidget(pItem, 1, NULL);
}

void COptionsWindow::OnBlockINet()
{
	if (m_HoldChange)
		return;

	//bool Enable = ui.chkBlockINet->isChecked();

	int Mode = ui.cmbBlockINet->currentData().toInt();

	ui.chkINetBlockPrompt->setEnabled(Mode != 0);
	ui.chkINetBlockMsg->setEnabled(Mode != 0);
	if (Mode == 2)
		SetAccessEntry(eFile, "!<InternetAccess>", eClosed, "InternetAccessDevices");
	else
		DelAccessEntry(eFile, "!<InternetAccess>", eClosed, "InternetAccessDevices");

	m_INetBlockChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnAddINetProg()
{
	QString Value = SelectProgram();
	if (Value.isEmpty())
		return;

	int Mode = 0;

	QTreeWidgetItem* pItem = new QTreeWidgetItem();
			
	SetProgramItem(Value, pItem, 0);
	
	pItem->setText(1, GetINetModeStr(Mode));
	pItem->setData(1, Qt::UserRole, Mode);

	pItem->setCheckState(0, Qt::Checked);
	ui.treeINet->addTopLevelItem(pItem);

	AddProgramToGroup(Value, INetModeToGroup(Mode));

	//m_INetBlockChanged = true;
	//OnOptChanged();
}

void COptionsWindow::OnDelINetProg()
{
	QTreeWidgetItem* pItem = ui.treeINet->currentItem();
	if (!pItem)
		return;

	QString OldProgram = pItem->data(0, Qt::UserRole).toString();
	int OldMode = pItem->data(1, Qt::UserRole).toInt();
	if (pItem->checkState(0) == Qt::Unchecked)
		OldMode |= 0x10;
	DelProgramFromGroup(OldProgram, INetModeToGroup(OldMode));

	delete pItem;

	//m_INetBlockChanged = true;
	//OnOptChanged();
}

bool COptionsWindow::FindEntryInSettingList(const QString& Name, const QString& Value)
{
	QStringList Settings = m_pBox->GetTextList(Name, false);
	foreach(const QString & Setting, Settings) {
		if (Setting.compare(Value, Qt::CaseInsensitive) == 0) 
			return true;
	}
	return false;
}

void COptionsWindow::CheckINetBlock()
{
	SetAccessEntry(eFile, "<BlockNetDevices>", eClosed, "InternetAccessDevices");

	if (m_IsEnabledWFP && !FindEntryInSettingList("AllowNetworkAccess", "<BlockNetAccess>,n"))
		m_pBox->InsertText("AllowNetworkAccess", "<BlockNetAccess>,n");
}

void COptionsWindow::LoadNetFwRules()
{
	ui.treeNetFw->clear();
	foreach(const QString & Value, m_pBox->GetTextList("NetworkAccess", m_Template))
		ParseAndAddFwRule(Value);

	foreach(const QString & Value, m_pBox->GetTextList("NetworkAccessDisabled", m_Template))
		ParseAndAddFwRule(Value, true);

	LoadNetFwRulesTmpl();

	m_NetFwRulesChanged = false;
}

void COptionsWindow::LoadNetFwRulesTmpl(bool bUpdate)
{
	if (ui.chkShowNetFwTmpl->isChecked())
	{
		foreach(const QString& Template, m_pBox->GetTemplates())
		{
			foreach(const QString& Value, m_pBox->GetTextListTmpl("NetworkAccess", Template))
				ParseAndAddFwRule(Value, false, Template);
		}
	}
	else if (bUpdate)
	{
		for (int i = 0; i < ui.treeNetFw->topLevelItemCount(); )
		{
			QTreeWidgetItem* pItem = ui.treeNetFw->topLevelItem(i);
			int Type = pItem->data(1, Qt::UserRole).toInt();
			if (Type == -1) {
				delete pItem;
				continue; // entry from template
			}
			i++;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Firewall
//


COptionsWindow::ENetWfAction COptionsWindow::GetFwRuleAction(const QString& Value)
{
	if (Value.compare("Allow", Qt::CaseInsensitive) == 0)
		return eAllow;
	//if (Value.compare("Block", Qt::CaseInsensitive) == 0)
		return eBlock;
}

QString COptionsWindow::GetFwRuleActionStr(ENetWfAction Action)
{
	switch (Action)
	{
	case eAllow:		return "Allow";
	case eBlock:		return "Block";
	}
	return "";
}

COptionsWindow::ENetWfProt COptionsWindow::GetFwRuleProt(const QString& Value)
{
	if (Value.compare("TCP", Qt::CaseInsensitive) == 0)
		return eTcp;
	if (Value.compare("UDP", Qt::CaseInsensitive) == 0)
		return eUdp;
	if (Value.compare("ICMP", Qt::CaseInsensitive) == 0)
		return eIcmp;
	return eAny;
}

void COptionsWindow::ParseAndAddFwRule(const QString& Value, bool disabled, const QString& Template)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem();
	
	//NetworkAccess=explorer.exe,Allow;Port=137,138,139,445;Address=192.168.0.1-192.168.100.255;Protocol=TCP;

	QString FirstStr;
	TArguments Tags = GetArguments(Value, L';', L'=', &FirstStr, true);
	StrPair ProgAction = Split2(FirstStr, ",", true);
	QString Program = ProgAction.second.isEmpty() ? "" : ProgAction.first;
	QString Action = ProgAction.second.isEmpty() ? ProgAction.first : ProgAction.second;

	pItem->setData(0, Qt::UserRole, Program);
	bool bAll = Program.isEmpty() || Program == "*";
	if (bAll)
		Program = tr("All Programs");
	bool Not = Program.left(1) == "!";
	if (Not)
		Program.remove(0, 1);
	if (Program.left(1) == "<")
		Program = tr("Group: %1").arg(Program.mid(1, Program.length() - 2));
	else if(!bAll)
		m_Programs.insert(Program);
	pItem->setText(0, (Not ? "NOT " : "") + Program);
	
	pItem->setText(1, Action + (Template.isEmpty() ? "" : " (" + Template + ")"));
	pItem->setData(1, Qt::UserRole, Template.isEmpty() ? (int)GetFwRuleAction(Action) : -1);
	
	QString Port = Tags.value("port");
	pItem->setText(2, Port);
	pItem->setData(2, Qt::UserRole, Port);

	QString IP = Tags.value("address");
	pItem->setText(3, IP);
	pItem->setData(3, Qt::UserRole, IP);

	QString Prot = Tags.value("protocol");
	pItem->setText(4, Prot);
	pItem->setData(4, Qt::UserRole, (int)GetFwRuleProt(Prot));

	if(Template.isEmpty())
		pItem->setCheckState(0, disabled ? Qt::Unchecked : Qt::Checked);
	ui.treeNetFw->addTopLevelItem(pItem);
}

void COptionsWindow::SaveNetFwRules()
{
	QList<QString> Rules;
	QList<QString> RulesDisabled;
	for (int i = 0; i < ui.treeNetFw->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeNetFw->topLevelItem(i);
		int Type = pItem->data(1, Qt::UserRole).toInt();
		if (Type == -1)
			continue; // entry from template
		QString Program = pItem->data(0, Qt::UserRole).toString();
		ENetWfAction Action = (ENetWfAction)pItem->data(1, Qt::UserRole).toInt();
		QString Port = pItem->data(2, Qt::UserRole).toString();
		QString IP = pItem->data(3, Qt::UserRole).toString();
		QString Prot = pItem->text(4);

		QString Temp = GetFwRuleActionStr(Action);
		//if (Program.contains("=") || Program.contains(";") || Program.contains(",")) // todo: make SBIE parses this properly
		//	Program = "\'" + Program + "\'"; 
		if (Program.isEmpty())
			Program = "*";
		Temp.prepend(Program + ",");
		QStringList Tags = QStringList(Temp);
		if (!Port.isEmpty()) Tags.append("Port=" + Port);
		if (!IP.isEmpty()) Tags.append("Address=" + IP);
		if (!Prot.isEmpty()) Tags.append("Protocol=" + Prot);

		if(pItem->checkState(0) == Qt::Checked)
			Rules.append(Tags.join(";"));
		else
			RulesDisabled.append(Tags.join(";"));
	}
	WriteTextList("NetworkAccess", Rules);
	WriteTextList("NetworkAccessDisabled", RulesDisabled);

	m_NetFwRulesChanged = false;
}

void COptionsWindow::OnNetFwItemDoubleClicked(QTreeWidgetItem* pItem, int Column)
{
	int Action = pItem->data(1, Qt::UserRole).toInt();
	if (Action == -1) {
		QMessageBox::warning(this, "SandboxiePlus", tr("Template values can not be edited."));
		return;
	}

	QString Program = pItem->data(0, Qt::UserRole).toString();

	QWidget* pProgram = new QWidget();
	pProgram->setAutoFillBackground(true);
	QHBoxLayout* pLayout = new QHBoxLayout();
	pLayout->setContentsMargins(0,0,0,0);
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

	ui.treeNetFw->setItemWidget(pItem, 0, pProgram);

	QComboBox* pMode = new QComboBox();
	pMode->addItem(tr("Allow"), (int)eAllow);
	pMode->addItem(tr("Block"), (int)eBlock);
	pMode->setCurrentIndex(pMode->findData(pItem->data(1, Qt::UserRole)));
	ui.treeNetFw->setItemWidget(pItem, 1, pMode);

	QLineEdit* pPort = new QLineEdit();
	pPort->setText(pItem->data(2, Qt::UserRole).toString());
	ui.treeNetFw->setItemWidget(pItem, 2, pPort);

	QLineEdit* pIP = new QLineEdit();
	pIP->setText(pItem->data(3, Qt::UserRole).toString());
	ui.treeNetFw->setItemWidget(pItem, 3, pIP);

	QComboBox* pProt = new QComboBox();
	pProt->addItem(tr("Any"), (int)eAny);
	pProt->addItem(tr("TCP"), (int)eTcp);
	pProt->addItem(tr("UDP"), (int)eUdp);
	pProt->addItem(tr("ICMP"), (int)eIcmp);
	pProt->setCurrentIndex(pProt->findData(pItem->data(4, Qt::UserRole)));
	ui.treeNetFw->setItemWidget(pItem, 4, pProt);
}

void COptionsWindow::OnNetFwChanged(QTreeWidgetItem* pItem, int Column)
{
	if (Column != 0)
		return;

	m_NetFwRulesChanged = true;
	OnOptChanged();
}

void COptionsWindow::CloseNetFwEdit(bool bSave)
{
	for (int i = 0; i < ui.treeNetFw->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeNetFw->topLevelItem(i);
		CloseNetFwEdit(pItem, bSave);
	}
}

void COptionsWindow::CloseNetFwEdit(QTreeWidgetItem* pItem, bool bSave)
{
	QWidget* pProgram = ui.treeNetFw->itemWidget(pItem, 0);
	if (!pProgram)
		return;

	if (bSave)
	{
		QHBoxLayout* pLayout = (QHBoxLayout*)pProgram->layout();
		QToolButton* pNot = (QToolButton*)pLayout->itemAt(0)->widget();
		QComboBox* pCombo = (QComboBox*)pLayout->itemAt(1)->widget();

		QComboBox* pAction = (QComboBox*)ui.treeNetFw->itemWidget(pItem, 1);

		QLineEdit* pPort = (QLineEdit*)ui.treeNetFw->itemWidget(pItem, 2);

		QLineEdit* pIP = (QLineEdit*)ui.treeNetFw->itemWidget(pItem, 3);

		QComboBox* pProt = (QComboBox*)ui.treeNetFw->itemWidget(pItem, 4);

		QString Program = pCombo->currentText();
		int Index = pCombo->findText(Program);
		if (Index != -1)
			Program = pCombo->itemData(Index, Qt::UserRole).toString();

		pItem->setText(0, (pNot->isChecked() ? "NOT " : "") + pCombo->currentText());
		pItem->setData(0, Qt::UserRole, (pNot->isChecked() ? "!" : "") + Program);

		pItem->setText(1, pAction->currentText());
		pItem->setData(1, Qt::UserRole, pAction->currentData());
	
		pItem->setText(2, pPort->text());
		pItem->setData(2, Qt::UserRole, pPort->text());

		pItem->setText(3, pIP->text());
		pItem->setData(3, Qt::UserRole, pIP->text());

		pItem->setText(4, pProt->currentText());
		pItem->setData(4, Qt::UserRole, pProt->currentData());

		m_NetFwRulesChanged = true;
		OnOptChanged();
	}

	for (int i = 0; i < 5; i++)
		ui.treeNetFw->setItemWidget(pItem, i, NULL);
}

void COptionsWindow::OnAddNetFwRule()
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem();
			
	pItem->setData(0, Qt::UserRole, "");
	pItem->setText(0, tr("All Programs"));
	
	pItem->setText(1, GetFwRuleActionStr(eBlock));
	pItem->setData(1, Qt::UserRole, (int)eBlock);

	pItem->setCheckState(0, Qt::Checked);
	ui.treeNetFw->addTopLevelItem(pItem);

	m_NetFwRulesChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnDelNetFwRule()
{
	QTreeWidgetItem* pItem = ui.treeNetFw->currentItem();
	if (!pItem)
		return;

	delete pItem;

	m_NetFwRulesChanged = true;
	OnOptChanged();
}

void COptionsWindow__SetRowColor(QTreeWidgetItem* pItem, bool bMatch, bool bConflict = false, bool bBlock = false, bool bActive = false)
{
	for (int i = 0; i < pItem->columnCount(); i++)
	{
		if (!bMatch)
		{
			pItem->setBackground(i, Qt::white); // todo dark mode
		}
		else if(bConflict)
			pItem->setBackground(i, QColor(255, 255, 0)); // yellow
		else if (!bBlock)
		{
			if (bActive)
				pItem->setBackground(i, QColor(128, 255, 128)); // dark green
			else
				pItem->setBackground(i, QColor(224, 240, 224)); // light green
		}
		else
		{
			if (bActive)
				pItem->setBackground(i, QColor(255, 128, 128)); // dark red
			else
				pItem->setBackground(i, QColor(240, 224, 224)); // light red
		}
	}
}

struct SFirewallRule
{
	enum EMatch
	{
		eNone = 0,
		eGlobal,
		eByNot,
		eByRange,
		eExact
	};

	EMatch MatchProg(const QString& TestProg)
	{
		if (Program.isEmpty())
			return eGlobal;

		QString Test = Program;
		bool bNot = false;
		if (Test.left(1) == "!") {
			Test.remove(0, 1);
			bNot = true;
		}
	
		// ToDo: match groups and wildcards

		if (TestProg.isEmpty() || ((Test.compare(TestProg, Qt::CaseInsensitive) == 0) != bNot))
			return bNot ? eByNot : eExact;
		return eNone;
	}

	void SetPorts(const QString& Ports)
	{
		PortMap.clear();
		QStringList PortList = SplitStr(Ports, ",");
		foreach(const QString & Port, PortList)
		{
			StrPair BeginEnd = Split2(Port, "-");
			quint16 Begin = BeginEnd.first.toInt();
			quint16 End = BeginEnd.second.isEmpty() ? Begin : BeginEnd.second.toInt();
			PortMap.insert(End, Begin);
		}
	}

	EMatch MatchPort(quint16 TestPort)
	{
		if (PortMap.isEmpty())
			return eGlobal;
		if (TestPort == 0)
			return eExact;

		auto I = PortMap.lowerBound(TestPort);
		if (I == PortMap.end())
			return eNone;
		if (I.key() < TestPort)
			return eNone;
		if (TestPort < I.value())
			return eNone;
		return I.key() == I.value() ? eExact : eByRange;
	}

	void SetAddresses(const QString& Addresses)
	{
		AddressMap.clear();
		QStringList AddressList = SplitStr(Addresses, ",");
		foreach(const QString & Address, AddressList)
		{
			StrPair BeginEnd = Split2(Address, "-");
			QHostAddress Begin = QHostAddress(BeginEnd.first);
			QHostAddress End = BeginEnd.second.isEmpty() ? Begin : QHostAddress(BeginEnd.second);
			AddressMap.insert(End, Begin);
		}
	}

	EMatch MatchAddress(const QHostAddress& TestAddress)
	{
		if (AddressMap.isEmpty())
			return eGlobal;
		if (TestAddress.isNull())
			return eExact;

		auto I = AddressMap.lowerBound(TestAddress);
		if (I == AddressMap.end())
			return eNone;
		if (I.key() < TestAddress)
			return eNone;
		if (TestAddress < I.value())
			return eNone;
		return I.key() == I.value() ? eExact : eByRange;
	}

	EMatch MatchProtocol(COptionsWindow::ENetWfProt TestProt)
	{
		if (Protocol == COptionsWindow::eAny)
			return eGlobal;
		if (TestProt == COptionsWindow::eAny)
			return eExact;

		if (Protocol == TestProt)
			return eExact;
		return eNone;
	}

	struct SMatch
	{
		SMatch() { memset(this, 0, sizeof(SMatch)); }
		EMatch ByProg;
		EMatch ByPort;
		EMatch ByAddress;
		EMatch ByEndPoint;
		EMatch ByProtocol;
		COptionsWindow::ENetWfAction Action;
	};

	bool MatchRule(const QString& TestProg, quint16 TestPort, const QHostAddress& TestAddress, COptionsWindow::ENetWfProt TestProt, SMatch* Match) 
	{
		//SMatch Dummy;
		//if (!Match) Match = &Dummy;

		if (!(Match->ByProg = MatchProg(TestProg)))
			return false;
		if (!(Match->ByPort = MatchPort(TestPort)))
			return false;
		if (!(Match->ByAddress = MatchAddress(TestAddress)))
			return false;
		if (!(Match->ByProtocol = MatchProtocol(TestProt)))
			return false;

		if (Match->ByAddress > eGlobal && Match->ByPort > eGlobal)
			Match->ByEndPoint = Max(Match->ByAddress, Match->ByPort);

		Match->Action = Action;

		return true;
	}

#define COMPARE_AND_RETURN(x, y) if(x != y) return x > y

	static bool IsBetterMatch(SMatch MyMatch, SMatch OtherMatch)
	{
		// 1. A rule for a specified program trumps a rule for all programs except a given one, trumps a rule for all programs
		COMPARE_AND_RETURN(MyMatch.ByProg, OtherMatch.ByProg);
		
		// 2. a rule with a Port or IP trumps a rule without
		// 2a. a rule with ip and port trums a rule with ip or port only
		// 2b. a rule with one ip trumps a rule with an ip range that is besides that on the same level
		COMPARE_AND_RETURN(MyMatch.ByEndPoint, OtherMatch.ByEndPoint);
		COMPARE_AND_RETURN(MyMatch.ByPort, OtherMatch.ByPort);
		COMPARE_AND_RETURN(MyMatch.ByAddress, OtherMatch.ByAddress);

		// 3. block rules trump allow rules
		if(MyMatch.Action == COptionsWindow::eBlock && OtherMatch.Action != COptionsWindow::eBlock)
			return true;
		
		// 4. a rule without a protocol means all protocols, a rule with a protocol trumps a rule without if its the only difference
		COMPARE_AND_RETURN(MyMatch.ByProtocol, OtherMatch.ByProtocol);

		return false;
	}

	QString Program;
	COptionsWindow::ENetWfAction Action;
	QMap<quint16, quint16> PortMap;
	QMap<QHostAddress, QHostAddress> AddressMap;
	COptionsWindow::ENetWfProt Protocol;
};

void COptionsWindow::OnTestNetFwRule()
{
	QString TestProg = ui.txtProgFwTest->text();
	quint16 TestPort = ui.txtPortFwTest->text().toInt();
	QHostAddress TestAddress = QHostAddress(ui.txtIPFwTest->text());
	ENetWfProt TestProt = (ENetWfProt)ui.cmbProtFwTest->currentData().toInt();

	QTreeWidgetItem* pBestItem = NULL;
	SFirewallRule BestRule;
	SFirewallRule::SMatch BestMatch;

	/*BestRule.SetPorts("100-200,400-500,700-800");
	bool test1 = BestRule.MatchPort(500);
	bool test2 = BestRule.MatchPort(450);
	bool test3 = BestRule.MatchPort(350);*/

	/*BestRule.SetAddresses("10.0.0.0-10.255.255.255,172.16.0.0-172.31.255.255,192.168.0.0-192.168.255.255");
	bool test1 = BestRule.MatchAddress(QHostAddress("10.70.0.1"));
	bool test2 = BestRule.MatchAddress(QHostAddress("8.8.8.8"));
	bool test3 = BestRule.MatchAddress(QHostAddress("244.123.0.0"));
	bool test4 = BestRule.MatchAddress(QHostAddress("192.168.100.1"));*/

	for (int i = 0; i < ui.treeNetFw->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeNetFw->topLevelItem(i);
		int Type = pItem->data(1, Qt::UserRole).toInt();
		if (Type == -1)
			continue; // entry from template

		SFirewallRule Rule;
		Rule.Program = pItem->data(0, Qt::UserRole).toString();
		Rule.Action = (ENetWfAction)pItem->data(1, Qt::UserRole).toInt();
		Rule.SetPorts(pItem->data(2, Qt::UserRole).toString());
		Rule.SetAddresses(pItem->data(3, Qt::UserRole).toString());
		Rule.Protocol = (ENetWfProt)pItem->data(4, Qt::UserRole).toInt();


		SFirewallRule::SMatch Match;
		if (Rule.MatchRule(TestProg, TestPort, TestAddress, TestProt, &Match))
		{
			if (!pBestItem || SFirewallRule::IsBetterMatch(Match, BestMatch)) {
				BestRule = Rule;
				BestMatch = Match;
				pBestItem = pItem;
			}
			else if(!SFirewallRule::IsBetterMatch(BestMatch, Match)) // if this is not true we have a conflict
			{
				COptionsWindow__SetRowColor(pItem, true, true);
				COptionsWindow__SetRowColor(pBestItem, true, true);
				pBestItem = NULL;
				continue;
			}
			COptionsWindow__SetRowColor(pItem, true, false, Rule.Action == eBlock, false);
		}
		else
			COptionsWindow__SetRowColor(pItem, false);
	}

	if(pBestItem)
		COptionsWindow__SetRowColor(pBestItem, true, false, BestRule.Action == eBlock, true);
	
	//
	// rule merging
	// 	   if the rule is for the same prog and has the same action
	// 	   merge all rules with ip only together
	// 	   merge all rules with ports only together
	// 
	// 

}

void COptionsWindow::OnClearNetFwTest()
{
	ui.txtProgFwTest->setText("");
	ui.txtPortFwTest->setText("");
	ui.txtIPFwTest->setText("");
	ui.cmbProtFwTest->setCurrentIndex(0);
	
	for (int i = 0; i < ui.treeNetFw->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeNetFw->topLevelItem(i);
		COptionsWindow__SetRowColor(pItem, false);
	}
}
