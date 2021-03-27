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
// GUI Proxy Server
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "PipeServer.h"
#include "GuiServer.h"
#include "QueueWire.h"
#include "GuiWire.h"
#include "core/dll/sbiedll.h"
#include "core/drv/api_defs.h"
#include "common/my_version.h"
#include <stdlib.h>
#include <sddl.h>
#include <aclapi.h>
#include <dde.h>
#include "misc.h"

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


typedef struct _GUI_SLAVE {

    LIST_ELEM list_elem;

    HANDLE hProcess;

    ULONG session_id;

} GUI_SLAVE;


typedef struct _WND_HOOK {

    LIST_ELEM list_elem;
    ULONG pid;
    DWORD hthread;
    ULONG64 hproc;
    int HookCount;

} WND_HOOK;

//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static HWND DDE_Request_ProxyWnd = NULL;


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


GuiServer::GuiServer()
{
    InitializeCriticalSection(&m_SlavesLock);
    List_Init(&m_SlavesList);
    m_QueueEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    // slave data
    m_QueueName = NULL;
    m_ParentPid = 0;
    m_SessionId = 0;

    List_Init(&m_WndHooks);

    OSVERSIONINFOW osvi = { 0 };
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
	/*NTSTATUS(WINAPI *RtlGetVersion)(LPOSVERSIONINFOW);
	*(FARPROC*)&RtlGetVersion = GetProcAddress(_Ntdll, "RtlGetVersion");
	if (RtlGetVersion != NULL)
		RtlGetVersion(&osvi);
	else*/
	GetVersionExW(&osvi); // since windows 10 this one is lying
    m_nOSVersion = osvi.dwMajorVersion * 10 + osvi.dwMinorVersion;
}


//---------------------------------------------------------------------------
// GuiServer
//---------------------------------------------------------------------------


GuiServer *GuiServer::GetInstance()
{
    static GuiServer *_instance = NULL;
    if (! _instance)
        _instance = new GuiServer();
    return _instance;
}


//---------------------------------------------------------------------------
// InitProcess
//---------------------------------------------------------------------------


bool GuiServer::InitProcess(
    HANDLE hProcess, ULONG process_id, ULONG session_id, BOOLEAN add_to_job)
{
    ULONG status;
    ULONG errlvl;
    BOOL is_in_job;

    //
    // the new process must not already be in a job
    //

    errlvl = 0x11;

    if (add_to_job) {

        // See AssignProcessToJobObject in http://msdn.microsoft.com/en-us/library/windows/desktop/ms681949(v=vs.85).aspx
        // A process can be associated with more than one job starting in Windows 8 and Windows Server 2012

        // Note, almost all processes in Dell Venue tablet with Win8.1 belong to a job object already.
        // The process can still be safely assigned to a sandbox object later.
        // Skip IsProcessInJob checking after Win8 (6.2)

        if (m_nOSVersion < 62)
        {
            if (IsProcessInJob(hProcess, NULL, &is_in_job)) {

                if (is_in_job) {
                    status = STATUS_ALREADY_COMMITTED;
                    goto finish;
                }

            }
            else {
                status = GetLastError();
                goto finish;
            }
        }
    }

    //
    // instruct a GUI slave to put the new process into a restricted job
    //

    EnterCriticalSection(&m_SlavesLock);

    status = SendMessageToSlave(session_id, process_id, add_to_job);
    if (status == STATUS_OBJECT_NAME_NOT_FOUND) {

        //
        // if the message could not be sent on the queue,
        // start a new slave and send another message
        //

        status = StartSlave(session_id);

        if (status != 0)
            errlvl = 0x22;
        else {

            status = SendMessageToSlave(session_id, process_id, add_to_job);
            if (status != STATUS_SUCCESS)
                errlvl = 0x33;
        }

    } else if (status != STATUS_SUCCESS) {

        errlvl = 0x44;
    }

    LeaveCriticalSection(&m_SlavesLock);

    //
    // the new process must now be associated with a job
    //

    if (add_to_job && (status == 0)) {

        errlvl = 0x55;

        if (IsProcessInJob(hProcess, NULL, &is_in_job)) {

            if (! is_in_job) {

                status = STATUS_LOGON_NOT_GRANTED;
                goto finish;
            }

        } else {
            status = GetLastError();
            goto finish;
        }
    }

    //
    // finish
    //

finish:

    if (status != 0) {
        ReportError2336(session_id, errlvl, status);
        return false;
    }

    return true;
}


//---------------------------------------------------------------------------
// SendMessageToSlave
//---------------------------------------------------------------------------


ULONG GuiServer::SendMessageToSlave(ULONG session_id, ULONG process_id,
                                    BOOLEAN add_to_job)
{
    //
    // prepare a QUEUE_PUTREQ_REQ message to send to the slave process
    //

    const ULONG req_len =
                    sizeof(QUEUE_PUTREQ_REQ) + sizeof(GUI_INIT_PROCESS_REQ);
    ULONG64 req_space[(req_len + sizeof(ULONG64) - 1) / sizeof(ULONG64)];
    QUEUE_PUTREQ_REQ *req1 = (QUEUE_PUTREQ_REQ *)req_space;

    req1->h.length = req_len;
    req1->h.msgid = MSGID_QUEUE_PUTREQ;
    wsprintf(req1->queue_name, L"*GUIPROXY_%08X", session_id);
    req1->event_handle = (ULONG64)(ULONG_PTR)m_QueueEvent;
    req1->data_len = sizeof(GUI_INIT_PROCESS_REQ);

    GUI_INIT_PROCESS_REQ *data = (GUI_INIT_PROCESS_REQ *)req1->data;
    data->msgid = process_id ? GUI_INIT_PROCESS : GUI_SHUTDOWN;
    data->process_id = process_id;
    data->add_to_job = add_to_job;

    //
    // send the message through the queue service
    //

    PipeServer *pipe = PipeServer::GetPipeServer();
    ULONG status, req_id;

    QUEUE_PUTREQ_RPL *rpl1 = (QUEUE_PUTREQ_RPL *)pipe->Call(&req1->h);
    if (rpl1) {

        status = rpl1->h.status;
        req_id = rpl1->req_id;
        pipe->FreeMsg(&rpl1->h);

    } else
        status = STATUS_INSUFFICIENT_RESOURCES;

    //
    // wait for a reply
    //

    if (status == 0) {

        status = WaitForSingleObject(m_QueueEvent, 10 * 1000);
    }

    //
    // receive the reply
    //

    if (status == 0) {

        QUEUE_GETRPL_REQ req2;
        req2.h.length = sizeof(QUEUE_GETRPL_REQ);
        req2.h.msgid = MSGID_QUEUE_GETRPL;
        wcscpy(req2.queue_name, req1->queue_name);
        req2.req_id = req_id;

        QUEUE_GETRPL_RPL *rpl2 = (QUEUE_GETRPL_RPL *)pipe->Call(&req2.h);
        if (rpl2) {

            status = rpl2->h.status;
            if (status == 0) {
                if (rpl2->data_len >= sizeof(ULONG))
                    status = *(ULONG *)rpl2->data;
                else
                    status = STATUS_INFO_LENGTH_MISMATCH;
            }

            pipe->FreeMsg(&rpl2->h);

        } else
            status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}


//---------------------------------------------------------------------------
// StartSlave
//---------------------------------------------------------------------------


ULONG GuiServer::StartSlave(ULONG session_id)
{
    const ULONG TOKEN_RIGHTS = TOKEN_QUERY          | TOKEN_DUPLICATE
                             | TOKEN_ADJUST_DEFAULT | TOKEN_ADJUST_SESSIONID
                             | TOKEN_ADJUST_GROUPS  | TOKEN_ASSIGN_PRIMARY;
    HANDLE hOldToken = NULL;
    HANDLE hNewToken = NULL;
    ULONG status;
    BOOL ok = TRUE;

    //
    // terminate an existing slave process that stopped functioning
    //

    GUI_SLAVE *slave = (GUI_SLAVE *)List_Head(&m_SlavesList);
    while (slave) {

        GUI_SLAVE *slave_next = (GUI_SLAVE *)List_Next(slave);

        if (slave->session_id == session_id) {

            TerminateProcess(slave->hProcess, 1);
            CloseHandle(slave->hProcess);

            List_Remove(&m_SlavesList, slave);
            HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, slave);
        }

        slave = slave_next;
    }

    //
    // build the command line for the GUI Slave Proxy Server Process
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
        wsprintf(cmdptr, L"%s_GuiProxy_%08X,%d",
            SANDBOXIE, session_id, GetCurrentProcessId());

        wcscpy(EventName, L"Global\\");
        wcscat(EventName, cmdptr);
    }

    //
    // duplicate current process token (LocalSystem) and specify the
    // desired session id for the new slave process
    //

    if (ok) {
        ok = OpenProcessToken(GetCurrentProcess(), TOKEN_RIGHTS, &hOldToken);
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

    if (ok) {
        ok = SetTokenInformation(
                hNewToken, TokenSessionId, &session_id, sizeof(ULONG));
        if (! ok)
            status = 0x74000000 | GetLastError();
    }

    //
    // create an event object for the new GUI slave process
    //

    HANDLE EventHandle = CreateEvent(NULL, TRUE, FALSE, EventName);
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
                // create a new slave process element
                //

                slave = (GUI_SLAVE *)HeapAlloc(
                                        GetProcessHeap(), 0, sizeof(GUI_SLAVE));
                if (! slave) {

                    status = STATUS_INSUFFICIENT_RESOURCES;
                    ok = FALSE;

                } else {

                    slave->session_id = session_id;
                    slave->hProcess = pi.hProcess;

                    List_Insert_After(&m_SlavesList, NULL, slave);

                    status = 0;
                }
            }

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
// ReportError2336
//---------------------------------------------------------------------------


void GuiServer::ReportError2336(ULONG session_id, ULONG errlvl, ULONG status)
{
    SbieApi_LogEx(session_id, 2336, L"[%02X / %08X]", errlvl, status);
}


//---------------------------------------------------------------------------
//
// Slave Process
//
//---------------------------------------------------------------------------


typedef struct GUI_JOB {

    LIST_ELEM list_elem;
    HANDLE handle;
    WCHAR boxname[64];

} GUI_JOB;


//---------------------------------------------------------------------------
// RunSlave
//---------------------------------------------------------------------------


void GuiServer::RunSlave(const WCHAR *cmdline)
{
    //
    // select between a normal SbieSVC GUI slave, and a console helper slave
    //

    PCWCH ConsoleCmdline = wcsstr(cmdline, L"_GuiProxy_Console,");
    if (ConsoleCmdline) {
        RunConsoleSlave(ConsoleCmdline + 18); // this exits the process
        return;
    }

    GuiServer *pThis = GetInstance();

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
    // create window station
    //

    if (! pThis->GetWindowStationAndDesktopName(NULL))
        return;

    //
    // create message queue and process incoming requests
    //

    if (! pThis->CreateQueueSlave(cmdline))
        return;

    //
    // get address of NtUserQueryWindow
    //

    GetNtUserQueryWindow();

    //
    // exit when parent dies
    //

    HANDLE hParentProcess =
                    OpenProcess(SYNCHRONIZE, FALSE, pThis->m_ParentPid);
    if (! hParentProcess)
        hParentProcess = NtCurrentProcess();
    status = WaitForSingleObject(hParentProcess, INFINITE);
    if (status == WAIT_OBJECT_0)
        ExitProcess(0);
}


//---------------------------------------------------------------------------
// GetNtUserQueryWindow
//---------------------------------------------------------------------------


void *GuiServer::GetNtUserQueryWindow(void)
{
    static void *_NtUserQueryWindow = NULL;

    if (_NtUserQueryWindow)
        return _NtUserQueryWindow;

    UCHAR *code = (UCHAR *)GetProcAddress(
                        GetModuleHandle(L"user32.dll"), "IsHungAppWindow");
    if (code) {

        //
        // IsHungAppWindow should start with a call to NtUserQueryWindow
        //

#ifdef _WIN64
        const ULONG _E8_Offset = 9;
#else ! _WIN64
        const ULONG _E8_Offset = 10;
#endif _WIN64

        if (code[_E8_Offset] == 0xE8) {

            code = code + _E8_Offset + 5
                 + (LONG_PTR)*(LONG *)(code + _E8_Offset + 1);

            //
            // make sure the address we think is NtUserQueryWindow
            // actually looks like a syscall stub
            //

            BOOLEAN ok = FALSE;

#ifdef _WIN64

            if (*(ULONG *)code == 0xB8D18B4C
                && (*(USHORT *)(code + 8) == 0x050F) || *(USHORT *)(code + 8) == 0x04F6)
                ok = TRUE;

#else ! _WIN64

            // 32-bit prior to Windows 8
            if (    code[0] == 0xB8 && code[5] == 0xBA
                 && *(USHORT *)(code + 10) == 0x12FF)
                ok = TRUE;

            // 32-bit Windows 8
            if (    code[0] == 0xB8 && code[5] == 0xE8
                 && *(USHORT *)(code + 10) == 0x08C2)
                ok = TRUE;

#endif _WIN64

            if (ok) {
                _NtUserQueryWindow = code;
                return _NtUserQueryWindow;
            }
        }
    }

    //Windows 10 RS1 now exports win32k functions in win32u.dll
    code = (UCHAR *)GetProcAddress(GetModuleHandle(L"win32u.dll"), "NtUserQueryWindow");
    if (code) {
        _NtUserQueryWindow = code;
        return _NtUserQueryWindow;
    }
    return NULL;
}


//---------------------------------------------------------------------------
// CreateQueueSlave
//---------------------------------------------------------------------------


bool GuiServer::CreateQueueSlave(const WCHAR *cmdline)
{
    //
    // create a queue with the queue manager
    //

    WCHAR *ptr = (WCHAR*)wcsstr(cmdline, L"_GuiProxy");
    if (! ptr)
        return false;
    ULONG len = (wcslen(ptr) + 1) * sizeof(WCHAR);
    m_QueueName = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, len);
    memcpy(m_QueueName, ptr, len);
    *m_QueueName = L'*';
    _wcsupr(m_QueueName);

    m_SessionId = wcstol(m_QueueName + 10, &ptr, 16);
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

    const ULONG m_SlaveFuncs_len =
                    sizeof(SlaveFunc) * (GUI_MAX_REQUEST_CODE + 4);
    m_SlaveFuncs = (SlaveFunc *)
                    HeapAlloc(GetProcessHeap(), 0, m_SlaveFuncs_len);
    memzero(m_SlaveFuncs, m_SlaveFuncs_len);

    m_SlaveFuncs[GUI_INIT_PROCESS]          = &GuiServer::InitProcessSlave;
    m_SlaveFuncs[GUI_GET_WINDOW_STATION]    = &GuiServer::GetWindowStationSlave;
    m_SlaveFuncs[GUI_CREATE_CONSOLE]        = &GuiServer::CreateConsoleSlave;
    m_SlaveFuncs[GUI_QUERY_WINDOW]          = &GuiServer::QueryWindowSlave;
    m_SlaveFuncs[GUI_IS_WINDOW]             = &GuiServer::IsWindowSlave;
    m_SlaveFuncs[GUI_GET_WINDOW_LONG]       = &GuiServer::GetWindowLongSlave;
    m_SlaveFuncs[GUI_GET_WINDOW_PROP]       = &GuiServer::GetWindowPropSlave;
    m_SlaveFuncs[GUI_GET_WINDOW_HANDLE]     = &GuiServer::GetWindowHandleSlave;
    m_SlaveFuncs[GUI_GET_CLASS_NAME]        = &GuiServer::GetClassNameSlave;
    m_SlaveFuncs[GUI_GET_WINDOW_RECT]       = &GuiServer::GetWindowRectSlave;
    m_SlaveFuncs[GUI_GET_WINDOW_INFO]       = &GuiServer::GetWindowInfoSlave;
    m_SlaveFuncs[GUI_GRANT_HANDLE]          = &GuiServer::GrantHandleSlave;
    m_SlaveFuncs[GUI_ENUM_WINDOWS]          = &GuiServer::EnumWindowsSlave;
    m_SlaveFuncs[GUI_FIND_WINDOW]           = &GuiServer::FindWindowSlave;
    m_SlaveFuncs[GUI_MAP_WINDOW_POINTS]     = &GuiServer::MapWindowPointsSlave;
    m_SlaveFuncs[GUI_SET_WINDOW_POS]        = &GuiServer::SetWindowPosSlave;
    m_SlaveFuncs[GUI_CLOSE_CLIPBOARD]       = &GuiServer::CloseClipboardSlave;
    m_SlaveFuncs[GUI_GET_CLIPBOARD_DATA]    = &GuiServer::GetClipboardDataSlave;
    m_SlaveFuncs[GUI_GET_CLIPBOARD_METAFILE] = &GuiServer::GetClipboardMetaFileSlave;
    m_SlaveFuncs[GUI_SEND_POST_MESSAGE]     = &GuiServer::SendPostMessageSlave;
    m_SlaveFuncs[GUI_SEND_COPYDATA]         = &GuiServer::SendCopyDataSlave;
    m_SlaveFuncs[GUI_CLIP_CURSOR]           = &GuiServer::ClipCursorSlave;
    m_SlaveFuncs[GUI_SET_FOREGROUND_WINDOW] = &GuiServer::SetForegroundWindowSlave;
    m_SlaveFuncs[GUI_MONITOR_FROM_WINDOW]   = &GuiServer::MonitorFromWindowSlave;
    m_SlaveFuncs[GUI_SPLWOW64]              = &GuiServer::SplWow64Slave;
    m_SlaveFuncs[GUI_CHANGE_DISPLAY_SETTINGS]   = &GuiServer::ChangeDisplaySettingsSlave;
    m_SlaveFuncs[GUI_SET_CURSOR_POS]        = &GuiServer::SetCursorPosSlave;
    m_SlaveFuncs[GUI_REMOVE_HOST_WINDOW]    = &GuiServer::RemoveHostWindow;
    m_SlaveFuncs[GUI_GET_RAW_INPUT_DEVICE_INFO] = &GuiServer::GetRawInputDeviceInfoSlave;
    m_SlaveFuncs[GUI_WND_HOOK_NOTIFY]       = &GuiServer::WndHookNotifySlave;
    m_SlaveFuncs[GUI_WND_HOOK_REGISTER]     = &GuiServer::WndHookRegisterSlave;


    //
    // register a worker thread to process incoming queue requests
    //

    HANDLE WaitHandle;
    if (! RegisterWaitForSingleObject(&WaitHandle, m_QueueEvent,
                                      QueueCallbackSlave, (void *)this,
                                      INFINITE, WT_EXECUTEDEFAULT))
        return false;

    return true;
}


//---------------------------------------------------------------------------
// QueueCallbackSlave
//---------------------------------------------------------------------------


void GuiServer::QueueCallbackSlave(void *arg, BOOLEAN timeout)
{
    GuiServer *pThis = (GuiServer *)arg;
    while (1) {
        bool check_for_more_requests = pThis->QueueCallbackSlave2();
        if (! check_for_more_requests)
            break;
    }
}


//---------------------------------------------------------------------------
// QueueCallbackSlave2
//---------------------------------------------------------------------------


bool GuiServer::QueueCallbackSlave2(void)
{
    //
    // get next request
    //
    // note that STATUS_END_OF_FILE here indicates there are no more requests
    // in the queue at this time and we should go resume waiting on the event
    //

    SlaveArgs args;
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

    if (msgid < GUI_MAX_REQUEST_CODE) {

        SlaveFunc SlaveFuncPtr = m_SlaveFuncs[msgid];
        if (SlaveFuncPtr) {

            bool issue_request = true;

            //
            // make sure the request is coming from the same session
            // (with the exception of the GUI_INIT_PROCESS message)
            //

            if (msgid != GUI_INIT_PROCESS) {

                ULONG session_id;
                status = SbieApi_QueryProcess((HANDLE)(ULONG_PTR)args.pid,
                                              NULL, NULL, NULL, &session_id);

                if (status != 0)
                    issue_request = false;

                else if (session_id != m_SessionId) {
                    status = STATUS_NOT_SAME_DEVICE;
                    issue_request = false;
                }
            }

            //
            // issue request
            //

            if (issue_request) {

                args.req_len = data_len;
                args.req_buf = data_ptr;
                args.rpl_len = rpl_len;
                args.rpl_buf = rpl_buf;

                status = (this->*SlaveFuncPtr)(&args);
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
// GetJobObjectForAssign
//---------------------------------------------------------------------------


HANDLE GuiServer::GetJobObjectForAssign(const WCHAR *boxname)
{
    //
    // find an existing job corresponding to a specific session_id/boxname
    // combination.  if the job exists and has zero active processes, then
    // we delete it and recreate the job object.  if the job object did not
    // exist, we simply create a new job object.  the idea here is to make
    // sure that a new sandboxing sequence starts with a clean job that is
    // not affected by any old USER handle grants
    //
    // we use the SlavesLock and SlavesList data members to manage jobs
    //

    HANDLE hJobObject = NULL;

    EnterCriticalSection(&m_SlavesLock);

    GUI_JOB *job = (GUI_JOB *)List_Head(&m_SlavesList);
    while (job) {

        if (_wcsicmp(job->boxname, boxname) == 0) {

            if (! job->handle)
                break;

            JOBOBJECT_BASIC_ACCOUNTING_INFORMATION info;
            BOOL ok = QueryInformationJobObject(
                            job->handle, JobObjectBasicAccountingInformation,
                            &info, sizeof(info), NULL);
            if (! ok) {
                // this should not happen
                ReportError2336(-1, 0x99, GetLastError());
                info.ActiveProcesses = 0;
            }

            if (info.ActiveProcesses) {

                //
                // there are running programs associated with an existing
                // job object, so attach the new process to the same job
                //

                hJobObject = job->handle;

            } else {

                //
                // an existing job object is not in use, delete it
                //

                CloseHandle(job->handle);
                job->handle = NULL;
            }

            break;
        }

        job = (GUI_JOB *)List_Next(job);
    }

    //
    // create a new job object if necessary
    //

    if (! hJobObject) {

        static PSECURITY_DESCRIPTOR _jobsd = NULL;
        if (! _jobsd) {

            //
            // create a security descriptor with a limited dacl
            // owner:system, group:system, dacl(allow;generic_all;system)
            //

            if (! ConvertStringSecurityDescriptorToSecurityDescriptor(
                        L"O:SYG:SYD:(A;;GA;;;SY)", SDDL_REVISION_1,
                        &_jobsd, NULL)) {
                // this should not happen
                ReportError2336(-1, 0x98, GetLastError());
            }
        }

        if (! job) {
            job = (GUI_JOB *)HeapAlloc(GetProcessHeap(), 0, sizeof(GUI_JOB));
            if (job) {
                job->handle = NULL;
                wcscpy(job->boxname, boxname);
                List_Insert_After(&m_SlavesList, NULL, job);
            }
        }

        if (job) {

            WCHAR jobname[128];
            wsprintf(jobname, L"%s_%s_Session_%d_Job_%08X",
                        SANDBOXIE, boxname, m_SessionId, GetTickCount());

            SECURITY_ATTRIBUTES sa;
            sa.nLength = sizeof(sa);
            sa.lpSecurityDescriptor = _jobsd;
            sa.bInheritHandle = FALSE;

            hJobObject = CreateJobObject(&sa, jobname);
            if (hJobObject) {

                //
                // set UI restrictions on the job object
                //

                JOBOBJECT_BASIC_UI_RESTRICTIONS info;
                info.UIRestrictionsClass = JOB_OBJECT_UILIMIT_EXITWINDOWS
                                         | JOB_OBJECT_UILIMIT_HANDLES
                                         | JOB_OBJECT_UILIMIT_SYSTEMPARAMETERS
                                         | JOB_OBJECT_UILIMIT_READCLIPBOARD;

                BOOL ok = FALSE;        // set TRUE to skip UIRestrictions

				// OpenToken BEGIN
				if ((SbieApi_QueryConfBool(boxname, L"OpenToken", FALSE) || SbieApi_QueryConfBool(boxname, L"UnrestrictedToken", FALSE)))
					ok = TRUE;
				// OpenToken END
				// OriginalToken BEGIN
				if (SbieApi_QueryConfBool(boxname, L"OriginalToken", FALSE))
					ok = TRUE;
				// OriginalToken END

                if (! ok) {
                    ok = SetInformationJobObject(
                                hJobObject, JobObjectBasicUIRestrictions,
                                &info, sizeof(info));

                    if (ok) {

                        //
                        // grant access to the desktop window, otherwise
                        // a lot of stuff doesn't work correctly
                        //

                        ok = UserHandleGrantAccess(
                                GetDesktopWindow(), hJobObject, TRUE);
                    }
                }
                if (! ok) {
                    // this should not happen
                    ReportError2336(-1, 0x97, GetLastError());
                    CloseHandle(hJobObject);
                    hJobObject = NULL;
                }
            }

            if (hJobObject) {

                wcscpy(job->boxname, boxname);
                job->handle = hJobObject;
            }
        }
    }

    //
    // finish
    //

    ULONG LastError = 0;
    if (! hJobObject)
        LastError = GetLastError();

    LeaveCriticalSection(&m_SlavesLock);

    if (! hJobObject)
        SetLastError(LastError);

    return hJobObject;
}


//---------------------------------------------------------------------------
// GetJobObjectForGrant
//---------------------------------------------------------------------------


HANDLE GuiServer::GetJobObjectForGrant(ULONG pid)
{
    HANDLE hJobObject = NULL;

    WCHAR BoxName[48];
    ULONG SessionId;
    ULONG status = SbieApi_QueryProcess(
                    (HANDLE)(ULONG_PTR)pid, BoxName, NULL, NULL, &SessionId);

    if (status == 0 && SessionId == m_SessionId) {

        EnterCriticalSection(&m_SlavesLock);

        GUI_JOB *job = (GUI_JOB *)List_Head(&m_SlavesList);
        while (job) {

            if (_wcsicmp(job->boxname, BoxName) == 0) {

                hJobObject = job->handle;
                if (hJobObject)
                    break;
            }

            job = (GUI_JOB *)List_Next(job);
        }

        LeaveCriticalSection(&m_SlavesLock);
    }

    return hJobObject;
}


//---------------------------------------------------------------------------
// GetWindowStationAndDesktopName
//---------------------------------------------------------------------------


bool GuiServer::GetWindowStationAndDesktopName(WCHAR *out_name)
{
    static HWINSTA _hWinSta = NULL;
    static HDESK   _hDesk   = NULL;
    static WCHAR   _CombinedName[96];

    //
    // return cached dummy window station and desktop path
    // see also Gui_ConnectToWindowStationAndDesktop core/dll/gui.c
    // which copies this path into RTL_USER_PROCESS_PARAMETERS
    //

    if (out_name) {

        wcscpy(out_name, _CombinedName);
        return true;
    }

    //
    // save the current window station and desktop
    //

    HWINSTA hWinStaSave = GetProcessWindowStation();
    HDESK hDeskSave = GetThreadDesktop(GetCurrentThreadId());

    ULONG errlvl;

    //
    // create dummy window station and desktop objects that have a NULL DACL
    // to make them accessible even from our highly restricted processes
    //

    ULONG sd_space[16];
    memzero(&sd_space, sizeof(sd_space));
    PSECURITY_DESCRIPTOR sd = (PSECURITY_DESCRIPTOR)&sd_space;
    InitializeSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(sd, TRUE, NULL, FALSE);

    SECURITY_ATTRIBUTES sa;
    memzero(&sa, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = sd;
    sa.bInheritHandle = FALSE;

    //
    // on Windows Vista, we set the process token to the untrusted integrity
    // level, so we need to adjust the window station and desktop objects to
    // allow access from processes at untrusted integrity.  the first step
    // is to create an untrusted integrity sacl.  note that the utility
    // ConvertStringSecurityDescriptorToSecurityDescriptor only supports
    // low integrity, so we have to manually adjust the sacl to untrusted
    //

    PACL sacl = NULL;
    PSECURITY_DESCRIPTOR label_sd = NULL;
    if (ConvertStringSecurityDescriptorToSecurityDescriptor(
            L"S:(ML;;NW;;;LW)", SDDL_REVISION_1, &label_sd, NULL)) {

        ULONG_PTR sacl_offset =
            (ULONG_PTR)((SECURITY_DESCRIPTOR_RELATIVE *)label_sd)->Sacl;
        ULONG *sacl_ulongs = (ULONG *)((ULONG_PTR)label_sd + sacl_offset);
        sacl_ulongs[6] = 0;     // change "low" to "untrusted" level

        BOOL sacl_present, sacl_defaulted;
        if (! GetSecurityDescriptorSacl(
                    label_sd, &sacl_present, &sacl, &sacl_defaulted))
            sacl = NULL;
    }

    //
    // create window station object, then switch to this window station
    // to create the desktop object within this window station.
    // WRITE_OWNER access is needed in order to adjust integrity levels
    //

    wsprintf(_CombinedName, L"%s_WinSta_%d", SANDBOXIE, GetTickCount());
    _hWinSta = CreateWindowStation(_CombinedName, CWF_CREATE_ONLY,
                                   WINSTA_ALL_ACCESS | WRITE_OWNER , &sa);
    if (! _hWinSta)
        errlvl = 0x61;

    else {

        if (! SetProcessWindowStation(_hWinSta))
            errlvl = 0x62;
        else {

            WCHAR *desktop_name = _CombinedName + wcslen(_CombinedName);
            wsprintf(desktop_name,
                            L"\\%s_Desktop_%d", SANDBOXIE, GetTickCount());
            ++desktop_name;

            const ACCESS_MASK DESKTOP_ALL_ACCESS = 0x1FF; // see WinUser.h
            _hDesk = CreateDesktop(desktop_name, NULL, NULL, 0,
                                   DESKTOP_ALL_ACCESS | WRITE_OWNER, &sa);
            if (! _hDesk)
                errlvl = 0x63;

            else {

                //
                // on Windows Vista we now need to apply the sacl for
                // untrusted integrity into the object we created
                //

                if (sacl) {

                    //const ULONG LABEL_SECURITY_INFORMATION = (0x00000010L);

                    errlvl = SetSecurityInfo(_hWinSta, SE_WINDOW_OBJECT,
                                LABEL_SECURITY_INFORMATION, NULL, NULL, NULL,
                                sacl);
                    if (errlvl) {
                        SetLastError(errlvl);
                        errlvl = 0x64;

                    } else {

                        errlvl = SetSecurityInfo(_hDesk, SE_WINDOW_OBJECT,
                                LABEL_SECURITY_INFORMATION, NULL, NULL, NULL,
                                sacl);

                        if (errlvl) {
                            SetLastError(errlvl);
                            errlvl = 0x65;
                        }
                    }

                } else
                    errlvl = 0;

                //
                // restore the original window station and desktop
                // and return the handle to the new dummy window station
                //

                if (errlvl == 0) {

                    if (SetProcessWindowStation(hWinStaSave)) {

                        if (SetThreadDesktop(hDeskSave)) {

                            return true;

                        } else
                            errlvl = 0x67;

                    } else
                        errlvl = 0x66;
                }
            }
        }
    }

    ReportError2336(-1, errlvl, GetLastError());
    return false;
}


//---------------------------------------------------------------------------
// InitProcessSlave
//---------------------------------------------------------------------------


ULONG GuiServer::InitProcessSlave(SlaveArgs *args)
{
    GUI_INIT_PROCESS_REQ *req = (GUI_INIT_PROCESS_REQ *)args->req_buf;
    ULONG errlvl;
    ULONG status;
    ULONG session_id;
    WCHAR boxname[64];

    //
    // validate the request
    //

    if (args->pid != m_ParentPid)
        return STATUS_ACCESS_DENIED;

    if (args->req_len != sizeof(GUI_INIT_PROCESS_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    //
    // verify the requested process is running sandboxed in the same session
    //

    HANDLE hProcess = OpenProcess(PROCESS_SET_QUOTA | PROCESS_TERMINATE,
                                  FALSE, req->process_id);
    if (! hProcess) {
        status = GetLastError();
        errlvl = 0x91;
        goto finish;
    }

    status = SbieApi_QueryProcess((HANDLE)(ULONG_PTR)req->process_id,
                                  boxname, NULL, NULL, &session_id);
    if (status != 0) {
        errlvl = 0x92;
        goto finish;
    }

    if (session_id != m_SessionId) {
        status = STATUS_INVALID_PARAMETER;
        errlvl = 0x93;
        goto finish;
    }

    //
    // get the job to put the new process into
    //

    if (req->add_to_job) {

        HANDLE hJobObject = GetJobObjectForAssign(boxname);
        if (! hJobObject) {
            status = GetLastError();
            errlvl = 0x94;
            goto finish;
        }

        // GuiServer::InitProcessSlave is being called twice for forced processes. Starting in FR 15031, this will cause the call below to fail.
        if (! AssignProcessToJobObject(hJobObject, hProcess)) {
            status = GetLastError();
            //errlvl = 0x95;
            //goto finish;
        }
    }

    errlvl = 0;

finish:

    if (hProcess)
        CloseHandle(hProcess);

    if (errlvl) {
        ReportError2336(-1, errlvl, status);
        return STATUS_ACCESS_DENIED;
    }

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// GetWindowStationSlave
//---------------------------------------------------------------------------


ULONG GuiServer::GetWindowStationSlave(SlaveArgs *args)
{
    ULONG errlvl;
    ULONG status;
    ULONG session_id;

    HANDLE hProcess = OpenProcess(   PROCESS_QUERY_INFORMATION
                                   | PROCESS_DUP_HANDLE, FALSE, args->pid);
    if (! hProcess) {
        status = GetLastError();
        errlvl = 0x71;
        goto finish;
    }

    status = SbieApi_QueryProcess((HANDLE)(ULONG_PTR)args->pid,
                                  NULL, NULL, NULL, &session_id);
    if (status != 0 || session_id != m_SessionId) {

        if (status == 0)
            status = STATUS_NOT_SAME_DEVICE;
        errlvl = 0x72;
        goto finish;

    } else {

        GUI_GET_WINDOW_STATION_RPL *rpl =
                        (GUI_GET_WINDOW_STATION_RPL *)args->rpl_buf;

        if (! GetWindowStationAndDesktopName(rpl->name)) {
            status = -1;
            errlvl = 0x73;
            goto finish;
        }

        //
        // give the requesting process limited access to the
        // real window station and desktop
        //
        // if OpenWinClass=* is in effect for the process,
        // give increased access to the window stationn
        //
        //

        HWINSTA local_winsta  = GetProcessWindowStation();
        HDESK   local_desktop = GetThreadDesktop(GetCurrentThreadId());

        if ((! local_winsta) || (! local_desktop)) {
            status = -1;
            errlvl = 0x74;
            goto finish;
        }

        ULONG _WinStaAccess =       WINSTA_ENUMDESKTOPS
                                  | WINSTA_READATTRIBUTES
                                  | WINSTA_ACCESSCLIPBOARD
                               // | WINSTA_CREATEDESKTOP
                               // | WINSTA_WRITEATTRIBUTES
                                  | WINSTA_ACCESSGLOBALATOMS
                               // | WINSTA_EXITWINDOWS
                                  | WINSTA_ENUMERATE
                                  | WINSTA_READSCREEN;

        ULONG64 ProcessFlags =
            SbieApi_QueryProcessInfo((HANDLE)(ULONG_PTR)args->pid, 0);
        if (ProcessFlags & SBIE_FLAG_OPEN_ALL_WIN_CLASS) {

            _WinStaAccess |= WINSTA_WRITEATTRIBUTES;
        }

        if (! DuplicateHandle(NtCurrentProcess(), local_winsta,
                              hProcess, (HANDLE *)&rpl->hwinsta,
                              _WinStaAccess, FALSE, 0)) {
            status = GetLastError();
            errlvl = 0x75;
            goto finish;
        }

        if (! DuplicateHandle(NtCurrentProcess(), local_desktop,
                              hProcess, (HANDLE *)&rpl->hdesk,
                              0, FALSE, DUPLICATE_SAME_ACCESS)) {
            status = GetLastError();
            errlvl = 0x76;
            goto finish;
        }

#ifdef _WIN64

        //
        // if caller is a 32-bit process on 64-bit Windows then it will
        // need the address of its 64-bit PEB.
        // see Gui_ConnectToWindowStationAndDesktop in core/dll/gui.c
        //

        PROCESS_BASIC_INFORMATION info;
        status = NtQueryInformationProcess(
                        hProcess, ProcessBasicInformation,
                        &info, sizeof(PROCESS_BASIC_INFORMATION), &errlvl);
        if (status != 0) {
            errlvl = 0x77;
            goto finish;
        }
        rpl->peb64 = (ULONG)(ULONG_PTR)info.PebBaseAddress;

#endif _WIN64

    }

    errlvl = 0;

finish:

    if (hProcess)
        CloseHandle(hProcess);

    if (errlvl) {
        ReportError2336(-1, errlvl, status);
        return STATUS_ACCESS_DENIED;
    }

    args->rpl_len = sizeof(GUI_GET_WINDOW_STATION_RPL);
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// CreateConsoleSlave
//---------------------------------------------------------------------------


ULONG GuiServer::CreateConsoleSlave(SlaveArgs *args)
{
    GUI_CREATE_CONSOLE_REQ *req = (GUI_CREATE_CONSOLE_REQ *)args->req_buf;
    GUI_CREATE_CONSOLE_RPL *rpl = (GUI_CREATE_CONSOLE_RPL *)args->rpl_buf;

    if (args->req_len != sizeof(GUI_CREATE_CONSOLE_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    //
    // make sure caller is a sandboxed process in our session
    //

    HANDLE hProcess = OpenProcess(PROCESS_DUP_HANDLE | SYNCHRONIZE,
                                  FALSE, args->pid);
    if (! hProcess)
        return STATUS_INVALID_CID;

    WCHAR boxname[48];
    WCHAR image_name[128];
    WCHAR *cmdline = NULL;
    HANDLE hToken1 = NULL;
    HANDLE hToken2 = NULL;
    HANDLE hEvent = NULL;
    BOOL ok;

    ULONG session_id;
    ULONG status = SbieApi_QueryProcess((HANDLE)(ULONG_PTR)args->pid, boxname,
                                        image_name, NULL, &session_id);
    if (status != 0 || session_id != m_SessionId) {

        if (status == 0)
            status = STATUS_NOT_SAME_DEVICE;
        goto finish;
    }

    //
    // the caller should send us an impersonation token for use with
    // CreateProcessAsUser that we need to convert to a primary token
    //

    ok = DuplicateHandle(hProcess, (HANDLE)(ULONG_PTR)req->token,
                         NtCurrentProcess(), &hToken1,
                TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY,
                         FALSE, 0);
    if (! ok) {
        hToken1 = NULL;
        status = STATUS_ACCESS_DENIED;
        goto finish;
    }

    ok = DuplicateTokenEx(
                hToken1, MAXIMUM_ALLOWED, NULL, SecurityAnonymous,
                TokenPrimary, &hToken2);
    if (! ok) {
        hToken2 = NULL;
        status = STATUS_ACCESS_DENIED;
        goto finish;
    }

    //
    // create event object to wait for startup of console helper
    //

    SECURITY_ATTRIBUTES sa;
    memzero(&sa, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = FALSE;

    WCHAR evtname[96];
    wsprintf(evtname, SANDBOXIE L"_ConsoleReadyEvent_%08X:%s", GetTickCount(), boxname);
    hEvent = CreateEvent(&sa, TRUE, FALSE, evtname);
    if (! hEvent) {
        status = STATUS_OBJECT_NAME_COLLISION;
        goto finish;
    }

    //
    // prepare commnand line for console helper process
    //

    cmdline = (WCHAR *)HeapAlloc(
                            GetProcessHeap(), 0, 768 * sizeof(WCHAR));
    if (! cmdline) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto finish;
    }

    cmdline[0] = L'\"';
    status = SbieApi_GetHomePath(NULL, 0, &cmdline[1], 512);
    if (status != 0)
        goto finish;
    wcscat(cmdline, L"\\" SBIESVC_EXE L"\" ");
    wcscat(cmdline, SANDBOXIE L"_GuiProxy_Console,");
    wcscat(cmdline, evtname);

    //
    // start the console helper process
    //

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    memzero(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_FORCEOFFFEEDBACK;
    si.lpTitle = image_name;
    si.wShowWindow = req->show_window;

    ok = CreateProcessAsUser(
            hToken2, NULL, cmdline, NULL, NULL, FALSE,
            ABOVE_NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);

    if (! ok) {
        status = STATUS_INVALID_IMAGE_FORMAT;
        goto finish;
    }

    //
    // wait for the console helper to start and create a console
    // see also RunConsoleSlave below
    //

    status = WaitForSingleObject(hEvent, 20 * 1000);
    if (status != 0) {
        status = STATUS_TIMEOUT;
        goto finish;
    }

    rpl->process_id = pi.dwProcessId;

    //
    // register a callback that will be signalled when the client
    // process is terminated, in which case we will terminate the
    // SbieSvc.exe console helper instance that we started
    //

    HANDLE *WaitHandles = (HANDLE *)cmdline;
    WaitHandles[1] = hProcess;
    WaitHandles[2] = pi.hProcess;

    if (RegisterWaitForSingleObject(
                            &WaitHandles[0], hProcess,
                            ConsoleCallbackSlave, (void *)WaitHandles,
                            INFINITE, WT_EXECUTEONLYONCE)) {
        //
        // if we successfully set up a wait callback, let this callback
        // take care of closing process handles and releasing memory
        //

        cmdline = NULL;
        hProcess = NULL;
        pi.hProcess = NULL;

    } else
        status = STATUS_INSUFFICIENT_RESOURCES;

    //
    // clean up open handles and finish
    //

    CloseHandle(pi.hThread);
    if (pi.hProcess)
        CloseHandle(pi.hProcess);

finish:

    if (cmdline)
        HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, cmdline);
    if (hEvent)
        CloseHandle(hEvent);
    if (hToken2)
        CloseHandle(hToken2);
    if (hToken1)
        CloseHandle(hToken1);
    if (hProcess)
        CloseHandle(hProcess);

    if (status == STATUS_SUCCESS)
        args->rpl_len = sizeof(GUI_CREATE_CONSOLE_RPL);

    return status;
}


//---------------------------------------------------------------------------
// QueryWindowSlave
//---------------------------------------------------------------------------


ULONG GuiServer::QueryWindowSlave(SlaveArgs *args)
{
    GUI_QUERY_WINDOW_REQ *req = (GUI_QUERY_WINDOW_REQ *)args->req_buf;
    GUI_QUERY_WINDOW_RPL *rpl = (GUI_QUERY_WINDOW_RPL *)args->rpl_buf;

    if (args->req_len != sizeof(GUI_QUERY_WINDOW_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    typedef ULONG_PTR (*P_NtUserQueryWindow)(HWND, ULONG_PTR);
    P_NtUserQueryWindow NtUserQueryWindow =
                        (P_NtUserQueryWindow) GetNtUserQueryWindow();
    if (! NtUserQueryWindow)
        return STATUS_NOT_IMPLEMENTED;

    SetLastError(req->error);
    HWND hwnd = (HWND)(LONG_PTR)req->hwnd;
    rpl->result = NtUserQueryWindow(hwnd, (ULONG_PTR)req->type);
    rpl->error = GetLastError();

    args->rpl_len = sizeof(GUI_QUERY_WINDOW_RPL);
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// IsWindowSlave
//---------------------------------------------------------------------------


ULONG GuiServer::IsWindowSlave(SlaveArgs *args)
{
    GUI_IS_WINDOW_REQ *req = (GUI_IS_WINDOW_REQ *)args->req_buf;
    GUI_IS_WINDOW_RPL *rpl = (GUI_IS_WINDOW_RPL *)args->rpl_buf;

    if (args->req_len != sizeof(GUI_IS_WINDOW_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    SetLastError(req->error);
    HWND hwnd = (HWND)(LONG_PTR)req->hwnd;
    rpl->window = (BOOLEAN)IsWindow(hwnd);
    rpl->error = GetLastError();
    if (rpl->window) {
        rpl->visible = (BOOLEAN)IsWindowVisible(hwnd);
        rpl->enabled = (BOOLEAN)IsWindowEnabled(hwnd);
        rpl->unicode = (BOOLEAN)IsWindowUnicode(hwnd);
        rpl->iconic  = (BOOLEAN)IsIconic(hwnd);
        rpl->zoomed  = (BOOLEAN)IsZoomed(hwnd);
    } else {
        rpl->visible = FALSE;
        rpl->enabled = FALSE;
        rpl->unicode = FALSE;
        rpl->iconic  = FALSE;
        rpl->zoomed  = FALSE;
    }

    args->rpl_len = sizeof(GUI_IS_WINDOW_RPL);
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// GetWindowLongSlave
//---------------------------------------------------------------------------


ULONG GuiServer::GetWindowLongSlave(SlaveArgs *args)
{
    GUI_GET_WINDOW_LONG_REQ *req = (GUI_GET_WINDOW_LONG_REQ *)args->req_buf;
    GUI_GET_WINDOW_LONG_RPL *rpl = (GUI_GET_WINDOW_LONG_RPL *)args->rpl_buf;

    if (args->req_len != sizeof(GUI_GET_WINDOW_LONG_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    SetLastError(req->error);
    HWND hwnd = (HWND)(LONG_PTR)req->hwnd;

    if (req->which == 'clpw')           // ClassLongPtrW
        rpl->result = GetClassLongPtrW(hwnd, req->index);

    else if (req->which == 'clpa')      // ClassLongPtrA
        rpl->result = GetClassLongPtrA(hwnd, req->index);

    else if (req->which == 'cl w')      // ClassLongW
        rpl->result = GetClassLongW(hwnd, req->index);

    else if (req->which == 'cl a')      // ClassLongA
        rpl->result = GetClassLongA(hwnd, req->index);

    else if (req->which == 'wlpw')      // WindowLongPtrW
        rpl->result = GetWindowLongPtrW(hwnd, req->index);

    else if (req->which == 'wlpa')      // WindowLongPtrA
        rpl->result = GetWindowLongPtrA(hwnd, req->index);

    else if (req->which == 'wl w')      // WindowLongW
        rpl->result = GetWindowLongW(hwnd, req->index);

    else if (req->which == 'wl a')      // WindowLongA
        rpl->result = GetWindowLongA(hwnd, req->index);

    else
        return STATUS_INVALID_PARAMETER;

    rpl->error = GetLastError();

    args->rpl_len = sizeof(GUI_GET_WINDOW_LONG_RPL);
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// GetWindowPropSlave
//---------------------------------------------------------------------------


ULONG GuiServer::GetWindowPropSlave(SlaveArgs *args)
{
    GUI_GET_WINDOW_PROP_REQ *req = (GUI_GET_WINDOW_PROP_REQ *)args->req_buf;
    GUI_GET_WINDOW_PROP_RPL *rpl = (GUI_GET_WINDOW_PROP_RPL *)args->rpl_buf;

    if (args->req_len != sizeof(GUI_GET_WINDOW_PROP_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    HWND hwnd = (HWND)(LONG_PTR)req->hwnd;

    WCHAR *prop;
    if (req->prop_is_text) {
        req->prop_text[RTL_NUMBER_OF_V1(req->prop_text) - 1] = L'\0';
        prop = req->prop_text;
    } else
        prop = (WCHAR *)(ULONG_PTR)req->prop_atom;

    SetLastError(req->error);

    if (req->unicode)
        rpl->result = (ULONG64)GetPropW(hwnd, prop);

    else
        rpl->result = (ULONG64)GetPropA(hwnd, (char *)prop);

    rpl->error = GetLastError();

    args->rpl_len = sizeof(GUI_GET_WINDOW_PROP_RPL);
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// GetWindowHandleSlave
//---------------------------------------------------------------------------


ULONG GuiServer::GetWindowHandleSlave(SlaveArgs *args)
{
    GUI_GET_WINDOW_HANDLE_REQ *req =
                                (GUI_GET_WINDOW_HANDLE_REQ *)args->req_buf;
    GUI_GET_WINDOW_HANDLE_RPL *rpl =
                                (GUI_GET_WINDOW_HANDLE_RPL *)args->rpl_buf;
    HWND hwnd = (HWND)(LONG_PTR)req->hwnd;

    if (args->req_len != sizeof(GUI_GET_WINDOW_HANDLE_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    SetLastError(req->error);

    if (req->which == 'desk')
        hwnd = GetDesktopWindow();

    else if (req->which == 'shel')
        hwnd = GetShellWindow();

    else if (req->which == 'prnt')
        hwnd = GetParent(hwnd);

    else if (req->which < 0x10)
        hwnd = GetWindow(hwnd, (req->which & 0x0F));

    else
        return STATUS_INVALID_PARAMETER;

    rpl->error = GetLastError();
    rpl->hwnd = (ULONG)(ULONG_PTR)hwnd;

    args->rpl_len = sizeof(GUI_GET_WINDOW_HANDLE_RPL);
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// GetClassNameSlave
//---------------------------------------------------------------------------


ULONG GuiServer::GetClassNameSlave(SlaveArgs *args)
{
    GUI_GET_CLASS_NAME_REQ *req = (GUI_GET_CLASS_NAME_REQ *)args->req_buf;
    GUI_GET_CLASS_NAME_RPL *rpl = (GUI_GET_CLASS_NAME_RPL *)args->rpl_buf;

    if (args->req_len != sizeof(GUI_GET_CLASS_NAME_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    if (req->maxlen >= 2048)
        return STATUS_INVALID_PARAMETER;

    SetLastError(req->error);
    HWND hwnd = (HWND)(LONG_PTR)req->hwnd;

    if (req->unicode) {

        rpl->result = GetClassNameW(hwnd, rpl->name, req->maxlen);
        args->rpl_len = (rpl->result + 1) * sizeof(WCHAR);

    } else {

        char *name = (char *)rpl->name;
        rpl->result = GetClassNameA(hwnd, name, req->maxlen);
        args->rpl_len = rpl->result + 1;
    }

    rpl->error = GetLastError();

    args->rpl_len += sizeof(GUI_GET_CLASS_NAME_RPL);
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// GetWindowRectSlave
//---------------------------------------------------------------------------


ULONG GuiServer::GetWindowRectSlave(SlaveArgs *args)
{
    GUI_GET_WINDOW_RECT_REQ *req = (GUI_GET_WINDOW_RECT_REQ *)args->req_buf;
    GUI_GET_WINDOW_RECT_RPL *rpl = (GUI_GET_WINDOW_RECT_RPL *)args->rpl_buf;

    if (args->req_len != sizeof(GUI_GET_WINDOW_RECT_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    SetLastError(req->error);
    HWND hwnd = (HWND)(LONG_PTR)req->hwnd;

    if (req->which == 'wind') {

        rpl->result = GetWindowRect(hwnd, &rpl->rect);

    } else if (req->which == 'clnt') {

        rpl->result = GetClientRect(hwnd, &rpl->rect);

    } else
        return STATUS_INVALID_PARAMETER;

    rpl->error = GetLastError();

    args->rpl_len = sizeof(GUI_GET_WINDOW_RECT_RPL);
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// GetWindowInfoSlave
//---------------------------------------------------------------------------


ULONG GuiServer::GetWindowInfoSlave(SlaveArgs *args)
{
    GUI_GET_WINDOW_INFO_REQ *req = (GUI_GET_WINDOW_INFO_REQ *)args->req_buf;
    GUI_GET_WINDOW_INFO_RPL *rpl = (GUI_GET_WINDOW_INFO_RPL *)args->rpl_buf;

    if (args->req_len != sizeof(GUI_GET_WINDOW_INFO_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    SetLastError(req->error);
    HWND hwnd = (HWND)(LONG_PTR)req->hwnd;

    rpl->result = GetWindowInfo(hwnd, &rpl->info);
    rpl->error = GetLastError();

    args->rpl_len = sizeof(GUI_GET_WINDOW_INFO_RPL);
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// GrantHandleSlave
//---------------------------------------------------------------------------


ULONG GuiServer::GrantHandleSlave(SlaveArgs *args)
{
    if (args->req_len != sizeof(GUI_GRANT_HANDLE_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    GUI_GRANT_HANDLE_REQ *req = (GUI_GRANT_HANDLE_REQ *)args->req_buf;
    LONG_PTR user_handle = (LONG_PTR)req->handle_value;

    HANDLE hJobObject = GetJobObjectForGrant(args->pid);
    if (! hJobObject)
        return STATUS_INVALID_CID;

    if (req->handle_type == 1) {            // bitmap/cursor/icon

        ICONINFO icon_info;
        if (! GetIconInfo((HICON)user_handle, &icon_info))
            return STATUS_OBJECT_TYPE_MISMATCH;

    } else
        return STATUS_INVALID_PARAMETER;

    if (! UserHandleGrantAccess((HANDLE)user_handle, hJobObject, TRUE))
        return STATUS_UNSUCCESSFUL;

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// EnumWindowsSlave
//---------------------------------------------------------------------------


ULONG GuiServer::EnumWindowsSlave(SlaveArgs *args)
{
    GUI_ENUM_WINDOWS_REQ *req = (GUI_ENUM_WINDOWS_REQ *)args->req_buf;
    GUI_ENUM_WINDOWS_RPL *rpl = (GUI_ENUM_WINDOWS_RPL *)args->rpl_buf;

    if (args->req_len != sizeof(GUI_ENUM_WINDOWS_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    rpl->num_hwnds = 0;

    if (req->which == 'W') {

        rpl->ok = EnumWindows(
                    EnumWindowsSlaveEnumProc, (LPARAM)&rpl->num_hwnds);

    } else if (req->which == 'C') {

        HWND hwnd = (HWND)(LONG_PTR)req->arg;
        rpl->ok = EnumChildWindows(hwnd,
                    EnumWindowsSlaveEnumProc, (LPARAM)&rpl->num_hwnds);

    } else if (req->which == 'T') {

        ULONG thread_id = req->arg;
        rpl->ok = EnumThreadWindows(thread_id,
                    EnumWindowsSlaveEnumProc, (LPARAM)&rpl->num_hwnds);

    } else
        return STATUS_INVALID_PARAMETER;

    if ((! rpl->ok) && rpl->num_hwnds == -1)
        return STATUS_DISK_FULL;
    rpl->error = GetLastError();

    ULONG status = EnumWindowsFilterSlave(args->pid, rpl);
    if (status != STATUS_SUCCESS)
        return status;

    args->rpl_len = sizeof(GUI_ENUM_WINDOWS_RPL)
                  + (ULONG)rpl->num_hwnds * sizeof(ULONG);

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// EnumWindowsSlaveEnumProc
//---------------------------------------------------------------------------


BOOL GuiServer::EnumWindowsSlaveEnumProc(HWND hwnd, LPARAM lParam)
{
    ULONG max_hwnds = (MAX_RPL_BUF_SIZE - sizeof(GUI_ENUM_WINDOWS_RPL))
                    / sizeof(ULONG);
    ULONG *buf = (ULONG *)lParam;
    ULONG num_hwnds = (ULONG)buf[0];
    if (num_hwnds >= max_hwnds) {
        buf[0] = -1;
        return FALSE;
    }
    ++num_hwnds;
    buf[num_hwnds] = (ULONG)(ULONG_PTR)hwnd;
    buf[0] = num_hwnds;
    return TRUE;
}


//---------------------------------------------------------------------------
// EnumWindowsFilterSlave
//---------------------------------------------------------------------------


ULONG GuiServer::EnumWindowsFilterSlave(ULONG pid, void *rpl_buf)
{
    GUI_ENUM_WINDOWS_RPL *rpl = (GUI_ENUM_WINDOWS_RPL *)rpl_buf;
    if (! rpl->num_hwnds)
        return 0;

    WCHAR boxname[48];
    ULONG status = SbieApi_QueryProcess((HANDLE)(ULONG_PTR)pid,
                                        boxname, NULL, NULL, NULL);
    if (status != 0)
        return status;

    POOL *pool;
    LIST *list;
    status = GetProcessPathList(pid, (void **)&pool, &list);
    if (status != 0)
        return status;

    //
    // for each hwnd in the reply, check if this is a window that should be
    // visible to the calling process, and remove it from the reply if not
    //

    ULONG n = 0;
    for (ULONG i = 0; i < rpl->num_hwnds; ++i) {
        HWND hwnd = (HWND)(LONG_PTR)rpl->hwnds[i];
        if (CheckWindowAccessible(pid, boxname, list, hwnd)) {
            rpl->hwnds[n] = (ULONG)(ULONG_PTR)hwnd;
            ++n;
        }
    }
    rpl->num_hwnds = n;

    Pool_Delete(pool);

    return 0;
}


//---------------------------------------------------------------------------
// FindWindowSlave
//---------------------------------------------------------------------------


ULONG GuiServer::FindWindowSlave(SlaveArgs *args)
{
    GUI_FIND_WINDOW_REQ *req = (GUI_FIND_WINDOW_REQ *)args->req_buf;
    GUI_FIND_WINDOW_RPL *rpl = (GUI_FIND_WINDOW_RPL *)args->rpl_buf;

    if (args->req_len < sizeof(GUI_FIND_WINDOW_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    HWND hwnd_parent = (HWND)(LONG_PTR)req->hwnd_parent;
    HWND hwnd_child  = (HWND)(LONG_PTR)req->hwnd_child;

    WCHAR *class_w;
    if (req->class_is_text) {
        req->class_text[RTL_NUMBER_OF_V1(req->class_text) - 1] = L'\0';
        class_w = req->class_text;
    } else
        class_w = (WCHAR *)(ULONG_PTR)req->class_atom;
    char *class_a = (char *)class_w;

    WCHAR *title_w;
    if (req->title_is_text) {
        req->title_text[RTL_NUMBER_OF_V1(req->title_text) - 1] = L'\0';
        title_w = req->title_text;
    } else
        title_w = NULL;
    char *title_a = (char *)title_w;

    HWND hwnd = NULL;

    if (req->which == 'fw w')
        hwnd = FindWindowW(class_w, title_w);

    else if (req->which == 'fw a')
        hwnd = FindWindowA(class_a, title_a);

    else if (req->which == 'fwxw')
        hwnd = FindWindowExW(hwnd_parent, hwnd_child, class_w, title_w);

    else if (req->which == 'fwxa')
        hwnd = FindWindowExA(hwnd_parent, hwnd_child, class_a, title_a);

    else
        return STATUS_INVALID_PARAMETER;

    rpl->hwnd = (ULONG)(ULONG_PTR)hwnd;
    rpl->error = GetLastError();

    args->rpl_len = sizeof(GUI_FIND_WINDOW_RPL);
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// MapWindowPointsSlave
//---------------------------------------------------------------------------


ULONG GuiServer::MapWindowPointsSlave(SlaveArgs *args)
{
    GUI_MAP_WINDOW_POINTS_REQ *req =
                                (GUI_MAP_WINDOW_POINTS_REQ *)args->req_buf;
    GUI_MAP_WINDOW_POINTS_RPL *rpl =
                                (GUI_MAP_WINDOW_POINTS_RPL *)args->rpl_buf;

    if (args->req_len < sizeof(GUI_MAP_WINDOW_POINTS_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    if (! req->num_pts)
        return STATUS_INVALID_PARAMETER;

    ULONG rpl_len = sizeof(GUI_MAP_WINDOW_POINTS_RPL);

    HWND hwnd_from = (HWND)(LONG_PTR)req->hwnd_from;
    HWND hwnd_to = (HWND)(LONG_PTR)req->hwnd_to;

    SetLastError(ERROR_SUCCESS);

    if (req->num_pts == (0xFF000000 | 'c2s')) {

        //
        // ClientToScreen
        //

        rpl->num_pts = 1;
        rpl->pts[0] = req->pts[0];

        rpl->retval = ClientToScreen(hwnd_from, &rpl->pts[0]);

    } else if (req->num_pts == (0xFF000000 | 's2c')) {

        //
        // ScreenToClient
        //

        rpl->num_pts = 1;
        rpl->pts[0] = req->pts[0];

        rpl->retval = ScreenToClient(hwnd_to, &rpl->pts[0]);

    } else {

        //
        // MapWindowPoints
        //

        ULONG max_points =
                (MAX_RPL_BUF_SIZE - sizeof(GUI_MAP_WINDOW_POINTS_RPL))
              / sizeof(POINT);
        if (req->num_pts > max_points)
            return STATUS_INVALID_PARAMETER;

        rpl_len += req->num_pts * sizeof(POINT);

        rpl->num_pts = (USHORT)req->num_pts;
        memcpy(rpl->pts, req->pts, req->num_pts * sizeof(POINT));

        rpl->retval = MapWindowPoints(hwnd_from, hwnd_to,
                                      rpl->pts, rpl->num_pts);
    }

    rpl->error = GetLastError();
    args->rpl_len = rpl_len;
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// SetWindowPosSlave
//---------------------------------------------------------------------------


ULONG GuiServer::SetWindowPosSlave(SlaveArgs *args)
{
    GUI_SET_WINDOW_POS_REQ *req = (GUI_SET_WINDOW_POS_REQ *)args->req_buf;
    GUI_SET_WINDOW_POS_RPL *rpl = (GUI_SET_WINDOW_POS_RPL *)args->rpl_buf;

    if (args->req_len < sizeof(GUI_SET_WINDOW_POS_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    HWND hwnd = (HWND)(LONG_PTR)req->hwnd;
    HWND hwnd_insert_after = (HWND)(LONG_PTR)req->hwnd_insert_after;

    //
    // check access according to OpenWinClass rules
    //

    POOL *pool;
    LIST *list;
    ULONG status = GetProcessPathList(args->pid, (void **)&pool, &list);
    if (status != 0)
        return status;
    bool access = CheckWindowAccessible(args->pid, NULL, list, hwnd);
    Pool_Delete(pool);

    if (access) {

        //
        // check access according to integrity level
        //

        if (! CompareIntegrityLevels(args->pid, hwnd))
            access = false;
    }

    if (! access) {

        rpl->error = ERROR_INVALID_WINDOW_HANDLE;
        rpl->retval = FALSE;

    } else {

        SetLastError(req->error);
        rpl->retval = SetWindowPos(hwnd, hwnd_insert_after, req->x, req->y,
                                   req->w, req->h, req->flags);
        rpl->error = GetLastError();
    }

    args->rpl_len = sizeof(GUI_SET_WINDOW_POS_RPL);
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// CloseClipboardSlave
//---------------------------------------------------------------------------


ULONG GuiServer::CloseClipboardSlave(SlaveArgs *args)
{
    static ULONG _ClipboardSequence = -1;

    //
    // this request is used on Windows Vista and later where we need to
    // adjust the internal integrity level numbers stored in the clipboard.
    // see Gui_CloseClipboard in core/dll/gui.c, and also see core/drv/gui.c
    //

    ULONG status = 0;

    if (args->req_len != sizeof(ULONG))
        status = STATUS_INFO_LENGTH_MISMATCH;

    else if (GetClipboardSequenceNumber() != _ClipboardSequence) {

        ULONG caller_il = (ULONG)(ULONG_PTR)SbieApi_QueryProcessInfo(
                                    (HANDLE)(ULONG_PTR)args->pid, 'pril');
        if (caller_il == tzuk) // Windows XP
            status = STATUS_WRONG_PASSWORD;
        else {

            status = STATUS_SHARING_VIOLATION; // in case OpenClipboard fails

            for (int retry = 0; retry < 5 * (1000 / 50); ++retry) {

                if (OpenClipboard(NULL)) {

                    //
                    // some data may have been placed on the clipboard with
                    // delayed rendering with SetClipboardData(fmt, NULL).
                    // when this data is requested, the clipboard owner will
                    // issue a second SetClipboardData with the actual data,
                    // but this will reset the IL for the clipboard item.
                    //
                    // to work around this, we need to force rendering now,
                    // to adjust the IL on the rendered clipboard item now.
                    // we also need to first set the IL to 0x4000 otherwise
                    // this process will fail to access clipboard items.
                    //

                    ULONG fmt = 0;

                    while (1) {
                        status = SbieApi_CallOne(API_GUI_CLIPBOARD, 0x4000);
                        if (status != 0)
                            break;
                        fmt = EnumClipboardFormats(fmt);
                        if (! fmt)
                            break;
                        GetClipboardData(fmt);
                    }

                    /*if (status == 0) {

                        //
                        // now that any delayed rendering has been completed
                        // and the clipboard is finalized, adjust IL again
                        // according to the process integrity level
                        //
                        // removed in version 4.03.02:  win32k!FindClipFormat
                        // does access check the following way:
                        //
                        // allow if (clip item IL + 0x1000) <= process IL
                        //
                        // if we restore to caller_il which is 0x2000 then
                        // 0x3000 always <= 0x4000 and GetClipboardDataSlave
                        // always fails.  so we want clip_il to stay 0x4000
                        //

                        status = SbieApi_CallOne(
                                        API_GUI_CLIPBOARD, caller_il);
                    }*/

                    CloseClipboard();

                    _ClipboardSequence = GetClipboardSequenceNumber();
                    if (! _ClipboardSequence)
                        _ClipboardSequence = -1;

                    break;
                }

                Sleep(50);
            }

            if (status != 0)
                SbieApi_Log(2205, L"CloseClipboard %08X", status);
        }
    }

    *(ULONG *)args->rpl_buf = status;
    args->rpl_len = sizeof(ULONG);
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// GetClipboardDataSlave
//---------------------------------------------------------------------------


ULONG GuiServer::GetClipboardDataSlave(SlaveArgs *args)
{
    GUI_GET_CLIPBOARD_DATA_REQ *req =
                                (GUI_GET_CLIPBOARD_DATA_REQ *)args->req_buf;
    GUI_GET_CLIPBOARD_DATA_RPL *rpl =
                                (GUI_GET_CLIPBOARD_DATA_RPL *)args->rpl_buf;

    //todo:  fail if the calling process should not have clipboard access

    if (args->req_len != sizeof(GUI_GET_CLIPBOARD_DATA_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    rpl->result = 0;

    EnterCriticalSection(&m_SlavesLock);

    if (! OpenClipboard(NULL))
        rpl->error = GetLastError();
    else {

        HANDLE mem_handle = GetClipboardData(req->format);
        rpl->error = GetLastError();
        if (! mem_handle) {
            //
            // if the clipboard item has integrity level 0x2000 or less
            // then we can't get it, see more in CloseClipboardSlave above.
            // work around that by setting IL to 0x4000
            //
            if (SbieApi_CallOne(API_GUI_CLIPBOARD, 0x4000) == 0) {
                mem_handle = GetClipboardData(req->format);
                rpl->error = GetLastError();
            }
        }

        if (mem_handle && (req->format == CF_BITMAP))
            mem_handle = GetClipboardBitmapSlave(mem_handle);

        if (mem_handle && (req->format == CF_ENHMETAFILE))
            mem_handle = GetClipboardEnhMetaFileSlave(mem_handle);

        if (mem_handle) {

            void *mem_ptr = GlobalLock(mem_handle);
            if (! mem_ptr)
                rpl->error = GetLastError();
            else {
                SIZE_T mem_len = GlobalSize(mem_handle);

                rpl->section_length = mem_len;
                rpl->section_handle = (ULONG64)(ULONG_PTR)
                    GetClipboardDataSlave2(args->pid, mem_ptr, mem_len);

                if (rpl->section_handle) {
                    rpl->result = 1;
                    rpl->error = 0;
                } else
                    rpl->error = GetLastError();

                GlobalUnlock(mem_handle);

                if (req->format == CF_BITMAP || 
                    req->format == CF_ENHMETAFILE) {
                    // was allocated by GetClipboardBitmapSlave/EnhMetaFile, so free it
                    GlobalFree(mem_handle);
                }
            }
        }

        CloseClipboard();
    }

    LeaveCriticalSection(&m_SlavesLock);

    args->rpl_len = sizeof(GUI_GET_CLIPBOARD_DATA_RPL);
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// GetClipboardDataSlave2
//---------------------------------------------------------------------------


HANDLE GuiServer::GetClipboardDataSlave2(
    ULONG pid, void *mem_ptr, SIZE_T mem_len)
{
    HANDLE hProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, pid);
    if (! hProcess)
        return 0;

#ifdef _WIN64
    ULONG size_high = (ULONG)(mem_len >> 32);
    ULONG size_low = (ULONG)(mem_len & 0xFFFFFFFF);
#else
    ULONG size_high = 0;
    ULONG size_low = mem_len;
#endif _WIN64

    HANDLE hSectionRemote = NULL;
    HANDLE hSectionLocal = CreateFileMapping(INVALID_HANDLE_VALUE, NULL,
                                             PAGE_READWRITE | SEC_COMMIT,
                                             size_high, size_low, NULL);
    if (hSectionLocal) {

        void *data =
            MapViewOfFile(hSectionLocal, FILE_MAP_WRITE, 0, 0, mem_len);

        if (data) {

            memcpy(data, mem_ptr, mem_len);
            UnmapViewOfFile(data);

            if (! DuplicateHandle(NtCurrentProcess(), hSectionLocal,
                                  hProcess, &hSectionRemote,
                                  SECTION_QUERY | SECTION_MAP_READ,
                                  FALSE, 0)) {

                hSectionRemote = NULL;
            }
        }
    }

    ULONG LastError = GetLastError();
    if (hSectionLocal)
        CloseHandle(hSectionLocal);
    CloseHandle(hProcess);
    SetLastError(LastError);

    return hSectionRemote;
}


//---------------------------------------------------------------------------
// GetClipboardBitmapSlave
//---------------------------------------------------------------------------


void *GuiServer::GetClipboardBitmapSlave(void *hBitmap)
{
    //
    // see Gui_GetClipboardData_BMP in core/dll/guimisc.c
    //

    UCHAR *buf = NULL;

    HDC hdc = GetDC(NULL);

    if (hdc)
    {
        HDC hdcMem = CreateCompatibleDC(hdc);

        if (hdcMem)
        {
            BITMAPINFO bitmapInfo = { 0 };
            bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);

            if (GetDIBits(hdcMem, (HBITMAP)hBitmap, 0, 0, NULL, &bitmapInfo, DIB_RGB_COLORS))
            {
                buf = (UCHAR *)GlobalAlloc(GMEM_FIXED, bitmapInfo.bmiHeader.biSizeImage + 128);

                if (buf)
                {
                    bitmapInfo.bmiHeader.biCompression = BI_RGB;
                    memzero(buf, 128);
                    memcpy(buf, &bitmapInfo, sizeof(bitmapInfo.bmiHeader));
                    GetDIBits(hdcMem, (HBITMAP)hBitmap, 0, bitmapInfo.bmiHeader.biHeight, buf + 128, &bitmapInfo, DIB_RGB_COLORS);
                }
            }
            DeleteDC(hdcMem);
        }

        ReleaseDC(NULL, hdc);
    }

    return buf;
}


//---------------------------------------------------------------------------
// GetClipboardEnhMetaFileSlave
//---------------------------------------------------------------------------


void *GuiServer::GetClipboardEnhMetaFileSlave(void *hEnhMetaFile)
{
    //
    // see Gui_GetClipboardData_EMF in core/dll/guimisc.c
    //

    UCHAR *pBuffer = NULL;

    if (hEnhMetaFile) {

        DWORD dwMemSize = GetEnhMetaFileBits((HENHMETAFILE)hEnhMetaFile, 0, NULL);
        pBuffer = (UCHAR*)GlobalAlloc(GMEM_FIXED, dwMemSize);
        if (pBuffer)
            GetEnhMetaFileBits((HENHMETAFILE)hEnhMetaFile, dwMemSize, (LPBYTE)pBuffer);
    }

    return pBuffer;
}


//---------------------------------------------------------------------------
// GetClipboardMetaFileSlave
//---------------------------------------------------------------------------


ULONG GuiServer::GetClipboardMetaFileSlave(SlaveArgs *args)
{
    //
    // see Gui_GetClipboardData_MF in core/dll/guimisc.c
    //

    GUI_GET_CLIPBOARD_DATA_REQ *req =
                                (GUI_GET_CLIPBOARD_DATA_REQ *)args->req_buf;
    GUI_GET_CLIPBOARD_DATA_RPL *rpl =
                                (GUI_GET_CLIPBOARD_DATA_RPL *)args->rpl_buf;

    //todo:  fail if the calling process should not have clipboard access

    if (args->req_len != sizeof(GUI_GET_CLIPBOARD_DATA_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    if (req->format != CF_METAFILEPICT)
        return STATUS_INVALID_PARAMETER;

    rpl->result = 0;
    rpl->error = 0;

    EnterCriticalSection(&m_SlavesLock);

    if (OpenClipboard(NULL)) {

        HANDLE mem_handle = GetClipboardData(req->format);
        if (mem_handle) {

            if (GlobalSize(mem_handle) >= sizeof(METAFILEPICT)) {

                METAFILEPICT *mf = (METAFILEPICT *)GlobalLock(mem_handle);
                if (mf) {

                    ULONG mem_len = GetMetaFileBitsEx(mf->hMF, 0, NULL);
                    if (mem_len) {

                        void *mem_ptr =
                                HeapAlloc(GetProcessHeap(), 0, mem_len);
                        if (mem_ptr) {

                            if (GetMetaFileBitsEx(mf->hMF, mem_len, mem_ptr)
                                    == mem_len) {

                                rpl->section_length = mem_len;
                                rpl->section_handle = (ULONG64)(ULONG_PTR)
                                    GetClipboardDataSlave2(
                                        args->pid, mem_ptr, mem_len);

                                if (rpl->section_handle)
                                    rpl->result = 1;
                            }

                            HeapFree(GetProcessHeap(), 0, mem_ptr);
                        }
                    }

                    GlobalUnlock(mem_handle);
                }
            }
        }

        CloseClipboard();
    }

    LeaveCriticalSection(&m_SlavesLock);

    args->rpl_len = sizeof(GUI_GET_CLIPBOARD_DATA_RPL);
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// SendPostMessageSlave
//---------------------------------------------------------------------------


ULONG GuiServer::SendPostMessageSlave(SlaveArgs *args)
{
    GUI_SEND_POST_MESSAGE_REQ *req =
                                (GUI_SEND_POST_MESSAGE_REQ *)args->req_buf;
    GUI_SEND_POST_MESSAGE_RPL *rpl =
                                (GUI_SEND_POST_MESSAGE_RPL *)args->rpl_buf;

    if (args->req_len != sizeof(GUI_SEND_POST_MESSAGE_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    if (req->hwnd == 0xFFFF || req->hwnd == 0xFFFFFFFF)
        return STATUS_ACCESS_DENIED;

    HWND hwnd = (HWND)(LONG_PTR)req->hwnd;
    UINT msg  = req->msg;
    WPARAM wparam = (WPARAM)req->wparam;
    LPARAM lparam = (LPARAM)req->lparam;

    //
    // WM_DDE_ACK
    //

    if (msg == WM_DDE_ACK &&
            (   req->which == 'sm w' || req->which == 'sm a'
             || req->which == 'smtw' || req->which == 'smta')) {

        //
        // when a sandboxed process sends an WM_DDE_ACK, it is the start of
        // a DDE conversation.  to work around the IL bug in DDE (see
        // core/dll/guidde.c), we use a proxy middleman window.
        //

        if ((req->which == 'sm w') || (req->which == 'sm a')
                                                    && IsWindow(hwnd)) {

            rpl->lresult1 = 0;
            rpl->lresult2 = 0;

            ULONG_PTR *DdeArgs = (ULONG_PTR *)
                HeapAlloc(GetProcessHeap(), 0, (sizeof(ULONG_PTR) * 4));
            if (DdeArgs) {

                DdeArgs[0] = (ULONG_PTR)hwnd;
                DdeArgs[1] = (ULONG_PTR)wparam;
                DdeArgs[2] = (ULONG_PTR)lparam;

                ULONG tid;
                HANDLE hThread = CreateThread(
                    NULL, 0, DdeProxyThreadSlave, (void *)DdeArgs, 0, &tid);
                if (hThread) {

                    CloseHandle(hThread);
                    rpl->lresult1 = 1;
                    rpl->error = 0;
                }
            }

            if (! rpl->lresult1)
                rpl->error = GetLastError();
            args->rpl_len = sizeof(GUI_SEND_POST_MESSAGE_RPL);
            return STATUS_SUCCESS;
        }
    }

    //
    // check access according to OpenWinClass rules
    //

    POOL *pool;
    LIST *list;
    ULONG status = GetProcessPathList(args->pid, (void **)&pool, &list);
    if (status != 0)
        return status;
    bool access = CheckWindowAccessible(args->pid, NULL, list, hwnd);
    Pool_Delete(pool);

    if (access) {

        //
        // check access according to integrity level
        //

        if ((! CompareIntegrityLevels(args->pid, hwnd))
                    && (! ShouldIgnoreIntegrityLevels(args->pid, hwnd)))
            access = false;

        else {

            bool IsSendMsg = true;
            if (req->which == 'pm w' || req->which == 'pm a')
                IsSendMsg = false;

            if (! AllowSendPostMessage(args->pid, msg, IsSendMsg, hwnd))
                access = false;
        }
    }

    rpl->lresult2 = 0;

    if (! access) {
        rpl->lresult1 = 0;
        rpl->error = ERROR_INVALID_WINDOW_HANDLE;
        args->rpl_len = sizeof(GUI_SEND_POST_MESSAGE_RPL);
        return STATUS_SUCCESS;
    }

    //
    // invoke appropriate message send/post API
    //

    if (req->which == 'pm w')
        rpl->lresult1 = PostMessageW(hwnd, msg, wparam, lparam);
    else if (req->which == 'pm a')
        rpl->lresult1 = PostMessageA(hwnd, msg, wparam, lparam);

    else if (req->which == 'sm w')
        rpl->lresult1 = SendMessageW(hwnd, msg, wparam, lparam);
    else if (req->which == 'sm a')
        rpl->lresult1 = SendMessageA(hwnd, msg, wparam, lparam);

    else if (req->which == 'snmw')
        rpl->lresult1 = SendNotifyMessageW(hwnd, msg, wparam, lparam);
    else if (req->which == 'snma')
        rpl->lresult1 = SendNotifyMessageA(hwnd, msg, wparam, lparam);

    else if (req->which == 'smtw') {
        DWORD_PTR x;
        rpl->lresult1 = SendMessageTimeoutW(hwnd, msg, wparam, lparam,
                                            req->flags, req->timeout, &x);
        rpl->lresult2 = x;

    } else if (req->which == 'smta') {
        DWORD_PTR x;
        rpl->lresult1 = SendMessageTimeoutW(hwnd, msg, wparam, lparam,
                                            req->flags, req->timeout, &x);
        rpl->lresult2 = x;

    } else
        return STATUS_INVALID_PARAMETER;

    rpl->error = GetLastError();

    args->rpl_len = sizeof(GUI_SEND_POST_MESSAGE_RPL);
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// SendCopyDataSlave
//---------------------------------------------------------------------------


ULONG GuiServer::SendCopyDataSlave(SlaveArgs *args)
{
    GUI_SEND_COPYDATA_REQ *req = (GUI_SEND_COPYDATA_REQ *)args->req_buf;
    GUI_SEND_COPYDATA_RPL *rpl = (GUI_SEND_COPYDATA_RPL *)args->rpl_buf;

    if (args->req_len < sizeof(GUI_SEND_COPYDATA))
        return STATUS_INFO_LENGTH_MISMATCH;

    if (req->cds_len > 1024*1024)
        return STATUS_INFO_LENGTH_MISMATCH;

    ULONG max_offset = FIELD_OFFSET(GUI_SEND_COPYDATA_REQ, cds_buf)
                     + req->cds_len;
    if (max_offset > args->req_len)
        return STATUS_INFO_LENGTH_MISMATCH;

    if (req->hwnd == 0xFFFF || req->hwnd == 0xFFFFFFFF)
        return STATUS_ACCESS_DENIED;

    COPYDATASTRUCT cds;
    cds.dwData = (ULONG_PTR)req->cds_key;
    cds.cbData = req->cds_len;
    cds.lpData = req->cds_buf;

    HWND hwnd = (HWND)(LONG_PTR)req->hwnd;
    const UINT msg = WM_COPYDATA;
    WPARAM wparam = (WPARAM)(ULONG_PTR)req->wparam;
    LPARAM lparam = (LPARAM)(ULONG_PTR)&cds;

    //
    // check access according to OpenWinClass rules
    //

    POOL *pool;
    LIST *list;
    ULONG status = GetProcessPathList(args->pid, (void **)&pool, &list);
    if (status != 0)
        return status;
    bool access = CheckWindowAccessible(args->pid, NULL, list, hwnd);
    Pool_Delete(pool);

    if (access) {

        //
        // check access according to integrity level,
        // but always allow copying data to the taskbar
        //

        if ((! CompareIntegrityLevels(args->pid, hwnd))
            && (! ShouldIgnoreIntegrityLevels(args->pid, hwnd))) {

            HWND hShellTrayWnd = FindWindow(L"Shell_TrayWnd", NULL);
            if (hwnd != hShellTrayWnd) {

                access = false;
            }
        }
    }

    rpl->lresult2 = 0;

    if (! access) {
        rpl->lresult1 = 0;
        rpl->error = ERROR_INVALID_WINDOW_HANDLE;
        args->rpl_len = sizeof(GUI_SEND_COPYDATA_RPL);
        return STATUS_SUCCESS;
    }

    //
    // invoke appropriate message send/post API
    //

    if (req->which == 'sm w')
        rpl->lresult1 = SendMessageW(hwnd, msg, wparam, lparam);
    else if (req->which == 'sm a')
        rpl->lresult1 = SendMessageA(hwnd, msg, wparam, lparam);

    else if (req->which == 'smtw') {
        DWORD_PTR x;
        rpl->lresult1 = SendMessageTimeoutW(hwnd, msg, wparam, lparam,
                                            req->flags, req->timeout, &x);
        rpl->lresult2 = x;

    } else if (req->which == 'smta') {
        DWORD_PTR x;
        rpl->lresult1 = SendMessageTimeoutW(hwnd, msg, wparam, lparam,
                                            req->flags, req->timeout, &x);
        rpl->lresult2 = x;

    } else if (req->which == 'dde ') {

        //
        // special case for WM_DDE_DATA
        // see Gui_DDE_DATA_Posting in core/dll/guidde.c for calling code
        //
        // note that in the case of cross-sandbox DDE REQUEST/DATA exchange,
        // we need the WM_DDE_DATA reply to specify the window from
        // DdeProxyThreadSlave, and we use a global variable hack to do that
        //

        rpl->lresult1 = 0;
        rpl->error = ERROR_INVALID_WINDOW_HANDLE;

        HGLOBAL hGlobal =
                    GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, req->cds_len);
        if (hGlobal) {
            void *pGlobal = GlobalLock(hGlobal);
            if (pGlobal) {
                memcpy(pGlobal, req->cds_buf, req->cds_len);
                GlobalUnlock(pGlobal);

                wparam = (WPARAM)DDE_Request_ProxyWnd;

                lparam = PackDDElParam(WM_DDE_DATA,
                                (UINT_PTR)hGlobal, (UINT_PTR)req->cds_key);

                // we can't post WM_DDE_DATA from this thread because the
                // DdeProxyThreadSlave proxy window is in another thread
                rpl->lresult1 = PostMessage(DDE_Request_ProxyWnd,
                                            (WM_USER + 0x123), tzuk, lparam);

                rpl->error = 0;

                //FreeDDElParam(WM_DDE_DATA, lparam);
            }
            //GlobalFree(hGlobal);
        }

    } else
        return STATUS_INVALID_PARAMETER;

    rpl->error = GetLastError();

    args->rpl_len = sizeof(GUI_SEND_COPYDATA_RPL);
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// ClipCursorSlave
//---------------------------------------------------------------------------


ULONG GuiServer::ClipCursorSlave(SlaveArgs *args)
{
    if (args->req_len != sizeof(GUI_CLIP_CURSOR_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    GUI_CLIP_CURSOR_REQ *req = (GUI_CLIP_CURSOR_REQ *)args->req_buf;

    RECT *rect = NULL;
    if (req->have_rect)
        rect = &req->rect;

    ClipCursor(rect); //if (! ) // as this seam to randomly fail don't issue errors
    //    return STATUS_ACCESS_DENIED; // todo: add reply and return ret value

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// SetForegroundWindowSlave
//---------------------------------------------------------------------------


ULONG GuiServer::SetForegroundWindowSlave(SlaveArgs *args)
{
    if (args->req_len != sizeof(GUI_SET_FOREGROUND_WINDOW_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    GUI_SET_FOREGROUND_WINDOW_REQ *req =
                            (GUI_SET_FOREGROUND_WINDOW_REQ *)args->req_buf;

    HWND hwnd = (HWND)(LONG_PTR)req->hwnd;
    SetForegroundWindow(hwnd);

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// MonitorFromWindowSlave
//---------------------------------------------------------------------------


ULONG GuiServer::MonitorFromWindowSlave(SlaveArgs *args)
{
    GUI_MONITOR_FROM_WINDOW_REQ *req =
                                (GUI_MONITOR_FROM_WINDOW_REQ *)args->req_buf;
    GUI_MONITOR_FROM_WINDOW_RPL *rpl =
                                (GUI_MONITOR_FROM_WINDOW_RPL *)args->rpl_buf;

    if (args->req_len != sizeof(GUI_MONITOR_FROM_WINDOW_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    SetLastError(req->error);

    HWND hwnd = (HWND)(LONG_PTR)req->hwnd;
    HMONITOR hmonitor = MonitorFromWindow(hwnd, req->flags);
    rpl->error = GetLastError();
    rpl->retval = (ULONG)(ULONG_PTR)hmonitor;

    args->rpl_len = sizeof(GUI_MONITOR_FROM_WINDOW_RPL);
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// SplWow64Slave
//---------------------------------------------------------------------------


ULONG GuiServer::SplWow64Slave(SlaveArgs *args)
{
    //
    // there can be only one sandboxed SplWow64 process for each session,
    // even if there are multiple sandboxes.  this is discussed in more
    // detail in Ipc_GetName_AdjustSplWow64Path in core/dll/ipc.c.
    //
    // this code manages SplWow64 instances and will terminate a running
    // instance of SplWow64, associated with one sandbox, when a process
    // from another sandbox says it needs its own instance of SplWow64.
    //

    static ULONG   _SplWow64Pid = 0;
    static ULONG64 _SplWow64CreateTime = 0;
    static WCHAR   _SplWow64BoxName[48];

    if (args->req_len != sizeof(GUI_SPLWOW64_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    GUI_SPLWOW64_REQ *req = (GUI_SPLWOW64_REQ *)args->req_buf;

    ULONG status;
    ULONG64 create_time;
    WCHAR boxname[48];

    //
    // scenario 1:  req->set == TRUE
    // new SplWow64 processes registers itself
    //

    if (req->set) {

        status = SbieApi_QueryProcessEx2((HANDLE)(ULONG_PTR)args->pid, 0,
                                    boxname, NULL, NULL, NULL, &create_time);
        if (status != 0) {
            // error, can't get information about new SplWow64 process
            return status;
        }

        _SplWow64Pid = args->pid;
        _SplWow64CreateTime = create_time;
        memcpy(_SplWow64BoxName, boxname, sizeof(_SplWow64BoxName));
        boxname[47] = L'\0';

        return STATUS_SUCCESS;
    }

    //
    // scenario 2:  req->set == FALSE
    // some 32-bit process wants to make sure there isn't an SplWow64
    // process running in some other sandbox
    //

    status = SbieApi_QueryProcessEx2((HANDLE)(ULONG_PTR)_SplWow64Pid, 0,
                                    boxname, NULL, NULL, NULL, &create_time);

    if (status != 0
            || create_time != _SplWow64CreateTime
                    || _wcsicmp(boxname, _SplWow64BoxName) != 0) {

        //
        // either a process with the recorded _SplWow64Pid doesn't exist at
        // this time, or a process does exist for the recorded _SplWow64Pid,
        // but it isn't (any longer?) the process that we think it is
        //

        _SplWow64Pid = 0;
        _SplWow64CreateTime = 0;
        _SplWow64BoxName[0] = L'\0';


        // Fix of ICD-12446 doesn't kill host SplWow64. We my remove the code after ICD-12446 is verified.
        // if (req->win8)
        //    SplWow64SlaveWin8();

        return STATUS_SUCCESS;
    }

    status = SbieApi_QueryProcessEx2((HANDLE)(ULONG_PTR)args->pid, 0,
                                boxname, NULL, NULL, NULL, NULL);

    if (status == 0 && _wcsicmp(boxname, _SplWow64BoxName) != 0) {

        //
        // the caller is running in a different sandbox than the SplWow64
        // so we want to terminate SplWow64
        //

        SbieDll_KillOne(_SplWow64Pid);

        _SplWow64Pid = 0;
        _SplWow64CreateTime = 0;
        _SplWow64BoxName[0] = L'\0';
    }

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// SplWow64SlaveWin8
//---------------------------------------------------------------------------


void GuiServer::SplWow64SlaveWin8()
{
    //
    // on Windows 8, the SplWow64 communication objects always include
    // 2000 in their name, as opposed to 0 in earlier Windows.  this is
    // explained in Ipc_GetName_AdjustSplWow64Path() in core/dll/ipc.c.
    //
    // this means that an existing instance of SplWow64 running outside
    // the sandbox will conflict with printing in the sandbox, because
    // the SplWow64 ports/events/mutexes all have the same name.
    //
    // to work around this, we look for a process SplWow64.exe outside
    // the sandbox and terminate it, whenever a 32-bit program in the
    // sandbox needs to print.
    //

    typedef BOOL (*P_EnumProcesses)(ULONG *pids, ULONG len, ULONG *plen);
    typedef BOOL (*P_QueryFullProcessImageNameW)(
        HANDLE hProcess, ULONG dwFlags, WCHAR *lpExeName, ULONG *lpdwSize);

    HMODULE KernelBase = GetModuleHandle(L"KernelBase.dll");
    P_EnumProcesses pEnumProcesses = (P_EnumProcesses)
        GetProcAddress(KernelBase, "EnumProcesses");
    P_QueryFullProcessImageNameW pQueryFullProcessImageNameW =
        (P_QueryFullProcessImageNameW)
            GetProcAddress(KernelBase, "QueryFullProcessImageNameW");

    if ((! pEnumProcesses) || (! pQueryFullProcessImageNameW))
        return;

    const ULONG max_pids = 16000;
    ULONG pids_len = max_pids * sizeof(ULONG);
    ULONG *pids = (ULONG *)HeapAlloc(GetProcessHeap(), 0, pids_len);
    if (! pids)
        return;

    if (pEnumProcesses(pids, pids_len, &pids_len)) {

        ULONG num_pids = pids_len / sizeof(ULONG);
        WCHAR path[128+2];
        ULONG SessionId;

        for (ULONG i = 0; i < num_pids; ++i) {

            if (ProcessIdToSessionId(pids[i], &SessionId)
                    && (SessionId == m_SessionId)) {

                HANDLE hProcess = OpenProcess(
                        PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION |
                        SYNCHRONIZE, FALSE, pids[i]);

                if (hProcess) {

                    ULONG path_len = 128;
                    if (pQueryFullProcessImageNameW(
                                hProcess, 1, path, &path_len)) {

                        WCHAR *ptr = wcsrchr(path, L'\\');
                        if (ptr && _wcsicmp(ptr + 1, L"splwow64.exe") == 0) {

                            if (SbieApi_QueryProcessInfo(
                                        (HANDLE)(ULONG_PTR)pids[i], 0) == 0) {

                                TerminateProcess(hProcess, 0);
                                WaitForSingleObject(hProcess, 3000);
                            }
                        }
                    }

                    CloseHandle(hProcess);
                }
            }
        }
    }

    HeapFree(GetProcessHeap(), 0, pids);
}


//---------------------------------------------------------------------------
// ChangeDisplaySettingsSlave
//---------------------------------------------------------------------------


ULONG GuiServer::ChangeDisplaySettingsSlave(SlaveArgs *args)
{
    //
    // Windows 8 does not allow a process with a zero integrity level to
    // change display mode, so we have to do it here
    //

    if (args->req_len != sizeof(GUI_CHANGE_DISPLAY_SETTINGS_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    GUI_CHANGE_DISPLAY_SETTINGS_REQ *req =
                            (GUI_CHANGE_DISPLAY_SETTINGS_REQ *)args->req_buf;
    GUI_CHANGE_DISPLAY_SETTINGS_RPL *rpl =
                            (GUI_CHANGE_DISPLAY_SETTINGS_RPL *)args->rpl_buf;

    if (req->flags & ~(CDS_UNKNOWNFLAG | CDS_RESET | CDS_FULLSCREEN | CDS_TEST))
        return STATUS_INVALID_PARAMETER;

    void *devname = (req->have_devname ? req->devname : NULL);
    void *devmode = (req->have_devmode ? &req->devmode : NULL);

    SetLastError(ERROR_SUCCESS);
    if (req->unicode) {
        rpl->retval = ChangeDisplaySettingsExW(
            (LPCWSTR)devname, (DEVMODEW *)devmode, NULL, req->flags, NULL);
    } else {
        rpl->retval = ChangeDisplaySettingsExA(
            (LPCSTR)devname,  (DEVMODEA *)devmode, NULL, req->flags, NULL);
    }
    rpl->error = GetLastError();

    args->rpl_len = sizeof(GUI_CHANGE_DISPLAY_SETTINGS_RPL);
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// SetCursorPosSlave
//---------------------------------------------------------------------------


ULONG GuiServer::SetCursorPosSlave(SlaveArgs *args)
{
    GUI_SET_CURSOR_POS_REQ *req = (GUI_SET_CURSOR_POS_REQ *)args->req_buf;
    GUI_SET_CURSOR_POS_RPL *rpl = (GUI_SET_CURSOR_POS_RPL *)args->rpl_buf;

    if (args->req_len != sizeof(GUI_SET_CURSOR_POS_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    SetLastError(req->error);

    rpl->retval = (ULONG)SetCursorPos(req->x, req->y);
    rpl->error = GetLastError();

    args->rpl_len = sizeof(GUI_SET_CURSOR_POS_RPL);
    return STATUS_SUCCESS;
}

BOOL isGuestProcessWindow(HWND hWnd)
{
    BOOL bRet = FALSE;
    DWORD nPid = 0;

    GetWindowThreadProcessId(hWnd, &nPid);

    if (nPid)
    {
        ULONG64 ProcessFlags = SbieApi_QueryProcessInfo((HANDLE)nPid, 0);

        if (ProcessFlags & SBIE_FLAG_VALID_PROCESS)
        {
            bRet = TRUE;
        }
    }

    return bRet;
}

BOOL CALLBACK EnumThreadWndProc(HWND hwnd, LPARAM lParam)
{
    BOOL bContinue = TRUE;
    GUI_REMOVE_HOST_WINDOW_RPL* pRpl = (GUI_REMOVE_HOST_WINDOW_RPL*)lParam; // pRpl is from caller's stack.

    // thread window should from guest process. We only need check the first window's process.
    // Note, GetWindowThreadProcessId is not availabe in XP.
    if (pRpl->status == STATUS_UNSUCCESSFUL)
    {
        if (isGuestProcessWindow(hwnd))
        {
            pRpl->status = STATUS_SUCCESS;
        }
    }

    if (pRpl->status == STATUS_SUCCESS)
    {
        // webex put its ASIndicator in the previous window.
        HWND hPreWnd = GetWindow(hwnd, GW_HWNDPREV);

        if (hPreWnd && !isGuestProcessWindow(hPreWnd))
        {
            HWND hOwnerWnd = GetWindow(hPreWnd, GW_OWNER);

            // host process window's owner is a guest process windows.
            if (hOwnerWnd && isGuestProcessWindow(hOwnerWnd))
            {
                // We currently only remove Webex's 'ASIndicator' window.
                CHAR name[64] = { 0 };
                GetClassNameA(hPreWnd, name, 62);
                if (_strcmpi(name, "ASIndicator") == 0)
                {
                    SendMessage((HWND)hPreWnd, WM_CLOSE, 0, 0);
                    pRpl->bRemoved = TRUE;
                }
            }
        }
    }
    else
    {
        // don't do anything in the host process and stop enumerating
        pRpl->status = STATUS_ACCESS_DENIED;
        bContinue = FALSE;
    }

    return bContinue;
}

//---------------------------------------------------------------------------
// RemoveHostWindow
//---------------------------------------------------------------------------

ULONG GuiServer::RemoveHostWindow(SlaveArgs *args)
{
    GUI_REMOVE_HOST_WINDOW_REQ *req = (GUI_REMOVE_HOST_WINDOW_REQ *)args->req_buf;
    GUI_REMOVE_HOST_WINDOW_RPL *rpl = (GUI_REMOVE_HOST_WINDOW_RPL *)args->rpl_buf;

    if (args->req_len != sizeof(GUI_REMOVE_HOST_WINDOW_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    rpl->status = STATUS_UNSUCCESSFUL;
    rpl->bRemoved = FALSE;

    EnumThreadWindows(req->threadid, EnumThreadWndProc, (LPARAM)rpl);

    args->rpl_len = sizeof(GUI_REMOVE_HOST_WINDOW_RPL);

    if (rpl->bRemoved)
    {
        Sleep(100);
    }

    return STATUS_SUCCESS;
}

//---------------------------------------------------------------------------
// GetRawInputDeviceInfoSlave
//---------------------------------------------------------------------------

ULONG GuiServer::GetRawInputDeviceInfoSlave(SlaveArgs *args)
{
    GUI_GET_RAW_INPUT_DEVICE_INFO_REQ *req = (GUI_GET_RAW_INPUT_DEVICE_INFO_REQ *)args->req_buf;
    GUI_GET_RAW_INPUT_DEVICE_INFO_RPL *rpl = (GUI_GET_RAW_INPUT_DEVICE_INFO_RPL *)args->rpl_buf;

    if (args->req_len < sizeof(GUI_GET_RAW_INPUT_DEVICE_INFO_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    LPVOID reqData = req->hasData ? (BYTE*)req + sizeof(GUI_GET_RAW_INPUT_DEVICE_INFO_REQ) : NULL;
    PUINT pcbSize = NULL;
    if (req->cbSize != -1)
        pcbSize = &req->cbSize;

    SetLastError(ERROR_SUCCESS);
    if (req->unicode) {
        rpl->retval = GetRawInputDeviceInfoW((HANDLE)req->hDevice, req->uiCommand, reqData, pcbSize);
    }
    else {
        rpl->retval = GetRawInputDeviceInfoA((HANDLE)req->hDevice, req->uiCommand, reqData, pcbSize);
    }
    rpl->error = GetLastError();

    rpl->cbSize = req->cbSize;
    if (pcbSize && req->hasData)
    {
        // Note: pcbSize seams to be in tchars not in bytes!
        ULONG lenData = (*pcbSize) * (req->unicode ? sizeof(WCHAR) : 1);

        rpl->hasData = TRUE;
        LPVOID rplData = (BYTE*)rpl + sizeof(GUI_GET_RAW_INPUT_DEVICE_INFO_RPL);
        memcpy(rplData, reqData, lenData);
    }
    else
        rpl->hasData = FALSE;

    args->rpl_len = args->req_len;

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// WndHookNotifySlave
//---------------------------------------------------------------------------

ULONG GuiServer::WndHookNotifySlave(SlaveArgs *args)
{
    GUI_WND_HOOK_NOTIFY_REQ *req = (GUI_WND_HOOK_NOTIFY_REQ *)args->req_buf;
    GUI_WND_HOOK_NOTIFY_RPL *rpl = (GUI_WND_HOOK_NOTIFY_RPL *)args->rpl_buf;

    if (args->req_len != sizeof(GUI_WND_HOOK_NOTIFY_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    rpl->status = STATUS_UNSUCCESSFUL;

    EnterCriticalSection(&m_SlavesLock);

    WND_HOOK* whk = (WND_HOOK*)List_Head(&m_WndHooks);
    while (whk) {
        
        HANDLE hThread = OpenThread(THREAD_SET_CONTEXT, FALSE, (DWORD)whk->hthread);
		if (hThread)
		{
			QueueUserAPC((PAPCFUNC)whk->hproc, hThread, (ULONG_PTR)req->threadid);

			CloseHandle(hThread);

            whk = (WND_HOOK*)List_Next(whk);
		}
        else // hook helper thread is no longer valid
        {
            WND_HOOK* old_whk = whk; // invalid entry

            whk = (WND_HOOK*)List_Next(whk); // advance next

            // remove invalid entries
            List_Remove(&m_WndHooks, old_whk);
            HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, old_whk);
        }
    } 

    LeaveCriticalSection(&m_SlavesLock);

    rpl->status = STATUS_SUCCESS;

    args->rpl_len = sizeof(GUI_WND_HOOK_NOTIFY_RPL);
    
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// WndHookRegisterSlave
//---------------------------------------------------------------------------

ULONG GuiServer::WndHookRegisterSlave(SlaveArgs* args)
{
    GUI_WND_HOOK_REGISTER_REQ* req = (GUI_WND_HOOK_REGISTER_REQ*)args->req_buf;
    GUI_WND_HOOK_REGISTER_RPL* rpl = (GUI_WND_HOOK_REGISTER_RPL*)args->rpl_buf;

    if (args->req_len != sizeof(GUI_WND_HOOK_REGISTER_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    rpl->status = STATUS_UNSUCCESSFUL;

    EnterCriticalSection(&m_SlavesLock);

    WND_HOOK* whk = (WND_HOOK*)List_Head(&m_WndHooks);
    while (whk) {
        if (whk->pid == args->pid)
            break;
        whk = (WND_HOOK*)List_Next(whk);
    }    
    
    if (req->hthread && req->hproc) // register
    {
        if (!whk) // add if not already added
        {
            whk = (WND_HOOK *)HeapAlloc(GetProcessHeap(), 0, sizeof(WND_HOOK));
            whk->pid = args->pid;
            whk->hthread = req->hthread;
            whk->hproc = req->hproc;
            whk->HookCount = 0;

            List_Insert_After(&m_WndHooks, NULL, whk);
        }
        whk->HookCount++;
    }
    else if (whk) // unregister
    {
        whk->HookCount--;
        if (whk->HookCount <= 0) { // remobe if this was the last hook
            List_Remove(&m_WndHooks, whk);
            HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, whk);
        }
    }

    LeaveCriticalSection(&m_SlavesLock);

    rpl->status = STATUS_SUCCESS;

    args->rpl_len = sizeof(GUI_WND_HOOK_REGISTER_RPL);
    
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// GetProcessPathList
//---------------------------------------------------------------------------


ULONG GuiServer::GetProcessPathList(
    ULONG pid, void **out_pool, LIST **out_list)
{
    const ULONG path_code = 'wo';
    const HANDLE xpid = (HANDLE)(ULONG_PTR)pid;

    ULONG len;
    LONG status = SbieApi_QueryPathList(path_code, &len, NULL, xpid);
    if (status != 0)
        return status;

    status = STATUS_INSUFFICIENT_RESOURCES;

    POOL *pool = Pool_Create();
    if (! pool)
        return status;

    WCHAR *path = (WCHAR *)Pool_Alloc(pool, len);
    LIST *list = (LIST *)Pool_Alloc(pool, sizeof(LIST));

    if (path && list)
        status = SbieApi_QueryPathList(path_code, NULL, path, xpid);

    if (status != STATUS_SUCCESS) {
        Pool_Delete(pool);
        return status;
    }

    List_Init(list);
    while (*path) {
        PATTERN *pattern = Pattern_Create(pool, path, TRUE);
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


bool GuiServer::CheckProcessPathList(LIST *list, const WCHAR *str)
{
    ULONG len = wcslen(str);
    PATTERN *pat = (PATTERN *)List_Head(list);
    while (pat) {
        if (Pattern_Match(pat, str, len))
            return true;
        pat = (PATTERN *)List_Next(pat);
    }
    return false;
}


//---------------------------------------------------------------------------
// CheckSameProcessBoxes
//---------------------------------------------------------------------------


bool GuiServer::CheckSameProcessBoxes(
    ULONG in_pid, WCHAR *boxname, HWND hwnd, ULONG *out_pid)
{
    *out_pid = 0;
    ULONG idThread = GetWindowThreadProcessId(hwnd, out_pid);
    if (! (*out_pid))
        return false;

    if (*out_pid == in_pid)
        return true;

    WCHAR boxname2[48];
    ULONG status = SbieApi_QueryProcess((HANDLE)(ULONG_PTR)*out_pid,
                                        boxname2, NULL, NULL, NULL);
    if (! NT_SUCCESS(status))
        return false;

    if (! boxname) {
        WCHAR boxname1[48];
        status = SbieApi_QueryProcess((HANDLE)(ULONG_PTR)in_pid,
                                      boxname1, NULL, NULL, NULL);
        if (! NT_SUCCESS(status))
            return false;
        boxname = boxname1;
    }

    if (_wcsicmp(boxname, boxname2) != 0) {
        *out_pid = -1;      // both processes in box, but not same box
        return false;
    }

    return true;
}


//---------------------------------------------------------------------------
// CheckWindowAccessible
//---------------------------------------------------------------------------


bool GuiServer::CheckWindowAccessible(
    ULONG pid, WCHAR *boxname, LIST *list, HWND hwnd)
{
    //
    // allow if target window is part of a process in the same sandbox
    //

    ULONG pid2;
    if (CheckSameProcessBoxes(pid, boxname, hwnd, &pid2))
        return true;

    if (pid2 == -1)     // both processes in box, but not same box
        return false;

    //
    // allow if class name of target window matches OpenWinClass
    //

    WCHAR clsnm[256];
    if (GetClassName(hwnd, clsnm, 255)) {

        clsnm[255] = L'\0';
        _wcslwr(clsnm);

        if (CheckProcessPathList(list, clsnm))
            return true;
    }

    //
    // allow if process name of target window matches $:OpenWinClass
    //

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid2);
    if (hProcess) {

        ULONG x;
        NTSTATUS status = NtQueryInformationProcess(
                    hProcess, ProcessImageFileName,
                    clsnm, sizeof(clsnm) - 8, &x);
        x = 0;

        if (NT_SUCCESS(status)) {

            WCHAR *name;
            UNICODE_STRING *uni = (UNICODE_STRING *)clsnm;

            if (uni->Buffer) {      // Curt 26-Aug-2013

                uni->Buffer[uni->Length / sizeof(WCHAR)] = L'\0';
                name = wcsrchr(uni->Buffer, L'\\');
                if (name)
                    ++name;
                else
                    name = uni->Buffer;

                _wcslwr(name);
                --name;
                *name = L':';
                --name;
                *name = L'$';

                if (CheckProcessPathList(list, name))
                    x = 1;
            }
        }

        CloseHandle(hProcess);

        if (x == 1)
            return true;
    }

    return false;
}


//---------------------------------------------------------------------------
// CompareIntegrityLevels
//---------------------------------------------------------------------------


_FX bool GuiServer::CompareIntegrityLevels(ULONG src_pid, HWND dst_hwnd)
{
    ULONG src_il = (ULONG)(ULONG_PTR)SbieApi_QueryProcessInfo(
                                        (HANDLE)(ULONG_PTR)src_pid, 'pril');
    if (src_il == tzuk) // Windows XP
        return true;

    ULONG dst_pid = 0;
    ULONG dst_tid = GetWindowThreadProcessId(dst_hwnd, &dst_pid);
    if ((! dst_tid) || (! dst_pid))
        return false;

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dst_pid);
    if (! hProcess)
        return false;

    HANDLE hToken;
    BOOL ok = OpenProcessToken(hProcess, TOKEN_QUERY, &hToken);
    CloseHandle(hProcess);
    if (! ok)
        return false;

    union {
        SID_AND_ATTRIBUTES label;       // TOKEN_MANDATORY_LABEL
        UCHAR info_space[64];
    } info;
    ULONG len;
    ok = GetTokenInformation(
                hToken, (TOKEN_INFORMATION_CLASS)TokenIntegrityLevel,
                &info, sizeof(info), &len);
    CloseHandle(hToken);
    if (! ok)
        return false;

    ULONG dst_il = ((ULONG *)info.label.Sid)[2];
    return (src_il >= dst_il) ? true : false;
}


//---------------------------------------------------------------------------
// ShouldIgnoreIntegrityLevels
//---------------------------------------------------------------------------


bool GuiServer::ShouldIgnoreIntegrityLevels(ULONG pid, HWND hwnd)
{
    //
    // we can't tell if the target window used ChangeWindowMessageFilter*Ex
    // so we rely on OpenWinClass=<clsnm>/IgnoreUIPI setting to tell us when
    // it is ok to violate UIPI
    //

    static const WCHAR *_IgnoreUIPI = L"/ignoreuipi";

    POOL *pool;
    LIST *list;
    ULONG status = GetProcessPathList(pid, (void **)&pool, &list);
    if (status != 0)
        return false;

    //
    // check if we have an OpenWinClass=<clsnm>/IgnoreUIPI
    //

    WCHAR clsnm[256+32];
    if (GetClassName(hwnd, clsnm, 255)) {

        clsnm[255] = L'\0';
        _wcslwr(clsnm);
        wcscat(clsnm, _IgnoreUIPI);

        if (CheckProcessPathList(list, clsnm)) {
            Pool_Delete(pool);
            return true;
        }
    }

    //
    // allow if process name of target window matches $:OpenWinClass
    //

    ULONG idProcess = 0;
    ULONG idThread = GetWindowThreadProcessId(hwnd, &idProcess);
    if (! idProcess) {
        Pool_Delete(pool);
        return false;
    }

    HANDLE hProcess =
                OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, idProcess);
    if (hProcess) {

        ULONG x;
        NTSTATUS status = NtQueryInformationProcess(
                    hProcess, ProcessImageFileName,
                    clsnm, sizeof(clsnm) - 8, &x);
        x = 0;

        if (NT_SUCCESS(status)) {

            WCHAR *name;
            UNICODE_STRING *uni = (UNICODE_STRING *)clsnm;
            uni->Buffer[uni->Length / sizeof(WCHAR)] = L'\0';
            name = wcsrchr(uni->Buffer, L'\\');
            if (name)
                ++name;
            else
                name = uni->Buffer;

            _wcslwr(name);
            if (wcslen(name) < 250) {

                wcscpy(clsnm, L"$:");
                wcscat(clsnm, name);
                wcscat(clsnm, _IgnoreUIPI);

                if (CheckProcessPathList(list, clsnm))
                    x = 1;
            }
        }

        CloseHandle(hProcess);

        if (x == 1) {
            Pool_Delete(pool);
            return true;
        }
    }

    Pool_Delete(pool);
    return false;
}


//---------------------------------------------------------------------------
// AllowSendPostMessage
//---------------------------------------------------------------------------


bool GuiServer::AllowSendPostMessage(
    ULONG pid, ULONG msg, bool IsSendMsg, HWND hwnd)
{

#define WM_NCMOUSEFIRST                 0x00A0
#define WM_NCMOUSELAST                  0x00AD

#define WM_MOUSEHOVERFIRST              0x02A0
#define WM_MOUSEHOVERLAST               0x02A3

    //
    // if all messages are allowed, or,
    // if fake input is allowed, and this is a fake input message,
    // then don't check anything else
    //

    ULONG64 ProcessFlags =
        SbieApi_QueryProcessInfo((HANDLE)(ULONG_PTR)pid, 0);

    if (ProcessFlags & SBIE_FLAG_OPEN_ALL_WIN_CLASS)
        return true;

#define IS_INPUT_MESSAGE(msg) (                                             \
    msg == WM_INPUT                                                 ||      \
    (msg >= WM_KEYFIRST && msg <= WM_KEYLAST)                       ||      \
    (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST)                   ||      \
    (msg >= WM_NCMOUSEFIRST && msg <= WM_NCMOUSELAST)               ||      \
    (msg >= WM_MOUSEHOVERFIRST && msg <= WM_MOUSEHOVERLAST)         ||      \
    (msg >= WM_TABLET_FIRST && msg <= WM_TABLET_LAST))

    if (IS_INPUT_MESSAGE(msg))
        return true;

    //
    // we let a sandboxed process send or post WM_DDE_ACK to
    // any window it wants, inside or outside the sandbox
    //

    //if (msg == WM_DDE_ACK)
        //return true;

    //
    // discard some messages that might hide, close or crash windows
    // outside the sandbox.  note that these messages are aimed
    // primarily at protecting Windows Explorer outside the sandbox:
    // for example, WM_QUERYENDSESSION seems like a harmless message,
    // but Shell_TrayWnd reacts to it badly
    //

    if (msg < WM_USER) {

        static const ULONG sysmsgs[] = {
            0x0002,             // WM_DESTROY
            0x000B,             // WM_SETREDRAW
            0x0010,             // WM_CLOSE
            0x0011,             // WM_QUERYENDSESSION
            0x0012,             // WM_QUIT
            0x0016,             // WM_ENDSESSION
            0x003B,             // ?
            0x004E,             // WM_NOTIFY
            0x0082,             // WM_NCDESTROY
            0x0111,             // WM_COMMAND
            0x0112,             // WM_SYSCOMMAND
            0x0319,             // WM_APPCOMMAND
            0x000F,             // WM_PAINT see http://www.welivesecurity.com/2012/12/27/win32gapz-steps-of-evolution/ (Win32/Gapz)
            0
        };

        for (ULONG i = 0; sysmsgs[i]; ++i) {
            if (msg == sysmsgs[i]) {

                return false;
            }
        }
    }

    //
    // block non-system messages sent to a Windows Explorer window
    // (initially was going to block only 0x0404 0x0408 0x0553 0x0556
    // 0x055A 0x0562 0x0575, but the list seems to grow and grow)
    //

    if ((msg >= WM_USER) || IS_INPUT_MESSAGE(msg)) {

        WCHAR clsnm[256];
        ULONG pidExplorer = 0;
        ULONG pidWindow = 0;
        ULONG tid = GetWindowThreadProcessId(GetShellWindow(), &pidExplorer);
        tid = GetWindowThreadProcessId(hwnd, &pidWindow);

        bool isWindowInExplorer = false;
        if (pidExplorer && pidExplorer == pidWindow) {
            if (GetClassName(hwnd, clsnm, 255)) {
                clsnm[255] = L'\0';
                isWindowInExplorer = true;
            }
        }

        if (isWindowInExplorer) {

            bool blocked = true;

            ULONG len = wcslen(clsnm);

#define ISWNDCLASS(n,s) (len == n && _wcsicmp(clsnm, s) == 0)

            //
            // allow any message to CicMarshalWndClass
            //

            if (blocked && ISWNDCLASS(18, L"CicMarshalWndClass")) {

                blocked = false;
            }

            //
            // allow post message 0x412 to Progman (recent docs feature)
            //

            if (blocked && (! IsSendMsg) && (msg == 0x0412)
                    && ISWNDCLASS(7, L"Progman")) {

                blocked = false;
            }

            //
            // allow interacting with MSTaskSwWClass on
            // Windows 7.  used by ITaskbarList3 interface
            //

            if (blocked &&
                    // if SendMessage 0x44C 0x44D 0x44E 0x44F 0x450
                    //                0x451 0x455
                    // or PostMessage 0x440 0x441 0x443 0x444 0x447
                    //                0x448 0x44B 0x457
                        ((IsSendMsg && (
                                msg == 0x044C || msg == 0x044D ||
                                msg == 0x044E || msg == 0x044F ||
                                msg == 0x0450 || msg == 0x0451 ||
                                msg == 0x0455))
                    ||  ((! IsSendMsg) && (
                                msg == 0x0440 || msg == 0x0441 ||
                                msg == 0x0443 || msg == 0x0444 ||
                                msg == 0x0447 || msg == 0x0448 ||
                                msg == 0x044B || msg == 0x0457)))
                    /* then if wndclass ... */
                && ISWNDCLASS(14, L"MSTaskSwWClass")) {

                blocked = false;
            }

            //
            // allow Excel and PowerPoint to interact with
            // MSTaskSwWClass freely
            //

            if (blocked
                 && (! (ProcessFlags & SBIE_FLAG_IMAGE_FROM_SANDBOX))
                 && ISWNDCLASS(14, L"MSTaskSwWClass")) {

                WCHAR sender_exename[128];
                SbieApi_QueryProcess((HANDLE)(ULONG_PTR)pid, NULL,
                                     sender_exename, NULL, NULL);

                if (   _wcsicmp(sender_exename, L"excel.exe")    == 0
                    || _wcsicmp(sender_exename, L"powerpnt.exe") == 0) {

                    blocked = false;
                }
            }

            //
            // allow interacting with Shell_TrayWnd
            //

            if (blocked && ISWNDCLASS(13, L"Shell_TrayWnd")) {

                //
                // Windows before 7: allow send msg 0x04EC which
                // is used by ITaskBarList interface to locate
                // the MSTaskSwWClass window.  (Windows 7 uses
                // GetProp instead of send msg.)
                //

                if (IsSendMsg && (msg == 0x4EC)) {

                    blocked = false;
                }

                //
                // Windows 7:  allow post msg 0x04F2 which is
                // used to minimize all windows except poster
                //

                if ((! IsSendMsg) && (msg == 0x04F2)) {

                    blocked = false;
                }

                //
                // Windows Vista and later:  allow send msg 0x04EF
                // used by SHELL32!SHQueryUserNotificationState
                //

                if (IsSendMsg && (msg == 0x04EF)) {

                    blocked = false;
                }
            }

            //
            // block WM_USER messages to Windows Explorer, unless we
            // determined the message should not be blocked
            //

            if (blocked)
                return false;
        }
    }

    //
    //
    //

    return true;
}


#undef ISWNDCLASS
#undef IS_INPUT_MESSAGE

//---------------------------------------------------------------------------
// GuiServer__DropConsoleIntegrity
//---------------------------------------------------------------------------


void GuiServer::DropConsoleIntegrity()
{
    ULONG_PTR consoleHostProcess;
    if (!NT_SUCCESS(NtQueryInformationProcess(GetCurrentProcess(), ProcessConsoleHostProcess, &consoleHostProcess, sizeof(ULONG_PTR), NULL)))
        return;
    
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, (DWORD)consoleHostProcess);
    if (! hProcess)
        return;

    HANDLE tokenHandle;
    if (OpenProcessToken(hProcess, TOKEN_QUERY | TOKEN_ADJUST_DEFAULT, &tokenHandle))
    {
        static SID_IDENTIFIER_AUTHORITY mandatoryLabelAuthority = SECURITY_MANDATORY_LABEL_AUTHORITY;

        UCHAR newSidBuffer[FIELD_OFFSET(SID, SubAuthority) + sizeof(ULONG)];
        PSID newSid;
        newSid = (PSID)newSidBuffer;
        RtlInitializeSid(newSid, &mandatoryLabelAuthority, 1);
        *RtlSubAuthoritySid(newSid, 0) = SECURITY_MANDATORY_UNTRUSTED_RID;

        TOKEN_MANDATORY_LABEL mandatoryLabel;
        mandatoryLabel.Label.Sid = newSid;
        mandatoryLabel.Label.Attributes = SE_GROUP_INTEGRITY;

        NtSetInformationToken(tokenHandle, (TOKEN_INFORMATION_CLASS)TokenIntegrityLevel, &mandatoryLabel, sizeof(TOKEN_MANDATORY_LABEL));

        NtClose(tokenHandle);
    }

    CloseHandle(hProcess);
}


//---------------------------------------------------------------------------
// RunConsoleSlave
//---------------------------------------------------------------------------


void GuiServer::RunConsoleSlave(const WCHAR *evtname)
{
    //
    // the console helper process creates a console, which causes
    // csrss to start a conhost.exe process outside the sandbox.
    // Gui_ConnectConsole in core/dll/guicon.c will then connect
    // to this console using the process id of the console helper
    //

    const WCHAR* boxname = wcsrchr(evtname, L':');

    HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, evtname);

    const ULONG max_pids = 16000;
    ULONG pids_len = max_pids * sizeof(ULONG);
    ULONG *pids = (ULONG *)HeapAlloc(GetProcessHeap(), 0, pids_len);

    if (hEvent && pids) {

        if (AllocConsole()) {

            if (boxname++ && SbieApi_QueryConfBool(boxname, L"DropConHostIntegrity", FALSE))
                DropConsoleIntegrity();

            AdjustConsoleTaskbarButton();

            SetEvent(hEvent);

            //
            // wait for the process in the sandbox to connect to the
            // console we created, then we can detach from that console
            // and terminate this console helper process
            //

            while (1) {

                Sleep(2000);

                ULONG num_pids = GetConsoleProcessList(pids, max_pids);
                if (num_pids > 1 && num_pids < max_pids) {

                    Sleep(2000);
                    break;
                }
            }
        }
    }

    ExitProcess(0);
}


//---------------------------------------------------------------------------
// ConsoleCallbackSlave
//---------------------------------------------------------------------------


void GuiServer::ConsoleCallbackSlave(void *arg, BOOLEAN timeout)
{
    //
    // this wait callback set up in CreateConsoleSlave above will execute
    // on termination of the sandboxed process that requested the creation
    // of the console.  this is necessary to make sure the console helper
    // is terminated even if the process in the sandbox never actually
    // connected to the console
    //

    HANDLE *WaitHandles = (HANDLE *)arg;

    UnregisterWait(WaitHandles[0]);

    CloseHandle(WaitHandles[1]);

    TerminateProcess(WaitHandles[2], 0);
    CloseHandle(WaitHandles[2]);

    HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, WaitHandles);
}


//---------------------------------------------------------------------------
// AdjustConsoleTaskbarButton
//---------------------------------------------------------------------------


#define PROPSYS_INITGUID
#include "core/dll/propsys.h"

DEFINE_PROPERTYKEY(PKEY_AppUserModel_PreventPinning, 0x9F4C2855, 0x9F79, 0x4B39, 0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3, 9);


void GuiServer::AdjustConsoleTaskbarButton()
{
    //
    // the sandboxed process will not be able to use SetProp to adjust
    // the taskbar properties for the console window, which belongs to
    // a process outside the sandbox.  work around that limitation here
    //

    static const GUID IID_IPropertyStore = {
        0x886D8EEB, 0x8CF2, 0x4446,
            { 0x8D, 0x02, 0xCD, 0xBA, 0x1D, 0xBD, 0xCF, 0x99 } };

    typedef HRESULT (*P_SHGetPropertyStoreForWindow)(
        HWND hwnd, REFIID riid, IPropertyStore **ppv);

    IPropertyStore *pPropertyStore;
    PROPVARIANT v;
    HRESULT hr;

    P_SHGetPropertyStoreForWindow SHGetPropertyStoreForWindow =
        (P_SHGetPropertyStoreForWindow) GetProcAddress(
            LoadLibrary(L"shell32.dll"), "SHGetPropertyStoreForWindow");
    if (! SHGetPropertyStoreForWindow)
        return;

    hr = SHGetPropertyStoreForWindow(
                GetConsoleWindow(), IID_IPropertyStore, &pPropertyStore);
    if (FAILED(hr))
        return;

    //
    // the PKEY_AppUserModel_PreventPinning property will cause the
    // taskbar to display only the "close window" button.  this property
    // must be set before the PKEY_AppUserModel_ID property
    //

    v.vt = VT_BOOL;
    v.boolVal = -1;     // VT_BOOL true
    pPropertyStore->lpVtbl->SetValue(
        pPropertyStore, &PKEY_AppUserModel_PreventPinning, v);

    v.vt = VT_BSTR;
    v.bstrVal = SANDBOXIE L"_v4_Proxy_Console_Window";
    pPropertyStore->lpVtbl->SetValue(
        pPropertyStore, &PKEY_AppUserModel_ID, v);

    pPropertyStore->lpVtbl->Release(pPropertyStore);
}


//---------------------------------------------------------------------------
// DdeProxyThreadSlave
//---------------------------------------------------------------------------


ULONG GuiServer::DdeProxyThreadSlave(void *xDdeArgs)
{
    //
    // this function, which runs in its own thread, creates a proxy window
    // for a DDE conversation.  to the client window/process, this window
    // is the server.  to the real server window/process, this window looks
    // is the client.
    //
    // this is needed because the IL bug in core/dll/guidde.c prevents
    // PostMessage from working correctly between DDE client/server.
    // however the proxy window is not in the sandbox and not subject to
    // that IL bug, so it can receive WM_DDE_EXECUTE and WM_DDE_REQUEST
    // messages posted by the client.  to the server it sends WM_COPYDATA
    // messages instead of posting DDE messages.
    //

    static ATOM _atom = 0;
    static UINT _dde_exec = 0;

    ULONG_PTR *DdeArgs = (ULONG_PTR *)xDdeArgs;
    HWND hClientWnd = (HWND)DdeArgs[0];
    HWND hServerWnd = (HWND)DdeArgs[1];
    LPARAM lParam = (LPARAM)DdeArgs[2];
    HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, DdeArgs);

    //
    // create the proxy window class and window instance
    //
    // note the RPCSS process uses the in-sandbox window class
    // _DDE_ProxyClass1 for another DDE-related proxy window,
    // see file apps/com/rpcss/dde.c
    //

    if (! _atom) {

        WNDCLASS wc;
        memzero(&wc, sizeof(wc));
        wc.lpfnWndProc = DefWindowProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = SANDBOXIE L"_DDE_ProxyClass2";
        _atom = RegisterClass(&wc);
        if (! _atom)
            return 0;
    }

    HWND hProxyWnd = CreateWindowEx(
            0, (LPCWSTR)_atom, L"", 0,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            NULL, NULL, GetModuleHandle(NULL), NULL);
    if (! hProxyWnd)
        return 0;

    //
    // we are started by SendPostMessageSlave due to request to send
    // a WM_DDE_ACK message, so send that to the client window
    //

    SendMessage(hClientWnd, WM_DDE_ACK, (WPARAM)hProxyWnd, lParam);

    //
    // allow 10 seconds for the DDE conversation, then destroy window
    //

    ULONG_PTR TimerId = SetTimer(hProxyWnd, tzuk, 10 * 1000, NULL);
    if (! TimerId) {
        DestroyWindow(hProxyWnd);
        return 0;
    }

    //
    // main loop for proxy window
    //

    while (1) {

        MSG msg;
        BOOL ok = GetMessageW(&msg, NULL, 0, 0);

        /*WCHAR txt[128]; wsprintf(txt, L"Proxy HWND=%08X Receives MSG=%04X from WPARAM=%08X\n",
            msg.hwnd, msg.message, msg.wParam); OutputDebugString(txt);*/

        if ((! ok) || (ok == -1))
            break;
        DispatchMessage(&msg);

        if (msg.message == WM_TIMER && msg.wParam == tzuk)
            break;

        if ((msg.message == WM_DDE_EXECUTE || msg.message == WM_DDE_REQUEST)
                                    && (HWND)msg.wParam == hClientWnd) {

            //
            // WM_DDE_EXECUTE or WM_DDE_REQUEST posted by the client.
            //
            // as described above, we can't just post this "as is" to the
            // server window, so instead we send a WM_COPYDATA message to
            // the server window.  the WM_COPYDATA messages has to specify
            // in its wParam that our proxy window here is the other side.
            //
            // to work around this, we put a property on the window, which
            // will help Gui_DDE_COPYDATA_Received in core/dll/guidde.c to
            // figure out the real client window.
            //

            WCHAR prop_name[64];
            wsprintf(prop_name, SBIE L"_DDE_%08X", (ULONG_PTR)hServerWnd);
            SetProp(hProxyWnd, prop_name, hClientWnd);

            lParam = msg.lParam;

            COPYDATASTRUCT cds;
            cds.dwData = tzuk;

            if (msg.message == WM_DDE_EXECUTE) {

                //
                // WM_DDE_EXECUTE:  send command text in WM_COPYDATA
                //

                cds.cbData = (ULONG)(ULONG_PTR)GlobalSize((HGLOBAL)lParam);
                cds.lpData = GlobalLock((HGLOBAL)lParam);
                if (cds.lpData) {

                    SendMessage(hServerWnd, WM_COPYDATA,
                                (WPARAM)hProxyWnd, (LPARAM)&cds);
                    GlobalUnlock((HGLOBAL)lParam);
                }

            } else if (msg.message == WM_DDE_REQUEST) {

                //
                // WM_DDE_REQUEST:  send special command text in WM_COPYDATA.
                // note that we put the lParam from WM_DDE_REQUEST into the
                // command text.  see Gui_DDE_COPYDATA_Received
                //

                // keep following string in sync with core/dll/guidde.c
                wsprintf(prop_name,
                    L"//" SANDBOXIE L"//DDE//REQ//%08X", lParam);
                cds.cbData = (wcslen(prop_name) + 1) * sizeof(WCHAR);
                cds.lpData = prop_name;
                SendMessage(hServerWnd, WM_COPYDATA,
                            (WPARAM)hProxyWnd, (LPARAM)&cds);

                // see SendCopyDataSlave
                DDE_Request_ProxyWnd = hProxyWnd;
            }
        }

        //
        // Post DDE DATA on behalf of SendCopyDataSlave
        //

        if (msg.message == (WM_USER + 0x123) && msg.wParam == tzuk) {

            DDE_Request_ProxyWnd = 0;
            PostMessage(
                hClientWnd, WM_DDE_DATA, (WPARAM)hProxyWnd, msg.lParam);
        }

        //
        // if the server posts WM_DDE_ACK, forward to client
        //

        if (msg.message == WM_DDE_ACK && (HWND)msg.wParam == hServerWnd) {

            PostMessage(hClientWnd, WM_DDE_ACK, (WPARAM)hProxyWnd, lParam);
        }

        //
        // if the client posts WM_DDE_TERMINATE, reply with same
        //

        if (msg.message == WM_DDE_TERMINATE &&
                                    (HWND)msg.wParam == hClientWnd) {

            PostMessage(hClientWnd, WM_DDE_TERMINATE, (WPARAM)hProxyWnd, 0);
        }
    }

    KillTimer(hProxyWnd, TimerId);
    DestroyWindow(hProxyWnd);

    return 0;
}
