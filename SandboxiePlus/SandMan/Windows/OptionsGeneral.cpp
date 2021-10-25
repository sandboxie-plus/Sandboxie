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

	connect(ui.cmbBoxIndicator, SIGNAL(currentIndexChanged(int)), this, SLOT(OnGeneralChanged()));
	connect(ui.cmbBoxBorder, SIGNAL(currentIndexChanged(int)), this, SLOT(OnGeneralChanged()));
	connect(ui.btnBorderColor, SIGNAL(clicked(bool)), this, SLOT(OnPickColor()));
	connect(ui.spinBorderWidth, SIGNAL(valueChanged(int)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkShowForRun, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));

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

	connect(ui.btnAddAutoExe, SIGNAL(clicked(bool)), this, SLOT(OnAddAutoCmd()));
	QMenu* pAutoBtnMenu = new QMenu(ui.btnAddFile);
	pAutoBtnMenu->addAction(tr("Browse for Program"), this, SLOT(OnAddAutoExe()));
	ui.btnAddAutoExe->setPopupMode(QToolButton::MenuButtonPopup);
	ui.btnAddAutoExe->setMenu(pAutoBtnMenu);
	connect(ui.btnAddAutoExe, SIGNAL(clicked(bool)), this, SLOT(OnAddAutoExe()));
	connect(ui.btnAddAutoSvc, SIGNAL(clicked(bool)), this, SLOT(OnDelAutoSvc()));
	connect(ui.btnDelAuto, SIGNAL(clicked(bool)), this, SLOT(OnDelAuto()));
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

	ui.chkBlockNetShare->setChecked(m_pBox->GetBool("BlockNetworkFiles", true));
	ui.chkBlockNetParam->setChecked(m_pBox->GetBool("BlockNetParam", true));
	ui.chkDropRights->setChecked(m_pBox->GetBool("DropAdminRights", false));
	ui.chkFakeElevation->setChecked(m_pBox->GetBool("FakeAdminRights", false));
	ui.chkMsiExemptions->setChecked(m_pBox->GetBool("MsiInstallerExemptions", false));
		
	ui.chkBlockSpooler->setChecked(m_pBox->GetBool("ClosePrintSpooler", false));
	ui.chkOpenSpooler->setChecked(m_pBox->GetBool("OpenPrintSpooler", false));
	ui.chkOpenSpooler->setEnabled(!ui.chkBlockSpooler->isChecked());
	ui.chkPrintToFile->setChecked(m_pBox->GetBool("AllowSpoolerPrintToFile", false));
	ui.chkPrintToFile->setEnabled(!ui.chkBlockSpooler->isChecked());

	ui.chkOpenProtectedStorage->setChecked(m_pBox->GetBool("OpenProtectedStorage", false));
	ui.chkOpenCredentials->setEnabled(!ui.chkOpenProtectedStorage->isChecked());
	ui.chkOpenCredentials->setChecked(!ui.chkOpenCredentials->isEnabled() || m_pBox->GetBool("OpenCredentials", false));
	ui.chkCloseClipBoard->setChecked(!m_pBox->GetBool("OpenClipboard", true));
	//ui.chkOpenSmartCard->setChecked(m_pBox->GetBool("OpenSmartCard", true));
	//ui.chkOpenBluetooth->setChecked(m_pBox->GetBool("OpenBluetooth", false));

	ui.treeAutoStart->clear();
	foreach(const QString & Value, m_pBox->GetTextList("StartProgram", m_Template))
		AddAutoRunItem(Value, 0);
	foreach(const QString & Value, m_pBox->GetTextList("StartService", m_Template))
		AddAutoRunItem(Value, 1);

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

	WriteAdvancedCheck(ui.chkBlockNetShare, "BlockNetworkFiles", "", "n");
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


	QStringList StartProgram;
	QStringList StartService;
	for (int i = 0; i < ui.treeAutoStart->topLevelItemCount(); i++) {
		QTreeWidgetItem* pItem = ui.treeAutoStart->topLevelItem(i);
		if (pItem->data(0, Qt::UserRole).toInt())
			StartService.append(pItem->text(1));
		else
			StartProgram.append(pItem->text(1));
	}
	WriteTextList("StartProgram", StartProgram);
	WriteTextList("StartService", StartService);

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
	m_GeneralChanged = true;

	ui.lblCopyLimit->setEnabled(ui.chkCopyLimit->isChecked());
	ui.txtCopyLimit->setEnabled(ui.chkCopyLimit->isChecked());
	ui.lblCopyLimit->setText(tr("kilobytes (%1)").arg(FormatSize(ui.txtCopyLimit->text().toULongLong() * 1024)));
	ui.chkCopyPrompt->setEnabled(ui.chkCopyLimit->isChecked());
	ui.chkNoCopyWarn->setEnabled(ui.chkCopyLimit->isChecked() && !ui.chkCopyPrompt->isChecked());

	ui.chkAutoEmpty->setEnabled(!ui.chkProtectBox->isChecked());

	ui.chkOpenSpooler->setEnabled(!ui.chkBlockSpooler->isChecked());
	ui.chkPrintToFile->setEnabled(!ui.chkBlockSpooler->isChecked());
}

void COptionsWindow::OnPickColor()
{
	QColor color = QColorDialog::getColor(m_BorderColor, this, tr("Select color"));
	if (!color.isValid())
		return;
	m_GeneralChanged = true;
	m_BorderColor = color;
	ui.btnBorderColor->setStyleSheet("background-color: " + m_BorderColor.name());
}

void COptionsWindow::OnAddAutoCmd()
{
	QString Value = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter a program path"), QLineEdit::Normal);
	if (Value.isEmpty())
		return;

	AddAutoRunItem(Value, 0);
	m_GeneralChanged = true;
}

void COptionsWindow::OnAddAutoExe()
{
	QString Value = QFileDialog::getOpenFileName(this, tr("Select Program"), "", tr("Executables (*.exe *.cmd);;All files (*.*)")).replace("/", "\\");;
	if (Value.isEmpty())
		return;

	AddAutoRunItem(Value, 0);
	m_GeneralChanged = true;
}

void COptionsWindow::OnDelAutoSvc()
{
	QString Value = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter a service identifier"), QLineEdit::Normal);
	if (Value.isEmpty())
		return;

	AddAutoRunItem(Value, 1);
	m_GeneralChanged = true;
}

void COptionsWindow::AddAutoRunItem(const QString& Value, int Type)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem();
	pItem->setText(0, Type ? tr("Service") : tr("Program"));
	pItem->setData(0, Qt::UserRole, Type);
	pItem->setText(1, Value);
	pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
	ui.treeAutoStart->addTopLevelItem(pItem);
}

void COptionsWindow::OnDelAuto()
{
	QTreeWidgetItem* pItem = ui.treeAutoStart->currentItem();
	if (!pItem)
		return;

	delete pItem;
	m_GeneralChanged = true;
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
