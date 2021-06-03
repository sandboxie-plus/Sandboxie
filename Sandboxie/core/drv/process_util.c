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
// Process Management:  Various Utilities
//---------------------------------------------------------------------------


#include "process.h"
#include "conf.h"
#include "file.h"
#include "token.h"
#include "api.h"
#include "obj.h"
#include "common/pattern.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN Process_MatchImageGroup(
    BOX *box, const WCHAR *group, ULONG group_len, const WCHAR *test_str,
    ULONG depth);

static BOOLEAN Process_AddPath_2(
    PROCESS *proc, LIST *list, const WCHAR *value, const WCHAR *setting_name,
    BOOLEAN AddFirst, BOOLEAN AddStar,
    BOOLEAN RemoveBackslashes, BOOLEAN CheckReparse);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static const WCHAR *Process_Open   = L"Open";
static const WCHAR *Process_Read   = L"Read";
static const WCHAR *Process_Write  = L"Write";
static const WCHAR *Process_Closed = L"Closed";


//---------------------------------------------------------------------------
// Process_IsSameBox
//---------------------------------------------------------------------------


_FX BOOLEAN Process_IsSameBox(
    PROCESS *proc, PROCESS *proc2, ULONG_PTR proc2_pid)
{
    KIRQL irql;
    BOOLEAN ok;
    BOOLEAN locked;

    if (proc2)
        locked = FALSE;
    else {
        proc2 = Process_Find((HANDLE)(ULONG_PTR)proc2_pid, &irql);
        locked = TRUE;
    }

    if (proc2 == proc) {

        //
        // write access is permitted into the same process
        //

        ok = TRUE;

    } else if (proc2 && (! proc2->terminated) && (! proc2->untouchable)) {

        //
        // write access is only permitted within the same sandbox
        // and same session
        //

        ok = (proc->box->session_id == proc2->box->session_id) &&
             (proc->box->name_len == proc2->box->name_len)
          && (_wcsicmp(proc->box->name, proc2->box->name) == 0);

    } else {

        //
        // deny write access
        //

        ok = FALSE;
    }

    if (locked) {

        ExReleaseResourceLite(Process_ListLock);
        KeLowerIrql(irql);
    }

    return ok;
}


//---------------------------------------------------------------------------
// Process_CheckTooManyBoxes
//---------------------------------------------------------------------------


_FX BOOLEAN Process_CheckTooManyBoxes(const BOX *box)
{
    PROCESS *proc;
    BOOLEAN ok;
    KIRQL irql;

    // if the user account of the caller has an active sandbox,
    // then a new process must be started in that same sandbox.
    // note that we intentionally don't look at logon sessions here.

    ok = TRUE;

    KeRaiseIrql(APC_LEVEL, &irql);
    ExAcquireResourceSharedLite(Process_ListLock, TRUE);

    proc = List_Head(&Process_List);
    while (proc) {
        if (! proc->terminated) {
            if (wcscmp(proc->box->sid, box->sid) == 0) {
                if (_wcsicmp(proc->box->name, box->name) != 0) {
                    ok = FALSE;
                    break;
                }
            }
        }
        proc = (PROCESS *)List_Next(proc);
    }

    ExReleaseResourceLite(Process_ListLock);
    KeLowerIrql(irql);

    return (! ok);
}


//---------------------------------------------------------------------------
// Process_MatchImage
//---------------------------------------------------------------------------


_FX BOOLEAN Process_MatchImage(
    BOX *box, const WCHAR *pat_str, ULONG pat_len, const WCHAR *test_str,
    ULONG depth)
{
    PATTERN *pat;
    WCHAR *tmp, *expnd;
    ULONG tmp_len;
    BOOLEAN ok;

    //
    // if pat_len was specified, we should create the match pattern
    // using only the first pat_len characters of pat_str
    //

    if (pat_len) {

        tmp_len = (pat_len + 1) * sizeof(WCHAR);
        tmp = Mem_Alloc(box->expand_args->pool, tmp_len);
        if (! tmp)
            return FALSE;

        wcsncpy(tmp, pat_str, pat_len);
        tmp[pat_len] = L'\0';

        expnd = Conf_Expand(box->expand_args, tmp, NULL);

        Mem_Free(tmp, tmp_len);

    } else {

        expnd = Conf_Expand(box->expand_args, pat_str, NULL);
    }

    if (! expnd)
        return FALSE;

    pat = Pattern_Create(box->expand_args->pool, expnd, TRUE);

    Mem_FreeString(expnd);

    if (! pat)
        return FALSE;

    //
    //
    //

    if (*pat_str == L'<') {

        Conf_AdjustUseCount(TRUE);

        ok = Process_MatchImageGroup(
                box, Pattern_Source(pat), 0, test_str, depth + 1);

        Conf_AdjustUseCount(FALSE);

        Pattern_Free(pat);

        return ok;
    }

    //
    // create a lower-case copy of test_str
    //

    ok = FALSE;

    tmp_len = (wcslen(test_str) + 1) * sizeof(WCHAR);
    tmp = Mem_Alloc(box->expand_args->pool, tmp_len);
    if (tmp) {

        memcpy(tmp, test_str, tmp_len);
        _wcslwr(tmp);

        ok = Pattern_Match(pat, tmp, wcslen(tmp));

        Mem_Free(tmp, tmp_len);
    }

    Pattern_Free(pat);

    return ok;
}


//---------------------------------------------------------------------------
// Process_MatchImageGroup
//---------------------------------------------------------------------------


_FX BOOLEAN Process_MatchImageGroup(
    BOX *box, const WCHAR *group, ULONG group_len, const WCHAR *test_str,
    ULONG depth)
{
    ULONG index;
    BOOLEAN match = FALSE;

    if (! group_len)
        group_len = wcslen(group);

    Conf_AdjustUseCount(TRUE);

    for (index = 0; (! match); ++index) {

        //
        // get next process group setting, compare to passed group name.
        // if the setting is <passed_group_name>= then we accept it.
        //

        ULONG value_len;
        const WCHAR *value = Conf_Get(box->name, L"ProcessGroup", index);
        if (! value)
            break;

        value_len = wcslen(value);
        if (value_len <= group_len + 1)
            continue;
        if (_wcsnicmp(value, group, group_len) != 0)
            continue;

        value += group_len;
        if (*value != L',')
            continue;
        ++value;

        //
        // value now points at the comma-separated
        // list of processes in this process group
        //

        while (*value) {
            WCHAR *ptr = wcschr(value, L',');
            if (ptr)
                value_len = (ULONG)(ULONG_PTR)(ptr - value);
            else
                value_len = wcslen(value);

            if (value_len) {
                if (*value != L'<') {
                    match = Process_MatchImage(
                            box, value, value_len, test_str, depth + 1);
                } else if (depth < 6) {
                    match = Process_MatchImageGroup(
                            box, value, value_len, test_str, depth + 1);
                }
                if (match)
                    break;
            }

            value += value_len;
            while (*value == L',')
                ++value;
        }
    }

    Conf_AdjustUseCount(FALSE);

    return match;
}


//---------------------------------------------------------------------------
// Process_GetPaths
//---------------------------------------------------------------------------


_FX BOOLEAN Process_GetPaths(
    PROCESS *proc, LIST *list, const WCHAR *setting_name, BOOLEAN AddStar)
{
    ULONG index;
    const WCHAR *value;
    BOOLEAN ok = TRUE;

    BOOLEAN closed = (_wcsnicmp(setting_name, Process_Closed, 6) == 0);
    BOOLEAN closed_ipc = FALSE;
    if (closed)
        closed_ipc = (_wcsnicmp(setting_name + 6, L"Ipc", 3) == 0);

    Conf_AdjustUseCount(TRUE);

    for (index = 0; ; ++index) {

        //
        // get next configuration setting for this path list
        //

        value = Conf_Get(proc->box->name, setting_name, index);
        if (! value)
            break;

        if (closed && (*value == L'!')) {

            if (closed_ipc && proc->image_sbie)
                continue;

            if (proc->image_from_box) {

                value = wcschr(value, L',');
                if (! value)
                    continue;
                ++value;
            }
        }

        if (! Process_AddPath(
                        proc, list, setting_name, FALSE, value, AddStar)) {
            ok = FALSE;
            break;
        }
    }

    Conf_AdjustUseCount(FALSE);

    return ok;
}


//---------------------------------------------------------------------------
// Process_GetPaths2
//---------------------------------------------------------------------------


_FX BOOLEAN Process_GetPaths2(
    PROCESS *proc, LIST *list, LIST *list2,
    const WCHAR *setting_name, BOOLEAN AddStar)
{
    LIST dummy_list;
    PATTERN *pat;
    const WCHAR *value;
    ULONG len;
    BOOLEAN is_open, is_closed;

    //
    // this function gets a list of settings, typically WriteXxxPath,
    // and compares it to an already-populated list of settings, typically
    // ClosedXxxPath, in order to discard duplicate settings.  the intent
    // is to keep the general rule that ClosedXxxPath settings override
    // any other settings, including WriteXxxPath settings
    //

    List_Init(&dummy_list);
    if (! Process_GetPaths(proc, &dummy_list, setting_name, AddStar))
        return FALSE;

    while (1) {

        pat = List_Head(&dummy_list);
        if (! pat)
            break;

        //
        // get a setting from the list of potential settings,
        // and discard suffix wildcards
        //

        value = Pattern_Source(pat);
        for (len = wcslen(value); len && value[len - 1] == L'*'; --len)
            ;

        if (! len)
            is_closed = TRUE;
        else {

            Process_MatchPath(proc->pool, value, len,
                              NULL, list2, &is_open, &is_closed);
        }

        List_Remove(&dummy_list, pat);

        if (is_closed)
            Pattern_Free(pat);
        else
            List_Insert_After(list, NULL, pat);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Process_AddPath
//---------------------------------------------------------------------------


_FX BOOLEAN Process_AddPath(
    PROCESS *proc, LIST *list, const WCHAR *setting_name,
    BOOLEAN AddFirst, const WCHAR *value, BOOLEAN AddStar)
{
    WCHAR *tmp;
    ULONG len;
    BOOLEAN RemoveBackslashes = FALSE;
    BOOLEAN CheckReparse = FALSE;
    BOOLEAN ok;

    //
    // if this is a file/pipe/key setting, remove duplicate backslashes
    // if this is a file setting, also check the path for reparse points
    //

    if (setting_name) {

        const WCHAR *setting_name_ptr = setting_name;
        if (_wcsnicmp(setting_name, Process_Closed, 6) == 0)
            setting_name_ptr = setting_name + 6;
        else if (_wcsnicmp(setting_name, Process_Write, 5) == 0)
            setting_name_ptr = setting_name + 5;
        else if (_wcsnicmp(setting_name, Process_Read, 4) == 0 ||
                 _wcsnicmp(setting_name, Process_Open, 4) == 0)
            setting_name_ptr = setting_name + 4;
        else
            setting_name_ptr = NULL;

        if (setting_name_ptr) {

            if (_wcsnicmp(setting_name_ptr, L"Key", 3) == 0)
                RemoveBackslashes = TRUE;

            else if (_wcsnicmp(setting_name_ptr, L"File", 4) == 0
                  || _wcsnicmp(setting_name_ptr, L"Pipe", 4) == 0) {

                RemoveBackslashes = TRUE;
                CheckReparse = TRUE;
            }
        }
    }

    //
    // if the setting indicates an image name followed by a comma,
    // then match the image name against the executing process.
    //

    tmp = wcschr(value, L',');
    if (tmp) {

        BOOLEAN inv, match;

        //
        // exclamation marks negates the matching
        //

        if (*value == L'!') {
            inv = TRUE;
            ++value;
        } else
            inv = FALSE;

        len = (ULONG)(tmp - value);
        if (len) {
            match = Process_MatchImage(
                                proc->box, value, len, proc->image_name, 1);
            if (inv)
                match = ! match;
            if (! match)
                return TRUE;
        }

        value = tmp + 1;
    }

    if (! *value)
        return TRUE;

    //
    // image name matches, or was not specified.  next, expand
    // the configuration path setting.
    //
    // note that if we're removing backslashes (i.e. for file/pipe/key)
    // and the setting value begins with a pipe character, then we
    // do not append a suffix wildcard character
    //

    if (RemoveBackslashes && *value == L'|') {
        ++value;
        AddStar = FALSE;
    }

    //
    // add the value as requested
    //

    ok = Process_AddPath_2(proc, list, value, setting_name,
               AddFirst, AddStar, RemoveBackslashes, CheckReparse);

    //
    // if this is a file setting (CheckReparse == TRUE) and starts with
    // *: or ?: then manually replace with each of the 26 possible drives
    //

    if (ok && CheckReparse && (value[0] == L'?' || value[0] == L'*')
                           && (value[1] == L':')) {

        tmp = Mem_AllocString(proc->pool, value);
        if (! tmp)
            return FALSE;
        for (len = L'A'; (len <= L'Z') && ok; ++len) {
            *tmp = (WCHAR)len;
            ok = Process_AddPath_2(proc, list, tmp, setting_name,
                   AddFirst, AddStar, RemoveBackslashes, CheckReparse);
        }
        Mem_FreeString(tmp);
    }

    return ok;
}


//---------------------------------------------------------------------------
// Process_AddPath_2
//---------------------------------------------------------------------------


_FX BOOLEAN Process_AddPath_2(
    PROCESS *proc, LIST *list, const WCHAR *value, const WCHAR *setting_name,
    BOOLEAN AddFirst, BOOLEAN AddStar,
    BOOLEAN RemoveBackslashes, BOOLEAN CheckReparse)
{
    PATTERN *pat;
    WCHAR *expand, *tmp;
    ULONG len;

    //
    // expand any variables in the value
    //

    expand = Conf_Expand(proc->box->expand_args, value, setting_name);
    if (! expand)
        return FALSE;

    //
    // duplicate the expanded string as a temp string, in case we
    // need to add a star at the end
    //

    len = (wcslen(expand) + 1) * sizeof(WCHAR);
    if (AddStar) {
        if (wcschr(expand, L'*') == NULL)
            len += sizeof(WCHAR);
        else
            AddStar = FALSE;
    }

    tmp = Mem_Alloc(proc->pool, len);
    if (! tmp) {
        Mem_FreeString(expand);
        return FALSE;
    }

    //
    // copy the expanded path string into the temporary string
    // optionally, removing backslashes
    // optionally, adding a star at the end
    //

    if (RemoveBackslashes) {

        WCHAR *src_ptr = expand;
        WCHAR *dst_ptr = tmp;
        while (*src_ptr) {
            if (src_ptr[0] == L'\\' && src_ptr[1] == L'\\') {
                ++src_ptr;
                continue;
            }
            *dst_ptr = *src_ptr;
            ++src_ptr;
            ++dst_ptr;
        }
        *dst_ptr = L'\0';

    } else
        wcscpy(tmp, expand);

    if (AddStar)
        wcscat(tmp, L"*");

    //
    // check for reparse points
    //

    if (CheckReparse) {

        WCHAR *tmp2 = File_TranslateReparsePoints(tmp, proc->pool);
        if (tmp2) {
            Mem_FreeString(tmp);
            tmp = tmp2;
        }
    }

    //
    // add the pattern
    //

    pat = Pattern_Create(proc->pool, tmp, TRUE);
    if (pat) {

        if (AddFirst)
            List_Insert_Before(list, NULL, pat);
        else
            List_Insert_After(list, NULL, pat);
    }

    Mem_FreeString(tmp);
    Mem_FreeString(expand);

    if (! pat)
        return FALSE;

    return TRUE;
}


//---------------------------------------------------------------------------
// Process_MatchPath
//---------------------------------------------------------------------------


_FX const WCHAR *Process_MatchPath(
    POOL *pool, const WCHAR *path, ULONG path_len,
    LIST *open_list, LIST *closed_list,
    BOOLEAN *is_open, BOOLEAN *is_closed)
{
    PATTERN *pat;
    WCHAR *path_lwr;
    ULONG path_lwr_len;
    const WCHAR *patsrc = NULL;

    *is_open = FALSE;
    *is_closed = FALSE;

    //
    // scan paths list.  if the path to match does not already end with
    // a backslash character, we will check it twice, second time with
    // a suffixing backslash.  this will make sure we match C:\X even
    // even when {Open,Closed}XxxPath=C:\X\ (with a backslash suffix)
    //

    path_lwr_len = (path_len + 4) * sizeof(WCHAR);
    path_lwr = Mem_Alloc(pool, path_lwr_len);
    if (! path_lwr)
        return NULL;

    wmemcpy(path_lwr, path, path_len);
    path_lwr[path_len]     = L'\0';
    path_len = wcslen(path_lwr);
    if (! path_len) {
        Mem_Free(path_lwr, path_lwr_len);
        return NULL;
    }
    path_lwr[path_len]     = L'\0';
    path_lwr[path_len + 1] = L'\0';
    _wcslwr(path_lwr);

    if (closed_list) {

        pat = List_Head(closed_list);
        while (pat) {

            if (Pattern_Match(pat, path_lwr, path_len)) {

                *is_closed = TRUE;
                patsrc = Pattern_Source(pat);
                break;
            }

            if (path_lwr[path_len - 1] != L'\\') {
                path_lwr[path_len] = L'\\';
                if (Pattern_Match(pat, path_lwr, path_len + 1)) {

                    path_lwr[path_len] = L'\0';
                    *is_closed = TRUE;
                    patsrc = Pattern_Source(pat);
                    break;
                }
                path_lwr[path_len] = L'\0';
            }

            pat = List_Next(pat);
        }
    }

    if (open_list && (! *is_closed)) {

        pat = List_Head(open_list);
        while (pat) {

            if (Pattern_Match(pat, path_lwr, path_len)) {

                *is_open = TRUE;
                patsrc = Pattern_Source(pat);
                break;
            }

            if (path_lwr[path_len - 1] != L'\\') {
                path_lwr[path_len] = L'\\';
                if (Pattern_Match(pat, path_lwr, path_len + 1)) {

                    path_lwr[path_len] = L'\0';
                    *is_open = TRUE;
                    patsrc = Pattern_Source(pat);
                    break;
                }
                path_lwr[path_len] = L'\0';
            }

            pat = List_Next(pat);
        }
    }

    Mem_Free(path_lwr, path_lwr_len);
    return patsrc;
}


//---------------------------------------------------------------------------
// Process_GetProcessName
//---------------------------------------------------------------------------


_FX void Process_GetProcessName(
    POOL *pool, ULONG_PTR idProcess,
    void **out_buf, ULONG *out_len, WCHAR **out_ptr)
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    CLIENT_ID cid;
    HANDLE handle;
    ULONG len;

    *out_buf = NULL;
    *out_len = 0;
    *out_ptr = NULL;

    if (! idProcess)
        return;

    InitializeObjectAttributes(&objattrs,
        NULL, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
    cid.UniqueProcess = (HANDLE)idProcess;
    cid.UniqueThread = 0;

    status = ZwOpenProcess(
        &handle, PROCESS_QUERY_INFORMATION, &objattrs, &cid);

    if (! NT_SUCCESS(status))
        return;

    status = ZwQueryInformationProcess(
                        handle, ProcessImageFileName, NULL, 0, &len);

    if (status == STATUS_INFO_LENGTH_MISMATCH) {

        ULONG uni_len = len + 8 + 8;
        UNICODE_STRING *uni = Mem_Alloc(pool, uni_len);
        if (uni) {

            uni->Buffer = NULL;

            status = ZwQueryInformationProcess(
                        handle, ProcessImageFileName, uni, len + 8, &len);

            if (NT_SUCCESS(status) && uni->Buffer) {

                WCHAR *ptr;
                uni->Buffer[uni->Length / sizeof(WCHAR)] = L'\0';
                if (! uni->Buffer[0]) {
                    uni->Buffer[0] = L'?';
                    uni->Buffer[1] = L'\0';
                }
                ptr = wcsrchr(uni->Buffer, L'\\');
                if (ptr) {
                    ++ptr;
                    if (! *ptr)
                        ptr = uni->Buffer;
                } else
                    ptr = uni->Buffer;
                *out_buf = uni;
                *out_len = uni_len;
                *out_ptr = ptr;

            } else
                Mem_Free(uni, uni_len);
        }
    }

    ZwClose(handle);
}


//---------------------------------------------------------------------------
// Process_CheckProcessName
//---------------------------------------------------------------------------


_FX NTSTATUS Process_CheckProcessName(
    PROCESS *proc, LIST *open_paths, ULONG_PTR idProcess,
    const WCHAR **pSetting)
{
    NTSTATUS status;
    PATTERN *pat;
    void *nbuf;
    ULONG nlen;
    WCHAR *nptr;

    status = STATUS_ACCESS_DENIED;

    if (pSetting)
        *pSetting = NULL;

    if (! idProcess)
        return status;

    nbuf = NULL;
    nlen = 0;
    nptr = NULL;

    //
    // Scan settings list for "$:ProcessName"
    //

    pat = List_Head(open_paths);
    while (pat) {

        const WCHAR *src = Pattern_Source(pat);
        pat = List_Next(pat);
        if (wcslen(src) > 3 && src[0] == L'$' && src[1] == L':') {

            if (! nptr) {
                Process_GetProcessName(
                    proc->pool, idProcess, &nbuf, &nlen, &nptr);
                if (! nptr)
                    break;
            }
            if (_wcsicmp(nptr, src + 2) == 0) {
                status = STATUS_SUCCESS;
                if (pSetting)
                    *pSetting = src;
                break;
            }
        }
    }

    if (nbuf)
        Mem_Free(nbuf, nlen);

    return status;
}


//---------------------------------------------------------------------------
// Process_GetSidStringAndSessionId
//---------------------------------------------------------------------------


_FX NTSTATUS Process_GetSidStringAndSessionId(
    HANDLE ProcessHandle, HANDLE ProcessId,
    UNICODE_STRING *SidString, ULONG *SessionId)
{
    NTSTATUS status;
    PEPROCESS ProcessObject = NULL;
    PACCESS_TOKEN TokenObject;

    if (ProcessHandle == NtCurrentProcess()) {

        ProcessObject = PsGetCurrentProcess();
        ObReferenceObject(ProcessObject);
        status = STATUS_SUCCESS;

    } else if (ProcessHandle) {

        const KPROCESSOR_MODE AccessMode =
            ((ProcessHandle == NtCurrentProcess()) ? KernelMode : UserMode);

        status = ObReferenceObjectByHandle(ProcessHandle, 0, *PsProcessType,
                                           AccessMode, &ProcessObject, NULL);

    } else if (ProcessId) {

        status = PsLookupProcessByProcessId(ProcessId, &ProcessObject);

    } else {

        status = STATUS_INVALID_PARAMETER;
    }

    if (NT_SUCCESS(status)) {

        *SessionId = PsGetProcessSessionId(ProcessObject);

        TokenObject = PsReferencePrimaryToken(ProcessObject);
        status = Token_QuerySidString(TokenObject, SidString);
        PsDereferencePrimaryToken(TokenObject);

        ObDereferenceObject(ProcessObject);
    }

    if (! NT_SUCCESS(status)) {

        SidString->Buffer = NULL;
        *SessionId = -1;
    }

    return status;
}


//---------------------------------------------------------------------------
// Process_LogMessage
//---------------------------------------------------------------------------


_FX void Process_LogMessage(PROCESS *proc, ULONG msgid)
{
    BOX *box = proc->box;
    ULONG len = proc->image_name_len + box->name_len + 8 * sizeof(WCHAR);
    WCHAR *text = Mem_Alloc(proc->pool, len);
    RtlStringCbPrintfW(text, len, L"%s [%s]", proc->image_name, box->name);
    if (proc->image_from_box)
        wcscat(text, L" *");
    Log_MsgP1(msgid, text, proc->pid);
    Mem_Free(text, len);
}


//---------------------------------------------------------------------------
// Process_TrackProcessLimit
//---------------------------------------------------------------------------


//_FX void Process_TrackProcessLimit(PROCESS *proc)
//{
//    ULONG v;
//    ULONG ProcessLimit1;
//    ULONG ProcessLimit2;
//
//    //
//    // get the process limits in this sandbox
//    //
//
//    ProcessLimit1 = 100;
//    ProcessLimit2 = 200;
//
//    v = Conf_Get_Number(proc->box->name, L"ProcessLimit1", 0, 0);
//    if (v >= 1 && v <= 999999)
//        ProcessLimit1 = v;
//
//    v = Conf_Get_Number(proc->box->name, L"ProcessLimit2", 0, 0);
//    if (v >= 1 && v <= 999999)
//        ProcessLimit2 = v;
//
//    if (ProcessLimit2 <= ProcessLimit1)
//        ProcessLimit2 = ProcessLimit1 + 1;
//
//    //
//    // count number of processes in this sandbox
//    //
//
//    Process_Enumerate(proc->box->name, FALSE, proc->box->session_id,
//                      NULL, &v);
//
//    if (v > ProcessLimit2) {
//
//        Process_SetTerminated(proc, 4);
//
//    } else if (v > ProcessLimit1) {
//
//        LARGE_INTEGER time;
//
//        time.QuadPart = -SECONDS(10);
//        KeDelayExecutionThread(KernelMode, FALSE, &time);
//    }
//}


//---------------------------------------------------------------------------
// Process_CancelProcess
//---------------------------------------------------------------------------


_FX void Process_CancelProcess(PROCESS *proc)
{
    SVC_PROCESS_MSG msg;

    ULONG len = wcslen(proc->image_name);
    const ULONG max_len = sizeof(msg.process_name) / sizeof(WCHAR) - 1;
    if (len > max_len)
        len = max_len;
    wmemcpy(msg.process_name, proc->image_name, len);
    msg.process_name[len] = L'\0';

    msg.process_id = (ULONG)(ULONG_PTR)proc->pid;
    msg.session_id = proc->box->session_id;
    msg.create_time = proc->create_time;
    msg.is_wow64 = FALSE;
    msg.add_to_job = FALSE;
    msg.reason = proc->reason;

    Api_SendServiceMessage(SVC_CANCEL_PROCESS, sizeof(msg), &msg);
}


//---------------------------------------------------------------------------
// Process_IsPcaJob
//---------------------------------------------------------------------------


_FX BOOLEAN Process_IsInPcaJob(HANDLE ProcessId)
{
    PEPROCESS ProcessObject;
    ULONG_PTR JobObject;
    OBJECT_NAME_INFORMATION *Name;
    ULONG NameLength;
    NTSTATUS status;
    BOOLEAN IsInPcaJob = FALSE;

    status = PsLookupProcessByProcessId(ProcessId, &ProcessObject);
    if (NT_SUCCESS(status)) {

        JobObject = PsGetProcessJob(ProcessObject);
        if (JobObject) {

            status = Obj_GetName(Driver_Pool, (void *)JobObject,
                                 &Name, &NameLength);
            if (NT_SUCCESS(status) && (Name != &Obj_Unnamed)) {

                if (Name->Name.Length == 60 * sizeof(WCHAR)
                        && 0 == _wcsnicmp(Name->Name.Buffer,
                                    L"\\BaseNamedObjects\\PCA_", 22)) {

                    IsInPcaJob = TRUE;
                }

                Mem_Free(Name, NameLength);

            } else if (NT_SUCCESS(status) && (Name == &Obj_Unnamed) &&
                            Driver_OsVersion >= DRIVER_WINDOWS_8) {
                //
                // on Windows 8 the PCA job is unnamed
                //

                IsInPcaJob = TRUE;
            }
        }

        ObDereferenceObject(ProcessObject);
    }

    return IsInPcaJob;
}
