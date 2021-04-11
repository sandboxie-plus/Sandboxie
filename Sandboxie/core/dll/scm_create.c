/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2021 David Xanatos, xanasoft.com
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
// Service Control Manager
//---------------------------------------------------------------------------


#include "core/svc/ProcessWire.h"
#include "advapi.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static SC_LOCK Scm_LockServiceDatabase(SC_HANDLE hSCManager);

static BOOL Scm_UnlockServiceDatabase(SC_LOCK ScLock);

static HANDLE Scm_AcquireMutex(void);

static void Scm_ReleaseMutex(HANDLE hMutex);

static NTSTATUS Scm_AddBoxedService(const WCHAR *ServiceName);

static SC_HANDLE Scm_CreateServiceW(
    SC_HANDLE hSCManager,
    WCHAR *lpServiceName, WCHAR *lpDisplayName, ULONG dwDesiredAccess,
    ULONG dwServiceType, ULONG dwStartType, ULONG dwErrorControl,
    WCHAR *lpBinaryPathName, WCHAR *lpLoadOrderGroup, WCHAR *lpdwTagId,
    WCHAR *lpDependencies, WCHAR *lpServiceStartName, WCHAR *lpPassword);

static SC_HANDLE Scm_CreateServiceA(
    SC_HANDLE hSCManager,
    UCHAR *lpServiceName, UCHAR *lpDisplayName, ULONG dwDesiredAccess,
    ULONG dwServiceType, ULONG dwStartType, ULONG dwErrorControl,
    UCHAR *lpBinaryPathName, UCHAR *lpLoadOrderGroup, UCHAR *lpdwTagId,
    UCHAR *lpDependencies, UCHAR *lpServiceStartName, UCHAR *lpPassword);

static BOOL Scm_ChangeServiceConfigW(
    SC_HANDLE hService,
    ULONG dwServiceType, ULONG dwStartType, ULONG dwErrorControl,
    WCHAR *lpBinaryPathName, WCHAR *lpLoadOrderGroup,
    WCHAR *lpdwTagId, WCHAR *lpDependencies,
    WCHAR *lpServiceStartName, WCHAR *lpPassword, WCHAR *lpDisplayName);

static BOOL Scm_ChangeServiceConfigA(
    SC_HANDLE hService,
    ULONG dwServiceType, ULONG dwStartType, ULONG dwErrorControl,
    UCHAR *lpBinaryPathName, UCHAR *lpLoadOrderGroup,
    UCHAR *lpdwTagId, UCHAR *lpDependencies,
    UCHAR *lpServiceStartName, UCHAR *lpPassword, UCHAR *lpDisplayName);

static BOOL Scm_ChangeServiceConfig2W(
    SC_HANDLE hService, ULONG dwInfoLevel, void *lpInfo);

static BOOL Scm_ChangeServiceConfig2A(
    SC_HANDLE hService, ULONG dwInfoLevel, void *lpInfo);

static BOOL Scm_DeleteService(SC_HANDLE hService);

static ULONG Scm_StartBoxedService2(
    const WCHAR *name, SERVICE_QUERY_RPL *qrpl);

static BOOL Scm_StartServiceW(
    SC_HANDLE hService, DWORD dwNumServiceArgs, void *lpServiceArgVector);

static BOOL Scm_StartServiceA(
    SC_HANDLE hService, DWORD dwNumServiceArgs, void *lpServiceArgVector);

static ULONG Scm_ServiceMainThread(ULONG_PTR *args);

static BOOL Scm_StartServiceCtrlDispatcherX(
    const void *ServiceTable, BOOLEAN IsUnicode);

static BOOL Scm_StartServiceCtrlDispatcherW(
    const SERVICE_TABLE_ENTRYW *ServiceTable);

static BOOL Scm_StartServiceCtrlDispatcherA(
    const SERVICE_TABLE_ENTRYA *ServiceTable);

static SERVICE_STATUS_HANDLE Scm_RegisterServiceCtrlHandlerW(
    WCHAR *ServiceName, LPHANDLER_FUNCTION HandlerProc);

static SERVICE_STATUS_HANDLE Scm_RegisterServiceCtrlHandlerA(
    UCHAR *ServiceName, LPHANDLER_FUNCTION HandlerProc);

static SERVICE_STATUS_HANDLE Scm_RegisterServiceCtrlHandlerExW(
    WCHAR *ServiceName, LPHANDLER_FUNCTION_EX HandlerProc, void *Context);

static SERVICE_STATUS_HANDLE Scm_RegisterServiceCtrlHandlerExA(
    UCHAR *ServiceName, LPHANDLER_FUNCTION_EX HandlerProc, void *Context);

#ifdef _WIN64

static SERVICE_STATUS_HANDLE Scm_RegisterServiceCtrlHandlerHelp(
    UCHAR *ServiceName, ULONG_PTR HandlerProc, void *Context, ULONG Flags);

#endif _WIN64

static BOOL Scm_SetServiceStatus_Internal(
    HKEY ServiceKeyHandle,
    SERVICE_STATUS_HANDLE hServiceStatus, LPSERVICE_STATUS lpServiceStatus,
    BOOLEAN SetProcessId);

static BOOL Scm_SetServiceStatus(
    SERVICE_STATUS_HANDLE hServiceStatus, LPSERVICE_STATUS lpServiceStatus);

static BOOL Scm_ControlService(
    SC_HANDLE hService, DWORD dwControl, SERVICE_STATUS *lpServiceStatus);

static void Scm_DeletePermissions(const WCHAR *AppIdGuid);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static HANDLE   Scm_ServiceKey = NULL;

static LPHANDLER_FUNCTION       Scm_Handler = NULL;
static LPHANDLER_FUNCTION_EX    Scm_HandlerEx = NULL;

static void *Scm_HandlerContext = NULL;

//static volatile WCHAR*  Scm_ServiceName = NULL;
static volatile BOOLEAN Scm_Started = FALSE;
static volatile BOOLEAN Scm_Stopped = FALSE;


//---------------------------------------------------------------------------
// Scm_LockServiceDatabase
//---------------------------------------------------------------------------


_FX SC_LOCK Scm_LockServiceDatabase(SC_HANDLE hSCManager)
{
    if (hSCManager != HANDLE_SERVICE_MANAGER) {
        SetLastError(ERROR_INVALID_HANDLE);
        return NULL;
    }
    return HANDLE_SERVICE_LOCK;
}


//---------------------------------------------------------------------------
// Scm_UnlockServiceDatabase
//---------------------------------------------------------------------------


_FX BOOL Scm_UnlockServiceDatabase(SC_LOCK ScLock)
{
    if (ScLock != HANDLE_SERVICE_LOCK) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    return TRUE;
}


//---------------------------------------------------------------------------
// Scm_AcquireMutex
//---------------------------------------------------------------------------


_FX HANDLE Scm_AcquireMutex(void)
{
    const WCHAR *MutexName = SBIE L"_ServiceManager_Mutex";

    HANDLE hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MutexName);
    if (! hMutex)
        hMutex = CreateMutex(NULL, FALSE, MutexName);
    if (hMutex)
        WaitForSingleObject(hMutex, 5000);

    Scm_ReleaseMutex(NULL);

    return hMutex;
}


//---------------------------------------------------------------------------
// Scm_ReleaseMutex
//---------------------------------------------------------------------------


_FX void Scm_ReleaseMutex(HANDLE hMutex)
{
    if (hMutex) {

        ReleaseMutex(hMutex);

        CloseHandle(hMutex);
    }

    Scm_DiscardKeyCache(SBIESVC);
}


//---------------------------------------------------------------------------
// Scm_AddBoxedService
//---------------------------------------------------------------------------


_FX NTSTATUS Scm_AddBoxedService(const WCHAR *ServiceName)
{
    NTSTATUS status;
    WCHAR *names1, *name;
    WCHAR *names2;
    ULONG len;
    HANDLE hkey;
    UNICODE_STRING uni;

    HANDLE hMutex = Scm_AcquireMutex();

    len = 0;
    names1 = name = Scm_GetBoxedServices();
    while (*name) {
        if (_wcsicmp(name, ServiceName) == 0) {
            status = STATUS_OBJECT_NAME_COLLISION;
            goto finish;
        }
        len += wcslen(name) + 1;
        name += wcslen(name) + 1;
    }

    names2 = Dll_AllocTemp((len + wcslen(ServiceName) + 8) * sizeof(WCHAR));
    wmemcpy(names2, names1, len);
    wcscpy(&names2[len], ServiceName);
    len += wcslen(ServiceName) + 1;
    names2[len] = L'\0';
    ++len;

    hkey = Scm_OpenKeyForService(SBIESVC, TRUE);
    if (! hkey)
        status = STATUS_UNSUCCESSFUL;
    else {

        RtlInitUnicodeString(&uni, L"SandboxedServices");
        status = NtSetValueKey(
            hkey, &uni, 0, REG_MULTI_SZ, names2, len * sizeof(WCHAR));

        NtClose(hkey);
    }

    Dll_Free(names2);

finish:

    Scm_ReleaseMutex(hMutex);

    Dll_Free(names1);

    return status;
}


//---------------------------------------------------------------------------
// Scm_CreateServiceW
//---------------------------------------------------------------------------


_FX SC_HANDLE Scm_CreateServiceW(
    SC_HANDLE hSCManager,
    WCHAR *lpServiceName, WCHAR *lpDisplayName, ULONG dwDesiredAccess,
    ULONG dwServiceType, ULONG dwStartType, ULONG dwErrorControl,
    WCHAR *lpBinaryPathName, WCHAR *lpLoadOrderGroup, WCHAR *lpdwTagId,
    WCHAR *lpDependencies, WCHAR *lpServiceStartName, WCHAR *lpPassword)
{
    NTSTATUS status;
    SC_HANDLE hService;
    HANDLE hkey;
    UNICODE_STRING uni;
    WCHAR *name;

    //
    // verify the service does not exist (also verifies hSCManager,
    // and that the service name is not NULL)
    //

    hService = Scm_OpenServiceW(
        hSCManager, lpServiceName, SERVICE_QUERY_STATUS);

    if (hService) {
        Scm_CloseServiceHandle(hService);
        SetLastError(ERROR_SERVICE_EXISTS);
        return NULL;
    }

    if (GetLastError() != ERROR_SERVICE_DOES_NOT_EXIST)
        return NULL;

    //
    // create a key for the new service
    //

    hkey = Scm_OpenKeyForService(lpServiceName, TRUE);
    if (! hkey) {
        // SetLastError already called
        return NULL;
    }

    RtlInitUnicodeString(&uni, L"Type");
    status = NtSetValueKey(
        hkey, &uni, 0, REG_DWORD, &dwServiceType, sizeof(ULONG));
    if (! NT_SUCCESS(status))
        goto abort;

    RtlInitUnicodeString(&uni, L"Start");
    status = NtSetValueKey(
        hkey, &uni, 0, REG_DWORD, &dwStartType, sizeof(ULONG));
    if (! NT_SUCCESS(status))
        goto abort;

    RtlInitUnicodeString(&uni, L"ErrorControl");
    status = NtSetValueKey(
        hkey, &uni, 0, REG_DWORD, &dwErrorControl, sizeof(ULONG));
    if (! NT_SUCCESS(status))
        goto abort;

    if (lpDisplayName) {

        RtlInitUnicodeString(&uni, L"DisplayName");
        status = NtSetValueKey(
            hkey, &uni, 0, REG_SZ, lpDisplayName,
            (wcslen(lpDisplayName) + 1) * sizeof(WCHAR));
        if (! NT_SUCCESS(status))
            goto abort;
    }

    if (lpBinaryPathName) {

        RtlInitUnicodeString(&uni, L"ImagePath");
        status = NtSetValueKey(
            hkey, &uni, 0, REG_SZ, lpBinaryPathName,
            (wcslen(lpBinaryPathName) + 1) * sizeof(WCHAR));
        if (! NT_SUCCESS(status))
            goto abort;
    }

    if (lpServiceStartName) {

        RtlInitUnicodeString(&uni, L"ObjectName");
        status = NtSetValueKey(
            hkey, &uni, 0, REG_SZ, lpServiceStartName,
            (wcslen(lpServiceStartName) + 1) * sizeof(WCHAR));
        if (! NT_SUCCESS(status))
            goto abort;
    }

    //
    // add the service to SbieSvc list of sandboxed services
    //

    status = Scm_AddBoxedService(lpServiceName);

    if (! NT_SUCCESS(status))
        goto abort;

    NtClose(hkey);

    //
    // allocate a 'handle' that points to the service name
    //

    name = Dll_Alloc(
        sizeof(ULONG) + (wcslen(lpServiceName) + 1) * sizeof(WCHAR));
    *(ULONG *)name = tzuk;
    wcscpy((WCHAR *)(((ULONG *)name) + 1), lpServiceName);
    _wcslwr(name);

    SetLastError(0);
    return (SC_HANDLE)name;

    //
    // failure:  delete the key for the new service
    //

abort:

    NtDeleteKey(hkey);
    NtClose(hkey);

    SetLastError(ERROR_INVALID_PARAMETER);
    return NULL;
}


//---------------------------------------------------------------------------
// Scm_CreateServiceA
//---------------------------------------------------------------------------


_FX SC_HANDLE Scm_CreateServiceA(
    SC_HANDLE hSCManager,
    UCHAR *lpServiceName, UCHAR *lpDisplayName, ULONG dwDesiredAccess,
    ULONG dwServiceType, ULONG dwStartType, ULONG dwErrorControl,
    UCHAR *lpBinaryPathName, UCHAR *lpLoadOrderGroup, UCHAR *lpdwTagId,
    UCHAR *lpDependencies, UCHAR *lpServiceStartName, UCHAR *lpPassword)
{
    NTSTATUS status;
    UNICODE_STRING lpServiceNameW, lpDisplayNameW;
    UNICODE_STRING lpBinaryPathNameW, lpLoadOrderGroupW;
    UNICODE_STRING lpdwTagIdW, lpDependenciesW;
    UNICODE_STRING lpServiceStartNameW, lpPasswordW;
    SC_HANDLE handle;
    ULONG error;

#define ANSI_TO_UNI(x)                                              \
    x##W.Buffer = NULL;                                             \
    if (x && NT_SUCCESS(status)) {                                  \
        ANSI_STRING ansi;                                           \
        RtlInitString(&ansi, x);                                    \
        status = RtlAnsiStringToUnicodeString(&x##W, &ansi, TRUE);  \
    }

#define FREE_UNI(x)                                                 \
    if (x##W.Buffer)                                                \
        RtlFreeUnicodeString(&x##W);

    status = STATUS_SUCCESS;

    ANSI_TO_UNI(lpServiceName);
    ANSI_TO_UNI(lpDisplayName);
    ANSI_TO_UNI(lpBinaryPathName);
    ANSI_TO_UNI(lpLoadOrderGroup);
    ANSI_TO_UNI(lpdwTagId);
    ANSI_TO_UNI(lpDependencies);        // this is multi_sz, but whatever
    ANSI_TO_UNI(lpServiceStartName);
    ANSI_TO_UNI(lpPassword);

    if (! NT_SUCCESS(status)) {

        handle = NULL;
        error = ERROR_NOT_ENOUGH_MEMORY;

    } else {

        handle = Scm_CreateServiceW(
            hSCManager,
            lpServiceNameW.Buffer, lpDisplayNameW.Buffer,
            dwDesiredAccess, dwServiceType, dwStartType, dwErrorControl,
            lpBinaryPathNameW.Buffer, lpLoadOrderGroupW.Buffer,
            lpdwTagIdW.Buffer, lpDependenciesW.Buffer,
            lpServiceStartNameW.Buffer, lpPasswordW.Buffer);

        error = GetLastError();
    }

    FREE_UNI(lpServiceName);
    FREE_UNI(lpDisplayName);
    FREE_UNI(lpBinaryPathName);
    FREE_UNI(lpLoadOrderGroup);
    FREE_UNI(lpdwTagId);
    FREE_UNI(lpDependencies);
    FREE_UNI(lpServiceStartName);
    FREE_UNI(lpPassword);

#undef FREE_UNI
#undef ANSI_TO_UNI

    SetLastError(error);
    return handle;
}


//---------------------------------------------------------------------------
// Scm_ChangeServiceConfigW
//---------------------------------------------------------------------------


_FX BOOL Scm_ChangeServiceConfigW(
    SC_HANDLE hService,
    ULONG dwServiceType, ULONG dwStartType, ULONG dwErrorControl,
    WCHAR *lpBinaryPathName, WCHAR *lpLoadOrderGroup,
    WCHAR *lpdwTagId, WCHAR *lpDependencies,
    WCHAR *lpServiceStartName, WCHAR *lpPassword, WCHAR *lpDisplayName)
{
    NTSTATUS status;
    WCHAR *ServiceName;
    HANDLE hkey;
    UNICODE_STRING uni;

    //
    // verify the service does not exist (also verifies hSCManager,
    // and that the service name is not NULL)
    //

    ServiceName = Scm_GetHandleName(hService);
    if (! ServiceName)
        return FALSE;

    if (! Scm_IsBoxedService(ServiceName)) {

        if (_wcsnicmp(ServiceName, L"clr_optimization_", 17) == 0) {

            //
            // if this a .Net service used during assembly installation,
            // then succeed without doing anything
            //

            SetLastError(0);
            return TRUE;
        }

        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

    //
    // create a key for the new service
    //

    hkey = Scm_OpenKeyForService(ServiceName, TRUE);
    if (! hkey) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (dwServiceType != SERVICE_NO_CHANGE) {

        RtlInitUnicodeString(&uni, L"Type");
        status = NtSetValueKey(
            hkey, &uni, 0, REG_DWORD, &dwServiceType, sizeof(ULONG));
        if (! NT_SUCCESS(status))
            goto abort;
    }

    if (dwStartType != SERVICE_NO_CHANGE) {

        RtlInitUnicodeString(&uni, L"Start");
        status = NtSetValueKey(
            hkey, &uni, 0, REG_DWORD, &dwStartType, sizeof(ULONG));
        if (! NT_SUCCESS(status))
            goto abort;
    }

    if (dwErrorControl != SERVICE_NO_CHANGE) {

        RtlInitUnicodeString(&uni, L"ErrorControl");
        status = NtSetValueKey(
            hkey, &uni, 0, REG_DWORD, &dwErrorControl, sizeof(ULONG));
        if (! NT_SUCCESS(status))
            goto abort;
    }

    if (lpDisplayName) {

        RtlInitUnicodeString(&uni, L"DisplayName");
        status = NtSetValueKey(
            hkey, &uni, 0, REG_SZ, lpDisplayName,
            (wcslen(lpDisplayName) + 1) * sizeof(WCHAR));
        if (! NT_SUCCESS(status))
            goto abort;
    }

    if (lpBinaryPathName) {

        RtlInitUnicodeString(&uni, L"ImagePath");
        status = NtSetValueKey(
            hkey, &uni, 0, REG_SZ, lpBinaryPathName,
            (wcslen(lpBinaryPathName) + 1) * sizeof(WCHAR));
        if (! NT_SUCCESS(status))
            goto abort;
    }

    if (lpServiceStartName) {

        RtlInitUnicodeString(&uni, L"ObjectName");
        status = NtSetValueKey(
            hkey, &uni, 0, REG_SZ, lpServiceStartName,
            (wcslen(lpServiceStartName) + 1) * sizeof(WCHAR));
        if (! NT_SUCCESS(status))
            goto abort;
    }

    //
    // finish
    //

    NtClose(hkey);

    SetLastError(0);
    return TRUE;

    //
    // failure:  delete the key for the new service
    //

abort:

    NtClose(hkey);

    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
}


//---------------------------------------------------------------------------
// Scm_ChangeServiceConfigA
//---------------------------------------------------------------------------


_FX BOOL Scm_ChangeServiceConfigA(
    SC_HANDLE hService,
    ULONG dwServiceType, ULONG dwStartType, ULONG dwErrorControl,
    UCHAR *lpBinaryPathName, UCHAR *lpLoadOrderGroup,
    UCHAR *lpdwTagId, UCHAR *lpDependencies,
    UCHAR *lpServiceStartName, UCHAR *lpPassword, UCHAR *lpDisplayName)
{
    NTSTATUS status;
    UNICODE_STRING lpBinaryPathNameW, lpLoadOrderGroupW;
    UNICODE_STRING lpdwTagIdW, lpDependenciesW;
    UNICODE_STRING lpServiceStartNameW, lpPasswordW;
    UNICODE_STRING lpDisplayNameW;
    ULONG error;
    BOOL ok;

#define ANSI_TO_UNI(x)                                              \
    x##W.Buffer = NULL;                                             \
    if (x && NT_SUCCESS(status)) {                                  \
        ANSI_STRING ansi;                                           \
        RtlInitString(&ansi, x);                                    \
        status = RtlAnsiStringToUnicodeString(&x##W, &ansi, TRUE);  \
    }

#define FREE_UNI(x)                                                 \
    if (x##W.Buffer)                                                \
        RtlFreeUnicodeString(&x##W);

    status = STATUS_SUCCESS;

    ANSI_TO_UNI(lpBinaryPathName);
    ANSI_TO_UNI(lpLoadOrderGroup);
    ANSI_TO_UNI(lpdwTagId);
    ANSI_TO_UNI(lpDependencies);
    ANSI_TO_UNI(lpServiceStartName);
    ANSI_TO_UNI(lpPassword);
    ANSI_TO_UNI(lpDisplayName);

    if (! NT_SUCCESS(status)) {

        ok = FALSE;
        error = ERROR_NOT_ENOUGH_MEMORY;

    } else {

        ok = Scm_ChangeServiceConfigW(
            hService,
            dwServiceType, dwStartType, dwErrorControl,
            lpBinaryPathNameW.Buffer, lpLoadOrderGroupW.Buffer,
            lpdwTagIdW.Buffer, lpDependenciesW.Buffer,
            lpServiceStartNameW.Buffer, lpPasswordW.Buffer,
            lpDisplayNameW.Buffer);

        error = GetLastError();
    }

    FREE_UNI(lpBinaryPathName);
    FREE_UNI(lpLoadOrderGroup);
    FREE_UNI(lpdwTagId);
    FREE_UNI(lpDependencies);
    FREE_UNI(lpServiceStartName);
    FREE_UNI(lpPassword);
    FREE_UNI(lpDisplayName);

#undef FREE_UNI
#undef ANSI_TO_UNI

    SetLastError(error);
    return ok;
}


//---------------------------------------------------------------------------
// Scm_ChangeServiceConfig2W
//---------------------------------------------------------------------------


_FX BOOL Scm_ChangeServiceConfig2W(
    SC_HANDLE hService, ULONG dwInfoLevel, void *lpInfo)
{
    WCHAR *ServiceName;

    ServiceName = Scm_GetHandleName(hService);
    if (! ServiceName)
        return FALSE;

    if (! Scm_IsBoxedService(ServiceName)) {
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

    SetLastError(0);
    return TRUE;
}

//---------------------------------------------------------------------------
// Scm_ChangeServiceConfig2A
//---------------------------------------------------------------------------


_FX BOOL Scm_ChangeServiceConfig2A(
    SC_HANDLE hService, ULONG dwInfoLevel, void *lpInfo)
{
    return Scm_ChangeServiceConfig2W(hService, dwInfoLevel, lpInfo);
}


//---------------------------------------------------------------------------
// Scm_DeleteService
//---------------------------------------------------------------------------


_FX BOOL Scm_DeleteService(SC_HANDLE hService)
{
    NTSTATUS status;
    WCHAR *ServiceName;
    WCHAR *names, *name;
    ULONG len;
    HANDLE hkey, hkey2;
    UNICODE_STRING uni;
    HANDLE hMutex;

    ServiceName = Scm_GetHandleName(hService);
    if (! ServiceName)
        return FALSE;

    if (! Scm_IsBoxedService(ServiceName)) {
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

    //
    // open keys for the service itself and for SbieSvc
    //

    hkey = Scm_OpenKeyForService(ServiceName, TRUE);
    if (! hkey) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    hkey2 = Scm_OpenKeyForService(SBIESVC, TRUE);
    if (! hkey2) {
        NtClose(hkey);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    //
    // delete the service key
    //

    status = Key_NtDeleteKeyTree(hkey, TRUE);

    NtClose(hkey);

    if (! NT_SUCCESS(status)) {
        NtClose(hkey2);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    //
    // delete the service from SbieSvc list of sandboxed services
    //

    hMutex = Scm_AcquireMutex();

    len = 0;
    names = name = Scm_GetBoxedServices();
    while (*name) {
        if (_wcsicmp(name, ServiceName) == 0)
            *name = L'*';
        len += wcslen(name) + 1;
        name += wcslen(name) + 1;
    }

    ++len;
    RtlInitUnicodeString(&uni, L"SandboxedServices");
    status = NtSetValueKey(
        hkey2, &uni, 0, REG_MULTI_SZ, names, len * sizeof(WCHAR));

    Scm_ReleaseMutex(hMutex);

    NtClose(hkey2);

    Dll_Free(names);

    SetLastError(0);
    return TRUE;
}


//---------------------------------------------------------------------------
// SbieDll_StartBoxedService
//---------------------------------------------------------------------------


_FX BOOL SbieDll_StartBoxedService(const WCHAR *ServiceName, BOOLEAN WithAdd)
{
    HANDLE hkey;
    SERVICE_STATUS ss;
    SERVICE_QUERY_RPL *rpl;
    ULONG retries, error;

	WCHAR text[130];
	Sbie_snwprintf(text, 130, L"StartBoxedService; name: '%s'", ServiceName); 
    SbieApi_MonitorPut(MONITOR_OTHER, text);

    //
    // when invoked from SandboxieRpcSs to handle StartProcess,
    // specify WithAdd to add the service to the sandbox
    //

    if (WithAdd)
        Scm_AddBoxedService(ServiceName);

    //
    // get service parameters and check that the service can be started
    //

    rpl = Scm_QueryBoxedServiceByName(ServiceName, TRUE, -1);
    if (! rpl)
        return FALSE;

    if (rpl->service_status.dwCurrentState != SERVICE_STOPPED &&
        rpl->service_status.dwCurrentState != SERVICE_START_PENDING) {

        Dll_Free(rpl);
        SetLastError(ERROR_SERVICE_ALREADY_RUNNING);
        return FALSE;
    }

    if (rpl->service_status.dwServiceType & SERVICE_DRIVER) {

        SbieApi_Log(2103, L"%S [%S] (StartService)", ServiceName, Dll_BoxName);
        Dll_Free(rpl);
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

    if (! (rpl->service_status.dwServiceType & SERVICE_WIN32)) {

        Dll_Free(rpl);
        SetLastError(ERROR_SERVICE_LOGON_FAILED);
        return FALSE;
    }

    //
    // set service status - start pending
    //

    hkey = Scm_OpenKeyForService(ServiceName, TRUE);
    if (! hkey) {
        error = GetLastError();
        Dll_Free(rpl);
        SetLastError(error);
        return FALSE;
    }

    //
    // indicate the service is initializing
    //

    memzero(&ss, sizeof(SERVICE_STATUS));
    ss.dwCurrentState = SERVICE_START_PENDING;
    ss.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    ss.dwWaitHint = 5000;

    Scm_SetServiceStatus_Internal(
        hkey, HANDLE_SERVICE_STATUS, &ss, FALSE);

    CloseHandle(hkey);

    //
    // launch the service
    //

    error = Scm_StartBoxedService2(ServiceName, rpl);

    Dll_Free(rpl);

    //
    // wait for the service to indicate it has started
    //

    if (! error) {

        error = ERROR_SERVICE_LOGON_FAILED;

        for (retries = 0; retries < 40; ++retries) {

            Sleep(500);

            rpl = Scm_QueryBoxedServiceByName(ServiceName, TRUE, 0);
            if (! rpl)
                return FALSE;

            if (rpl->service_status.dwCurrentState == SERVICE_RUNNING) {
                error = 0;
                break;
            }

            Dll_Free(rpl);
        }
    }

    if (error) {

        hkey = Scm_OpenKeyForService(ServiceName, TRUE);
        if (hkey) {

            memzero(&ss, sizeof(SERVICE_STATUS));
            ss.dwCurrentState = SERVICE_STOPPED;
            ss.dwControlsAccepted = 0;
            ss.dwWaitHint = 0;

            Scm_SetServiceStatus_Internal(
                hkey, HANDLE_SERVICE_STATUS, &ss, FALSE);

            CloseHandle(hkey);
        }

        SetLastError(error);
        return FALSE;
    }

    SetLastError(0);
    return TRUE;
}


//---------------------------------------------------------------------------
// Scm_StartBoxedService2
//---------------------------------------------------------------------------


_FX ULONG Scm_StartBoxedService2(const WCHAR *name, SERVICE_QUERY_RPL *qrpl)
{
    SERVICE_RUN_REQ *req;
    MSG_HEADER *rpl;
    ULONG error, path_len, req_len;
    WCHAR *path;
    BOOL free_path;

    //
    // special handling for Sandboxie services:
    // either run directly (for SandboxieCrypto)
    // or build a path to the Sandboxie executable
    //

    free_path = FALSE;

    if (_wcsicmp(name, _bits) == 0           ||
        _wcsicmp(name, _wuauserv) == 0       ||
        _wcsicmp(name, Scm_CryptSvc) == 0) {

        PROCESS_INFORMATION pi;
        STARTUPINFO si;

        const WCHAR *ProcessName;
        BOOLEAN use_sbiesvc = TRUE;

        if (_wcsicmp(name, _bits) == 0) {
            ProcessName = SandboxieBITS;
            Scm_DeletePermissions(L"69AD4AEE-51BE-439B-A92C-86AE490E8B30");
        } else if (_wcsicmp(name, _wuauserv) == 0) {
            ProcessName = SandboxieWUAU;
            Scm_DeletePermissions(L"653C5148-4DCE-4905-9CFD-1B23662D3D9E");
        }
        else if (_wcsicmp(name, Scm_CryptSvc) == 0) {
            ProcessName = SandboxieCrypto;
            use_sbiesvc = FALSE;
        } else
            ProcessName = NULL;

        if (! use_sbiesvc) {

            memzero(&si, sizeof(STARTUPINFO));
            si.cb = sizeof(STARTUPINFO);
            si.dwFlags = STARTF_FORCEOFFFEEDBACK;

            if (! SbieDll_RunFromHome(ProcessName, NULL, &si, &pi))
                return ERROR_SERVICE_LOGON_FAILED;

            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);

            return ERROR_SUCCESS;
        }

        si.lpReserved = NULL;
        if (SbieDll_RunFromHome(ProcessName, NULL, &si, NULL)) {

            path = (WCHAR *)si.lpReserved;
            if (path)
                free_path = TRUE;
        }
    }

    if (! free_path) {

        path = Scm_GetServiceConfigString(qrpl, 'P');
    }

    //
    // otherwise start service through SbieSvc
    //

    path_len = (wcslen(path) + 1) * sizeof(WCHAR);
    req_len = sizeof(SERVICE_RUN_REQ) + path_len;

    req = Dll_Alloc(req_len);
    req->h.length = req_len;
    req->h.msgid = MSGID_SERVICE_RUN;

    req->type = qrpl->service_status.dwServiceType;

    if (_wcsicmp(name, L"McAfee SiteAdvisor Service") == 0) {

        //
        // the McAfee SiteAdvisor Service is a WIN32_SHARE_PROCESS service
        // when it is part of the Total Protection suite, so we need to
        // make sure it will always be treated as SERVICE_WIN32_OWN_PROCESS
        //

        req->type = SERVICE_WIN32_OWN_PROCESS;
    }

    wcsncpy(req->name, name, 64);
    req->name[63] = L'\0';

    req->devmap[0] = L'\0';
    File_GetSetDeviceMap(req->devmap);

    req->path_len = path_len;
    memcpy(req->path, path, path_len);

    rpl = (MSG_HEADER *)SbieDll_CallServer(&req->h);
    if (! rpl)
        error = ERROR_NOT_ENOUGH_MEMORY;
    else {
        error = rpl->status;
        Dll_Free(rpl);
    }

    //
    // finish
    //

    if (free_path)
        HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, path);

    return error;
}


//---------------------------------------------------------------------------
// Scm_StartServiceW
//---------------------------------------------------------------------------


_FX BOOL Scm_StartServiceW(
    SC_HANDLE hService, DWORD dwNumServiceArgs, void *lpServiceArgVector)
{
    union {
        SERVICE_START_REQ req;
        WCHAR req_space[384];
    } u;
    SERVICE_START_RPL *rpl;
    WCHAR *ServiceName;

    ServiceName = Scm_GetHandleName(hService);
    if (! ServiceName)
        return FALSE;

    WCHAR text[130];
	Sbie_snwprintf(text, 130, L"StartService: %s", ServiceName);
    SbieApi_MonitorPut(MONITOR_OTHER, text);

    if (Scm_IsBoxedService(ServiceName))
        return SbieDll_StartBoxedService(ServiceName, FALSE);

    u.req.name_len = wcslen(ServiceName);
    wcscpy(u.req.name, ServiceName);

    u.req.h.length = sizeof(SERVICE_START_REQ)
                   + (u.req.name_len + 1) * sizeof(WCHAR);
    u.req.h.msgid = MSGID_SERVICE_START;

    rpl = (SERVICE_START_RPL *)SbieDll_CallServer(&u.req.h);

    if (rpl) {

        ULONG rpl_status = rpl->h.status;
        Dll_Free(rpl);

        SetLastError(rpl_status);

        if (rpl_status == 0)
            return TRUE;
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// Scm_StartServiceA
//---------------------------------------------------------------------------


_FX BOOL Scm_StartServiceA(
    SC_HANDLE hService, DWORD dwNumServiceArgs, void *lpServiceArgVector)
{
    return Scm_StartServiceW(hService, dwNumServiceArgs, lpServiceArgVector);
}


//---------------------------------------------------------------------------
// Scm_ServiceMainThread
//---------------------------------------------------------------------------


_FX ULONG Scm_ServiceMainThread(ULONG_PTR *args)
{
    WCHAR text[130];
    Sbie_snwprintf(text, 130, L"ServiceMainThread; begin");
    SbieApi_MonitorPut(MONITOR_OTHER, text);

    typedef void (*P_Main)(ULONG argc, void **argv);
    ((P_Main)args[0])(1, (void **)&args[1]);

    Sbie_snwprintf(text, 130, L"ServiceMainThread; end");
    SbieApi_MonitorPut(MONITOR_OTHER, text);

    //
    // if this is the MSI Server, then wait for all our callers to end
    //

    if (Scm_IsMsiServer) {
        Scm_SetupMsiWaiter();
    }

    return 0;
}


//---------------------------------------------------------------------------
// Scm_StartServiceCtrlDispatcherX
//---------------------------------------------------------------------------


_FX BOOL Scm_StartServiceCtrlDispatcherX(
    const void *ServiceTable, BOOLEAN IsUnicode)
{
    static const WCHAR *ServiceName_EnvVar =
        L"00000000_" SBIE L"_SERVICE_NAME";
    WCHAR *ServiceName;
    WCHAR *Buffer;
    UNICODE_STRING uni;
    void *args[3];
    ULONG ThreadId;
    HANDLE hEvent;
    SERVICE_STATUS ss;
    NTSTATUS status;
    IO_STATUS_BLOCK iosb;

    //
    // get the service name
    //
    // SERVICE_WIN32_OWN_PROCESS service is not required to pass the
    // correct service name, so we have Scm_StartBoxedService2 pass
    // the service name to SbieSvc, which passes that name to us
    // through an environment variable
    //

    if (GetEnvironmentVariable(ServiceName_EnvVar, NULL, 0) != 0) {

        ServiceName = Dll_Alloc(1024 * sizeof(WCHAR));
        GetEnvironmentVariable(ServiceName_EnvVar, ServiceName, 1020);

        SetEnvironmentVariable(ServiceName_EnvVar, NULL);

        //
        // potential problem:
        //
        // it is possible the service program actually hosts more than
        // one service, but we faked type == SERVICE_WIN32_OWN_PROCESS
        // in Scm_StartBoxedService2.  in this case the service table
        // will contain more than one service, and the one we want to
        // run is not necessarily the first entry in the table.
        //

    } else {

        BOOLEAN TooMany;

        if (IsUnicode) {

            SERVICE_TABLE_ENTRYW *svc = (SERVICE_TABLE_ENTRYW *)ServiceTable;
            ServiceName = svc->lpServiceName;
            TooMany = (svc[1].lpServiceName || svc[1].lpServiceProc);

        } else {

            SERVICE_TABLE_ENTRYA *svc = (SERVICE_TABLE_ENTRYA *)ServiceTable;

            ANSI_STRING ansi;
            RtlInitString(&ansi, svc->lpServiceName);
            RtlAnsiStringToUnicodeString(&uni, &ansi, TRUE);
            ServiceName = uni.Buffer;

            TooMany = (svc[1].lpServiceName || svc[1].lpServiceProc);
        }

        if (TooMany) {

            SbieApi_Log(2205, L"StartServiceCtrlDispatcher");
            //SetLastError(ERROR_INVALID_PARAMETER);
            //return FALSE;
        }
    }

    WCHAR text[130];
	Sbie_snwprintf(text, 130, L"StartServiceCtrlDispatcher; name: '%s'", ServiceName);
    SbieApi_MonitorPut(MONITOR_OTHER, text);

    //
    // open the key for the service
    //

    Scm_ServiceKey = Scm_OpenKeyForService(ServiceName, TRUE);
    if (! Scm_ServiceKey) {
        SetLastError(ERROR_SERVICE_DOES_NOT_EXIST);
        return FALSE;
    }

    //
    // indicate the service is initializing
    //

    memzero(&ss, sizeof(SERVICE_STATUS));
    ss.dwCurrentState = SERVICE_START_PENDING;
    ss.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    ss.dwWaitHint = 5000;

    Scm_SetServiceStatus(HANDLE_SERVICE_STATUS, &ss);

    //
    // launch ServiceMain to initialize
    //

    if (IsUnicode) {

        args[0] = ((SERVICE_TABLE_ENTRYW *)ServiceTable)->lpServiceProc;
        args[1] = ((SERVICE_TABLE_ENTRYW *)ServiceTable)->lpServiceName;
        args[2] = NULL;

    } else {

        args[0] = ((SERVICE_TABLE_ENTRYA *)ServiceTable)->lpServiceProc;
        args[1] = ((SERVICE_TABLE_ENTRYA *)ServiceTable)->lpServiceName;
        args[2] = NULL;
    }

	if (_wcsicmp(ServiceName, Scm_MsiServer) == 0) {

		Scm_IsMsiServer = TRUE;
        Scm_SetupMsiHooks();
	}

	HANDLE ThreadHandle = CreateThread(NULL, 0, Scm_ServiceMainThread, args, 0, &ThreadId);
	if (ThreadHandle)
		CloseHandle(ThreadHandle);
	else
		Scm_Stopped = TRUE;

    //
    // main loop:  wait for changes on the service key
    //

    hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (! hEvent) {
        SbieApi_Log(2211, ServiceName);
        return FALSE;
    }

    Buffer = Dll_Alloc(1024);
    RtlInitUnicodeString(&uni, SBIE L"_ControlCode");

    status = NtNotifyChangeKey(
        Scm_ServiceKey, hEvent, NULL, NULL, &iosb,
        REG_NOTIFY_CHANGE_LAST_SET, FALSE, Buffer, 1024, TRUE);

    if (!NT_SUCCESS(status)) {
        SbieApi_Log(2211, ServiceName);
        return FALSE;
    }

    while (! Scm_Stopped) {

        union {
            KEY_VALUE_PARTIAL_INFORMATION info;
            WCHAR info_space[64];
        } u;
        ULONG len;

        // Wait for the reg value notification event, or 1 sec.
        status = WaitForSingleObject(hEvent, 1000);

        if (NT_SUCCESS(status)) {

            Scm_DiscardKeyCache(ServiceName);

            status = NtQueryValueKey(
                Scm_ServiceKey, &uni, KeyValuePartialInformation,
                &u.info, sizeof(u), &len);

            if (NT_SUCCESS(status) &&
                u.info.Type == REG_DWORD && u.info.DataLength == 4) {

                if (Scm_Handler)
                    Scm_Handler(*(ULONG *)u.info.Data);
                else if (Scm_HandlerEx) {
                    Scm_HandlerEx(
                        *(ULONG *)u.info.Data, 0, NULL, Scm_HandlerContext);
                }
            }

            status = NtDeleteValueKey(Scm_ServiceKey, &uni);
            if (NT_SUCCESS(status)) {           // if key was actually deleted, we need to reissue the reg notification
                status = NtNotifyChangeKey(
                    Scm_ServiceKey, hEvent, NULL, NULL, &iosb,
                    REG_NOTIFY_CHANGE_LAST_SET, FALSE, Buffer, 1024, TRUE);
            }
        }
    }

    //
    // if the service never started, report error
    //

    Sbie_snwprintf(text, 130, L"StartServiceCtrlDispatcher; result: %s", Scm_Started ? L"sucess" : L"failure");
    SbieApi_MonitorPut(MONITOR_OTHER, text);

    if (! Scm_Started) {
        SbieApi_Log(2211, ServiceName);
        return FALSE;
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Scm_StartServiceCtrlDispatcherW
//---------------------------------------------------------------------------


_FX BOOL Scm_StartServiceCtrlDispatcherW(
    const SERVICE_TABLE_ENTRYW *ServiceTable)
{
    return Scm_StartServiceCtrlDispatcherX(ServiceTable, TRUE);
}


//---------------------------------------------------------------------------
// Scm_StartServiceCtrlDispatcherA
//---------------------------------------------------------------------------


_FX BOOL Scm_StartServiceCtrlDispatcherA(
    const SERVICE_TABLE_ENTRYA *ServiceTable)
{
    return Scm_StartServiceCtrlDispatcherX(ServiceTable, FALSE);
}


//---------------------------------------------------------------------------
// Scm_RegisterServiceCtrlHandlerW
//---------------------------------------------------------------------------


_FX SERVICE_STATUS_HANDLE Scm_RegisterServiceCtrlHandlerW(
    WCHAR *ServiceName, LPHANDLER_FUNCTION HandlerProc)
{
    Scm_Handler = HandlerProc;
    return (SERVICE_STATUS_HANDLE)HANDLE_SERVICE_STATUS;
}


//---------------------------------------------------------------------------
// Scm_RegisterServiceCtrlHandlerA
//---------------------------------------------------------------------------


_FX SERVICE_STATUS_HANDLE Scm_RegisterServiceCtrlHandlerA(
    UCHAR *ServiceName, LPHANDLER_FUNCTION HandlerProc)
{
    Scm_Handler = HandlerProc;
    return (SERVICE_STATUS_HANDLE)HANDLE_SERVICE_STATUS;
}


//---------------------------------------------------------------------------
// Scm_RegisterServiceCtrlHandlerExW
//---------------------------------------------------------------------------


_FX SERVICE_STATUS_HANDLE Scm_RegisterServiceCtrlHandlerExW(
    WCHAR *ServiceName, LPHANDLER_FUNCTION_EX HandlerProc, void *Context)
{
    Scm_HandlerEx = HandlerProc;
    Scm_HandlerContext = Context;
    return (SERVICE_STATUS_HANDLE)HANDLE_SERVICE_STATUS;
}


//---------------------------------------------------------------------------
// Scm_RegisterServiceCtrlHandlerExA
//---------------------------------------------------------------------------


_FX SERVICE_STATUS_HANDLE Scm_RegisterServiceCtrlHandlerExA(
    UCHAR *ServiceName, LPHANDLER_FUNCTION_EX HandlerProc, void *Context)
{
    Scm_HandlerEx = HandlerProc;
    Scm_HandlerContext = Context;
    return (SERVICE_STATUS_HANDLE)HANDLE_SERVICE_STATUS;
}


//---------------------------------------------------------------------------
// Scm_RegisterServiceCtrlHandlerHelp
//---------------------------------------------------------------------------


#ifdef _WIN64


_FX SERVICE_STATUS_HANDLE Scm_RegisterServiceCtrlHandlerHelp(
    UCHAR *ServiceName, ULONG_PTR HandlerProc, void *Context, ULONG Flags)
{
    if (Flags == 0) {
        Scm_Handler = (LPHANDLER_FUNCTION)HandlerProc;
    } else if (Flags == 2) {
        Scm_HandlerEx = (LPHANDLER_FUNCTION_EX)HandlerProc;
        Scm_HandlerContext = Context;
    } else {
        SbieApi_Log(2205, L"RegisterServiceCtrlHandlerHelp");
        SetLastError(ERROR_SERVICE_NOT_IN_EXE);
        return NULL;
    }
    return (SERVICE_STATUS_HANDLE)HANDLE_SERVICE_STATUS;
}


#endif _WIN64


//---------------------------------------------------------------------------
// Scm_SetServiceStatus_Internal
//---------------------------------------------------------------------------


_FX BOOL Scm_SetServiceStatus_Internal(
    HKEY ServiceKeyHandle,
    SERVICE_STATUS_HANDLE hServiceStatus, LPSERVICE_STATUS lpServiceStatus,
    BOOLEAN SetProcessId)
{
    UNICODE_STRING uni;
    ULONG val;

    if (hServiceStatus != HANDLE_SERVICE_STATUS) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    WCHAR text[130];
    Sbie_snwprintf(text, 130, L"SetServiceStatus; status: <%08X>", lpServiceStatus->dwCurrentState);
    SbieApi_MonitorPut(MONITOR_OTHER, text);

#define MySetValueKey()                         \
    NtSetValueKey(ServiceKeyHandle, &uni,       \
                  0, REG_DWORD, (BYTE *)&val, sizeof(ULONG))

    RtlInitUnicodeString(&uni, SBIE L"_ProcessId");
    if (SetProcessId) {
        val = Dll_ProcessId;
        MySetValueKey();
    } else
        NtDeleteValueKey(ServiceKeyHandle, &uni);

    RtlInitUnicodeString(&uni, SBIE L"_CurrentState");
    val = lpServiceStatus->dwCurrentState;
    MySetValueKey();

    RtlInitUnicodeString(&uni, SBIE L"_StartTicks");
    if (lpServiceStatus->dwCurrentState == SERVICE_START_PENDING) {
        val = GetTickCount();
        MySetValueKey();
    } else
        NtDeleteValueKey(ServiceKeyHandle, &uni);

    RtlInitUnicodeString(&uni, SBIE L"_ControlsAccepted");
    val = lpServiceStatus->dwControlsAccepted;
    MySetValueKey();

    RtlInitUnicodeString(&uni, SBIE L"_Win32ExitCode");
    val = lpServiceStatus->dwWin32ExitCode;
    MySetValueKey();

    RtlInitUnicodeString(&uni, SBIE L"_ServiceSpecificExitCode");
    val = lpServiceStatus->dwServiceSpecificExitCode;
    MySetValueKey();

    RtlInitUnicodeString(&uni, SBIE L"_CheckPoint");
    val = lpServiceStatus->dwCheckPoint;
    MySetValueKey();

    RtlInitUnicodeString(&uni, SBIE L"_WaitHint");
    val = lpServiceStatus->dwWaitHint;
    MySetValueKey();

#undef MySetValueKey

    if (lpServiceStatus->dwCurrentState == SERVICE_RUNNING)
        Scm_Started = TRUE;
    else if (lpServiceStatus->dwCurrentState == SERVICE_STOPPED)
        Scm_Stopped = TRUE;

    return TRUE;
}


//---------------------------------------------------------------------------
// Scm_SetServiceStatus
//---------------------------------------------------------------------------


_FX BOOL Scm_SetServiceStatus(
    SERVICE_STATUS_HANDLE hServiceStatus, LPSERVICE_STATUS lpServiceStatus)
{
    return Scm_SetServiceStatus_Internal(
        Scm_ServiceKey, hServiceStatus, lpServiceStatus, TRUE);
}


//---------------------------------------------------------------------------
// Scm_ControlService
//---------------------------------------------------------------------------


_FX BOOL Scm_ControlService(
    SC_HANDLE hService, DWORD dwControl, SERVICE_STATUS *lpServiceStatus)
{
    NTSTATUS status;
    WCHAR *ServiceName;
    HANDLE hkey;
    SERVICE_QUERY_RPL *rpl;
    UNICODE_STRING uni;
    ULONG val, retries;
    union {
        KEY_VALUE_PARTIAL_INFORMATION info;
        WCHAR info_space[64];
    } u;

    //
    // get service name
    //

    ServiceName = Scm_GetHandleName(hService);
    if (! ServiceName)
        return FALSE;

    if (! Scm_IsBoxedService(ServiceName)) {

        if (dwControl == SERVICE_CONTROL_CONTINUE ||
            dwControl == SERVICE_CONTROL_INTERROGATE) {

            return Scm_QueryServiceStatus(hService, lpServiceStatus);

        } else {

            SetLastError(ERROR_ACCESS_DENIED);
            return FALSE;
        }
    }

    //
    // verify that the service is in the running or paused state
    //

    rpl = Scm_QueryBoxedServiceByName(ServiceName, TRUE, 0);
    if (! rpl)
        return FALSE;
    val = rpl->service_status.dwCurrentState;
    Dll_Free(rpl);

    if (val == SERVICE_STOPPED) {
        SetLastError(ERROR_SERVICE_NOT_ACTIVE);
        return FALSE;
    }

    if (val != SERVICE_RUNNING && val != SERVICE_PAUSED) {
        SetLastError(ERROR_SERVICE_CANNOT_ACCEPT_CTRL);
        return FALSE;
    }

    //
    // open the key for the service and transmit the control
    //

    hkey = Scm_OpenKeyForService(ServiceName, TRUE);
    if (! hkey) {
        SetLastError(ERROR_SERVICE_REQUEST_TIMEOUT);
        return FALSE;
    }

    RtlInitUnicodeString(&uni, SBIE L"_ControlCode");
    val = dwControl;
    status = NtSetValueKey(
        hkey, &uni, 0, REG_DWORD, (BYTE *)&val, sizeof(ULONG));
    if (! NT_SUCCESS(status)) {
        NtClose(hkey);
        SetLastError(ERROR_SERVICE_REQUEST_TIMEOUT);
        return FALSE;
    }

    //
    // wait up for the value to disappear, this indicates the
    // service has picked up the control
    //

    for (retries = 0; retries < 40; ++retries) {

        Scm_DiscardKeyCache(ServiceName);

        status = NtQueryValueKey(
            hkey, &uni, KeyValuePartialInformation,
            &u.info, sizeof(u), &val);

        if (NT_SUCCESS(status)) {

            Sleep(500);
            continue;
        }

        break;
    }

    NtClose(hkey);

    if (status != STATUS_OBJECT_NAME_NOT_FOUND) {
        SetLastError(ERROR_SERVICE_REQUEST_TIMEOUT);
        return FALSE;
    }

    return Scm_QueryServiceStatus(hService, lpServiceStatus);
}


//---------------------------------------------------------------------------
// Scm_DeletePermissions
//---------------------------------------------------------------------------


_FX void Scm_DeletePermissions(const WCHAR *AppIdGuid)
{
    //
    // delete LaunchPermission and AccessPermission in the APPID key
    // for the service we're about to start
    //

    Key_DeleteValueFromCLSID(L"appid", AppIdGuid, L"LaunchPermission");
    Key_DeleteValueFromCLSID(L"appid", AppIdGuid, L"AccessPermission");
}
