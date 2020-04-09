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
// Channel Proxy -- using PipeServer
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "namedpipeserver.h"
#include "namedpipewire.h"
#include "misc.h"
#include "core/dll/sbiedll.h"
#include "core/drv/api_defs.h"
#include "common/defines.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define NAMED_PIPE_DESIRED_ACCESS   \
    (GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE | FILE_READ_ATTRIBUTES)

#define NAMED_PIPE_SHARE_ACCESS     \
    (FILE_SHARE_READ | FILE_SHARE_WRITE)

#define NAMED_PIPE_CREATE_DISPOSITION   \
    (FILE_OPEN)

#define NAMED_PIPE_CREATE_OPTIONS \
    (FILE_NON_DIRECTORY_FILE | FILE_OPEN_NO_RECALL)


//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------


typedef struct _PROXY_PIPE {

    HANDLE hPipe;
    HANDLE hEvent;

} PROXY_PIPE;


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


NamedPipeServer::NamedPipeServer(PipeServer *pipeServer)
{
    m_pNtAlpcConnectPort =
        GetProcAddress(_Ntdll, "NtAlpcConnectPort");

    m_pNtAlpcSendWaitReceivePort =
        GetProcAddress(_Ntdll, "NtAlpcSendWaitReceivePort");

    m_ProxyHandle = new ProxyHandle(NULL, sizeof(PROXY_PIPE),
                                    CloseCallback, NULL);

    pipeServer->Register(MSGID_NAMED_PIPE, this, Handler);
}


//---------------------------------------------------------------------------
// CloseCallback
//---------------------------------------------------------------------------


void NamedPipeServer::CloseCallback(void *context, void *data)
{
    PROXY_PIPE *ProxyPipe = (PROXY_PIPE *)data;
    if (ProxyPipe->hEvent)
        NtClose(ProxyPipe->hEvent);
    if (ProxyPipe->hPipe)
        NtClose(ProxyPipe->hPipe);
}


//---------------------------------------------------------------------------
// Handler
//---------------------------------------------------------------------------


MSG_HEADER *NamedPipeServer::Handler(void *_this, MSG_HEADER *msg)
{
    NamedPipeServer *pThis = (NamedPipeServer *)_this;

    HANDLE idProcess = (HANDLE)(ULONG_PTR)PipeServer::GetCallerProcessId();

    if (msg->msgid == MSGID_NAMED_PIPE_NOTIFICATION) {
        pThis->NotifyHandler(idProcess);
        return NULL;
    }

    if (0 != SbieApi_QueryProcess(idProcess, NULL, NULL, NULL, NULL))
        return SHORT_REPLY(STATUS_ACCESS_DENIED);

    if (PipeServer::ImpersonateCaller(&msg) != 0)
        return msg;

    if (msg->msgid == MSGID_NAMED_PIPE_OPEN)
        return pThis->OpenHandler(msg, idProcess);

    if (msg->msgid == MSGID_NAMED_PIPE_CLOSE)
        return pThis->CloseHandler(msg, idProcess);

    if (msg->msgid == MSGID_NAMED_PIPE_SET)
        return pThis->SetHandler(msg, idProcess);

    if (msg->msgid == MSGID_NAMED_PIPE_READ)
        return pThis->ReadHandler(msg, idProcess);

    if (msg->msgid == MSGID_NAMED_PIPE_WRITE)
        return pThis->WriteHandler(msg, idProcess);

    if (msg->msgid == MSGID_NAMED_PIPE_LPC_CONNECT)
        return pThis->LpcConnectHandler(msg, idProcess);

    if (msg->msgid == MSGID_NAMED_PIPE_LPC_REQUEST)
        return pThis->LpcRequestHandler(msg, idProcess);

    if (msg->msgid == MSGID_NAMED_PIPE_ALPC_REQUEST)
        return pThis->AlpcRequestHandler(msg, idProcess);

    return NULL;
}


//---------------------------------------------------------------------------
// NotifyHandler
//---------------------------------------------------------------------------


void NamedPipeServer::NotifyHandler(HANDLE idProcess)
{
    m_ProxyHandle->ReleaseProcess(idProcess);
}


//---------------------------------------------------------------------------
// OpenHandler
//---------------------------------------------------------------------------


MSG_HEADER *NamedPipeServer::OpenHandler(MSG_HEADER *msg, HANDLE idProcess)
{
    NAMED_PIPE_OPEN_REQ *req = (NAMED_PIPE_OPEN_REQ *)msg;
    if (req->h.length < sizeof(NAMED_PIPE_OPEN_REQ))
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);

    bool permitted = false;
    if (_wcsicmp(req->name, L"lsarpc")      == 0 ||
        _wcsicmp(req->name, L"srvsvc")      == 0 ||
        _wcsicmp(req->name, L"wkssvc")      == 0 ||
        _wcsicmp(req->name, L"samr")        == 0 ||
        _wcsicmp(req->name, L"netlogon")    == 0)
        permitted = true;
    if (! permitted)
        return SHORT_REPLY(STATUS_ACCESS_DENIED);

    const ULONG rpl_len = sizeof(NAMED_PIPE_OPEN_RPL);
    NAMED_PIPE_OPEN_RPL *rpl = (NAMED_PIPE_OPEN_RPL *)LONG_REPLY(rpl_len);

    if (rpl) {

        PROXY_PIPE ProxyPipe;
        OBJECT_ATTRIBUTES objattrs;
        UNICODE_STRING objname;
        WCHAR pipename[160];
        IO_STATUS_BLOCK IoStatusBlock;

        if (req->server[0]) {
            wcscpy(pipename, L"\\device\\mup\\");
            wcscat(pipename, req->server);
            wcscat(pipename, L"\\PIPE\\");
        } else
            wcscpy(pipename, L"\\device\\namedpipe\\");
        wcscat(pipename, req->name);

        RtlInitUnicodeString(&objname, pipename);
        InitializeObjectAttributes(
            &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

        if (RestrictToken()) {

            rpl->h.status = NtCreateFile(
                &ProxyPipe.hPipe,
                NAMED_PIPE_DESIRED_ACCESS, &objattrs, &IoStatusBlock, NULL,
                0, NAMED_PIPE_SHARE_ACCESS, NAMED_PIPE_CREATE_DISPOSITION,
                req->create_options & NAMED_PIPE_CREATE_OPTIONS,
                NULL, 0);

        } else {

            rpl->h.status = STATUS_ACCESS_DENIED;
            memzero(&IoStatusBlock, sizeof(IoStatusBlock));
        }

        rpl->iosb.status = IoStatusBlock.Status;
        rpl->iosb.information = IoStatusBlock.Information;

        if (NT_SUCCESS(rpl->h.status)) {

            ProxyPipe.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
            if (! ProxyPipe.hEvent) {
                NtClose(ProxyPipe.hPipe);
                rpl->h.status = STATUS_INSUFFICIENT_RESOURCES;
            } else {

                rpl->handle = m_ProxyHandle->Create(idProcess, &ProxyPipe);
                if (! rpl->handle) {
                    // CloseCallback was already invoked
                    rpl->h.status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }
        }

        if (! NT_SUCCESS(rpl->h.status))
            rpl->handle = 0;
    }

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// CloseHandler
//---------------------------------------------------------------------------


MSG_HEADER *NamedPipeServer::CloseHandler(MSG_HEADER *msg, HANDLE idProcess)
{
    NAMED_PIPE_CLOSE_REQ *req = (NAMED_PIPE_CLOSE_REQ *)msg;
    if (req->h.length < sizeof(NAMED_PIPE_CLOSE_REQ))
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);

    ULONG status = 0;

    PROXY_PIPE *ProxyPipe =
        (PROXY_PIPE *)m_ProxyHandle->Find(idProcess, req->handle);

    if (ProxyPipe)
        m_ProxyHandle->Close(ProxyPipe);
    else
        status = STATUS_INVALID_HANDLE;

    return SHORT_REPLY(status);
}


//---------------------------------------------------------------------------
// SetHandler
//---------------------------------------------------------------------------


MSG_HEADER *NamedPipeServer::SetHandler(MSG_HEADER *msg, HANDLE idProcess)
{
    NAMED_PIPE_SET_REQ *req = (NAMED_PIPE_SET_REQ *)msg;
    if (req->h.length < sizeof(NAMED_PIPE_SET_REQ))
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);
    if (req->data_len > PIPE_MAX_DATA_LEN)
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);
    ULONG offset = FIELD_OFFSET(NAMED_PIPE_SET_REQ, data);
    if (offset + req->data_len > req->h.length)
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);

    PROXY_PIPE *ProxyPipe =
        (PROXY_PIPE *)m_ProxyHandle->Find(idProcess, req->handle);
    if (! ProxyPipe)
        return SHORT_REPLY(STATUS_INVALID_HANDLE);

    const ULONG rpl_len = sizeof(NAMED_PIPE_SET_RPL);
    NAMED_PIPE_SET_RPL *rpl = (NAMED_PIPE_SET_RPL *)LONG_REPLY(rpl_len);

    if (rpl) {

        IO_STATUS_BLOCK IoStatusBlock;

        rpl->h.status = NtSetInformationFile(
            ProxyPipe->hPipe, &IoStatusBlock,
            req->data, req->data_len, FilePipeInformation);

        rpl->iosb.status = IoStatusBlock.Status;
        rpl->iosb.information = IoStatusBlock.Information;
    }

    m_ProxyHandle->Release(ProxyPipe);
    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// ReadHandler
//---------------------------------------------------------------------------


MSG_HEADER *NamedPipeServer::ReadHandler(MSG_HEADER *msg, HANDLE idProcess)
{
    NAMED_PIPE_READ_REQ *req = (NAMED_PIPE_READ_REQ *)msg;
    if (req->h.length < sizeof(NAMED_PIPE_READ_REQ))
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);
    if (req->read_len > PAGE_SIZE * 4)
        return SHORT_REPLY(STATUS_BUFFER_OVERFLOW);

    PROXY_PIPE *ProxyPipe =
        (PROXY_PIPE *)m_ProxyHandle->Find(idProcess, req->handle);
    if (! ProxyPipe)
        return SHORT_REPLY(STATUS_INVALID_HANDLE);

    if (! RestrictToken()) {
        m_ProxyHandle->Release(ProxyPipe);
        return SHORT_REPLY(STATUS_ACCESS_DENIED);
    }

    const ULONG rpl_len = sizeof(NAMED_PIPE_READ_RPL) + req->read_len;
    NAMED_PIPE_READ_RPL *rpl = (NAMED_PIPE_READ_RPL *)LONG_REPLY(rpl_len);

    if (rpl) {

        IO_STATUS_BLOCK IoStatusBlock;
        LARGE_INTEGER li;
        li.QuadPart = 0;

        rpl->data_len = req->read_len;

        rpl->h.status = NtReadFile(
            ProxyPipe->hPipe, ProxyPipe->hEvent, NULL, NULL, &IoStatusBlock,
            rpl->data, rpl->data_len, &li, NULL);

        if (rpl->h.status == STATUS_PENDING) {
            ULONG WaitStatus = WaitForSingleObject(ProxyPipe->hEvent, 10000);
            if (WaitStatus != WAIT_OBJECT_0) {
                CancelIo(ProxyPipe->hPipe);
                rpl->h.status = STATUS_CANCELLED;
            } else
                rpl->h.status = IoStatusBlock.Status;
        }

        rpl->iosb.status = IoStatusBlock.Status;
        rpl->iosb.information = IoStatusBlock.Information;
    }

    m_ProxyHandle->Release(ProxyPipe);
    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// WriteHandler
//---------------------------------------------------------------------------


MSG_HEADER *NamedPipeServer::WriteHandler(MSG_HEADER *msg, HANDLE idProcess)
{
    NAMED_PIPE_WRITE_REQ *req = (NAMED_PIPE_WRITE_REQ *)msg;
    if (req->h.length < sizeof(NAMED_PIPE_WRITE_REQ))
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);
    if (req->data_len > PAGE_SIZE * 4)
        return SHORT_REPLY(STATUS_BUFFER_OVERFLOW);
    ULONG offset = FIELD_OFFSET(NAMED_PIPE_WRITE_REQ, data);
    if (offset + req->data_len > req->h.length)
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);

    PROXY_PIPE *ProxyPipe =
        (PROXY_PIPE *)m_ProxyHandle->Find(idProcess, req->handle);
    if (! ProxyPipe)
        return SHORT_REPLY(STATUS_INVALID_HANDLE);

    const ULONG rpl_len = sizeof(NAMED_PIPE_WRITE_RPL);
    NAMED_PIPE_WRITE_RPL *rpl = (NAMED_PIPE_WRITE_RPL *)LONG_REPLY(rpl_len);

    if (rpl) {

        IO_STATUS_BLOCK IoStatusBlock;
        LARGE_INTEGER li;
        li.QuadPart = 0;

        if (RestrictToken()) {

            rpl->h.status = NtWriteFile(
                ProxyPipe->hPipe, ProxyPipe->hEvent, NULL, NULL,
                &IoStatusBlock, req->data, req->data_len, &li, NULL);

        } else {

            rpl->h.status = STATUS_ACCESS_DENIED;
            memzero(&IoStatusBlock, sizeof(IoStatusBlock));
        }

        if (rpl->h.status == STATUS_PENDING) {
            ULONG WaitStatus = WaitForSingleObject(ProxyPipe->hEvent, 10000);
            if (WaitStatus != WAIT_OBJECT_0) {
                CancelIo(ProxyPipe->hPipe);
                rpl->h.status = STATUS_CANCELLED;
            } else
                rpl->h.status = IoStatusBlock.Status;
        }

        rpl->iosb.status = IoStatusBlock.Status;
        rpl->iosb.information = IoStatusBlock.Information;
    }

    m_ProxyHandle->Release(ProxyPipe);
    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// LpcConnectHandler
//---------------------------------------------------------------------------


MSG_HEADER *NamedPipeServer::LpcConnectHandler(
    MSG_HEADER *msg, HANDLE idProcess)
{
    ULONG status = STATUS_INVALID_PARAMETER;
    NAMED_PIPE_LPC_CONNECT_RPL *rpl = NULL;
    HANDLE hPort = NULL;

    //
    // validate request packet
    //

    NAMED_PIPE_LPC_CONNECT_REQ *req = (NAMED_PIPE_LPC_CONNECT_REQ *)msg;
    if (req->h.length < sizeof(NAMED_PIPE_LPC_CONNECT_REQ))
        goto finish;

    WCHAR port_name[96];
    port_name[0] = L'\0';
    if (_wcsicmp(req->name, L"ntsvcs")      == 0 ||
        _wcsicmp(req->name, L"plugplay")    == 0) {

            wcscpy(port_name, L"\\RPC Control\\");
            wcscat(port_name, req->name);
    }
    if (! port_name[0]) {
        status = STATUS_ACCESS_DENIED;
        goto finish;
    }

    if (req->info_len > PIPE_MAX_DATA_LEN)
        goto finish;

    ULONG offset = FIELD_OFFSET(NAMED_PIPE_LPC_CONNECT_REQ, info_data);
    if (offset + req->info_len > req->h.length)
        goto finish;

    //
    // open lpc port
    //

    ULONG max_msg_len;

    if (RestrictToken()) {

        UNICODE_STRING objname;
        SECURITY_QUALITY_OF_SERVICE qos;

        RtlInitUnicodeString(&objname, port_name);

        memzero(&qos, sizeof(SECURITY_QUALITY_OF_SERVICE));
        qos.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
        qos.ImpersonationLevel = SecurityImpersonation;
        qos.ContextTrackingMode = TRUE;
        qos.EffectiveOnly = TRUE;

        if (req->max_msg_len == -1) {       // old lpc

            status = NtConnectPort(
                &hPort, &objname, &qos, NULL, NULL,
                &max_msg_len, req->info_data, &req->info_len);

        } else {                            // new alpc

            OBJECT_ATTRIBUTES objattrs;
            ALPC_PORT_ATTRIBUTES alpc;

            memzero(&alpc, sizeof(ALPC_PORT_ATTRIBUTES));
            alpc.Flags = 0x10000;       // can-impersonate flag??
            memcpy(&alpc.SecurityQos, &qos, sizeof(qos));
            alpc.MaxMessageLength = req->max_msg_len;
            alpc.MaxPoolUsage = (ULONG_PTR)(ULONG)-1;
            alpc.MaxSectionSize = (ULONG_PTR)(ULONG)-1;
            alpc.MaxViewSize = (ULONG_PTR)(ULONG)-1;
            alpc.MaxTotalSectionSize = (ULONG_PTR)(ULONG)-1;

            memzero(&objattrs, sizeof(OBJECT_ATTRIBUTES));
            objattrs.Length = sizeof(OBJECT_ATTRIBUTES);

            if (m_pNtAlpcConnectPort) {

                status = ((P_NtAlpcConnectPort)m_pNtAlpcConnectPort)(
                    &hPort, &objname, &objattrs, &alpc,
                    0x20000,            // sync-connection flag??
                    NULL, NULL, NULL, NULL, NULL, NULL);

            } else
                status = STATUS_NOT_SUPPORTED;
        }

    } else
        status = STATUS_ACCESS_DENIED;

    if (! NT_SUCCESS(status))
        goto finish;

    //
    // prepare output
    //

    ULONG rpl_len = sizeof(NAMED_PIPE_LPC_CONNECT_RPL)
                  + req->info_len;
    rpl = (NAMED_PIPE_LPC_CONNECT_RPL *)LONG_REPLY(rpl_len);
    if (! rpl) {
        status = STATUS_INSUFFICIENT_RESOURCES;

    } else {

        PROXY_PIPE ProxyPipe;
        ProxyPipe.hPipe = hPort;
        ProxyPipe.hEvent = NULL;

        rpl->handle = m_ProxyHandle->Create(idProcess, &ProxyPipe);
        if (! rpl->handle)
            status = STATUS_INSUFFICIENT_RESOURCES;
        else {

            if (req->max_msg_len == -1) {       // old lpc

                rpl->max_msg_len = max_msg_len;
                rpl->info_len = req->info_len;
                memcpy(rpl->info_data, req->info_data, req->info_len);
            }

            status = STATUS_SUCCESS;
            hPort = NULL;
        }
    }

    //
    // finish
    //

finish:

    if (! rpl)
        rpl = (NAMED_PIPE_LPC_CONNECT_RPL *)SHORT_REPLY(status);
    else
        rpl->h.status = status;

    if (hPort)
        NtClose(hPort);

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// LpcRequestHandler
//---------------------------------------------------------------------------


MSG_HEADER *NamedPipeServer::LpcRequestHandler(
    MSG_HEADER *msg, HANDLE idProcess)
{
    ULONG status = STATUS_INVALID_PARAMETER;
    NAMED_PIPE_LPC_REQUEST_RPL *rpl = NULL;

    //
    // validate request packet
    //

    NAMED_PIPE_LPC_REQUEST_REQ *req = (NAMED_PIPE_LPC_REQUEST_REQ *)msg;
    if (req->h.length < sizeof(NAMED_PIPE_LPC_REQUEST_REQ))
        goto finish;

    PORT_MESSAGE *port_msg = (PORT_MESSAGE *)req->data;

    ULONG offset = FIELD_OFFSET(NAMED_PIPE_LPC_REQUEST_REQ, data);
    if (offset + (ULONG)port_msg->u1.s1.TotalLength > req->h.length)
        goto finish;

    ULONG info_len = 0;
    PORT_DATA_INFO *info = NULL;

    if (port_msg->u2.s2.DataInfoOffset) {

        //
        // an LPC data info is used to send/receive more data than
        // there is room in a single PORT_MESSAGE.  we can handle
        // at most one data info entry.
        //

        offset = port_msg->u2.s2.DataInfoOffset;
        if (offset >= (ULONG)port_msg->u1.s1.TotalLength)
            goto finish;
        info = (PORT_DATA_INFO *)((UCHAR *)port_msg + offset);
        if (info->NumDataInfo != 1)
            goto finish;
        info_len = info->BufferLen;
        if (info_len > PIPE_MAX_DATA_LEN)
            goto finish;
        offset = FIELD_OFFSET(NAMED_PIPE_LPC_REQUEST_REQ, info);
        if (offset + info_len > req->h.length)
            goto finish;
    }

    //
    // issue request on lpc port
    //

    PROXY_PIPE *ProxyPipe =
        (PROXY_PIPE *)m_ProxyHandle->Find(idProcess, req->handle);
    if (! ProxyPipe) {
        status = STATUS_INVALID_HANDLE;
        goto finish;
    }

    ULONG rpl_len = sizeof(NAMED_PIPE_LPC_REQUEST_REQ) + info_len + 64;
    rpl = (NAMED_PIPE_LPC_REQUEST_RPL *)LONG_REPLY(rpl_len);
    if (! rpl)
        status = STATUS_INSUFFICIENT_RESOURCES;

    else if (! RestrictToken())
        status = STATUS_ACCESS_DENIED;

    else {

        if (info) {
            info->Buffer = rpl->info;
            memcpy(info->Buffer, req->info, info_len);
        }

        status = NtRequestWaitReplyPort(
            ProxyPipe->hPipe, port_msg, (PORT_MESSAGE *)rpl->data);
    }

    m_ProxyHandle->Release(ProxyPipe);

    //
    // finish
    //

finish:

    if (! rpl)
        rpl = (NAMED_PIPE_LPC_REQUEST_RPL *)SHORT_REPLY(status);
    else
        rpl->h.status = status;

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// AlpcRequestHandler
//---------------------------------------------------------------------------


MSG_HEADER *NamedPipeServer::AlpcRequestHandler(
    MSG_HEADER *msg, HANDLE idProcess)
{
    ULONG status = STATUS_INVALID_PARAMETER;
    NAMED_PIPE_ALPC_REQUEST_RPL *rpl = NULL;
    PORT_MESSAGE *rcv_msg = NULL;

    //
    // validate request packet
    //

    if (! m_pNtAlpcSendWaitReceivePort) {
        status = STATUS_NOT_SUPPORTED;
        goto finish;
    }

    NAMED_PIPE_ALPC_REQUEST_REQ *req = (NAMED_PIPE_ALPC_REQUEST_REQ *)msg;
    if (req->h.length < sizeof(NAMED_PIPE_ALPC_REQUEST_REQ))
        goto finish;
    if (req->h.length > PIPE_MAX_DATA_LEN)
        goto finish;

    PORT_MESSAGE *snd_msg = (PORT_MESSAGE *)req->data;

    ULONG msg_len = snd_msg->u1.s1.TotalLength;
    if (msg_len > PIPE_MAX_DATA_LEN / 2)
        goto finish;
    ULONG offset = FIELD_OFFSET(NAMED_PIPE_ALPC_REQUEST_REQ, data);
    if (offset + msg_len > req->h.length)
        goto finish;

    if (snd_msg->u2.s2.DataInfoOffset)
        goto finish;

    if (req->msg_len > PIPE_MAX_DATA_LEN / 2)
        goto finish;

    //
    // we don't need more than the two first ULONG fields of the
    // input (send) message view buffer.  note that a message view
    // buffer is separate from the message data
    //

    ALPC_MESSAGE_VIEW view;
    memzero(&view, sizeof(ALPC_MESSAGE_VIEW));
    view.SendFlags = req->view[0];
    view.ReceiveFlags = req->view[1];

    if ((view.SendFlags & 0x9FFFFFFF) || (view.ReceiveFlags & 0x9FFFFFFF)) {
        // we only accept 0x20000000 or 0x40000000 or 0x60000000
        goto finish;
    }

    //
    // issue request on lpc port
    //

    const P_NtAlpcSendWaitReceivePort pNtAlpcSendWaitReceivePort =
        (P_NtAlpcSendWaitReceivePort)m_pNtAlpcSendWaitReceivePort;

    PROXY_PIPE *ProxyPipe =
        (PROXY_PIPE *)m_ProxyHandle->Find(idProcess, req->handle);
    if (! ProxyPipe) {
        status = STATUS_INVALID_HANDLE;
        goto finish;
    }

    msg_len = snd_msg->u1.s1.TotalLength;
    if (req->msg_len > msg_len)
        msg_len = req->msg_len;

    rcv_msg = (PORT_MESSAGE *)
                    HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, msg_len);
    if (! rcv_msg)
        status = STATUS_INSUFFICIENT_RESOURCES;
    else if (! RestrictToken())
        status = STATUS_ACCESS_DENIED;
    else {

        msg_len = req->msg_len;

        status = pNtAlpcSendWaitReceivePort(ProxyPipe->hPipe,
            0x20000,            // sync-connection flag??
            snd_msg, &view, rcv_msg, &msg_len, &view, NULL);

        if (NT_SUCCESS(status) && (
                (msg_len > PIPE_MAX_DATA_LEN / 2)
             || (rcv_msg->u1.s1.TotalLength > PIPE_MAX_DATA_LEN / 2))) {

            status = STATUS_BUFFER_OVERFLOW;
        }
    }

    //
    // prepare response
    //

    if (NT_SUCCESS(status)) {

        UCHAR *ViewBase = NULL;
        ULONG  ViewSize = 0;

        if ((view.ReceiveFlags & 0x40000000)
                            && view.u.s2.ViewBase && view.u.s2.ViewSize) {

            //
            // if the reply message view buffer has bit 30 (0x40000000)
            // set then a section view has been mapped into the process
            // and this section view contains most of the reply data.
            //
            // we will copy the contents of the mapped view into our
            // reply -- see below.
            //
            // note that the section view buffer mysterously gets
            // unmapped when the calling sandboxed program terminates,
            // so we don't bother trying to keep track of it.
            //

            ViewBase = (UCHAR *)view.u.s2.ViewBase;
            ViewSize = view.u.s2.ViewSize;
            if (ViewSize > PIPE_MAX_DATA_LEN / 2) {
                ViewSize = PIPE_MAX_DATA_LEN / 2;
                status = STATUS_BUFFER_OVERFLOW;
            }

            // { WCHAR txt[128]; wsprintf(txt, L"View Base = %p\n", ViewBase); OutputDebugString(txt); }
        }

        //
        // allocate and populate our reply
        //

        ULONG rpl_len = sizeof(NAMED_PIPE_ALPC_REQUEST_REQ)
                      + msg_len + ViewSize + 8;
        rpl = (NAMED_PIPE_ALPC_REQUEST_RPL *)LONG_REPLY(rpl_len);
        if (! rpl)
            status = STATUS_INSUFFICIENT_RESOURCES;
        else {

            rpl->msg_len = msg_len;

            rpl->view[0] = view.SendFlags;
            rpl->view[1] = view.ReceiveFlags;

            memcpy(&rpl->data, rcv_msg, msg_len);

            if (ViewSize) {

                rpl->view[2] = ViewSize;
                offset = (msg_len + 7) & (~7);
                memcpy(&rpl->data[offset], ViewBase, ViewSize);

            } else
                rpl->view[2] = view.u.s1.ReplyLength;
        }
    }

    m_ProxyHandle->Release(ProxyPipe);

    //
    // finish
    //

finish:

    if (rcv_msg)
        HeapFree(GetProcessHeap(), 0, rcv_msg);

    if (! rpl)
        rpl = (NAMED_PIPE_ALPC_REQUEST_RPL *)SHORT_REPLY(status);
    else
        rpl->h.status = status;

    return (MSG_HEADER *)rpl;
}
