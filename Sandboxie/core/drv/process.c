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
// Process Management
//---------------------------------------------------------------------------


#include "process.h"
#include "util.h"
#include "conf.h"
#include "key.h"
#include "file.h"
#include "ipc.h"
#include "api.h"
#include "dll.h"
#include "hook.h"
#include "session.h"
#include "gui.h"
#include "token.h"
#include "thread.h"
#include "common/my_version.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


#ifndef _WIN64

static NTSTATUS Process_HookProcessNotify(
    PCREATE_PROCESS_NOTIFY_ROUTINE NewNotifyRoutine);

#endif _WIN64

static ULONG Process_GetTraceFlag(PROCESS *proc, const WCHAR *setting);

static void Process_NotifyProcess(
    HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create);

static void Process_CreateTerminated(HANDLE ProcessId, ULONG SessionId);

static PROCESS *Process_Create(
    HANDLE ProcessId, const BOX *box, const WCHAR *image_path,
    KIRQL *out_irql);

static void Process_NotifyProcess_Delete(HANDLE ProcessId);

static void Process_Delete(HANDLE ProcessId);

static void Process_NotifyImage(
    const UNICODE_STRING *FullImageName,
    HANDLE ProcessId, IMAGE_INFO *ImageInfo);


//---------------------------------------------------------------------------


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, Process_Init)
#ifndef _WIN64
#pragma alloc_text (INIT, Process_HookProcessNotify)
#endif _WIN64
#endif // ALLOC_PRAGMA


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


LIST Process_List;
LIST Process_ListDfp;
PERESOURCE Process_ListLock = NULL;

static BOOLEAN Process_NotifyImageInstalled = FALSE;
static BOOLEAN Process_NotifyProcessInstalled = FALSE;

volatile BOOLEAN Process_ReadyToSandbox = FALSE;

#ifndef _WIN64

static PCREATE_PROCESS_NOTIFY_ROUTINE *Process_pOldNotifyProcess = NULL;

#endif _WIN64


//---------------------------------------------------------------------------
// Process_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Process_Init(void)
{
    NTSTATUS status;

    List_Init(&Process_List);
    List_Init(&Process_ListDfp);

    if (! Mem_GetLockResource(&Process_ListLock, TRUE))
        return FALSE;

    if (! Process_Low_Init())
        return FALSE;

    //
    // install process notify routines
    //

    status = PsSetCreateProcessNotifyRoutine(Process_NotifyProcess, FALSE);

#ifndef _WIN64

    if ((status == STATUS_INVALID_PARAMETER) &&
                    (Driver_OsVersion < DRIVER_WINDOWS_VISTA)) {

        status = Process_HookProcessNotify(Process_NotifyProcess);
    }

#endif _WIN64

    if (NT_SUCCESS(status)) {

        Process_NotifyProcessInstalled = TRUE;

    } else {

        // too many notify routines are already installed in the system
        Log_Status(MSG_PROCESS_NOTIFY, 0x11, status);
        return FALSE;
    }

    //
    // install image notify routines
    //

    status = PsSetLoadImageNotifyRoutine(Process_NotifyImage);
    if (NT_SUCCESS(status))
        Process_NotifyImageInstalled = TRUE;
    else {
        Log_Status(MSG_PROCESS_NOTIFY, 0x22, status);
        return FALSE;
    }

    //
    // set API functions
    //

    Api_SetFunction(API_START_PROCESS,        Process_Api_Start);
    Api_SetFunction(API_QUERY_PROCESS,        Process_Api_Query);
    Api_SetFunction(API_QUERY_PROCESS_INFO,   Process_Api_QueryInfo);
    Api_SetFunction(API_QUERY_BOX_PATH,       Process_Api_QueryBoxPath);
    Api_SetFunction(API_QUERY_PROCESS_PATH,   Process_Api_QueryProcessPath);
    Api_SetFunction(API_QUERY_PATH_LIST,      Process_Api_QueryPathList);
    Api_SetFunction(API_ENUM_PROCESSES,       Process_Api_Enum);

    return TRUE;
}


//---------------------------------------------------------------------------
// Process_Unload
//---------------------------------------------------------------------------


_FX void Process_Unload(BOOLEAN FreeLock)
{
    if (Process_NotifyImageInstalled) {

        PsRemoveLoadImageNotifyRoutine(Process_NotifyImage);
        Process_NotifyImageInstalled = FALSE;
    }

    if (Process_NotifyProcessInstalled) {

        PsSetCreateProcessNotifyRoutine(Process_NotifyProcess, TRUE);
        Process_NotifyProcessInstalled = FALSE;

#ifndef _WIN64

        if (Process_pOldNotifyProcess) {
            PCREATE_PROCESS_NOTIFY_ROUTINE OldNotifyProcess =
                (PCREATE_PROCESS_NOTIFY_ROUTINE)InterlockedExchange(
                    (volatile LONG *)Process_pOldNotifyProcess, 0);
            if (OldNotifyProcess)
                PsSetCreateProcessNotifyRoutine(OldNotifyProcess, FALSE);
            Process_pOldNotifyProcess = NULL;
        }

#endif _WIN64

    }

    Process_Low_Unload();

    if (FreeLock)
        Mem_FreeLockResource(&Process_ListLock);
}


//---------------------------------------------------------------------------
// Process_HookProcessNotify
//---------------------------------------------------------------------------


#ifndef _WIN64


_FX NTSTATUS Process_HookProcessNotify(
    PCREATE_PROCESS_NOTIFY_ROUTINE NewNotifyRoutine)
{
    static UCHAR HookBytes[] = {
        // 0xB9, xx,xx,xx,xx                // mov   ecx,xxxxxxxx
        0x80, 0x7C, 0x24, 0x08, 0x00,       // cmp   byte ptr [esp+8],0
        0x74, 0x13,                         // je    j1
        0x8B, 0x01,                         // mov   eax,[ecx]
        0x3B, 0x44, 0x24, 0x04,             // cmp   eax,[esp+4]
        0x75, 0x0B,                         // jne   j1
        0x31, 0xD2,                         // xor   edx,edx
        0xF0, 0x0F, 0xB1, 0x11,             // lock cmpxchg [ecx],edx
        0x31, 0xC0,                         // xor   eax,eax
        0xC2, 0x08, 0x00,                   // ret   8
        0xE9  // xx,xx,xx,xx                // j1:   jmp   xxxxxxxx
    };

    NTSTATUS status;
    UCHAR *ptr, *tramp;
    ULONG *PspCreateProcessNotifyRoutine;
    ULONG OldNotifyRoutine;

    //
    // on Windows XP we try a little harder to install a process notify
    // routine, by removing a competing routine, installing our routine
    // in its place, and jump from our notify routine to the original.
    //

    if (Driver_OsVersion != DRIVER_WINDOWS_XP)
        return STATUS_UNKNOWN_REVISION;

    //
    // to start this we need to find the PspCreateProcessNotifyRoutine
    // array by analyzing PsSetCreateProcessNotifyRoutine, so make sure
    // we are looking at the right version.  also, we will need to hook
    // PsSetCreateProcessNotifyRoutine itself, so make sure it is
    // hot-patchable.
    //

    ptr = (void *)PsSetCreateProcessNotifyRoutine;

    tramp = NULL;

    if (ptr[0] != 0x8B || ptr[1] != 0xFF ||
        (ptr[-5] != 0xCC && ptr[-5] != 0x90) ||
        (ptr[-4] != 0xCC && ptr[-4] != 0x90) ||
        (ptr[-3] != 0xCC && ptr[-3] != 0x90) ||
        (ptr[-2] != 0xCC && ptr[-2] != 0x90) ||
        (ptr[-1] != 0xCC && ptr[-1] != 0x90)) {

        if (*(USHORT *)ptr == 0xF9EB && ptr[-5] == 0xE9) {

            //
            // if PsCreateProcessNotifyRoutine is hotpatched and jumps to
            // what looks like our hook (from a previously-loaded instance
            // of our driver), then it's ok to use it
            //

            tramp = ptr + *(ULONG *)(ptr - 4);
            if (*tramp != 0xB9 ||
                    memcmp(tramp + 5, HookBytes, sizeof(HookBytes)) != 0)
                tramp = NULL;
        }

        if (! tramp)
            return STATUS_UNKNOWN_REVISION;
    }

    if (ptr[0x0B] != 0x56 || ptr[0x0C] != 0x57 || ptr[0x0D] != 0x74 ||
        ptr[0x0F] != 0xBF || ptr[0x14] != 0x57)
        return STATUS_UNKNOWN_REVISION;

    //
    // find the first notify routine that is already installed
    //

    PspCreateProcessNotifyRoutine = *(ULONG **)(ptr + 0x10);
    OldNotifyRoutine = *PspCreateProcessNotifyRoutine;
    if (! OldNotifyRoutine)
        return STATUS_INVALID_PARAMETER_1;
    OldNotifyRoutine = *(ULONG *)(OldNotifyRoutine & (~3));
    if (! OldNotifyRoutine)
        return STATUS_INVALID_PARAMETER_1;

    //
    // build our PsCreateProcessNotifyRoutine replacement hook, which
    // is there just to trap calls that request to remove the competing
    // process notify routine that we are about to replace
    //

    if (! tramp) {
        tramp = Hook_BuildTramp(NULL, NULL, FALSE, FALSE);
        if (! tramp)
            return STATUS_INSUFFICIENT_RESOURCES;
    }

    Process_pOldNotifyProcess = (PCREATE_PROCESS_NOTIFY_ROUTINE *)
        &(HOOK_TRAMP_CODE_TO_TRAMP_HEAD(tramp))->target;

    ptr = tramp;

    *ptr = 0xB9;                    // mov   ecx,&tramp->target
    *(ULONG *)(ptr + 0x01) = (ULONG)(ULONG_PTR)Process_pOldNotifyProcess;
    ptr += 5;
    memcpy(ptr, HookBytes, sizeof(HookBytes));
    ptr += sizeof(HookBytes);
    *(ULONG *)ptr =                 // jmp   original_routine
        (ULONG)((UCHAR *)PsSetCreateProcessNotifyRoutine + 2 - (ptr + 4));

    //
    // now try to remove the competing process notify routine,
    // and install our routine in its place
    //

    status = PsSetCreateProcessNotifyRoutine(
        (PCREATE_PROCESS_NOTIFY_ROUTINE)OldNotifyRoutine, TRUE);

    if (! NT_SUCCESS(status)) {
        Process_pOldNotifyProcess = NULL;
        return status;
    }

    *Process_pOldNotifyProcess =
        (PCREATE_PROCESS_NOTIFY_ROUTINE)OldNotifyRoutine;

    status = PsSetCreateProcessNotifyRoutine(NewNotifyRoutine, FALSE);
    if (! NT_SUCCESS(status))
        return status;

    //
    // and finally hot-patch PsCreateProcessNotifyRoutine
    //

    DisableWriteProtect();

    ptr = (void *)PsSetCreateProcessNotifyRoutine;
    ptr[-5] = 0xE9;
    *(ULONG *)(ptr - 4) = (ULONG)(tramp - ptr);

    *(USHORT *)ptr = 0xF9EB;                // jmp short -5

    EnableWriteProtect();

    //
    // done
    //

    return STATUS_SUCCESS;
}

#endif _WIN64


//---------------------------------------------------------------------------
// Process_Find
//---------------------------------------------------------------------------


_FX PROCESS *Process_Find(HANDLE ProcessId, KIRQL *out_irql)
{
    PROCESS *proc;
    KIRQL irql;
    BOOLEAN check_terminated;

    //
    // if we're looking for the current process, then check execution mode
    // to make sure it isn't a system process or kernel mode caller
    //

    if (! ProcessId) {

        if (ExGetPreviousMode() == KernelMode)
            ProcessId = (HANDLE) 0;
        else
            ProcessId = PsGetCurrentProcessId();

        if (ProcessId < (HANDLE) 8) {

            if (out_irql) {
                // in case caller expects a locked process list on return
                KeRaiseIrql(APC_LEVEL, out_irql);
                ExAcquireResourceSharedLite(Process_ListLock, TRUE);
            }

            return NULL;
        }

        check_terminated = TRUE;

    } else
        check_terminated = FALSE;

    //
    // find a PROCESS block that matches the current ProcessId
    //

    KeRaiseIrql(APC_LEVEL, &irql);
    ExAcquireResourceSharedLite(Process_ListLock, TRUE);

    proc = List_Head(&Process_List);

    if (check_terminated) {

        while (proc) {
            if (proc->pid == ProcessId) {

                if (proc->terminated) {
                    //
                    // ntdll is going to call NtRaiseHardError before
                    // aborting, so disable hard errors to avoid the
                    // pop up box from csrss
                    //

                    if (proc->terminated != 9) {
                        proc->terminated = 9;
                        PsSetThreadHardErrorsAreDisabled(
                            (PETHREAD)KeGetCurrentThread(), TRUE);
                    }
                    //
                    // signal that the caller should return status
                    //     STATUS_PROCESS_IS_TERMINATING
                    // (see Api_FastIo_DEVICE_CONTROL for example)
                    //
                    proc = PROCESS_TERMINATED;
                }

                break;
            }

            proc = List_Next(proc);
        }

    } else {

        while (proc) {
            if (proc->pid == ProcessId)
                break;
            proc = List_Next(proc);
        }
    }

    if (out_irql) {

        *out_irql = irql;

    } else {

        ExReleaseResourceLite(Process_ListLock);
        KeLowerIrql(irql);
    }

    return proc;
}

//---------------------------------------------------------------------------
// Process_FindSandboxed
//---------------------------------------------------------------------------


_FX PROCESS *Process_FindSandboxed(HANDLE ProcessId, KIRQL *out_irql)
{
    PROCESS* proc = Process_Find(ProcessId, out_irql);
    if (proc && proc != PROCESS_TERMINATED)
    {
        if (proc->bHostInject)
        {
            proc = NULL;
        }
    }
    return proc;
}


//---------------------------------------------------------------------------
// Process_CreateTerminated
//---------------------------------------------------------------------------


_FX void Process_CreateTerminated(HANDLE ProcessId, ULONG SessionId)
{
    UNICODE_STRING pid_str;
    PROCESS *proc;
    KIRQL irql;

    pid_str.Length = 10 * sizeof(WCHAR);
    pid_str.MaximumLength = pid_str.Length + sizeof(WCHAR);

    pid_str.Buffer = Mem_Alloc(Driver_Pool, pid_str.MaximumLength);
    if (pid_str.Buffer) {

        RtlIntPtrToUnicodeString((ULONG_PTR)ProcessId, 10, &pid_str);
		if (SessionId != -1) // for StartRunAlertDenied
			Log_Msg_Process(MSG_1211, pid_str.Buffer, NULL, SessionId, ProcessId);

        Mem_Free(pid_str.Buffer, pid_str.MaximumLength);
    }

    proc = Mem_Alloc(Driver_Pool, sizeof(PROCESS));
    if (proc) {

        memzero(proc, sizeof(PROCESS));
        proc->pid = ProcessId;
        proc->pool = Driver_Pool;
        Process_SetTerminated(proc, 1);

        KeRaiseIrql(APC_LEVEL, &irql);
        ExAcquireResourceExclusiveLite(Process_ListLock, TRUE);

        List_Insert_After(&Process_List, NULL, proc);

        ExReleaseResourceLite(Process_ListLock);
        KeLowerIrql(irql);
    }
}


//---------------------------------------------------------------------------
// Process_Create
//---------------------------------------------------------------------------


_FX PROCESS *Process_Create(
    HANDLE ProcessId, const BOX *box, const WCHAR *image_path,
    KIRQL *out_irql)
{
    POOL *pool;
    PROCESS *proc;
    PEPROCESS ProcessObject;
    NTSTATUS status;
    KIRQL irql;
    BOOLEAN locks_ok;

    //
    // create a pool and initialize a PROCESS block
    //

    pool = Pool_Create();
    if (! pool) {
		Log_Msg_Process(MSG_1201, NULL, NULL, box->session_id, ProcessId);
        Process_CreateTerminated(ProcessId, box->session_id);
        return NULL;
    }

    proc = Mem_Alloc(pool, sizeof(PROCESS));
    if (! proc) {
        // first allocation from a new pool should never fail
		Log_Msg_Process(MSG_1201, NULL, NULL, box->session_id, ProcessId);
        Pool_Delete(pool);
        Process_CreateTerminated(ProcessId, box->session_id);
        return NULL;
    }

    memzero(proc, sizeof(PROCESS));

    proc->pid = ProcessId;
    proc->pool = pool;

    proc->box = Box_Clone(pool, box);
    if (! proc->box) {
        Pool_Delete(pool);
        Process_CreateTerminated(ProcessId, box->session_id);
        return NULL;
    }

    //
    // initialize process creation time and integrity level
    //

    status = PsLookupProcessByProcessId(proc->pid, &ProcessObject);
    if (! NT_SUCCESS(status)) {

		Log_Status_Ex_Process(MSG_1231, 0x33, status, L"???", box->session_id, ProcessId);

        Pool_Delete(pool);
        Process_CreateTerminated(ProcessId, box->session_id);
        return NULL;
    }

    proc->create_time = PsGetProcessCreateTimeQuadPart(ProcessObject);

    ObDereferenceObject(ProcessObject);

    proc->integrity_level = tzuk;   // default to no integrity level

    //
    // initialize image name from image path
    //

    if (image_path) {

        UNICODE_STRING image_uni;
        WCHAR *image_name = wcsrchr(image_path, L'\\');
        if (image_name) {

            ULONG len = (ULONG)(image_name - image_path);
            if ((len == Driver_HomePathNt_Len) &&
                    (wcsncmp(image_path, Driver_HomePathNt, len) == 0)) {

                proc->image_sbie = TRUE;

                if (_wcsicmp(image_name + 1, START_EXE) == 0) {

                    proc->is_start_exe = TRUE;
                }
            }

            RtlInitUnicodeString(&image_uni, image_path);
            if (Box_IsBoxedPath(proc->box, file, &image_uni)) {

                proc->image_from_box = TRUE;
            }

            ++image_name;

            len = wcslen(image_name);
            if (len) {

                proc->image_name_len = (len + 1) * sizeof(WCHAR);
                proc->image_name =
                            Mem_Alloc(proc->pool, proc->image_name_len);
                if (proc->image_name) {
                    memcpy(proc->image_name, image_name,
                           proc->image_name_len);
                } else
					Log_Msg_Process(MSG_1201, NULL, NULL, box->session_id, proc->pid);
            }
        }

        proc->image_path = Mem_AllocString(proc->pool, image_path);
    }

    if ((! proc->image_name) || (! proc->image_path)) {

        const ULONG status = STATUS_INVALID_IMAGE_FORMAT;
		Log_Status_Ex_Process(MSG_1231, 0x11, status, L"???", box->session_id, proc->pid);

        Pool_Delete(pool);
        Process_CreateTerminated(ProcessId, box->session_id);
        return NULL;
    }

    //
    // initialize various locks
    //

    locks_ok = FALSE;
    if (Mem_GetLockResource(&proc->file_lock, FALSE))
        if (Mem_GetLockResource(&proc->key_lock, FALSE))
            if (Mem_GetLockResource(&proc->ipc_lock, FALSE))
                if (Mem_GetLockResource(&proc->gui_lock, FALSE))
                        locks_ok = TRUE;

    if (! locks_ok) {

        if (proc->file_lock)
            Mem_FreeLockResource(&proc->file_lock);
        if (proc->key_lock)
            Mem_FreeLockResource(&proc->key_lock);
        if (proc->ipc_lock)
            Mem_FreeLockResource(&proc->ipc_lock);
        if (proc->gui_lock)
            Mem_FreeLockResource(&proc->gui_lock);

		Log_Msg_Process(MSG_1201, NULL, NULL, box->session_id, ProcessId);
        Pool_Delete(pool);
        Process_CreateTerminated(ProcessId, box->session_id);
        return NULL;
    }

    //
    // initialize trace flags
    //

    proc->call_trace = Process_GetTraceFlag(proc, L"CallTrace");
    proc->file_trace = Process_GetTraceFlag(proc, L"FileTrace");
    proc->pipe_trace = Process_GetTraceFlag(proc, L"PipeTrace");
    proc->key_trace  = Process_GetTraceFlag(proc, L"KeyTrace");
    proc->ipc_trace  = Process_GetTraceFlag(proc, L"IpcTrace");
    proc->gui_trace  = Process_GetTraceFlag(proc, L"GuiTrace");

    //
    // check if OpenWinClass=* is specified for the box
    //

    Gui_Check_OpenWinClass(proc);

    //
    // insert the new process into the list of sandboxed processes
    //

    KeRaiseIrql(APC_LEVEL, &irql);
    ExAcquireResourceExclusiveLite(Process_ListLock, TRUE);

    List_Insert_After(&Process_List, NULL, proc);

    *out_irql = irql;

    return proc;
}


//---------------------------------------------------------------------------
// Process_GetTraceFlag
//---------------------------------------------------------------------------


_FX ULONG Process_GetTraceFlag(PROCESS *proc, const WCHAR *setting)
{
    ULONG flag = 0;
    const WCHAR *value;

    Conf_AdjustUseCount(TRUE);

    value = Conf_Get(proc->box->name, setting, 0);
    while (value && *value) {
        if (*value == L'a' || *value == L'A' || *value == L'*')
            flag |= TRACE_ALLOW;
        if (*value == L'd' || *value == L'D' || *value == L'*')
            flag |= TRACE_DENY;
        if (*value == L'i' || *value == L'I' || *value == L'*')
            flag |= TRACE_IGNORE;
        ++value;
    }

    Conf_AdjustUseCount(FALSE);

    return flag;
}


//---------------------------------------------------------------------------
// Process_NotifyProcess
//---------------------------------------------------------------------------


_FX void Process_NotifyProcess(
    HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create)
{
    //
    // if we replaced an existing notify routine, call it first
    //

#ifndef _WIN64

    if (Process_pOldNotifyProcess && (*Process_pOldNotifyProcess))
        (*Process_pOldNotifyProcess)(ParentId, ProcessId, Create);

#endif _WIN64

    //
    // don't do anything before the main driver init says it's ok
    //

    if (! Process_ReadyToSandbox)
        return;

    //
    // handle process creation and deletion.  note that we are running
    // in an arbitrary thread context
    //

    if (ProcessId) {

        if (Create) {

            //
            // it is possible to specify the parrent process when calling RtlCreateUserProcess
            // this is for example done by the appinfo service running under svchost.exe
            // to start LocalBridge.exe with RuntimeBroker.exe as parent
            // hence we take for our purposes the ID of the process calling RtlCreateUserProcess instead
            //

            //DbgPrint("Process_NotifyProcess_Create pid=%d parent=%d current=%d\n", ProcessId, ParentId, PsGetCurrentProcessId());
            
            Process_NotifyProcess_Create(ProcessId, ParentId, PsGetCurrentProcessId(), NULL);

        } else {

            Process_NotifyProcess_Delete(ProcessId);
        }
    }
}


//---------------------------------------------------------------------------
// Process_NotifyProcess_Create
//---------------------------------------------------------------------------


_FX void Process_NotifyProcess_Create(
    HANDLE ProcessId, HANDLE ParentId, HANDLE CallerId, BOX *box)
{
    void *nbuf1, *nbuf2;
    ULONG nlen1, nlen2;
    WCHAR *nptr1, *nptr2;
    const WCHAR *ImagePath;
    BOOLEAN parent_was_start_exe = FALSE;
    BOOLEAN parent_had_rights_dropped = FALSE;
    BOOLEAN process_is_forced = FALSE;
    BOOLEAN add_process_to_job = FALSE;
    BOOLEAN bHostInject = FALSE;
    KIRQL irql;

    //
    // get image name for new process
    //

    Process_GetProcessName(
                Driver_Pool, (ULONG_PTR)ProcessId, &nbuf1, &nlen1, &nptr1);
    if (! nbuf1) {

        Process_CreateTerminated(ProcessId, -1);
        return;
    }

    ImagePath = ((UNICODE_STRING *)nbuf1)->Buffer;

    //
    // determine if new process should be sandboxed:
    //
    // 1.  if process was created by the thread in SbieSvc which called
    //     Process_Api_Start, in this case we are called by this API
    //     handler function, and the box argument is valid
    //
    // in all other cases we are coming from Process_NotifyProcess which
    // was called by the system, the box argument should be NULL.
    //
    // 2.  otherwise if parent process is sandboxed
    //
    // 3.  otherwise check if "disabled forced programs" is in effect
    //     for the session or for the program
    //
    // 4.  if dfp not in effect, check if program should be forced
    //

    if (ParentId == Api_ServiceProcessId) {

        //
        // 1.  process created by SbieSvc
        //

        add_process_to_job = TRUE;

    } else if (! box) {

        //
        // 2.  check if parent is sandboxed
        //

        BOOLEAN create_terminated = FALSE;
        BOOLEAN added_to_dfp_list = FALSE;
        BOOLEAN check_forced_program = FALSE;

        //
        // there are a couple of scenarios here
        // a. CallerId == ParentId boring, all's fine
        // b. Caller is sandboxed designated Parent is NOT sandboxed, 
        //      possible sandbox escape atempt
        // c. Caller is not sandboxed, designated Parent IS sandboxed, 
        //      service trying to start something on the behalf of a sandboxed process 
        //      eg. seclogon reacting to a runas request 
        //      in which case the created process must be sandboxed to
        //

        PROCESS *parent_proc = Process_Find(CallerId, &irql);
        if (!(parent_proc && !parent_proc->bHostInject) && CallerId != ParentId) {
            
            //
            // release lock on process list
            //

            ExReleaseResourceLite(Process_ListLock);
            KeLowerIrql(irql);

            //
            // Process_Find will lock process list again
            //

            parent_proc = Process_Find(ParentId, &irql);
        }

        if (parent_proc && !parent_proc->bHostInject) {

            //
            // since this notify is called only when the first thread is
            // created for the new process, it is possible for process A
            // to create a new process N, but let process B create the
            // new thread.  then we are invoked here in the context of
            // process B, but we really need information from process A,
            // which can terminate itself unexpectedly, and will thus
            // cause us to crash.
            //
            // to work around this, we have Process_Find return with the
            // process list still locked, so process A can't die, and
            // then we clone process A's box before going further
            //

            if (! parent_proc->terminated) {

                box = Box_Clone(Driver_Pool, parent_proc->box);
                if (box) {

                    if (parent_proc->is_start_exe)
                        parent_was_start_exe = TRUE;

                    if (parent_proc->rights_dropped)
                        parent_had_rights_dropped = TRUE;

                } else
                    create_terminated = TRUE;

            } else
                create_terminated = TRUE;

        } else {

            //
            // 3.  if parent process is not sandboxed, then it might be
            // in the list of disabled forced processes, in that case
            // add the new process to that same list
            //

            added_to_dfp_list = Process_DfpInsert(ParentId, ProcessId);

            if (! added_to_dfp_list)
                check_forced_program = TRUE;
        }

        //
        // release lock on process list
        //

        ExReleaseResourceLite(Process_ListLock);
        KeLowerIrql(irql);

        if (check_forced_program) {

            //
            // 4.  if parent process is not in the dfp list, then
            // check if it might be a forced process
            //

            box = Process_GetForcedStartBox(ProcessId, ParentId, ImagePath, FALSE);

            if (box == (BOX *)-1) {

                create_terminated = TRUE;
                box = NULL;

            } else if (box) {

                process_is_forced = TRUE;
                add_process_to_job = TRUE;
            }
            else
            {
                box = Process_GetForcedStartBox(ProcessId, ParentId, ImagePath, TRUE);

                if (box == (BOX *)-1) {

                    create_terminated = TRUE;
                    box = NULL;

                }
                else if (box) {

                    bHostInject = TRUE;
                    add_process_to_job = FALSE;

                }
            }
        }

        //
        // if parent is a sandboxed process but for some reason we don't
        // have a box at this point, then terminate the new process
        //

        if (create_terminated) {

            Process_CreateTerminated(ProcessId, -1);
        }

        //
        // if the new process was added to the 'dfp' list because
        // its parent already appears in the 'dfp' list, then check
        // for matching process name
        //

        if (added_to_dfp_list) {

            BOOLEAN same_image_name = FALSE;

            Process_GetProcessName(
                Driver_Pool, (ULONG_PTR)ParentId, &nbuf2, &nlen2, &nptr2);

            if (nbuf2) {

                if (_wcsicmp(nptr1, nptr2) == 0)
                    same_image_name = TRUE;

                Mem_Free(nbuf2, nlen2);
            }

            if (same_image_name)
                Process_DfpCheck(ProcessId, &same_image_name);
        }
    }

    //
    // allocate process object for new process
    //

    if (box) {

        PROCESS *new_proc = Process_Create(ProcessId, box, ImagePath, &irql);
        Box_Free(box);

        if (new_proc) {

            HANDLE pid = new_proc->pid;
            ULONG64 create_time = new_proc->create_time;
            ULONG session_id = new_proc->box->session_id;

            new_proc->bHostInject = bHostInject;
            new_proc->parent_was_start_exe = parent_was_start_exe;
            new_proc->rights_dropped = parent_had_rights_dropped;
            new_proc->forced_process = process_is_forced;

            if (! bHostInject)
            {
				WCHAR msg[48], *buf = msg;
				RtlStringCbPrintfW(buf, sizeof(msg), L"%s%c%d", new_proc->box->name, L'\0', (ULONG)ParentId);
                buf += wcslen(buf) + 1;
				Log_Popup_MsgEx(MSG_1399, new_proc->image_path, wcslen(new_proc->image_path), msg, (ULONG)(buf - msg), new_proc->box->session_id, ProcessId);

                if (! add_process_to_job)
                    new_proc->parent_was_sandboxed = TRUE;

                //
                // don't put the process into a job if OpenWinClass=*
                //

				if (new_proc->open_all_win_classes || Conf_Get_Boolean(new_proc->box->name, L"NoAddProcessToJob", 0, FALSE)) {

					add_process_to_job = FALSE;
				}

                //
                // on Windows Vista, a forced process may start inside a
                // Program Compatibility Assistant (PCA) job, if its parent
                // process was in a PCA job.  in this case we can't put it
                // into a job in SbieSvc GuiServer::InitProcess, instead
                // we have SbieDll restart the process without a job.
                // see also Proc_RestartProcessOutOfPcaJob in core/dll/proc.c
                //

                if (add_process_to_job && process_is_forced &&
                        (Driver_OsVersion >= DRIVER_WINDOWS_VISTA) &&
                        Process_IsInPcaJob(pid)) {

                    add_process_to_job = FALSE;

                    new_proc->in_pca_job = TRUE;
                    new_proc->untouchable = TRUE;
                }
            }

            //
            // if we are called by Process_Api_Start rather than by
            // Process_NotifyProcess, then the process is not locked by
            // the system, and our process structure can disappear at
            // any time if Process_NotifyProcess_Delete is invoked.
            // we should only use local variables and not touch new_proc
            //

            ExReleaseResourceLite(Process_ListLock);
            KeLowerIrql(irql);

            Process_Low_Inject(
                pid, session_id, create_time, nptr1, add_process_to_job, bHostInject);
        }
    }

    Mem_Free(nbuf1, nlen1);
}


//---------------------------------------------------------------------------
// Process_NotifyProcess_Delete
//---------------------------------------------------------------------------


_FX void Process_NotifyProcess_Delete(HANDLE ProcessId)
{
    if (ProcessId == Api_ServiceProcessId)
        Api_ResetServiceProcess();

    Process_Delete(ProcessId);
    Session_Cancel(ProcessId);
}


//---------------------------------------------------------------------------
// Process_Delete
//---------------------------------------------------------------------------


_FX void Process_Delete(HANDLE ProcessId)
{
    PROCESS *proc;
    KIRQL irql;

    //
    // find the sandboxed process with the specified process id
    //

    proc = NULL;

    KeRaiseIrql(APC_LEVEL, &irql);
    ExAcquireResourceExclusiveLite(Process_ListLock, TRUE);

    proc = List_Head(&Process_List);
    while (proc) {
        if (proc->pid == ProcessId) {
            List_Remove(&Process_List, proc);
            break;
        }
        proc = (PROCESS *)List_Next(proc);
    }

    Process_DfpDelete(ProcessId);

    ExReleaseResourceLite(Process_ListLock);
    KeLowerIrql(irql);

    if (proc) {

        if (proc->pool == Driver_Pool)
            Mem_Free(proc, sizeof(PROCESS));

        else {

            //
            // process was found to be sandboxed:  it was already unlinked
            // from Process_List.  we have to do some process clean-up
            //

            Key_UnmountHive(proc);

            if (proc->file_lock)
                Mem_FreeLockResource(&proc->file_lock);
            if (proc->key_lock)
                Mem_FreeLockResource(&proc->key_lock);
            if (proc->ipc_lock)
                Mem_FreeLockResource(&proc->ipc_lock);
            if (proc->gui_lock)
                Mem_FreeLockResource(&proc->gui_lock);

			Token_ResetPrimary(proc);

            Thread_ReleaseProcess(proc);

            Token_ReleaseProcess(proc);

            Pool_Delete(proc->pool);
        }
    }
}


//---------------------------------------------------------------------------
// Process_NotifyImage
//---------------------------------------------------------------------------


_FX void Process_NotifyImage(
    const UNICODE_STRING *FullImageName,
    HANDLE ProcessId, IMAGE_INFO *ImageInfo)
{
    static const WCHAR *_Ntdll32 = L"\\syswow64\\ntdll.dll";    // 19 chars
    PROCESS *proc;
    ULONG fail = 0;

    //
    // the notify routine is invoked for any image mapped for any purpose.
    // we don't care about system images here
    //

    if ((! ProcessId) || ImageInfo->SystemModeImage)
        return;

    //
    // if the process was allocated by Process_NotifyProcess_Create but
    // not fully initialized yet then initialize it now
    //

    proc = Process_Find(ProcessId, NULL);

    if ((! proc) || proc->initialized) {

#ifdef _WIN64

        if (proc && (! proc->ntdll32_base)) {

            // DbgPrint("Image %*.*S Base %p\n", FullImageName->Length/sizeof(WCHAR),FullImageName->Length/sizeof(WCHAR),FullImageName->Buffer, ImageInfo->ImageBase);

            if (FullImageName->Length > 19 * sizeof(WCHAR)) {

                WCHAR *path = FullImageName->Buffer
                            + FullImageName->Length / sizeof(WCHAR)
                            - 19;
                if (_wcsicmp(path, _Ntdll32) == 0) {

                    proc->ntdll32_base =
                        (ULONG)(ULONG_PTR)ImageInfo->ImageBase;
                }
            }
        }

#endif _WIN64

        return;
    }

    if (proc->terminated)
        return;

    //
    // create the sandbox space
    //

    if (!proc->bHostInject)
    {
		if (!fail && !File_CreateBoxPath(proc))
			fail = 0x01;

        if (!fail && !Ipc_CreateBoxPath(proc))
			fail = 0x02;

        if (!fail && !Key_MountHive(proc))
			fail = 0x03;

        //
        // initialize the filtering components
        //

        if (!fail && !File_InitProcess(proc))
			fail = 0x04;

        if (!fail && !Key_InitProcess(proc))
			fail = 0x05;

        if (!fail && !Ipc_InitProcess(proc))
			fail = 0x06;

        if (!fail && !Gui_InitProcess(proc))
			fail = 0x07;

        if (!fail && !Process_Low_InitConsole(proc))
			fail = 0x08;

		if (!fail && !Token_ReplacePrimary(proc))
			fail = 0x09;

        if (!fail && !Thread_InitProcess(proc))
			fail = 0x0A;
    }

    //
    // terminate process if initialization failed
    //

    if (!fail && !Ipc_IsRunRestricted(proc)) {

        proc->initialized = TRUE;

    } else {

		if (fail)
			Log_Status_Ex_Process(MSG_1231, 0xA0 + fail, STATUS_UNSUCCESSFUL, NULL, proc->box->session_id, proc->pid);

        proc->terminated = TRUE;
		proc->reason = (!fail) ? -1 : 0;
        Process_CancelProcess(proc);
    }

    //DbgPrint("IMAGE LOADED, PROCESS INITIALIZATION %d COMPLETE %d\n", proc->pid, ok);
}


void Process_SetTerminated(PROCESS *proc, ULONG reason)
{
    if (!proc->terminated)
    {
        proc->terminated = TRUE;
        proc->reason = reason;
    }
}