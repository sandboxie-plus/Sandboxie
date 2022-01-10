#include "stdafx.h"
#include "OptionsWindow.h"
#include "SandMan.h"
#include "SettingsWindow.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/Common.h"
#include "../MiscHelpers/Common/ComboInputDialog.h"
#include "../MiscHelpers/Common/SettingsWidgets.h"
#include "Helpers/WinAdmin.h"

void COptionsWindow::CreateAdvanced()
{
	connect(ui.chkPreferExternalManifest, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkNoWindowRename, SIGNAL(clicked(bool)), this, SLOT(OnNoWindowRename()));
	connect(ui.chkNestedJobs, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkUseSbieWndStation, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));

	connect(ui.chkAddToJob, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkProtectSCM, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkRestrictServices, SIGNAL(clicked(bool)), this, SLOT(OnSysSvcChanged()));
	connect(ui.chkElevateRpcss, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkProtectSystem, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkDropPrivileges, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));

	connect(ui.chkOpenCOM, SIGNAL(clicked(bool)), this, SLOT(OnOpenCOM()));
	connect(ui.chkComTimeout, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));

	connect(ui.chkNoSecurityIsolation, SIGNAL(clicked(bool)), this, SLOT(OnIsolationChanged()));
	connect(ui.chkNoSecurityFiltering, SIGNAL(clicked(bool)), this, SLOT(OnIsolationChanged()));

	connect(ui.chkOpenDevCMApi, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	//connect(ui.chkOpenLsaSSPI, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkOpenSamEndpoint, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkOpenLsaEndpoint, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));


	connect(ui.chkDisableMonitor, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));

	connect(ui.chkCallTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkFileTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkPipeTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkKeyTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkIpcTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkGuiTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkComTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkNetFwTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkApiTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkDbgTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.chkErrTrace, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));

	connect(ui.btnAddAutoExec, SIGNAL(clicked(bool)), this, SLOT(OnAddAutoExec()));
	connect(ui.btnDelAutoExec, SIGNAL(clicked(bool)), this, SLOT(OnDelAutoExec()));

	connect(ui.chkHideOtherBoxes, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
	connect(ui.btnAddProcess, SIGNAL(clicked(bool)), this, SLOT(OnAddProcess()));
	connect(ui.btnDelProcess, SIGNAL(clicked(bool)), this, SLOT(OnDelProcess()));

	connect(ui.btnAddUser, SIGNAL(clicked(bool)), this, SLOT(OnAddUser()));
	connect(ui.btnDelUser, SIGNAL(clicked(bool)), this, SLOT(OnDelUser()));
	connect(ui.chkMonitorAdminOnly, SIGNAL(clicked(bool)), this, SLOT(OnAdvancedChanged()));
}


void COptionsWindow::LoadAdvanced()
{
	ui.chkPreferExternalManifest->setChecked(m_pBox->GetBool("PreferExternalManifest", false));
	ui.chkNestedJobs->setChecked(m_pBox->GetBool("AllowBoxedJobs", false));
	ui.chkUseSbieWndStation->setChecked(m_pBox->GetBool("UseSbieWndStation", true));

	ui.chkAddToJob->setChecked(!m_pBox->GetBool("NoAddProcessToJob", false));
	ui.chkProtectSCM->setChecked(!m_pBox->GetBool("UnrestrictedSCM", false));
	ui.chkRestrictServices->setChecked(!m_pBox->GetBool("RunServicesAsSystem", false));
	ui.chkElevateRpcss->setChecked(m_pBox->GetBool("RunRpcssAsSystem", false));
	ui.chkProtectSystem->setChecked(!m_pBox->GetBool("ExposeBoxedSystem", false));
	ui.chkDropPrivileges->setChecked(m_pBox->GetBool("StripSystemPrivileges", true));

	ui.chkComTimeout->setChecked(!m_pBox->GetBool("RpcMgmtSetComTimeout", true));

	ui.chkNoSecurityIsolation->setChecked(m_pBox->GetBool("NoSecurityIsolation", false));
	ui.chkNoSecurityFiltering->setChecked(m_pBox->GetBool("NoSecurityFiltering", false));

	ui.chkOpenDevCMApi->setChecked(m_pBox->GetBool("OpenDevCMApi", false));
	//ui.chkOpenLsaSSPI->setChecked(!m_pBox->GetBool("BlockPassword", true)); // OpenLsaSSPI
	ui.chkOpenSamEndpoint->setChecked(m_pBox->GetBool("OpenSamEndpoint", false));
	ui.chkOpenLsaEndpoint->setChecked(m_pBox->GetBool("OpenLsaEndpoint", false));


	QStringList AutoExec = m_pBox->GetTextList("AutoExec", m_Template);
	ui.lstAutoExec->clear();
	ui.lstAutoExec->addItems(AutoExec);


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
	ui.chkDbgTrace->setChecked(m_pBox->GetBool("DebugTrace", false));
	ui.chkErrTrace->setChecked(m_pBox->GetBool("ErrorTrace", false));
	QSharedPointer<CSandBoxPlus> pBoxPlus = m_pBox.objectCast<CSandBoxPlus>();
	if (pBoxPlus)
		ui.chkApiTrace->setChecked(pBoxPlus->HasLogApi());

	ui.chkHideOtherBoxes->setChecked(m_pBox->GetBool("HideOtherBoxes", false));
	QStringList Processes = m_pBox->GetTextList("HideHostProcess", m_Template);
	ui.lstProcesses->clear();
	ui.lstProcesses->addItems(Processes);


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
}

void COptionsWindow::SaveAdvanced()
{
	WriteAdvancedCheck(ui.chkPreferExternalManifest, "PreferExternalManifest", "y", "");
	WriteAdvancedCheck(ui.chkUseSbieWndStation, "UseSbieWndStation", "", "n");

	WriteAdvancedCheck(ui.chkAddToJob, "NoAddProcessToJob", "", "y");
	WriteAdvancedCheck(ui.chkProtectSCM, "UnrestrictedSCM", "", "y");
	WriteAdvancedCheck(ui.chkNestedJobs, "AllowBoxedJobs", "y", "");
	WriteAdvancedCheck(ui.chkRestrictServices, "RunServicesAsSystem", "", "y");
	WriteAdvancedCheck(ui.chkElevateRpcss, "RunRpcssAsSystem", "y", "");
	WriteAdvancedCheck(ui.chkProtectSystem, "ExposeBoxedSystem", "", "y");
	WriteAdvancedCheck(ui.chkDropPrivileges, "StripSystemPrivileges", "", "n");

	WriteAdvancedCheck(ui.chkComTimeout, "RpcMgmtSetComTimeout", "n", "");

	WriteAdvancedCheck(ui.chkNoSecurityIsolation, "NoSecurityIsolation", "y", "");
	WriteAdvancedCheck(ui.chkNoSecurityFiltering, "NoSecurityFiltering", "y", "");

	WriteAdvancedCheck(ui.chkOpenDevCMApi, "OpenDevCMApi", "y", "");
	//WriteAdvancedCheck(ui.chkOpenLsaSSPI, "BlockPassword", "n", ""); // OpenLsaSSPI
	WriteAdvancedCheck(ui.chkOpenSamEndpoint, "OpenSamEndpoint", "y", "");
	WriteAdvancedCheck(ui.chkOpenLsaEndpoint, "OpenLsaEndpoint", "y", "");


	QStringList AutoExec;
	for (int i = 0; i < ui.lstAutoExec->count(); i++)
		AutoExec.append(ui.lstAutoExec->item(i)->text());
	WriteTextList("AutoExec", AutoExec);

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
	WriteAdvancedCheck(ui.chkDbgTrace, "DebugTrace", "y");
	WriteAdvancedCheck(ui.chkErrTrace, "ErrorTrace", "y");
	QSharedPointer<CSandBoxPlus> pBoxPlus = m_pBox.objectCast<CSandBoxPlus>();
	if (pBoxPlus)
		pBoxPlus->SetLogApi(ui.chkApiTrace->isChecked());

	WriteAdvancedCheck(ui.chkHideOtherBoxes, "HideOtherBoxes");

	QStringList Processes;
	for (int i = 0; i < ui.lstProcesses->count(); i++)
		Processes.append(ui.lstProcesses->item(i)->text());
	WriteTextList("HideHostProcess", Processes);

	QStringList Users;
	for (int i = 0; i < ui.lstUsers->count(); i++)
		Users.append(ui.lstUsers->item(i)->text());
	WriteText("Enabled", Users.count() > 0 ? "y," + Users.join(",") : "y");
	WriteAdvancedCheck(ui.chkMonitorAdminOnly, "MonitorAdminOnly");

	m_AdvancedChanged = false;
}

void COptionsWindow::OnIsolationChanged()
{
	if (ui.chkPrivacy->isChecked() || ui.chkUseSpecificity->isChecked())
		theGUI->CheckCertificate();

	UpdateBoxIsolation();

	m_AdvancedChanged = true;
	OnOptChanged();
}

void COptionsWindow::UpdateBoxIsolation()
{
	ui.chkNoSecurityFiltering->setEnabled(ui.chkNoSecurityIsolation->isChecked());

	ui.chkAddToJob->setEnabled(!ui.chkNoSecurityIsolation->isChecked());
	ui.chkNestedJobs->setEnabled(!ui.chkNoSecurityIsolation->isChecked());

	ui.chkOpenDevCMApi->setEnabled(!ui.chkNoSecurityIsolation->isChecked());
	ui.chkOpenSamEndpoint->setEnabled(!ui.chkNoSecurityIsolation->isChecked());
	ui.chkOpenLsaEndpoint->setEnabled(!ui.chkNoSecurityIsolation->isChecked());


	ui.chkRawDiskRead->setEnabled(!ui.chkNoSecurityIsolation->isChecked()); //  without isolation only user mode
	ui.chkRawDiskNotify->setEnabled(!ui.chkNoSecurityIsolation->isChecked());

	ui.chkDropRights->setEnabled(!ui.chkNoSecurityIsolation->isChecked() && !theAPI->IsRunningAsAdmin());

	ui.chkBlockNetShare->setEnabled(!ui.chkNoSecurityFiltering->isChecked());

	ui.chkBlockSpooler->setEnabled(!ui.chkNoSecurityIsolation->isChecked());
	ui.chkOpenSpooler->setEnabled(!ui.chkBlockSpooler->isChecked() && !ui.chkNoSecurityIsolation->isChecked());
	ui.chkPrintToFile->setEnabled(!ui.chkBlockSpooler->isChecked() && !ui.chkNoSecurityFiltering->isChecked());

	ui.chkCloseClipBoard->setEnabled(!ui.chkNoSecurityIsolation->isChecked());
}

void COptionsWindow::OnSysSvcChanged()
{
	ui.chkElevateRpcss->setDisabled(ui.chkNoSecurityIsolation->isChecked() && (!ui.chkRestrictServices->isChecked() || ui.chkMsiExemptions->isChecked()));
	m_AdvancedChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnAdvancedChanged()
{
	m_AdvancedChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnOpenCOM()
{
	if (ui.chkOpenCOM->isChecked()) {
		SetAccessEntry(eIPC, "", eOpen, "\\RPC Control\\epmapper");
		SetAccessEntry(eIPC, "", eOpen, "\\RPC Control\\LRPC*");
		SetAccessEntry(eIPC, "", eOpen, "\\RPC Control\\OLE*");
		SetAccessEntry(eIPC, "", eOpen, "*\\BaseNamedObjects*\\__ComCatalogCache__");
	}
	else {
		DelAccessEntry(eIPC, "", eOpen, "\\RPC Control\\epmapper");
		DelAccessEntry(eIPC, "", eOpen, "\\RPC Control\\LRPC*");
		DelAccessEntry(eIPC, "", eOpen, "\\RPC Control\\OLE*");
		DelAccessEntry(eIPC, "", eOpen, "*\\BaseNamedObjects*\\__ComCatalogCache__");
	}
}

void COptionsWindow::OnNoWindowRename()
{
	if (ui.chkNoWindowRename->isChecked())
		SetAccessEntry(eWnd, "", eOpen, "#");
	else
		DelAccessEntry(eWnd, "", eOpen, "#");
	m_AdvancedChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnAddAutoExec()
{
	QString Process = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter an auto exec command"));
	if (Process.isEmpty())
		return;

	ui.lstAutoExec->addItem(Process);

	m_AdvancedChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnDelAutoExec()
{
	foreach(QListWidgetItem * pItem, ui.lstAutoExec->selectedItems())
		delete pItem;

	m_AdvancedChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnAddProcess()
{
	QString Process = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter a program file name"));
	if (Process.isEmpty())
		return;

	ui.lstProcesses->addItem(Process);

	m_AdvancedChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnDelProcess()
{
	foreach(QListWidgetItem* pItem, ui.lstProcesses->selectedItems())
		delete pItem;

	m_AdvancedChanged = true;
	OnOptChanged();
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

	m_AdvancedChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnDelUser()
{
	foreach(QListWidgetItem* pItem, ui.lstUsers->selectedItems())
		delete pItem;
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
			for (; Description[0] == "-" && Column < 10; Column++) Description.remove(0, 1);

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