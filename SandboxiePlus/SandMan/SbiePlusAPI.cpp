#include "stdafx.h"
#include "SbiePlusAPI.h"
#include "..\MiscHelpers\Common\Common.h"


CSbiePlusAPI::CSbiePlusAPI(QObject* parent) : CSbieAPI(parent)
{

}

CSbiePlusAPI::~CSbiePlusAPI()
{
}

CSandBox* CSbiePlusAPI::NewSandBox(const QString& BoxName, class CSbieAPI* pAPI)
{
	return new CSandBoxPlus(BoxName, pAPI);
}

CBoxedProcess* CSbiePlusAPI::NewBoxedProcess(quint64 ProcessId, class CSandBox* pBox)
{
	return new CBoxedProcess(ProcessId, pBox);
}


///////////////////////////////////////////////////////////////////////////////
// CSandBox
//

CSandBoxPlus::CSandBoxPlus(const QString& BoxName, class CSbieAPI* pAPI) : CSandBox(BoxName, pAPI)
{
	m_bLogApiFound = false;
	m_bINetBlocked = false;
	m_bSharesAllowed = false;
	m_bDropRights = false;
	

	m_bSecurityRestricted = false;
	m_iUnsecureDebugging = 0;
}

CSandBoxPlus::~CSandBoxPlus()
{
}

void CSandBoxPlus::UpdateDetails()
{
	m_bLogApiFound = GetTextList("OpenPipePath").contains("\\Device\\NamedPipe\\LogAPI");

	m_bINetBlocked = GetTextList("ClosedFilePath").contains("InternetAccessDevices");

	m_bSharesAllowed = GetBool("BlockNetworkFiles", true) == false;

	m_bDropRights = GetBool("DropAdminRights", false);

	if (CheckOpenToken())
		m_iUnsecureDebugging = 1;
	else if(GetBool("ExposeBoxedSystem", false) || GetBool("UnrestrictedSCM", false))
		m_iUnsecureDebugging = 2;
	else
		m_iUnsecureDebugging = 0;

	//GetBool("SandboxieLogon", false)

	m_bSecurityRestricted = m_iUnsecureDebugging == 0 && (GetBool("DropAdminRights", false) || GetBool("ProtectRpcSs", false) || !GetBool("OpenDefaultClsid", true));

	CSandBox::UpdateDetails();
}

QString CSandBoxPlus::GetStatusStr() const
{
	QStringList Status;

	if (m_iUnsecureDebugging == 1)
		Status.append(tr("NOT SECURE (Debug Config)"));
	else if (m_iUnsecureDebugging == 2)
		Status.append(tr("Reduced Isolation"));
	else if(m_bSecurityRestricted)
		Status.append(tr("Enhanced Isolation"));

	if (m_bLogApiFound)
		Status.append(tr("API Log"));
	if (m_bINetBlocked)
		Status.append(tr("No INet"));
	if (m_bSharesAllowed)
		Status.append(tr("Net Share"));
	if (m_bDropRights)
		Status.append(tr("No Admin"));

	if (Status.isEmpty())
		return tr("Normal");
	return Status.join(", ");
}

bool CSandBoxPlus::CheckOpenToken() const
{
	if (GetBool("OpenToken", false)) return true;
		if(GetBool("UnrestrictedToken", false)) return true;
			if (!GetBool("AnonymousLogon", true)) return true;
			if (GetBool("KeepTokenIntegrity", false)) return true;
		if(GetBool("UnfilteredToken", false)) return true;
	return false;
}

void CSandBoxPlus::SetLogApi(bool bEnable)
{
	if (bEnable)
	{
		InsertText("OpenPipePath", "\\Device\\NamedPipe\\LogAPI");
		InsertText("InjectDll", "\\LogAPI\\logapi32.dll");
		InsertText("InjectDll64", "\\LogAPI\\logapi64.dll");
	}
	else
	{
		DelValue("OpenPipePath", "\\Device\\NamedPipe\\LogAPI");
		DelValue("InjectDll", "\\LogAPI\\logapi32.dll");
		DelValue("InjectDll64", "\\LogAPI\\logapi64.dll");
	}
}

void CSandBoxPlus::SetINetBlock(bool bEnable)
{
	if (bEnable)
		DelValue("ClosedFilePath", "!<InternetAccess>,InternetAccessDevices");
	else
		InsertText("ClosedFilePath", "InternetAccessDevices");
}

void CSandBoxPlus::SetAllowShares(bool bEnable)
{
	SetBool("BlockNetworkFiles", !bEnable);
}

void CSandBoxPlus::SetDropRights(bool bEnable)
{
	SetBool("DropAdminRights", bEnable);
}
