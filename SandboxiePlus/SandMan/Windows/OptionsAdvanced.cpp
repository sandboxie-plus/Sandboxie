#include "stdafx.h"
#include "OptionsWindow.h"
#include "SandMan.h"
#include "SettingsWindow.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/Common.h"
#include "../MiscHelpers/Common/ComboInputDialog.h"
#include "../MiscHelpers/Common/SettingsWidgets.h"
#include "../AddonManager.h"
#include "Helpers/WinAdmin.h"

void COptionsWindow::CreateAdvanced()
{
	connect(ui.chkNoPanic, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));

	connect(ui.chkPreferExternalManifest, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkElevateCreateProcessFix, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkNoWindowRename, SIGNAL(clicked(bool)), this, SLOT(OnNoWindowRename()));
	connect(ui.chkNestedJobs, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));

	connect(ui.txtSingleMemory, SIGNAL(textChanged(const QString&)), this, SLOT(OnAdvancedChanged()));
	connect(ui.txtTotalMemory, SIGNAL(textChanged(const QString&)), this, SLOT(OnAdvancedChanged()));
	connect(ui.txtTotalNumber, SIGNAL(textChanged(const QString&)), this, SLOT(OnAdvancedChanged()));

	connect(ui.chkUseSbieDeskHack, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkUseSbieWndStation, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));

	connect(ui.chkAddToJob, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkProtectSCM, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkRestrictServices, SIGNAL(clicked(bool)), this, SLOT(OnSysSvcChanged()));
	connect(ui.chkElevateRpcss, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkProtectSystem, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkDropPrivileges, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkDropConHostIntegrity, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));

	//Do not force untrusted integrity level on the sanboxed token (reduces desktop isolation)
	//connect(ui.chkNotUntrusted, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));

	connect(ui.chkOpenCOM, SIGNAL(clicked(bool)), this, SLOT(OnOpenCOM()));
	connect(ui.chkComTimeout, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));

	connect(ui.chkForceRestart, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));

	connect(ui.chkNoSecurityIsolation, SIGNAL(clicked(bool)), this, SLOT(OnIsolationChanged()));
	connect(ui.chkNoSecurityFiltering, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));

#ifdef INSIDER_BUILD
	connect(ui.chkSbieDesktop, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
#else
	ui.chkSbieDesktop->setVisible(false);
#endif
	connect(ui.chkOpenWndStation, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));

	connect(ui.chkOpenDevCMApi, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	//connect(ui.chkOpenLsaSSPI, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkOpenSamEndpoint, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkOpenLsaEndpoint, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkOpenWpadEndpoint, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));

	connect(ui.chkSbieLogon, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkCreateToken, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));

	m_AdvOptions.insert("UseWin32kHooks",				SAdvOption{eSpec, QStringList() << "y" << "n", tr("Enable the use of win32 hooks for selected processes. Note: You need to enable win32k syscall hook support globally first.")});
	m_AdvOptions.insert("EnableMiniDump",				SAdvOption{eSpec, QStringList() << "y" << "n", tr("Enable crash dump creation in the sandbox folder")});
	m_AdvOptions.insert("ApplyElevateCreateProcessFix", SAdvOption{eOnlySpec, QStringList() << "y" << "n", tr("Always use ElevateCreateProcess fix, as sometimes applied by the Program Compatibility Assistant.")});
	m_AdvOptions.insert("PreferExternalManifest",		SAdvOption{eOnlySpec, QStringList() << "y" << "n", tr("")});
	m_AdvOptions.insert("ExternalManifestHack",			SAdvOption{eSpec, QStringList() << "y" << "n", tr("Enable special inconsistent PreferExternalManifest behaviour, as needed for some Edge fixes")});
	m_AdvOptions.insert("RpcMgmtSetComTimeout",			SAdvOption{eSpec, QStringList() << "n" << "y", tr("Set RpcMgmtSetComTimeout usage for specific processes")});
	m_AdvOptions.insert("CopyBlockDenyWrite",			SAdvOption{eSpec, QStringList() << "y" << "n", tr("Makes a write open call to a file that won't be copied fail instead of turning it read-only.")});
	m_AdvOptions.insert("UseSbieDeskHack",				SAdvOption{eOnlySpec, QStringList() << "n" << "y", tr("")});
	m_AdvOptions.insert("UseSbieWndStation",			SAdvOption{eOnlySpec, QStringList() << "n" << "y", tr("")});
	m_AdvOptions.insert("FakeAdminRights",				SAdvOption{eOnlySpec, QStringList() << "y" << "n", tr("Make specified processes think they have admin permissions.")});
	m_AdvOptions.insert("WaitForDebugger",				SAdvOption{eList, QStringList(), tr("Force specified processes to wait for a debugger to attach.")});
	m_AdvOptions.insert("BoxNameTitle",					SAdvOption{eOnlySpec, QStringList() << "y" << "n" << "-", tr("")});
	m_AdvOptions.insert("FileRootPath",					SAdvOption{eNoSpec, QStringList(), tr("Sandbox file system root")});
	m_AdvOptions.insert("KeyRootPath",					SAdvOption{eNoSpec, QStringList(), tr("Sandbox registry root")});
	m_AdvOptions.insert("IpcRootPath",					SAdvOption{eNoSpec, QStringList(), tr("Sandbox ipc root")});

		

	connect(ui.btnAddOption, SIGNAL(clicked(bool)), this, SLOT(OnAddOption()));
	connect(ui.chkShowOptionsTmpl, SIGNAL(clicked(bool)), this, SLOT(OnShowOptionTmpl()));
	connect(ui.btnDelOption, SIGNAL(clicked(bool)), this, SLOT(OnDelOption()));

	//connect(ui.treeOptions, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(OnOptionItemClicked(QTreeWidgetItem*, int)));
	connect(ui.treeOptions, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnOptionItemDoubleClicked(QTreeWidgetItem*, int)));
	connect(ui.treeOptions, SIGNAL(itemSelectionChanged()), this, SLOT(OnOptionSelectionChanged()));
	connect(ui.treeOptions, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(OnOptionChanged(QTreeWidgetItem *, int)));


	connect(ui.chkDisableMonitor, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));

	connect(ui.chkCallTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkFileTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkPipeTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkKeyTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkIpcTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkGuiTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkComTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkNetFwTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkDnsTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkApiTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkHookTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkDbgTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkErrTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));

	connect(ui.treeTriggers, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(OnTriggerChanged()));
	connect(ui.btnAddAutoRun, SIGNAL(clicked(bool)), this, SLOT(OnAddAutoRun()));
	connect(ui.btnAddAutoSvc, SIGNAL(clicked(bool)), this, SLOT(OnAddAutoSvc()));
	connect(ui.btnAddAutoExec, SIGNAL(clicked(bool)), this, SLOT(OnAddAutoExec()));
	connect(ui.btnAddRecoveryCmd, SIGNAL(clicked(bool)), this, SLOT(OnAddRecoveryCheck()));
	connect(ui.btnAddDeleteCmd, SIGNAL(clicked(bool)), this, SLOT(OnAddDeleteCmd()));
	connect(ui.btnAddTerminateCmd, SIGNAL(clicked(bool)), this, SLOT(OnAddTerminateCmd()));
	connect(ui.btnDelAuto, SIGNAL(clicked(bool)), this, SLOT(OnDelAuto()));
	connect(ui.chkShowTriggersTmpl, SIGNAL(clicked(bool)), this, SLOT(OnShowTriggersTmpl()));

	InitLangID();

	connect(ui.chkHideFirmware, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkHideUID, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkHideSerial, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkHideMac, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.cmbLangID, SIGNAL(currentIndexChanged(int)), this, SLOT(OnAdvancedChanged()));
	connect(ui.btnDumpFW, SIGNAL(clicked(bool)), this, SLOT(OnDumpFW()));

	connect(ui.chkHideOtherBoxes, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkHideNonSystemProcesses, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.btnAddProcess, SIGNAL(clicked(bool)), this, SLOT(OnAddProcess()));
	connect(ui.btnDelProcess, SIGNAL(clicked(bool)), this, SLOT(OnDelProcess()));
	connect(ui.chkShowHiddenProcTmpl, SIGNAL(clicked(bool)), this, SLOT(OnShowHiddenProcTmpl()));

	connect(ui.btnHostProcessAllow, SIGNAL(clicked(bool)), this, SLOT(OnHostProcessAllow()));
	connect(ui.btnHostProcessDeny, SIGNAL(clicked(bool)), this, SLOT(OnHostProcessDeny()));
	connect(ui.btnDelHostProcess, SIGNAL(clicked(bool)), this, SLOT(OnDelHostProcess()));
	connect(ui.chkShowHostProcTmpl, SIGNAL(clicked(bool)), this, SLOT(OnShowHostProcTmpl()));
	connect(ui.chkConfidential, SIGNAL(clicked(bool)), this, SLOT(OnConfidentialChanged()));
	connect(ui.chkLessConfidential, SIGNAL(clicked(bool)), this, SLOT(OnLessConfidentialChanged()));
	connect(ui.chkProtectWindow, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkAdminOnly, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkBlockCapture, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkNotifyProtect, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));

	connect(ui.treeInjectDll, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(OnToggleInjectDll(QTreeWidgetItem *, int)));
	connect(ui.treeInjectDll, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnDblClickInjedtDll(QTreeWidgetItem*, int)));

	connect(ui.chkHostProtect, SIGNAL(clicked(bool)), this, SLOT(OnHostProtectChanged()));
	connect(ui.chkHostProtectMsg, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));

	connect(ui.btnAddUser, SIGNAL(clicked(bool)), this, SLOT(OnAddUser()));
	connect(ui.btnDelUser, SIGNAL(clicked(bool)), this, SLOT(OnDelUser()));
	connect(ui.chkMonitorAdminOnly, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
}


void COptionsWindow::LoadAdvanced()
{
	ui.chkNoPanic->setChecked(m_pBox->GetBool("ExcludeFromTerminateAll", false));

	ui.chkPreferExternalManifest->setChecked(m_pBox->GetBool("PreferExternalManifest", false));
	ui.chkElevateCreateProcessFix->setChecked(m_pBox->GetBool("ApplyElevateCreateProcessFix", false));

	ui.chkAddToJob->setChecked(!m_pBox->GetBool("NoAddProcessToJob", false));
	ui.chkNestedJobs->setChecked(m_pBox->GetBool("AllowBoxedJobs", false));

	qint64 iSingleMemory = m_pBox->GetNum64("ProcessMemoryLimit", 0);
	if (iSingleMemory > 0x0LL && iSingleMemory <= 0x7FFFFFFFFFFFFFFFLL)
		ui.txtSingleMemory->setText(QString::number(iSingleMemory));

	qint64 iTotalMemory = m_pBox->GetNum64("TotalMemoryLimit", 0);
	if (iTotalMemory > 0x0LL && iTotalMemory <= 0x7FFFFFFFFFFFFFFFLL)
		ui.txtTotalMemory->setText(QString::number(iTotalMemory));

	qint64 iTotalNumber = m_pBox->GetNum64("ProcessNumberLimit", 0);
	if (iTotalNumber > 0x0LL && iTotalNumber <= 0xFFFFFFFFLL)
		ui.txtTotalNumber->setText(QString::number(iTotalNumber));

	ui.chkUseSbieDeskHack->setChecked(m_pBox->GetBool("UseSbieDeskHack", true));
	ui.chkUseSbieWndStation->setChecked(m_pBox->GetBool("UseSbieWndStation", true));

	ui.chkProtectSCM->setChecked(!m_pBox->GetBool("UnrestrictedSCM", false));
	ui.chkRestrictServices->setChecked(!m_pBox->GetBool("RunServicesAsSystem", false));
	ui.chkElevateRpcss->setChecked(m_pBox->GetBool("RunRpcssAsSystem", false));
	ui.chkProtectSystem->setChecked(!m_pBox->GetBool("ExposeBoxedSystem", false));
	ui.chkDropPrivileges->setChecked(m_pBox->GetBool("StripSystemPrivileges", true));
	ui.chkDropConHostIntegrity->setChecked(m_pBox->GetBool("DropConHostIntegrity", false));


	//ui.chkNotUntrusted->setChecked(m_pBox->GetBool("NoUntrustedToken", false));

	ui.chkForceRestart->setChecked(m_pBox->GetBool("ForceRestartAll", false));

	CheckOpenCOM();
	ui.chkComTimeout->setChecked(!m_pBox->GetBool("RpcMgmtSetComTimeout", true));

	ui.chkNoSecurityIsolation->setChecked(m_pBox->GetBool("NoSecurityIsolation", false));
	ui.chkNoSecurityFiltering->setChecked(m_pBox->GetBool("NoSecurityFiltering", false));

#ifdef INSIDER_BUILD
	ui.chkSbieDesktop->setChecked(m_pBox->GetBool("UseSandboxDesktop", false));
#endif
	ui.chkOpenWndStation->setChecked(m_pBox->GetBool("OpenWndStation", false));

	ui.chkOpenDevCMApi->setChecked(m_pBox->GetBool("OpenDevCMApi", false));
	//ui.chkOpenLsaSSPI->setChecked(!m_pBox->GetBool("BlockPassword", true)); // OpenLsaSSPI
	ui.chkOpenSamEndpoint->setChecked(m_pBox->GetBool("OpenSamEndpoint", false));
	ui.chkOpenLsaEndpoint->setChecked(m_pBox->GetBool("OpenLsaEndpoint", false));
	ui.chkOpenWpadEndpoint->setChecked(m_pBox->GetBool("OpenWPADEndpoint", false));

	ui.treeInjectDll->clear();
	QStringList InjectDll = m_pBox->GetTextList("InjectDll", false);
	QStringList InjectDll64 = m_pBox->GetTextList("InjectDll64", false);
#ifdef _M_ARM64
	QStringList InjectDllARM64 = m_pBox->GetTextList("InjectDllARM64", false);
#endif
	foreach(const CAddonInfoPtr pAddon, theGUI->GetAddonManager()->GetAddons()) {
		if (!pAddon->Installed)
			continue;
		QVariantMap InjectDlls = pAddon->Data["injectDlls"].toMap();
		if (!InjectDlls.isEmpty()) 
		{
			int Found = 0;
			int Count = 0;
			foreach(const QString & Key, InjectDlls.keys()) {
				QStringList List;
				if (Key == "x64")		List = InjectDll64;
				else if (Key == "x86")	List = InjectDll;
#ifdef _M_ARM64
				else if (Key == "a64")	List = InjectDllARM64;
#endif
				else
					continue;
				Count++;
				foreach(const QString & DllPath, List) {
					if (DllPath.endsWith(InjectDlls[Key].toString(), Qt::CaseInsensitive)) {
						Found++;
						break;
					}
				}
			}

			QTreeWidgetItem* pItem = new QTreeWidgetItem();
			pItem->setData(0, Qt::UserRole, pAddon->Id);
			pItem->setText(0, pAddon->GetLocalizedEntry("name"));
			if (Found == Count) {
				pItem->setCheckState(0, Qt::Checked);
				pItem->setData(0, Qt::UserRole + 1, Qt::Checked);
			} else if (Found > 0) {
				pItem->setCheckState(0, Qt::PartiallyChecked);
				pItem->setData(0, Qt::UserRole + 1, Qt::PartiallyChecked);
			}
			else {
				pItem->setCheckState(0, Qt::Unchecked);
				pItem->setData(0, Qt::UserRole + 1, Qt::Unchecked);
			}
			pItem->setText(1, pAddon->GetLocalizedEntry("description"));
			ui.treeInjectDll->addTopLevelItem(pItem);
		}
	}

	
	ui.chkHostProtect->setChecked(m_pBox->GetBool("ProtectHostImages", false));
	ui.chkHostProtectMsg->setEnabled(ui.chkHostProtect->isChecked());
	ui.chkHostProtectMsg->setChecked(m_pBox->GetBool("NotifyImageLoadDenied", true));

	LoadOptionList();

	bool bGlobalNoMon = m_pBox->GetAPI()->GetGlobalSettings()->GetBool("DisableResourceMonitor", false);
	ui.chkDisableMonitor->setChecked(m_pBox->GetBool("DisableResourceMonitor", bGlobalNoMon));
	ReadAdvancedCheck("CallTrace", ui.chkCallTrace, "*");
	ReadAdvancedCheck("FileTrace", ui.chkFileTrace, "*");
	ReadAdvancedCheck("PipeTrace", ui.chkPipeTrace, "*");
	ReadAdvancedCheck("KeyTrace", ui.chkKeyTrace, "*");
	ReadAdvancedCheck("IpcTrace", ui.chkIpcTrace, "*");
	ReadAdvancedCheck("GuiTrace", ui.chkGuiTrace, "*");
	ReadAdvancedCheck("ClsidTrace", ui.chkComTrace, "*");
	ReadAdvancedCheck("NetFwTrace", ui.chkNetFwTrace, "*");
	ui.chkDnsTrace->setChecked(m_pBox->GetBool("DnsTrace", false));
	ui.chkApiTrace->setChecked(m_pBox->GetBool("ApiTrace", false));
	ui.chkHookTrace->setChecked(m_pBox->GetBool("HookTrace", false));
	ui.chkDbgTrace->setChecked(m_pBox->GetBool("DebugTrace", false));
	ui.chkErrTrace->setChecked(m_pBox->GetBool("ErrorTrace", false));

	// triggers
	ui.treeTriggers->clear();
	foreach(const QString & Value, m_pBox->GetTextList("StartProgram", m_Template))
		AddTriggerItem(Value, eOnStartCmd);
	foreach(const QString & Value, m_pBox->GetTextList("StartService", m_Template))
		AddTriggerItem(Value, eOnStartSvc);
	foreach(const QString & Value, m_pBox->GetTextList("AutoExec", m_Template))
		AddTriggerItem(Value, eAutoExec);
	foreach(const QString & Value, m_pBox->GetTextList("OnFileRecovery", m_Template))
		AddTriggerItem(Value, eRecoveryCheck);
	foreach(const QString & Value, m_pBox->GetTextList("OnBoxDelete", m_Template))
		AddTriggerItem(Value, eDeleteCmd);
	foreach(const QString & Value, m_pBox->GetTextList("OnBoxTerminate", m_Template))
		AddTriggerItem(Value, eTerminateCmd);

	foreach(const QString & Value, m_pBox->GetTextList("StartProgramDisabled", m_Template))
		AddTriggerItem(Value, eOnStartCmd, true);
	foreach(const QString & Value, m_pBox->GetTextList("StartServiceDisabled", m_Template))
		AddTriggerItem(Value, eOnStartSvc, true);
	foreach(const QString & Value, m_pBox->GetTextList("AutoExecDisabled", m_Template))
		AddTriggerItem(Value, eAutoExec, true);
	foreach(const QString & Value, m_pBox->GetTextList("OnFileRecoveryDisabled", m_Template))
		AddTriggerItem(Value, eRecoveryCheck, true);
	foreach(const QString & Value, m_pBox->GetTextList("OnBoxDeleteDisabled", m_Template))
		AddTriggerItem(Value, eDeleteCmd, true);
	foreach(const QString & Value, m_pBox->GetTextList("OnBoxTerminateDisabled", m_Template))
		AddTriggerItem(Value, eTerminateCmd, true);

	ShowTriggersTmpl();
	//

	ui.chkHideFirmware->setChecked(m_pBox->GetBool("HideFirmwareInfo", false));
	ui.chkHideUID->setChecked(m_pBox->GetBool("RandomRegUID",false));
	ui.chkHideSerial->setChecked(m_pBox->GetBool("HideDiskSerialNumber", false));
	ui.chkHideMac->setChecked(m_pBox->GetBool("HideNetworkAdapterMAC", false));

	ui.cmbLangID->setCurrentIndex(ui.cmbLangID->findData(m_pBox->GetNum("CustomLCID", 0)));

	ui.chkHideOtherBoxes->setChecked(m_pBox->GetBool("HideOtherBoxes", true));
	ui.chkHideNonSystemProcesses->setChecked(m_pBox->GetBool("HideNonSystemProcesses", false));
	
	ui.treeHideProc->clear();
	foreach(const QString& Value, m_pBox->GetTextList("HideHostProcess", m_Template))
		AddHiddenProcEntry(Value);
	ShowHiddenProcTmpl();

	ui.treeHostProc->clear();
	foreach(const QString & Value, m_pBox->GetTextList("DenyHostAccess", m_Template)) {
		StrPair NameVal = Split2(Value, ",");
		if (NameVal.second.isEmpty()) {
			NameVal.second = NameVal.first;
			NameVal.first = "*";
		}
		AddHostProcEntry(NameVal.first, NameVal.second.left(1).toLower() == "y");
	}
	ShowHostProcTmpl();

	ui.chkConfidential->setChecked(m_pBox->GetBool("ConfidentialBox", false));
	ui.chkLessConfidential->setEnabled(ui.chkConfidential->isChecked());
	ui.chkLessConfidential->setChecked(m_BoxTemplates.contains("LessConfidentialBox"));
	ui.chkNotifyProtect->setChecked(m_pBox->GetBool("NotifyBoxProtected", false));

	ui.chkProtectWindow->setChecked(m_pBox->GetBool("CoverBoxedWindows"));
	QString str = m_pBox->GetText("OpenWinClass", "");
	ui.chkBlockCapture->setChecked(m_pBox->GetBool("BlockScreenCapture") && QString::compare(str, "*") != 0);
	ui.chkBlockCapture->setCheckable(QString::compare(str, "*") != 0);
	
	ui.chkAdminOnly->setChecked(m_pBox->GetBool("EditAdminOnly", false));

	QStringList Users = m_pBox->GetText("Enabled").split(",");
	ui.lstUsers->clear();
	if (Users.count() > 1)
		ui.lstUsers->addItems(Users.mid(1));
	ui.chkMonitorAdminOnly->setChecked(m_pBox->GetBool("MonitorAdminOnly", false));

	UpdateBoxIsolation();
	OnSysSvcChanged();

	ui.chkOpenCredentials->setEnabled(!ui.chkOpenProtectedStorage->isChecked());
	if (!ui.chkOpenCredentials->isEnabled()) ui.chkOpenCredentials->setChecked(true);

	m_AdvancedChanged = false;

	UpdateJobOptions();
}

void COptionsWindow::OnPSTChanged()
{
	if(!ui.chkOpenProtectedStorage->isChecked())
		ui.chkOpenCredentials->setChecked(m_pBox->GetBool("OpenCredentials", false));

	OnGeneralChanged();
}

void COptionsWindow::ShowTriggersTmpl(bool bUpdate)
{
	if (ui.chkShowTriggersTmpl->isChecked())
	{
		foreach(const QString& Template, m_pBox->GetTemplates())
		{
			foreach(const QString & Value, m_pBox->GetTextListTmpl("StartProgram", Template))
				AddTriggerItem(Value, eOnStartCmd, false, Template);
			foreach(const QString & Value, m_pBox->GetTextListTmpl("StartService", Template))
				AddTriggerItem(Value, eOnStartSvc, false, Template);
			foreach(const QString & Value, m_pBox->GetTextListTmpl("AutoExec", Template))
				AddTriggerItem(Value, eAutoExec, false, Template);
			foreach(const QString & Value, m_pBox->GetTextListTmpl("OnFileRecovery", Template))
				AddTriggerItem(Value, eRecoveryCheck, false, Template);
			foreach(const QString & Value, m_pBox->GetTextListTmpl("OnBoxDelete", Template))
				AddTriggerItem(Value, eDeleteCmd, false, Template);
			foreach(const QString & Value, m_pBox->GetTextListTmpl("OnBoxTerminate", Template))
				AddTriggerItem(Value, eTerminateCmd, false, Template);
		}
	}
	else if (bUpdate)
	{
		for (int i = 0; i < ui.treeTriggers->topLevelItemCount(); )
		{
			QTreeWidgetItem* pItem = ui.treeTriggers->topLevelItem(i);
			int Type = pItem->data(0, Qt::UserRole).toInt();
			if (Type == -1) {
				delete pItem;
				continue; // entry from template
			}
			i++;
		}
	}
}

void COptionsWindow::SaveAdvanced()
{
	WriteAdvancedCheck(ui.chkNoPanic, "ExcludeFromTerminateAll", "y", "");

	WriteAdvancedCheck(ui.chkPreferExternalManifest, "PreferExternalManifest", "y", "");
	WriteAdvancedCheck(ui.chkElevateCreateProcessFix, "ApplyElevateCreateProcessFix", "y", "");

	WriteAdvancedCheck(ui.chkUseSbieDeskHack, "UseSbieDeskHack", "", "n");
	WriteAdvancedCheck(ui.chkUseSbieWndStation, "UseSbieWndStation", "", "n");

	WriteAdvancedCheck(ui.chkAddToJob, "NoAddProcessToJob", "", "y");
	WriteAdvancedCheck(ui.chkProtectSCM, "UnrestrictedSCM", "", "y");
	WriteAdvancedCheck(ui.chkNestedJobs, "AllowBoxedJobs", "y", "");

	qint64 iSingleMemory = !ui.txtSingleMemory->text().isEmpty() ? ui.txtSingleMemory->text().toLongLong() : -1;
	if (iSingleMemory > 0x0LL && iSingleMemory <= 0x7FFFFFFFFFFFFFFFLL)
		WriteText("ProcessMemoryLimit", QString::number(iSingleMemory));
	else
		m_pBox->DelValue("ProcessMemoryLimit");

	qint64 iTotalMemory = !ui.txtTotalMemory->text().isEmpty() ? ui.txtTotalMemory->text().toLongLong() : -1;
	if (iTotalMemory > 0x0LL && iTotalMemory <= 0x7FFFFFFFFFFFFFFFLL)
		WriteText("TotalMemoryLimit", QString::number(iTotalMemory));
	else
		m_pBox->DelValue("TotalMemoryLimit");

	qint64 iTotalNumber = !ui.txtTotalNumber->text().isEmpty() ? ui.txtTotalNumber->text().toLongLong() : -1;
	if (iTotalNumber > 0x0LL && iTotalNumber <= 0xFFFFFFFFLL)
		WriteText("ProcessNumberLimit", QString::number(iTotalNumber));
	else
		m_pBox->DelValue("ProcessNumberLimit");

	WriteAdvancedCheck(ui.chkRestrictServices, "RunServicesAsSystem", "", "y");
	WriteAdvancedCheck(ui.chkElevateRpcss, "RunRpcssAsSystem", "y", "");
	WriteAdvancedCheck(ui.chkProtectSystem, "ExposeBoxedSystem", "", "y");
	WriteAdvancedCheck(ui.chkDropPrivileges, "StripSystemPrivileges", "", "n");
	WriteAdvancedCheck(ui.chkDropConHostIntegrity, "DropConHostIntegrity", "y", "");

	//WriteAdvancedCheck(ui.chkNotUntrusted, "NoUntrustedToken", "y", "");

	WriteAdvancedCheck(ui.chkComTimeout, "RpcMgmtSetComTimeout", "n", "");

	WriteAdvancedCheck(ui.chkForceRestart, "ForceRestartAll", "y", "");

	WriteAdvancedCheck(ui.chkNoSecurityIsolation, "NoSecurityIsolation", "y", "");
	WriteAdvancedCheck(ui.chkNoSecurityFiltering, "NoSecurityFiltering", "y", "");

#ifdef INSIDER_BUILD
	WriteAdvancedCheck(ui.chkSbieDesktop, "UseSandboxDesktop", "y", "");
#endif
	WriteAdvancedCheck(ui.chkOpenWndStation, "OpenWndStation", "y", "");

	WriteAdvancedCheck(ui.chkOpenDevCMApi, "OpenDevCMApi", "y", "");
	//WriteAdvancedCheck(ui.chkOpenLsaSSPI, "BlockPassword", "n", ""); // OpenLsaSSPI
	WriteAdvancedCheck(ui.chkOpenSamEndpoint, "OpenSamEndpoint", "y", "");
	WriteAdvancedCheck(ui.chkOpenLsaEndpoint, "OpenLsaEndpoint", "y", "");
	WriteAdvancedCheck(ui.chkOpenWpadEndpoint, "OpenWPADEndpoint", "y", "");

	QStringList InjectDll = m_pBox->GetTextList("InjectDll", false);
	QStringList InjectDll64 = m_pBox->GetTextList("InjectDll64", false);
#ifdef _M_ARM64
	QStringList InjectDllARM64 = m_pBox->GetTextList("InjectDllARM64", false);
#endif
	for (int i = 0; i < ui.treeInjectDll->topLevelItemCount(); i++) {
		QTreeWidgetItem* pItem = ui.treeInjectDll->topLevelItem(i);
		CAddonPtr pAddon = theGUI->GetAddonManager()->GetAddon(pItem->data(0, Qt::UserRole).toString());
		if (pAddon && pItem->checkState(0) != Qt::PartiallyChecked && pItem->checkState(0) != pItem->data(0, Qt::UserRole + 1))
		{
			QVariantMap InjectDlls = pAddon->Data["injectDlls"].toMap();
			foreach(const QString & Key, InjectDlls.keys()) {
				QStringList* pList;
				if (Key == "x64")		pList = &InjectDll64;
				else if (Key == "x86")	pList = &InjectDll;
#ifdef _M_ARM64
				else if (Key == "a64")	pList = &InjectDllARM64;
#endif
				else
					continue;

				// remove old entries
				for (int i = 0; i < pList->size(); i++) {
					if (pList->at(i).endsWith(InjectDlls[Key].toString(), Qt::CaseInsensitive))
						pList->removeAt(i--);
				}

				// add new entries
				if (pItem->checkState(0) == Qt::Checked)
					pList->append(pAddon->Data["installPath"].toString() + InjectDlls[Key].toString());
			}
		}
	}
	m_pBox->UpdateTextList("InjectDll", InjectDll, false);
	m_pBox->UpdateTextList("InjectDll64", InjectDll64, false);
#ifdef _M_ARM64
	m_pBox->UpdateTextList("InjectDllARM64", InjectDllARM64, false);
#endif

	WriteAdvancedCheck(ui.chkHostProtect, "ProtectHostImages", "y", "");
	WriteAdvancedCheck(ui.chkHostProtectMsg, "NotifyImageLoadDenied", "", "n");

	bool bGlobalSbieLogon = m_pBox->GetAPI()->GetGlobalSettings()->GetBool("SandboxieLogon", false);
	WriteAdvancedCheck(ui.chkSbieLogon, "SandboxieLogon", bGlobalSbieLogon ? "" : "y", bGlobalSbieLogon ? "n" : "");

	bool bGlobalSandboxGroup = m_pBox->GetAPI()->GetGlobalSettings()->GetBool("SandboxieAllGroup", false);
	bool bGlobalCreateToken = m_pBox->GetAPI()->GetGlobalSettings()->GetBool("UseCreateToken", false);
	if (ui.chkCreateToken->checkState() == Qt::Checked) {
		WriteAdvancedCheck(ui.chkCreateToken, "SandboxieAllGroup", bGlobalSandboxGroup ? "" : "y");
		m_pBox->DelValue("UseCreateToken");
	}
	else if (ui.chkCreateToken->checkState() == Qt::PartiallyChecked) {
		m_pBox->SetText("SandboxieAllGroup", "n");
		m_pBox->SetText("UseCreateToken", "y");
	}
	else {
		WriteAdvancedCheck(ui.chkCreateToken, "SandboxieAllGroup", bGlobalSandboxGroup ? "" : "y", bGlobalSandboxGroup ? "n" : "");
		WriteAdvancedCheck(ui.chkCreateToken, "UseCreateToken", bGlobalCreateToken ? "" : "y", bGlobalCreateToken ? "n" : "");
	}

	SaveOptionList();

	bool bGlobalNoMon = m_pBox->GetAPI()->GetGlobalSettings()->GetBool("DisableResourceMonitor", false);
	WriteAdvancedCheck(ui.chkDisableMonitor, "DisableResourceMonitor", bGlobalNoMon ? "" : "y", bGlobalNoMon ? "n" : "");
	WriteAdvancedCheck(ui.chkCallTrace, "CallTrace", "*");
	WriteAdvancedCheck(ui.chkFileTrace, "FileTrace", "*");
	WriteAdvancedCheck(ui.chkPipeTrace, "PipeTrace", "*");
	WriteAdvancedCheck(ui.chkKeyTrace, "KeyTrace", "*");
	WriteAdvancedCheck(ui.chkIpcTrace, "IpcTrace", "*");
	WriteAdvancedCheck(ui.chkGuiTrace, "GuiTrace", "*");
	WriteAdvancedCheck(ui.chkComTrace, "ClsidTrace", "*");
	WriteAdvancedCheck(ui.chkNetFwTrace, "NetFwTrace", "*");
	WriteAdvancedCheck(ui.chkDnsTrace, "DnsTrace", "y");
	WriteAdvancedCheck(ui.chkApiTrace, "ApiTrace", "y");
	WriteAdvancedCheck(ui.chkHookTrace, "HookTrace", "y");
	WriteAdvancedCheck(ui.chkDbgTrace, "DebugTrace", "y");
	WriteAdvancedCheck(ui.chkErrTrace, "ErrorTrace", "y");

	// triggers
	QStringList StartProgram;
	QStringList StartService;
	QStringList RecoveryCheck;
	QStringList DeleteCommand;
	QStringList AutoExec;
	QStringList TerminateCommand;

	QStringList StartProgramDisabled;
	QStringList StartServiceDisabled;
	QStringList RecoveryCheckDisabled;
	QStringList DeleteCommandDisabled;
	QStringList AutoExecDisabled;
	QStringList TerminateCommandDisabled;

	for (int i = 0; i < ui.treeTriggers->topLevelItemCount(); i++) {
		QTreeWidgetItem* pItem = ui.treeTriggers->topLevelItem(i);
		if (pItem->checkState(0) == Qt::Checked) 
		{
			switch (pItem->data(0, Qt::UserRole).toInt())
			{
			case eOnStartCmd:		StartProgram.append(pItem->text(2)); break;
			case eOnStartSvc:		StartService.append(pItem->text(2)); break;
			case eAutoExec:			AutoExec.append(pItem->text(2)); break;
			case eRecoveryCheck:	RecoveryCheck.append(pItem->text(2)); break;
			case eDeleteCmd:		DeleteCommand.append(pItem->text(2)); break;
			case eTerminateCmd:		TerminateCommand.append(pItem->text(2)); break;
			}
		}
		else
		{
			switch (pItem->data(0, Qt::UserRole).toInt())
			{
			case eOnStartCmd:		StartProgramDisabled.append(pItem->text(2)); break;
			case eOnStartSvc:		StartServiceDisabled.append(pItem->text(2)); break;
			case eAutoExec:			AutoExecDisabled.append(pItem->text(2)); break;
			case eRecoveryCheck:	RecoveryCheckDisabled.append(pItem->text(2)); break;
			case eDeleteCmd:		DeleteCommandDisabled.append(pItem->text(2)); break;
			case eTerminateCmd:		TerminateCommandDisabled.append(pItem->text(2)); break;
			}
		}
	}

	WriteTextList("StartProgram", StartProgram);
	WriteTextList("StartService", StartService);
	WriteTextList("AutoExec", AutoExec);
	WriteTextList("OnFileRecovery", RecoveryCheck);
	WriteTextList("OnBoxDelete", DeleteCommand);
	WriteTextList("OnBoxTerminate", TerminateCommand);

	WriteTextList("StartProgramDisabled", StartProgramDisabled);
	WriteTextList("StartServiceDisabled", StartServiceDisabled);
	WriteTextList("AutoExecDisabled", AutoExecDisabled);
	WriteTextList("OnFileRecoveryDisabled", RecoveryCheckDisabled);
	WriteTextList("OnBoxDeleteDisabled", DeleteCommandDisabled);
	WriteTextList("OnBoxTerminateDisabled", TerminateCommandDisabled);
	//

	WriteAdvancedCheck(ui.chkHideFirmware, "HideFirmwareInfo", "y", "");
	WriteAdvancedCheck(ui.chkHideUID, "RandomRegUID", "y", "");
	WriteAdvancedCheck(ui.chkHideSerial, "HideDiskSerialNumber", "y", "");
	WriteAdvancedCheck(ui.chkHideMac, "HideNetworkAdapterMAC", "y", "");

	int CustomLCID = ui.cmbLangID->currentData().toInt();
	if (CustomLCID) m_pBox->SetNum("CustomLCID", CustomLCID);
	else m_pBox->DelValue("CustomLCID");

	WriteAdvancedCheck(ui.chkHideOtherBoxes, "HideOtherBoxes", "", "n");
	WriteAdvancedCheck(ui.chkHideNonSystemProcesses, "HideNonSystemProcesses", "y", "");

	QStringList HideProcesses;
	for (int i = 0; i < ui.treeHideProc->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeHideProc->topLevelItem(i);
		int Type = pItem->data(0, Qt::UserRole).toInt();
		if (Type == -1)
			continue; // entry from template
		HideProcesses.append(pItem->text(0)); 
	}
	WriteTextList("HideHostProcess", HideProcesses);

	QStringList DenyHostProcesses;
	for (int i = 0; i < ui.treeHostProc->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeHostProc->topLevelItem(i);
		int Type = pItem->data(0, Qt::UserRole).toInt();
		if (Type == -1)
			continue; // entry from template
		DenyHostProcesses.append(pItem->text(0) + "," + (pItem->data(1, Qt::UserRole).toBool() ? "y" : "n")); 
	}
	WriteTextList("DenyHostAccess", DenyHostProcesses);

	WriteAdvancedCheck(ui.chkConfidential, "ConfidentialBox", "y", "");
	WriteAdvancedCheck(ui.chkNotifyProtect, "NotifyBoxProtected", "y", "");

	WriteAdvancedCheck(ui.chkProtectWindow, "CoverBoxedWindows", "y", "");
	WriteAdvancedCheck(ui.chkBlockCapture, "BlockScreenCapture", "y", "");

	WriteAdvancedCheck(ui.chkAdminOnly, "EditAdminOnly", "y", "");

	QStringList Users;
	for (int i = 0; i < ui.lstUsers->count(); i++)
		Users.append(ui.lstUsers->item(i)->text());
	WriteText("Enabled", Users.count() > 0 ? "y," + Users.join(",") : "y");
	WriteAdvancedCheck(ui.chkMonitorAdminOnly, "MonitorAdminOnly");

	m_AdvancedChanged = false;
}

void COptionsWindow::OnIsolationChanged()
{
	if (sender() == ui.chkNoSecurityIsolation) {
		// we can ignore chkNoSecurityFiltering as it requires chkNoSecurityIsolation
		if (ui.chkNoSecurityIsolation->isChecked())
			theGUI->CheckCertificate(this, 0);
	}

	UpdateBoxIsolation();

	if (sender() == ui.chkNoSecurityIsolation && !ui.chkNoSecurityIsolation->isChecked()) {
		ui.chkCloseForBox->setChecked(m_pBox->GetBool("AlwaysCloseForBoxed", true));
		ui.chkNoOpenForBox->setChecked(m_pBox->GetBool("DontOpenForBoxed", true));
	}

	OnAdvancedChanged();
}

void COptionsWindow::UpdateBoxIsolation()
{
	ui.chkNoSecurityFiltering->setEnabled(ui.chkNoSecurityIsolation->isChecked());

	//ui.chkNotUntrusted->setEnabled(!ui.chkNoSecurityIsolation->isChecked());

	UpdateJobOptions();

	ui.chkOpenDevCMApi->setEnabled(!ui.chkNoSecurityIsolation->isChecked());
	ui.chkOpenSamEndpoint->setEnabled(!ui.chkNoSecurityIsolation->isChecked());
	ui.chkOpenLsaEndpoint->setEnabled(!ui.chkNoSecurityIsolation->isChecked());
	ui.chkOpenWpadEndpoint->setEnabled(!ui.chkNoSecurityIsolation->isChecked());


	ui.chkRawDiskRead->setEnabled(!ui.chkNoSecurityIsolation->isChecked()); //  without isolation only user mode
	ui.chkRawDiskNotify->setEnabled(!ui.chkNoSecurityIsolation->isChecked());
	ui.chkAllowEfs->setEnabled(!ui.chkNoSecurityIsolation->isChecked());

	ui.chkBlockNetShare->setEnabled(!ui.chkNoSecurityFiltering->isChecked());

	ui.chkBlockSpooler->setEnabled(!ui.chkNoSecurityIsolation->isChecked());
	ui.chkOpenSpooler->setEnabled(!ui.chkBlockSpooler->isChecked() && !ui.chkNoSecurityIsolation->isChecked());
	ui.chkPrintToFile->setEnabled(!ui.chkBlockSpooler->isChecked() && !ui.chkNoSecurityFiltering->isChecked());

	ui.chkCloseClipBoard->setEnabled(!ui.chkNoSecurityIsolation->isChecked());
	ui.chkVmRead->setEnabled(!ui.chkNoSecurityIsolation->isChecked());

	//ui.chkBlockCapture->setEnabled(!ui.chkNoSecurityIsolation->isChecked());
	ui.chkProtectPower->setEnabled(!ui.chkNoSecurityIsolation->isChecked());

	ui.chkCloseForBox->setEnabled(!ui.chkNoSecurityIsolation->isChecked());
	ui.chkNoOpenForBox->setEnabled(!ui.chkNoSecurityIsolation->isChecked());

	ui.chkSbieLogon->setEnabled(!ui.chkNoSecurityIsolation->isChecked());
	ui.chkCreateToken->setEnabled(!ui.chkNoSecurityIsolation->isChecked());
	if (ui.chkNoSecurityIsolation->isChecked()) {
		ui.chkCloseForBox->setChecked(false);
		ui.chkNoOpenForBox->setChecked(false);
		ui.chkSbieLogon->setChecked(false);
		ui.chkCreateToken->setChecked(false);
	}
	else {
		ReadGlobalCheck(ui.chkSbieLogon, "SandboxieLogon", false);

		if (m_pBox->GetBool("SandboxieAllGroup", false, true))
			ui.chkCreateToken->setCheckState(Qt::Checked);
		else if (m_pBox->GetBool("UseCreateToken", false, true))
			ui.chkCreateToken->setCheckState(Qt::PartiallyChecked);
		else
			ui.chkCreateToken->setCheckState(Qt::Unchecked);
	}
}

void COptionsWindow::OnSysSvcChanged()
{
	ui.chkElevateRpcss->setDisabled(ui.chkNoSecurityIsolation->isChecked() && (!ui.chkRestrictServices->isChecked() || ui.chkMsiExemptions->isChecked()));
	OnAdvancedChanged();
}

void COptionsWindow::OnConfidentialChanged()
{
	ui.chkLessConfidential->setEnabled(ui.chkConfidential->isChecked());
	OnAdvancedChanged();
}

void COptionsWindow::OnLessConfidentialChanged()
{
	SetTemplate("LessConfidentialBox", ui.chkLessConfidential->isChecked());
}

void COptionsWindow::OnAdvancedChanged()
{
	UpdateJobOptions();

	m_AdvancedChanged = true;
	OnOptChanged();
}

void COptionsWindow::UpdateJobOptions()
{
	bool IsAllWndOpen = ui.chkNoSecurityIsolation->isChecked() || IsAccessEntrySet(eWnd, "", eOpen, "*");
	ui.chkAddToJob->setEnabled(!IsAllWndOpen);

	bool bUseJobObject = !IsAllWndOpen && ui.chkAddToJob->isChecked();
	ui.chkNestedJobs->setEnabled(bUseJobObject);

	qint64 iSingleMemory = ui.txtSingleMemory->text().toLongLong();
	if (!(iSingleMemory > 0x0LL && iSingleMemory <= 0x7FFFFFFFFFFFFFFFLL)) {
		ui.lblSingleMemory->setText(tr("bytes (unlimited)"));
	}
	else {
		ui.lblSingleMemory->setText(tr("bytes (%1)").arg(FormatSize(iSingleMemory)));
	}
	ui.txtSingleMemory->setEnabled(bUseJobObject);

	qint64 iTotalMemory = ui.txtTotalMemory->text().toLongLong();
	if (!(iTotalMemory > 0x0LL && iTotalMemory <= 0x7FFFFFFFFFFFFFFFLL)) {
		ui.lblTotalMemory->setText(tr("bytes (unlimited)"));
	}
	else {
		ui.lblTotalMemory->setText(tr("bytes (%1)").arg(FormatSize(iTotalMemory)));
	}
	ui.txtTotalMemory->setEnabled(bUseJobObject);

	qint64 iTotalNumber = ui.txtTotalNumber->text().toLongLong();
	if (!(iTotalNumber > 0x0LL && iTotalNumber <= 0xFFFFFFFFLL)) {
		ui.lblTotalNumber->setText(tr("unlimited"));
	}
	else {
		ui.lblTotalNumber->setText("");
	}
	ui.txtTotalNumber->setEnabled(bUseJobObject);
}

void COptionsWindow::CheckOpenCOM()
{
	bool bComIpcOpen = IsAccessEntrySet(eIPC, "", eOpen, "\\RPC Control\\epmapper") || IsAccessEntrySet(eIPC, "", eOpen, "*");
	if(bComIpcOpen)
		ui.chkOpenCOM->setChecked(!m_BoxTemplates.contains("BoxedCOM"));
	else
		ui.chkOpenCOM->setChecked(m_BoxTemplates.contains("OpenCOM"));
}

void COptionsWindow::OnOpenCOM()
{
	bool bComIpcOpen = IsAccessEntrySet(eIPC, "", eOpen, "\\RPC Control\\epmapper") || IsAccessEntrySet(eIPC, "", eOpen, "*");
	SetTemplate("OpenCOM", !bComIpcOpen && ui.chkOpenCOM->isChecked());
	SetTemplate("BoxedCOM", bComIpcOpen && !ui.chkOpenCOM->isChecked());
}

void COptionsWindow::OnNoWindowRename()
{
	if (ui.chkNoWindowRename->isChecked())
		SetAccessEntry(eWnd, "", eOpen, "#");
	else
		DelAccessEntry(eWnd, "", eOpen, "#");
}

void COptionsWindow::OnToggleInjectDll(QTreeWidgetItem* pItem, int Column)
{
	OnAdvancedChanged();
}

void COptionsWindow::OnDblClickInjedtDll(QTreeWidgetItem* pItem, int Column)
{
	CAddonPtr pAddon = theGUI->GetAddonManager()->GetAddon(pItem->data(0, Qt::UserRole).toString());
	if (!pAddon || pAddon->Data["configFile"].toString().isEmpty())
		return;

	theGUI->EditIni(theAPI->GetSbiePath() + pAddon->Data["installPath"].toString() + pAddon->Data["configFile"].toString());
}

void COptionsWindow::OnHostProtectChanged()
{
	ui.chkHostProtectMsg->setEnabled(ui.chkHostProtect->isChecked());
	OnAdvancedChanged();
}

// options
void COptionsWindow::LoadOptionList()
{
	if (!ui.treeOptions) return;

	ui.treeOptions->clear();
	foreach(const QString& Name, m_AdvOptions.keys()) {
		foreach(const QString & Value, m_pBox->GetTextList(Name, m_Template)) {
			QStringList Values = Value.split(",");
			if (Values.count() >= 2) 
				AddOptionEntry(Name, Values[0], Values[1]);
			else if(m_AdvOptions[Name].ProcSpec == eList)
				AddOptionEntry(Name, Values[0], "");
			else if(m_AdvOptions[Name].ProcSpec != eOnlySpec) // eOnlySpec shows only process specific entries, no global once
				AddOptionEntry(Name, "", Values[0]);
		}
	}

	LoadOptionListTmpl();
}

void COptionsWindow::LoadOptionListTmpl(bool bUpdate)
{
	if (ui.chkShowOptionsTmpl->isChecked())
	{
		foreach(const QString& Template, m_pBox->GetTemplates())
		{
			foreach(const QString& Name, m_AdvOptions.keys()) {
				foreach(const QString & Value, m_pBox->GetTextListTmpl(Name, Template)) {
					QStringList Values = Value.split(",");
					if (Values.count() >= 2) 
						AddOptionEntry(Name, Values[0], Values[1], Template);
					else // all programs
						AddOptionEntry(Name, "", Values[0], Template);
				}
			}
		}
	}
	else if (bUpdate)
	{
		for (int i = 0; i < ui.treeOptions->topLevelItemCount(); )
		{
			QTreeWidgetItem* pItem = ui.treeOptions->topLevelItem(i);
			QString Name = pItem->data(0, Qt::UserRole).toString();
			if (Name.isEmpty()) {
				delete pItem;
				continue; // entry from template
			}
			i++;
		}
	}
}

void COptionsWindow::SaveOptionList()
{
	CloseOptionEdit(true);

	if (!ui.treeOptions) return;

	QMap<QString, QList<QString>> OptionMap;

	// cache unlisted set eOnlySpec global presets
	foreach(const QString& Name, m_AdvOptions.keys()) {
		foreach(const QString & Value, m_pBox->GetTextList(Name, m_Template)) {
			QStringList Values = Value.split(",");
			if (Values.count() < 2 && m_AdvOptions[Name].ProcSpec == eOnlySpec)
				OptionMap[Name].append(Values[0]);
		}
	}

	for (int i = 0; i < ui.treeOptions->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeOptions->topLevelItem(i);
		QString Name = pItem->data(0, Qt::UserRole).toString();
		if (Name.isEmpty())
			continue; // entry from template
		QString Program = pItem->data(1, Qt::UserRole).toString();
		QString Value = pItem->data(2, Qt::UserRole).toString();
		QStringList Options;
		if (!Program.isEmpty()) Options.append(Program);
		if (!Value.isEmpty()) Options.append(Value);
		OptionMap[Name].append(Options.join(","));
	}

	foreach(const QString & Key, m_AdvOptions.keys()) {
		WriteTextList(Key, OptionMap[Key]);
	}
}

void COptionsWindow::AddOptionEntry(const QString& Name, QString Program, const QString& Value, const QString& Template)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem();

	pItem->setText(0, Name + (Template.isEmpty() ? "" : " (" + Template + ")"));
	pItem->setData(0, Qt::UserRole, !Template.isEmpty() ? "" : Name);

	pItem->setData(1, Qt::UserRole, Program);
	bool bAll = Program.isEmpty();
	if (bAll)
		Program = tr("All Programs");
	bool Not = Program.left(1) == "!";
	if (Not)
		Program.remove(0, 1);
	//if (Program.left(1) == "<")
	//	Program = tr("Group: %1").arg(Program.mid(1, Program.length() - 2));
	//else if(!bAll)
	//	m_Programs.insert(Program);
	pItem->setText(1, (Not ? "NOT " : "") + Program);
	
	pItem->setText(2, Value);
	pItem->setData(2, Qt::UserRole, Value);

	//if(Template.isEmpty())
	//	pItem->setCheckState(0, disabled ? Qt::Unchecked : Qt::Checked);
	ui.treeOptions->addTopLevelItem(pItem);
}

void COptionsWindow::OnAddOption()
{ 
	CComboInputDialog progDialog(this);
	progDialog.setText(tr("Add special option:"));
	progDialog.setEditable(true);

	foreach(const QString & Name, m_AdvOptions.keys())
		progDialog.addItem(Name, Name, m_AdvOptions[Name].Description);

	progDialog.setValue("EnableMiniDump");

	if (!progDialog.exec())
		return;

	QString Name = progDialog.value(); 

	AddOptionEntry(Name, "", "");
}

void COptionsWindow::OnDelOption()
{
	DeleteAccessEntry(ui.treeOptions->currentItem());
	OnAdvancedChanged();
}

void COptionsWindow::OnOptionItemDoubleClicked(QTreeWidgetItem* pItem, int Column)
{
	//if (Column == 0)
	//	return;

	QString Name = pItem->data(0, Qt::UserRole).toString();
	if (Name.isEmpty()) {
		QMessageBox::warning(this, "SandboxiePlus", tr("Template values can not be edited."));
		return;
	}

	QString Program = pItem->data(1, Qt::UserRole).toString();

	if (m_AdvOptions[Name].ProcSpec != eNoSpec)
	{
		QWidget* pProgram = new QWidget();
		pProgram->setAutoFillBackground(true);
		QHBoxLayout* pLayout = new QHBoxLayout();
		pLayout->setContentsMargins(0,0,0,0);
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
		if (m_AdvOptions[Name].ProcSpec != eOnlySpec)
			pCombo->addItem(tr("All Programs"), "");

		//foreach(const QString Group, GetCurrentGroups()){
		//	QString GroupName = Group.mid(1, Group.length() - 2);
		//	pCombo->addItem(tr("Group: %1").arg(GroupName), Group);
		//}

		foreach(const QString & Name, m_Programs)
			pCombo->addItem(Name, Name);


		pCombo->setEditable(true);
		int Index = pCombo->findData(Program);
		pCombo->setCurrentIndex(Index);
		if (Index == -1)
			pCombo->setCurrentText(Program);
		pLayout->addWidget(pCombo);

		ui.treeOptions->setItemWidget(pItem, 1, pProgram);
	}

	QComboBox* pValue = new QComboBox();
	pValue->setEditable(true);
	foreach(const QString& Value, m_AdvOptions[Name].Values)
		pValue->addItem(Value);
	int pos = pValue->findData(pItem->data(2, Qt::UserRole));
	pValue->setCurrentIndex(pos);
	if (pos == -1)
		pValue->setCurrentText(pItem->text(2));
	ui.treeOptions->setItemWidget(pItem, 2, pValue);
}

void COptionsWindow::OnOptionChanged(QTreeWidgetItem* pItem, int Column)
{
	if (Column != 0)
		return;

	OnAdvancedChanged();
}
	
void COptionsWindow::CloseOptionEdit(bool bSave)
{
	if (!ui.treeOptions) return;

	for (int i = 0; i < ui.treeOptions->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeOptions->topLevelItem(i);
		CloseOptionEdit(pItem, bSave);
	}
}

void COptionsWindow::CloseOptionEdit(QTreeWidgetItem* pItem, bool bSave)
{
	QComboBox* pValue = (QComboBox*)ui.treeOptions->itemWidget(pItem, 2);
	if (!pValue)
		return;

	if (bSave)
	{
		QWidget* pProgram = ui.treeOptions->itemWidget(pItem, 1);
		if (pProgram) 
		{
			QHBoxLayout* pLayout = (QHBoxLayout*)pProgram->layout();
			QToolButton* pNot = (QToolButton*)pLayout->itemAt(0)->widget();
			QComboBox* pCombo = (QComboBox*)pLayout->itemAt(1)->widget();

			QString Program = pCombo->currentText();
			int Index = pCombo->findText(Program);
			if (Index != -1)
				Program = pCombo->itemData(Index, Qt::UserRole).toString();
			//if (!Program.isEmpty() && Program.left(1) != "<")
			//	m_Programs.insert(Program);

			pItem->setText(1, (pNot->isChecked() ? "NOT " : "") + pCombo->currentText());
			pItem->setData(1, Qt::UserRole, (pNot->isChecked() ? "!" : "") + Program);
		}
		pItem->setText(2, pValue->currentText());
		pItem->setData(2, Qt::UserRole, pValue->currentText());

		OnAdvancedChanged();
	}

	ui.treeOptions->setItemWidget(pItem, 1, NULL);
	ui.treeOptions->setItemWidget(pItem, 2, NULL);
}
//

// triggers
void COptionsWindow::AddTriggerItem(const QString& Value, ETriggerAction Type, bool disabled, const QString& Template)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem();
	pItem->setData(0, Qt::UserRole, Template.isEmpty() ? Type : -1);
	switch (Type)
	{
		case eOnStartCmd:
			pItem->setText(0, tr("On Start"));
			pItem->setText(1, tr("Run Command"));
			break;
		case eOnStartSvc:
			pItem->setText(0, tr("On Start"));
			pItem->setText(1, tr("Start Service"));
			break;
		case eAutoExec:
			pItem->setText(0, tr("On Init"));
			pItem->setText(1, tr("Run Command"));
			break;
		case eRecoveryCheck:
			pItem->setText(0, tr("On File Recovery"));
			pItem->setText(1, tr("Run Command"));
			break;
		case eDeleteCmd:
			pItem->setText(0, tr("On Delete Content"));
			pItem->setText(1, tr("Run Command"));
			break;
		case eTerminateCmd:
			pItem->setText(0, tr("On Terminate"));
			pItem->setText(1, tr("Run Command"));
	}
	pItem->setText(2, Value);
	pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);

	if (Template.isEmpty())
		pItem->setCheckState(0, disabled ? Qt::Unchecked : Qt::Checked);

	ui.treeTriggers->addTopLevelItem(pItem);
}

void COptionsWindow::OnAddAutoRun()
{
	QString Value = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter the command line to be executed"), QLineEdit::Normal);
	if (Value.isEmpty())
		return;

	AddTriggerItem(Value, eOnStartCmd);
	OnAdvancedChanged();
}

void COptionsWindow::OnAddAutoSvc()
{
	QString Value = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter a service identifier"), QLineEdit::Normal);
	if (Value.isEmpty())
		return;

	AddTriggerItem(Value, eOnStartSvc);
	OnAdvancedChanged();
}

void COptionsWindow::OnAddAutoExec()
{
	QString Value = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter the command line to be executed"), QLineEdit::Normal);
	if (Value.isEmpty())
		return;

	AddTriggerItem(Value, eAutoExec);
	OnAdvancedChanged();
}

void COptionsWindow::OnAddDeleteCmd()
{
	QString Value = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter the command line to be executed"), QLineEdit::Normal);
	if (Value.isEmpty())
		return;

	AddTriggerItem(Value, eDeleteCmd);
	OnAdvancedChanged();
}

void COptionsWindow::OnAddTerminateCmd()
{
	QString Value = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter the command line to be executed"), QLineEdit::Normal);
	if (Value.isEmpty())
		return;

	AddTriggerItem(Value, eTerminateCmd);
	OnAdvancedChanged();
}

void COptionsWindow::OnAddRecoveryCheck()
{
	QString Value = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter the command line to be executed"), QLineEdit::Normal);
	if (Value.isEmpty())
		return;

	AddTriggerItem(Value, eRecoveryCheck);
	OnAdvancedChanged();
}

void COptionsWindow::OnDelAuto()
{
	DeleteAccessEntry(ui.treeTriggers->currentItem());
	OnAdvancedChanged();
}
//

void COptionsWindow::OnAddProcess()
{
	QString Process = SelectProgram();
	if (Process.isEmpty())
		return;

	AddHiddenProcEntry(Process);

	OnAdvancedChanged();
}

void COptionsWindow::OnDelProcess()
{
	DeleteAccessEntry(ui.treeHideProc->currentItem());
	OnAdvancedChanged();
}

void COptionsWindow::OnHostProcessAllow()
{
	QString Process = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter a program file name to allow access to this sandbox"));
	if (Process.isEmpty())
		return;

	AddHostProcEntry(Process, false);

	OnAdvancedChanged();
}

void COptionsWindow::OnHostProcessDeny()
{
	QString Process = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter a program file name to deny access to this sandbox"));
	if (Process.isEmpty())
		return;

	AddHostProcEntry(Process, true);

	OnAdvancedChanged();
}

void COptionsWindow::OnDelHostProcess()
{
	DeleteAccessEntry(ui.treeHostProc->currentItem());
	OnAdvancedChanged();
}

void COptionsWindow::ShowHiddenProcTmpl(bool bUpdate)
{
	if (ui.chkShowHiddenProcTmpl->isChecked())
	{
		foreach(const QString& Template, m_pBox->GetTemplates())
		{
			foreach(const QString& Value, m_pBox->GetTextListTmpl("HideHostProcess", Template))
				AddHiddenProcEntry(Value, Template);
		}
	}
	else if (bUpdate)
	{
		for (int i = 0; i < ui.treeHideProc->topLevelItemCount(); )
		{
			QTreeWidgetItem* pItem = ui.treeHideProc->topLevelItem(i);
			int Type = pItem->data(0, Qt::UserRole).toInt();
			if (Type == -1) {
				delete pItem;
				continue; // entry from template
			}
			i++;
		}
	}
}

void COptionsWindow::ShowHostProcTmpl(bool bUpdate)
{
	if (ui.chkShowHostProcTmpl->isChecked())
	{
		foreach(const QString& Template, m_pBox->GetTemplates())
		{
			foreach(const QString & Value, m_pBox->GetTextListTmpl("DenyHostAccess", Template)) {
				StrPair NameVal = Split2(Value, ",");
				AddHostProcEntry(NameVal.first, NameVal.second.left(1).toLower() == "y", Template);
			}
		}
	}
	else if (bUpdate)
	{
		for (int i = 0; i < ui.treeHostProc->topLevelItemCount(); )
		{
			QTreeWidgetItem* pItem = ui.treeHostProc->topLevelItem(i);
			int Type = pItem->data(0, Qt::UserRole).toInt();
			if (Type == -1) {
				delete pItem;
				continue; // entry from template
			}
			i++;
		}
	}
}

void COptionsWindow::AddHiddenProcEntry(const QString& Name, const QString& Template)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem();
	pItem->setText(0, Name + (Template.isEmpty() ? "" : " (" + Template + ")"));
	pItem->setData(0, Qt::UserRole, Template.isEmpty() ? 0 : -1);
	ui.treeHideProc->addTopLevelItem(pItem);
}

void COptionsWindow::AddHostProcEntry(const QString& Name, bool Deny, const QString& Template)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem();
	pItem->setText(0, Name + (Template.isEmpty() ? "" : " (" + Template + ")"));
	pItem->setData(0, Qt::UserRole, Template.isEmpty() ? 0 : -1);
	pItem->setText(1, Deny ? tr("Deny") : tr("Allow"));
	pItem->setData(1, Qt::UserRole, Deny);
	ui.treeHostProc->addTopLevelItem(pItem);
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

	OnAdvancedChanged();
}

void COptionsWindow::OnDelUser()
{
	foreach(QListWidgetItem* pItem, ui.lstUsers->selectedItems())
		delete pItem;

	OnAdvancedChanged();
}

void COptionsWindow::CreateDebug()
{
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
			for (; Description[0] == '-' && Column < 10; Column++) Description.remove(0, 1);

			SDbgOpt DbgOption = { ValueDescr[0], ValueDescr.size() >= 2 ? ValueDescr[1] : "y" , false};

			QString Info = DbgOption.Name + "=" + DbgOption.Value;
			QCheckBox* pCheck = new QCheckBox(tr("%1 (%2)").arg(Description).arg(Info));
			if (ValueDescr.size() >= 2 && ValueDescr[1] == "x")
				pCheck->setDisabled(true);
			//pCheck->setToolTip(Info);
			ui.dbgLayout->addWidget(pCheck, RowCount++, Column, 1, 10-Column);

			connect(pCheck, SIGNAL(clicked(bool)), this, SLOT(OnDebugChanged()));
			m_DebugOptions.insert(pCheck, DbgOption);
		}

		for(int i=0; i < 10; i++)
			ui.dbgLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding), RowCount, i);
		ui.dbgLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), RowCount, 10);
	}
}

void COptionsWindow::OnDebugChanged()
{
	QCheckBox* pCheck = qobject_cast<QCheckBox*>(sender());
	m_DebugOptions[pCheck].Changed = true;
	OnOptChanged();
}

void COptionsWindow::LoadDebug()
{
	foreach(QCheckBox* pCheck, m_DebugOptions.keys()) 
	{
		SDbgOpt& DbgOption = m_DebugOptions[pCheck];
		DbgOption.Changed = false;
		ReadAdvancedCheck(DbgOption.Name, pCheck, DbgOption.Value);
	}
}

void COptionsWindow::SaveDebug()
{
	foreach(QCheckBox * pCheck, m_DebugOptions.keys())
	{
		SDbgOpt& DbgOption = m_DebugOptions[pCheck];
		if (!DbgOption.Changed)
			continue;
		WriteAdvancedCheck(pCheck, DbgOption.Name, DbgOption.Value);
		DbgOption.Changed = false;
	}
}

#define WIN32_NO_STATUS
typedef long NTSTATUS;

#include "..\..\Sandboxie\common\win32_ntddk.h"

typedef struct _SYSTEM_FIRMWARE_TABLE_INFORMATION {
    ULONG ProviderSignature;
    ULONG Action;
    ULONG TableID;
    ULONG TableBufferLength;
    UCHAR TableBuffer[ANYSIZE_ARRAY];
} SYSTEM_FIRMWARE_TABLE_INFORMATION, *PSYSTEM_FIRMWARE_TABLE_INFORMATION;

#define FIRMWARE_TABLE_PROVIDER_ACPI  'ACPI'
#define FIRMWARE_TABLE_PROVIDER_SMBIOS 'RSMB'

typedef enum _SYSTEM_FIRMWARE_TABLE_ACTION {
    SystemFirmwareTable_Enumerate,
    SystemFirmwareTable_Get
} SYSTEM_FIRMWARE_TABLE_ACTION;

void COptionsWindow::OnDumpFW()
{
    ULONG returnLength = 0;
    NTSTATUS status;
    SYSTEM_FIRMWARE_TABLE_INFORMATION* firmwareTableInfo;
    ULONG firmwareTableSize = sizeof(SYSTEM_FIRMWARE_TABLE_INFORMATION) + 0x10000; // Initial size

retry:
    firmwareTableInfo = (SYSTEM_FIRMWARE_TABLE_INFORMATION*)malloc(firmwareTableSize);
    firmwareTableInfo->ProviderSignature = FIRMWARE_TABLE_PROVIDER_SMBIOS;
    firmwareTableInfo->Action = SystemFirmwareTable_Get;
    firmwareTableInfo->TableID = 0;
    firmwareTableInfo->TableBufferLength = firmwareTableSize - sizeof(SYSTEM_FIRMWARE_TABLE_INFORMATION);

    status = NtQuerySystemInformation(SystemFirmwareTableInformation, firmwareTableInfo, firmwareTableSize, &returnLength);

    if (status == 0xC0000023L/*STATUS_BUFFER_TOO_SMALL*/)  {
		free(firmwareTableInfo);
		firmwareTableSize += 0x10000;
		goto retry;
    }

    if (!NT_SUCCESS(status))
		CSandMan::ShowMessageBox(this, QMessageBox::Critical, tr("Failed to retrieve firmware table information."));
    else if(firmwareTableInfo->TableBufferLength)
	{
		HKEY hKey;
		DWORD disposition;
		if(RegCreateKeyExW(HKEY_CURRENT_USER, L"System\\SbieCustom", 0, 0, 0, KEY_WRITE, NULL, &hKey, &disposition) == ERROR_SUCCESS) 
		{
			if(RegSetValueExW(hKey, L"SMBiosTable", 0, REG_BINARY, firmwareTableInfo->TableBuffer, firmwareTableInfo->TableBufferLength) == ERROR_SUCCESS)
				CSandMan::ShowMessageBox(this, QMessageBox::Information, tr("Firmware table saved successfully to host registry: HKEY_CURRENT_USER\\System\\SbieCustom<br />you can copy it to the sandboxed registry to have a different value for each box."));

            RegCloseKey(hKey);
        }
    }

    free(firmwareTableInfo);
}

void COptionsWindow::InitLangID()
{
	// Note: list by ChatGPT
	ui.cmbLangID->addItem("Use System Default", 0);
	ui.cmbLangID->addItem("Afrikaans (af-ZA)", 1078);
	ui.cmbLangID->addItem("Albanian (sq-AL)", 1052);
	ui.cmbLangID->addItem("Amharic (am-ET)", 1118);
	ui.cmbLangID->addItem("Arabic (Algeria) (ar-DZ)", 5121);
	ui.cmbLangID->addItem("Arabic (Bahrain) (ar-BH)", 15361);
	ui.cmbLangID->addItem("Arabic (Egypt) (ar-EG)", 3073);
	ui.cmbLangID->addItem("Arabic (Iraq) (ar-IQ)", 2049);
	ui.cmbLangID->addItem("Arabic (Jordan) (ar-JO)", 11265);
	ui.cmbLangID->addItem("Arabic (Kuwait) (ar-KW)", 13313);
	ui.cmbLangID->addItem("Arabic (Lebanon) (ar-LB)", 12289);
	ui.cmbLangID->addItem("Arabic (Libya) (ar-LY)", 4097);
	ui.cmbLangID->addItem("Arabic (Morocco) (ar-MA)", 6145);
	ui.cmbLangID->addItem("Arabic (Oman) (ar-OM)", 8193);
	ui.cmbLangID->addItem("Arabic (Qatar) (ar-QA)", 16385);
	ui.cmbLangID->addItem("Arabic (Saudi Arabia) (ar-SA)", 1025);
	ui.cmbLangID->addItem("Arabic (Syria) (ar-SY)", 10241);
	ui.cmbLangID->addItem("Arabic (Tunisia) (ar-TN)", 7169);
	ui.cmbLangID->addItem("Arabic (U.A.E.) (ar-AE)", 14337);
	ui.cmbLangID->addItem("Arabic (Yemen) (ar-YE)", 9217);
	ui.cmbLangID->addItem("Armenian (hy-AM)", 1067);
	ui.cmbLangID->addItem("Assamese (as-IN)", 1101);
	ui.cmbLangID->addItem("Azerbaijani (Cyrillic) (az-Cyrl-AZ)", 2092);
	ui.cmbLangID->addItem("Azerbaijani (Latin) (az-Latn-AZ)", 1068);
	ui.cmbLangID->addItem("Basque (eu-ES)", 1069);
	ui.cmbLangID->addItem("Belarusian (be-BY)", 1059);
	ui.cmbLangID->addItem("Bengali (Bangladesh) (bn-BD)", 2117);
	ui.cmbLangID->addItem("Bengali (India) (bn-IN)", 1093);
	ui.cmbLangID->addItem("Bosnian (Cyrillic, Bosnia and Herzegovina) (bs-Cyrl-BA)", 8218);
	ui.cmbLangID->addItem("Bosnian (Latin, Bosnia and Herzegovina) (bs-Latn-BA)", 5146);
	ui.cmbLangID->addItem("Bulgarian (bg-BG)", 1026);
	ui.cmbLangID->addItem("Catalan (ca-ES)", 1027);
	ui.cmbLangID->addItem("Chinese (Simplified) (zh-CN)", 2052);
	ui.cmbLangID->addItem("Chinese (Traditional) (zh-TW)", 1028);
	ui.cmbLangID->addItem("Chinese (Hong Kong SAR) (zh-HK)", 3076);
	ui.cmbLangID->addItem("Chinese (Macau SAR) (zh-MO)", 5124);
	ui.cmbLangID->addItem("Chinese (Singapore) (zh-SG)", 4100);
	ui.cmbLangID->addItem("Croatian (hr-HR)", 1050);
	ui.cmbLangID->addItem("Croatian (Latin, Bosnia and Herzegovina) (hr-BA)", 4122);
	ui.cmbLangID->addItem("Czech (cs-CZ)", 1029);
	ui.cmbLangID->addItem("Danish (da-DK)", 1030);
	ui.cmbLangID->addItem("Dari (prs-AF)", 1164);
	ui.cmbLangID->addItem("Dutch (Belgium) (nl-BE)", 2067);
	ui.cmbLangID->addItem("Dutch (Netherlands) (nl-NL)", 1043);
	ui.cmbLangID->addItem("English (Australia) (en-AU)", 3081);
	ui.cmbLangID->addItem("English (Belize) (en-BZ)", 10249);
	ui.cmbLangID->addItem("English (Canada) (en-CA)", 4105);
	ui.cmbLangID->addItem("English (Caribbean) (en-029)", 9225);
	ui.cmbLangID->addItem("English (Hong Kong SAR) (en-HK)", 15369);
	ui.cmbLangID->addItem("English (India) (en-IN)", 16393);
	ui.cmbLangID->addItem("English (Indonesia) (en-ID)", 14345);
	ui.cmbLangID->addItem("English (Ireland) (en-IE)", 6153);
	ui.cmbLangID->addItem("English (Jamaica) (en-JM)", 8201);
	ui.cmbLangID->addItem("English (Malaysia) (en-MY)", 17417);
	ui.cmbLangID->addItem("English (New Zealand) (en-NZ)", 5129);
	ui.cmbLangID->addItem("English (Philippines) (en-PH)", 13321);
	ui.cmbLangID->addItem("English (Singapore) (en-SG)", 18441);
	ui.cmbLangID->addItem("English (South Africa) (en-ZA)", 7177);
	ui.cmbLangID->addItem("English (Trinidad and Tobago) (en-TT)", 11273);
	ui.cmbLangID->addItem("English (United Kingdom) (en-GB)", 2057);
	ui.cmbLangID->addItem("English (United States) (en-US)", 1033);
	ui.cmbLangID->addItem("English (Zimbabwe) (en-ZW)", 12297);
	ui.cmbLangID->addItem("Estonian (et-EE)", 1061);
	ui.cmbLangID->addItem("Faroese (fo-FO)", 1080);
	ui.cmbLangID->addItem("Filipino (fil-PH)", 1124);
	ui.cmbLangID->addItem("Finnish (fi-FI)", 1035);
	ui.cmbLangID->addItem("French (Belgium) (fr-BE)", 2060);
	ui.cmbLangID->addItem("French (Canada) (fr-CA)", 3084);
	ui.cmbLangID->addItem("French (France) (fr-FR)", 1036);
	ui.cmbLangID->addItem("French (Luxembourg) (fr-LU)", 5132);
	ui.cmbLangID->addItem("French (Monaco) (fr-MC)", 6156);
	ui.cmbLangID->addItem("French (Switzerland) (fr-CH)", 4108);
	ui.cmbLangID->addItem("Galician (gl-ES)", 1110);
	ui.cmbLangID->addItem("Georgian (ka-GE)", 1079);
	ui.cmbLangID->addItem("German (Austria) (de-AT)", 3079);
	ui.cmbLangID->addItem("German (Germany) (de-DE)", 1031);
	ui.cmbLangID->addItem("German (Liechtenstein) (de-LI)", 5127);
	ui.cmbLangID->addItem("German (Luxembourg) (de-LU)", 4103);
	ui.cmbLangID->addItem("German (Switzerland) (de-CH)", 2055);
	ui.cmbLangID->addItem("Greek (el-GR)", 1032);
	ui.cmbLangID->addItem("Gujarati (gu-IN)", 1095);
	ui.cmbLangID->addItem("Hebrew (he-IL)", 1037);
	ui.cmbLangID->addItem("Hindi (hi-IN)", 1081);
	ui.cmbLangID->addItem("Hungarian (hu-HU)", 1038);
	ui.cmbLangID->addItem("Icelandic (is-IS)", 1039);
	ui.cmbLangID->addItem("Igbo (ig-NG)", 1136);
	ui.cmbLangID->addItem("Indonesian (id-ID)", 1057);
	ui.cmbLangID->addItem("Irish (ga-IE)", 2108);
	ui.cmbLangID->addItem("Italian (Italy) (it-IT)", 1040);
	ui.cmbLangID->addItem("Italian (Switzerland) (it-CH)", 2064);
	ui.cmbLangID->addItem("Japanese (ja-JP)", 1041);
	ui.cmbLangID->addItem("Kannada (kn-IN)", 1099);
	ui.cmbLangID->addItem("Kazakh (kk-KZ)", 1087);
	ui.cmbLangID->addItem("Khmer (km-KH)", 1107);
	ui.cmbLangID->addItem("K'iche' (quc-Latn-GT)", 1152);
	ui.cmbLangID->addItem("Kinyarwanda (rw-RW)", 1159);
	ui.cmbLangID->addItem("Konkani (kok-IN)", 1111);
	ui.cmbLangID->addItem("Korean (ko-KR)", 1042);
	ui.cmbLangID->addItem("Kyrgyz (ky-KG)", 1088);
	ui.cmbLangID->addItem("Lao (lo-LA)", 1108);
	ui.cmbLangID->addItem("Latvian (lv-LV)", 1062);
	ui.cmbLangID->addItem("Lithuanian (lt-LT)", 1063);
	ui.cmbLangID->addItem("Luxembourgish (lb-LU)", 1134);
	ui.cmbLangID->addItem("Macedonian (mk-MK)", 1071);
	ui.cmbLangID->addItem("Malay (Brunei Darussalam) (ms-BN)", 2110);
	ui.cmbLangID->addItem("Malay (Malaysia) (ms-MY)", 1086);
	ui.cmbLangID->addItem("Malayalam (ml-IN)", 1100);
	ui.cmbLangID->addItem("Maltese (mt-MT)", 1082);
	ui.cmbLangID->addItem("Maori (mi-NZ)", 1153);
	ui.cmbLangID->addItem("Mapudungun (arn-CL)", 1146);
	ui.cmbLangID->addItem("Marathi (mr-IN)", 1102);
	ui.cmbLangID->addItem("Mongolian (Cyrillic) (mn-MN)", 1104);
	ui.cmbLangID->addItem("Mongolian (Traditional Mongolian) (mn-Mong-CN)", 2128);
	ui.cmbLangID->addItem("Nepali (ne-NP)", 1121);
	ui.cmbLangID->addItem("Norwegian (Bokmal) (nb-NO)", 1044);
	ui.cmbLangID->addItem("Norwegian (Nynorsk) (nn-NO)", 2068);
	ui.cmbLangID->addItem("Occitan (oc-FR)", 1154);
	ui.cmbLangID->addItem("Odia (or-IN)", 1096);
	ui.cmbLangID->addItem("Pashto (ps-AF)", 1123);
	ui.cmbLangID->addItem("Persian (fa-IR)", 1065);
	ui.cmbLangID->addItem("Polish (pl-PL)", 1045);
	ui.cmbLangID->addItem("Portuguese (Brazil) (pt-BR)", 1046);
	ui.cmbLangID->addItem("Portuguese (Portugal) (pt-PT)", 2070);
	ui.cmbLangID->addItem("Punjabi (Gurmukhi) (pa-IN)", 1094);
	ui.cmbLangID->addItem("Quechua (Bolivia) (quz-BO)", 1131);
	ui.cmbLangID->addItem("Quechua (Ecuador) (quz-EC)", 2155);
	ui.cmbLangID->addItem("Quechua (Peru) (quz-PE)", 3179);
	ui.cmbLangID->addItem("Romanian (ro-RO)", 1048);
	ui.cmbLangID->addItem("Romansh (rm-CH)", 1047);
	ui.cmbLangID->addItem("Russian (ru-RU)", 1049);
	ui.cmbLangID->addItem("Sami (Inari) (smn-FI)", 9275);
	ui.cmbLangID->addItem("Sami (Lule, Norway) (smj-NO)", 4155);
	ui.cmbLangID->addItem("Sami (Lule, Sweden) (smj-SE)", 5179);
	ui.cmbLangID->addItem("Sami (Northern, Finland) (se-FI)", 3131);
	ui.cmbLangID->addItem("Sami (Northern, Norway) (se-NO)", 1083);
	ui.cmbLangID->addItem("Sami (Northern, Sweden) (se-SE)", 2107);
	ui.cmbLangID->addItem("Sami (Skolt) (sms-FI)", 8251);
	ui.cmbLangID->addItem("Sami (Southern, Norway) (sma-NO)", 6203);
	ui.cmbLangID->addItem("Sami (Southern, Sweden) (sma-SE)", 7227);
	ui.cmbLangID->addItem("Sanskrit (sa-IN)", 1103);
	ui.cmbLangID->addItem("Serbian (Cyrillic, Bosnia and Herzegovina) (sr-Cyrl-BA)", 1026);
	ui.cmbLangID->addItem("Serbian (Cyrillic, Montenegro) (sr-Cyrl-ME)", 12314);
	ui.cmbLangID->addItem("Serbian (Cyrillic, Serbia) (sr-Cyrl-RS)", 3098);
	ui.cmbLangID->addItem("Serbian (Latin, Bosnia and Herzegovina) (sr-Latn-BA)", 2074);
	ui.cmbLangID->addItem("Serbian (Latin, Montenegro) (sr-Latn-ME)", 13317);
	ui.cmbLangID->addItem("Serbian (Latin, Serbia) (sr-Latn-RS)", 9242);
	ui.cmbLangID->addItem("Sesotho sa Leboa (nso-ZA)", 1132);
	ui.cmbLangID->addItem("Sinhala (si-LK)", 1115);
	ui.cmbLangID->addItem("Slovak (sk-SK)", 1051);
	ui.cmbLangID->addItem("Slovenian (sl-SI)", 1060);
	ui.cmbLangID->addItem("Spanish (Argentina) (es-AR)", 11274);
	ui.cmbLangID->addItem("Spanish (Bolivia) (es-BO)", 16394);
	ui.cmbLangID->addItem("Spanish (Chile) (es-CL)", 13322);
	ui.cmbLangID->addItem("Spanish (Colombia) (es-CO)", 9226);
	ui.cmbLangID->addItem("Spanish (Costa Rica) (es-CR)", 5130);
	ui.cmbLangID->addItem("Spanish (Dominican Republic) (es-DO)", 7178);
	ui.cmbLangID->addItem("Spanish (Ecuador) (es-EC)", 12298);
	ui.cmbLangID->addItem("Spanish (El Salvador) (es-SV)", 17418);
	ui.cmbLangID->addItem("Spanish (Guatemala) (es-GT)", 4106);
	ui.cmbLangID->addItem("Spanish (Honduras) (es-HN)", 18442);
	ui.cmbLangID->addItem("Spanish (Mexico) (es-MX)", 2058);
	ui.cmbLangID->addItem("Spanish (Nicaragua) (es-NI)", 19466);
	ui.cmbLangID->addItem("Spanish (Panama) (es-PA)", 6154);
	ui.cmbLangID->addItem("Spanish (Paraguay) (es-PY)", 15370);
	ui.cmbLangID->addItem("Spanish (Peru) (es-PE)", 10250);
	ui.cmbLangID->addItem("Spanish (Puerto Rico) (es-PR)", 20490);
	ui.cmbLangID->addItem("Spanish (Spain) (es-ES)", 1034);
	ui.cmbLangID->addItem("Spanish (United States) (es-US)", 21514);
	ui.cmbLangID->addItem("Spanish (Uruguay) (es-UY)", 14346);
	ui.cmbLangID->addItem("Spanish (Venezuela) (es-VE)", 8202);
	ui.cmbLangID->addItem("Swahili (sw-KE)", 1089);
	ui.cmbLangID->addItem("Swedish (Finland) (sv-FI)", 2077);
	ui.cmbLangID->addItem("Swedish (Sweden) (sv-SE)", 1053);
	ui.cmbLangID->addItem("Syriac (syr-SY)", 1114);
	ui.cmbLangID->addItem("Tajik (Cyrillic) (tg-Cyrl-TJ)", 1064);
	ui.cmbLangID->addItem("Tamil (ta-IN)", 1097);
	ui.cmbLangID->addItem("Tatar (tt-RU)", 1092);
	ui.cmbLangID->addItem("Telugu (te-IN)", 1098);
	ui.cmbLangID->addItem("Thai (th-TH)", 1054);
	ui.cmbLangID->addItem("Tibetan (bo-CN)", 1105);
	ui.cmbLangID->addItem("Turkish (tr-TR)", 1055);
	ui.cmbLangID->addItem("Turkmen (tk-TM)", 1090);
	ui.cmbLangID->addItem("Ukrainian (uk-UA)", 1058);
	ui.cmbLangID->addItem("Upper Sorbian (hsb-DE)", 1070);
	ui.cmbLangID->addItem("Urdu (India) (ur-IN)", 2080);
	ui.cmbLangID->addItem("Urdu (Pakistan) (ur-PK)", 1056);
	ui.cmbLangID->addItem("Uzbek (Cyrillic) (uz-Cyrl-UZ)", 2115);
	ui.cmbLangID->addItem("Uzbek (Latin) (uz-Latn-UZ)", 1091);
	ui.cmbLangID->addItem("Vietnamese (vi-VN)", 1066);
	ui.cmbLangID->addItem("Welsh (cy-GB)", 1106);
	ui.cmbLangID->addItem("Wolof (wo-SN)", 1160);
	ui.cmbLangID->addItem("Xhosa (xh-ZA)", 1076);
	ui.cmbLangID->addItem("Yi (ii-CN)", 1144);
	ui.cmbLangID->addItem("Yoruba (yo-NG)", 1130);
	ui.cmbLangID->addItem("Zulu (zu-ZA)", 1077);
}
