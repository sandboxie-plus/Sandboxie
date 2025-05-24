#include "stdafx.h"
#include "SbieUtils.h"
#include <QCoreApplication>
#include <QWinEventNotifier>

#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

#include <windows.h>
#include <Shlobj.h>
#include "SbieDefs.h"

#include "..\..\Sandboxie\common\win32_ntddk.h"
#include "..\..\Sandboxie\common\defines.h"

#include "SbieAPI.h"

template <typename T>
__forceinline bool charIsNull(const T* v) { return *v == 0; }
__forceinline bool charIsNull(const QChar* v) { return v->isNull(); }

template <typename T>
bool wildcmpex(const T* Wild, const T* Str)
{
	const T *cp = NULL, *mp = NULL;

	while (!charIsNull(Str) && (*Wild != '*'))
	{
		if ((*Wild != *Str) && (*Wild != '?'))
			return false;
		Wild++;
		Str++;
	}

	while (!charIsNull(Str))
	{
		if (*Wild == '*')
		{
			if (charIsNull(++Wild))
				return Str;
			mp = Wild;
			cp = Str + 1;
		}
		else if ((*Wild == *Str) || (*Wild == '?'))
		{
			Wild++;
			Str++;
		}
		else
		{
			Wild = mp;
			Str = cp++;
		}
	}

	while (*Wild == '*')
		Wild++;
	return charIsNull(Wild);
}

bool CSbieUtils::WildCompare(const QString& L, const QString& R)
{
	return wildcmpex(L.data(), R.data());
}

int GetServiceStatus(const wchar_t* name)
{
	SC_HANDLE scm = OpenSCManager(nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE);
	if (!scm)
		return -1;

	SC_HANDLE service = OpenServiceW(scm, name, SERVICE_QUERY_STATUS);
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

SB_RESULT(void*) CSbieUtils::Start(EComponent Component)
{
	QStringList Ops;
	if(!IsInstalled(Component))
		Install(Component, Ops);
	Start(Component, Ops);
	return ElevateOps(Ops);
}

void CSbieUtils::Start(EComponent Component, QStringList& Ops)
{
	// Note: Service always starts the driver
	if ((Component & eService) != 0 && GetServiceStatus(SBIESVC) != SERVICE_RUNNING)
		Ops.append(QString::fromWCharArray(L"kmdutil.exe|start|" SBIESVC));
	else if ((Component & eDriver) != 0 && GetServiceStatus(SBIEDRV) != SERVICE_RUNNING)
		Ops.append(QString::fromWCharArray(L"kmdutil.exe|start|" SBIEDRV));
}

SB_RESULT(void*) CSbieUtils::Stop(EComponent Component)
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

SB_RESULT(void*) CSbieUtils::Install(EComponent Component)
{
	QStringList Ops;
	Install(Component, Ops);
	return ElevateOps(Ops);
}

void CSbieUtils::Install(EComponent Component, QStringList& Ops)
{
	QString HomePath = QCoreApplication::applicationDirPath().replace("/", "\\"); // "C:\\Program Files\\Sandboxie	"
	if ((Component & eDriver) != 0 && GetServiceStatus(SBIEDRV) == 0) 
		Ops.append(QString::fromWCharArray(L"kmdutil.exe|install|" SBIEDRV L"|") + HomePath + "\\" + QString::fromWCharArray(SBIEDRV_SYS) + "|type=kernel|start=demand|altitude=86900");
	if ((Component & eService) != 0 && GetServiceStatus(SBIESVC) == 0) {
		Ops.append(QString::fromWCharArray(L"kmdutil.exe|install|" SBIESVC L"|") + HomePath + "\\" + QString::fromWCharArray(SBIESVC_EXE) + "|type=own|start=auto|display=\"Sandboxie Service\"|group=UIGroup");
		Ops.append("reg.exe|ADD|HKLM\\SYSTEM\\ControlSet001\\Services\\SbieSvc|/v|PreferExternalManifest|/t|REG_DWORD|/d|1|/f");
	}
}

SB_RESULT(void*) CSbieUtils::Uninstall(EComponent Component)
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

SB_RESULT(void*) CSbieUtils::ElevateOps(const QStringList& Ops)
{
	if (Ops.isEmpty())
		return SB_OK;

	if (IsProcessElevatd()) {
		SB_RESULT(void*) result = ExecOps(Ops);
		QThread::msleep(1000); // wait for the operation to finish properly
		return result;
	}

	std::wstring path = QCoreApplication::applicationFilePath().toStdWString();
	std::wstring params = L"-assist \"" + Ops.join("\" \"").toStdWString() + L"\"";

	SHELLEXECUTEINFOW shex;
	memset(&shex, 0, sizeof(shex));
	shex.cbSize = sizeof(shex);
	shex.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
	shex.hwnd = NULL;
	shex.lpFile = path.c_str();
	shex.lpParameters = params.c_str();
	shex.nShow = SW_SHOWNORMAL;
	shex.lpVerb = L"runas";

	if (!ShellExecuteExW(&shex))
		return SB_ERR(SB_NeedAdmin);
	return CSbieResult<void*>(OP_ASYNC, shex.hProcess);
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

CSbieProgressPtr CSbieUtils::RunCommand(const QString& Command, bool noGui)
{
	STARTUPINFOW si = { 0 };
	si.cb = sizeof(si);
	if (noGui) {	
		si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
		si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
		si.wShowWindow = SW_HIDE;
	}
	PROCESS_INFORMATION pi = { 0 };
	if (!CreateProcessW(NULL, (LPWSTR)Command.toStdWString().c_str(), NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi))
		return CSbieProgressPtr();
	
	HANDLE hJobObject = CreateJobObject(NULL, NULL);
	if (hJobObject)
		AssignProcessToJobObject(hJobObject, pi.hProcess);

	CSbieProgressPtr pProgress = CSbieProgressPtr(new CSbieProgress());

	QWinEventNotifier* processFinishedNotifier = new QWinEventNotifier(pi.hProcess);
	processFinishedNotifier->setEnabled(true);
	QObject::connect(processFinishedNotifier, &QWinEventNotifier::activated, [=]() {
		processFinishedNotifier->setEnabled(false);
		processFinishedNotifier->deleteLater();

		pProgress->Finish(SB_OK);

		if(hJobObject)
			CloseHandle(hJobObject);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	});

	QObject::connect(pProgress.data(), &CSbieProgress::Canceled, [=]() {
		if (hJobObject)
			TerminateJobObject(hJobObject, 0);
		else
			TerminateProcess(pi.hProcess, 0);
	});

	ResumeThread(pi.hThread);
	
	return pProgress;
}

int CSbieUtils::ExecCommand(const QString& Command, bool noGui, quint32 Timeout)
{
	STARTUPINFOW si = { 0 };
	si.cb = sizeof(si);
	if (noGui) {
		si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
		si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
		si.wShowWindow = SW_HIDE;
	}

	PROCESS_INFORMATION pi = { 0 };
	if (!CreateProcessW(NULL, (LPWSTR)Command.toStdWString().c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
		return -1;

	int iRet = -2;
	if (WaitForSingleObject(pi.hProcess, (DWORD)Timeout) == WAIT_OBJECT_0)
	{
		DWORD dwRet;
		if (GetExitCodeProcess(pi.hProcess, &dwRet))
			iRet = (int)dwRet;
	}

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	return iRet;
}

int CSbieUtils::ExecCommandEx(const QString& Command, QString* pOutput, quint32 Timeout)
{
	HANDLE stdoutReadHandle;
	HANDLE stdoutWriteHandle;
	SECURITY_ATTRIBUTES saAttr;
	// Set the bInheritHandle flag so pipe handles are inherited.
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = nullptr;
	// Create a pipe for the child process's STDOUT.
	if (!CreatePipe(&stdoutReadHandle, &stdoutWriteHandle, &saAttr, 0))
		return -4;
	if (!SetHandleInformation(stdoutReadHandle, HANDLE_FLAG_INHERIT, 0))
		return -4;

	STARTUPINFOW si = { 0 };
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	si.hStdOutput = stdoutWriteHandle;
	si.hStdError = stdoutWriteHandle;
	si.wShowWindow = SW_HIDE;

	PROCESS_INFORMATION pi = { 0 };
	if (!CreateProcessW(NULL, (LPWSTR)Command.toStdWString().c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
		return -1;
	
	DWORD exitCode, dataSize;
	do
	{
		// Check if the process is alive.
		GetExitCodeProcess(pi.hProcess, &exitCode);

		// Check if there is anything in the pipe.
		if (!PeekNamedPipe(stdoutReadHandle, nullptr, 0, nullptr, &dataSize, nullptr))
			return -3;
		if (dataSize == 0)
			Sleep(10);
		else {
			// Read the data out of the pipe.
			CHAR buffer[4096] = { 0 };
			if (!ReadFile(stdoutReadHandle, buffer, sizeof(buffer) - 1, &dataSize, nullptr))
				return -3;
				
			pOutput->append(QString(buffer));
		}
	} while (exitCode == STILL_ACTIVE || dataSize != 0);

	CloseHandle(stdoutReadHandle);
	CloseHandle(stdoutWriteHandle);

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	return (int)exitCode;
}

//////////////////////////////////////////////////////////////////////////////
// Shell integration

QString CSbieUtils::GetContextMenuStartCmd()
{
	const wchar_t* key = L"Software\\Classes\\*\\shell\\sandbox\\command";
	HKEY hkey;
	LONG rc = RegOpenKeyExW(HKEY_CURRENT_USER, key, 0, KEY_READ, &hkey);
	if (rc != 0)
		return QString();

	ULONG type;
	WCHAR path[512];
	ULONG path_len = sizeof(path) - sizeof(WCHAR) * 4;
	rc = RegQueryValueExW(hkey, NULL, NULL, &type, (BYTE *)path, &path_len);
	RegCloseKey(hkey);
	if (rc != 0)
		return QString();

	return QString::fromWCharArray(path);
}

void CSbieUtils::AddContextMenu(const QString& StartPath, const QString& RunStr, /*const QString& ExploreStr,*/ const QString& IconPath)
{
	std::wstring start_path = L"\"" + StartPath.toStdWString() + L"\"";
	std::wstring icon_path = L"\"" + (IconPath.isEmpty() ? StartPath : IconPath).toStdWString() + L"\"";

	CreateShellEntry(L"*", L"sandbox", RunStr.toStdWString(), icon_path, start_path + L" /box:__ask__ \"%1\" %*");

	std::wstring explorer_path(512, L'\0');

	HKEY hkeyWinlogon;
	LONG rc = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"software\\microsoft\\windows nt\\currentversion\\winlogon", 0, KEY_READ, &hkeyWinlogon);
	if (rc == 0)
	{
		ULONG path_len = explorer_path.size() * sizeof(WCHAR);
		ULONG type;
		rc = RegQueryValueExW(hkeyWinlogon, L"Shell", NULL, &type, (BYTE *)explorer_path.c_str(), &path_len);
		if (rc == 0 && (type == REG_SZ || type == REG_EXPAND_SZ))
			explorer_path.resize(path_len / sizeof(WCHAR));
		RegCloseKey(hkeyWinlogon);
	}

	// get default explorer path
	if (*explorer_path.c_str() == L'\0' || _wcsicmp(explorer_path.c_str(), L"explorer.exe") == 0)
	{
		GetWindowsDirectoryW((wchar_t*)explorer_path.c_str(), MAX_PATH);
		ULONG path_len = wcslen(explorer_path.c_str());
		explorer_path.resize(path_len);
		explorer_path.append(L"\\explorer.exe");
	}

	CreateShellEntry(L"Folder", L"sandbox", RunStr.toStdWString(), icon_path, start_path + L" /box:__ask__ " + explorer_path + L" \"%1\""); // ExploreStr
}

void CSbieUtils::CreateShellEntry(const std::wstring& classname, const std::wstring& key, const std::wstring& cmdtext, const std::wstring& iconpath, const std::wstring& startcmd)
{
	HKEY hkey;
	LONG rc = RegCreateKeyExW(HKEY_CURRENT_USER, (L"software\\classes\\" + classname + L"\\shell\\" + key).c_str(), 0, NULL, 0, KEY_WRITE, NULL, &hkey, NULL);
	if (rc != 0)
		return;

	RegSetValueExW(hkey, NULL, 0, REG_SZ, (BYTE *)cmdtext.c_str(), (cmdtext.length() + 1) * sizeof(WCHAR));
	RegSetValueExW(hkey, L"Icon", 0, REG_SZ, (BYTE *)iconpath.c_str(), (iconpath.length() + 1) * sizeof(WCHAR));

	RegCloseKey(hkey);
	if (rc != 0)
		return;

	rc = RegCreateKeyExW(HKEY_CURRENT_USER, (L"software\\classes\\" + classname + L"\\shell\\"  + key + L"\\command").c_str(), 0, NULL, 0, KEY_WRITE, NULL, &hkey, NULL);
	if (rc != 0)
		return;

	RegSetValueExW(hkey, NULL, 0, REG_SZ, (BYTE *)startcmd.c_str(), (startcmd.length() + 1) * sizeof(WCHAR));

	RegCloseKey(hkey);
}

void CSbieUtils::RemoveContextMenu()
{
	RegDeleteTreeW(HKEY_CURRENT_USER, L"software\\classes\\*\\shell\\sandbox");
	RegDeleteTreeW(HKEY_CURRENT_USER, L"software\\classes\\folder\\shell\\sandbox");
}

bool CSbieUtils::HasContextMenu2()
{
	const wchar_t* key = L"Software\\Classes\\*\\shell\\unbox\\command";
	HKEY hkey;
	LONG rc = RegOpenKeyExW(HKEY_CURRENT_USER, key, 0, KEY_READ, &hkey);
	if (rc != 0)
		return false;

	RegCloseKey(hkey);

	return true;
}

void CSbieUtils::AddContextMenu2(const QString& StartPath, const QString& RunStr, const QString& IconPath)
{
	std::wstring start_path = L"\"" + StartPath.toStdWString() + L"\"";
	std::wstring icon_path = L"\"" + (IconPath.isEmpty() ? StartPath : IconPath).toStdWString() + L"\",-104";

	CreateShellEntry(L"*", L"unbox", RunStr.toStdWString(), icon_path, start_path + L" /disable_force \"%1\" %*");
}

void CSbieUtils::RemoveContextMenu2()
{
	RegDeleteTreeW(HKEY_CURRENT_USER, L"software\\classes\\*\\shell\\unbox");
}

bool CSbieUtils::HasContextMenu3()
{
	const wchar_t* key = L"Software\\Classes\\*\\shell\\addforce\\command";
	//const wchar_t* key2 = L"Software\\Classes\\*\\Folder\\addforce\\command";
	HKEY hkey,hKey2;
	LONG rc = RegOpenKeyExW(HKEY_CURRENT_USER, key, 0, KEY_READ, &hkey);
	if (rc != 0)
		return false;

	RegCloseKey(hkey);


	/*rc = RegOpenKeyEx(HKEY_CURRENT_USER, key2, 0, KEY_READ, &hkey2);
	if (rc != 0)
		return false;

	RegCloseKey(hkey2);*/

	return true;
}

void CSbieUtils::AddContextMenu3(const QString& StartPath, const QString& RunStr, const QString& IconPath)
{
	std::wstring start_path = L"\"" + StartPath.toStdWString() + L"\"";
	std::wstring icon_path = L"\"" + (IconPath.isEmpty() ? StartPath : IconPath).toStdWString() + L"\"";

	CreateShellEntry(L"*", L"addforce", RunStr.toStdWString(), icon_path, start_path + L" /add_force \"%1\" %*");
	CreateShellEntry(L"Folder", L"addforce", RunStr.toStdWString(), icon_path, start_path + L" /add_force \"%1\" %*");
}

void CSbieUtils::RemoveContextMenu3()
{
	RegDeleteTreeW(HKEY_CURRENT_USER, L"software\\classes\\*\\shell\\addforce");
	RegDeleteTreeW(HKEY_CURRENT_USER, L"software\\classes\\folder\\shell\\addforce");
}


bool CSbieUtils::HasContextMenu4()
{
	const wchar_t* key = L"Software\\Classes\\*\\shell\\addopen\\command";
	//const wchar_t* key2 = L"Software\\Classes\\*\\Folder\\addforce\\command";
	HKEY hkey, hKey2;
	LONG rc = RegOpenKeyExW(HKEY_CURRENT_USER, key, 0, KEY_READ, &hkey);
	if (rc != 0)
		return false;

	RegCloseKey(hkey);


	/*rc = RegOpenKeyEx(HKEY_CURRENT_USER, key2, 0, KEY_READ, &hkey2);
	if (rc != 0)
		return false;

	RegCloseKey(hkey2);*/

	return true;
}

void CSbieUtils::AddContextMenu4(const QString& StartPath, const QString& RunStr, const QString& IconPath)
{
	std::wstring start_path = L"\"" + StartPath.toStdWString() + L"\"";
	std::wstring icon_path = L"\"" + (IconPath.isEmpty() ? StartPath : IconPath).toStdWString() + L"\"";

	CreateShellEntry(L"*", L"addopen", RunStr.toStdWString(), icon_path, start_path + L" /add_open \"%1\" %*");
	CreateShellEntry(L"Folder", L"addopen", RunStr.toStdWString(), icon_path, start_path + L" /add_open \"%1\" %*");
}

void CSbieUtils::RemoveContextMenu4()
{
	RegDeleteTreeW(HKEY_CURRENT_USER, L"software\\classes\\*\\shell\\addopen");
	RegDeleteTreeW(HKEY_CURRENT_USER, L"software\\classes\\folder\\shell\\addopen");
}

//////////////////////////////////////////////////////////////////////////////
// Shortcuts

bool CSbieUtils::CreateShortcut(const QString& StartExe, QString LinkPath, const QString &LinkName, const QString &boxname, const QString &arguments, const QString &iconPath, int iconIndex, const QString &workdir, bool bRunElevated)
{
	QString StartArgs;
	if (bRunElevated)
		StartArgs += "/elevated ";
	if (!boxname.isEmpty())
		StartArgs += "/box:" + boxname;
	if (!arguments.isEmpty()) {
		if (!StartArgs.isEmpty()) StartArgs += " ";
		if(arguments.contains(" ") && arguments.left(1) != "\"")
			StartArgs += "\"" + arguments + "\"";
		else
			StartArgs += arguments;
	}

	IUnknown *pUnknown;
	HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC, IID_IUnknown, (void **)&pUnknown);
	if (FAILED(hr))
		return false;

	IShellLinkW *pShellLink;
	hr = pUnknown->QueryInterface(IID_IShellLinkW, (void **)&pShellLink);
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
			if (LinkPath.right(4) != ".lnk")
				LinkPath.append(".lnk");

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
	wsprintfW(MapName, SANDBOXIE L"_StartMenu_WorkArea_%08X_%08X", GetCurrentProcessId(), GetTickCount());
	HANDLE hMapping = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 8192, MapName);
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
	pApi->RunStart(BoxName, Command, CSbieAPI::eStartDefault, QString(), &Process);
	//Process.waitForFinished(-1);
	while(Process.state() != QProcess::NotRunning)
		QCoreApplication::processEvents(); // keep UI responsive
	

	struct SLnk {
		WCHAR box_name[BOXNAME_COUNT];	//0
		WCHAR reserved[30];				//34
		WCHAR link_path[956];			//64
		ULONG IconIndex;				//1020
		WCHAR unused[2];				//1022
		WCHAR icon_path[1024];			//1024
		WCHAR work_dir[1024];			//2048
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


BOOLEAN SelectFavoriteInRegedit(HWND RegeditWindow, std::wstring FavoriteName)
{
    HMENU menu;
    HMENU favoritesMenu;
    ULONG count;
    ULONG i;
    ULONG id = ULONG_MAX;

	if (IsIconic(RegeditWindow)) {
        ShowWindow(RegeditWindow, SW_RESTORE);
        SetForegroundWindow(RegeditWindow);
    }
    else {
        SetForegroundWindow(RegeditWindow);
    }
	
	if (!(menu = GetMenu(RegeditWindow)))
        return FALSE;

	/*UINT scan = MapVirtualKey(VK_F5, 0);
	LPARAM lparam = 0x00000001 | (LPARAM)(scan << 16); 
	SendMessage(RegeditWindow, WM_KEYDOWN, VK_F5, lparam);
	SendMessage(RegeditWindow, WM_KEYUP, VK_F5, 0);*/
	SendMessage(RegeditWindow, WM_MENUSELECT, MAKEWPARAM(2, MF_POPUP), (LPARAM)menu);
    SendMessage(RegeditWindow, WM_COMMAND, MAKEWPARAM(0x288, 0), 0); // F5 menu entry
    PostMessage(RegeditWindow, WM_MENUSELECT, MAKEWPARAM(0, 0xffff), 0);


    SendMessage(RegeditWindow, WM_MENUSELECT, MAKEWPARAM(3, MF_POPUP), (LPARAM)menu);
    if (!(favoritesMenu = GetSubMenu(menu, 3)))
        return FALSE;

    count = GetMenuItemCount(favoritesMenu);
    if (count == -1) return FALSE;
    if (count > 1000) count = 1000;

    for (i = 3; i < count; i++) {

        MENUITEMINFOW info = { sizeof(MENUITEMINFO) };
        WCHAR buffer[MAX_PATH];

        info.fMask = MIIM_ID | MIIM_STRING;
        info.dwTypeData = buffer;
        info.cch = RTL_NUMBER_OF(buffer);
        GetMenuItemInfoW(favoritesMenu, i, TRUE, &info);

        if (info.cch > 0 && _wcsicmp(FavoriteName.c_str(),buffer) == 0) {
            id = info.wID;
            break;
        }
    }

    if (id == ULONG_MAX)
        return FALSE;

    SendMessage(RegeditWindow, WM_COMMAND, MAKEWPARAM(id, 0), 0);
    PostMessage(RegeditWindow, WM_MENUSELECT, MAKEWPARAM(0, 0xffff), 0);

    return TRUE;
}

bool ShellOpenRegKey(const QString& KeyName)
{
	std::wstring keyName = KeyName.toStdWString();

    HWND regeditWindow = FindWindowW(L"RegEdit_RegEdit", NULL);

    if (!regeditWindow)
    {
		RegSetKeyValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Regedit", L"LastKey", REG_SZ, keyName.c_str(), (keyName.size() + 1) * sizeof(WCHAR));

		SHELLEXECUTEINFOW sei = { sizeof(sei) };
		sei.fMask = 0;
		sei.lpVerb = NULL;
		sei.lpFile = L"regedit.exe";
		sei.lpParameters = NULL;
		sei.hwnd = NULL;
		sei.nShow = SW_NORMAL;
	
		ShellExecuteExW(&sei);

        return TRUE;
    }
	
	// for this to work we need elevated privileges !!!

	QString FavoriteName = "A_Sandbox_" + QString::number((quint32)rand(), 16);
	std::wstring favoriteName = FavoriteName.toStdWString();

	RegSetKeyValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Regedit\\Favorites", favoriteName.c_str(), REG_SZ, keyName.c_str(), (keyName.size() + 1) * sizeof(WCHAR));

    BOOLEAN result = SelectFavoriteInRegedit(regeditWindow, favoriteName);

	HKEY key;
	RegOpenKeyW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Regedit\\Favorites", &key);
	RegDeleteValueW(key, favoriteName.c_str());
	RegCloseKey(key);

    return result;
}

