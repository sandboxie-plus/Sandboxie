/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2024 David Xanatos, xanasoft.com
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
#ifdef _M_ARM64
#include "common/arm64_asm.h"
#endif
#include "session.h"
#include "dyn_data.h"


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
    void *TokenObject, PROCESS *proc);

static NTSTATUS Token_RestrictHelper2(
    void *TokenObject, PROCESS *proc);

static void *Token_RestrictHelper3(
    void *TokenObject, TOKEN_GROUPS *Groups, TOKEN_PRIVILEGES *Privileges,
    PSID UserSid, ULONG FilterFlags, PROCESS *proc);

static BOOLEAN Token_AssignPrimary(
    void *ProcessObject, void *TokenObject, ULONG SessionId);

static void *Token_DuplicateToken(void *TokenObject, PROCESS *proc);

static void *Token_CreateToken(void *TokenObject, PROCESS *proc);


//---------------------------------------------------------------------------


NTSTATUS Thread_GetKernelHandleForUserHandle(
    HANDLE *OutKernelHandle, HANDLE InUserHandle);

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

//UCHAR SandboxieLogonSid[SECURITY_MAX_SID_SIZE] = { 0 }; // SbieLogin

UCHAR SandboxieAllSid[16] = { // S-1-5-100-0
    1,                                      // Revision
    2,                                      // SubAuthorityCount
    0,0,0,0,0,5, // SECURITY_NT_AUTHORITY   // IdentifierAuthority
    SBIE_RID,0,0,0,                         // SubAuthority[0] = SBIE_RID
    0,0,0,0                                 // SubAuthority[1] = 0
};

UCHAR SandboxieAdminSid[16] = { // S-1-5-100-544
    1,                                      // Revision
    2,                                      // SubAuthorityCount
    0,0,0,0,0,5, // SECURITY_NT_AUTHORITY   // IdentifierAuthority
    SBIE_RID,0,0,0,                         // SubAuthority[0]
    0x20, 0x02, 0x00, 0x00                  // SubAuthority[1] = 544 (0x220 in little endian = 0x20 0x02 0x00 0x00)
};

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
    const ULONG NumBasePrivs = 7;
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
    MySetPrivilege(6) = SE_MANAGE_VOLUME_PRIVILEGE;
    MySetPrivilege(7) = SE_TIME_ZONE_PRIVILEGE; // vista

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
    // find SepFilterToken for Token_RestrictHelper1
    //

    if (!Token_Init_SepFilterToken())
        return FALSE;

    //
    // finish
    //

    Api_SetFunction(API_FILTER_TOKEN,       Token_Api_Filter);

    return TRUE;
}


//---------------------------------------------------------------------------
// Token_Init_SbieLogin
//---------------------------------------------------------------------------


//_FX BOOLEAN Token_Init_SbieLogin(void)
//{
//    WCHAR AccountBuffer[64]; // DNLEN + 1 + sizeof(SANDBOXIE_USER) + reserve
//	UNICODE_STRING AccountName = { 0, sizeof(AccountBuffer), AccountBuffer }; // Note: max valid length is (DNLEN (15) + 1) * sizeof(WCHAR), length is in bytes leave half empty
//	if (NT_SUCCESS(GetRegString(RTL_REGISTRY_ABSOLUTE, L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ActiveComputerName", L"ComputerName", &AccountName)) && AccountName.Length < 64)
//	{
//		wcscpy(AccountName.Buffer + (AccountName.Length / sizeof(WCHAR)), L"\\" SANDBOXIE_USER);
//		AccountName.Length += (1 + wcslen(SANDBOXIE_USER)) * sizeof(WCHAR);
//		//DbgPrint("Sbie, AccountName: %S\n", AccountName.Buffer);
//
//		SID_NAME_USE use;
//		ULONG userSize = sizeof(SandboxieLogonSid), domainSize = 0;
//		WCHAR DomainBuff[20]; // doesn't work without this
//		UNICODE_STRING DomainName = { 0, sizeof(DomainBuff), DomainBuff };
//
//		SecLookupAccountName(&AccountName, &userSize, (PSID)SandboxieLogonSid, &use, &domainSize, &DomainName);
//		//DbgPrint("Sbie, SecLookupAccountName: %x; size:%d %d\n", status, userSize, domainSize);
//	}
//
//    return TRUE;
//}


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

#ifdef _M_ARM64

        ULONG i;
        for (i = 0; i < 256; i += 4) {

            //
            // ARM64: look for "BL SepFilterToken"
            //

            BL bl;
            bl.OP = *(ULONG*)ptr;

            if (bl.op1 == 0b00101 && bl.op2 == 0b1) {

                LONG delta = (bl.imm26 << 2); // * 4
                if (delta & (1 << 27)) // if this is negative
                    delta |= 0xF0000000; // make it properly negative

                Token_SepFilterToken = (P_SepFilterToken)(ptr + delta);
                DbgPrint("SepFilterToken: %p\n", Token_SepFilterToken);

                break;
            }

            ptr += 4;
        }

#else !_M_ARM64

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

#endif _M_ARM64
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

	// UnfilteredToken BEGIN
	if (Conf_Get_Boolean(proc->box->name, L"UnfilteredToken", 0, FALSE)) {
		return PrimaryToken;
	}
	// UnfilteredToken END

    // DbgPrint("   Process Token %08X - %d <%S>\n", PrimaryToken, proc->pid, proc->image_name);

    proc->drop_rights = proc->use_security_mode || Process_GetConf_bool(proc, L"DropAdminRights", FALSE);

    DropRights = (proc->drop_rights ? -1 : 0);

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
    void *TokenObject, ULONG FilterFlags,
    PROCESS *proc)
{
    TOKEN_GROUPS *groups;
    TOKEN_PRIVILEGES *privs;
    TOKEN_USER *user;
    void *NewTokenObject = NULL;
    void *FixedTokenObject;

	// UnrestrictedToken BEGIN
	if (Conf_Get_Boolean(proc->box->name, L"UnrestrictedToken", 0, FALSE)) {

        //NTSTATUS status = SeFilterToken(TokenObject, 0, NULL, NULL, NULL, &NewTokenObject);
        //if(!NT_SUCCESS(status))
        //    Log_Status_Ex_Process(MSG_1222, 0xA0, status, NULL, proc->box->session_id, proc->pid);
        // return NewTokenObject;

        return Token_DuplicateToken(TokenObject, proc);
	}
	// UnrestrictedToken END

    //
    // Create a heavily restricted primary token
    //

	if (Conf_Get_Boolean(proc->box->name, L"UseCreateToken", 0, FALSE) || 
        Conf_Get_Boolean(proc->box->name, L"SandboxieAllGroup", 0, FALSE)) {

        //
        // Create a custom restricted token from scratch
        //

        return Token_CreateToken(TokenObject, proc);
	}

    else {
            
        //
        // Create a modified token from the original one
        //

        FixedTokenObject = Token_RestrictHelper1(TokenObject, proc);
    }

    if (!FixedTokenObject)
        return NULL;

    // OpenToken BEGIN
    if (!Conf_Get_Boolean(proc->box->name, L"KeepTokenIntegrity", 0, FALSE))
    // OpenToken END
    {
        //
        // on Windows Vista and later, set the untrusted integrity integrity label,
        // primarily to prevent programs in the sandbox from being able to
        // call PostThreadMessage to threads of programs outside the sandbox
        // and to prevent injection of Win32 and WinEvent hooks
        //

        if (!NT_SUCCESS(Token_RestrictHelper2(FixedTokenObject, proc))) {

            ObDereferenceObject(FixedTokenObject);
            return NULL;
        }
    }

    // OpenToken BEGIN
    if (Conf_Get_Boolean(proc->box->name, L"UnstrippedToken", 0, FALSE))
        return FixedTokenObject;
    // OpenToken END

    groups = Token_Query(TokenObject, TokenGroups, proc->box->session_id);
    privs = Token_Query(TokenObject, TokenPrivileges, proc->box->session_id);
    user = Token_Query(TokenObject, TokenUser, proc->box->session_id);

    if (groups && privs && user) {

        TOKEN_PRIVILEGES* privs_arg =
            (FilterFlags & DISABLE_MAX_PRIVILEGE) ? NULL : privs;

        NewTokenObject = Token_RestrictHelper3(
            FixedTokenObject, groups, privs_arg,
            user->User.Sid, FilterFlags, proc);
    }

    if (user)
        ExFreePool(user);
    if (privs)
        ExFreePool(privs);
    if (groups)
        ExFreePool(groups);

    ObDereferenceObject(FixedTokenObject);

    return NewTokenObject;
}

//---------------------------------------------------------------------------
// Token_ResetPrimary
//---------------------------------------------------------------------------


_FX BOOLEAN Token_ResetPrimary(PROCESS *proc)
{
    PEPROCESS ProcessObject;
    NTSTATUS status;
    BOOLEAN ok = FALSE;
	if (!proc->primary_token)
		return TRUE;

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
        else {

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

    // $Offset$
    if (TokenObject && Dyndata_Active
        &&  Driver_OsVersion >= DRIVER_WINDOWS_8
        &&  Driver_OsVersion <= DRIVER_WINDOWS_10) {

        nUserAndGroupCount = *(ULONG*)((ULONG_PTR)TokenObject + Dyndata_Config.UserAndGroupCount_offset);
        SidAndAttrsInToken = *(SID_AND_ATTRIBUTES **)((ULONG_PTR)TokenObject + Dyndata_Config.UserAndGroups_offset);

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
    void *TokenObject, PROCESS *proc)
{
    void *NewTokenObject = NULL;
    SID_AND_ATTRIBUTES *SidAndAttrsInToken = NULL;
    NTSTATUS status;

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

    // $Offset$
    if (Dyndata_Active) {

        //
        // SeFilterToken will fail if the existing token has
        // restricting SIDs and we specify a NULL RestrictSids parameter,
        // so we just grab the first entry in the token's RestrictedSid
        // field and pass that.  note that SeQueryInformationToken does
        // not support the TokenRestrictedSids information class, so we
        // have to go directly to the token
        //

        void *TempNewTokenObject;
		PSID OrigTokenSid = NULL;
        ULONG_PTR RestrictSids_Space[8];
        TOKEN_GROUPS *RestrictSids = NULL;
        ULONG_PTR SidPtr = 0;
        ULONG SidCount = 0;

        ULONG RestrictSidCountInToken = *(ULONG *)
            ((ULONG_PTR)TokenObject + Dyndata_Config.RestrictedSidCount_offset);
        SID_AND_ATTRIBUTES *RestrictSidsInToken = *(SID_AND_ATTRIBUTES **)
            ((ULONG_PTR)TokenObject + Dyndata_Config.RestrictedSids_offset);

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
		// Note: Sandboxie originally called here SeFilterToken already,
		// this duplicated NewTokenObject and discarded it.
		//
		// However the blunt method used in the code below to replace the
		// token SID can create a dependency on proc->SandboxieLogonSid
		// formally AnonymousLogonSid which can cause issues and BSOD's
		// It also has required a mitigation in Token_ResetPrimary, restoring
		// the SID pointer so that the token object can be safely destroyed.
		//
		// Therefore the invocation of SeFilterToken has been moved after
		// the SID manipulation, this way the modified TempNewTokenObject
		// will be quickly and safely disposed of. So we continue from there 
		// on out with a proper unhacked token object.
		//

        if (NT_SUCCESS(status)) {

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

            if (Dyndata_Config.UserAndGroups_offset) {

                SidAndAttrsInToken = *(SID_AND_ATTRIBUTES **)
                    ((ULONG_PTR)TempNewTokenObject + Dyndata_Config.UserAndGroups_offset);
            }

            if (SidAndAttrsInToken) {

                UCHAR *SidInToken = (UCHAR *)SidAndAttrsInToken->Sid;
                if (SidInToken && SidInToken[1] >= 1) { // SubAuthorityCount >= 1

                    // debug tip. To disable anonymous logon, set AnonymousLogon=n

                    if (!proc->SandboxieLogonSid && Conf_Get_Boolean(proc->box->name, L"AnonymousLogon", 0, TRUE))
                    {
					    proc->SandboxieLogonSid = (PSID)AnonymousLogonSid;
                    }

				    if (proc->SandboxieLogonSid)
				    {
					    //  In windows 8.1 Sid can be in two difference places. One is relative to SidAndAttrsInToken. 
					    //  By debugger, the offset is 0xf0 after SidAndAttrsInToken. The other one is with KB2919355, 
					    //  Sid is not relative to SidAndAttrsInToken, it is shared with other processes and it doesn't 
					    //  have its own memory inside the token. We can't call memcpy on this shared memory. Workaround is
					    //  to assign Sandbox's AnonymousLogonSid to it.

					    // If user sid points to the end of token's UserAndGroups, the sid is not shared. 

					    if (Token_IsSharedSid_W8(TempNewTokenObject)
					
					    // When trying apply the SbieLogin token to a system process there is not enough space in the SID
					    // so we need to use a workaround not unlike the one for win 8
				        || (RtlLengthSid(SidInToken) < RtlLengthSid(proc->SandboxieLogonSid))
						    ) {

						    //DbgPrint("Sbie, hacking token pointer\n");

							OrigTokenSid = SidAndAttrsInToken->Sid;

						    SidAndAttrsInToken->Sid = proc->SandboxieLogonSid;
					    }
					    else {
						    memcpy(SidInToken, proc->SandboxieLogonSid, RtlLengthSid(proc->SandboxieLogonSid));
					    }
				    }
                }
                else
                    status = STATUS_UNKNOWN_REVISION;
            }
            else
                status = STATUS_UNKNOWN_REVISION;
        }

        //
        // now we need to call the wrapper SeFilterToken because it inserts
        // the the new token object as a handle which is going to be needed
        // for the ObOpenObjectByPointer call in Token_RestrictHelper2
        //

        if (NT_SUCCESS(status)) {

            status = Sbie_SeFilterToken_KernelMode(
                TempNewTokenObject, 0, NULL, NULL, RestrictSids,
                &NewTokenObject);

			//
			// Here we restore the original sid pointer as it was previously
			// done in Token_ResetPrimary before dereferencing the token object.
			//

			if (SidAndAttrsInToken) {

				// Windows 8.1 update
				if (SidAndAttrsInToken->Sid == (PSID)proc->SandboxieLogonSid)
				{
					//DbgPrint("Sbie, restore token pointer\n");

					SidAndAttrsInToken->Sid = OrigTokenSid;
				}
			}

            ObDereferenceObject(TempNewTokenObject);
        }

        if (NewTokenObject && RestrictSidsInToken) {

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
                ((ULONG_PTR)NewTokenObject + Dyndata_Config.RestrictedSidCount_offset);
            *(ULONG *)AddressToSetZero = 0;

            AddressToSetZero =
                ((ULONG_PTR)NewTokenObject + Dyndata_Config.RestrictedSids_offset);
            *(ULONG_PTR *)AddressToSetZero = 0;
        }

    }
    else
        status = STATUS_UNKNOWN_REVISION;

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
    void *TokenObject, PROCESS *proc)
{
    NTSTATUS status;
    ULONG label;

    if (Driver_OsVersion < DRIVER_WINDOWS_VISTA)
        return STATUS_SUCCESS;

    BOOLEAN NoUntrustedToken = Conf_Get_Boolean(proc->box->name, L"NoUntrustedToken", 0, FALSE);
    BOOLEAN OpenWndStation = Conf_Get_Boolean(proc->box->name, L"OpenWndStation", 0, FALSE);

    label = (ULONG)(ULONG_PTR)Token_Query(
        TokenObject, TokenIntegrityLevel, proc->box->session_id);

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
        if(NoUntrustedToken || OpenWndStation)
            LabelSid[2] = SECURITY_MANDATORY_LOW_RID;
        else
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

    if (!NT_SUCCESS(status))
		Log_Status_Ex_Process(MSG_1222, 0x33, status, NULL, proc->box->session_id, proc->pid);

    return status;
}


//---------------------------------------------------------------------------
// Token_RestrictHelper3
//---------------------------------------------------------------------------


_FX void *Token_RestrictHelper3(
    void *TokenObject, TOKEN_GROUPS *Groups, TOKEN_PRIVILEGES *Privileges,
    PSID UserSid, ULONG FilterFlags, PROCESS *proc)
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
		
        BOOLEAN KeepUserGroup = Conf_Get_Boolean(proc->box->name, L"KeepUserGroup", 0, FALSE);
        BOOLEAN KeepLogonSession = Conf_Get_Boolean(proc->box->name, L"KeepLogonSession", 0, FALSE);
        BOOLEAN OpenWndStation = Conf_Get_Boolean(proc->box->name, L"OpenWndStation", 0, FALSE);

        n = 0;

        for (i = 0; i < Groups->GroupCount; ++i) {

            if (Groups->Groups[i].Attributes & SE_GROUP_INTEGRITY)
                continue;

            if ((KeepLogonSession || OpenWndStation) && (Groups->Groups[i].Attributes & SE_GROUP_LOGON_ID))
                continue;

            if (RtlEqualSid(Groups->Groups[i].Sid, UserSid)) {
                if (KeepUserGroup)
                    continue;
                UserSidAlreadyInGroups = TRUE;
            }

            if (RtlEqualSid(Groups->Groups[i].Sid, AnonymousLogonSid))
                AnonymousLogonSidAlreadyInGroups = TRUE;

            Disabled->Groups[n].Sid = Groups->Groups[i].Sid;
            Disabled->Groups[n].Attributes = 0;
            ++n;
        }

        //
        // append the user SID and the anonymous logon SID to the array
        //

        if (!UserSidAlreadyInGroups && !KeepUserGroup) {

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
            Log_Status_Ex_Session(MSG_1222, 0x33, status, NULL, proc->box->session_id);
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

    // dt nt!_eprocess

    // $Offset$
    if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA && Dyndata_Active) {

        ULONG Flags2_Offset = 0;                // EPROCESS.Flags2
        ULONG SignatureLevel_Offset;        // EPROCESS.SignatureLevel
        ULONG MitigationFlags_Offset = 0;   // EPROCESS.MitigationFlags

#ifdef _WIN64

        Flags2_Offset = Dyndata_Config.Flags2_offset;
        SignatureLevel_Offset = Dyndata_Config.SignatureLevel_offset;
        MitigationFlags_Offset = Dyndata_Config.MitigationFlags_offset;

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

#endif _WIN64

        /*WCHAR msg[256];
		swprintf(msg, L"BAM: Flags2_Offset=%d MitigationFlags_Offset=%d SignatureLevel_Offset=%d\n", Flags2_Offset, MitigationFlags_Offset, SignatureLevel_Offset);
		Session_MonitorPut(MONITOR_OTHER, msg, PsGetCurrentProcessId());*/

        PtrPrimaryTokenFrozen = (ULONG *)((UCHAR *)ProcessObject + Flags2_Offset);

        if (Driver_OsBuild >= 16241) {
            
            //Turn off process level Control Flow Guard and other undocumented security settings 
            //really Flag4_Offset in this version of windows 10
            if(MitigationFlags_Offset)
                *(UCHAR *)((UCHAR *)ProcessObject + MitigationFlags_Offset) = 0x00;

            //Turn off signature Integrity Check
            if(SignatureLevel_Offset)
                *(USHORT *)((UCHAR*)ProcessObject + SignatureLevel_Offset) = 0;
        }
        else if (Driver_OsBuild >= 15031) {

            if (*PtrPrimaryTokenFrozen & 0x400) { // DisableDynamicCode = 0x400
                
                //Turn off process level Control Flow Guard and other undocumented security settings 
                if (MitigationFlags_Offset) 
                    *(ULONG *)((UCHAR *)ProcessObject + MitigationFlags_Offset) = 0x20;

                //Turn off signature Integrity Check
                if (SignatureLevel_Offset) 
                    *(USHORT*)((UCHAR*)ProcessObject + SignatureLevel_Offset) = 0;
            }
        }
        
        if (Driver_OsBuild >= 15031) {

            PROCESS_INFO myProcessInfo;
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

    // $Offset$
    if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA && Dyndata_Active) {

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
	if (proc->bAppCompartment || Conf_Get_Boolean(proc->box->name, L"OriginalToken", 0, FALSE))
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


            if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA) {
                proc->integrity_level = (ULONG)(ULONG_PTR)Token_Query(
                    OriginalToken, TokenIntegrityLevel, proc->box->session_id);
            }

            //
            // restrict the primary token
            //
            // SRP/AppLocker use the process primary token for security
            // checks, which would fail because the primary token does
            // not grant access to anything.  work around this issue by
            // specifying the SANDBOX_INERT flag to inhibit SRP/AppLocker
            //

            void *RestrictedToken =
                Token_Restrict(OriginalToken, SANDBOX_INERT | DISABLE_MAX_PRIVILEGE, proc);

            if (RestrictedToken) {

#ifdef _WIN64
                // OpenToken BEGIN
                if (!Conf_Get_Boolean(proc->box->name, L"ReplicateToken", 0, FALSE)
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

_FX NTSTATUS Sbie_SepFilterTokenHandler_asm(void* TokenObject, ULONG_PTR SidCount, ULONG_PTR SidPtr, ULONG_PTR LengthIncrease, void** NewToken);

_FX NTSTATUS Sbie_SepFilterTokenHandler(void *TokenObject,
    ULONG_PTR   SidCount,
    ULONG_PTR   SidPtr,
    ULONG_PTR   LengthIncrease,
    void        **NewToken)
{
    NTSTATUS status = 0;

#ifdef _WIN64
    //
    // When built with VS2019 on systems with enabled "Core Isolation" (HVCI) we get a BSOD.
    // This is caused by "Control Flow Guard", we could either disable it for this file or
    // work around this issue by providing a hand crafted wrapper function that performs the call.
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


//---------------------------------------------------------------------------
// Token_Api_Filter
//---------------------------------------------------------------------------


_FX NTSTATUS Token_Api_Filter(PROCESS* proc, ULONG64* parms)
{
    NTSTATUS status;
    HANDLE ProcessId = (HANDLE)parms[1];
    KIRQL irql;
    HANDLE hToken = (HANDLE)parms[2];
    PHANDLE pHandle = (PHANDLE)parms[3];
    void *TokenObject;

    if (proc) // only unsandboxed processes - if(PsGetCurrentProcessId() != Api_ServiceProcessId)
        return STATUS_NOT_IMPLEMENTED;

    if (! hToken || !pHandle)
        return STATUS_INVALID_PARAMETER;

    ProbeForWrite(pHandle, sizeof(HANDLE), sizeof(HANDLE));

    proc = Process_Find(ProcessId, &irql);
    if (!proc || proc->terminated) {
        ExReleaseResourceLite(Process_ListLock);
        KeLowerIrql(irql);
        return STATUS_INVALID_CID;
    }

	BOOLEAN DropRights = proc->drop_rights;
	ULONG SessionId = proc->box->session_id;

    ExReleaseResourceLite(Process_ListLock);
    KeLowerIrql(irql);

    status = ObReferenceObjectByHandle(
                hToken, TOKEN_ALL_ACCESS,
                *SeTokenObjectType, UserMode, &TokenObject, NULL);

    if (NT_SUCCESS(status)) {

        void *FilteredTokenObject =
            Token_Filter(TokenObject, DropRights, SessionId);

        if (! FilteredTokenObject)
            status = STATUS_ACCESS_DENIED;
        else {

            HANDLE MyTokenHandle;
            status = ObOpenObjectByPointer(FilteredTokenObject, 0, NULL, TOKEN_ALL_ACCESS, *SeTokenObjectType, UserMode, &MyTokenHandle);

            if (NT_SUCCESS(status))
                *pHandle = MyTokenHandle;

            ObDereferenceObject(FilteredTokenObject);
        }

        ObDereferenceObject(TokenObject);
    }

    return status;
}


//---------------------------------------------------------------------------
// Token_DuplicateToken
//---------------------------------------------------------------------------


_FX void *Token_DuplicateToken(void *TokenObject, PROCESS *proc)
{
    void *NewTokenObject = NULL;

    //
    // This just duplicates a token starting with an object instead of a handle
    // using SepDuplicateToken would be more convenient but it is unexported :/
    //

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


//---------------------------------------------------------------------------
// SbieCreateToken
//---------------------------------------------------------------------------


_FX NTSTATUS SbieCreateToken(PHANDLE TokenHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes,
    TOKEN_TYPE Type, PLUID AuthenticationId, PLARGE_INTEGER ExpirationTime, PTOKEN_USER User, PTOKEN_GROUPS Groups, PTOKEN_PRIVILEGES Privileges,
    PTOKEN_SECURITY_ATTRIBUTES_INFORMATION UserAttributes, PTOKEN_SECURITY_ATTRIBUTES_INFORMATION DeviceAttributes,
    PTOKEN_GROUPS DeviceGroups, PTOKEN_MANDATORY_POLICY MandatoryPolicy,
    PTOKEN_OWNER Owner, PTOKEN_PRIMARY_GROUP PrimaryGroup, PTOKEN_DEFAULT_DACL DefaultDacl, PTOKEN_SOURCE Source)
{
#ifdef _M_ARM64
    if(!Driver_KiServiceInternal)
        return STATUS_INVALID_SYSTEM_SERVICE;
    
    if (ZwCreateTokenEx_num) { // Win 8+
        return Sbie_CallZwServiceFunction_asm((ULONG_PTR)TokenHandle, (ULONG_PTR)DesiredAccess, (ULONG_PTR)ObjectAttributes,
            (ULONG_PTR)Type, (ULONG_PTR)AuthenticationId, (ULONG_PTR)ExpirationTime, (ULONG_PTR)User, (ULONG_PTR)Groups, (ULONG_PTR)Privileges,
            (ULONG_PTR)UserAttributes, (ULONG_PTR)DeviceAttributes, (ULONG_PTR)DeviceGroups, (ULONG_PTR)MandatoryPolicy,
            (ULONG_PTR)Owner, (ULONG_PTR)PrimaryGroup, (ULONG_PTR)DefaultDacl, (ULONG_PTR)Source, 
            0, 0,
            ZwCreateTokenEx_num);
    }
    if (ZwCreateToken_num) {
        NTSTATUS status =  Sbie_CallZwServiceFunction_asm((ULONG_PTR)TokenHandle, (ULONG_PTR)DesiredAccess, (ULONG_PTR)ObjectAttributes,
            (ULONG_PTR)Type, (ULONG_PTR)AuthenticationId, (ULONG_PTR)ExpirationTime, (ULONG_PTR)User, (ULONG_PTR)Groups, (ULONG_PTR)Privileges,
            (ULONG_PTR)Owner, (ULONG_PTR)PrimaryGroup, (ULONG_PTR)DefaultDacl, (ULONG_PTR)Source, 
            0, 0, 0, 0, 0, 0,
            ZwCreateToken_num);
#else
    if (ZwCreateTokenEx) { // Win 8+
#ifdef _WIN64
        return Sbie_CallFunction_asm(ZwCreateTokenEx, (UINT_PTR)TokenHandle, (UINT_PTR)DesiredAccess, (UINT_PTR)ObjectAttributes,
            (UINT_PTR)Type, (UINT_PTR)AuthenticationId, (UINT_PTR)ExpirationTime, (UINT_PTR)User, (UINT_PTR)Groups, (UINT_PTR)Privileges,
            (UINT_PTR)UserAttributes, (UINT_PTR)DeviceAttributes, (UINT_PTR)DeviceGroups, (UINT_PTR)MandatoryPolicy,
            (UINT_PTR)Owner, (UINT_PTR)PrimaryGroup, (UINT_PTR)DefaultDacl, (UINT_PTR)Source, 0, 0);
#else
        return ZwCreateTokenEx(TokenHandle, DesiredAccess, ObjectAttributes,
            Type, AuthenticationId, ExpirationTime, User, Groups, Privileges,
            UserAttributes, DeviceAttributes, DeviceGroups, MandatoryPolicy,
            Owner, PrimaryGroup, DefaultDacl, Source);
#endif
    }
    if (ZwCreateToken) {
#ifdef _WIN64
        NTSTATUS status = Sbie_CallFunction_asm(ZwCreateToken, (UINT_PTR)TokenHandle, (UINT_PTR)DesiredAccess, (UINT_PTR)ObjectAttributes,
            (UINT_PTR)Type, (UINT_PTR)AuthenticationId, (UINT_PTR)ExpirationTime, (UINT_PTR)User, (UINT_PTR)Groups, (UINT_PTR)Privileges,
            (UINT_PTR)Owner, (UINT_PTR)PrimaryGroup, (UINT_PTR)DefaultDacl, (UINT_PTR)Source, 0, 0, 0, 0, 0, 0);
#else
        NTSTATUS status = ZwCreateToken(TokenHandle, DesiredAccess, ObjectAttributes,
            Type, AuthenticationId, ExpirationTime, User, Groups, Privileges,
            Owner, PrimaryGroup, DefaultDacl, Source);
#endif
#endif
        if (NT_SUCCESS(status)) {
            if(MandatoryPolicy)
                ZwSetInformationToken(TokenHandle, TokenMandatoryPolicy, MandatoryPolicy, sizeof(TOKEN_MANDATORY_POLICY));
        }
        return status;
    }
    return STATUS_INVALID_SYSTEM_SERVICE;
}


//---------------------------------------------------------------------------
// Token_CreateToken
//---------------------------------------------------------------------------


_FX void* Token_CreateToken(void* TokenObject, PROCESS* proc)
{
    HANDLE TokenHandle = NULL;
    HANDLE KernelTokenHandle = NULL;
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    PTOKEN_STATISTICS		LocalStatistics = NULL;
    PTOKEN_USER				LocalUser = NULL;
    PTOKEN_GROUPS			LocalGroups = NULL;
    PTOKEN_GROUPS			OldLocalGroups = NULL;
    PTOKEN_PRIVILEGES		LocalPrivileges = NULL;
    
    //PTOKEN_SECURITY_ATTRIBUTES_INFORMATION UserAttributes = NULL;
    //PTOKEN_SECURITY_ATTRIBUTES_INFORMATION DeviceAttributes = NULL;
    //PTOKEN_GROUPS           DeviceGroups = NULL;
    PTOKEN_MANDATORY_POLICY MandatoryPolicy = NULL;

    PTOKEN_OWNER			LocalOwner = NULL;
    PTOKEN_PRIMARY_GROUP	LocalPrimaryGroup = NULL;
    PTOKEN_DEFAULT_DACL		LocalDefaultDacl = NULL;
    PTOKEN_SOURCE			LocalSource = NULL;

    //PTOKEN_DEFAULT_DACL		NewDefaultDacl = NULL;
    //ULONG                   DefaultDacl_Length = 0;
    //PACL                    NewDacl = NULL;


    TOKEN_TYPE              TokenType = TokenPrimary;
    LUID                    AuthenticationId = ANONYMOUS_LOGON_LUID;
    LARGE_INTEGER           ExpirationTime;

    OBJECT_ATTRIBUTES       ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE SecurityQos;

    TOKEN_PRIVILEGES		AllowedPrivilege;
    AllowedPrivilege.PrivilegeCount = 1;
    AllowedPrivilege.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT;
    AllowedPrivilege.Privileges[0].Luid.HighPart = 0;
    AllowedPrivilege.Privileges[0].Luid.LowPart = SE_CHANGE_NOTIFY_PRIVILEGE;

    //
    // Gather information from the original token
    //

    if (   !NT_SUCCESS(SeQueryInformationToken(TokenObject, TokenStatistics, &LocalStatistics))
        || !NT_SUCCESS(SeQueryInformationToken(TokenObject, TokenUser, &LocalUser))
        || !NT_SUCCESS(SeQueryInformationToken(TokenObject, TokenGroups, &LocalGroups))
        || !NT_SUCCESS(SeQueryInformationToken(TokenObject, TokenPrivileges, &LocalPrivileges))

        || !NT_SUCCESS(SeQueryInformationToken(TokenObject, TokenOwner, &LocalOwner))
        || !NT_SUCCESS(SeQueryInformationToken(TokenObject, TokenPrimaryGroup, &LocalPrimaryGroup))
        || !NT_SUCCESS(SeQueryInformationToken(TokenObject, TokenDefaultDacl, &LocalDefaultDacl))
        || !NT_SUCCESS(SeQueryInformationToken(TokenObject, TokenSource, &LocalSource))
        )
    {
        Log_Status_Ex_Process(MSG_1222, 0xA1, STATUS_UNSUCCESSFUL, NULL, proc->box->session_id, proc->pid);
        goto finish;
    }

    MandatoryPolicy = (PTOKEN_MANDATORY_POLICY)ExAllocatePoolWithTag(PagedPool, sizeof(TOKEN_MANDATORY_POLICY), tzuk);
    if (MandatoryPolicy) MandatoryPolicy->Policy = TOKEN_MANDATORY_POLICY_NO_WRITE_UP;

    //
    // Create a new token from scratch
    //

    InitializeObjectAttributes(&ObjectAttributes, NULL, OBJ_CASE_INSENSITIVE, NULL, NULL);

    SecurityQos.Length = sizeof(SecurityQos);
    SecurityQos.ContextTrackingMode = SECURITY_STATIC_TRACKING;
    SecurityQos.EffectiveOnly = FALSE;    
    ObjectAttributes.SecurityQualityOfService = &SecurityQos;

    if (Conf_Get_Boolean(proc->box->name, L"ReplicateToken", 0, FALSE))
    {
        SecurityQos.ImpersonationLevel = LocalStatistics->ImpersonationLevel;

        TokenType = LocalStatistics->TokenType;
        AuthenticationId = LocalStatistics->AuthenticationId;
        ExpirationTime = LocalStatistics->ExpirationTime;
        
    }
    else
    {
        SecurityQos.ImpersonationLevel = SecurityAnonymous;

        ExpirationTime.QuadPart = 0x7FFFFFFFFFFFFFFF;

        if (!Conf_Get_Boolean(proc->box->name, L"UnstrippedToken", 0, FALSE))
        {
            BOOLEAN NoUntrustedToken = Conf_Get_Boolean(proc->box->name, L"NoUntrustedToken", 0, FALSE);
            BOOLEAN OpenWndStation = Conf_Get_Boolean(proc->box->name, L"OpenWndStation", 0, FALSE);
            BOOLEAN KeepUserGroup = Conf_Get_Boolean(proc->box->name, L"KeepUserGroup", 0, FALSE);
            BOOLEAN KeepLogonSession = Conf_Get_Boolean(proc->box->name, L"KeepLogonSession", 0, FALSE);

            for (ULONG i = 0; i < LocalGroups->GroupCount; i++) {

                if (LocalGroups->Groups[i].Attributes & SE_GROUP_INTEGRITY) {
                    if (!Conf_Get_Boolean(proc->box->name, L"KeepTokenIntegrity", 0, FALSE)) {
                        if(NoUntrustedToken || OpenWndStation)
                            *RtlSubAuthoritySid(LocalGroups->Groups[i].Sid, 0) = SECURITY_MANDATORY_LOW_RID;
                        else
                            *RtlSubAuthoritySid(LocalGroups->Groups[i].Sid, 0) = SECURITY_MANDATORY_UNTRUSTED_RID;
                    }
                    continue;
                }

				if ((LocalGroups->Groups[i].Attributes & SE_GROUP_LOGON_ID)) {
					if(!KeepLogonSession)
						LocalGroups->Groups[i].Attributes = SE_GROUP_LOGON_ID | SE_GROUP_USE_FOR_DENY_ONLY;
					continue;
				}

                if (RtlEqualSid(LocalGroups->Groups[i].Sid, LocalUser->User.Sid)) {
                    if (KeepUserGroup)
                        continue;
                }

                LocalGroups->Groups[i].Attributes = SE_GROUP_USE_FOR_DENY_ONLY;
            }
        }

        if (Conf_Get_Boolean(proc->box->name, L"SandboxieAllGroup", 0, FALSE)) // & Driver_SandboxieSid)
        {
            OldLocalGroups = LocalGroups;

            ULONG NewGroupCount = OldLocalGroups->GroupCount + 1;
            SIZE_T NewSize = FIELD_OFFSET(TOKEN_GROUPS, Groups) + NewGroupCount * sizeof(SID_AND_ATTRIBUTES);

            LocalGroups = (PTOKEN_GROUPS)ExAllocatePoolWithTag(PagedPool, NewSize, tzuk);
            RtlZeroMemory(LocalGroups, NewSize);

            LocalGroups->Groups[0].Attributes = SE_GROUP_ENABLED | SE_GROUP_ENABLED_BY_DEFAULT;
            LocalGroups->Groups[0].Sid = SandboxieAllSid; // Driver_SandboxieSid;

            RtlCopyMemory(&LocalGroups->Groups[1], OldLocalGroups->Groups, OldLocalGroups->GroupCount * sizeof(SID_AND_ATTRIBUTES));
            LocalGroups->GroupCount = NewGroupCount;
        }

        /*for (ULONG i = 0; i < LocalPrivileges->PrivilegeCount; ++i) {
            LUID_AND_ATTRIBUTES *entry_i = &LocalPrivileges->Privileges[i];

            DbgPrint("Priv: %d-%d (0x%x)\n", entry_i->Luid.HighPart, entry_i->Luid.LowPart, entry_i->Attributes);
        }*/

        if (LocalPrivileges) ExFreePool((PVOID)LocalPrivileges);
        LocalPrivileges = &AllowedPrivilege;
    }

    //
    // Change the SID
    //
                
    if (!proc->SandboxieLogonSid && Conf_Get_Boolean(proc->box->name, L"AnonymousLogon", 0, TRUE))
    {
		proc->SandboxieLogonSid = (PSID)AnonymousLogonSid;
    }

	if (proc->SandboxieLogonSid)
	{
        //
        // free old user and create a new one with the new SID
        //

        ULONG Attributes = LocalUser->User.Attributes;

        ExFreePool((PVOID)LocalUser);
        LocalUser = ExAllocatePoolWithTag(PagedPool, sizeof(TOKEN_USER) + RtlLengthSid(proc->SandboxieLogonSid), tzuk);

        LocalUser->User.Attributes = Attributes;
        LocalUser->User.Sid = ((UCHAR*)LocalUser) + sizeof(TOKEN_USER);

		memcpy(LocalUser->User.Sid, proc->SandboxieLogonSid, RtlLengthSid(proc->SandboxieLogonSid));
	}
    
retry:
    status = SbieCreateToken(
        &TokenHandle,
        TOKEN_ALL_ACCESS,
        &ObjectAttributes,
        TokenType,
        &AuthenticationId,
        &ExpirationTime,
        LocalUser,
        LocalGroups,
        LocalPrivileges,

        0, //UserAttributes,
        0, //DeviceAttributes,
        0, //DeviceGroups,
        MandatoryPolicy,

        LocalOwner,
        LocalPrimaryGroup,
        LocalDefaultDacl,
        LocalSource
    );

    if (proc->SandboxieLogonSid && status == STATUS_INVALID_PRIMARY_GROUP && LocalPrimaryGroup->PrimaryGroup != LocalUser->User.Sid)
    {
        //
        // For online accounts we must change the primary group
        //

        ExFreePool((PVOID)LocalPrimaryGroup);
        LocalPrimaryGroup = (PTOKEN_PRIMARY_GROUP)ExAllocatePoolWithTag(PagedPool, sizeof(PTOKEN_PRIMARY_GROUP), tzuk);
        LocalPrimaryGroup->PrimaryGroup = LocalUser->User.Sid;

        goto retry;
    }
    else if (proc->SandboxieLogonSid && status == STATUS_INVALID_OWNER && LocalOwner->Owner != LocalUser->User.Sid)
    {
        //
        // Retry with new DACLs on error
        //

        ExFreePool((PVOID)LocalOwner);
        LocalOwner = (PTOKEN_OWNER)ExAllocatePoolWithTag(PagedPool, sizeof(TOKEN_OWNER), tzuk);
        LocalOwner->Owner = LocalUser->User.Sid;


        //DefaultDacl_Length = LocalDefaultDacl->DefaultDacl->AclSize;

        //// Construct a new ACL 
        //NewDefaultDacl = (PTOKEN_DEFAULT_DACL)ExAllocatePoolWithTag(PagedPool, sizeof(TOKEN_DEFAULT_DACL) + 8 + DefaultDacl_Length + 128, tzuk);
        //memcpy(NewDefaultDacl, LocalDefaultDacl, DefaultDacl_Length);

        //NewDefaultDacl->DefaultDacl = NewDacl = (PACL)((ULONG_PTR)NewDefaultDacl + sizeof(TOKEN_DEFAULT_DACL));
        //NewDefaultDacl->DefaultDacl->AclSize += 128;

        //RtlAddAccessAllowedAce(NewDacl, ACL_REVISION2, GENERIC_ALL, LocalOwner->Owner);

        goto retry;
    }


    if (!NT_SUCCESS(status))
    {
        Log_Status_Ex_Process(MSG_1222, 0xA3, status, NULL, proc->box->session_id, proc->pid);
        goto finish;
    }

    if (NT_SUCCESS(status))
        status = Thread_GetKernelHandleForUserHandle(&KernelTokenHandle, TokenHandle);

    //if (NT_SUCCESS(status) && NewDacl)
    //{
    //    Token_SetHandleDacl(NtCurrentProcess(), NewDacl);
    //    Token_SetHandleDacl(NtCurrentThread(), NewDacl);
    //    Token_SetHandleDacl(KernelTokenHandle, NewDacl);
    //}

    if (NT_SUCCESS(status)) 
    {
        ULONG virtualizationAllowed = 1;
        status = ZwSetInformationToken(KernelTokenHandle, TokenVirtualizationAllowed, &virtualizationAllowed, sizeof(ULONG));
    }

    if (!NT_SUCCESS(status))
    {
        Log_Status_Ex_Process(MSG_1222, 0xA4, status, NULL, proc->box->session_id, proc->pid);
        goto finish;
    }

    if (Conf_Get_Boolean(proc->box->name, L"CopyTokenAttributes", 0, FALSE))
    {
        HANDLE OldTokenHandle;
        status = ObOpenObjectByPointer(
            TokenObject, OBJ_KERNEL_HANDLE, NULL, TOKEN_ALL_ACCESS,
            *SeTokenObjectType, KernelMode, &OldTokenHandle);
        if (NT_SUCCESS(status)) 
        {
            void* ptr = ExAllocatePoolWithTag(PagedPool, PAGE_SIZE, tzuk);

            ULONG len = 0;
            status = ZwQueryInformationToken(OldTokenHandle, TokenSecurityAttributes, ptr, PAGE_SIZE, &len);
            if (NT_SUCCESS(status)) {

                PTOKEN_SECURITY_ATTRIBUTES_AND_OPERATION_INFORMATION data = (PTOKEN_SECURITY_ATTRIBUTES_AND_OPERATION_INFORMATION)(((UCHAR*)ptr) + len);
                len += sizeof(TOKEN_SECURITY_ATTRIBUTES_AND_OPERATION_INFORMATION);

                data->Attributes = ptr;
                data->Operations = (PTOKEN_SECURITY_ATTRIBUTE_OPERATION)(((UCHAR*)ptr) + len);
                len += sizeof(TOKEN_SECURITY_ATTRIBUTE_OPERATION) * data->Attributes->AttributeCount;
                for (ULONG i = 0; i < data->Attributes->AttributeCount; i++)
                    data->Operations[i] = TOKEN_SECURITY_ATTRIBUTE_OPERATION_ADD;

                status = ZwSetInformationToken(TokenHandle, TokenSecurityAttributes, data, len);
            }

            ExFreePool(ptr);

            ZwClose(OldTokenHandle);
        }
    }

finish:
    if (KernelTokenHandle)  ZwClose(KernelTokenHandle);

    //UNICODE_STRING unicodeString;

    //DbgPrint("Create Token: 0x%08x\n", status);
    //if (NT_SUCCESS(RtlConvertSidToUnicodeString(&unicodeString, LocalUser->User.Sid, TRUE))) {
    //    DbgPrint("LocalUser: %wZ (0x%x)\n", &unicodeString, LocalUser->User.Attributes);
    //    RtlFreeUnicodeString(&unicodeString);
    //}

    //for (ULONG i = 0; i < LocalGroups->GroupCount; i++) {
    //    if (NT_SUCCESS(RtlConvertSidToUnicodeString(&unicodeString, LocalGroups->Groups[i].Sid, TRUE))) {
    //        DbgPrint("LocalGroups[%d]: %wZ (0x%x)\n", i, &unicodeString, LocalGroups->Groups[i].Attributes);
    //        RtlFreeUnicodeString(&unicodeString);
    //    }
    //}

    //if (NT_SUCCESS(RtlConvertSidToUnicodeString(&unicodeString, LocalOwner->Owner, TRUE))) {
    //    DbgPrint("LocalOwner: %wZ\n", &unicodeString);
    //    RtlFreeUnicodeString(&unicodeString);
    //}

    //if (NT_SUCCESS(RtlConvertSidToUnicodeString(&unicodeString, LocalPrimaryGroup->PrimaryGroup, TRUE))) {
    //    DbgPrint("LocalPrimaryGroup: %wZ\n", &unicodeString);
    //    RtlFreeUnicodeString(&unicodeString);
    //}
    //DbgPrint("+++\n");


    if (LocalStatistics)    ExFreePool((PVOID)LocalStatistics);
    if (LocalUser)          ExFreePool((PVOID)LocalUser);
    if (LocalGroups)        ExFreePool((PVOID)LocalGroups);
    if (OldLocalGroups)     ExFreePool((PVOID)OldLocalGroups);
    if (LocalPrivileges && LocalPrivileges != &AllowedPrivilege) ExFreePool((PVOID)LocalPrivileges);

    //if (UserAttributes)     ExFreePool((PVOID)UserAttributes);
    //if (DeviceAttributes)   ExFreePool((PVOID)DeviceAttributes);
    //if (DeviceGroups)       ExFreePool((PVOID)DeviceGroups);
    if (MandatoryPolicy)    ExFreePool((PVOID)MandatoryPolicy);

    if (LocalOwner)         ExFreePool((PVOID)LocalOwner);
    if (LocalPrimaryGroup)  ExFreePool((PVOID)LocalPrimaryGroup);
    if (LocalDefaultDacl)   ExFreePool((PVOID)LocalDefaultDacl);
    if (LocalSource)        ExFreePool((PVOID)LocalSource);

    //if (NewDefaultDacl)     ExFreePool((PVOID)NewDefaultDacl);

    //
    // get the actual token object from the handle
    //

    void* NewTokenObject = NULL;
    if (TokenHandle != NULL) {
        status = ObReferenceObjectByHandle(TokenHandle, 0, *SeTokenObjectType, UserMode, &NewTokenObject, NULL);
        if (!NT_SUCCESS(status))
            Log_Status_Ex_Process(MSG_1222, 0xA5, status, NULL, proc->box->session_id, proc->pid);

        NtClose(TokenHandle);
    }
    return NewTokenObject;
}

