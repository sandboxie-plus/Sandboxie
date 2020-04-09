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
// Service handling code, common to RpcSs and DcomLauncher
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define SE_ASSIGNPRIMARYTOKEN_PRIVILEGE     (3L)


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


DWORD my_RtlAdjustPrivilege(
    DWORD dwPrivilege,
    BOOL bEnablePrivilege,
    DWORD dwAdjustWhat,
    DWORD *dwPreviouslyEnabled);

BOOL my_AccessCheckByType(
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


//---------------------------------------------------------------------------


__declspec(dllimport) ULONG RtlAdjustPrivilege(
    DWORD dwPrivilege,
    BOOL bEnablePrivilege,
    DWORD dwAdjustWhat,
    DWORD *dwPreviouslyEnabled);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static ULONG ThreadTokenTlsIndex = TLS_OUT_OF_INDEXES;


static ULONG_PTR __sys_RtlAdjustPrivilege                       = 0;
static ULONG_PTR __sys_AccessCheckByType                        = 0;
static ULONG_PTR __sys_SetThreadToken                           = 0;
static ULONG_PTR __sys_GetTokenInformation                      = 0;


//---------------------------------------------------------------------------
// my_RtlAdjustPrivilege
//---------------------------------------------------------------------------


ALIGNED DWORD my_RtlAdjustPrivilege(
    DWORD dwPrivilege,
    BOOL bEnablePrivilege,
    DWORD dwAdjustWhat,
    DWORD *dwPreviouslyEnabled)
{
    typedef DWORD (*P_RtlAdjustPrivilege)(
        DWORD dwPrivilege,
        BOOL bEnablePrivilege,
        DWORD dwAdjustWhat,
        DWORD *dwPreviouslyEnabled);

    DWORD rv = ((P_RtlAdjustPrivilege)__sys_RtlAdjustPrivilege)(
        dwPrivilege, bEnablePrivilege, dwAdjustWhat, dwPreviouslyEnabled);

    if (rv == STATUS_PRIVILEGE_NOT_HELD &&
        dwPrivilege == SE_ASSIGNPRIMARYTOKEN_PRIVILEGE)
        rv = 0;

    return rv;
}

//---------------------------------------------------------------------------
// my_AccessCheckByType
//---------------------------------------------------------------------------


ALIGNED BOOL my_AccessCheckByType(
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
// my_SetThreadToken
//---------------------------------------------------------------------------


ALIGNED BOOL my_SetThreadToken(PHANDLE Thread, HANDLE Token)
{
    typedef BOOL (*P_SetThreadToken)(PHANDLE Thread, HANDLE Token);
    BOOL ok;

    TlsSetValue(ThreadTokenTlsIndex, NULL);

    ok = ((P_SetThreadToken)__sys_SetThreadToken)(Thread, Token);

    if ((! ok) && GetLastError() == ERROR_ACCESS_DENIED) {

        //
        // RpcSs and DcomLaunch typically run as a normal user account
        // that does not have the the impersonation privilege.
        // this means that if some other services (say EventSystem)
        // runs as LocalSystem, RpcSs is going to have trouble
        // impersonating that other service.
        // to work around that, we do impersonate per the caler's
        // request, but we impersonate ourselves
        //
        // and also:
        //
        // CryptSvc indirectly calls rsaenh!CPAcquireContext, which checks
        // if it is running as LocalSystem and fails otherwise.  the check
        // begins with a call to SetThreadToken(NULL, hTokenLocalSystem),
        // follows with GetTokenInformation and concludes by comparing the
        // SID. we fake success by impersonating our process token, but
        // remember hTokenLocalSystem for GetTokenInformation (see below).
        //

        if (Thread == NULL) {

            HANDLE PriToken;
            ok = OpenProcessToken(
                            NtCurrentProcess(), TOKEN_ALL_ACCESS, &PriToken);
            if (ok) {

                HANDLE ImpToken;
                ok = DuplicateToken(
                                PriToken, SecurityImpersonation, &ImpToken);
                CloseHandle(PriToken);

                if (ok) {

                    ok = ((P_SetThreadToken)__sys_SetThreadToken)(
                                                        Thread, ImpToken);
                    CloseHandle(ImpToken);

                    if (ok)
                        TlsSetValue(ThreadTokenTlsIndex, Token);
                }
            }

            if (ok)
                SetLastError(ERROR_SUCCESS);
            else
                SetLastError(ERROR_ACCESS_DENIED);
        }
    }

    return ok;
}


//---------------------------------------------------------------------------
// my_GetTokenInformation
//---------------------------------------------------------------------------


ALIGNED BOOL my_GetTokenInformation(
    HANDLE TokenHandle,
    TOKEN_INFORMATION_CLASS TokenInformationClass,
    LPVOID TokenInformation,
    DWORD TokenInformationLength,
    PDWORD ReturnLength)
{
    typedef BOOL (*P_GetTokenInformation)(
        HANDLE TokenHandle,
        TOKEN_INFORMATION_CLASS TokenInformationClass,
        LPVOID TokenInformation,
        DWORD TokenInformationLength,
        PDWORD ReturnLength);

    //
    // if our faked SetThreadToken saved hTokenLocalSystem,
    // then use that saved token.
    //

    HANDLE SavedToken = TlsGetValue(ThreadTokenTlsIndex);
    if (SavedToken)
        TokenHandle = SavedToken;

    return ((P_GetTokenInformation)__sys_GetTokenInformation)(
        TokenHandle, TokenInformationClass, TokenInformation,
        TokenInformationLength, ReturnLength);
}


//---------------------------------------------------------------------------
// Hook_Privilege
//---------------------------------------------------------------------------


ALIGNED BOOL Hook_Privilege(void)
{
    BOOL hook_success = TRUE;

#ifndef SANDBOXIECRYPTO
    HOOK_WIN32(RtlAdjustPrivilege);
#endif
    HOOK_WIN32(AccessCheckByType);
    HOOK_WIN32(SetThreadToken);
    HOOK_WIN32(GetTokenInformation);

    if (hook_success) {

        ThreadTokenTlsIndex = TlsAlloc();

    } else {

        ErrorMessageBox(L"Could not hook privilege adjustment services");
        return FALSE;
    }

    return TRUE;
}














//---------------------------------------------------------------------------
// SetPrivilege
//---------------------------------------------------------------------------


#if 0


BOOL SetPrivilege(
    HANDLE hToken,
    LPCTSTR lpszPrivilege,
    BOOL bEnablePrivilege)
{
TOKEN_PRIVILEGES tp;
LUID luid;

if ( !LookupPrivilegeValue(
        NULL,            // lookup privilege on local system
        lpszPrivilege,   // privilege to lookup
        &luid ) )        // receives LUID of privilege
{
    printf("LookupPrivilegeValue error: %u\n", GetLastError() );
    return FALSE;
}

tp.PrivilegeCount = 1;
tp.Privileges[0].Luid = luid;
if (bEnablePrivilege)
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
else
    tp.Privileges[0].Attributes = 0;

// Enable the privilege or disable all privileges.

if ( !AdjustTokenPrivileges(
       hToken,
       FALSE,
       &tp,
       sizeof(TOKEN_PRIVILEGES),
       (PTOKEN_PRIVILEGES) NULL,
       (PDWORD) NULL) )
{
      printf("AdjustTokenPrivileges error: %u\n", GetLastError() );
      return FALSE;
}

if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)

{
      printf("The token does not have the specified privilege. \n");
      return FALSE;
}

return TRUE;
}

//---------------------------------------------------------------------------
// SetPrivilegeProcess
//---------------------------------------------------------------------------

BOOL SetPrivilegeProcess(const WCHAR *PrivName, BOOL Enable)
{
    HANDLE ProcessHandle;
    BOOL ok;

    ok = OpenProcessToken(
        GetCurrentProcess(), TOKEN_ALL_ACCESS, &ProcessHandle);
    if (ok) {
        ok = SetPrivilege(ProcessHandle, PrivName, Enable);
        CloseHandle(ProcessHandle);
    }
    return ok;
}


#endif
