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
// Sandboxie DLL (SbieDll) to Sandboxie Service (SbieSvc) RPC Interface
//---------------------------------------------------------------------------


#include "dll.h"
#include "core/svc/QueueWire.h"
#include "core/svc/SbieIniWire.h"
#include "core/svc/ProcessWire.h"
#include "common/my_version.h"


//---------------------------------------------------------------------------
// SbieDll_PortName
//---------------------------------------------------------------------------


_FX const WCHAR *SbieDll_PortName(void)
{
    static const WCHAR *_name = L"\\RPC Control\\" SBIESVC L"Port";
    return _name;
}


//---------------------------------------------------------------------------
// SbieDll_IsWow64
//---------------------------------------------------------------------------


_FX BOOLEAN SbieDll_IsWow64(void)
{
    //
    // in a sandbox process, Dll_IsWow64 is initialized during
    // Dll_Ordinal1.  for a process outside the sandbox, we
    // initialize the variable here
    //

#ifndef _WIN64

    static BOOLEAN init = FALSE;
    typedef BOOL (*P_IsWow64Process)(HANDLE, BOOL *);

    if (! init) {

        P_IsWow64Process __sys_IsWow64Process =
            (P_IsWow64Process)GetProcAddress(Dll_Kernel32, "IsWow64Process");
        if (__sys_IsWow64Process) {
            BOOL x64;
            if (! __sys_IsWow64Process(GetCurrentProcess(), &x64))
                x64 = FALSE;
            if (x64)
                Dll_IsWow64 = TRUE;
        }

        init = TRUE;
    }

#endif ! _WIN64

    return Dll_IsWow64;
}


//---------------------------------------------------------------------------
// SbieDll_ConnectPort
//---------------------------------------------------------------------------


_FX BOOLEAN SbieDll_ConnectPort(BOOLEAN Silent)
{
    static BOOLEAN ErrorReported = FALSE;

    THREAD_DATA *data = Dll_GetTlsData(NULL);
    if (! data->PortHandle) {

        NTSTATUS status;
        SECURITY_QUALITY_OF_SERVICE QoS;
        UNICODE_STRING PortName;

        QoS.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
        QoS.ImpersonationLevel = SecurityImpersonation;
        QoS.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
        QoS.EffectiveOnly = TRUE;

        RtlInitUnicodeString(&PortName, SbieDll_PortName());

        status = NtConnectPort(
            &data->PortHandle, &PortName, &QoS,
            NULL, NULL, &data->MaxDataLen, NULL, NULL);

        if (! NT_SUCCESS(status)) {
            if (! ErrorReported) {
                if (! Silent)
                    SbieApi_Log(2203, L"connect %08X", status);
                ErrorReported = TRUE;
            }
            return FALSE;
        }

        NtRegisterThreadTerminatePort(data->PortHandle);

        //
        // compute sizes and offsets
        //

        data->SizeofPortMsg = sizeof(PORT_MESSAGE);

        if (! Dll_BoxName)
            SbieDll_IsWow64();

        if (Dll_IsWow64) {

            //
            // if we are running in 32-bit mode on a 64-bit OS, we have to
            // adjust sizes and offsets to account for the longer fields
            // ClientId and ClientViewSize.  Note that the 64-bit SbieSvc
            // is already adjusted by nature of being a 64-bit process
            //

            data->SizeofPortMsg += sizeof(ULONG) * 4;
        }

        data->MaxDataLen -= data->SizeofPortMsg;
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// SbieDll_CallServer
//---------------------------------------------------------------------------


_FX MSG_HEADER *SbieDll_CallServer(MSG_HEADER *req)
{
    static volatile ULONG last_sequence = 0;
    UCHAR curr_sequence;
    THREAD_DATA *data = Dll_GetTlsData(NULL);
    UCHAR spaceReq[MAX_PORTMSG_LENGTH], spaceRpl[MAX_PORTMSG_LENGTH];
    NTSTATUS status;
    PORT_MESSAGE *msg;
    UCHAR *buf, *msg_data;
    ULONG buf_len, send_len;
    MSG_HEADER *rpl;

    //
    // connect to the service API port.  note that we don't issue
    // an error message for a few specific request codes
    //

    if (! data->PortHandle) {
        BOOLEAN Silent = (req->msgid == MSGID_SBIE_INI_GET_VERSION ||
                          req->msgid == MSGID_SBIE_INI_GET_USER ||
                          req->msgid == MSGID_PROCESS_CHECK_INIT_COMPLETE);
        if (! SbieDll_ConnectPort(Silent))
            return NULL;
    }

    //
    // transmit the request message on the port.  LPC ports are designed
    // for short messages so we have to break the message into chunks.
    //

    curr_sequence = (UCHAR) InterlockedIncrement(&last_sequence);

    buf = (UCHAR *)req;
    buf_len = req->length;

    while (buf_len) {

        msg = (PORT_MESSAGE *)spaceReq;

        memzero(msg, data->SizeofPortMsg);
        msg_data = (UCHAR *)msg + data->SizeofPortMsg;

        if (buf_len > data->MaxDataLen)
            send_len = data->MaxDataLen;
        else
            send_len = buf_len;

        msg->u1.s1.DataLength = (USHORT)send_len;
        msg->u1.s1.TotalLength = (USHORT)(data->SizeofPortMsg + send_len);

        memcpy(msg_data, buf, send_len);

        if (buf == (UCHAR *)req) {

            //
            // a service message must be shorter than 0x00FFFFFF bytes
            // (as defined in core/svc/PipeServer.h) so we can use the
            // high byte of MSG_HEADER.length (offset 3 in the first chunk)
            // to store a verification sequenece number
            //

            msg_data[3] = curr_sequence;
        }

        buf += send_len;
        buf_len -= send_len;

        //
        // send the chunk on the LPC port and wait for acknowledgement.
        // while the service is collecting the incoming chunks on its end,
        // it replies with zero length chunks.  when sending the last chunk,
        // we should get a non-zero reply which contains the first chunk
        // of the response message from the service
        //

        status = NtRequestWaitReplyPort(data->PortHandle,
                        (PORT_MESSAGE *)spaceReq, (PORT_MESSAGE *)spaceRpl);
        if (! NT_SUCCESS(status))
            break;

        msg = (PORT_MESSAGE *)spaceRpl;

        if (buf_len && msg->u1.s1.DataLength) {
            SbieApi_Log(2203, L"early reply");
            return NULL;
        }
    }

    if (! NT_SUCCESS(status)) {

        NtClose(data->PortHandle);
        data->PortHandle = NULL;

        SbieApi_Log(2203, L"request %08X", status);
        return NULL;
    }

    //
    // inspect the first chunks of the response message,
    // it should have a matching sequence number, and valid length
    //

    msg = (PORT_MESSAGE *)spaceRpl;

    msg_data = ((UCHAR *)msg + data->SizeofPortMsg);

    if (msg->u1.s1.DataLength >= sizeof(MSG_HEADER)) {

        if (msg_data[3] != curr_sequence) {
            SbieApi_Log(2203, L"mismatched reply");
            return NULL;
        }

        msg_data[3] = 0;
        buf_len = ((MSG_HEADER *)msg_data)->length;

    } else
        buf_len = 0;

    if (buf_len == 0) {
        SbieApi_Log(2203, L"null reply (msg %08X len %d)",
                    req->msgid, req->length);
        return NULL;
    }

    //
    // collect the chunks of the response message.  we have to keep sending
    // short dummy LPC chunks on the port in order to receive the next chunk
    // of the response
    //

    rpl = Dll_AllocTemp(buf_len + 8);
    buf = (UCHAR *)rpl;

    while (1) {

        ULONG buf_len_plus_msg = (ULONG)(ULONG_PTR)(buf - (UCHAR *)rpl)
                               + msg->u1.s1.DataLength;
        if (buf_len_plus_msg > buf_len)
            status = STATUS_PORT_MESSAGE_TOO_LONG;
        else {

            msg_data = ((UCHAR *)msg + data->SizeofPortMsg);
            memcpy(buf, msg_data, msg->u1.s1.DataLength);

            buf += msg->u1.s1.DataLength;
            if ((ULONG_PTR)(buf - (UCHAR *)rpl) >= buf_len)
                break;

            msg = (PORT_MESSAGE *)spaceReq;

            memzero(msg, data->SizeofPortMsg);
            msg->u1.s1.TotalLength = (USHORT)data->SizeofPortMsg;

            status = NtRequestWaitReplyPort(data->PortHandle,
                        (PORT_MESSAGE *)spaceReq, (PORT_MESSAGE *)spaceRpl);

            msg = (PORT_MESSAGE *)spaceRpl;
        }

        if (! NT_SUCCESS(status)) {

            Dll_Free(rpl);

            NtClose(data->PortHandle);
            data->PortHandle = NULL;

            SbieApi_Log(2203, L"reply %08X", status);
            return NULL;
        }
    }

    memzero(buf, 8);
    return rpl;
}


//---------------------------------------------------------------------------
// SbieDll_CallServerQueue
//---------------------------------------------------------------------------


_FX void *SbieDll_CallServerQueue(const WCHAR* queue, void *req, ULONG req_len, ULONG rpl_min_len)
{
	//static ULONG _Ticks = 0;
	//static ULONG _Ticks1 = 0;
	WCHAR QueueName[64];
	NTSTATUS status;
	ULONG req_id;
	ULONG data_len;
	void *data;
	HANDLE event;

	//ULONG Ticks0 = GetTickCount();

	/*if (1) {
		WCHAR txt[128];
		Sbie_snwprintf(txt, 128, L"Request command is %08X\n", *(ULONG *)req);
		OutputDebugString(txt);
	}*/

	Sbie_snwprintf(QueueName, 64, L"*%s_%08X", queue, Dll_SessionId);

	status = SbieDll_QueuePutReq(QueueName, req, req_len, &req_id, &event);
	if (NT_SUCCESS(status)) {

		if (WaitForSingleObject(event, 60 * 1000) != 0)
			status = STATUS_TIMEOUT;

		CloseHandle(event);
	}

	if (status == 0) {

		status = SbieDll_QueueGetRpl(QueueName, req_id, &data, &data_len);

		if (NT_SUCCESS(status)) {

			if (data_len >= sizeof(ULONG) && *(ULONG *)data) {

				status = *(ULONG *)data;

			}
			else if (data_len >= rpl_min_len) {

				/*_Ticks += GetTickCount() - Ticks0;
				if (_Ticks > _Ticks1 + 1000) {
					WCHAR txt[128];
					Sbie_snwprintf(txt, 128, L"Already spent %d ticks in gui\n", _Ticks);
					OutputDebugString(txt);
					_Ticks1 = _Ticks;
				}*/

				return data;

			}
			else
				status = STATUS_INFO_LENGTH_MISMATCH;

			Dll_Free(data);
		}
	}

	SbieApi_Log(2203, L"%S - %S [%08X]", QueueName, Dll_ImageName, status);
	SetLastError(ERROR_SERVER_DISABLED);
	return NULL;
}


//---------------------------------------------------------------------------
// SbieDll_FreeMem
//---------------------------------------------------------------------------


_FX void SbieDll_FreeMem(void *data)
{
    if (data)
        Dll_Free(data);
}


//---------------------------------------------------------------------------
// SbieDll_QueueCreate
//---------------------------------------------------------------------------


_FX ULONG SbieDll_QueueCreate(const WCHAR *QueueName,
                              HANDLE *out_EventHandle)
{
    NTSTATUS status;
    QUEUE_CREATE_REQ req;
    QUEUE_CREATE_RPL *rpl;

    req.h.length = sizeof(QUEUE_CREATE_REQ);
    req.h.msgid  = MSGID_QUEUE_CREATE;
    wcscpy(req.queue_name, QueueName);
    req.event_handle =
        (ULONG64)(ULONG_PTR)CreateEvent(NULL, FALSE, FALSE, NULL);

    if (! req.event_handle)
        status = STATUS_UNSUCCESSFUL;
    else {

        rpl = (QUEUE_CREATE_RPL *)SbieDll_CallServer(&req.h);
        if (! rpl)
            status = STATUS_SERVER_DISABLED;
        else {
            status = rpl->h.status;
            Dll_Free(rpl);
        }

        if (! NT_SUCCESS(status))
            CloseHandle((HANDLE)(ULONG_PTR)req.event_handle);
        else {

            *out_EventHandle = (HANDLE)(ULONG_PTR)req.event_handle;
        }
    }

    return status;
}


//---------------------------------------------------------------------------
// SbieDll_QueueGetReq
//---------------------------------------------------------------------------


_FX ULONG SbieDll_QueueGetReq(const WCHAR *QueueName,
                              ULONG *out_ClientPid,
                              ULONG *out_ClientTid,
                              ULONG *out_RequestId,
                              void **out_DataPtr,
                              ULONG *out_DataLen)
{
    NTSTATUS status;
    QUEUE_GETREQ_REQ req;
    QUEUE_GETREQ_RPL *rpl;

    req.h.length = sizeof(QUEUE_GETREQ_REQ);
    req.h.msgid  = MSGID_QUEUE_GETREQ;
    wcscpy(req.queue_name, QueueName);

    rpl = (QUEUE_GETREQ_RPL *)SbieDll_CallServer(&req.h);
    if (! rpl)
        status = STATUS_SERVER_DISABLED;
    else {

        status = rpl->h.status;
        if (NT_SUCCESS(status)) {

            if (out_ClientPid)
                *out_ClientPid = rpl->client_pid;
            if (out_ClientTid)
                *out_ClientTid = rpl->client_tid;
            if (out_RequestId)
                *out_RequestId = rpl->req_id;
            if (out_DataLen)
                *out_DataLen = rpl->data_len;
            if (out_DataPtr) {
                *out_DataPtr = Dll_Alloc(rpl->data_len);
                memcpy(*out_DataPtr, rpl->data, rpl->data_len);
            }
        }

        Dll_Free(rpl);
    }

    if (! NT_SUCCESS(status)) {
        if (out_ClientPid)
            *out_ClientPid = 0;
        if (out_RequestId)
            *out_RequestId = 0;
        if (out_DataLen)
            *out_DataLen = 0;
        if (out_DataPtr)
            *out_DataPtr = NULL;
    }

    return status;
}


//---------------------------------------------------------------------------
// SbieDll_QueuePutRpl
//---------------------------------------------------------------------------


_FX ULONG SbieDll_QueuePutRpl(const WCHAR *QueueName,
                              ULONG RequestId,
                              void *DataPtr,
                              ULONG DataLen)
{
    NTSTATUS status;
    ULONG req_len;
    QUEUE_PUTRPL_REQ *req;
    QUEUE_PUTRPL_RPL *rpl;

    req_len = sizeof(QUEUE_PUTRPL_REQ) + DataLen;
    req = Dll_Alloc(req_len);
    req->h.length = req_len;
    req->h.msgid  = MSGID_QUEUE_PUTRPL;
    wcscpy(req->queue_name, QueueName);
    req->req_id = RequestId;
    req->data_len = DataLen;
    memcpy(req->data, DataPtr, DataLen);

    rpl = (QUEUE_PUTRPL_RPL *)SbieDll_CallServer(&req->h);
    if (! rpl)
        status = STATUS_SERVER_DISABLED;
    else {

        status = rpl->h.status;
        Dll_Free(rpl);
    }

    Dll_Free(req);

    return status;
}


//---------------------------------------------------------------------------
// SbieDll_QueuePutReq
//---------------------------------------------------------------------------


_FX ULONG SbieDll_QueuePutReq(const WCHAR *QueueName,
                              void *DataPtr,
                              ULONG DataLen,
                              ULONG *out_RequestId,
                              HANDLE *out_EventHandle)
{
    NTSTATUS status;
    ULONG req_len;
    QUEUE_PUTREQ_REQ *req;
    QUEUE_PUTREQ_RPL *rpl;

    req_len = sizeof(QUEUE_PUTREQ_REQ) + DataLen;
    req = Dll_Alloc(req_len);
    req->h.length = req_len;
    req->h.msgid  = MSGID_QUEUE_PUTREQ;
    wcscpy(req->queue_name, QueueName);
    req->data_len = DataLen;
    memcpy(req->data, DataPtr, DataLen);

    req->event_handle =
        (ULONG64)(ULONG_PTR)CreateEvent(NULL, FALSE, FALSE, NULL);
    if (! req->event_handle)
        status = STATUS_UNSUCCESSFUL;
    else {

        rpl = (QUEUE_PUTREQ_RPL *)SbieDll_CallServer(&req->h);
        if (! rpl)
            status = STATUS_SERVER_DISABLED;
        else {

            status = rpl->h.status;
            if (NT_SUCCESS(status)) {

                if (out_RequestId)
                    *out_RequestId = rpl->req_id;
                if (out_EventHandle)
                    *out_EventHandle = (HANDLE)(ULONG_PTR)req->event_handle;
            }

            Dll_Free(rpl);
        }
    }

    Dll_Free(req);

    if (! NT_SUCCESS(status)) {

        if (out_RequestId)
            *out_RequestId = 0;
        if (out_EventHandle)
            *out_EventHandle = NULL;
    }

    return status;
}


//---------------------------------------------------------------------------
// SbieDll_QueueGetRpl
//---------------------------------------------------------------------------


_FX ULONG SbieDll_QueueGetRpl(const WCHAR *QueueName,
                                          ULONG RequestId,
                                          void **out_DataPtr,
                                          ULONG *out_DataLen)
{
    NTSTATUS status;
    QUEUE_GETRPL_REQ req;
    QUEUE_GETRPL_RPL *rpl;

    req.h.length = sizeof(QUEUE_GETRPL_REQ);
    req.h.msgid  = MSGID_QUEUE_GETRPL;
    wcscpy(req.queue_name, QueueName);
    req.req_id = RequestId;

    rpl = (QUEUE_GETRPL_RPL *)SbieDll_CallServer(&req.h);
    if (! rpl)
        status = STATUS_SERVER_DISABLED;
    else {

        status = rpl->h.status;
        if (NT_SUCCESS(status)) {

            if (out_DataLen)
                *out_DataLen = rpl->data_len;
            if (out_DataPtr) {
                *out_DataPtr = Dll_Alloc(rpl->data_len);
                memcpy(*out_DataPtr, rpl->data, rpl->data_len);
            }
        }

        Dll_Free(rpl);
    }

    if (! NT_SUCCESS(status)) {
        if (out_DataLen)
            *out_DataLen = 0;
        if (out_DataPtr)
            *out_DataPtr = NULL;
    }

    return status;
}


//---------------------------------------------------------------------------
// SbieDll_UpdateConf
//---------------------------------------------------------------------------


_FX ULONG SbieDll_UpdateConf(
    WCHAR OpCode, const WCHAR *Password, const WCHAR *Section,
    const WCHAR *Setting, const WCHAR *Value)
{
    SBIE_INI_SETTING_REQ *req;
    MSG_HEADER *rpl;
    ULONG ValueLen, RequestLen;
    ULONG status;

    OpCode = towlower(OpCode);
    if (OpCode != L's' && OpCode != L'a' && OpCode != L'i' && OpCode != L'd')
        return STATUS_INVALID_PARAMETER;
    if ((! Section) || (! Setting))
        return STATUS_INVALID_PARAMETER;

    if (Value)
        ValueLen = (wcslen(Value) + 1) * sizeof(WCHAR);
    else
        ValueLen = 0;
    RequestLen = sizeof(SBIE_INI_SETTING_REQ) + ValueLen;
    req = Dll_Alloc(RequestLen);
    if (! req)
        return STATUS_INSUFFICIENT_RESOURCES;

    req->h.length = RequestLen;
    if (OpCode == L's')
        req->h.msgid  = MSGID_SBIE_INI_SET_SETTING;
    else if (OpCode == L'a')
        req->h.msgid  = MSGID_SBIE_INI_ADD_SETTING;
    else if (OpCode == L'i')
        req->h.msgid  = MSGID_SBIE_INI_INS_SETTING;
    else if (OpCode == L'd')
        req->h.msgid  = MSGID_SBIE_INI_DEL_SETTING;
    else
        return STATUS_INVALID_PARAMETER;

    if (Password)
        wcscpy(req->password, Password);
    else
        req->password[0] = L'\0';

    wcscpy(req->section, Section);
    wcscpy(req->setting, Setting);
    if (Value)
        memcpy(req->value, Value, ValueLen);
    else
        req->value[0] = L'\0';
    req->value_len = wcslen(req->value);

    rpl = (MSG_HEADER *)SbieDll_CallServer(&req->h);

    if (! rpl)
        status = STATUS_INSUFFICIENT_RESOURCES;
    else {
        status = rpl->status;
        Dll_Free(rpl);
    }

    Dll_Free(req);
    return status;
}


//---------------------------------------------------------------------------
// SbieDll_RunSandboxed
//---------------------------------------------------------------------------


_FX BOOL SbieDll_RunSandboxed(
    const WCHAR *box_name, const WCHAR *cmd, const WCHAR *dir,
    ULONG creation_flags, STARTUPINFO *si, PROCESS_INFORMATION *pi)
{
    PROCESS_RUN_SANDBOXED_REQ *req;
    PROCESS_RUN_SANDBOXED_RPL *rpl;
    WCHAR *env, *ptr;
    ULONG req_len, cmd_len, dir_len, env_len;
    ULONG err;
    BOOL ok;

    //
    // prepare request packet
    //

    if ((! cmd) || (! dir)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    cmd_len = wcslen(cmd);
    dir_len = wcslen(dir);
    env = File_AllocAndInitEnvironment(NULL, TRUE, TRUE, &env_len);
    env_len /= sizeof(WCHAR);

    req_len = sizeof(PROCESS_RUN_SANDBOXED_REQ)
            + (cmd_len + dir_len + env_len + 8) * sizeof(WCHAR);
    req = Dll_AllocTemp(req_len);

    req->h.length = req_len;
    req->h.msgid = MSGID_PROCESS_RUN_SANDBOXED;
    wcscpy(req->boxname, box_name);
    req->si_flags = si->dwFlags;
    req->si_show_window = si->wShowWindow;
    req->creation_flags = creation_flags;

    ptr = (WCHAR *)((ULONG_PTR)req + sizeof(PROCESS_RUN_SANDBOXED_REQ));

    req->cmd_ofs = (ULONG)((ULONG_PTR)ptr - (ULONG_PTR)req);
    req->cmd_len = cmd_len;
    if (cmd_len) {
        wmemcpy(ptr, cmd, cmd_len);
        ptr += cmd_len;
    }
    *ptr = L'\0';
    ++ptr;

    req->dir_ofs = (ULONG)((ULONG_PTR)ptr - (ULONG_PTR)req);
    req->dir_len = dir_len;
    if (dir_len) {
        wmemcpy(ptr, dir, dir_len);
        ptr += dir_len;
    }
    *ptr = L'\0';
    ++ptr;

    req->env_ofs = (ULONG)((ULONG_PTR)ptr - (ULONG_PTR)req);
    req->env_len = env_len;
    if (env_len) {
        wmemcpy(ptr, env, env_len);
        ptr += env_len;
    }
    *ptr = L'\0';
    ++ptr;

    //
    // execute request
    //

    rpl = (PROCESS_RUN_SANDBOXED_RPL *)SbieDll_CallServer(&req->h);

    Dll_Free(req);

    if (rpl) {

        err = rpl->h.status;
        if (err == 0) {

            pi->hProcess = (HANDLE)(ULONG_PTR)rpl->hProcess;
            pi->hThread = (HANDLE)(ULONG_PTR)rpl->hThread;
            pi->dwProcessId = rpl->dwProcessId;
            pi->dwThreadId = rpl->dwThreadId;

            ok = TRUE;
        } else
            ok = FALSE;

        Dll_Free(rpl);

    } else {

        err = ERROR_SERVER_DISABLED;
        ok = FALSE;
    }

    if (env)
        Dll_Free(env);

    SetLastError(err);
    return ok;
}

