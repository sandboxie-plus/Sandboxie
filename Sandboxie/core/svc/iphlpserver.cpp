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
// IP Helper API Server -- using PipeServer
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "iphlpserver.h"
#include "iphlpwire.h"
#include "misc.h"
#include "core/dll/sbiedll.h"
#include "common/defines.h"


//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------


typedef HANDLE (*P_IcmpCreateFile)(void);

typedef BOOL (*P_IcmpCloseHandle)(HANDLE IcmpHandle);

typedef ULONG (*P_IcmpSendEcho2)(   HANDLE IcmpHandle,
                                    HANDLE Event,
                                    void *ApcRoutine, void *ApcContext,
                                    IPAddr DstAddr,
                                    void *RequestData, WORD RequestSize,
                                    PIP_OPTION_INFORMATION RequestOptions,
                                    void *ReplyBuffer, ULONG ReplySize,
                                    ULONG Timeout);

typedef ULONG (*P_IcmpSendEcho2Ex)( HANDLE IcmpHandle,
                                    HANDLE Event,
                                    void *ApcRoutine, void *ApcContext,
                                    IPAddr SrcAddr,
                                    IPAddr DstAddr,
                                    void *RequestData, WORD RequestSize,
                                    PIP_OPTION_INFORMATION RequestOptions,
                                    void *ReplyBuffer, ULONG ReplySize,
                                    ULONG Timeout);

typedef ULONG (*P_Icmp6SendEcho2)(  HANDLE IcmpHandle,
                                    HANDLE Event,
                                    void *ApcRoutine, void *ApcContext,
                                    void *SrcAdddr,
                                    void *DstAddr,
                                    void *RequestData, WORD RequestSize,
                                    PIP_OPTION_INFORMATION RequestOptions,
                                    void *ReplyBuffer, ULONG ReplySize,
                                    ULONG Timeout);


//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------


typedef struct _PROXY_ICMP_HANDLE {

    HANDLE handle;
    int ipver;

} PROXY_ICMP_HANDLE;


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


IpHlpServer::IpHlpServer(PipeServer *pipeServer)
{
    //
    // initialize proxy handle manager
    //

    m_ProxyHandle = new ProxyHandle(NULL, sizeof(PROXY_ICMP_HANDLE),
                                    CloseCallback, this);

    //
    // get API entry points
    //

    m_IcmpCreateFile  = NULL;
    m_Icmp6CreateFile = NULL;
    m_IcmpCloseHandle = NULL;

    HMODULE _iphlpapi = LoadLibrary(L"iphlpapi.dll");
    if (_iphlpapi) {

        m_IcmpCreateFile  = GetProcAddress(_iphlpapi, "IcmpCreateFile");
        m_Icmp6CreateFile = GetProcAddress(_iphlpapi, "Icmp6CreateFile");
        m_IcmpCloseHandle = GetProcAddress(_iphlpapi, "IcmpCloseHandle");

        m_IcmpSendEcho2   = GetProcAddress(_iphlpapi, "IcmpSendEcho2");
        m_IcmpSendEcho2Ex = GetProcAddress(_iphlpapi, "IcmpSendEcho2Ex");
        m_Icmp6SendEcho2  = GetProcAddress(_iphlpapi, "Icmp6SendEcho2");
    }

    //
    // install pipe server target
    //

    pipeServer->Register(MSGID_IPHLP, this, Handler);
}


//---------------------------------------------------------------------------
// CloseCallback
//---------------------------------------------------------------------------


void IpHlpServer::CloseCallback(void *context, void *data)
{
    PROXY_ICMP_HANDLE *ProxyIcmp = (PROXY_ICMP_HANDLE *)data;
    void *p_IcmpCloseHandle = ((IpHlpServer *)context)->m_IcmpCloseHandle;
    ((P_IcmpCloseHandle)p_IcmpCloseHandle)(ProxyIcmp->handle);
}


//---------------------------------------------------------------------------
// Handler
//---------------------------------------------------------------------------


MSG_HEADER *IpHlpServer::Handler(void *_this, MSG_HEADER *msg)
{
    IpHlpServer *pThis = (IpHlpServer *)_this;

    HANDLE idProcess = (HANDLE)(ULONG_PTR)PipeServer::GetCallerProcessId();

    if (msg->msgid == MSGID_IPHLP_NOTIFICATION) {
        pThis->NotifyHandler(idProcess);
        return NULL;
    }

    if (0 != SbieApi_QueryProcess(idProcess, NULL, NULL, NULL, NULL))
        return SHORT_REPLY(ERROR_ACCESS_DENIED);

    if (PipeServer::ImpersonateCaller(&msg) != 0)
        return msg;

    if (msg->msgid == MSGID_IPHLP_CREATE_FILE)
        return pThis->CreateHandler(msg, idProcess);

    if (msg->msgid == MSGID_IPHLP_CLOSE_HANDLE)
        return pThis->CloseHandler(msg, idProcess);

    if (msg->msgid == MSGID_IPHLP_SEND_ECHO)
        return pThis->SendEchoHandler(msg, idProcess);

    return NULL;
}


//---------------------------------------------------------------------------
// CreateHandler
//---------------------------------------------------------------------------


MSG_HEADER *IpHlpServer::CreateHandler(MSG_HEADER *msg, HANDLE idProcess)
{
    IPHLP_CREATE_FILE_REQ *req = (IPHLP_CREATE_FILE_REQ *)msg;
    if (req->h.length < sizeof(IPHLP_CREATE_FILE_REQ))
        return SHORT_REPLY(ERROR_INVALID_PARAMETER);

    void *p_IcmpCreateFile = m_IcmpCreateFile;
    if (req->ip6)
        p_IcmpCreateFile = m_Icmp6CreateFile;
    if ((! p_IcmpCreateFile) || (! m_IcmpCloseHandle))
        return SHORT_REPLY(ERROR_NOT_SUPPORTED);

    WCHAR boxname[BOXNAME_COUNT];
    WCHAR exename[99];
    if (!NT_SUCCESS(SbieApi_QueryProcess(idProcess, boxname, exename, NULL, NULL)))
        return SHORT_REPLY(E_FAIL);

    if (!SbieDll_GetSettingsForName_bool(boxname, exename, L"AllowNetworkAccess", TRUE))
        return SHORT_REPLY(ERROR_ACCESS_DENIED);

    if (0 != SbieApi_CheckInternetAccess(
                                idProcess, req->ip6 ? L"Ip6" : L"Ip", TRUE))
        return SHORT_REPLY(ERROR_ACCESS_DENIED);

    const ULONG rpl_len = sizeof(IPHLP_CREATE_FILE_RPL);
    IPHLP_CREATE_FILE_RPL *rpl =
        (IPHLP_CREATE_FILE_RPL *)LONG_REPLY(rpl_len);

    if (rpl) {

        PROXY_ICMP_HANDLE ProxyIcmp;

        if (RestrictToken()) {

            ProxyIcmp.handle = ((P_IcmpCreateFile)p_IcmpCreateFile)();

        } else {

            ProxyIcmp.handle = INVALID_HANDLE_VALUE;
            SetLastError(ERROR_ACCESS_DENIED);
        }

        if (ProxyIcmp.handle == INVALID_HANDLE_VALUE) {

            rpl->h.status = GetLastError();
            rpl->handle = 0;

        } else {

            ProxyIcmp.ipver = req->ip6 ? 6 : 4;

            rpl->handle = m_ProxyHandle->Create(idProcess, &ProxyIcmp);

            if (rpl->handle)
                rpl->h.status = ERROR_SUCCESS;
            else // CloseCallback was already invoked
                rpl->h.status = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// CloseHandler
//---------------------------------------------------------------------------


MSG_HEADER *IpHlpServer::CloseHandler(MSG_HEADER *msg, HANDLE idProcess)
{
    IPHLP_CLOSE_HANDLE_REQ *req = (IPHLP_CLOSE_HANDLE_REQ *)msg;
    if (req->h.length < sizeof(IPHLP_CLOSE_HANDLE_REQ))
        return SHORT_REPLY(ERROR_INVALID_PARAMETER);

    ULONG status = 0;

    PROXY_ICMP_HANDLE *ProxyIcmp =
        (PROXY_ICMP_HANDLE *)m_ProxyHandle->Find(idProcess, req->handle);

    if (ProxyIcmp)
        m_ProxyHandle->Close(ProxyIcmp);
    else
        status = STATUS_INVALID_HANDLE;

    return SHORT_REPLY(status);
}


//---------------------------------------------------------------------------
// SendEchoHandler
//---------------------------------------------------------------------------


MSG_HEADER *IpHlpServer::SendEchoHandler(MSG_HEADER *msg, HANDLE idProcess)
{
    //
    // validate request packet
    //

    IPHLP_SEND_ECHO_REQ *req = (IPHLP_SEND_ECHO_REQ *)msg;
    if (req->h.length < sizeof(IPHLP_SEND_ECHO_REQ))
        return SHORT_REPLY(ERROR_INVALID_PARAMETER);
    if ((req->request_size > 0xFFFF) || (req->reply_size > 0x0FFFFF))
        return SHORT_REPLY(ERROR_INVALID_PARAMETER);

    PROXY_ICMP_HANDLE *ProxyIcmp =
        (PROXY_ICMP_HANDLE *)m_ProxyHandle->Find(idProcess, req->handle);
    if (! ProxyIcmp)
        return SHORT_REPLY(ERROR_INVALID_HANDLE);

    int ipver = req->ip6 ? 6 : 4;
    if (ipver != ProxyIcmp->ipver) {
        m_ProxyHandle->Release(ProxyIcmp);
        return SHORT_REPLY(ERROR_INVALID_HANDLE);
    }

    void *p_IcmpSendEcho = m_IcmpSendEcho2;
    if (req->ip6)
        p_IcmpSendEcho = m_Icmp6SendEcho2;
    else if (req->ex2)
        p_IcmpSendEcho = m_IcmpSendEcho2Ex;
    if (! p_IcmpSendEcho) {
        m_ProxyHandle->Release(ProxyIcmp);
        return SHORT_REPLY(ERROR_NOT_SUPPORTED);
    }

    //
    // issue echo request
    //

    IP_OPTION_INFORMATION RequestOptions, *pRequestOptions;
    if (req->ipopt_valid) {
        memzero(&RequestOptions, sizeof(IP_OPTION_INFORMATION));
        RequestOptions.Ttl   = req->ipopt_ttl;
        RequestOptions.Tos   = req->ipopt_tos;
        RequestOptions.Flags = req->ipopt_flags;
        pRequestOptions = &RequestOptions;
    } else
        pRequestOptions = NULL;

    ULONG reply_size = req->reply_size;
#ifdef _WIN64
    if (req->iswow64 && reply_size == 0x1C + req->request_size) {
        // 32-bit caller specified a minimal reply buffer which has room
        // for the 32-bit ICMP_ECHO_REPLY (0x1C bytes) plus request bytes.
        // we need to make room for a 64-bit ICMP_ECHO_REPLY
        reply_size = 0x28 + req->request_size;
    }
#endif _WIN64

    ULONG rpl_len = sizeof(IPHLP_SEND_ECHO_RPL) + reply_size;
    IPHLP_SEND_ECHO_RPL *rpl =
        (IPHLP_SEND_ECHO_RPL *)LONG_REPLY(rpl_len);
    if (rpl) {

        memzero(rpl->reply_data, reply_size);

        WORD RequestSize = (WORD)req->request_size;

        ULONG num_replies = 0;

        if (req->ip6) {

            num_replies = ((P_Icmp6SendEcho2)p_IcmpSendEcho)(
                ProxyIcmp->handle, NULL, NULL, NULL,
                req->src_addr, req->dst_addr,
                req->request_data, RequestSize, pRequestOptions,
                rpl->reply_data, reply_size, req->timeout);

        } else if (req->ex2) {

            num_replies = ((P_IcmpSendEcho2Ex)p_IcmpSendEcho)(
                ProxyIcmp->handle, NULL, NULL, NULL,
                *(ULONG *)&req->src_addr, *(ULONG *)&req->dst_addr,
                req->request_data, RequestSize, pRequestOptions,
                rpl->reply_data, reply_size, req->timeout);

        } else {

            num_replies = ((P_IcmpSendEcho2)p_IcmpSendEcho)(
                ProxyIcmp->handle, NULL, NULL, NULL,
                *(ULONG *)&req->dst_addr,
                req->request_data, RequestSize, pRequestOptions,
                rpl->reply_data, reply_size, req->timeout);
        }

        if (num_replies == 0) {
            rpl->h.status = GetLastError();
            num_replies = 1; // even on error we need to return one valid result buffer
        } else
            rpl->h.status = ERROR_SUCCESS;

        //
        // for an IPv4 reply buffer, we need to adjust the pointers.
        // for a 32-bit caller, we also need to convert 64-bit reply
        // structures to 32-bit structures
        //

        if ((! req->ip6) && num_replies) {

            ULONG i;

#ifdef _WIN64

            if (req->iswow64) {

                ICMP_ECHO_REPLY32 *rpl32 = (ICMP_ECHO_REPLY32 *)HeapAlloc(
                    GetProcessHeap(), HEAP_ZERO_MEMORY, reply_size);
                if (! rpl32)
                    rpl->h.status = ERROR_NOT_ENOUGH_MEMORY;
                else {

                    UCHAR *data = (UCHAR *)(rpl32 + num_replies);

                    ICMP_ECHO_REPLY *replies =
                        (ICMP_ECHO_REPLY *)rpl->reply_data;

                    for (i = 0; i < num_replies; ++i) {
                        ICMP_ECHO_REPLY *reply = &replies[i];

                        ULONG_PTR offset = data - (UCHAR *)rpl32;
                        if (offset + reply->DataSize > req->reply_size) {
                            rpl->h.status = IP_BUF_TOO_SMALL;
                            num_replies = 0;
                            reply_size = 0;
                            break;
                        }
                        memcpy(data, reply->Data, reply->DataSize);
                        data += reply->DataSize;

                        rpl32[i].Address          = reply->Address;
                        rpl32[i].Status           = reply->Status;
                        rpl32[i].RoundTripTime    = reply->RoundTripTime;
                        rpl32[i].DataSize         = reply->DataSize;
                        rpl32[i].Data             = (void *POINTER_32)offset;
                        rpl32[i].Options.Ttl      = reply->Options.Ttl;
                        rpl32[i].Options.Tos      = reply->Options.Tos;
                        rpl32[i].Options.Flags    = reply->Options.Flags;
                    }

                    if (num_replies) {
                        reply_size = (ULONG)((UCHAR *)data - (UCHAR *)rpl32);
                        memcpy(rpl->reply_data, rpl32, reply_size);
                    }

                    HeapFree(GetProcessHeap(), 0, rpl32);
                }

            } else {

#endif _WIN64

                ICMP_ECHO_REPLY *replies =
                    (ICMP_ECHO_REPLY *)rpl->reply_data;

                for (i = 0; i < num_replies; ++i) {
                    ICMP_ECHO_REPLY *reply = &replies[i];

                    reply->Options.OptionsSize = 0;
                    reply->Options.OptionsData = 0;
                    reply->Data = (void *)
                        ((ULONG_PTR)reply->Data - (ULONG_PTR)replies);
                }

#ifdef _WIN64
            }
#endif _WIN64
        }

        rpl->num_replies = num_replies;
        rpl->reply_size = reply_size;
    }

    m_ProxyHandle->Release(ProxyIcmp);
    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// NotifyHandler
//---------------------------------------------------------------------------


void IpHlpServer::NotifyHandler(HANDLE idProcess)
{
    m_ProxyHandle->ReleaseProcess(idProcess);
}
