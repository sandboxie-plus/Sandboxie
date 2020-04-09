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
// Session Management
//---------------------------------------------------------------------------


#include "session.h"
#include "util.h"
#include "conf.h"
#include "api.h"
#include "process.h"
#include "obj.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define SESSION_MONITOR_BUF_SIZE    (PAGE_SIZE * 32)


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


struct _SESSION {

    // changes to the linked list of SESSION blocks are synchronized by
    // an exclusive lock on Session_ListLock

    LIST_ELEM list_elem;

    //
    // session id
    //

    ULONG session_id;

    //
    // session leader process id
    //

    HANDLE leader_pid;

    //
    // disable forced process
    //

    LONGLONG disable_force_time;

    //
    // resource monitor
    //

    WCHAR *monitor_buf;
    WCHAR *monitor_read_ptr;
    WCHAR *monitor_write_ptr;

};


typedef struct _SESSION             SESSION;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN Session_AddObjectType(const WCHAR *TypeName);

static void Session_Unlock(KIRQL irql);

static SESSION *Session_Get(
    BOOLEAN create, ULONG SessionId, KIRQL *out_irql);

static BOOLEAN Session_CheckAdminAccess(const WCHAR *setting);


//---------------------------------------------------------------------------


static NTSTATUS Session_Api_Leader(PROCESS *proc, ULONG64 *parms);

static NTSTATUS Session_Api_DisableForce(PROCESS *proc, ULONG64 *parms);

static NTSTATUS Session_Api_MonitorControl(PROCESS *proc, ULONG64 *parms);

static NTSTATUS Session_Api_MonitorPut(PROCESS *proc, ULONG64 *parms);

static NTSTATUS Session_Api_MonitorPut2(PROCESS *proc, ULONG64 *parms);

static NTSTATUS Session_Api_MonitorGet(PROCESS *proc, ULONG64 *parms);


//---------------------------------------------------------------------------


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, Session_AddObjectType)
#endif // ALLOC_PRAGMA


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static LIST Session_List;
PERESOURCE Session_ListLock = NULL;

volatile LONG Session_MonitorCount = 0;

static POBJECT_TYPE *Session_ObjectTypes = NULL;


//---------------------------------------------------------------------------
// Session_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Session_Init(void)
{
    List_Init(&Session_List);

    if (! Mem_GetLockResource(&Session_ListLock, TRUE))
        return FALSE;

    Api_SetFunction(API_SESSION_LEADER,         Session_Api_Leader);
    Api_SetFunction(API_DISABLE_FORCE_PROCESS,  Session_Api_DisableForce);
    Api_SetFunction(API_MONITOR_CONTROL,        Session_Api_MonitorControl);
    Api_SetFunction(API_MONITOR_PUT,            Session_Api_MonitorPut);
    Api_SetFunction(API_MONITOR_PUT2,           Session_Api_MonitorPut2);
    Api_SetFunction(API_MONITOR_GET,            Session_Api_MonitorGet);

    //
    // initialize set of recognized objects types for Session_Api_MonitorPut
    //

    Session_ObjectTypes = Mem_AllocEx(
                            Driver_Pool, sizeof(POBJECT_TYPE) * 9, TRUE);
    if (! Session_ObjectTypes)
        return FALSE;
    memzero(Session_ObjectTypes, sizeof(POBJECT_TYPE) * 9);

    if (! Session_AddObjectType(L"Job"))
        return FALSE;
    if (! Session_AddObjectType(L"Event"))
        return FALSE;
    if (! Session_AddObjectType(L"Mutant"))
        return FALSE;
    if (! Session_AddObjectType(L"Semaphore"))
        return FALSE;
    if (! Session_AddObjectType(L"Section"))
        return FALSE;
    if (Driver_OsVersion < DRIVER_WINDOWS_VISTA) {
        if (! Session_AddObjectType(L"Port"))
            return FALSE;
    } else {
        if (! Session_AddObjectType(L"ALPC Port"))
            return FALSE;
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Session_Unload
//---------------------------------------------------------------------------


_FX void Session_Unload(void)
{
    if (Session_ListLock) {

        Session_Cancel(NULL);
        Mem_FreeLockResource(&Session_ListLock);
    }
}


//---------------------------------------------------------------------------
// Session_AddObjectType
//---------------------------------------------------------------------------


_FX BOOLEAN Session_AddObjectType(const WCHAR *TypeName)
{
    NTSTATUS status;
    WCHAR ObjectName[64];
    UNICODE_STRING uni;
    OBJECT_ATTRIBUTES objattrs;
    HANDLE handle;
    OBJECT_TYPE *object;
    ULONG i;

    wcscpy(ObjectName, L"\\ObjectTypes\\");
    wcscat(ObjectName, TypeName);
    RtlInitUnicodeString(&uni, ObjectName);
    InitializeObjectAttributes(&objattrs,
        &uni, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

    //
    // Windows 7 requires that we pass ObjectType in the second parameter
    // below, while earlier versions of Windows do not require this.
    // Obj_GetTypeObjectType() returns ObjectType on Windows 7, and
    // NULL on earlier versions of Windows
    //

    status = ObOpenObjectByName(
                    &objattrs, Obj_GetTypeObjectType(), KernelMode,
                    NULL, 0, NULL, &handle);
    if (! NT_SUCCESS(status)) {
        Log_Status_Ex(MSG_OBJ_HOOK_ANY_PROC, 0x44, status, TypeName);
        return FALSE;
    }

    status = ObReferenceObjectByHandle(
                    handle, 0, NULL, KernelMode, &object, NULL);

    ZwClose(handle);

    if (! NT_SUCCESS(status)) {
        Log_Status_Ex(MSG_OBJ_HOOK_ANY_PROC, 0x55, status, TypeName);
        return FALSE;
    }

    ObDereferenceObject(object);

    for (i = 0; Session_ObjectTypes[i]; ++i)
        ;
    Session_ObjectTypes[i] = object;

    return TRUE;
}


//---------------------------------------------------------------------------
// Session_Unlock
//---------------------------------------------------------------------------


_FX void Session_Unlock(KIRQL irql)
{
    ExReleaseResourceLite(Session_ListLock);
    KeLowerIrql(irql);
}


//---------------------------------------------------------------------------
// Session_Get
//---------------------------------------------------------------------------


_FX SESSION *Session_Get(BOOLEAN create, ULONG SessionId, KIRQL *out_irql)
{
    NTSTATUS status;
    SESSION *session;

    if (SessionId == -1) {
        status = MyGetSessionId(&SessionId);
        if (! NT_SUCCESS(status))
            return NULL;
    }

    //
    // find an existing SESSION block or create a new one
    //

    KeRaiseIrql(APC_LEVEL, out_irql);
    ExAcquireResourceExclusiveLite(Session_ListLock, TRUE);

    session = List_Head(&Session_List);
    while (session) {
        if (session->session_id == SessionId)
            break;
        session = List_Next(session);
    }

    if ((! session) && create) {

        session = Mem_Alloc(Driver_Pool, sizeof(SESSION));
        if (session) {

            memzero(session, sizeof(SESSION));
            session->session_id = SessionId;

            List_Insert_After(&Session_List, NULL, session);
        }
    }

    if (! session)
        Session_Unlock(*out_irql);
    else if (create)
        session->leader_pid = PsGetCurrentProcessId();

    return session;
}


//---------------------------------------------------------------------------
// Session_Cancel
//---------------------------------------------------------------------------


_FX void Session_Cancel(HANDLE ProcessId)
{
    KIRQL irql;
    SESSION *session;

    //
    // find an existing SESSION block with leader_pid == ProcessId
    //

    KeRaiseIrql(APC_LEVEL, &irql);
    ExAcquireResourceExclusiveLite(Session_ListLock, TRUE);

    session = List_Head(&Session_List);
    while (session) {
        if ((session->leader_pid == ProcessId) || (! ProcessId)) {

            if (session->monitor_buf) {
                ExFreePoolWithTag(session->monitor_buf, tzuk);
                InterlockedDecrement(&Session_MonitorCount);
            }

            List_Remove(&Session_List, session);
            Mem_Free(session, sizeof(SESSION));

            break;
        }

        session = List_Next(session);
    }

    Session_Unlock(irql);
}


//---------------------------------------------------------------------------
// Session_CheckAdminAccess
//---------------------------------------------------------------------------


_FX BOOLEAN Session_CheckAdminAccess(const WCHAR *setting)
{
    if (Conf_Get_Boolean(NULL, setting, 0, FALSE)) {

        //
        // check if token is member of the Administrators group
        //

        PACCESS_TOKEN pAccessToken =
            PsReferencePrimaryToken(PsGetCurrentProcess());
        BOOLEAN IsAdmin = SeTokenIsAdmin(pAccessToken);
        if ((! IsAdmin) && Driver_OsVersion >= DRIVER_WINDOWS_VISTA) {

            //
            // on Windows Vista, check for UAC split token
            //

            ULONG *pElevationType;
            NTSTATUS status = SeQueryInformationToken(
                pAccessToken, TokenElevationType, &pElevationType);
            if (NT_SUCCESS(status)) {
                if (*pElevationType == TokenElevationTypeFull ||
                    *pElevationType == TokenElevationTypeLimited)
                    IsAdmin = TRUE;
                ExFreePool(pElevationType);
            }
        }

        PsDereferencePrimaryToken(pAccessToken);
        if (! IsAdmin)
            return FALSE;
    }
    return TRUE;
}


//---------------------------------------------------------------------------
// Session_Api_Leader
//---------------------------------------------------------------------------


_FX NTSTATUS Session_Api_Leader(PROCESS *proc, ULONG64 *parms)
{
    API_SESSION_LEADER_ARGS *args = (API_SESSION_LEADER_ARGS *)parms;
    NTSTATUS status = STATUS_SUCCESS;
    ULONG64 ProcessIdToReturn = 0;
    SESSION *session = NULL;
    KIRQL irql;

    ULONG64 *user_pid = args->process_id.val;
    if (! user_pid) {

        //
        // set leader
        //

        if (proc)
            status = STATUS_NOT_IMPLEMENTED;
        else {

            session = Session_Get(TRUE, -1, &irql);
            if (! session)
                status = STATUS_INSUFFICIENT_RESOURCES;
        }

    } else {

        //
        // get leader
        //

        HANDLE TokenHandle = args->token_handle.val;

        ULONG SessionId;
        ULONG len = sizeof(ULONG);

        status = ZwQueryInformationToken(
                        TokenHandle, TokenSessionId, &SessionId, len, &len);

        if (NT_SUCCESS(status)) {

            __try {

                session = Session_Get(FALSE, SessionId, &irql);
                if (session)
                    ProcessIdToReturn = (ULONG64)session->leader_pid;

            } __except (EXCEPTION_EXECUTE_HANDLER) {
                status = GetExceptionCode();
            }
        }
    }

    if (session)
        Session_Unlock(irql);

    if (user_pid && NT_SUCCESS(status)) {
        ProbeForWrite(user_pid, sizeof(ULONG64), sizeof(ULONG64));
        *user_pid = ProcessIdToReturn;
    }

    return status;
}


//---------------------------------------------------------------------------
// Session_Api_DisableForce
//---------------------------------------------------------------------------


_FX NTSTATUS Session_Api_DisableForce(PROCESS *proc, ULONG64 *parms)
{
    API_DISABLE_FORCE_PROCESS_ARGS *args =
        (API_DISABLE_FORCE_PROCESS_ARGS *)parms;
    ULONG *in_flag;
    ULONG *out_flag;
    LARGE_INTEGER time;
    SESSION *session;
    KIRQL irql;

    if (proc)
        return STATUS_NOT_IMPLEMENTED;

    //
    // get status
    //

    out_flag = args->get_flag.val;
    if (out_flag) {
        ProbeForWrite(out_flag, sizeof(ULONG), sizeof(ULONG));
        *out_flag = Session_IsForceDisabled(-1);
    }

    //
    // set status
    //

    in_flag = args->set_flag.val;
    if (in_flag) {
        ProbeForRead(in_flag, sizeof(ULONG), sizeof(ULONG));
        if (*in_flag) {

            if (! Session_CheckAdminAccess(L"ForceDisableAdminOnly"))
                    return STATUS_ACCESS_DENIED;
            KeQuerySystemTime(&time);

        } else
            time.QuadPart = 0;

        if (*in_flag == DISABLE_JUST_THIS_PROCESS) {

            Process_DfpInsert(PROCESS_TERMINATED, PsGetCurrentProcessId());

        } else {

            session = Session_Get(FALSE, -1, &irql);
            if (session) {
                session->disable_force_time = time.QuadPart;
                Session_Unlock(irql);
            }
        }
    }

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Session_IsForceDisabled
//---------------------------------------------------------------------------


_FX BOOLEAN Session_IsForceDisabled(ULONG SessionId)
{
    int seconds;
    LARGE_INTEGER time;
    LONGLONG diff;
    SESSION *session;
    KIRQL irql;

    // compute the number of seconds that force remains disabled

    seconds = Conf_Get_Number(NULL, L"ForceDisableSeconds", 0, 10);

    if (seconds == 0) {
        // zero means never allow to disable force process
        return FALSE;
    }

    // get the number of seconds passed since force was disabled

    KeQuerySystemTime(&time);

    session = Session_Get(FALSE, SessionId, &irql);
    if (! session)
        return FALSE;

    diff = (time.QuadPart - session->disable_force_time) / SECONDS(1);

    Session_Unlock(irql);

    // compare and return

    return (diff <= seconds);
}


//---------------------------------------------------------------------------
// Session_MonitorPut
//---------------------------------------------------------------------------


_FX void Session_MonitorPut(USHORT type, const WCHAR *name)
{
    SESSION *session;
    KIRQL irql;

    session = Session_Get(FALSE, -1, &irql);
    if (! session)
        return;

    if (session->monitor_buf && *name) {

        WCHAR *buf0 = session->monitor_buf;
        WCHAR *wptr = session->monitor_write_ptr;
        WCHAR *rptr = session->monitor_read_ptr;

        ULONG name_len = wcslen(name) + 1;

        BOOLEAN first = TRUE;

        while (name_len) {

            if (first) {
                first = FALSE;
                *wptr = type;
            } else {
                *wptr = *name;
                ++name;
                --name_len;
            }

            ++wptr;
            if (wptr >= buf0 + SESSION_MONITOR_BUF_SIZE)
                wptr = buf0;
            if (wptr == rptr) {
                Log_Msg0(MSG_MONITOR_OVERFLOW);
                *buf0 = L'\0';
                wptr = buf0;
                session->monitor_read_ptr = buf0;
                break;
            }
        }

        *wptr = L'\0';
        session->monitor_write_ptr = wptr;
    }

    Session_Unlock(irql);
}


//---------------------------------------------------------------------------
// Session_Api_MonitorControl
//---------------------------------------------------------------------------


_FX NTSTATUS Session_Api_MonitorControl(PROCESS *proc, ULONG64 *parms)
{
    API_MONITOR_CONTROL_ARGS *args = (API_MONITOR_CONTROL_ARGS *)parms;
    ULONG *in_flag;
    ULONG *out_flag;
    SESSION *session;
    KIRQL irql;
    BOOLEAN EnableMonitor;

    if (proc)
        return STATUS_NOT_IMPLEMENTED;

    //
    // get status
    //

    out_flag = args->get_flag.val;
    if (out_flag) {
        ProbeForWrite(out_flag, sizeof(ULONG), sizeof(ULONG));
        *out_flag = FALSE;
        session = Session_Get(FALSE, -1, &irql);
        if (session) {
            if (session->monitor_buf)
                *out_flag = TRUE;
            Session_Unlock(irql);
        }
    }

    //
    // set status
    //

    in_flag = args->set_flag.val;
    if (in_flag) {
        ProbeForRead(in_flag, sizeof(ULONG), sizeof(ULONG));
        if (*in_flag) {

            if (! Session_CheckAdminAccess(L"MonitorAdminOnly"))
                    return STATUS_ACCESS_DENIED;

            EnableMonitor = TRUE;

        } else
            EnableMonitor = FALSE;

        session = Session_Get(FALSE, -1, &irql);
        if (session) {

            if (EnableMonitor && (! session->monitor_buf)) {

                session->monitor_buf = ExAllocatePoolWithTag(PagedPool,
                    SESSION_MONITOR_BUF_SIZE * sizeof(WCHAR), tzuk);
                session->monitor_write_ptr = session->monitor_buf;
                session->monitor_read_ptr  = session->monitor_buf;
                if (session->monitor_buf) {
                    *session->monitor_buf = L'\0';
                    InterlockedIncrement(&Session_MonitorCount);
                } else
                    Log_Msg0(MSG_1201);

            } else if ((! EnableMonitor) && session->monitor_buf) {

                ExFreePoolWithTag(session->monitor_buf, tzuk);
                session->monitor_buf = NULL;
                session->monitor_write_ptr = session->monitor_buf;
                session->monitor_read_ptr  = session->monitor_buf;
                InterlockedDecrement(&Session_MonitorCount);
            }

            Session_Unlock(irql);
        }
    }

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Session_Api_MonitorPut
//---------------------------------------------------------------------------


_FX NTSTATUS Session_Api_MonitorPut(PROCESS *proc, ULONG64 *parms)
{
    API_MONITOR_GET_PUT_ARGS *args = (API_MONITOR_GET_PUT_ARGS *)parms;
    API_MONITOR_PUT2_ARGS args2 = { args->func_code, args->name_type.val64, args->name_len.val64, args->name_ptr.val64, TRUE };

    return Session_Api_MonitorPut2(proc, (ULONG64*)&args2);
}

//---------------------------------------------------------------------------
// Session_Api_MonitorPut
//---------------------------------------------------------------------------


_FX NTSTATUS Session_Api_MonitorPut2(PROCESS *proc, ULONG64 *parms)
{
    API_MONITOR_PUT2_ARGS *args = (API_MONITOR_PUT2_ARGS *)parms;
    UNICODE_STRING objname;
    void *object;
    USHORT *user_type;
    WCHAR *user_name;
    WCHAR *name;
    NTSTATUS status;
    ULONG name_len;
    USHORT type;

    if (! proc)
        return STATUS_NOT_IMPLEMENTED;

    if (! Session_MonitorCount)
        return STATUS_SUCCESS;

    user_type = args->name_type.val;
    ProbeForRead(user_type, sizeof(USHORT), sizeof(USHORT));
    type = *user_type;
    if (! type)
        return STATUS_INVALID_PARAMETER;

    name_len = args->name_len.val / sizeof(WCHAR);
    if ((! name_len) || name_len > 256)
        return STATUS_INVALID_PARAMETER;
    user_name = args->name_ptr.val;
    ProbeForRead(user_name, name_len * sizeof(WCHAR), sizeof(WCHAR));

    name = Mem_Alloc(proc->pool, 260 * sizeof(WCHAR));
    if (! name)
        return STATUS_INSUFFICIENT_RESOURCES;

    //
    // we do everything else within a try/except block to make sure
    // that we always free the name buffer regardless of errors
    //

    __try {

        wmemcpy(name, user_name, name_len);
        name[name_len] = L'\0';

        status = STATUS_SUCCESS;
        object = NULL;

        //
        // if type is MONITOR_IPC we try to open the object
        // to get the name assigned to it at time of creation
        //

        if ((type & 0xFFF) == MONITOR_IPC) {

            ULONG i;

            RtlInitUnicodeString(&objname, name);

            for (i = 0; Session_ObjectTypes[i]; ++i) {

                // ObReferenceObjectByName needs a non-zero ObjectType
                // so we have to keep going through all possible object
                // types as long as we get STATUS_OBJECT_TYPE_MISMATCH

                status = ObReferenceObjectByName(
                            &objname, OBJ_CASE_INSENSITIVE, NULL, 0,
                            Session_ObjectTypes[i], KernelMode, NULL,
                            &object);

                if (status != STATUS_OBJECT_TYPE_MISMATCH)
                    break;
            }

            // DbgPrint("IPC  Status = %08X Object = %08X for Open <%S>\n", status, object, name);
        }

        //
        // if type is MONITOR_PIPE we try to open the pipe
        // to get the name assigned to it at time of creation
        //

        if ((type & 0xFFF) == MONITOR_PIPE) {

            if (args->check_object_exists.val64)
            {
                OBJECT_ATTRIBUTES objattrs;
                IO_STATUS_BLOCK IoStatusBlock;
                HANDLE handle;

                InitializeObjectAttributes(&objattrs,
                    &objname, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
                    NULL, NULL);

                RtlInitUnicodeString(&objname, name);

                status = IoCreateFileSpecifyDeviceObjectHint(
                    &handle, 0, &objattrs, &IoStatusBlock,
                    NULL, 0, FILE_SHARE_VALID_FLAGS, FILE_OPEN, 0,
                    NULL, 0, CreateFileTypeNone, NULL,
                    IO_IGNORE_SHARE_ACCESS_CHECK, NULL);

                if (NT_SUCCESS(status)) {

                    status = ObReferenceObjectByHandle(
                        handle, 0, NULL, KernelMode, &object, NULL);

                    ZwClose(handle);

                }
                else if (status == STATUS_UNSUCCESSFUL
                    || status == STATUS_PIPE_NOT_AVAILABLE
                    || status == STATUS_NOT_SUPPORTED) {

                    //
                    // might be a strange device like \Device\NDMP4 which can't
                    // be opened, change the status to prevent logging of an
                    // error entry (i.e. question mark, see below)
                    //

                    status = STATUS_OBJECT_NAME_NOT_FOUND;
                }

                //DbgPrint("PIPE Status3 = %08X Object = %08X for Open <%S>\n", status, object, name);
            }
        }

        //
        // if we have an object, get its name from the kernel object
        //

        if (NT_SUCCESS(status) && object) {

            OBJECT_NAME_INFORMATION *Name;
            ULONG NameLength;

            status = Obj_GetNameOrFileName(
                                    proc->pool, object, &Name, &NameLength);

            if (NT_SUCCESS(status)) {

                name_len = Name->Name.Length / sizeof(WCHAR);
                if (name_len > 256)
                    name_len = 256;
                wmemcpy(name, Name->Name.Buffer, name_len);
                name[name_len] = L'\0';

                if (Name != &Obj_Unnamed)
                    Mem_Free(Name, NameLength);

                // DbgPrint("Determined Object Name <%S>\n", name);
            }

            ObDereferenceObject(object);
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    //
    // append the object name into to the monitor log
    //

    if (    status != STATUS_OBJECT_NAME_NOT_FOUND
         && status != STATUS_OBJECT_PATH_NOT_FOUND
         && status != STATUS_OBJECT_PATH_SYNTAX_BAD) {

        if (! NT_SUCCESS(status)) {
            name[0] = L'?';
            name[1] = L'\0';
        }

        Session_MonitorPut(type, name);
    }

    Mem_Free(name, 260 * sizeof(WCHAR));

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Session_Api_MonitorGet
//---------------------------------------------------------------------------


_FX NTSTATUS Session_Api_MonitorGet(PROCESS *proc, ULONG64 *parms)
{
    API_MONITOR_GET_PUT_ARGS *args = (API_MONITOR_GET_PUT_ARGS *)parms;
    NTSTATUS status;
    USHORT *user_type;
    ULONG name_len;
    WCHAR *user_name;
    SESSION *session;
    KIRQL irql;

    if (proc)
        return STATUS_NOT_IMPLEMENTED;

    user_type = args->name_type.val;
    ProbeForWrite(user_type, sizeof(USHORT), sizeof(USHORT));

    name_len = args->name_len.val / sizeof(WCHAR);
    if ((! name_len) || name_len > 256)
        return STATUS_INVALID_PARAMETER;
    user_name = args->name_ptr.val;
    ProbeForWrite(user_name, name_len * sizeof(WCHAR), sizeof(WCHAR));

    *user_type = 0;
    *user_name = L'\0';
    status = STATUS_SUCCESS;

    session = Session_Get(FALSE, -1, &irql);
    if (! session)
        return STATUS_SUCCESS;

    __try {

        if (session->monitor_buf) {

            WCHAR *buf0 = session->monitor_buf;
            WCHAR *rptr = session->monitor_read_ptr;
            BOOLEAN first = TRUE;

            if (*rptr) {

                while (*rptr) {

                    if (first) {
                        first = FALSE;
                        *user_type = *rptr;
                    } else if (name_len) {
                        *user_name = *rptr;
                        ++user_name;
                        --name_len;
                    } else
                        status = STATUS_BUFFER_OVERFLOW;

                    ++rptr;
                    if (rptr >= buf0 + SESSION_MONITOR_BUF_SIZE)
                        rptr = buf0;
                }

                ++rptr;
                if (rptr >= buf0 + SESSION_MONITOR_BUF_SIZE)
                    rptr = buf0;
                session->monitor_read_ptr = rptr;

                if (name_len)
                    *user_name = L'\0';
            }
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    Session_Unlock(irql);

    return status;
}
