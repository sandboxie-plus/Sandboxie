/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2021 David Xanatos, xanasoft.com
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
// Tokens
//---------------------------------------------------------------------------


#include "token.h"
#include "process.h"
#include "conf.h"
#include "api.h"
#include "util.h"
#include "common/my_version.h"
#include "session.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

NTSTATUS Sbie_SeFilterToken_KernelMode(
    IN PACCESS_TOKEN  ExistingToken,
    IN ULONG  Flags,
    IN PTOKEN_GROUPS  SidsToDisable  OPTIONAL,
    IN PTOKEN_PRIVILEGES  PrivilegesToDelete  OPTIONAL,
    IN PTOKEN_GROUPS  RestrictedSids  OPTIONAL,
    OUT PACCESS_TOKEN  *NewToken
);

NTSTATUS Sbie_SepFilterToken_KernelMode(
    void*       TokenObject,
    ULONG_PTR   SidCount,
    ULONG_PTR   SidPtr,
    ULONG_PTR   LengthIncrease,
    void        **NewToken
);

static BOOLEAN Token_Init_SepFilterToken(void);

static void *Token_FilterPrimary(PROCESS *proc, void *ProcessObject);

static NTSTATUS Token_FilterDacl(void *TokenObject, ULONG SessionId);

static NTSTATUS Token_SetHandleDacl(HANDLE Handle, ACL *Dacl);

static void *Token_RestrictHelper1(
    void *TokenObject, ULONG *OutIntegrityLevel, PROCESS *proc);

static NTSTATUS Token_RestrictHelper2(
    void *TokenObject, ULONG *OutIntegrityLevel, PROCESS *proc);

static void *Token_RestrictHelper3(
    void *TokenObject, TOKEN_GROUPS *Groups, TOKEN_PRIVILEGES *Privileges,
    PSID UserSid, ULONG FilterFlags, ULONG SessionId);

static BOOLEAN Token_AssignPrimary(
    void *ProcessObject, void *TokenObject, ULONG SessionId);


//---------------------------------------------------------------------------


typedef NTSTATUS(*P_SepFilterToken)(
    void *TokenObject,
    ULONG_PTR Zero2,
    ULONG_PTR Zero3,
    ULONG_PTR Zero4,
    ULONG_PTR Zero5,
    ULONG_PTR Zero6,
    ULONG_PTR Zero7,
    ULONG_PTR RestrictedSidCount,
    ULONG_PTR RestrictedSidPtr,
    ULONG_PTR VariableLengthIncrease,
    void **NewTokenObject);

typedef NTSTATUS(__fastcall *P_SepFilterToken_W81)(
    void *TokenObject,
    ULONG_PTR Zero2,
    ULONG_PTR Zero3,
    ULONG_PTR Zero4,
    ULONG_PTR Zero5,
    ULONG_PTR Zero6,
    ULONG_PTR Zero7,
    ULONG_PTR RestrictedSidCount,
    ULONG_PTR RestrictedSidPtr,
    ULONG_PTR VariableLengthIncrease,
    void **NewTokenObject);

P_SepFilterToken Token_SepFilterToken = NULL;
//---------------------------------------------------------------------------


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, Token_Init)
#pragma alloc_text (INIT, Token_Init_SepFilterToken)
#endif


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static TOKEN_PRIVILEGES *Token_FilterPrivileges = NULL;
static TOKEN_GROUPS     *Token_FilterGroups = NULL;

static UCHAR Token_AdministratorsSid[16] = {
    1,                                      // Revision
    2,                                      // SubAuthorityCount
    0,0,0,0,0,5, // SECURITY_NT_AUTHORITY   // IdentifierAuthority
    SECURITY_BUILTIN_DOMAIN_RID,0,0,0,      // SubAuthority 1
    (DOMAIN_ALIAS_RID_ADMINS & 0xFF),       // SubAuthority 2
        ((DOMAIN_ALIAS_RID_ADMINS & 0xFF00) >> 8),0,0
};

static UCHAR Token_PowerUsersSid[16] = {
    1,                                      // Revision
    2,                                      // SubAuthorityCount
    0,0,0,0,0,5, // SECURITY_NT_AUTHORITY   // IdentifierAuthority
    SECURITY_BUILTIN_DOMAIN_RID,0,0,0,      // SubAuthority 1
    (DOMAIN_ALIAS_RID_POWER_USERS & 0xFF),  // SubAuthority 2
        ((DOMAIN_ALIAS_RID_POWER_USERS & 0xFF00) >> 8),0,0
};

static UCHAR AnonymousLogonSid[12] = {
    1,                                      // Revision
    1,                                      // SubAuthorityCount
    0,0,0,0,0,5, // SECURITY_NT_AUTHORITY   // IdentifierAuthority
    SECURITY_ANONYMOUS_LOGON_RID,0,0,0      // SubAuthority
};

static UCHAR SandboxieLogonSid[SECURITY_MAX_SID_SIZE] = { 0 }; // SbieLogin

static UCHAR SystemLogonSid[12] = {
	1,                                      // Revision
	1,                                      // SubAuthorityCount
	0,0,0,0,0,5, // SECURITY_NT_AUTHORITY   // IdentifierAuthority
	SECURITY_LOCAL_SYSTEM_RID,0,0,0         // SubAuthority
};

UCHAR Sbie_Token_SourceName[5] = { 's', 'b', 'o', 'x', 0 };

#define ProcessMitigationPolicy 52
//---------------------------------------------------------------------------
// Token_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Token_Init(void)
{
    const ULONG NumBasePrivs = 6;
    const ULONG NumVistaPrivs = 1;

    //
    // create a list of privileges to filter
    //

    ULONG len = (NumBasePrivs + NumVistaPrivs) * sizeof(LUID_AND_ATTRIBUTES)
        + 32; // for TOKEN_PRIVILEGES.PrivilegeCount, and spare room

    Token_FilterPrivileges = Mem_AllocEx(Driver_Pool, len, TRUE);
    if (!Token_FilterPrivileges)
        return FALSE;
    memzero(Token_FilterPrivileges, len);

    Token_FilterPrivileges->PrivilegeCount = NumBasePrivs;
    if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA)
        Token_FilterPrivileges->PrivilegeCount += NumVistaPrivs;

#define MySetPrivilege(i) Token_FilterPrivileges->Privileges[i].Luid.LowPart

    MySetPrivilege(0) = SE_RESTORE_PRIVILEGE;
    MySetPrivilege(1) = SE_BACKUP_PRIVILEGE;
    MySetPrivilege(2) = SE_LOAD_DRIVER_PRIVILEGE;
    MySetPrivilege(3) = SE_SHUTDOWN_PRIVILEGE;
    MySetPrivilege(4) = SE_DEBUG_PRIVILEGE;
    MySetPrivilege(5) = SE_SYSTEMTIME_PRIVILEGE;
    MySetPrivilege(6) = SE_TIME_ZONE_PRIVILEGE; // vista

#undef MySetPrivilege

    //
    // create a list of groups to filter when Drop Rights is in use
    //

    len = 2 * sizeof(SID_AND_ATTRIBUTES)
        + 32; // for TOKEN_GROUPS.GroupCount, and spare room

    Token_FilterGroups = Mem_AllocEx(Driver_Pool, len, TRUE);
    if (!Token_FilterGroups)
        return FALSE;
    memzero(Token_FilterGroups, len);

    Token_FilterGroups->GroupCount = 2;

#define MySetGroup(i) Token_FilterGroups->Groups[i].Sid

    MySetGroup(0) = Token_AdministratorsSid;
    MySetGroup(1) = Token_PowerUsersSid;

#undef MySetGroup

	//
	// find the sid of the sandboxie user if present
	//

	// SbieLogin BEGIN
	if (Conf_Get_Boolean(NULL, L"AllowSandboxieLogon", 0, FALSE))
	{
		WCHAR AccountBuffer[64]; // DNLEN + 1 + sizeof(SANDBOXIE_USER) + reserve
		UNICODE_STRING AccountName = { 0, sizeof(AccountBuffer), AccountBuffer }; // Note: max valid length is (DNLEN (15) + 1) * sizeof(WCHAR), length is in bytes leave half empty
		if (GetRegString(RTL_REGISTRY_ABSOLUTE, L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ActiveComputerName", L"ComputerName", &AccountName) && AccountName.Length < 64)
		{
			wcscpy(AccountName.Buffer + (AccountName.Length / sizeof(WCHAR)), L"\\" SANDBOXIE_USER);
			AccountName.Length += (1 + wcslen(SANDBOXIE_USER)) * sizeof(WCHAR);
			//DbgPrint("Sbie, AccountName: %S\n", AccountName.Buffer);

			SID_NAME_USE use;
			ULONG userSize = sizeof(SandboxieLogonSid), domainSize = 0;
			WCHAR DomainBuff[20]; // doesn't work without this
			UNICODE_STRING DomainName = { 0, sizeof(DomainBuff), DomainBuff };

			SecLookupAccountName(&AccountName, &userSize, (PSID)SandboxieLogonSid, &use, &domainSize, &DomainName);
			//DbgPrint("Sbie, SecLookupAccountName: %x; size:%d %d\n", status, userSize, domainSize);
		}
	}
	// SbieLogin END

    //
    // find SepFilterToken for Token_RestrictHelper1
    //

    if (!Token_Init_SepFilterToken())
        return FALSE;

    //
    // finish
    //

    return TRUE;
}


//---------------------------------------------------------------------------
// Token_Init_SepFilterToken
//---------------------------------------------------------------------------


_FX BOOLEAN Token_Init_SepFilterToken(void)
{
    UNICODE_STRING uni;
    UCHAR *ptr;

    //
    // nt!SeFilterToken is a wrapper around unexported nt!SepFilterToken.
    // in function Token_RestrictHelper1 we need the underlying unexported
    // function, so analyze nt!SeFilterToken to determine its address
    //

    RtlInitUnicodeString(&uni, L"SeFilterToken");
    ptr = MmGetSystemRoutineAddress(&uni);
    if (ptr) {

        ULONG i;
        for (i = 0; i < 256; ++i) {

#ifdef _WIN64

            //
            // 64-bit: look for "and dword ptr [rsp+48h],0"
            // later followed by "call nt!SepFilterToken"
            //

            if (*(ULONG *)ptr == 0x48246483 && ptr[4] == 0) {

                for (; i < 256; ++i) {

                    if (*ptr == 0xE8) {

                        ptr += *(LONG *)(ptr + 1) + 5;
                        Token_SepFilterToken = (P_SepFilterToken)ptr;

                        break;
                    }

                    ++ptr;
                }
            }

#else ! _WIN64

            //
            // 32-bit: look for "push dword ptr [ebp+8]"
            // or "push dword ptr [ebp+0C]"
            // immediately followed by "call nt!SepFilterToken"
            //

            if (*(ULONG *)ptr == 0xE80875FF
                || *(ULONG *)ptr == 0xE80C75FF) {

                ptr += *(LONG *)(ptr + 4) + 8;
                Token_SepFilterToken = (P_SepFilterToken)ptr;

                break;
            }

#endif _WIN64

            ++ptr;
        }
    }

    if (!Token_SepFilterToken) {
        Log_Msg1(MSG_1108, uni.Buffer);
        return FALSE;
    }
    return TRUE;
}


//---------------------------------------------------------------------------
// Token_Query
//---------------------------------------------------------------------------


_FX void *Token_Query(
    void *TokenObject, TOKEN_INFORMATION_CLASS InfoClass, ULONG SessionId)
{
    void *ReturnValue;
    NTSTATUS status;

    status = SeQueryInformationToken(TokenObject, InfoClass, &ReturnValue);
    if (!NT_SUCCESS(status)) {

        ReturnValue = NULL;

        Log_Status_Ex_Session(MSG_1222, 0x11, status, NULL, SessionId);
    }

    return ReturnValue;
}


//---------------------------------------------------------------------------
// Token_QuerySidString
//---------------------------------------------------------------------------


_FX NTSTATUS Token_QuerySidString(
    void *TokenObject, UNICODE_STRING *SidString)
{
    TOKEN_USER *TokenUserData = NULL;
    NTSTATUS status = SeQueryInformationToken(
        TokenObject, TokenUser, &TokenUserData);
    if (NT_SUCCESS(status)) {

        status = RtlConvertSidToUnicodeString(
            SidString, TokenUserData->User.Sid, TRUE);

        ExFreePool(TokenUserData);
    }

    return status;
}


//---------------------------------------------------------------------------
// Token_QueryPrimary
//---------------------------------------------------------------------------


_FX BOOLEAN Token_CheckPrivilege(
    void *TokenObject, ULONG PrivilegeLowPart, ULONG SessionId)
{
    BOOLEAN HavePrivilege = FALSE;

    TOKEN_PRIVILEGES *privs =
        Token_Query(TokenObject, TokenPrivileges, SessionId);
    if (privs) {

        ULONG i;

        LUID PrivilegeToCheck;
        PrivilegeToCheck.LowPart = PrivilegeLowPart;
        PrivilegeToCheck.HighPart = 0;

        for (i = 0; i < privs->PrivilegeCount; ++i) {
            LUID_AND_ATTRIBUTES *entry = &privs->Privileges[i];
            if (RtlEqualLuid(&entry->Luid, &PrivilegeToCheck)) {

                HavePrivilege = TRUE;
                break;
            }
        }

        ExFreePool(privs);
    }

    return HavePrivilege;
}


//---------------------------------------------------------------------------
// Token_QueryPrimary
//---------------------------------------------------------------------------


_FX void *Token_QueryPrimary(
    TOKEN_INFORMATION_CLASS InfoClass, ULONG SessionId)
{
    void *TokenObject;
    void *ReturnValue;

    TokenObject = PsReferencePrimaryToken(PsGetCurrentProcess());
    if (!TokenObject) {

        ReturnValue = NULL;

        Log_Status_Ex_Session(
            MSG_1222, 0x15, STATUS_UNSUCCESSFUL, NULL, SessionId);

    }
    else {

        ReturnValue = Token_Query(TokenObject, InfoClass, SessionId);

        PsDereferencePrimaryToken(TokenObject);
    }

    return ReturnValue;
}


//---------------------------------------------------------------------------
// Token_FilterPrimary
//---------------------------------------------------------------------------


_FX void *Token_FilterPrimary(PROCESS *proc, void *ProcessObject)
{
    void *PrimaryToken, *ReturnToken;
    ULONG DropRights;

    //
    // open process token and check if filtering is needed
    //

    // DbgPrint("Filtering token for process - %d <%S>\n", proc->pid, proc->image_name);

    PrimaryToken = PsReferencePrimaryToken(ProcessObject);
    if (!PrimaryToken) {

		Log_Status_Ex_Process(MSG_1222, 0x31, STATUS_NO_TOKEN, NULL, proc->box->session_id, proc->pid);
        return NULL;
    }

	// OpenToken BEGIN
	if (Conf_Get_Boolean(proc->box->name, L"OpenToken", 0, FALSE) || Conf_Get_Boolean(proc->box->name, L"UnfilteredToken", 0, FALSE)) {
		return PrimaryToken;
	}
	// OpenToken END

    // DbgPrint("   Process Token %08X - %d <%S>\n", PrimaryToken, proc->pid, proc->image_name);

    proc->drop_rights =
        Conf_Get_Boolean(proc->box->name, L"DropAdminRights", 0, FALSE);

    DropRights = (proc->drop_rights ? -1 : 0);

    //
    // special allowance for MSIServer - does not seam to be needed with the CreateWaitableTimerW hook
    //
    //if (DropRights && !proc->image_from_box && _wcsicmp(proc->image_name, L"msiexec.exe") == 0
    //    && Conf_Get_Boolean(proc->box->name, L"MsiInstallerExemptions", 0, FALSE)) 
    //{
    //    DropRights = 0;
    //}

    // DbgPrint("   Drop rights %d - %d <%S>\n", proc->drop_rights, proc->pid, proc->image_name);

    ReturnToken = Token_Filter(
        PrimaryToken, DropRights, proc->box->session_id);

    PsDereferencePrimaryToken(PrimaryToken);

    return ReturnToken;
}


//---------------------------------------------------------------------------
// Token_Filter
//---------------------------------------------------------------------------


_FX void *Token_Filter(void *TokenObject, ULONG DropRights, ULONG SessionId)
{
    TOKEN_GROUPS *groups;
    void *ReturnToken = NULL;
    NTSTATUS status;
    ULONG i, j;
    BOOLEAN ShouldFilterToken = FALSE;

    if (DropRights) {

        //
        // if Drop Rights is in effect then we have to check if the token
        // is associated with the Administrators or Power Users groups
        //

        groups = Token_Query(TokenObject, TokenGroups, SessionId);
        if (!groups) {
            status = STATUS_UNSUCCESSFUL;
            goto finish;
        }

        for (i = 0; i < groups->GroupCount; ++i) {
            SID_AND_ATTRIBUTES *entry_i = &groups->Groups[i];
            for (j = 0; j < Token_FilterGroups->GroupCount; ++j) {
                SID_AND_ATTRIBUTES *entry_j = &Token_FilterGroups->Groups[j];

                if (RtlEqualSid(entry_i->Sid, entry_j->Sid)) {

                    //
                    // then token includes Administrators or Power Users,
                    // but if it's just as a deny-only group then we don't
                    // necessarily have to filter yet
                    //

                    const ULONG attrs_to_check = entry_i->Attributes &
                        (SE_GROUP_USE_FOR_DENY_ONLY | SE_GROUP_ENABLED);
                    if (attrs_to_check != SE_GROUP_USE_FOR_DENY_ONLY) {

                        ShouldFilterToken = TRUE;
                        break;
                    }
                }
            }

            if (ShouldFilterToken)
                break;
        }

        if (groups)
            ExFreePool(groups);
    }

    //
    // check if the token includes one of the privileges that we don't allow
    //

    if (!ShouldFilterToken) {

        TOKEN_PRIVILEGES *privs =
            Token_Query(TokenObject, TokenPrivileges, SessionId);
        if (!privs) {
            status = STATUS_UNSUCCESSFUL;
            goto finish;
        }

        for (i = 0; i < privs->PrivilegeCount; ++i) {
            LUID_AND_ATTRIBUTES *entry_i = &privs->Privileges[i];
            for (j = 0; j < Token_FilterPrivileges->PrivilegeCount; ++j) {
                LUID_AND_ATTRIBUTES *entry_j =
                    &Token_FilterPrivileges->Privileges[j];

                if (RtlEqualLuid(&entry_i->Luid, &entry_j->Luid)) {

                    ShouldFilterToken = TRUE;
                    break;
                }
            }

            if (ShouldFilterToken)
                break;
        }

        if (privs)
            ExFreePool(privs);
    }

    // DbgPrint("   Should filter %d\n", ShouldFilterToken);

    //
    // filter the token by removing undesired groups and privileges
    //

    if (ShouldFilterToken) {

        //
        // remove undesired privileges from the original token,
        // and if Drop Rights is in effect, also remove some groups
        //

        TOKEN_GROUPS *DisabledGroups;

        if (DropRights)
            DisabledGroups = Token_FilterGroups;
        else
            DisabledGroups = NULL;

        status = Sbie_SeFilterToken_KernelMode(TokenObject, 0, DisabledGroups,
            Token_FilterPrivileges, NULL, &ReturnToken);

        if (NT_SUCCESS(status) && (DropRights == -1)) {

            //
            // a process that is running as Administrator usually lists
            // the Administrators group in DACLs rather than a specific
            // user SID.  if Drop Rights is in effect, we removed the
            // Administrators group, so we now add the user SID instead
            //
            // we want to do this only when coming from Token_FilterPrimary
            // (DropRights == -1) but not Thread_CheckTokenForImpersonation
            //

            status = Token_FilterDacl(ReturnToken, SessionId);

            if (!NT_SUCCESS(status))
                ObReferenceObject(ReturnToken);
        }

        if (!NT_SUCCESS(status))
            ReturnToken = NULL;

    }
    else {

        //
        // if token is already filtered, e.g. a child process was created by
        // an already-filtered token of a parent process, then increment the
        // reference count on the original token and return it
        //

        ObReferenceObject(TokenObject);
        ReturnToken = TokenObject;

        status = STATUS_SUCCESS;
    }

    if (ReturnToken)
    {
        PROCESS *proc;
        // add our token source for identification in minifilter (file_flt.c)
        TOKEN_SOURCE* tokenName = (TOKEN_SOURCE*)ReturnToken;
        memcpy(tokenName->SourceName, Sbie_Token_SourceName, sizeof(Sbie_Token_SourceName));
        // store current PID in the token source
        proc = Process_Find(NULL, NULL);
        if (proc && (proc != PROCESS_TERMINATED))
            tokenName->SourceIdentifier.LowPart = (ULONG)(ULONG_PTR)proc->pid;
    }

    //
    // finish
    //

finish:

    if (!NT_SUCCESS(status)) {

        Log_Status_Ex_Session(MSG_1222, 0x31, status, NULL, SessionId);
    }

    return ReturnToken;
}


//---------------------------------------------------------------------------
// Token_FilterDacl
//---------------------------------------------------------------------------


_FX NTSTATUS Token_FilterDacl(void *TokenObject, ULONG SessionId)
{
    HANDLE TokenHandle = NULL;
    TOKEN_USER *user = NULL;
    TOKEN_DEFAULT_DACL *dacl = NULL;
    TOKEN_DEFAULT_DACL *x_dacl = NULL;
    ACL *dacl_ptr, *x_dacl_ptr;
    ULONG dacl_len, x_dacl_len;
    NTSTATUS status;

    //
    // this function is called from Token_Filter and adds the user SID
    // to various DACLs, to account for the case where those DACLs
    // originally included the Administrators group, and that group was
    // deleted by Token_Filter
    //

    //
    // get the user SID from the token
    //

    user = Token_Query(TokenObject, TokenUser, SessionId);
    if (!user) {
        status = STATUS_INVALID_OWNER;
        goto finish;
    }

    //
    // get the default DACL from the token
    //

    dacl = Token_Query(TokenObject, TokenDefaultDacl, SessionId);
    if (!dacl) {
        status = STATUS_INVALID_SECURITY_DESCR;
        goto finish;
    }

    dacl_ptr = dacl->DefaultDacl;
    dacl_len = dacl_ptr->AclSize;

    //
    // create a new default DACL that grants GENERIC_ALL access to user SID
    //

    x_dacl_len = sizeof(TOKEN_DEFAULT_DACL) + 8 // header and some padding
        + dacl_len + 128;                // old ACL + new SID
    x_dacl = (TOKEN_DEFAULT_DACL *)
        ExAllocatePoolWithTag(PagedPool, x_dacl_len, tzuk);
    if (!x_dacl) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto finish;
    }

    x_dacl_ptr = (ACL *)((ULONG_PTR)x_dacl + sizeof(TOKEN_DEFAULT_DACL));
    x_dacl->DefaultDacl = x_dacl_ptr;

    memcpy(x_dacl_ptr, dacl_ptr, dacl_len);
    x_dacl_ptr->AclSize += 128;
    RtlAddAccessAllowedAce(
        x_dacl_ptr, ACL_REVISION, GENERIC_ALL, user->User.Sid);

    //
    // get a handle to the token and replace its default DACL so new
    // objects are created with a DACL that grants access to the user SID
    //

    status = ObOpenObjectByPointer(TokenObject, OBJ_KERNEL_HANDLE, NULL,
        0, NULL, KernelMode, &TokenHandle);
    if (!NT_SUCCESS(status)) {
        TokenHandle = NULL;
        goto finish;
    }

    status = ZwSetInformationToken(TokenHandle, TokenDefaultDacl,
        x_dacl, x_dacl_len);

    //
    // replace the DACL on the token object itself, as well as the
    // process and thread objects, so the process can open these objects
    //

    if (NT_SUCCESS(status))
        status = Token_SetHandleDacl(TokenHandle, x_dacl_ptr);

    if (NT_SUCCESS(status))
        status = Token_SetHandleDacl(NtCurrentProcess(), x_dacl_ptr);

    if (NT_SUCCESS(status))
        status = Token_SetHandleDacl(NtCurrentThread(), x_dacl_ptr);

    //
    // finish
    //

finish:

    if (x_dacl)
        ExFreePoolWithTag(x_dacl, tzuk);

    if (TokenHandle)
        ZwClose(TokenHandle);

    if (dacl)
        ExFreePool(dacl);

    if (user)
        ExFreePool(user);

    return status;
}


//---------------------------------------------------------------------------
// Token_SetHandleDacl
//---------------------------------------------------------------------------


_FX NTSTATUS Token_SetHandleDacl(HANDLE Handle, ACL *Dacl)
{
    UCHAR sd_space[64];
    PSECURITY_DESCRIPTOR sd = (PSECURITY_DESCRIPTOR)sd_space;

    RtlCreateSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION);
    RtlSetDaclSecurityDescriptor(sd, TRUE, Dacl, FALSE);

    return ZwSetSecurityObject(Handle, DACL_SECURITY_INFORMATION, sd);
}


//---------------------------------------------------------------------------
// Token_Restrict
//---------------------------------------------------------------------------


_FX void *Token_Restrict(
    void *TokenObject, ULONG FilterFlags, ULONG *OutIntegrityLevel,
    PROCESS *proc)
{
    TOKEN_GROUPS *groups;
    TOKEN_PRIVILEGES *privs;
    TOKEN_USER *user;
    void *NewTokenObject = NULL;
	
    /*if (Conf_Get_Boolean(proc->box->name, L"CreateToken", 0, FALSE))
    {
        
    }*/

	// OpenToken BEGIN
	if (Conf_Get_Boolean(proc->box->name, L"OpenToken", 0, FALSE) || Conf_Get_Boolean(proc->box->name, L"UnrestrictedToken", 0, FALSE)) {

		//NTSTATUS status = SeFilterToken(TokenObject, 0, NULL, NULL, NULL, &NewTokenObject);
        //if(!NT_SUCCESS(status))
        //    Log_Status_Ex_Process(MSG_1222, 0xA0, status, NULL, proc->box->session_id, proc->pid);

        HANDLE OldTokenHandle;
        NTSTATUS status = ObOpenObjectByPointer(
            TokenObject, OBJ_KERNEL_HANDLE, NULL, TOKEN_ALL_ACCESS,
            *SeTokenObjectType, KernelMode, &OldTokenHandle);
        if (NT_SUCCESS(status)) {

            HANDLE NewTokenHandle;
            status = ZwDuplicateToken(OldTokenHandle, TOKEN_ALL_ACCESS, NULL,
                FALSE, TokenPrimary, &NewTokenHandle);
            if (NT_SUCCESS(status)) {

                status = ObReferenceObjectByHandle(NewTokenHandle, 0, *SeTokenObjectType,
                    UserMode, &NewTokenObject, NULL);
                if (!NT_SUCCESS(status))
                    Log_Status_Ex_Process(MSG_1222, 0xA3, status, NULL, proc->box->session_id, proc->pid);

                NtClose(NewTokenHandle);
            }
            else
                Log_Status_Ex_Process(MSG_1222, 0xA2, status, NULL, proc->box->session_id, proc->pid);

            ZwClose(OldTokenHandle);
        }
        else
            Log_Status_Ex_Process(MSG_1222, 0xA1, status, NULL, proc->box->session_id, proc->pid);

		return NewTokenObject;
	}
	// OpenToken END

    groups = Token_Query(TokenObject, TokenGroups, proc->box->session_id);
    privs = Token_Query(TokenObject, TokenPrivileges, proc->box->session_id);
    user = Token_Query(TokenObject, TokenUser, proc->box->session_id);

    if (groups && privs && user) {

        void *FixedTokenObject = Token_RestrictHelper1(
            TokenObject, OutIntegrityLevel, proc);

        if (FixedTokenObject) {

            TOKEN_PRIVILEGES *privs_arg =
                (FilterFlags & DISABLE_MAX_PRIVILEGE) ? NULL : privs;

            NewTokenObject = Token_RestrictHelper3(
                FixedTokenObject, groups, privs_arg,
                user->User.Sid, FilterFlags, proc->box->session_id);

            ObDereferenceObject(FixedTokenObject);

        }
        else
            NewTokenObject = NULL;

    }
    else
        NewTokenObject = NULL;

    if (user)
        ExFreePool(user);
    if (privs)
        ExFreePool(privs);
    if (groups)
        ExFreePool(groups);

    return NewTokenObject;
}


_FX BOOLEAN Token_ResetPrimary(PROCESS *proc)
{
    PEPROCESS ProcessObject;
    NTSTATUS status;
    BOOLEAN ok = FALSE;
    ULONG UserAndGroups_offset = 0;

	if (!proc->primary_token)
		return TRUE;

#ifdef _WIN64

    if (Driver_OsVersion <= DRIVER_WINDOWS_7) {

        UserAndGroups_offset = 0x90;

    }
    else if (Driver_OsVersion >= DRIVER_WINDOWS_8
        && Driver_OsVersion <= DRIVER_WINDOWS_10) {


        UserAndGroups_offset = 0x98; //good for windows 10 - 10041
    }

#else ! _WIN64

    if (Driver_OsVersion >= DRIVER_WINDOWS_XP
        && Driver_OsVersion <= DRIVER_WINDOWS_2003) {

        UserAndGroups_offset = 0x68;

    }
    else if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA
        && Driver_OsVersion <= DRIVER_WINDOWS_7) {

        UserAndGroups_offset = 0x90;

    }
    else if (Driver_OsVersion >= DRIVER_WINDOWS_8
        && Driver_OsVersion <= DRIVER_WINDOWS_10) {

        UserAndGroups_offset = 0x94; //good for windows 10 build 10041
    }

#endif _WIN64

    //
    // lookup the process object to get the primary token
    //

    status = PsLookupProcessByProcessId(proc->pid, &ProcessObject);
    if (!NT_SUCCESS(status)) {

		Log_Status_Ex_Process(MSG_1222, 0x37, status, NULL, proc->box->session_id, proc->pid);

    }
    else {

        void *TokenObject = PsReferencePrimaryToken(ProcessObject);
        if (!TokenObject) {

			Log_Status_Ex_Process(MSG_1222, 0x31, STATUS_NO_TOKEN, NULL, proc->box->session_id, proc->pid);

        }
        else
        {
            SID_AND_ATTRIBUTES *SidAndAttrsInToken = NULL;
            SID_AND_ATTRIBUTES *SidAndAttrsInTokenOrig = NULL;

            if (UserAndGroups_offset) {

                SidAndAttrsInToken = *(SID_AND_ATTRIBUTES **)
                    ((ULONG_PTR)TokenObject + UserAndGroups_offset);

                // Windows 8.1 update
                if (SidAndAttrsInToken->Sid == (PSID)AnonymousLogonSid || SidAndAttrsInToken->Sid == (PSID)SandboxieLogonSid)
                {
					//DbgPrint("Sbie, restore token pointer\n");

                    SidAndAttrsInTokenOrig = *(SID_AND_ATTRIBUTES **)
                        ((ULONG_PTR)(proc->primary_token) + UserAndGroups_offset);

                    SidAndAttrsInToken->Sid = SidAndAttrsInTokenOrig->Sid;
                }
            }

            PsDereferencePrimaryToken(TokenObject);
			ok = TRUE;
        }

        ObDereferenceObject(ProcessObject);
    }

    return ok;
}

//---------------------------------------------------------------------------
// Token_IsSharedSid_W8
//---------------------------------------------------------------------------

_FX BOOLEAN Token_IsSharedSid_W8(void *TokenObject)
{
    BOOLEAN     bRet = TRUE;
    ULONG       nUserAndGroupCount = 0;
    SID_AND_ATTRIBUTES  *SidAndAttrsInToken = NULL;
    ULONG       UserAndGroupCount_offset = 0;
    ULONG       UserAndGroups_offset = 0;

    if (TokenObject
        &&  Driver_OsVersion >= DRIVER_WINDOWS_8
        &&  Driver_OsVersion <= DRIVER_WINDOWS_10) {

#ifdef _WIN64
        UserAndGroupCount_offset = 0x7c;  //Good for windows 10 - 10041
        UserAndGroups_offset = 0x98;

#else ! _WIN64
        UserAndGroupCount_offset = 0x7c; //Good for windows 10 - 10041
        UserAndGroups_offset = 0x94;
#endif _WIN64

        nUserAndGroupCount = *(ULONG*)((ULONG_PTR)TokenObject + UserAndGroupCount_offset);
        SidAndAttrsInToken = *(SID_AND_ATTRIBUTES **)((ULONG_PTR)TokenObject + UserAndGroups_offset);

        if ((ULONG_PTR)SidAndAttrsInToken->Sid > (ULONG_PTR)SidAndAttrsInToken
            && (ULONG_PTR)SidAndAttrsInToken->Sid <= ((ULONG_PTR)SidAndAttrsInToken + (nUserAndGroupCount + 5) * sizeof(SID_AND_ATTRIBUTES)))
        {
            // The sid is relative to SidAndAttrsInToken.
            // It is normally at ( SidAndAttrsInToken + (nUserAndGroupCount) * sizeof(SID_AND_ATTRIBUTES) ).

            bRet = FALSE;
        }
        else
        {
            bRet = TRUE;
        }
    }

    return bRet;
}

//---------------------------------------------------------------------------
// Token_RestrictHelper1
//---------------------------------------------------------------------------


_FX void *Token_RestrictHelper1(
    void *TokenObject, ULONG *OutIntegrityLevel, PROCESS *proc)
{
    void *NewTokenObject = NULL;
    SID_AND_ATTRIBUTES *SidAndAttrsInToken = NULL;
    ULONG RestrictedSidCount_offset = 0;
    ULONG RestrictedSids_offset = 0;
    ULONG UserAndGroups_offset = 0;
    NTSTATUS status;

#ifdef _WIN64

    if (Driver_OsVersion <= DRIVER_WINDOWS_7) {

        RestrictedSidCount_offset = 0x7C;
        RestrictedSids_offset = 0x98;
        UserAndGroups_offset = 0x90;

    }
    else if (Driver_OsVersion >= DRIVER_WINDOWS_8
        && Driver_OsVersion <= DRIVER_WINDOWS_10) {

        RestrictedSidCount_offset = 0x80;
        RestrictedSids_offset = 0xA0;
        UserAndGroups_offset = 0x98;  //good for windows 10 - 10041
    }

#else ! _WIN64

    if (Driver_OsVersion >= DRIVER_WINDOWS_XP
        && Driver_OsVersion <= DRIVER_WINDOWS_2003) {

        RestrictedSidCount_offset = 0x50;
        RestrictedSids_offset = 0x6C;
        UserAndGroups_offset = 0x68;

    }
    else if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA
        && Driver_OsVersion <= DRIVER_WINDOWS_7) {

        RestrictedSidCount_offset = 0x7C;
        RestrictedSids_offset = 0x94;
        UserAndGroups_offset = 0x90;

    }
    else if (Driver_OsVersion >= DRIVER_WINDOWS_8
        && Driver_OsVersion <= DRIVER_WINDOWS_10) {

        RestrictedSidCount_offset = 0x80;
        RestrictedSids_offset = 0x98;
        UserAndGroups_offset = 0x94;
    }

#endif _WIN64

    //
    // create a temporary token, which we will fix a bit before it is
    // duplicated again in Token_RestrictHelper3
    //
    // first, if the existing token already includes restricting sids,
    // then we need to remove these, because we want Token_RestrictHelper3
    // to be able to specify just the anonymous SID.  we do this below.
    //
    // we will also need to enlarge the token before calling SeFilterToken
    // in Token_RestrictHelper3, see below in the next section of code
    //

    if (RestrictedSids_offset) {

        //
        // SeFilterToken will fail if the existing token has
        // restricting SIDs and we specify a NULL RestrictSids parameter,
        // so we just grab the first entry in the token's RestrictedSid
        // field and pass that.  note that SeQueryInformationToken does
        // not support the TokenRestrictedSids information class, so we
        // have to go directly to the token
        //

        void *TempNewTokenObject;
        ULONG_PTR RestrictSids_Space[8];
        TOKEN_GROUPS *RestrictSids = NULL;
        ULONG_PTR SidPtr = 0;
        ULONG SidCount = 0;

        ULONG RestrictSidCountInToken = *(ULONG *)
            ((ULONG_PTR)TokenObject + RestrictedSidCount_offset);
        SID_AND_ATTRIBUTES *RestrictSidsInToken = *(SID_AND_ATTRIBUTES **)
            ((ULONG_PTR)TokenObject + RestrictedSids_offset);

        if (RestrictSidCountInToken && RestrictSidsInToken) {

            RestrictSids = (TOKEN_GROUPS *)RestrictSids_Space;
            RestrictSids->GroupCount = 1;
            RestrictSids->Groups[0].Sid = RestrictSidsInToken->Sid;
            RestrictSids->Groups[0].Attributes = 0;

            SidCount = RestrictSids->GroupCount;
            SidPtr = (ULONG_PTR)RestrictSids->Groups;
        }

        //
        // we actually call the unexported SepFilterToken function rather
        // than the SeFilterToken wrapper, because SeFilterToken always
        // passes zero in the second-to-last parameter, which is used to
        // extend the data block allocated to the new token.  we pass 128
        // in that parameter to make room for the restricted sid
        //

        status = Sbie_SepFilterToken_KernelMode(
            TokenObject, SidCount, SidPtr, 128,
            &TempNewTokenObject);

        //
        // now we need to call the wrapper SeFilterToken because it inserts
        // the the new token object as a handle which is going to be needed
        // for the ObOpenObjectByPointer call in Token_RestrictHelper2
        //

        if (NT_SUCCESS(status)) {

            status = Sbie_SeFilterToken_KernelMode(
                TempNewTokenObject, 0, NULL, NULL, RestrictSids,
                &NewTokenObject);

            ObDereferenceObject(TempNewTokenObject);
        }

        if (RestrictSidsInToken) {

            //
            // if the new token has a restricting SID now, then SeFilterToken
            // in Token_RestrictHelper3 will probably fail because the
            // intersection between that SID and the anonymous SID which is
            // specified in Token_RestrictHelper3 probably produces a null
            // set, which is an error.  work around this by getting rid of
            // the restring SID, which we really only added in the first
            // place in order to coax SeFilterToken (above) to not fail
            //

            ULONG_PTR AddressToSetZero =
                ((ULONG_PTR)NewTokenObject + RestrictedSidCount_offset);
            *(ULONG *)AddressToSetZero = 0;

            AddressToSetZero =
                ((ULONG_PTR)NewTokenObject + RestrictedSids_offset);
            *(ULONG_PTR *)AddressToSetZero = 0;
        }

    }
    else
        status = STATUS_UNKNOWN_REVISION;

    if (!NT_SUCCESS(status))
        NewTokenObject = NULL;

    else {

        //
        // the kernel function PsImpersonateClient (which is used by all
        // impersonation services) will grant impersonation even without
        // the SeImpersonatePrivilege privilege, if the requested token
        // has the same SID string as the primary token in the process
        //
        // a thread in the sandbox might abuse this to elevate itself
        // to full privileges by opening its process token through our
        // syscall interface, and then invoking an impersonation request
        // that does not go through our syscall interface
        //
        // to work around this limitation, we overwrite the SID in the
        // primary token to the anonymous SID so the thread can at best
        // elevate from a restricted token to the anonymous token.
        // (we first check the SubAuthorityCount field to make sure the
        // current SID is long enough to be overwritten.)
        //
        // caveat: for compatibility with Windows 2000, it is possible to
        // configure Windows to include the anonymous SID in the Everyone
        // group.  see also:  http://support.microsoft.com/kb/278259
        //

        if (UserAndGroups_offset) {

            SidAndAttrsInToken = *(SID_AND_ATTRIBUTES **)
                ((ULONG_PTR)NewTokenObject + UserAndGroups_offset);
        }

        if (SidAndAttrsInToken) {

            UCHAR *SidInToken = (UCHAR *)SidAndAttrsInToken->Sid;
            if (SidInToken && SidInToken[1] >= 1) { // SubAuthorityCount >= 1

				PSID NewSid = NULL;

                //
                // Alternative (less secure) workaround for msi issue started with windows 17763
                // the workaround in Proc_CreateProcessInternalW_RS5 makes solves thsi usse well
                //
                //if (!proc->image_from_box && _wcsicmp(proc->image_name, L"msiexec.exe") == 0
                //    && RtlEqualSid(SidInToken, SystemLogonSid)
                //    && Conf_Get_Boolean(proc->box->name, L"MsiInstallerExemptions", 0, FALSE))
                //{
                //    //DbgPrint("Sbie, MsiServer was allowed to keep its system token\n");
                //}
                //else
                
                // SbieLogin BEGIN
				if (Conf_Get_Boolean(proc->box->name, L"SandboxieLogon", 0, FALSE))
				{
					if (SandboxieLogonSid[0] != 0)
						NewSid = (PSID)SandboxieLogonSid;
					else
						status = STATUS_UNSUCCESSFUL;
				}
				else
				// SbieLogin END

                // debug tip. To disable anonymous logon, set AnonymousLogon=n

                if (Conf_Get_Boolean(proc->box->name, L"AnonymousLogon", 0, TRUE))
                {
					NewSid = (PSID)AnonymousLogonSid;
                }

				if (NewSid != NULL)
				{
					//  In windows 8.1 Sid can be in two difference places. One is relative to SidAndAttrsInToken. 
					//  By debugger, the offset is 0xf0 after SidAndAttrsInToken. The other one is with KB2919355, 
					//  Sid is not relative to SidAndAttrsInToken, it is shared with other processes and it doesn't 
					//  have its own memory inside the token. We can't call memcpy on this shared memory. Workaround is
					//  to assign Sandbox's AnonymousLogonSid to it.

					// If user sid points to the end of token's UserAndGroups, the sid is not shared. 

					if ((Driver_OsVersion >= DRIVER_WINDOWS_8
						&& Driver_OsVersion <= DRIVER_WINDOWS_10
						&& Token_IsSharedSid_W8(NewTokenObject))
					
					// When trying apply the SbieLogin token to a system process there is not enough space in the SID
					// so we need to use a workaround not unlike the one for win 8
						|| (RtlLengthSid(SidInToken) < RtlLengthSid(NewSid))
						) {

						//DbgPrint("Sbie, hack token pointer\n");
						SidAndAttrsInToken->Sid = (PSID)NewSid;
					}
					else {
						memcpy(SidInToken, NewSid, RtlLengthSid(NewSid));
					}
				}
            }
            else
                status = STATUS_UNKNOWN_REVISION;
        }
        else
            status = STATUS_UNKNOWN_REVISION;

        //
        // on Windows Vista, set the untrusted integrity integrity label,
        // primarily to prevent programs in the sandbox from being able to
        // call PostThreadMessage to threads of programs outside the sandbox
        // and to prevent injection of Win32 and WinEvent hooks
        //

        if (NT_SUCCESS(status)) {

            status = Token_RestrictHelper2(
                NewTokenObject, OutIntegrityLevel, proc);
        }

        if (!NT_SUCCESS(status)) {

            ObDereferenceObject(NewTokenObject);
            NewTokenObject = NULL;
        }
    }

    //
    // finish
    //

    if (!NT_SUCCESS(status))
		Log_Status_Ex_Process(MSG_1222, 0x32, status, NULL, proc->box->session_id, proc->pid);

    return NewTokenObject;
}


//---------------------------------------------------------------------------
// Token_RestrictHelper2
//---------------------------------------------------------------------------


_FX NTSTATUS Token_RestrictHelper2(
    void *TokenObject, ULONG *OutIntegrityLevel, PROCESS *proc)
{
    NTSTATUS status;
    ULONG label;

    if (Driver_OsVersion < DRIVER_WINDOWS_VISTA)
        return STATUS_SUCCESS;

    label = (ULONG)(ULONG_PTR)Token_Query(
        TokenObject, TokenIntegrityLevel, proc->box->session_id);

    if (OutIntegrityLevel)
        *OutIntegrityLevel = label;

	// OpenToken BEGIN
	if (Conf_Get_Boolean(proc->box->name, L"KeepTokenIntegrity", 0, FALSE))
		return STATUS_SUCCESS;
	// OpenToken END

    if (label & 0xFFFF00FF)
        status = STATUS_INVALID_LEVEL;

    else if (label == SECURITY_MANDATORY_UNTRUSTED_RID)
        status = STATUS_SUCCESS;

    else {

        //
        // we determined the token has a non-zero integrity level,
        // i.e. anything higher than an untrusted label, so we have
        // to replace the integrity label in the token
        //

        ULONG LabelSid[4];
        SID_AND_ATTRIBUTES SidAndAttrs;
        HANDLE TokenHandle;

        LabelSid[0] = 0x00000101;
        LabelSid[1] = 0x10000000;
        // debug tip. You can change the sandboxed process's integrity level below
        //LabelSid[2] = SECURITY_MANDATORY_HIGH_RID;
        LabelSid[2] = SECURITY_MANDATORY_UNTRUSTED_RID;
        LabelSid[3] = 0;
        SidAndAttrs.Sid = LabelSid;
        SidAndAttrs.Attributes = 0;

        //
        // get a kernel handle for the token object
        //

        status = ObOpenObjectByPointer(TokenObject, OBJ_KERNEL_HANDLE,
            NULL, 0, NULL, KernelMode, &TokenHandle);

        if (NT_SUCCESS(status)) {

            //
            // adjust the integrity level.  note that the Vista token
            // includes a structure field called IntegrityLevelIndex which
            // may cache the integrity level in some way.  this field is
            // not changed by the call below
            //

            status = ZwSetInformationToken(TokenHandle, TokenIntegrityLevel,
                &SidAndAttrs, sizeof(SidAndAttrs));

            ZwClose(TokenHandle);
        }
    }

    return status;
}


//---------------------------------------------------------------------------
// Token_RestrictHelper3
//---------------------------------------------------------------------------


_FX void *Token_RestrictHelper3(
    void *TokenObject, TOKEN_GROUPS *Groups, TOKEN_PRIVILEGES *Privileges,
    PSID UserSid, ULONG FilterFlags, ULONG SessionId)
{
    void *NewTokenObject;
    TOKEN_GROUPS *Disabled;
    TOKEN_GROUPS *Restricted;
    ULONG Disabled_len, Restricted_len, i, n;
    NTSTATUS status;

    //
    // allocate storage for a couple of group structures, one for the
    // sids we disable, the other for the list of restricting sids
    //

    Disabled_len = sizeof(TOKEN_GROUPS)
        + (Groups->GroupCount + 2) * sizeof(SID_AND_ATTRIBUTES);
    Restricted_len = sizeof(TOKEN_GROUPS) + sizeof(SID_AND_ATTRIBUTES);

    Disabled = Mem_Alloc(Driver_Pool, Disabled_len);
    Restricted = Mem_Alloc(Driver_Pool, Restricted_len);

    //
    // populate the disabled groups array with the groups from the token
    //

    if (Disabled && Restricted) {

        BOOLEAN UserSidAlreadyInGroups = FALSE;
        BOOLEAN AnonymousLogonSidAlreadyInGroups = FALSE;
		// todo: should we do somethign with SandboxieLogonSid here?

        n = 0;

        for (i = 0; i < Groups->GroupCount; ++i) {

            if (Groups->Groups[i].Attributes & SE_GROUP_INTEGRITY)
                continue;

            if (RtlEqualSid(Groups->Groups[i].Sid, UserSid))
                UserSidAlreadyInGroups = TRUE;

            if (RtlEqualSid(Groups->Groups[i].Sid, AnonymousLogonSid))
                AnonymousLogonSidAlreadyInGroups = TRUE;

            Disabled->Groups[n].Sid = Groups->Groups[i].Sid;
            Disabled->Groups[n].Attributes = 0;
            ++n;
        }

        //
        // append the user SID and the anonymous logon SID to the array
        //

        if (!UserSidAlreadyInGroups) {

            Disabled->Groups[n].Sid = UserSid;
            Disabled->Groups[n].Attributes = 0;
            ++n;
        }

        // Adding Anonymous to the disabled group causes a STATUS_ACCESS_DENIED in Win 10 win32k.
        // See ICD-13779
        if (!AnonymousLogonSidAlreadyInGroups && (Driver_OsVersion <= DRIVER_WINDOWS_7)) {

            Disabled->Groups[n].Sid = AnonymousLogonSid;
            Disabled->Groups[n].Attributes = 0;
            ++n;
        }

        Disabled->GroupCount = n;

        //
        // add just the Anonymous Logon SID to the restricted groups array.
        // because we set Anonymous Logon SID as a deny-only SID (above),
        // we get a token that cannot pass any access checks
        //

        // Adding Anonymous to the restricted group causes a STATUS_ACCESS_DENIED in Win 10 win32k.
        // See ICD-13779
        if (Driver_OsVersion <= DRIVER_WINDOWS_7) {
            Restricted->Groups[0].Sid = AnonymousLogonSid;
            Restricted->Groups[0].Attributes = 0;

            Restricted->GroupCount = 1;
        }
        else
            Restricted->GroupCount = 0;

        //
        // create the new token as a restricted token
        //

        // Debug Tip: to make an unrestricted token, swap the Sbie_SeFilterToken_KernelMode calls below:
        //status = Sbie_SeFilterToken_KernelMode(TokenObject, 0, 0, 0, 0, &NewTokenObject);

        status = Sbie_SeFilterToken_KernelMode(TokenObject, FilterFlags, Disabled, Privileges, Restricted, &NewTokenObject);

        if (!NT_SUCCESS(status)) {

            NewTokenObject = NULL;
            Log_Status_Ex_Session(MSG_1222, 0x33, status, NULL, SessionId);
        }

    }
    else
        NewTokenObject = NULL;

    //
    // finish
    //

    if (Restricted)
        Mem_Free(Restricted, Restricted_len);
    if (Disabled)
        Mem_Free(Disabled, Disabled_len);

    return NewTokenObject;
}


//---------------------------------------------------------------------------
// Token_AssignPrimaryHandle
//---------------------------------------------------------------------------

typedef struct _PROCESS_INFO {
    unsigned int type;
    unsigned int data;
} PROCESS_INFO;

_FX NTSTATUS Token_AssignPrimaryHandle(
    void *ProcessObject, HANDLE TokenKernelHandle, ULONG SessionId)
{
    NTSTATUS status;
    HANDLE ProcessHandle;
    ULONG *PtrPrimaryTokenFrozen = NULL;
    ULONG SavePrimaryTokenFrozen;

    //
    // get a kernel handle for the process object
    //

    status = ObOpenObjectByPointer(ProcessObject, OBJ_KERNEL_HANDLE,
        NULL, 0, NULL, KernelMode, &ProcessHandle);
    if (!NT_SUCCESS(status)) {

        Log_Status_Ex_Session(MSG_1222, 0x34, status, NULL, SessionId);
        return status;
    }

    //
    // on Windows Vista and later, we need to clear the PrimaryTokenFrozen
    // bit in the EPROCESS structure before we can replace the primary token

    // Hard Offset Dependency

    // dt nt!_eprocess

    if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA) {

        ULONG Flags2_Offset;                // EPROCESS.Flags2
        ULONG SignatureLevel_Offset;        // EPROCESS.SignatureLevel
        ULONG MitigationFlags_Offset = 0;   // EPROCESS.MitigationFlags
        PROCESS_INFO myProcessInfo;
#ifdef _WIN64

        if (Driver_OsVersion >= DRIVER_WINDOWS_10) {
            if (Driver_OsBuild >= 19013) {
                Flags2_Offset = 0x460;
                MitigationFlags_Offset = 0x9d0;
                SignatureLevel_Offset = 0x878;
            }
            else if (Driver_OsBuild >= 18980) {
                Flags2_Offset = 0x460;
                MitigationFlags_Offset = 0x9d0;
                SignatureLevel_Offset = 0x879;
            }
            else if (Driver_OsBuild >= 18885) { //Windows 10 RS7 FR
                Flags2_Offset = 0x318;
                MitigationFlags_Offset = 0x890;
                SignatureLevel_Offset = 0x738;
            }
            else if (Driver_OsBuild >= 18312) { // Windows 10 May 2019 Update
                Flags2_Offset = 0x308;
                MitigationFlags_Offset = 0x850;
                SignatureLevel_Offset = 0x6f8;
            }
            else if (Driver_OsBuild >= 18290) { //Windows 10 RS6 FR
                Flags2_Offset = 0x308;
                MitigationFlags_Offset = 0x828;
                SignatureLevel_Offset = 0x6d0;
            }
            else if (Driver_OsBuild >= 17661) { //Windows 10 RS5 FR
                Flags2_Offset = 0x300;
                MitigationFlags_Offset = 0x820;
                SignatureLevel_Offset = 0x6c8;
            }
            else if (Driver_OsBuild >= 16241) {
                Flags2_Offset = 0x300;
                MitigationFlags_Offset = 0x828; //Flags4_Offset in windows 10 FCU
                SignatureLevel_Offset = 0x6c8;
            }
            else if (Driver_OsBuild < 14965 || Driver_OsBuild >= 15042) {
                Flags2_Offset = 0x300;  // Windows 10,  64-bit
                SignatureLevel_Offset = 0x6c8;
                MitigationFlags_Offset = 0x6cc;
            }
            else {
                Flags2_Offset = 0x308;  // Windows 10 Fast Ring build 14965+,  64-bit
                SignatureLevel_Offset = 0x6d0;

            }
        }
        else if (Driver_OsVersion == DRIVER_WINDOWS_8 || Driver_OsVersion == DRIVER_WINDOWS_81)
            Flags2_Offset = 0x2F8;  // Windows 8, 8.1,  64-bit

        else if (Driver_OsVersion == DRIVER_WINDOWS_7)
            Flags2_Offset = 0x43C;  // Windows 7,       64-bit

        else
            Flags2_Offset = 0x36C;  // Windows XP,      64-bit
                                    // Windows Vista,   64-bit

#else ! _WIN64

        //Set Flags2_Offset;
        if (Driver_OsVersion > DRIVER_WINDOWS_XP) {
            Flags2_Offset = Process_Flags2;
        }
        else
            Flags2_Offset = 0x224;  // Windows Vista,   32-bit

        //Set Flags3 / Mitigation Flag Offset
        if (Driver_OsVersion >= DRIVER_WINDOWS_10) {
            if (Driver_OsBuild < 16241) {
                MitigationFlags_Offset = Process_Flags3;
                SignatureLevel_Offset = MitigationFlags_Offset - 4;
            }
            else {
                SignatureLevel_Offset = Process_Flags3 - 4;
                MitigationFlags_Offset = Process_Flags3 + 0xe8;
            }

        }

        /*WCHAR msg[256];
		swprintf(msg, L"BAM: Flags2_Offset=%d MitigationFlags_Offset=%d SignatureLevel_Offset=%d\n", Flags2_Offset, MitigationFlags_Offset, SignatureLevel_Offset);
		Session_MonitorPut(MONITOR_OTHER, msg, PsGetCurrentProcessId());*/

#endif _WIN64

        PtrPrimaryTokenFrozen = (ULONG *)((UCHAR *)ProcessObject + Flags2_Offset);
        if (Driver_OsBuild >= 15031 && Driver_OsBuild < 16241) {
            if (*PtrPrimaryTokenFrozen & 0x400) {       // DisableDynamicCode = 0x400

            //Turn off signature Integrity Check
                *(USHORT *)((UCHAR*)ProcessObject + SignatureLevel_Offset) = 0;

                if (MitigationFlags_Offset) {
                    //Turn off process level Control Flow Guard and other undocumented security settings 
                    *(ULONG *)((UCHAR *)ProcessObject + MitigationFlags_Offset) = 0x20;
                }

            }
            myProcessInfo.type = 2; //ProcessDynamicCodePolicy
            myProcessInfo.data = 0; //Turn off DynamicCodePolicy

            ZwSetInformationProcess(ProcessHandle, (PROCESSINFOCLASS)ProcessMitigationPolicy, &myProcessInfo, sizeof(PROCESS_INFO));

        }
        else if (Driver_OsBuild >= 16241) {
            //Turn off process level Control Flow Guard and other undocumented security settings 
            //really Flag4_Offset in this version of windows 10
            *(UCHAR *)((UCHAR *)ProcessObject + MitigationFlags_Offset) = 0x00;
            //Turn off signature Integrity Check
            *(USHORT *)((UCHAR*)ProcessObject + SignatureLevel_Offset) = 0;
            myProcessInfo.type = 2; //ProcessDynamicCodePolicy
            myProcessInfo.data = 0; //Turn off DynamicCodePolicy

            ZwSetInformationProcess(ProcessHandle, (PROCESSINFOCLASS)ProcessMitigationPolicy, &myProcessInfo, sizeof(PROCESS_INFO));
        }
        SavePrimaryTokenFrozen = *PtrPrimaryTokenFrozen & 0x8000;
        *PtrPrimaryTokenFrozen &= ~SavePrimaryTokenFrozen;
    }

    //
    // replace the primary token and restore the PrimaryTokenFrozen bit
    //
    // note that on Windows 7, driver verifier will crash if the token
    // handle is not a kernel handle
    //

    PROCESS_ACCESS_TOKEN info;
    info.Token = TokenKernelHandle;
    info.Thread = NULL;         // not used

    status = ZwSetInformationProcess(ProcessHandle, ProcessAccessToken, &info, sizeof(info));

    if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA) {

        *PtrPrimaryTokenFrozen |= SavePrimaryTokenFrozen;
    }

    //
    // finish
    //

    ZwClose(ProcessHandle);

    if (!NT_SUCCESS(status))
        Log_Status_Ex_Session(MSG_1222, 0x35, status, NULL, SessionId);

    return status;
}


//---------------------------------------------------------------------------
// Token_AssignPrimary
//---------------------------------------------------------------------------


_FX BOOLEAN Token_AssignPrimary(
    void *ProcessObject, void *TokenObject, ULONG SessionId)
{
    NTSTATUS status;
    HANDLE TokenHandle;

    //
    // get a kernel handle for the token object
    //

    status = ObOpenObjectByPointer(TokenObject, OBJ_KERNEL_HANDLE,
        NULL, 0, NULL, KernelMode, &TokenHandle);

    if (NT_SUCCESS(status)) {

        status = Token_AssignPrimaryHandle(
            ProcessObject, TokenHandle, SessionId);

        ZwClose(TokenHandle);
    }

    //
    // finish
    //

    if (!NT_SUCCESS(status)) {

        Log_Status_Ex_Session(MSG_1222, 0x36, status, NULL, SessionId);
        return FALSE;
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Token_ReplacePrimary
//---------------------------------------------------------------------------


_FX BOOLEAN Token_ReplacePrimary(PROCESS *proc)
{
    PEPROCESS ProcessObject;
    NTSTATUS status;
    BOOLEAN ok = FALSE;

	// OriginalToken BEGIN
	if (Conf_Get_Boolean(proc->box->name, L"OriginalToken", 0, FALSE))
		return TRUE;
	// OriginalToken END

    //
    // lookup the process object to get the old primary token
    //

    status = PsLookupProcessByProcessId(proc->pid, &ProcessObject);
    if (!NT_SUCCESS(status)) {

		Log_Status_Ex_Process(MSG_1222, 0x37, status, NULL, proc->box->session_id, proc->pid);

    }
    else {

        void *OriginalToken = Token_FilterPrimary(proc, ProcessObject);

        if (OriginalToken) {

            //
            // restrict the primary token
            //
            // SRP/AppLocker use the process primary token for security
            // checks, which would fail because the primary token does
            // not grant access to anything.  work around this issue by
            // specifying the SANDBOX_INERT flag to inhibit SRP/AppLocker
            //

            void *RestrictedToken =
                Token_Restrict(OriginalToken, SANDBOX_INERT | DISABLE_MAX_PRIVILEGE,
                    &proc->integrity_level, proc);

            if (RestrictedToken) {

#ifdef _WIN64
                // OpenToken BEGIN
                if (!Conf_Get_Boolean(proc->box->name, L"OpenToken", 0, FALSE) 
                 && !Conf_Get_Boolean(proc->box->name, L"UnrestrictedToken", 0, FALSE)
                  && Conf_Get_Boolean(proc->box->name, L"AnonymousLogon", 0, TRUE))
                // OpenToken END
                if (Driver_OsVersion >= DRIVER_WINDOWS_8)
                {
                    UCHAR* pTokenAuthId = (UCHAR*)RestrictedToken;
                    if (pTokenAuthId)
                    {
                        LUID authLuid = ANONYMOUS_LOGON_LUID;
                        pTokenAuthId += 0x18;
                        memcpy(pTokenAuthId, &authLuid, sizeof(LUID));
                    }
                }
#endif

                //
                // set the restricted token as the process token and
                // save a referenced pointer to the new token object
                //

                if (Token_AssignPrimary(ProcessObject, RestrictedToken,
                    proc->box->session_id)) {

                    proc->primary_token = OriginalToken;

                    OriginalToken = NULL;

                    if (proc->drop_rights
                        && proc->integrity_level != tzuk
                        && proc->integrity_level >= 0x3000) {

                        proc->integrity_level = 0x2000;
                    }

                    ok = TRUE;

                }

                ObDereferenceObject(RestrictedToken);
            }

            if (OriginalToken) {
                //
                // note that OriginalToken can be either the original
                // primary token, or a filtered version of it, so we use
                // ObDereferenceObject rather than calling
                // PsDereferencePrimaryToken (which would simply call
                // ObDereferenceObject indirectly)
                //
                ObDereferenceObject(OriginalToken);
            }
        }

        ObDereferenceObject(ProcessObject);
    }

    return ok;
}


//---------------------------------------------------------------------------
// Token_ReleaseProcess
//---------------------------------------------------------------------------


_FX void Token_ReleaseProcess(PROCESS *proc)
{
    if (proc->primary_token) {
        ObDereferenceObject(proc->primary_token);
        proc->primary_token = NULL;
    }
}

_FX NTSTATUS Sbie_SeFilterToken_KernelMode(
    IN PACCESS_TOKEN  ExistingToken,
    IN ULONG  Flags,
    IN PTOKEN_GROUPS  SidsToDisable  OPTIONAL,
    IN PTOKEN_PRIVILEGES  PrivilegesToDelete  OPTIONAL,
    IN PTOKEN_GROUPS  RestrictedSids  OPTIONAL,
    OUT PACCESS_TOKEN  *NewToken
)
{
    UNICODE_STRING  usSboxDevName = { 0 };
    IO_STATUS_BLOCK ios = { 0 };
    NTSTATUS status = STATUS_SUCCESS;
    HANDLE  hFile = NULL;
    OBJECT_ATTRIBUTES oa = { 0 };
    NTSTATUS statusRet = STATUS_UNSUCCESSFUL;

    if (ExGetPreviousMode() == KernelMode)
    {
        statusRet = SeFilterToken(ExistingToken, Flags, SidsToDisable, PrivilegesToDelete, RestrictedSids, NewToken);
    }
    else
    {

        RtlInitUnicodeString(&usSboxDevName, API_DEVICE_NAME);
        InitializeObjectAttributes(&oa, &usSboxDevName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, 0, 0);

        status = ZwCreateFile(&hFile
            , GENERIC_READ | GENERIC_WRITE
            , &oa, &ios, 0, FILE_ATTRIBUTE_NORMAL
            , FILE_SHARE_READ | FILE_SHARE_WRITE
            , FILE_OPEN
            , FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE
            , 0, 0
        );

        if (NT_SUCCESS(status))
        {
            Sbie_SeFilterTokenArg arg = { ExistingToken, Flags, SidsToDisable, PrivilegesToDelete, RestrictedSids, NewToken, &statusRet };

            status = ZwDeviceIoControlFile(hFile
                , NULL
                , NULL
                , NULL
                , &ios
                , API_SBIEDRV_FILTERTOKEN_CTLCODE
                , &arg
                , sizeof(Sbie_SeFilterTokenArg)
                , NULL
                , 0
            );

            ZwClose(hFile);
        }
    }

    return statusRet;
}


_FX NTSTATUS Sbie_SepFilterToken_KernelMode(
    void*       TokenObject,
    ULONG_PTR   SidCount,
    ULONG_PTR   SidPtr,
    ULONG_PTR   LengthIncrease,
    void        **NewToken
)
{
    UNICODE_STRING  usSboxDevName = { 0 };
    IO_STATUS_BLOCK ios = { 0 };
    NTSTATUS status = STATUS_SUCCESS;
    HANDLE  hFile = NULL;
    OBJECT_ATTRIBUTES oa = { 0 };
    NTSTATUS statusRet = STATUS_UNSUCCESSFUL;

    if (ExGetPreviousMode() == KernelMode)
    {
        statusRet = Sbie_SepFilterTokenHandler(TokenObject, SidCount, SidPtr, LengthIncrease, NewToken);
    }
    else
    {
        RtlInitUnicodeString(&usSboxDevName, API_DEVICE_NAME);
        InitializeObjectAttributes(&oa, &usSboxDevName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, 0, 0);

        status = ZwCreateFile(&hFile
            , GENERIC_READ | GENERIC_WRITE
            , &oa, &ios, 0, FILE_ATTRIBUTE_NORMAL
            , FILE_SHARE_READ | FILE_SHARE_WRITE
            , FILE_OPEN
            , FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE
            , 0, 0
        );

        if (NT_SUCCESS(status))
        {
            Sbie_SepFilterTokenArg arg = { TokenObject, SidCount, SidPtr, LengthIncrease, NewToken, &statusRet };

            status = ZwDeviceIoControlFile(hFile
                , NULL
                , NULL
                , NULL
                , &ios
                , API_SBIEDRV_PFILTERTOKEN_CTLCODE
                , &arg
                , sizeof(Sbie_SepFilterTokenArg)
                , NULL
                , 0
            );

            ZwClose(hFile);
        }
    }

    return statusRet;
}

_FX NTSTATUS Sbie_SepFilterTokenHandler_asm(void* TokenObject, ULONG_PTR   SidCount, ULONG_PTR   SidPtr, ULONG_PTR   LengthIncrease, void** NewToken);

_FX NTSTATUS Sbie_SepFilterTokenHandler(void *TokenObject,
    ULONG_PTR   SidCount,
    ULONG_PTR   SidPtr,
    ULONG_PTR   LengthIncrease,
    void        **NewToken)
{
    NTSTATUS status = 0;

#ifdef _WIN64
    //
    // When built with VS2019 on systems with enabled "Core Isolation" we get a BSOD pointing to _chkstk,
    // this is a function added by the compiler under certain conditions.
    // We work around this issue by providing a hand crafter wrapper function that performs the call.
    //

    status = Sbie_SepFilterTokenHandler_asm(TokenObject, SidCount, SidPtr, LengthIncrease, NewToken);
#else
    if (Driver_OsVersion >= DRIVER_WINDOWS_81) {

        status = ((P_SepFilterToken_W81)Token_SepFilterToken)(
            TokenObject, 0, 0, 0, 0, 0, 0, SidCount, SidPtr, LengthIncrease,
            NewToken);

    }
    else {

        status = Token_SepFilterToken(
            TokenObject, 0, 0, 0, 0, 0, 0, SidCount, SidPtr, LengthIncrease,
            NewToken);
    }
#endif

    return status;
}

ULONG GetThreadTokenOwnerPid()
{
    HANDLE hHandle = 0;
    ULONG ulResult = 0;
    PVOID impToken = NULL;

	if (NT_SUCCESS(ZwOpenThreadToken(NtCurrentThread(), TOKEN_ALL_ACCESS, FALSE, &hHandle)) &&
		NT_SUCCESS(ObReferenceObjectByHandle(hHandle, TOKEN_ALL_ACCESS, *SeTokenObjectType, UserMode, &impToken, NULL)))
    {
        // first field is token source
        TOKEN_SOURCE* tokenName = (TOKEN_SOURCE*)impToken;
        // Is this a Sandboxie token?
        if (memcmp(tokenName->SourceName, Sbie_Token_SourceName, sizeof(Sbie_Token_SourceName)) == 0)
        {
            ulResult = tokenName->SourceIdentifier.LowPart;
        }
    }
    if (hHandle)
        NtClose(hHandle);
    if (impToken)
        ObDereferenceObject(impToken);
    return ulResult;
}