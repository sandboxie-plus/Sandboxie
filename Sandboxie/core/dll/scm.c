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
// Service Control Manager
//---------------------------------------------------------------------------

#include "dll.h"

#include <windows.h>
#include <aclapi.h>
#include <stdio.h>
#include "dll.h"
#include "common/win32_ntddk.h"
#include "core/svc/ServiceWire.h"
#include "common/my_version.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define HANDLE_SERVICE_MANAGER          ((SC_HANDLE)0x12340001)
#define HANDLE_SERVICE_STATUS           ((SERVICE_STATUS_HANDLE)0x12340003)
#define HANDLE_SERVICE_LOCK             ((SC_LOCK)0x12340005)
#define HANDLE_EVENT_LOG                ((SC_LOCK)0x12340007)


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN Scm_HookRegisterServiceCtrlHandler(void);

//---------------------------------------------------------------------------

static SC_HANDLE Scm_OpenSCManagerA(
    UCHAR *lpMachineName,
    UCHAR *lpDatabaseName,
    DWORD dwDesiredAccess);

static SC_HANDLE Scm_OpenSCManagerW(
    WCHAR *lpMachineName,
    WCHAR *lpDatabaseName,
    DWORD dwDesiredAccess);

static SC_HANDLE Scm_OpenServiceA(
    SC_HANDLE hSCManager,
    const UCHAR *lpServiceName,
    DWORD dwDesiredAccess);

static SC_HANDLE Scm_OpenServiceW(
    SC_HANDLE hSCManager,
    const WCHAR *lpServiceName,
    DWORD dwDesiredAccess);

static BOOL Scm_CloseServiceHandle(SC_HANDLE hSCObject);


//---------------------------------------------------------------------------


static WCHAR *Scm_GetHandleName(SC_HANDLE hService);

static BOOLEAN Scm_IsBoxedService(const WCHAR *ServiceName);

static WCHAR *Scm_GetBoxedServices(void);

static WCHAR *Scm_GetAllServices(void);

static void Scm_DiscardKeyCache(const WCHAR *ServiceName);

static BOOLEAN Scm_DllHack(HMODULE module, const WCHAR *svcname);


//---------------------------------------------------------------------------
// Prototypes
//---------------------------------------------------------------------------


typedef SC_HANDLE (*P_OpenSCManager)(
    void *lpMachineName, void *lpDatabaseName, DWORD dwDesiredAccess);

typedef SC_HANDLE (*P_OpenService)(
    SC_HANDLE hSCManager, void *lpServiceName, DWORD dwDesiredAccess);

typedef BOOL (*P_CloseServiceHandle)(
    SC_HANDLE hSCObject);

typedef BOOL (*P_QueryServiceStatus)(
    SC_HANDLE hSCObject, SERVICE_STATUS *lpServiceStatus);

typedef BOOL (*P_QueryServiceStatusEx)(
    SC_HANDLE hService, SC_STATUS_TYPE InfoLevel,
    LPBYTE lpBuffer, DWORD cbBufSize, LPDWORD pcbBytesNeeded);

typedef BOOL (*P_QueryServiceConfig)(
    SC_HANDLE hService, void *lpServiceConfig,
    DWORD cbBufSize, LPDWORD pcbBytesNeeded);

typedef BOOL (*P_QueryServiceConfig2)(
    SC_HANDLE hService, DWORD InfoLevel, LPBYTE lpBuffer,
    DWORD cbBufSize, LPDWORD pcbBytesNeeded);

typedef BOOL (*P_EnumServicesStatus)(
    SC_HANDLE hSCManager, DWORD dwServiceType, DWORD dwServiceState,
    void *lpServices, DWORD cbBufSize, LPDWORD pcbBytesNeeded,
    LPDWORD lpServicesReturned, LPDWORD lpResumeHandle);

typedef BOOL (*P_EnumServicesStatusEx)(
    SC_HANDLE hSCManager, SC_ENUM_TYPE InfoLevel,
    DWORD dwServiceType, DWORD dwServiceState,
    void *lpServices, DWORD cbBufSize, LPDWORD pcbBytesNeeded,
    LPDWORD lpServicesReturned, LPDWORD lpResumeHandle, void *GroupName);

typedef BOOL (*P_QueryServiceLockStatus)(
    SC_HANDLE hService, void *lpLockStatus,
    DWORD cbBufSize, LPDWORD pcbBytesNeeded);

typedef BOOL (*P_GetServiceDisplayName)(
    SC_HANDLE hSCManager, WCHAR *lpServiceName,
    WCHAR *lpDisplayName, LPDWORD lpcchBuffer);

typedef BOOL (*P_GetServiceKeyName)(
    SC_HANDLE hSCManager, WCHAR *lpDisplayName,
    WCHAR *lpServiceName, LPDWORD lpcchBuffer);

typedef BOOL (*P_EnumDependentServices)(
    SC_HANDLE hService, DWORD dwServiceState,
    LPENUM_SERVICE_STATUS lpServices, DWORD cbBufSize,
    LPDWORD pcbBytesNeeded, LPDWORD lpServicesReturned);

typedef BOOL (*P_QueryServiceObjectSecurity)(
    SC_HANDLE hService,
    SECURITY_INFORMATION dwSecurityInformation,
    PSECURITY_DESCRIPTOR lpSecurityDescriptor,
    DWORD cbBufSize, LPDWORD pcbBytesNeeded);

typedef BOOL (*P_SetServiceObjectSecurity)(
    SC_HANDLE hService,
    SECURITY_INFORMATION dwSecurityInformation,
    PSECURITY_DESCRIPTOR lpSecurityDescriptor);


//---------------------------------------------------------------------------


typedef SC_LOCK (*P_LockServiceDatabase)(SC_HANDLE hSCManager);

typedef BOOL (*P_UnlockServiceDatabase)(SC_LOCK ScLock);

typedef BOOL (*P_CreateService)(
    SC_HANDLE hSCManager,
    void *lpServiceName, void *lpDisplayName, ULONG dwDesiredAccess,
    ULONG dwServiceType, ULONG dwStartType, ULONG dwErrorControl,
    void *lpBinaryPathName, void *lpLoadOrderGroup, void *lpdwTagId,
    void *lpDependencies, void *lpServiceStartName, void *lpPassword);

typedef BOOL (*P_ChangeServiceConfig)(
    SC_HANDLE hService,
    ULONG dwServiceType, ULONG dwStartType, ULONG dwErrorControl,
    void *lpBinaryPathName, void *lpLoadOrderGroup,
    void *lpdwTagId, void *lpDependencies,
    void *lpServiceStartName, void *lpPassword, void *lpDisplayName);

typedef BOOL (*P_ChangeServiceConfig2)(
    SC_HANDLE hService, ULONG dwInfoLevel, void *lpInfo);

typedef BOOL (*P_DeleteService)(SC_HANDLE hService);

typedef BOOL (*P_StartService)(
    SC_HANDLE hService, DWORD dwNumServiceArgs, void *lpServiceArgVector);

typedef BOOL (*P_StartServiceCtrlDispatcher)(const void *ServiceTable);

typedef SERVICE_STATUS_HANDLE (*P_RegisterServiceCtrlHandler)(
    void *ServiceName, void *HandlerProc);

typedef SERVICE_STATUS_HANDLE (*P_RegisterServiceCtrlHandlerEx)(
    void *ServiceName, void *HandlerProc, void *Context);

typedef BOOL (*P_SetServiceStatus)(
    SERVICE_STATUS_HANDLE hServiceStatus, SERVICE_STATUS *ServiceStatus);

typedef BOOL (*P_ControlService)(
    SC_HANDLE hService, DWORD dwControl, SERVICE_STATUS *lpServiceStatus);

typedef BOOL (*P_ControlServiceEx)(
    SC_HANDLE hService, DWORD dwControl,
    ULONG dwInfoLevel, void *pControlParams);


//---------------------------------------------------------------------------


typedef HANDLE (*P_RegisterEventSource)(void *ServerName, void *SourceName);

typedef BOOL (*P_DeregisterEventSource)(HANDLE hEventLog);

typedef BOOL (*P_ReportEvent)(
    HANDLE hEventLog, WORD wType, WORD wCategory, DWORD dwEventID,
    PSID lpUserSid, WORD wNumStrings, DWORD dwDataSize,
    void *Strings, void *RawData);

typedef BOOL (*P_CloseEventLog)(
    HANDLE hEventLog);

//---------------------------------------------------------------------------
// Pointers
//---------------------------------------------------------------------------


static P_OpenSCManager          __sys_OpenSCManagerW            = NULL;
static P_OpenSCManager          __sys_OpenSCManagerA            = NULL;

static P_OpenService            __sys_OpenServiceW              = NULL;
static P_OpenService            __sys_OpenServiceA              = NULL;

static P_CloseServiceHandle     __sys_CloseServiceHandle        = NULL;

static P_QueryServiceStatus     __sys_QueryServiceStatus        = NULL;

static P_QueryServiceStatusEx   __sys_QueryServiceStatusEx      = NULL;

static P_QueryServiceConfig     __sys_QueryServiceConfigW       = NULL;
static P_QueryServiceConfig     __sys_QueryServiceConfigA       = NULL;

static P_QueryServiceConfig2    __sys_QueryServiceConfig2W      = NULL;
static P_QueryServiceConfig2    __sys_QueryServiceConfig2A      = NULL;

static P_EnumServicesStatus     __sys_EnumServicesStatusW       = NULL;
static P_EnumServicesStatus     __sys_EnumServicesStatusA       = NULL;

static P_EnumServicesStatusEx   __sys_EnumServicesStatusExW     = NULL;
static P_EnumServicesStatusEx   __sys_EnumServicesStatusExA     = NULL;

static P_QueryServiceLockStatus __sys_QueryServiceLockStatusW   = NULL;
static P_QueryServiceLockStatus __sys_QueryServiceLockStatusA   = NULL;

static P_GetServiceDisplayName  __sys_GetServiceDisplayNameW    = NULL;
static P_GetServiceDisplayName  __sys_GetServiceDisplayNameA    = NULL;

static P_GetServiceKeyName      __sys_GetServiceKeyNameW        = NULL;
static P_GetServiceKeyName      __sys_GetServiceKeyNameA        = NULL;

static P_EnumDependentServices  __sys_EnumDependentServicesW    = NULL;
static P_EnumDependentServices  __sys_EnumDependentServicesA    = NULL;

static P_QueryServiceObjectSecurity
                                __sys_QueryServiceObjectSecurity= NULL;

static P_SetServiceObjectSecurity
                                __sys_SetServiceObjectSecurity  = NULL;


//---------------------------------------------------------------------------


static P_LockServiceDatabase    __sys_LockServiceDatabase       = NULL;

static P_UnlockServiceDatabase  __sys_UnlockServiceDatabase     = NULL;

static P_CreateService          __sys_CreateServiceW            = NULL;
static P_CreateService          __sys_CreateServiceA            = NULL;

static P_ChangeServiceConfig    __sys_ChangeServiceConfigW      = NULL;
static P_ChangeServiceConfig    __sys_ChangeServiceConfigA      = NULL;

static P_ChangeServiceConfig2   __sys_ChangeServiceConfig2W     = NULL;
static P_ChangeServiceConfig2   __sys_ChangeServiceConfig2A     = NULL;

static P_DeleteService          __sys_DeleteService             = NULL;

static P_StartService           __sys_StartServiceW             = NULL;
static P_StartService           __sys_StartServiceA             = NULL;

static P_StartServiceCtrlDispatcher
                              __sys_StartServiceCtrlDispatcherW = NULL;
static P_StartServiceCtrlDispatcher
                              __sys_StartServiceCtrlDispatcherA = NULL;

static P_RegisterServiceCtrlHandler
                              __sys_RegisterServiceCtrlHandlerW = NULL;
static P_RegisterServiceCtrlHandler
                              __sys_RegisterServiceCtrlHandlerA = NULL;

static P_RegisterServiceCtrlHandlerEx
                            __sys_RegisterServiceCtrlHandlerExW = NULL;
static P_RegisterServiceCtrlHandlerEx
                            __sys_RegisterServiceCtrlHandlerExA = NULL;

static P_SetServiceStatus       __sys_SetServiceStatus          = NULL;

static P_ControlService         __sys_ControlService            = NULL;

static P_ControlServiceEx       __sys_ControlServiceExW         = NULL;
static P_ControlServiceEx       __sys_ControlServiceExA         = NULL;


//---------------------------------------------------------------------------


static P_RegisterEventSource    __sys_RegisterEventSourceW      = NULL;
static P_RegisterEventSource    __sys_RegisterEventSourceA      = NULL;

static P_DeregisterEventSource  __sys_DeregisterEventSource     = NULL;

static P_ReportEvent            __sys_ReportEventW              = NULL;
static P_ReportEvent            __sys_ReportEventA              = NULL;

static P_CloseEventLog          __sys_CloseEventLog             = NULL;

//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static const WCHAR *Scm_ServicesKeyPath =
    L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Services\\";

static const WCHAR *_MsiServerInUseEventName =
    SBIE L"_WindowsInstallerInUse";

static const WCHAR *Scm_MsiServer     = L"MSIServer";
       const WCHAR *Scm_CryptSvc      = L"cryptsvc";

static const WCHAR *SandboxieBITS     = SANDBOXIE L"BITS.exe";
static const WCHAR *SandboxieWUAU     = SANDBOXIE L"WUAU.exe";
static const WCHAR *SandboxieCrypto   = SANDBOXIE L"Crypto.exe";

static const WCHAR *_bits             = L"bits";
static const WCHAR *_wuauserv         = L"wuauserv";
static const WCHAR *_TrustedInstaller = L"TrustedInstaller";


//---------------------------------------------------------------------------
// SCM_IMPORT
//---------------------------------------------------------------------------


#define SCM_IMPORT_XX(base,suffix) {                                    \
    const UCHAR *ProcName = #base#suffix;                               \
    __sys_##base##suffix =                                              \
    (P_##base)Ldr_GetProcAddrNew(DllName_advapi32, L#base L#suffix,#base #suffix);   \
    }

#define SCM_IMPORT_(m,base,suffix) {                                      \
    const UCHAR *ProcName = #base#suffix;                               \
    __sys_##base##suffix =                                              \
    (P_##base)Ldr_GetProcAddrNew(m, L#base L#suffix,#base #suffix);   \
    if (! __sys_##base##suffix) {                                       \
        SbieApi_Log(2303, L"%s (ADV)", ProcName);                       \
        return FALSE;                                                   \
    }                                                                   \
    }

#define SCM_IMPORT___(base) SCM_IMPORT_(DllName_advapi32,base,)
#define SCM_IMPORT_W8___(base) SCM_IMPORT_(DllName_sechost,base,)
#define SCM_IMPORT_AW(base) SCM_IMPORT_(DllName_advapi32,base,A) SCM_IMPORT_(DllName_advapi32,base,W)


//---------------------------------------------------------------------------
// SBIEDLL_HOOK_SCM
//---------------------------------------------------------------------------


#define SBIEDLL_HOOK_SCM(proc)                              \
    *(ULONG_PTR *)&__sys_##proc = (ULONG_PTR)               \
        SbieDll_Hook(#proc, __sys_##proc, Scm_##proc);      \
    if (! __sys_##proc) return FALSE;


//---------------------------------------------------------------------------
// Scm (other modules)
//---------------------------------------------------------------------------


#include "scm_query.c"
#include "scm_create.c"
#include "scm_event.c"
#include "scm_notify.c"
#include "scm_misc.c"


//---------------------------------------------------------------------------
// Undo Unicode defines from windows.h
//---------------------------------------------------------------------------


#ifdef OpenSCManager
#undef OpenSCManager
#endif

#ifdef OpenService
#undef OpenService
#endif

#ifdef QueryServiceConfig
#undef QueryServiceConfig
#endif

#ifdef QueryServiceConfig2
#undef QueryServiceConfig2
#endif

#ifdef EnumServicesStatus
#undef EnumServicesStatus
#endif

#ifdef EnumServicesStatusEx
#undef EnumServicesStatusEx
#endif

#ifdef QueryServiceLockStatus
#undef QueryServiceLockStatus
#endif

#ifdef GetServiceDisplayName
#undef GetServiceDisplayName
#endif

#ifdef GetServiceKeyName
#undef GetServiceKeyName
#endif

#ifdef EnumDependentServices
#undef EnumDependentServices
#endif

#ifdef CreateService
#undef CreateService
#endif

#ifdef ChangeServiceConfig
#undef ChangeServiceConfig
#endif

#ifdef ChangeServiceConfig2
#undef ChangeServiceConfig2
#endif

#ifdef StartService
#undef StartService
#endif

#ifdef StartServiceCtrlDispatcher
#undef StartServiceCtrlDispatcher
#endif

#ifdef RegisterServiceCtrlHandler
#undef RegisterServiceCtrlHandler
#endif

#ifdef RegisterServiceCtrlHandlerEx
#undef RegisterServiceCtrlHandlerEx
#endif

#ifdef RegisterEventSource
#undef RegisterEventSource
#endif

#ifdef ReportEvent
#undef ReportEvent
#endif


//---------------------------------------------------------------------------
// Scm_Init_AdvApi
//---------------------------------------------------------------------------


_FX BOOLEAN Scm_Init_AdvApi(HMODULE module)
{
    //
    // import functions
    //

    SCM_IMPORT_AW(OpenSCManager);
    SCM_IMPORT_AW(OpenService);
    SCM_IMPORT___(CloseServiceHandle);

    SCM_IMPORT___(QueryServiceStatus);
    SCM_IMPORT___(QueryServiceStatusEx);
    SCM_IMPORT_AW(QueryServiceConfig);
    SCM_IMPORT_AW(QueryServiceConfig2);
    SCM_IMPORT_AW(EnumServicesStatus);
    SCM_IMPORT_AW(EnumServicesStatusEx);
    SCM_IMPORT_AW(QueryServiceLockStatus);
    SCM_IMPORT_AW(GetServiceDisplayName);
    SCM_IMPORT_AW(GetServiceKeyName);
    SCM_IMPORT_AW(EnumDependentServices);
    SCM_IMPORT___(QueryServiceObjectSecurity);
    SCM_IMPORT___(SetServiceObjectSecurity);

    SCM_IMPORT___(LockServiceDatabase);
    SCM_IMPORT___(UnlockServiceDatabase);
    SCM_IMPORT_AW(CreateService);
    SCM_IMPORT_AW(ChangeServiceConfig);
    SCM_IMPORT_AW(ChangeServiceConfig2);
    SCM_IMPORT___(DeleteService);
    SCM_IMPORT_AW(StartService);
    SCM_IMPORT_AW(StartServiceCtrlDispatcher);
    SCM_IMPORT_AW(RegisterServiceCtrlHandler);
    SCM_IMPORT_AW(RegisterServiceCtrlHandlerEx);
    SCM_IMPORT___(SetServiceStatus);
    SCM_IMPORT___(ControlService);

    SCM_IMPORT_XX(ControlServiceEx,W);
    SCM_IMPORT_XX(ControlServiceEx,A);

    SCM_IMPORT_AW(RegisterEventSource);
    SCM_IMPORT___(DeregisterEventSource);
    SCM_IMPORT_AW(ReportEvent);
    SCM_IMPORT___(CloseEventLog);

    //
    // hook event log functions
    //

    SBIEDLL_HOOK_SCM(RegisterEventSourceA);
    SBIEDLL_HOOK_SCM(RegisterEventSourceW);

    SBIEDLL_HOOK_SCM(DeregisterEventSource);

    SBIEDLL_HOOK_SCM(ReportEventA);
    SBIEDLL_HOOK_SCM(ReportEventW);

    SBIEDLL_HOOK_SCM(CloseEventLog);

    //
    // hook SCM functions
    //

    SBIEDLL_HOOK_SCM(OpenSCManagerA);
    SBIEDLL_HOOK_SCM(OpenSCManagerW);

    SBIEDLL_HOOK_SCM(OpenServiceA);
    SBIEDLL_HOOK_SCM(OpenServiceW);

    SBIEDLL_HOOK_SCM(CloseServiceHandle);

    SBIEDLL_HOOK_SCM(QueryServiceStatus);
    SBIEDLL_HOOK_SCM(QueryServiceStatusEx);

    SBIEDLL_HOOK_SCM(QueryServiceConfigA);
    SBIEDLL_HOOK_SCM(QueryServiceConfigW);
    SBIEDLL_HOOK_SCM(QueryServiceConfig2A);
    SBIEDLL_HOOK_SCM(QueryServiceConfig2W);

    SBIEDLL_HOOK_SCM(EnumServicesStatusA);
    SBIEDLL_HOOK_SCM(EnumServicesStatusW);
    SBIEDLL_HOOK_SCM(EnumServicesStatusExA);
    SBIEDLL_HOOK_SCM(EnumServicesStatusExW);

    SBIEDLL_HOOK_SCM(QueryServiceLockStatusA);
    SBIEDLL_HOOK_SCM(QueryServiceLockStatusW);

    SBIEDLL_HOOK_SCM(GetServiceDisplayNameA);
    SBIEDLL_HOOK_SCM(GetServiceDisplayNameW);

    SBIEDLL_HOOK_SCM(GetServiceKeyNameA);
    SBIEDLL_HOOK_SCM(GetServiceKeyNameW);

    SBIEDLL_HOOK_SCM(EnumDependentServicesA);
    SBIEDLL_HOOK_SCM(EnumDependentServicesW);

    SBIEDLL_HOOK_SCM(QueryServiceObjectSecurity);
    SBIEDLL_HOOK_SCM(SetServiceObjectSecurity);

    SBIEDLL_HOOK_SCM(LockServiceDatabase);
    SBIEDLL_HOOK_SCM(UnlockServiceDatabase);

    SBIEDLL_HOOK_SCM(CreateServiceA);
    SBIEDLL_HOOK_SCM(CreateServiceW);

    SBIEDLL_HOOK_SCM(ChangeServiceConfigA);
    SBIEDLL_HOOK_SCM(ChangeServiceConfigW);

    SBIEDLL_HOOK_SCM(ChangeServiceConfig2A);
    SBIEDLL_HOOK_SCM(ChangeServiceConfig2W);

    SBIEDLL_HOOK_SCM(DeleteService);

    SBIEDLL_HOOK_SCM(StartServiceA);
    SBIEDLL_HOOK_SCM(StartServiceW);

    SBIEDLL_HOOK_SCM(StartServiceCtrlDispatcherA);
    SBIEDLL_HOOK_SCM(StartServiceCtrlDispatcherW);

    SBIEDLL_HOOK_SCM(SetServiceStatus);

    SBIEDLL_HOOK_SCM(ControlService);

    Scm_Notify_Init(module);

    return Scm_HookRegisterServiceCtrlHandler();
}


//---------------------------------------------------------------------------
// Scm_HookRegisterServiceCtrlHandler
//---------------------------------------------------------------------------


BOOLEAN Scm_HookRegisterServiceCtrlHandler(void)
{
    static const UCHAR PrologW[] = {
        0x45, 0x33, 0xC9,                       // xor r9d,r9d
        0x45, 0x33, 0xC0,                       // xor r8d,r8d
        0xE9                                    // jmp ...
    };
    static const UCHAR PrologExW[] = {
        0x41, 0xB9, 0x02, 0x00, 0x00, 0x00,     // mov r9d,2
        0xE9                                    // jmp ...
    };
    BOOLEAN HookedRegisterServiceCtrlHandler = FALSE;

#ifdef _WIN64

    //
    // on 64-bit Windows, ADVAPI32!RegisterServiceCtrlHandlerW is an 11-byte
    // function embedded in the code space of another function, so to prevent
    // overwriting the other function, we instead hook the internal function
    // ADVAPI32!RegisterServiceCtrlHandlerHelp
    //

    if (memcmp(__sys_RegisterServiceCtrlHandlerW, PrologW, 7) == 0 &&
        memcmp(__sys_RegisterServiceCtrlHandlerExW, PrologExW, 7) == 0) {

        ULONG64 AddrW =
            (ULONG64)(ULONG_PTR)__sys_RegisterServiceCtrlHandlerW
            + 3 + 3 + 5 +       // xor, xor, jmp
            *(LONG *)((UCHAR *)__sys_RegisterServiceCtrlHandlerW + 7);
        ULONG64 AddrExW =
            (ULONG64)(ULONG_PTR)__sys_RegisterServiceCtrlHandlerExW
            + 6 + 5 +           // mov, jmp
            *(LONG *)((UCHAR *)__sys_RegisterServiceCtrlHandlerExW + 7);

        if (AddrW == AddrExW) {

            void *__sys_RegisterServiceCtrlHandlerHelp = (void *)AddrW;

            SBIEDLL_HOOK_SCM(RegisterServiceCtrlHandlerHelp);

            HookedRegisterServiceCtrlHandler = TRUE;
        }
    }

#endif _WIN64

    //
    // otherwise hook the four functions normally
    //

    if (! HookedRegisterServiceCtrlHandler) {

        SBIEDLL_HOOK_SCM(RegisterServiceCtrlHandlerA);
        SBIEDLL_HOOK_SCM(RegisterServiceCtrlHandlerW);
        SBIEDLL_HOOK_SCM(RegisterServiceCtrlHandlerExA);
        SBIEDLL_HOOK_SCM(RegisterServiceCtrlHandlerExW);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Scm_OpenSCManagerW
//---------------------------------------------------------------------------


_FX SC_HANDLE Scm_OpenSCManagerW(
    WCHAR *lpMachineName,
    WCHAR *lpDatabaseName,
    DWORD dwDesiredAccess)
{
    if (Secure_IsRestrictedToken(TRUE)) {
        SetLastError(ERROR_ACCESS_DENIED);
        return NULL;
    }

    return HANDLE_SERVICE_MANAGER;
}


//---------------------------------------------------------------------------
// Scm_OpenSCManagerA
//---------------------------------------------------------------------------


_FX SC_HANDLE Scm_OpenSCManagerA(
    UCHAR *lpMachineName,
    UCHAR *lpDatabaseName,
    DWORD dwDesiredAccess)
{
    return Scm_OpenSCManagerW(NULL, NULL, dwDesiredAccess);
}


//---------------------------------------------------------------------------
// Scm_OpenServiceW
//---------------------------------------------------------------------------


_FX SC_HANDLE Scm_OpenServiceW(
    SC_HANDLE hSCManager,
    const WCHAR *lpServiceName,
    DWORD dwDesiredAccess)
{
    WCHAR *name;
    BOOLEAN ok = FALSE;

    if (hSCManager != HANDLE_SERVICE_MANAGER) {
        SetLastError(ERROR_INVALID_HANDLE);
        return (SC_HANDLE)0;
    }

    if ((! lpServiceName) || (! *lpServiceName)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (SC_HANDLE)0;
    }

    //
    // open the service if we know its name, first check in the sandbox,
    // and if not found, outside the sandbox
    //

    Scm_DiscardKeyCache(lpServiceName);

    if (Scm_IsBoxedService(lpServiceName)) {

        HANDLE hkey = Scm_OpenKeyForService(lpServiceName, FALSE);
        if (hkey) {
            NtClose(hkey);
            ok = TRUE;
        }

    } else {

        SERVICE_QUERY_RPL *rpl = (SERVICE_QUERY_RPL *)
            Scm_QueryServiceByName(lpServiceName, FALSE, FALSE);
        if (rpl) {
            Dll_Free(rpl);
            ok = TRUE;
        }
    }

    if (! ok) {
        // either Scm_OpenKeyForService or Scm_QueryServiceByName
        // has already called SetLastError
        return (SC_HANDLE)0;
    }

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
}


//---------------------------------------------------------------------------
// Scm_OpenServiceA
//---------------------------------------------------------------------------


_FX SC_HANDLE Scm_OpenServiceA(
    SC_HANDLE hSCManager,
    const UCHAR *lpServiceName,
    DWORD dwDesiredAccess)
{
    SC_HANDLE h;
    ANSI_STRING ansi;
    UNICODE_STRING uni;
    DWORD err;

    uni.Buffer = NULL;
    if (lpServiceName) {
        RtlInitString(&ansi, lpServiceName);
        RtlAnsiStringToUnicodeString(&uni, &ansi, TRUE);
    }

    h = Scm_OpenServiceW(hSCManager, uni.Buffer, dwDesiredAccess);
    err = GetLastError();

    if (uni.Buffer)
        RtlFreeUnicodeString(&uni);

    SetLastError(err);
    return h;
}


//---------------------------------------------------------------------------
// Scm_CloseServiceHandle
//---------------------------------------------------------------------------


_FX BOOL Scm_CloseServiceHandle(SC_HANDLE hSCObject)
{
    BOOL ok = FALSE;

    if (hSCObject == HANDLE_SERVICE_MANAGER)
        ok = TRUE;
    else if (Scm_GetHandleName(hSCObject)) {
        Scm_Notify_CloseHandle(hSCObject);
        Dll_Free(hSCObject);
        ok = TRUE;
    }

    if (ok)
        SetLastError(0);
    else
        SetLastError(ERROR_INVALID_HANDLE);
    return ok;
}


//---------------------------------------------------------------------------
// Scm_GetHandleName
//---------------------------------------------------------------------------


_FX WCHAR *Scm_GetHandleName(SC_HANDLE hService)
{
    WCHAR *name = NULL;
	// fix-me: refactory include of SC_HANDLE_
	if (hService == /*SC_HANDLE_RPCSS*/((SC_HANDLE)0x12345672))
		return L"RpcSs";
	if (hService == /*SC_HANDLE_MSISERVER*/((SC_HANDLE)0x12345673))
		return L"MSIServer";
	if (hService == /*SC_HANDLE_EVENTSYSTEM*/((SC_HANDLE)0x12345674))
		return L"EventSystem";
    __try {
        if (hService && *(ULONG *)hService == tzuk)
            name = (WCHAR *)(((ULONG *)hService) + 1);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
    if (! name)
        SetLastError(ERROR_INVALID_HANDLE);
    return name;
}


//---------------------------------------------------------------------------
// Scm_IsBoxedService
//---------------------------------------------------------------------------


_FX BOOLEAN Scm_IsBoxedService(const WCHAR *ServiceName)
{
    WCHAR *name, *names;

    BOOLEAN found = FALSE;

    names = Scm_GetBoxedServices();

    for (name = names; *name; name += wcslen(name) + 1) {
        if (_wcsicmp(name, ServiceName) == 0) {
            found = TRUE;
            break;
        }
    }

    Dll_Free(names);

    /*
    if (_wcsicmp(ServiceName, _eventsystem) == 0) {
        //
        // SandboxieEventSys does not run well on Windows Vista,
        //  so make it available only on earlier systems
        //
        if (! __sys_ControlServiceExW)
            return TRUE;

        return TRUE;
    }*/

    if (! found) {

        if (_wcsicmp(ServiceName, Scm_MsiServer) == 0           ||
            _wcsicmp(ServiceName, _TrustedInstaller) == 0       ||
            _wcsicmp(ServiceName, _bits) == 0                   ||
            _wcsicmp(ServiceName, _wuauserv) == 0               ||
            _wcsicmp(ServiceName, Scm_CryptSvc) == 0) {

            found = TRUE;
        }
    }

    return found;
}


//---------------------------------------------------------------------------
// Scm_GetBoxedServices
//---------------------------------------------------------------------------


_FX WCHAR *Scm_GetBoxedServices(void)
{
    WCHAR *names = NULL;
    NTSTATUS status;
    HANDLE hkey;
    UNICODE_STRING uni;
    union {
        KEY_VALUE_PARTIAL_INFORMATION info;
        WCHAR info_space[256];
    } u;
    ULONG len;

    //
    // otherwise get the MULTI_SZ list of sandboxed services from
    // the SandboxedServices value of the SbieSvc key
    //

    hkey = Scm_OpenKeyForService(SBIESVC, FALSE);
    if (! hkey)
        goto finish;

    RtlInitUnicodeString(&uni, L"SandboxedServices");
    status = NtQueryValueKey(
        hkey, &uni, KeyValuePartialInformation, &u.info, sizeof(u), &len);

    NtClose(hkey);

    if ((! NT_SUCCESS(status)) ||
                u.info.Type != REG_MULTI_SZ || u.info.DataLength == 0)
        goto finish;

    names = Dll_AllocTemp(u.info.DataLength + 8);
    memzero(names, u.info.DataLength + 8);
    memcpy(names, u.info.Data, u.info.DataLength);

finish:

    if (! names) {

        names = Dll_AllocTemp(sizeof(WCHAR) * 2);
        memzero(names, sizeof(WCHAR) * 2);
    }

    return names;
}


//---------------------------------------------------------------------------
// Scm_GetAllServices
//---------------------------------------------------------------------------


_FX WCHAR *Scm_GetAllServices(void)
{
    SERVICE_LIST_REQ req;
    SERVICE_LIST_RPL *rpl;
    WCHAR *true_names, *copy_names, *out_names;
    ULONG out_max_len, out_cur_len;
    WCHAR *small_ptr, *work_ptr, *last_ptr, last_ptr0;
    BOOLEAN free_true_names;

    //
    // get list of services inside and outside sandbox
    //

    req.h.length = sizeof(SERVICE_LIST_REQ);
    req.h.msgid = MSGID_SERVICE_LIST;
    req.type_filter = SERVICE_TYPE_ALL;
    req.state_filter = SERVICE_STATE_ALL;

    rpl = (SERVICE_LIST_RPL *)SbieDll_CallServer(&req.h);
    if (rpl && rpl->h.status == 0) {
        true_names = rpl->names;
        free_true_names = FALSE;
    } else {
        true_names = Dll_AllocTemp(sizeof(WCHAR) * 2);
        memzero(true_names, sizeof(WCHAR) * 2);
        free_true_names = TRUE;
    }

    copy_names = Scm_GetBoxedServices();

    //
    //
    //

    out_max_len = 1000;
    out_names = Dll_AllocTemp(out_max_len * sizeof(WCHAR));
    out_cur_len = 0;

    last_ptr = NULL;

    while (1) {

        //
        // find smallest service alphanumerically.
        // scan both real services and sandboxed services
        //

        small_ptr = NULL;

        for (work_ptr = true_names;
             *work_ptr;
             work_ptr += wcslen(work_ptr) + 1)
        {
            if (*work_ptr == L'*')
                continue;
            if (! small_ptr)
                small_ptr = work_ptr;
            else if (_wcsicmp(work_ptr, small_ptr) < 0)
                small_ptr = work_ptr;
        }

        if (copy_names) {

            for (work_ptr = copy_names;
                 *work_ptr;
                 work_ptr += wcslen(work_ptr) + 1)
            {
                if (*work_ptr == L'*')
                    continue;
                if (! small_ptr)
                    small_ptr = work_ptr;
                else if (_wcsicmp(work_ptr, small_ptr) < 0)
                    small_ptr = work_ptr;
            }
        }

        if (! small_ptr)
            break;

        //
        // we might hit a duplicate service name as the result of use
        // of the StartService setting, so skip the duplicates
        //

        if (last_ptr && *small_ptr == last_ptr0 &&
                _wcsicmp(small_ptr + 1, last_ptr + 1) == 0) {

            *small_ptr = L'*';
            continue;
        }

        last_ptr = small_ptr;
        last_ptr0 = *last_ptr;

        //
        // add the service to the output buffer
        //

        if (wcslen(small_ptr) + 1 + out_cur_len >= out_max_len) {

            WCHAR *new_out;
            out_max_len += 1000;
            new_out = Dll_AllocTemp(out_max_len * sizeof(WCHAR));
            memcpy(new_out, out_names, out_cur_len * sizeof(WCHAR));
            Dll_Free(out_names);
            out_names = new_out;
        }

        wcscpy(&out_names[out_cur_len], small_ptr);
        out_cur_len += wcslen(small_ptr) + 1;

        *small_ptr = L'*';
    }

    //
    // finish
    //

    out_names[out_cur_len] = L'\0';

    if (copy_names)
        Dll_Free(copy_names);

    if (free_true_names)
        Dll_Free(true_names);

    if (rpl)
        Dll_Free(rpl);

    return out_names;
}


//---------------------------------------------------------------------------
// Scm_OpenKeyForService
//---------------------------------------------------------------------------


_FX HANDLE Scm_OpenKeyForService(const WCHAR *ServiceName, BOOLEAN ForWrite)
{
    NTSTATUS status;
    WCHAR keyname[128];
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    HANDLE handle;
    ULONG error;

    wcscpy(keyname, Scm_ServicesKeyPath);
    wcscat(keyname, ServiceName);
    RtlInitUnicodeString(&objname, keyname);

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    if (ForWrite) {

        ULONG disp;
        status = NtCreateKey(
            &handle, KEY_ALL_ACCESS, &objattrs, 0, NULL, 0, &disp);

    } else {

        status = NtOpenKey(&handle, KEY_QUERY_VALUE, &objattrs);
    }

    if (NT_SUCCESS(status))
        error = 0;
    else {

        handle = NULL;
        if (status == STATUS_OBJECT_NAME_NOT_FOUND ||
            status == STATUS_OBJECT_PATH_NOT_FOUND) {

            error = ERROR_SERVICE_DOES_NOT_EXIST;
        } else
            error = RtlNtStatusToDosError(status);
    }
    SetLastError(error);

    return handle;
}

_FX int Scm_Start_Sppsvc()
{
    SC_HANDLE handle1 = Scm_OpenSCManagerW(NULL, NULL, GENERIC_READ);
    SC_HANDLE handle2 = NULL;
    int rc = 0;

    if (handle1) {
        SC_HANDLE handle2 = Scm_OpenServiceW(handle1, L"sppsvc", SERVICE_START);
        if (handle2) {
            SERVICE_STATUS lpServiceStatus;
            int count = 0;
            lpServiceStatus.dwCurrentState = 0;
            Scm_StartServiceW(handle2, 0, NULL);

            while (lpServiceStatus.dwCurrentState != SERVICE_RUNNING && count++ < 10) {
                Sleep(50);
                Scm_QueryServiceStatus(handle2, &lpServiceStatus);
                if (lpServiceStatus.dwCurrentState == 4) {
                    rc = 1;
                }
            }
        }
    }

    if (handle1)
        Scm_CloseServiceHandle(handle1);
    if (handle2)
        Scm_CloseServiceHandle(handle2);
    return rc;
}

//---------------------------------------------------------------------------
// SbieDll_IsBoxedService
//---------------------------------------------------------------------------


_FX BOOLEAN SbieDll_IsBoxedService(HANDLE hService)
{
    WCHAR *ServiceName = Scm_GetHandleName(hService);
    if (! ServiceName)
        return FALSE;
    return Scm_IsBoxedService(ServiceName);
}


//---------------------------------------------------------------------------
// Scm_DiscardKeyCache
//---------------------------------------------------------------------------


_FX void Scm_DiscardKeyCache(const WCHAR *ServiceName)
{
    WCHAR *keyname = Dll_AllocTemp(sizeof(WCHAR) * 256);
    wcscpy(keyname, Scm_ServicesKeyPath);
    wcscat(keyname, ServiceName);
    Key_DiscardMergeByPath(keyname, TRUE);
    Dll_Free(keyname);
}
