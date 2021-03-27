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
// Sandboxie Cryptography Service (CRYPTSVC)
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


#define SINGLE_INSTANCE_MUTEX_NAME L"ServiceCrypto_Mutex1"

const WCHAR *ServiceTitle = SANDBOXIE L" Crypto";
#define SANDBOXIECRYPTO 1
#include "../common.h"
#include "../privs.h"


//---------------------------------------------------------------------------


static ULONG_PTR __sys_DuplicateHandle                          = 0;
static ULONG_PTR __sys_CreateFileW                              = 0;


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
// my_DuplicateHandle
//---------------------------------------------------------------------------


ALIGNED BOOL my_DuplicateHandle(
    HANDLE hSourceProcessHandle,
    HANDLE hSourceHandle,
    HANDLE hTargetProcessHandle,
    LPHANDLE lpTargetHandle,
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    DWORD dwOptions)
{
    typedef BOOL (*P_DuplicateHandle)(
        HANDLE hSourceProcessHandle,
        HANDLE hSourceHandle,
        HANDLE hTargetProcessHandle,
        LPHANDLE lpTargetHandle,
        DWORD dwDesiredAccess,
        BOOL bInheritHandle,
        DWORD dwOptions);

    BOOL ok = ((P_DuplicateHandle)__sys_DuplicateHandle)(
                    hSourceProcessHandle, hSourceHandle,
                    hTargetProcessHandle, lpTargetHandle,
                    dwDesiredAccess, bInheritHandle, dwOptions);

    if ((! ok) && GetLastError() == ERROR_ACCESS_DENIED) {

        //
        // the client process for the CryptSvc service is ocassionally
        // running under LocalSystem (typically during MSI installations).
        // CryptSvc tries to duplicate an event handle and fails so
        // we fake successful operation.
        //

        if (hSourceProcessHandle != NtCurrentProcess() &&
            hTargetProcessHandle == NtCurrentProcess()) {

            *lpTargetHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
            SetLastError(ERROR_SUCCESS);
            ok = TRUE;
        }
    }

    return ok;
}


//---------------------------------------------------------------------------
// my_CreateFileW
//---------------------------------------------------------------------------

ALIGNED HANDLE my_CreateFileW(
    LPCWSTR               lpFileName,
    DWORD                 dwDesiredAccess,
    DWORD                 dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD                 dwCreationDisposition,
    DWORD                 dwFlagsAndAttributes,
    HANDLE                hTemplateFile)
{
    typedef HANDLE(*P_CreateFileW)(
        LPCWSTR               lpFileName,
        DWORD                 dwDesiredAccess,
        DWORD                 dwShareMode,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        DWORD                 dwCreationDisposition,
        DWORD                 dwFlagsAndAttributes,
        HANDLE                hTemplateFile);

    //
    // prevent SBIE1313, dont even try to access the block devcie for raw reading
    //

    if (wcsnicmp(lpFileName, L"\\\\.\\PhysicalDrive", 17) == 0 && wcschr(lpFileName + 17, L'\\') == NULL) {
        if (dwDesiredAccess == GENERIC_READ)
            dwDesiredAccess = 0;
    }

    //
    // issue #561 Sandbox with some apps directly uses catdb than blocks access to it
    // to prevent our instance form locking the real file we request write access
    // that forces the file to be migrated and our sandboxed copy opened
    //
    
    WCHAR* CatRoot = wcsstr(lpFileName, L"\\system32\\CatRoot2\\");
    if (CatRoot) { // L"C:\\WINDOWS\\system32\\CatRoot2\\{00000000-0000-0000-0000-000000000000}\\catdb"
        WCHAR win_dir[MAX_PATH + 64];
        GetWindowsDirectory(win_dir, MAX_PATH);
        if (wcsnicmp(win_dir, lpFileName, CatRoot - lpFileName) == 0) {
            if (dwDesiredAccess == GENERIC_READ)
                dwDesiredAccess |= GENERIC_WRITE;
        }
    }


    return ((P_CreateFileW)__sys_CreateFileW)(lpFileName, dwDesiredAccess, dwShareMode,
        lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
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
    PROCESS_DATA *myData;

    if (! CheckSingleInstance()) {
        // return special error code to indicate elective termination
        // because this is a duplicate copy of SandboxieRpcSs.exe
        // see also core/dll/ipc_start.c
        return STATUS_LICENSE_QUOTA_EXCEEDED;
    }

    Check_Windows_7();

    SetupExceptionHandler();

    HOOK_WIN32(DuplicateHandle);

    HOOK_WIN32(CreateFileW);

    // hook privilege-related functions
    if (! Hook_Privilege())
        return EXIT_FAILURE;

    wcscpy(ServiceName, L"CryptSvc");

    // hook SetServiceStatus
    // use ServiceStatus_Check to signal the CryptSvc init complete event
 
    myData = my_findProcessData(ServiceName, 1);
    if (!myData) {
        return FALSE;
    }
    HOOK_WIN32(SetServiceStatus);
    myData->tid = 0;
    myData->initFlag = 0;

    // run the service
    ok = Service_Start_ServiceMain(ServiceName, L"cryptsvc.dll", "CryptServiceMain", FALSE);
    return (ok ? EXIT_SUCCESS : EXIT_FAILURE);
}
