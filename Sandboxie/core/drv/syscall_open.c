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
// Syscall Management
//---------------------------------------------------------------------------

#include "conf.h"

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static HANDLE *Syscall_ReplaceTargetHandle(
    void *UserHandleStackPtr, BOOLEAN CheckIfNullHandlePtr,
    HANDLE **TlsPtr_out, HANDLE *TlsValue_out);

static HANDLE Syscall_RestoreTargetHandle(
    void *UserHandleStackPtr, HANDLE *UserHandlePtr,
    HANDLE *TlsPtr, HANDLE TlsValue);

static NTSTATUS Syscall_CheckObject(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry,
    void *OpenedObject, OBJECT_HANDLE_INFORMATION *HandleInfo, PUNICODE_STRING puName);

static NTSTATUS Syscall_DuplicateHandle_2(
    HANDLE TargetProcessHandle, HANDLE TargetHandle,
    void *ExpectedTargetProcessObject, PROCESS *proc);

static SYSCALL_ENTRY *Syscall_DuplicateHandle_3(
    USHORT TypeLength, WCHAR *TypeBuffer);


//---------------------------------------------------------------------------
// Syscall_Set2
//---------------------------------------------------------------------------


_FX BOOLEAN Syscall_Set2(const UCHAR *name, P_Syscall_Handler2 handler_func)
{
    SYSCALL_ENTRY *entry = Syscall_GetByName(name);
    if (! entry)
        return FALSE;
    entry->handler1_func = Syscall_OpenHandle;
    entry->handler2_func = handler_func;
    return TRUE;
}


//---------------------------------------------------------------------------
// Syscall_ReplaceTargetHandle
//---------------------------------------------------------------------------


_FX HANDLE *Syscall_ReplaceTargetHandle(
    void *UserHandleStackPtr, BOOLEAN CheckIfNullHandlePtr,
    HANDLE **TlsPtr_out, HANDLE *TlsValue_out)
{
    HANDLE *TempHandlePtr;
    HANDLE *UserHandlePtr;
    ULONG_PTR Teb;
    ULONG_PTR *TlsSlots;
    LARGE_INTEGER time;
    ULONG RandomIndex;

    //
    // there are 64 ULONG_PTR slots for thread local storage,
    // we will use a random slot to store the opened handle
    //

    KeQuerySystemTime(&time);
    RandomIndex = (ULONG)
            (((ULONG_PTR)PsGetCurrentThread() * time.LowPart) % 64);

#ifdef _WIN64
    Teb = (ULONG_PTR)__readgsqword(0x30);
    TlsSlots = (ULONG_PTR *)(Teb + 0x1480);
#else ! _WIN64
    Teb = (ULONG_PTR)__readfsdword(0x18);
    TlsSlots = (ULONG_PTR *)(Teb + 0x0E10);
#endif _WIN64

    TempHandlePtr = (HANDLE *)&TlsSlots[RandomIndex];

    //
    // make sure the TLS slot can be read and written,
    // and save the original data in the TLS slot
    //

    ProbeForRead(TempHandlePtr, sizeof(HANDLE), sizeof(HANDLE));
    ProbeForWrite(TempHandlePtr, sizeof(HANDLE), sizeof(HANDLE));

    *TlsValue_out = *TempHandlePtr;
    *TlsPtr_out = TempHandlePtr;

    //
    // replace the address of the handle in the user stack so the opened
    // handle is sent to the random entry in the TLS rather than directly
    // to the address specified by the caller
    //

    ProbeForWrite(UserHandleStackPtr, sizeof(ULONG_PTR), sizeof(ULONG_PTR));

    UserHandlePtr = *(HANDLE **)UserHandleStackPtr;

    // make sure the user specified a valid address for the handle
    if ((! CheckIfNullHandlePtr) || UserHandlePtr)
        ProbeForWrite(UserHandlePtr, sizeof(HANDLE), sizeof(UCHAR));

    // replace address in the stack
    InterlockedExchangePointer(UserHandleStackPtr, TempHandlePtr);

    return UserHandlePtr;
}


//---------------------------------------------------------------------------
// Syscall_RestoreTargetHandle
//---------------------------------------------------------------------------


_FX HANDLE Syscall_RestoreTargetHandle(
    void *UserHandleStackPtr, HANDLE *UserHandlePtr,
    HANDLE *TlsPtr, HANDLE TlsValue)
{
    HANDLE NewHandle;
    HANDLE *ShouldBeTlsPtr;

    __try {

        NewHandle = InterlockedExchangePointer(TlsPtr, TlsValue);

        if (! NewHandle)        // don't return zero if the syscall failed
            NewHandle = (HANDLE)-1;

        ShouldBeTlsPtr =
            InterlockedExchangePointer(UserHandleStackPtr, UserHandlePtr);

        if (ShouldBeTlsPtr != TlsPtr) {

            // if ShouldBeTlsPtr != TlsPtr then the address of the
            // target handle in the stack was replaced while we executed
            // the syscall, this is malicious behavior and the process
            // will be terminated

            NewHandle = NULL;
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        NewHandle = NULL;
    }

    return NewHandle;
}


//---------------------------------------------------------------------------
// Syscall_CheckObject
//---------------------------------------------------------------------------


_FX NTSTATUS Syscall_CheckObject(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry,
    void *OpenedObject, OBJECT_HANDLE_INFORMATION *HandleInfo, PUNICODE_STRING puName)
{
    OBJECT_NAME_INFORMATION *Name;
    ULONG NameLength;
    NTSTATUS status;

    status = Obj_GetNameOrFileName(
                            proc->pool, OpenedObject, &Name, &NameLength);
    if (NT_SUCCESS(status)) {

        status = syscall_entry->handler2_func(
            proc, OpenedObject, &Name->Name, HandleInfo->GrantedAccess);

        if ((status != STATUS_SUCCESS)
                            && (status != STATUS_BAD_INITIAL_PC)) {

            if (puName == NULL && Name != NULL && Name->Name.Length != 0)
                puName = &Name->Name;

            WCHAR msg[256];
            RtlStringCbPrintfW(msg, sizeof(msg), L"%S (%08X) access=%08X initialized=%d", syscall_entry->name, status, HandleInfo->GrantedAccess, proc->initialized);
			Log_Msg_Process(MSG_2101, msg, puName != NULL ? puName->Buffer : L"Unnamed object", -1, proc->pid);
        }

        if (Name != &Obj_Unnamed)
            Mem_Free(Name, NameLength);
    }

    return status;
}


//---------------------------------------------------------------------------
// Syscall_OpenHandle
//---------------------------------------------------------------------------


_FX NTSTATUS Syscall_OpenHandle(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args)
{
    NTSTATUS status;
    NTSTATUS orig_status;
    HANDLE *UserHandlePtr;
    HANDLE *TlsPtr;
    HANDLE TlsValue;
    HANDLE NewHandle;
    void *OpenedObject;
    OBJECT_HANDLE_INFORMATION HandleInfo;

    // HACK ALERT! Starting in Win 10 1903, MS has started doing strange things in WOW64. See jira SBIE-33.
    // Inside wow64!Wow64HideThreadFromGuestByClientId, they are attempting to open threads from several processes in the host.
    // Then they read various chunks of memory out of these threads. The purpose is unknown at this time.
    // They are requesting THREAD_GET_CONTEXT | THREAD_SET_CONTEXT access on the NtOpenThread call, but they really do not require THREAD_SET_CONTEXT.
    // Sandboxie will block all OpenThread calls that request write access and return ACCESS_DENIED. It also terminates the offending process in the sandbox.
    // So, if we see an OpenThread call, with only THREAD_GET_CONTEXT | THREAD_SET_CONTEXT, and the target process is running outside the sandbox,
    // We will remove the THREAD_SET_CONTEXT option and request r/o.
    // The theory is that if some other process is attempting an OpenThread with write access, it will fail later on when it actually tries to write.
    // But these strange calls from WOW64 will succeed. So no hole is created.

    if ((strcmp(syscall_entry->name, "OpenThread") == 0) && (user_args[1] == (THREAD_GET_CONTEXT | THREAD_SET_CONTEXT))) {
        PCLIENT_ID  ClientId = (PCLIENT_ID)user_args[3];
        if ( (ClientId == NULL) || !Process_IsSameBox(proc, NULL, (ULONG_PTR)ClientId->UniqueProcess)) {
            user_args[1] = THREAD_GET_CONTEXT;
        }
    }

    PUNICODE_STRING puName = NULL;
    __try {

        if ((strcmp(syscall_entry->name, "ConnectPort") == 0) ||
            (strcmp(syscall_entry->name, "AlpcConnectPort") == 0))
        {
            puName = (UNICODE_STRING*)user_args[1];
        }
        else if ((strcmp(syscall_entry->name, "CreateFile") == 0) ||
            (strcmp(syscall_entry->name, "OpenFile") == 0))
        {
            POBJECT_ATTRIBUTES pObj = (POBJECT_ATTRIBUTES)user_args[2];
            if (pObj && pObj->ObjectName)
            {
                puName = pObj->ObjectName;

                ACCESS_MASK DesiredAccess = (ACCESS_MASK)user_args[1];
                if (puName->Buffer != NULL && puName->Length > (4 * sizeof(WCHAR)) && wcsncmp(puName->Buffer, L"\\??\\", 4) == 0
                    && (DesiredAccess & ~(SYNCHRONIZE | READ_CONTROL | FILE_READ_EA | FILE_READ_ATTRIBUTES)) != 0)
                {
                    if (!Conf_Get_Boolean(proc->box->name, L"AllowRawDiskRead", 0, FALSE))
                    if ((puName->Length == (6 * sizeof(WCHAR)) && puName->Buffer[5] == L':') // \??\C:
                        || wcsncmp(&puName->Buffer[4], L"PhysicalDrive", 13) == 0 // \??\PhysicalDrive1
                        || wcsncmp(&puName->Buffer[4], L"Volume", 6) == 0) // \??\Volume{2b985816-4b6f-11ea-bd33-48a4725d5bbe}
                    {
                        WCHAR access_str[24];
                        RtlStringCbPrintfW(access_str, sizeof(access_str), L"(DD) %08X", DesiredAccess);
                        Log_Debug_Msg(MONITOR_DRIVE | MONITOR_DENY, access_str, puName->Buffer);

                        if (proc->file_warn_direct_access) {

                            //Log_MsgP1(MSG_BLOCKED_DIRECT_DISK_ACCESS, proc->image_name, proc->pid);
                            Process_LogMessage(proc, MSG_BLOCKED_DIRECT_DISK_ACCESS);
                        }

                        return STATUS_ACCESS_DENIED;
                    }
                }
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}



    //
    // replace the address of the handle in the user stack
    //

    UserHandlePtr = Syscall_ReplaceTargetHandle(
                                (HANDLE *)&user_args[0], FALSE,
                                &TlsPtr, &TlsValue);

    //
    // execute the syscall to get the handle into the TLS, then extract
    // the handle value and restore the original value at the TLS slot
    //
    // note that the syscall can return a non-zero success status code
    // like STATUS_OBJECT_NAME_EXISTS so we have to save the exact status
    //
    // if the syscall did not complete due to an APC, then we abort early
    //

    status = Syscall_Invoke(syscall_entry, user_args);

    orig_status = status;

    NewHandle = Syscall_RestoreTargetHandle(
                                &user_args[0], UserHandlePtr,
                                TlsPtr, TlsValue);

    if (! NewHandle) {

        //WCHAR trace_str[128];
        //RtlStringCbPrintfW(trace_str, sizeof(trace_str), L"Syscall %.*S security violation terminating process", max(strlen(syscall_entry->name), 64), syscall_entry->name);
        //Session_MonitorPut(MONITOR_OTHER, trace_str, PsGetCurrentProcessId());

        Process_SetTerminated(proc, 6);
        status = STATUS_PROCESS_IS_TERMINATING;
    }

    //
    // return on error if an APC interrupted the syscall
    //

    if ((! NT_SUCCESS(status)) || (status == STATUS_USER_APC))
        return status;

    //
    // get object pointer and information for the opened handle
    //

    status = ObReferenceObjectByHandle(
                NewHandle, 0, NULL, UserMode, &OpenedObject, &HandleInfo);

    if (NT_SUCCESS(status)) {        

        //
        // check the access that was granted to the object
        //

        status = Syscall_CheckObject(
                    proc, syscall_entry, OpenedObject, &HandleInfo, puName);

        ObDereferenceObject(OpenedObject);

        if (!NT_SUCCESS(status))
            NtClose(NewHandle);

        if (status == STATUS_BAD_INITIAL_PC) {

            //
            // special status for immediate return;
            // see also File_CheckFileObject in file_flt.c
            //

            return status;
        }
    }

    //
    // if all went well, copy the opened handle into the first parameter
    //

    if (NT_SUCCESS(status)) {

        __try {

            if (UserHandlePtr)
                *UserHandlePtr = NewHandle;
            status = orig_status;

        } __except (EXCEPTION_EXECUTE_HANDLER) {

            status = STATUS_PROCESS_IS_TERMINATING;
        }
    }

    if (!NT_SUCCESS(status)) {

        //WCHAR trace_str[128];
        //RtlStringCbPrintfW(trace_str, sizeof(trace_str), L"Syscall %.*S security violation, status = 0x%X, terminating process", max(strlen(syscall_entry->name), 64), syscall_entry->name, status);
        //Session_MonitorPut(MONITOR_OTHER, trace_str, PsGetCurrentProcessId());

        Process_SetTerminated(proc, 7);
    }

    return status;
}

//---------------------------------------------------------------------------
// Syscall_GetNextProcess
//---------------------------------------------------------------------------
// See ICD-11903.  NtGetNextProcess can be used inside the sandbox to get writable handles to processes outside the sandbox.
// 

_FX NTSTATUS Syscall_GetNextProcess(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args)
{
    return STATUS_ACCESS_DENIED;
}

//---------------------------------------------------------------------------
// Syscall_DeviceIoControlFile
//---------------------------------------------------------------------------


#include "file_ctrl.c"


//---------------------------------------------------------------------------
// Syscall_DuplicateHandle
//---------------------------------------------------------------------------


_FX NTSTATUS Syscall_DuplicateHandle(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args)
{
    NTSTATUS status;
    NTSTATUS orig_status;
    HANDLE *UserHandlePtr;
    HANDLE *TlsPtr;
    HANDLE TlsValue;
    HANDLE NewHandle;
    void *TargetProcessObject;

    //
    // if there is a target process handle, keep a record of the
    // associated process object so we can check it later
    //

    if (user_args[2]) {     // TargetProcessHandle parameter

        status = ObReferenceObjectByHandle(
                    (HANDLE)user_args[2], 0, *PsProcessType, UserMode,
                    &TargetProcessObject, NULL);

        if (! NT_SUCCESS(status))
            return status;

        ObDereferenceObject(TargetProcessObject);

    } else
        TargetProcessObject = NULL;

    //
    // replace the address of the handle in the user stack
    //

    UserHandlePtr = Syscall_ReplaceTargetHandle(
                                (HANDLE *)&user_args[3], TRUE,
                                &TlsPtr, &TlsValue);

    //
    // execute the syscall to get the handle into the TLS, then extract
    // the handle value and restore the original value at the TLS slot
    //
    // save the original return code from the syscall, in case there is
    // a non-zero success status code (see also Syscall_OpenHandle)
    //
    // if the syscall did not complete due to an APC, then we abort early
    //

    status = Syscall_Invoke(syscall_entry, user_args);

    orig_status = status;

    NewHandle = Syscall_RestoreTargetHandle(
                                &user_args[3], UserHandlePtr,
                                TlsPtr, TlsValue);

    if (! NewHandle) {

        //WCHAR trace_str[128];
        //RtlStringCbPrintfW(trace_str, sizeof(trace_str), L"Syscall %.*S security violation terminating process", max(strlen(syscall_entry->name), 64), syscall_entry->name);
        //Session_MonitorPut(MONITOR_OTHER, trace_str, PsGetCurrentProcessId());

        Process_SetTerminated(proc, 8);
        status = STATUS_PROCESS_IS_TERMINATING;
    }

    //
    // return on error if an APC interrupted the syscall
    //

    if ((! NT_SUCCESS(status)) || (status == STATUS_USER_APC))
        return status;

    //
    // if a target process handle was not specified on entry to
    // NtDuplicateObject, before we executed the syscall, then
    // (1) there should not be a target process handle now
    // (2) no handle was duplicated so we can just return
    //

    if (! TargetProcessObject) {

        if (user_args[2])
            status = STATUS_ACCESS_DENIED;

    } else {

        //
        // a handle was duplicated, so check the access that
        // was granted to the object
        //

        status = Syscall_DuplicateHandle_2(
            (HANDLE)user_args[2], NewHandle, TargetProcessObject, proc);

		if (!NT_SUCCESS(status))
			NtClose(NewHandle);
    }

    //
    // if all went well, copy the opened handle into the first parameter
    //

    if (NT_SUCCESS(status)) {

        __try {

            if (UserHandlePtr)
                *UserHandlePtr = NewHandle;
            status = orig_status;

        } __except (EXCEPTION_EXECUTE_HANDLER) {

            status = STATUS_PROCESS_IS_TERMINATING;
        }
    }

    //  if (! NT_SUCCESS(status)) {
    //      if(!wcsicmp(proc->image_name,L"SandboxieBITS.exe") && status == STATUS_ACCESS_DENIED) { 
    //          return status;
    //  }
    //
    //    //WCHAR trace_str[128];
    //    //RtlStringCbPrintfW(trace_str, sizeof(trace_str), L"Syscall %.*S security violation terminating process", max(strlen(syscall_entry->name), 64), syscall_entry->name);
    //    //Session_MonitorPut(MONITOR_OTHER, trace_str, PsGetCurrentProcessId());
    //
    //  Process_SetTerminated(proc, 9);
    //}

    return status;
}


//---------------------------------------------------------------------------
// Syscall_DuplicateHandle_2
//---------------------------------------------------------------------------


_FX NTSTATUS Syscall_DuplicateHandle_2(
    HANDLE TargetProcessHandle, HANDLE TargetHandle,
    void *ExpectedTargetProcessObject, PROCESS *proc)
{
    NTSTATUS status;
    void *CurrentTargetProcessObject;
    void *OpenedObject;
    KAPC_STATE ApcState;
    OBJECT_HANDLE_INFORMATION HandleInfo;
    USHORT TypeLength;
    WCHAR *TypeBuffer;
    SYSCALL_ENTRY *syscall_entry;
    BOOLEAN ShouldDetach;

    //
    // check that the target process handle in the stack was not modified
    // during the call and still points to the process object that we
    // recorded on entry to Syscall_DuplicateHandle
    //

    status = ObReferenceObjectByHandle(
                TargetProcessHandle, 0, *PsProcessType, UserMode,
                &CurrentTargetProcessObject, NULL);

    if (! NT_SUCCESS(status))
        return status;

    if (CurrentTargetProcessObject != ExpectedTargetProcessObject) {

        ObDereferenceObject(CurrentTargetProcessObject);
        return STATUS_ACCESS_DENIED;
    }

    //
    // access the object handle in the context of the target process
    //

    if (CurrentTargetProcessObject == PsGetCurrentProcess())
        ShouldDetach = FALSE;
    else {
        KeStackAttachProcess(CurrentTargetProcessObject, &ApcState);
        ShouldDetach = TRUE;
    }

    status = ObReferenceObjectByHandle(
                TargetHandle, 0, NULL, UserMode, &OpenedObject, &HandleInfo);

    if (ShouldDetach)
        KeUnstackDetachProcess(&ApcState);

    ObDereferenceObject(CurrentTargetProcessObject);

    if (! NT_SUCCESS(status))
        return status;

    //
    // get object type name, see also Ipc_CheckObjectName
    //

    TypeLength = 0;
    TypeBuffer = NULL;

    if (Driver_OsVersion >= DRIVER_WINDOWS_7) {

        OBJECT_TYPE_VISTA_SP1 *ObjectType = pObGetObjectType(OpenedObject);
        TypeLength = ObjectType->Name.Length;
        TypeBuffer = ObjectType->Name.Buffer;

    } else {

        OBJECT_HEADER *ObjectHeader = OBJECT_TO_OBJECT_HEADER(OpenedObject);

        if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA &&
            Driver_OsBuild > 6000) {

            OBJECT_TYPE_VISTA_SP1 *ObjectType =
                (OBJECT_TYPE_VISTA_SP1 *)ObjectHeader->Type;

            TypeLength = ObjectType->Name.Length;
            TypeBuffer = ObjectType->Name.Buffer;

        } else {

            OBJECT_TYPE *ObjectType = ObjectHeader->Type;
            TypeLength = ObjectType->Name.Length;
            TypeBuffer = ObjectType->Name.Buffer;
        }
    }

    //DbgPrint("Object %08X TypeBuffer %*.*S (%d)\n", OpenedObject, TypeLength/sizeof(WCHAR), TypeLength/sizeof(WCHAR), TypeBuffer, TypeLength);

    //
    // search the syscall tables for an entry matching the object type,
    // so we can call the corresponding P_Syscall_Handler2 function,
    // in order to make sure that the new handle that was duplicated
    // did not get any undesired permissions as part of duplication.
    //
    // for example, for a process object, we would find NtOpenProcess
    // and then call Thread_CheckProcessObject (see Thread_Init)
    //
    // note that files and registry keys don't have a P_Syscall_Handler2
    // function, but these objects don't support adding new permissions
    // on the handle during duplication.  (this is true for any
    // object type which specifies a SecurityProcedure.)
    //

    syscall_entry = Syscall_DuplicateHandle_3(TypeLength, TypeBuffer);

    if (syscall_entry) {

        //
        // check the access that was granted to the object
        //

        status = Syscall_CheckObject(
                    proc, syscall_entry, OpenedObject, &HandleInfo, NULL);

    } else if (    TypeLength == 5 * sizeof(WCHAR)
                && wmemcmp(TypeBuffer, L"Token", 5) == 0) {

        //
        // we have special handling for token objects which don't have
        // an NtOpenToken API and use P_Syscall_Handler1 functions for
        // opening tokens (see Thread_Init and thread_token.c).
        // thread_token.c has a function for this specific case.
        //

        status = Thread_CheckTokenObject(
                    proc, OpenedObject, HandleInfo.GrantedAccess);
    }

    ObDereferenceObject(OpenedObject);

    return status;
}


//---------------------------------------------------------------------------
// Syscall_DuplicateHandle_3
//---------------------------------------------------------------------------


_FX SYSCALL_ENTRY *Syscall_DuplicateHandle_3(
    USHORT TypeLength, WCHAR *TypeBuffer)
{
    static const WCHAR *_Port = L"Port";
    SYSCALL_ENTRY *entry;
    ULONG name_len;
    UCHAR SyscallName[32];

    //
    // if this is a Port (or the Vista "ALPC Port") object, there isn't
    // an OpenPort API, so we search for the ConnectPort API instead
    //
    // otherwise we search for the Open(TokenType) API, e.g. OpenEvent
    //

    memzero(SyscallName, sizeof(SyscallName));

    TypeLength /= sizeof(WCHAR);

    if ((TypeLength == 4 && wmemcmp(TypeBuffer,     _Port, 4) == 0) ||
        (TypeLength == 9 && wmemcmp(TypeBuffer + 5, _Port, 4) == 0)) {

        strcpy(SyscallName, "ConnectPort");

    } else if (TypeLength <= 24) {

        memcpy(SyscallName, "Open", 4);
        do {
            --TypeLength;
            SyscallName[TypeLength + 4] = (UCHAR)TypeBuffer[TypeLength];
        } while (TypeLength);

    } else
        return NULL;

    //
    // search the list of syscalls for the entry we want
    //

    name_len = strlen(SyscallName);

    entry = List_Head(&Syscall_List);
    while (entry) {

        if (entry->handler2_func && entry->name_len == name_len
                    && memcmp(entry->name, SyscallName, name_len) == 0) {

            return entry;
        }

        entry = List_Next(entry);
    }

    return NULL;
}
