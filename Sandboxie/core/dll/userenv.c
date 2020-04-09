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
// User Env
//---------------------------------------------------------------------------


#include "dll.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOL UserEnv_RegisterGPNotification(HANDLE hEvent, BOOL bMachine);

static BOOL UserEnv_UnregisterGPNotification(HANDLE hEvent);

static ULONG UserEnv_GetAppliedGPOList(
    ULONG dwFlags, const WCHAR *pMachineName, PSID pSidUser,
    GUID *pGuidExtension, void *ppGPOList);

static BOOL UserEnv_GetVersionExW(LPOSVERSIONINFOEXW lpVersionInfo);
static BOOL UserEnv_GetVersionExA(LPOSVERSIONINFOEXA lpVersionInfo);

//---------------------------------------------------------------------------


typedef BOOL (*P_RegisterGPNotification)(HANDLE hEvent, BOOL bMachine);

typedef BOOL (*P_UnregisterGPNotification)(HANDLE hEvent, BOOL bMachine);

typedef ULONG (*P_GetAppliedGPOList)(
    ULONG dwFlags, const WCHAR *pMachineName, PSID pSidUser,
    GUID *pGuidExtension, void *ppGPOList);

typedef BOOL (*P_GetVersionExW)(LPOSVERSIONINFOEXW lpVersionInfo);
typedef BOOL (*P_GetVersionExA)(LPOSVERSIONINFOEXA lpVersionInfo);

//---------------------------------------------------------------------------


static P_RegisterGPNotification     __sys_RegisterGPNotification    = NULL;
static P_UnregisterGPNotification   __sys_UnregisterGPNotification  = NULL;
static P_GetAppliedGPOList          __sys_GetAppliedGPOList         = NULL;
static P_GetVersionExW              __sys_GetVersionExW             = NULL;
static P_GetVersionExA              __sys_GetVersionExA             = NULL;


//---------------------------------------------------------------------------
// UserEnv_Init
//---------------------------------------------------------------------------


_FX BOOLEAN UserEnv_Init(HMODULE module)
{
    void *RegisterGPNotification;
    void *UnregisterGPNotification;
    void *GetAppliedGPOList;

    if (module == Dll_KernelBase) {
        void *GetVersionExW;
        void *GetVersionExA;

        //
        // on Windows 8.1, UserEnv!GetAppliedGPOList calls
        // KernelBase!GetAppliedGPOListInternalW, which just hangs
        //

        GetAppliedGPOList = (P_GetAppliedGPOList)
            GetProcAddress(module, "GetAppliedGPOListInternalW");

        SBIEDLL_HOOK(UserEnv_,GetAppliedGPOList);

        GetVersionExW = (P_GetVersionExW) GetProcAddress(module, "GetVersionExW");
        GetVersionExA = (P_GetVersionExA) GetProcAddress(module, "GetVersionExA");
        SBIEDLL_HOOK(UserEnv_,GetVersionExW);
        SBIEDLL_HOOK(UserEnv_,GetVersionExA);

    } else {

        //
        // hook UserEnv entrypoints
        //

        RegisterGPNotification = (P_RegisterGPNotification)
            GetProcAddress(module, "RegisterGPNotification");

        UnregisterGPNotification = (P_UnregisterGPNotification)
            GetProcAddress(module, "UnregisterGPNotification");

        SBIEDLL_HOOK(UserEnv_,RegisterGPNotification);
        SBIEDLL_HOOK(UserEnv_,UnregisterGPNotification);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// UserEnv_RegisterGPNotification
//---------------------------------------------------------------------------


_FX BOOL UserEnv_RegisterGPNotification(HANDLE hEvent, BOOL bMachine)
{
    SetLastError(ERROR_SUCCESS);
    return TRUE;
}


//---------------------------------------------------------------------------
// UserEnv_RegisterGPNotification
//---------------------------------------------------------------------------


_FX BOOL UserEnv_UnregisterGPNotification(HANDLE hEvent)
{
    SetLastError(ERROR_SUCCESS);
    return TRUE;
}


//---------------------------------------------------------------------------
// UserEnv_GetAppliedGPOList
//---------------------------------------------------------------------------


ULONG UserEnv_GetAppliedGPOList(
    ULONG dwFlags, const WCHAR *pMachineName, PSID pSidUser,
    GUID *pGuidExtension, void *ppGPOList)
{
    // emulate error return code from KernelBase!GetAppliedGPOListInternalW
    SetLastError(ERROR_INVALID_FUNCTION);
    return ERROR_PROC_NOT_FOUND;
}

//---------------------------------------------------------------------------
// UserEnv_GetVersionExW
//---------------------------------------------------------------------------

_FX BOOL UserEnv_GetVersionExW(LPOSVERSIONINFOEXW lpVersionInfo)
{
    // Get the version from the kernel
    NTSTATUS (WINAPI *RtlGetVersion)(LPOSVERSIONINFOEXW);
    *(FARPROC*)&RtlGetVersion = GetProcAddress(GetModuleHandleW(L"ntdll"), "RtlGetVersion");

    if (RtlGetVersion != NULL)
    {
        RtlGetVersion(lpVersionInfo);

        // RtlGetVersion always returns STATUS_SUCCESS
        return TRUE;
    }

    // Error
    return FALSE;
}

_FX BOOL UserEnv_GetVersionExA(LPOSVERSIONINFOEXA lpVersionInfo)
{
    BOOL rc;
    rc = __sys_GetVersionExA(lpVersionInfo);
    lpVersionInfo->dwMajorVersion = GET_PEB_MAJOR_VERSION;
    lpVersionInfo->dwMinorVersion = GET_PEB_MINOR_VERSION;
    return rc;
}
