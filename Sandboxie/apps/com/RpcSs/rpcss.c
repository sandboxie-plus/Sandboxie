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
// Sandboxie RPCSS Service
//---------------------------------------------------------------------------

#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

#include <windows.h>
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include "common/defines.h"
#include "common/my_version.h"
#include "core/svc/msgids.h"


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


#define SINGLE_INSTANCE_MUTEX_NAME L"ServiceInitComplete_Mutex1"

const WCHAR *ServiceTitle = SANDBOXIE L" RpcSs";
#include "../common.h"
#include "../privs.h"

HANDLE ComPlusCOMRegTable = NULL;

const WCHAR *ComPlusCOMRegTable_Name = L"Global\\ComPlusCOMRegTable";

const char *_localhost = "localhost";


static ULONG_PTR __sys_bind                                     = 0;
static ULONG_PTR __sys_listen                                   = 0;
static ULONG_PTR __sys_gethostname                              = 0;
static ULONG_PTR __sys_gethostbyname                            = 0;
static ULONG_PTR __sys_WSASocketW                               = 0;
static ULONG_PTR __sys_OpenThreadToken                          = 0;
static ULONG_PTR __sys_RegOpenKeyExW                            = 0;
static ULONG_PTR __sys_RegQueryValueExW                         = 0;


//---------------------------------------------------------------------------
// my_CreateFileMapping
//---------------------------------------------------------------------------


_FX HANDLE my_CreateFileMappingW(
    HANDLE hFile,
    LPSECURITY_ATTRIBUTES lpAttributes,
    DWORD flProtect,
    DWORD dwMaximumSizeHigh,
    DWORD dwMaximumSizeLow,
    LPCWSTR lpName)
{
    typedef HANDLE (__stdcall *P_CreateFileMappingW)(
        HANDLE hFile,
        LPSECURITY_ATTRIBUTES lpAttributes,
        DWORD flProtect,
        DWORD dwMaximumSizeHigh,
        DWORD dwMaximumSizeLow,
        LPCWSTR lpName);

    HANDLE handle = 0;

    if (lpName && _wcsicmp(lpName, ComPlusCOMRegTable_Name) == 0)
        handle = ComPlusCOMRegTable;

    if (! handle) {
        handle = ((P_CreateFileMappingW)__sys_CreateFileMappingW)(
            hFile, lpAttributes, flProtect,
            dwMaximumSizeHigh, dwMaximumSizeLow, lpName);
    }

    return handle;
}


//---------------------------------------------------------------------------
// my_bind
//---------------------------------------------------------------------------


_FX int my_bind(SOCKET s, const struct sockaddr *name, int namelen)
{
    return 0;
}


//---------------------------------------------------------------------------
// my_listen
//---------------------------------------------------------------------------


_FX int my_listen(SOCKET s, int backlog)
{
    return 0;
}


//---------------------------------------------------------------------------
// my_gethostname
//---------------------------------------------------------------------------


_FX int my_gethostname(char *name, int namelen)
{
    if (namelen >= 10) {
        strcpy(name, _localhost);
        return 0;
    } else {
        WSASetLastError(WSAEFAULT);
        return SOCKET_ERROR;
    }
}


//---------------------------------------------------------------------------
// my_gethostbyname
//---------------------------------------------------------------------------


struct hostent *my_gethostbyname(const char *name)
{
    typedef struct hostent *(*P_gethostbyname)(const char *name);
    static struct hostent my_hostent;
    static char *my_aliases = NULL;
    static ULONG_PTR my_addr_list[2] = { 0, 0 };
    static ULONG my_ip = 0x0100007f;
    if (strcmp(name, _localhost) != 0)
        return ((P_gethostbyname)__sys_gethostbyname)(name);
    my_hostent.h_name = (char *)_localhost;
    my_hostent.h_aliases = &my_aliases;
    my_hostent.h_addrtype = AF_INET;
    my_hostent.h_length = sizeof(ULONG);
    my_hostent.h_addr_list = (char **)my_addr_list;
    my_addr_list[0] = (ULONG_PTR)&my_ip;
    return &my_hostent;
}


//---------------------------------------------------------------------------
// my_WSASocketW
//---------------------------------------------------------------------------


_FX SOCKET my_WSASocketW(
    int af, int type, int protocol, LPWSAPROTOCOL_INFO lpProtocolInfo,
    GROUP g, DWORD dwFlags)
{
    WSASetLastError(WSAEACCES);
    return INVALID_SOCKET;
}


//---------------------------------------------------------------------------
// my_OpenThreadToken
//---------------------------------------------------------------------------


_FX BOOL my_OpenThreadToken(
    HANDLE ThreadHandle, DWORD DesiredAccess, BOOL OpenAsSelf,
    PHANDLE TokenHandle)
{
    typedef BOOL (*P_OpenThreadToken)(
        HANDLE ThreadHandle, DWORD DesiredAccess, BOOL OpenAsSelf,
        PHANDLE TokenHandle);

    BOOL ok = ((P_OpenThreadToken)__sys_OpenThreadToken)(
        ThreadHandle, DesiredAccess, OpenAsSelf, TokenHandle);

    if ((! ok) && GetLastError() == ERROR_BAD_IMPERSONATION_LEVEL
            && (! OpenAsSelf)) {

        OpenAsSelf = TRUE;

        ok = ((P_OpenThreadToken)__sys_OpenThreadToken)(
            ThreadHandle, DesiredAccess, OpenAsSelf, TokenHandle);
    }

    return ok;
}


//---------------------------------------------------------------------------
// my_RegOpenKeyExW
//---------------------------------------------------------------------------


_FX LONG my_RegOpenKeyExW(
    HKEY hKey, LPCWSTR lpSubKey, DWORD ulOptions, REGSAM samDesired,
    PHKEY phkResult)
{
    typedef BOOL (*P_RegOpenKeyExW)(
        HKEY hKey, LPCWSTR lpSubKey, DWORD ulOptions, REGSAM samDesired,
        PHKEY phkResult);
    typedef BOOL (*P_SetThreadToken)(PHANDLE Thread, HANDLE Token);

    LONG rc = ((P_RegOpenKeyExW)__sys_RegOpenKeyExW)(
        hKey, lpSubKey, ulOptions, samDesired, phkResult);

    if (rc == ERROR_BAD_IMPERSONATION_LEVEL) {

        ULONG LastError = GetLastError();

        HANDLE Token;
        BOOL ok = OpenThreadToken(
            NtCurrentThread(), TOKEN_IMPERSONATE, TRUE, &Token);
        if (ok && Token) {

            ((P_SetThreadToken)__sys_SetThreadToken)(NULL, NULL);
            rc = ((P_RegOpenKeyExW)__sys_RegOpenKeyExW)(
                hKey, lpSubKey, ulOptions, samDesired, phkResult);
            ((P_SetThreadToken)__sys_SetThreadToken)(NULL, Token);
        }

        SetLastError(LastError);
    }

    return rc;
}


//---------------------------------------------------------------------------
// my_RegQueryValueKeyExW
//---------------------------------------------------------------------------


_FX LONG my_RegQueryValueExW(
    HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved,
    LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
    typedef BOOL (*P_RegQueryValueExW)(
        HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved,
        LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
    typedef BOOL (*P_SetThreadToken)(PHANDLE Thread, HANDLE Token);

    LONG rc = ((P_RegQueryValueExW)__sys_RegQueryValueExW)(
        hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);

    if (rc == ERROR_BAD_IMPERSONATION_LEVEL) {

        ULONG LastError = GetLastError();

        HANDLE Token;
        BOOL ok = OpenThreadToken(
            NtCurrentThread(), TOKEN_IMPERSONATE, TRUE, &Token);
        if (ok && Token) {

            ((P_SetThreadToken)__sys_SetThreadToken)(NULL, NULL);
            rc = ((P_RegQueryValueExW)__sys_RegQueryValueExW)(
                hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
            ((P_SetThreadToken)__sys_SetThreadToken)(NULL, Token);
        }

        SetLastError(LastError);
    }

    return rc;
}


//---------------------------------------------------------------------------
// Start_WinSock
//---------------------------------------------------------------------------


_FX BOOL Start_WinSock(void)
{
    if (SbieDll_IsDllSkipHook(L"ws2_32.dll"))
        return TRUE;

    WORD wVersionRequested;
    WSADATA wsaData;
    BOOL hook_success = TRUE;

    wVersionRequested = MAKEWORD(2, 0);
    if (WSAStartup(wVersionRequested, &wsaData) != 0) {
        ErrorMessageBox(L"Could not initialize WinSock");
        return FALSE;
    }

    HOOK_WIN32(bind);
    HOOK_WIN32(listen);
    HOOK_WIN32(gethostname);
    HOOK_WIN32(gethostbyname);
    HOOK_WIN32(WSASocketW);
    HOOK_WIN32(OpenThreadToken);
    HOOK_WIN32(RegOpenKeyExW);
    HOOK_WIN32(RegQueryValueExW);

    if (! hook_success) {
        ErrorMessageBox(L"Could not intercept WinSock services");
        return FALSE;
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// StartRpcEptMapper
//---------------------------------------------------------------------------


BOOL StartRpcEptMapper(void)
{
    static WCHAR ServiceName2[16];
    WCHAR EventName[128];
    HANDLE hEvent;
    BOOL ok;

    //
    // windows 7 requires an additional RpcEptMapper service
    //

    if (! KernelBase)
        return TRUE;

    if (IsWindows81)
        CreateEvent(NULL, FALSE, TRUE, L"Global\\SC_AutoStartComplete");

    wcscpy(ServiceName2, L"RpcEptMapper");

    //
    // prepare event for service
    //

    swprintf(EventName, SBIE_BOXED_ L"ServiceInitComplete_%s", ServiceName2);
    hEvent = CreateEvent(NULL, TRUE, FALSE, EventName);
    if (! hEvent)
        return FALSE;

    //
    // start RpcEptMapper service on Windows 7
    //

    ok = Service_Start_ServiceMain(
                ServiceName2, L"rpcepmap.dll", "ServiceMain", TRUE);
    if (! ok)
        return FALSE;

    //
    // wait for init complete
    //

    WaitForSingleObject(hEvent, INFINITE);

    //
    // reset service-tracking variables
    //

    return TRUE;
}



//---------------------------------------------------------------------------
// WinMain
//---------------------------------------------------------------------------


_FX int __stdcall WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    extern int DoLingerLeader(void);
    extern ULONG Sxs_Thread(void *arg);
    extern ULONG Dde_Thread(void *arg);

    WCHAR ServiceName[16];
    BOOL ok;

    if (! (SbieApi_QueryProcessInfo(0, 0) & SBIE_FLAG_VALID_PROCESS))
        return EXIT_FAILURE;

    if (! CheckSingleInstance()) {
        // return special error code to indicate elective termination
        // because this is a duplicate copy of SandboxieRpcSs.exe
        // see also core/dll/ipc_start.c
        return STATUS_LICENSE_QUOTA_EXCEEDED;
    }

    if (1) {
        ULONG idThread;
        HANDLE hThread = CreateThread(
                    NULL, 0, Sxs_Thread, 0, CREATE_SUSPENDED, &idThread);
        if (hThread) {
            SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);
            ResumeThread(hThread);
            CloseHandle(hThread);
        }
    }

    if (!SbieDll_IsOpenCOM()) {

        Check_Windows_7();

        // pretend we are the SCM
        if (!Hook_Service_Control_Manager())
            return EXIT_FAILURE;

        // hook privilege-related functions
        if (!Hook_Privilege())
            return EXIT_FAILURE;

        // start WinSock
        if (!Start_WinSock())
            return EXIT_FAILURE;

        // create COM shared memory sections
        ComPlusCOMRegTable = CreateFileMapping(
            INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT,
            0, (PAGE_SIZE * 2), ComPlusCOMRegTable_Name);

        if (!StartRpcEptMapper())
            return EXIT_FAILURE;

        // start rpcss service
        wcscpy(ServiceName, L"RPCSS");
        ok = Service_Start_ServiceMain(
            ServiceName, L"rpcss.dll", "ServiceMain", TRUE);
        if (!ok)
            return EXIT_FAILURE;

    }
    else {

        PROCESS_DATA *myData;
        myData = my_findProcessData(L"RPCSS", 1);
        if (!myData) {
            return EXIT_FAILURE;
        }
        myData->state = SERVICE_RUNNING;
        InitComplete(myData);
    }

    if (1) {
        ULONG idThread;
        HANDLE hThread = CreateThread(
            NULL, 0, (LPTHREAD_START_ROUTINE)Dde_Thread, 0, CREATE_SUSPENDED, &idThread);
        if (hThread) {
            SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);
            ResumeThread(hThread);
            CloseHandle(hThread);
        }
    }

    if (1) {
        MSG_HEADER req;
        req.length = sizeof(req);
        req.msgid = MSGID_SBIE_INI_RUN_SBIE_CTRL;
        SbieDll_CallServer(&req);
    }

    return DoLingerLeader();
}
