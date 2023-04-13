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
// Process Server -- using PipeServer
//---------------------------------------------------------------------------

#include "stdafx.h"

#include <wtsapi32.h>
#include <userenv.h>
#include "ProcessServer.h"
#include "Processwire.h"
#include "DriverAssist.h"
#include "GuiServer.h"
#include "GuiWire.h"
#include "FileServer.h"
#include "misc.h"
#include "common/defines.h"
#include "common/my_version.h"
#include "core/dll/sbiedll.h"
#include "core/drv/api_defs.h"
#include <sddl.h>
#include "sbieiniserver.h"

#define SECONDS(n64)            (((LONGLONG)n64) * 10000000L)
#define MINUTES(n64)            (SECONDS(n64) * 60)


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


ProcessServer::ProcessServer(PipeServer *pipeServer)
{
    pipeServer->Register(MSGID_PROCESS, this, Handler);
}


//---------------------------------------------------------------------------
// Handler
//---------------------------------------------------------------------------


MSG_HEADER *ProcessServer::Handler(void *_this, MSG_HEADER *msg)
{
    ProcessServer *pThis = (ProcessServer *)_this;

    if (msg->msgid == MSGID_PROCESS_CHECK_INIT_COMPLETE)
        return pThis->CheckInitCompleteHandler();

    HANDLE idProcess = (HANDLE)(ULONG_PTR)PipeServer::GetCallerProcessId();

    if (msg->msgid == MSGID_PROCESS_KILL_ONE)
        return pThis->KillOneHandler(idProcess, msg);

    if (msg->msgid == MSGID_PROCESS_KILL_ALL)
        return pThis->KillAllHandler(idProcess, msg);

    if (msg->msgid == MSGID_PROCESS_SET_DEVICE_MAP)
        return pThis->SetDeviceMap(idProcess, msg);

    if (msg->msgid == MSGID_PROCESS_OPEN_DEVICE_MAP)
        return pThis->OpenDeviceMap(idProcess, msg);

    if (msg->msgid == MSGID_PROCESS_RUN_SANDBOXED)
        return pThis->RunSandboxedHandler(msg);

    if (msg->msgid == MSGID_PROCESS_RUN_UPDATER)
        return pThis->RunUpdaterHandler(msg);

    return NULL;
}


//---------------------------------------------------------------------------
// CheckInitCompleteHandler
//---------------------------------------------------------------------------


MSG_HEADER *ProcessServer::CheckInitCompleteHandler()
{
    ULONG status = STATUS_SUCCESS;
    if (! DriverAssist::IsDriverReady())
        status = STATUS_DEVICE_NOT_READY;
    return SHORT_REPLY(status);
}


//---------------------------------------------------------------------------
// KillProcess
//---------------------------------------------------------------------------


BOOL ProcessServer::KillProcess(ULONG ProcessId)
{
    ULONG LastError = 0;
    BOOL ok = FALSE;
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, ProcessId);
    if (! hProcess)
        LastError = GetLastError() * 10000;
    else {
        ok = TerminateProcess(hProcess, 1);
        if (! ok)
            LastError = GetLastError();
        CloseHandle(hProcess);
    }
    //WCHAR txt[512]; wsprintf(txt, L"Killing Process Id %d --> %d/%d\n", ProcessId, ok, LastError); OutputDebugString(txt);
    return ok;
}


//---------------------------------------------------------------------------
// KillOneHandler
//---------------------------------------------------------------------------


MSG_HEADER *ProcessServer::KillOneHandler(
    HANDLE CallerProcessId, MSG_HEADER *msg)
{
    ULONG TargetSessionId;
    WCHAR TargetBoxName[BOXNAME_COUNT];
    ULONG CallerSessionId;
    WCHAR CallerBoxName[BOXNAME_COUNT];
    NTSTATUS status;

    //
    // parse request packet
    //

    PROCESS_KILL_ONE_REQ *req = (PROCESS_KILL_ONE_REQ *)msg;
    if (req->h.length < sizeof(PROCESS_KILL_ONE_REQ))
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);

    //
    // get session id and box name for target process
    //

    status = SbieApi_QueryProcess((HANDLE)(ULONG_PTR)req->pid, TargetBoxName,
                                  NULL, NULL, &TargetSessionId);

    if (status != STATUS_SUCCESS)
        return SHORT_REPLY(status);

    //
    // get session id for caller.  if sandboxed, get also box name
    //

    status = SbieApi_QueryProcess(CallerProcessId, CallerBoxName,
                                  NULL, NULL, &CallerSessionId);

    if (status == STATUS_INVALID_CID) {

        CallerBoxName[0] = L'\0';

        CallerSessionId = PipeServer::GetCallerSessionId();

    } else if (status != STATUS_SUCCESS)
        return SHORT_REPLY(status);

    //
    // match session id and box name
    //

    if (CallerSessionId != TargetSessionId && !PipeServer::IsCallerAdmin())
        return SHORT_REPLY(STATUS_ACCESS_DENIED);

    if (CallerBoxName[0] && _wcsicmp(CallerBoxName, TargetBoxName) != 0)
        return SHORT_REPLY(STATUS_ACCESS_DENIED);

    //
    // kill target process
    //

    if (KillProcess(req->pid))
        status = STATUS_SUCCESS;
    else
        status = STATUS_UNSUCCESSFUL;

    return SHORT_REPLY(status);
}


//---------------------------------------------------------------------------
// KillAllHandler
//---------------------------------------------------------------------------


MSG_HEADER *ProcessServer::KillAllHandler(
    HANDLE CallerProcessId, MSG_HEADER *msg)
{
    ULONG TargetSessionId;
    WCHAR TargetBoxName[BOXNAME_COUNT];
    ULONG CallerSessionId;
    WCHAR CallerBoxName[BOXNAME_COUNT];
    BOOLEAN TerminateJob;
    NTSTATUS status;

    //
    // parse request packet
    //

    PROCESS_KILL_ALL_REQ *req = (PROCESS_KILL_ALL_REQ *)msg;
    if (req->h.length < sizeof(PROCESS_KILL_ALL_REQ))
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);

    TargetSessionId = req->session_id;
    wcscpy(TargetBoxName, req->boxname);
    if (! TargetBoxName[0])
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);

    //
    // get session id for caller.  if sandboxed, get also box name
    //

    status = SbieApi_QueryProcess(CallerProcessId, CallerBoxName,
                                  NULL, NULL, &CallerSessionId);

    if (status == STATUS_INVALID_CID) {

        CallerBoxName[0] = L'\0';

        CallerSessionId = PipeServer::GetCallerSessionId();

    } else if (status != STATUS_SUCCESS)
        return SHORT_REPLY(status);

    if (status != STATUS_INVALID_CID) // if this is true the caller is boxed, should be rpcss
        TerminateJob = FALSE; // if rpcss requests box termination, don't use the job method, fix-me: we get some stuck request in the queue
    else
        TerminateJob = !SbieApi_QueryConfBool(TargetBoxName, L"NoAddProcessToJob", FALSE);

    //
    // match session id and box name
    //

    if (TargetSessionId == -1)
        TargetSessionId = CallerSessionId;
    else if (CallerSessionId != TargetSessionId && !PipeServer::IsCallerAdmin())
        return SHORT_REPLY(STATUS_ACCESS_DENIED);

    if (CallerBoxName[0] && _wcsicmp(CallerBoxName, TargetBoxName) != 0)
        return SHORT_REPLY(STATUS_ACCESS_DENIED);

    //
    // kill target processes
    //

    status = KillAllHelper(TargetBoxName, TargetSessionId, TerminateJob);

    return SHORT_REPLY(status);
}


//---------------------------------------------------------------------------
// KillAllHelper
//---------------------------------------------------------------------------


NTSTATUS ProcessServer::KillAllHelper(const WCHAR *BoxName, ULONG SessionId, BOOLEAN TerminateJob)
{
    NTSTATUS status;
    ULONG retries, i;
    const ULONG pids_len = 512;
    ULONG pids[pids_len];
    ULONG count;

    if (TerminateJob) {

        //
        // try killing the entire job in one go first
        //

        GUI_KILL_JOB_REQ data;
        data.msgid = GUI_KILL_JOB;
        if (BoxName) wcscpy(data.boxname, BoxName);
        else data.boxname[0] = L'\0';

        GuiServer::GetInstance()->SendMessageToSlave(SessionId, &data, sizeof(data));

        //
        // as fallback and for the case where jobs are not used run the manual termination
        // 
    }

    
    for (retries = 0; retries < 10; ) {

        count = pids_len;
        status = SbieApi_EnumProcessEx(BoxName, FALSE, SessionId, pids, &count);
        if (status != STATUS_SUCCESS)
            break;
        if (count == 0)
            break;

        if (count < pids_len)
            retries++;

        if (retries) {
            if (retries >= 10 - 1) {
                status = STATUS_UNSUCCESSFUL;
                break;
            }
            Sleep(100);
        }

        for (i = 0; i <= count; ++i)
            KillProcess(pids[i]);
    }

    return status;
}


//---------------------------------------------------------------------------
// SetDeviceMap
//---------------------------------------------------------------------------


MSG_HEADER *ProcessServer::SetDeviceMap(
    HANDLE CallerProcessId, MSG_HEADER *msg)
{
    //
    // 32-bit process on 64-bit Windows can't set its own device map
    // due to an error in the wow64 api layer, so we offer a request
    // to set the device map for it.  see also core/dll/file_init.c
    //

    NTSTATUS status = STATUS_SUCCESS;

    PROCESS_SET_DEVICE_MAP_REQ *req = (PROCESS_SET_DEVICE_MAP_REQ *)msg;
    if (req->h.length < sizeof(PROCESS_SET_DEVICE_MAP_REQ))
        status = STATUS_INVALID_PARAMETER;

    else if (! SbieApi_QueryProcessInfo(
                                (HANDLE)(ULONG_PTR)CallerProcessId, 0))
        status = STATUS_ACCESS_DENIED;

    else {

        HANDLE CallerProcessHandle = OpenProcess(
                        PROCESS_SET_INFORMATION | PROCESS_DUP_HANDLE,
                        FALSE, (ULONG)(ULONG_PTR)CallerProcessId);
        if (! CallerProcessHandle)
            status = RtlNtStatusToDosError(GetLastError());
        else {

            PROCESS_DEVICEMAP_INFORMATION info;
            BOOL ok = DuplicateHandle(
                CallerProcessHandle, (HANDLE)(ULONG_PTR)req->DirectoryHandle,
                NtCurrentProcess(), &info.Set.DirectoryHandle,
                DIRECTORY_TRAVERSE, FALSE, 0);
            if (! ok)
                status = RtlNtStatusToDosError(GetLastError());
            else {

                status = NtSetInformationProcess(
                            CallerProcessHandle, ProcessDeviceMap,
                            &info, sizeof(info.Set));

                NtClose(info.Set.DirectoryHandle);
            }

            NtClose(CallerProcessHandle);
        }
    }

    return SHORT_REPLY(status);
}


//---------------------------------------------------------------------------
// OpenDeviceMap
//---------------------------------------------------------------------------


MSG_HEADER *ProcessServer::OpenDeviceMap(
    HANDLE CallerProcessId, MSG_HEADER *msg)
{
    //
    // the process may not be able to open the device map it needs.
    // one possible scenario is logging into an Administrator account
    // and then starting a process in a sandbox with Drop Rights.
    // this helper service can open the device map for the caller.
    //

    NTSTATUS status = STATUS_SUCCESS;

    PROCESS_OPEN_DEVICE_MAP_REQ *req = (PROCESS_OPEN_DEVICE_MAP_REQ *)msg;
    if (req->h.length < sizeof(PROCESS_OPEN_DEVICE_MAP_REQ))
        status = STATUS_INVALID_PARAMETER;

    else if (! SbieApi_QueryProcessInfo(
                                (HANDLE)(ULONG_PTR)CallerProcessId, 0))
        status = STATUS_ACCESS_DENIED;

    else {

        HANDLE LocalDirectoryHandle;
        UNICODE_STRING objname;
        OBJECT_ATTRIBUTES objattrs;

        RtlInitUnicodeString(&objname, req->DirectoryName);
        InitializeObjectAttributes(
            &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

        status = NtOpenDirectoryObject(
                    &LocalDirectoryHandle, DIRECTORY_TRAVERSE, &objattrs);

        if (NT_SUCCESS(status)) {

            HANDLE CallerProcessHandle = OpenProcess(PROCESS_DUP_HANDLE
                            | PROCESS_VM_OPERATION | PROCESS_VM_WRITE,
                            FALSE, (ULONG)(ULONG_PTR)CallerProcessId);
            if (! CallerProcessHandle)
                status = RtlNtStatusToDosError(GetLastError());
            else {

                HANDLE RemoteDirectoryHandle;
                BOOL ok = DuplicateHandle(
                    NtCurrentProcess(), LocalDirectoryHandle,
                    CallerProcessHandle, (HANDLE *)&RemoteDirectoryHandle,
                    DIRECTORY_TRAVERSE, FALSE, 0);
                if (! ok)
                    status = RtlNtStatusToDosError(GetLastError());
                else {

                    ok = WriteProcessMemory(
                        CallerProcessHandle, (void *)req->DirectoryHandlePtr,
                        &RemoteDirectoryHandle, sizeof(HANDLE), NULL);
                    if (! ok)
                        status = RtlNtStatusToDosError(GetLastError());
                }

                NtClose(CallerProcessHandle);
            }

            NtClose(LocalDirectoryHandle);
        }
    }

    return SHORT_REPLY(status);
}


//---------------------------------------------------------------------------
// RunSandboxedHandler
//---------------------------------------------------------------------------


MSG_HEADER *ProcessServer::RunSandboxedHandler(MSG_HEADER *msg)
{
    //
    // validate request structure
    //

    ULONG err, lvl;

    PROCESS_RUN_SANDBOXED_REQ *req = (PROCESS_RUN_SANDBOXED_REQ *)msg;
    if (req->h.length < sizeof(PROCESS_RUN_SANDBOXED_REQ))
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);

    WCHAR *cmd = RunSandboxedCopyString(&req->h, req->cmd_ofs, req->cmd_len);
    WCHAR *dir = RunSandboxedCopyString(&req->h, req->dir_ofs, req->dir_len);
    WCHAR *env = RunSandboxedCopyString(&req->h, req->env_ofs, req->env_len);

    PROCESS_INFORMATION piReply;
    memzero(&piReply, sizeof(PROCESS_INFORMATION));

    /*if (env && req->devmap[0]) {

        WCHAR *env2 = RunSandboxedCopyDeviceMap(env, req->devmap[0]);
        HeapFree(GetProcessHeap(), 0, env);
        env = env2;
    }*/

    //
    // execute request, we start by opening the calling process
    //

    if (cmd && dir && env) {

        ULONG CallerPid = PipeServer::GetCallerProcessId();

        HANDLE CallerProcessHandle = OpenProcess(
           PROCESS_QUERY_INFORMATION | PROCESS_DUP_HANDLE, FALSE, CallerPid);

        if (CallerProcessHandle) {

            //
            // if caller is sandboxed, its pid number determines the
            // BoxNameOrModelPid parameter for the API_START_PROCESS
            // call.  a caller outside the sandbox specifies a boxname
            //

            LONG_PTR BoxNameOrModelPid;
            bool CallerInSandbox;
            WCHAR boxname[BOXNAME_COUNT] = { 0 };
            WCHAR sid[96];
            ULONG session_id;
            BOOL FilterHandles = FALSE;

            if (SbieApi_QueryProcessInfo((HANDLE)(ULONG_PTR)CallerPid, 0)) {
                CallerInSandbox = true;
                SbieApi_QueryProcess((HANDLE)(ULONG_PTR)CallerPid, boxname, NULL, sid, &session_id);
                BoxNameOrModelPid = -(LONG_PTR)(LONG)CallerPid;
                if ((req->si_flags & 0x80000000) != 0) { // bsession0 - this is only allowed for unsandboxed processes
                    lvl = 0xFF;
                    err = ERROR_NOT_SUPPORTED;
                    goto end;
                }
            } else {
                CallerInSandbox = false;
                if (*req->boxname == L'-') {
                    int Pid = _wtoi(req->boxname + 1);
                    SbieApi_QueryProcess((HANDLE)(ULONG_PTR)Pid, boxname, NULL, sid, &session_id);
                    BoxNameOrModelPid = -Pid;
                } else {
                    BoxNameOrModelPid = (LONG_PTR)req->boxname;
                    wcscpy(boxname, req->boxname);
                }
            }

#ifndef DRV_BREAKOUT
            if (CallerInSandbox && wcscmp(req->boxname, L"*UNBOXED*") == 0 && *cmd == L'\"') {

                //ULONG flags = 0;
                //if (!NT_SUCCESS(SbieApi_Call(API_QUERY_DRIVER_INFO, 2, 0, (ULONG_PTR)&flags)) || (flags & SBIE_FEATURE_FLAG_CERTIFIED) == 0) {
                //    ULONG SessionId = PipeServer::GetCallerSessionId();
                //    SbieApi_LogEx(SessionId, 6004, L"%S", boxname);
                //    lvl = 0x66;
                //    err = ERROR_NOT_SUPPORTED;
                //    goto end;
                //} 

                WCHAR* lpApplicationName = cmd + 1;
                WCHAR* ptr = wcschr(lpApplicationName, L'\"');
                if (ptr) {
                    *ptr = L'\0'; // end cmd where lpApplicationName ends
                    WCHAR* lpProgram = wcsrchr(lpApplicationName, L'\\');
                    if (lpProgram) {

                        //
                        // check if the process/directory is configured for breakout
                        // if its a BreakoutProcess we must also test if the path is not in the sandbox itself
                        //

                        if ((SbieDll_CheckStringInList(lpProgram + 1, boxname, L"BreakoutProcess")
                            && IsHostPath((HANDLE)(ULONG_PTR)CallerPid, lpApplicationName))
                            || SbieDll_CheckPatternInList(lpApplicationName, (ULONG)(lpProgram - lpApplicationName), boxname, L"BreakoutFolder")) {

                            //
                            // this is a breakout process, it is allowed to leave the sandbox
                            //

                            BoxNameOrModelPid = 0;
                            FilterHandles = TRUE;

                            //
                            // check if it should end up in another box
                            //

                            WCHAR BoxName[BOXNAME_COUNT];
                            int index = -1;
                            while (1) {
                                index = SbieApi_EnumBoxesEx(index, BoxName, TRUE);
                                if (index == -1)
                                    break;
                                if (!NT_SUCCESS(SbieApi_Call(API_IS_BOX_ENABLED, 3, (ULONG_PTR)BoxName, (ULONG_PTR)sid, (ULONG_PTR)session_id)))
                                    continue;

                                if (SbieDll_CheckStringInList(lpProgram + 1, BoxName, L"ForceProcess")
                                    || SbieDll_CheckPatternInList(lpApplicationName, (ULONG)(lpProgram - lpApplicationName), BoxName, L"ForceFolder")) {

                                    //
                                    // check if the breakout process is supposed to end in the box it is trying to break out of
                                    // and deny the breakout in that case, to take the normal process creation route
                                    // 
                                    // this happens when a break out is configured globally
                                    //

                                    if (_wcsicmp(boxname, BoxName) == 0) {
                                        lvl = 0;
                                        err = ERROR_NOT_SUPPORTED;
                                        goto end;
                                    }

                                    //
                                    // set other box
                                    //

                                    BoxNameOrModelPid = (LONG_PTR)boxname;
                                    wcscpy(boxname, BoxName);
                                    break;
                                }
                            }
                        }
                    }
                    // restore cmd
                    *ptr = L'\"';
                }
            }
#endif

            HANDLE PrimaryTokenHandle = RunSandboxedGetToken(
                        CallerProcessHandle, CallerInSandbox, boxname, cmd);

            if (PrimaryTokenHandle) {

                //
                // copy STARTUPINFO parameters from caller
                //

                STARTUPINFO si;
                PROCESS_INFORMATION pi;

                memzero(&pi, sizeof(PROCESS_INFORMATION));
                memzero(&si, sizeof(STARTUPINFO));
                si.cb = sizeof(STARTUPINFO);
                si.dwFlags = req->si_flags;
                si.wShowWindow = (USHORT)req->si_show_window;

                //
                // notify the driver and start the new process, then
                // duplicate the handle into the caller process
                //

                if (RunSandboxedStartProcess(
                        PrimaryTokenHandle, BoxNameOrModelPid,
                        cmd, dir, env, &FilterHandles, req->creation_flags, &si, &pi)) {

                    if (RunSandboxedDupAndCloseHandles(
                            CallerProcessHandle, FilterHandles, req->creation_flags,
                            &pi, &piReply)) {

                        err = 0;
                        lvl = 0;

                    } else {

                        err = GetLastError();
                        lvl = 0x55;
                    }

                } else {

                    err = GetLastError();
                    lvl = 0x44;
                }

                CloseHandle(PrimaryTokenHandle);

            } else {

                err = GetLastError();
                lvl = 0x33;
            }

        end:
            CloseHandle(CallerProcessHandle);

        } else {

            err = GetLastError();
            lvl = 0x22;
        }

    } else {

        err = ERROR_INVALID_PARAMETER;
        lvl = 0x11;
    }

    //
    // finish
    //

    if (env)
        HeapFree(GetProcessHeap(), 0, env);
    if (dir)
        HeapFree(GetProcessHeap(), 0, dir);
    if (cmd)
        HeapFree(GetProcessHeap(), 0, cmd);

    if (lvl) {

        bool show_msg = true;
        if (lvl == 0x44 && (err == ERROR_COUNTER_TIMEOUT))
            show_msg = false;

        if (show_msg) {
            ULONG SessionId = PipeServer::GetCallerSessionId();
            SbieApi_LogEx(SessionId, 2337, L"[%02X / %d]", lvl, err);
        }
    }

    PROCESS_RUN_SANDBOXED_RPL *rpl = (PROCESS_RUN_SANDBOXED_RPL *)
                            LONG_REPLY(sizeof(PROCESS_RUN_SANDBOXED_RPL));
    if (rpl) {
        rpl->h.status    = err;
        rpl->hProcess    = (ULONG64)(ULONG_PTR)piReply.hProcess;
        rpl->hThread     = (ULONG64)(ULONG_PTR)piReply.hThread;
        rpl->dwProcessId = piReply.dwProcessId;
        rpl->dwThreadId  = piReply.dwThreadId;
    }
    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// RunSandboxedCopyString
//---------------------------------------------------------------------------


WCHAR *ProcessServer::RunSandboxedCopyString(
    MSG_HEADER *msg, ULONG ofs, ULONG len)
{
    len *= sizeof(WCHAR);

    if (    ofs         <= PIPE_MAX_DATA_LEN
        &&  len         <= PIPE_MAX_DATA_LEN
        &&  (ofs + len) <= msg->length) {

        WCHAR *buffer = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, len + 4);
        if (buffer) {

            if (len)
                memcpy(buffer, (UCHAR *)msg + ofs, len);
            buffer[len / sizeof(WCHAR)] = L'\0';

            return buffer;
        }
    }

    return NULL;
}


//---------------------------------------------------------------------------
// RunSandboxedCopyDeviceMap
//---------------------------------------------------------------------------


/*WCHAR *ProcessServer::RunSandboxedCopyDeviceMap(
    const WCHAR *env, const WCHAR *devmap)
{
    // see also core/dll/file_init.c
    const WCHAR *_ENV_VAR_DEV_MAP = L"00000000_" SBIE L"_DEVICE_MAP";

    ULONG env_len = 0;
    const WCHAR *env_ptr = env;
    while (*env_ptr) {
        env_len += wcslen(env_ptr) + 1;
        env_ptr += env_len;
    }
    ++env_len;

    ULONG env_var_name_len = wcslen(_ENV_VAR_DEV_MAP);
    ULONG env_var_data_len = wcslen(devmap);

    ULONG env2_len = env_var_name_len + env_var_data_len + 8 + env_len;
    WCHAR *env2 = HeapAlloc(GetProcessHeap(), 0, env2_len * sizeof(WCHAR));
    if (env2) {

        WCHAR *env2_ptr = env2;
        wmemcpy(env2_ptr, _ENV_VAR_DEV_MAP, env_var_name_len);
        env2_ptr += env_var_name_len;
        *env2_ptr = L'=';
    }

    return env2;
}*/


//---------------------------------------------------------------------------
// ProcessServer__RunRpcssAsSystem
//---------------------------------------------------------------------------


bool ProcessServer__RunRpcssAsSystem(const WCHAR* boxname)
{
    if (SbieApi_QueryConfBool(boxname, L"RunRpcssAsSystem", FALSE))
        return true;
    // OriginalToken BEGIN
    if (SbieApi_QueryConfBool(boxname, L"NoSecurityIsolation", FALSE) || SbieApi_QueryConfBool(boxname, L"OriginalToken", FALSE)) {
    // OriginalToken END
    
        //
        // if we run MSIServer as system we need to run the sandboxed Rpcss as system to or else it will fail
        //

        if (SbieApi_QueryConfBool(boxname, L"MsiInstallerExemptions", FALSE) || SbieApi_QueryConfBool(boxname, L"RunServicesAsSystem", FALSE))
            return TRUE;
    }
    return FALSE;
}


//---------------------------------------------------------------------------
// RunSandboxedGetToken
//---------------------------------------------------------------------------


HANDLE ProcessServer::RunSandboxedGetToken(
    HANDLE CallerProcessHandle, bool CallerInSandbox, const WCHAR *boxname, const WCHAR* cmd)
{
    const ULONG TOKEN_RIGHTS = TOKEN_QUERY          | TOKEN_DUPLICATE
                             | TOKEN_ADJUST_DEFAULT | TOKEN_ADJUST_SESSIONID
                             | TOKEN_ADJUST_GROUPS  | TOKEN_ASSIGN_PRIMARY;

    HANDLE OldTokenHandle = NULL;
    HANDLE NewTokenHandle;
    ULONG LastError;
    BOOL ok;
    bool ShouldAdjustSessionId = true;
    bool ShouldAdjustDacl = false;

    if (CallerInSandbox) {

        if ((wcscmp(cmd, L"*RPCSS*") == 0 /* || wcscmp(cmd, L"*DCOM*") == 0 */) 
          && ProcessServer__RunRpcssAsSystem(boxname)) {
            
            //
            // use our system token
            //

            ok = OpenProcessToken(
                        GetCurrentProcess(), TOKEN_RIGHTS, &OldTokenHandle);
            if (! ok)
                return NULL;

            ShouldAdjustDacl = true;

        }
        else
        // OriginalToken BEGIN
        if (!SbieApi_QueryConfBool(boxname, L"NoSecurityIsolation", FALSE) && !SbieApi_QueryConfBool(boxname, L"OriginalToken", FALSE))
        // OriginalToken END
        {
            //
            // use its thread token
            //

            HANDLE ThreadHandle = OpenThread(THREAD_QUERY_INFORMATION, FALSE,
                                            PipeServer::GetCallerThreadId());
            if (! ThreadHandle)
                return NULL;

            ok = OpenThreadToken(
                        ThreadHandle, TOKEN_RIGHTS, TRUE, &OldTokenHandle);
            LastError = GetLastError();

            CloseHandle(ThreadHandle);

            if (! ok) {
                SetLastError(LastError);
                return NULL;
            }

        }
    }
    else
    {
        typedef LONG (WINAPI *P_GetApplicationUserModelId)(
                HANDLE hProcess, UINT32 * applicationUserModelIdLength, PWSTR applicationUserModelId);

        static P_GetApplicationUserModelId pGetApplicationUserModelId = (P_GetApplicationUserModelId)-1;
        if ((UINT_PTR)pGetApplicationUserModelId == -1)
            pGetApplicationUserModelId = (P_GetApplicationUserModelId)GetProcAddress(_Kernel32, "GetApplicationUserModelId");
         
        if (pGetApplicationUserModelId) {
        
            //
            // when the calling application is a modern app, we can't use its token
            //

            UINT32 length = 0;
            LONG rc = pGetApplicationUserModelId(CallerProcessHandle, &length, NULL);
            if (rc != APPMODEL_ERROR_NO_APPLICATION)
            {
                //
                // use session token
                //

                ULONG SessionId = PipeServer::GetCallerSessionId();

                ok = WTSQueryUserToken(SessionId, &OldTokenHandle);

                if (!ok)
                    return NULL;

                ShouldAdjustSessionId = false;
            }
        }
    }

    if (! OldTokenHandle) {

        //
        // caller is not sandboxed, or a sandboxed caller did not ask
        // for a special token, so use process token
        //

        ok = OpenProcessToken(
                    CallerProcessHandle, TOKEN_RIGHTS, &OldTokenHandle);
        if (! ok)
            return NULL;
    }

    //
    // duplicate the token into a new primary token,
    // then adjust session and default DACL
    //

    ok = DuplicateTokenEx(OldTokenHandle, TOKEN_ADJUST_PRIVILEGES | TOKEN_RIGHTS, NULL,
                          SecurityIdentification, TokenPrimary,
                          &NewTokenHandle);
    if (! ok)
        NewTokenHandle = NULL;

    if (ok && ShouldAdjustSessionId) {

        ULONG SessionId = PipeServer::GetCallerSessionId();
        ok = SetTokenInformation(NewTokenHandle, TokenSessionId,
                                 &SessionId, sizeof(ULONG));
    }

    if (ok && ShouldAdjustDacl) {

        //
        // if caller is sandboxed and asked for a system token,
        // then we want to adjust the DACL in the new token
        //

		if (SbieApi_QueryConfBool(boxname, L"ExposeBoxedSystem", FALSE))
			ok = RunSandboxedSetDacl(CallerProcessHandle, NewTokenHandle, GENERIC_ALL, TRUE);
        else if (SbieApi_QueryConfBool(boxname, L"AdjustBoxedSystem", TRUE))
            // OriginalToken BEGIN
            if(!SbieApi_QueryConfBool(boxname, L"NoSecurityIsolation", FALSE) && !SbieApi_QueryConfBool(boxname, L"OriginalToken", FALSE))
            // OriginalToken END
			ok = RunSandboxedSetDacl(CallerProcessHandle, NewTokenHandle, GENERIC_READ, FALSE);
    
        if (ok && SbieApi_QueryConfBool(boxname, L"StripSystemPrivileges", TRUE)) {

            ok = RunSandboxedStripPrivileges(NewTokenHandle);
        }
    }

    if (! ok) {
        LastError = GetLastError();
        if (NewTokenHandle) {
            CloseHandle(NewTokenHandle);
            NewTokenHandle = NULL;
        }
    }

    CloseHandle(OldTokenHandle);

    if (! ok)
        SetLastError(LastError);

    return NewTokenHandle;
}


//---------------------------------------------------------------------------
// RunSandboxedSetDacl
//---------------------------------------------------------------------------


BOOL ProcessServer::RunSandboxedSetDacl(
    HANDLE CallerProcessHandle, HANDLE NewTokenHandle, DWORD AccessMask, bool useUserSID, HANDLE idProcess)
{
    static UCHAR AnonymousLogonSid[12] = {
        1,                                      // Revision
        1,                                      // SubAuthorityCount
        0,0,0,0,0,5, // SECURITY_NT_AUTHORITY   // IdentifierAuthority
        SECURITY_ANONYMOUS_LOGON_RID,0,0,0      // SubAuthority
    };

    extern UCHAR SandboxieSid[12];

    ULONG LastError;
	HANDLE hToken;
	ULONG len;
    BOOL ok;

    //
    // When SbieSvc launches a service process as SYSTEM, make sure the
    // default DACL of the new process includes the caller's SID.  This
    // resolves a problem where a client MsiExec invokes the service
    // MsiExec, which in turn invokes a custom action MsiExec process,
    // and the client MsiExec fails to open the custom action process.
    //

    UCHAR *WorkSpace = (UCHAR *)HeapAlloc(GetProcessHeap(), 0, 8192);
    if (! WorkSpace)
        return FALSE;

	TOKEN_GROUPS	   *pLogOn = (TOKEN_GROUPS *)WorkSpace;
    TOKEN_USER         *pUser = (TOKEN_USER *)WorkSpace;
    TOKEN_DEFAULT_DACL *pDacl = (TOKEN_DEFAULT_DACL *)(WorkSpace + 512);
	PSID pSid;

    //
    // get the token for the calling process, extract the user SID
    //

    

    ok = OpenProcessToken(CallerProcessHandle, TOKEN_QUERY, &hToken);
    LastError = GetLastError();

    if (! ok)
        goto finish;

	if (useUserSID)
	{
		ok = GetTokenInformation(hToken, TokenUser, pUser, 512, &len);
		LastError = GetLastError();

        if (ok && idProcess != NULL) // this is used when starting a service
        {
            //
            // in Sandboxie version 4, the primary process token is going to be
            // the anonymous token which isn't very useful here, so get the
            // textual SID string and convert it into a SID value
            //

            //
            // in Sandboxie version 5.57 instead of using the anonymous SID 
            // we can use box specific custom SIDs,
            // when comparing we skip the revision and the SubAuthorityCount
            // also we conpare only the domain portion of the SID as the rest 
            // will be different for each box
            //
           
            if (memcmp(pUser->User.Sid, AnonymousLogonSid, sizeof(AnonymousLogonSid)) == 0
             || memcmp(((UCHAR*)pUser->User.Sid) + 2, SandboxieSid, 10) == 0) {

                PSID TempSid;
                WCHAR SidString[96];
                SbieApi_QueryProcess(idProcess, NULL, NULL, SidString, NULL);
                if (SidString[0]) {
                    if (ConvertStringSidToSid(SidString, &TempSid)) {
                        memcpy(pUser + 1, TempSid, GetLengthSid(TempSid));
                        pUser->User.Sid = (PSID)(pUser + 1);
                        LocalFree(TempSid);
                    }
                }
            }
        }

		pSid = pUser->User.Sid;
	}
	else
	{
		ok = GetTokenInformation(hToken, TokenLogonSid, pLogOn, 512, &len);
		LastError = GetLastError();

		pSid = pLogOn->Groups[0].Sid; // use the LogonSessionId token
	}

    CloseHandle(hToken);

    if (! ok)
        goto finish;

    //
    // extract the default DACL, update it and store it back
    //

    ok = GetTokenInformation(
            NewTokenHandle, TokenDefaultDacl, pDacl, (8192 - 512), &len);
    LastError = GetLastError();

    if (! ok)
        goto finish;

    PACL pAcl = pDacl->DefaultDacl;

    pAcl->AclSize += sizeof(ACCESS_ALLOWED_ACE)
                   - sizeof(DWORD)              // minus SidStart member
                   + (WORD)GetLengthSid(pSid);

    AddAccessAllowedAce(pAcl, ACL_REVISION, AccessMask, pSid);

    ok = SetTokenInformation(
            NewTokenHandle, TokenDefaultDacl, pDacl, (8192 - 512));
    LastError = GetLastError();

    //
    // finish
    //

finish:

    HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, WorkSpace);

    if (! ok)
        SetLastError(LastError);
    return ok;
}


//---------------------------------------------------------------------------
// RunSandboxedStripPrivilege
//---------------------------------------------------------------------------


BOOL ProcessServer::RunSandboxedStripPrivilege(HANDLE NewTokenHandle, LPCWSTR lpName)
{
    LUID luid;

    if (!LookupPrivilegeValue(NULL, lpName, &luid))
        return FALSE;

    TOKEN_PRIVILEGES NewState;
    NewState.PrivilegeCount = 1;
    NewState.Privileges[0].Luid = luid;
    NewState.Privileges[0].Attributes = SE_PRIVILEGE_REMOVED; // Note: A once removed pivilege can not be re added!

    NTSTATUS status = NtAdjustPrivilegesToken(NewTokenHandle, FALSE, &NewState, sizeof(NewState), (PTOKEN_PRIVILEGES)NULL, 0);

    return NT_SUCCESS(status); // STATUS_SUCCESS or STATUS_NOT_ALL_ASSIGNED when the privilege wasn't there in the first place, which is also passes NT_SUCCESS
}


//---------------------------------------------------------------------------
// RunSandboxedStripPrivilege
//---------------------------------------------------------------------------


BOOL ProcessServer::RunSandboxedStripPrivileges(HANDLE NewTokenHandle)
{
    BOOLEAN ok = RunSandboxedStripPrivilege(NewTokenHandle, SE_TCB_NAME);           // security critical
    if (ok) ok = RunSandboxedStripPrivilege(NewTokenHandle, SE_CREATE_TOKEN_NAME);  // usually not held, but in case
    //if (ok) ok = RunSandboxedStripPrivilege(NewTokenHandle, SE_ASSIGNPRIMARYTOKEN_NAME);
    return ok;
}


//---------------------------------------------------------------------------
// RunSandboxedStartProcess
//---------------------------------------------------------------------------


BOOL ProcessServer::RunSandboxedStartProcess(
    HANDLE PrimaryTokenHandle, LONG_PTR BoxNameOrModelPid,
    WCHAR *cmd, const WCHAR *dir, WCHAR *env, 
    BOOL* FilterHandles, ULONG crflags,
    STARTUPINFO *si, PROCESS_INFORMATION *pi)
{
    HANDLE ImpersonationTokenHandle = NULL;
    ULONG LastError;
    BOOL ok = TRUE;
    bool CmdAltered = false;
    bool StartProgramInSandbox = true;

    //
    // create the new process in the target session using the token handle
    //

    ULONG crflags2 = crflags & (CREATE_NO_WINDOW | CREATE_SUSPENDED
                |   HIGH_PRIORITY_CLASS | ABOVE_NORMAL_PRIORITY_CLASS
                |   BELOW_NORMAL_PRIORITY_CLASS | IDLE_PRIORITY_CLASS
                |   CREATE_UNICODE_ENVIRONMENT);
    if (crflags2 != crflags) {

        ok = FALSE;
        LastError = ERROR_INVALID_PARAMETER;
        
    } else {

        // RunSandboxedDupAndCloseHandles will un-suspend if necessary
        crflags2 |= CREATE_SUSPENDED | CREATE_UNICODE_ENVIRONMENT;

        const WCHAR* service = NULL;

#ifndef DRV_BREAKOUT
        if (BoxNameOrModelPid == 0) { // breakout process
            StartProgramInSandbox = false;
            *FilterHandles = TRUE;
        } else
#endif

        // check if special request to run Start.exe outside the sandbox
        if (wcscmp(cmd, L"*COMSRV*") == 0 && BoxNameOrModelPid < 0) {
            cmd = RunSandboxedComServer((ULONG) -BoxNameOrModelPid);
            if (! cmd) {
                SetLastError(ERROR_ACCESS_DENIED);
                return FALSE;
            }
            dir = NULL;
            CmdAltered = true;
            StartProgramInSandbox = false;
            *FilterHandles = TRUE;
        }

        // check if we are starting rpcss or dcomlaunch
        else if ((service = wcscmp(cmd, L"*RPCSS*") == 0 ? L"RpcSs" : NULL, service) 
              /*|| (service = wcscmp(cmd, L"*DCOM*") == 0 ? L"DcomLaunch" : NULL, service)*/) {
            WCHAR program[64];
            wcscpy(program, SANDBOXIE);
            wcscat(program, service);
            wcscat(program, L".exe");

            WCHAR homedir[MAX_PATH];
            SbieApi_GetHomePath(NULL, 0, homedir, MAX_PATH);

            cmd = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, 512 * sizeof(WCHAR));
            if (! cmd) {
                SetLastError(ERROR_ACCESS_DENIED);
                return FALSE;
            }
            _snwprintf(cmd, 512, L"\"%s\\%s\"", homedir, program);
            dir = NULL;
            CmdAltered = true;
        }

        // for certain usecases it may be desirable to run a sandbox in session 0
        // to start a process in that session we use a unused flag bit in STARTUPINFOW::dwFlags
        // if this bit is set we start a process based on our SYSTEM own token, security whise this is
        // similar to running boxed services with "RunServicesAsSystem=y" hence to mitigate potential
        // issues it is recommended to activate "DropAdminRights=y" for boxed using this feature
        // Note: BoxNameOrModelPid > 0 is only true when the caller is not sandboxed
        bool bSession0 = (BoxNameOrModelPid > 0) && ((si->dwFlags & 0x80000000) != 0);
        if (bSession0) {
            OpenProcessToken(GetCurrentProcess(), TOKEN_IMPERSONATE | TOKEN_QUERY | TOKEN_DUPLICATE | STANDARD_RIGHTS_READ, &PrimaryTokenHandle);
        }

        // impersonate caller in case they have a different device map
        // with different drive mappings
        ok = DuplicateToken(PrimaryTokenHandle,
                            SecurityImpersonation,
                            &ImpersonationTokenHandle);

        if (bSession0) {
            CloseHandle(PrimaryTokenHandle);
            PrimaryTokenHandle = NULL;
        }

        if (ok)
            ok = SetThreadToken(NULL, ImpersonationTokenHandle);

        if (ok) {

            // create new process
            ok = CreateProcessAsUser(
                    PrimaryTokenHandle, NULL, cmd, NULL, NULL, FALSE,
                    crflags2, env, dir, si, pi);
            LastError = GetLastError();
        }
    }

    //
    // if creation was successful, notify driver of the new process.
    // this is necessary because in cross session process creation,
    // the new process is a child of winlogon.exe, making it difficult
    // to associate the new process with this service, so it is best
    // that we tell the driver exactly which process we created
    //

    if (ok) {

        if (BoxNameOrModelPid > 0) { // > 0 BoxName, 0 break out, < 0 PID

            ok = SetThreadToken(NULL, ImpersonationTokenHandle);
            if (! ok)
                LastError = GetLastError();
        }

        if (ok && StartProgramInSandbox) {

            LONG rc = SbieApi_Call(API_START_PROCESS, 2,
                                      (ULONG_PTR)BoxNameOrModelPid, (ULONG_PTR)pi->dwProcessId);
            if (rc != 0) {

                LastError = RtlNtStatusToDosError(rc);
                ok = FALSE;
            }
        }

        if (! ok) {
            SetThreadToken(NULL, NULL);
            TerminateProcess(pi->hProcess, 1);
            CloseHandle(pi->hThread);
            CloseHandle(pi->hProcess);
        }
    }

    SetThreadToken(NULL, NULL);
    if (ImpersonationTokenHandle)
        CloseHandle(ImpersonationTokenHandle);

    if (CmdAltered)
        HeapFree(GetProcessHeap(), 0, cmd);

    if (! ok)
        SetLastError(LastError);
    return ok;
}


//---------------------------------------------------------------------------
// RunSandboxedComServer
//---------------------------------------------------------------------------


WCHAR *ProcessServer::RunSandboxedComServer(ULONG CallerProcessId)
{
    const HANDLE CallerPid = (HANDLE)(ULONG_PTR)CallerProcessId;

    // make sure caller is a COM server process,
    // see also Custom_ComServer in core/dll/custom.c

    const ULONG _FlagsOn    = SBIE_FLAG_FORCED_PROCESS
                            | SBIE_FLAG_PROTECTED_PROCESS;
    const ULONG _FlagsOff   = SBIE_FLAG_IMAGE_FROM_SANDBOX
                            | SBIE_FLAG_PROCESS_IN_PCA_JOB;
    ULONG CallerProcessFlags =
                (ULONG)SbieApi_QueryProcessInfo(CallerPid, 0);

    if ((CallerProcessFlags & (_FlagsOn | _FlagsOff)) != _FlagsOn)
        return NULL;

    WCHAR CallerBoxName[BOXNAME_COUNT];
    if (0 != SbieApi_QueryProcess(
                            CallerPid, CallerBoxName, NULL, NULL, NULL))
        return NULL;

    //
    // create a new command line:
    // SbieSvc.exe SANDBOXIE_ComProxy_ComServer:BoxName
    //

#ifdef _WIN64
    ULONG ntdll32_base = (ULONG)SbieApi_QueryProcessInfo(CallerPid, 'nt32');
#else
    const ULONG ntdll32_base = 0;
#endif _WIN64

    const ULONG cmd_len = (MAX_PATH + 128) * sizeof(WCHAR);
    WCHAR *cmd = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, cmd_len);
    if (! cmd)
        return NULL;
    cmd[0] = L'\"';
    SbieApi_GetHomePath(NULL, 0, &cmd[1], MAX_PATH);
    if (ntdll32_base)
        wcscat(cmd, L"\\32");
    wcscat(cmd, L"\\" SBIESVC_EXE L"\" " SANDBOXIE L"_ComProxy_ComServer:");
    wcscat(cmd, CallerBoxName);

    return cmd;
}


//---------------------------------------------------------------------------
// RunSandboxedDupAndCloseHandles
//---------------------------------------------------------------------------


BOOL ProcessServer::RunSandboxedDupAndCloseHandles(
    HANDLE CallerProcessHandle, BOOL FilterHandles, ULONG crflags,
    PROCESS_INFORMATION *piInput, PROCESS_INFORMATION *piReply)
{
    ULONG LastError;
    BOOL ok = TRUE;

    if (!FilterHandles) {      // *COMSRV* case or breakout process

        if (! SbieApi_QueryProcessInfo( // check is sandboxed
                    (HANDLE)(ULONG_PTR)piInput->dwProcessId, 0)) {

            SetLastError(ERROR_PROCESS_ABORTED);
            ok = FALSE;
        }
    }

    if (ok) {
        // Note: PROCESS_SUSPEND_RESUME is enough to start a debugging session which will give a full access handle in the first debug event (diversenok)
        DWORD dwRead =  STANDARD_RIGHTS_READ | SYNCHRONIZE |
                        PROCESS_VM_READ | PROCESS_QUERY_INFORMATION | //PROCESS_SUSPEND_RESUME | unlike THREAD_SUSPEND_RESUME this one is dangerous
                        PROCESS_QUERY_LIMITED_INFORMATION;
        ok = DuplicateHandle(GetCurrentProcess(), piInput->hProcess,
                             CallerProcessHandle, &piReply->hProcess,
                             FilterHandles ? dwRead : 0, FALSE, FilterHandles ? 0 : DUPLICATE_SAME_ACCESS);
    }
    if (ok) {
        DWORD dwRead =  STANDARD_RIGHTS_READ | SYNCHRONIZE |
                        THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION | THREAD_SUSPEND_RESUME | 
                        THREAD_QUERY_LIMITED_INFORMATION;
        ok = DuplicateHandle(GetCurrentProcess(), piInput->hThread,
                             CallerProcessHandle, &piReply->hThread,
                             FilterHandles ? dwRead : 0, FALSE, FilterHandles ? 0 : DUPLICATE_SAME_ACCESS);
    }

    if (ok) {
        if (! (crflags & CREATE_SUSPENDED)) {
            if (ResumeThread(piInput->hThread) == -1)
                ok = FALSE;
        }
    }

    if (ok) {

        piReply->dwProcessId = piInput->dwProcessId;
        piReply->dwThreadId  = piInput->dwThreadId;

    } else {

        LastError = GetLastError();
        TerminateProcess(piInput->hProcess, 1);
    }

    CloseHandle(piInput->hThread);
    CloseHandle(piInput->hProcess);

    if (! ok)
        SetLastError(LastError);
    return ok;
}


//---------------------------------------------------------------------------
// RunUpdaterHandler
//---------------------------------------------------------------------------


MSG_HEADER *ProcessServer::RunUpdaterHandler(MSG_HEADER *msg)
{
    //
    // validate request structure
    //

    ULONG err, lvl;

    PROCESS_RUN_UPDATER_REQ *req = (PROCESS_RUN_UPDATER_REQ *)msg;
    if (req->h.length < sizeof(PROCESS_RUN_UPDATER_REQ))
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);

    if (!(   req->cmd_ofs                           <= PIPE_MAX_DATA_LEN
        &&  (req->cmd_len * sizeof(WCHAR))          <= PIPE_MAX_DATA_LEN
        &&  (req->cmd_ofs + (req->cmd_len * sizeof(WCHAR))) <= req->h.length))
        return SHORT_REPLY(ERROR_INVALID_PARAMETER);

    ULONG CallerPid = PipeServer::GetCallerProcessId();
    ULONG CallerSession = PipeServer::GetCallerSessionId();
    
    //
    // only unsandboxed signed programs are allowed to use this mechanism
    //

    if(SbieApi_QueryProcessInfo((HANDLE)(ULONG_PTR)CallerPid, 0))
        return SHORT_REPLY(STATUS_ACCESS_DENIED);

#ifndef WITH_DEBUG
    if (!PipeServer::IsCallerSigned())
        return SHORT_REPLY(STATUS_INVALID_SIGNATURE);
#endif

    //
    // create full updater command line
    //

    ULONG len = MAX_PATH * 2 + req->cmd_len;
    WCHAR *cmd = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, len * sizeof(WCHAR));

    cmd[0] = L'\"';
    GetModuleFileName(NULL, &cmd[1], MAX_PATH);
    WCHAR *ptr = wcsrchr(cmd, L'\\');
    if (ptr)
        ptr[1] = L'\0';
    wcscat(cmd, L"UpdUtil.exe\" ");
    ptr = wcschr(cmd, L'\0');

    memcpy(ptr, ((UCHAR *)&req->h) + req->cmd_ofs, req->cmd_len * sizeof(WCHAR));
    ptr[req->cmd_len] = L'\0';

    //
    // execute request
    //

    PROCESS_INFORMATION piReply;
    memzero(&piReply, sizeof(PROCESS_INFORMATION));

    //
    // we start by opening the calling process
    //

    HANDLE CallerProcessHandle = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_DUP_HANDLE, FALSE, CallerPid);

    if (CallerProcessHandle) {

        HANDLE PrimaryTokenHandle = NULL;

        if (req->elevate == 2) {

            //
            // run as system, works also for non administrative users
            //

            const ULONG TOKEN_RIGHTS = TOKEN_QUERY          | TOKEN_DUPLICATE
                                        | TOKEN_ADJUST_DEFAULT | TOKEN_ADJUST_SESSIONID
                                        | TOKEN_ADJUST_GROUPS  | TOKEN_ASSIGN_PRIMARY;

            BOOL ok = OpenProcessToken(GetCurrentProcess(), TOKEN_RIGHTS, &PrimaryTokenHandle);

            if (ok) {
                HANDLE hNewToken;
                ok = DuplicateTokenEx(
                    PrimaryTokenHandle, TOKEN_RIGHTS, NULL, SecurityAnonymous,
                    TokenPrimary, &hNewToken);
                if (ok) {
                    CloseHandle(PrimaryTokenHandle);
                    PrimaryTokenHandle = hNewToken;
                }
            }

            if (ok) {
                ok = SetTokenInformation(PrimaryTokenHandle, TokenSessionId, &CallerSession, sizeof(ULONG));
            }

        } else {

            //
            // get calling user's token
            //

            WTSQueryUserToken(CallerSession, &PrimaryTokenHandle);

            if (req->elevate == 1 && !SbieIniServer::TokenIsAdmin(PrimaryTokenHandle, true)) {

                //
                // run elevated as the current user, if the user is not in the admin group
                // this will fail, and the process started as normal user
                //

                ULONG returnLength;
                TOKEN_LINKED_TOKEN linkedToken = {0};
                NtQueryInformationToken(PrimaryTokenHandle, (TOKEN_INFORMATION_CLASS)TokenLinkedToken,
                    &linkedToken, sizeof(TOKEN_LINKED_TOKEN), &returnLength);

                CloseHandle(PrimaryTokenHandle);
                PrimaryTokenHandle = linkedToken.LinkedToken;                
            }
        }

        if (PrimaryTokenHandle) {

            //
            // copy STARTUPINFO parameters from caller
            //

            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            memzero(&pi, sizeof(PROCESS_INFORMATION));
            memzero(&si, sizeof(STARTUPINFO));
            si.cb = sizeof(STARTUPINFO);
            si.dwFlags = STARTF_FORCEOFFFEEDBACK;
            si.wShowWindow = SW_SHOWNORMAL;

            if (CreateProcessAsUser(PrimaryTokenHandle, NULL, cmd, NULL, NULL, FALSE, 
                CREATE_SUSPENDED, NULL, NULL, &si, &pi)) {

                //
                // FilterHandles = TRUE to prevent privilege escalation in case 
                // a signed but hijacked agent requested the start of a utility process
                // and would subsequenty try to hijack the utility process.
                //

                if (RunSandboxedDupAndCloseHandles( // resumes the process if needed
                        CallerProcessHandle, TRUE, 0, &pi, &piReply)) {

                    err = 0;
                    lvl = 0;

                } else {

                    err = GetLastError();
                    lvl = 0x55;
                }

            } else {

                err = GetLastError();
                lvl = 0x44;
            }


            CloseHandle(PrimaryTokenHandle);

        } else {

            err = GetLastError();
            lvl = 0x33;
        }

        CloseHandle(CallerProcessHandle);

    } else {

        err = GetLastError();
        lvl = 0x22;
    }

    HeapFree(GetProcessHeap(), 0, cmd);

    PROCESS_RUN_UPDATER_RPL *rpl = (PROCESS_RUN_UPDATER_RPL *)
                            LONG_REPLY(sizeof(PROCESS_RUN_UPDATER_RPL));
    if (rpl) {
        rpl->h.status    = err;
        rpl->hProcess    = (ULONG64)(ULONG_PTR)piReply.hProcess;
        rpl->hThread     = (ULONG64)(ULONG_PTR)piReply.hThread;
        rpl->dwProcessId = piReply.dwProcessId;
        rpl->dwThreadId  = piReply.dwThreadId;
    }
    return (MSG_HEADER *)rpl;
}
