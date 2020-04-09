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
// AdvApi32 Private Include
//---------------------------------------------------------------------------


#ifndef _MY_ADVAPI_H
#define _MY_ADVAPI_H

#include <AclAPI.h>

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------


typedef LONG (*P_RegOpenKeyEx)(
    HKEY hKey, void *lpSubKey, DWORD ulOptions,
    REGSAM samDesired, HKEY *phkResult);

typedef LONG (*P_RegCloseKey)(
    HKEY hKey);

typedef LONG (*P_RegGetKeySecurity)(
    HKEY hKey,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    LPDWORD lpcbSecurityDescriptor);

typedef LONG (*P_RegSetKeySecurity)(
    HKEY hKey,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor);

typedef BOOL (*P_GetFileSecurity)(
    void *FileName,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    DWORD nLength,
    LPDWORD lpnLengthNeeded);

typedef BOOL (*P_SetFileSecurity)(
    void *FileName,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor);

typedef BOOL (*P_CreateProcessAsUser)(
    HANDLE hToken,
    LPVOID lpApplicationName,
    LPVOID lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPVOID lpCurrentDirectory,
    LPVOID lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation);

typedef BOOL (*P_LookupAccountName)(
    void *lpSystemName,
    void *lpAccountName,
    PSID Sid,
    LPDWORD cbSid,
    void *ReferencedDomainName,
    LPDWORD cchReferencedDomainName,
    PSID_NAME_USE peUse);

typedef BOOL (*P_LookupAccountName)(
    void *lpSystemName,
    void *lpAccountName,
    PSID Sid,
    LPDWORD cbSid,
    void *ReferencedDomainName,
    LPDWORD cchReferencedDomainName,
    PSID_NAME_USE peUse);

typedef BOOL (*P_LookupPrivilegeValue)(
    void *lpSystemName,
    void *lpName,
    PLUID lpLuid);

typedef ULONG (*P_RegConnectRegistry)(
    void *lpMachineName, HKEY hKey, HKEY *phkResult);

typedef BOOL (*P_CryptVerifySignature)(
    void *hHash,
    BYTE *pbSignature,
    DWORD dwSigLen,
    void *hPubKey,
    void *sDescription,
    DWORD dwFlags);

typedef BOOL(*P_CreateRestrictedToken)(
    HANDLE ExistingTokenHandle,
    DWORD Flags,
    DWORD DisableSidCount,
    PSID_AND_ATTRIBUTES SidsToDisable,
    DWORD DeletePrivilegeCount,
    PLUID_AND_ATTRIBUTES PrivilegesToDelete,
    DWORD RestrictedSidCount,
    PSID_AND_ATTRIBUTES SidsToRestrict,
    PHANDLE NewTokenHandle);

typedef DWORD(*P_SetSecurityInfo)(
    HANDLE handle,
    SE_OBJECT_TYPE ObjectType,
    SECURITY_INFORMATION SecurityInfo,
    PSID psidOwner,
    PSID psidGroup,
    PACL pDacl,
    PACL pSacl);

typedef DWORD(*P_GetSecurityInfo)(
    HANDLE handle,
    SE_OBJECT_TYPE ObjectType,
    SECURITY_INFORMATION SecurityInfo,
    PSID psidOwner,
    PSID psidGroup,
    PACL pDacl,
    PACL pSacl,
    PSECURITY_DESCRIPTOR *ppSecurityDescriptor);


//---------------------------------------------------------------------------
// Function Pointers
//---------------------------------------------------------------------------


extern P_RegOpenKeyEx           __sys_RegOpenKeyExA;
extern P_RegOpenKeyEx           __sys_RegOpenKeyExW;

extern P_RegCloseKey            __sys_RegCloseKey;

extern P_RegGetKeySecurity      __sys_RegGetKeySecurity;

extern P_RegSetKeySecurity      __sys_RegSetKeySecurity;

extern P_GetFileSecurity        __sys_GetFileSecurityW;
extern P_GetFileSecurity        __sys_GetFileSecurityA;

extern P_SetFileSecurity        __sys_SetFileSecurityW;
extern P_SetFileSecurity        __sys_SetFileSecurityA;

extern P_CreateProcessAsUser    __sys_CreateProcessAsUserW;
extern P_CreateProcessAsUser    __sys_CreateProcessAsUserA;

extern P_LookupAccountName      __sys_LookupAccountNameW;

extern P_LookupPrivilegeValue   __sys_LookupPrivilegeValueW;

extern P_RegConnectRegistry     __sys_RegConnectRegistryW;
extern P_RegConnectRegistry     __sys_RegConnectRegistryA;

extern P_CreateRestrictedToken  __sys_CreateRestrictedToken;
extern P_SetSecurityInfo        __sys_SetSecurityInfo;


//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Prototypes
//---------------------------------------------------------------------------


typedef BOOL (*P_CredWrite)(void *pCredential, ULONG Flags);

typedef BOOL(*P_CredRead)(
    void *TargetName, ULONG Type, ULONG Flags, void *pCredential);

typedef BOOL(*P_CredWriteDomainCredentials)(
    void *pTargetInfo, void *pCredential, ULONG Flags);

typedef BOOL(*P_CredReadDomainCredentials)(
    void *TargetInfo, ULONG Flags, ULONG *pCount, void ***ppCredentials);

typedef BOOL(*P_CredGetTargetInfo)(
    void *pTargetName, ULONG Flags, void **pTargetInfo);

typedef BOOL(*P_CredRename)(
    void *OldTargetName, void *NewTargetName, ULONG Type, ULONG Flags);

typedef BOOL(*P_CredDelete)(void *TargetName, ULONG Type, ULONG Flags);

typedef BOOL(*P_CredEnumerate)(
    void *pFilter, ULONG Flags, ULONG *Count, void *ppCredential);

//---------------------------------------------------------------------------
// Pointers
//---------------------------------------------------------------------------


extern P_CredWrite              __sys_CredWriteA;
extern P_CredWrite              __sys_CredWriteW;

extern P_CredRead               __sys_CredReadA ;
extern P_CredRead               __sys_CredReadW ;

extern P_CredWriteDomainCredentials __sys_CredWriteDomainCredentialsW;
extern P_CredWriteDomainCredentials __sys_CredWriteDomainCredentialsA;

extern P_CredReadDomainCredentials __sys_CredReadDomainCredentialsW;
extern P_CredReadDomainCredentials __sys_CredReadDomainCredentialsA;

//extern P_CredGetTargetInfo      __sys_CredGetTargetInfoA;
//extern P_CredGetTargetInfo      __sys_CredGetTargetInfoW;

extern P_CredRename             __sys_CredRenameA;
extern P_CredRename             __sys_CredRenameW;

extern P_CredDelete             __sys_CredDeleteA;
extern P_CredDelete             __sys_CredDeleteW;

extern P_CredEnumerate          __sys_CredEnumerateA;
extern P_CredEnumerate          __sys_CredEnumerateW;

#endif /* _MY_ADVAPI_H */
