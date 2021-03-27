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
// Process Management:  API for User Mode
//---------------------------------------------------------------------------


#include "process.h"
#include "api.h"
#include "util.h"
#include "conf.h"
#include "token.h"
#include "file.h"
#include "key.h"
#include "ipc.h"
#include "common/pattern.h"
#include "common/my_version.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static NTSTATUS Process_Api_CopyBoxPathsToUser(
    BOX *box, ULONG *file_path_len, ULONG *key_path_len, ULONG *ipc_path_len,
    UNICODE_STRING64 *file_path,
    UNICODE_STRING64 *key_path,
    UNICODE_STRING64 *ipc_path);


//---------------------------------------------------------------------------
// Process_Api_Start
//---------------------------------------------------------------------------


_FX NTSTATUS Process_Api_Start(PROCESS *proc, ULONG64 *parms)
{
    LONG_PTR user_box_parm;
    HANDLE user_pid_parm;
    BOX *box = NULL;
    PEPROCESS ProcessObject = NULL;
    NTSTATUS status;

    //
    // already sandboxed?
    //

    if (proc || (PsGetCurrentProcessId() != Api_ServiceProcessId))
        return STATUS_NOT_IMPLEMENTED;

    //
    // if not ready, don't even try
    //

    if (! Process_ReadyToSandbox)
        return STATUS_SERVER_DISABLED;

    //
    // first parameter is box name or box model pid
    //

    user_box_parm = (LONG_PTR)parms[1];

    if (user_box_parm < 0) {

        //
        // if paramter is negative, it specifies the pid number for a
        // process, from which we copy the box information, including
        // SID and session
        //

        PROCESS *proc2;
        KIRQL irql;

        proc2 = Process_Find((HANDLE)(-user_box_parm), &irql);
        if (proc2)
            box = Box_Clone(Driver_Pool, proc2->box);

        ExReleaseResourceLite(Process_ListLock);
        KeLowerIrql(irql);

        if (! proc2)
            return STATUS_INVALID_CID;

        if (! box)
            return STATUS_INSUFFICIENT_RESOURCES;

    } else {

        //
        // otherwise parameter specifies the box name to use, and the
        // thread impersonation token specifies SID and session
        //

        WCHAR boxname[34];

        void *TokenObject;
        BOOLEAN CopyOnOpen;
        BOOLEAN EffectiveOnly;
        SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;

        UNICODE_STRING SidString;
        ULONG SessionId;

        if (! Api_CopyBoxNameFromUser(boxname, (WCHAR *)user_box_parm))
            return STATUS_INVALID_PARAMETER;

        TokenObject = PsReferenceImpersonationToken(PsGetCurrentThread(),
                        &CopyOnOpen, &EffectiveOnly, &ImpersonationLevel);

        if (! TokenObject)
            return STATUS_NO_IMPERSONATION_TOKEN;

        status = SeQuerySessionIdToken(TokenObject, &SessionId);

        if (NT_SUCCESS(status)) {

            status = Token_QuerySidString(TokenObject, &SidString);
        }

        PsDereferenceImpersonationToken(TokenObject);

        if (! NT_SUCCESS(status))
            return status;

        box = Box_CreateEx(
                Driver_Pool, boxname, SidString.Buffer, SessionId, TRUE);

        RtlFreeUnicodeString(&SidString);

        if (! box)
            return STATUS_INSUFFICIENT_RESOURCES;

        if (! Conf_IsBoxEnabled(boxname, box->sid, box->session_id)) {
            Box_Free(box);
            return STATUS_ACCOUNT_RESTRICTION;
        }
    }

    //
    // second parameter is the process id for the new process
    //

    user_pid_parm = (HANDLE)parms[2];

    if (! user_pid_parm)
        status = STATUS_INVALID_CID;
    else
        status = PsLookupProcessByProcessId(user_pid_parm, &ProcessObject);

    if (NT_SUCCESS(status)) {

        if (PsGetProcessSessionId(ProcessObject) != box->session_id) {

            status = STATUS_LOGON_SESSION_COLLISION;

        } else {

            Process_NotifyProcess_Create(
                                user_pid_parm, Api_ServiceProcessId, Api_ServiceProcessId, box);

            box = NULL;         // freed by Process_NotifyProcess_Create
        }

        ObDereferenceObject(ProcessObject);
    }

    if (box)
        Box_Free(box);

    return status;
}


//---------------------------------------------------------------------------
// Process_Api_Query
//---------------------------------------------------------------------------


_FX NTSTATUS Process_Api_Query(PROCESS *proc, ULONG64 *parms)
{
    API_QUERY_PROCESS_ARGS *args = (API_QUERY_PROCESS_ARGS *)parms;
    NTSTATUS status;
    HANDLE ProcessId;
    ULONG *num32;
    ULONG64 *num64;
    KIRQL irql;

    //
    // this is the first SbieApi call by SbieDll
    //

    if (proc)
        proc->sbiedll_loaded = TRUE;

    //
    // if a ProcessId was specified, then locate and lock the matching
    // process. ProcessId must be specified if the caller is not sandboxed
    //

    ProcessId = args->process_id.val;
    if (proc) {
        if (ProcessId == proc->pid || IS_ARG_CURRENT_PROCESS(ProcessId))
            ProcessId = 0;  // don't have to search for the current pid
    } else {
        if ((! ProcessId) || IS_ARG_CURRENT_PROCESS(ProcessId))
            return STATUS_INVALID_CID;
    }
    if (ProcessId) {

        proc = Process_Find(ProcessId, &irql);
        if (! proc) {
            ExReleaseResourceLite(Process_ListLock);
            KeLowerIrql(irql);
            return STATUS_INVALID_CID;
        }
    }

    //
    // the rest of the code now has to be protected, because we may
    // have a lock on the Process_List, that we must release
    //

    status = STATUS_SUCCESS;

    __try {

        // boxname unicode string can be specified in parameter 2

        Api_CopyStringToUser(
            (UNICODE_STRING64 *)parms[2],
            proc->box->name, proc->box->name_len);

        // image name unicode string can be specified in parameter 3

        Api_CopyStringToUser(
            (UNICODE_STRING64 *)parms[3],
            proc->image_name, proc->image_name_len);

        // sid unicode string can be specified in parameter 4

        Api_CopyStringToUser(
            (UNICODE_STRING64 *)parms[4],
            proc->box->sid, proc->box->sid_len);

        // session_id number can be specified in parameter 5

        num32 = (ULONG *)parms[5];
        if (num32) {
            ProbeForWrite(num32, sizeof(ULONG), sizeof(ULONG));
            *num32 = proc->box->session_id;
        }

        // create_time timestamp can be specified in parameter 6

        num64 = (ULONG64 *)parms[6];
        if (num64) {
            ProbeForWrite(num64, sizeof(ULONG64), sizeof(ULONG));
            *num64 = proc->create_time;
        }

    //
    // release the lock, if taken
    //

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    if (ProcessId) {

        ExReleaseResourceLite(Process_ListLock);
        KeLowerIrql(irql);
    }

    return status;
}


//---------------------------------------------------------------------------
// Process_Api_QueryInfo
//---------------------------------------------------------------------------


_FX NTSTATUS Process_Api_QueryInfo(PROCESS *proc, ULONG64 *parms)
{
    API_QUERY_PROCESS_INFO_ARGS *args = (API_QUERY_PROCESS_INFO_ARGS *)parms;
    NTSTATUS status;
    HANDLE ProcessId;
    KIRQL irql;
	BOOLEAN is_caller_sandboxed = FALSE;

    //
    // if a ProcessId was specified, then locate and lock the matching
    // process. ProcessId must be specified if the caller is not sandboxed
    //

    ProcessId = args->process_id.val;
    if (proc) {
		is_caller_sandboxed = TRUE;
        if (ProcessId == proc->pid || IS_ARG_CURRENT_PROCESS(ProcessId))
            ProcessId = 0;  // don't have to search for the current pid
    } else {
        if ((! ProcessId) || IS_ARG_CURRENT_PROCESS(ProcessId))
            return STATUS_INVALID_CID;
    }
    if (ProcessId) {

        proc = Process_Find(ProcessId, &irql);
        if (! proc) {
            ExReleaseResourceLite(Process_ListLock);
            KeLowerIrql(irql);
            return STATUS_INVALID_CID;
        }
    }

    // the rest of the code now has to be protected, because we may
    // have a lock on the Process_List, that we must release

    status = STATUS_SUCCESS;

    __try {

        ULONG64 *data = args->info_data.val;
        ProbeForWrite(data, sizeof(ULONG64), sizeof(ULONG64));

        if (args->info_type.val == 0) {

            ULONG64 flags = 0;

            if (!proc->bHostInject)
            {
                flags = SBIE_FLAG_VALID_PROCESS;

                if (proc->forced_process)
                    flags |= SBIE_FLAG_FORCED_PROCESS;
                if (proc->is_start_exe)
                    flags |= SBIE_FLAG_PROCESS_IS_START_EXE;
                if (proc->parent_was_start_exe)
                    flags |= SBIE_FLAG_PARENT_WAS_START_EXE;
                if (proc->drop_rights)
                    flags |= SBIE_FLAG_DROP_RIGHTS;
                if (proc->rights_dropped)
                    flags |= SBIE_FLAG_RIGHTS_DROPPED;
                if (proc->untouchable)
                    flags |= SBIE_FLAG_PROTECTED_PROCESS;
                if (proc->image_sbie)
                    flags |= SBIE_FLAG_IMAGE_FROM_SBIE_DIR;
                if (proc->image_from_box)
                    flags |= SBIE_FLAG_IMAGE_FROM_SANDBOX;
                if (proc->in_pca_job)
                    flags |= SBIE_FLAG_PROCESS_IN_PCA_JOB;

                if (proc->create_console_flag == 'S')
                    flags |= SBIE_FLAG_CREATE_CONSOLE_SHOW;
                else if (proc->create_console_flag == 'H')
                    flags |= SBIE_FLAG_CREATE_CONSOLE_HIDE;

                if (proc->open_all_win_classes)
                    flags |= SBIE_FLAG_OPEN_ALL_WIN_CLASS;
            }
            else
            {
                flags = SBIE_FLAG_HOST_INJECT_PROCESS;
            }

            *data = flags;

        } else if (args->info_type.val == 'pril') {

            *data = proc->integrity_level;

        } else if (args->info_type.val == 'nt32') {

            *data = proc->ntdll32_base;

        } else if (args->info_type.val == 'ptok') {

			if(is_caller_sandboxed)
				status = STATUS_ACCESS_DENIED;
			else
			{
				void *PrimaryTokenObject = proc->primary_token;
				if (PrimaryTokenObject)
				{
					ObReferenceObject(PrimaryTokenObject);

					HANDLE MyTokenHandle;
					status = ObOpenObjectByPointer(PrimaryTokenObject, 0, NULL, TOKEN_QUERY | TOKEN_DUPLICATE, *SeTokenObjectType, UserMode, &MyTokenHandle);

					ObDereferenceObject(PrimaryTokenObject);

					*data = (ULONG64)MyTokenHandle;
				}
				else
					status = STATUS_NOT_FOUND;
			}

		} else if (args->info_type.val == 'ippt') { // is primary process token

            HANDLE handle = (HANDLE)(args->ext_data.val);

            OBJECT_TYPE* object;
            status = ObReferenceObjectByHandle(handle, 0, NULL, UserMode, &object, NULL);
            if (NT_SUCCESS(status))
            {
                *data = (object == proc->primary_token);

                ObDereferenceObject(object);
            }

        } else
            status = STATUS_INVALID_INFO_CLASS;

    //
    // release the lock, if taken
    //

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    if (ProcessId) {

        ExReleaseResourceLite(Process_ListLock);
        KeLowerIrql(irql);
    }

    return status;
}


//---------------------------------------------------------------------------
// Process_Api_QueryBoxPath
//---------------------------------------------------------------------------


_FX NTSTATUS Process_Api_QueryBoxPath(PROCESS *proc, ULONG64 *parms)
{
    API_QUERY_BOX_PATH_ARGS *args = (API_QUERY_BOX_PATH_ARGS *)parms;
    BOX *box;
    BOOLEAN free_box;
    NTSTATUS status;

    //
    // if this API is invoked by a sandboxed process, use its box.
    // otherwise, construct a box according to specified box name
    //

    if (proc) {

        box = proc->box;
        free_box = FALSE;

    } else {

        WCHAR boxname[34];
        BOOLEAN ok = Api_CopyBoxNameFromUser(
            boxname, (WCHAR *)args->box_name.val);
        if (! ok)
            return STATUS_INVALID_PARAMETER;

        box = Box_Create(Driver_Pool, boxname, TRUE);
        if (! box)
            return STATUS_UNSUCCESSFUL;
        free_box = TRUE;
    }

    status = Process_Api_CopyBoxPathsToUser(
                box,
                args->file_path_len.val,
                args->key_path_len.val,
                args->ipc_path_len.val,
                args->file_path.val,
                args->key_path.val,
                args->ipc_path.val);

    if (free_box)
        Box_Free(box);

    return status;
}


//---------------------------------------------------------------------------
// Process_Api_QueryProcessPath
//---------------------------------------------------------------------------


_FX NTSTATUS Process_Api_QueryProcessPath(PROCESS *proc, ULONG64 *parms)
{
    API_QUERY_PROCESS_PATH_ARGS *args = (API_QUERY_PROCESS_PATH_ARGS *)parms;
    HANDLE ProcessId;
    NTSTATUS status;
    KIRQL irql;

    //
    // if a ProcessId was specified, then locate and lock the matching
    // process. ProcessId must be specified if the caller is not sandboxed
    //

    ProcessId = args->process_id.val;
    if (proc) {
        if (ProcessId == proc->pid || IS_ARG_CURRENT_PROCESS(ProcessId))
            ProcessId = 0;  // don't have to search for the current pid
    } else {
        if ((! ProcessId) || IS_ARG_CURRENT_PROCESS(ProcessId))
            return STATUS_INVALID_CID;
    }
    if (ProcessId) {

        proc = Process_Find(ProcessId, &irql);
        if ((! proc) || proc->terminated) {
            ExReleaseResourceLite(Process_ListLock);
            KeLowerIrql(irql);
            return STATUS_INVALID_CID;
        }
    }

    status = Process_Api_CopyBoxPathsToUser(
                proc->box,
                args->file_path_len.val,
                args->key_path_len.val,
                args->ipc_path_len.val,
                args->file_path.val,
                args->key_path.val,
                args->ipc_path.val);

    if (ProcessId) {

        ExReleaseResourceLite(Process_ListLock);
        KeLowerIrql(irql);
    }

    return status;
}


//---------------------------------------------------------------------------
// Process_Api_CopyBoxPathsToUser
//---------------------------------------------------------------------------


_FX NTSTATUS Process_Api_CopyBoxPathsToUser(
    BOX *box, ULONG *file_path_len, ULONG *key_path_len, ULONG *ipc_path_len,
    UNICODE_STRING64 *file_path,
    UNICODE_STRING64 *key_path,
    UNICODE_STRING64 *ipc_path)
{
    NTSTATUS status;

    __try {

    //
    // store the result lengths, if parameters were given
    //

    if (file_path_len) {
        ProbeForWrite(file_path_len, sizeof(ULONG), sizeof(ULONG));
        *file_path_len = box->file_path_len;
    }

    if (key_path_len) {
        ProbeForWrite(key_path_len, sizeof(ULONG), sizeof(ULONG));
        *key_path_len = box->key_path_len;
    }

    if (ipc_path_len) {
        ProbeForWrite(ipc_path_len, sizeof(ULONG), sizeof(ULONG));
        *ipc_path_len = box->ipc_path_len;
    }

    //
    // copy the strings, if specified
    //

    Api_CopyStringToUser(file_path, box->file_path, box->file_path_len);
    Api_CopyStringToUser(key_path, box->key_path, box->key_path_len);
    Api_CopyStringToUser(ipc_path, box->ipc_path, box->ipc_path_len);

    status = STATUS_SUCCESS;

    //
    // finish
    //

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    return status;
}


//---------------------------------------------------------------------------
// Process_Api_QueryPathList
//---------------------------------------------------------------------------


_FX NTSTATUS Process_Api_QueryPathList(PROCESS *proc, ULONG64 *parms)
{
    API_QUERY_PATH_LIST_ARGS *args = (API_QUERY_PATH_LIST_ARGS *)parms;
    PERESOURCE lock;
    LIST *list;
    WCHAR *path;
    PATTERN *pat;
    NTSTATUS status;
    ULONG path_len;
    KIRQL irql;
    BOOLEAN process_list_locked;

    //
    // caller can either be a sandboxed process asking its own path list,
    // or the Sandboxie Service asking the path list of a sandboxed process
    //

    if (proc) {

        process_list_locked = FALSE;

    } else {

        if (! MyIsCurrentProcessRunningAsLocalSystem())
            return STATUS_NOT_IMPLEMENTED;

        proc = Process_Find(args->process_id.val, &irql);

        if (! proc) {

            ExReleaseResourceLite(Process_ListLock);
            KeLowerIrql(irql);
            return STATUS_INVALID_CID;
        }

        process_list_locked = TRUE;
    }

    //
    // select path list based on the parameter given
    //

    if (args->path_code.val == 'fo') {
        list   = &proc->open_file_paths;
        lock   =  proc->file_lock;
    } else if (args->path_code.val == 'fc') {
        list   = &proc->closed_file_paths;
        lock   =  proc->file_lock;
    } else if (args->path_code.val == 'fr') {
        list   = &proc->read_file_paths;
        lock   =  proc->file_lock;
    } else if (args->path_code.val == 'fw') {
        list   = &proc->write_file_paths;
        lock   =  proc->file_lock;

    } else if (args->path_code.val == 'ko') {
        list   = &proc->open_key_paths;
        lock   =  proc->key_lock;
    } else if (args->path_code.val == 'kc') {
        list   = &proc->closed_key_paths;
        lock   =  proc->key_lock;
    } else if (args->path_code.val == 'kr') {
        list   = &proc->read_key_paths;
        lock   =  proc->key_lock;
    } else if (args->path_code.val == 'kw') {
        list   = &proc->write_key_paths;
        lock   =  proc->key_lock;

    } else if (args->path_code.val == 'io') {
        list   = &proc->open_ipc_paths;
        lock   =  proc->ipc_lock;
    } else if (args->path_code.val == 'ic') {
        list   = &proc->closed_ipc_paths;
        lock   =  proc->ipc_lock;

    } else if (args->path_code.val == 'wo') {
        list   = &proc->open_win_classes;
        lock   =  proc->gui_lock;

    } else {

        if (process_list_locked) {
            ExReleaseResourceLite(Process_ListLock);
            KeLowerIrql(irql);
        }

        return STATUS_INVALID_PARAMETER;
    }

    //
    // take a lock on the specified path list.  note that if we called
    // Process_Find above, then IRQL was already raised
    //

    if (! process_list_locked)
        KeRaiseIrql(APC_LEVEL, &irql);

    ExAcquireResourceSharedLite(lock, TRUE);

    //
    // count the length of the desired path list
    //

    path_len = 0;

    pat = List_Head(list);
    while (pat) {
        path_len += (wcslen(Pattern_Source(pat)) + 1) * sizeof(WCHAR);
        pat = List_Next(pat);
    }

    path_len += sizeof(WCHAR);

    //
    // copy data to caller
    //

    __try {

        if (args->path_len.val) {

            //
            // if the length parameter was specified, store length
            //

            ProbeForWrite(args->path_len.val, sizeof(ULONG), sizeof(ULONG));
            *args->path_len.val = path_len;

        } else {

            //
            // otherwise store the paths into the output buffer
            //

            path = args->path_str.val;
            ProbeForWrite(path, path_len, sizeof(WCHAR));

            pat = List_Head(list);
            while (pat) {
                const WCHAR *pat_src = Pattern_Source(pat);
                ULONG pat_len = wcslen(pat_src) + 1;
                wmemcpy(path, pat_src, pat_len);
                path += pat_len;
                pat = List_Next(pat);
            }

            *path = L'\0';
        }

        status = STATUS_SUCCESS;

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    //
    // release locks and finish
    //

    ExReleaseResourceLite(lock);

    if (process_list_locked)
        ExReleaseResourceLite(Process_ListLock);

    KeLowerIrql(irql);

    return status;
}


//---------------------------------------------------------------------------
// Process_Enumerate
//---------------------------------------------------------------------------


_FX NTSTATUS Process_Enumerate(
    const WCHAR *boxname, BOOLEAN all_sessions, ULONG session_id,
    ULONG *pids, ULONG *count)
{
    NTSTATUS status;
    PROCESS *proc1;
    ULONG num;
    KIRQL irql;

    if (count == NULL)
        return STATUS_INVALID_PARAMETER;

    //
    // return only processes of the caller user in their logon session
    //

    if ((! all_sessions) && (session_id == -1)) {

        status = MyGetSessionId(&session_id);
        if (! NT_SUCCESS(status))
            return status;
    }

    KeRaiseIrql(APC_LEVEL, &irql);
    ExAcquireResourceSharedLite(Process_ListLock, TRUE);

    __try {

        num = 0;
        proc1 = List_Head(&Process_List);
        while (proc1) {

            BOX *box1 = proc1->box;
            if (box1 && !proc1->bHostInject) {
                BOOLEAN same_box =
                    (! boxname[0]) || (_wcsicmp(box1->name, boxname) == 0);
                BOOLEAN same_session =
                    (all_sessions || box1->session_id == session_id);
                if (same_box && same_session) {
                    if (pids) {
						if(num >= *count)
							break;
                        pids[num] = (ULONG)(ULONG_PTR)proc1->pid;
                    }
                    ++num;
                }
            }

            proc1 = (PROCESS *)List_Next(proc1);
        }

        *count = num;

        status = STATUS_SUCCESS;

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    ExReleaseResourceLite(Process_ListLock);
    KeLowerIrql(irql);

    return status;
}


//---------------------------------------------------------------------------
// Process_Api_Enum
//---------------------------------------------------------------------------


_FX NTSTATUS Process_Api_Enum(PROCESS *proc, ULONG64 *parms)
{
    NTSTATUS status;
    ULONG count;
    ULONG *user_pids;                   // user mode ULONG [512]
    WCHAR *user_boxname;                // user mode WCHAR [34]
    BOOLEAN all_sessions;
    ULONG session_id;
    WCHAR boxname[48];
    ULONG *user_count;

    // get boxname from second parameter

    memzero(boxname, sizeof(boxname));
    if (proc)
        wcscpy(boxname, proc->box->name);
    user_boxname = (WCHAR *)parms[2];
    if ((! boxname[0]) && user_boxname) {
        ProbeForRead(user_boxname, sizeof(WCHAR) * 32, sizeof(UCHAR));
        if (user_boxname[0])
            wcsncpy(boxname, user_boxname, 32);
    }

    // get "all users/current user only" flag from third parameter

    all_sessions = FALSE;
    if (parms[3])
        all_sessions = TRUE;

    session_id = (ULONG)parms[4];

    // get user pid buffer from first parameter

    user_count = (ULONG *)parms[5];
    user_pids = (ULONG *)parms[1];
    
    if (user_count) {
        ProbeForRead(user_count, sizeof(ULONG), sizeof(ULONG));
        count = user_pids ? *user_count : 0;
    }
    else // legacy case
    {
        if (!user_pids)
            return STATUS_INVALID_PARAMETER;
        count = API_MAX_PIDS - 1;
        user_count = user_pids;
        user_pids += 1;
    }

    ProbeForWrite(user_count, sizeof(ULONG), sizeof(ULONG));
    if (user_pids) {
        ProbeForWrite(user_pids, sizeof(ULONG) * count, sizeof(ULONG));
    }

    status = Process_Enumerate(boxname, all_sessions, session_id,
                               user_pids, &count);
    if (! NT_SUCCESS(status))
        return status;

    *user_count = count;

    return status;
}
