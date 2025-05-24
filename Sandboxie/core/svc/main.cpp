/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 *
 * This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

//---------------------------------------------------------------------------
// Sandboxie User-Mode Server Service
//---------------------------------------------------------------------------

#include "stdafx.h"

#include <Sddl.h>
#include <lmcons.h>
#include "MountManager.h"
#include "DriverAssist.h"
#include "PipeServer.h"
#include "GuiServer.h"
#include "UserServer.h"
#include "ProcessServer.h"
#include "sbieiniserver.h"
#include "serviceserver.h"
#include "pstoreserver.h"
#include "terminalserver.h"
#include "namedpipeserver.h"
#include "fileserver.h"
#include "comserver.h"
#include "iphlpserver.h"
#include "netapiserver.h"
#include "queueserver.h"
#include "EpMapperServer.h"
#include "misc.h"
#include "core/dll/sbiedll.h"
#include "common/my_version.h"
#include "common/defines.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


void WINAPI ServiceMain(DWORD argc, WCHAR *argv[]);
DWORD InitializeEventLog(void);
DWORD InitializePipe(void);
DWORD WINAPI ServiceHandlerEx(
    DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static WCHAR                *ServiceName = SBIESVC;
static SERVICE_STATUS        ServiceStatus;
static SERVICE_STATUS_HANDLE ServiceStatusHandle = NULL;

static HANDLE                EventLog = NULL;

static ComServer            *pComServer = NULL;

extern "C" {
const  ULONG                 tzuk = 'xobs';
}

       HMODULE               _Ntdll = NULL;
       HMODULE               _Kernel32 = NULL;

       SYSTEM_INFO           _SystemInfo;

#ifdef _M_ARM64
       BOOLEAN               DisableCHPE = FALSE;
#endif

//---------------------------------------------------------------------------
// WinMain
//---------------------------------------------------------------------------


int WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    SERVICE_TABLE_ENTRY myServiceTable[] = {
        { ServiceName, ServiceMain },
        { NULL, NULL }
    };

    _Ntdll      = GetModuleHandle(L"ntdll.dll");
    _Kernel32   = GetModuleHandle(L"kernel32.dll");
    GetSystemInfo(&_SystemInfo);

    DriverAssist::InitializeSidCache();

    WCHAR *cmdline = GetCommandLine();
    if (cmdline) {

        WCHAR *cmdline2 = wcsstr(cmdline, SANDBOXIE L"_ComProxy");
        if (cmdline2) {
            ComServer::RunSlave(cmdline2);
            return NO_ERROR;
        }

        WCHAR *cmdline3 = wcsstr(cmdline, SANDBOXIE L"_UacProxy");
        if (cmdline3) {
            ServiceServer::RunUacSlave(cmdline3);
            return NO_ERROR;
        }

        WCHAR *cmdline4 = wcsstr(cmdline, SANDBOXIE L"_NetProxy");
        if (cmdline4) {
            NetApiServer::RunSlave(cmdline4);
            return NO_ERROR;
        }

        WCHAR *cmdline5 = wcsstr(cmdline, SANDBOXIE L"_GuiProxy");
        if (cmdline5) {
            GuiServer::RunSlave(cmdline5);
            return NO_ERROR;
        }

        WCHAR *cmdline6 = wcsstr(cmdline, SANDBOXIE L"_UserProxy");
        if (cmdline6) {
            UserServer::RunWorker(cmdline6);
            return NO_ERROR;
        }

    }

    if (! StartServiceCtrlDispatcher(myServiceTable))
        return GetLastError();

    DriverAssist::DestroySidCache();

    return NO_ERROR;
}


//---------------------------------------------------------------------------
// ServiceMain
//---------------------------------------------------------------------------


void WINAPI ServiceMain(DWORD argc, WCHAR *argv[])
{
    ServiceStatusHandle = RegisterServiceCtrlHandlerEx(
        ServiceName, ServiceHandlerEx, NULL);
    if (! ServiceStatusHandle)
        return;

    ServiceStatus.dwServiceType                 = SERVICE_WIN32;
    ServiceStatus.dwCurrentState                = SERVICE_START_PENDING;
    ServiceStatus.dwControlsAccepted            = SERVICE_ACCEPT_STOP
                                                | SERVICE_ACCEPT_SHUTDOWN;
    ServiceStatus.dwWin32ExitCode               = 0;
    ServiceStatus.dwServiceSpecificExitCode     = 0;
    ServiceStatus.dwCheckPoint                  = 1;
    ServiceStatus.dwWaitHint                    = 6000;

    ULONG status = 0;

    if (! SetServiceStatus(ServiceStatusHandle, &ServiceStatus))
        status = GetLastError();

    /*while (! IsDebuggerPresent()) {
        Sleep(1000);
    } __debugbreak();*/

    if (status == 0)
        status = InitializeEventLog();

    if (status == 0) {
        bool ok = DriverAssist::Initialize();
        if (! ok)
            status = 0x1234;
    }

    if (status == 0) {
        status = InitializePipe();

		SbieDll_DisableCHPE();
    }

    if (status == 0) {

        ServiceStatus.dwCurrentState        = SERVICE_RUNNING;
        ServiceStatus.dwCheckPoint          = 0;
        ServiceStatus.dwWaitHint            = 0;

    } else {

        ServiceStatus.dwCurrentState        = SERVICE_STOPPED;
        ServiceStatus.dwWin32ExitCode       = ERROR_SERVICE_SPECIFIC_ERROR;
        ServiceStatus.dwServiceSpecificExitCode = status;
    }

    SetServiceStatus(ServiceStatusHandle, &ServiceStatus);
}


//---------------------------------------------------------------------------
// InitializeEventLog
//---------------------------------------------------------------------------


DWORD InitializeEventLog(void)
{
    EventLog = OpenEventLog(NULL, ServiceName);
    return 0;
}


//---------------------------------------------------------------------------
// InitializePipe
//---------------------------------------------------------------------------


DWORD InitializePipe(void)
{
    // get a pipe server running and start sub servers
    PipeServer *pipeServer = PipeServer::GetPipeServer();
    if (! pipeServer)
        return (0x00300000 + GetLastError());

    new ProcessServer(pipeServer);
    new SbieIniServer(pipeServer);
    new ServiceServer(pipeServer);
    new PStoreServer(pipeServer);
    new TerminalServer(pipeServer);
    new NamedPipeServer(pipeServer);
    new FileServer(pipeServer);
    pComServer = new ComServer(pipeServer);
    new IpHlpServer(pipeServer);
    new NetApiServer(pipeServer);
    new QueueServer(pipeServer);
    new MountManager(pipeServer);
    new EpMapperServer(pipeServer);

    if (! pipeServer->Start())
        return (0x00200000 + GetLastError());
    return 0;
}


//---------------------------------------------------------------------------
// ServiceHandlerEx
//---------------------------------------------------------------------------


DWORD WINAPI ServiceHandlerEx(
    DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
    if (dwControl == SERVICE_CONTROL_STOP ||
        dwControl == SERVICE_CONTROL_SHUTDOWN)
    {
        PipeServer *pipeServer = PipeServer::GetPipeServer();
        delete pipeServer;

#ifdef _M_ARM64
        if (DisableCHPE) {
            HKEY hkey = NULL;
            LSTATUS rc = RegCreateKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Wow64\\x86\\xtajit",
                0, NULL, 0, KEY_ALL_ACCESS, NULL, &hkey, NULL);
            if (rc == 0)
            {
                DWORD value;
                DWORD size = sizeof(value);
                rc = RegQueryValueEx(hkey, L"LoadCHPEBinaries_old", NULL, NULL, (BYTE*)&value, &size);
                if (rc == 0) {
                    RegSetValueEx(hkey, L"LoadCHPEBinaries", NULL, REG_DWORD, (BYTE*)&value, size);
                    RegDeleteValue(hkey, L"LoadCHPEBinaries_old");
                }
                else
                    RegDeleteValue(hkey, L"LoadCHPEBinaries");

                RegCloseKey(hkey);
            }
        }
#endif

        ServiceStatus.dwCurrentState        = SERVICE_STOPPED;
        ServiceStatus.dwCheckPoint          = 0;
        ServiceStatus.dwWaitHint            = 0;

        if (pComServer)
            pComServer->DeleteAllSlaves();

        DriverAssist::Shutdown();

        MountManager::Shutdown();

    } else if (dwControl != SERVICE_CONTROL_INTERROGATE)
        return ERROR_CALL_NOT_IMPLEMENTED;

    if (! SetServiceStatus(ServiceStatusHandle, &ServiceStatus))
        return GetLastError();

    return 0;
}


//---------------------------------------------------------------------------
// LogEvent
//---------------------------------------------------------------------------


void LogEvent(ULONG msgid, ULONG level, ULONG detail)
{
    WCHAR extra[64];
    const WCHAR *ptr_extra[2];
    USHORT num_extra;

    ptr_extra[0] = NULL;
    if (msgid == MSG_9234) {
        wsprintf(extra, L"level %04X status=%08X error=%d",
                    level, detail, detail);
        ptr_extra[1] = extra;
        num_extra = 2;
    } else {
        ptr_extra[1] = NULL;
        num_extra = 0;
    }

    if (EventLog) {
        ReportEvent(EventLog, EVENTLOG_ERROR_TYPE, 0, msgid, NULL,
                    num_extra, 0, ptr_extra, NULL);
    }
}


//---------------------------------------------------------------------------
// LogMessage_Event
//---------------------------------------------------------------------------


void LogMessage_Event(ULONG code, wchar_t* data, ULONG pid)
{
    //
    // get log message
    //

    WCHAR *str1 = data;
    ULONG str1_len = wcslen(str1);
    WCHAR *str2 = str1 + str1_len + 1;
    ULONG str2_len = wcslen(str2);

    WCHAR *text = SbieDll_FormatMessage2(code, str1, str2);
    if (! text)
        return;

    //
    // add user name
    //
    /*
    WCHAR user[UNLEN + 1];
    WCHAR domain[DNLEN + 1];
    bool GetUserNameFromProcess(DWORD pid, WCHAR * user, DWORD userSize, WCHAR * domain, DWORD domainSize);
    if (GetUserNameFromProcess(pid, user, UNLEN + 1, domain, DNLEN + 1)) {

        WCHAR *text2 = (WCHAR *)LocalAlloc(
            LMEM_FIXED, (wcslen(text) + UNLEN + DNLEN + 10) * sizeof(WCHAR));
        if (text2) {

            wsprintf(text2, L"%s (%s\\%s)", text, domain, user);

            LocalFree(text);
            text = text2;
        }
    }*/

    //
    // add event
    //

    const WCHAR* ptr_extra[2] = { text, NULL };
    USHORT num_extra = 1;

    if (EventLog) {
        ReportEvent(EventLog, EVENTLOG_INFORMATION_TYPE, 0, code, NULL, num_extra, 0, ptr_extra, NULL);
    }

    LocalFree(text);
}


//---------------------------------------------------------------------------
// AbortServer
//---------------------------------------------------------------------------


void AbortServer(void)
{
    SC_HANDLE handle1 = OpenSCManager(NULL, NULL, GENERIC_READ);
    SC_HANDLE handle2 = NULL;
    if (handle1)
        handle2 = OpenService(handle1, SBIESVC, SERVICE_STOP);
    if (handle2) {
        SERVICE_STATUS ss;
        ControlService(handle2, SERVICE_CONTROL_STOP, &ss);
        Sleep(500);
    }
    ExitProcess(0);
}


//---------------------------------------------------------------------------
// RestrictToken
//---------------------------------------------------------------------------


bool RestrictToken(void)
{
    static const UCHAR AdminSid[16] = {
        0x01,                                   // SID Revision
        0x02,                                   // SubAuthority Count
        0x00, 0x00, 0x00, 0x00, 0x00, 0x05,     // Identifier Authority
        0x20, 0x00, 0x00, 0x00,                 // SubAuthority 1: 32
        0x20, 0x02, 0x00, 0x00                  // SubAuthority 2: 544
    };
    static const UCHAR PowerUserSid[16] = {
        0x01,                                   // SID Revision
        0x02,                                   // SubAuthority Count
        0x00, 0x00, 0x00, 0x00, 0x00, 0x05,     // Identifier Authority
        0x20, 0x00, 0x00, 0x00,                 // SubAuthority 1: 32
        0x23, 0x02, 0x00, 0x00                  // SubAuthority 2: 547
    };
    const ULONG buf_size = 8192;

    NTSTATUS status;
    HANDLE OldToken, NewToken, DupToken;
    OBJECT_ATTRIBUTES objattrs;
    SECURITY_QUALITY_OF_SERVICE QoS;
    TOKEN_GROUPS *groups = NULL;
    TOKEN_GROUPS *rsids = NULL;
    ULONG i;
    bool ok = false;

    //
    // get the effective token
    //

    status = NtOpenThreadToken(
        NtCurrentThread(), MAXIMUM_ALLOWED, FALSE, &OldToken);
    if (! NT_SUCCESS(status))
        status = NtOpenProcessToken(
            NtCurrentProcess(), MAXIMUM_ALLOWED, &OldToken);

    if (! NT_SUCCESS(status)) {
        OldToken = NULL;
        goto finish;
    }

    //
    // scan for the BUILTIN\Administrators group SID
    //

    groups = (TOKEN_GROUPS *)HeapAlloc(GetProcessHeap(), 0, buf_size);
    if (! groups)
        goto finish;

    status = NtQueryInformationToken(
                        OldToken, TokenGroups, groups, buf_size, &i);
    if (NT_SUCCESS(status)) {

        bool found = false;

        for (i = 0; (! found) && (i < groups->GroupCount); ++i) {

            char *sid = (char *)groups->Groups[i].Sid;
            if (memcmp(sid, AdminSid, sizeof(AdminSid)) == 0)
                found = true;
            if (memcmp(sid, PowerUserSid, sizeof(PowerUserSid)) == 0)
                found = true;
        }

        if (! found)
            ok = true;

        if (found) {

            //
            // we should duplicate the list of restricting sids from
            // the old token, or NtFilterToken might return an error
            //

            rsids = (TOKEN_GROUPS *)HeapAlloc(GetProcessHeap(), 0, buf_size);
            if (! rsids)
                goto finish;

            status = NtQueryInformationToken(
                        OldToken, TokenRestrictedSids, rsids, buf_size, &i);
            if (! NT_SUCCESS(status))
                goto finish;

            for (i = 0; i < rsids->GroupCount; ++i)
                rsids->Groups[i].Attributes = 0;

            //
            // SID found, so created a restricted (filtered) token
            //

            groups->GroupCount = 2;
            groups->Groups[0].Sid        = (PSID)AdminSid;
            groups->Groups[0].Attributes = 0;
            groups->Groups[1].Sid        = (PSID)PowerUserSid;
            groups->Groups[1].Attributes = 0;

            status = NtFilterToken(
                OldToken, DISABLE_MAX_PRIVILEGE, groups, NULL, rsids,
                &NewToken);

            if (NT_SUCCESS(status)) {

                //
                // now create an impersonation token for the restricted one
                //

                InitializeObjectAttributes(&objattrs, NULL, 0, NULL, NULL);
                QoS.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
                QoS.ImpersonationLevel = SecurityImpersonation;
                QoS.ContextTrackingMode = SECURITY_STATIC_TRACKING;
                QoS.EffectiveOnly = FALSE;
                objattrs.SecurityQualityOfService = &QoS;

                status = NtDuplicateToken(
                    NewToken, MAXIMUM_ALLOWED, &objattrs,
                    FALSE, TokenImpersonation, &DupToken);

                if (NT_SUCCESS(status)) {

                    //
                    // finally, impersonate using the restricted token
                    //

                    status = NtSetInformationThread(
                        NtCurrentThread(), ThreadImpersonationToken,
                        &DupToken, sizeof(HANDLE));

                    if (NT_SUCCESS(status))
                        ok = true;

                    NtClose(DupToken);
                }

                NtClose(NewToken);
            }
        }
    }

    //
    // finish
    //

finish:

    if (rsids)
        HeapFree(GetProcessHeap(), 0, rsids);

    if (groups)
        HeapFree(GetProcessHeap(), 0, groups);

    if (OldToken)
        NtClose(OldToken);

    return ok;
}


//---------------------------------------------------------------------------
// CheckDropRights
//---------------------------------------------------------------------------


bool CheckDropRights(const WCHAR *BoxName, const WCHAR *ExeName)
{
    // Allow setting of DropAdminRights to suppress UAC prompts / elevation from the sandboxed realm
    // NOTE: use the SBIE_FLAG_APP_COMPARTMENT !!!!
    //if (SbieApi_QueryConfBool(BoxName, L"NoSecurityIsolation", FALSE))
    //    return false; // if we are not swapping the token we can not drop admin rights so keep this consistent
    if (SbieApi_QueryConfBool(BoxName, L"UseSecurityMode", FALSE))
        return true;
    if (SbieDll_GetSettingsForName_bool(BoxName, ExeName, L"DropAdminRights", FALSE))
        return true;
    return false;
}


//---------------------------------------------------------------------------
// IsProcessWoW64
//---------------------------------------------------------------------------


bool IsProcessWoW64(HANDLE pid)
{
    typedef BOOL (*P_IsWow64Process)(HANDLE, BOOL *);
    static P_IsWow64Process pIsWow64Process = NULL;
    if(!pIsWow64Process)
        pIsWow64Process = (P_IsWow64Process)GetProcAddress(_Kernel32, "IsWow64Process");

    if (!pIsWow64Process)
        return false;

    bool IsWow64 = false;

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,
                                    FALSE, (ULONG)(ULONG_PTR)pid);
    if (hProcess) {

        BOOL xwow64 = FALSE;
        if (pIsWow64Process && pIsWow64Process(hProcess, &xwow64) && xwow64) {

            IsWow64 = true;
        }

        CloseHandle(hProcess);
    }
#ifdef DEBUG_COMSERVER
    else {

        WCHAR txt[256]; wsprintf(txt, L"Cannot determine wow64ness for idProcess=%d\n", idProcess);
        OutputDebugString(txt);
    }
#endif

    return IsWow64;
}


//---------------------------------------------------------------------------
// IsBoxedPath
//---------------------------------------------------------------------------


extern "C" {
    WINBASEAPI DWORD WINAPI GetFinalPathNameByHandleW(
        _In_ HANDLE hFile,
        _Out_writes_(cchFilePath) LPWSTR lpszFilePath,
        _In_ DWORD cchFilePath,
        _In_ DWORD dwFlags
    );
}

bool IsHostPath(HANDLE idProcess, WCHAR* dos_path)
{
    bool result = false; // false on failure
    WCHAR* request_path = NULL;
    WCHAR* sandbox_path = NULL;
    HANDLE handle = INVALID_HANDLE_VALUE;
    ULONG len = 0;

    //
    // get the final file path by opening it and retrieving it from the handle
    //

    handle = CreateFileW(dos_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (handle == INVALID_HANDLE_VALUE)
        goto finish;

    len = 8192;
    request_path = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, len * sizeof(WCHAR));
    if (!request_path)
        goto finish;

    DWORD dwRet = GetFinalPathNameByHandleW(handle, request_path, len, VOLUME_NAME_NT);
    if (dwRet == 0 || dwRet > len) // failed || buffer to small
        goto finish;

    if(len > 12 && _wcsnicmp(request_path, L"\\Device\\Mup\\", 12) == 0)
        goto finish; // files on network shares are not files on the host

    //
    // get the box file path for the calling process
    //

    if (!NT_SUCCESS(SbieApi_QueryProcessPath(idProcess, NULL, NULL, NULL, &len, NULL, NULL)))
        goto finish;

    sandbox_path = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, len + 8 * sizeof(WCHAR));
    if (!sandbox_path)
        goto finish;

    if (!NT_SUCCESS(SbieApi_QueryProcessPath(idProcess, sandbox_path, NULL, NULL, &len, NULL, NULL)))
        goto finish;

    //
    // make sure the specified path is _NOT_ inside the sandbox
    //

    ULONG sandbox_path_len = wcslen(sandbox_path);
    ULONG request_path_len = wcslen(request_path);
    if (request_path_len <= sandbox_path_len || _wcsnicmp(sandbox_path, request_path, sandbox_path_len) != 0) {

        result = true;
    }

finish:
    if (request_path)
        HeapFree(GetProcessHeap(), 0, request_path);
    if (sandbox_path)
        HeapFree(GetProcessHeap(), 0, sandbox_path);
    if (handle != INVALID_HANDLE_VALUE) 
        NtClose(handle);

    return result;
}
