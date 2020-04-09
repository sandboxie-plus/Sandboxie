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
// AdvApi32
//---------------------------------------------------------------------------


#include "dll.h"
#include "advapi.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOL AdvApi_LookupAccountNameW(
    void *lpSystemName, void *lpAccountName, PSID Sid, LPDWORD cbSid,
    void *ReferencedDomainName, LPDWORD cchReferencedDomainName,
    PSID_NAME_USE peUse);

static ULONG AdvApi_RegConnectRegistryA(
    void *lpMachineName, HKEY hKey, HKEY *phkResult);

static ULONG AdvApi_RegConnectRegistryW(
    void *lpMachineName, HKEY hKey, HKEY *phkResult);

static BOOL AdvApi_AccessCheckByType(
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    PSID PrincipalSelfSid,
    HANDLE ClientToken,
    DWORD DesiredAccess,
    POBJECT_TYPE_LIST ObjectTypeList,
    DWORD ObjectTypeListLength,
    PGENERIC_MAPPING GenericMapping,
    PPRIVILEGE_SET PrivilegeSet,
    LPDWORD PrivilegeSetLength,
    LPDWORD GrantedAccess,
    LPBOOL AccessStatus);

static BOOL AdvApi_SaferComputeTokenFromLevel(
    ULONG_PTR LevelHandle, HANDLE InAccessToken, HANDLE *OutAccessToken,
    ULONG dwFlags, void *lpReserved);

static ULONG AdvApi_GetEffectiveRightsFromAclW(
    PACL pacl, void *pTrustee, PACCESS_MASK pAccessRights);

/*static ULONG AdvApi_SaferiIsDllAllowed(
    HANDLE FileHandle, const UNICODE_STRING *FileName, ULONG_PTR Unknown);*/

static ULONG AdvApi_CreateRestrictedToken(
    HANDLE ExistingTokenHandle,
    DWORD Flags,
    DWORD DisableSidCount,
    PSID_AND_ATTRIBUTES SidsToDisable,
    DWORD DeletePrivilegeCount,
    PLUID_AND_ATTRIBUTES PrivilegesToDelete,
    DWORD RestrictedSidCount,
    PSID_AND_ATTRIBUTES SidsToRestrict,
    PHANDLE NewTokenHandle);

static DWORD AdvApi_SetSecurityInfo(
    HANDLE handle,
    SE_OBJECT_TYPE ObjectType,
    SECURITY_INFORMATION SecurityInfo,
    PSID psidOwner,
    PSID psidGroup,
    PACL pDacl,
    PACL pSacl);

static DWORD Ntmarta_SetSecurityInfo(
    HANDLE handle,
    SE_OBJECT_TYPE ObjectType,
    SECURITY_INFORMATION SecurityInfo,
    PSID psidOwner,
    PSID psidGroup,
    PACL pDacl,
    PACL pSacl);

static DWORD AdvApi_GetSecurityInfo(
    HANDLE handle,
    SE_OBJECT_TYPE ObjectType,
    SECURITY_INFORMATION SecurityInfo,
    PSID psidOwner,
    PSID psidGroup,
    PACL pDacl,
    PACL pSacl,
    PSECURITY_DESCRIPTOR *ppSecurityDescriptor);

//---------------------------------------------------------------------------


typedef BOOL (*P_SaferComputeTokenFromLevel)(
    ULONG_PTR LevelHandle, HANDLE InAccessToken, HANDLE *OutAccessToken,
    ULONG dwFlags, void *lpReserved);

typedef ULONG (*P_GetEffectiveRightsFromAcl)(
    PACL pacl, void *pTrustee, PACCESS_MASK pAccessRights);

typedef BOOL (*P_EnumWindowStations) (_In_ WINSTAENUMPROC lpEnumFunc, _In_ LPARAM lParam);
typedef HANDLE (*P_OpenWindowStationW) (LPCWSTR lpszWinSta, BOOL fInherit, ACCESS_MASK dwDesiredAccess);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


       P_RegOpenKeyEx           __sys_RegOpenKeyExA             = NULL;
       P_RegOpenKeyEx           __sys_RegOpenKeyExW             = NULL;

       P_RegCloseKey            __sys_RegCloseKey               = NULL;

       P_RegGetKeySecurity      __sys_RegGetKeySecurity         = NULL;

       P_RegSetKeySecurity      __sys_RegSetKeySecurity         = NULL;

       P_GetFileSecurity        __sys_GetFileSecurityW          = NULL;
       P_GetFileSecurity        __sys_GetFileSecurityA          = NULL;

       P_SetFileSecurity        __sys_SetFileSecurityW          = NULL;
       P_SetFileSecurity        __sys_SetFileSecurityA          = NULL;

       P_CreateProcessAsUser    __sys_CreateProcessAsUserW      = NULL;
       P_CreateProcessAsUser    __sys_CreateProcessAsUserA      = NULL;

       P_LookupAccountName      __sys_LookupAccountNameW        = NULL;

       P_LookupPrivilegeValue   __sys_LookupPrivilegeValueW     = NULL;

       P_RegConnectRegistry     __sys_RegConnectRegistryW       = NULL;
       P_RegConnectRegistry     __sys_RegConnectRegistryA       = NULL;

       P_CreateRestrictedToken  __sys_CreateRestrictedToken = NULL;
       P_SetSecurityInfo        __sys_SetSecurityInfo = NULL;
       P_GetSecurityInfo        __sys_GetSecurityInfo = NULL;
       P_SetSecurityInfo        __sys_Ntmarta_SetSecurityInfo = NULL;
       P_GetSecurityInfo        __sys_Ntmarta_GetSecurityInfo = NULL;

       P_SaferComputeTokenFromLevel __sys_SaferComputeTokenFromLevel = NULL;

       P_GetEffectiveRightsFromAcl __sys_GetEffectiveRightsFromAclW = NULL;
       P_EnumWindowStations     __sys_EnumWindowStationsW       = NULL;
       P_OpenWindowStationW     __sys_OpenWindowStationW        = NULL;


static HMODULE AdvApi_Module = NULL;
static HMODULE Gui_Module = NULL;
static ULONG AdvApi_EnableSRP = TRUE;


//---------------------------------------------------------------------------
// AdvApi_Init
//---------------------------------------------------------------------------


_FX BOOLEAN AdvApi_Init(HMODULE module)
{
    P_LookupAccountName LookupAccountNameW;
    P_RegConnectRegistry RegConnectRegistryA, RegConnectRegistryW;
    P_CreateRestrictedToken CreateRestrictedToken;
    P_SetSecurityInfo SetSecurityInfo;
    P_GetSecurityInfo GetSecurityInfo;

    BOOLEAN ok;

    AdvApi_Module = module;

#define GETPROC(x,s) __sys_##x##s = (P_##x) Ldr_GetProcAddrNew(DllName_advapi32, L#x L#s,#x #s);

    GETPROC(RegOpenKeyEx,A);
    GETPROC(RegOpenKeyEx,W);
    GETPROC(RegCloseKey,);
    GETPROC(RegGetKeySecurity,);
    GETPROC(RegSetKeySecurity,);
    GETPROC(GetFileSecurity,A);
    GETPROC(GetFileSecurity,W);
    GETPROC(SetFileSecurity,A);
    GETPROC(SetFileSecurity,W);
    GETPROC(LookupAccountName,W);
    GETPROC(LookupPrivilegeValue,W);
    GETPROC(RegConnectRegistry,A);
    GETPROC(RegConnectRegistry,W);
    GETPROC(CreateRestrictedToken, );
    GETPROC(SetSecurityInfo, );
    GETPROC(GetSecurityInfo, );
    GETPROC(CreateProcessAsUser,A);


    if (! Dll_KernelBase) {
        // on Windows 7, CreateProcessAsUserW is in kernel32
        GETPROC(CreateProcessAsUser,W);
    }

    GETPROC(GetEffectiveRightsFromAcl,W);

    //GETPROC(SaferiIsDllAllowed,);

#undef GETPROC

    ok = Scm_Init_AdvApi(module);
    if (! ok)
        return FALSE;

    ok = Proc_Init_AdvApi(module);
    if (! ok)
        return FALSE;

    ok = Cred_Init_AdvApi(module);
    if (! ok)
        return FALSE;

    LookupAccountNameW = __sys_LookupAccountNameW;
    SBIEDLL_HOOK(AdvApi_,LookupAccountNameW);

    RegConnectRegistryA = __sys_RegConnectRegistryA;
    SBIEDLL_HOOK(AdvApi_,RegConnectRegistryA);

    RegConnectRegistryW = __sys_RegConnectRegistryW;
    SBIEDLL_HOOK(AdvApi_,RegConnectRegistryW);

    CreateRestrictedToken = __sys_CreateRestrictedToken;
    SBIEDLL_HOOK(AdvApi_, CreateRestrictedToken);

    // only hook SetSecurityInfo if this is Chrome.  Outlook 2013 uses delayed loading and will cause infinite callbacks
    // Starting with Win 10, we only want to hook ntmarta!SetSecurityInfo. Do NOT hook advapi!SetSecurityInfo. Delay loading for advapi will cause infinite recursion.
    if (((Dll_ImageType == DLL_IMAGE_GOOGLE_CHROME) || (Dll_ImageType == DLL_IMAGE_ACROBAT_READER)) && (Dll_Windows < 10)) {
        SetSecurityInfo = __sys_SetSecurityInfo;
        GetSecurityInfo = __sys_GetSecurityInfo;
        SBIEDLL_HOOK(AdvApi_, SetSecurityInfo);
        SBIEDLL_HOOK(AdvApi_, GetSecurityInfo);
    }

    if (__sys_GetEffectiveRightsFromAclW) {
        void *GetEffectiveRightsFromAclW = __sys_GetEffectiveRightsFromAclW;
        SBIEDLL_HOOK(AdvApi_,GetEffectiveRightsFromAclW);
    }

    //
    // AppLocker rules don't work correctly because kernel driver routines
    // appid!SrpVerifyDll and appid!SrpDoesPolicyApply use the primary token
    // for SeAccessCheck, and this always fails.  work around this
    //
    // note:  the SANDBOX_INERT flag used in Token_ReplacePrimary in file
    // core/drv/token.c disables SRP/AppLocker so this hook is unnecessary
    //

    //
    // some Windows services use AccessCheckByType to verify the callers
    // are using the right SIDs.  fake success for the following:
    // wuauclt.exe
    // SandboxieWUAU (wuauserv service)
    // SandboxieBITS (bits service)
    //

    if (Dll_Windows < 10) {
        if (Dll_ImageType == DLL_IMAGE_SANDBOXIE_BITS ||
            Dll_ImageType == DLL_IMAGE_SANDBOXIE_WUAU ||
            Dll_ImageType == DLL_IMAGE_WUAUCLT) {

            void *AccessCheckByType = Ldr_GetProcAddrNew(
                (Dll_KernelBase ? DllName_kernelbase : DllName_advapi32),
                L"AccessCheckByType", "AccessCheckByType");
            if (AccessCheckByType) {
                void *__sys_AccessCheckByType;
                SBIEDLL_HOOK(AdvApi_, AccessCheckByType);
            }
        }
    }
    return TRUE;
}


//---------------------------------------------------------------------------
// AdvApi_LookupAccountNameW
//---------------------------------------------------------------------------


_FX BOOL AdvApi_LookupAccountNameW(
    void *lpSystemName, void *lpAccountName, PSID Sid, LPDWORD cbSid,
    void *ReferencedDomainName, LPDWORD cchReferencedDomainName,
    PSID_NAME_USE peUse)
{
    BOOL ok = __sys_LookupAccountNameW(
                    lpSystemName, lpAccountName, Sid, cbSid,
                    ReferencedDomainName, cchReferencedDomainName, peUse);

    if ((! ok) && GetLastError() == ERROR_NONE_MAPPED &&
            (! lpSystemName) && lpAccountName) {

        WCHAR *name = (WCHAR *)lpAccountName;
        if (name && _wcsnicmp(name, L"NT SERVICE\\", 11) == 0) {
            name = L"NT AUTHORITY\\SYSTEM";

            ok = __sys_LookupAccountNameW(
                    lpSystemName, name, Sid, cbSid,
                    ReferencedDomainName, cchReferencedDomainName, peUse);
        }
    }

    return ok;
}


//---------------------------------------------------------------------------
// AdvApi_RegConnectRegistryA
//---------------------------------------------------------------------------


_FX ULONG AdvApi_RegConnectRegistryA(
    void *lpMachineName, HKEY hKey, HKEY *phkResult)
{
    if (lpMachineName && *(UCHAR *)lpMachineName)
        return ERROR_ACCESS_DENIED;
    return __sys_RegOpenKeyExA(hKey, NULL, 0, MAXIMUM_ALLOWED, phkResult);
}


//---------------------------------------------------------------------------
// AdvApi_RegConnectRegistryW
//---------------------------------------------------------------------------


_FX ULONG AdvApi_RegConnectRegistryW(
    void *lpMachineName, HKEY hKey, HKEY *phkResult)
{
    if (lpMachineName && *(WCHAR *)lpMachineName)
        return ERROR_ACCESS_DENIED;
    return __sys_RegOpenKeyExW(hKey, NULL, 0, MAXIMUM_ALLOWED, phkResult);
}


//---------------------------------------------------------------------------
// AdvApi_CreateRestrictedToken
//---------------------------------------------------------------------------


_FX ULONG AdvApi_CreateRestrictedToken(
    HANDLE ExistingTokenHandle,
    DWORD Flags,
    DWORD DisableSidCount,
    PSID_AND_ATTRIBUTES SidsToDisable,
    DWORD DeletePrivilegeCount,
    PLUID_AND_ATTRIBUTES PrivilegesToDelete,
    DWORD RestrictedSidCount,
    PSID_AND_ATTRIBUTES SidsToRestrict,
    PHANDLE NewTokenHandle)
{
    DWORD   i, n;
    LUID    luidChangeNotify;
    BOOL    bChangeNotifyFound = FALSE;
    BOOL    bResult;
    PLUID_AND_ATTRIBUTES pModifiedPrivilegesToDelete = NULL;
    DWORD   dwModifiedDeletePrivilegeCount = DeletePrivilegeCount;
    PSID_AND_ATTRIBUTES pModifiedDisableSids = NULL;
    DWORD dwModifiedDisableSidCount = DisableSidCount;
    PSID_AND_ATTRIBUTES pModifiedRestrictedSids = NULL;
    DWORD dwModifiedRestrictedSidCount = RestrictedSidCount;
    //LPWSTR        pStr;

#ifdef CHROME_DEBUG
    if (Dll_ImageType == DLL_IMAGE_GOOGLE_CHROME)
    {
        //while (!IsDebuggerPresent())
            //Sleep(500);
        //__debugbreak();

        OutputDebugStringW(L"Restrict:\n");
        pModifiedRestrictedSids = GlobalAlloc(GMEM_FIXED, sizeof(SID_AND_ATTRIBUTES) * RestrictedSidCount);
        memset(pModifiedRestrictedSids, 0xcd, sizeof(SID_AND_ATTRIBUTES) * RestrictedSidCount);

        for (i = 0, n = 0; i < RestrictedSidCount; i++)
        {
            ConvertSidToStringSidW(SidsToRestrict[i].Sid, &pStr);
            OutputDebugStringW(pStr);
            OutputDebugStringW(L"\n");
            if ((wcscmp(pStr, L"X-99-99-99") == 0)  // dummy
                || (wcscmp(pStr, L"S-1-1-0") == 0)  // Everyone
                //|| (wcscmp(pStr, L"S-1-5-21-3276156215-2989485263-3803298249-1002") == 0) // 
                //|| (wcscmp(pStr, L"S-1-5-21-3276156215-2989485263-3803298249-513") == 0)  // 
                //|| (wcscmp(pStr, L"S-1-5-114") == 0)  // NT AUTHORITY\Local account and member of Administrators group)
                //|| (wcscmp(pStr, L"S-1-5-32-544") == 0)   // BUILTIN\Administrators
                //|| (wcscmp(pStr, L"S-1-5-32-545") == 0)   // BUILTIN\Users
                //|| (wcscmp(pStr, L"S-1-5-4") == 0)    // NT AUTHORITY\INTERACTIVE
                //|| (wcscmp(pStr, L"S-1-2-1") == 0)    // localhost\CONSOLE LOGON
                //|| (wcscmp(pStr, L"S-1-5-11") == 0)   // NT AUTHORITY\Authenticated Users
                //|| (wcscmp(pStr, L"S-1-5-15") == 0)   // NT AUTHORITY\This Organization
                //|| (wcscmp(pStr, L"S-1-5-113") == 0)  // NT AUTHORITY\Local account
                //|| (wcscmp(pStr, L"S-1-2-0") == 0)    // localhost\LOCAL)
                //|| (wcscmp(pStr, L"S-1-5-64-10") == 0)    // NT AUTHORITY\NTLM Authentication
                //|| (wcscmp(pStr, L"S-1-5-5-0-403937") == 0)   // WTF
                )
                --dwModifiedRestrictedSidCount;
            else
                pModifiedRestrictedSids[n++] = SidsToRestrict[i];
            LocalFree(pStr);
        }

        OutputDebugStringW(L"Disable:\n");
        pModifiedDisableSids = GlobalAlloc(GMEM_FIXED, sizeof(SID_AND_ATTRIBUTES) * DisableSidCount);
        for (i = 0, n = 0; i < DisableSidCount; i++)
        {
            ConvertSidToStringSidW(SidsToDisable[i].Sid, &pStr);
            OutputDebugStringW(pStr);
            OutputDebugStringW(L"\n");
            if ((wcscmp(pStr, L"X-99-99-99") == 0)  // dummy
                || (wcscmp(pStr, L"S-1-1-0") == 0)  // Everyone
                //|| (wcscmp(pStr, L"S-1-5-21-3276156215-2989485263-3803298249-1002") == 0) // 
                //|| (wcscmp(pStr, L"S-1-5-21-3276156215-2989485263-3803298249-513") == 0)  // 
                //|| (wcscmp(pStr, L"S-1-5-114") == 0)  // NT AUTHORITY\Local account and member of Administrators group)
                //|| (wcscmp(pStr, L"S-1-5-32-544") == 0)   // BUILTIN\Administrators
                //|| (wcscmp(pStr, L"S-1-5-32-545") == 0)   // BUILTIN\Users
                //|| (wcscmp(pStr, L"S-1-5-4") == 0)    // NT AUTHORITY\INTERACTIVE
                //|| (wcscmp(pStr, L"S-1-2-1") == 0)    // localhost\CONSOLE LOGON
                //|| (wcscmp(pStr, L"S-1-5-11") == 0)   // NT AUTHORITY\Authenticated Users
                //|| (wcscmp(pStr, L"S-1-5-15") == 0)   // NT AUTHORITY\This Organization
                //|| (wcscmp(pStr, L"S-1-5-113") == 0)  // NT AUTHORITY\Local account
                //|| (wcscmp(pStr, L"S-1-2-0") == 0)    // localhost\LOCAL)
                //|| (wcscmp(pStr, L"S-1-5-64-10") == 0)    // NT AUTHORITY\NTLM Authentication
                //|| (wcscmp(pStr, L"S-1-5-5-0-403937") == 0)   // WTF
                )
                --dwModifiedDisableSidCount;
            else
                pModifiedDisableSids[n++] = SidsToDisable[i];

            LocalFree(pStr);
        }

    }
#endif

    // This is a HACK to get Chrome 37 to work with dropped rights.  A work in progress.
    // filter out the SE_CHANGE_NOTIFY_NAME if it is in the list

    //OutputDebugStringW(L"Privileges\n");
    __sys_LookupPrivilegeValueW(NULL, SE_CHANGE_NOTIFY_NAME, &luidChangeNotify);
    pModifiedPrivilegesToDelete = GlobalAlloc(GMEM_FIXED, sizeof(LUID_AND_ATTRIBUTES) * DeletePrivilegeCount);

    for (i = 0; i < DeletePrivilegeCount; i++)
    {
        //wchar_t   buf[80];
        //DWORD len;
        //len = 80;
        //LookupPrivilegeNameW(NULL, &PrivilegesToDelete[i], &buf, &len);
        //OutputDebugStringW(buf);
        //OutputDebugStringW(L"\n");

        for (i = 0, n = 0; i < DeletePrivilegeCount; i++)
        {
            if ((PrivilegesToDelete[i].Luid.HighPart == luidChangeNotify.HighPart) && (PrivilegesToDelete[i].Luid.LowPart == luidChangeNotify.LowPart))
                --dwModifiedDeletePrivilegeCount;
            else
                pModifiedPrivilegesToDelete[n++] = PrivilegesToDelete[i];
        }
    }

    bResult = __sys_CreateRestrictedToken(ExistingTokenHandle, Flags,
        DisableSidCount, SidsToDisable,
        //dwModifiedDisableSidCount, pModifiedDisableSids,
        //DeletePrivilegeCount, PrivilegesToDelete,
        dwModifiedDeletePrivilegeCount, pModifiedPrivilegesToDelete,
        RestrictedSidCount, SidsToRestrict,
        //dwModifiedRestrictedSidCount, pModifiedRestrictedSids,
        NewTokenHandle);

    if (pModifiedPrivilegesToDelete)
        GlobalFree(pModifiedPrivilegesToDelete);
    if (pModifiedRestrictedSids)
        GlobalFree(pModifiedRestrictedSids);
    if (pModifiedDisableSids)
        GlobalFree(pModifiedDisableSids);
    return bResult;

}


HANDLE Sandboxie_WinSta = 0;

BOOL CALLBACK myEnumWindowStationProc(
    _In_ LPTSTR lpszWindowStation,
    _In_ LPARAM lParam);

// Get Sandbox Dummy WindowStation Handle
BOOL CALLBACK myEnumWindowStationProc(
    _In_ LPTSTR lpszWindowStation,
    _In_ LPARAM lParam)
{
    if ((!lpszWindowStation) || (!__sys_OpenWindowStationW)) {
        return FALSE;
    }
    if (!_wcsnicmp(lpszWindowStation, L"Sandbox", 7)) {
        Sandboxie_WinSta = __sys_OpenWindowStationW(lpszWindowStation, 1, WINSTA_ALL_ACCESS | STANDARD_RIGHTS_REQUIRED);
        return FALSE;
    }
    return TRUE;
}


// Chrome 52+ now needs to be able to create a WindowStation and Desktop for its sandbox
// GetSecurityInfo will fail when chrome tries to do a DACL read on the default WindowStation.
// To pass this security check sandboxie needs to use the handle to the Sandboxie Dummy WindowStation
// this will allow chrome to create the required WindowStation and Desktop.  See comment in 
// GuiServer.cpp: GuiServer::GetWindowStationAndDesktopName.

_FX DWORD AdvApi_GetSecurityInfo(
    HANDLE handle,
    SE_OBJECT_TYPE ObjectType,
    SECURITY_INFORMATION SecurityInfo,
    PSID psidOwner,
    PSID psidGroup,
    PACL pDacl,
    PACL pSacl,
    PSECURITY_DESCRIPTOR *ppSecurityDescriptor)
{
    DWORD rc = 0;
    rc = __sys_GetSecurityInfo(handle, ObjectType, SecurityInfo, psidOwner, psidGroup, pDacl, pSacl, ppSecurityDescriptor);

    if (rc && ObjectType == SE_WINDOW_OBJECT && SecurityInfo == DACL_SECURITY_INFORMATION) {
        __sys_EnumWindowStationsW = (P_EnumWindowStations)Ldr_GetProcAddrNew(L"User32.dll", L"EnumWindowStationsW", "EnumWindowStationsW");
        __sys_OpenWindowStationW = (P_OpenWindowStationW)Ldr_GetProcAddrNew(L"User32.dll", L"OpenWindowStationW", "OpenWindowStationW");
        if (!Sandboxie_WinSta) {
            if (__sys_EnumWindowStationsW) {
                rc = __sys_EnumWindowStationsW(myEnumWindowStationProc, 0);
            }
        }
        rc = __sys_GetSecurityInfo(Sandboxie_WinSta, ObjectType, SecurityInfo, psidOwner, psidGroup, pDacl, pSacl, ppSecurityDescriptor);
    }
    return rc;
}


//---------------------------------------------------------------------------
// AdvApi_SetSecurityInfo
//---------------------------------------------------------------------------

_FX DWORD AdvApi_SetSecurityInfo(
    HANDLE handle,
    SE_OBJECT_TYPE ObjectType,
    SECURITY_INFORMATION SecurityInfo,
    PSID psidOwner,
    PSID psidGroup,
    PACL pDacl,
    PACL pSacl)
{
    // this is a HACK to get Chrome 38 to work
    if ((Dll_ImageType == DLL_IMAGE_GOOGLE_CHROME) && (ObjectType == SE_WINDOW_OBJECT) && (handle == NULL))
        return 0;

    return __sys_SetSecurityInfo(handle, ObjectType, SecurityInfo, psidOwner, psidGroup, pDacl, pSacl);
}


//---------------------------------------------------------------------------
// AdvApi_AccessCheckByType
//---------------------------------------------------------------------------


_FX BOOL AdvApi_AccessCheckByType(
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    PSID PrincipalSelfSid,
    HANDLE ClientToken,
    DWORD DesiredAccess,
    POBJECT_TYPE_LIST ObjectTypeList,
    DWORD ObjectTypeListLength,
    PGENERIC_MAPPING GenericMapping,
    PPRIVILEGE_SET PrivilegeSet,
    LPDWORD PrivilegeSetLength,
    LPDWORD GrantedAccess,
    LPBOOL AccessStatus)
{
    *GrantedAccess = 0xFFFFFFFF;
    *AccessStatus = TRUE;
    SetLastError(0);
    return TRUE;
}


//---------------------------------------------------------------------------
// AdvApi_SaferComputeTokenFromLevel
//---------------------------------------------------------------------------


_FX BOOL AdvApi_SaferComputeTokenFromLevel(
    ULONG_PTR LevelHandle, HANDLE InAccessToken, HANDLE *OutAccessToken,
    ULONG dwFlags, void *lpReserved)
{
    if (! AdvApi_EnableSRP) {
        const ULONG SAFER_TOKEN_NULL_IF_EQUAL = 1;
        if (dwFlags & SAFER_TOKEN_NULL_IF_EQUAL) {

            *OutAccessToken = NULL;
            SetLastError(ERROR_SUCCESS);
            return TRUE;
        }
    }

    return __sys_SaferComputeTokenFromLevel(
        LevelHandle, InAccessToken, OutAccessToken, dwFlags, lpReserved);
}


//---------------------------------------------------------------------------
// AdvApi_EnableDisableSRP
//---------------------------------------------------------------------------


_FX BOOLEAN AdvApi_EnableDisableSRP(BOOLEAN Enable)
{
    //
    // CreateProcess uses SaferComputeTokenFromLevel to check SRP/AppLocker
    // rules.  it passes the SAFER_TOKEN_NULL_IF_EQUAL flag and if
    // SaferComputeTokenFromLevel still returns a token, the new process
    // is aborted.  we hook SaferComputeTokenFromLevel so it can't interfere
    // with SbieDll_RunFromHome
    //

    if (! AdvApi_Module)
        return FALSE;
    if (! __sys_SaferComputeTokenFromLevel) {
        P_SaferComputeTokenFromLevel SaferComputeTokenFromLevel =
            (P_SaferComputeTokenFromLevel)GetProcAddress(
                AdvApi_Module, "SaferComputeTokenFromLevel");
        if (SaferComputeTokenFromLevel) {
            SBIEDLL_HOOK(AdvApi_,SaferComputeTokenFromLevel);
        }
    }
    if (! __sys_SaferComputeTokenFromLevel)
        return FALSE;
    AdvApi_EnableSRP = Enable;
    return TRUE;
}


//---------------------------------------------------------------------------
// AdvApi_GetEffectiveRightsFromAclW
//---------------------------------------------------------------------------


_FX ULONG AdvApi_GetEffectiveRightsFromAclW(
    PACL pacl, void *pTrustee, PACCESS_MASK pAccessRights)
{
    //
    // some programs (e.g HP printer software intaller) use
    // GetEffectiveRightsFromAcl to confirm that an object (like a
    // registry key) was created with appropriate rights, so fake
    // a return value that always shows full access
    //

    ULONG rc =
        __sys_GetEffectiveRightsFromAclW(pacl, pTrustee, pAccessRights);
    if (rc == 0 && pAccessRights && *pAccessRights == 0) {
        *pAccessRights = STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL;
        SbieApi_Log(2205, L"GetEffectiveRightsFromAcl");
    }
    return rc;
}


//---------------------------------------------------------------------------
// Ntmarta_Init
//---------------------------------------------------------------------------
DWORD Ntmarta_GetSecurityInfo(
    HANDLE handle,
    SE_OBJECT_TYPE ObjectType,
    SECURITY_INFORMATION SecurityInfo,
    PSID psidOwner,
    PSID psidGroup,
    PACL pDacl,
    PACL pSacl,
    PSECURITY_DESCRIPTOR *ppSecurityDescriptor);


#define SBIEDLL_HOOK2(pfx,proc)                  \
    *(ULONG_PTR *)&__sys_##pfx##proc = (ULONG_PTR)   \
    SbieDll_Hook(#proc, proc, pfx##proc);   \
    if (! __sys_##pfx##proc) return FALSE;

_FX BOOLEAN Ntmarta_Init(HMODULE module)
{
#ifdef _WIN64
    P_SetSecurityInfo SetSecurityInfo;
#endif
    P_GetSecurityInfo GetSecurityInfo;
#define GETPROC2(x,s) __sys_Ntmarta_##x##s = (P_##x) Ldr_GetProcAddrNew(DllName_ntmarta, L#x L#s,#x #s);

    GETPROC2(GetSecurityInfo, );
    if ((Dll_ImageType == DLL_IMAGE_GOOGLE_CHROME) || (Dll_ImageType == DLL_IMAGE_ACROBAT_READER)) {

        GetSecurityInfo = __sys_Ntmarta_GetSecurityInfo;
        if (GetSecurityInfo)
        {
            // this hook conflicts with the AdvApi32 hook and causes infinite callbacks if delay loading.
#ifndef _WIN64
            if (Dll_ImageType == DLL_IMAGE_ACROBAT_READER) {
                //This hook is need for Adobe Acrobat version 2019.010.x to allow
                //the creation of a desktop with the sandboxie restricted token.
                //See Gui_CreateDesktopW in guienum.c

                //Due to the risk of the stack overflow issue limited this hook for
                //Acrobat Reader in 32 bit only
                SBIEDLL_HOOK2(Ntmarta_, GetSecurityInfo);
            }
#endif
            __sys_GetSecurityInfo = GetSecurityInfo;
        }
    }
#ifdef _WIN64
    GETPROC2(SetSecurityInfo, );

    // only need to hook if Advapi32!SetSecurityInfo can't be resolved
    if (Dll_ImageType == DLL_IMAGE_GOOGLE_CHROME) {

        SetSecurityInfo = __sys_Ntmarta_SetSecurityInfo;
        if (SetSecurityInfo)
        {
            // this hook conflicts with the AdvApi32 hook and causes infinite callbacks if delay loading.
            //SBIEDLL_HOOK2(Ntmarta_,SetSecurityInfo);
            __sys_SetSecurityInfo = SetSecurityInfo;
        }
    }

#endif _WIN64
    return TRUE;
}

_FX DWORD Ntmarta_GetSecurityInfo(
    HANDLE handle,
    SE_OBJECT_TYPE ObjectType,
    SECURITY_INFORMATION SecurityInfo,
    PSID psidOwner,
    PSID psidGroup,
    PACL pDacl,
    PACL pSacl,
    PSECURITY_DESCRIPTOR *ppSecurityDescriptor)
{
    DWORD rc = 0;
    rc = __sys_Ntmarta_GetSecurityInfo(handle, ObjectType, SecurityInfo, psidOwner, psidGroup, pDacl, pSacl, ppSecurityDescriptor);

    if (rc && ObjectType == SE_WINDOW_OBJECT && SecurityInfo == DACL_SECURITY_INFORMATION) {
        __sys_EnumWindowStationsW = (P_EnumWindowStations)Ldr_GetProcAddrNew(L"User32.dll", L"EnumWindowStationsW", "EnumWindowStationsW");
        __sys_OpenWindowStationW = (P_OpenWindowStationW)Ldr_GetProcAddrNew(L"User32.dll", L"OpenWindowStationW", "OpenWindowStationW");
        if (!Sandboxie_WinSta) {
            if (__sys_EnumWindowStationsW) {
                rc = __sys_EnumWindowStationsW(myEnumWindowStationProc, 0);
            }
        }
        rc = __sys_Ntmarta_GetSecurityInfo(Sandboxie_WinSta, ObjectType, SecurityInfo, psidOwner, psidGroup, pDacl, pSacl, ppSecurityDescriptor);
    }
    return rc;
}

#ifdef _WIN64
// Same as AdvApi_SetSecurityInfo
_FX DWORD Ntmarta_SetSecurityInfo(
    HANDLE handle,
    SE_OBJECT_TYPE ObjectType,
    SECURITY_INFORMATION SecurityInfo,
    PSID psidOwner,
    PSID psidGroup,
    PACL pDacl,
    PACL pSacl)
{
    // this is a HACK to get Chrome 38 to work.
    if ((Dll_ImageType == DLL_IMAGE_GOOGLE_CHROME) && (ObjectType == SE_WINDOW_OBJECT) && (handle == NULL))
        return 0;

    return __sys_Ntmarta_SetSecurityInfo(handle, ObjectType, SecurityInfo, psidOwner, psidGroup, pDacl, pSacl);
}

#endif _WIN64
