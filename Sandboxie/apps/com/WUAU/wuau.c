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
// Sandboxie Windows Automatic Update Service (WUAUSERV)
//---------------------------------------------------------------------------

#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "common/defines.h"
#include "common/my_version.h"


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


const WCHAR *ServiceTitle = SANDBOXIE L" WUAUSERV";
#include "../common.h"


//---------------------------------------------------------------------------
// my_CreateFileMapping
//---------------------------------------------------------------------------


HANDLE my_CreateFileMappingW(
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

    return ((P_CreateFileMappingW)__sys_CreateFileMappingW)(
        hFile, lpAttributes, flProtect,
        dwMaximumSizeHigh, dwMaximumSizeLow, lpName);
}


//---------------------------------------------------------------------------
// my_CreateProcessW
//---------------------------------------------------------------------------


ULONG_PTR __sys_CreateProcessW = 0;

BOOL my_CreateProcessW(
    LPVOID lpApplicationName,
    LPVOID lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPVOID lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation)
{
    typedef BOOL (*P_CreateProcess)(
        LPVOID lpApplicationName,
        LPVOID lpCommandLine,
        LPSECURITY_ATTRIBUTES lpProcessAttributes,
        LPSECURITY_ATTRIBUTES lpThreadAttributes,
        BOOL bInheritHandles,
        DWORD dwCreationFlags,
        LPVOID lpEnvironment,
        LPVOID lpCurrentDirectory,
        LPSTARTUPINFOW lpStartupInfo,
        LPPROCESS_INFORMATION lpProcessInformation);

    //
    // make sure SandboxieWUAU only launches one copy of WUAUCLT.EXE
    // at a time.  the sequence is SandboxieWUAU launching WUAUCLT.
    // then WUAUCLT calls CoRegisterClassObject for some unregistered
    // CLSID and signals SandboxieWUAU.  then SandboxieWUAU tries
    // CoCreateInstance on that CLSID.  It fails REGDB_E_CLASSNOTREG,
    // and then retries from the top, spawning another instance of
    // WUAUCLT.  this hack prevents more than one instance of WUAUCLT.
    //

    WCHAR *ptr = lpCommandLine;

    while (ptr) {
        ptr = wcschr(ptr, L'\\');
        if (ptr) {
            ++ptr;
            if (_wcsnicmp(ptr, L"system32\\wuauclt.exe", 20) == 0) {

                ULONG pid = FindProcessId(L"wuauclt.exe", FALSE);
                if (! pid)
                    break;

                SetLastError(ERROR_ACCESS_DENIED);
                return FALSE;
            }
        }
    }

    return ((P_CreateProcess)__sys_CreateProcessW)(
        lpApplicationName, lpCommandLine,
        lpProcessAttributes, lpThreadAttributes,
        bInheritHandles, dwCreationFlags,
        lpEnvironment, lpCurrentDirectory,
        lpStartupInfo, lpProcessInformation);
}


//---------------------------------------------------------------------------
// my_NtAlpcConnectPort
//---------------------------------------------------------------------------


ULONG_PTR __sys_NtAlpcConnectPort = 0;

_FX NTSTATUS my_NtAlpcConnectPort(
    HANDLE *PortHandle, UNICODE_STRING *PortName, void *ObjectAttributes,
    void *AlpcConnectInfo, ULONG ConnectionFlags, void *ServerSid,
    void *ConnectionInfo, void *ConnectionInfoLength,
    void *InMessageBuffer, void *OutMessageBuffer, void *Timeout)
{
    NTSTATUS status;

    if (PortName->Length == 8 * sizeof(WCHAR) &&
            _wcsnicmp(PortName->Buffer, L"\\PdcPort", 8) == 0) {

        //
        // on Windows 8 the service tries to connect to an undocumented port
        // provided by a driver pdc.sys.  work around this by faking success
        //

        *PortHandle = NULL;
        status = STATUS_SUCCESS;

    } else {

        typedef NTSTATUS (*P_NtAlpcConnectPort)(
            HANDLE *PortHandle, UNICODE_STRING *PortName,
            void *ObjectAttributes, void *AlpcConnectInfo,
            ULONG ConnectionFlags, void *ServerSid,
            void *ConnectionInfo, void *ConnectionInfoLength,
            void *InMessageBuffer, void *OutMessageBuffer, void *Timeout);

        status = ((P_NtAlpcConnectPort)__sys_NtAlpcConnectPort)(
            PortHandle, PortName, ObjectAttributes, AlpcConnectInfo,
            ConnectionFlags, ServerSid, ConnectionInfo, ConnectionInfoLength,
            InMessageBuffer, OutMessageBuffer, Timeout);
    }

    return status;
}


//---------------------------------------------------------------------------
// WinMain
//---------------------------------------------------------------------------


int __stdcall WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    UCHAR WUAU_ServiceMain[64];
    WCHAR ServiceName[16];
    WCHAR ServiceDll[64];
    BOOL hook_success = TRUE;
    OSVERSIONINFO osvi;
    //BOOL ok;

    Check_Windows_7();

    HOOK_WIN32(CreateProcessW);

    memzero(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx((OSVERSIONINFO *)&osvi);
    if (osvi.dwMajorVersion >= 6) {
        wcscpy(ServiceDll, L"wuaueng.dll");
        strcpy(WUAU_ServiceMain, "WUServiceMain");
    } else {
        wcscpy(ServiceDll, L"wuauserv.dll");
        strcpy(WUAU_ServiceMain, "ServiceMain");
    }

    if ((osvi.dwMajorVersion == 10) || (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion >= 2)) {
        // Windows 10, Windows 8, or Windows 8.1
        void *NtAlpcConnectPort = GetProcAddress(
            GetModuleHandle(L"ntdll.dll"), "NtAlpcConnectPort");
        HOOK_WIN32(NtAlpcConnectPort);
    }

    wcscpy(ServiceName, L"wuauserv");

    // We NEVER want updates to run in the sandbox.  Commenting
    // out the next lines means the update service is not started
    // in the sandbox, so all updates will fail.

    // ok = Service_Start_ServiceMain(
    //            ServiceName, ServiceDll, WUAU_ServiceMain, FALSE);
    //return (ok ? EXIT_SUCCESS : EXIT_FAILURE);

    return EXIT_SUCCESS;
}
