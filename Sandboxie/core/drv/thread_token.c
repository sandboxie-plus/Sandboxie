/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020 David Xanatos, xanasoft.com
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
// Thread Management (Security Tokens)
//---------------------------------------------------------------------------


#include "common/my_version.h"
#include "conf.h"

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#define ProcessAccessTokenEx  93        //Added for Windows 10 RS5


#define TOKEN_DENIED_ACCESS_MASK                                \
        ~(  STANDARD_RIGHTS_READ | GENERIC_READ | SYNCHRONIZE | \
            TOKEN_QUERY | TOKEN_QUERY_SOURCE)


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static NTSTATUS Thread_OpenProcessToken(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args);

static NTSTATUS Thread_OpenProcessTokenEx(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args);

static NTSTATUS Thread_OpenProcessToken_Common(
    PROCESS *proc, HANDLE ProcessHandle, ACCESS_MASK DesiredAccess,
    HANDLE *TokenHandle);

static NTSTATUS Thread_SetInformationProcess(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args);

static NTSTATUS Thread_SetInformationProcess_PrimaryToken(
    PROCESS *proc, HANDLE ProcessHandle, void *InfoBuffer, ULONG InfoLength);

static NTSTATUS Thread_SetInformationProcess_PrimaryToken_2(
    PROCESS *proc, void *ProcessObject, void *TokenObject, ULONG SessionId);

static void *Thread_SetInformationProcess_PrimaryToken_3(
    PROCESS *proc, void *TokenObject1, ULONG SessionId);


//---------------------------------------------------------------------------


static NTSTATUS Thread_OpenThreadToken(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args);

static NTSTATUS Thread_OpenThreadTokenEx(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args);

static NTSTATUS Thread_OpenThreadToken_Common(
    PROCESS *proc, HANDLE ThreadHandle, ACCESS_MASK DesiredAccess,
    BOOLEAN OpenAsSelf, HANDLE *TokenHandle);

static NTSTATUS Thread_OpenThreadToken_OpenHandle(
    void *TokenObject, BOOLEAN CopyOnOpen, BOOLEAN EffectiveOnly,
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    ACCESS_MASK DesiredAccess, HANDLE *NewHandle,
    HANDLE OpenAsSelf_PrimaryTokenObject);

static NTSTATUS Thread_OpenThreadToken_SwitchToken(
    void *PrimaryTokenObject, void **TokenObject,
    BOOLEAN *CopyOnOpen, BOOLEAN *EffectiveOnly,
    SECURITY_IMPERSONATION_LEVEL *ImpersonationLevel);

static _FX NTSTATUS Thread_SetInformationThread(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args);

static NTSTATUS Thread_SetInformationThread_ImpersonationToken(
    PROCESS *proc, HANDLE ThreadHandle, void *InfoBuffer, ULONG InfoLength);

static NTSTATUS Thread_CheckTokenForImpersonation(
    void *TokenObject, BOOLEAN CheckTokenType,
	PROCESS *proc);

static NTSTATUS Thread_SetInformationThread_ChangeNotifyToken(PROCESS *proc);

static NTSTATUS Thread_ImpersonateAnonymousToken(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args);


//---------------------------------------------------------------------------


NTSTATUS Thread_GetKernelHandleForUserHandle(
    HANDLE *OutKernelHandle, HANDLE InUserHandle);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


void *Thread_AnonymousToken = NULL;


//---------------------------------------------------------------------------
// Thread_OpenProcessToken
//---------------------------------------------------------------------------


_FX NTSTATUS Thread_OpenProcessToken(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args)
{
    NTSTATUS status;

    if (! proc->primary_token) {

        status = Syscall_Invoke(syscall_entry, user_args);

    } else {

        HANDLE ProcessHandle = (HANDLE *)user_args[0];
        ACCESS_MASK DesiredAccess = (ACCESS_MASK)user_args[1];
        HANDLE *TokenHandle = (HANDLE *)user_args[2];

        status = Thread_OpenProcessToken_Common(
                        proc, ProcessHandle, DesiredAccess, TokenHandle);
    }

    return status;
}


//---------------------------------------------------------------------------
// Thread_OpenProcessTokenEx
//---------------------------------------------------------------------------


_FX NTSTATUS Thread_OpenProcessTokenEx(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args)
{
    NTSTATUS status;

    if (! proc->primary_token) {

        status = Syscall_Invoke(syscall_entry, user_args);

    } else {

        HANDLE ProcessHandle = (HANDLE *)user_args[0];
        ACCESS_MASK DesiredAccess = (ACCESS_MASK)user_args[1];
        // HandleAttributes = user_args[2]
        HANDLE *TokenHandle = (HANDLE *)user_args[3];

        status = Thread_OpenProcessToken_Common(
                        proc, ProcessHandle, DesiredAccess, TokenHandle);
    }

    return status;
}


//---------------------------------------------------------------------------
// Thread_OpenProcessToken_Common
//---------------------------------------------------------------------------


_FX NTSTATUS Thread_OpenProcessToken_Common(
    PROCESS *proc, HANDLE ProcessHandle, ACCESS_MASK DesiredAccess,
    HANDLE *TokenHandle)
{
    ACCESS_MASK _ProcessAccess;
    void *ProcessObject;
    PROCESS *proc2;
    HANDLE MyTokenHandle;
    NTSTATUS status;
    KIRQL irql;

    //
    // syscall handler for NtOpenProcessToken and NtOpenProcessTokenEx
    //
    // if the target process is in the same sandbox as the caller,
    // we open a handle to the unrestricted primary token object saved
    // in our process structure
    //
    // if the target process is not in the same sandbox, we only allow
    // query access rights, and use NtOpenProcessTokenEx to open the
    // target process
    //
    // note that NtOpenProcessTokenEx is a version of NtOpenProcessToken
    // which also accepts a HandleAttributes parameters which is used to
    // specify an OBJ_KERNEL_HANDLE attribute.  but our caller is always
    // a user mode process so we can always just use NtOpenProcessToken
    //

    __try {
        ProbeForWrite(TokenHandle, sizeof(HANDLE), sizeof(UCHAR));
        status = STATUS_SUCCESS;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }
    if (! NT_SUCCESS(status))
        return status;

    //
    // open the specified process
    //

    _ProcessAccess  = (Driver_OsVersion >= DRIVER_WINDOWS_VISTA)
                    ? PROCESS_QUERY_LIMITED_INFORMATION
                    : PROCESS_QUERY_INFORMATION;

    status = ObReferenceObjectByHandle(
                ProcessHandle, _ProcessAccess, *PsProcessType, UserMode,
                &ProcessObject, NULL);

    if (! NT_SUCCESS(status))
        return status;

    //
    // open the token for the process
    //

    proc2 = Thread_FindAndInitProcess(proc, ProcessObject, &irql);

    if (proc2 && proc2->primary_token) {

        //
        // both caller and target processes are fully sandboxed and
        // restricted, so open a handle to the the primary token object
        // which was stored in our process structure
        //

        void *PrimaryTokenObject = proc2->primary_token;
        ObReferenceObject(PrimaryTokenObject);

        ExReleaseResourceLite(Process_ListLock);
        KeLowerIrql(irql);

        status = ObOpenObjectByPointer(
                    PrimaryTokenObject, 0, NULL, DesiredAccess,
                    *SeTokenObjectType, UserMode, &MyTokenHandle);

        ObDereferenceObject(PrimaryTokenObject);

    } else {

        //
        // the target process does not have a primary token in its
        // process structure or is not in the same sandbox as the
        // caller process, so ask the system to open the token
        //

        ExReleaseResourceLite(Process_ListLock);
        KeLowerIrql(irql);

        MyTokenHandle = NULL;

        if ((! proc2) && (DesiredAccess & TOKEN_DENIED_ACCESS_MASK)) {

            // deny access to token of process outside the sandbox
            status = STATUS_ACCESS_DENIED;

        } else {

            HANDLE ProcessKernelHandle;
            status = Thread_GetKernelHandleForUserHandle(
                                        &ProcessKernelHandle, ProcessHandle);
            if (NT_SUCCESS(status)) {

                status = ZwOpenProcessToken(ProcessKernelHandle, DesiredAccess,
                                            &MyTokenHandle);

                ZwClose(ProcessKernelHandle);
            }
        }
    }

    //
    // finish
    //

    ObDereferenceObject(ProcessObject);

    if (NT_SUCCESS(status) && MyTokenHandle) {

        __try {
            *TokenHandle = MyTokenHandle;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
        }
    }

    return status;
}


//---------------------------------------------------------------------------
// Thread_SetInformationProcess
//---------------------------------------------------------------------------


_FX NTSTATUS Thread_SetInformationProcess(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args)
{
    NTSTATUS status;
    //Windows RS5 adds a new "undocumented" Process Information class: 0x5d (93) that is likely ProcessAccessTokenEx
    if (((user_args[1] == ProcessAccessToken) || (user_args[1] == ProcessAccessTokenEx)) && proc->primary_token) {
        HANDLE ProcessHandle = (HANDLE)user_args[0];
        void  *InfoBuffer    = (void *)user_args[2];
        ULONG  InfoLength    = (ULONG)user_args[3];

        status = Thread_SetInformationProcess_PrimaryToken(
                        proc, ProcessHandle, InfoBuffer, InfoLength);

    } else {

        status = Syscall_Invoke(syscall_entry, user_args);
    }

    return status;
}


//---------------------------------------------------------------------------
// Thread_SetInformationProcess_PrimaryToken
//---------------------------------------------------------------------------


_FX NTSTATUS Thread_SetInformationProcess_PrimaryToken(
    PROCESS *proc, HANDLE ProcessHandle, void *InfoBuffer, ULONG InfoLength)
{
    void *ProcessObject;
    PROCESS *proc2;
    NTSTATUS status;
    KIRQL irql;
    BOOLEAN SpecialCallFromSbieDll;

    //
    // syscall handler for NtSetInformationProcess with info class
    // ProcessAccessToken, i.e. replace the primary token for a
    // process
    //
    // if the target process is in the same sandbox as the caller,
    // and was not initialized yet, then we copy the primary token
    // from the current process into the target process
    //
    // note that the primary token is restricted and replaced during
    // process initialization (when Process_NotifyImage calls
    // Token_ReplacePrimary), and we must not allow the primary token
    // to change after this point
    //

    SpecialCallFromSbieDll = FALSE;
    __try {
        status = STATUS_SUCCESS;
        if (InfoLength == sizeof(PROCESS_ACCESS_TOKEN)) {
            PROCESS_ACCESS_TOKEN *Info = (PROCESS_ACCESS_TOKEN *)InfoBuffer;
            ProbeForRead(InfoBuffer, InfoLength, sizeof(UCHAR));
            if ((! Info->Token) && (! Info->Thread))
                SpecialCallFromSbieDll = TRUE;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }
    if (! NT_SUCCESS(status))
        return status;

    //
    // open the specified process
    //

    status = ObReferenceObjectByHandle(
                ProcessHandle, PROCESS_SET_INFORMATION,
                *PsProcessType, UserMode, &ProcessObject, NULL);
    if (! NT_SUCCESS(status))
        return status;

    proc2 = Thread_FindAndInitProcess(proc, ProcessObject, &irql);

    if (proc2 && (! proc2->initialized)) {

        ULONG SessionId = proc->box->session_id;

        void *PrimaryTokenObject = proc->primary_token;
        ObReferenceObject(PrimaryTokenObject);

        ExReleaseResourceLite(Process_ListLock);
        KeLowerIrql(irql);

        //
        // a process in the sandbox should only issue NtSetInformationProcess
        // with ProcessAccessToken as part of our CreateProcessInternalW hook
        // in SbieDll, and in this case, InfoBuffer should contain zeroes,
        // and SpecialCallFromSbieDll will be set by the code sequenece above
        //

        if (! SpecialCallFromSbieDll) {

            status = STATUS_ACCESS_DENIED;

        } else {

            //
            // the special call from SbieDll is a request to copy the active
            // impersonation token into the new process as its primary token,
            // or if no impersonation, copy the original unrestricted primary
            // token of this parent process into the new child process
            //
            // note that once the child begins to execute, control will reach
            // Token_ReplacePrimary and it will restrict the primary token
            //

            status = Thread_SetInformationProcess_PrimaryToken_2(
                proc, ProcessObject, PrimaryTokenObject, SessionId);
        }

        ObDereferenceObject(PrimaryTokenObject);

    } else {

        ExReleaseResourceLite(Process_ListLock);
        KeLowerIrql(irql);

        status = STATUS_ACCESS_DENIED;
    }

    //
    // finish
    //

    ObDereferenceObject(ProcessObject);

    return status;
}


//---------------------------------------------------------------------------
// Thread_SetInformationProcess_PrimaryToken_2
//---------------------------------------------------------------------------


_FX NTSTATUS Thread_SetInformationProcess_PrimaryToken_2(
    PROCESS *proc, void *ProcessObject, void *TokenObject, ULONG SessionId)
{
    HANDLE OldTokenHandle;
    HANDLE NewTokenHandle;
    HANDLE NewTokenKernelHandle;
    void *TokenObject2;
    NTSTATUS status;

    //
    // get the token which the caller wants us to pass to the new process
    //

    TokenObject2 = Thread_SetInformationProcess_PrimaryToken_3(
                                                proc, TokenObject, SessionId);

    if (TokenObject2 == (void *)-1)
        return STATUS_PRIVILEGE_NOT_HELD;

    if (TokenObject2)
        TokenObject = TokenObject2;

    //
    // open the primary token recorded in our process structure
    // for this process and duplicate it
    //

    status = ObOpenObjectByPointer(
                TokenObject, OBJ_KERNEL_HANDLE, NULL, TOKEN_ALL_ACCESS,
                *SeTokenObjectType, KernelMode, &OldTokenHandle);

    if (! NT_SUCCESS(status))
        return status;

    status = ZwDuplicateToken(OldTokenHandle, TOKEN_ALL_ACCESS, NULL,
                              FALSE, TokenPrimary, &NewTokenHandle);

    if (NT_SUCCESS(status)) {

        //
        // get a kernel handle for the duplicate token object
        //

        status = Thread_GetKernelHandleForUserHandle(
                                &NewTokenKernelHandle, NewTokenHandle);

        if (NT_SUCCESS(status)) {

            status = Token_AssignPrimaryHandle(
                        ProcessObject, NewTokenKernelHandle, SessionId);

            ZwClose(NewTokenKernelHandle);
        }

        NtClose(NewTokenHandle);

        // DbgPrint("Token Handle = %p Kernel Handle = %p\n", NewTokenHandle, NewTokenKernelHandle);
    }

    ZwClose(OldTokenHandle);

    if (TokenObject2)
        ObDereferenceObject(TokenObject2);

    return status;
}


//---------------------------------------------------------------------------
// Thread_SetInformationProcess_PrimaryToken_3
//---------------------------------------------------------------------------


_FX void *Thread_SetInformationProcess_PrimaryToken_3(
    PROCESS *proc, void *TokenObject1, ULONG SessionId)
{
    void *TokenObject2;
    ULONG TokenId_offset = 0;
    ULONG ParentTokenId_offset = 0;
    BOOLEAN CopyOnOpen;
    BOOLEAN EffectiveOnly;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;

    //
    // the caller may specify the token to pass to the child process via
    // impersonation of that token.  in this case we want to emulate
    // Windows behavior to prevent a non-privileged parent process from
    // passing an arbitrary token to the child process:
    //
    // the parent token id of the new token must match the token id of
    // the parent process, indicating that the new token was created by
    // restricting the parent token;
    //
    // or the parent token id of the new token must match the parent
    // token id of the parent process, indicating that the new token was
    // duplicated from the parent token.
    //
    // note that the TokenObject1 parameter is the primary token for the
    // parent process.
    //
    // note also that a privileged parent (SeAssignPrimaryTokenPrivilege)
    // would use the "legitimate" NtSetInformationProcess case (see
    // Thread_SetInformationProcess_PrimaryToken), which means  we don't
    // have to check for SeAssignPrimaryTokenPrivilege here
    //

    TokenObject2 = PsReferenceImpersonationToken(PsGetCurrentThread(),
                        &CopyOnOpen, &EffectiveOnly, &ImpersonationLevel);
    if (! TokenObject2) {
        //
        // caller is not impersonating so we can just pass its primary token
        // to the new process without having to do any checks on the token
        //
        return NULL;
    }

    if (TokenObject2 == TokenObject1) {
        ObDereferenceObject(TokenObject2);
        return NULL;
    }

    //
    // get offset to token id fields
    //

    if (Driver_OsVersion <= DRIVER_WINDOWS_10) {

        TokenId_offset          = 0x10;
        ParentTokenId_offset    = 0x20;  //good for windows 10 - 10041
    }

    if (! TokenId_offset)
        Log_Status(MSG_1222, 0x63, STATUS_UNKNOWN_REVISION);

    //
    // compare parent id in new token with token id in primary token
    //

    if (RtlEqualLuid(
                (LUID *)((ULONG_PTR)TokenObject1 + TokenId_offset),
                (LUID *)((ULONG_PTR)TokenObject2 + ParentTokenId_offset))) {

        //
        // TokenObject2.ParentId == TokenObject1.TokenId
        //

        return TokenObject2;
    }

    if (RtlEqualLuid(
                (LUID *)((ULONG_PTR)TokenObject1 + ParentTokenId_offset),
                (LUID *)((ULONG_PTR)TokenObject2 + ParentTokenId_offset))) {

        //
        // TokenObject2.ParentId == TokenObject1.ParentId
        //

        return TokenObject2;
    }

    //
    // if the caller has SeAssignPrimaryTokenPrivilege then we permit
    // assignment of any arbitrary token
    //

    if (    Token_CheckPrivilege(
                TokenObject1, SE_ASSIGNPRIMARYTOKEN_PRIVILEGE, SessionId)
        ||  Token_CheckPrivilege(
                TokenObject2, SE_ASSIGNPRIMARYTOKEN_PRIVILEGE, SessionId)) {

        return TokenObject2;
    }

    //
    // permit also if the caller is SandboxieDcomLaunch, which is running
    // without system privileges (i.e. no SeAssignPrimaryTokenPrivilege)
    //

    if (proc->image_sbie &&
            _wcsicmp(proc->image_name, SANDBOXIE L"DcomLaunch.exe") == 0) {

        return TokenObject2;
    }

    //
    // special allowance for MSIServer running without system privileges
    //

    if (!proc->image_from_box &&
        _wcsicmp(proc->image_name, L"msiexec.exe") == 0) {

        return TokenObject2;
    }

    //
    // otherwise, deny request to set an arbitrary token
    //

    ObDereferenceObject(TokenObject2);
    return (void *)-1;
}


//---------------------------------------------------------------------------
// Thread_OpenThreadToken
//---------------------------------------------------------------------------


_FX NTSTATUS Thread_OpenThreadToken(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args)
{
    NTSTATUS status;

    if (! proc->primary_token) {

        status = Syscall_Invoke(syscall_entry, user_args);

    } else {

        HANDLE ThreadHandle = (HANDLE *)user_args[0];
        ACCESS_MASK DesiredAccess = (ACCESS_MASK)user_args[1];
        BOOLEAN OpenAsSelf = (BOOLEAN)user_args[2];
        HANDLE *TokenHandle = (HANDLE *)user_args[3];

        status = Thread_OpenThreadToken_Common(
                proc, ThreadHandle, DesiredAccess, OpenAsSelf, TokenHandle);
    }

    return status;
}


//---------------------------------------------------------------------------
// Thread_OpenThreadTokenEx
//---------------------------------------------------------------------------


_FX NTSTATUS Thread_OpenThreadTokenEx(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args)
{
    NTSTATUS status;

    if (! proc->primary_token) {

        status = Syscall_Invoke(syscall_entry, user_args);

    } else {

        HANDLE ThreadHandle = (HANDLE *)user_args[0];
        ACCESS_MASK DesiredAccess = (ACCESS_MASK)user_args[1];
        BOOLEAN OpenAsSelf = (BOOLEAN)user_args[2];
        // HandleAttributes = user_args[3]
        HANDLE *TokenHandle = (HANDLE *)user_args[4];

        status = Thread_OpenThreadToken_Common(
                proc, ThreadHandle, DesiredAccess, OpenAsSelf, TokenHandle);
    }

    return status;
}


//---------------------------------------------------------------------------
// Thread_OpenThreadToken_Common
//---------------------------------------------------------------------------


_FX NTSTATUS Thread_OpenThreadToken_Common(
    PROCESS *proc, HANDLE ThreadHandle, ACCESS_MASK DesiredAccess,
    BOOLEAN OpenAsSelf, HANDLE *TokenHandle)
{
    ACCESS_MASK _ThreadAccess;
    void *TokenObject;
    void *ThreadObject;
    void *ProcessObject;
    void *PrimaryTokenObject;
    PROCESS *proc2;
    THREAD *thrd2;
    HANDLE MyTokenHandle;
    NTSTATUS status, status2;
    KIRQL irql;
    BOOLEAN CopyOnOpen;
    BOOLEAN EffectiveOnly;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;

    //
    // syscall handler for NtOpenThreadToken and NtOpenThreadTokenEx
    //
    // if the target process is in the same sandbox as the caller,
    // we open a handle to the unrestricted primary token object saved
    // in our process structure
    //
    // if the target process is not in the same sandbox, we only allow
    // query access rights, and use NtOpenProcessTokenEx to open the
    // target process
    //
    // note that NtOpenThreadTokenEx is a version of NtOpenThreadToken
    // which also accepts a HandleAttributes parameters which is used to
    // specify an OBJ_KERNEL_HANDLE attribute.  but our caller is always
    // a user mode process so we can always just use NtOpenThreadToken
    //

    __try {
        ProbeForWrite(TokenHandle, sizeof(HANDLE), sizeof(UCHAR));
        status = STATUS_SUCCESS;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }
    if (! NT_SUCCESS(status))
        return status;

    //
    // the OpenAsSelf says the caller wants to use process primary token
    // when opening the specified thread, instead of the currently active
    // impersonation token.  our real primary token is restricted so if
    // OpenAsSelf is specified, we impersonate the stored primary token
    //

    if (OpenAsSelf) {

        status = Thread_OpenThreadToken_SwitchToken(
                        proc->primary_token, &TokenObject,
                        &CopyOnOpen, &EffectiveOnly, &ImpersonationLevel);

        if (! NT_SUCCESS(status))
            return status;
    }

    //
    // open the selected thread and restore the previous impersonation
    //

    _ThreadAccess   = (Driver_OsVersion >= DRIVER_WINDOWS_VISTA)
                    ? THREAD_QUERY_LIMITED_INFORMATION
                    : THREAD_QUERY_INFORMATION;

    status = ObReferenceObjectByHandle(
                ThreadHandle, _ThreadAccess, *PsThreadType, UserMode,
                &ThreadObject, NULL);

    if (OpenAsSelf) {

        status2 = Thread_OpenThreadToken_SwitchToken(
                        NULL, &TokenObject,
                        &CopyOnOpen, &EffectiveOnly, &ImpersonationLevel);

        if (NT_SUCCESS(status) && (! NT_SUCCESS(status2))) {

            ObDereferenceObject(ThreadObject);
            return status2;
        }
    }

    if (! NT_SUCCESS(status))
        return status;

    //
    // open the token for the thread
    //

    ProcessObject = PsGetThreadProcess(ThreadObject);

    proc2 = Thread_FindAndInitProcess(proc, ProcessObject, &irql);

    MyTokenHandle = NULL;

    PrimaryTokenObject = proc->primary_token;
    ObReferenceObject(PrimaryTokenObject);

    if (proc2) {

        //
        // both caller and target processes are fully sandboxed and
        // restricted, so open a handle to the the impersonation token
        // object which was stored in our thread structure (if any)
        //

        ExAcquireResourceExclusiveLite(proc->threads_lock, TRUE);

        thrd2 = Thread_GetOrCreate(
                            proc2, PsGetThreadId(ThreadObject), FALSE);
        if (thrd2) {

            TokenObject = thrd2->token_object;
            CopyOnOpen = thrd2->token_CopyOnOpen;
            EffectiveOnly = thrd2->token_EffectiveOnly;
            ImpersonationLevel = thrd2->token_ImpersonationLevel;

            if (TokenObject)
                ObReferenceObject(TokenObject);

        } else
            TokenObject = NULL;

        ExReleaseResourceLite(proc->threads_lock);

        ExReleaseResourceLite(Process_ListLock);
        KeLowerIrql(irql);

        if (! TokenObject) {

            status = STATUS_NO_TOKEN;

        } else {

            status = Thread_OpenThreadToken_OpenHandle(
                TokenObject,  CopyOnOpen, EffectiveOnly, ImpersonationLevel,
                DesiredAccess, &MyTokenHandle,
                (OpenAsSelf ? PrimaryTokenObject : NULL));

            ObDereferenceObject(TokenObject);
        }

    } else {

        //
        // the target process does not have an impersonation token in
        // its thread structure or is not in the same sandbox as the
        // caller process, so ask the system to open the token
        //

        ExReleaseResourceLite(Process_ListLock);
        KeLowerIrql(irql);

        if (DesiredAccess & TOKEN_DENIED_ACCESS_MASK) {

            status = STATUS_ACCESS_DENIED;

        } else {

            //
            // the caller may want to use the primary token to open the
            // target impersonation token, but we can't pass the OpenAsSelf
            // flag to the system because our primary token is restricted,
            // so impersonate the full primary token here instead
            //

            if (OpenAsSelf) {

                status = Thread_OpenThreadToken_SwitchToken(
                        PrimaryTokenObject, &TokenObject,
                        &CopyOnOpen, &EffectiveOnly, &ImpersonationLevel);
            }

            if (NT_SUCCESS(status)) {

                HANDLE ThreadKernelHandle;
                status = Thread_GetKernelHandleForUserHandle(
                                        &ThreadKernelHandle, ThreadHandle);

                if (NT_SUCCESS(status)) {

                    status = ZwOpenThreadToken(
                                    ThreadKernelHandle, DesiredAccess, FALSE,
                                    &MyTokenHandle);

                    ZwClose(ThreadKernelHandle);
                }
            }

            if (OpenAsSelf) {

                status2 = Thread_OpenThreadToken_SwitchToken(
                        NULL, &TokenObject,
                        &CopyOnOpen, &EffectiveOnly, &ImpersonationLevel);

                if (NT_SUCCESS(status) && (! NT_SUCCESS(status2))) {

                    ZwClose(MyTokenHandle);
                    MyTokenHandle = NULL;
                    status = status2;
                }
            }
        }

    }

    //
    // finish
    //

    ObDereferenceObject(PrimaryTokenObject);

    ObDereferenceObject(ThreadObject);

    if (NT_SUCCESS(status) && MyTokenHandle) {

        __try {
            *TokenHandle = MyTokenHandle;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
        }
    }

    return status;
}


//---------------------------------------------------------------------------
// Thread_OpenThreadToken_OpenHandle
//---------------------------------------------------------------------------


_FX NTSTATUS Thread_OpenThreadToken_OpenHandle(
    void *TokenObject, BOOLEAN CopyOnOpen, BOOLEAN EffectiveOnly,
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    ACCESS_MASK DesiredAccess, HANDLE *NewHandle,
    HANDLE OpenAsSelf_PrimaryTokenObject)
{
    NTSTATUS status;

    //
    // even if impersonation level SecurityImpersonation was specified at
    // the time of impersonation, we probably have a token object that is
    // at SecurityIdentification level, due to the lack of impersonation
    // privileges (as discussed in Thread_MyImpersonateClient).
    //
    // this makes it difficult to emulate the behavior of NtOpenThreadToken
    // for the CopyOnOpen case because ZwDuplicateToken will not let us
    // convert a SecurityIdentification token to a SecurityImpersonation
    // token.  (see the case of SecurityIdentification below for how we
    // use ZwDuplicateToken to honor CopyOnOpen.)
    //
    // we can solve this problem by impersonating the requested token in
    // the current thread with the CopyOnOpen flag, and then asking
    // NtOpenThreadToken to open this token for us
    //
    // note this is only needed for SecurityImpersonation and CopyOnOpen,
    // and that using NtOpenThreadToken on the current thread token is only
    // possible when the token is at SecurityImpersonation level or higher
    //

    if (ImpersonationLevel >= SecurityImpersonation && CopyOnOpen) {

        status = Thread_MyImpersonateClient(PsGetCurrentThread(),
            TokenObject, CopyOnOpen, EffectiveOnly, ImpersonationLevel);

        if (NT_SUCCESS(status)) {

            status = ZwOpenThreadToken(
                        NtCurrentThread(), DesiredAccess, FALSE, NewHandle);
        }

    //
    // an impersonation token at the SecurityAnonymous level can't be opened
    //

    } else if (ImpersonationLevel <= SecurityAnonymous) {

        status = STATUS_CANT_OPEN_ANONYMOUS;

    //
    // otherwise the token is at an impersonation level that is less than
    // SecurityImpersonation, or it is at SecurityImpersonation level but
    // the CopyOnOpen flag is not set
    //
    // in these cases we can open and duplicate handles to the token object
    //

    } else {

        void *TokenObject2;
        BOOLEAN CopyOnOpen2;
        BOOLEAN EffectiveOnly2;
        SECURITY_IMPERSONATION_LEVEL ImpersonationLevel2;

        if (OpenAsSelf_PrimaryTokenObject) {

            status = Thread_OpenThreadToken_SwitchToken(
                    OpenAsSelf_PrimaryTokenObject, &TokenObject2,
                    &CopyOnOpen2, &EffectiveOnly2, &ImpersonationLevel2);

        } else
            status = STATUS_SUCCESS;

        if (NT_SUCCESS(status)) {

            status = ObOpenObjectByPointer(TokenObject, 0, NULL,
                DesiredAccess, *SeTokenObjectType, UserMode, NewHandle);
        }

        if (NT_SUCCESS(status) && CopyOnOpen) {

            //
            // duplicate the token if it specifies copy-on-open
            //

            HANDLE OldHandle = *NewHandle;

            OBJECT_ATTRIBUTES ObjectAttributes;
            SECURITY_QUALITY_OF_SERVICE QualifyOfService;

            memzero(&ObjectAttributes, sizeof(ObjectAttributes));
            memzero(&QualifyOfService, sizeof(QualifyOfService));
            QualifyOfService.Length = sizeof(QualifyOfService);
            QualifyOfService.ImpersonationLevel = ImpersonationLevel;
            ObjectAttributes.Length = sizeof(ObjectAttributes);
            ObjectAttributes.SecurityQualityOfService = &QualifyOfService;

            status = ZwDuplicateToken(
                        OldHandle, DesiredAccess, &ObjectAttributes,
                        EffectiveOnly, TokenImpersonation, NewHandle);

            ZwClose(OldHandle);
        }

        if (OpenAsSelf_PrimaryTokenObject) {

            NTSTATUS status2 = Thread_OpenThreadToken_SwitchToken(
                        NULL, &TokenObject2,
                        &CopyOnOpen2, &EffectiveOnly2, &ImpersonationLevel2);

            if (NT_SUCCESS(status) && (! NT_SUCCESS(status2))) {

                ZwClose(*NewHandle);
                *NewHandle = NULL;
                status = status2;
            }
        }
    }

    return status;
}


//---------------------------------------------------------------------------
// Thread_OpenThreadToken_SwitchToken
//---------------------------------------------------------------------------


_FX NTSTATUS Thread_OpenThreadToken_SwitchToken(
    void *PrimaryTokenObject, void **TokenObject,
    BOOLEAN *CopyOnOpen, BOOLEAN *EffectiveOnly,
    SECURITY_IMPERSONATION_LEVEL *ImpersonationLevel)
{
    NTSTATUS status;

    if (PrimaryTokenObject) {

        //
        // switch the calling thread to the unrestricted primary token
        // that was stored in our process structure
        //

        *TokenObject = PsReferenceImpersonationToken(PsGetCurrentThread(),
                            CopyOnOpen, EffectiveOnly, ImpersonationLevel);

        status = Thread_MyImpersonateClient(PsGetCurrentThread(),
                    PrimaryTokenObject, FALSE, FALSE, SecurityImpersonation);

        if (NT_SUCCESS(status)) {

            if (! *TokenObject) {
                *CopyOnOpen = FALSE;
                *EffectiveOnly = FALSE;
                *ImpersonationLevel = SecurityAnonymous;
            }

        } else {

            if (*TokenObject) {
                ObDereferenceObject(*TokenObject);
                *TokenObject = NULL;
            }
        }

    } else {

        //
        // switch the calling thread back to the impersonation token that
        // was stored in the parameters by the first call to this function
        //

        status = Thread_MyImpersonateClient(PsGetCurrentThread(),
            *TokenObject, *CopyOnOpen, *EffectiveOnly, *ImpersonationLevel);

        if (*TokenObject)
            ObDereferenceObject(*TokenObject);
    }

    return status;
}


//---------------------------------------------------------------------------
// Thread_SetInformationThread
//---------------------------------------------------------------------------


_FX NTSTATUS Thread_SetInformationThread(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args)
{
    NTSTATUS status;

    ULONG InfoClass = (ULONG)user_args[1];

    if (InfoClass == ThreadImpersonationToken && proc->primary_token) {

        HANDLE ThreadHandle  = (HANDLE)user_args[0];
        void  *InfoBuffer    = (void *)user_args[2];
        ULONG  InfoLength    = (ULONG)user_args[3];

        status = Thread_SetInformationThread_ImpersonationToken(
                        proc, ThreadHandle, InfoBuffer, InfoLength);

    } else {

        status = Syscall_Invoke(syscall_entry, user_args);
    }

    return status;
}


//---------------------------------------------------------------------------
// Thread_SetInformationThread_ImpersonationToken
//---------------------------------------------------------------------------


_FX NTSTATUS Thread_SetInformationThread_ImpersonationToken(
    PROCESS *proc, HANDLE ThreadHandle, void *InfoBuffer, ULONG InfoLength)
{
    void *TokenObject;
    void *ThreadObject;
    void *ProcessObject;
    PROCESS *proc2;
    THREAD *thrd2;
    HANDLE MyTokenHandle = NULL;
    NTSTATUS status;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
    BOOLEAN MustCreateThread;
    KIRQL irql;

    //
    // syscall handler for NtSetInformationThread with info class
    // ThreadImpersonationToken, i.e. replace the impersonation token
    // for a thread
    //
    // if the target thread is in the same sandbox as the caller, we
    // replace the token stored in our thread structure for the target
    //

    __try {
        if (InfoLength != sizeof(HANDLE))
            status = STATUS_INFO_LENGTH_MISMATCH;
        else {
            ProbeForRead(InfoBuffer, InfoLength, sizeof(UCHAR));
            MyTokenHandle = *(HANDLE *)InfoBuffer;
            status = STATUS_SUCCESS;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }
    if (! NT_SUCCESS(status))
        return status;

    //
    // check if this is the special request from
    // Gui_ConnectToWindowStationAndDesktop in core/dll/gui.c
    // which is intended to impersonate using a version of the
    // primary process token which includes a change notify privilege
    //

    if (ThreadHandle == NtCurrentThread() && (
                MyTokenHandle == NtCurrentThread()
#ifdef _WIN64
            ||  (ULONG)(ULONG_PTR)MyTokenHandle ==
                            (ULONG)(ULONG_PTR)NtCurrentThread()
#endif _WIN64
                                                )) {
        status = Thread_SetInformationThread_ChangeNotifyToken(proc);
        if (    status == STATUS_THREAD_NOT_IN_PROCESS
             || status == STATUS_ALREADY_COMMITTED) {

            return status;
        }
    }

    //
    // open the selected thread
    //

    status = ObReferenceObjectByHandle(
                ThreadHandle, THREAD_SET_THREAD_TOKEN,
                *PsThreadType, UserMode, &ThreadObject, NULL);

    if (! NT_SUCCESS(status))
        return status;

    //
    // get the token object for the specified token handle
    //

    if (MyTokenHandle) {

        status = ObReferenceObjectByHandle(
                    MyTokenHandle, TOKEN_IMPERSONATE,
                    *SeTokenObjectType, UserMode, &TokenObject, NULL);

        if (NT_SUCCESS(status)) {

            status = Thread_CheckTokenForImpersonation(TokenObject, TRUE, proc);

            if (! NT_SUCCESS(status))
                ObDereferenceObject(TokenObject);
        }

        if (! NT_SUCCESS(status)) {
            ObDereferenceObject(ThreadObject);
            return status;
        }

        ImpersonationLevel = SeTokenImpersonationLevel(TokenObject);

        MustCreateThread = TRUE;

    } else {

        TokenObject = NULL;

        ImpersonationLevel = SecurityAnonymous;

        MustCreateThread = FALSE;
    }

    //
    // locate the target process
    //

    ProcessObject = PsGetThreadProcess(ThreadObject);

    proc2 = Thread_FindAndInitProcess(proc, ProcessObject, &irql);

    if (proc2) {

        //
        // both caller and target processes are fully sandboxed and
        // restricted, so replace the handle to the the impersonation
        // token object in our thread structure
        //
        // note that for a NULL token object (i.e. a RevertToSelf operation)
        // we don't require the creation of a new thread structure if the
        // thread in question did not already have a thread structure
        //

        ExAcquireResourceExclusiveLite(proc->threads_lock, TRUE);

        thrd2 = Thread_GetOrCreate(
                    proc2, PsGetThreadId(ThreadObject), MustCreateThread);
        if (thrd2) {

            TokenObject = InterlockedExchangePointer(
                                        &thrd2->token_object, TokenObject);
            thrd2->token_CopyOnOpen = FALSE;    // PsAssignImpersonationToken
            thrd2->token_EffectiveOnly = FALSE;
            thrd2->token_ImpersonationLevel = ImpersonationLevel;

        } else if (MustCreateThread)
            status = STATUS_INSUFFICIENT_RESOURCES;

        ExReleaseResourceLite(proc->threads_lock);

    } else {

        //
        // the target process is not in the same sandbox as the caller
        // so we deny this request
        //

        status = STATUS_ACCESS_DENIED;
    }

    ExReleaseResourceLite(Process_ListLock);
    KeLowerIrql(irql);

    if (TokenObject)
        ObDereferenceObject(TokenObject);

    ObDereferenceObject(ThreadObject);

    if (status == STATUS_INSUFFICIENT_RESOURCES) {
        Log_Status(MSG_1222, 0x64, status);
        Process_SetTerminated(proc, 13);
    }

    return status;
}


//---------------------------------------------------------------------------
// Thread_CheckTokenForImpersonation
//---------------------------------------------------------------------------


_FX NTSTATUS Thread_CheckTokenForImpersonation(
    void *TokenObject, BOOLEAN CheckTokenType,
	PROCESS *proc)
{
    NTSTATUS status;

    // OriginalToken BEGIN
	if (Conf_Get_Boolean(proc->box->name, L"OriginalToken", 0, FALSE))
		return STATUS_SUCCESS;
	// OriginalToken END
	// OpenToken BEGIN
	if ((Conf_Get_Boolean(proc->box->name, L"OpenToken", 0, FALSE) || Conf_Get_Boolean(proc->box->name, L"UnfilteredToken", 0, FALSE)))
		return STATUS_SUCCESS;
	// OpenToken END

	BOOLEAN DropRights = proc->drop_rights;
	ULONG SessionId = proc->box->session_id;

    //
    // make sure this is an impersonation token
    //

    TOKEN_TYPE TokenType = CheckTokenType
                         ? SeTokenType(TokenObject)
                         : TokenImpersonation;
    if (TokenType != TokenImpersonation)
        status = STATUS_BAD_TOKEN_TYPE;
    else {

        //
        // make sure the new impersonation token doesn't have
        // privileges or group associates that we don't like
        //

        void *FilteredTokenObject =
                        Token_Filter(TokenObject, DropRights, SessionId);

        if (! FilteredTokenObject)
            status = STATUS_ACCESS_DENIED;
        else {

            if (FilteredTokenObject != TokenObject)
                status = STATUS_ACCESS_DENIED;

            else {

                //
                // token passed all checks
                //

                status = STATUS_SUCCESS;
            }

            ObDereferenceObject(FilteredTokenObject);
        }
    }

    return status;
}


//---------------------------------------------------------------------------
// Thread_SetInformationThread_ChangeNotifyToken
//---------------------------------------------------------------------------


_FX NTSTATUS Thread_SetInformationThread_ChangeNotifyToken(PROCESS *proc)
{
    //
    // the call to SetProcessWindowStation by SbieDll during process
    // initialization will fail in a non-zero session because the
    // \Session\N object directory does not grant access to Everyone,
    // and our highly restricted process token does not include the
    // change notify privilege.
    //
    // to work around this, we have a special case where just once,
    // we will return to the caller with an active impersonation
    // thread that does not strip the change notify privilege.
    //
    // note that Syscall_Api_Invoke wants to clear the thread token
    // before returning to the caller, so we use a hack with special
    // status return code, see Syscall_Api_Invoke
    //

    if (! proc->change_notify_token_flag) {

        //
        // we use the thread impersonation which in most cases is the
        // original primary token at this early point in the life of
        // the process.  however, in case the program in the sandbox
        // intentionally started a new process with a restricted token,
        // then the original primary token would be too restricted, but
        // the parent process should have taken care to give our thread
        // a useful impersonation token by this time.
        //

        BOOLEAN CopyOnOpen;
        BOOLEAN EffectiveOnly;
        SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;

        void *FilteredTokenObject;

        void *CurrentToken = PsReferenceImpersonationToken(
                    PsGetCurrentThread(),
                    &CopyOnOpen, &EffectiveOnly, &ImpersonationLevel);

        if (! CurrentToken) {

            CurrentToken = proc->primary_token;
            ObReferenceObject(CurrentToken);
        }

        //
        // run a special filter the selected token which does not
        // remove the privilege for bypass object directory checks
        //

        FilteredTokenObject = Token_Restrict(
                CurrentToken, DISABLE_MAX_PRIVILEGE, NULL,
                proc);

        ObDereferenceObject(CurrentToken);

        if (FilteredTokenObject) {

            NTSTATUS status = Thread_MyImpersonateClient(
                PsGetCurrentThread(), FilteredTokenObject,
                FALSE, FALSE, SecurityImpersonation);

            ObDereferenceObject(FilteredTokenObject);

            if (NT_SUCCESS(status)) {

                //
                // return a nonsensical status code ("thread not found"
                // when ThreadHandle == NtCurrentThread) to indicate to
                // Syscall_Api_Invoke to not do Thread_ClearThreadToken
                //

                proc->change_notify_token_flag = TRUE;

                return STATUS_THREAD_NOT_IN_PROCESS;
            }
        }

    } else
        return STATUS_ALREADY_COMMITTED;

    return STATUS_ACCESS_DENIED;
}


//---------------------------------------------------------------------------
// Thread_InitAnonymousToken
//---------------------------------------------------------------------------


_FX void Thread_InitAnonymousToken(void)
{
    SYSCALL_ENTRY *syscall_entry;
    P_NtImpersonateAnonymousToken pNtImpersonateAnonymousToken;
    void *TokenObject;
    BOOLEAN CopyOnOpen;
    BOOLEAN EffectiveOnly;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
    NTSTATUS status;

    syscall_entry = Syscall_GetByName("ImpersonateAnonymousToken");
    if (! syscall_entry)
        return;

    TokenObject = PsReferenceImpersonationToken(PsGetCurrentThread(),
                    &CopyOnOpen, &EffectiveOnly, &ImpersonationLevel);

    pNtImpersonateAnonymousToken =
            (P_NtImpersonateAnonymousToken)syscall_entry->ntos_func;

    status = pNtImpersonateAnonymousToken(NtCurrentThread());

    if (NT_SUCCESS(status)) {

        BOOLEAN CopyOnOpen2;
        BOOLEAN EffectiveOnly2;
        SECURITY_IMPERSONATION_LEVEL ImpersonationLevel2;

        Thread_AnonymousToken =
                    PsReferenceImpersonationToken(PsGetCurrentThread(),
                    &CopyOnOpen2, &EffectiveOnly2, &ImpersonationLevel2);
    }

    PsImpersonateClient(PsGetCurrentThread(), TokenObject,
                        CopyOnOpen, EffectiveOnly, ImpersonationLevel);
}


//---------------------------------------------------------------------------
// Thread_ImpersonateAnonymousToken
//---------------------------------------------------------------------------


_FX NTSTATUS Thread_ImpersonateAnonymousToken(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args)
{
    NTSTATUS status;

    HANDLE ThreadHandle = (HANDLE *)user_args[0];

    if (proc->primary_token) {

        if (ThreadHandle != NtCurrentThread())
            status = STATUS_ACCESS_DENIED;
        else {

            status = PsImpersonateClient(PsGetCurrentThread(),
                Thread_AnonymousToken, TRUE, FALSE, SecurityImpersonation);

            if (NT_SUCCESS(status)) {

                status = Thread_StoreThreadToken(proc);
            }
        }

    } else {

        P_NtImpersonateAnonymousToken pNtImpersonateAnonymousToken =
            (P_NtImpersonateAnonymousToken)syscall_entry->ntos_func;

        status = pNtImpersonateAnonymousToken(ThreadHandle);
    }

    return status;
}


//---------------------------------------------------------------------------
// Thread_CheckTokenObject
//---------------------------------------------------------------------------


_FX NTSTATUS Thread_CheckTokenObject(
    PROCESS *proc, void *Object, ACCESS_MASK GrantedAccess)
{
    //
    // this function is called from Syscall_DuplicateHandle_2 to check
    // access granted to a token object.  if none of the special permissions
    // were requested, we can immmediately approve the request
    //

    if (! (GrantedAccess & TOKEN_DENIED_ACCESS_MASK))
        return STATUS_SUCCESS;

    //
    // otherwise we want to make sure this token doesn't include any groups
    // or privileges that would have been stripped if the token was filtered
    // by our Token module.  in other words, make sure the token was created
    // by a process in a sandbox
    //

    return Thread_CheckTokenForImpersonation(Object, FALSE, proc);
}


//---------------------------------------------------------------------------
// Thread_GetKernelHandleForUserHandle
//---------------------------------------------------------------------------


_FX NTSTATUS Thread_GetKernelHandleForUserHandle(
    HANDLE *OutKernelHandle, HANDLE InUserHandle)
{
    //
    // on Windows 7, driver verifier doesn't like when we call ZwXxx APIs
    // and pass user mode handles, so this utility function is used to
    // get kernel mode handles that can be passed to the ZwXxx functions
    //

    void *Object;
    NTSTATUS status;

    status = ObReferenceObjectByHandle(
                        InUserHandle, 0, NULL, UserMode, &Object, NULL);

    if (NT_SUCCESS(status)) {

        status = ObOpenObjectByPointer(Object, OBJ_KERNEL_HANDLE,
                                       NULL, 0, NULL, KernelMode,
                                       OutKernelHandle);

        ObDereferenceObject(Object);
    }

    if (! NT_SUCCESS(status))
        *OutKernelHandle = NULL;

    return status;
}
