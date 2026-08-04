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
// Session Management
//---------------------------------------------------------------------------


#include "session.h"
#include "util.h"
#include "conf.h"
#include "api.h"
#include "process.h"
#include "obj.h"
#include "log_buff.h"


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

	LOG_BUFFER* monitor_log;

    BOOLEAN monitor_stack_trace;

    BOOLEAN monitor_overflow;

};


typedef struct _SESSION             SESSION;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static void Session_Unlock(KIRQL irql);

static SESSION *Session_Get(
    BOOLEAN create, ULONG SessionId, KIRQL *out_irql);

static BOOLEAN Session_CheckAdminAccess2(const WCHAR *setting);


//---------------------------------------------------------------------------


static NTSTATUS Session_Api_Leader(PROCESS *proc, ULONG64 *parms);

static NTSTATUS Session_Api_DisableForce(PROCESS *proc, ULONG64 *parms);

static NTSTATUS Session_Api_ForceChildren(PROCESS *proc, ULONG64 *parms);

static NTSTATUS Session_Api_MonitorControl(PROCESS *proc, ULONG64 *parms);

//static NTSTATUS Session_Api_MonitorPut(PROCESS *proc, ULONG64 *parms);

static NTSTATUS Session_Api_MonitorPut2(PROCESS *proc, ULONG64 *parms);

static NTSTATUS Session_Api_MonitorPutEx(PROCESS *proc, ULONG64 *parms);

//static NTSTATUS Session_Api_MonitorGet(PROCESS *proc, ULONG64 *parms);

static NTSTATUS Session_Api_MonitorGetEx(PROCESS *proc, ULONG64 *parms);

static NTSTATUS Session_Api_MonitorGet2(PROCESS *proc, ULONG64 *parms);

//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static LIST Session_List;
PERESOURCE Session_ListLock = NULL;

volatile LONG Session_MonitorCount = 0;


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
    Api_SetFunction(API_FORCE_CHILDREN,         Session_Api_ForceChildren);
    Api_SetFunction(API_MONITOR_CONTROL,        Session_Api_MonitorControl);
    //Api_SetFunction(API_MONITOR_PUT,            Session_Api_MonitorPut);
    Api_SetFunction(API_MONITOR_PUT2,           Session_Api_MonitorPut2);
    Api_SetFunction(API_MONITOR_PUT_EX,         Session_Api_MonitorPutEx);
    //Api_SetFunction(API_MONITOR_GET,            Session_Api_MonitorGet);
	Api_SetFunction(API_MONITOR_GET_EX,			Session_Api_MonitorGetEx);
    Api_SetFunction(API_MONITOR_GET2,            Session_Api_MonitorGet2);


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
            session->leader_pid = PsGetCurrentProcessId();

            List_Insert_After(&Session_List, NULL, session);
        }
    }

    if (! session)
        Session_Unlock(*out_irql);

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

            if (session->monitor_log) {
				log_buffer_free(session->monitor_log);
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


_FX BOOLEAN Session_CheckAdminAccess(BOOLEAN OnlyFull)
{
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
                (!OnlyFull && *pElevationType == TokenElevationTypeLimited))
                IsAdmin = TRUE;
            ExFreePool(pElevationType);
        }
    }

    PsDereferencePrimaryToken(pAccessToken);
    return IsAdmin;
}


//---------------------------------------------------------------------------
// Session_CheckAdminAccess
//---------------------------------------------------------------------------


_FX BOOLEAN Session_CheckAdminAccess2(const WCHAR *setting)
{
    if (!setting || Conf_Get_Boolean(NULL, setting, 0, FALSE)) {

        return Session_CheckAdminAccess(FALSE);
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
        else if (!MyIsCallerSigned()) 
            status = STATUS_INVALID_SIGNATURE; // STATUS_ACCESS_DENIED
        else {

            session = Session_Get(TRUE, -1, &irql);
            if (! session)
                status = STATUS_INSUFFICIENT_RESOURCES;
            else if (session->leader_pid != PsGetCurrentProcessId())
                status = STATUS_DEVICE_ALREADY_ATTACHED;  // STATUS_ALREADY_REGISTERED
        }

    } else {

        //
        // get leader
        //

        ULONG session_id = args->session_id.val;

        if (session_id == -1) {

            HANDLE TokenHandle = args->token_handle.val;

            ULONG len = sizeof(session_id);
            status = ZwQueryInformationToken(
                TokenHandle, TokenSessionId, &session_id, len, &len);
        }

        if (NT_SUCCESS(status)) {

            __try {

                session = Session_Get(FALSE, session_id, &irql);
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

        if (!MyIsCallerSigned())
            return STATUS_ACCESS_DENIED;

        ProbeForRead(in_flag, sizeof(ULONG), sizeof(ULONG));
        ULONG in_flag_value = *in_flag;
        if (in_flag_value) {

            if (! Session_CheckAdminAccess2(L"ForceDisableAdminOnly"))
                return STATUS_ACCESS_DENIED;
            KeQuerySystemTime(&time);

        } else
            time.QuadPart = 0;

        if (in_flag_value == DISABLE_JUST_THIS_PROCESS) {

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
// Session_Api_ForceChildren
//---------------------------------------------------------------------------


_FX NTSTATUS Session_Api_ForceChildren(PROCESS *proc, ULONG64 *parms)
{
    HANDLE process_id;
    WCHAR *user_boxname;
    WCHAR boxname[BOXNAME_COUNT];

    if (proc)
        return STATUS_NOT_IMPLEMENTED;

    if (!MyIsCallerSigned())
        return STATUS_ACCESS_DENIED;

    process_id = (HANDLE)parms[1];

    memzero(boxname, sizeof(boxname));
    user_boxname = (WCHAR *)parms[2];
    if (user_boxname) {
        ProbeForRead(user_boxname, sizeof(WCHAR) * (BOXNAME_COUNT - 2), sizeof(UCHAR));
        if (user_boxname[0])
            wcsncpy(boxname, user_boxname, (BOXNAME_COUNT - 2));
    }
    if(!process_id || process_id == (HANDLE)-1 || !boxname[0])
        return STATUS_INVALID_PARAMETER;

    Process_FcpInsert(process_id, boxname);

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Session_IsLeader
//---------------------------------------------------------------------------


_FX ULONG Session_GetLeadSession(HANDLE pid)
{
    NTSTATUS status;
    SESSION* session;
    KIRQL irql;
    ULONG lead_session = 0;

    KeRaiseIrql(APC_LEVEL, &irql);
    ExAcquireResourceExclusiveLite(Session_ListLock, TRUE);

    session = List_Head(&Session_List);
    while (session) {
        if (session->leader_pid == pid) {
            lead_session = session->session_id;
            break;
        }
        session = List_Next(session);
    }

    ExReleaseResourceLite(Session_ListLock);
    KeLowerIrql(irql);

    return lead_session;
}


//---------------------------------------------------------------------------
// Session_MonitorPut
//---------------------------------------------------------------------------


_FX void Session_MonitorPut(ULONG type, const WCHAR *name, HANDLE pid)
{
	const WCHAR* strings[2] = { name, NULL };
	Session_MonitorPutEx(type, strings, NULL, pid, PsGetCurrentThreadId());
}


//---------------------------------------------------------------------------
// Session_MonitorPutEx
//---------------------------------------------------------------------------


_FX void Session_MonitorPutEx(ULONG type, const WCHAR** strings, ULONG* lengths, HANDLE hpid, HANDLE htid)
{
    SESSION *session;
    KIRQL irql;

    session = Session_Get(FALSE, -1, &irql);
    if (! session)
        return;

    if (session->monitor_log) {

        LARGE_INTEGER timestamp = Util_GetTimestamp();

		ULONG pid = (ULONG)hpid;
        ULONG tid = (ULONG)htid;

		SIZE_T data_len = 0;
		for(int i=0; strings[i] != NULL; i++)
			data_len += ((lengths ? lengths [i] : wcslen(strings[i])) + 1) * sizeof(WCHAR);

        
		//[Time 8][Type 4][PID 4][TID 4][Data n*2](0xFFFF[ID1][LEN1][DATA1]...[IDn][LENn][DATAn])
		SIZE_T entry_size = 8 + 4 + 4 + 4 + data_len;

        PVOID backTrace[MAX_STACK_DEPTH];
        ULONG frames = 0;
        if (session->monitor_stack_trace) {
            frames = Util_CaptureStack(backTrace, MAX_STACK_DEPTH);
            entry_size += sizeof(WCHAR) + sizeof(ULONG) + sizeof(ULONG) + (frames * sizeof(PVOID));
        }

		CHAR* write_ptr = log_buffer_push_entry((LOG_BUFFER_SIZE_T)entry_size, session->monitor_log, FALSE);
		if (write_ptr) {
            WCHAR null_char = L'\0';
            log_buffer_push_bytes((CHAR*)&timestamp.QuadPart, 8, &write_ptr, session->monitor_log);
			log_buffer_push_bytes((CHAR*)&type, 4, &write_ptr, session->monitor_log);
			log_buffer_push_bytes((CHAR*)&pid, 4, &write_ptr, session->monitor_log);
            log_buffer_push_bytes((CHAR*)&tid, 4, &write_ptr, session->monitor_log);

			// add strings '\0' separated
            for (int i = 0; strings[i] != NULL; i++) {
                log_buffer_push_bytes((CHAR*)strings[i], (lengths ? lengths[i] : wcslen(strings[i])) * sizeof(WCHAR), &write_ptr, session->monitor_log);
                log_buffer_push_bytes((CHAR*)&null_char, sizeof(WCHAR), &write_ptr, session->monitor_log);
            }

            if (frames) {
                WCHAR strings_end = 0xFFFF;
                log_buffer_push_bytes((CHAR*)&strings_end, sizeof(WCHAR), &write_ptr, session->monitor_log);

                ULONG tag_id = 'STCK';
                ULONG tag_len = frames * sizeof(PVOID);
                log_buffer_push_bytes((CHAR*)&tag_id, sizeof(ULONG), &write_ptr, session->monitor_log);
                log_buffer_push_bytes((CHAR*)&tag_len, sizeof(ULONG), &write_ptr, session->monitor_log);
                log_buffer_push_bytes((CHAR*)backTrace, frames * sizeof(PVOID), &write_ptr, session->monitor_log);
            }
		}
        else if (!session->monitor_overflow) {
            session->monitor_overflow = TRUE;
            Log_Msg0(MSG_MONITOR_OVERFLOW);
        }
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
    ULONG *out_used;
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
            if (session->monitor_log)
                *out_flag = TRUE;
            Session_Unlock(irql);
        }
    }

    //out_used = args->get_used.val;
    //if (out_used) {
    //    ProbeForWrite(out_used, sizeof(ULONG), sizeof(ULONG));
    //    *out_used = 0;
    //    session = Session_Get(FALSE, -1, &irql);
    //    if (session) {
    //        if (session->monitor_log)
    //            *out_used = (ULONG)session->monitor_log->buffer_used;
    //        Session_Unlock(irql);
    //    }
    //}

    //
    // set status
    //

    in_flag = args->set_flag.val;
    if (in_flag) {
        ProbeForRead(in_flag, sizeof(ULONG), sizeof(ULONG));
        if (*in_flag) {

            if (! Session_CheckAdminAccess2(L"MonitorAdminOnly"))
                return STATUS_ACCESS_DENIED;

            EnableMonitor = TRUE;

        } else
            EnableMonitor = FALSE;

        session = Session_Get(FALSE, -1, &irql);
        if (session) {

            if (EnableMonitor && (! session->monitor_log)) {

                ULONG BuffSize = Conf_Get_Number(NULL, L"TraceBufferPages", 0, 256) * PAGE_SIZE;

				session->monitor_log = log_buffer_init(BuffSize * sizeof(WCHAR));
                if (!session->monitor_log) {
                    Log_Msg0(MSG_1201);
                    session->monitor_log = log_buffer_init(SESSION_MONITOR_BUF_SIZE * sizeof(WCHAR));
                }

                if (session->monitor_log) {
                    InterlockedIncrement(&Session_MonitorCount);
                } else
                    Log_Msg0(MSG_1201);

                session->monitor_stack_trace = Conf_Get_Boolean(NULL, L"MonitorStackTrace", 0, FALSE);

            } else if ((! EnableMonitor) && session->monitor_log) {

				log_buffer_free(session->monitor_log);
				session->monitor_log = NULL;
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


//_FX NTSTATUS Session_Api_MonitorPut(PROCESS *proc, ULONG64 *parms)
//{
//    API_MONITOR_GET_PUT_ARGS *args = (API_MONITOR_GET_PUT_ARGS *)parms;
//    API_MONITOR_PUT2_ARGS args2 = { args->func_code, args->log_type.val64, args->log_len.val64, args->log_ptr.val64, TRUE, 0 };
//
//    return Session_Api_MonitorPut2(proc, (ULONG64*)&args2);
//}


//---------------------------------------------------------------------------
// Session_Api_MonitorPut
//---------------------------------------------------------------------------


_FX NTSTATUS Session_Api_MonitorPut2(PROCESS *proc, ULONG64 *parms)
{
    API_MONITOR_PUT2_ARGS *args = (API_MONITOR_PUT2_ARGS *)parms;
    ULONG log_type;
    WCHAR *log_data;
    WCHAR *name;
    const WCHAR *type_pipe = L"Pipe";
    const WCHAR *type_file = L"File";
    const WCHAR *type_name = NULL;
    NTSTATUS status;
    ULONG log_len;

    if (! proc)
        return STATUS_NOT_IMPLEMENTED;

    if (! Session_MonitorCount || proc->disable_monitor)
        return STATUS_SUCCESS;

    log_type = args->log_type.val;
    if (!log_type)
        return STATUS_INVALID_PARAMETER;

	log_len = args->log_len.val / sizeof(WCHAR);
    if (!log_len)
        return STATUS_INVALID_PARAMETER;

    log_data = args->log_ptr.val;
    ProbeForRead(log_data, log_len * sizeof(WCHAR), sizeof(WCHAR));

    //
    // if we don't need to check_object_exists we can use a shortcut
    //

    if (!args->check_object_exists.val64){ 
        const WCHAR* strings[3] = { args->is_message.val64 ? Driver_Empty : log_data, args->is_message.val64 ? log_data : NULL, NULL };
        ULONG lengths[3] = { args->is_message.val64 ? 0 : log_len, args->is_message.val64 ? log_len : 0, 0 };
        Session_MonitorPutEx(log_type | MONITOR_USER, strings, lengths, proc->pid, PsGetCurrentThreadId());
        return STATUS_SUCCESS;
    }

    const ULONG max_buff = 2048;
	if (log_len > max_buff) // truncate as we only have 1028 in buffer
		log_len = max_buff;
    name = Mem_Alloc(proc->pool, (max_buff + 4) * sizeof(WCHAR)); // todo: should we increase this ?
    if (! name)
        return STATUS_INSUFFICIENT_RESOURCES;

    //
    // we do everything else within a try/except block to make sure
    // that we always free the name buffer regardless of errors
    //

    __try {

        wmemcpy(name, log_data, log_len);
        name[log_len] = L'\0';

        status = STATUS_SUCCESS;

        if (args->check_object_exists.val64 && ((log_type & MONITOR_TRACE) == 0)) { // do not check objects if this is a trace entry

            UNICODE_STRING objname;
            void* object = NULL;

            //
            // if type is MONITOR_IPC we try to open the object
            // to get the name assigned to it at time of creation
            //

            if ((log_type & MONITOR_TYPE_MASK) == MONITOR_IPC) {

                ULONG i;

                RtlInitUnicodeString(&objname, name);

                for (i = 0; Obj_ObjectTypes[i]; ++i) {

                    // ObReferenceObjectByName needs a non-zero ObjectType
                    // so we have to keep going through all possible object
                    // types as long as we get STATUS_OBJECT_TYPE_MISMATCH

                    status = ObReferenceObjectByName(
                                &objname, OBJ_CASE_INSENSITIVE, NULL, 0,
                                Obj_ObjectTypes[i], KernelMode, NULL,
                                &object);

                    if (status != STATUS_OBJECT_TYPE_MISMATCH) {
                        type_name = Obj_ObjectTypes[i]->Name.Buffer;
                        break;
                    }
                }

                // DbgPrint("IPC  Status = %08X Object = %08X for Open <%S>\n", status, object, name);
            }

            //
            // if type is MONITOR_PIPE we try to open the pipe
            // to get the name assigned to it at time of creation
            //

            else if ((log_type & MONITOR_TYPE_MASK) == MONITOR_PIPE) {

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

                type_name = type_pipe;

                //DbgPrint("PIPE Status3 = %08X Object = %08X for Open <%S>\n", status, object, name);
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

				    log_len = Name->Name.Length / sizeof(WCHAR);
                    if (log_len > max_buff) // truncate as we only have 1028 in buffer
					    log_len = max_buff;
                    wmemcpy(name, Name->Name.Buffer, log_len);
                    name[log_len] = L'\0';

                    if (Name != &Obj_Unnamed)
                        Mem_Free(Name, NameLength);

                    // DbgPrint("Determined Object Name <%S>\n", name);
                }

                ObDereferenceObject(object);
            }

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

        /*if (! NT_SUCCESS(status)) {
            name[0] = L'?';
            name[1] = L'\0';
        }*/

        const WCHAR* strings[4] = { name, L"", type_name, NULL };
        Session_MonitorPutEx(log_type | MONITOR_USER, strings, NULL, proc->pid, PsGetCurrentThreadId());
    }

    Mem_Free(name, (max_buff + 4) * sizeof(WCHAR));

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Session_Api_MonitorGet
//---------------------------------------------------------------------------


_FX NTSTATUS Session_Api_MonitorPutEx(PROCESS* proc, ULONG64* parms)
{
    API_MONITOR_PUT_EX_ARGS* args = (API_MONITOR_PUT_EX_ARGS*)parms;
    ULONG log_type;
    WCHAR *log_data;
    WCHAR *name;
    NTSTATUS status;
    ULONG log_len;

    //
    // caller must not be sandboxed
    //

    if (proc)
        return STATUS_ACCESS_DENIED;

    if (! Session_MonitorCount)
        return STATUS_SUCCESS;

    log_type = args->log_type.val;
    if (!log_type)
        return STATUS_INVALID_PARAMETER;

    log_len = args->log_len.val / sizeof(WCHAR);
    if (!log_len)
        return STATUS_INVALID_PARAMETER;

    log_data = args->log_ptr.val;
    ProbeForRead(log_data, log_len * sizeof(WCHAR), sizeof(WCHAR));

    //
    // if we don't need to check_object_exists we can use a shortcut
    //

    const WCHAR* strings[3] = { Driver_Empty, log_data, NULL };
    ULONG lengths[3] = { 0, log_len, 0 };
    Session_MonitorPutEx(log_type | MONITOR_USER, strings, lengths, (HANDLE)args->log_pid.val, (HANDLE)args->log_tid.val);
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Session_Api_MonitorGet
//---------------------------------------------------------------------------

//_FX NTSTATUS Session_Api_MonitorGet(PROCESS *proc, ULONG64 *parms)
//{
//	API_MONITOR_GET_PUT_ARGS *args = (API_MONITOR_GET_PUT_ARGS *)parms;
//	API_MONITOR_GET_EX_ARGS args2 = { args->func_code, 0, args->log_type.val64, 0, args->log_len.val64, args->log_ptr.val64 };
//
//	return Session_Api_MonitorGetEx(proc, (ULONG64*)&args2);
//}

//---------------------------------------------------------------------------
// Session_Api_MonitorGetEx
//---------------------------------------------------------------------------

_FX NTSTATUS Session_Api_MonitorGetEx(PROCESS* proc, ULONG64* parms)
{
    API_MONITOR_GET_EX_ARGS* args = (API_MONITOR_GET_EX_ARGS*)parms;
    NTSTATUS status;
    //ULONG* seq_num;
    LARGE_INTEGER timestamp;
    ULONG* log_type;
    ULONG* log_pid;
    ULONG* log_tid;
    UNICODE_STRING64* log_data;
    WCHAR* log_buffer;
    SESSION* session;
    KIRQL irql;

    if (proc)
        return STATUS_NOT_IMPLEMENTED;

    // Note: when logging a lot of enries the performance is to low when keeping entries
    //seq_num = args->log_seq.val;
    //if (seq_num != NULL) {
    //    ProbeForRead(seq_num, sizeof(ULONG), sizeof(ULONG));
    //    ProbeForWrite(seq_num, sizeof(ULONG), sizeof(ULONG));
    //}

    log_type = args->log_type.val;
    ProbeForWrite(log_type, sizeof(ULONG), sizeof(ULONG));
    *log_type = 0;

    log_pid = args->log_pid.val;
    if (log_pid != NULL) {
        ProbeForWrite(log_pid, sizeof(ULONG), sizeof(ULONG));
        *log_pid = 0;
    }

    log_tid = args->log_tid.val;
    if (log_tid != NULL) {
        ProbeForWrite(log_tid, sizeof(ULONG), sizeof(ULONG));
        *log_tid = 0;
    }

    log_data = args->log_data.val;
    if (!log_data)
        return STATUS_INVALID_PARAMETER;
    ProbeForRead(log_data, sizeof(UNICODE_STRING64), sizeof(ULONG));
    ProbeForWrite(log_data, sizeof(UNICODE_STRING64), sizeof(ULONG));

    log_buffer = (WCHAR*)log_data->Buffer;
    if (!log_buffer)
        return STATUS_INVALID_PARAMETER;
    
    status = STATUS_SUCCESS;

    session = Session_Get(FALSE, -1, &irql);
    if (!session)
        return STATUS_UNSUCCESSFUL;

    __try {

        if (!session->monitor_log) {

            status = STATUS_DEVICE_NOT_READY;
            __leave;
        }

        CHAR* read_ptr = NULL;
        //if (seq_num != NULL)
        //    read_ptr = log_buffer_get_next(*seq_num, session->monitor_log);
        //else 
        if (session->monitor_log->buffer_used > 0)
            read_ptr = session->monitor_log->buffer_start_ptr;

        if (!read_ptr) {
            if(session->monitor_overflow)
                session->monitor_overflow = FALSE;
            status = STATUS_NO_MORE_ENTRIES;
            __leave;
        }

        LOG_BUFFER_SIZE_T entry_size = log_buffer_get_size(&read_ptr, session->monitor_log);
        LOG_BUFFER_SEQ_T seq_number = log_buffer_get_seq_num(&read_ptr, session->monitor_log);

        //if (seq_num != NULL && seq_number != *seq_num + 1) {
        //
        //	status = STATUS_REQUEST_OUT_OF_SEQUENCE;
        //	*seq_num = seq_number - 1;
        //	__leave;
        //}

        //[Time 8][Type 4][PID 4][TID 4][Data n*2]

        log_buffer_get_bytes((CHAR*)&timestamp.QuadPart, 8, &read_ptr, session->monitor_log);

        log_buffer_get_bytes((CHAR*)log_type, 4, &read_ptr, session->monitor_log);

        ULONG pid;
        log_buffer_get_bytes((CHAR*)&pid, 4, &read_ptr, session->monitor_log);
        if (log_pid != NULL)
            *log_pid = pid;

        ULONG tid;
        log_buffer_get_bytes((CHAR*)&tid, 4, &read_ptr, session->monitor_log);
        if (log_tid != NULL)
            *log_tid = tid;

        ULONG data_size = (entry_size - (4 + 4 + 4));
        if ((USHORT)data_size > (log_data->MaximumLength - 1))
        {
            data_size = (log_data->MaximumLength - 1);
            status = STATUS_BUFFER_TOO_SMALL;
        }
        
        log_data->Length = (USHORT)data_size;
        ProbeForWrite(log_buffer, data_size + 1, sizeof(WCHAR));
        log_buffer_get_bytes((CHAR*)log_buffer, data_size, &read_ptr, session->monitor_log);

        log_buffer[data_size / sizeof(wchar_t)] = L'\0';
        

        //if (seq_num != NULL)
        //    *seq_num = seq_number;
        //else // for compatibility with older versions we fall back to clearing the returned entry
        log_buffer_pop_entry(session->monitor_log);

    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    Session_Unlock(irql);

    return status;
}

//---------------------------------------------------------------------------
// Session_Api_MonitorGet2
//---------------------------------------------------------------------------

_FX NTSTATUS Session_Api_MonitorGet2(PROCESS *proc, ULONG64 *parms)
{
	API_MONITOR_GET2_ARGS *args = (API_MONITOR_GET2_ARGS *)parms;
	NTSTATUS status;
    ULONG buffer_len;
    UCHAR* buffer_ptr;
    SESSION* session;
    KIRQL irql;

    if (proc)
        return STATUS_NOT_IMPLEMENTED;

    ProbeForRead(args->buffer_len.val, sizeof(ULONG), sizeof(ULONG));
    buffer_len = *args->buffer_len.val;
    ProbeForWrite(args->buffer_len.val, sizeof(ULONG), sizeof(ULONG));
    *args->buffer_len.val = 0;

    ProbeForWrite(args->buffer_ptr.val, buffer_len, sizeof(UCHAR));
    buffer_ptr = (UCHAR*)args->buffer_ptr.val;

    status = STATUS_SUCCESS;

    session = Session_Get(FALSE, -1, &irql);
    if (!session)
        return STATUS_UNSUCCESSFUL;

    __try {

        if (!session->monitor_log) {

            status = STATUS_DEVICE_NOT_READY;
            __leave;
        }

        if (session->monitor_log->buffer_used == 0) {
            if(session->monitor_overflow)
                session->monitor_overflow = FALSE;
            status = STATUS_NO_MORE_ENTRIES;
            __leave;
        }

        while (session->monitor_log->buffer_used > 0)
        {
            CHAR* read_ptr = session->monitor_log->buffer_start_ptr;

            LOG_BUFFER_SIZE_T entry_size = log_buffer_get_size(&read_ptr, session->monitor_log);
            LOG_BUFFER_SEQ_T seq_number = log_buffer_get_seq_num(&read_ptr, session->monitor_log);
            if (entry_size > buffer_len - sizeof(LOG_BUFFER_SIZE_T)) {
                status = STATUS_MORE_ENTRIES;
                break;
            }

            *(LOG_BUFFER_SIZE_T*)buffer_ptr = entry_size;
            buffer_ptr += sizeof(LOG_BUFFER_SIZE_T);
            buffer_len -= sizeof(LOG_BUFFER_SIZE_T);

            log_buffer_get_bytes((CHAR*)buffer_ptr, entry_size, &read_ptr, session->monitor_log);
            buffer_ptr += entry_size;
            buffer_len -= entry_size;

            log_buffer_pop_entry(session->monitor_log);
        }

        // always terminate with null length
        *(LOG_BUFFER_SIZE_T*)buffer_ptr = 0;
        buffer_ptr += sizeof(LOG_BUFFER_SIZE_T);
        buffer_len -= sizeof(LOG_BUFFER_SIZE_T);

        // return total used buffer length
        *args->buffer_len.val = (ULONG)(buffer_ptr - (UCHAR*)args->buffer_ptr.val);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    Session_Unlock(irql);

    return status;
}