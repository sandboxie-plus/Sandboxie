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
// Terminal Services Server
//---------------------------------------------------------------------------


#include "dll.h"
#include "core/svc/terminalwire.h"
#include <WtsApi32.h>


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define LOGONID_CURRENT                 ((ULONG)-1)
#define SERVERNAME_CURRENT              ((HANDLE)NULL)
#define HANDLE_SERVER_FAKE              ((HANDLE)0x12345678)

typedef enum _WINSTATIONINFOCLASS {
    WinStationInformation = 8
} WINSTATIONINFOCLASS;

typedef struct _WINSTATIONENUMW {
    ULONG SessionId;
    WCHAR WinStationName[34];
    ULONG State;                        // (assumed)
} WINSTATIONENUMW, *PWINSTATIONENUMW;

#define WTS_VALUE_TYPE_ULONG        1
#define WTS_VALUE_TYPE_STRING       2
#define WTS_VALUE_TYPE_BINARY       3
#define WTS_VALUE_TYPE_GUID         4

typedef struct __WTS_PROPERTY_VALUE {

    unsigned short Type;
    union {
        ULONG ulVal;
        struct {
            ULONG size;
            WCHAR *pstrVal;
        } strVal;
        struct {
            ULONG size;
            char *pbVal;
        } bVal;
        GUID guidVal;
    } u;

} WTS_PROPERTY_VALUE;


//---------------------------------------------------------------------------
// Functions Prototypes
//---------------------------------------------------------------------------


typedef HWINSTA (*P_GetProcessWindowStation)(void);

typedef BOOL (*P_GetUserObjectInformation)(
    HANDLE hObj, int nIndex,
    void *pvInfo, DWORD nLength, DWORD *lpnLengthNeeded);


//---------------------------------------------------------------------------


typedef HANDLE (*P_WinStationOpenServer)(void *ServerName);

typedef BOOLEAN (*P_WinStationCloseServer)(HANDLE hServer);

typedef BOOLEAN (*P_WinStationDisconnect)(
    HANDLE hServer, ULONG LogonId, ULONG_PTR Unknown);

typedef BOOLEAN (*P_WinStationEnumerate)(
    HANDLE hServer, void *InfoBuffer, ULONG *InfoCount);

typedef BOOLEAN (*P_WinStationQueryInformation)(
    HANDLE, ULONG, WINSTATIONINFOCLASS, PVOID, ULONG, PULONG );

typedef BOOLEAN (*P_WinStationIsSessionRemoteable)(
    HANDLE hServer, ULONG LogonId, BOOLEAN *IsRemote);

typedef BOOLEAN (*P_WinStationNameFromLogonId)(
    HANDLE hServer, ULONG LogonId, WCHAR *Name);

typedef BOOLEAN (*P_WinStationGetConnectionProperty)(
    ULONG LogonId, GUID *TypeGuid, WTS_PROPERTY_VALUE **Value);

typedef BOOLEAN (*P_WinStationFreePropertyValue)(
    WTS_PROPERTY_VALUE *Value);

typedef BOOLEAN (*P_WinStationFreeMemory)(void *Memory);


//---------------------------------------------------------------------------


typedef BOOL (*P_WTSQueryUserToken)(ULONG SessionId, HANDLE *pToken);

typedef BOOL (*P_WTSEnumerateSessions)(
    HANDLE hServer, DWORD Reserved, DWORD Version,
    WTS_SESSION_INFO **ppSessionInfo, DWORD *pCount);

typedef BOOL (*P_WTSEnumerateProcesses)(
    HANDLE hServer, DWORD Reserved, DWORD Version,
    WTS_PROCESS_INFO **ppProcessInfo, DWORD *pCount);

typedef void (*P_WTSFreeMemory)(void *pMemory);

typedef BOOL (*P_WTSRegisterSessionNotification)(
    HWND hWnd, ULONG dwFlags);

typedef BOOL (*P_WTSRegisterSessionNotificationEx)(
    HANDLE hServer, HWND hWnd, ULONG dwFlags);

typedef BOOL (*P_WTSUnRegisterSessionNotification)(
    HWND hWnd);

typedef BOOL (*P_WTSUnRegisterSessionNotificationEx)(
    HANDLE hServer, HWND hWnd);


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static void *Terminal_Alloc(ULONG len);

static BOOLEAN Terminal_Free(void *pMem, P_WinStationFreeMemory __sys_Free);


//---------------------------------------------------------------------------


static HANDLE Terminal_WinStationOpenServerW(const WCHAR *ServerName);

static BOOLEAN Terminal_WinStationCloseServer(HANDLE hServer);

static BOOLEAN Terminal_WinStationDisconnect(
    HANDLE hServer, ULONG LogonId, ULONG_PTR Unknown);

static BOOLEAN Terminal_WinStationEnumerateW(
    HANDLE hServer, void *InfoBuffer, ULONG *InfoCount);

static BOOLEAN Terminal_WinStaQueryInformationW(
    HANDLE hServer, ULONG LogonId, WINSTATIONINFOCLASS InformationClass,
    void *Information, ULONG InformationLength, PULONG pReturnLength);

static BOOLEAN Terminal_WinStationIsSessionRemoteable(
    HANDLE hServer, ULONG LogonId, BOOLEAN *IsRemote);

static BOOLEAN Terminal_WinStationNameFromLogonIdW(
    HANDLE hServer, ULONG LogonId, WCHAR *Name);

static BOOLEAN Terminal_WinStationGetConnectionProperty(
    ULONG LogonId, GUID *PropType, WTS_PROPERTY_VALUE **Value);

static BOOLEAN Terminal_WinStationFreePropertyValue(
    WTS_PROPERTY_VALUE *Value);

static void Terminal_WinStationFreeMemory(void *pMemory);


//---------------------------------------------------------------------------


static BOOLEAN Terminal_DontHook(void);

static BOOL Terminal_WTSQueryUserToken(ULONG SessionId, HANDLE *pToken);

static WCHAR *Terminal_GetWindowStationName(void);

static BOOL Terminal_WTSEnumerateSessionsW(
    HANDLE hServer, DWORD Reserved, DWORD Version,
    WTS_SESSION_INFO **ppSessionInfo, DWORD *pCount);

static BOOL Terminal_WTSEnumerateProcessesW(
    HANDLE hServer, DWORD Reserved, DWORD Version,
    WTS_PROCESS_INFO **ppProcessInfo, DWORD *pCount);

static void Terminal_WTSFreeMemory(void *pMemory);

static BOOL Terminal_WTSRegisterSessionNotification(
    HWND hWnd, ULONG dwFlags);

static BOOL Terminal_WTSRegisterSessionNotificationEx(
    HANDLE hServer, HWND hWnd, ULONG dwFlags);

static BOOL Terminal_WTSUnRegisterSessionNotification(
    HWND hWnd);

static BOOL Terminal_WTSUnRegisterSessionNotificationEx(
    HANDLE hServer, HWND hWnd);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------

static P_WTSQueryUserToken      __sys_WTSQueryUserToken = 0;

static P_WinStationFreeMemory   __sys_WinStationFreeMemory      = 0;
static P_WTSFreeMemory          __sys_WTSFreeMemory             = 0;

extern const WCHAR *Ipc_SandboxieRpcSs;


//---------------------------------------------------------------------------
// Terminal_DontHook
//---------------------------------------------------------------------------


_FX BOOLEAN Terminal_DontHook(void)
{
    //
    // In in app mode we have a full token and this will be successful, hence no need for a hook
    //
    
    if (Dll_CompartmentMode) 
        return TRUE;

    //
    // On win 10 this endpoint does not exist, so hook always
    //

    if (Dll_OsBuild >= 10041)
        return FALSE;

    //
    // On older windows check the endpoint
    //

    ULONG mp_flags = SbieDll_MatchPath(L'i', L"\\RPC Control\\IcaApi");
    return (PATH_IS_OPEN(mp_flags));
}


//---------------------------------------------------------------------------
// Terminal_Alloc
//---------------------------------------------------------------------------


_FX void *Terminal_Alloc(ULONG len)
{
    //
    // we use LocalAlloc (and LocalFree in Terminal_Free) and not Dll_Alloc
    // because some callers use LocalFree instead of the WINSTA free
    // functions.  for example, MSI Server on 64-bit Windows 7.
    //

    ULONG64 *p_tzuk = LocalAlloc(LMEM_FIXED, sizeof(ULONG64) + len);
    if (! p_tzuk) {
        SbieApi_Log(2305, NULL);
        ExitProcess(-1);
    }
    *p_tzuk = tzuk;
    return (p_tzuk + 1);
}


//---------------------------------------------------------------------------
// Terminal_Free
//---------------------------------------------------------------------------


_FX BOOLEAN Terminal_Free(void *pMem, P_WinStationFreeMemory __sys_Free)
{
    ULONG64 *p_tzuk = ((ULONG64 *)pMem) - 1;
    BOOLEAN free_tzuk = FALSE;
    if (pMem) {
        __try {
            if (*p_tzuk == tzuk)
                free_tzuk = TRUE;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }
    }
    if (free_tzuk) {
        LocalFree(p_tzuk);
        return TRUE;
    } else if (__sys_Free)
        return __sys_Free(pMem);
    else
        return FALSE;
}


//---------------------------------------------------------------------------
// Terminal_GetWindowStationName
//---------------------------------------------------------------------------


_FX WCHAR *Terminal_GetWindowStationName(void)
{
    WCHAR *name = NULL;

    HWINSTA winsta = NULL;

    P_GetProcessWindowStation pGetProcessWindowStation =
        (P_GetProcessWindowStation) Ldr_GetProcAddrNew(DllName_user32, L"GetProcessWindowStation","GetProcessWindowStation");

    P_GetUserObjectInformation pGetUserObjectInformation = (P_GetUserObjectInformation)
            Ldr_GetProcAddrNew(DllName_user32, L"GetUserObjectInformationW","GetUserObjectInformationW");

    if (pGetProcessWindowStation)
        winsta = pGetProcessWindowStation();

    if (winsta && pGetUserObjectInformation) {

        ULONG len = 256;
        name = Dll_AllocTemp(len);
        if (! pGetUserObjectInformation(
                    winsta, UOI_NAME, name, len, &len)) {

            Dll_Free(name);
            name = NULL;
        }
    }

    return name;
}


//---------------------------------------------------------------------------
// Terminal_WinStationOpenServerW
//---------------------------------------------------------------------------


_FX HANDLE Terminal_WinStationOpenServerW(const WCHAR *ServerName)
{
    if (ServerName != SERVERNAME_CURRENT) {
        SetLastError(ERROR_ACCESS_DENIED);
        return NULL;
    }

    SetLastError(0);
    return HANDLE_SERVER_FAKE;
}


//---------------------------------------------------------------------------
// Terminal_WinStationCloseServer
//---------------------------------------------------------------------------


_FX BOOLEAN Terminal_WinStationCloseServer(HANDLE hServer)
{
    SetLastError(0);
    return TRUE;
}


//---------------------------------------------------------------------------
// Terminal_WinStationDisconnect
//---------------------------------------------------------------------------


_FX BOOLEAN Terminal_WinStationDisconnect(
    HANDLE hServer, ULONG LogonId, ULONG_PTR Unknown)
{
    MSG_HEADER req;
    MSG_HEADER *rpl;
    ULONG err;

    if ((hServer != SERVERNAME_CURRENT && hServer != HANDLE_SERVER_FAKE)
                                        || (LogonId != LOGONID_CURRENT)) {
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

    req.length = sizeof(MSG_HEADER);
    req.msgid = MSGID_TERMINAL_DISCONNECT;
    rpl = SbieDll_CallServer(&req);
    if (! rpl)
        err = ERROR_GEN_FAILURE;
    else {
        err = rpl->status;
        Dll_Free(rpl);
    }

    SetLastError(err);
    return (! err);
}


//---------------------------------------------------------------------------
// Terminal_WinStationEnumerateW
//---------------------------------------------------------------------------


_FX BOOLEAN Terminal_WinStationEnumerateW(
    HANDLE hServer, void **InfoBuffer, ULONG *InfoCount)
{
    WCHAR *WinStaName;
    WINSTATIONENUMW *info;

    if (hServer != SERVERNAME_CURRENT && hServer != HANDLE_SERVER_FAKE) {
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

    WinStaName = Terminal_GetWindowStationName();
    if (! WinStaName) {
        *InfoCount = 0;
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

    info = Terminal_Alloc(sizeof(WINSTATIONENUMW));

    info->SessionId = Dll_SessionId;
    info->State = WTSActive;
    wcsncpy(info->WinStationName, WinStaName, 34);

    *InfoBuffer = info;
    *InfoCount = 1;

    Dll_Free(WinStaName);

    SetLastError(0);
    return TRUE;
}


//---------------------------------------------------------------------------
// Terminal_WinStaQueryInformationW
//---------------------------------------------------------------------------


_FX BOOLEAN Terminal_WinStaQueryInformationW(
    HANDLE hServer, ULONG LogonId, WINSTATIONINFOCLASS InformationClass,
    void *Information, ULONG InformationLength, PULONG pReturnLength)
{
    TERMINAL_QUERY_INFO_REQ req;
    TERMINAL_QUERY_INFO_RPL *rpl;
    ULONG err;

    if (hServer != SERVERNAME_CURRENT && hServer != HANDLE_SERVER_FAKE) {
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

    if (LogonId == LOGONID_CURRENT)
        req.session_id = Dll_SessionId;
    else
        req.session_id = LogonId;

    req.h.length = sizeof(TERMINAL_QUERY_INFO_REQ);
    req.h.msgid = MSGID_TERMINAL_QUERY_INFO;
    req.info_class = InformationClass;
    req.data_len = InformationLength;

    rpl = (TERMINAL_QUERY_INFO_RPL *)SbieDll_CallServer(&req.h);
    if (! rpl)
        err = ERROR_GEN_FAILURE;
    else {

        err = rpl->h.status;
        if (! err) {

            memcpy(Information, &rpl->data[0], rpl->data_len);
            *pReturnLength = rpl->data_len;
        }

        Dll_Free(rpl);
    }

    SetLastError(err);
    return (! err);
}


//---------------------------------------------------------------------------
// Terminal_WinStationIsSessionRemoteable
//---------------------------------------------------------------------------


_FX BOOLEAN Terminal_WinStationIsSessionRemoteable(
    HANDLE hServer, ULONG LogonId, BOOLEAN *IsRemote)
{
    TERMINAL_CHECK_TYPE_REQ req;
    TERMINAL_CHECK_TYPE_RPL *rpl;
    ULONG err;

    if (hServer != SERVERNAME_CURRENT && hServer != HANDLE_SERVER_FAKE) {
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

    if (LogonId == LOGONID_CURRENT)
        req.session_id = Dll_SessionId;
    else
        req.session_id = LogonId;

    req.h.length = sizeof(TERMINAL_CHECK_TYPE_REQ);
    req.h.msgid = MSGID_TERMINAL_CHECK_TYPE;
    req.check_is_remote = TRUE;

    rpl = (TERMINAL_CHECK_TYPE_RPL *)SbieDll_CallServer(&req.h);
    if (! rpl)
        err = ERROR_GEN_FAILURE;
    else {

        err = rpl->h.status;
        if (! err) {
            if (rpl->data_len != sizeof(UCHAR))
                err = ERROR_GEN_FAILURE;
            else
                *IsRemote = rpl->data[0];
        }

        Dll_Free(rpl);
    }

    SetLastError(err);
    return (! err);
}


//---------------------------------------------------------------------------
// Terminal_WinStationNameFromLogonIdW
//---------------------------------------------------------------------------


_FX BOOLEAN Terminal_WinStationNameFromLogonIdW(
    HANDLE hServer, ULONG LogonId, WCHAR *Name)
{
    TERMINAL_GET_NAME_REQ req;
    TERMINAL_GET_NAME_RPL *rpl;
    ULONG err;

    if (hServer != SERVERNAME_CURRENT && hServer != HANDLE_SERVER_FAKE) {
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

    if (LogonId == LOGONID_CURRENT)
        req.session_id = Dll_SessionId;
    else
        req.session_id = LogonId;

    req.h.length = sizeof(TERMINAL_GET_NAME_REQ);
    req.h.msgid = MSGID_TERMINAL_GET_NAME;

    rpl = (TERMINAL_GET_NAME_RPL *)SbieDll_CallServer(&req.h);
    if (! rpl)
        err = ERROR_GEN_FAILURE;
    else {

        err = rpl->h.status;
        if (! err) {

            wcscpy(Name, rpl->name);
        }

        Dll_Free(rpl);
    }

    SetLastError(err);
    return (! err);
}


//---------------------------------------------------------------------------
// Terminal_WinStationGetConnectionProperty
//---------------------------------------------------------------------------


_FX BOOLEAN Terminal_WinStationGetConnectionProperty(
    ULONG LogonId, GUID *TypeGuid, WTS_PROPERTY_VALUE **Value)
{
    TERMINAL_GET_PROPERTY_REQ req;
    TERMINAL_GET_PROPERTY_RPL *rpl;
    ULONG err;

    if (LogonId == LOGONID_CURRENT)
        req.session_id = Dll_SessionId;
    else
        req.session_id = LogonId;

    req.h.length = sizeof(TERMINAL_GET_PROPERTY_REQ);
    req.h.msgid = MSGID_TERMINAL_GET_PROPERTY;
    memcpy(&req.guid, TypeGuid, sizeof(GUID));

    rpl = (TERMINAL_GET_PROPERTY_RPL *)SbieDll_CallServer(&req.h);
    if (! rpl)
        err = ERROR_GEN_FAILURE;
    else {

        err = rpl->h.status;
        if (! err) {

            UCHAR *ptr;
            WTS_PROPERTY_VALUE *value =
                Terminal_Alloc(sizeof(WTS_PROPERTY_VALUE) + rpl->len);
            memzero(value, sizeof(WTS_PROPERTY_VALUE));

            value->Type = (USHORT)rpl->type;

            if (value->Type == WTS_VALUE_TYPE_ULONG)
                ptr = (UCHAR *)&value->u.ulVal;
            else if (value->Type == WTS_VALUE_TYPE_GUID)
                ptr = (UCHAR *)&value->u.guidVal;
            else {
                ptr = (UCHAR *)(value + 1);
                if (value->Type == WTS_VALUE_TYPE_STRING) {
                    value->u.strVal.size = rpl->len / sizeof(WCHAR);
                    value->u.strVal.pstrVal = (WCHAR *)ptr;
                } else if (value->Type == WTS_VALUE_TYPE_BINARY) {
                    value->u.bVal.size = rpl->len;
                    value->u.bVal.pbVal = (char *)ptr;
                } else
                    ptr = NULL;
            }

            if (ptr)
                memcpy(ptr, rpl->data, rpl->len);
            else {
                Terminal_WinStationFreePropertyValue(value);
                value = NULL;
                err = ERROR_UNKNOWN_PROPERTY;
            }

            *Value = value;
        }

        Dll_Free(rpl);
    }

    SetLastError(err);
    return (! err);
}


//---------------------------------------------------------------------------
// Terminal_WinStationFreePropertyValue
//---------------------------------------------------------------------------


_FX BOOLEAN Terminal_WinStationFreePropertyValue(
    WTS_PROPERTY_VALUE *Value)
{
    return Terminal_Free(Value, NULL);
}


//---------------------------------------------------------------------------
// Terminal_WinStationFreeMemory
//---------------------------------------------------------------------------


_FX void Terminal_WinStationFreeMemory(void *pMemory)
{
    Terminal_Free(pMemory, __sys_WinStationFreeMemory);
}


//---------------------------------------------------------------------------
// Terminal_Init_WinSta
//---------------------------------------------------------------------------


_FX BOOLEAN Terminal_Init_WinSta(HMODULE module)
{
    P_WinStationOpenServer WinStationOpenServerW;
    P_WinStationCloseServer WinStationCloseServer;
    P_WinStationDisconnect WinStationDisconnect;
    P_WinStationEnumerate WinStationEnumerateW;
    P_WinStationQueryInformation WinStaQueryInformationW;
    P_WinStationIsSessionRemoteable WinStationIsSessionRemoteable;
    P_WinStationNameFromLogonId WinStationNameFromLogonIdW;
    P_WinStationGetConnectionProperty WinStationGetConnectionProperty;
    P_WinStationFreePropertyValue WinStationFreePropertyValue;
    P_WinStationFreeMemory WinStationFreeMemory;

    ULONG_PTR __sys_WinStationOpenServerW = 0;
    ULONG_PTR __sys_WinStationCloseServer = 0;
    ULONG_PTR __sys_WinStationDisconnect = 0;
    ULONG_PTR __sys_WinStationEnumerateW = 0;
    ULONG_PTR __sys_WinStaQueryInformationW = 0;
    ULONG_PTR __sys_WinStationIsSessionRemoteable = 0;
    ULONG_PTR __sys_WinStationNameFromLogonIdW = 0;
    ULONG_PTR __sys_WinStationGetConnectionProperty = 0;
    ULONG_PTR __sys_WinStationFreePropertyValue = 0;


    if (Terminal_DontHook())
        return TRUE;

    //
    // hook terminal services
    //

    WinStationOpenServerW = (P_WinStationOpenServer)
        GetProcAddress(module, "WinStationOpenServerW");

    WinStationCloseServer = (P_WinStationCloseServer)
        GetProcAddress(module, "WinStationCloseServer");

    WinStationDisconnect = (P_WinStationDisconnect)
        GetProcAddress(module, "WinStationDisconnect");

    WinStationEnumerateW = (P_WinStationEnumerate)
        GetProcAddress(module, "WinStationEnumerateW");

    WinStaQueryInformationW = (P_WinStationQueryInformation)
        GetProcAddress(module, "WinStationQueryInformationW");

    WinStationIsSessionRemoteable = (P_WinStationIsSessionRemoteable)
        GetProcAddress(module, "WinStationIsSessionRemoteable");

    WinStationNameFromLogonIdW = (P_WinStationNameFromLogonId)
        GetProcAddress(module, "WinStationNameFromLogonIdW");

    WinStationGetConnectionProperty = (P_WinStationGetConnectionProperty)
        GetProcAddress(module, "WinStationGetConnectionProperty");

    WinStationFreePropertyValue = (P_WinStationFreePropertyValue)
        GetProcAddress(module, "WinStationFreePropertyValue");

    WinStationFreeMemory = (P_WinStationFreeMemory)
        GetProcAddress(module, "WinStationFreeMemory");

    SBIEDLL_HOOK(Terminal_,WinStationOpenServerW);
    SBIEDLL_HOOK(Terminal_,WinStationCloseServer);
    SBIEDLL_HOOK(Terminal_,WinStationDisconnect);
    SBIEDLL_HOOK(Terminal_,WinStationEnumerateW);
    SBIEDLL_HOOK(Terminal_,WinStaQueryInformationW);
    SBIEDLL_HOOK(Terminal_,WinStationFreeMemory);
    if (WinStationIsSessionRemoteable) {
        SBIEDLL_HOOK(Terminal_,WinStationIsSessionRemoteable);
    }
    if (WinStationNameFromLogonIdW) {
        SBIEDLL_HOOK(Terminal_,WinStationNameFromLogonIdW);
    }
    if (WinStationGetConnectionProperty) {
        SBIEDLL_HOOK(Terminal_,WinStationGetConnectionProperty);
        if (WinStationFreePropertyValue) {
            SBIEDLL_HOOK(Terminal_,WinStationFreePropertyValue);
        }
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Terminal_WTSQueryUserToken
//---------------------------------------------------------------------------


_FX BOOL Terminal_WTSQueryUserToken(ULONG SessionId, HANDLE *pToken)
{
    if (Dll_SbieTrace) {
        SbieApi_MonitorPut2(MONITOR_OTHER | MONITOR_TRACE, L"WTSQueryUserToken", FALSE);
    }

    // WTSQueryUserToken needs SE_TCB_NAME privilege and some IPC paths
    //OpenIpcPath=\RPC Control\LSMApi
    //OpenIpcPath=\BaseNamedObjects\TermSrvReadyEvent
    //if (__sys_WTSQueryUserToken(SessionId, pToken))
    //    return TRUE;

#if 1

    GET_USER_TOKEN_REQ *req;
    GET_USER_TOKEN_RPL *rpl;
    ULONG req_len;
    ULONG error;

    req_len = sizeof(GET_USER_TOKEN_REQ);

    req = (GET_USER_TOKEN_REQ *)Dll_AllocTemp(req_len);
    req->h.length = req_len;
    req->h.msgid = MSGID_TERMINAL_GET_USER_TOKEN;

    rpl = (GET_USER_TOKEN_RPL *)SbieDll_CallServer((MSG_HEADER *)req);
    Dll_Free(req);

    if (! rpl)
        error = RPC_S_SERVER_UNAVAILABLE;
    else
        error = rpl->h.status;

    if (error == 0) {

        *pToken = rpl->hToken;
    }

    if (rpl)
        Dll_Free(rpl);
    SetLastError(error);
    return (error == 0 ? TRUE : FALSE);

#else
    ULONG *pids;
    ULONG i, err;

    err = ERROR_NO_TOKEN;
    *pToken = NULL;

    ULONG pid_count = 0;
    SbieApi_EnumProcessEx(NULL, FALSE, -1, NULL, &pid_count); // query count
    pid_count += 128;

    pids = Dll_AllocTemp(sizeof(ULONG) * pid_count);
    SbieApi_EnumProcessEx(NULL, FALSE, -1, pids, &pid_count); // query pids

    for (i = 0; i < pid_count; ++i) {

        WCHAR image[128];
        HANDLE pids_i = (HANDLE) (ULONG_PTR) pids[i];
        SbieApi_QueryProcess(pids_i, NULL, image, NULL, NULL);
        if (_wcsicmp(image, Ipc_SandboxieRpcSs) == 0) {

            HANDLE process = OpenProcess(
                                PROCESS_QUERY_INFORMATION, FALSE, pids[i]);

            if (process) {

                NTSTATUS status =
                    NtOpenProcessToken(process, TOKEN_ALL_ACCESS, pToken);
                if (status == 0)
                    err = 0;

                CloseHandle(process);

                break;
            }
        }
    }

    Dll_Free(pids);
    SetLastError(err);
    return (err ? FALSE : TRUE);

#endif
}


//---------------------------------------------------------------------------
// Terminal_WTSEnumerateSessionsW
//---------------------------------------------------------------------------


_FX BOOL Terminal_WTSEnumerateSessionsW(
    HANDLE hServer, DWORD Reserved, DWORD Version,
    WTS_SESSION_INFO **ppSessionInfo, DWORD *pCount)
{
    WCHAR *WinStaName;
    WTS_SESSION_INFO *info;
    ULONG len;

    if (hServer != SERVERNAME_CURRENT &&
        hServer != HANDLE_SERVER_FAKE)
    {
        *pCount = 0;
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

    if (Reserved || Version != 1) {
        *pCount = 0;
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    WinStaName = Terminal_GetWindowStationName();
    if (! WinStaName) {
        *pCount = 0;
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

    len = sizeof(WTS_SESSION_INFO)
        + (wcslen(WinStaName) + 8) * sizeof(WCHAR);

    info = (WTS_SESSION_INFO *)Terminal_Alloc(len);

    info->SessionId = Dll_SessionId;
    info->State = WTSActive;
    info->pWinStationName = (WCHAR *)(info + 1);
    wcscpy(info->pWinStationName, WinStaName);

    *ppSessionInfo = info;
    *pCount = 1;

    Dll_Free(WinStaName);

    SetLastError(0);
    return TRUE;
}


//---------------------------------------------------------------------------
// Terminal_WTSEnumerateProcessesW
//---------------------------------------------------------------------------


_FX BOOL Terminal_WTSEnumerateProcessesW(
    HANDLE hServer, DWORD Reserved, DWORD Version,
    WTS_PROCESS_INFO **ppProcessInfo, DWORD *pCount)
{
    SYSTEM_PROCESS_INFORMATION *info, *info_ptr;
    UCHAR *work_area, *work_ptr;
    UCHAR *out_buf, *out_ptr;
    ULONG i, num;

    if (hServer != SERVERNAME_CURRENT &&
        hServer != HANDLE_SERVER_FAKE)
    {
        *pCount = 0;
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

    if (Reserved || Version != 1) {
        *pCount = 0;
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    //
    // get list of processes active in the system and count them
    //

    info = SysInfo_QueryProcesses(NULL);
    if (! info) {
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

    num = 0;
    info_ptr = info;
    while (1) {
        ++num;
        i = info_ptr->NextEntryOffset;
        if (! i)
            break;
        info_ptr = (SYSTEM_PROCESS_INFORMATION *)((UCHAR *)info_ptr + i);
    }

    //
    // now we need to allocate a temporary work area with room for
    // 768 bytes for each of the processes we enumerated
    //
    // for each process, we extract its process name and SID and
    // store into the work area.  we store the offsets into the
    // work area in the SYSTEM_PROCESS_INFORMATION array
    //

    work_area = Dll_AllocTemp(num * (512 + 256));
    work_ptr = work_area;

    info_ptr = info;
    while (1) {

        void *name       = NULL;
        void *sid        = NULL;
        ULONG name_len   = 0;
        ULONG sid_len    = 0;
        ULONG session_id = 0;

        HANDLE hProcess = NULL;
        if ((ULONG)(ULONG_PTR)info_ptr->UniqueProcessId >= 8) {
            hProcess = OpenProcess(
                            PROCESS_QUERY_INFORMATION, FALSE,
                            (ULONG)(ULONG_PTR)info_ptr->UniqueProcessId);
        }

        if (hProcess) {

            HANDLE hToken;

            NTSTATUS status = NtQueryInformationProcess(
                hProcess, ProcessImageFileName, work_ptr, 512, &i);

            if (NT_SUCCESS(status)) {

                name = ((UNICODE_STRING *)work_ptr)->Buffer;
                i = (i + 7) & ~7;
                name_len = i;
                work_ptr += i;
            }

            status = NtOpenProcessToken(hProcess, TOKEN_QUERY, &hToken);

            if (NT_SUCCESS(status)) {

                status = NtQueryInformationToken(
                    hToken, TokenUser, work_ptr, 256, &i);

                if (NT_SUCCESS(status)) {

                    sid = ((TOKEN_USER *)work_ptr)->User.Sid;
                    i = (i + 7) & ~7;
                    sid_len = i;
                    work_ptr += i;
                }

                status = NtQueryInformationToken(
                    hToken, TokenSessionId, &session_id, sizeof(ULONG), &i);

                CloseHandle(hToken);
            }

            CloseHandle(hProcess);
        }

        if (! name) {

            ((WCHAR *)work_ptr)[0] = L'?';
            ((WCHAR *)work_ptr)[1] = L'\0';
            name = work_ptr;
            name_len = 2 * sizeof(WCHAR);
            work_ptr += 8;
        }

        info_ptr->Reserved5[0] = name;
        info_ptr->Reserved5[1] = (void *)(ULONG_PTR)name_len;
        info_ptr->Reserved5[2] = sid;
        info_ptr->Reserved5[3] = (void *)(ULONG_PTR)sid_len;
        info_ptr->Reserved5[4] = (void *)(ULONG_PTR)session_id;

        i = info_ptr->NextEntryOffset;
        if (! i)
            break;
        info_ptr = (SYSTEM_PROCESS_INFORMATION *)((UCHAR *)info_ptr + i);
    }

    //
    // finally we allocate the result buffer, which contains a number of
    // WTS_PROCESS_INFO structures and the strings we collected
    //

    i = num * sizeof(WTS_PROCESS_INFO)
      + (ULONG)(work_ptr - work_area);

    out_buf = Terminal_Alloc(i);
    /*if (! out_buf) {

        Dll_Free(work_area);
        Dll_Free(info);

        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }*/

    memset(out_buf, '*', i);

    out_ptr = out_buf + num * sizeof(WTS_PROCESS_INFO);

    //
    // fill the result buffer
    //

    num = 0;

    info_ptr = info;
    while (1) {

        WCHAR *name      = info_ptr->Reserved5[0];
        ULONG name_len   = (ULONG)(ULONG_PTR)info_ptr->Reserved5[1];
        void  *sid       = info_ptr->Reserved5[2];
        ULONG sid_len    = (ULONG)(ULONG_PTR)info_ptr->Reserved5[3];
        ULONG session_id = (ULONG)(ULONG_PTR)info_ptr->Reserved5[4];
        ULONG process_id = (ULONG)(ULONG_PTR)info_ptr->UniqueProcessId;

        if (name && name_len && sid && sid_len) {

            WTS_PROCESS_INFO *pi = ((WTS_PROCESS_INFO *)out_buf) + num;

            pi->SessionId = session_id;
            pi->ProcessId = process_id;

            pi->pProcessName = (WCHAR *)out_ptr;
            memcpy(out_ptr, name, name_len);
            out_ptr += name_len;

            name = wcsrchr(pi->pProcessName, L'\\');
            if (name)
                pi->pProcessName = name + 1;

            pi->pUserSid = out_ptr;
            memcpy(out_ptr, sid, sid_len);
            out_ptr += sid_len;

            ++num;
        }

        i = info_ptr->NextEntryOffset;
        if (! i)
            break;
        info_ptr = (SYSTEM_PROCESS_INFORMATION *)((UCHAR *)info_ptr + i);
    }

    //
    // finish
    //

    *ppProcessInfo = (WTS_PROCESS_INFO *)out_buf;
    *pCount        = num;

    Dll_Free(work_area);
    Dll_Free(info);

    SetLastError(ERROR_SUCCESS);
    return TRUE;
}


//---------------------------------------------------------------------------
// Terminal_WTSFreeMemory
//---------------------------------------------------------------------------


_FX void Terminal_WTSFreeMemory(void *pMemory)
{
    Terminal_Free(pMemory, (P_WinStationFreeMemory)__sys_WTSFreeMemory);
}


//---------------------------------------------------------------------------
// Terminal_WTSRegisterSessionNotification
//---------------------------------------------------------------------------


_FX BOOL Terminal_WTSRegisterSessionNotification(
    HWND hWnd, ULONG dwFlags)
{
    SetLastError(0);
    return TRUE;
}


//---------------------------------------------------------------------------
// Terminal_WTSRegisterSessionNotificationEx
//---------------------------------------------------------------------------


_FX BOOL Terminal_WTSRegisterSessionNotificationEx(
    HANDLE hServer, HWND hWnd, ULONG dwFlags)
{
    SetLastError(0);
    return TRUE;
}


//---------------------------------------------------------------------------
// Terminal_WTSUnRegisterSessionNotification
//---------------------------------------------------------------------------


_FX BOOL Terminal_WTSUnRegisterSessionNotification(
    HWND hWnd)
{
    SetLastError(0);
    return TRUE;
}


//---------------------------------------------------------------------------
// Terminal_WTSUnRegisterSessionNotificationEx
//---------------------------------------------------------------------------


_FX BOOL Terminal_WTSUnRegisterSessionNotificationEx(
    HANDLE hServer, HWND hWnd)
{
    SetLastError(0);
    return TRUE;
}


//---------------------------------------------------------------------------
// Terminal_Init_WtsApi
//---------------------------------------------------------------------------


_FX BOOLEAN Terminal_Init_WtsApi(HMODULE module)
{
    P_WTSQueryUserToken WTSQueryUserToken;
    P_WTSEnumerateSessions WTSEnumerateSessionsW;
    P_WTSEnumerateProcesses WTSEnumerateProcessesW;
    P_WTSFreeMemory WTSFreeMemory;
    P_WTSRegisterSessionNotification WTSRegisterSessionNotification;
    P_WTSRegisterSessionNotificationEx WTSRegisterSessionNotificationEx;
    P_WTSUnRegisterSessionNotification WTSUnRegisterSessionNotification;
    P_WTSUnRegisterSessionNotificationEx WTSUnRegisterSessionNotificationEx;
    ULONG_PTR __sys_WTSEnumerateSessionsW = 0;
    ULONG_PTR __sys_WTSEnumerateProcessesW = 0;
    ULONG_PTR __sys_WTSRegisterSessionNotification = 0;
    ULONG_PTR __sys_WTSRegisterSessionNotificationEx = 0;
    ULONG_PTR __sys_WTSUnRegisterSessionNotification = 0;
    ULONG_PTR __sys_WTSUnRegisterSessionNotificationEx = 0;

    
    WTSQueryUserToken = (P_WTSQueryUserToken)
        GetProcAddress(module, "WTSQueryUserToken");

    if (WTSQueryUserToken) {
        SBIEDLL_HOOK(Terminal_,WTSQueryUserToken);
    }

    if (Terminal_DontHook())
        return TRUE;

    //
    // hook terminal services
    //

    WTSEnumerateSessionsW = (P_WTSEnumerateSessions)
        GetProcAddress(module, "WTSEnumerateSessionsW");
    WTSEnumerateProcessesW = (P_WTSEnumerateProcesses)
        GetProcAddress(module, "WTSEnumerateProcessesW");
    WTSFreeMemory = (P_WTSFreeMemory)
        GetProcAddress(module, "WTSFreeMemory");
    WTSRegisterSessionNotification = (P_WTSRegisterSessionNotification)
        GetProcAddress(module, "WTSRegisterSessionNotification");
    WTSRegisterSessionNotificationEx = (P_WTSRegisterSessionNotificationEx)
        GetProcAddress(module, "WTSRegisterSessionNotificationEx");
    WTSUnRegisterSessionNotification = (P_WTSUnRegisterSessionNotification)
        GetProcAddress(module, "WTSUnRegisterSessionNotification");
    WTSUnRegisterSessionNotificationEx=(P_WTSUnRegisterSessionNotificationEx)
        GetProcAddress(module, "WTSUnRegisterSessionNotificationEx");


    if (WTSEnumerateSessionsW) {
        SBIEDLL_HOOK(Terminal_,WTSEnumerateSessionsW);
    }
    if (WTSEnumerateProcessesW) {
        SBIEDLL_HOOK(Terminal_,WTSEnumerateProcessesW);
    }
    if (WTSFreeMemory) {
        SBIEDLL_HOOK(Terminal_,WTSFreeMemory);
    }
    if (WTSRegisterSessionNotification) {
        SBIEDLL_HOOK(Terminal_,WTSRegisterSessionNotification);
    }
    if (WTSRegisterSessionNotificationEx) {
        SBIEDLL_HOOK(Terminal_,WTSRegisterSessionNotificationEx);
    }
    if (WTSUnRegisterSessionNotification) {
        SBIEDLL_HOOK(Terminal_,WTSUnRegisterSessionNotification);
    }
    if (WTSUnRegisterSessionNotificationEx) {
        SBIEDLL_HOOK(Terminal_,WTSUnRegisterSessionNotificationEx);
    }

    return TRUE;
}
