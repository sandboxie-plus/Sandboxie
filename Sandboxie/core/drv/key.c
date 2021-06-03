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
// Registry
//---------------------------------------------------------------------------


#include "key.h"
#include "process.h"
#include "obj.h"
#include "file.h"
#include "api.h"
#include "util.h"
#include "session.h"
#include "token.h"
#include "hook.h"
#include "conf.h"
#include "common/my_version.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define KEY_DENIED_ACCESS                                       \
    ~(STANDARD_RIGHTS_READ | GENERIC_READ | SYNCHRONIZE |       \
      KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS | KEY_NOTIFY |   \
      KEY_WOW64_32KEY | KEY_WOW64_64KEY)


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


struct _KEY_MOUNT {

    LIST_ELEM list_elem;

    WCHAR *hive_path;
    ULONG hive_path_len;

    WCHAR *root_key;
    ULONG root_key_len;

    volatile ULONG ref_count;
    volatile ULONG unmount_pending;
    volatile ULONG busy;

};


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN Key_InitPaths(PROCESS *proc);

static NTSTATUS Key_MyParseProc_2(OBJ_PARSE_PROC_ARGS_2);

static BOOLEAN Key_MountHive2(PROCESS *proc, KEY_MOUNT *mount);

static BOOLEAN Key_MountHive3(
    PROCESS *proc, OBJECT_ATTRIBUTES *target, OBJECT_ATTRIBUTES *source);


//---------------------------------------------------------------------------


NTSTATUS Key_Api_GetUnmountHive(PROCESS *proc, ULONG64 *parms);

NTSTATUS Key_Api_Open(PROCESS *proc, ULONG64 *parms);

NTSTATUS Key_Api_SetLowLabel(PROCESS *proc, ULONG64 *parms);


//---------------------------------------------------------------------------


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, Key_Init)
#endif // ALLOC_PRAGMA


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static LIST Key_Mounts;
static PERESOURCE Key_MountsLock = NULL;

static BOOLEAN Key_Have_KB979683 = FALSE;

const WCHAR *Key_Registry_Machine = L"\\REGISTRY\\MACHINE";

static BOOLEAN Key_NeverUnmountHives = FALSE;


//---------------------------------------------------------------------------
// Include code for registry filter
//---------------------------------------------------------------------------


#include "key_flt.c"


//---------------------------------------------------------------------------
// Include code for 32-bit Windows XP
//---------------------------------------------------------------------------


#ifndef _WIN64
#include "key_xp.c"
#endif _WIN64


//---------------------------------------------------------------------------
// Key_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Key_Init(void)
{
    //
    // install parse procedure hook on Windows XP/2003
    // register as a registry filter on Vista and later
    //

    typedef BOOLEAN (*P_Key_Init_2)(void);

    P_Key_Init_2 p_Key_Init_2 = Key_Init_Filter;

#ifndef _WIN64

    if (Driver_OsVersion < DRIVER_WINDOWS_VISTA) {

        p_Key_Init_2 = Key_Init_XpHook;
    }

#endif ! _WIN64

    if (! p_Key_Init_2())
        return FALSE;

    //
    // init key mounts data
    //

    List_Init(&Key_Mounts);

    if (! Mem_GetLockResource(&Key_MountsLock, TRUE))
        return FALSE;

    //
    // set API functions
    //

    Api_SetFunction(API_GET_UNMOUNT_HIVE,   Key_Api_GetUnmountHive);
    Api_SetFunction(API_OPEN_KEY,           Key_Api_Open);
    Api_SetFunction(API_SET_LOW_LABEL_KEY,  Key_Api_SetLowLabel);

    return TRUE;
}


//---------------------------------------------------------------------------
// Key_Unload
//---------------------------------------------------------------------------


_FX void Key_Unload(void)
{
    //
    // uninstall parse procedure hook on Windows XP
    // deregister as a registry filter on Vista and later
    //

    typedef void (*P_Key_Unload_2)(void);

    P_Key_Unload_2 p_Key_Unload_2 = Key_Unload_Filter;

#ifndef _WIN64

    if (Driver_OsVersion < DRIVER_WINDOWS_VISTA) {

        p_Key_Unload_2 = Key_Unload_XpHook;
    }

#endif ! _WIn64

    p_Key_Unload_2();

    //
    // free key mounts data
    //

    Mem_FreeLockResource(&Key_MountsLock);
}


//---------------------------------------------------------------------------
// Key_InitProcess
//---------------------------------------------------------------------------


_FX BOOLEAN Key_InitProcess(PROCESS *proc)
{
    static const WCHAR *_OpenPath = L"OpenKeyPath";
    static const WCHAR *_ClosedPath = L"ClosedKeyPath";
    static const WCHAR *_ReadPath = L"ReadKeyPath";
    static const WCHAR *_WritePath = L"WriteKeyPath";
    BOOLEAN ok;

    //
    // open paths
    //

    if (proc->image_from_box)
        ok = TRUE;
    else
        ok = Process_GetPaths(proc, &proc->open_key_paths, _OpenPath, TRUE);

    if (! ok) {
        Log_MsgP1(MSG_INIT_PATHS, _OpenPath, proc->pid);
        return FALSE;
    }

    //
    // closed paths
    //

    ok = Process_GetPaths(proc, &proc->closed_key_paths, _ClosedPath, TRUE);
    if (! ok) {
        Log_MsgP1(MSG_INIT_PATHS, _ClosedPath, proc->pid);
        return FALSE;
    }

    //
    // read-only paths (stored also as open paths)
    //

    ok = Process_GetPaths(proc, &proc->open_key_paths, _ReadPath, TRUE);
    if (ok)
        ok = Process_GetPaths(proc, &proc->read_key_paths, _ReadPath, TRUE);
    if (! ok) {
        Log_MsgP1(MSG_INIT_PATHS, _ReadPath, proc->pid);
        return FALSE;
    }

    //
    // write-only paths (stored also as closed paths)
    //

    if (Driver_OsVersion >= DRIVER_WINDOWS_XP) {

        ok = Process_GetPaths2(
                proc, &proc->write_key_paths, &proc->closed_key_paths,
                _WritePath, TRUE);
        if (ok) {
            ok = Process_GetPaths(
                    proc, &proc->closed_key_paths, _WritePath, TRUE);
        }
        if (! ok) {
            Log_MsgP1(MSG_INIT_PATHS, _WritePath, proc->pid);
            return FALSE;
        }
    }

    //
    // finish
    //

    return TRUE;
}


//---------------------------------------------------------------------------
// Key_MyParseProc_2
//---------------------------------------------------------------------------


_FX NTSTATUS Key_MyParseProc_2(OBJ_PARSE_PROC_ARGS_2)
{
    NTSTATUS status;
    OBJECT_NAME_INFORMATION *Name;
    ULONG NameLength;
    BOOLEAN IsBoxedPath;
    BOOLEAN ShouldMonitorAccess;
    BOOLEAN write_access;

    //
    // figure out the complete path.  deny access to unnamed objects
    //

    status = Obj_GetParseName(
        proc->pool, ParseObject, RemainingName, &Name, &NameLength);
    if (Name == &Obj_Unnamed)
        status = STATUS_ACCESS_DENIED;
    if (! NT_SUCCESS(status))
        return status;

    //
    // check if the specified path leads inside the box
    //

    IsBoxedPath = FALSE;

    ShouldMonitorAccess = FALSE;

    if (Box_IsBoxedPath(proc->box, key, &Name->Name))
        IsBoxedPath = TRUE;

    //
    // check desired access for write access.  if the call comes from
    // NtCreateKey, we consider this write access.  on most versions
    // of Windows, whether a Context pointer was provided, is the way
    // to differentiate NtCreateKey from NtOpenKey
    //

    write_access = FALSE;
    if (AccessState->OriginalDesiredAccess & KEY_DENIED_ACCESS)
        write_access = TRUE;

    if (Context) {

        if (Driver_OsVersion >= DRIVER_WINDOWS_2003) {

            //
            // on Windows 2003 and Windows Vista systems, a Context
            // structure is always passed.  bit 0 is set when the
            // call comes from NtCreateKey
            //

            if (*(ULONG *)Context & 1)
                write_access = TRUE;

        } else if (Key_Have_KB979683) {

            //
            // on Windows 2000/XP with KB979683 installed, a Context
            // structure is always passed.  byte 0x21 is set when
            // the call comes from NtCreateKey
            //

            if (*(((UCHAR *)Context) + 0x21) & 1)
                write_access = TRUE;

        } else      // Context is sent by NtCreateKey
            write_access = TRUE;
    }

    //
    // allow/deny rules:
    // if path leads inside the sandbox, we allow access
    //

    status = STATUS_SUCCESS;

    if (! IsBoxedPath) {

        LIST *open_key_paths;
        BOOLEAN is_open, is_closed;

        //
        // deny access in two cases:
        // - if unsandboxed path matches a closed path
        // - if unsandboxed path does not match an open path,
        //   and this is a write access
        //

        if (write_access)
            open_key_paths = &proc->open_key_paths;
        else
            open_key_paths = NULL;

        Process_MatchPath(
            proc->pool,
            Name->Name.Buffer, Name->Name.Length / sizeof(WCHAR),
            open_key_paths, &proc->closed_key_paths,
            &is_open, &is_closed);

        if (is_closed || (write_access && (! is_open))) {

            status = STATUS_ACCESS_DENIED;

            if (is_closed)
                ShouldMonitorAccess = TRUE;
        }

        //
        // read-only paths are also listed as open paths, so if we granted
        // write access to an open path, we need to check that this isn't
        // also a read-only path
        //

        if (write_access && is_open) {

            Process_MatchPath(
                proc->pool,
                Name->Name.Buffer, Name->Name.Length / sizeof(WCHAR),
                &proc->read_key_paths, NULL,
                &is_open, &is_closed);

            if (is_open) {

                status = STATUS_ACCESS_DENIED;
                ShouldMonitorAccess = TRUE;
            }
        }

        //
        // when Software Restriction Policies is in effect, ADVAPI32 opens
        // \REGISTRY\MACHINE for MAXIMUM_ALLOWED access even before SbieDll
        // was loaded, so permit those accesses
        //
        // note that since version 3.45, SbieDll takes over the job of
        // loading static import DLLs, however ADVAPI32 is loaded directly
        // by NTDLL even before manipulating static imports.
        //

        if (status == STATUS_ACCESS_DENIED && (! proc->sbiedll_loaded) &&
                Name->Name.Length == 17 * sizeof(WCHAR) &&
                _wcsicmp(Name->Name.Buffer, Key_Registry_Machine) == 0) {

            status = STATUS_SUCCESS;
        }
    }


    //
    // trace the request if so desired
    //

    if (proc->key_trace & (TRACE_ALLOW | TRACE_DENY)) {

        WCHAR access_str[24];
        WCHAR letter;

        if ((! NT_SUCCESS(status)) && (proc->key_trace & TRACE_DENY))
            letter = L'D';
        else if (NT_SUCCESS(status) && (proc->key_trace & TRACE_ALLOW))
            letter = L'A';
        else
            letter = 0;

        if (letter) {

            ULONG mon_type = MONITOR_KEY;
            if (!IsBoxedPath) {
                if (ShouldMonitorAccess == TRUE)
                    mon_type |= MONITOR_DENY;
                else
                    mon_type |= MONITOR_OPEN;
            }
            if (!ShouldMonitorAccess)
                mon_type |= MONITOR_TRACE;

            RtlStringCbPrintfW(access_str, sizeof(access_str), L"(K%c) %08X",
                letter, AccessState->OriginalDesiredAccess);
            Log_Debug_Msg(mon_type, access_str, Name->Name.Buffer);
        }
    }

    else if (ShouldMonitorAccess) {

        Session_MonitorPut(MONITOR_KEY | MONITOR_DENY, Name->Name.Buffer, proc->pid);
    }

    Mem_Free(Name, NameLength);

    return status;
}


//---------------------------------------------------------------------------
// Key_MountHive
//---------------------------------------------------------------------------


_FX BOOLEAN Key_MountHive(PROCESS *proc)
{
    static const WCHAR *_HiveFileName = L"\\RegHive";
    WCHAR *hive_path;
    ULONG hive_path_len;
    KEY_MOUNT *mount;
    BOOLEAN failed;
    KIRQL irql;

    //
    // this function should not be called twice on the same process
    //

    if (proc->key_mount)
        return TRUE;

    //
    // prepare the path to the registry hive data file
    //

    hive_path_len = proc->box->file_path_len    // includes NULL
                  + wcslen(_HiveFileName) * sizeof(WCHAR);
    hive_path = Mem_Alloc(Driver_Pool, hive_path_len);
    if (! hive_path)
        return FALSE;

    memcpy(hive_path, proc->box->file_path, proc->box->file_path_len);
    wcscat(hive_path, _HiveFileName);

        //DbgPrint("Searching for mount:\n");
        //DbgPrint(" ... %S (%d)\n", hive_path, hive_path_len);
        //DbgPrint(" ... %S (%d)\n", proc->box->key_path,proc->box->key_path_len);

    //
    // look for the KEY_MOUNT entry for this hive.  we scan primarily
    // by the registry key where the hive will be mounted.
    //

mount_loop:

    KeRaiseIrql(APC_LEVEL, &irql);
    ExAcquireResourceExclusiveLite(Key_MountsLock, TRUE);

    failed = FALSE;

    mount = List_Head(&Key_Mounts);
    while (mount) {

        //DbgPrint("Looking at mount %08X:\n", mount);
        //DbgPrint(" ... %S (%d)\n", mount->hive_path,mount->hive_path_len);
        //DbgPrint(" ... %S (%d)\n", mount->root_key,mount->root_key_len);

        if (mount->root_key_len == proc->box->key_path_len &&
            _wcsicmp(mount->root_key, proc->box->key_path) == 0) {

            //
            // we found a KEY_MOUNT representing the target key for the
            // mount.  if it's the same hive data file, then we're good.
            //

            if (mount->hive_path_len == hive_path_len &&
                _wcsicmp(mount->hive_path, hive_path) == 0) {

                break;
            }

            //
            // otherwise we have a KEY_MOUNT with a different hive file
            // file for the same target key.  it may be a stale entry
            // (for example, using an earlier box->file_path).  but if
            // it is in use, we can't mount our desired hive onto the
            // same key, so we fail
            //

#ifndef REGHIVE_ALWAYS_MOUNT_NEVER_UNMOUNT
            else if (mount->ref_count != 0) {

                Log_Status(MSG_MOUNT_FAILED, 0x11,
                           STATUS_CANNOT_LOAD_REGISTRY_FILE);
                failed = TRUE;
                break;
            }
#endif ! REGHIVE_ALWAYS_MOUNT_NEVER_UNMOUNT

        }

        mount = List_Next(mount);
    }

    //
    // if we haven't failed, but also couldn't find a mount, then
    // initialize a new mount point, with a zero reference count.
    // it will be actually mounted in the following section.
    //

    if ((! failed) && (! mount)) {

        mount = Mem_Alloc(Driver_Pool, sizeof(KEY_MOUNT));
        if (! mount)
            failed = TRUE;

        else {

            mount->hive_path = hive_path;
            mount->hive_path_len = hive_path_len;
            // ownership of hive_path now belongs to the new mount entry
            hive_path = NULL;

            mount->root_key =
                    Mem_Alloc(Driver_Pool, proc->box->key_path_len);
            if (! mount->root_key) {

                Mem_Free(mount->hive_path, mount->hive_path_len);
                Mem_Free(mount, sizeof(KEY_MOUNT));
                mount = NULL;
                failed = TRUE;

            } else {

                memcpy(mount->root_key, proc->box->key_path,
                       proc->box->key_path_len);
                mount->root_key_len = proc->box->key_path_len;

                mount->ref_count = 0;
                mount->unmount_pending = FALSE;
                mount->busy = FALSE;

                List_Insert_After(&Key_Mounts, NULL, mount);
            }
        }
    }

    //
    // if we still haven't failed by now, then we have a mount entry,
    // created now or earlier.  if it has a zero ref-count, mount the
    // hive data file, before associating the process with this entry.
    //

    if (! failed) {

        //DbgPrint("Mount %08X with ref_count = %d\n", mount, mount->ref_count);

        if (mount->busy) {

            //
            // if this particular mount is busy, we have to wait
            // for the mount/unmount operation on it to complete
            //

            ExReleaseResourceLite(Key_MountsLock);
            KeLowerIrql(irql);
            ZwYieldExecution();
            goto mount_loop;
        }

#ifdef REGHIVE_ALWAYS_MOUNT_NEVER_UNMOUNT
        mount->ref_count = 0;
        Key_NeverUnmountHives = TRUE;
#endif REGHIVE_ALWAYS_MOUNT_NEVER_UNMOUNT

        if (mount->ref_count == 0) {

            mount->busy = TRUE;
            ExReleaseResourceLite(Key_MountsLock);
            KeLowerIrql(irql);

            if (! Key_MountHive2(proc, mount))
                failed = TRUE;

            KeRaiseIrql(APC_LEVEL, &irql);
            ExAcquireResourceExclusiveLite(Key_MountsLock, TRUE);
            mount->busy = FALSE;
        }

        if (! failed) {

            ++mount->ref_count;
            mount->unmount_pending = FALSE;
            proc->key_mount = mount;
        }
    }

    //
    // free any resources we still own
    //

    ExReleaseResourceLite(Key_MountsLock);
    KeLowerIrql(irql);

    if (hive_path)
        Mem_Free(hive_path, hive_path_len);

    return (! failed);
}


//---------------------------------------------------------------------------
// Key_MountHive2
//---------------------------------------------------------------------------


_FX BOOLEAN Key_MountHive2(PROCESS *proc, KEY_MOUNT *mount)
{
    UNICODE_STRING hive_path_uni;
    UNICODE_STRING root_key_uni;
    UNICODE_STRING hivelist_uni;
    OBJECT_ATTRIBUTES source;
    OBJECT_ATTRIBUTES target;
    OBJECT_ATTRIBUTES hivelist;
    HANDLE handle;
    NTSTATUS status;

    RtlInitUnicodeString(&hive_path_uni, mount->hive_path);
    InitializeObjectAttributes(&source,
        &hive_path_uni, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL, NULL);

    RtlInitUnicodeString(&root_key_uni, mount->root_key);
    InitializeObjectAttributes(&target,
        &root_key_uni, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL, NULL);

    //
    // if the target key does not exist, we can surely mount it
    //

    status = ZwOpenKey(&handle, KEY_READ, &target);
    if (! NT_SUCCESS(status)) {

        return Key_MountHive3(proc, &target, &source);
    }

    ZwClose(handle);

    //
    // the target key exists, but if the system hivelist shows it's already
    // mapped to our source hive, then we don't have to do anything about it
    //

    RtlInitUnicodeString(&hivelist_uni,
    L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Control\\hivelist");
    InitializeObjectAttributes(&hivelist,
        &hivelist_uni, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL, NULL);

    status = ZwOpenKey(&handle, KEY_READ, &hivelist);
    if (NT_SUCCESS(status)) {

        const ULONG info_len = 2048;
        ULONG info_len_dummy;
        WCHAR *info = Mem_Alloc(proc->pool, info_len);
        if (! info)
            status = STATUS_INSUFFICIENT_RESOURCES;

        if (NT_SUCCESS(status)) {
            status = ZwQueryValueKey(
                handle, &root_key_uni, KeyValuePartialInformation,
                info, info_len, &info_len_dummy);
        }

        if (NT_SUCCESS(status)) {

            //
            // if hivelist says that the key we want to load, is already
            // in use by a hive from a different file path, then we must
            // report name collision
            //

            KEY_VALUE_PARTIAL_INFORMATION *kvpi =
                (KEY_VALUE_PARTIAL_INFORMATION *)info;
            if (kvpi->DataLength < hive_path_uni.Length ||
                _wcsnicmp((WCHAR *)kvpi->Data, hive_path_uni.Buffer,
                          hive_path_uni.Length / sizeof(WCHAR) + 1) != 0)
            {
                status = STATUS_OBJECT_NAME_COLLISION;
            }
        }

        if (info)
            Mem_Free(info, info_len);

        ZwClose(handle);
    }

    if (! NT_SUCCESS(status)) {
        Log_Status(MSG_MOUNT_FAILED, 0x33, status);
        return FALSE;
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Key_MountHive3
//---------------------------------------------------------------------------


_FX BOOLEAN Key_MountHive3(
    PROCESS *proc, OBJECT_ATTRIBUTES *target, OBJECT_ATTRIBUTES *source)
{
    NTSTATUS status, status2;
    HANDLE token;
    TOKEN_DEFAULT_DACL *old_token_dacl;
    TOKEN_DEFAULT_DACL new_token_dacl;

    BOOLEAN ok = FALSE;

    //
    // save current TokenDefaultDacl
    //

    old_token_dacl =
        Token_QueryPrimary(TokenDefaultDacl, proc->box->session_id);

    if (! old_token_dacl) {

        status = STATUS_UNSUCCESSFUL;
        Log_Status(MSG_MOUNT_FAILED, 0x42, status);

    } else {

        status = ZwOpenProcessTokenEx(
            NtCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_DEFAULT,
            OBJ_KERNEL_HANDLE, &token);

        if (! NT_SUCCESS(status)) {

            Log_Status(MSG_MOUNT_FAILED, 0x41, status);

        } else {

            //
            // replace TokenDefaultDacl with our public ACL
            //

            new_token_dacl.DefaultDacl = Driver_PublicAcl;

            status = ZwSetInformationToken(token, TokenDefaultDacl,
                                           &new_token_dacl,
                                           sizeof(TOKEN_DEFAULT_DACL));

            if (! NT_SUCCESS(status)) {

                Log_Status(MSG_MOUNT_FAILED, 0x43, status);

            } else {

                UNICODE_STRING uni;
                OBJECT_ATTRIBUTES objattrs;
                HANDLE handle;

                RtlInitUnicodeString(&uni, L"\\??\\C:");
                InitializeObjectAttributes(&objattrs,
                    &uni, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

                // ZwLoadKey can fail with device path if current process' devicemap is null
                // One workaround is to call ObOpenObjectByName and it will trigger devicemap
                // to be initialized. Note, Using C: is not necessary. The disk volume doesn't
                // need to be there.L"\\??\\A:" works in the tests.
                if (STATUS_SUCCESS == ObOpenObjectByName(
                    &objattrs, *IoFileObjectType, KernelMode, NULL, 0, NULL, &handle))
                {
                    ZwClose(handle);
                }

                //
                // load or create the new registry hive
                //

                status = ZwLoadKey(target, source);

                if (! NT_SUCCESS(status))
                    Log_Status(MSG_MOUNT_FAILED, 0x22, status);
                else
                    ok = TRUE;

                //
                // restore original TokenDefaultDacl
                //

                status2 = ZwSetInformationToken(token, TokenDefaultDacl,
                                                old_token_dacl,
                                                sizeof(TOKEN_DEFAULT_DACL));

                if (! NT_SUCCESS(status2)) {

                    Log_Status(MSG_MOUNT_FAILED, 0x44, status2);
                    ok = FALSE;
                }
            }

            ZwClose(token);
        }

        ExFreePool(old_token_dacl);
    }

    return ok;
}


//---------------------------------------------------------------------------
// Key_UnmountHive
//---------------------------------------------------------------------------


_FX void Key_UnmountHive(PROCESS *proc)
{
    KIRQL irql;
    BOOLEAN send_msg;

    if (! proc->key_mount)
        return;

    if (Key_NeverUnmountHives)
        return;

    KeRaiseIrql(APC_LEVEL, &irql);
    ExAcquireResourceExclusiveLite(Key_MountsLock, TRUE);

    --proc->key_mount->ref_count;
    if (proc->key_mount->ref_count == 0) {

        //
        // we were the last users of this hive, so it should be
        // unmounted.  however, most of the time, unloading here would
        // return STATUS_CANNOT_DELETE because the process still has
        // has open handles.  so instead, we tell SbieSvc to invoke
        // API_UNMOUNT_HIVES when the process has ended.
        //

        proc->key_mount->unmount_pending = TRUE;
        send_msg = TRUE;

    } else
        send_msg = FALSE;

    ExReleaseResourceLite(Key_MountsLock);
    KeLowerIrql(irql);

    if (send_msg) {

        SVC_UNMOUNT_MSG msg;

        msg.process_id = (ULONG)(ULONG_PTR)proc->pid;
        msg.session_id = proc->box->session_id;
        wcscpy(msg.boxname, proc->box->name);

        Api_SendServiceMessage(SVC_UNMOUNT_HIVE, sizeof(msg), &msg);
    }
}


//---------------------------------------------------------------------------
// Key_Api_GetUnmountHive
//---------------------------------------------------------------------------


_FX NTSTATUS Key_Api_GetUnmountHive(PROCESS *proc, ULONG64 *parms)
{
    API_GET_UNMOUNT_HIVE_ARGS *args = (API_GET_UNMOUNT_HIVE_ARGS *)parms;
    NTSTATUS status;
    KEY_MOUNT *mount;
    KIRQL irql;
    BOOLEAN locked;

    //
    // we expect to be called only by SbieSvc, so certainly not any
    // sandboxed process
    //

    if (proc || (PsGetCurrentProcessId() != Api_ServiceProcessId))
        return STATUS_NOT_IMPLEMENTED;

    ProbeForWrite(args->path.val, sizeof(WCHAR) * 256, sizeof(WCHAR));

    //
    // scan through the key hives and find an unused one
    //

    locked = FALSE;
    status = STATUS_OBJECT_NAME_NOT_FOUND;

    __try {

unmount_loop:

    KeRaiseIrql(APC_LEVEL, &irql);
    ExAcquireResourceExclusiveLite(Key_MountsLock, TRUE);
    locked = TRUE;

    mount = List_Head(&Key_Mounts);
    while (mount) {

        if (mount->ref_count == 0 && mount->unmount_pending) {

            if (mount->busy) {

                //
                // if this particular mount is busy, we have to wait
                // for the mount/unmount operation on it to complete
                //

                ExReleaseResourceLite(Key_MountsLock);
                KeLowerIrql(irql);
                locked = FALSE;
                ZwYieldExecution();
                goto unmount_loop;
            }

            wcscpy(args->path.val, mount->root_key);

            mount->unmount_pending = FALSE;

            ExReleaseResourceLite(Key_MountsLock);
            KeLowerIrql(irql);
            locked = FALSE;

            status = STATUS_SUCCESS;
            break;

        } else
            mount = List_Next(mount);
    }

    //
    // finish
    //

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    if (locked) {
        ExReleaseResourceLite(Key_MountsLock);
        KeLowerIrql(irql);
    }

#ifndef _WIN64

    //
    // 32-bit:  set the current process id for the HookWaitForSingleObject,
    // to prevent ZoneAlarm driver from issuing a 24-hour wait request
    // in the context of this process
    //

    if (Key_WaitForSingleObject_Pid) {
        if (NT_SUCCESS(status))
            *Key_WaitForSingleObject_Pid = (ULONG)PsGetCurrentProcessId();
        else
            *Key_WaitForSingleObject_Pid = -1;
    }

#endif _WIN64

    return status;
}


//---------------------------------------------------------------------------
// Key_Api_Open
//---------------------------------------------------------------------------


_FX NTSTATUS Key_Api_Open(PROCESS *proc, ULONG64 *parms)
{
    API_OPEN_KEY_ARGS *args = (API_OPEN_KEY_ARGS *)parms;
    ULONG path_len;
    WCHAR *path;
    HANDLE *user_handle;
    NTSTATUS status;
    HANDLE handle;
    UNICODE_STRING objname;
    OBJECT_ATTRIBUTES objattrs;

    //
    // this API must be invoked by a sandboxed process
    //

    if (! proc)
        return STATUS_NOT_IMPLEMENTED;

    //
    // check and capture parameters
    //

    user_handle = args->key_handle.val;
    if (! user_handle)
        return STATUS_INVALID_PARAMETER;

    ProbeForWrite(user_handle, sizeof(HANDLE), sizeof(HANDLE));

    path_len = args->path_len.val & ~1;
    if ((! path_len) || (path_len > 512 * sizeof(WCHAR)))
        return STATUS_INVALID_PARAMETER;

    ProbeForRead(args->path_str.val, path_len, sizeof(WCHAR));

    path = Mem_Alloc(proc->pool, path_len + 8);
    memcpy(path, args->path_str.val, path_len);
    path[path_len / sizeof(WCHAR)] = L'\0';

    //
    // if the path matches a ClosedKeyPath setting, we open the
    // key with minimal access rights
    //

    /*DesiredAccess = KEY_READ;

    Process_MatchPath(
        proc->pool, path, wcslen(path),
        NULL, &proc->closed_key_paths,
        &is_open, &is_closed);

    if (is_closed) {

        DesiredAccess  = KEY_NOTIFY;
    }*/

    //
    // issue open request and finish
    //

    RtlInitUnicodeString(&objname, path);

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = ZwOpenKey(&handle, KEY_NOTIFY, &objattrs);

    Mem_Free(path, path_len + 8);

    if (NT_SUCCESS(status))
        *user_handle = handle;

    return status;
}


//---------------------------------------------------------------------------
// Key_Api_SetLowLabel
//---------------------------------------------------------------------------


_FX NTSTATUS Key_Api_SetLowLabel(PROCESS *proc, ULONG64 *parms)
{
    API_SET_LOW_LABEL_ARGS *args = (API_SET_LOW_LABEL_ARGS *)parms;
    UNICODE_STRING objname;
    WCHAR *path;
    ULONG path_len;
    NTSTATUS status;

    //
    // this API must be invoked by a sandboxed process
    // on Windows Vista or later
    //

    if ((! proc) || (! Driver_LowLabelSd))
        return STATUS_NOT_IMPLEMENTED;

    //
    // check and capture parameters
    //

    path_len = args->path_len.val & ~1;
    if ((! path_len) || (path_len > 512 * sizeof(WCHAR)))
        return STATUS_INVALID_PARAMETER;

    ProbeForRead(args->path_str.val, path_len, sizeof(WCHAR));

    path = Mem_Alloc(proc->pool, path_len + 8);
    memcpy(path, args->path_str.val, path_len);
    path[path_len / sizeof(WCHAR)] = L'\0';

    //
    // path must be in the box
    //

    RtlInitUnicodeString(&objname, path);
    if (! Box_IsBoxedPath(proc->box, file, &objname))
        status = STATUS_SUCCESS;
    else
        status = STATUS_ACCESS_DENIED;

    //
    // issue open request and adjust integrity label
    //

    if (NT_SUCCESS(status)) {

        OBJECT_ATTRIBUTES objattrs;
        HANDLE handle;

        InitializeObjectAttributes(&objattrs,
            &objname, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

        status = ZwOpenKey(&handle, KEY_ALL_ACCESS | WRITE_DAC, &objattrs);

        if (NT_SUCCESS(status)) {

            status = ZwSetSecurityObject(handle,
                DACL_SECURITY_INFORMATION | LABEL_SECURITY_INFORMATION,
                Driver_LowLabelSd);

            ZwClose(handle);
        }
    }

    Mem_Free(path, path_len + 8);

    return status;
}
