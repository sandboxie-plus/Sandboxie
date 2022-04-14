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

/*static BOOL Setup_SetupDiCallClassInstaller(
    _In_ DI_FUNCTION InstallFunction,
    _In_ HDEVINFO DeviceInfoSet,
    _In_opt_ PSP_DEVINFO_DATA DeviceInfoData);*/

/*static BOOL Setup_SetupDiRemoveDevice(
    HDEVINFO         DeviceInfoSet,
    PSP_DEVINFO_DATA DeviceInfoData);*/

/*static BOOL Setup_DiUninstallDriverW(
    HWND    hwndParent,
    LPCWSTR InfPath,
    DWORD   Flags,
    PBOOL   NeedReboot);*/

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

/*static ULONG Setup_CM_Query_And_Remove_SubTreeW(
    PVOID          dnAncestor, // DEVINST
    PVOID          pVetoType, // PPNP_VETO_TYPE
    LPWSTR         pszVetoName,
    ULONG          ulNameLength,
    ULONG          ulFlags);*/

/*static ULONG Setup_CM_Uninstall_DevNode(
    PVOID   dnDevInst, // DEVNODE
    ULONG   ulFlags);*/


//---------------------------------------------------------------------------


typedef ULONG (*P_VerifyCatalogFile)(const WCHAR *CatalogFullPath);

/*typedef BOOL (*P_SetupDiCallClassInstaller)(
    _In_ DI_FUNCTION InstallFunction,
    _In_ HDEVINFO DeviceInfoSet,
    _In_opt_ PSP_DEVINFO_DATA DeviceInfoData);*/

/*typedef BOOL (*P_SetupDiRemoveDevice)(
    HDEVINFO         DeviceInfoSet,
    PSP_DEVINFO_DATA DeviceInfoData);*/

/*typedef BOOL (*P_DiUninstallDriverW)(
    HWND    hwndParent,
    LPCWSTR InfPath,
    DWORD   Flags,
    PBOOL   NeedReboot);*/

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

/*typedef ULONG (*P_CM_Query_And_Remove_SubTreeW)(
    PVOID          dnAncestor, // DEVINST
    PVOID          pVetoType, // PPNP_VETO_TYPE
    LPWSTR         pszVetoName,
    ULONG          ulNameLength,
    ULONG          ulFlags);*/

/*typedef ULONG (*P_CM_Uninstall_DevNode)(
    PVOID   dnDevInst, // DEVNODE
    ULONG   ulFlags);*/

//---------------------------------------------------------------------------


static P_VerifyCatalogFile              __sys_VerifyCatalogFile = NULL;
//static P_SetupDiCallClassInstaller      __sys_SetupDiCallClassInstaller = NULL;
//static P_SetupDiRemoveDevice            __sys_SetupDiRemoveDevice = NULL;

//static P_DiUninstallDriverW     __sys_DiUninstallDriverW = NULL;

static P_CM_Add_Driver_PackageW     __sys_CM_Add_Driver_PackageW    = NULL;
static P_CM_Add_Driver_Package_ExW  __sys_CM_Add_Driver_Package_ExW = NULL;

//static P_CM_Query_And_Remove_SubTreeW __sys_CM_Query_And_Remove_SubTreeW = NULL;
//static P_CM_Uninstall_DevNode       __sys_CM_Uninstall_DevNode = NULL;

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
    //FIND_EP(SetupDiCallClassInstaller);
    //FIND_EP(SetupDiRemoveDevice);

    DO_CALL_HOOK(VerifyCatalogFile,Setup_VerifyCatalogFile);
    //DO_CALL_HOOK(SetupDiCallClassInstaller,Setup_SetupDiCallClassInstaller);
    //DO_CALL_HOOK(SetupDiRemoveDevice, Setup_SetupDiRemoveDevice);
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
// Setup_SetupDiCallClassInstaller
//---------------------------------------------------------------------------


/*_FX BOOL Setup_SetupDiCallClassInstaller(
    _In_ DI_FUNCTION InstallFunction,
    _In_ HDEVINFO DeviceInfoSet,
    _In_opt_ PSP_DEVINFO_DATA DeviceInfoData)
{
    return __sys_SetupDiCallClassInstaller(InstallFunction, DeviceInfoSet, DeviceInfoData);
}*/


//---------------------------------------------------------------------------
// Setup_SetupDiRemoveDevice
//---------------------------------------------------------------------------


/*static BOOL Setup_SetupDiRemoveDevice(
    HDEVINFO         DeviceInfoSet,
    PSP_DEVINFO_DATA DeviceInfoData)
{
    return TRUE;
}*/


//---------------------------------------------------------------------------
// Setup_Init_NewDev
//---------------------------------------------------------------------------


/*_FX BOOLEAN Setup_Init_NewDev(HMODULE module)
{
    FIND_EP(DiUninstallDriverW);

    DO_CALL_HOOK(DiUninstallDriverW,Setup_DiUninstallDriverW);

    return TRUE;
}

static BOOL Setup_DiUninstallDriverW(
    HWND    hwndParent,
    LPCWSTR InfPath,
    DWORD   Flags,
    PBOOL   NeedReboot)
{
    return TRUE;
}*/


//---------------------------------------------------------------------------
// Setup_Init_CfgMgr32
//---------------------------------------------------------------------------


_FX BOOLEAN Setup_Init_CfgMgr32(HMODULE module)
{
    FIND_EP(CM_Add_Driver_PackageW);
    FIND_EP(CM_Add_Driver_Package_ExW);
    // Note: When the Add_Driver_Package is not hooked it will attempt to contact the deviceinstall service,
    // which uses a dynamic rpc port, heence as long as there is no blank OpenIpcPath=* this call will fail
    // We hook these two functions only to provide the user a SBIE2205 informing, that drivers can't be installed.
    if (__sys_CM_Add_Driver_PackageW) {
        DO_CALL_HOOK(
            CM_Add_Driver_PackageW,Setup_CM_Add_Driver_PackageW);
    }
    if (__sys_CM_Add_Driver_Package_ExW) {
        DO_CALL_HOOK(
            CM_Add_Driver_Package_ExW,Setup_CM_Add_Driver_Package_ExW);
    }

    //FIND_EP(CM_Query_And_Remove_SubTreeW);
    //FIND_EP(CM_Uninstall_DevNode);
    // Note: most of the CM_ functions use the "\Device\DeviceApi\CMApi" device/file for communication,
    // these requests are filtered by the driver and we let them silently fail.
    /*if (__sys_CM_Query_And_Remove_SubTreeW) {
        DO_CALL_HOOK(
            CM_Query_And_Remove_SubTreeW,Setup_CM_Query_And_Remove_SubTreeW);
    }*/
    /*if (__sys_CM_Uninstall_DevNode) {
        DO_CALL_HOOK(
            CM_Uninstall_DevNode,Setup_CM_Uninstall_DevNode);
    }*/

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


//---------------------------------------------------------------------------
// Setup_CM_Add_Driver_Package_ExW
//---------------------------------------------------------------------------


/*static ULONG Setup_CM_Query_And_Remove_SubTreeW(
    PVOID          dnAncestor, // DEVINST
    PVOID          pVetoType, // PPNP_VETO_TYPE
    LPWSTR         pszVetoName,
    ULONG          ulNameLength,
    ULONG          ulFlags)
{
    return __sys_CM_Query_And_Remove_SubTreeW(dnAncestor, pVetoType, pszVetoName, ulNameLength, ulFlags);
}*/


//---------------------------------------------------------------------------
// Setup_CM_Add_Driver_Package_ExW
//---------------------------------------------------------------------------


/*static ULONG Setup_CM_Uninstall_DevNode(
    PVOID   dnDevInst, // DEVNODE
    ULONG   ulFlags)
{
    LONG ret = __sys_CM_Uninstall_DevNode(dnDevInst, ulFlags);

    return ret;
}*/