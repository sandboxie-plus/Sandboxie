#include "stdafx.h"
#include "OptionsWindow.h"
#include "SandMan.h"
#include "SettingsWindow.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/Common.h"
#include "../MiscHelpers/Common/ComboInputDialog.h"
#include "../MiscHelpers/Common/SettingsWidgets.h"
#include "Helpers/WinAdmin.h"

void COptionsWindow::CreateGeneral()
{
	ui.cmbBoxIndicator->addItem(tr("Don't alter the window title"), "-");
	ui.cmbBoxIndicator->addItem(tr("Display [#] indicator only"), "n");
	ui.cmbBoxIndicator->addItem(tr("Display box name in title"), "y");

	ui.cmbBoxBorder->addItem(tr("Border disabled"), "off");
	ui.cmbBoxBorder->addItem(tr("Show only when title is in focus"), "ttl");
	ui.cmbBoxBorder->addItem(tr("Always show"), "on");

	ui.cmbBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eHardenedPlus), tr("Hardened Sandbox with Data Protection"), (int)CSandBoxPlus::eHardenedPlus);
	ui.cmbBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eHardened), tr("Security Hardened Sandbox"), (int)CSandBoxPlus::eHardened);
	ui.cmbBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eDefaultPlus), tr("Sandbox with Data Protection"), (int)CSandBoxPlus::eDefaultPlus);
	ui.cmbBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eDefault), tr("Standard Isolation Sandbox (Default)"), (int)CSandBoxPlus::eDefault);
	//ui.cmbBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eInsecure), tr("UNSECURE Configuration (please change)"), (int)CSandBoxPlus::eInsecure);
	ui.cmbBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eAppBoxPlus), tr("Application Compartment with Data Protection"), (int)CSandBoxPlus::eAppBoxPlus);
	ui.cmbBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eAppBox), tr("Application Compartment (NO Isolation)"), (int)CSandBoxPlus::eAppBox);

	ui.lblSupportCert->setVisible(false);
	if ((g_FeatureFlags & CSbieAPI::eSbieFeatureCert) == 0)
	{
		ui.lblSupportCert->setVisible(true);
		connect(ui.lblSupportCert, SIGNAL(linkActivated(const QString&)), theGUI, SLOT(OpenUrl(const QString&)));

		for (int i = 0; i < ui.cmbBoxType->count(); i++)
		{
			int BoxType = ui.cmbBoxType->itemData(i, Qt::UserRole).toInt();
			bool disabled = BoxType != CSandBoxPlus::eDefault && BoxType != CSandBoxPlus::eHardened;

			QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui.cmbBoxType->model());
			QStandardItem* item = model->item(i);
			item->setFlags(disabled ? item->flags() & ~Qt::ItemIsEnabled : item->flags() | Qt::ItemIsEnabled);
		}
	}

	m_HoldBoxType = false;

	connect(ui.cmbBoxType, SIGNAL(currentIndexChanged(int)), this, SLOT(OnBoxTypChanged()));
	connect(ui.chkDropRights, SIGNAL(clicked(bool)), this, SLOT(UpdateBoxType()));
	connect(ui.chkPrivacy, SIGNAL(clicked(bool)), this, SLOT(UpdateBoxType()));
	connect(ui.chkNoSecurityIsolation, SIGNAL(clicked(bool)), this, SLOT(UpdateBoxType()));
	connect(ui.chkNoSecurityFiltering, SIGNAL(clicked(bool)), this, SLOT(UpdateBoxType()));

	connect(ui.cmbBoxIndicator, SIGNAL(currentIndexChanged(int)), this, SLOT(OnGeneralChanged()));
	connect(ui.cmbBoxBorder, SIGNAL(currentIndexChanged(int)), this, SLOT(OnGeneralChanged()));
	connect(ui.btnBorderColor, SIGNAL(clicked(bool)), this, SLOT(OnPickColor()));
	connect(ui.spinBorderWidth, SIGNAL(valueChanged(int)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkShowForRun, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkPinToTray, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));

	connect(ui.chkBlockNetShare, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkBlockNetParam, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkDropRights, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkFakeElevation, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkMsiExemptions, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	
	connect(ui.chkBlockSpooler, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkOpenSpooler, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkPrintToFile, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));

	connect(ui.chkOpenCredentials, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkOpenProtectedStorage, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkCloseClipBoard, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	//connect(ui.chkOpenSmartCard, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	//connect(ui.chkOpenBluetooth, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));

	connect(ui.txtCopyLimit, SIGNAL(textChanged(const QString&)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkCopyLimit, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkCopyPrompt, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkNoCopyWarn, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));

	connect(ui.chkProtectBox, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkAutoEmpty, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));

	connect(ui.chkRawDiskRead, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkRawDiskNotify, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));

	connect(ui.btnAddCmd, SIGNAL(clicked(bool)), this, SLOT(OnAddCommand()));
	QMenu* pRunBtnMenu = new QMenu(ui.btnAddFile);
	pRunBtnMenu->addAction(tr("Browse for Program"), this, SLOT(OnBrowsePath()));
	ui.btnAddCmd->setPopupMode(QToolButton::MenuButtonPopup);
	ui.btnAddCmd->setMenu(pRunBtnMenu);
	connect(ui.btnDelCmd, SIGNAL(clicked(bool)), this, SLOT(OnDelCommand()));
	connect(ui.treeRun, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(OnRunChanged()));
}

void COptionsWindow::LoadGeneral()
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

	ui.chkShowForRun->setChecked(m_pBox->GetBool("ShowForRunIn", true));
	ui.chkPinToTray->setChecked(m_pBox->GetBool("PinToTray", false));

	ui.chkBlockNetShare->setChecked(m_pBox->GetBool("BlockNetworkFiles", false));
	ui.chkBlockNetParam->setChecked(m_pBox->GetBool("BlockNetParam", true));
	ui.chkDropRights->setChecked(m_pBox->GetBool("DropAdminRights", false));
	ui.chkFakeElevation->setChecked(m_pBox->GetBool("FakeAdminRights", false));
	ui.chkMsiExemptions->setChecked(m_pBox->GetBool("MsiInstallerExemptions", false));
		
	ui.chkBlockSpooler->setChecked(m_pBox->GetBool("ClosePrintSpooler", false));
	ui.chkOpenSpooler->setChecked(m_pBox->GetBool("OpenPrintSpooler", false));
	ui.chkPrintToFile->setChecked(m_pBox->GetBool("AllowSpoolerPrintToFile", false));

	ui.chkOpenProtectedStorage->setChecked(m_pBox->GetBool("OpenProtectedStorage", false));
	ui.chkOpenCredentials->setChecked(!ui.chkOpenCredentials->isEnabled() || m_pBox->GetBool("OpenCredentials", false));
	ui.chkCloseClipBoard->setChecked(!m_pBox->GetBool("OpenClipboard", true));
	//ui.chkOpenSmartCard->setChecked(m_pBox->GetBool("OpenSmartCard", true));
	//ui.chkOpenBluetooth->setChecked(m_pBox->GetBool("OpenBluetooth", false));

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
	ui.chkCopyPrompt->setChecked(m_pBox->GetBool("PromptForFileMigration", true));
	ui.chkNoCopyWarn->setChecked(!m_pBox->GetBool("CopyLimitSilent", false));
	
	ui.chkProtectBox->setChecked(m_pBox->GetBool("NeverDelete", false));
	ui.chkAutoEmpty->setChecked(m_pBox->GetBool("AutoDelete", false));

	ui.chkRawDiskRead->setChecked(m_pBox->GetBool("AllowRawDiskRead", false));
	ui.chkRawDiskNotify->setChecked(m_pBox->GetBool("NotifyDirectDiskAccess", false));

	OnGeneralChanged();

	m_GeneralChanged = false;
}

void COptionsWindow::SaveGeneral()
{
	WriteText("BoxNameTitle", ui.cmbBoxIndicator->currentData().toString());

	QStringList BorderCfg;
	BorderCfg.append(QString("#%1%2%3").arg(m_BorderColor.blue(), 2, 16, QChar('0')).arg(m_BorderColor.green(), 2, 16, QChar('0')).arg(m_BorderColor.red(), 2, 16, QChar('0')));
	BorderCfg.append(ui.cmbBoxBorder->currentData().toString());
	BorderCfg.append(QString::number(ui.spinBorderWidth->value()));
	WriteText("BorderColor", BorderCfg.join(","));

	WriteAdvancedCheck(ui.chkShowForRun, "ShowForRunIn", "", "n");
	WriteAdvancedCheck(ui.chkPinToTray, "PinToTray", "y", "");

	WriteAdvancedCheck(ui.chkBlockNetShare, "BlockNetworkFiles", "y", "");
	WriteAdvancedCheck(ui.chkBlockNetParam, "BlockNetParam", "", "n");
	WriteAdvancedCheck(ui.chkDropRights, "DropAdminRights", "y", "");
	WriteAdvancedCheck(ui.chkFakeElevation, "FakeAdminRights", "y", "");
	WriteAdvancedCheck(ui.chkMsiExemptions, "MsiInstallerExemptions", "y", "");

	WriteAdvancedCheck(ui.chkBlockSpooler, "ClosePrintSpooler", "y", "");
	WriteAdvancedCheck(ui.chkOpenSpooler, "OpenPrintSpooler", "y", "");
	WriteAdvancedCheck(ui.chkPrintToFile, "AllowSpoolerPrintToFile", "y", "");

	WriteAdvancedCheck(ui.chkOpenProtectedStorage, "OpenProtectedStorage", "y", "");
	if (ui.chkOpenCredentials->isEnabled())
		WriteAdvancedCheck(ui.chkOpenCredentials, "OpenCredentials", "y", "");
	WriteAdvancedCheck(ui.chkCloseClipBoard, "OpenClipboard", "n", "");
	//WriteAdvancedCheck(ui.chkOpenSmartCard, "OpenSmartCard", "", "n");
	//WriteAdvancedCheck(ui.chkOpenBluetooth, "OpenBluetooth", "y", "");


	QStringList RunCommands;
	for (int i = 0; i < ui.treeRun->topLevelItemCount(); i++) {
		QTreeWidgetItem* pItem = ui.treeRun->topLevelItem(i);
		RunCommands.append(pItem->text(0) + "|" + pItem->text(1));
	}
	WriteTextList("RunCommand", RunCommands);


	WriteText("CopyLimitKb", ui.chkCopyLimit->isChecked() ? ui.txtCopyLimit->text() : "-1");
	WriteAdvancedCheck(ui.chkCopyPrompt, "PromptForFileMigration", "", "n");
	WriteAdvancedCheck(ui.chkNoCopyWarn, "CopyLimitSilent", "", "y");

	WriteAdvancedCheck(ui.chkProtectBox, "NeverDelete", "y", "");
	WriteAdvancedCheck(ui.chkAutoEmpty, "AutoDelete", "y", "");

	WriteAdvancedCheck(ui.chkRawDiskRead, "AllowRawDiskRead", "y", "");
	WriteAdvancedCheck(ui.chkRawDiskNotify, "NotifyDirectDiskAccess", "y", "");

	m_GeneralChanged = false;
}

void COptionsWindow::OnGeneralChanged()
{
	ui.lblCopyLimit->setEnabled(ui.chkCopyLimit->isChecked());
	ui.txtCopyLimit->setEnabled(ui.chkCopyLimit->isChecked());
	ui.lblCopyLimit->setText(tr("kilobytes (%1)").arg(FormatSize(ui.txtCopyLimit->text().toULongLong() * 1024)));
	ui.chkCopyPrompt->setEnabled(ui.chkCopyLimit->isChecked());
	ui.chkNoCopyWarn->setEnabled(ui.chkCopyLimit->isChecked() && !ui.chkCopyPrompt->isChecked());

	ui.chkAutoEmpty->setEnabled(!ui.chkProtectBox->isChecked());

	ui.chkOpenSpooler->setEnabled(!ui.chkBlockSpooler->isChecked() && !ui.chkNoSecurityIsolation->isChecked());
	ui.chkPrintToFile->setEnabled(!ui.chkBlockSpooler->isChecked() && !ui.chkNoSecurityFiltering->isChecked());
	
	ui.chkOpenCredentials->setEnabled(!ui.chkOpenProtectedStorage->isChecked());
	if (!ui.chkOpenCredentials->isEnabled()) ui.chkOpenCredentials->setChecked(true);

	m_GeneralChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnPickColor()
{
	QColor color = QColorDialog::getColor(m_BorderColor, this, tr("Select color"));
	if (!color.isValid())
		return;
	m_GeneralChanged = true;
	OnOptChanged();
	m_BorderColor = color;
	ui.btnBorderColor->setStyleSheet("background-color: " + m_BorderColor.name());
}

void COptionsWindow::OnBrowsePath()
{
	QString Value = QFileDialog::getOpenFileName(this, tr("Select Program"), "", tr("Executables (*.exe *.cmd)")).replace("/", "\\");
	if (Value.isEmpty())
		return;

	QString Name = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter a menu title"), QLineEdit::Normal);
	if (Name.isEmpty())
		return;

	AddRunItem(Name, Value);
	m_GeneralChanged = true;
	OnOptChanged();
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
	OnOptChanged();
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
	OnOptChanged();
}

void COptionsWindow::UpdateBoxType()
{
	bool bPrivacyMode = ui.chkPrivacy->isChecked();
	bool bNoAdmin = ui.chkDropRights->isChecked();
	bool bAppBox = ui.chkNoSecurityIsolation->isChecked();

	int BoxType;
	if (bAppBox) 
		BoxType = bPrivacyMode ? (int)CSandBoxPlus::eAppBoxPlus : (int)CSandBoxPlus::eAppBox;
	else if (bNoAdmin) 
		BoxType = bPrivacyMode ? (int)CSandBoxPlus::eHardenedPlus : (int)CSandBoxPlus::eHardened;
	else 
		BoxType = bPrivacyMode ? (int)CSandBoxPlus::eDefaultPlus : (int)CSandBoxPlus::eDefault;

	ui.lblBoxInfo->setText(theGUI->GetBoxDescription(BoxType));

	if (m_HoldBoxType)
		return;

	m_HoldBoxType = true;
	ui.cmbBoxType->setCurrentIndex(ui.cmbBoxType->findData(BoxType));
	m_HoldBoxType = false;
}

void COptionsWindow::OnBoxTypChanged()
{
	if (m_HoldBoxType)
		return;

	int BoxType = ui.cmbBoxType->currentData().toInt();

	switch (BoxType) {
	case CSandBoxPlus::eHardenedPlus:
	case CSandBoxPlus::eHardened:
		ui.chkNoSecurityIsolation->setChecked(false);
		ui.chkNoSecurityFiltering->setChecked(false);
		ui.chkDropRights->setChecked(true);
		ui.chkMsiExemptions->setChecked(false);
		//ui.chkRestrictServices->setChecked(true);
		ui.chkPrivacy->setChecked(BoxType == CSandBoxPlus::eHardenedPlus);
		//SetTemplate("NoUACProxy", false);
		//if ((g_FeatureFlags & CSbieAPI::eSbieFeatureCert) == 0)
		//	SetTemplate("DeviceSecurity", true); // requirers rule specificity
		SetTemplate("RpcPortBindingsExt", false);
		break;
	case CSandBoxPlus::eDefaultPlus:
	case CSandBoxPlus::eDefault:
		ui.chkNoSecurityIsolation->setChecked(false);
		ui.chkNoSecurityFiltering->setChecked(false);
		ui.chkDropRights->setChecked(false);
		ui.chkMsiExemptions->setChecked(false);
		//ui.chkRestrictServices->setChecked(true);
		ui.chkPrivacy->setChecked(BoxType == CSandBoxPlus::eDefaultPlus);
		//SetTemplate("NoUACProxy", false);
		//SetTemplate("DeviceSecurity", false);
		break;
	case CSandBoxPlus::eAppBoxPlus:
	case CSandBoxPlus::eAppBox:
		ui.chkNoSecurityIsolation->setChecked(true);
		//ui.chkRestrictServices->setChecked(false);
		ui.chkPrivacy->setChecked(BoxType == CSandBoxPlus::eAppBoxPlus);
		//SetTemplate("NoUACProxy", true);
		//SetTemplate("DeviceSecurity", false);
		SetTemplate("RpcPortBindingsExt", true);
		break;
	}

	m_GeneralChanged = true;
	m_AccessChanged = true;
	m_AdvancedChanged = true;

	m_HoldBoxType = true;
	UpdateBoxType();
	m_HoldBoxType = false;
	OnOptChanged();
}
