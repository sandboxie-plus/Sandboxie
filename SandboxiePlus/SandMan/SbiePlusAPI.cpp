#include "stdafx.h"
#include "SbiePlusAPI.h"
#include "..\MiscHelpers\Common\Common.h"
#include <windows.h>

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

CBoxedProcess* CSbiePlusAPI::NewBoxedProcess(quint32 ProcessId, class CSandBox* pBox)
{
	return new CSbieProcess(ProcessId, pBox);
}

CBoxedProcessPtr CSbiePlusAPI::OnProcessBoxed(quint32 ProcessId, const QString& Path, const QString& Box, quint32 ParentId)
{
	CBoxedProcessPtr pProcess = CSbieAPI::OnProcessBoxed(ProcessId, Path, Box, ParentId);
	if (!pProcess.isNull() && pProcess->GetFileName().indexOf(theAPI->GetSbiePath(), 0, Qt::CaseInsensitive) != 0) {
		CSandBoxPlus* pBox = pProcess.objectCast<CSbieProcess>()->GetBox();
		pBox->m_RecentPrograms.insert(pProcess->GetProcessName());
	}
	return pProcess;
}

BOOL CALLBACK CSbiePlusAPI__WindowEnum(HWND hwnd, LPARAM lParam)
{
	if (GetParent(hwnd) || GetWindow(hwnd, GW_OWNER))
		return TRUE;
	ULONG style = GetWindowLong(hwnd, GWL_STYLE);
	if ((style & (WS_CAPTION | WS_SYSMENU)) != (WS_CAPTION | WS_SYSMENU))
		return TRUE;
	if (!IsWindowVisible(hwnd))
		return TRUE;
	/*
	if ((style & WS_OVERLAPPEDWINDOW) != WS_OVERLAPPEDWINDOW &&
		(style & WS_POPUPWINDOW)      != WS_POPUPWINDOW)
		return TRUE;
	*/

	ULONG pid;
	GetWindowThreadProcessId(hwnd, &pid);

	QMultiMap<quint32, QString>& m_WindowMap = *((QMultiMap<quint32, QString>*)(lParam));

	WCHAR title[256];
	GetWindowTextW(hwnd, title, 256);

	m_WindowMap.insert(pid, QString::fromWCharArray(title));

	return TRUE;
}

void CSbiePlusAPI::UpdateWindowMap()
{
	m_WindowMap.clear();
	EnumWindows(CSbiePlusAPI__WindowEnum, (LPARAM)&m_WindowMap);
}

///////////////////////////////////////////////////////////////////////////////
// CSandBoxPlus
//

CSandBoxPlus::CSandBoxPlus(const QString& BoxName, class CSbieAPI* pAPI) : CSandBox(BoxName, pAPI)
{
	m_bLogApiFound = false;
	m_bINetBlocked = false;
	m_bSharesAllowed = false;
	m_bDropRights = false;
	

	m_bSecurityRestricted = false;
	m_iUnsecureDebugging = 0;

	m_SuspendRecovery = false;
}

CSandBoxPlus::~CSandBoxPlus()
{
}

void CSandBoxPlus::UpdateDetails()
{
	//m_bLogApiFound = GetTextList("OpenPipePath", false).contains("\\Device\\NamedPipe\\LogAPI");
	m_bLogApiFound = false;
	QStringList InjectDlls = GetTextList("InjectDll", false);
	foreach(const QString & InjectDll, InjectDlls)
	{
		if (InjectDll.contains("logapi", Qt::CaseInsensitive)) {
			m_bLogApiFound = true;
			break;
		}
	}

	m_bINetBlocked = false;
	foreach(const QString& Entry, GetTextList("ClosedFilePath", false))
	{
		if (Entry.contains("InternetAccessDevices")) {
			m_bINetBlocked = true;
			break;
		}
	}

	m_bSharesAllowed = GetBool("BlockNetworkFiles", true) == false;

	m_bDropRights = GetBool("DropAdminRights", false);

	if (CheckOpenToken() || GetBool("StripSystemPrivileges", false))
		m_iUnsecureDebugging = 1;
	else if(GetBool("ExposeBoxedSystem", false) || GetBool("UnrestrictedSCM", false) /*|| GetBool("RunServicesAsSystem", false)*/)
		m_iUnsecureDebugging = 2;
	else
		m_iUnsecureDebugging = 0;

	//GetBool("SandboxieLogon", false)

	m_bSecurityRestricted = m_iUnsecureDebugging == 0 && (GetBool("DropAdminRights", false));

	CSandBox::UpdateDetails();
}

void CSandBoxPlus::CloseBox()
{
	CSandBox::CloseBox();

	m_SuspendRecovery = false;
}

QString CSandBoxPlus::GetStatusStr() const
{
	if (!m_IsEnabled)
		return tr("Disabled");

	QStringList Status;

	if (IsEmpty())
		Status.append(tr("Empty"));

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
	if (GetBool("OriginalToken", false)) return true;
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
		//InsertText("OpenPipePath", "\\Device\\NamedPipe\\LogAPI");
		InsertText("InjectDll", "\\LogAPI\\logapi32.dll");
		InsertText("InjectDll64", "\\LogAPI\\logapi64.dll");
	}
	else
	{
		//DelValue("OpenPipePath", "\\Device\\NamedPipe\\LogAPI");
		DelValue("InjectDll", "\\LogAPI\\logapi32.dll");
		DelValue("InjectDll64", "\\LogAPI\\logapi64.dll");
	}
	m_bLogApiFound = bEnable;
}

void CSandBoxPlus::SetINetBlock(bool bEnable)
{
	if (bEnable)
		InsertText("ClosedFilePath", "!<InternetAccess>,InternetAccessDevices");
	else
	{
		foreach(const QString& Entry, GetTextList("ClosedFilePath", false))
		{
			if (Entry.contains("InternetAccessDevices"))
				DelValue("ClosedFilePath", Entry);
		}
	}
}

void CSandBoxPlus::SetAllowShares(bool bEnable)
{
	SetBool("BlockNetworkFiles", !bEnable);
}

void CSandBoxPlus::SetDropRights(bool bEnable)
{
	SetBool("DropAdminRights", bEnable);
}

QStringList::iterator FindInStrList(QStringList& list, const QString& str)
{
	QStringList::iterator J = list.begin();
	for (; J != list.end(); ++J)
	{
		if (J->compare(str, Qt::CaseInsensitive) == 0)
			break;
	}
	return J;
}

void SetInStrList(QStringList& list, const QString& str, bool bSet)
{
	if (bSet)
		list.append(str);
	else
	{
		for (QStringList::iterator J = list.begin(); J != list.end();)
		{
			if (J->compare(str, Qt::CaseInsensitive) == 0) {
				J = list.erase(J);
				continue;
			}
			++J;
		}
	}
}

bool CSandBoxPlus::TestProgramGroup(const QString& Group, const QString& ProgName)
{
	QStringList ProcessGroups = GetTextList("ProcessGroup", false);
	foreach(const QString & ProcessGroup, ProcessGroups)
	{
		StrPair GroupPaths = Split2(ProcessGroup, ",");
		if (GroupPaths.first.compare(Group, Qt::CaseInsensitive) == 0)
		{
			QStringList Programs = SplitStr(GroupPaths.second, ",");
			return FindInStrList(Programs, ProgName) != Programs.end();
		}
	}
	return false;
}

void CSandBoxPlus::EditProgramGroup(const QString& Group, const QString& ProgName, bool bSet)
{
	QStringList ProcessGroups = GetTextList("ProcessGroup", false);

	QStringList Programs;
	QStringList::iterator I = ProcessGroups.begin();
	for (; I != ProcessGroups.end(); ++I)
	{
		StrPair GroupPaths = Split2(*I, ",");
		if (GroupPaths.first.compare(Group, Qt::CaseInsensitive) == 0)
		{
			Programs = SplitStr(GroupPaths.second, ",");
			break;
		}
	}
	if (I == ProcessGroups.end())
		I = ProcessGroups.insert(I, "");

	SetInStrList(Programs, ProgName, bSet);

	*I = Group + "," + Programs.join(",");

	UpdateTextList("ProcessGroup", ProcessGroups, false);
}

void CSandBoxPlus::BlockProgram(const QString& ProgName)
{
	bool WhiteList = false;
	bool BlackList = false;
	foreach(const QString& Entry, GetTextList("ClosedIpcPath", false))
	{
		StrPair ProgPath = Split2(Entry, ",");
		if (ProgPath.second != "*")
			continue;
		if (ProgPath.first.compare("!<StartRunAccess>", Qt::CaseInsensitive) == 0)
			WhiteList = true;
		else if (ProgPath.first.compare("<StartRunAccess>", Qt::CaseInsensitive) == 0)
			BlackList = true;
		else
			continue;
		break;
	}

	if (!WhiteList && !BlackList)
	{
		BlackList = true;
		InsertText("ClosedIpcPath", "<StartRunAccess>,*");
	}

	EditProgramGroup("<StartRunAccess>", ProgName, !WhiteList);
}

void CSandBoxPlus::SetInternetAccess(const QString& ProgName, bool bSet)
{
	EditProgramGroup("<InternetAccess>", ProgName, bSet);
}

bool CSandBoxPlus::HasInternetAccess(const QString& ProgName)
{
	return TestProgramGroup("<InternetAccess>", ProgName);
}

void CSandBoxPlus::SetForcedProgram(const QString& ProgName, bool bSet)
{
	QStringList Programs = GetTextList("ForceProcess", false);
	SetInStrList(Programs, ProgName, bSet);
	UpdateTextList("ForceProcess", Programs, false);
}

bool CSandBoxPlus::IsForcedProgram(const QString& ProgName)
{
	QStringList Programs = GetTextList("ForceProcess", false);
	return FindInStrList(Programs, ProgName) != Programs.end();
}

void CSandBoxPlus::SetLingeringProgram(const QString& ProgName, bool bSet)
{
	QStringList Programs = GetTextList("LingerProcess", false);
	SetInStrList(Programs, ProgName, bSet);
	UpdateTextList("LingerProcess", Programs, false);
}

int CSandBoxPlus::IsLingeringProgram(const QString& ProgName)
{
	QStringList Programs = GetTextList("LingerProcess", false);
	if (FindInStrList(Programs, ProgName) != Programs.end())
		return 1;
	Programs = GetTextList("LingerProcess", true);
	if (FindInStrList(Programs, ProgName) != Programs.end())
		return 2;
	return 0;
}

void CSandBoxPlus::SetLeaderProgram(const QString& ProgName, bool bSet)
{
	QStringList Programs = GetTextList("LeaderProcess", false);
	SetInStrList(Programs, ProgName, bSet);
	UpdateTextList("LeaderProcess", Programs, false);
}

int	CSandBoxPlus::IsLeaderProgram(const QString& ProgName)
{
	QStringList Programs = GetTextList("LeaderProcess", false);
	return FindInStrList(Programs, ProgName) != Programs.end() ? 1 : 0; 
}

///////////////////////////////////////////////////////////////////////////////
// CSbieProcess
//

QString CSbieProcess::GetStatusStr() const
{
	if (m_uTerminated != 0)
		return tr("Terminated");
	//if (m_bSuspended)
	//	return tr("Suspended");
	return tr("Running");
}
