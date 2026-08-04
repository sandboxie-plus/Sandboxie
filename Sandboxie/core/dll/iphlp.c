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
// IP Helper API
//---------------------------------------------------------------------------

#include "dll.h"

#include <windows.h>
#include "core/svc/IpHlpWire.h"
#include "dll.h"
#include "common/my_wsa.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static ULONG_PTR IpHlp_IcmpCreateFile(void);
static ULONG_PTR IpHlp_Icmp6CreateFile(void);
static ULONG_PTR IpHlp_CommonCreate(BOOLEAN ip6);

static BOOL IpHlp_IcmpCloseHandle(ULONG_PTR IcmpHandle);

static ULONG IpHlp_IcmpSendEcho(    ULONG_PTR IcmpHandle,
                                    IPAddr DstAddr,
                                    void *RequestData, WORD RequestSize,
                                    PIP_OPTION_INFORMATION RequestOptions,
                                    void *ReplyBuffer, ULONG ReplySize,
                                    ULONG Timeout);

static ULONG IpHlp_IcmpSendEcho2(   ULONG_PTR IcmpHandle,
                                    HANDLE Event,
                                    void *ApcRoutine, void *ApcContext,
                                    IPAddr DstAddr,
                                    void *RequestData, WORD RequestSize,
                                    PIP_OPTION_INFORMATION RequestOptions,
                                    void *ReplyBuffer, ULONG ReplySize,
                                    ULONG Timeout);

static ULONG IpHlp_IcmpSendEcho2Ex( ULONG_PTR IcmpHandle,
                                    HANDLE Event,
                                    void *ApcRoutine, void *ApcContext,
                                    IPAddr SrcAddr,
                                    IPAddr DstAddr,
                                    void *RequestData, WORD RequestSize,
                                    PIP_OPTION_INFORMATION RequestOptions,
                                    void *ReplyBuffer, ULONG ReplySize,
                                    ULONG Timeout);

static ULONG IpHlp_Icmp6SendEcho2(  ULONG_PTR IcmpHandle,
                                    HANDLE Event,
                                    void *ApcRoutine, void *ApcContext,
                                    struct sockaddr_in6 *SrcAdddr,
                                    struct sockaddr_in6 *DstAddr,
                                    void *RequestData, WORD RequestSize,
                                    PIP_OPTION_INFORMATION RequestOptions,
                                    void *ReplyBuffer, ULONG ReplySize,
                                    ULONG Timeout);

static ULONG IpHlp_CommonSend(      ULONG_PTR IcmpHandle,
                                    HANDLE Event,
                                    void *ApcRoutine, void *ApcContext,
                                    ULONG_PTR SrcAddr, ULONG_PTR DstAddr,
                                    void *RequestData, WORD RequestSize,
                                    PIP_OPTION_INFORMATION RequestOptions,
                                    void *ReplyBuffer, ULONG ReplySize,
                                    ULONG Timeout,
                                    BOOLEAN ip6, BOOLEAN ex2);

static ULONG IpHlp_CommonSendError(ULONG ErrorLevel, ULONG ErrorCode);

static ULONG IpHlp_NotifyRouteChange2(ADDRESS_FAMILY Family,
                                      PIPFORWARD_CHANGE_CALLBACK Callback,
                                      void *CallerContext,
                                      BOOLEAN InitialNotification,
                                      HANDLE *NotificationHandle);

static ULONG IpHlp_CancelMibChangeNotify2(HANDLE NotificationHandle);


//---------------------------------------------------------------------------


typedef ULONG_PTR (*P_IcmpCreateFile)(void);

#define P_Icmp6CreateFile P_IcmpCreateFile

typedef BOOL (*P_IcmpCloseHandle)(ULONG_PTR IcmpHandle);

typedef ULONG (*P_IcmpSendEcho)(    ULONG_PTR IcmpHandle,
                                    IPAddr DstAddr,
                                    void *RequestData, WORD RequestSize,
                                    PIP_OPTION_INFORMATION RequestOptions,
                                    void *ReplyBuffer, ULONG ReplySize,
                                    ULONG Timeout);

typedef ULONG (*P_IcmpSendEcho2)(   ULONG_PTR IcmpHandle,
                                    HANDLE Event,
                                    void *ApcRoutine, void *ApcContext,
                                    IPAddr DstAddr,
                                    void *RequestData, WORD RequestSize,
                                    PIP_OPTION_INFORMATION RequestOptions,
                                    void *ReplyBuffer, ULONG ReplySize,
                                    ULONG Timeout);

typedef ULONG (*P_IcmpSendEcho2Ex)( ULONG_PTR IcmpHandle,
                                    HANDLE Event,
                                    void *ApcRoutine, void *ApcContext,
                                    IPAddr SrcAddr,
                                    IPAddr DstAddr,
                                    void *RequestData, WORD RequestSize,
                                    PIP_OPTION_INFORMATION RequestOptions,
                                    void *ReplyBuffer, ULONG ReplySize,
                                    ULONG Timeout);

typedef ULONG (*P_Icmp6SendEcho2)(  ULONG_PTR IcmpHandle,
                                    HANDLE Event,
                                    void *ApcRoutine, void *ApcContext,
                                    struct sockaddr_in6 *SrcAdddr,
                                    struct sockaddr_in6 *DstAddr,
                                    void *RequestData, WORD RequestSize,
                                    PIP_OPTION_INFORMATION RequestOptions,
                                    void *ReplyBuffer, ULONG ReplySize,
                                    ULONG Timeout);

typedef ULONG (*P_NotifyRouteChange2)(ADDRESS_FAMILY Family,
                                      PIPFORWARD_CHANGE_CALLBACK Callback,
                                      void *CallerContext,
                                      BOOLEAN InitialNotification,
                                      HANDLE *NotificationHandle);

typedef ULONG (*P_CancelMibChangeNotify2)(HANDLE NotificationHandle);


//---------------------------------------------------------------------------


static P_IcmpCreateFile         __sys_IcmpCreateFile            = NULL;
static P_Icmp6CreateFile        __sys_Icmp6CreateFile           = NULL;
static P_IcmpCloseHandle        __sys_IcmpCloseHandle           = NULL;

static P_IcmpSendEcho           __sys_IcmpSendEcho              = NULL;
static P_IcmpSendEcho2          __sys_IcmpSendEcho2             = NULL;
static P_IcmpSendEcho2Ex        __sys_IcmpSendEcho2Ex           = NULL;
static P_Icmp6SendEcho2         __sys_Icmp6SendEcho2            = NULL;

static P_NotifyRouteChange2     __sys_NotifyRouteChange2        = NULL;
static P_CancelMibChangeNotify2 __sys_CancelMibChangeNotify2    = NULL;


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static HANDLE IpHlp_DummyEvent = NULL;


//---------------------------------------------------------------------------
// IpHlp_Init
//---------------------------------------------------------------------------


_FX BOOLEAN IpHlp_Init(HMODULE module)
{
    void *IcmpCreateFile;
    void *Icmp6CreateFile;
    void *IcmpCloseHandle;
    void *IcmpSendEcho;
    void *IcmpSendEcho2;
    void *IcmpSendEcho2Ex;
    void *Icmp6SendEcho2;
    void *NotifyRouteChange2;
    void *CancelMibChangeNotify2;

    if (Dll_CompartmentMode || Dll_OsBuild < 6000) { // in compartment mode we have a full token so no need to hook anything here

        //
        // earlier than Windows Vista, don't hook
        //

        return TRUE;
    }

    IcmpCreateFile          = GetProcAddress(module, "IcmpCreateFile");
    Icmp6CreateFile         = GetProcAddress(module, "Icmp6CreateFile");
    IcmpCloseHandle         = GetProcAddress(module, "IcmpCloseHandle");

    IcmpSendEcho            = GetProcAddress(module, "IcmpSendEcho");
    IcmpSendEcho2           = GetProcAddress(module, "IcmpSendEcho2");
    IcmpSendEcho2Ex         = GetProcAddress(module, "IcmpSendEcho2Ex");
    Icmp6SendEcho2          = GetProcAddress(module, "Icmp6SendEcho2");

    NotifyRouteChange2      = GetProcAddress(module, "NotifyRouteChange2");
    CancelMibChangeNotify2  =
                        GetProcAddress(module, "CancelMibChangeNotify2");

    SBIEDLL_HOOK(IpHlp_,IcmpCreateFile);
    SBIEDLL_HOOK(IpHlp_,Icmp6CreateFile);
    SBIEDLL_HOOK(IpHlp_,IcmpCloseHandle);

    SBIEDLL_HOOK(IpHlp_,IcmpSendEcho);
    SBIEDLL_HOOK(IpHlp_,IcmpSendEcho2);
    SBIEDLL_HOOK(IpHlp_,Icmp6SendEcho2);
    if (IcmpSendEcho2Ex) {
        SBIEDLL_HOOK(IpHlp_,IcmpSendEcho2Ex);
    }

    if (NotifyRouteChange2) {
        SBIEDLL_HOOK(IpHlp_,NotifyRouteChange2);
    }
    if (CancelMibChangeNotify2) {
        SBIEDLL_HOOK(IpHlp_,CancelMibChangeNotify2);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// IpHlp_IcmpCreateFile
//---------------------------------------------------------------------------


_FX ULONG_PTR IpHlp_IcmpCreateFile(void)
{
    return IpHlp_CommonCreate(FALSE);
}


//---------------------------------------------------------------------------
// IpHlp_Icmp6CreateFile
//---------------------------------------------------------------------------


_FX ULONG_PTR IpHlp_Icmp6CreateFile(void)
{
    return IpHlp_CommonCreate(TRUE);
}


//---------------------------------------------------------------------------
// IpHlp_CommonCreate
//---------------------------------------------------------------------------


_FX ULONG_PTR IpHlp_CommonCreate(BOOLEAN ip6)
{
    IPHLP_CREATE_FILE_REQ req;
    IPHLP_CREATE_FILE_RPL *rpl;
    ULONG error;
    ULONG_PTR handle;

    req.h.length = sizeof(IPHLP_CREATE_FILE_REQ);
    req.h.msgid = MSGID_IPHLP_CREATE_FILE;
    req.ip6 = ip6;

    handle = (ULONG_PTR)INVALID_HANDLE_VALUE;
    rpl = (IPHLP_CREATE_FILE_RPL *)SbieDll_CallServer(&req.h);
    if (! rpl)
        error = RPC_S_SERVER_UNAVAILABLE;
    else {
        error = rpl->h.status;
        if (! error)
            handle = (ULONG_PTR)rpl->handle;
        Dll_Free(rpl);
    }

    SetLastError(error);
    return handle;
}


//---------------------------------------------------------------------------
// IpHlp_IcmpCloseHandle
//---------------------------------------------------------------------------


_FX BOOL IpHlp_IcmpCloseHandle(ULONG_PTR IcmpHandle)
{
    IPHLP_CLOSE_HANDLE_REQ req;
    MSG_HEADER *rpl;
    ULONG error;

    req.h.length = sizeof(IPHLP_CLOSE_HANDLE_REQ);
    req.h.msgid = MSGID_IPHLP_CLOSE_HANDLE;
    req.handle = (ULONG)IcmpHandle;

    rpl = SbieDll_CallServer(&req.h);
    if (! rpl)
        error = RPC_S_SERVER_UNAVAILABLE;
    else {
        error = rpl->status;
        Dll_Free(rpl);
    }

    SetLastError(error);
    return (error == 0 ? TRUE : FALSE);
}


//---------------------------------------------------------------------------
// IpHlp_IcmpSendEcho
//---------------------------------------------------------------------------


_FX ULONG IpHlp_IcmpSendEcho(       ULONG_PTR IcmpHandle,
                                    IPAddr DstAddr,
                                    void *RequestData, WORD RequestSize,
                                    PIP_OPTION_INFORMATION RequestOptions,
                                    void *ReplyBuffer, ULONG ReplySize,
                                    ULONG Timeout)
{
    return IpHlp_CommonSend(    IcmpHandle, NULL, NULL, NULL,
                                0, DstAddr,
                                RequestData, RequestSize, RequestOptions,
                                ReplyBuffer, ReplySize, Timeout,
                                FALSE, FALSE);
}


//---------------------------------------------------------------------------
// IpHlp_IcmpSendEcho2
//---------------------------------------------------------------------------


_FX ULONG IpHlp_IcmpSendEcho2(      ULONG_PTR IcmpHandle,
                                    HANDLE Event,
                                    void *ApcRoutine, void *ApcContext,
                                    IPAddr DstAddr,
                                    void *RequestData, WORD RequestSize,
                                    PIP_OPTION_INFORMATION RequestOptions,
                                    void *ReplyBuffer, ULONG ReplySize,
                                    ULONG Timeout)
{
    return IpHlp_CommonSend(    IcmpHandle, Event, ApcRoutine, ApcContext,
                                0, DstAddr,
                                RequestData, RequestSize, RequestOptions,
                                ReplyBuffer, ReplySize, Timeout,
                                FALSE, FALSE);
}


//---------------------------------------------------------------------------
// IpHlp_IcmpSendEcho2Ex
//---------------------------------------------------------------------------


_FX ULONG IpHlp_IcmpSendEcho2Ex(    ULONG_PTR IcmpHandle,
                                    HANDLE Event,
                                    void *ApcRoutine, void *ApcContext,
                                    IPAddr SrcAddr, IPAddr DstAddr,
                                    void *RequestData, WORD RequestSize,
                                    PIP_OPTION_INFORMATION RequestOptions,
                                    void *ReplyBuffer, ULONG ReplySize,
                                    ULONG Timeout)
{
    return IpHlp_CommonSend(    IcmpHandle, Event, ApcRoutine, ApcContext,
                                SrcAddr, DstAddr,
                                RequestData, RequestSize, RequestOptions,
                                ReplyBuffer, ReplySize, Timeout,
                                FALSE, TRUE);
}


//---------------------------------------------------------------------------
// IpHlp_Icmp6SendEcho2
//---------------------------------------------------------------------------


_FX ULONG IpHlp_Icmp6SendEcho2(     ULONG_PTR IcmpHandle,
                                    HANDLE Event,
                                    void *ApcRoutine, void *ApcContext,
                                    struct sockaddr_in6 *SrcAddr,
                                    struct sockaddr_in6 *DstAddr,
                                    void *RequestData, WORD RequestSize,
                                    PIP_OPTION_INFORMATION RequestOptions,
                                    void *ReplyBuffer, ULONG ReplySize,
                                    ULONG Timeout)
{
    return IpHlp_CommonSend(    IcmpHandle, Event, ApcRoutine, ApcContext,
                                (ULONG_PTR)SrcAddr, (ULONG_PTR)DstAddr,
                                RequestData, RequestSize, RequestOptions,
                                ReplyBuffer, ReplySize, Timeout,
                                TRUE, FALSE);
}


//---------------------------------------------------------------------------
// IpHlp_CommonSend
//---------------------------------------------------------------------------


_FX ULONG IpHlp_CommonSend(         ULONG_PTR IcmpHandle,
                                    HANDLE Event,
                                    void *ApcRoutine, void *ApcContext,
                                    ULONG_PTR SrcAddr, ULONG_PTR DstAddr,
                                    void *RequestData, WORD RequestSize,
                                    PIP_OPTION_INFORMATION RequestOptions,
                                    void *ReplyBuffer, ULONG ReplySize,
                                    ULONG Timeout,
                                    BOOLEAN ip6, BOOLEAN ex2)
{
    IPHLP_SEND_ECHO_REQ *req;
    IPHLP_SEND_ECHO_RPL *rpl;
    ULONG len;
    ULONG error;

    //
    // abort if the request specifies options we don't support
    //

    if (ApcRoutine)
        return IpHlp_CommonSendError(1, ERROR_NOT_SUPPORTED);

    if (RequestOptions) {

        if (RequestOptions->OptionsSize)
            return IpHlp_CommonSendError(2, ERROR_NOT_SUPPORTED);
    }

    //
    // build request
    //

    len = sizeof(IPHLP_SEND_ECHO_REQ) + RequestSize;
    req = Dll_Alloc(len);
    memzero(req, sizeof(IPHLP_SEND_ECHO_REQ));

    req->h.length = len;
    req->h.msgid = MSGID_IPHLP_SEND_ECHO;

#ifndef _WIN64
    req->iswow64 = Dll_IsWow64;
#endif

    req->ip6 = ip6;
    req->ex2 = ex2;

    req->handle = (ULONG)IcmpHandle;

    if (ip6) {
        memcpy(req->src_addr, (void *)SrcAddr, sizeof(struct sockaddr_in6));
        memcpy(req->dst_addr, (void *)DstAddr, sizeof(struct sockaddr_in6));
    } else {
        *(ULONG *)&req->src_addr = (ULONG)SrcAddr;
        *(ULONG *)&req->dst_addr = (ULONG)DstAddr;
    }

    if (RequestOptions) {

        req->ipopt_valid = TRUE;
        req->ipopt_ttl   = RequestOptions->Ttl;
        req->ipopt_tos   = RequestOptions->Tos;
        req->ipopt_flags = RequestOptions->Flags;
    }

    req->timeout = Timeout;

    req->reply_size = ReplySize;
    req->request_size = RequestSize;
    memcpy(&req->request_data, RequestData, req->request_size);

    //
    // issue request
    //

    rpl = (IPHLP_SEND_ECHO_RPL *)SbieDll_CallServer(&req->h);

    Dll_Free(req);

    if (! rpl) {
        SetLastError(RPC_S_SERVER_UNAVAILABLE);
        return 0;
    }

    //
    // copy reply
    //

    error = rpl->h.status;

    len = rpl->reply_size;
    if (len > ReplySize)
        len = ReplySize;
    memcpy(ReplyBuffer, rpl->reply_data, len);

    len = rpl->num_replies;

    Dll_Free(rpl);

    //
    // for an IPv4 reply buffer, we need to adjust the pointers
    //

    if (! ip6) {

        ULONG i;
        ICMP_ECHO_REPLY *replies = (ICMP_ECHO_REPLY *)ReplyBuffer;
        for (i = 0; i < len; ++i) {
            ICMP_ECHO_REPLY *reply = &replies[i];
            ULONG offset = (ULONG)(ULONG_PTR)reply->Data;
            reply->Data = (void *)((ULONG_PTR)replies + offset);
        }
    }

    //
    // finish
    //
    // for asynchronous requests, ReplyBuffer->Reserved is expected
    // to contain the number of reply packets (undocumented)
    //

    if ((! error) && Event) {
        ((ICMP_ECHO_REPLY *)ReplyBuffer)->Reserved = (USHORT)len;
        error = ERROR_IO_PENDING;
        SetEvent(Event);
    }
	
    if (error != ERROR_SUCCESS)
        len = 0;

    SetLastError(error);
    return len;
}


//---------------------------------------------------------------------------
// IpHlp_CommonSendError
//---------------------------------------------------------------------------


_FX ULONG IpHlp_CommonSendError(ULONG ErrorLevel, ULONG ErrorCode)
{
    SbieApi_Log(2205, L"IcmpSendEcho (%d)", ErrorLevel);
    SetLastError(ErrorCode);
    return 0;
}


//---------------------------------------------------------------------------
// IpHlp_NotifyRouteChange2
//---------------------------------------------------------------------------


_FX ULONG IpHlp_NotifyRouteChange2(ADDRESS_FAMILY Family,
                                   PIPFORWARD_CHANGE_CALLBACK Callback,
                                   void *CallerContext,
                                   BOOLEAN InitialNotification,
                                   HANDLE *NotificationHandle)
{
    if (! IpHlp_DummyEvent)
        IpHlp_DummyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    *NotificationHandle = IpHlp_DummyEvent;

    if (InitialNotification && Callback)
        Callback(CallerContext, NULL, 3 /* MibInitialNotification */);

    return 0;
}


//---------------------------------------------------------------------------
// IpHlp_CancelMibChangeNotify2
//---------------------------------------------------------------------------


_FX ULONG IpHlp_CancelMibChangeNotify2(HANDLE NotificationHandle)
{
    ULONG rv;
    if (NotificationHandle == IpHlp_DummyEvent)
        rv = 0;
    else
        rv = __sys_CancelMibChangeNotify2(NotificationHandle);
    return rv;
}
