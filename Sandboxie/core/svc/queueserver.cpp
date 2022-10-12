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
// Queue Server -- using PipeServer
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "queueserver.h"
#include "queuewire.h"
#include "core/dll/sbieapi.h"


//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------


typedef struct _REQUEST_OBJ {

    LIST_ELEM list_elem;
    ULONG  client_pid;
    ULONG  client_tid;
    HANDLE client_event;
    ULONG  request_id;
    ULONG  req_data_len;
    ULONG  rpl_data_len;
    void   *req_data_ptr;
    void   *rpl_data_ptr;

} REQUEST_OBJ;


typedef struct _QUEUE_OBJ {

    LIST_ELEM list_elem;
    FILETIME server_create_time;
    HANDLE server_pid;
    HANDLE server_event;
    LIST  requests;
    ULONG queue_name_len;
    WCHAR queue_name[1];

} QUEUE_OBJ;


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


QueueServer::QueueServer(PipeServer *pipeServer)
{
    m_heap = HeapCreate(0, 0, 0);
    if (! m_heap)
        m_heap = GetProcessHeap();

    InitializeCriticalSectionAndSpinCount(&m_lock, 1000);
    List_Init(&m_queues);

    m_RequestId = 0x00000001;

    pipeServer->Register(MSGID_QUEUE, this, Handler);
}

QueueServer::~QueueServer()
{
	// cleanup CS
	DeleteCriticalSection(&m_lock);
}


//---------------------------------------------------------------------------
// CloseCallback
//---------------------------------------------------------------------------


void QueueServer::CloseCallback(void *context, void *data)
{
    QueueServer *pThis = (QueueServer *)context;
    REQUEST_OBJ *RequestObj = (REQUEST_OBJ *)data;
    NtClose(RequestObj->client_event);
    if (RequestObj->req_data_ptr)
        HeapFree(pThis->m_heap, 0, RequestObj->req_data_ptr);
}


//---------------------------------------------------------------------------
// Handler
//---------------------------------------------------------------------------


MSG_HEADER *QueueServer::Handler(void *_this, MSG_HEADER *msg)
{
    QueueServer *pThis = (QueueServer *)_this;

    HANDLE idProcess = (HANDLE)(ULONG_PTR)PipeServer::GetCallerProcessId();

    if (msg->msgid == MSGID_QUEUE_NOTIFICATION) {
        pThis->NotifyHandler(idProcess);
        return NULL;
    }

    if (msg->msgid == MSGID_QUEUE_CREATE)
        return pThis->CreateHandler(msg, idProcess);

    if (msg->msgid == MSGID_QUEUE_GETREQ)
        return pThis->GetReqHandler(msg, idProcess);

    if (msg->msgid == MSGID_QUEUE_PUTRPL)
        return pThis->PutRplHandler(msg, idProcess);

    if (msg->msgid == MSGID_QUEUE_PUTREQ)
        return pThis->PutReqHandler(msg, idProcess);

    if (msg->msgid == MSGID_QUEUE_GETRPL)
        return pThis->GetRplHandler(msg, idProcess);

    return NULL;
}


//---------------------------------------------------------------------------
// CreateHandler
//---------------------------------------------------------------------------


MSG_HEADER *QueueServer::CreateHandler(MSG_HEADER *msg, HANDLE idProcess)
{
    WCHAR *QueueName = NULL;
    HANDLE hProcess = NULL;
    HANDLE hEvent = NULL;
    ULONG status;

    EnterCriticalSection(&m_lock);

    QUEUE_CREATE_REQ *req = (QUEUE_CREATE_REQ *)msg;
    if (req->h.length < sizeof(QUEUE_CREATE_REQ)) {
        status = STATUS_INVALID_PARAMETER;
        goto finish;
    }

    //
    //
    //

    status = OpenProcess(idProcess, &hProcess,
                         PROCESS_DUP_HANDLE | PROCESS_QUERY_INFORMATION);
    if (! NT_SUCCESS(status))
        goto finish;

    QueueName = MakeQueueName(idProcess, req->queue_name, &status);
    if (! QueueName)
        goto finish;

    if (status == STATUS_ACCESS_DENIED) {
        // process in the sandbox is trying to create an asterisk queue
        goto finish;
    }

    QUEUE_OBJ *QueueObj = (QUEUE_OBJ *)FindQueueObj(QueueName);
    if (QueueObj) {
        status = STATUS_OBJECT_NAME_COLLISION;
        goto finish;
    }

    //
    //
    //

    FILETIME time1, time2, time3, time4;
    if (! GetProcessTimes(hProcess, &time1, &time2, &time3, &time4)) {
        status = STATUS_INVALID_CID;
        goto finish;
    }

    status = DuplicateEvent(hProcess, req->event_handle, &hEvent);
    if (! NT_SUCCESS(status))
        goto finish;

    ULONG queue_obj_len = sizeof(QUEUE_OBJ)
                        + (wcslen(QueueName) + 1) * sizeof(WCHAR);
    QueueObj = (QUEUE_OBJ *)HeapAlloc(m_heap, 0, queue_obj_len);
    if (! QueueObj) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto finish;
    }

    QueueObj->server_create_time.dwHighDateTime = time1.dwHighDateTime;
    QueueObj->server_create_time.dwLowDateTime  = time1.dwLowDateTime;

    QueueObj->server_pid = idProcess;
    QueueObj->server_event = hEvent;
    hEvent = NULL;

    List_Init(&QueueObj->requests);

    wcscpy(QueueObj->queue_name, QueueName);
    QueueObj->queue_name_len = wcslen(QueueObj->queue_name);

    List_Insert_After(&m_queues, NULL, QueueObj);

    status = STATUS_SUCCESS;

finish:

    LeaveCriticalSection(&m_lock);

    if (hEvent)
        CloseHandle(hEvent);

    if (hProcess)
        CloseHandle(hProcess);

    if (QueueName)
        HeapFree(m_heap, 0, QueueName);

    return SHORT_REPLY(status);
}


//---------------------------------------------------------------------------
// GetReqHandler
//---------------------------------------------------------------------------


MSG_HEADER *QueueServer::GetReqHandler(MSG_HEADER *msg, HANDLE idProcess)
{
    WCHAR *QueueName = NULL;
    HANDLE hProcess = NULL;
    ULONG status;
    QUEUE_GETREQ_RPL *rpl = NULL;

    EnterCriticalSection(&m_lock);

    QUEUE_GETREQ_REQ *req = (QUEUE_GETREQ_REQ *)msg;
    if (req->h.length < sizeof(QUEUE_GETREQ_REQ)) {
        status = STATUS_INVALID_PARAMETER;
        goto finish;
    }

    //
    //
    //

    status = OpenProcess(idProcess, &hProcess);
    if (! NT_SUCCESS(status))
        goto finish;

    QueueName = MakeQueueName(idProcess, req->queue_name, &status);
    if (! QueueName)
        goto finish;

    QUEUE_OBJ *QueueObj = (QUEUE_OBJ *)FindQueueObj(QueueName);
    if (! QueueObj) {
        status = STATUS_OBJECT_NAME_NOT_FOUND;
        goto finish;
    }

    if (QueueObj->server_pid != idProcess) {
        status = STATUS_ACCESS_DENIED;
        goto finish;
    }

    //
    //
    //

    REQUEST_OBJ *RequestObj = (REQUEST_OBJ *)List_Head(&QueueObj->requests);
    while (RequestObj) {
        if (RequestObj->req_data_len && RequestObj->req_data_ptr)
            break;
        RequestObj = (REQUEST_OBJ *)List_Next(RequestObj);
    }

    if (! RequestObj) {
        status = STATUS_END_OF_FILE;
        goto finish;
    }

    //
    //
    //

    ULONG rpl_len = sizeof(QUEUE_GETREQ_RPL)
                  + RequestObj->req_data_len
                  + 8;
    rpl = (QUEUE_GETREQ_RPL *)LONG_REPLY(rpl_len);
    if (! rpl) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto finish;
    }

    rpl->client_pid = RequestObj->client_pid;
    rpl->client_tid = RequestObj->client_tid;
    rpl->req_id = RequestObj->request_id;
    rpl->data_len = RequestObj->req_data_len;
    memcpy(rpl->data, RequestObj->req_data_ptr, RequestObj->req_data_len);
    memzero(rpl->data + RequestObj->req_data_len, 8);

    HeapFree(m_heap, 0, RequestObj->req_data_ptr);
    RequestObj->req_data_ptr = NULL;
    RequestObj->req_data_len = 0;

    status = STATUS_SUCCESS;

finish:

    LeaveCriticalSection(&m_lock);

    if (hProcess)
        CloseHandle(hProcess);

    if (QueueName)
        HeapFree(m_heap, 0, QueueName);

    if (! rpl)
        rpl = (QUEUE_GETREQ_RPL *)SHORT_REPLY(status);

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// PutRplHandler
//---------------------------------------------------------------------------


MSG_HEADER *QueueServer::PutRplHandler(MSG_HEADER *msg, HANDLE idProcess)
{
    WCHAR *QueueName = NULL;
    HANDLE hProcess = NULL;
    ULONG status;

    EnterCriticalSection(&m_lock);

    QUEUE_PUTRPL_REQ *req = (QUEUE_PUTRPL_REQ *)msg;
    if (req->h.length < sizeof(QUEUE_PUTRPL_REQ)) {
        status = STATUS_INVALID_PARAMETER;
        goto finish;
    }

    if ((! req->data_len) || (req->data_len > PIPE_MAX_DATA_LEN)) {
        status = STATUS_INVALID_PARAMETER;
        goto finish;
    }
    ULONG offset = FIELD_OFFSET(QUEUE_PUTRPL_REQ, data);
    if (offset + req->data_len > req->h.length) {
        status = STATUS_INVALID_PARAMETER;
        goto finish;
    }

    //
    //
    //

    status = OpenProcess(idProcess, &hProcess);
    if (! NT_SUCCESS(status))
        goto finish;

    QueueName = MakeQueueName(idProcess, req->queue_name, &status);
    if (! QueueName)
        goto finish;

    QUEUE_OBJ *QueueObj = (QUEUE_OBJ *)FindQueueObj(QueueName);
    if (! QueueObj) {
        status = STATUS_OBJECT_NAME_NOT_FOUND;
        goto finish;
    }

    if (QueueObj->server_pid != idProcess) {
        status = STATUS_ACCESS_DENIED;
        goto finish;
    }

    //
    //
    //

    REQUEST_OBJ *RequestObj = (REQUEST_OBJ *)List_Head(&QueueObj->requests);
    while (RequestObj) {
        if (RequestObj->request_id == req->req_id)
            break;
        RequestObj = (REQUEST_OBJ *)List_Next(RequestObj);
    }

    if (! RequestObj) {
        status = STATUS_END_OF_FILE;
        goto finish;
    }

    if (RequestObj->req_data_len || RequestObj->req_data_ptr) {

        DeleteRequestObj(&QueueObj->requests, RequestObj);
        status = STATUS_SHARING_VIOLATION;
        goto finish;
    }

    //
    //
    //

    void *ReplyData = HeapAlloc(m_heap, 0, req->data_len);
    if (! ReplyData) {

        DeleteRequestObj(&QueueObj->requests, RequestObj);
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto finish;
    }

    memcpy(ReplyData, req->data, req->data_len);
    RequestObj->rpl_data_ptr = ReplyData;
    RequestObj->rpl_data_len = req->data_len;

    if (RequestObj->client_event)
        SetEvent(RequestObj->client_event);

    status = STATUS_SUCCESS;

finish:

    LeaveCriticalSection(&m_lock);

    if (hProcess)
        CloseHandle(hProcess);

    if (QueueName)
        HeapFree(m_heap, 0, QueueName);

    return SHORT_REPLY(status);
}


//---------------------------------------------------------------------------
// PutReqHandler
//---------------------------------------------------------------------------


MSG_HEADER *QueueServer::PutReqHandler(MSG_HEADER *msg, HANDLE idProcess)
{
    WCHAR *QueueName = NULL;
    HANDLE hProcess = NULL;
    HANDLE hEvent = NULL;
    ULONG status;
    QUEUE_PUTREQ_RPL *rpl = NULL;

    EnterCriticalSection(&m_lock);

    QUEUE_PUTREQ_REQ *req = (QUEUE_PUTREQ_REQ *)msg;
    if (req->h.length < sizeof(QUEUE_PUTREQ_REQ)) {
        status = STATUS_INVALID_PARAMETER;
        goto finish;
    }

    if ((! req->data_len) || (req->data_len > PIPE_MAX_DATA_LEN)) {
        status = STATUS_INVALID_PARAMETER;
        goto finish;
    }
    ULONG offset = FIELD_OFFSET(QUEUE_PUTREQ_REQ, data);
    if (offset + req->data_len > req->h.length) {
        status = STATUS_INVALID_PARAMETER;
        goto finish;
    }

    //
    //
    //

    status = OpenProcess(idProcess, &hProcess);
    if (! NT_SUCCESS(status))
        goto finish;

    QueueName = MakeQueueName(idProcess, req->queue_name, &status);
    if (! QueueName)
        goto finish;

    QUEUE_OBJ *QueueObj = (QUEUE_OBJ *)FindQueueObj(QueueName);
    if (! QueueObj) {
        status = STATUS_OBJECT_NAME_NOT_FOUND;
        goto finish;
    }

    //
    //
    //

    status = DuplicateEvent(hProcess, req->event_handle, &hEvent);
    if (! NT_SUCCESS(status))
        goto finish;

    REQUEST_OBJ *RequestObj =
        (REQUEST_OBJ *)HeapAlloc(m_heap, 0, sizeof(REQUEST_OBJ));
    if (! RequestObj) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto finish;
    }

    void *RequestData = HeapAlloc(m_heap, 0, req->data_len);
    if (! RequestData) {
        HeapFree(m_heap, 0, RequestObj);
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto finish;
    }
    memcpy(RequestData, req->data, req->data_len);

    RequestObj->client_pid = (ULONG)(ULONG_PTR)idProcess;
    RequestObj->client_tid = PipeServer::GetCallerThreadId();
    RequestObj->client_event = hEvent;
    hEvent = NULL;

    RequestObj->request_id = InterlockedIncrement(&m_RequestId);
    while (m_RequestId == 0 || m_RequestId == -1)
        InterlockedIncrement(&m_RequestId);

    RequestObj->req_data_len = req->data_len;
    RequestObj->req_data_ptr = RequestData;

    RequestObj->rpl_data_len = 0;
    RequestObj->rpl_data_ptr = NULL;

    //
    //
    //

    rpl = (QUEUE_PUTREQ_RPL *)LONG_REPLY(sizeof(QUEUE_PUTREQ_RPL));
    if (! rpl) {
        HeapFree(m_heap, 0, RequestData);
        HeapFree(m_heap, 0, RequestObj);
        status = STATUS_INSUFFICIENT_RESOURCES;
    } else {

        rpl->req_id = RequestObj->request_id;

        List_Insert_After(&QueueObj->requests, NULL, RequestObj);

        if (QueueObj->server_event)
            SetEvent(QueueObj->server_event);
    }

finish:

    LeaveCriticalSection(&m_lock);

    if (hEvent)
        CloseHandle(hEvent);

    if (hProcess)
        CloseHandle(hProcess);

    if (QueueName)
        HeapFree(m_heap, 0, QueueName);

    if (! rpl)
        rpl = (QUEUE_PUTREQ_RPL *)SHORT_REPLY(status);

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// GetRplHandler
//---------------------------------------------------------------------------


MSG_HEADER *QueueServer::GetRplHandler(MSG_HEADER *msg, HANDLE idProcess)
{
    WCHAR *QueueName = NULL;
    HANDLE hProcess = NULL;
    ULONG status;
    QUEUE_GETRPL_RPL *rpl = NULL;

    EnterCriticalSection(&m_lock);

    QUEUE_GETRPL_REQ *req = (QUEUE_GETRPL_REQ *)msg;
    if (req->h.length < sizeof(QUEUE_GETRPL_REQ)) {
        status = STATUS_INVALID_PARAMETER;
        goto finish;
    }

    //
    //
    //

    status = OpenProcess(idProcess, &hProcess);
    if (! NT_SUCCESS(status))
        goto finish;

    QueueName = MakeQueueName(idProcess, req->queue_name, &status);
    if (! QueueName)
        goto finish;

    QUEUE_OBJ *QueueObj = (QUEUE_OBJ *)FindQueueObj(QueueName);
    if (! QueueObj) {
        status = STATUS_OBJECT_NAME_NOT_FOUND;
        goto finish;
    }

    //
    //
    //

    REQUEST_OBJ *RequestObj = (REQUEST_OBJ *)List_Head(&QueueObj->requests);
    while (RequestObj) {
        if (RequestObj->request_id == req->req_id)
            break;
        RequestObj = (REQUEST_OBJ *)List_Next(RequestObj);
    }

    if (! RequestObj) {
        status = STATUS_END_OF_FILE;
        goto finish;
    }

    if (RequestObj->client_pid != (ULONG)(ULONG_PTR)idProcess) {
        status = STATUS_ACCESS_DENIED;
        goto finish;
    }

    if ((! RequestObj->rpl_data_len) || (! RequestObj->rpl_data_ptr)) {
        DeleteRequestObj(&QueueObj->requests, RequestObj);
        status = STATUS_END_OF_FILE;
        goto finish;
    }

    //
    //
    //

    ULONG rpl_len = sizeof(QUEUE_GETRPL_RPL)
                  + RequestObj->rpl_data_len
                  + 8;
    rpl = (QUEUE_GETRPL_RPL *)LONG_REPLY(rpl_len);
    if (! rpl) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto finish;
    }

    rpl->data_len = RequestObj->rpl_data_len;
    memcpy(rpl->data, RequestObj->rpl_data_ptr, RequestObj->rpl_data_len);
    memzero(rpl->data + RequestObj->rpl_data_len, 8);

    DeleteRequestObj(&QueueObj->requests, RequestObj);

finish:

    LeaveCriticalSection(&m_lock);

    if (hProcess)
        CloseHandle(hProcess);

    if (QueueName)
        HeapFree(m_heap, 0, QueueName);

    if (! rpl)
        rpl = (QUEUE_GETRPL_RPL *)SHORT_REPLY(status);

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// OpenProcess
//---------------------------------------------------------------------------


LONG QueueServer::OpenProcess(
    HANDLE idProcess, HANDLE *out_hProcess, ULONG DesiredAccess)
{
    LONG status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    CLIENT_ID ClientId;
    InitializeObjectAttributes(&ObjectAttributes, NULL, 0, NULL, NULL);
    ClientId.UniqueThread = NULL;
    ClientId.UniqueProcess = idProcess;
    status = NtOpenProcess(
                out_hProcess, DesiredAccess, &ObjectAttributes, &ClientId);
    if (! NT_SUCCESS(status)) {
        *out_hProcess = NULL;
        if (status == STATUS_ACCESS_DENIED)
            status = STATUS_PRIVILEGE_NOT_HELD;
    }
    return status;
}


//---------------------------------------------------------------------------
// DuplicateEvent
//---------------------------------------------------------------------------


LONG QueueServer::DuplicateEvent(
    HANDLE hProcess, ULONG64 hEvent, HANDLE *out_hEvent)
{
    LONG status;
    if (! hEvent) {
        status = STATUS_SUCCESS;
        *out_hEvent = NULL;
    } else {
        status = NtDuplicateObject(hProcess, (HANDLE)(ULONG_PTR)hEvent,
                                   GetCurrentProcess(), out_hEvent,
                                   EVENT_MODIFY_STATE, 0, 0);
        if (! NT_SUCCESS(status)) {
            *out_hEvent = NULL;
            if (status == STATUS_ACCESS_DENIED)
                status = STATUS_PRIVILEGE_NOT_HELD;
        }
    }
    return status;
}


//---------------------------------------------------------------------------
// MakeQueueName
//---------------------------------------------------------------------------


WCHAR *QueueServer::MakeQueueName(
    HANDLE idProcess, WCHAR *req_name, ULONG *out_status)
{
    LONG status;
    ULONG ipc_len, req_name_len;
    WCHAR *name;
    BOOLEAN boxed_caller;

    req_name[QUEUE_NAME_MAXLEN - 1] = L'\0';
    req_name_len = (wcslen(req_name) + 1) * sizeof(WCHAR);

    if (SbieApi_QueryProcess(idProcess, NULL, NULL, NULL, NULL) == 0)
        boxed_caller = TRUE;
    else
        boxed_caller = FALSE;

    if (req_name[0] == L'*') {

        //
        // queue name starts with asterisk, don't prefix with ipc path,
        // and check if caller is inside sandbox
        //

        name = (WCHAR *)HeapAlloc(m_heap, 0, req_name_len + 8);
        if (! name) {
            *out_status = STATUS_INSUFFICIENT_RESOURCES;
            return NULL;
        }

        name[0] = L'*';
        name[1] = L'\\';
        memcpy(name + 2, req_name, req_name_len);

        *out_status = (boxed_caller ? STATUS_ACCESS_DENIED : STATUS_SUCCESS);

    } else if (! boxed_caller) {

        //
        // callers outside the sandbox can only use asterisk queues
        //

        *out_status = STATUS_ACCESS_DENIED;
        return NULL;

    } else {

        //
        // queue name does not start with asterisk, prefix ipc path for
        // calling process, return zero status on success
        //

        status = SbieApi_QueryProcessPath(
                    idProcess, NULL, NULL, NULL, NULL, NULL, &ipc_len);
        if (status != 0) {
            *out_status = STATUS_OBJECT_NAME_INVALID;
            return NULL;
        }

        name = (WCHAR *)HeapAlloc(m_heap, 0, ipc_len + req_name_len + 8);
        if (! name) {
            *out_status = STATUS_INSUFFICIENT_RESOURCES;
            return NULL;
        }

        status = SbieApi_QueryProcessPath(
                    idProcess, NULL, NULL, name, NULL, NULL, &ipc_len);
        if (status != 0) {
            HeapFree(m_heap, 0, name);
            *out_status = STATUS_OBJECT_NAME_INVALID;
            return NULL;
        }

        ipc_len = wcslen(name);
        name[ipc_len] = L'\\';
        memcpy(name + ipc_len + 1, req_name, req_name_len);

        *out_status = STATUS_SUCCESS;
    }

    return name;
}


//---------------------------------------------------------------------------
// FindQueueObj
//---------------------------------------------------------------------------


void *QueueServer::FindQueueObj(const WCHAR *QueueName)
{
    ULONG QueueNameLen = wcslen(QueueName);
    QUEUE_OBJ *QueueObj = (QUEUE_OBJ *)List_Head(&m_queues);
    while (QueueObj) {
        if (QueueObj->queue_name_len == QueueNameLen
                && _wcsicmp(QueueObj->queue_name, QueueName) == 0)
            break;
        QueueObj = (QUEUE_OBJ *)List_Next(QueueObj);
    }

    if (QueueObj) {
        bool ServerProcessIsActive = false;

        HANDLE hProcess = NULL;
        ULONG status = OpenProcess(QueueObj->server_pid, &hProcess,
                                   PROCESS_QUERY_INFORMATION);
        if (status == 0) {

            DWORD nExitCode = 0;

            if (GetExitCodeProcess(hProcess, &nExitCode) && nExitCode == STILL_ACTIVE)
            {
                FILETIME time1, time2, time3, time4;
                if (GetProcessTimes(hProcess, &time1, &time2, &time3, &time4)) {
                    time2 = QueueObj->server_create_time;
                    if (time1.dwHighDateTime == time2.dwHighDateTime &&
                        time1.dwLowDateTime == time2.dwLowDateTime) {

                        ServerProcessIsActive = true;
                    }
                }
            }
            CloseHandle(hProcess);
        }

        if (! ServerProcessIsActive) {
            DeleteQueueObj(QueueObj);
            QueueObj = NULL;
        }
    }

    return QueueObj;
}


//---------------------------------------------------------------------------
// NotifyHandler
//---------------------------------------------------------------------------


void QueueServer::NotifyHandler(HANDLE idProcess)
{
    QUEUE_OBJ *QueueObj;
    REQUEST_OBJ *RequestObj;

    EnterCriticalSection(&m_lock);

    QueueObj = (QUEUE_OBJ *)List_Head(&m_queues);
    while (QueueObj) {
        QUEUE_OBJ *QueueObjNext = (QUEUE_OBJ *)List_Next(QueueObj);

        if (QueueObj->server_pid == idProcess) {

            DeleteQueueObj(QueueObj);

        } else {

            RequestObj = (REQUEST_OBJ *)List_Head(&QueueObj->requests);
            while (RequestObj) {

                REQUEST_OBJ *RequestObjNext =
                    (REQUEST_OBJ *)List_Next(RequestObj);

                if (RequestObj->client_pid == (ULONG)(ULONG_PTR)idProcess) {

                    DeleteRequestObj(&QueueObj->requests, RequestObj);
                }

                RequestObj = RequestObjNext;
            }

        }

        QueueObj = QueueObjNext;
    }

    LeaveCriticalSection(&m_lock);
}


//---------------------------------------------------------------------------
// DeleteQueueObj
//---------------------------------------------------------------------------


void QueueServer::DeleteQueueObj(void *_QueueObj)
{
    QUEUE_OBJ *QueueObj = (QUEUE_OBJ *)_QueueObj;

    while (1) {

        REQUEST_OBJ *RequestObj =
                            (REQUEST_OBJ *)List_Head(&QueueObj->requests);
        if (! RequestObj)
            break;

        DeleteRequestObj(&QueueObj->requests, RequestObj);
    }

    if (QueueObj->server_event)
        NtClose(QueueObj->server_event);

    List_Remove(&m_queues, QueueObj);
    HeapFree(m_heap, 0, QueueObj);
}


//---------------------------------------------------------------------------
// DeleteRequestObj
//---------------------------------------------------------------------------


void QueueServer::DeleteRequestObj(LIST *RequestsList, void *_RequestObj)
{
    REQUEST_OBJ *RequestObj = (REQUEST_OBJ *)_RequestObj;
    if (RequestObj->client_event)
        NtClose(RequestObj->client_event);
    if (RequestObj->req_data_ptr)
        HeapFree(m_heap, 0, RequestObj->req_data_ptr);
    if (RequestObj->rpl_data_ptr)
        HeapFree(m_heap, 0, RequestObj->rpl_data_ptr);
    List_Remove(RequestsList, RequestObj);
    HeapFree(m_heap, 0, RequestObj);
}
