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
#ifndef _M_ARM64
#include "hook.h"
#endif
#include "session.h"
#include "gui.h"
#include "token.h"
#include "thread.h"
#include "wfp.h"
#include "common/my_version.h"
#define KERNEL_MODE
#include "verify.h"
#include "dyn_data.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


ULONG Process_GetTraceFlag(PROCESS *proc, const WCHAR *setting);

#ifdef XP_SUPPORT
#ifndef _WIN64

static NTSTATUS Process_HookProcessNotify(
    PCREATE_PROCESS_NOTIFY_ROUTINE NewNotifyRoutine);

#endif _WIN64

static void Process_NotifyProcess(
    HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create);
#endif

static void Process_NotifyProcessEx(
    PEPROCESS ParentId, HANDLE ProcessId, PPS_CREATE_NOTIFY_INFO CreateInfo);

static PROCESS *Process_Create(
    HANDLE ProcessId, const BOX *box, const WCHAR *image_path,
    KIRQL *out_irql);

static void Process_NotifyProcess_Delete(HANDLE ProcessId);

static void Process_Delete(HANDLE ProcessId);

static void Process_NotifyImage(
    const UNICODE_STRING *FullImageName,
    HANDLE ProcessId, IMAGE_INFO *ImageInfo);

static NTSTATUS Process_CreateUserProcess(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args);

//---------------------------------------------------------------------------


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, Process_Init)
#ifdef XP_SUPPORT
#ifndef _WIN64
#pragma alloc_text (INIT, Process_HookProcessNotify)
#endif _WIN64
#endif
#endif // ALLOC_PRAGMA


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


#ifdef USE_PROCESS_MAP
HASH_MAP Process_Map;
HASH_MAP Process_MapDfp;
HASH_MAP Process_MapFcp;
#else
LIST Process_List;
LIST Process_ListDfp;
LIST Process_ListFcp;
#endif
PERESOURCE Process_ListLock = NULL;

static BOOLEAN Process_NotifyImageInstalled = FALSE;
static BOOLEAN Process_NotifyProcessInstalled = FALSE;

volatile BOOLEAN Process_ReadyToSandbox = FALSE;

#ifdef XP_SUPPORT
#ifndef _WIN64

static PCREATE_PROCESS_NOTIFY_ROUTINE *Process_pOldNotifyProcess = NULL;

#endif _WIN64
#endif


//---------------------------------------------------------------------------
// Process_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Process_Init(void)
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;

#ifdef USE_PROCESS_MAP
    map_init(&Process_Map, Driver_Pool);
	map_resize(&Process_Map, 128); // prepare some buckets for better performance

    map_init(&Process_MapDfp, Driver_Pool);
	map_resize(&Process_MapDfp, 128); // prepare some buckets for better performance

    map_init(&Process_MapFcp, Driver_Pool);
	map_resize(&Process_MapFcp, 128); // prepare some buckets for better performance
#else
    List_Init(&Process_List);
    List_Init(&Process_ListDfp);
    List_Init(&Process_ListFcp);
#endif

    if (! Mem_GetLockResource(&Process_ListLock, TRUE))
        return FALSE;

    if (! Process_Low_Init())
        return FALSE;

    //
    // install process notify routines
    //

    if (Driver_OsVersion >= DRIVER_WINDOWS_7) {

        status = PsSetCreateProcessNotifyRoutineEx(Process_NotifyProcessEx, FALSE);
    }
#ifdef XP_SUPPORT
    else { // XP, Vista

        status = PsSetCreateProcessNotifyRoutine(Process_NotifyProcess, FALSE);
    }

#ifndef _WIN64

    if ((status == STATUS_INVALID_PARAMETER) &&
                    (Driver_OsVersion < DRIVER_WINDOWS_VISTA)) {

        status = Process_HookProcessNotify(Process_NotifyProcess);
    }

#endif _WIN64
#endif

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
    // set syscalls handlers that are applicable in Vista and later
    // Note: NtCreateProcess/NtCreateProcessEx seam not to be used
    //

    if (! Syscall_Set1("CreateUserProcess", Process_CreateUserProcess))
        return FALSE;

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
    Api_SetFunction(API_KILL_PROCESS,         Process_Api_Kill);

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

        if (Driver_OsVersion >= DRIVER_WINDOWS_7) {

            PsSetCreateProcessNotifyRoutineEx(Process_NotifyProcessEx, TRUE);
        }
#ifdef XP_SUPPORT
        else { // XP, Vista

            PsSetCreateProcessNotifyRoutine(Process_NotifyProcess, TRUE);
        }

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
#endif

        Process_NotifyProcessInstalled = FALSE;

    }

    Process_Low_Unload();

    if (FreeLock)
        Mem_FreeLockResource(&Process_ListLock);
}


#ifdef XP_SUPPORT
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
#endif


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

#ifdef USE_PROCESS_MAP
    proc = map_get(&Process_Map, ProcessId);
    if (proc) {
#else
    proc = List_Head(&Process_List);
    while (proc) {
        if (proc->pid == ProcessId) {
#endif

            if (check_terminated && proc->terminated) {
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

#ifndef USE_PROCESS_MAP
            break;
        }

        proc = List_Next(proc);
#endif
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

#ifdef XP_SUPPORT
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
#endif

//---------------------------------------------------------------------------
// Process_Find_ByHandle
//---------------------------------------------------------------------------


//_FX PROCESS *Process_Find_ByHandle(HANDLE Handle, KIRQL *out_irql)
//{
//    NTSTATUS Status;
//    PEPROCESS ProcessObject = NULL;
//    PROCESS* Process = NULL;
//    
//    Status = ObReferenceObjectByHandle(Handle, PROCESS_QUERY_INFORMATION, *PsProcessType, UserMode, (PVOID*)&ProcessObject, NULL);
//    if (NT_SUCCESS(Status)) {
//
//        Process = Process_Find(PsGetProcessId(ProcessObject), out_irql);
//
//        // Dereference the process object
//        ObDereferenceObject(ProcessObject);
//    }
//
//    return Process;
//}


//---------------------------------------------------------------------------
// Process_CreateTerminated
//---------------------------------------------------------------------------


_FX void Process_CreateTerminated(HANDLE ProcessId, ULONG SessionId)
{
    UNICODE_STRING pid_str;
    PROCESS *proc;
    KIRQL irql;

    if (SessionId != -1) { // for StartRunAlertDenied, don't log in this case
    
        pid_str.Length = 10 * sizeof(WCHAR);
        pid_str.MaximumLength = pid_str.Length + sizeof(WCHAR);

        pid_str.Buffer = Mem_Alloc(Driver_Pool, pid_str.MaximumLength);
        if (pid_str.Buffer) {

            RtlIntPtrToUnicodeString((ULONG_PTR)ProcessId, 10, &pid_str);
            Log_Msg_Process(MSG_1211, pid_str.Buffer, NULL, SessionId, ProcessId);

            Mem_Free(pid_str.Buffer, pid_str.MaximumLength);
        }
    }

    proc = Mem_Alloc(Driver_Pool, sizeof(PROCESS));
    if (proc) {

        memzero(proc, sizeof(PROCESS));
        proc->pid = ProcessId;
        proc->pool = Driver_Pool;
        Process_SetTerminated(proc, 1);

        KeRaiseIrql(APC_LEVEL, &irql);
        ExAcquireResourceExclusiveLite(Process_ListLock, TRUE);

#ifdef USE_PROCESS_MAP
        map_insert(&Process_Map, ProcessId, proc, 0);
#else
        List_Insert_After(&Process_List, NULL, proc);
#endif

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

    proc->detected_image_type = -1; // indicate non initialized

    //
    // initialize image name from image path
    //

    if (image_path) {

        Process_IsSbieImage(image_path, &proc->image_sbie, &proc->is_start_exe);

        UNICODE_STRING image_uni;
        RtlInitUnicodeString(&image_uni, image_path);
        if (Box_IsBoxedPath(proc->box, file, &image_uni)) {

            proc->image_from_box = TRUE;
        }

        WCHAR *image_name = wcsrchr(image_path, L'\\');
        if (image_name) {
            ++image_name;

            ULONG len = wcslen(image_name);
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
    // initialize box options
    //

    proc->bAppCompartment = Conf_Get_Boolean(proc->box->name, L"NoSecurityIsolation", 0, FALSE);

    //
    // by default, Close[...]=!<program>,path includes all boxed images
    // use AlwaysCloseForBoxed=n to disable this behaviour
    //

    proc->always_close_for_boxed = !proc->bAppCompartment && Conf_Get_Boolean(proc->box->name, L"AlwaysCloseForBoxed", 0, TRUE); 

    //
    // by default OpenFile and OpenKey apply only to unboxed processes
    // use DontOpenForBoxed=n to thread boxed and unboxed programs the same way
    //

    proc->dont_open_for_boxed = !proc->bAppCompartment && Conf_Get_Boolean(proc->box->name, L"DontOpenForBoxed", 0, TRUE); 

    //
    // Sandboxie attempts to protect per process rules by allowing them only for host binaries
    // this however has an obvious weakness, as those processes can still load boxed DLL's
    // with this option we can prevent that
    //

    proc->protect_host_images = !proc->bAppCompartment && Conf_Get_Boolean(proc->box->name, L"ProtectHostImages", 0, FALSE); 

    //
    // privacy mode requirers Rule Specificity
    //

    proc->use_security_mode = Conf_Get_Boolean(proc->box->name, L"UseSecurityMode", 0, FALSE);
    proc->is_locked_down = proc->use_security_mode || Conf_Get_Boolean(proc->box->name, L"SysCallLockDown", 0, FALSE);
    proc->open_all_nt = Conf_Get_Boolean(proc->box->name, L"OpenAllSysCalls", 0, FALSE);
#ifdef USE_MATCH_PATH_EX
    proc->restrict_devices = proc->use_security_mode || Conf_Get_Boolean(proc->box->name, L"RestrictDevices", 0, FALSE);

    proc->use_privacy_mode = Conf_Get_Boolean(proc->box->name, L"UsePrivacyMode", 0, FALSE); 
    proc->use_rule_specificity = proc->restrict_devices || proc->use_privacy_mode || Conf_Get_Boolean(proc->box->name, L"UseRuleSpecificity", 0, FALSE); 
#endif
    proc->confidential_box = Conf_Get_Boolean(proc->box->name, L"ConfidentialBox", 0, FALSE); 

    //
    // check certificate
    //

    if (!(Verify_CertInfo.active && Verify_CertInfo.opt_sec) && !proc->image_sbie) {

        const WCHAR* exclusive_setting = NULL;
        if (proc->use_security_mode)
            exclusive_setting = L"UseSecurityMode";
        else if (proc->is_locked_down)
            exclusive_setting = L"SysCallLockDown";
        else if (proc->restrict_devices)
            exclusive_setting = L"RestrictDevices";
        else
#ifdef USE_MATCH_PATH_EX
        if (proc->use_rule_specificity)
            exclusive_setting = L"UseRuleSpecificity";
        else if (proc->use_privacy_mode)
            exclusive_setting = L"UsePrivacyMode";
        else
#endif
        if (proc->bAppCompartment)
            exclusive_setting = L"NoSecurityIsolation";
        else if (proc->protect_host_images)
            exclusive_setting = L"ProtectHostImages";

        if (exclusive_setting) {

            Log_Msg_Process(MSG_6004, proc->box->name, exclusive_setting, box->session_id, proc->pid);

            // allow the process to run for a sort while to allow the features to be evaluated
            Process_ScheduleKill(proc, 5*60*1000); // 5 minutes
        }
    }

    if (!(Verify_CertInfo.active && Verify_CertInfo.opt_enc) && !proc->image_sbie) {
        
        const WCHAR* exclusive_setting = NULL;
        if (proc->confidential_box)
            exclusive_setting = L"ConfidentialBox";

        if (exclusive_setting) {

            Log_Msg_Process(MSG_6009, proc->box->name, exclusive_setting, box->session_id, proc->pid);

            Pool_Delete(pool);
            Process_CreateTerminated(ProcessId, box->session_id);
            return NULL;
        }
    }

    //
    // If we don't have valid Dyndata, we force NoSecurityIsolation=y on all boxes
    // and issue a security warning MSG_1207
    //

    if (!Dyndata_Active && !proc->bAppCompartment) {

        proc->bAppCompartment = TRUE;
		proc->always_close_for_boxed = FALSE;
		proc->dont_open_for_boxed = FALSE;
		proc->protect_host_images = FALSE;

        WCHAR info[12];
        RtlStringCbPrintfW(info, sizeof(info), L"%d", Driver_OsBuild);
        Log_Msg1(MSG_1207, info);
    }

    //
    // configure monitor options
    //

    proc->disable_monitor = Conf_Get_Boolean(proc->box->name, L"DisableResourceMonitor", 0, FALSE);

    //
    // initialize filtering options
    //

    BOOLEAN no_filtering = proc->bAppCompartment && Conf_Get_Boolean(proc->box->name, L"NoSecurityFiltering", 0, FALSE); // only in effect in app mode
    proc->disable_file_flt = no_filtering || Conf_Get_Boolean(proc->box->name, L"DisableFileFilter", 0, FALSE);
    proc->disable_key_flt = no_filtering || Conf_Get_Boolean(proc->box->name, L"DisableKeyFilter", 0, FALSE);
    proc->disable_object_flt = no_filtering || Conf_Get_Boolean(proc->box->name, L"DisableObjectFilter", 0, FALSE);

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

#ifdef USE_PROCESS_MAP
    map_insert(&Process_Map, ProcessId, proc, 0);
#else
    List_Insert_After(&Process_List, NULL, proc);
#endif

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


#ifdef XP_SUPPORT
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
        return TRUE;

    //
    // handle process creation and deletion.  note that we are running
    // in an arbitrary thread context
    //

    if (ProcessId) {

        if (Create) {

            //
            // it is possible to specify the parent process when calling RtlCreateUserProcess
            // this is for example done by the appinfo service running under svchost.exe
            // to start LocalBridge.exe with RuntimeBroker.exe as parent
            // hence we take for our purposes the ID of the process calling RtlCreateUserProcess instead
            //

            //DbgPrint("Process_NotifyProcess_Create pid=%d parent=%d current=%d\n", ProcessId, ParentId, PsGetCurrentProcessId());
            
            if (!Process_NotifyProcess_Create(ProcessId, ParentId, PsGetCurrentProcessId(), NULL)) {

                //
                // Note: the process is already marked for termination so we don't need to do anything
                //          in case one would want to schedule an explicit termination, the code below can be used
                //

                /*
                PEPROCESS ProcessObject;
                ULONG session_id;
                ULONG64 create_time;

                ProcessObject = Process_OpenAndQuery(ProcessId, NULL, &session_id);
                if (ProcessObject) {
    
                    create_time = PsGetProcessCreateTimeQuadPart(ProcessObject);
                    ObDereferenceObject(ProcessObject);
                }
                
                void *nbuf1;
                ULONG nlen1;
                WCHAR *nptr1;

                Process_GetProcessName(
                            Driver_Pool, (ULONG_PTR)ProcessId, &nbuf1, &nlen1, &nptr1);

                if (1) {

                    BOX dummy_box;
                    PROCESS dummy_proc;
                    memzero(&dummy_box, sizeof(dummy_box));
                    memzero(&dummy_proc, sizeof(dummy_proc));
                    dummy_box.session_id = session_id;
                    dummy_proc.box = &dummy_box;
                    dummy_proc.pid = ProcessId;
                    dummy_proc.create_time = create_time;
                    dummy_proc.image_name = (WCHAR*)nptr1;

                    Process_TerminateProcess(&dummy_proc);
                }

                Mem_Free(nbuf1, nlen1);
                */
            }

        } else {

            Process_NotifyProcess_Delete(ProcessId);
        }
    }
}
#endif


//---------------------------------------------------------------------------
// Process_NotifyProcessEx
//---------------------------------------------------------------------------


_FX void Process_NotifyProcessEx(
    PEPROCESS Process, HANDLE ProcessId, PPS_CREATE_NOTIFY_INFO CreateInfo)
{
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

        if (CreateInfo != NULL) {

            //
            // it is possible to specify the parent process when calling RtlCreateUserProcess
            // this is for example done by the appinfo service running under svchost.exe
            // to start LocalBridge.exe with RuntimeBroker.exe as parent
            // hence we take for our purposes the ID of the process calling RtlCreateUserProcess instead
            //

            //DbgPrint("Process_NotifyProcess_Create pid=%d parent=%d current=%d\n", ProcessId, CreateInfo->ParentProcessId, PsGetCurrentProcessId());
            
            if (!Process_NotifyProcess_Create(ProcessId, CreateInfo->ParentProcessId, PsGetCurrentProcessId(), NULL)) {

                CreateInfo->CreationStatus = STATUS_ACCESS_DENIED;
            }

        } else {

            Process_NotifyProcess_Delete(ProcessId);
        }
    }
}


//---------------------------------------------------------------------------
// Process_NotifyProcess_Create
//---------------------------------------------------------------------------


_FX BOOLEAN Process_NotifyProcess_Create(
    HANDLE ProcessId, HANDLE ParentId, HANDLE CallerId, BOX *box)
{
    void *nbuf1, *nbuf2;
    ULONG nlen1, nlen2;
    WCHAR *nptr1, *nptr2;
    const WCHAR *ImagePath;
    BOOLEAN parent_was_start_exe = FALSE;
    BOOLEAN parent_had_rights_dropped = FALSE;
    BOOLEAN parent_was_image_from_box = FALSE;
    BOOLEAN process_is_forced = FALSE;
    BOOLEAN add_process_to_job = FALSE;
	BOOLEAN create_terminated = FALSE;
    BOOLEAN bHostInject = FALSE;
    KIRQL irql;

    //
    // get image name for new process
    //

    Process_GetProcessName(
                Driver_Pool, (ULONG_PTR)ProcessId, &nbuf1, &nlen1, &nptr1);
    if (! nbuf1) {

        Process_CreateTerminated(ProcessId, -1);
        return FALSE;
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

        BOOLEAN added_to_dfp_list = FALSE;
        BOOLEAN check_forced_program = FALSE;

        //
        // there are a couple of scenarios here
        // a. CallerId == ParentId boring, all's fine
        // b. Caller is sandboxed designated Parent is NOT sandboxed, 
        //      possible sandbox escape attempt
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

                    if (parent_proc->image_from_box)
                        parent_was_image_from_box = TRUE;

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

#ifdef DRV_BREAKOUT
        //
        // check if this process is set up as break out program,
        // it mustn't be located in a sandboxed for this to work.
        //

        BOX* breakout_box = NULL;

        if (box && Process_IsBreakoutProcess(box, ImagePath)) {
            if(!Verify_CertInfo.active)
                Log_Msg_Process(MSG_6004, box->name, L"BreakoutProcess", box->session_id, CallerId);
            else {
                UNICODE_STRING image_uni;
                RtlInitUnicodeString(&image_uni, ImagePath);
                if (!Box_IsBoxedPath(box, file, &image_uni)) {

                    check_forced_program = TRUE; // the breakout process of one box may be the forced process of another
                    breakout_box = box;
                    box = NULL;
                }
            }
        }
#endif

        //
        // check forced processes
        //

        if (check_forced_program) {

            //
            // 4.  if parent process is not in the dfp list, then
            // check if it might be a forced process
            //

            const WCHAR* pSidString = NULL;
#ifdef DRV_BREAKOUT
            if (breakout_box)
                pSidString = breakout_box->sid;
#endif
            box = Process_GetForcedStartBox(ProcessId, ParentId, ImagePath, &bHostInject, pSidString);

            if (box == (BOX *)-1) {

                create_terminated = TRUE;
                box = NULL;

            } else if (box) {

                if (bHostInject) {

                    add_process_to_job = FALSE;

                } else {

                    process_is_forced = TRUE;
                    add_process_to_job = TRUE;
                }
            }
        }

#ifdef DRV_BREAKOUT
        //
        // if this is a break out process and no other box clamed it as forced, 
        // set bHostInject and threat it accordingly, we need this in order for
        // the custom SetInformationProcess call from CreateProcessInternalW to succeed
        //

        if (breakout_box) {
            if (!box) {
                bHostInject = TRUE;
                add_process_to_job = FALSE;
                box = breakout_box;
            }
            else {
                Box_Free(breakout_box);
                breakout_box = NULL;
            }
        }
#endif

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
        if (!new_proc) {
		
            create_terminated = TRUE;
		}
        else if (!new_proc->image_from_box && new_proc->protect_host_images && parent_was_image_from_box) {

            create_terminated = TRUE;

            Process_SetTerminated(new_proc, 14);
            new_proc = NULL;

            ExReleaseResourceLite(Process_ListLock);
            KeLowerIrql(irql);
        }
        Box_Free(box);

        if (new_proc) {

            HANDLE pid = new_proc->pid;
            ULONG64 create_time = new_proc->create_time;
            ULONG session_id = new_proc->box->session_id;

            new_proc->bHostInject = bHostInject;
            new_proc->starter_id = CallerId;
            new_proc->parent_was_start_exe = parent_was_start_exe;
            new_proc->rights_dropped = parent_had_rights_dropped;
            new_proc->forced_process = process_is_forced;

            if (! bHostInject) {

                //
                // Notify the agent about the new process using a specialized silent message
                //

				WCHAR sParentId[12];
                _ultow_s((ULONG)ParentId, sParentId, 12, 10);

                WCHAR *Buffer;
                ULONG Length;
                Process_GetCommandLine(ParentId, &Buffer, &Length);

                const WCHAR* strings[5] = { new_proc->image_path, new_proc->box->name, sParentId, Buffer, NULL };
                Api_AddMessage(MSG_1399, strings, NULL, new_proc->box->session_id, (ULONG)ProcessId);

                if (Buffer && Length)
                    Mem_Free(Buffer, Length);

                //
                //
                //

                if (! add_process_to_job)
                    new_proc->parent_was_sandboxed = TRUE;

                add_process_to_job = TRUE; // we need this because of JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK used in GuiServer::GetJobObjectForAssign

                //
                // don't put the process into a job if OpenWinClass=*
                // don't put the process into a job if NoSecurityIsolation=y
                //

				if (new_proc->open_all_win_classes || new_proc->bAppCompartment || Conf_Get_Boolean(new_proc->box->name, L"NoAddProcessToJob", 0, FALSE)) {

                    new_proc->can_use_jobs = TRUE;
					add_process_to_job = FALSE;
				}
                else if (Driver_OsVersion >= DRIVER_WINDOWS_8) {

                    //
                    // on windows 8 and later we can have nested jobs so assigning a 
                    // boxed job to a process will not interfere with the job assigned by SbieSvc
                    //

                    new_proc->can_use_jobs = Conf_Get_Boolean(new_proc->box->name, L"AllowBoxedJobs", 0, FALSE);
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

            if (!Process_Low_Inject(
                pid, session_id, create_time, nptr1, add_process_to_job, bHostInject)) {

                create_terminated = TRUE;

                //
                // re acquire the process object, and mark it terminated
                //

                new_proc = Process_Find(ProcessId, &irql);

                if (new_proc)
                    Process_SetTerminated(new_proc, 2);

                ExReleaseResourceLite(Process_ListLock);
                KeLowerIrql(irql);
            }
        }
    }

    Mem_Free(nbuf1, nlen1);

    return create_terminated == FALSE;
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

#ifdef USE_PROCESS_MAP
    map_take(&Process_Map, ProcessId, &proc, 0);
#else
    proc = List_Head(&Process_List);
    while (proc) {
        if (proc->pid == ProcessId) {
            List_Remove(&Process_List, proc);
            break;
        }
        proc = (PROCESS *)List_Next(proc);
    }
#endif

    Process_DfpDelete(ProcessId);

    Process_FcpDelete(ProcessId);

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

            WFP_DeleteProcess(proc);

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

        if (!fail && !WFP_InitProcess(proc))
			fail = 0x0B;

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
        Process_TerminateProcess(proc);
    }

    //DbgPrint("IMAGE LOADED, PROCESS INITIALIZATION %d COMPLETE %d\n", proc->pid, ok);
}


//---------------------------------------------------------------------------
// Process_SetTerminated
//---------------------------------------------------------------------------


_FX void Process_SetTerminated(PROCESS *proc, ULONG reason)
{
    //
    // This function marks a process for termination, this causes File_PreOperation 
    // and Key_Callback to return STATUS_PROCESS_IS_TERMINATING which prevents 
    // the process form accessing the file system and the registry
    // 
    // Note: if this is set during process creation the process won't be able to start at all
    //

    if (!proc->terminated)
    {
        proc->terminated = TRUE;
        proc->reason = reason;
    }
}


//---------------------------------------------------------------------------
// Process_CreateUserProcess
//---------------------------------------------------------------------------


_FX NTSTATUS Process_CreateUserProcess(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args)
{
    THREAD* thrd = NULL;
    KIRQL irql;

    if (proc->protect_host_images) 
    {
        KeRaiseIrql(APC_LEVEL, &irql);
        ExAcquireResourceExclusiveLite(proc->threads_lock, TRUE);
		
		thrd = Thread_GetOrCreate(proc, NULL, TRUE);
        if (thrd)
            thrd->create_process_in_progress = TRUE;

        ExReleaseResourceLite(proc->threads_lock);
        KeLowerIrql(irql);	
    }

    NTSTATUS status = Syscall_Invoke(syscall_entry, user_args);

    if (thrd)
        thrd->create_process_in_progress = FALSE;

    return status;
}

