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
// Device Setup
//---------------------------------------------------------------------------

#include "dll.h"

#include <windows.h>
#include <setupapi.h>
#include "dll.h"
#include "common/my_version.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static ULONG Setup_VerifyCatalogFile(const WCHAR *CatalogFullPath);

static ULONG Setup_CM_Add_Driver_PackageW(
    ULONG_PTR Unknown1, ULONG_PTR Unknown2, ULONG_PTR Unknown3,
    ULONG_PTR Unknown4, ULONG_PTR Unknown5, ULONG_PTR Unknown6,
    ULONG_PTR Unknown7, ULONG_PTR Unknown8, ULONG_PTR Unknown9,
    ULONG_PTR Unknown10);

static ULONG Setup_CM_Add_Driver_Package_ExW(
    ULONG_PTR Unknown1, ULONG_PTR Unknown2, ULONG_PTR Unknown3,
    ULONG_PTR Unknown4, ULONG_PTR Unknown5, ULONG_PTR Unknown6,
    ULONG_PTR Unknown7, ULONG_PTR Unknown8, ULONG_PTR Unknown9,
    ULONG_PTR Unknown10, ULONG_PTR Unknown11);


//---------------------------------------------------------------------------


typedef ULONG (*P_VerifyCatalogFile)(const WCHAR *CatalogFullPath);

typedef ULONG (*P_CM_Add_Driver_PackageW)(
    ULONG_PTR Unknown1, ULONG_PTR Unknown2, ULONG_PTR Unknown3,
    ULONG_PTR Unknown4, ULONG_PTR Unknown5, ULONG_PTR Unknown6,
    ULONG_PTR Unknown7, ULONG_PTR Unknown8, ULONG_PTR Unknown9,
    ULONG_PTR Unknown10);

typedef ULONG (*P_CM_Add_Driver_Package_ExW)(
    ULONG_PTR Unknown1, ULONG_PTR Unknown2, ULONG_PTR Unknown3,
    ULONG_PTR Unknown4, ULONG_PTR Unknown5, ULONG_PTR Unknown6,
    ULONG_PTR Unknown7, ULONG_PTR Unknown8, ULONG_PTR Unknown9,
    ULONG_PTR Unknown10, ULONG_PTR Unknown11);


//---------------------------------------------------------------------------


static P_VerifyCatalogFile      __sys_VerifyCatalogFile     = NULL;

static P_CM_Add_Driver_PackageW     __sys_CM_Add_Driver_PackageW    = NULL;
static P_CM_Add_Driver_Package_ExW  __sys_CM_Add_Driver_Package_ExW = NULL;


//---------------------------------------------------------------------------
// Defines for initialization
//---------------------------------------------------------------------------


#define DO_CALL_HOOK(name,devName)                              \
    __sys_##name = SbieDll_Hook(#name, __sys_##name, devName);  \
    if (! __sys_##name) return FALSE;

#define HOOK_AW(func)                                           \
    DO_CALL_HOOK(func##A,Dev_##func##A);                        \
    DO_CALL_HOOK(func##W,Dev_##func##W);

#define HOOK(func)                                              \
    DO_CALL_HOOK(func,Dev_##func);

#define FIND_EP(x) __sys_##x = (P_##x) GetProcAddress(module, #x)
#define FIND_EP1(x,s) __sys_##x##s = (P_##x) GetProcAddress(module, #x#s)
#define FIND_EP2(x) FIND_EP1(x,A); FIND_EP1(x,W);


//---------------------------------------------------------------------------
// Setup_Init_SetupApi
//---------------------------------------------------------------------------


_FX BOOLEAN Setup_Init_SetupApi(HMODULE module)
{
    FIND_EP(VerifyCatalogFile);

    DO_CALL_HOOK(VerifyCatalogFile,Setup_VerifyCatalogFile);

    return TRUE;
}


//---------------------------------------------------------------------------
// Setup_VerifyCatalogFile
//---------------------------------------------------------------------------


_FX ULONG Setup_VerifyCatalogFile(const WCHAR *CatalogFullPath)
{
    // ERROR_AUTHENTICODE_TRUSTED_PUBLISHER     (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR|0x241)
    ULONG rc = __sys_VerifyCatalogFile(CatalogFullPath);
    if (rc != 0 && rc != ERROR_AUTHENTICODE_TRUSTED_PUBLISHER) {
        SetLastError(0);
        rc = 0;
    }
    return rc;
}


//---------------------------------------------------------------------------
// Setup_Init_CfgMgr32
//---------------------------------------------------------------------------


_FX BOOLEAN Setup_Init_CfgMgr32(HMODULE module)
{
    FIND_EP(CM_Add_Driver_PackageW);
    FIND_EP(CM_Add_Driver_Package_ExW);

    if (__sys_CM_Add_Driver_PackageW) {
        DO_CALL_HOOK(
            CM_Add_Driver_PackageW,Setup_CM_Add_Driver_PackageW);
    }
    if (__sys_CM_Add_Driver_Package_ExW) {
        DO_CALL_HOOK(
            CM_Add_Driver_Package_ExW,Setup_CM_Add_Driver_Package_ExW);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Setup_CM_Add_Driver_PackageW
//---------------------------------------------------------------------------


_FX ULONG Setup_CM_Add_Driver_PackageW(
    ULONG_PTR Unknown1, ULONG_PTR Unknown2, ULONG_PTR Unknown3,
    ULONG_PTR Unknown4, ULONG_PTR Unknown5, ULONG_PTR Unknown6,
    ULONG_PTR Unknown7, ULONG_PTR Unknown8, ULONG_PTR Unknown9,
    ULONG_PTR Unknown10)
{
    SbieApi_Log(2205, L"CM Add Driver Package");
    return 0;
}


//---------------------------------------------------------------------------
// Setup_CM_Add_Driver_Package_ExW
//---------------------------------------------------------------------------


_FX ULONG Setup_CM_Add_Driver_Package_ExW(
    ULONG_PTR Unknown1, ULONG_PTR Unknown2, ULONG_PTR Unknown3,
    ULONG_PTR Unknown4, ULONG_PTR Unknown5, ULONG_PTR Unknown6,
    ULONG_PTR Unknown7, ULONG_PTR Unknown8, ULONG_PTR Unknown9,
    ULONG_PTR Unknown10, ULONG_PTR Unknown11)
{
    SbieApi_Log(2205, L"CM Add Driver Package Ex");
    return 0;
}
