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
// Pipe Server
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "PipeServer.h"
#include "misc.h"
#include "msgids.h"
#include "core/dll/sbiedll.h"
#include "common/defines.h"
#include "common/my_version.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define MAX_REQUEST_LENGTH      (2048 * 1024)
#define MSG_DATA_LEN            (MAX_PORTMSG_LENGTH - sizeof(PORT_MESSAGE))


//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------


typedef struct tagTARGET
{
    LIST_ELEM list_elem;
    ULONG serverId;
    void *context;
    PipeServer::Handler handler;
} TARGET;


typedef struct tagCLIENT_PROCESS
{
    LIST_ELEM list_elem;
    HANDLE idProcess;
    LARGE_INTEGER CreateTime;
    LIST threads;
} CLIENT_PROCESS;


typedef struct tagCLIENT_THREAD
{
    LIST_ELEM list_elem;
    HANDLE idThread;
    BOOLEAN replying;
    volatile BOOLEAN in_use;
    UCHAR sequence;
    HANDLE hPort;
    MSG_HEADER *buf_hdr;
    UCHAR *buf_ptr;
} CLIENT_THREAD;


typedef struct tagCLIENT_TLS_DATA
{
    HANDLE PortHandle;
    PORT_MESSAGE *PortMessage;
} CLIENT_TLS_DATA;


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


PipeServer *PipeServer::m_instance = NULL;


//---------------------------------------------------------------------------
// GetPipeServer
//---------------------------------------------------------------------------


PipeServer *PipeServer::GetPipeServer()
{
    if (! m_instance) {

        ULONG TlsIndex = TlsAlloc();
        if (TlsIndex == TLS_OUT_OF_INDEXES)
            return NULL;

        m_instance = new PipeServer();

        m_instance->m_TlsIndex = TlsIndex;

        m_instance->m_pool = Pool_Create();
        if (! m_instance->m_pool) {
            delete m_instance;
            m_instance = NULL;
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return NULL;
        }
    }

    return m_instance;
}


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


PipeServer::PipeServer()
{
    InitializeCriticalSectionAndSpinCount(&m_lock, 1000);
    List_Init(&m_targets);
    List_Init(&m_clients);

    m_hServerPort = NULL;

    ULONG len_threads = (NUMBER_OF_THREADS) * sizeof(HANDLE);
    m_Threads = (HANDLE *)HeapAlloc(GetProcessHeap(), 0, len_threads);
    if (m_Threads)
        memzero(m_Threads, len_threads);
    else
        LogEvent(MSG_9234, 0x9251, GetLastError());
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


PipeServer::~PipeServer()
{
    ULONG i;

    HANDLE PortHandle = InterlockedExchangePointer(&m_hServerPort, NULL);

    if (PortHandle) {

        //
        // reset m_hServerPort, then send enough messages to cause
        // the worker threads to wake up and shut down
        //

        UCHAR space[MAX_PORTMSG_LENGTH];

        for (i = 0; i < NUMBER_OF_THREADS; ++i) {
            PORT_MESSAGE *msg = (PORT_MESSAGE *)space;
            memzero(msg, MAX_PORTMSG_LENGTH);
            msg->u1.s1.TotalLength = (USHORT)sizeof(PORT_MESSAGE);
            NtRequestPort(PortHandle, msg);
        }
    }

    if (m_Threads) {

        if (WAIT_TIMEOUT == WaitForMultipleObjects(
                                NUMBER_OF_THREADS, m_Threads, TRUE, 5000)) {

            for (i = 0; i < NUMBER_OF_THREADS; ++i)
                TerminateThread(m_Threads[i], 0);
            WaitForMultipleObjects(NUMBER_OF_THREADS, m_Threads, TRUE, 5000);
        }
    }

    if (PortHandle)
        NtClose(PortHandle);

    if (m_pool)
        Pool_Delete(m_pool);
}


//---------------------------------------------------------------------------
// Register
//---------------------------------------------------------------------------


void PipeServer::Register(ULONG serverId, void *context, Handler handler)
{
    TARGET *target = (TARGET *)Pool_Alloc(m_pool, sizeof(TARGET));
    if (target) {
        target->serverId = serverId;
        target->context = context;
        target->handler = handler;
        List_Insert_After(&m_targets, NULL, target);
    }
}


//---------------------------------------------------------------------------
// Start
//---------------------------------------------------------------------------


bool PipeServer::Start()
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING PortName;
    ULONG i;
    ULONG idThread;

    //
    // the server port should have a NULL DACL so any process can connect
    //

    ULONG sd_space[16];
    memzero(&sd_space, sizeof(sd_space));
    PSECURITY_DESCRIPTOR sd = (PSECURITY_DESCRIPTOR)&sd_space;
    InitializeSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(sd, TRUE, NULL, FALSE);

    //
    // create server port
    //

    RtlInitUnicodeString(&PortName, SbieDll_PortName());

    InitializeObjectAttributes(
        &objattrs, &PortName, OBJ_CASE_INSENSITIVE, NULL, sd);

    status = NtCreatePort(
        (HANDLE *)&m_hServerPort, &objattrs, 0, MAX_PORTMSG_LENGTH, NULL);

    if (! NT_SUCCESS(status)) {
        LogEvent(MSG_9234, 0x9252, status);
        return false;
    }

    //
    // make sure threads on other CPUs will see the port
    //

    InterlockedExchangePointer(&m_hServerPort, m_hServerPort);

    //
    // create server threads
    //

    for (i = 0; i < NUMBER_OF_THREADS; ++i) {

        m_Threads[i] = CreateThread(
            NULL, 0, (LPTHREAD_START_ROUTINE)ThreadStub, this, 0, &idThread);
        if (! m_Threads[i]) {
            LogEvent(MSG_9234, 0x9253, GetLastError());
            return false;
        }
    }

    return true;
}


//---------------------------------------------------------------------------
// ThreadStub
//---------------------------------------------------------------------------


void PipeServer::ThreadStub(void *parm)
{
    ((PipeServer *)parm)->Thread();
}


//---------------------------------------------------------------------------
// Thread
//---------------------------------------------------------------------------


void PipeServer::Thread()
{
    NTSTATUS status;
    UCHAR space[MAX_PORTMSG_LENGTH], spaceReply[MAX_PORTMSG_LENGTH];
    PORT_MESSAGE *msg = (PORT_MESSAGE *)space;
    HANDLE hReplyPort;
    PORT_MESSAGE *ReplyMsg;

    //
    // initially we have no reply to send.  we will also revert to
    // this no-reply state after each reply has been sent
    //

    hReplyPort = m_hServerPort;
    ReplyMsg = NULL;

    while (1) {

        //
        // send the outgoing reply in ReplyMsg, if any, to the port in
        // hReplyPort.  then wait for an incoming message.  note that even
        // if hReplyPort indicates a client port to send the message, the
        // server-side NtReplyWaitReceivePort will listen on the associated
        // server port, after the message has been sent.
        //

        if (ReplyMsg) {

            memcpy(spaceReply, ReplyMsg, ReplyMsg->u1.s1.TotalLength);
            ReplyMsg = (PORT_MESSAGE *)spaceReply;
        }

        status = NtReplyWaitReceivePort(hReplyPort, NULL, ReplyMsg, msg);

        if (! m_hServerPort)    // service is shutting down
            break;

        if (ReplyMsg) {

            hReplyPort = m_hServerPort;
            ReplyMsg = NULL;

            if (! NT_SUCCESS(status))
                continue;       // ignore errors on client port

        } else if (! NT_SUCCESS(status)) {

            if (status == STATUS_UNSUCCESSFUL) {
                // can be considered a warning rather than an error
                continue;
            }
            break;              // abort on errors on server port
        }

        if (msg->u2.s2.Type == LPC_CONNECTION_REQUEST) {

            PortConnect(msg);

        } else if (msg->u2.s2.Type == LPC_REQUEST) {

            CLIENT_THREAD *client = (CLIENT_THREAD *)PortFindClient(msg);
            if (! client)
                continue;

            if (! client->replying)
                PortRequest(client->hPort, msg, client);

            msg->u2.ZeroInit = 0;

            if (client->replying)
                PortReply(msg, client);
            else {
                msg->u1.s1.DataLength = (USHORT) 0;
                msg->u1.s1.TotalLength = sizeof(PORT_MESSAGE);
            }

            hReplyPort = client->hPort;
            ReplyMsg = msg;

            client->in_use = FALSE;

        } else if (msg->u2.s2.Type == LPC_PORT_CLOSED ||
                   msg->u2.s2.Type == LPC_CLIENT_DIED) {

            PortDisconnect(msg);
        }
    }
}


//---------------------------------------------------------------------------
// PortConnect
//---------------------------------------------------------------------------


void PipeServer::PortConnect(PORT_MESSAGE *msg)
{
    NTSTATUS status;
    CLIENT_PROCESS *clientProcess;
    CLIENT_THREAD *clientThread;

    //
    // find a previous connection to that same client, or create a new one
    //

    EnterCriticalSection(&m_lock);

    clientProcess = (CLIENT_PROCESS *)List_Head(&m_clients);
    clientThread = NULL;

    while (clientProcess) {

        if (clientProcess->idProcess == msg->ClientId.UniqueProcess) {

            clientThread =
                (CLIENT_THREAD *)List_Head(&clientProcess->threads);
            while (clientThread) {

                if (clientThread->idThread == msg->ClientId.UniqueThread)
                    break;
                clientThread = (CLIENT_THREAD *)List_Next(clientThread);
            }

            break;
        }

        clientProcess = (CLIENT_PROCESS *)List_Next(clientProcess);
    }

    //
    // create new process and thread structures where needed
    //

    if (! clientProcess) {

        clientProcess =
            (CLIENT_PROCESS *)Pool_Alloc(m_pool, sizeof(CLIENT_PROCESS));
        if (clientProcess) {

            clientProcess->idProcess = msg->ClientId.UniqueProcess;
            List_Init(&clientProcess->threads);
            List_Insert_After(&m_clients, NULL, clientProcess);

            //
            // prepare for the case where a disconnect message only
            // specifies process creation time:  record the process
            // creation time, so it can be used later
            //

            clientProcess->CreateTime.HighPart = 0;
            clientProcess->CreateTime.LowPart = 0;
            HANDLE hProcess = OpenProcess(
                PROCESS_QUERY_INFORMATION, FALSE,
                (ULONG)(ULONG_PTR)msg->ClientId.UniqueProcess);
            if (hProcess) {
                FILETIME time, time1, time2, time3;
                BOOL ok = GetProcessTimes(
                    hProcess, &time, &time1, &time2, &time3);
                if (ok) {
                    clientProcess->CreateTime.HighPart = time.dwHighDateTime;
                    clientProcess->CreateTime.LowPart  = time.dwLowDateTime;
                }
                CloseHandle(hProcess);
            }

            /*WCHAR msg[128];
            wsprintf(msg, L"PortConnect - Connected pid %d with timestamp %08X-%08X\n",
                clientProcess->idProcess, clientProcess->CreateTime.HighPart, clientProcess->CreateTime.LowPart);
            OutputDebugString(msg);*/
        }
    }

    if (clientProcess && (! clientThread)) {

        clientThread =
            (CLIENT_THREAD *)Pool_Alloc(m_pool, sizeof(CLIENT_THREAD));
        if (clientThread) {

            memset(clientThread, 0, sizeof(CLIENT_THREAD));
            clientThread->idThread = msg->ClientId.UniqueThread;
            List_Insert_After(&clientProcess->threads, NULL, clientThread);
        }
    }

    //
    // if we couldn't create a new connection (not enough memory)
    // or if a previous connection was found,
    // then reject the new connection
    //

    if ((! clientThread) || clientThread->hPort) {

        HANDLE hPort;
        NtAcceptConnectPort(&hPort, NULL, msg, FALSE, NULL, NULL);

        LeaveCriticalSection(&m_lock);

        return;
    }

    //
    // if a new client structure was created, accept the connection
    //

    status = NtAcceptConnectPort(
        &clientThread->hPort, NULL, msg, TRUE, NULL, NULL);

    if (NT_SUCCESS(status))
        status = NtCompleteConnectPort(clientThread->hPort);

    LeaveCriticalSection(&m_lock);
}


//---------------------------------------------------------------------------
// PortDisconnect
//---------------------------------------------------------------------------


void PipeServer::PortDisconnect(PORT_MESSAGE *msg)
{
    CLIENT_PROCESS *clientProcess;
    CLIENT_THREAD *clientThread;

    //
    // on Windows Vista, a LPC_PORT_CLOSED messages arrives with zero CID,
    // but includes a process timestamp
    //

    if ((! msg->ClientId.UniqueProcess) || (! msg->ClientId.UniqueThread)) {

        if (msg->u1.s1.DataLength == 8) {

            PortDisconnectByCreateTime((LARGE_INTEGER *)msg->Data);
        }

        return;
    }

    //
    // find a previous connection to that same client
    //

    EnterCriticalSection(&m_lock);

    clientProcess = (CLIENT_PROCESS *)List_Head(&m_clients);
    clientThread = NULL;

    while (clientProcess) {

        if (clientProcess->idProcess == msg->ClientId.UniqueProcess) {

            clientThread =
                (CLIENT_THREAD *)List_Head(&clientProcess->threads);
            while (clientThread) {

                if (clientThread->idThread == msg->ClientId.UniqueThread) {

                    while (clientThread->in_use)
                        Sleep(3);

                    List_Remove(&clientProcess->threads, clientThread);
                    NtClose(clientThread->hPort);
                    if (clientThread->buf_hdr)
                        FreeMsg(clientThread->buf_hdr);
                    Pool_Free(clientThread, sizeof(CLIENT_THREAD));
                    break;
                }

                clientThread = (CLIENT_THREAD *)List_Next(clientThread);
            }

            if (! List_Head(&clientProcess->threads)) {

                NotifyTargets(clientProcess->idProcess);

                List_Remove(&m_clients, clientProcess);
                Pool_Free(clientProcess, sizeof(CLIENT_PROCESS));
            }

            break;
        }

        clientProcess = (CLIENT_PROCESS *)List_Next(clientProcess);
    }

    LeaveCriticalSection(&m_lock);
}


//---------------------------------------------------------------------------
// PortDisconnectByCreateTime
//---------------------------------------------------------------------------


void PipeServer::PortDisconnectByCreateTime(LARGE_INTEGER *CreateTime)
{
    typedef HANDLE (*P_GetProcessIdOfThread)(HANDLE Thread);
    static P_GetProcessIdOfThread pGetProcessIdOfThread = NULL;
    static BOOLEAN init_done = FALSE;

    //
    // the limited LPC_PORT_CLOSED only occurs on Windows Vista,
    // so we can afford to quit if the Vista-only GetProcessIdOfThread
    // function is not available
    //

    if (! init_done) {

        pGetProcessIdOfThread = (P_GetProcessIdOfThread)
                    GetProcAddress(_Kernel32, "GetProcessIdOfThread");

        init_done = TRUE;
    }

    if (! pGetProcessIdOfThread)
        return;

    //
    // find the process id by its creation timestamp
    //

    /*WCHAR txt[128];
    wsprintf(txt, L"Message has no CID but has timestamp %08X-%08X", CreateTime->HighPart, CreateTime->LowPart);
    OutputDebugString(txt);*/

    EnterCriticalSection(&m_lock);

    CLIENT_PROCESS *clientProcess = (CLIENT_PROCESS *)List_Head(&m_clients);

    while (clientProcess) {

        if (clientProcess->CreateTime.HighPart == CreateTime->HighPart &&
            clientProcess->CreateTime.LowPart  == CreateTime->LowPart) {

            CLIENT_THREAD *clientThread =
                (CLIENT_THREAD *)List_Head(&clientProcess->threads);
            while (clientThread) {

                //
                // for each thread in the process, assume it is stale,
                // unless we can open it, and it still has the same
                // process id
                //

                BOOLEAN DeleteThread = TRUE;

                HANDLE hThread = OpenThread(
                    THREAD_QUERY_INFORMATION, FALSE,
                    (ULONG)(ULONG_PTR)clientThread->idThread);
                if (hThread) {
                    HANDLE ThreadProcessId = pGetProcessIdOfThread(hThread);
                    if (ThreadProcessId == clientProcess->idProcess)
                        DeleteThread = FALSE;
                    CloseHandle(hThread);
                }

                if (DeleteThread) {

                    while (clientThread->in_use)
                        Sleep(3);

                    List_Remove(&clientProcess->threads, clientThread);
                    NtClose(clientThread->hPort);
                    if (clientThread->buf_hdr)
                        FreeMsg(clientThread->buf_hdr);
                    Pool_Free(clientThread, sizeof(CLIENT_THREAD));
                    break;
                }

                clientThread = (CLIENT_THREAD *)List_Next(clientThread);
            }

            if (! List_Head(&clientProcess->threads)) {

                NotifyTargets(clientProcess->idProcess);

                List_Remove(&m_clients, clientProcess);
                Pool_Free(clientProcess, sizeof(CLIENT_PROCESS));
            }

            break;
        }

        clientProcess = (CLIENT_PROCESS *)List_Next(clientProcess);
    }

    LeaveCriticalSection(&m_lock);
}


//---------------------------------------------------------------------------
// PortRequest
//---------------------------------------------------------------------------


void PipeServer::PortRequest(
    HANDLE PortHandle, PORT_MESSAGE *msg, void *voidClient)
{
    CLIENT_THREAD *client = (CLIENT_THREAD *)voidClient;
    ULONG buf_len;
    void *buf_ptr = NULL;

    if (! client->buf_hdr) {

        ULONG *msg_Data = (ULONG *)msg->Data;
        ULONG msgid = msg_Data[1];

        client->sequence = ((UCHAR *)msg_Data)[3];
        ((UCHAR *)msg_Data)[3] = 0;

        buf_len = msg_Data[0];

        if (msgid && buf_len &&
                buf_len < MAX_REQUEST_LENGTH &&
                buf_len >= sizeof(MSG_HEADER) &&
                buf_len >= msg->u1.s1.DataLength) {

            client->buf_hdr = AllocMsg(buf_len);
            client->buf_ptr = (UCHAR *)client->buf_hdr;
        }

        if (! client->buf_hdr) {
            client->sequence = 0;
            goto finish;
        }

        buf_len = 0;

    } else {

        buf_len = (ULONG)(client->buf_ptr - (UCHAR *)client->buf_hdr);

        if (buf_len + msg->u1.s1.DataLength > client->buf_hdr->length)
            goto finish;
    }

    memcpy(client->buf_ptr, msg->Data, msg->u1.s1.DataLength);
    client->buf_ptr += msg->u1.s1.DataLength;
    buf_len += msg->u1.s1.DataLength;

    if (buf_len < client->buf_hdr->length)
        return;

    buf_ptr = CallTarget(client->buf_hdr, PortHandle, msg);

finish:

    if (client->buf_hdr)
        FreeMsg(client->buf_hdr);

    client->buf_hdr = (MSG_HEADER *)buf_ptr;
    client->buf_ptr = (UCHAR *)buf_ptr;
    client->replying = TRUE;
}


//---------------------------------------------------------------------------
// PortFindClient
//---------------------------------------------------------------------------


void *PipeServer::PortFindClient(PORT_MESSAGE *msg)
{
    CLIENT_PROCESS *clientProcess;
    CLIENT_THREAD *clientThread;

    EnterCriticalSection(&m_lock);

    clientProcess = (CLIENT_PROCESS *)List_Head(&m_clients);
    clientThread = NULL;

    while (clientProcess) {

        if (clientProcess->idProcess == msg->ClientId.UniqueProcess) {

            clientThread =
                (CLIENT_THREAD *)List_Head(&clientProcess->threads);
            while (clientThread) {

                if (clientThread->idThread == msg->ClientId.UniqueThread)
                    break;
                clientThread = (CLIENT_THREAD *)List_Next(clientThread);
            }

            break;
        }

        clientProcess = (CLIENT_PROCESS *)List_Next(clientProcess);
    }

    if (clientThread)
        clientThread->in_use = TRUE;

    LeaveCriticalSection(&m_lock);

    return clientThread;
}


//---------------------------------------------------------------------------
// CallTarget
//---------------------------------------------------------------------------


MSG_HEADER *PipeServer::CallTarget(
    MSG_HEADER *msg, HANDLE PortHandle, PORT_MESSAGE *PortMessage)
{
    //
    // find target server.
    // don't let a caller specify a NOTIFICATION message id
    //

    TARGET *target = NULL;

    ULONG msgid = msg->msgid;
    if ((msgid & 0xFF) != 0xFF) {

        ULONG serverId = msgid & 0xFFFFFF00;
        target = (TARGET *)List_Head(&m_targets);
        while (target) {
            if (target->serverId == serverId)
                break;
            target = (TARGET *)List_Next(target);
        }
    }

    if (! target)
        return AllocShortMsg(STATUS_INVALID_SYSTEM_SERVICE);

    //
    // invoke target server
    //

    CLIENT_TLS_DATA TlsData;
    TlsData.PortHandle = PortHandle;
    TlsData.PortMessage = PortMessage;
    TlsSetValue(m_TlsIndex, &TlsData);

    MSG_HEADER *msgOut = NULL;

    __try {

        msgOut = (*target->handler)(target->context, msg);

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ULONG exc = GetExceptionCode();
        msgOut = AllocShortMsg(exc);
    }

    RevertToSelf();
    TlsSetValue(m_TlsIndex, NULL);

    return msgOut;
}


//---------------------------------------------------------------------------
// PortReply
//---------------------------------------------------------------------------


void PipeServer::PortReply(PORT_MESSAGE *msg, void *voidClient)
{
    CLIENT_THREAD *client = (CLIENT_THREAD *)voidClient;
    ULONG buf_len;

    if (! client->buf_ptr) {
        msg->u1.s1.DataLength = (USHORT) 0;
        msg->u1.s1.TotalLength = sizeof(PORT_MESSAGE);
        client->replying = FALSE;
        return;
    }

    buf_len = client->buf_hdr->length
            - (ULONG)(client->buf_ptr - (UCHAR *)client->buf_hdr);
    if (buf_len > MSG_DATA_LEN)
        buf_len = MSG_DATA_LEN;

    msg->u1.s1.DataLength = (USHORT) buf_len;
    msg->u1.s1.TotalLength = (USHORT)(sizeof(PORT_MESSAGE) + buf_len);
    memcpy(msg->Data, client->buf_ptr, buf_len);

    if (client->buf_ptr == (UCHAR *)client->buf_hdr)
        ((UCHAR *)msg->Data)[3] = client->sequence;

    client->buf_ptr += buf_len;

    buf_len = (ULONG)(client->buf_ptr - (UCHAR *)client->buf_hdr);
    if (buf_len >= client->buf_hdr->length) {
        FreeMsg(client->buf_hdr);
        client->buf_hdr = NULL;
        client->buf_ptr = NULL;
        client->replying = FALSE;
    }
}


//---------------------------------------------------------------------------
// NotifyTargets
//---------------------------------------------------------------------------


void PipeServer::NotifyTargets(HANDLE idProcess)
{
    PORT_MESSAGE PortMsg;
    PortMsg.ClientId.UniqueProcess = idProcess;
    PortMsg.ClientId.UniqueThread = 0;

    CLIENT_TLS_DATA TlsData;
    TlsData.PortHandle = NULL;
    TlsData.PortMessage = &PortMsg;
    TlsSetValue(m_TlsIndex, &TlsData);

    MSG_HEADER msg;
    msg.length = sizeof(MSG_HEADER);

    TARGET *target = (TARGET *)List_Head(&m_targets);
    while (target) {

        msg.msgid = target->serverId | 0xFF;

        (*target->handler)(target->context, &msg);

        target = (TARGET *)List_Next(target);
    }
}


//---------------------------------------------------------------------------
// AllocShortMsg
//---------------------------------------------------------------------------


MSG_HEADER *PipeServer::AllocShortMsg(ULONG status)
{
    MSG_HEADER *msg = AllocMsg(sizeof(MSG_HEADER));
    if (msg)
        msg->status = status;
    return msg;
}


//---------------------------------------------------------------------------
// AllocMsg
//---------------------------------------------------------------------------


MSG_HEADER *PipeServer::AllocMsg(ULONG length)
{
    UCHAR *buf = (UCHAR *)Pool_Alloc(m_pool, length + sizeof(ULONG) * 2);
    if (buf) {
        ((MSG_HEADER *)buf)->length = length;
        ((MSG_HEADER *)buf)->status = 0;
        *(ULONG *)(buf + length) = 0;
        *(ULONG *)(buf + length + sizeof(ULONG)) = tzuk;
    }
    return (MSG_HEADER *)buf;
}


//---------------------------------------------------------------------------
// FreeMsg
//---------------------------------------------------------------------------


void PipeServer::FreeMsg(MSG_HEADER *msg)
{
    UCHAR *buf = (UCHAR *)msg;
    if (    *(ULONG *)(buf + msg->length) != 0 ||
            *(ULONG *)(buf + msg->length + sizeof(ULONG)) != tzuk) {
        SbieApi_Log(2316, NULL);
        __debugbreak();
    }
    Pool_Free(msg, msg->length + sizeof(ULONG) * 2);
}


//---------------------------------------------------------------------------
// GetCallerProcessId
//---------------------------------------------------------------------------


ULONG PipeServer::GetCallerProcessId()
{
    CLIENT_TLS_DATA *TlsData =
                (CLIENT_TLS_DATA *)TlsGetValue(m_instance->m_TlsIndex);
    return (ULONG)(ULONG_PTR)TlsData->PortMessage->ClientId.UniqueProcess;
}


//---------------------------------------------------------------------------
// GetCallerThreadId
//---------------------------------------------------------------------------


ULONG PipeServer::GetCallerThreadId()
{
    CLIENT_TLS_DATA *TlsData =
                (CLIENT_TLS_DATA *)TlsGetValue(m_instance->m_TlsIndex);
    return (ULONG)(ULONG_PTR)TlsData->PortMessage->ClientId.UniqueThread;
}


//---------------------------------------------------------------------------
// GetCallerSessionId
//---------------------------------------------------------------------------


ULONG PipeServer::GetCallerSessionId()
{
    ULONG SessionId;
    if (! ProcessIdToSessionId(GetCallerProcessId(), &SessionId))
        SessionId = 0;
    return SessionId;
}


//---------------------------------------------------------------------------
// ImpersonateCaller
//---------------------------------------------------------------------------


ULONG PipeServer::ImpersonateCaller(MSG_HEADER **pmsg)
{
    ULONG status;
    CLIENT_TLS_DATA *TlsData =
                (CLIENT_TLS_DATA *)TlsGetValue(m_instance->m_TlsIndex);
    if (TlsData->PortHandle) {

        status = NtImpersonateClientOfPort(
                                TlsData->PortHandle, TlsData->PortMessage);

        if ((! NT_SUCCESS(status)) && pmsg)
            *pmsg = m_instance->AllocShortMsg(status);

    } else
        status = STATUS_SUCCESS;

    return status;
}


//---------------------------------------------------------------------------
// Call
//---------------------------------------------------------------------------


MSG_HEADER *PipeServer::Call(MSG_HEADER *msg)
{
    PORT_MESSAGE PortMsg;
    PortMsg.ClientId.UniqueProcess =
                            (HANDLE)(ULONG_PTR)GetCurrentProcessId();
    PortMsg.ClientId.UniqueThread =
                            (HANDLE)(ULONG_PTR)GetCurrentThreadId();

    void *OldTlsData = TlsGetValue(m_TlsIndex);
    MSG_HEADER *msg_out = CallTarget(msg, NULL, &PortMsg);
    TlsSetValue(m_TlsIndex, OldTlsData);

    return msg_out;
}
