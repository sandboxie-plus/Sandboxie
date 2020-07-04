#include "stdafx.h"
#include "SbieUtils.h"
#include <QCoreApplication>

#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

#include <windows.h>
#include "SbieDefs.h"

#include "..\..\Sandboxie\common\win32_ntddk.h"

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
	return SB_ERR("OK", ERROR_OK);
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
	if ((Component & eDriver) != 0 && GetServiceStatus(SBIEDRV) != SERVICE_RUNNING)
		Ops.append(QString::fromWCharArray(L"kmdutil.exe|start|" SBIEDRV));
	if ((Component & eService) != 0 && GetServiceStatus(SBIESVC) != SERVICE_RUNNING)
		Ops.append(QString::fromWCharArray(L"kmdutil.exe|start|" SBIESVC));
}

SB_STATUS CSbieUtils::Stop(EComponent Component)
{
	QStringList Ops;
	Stop(Component, Ops);
	return ElevateOps(Ops);
}

void CSbieUtils::Stop(EComponent Component, QStringList& Ops)
{
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
	if ((Component & eDriver) != 0 && GetServiceStatus(SBIEDRV) == 0)
		Ops.append(QString::fromWCharArray(L"kmdutil.exe|install|" SBIEDRV L"|") + "\"" + HomePath + "\\" + QString::fromWCharArray(SBIEDRV_SYS) + "\"" + "|type=kernel|start=demand|altitude=86900");
	if ((Component & eService) != 0 && GetServiceStatus(SBIESVC) == 0) {
		Ops.append(QString::fromWCharArray(L"kmdutil.exe|install|" SBIESVC L"|") + "\"" + HomePath + "\\" + QString::fromWCharArray(SBIESVC_EXE) + "\"" + "|type=own|start=auto|display=\"Sandboxie Service\"|group=UIGroup");
		Ops.append("reg.exe|ADD|HKLM\\SYSTEM\\ControlSet001\\Services\\SbieSvc|/v|PreferExternalManifest|/t|REG_DWORD|/d|1");
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
		return SB_ERR("Admin rights required");
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
			return SB_ERR("Failed to execute: " + Args.join(" "));
	}
	return SB_OK;
}