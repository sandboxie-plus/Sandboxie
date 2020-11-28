/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020 David Xanatos, xanasoft.com
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
// Sandboxie DCOMLAUNCH Service
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

const WCHAR *ServiceTitle = SANDBOXIE L" DcomLaunch";
#include "../common.h"
#include "../privs.h"

HANDLE RotHintTable = NULL;

const WCHAR *RotHintTable_Name = L"Global\\RotHintTable";


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

    if (lpName && _wcsicmp(lpName, RotHintTable_Name) == 0)
        handle = RotHintTable;

    if (! handle) {
        handle = ((P_CreateFileMappingW)__sys_CreateFileMappingW)(
            hFile, lpAttributes, flProtect,
            dwMaximumSizeHigh, dwMaximumSizeLow, lpName);
    }

    return handle;
}


//---------------------------------------------------------------------------
// ExitWhenParentDies
//---------------------------------------------------------------------------


_FX ULONG ExitWhenParentDies(void *arg)
{
    NTSTATUS status;
    ULONG len;
    PROCESS_BASIC_INFORMATION info;
    HANDLE hParentProcess;

    status = NtQueryInformationProcess(
        NtCurrentProcess(), ProcessBasicInformation,
        &info, sizeof(PROCESS_BASIC_INFORMATION), &len);

    if (! NT_SUCCESS(status))
        hParentProcess = NULL;
    else {
        hParentProcess = OpenProcess(
            SYNCHRONIZE, FALSE, (ULONG)info.InheritedFromUniqueProcessId);
    }
    if (! hParentProcess)
        hParentProcess = NtCurrentProcess();

    status = WaitForSingleObject(hParentProcess, INFINITE);
    if (status == WAIT_OBJECT_0)
        ExitProcess(0);

    return 0;
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
    BOOL ok;
    HANDLE hThreadEvent;

    SetupExceptionHandler();

    Check_Windows_7();

    // pretend we are the SCM
    if (! Hook_Service_Control_Manager())
        return EXIT_FAILURE;

    // hook privilege-related functions
    if (! Hook_Privilege())
        return EXIT_FAILURE;

    hThreadEvent = CreateThread(NULL, 0, ExitWhenParentDies, 0, 0, NULL);

    // start dcom launcher service
    wcscpy(ServiceName, L"DCOMLAUNCH");
    ok = Service_Start_ServiceMain( ServiceName, L"rpcss.dll", "ServiceMain", TRUE);
	if (ok)
		WaitForSingleObject(hThreadEvent, INFINITE);
	CloseHandle(hThreadEvent);
    if (! ok)
        return EXIT_FAILURE;
    return 0;
}
