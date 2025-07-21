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
#include "Windows/BoxImageWindow.h"
#include "AddonManager.h"
#include "../../../SandboxieTools/ImBox/ImBox.h"

class CCertBadge: public QLabel
{
public:
	CCertBadge(bool bAdvanced, QWidget* parent = NULL): QLabel(parent) 
	{
		m_bAdvanced = bAdvanced;
		setPixmap(QPixmap(":/Actions/Cert.png").scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
		if(bAdvanced)
			setToolTip(COptionsWindow::tr("This option requires an active <b>advanced</b> supporter certificate"));
		else
			setToolTip(COptionsWindow::tr("This option requires an active supporter certificate"));
		setCursor(Qt::PointingHandCursor);
	}

protected:
	void mousePressEvent(QMouseEvent* event)
	{
		if(m_bAdvanced && g_CertInfo.active)
			theGUI->OpenUrl(QUrl("https://sandboxie-plus.com/go.php?to=sbie-upgrade-cert"));
		else
			theGUI->OpenUrl(QUrl("https://sandboxie-plus.com/go.php?to=sbie-get-cert"));
	}
	bool m_bAdvanced;
};

void COptionsWindow__AddCertIcon(QWidget* pOriginalWidget, bool bAdvanced = false)
{
	QWidget* pWidget = new QWidget();
	QHBoxLayout* pLayout = new QHBoxLayout(pWidget);
	pLayout->setContentsMargins(0, 0, 0, 0);
	pLayout->setSpacing(0);
	pLayout->addWidget(new CCertBadge(bAdvanced));
	pLayout->setAlignment(Qt::AlignLeft);
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
	ui.cmbBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eAppBox), tr("Application Compartment"), (int)CSandBoxPlus::eAppBox);

	connect(ui.lblBoxInfo, SIGNAL(linkActivated(const QString&)), theGUI, SLOT(OpenUrl(const QString&)));

	ui.lblSupportCert->setVisible(false);
	if (!g_CertInfo.active)
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

	if (!g_CertInfo.opt_sec) {
		QWidget* ExWidgets[] = { ui.chkSecurityMode, ui.chkLockDown, ui.chkRestrictDevices, ui.chkPrivacy, ui.chkUseSpecificity, ui.chkNoSecurityIsolation, ui.chkNoSecurityFiltering, ui.chkHostProtect, NULL };
		for (QWidget** ExWidget = ExWidgets; *ExWidget != NULL; ExWidget++)
			COptionsWindow__AddCertIcon(*ExWidget);
	}
	if (!g_CertInfo.active)
		COptionsWindow__AddCertIcon(ui.chkRamBox, true);
	if (!g_CertInfo.opt_enc) {
		COptionsWindow__AddCertIcon(ui.chkConfidential, true);
		COptionsWindow__AddCertIcon(ui.chkEncrypt, true);
		COptionsWindow__AddCertIcon(ui.chkAllowEfs, true);
	}


	m_HoldBoxType = false;

	connect(ui.cmbBoxType, SIGNAL(currentIndexChanged(int)), this, SLOT(OnBoxTypChanged()));

	connect(ui.chkSecurityMode, SIGNAL(clicked(bool)), this, SLOT(UpdateBoxType()));
	connect(ui.chkPrivacy, SIGNAL(clicked(bool)), this, SLOT(UpdateBoxType()));
	connect(ui.chkNoSecurityIsolation, SIGNAL(clicked(bool)), this, SLOT(UpdateBoxType()));
	//connect(ui.chkNoSecurityFiltering, SIGNAL(clicked(bool)), this, SLOT(UpdateBoxType()));

	
	ui.btnBorderColor->setPopupMode(QToolButton::MenuButtonPopup);
	ui.btnBorderColor->setStyle(QStyleFactory::create("Fusion"));
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

	connect(ui.chkSecurityMode, SIGNAL(clicked(bool)), this, SLOT(OnSecurityMode()));
	connect(ui.chkLockDown, SIGNAL(clicked(bool)), this, SLOT(OnSecurityMode()));
	connect(ui.chkRestrictDevices, SIGNAL(clicked(bool)), this, SLOT(OnSecurityMode()));

	connect(ui.chkDropRights, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkFakeElevation, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkMsiExemptions, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkACLs, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	
	connect(ui.chkBlockSpooler, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkOpenSpooler, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkPrintToFile, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));

	connect(ui.chkOpenProtectedStorage, SIGNAL(clicked(bool)), this, SLOT(OnPSTChanged()));
	connect(ui.chkOpenCredentials, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkCloseClipBoard, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	//connect(ui.chkBlockCapture, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkProtectPower, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkVmRead, SIGNAL(clicked(bool)), this, SLOT(OnVmRead()));
	connect(ui.chkVmReadNotify, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	//connect(ui.chkOpenSmartCard, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	//connect(ui.chkOpenBluetooth, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));

	ui.cmbVersion->addItem(tr("Version 1"));
	ui.cmbVersion->addItem(tr("Version 2"));

	auto pBoxEx = m_pBox.objectCast<CSandBoxPlus>();
	bool bEmpty = pBoxEx ? pBoxEx->IsEmpty() : true;
	ui.lblWhenEmpty->setVisible(!bEmpty);
	ui.lblScheme->setEnabled(bEmpty);
	ui.cmbVersion->setEnabled(bEmpty);
	ui.chkSeparateUserFolders->setEnabled(bEmpty);
	ui.chkUseVolumeSerialNumbers->setEnabled(bEmpty);

	connect(ui.cmbVersion, SIGNAL(currentIndexChanged(int)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkSeparateUserFolders, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkUseVolumeSerialNumbers, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));

	connect(ui.chkRamBox, SIGNAL(clicked(bool)), this, SLOT(OnDiskChanged()));
	connect(ui.chkEncrypt, SIGNAL(clicked(bool)), this, SLOT(OnDiskChanged()));
	connect(ui.chkForceProtection, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkUserOperation, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkCoverBar, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.btnPassword, SIGNAL(clicked(bool)), this, SLOT(OnSetPassword()));

	bool bImDiskReady = theGUI->IsImDiskReady();
	ui.lblImDisk->setVisible(!bImDiskReady);
	connect(ui.lblImDisk, SIGNAL(linkActivated(const QString&)), theGUI, SLOT(OpenUrl(const QString&)));
	QObject::connect(theGUI->GetAddonManager(), &CAddonManager::AddonInstalled, this, [=] {
		if (!theGUI->GetAddonManager()->GetAddon("ImDisk", CAddonManager::eInstalled).isNull()) {
			ui.lblImDisk->setVisible(false);
			ui.chkRamBox->setEnabled(bEmpty);
			ui.chkEncrypt->setEnabled(bEmpty);
			ui.lblCrypto->setEnabled(true);
		}
	});
	ui.chkRamBox->setEnabled(bImDiskReady && bEmpty);
	ui.chkEncrypt->setEnabled(bImDiskReady && bEmpty);
	ui.lblCrypto->setEnabled(bImDiskReady);

	connect(ui.txtCopyLimit, SIGNAL(textChanged(const QString&)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkCopyLimit, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkCopyPrompt, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkNoCopyWarn, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkDenyWrite, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkNoCopyMsg, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));

	connect(ui.btnAddCopy, SIGNAL(clicked(bool)), this, SLOT(OnAddCopyRule()));
	connect(ui.btnDelCopy, SIGNAL(clicked(bool)), this, SLOT(OnDelCopyRule()));
	connect(ui.treeCopy, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnCopyItemDoubleClicked(QTreeWidgetItem*, int)));
	connect(ui.treeCopy, SIGNAL(itemSelectionChanged()), this, SLOT(OnCopySelectionChanged()));
	connect(ui.treeCopy, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(OnCopyChanged(QTreeWidgetItem*, int)));
	connect(ui.chkShowCopyTmpl, SIGNAL(clicked(bool)), this, SLOT(OnShowCopyTmpl()));

	connect(ui.chkProtectBox, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkAutoEmpty, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));

	connect(ui.chkRawDiskRead, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkRawDiskNotify, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));
	
	connect(ui.chkAllowEfs, SIGNAL(clicked(bool)), this, SLOT(OnGeneralChanged()));

	connect(ui.btnAddCmd, SIGNAL(clicked(bool)), this, SLOT(OnAddCommand()));
	QMenu* pRunBtnMenu = new QMenu(ui.btnAddCmd);
	pRunBtnMenu->addAction(tr("Browse for Program"), this, SLOT(OnBrowsePath()));
	ui.btnAddCmd->setPopupMode(QToolButton::MenuButtonPopup);
	ui.btnAddCmd->setMenu(pRunBtnMenu);
	connect(ui.btnCmdUp, SIGNAL(clicked(bool)), this, SLOT(OnCommandUp()));
	connect(ui.btnCmdDown, SIGNAL(clicked(bool)), this, SLOT(OnCommandDown()));
	connect(ui.btnDelCmd, SIGNAL(clicked(bool)), this, SLOT(OnDelCommand()));
	connect(ui.treeRun, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(OnRunChanged()));

	connect(ui.txtNotes, SIGNAL(textChanged()), this, SLOT(OnGeneralChanged()));
}

void COptionsWindow::LoadGeneral()
{
	QString BoxNameTitle = ReadTextSafe("BoxNameTitle", "n");
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
	
	ui.chkSecurityMode->setChecked(m_pBox->GetBool("UseSecurityMode", false));
	ui.chkLockDown->setChecked(m_pBox->GetBool("SysCallLockDown", false));
	ui.chkRestrictDevices->setChecked(m_pBox->GetBool("RestrictDevices", false));
		
	ui.chkDropRights->setChecked(m_pBox->GetBool("DropAdminRights", false));
	ui.chkFakeElevation->setChecked(m_pBox->GetBool("FakeAdminRights", false));
	ui.chkMsiExemptions->setChecked(m_pBox->GetBool("MsiInstallerExemptions", false));
	ui.chkACLs->setChecked(m_pBox->GetBool("UseOriginalACLs", false));

	ui.chkBlockSpooler->setChecked(m_pBox->GetBool("ClosePrintSpooler", false));
	ui.chkOpenSpooler->setChecked(m_pBox->GetBool("OpenPrintSpooler", false));
	ui.chkPrintToFile->setChecked(m_pBox->GetBool("AllowSpoolerPrintToFile", false));

	//ui.chkOpenProtectedStorage->setChecked(m_pBox->GetBool("OpenProtectedStorage", false));
	ui.chkOpenProtectedStorage->setChecked(m_BoxTemplates.contains("OpenProtectedStorage"));
	ui.chkOpenCredentials->setChecked(!ui.chkOpenCredentials->isEnabled() || m_pBox->GetBool("OpenCredentials", false));
	ui.chkCloseClipBoard->setChecked(!m_pBox->GetBool("OpenClipboard", true));
	//ui.chkBlockCapture->setChecked(m_pBox->GetBool("BlockScreenCapture", false));
	ui.chkProtectPower->setChecked(m_pBox->GetBool("BlockInterferePower", false));
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
		AddRunItem(ui.treeRun, GetRunEntry(Value));

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


	bool bUseFileDeleteV2Global = theAPI->GetGlobalSettings()->GetBool("UseFileDeleteV2", false);
	bool bUseRegDeleteV2Global = theAPI->GetGlobalSettings()->GetBool("UseRegDeleteV2", false);
	if (m_pBox->GetBool("UseFileDeleteV2", bUseFileDeleteV2Global) && m_pBox->GetBool("UseRegDeleteV2", bUseRegDeleteV2Global))
		ui.cmbVersion->setCurrentIndex(1);
	else if (!m_pBox->GetBool("UseFileDeleteV2", bUseFileDeleteV2Global) && !m_pBox->GetBool("UseRegDeleteV2", bUseRegDeleteV2Global))
		ui.cmbVersion->setCurrentIndex(0);
	else {
		ui.cmbVersion->setEditable(true);
		ui.cmbVersion->lineEdit()->setReadOnly(true);
		ui.cmbVersion->setCurrentText(tr("Indeterminate"));
	}

	ReadGlobalCheck(ui.chkSeparateUserFolders, "SeparateUserFolders", theAPI->GetGlobalSettings()->GetBool("SeparateUserFolders", true));
	ReadGlobalCheck(ui.chkUseVolumeSerialNumbers, "UseVolumeSerialNumbers", theAPI->GetGlobalSettings()->GetBool("UseVolumeSerialNumbers", false));

	ui.chkRamBox->setChecked(m_pBox->GetBool("UseRamDisk", false));
	ui.chkEncrypt->setChecked(m_pBox->GetBool("UseFileImage", false));
	ui.chkForceProtection->setChecked(m_pBox->GetBool("ForceProtectionOnMount", false));
	ui.chkUserOperation->setChecked(m_pBox->GetBool("BlockInterferenceControl", false));
	ui.chkCoverBar->setChecked(m_pBox->GetBool("AllowCoverTaskbar", false));
	if (ui.chkRamBox->isEnabled())
		ui.chkEncrypt->setEnabled(!ui.chkRamBox->isChecked());
	ui.chkForceProtection->setEnabled(ui.chkEncrypt->isEnabled() && ui.chkEncrypt->isChecked());

	auto pBoxEx = m_pBox.objectCast<CSandBoxPlus>();
	if (pBoxEx && QFile::exists(pBoxEx->GetBoxImagePath())) 
	{
		if (!ui.btnPassword->menu()) {
			QMenu* pCryptoMenu = new QMenu();
			pCryptoMenu->addAction(tr("Backup Image Header"), this, SLOT(OnBackupHeader()));
			pCryptoMenu->addAction(tr("Restore Image Header"), this, SLOT(OnRestoreHeader()));
			ui.btnPassword->setPopupMode(QToolButton::MenuButtonPopup);
			ui.btnPassword->setMenu(pCryptoMenu);
		}
		ui.btnPassword->setText(tr("Change Password"));
	}
	ui.btnPassword->setEnabled(!ui.chkRamBox->isChecked() && ui.chkEncrypt->isChecked() && pBoxEx && pBoxEx->GetMountRoot().isEmpty());

	int iLimit = m_pBox->GetNum("CopyLimitKb", 80 * 1024);
	ui.chkCopyLimit->setChecked(iLimit != -1);
	ui.txtCopyLimit->setText(QString::number(iLimit > 0 ? iLimit : 80 * 1024));
	ui.chkCopyPrompt->setChecked(m_pBox->GetBool("PromptForFileMigration", true));
	ui.chkNoCopyWarn->setChecked(!m_pBox->GetBool("CopyLimitSilent", false));
	ui.chkDenyWrite->setChecked(!m_pBox->GetBool("CopyBlockDenyWrite", false));
	ui.chkNoCopyMsg->setChecked(m_pBox->GetBool("NotifyNoCopy", false));
	
	LoadCopyRules();

	if (m_pBox->GetBool("NeverDelete", false))
		ui.chkProtectBox->setCheckState(Qt::Checked);
	else if (m_pBox->GetBool("NeverRemove", false))
		ui.chkProtectBox->setCheckState(Qt::PartiallyChecked);
	else
		ui.chkProtectBox->setCheckState(Qt::Unchecked);
	ui.chkAutoEmpty->setChecked(m_pBox->GetBool("AutoDelete", false));

	ui.chkRawDiskRead->setChecked(m_pBox->GetBool("AllowRawDiskRead", false));
	ui.chkRawDiskNotify->setChecked(m_pBox->GetBool("NotifyDirectDiskAccess", false));

	ui.chkAllowEfs->setChecked(m_pBox->GetBool("EnableEFS", false));

	QString Note;
	foreach(QString Value, m_pBox->GetTextList("Note", false)) {
		if (!Note.isEmpty())
			Note += "\n";
		if (Value == "_")
			Value = "";
		Note += Value;
	}
	ui.txtNotes->setPlainText(Note);

	OnGeneralChanged();

	m_GeneralChanged = false;
}

void COptionsWindow::SaveGeneral()
{
	QString BoxNameTitle = ui.cmbBoxIndicator->currentData().toString();
	if (BoxNameTitle == "n")
		WriteTextSafe("BoxNameTitle", "");
	else
		WriteTextSafe("BoxNameTitle", BoxNameTitle);

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

	WriteAdvancedCheck(ui.chkSecurityMode, "UseSecurityMode", "y", "");
	WriteAdvancedCheck(ui.chkLockDown, "SysCallLockDown", "y", "");
	WriteAdvancedCheck(ui.chkRestrictDevices, "RestrictDevices", "y", "");

	WriteAdvancedCheck(ui.chkDropRights, "DropAdminRights", "y", "");
	WriteAdvancedCheck(ui.chkFakeElevation, "FakeAdminRights", "y", "");
	WriteAdvancedCheck(ui.chkMsiExemptions, "MsiInstallerExemptions", "y", "");
	WriteAdvancedCheck(ui.chkACLs, "UseOriginalACLs", "y", "");

	WriteAdvancedCheck(ui.chkBlockSpooler, "ClosePrintSpooler", "y", "");
	WriteAdvancedCheck(ui.chkOpenSpooler, "OpenPrintSpooler", "y", "");
	WriteAdvancedCheck(ui.chkPrintToFile, "AllowSpoolerPrintToFile", "y", "");

	//WriteAdvancedCheck(ui.chkOpenProtectedStorage, "OpenProtectedStorage", "y", "");
	SetTemplate("OpenProtectedStorage", ui.chkOpenProtectedStorage->isChecked());
	if (ui.chkOpenCredentials->isEnabled())
		WriteAdvancedCheck(ui.chkOpenCredentials, "OpenCredentials", "y", "");
	WriteAdvancedCheck(ui.chkCloseClipBoard, "OpenClipboard", "n", "");
	//WriteAdvancedCheck(ui.chkBlockCapture, "BlockScreenCapture", "y", "");
	WriteAdvancedCheck(ui.chkProtectPower, "BlockInterferePower", "y", "");
	WriteAdvancedCheck(ui.chkForceProtection, "ForceProtectionOnMount", "y", "");
	WriteAdvancedCheck(ui.chkUserOperation, "BlockInterferenceControl", "y", "");
	WriteAdvancedCheck(ui.chkCoverBar, "AllowCoverTaskbar", "y", "");
	WriteAdvancedCheck(ui.chkVmReadNotify, "NotifyProcessAccessDenied", "y", "");
	//WriteAdvancedCheck(ui.chkOpenSmartCard, "OpenSmartCard", "", "n");
	//WriteAdvancedCheck(ui.chkOpenBluetooth, "OpenBluetooth", "y", "");


	QStringList RunCommands;
	for (int i = 0; i < ui.treeRun->topLevelItemCount(); i++)
		RunCommands.prepend(MakeRunEntry(ui.treeRun->topLevelItem(i)));

	//WriteTextList("RunCommand", RunCommands);
	m_pBox->DelValue("RunCommand");
	foreach(const QString& Value, RunCommands)
		m_pBox->AppendText("RunCommand", Value);


	if (ui.cmbVersion->isEnabled()) 
	{
		if (ui.cmbVersion->currentIndex() == 1) // V2
		{
			m_pBox->SetBool("UseFileDeleteV2", true);
			m_pBox->SetBool("UseRegDeleteV2", true);
		}
		else if (ui.cmbVersion->currentIndex() == 0) // V1
		{
			bool bUseFileDeleteV2Global = theAPI->GetGlobalSettings()->GetBool("UseFileDeleteV2", false);
			if(bUseFileDeleteV2Global)
				m_pBox->SetBool("UseFileDeleteV2", false);
			else
				m_pBox->DelValue("UseFileDeleteV2");

			bool bUseRegDeleteV2Global = theAPI->GetGlobalSettings()->GetBool("UseRegDeleteV2", false);
			if(bUseRegDeleteV2Global)
				m_pBox->SetBool("UseRegDeleteV2", false);
			else
				m_pBox->DelValue("UseRegDeleteV2");
		}

		WriteGlobalCheck(ui.chkSeparateUserFolders, "SeparateUserFolders", true);
		WriteGlobalCheck(ui.chkUseVolumeSerialNumbers, "UseVolumeSerialNumbers", false);

		WriteAdvancedCheck(ui.chkRamBox, "UseRamDisk", "y", "");
		WriteAdvancedCheck(ui.chkEncrypt, "UseFileImage", "y", "");
	}

	int iLimit = ui.chkCopyLimit->isChecked() ? ui.txtCopyLimit->text().toInt() : -1;
	if(iLimit != 80 * 1024)
		WriteText("CopyLimitKb", QString::number(iLimit));
	else
		m_pBox->DelValue("CopyLimitKb");

	WriteAdvancedCheck(ui.chkCopyPrompt, "PromptForFileMigration", "", "n");
	WriteAdvancedCheck(ui.chkNoCopyWarn, "CopyLimitSilent", "", "y");
	WriteAdvancedCheck(ui.chkDenyWrite, "CopyBlockDenyWrite", "", "y");
	WriteAdvancedCheck(ui.chkNoCopyMsg, "NotifyNoCopy", "y", "");

	if (ui.chkProtectBox->checkState() == Qt::Checked) {
		m_pBox->SetText("NeverDelete", "y");
		m_pBox->SetText("NeverRemove", "y");
	}
	else if (ui.chkProtectBox->checkState() == Qt::PartiallyChecked) {
		m_pBox->DelValue("NeverDelete");
		m_pBox->SetText("NeverRemove", "y");
	}
	else {
		m_pBox->DelValue("NeverDelete");
		m_pBox->DelValue("NeverRemove");
	}
	WriteAdvancedCheck(ui.chkAutoEmpty, "AutoDelete", "y", "");

	WriteAdvancedCheck(ui.chkRawDiskRead, "AllowRawDiskRead", "y", "");
	WriteAdvancedCheck(ui.chkRawDiskNotify, "NotifyDirectDiskAccess", "y", "");

	WriteAdvancedCheck(ui.chkAllowEfs, "EnableEFS", "y", "");

	m_pBox->DelValue("Note");
	QString Note = ui.txtNotes->toPlainText();
	if (!Note.isEmpty()) {
		foreach(QString Value, Note.split("\n")) {
			if (Value == "")
				Value = "_";
			m_pBox->AppendText("Note", Value);
		}
	}

	m_GeneralChanged = false;
}

// copy
void COptionsWindow::LoadCopyRules()
{
	ui.treeCopy->clear();

	foreach(const QString & Value, m_pBox->GetTextList("CopyAlways", m_Template))
		ParseAndAddCopyRule(Value, eCopyAlways);
	foreach(const QString & Value, m_pBox->GetTextList("DontCopy", m_Template))
		ParseAndAddCopyRule(Value, eDontCopy);
	foreach(const QString & Value, m_pBox->GetTextList("CopyEmpty", m_Template))
		ParseAndAddCopyRule(Value, eCopyEmpty);

	foreach(const QString & Value, m_pBox->GetTextList("CopyAlwaysDisabled", m_Template))
		ParseAndAddCopyRule(Value, eCopyAlways, true);
	foreach(const QString & Value, m_pBox->GetTextList("DontCopyDisabled", m_Template))
		ParseAndAddCopyRule(Value, eDontCopy, true);
	foreach(const QString & Value, m_pBox->GetTextList("CopyEmptyDisabled", m_Template))
		ParseAndAddCopyRule(Value, eCopyEmpty, true);

	LoadCopyRulesTmpl();

	m_CopyRulesChanged = false;
}

void COptionsWindow::LoadCopyRulesTmpl(bool bUpdate)
{
	if (ui.chkShowCopyTmpl->isChecked())
	{
		foreach(const QString & Template, m_pBox->GetTemplates())
		{
			foreach(const QString & Value, m_pBox->GetTextListTmpl("CopyAlways", Template))
				ParseAndAddCopyRule(Value, eCopyAlways, false, Template);
			foreach(const QString & Value, m_pBox->GetTextListTmpl("DontCopy", Template))
				ParseAndAddCopyRule(Value, eDontCopy, false, Template);
			foreach(const QString & Value, m_pBox->GetTextListTmpl("CopyEmpty", Template))
				ParseAndAddCopyRule(Value, eCopyEmpty, false, Template);
		}
	}
	else if (bUpdate)
	{
		for (int i = 0; i < ui.treeCopy->topLevelItemCount(); )
		{
			QTreeWidgetItem* pItem = ui.treeCopy->topLevelItem(i);
			int Type = pItem->data(0, Qt::UserRole).toInt();
			if (Type == -1) {
				delete pItem;
				continue; // entry from template
			}
			i++;
		}
	}
}

QString COptionsWindow::GetCopyActionStr(ECopyAction Action)
{
	switch (Action)
	{
	case eCopyAlways:	return tr("Always copy");
	case eDontCopy:		return tr("Don't copy");
	case eCopyEmpty:	return tr("Copy empty");
	}
	return "";
}

void COptionsWindow::ParseAndAddCopyRule(const QString& Value, ECopyAction Action, bool disabled, const QString& Template)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem();

	pItem->setText(0, GetCopyActionStr(Action));
	pItem->setData(0, Qt::UserRole, Template.isEmpty() ? (int)Action : -1);

	QString Program;
	QString Pattern;
	QStringList Values = Value.split(",");
	if (Values.size() >= 2) {
		Program = Values[0];
		Pattern = Values[1];
	}
	else 
		Pattern = Values[0];

	// todo this block is also used by access move this to an own function
	pItem->setData(1, Qt::UserRole, Program);
	bool bAll = Program.isEmpty();
	if (bAll)
		Program = tr("All Programs");
	bool Not = Program.left(1) == "!";
	if (Not)
		Program.remove(0, 1);
	if (Program.left(1) == "<")
		Program = tr("Group: %1").arg(Program.mid(1, Program.length() - 2));
	else if (!bAll)
		m_Programs.insert(Program);
	pItem->setText(1, (Not ? "NOT " : "") + Program);

	pItem->setText(2, Pattern);

	if (Template.isEmpty())
		pItem->setCheckState(0, disabled ? Qt::Unchecked : Qt::Checked);
	ui.treeCopy->addTopLevelItem(pItem);
}

void COptionsWindow::SaveCopyRules()
{
	QList<QString> CopyAlways;
	QList<QString> CopyAlwaysDisabled;
	QList<QString> DontCopy;
	QList<QString> DontCopyDisabled;
	QList<QString> CopyEmpty;
	QList<QString> CopyEmptyDisabled;
	for (int i = 0; i < ui.treeCopy->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeCopy->topLevelItem(i);
		int Type = pItem->data(0, Qt::UserRole).toInt();
		if (Type == -1)
			continue; // entry from template
		ECopyAction Action = (ECopyAction)pItem->data(0, Qt::UserRole).toInt();
		QString Program = pItem->data(1, Qt::UserRole).toString();
		QString Pattern = pItem->text(2);
		
		if (!Program.isEmpty())
			Pattern.prepend(Program + ",");

		if (pItem->checkState(0) == Qt::Checked) {
			switch (Action) {
			case eCopyAlways:	CopyAlways.append(Pattern); break;
			case eDontCopy:		DontCopy.append(Pattern); break;
			case eCopyEmpty:	CopyEmpty.append(Pattern); break;
			}
		}
		else {
			switch (Action) {
			case eCopyAlways:	CopyAlwaysDisabled.append(Pattern); break;
			case eDontCopy:		DontCopyDisabled.append(Pattern); break;
			case eCopyEmpty:	CopyEmptyDisabled.append(Pattern); break;
			}
		}
	}
	WriteTextList("CopyAlways", CopyAlways);
	WriteTextList("CopyAlwaysDisabled", CopyAlwaysDisabled);
	WriteTextList("DontCopy", DontCopy);
	WriteTextList("DontCopyDisabled", DontCopyDisabled);
	WriteTextList("CopyEmpty", CopyEmpty);
	WriteTextList("CopyEmptyDisabled", CopyEmptyDisabled);

	m_CopyRulesChanged = false;
}

void COptionsWindow::OnCopyItemDoubleClicked(QTreeWidgetItem* pItem, int Column)
{
	int Action = pItem->data(0, Qt::UserRole).toInt();
	if (Action == -1) {
		QMessageBox::warning(this, "SandboxiePlus", tr("Template values can not be edited."));
		return;
	}

	QComboBox* pMode = new QComboBox();
	pMode->addItem(tr("Always copy"), (int)eCopyAlways);
	pMode->addItem(tr("Don't copy"), (int)eDontCopy);
	pMode->addItem(tr("Copy empty"), (int)eCopyEmpty);
	pMode->setCurrentIndex(pMode->findData(pItem->data(0, Qt::UserRole)));
	ui.treeCopy->setItemWidget(pItem, 0, pMode);

	QString Program = pItem->data(1, Qt::UserRole).toString();

	// todo: 
	QWidget* pProgram = new QWidget();
	pProgram->setAutoFillBackground(true);
	QHBoxLayout* pLayout = new QHBoxLayout();
	pLayout->setContentsMargins(0, 0, 0, 0);
	pLayout->setSpacing(0);
	pProgram->setLayout(pLayout);
	QToolButton* pNot = new QToolButton(pProgram);
	pNot->setText("!");
	pNot->setCheckable(true);
	if (Program.left(1) == "!") {
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
	if (Index == -1)
		pCombo->setCurrentText(Program);
	pLayout->addWidget(pCombo);

	ui.treeCopy->setItemWidget(pItem, 1, pProgram);

	QLineEdit* pPattern = new QLineEdit();
	pPattern->setText(pItem->text(2));
	ui.treeCopy->setItemWidget(pItem, 2, pPattern);
}

void COptionsWindow::OnCopyChanged(QTreeWidgetItem* pItem, int Column)
{
	if (Column != 0)
		return;

	m_CopyRulesChanged = true;
	OnOptChanged();
}

void COptionsWindow::CloseCopyEdit(bool bSave)
{
	for (int i = 0; i < ui.treeCopy->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeCopy->topLevelItem(i);
		CloseCopyEdit(pItem, bSave);
	}
}

void COptionsWindow::CloseCopyEdit(QTreeWidgetItem* pItem, bool bSave)
{
	QWidget* pProgram = ui.treeCopy->itemWidget(pItem, 1);
	if (!pProgram)
		return;

	if (bSave)
	{
		QComboBox* pAction = (QComboBox*)ui.treeCopy->itemWidget(pItem, 0);

		QHBoxLayout* pLayout = (QHBoxLayout*)pProgram->layout();
		QToolButton* pNot = (QToolButton*)pLayout->itemAt(0)->widget();
		QComboBox* pCombo = (QComboBox*)pLayout->itemAt(1)->widget();

		QLineEdit* pPattern = (QLineEdit*)ui.treeCopy->itemWidget(pItem, 2);

		QString Program = pCombo->currentText();
		int Index = pCombo->findText(Program);
		if (Index != -1)
			Program = pCombo->itemData(Index, Qt::UserRole).toString();

		pItem->setText(0, pAction->currentText());
		pItem->setData(0, Qt::UserRole, pAction->currentData());

		pItem->setText(1, (pNot->isChecked() ? "NOT " : "") + pCombo->currentText());
		pItem->setData(1, Qt::UserRole, (pNot->isChecked() ? "!" : "") + Program);

		pItem->setText(2, pPattern->text());
		
		m_CopyRulesChanged = true;
		OnOptChanged();
	}

	for (int i = 0; i < 3; i++)
		ui.treeCopy->setItemWidget(pItem, i, NULL);
}

void COptionsWindow::OnAddCopyRule()
{
	ParseAndAddCopyRule("", eCopyAlways);
	
	m_CopyRulesChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnDelCopyRule()
{
	QTreeWidgetItem* pItem = ui.treeCopy->currentItem();
	if (!pItem || pItem->data(0, Qt::UserRole).toInt() == -1)
		return;

	delete pItem;

	m_CopyRulesChanged = true;
	OnOptChanged();
}
//

void COptionsWindow::OnGeneralChanged()
{
	ui.lblCopyLimit->setEnabled(ui.chkCopyLimit->isChecked());
	ui.txtCopyLimit->setEnabled(ui.chkCopyLimit->isChecked());
	ui.lblCopyLimit->setText(tr("kilobytes (%1)").arg(FormatSize(ui.txtCopyLimit->text().toULongLong() * 1024)));
	ui.chkCopyPrompt->setEnabled(ui.chkCopyLimit->isChecked());
	ui.chkNoCopyWarn->setEnabled(ui.chkCopyLimit->isChecked() && !ui.chkCopyPrompt->isChecked());

	ui.chkAutoEmpty->setEnabled(ui.chkProtectBox->checkState() != Qt::Checked);

	ui.chkOpenSpooler->setEnabled(!ui.chkBlockSpooler->isChecked() && !ui.chkNoSecurityIsolation->isChecked());
	ui.chkPrintToFile->setEnabled(!ui.chkBlockSpooler->isChecked() && !ui.chkNoSecurityFiltering->isChecked());

	ui.chkCoverBar->setEnabled(ui.chkUserOperation->isChecked());

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
		ui.chkDropRights->setEnabled(!ui.chkSecurityMode->isChecked() /*&& !ui.chkNoSecurityIsolation->isChecked() && !theAPI->IsRunningAsAdmin()*/);
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
		theGUI->CheckCertificate(this, 0);

	UpdateBoxSecurity();

	if (sender() == ui.chkSecurityMode && !ui.chkSecurityMode->isChecked()) {
		ui.chkLockDown->setChecked(m_pBox->GetBool("SysCallLockDown", false));
		ui.chkRestrictDevices->setChecked(m_pBox->GetBool("RestrictDevices", false));
		
		ui.chkDropRights->setChecked(m_pBox->GetBool("DropAdminRights", false));
	}

	m_GeneralChanged = true;
	OnOptChanged();

	OnAccessChangedEx(); // for rule specificity
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
		auto pBoxEx = m_pBox.objectCast<CSandBoxPlus>();
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

	QString Name = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter a menu title"), QLineEdit::Normal, Split2(Value, "\\", true).second);
	if (Name.isEmpty())
		return;

	auto pBoxEx = m_pBox.objectCast<CSandBoxPlus>();
	
	QVariantMap Entry;
	Entry["Name"] = Name;
	Entry["Command"] = (pBoxEx ? pBoxEx->MakeBoxCommand(Value) : Value);
	AddRunItem(ui.treeRun, Entry);

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

	QVariantMap Entry;
	Entry["Name"] = Name;
	Entry["Command"] = Value;
	AddRunItem(ui.treeRun, Entry);

	OnRunChanged();
}

void COptionsWindow::OnCommandUp()
{
	int index = ui.treeRun->indexOfTopLevelItem(ui.treeRun->currentItem());
	if (index > 0) {
		QTreeWidgetItem* pItem = ui.treeRun->takeTopLevelItem(index);
		ui.treeRun->insertTopLevelItem(index - 1, pItem);
		ui.treeRun->setCurrentItem(pItem);
		OnRunChanged();
	}
}

void COptionsWindow::OnCommandDown()
{
	int index = ui.treeRun->indexOfTopLevelItem(ui.treeRun->currentItem());
	if (index < ui.treeRun->topLevelItemCount()-1) {
		QTreeWidgetItem* pItem = ui.treeRun->takeTopLevelItem(index);
		ui.treeRun->insertTopLevelItem(index + 1, pItem);
		ui.treeRun->setCurrentItem(pItem);
		OnRunChanged();
	}
}

void COptionsWindow::OnDelCommand()
{
	QTreeWidgetItem* pItem = ui.treeRun->currentItem();
	if (!pItem)
		return;

	delete pItem;
	OnRunChanged();
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
	OnAdvancedChanged();
}

void COptionsWindow::OnDiskChanged()
{
	if (sender() == ui.chkEncrypt) {
		if (ui.chkEncrypt->isChecked())
			theGUI->CheckCertificate(this, 1);
	}

	if (ui.chkRamBox->isChecked()) {
		ui.chkEncrypt->setEnabled(false);
		ui.chkEncrypt->setChecked(false);
		ui.btnPassword->setEnabled(false);
		ui.chkForceProtection->setEnabled(false);
	}
	else {
		ui.chkEncrypt->setEnabled(true);
		auto pBoxEx = m_pBox.objectCast<CSandBoxPlus>();
		ui.btnPassword->setEnabled(ui.chkEncrypt->isChecked() && pBoxEx && pBoxEx->GetMountRoot().isEmpty());
		ui.chkForceProtection->setEnabled(ui.chkEncrypt->isChecked());
	}
	
	ui.chkForceProtection->setEnabled(ui.chkEncrypt->isEnabled() && ui.chkEncrypt->isChecked());

	OnGeneralChanged();
}

bool COptionsWindow::RunImBox(const QStringList& Arguments)
{
	QProcess Process;
	Process.start(theAPI->GetSbiePath() + "\\ImBox.exe", Arguments);
	Process.waitForFinished();
	int ret = Process.exitCode();
	if (ret != ERR_OK) {
		QString Message;
		switch (ret) {
		case ERR_FILE_NOT_OPENED:	Message = tr("The image file does not exist"); break;
		case ERR_WRONG_PASSWORD:    Message = tr("The password is wrong"); break;
		default:                    Message = tr("Unexpected error: %1").arg(ret); break;
		}
		QMessageBox::critical(this, "Sandboxie-Plus", Message);
		return false;
	}
	return true;
}

void COptionsWindow::OnSetPassword()
{
	auto pBoxEx = m_pBox.objectCast<CSandBoxPlus>();
	bool bNew = !QFile::exists(pBoxEx->GetBoxImagePath());
	CBoxImageWindow window(bNew ? CBoxImageWindow::eNew : CBoxImageWindow::eChange, this);
	if (bNew) window.SetImageSize(m_ImageSize);
	if (theGUI->SafeExec(&window) == 1) {
		m_Password = window.GetPassword();
		if (bNew) {
			m_ImageSize = window.GetImageSize();
			OnGeneralChanged();
		}
		else {

			QStringList Arguments;
			Arguments.append("type=image");
			Arguments.append("image=" + pBoxEx->GetBoxImagePath());
			Arguments.append("key=" + m_Password);
			Arguments.append("new_key=" + window.GetNewPassword());

			if (RunImBox(Arguments))
				QMessageBox::information(this, "Sandboxie-Plus", tr("Image Password Changed"));
		}
	}
}

void COptionsWindow::OnBackupHeader()
{
	auto pBoxEx = m_pBox.objectCast<CSandBoxPlus>();

	QString FileName = QFileDialog::getSaveFileName(theGUI, tr("Backup Image Header for %1").arg(m_pBox->GetName()), "", QString("Image Header File (*.hdr)")).replace("/", "\\");

	QStringList Arguments;
	Arguments.append("type=image");
	Arguments.append("image=" + pBoxEx->GetBoxImagePath());
	Arguments.append("backup=" + FileName);

	if (RunImBox(Arguments))
		QMessageBox::information(this, "Sandboxie-Plus", tr("Image Header Backuped"));
}

void COptionsWindow::OnRestoreHeader()
{
	auto pBoxEx = m_pBox.objectCast<CSandBoxPlus>();

	QString FileName = QFileDialog::getOpenFileName(theGUI, tr("Restore Image Header for %1").arg(m_pBox->GetName()), "", QString("Image Header File (*.hdr)")).replace("/", "\\");

	QStringList Arguments;
	Arguments.append("type=image");
	Arguments.append("image=" + pBoxEx->GetBoxImagePath());
	Arguments.append("restore=" + FileName);

	if (RunImBox(Arguments))
		QMessageBox::information(this, "Sandboxie-Plus", tr("Image Header Restored"));
}
