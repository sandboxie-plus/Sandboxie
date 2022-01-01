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
// Sandboxie Background Intelligent Transfer Service (BITS)
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


const WCHAR *ServiceTitle = SANDBOXIE L" BITS";
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
// my_LogonUserW
//---------------------------------------------------------------------------


ULONG_PTR __sys_LogonUserW = 0;


BOOL my_LogonUserW(
    const WCHAR *UserName, const WCHAR *DomainName, const WCHAR *Password,
    DWORD dwLogonType, DWORD dwLogonProvider, HANDLE *phToken)
{
    return OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, phToken);
}


//---------------------------------------------------------------------------
// my_CoImpersonateClient
//---------------------------------------------------------------------------


__declspec(dllimport) HRESULT CoImpersonateClient(void);
BOOL WTSQueryUserToken(ULONG SessionId, HANDLE *pToken);


ULONG_PTR __sys_CoImpersonateClient = 0;

HRESULT my_CoImpersonateClient(void)
{
    HANDLE hPriToken, hImpToken;
    BOOL ok;

    if (WTSQueryUserToken(0, &hPriToken)) {
        ok = DuplicateTokenEx(
                hPriToken, TOKEN_ALL_ACCESS, NULL,
                SecurityImpersonation, TokenImpersonation, &hImpToken);
        if (ok) {
            ok = SetThreadToken(NULL, hImpToken);
            CloseHandle(hImpToken);
        }
        CloseHandle(hPriToken);
    }
    return (ok ? S_OK : CO_E_FAILEDTOIMPERSONATE);
}


//---------------------------------------------------------------------------
// WinMain
//---------------------------------------------------------------------------

int __stdcall WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    WCHAR ServiceName[16];
    BOOL hook_success = TRUE;
    BOOL ok;

    //while(!IsDebuggerPresent()) Sleep(500); __debugbreak();

    Check_Windows_7();

    HOOK_WIN32(CoImpersonateClient);
    HOOK_WIN32(LogonUserW);

    wcscpy(ServiceName, L"BITS");
    ok = Service_Start_ServiceMain(
                ServiceName, L"qmgr.dll", "ServiceMain", FALSE);
    return (ok ? EXIT_SUCCESS : EXIT_FAILURE);
}
