#include "stdafx.h"
#include "OptionsWindow.h"
#include "SandMan.h"
#include "SettingsWindow.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/Common.h"
#include "../MiscHelpers/Common/ComboInputDialog.h"
#include "../MiscHelpers/Common/SettingsWidgets.h"
#include "Helpers/WinAdmin.h"
#include "Helpers/WinHelper.h"

class CCertBadge: public QLabel
{
public:
	CCertBadge(QWidget* parent = NULL): QLabel(parent) 
	{
		setPixmap(QPixmap(":/Actions/Cert.png").scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
		if (!g_CertInfo.valid) {
			setToolTip(COptionsWindow::tr("This option requires a valid supporter certificate"));
			setCursor(Qt::PointingHandCursor);
		} else {
			setToolTip(COptionsWindow::tr("Supporter exclusive option"));
		}
	}

protected:
	void mousePressEvent(QMouseEvent* event)
	{
		if(!g_CertInfo.valid)
			theGUI->OpenUrl(QUrl("https://sandboxie-plus.com/go.php?to=sbie-get-cert"));
	}
};

void COptionsWindow__AddCertIcon(QWidget* pOriginalWidget)
{
	QWidget* pWidget = new QWidget();
	QHBoxLayout* pLayout = new QHBoxLayout(pWidget);
	pLayout->setContentsMargins(0, 0, 0, 0);
	pLayout->setSpacing(0);
	pLayout->addWidget(new CCertBadge());
	pOriginalWidget->parentWidget()->layout()->replaceWidget(pOriginalWidget, pWidget);
	pLayout->insertWidget(0, pOriginalWidget);
}

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
	//ui.cmbBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eInsecure), tr("INSECURE Configuration (please change)"), (int)CSandBoxPlus::eInsecure);
	ui.cmbBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eAppBoxPlus), tr("Application Compartment with Data Protection"), (int)CSandBoxPlus::eAppBoxPlus);
	ui.cmbBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eAppBox), tr("Application Compartment (NO Isolation)"), (int)CSandBoxPlus::eAppBox);

	ui.lblSupportCert->setVisible(false);
	if (!g_CertInfo.valid)
	{
		ui.lblSupportCert->setVisible(true);
		connect(ui.lblSupportCert, SIGNAL(linkActivated(const QString&)), theGUI, SLOT(OpenUrl(const QString&)));

		for (int i = 0; i < ui.cmbBoxType->count(); i++)
		{
			int BoxType = ui.cmbBoxType->itemData(i, Qt::UserRole).toInt();
			bool disabled = BoxType != CSandBoxPlus::eDefault;

			QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui.cmbBoxType->model());
			QStandardItem* item = model->item(i);
			item->setFlags(disabled ? item->flags() & ~Qt::ItemIsEnabled : item->flags() | Qt::ItemIsEnabled);
		}
	}

	if (g_Certificate.isEmpty()) {
		QWidget* ExWidgets[] = { ui.chkSecurityMode, ui.chkLockDown, ui.chkRestrictDevices,
			ui.chkPrivacy, ui.chkUseSpecificity,
			ui.chkNoSecurityIsolation, ui.chkNoSecurityFiltering, ui.chkConfidential, NULL };
		for (QWidget** ExWidget = ExWidgets; *ExWidget != NULL; ExWidget++)
			COptionsWindow__AddCertIcon(*ExWidget);
	}


	m_HoldBoxType = false;

	connect(ui.cmbBoxType, SIGNAL(currentIndexChanged(int)), this, SLOT(OnBoxTypChanged()));

	connect(ui.chkSecurityMode, SIGNAL(clicked(bool)), this, SLOT(UpdateBoxType()));
	connect(ui.chkPrivacy, SIGNAL(clicked(bool)), this, SLOT(UpdateBoxType()));
	connect(ui.chkNoSecurityIsolation, SIGNAL(clicked(bool)), this, SLOT(UpdateBoxType()));
	//connect(ui.chkNoSecurityFiltering, SIGNAL(clicked(bool)), this, SLOT(UpdateBoxType()));

	
	ui.btnBorderColor->setPopupMode(QToolButton::MenuButtonPopup);
	QMenu* pColorMenu = new QMenu(this);

	QWidgetAction* pActionWidget = new QWidgetAction(this);
	QWidget* pIconWidget = new QWidget(this);
	QHBoxLayout* pIconLayout = new QHBoxLayout(pIconWidget);
	pIconLayout->setContentsMargins(0, 0, 0, 0);
	m_pUseIcon = new QCheckBox(tr("Custom icon"));
	connect(m_pUseIcon, SIGNAL(clicked(bool)), this, SLOT(OnUseIcon(bool)));
	pIconLayout->addWidget(m_pUseIcon);
	m_pPickIcon = new QToolButton(this);
	m_pPickIcon->setText("...");
	connect(m_pPickIcon, SIGNAL(clicked(bool)), this, SLOT(OnPickIcon()));
	pIconLayout->addWidget(m_pPickIcon);
    pActionWidget->setDefaultWidget(pIconWidget);
	pColorMenu->addAction(pActionWidget);
	pColorMenu->addSeparator();
	m_pColorSlider = new QSlider(Qt::Horizontal, this);
	m_pColorSlider->setMinimum(0);
	m_pColorSlider->setMaximum(359);
	m_pColorSlider->setMinimumHeight(16);
	connect(m_pColorSlider, SIGNAL(valueChanged(int)), this, SLOT(OnColorSlider(int)));
	pActionWidget = new QWidgetAction(this);
    pActionWidget->setDefaultWidget(m_pColorSlider);
	pColorMenu->addAction(pActionWidget);
	ui.btnBorderColor->setMenu(pColorMenu);

	connect(ui.cmbBoxIndicator, SIGNAL(currentIndexChanged(int)), this, SLOT(OnGeneralChanged()));
	connect(ui.cmbBoxBorder, SIGNAL(currentIndexChanged(int)), this, SLOT(OnGeneralChanged()));
	connect(ui.btnBorderColor, SIGNAL(clicked(bool)), this, SLOT(OnPickColor()));
	connect(ui.spinBorderWidth, SIGNAL(valueChanged(int)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkShowForRun, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkPinToTray, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));

	connect(ui.cmbDblClick, SIGNAL(currentIndexChanged(int)), this, SLOT(OnActionChanged()));

	connect(ui.chkBlockNetShare, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkBlockNetParam, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));

	connect(ui.chkSecurityMode, SIGNAL(clicked(bool)), this, SLOT(OnSecurityMode()));
	connect(ui.chkLockDown, SIGNAL(clicked(bool)), this, SLOT(OnSecurityMode()));
	connect(ui.chkRestrictDevices, SIGNAL(clicked(bool)), this, SLOT(OnSecurityMode()));

	connect(ui.chkDropRights, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkFakeElevation, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkMsiExemptions, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	
	connect(ui.chkBlockSpooler, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkOpenSpooler, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkPrintToFile, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));

	connect(ui.chkOpenProtectedStorage, SIGNAL(clicked(bool)), this, SLOT(OnPSTChanged()));
	connect(ui.chkOpenCredentials, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkCloseClipBoard, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkVmRead, SIGNAL(clicked(bool)), this, SLOT(OnVmRead()));
	connect(ui.chkVmReadNotify, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	//connect(ui.chkOpenSmartCard, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	//connect(ui.chkOpenBluetooth, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));

	connect(ui.chkSeparateUserFolders, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));

	connect(ui.txtCopyLimit, SIGNAL(textChanged(const QString&)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkCopyLimit, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkCopyPrompt, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkNoCopyWarn, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));

	connect(ui.chkProtectBox, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkAutoEmpty, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));

	connect(ui.chkRawDiskRead, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkRawDiskNotify, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));

	connect(ui.btnAddCmd, SIGNAL(clicked(bool)), this, SLOT(OnAddCommand()));
	QMenu* pRunBtnMenu = new QMenu(ui.btnAddCmd);
	pRunBtnMenu->addAction(tr("Browse for Program"), this, SLOT(OnBrowsePath()));
	ui.btnAddCmd->setPopupMode(QToolButton::MenuButtonPopup);
	ui.btnAddCmd->setMenu(pRunBtnMenu);
	connect(ui.btnDelCmd, SIGNAL(clicked(bool)), this, SLOT(OnDelCommand()));
	connect(ui.treeRun, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(OnRunChanged()));
}

void COptionsWindow::LoadGeneral()
{
	QString BoxNameTitle = m_pBox->GetText("BoxNameTitle", "n", false, true, false);
	ui.cmbBoxIndicator->setCurrentIndex(ui.cmbBoxIndicator->findData(BoxNameTitle.toLower()));

	QStringList BorderCfg = m_pBox->GetText("BorderColor").split(",");
	ui.cmbBoxBorder->setCurrentIndex(ui.cmbBoxBorder->findData(BorderCfg.size() >= 2 ? BorderCfg[1].toLower() : "on"));
	SetBoxColor(QColor("#" + BorderCfg[0].mid(5, 2) + BorderCfg[0].mid(3, 2) + BorderCfg[0].mid(1, 2)));
	int BorderWidth = BorderCfg.count() >= 3 ? BorderCfg[2].toInt() : 0;
	if (!BorderWidth) BorderWidth = 6;
	ui.spinBorderWidth->setValue(BorderWidth);

	m_BoxIcon = m_pBox->GetText("BoxIcon");
	m_pUseIcon->setChecked(!m_BoxIcon.isEmpty());
	m_pPickIcon->setEnabled(!m_BoxIcon.isEmpty());
	StrPair PathIndex = Split2(m_BoxIcon, ",");
	if (!PathIndex.second.isEmpty() && !PathIndex.second.contains("."))
		ui.btnBorderColor->setIcon(LoadWindowsIcon(PathIndex.first, PathIndex.second.toInt()));
	else if (!m_BoxIcon.isEmpty())
		ui.btnBorderColor->setIcon(QPixmap(m_BoxIcon));
	else
		ui.btnBorderColor->setIcon(QIcon());
	

	ui.chkShowForRun->setChecked(m_pBox->GetBool("ShowForRunIn", true));
	ui.chkPinToTray->setChecked(m_pBox->GetBool("PinToTray", false));

	ui.chkBlockNetShare->setChecked(m_pBox->GetBool("BlockNetworkFiles", false));
	ui.chkBlockNetParam->setChecked(m_pBox->GetBool("BlockNetParam", true));
	
	ui.chkSecurityMode->setChecked(m_pBox->GetBool("UseSecurityMode", false));
	ui.chkLockDown->setChecked(m_pBox->GetBool("SysCallLockDown", false));
	ui.chkRestrictDevices->setChecked(m_pBox->GetBool("RestrictDevices", false));
		
	ui.chkDropRights->setChecked(m_pBox->GetBool("DropAdminRights", false));
	ui.chkFakeElevation->setChecked(m_pBox->GetBool("FakeAdminRights", false));
	ui.chkMsiExemptions->setChecked(m_pBox->GetBool("MsiInstallerExemptions", false));

	ui.chkBlockSpooler->setChecked(m_pBox->GetBool("ClosePrintSpooler", false));
	ui.chkOpenSpooler->setChecked(m_pBox->GetBool("OpenPrintSpooler", false));
	ui.chkPrintToFile->setChecked(m_pBox->GetBool("AllowSpoolerPrintToFile", false));

	ui.chkOpenProtectedStorage->setChecked(m_pBox->GetBool("OpenProtectedStorage", false));
	ui.chkOpenCredentials->setChecked(!ui.chkOpenCredentials->isEnabled() || m_pBox->GetBool("OpenCredentials", false));
	ui.chkCloseClipBoard->setChecked(!m_pBox->GetBool("OpenClipboard", true));
	ui.chkVmReadNotify->setChecked(m_pBox->GetBool("NotifyProcessAccessDenied", false));
	//ui.chkOpenSmartCard->setChecked(m_pBox->GetBool("OpenSmartCard", true));
	//ui.chkOpenBluetooth->setChecked(m_pBox->GetBool("OpenBluetooth", false));

	ui.cmbDblClick->clear();
	ui.cmbDblClick->addItem(tr("Open Box Options"), "!options");
	ui.cmbDblClick->addItem(tr("Browse Content"), "!browse");
	ui.cmbDblClick->addItem(tr("Start File Recovery"), "!recovery");
	ui.cmbDblClick->addItem(tr("Show Run Dialog"), "!run");
	ui.cmbDblClick->insertSeparator(4);

	ui.treeRun->clear();
	foreach(const QString& Value, m_pBox->GetTextList("RunCommand", m_Template))
	{
		StrPair NameCmd = Split2(Value, "|");
		QTreeWidgetItem* pItem = new QTreeWidgetItem();
		AddRunItem(NameCmd.first, NameCmd.second);
		ui.cmbDblClick->addItem(NameCmd.second, "");
	}

	QString Action = m_pBox->GetText("DblClickAction");
	int pos = -1;
	if (Action.isEmpty())
		pos = 0;
	else if (Action.left(1) == "!")
		pos = ui.cmbDblClick->findData(Action);
	else if (!Action.isEmpty()) 
		pos = ui.cmbDblClick->findText(Action);
	ui.cmbDblClick->setCurrentIndex(pos);
	if (pos == -1) ui.cmbDblClick->setCurrentText(Action);

	ReadGlobalCheck(ui.chkSeparateUserFolders, "SeparateUserFolders", true);

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

	if(m_pUseIcon->isChecked())
		WriteText("BoxIcon", m_BoxIcon);
	else
		m_pBox->DelValue("BoxIcon");
		

	WriteAdvancedCheck(ui.chkShowForRun, "ShowForRunIn", "", "n");
	WriteAdvancedCheck(ui.chkPinToTray, "PinToTray", "y", "");

	QString Action = ui.cmbDblClick->currentData().toString();
	if (Action.isEmpty()) Action = ui.cmbDblClick->currentText();
	if (Action == "!options") m_pBox->DelValue("DblClickAction");
	else m_pBox->SetText("DblClickAction", Action);

	WriteAdvancedCheck(ui.chkBlockNetShare, "BlockNetworkFiles", "y", "");
	WriteAdvancedCheck(ui.chkBlockNetParam, "BlockNetParam", "", "n");

	WriteAdvancedCheck(ui.chkSecurityMode, "UseSecurityMode", "y", "");
	WriteAdvancedCheck(ui.chkLockDown, "SysCallLockDown", "y", "");
	WriteAdvancedCheck(ui.chkRestrictDevices, "RestrictDevices", "y", "");

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
	WriteAdvancedCheck(ui.chkVmReadNotify, "NotifyProcessAccessDenied", "y", "");
	//WriteAdvancedCheck(ui.chkOpenSmartCard, "OpenSmartCard", "", "n");
	//WriteAdvancedCheck(ui.chkOpenBluetooth, "OpenBluetooth", "y", "");


	QStringList RunCommands;
	for (int i = 0; i < ui.treeRun->topLevelItemCount(); i++) {
		QTreeWidgetItem* pItem = ui.treeRun->topLevelItem(i);
		RunCommands.append(pItem->text(0) + "|" + pItem->text(1));
	}
	WriteTextList("RunCommand", RunCommands);

	WriteGlobalCheck(ui.chkSeparateUserFolders, "SeparateUserFolders", true);

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

	UpdateBoxSecurity();

	m_GeneralChanged = true;
	OnOptChanged();
}

void COptionsWindow::UpdateBoxSecurity()
{
	ui.chkLockDown->setEnabled(!ui.chkSecurityMode->isChecked());
	ui.chkRestrictDevices->setEnabled(!ui.chkSecurityMode->isChecked());

	if (!theAPI->IsRunningAsAdmin()) {
		ui.chkDropRights->setEnabled(!ui.chkSecurityMode->isChecked() && !ui.chkNoSecurityIsolation->isChecked() && !theAPI->IsRunningAsAdmin());
	}

	if (ui.chkSecurityMode->isChecked()) {
		ui.chkLockDown->setChecked(true);
		ui.chkRestrictDevices->setChecked(true);

		ui.chkDropRights->setChecked(true);
	}

	ui.chkMsiExemptions->setEnabled(!ui.chkDropRights->isChecked());
}

void COptionsWindow::OnSecurityMode()
{
	if (ui.chkSecurityMode->isChecked() || (ui.chkLockDown->isEnabled() && ui.chkLockDown->isChecked()) || (ui.chkRestrictDevices->isEnabled() && ui.chkRestrictDevices->isChecked()))
		theGUI->CheckCertificate(this);

	UpdateBoxSecurity();

	if (sender() == ui.chkSecurityMode && !ui.chkSecurityMode->isChecked()) {
		ui.chkLockDown->setChecked(m_pBox->GetBool("SysCallLockDown", false));
		ui.chkRestrictDevices->setChecked(m_pBox->GetBool("RestrictDevices", false));
		
		ui.chkDropRights->setChecked(m_pBox->GetBool("DropAdminRights", false));
	}

	m_GeneralChanged = true;
	OnOptChanged();

	OnAccessChanged(); // for rule specificity
}

void COptionsWindow::OnUseIcon(bool bUse)
{
	if (bUse) {
		if (m_BoxIcon.isEmpty()) {
			QString ActionFile = GetActionFile();
			if (!ActionFile.isEmpty()) {
				ui.btnBorderColor->setIcon(LoadWindowsIcon(ActionFile, 0));
				m_BoxIcon = QString("%1,0").arg(ActionFile);
			}
		}
		if (m_BoxIcon.isEmpty()) {
			if (!OnPickIcon())
				m_pUseIcon->setChecked(false);
		}
	}
	else
		ui.btnBorderColor->setIcon(QPixmap());

	m_pPickIcon->setEnabled(m_pUseIcon->isChecked());

	m_GeneralChanged = true;
	OnOptChanged();
}

bool COptionsWindow::OnPickIcon()
{
	QString Path;
	quint32 Index = 0;

	StrPair PathIndex = Split2(m_BoxIcon, ",");
	if (!PathIndex.second.isEmpty() && !PathIndex.second.contains(".")) {
		Path = PathIndex.first;
		Index = PathIndex.second.toInt();
	}

	if (!PickWindowsIcon(this, Path, Index))
		return false;

	ui.btnBorderColor->setIcon(LoadWindowsIcon(Path, Index));
	m_BoxIcon = QString("%1,%2").arg(Path).arg(Index);

	m_GeneralChanged = true;
	OnOptChanged();

	return true;
}

void COptionsWindow::OnPickColor()
{
	QColor color = QColorDialog::getColor(m_BorderColor, this, tr("Select color"));
	if (!color.isValid())
		return;
	m_GeneralChanged = true;
	OnOptChanged();
	SetBoxColor(color);
}

void COptionsWindow::SetBoxColor(const QColor& color)
{
	if (m_BorderColor == color)
		return;
	QRgb qrgb = color.rgba();
	my_rgb rgb = { (double)qRed(qrgb), (double)qGreen(qrgb), (double)qBlue(qrgb) };
	my_hsv hsv = rgb2hsv(rgb);
	m_pColorSlider->setValue((int)hsv.h);
	m_BorderColor = color;
	UpdateBoxColor();
}

void COptionsWindow::OnColorSlider(int value)
{
	my_hsv hsv = { (double)value, 1, 255 };
	my_rgb rgb = hsv2rgb(hsv);
	m_GeneralChanged = true;
	OnOptChanged();
	SetBoxColor(qRgb(rgb.r, rgb.g, rgb.b));
}

void COptionsWindow::UpdateBoxColor()
{
	if(theConf->GetBool("Options/ColorBoxIcons", false))
		ui.btnBorderColor->setIcon(theGUI->GetColorIcon(m_BorderColor, true));
	else
		ui.btnBorderColor->setStyleSheet("background-color: " + m_BorderColor.name());
}

QString COptionsWindow::GetActionFile()
{
	QString Action = ui.cmbDblClick->currentData().toString();
	if (Action.isEmpty()) Action = ui.cmbDblClick->currentText();
	if (!Action.isEmpty() && Action.left(1) != "!") {
		CSandBoxPlus* pBoxEx = qobject_cast<CSandBoxPlus*>(m_pBox.data());
		if (pBoxEx) {
			QString Path = pBoxEx->GetCommandFile(Action);
			ui.btnBorderColor->setIcon(LoadWindowsIcon(Path, 0));
			return Path;
		}
	}
	return QString();
}

void COptionsWindow::OnActionChanged()
{
	if (m_HoldChange)
		return;

	QString ActionFile = GetActionFile();
	if (!ActionFile.isEmpty()) {
		ui.btnBorderColor->setIcon(LoadWindowsIcon(ActionFile, 0));
		m_pUseIcon->setChecked(true);
		m_BoxIcon = QString("%1,0").arg(ActionFile);
	}
	else {
		ui.btnBorderColor->setIcon(QPixmap());
		m_pUseIcon->setChecked(false);
		m_BoxIcon.clear();
	}

	m_GeneralChanged = true;
	OnOptChanged();
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
	bool bSecurityMode = ui.chkSecurityMode->isChecked();
	bool bAppBox = ui.chkNoSecurityIsolation->isChecked();

	int BoxType;
	if (bAppBox) 
		BoxType = bPrivacyMode ? (int)CSandBoxPlus::eAppBoxPlus : (int)CSandBoxPlus::eAppBox;
	else if (bSecurityMode) 
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
		ui.chkSecurityMode->setChecked(true);
		//ui.chkRestrictServices->setChecked(true);
		ui.chkPrivacy->setChecked(BoxType == CSandBoxPlus::eHardenedPlus);
		//SetTemplate("NoUACProxy", false);
		SetTemplate("RpcPortBindingsExt", false);
		break;
	case CSandBoxPlus::eDefaultPlus:
	case CSandBoxPlus::eDefault:
		ui.chkNoSecurityIsolation->setChecked(false);
		ui.chkNoSecurityFiltering->setChecked(false);
		ui.chkSecurityMode->setChecked(false);
		//ui.chkRestrictServices->setChecked(true);
		ui.chkPrivacy->setChecked(BoxType == CSandBoxPlus::eDefaultPlus);
		//SetTemplate("NoUACProxy", false);
		break;
	case CSandBoxPlus::eAppBoxPlus:
	case CSandBoxPlus::eAppBox:
		ui.chkNoSecurityIsolation->setChecked(true);
		ui.chkSecurityMode->setChecked(false);
		//ui.chkRestrictServices->setChecked(false);
		ui.chkPrivacy->setChecked(BoxType == CSandBoxPlus::eAppBoxPlus);
		//SetTemplate("NoUACProxy", true);
		SetTemplate("RpcPortBindingsExt", true);
		break;
	}

	SetBoxColor(theGUI->GetBoxColor(BoxType));

	m_GeneralChanged = true;
	m_AccessChanged = true;
	m_AdvancedChanged = true;

	m_HoldBoxType = true;
	UpdateBoxType();
	m_HoldBoxType = false;
	OnOptChanged();
}

void COptionsWindow::OnVmRead()
{
	if (ui.chkVmRead->isChecked())
		SetAccessEntry(eIPC, "", eReadOnly, "$:*");
	else
		DelAccessEntry(eIPC, "", eReadOnly, "$:*");
	m_AdvancedChanged = true;
	OnOptChanged();
}
