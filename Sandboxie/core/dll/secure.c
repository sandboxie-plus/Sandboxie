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
// Secure
//---------------------------------------------------------------------------


#include "dll.h"
#include "obj.h"
#include "core/drv/api_defs.h"
#include "core/svc/ServiceWire.h"
#include <stdio.h>


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static NTSTATUS Secure_NtOpenProcess(
    HANDLE *ProcessHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    CLIENT_ID *ClientId);

static NTSTATUS Secure_NtOpenThread(
    HANDLE *ThreadHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    CLIENT_ID *ClientId);

static NTSTATUS Secure_NtDuplicateObject(
    HANDLE SourceProcessHandle,
    HANDLE SourceHandle,
    HANDLE TargetProcessHandle,
    HANDLE *TargetHandle,
    ACCESS_MASK DesiredAccess,
    ULONG HandleAttributes,
    ULONG Options);

static NTSTATUS Secure_NtQuerySecurityObject(
    HANDLE Handle,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    ULONG Length,
    ULONG *LengthNeeded);

static NTSTATUS Secure_NtSetSecurityObject(
    HANDLE Handle,
    SECURITY_INFORMATION SecurityInformation,
    SECURITY_DESCRIPTOR *SecurityDescriptor);

static NTSTATUS Secure_NtQueryInformationToken(
    HANDLE TokenHandle,
    TOKEN_INFORMATION_CLASS TokenInformationClass,
    void *TokenInformation,
    ULONG TokenInformationLength,
    ULONG *ReturnLength);

static NTSTATUS Secure_NtSetInformationToken(
    HANDLE TokenHandle,
    TOKEN_INFORMATION_CLASS TokenInformationClass,
    void *TokenInformation,
    ULONG TokenInformationLength);

static NTSTATUS Secure_NtAdjustPrivilegesToken(
    HANDLE TokenHandle,
    BOOLEAN DisableAllPrivileges,
    TOKEN_PRIVILEGES *NewState,
    ULONG BufferLength,
    TOKEN_PRIVILEGES *PreviousState,
    ULONG *ReturnLength);

static NTSTATUS Secure_RtlQueryElevationFlags(ULONG *Flags);

static NTSTATUS Secure_RtlCheckTokenMembershipEx(
    HANDLE tokenHandle,
    PSID sidToCheck,
    DWORD flags,
    PUCHAR isMember);

static BOOLEAN Secure_IsSameBox(HANDLE idProcess);


//---------------------------------------------------------------------------


static P_NtOpenProcess              __sys_NtOpenProcess             = NULL;
static P_NtOpenThread               __sys_NtOpenThread              = NULL;
static P_NtDuplicateObject          __sys_NtDuplicateObject         = NULL;
static P_NtQuerySecurityObject      __sys_NtQuerySecurityObject     = NULL;
static P_NtSetSecurityObject        __sys_NtSetSecurityObject       = NULL;
static P_NtQueryInformationToken    __sys_NtQueryInformationToken   = NULL;
static P_NtSetInformationToken      __sys_NtSetInformationToken     = NULL;
static P_NtAdjustPrivilegesToken    __sys_NtAdjustPrivilegesToken   = NULL;
static P_RtlQueryElevationFlags     __sys_RtlQueryElevationFlags    = NULL;
static P_RtlCheckTokenMembershipEx  __sys_RtlCheckTokenMembershipEx = NULL;
static P_NtQuerySecurityAttributesToken __sys_NtQuerySecurityAttributesToken = NULL;


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


PSECURITY_DESCRIPTOR Secure_NormalSD = NULL;

PSECURITY_DESCRIPTOR Secure_EveryoneSD = NULL;

BOOLEAN Secure_IsInternetExplorerTabProcess = FALSE;
BOOLEAN Secure_Is_IE_NtQueryInformationToken = FALSE;

BOOLEAN Secure_FakeAdmin = FALSE;

//---------------------------------------------------------------------------
// Secure_InitSecurityDescriptors
//---------------------------------------------------------------------------


void Secure_InitSecurityDescriptors(void)
{
    typedef (*P_RtlAddMandatoryAce)(
        PACL Acl, ULONG AceRevision, ULONG AceFlags,
        PSID Sid, ULONG AceType, ULONG MandatoryPolicy);

    PACL MyAcl;
    P_RtlAddMandatoryAce pRtlAddMandatoryAce;

    static UCHAR AuthenticatedUsersSid[12] = {
        1,                                      // Revision
        1,                                      // SubAuthorityCount
        0,0,0,0,0,5, // SECURITY_NT_AUTHORITY   // IdentifierAuthority
        SECURITY_AUTHENTICATED_USER_RID         // SubAuthority
    };
    static UCHAR EveryoneSid[12] = {
        1,                                      // Revision
        1,                                      // SubAuthorityCount
        0,0,0,0,0,1, // SECURITY_WORLD_SID_AUTHORITY
        SECURITY_WORLD_RID                      // SubAuthority
    };
    /*static UCHAR RestrictedSid[12] = {
        1,                                      // Revision
        1,                                      // SubAuthorityCount
        0,0,0,0,0,5, // SECURITY_NT_AUTHORITY   // IdentifierAuthority
        SECURITY_RESTRICTED_CODE_RID            // SubAuthority
    };*/
    static UCHAR LowLabelSid[12] = {
        1,                                      // Revision
        1,                                      // SubAuthorityCount
        0,0,0,0,0,16,// SECURITY_MANDATORY_LABEL_AUTHORITY
        0x00, 0x10   // SECURITY_MANDATORY_LOW_RID
    };

    void *UserSid = NULL;

#define MyAddAccessAllowedAce(dacl,owner)                                   \
    RtlAddAccessAllowedAceEx(                                               \
        (dacl),                                                             \
        ACL_REVISION,                                                       \
        CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE | INHERITED_ACE,         \
        GENERIC_ALL,                                                        \
        (owner))

#define MyAllocAndInitACL(acl,aclsz)                                        \
    (acl) = Dll_Alloc(aclsz);                                               \
    RtlCreateAcl((acl), (aclsz), ACL_REVISION);

#define MyAllocAndInitSD(sd)                                                \
    (sd) = Dll_Alloc(64);                                                   \
    RtlCreateSecurityDescriptor((sd), SECURITY_DESCRIPTOR_REVISION);

    //
    // build Normal Security Descriptor used for files, keys, etc
    //

    MyAllocAndInitACL(MyAcl, 256);
    MyAddAccessAllowedAce(MyAcl, &AuthenticatedUsersSid);

    if (Dll_SidString) {
        UserSid = Dll_SidStringToSid(Dll_SidString);
        if (UserSid)
            MyAddAccessAllowedAce(MyAcl, UserSid);
    }

    MyAllocAndInitSD(Secure_NormalSD);
    RtlSetDaclSecurityDescriptor(Secure_NormalSD, TRUE, MyAcl, FALSE);

    //
    // build Everyone Security Descriptor used for named pipes
    //

    MyAllocAndInitACL(MyAcl, 128);
    MyAddAccessAllowedAce(MyAcl, &AuthenticatedUsersSid);
    MyAddAccessAllowedAce(MyAcl, &EveryoneSid);
    //MyAddAccessAllowedAce(MyAcl, &RestrictedSid);

    MyAllocAndInitSD(Secure_EveryoneSD);
    RtlSetDaclSecurityDescriptor(Secure_EveryoneSD, TRUE, MyAcl, FALSE);

    pRtlAddMandatoryAce = (P_RtlAddMandatoryAce)
                            GetProcAddress(Dll_Ntdll, "RtlAddMandatoryAce");
    if (pRtlAddMandatoryAce) {

        //
        // on Windows Vista, also add a low mandatory label to make
        // it possible for low integrity processes to connect
        //

        MyAllocAndInitACL(MyAcl, 64);
        pRtlAddMandatoryAce(MyAcl, ACL_REVISION, 0, LowLabelSid,
                            SYSTEM_MANDATORY_LABEL_ACE_TYPE,
                            SYSTEM_MANDATORY_LABEL_NO_WRITE_UP);

        RtlSetSaclSecurityDescriptor(Secure_EveryoneSD, TRUE, MyAcl, FALSE);
    }

    //
    // finish
    //

    if (UserSid)
        Dll_Free(UserSid);

#undef MyAllocAndInitSD
#undef MyAllocAndInitACL
#undef MyAddAccessAllowedAce
}


//---------------------------------------------------------------------------
// Secure_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Secure_Init(void)
{
    void *RtlQueryElevationFlags;
    void *RtlCheckTokenMembershipEx;

    //
    // intercept NTDLL entry points
    //

    SBIEDLL_HOOK(Secure_,NtOpenProcess);
    SBIEDLL_HOOK(Secure_,NtOpenThread);
    SBIEDLL_HOOK(Secure_,NtDuplicateObject);
    SBIEDLL_HOOK(Secure_,NtQuerySecurityObject);
    SBIEDLL_HOOK(Secure_,NtSetSecurityObject);
    SBIEDLL_HOOK(Secure_,NtSetInformationToken);
    SBIEDLL_HOOK(Secure_,NtAdjustPrivilegesToken);
    if (Dll_Windows < 10) {
        SBIEDLL_HOOK(Secure_, NtQueryInformationToken);
    }

    //
    // check if this is an Internet Explorer 8 tab process
    //

    if (Dll_ImageType == DLL_IMAGE_INTERNET_EXPLORER) {

        const WCHAR *CmdLine = GetCommandLine();
        if (CmdLine && wcsstr(CmdLine, L"SCODEF:")
                    && wcsstr(CmdLine, L"CREDAT:"))
            Secure_IsInternetExplorerTabProcess = TRUE;
    }

    //
    // install hooks to fake administrator privileges
    //

    Secure_FakeAdmin = Config_GetSettingsForImageName_bool(L"FakeAdminRights", FALSE);

    RtlQueryElevationFlags =
        GetProcAddress(Dll_Ntdll, "RtlQueryElevationFlags");

    if (RtlQueryElevationFlags) {

        BOOLEAN ShouldFakeRunningAsAdmin = Secure_FakeAdmin
                ||  Dll_ImageType == DLL_IMAGE_SANDBOXIE_SBIESVC
                ||  Dll_ImageType == DLL_IMAGE_SANDBOXIE_RPCSS
                ||  Dll_ImageType == DLL_IMAGE_INTERNET_EXPLORER
                ||  (_wcsicmp(Dll_ImageName, L"SynTPEnh.exe") == 0)
                ||  (_wcsicmp(Dll_ImageName, L"SynTPHelper.exe") == 0);

        if (ShouldFakeRunningAsAdmin) {

            SBIEDLL_HOOK(Secure_,RtlQueryElevationFlags);

            //
            // if this is an Internet Explorer tab process then we always
            // need to fake administrator privileges because they will be
            // queried by urlmon!InstallBrokerIsNeeded to decide whether
            // to use the Protected Mode ActiveX Installation Broker.
            //
            // or, if Internet Explorer was going to run as administrator
            // but we removed admin privileges due to drop rights, then
            // we have to fake the administrator privileges.
            //
            // otherwise if Internet Explorer was running without admin
            // privileges then we just fake that protected mode is off,
            // see Key_NtQueryValueKeyFakeForInternetExplorer
            //
            // for Internet Explorer 10, we have to fake admin privileges
            // on the parent process as well, otherwise the processes
            // can't connect.  (and assuming that we always want to fake
            // the tab process due to urlmon!InstallBrokerIsNeeded.)
            //

            if (Dll_ImageType == DLL_IMAGE_INTERNET_EXPLORER) {

                if (Secure_IsInternetExplorerTabProcess ||
                        (SH_GetInternetExplorerVersion() >= 10) ||
                        (Dll_ProcessFlags & SBIE_FLAG_RIGHTS_DROPPED)) {

                    Secure_Is_IE_NtQueryInformationToken = TRUE;
                }
            }
        }
    }

    RtlCheckTokenMembershipEx =
        GetProcAddress(Dll_Ntdll, "RtlCheckTokenMembershipEx");

    if (RtlCheckTokenMembershipEx) {

        if (Secure_FakeAdmin) {

            SBIEDLL_HOOK(Secure_, RtlCheckTokenMembershipEx);
        }

    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Secure_NtOpenProcess
//---------------------------------------------------------------------------


_FX NTSTATUS Secure_NtOpenProcess(
    HANDLE *ProcessHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    CLIENT_ID *ClientId)
{
    NTSTATUS status;

    //
    // reduce desired access if trying to open a process outside the sandbox
    // (or in another sandbox) otherwise the driver will cancel our process
    //

    if (ClientId) {

        ULONG64 OtherProcessFlags =
                        SbieApi_QueryProcessInfo(ClientId->UniqueProcess, 0);
        if (OtherProcessFlags) {
            if (! Secure_IsSameBox(ClientId->UniqueProcess))
                OtherProcessFlags = 0;
        }

        if (! OtherProcessFlags) {

            if (DesiredAccess & (MAXIMUM_ALLOWED | PROCESS_DUP_HANDLE)) {
                DesiredAccess |= STANDARD_RIGHTS_READ |
                                 PROCESS_QUERY_INFORMATION |
                                 SYNCHRONIZE;
            }

            DesiredAccess &= STANDARD_RIGHTS_READ | SYNCHRONIZE
                           | PROCESS_VM_READ
                           | PROCESS_QUERY_INFORMATION
                           | PROCESS_QUERY_LIMITED_INFORMATION;
        }
    }

    //
    // open handle to process object
    //

    status = __sys_NtOpenProcess(
        ProcessHandle, DesiredAccess, ObjectAttributes, ClientId);

    if (status == STATUS_ACCESS_DENIED) {

        if (DesiredAccess == PROCESS_QUERY_INFORMATION ||
            DesiredAccess == PROCESS_QUERY_LIMITED_INFORMATION) {

            //
            // possibly an administrator process trying to open a process
            // outside the sandbox, but failing because we stripped the
            // debug privileges
            //

            return SbieApi_OpenProcess(
                        ProcessHandle, ClientId->UniqueProcess);
        }

        //
        // otherwise reduce the desired access and try again
        //

        if (DesiredAccess & (MAXIMUM_ALLOWED | PROCESS_DUP_HANDLE)) {
            DesiredAccess |= STANDARD_RIGHTS_READ |
                             PROCESS_QUERY_INFORMATION |
                             SYNCHRONIZE;
        }

        DesiredAccess &= STANDARD_RIGHTS_READ | SYNCHRONIZE
                       | PROCESS_VM_READ
                       | PROCESS_QUERY_INFORMATION
                       | PROCESS_QUERY_LIMITED_INFORMATION;

        if (DesiredAccess) {
            status = __sys_NtOpenProcess(
                ProcessHandle, DesiredAccess, ObjectAttributes, ClientId);
        }
    }

    return status;
}


//---------------------------------------------------------------------------
// Secure_NtOpenThread
//---------------------------------------------------------------------------


_FX NTSTATUS Secure_NtOpenThread(
    HANDLE *ThreadHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    CLIENT_ID *ClientId)
{
    NTSTATUS status;

    status = __sys_NtOpenThread(
        ThreadHandle, DesiredAccess, ObjectAttributes, ClientId);

    if (status == STATUS_ACCESS_DENIED) {

        if (DesiredAccess & MAXIMUM_ALLOWED) {
            DesiredAccess |= STANDARD_RIGHTS_READ |
                             THREAD_QUERY_INFORMATION |
                             SYNCHRONIZE;
        }

        DesiredAccess &= STANDARD_RIGHTS_READ |
                         THREAD_GET_CONTEXT |
                         THREAD_QUERY_INFORMATION |
                         SYNCHRONIZE;

        if (DesiredAccess) {
            status = __sys_NtOpenThread(
                ThreadHandle, DesiredAccess, ObjectAttributes, ClientId);
        }
    }

    return status;
}


//---------------------------------------------------------------------------
// Secure_NtDuplicateObject
//---------------------------------------------------------------------------


_FX NTSTATUS Secure_NtDuplicateObject(
    HANDLE SourceProcessHandle,
    HANDLE SourceHandle,
    HANDLE TargetProcessHandle,
    HANDLE *TargetHandle,
    ACCESS_MASK DesiredAccess,
    ULONG HandleAttributes,
    ULONG Options)
{
    NTSTATUS status;
    HANDLE OtherProcessHandle;

    //
    // a process may get a read-only handle to a process outside the sandbox
    // and then try to extend the granted access through NtDuplicateObject
    // (for example the GameGaurd process GameDes.mon).  we don't want to
    // send that directly to our driver because it would cancel the process
    //

    if (SourceProcessHandle == NtCurrentProcess() &&
            (! (Options & DUPLICATE_SAME_ACCESS)) &&
            Obj_GetObjectType(SourceHandle) == OBJ_TYPE_PROCESS) {

        if (DesiredAccess & (   PROCESS_CREATE_THREAD
                              | PROCESS_VM_OPERATION
                              | PROCESS_VM_WRITE)) {

            HANDLE OtherProcessId =
                            (HANDLE)(ULONG_PTR)GetProcessId(SourceHandle);
            ULONG64 OtherProcessFlags =
                            SbieApi_QueryProcessInfo(OtherProcessId, 0);
            if (OtherProcessFlags) {
                if (! Secure_IsSameBox(OtherProcessId))
                    OtherProcessFlags = 0;
            }

            if (! OtherProcessFlags) {

                DesiredAccess &= ~(   PROCESS_CREATE_THREAD
                                    | PROCESS_VM_OPERATION
                                    | PROCESS_VM_WRITE);
            }
        }
    }

    //
    // try to issue the request but we may get STATUS_ACCESS_DENIED so
    // be careful not to close the source handle.  on the other hand,
    // if we are successful, then make sure the handle gets closed
    //

    status = __sys_NtDuplicateObject(
        SourceProcessHandle, SourceHandle, TargetProcessHandle, TargetHandle,
        DesiredAccess, HandleAttributes, Options & ~DUPLICATE_CLOSE_SOURCE);

    if (NT_SUCCESS(status)) {

        if (Options & DUPLICATE_CLOSE_SOURCE) {

            //
            // issue NtDuplicateObject again with no TargetProcessHandle
            // to just close the source handle
            //

            __sys_NtDuplicateObject(
                SourceProcessHandle, SourceHandle, NULL, NULL,
                DesiredAccess, HandleAttributes, DUPLICATE_CLOSE_SOURCE);
        }

        if (SourceProcessHandle == NtCurrentProcess()) {

            if (TargetProcessHandle == NtCurrentProcess() && TargetHandle)
                File_DuplicateRecover(SourceHandle, *TargetHandle);

            if (SourceHandle)
                Key_NtClose(SourceHandle);
        }

    //
    // else invoke the driver to duplicate the object
    //

    } else if (status == STATUS_ACCESS_DENIED) {

        OtherProcessHandle = NULL;

        if (TargetProcessHandle == NtCurrentProcess()) {

            if (SourceProcessHandle != NtCurrentProcess()) {

                OtherProcessHandle = SourceProcessHandle;
                /* Options |= DUPLICATE_FROM_OTHER; */
            }

        } else if (SourceProcessHandle == NtCurrentProcess()) {

            OtherProcessHandle = TargetProcessHandle;
            Options |= DUPLICATE_INTO_OTHER;
        }

        if (OtherProcessHandle) {

            if (HandleAttributes & OBJ_INHERIT)
                Options |= DUPLICATE_INHERIT;

            status = SbieApi_DuplicateObject(
                TargetHandle, OtherProcessHandle, SourceHandle,
                DesiredAccess, Options);
        }
    }

    return status;
}


//---------------------------------------------------------------------------
// Secure_NtQuerySecurityObject
//---------------------------------------------------------------------------


_FX NTSTATUS Secure_NtQuerySecurityObject(
    HANDLE Handle,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    ULONG Length,
    ULONG *LengthNeeded)
{
    NTSTATUS status;
    HANDLE handle2 = NULL;
    BOOLEAN IsOpenPath = FALSE;

    ULONG type = Obj_GetObjectType(Handle);
    if (type == OBJ_TYPE_FILE)
        handle2 = File_GetTrueHandle(Handle, &IsOpenPath);
    else if (type == OBJ_TYPE_KEY)
        handle2 = Key_GetTrueHandle(Handle, &IsOpenPath);

    if (IsOpenPath) {

        if (handle2)
            NtClose(handle2);

        status = __sys_NtQuerySecurityObject(
            Handle, SecurityInformation, SecurityDescriptor,
            Length, LengthNeeded);

        return status;
    }

    if (! handle2)
        handle2 = Handle;

    status = __sys_NtQuerySecurityObject(
        handle2, SecurityInformation, SecurityDescriptor,
        Length, LengthNeeded);

    if (status == STATUS_ACCESS_DENIED) {

        const ULONG SACL_INFO = (SACL_SECURITY_INFORMATION |
                                 PROTECTED_SACL_SECURITY_INFORMATION |
                                 UNPROTECTED_SACL_SECURITY_INFORMATION);
        if (SecurityInformation & SACL_INFO) {

            //
            // we never have ACCESS_SYSTEM_SECURITY access for True objects
            //

            SecurityInformation &= ~SACL_INFO;

            status = __sys_NtQuerySecurityObject(
                handle2, SecurityInformation, SecurityDescriptor,
                Length, LengthNeeded);
        }
    }

    if (handle2 != Handle)
        NtClose(handle2);

    return status;
}


//---------------------------------------------------------------------------
// Secure_NtSetSecurityObject
//---------------------------------------------------------------------------


_FX NTSTATUS Secure_NtSetSecurityObject(
    HANDLE Handle,
    SECURITY_INFORMATION SecurityInformation,
    SECURITY_DESCRIPTOR *SecurityDescriptor)
{
    NTSTATUS status;
    HANDLE handle2 = NULL;
    BOOLEAN IsOpenPath = FALSE;
    BOOLEAN IsUnnamedObject = FALSE;

    ULONG type = Obj_GetObjectType(Handle);
    if (type == OBJ_TYPE_FILE)
        handle2 = File_GetTrueHandle(Handle, &IsOpenPath);
    else if (type == OBJ_TYPE_KEY)
        handle2 = Key_GetTrueHandle(Handle, &IsOpenPath);
    else {

        ULONG name_space[16];
        ULONG name_len = sizeof(name_space);
        OBJECT_NAME_INFORMATION *name =
            (OBJECT_NAME_INFORMATION *)name_space;
        status = Obj_GetObjectName(Handle, name, &name_len);
        if (NT_SUCCESS(status) && name->Name.Length == 0) {

            IsUnnamedObject = TRUE;
        }
    }

    //WCHAR txt[256];
    //Sbie_snwprintf(txt, 256, L"NtSetSecurityObject Open=%d Handle=%08X Type=%d Info=%08X\n", IsOpenPath, Handle, type, SecurityInformation);
    //OutputDebugString(txt);
    //while (! IsDebuggerPresent()) Sleep(500); __debugbreak();

    if (IsOpenPath || IsUnnamedObject) {

        if (handle2)
            NtClose(handle2);

        status = __sys_NtSetSecurityObject(
            Handle, SecurityInformation, SecurityDescriptor);

        if (IsOpenPath || (! NT_SUCCESS(status)))
            return status;

        // if IsUnnamedObject then we continue so that we can adjust
        // the label if necessary
    }

    //
    // if the program wants to change the integrity label, we assume
    // it must be a broker process trying to make a resource accessible
    // by a managed process, so we use our low integrity label
    //

    if (Dll_OsBuild >= 6000 &&
            (SecurityInformation & LABEL_SECURITY_INFORMATION)) {

        __sys_NtSetSecurityObject(
            Handle, LABEL_SECURITY_INFORMATION, Secure_EveryoneSD);

        //SecurityInformation &= ~LABEL_SECURITY_INFORMATION;
    }

    //
    // objects created inside the sandbox get a public dacl,
    // and we don't want anyone changing that
    //

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Secure_NtQueryInformationToken
//---------------------------------------------------------------------------


_FX NTSTATUS Secure_NtQueryInformationToken(
    HANDLE TokenHandle,
    TOKEN_INFORMATION_CLASS TokenInformationClass,
    void *TokenInformation,
    ULONG TokenInformationLength,
    ULONG *ReturnLength)
{
    NTSTATUS status = 0;
    THREAD_DATA *TlsData = NULL;


    status = __sys_NtQueryInformationToken(
        TokenHandle, TokenInformationClass,
        TokenInformation, TokenInformationLength, ReturnLength);

    if (!Secure_Is_IE_NtQueryInformationToken)
    {
        return status;
    }

    TlsData = Dll_GetTlsData(NULL);

    //
    // NtQueryInformationToken is hooked only for Internet Explorer.
    //
    // if the check occurs during CreateProcess, then return the real
    // information, so UAC elevation may occur for the new process.
    //
    // otherwise, this check is related to Protected Mode, so pretend
    // we are running as Administrator
    //

    if (NT_SUCCESS(status) && (! TlsData->proc_create_process)) {

        if (TokenInformationClass == TokenElevationType) {

            //
            // on Vista, fake a return value for a full token
            //

            *(ULONG *)TokenInformation = TokenElevationTypeFull;
        }

        if (TokenInformationClass == TokenIntegrityLevel) {

            //
            // on Vista, fake a high integrity level
            //

#include "pshpack4.h"

            typedef struct {

                ULONG_PTR Pointer;
                ULONG_PTR Sixty;
                ULONG     OneOhOne;
                ULONG     HighBitSet;
                ULONG     ThreeK;

            } TOKEN_INTEGRITY_LEVEL;

#include "poppack.h"

            if (TokenInformationLength >= sizeof(TOKEN_INTEGRITY_LEVEL)) {

                TOKEN_INTEGRITY_LEVEL *Info =
                    (TOKEN_INTEGRITY_LEVEL *)TokenInformation;

                Info->Pointer       = (ULONG_PTR)TokenInformation
                                    + sizeof(ULONG_PTR) * 2;
                Info->Sixty         = 0x60;
                Info->OneOhOne      = 0x101;
                Info->HighBitSet    = 0x10000000;
                Info->ThreeK        = 0x3000;

                if (ReturnLength)
                    *ReturnLength = sizeof(TOKEN_INTEGRITY_LEVEL);
            }
        }
    }

    return status;
}


//---------------------------------------------------------------------------
// Secure_NtSetInformationToken
//---------------------------------------------------------------------------


static NTSTATUS Secure_NtSetInformationToken(
    HANDLE TokenHandle,
    TOKEN_INFORMATION_CLASS TokenInformationClass,
    void *TokenInformation,
    ULONG TokenInformationLength)
{
    NTSTATUS status = __sys_NtSetInformationToken(
        TokenHandle, TokenInformationClass,
        TokenInformation, TokenInformationLength);

    if ((! NT_SUCCESS(status)) && (
            TokenInformationClass == TokenSessionId ||
            TokenInformationClass == TokenIntegrityLevel))
        status = STATUS_SUCCESS;

    return status;
}


//---------------------------------------------------------------------------
// Secure_NtAdjustPrivilegesToken
//---------------------------------------------------------------------------


_FX NTSTATUS Secure_NtAdjustPrivilegesToken(
    HANDLE TokenHandle,
    BOOLEAN DisableAllPrivileges,
    TOKEN_PRIVILEGES *NewState,
    ULONG BufferLength,
    TOKEN_PRIVILEGES *PreviousState,
    ULONG *ReturnLength)
{
    ULONG status = __sys_NtAdjustPrivilegesToken(
        TokenHandle, DisableAllPrivileges, NewState,
        BufferLength, PreviousState, ReturnLength);
    if (status == STATUS_NOT_ALL_ASSIGNED)
        status = STATUS_SUCCESS;
    return status;
}


//---------------------------------------------------------------------------
// Secure_RtlQueryElevationFlags
//---------------------------------------------------------------------------


_FX NTSTATUS Secure_RtlQueryElevationFlags(ULONG *Flags)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);
    NTSTATUS status;

    //
    // RtlQueryElevationFlags
    //
    // Retrieve UAC system information from the shared memory area
    // (KUSER_SHARED_DATA)
    //
    // Return flags:
    // - ElevationEnabled (0x01) - UAC is enabled
    // - VirtEnabled (0x02) - Virtualization is enabled
    // - InstallerDetectEnabled (0x04) - Detection of installers
    //

    BOOLEAN fake = Secure_FakeAdmin; // FALSE;

    if (Dll_ImageType == DLL_IMAGE_INTERNET_EXPLORER) {

        //
        // RtlQueryElevationFlags hook for Internet Explorer:
        //
        // if the check occurs during CreateProcess, then return the real
        // elevation flags, so UAC elevation may occur for the new process.
        //
        // otherwise, this check is related to Protected Mode, so pretend
        // there is no need to elevate
        //

        if (! TlsData->proc_create_process)
            fake = TRUE;

    } else if (Dll_ImageType == DLL_IMAGE_SANDBOXIE_SBIESVC) {

        //
        // RtlQueryElevationFlags hook for SbieSvc UAC elevation process:
        //
        // even when running as Administrator, in some cases the
        // kernel32!CheckElevationEnabled function (called by
        // kernel32!kernel32!CreateProcessInternalW) will decide that
        // elevation is required for some EXEs, and fail CreateProcess
        // with ERROR_ELEVATION_REQUIRED.  this will cause SH32_DoRunAs
        // to run and invoke ShellExecuteEx/runas, which will result in
        // another invocation of CreateProcess, and an infinite loop.
        //
        // to work around this problem, we need to turn off the bit
        // InstallerDetectEnabled (returning zero flags is also ok), as
        // this disables the checks in kernel32!CheckElevationEnabled
        //
        // we do this only for the SbieSvc UAC elevator process, because
        // we know that process is already running as Administrator.
        //

        if (TlsData->proc_create_process)
            fake = TRUE;

    } else {

        //
        // RtlQueryElevationFlags hook for anything else:
        //
        // - SandboxieRpcSs, which is used to run elevated COM objects,
        // for example the Internet Explorer Protected Mode ActiveX
        // Installation Broker, or elevated Control Panel applets.
        // we return zero flags and the COM object runs without elevation.
        //
        // - a couple of Synaptics programs, which reportedly caused
        // UAC prompts, and seem to run well without actually elevating
        //
        // pretend there is no need to elevate
        //

        fake = TRUE;
    }

    //
    //
    //

    if (fake) {

        *Flags = 0;
        status = STATUS_SUCCESS;

    } else {

        status = __sys_RtlQueryElevationFlags(Flags);
    }

    return status;
}


//---------------------------------------------------------------------------
// Secure_IsRestrictedToken
//---------------------------------------------------------------------------

NTSTATUS Secure_RtlCheckTokenMembershipEx(
    HANDLE tokenHandle,
    PSID sidToCheck,
    DWORD flags,
    PUCHAR isMember)
{

    static UCHAR AdministratorsSid[16] = {
        1,                                      // Revision
        2,                                      // SubAuthorityCount
        0,0,0,0,0,5, // SECURITY_NT_AUTHORITY   // IdentifierAuthority
        0x20, 0, 0, 0,   // SubAuthority 1 - SECURITY_BUILTIN_DOMAIN_RID
        0x20, 2, 0, 0    // SubAuthority 2 - DOMAIN_ALIAS_RID_ADMINS
    };

    typedef BOOL (*P_EqualSid)(PSID pSid1, PSID pSid2);
    extern P_EqualSid __sys_RtlEqualSid;

    if (Secure_FakeAdmin && __sys_RtlEqualSid && __sys_RtlEqualSid(sidToCheck, AdministratorsSid)) {
        if (isMember) *isMember = TRUE;
        return STATUS_SUCCESS;
    }

    NTSTATUS status = __sys_RtlCheckTokenMembershipEx(tokenHandle, sidToCheck, flags, isMember);

    return status;
}


//---------------------------------------------------------------------------
// Secure_IsRestrictedToken
//---------------------------------------------------------------------------


_FX BOOLEAN Secure_IsRestrictedToken(BOOLEAN CheckThreadToken)
{
    NTSTATUS status;
    HANDLE hToken;

    static int _restricted_process = -1;
    BOOLEAN have_process_token = FALSE;
    BOOLEAN return_value = FALSE;

    //
    // open token in current thread or process
    //

    if (CheckThreadToken) {
        status = NtOpenThreadToken(
                        NtCurrentThread(), TOKEN_QUERY, FALSE, &hToken);
    } else
        status = STATUS_NO_TOKEN;

    if (status == STATUS_NO_TOKEN) {

        if (_restricted_process != -1)
            return ((_restricted_process == 1) ? TRUE : FALSE);

        status = NtOpenProcessToken(
                        NtCurrentProcess(), TOKEN_QUERY, &hToken);

        have_process_token = TRUE;
    }

    //
    // check if token has restricted groups
    //

    if (NT_SUCCESS(status)) {

        ULONG64 groups_space[8];
        PTOKEN_GROUPS sids = (PTOKEN_GROUPS)groups_space;
        ULONG len;

        len = sizeof(groups_space);
        status = NtQueryInformationToken(
                            hToken, TokenRestrictedSids, sids, len, &len);

        if (status == STATUS_BUFFER_TOO_SMALL) {

            sids = Dll_AllocTemp(len);
            status = NtQueryInformationToken(
                            hToken, TokenRestrictedSids, sids, len, &len);
        }

        if (NT_SUCCESS(status)) {

            if (sids->GroupCount != 0)
                return_value = TRUE;

            if (_restricted_process == -1 && have_process_token)
                _restricted_process = (return_value ? 1 : 0);
        }

        if (sids != (PTOKEN_GROUPS)groups_space)
            Dll_Free(sids);

        NtClose(hToken);
    }

    return return_value;
}


//---------------------------------------------------------------------------
// Secure_IsLocalSystemToken
//---------------------------------------------------------------------------


_FX BOOLEAN Secure_IsLocalSystemToken(BOOLEAN CheckThreadToken)
{
    NTSTATUS status;
    HANDLE hToken;

    BOOLEAN return_value = FALSE;

    //
    // open token in current thread or process
    //

    if (CheckThreadToken) {
        status = NtOpenThreadToken(
                        NtCurrentThread(), TOKEN_QUERY, FALSE, &hToken);
    } else
        status = STATUS_NO_TOKEN;

    if (status == STATUS_NO_TOKEN) {

        status = NtOpenProcessToken(
                        NtCurrentProcess(), TOKEN_QUERY, &hToken);
    }

    //
    // check if the user in the token is the SYSTEM sid
    //

    if (NT_SUCCESS(status)) {

        ULONG64 user_space[8];
        PTOKEN_USER user = (PTOKEN_USER)user_space;
        ULONG len;

        len = sizeof(user_space);
        status = NtQueryInformationToken(
                            hToken, TokenUser, user, len, &len);

        if (status == STATUS_BUFFER_TOO_SMALL) {

            user = Dll_AllocTemp(len);
            status = NtQueryInformationToken(
                            hToken, TokenUser, user, len, &len);
        }

        if (NT_SUCCESS(status)) {

            UNICODE_STRING SidString;

            status = RtlConvertSidToUnicodeString(
                &SidString, user->User.Sid, TRUE);

            if (NT_SUCCESS(status)) {

                if (_wcsicmp(SidString.Buffer, L"S-1-5-18") == 0)
                    return_value = TRUE;

                RtlFreeUnicodeString(&SidString);
            }
        }

        if (user != (PTOKEN_USER)user_space)
            Dll_Free(user);

        NtClose(hToken);
    }

    return return_value;
}


//---------------------------------------------------------------------------
// Secure_IsSameBox
//---------------------------------------------------------------------------


_FX BOOLEAN Secure_IsSameBox(HANDLE idProcess)
{
    WCHAR boxname[48];
    ULONG session_id;
    NTSTATUS status =
        SbieApi_QueryProcess(idProcess, boxname, NULL, NULL, &session_id);
    if (! NT_SUCCESS(status))
        return FALSE;
    if (session_id != Dll_SessionId)
        return FALSE;
    if (_wcsicmp(boxname, Dll_BoxName) != 0)
        return FALSE;
    return TRUE;
}


//---------------------------------------------------------------------------
//
// Support for UAC Elevation
//
// UAC elevation is invoked via a direct call to NdrAsyncClientCall
// (rather than normal COM wrappers) specifying the target program.
// We intercept the call and signal Sandboxie Service instead.
//
//---------------------------------------------------------------------------


typedef struct _RPC_ASYNC_STATE {

    unsigned int    Size; // size of this structure
    unsigned long   Signature;
    long   Lock;
    unsigned long   Flags;
    void           *StubInfo;
    void           *UserInfo;
    void           *RuntimeInfo;
    ULONG           Event;

    ULONG           NotificationType;
    HANDLE          hEvent;

} RPC_ASYNC_STATE, *PRPC_ASYNC_STATE;


typedef struct _SECURE_UAC_ARGS {

    RPC_ASYNC_STATE *AsyncState;
    void *BindingHandle;
    union {

        ULONG_PTR UnknownParameters[16];

        struct {
            // Elevate Program
            WCHAR *ApplicationName;
            WCHAR *CommandLine;
            ULONG_PTR UnknownParameter5;
            ULONG_PTR UnknownParameter6;
            WCHAR *CurrentDirectory;
            WCHAR *WindowStation;
            ULONG_PTR UnknownParameter9;
            ULONG_PTR UnknownParameter10;
            ULONG_PTR UnknownParameter11;
            HANDLE *ProcessHandle;
        } Args1;

    } u;

} SECURE_UAC_ARGS;


typedef struct _SECURE_UAC_PACKET {

    //
    // keep in sync with SbieSvc.exe / ServiceServer2
    //

    ULONG   tzuk;
    ULONG   len;
    ULONG   app_len;
    ULONG   app_ofs;
    ULONG   cmd_len;
    ULONG   cmd_ofs;
    ULONG   dir_len;
    ULONG   dir_ofs;
    ULONG   inv_len;
    ULONG64 hEvent;
    ULONG64 hResult;
    ULONG64 ret_code;
    WCHAR   text[1];

} SECURE_UAC_PACKET;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


#ifdef _WIN64

extern ULONG_PTR __cdecl Secure_Ndr64AsyncClientCall(
    void *pProxyInfo, ULONG nProcNum, void *pReturnValue, ...);

#else

extern ULONG_PTR __cdecl Secure_NdrAsyncClientCall(
    void *pStubDescriptor, void *pFormat, ULONG_PTR *Args);

#endif _WIN64

#ifdef _WIN64

BOOLEAN __cdecl Secure_CheckElevation64(
    PVOID* pProxyInfo, ULONG nProcNum, void* pReturnValue, va_list vl);

#endif _WIN64

BOOLEAN __cdecl Secure_CheckElevation(
    void *ReturnAddressFromNdrAsyncClientCall,
    void *pStubDescriptor, void *pFormat, SECURE_UAC_ARGS *Args);

ULONG_PTR __cdecl Secure_HandleElevation(
    void *pStubDescriptor, void *pFormat, SECURE_UAC_ARGS *Args);

static ULONG_PTR Secure_RpcAsyncCompleteCall(
    RPC_ASYNC_STATE *AsyncState, void *Reply);


//---------------------------------------------------------------------------


typedef ULONG_PTR (__cdecl *P_NdrAsyncClientCall)(
    void *pStubDescriptor, void *pFormat, ULONG_PTR *Args);

typedef ULONG_PTR (__cdecl *P_Ndr64AsyncClientCall)(
    void *pProxyInfo, ULONG nProcNum, void *pReturnValue, ...);

typedef ULONG_PTR (*P_RpcAsyncCompleteCall)(
    RPC_ASYNC_STATE *AsyncState, void *Reply);


//---------------------------------------------------------------------------


#ifdef _WIN64
        P_Ndr64AsyncClientCall  __sys_Ndr64AsyncClientCall          = NULL;
#else
        P_NdrAsyncClientCall    __sys_NdrAsyncClientCall            = NULL;
#endif _WIN64

static  P_RpcAsyncCompleteCall  __sys_RpcAsyncCompleteCall          = NULL;


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static ULONG Secure_Elevation_Type = 0;
static RPC_ASYNC_STATE *Secure_Elevation_AsyncState = NULL;
static HANDLE *Secure_Elevation_ResultHandle = NULL;
static SECURE_UAC_PACKET *Secure_Elevation_Packet = NULL;

static BOOLEAN Secure_Elevation_HookDisabled = FALSE;


//---------------------------------------------------------------------------
// Secure_Init_Elevation
//---------------------------------------------------------------------------


_FX BOOLEAN Secure_Init_Elevation(HMODULE module)
{
#ifdef _WIN64
    void *Ndr64AsyncClientCall;
#else
    void *NdrAsyncClientCall;
#endif _WIN64
    void *RpcAsyncCompleteCall;

    if (Dll_OsBuild < 6000) {

        //
        // earlier than Windows Vista, no UAC
        //

        return TRUE;
    }

    RpcAsyncCompleteCall = (P_RpcAsyncCompleteCall)
        Ldr_GetProcAddrNew(DllName_rpcrt4, L"RpcAsyncCompleteCall","RpcAsyncCompleteCall");

#ifdef _WIN64

    Ndr64AsyncClientCall = (P_Ndr64AsyncClientCall)
        Ldr_GetProcAddrNew(DllName_rpcrt4, L"Ndr64AsyncClientCall","Ndr64AsyncClientCall");

    SBIEDLL_HOOK(Secure_,Ndr64AsyncClientCall);

    //NdrAsyncClientCall(PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING pFormat, ...)
    //Ndr64AsyncClientCall(MIDL_STUBLESS_PROXY_INFO* pProxyInfo, unsigned int nProcNum, void* pReturnValue, ...) <- hook

    //NdrDcomAsyncClientCall(PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING pFormat, ...)
    //Ndr64DcomAsyncClientCall(MIDL_STUBLESS_PROXY_INFO* pProxyInfo, unsigned int nProcNum, void* pReturnValue, ...)

#else ! _WIN64

    NdrAsyncClientCall = (P_NdrAsyncClientCall)
        Ldr_GetProcAddrNew(DllName_rpcrt4, L"NdrAsyncClientCall","NdrAsyncClientCall");

    SBIEDLL_HOOK(Secure_,NdrAsyncClientCall);

    //NdrAsyncClientCall(PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING pFormat, ...) <- hook
    //NdrAsyncClientCall2(PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING pFormat, ...) -> NdrAsyncClientCall

    //NdrDcomAsyncClientCall(PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING pFormat, ...)
    //NdrDcomAsyncClientCall2(PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING pFormat, ...)->NdrDcomAsyncClientCall

#endif _WIN64

    SBIEDLL_HOOK(Secure_,RpcAsyncCompleteCall);

    return TRUE;
}


//---------------------------------------------------------------------------
// Secure_CheckElevation
//---------------------------------------------------------------------------

#ifdef _WIN64

ALIGNED BOOLEAN __cdecl Secure_CheckElevation64(
    PVOID* pProxyInfo, ULONG nProcNum, void* pReturnValue, va_list vl)
{
    void* ReturnAddress = NULL;
    void* pStubDescriptor = NULL;
    __try {
        ReturnAddress = *(__int64**)(vl - (4 * 8));
        pStubDescriptor = *pProxyInfo;
    } __except (EXCEPTION_EXECUTE_HANDLER) {}

    return Secure_CheckElevation(ReturnAddress, pStubDescriptor, NULL, vl);
}

#endif _WIN64

extern BOOLEAN g_rpc_client_hooks;
void RpcRt_NdrClientCallX(const WCHAR* Function, void* ReturnAddress, VOID* pStubDescriptor);

ALIGNED BOOLEAN __cdecl Secure_CheckElevation(
    void *ReturnAddressFromNdrAsyncClientCall,
    void *pStubDescriptor, void *pFormat, SECURE_UAC_ARGS *Args)
{
    if (g_rpc_client_hooks) {
#ifdef _WIN64
        RpcRt_NdrClientCallX(L"Ndr64AsyncClientCall", ReturnAddressFromNdrAsyncClientCall, pStubDescriptor);
#else
        RpcRt_NdrClientCallX(L"NdrAsyncClientCall", ReturnAddressFromNdrAsyncClientCall, pStubDescriptor);
#endif
    }

    static UCHAR elevation_binding_1[16] = {
        0x9A, 0xF9, 0x1E, 0x20, 0xA0, 0x7F, 0x4C, 0x44,
        0x93, 0x99, 0x19, 0xBA, 0x84, 0xF1, 0x2A, 0x1A };
    static UCHAR elevation_binding_2_Vista[16] = {
        0x7D, 0xCE, 0x54, 0x5F, 0x79, 0x5B, 0x75, 0x41,
        0x85, 0x84, 0xCB, 0x65, 0x31, 0x3A, 0x0E, 0x98 };
    static UCHAR elevation_binding_2_Win7[16] = {
        0x23, 0x05, 0x7A, 0xFD, 0x70, 0xDC, 0xDD, 0x43,
        0x9B, 0x2E, 0x9C, 0x5E, 0xD4, 0x82, 0x25, 0xB1 };
    const ULONG RpcNotificationTypeEvent = 1;

#ifdef _WIN64
    const SIZEOF_RPC_ASYNC_STATE =  0x70;
    const OFFSET_OF_BINDING_GUID =  0x18;
#else
    const SIZEOF_RPC_ASYNC_STATE =  0x44;
    const OFFSET_OF_BINDING_GUID =  0x10;
#endif _WIN64

    RPC_ASYNC_STATE *AsyncState;
    UCHAR *ptr;

    BOOLEAN ok = FALSE;

    __try {

        //
        // elevation hook disabled?
        //

        if (Secure_Elevation_HookDisabled)
            __leave;

        //
        // elevation already in progress?
        //

        if (Secure_Elevation_AsyncState)
            __leave;

        //
        // recognize that NdrAsyncClientCall is used to contact the
        // Application Information (AppInfo) service for a UAC request:
        //

        if (! Args)
            __leave;

        //
        // confirm RPC_ASYNC_STATE structure with size 0x44 / 0x70,
        // notification type == event, and a handle to an event
        //

        AsyncState = Args->AsyncState;
        if (! AsyncState)
            __leave;
        if (AsyncState->Size != SIZEOF_RPC_ASYNC_STATE)
            __leave;
        if (AsyncState->NotificationType != RpcNotificationTypeEvent)
            __leave;
        if (! AsyncState->hEvent)
            __leave;

        //
        // confirm a binding handle that is the result of calling
        // RpcBindingFromStringBinding with the string parameter:
        // 201ef99a-7fa0-444c-9399-19ba84f12a1a@ncalrpc
        // or
        // 5f54ce7d-5b79-4175-8584-cb65313a0e98@ncalrpc
        //

#define IS_BINDING_GUID(n) \
    (0 == memcmp(ptr + OFFSET_OF_BINDING_GUID, elevation_binding_##n, 16))

        ptr = (UCHAR *)Args->BindingHandle;
        // The name "BindingHandle" implies a handle.  But the original code treats it as a ptr to mem (see memcmp above).
        // Windows 10 is passing in real handles sometimes.  We need a better solution that this HACK to filter out handles.
        if (! ptr || ptr < (UCHAR*)0x1fff)
            __leave;

        if (IS_BINDING_GUID(1)) {

            //
            // type 1 elevation, Elevate Process:
            // confirm output parameter for process handle
            //

            if (! Args->u.Args1.ProcessHandle)
                __leave;

            Secure_Elevation_Type = 1;

            Secure_Elevation_ResultHandle = Args->u.Args1.ProcessHandle;

        } else if (IS_BINDING_GUID(2_Vista) || IS_BINDING_GUID(2_Win7)) {

            //
            // type 2 elevation, Get Admin Token
            // scan for parameter with value -1, output token follows
            //

            ULONG i;

            for (i = 3; i < 16; ++i) {
                if ((ULONG)Args->u.UnknownParameters[i] == (ULONG)-1) {

                    Secure_Elevation_Type = 2;

                    Secure_Elevation_ResultHandle = (HANDLE *)
                        Args->u.UnknownParameters[i + 1];

                    break;
                }
            }

            if (! Secure_Elevation_Type)
                __leave;

        } else
            __leave;

#undef IS_BINDING_GUID

        //
        // finish
        //

        Secure_Elevation_AsyncState = Args->AsyncState;

        ok = TRUE;

    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }

    return ok;
}


//---------------------------------------------------------------------------
// Secure_HandleElevation
//---------------------------------------------------------------------------


ALIGNED ULONG_PTR __cdecl Secure_HandleElevation(
    void *pStubDescriptor, void *pFormat, SECURE_UAC_ARGS *Args)
{
    WCHAR *ApplicationName, *CommandLine, *CurrentDirectory;
    ULONG pkt_len, app_len, cmd_len, dir_len;
    SECURE_UAC_PACKET *pkt;
    WCHAR *ptr;
    SERVICE_UAC_REQ req;
    MSG_HEADER *rpl;

    //
    // Secure_CheckElevation determined this is an elevation call,
    // build a request packet for Sandboxie Service :: Service Server
    //

    if (Secure_Elevation_Type == 1) {

        ApplicationName  = Args->u.Args1.ApplicationName;
        CommandLine      = Args->u.Args1.CommandLine;
        CurrentDirectory = Args->u.Args1.CurrentDirectory;


    } else if (Secure_Elevation_Type == 2) {

        ApplicationName  = L"*MSI*";
        CommandLine      = ApplicationName;
        CurrentDirectory = ApplicationName;

    } else
        return 0;

    //
    // prepare elevation packet
    //

    app_len = wcslen(ApplicationName);
    if (app_len > 1024)
        app_len = 1024;

    cmd_len = wcslen(CommandLine);
    if (cmd_len > 1024)
        cmd_len = 1024;

    dir_len = wcslen(CurrentDirectory);
    if (dir_len > 1024)
        dir_len = 1024;

    pkt_len = sizeof(SECURE_UAC_PACKET)
            + (app_len + cmd_len + dir_len) * sizeof(WCHAR)
            + sizeof(ULONG);

    pkt = Dll_Alloc(pkt_len);

    pkt->tzuk = tzuk;
    pkt->len = pkt_len;
    pkt->inv_len = ~pkt->len;

    pkt->hEvent = (ULONG64)(ULONG_PTR)Secure_Elevation_AsyncState->hEvent;
    pkt->hResult = 0;
    pkt->ret_code = ERROR_CANCELLED;

    ptr = pkt->text;

    pkt->app_len = app_len;
    pkt->app_ofs = (ULONG)((UCHAR *)ptr - (UCHAR *)pkt);
    wmemcpy(ptr, ApplicationName, app_len);

    ptr += app_len;

    pkt->cmd_len = cmd_len;
    pkt->cmd_ofs = (ULONG)((UCHAR *)ptr - (UCHAR *)pkt);
    wmemcpy(ptr, CommandLine, cmd_len);

    ptr += cmd_len;

    pkt->dir_len = dir_len;
    pkt->dir_ofs = (ULONG)((UCHAR *)ptr - (UCHAR *)pkt);
    wmemcpy(ptr, CurrentDirectory, dir_len);

    ptr += dir_len;

    *(ULONG *)ptr = tzuk;

    //
    // keep parameters for later
    //

    Secure_Elevation_Packet = pkt;

    //
    // signal Sandboxie Service
    // processing continues in SbieSvc::ServiceServer::UacHandler
    //

    req.h.length = sizeof(SERVICE_UAC_REQ);
    req.h.msgid = MSGID_SERVICE_UAC;

    req.devmap[0] = L'\0';
    File_GetSetDeviceMap(req.devmap);

    req.uac_pkt_addr = (ULONG64)(ULONG_PTR)pkt;
    req.uac_pkt_len  = pkt_len;

    Gui_AllowSetForegroundWindow();

    rpl = SbieDll_CallServer(&req.h);

    if ((! rpl) || (rpl->status != 0))
        SetEvent((HANDLE)pkt->hEvent);

    if (rpl)
        Dll_Free(rpl);

    return 0;
}


//---------------------------------------------------------------------------
// Secure_RpcAsyncCompleteCall
//---------------------------------------------------------------------------


ALIGNED ULONG_PTR Secure_RpcAsyncCompleteCall(
    RPC_ASYNC_STATE *AsyncState, void *Reply)
{
    if (AsyncState != Secure_Elevation_AsyncState)
        return __sys_RpcAsyncCompleteCall(AsyncState, Reply);

    //
    // Start.exe posted the event referenced by pkt->hEvent (actually
    // AsyncState->hEvent), and used cross-memory calls to update
    // pkt->hProcess and pkt->ret_code, which we can now return to caller
    //

    *Secure_Elevation_ResultHandle =
        (HANDLE)(ULONG_PTR)Secure_Elevation_Packet->hResult;
    *(ULONG *)Reply = (ULONG)Secure_Elevation_Packet->ret_code;

    Dll_Free(Secure_Elevation_Packet);

    Secure_Elevation_Packet = NULL;
    Secure_Elevation_ResultHandle = NULL;
    Secure_Elevation_AsyncState = NULL;
    Secure_Elevation_Type = 0;

    return 0;
}


//---------------------------------------------------------------------------
// SbieDll_DisableElevationHook
//---------------------------------------------------------------------------


_FX void SbieDll_DisableElevationHook(void)
{
    Secure_Elevation_HookDisabled = TRUE;
}


//---------------------------------------------------------------------------
// SbieDll_GetPublicSecurityDescriptor
//---------------------------------------------------------------------------


/*_FX const void *SbieDll_GetPublicSecurityDescriptor(void)
{
    if (! Secure_EveryoneSD)
        Secure_InitSecurityDescriptors();
    return Secure_EveryoneSD;
}*/
