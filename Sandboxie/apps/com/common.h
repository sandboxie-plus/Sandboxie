/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2021-2023 David Xanatos, xanasoft.com
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
// Service handling code, common to RpcSs and DcomLauncher
//---------------------------------------------------------------------------


#include "common/my_version.h"
#include "common/win32_ntddk.h"
#include "core/dll/sbiedll.h"
#include <sddl.h>

#pragma warning(disable : 4996)

//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


const WCHAR *_SandboxieRpcSs = SANDBOXIE L"RpcSs.exe";
const WCHAR *_msiexec = L"msiexec.exe";

static HMODULE KernelBase = NULL;
static BOOLEAN IsWindows81 = FALSE;

//---------------------------------------------------------------------------
// Useful Macros
//---------------------------------------------------------------------------


#define ErrorMessageBox(txt) {                                      \
    MessageBox(                                                     \
        NULL, txt, ServiceTitle, MB_ICONEXCLAMATION | MB_OK);       \
    }                                                               \

#define HOOK_WIN32(func) {                                          \
    const char *FuncName = #func;                                   \
    void *SourceFunc = (void *)func;                                \
    if (KernelBase) {                                               \
        SourceFunc = GetProcAddress(KernelBase, FuncName);          \
        if (! SourceFunc)                                           \
            SourceFunc = (void *)func;                              \
    }                                                               \
    __sys_##func =                                                  \
        (ULONG_PTR)SbieDll_Hook(FuncName, SourceFunc, my_##func, KernelBase);   \
    if (! __sys_##func)                                             \
        hook_success = FALSE;                                       \
    }

typedef BOOL(*P_SetServiceStatus)(SERVICE_STATUS_HANDLE hServiceStatus, LPSERVICE_STATUS lpServiceStatus);

//---------------------------------------------------------------------------
// Check_Windows_7
//---------------------------------------------------------------------------


void Check_Windows_7(void)
{
    OSVERSIONINFO osvi;

    memzero(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx((OSVERSIONINFO *)&osvi);
    if (osvi.dwMajorVersion > 6 ||
        osvi.dwMajorVersion == 6 && osvi.dwMinorVersion >= 1) {

        KernelBase = LoadLibrary(L"KernelBase.dll");

        //
        // GetVersionEx in Windows 8.1 returns version 6.2, same as
        // Windows 8.  we detect 8.1 by looking up a new NTDLL entrypoint
        //

        if (GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtCreateTimer2"))
            IsWindows81 = TRUE;
    }
}


//---------------------------------------------------------------------------
// FindProcessId
//---------------------------------------------------------------------------


_FX ULONG FindProcessId(
    const WCHAR *ProcessName,
    BOOL FindSystemToken)
{
    ULONG ret_pid = 0;
    ULONG *pids;
    ULONG i;
    WCHAR this_sid[96], that_sid[96], image[96];
    HANDLE process;

    process = (HANDLE)(ULONG_PTR)GetCurrentProcessId();
    SbieApi_QueryProcess(process, NULL, NULL, this_sid, NULL);

    ULONG pid_count = 0;
    SbieApi_EnumProcessEx(NULL, FALSE, -1, NULL, &pid_count); // query count
    pid_count += 128;

    pids = HeapAlloc(
        GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, sizeof(ULONG) * pid_count);
    SbieApi_EnumProcessEx(NULL, FALSE, -1, pids, &pid_count); // query pids

    for (i = 0; i < pid_count; ++i) {

        HANDLE pids_i = (HANDLE)(ULONG_PTR)pids[i];
        SbieApi_QueryProcess(pids_i, NULL, image, that_sid, NULL);
        if (_wcsicmp(image, ProcessName) == 0 &&
            _wcsicmp(this_sid, that_sid) == 0) {

            BOOL found = FALSE;

            if (FindSystemToken) {
                SbieApi_OpenProcess(&process, pids_i);
                if (!process) {
                    process = OpenProcess(
                        PROCESS_QUERY_INFORMATION, FALSE, pids[i]);
                }
                if ((!process) && GetLastError() == ERROR_ACCESS_DENIED) {
                    // we got access denied error, so it's reasonable to
                    // assume the process is running under a system account
                    process = 0;
                    found = TRUE;
                }
            }
            else {
                process = 0;
                found = TRUE;
            }

            if (process) {
                if (SbieDll_CheckProcessLocalSystem(process))
                    found = TRUE;
                CloseHandle(process);
            }

            if (found) {
                ret_pid = pids[i];
                break;
            }
        }
    }

    HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, pids);
    return ret_pid;
}

#include "header.h"


//---------------------------------------------------------------------------
// my_findProcessData
//---------------------------------------------------------------------------


PROCESS_DATA myData[DATA_SLOTS] = {
    {0,0,0,0,0,0,NULL,NULL},
    {0,0,0,0,0,0,NULL,NULL},
    {0,0,0,0,0,0,NULL,NULL},
    {0,0,0,0,0,0,NULL,NULL},
    {0,0,0,0,0,0,NULL,NULL}
};

PROCESS_DATA *my_findProcessData(WCHAR *name, int createNew) {

    int i = 0;
    for (i = 0; i < DATA_SLOTS; i++) {
        if (myData[i].name) {
            if (!_wcsicmp(name, myData[i].name)) {
                return &myData[i];
            }
        }
        else {
            if (createNew) {
                int lenName = 0;
                if (!name) {
                    return NULL;
                }
                while (name[lenName++] && lenName < 256); //strlen
                if (!lenName || lenName >= 256) return NULL;

                memset(&myData[i], 0, sizeof(PROCESS_DATA));
                myData[i].name = (WCHAR *)calloc(sizeof(WCHAR), lenName);
                myData[i].EventName = (WCHAR *)calloc(sizeof(WCHAR), 128 + lenName);
                swprintf(myData[i].name, L"%s", name);
                swprintf(myData[i].EventName, SBIE_BOXED_ L"ServiceInitComplete_%s", myData[i].name);

                if (!_wcsicmp(name, L"RPCSS")) {
                    myData[i].hStartLingerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
                }
                return &myData[i];
            }
            return NULL;
        }
    }
    return NULL;
}

DWORD ServiceStatus_CurrentState = 0;
DWORD ServiceStatus_CheckPoint = 0;
DWORD ServiceStatus_ErrorCode = 0;
ULONG_PTR __sys_SetServiceStatus = 0;

_FX void InitComplete(PROCESS_DATA *data) {
    HANDLE hInitEvent;
    if (!data) {
        return;
    }
    if (data->hStartLingerEvent) {
        SetEvent(data->hStartLingerEvent);
    }

    hInitEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, data->EventName);
    if (!hInitEvent) {
        hInitEvent = CreateEvent(NULL, TRUE, FALSE, data->EventName);
    }
    if (hInitEvent) {
        SetEvent(hInitEvent);
    }
    return;
}

_FX BOOL my_SetServiceStatus(SERVICE_STATUS_HANDLE hServiceStatus, LPSERVICE_STATUS lpServiceStatus) {
    int i = 0;
    //
    // update service status in SbieDll
    // needed for SandboxieCrypto which hooks SetServiceStatus
    //
#ifdef SANDBOXIECRYPTO

    ((P_SetServiceStatus)__sys_SetServiceStatus)(hServiceStatus, lpServiceStatus);

#endif SANDBOXIECRYPTO

    //
    // update status in this process
    //
    while (i < DATA_SLOTS && myData[i].name)
    {
        if (!myData[i].initFlag)
        {
            if (myData[i].tid == 0 || myData[i].tid == GetCurrentThreadId())
            {
                myData[i].tid = GetCurrentThreadId();
                if (!((lpServiceStatus->dwCurrentState == 0) || (lpServiceStatus->dwCurrentState == SERVICE_START_PENDING)))
                {
                    if (lpServiceStatus->dwCurrentState & SERVICE_STOPPED)
                        myData[i].errorCode = lpServiceStatus->dwWin32ExitCode;
                    else
                        myData[i].checkpoint = lpServiceStatus->dwCheckPoint;
                    ServiceStatus_CheckPoint = lpServiceStatus->dwCheckPoint;
                    myData[i].initFlag = 1;
                    myData[i].state = lpServiceStatus->dwCurrentState;
                    InitComplete(&myData[i]);
                }
                break;
            }
        }
        i++;
    }
    return TRUE;
}

//---------------------------------------------------------------------------
// my_StartServiceCtrlDispatcherW
//---------------------------------------------------------------------------

ULONG_PTR __sys_StartServiceCtrlDispatcherW = 0;


BOOL my_StartServiceCtrlDispatcherW(
    const LPSERVICE_TABLE_ENTRY lpServiceTable)
{
    LPTSTR parms[1];
    LPSERVICE_MAIN_FUNCTION ServiceMain;

    SERVICE_TABLE_ENTRY *entry = lpServiceTable;
    while (entry->lpServiceName) {

        ServiceMain = entry->lpServiceProc;
        parms[0] = entry->lpServiceName;
        ServiceMain(1, parms);

        ++entry;
    }

    return 1;
}


//---------------------------------------------------------------------------
// my_OpenServiceW
//---------------------------------------------------------------------------


ULONG_PTR __sys_OpenServiceW = 0;


SC_HANDLE my_OpenServiceW(
    SC_HANDLE hSCManager,
    LPCTSTR lpServiceName,
    DWORD dwDesiredAccess)
{


    if (_wcsicmp(lpServiceName, L"RpcSs") == 0) {

        // DcomLaunch is querying the RPCSS service

        SetLastError(0);
        return SC_HANDLE_RPCSS;

    }
    else if (_wcsicmp(lpServiceName, L"MSIServer") == 0) {

        // COM (in the RPCSS part) needs to query service status for
        // MSIServer; see more about this in QueryServiceStatus()

        SetLastError(0);
        return SC_HANDLE_MSISERVER;

    }
    else if (_wcsicmp(lpServiceName, L"EventSystem") == 0) {

        // same as above, for EventSystem

        SetLastError(0);
        return SC_HANDLE_EVENTSYSTEM;

    }
    else {

        // fallback to SbieDll SCM implementation

        typedef SC_HANDLE(*P_OpenService)(
            SC_HANDLE hSCManager,
            const void *lpServiceName,
            DWORD dwDesiredAccess);

        SC_HANDLE hService = ((P_OpenService)__sys_OpenServiceW)(
            hSCManager, lpServiceName, dwDesiredAccess);
        ULONG err = GetLastError();

        //{WCHAR txt[512];
        //wsprintf(txt, L"OpenService %s by Process ID %d gives %08X\n", lpServiceName, GetCurrentProcessId(), hService);
        //OutputDebugString(txt);}

        if (SbieDll_IsBoxedService(hService)) {

            SetLastError(err);
            return hService;

        }
        else {

            SetLastError(ERROR_SERVICE_DOES_NOT_EXIST);
            return 0;
        }
    }
}


//---------------------------------------------------------------------------
// my_CloseServiceHandle
//---------------------------------------------------------------------------


ULONG_PTR __sys_CloseServiceHandle = 0;


BOOL my_CloseServiceHandle(SC_HANDLE hSCObject)
{
    if (SC_HANDLE_IS_FAKE(hSCObject)) {
        SetLastError(0);
        return TRUE;
    }
    else {
        typedef BOOL(*P_CloseServiceHandle)(SC_HANDLE hSCObject);
        return ((P_CloseServiceHandle)__sys_CloseServiceHandle)(hSCObject);
    }
}


//---------------------------------------------------------------------------
// my_QueryServiceStatusEx
//---------------------------------------------------------------------------


ULONG_PTR __sys_QueryServiceStatusEx = 0;


BOOL my_QueryServiceStatusEx(
    SC_HANDLE hService,
    SC_STATUS_TYPE InfoLevel,
    LPBYTE lpBuffer,
    DWORD cbBufSize,
    LPDWORD pcbBytesNeeded)
{
    //WCHAR txt[256];
    //swprintf(txt, L"QueryServiceStatusEx for Service Handle %X\n", hService);
    //OutputDebugString(txt);
    //swprintf(txt, L"lpBuffer = %p cbBufSize = %d pcbBytesNeeded = %p\n",
    //    lpBuffer, cbBufSize, pcbBytesNeeded);
    //OutputDebugString(txt);

    if (InfoLevel != SC_STATUS_PROCESS_INFO) {
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }
    if (cbBufSize < sizeof(SERVICE_STATUS_PROCESS)) {
        *pcbBytesNeeded = sizeof(SERVICE_STATUS_PROCESS);
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    if (InfoLevel == SC_STATUS_PROCESS_INFO) {

        // fill in some defaults

        SERVICE_STATUS_PROCESS *buf = (SERVICE_STATUS_PROCESS *)lpBuffer;
        buf->dwServiceType = SERVICE_WIN32_OWN_PROCESS;
        buf->dwCurrentState = SERVICE_RUNNING;
        buf->dwControlsAccepted =
            SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
        buf->dwWin32ExitCode = 0;
        buf->dwServiceSpecificExitCode = 0;
        buf->dwCheckPoint = 0;
        buf->dwWaitHint = 0;
        buf->dwServiceFlags = SERVICE_RUNS_IN_SYSTEM_PROCESS;
        buf->dwProcessId = 0;

        if (hService == SC_HANDLE_RPCSS) {

            buf->dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
            buf->dwProcessId = FindProcessId(_SandboxieRpcSs, FALSE);

        }
        else if (hService == SC_HANDLE_MSISERVER) {

            // when MSI Server calls CoRegisterClassObject, RPCSS/DCOMLAUNCH
            // service will query the status of MSIServer service, and
            // expect the service to NOT be stopped or stop-pending.
            // without this, MSI server gets CO_E_WRONG_SERVER_IDENTITY.

            //buf->dwProcessId = FindProcessId(_msiexec, TRUE);

            WCHAR keyname[128];
            wcscpy(keyname, L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Services\\");
            wcscat(keyname, L"MSIServer");

            UNICODE_STRING objname;
            RtlInitUnicodeString(&objname, keyname);

            HANDLE hkey;
            OBJECT_ATTRIBUTES objattrs;
            InitializeObjectAttributes(&objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);
            if (NT_SUCCESS(NtOpenKey(&hkey, KEY_QUERY_VALUE, &objattrs))) {

                NTSTATUS status;
                ULONG len;
                UNICODE_STRING uni;
                union {
                    KEY_VALUE_PARTIAL_INFORMATION info;
                    WCHAR info_space[256];
                } u;

                RtlInitUnicodeString(&uni, SBIE L"_ProcessId");
                status = NtQueryValueKey(hkey, &uni, KeyValuePartialInformation, &u.info, sizeof(u), &len);

                if (NT_SUCCESS(status) && u.info.Type == REG_DWORD && u.info.DataLength == 4) {

                    ULONG pid;
                    pid = *(ULONG*)u.info.Data;

                    buf->dwProcessId = pid;
                }

                NtClose(hkey);
            }

        }
        else if (hService == SC_HANDLE_EVENTSYSTEM) {

            // same as above, for EventSystem service

            buf->dwProcessId = 0;

        }
        else {

            // fallback to SbieDll SCM implementation

            typedef BOOL(*P_QueryServiceStatusEx)(
                SC_HANDLE hService, SC_STATUS_TYPE InfoLevel,
                LPBYTE lpBuffer, DWORD cbBufSize, LPDWORD pcbBytesNeeded);

            return ((P_QueryServiceStatusEx)__sys_QueryServiceStatusEx)(
                hService, InfoLevel, lpBuffer, cbBufSize, pcbBytesNeeded);
        }
    }

    SetLastError(0);
    return TRUE;
}


//---------------------------------------------------------------------------
// my_QueryServiceStatus
//---------------------------------------------------------------------------


ULONG_PTR __sys_QueryServiceStatus = 0;


BOOL my_QueryServiceStatus(
    SC_HANDLE hService,
    SERVICE_STATUS *ServiceStatus)
{
    SERVICE_STATUS_PROCESS buf;
    DWORD cb;
    BOOL ok;

    ok = QueryServiceStatusEx(
        hService, SC_STATUS_PROCESS_INFO, (void *)&buf, sizeof(buf), &cb);
    if (ok) {
        ServiceStatus->dwServiceType = buf.dwServiceType;
        ServiceStatus->dwCurrentState = buf.dwCurrentState;
        ServiceStatus->dwControlsAccepted = buf.dwControlsAccepted;
        ServiceStatus->dwWin32ExitCode = buf.dwWin32ExitCode;
        ServiceStatus->dwServiceSpecificExitCode
            = buf.dwServiceSpecificExitCode;
        ServiceStatus->dwCheckPoint = buf.dwCheckPoint;
        ServiceStatus->dwWaitHint = buf.dwWaitHint;
    }

    return ok;
}


//---------------------------------------------------------------------------
// my_StartService
//---------------------------------------------------------------------------


ULONG_PTR __sys_StartService = 0;


BOOL my_StartService(
    SC_HANDLE hService,
    DWORD NumArgs,
    void *ArgVector)
{
    BOOL ok;

    if (hService == SC_HANDLE_MSISERVER) {

        ok = SbieDll_StartBoxedService(L"MSIServer", FALSE);

    }
    else if (SbieDll_IsBoxedService(hService)) {

        typedef BOOL(*P_StartService)(
            SC_HANDLE hService, DWORD NumArgs, void *ArgVector);
        ok = ((P_StartService)__sys_StartService)(
            hService, NumArgs, ArgVector);

    }
    else {

        SetLastError(ERROR_ACCESS_DENIED);
        ok = FALSE;
    }

    return ok;
}


//---------------------------------------------------------------------------
// my_ControlService
//---------------------------------------------------------------------------


ULONG_PTR __sys_ControlService = 0;

BOOL my_ControlService(
    SC_HANDLE hService,
    DWORD dwControl,
    LPSERVICE_STATUS lpServiceStatus)
{
    if (SbieDll_IsBoxedService(hService)) {

        typedef BOOL(*P_ControlService)(
            SC_HANDLE hService,
            DWORD dwControl,
            LPSERVICE_STATUS lpServiceStatus);

        return ((P_ControlService)__sys_ControlService)(
            hService, dwControl, lpServiceStatus);

    }
    else
        return my_QueryServiceStatus(hService, lpServiceStatus);
}


//---------------------------------------------------------------------------
// my_CreateEventW
//---------------------------------------------------------------------------


ULONG_PTR __sys_CreateEventW = 0;

HANDLE my_CreateEventW(
    LPSECURITY_ATTRIBUTES lpEventAttributes,
    BOOL bManualReset,
    BOOL bInitialState,
    LPCWSTR lpName)
{
    typedef HANDLE(*P_CreateEventW)(
        LPSECURITY_ATTRIBUTES lpEventAttributes,
        BOOL bManualReset,
        BOOL bInitialState,
        LPCWSTR lpName);
    HANDLE h;

    if (lpName && wcscmp(lpName, L"ScmCreatedEvent") == 0) {
        h = ((P_CreateEventW)__sys_CreateEventW)(NULL, TRUE, TRUE, NULL);
    }
    else {
        h = ((P_CreateEventW)__sys_CreateEventW)(
            lpEventAttributes, bManualReset, bInitialState, lpName);
    }

    return h;
}


//---------------------------------------------------------------------------
// my_CreateFileMappingW (forward reference)
//---------------------------------------------------------------------------

ULONG_PTR __sys_CreateFileMappingW = 0;

HANDLE my_CreateFileMappingW(
    HANDLE hFile,
    LPSECURITY_ATTRIBUTES lpAttributes,
    DWORD flProtect,
    DWORD dwMaximumSizeHigh,
    DWORD dwMaximumSizeLow,
    LPCWSTR lpName);



#if 0


ULONG_PTR __sys_RtlSetLastWin32Error = 0;

typedef void(*P_RtlSetLastWin32Error)(DWORD dwError);

void my_RtlSetLastWin32Error(DWORD dwError)
{
    static ULONG Depth = 0;
    if (dwError && (Depth == 0)) {
        WCHAR txt[64];
        ++Depth;
        wsprintf(txt, L"(%06d) SetLastError - dec=%d hex=%X\n", GetCurrentProcessId(), dwError, dwError);
        OutputDebugString(txt);
        --Depth;
    }
    ((P_RtlSetLastWin32Error)__sys_RtlSetLastWin32Error)(dwError);
}

__declspec(dllimport) void RtlSetLastWin32Error(DWORD dwError);


#endif


//---------------------------------------------------------------------------
// PowerSettingRegisterNotification (for Windows 8.1)
//---------------------------------------------------------------------------


ULONG_PTR __sys_PowerSettingRegisterNotification = 0;


ULONG my_PowerSettingRegisterNotification(
    void *SettingGuid, ULONG Flags, HANDLE Recipient, HANDLE *RegistrationHandle)
{
    *RegistrationHandle = (HANDLE)0x12345678;   // fake handle
    return ERROR_SUCCESS;
}


//---------------------------------------------------------------------------
// Hook_Service_Control_Manager
//---------------------------------------------------------------------------

BOOL Hook_Service_Control_Manager(void)
{
    BOOL hook_success = TRUE;
    HOOK_WIN32(SetServiceStatus);
    HOOK_WIN32(StartServiceCtrlDispatcherW);
    HOOK_WIN32(OpenServiceW);
    HOOK_WIN32(CloseServiceHandle);
    HOOK_WIN32(QueryServiceStatusEx);
    HOOK_WIN32(QueryServiceStatus);
    HOOK_WIN32(StartService);
    HOOK_WIN32(ControlService);

    HOOK_WIN32(CreateFileMappingW);

#if 0
    HOOK_WIN32(RtlSetLastWin32Error);
#endif

    if (IsWindows81) {
        HMODULE pp = LoadLibrary(L"powrprof.dll");
        void *PowerSettingRegisterNotification =
            GetProcAddress(pp, "PowerSettingRegisterNotification");
        HOOK_WIN32(PowerSettingRegisterNotification);
    }

    if (!hook_success) {
        ErrorMessageBox(L"Could not instrument service functions");
        return FALSE;
    }
    else
        return TRUE;
}


//---------------------------------------------------------------------------
// Service_Start_ServiceMain
//---------------------------------------------------------------------------


BOOL Service_Start_ServiceMain(WCHAR *SvcName, const WCHAR *SvcDllName, const UCHAR *SvcProcName, BOOL UseMyStartServiceCtrlDispatcher)
{
    static const WCHAR *ServiceName_EnvVar =
        L"00000000_" SBIE L"_SERVICE_NAME";
    HMODULE dll;
    LPSERVICE_MAIN_FUNCTION ServiceMain;
    ULONG table_len;
    SERVICE_TABLE_ENTRY *table;
    WCHAR ErrorText[128];

    dll = LoadLibrary(SvcDllName);

    if (!dll) {
        wcscpy(ErrorText, L"Could not load service DLL - ");
        wcscat(ErrorText, SvcDllName);
        ErrorMessageBox(ErrorText);
        return FALSE;
    }

    ServiceMain = (LPSERVICE_MAIN_FUNCTION)GetProcAddress(dll, SvcProcName);

    if (!ServiceMain) {
        wsprintf(ErrorText,
            L"Could not locate ServiceMain routine - %S", SvcProcName);
        ErrorMessageBox(ErrorText);
        return FALSE;
    }

    table_len = sizeof(SERVICE_TABLE_ENTRY) * 2;
    table = HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, table_len);
    ZeroMemory(table, table_len);
    table[0].lpServiceName = SvcName;
    table[0].lpServiceProc = ServiceMain;

    SetEnvironmentVariable(ServiceName_EnvVar, SvcName);

    if (UseMyStartServiceCtrlDispatcher) {
        PROCESS_DATA *myData;
        myData = my_findProcessData(SvcName, 1);
        if (!myData) {
            return FALSE;
        }
        myData->tid = 0;
        myData->initFlag = 0;

        CreateThread(NULL, 0, my_StartServiceCtrlDispatcherW, table, 0, NULL);
    }
    else {
        StartServiceCtrlDispatcher(table);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// SuspendMainThread
//---------------------------------------------------------------------------


#define SuspendMainThread()                     \
    while (1)                                   \
        SuspendThread(GetCurrentThread());      \
    return EXIT_SUCCESS;


//---------------------------------------------------------------------------
// CheckSingleInstance
//---------------------------------------------------------------------------


#ifdef SINGLE_INSTANCE_MUTEX_NAME

BOOLEAN CheckSingleInstance(void)
{
    const WCHAR *MutexName = SBIE_BOXED_ SINGLE_INSTANCE_MUTEX_NAME;

    HANDLE hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MutexName);
    if (!hMutex)
        hMutex = CreateMutex(NULL, TRUE, MutexName);

    if (hMutex) {
        if (WaitForSingleObject(hMutex, 0) != WAIT_OBJECT_0) {
            //
            // some other instance holds the mutex, so abort
            //
            return FALSE;
        }
    }

    return TRUE;
}

#endif SINGLE_INSTANCE_MUTEX_NAME
