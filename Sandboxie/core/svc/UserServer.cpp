/*
 * Copyright 2022 David Xanatos, xanasoft.com
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
// User Proxy Server
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "PipeServer.h"
#include "UserServer.h"
#include "QueueWire.h"
#include "UserWire.h"
#include "core/dll/sbiedll.h"
#include "core/drv/api_defs.h"
#include "common/my_version.h"
#include <stdlib.h>
#include <sddl.h>
#include <aclapi.h>
#include <wtsapi32.h>
#include <shellapi.h>
#include "misc.h"
#include "core/drv/verify.h"

#define PATTERN XPATTERN
extern "C" {
#include "common/pattern.h"
} // extern "C"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define MAX_RPL_BUF_SIZE    32768


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _USER_WORKER {

    LIST_ELEM list_elem;

    HANDLE hProcess;

    ULONG session_id;

} USER_WORKER;


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


UserServer::UserServer()
{
    InitializeCriticalSection(&m_WorkersLock);
    List_Init(&m_WorkersList);
    m_QueueEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    // worker data
    m_QueueName = NULL;
    m_ParentPid = 0;
    m_SessionId = 0;
}

UserServer::~UserServer()
{
	// cleanup CS
	DeleteCriticalSection(&m_WorkersLock);
}


//---------------------------------------------------------------------------
// UserServer
//---------------------------------------------------------------------------


UserServer *UserServer::GetInstance()
{
    static UserServer *_instance = NULL;
    if (! _instance)
        _instance = new UserServer();
    return _instance;
}


//---------------------------------------------------------------------------
// StartWorker
//---------------------------------------------------------------------------

volatile HANDLE UserServer__hParentProcess = NULL;

_FX VOID UserServer__APC(ULONG_PTR hParent)
{
    UserServer__hParentProcess = (HANDLE)hParent;
}

ULONG UserServer::StartWorker(ULONG session_id)
{
    const ULONG TOKEN_RIGHTS = TOKEN_QUERY          | TOKEN_DUPLICATE
                             | TOKEN_ADJUST_DEFAULT | TOKEN_ADJUST_SESSIONID
                             | TOKEN_ADJUST_GROUPS  | TOKEN_ASSIGN_PRIMARY;
    HANDLE hOldToken = NULL;
    HANDLE hNewToken = NULL;
    ULONG status;
    BOOL ok = TRUE;

    //
    // terminate an existing worker process that stopped functioning
    //

    USER_WORKER *worker = (USER_WORKER *)List_Head(&m_WorkersList);
    while (worker) {

        USER_WORKER *worker_next = (USER_WORKER *)List_Next(worker);

        if (worker->session_id == session_id) {

            TerminateProcess(worker->hProcess, 1);
            CloseHandle(worker->hProcess);

            List_Remove(&m_WorkersList, worker);
            HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, worker);
        }

        worker = worker_next;
    }

    //
    // build the command line for the User Worker Proxy Server Process
    //

    WCHAR EventName[96];

    WCHAR *cmdline = (WCHAR *)HeapAlloc(
                                GetProcessHeap(), 0, 768 * sizeof(WCHAR));
    if (! cmdline)
        return STATUS_INSUFFICIENT_RESOURCES;

    cmdline[0] = L'\"';
    status = SbieApi_GetHomePath(NULL, 0, &cmdline[1], 512);
    if (status != 0)
        ok = FALSE;
    else {

        WCHAR *cmdptr = cmdline + wcslen(cmdline);
        wcscpy(cmdptr, L"\\" SBIESVC_EXE L"\" ");
        cmdptr += wcslen(cmdptr);
        wsprintf(cmdptr, L"%s_UserProxy_%08X,%d",
            SANDBOXIE, session_id, GetCurrentProcessId());

        wcscpy(EventName, L"Global\\");
        wcscat(EventName, cmdptr);
    }

    //
    // use the users security token for the worker
    //

    if (ok) {
        //ok = OpenProcessToken(GetCurrentProcess(), TOKEN_RIGHTS, &hOldToken);
        ok = WTSQueryUserToken(session_id, &hOldToken);
        if (! ok)
            status = 0x72000000 | GetLastError();
    }

    if (ok) {
        ok = DuplicateTokenEx(
                hOldToken, TOKEN_RIGHTS, NULL, SecurityAnonymous,
                TokenPrimary, &hNewToken);
        if (! ok)
            status = 0x73000000 | GetLastError();
    }

    //if (ok) {
    //    ok = SetTokenInformation(
    //            hNewToken, TokenSessionId, &session_id, sizeof(ULONG));
    //    if (! ok)
    //        status = 0x74000000 | GetLastError();
    //}

    //
    // create an event object for the new User Worker process
    // the user process needs to be able to set this event
    // so set the appropriate security descriptor
    //

    SECURITY_DESCRIPTOR sd;
    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);

    SECURITY_ATTRIBUTES sa = {0};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = FALSE;
    sa.lpSecurityDescriptor = &sd;

    HANDLE EventHandle = CreateEvent(&sa, TRUE, FALSE, EventName);
    if (EventHandle)
        ResetEvent(EventHandle);
    else {
        status = 0x75000000 | GetLastError();
        ok = FALSE;
    }

    //
    // create the new process
    //

    if (ok) {

        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        memzero(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);
        si.dwFlags = STARTF_FORCEOFFFEEDBACK;
        si.lpDesktop = L"WinSta0\\Default";

        ok = CreateProcessAsUser(
                hNewToken, NULL, cmdline, NULL, NULL, FALSE,
                ABOVE_NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);
        if (! ok)
            status = 0x76000000 | GetLastError();
        else {

            //
            // wait for the new process to signal the event
            //

            HANDLE WaitHandles[2];
            WaitHandles[0] = EventHandle;
            WaitHandles[1] = pi.hProcess;

            status = WaitForMultipleObjects(2, WaitHandles, FALSE, 15 * 1000);
            if (status != WAIT_OBJECT_0) {
                status = 0x77000000 | status;
                ok = FALSE;

            } else {

                //
                // create a new worker process element
                //

                worker = (USER_WORKER *)HeapAlloc(
                                        GetProcessHeap(), 0, sizeof(USER_WORKER));
                if (! worker) {

                    status = STATUS_INSUFFICIENT_RESOURCES;
                    ok = FALSE;

                } else {

                    worker->session_id = session_id;
                    worker->hProcess = pi.hProcess;

                    List_Insert_After(&m_WorkersList, NULL, worker);

                    status = 0;
                }
            }

            //
            // since the worker is running as user it can't open this service process, even for SYNCHRONIZE only
            // hence we duplicate the required token and use APC to pass it to our new worker.
            //

            HANDLE hThis;
            if(NT_SUCCESS(DuplicateHandle(NtCurrentProcess(), NtCurrentProcess(), pi.hProcess, &hThis, SYNCHRONIZE, FALSE, 0)))
                QueueUserAPC(UserServer__APC, pi.hThread, (ULONG_PTR)hThis);

            CloseHandle(pi.hThread);
            if (! ok)
                CloseHandle(pi.hProcess);
        }
    }

    if (EventHandle)
        CloseHandle(EventHandle);
    if (hNewToken)
        CloseHandle(hNewToken);
    if (hOldToken)
        CloseHandle(hOldToken);
    HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, cmdline);

    return status;
}


//---------------------------------------------------------------------------
// StartAsync
//---------------------------------------------------------------------------

struct SStartupParam
{
    ULONG session_id;
    HANDLE hEvent;
};

ULONG UserServer__StartupWorker(void* _Param)
{
    SStartupParam* pParam = (SStartupParam*)_Param;

    //
    // thart the proxy process
    //

    UserServer::GetInstance()->StartWorker(pParam->session_id);

    //
    // notify the requesting party that the server is now up and running
    //

    SetEvent(pParam->hEvent);

    HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, pParam);
    return 0;
}

ULONG UserServer::StartAsync(ULONG session_id, HANDLE hEvent)
{
    SStartupParam* pParam = (SStartupParam*)HeapAlloc(GetProcessHeap(), 0, sizeof(SStartupParam));
    pParam->session_id = session_id;
    pParam->hEvent = hEvent;

    HANDLE hThread = CreateThread(NULL, 0, UserServer__StartupWorker, (void *)pParam, 0, NULL);
    if (!hThread) {
        HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, pParam);
        return STATUS_UNSUCCESSFUL;
    }
    CloseHandle(hThread);
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// ReportError2336
//---------------------------------------------------------------------------


void UserServer::ReportError2336(ULONG session_id, ULONG errlvl, ULONG status)
{
    SbieApi_LogEx(session_id, 2336, L"[%02X / %08X]", errlvl, status);
}


//---------------------------------------------------------------------------
//
// Worker Process
//
//---------------------------------------------------------------------------


typedef struct USER_JOB {

    LIST_ELEM list_elem;
    HANDLE handle;

} USER_JOB;


//---------------------------------------------------------------------------
// RunWorker
//---------------------------------------------------------------------------


void UserServer::RunWorker(const WCHAR *cmdline)
{
    //
    // select between a normal SbieSVC User Worker
    //

    UserServer *pThis = GetInstance();

    //
    // get process id for parent (which should be the main SbieSvc process)
    //

    NTSTATUS status;
    ULONG len;
    PROCESS_BASIC_INFORMATION info;

    status = NtQueryInformationProcess(
        NtCurrentProcess(), ProcessBasicInformation,
        &info, sizeof(PROCESS_BASIC_INFORMATION), &len);

    if (! NT_SUCCESS(status))
        return;

    pThis->m_ParentPid = (ULONG)info.InheritedFromUniqueProcessId;
    if (! pThis->m_ParentPid)
        return;

    //
    // create message queue and process incoming requests
    //

    if (! pThis->CreateQueueWorker(cmdline))
        return;

    //
    // exit when parent dies
    //

    while (UserServer__hParentProcess == NULL)
        SleepEx(10, TRUE); // be in a waitable state for he APC's

    //HANDLE hParentProcess =
    //                OpenProcess(SYNCHRONIZE, FALSE, pThis->m_ParentPid);
    //if (! hParentProcess)
    //    hParentProcess = NtCurrentProcess();
    status = WaitForSingleObject(UserServer__hParentProcess, INFINITE);
    if (status == WAIT_OBJECT_0)
        ExitProcess(0);
}


//---------------------------------------------------------------------------
// CreateQueueWorker
//---------------------------------------------------------------------------


bool UserServer::CreateQueueWorker(const WCHAR *cmdline)
{
    //
    // create a queue with the queue manager
    //

    WCHAR *ptr = (WCHAR*)wcsstr(cmdline, L"_UserProxy");
    if (! ptr)
        return false;
    ULONG len = (wcslen(ptr) + 1) * sizeof(WCHAR);
    m_QueueName = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, len);
    memcpy(m_QueueName, ptr, len);
    *m_QueueName = L'*';
    _wcsupr(m_QueueName);

    m_SessionId = wcstol(m_QueueName + 11, &ptr, 16);
    if (*ptr != L',')
        return false;
    *ptr = L'\0';           // terminate queue name

    m_ParentPid = wcstol(ptr + 1, &ptr, 10);
    if (*ptr != L'\0')
        return false;

    ULONG status = SbieDll_QueueCreate(m_QueueName, &m_QueueEvent);
    if (status != 0)
        return false;

    //
    // signal the event object
    //

    WCHAR EventName[96];
    wcscpy(EventName, L"Global\\");
    wcscat(EventName, cmdline);
    HANDLE EventHandle = OpenEvent(EVENT_MODIFY_STATE, FALSE, EventName);
    if (EventHandle) {
        SetEvent(EventHandle);
        CloseHandle(EventHandle);
    }

    //
    // prepare the dispatch table for incoming requests
    //

    const ULONG m_WorkerFuncs_len =
                    sizeof(WorkerFunc) * (USER_MAX_REQUEST_CODE + 4);
    m_WorkerFuncs = (WorkerFunc *)
                    HeapAlloc(GetProcessHeap(), 0, m_WorkerFuncs_len);
    memzero(m_WorkerFuncs, m_WorkerFuncs_len);

    m_WorkerFuncs[USER_OPEN_FILE]          = &UserServer::OpenFile;
    m_WorkerFuncs[USER_SHELL_EXEC]         = &UserServer::OpenDocument;
    


    //
    // register a worker thread to process incoming queue requests
    //

    HANDLE WaitHandle;
    if (! RegisterWaitForSingleObject(&WaitHandle, m_QueueEvent,
                                      QueueCallbackWorker, (void *)this,
                                      INFINITE, WT_EXECUTEDEFAULT))
        return false;

    return true;
}


//---------------------------------------------------------------------------
// QueueCallbackWorker
//---------------------------------------------------------------------------


void UserServer::QueueCallbackWorker(void *arg, BOOLEAN timeout)
{
    UserServer *pThis = (UserServer *)arg;
    while (1) {
        bool check_for_more_requests = pThis->QueueCallbackWorker2();
        if (! check_for_more_requests)
            break;
    }
}


//---------------------------------------------------------------------------
// QueueCallbackWorker2
//---------------------------------------------------------------------------


bool UserServer::QueueCallbackWorker2(void)
{
    //
    // get next request
    //
    // note that STATUS_END_OF_FILE here indicates there are no more requests
    // in the queue at this time and we should go resume waiting on the event
    //

    WorkerArgs args;
    ULONG request_id;
    ULONG data_len;
    ULONG rpl_len;
    void *data_ptr;
    ULONG rpl_buf[MAX_RPL_BUF_SIZE / sizeof(ULONG)];

    ULONG status = SbieDll_QueueGetReq(m_QueueName, &args.pid, NULL,
                                       &request_id, &data_ptr, &data_len);
    if (status != 0) {
        if (status != STATUS_END_OF_FILE)
            ReportError2336(-1, 0x81, status);
        return false;
    }

    //
    // process request
    //

    status = STATUS_INVALID_SYSTEM_SERVICE;
    rpl_len = sizeof(ULONG);

    ULONG msgid = *(ULONG *)data_ptr;

    if (msgid < USER_MAX_REQUEST_CODE) {

        WorkerFunc WorkerFuncPtr = m_WorkerFuncs[msgid];
        if (WorkerFuncPtr) {

            bool issue_request = true;

            //
            // issue request
            //

            if (issue_request) {

                args.req_len = data_len;
                args.req_buf = data_ptr;
                args.rpl_len = rpl_len;
                args.rpl_buf = rpl_buf;

                status = (this->*WorkerFuncPtr)(&args);
                if (status == 0)
                    rpl_len = args.rpl_len;
            }
        }
    }

    //
    // send reply
    //
    // note that STATUS_END_OF_FILE here indicates the calling process is no
    // longer there, in which case we still return true to process any other
    // requests from other processes which may be in the queue
    //

    *rpl_buf = status;
    status = SbieDll_QueuePutRpl(
                            m_QueueName, request_id, rpl_buf, rpl_len);

    SbieDll_FreeMem(data_ptr);

    if (status != 0 && status != STATUS_END_OF_FILE) {
        ReportError2336(-1, 0x82, status);
        return false;
    }

    return true;
}


//---------------------------------------------------------------------------
// OpenFile
//---------------------------------------------------------------------------


ULONG UserServer::OpenFile(WorkerArgs *args)
{
    USER_OPEN_FILE_REQ *req = (USER_OPEN_FILE_REQ *)args->req_buf;
    USER_OPEN_FILE_RPL *rpl = (USER_OPEN_FILE_RPL *)args->rpl_buf;

    if (args->req_len < sizeof(USER_OPEN_FILE_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    WCHAR* path_buff = (WCHAR*)(((UCHAR*)req) + req->FileNameOffset);

    //
    // check if the caller belongs to our session
    //

    ULONG session_id;
    WCHAR boxname[BOXNAME_COUNT];
    if (!NT_SUCCESS(SbieApi_QueryProcess((HANDLE)(ULONG_PTR)args->pid, boxname, NULL, NULL, &session_id))
     || session_id != m_SessionId 
     || !SbieApi_QueryConfBool(boxname, L"EnableEFS", FALSE)) {

        return STATUS_ACCESS_DENIED;
    }

    SCertInfo CertInfo = { 0 };
    if (!NT_SUCCESS(SbieApi_QueryDrvInfo(-1, &CertInfo, sizeof(CertInfo))) || !(CertInfo.active && CertInfo.opt_enc)) {
        const WCHAR* strings[] = { boxname, L"EnableEFS", NULL };
        SbieApi_LogMsgExt(session_id, 6004, strings);
        return STATUS_ACCESS_DENIED;
    } 

    //
    // check if operation is permitted, it must be for a file on a disk
    // and the file access rules must allow for the access
    //

    if(_wcsnicmp(path_buff, L"\\Device\\HarddiskVolume", 22) != 0)
        return STATUS_ACCESS_DENIED;
    
    BOOL write_access = FALSE;

#define FILE_DENIED_ACCESS ~(                                           \
    STANDARD_RIGHTS_READ | GENERIC_READ | SYNCHRONIZE | READ_CONTROL |  \
    FILE_READ_DATA | FILE_READ_EA | FILE_READ_ATTRIBUTES | FILE_EXECUTE)
            
    if (req->DesiredAccess & FILE_DENIED_ACCESS)
        write_access = TRUE;
            
    if (req->CreateDisposition != FILE_OPEN)
        write_access = TRUE;

    if (req->CreateOptions & FILE_DELETE_ON_CLOSE)
        write_access = TRUE;

#undef FILE_DENIED_ACCESS

    POOL *pool;
#ifdef USE_MATCH_PATH_EX
    LIST *normal_list, *open_list, *closed_list, *write_list, *read_list;
#else
    LIST *open_list, *closed_list, *write_list;
#endif

    if (!NT_SUCCESS(GetProcessPathList('fo', args->pid, (void **)&pool, &open_list))
     || !NT_SUCCESS(GetProcessPathList('fc', args->pid, (void **)&pool, &closed_list))
     || !NT_SUCCESS(GetProcessPathList('fr', args->pid, (void **)&pool, &write_list))
#ifdef USE_MATCH_PATH_EX
     || !NT_SUCCESS(GetProcessPathList('fn', args->pid, (void **)&pool, &normal_list))
     || !NT_SUCCESS(GetProcessPathList('fw', args->pid, (void **)&pool, &read_list))
#endif
        )  
        return STATUS_INTERNAL_ERROR;

#ifdef USE_MATCH_PATH_EX
    ULONG64 Dll_ProcessFlags = SbieApi_QueryProcessInfo((HANDLE)args->pid, 0);

    BOOLEAN use_rule_specificity = (Dll_ProcessFlags & SBIE_FLAG_RULE_SPECIFICITY) != 0;
    //BOOLEAN use_privacy_mode = (Dll_ProcessFlags & SBIE_FLAG_PRIVACY_MODE) != 0;

    //ULONG mp_flags = SbieDll_MatchPathImpl(use_rule_specificity, use_privacy_mode, path_buff, normal_list, open_list, closed_list, write_list, read_list);
    ULONG mp_flags = SbieDll_MatchPathImpl(use_rule_specificity, path_buff, normal_list, open_list, closed_list, write_list, read_list);
#else
    ULONG mp_flags = SbieDll_MatchPathImpl(path_buff, open_list, closed_list, write_list);
#endif

    Pool_Delete(pool);

    if(write_access && (!PATH_IS_OPEN(mp_flags) || PATH_IS_READ(mp_flags)))
        return STATUS_ACCESS_DENIED;
    if(PATH_IS_CLOSED(mp_flags) || PATH_IS_WRITE(mp_flags))
        return STATUS_ACCESS_DENIED;

    //
    // open the file on behalf of the caller
    //

    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    RtlInitUnicodeString(&objname, path_buff);

    LARGE_INTEGER AllocSize;
    AllocSize.QuadPart = req->AllocationSize;

    void* pEaBuff = NULL;
    if (req->EaBufferOffset != 0)
        pEaBuff = ((UCHAR*)req) + req->EaBufferOffset;

    HANDLE hFile;
    IO_STATUS_BLOCK IoStatusBlock;
    rpl->error = NtCreateFile(&hFile, req->DesiredAccess, &objattrs, &IoStatusBlock, AllocSize.QuadPart != 0 ? &AllocSize : NULL, 
        req->FileAttributes, req->ShareAccess, req->CreateDisposition, req->CreateOptions, pEaBuff, req->EaLength);
    rpl->Status = IoStatusBlock.Status;
    rpl->Information = IoStatusBlock.Information;

    if (NT_SUCCESS(rpl->error)) {

        //
        // duplicate the handle into the calling process, and close our own
        //

        HANDLE hProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, args->pid);
        if (hProcess) {
            DuplicateHandle(NtCurrentProcess(), hFile, hProcess, (HANDLE*)&rpl->FileHandle, req->DesiredAccess, FALSE, 0);
            CloseHandle(hProcess);
        }
        else
            rpl->error = STATUS_UNSUCCESSFUL;

        NtClose(hFile);
    }

    args->rpl_len = sizeof(USER_OPEN_FILE_RPL);
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// OpenFile
//---------------------------------------------------------------------------


ULONG UserServer::OpenDocument(WorkerArgs *args)
{
    USER_SHELL_EXEC_REQ *req = (USER_SHELL_EXEC_REQ *)args->req_buf;

    if (args->req_len < sizeof(USER_SHELL_EXEC_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    WCHAR* path_buff = (WCHAR*)(((UCHAR*)req) + req->FileNameOffset);

    //
    // check if the caller belongs to our session
    //

    ULONG session_id;
    WCHAR boxname[BOXNAME_COUNT];
    if (!NT_SUCCESS(SbieApi_QueryProcess((HANDLE)(ULONG_PTR)args->pid, boxname, NULL, NULL, &session_id))
     || session_id != m_SessionId) {

        return STATUS_ACCESS_DENIED;
    }

    //
    // check the BreakoutDocument list and execute if ok
    //

    if (SbieDll_CheckPatternInList(path_buff, (ULONG)wcslen(path_buff), boxname, L"BreakoutDocument")) {

        SHELLEXECUTEINFO shex;
        memzero(&shex, sizeof(SHELLEXECUTEINFO));
        shex.cbSize = sizeof(SHELLEXECUTEINFO);
        shex.fMask = 0;
        shex.hwnd = NULL;
        shex.lpVerb = L"open";
        shex.lpFile = path_buff;
        shex.lpParameters = NULL;
        shex.lpDirectory = NULL;
        shex.nShow = SW_SHOWNORMAL;
        shex.hInstApp = NULL;

        typedef BOOL (*P_ShellExecuteEx)(void *);
        HMODULE shell32 = LoadLibrary(L"shell32.dll");
        P_ShellExecuteEx pShellExecuteEx = (P_ShellExecuteEx)GetProcAddress(shell32, "ShellExecuteExW");
        if (!pShellExecuteEx)
            return STATUS_ENTRYPOINT_NOT_FOUND;

        if (pShellExecuteEx(&shex))
            return STATUS_SUCCESS;
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_ACCESS_DENIED;
}


//---------------------------------------------------------------------------
// GetProcessPathList
//---------------------------------------------------------------------------


ULONG UserServer::GetProcessPathList(ULONG path_code,
    ULONG pid, void **out_pool, LIST **out_list)
{
    const HANDLE xpid = (HANDLE)(ULONG_PTR)pid;

    ULONG len;
    LONG status = SbieApi_QueryPathList(path_code, &len, NULL, xpid, TRUE);
    if (status != 0)
        return status;

    status = STATUS_INSUFFICIENT_RESOURCES;

    POOL *pool = Pool_Create();
    if (! pool)
        return status;

    WCHAR *path = (WCHAR *)Pool_Alloc(pool, len);
    LIST *list = (LIST *)Pool_Alloc(pool, sizeof(LIST));

    if (path && list)
        status = SbieApi_QueryPathList(path_code, NULL, path, xpid, TRUE);

    if (status != STATUS_SUCCESS) {
        Pool_Delete(pool);
        return status;
    }

    List_Init(list);
    while (*((ULONG*)path) != -1) {
        ULONG level = *((ULONG*)path);
        path += sizeof(ULONG)/sizeof(WCHAR);
        PATTERN *pattern = Pattern_Create(pool, path, TRUE, level);
        if (! pattern) {
            Pool_Delete(pool);
            return status;
        }
        List_Insert_After(list, NULL, pattern);
        path += wcslen(path) + 1;
    }

    *out_pool = pool;
    *out_list = list;

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// CheckProcessPathList
//---------------------------------------------------------------------------


//bool UserServer::CheckProcessPathList(LIST *list, const WCHAR *string)
//{
//    BOOLEAN ret = FALSE;
//
//    ULONG length = wcslen(string);
//    ULONG path_len = (length + 1) * sizeof(WCHAR);
//    WCHAR* path_lwr = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, path_len);
//    if (!path_lwr) {
//        SbieApi_Log(2305, NULL);
//        goto finish;
//    }
//    memcpy(path_lwr, string, path_len);
//    path_lwr[length] = L'\0';
//    _wcslwr(path_lwr);
//
//    PATTERN *pat = (PATTERN *)List_Head(list);
//    while (pat) {
//        if (Pattern_Match(pat, path_lwr, length)) {
//            ret = TRUE;
//            goto finish;
//        }
//        pat = (PATTERN *)List_Next(pat);
//    }
//
//finish:
//    HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, path_lwr);
//
//    return ret;
//}

