#include "stdafx.h"
#include "SbieUtils.h"
#include <QCoreApplication>

#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

#include <windows.h>
#include <Shlobj.h>
#include "SbieDefs.h"

#include "..\..\Sandboxie\common\win32_ntddk.h"

#include "SbieAPI.h"

int GetServiceStatus(const wchar_t* name)
{
	SC_HANDLE scm = OpenSCManager(nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE);
	if (!scm)
		return -1;

	SC_HANDLE service = OpenService(scm, name, SERVICE_QUERY_STATUS);
	if (!service) {
		CloseServiceHandle(scm);
		return 0;
	}

	DWORD dwBytesNeeded;
	SERVICE_STATUS_PROCESS sStatus;
	BOOL result = QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, (LPBYTE)&sStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded);

	CloseServiceHandle(service);

	CloseServiceHandle(scm);

	if (result == 0)
		return -1;
	return sStatus.dwCurrentState;
}

bool IsProcessElevatd()
{
	ULONG returnLength;
	TOKEN_ELEVATION elevation;
	if (NT_SUCCESS(NtQueryInformationToken(/*NtCurrentProcessToken()*/((HANDLE)(LONG_PTR)-4), TokenElevation, &elevation, sizeof(TOKEN_ELEVATION), &returnLength)))
		return !!elevation.TokenIsElevated;
	return false;
}

SB_STATUS CSbieUtils::DoAssist()
{
	QStringList Args = QCoreApplication::arguments();
	int AssistPos = Args.indexOf("-assist");
	if(AssistPos == -1)
		return SB_OK;

	SB_STATUS Status = ExecOps(Args.mid(AssistPos + 1));
	if (Status.IsError())
		return Status;
	return SB_ERR(ERROR_OK);
}

SB_STATUS CSbieUtils::Start(EComponent Component)
{
	QStringList Ops;
	if(!IsInstalled(Component))
		Install(Component, Ops);
	Start(Component, Ops);
	return ElevateOps(Ops);
}

void CSbieUtils::Start(EComponent Component, QStringList& Ops)
{
	// Note: Service aways starts the driver
	if ((Component & eService) != 0 && GetServiceStatus(SBIESVC) != SERVICE_RUNNING)
		Ops.append(QString::fromWCharArray(L"kmdutil.exe|start|" SBIESVC));
	else if ((Component & eDriver) != 0 && GetServiceStatus(SBIEDRV) != SERVICE_RUNNING)
		Ops.append(QString::fromWCharArray(L"kmdutil.exe|start|" SBIEDRV));
}

SB_STATUS CSbieUtils::Stop(EComponent Component)
{
	QStringList Ops;
	Stop(Component, Ops);
	return ElevateOps(Ops);
}

void CSbieUtils::Stop(EComponent Component, QStringList& Ops)
{
	Ops.append(QString::fromWCharArray(L"kmdutil.exe|scandll"));
	if ((Component & eService) != 0 && GetServiceStatus(SBIESVC) != SERVICE_STOPPED)
		Ops.append(QString::fromWCharArray(L"kmdutil.exe|stop|" SBIESVC));
	if ((Component & eDriver) != 0 && GetServiceStatus(SBIEDRV) != SERVICE_STOPPED)
		Ops.append(QString::fromWCharArray(L"kmdutil.exe|stop|" SBIEDRV));
}

bool CSbieUtils::IsRunning(EComponent Component)
{
	if ((Component & eDriver) != 0 && GetServiceStatus(SBIEDRV) != SERVICE_RUNNING)
		return false;
	if ((Component & eService) != 0 && GetServiceStatus(SBIESVC) != SERVICE_RUNNING)
		return false;
	return true;
}

SB_STATUS CSbieUtils::Install(EComponent Component)
{
	QStringList Ops;
	Install(Component, Ops);
	return ElevateOps(Ops);
}

void CSbieUtils::Install(EComponent Component, QStringList& Ops)
{
	QString HomePath = QCoreApplication::applicationDirPath().replace("/", "\\"); // "C:\\Program Files\\Sandboxie	"
	if ((Component & eDriver) != 0 && GetServiceStatus(SBIEDRV) == 0) // todo: why when we are admin we need \??\ and else not and why knd util from console as admin also does not need that???
		Ops.append(QString::fromWCharArray(L"kmdutil.exe|install|" SBIEDRV L"|") + "\"\\??\\" + HomePath + "\\" + QString::fromWCharArray(SBIEDRV_SYS) + "\"" + "|type=kernel|start=demand|altitude=86900");
	if ((Component & eService) != 0 && GetServiceStatus(SBIESVC) == 0) {
		Ops.append(QString::fromWCharArray(L"kmdutil.exe|install|" SBIESVC L"|") + "\"" + HomePath + "\\" + QString::fromWCharArray(SBIESVC_EXE) + "\"" + "|type=own|start=auto|display=\"Sandboxie Service\"|group=UIGroup");
		Ops.append("reg.exe|ADD|HKLM\\SYSTEM\\ControlSet001\\Services\\SbieSvc|/v|PreferExternalManifest|/t|REG_DWORD|/d|1|/f");
	}
}

SB_STATUS CSbieUtils::Uninstall(EComponent Component)
{
	QStringList Ops;
	Stop(Component, Ops);
	Uninstall(Component, Ops);
	return ElevateOps(Ops);
}

void CSbieUtils::Uninstall(EComponent Component, QStringList& Ops)
{
	if ((Component & eService) != 0 && GetServiceStatus(SBIESVC) != 0)
		Ops.append(QString::fromWCharArray(L"kmdutil.exe|delete|" SBIESVC));
	if ((Component & eDriver) != 0 && GetServiceStatus(SBIEDRV) != 0)
		Ops.append(QString::fromWCharArray(L"kmdutil.exe|delete|" SBIEDRV));
}

bool CSbieUtils::IsInstalled(EComponent Component)
{
	if ((Component & eDriver) != 0 && GetServiceStatus(SBIEDRV) == 0)
		return false;
	if ((Component & eService) != 0 && GetServiceStatus(SBIESVC) == 0)
		return false;
	return true;
}

SB_STATUS CSbieUtils::ElevateOps(const QStringList& Ops)
{
	if (Ops.isEmpty())
		return SB_OK;

	if (IsProcessElevatd())
		return ExecOps(Ops);

	wstring path = QCoreApplication::applicationFilePath().toStdWString();
	wstring params = L"-assist " + Ops.join(" ").toStdWString();

	SHELLEXECUTEINFO shex;
	memset(&shex, 0, sizeof(SHELLEXECUTEINFO));
	shex.cbSize = sizeof(SHELLEXECUTEINFO);
	shex.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
	shex.hwnd = NULL;
	shex.lpFile = path.c_str();
	shex.lpParameters = params.c_str();
	shex.nShow = SW_SHOWNORMAL;
	shex.lpVerb = L"runas";

	if (!ShellExecuteEx(&shex))
		return SB_ERR(SB_NeedAdmin);
	return SB_ERR(OP_ASYNC);
}

SB_STATUS CSbieUtils::ExecOps(const QStringList& Ops)
{
	foreach(const QString& Op, Ops)
	{
		QStringList Args = Op.split("|");
		QString Prog = Args.takeFirst();

		QProcess Proc;
		Proc.execute(Prog, Args);
		Proc.waitForFinished();
		int ret = Proc.exitCode();
		if (ret != 0)
			return SB_ERR(SB_ExecFail, QVariantList() << Args.join(" "));
	}
	return SB_OK;
}

//////////////////////////////////////////////////////////////////////////////
// Shell integration

int CSbieUtils::IsContextMenu()
{
	if (!CheckRegValue(L"Software\\Classes\\*\\shell\\sandbox\\command"))
		return 0;
	if (!CheckRegValue(L"software\\classes\\folder\\shell\\sandbox\\command"))
		return 1;
	return 2;
}

bool CSbieUtils::CheckRegValue(const wchar_t* key)
{
	HKEY hkey;
	LONG rc = RegOpenKeyEx(HKEY_CURRENT_USER, key, 0, KEY_READ, &hkey);
	if (rc != 0)
		return false;

	ULONG type;
	WCHAR path[512];
	ULONG path_len = sizeof(path) - sizeof(WCHAR) * 4;
	rc = RegQueryValueEx(hkey, NULL, NULL, &type, (BYTE *)path, &path_len);
	RegCloseKey(hkey);
	if (rc != 0)
		return false;

	return true;
}

void CSbieUtils::AddContextMenu(const QString& StartPath)
{
	wstring start_path = L"\"" + StartPath.toStdWString() + L"\"";

	CreateShellEntry(L"*", L"Run &Sandboxed", start_path, start_path + L" /box:__ask__ \"%1\" %*");

	wstring explorer_path(512, L'\0');

	HKEY hkeyWinlogon;
	LONG rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"software\\microsoft\\windows nt\\currentversion\\winlogon", 0, KEY_READ, &hkeyWinlogon);
	if (rc == 0)
	{
		ULONG path_len = explorer_path.size() * sizeof(WCHAR);
		ULONG type;
		rc = RegQueryValueEx(hkeyWinlogon, L"Shell", NULL, &type, (BYTE *)explorer_path.c_str(), &path_len);
		if (rc == 0 && (type == REG_SZ || type == REG_EXPAND_SZ))
			explorer_path.resize(path_len / sizeof(WCHAR));
		RegCloseKey(hkeyWinlogon);
	}

	// get default explorer path
	if (*explorer_path.c_str() == L'\0' || _wcsicmp(explorer_path.c_str(), L"explorer.exe") == 0)
	{
		GetWindowsDirectory((wchar_t*)explorer_path.c_str(), MAX_PATH);
		ULONG path_len = wcslen(explorer_path.c_str());
		explorer_path.resize(path_len);
		explorer_path.append(L"\\explorer.exe");
	}

	CreateShellEntry(L"Folder", L"Explore &Sandboxed", start_path, start_path + L" /box:__ask__ " + explorer_path + L" \"%1\"");
}

void CSbieUtils::CreateShellEntry(const wstring& classname, const wstring& cmdtext, const wstring& iconpath, const wstring& startcmd)
{
	HKEY hkey;
	LONG rc = RegCreateKeyEx(HKEY_CURRENT_USER, (L"software\\classes\\" + classname + L"\\shell\\sandbox").c_str(), 0, NULL, 0, KEY_WRITE, NULL, &hkey, NULL);
	if (rc != 0)
		return;

	RegSetValueEx(hkey, NULL, 0, REG_SZ, (BYTE *)cmdtext.c_str(), (cmdtext.length() + 1) * sizeof(WCHAR));
	RegSetValueEx(hkey, L"Icon", 0, REG_SZ, (BYTE *)iconpath.c_str(), (iconpath.length() + 1) * sizeof(WCHAR));

	RegCloseKey(hkey);
	if (rc != 0)
		return;

	rc = RegCreateKeyEx(HKEY_CURRENT_USER, (L"software\\classes\\" + classname + L"\\shell\\sandbox\\command").c_str(), 0, NULL, 0, KEY_WRITE, NULL, &hkey, NULL);
	if (rc != 0)
		return;

	RegSetValueEx(hkey, NULL, 0, REG_SZ, (BYTE *)startcmd.c_str(), (startcmd.length() + 1) * sizeof(WCHAR));

	RegCloseKey(hkey);
}

void CSbieUtils::RemoveContextMenu()
{
	RegDeleteTreeW(HKEY_CURRENT_USER, L"software\\classes\\*\\shell\\sandbox");
	RegDeleteTreeW(HKEY_CURRENT_USER, L"software\\classes\\folder\\shell\\sandbox");
}

//////////////////////////////////////////////////////////////////////////////
// Shortcuts

bool CSbieUtils::CreateShortcut(CSbieAPI* pApi, const QString &LinkPath, const QString &LinkName, const QString &boxname, const QString &arguments, const QString &iconPath, int iconIndex, const QString &workdir, bool bRunElevated)
{
	QString StartExe = pApi->GetStartPath();

	QString StartArgs;
	if (bRunElevated)
		StartArgs += "/elevated ";
	StartArgs += "/box:" + boxname;
	if (!arguments.isEmpty())
		StartArgs += " \"" + arguments + "\"";

	IUnknown *pUnknown;
	HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC, IID_IUnknown, (void **)&pUnknown);
	if (FAILED(hr))
		return false;

	IShellLink *pShellLink;
	hr = pUnknown->QueryInterface(IID_IShellLink, (void **)&pShellLink);
	if (SUCCEEDED(hr)) 
	{			
		pShellLink->SetPath(StartExe.toStdWString().c_str());
		pShellLink->SetArguments(StartArgs.toStdWString().c_str());
		if (!iconPath.isEmpty())
			pShellLink->SetIconLocation(iconPath.toStdWString().c_str(), iconIndex);
		if (!workdir.isEmpty())
			pShellLink->SetWorkingDirectory(workdir.toStdWString().c_str());
		if (!LinkName.isEmpty()) {
			QString desc = QString("%1 [%2]").arg(LinkName).arg(boxname);
			pShellLink->SetDescription(desc.toStdWString().c_str());
		}

		IPersistFile *pPersistFile;
		hr = pUnknown->QueryInterface(IID_IPersistFile, (void **)&pPersistFile);
		if (SUCCEEDED(hr)) 
		{
			pPersistFile->Save(LinkPath.toStdWString().c_str(), FALSE);

			pPersistFile->Release();
		}

		pShellLink->Release();
	}

	pUnknown->Release();
	return (SUCCEEDED(hr));
}

bool CSbieUtils::GetStartMenuShortcut(CSbieAPI* pApi, QString &BoxName, QString &LinkPath, QString &IconPath, quint32& IconIndex, QString &WorkDir)
{
	WCHAR MapName[128];
	wsprintf(MapName, SANDBOXIE L"_StartMenu_WorkArea_%08X_%08X", GetCurrentProcessId(), GetTickCount());
	HANDLE hMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 8192, MapName);
	if (!hMapping)
		return false;

	WCHAR* buf = (WCHAR *)MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, 8192);
	if (!buf) {
		CloseHandle(hMapping);
		return false;
	}
	memset(buf, 0, 8192);
	

	QProcess Process;
	QString Command = "start_menu:" + QString::fromWCharArray(MapName);
	if (!LinkPath.isEmpty())
		Command += ":" + LinkPath;
	pApi->RunStart(BoxName, Command, &Process);
	//Process.waitForFinished(-1);
	while(Process.state() != QProcess::NotRunning)
		QCoreApplication::processEvents(); // keep UI responsive
	

	struct SLnk {
		WCHAR box_name[34];		//0
		WCHAR reserved[30];		//34
		WCHAR link_path[956];	//64
		ULONG IconIndex;		//1020
		WCHAR unused[2];		//1022
		WCHAR icon_path[1024];	//1024
		WCHAR work_dir[1024];	//2048
								//3072
	} *lnk = (SLnk*)buf;

	BoxName = QString::fromWCharArray(lnk->box_name, wcsnlen_s(lnk->box_name, sizeof(lnk->box_name)));
	LinkPath = QString::fromWCharArray(lnk->link_path, wcsnlen_s(lnk->link_path, sizeof(lnk->link_path)));
	if (*lnk->icon_path) {
		IconIndex = lnk->IconIndex;
		IconPath = QString::fromWCharArray(lnk->icon_path, wcsnlen_s(lnk->icon_path, sizeof(lnk->icon_path)));
	}
	if (*lnk->work_dir)	WorkDir = QString::fromWCharArray(lnk->work_dir, wcsnlen_s(lnk->work_dir, sizeof(lnk->work_dir)));
	else				WorkDir = LinkPath.left(LinkPath.lastIndexOf('\\'));

	UnmapViewOfFile(buf);
	CloseHandle(hMapping);

	if (BoxName.isEmpty() || LinkPath.isEmpty())
		return false;
	return true;
}