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
// Thread Management
//---------------------------------------------------------------------------


#include "thread.h"
#include "process.h"
#include "syscall.h"
#include "token.h"
#include "session.h"
#include "api.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define PROCESS_DENIED_ACCESS_MASK                              \
        ~(  STANDARD_RIGHTS_READ | SYNCHRONIZE |                \
            PROCESS_VM_READ | PROCESS_QUERY_INFORMATION |       \
            PROCESS_QUERY_LIMITED_INFORMATION )

#define THREAD_DENIED_ACCESS_MASK                               \
        ~(  STANDARD_RIGHTS_READ | SYNCHRONIZE |                \
            THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION |     \
            THREAD_QUERY_LIMITED_INFORMATION )


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


struct _THREAD {

    LIST_ELEM list_elem;

    HANDLE tid;

    void *token_object;
    BOOLEAN token_CopyOnOpen;
    BOOLEAN token_EffectiveOnly;
    SECURITY_IMPERSONATION_LEVEL token_ImpersonationLevel;

};


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static void Thread_Notify(HANDLE ProcessId, HANDLE ThreadId, BOOLEAN Create);

static PROCESS *Thread_FindAndInitProcess(
    PROCESS *proc1, void *ProcessObject2, KIRQL *out_irql);

static THREAD *Thread_GetCurrent(PROCESS *proc);

static THREAD *Thread_GetOrCreate(PROCESS *proc, HANDLE tid, BOOLEAN create);

static NTSTATUS Thread_MyImpersonateClient(
    PETHREAD ThreadObject, void *TokenObject,
    BOOLEAN CopyOnOpen, BOOLEAN EffectiveOnly,
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel);


//---------------------------------------------------------------------------


static NTSTATUS Thread_CheckProcessObject(
    PROCESS *proc, void *Object, UNICODE_STRING *Name,
    ACCESS_MASK GrantedAccess);

static NTSTATUS Thread_CheckThreadObject(
    PROCESS *proc, void *Object, UNICODE_STRING *Name,
    ACCESS_MASK GrantedAccess);

static NTSTATUS Thread_CheckObject_Common(
    PROCESS *proc, PEPROCESS ProcessObject,
    ACCESS_MASK GrantedAccess, ACCESS_MASK WriteAccess, WCHAR Letter1);


//---------------------------------------------------------------------------


static NTSTATUS Thread_Api_OpenProcess(PROCESS *proc, ULONG64 *parms);

static void Thread_InitAnonymousToken(void);


//---------------------------------------------------------------------------


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, Thread_Init)
#pragma alloc_text (INIT, Thread_InitAnonymousToken)
#endif // ALLOC_PRAGMA


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static BOOLEAN Thread_NotifyInstalled = FALSE;


//---------------------------------------------------------------------------
// Include additional source code
//---------------------------------------------------------------------------


#include "thread_token.c"


//---------------------------------------------------------------------------
// Thread_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Thread_Init(void)
{
    NTSTATUS status;

    //
    // set up the thread notify routine
    //

    status = PsSetCreateThreadNotifyRoutine(Thread_Notify);

    if (NT_SUCCESS(status)) {

        Thread_NotifyInstalled = TRUE;

    } else {

        // too many notify routines are already installed in the system
        Log_Status(MSG_PROCESS_NOTIFY, 0x33, status);
        return FALSE;
    }

    //
    // get an impersonation token for Thread_ImpersonateAnonymousToken
    //

    Thread_InitAnonymousToken();

    //
    // set syscalls handlers
    //

    if (! Syscall_Set1("OpenProcessToken",      Thread_OpenProcessToken))
        return FALSE;
    if (! Syscall_Set1("OpenProcessTokenEx",    Thread_OpenProcessTokenEx))
        return FALSE;

    if (! Syscall_Set1("OpenThreadToken",       Thread_OpenThreadToken))
        return FALSE;
    if (! Syscall_Set1("OpenThreadTokenEx",     Thread_OpenThreadTokenEx))
        return FALSE;

    if (! Syscall_Set1("SetInformationProcess", Thread_SetInformationProcess))
        return FALSE;
    if (! Syscall_Set1("SetInformationThread",  Thread_SetInformationThread))
        return FALSE;

    if (! Syscall_Set1(
                    "ImpersonateAnonymousToken", Thread_ImpersonateAnonymousToken))
        return FALSE;

    //
    // set object open handlers
    //

    if (! Syscall_Set2("OpenProcess",           Thread_CheckProcessObject))
        return FALSE;

    if (! Syscall_Set2("OpenThread",            Thread_CheckThreadObject))
        return FALSE;

    if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA) {

        if (! Syscall_Set2(
                    "AlpcOpenSenderProcess",    Thread_CheckProcessObject))
            return FALSE;

        if (! Syscall_Set2(
                    "AlpcOpenSenderThread",     Thread_CheckThreadObject))
            return FALSE;
    }

    //
    // set API handlers
    //

    Api_SetFunction(API_OPEN_PROCESS,           Thread_Api_OpenProcess);

    return TRUE;
}


//---------------------------------------------------------------------------
// Thread_Unload
//---------------------------------------------------------------------------


_FX void Thread_Unload(void)
{
    if (Thread_NotifyInstalled) {

        PsRemoveCreateThreadNotifyRoutine(Thread_Notify);
        Thread_NotifyInstalled = FALSE;
    }

    if (Thread_AnonymousToken) {
        ObDereferenceObject(Thread_AnonymousToken);
        Thread_AnonymousToken = NULL;
    }
}


//---------------------------------------------------------------------------
// Thread_Notify
//---------------------------------------------------------------------------


_FX void Thread_Notify(HANDLE ProcessId, HANDLE ThreadId, BOOLEAN Create)
{
    void *TokenObject = NULL;
    PROCESS *proc;
    THREAD *thrd;
    KIRQL irql;

    //
    // implement Gui_ThreadModifyCount watchdog hook for gui_xp module
    //

#ifndef _WIN64

    if ((Create) && (Driver_OsVersion < DRIVER_WINDOWS_VISTA)) {

        extern volatile ULONG Gui_ThreadModifyCount;
        InterlockedIncrement(&Gui_ThreadModifyCount);
    }

#endif _WIN64

    //
    //
    //

    proc = Process_Find(ProcessId, &irql);
    if (proc && proc->threads_lock) {

        ExAcquireResourceExclusiveLite(proc->threads_lock, TRUE);

        thrd = List_Head(&proc->threads);
        while (thrd) {
            if (thrd->tid == ThreadId)
                break;
            thrd = List_Next(thrd);
        }

        if (thrd) {

            if (Create) {

                // already have an entry for this thread id, shouldn't happen
                Process_SetTerminated(proc, 10);

            } else {

                TokenObject = InterlockedExchangePointer(
                                            &thrd->token_object, NULL);

                List_Remove(&proc->threads, thrd);
                Mem_Free(thrd, sizeof(THREAD));
            }
        }

        ExReleaseResourceLite(proc->threads_lock);
    }

    ExReleaseResourceLite(Process_ListLock);
    KeLowerIrql(irql);

    if (TokenObject)
        ObDereferenceObject(TokenObject);
}


//---------------------------------------------------------------------------
// Thread_InitProcess
//---------------------------------------------------------------------------


_FX BOOLEAN Thread_InitProcess(PROCESS *proc)
{
    NTSTATUS status = STATUS_SUCCESS;
    BOOLEAN ok;

    if (! proc->threads_lock) {

        List_Init(&proc->threads);

        ok = Mem_GetLockResource(&proc->threads_lock, FALSE);
        if (! ok)
            status = STATUS_INSUFFICIENT_RESOURCES;

        if (! NT_SUCCESS(status)) {

			Log_Status_Ex_Process( MSG_1231, 0x44, status, NULL, proc->box->session_id, proc->pid);
            return FALSE;
        }
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Thread_ReleaseProcess
//---------------------------------------------------------------------------


_FX void Thread_ReleaseProcess(PROCESS *proc)
{
    THREAD *thrd;
    KIRQL irql;

    if (proc->threads_lock) {

        while (1) {

            void *TokenObject = NULL;

            KeRaiseIrql(APC_LEVEL, &irql);
            ExAcquireResourceExclusiveLite(proc->threads_lock, TRUE);

            thrd = List_Head(&proc->threads);
            while (thrd) {

                TokenObject = InterlockedExchangePointer(
                                            &thrd->token_object, NULL);
                if (TokenObject)
                    break;

                thrd = List_Next(thrd);
            }

            ExReleaseResourceLite(proc->threads_lock);
            KeLowerIrql(irql);

            if (! TokenObject)
                break;

            ObDereferenceObject(TokenObject);
        }

        Mem_FreeLockResource(&proc->threads_lock);
    }
}


//---------------------------------------------------------------------------
// Thread_FindAndInitProcess
//---------------------------------------------------------------------------


_FX PROCESS *Thread_FindAndInitProcess(
    PROCESS *proc1, void *ProcessObject2, KIRQL *out_irql)
{
    PROCESS *proc2 = Process_Find(PsGetProcessId(ProcessObject2), out_irql);
    if (proc2) {

        if (! Process_IsSameBox(proc1, proc2, 0))
            proc2 = NULL;

        else if (! proc2->threads_lock) {
            BOOLEAN ok = Thread_InitProcess(proc2);
            if (! ok) {
                Process_SetTerminated(proc2, 11);
                proc2 = NULL;
            }
        }
    }

    return proc2;
}


//---------------------------------------------------------------------------
// Thread_AdjustGrantedAccess
//---------------------------------------------------------------------------


_FX BOOLEAN Thread_AdjustGrantedAccess(void)
{

#ifndef _WIN64

    static BOOLEAN _GrantedAccessOffsetIsGood = FALSE;

    //
    // on Windows XP, the kernel caches a granted access value for use
    // with the psuedo handle NtCurrentThread(), but this value is
    // computed using the real primary token which is highly restricted.
    // we have to fix this value
    //
    // note that we don't mind assigning impersonation rights because all
    // impersonation requests end up in PsImpersonateClient which only
    // grants the SecurityIdentification impersonation level if the primary
    // token does not include the SeImpersonatePrivilege privilege.
    // see also Thread_MyImpersonateClient
    //

    if (   Driver_OsVersion >= DRIVER_WINDOWS_XP
        && Driver_OsVersion <= DRIVER_WINDOWS_2003) {

        PETHREAD ThreadObject = PsGetCurrentThread();

        const ULONG GrantedAccessOffset =
            (Driver_OsVersion == DRIVER_WINDOWS_XP)
                ? 0x244         // Windows XP offset
                : 0x23C;        // Windows 2003 offset

        ULONG *pGrantedAccess = (ULONG *)
            ((ULONG_PTR)ThreadObject + GrantedAccessOffset);

        if (! _GrantedAccessOffsetIsGood) {
            if (*pGrantedAccess != 0x001F03FF) {
                // must not have the right offset, shouldn't happen
                Log_Status_Ex_Session(
                        MSG_1231, 0x55, *pGrantedAccess, NULL, -1);
                return FALSE;
            }
            _GrantedAccessOffsetIsGood = TRUE;
        }

        *pGrantedAccess |=
                  STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE
                | THREAD_TERMINATE | THREAD_SUSPEND_RESUME
                | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT
                | THREAD_SET_INFORMATION | THREAD_QUERY_INFORMATION
                | THREAD_SET_THREAD_TOKEN | THREAD_IMPERSONATE
                | THREAD_DIRECT_IMPERSONATION
                | THREAD_SET_LIMITED_INFORMATION
                | THREAD_QUERY_LIMITED_INFORMATION;
    }

#endif ! _WIN64

    return TRUE;
}


//---------------------------------------------------------------------------
// Thread_GetCurrent
//---------------------------------------------------------------------------


_FX THREAD *Thread_GetCurrent(PROCESS *proc)
{
    THREAD *thrd;
    HANDLE tid;
    KIRQL irql;

    if (! proc->threads_lock)
        return NULL;

    tid = PsGetCurrentThreadId();
    KeRaiseIrql(APC_LEVEL, &irql);
    ExAcquireResourceExclusiveLite(proc->threads_lock, TRUE);

    thrd = List_Head(&proc->threads);
    while (thrd) {
        if (thrd->tid == tid)
            break;
        thrd = List_Next(thrd);
    }

    ExReleaseResourceLite(proc->threads_lock);
    KeLowerIrql(irql);
    return thrd;
}


//---------------------------------------------------------------------------
// Thread_GetOrCreate
//---------------------------------------------------------------------------


_FX THREAD *Thread_GetOrCreate(PROCESS *proc, HANDLE tid, BOOLEAN create)
{
    THREAD *thrd;

    if (! tid)
        tid = PsGetCurrentThreadId();

    thrd = List_Head(&proc->threads);
    while (thrd) {
        if (thrd->tid == tid)
            break;
        thrd = List_Next(thrd);
    }

    if ((! thrd) && create) {
        thrd = Mem_Alloc(proc->pool, sizeof(THREAD));
        if (thrd) {
            memzero(thrd, sizeof(THREAD));
            thrd->tid = tid;
            List_Insert_After(&proc->threads, NULL, thrd);
        }
    }

    return thrd;
}


//---------------------------------------------------------------------------
// Thread_MyImpersonateClient
//---------------------------------------------------------------------------


_FX NTSTATUS Thread_MyImpersonateClient(
    PETHREAD ThreadObject, void *TokenObject,
    BOOLEAN CopyOnOpen, BOOLEAN EffectiveOnly,
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel)
{
    //
    // (starting with Windows XP SP 2) PsImpersonateClient will not
    // directly impersonate the requested token if:
    // - the primary token of the process does not include the
    //   privilege SeImpersonatePrivilege
    // - the requested impersonation level is >= SecurityImpersonation
    // - the impersonate privilege is not granted in the primary token
    //   that is associated with the process of the specified
    // - the SID in the requested token is different from the SID in
    //   the primary process token
    //
    // the conditions above will always be true because a sandboxed
    // process has a highly restricted anonymous primary token (see
    // Token_AssignPrimary).  in this case PsImpersonateClient will clone
    // the requested token, and will impersonate the cloned token with
    // SecurityIdentification impersonation level.
    //
    // impersonation at SecurityIdentification level (or lower) does not
    // allow the calling thread to open any securable objects, not even
    // its own thread token
    //
    // to work around this, we intentionally call PsImpersonateClient
    // with SecurityIdentification so it can impersonate the specified
    // token directly, and then we adjust the impersonation level stored
    // in the thread object to SecurityImpersonation
    //
    // see also Token_AssignPrimary
    //

    NTSTATUS status = PsImpersonateClient(ThreadObject, TokenObject,
                        CopyOnOpen, EffectiveOnly, SecurityIdentification);

    // Hard Offset Dependency

    // ***** ImpersonationInfo_offset is the offset of ClientSecurity field in nt!ETHREAD structure *****

    if (NT_SUCCESS(status) && TokenObject) {

        ULONG ImpersonationInfo_offset = 0;

#ifdef _WIN64

        // offset of ClientSecurity field in nt!ETHREAD structure

        if (Driver_OsVersion == DRIVER_WINDOWS_VISTA)
            ImpersonationInfo_offset = 0x3B0;

        else if (Driver_OsVersion == DRIVER_WINDOWS_7)
            ImpersonationInfo_offset = 0x3E0;

        else if (Driver_OsVersion == DRIVER_WINDOWS_8)
            ImpersonationInfo_offset = 0x3C8;

        else if (Driver_OsVersion == DRIVER_WINDOWS_81)
            ImpersonationInfo_offset = 0x650;

        else if (Driver_OsVersion == DRIVER_WINDOWS_10) {
            if (Driver_OsBuild < 14316)
                ImpersonationInfo_offset = 0x658;
            else if (Driver_OsBuild < 15031)
                ImpersonationInfo_offset = 0x660;
            else if (Driver_OsBuild < 18312)
                ImpersonationInfo_offset = 0x668;
            else if (Driver_OsBuild <= 18363)
                ImpersonationInfo_offset = 0x678;
            else if (Driver_OsBuild < 18980)
                ImpersonationInfo_offset = 0x688;
            else if (Driver_OsBuild < 21286)
                ImpersonationInfo_offset = 0x4a8;
            else
                ImpersonationInfo_offset = 0x4f8;
        }
#else ! _WIN64

        if (Driver_OsVersion == DRIVER_WINDOWS_XP)
            ImpersonationInfo_offset = 0x20C;

        else if (Driver_OsVersion == DRIVER_WINDOWS_2003)
            ImpersonationInfo_offset = 0x204;

        else if (Driver_OsVersion == DRIVER_WINDOWS_VISTA)
            ImpersonationInfo_offset = 0x228;

        else if (Driver_OsVersion == DRIVER_WINDOWS_7)
            ImpersonationInfo_offset = 0x248;

        else if (Driver_OsVersion == DRIVER_WINDOWS_8)
            ImpersonationInfo_offset = 0x230;

        else if (Driver_OsVersion == DRIVER_WINDOWS_81)
            ImpersonationInfo_offset = 0x380;

        else if (Driver_OsVersion == DRIVER_WINDOWS_10) {
            if (Driver_OsBuild < 14965)
                ImpersonationInfo_offset = 0x390;
            else if (Driver_OsBuild <= 18309) 
                ImpersonationInfo_offset = 0x398;
            else  if (Driver_OsBuild < 18980) 
                ImpersonationInfo_offset = 0x3A0;
            else if (Driver_OsBuild < 21286)
                ImpersonationInfo_offset = 0x2c8;
            else
                ImpersonationInfo_offset = 0x2f0;
        }
        //
        // on Windows XP (which is supported only in a 32-bit)
        // impersonation information is a structure
        //

        if (ImpersonationInfo_offset &&
                Driver_OsVersion >= DRIVER_WINDOWS_XP &&
                Driver_OsVersion <= DRIVER_WINDOWS_2003) {

            typedef struct _PS_IMPERSONATION_INFORMATION {
                void *TokenObject;
                BOOLEAN CopyOnOpen;
                BOOLEAN EffectiveOnly;
                SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
            } PS_IMPERSONATION_INFORMATION;

            PS_IMPERSONATION_INFORMATION *ImpersonationInfo =
                *(PS_IMPERSONATION_INFORMATION **)
                    ((ULONG_PTR)ThreadObject + ImpersonationInfo_offset);

            if (ImpersonationInfo->TokenObject != TokenObject)
                ImpersonationInfo_offset = 0;   // trigger error message

            else if (ImpersonationLevel != SecurityIdentification) {

                ImpersonationInfo->ImpersonationLevel = ImpersonationLevel;
            }
        }

#endif _WIN64

        //
        // on Windows Vista and later, impersonation info is a ULONG_PTR
        // where bits 0..1 are the impersonation level,
        // bit 2 is the effective only flag,
        // bits 3..31 (or 3..63) is the token object pointer
        // and CopyOnOpen is recorded somewhere else in the thread structure
        // (check output of "dt nt!_PS_CLIENT_SECURITY_CONTEXT" in windbg)
        //

        if (ImpersonationInfo_offset &&
                Driver_OsVersion >= DRIVER_WINDOWS_VISTA) {

            ULONG_PTR *ImpersonationInfo = (ULONG_PTR *)
                    ((ULONG_PTR)ThreadObject + ImpersonationInfo_offset);

            if ((*ImpersonationInfo & ~7) != (ULONG_PTR)TokenObject) {
                // kernel updates occasionally alter structure of ETHREAD
                // and ImpersonationInfo_offset may be off by a ULONG_PTR
                ++ImpersonationInfo;
            }

            if ((*ImpersonationInfo & ~7) != (ULONG_PTR)TokenObject)
                ImpersonationInfo_offset = 0;   // trigger error message

            else if (ImpersonationLevel != SecurityIdentification) {

                *ImpersonationInfo = ((*ImpersonationInfo) & ~3)
                                   | (ImpersonationLevel & 3);
            }
        }

        if (!ImpersonationInfo_offset) {
            status = STATUS_ACCESS_DENIED;
            Log_Status(MSG_1222, 0x62, STATUS_UNKNOWN_REVISION);

            /*__try {
                for (int i = 0; i < 0x0a00; i++)
                {
                    ULONG_PTR* ImpersonationInfo = (ULONG_PTR*)((ULONG_PTR)ThreadObject + i);

                    if ((*ImpersonationInfo & ~7) == (ULONG_PTR)TokenObject)
                    {
                        WCHAR str[64];
                        swprintf(str, L"BAM! found: %d", i);
                        Session_MonitorPut(MONITOR_OTHER, str, PsGetCurrentProcessId());
                    }
                }
            } __except (EXCEPTION_EXECUTE_HANDLER) {}*/
        }
    }

    return status;
}


//---------------------------------------------------------------------------
// Thread_SetThreadToken
//---------------------------------------------------------------------------


_FX void Thread_SetThreadToken(PROCESS *proc)
{
    void *TokenObject;
    THREAD *thrd;
    NTSTATUS status;
    KIRQL irql;
    BOOLEAN DerefToken;
    BOOLEAN CopyOnOpen;
    BOOLEAN EffectiveOnly;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;

    if (! proc->primary_token)
        return;

    //
    // enable security before processing a system call
    //
    // a user mode thread in a sandboxed process should always be running
    // without impersonation and with highly restricted primary token.
    // when invoking a system call, we want to restore the original
    // security context so call can succeed
    //
    // if the thread has a recorded impersonation token in our thread
    // structure, we restore this token;  otherwise we use the original
    // primary token which is stored in our process structure
    //
    // after processing a system call, the impersonation token is discarded
    //

    DerefToken = FALSE;

    thrd = Thread_GetCurrent(proc);
    if (thrd) {

        //
        // we have a thread structure which means we might have an
        // impersonation token, although not necessarily
        //

        KeRaiseIrql(APC_LEVEL, &irql);
        ExAcquireResourceExclusiveLite(proc->threads_lock, TRUE);

        TokenObject = thrd->token_object;
        CopyOnOpen = thrd->token_CopyOnOpen;
        EffectiveOnly = thrd->token_EffectiveOnly;
        ImpersonationLevel = thrd->token_ImpersonationLevel;

        if (TokenObject) {
            ObReferenceObject(TokenObject);
            DerefToken = TRUE;
        }

        ExReleaseResourceLite(proc->threads_lock);
        KeLowerIrql(irql);

    } else {

        //
        // no thread structure means the thread has never impersonated
        //

        TokenObject = NULL;
    }

    if (! TokenObject) {

        //
        // if the thread is not currently impersonating, use primary token
        //

        TokenObject = proc->primary_token;
        CopyOnOpen = FALSE;
        EffectiveOnly = FALSE;
        ImpersonationLevel = SecurityImpersonation;
    }

    //
    // begin thread impersonation for the selected token
    //

    status = Thread_MyImpersonateClient(PsGetCurrentThread(), TokenObject,
                CopyOnOpen, EffectiveOnly, ImpersonationLevel);

    if (! NT_SUCCESS(status)) {

        Log_Status(MSG_1222, 0x61, status);
        Process_SetTerminated(proc, 12);
    }

    if (DerefToken)
        ObDereferenceObject(TokenObject);
}


//---------------------------------------------------------------------------
// Thread_StoreThreadToken
//---------------------------------------------------------------------------


_FX NTSTATUS Thread_StoreThreadToken(PROCESS *proc)
{
    THREAD *thrd;
    void *TokenObject;
    BOOLEAN CopyOnOpen;
    BOOLEAN EffectiveOnly;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
    KIRQL irql;
    NTSTATUS status;

    //
    // syscalls like NtImpersonateAnonymousToken/NtImpersonateClientOfPort
    // will not be able to impersonate with SecurityImpersonation level
    // because our highly restricted process token does not include the
    // SeImpersonatePrivilege privilege.  to work around this limitation,
    // this functions gets the active impersonation token which was put in
    // place by a syscall like NtImpersonateAnonymousToken, and stores it
    // in our thread structure with SecurityImpersonation level.
    //

    //
    // get the active impersonation token
    //

    if (! proc->primary_token)
        return STATUS_SUCCESS;

    status = STATUS_NO_IMPERSONATION_TOKEN;

    TokenObject = PsReferenceImpersonationToken(PsGetCurrentThread(),
                    &CopyOnOpen, &EffectiveOnly, &ImpersonationLevel);

    if (! TokenObject) {
        // if we are called, there should be an impersonation token
        return status;
    }

    //
    // check if we need to store the new token into our thread struccture
    //

    KeRaiseIrql(APC_LEVEL, &irql);
    ExAcquireResourceExclusiveLite(proc->threads_lock, TRUE);

    thrd = Thread_GetCurrent(proc);
    if (thrd) {

        //
        // store the currently active impersonation token into our
        // thread structure, unless the same token is already there
        //

        if (thrd->token_object != TokenObject)
            status = STATUS_SUCCESS;

    } else {

        thrd = Thread_GetOrCreate(proc, NULL, TRUE);
        if (thrd)
            status = STATUS_SUCCESS;
        else
            status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (NT_SUCCESS(status)) {

        TokenObject = InterlockedExchangePointer(
                                        &thrd->token_object, TokenObject);
        thrd->token_CopyOnOpen = CopyOnOpen;
        thrd->token_EffectiveOnly = EffectiveOnly;
        thrd->token_ImpersonationLevel = SecurityImpersonation;

        /*if (CopyOnOpen)
            DbgPrint("Stored Thread Token With CopyOnOpen\n");*/

    } else if (status == STATUS_NO_IMPERSONATION_TOKEN)
        status = STATUS_SUCCESS;

    ExReleaseResourceLite(proc->threads_lock);
    KeLowerIrql(irql);

    //
    // dereference the old token, which is either the old token that was
    // stored in our thread structure and has to be dereferenceed, or it
    // is the token we got from PsReferenceImpersonationToken which also
    // would have to be dereference
    //

    if (TokenObject)
        ObDereferenceObject(TokenObject);

    return status;
}


//---------------------------------------------------------------------------
// Thread_CheckProcessObject
//---------------------------------------------------------------------------


_FX NTSTATUS Thread_CheckProcessObject(
    PROCESS *proc, void *Object, UNICODE_STRING *Name,
    ACCESS_MASK GrantedAccess)
{
    PEPROCESS ProcessObject = (PEPROCESS)Object;
    ACCESS_MASK WriteAccess = (GrantedAccess & PROCESS_DENIED_ACCESS_MASK);
    return Thread_CheckObject_Common(
                proc, ProcessObject, GrantedAccess, WriteAccess, L'P');
}


//---------------------------------------------------------------------------
// Thread_CheckThreadObject
//---------------------------------------------------------------------------


_FX NTSTATUS Thread_CheckThreadObject(
    PROCESS *proc, void *Object, UNICODE_STRING *Name,
    ACCESS_MASK GrantedAccess)
{
    PEPROCESS ProcessObject = PsGetThreadProcess(Object);
    ACCESS_MASK WriteAccess = (GrantedAccess & THREAD_DENIED_ACCESS_MASK);
    return Thread_CheckObject_Common(
                proc, ProcessObject, GrantedAccess, WriteAccess, L'T');
}


//---------------------------------------------------------------------------
// Thread_CheckObject_Common
//---------------------------------------------------------------------------


_FX NTSTATUS Thread_CheckObject_Common(
    PROCESS *proc, PEPROCESS ProcessObject,
    ACCESS_MASK GrantedAccess, ACCESS_MASK WriteAccess, WCHAR Letter1)
{
    ULONG_PTR pid;
    const WCHAR *pSetting;
    NTSTATUS status;

    //
    // if an error occured and can't find pid, then don't allow
    //

    pid = (ULONG_PTR)PsGetProcessId(ProcessObject);

    if (! pid)
        return STATUS_ACCESS_DENIED;

    //
    // for read-only access to the target process, we don't care
    // if/which boxes are involved
    //

    if (pid && (WriteAccess == 0)) {
        status = STATUS_SUCCESS;
        goto trace;
    }

    //
    // otherwise this is write access, confirm if same box
    //

    if (Process_IsSameBox(proc, NULL, pid)) {
        status = STATUS_SUCCESS;
        goto trace;
    }

    //
    // also permit if process is exiting, because it is possible that
    // Process_NotifyProcess_Delete already executed, while our caller
    // still has a handle to the process and calls NtDuplicateObject
    // (e.g. VS2012 MSBuild.exe does this with the csc.exe compiler)
    //

    if (PsGetProcessExitProcessCalled(ProcessObject)) {
        status = STATUS_SUCCESS;
        goto trace;
    }

    //
    // write access outside box, check if we have the following setting
    // OpenIpcPath=$:ProcessName.exe
    //

    status = Process_CheckProcessName(
                    proc, &proc->open_ipc_paths, pid, &pSetting);

    //
    // log the cross-sandbox access attempt, based on the status code
    //

    if (Session_MonitorCount) {

        void *nbuf;
        ULONG nlen;
        WCHAR *nptr;

        Process_GetProcessName(proc->pool, pid, &nbuf, &nlen, &nptr);
        if (nbuf) {

            ULONG mon_type = MONITOR_IPC;
            if (NT_SUCCESS(status))
                mon_type |= MONITOR_OPEN;
            else
                mon_type |= MONITOR_DENY;

            --nptr; *nptr = L':';
            --nptr; *nptr = L'$';

            Session_MonitorPut(mon_type, nptr, proc->pid);

            Mem_Free(nbuf, nlen);
        }
    }

    //
    // trace
    //

trace:

    if (proc->ipc_trace & (TRACE_ALLOW | TRACE_DENY)) {

        WCHAR str[32];
        WCHAR Letter2;

        if ((! NT_SUCCESS(status)) && (proc->ipc_trace & TRACE_DENY))
            Letter2 = L'D';
        else if (NT_SUCCESS(status) && (proc->ipc_trace & TRACE_ALLOW))
            Letter2 = L'A';
        else
            Letter2 = 0;

        if (Letter2) {
            RtlStringCbPrintfW(str, sizeof(str), L"(%c%c) %08X %06d",
                                Letter1, Letter2, GrantedAccess, (int)pid);
            Log_Debug_Msg(MONITOR_IPC | MONITOR_TRACE, str, Driver_Empty);
        }
    }

    return status;
}


//---------------------------------------------------------------------------
// Thread_Api_OpenProcess
//---------------------------------------------------------------------------


_FX NTSTATUS Thread_Api_OpenProcess(PROCESS *proc, ULONG64 *parms)
{
    API_OPEN_PROCESS_ARGS *args = (API_OPEN_PROCESS_ARGS *)parms;
    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    CLIENT_ID clientid;
    ACCESS_MASK DesiredAccess;
    HANDLE handle;
    HANDLE user_pid;
    HANDLE *user_handle;

    //
    // this API must be invoked by a sandboxed process
    //

    if (! proc)
        return STATUS_NOT_IMPLEMENTED;

    //
    // collect and verify parameters
    //

    user_pid = args->process_id.val;

    user_handle = args->process_handle.val;

    ProbeForWrite(user_handle, sizeof(HANDLE), sizeof(ULONG_PTR));

    //
    // decide how to open the target process
    //

    DesiredAccess = 0;

    if (Process_IsSameBox(proc, NULL, (ULONG_PTR)user_pid)) {

        //
        // scenario 1:  requesting access to another process in
        // the same sandbox.  give full access.
        //

        DesiredAccess = PROCESS_ALL_ACCESS;

    } else {

        //
        // scenario 2:  requesting access to a process outside
        // any sandbox.  for instance a process with administrator
        // privileges trying to open csrss.exe but failing because
        // we stripped the debug privileges.  in this case grant
        // PROCESS_QUERY_INFORMATION access.
        //

        BOOLEAN IsAdmin = FALSE;

        if (proc->primary_token)
            IsAdmin = SeTokenIsAdmin(proc->primary_token);
        else {

            PACCESS_TOKEN pAccessToken =
                PsReferencePrimaryToken(PsGetCurrentProcess());
            IsAdmin = SeTokenIsAdmin(pAccessToken);
            PsDereferencePrimaryToken(pAccessToken);
        }

        if (IsAdmin) {

            PROCESS *proc2 = Process_Find(user_pid, NULL);
            if (! proc2) {

                DesiredAccess = PROCESS_QUERY_INFORMATION;
            }
        }
    }

    if (! DesiredAccess)
        return STATUS_ACCESS_DENIED;

    //
    // open process object
    //

    InitializeObjectAttributes(&objattrs, NULL, 0, NULL, NULL);
    clientid.UniqueThread = NULL;
    clientid.UniqueProcess = user_pid;

    status = ZwOpenProcess(&handle, DesiredAccess, &objattrs, &clientid);

    if (NT_SUCCESS(status)) {

        __try {

            *user_handle = handle;

        } __except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
        }

        if (! NT_SUCCESS(status))
            ZwClose(handle);
    }

    return status;
}
