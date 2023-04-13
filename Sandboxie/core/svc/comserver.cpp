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
// COM Proxy Server -- using PipeServer
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "comserver.h"
#include "comwire.h"
#include "common/defines.h"
#include "common/my_version.h"

#include <objbase.h>
#include <userenv.h>
#include "core/dll/sbiedll.h"
#include "misc.h"

#pragma auto_inline(off)


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#undef  DEBUG_COMSERVER
//#define DEBUG_COMSERVER


#ifdef  DEBUG_COMSERVER
#define SLAVE_TIMEOUT_SECONDS   6666
#else
#define SLAVE_TIMEOUT_SECONDS   30
#endif


#define FLAG_CLASS_FACTORY      1
#define FLAG_COPY_PROXY         2
#define FLAG_INPROC_SERVER      4
#define FLAG_WMI                0x100


#define ERROR_WAIT_1                     731L
#define ERROR_WAIT_2                     732L
#define ERROR_WAIT_3                     733L


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _COM_SLAVE_MAP {

    union {
        ULONG msgid;
        ULONG exc;
    };
    union {
        ULONG idProcess;
        HRESULT hr;
    };
    ULONG objidx;
    ULONG DataRepresentation;
    ULONG ProcNum;
    ULONG BufferLength;
    UCHAR Buffer[1];

} COM_SLAVE_MAP;


#define COM_SLAVE_MAP_SIZE  (PAGE_SIZE * 512)

#define MAX_MAP_BUFFER_LENGTH \
                (COM_SLAVE_MAP_SIZE - FIELD_OFFSET(COM_SLAVE_MAP, Buffer))


typedef struct _COM_SLAVE {

    LIST_ELEM list_elem;

    WCHAR SidString[96];
    WCHAR BoxName[BOXNAME_COUNT];
    ULONG SessionId;
    BOOLEAN IsWow64;

    BOOLEAN lock_initialized;
    BOOLEAN not_responding;
    BOOLEAN non_com_request;
    CRITICAL_SECTION lock;
    ULONG lock_ticks;

    HANDLE hMutex;
    HANDLE hEvent1;
    HANDLE hEvent2;
    HANDLE hMap;
    COM_SLAVE_MAP *pMap;
    HANDLE hProcess;
    ULONG_PTR idProcess;

} COM_SLAVE;


typedef struct _COM_OBJECT {

    LIST_ELEM list_elem;
    ULONG idProcess;
    ULONG objidx;
    GUID iid;
    IUnknown *pUnknown;
    ULONG_PTR pUnknownVtbl;
    IRpcStubBuffer *pStub;
    IRpcChannelBuffer *pChannel;
    ULONG Flags;

} COM_OBJECT;


//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


volatile LONG ComServer::m_ObjIdx = 0x00000001;

HANDLE ComServer::m_heap = NULL;

EXTERN_C const IID GUID_NULL;

static const GUID IID_IWbemServices = {
    0x9556DC99, 0x828C, 0x11CF,
                    { 0xA3, 0x7E, 0x00, 0xAA, 0x00, 0x32, 0x40, 0xC7 } };
static const GUID IID_IWbemClassObject = {
    0xDC12A681, 0x737F, 0x11CF,
                    { 0x88, 0x4D, 0x00, 0xAA, 0x00, 0x4B, 0x2E, 0x24 } };


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


ComServer::ComServer(PipeServer *pipeServer)
{
    m_SlaveReleasedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    InitializeCriticalSection(&m_SlavesLock);
    List_Init(&m_SlavesList);

    pipeServer->Register(MSGID_COM, this, Handler);
}

ComServer::~ComServer()
{
	// cleanup CS
	DeleteCriticalSection(&m_SlavesLock);
}


//---------------------------------------------------------------------------
// Handler
//---------------------------------------------------------------------------


MSG_HEADER *ComServer::Handler(void *_this, MSG_HEADER *msg)
{
    ComServer *pThis = (ComServer *)_this;

    HANDLE idProcess = (HANDLE)(ULONG_PTR)PipeServer::GetCallerProcessId();

    if (msg->msgid == MSGID_COM_NOTIFICATION) {
        pThis->NotifyAllSlaves(idProcess);
        return NULL;
    }

    if (PipeServer::ImpersonateCaller(&msg) != 0)
        return msg;

    COM_SLAVE *slave = (COM_SLAVE *)pThis->LockSlave(idProcess, msg->msgid);
    if (! slave) {
        SetEvent(pThis->m_SlaveReleasedEvent);
        return SHORT_REPLY(RPC_S_SERVER_UNAVAILABLE);
    }

    void *rpl = NULL;
    BOOLEAN deleted = FALSE;

    if (msg->msgid == MSGID_COM_GET_CLASS_OBJECT)
        rpl = pThis->GetClassObjectHandler(msg, slave, &deleted);

    if (msg->msgid == MSGID_COM_CREATE_INSTANCE)
        rpl = pThis->CreateInstanceHandler(msg, slave, &deleted);

    if (msg->msgid == MSGID_COM_QUERY_INTERFACE)
        rpl = pThis->QueryInterfaceHandler(msg, slave, &deleted);

    if (msg->msgid == MSGID_COM_ADD_REF_RELEASE)
        rpl = pThis->AddRefReleaseHandler(msg, slave, &deleted);

    if (msg->msgid == MSGID_COM_INVOKE_METHOD)
        rpl = pThis->InvokeMethodHandler(msg, slave, &deleted);

    if (msg->msgid == MSGID_COM_UNMARSHAL_INTERFACE)
        rpl = pThis->UnmarshalInterfaceHandler(msg, slave, &deleted);

    if (msg->msgid == MSGID_COM_MARSHAL_INTERFACE)
        rpl = pThis->MarshalInterfaceHandler(msg, slave, &deleted);

    if (msg->msgid == MSGID_COM_QUERY_BLANKET)
        rpl = pThis->QueryBlanketHandler(msg, slave, &deleted);

    if (msg->msgid == MSGID_COM_SET_BLANKET)
        rpl = pThis->SetBlanketHandler(msg, slave, &deleted);

    if (msg->msgid == MSGID_COM_COPY_PROXY)
        rpl = pThis->CopyProxyHandler(msg, slave, &deleted);

    if ((msg->msgid & 0xFF) > 0x80)
        rpl = pThis->NonComHandler(msg, slave, &deleted);

    if (! deleted)
        LeaveCriticalSection(&slave->lock);
    SetEvent(pThis->m_SlaveReleasedEvent);

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// GetClassObjectHandler
//---------------------------------------------------------------------------


MSG_HEADER *ComServer::GetClassObjectHandler(
    MSG_HEADER *msg, void *_slave, BOOLEAN *deleted)
{
    COM_GET_CLASS_OBJECT_REQ *req = (COM_GET_CLASS_OBJECT_REQ *)msg;
    if (req->h.length < sizeof(COM_GET_CLASS_OBJECT_REQ))
        return SHORT_REPLY(E_INVALIDARG);

    COM_SLAVE *slave = (COM_SLAVE *)_slave;
    COM_SLAVE_MAP *pMap = slave->pMap;

    pMap->BufferLength = sizeof(GUID) * 2;
    GUID *guids = (GUID *)pMap->Buffer;
    memcpy(&guids[0], &req->clsid, sizeof(GUID));
    memcpy(&guids[1], &req->iid, sizeof(GUID));

    ULONG exc;
    if (! SbieDll_IsOpenClsid(
                        guids[0], CLSCTX_LOCAL_SERVER, slave->BoxName))
        exc = RPC_E_ACCESS_DENIED;
    else {

        exc = 0;
        pMap->ProcNum = 0;
        if (req->elevate) {
            if (CheckDropRights(slave->BoxName, NULL))
                exc = ERROR_ELEVATION_REQUIRED;
            else
                pMap->ProcNum = 1;
        }

        if (exc == 0)
            exc = CallSlave(slave, 0x31, deleted);
    }

    if (exc)
        return SHORT_REPLY(exc);

    COM_GET_CLASS_OBJECT_RPL *rpl = (COM_GET_CLASS_OBJECT_RPL *)
                                LONG_REPLY(sizeof(COM_GET_CLASS_OBJECT_RPL));
    if (rpl) {
        rpl->hr = pMap->hr;
        rpl->objidx = pMap->objidx;
    }

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// CreateInstanceHandler
//---------------------------------------------------------------------------


MSG_HEADER *ComServer::CreateInstanceHandler(
    MSG_HEADER *msg, void *_slave, BOOLEAN *deleted)
{
    COM_CREATE_INSTANCE_REQ *req = (COM_CREATE_INSTANCE_REQ *)msg;
    if (req->h.length < sizeof(COM_CREATE_INSTANCE_REQ))
        return SHORT_REPLY(E_INVALIDARG);

    COM_SLAVE *slave = (COM_SLAVE *)_slave;
    COM_SLAVE_MAP *pMap = slave->pMap;

    pMap->objidx = req->objidx;
    pMap->BufferLength = sizeof(GUID);
    memcpy(pMap->Buffer, &req->iid, sizeof(GUID));

    ULONG exc = CallSlave(slave, 0x32, deleted);
    if (exc)
        return SHORT_REPLY(exc);

    COM_CREATE_INSTANCE_RPL *rpl = (COM_CREATE_INSTANCE_RPL *)
                                LONG_REPLY(sizeof(COM_CREATE_INSTANCE_RPL));
    if (rpl) {
        rpl->hr = pMap->hr;
        rpl->objidx = pMap->objidx;
    }

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// QueryInterfaceHandler
//---------------------------------------------------------------------------


MSG_HEADER *ComServer::QueryInterfaceHandler(
    MSG_HEADER *msg, void *_slave, BOOLEAN *deleted)
{
    COM_QUERY_INTERFACE_REQ *req = (COM_QUERY_INTERFACE_REQ *)msg;
    if (req->h.length < sizeof(COM_QUERY_INTERFACE_REQ))
        return SHORT_REPLY(E_INVALIDARG);

    COM_SLAVE *slave = (COM_SLAVE *)_slave;
    COM_SLAVE_MAP *pMap = slave->pMap;

    pMap->objidx = req->objidx;
    pMap->BufferLength = sizeof(GUID);
    memcpy(pMap->Buffer, &req->iid, sizeof(GUID));

    ULONG exc = CallSlave(slave, 0x33, deleted);
    if (exc)
        return SHORT_REPLY(exc);

    COM_QUERY_INTERFACE_RPL *rpl = (COM_QUERY_INTERFACE_RPL *)
                                LONG_REPLY(sizeof(COM_QUERY_INTERFACE_RPL));
    if (rpl) {
        rpl->hr = pMap->hr;
        rpl->objidx = pMap->objidx;
    }

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// AddRefReleaseHandler
//---------------------------------------------------------------------------


MSG_HEADER *ComServer::AddRefReleaseHandler(
    MSG_HEADER *msg, void *_slave, BOOLEAN *deleted)
{
    COM_ADD_REF_RELEASE_REQ *req = (COM_ADD_REF_RELEASE_REQ *)msg;
    if (req->h.length < sizeof(COM_ADD_REF_RELEASE_REQ))
        return SHORT_REPLY(E_INVALIDARG);

    COM_SLAVE *slave = (COM_SLAVE *)_slave;
    COM_SLAVE_MAP *pMap = slave->pMap;

    pMap->objidx = req->objidx;
    pMap->ProcNum = req->op;

    ULONG exc = CallSlave(slave, 0x34, deleted);
    if (exc)
        return SHORT_REPLY(exc);

    COM_ADD_REF_RELEASE_RPL *rpl = (COM_ADD_REF_RELEASE_RPL *)
                                LONG_REPLY(sizeof(COM_ADD_REF_RELEASE_RPL));
    if (rpl) {
        rpl->hr = pMap->hr;
        rpl->refcount = pMap->ProcNum;
    }

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// InvokeMethodHandler
//---------------------------------------------------------------------------


MSG_HEADER *ComServer::InvokeMethodHandler(
    MSG_HEADER *msg, void *_slave, BOOLEAN *deleted)
{
    COM_INVOKE_METHOD_REQ *req = (COM_INVOKE_METHOD_REQ *)msg;
    if (req->h.length < sizeof(COM_INVOKE_METHOD_REQ))
        return SHORT_REPLY(E_INVALIDARG);
    if (req->BufferLength >= MAX_MAP_BUFFER_LENGTH) {
        LogErr(_slave, 0x51, req->BufferLength);
        return SHORT_REPLY(MEM_E_INVALID_SIZE);
    }
    ULONG offset = FIELD_OFFSET(COM_INVOKE_METHOD_REQ, Buffer);
    if (offset + req->BufferLength > req->h.length)
        return SHORT_REPLY(E_INVALIDARG);

    COM_SLAVE *slave = (COM_SLAVE *)_slave;
    COM_SLAVE_MAP *pMap = slave->pMap;

    pMap->objidx = req->objidx;
    pMap->DataRepresentation = req->DataRepresentation;
    pMap->ProcNum = req->ProcNum;
    pMap->BufferLength = req->BufferLength;
    memcpy(pMap->Buffer, req->Buffer, req->BufferLength);

    ULONG exc = CallSlave(slave, 0x35, deleted);
    if (exc) {
        if (exc == RPC_S_UNSUPPORTED_TYPE && pMap->hr == E_ABORT)
            LogErr(slave, 0x42, pMap->BufferLength);
        return SHORT_REPLY(exc);
    }

    ULONG rpl_len = sizeof(COM_INVOKE_METHOD_RPL) + slave->pMap->BufferLength;
    COM_INVOKE_METHOD_RPL *rpl = (COM_INVOKE_METHOD_RPL *)LONG_REPLY(rpl_len);
    if (rpl) {
        rpl->hr = pMap->hr;
        rpl->DataRepresentation = pMap->DataRepresentation;
        rpl->BufferLength = pMap->BufferLength;
        memcpy(rpl->Buffer, pMap->Buffer, pMap->BufferLength);
    }

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// UnmarshalInterfaceHandler
//---------------------------------------------------------------------------


MSG_HEADER *ComServer::UnmarshalInterfaceHandler(
    MSG_HEADER *msg, void *_slave, BOOLEAN *deleted)
{
    COM_UNMARSHAL_INTERFACE_REQ *req = (COM_UNMARSHAL_INTERFACE_REQ *)msg;
    if (req->h.length < sizeof(COM_UNMARSHAL_INTERFACE_REQ))
        return SHORT_REPLY(E_INVALIDARG);
    if (req->BufferLength >= COM_MAX_UNMARSHAL_BUF_LEN) {
        LogErr(_slave, 0x52, req->BufferLength);
        return SHORT_REPLY(MEM_E_INVALID_SIZE);
    }
    ULONG offset = FIELD_OFFSET(COM_UNMARSHAL_INTERFACE_REQ, Buffer);
    if (offset + req->BufferLength > req->h.length)
        return SHORT_REPLY(E_INVALIDARG);

    COM_SLAVE *slave = (COM_SLAVE *)_slave;
    COM_SLAVE_MAP *pMap = slave->pMap;

    pMap->BufferLength = sizeof(GUID) + req->BufferLength;
    GUID *guid = (GUID *)pMap->Buffer;
    memcpy(guid, &req->iid, sizeof(GUID));
    memcpy(&guid[1], req->Buffer, req->BufferLength);

    ULONG exc = CallSlave(slave, 0x36, deleted);
    if (exc)
        return SHORT_REPLY(exc);

    COM_UNMARSHAL_INTERFACE_RPL *rpl = (COM_UNMARSHAL_INTERFACE_RPL *)
                            LONG_REPLY(sizeof(COM_UNMARSHAL_INTERFACE_RPL));
    if (rpl) {
        rpl->hr = pMap->hr;
        rpl->objidx = pMap->objidx;
    }

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// MarshalInterfaceHandler
//---------------------------------------------------------------------------


MSG_HEADER *ComServer::MarshalInterfaceHandler(
    MSG_HEADER *msg, void *_slave, BOOLEAN *deleted)
{
    COM_MARSHAL_INTERFACE_REQ *req = (COM_MARSHAL_INTERFACE_REQ *)msg;
    if (req->h.length < sizeof(COM_MARSHAL_INTERFACE_REQ))
        return SHORT_REPLY(E_INVALIDARG);

    COM_SLAVE *slave = (COM_SLAVE *)_slave;
    COM_SLAVE_MAP *pMap = slave->pMap;

    pMap->objidx = req->objidx;
    pMap->DataRepresentation = req->destctx;
    pMap->ProcNum = req->mshlflags;
    pMap->BufferLength = sizeof(GUID);
    memcpy(pMap->Buffer, &req->iid, sizeof(GUID));

    ULONG exc = CallSlave(slave, 0x37, deleted);
    if (exc)
        return SHORT_REPLY(exc);

    ULONG rpl_len = sizeof(COM_MARSHAL_INTERFACE_RPL)
                  + slave->pMap->BufferLength;
    COM_MARSHAL_INTERFACE_RPL *rpl =
                        (COM_MARSHAL_INTERFACE_RPL *)LONG_REPLY(rpl_len);
    if (rpl) {
        rpl->hr = pMap->hr;
        rpl->BufferLength = pMap->BufferLength;
        memcpy(rpl->Buffer, pMap->Buffer, pMap->BufferLength);
    }

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// QueryBlanketHandler
//---------------------------------------------------------------------------


MSG_HEADER *ComServer::QueryBlanketHandler(
    MSG_HEADER *msg, void *_slave, BOOLEAN *deleted)
{
    COM_QUERY_BLANKET_REQ *req = (COM_QUERY_BLANKET_REQ *)msg;
    if (req->h.length < sizeof(COM_QUERY_BLANKET_REQ))
        return SHORT_REPLY(E_INVALIDARG);

    COM_SLAVE *slave = (COM_SLAVE *)_slave;
    COM_SLAVE_MAP *pMap = slave->pMap;

    pMap->objidx = req->objidx;
    pMap->BufferLength = 0;

    ULONG exc = CallSlave(slave, 0x38, deleted);
    if (exc)
        return SHORT_REPLY(exc);

    COM_QUERY_BLANKET_RPL *rpl =
        (COM_QUERY_BLANKET_RPL *)LONG_REPLY(sizeof(COM_QUERY_BLANKET_RPL));
    if (rpl) {
        rpl->hr = pMap->hr;

        COM_QUERY_BLANKET_RPL *buf = (COM_QUERY_BLANKET_RPL *)pMap->Buffer;
        rpl->AuthnSvc     = buf->AuthnSvc;
        rpl->AuthzSvc     = buf->AuthzSvc;
        rpl->AuthnLevel   = buf->AuthnLevel;
        rpl->ImpLevel     = buf->ImpLevel;
        rpl->Capabilities = buf->Capabilities;
        memcpy(rpl->ServerPrincName, buf->ServerPrincName,
               sizeof(rpl->ServerPrincName));
    }

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// SetBlanketHandler
//---------------------------------------------------------------------------


MSG_HEADER *ComServer::SetBlanketHandler(
    MSG_HEADER *msg, void *_slave, BOOLEAN *deleted)
{
    COM_SET_BLANKET_REQ *req = (COM_SET_BLANKET_REQ *)msg;
    if (req->h.length < sizeof(COM_SET_BLANKET_REQ))
        return SHORT_REPLY(E_INVALIDARG);

    COM_SLAVE *slave = (COM_SLAVE *)_slave;
    COM_SLAVE_MAP *pMap = slave->pMap;

    pMap->objidx = req->objidx;
    pMap->BufferLength = sizeof(COM_SET_BLANKET_REQ);
    memcpy(pMap->Buffer, req, sizeof(COM_SET_BLANKET_REQ));

    ULONG exc = CallSlave(slave, 0x39, deleted);
    if (exc)
        return SHORT_REPLY(exc);

    COM_SET_BLANKET_RPL *rpl =
        (COM_SET_BLANKET_RPL *)LONG_REPLY(sizeof(COM_SET_BLANKET_RPL));
    if (rpl)
        rpl->hr = pMap->hr;

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// CopyProxyHandler
//---------------------------------------------------------------------------


MSG_HEADER *ComServer::CopyProxyHandler(
    MSG_HEADER *msg, void *_slave, BOOLEAN *deleted)
{
    COM_COPY_PROXY_REQ *req = (COM_COPY_PROXY_REQ *)msg;
    if (req->h.length < sizeof(COM_COPY_PROXY_REQ))
        return SHORT_REPLY(E_INVALIDARG);

    COM_SLAVE *slave = (COM_SLAVE *)_slave;
    COM_SLAVE_MAP *pMap = slave->pMap;

    pMap->objidx = req->objidx;
    pMap->BufferLength = 0;

    ULONG exc = CallSlave(slave, 0x3A, deleted);
    if (exc)
        return SHORT_REPLY(exc);

    COM_COPY_PROXY_RPL *rpl =
        (COM_COPY_PROXY_RPL *)LONG_REPLY(sizeof(COM_COPY_PROXY_RPL));
    if (rpl) {
        rpl->hr = pMap->hr;
        rpl->objidx = pMap->objidx;
    }

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// NonComHandler
//---------------------------------------------------------------------------


MSG_HEADER *ComServer::NonComHandler(
    MSG_HEADER *msg, void *_slave, BOOLEAN *deleted)
{
    if (msg->length < sizeof(MSG_HEADER) ||
        msg->length >= MAX_MAP_BUFFER_LENGTH)
        return SHORT_REPLY(ERROR_INVALID_PARAMETER);

    COM_SLAVE *slave = (COM_SLAVE *)_slave;
    COM_SLAVE_MAP *pMap = slave->pMap;

    pMap->msgid = msg->msgid;
    pMap->BufferLength = msg->length;
    memcpy(pMap->Buffer, msg, msg->length);

    ULONG exc = CallSlave(slave, (msg->msgid & 0xFF), deleted);
    if (exc)
        return SHORT_REPLY(exc);

    msg = (MSG_HEADER *)pMap->Buffer;
    MSG_HEADER *rpl = (MSG_HEADER *)LONG_REPLY(msg->length);
    if (rpl)
        memcpy(rpl, msg, msg->length);

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// LockSlave
//---------------------------------------------------------------------------


void *ComServer::LockSlave(HANDLE idProcess, ULONG msgid)
{
    static const WCHAR *_tmpl = SANDBOXIE L"_ComProxy_%s_%s_%d_%d_%s";
    ULONG session_id;
    union {
        struct {
            WCHAR boxname[BOXNAME_COUNT];
            WCHAR sid[96];
        } s;
        WCHAR path[192];
    } u;

    //
    // identify the calling sandboxed program so we can select or
    // create the proper SbieSvc proxy COM server for it
    //

    LONG rc = SbieApi_QueryProcess(
                    idProcess, u.s.boxname, NULL, u.s.sid, &session_id);
    if (rc != 0)
        return NULL;

    //
    // identify 32-bit or 64-bit process
    //

    BOOLEAN IsWow64 = FALSE;

#ifdef _WIN64

    IsWow64 = IsProcessWoW64(idProcess);

#ifdef DEBUG_COMSERVER
    WCHAR txt[256]; wsprintf(txt, L"LockSlave     idProcess=%d Wow64=%d msgid=%X\n", idProcess, IsWow64, msgid);
    OutputDebugString(txt);
#endif
#endif _WIN64

    //
    // prepare for main loop
    //

    ULONG error = 0;
    ULONG detail;

    //
    // try to locate an existing server
    //

    bool issued_timeout_message = false;

RetryLockSlave:

    EnterCriticalSection(&m_SlavesLock);

    COM_SLAVE *slave = (COM_SLAVE *)List_Head(&m_SlavesList);
    while (slave) {

        //
        // if we found a slave that is scheduled for deletion,
        // check if we can delete it now
        //

        if (slave->not_responding) {

            if (TryEnterCriticalSection(&slave->lock)) {

                DeleteSlave(slave);

                LeaveCriticalSection(&m_SlavesLock);

                goto RetryLockSlave;
            }

        //
        // otherwise check if this is a functional slave that
        // is associated with the caller
        //

        } else if (_wcsicmp(slave->SidString, u.s.sid) == 0 &&
                   _wcsicmp(slave->BoxName, u.s.boxname) == 0 &&
                   slave->SessionId == session_id &&
                   slave->IsWow64 == IsWow64) {

#ifdef DEBUG_COMSERVER
            WCHAR txt[256]; wsprintf(txt, L"LockSlave     idProcess=%d SlaveProcess=%d\n", idProcess, slave->idProcess);
            OutputDebugString(txt);
#endif

            break;
        }

        slave = (COM_SLAVE *)List_Next(slave);
    }

    //
    // if we found an existing slave object, try to take ownership.
    // if not possible, some other thread must be holding it, so wait
    // for event that says some slave has been released, then retry
    //

    if (slave) {

        BOOL locked = TryEnterCriticalSection(&slave->lock);
        if (locked) {

            slave->not_responding = FALSE;
            slave->lock_ticks = GetTickCount();

        } else {

            if (GetTickCount() - slave->lock_ticks >=
                                        SLAVE_TIMEOUT_SECONDS * 2 * 1000) {

                //
                // slave is not responding for too long, mark it as such
                //

                if (! issued_timeout_message) {
                    LogErr(session_id, 0x30, ERROR_WAIT_1);
                    issued_timeout_message = true;
                }

                if (! slave->non_com_request) {
                    slave->not_responding = TRUE;
                    slave = NULL;
                }
            }

            if (slave) {

                //
                // wait a while for the slave to become responsive
                //

                LeaveCriticalSection(&m_SlavesLock);

                WaitForSingleObject(m_SlaveReleasedEvent,
                                    SLAVE_TIMEOUT_SECONDS * 1000);

                goto RetryLockSlave;
            }
        }
    }

    //
    // if the slave process was killed, discard it
    //

    if (slave) {

        if (WaitForSingleObject(slave->hProcess, 0) == WAIT_OBJECT_0) {

            DeleteSlave(slave);
            slave = NULL;
        }
    }

    //
    // if a matching server was not found, and this is a proxy
    // creation request (MSGID_COM_GET_CLASS_OBJECT), then create
    // a new slave process.
    //
    // we also create a server if this is a non-COM request,
    // i.e. requests with msgid > 0x80
    //

    if ((! slave) && (msgid == MSGID_COM_GET_CLASS_OBJECT ||
                      msgid == MSGID_COM_UNMARSHAL_INTERFACE ||
                      ((msgid & 0xFF) > 0x80))) {

        HANDLE hImpToken = NULL;
        HANDLE hPriToken = NULL;
        void *env = NULL;
        WCHAR *cmdline = NULL;
        BOOL ok;

        slave = (COM_SLAVE *)
            HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(COM_SLAVE));
        if (! slave) {
            error = 0x11;
            goto slave_create_done;
        }

        wcscpy(slave->SidString, u.s.sid);
        wcscpy(slave->BoxName, u.s.boxname);
        slave->SessionId = session_id;
        slave->IsWow64 = IsWow64;

        InitializeCriticalSection(&slave->lock);
        slave->lock_initialized = TRUE;

        slave->not_responding = FALSE;
        slave->lock_ticks = GetTickCount();

        slave->non_com_request = FALSE;

        wsprintf(u.path, _tmpl, slave->SidString, slave->BoxName,
                                slave->SessionId, slave->IsWow64, L"Mutex");
        slave->hMutex = CreateMutex(NULL, TRUE, u.path);
        if (! slave->hMutex) {
            error = 0x17;
            goto slave_create_done;
        }

        wsprintf(u.path, _tmpl, slave->SidString, slave->BoxName,
                                slave->SessionId, slave->IsWow64, L"Event1");
        slave->hEvent1 = CreateEvent(NULL, FALSE, FALSE, u.path);
        if (! slave->hEvent1) {
            error = 0x13;
            goto slave_create_done;
        }

        wsprintf(u.path, _tmpl, slave->SidString, slave->BoxName,
                                slave->SessionId, slave->IsWow64, L"Event2");
        slave->hEvent2 = CreateEvent(NULL, FALSE, FALSE, u.path);
        if (! slave->hEvent2) {
            error = 0x14;
            goto slave_create_done;
        }

        wsprintf(u.path, _tmpl, slave->SidString, slave->BoxName,
                                slave->SessionId, slave->IsWow64, L"Map");
        slave->hMap = CreateFileMapping(
                        INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
                        COM_SLAVE_MAP_SIZE, u.path);
        if (! slave->hMap) {
            error = 0x15;
            goto slave_create_done;
        }

        slave->pMap = (COM_SLAVE_MAP *)MapViewOfFile(
            slave->hMap, FILE_MAP_ALL_ACCESS, 0, 0, COM_SLAVE_MAP_SIZE);
        if (! slave->pMap) {
            error = 0x16;
            goto slave_create_done;
        }

        ok = OpenThreadToken(GetCurrentThread(),
                TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY,
                FALSE, &hImpToken);
        if (! ok) {
            error = 0x18;
            goto slave_create_done;
        }

        ok = DuplicateTokenEx(hImpToken, 0, NULL, SecurityImpersonation,
                              TokenPrimary, &hPriToken);
        if (! ok) {
            error = 0x19;
            goto slave_create_done;
        }

        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        const WCHAR *_SbieSvc_Exe =
#ifdef _WIN64
                        IsWow64 ? L"32\\" SBIESVC_EXE : SBIESVC_EXE;
#else ! _WIN64
                        SBIESVC_EXE;
#endif _WIN64

        wsprintf(u.path, _tmpl, slave->SidString, slave->BoxName,
                                slave->SessionId, slave->IsWow64, L":");
        ok = SbieDll_RunFromHome(_SbieSvc_Exe, u.path, &si, NULL);
        if (! ok) {
            error = 0x20;
            goto slave_create_done;
        }

        ok = CreateEnvironmentBlock(&env, hPriToken, FALSE);
        if (! ok) {
            error = 0x21;
            goto slave_create_done;
        }

        cmdline = (WCHAR *)si.lpReserved;

        memzero(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);
        si.dwFlags = STARTF_FORCEOFFFEEDBACK;

        ok = CreateProcessAsUser(hPriToken, NULL, cmdline, NULL, NULL,
                                 FALSE, CREATE_UNICODE_ENVIRONMENT, env,
                                 NULL, &si, &pi);
        if (! ok) {
            error = 0x22;
            goto slave_create_done;
        }

        CloseHandle(pi.hThread);
        slave->hProcess = pi.hProcess;
        slave->idProcess = pi.dwProcessId;

#ifdef DEBUG_COMSERVER
            WCHAR txt[256]; wsprintf(txt, L"LockSlave     SlaveProcess=%d created for idProcess=%d\n", slave->idProcess, idProcess);
            OutputDebugString(txt);
#endif

slave_create_done:

        if (error)
            detail = GetLastError();

        if (env)
            DestroyEnvironmentBlock(env);

        if (cmdline)
            HeapFree(GetProcessHeap(), 0, cmdline);

        if (hPriToken)
            CloseHandle(hPriToken);

        if (hImpToken)
            CloseHandle(hImpToken);

        if (! error) {

            EnterCriticalSection(&slave->lock);

            List_Insert_After(&m_SlavesList, NULL, slave);

        } else {

            DeleteSlave(slave);
            slave = NULL;
        }
    }

    //
    // at this point, the slave is locked,
    // prepare its communication area for a call to CallSlave
    //

    if (slave) {

        memzero(slave->pMap, sizeof(COM_SLAVE_MAP));
        slave->pMap->msgid = msgid;
        slave->pMap->idProcess = (ULONG)(ULONG_PTR)idProcess;
    }

    //
    // finish
    //

    LeaveCriticalSection(&m_SlavesLock);

    if (error)
        LogErr(session_id, error, detail);

    return slave;
}


//---------------------------------------------------------------------------
// CallSlave
//---------------------------------------------------------------------------


ULONG ComServer::CallSlave(void *_slave, ULONG callid, BOOLEAN *deleted)
{
    COM_SLAVE *slave = (COM_SLAVE *)_slave;

    //
    // at this point the slave communication area is populated,
    // wake the slave to do the job
    //

    SetEvent(slave->hEvent1);

    //
    // wait for the slave to finish, or for its process to end
    //

    HANDLE handles[2];
    handles[0] = slave->hProcess;
    handles[1] = slave->hEvent2;

    ULONG timeout = SLAVE_TIMEOUT_SECONDS * 1000;
    if (callid > 0x80) {    // non-COM slave
        timeout = INFINITE;
        slave->non_com_request = TRUE;
    }

    ULONG status = WaitForMultipleObjects(2, handles, FALSE, timeout);

    //
    // if this was a delete slave command, we don't care about
    // the results, otherwise complain if the slave process died
    //

    if (callid == 0x5F)
        return 0;

    if (status == STATUS_WAIT_1) {

        if (slave->non_com_request) {
            slave->lock_ticks = GetTickCount();
            slave->non_com_request = FALSE;
        }

        return slave->pMap->exc;
    }

    //
    // slave failed, report error and delete it
    //

    LogErr(slave, callid, status);
#ifdef DEBUG_COMSERVER
    WCHAR txt[256]; wsprintf(txt, L"CallSlave error, Pid = %d, status = %d\n",
        GetCurrentProcessId(), status); OutputDebugString(txt);
#endif

    EnterCriticalSection(&m_SlavesLock);

    DeleteSlave(slave);

    LeaveCriticalSection(&m_SlavesLock);

    *deleted = TRUE;

    return RPC_S_SERVER_UNAVAILABLE;
}


//---------------------------------------------------------------------------
// DeleteSlave
//---------------------------------------------------------------------------


void ComServer::DeleteSlave(void *_slave)
{
    COM_SLAVE *slave = (COM_SLAVE *)_slave;

    List_Remove(&m_SlavesList, slave);

    if (slave->hProcess) {

        if (slave->hEvent1 && slave->hEvent2 && slave->pMap) {
            slave->pMap->msgid = -1;
            CallSlave(slave, 0x5F, NULL);
        }

        TerminateProcess(slave->hProcess, 0);

        CloseHandle(slave->hProcess);
    }

    if (slave->pMap)
        UnmapViewOfFile(slave->pMap);

    if (slave->hMap)
        CloseHandle(slave->hMap);

    if (slave->hEvent2)
        CloseHandle(slave->hEvent2);

    if (slave->hEvent1)
        CloseHandle(slave->hEvent1);

    if (slave->hMutex)
        CloseHandle(slave->hMutex);

    if (slave->lock_initialized)
        DeleteCriticalSection(&slave->lock);

    HeapFree(GetProcessHeap(), 0, slave);
}


//---------------------------------------------------------------------------
// NotifyAllSlaves
//---------------------------------------------------------------------------


void ComServer::NotifyAllSlaves(HANDLE idProcess)
{
    //
    // create a thread for sending notifications to the slaves
    //

    HANDLE hThread;
    ULONG idThread;

    ULONG64 *ThreadData = (ULONG64 *)
        HeapAlloc(GetProcessHeap(), 0, sizeof(ULONG64) * 2);
    if (! ThreadData)
        return;

    ThreadData[0] = (ULONG64)this;
    ThreadData[1] = (ULONG64)(ULONG_PTR)idProcess;
    hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)NotifyAllSlaves2,
                           ThreadData, 0, &idThread);
    if (hThread)
        CloseHandle(hThread);
    else
        HeapFree(GetProcessHeap(), 0, ThreadData);
}


//---------------------------------------------------------------------------
// NotifyAllSlaves2
//---------------------------------------------------------------------------


void ComServer::NotifyAllSlaves2(ULONG64 *ThreadData)
{
    ComServer *pThis = (ComServer *)ThreadData[0];
    ULONG_PTR idProcess = (ULONG)ThreadData[1];
    HeapFree(GetProcessHeap(), 0, ThreadData);
    pThis->NotifyAllSlaves3((HANDLE)idProcess);
}


//---------------------------------------------------------------------------
// NotifyAllSlaves3
//---------------------------------------------------------------------------


void ComServer::NotifyAllSlaves3(HANDLE idProcess)
{
    BOOLEAN any_slaves_skipped = FALSE;
    ULONG retry_count = 0;

    //
    // go through the list of slaves, notify and potentially
    // delete each one
    //

RetryNotifySlave:

    if (! TryEnterCriticalSection(&m_SlavesLock)) {

        //
        // LockSlave might be waiting for m_SlaveReleasedEvent
        //

        SetEvent(m_SlaveReleasedEvent);
        Sleep(50);
        goto RetryNotifySlave;
    }

    COM_SLAVE *slave = (COM_SLAVE *)List_Head(&m_SlavesList);
    while (slave) {
        COM_SLAVE *next_slave = (COM_SLAVE *)List_Next(slave);

        //
        // try to take ownership of the slave, if not possible,
        // delay it for later
        //

        BOOL locked = TryEnterCriticalSection(&slave->lock);
        if (! locked) {

            any_slaves_skipped = TRUE;

            slave = next_slave;
            continue;
        }

        //
        // call the slave to tell it a process has terminated,
        // and see if it reports it has no more objects left
        //

        memzero(slave->pMap, sizeof(COM_SLAVE_MAP));
        slave->pMap->msgid = MSGID_COM_NOTIFICATION;
        slave->pMap->idProcess = (ULONG)(ULONG_PTR)idProcess;

        ULONG exc = CallSlave(slave, 0x5F, NULL);
        if (exc == 0 && slave->pMap->ProcNum == -1) {

            //
            // slave returns with ProcNum == -1 when it has no more objects
            //

            DeleteSlave(slave);

        } else {

            LeaveCriticalSection(&slave->lock);
        }

        slave = next_slave;
    }

    LeaveCriticalSection(&m_SlavesLock);

    //
    // if we skipped some slaves because they were busy at the time,
    // then try again, for a limited number of times
    //

    if (any_slaves_skipped && retry_count < 5) {

        any_slaves_skipped = FALSE;
        ++retry_count;

        Sleep(250 + retry_count * 250);

        goto RetryNotifySlave;
    }
}


//---------------------------------------------------------------------------
// DeleteAllSlaves
//---------------------------------------------------------------------------


void ComServer::DeleteAllSlaves()
{
    EnterCriticalSection(&m_SlavesLock);

    while (1) {
        COM_SLAVE *slave = (COM_SLAVE *)List_Head(&m_SlavesList);
        if (! slave)
            break;
        DeleteSlave(slave);
    }

    LeaveCriticalSection(&m_SlavesLock);
}


//---------------------------------------------------------------------------
// LogErr
//---------------------------------------------------------------------------


void ComServer::LogErr(ULONG session, ULONG n1, ULONG n2)
{
    SbieApi_LogEx(session, 2327, L"[%02X / %d]", n1, n2);
}


void ComServer::LogErr(void *_slave, ULONG n1, ULONG n2)
{
    COM_SLAVE *slave = (COM_SLAVE *)_slave;
    LogErr(slave->SessionId, n1, n2);
}


//---------------------------------------------------------------------------
//
// Slave Process
//
//---------------------------------------------------------------------------


extern "C" void ComServer_Main9(const WCHAR *cmdline);   // comserver9.cpp


bool ComServer::m_AnySlaveObjectCreated = false;


//---------------------------------------------------------------------------
// RunSlave
//---------------------------------------------------------------------------


void ComServer::RunSlave(const WCHAR *cmdline)
{
    static const WCHAR *_Global = L"Global\\";

    /*while (! IsDebuggerPresent()) {
        OutputDebugString(cmdline);
        Sleep(1000);
    } __debugbreak();*/

    //
    // locate IPC objects set up by the parent process SbieSvc
    //

    if (wcslen(cmdline) > 100)
        return;

    if (wcsstr(cmdline, SANDBOXIE L"_ComProxy_ComServer:")) {

        //
        // this is a request to run a simulated COM server,
        // rather than a proxy slave for SbieSvc ComServer
        //
        // see also Custom_ComServer in core/dll/custom.c
        // and core/svc/ProcessServer.cpp
        //

        ComServer_Main9(cmdline);
        return;
    }

    WCHAR objname[192];
    wcscpy(objname, _Global);
    wcscat(objname, cmdline);
    WCHAR *colon = wcsrchr(objname, L':');

    wcscpy(objname, _Global);
    wcscat(objname, cmdline);
    wcscpy(colon, L"Mutex");
    HANDLE hParentProcessMutex =
                OpenMutex(MUTEX_MODIFY_STATE | SYNCHRONIZE, FALSE, objname);
    if (! hParentProcessMutex)
        return;

    wcscpy(objname, _Global);
    wcscat(objname, cmdline);
    wcscpy(colon, L"Event1");
    HANDLE hEvent1 =
                OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, objname);
    if (! hEvent1)
        return;

    wcscpy(objname, _Global);
    wcscat(objname, cmdline);
    wcscpy(colon, L"Event2");
    HANDLE hEvent2 =
                OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, objname);
    if (! hEvent1)
        return;

    wcscpy(objname, _Global);
    wcscat(objname, cmdline);
    wcscpy(colon, L"Map");
    HANDLE hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, objname);
    if (! hMap)
        return;

    COM_SLAVE_MAP *pMap = (COM_SLAVE_MAP *)
        MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, COM_SLAVE_MAP_SIZE);
    if (! pMap)
        return;

    //
    // initialize and begin main loop
    //

    m_heap = HeapCreate(0, 0, 0);
    if (! m_heap)
        return;

    LIST ObjectsList;
    List_Init(&ObjectsList);

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    CoInitializeSecurity(
        NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);

    //
    // main loop
    //

    while (1) {

        //
        // wait for a request or for the parent process SbieSvc to end
        // (would get STATUS_ABANDONED on the mutex object)
        //

        HANDLE handles[2];
        handles[0] = hParentProcessMutex;
        handles[1] = hEvent1;

        ULONG rc = WaitForMultipleObjects(2, handles, FALSE, INFINITE);

        if (rc != STATUS_WAIT_1 || pMap->msgid == -1) {

            ExitProcess(0);
            //CoUninitialize();
            //return;
        }

        //
        // handle incoming request
        //

        ULONG exc = 0;
        HRESULT hr = 0;

        __try {

            if (pMap->msgid == MSGID_COM_GET_CLASS_OBJECT)
                GetClassObjectSlave(pMap, &ObjectsList, &exc, &hr);

            else if (pMap->msgid == MSGID_COM_CREATE_INSTANCE)
                CreateInstanceSlave(pMap, &ObjectsList, &exc, &hr);

            else if (pMap->msgid == MSGID_COM_QUERY_INTERFACE)
                QueryInterfaceSlave(pMap, &ObjectsList, &exc, &hr);

            else if (pMap->msgid == MSGID_COM_ADD_REF_RELEASE)
                AddRefReleaseSlave(pMap, &ObjectsList, &exc, &hr);

            else if (pMap->msgid == MSGID_COM_INVOKE_METHOD)
                InvokeMethodSlave(pMap, &ObjectsList, &exc, &hr);

            else if (pMap->msgid == MSGID_COM_UNMARSHAL_INTERFACE)
                UnmarshalInterfaceSlave(pMap, &ObjectsList, &exc, &hr);

            else if (pMap->msgid == MSGID_COM_MARSHAL_INTERFACE)
                MarshalInterfaceSlave(pMap, &ObjectsList, &exc, &hr);

            else if (pMap->msgid == MSGID_COM_QUERY_BLANKET)
                QueryBlanketSlave(pMap, &ObjectsList, &exc, &hr);

            else if (pMap->msgid == MSGID_COM_SET_BLANKET)
                SetBlanketSlave(pMap, &ObjectsList, &exc, &hr);

            else if (pMap->msgid == MSGID_COM_COPY_PROXY)
                CopyProxySlave(pMap, &ObjectsList, &exc, &hr);

            else if (pMap->msgid == MSGID_COM_CRYPT_PROTECT_DATA)
                exc = CryptProtectDataSlave(pMap->Buffer);

            else if (pMap->msgid == MSGID_COM_NOTIFICATION)
                ProcessNotifySlave(pMap, &ObjectsList, &exc, &hr);

            else {
                exc = RPC_S_CANNOT_SUPPORT;
                hr = E_NOTIMPL;
            }

        } __except (EXCEPTION_EXECUTE_HANDLER) {

            exc = RPC_S_INTERNAL_ERROR;
            hr = E_ABORT;
        }

        if (exc != -1 || hr != -1) {

            if (exc != 0 || hr != 0)
                pMap->objidx = 0;

            pMap->exc = exc;
            pMap->hr = hr;
        }

        SetEvent(hEvent2);
    }
}


//---------------------------------------------------------------------------
// FindSlaveObject
//---------------------------------------------------------------------------


void *ComServer::FindSlaveObject(void *_map, LIST *ObjectsList,
                                 ULONG *exc, HRESULT *hr)
{
    COM_SLAVE_MAP *pMap = (COM_SLAVE_MAP *)_map;

    COM_OBJECT *obj = (COM_OBJECT *)List_Head(ObjectsList);
    while (obj) {
        if (pMap->idProcess == obj->idProcess &&
            pMap->objidx    == obj->objidx)
        {
            break;
        }
        obj = (COM_OBJECT *)List_Next(obj);
    }

#ifdef DEBUG_COMSERVER
    WCHAR txt[256]; wsprintf(txt, L"FindSlaveObject         objidx=%08X idProcess=%06d obj=%08X\n",
        pMap->objidx, pMap->idProcess, obj); OutputDebugString(txt);
#endif

    if (! obj) {
        *exc = RPC_S_OBJECT_NOT_FOUND;
        *hr = E_ABORT;
    }

    return obj;
}


//---------------------------------------------------------------------------
// RefOrAllocSlaveObject
//---------------------------------------------------------------------------


void *ComServer::RefOrAllocSlaveObject(ULONG idProcess, void *pUnknown,
                                       LIST *ObjectsList,
                                       ULONG *exc, HRESULT *hr)
{
    COM_OBJECT *obj = (COM_OBJECT *)List_Head(ObjectsList);
    while (obj) {
        if (idProcess == obj->idProcess &&
            pUnknown  == obj->pUnknown)
        {
            obj->pUnknown->AddRef();
            break;
        }
        obj = (COM_OBJECT *)List_Next(obj);
    }

#ifdef DEBUG_COMSERVER
    WCHAR txt[256]; wsprintf(txt, L"RefSlaveObject          pUnknown=%08X idProcess=%06d obj=%08X\n",
        pUnknown, idProcess, obj); OutputDebugString(txt);
#endif

    if (! obj) {

        obj = (COM_OBJECT *)HeapAlloc(m_heap, 0, sizeof(COM_OBJECT));
        if (obj) {

            memzero(obj, sizeof(COM_OBJECT));

            obj->pUnknown = (IUnknown *)pUnknown;
            obj->pUnknownVtbl = *(ULONG_PTR *)pUnknown;

            obj->idProcess = idProcess;

            obj->objidx   = InterlockedIncrement(&m_ObjIdx);
            while (m_ObjIdx == 0 || m_ObjIdx == -1)
                InterlockedIncrement(&m_ObjIdx);

            List_Insert_After(ObjectsList, NULL, obj);

            m_AnySlaveObjectCreated = true;

        } else {

            ((IUnknown *)pUnknown)->Release();
            *exc = RPC_S_OUT_OF_RESOURCES;
            *hr = E_OUTOFMEMORY;
        }
    }

    return obj;
}


//---------------------------------------------------------------------------
// DeleteSlaveObject
//---------------------------------------------------------------------------


void ComServer::DeleteSlaveObject(void *_obj, LIST *ObjectsList)
{
    COM_OBJECT *obj = (COM_OBJECT *)_obj;
    COM_OBJECT *obj2;
    ULONG refcount, objcount;

#ifdef DEBUG_COMSERVER
    WCHAR txt[256]; wsprintf(txt, L"DeleteSlaveObject       objidx=%08X idProcess=%06d obj=%08X\n",
        obj->objidx, obj->idProcess, obj); OutputDebugString(txt);
#endif

    if (obj->Flags & FLAG_INPROC_SERVER) {
        // we don't delete inproc class factories when a process ends,
        // because the underlying DLL may be used by more than one process,
        // and forcefully deleting the object would mess up the DLL data
        return;
    }

    if (obj->pStub) {
        do {
            refcount = obj->pStub->Release();
        } while (refcount);
    }

    if (obj->pChannel) {
        do {
            refcount = obj->pChannel->Release();
        } while (refcount);
    }

    //
    // obj->pUnknown may be shared with other objects in the list.
    // if that is the case, then we don't want to forcibly release it,
    // only unlink it from our list
    //

    objcount = 0;

    obj2 = (COM_OBJECT *)List_Head(ObjectsList);
    while (obj2) {
        if (obj2->pUnknown == obj->pUnknown)
            ++objcount;
        obj2 = (COM_OBJECT *)List_Next(obj2);
    }

#ifdef DEBUG_COMSERVER
    wsprintf(txt, L"DeleteSlaveObject       %d shared objects\n", objcount); OutputDebugString(txt);
#endif

    if (objcount == 1) {

        __try {

            //
            // take precautions when releasing the object, because it
            // may have been already freed
            //

            if (*(ULONG_PTR *)obj->pUnknown == obj->pUnknownVtbl) {

                ULONG retries = 0;
                do {
                    refcount = obj->pUnknown->Release();
                    ++retries;
                } while (refcount && retries < 20);
            }

        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }
    }

    List_Remove(ObjectsList, obj);
    HeapFree(m_heap, 0, obj);
}


//---------------------------------------------------------------------------
// GetClassObjectSlave
//---------------------------------------------------------------------------


void ComServer::GetClassObjectSlave(void *_map, LIST *ObjectsList,
                                    ULONG *exc, HRESULT *hr)
{
    COM_SLAVE_MAP *pMap = (COM_SLAVE_MAP *)_map;

    if (pMap->BufferLength != sizeof(GUID) * 2) {
        *exc = RPC_S_INVALID_TAG;
        *hr = E_ABORT;
        return;
    }

    //
    // create the object
    //

    ULONG ObjectFlags = FLAG_CLASS_FACTORY;

    IUnknown *pUnknown;
    GUID *guids = (GUID *)pMap->Buffer;

    if (pMap->ProcNum) {

        //
        // elevate using CoGetObject
        // this is primarily intended for the firewall object
        //

        typedef struct tagBIND_OPTS3 {
            DWORD cbStruct;
            DWORD grfFlags;
            DWORD grfMode;
            DWORD dwTickCountDeadline;
            DWORD dwTrackFlags;
            DWORD dwClassContext;
            LCID locale;
            COSERVERINFO *pServerInfo;
            HWND hwnd;
        } BIND_OPTS3;

        WCHAR moniker[96];
        wcscpy(moniker, L"Elevation:Administrator!clsid:");
        StringFromGUID2(guids[0], moniker + wcslen(moniker), 48);

        BIND_OPTS3 bindopts;
        memzero(&bindopts, sizeof(BIND_OPTS3));
        bindopts.cbStruct = sizeof(BIND_OPTS3);
        bindopts.dwClassContext = CLSCTX_LOCAL_SERVER;

        *hr = CoGetObject(moniker, (BIND_OPTS *)&bindopts,
                          (REFIID)guids[1], (void **)&pUnknown);

    } else {

        //
        // standard CoGetClassObject.
        // note that the second attempt with CLSCTX_INPROC_SERVER
        // is there only for the firewall COM object
        //

        *hr = CoGetClassObject((REFCLSID)guids[0], CLSCTX_LOCAL_SERVER,
                               NULL, (REFIID)guids[1], (void **)&pUnknown);
        if (FAILED(*hr)) {

            *hr = CoGetClassObject((REFCLSID)guids[0], CLSCTX_INPROC_SERVER,
                                NULL, (REFIID)guids[1], (void **)&pUnknown);
            if (FAILED(*hr))
                return;

            ObjectFlags |= FLAG_INPROC_SERVER;
        }
    }

    //
    //
    //

    COM_OBJECT *obj = (COM_OBJECT *)RefOrAllocSlaveObject(
                        pMap->idProcess, pUnknown, ObjectsList, exc, hr);
    if (! obj)
        return;

    obj->Flags |= ObjectFlags;

    pMap->objidx = obj->objidx;

#ifdef DEBUG_COMSERVER
    WCHAR txt[256]; wsprintf(txt, L"(%04d) GetClassObjectSlave     objidx=%08X pUnknown=%08X pChannel=%08X pStub=%08X\n",
        List_Count(ObjectsList), obj->objidx, obj->pUnknown, obj->pChannel, obj->pStub); OutputDebugString(txt);
#endif
}


//---------------------------------------------------------------------------
// CreateInstanceSlave
//---------------------------------------------------------------------------


void ComServer::CreateInstanceSlave(void *_map, LIST *ObjectsList,
                                    ULONG *exc, HRESULT *hr)
{
    COM_SLAVE_MAP *pMap = (COM_SLAVE_MAP *)_map;

    if (pMap->BufferLength != sizeof(GUID)) {
        *exc = RPC_S_INVALID_TAG;
        *hr = E_ABORT;
        return;
    }

    COM_OBJECT *obj = (COM_OBJECT *)FindSlaveObject(
                                            pMap, ObjectsList, exc, hr);
    if (! obj)
        return;

    //
    //
    //

    if (! (obj->Flags & FLAG_CLASS_FACTORY)) {
        *hr = E_ACCESSDENIED;
        return;
    }

    IUnknown *pUnknown;
    GUID *guid = (GUID *)pMap->Buffer;
    IClassFactory *pClassFactory = (IClassFactory *)obj->pUnknown;
    *hr = pClassFactory->CreateInstance(NULL, *guid, (void **)&pUnknown);
    if (FAILED(*hr))
        return;

    //
    //
    //

    obj = (COM_OBJECT *)RefOrAllocSlaveObject(
                        pMap->idProcess, pUnknown, ObjectsList, exc, hr);
    if (! obj)
        return;

    if (memcmp(&obj->iid, &GUID_NULL, sizeof(GUID)) == 0) {

        obj->iid = *guid;

        *hr = SbieDll_ComCreateStub(
            *guid, pUnknown, (void **)&obj->pStub, (void **)&obj->pChannel);

        if (FAILED(*hr)) {
            DeleteSlaveObject(obj, ObjectsList);
            return;
        }
    }

    pMap->objidx = obj->objidx;

#ifdef DEBUG_COMSERVER
    WCHAR txt[256]; wsprintf(txt, L"(%04d) CreateInstanceSlave     objidx=%08X pUnknown=%08X pChannel=%08X pStub=%08X\n",
        List_Count(ObjectsList), obj->objidx, obj->pUnknown, obj->pChannel, obj->pStub); OutputDebugString(txt);
#endif
}


//---------------------------------------------------------------------------
// QueryInterfaceSlave
//---------------------------------------------------------------------------


void ComServer::QueryInterfaceSlave(void *_map, LIST *ObjectsList,
                                    ULONG *exc, HRESULT *hr)
{
    COM_SLAVE_MAP *pMap = (COM_SLAVE_MAP *)_map;

    if (pMap->BufferLength != sizeof(GUID)) {
        *exc = RPC_S_INVALID_TAG;
        *hr = E_ABORT;
        return;
    }

    COM_OBJECT *obj = (COM_OBJECT *)FindSlaveObject(
                                            pMap, ObjectsList, exc, hr);
    if (! obj)
        return;

    //
    //
    //

    IUnknown *pUnknown;
    GUID *guid = (GUID *)pMap->Buffer;
    *hr = obj->pUnknown->QueryInterface(*guid, (void **)&pUnknown);
    if (FAILED(*hr))
        return;

    //
    //
    //

    obj = (COM_OBJECT *)RefOrAllocSlaveObject(
                        pMap->idProcess, pUnknown, ObjectsList, exc, hr);
    if (! obj)
        return;

    if (memcmp(&obj->iid, &GUID_NULL, sizeof(GUID)) == 0) {

        obj->iid = *guid;

        if (memcmp(&obj->iid, &IID_IWbemServices, sizeof(GUID)) == 0)
            obj->Flags |= FLAG_WMI;

        *hr = SbieDll_ComCreateStub(
            *guid, pUnknown, (void **)&obj->pStub, (void **)&obj->pChannel);

        if (FAILED(*hr)) {
            DeleteSlaveObject(obj, ObjectsList);
            return;
        }
    }

    pMap->objidx = obj->objidx;

#ifdef DEBUG_COMSERVER
    WCHAR txt[256]; wsprintf(txt, L"(%04d) QueryInterfaceSlave     objidx=%08X pUnknown=%08X pChannel=%08X pStub=%08X\n",
        List_Count(ObjectsList), obj->objidx, obj->pUnknown, obj->pChannel, obj->pStub); OutputDebugString(txt);
#endif
}


//---------------------------------------------------------------------------
// AddRefReleaseSlave
//---------------------------------------------------------------------------


void ComServer::AddRefReleaseSlave(void *_map, LIST *ObjectsList,
                                    ULONG *exc, HRESULT *hr)
{
    COM_SLAVE_MAP *pMap = (COM_SLAVE_MAP *)_map;

    if (pMap->BufferLength != 0) {
        *exc = RPC_S_INVALID_TAG;
        *hr = E_ABORT;
        return;
    }

    COM_OBJECT *obj = (COM_OBJECT *)FindSlaveObject(
                                            pMap, ObjectsList, exc, hr);
    if (! obj)
        return;

    //
    //
    //

    if (pMap->ProcNum == 'a') {

        pMap->ProcNum = obj->pUnknown->AddRef();

    } else if (pMap->ProcNum == 'r') {

        pMap->ProcNum = obj->pUnknown->Release();

        if (pMap->ProcNum == 1 && obj->pStub) {
            obj->pStub->Disconnect();
            obj->pStub->Release();
            --pMap->ProcNum;
        }

        if (pMap->ProcNum == 0 && obj->pChannel)
            obj->pChannel->Release();

        if (pMap->ProcNum == 0) {
            List_Remove(ObjectsList, obj);
            HeapFree(m_heap, 0, obj);
        }

    } else
        pMap->ProcNum = -1;

#ifdef DEBUG_COMSERVER
    WCHAR txt[256]; wsprintf(txt, L"(%04d) AddRefReleaseSlave      objidx=%08X pUnknown=%08X RefCount=%d\n",
        List_Count(ObjectsList), obj->objidx, obj->pUnknown, pMap->ProcNum); OutputDebugString(txt);
#endif
}


//---------------------------------------------------------------------------
// InvokeMethodSlave
//---------------------------------------------------------------------------


void ComServer::InvokeMethodSlave(void *_map, LIST *ObjectsList,
                                  ULONG *exc, HRESULT *hr)
{
    COM_SLAVE_MAP *pMap = (COM_SLAVE_MAP *)_map;

    COM_OBJECT *obj = (COM_OBJECT *)FindSlaveObject(
                                            pMap, ObjectsList, exc, hr);
    if (! obj)
        return;

    //
    //
    //

    if (obj->Flags & FLAG_CLASS_FACTORY) {
        *hr = E_ACCESSDENIED;
        return;
    }

    if (obj->Flags & FLAG_WMI) {

        BOOLEAN ok;

        if (pMap->ProcNum == 3  ||
            pMap->ProcNum == 6  ||
            pMap->ProcNum == 18 ||
            pMap->ProcNum == 20 ||
            pMap->ProcNum == 23) {
            // approve IWbemServices::OpenNamespace      (method 3)
            // and     IWbemServices::GetObject          (method 6)
            // and     IWbemServices::CreateInstanceEnum (method 18)
            // and     IWbemServices::ExecQuery          (method 20)
            // and     IWbemServices::ExecNotificationQueryAsync (method 23 -- in Win 10 anyway)
            ok = TRUE;
        } else if (pMap->ProcNum == 24) {
            // check parameters for IWbemServices::ExecMethod (method 24)
            ok = CheckWmiExecMethod(pMap->Buffer, pMap->BufferLength);
        } else
            ok = FALSE;

        if (! ok) {
            //
            // issue error if IWbemServices method is not approved
            //
            SbieApi_Log(2205, L"WMI IWbemServices %d", pMap->ProcNum);
            *hr = 0x80041003;   // WBEM_E_ACCESS_DENIED
            return;
        }
    }

    //
    //
    //

    COM_RPC_MESSAGE RpcMsg;
    memzero(&RpcMsg, sizeof(COM_RPC_MESSAGE));

    RpcMsg.DataRepresentation = pMap->DataRepresentation;
    RpcMsg.ProcNum = pMap->ProcNum;
    RpcMsg.BufferLength = pMap->BufferLength;
    *hr = obj->pChannel->GetBuffer((RPCOLEMESSAGE *)&RpcMsg, (GUID)obj->iid);
    if (SUCCEEDED(*hr)) {

        memcpy(RpcMsg.Buffer, pMap->Buffer, pMap->BufferLength);

        *hr = obj->pStub->Invoke((RPCOLEMESSAGE *)&RpcMsg, obj->pChannel);

        pMap->DataRepresentation = RpcMsg.DataRepresentation;

        pMap->BufferLength = RpcMsg.BufferLength;
        if (pMap->BufferLength < MAX_MAP_BUFFER_LENGTH)
            memcpy(pMap->Buffer, RpcMsg.Buffer, RpcMsg.BufferLength);
        else {

            *exc = RPC_S_UNSUPPORTED_TYPE;
            *hr = E_ABORT;
        }

        obj->pChannel->FreeBuffer((RPCOLEMESSAGE *)&RpcMsg);
    }

#ifdef DEBUG_COMSERVER
    WCHAR txt[256]; wsprintf(txt, L"(%04d) InvokeMethodSlave       objidx=%08X pUnknown=%08X pChannel=%08X pStub=%08X HR=%08X\n",
        List_Count(ObjectsList), obj->objidx, obj->pUnknown, obj->pChannel, obj->pStub, *hr); OutputDebugString(txt);
#endif
}


//---------------------------------------------------------------------------
// UnmarshalInterfaceSlave
//---------------------------------------------------------------------------


void ComServer::UnmarshalInterfaceSlave(void *_map, LIST *ObjectsList,
                                        ULONG *exc, HRESULT *hr)
{
    COM_SLAVE_MAP *pMap = (COM_SLAVE_MAP *)_map;

    if (pMap->BufferLength < sizeof(GUID) + 0x18) {
        *exc = RPC_S_INVALID_TAG;
        *hr = E_ABORT;
        return;
    }

    //
    //
    //

    struct buf_t {
        GUID riid;
        ULONG meow;
        ULONG type;
        GUID iid;
    };
    buf_t *buf = (buf_t *)pMap->Buffer;

    ULONG MrshBufLen = pMap->BufferLength - sizeof(GUID);
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, MrshBufLen);
    if (! hGlobal) {
        *exc = RPC_S_OUT_OF_RESOURCES;
        *hr = E_OUTOFMEMORY;
        return;
    }
    void *MrshBufPtr = GlobalLock(hGlobal);
    memcpy(MrshBufPtr, &buf->meow, MrshBufLen);
    GlobalUnlock(hGlobal);

    IStream *pStream;
    *hr = CreateStreamOnHGlobal(hGlobal, TRUE, &pStream);
    if (FAILED(*hr)) {
        GlobalFree(hGlobal);
        *exc = RPC_S_OUT_OF_RESOURCES;
    }

    IUnknown *pUnknown;
    *hr = CoUnmarshalInterface(pStream, buf->riid, (void **)&pUnknown);

    pStream->Release();

    if (FAILED(*hr))
        return;

    //
    //
    //

    COM_OBJECT *obj = (COM_OBJECT *)RefOrAllocSlaveObject(
                        pMap->idProcess, pUnknown, ObjectsList, exc, hr);
    if (! obj)
        return;

    if (memcmp(&obj->iid, &GUID_NULL, sizeof(GUID)) == 0) {

        if (memcmp(&buf->riid, &GUID_NULL, sizeof(GUID)) == 0)
            obj->iid = buf->iid;
        else
            obj->iid = buf->riid;

        if (memcmp(&obj->iid, &IID_IWbemServices, sizeof(GUID)) == 0)
            obj->Flags |= FLAG_WMI;

        *hr = SbieDll_ComCreateStub(obj->iid, pUnknown,
            (void **)&obj->pStub, (void **)&obj->pChannel);

        if (FAILED(*hr)) {
            DeleteSlaveObject(obj, ObjectsList);
            return;
        }
    }

    pMap->objidx = obj->objidx;

#ifdef DEBUG_COMSERVER
    WCHAR txt[256]; wsprintf(txt, L"(%04d) UnmarshalInterfaceSlave objidx=%08X pUnknown=%08X pChannel=%08X pStub=%08X\n",
        List_Count(ObjectsList), obj->objidx, obj->pUnknown, obj->pChannel, obj->pStub); OutputDebugString(txt);
#endif
}


//---------------------------------------------------------------------------
// MarshalInterfaceSlave
//---------------------------------------------------------------------------


void ComServer::MarshalInterfaceSlave(void *_map, LIST *ObjectsList,
                                      ULONG *exc, HRESULT *hr)
{
    COM_SLAVE_MAP *pMap = (COM_SLAVE_MAP *)_map;

    if (pMap->BufferLength < sizeof(GUID)) {
        *exc = RPC_S_INVALID_TAG;
        *hr = E_ABORT;
        return;
    }

    COM_OBJECT *obj;

    if (pMap->objidx == -1) {

        obj = (COM_OBJECT *)FindOrCreateDummySlaveObject(
                                            pMap, ObjectsList, exc, hr);

    } else
        obj = (COM_OBJECT *)FindSlaveObject(pMap, ObjectsList, exc, hr);

    if (! obj)
        return;

    //
    //
    //

    GUID *guid = (GUID *)pMap->Buffer;

    pMap->BufferLength = 0;

    IStream *pStream;
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, 1024);
    if (! hGlobal) {
        *hr = E_OUTOFMEMORY;
        return;
    }
    *hr = CreateStreamOnHGlobal(hGlobal, TRUE, &pStream);
    if (FAILED(*hr)) {
        GlobalFree(hGlobal);
        return;
    }

    ULONG dwDestContext = pMap->DataRepresentation;
    ULONG mshlflags = pMap->ProcNum;
    *hr = CoMarshalInterface(
            pStream, *guid, obj->pUnknown, dwDestContext, NULL, mshlflags);
    if (SUCCEEDED(*hr)) {

        LARGE_INTEGER zero;
        ULARGE_INTEGER size;
        ULONG buf_len;

        zero.QuadPart = 0;
        *hr = pStream->Seek(zero, STREAM_SEEK_CUR, &size);
        if (SUCCEEDED(*hr) && (size.HighPart || size.LowPart > 1024))
            *hr = STG_E_INCOMPLETE;

        if (SUCCEEDED(*hr)) {

            buf_len = size.LowPart;

            *hr = pStream->Seek(zero, STREAM_SEEK_SET, &size);
        }

        if (SUCCEEDED(*hr)) {

            ULONG count = 0;
            *hr = pStream->Read(pMap->Buffer, buf_len, &count);
            if (SUCCEEDED(*hr) && count != buf_len)
                *hr = STG_E_INCOMPLETE;
            if (SUCCEEDED(*hr))
                pMap->BufferLength = buf_len;
        }
    }

    pStream->Release();

#ifdef DEBUG_COMSERVER
    WCHAR txt[256]; wsprintf(txt, L"(%04d) MarshalInterfaceSlave objidx=%08X pUnknown=%08X pChannel=%08X pStub=%08X\n",
        List_Count(ObjectsList), obj->objidx, obj->pUnknown, obj->pChannel, obj->pStub); OutputDebugString(txt);
#endif
}


//---------------------------------------------------------------------------
// QueryBlanketSlave
//---------------------------------------------------------------------------


void ComServer::QueryBlanketSlave(void *_map, LIST *ObjectsList,
                                  ULONG *exc, HRESULT *hr)
{
    COM_SLAVE_MAP *pMap = (COM_SLAVE_MAP *)_map;

    if (pMap->BufferLength != 0) {
        *exc = RPC_S_INVALID_TAG;
        *hr = E_ABORT;
        return;
    }

    COM_OBJECT *obj = (COM_OBJECT *)FindSlaveObject(
                                            pMap, ObjectsList, exc, hr);
    if (! obj)
        return;

    //
    //
    //

    COM_QUERY_BLANKET_RPL *buf = (COM_QUERY_BLANKET_RPL *)pMap->Buffer;

    WCHAR *ServerPrincName;

    *hr = CoQueryProxyBlanket(
            obj->pUnknown, &buf->AuthnSvc, &buf->AuthzSvc, &ServerPrincName,
            &buf->AuthnLevel, &buf->ImpLevel, NULL, &buf->Capabilities);

    if (FAILED(*hr))
        return;

    if (ServerPrincName) {

        ULONG copy_len = wcslen(ServerPrincName) * sizeof(WCHAR);
        ULONG max_len = sizeof(buf->ServerPrincName) - sizeof(WCHAR);
        if (copy_len > max_len)
            copy_len = max_len;
        memcpy(buf->ServerPrincName, ServerPrincName, copy_len);
        buf->ServerPrincName[copy_len / sizeof(WCHAR)] = L'\0';
        CoTaskMemFree(ServerPrincName);

    } else
        buf->ServerPrincName[0] = L'\0';

#ifdef DEBUG_COMSERVER
    WCHAR txt[256]; wsprintf(txt, L"(%04d) QueryBlanketSlave objidx=%08X\n",
        List_Count(ObjectsList), obj->objidx); OutputDebugString(txt);
#endif
}


//---------------------------------------------------------------------------
// SetBlanketSlave
//---------------------------------------------------------------------------


void ComServer::SetBlanketSlave(void *_map, LIST *ObjectsList,
                                ULONG *exc, HRESULT *hr)
{
    COM_SLAVE_MAP *pMap = (COM_SLAVE_MAP *)_map;

    if (pMap->BufferLength != sizeof(COM_SET_BLANKET_REQ)) {
        *exc = RPC_S_INVALID_TAG;
        *hr = E_ABORT;
        return;
    }

    COM_OBJECT *obj = (COM_OBJECT *)FindSlaveObject(
                                            pMap, ObjectsList, exc, hr);
    if (! obj)
        return;

    //
    //
    //

    COM_SET_BLANKET_REQ *buf = (COM_SET_BLANKET_REQ *)pMap->Buffer;

    WCHAR *pServerPrincName = buf->ServerPrincName;
    if (buf->DefaultServerPrincName)
        pServerPrincName = COLE_DEFAULT_PRINCIPAL;

    *hr = CoSetProxyBlanket(
            obj->pUnknown, buf->AuthnSvc, buf->AuthzSvc, pServerPrincName,
            buf->AuthnLevel, buf->ImpLevel, NULL, buf->Capabilities);

#ifdef DEBUG_COMSERVER
    WCHAR txt[256]; wsprintf(txt, L"(%04d) SetBlanketSlave objidx=%08X\n",
        List_Count(ObjectsList), obj->objidx); OutputDebugString(txt);
#endif
}


//---------------------------------------------------------------------------
// CopyProxySlave
//---------------------------------------------------------------------------


void ComServer::CopyProxySlave(void *_map, LIST *ObjectsList,
                               ULONG *exc, HRESULT *hr)
{
    COM_SLAVE_MAP *pMap = (COM_SLAVE_MAP *)_map;

    if (pMap->BufferLength != 0) {
        *exc = RPC_S_INVALID_TAG;
        *hr = E_ABORT;
        return;
    }

    COM_OBJECT *obj = (COM_OBJECT *)FindSlaveObject(
                                            pMap, ObjectsList, exc, hr);
    if (! obj)
        return;

    //
    //
    //

    IUnknown *pUnknown;
    *hr = CoCopyProxy(obj->pUnknown, &pUnknown);
    if (FAILED(*hr))
        return;

    GUID *guid = &obj->iid;

    //
    //
    //

    obj = (COM_OBJECT *)RefOrAllocSlaveObject(
                        pMap->idProcess, pUnknown, ObjectsList, exc, hr);
    if (! obj)
        return;

    if (memcmp(&obj->iid, &GUID_NULL, sizeof(GUID)) == 0) {

        obj->iid = *guid;

        *hr = SbieDll_ComCreateStub(
            *guid, pUnknown, (void **)&obj->pStub, (void **)&obj->pChannel);

        if (FAILED(*hr)) {
            DeleteSlaveObject(obj, ObjectsList);
            return;
        }

        obj->Flags |= FLAG_COPY_PROXY;
    }

#ifdef DEBUG_COMSERVER
    WCHAR txt[256]; wsprintf(txt, L"(%04d) CopyProxy objidx=%08X on objidx=%08X\n",
        List_Count(ObjectsList), obj->objidx, pMap->objidx); OutputDebugString(txt);
#endif

    pMap->objidx = obj->objidx;
}


//---------------------------------------------------------------------------
// ProcessNotifySlave
//---------------------------------------------------------------------------


void ComServer::ProcessNotifySlave(void *_map, LIST *ObjectsList,
                                   ULONG *exc, HRESULT *hr)
{
    COM_SLAVE_MAP *pMap = (COM_SLAVE_MAP *)_map;

#ifdef DEBUG_COMSERVER
    WCHAR txt[128]; wsprintf(txt, L"(%04d) ProcessNotifySlave      Before idProcess=%06d\n",
        List_Count(ObjectsList), pMap->idProcess); OutputDebugString(txt);
#endif

    if (pMap->BufferLength != 0) {
        *exc = RPC_S_INVALID_TAG;
        *hr = E_ABORT;
        return;
    }

    ULONG idProcess = pMap->idProcess;

    //
    // delete proxy copies first, then everything else
    //

    COM_OBJECT *obj, *next_obj;

    obj = (COM_OBJECT *)List_Head(ObjectsList);
    while (obj) {
        next_obj = (COM_OBJECT *)List_Next(obj);
        if ((obj->idProcess == idProcess) && (obj->Flags & FLAG_COPY_PROXY))
            DeleteSlaveObject(obj, ObjectsList);
        obj = next_obj;
    }

    obj = (COM_OBJECT *)List_Head(ObjectsList);
    while (obj) {
        next_obj = (COM_OBJECT *)List_Next(obj);
        if (obj->idProcess == idProcess)
            DeleteSlaveObject(obj, ObjectsList);
        obj = next_obj;
    }

#ifdef DEBUG_COMSERVER
    wsprintf(txt, L"(%04d) ProcessNotifySlave      After  idProcess=%06d\n",
        List_Count(ObjectsList), pMap->idProcess); OutputDebugString(txt);
#endif

    if (List_Count(ObjectsList) == 0 && m_AnySlaveObjectCreated) {
        //
        // if this slave does not host any objects now, but did create
        // some objects at some point, then mark it for termination now
        //
        pMap->ProcNum = -1;
    }
}


//---------------------------------------------------------------------------
// CheckWmiExecMethod
//---------------------------------------------------------------------------


BOOLEAN ComServer::CheckWmiExecMethod(UCHAR *BufPtr, ULONG BufLen)
{
    static const WCHAR *AllowedCombinedNames[] = {
        //
        // Win32 namespace
        //
        L"Win32_Directory::GetEffectivePermission",
        L"Win32_LogicalFileSecuritySetting::GetSecurityDescriptor",
        L"Win32_Process::GetOwner",
        //
        // StdRegProv namespace
        //
        L"StdRegProv::CheckAccess",
        L"StdRegProv::EnumKey",
        L"StdRegProv::EnumValues",
        L"StdRegProv::GetBinaryValue",
        L"StdRegProv::GetDWORDValue",
        L"StdRegProv::GetExpandedStringValue",
        L"StdRegProv::GetMultiStringValue",
        L"StdRegProv::GetStringValue",
        //
        // __SystemSecurity namespace
        //
        L"__SystemSecurity::GetSD",
        //
        // end of list
        //
        NULL
    };
    UCHAR *ptr;
    ULONG len;
    WCHAR *ClassName = NULL;
    WCHAR *MethodName = NULL;
    WCHAR *CombinedName = NULL;
    WCHAR *ptr2;
    ULONG i;
    BOOLEAN ok, logged;

    ok = FALSE;
    logged = FALSE;

    ptr = BufPtr;
    do {

        //
        // expect User....LLLL....ClassName..
        // where User is a constant, .... are bytes we don't care,
        // and LLLL is the length of the following WMI class name
        // (non-zero-terminated) and two more bytes we don't care
        //

        if (ptr - BufPtr + 16U > BufLen)
            break;
        if (memcmp(ptr, "User", 4) != 0)
            break;

        len = *(ULONG *)(ptr + 8);
        if (len & 1)
            break;
        ptr += 16;
        if (ptr + len < BufPtr) {
            // protect against a large len causing wraparound
            break;
        }
        if (ptr - BufPtr + len > BufLen)
            break;

        ClassName = (WCHAR *)HeapAlloc(m_heap, 0, len + 8);
        if (! ClassName)
            break;
        memcpy(ClassName, ptr, len);
        memzero(ClassName + len / sizeof(WCHAR), 4);

        ptr2 = wcschr(ClassName, L'.');
        if (ptr2)
            *ptr2 = L'\0';

        ptr += len;

        //
        // expect User....LLLL....MethodName
        // where User is a constant, .... are bytes we don't care,
        // and LLLL is the length of the following WMI method name
        // (non-zero-terminated)
        //
        // note that "User" may be aligned on a 32-bit boundary
        //

        if (ptr - BufPtr + 16U > BufLen)
            break;
        if (memcmp(ptr, "User", 4) != 0) {

            ptr += 2;
            if (ptr - BufPtr + 16U > BufLen)
                break;
            if (memcmp(ptr, "User", 4) != 0)
                break;
        }

        len = *(ULONG *)(ptr + 8);
        if (len & 1)
            break;
        ptr += 16;
        if (ptr + len < BufPtr) {
            // protect against a large len causing wraparound
            break;
        }
        if (ptr - BufPtr + len > BufLen)
            break;

        MethodName = (WCHAR *)HeapAlloc(m_heap, 0, len + 8);
        if (! MethodName)
            break;
        memcpy(MethodName, ptr, len);
        memzero(MethodName + len / sizeof(WCHAR), 4);

        //
        // combine class and method name and check if allowed
        //

        len = (wcslen(ClassName) + wcslen(MethodName) + 4) * sizeof(WCHAR);
        CombinedName = (WCHAR *)HeapAlloc(m_heap, 0, len);
        if (! CombinedName)
            break;
        wcscpy(CombinedName, ClassName);
        wcscat(CombinedName, L"::");
        wcscat(CombinedName, MethodName);

        for (i = 0; AllowedCombinedNames[i]; ++i)
            if (_wcsicmp(AllowedCombinedNames[i], CombinedName) == 0) {
                ok = TRUE;
                break;
            }

        if (! ok) {
            SbieApi_Log(2205, L"WMI %S", CombinedName);
            logged = TRUE;
        }

    } while (0);

    if ((! ok) && (! logged)) {
        SbieApi_Log(2205, L"WMI ExecMethod Error");
#ifdef DEBUG_COMSERVER
        while (! IsDebuggerPresent()) Sleep(500);
        __debugbreak();
#endif
    }

    if (CombinedName)
        HeapFree(m_heap, 0, CombinedName);
    if (MethodName)
        HeapFree(m_heap, 0, MethodName);
    if (ClassName)
        HeapFree(m_heap, 0, ClassName);

    return ok;
}


#include "comserver2.cpp"
