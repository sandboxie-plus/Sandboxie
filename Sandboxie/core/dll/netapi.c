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
// Net Api
//---------------------------------------------------------------------------


#if 0


#include <windows.h>
#include <wincrypt.h>
#include "core/svc/NetApiWire.h"
#include "dll.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static ULONG NetApi_NetWkstaGetInfo(
    const WCHAR *servername, ULONG level, BYTE **buf_ptr);

static ULONG NetApi_NetServerGetInfo(
    const WCHAR *servername, ULONG level, BYTE **buf_ptr);


//---------------------------------------------------------------------------


typedef ULONG (*P_NetWkstaGetInfo)(
    const WCHAR *servername, ULONG level, BYTE **buf_ptr);

typedef ULONG (*P_NetServerGetInfo)(
    const WCHAR *servername, ULONG level, BYTE **buf_ptr);

typedef ULONG (*P_NetApiBufferAllocate)(ULONG len, BYTE **buf_ptr);


//---------------------------------------------------------------------------


static P_NetWkstaGetInfo        __sys_NetWkstaGetInfo       = NULL;
static P_NetServerGetInfo       __sys_NetServerGetInfo      = NULL;
static P_NetApiBufferAllocate   __sys_NetApiBufferAllocate  = NULL;


//---------------------------------------------------------------------------
// NetApi_Init
//---------------------------------------------------------------------------


ALIGNED BOOLEAN NetApi_Init(HMODULE module)
{
    void *NetWkstaGetInfo;
    void *NetServerGetInfo;

    if (! module)
        return TRUE;

    __sys_NetApiBufferAllocate = (P_NetApiBufferAllocate)
        GetProcAddress(module, "NetApiBufferAllocate");
    if (! __sys_NetApiBufferAllocate)
        return FALSE;

    //
    // hook NetApi services
    //

    NetWkstaGetInfo  = GetProcAddress(module, "NetWkstaGetInfo");
    NetServerGetInfo = GetProcAddress(module, "NetServerGetInfo");

    SBIEDLL_HOOK(NetApi_,NetWkstaGetInfo);
    SBIEDLL_HOOK(NetApi_,NetServerGetInfo);

    return TRUE;
}


//---------------------------------------------------------------------------
// NetApi_NetWkstaGetInfo
//---------------------------------------------------------------------------


ALIGNED ULONG NetApi_NetWkstaGetInfo(
    const WCHAR *servername, ULONG level, BYTE **buf_ptr)
{
    NETAPI_WKSTA_GET_INFO_REQ *req;
    NETAPI_WKSTA_GET_INFO_RPL *rpl;
    ULONG name_len;
    ULONG req_len;
    ULONG status;

    req_len = sizeof(NETAPI_WKSTA_GET_INFO_REQ);
    if (servername) {
        name_len = (wcslen(servername) + 1) * sizeof(WCHAR);
        req_len += name_len;
    } else
        name_len = -1;

    req = (NETAPI_WKSTA_GET_INFO_REQ *)Dll_Alloc(req_len);
    req->h.length = req_len;
    req->h.msgid = MSGID_NETAPI_WKSTA_GET_INFO;
    req->level = level;
    req->name_len = name_len;
    if (servername)
        wcscpy(req->name, servername);

    rpl = (NETAPI_WKSTA_GET_INFO_RPL *)
                SbieDll_CallServer((MSG_HEADER *)req);
    Dll_Free(req);

    if (! rpl)
        return ERROR_NOT_ENOUGH_MEMORY;

    status = rpl->h.status;
    if (status == 0)
        status = __sys_NetApiBufferAllocate(rpl->data_len, buf_ptr);
    if (status == 0)
        memcpy(*buf_ptr, rpl->data, rpl->data_len);

    Dll_Free(rpl);

    return status;
}


//---------------------------------------------------------------------------
// NetApi_NetServerGetInfo
//---------------------------------------------------------------------------


ALIGNED ULONG NetApi_NetServerGetInfo(
    const WCHAR *servername, ULONG level, BYTE **buf_ptr)
{
    NETAPI_SERVER_GET_INFO_REQ *req;
    NETAPI_SERVER_GET_INFO_RPL *rpl;
    ULONG name_len;
    ULONG req_len;
    ULONG status;

    req_len = sizeof(NETAPI_SERVER_GET_INFO_REQ);
    if (servername) {
        name_len = (wcslen(servername) + 1) * sizeof(WCHAR);
        req_len += name_len;
    } else
        name_len = -1;

    req = (NETAPI_SERVER_GET_INFO_REQ *)Dll_Alloc(req_len);
    req->h.length = req_len;
    req->h.msgid = MSGID_NETAPI_SERVER_GET_INFO;
    req->level = level;
    req->name_len = name_len;
    if (servername)
        wcscpy(req->name, servername);

    rpl = (NETAPI_SERVER_GET_INFO_RPL *)
                SbieDll_CallServer((MSG_HEADER *)req);
    Dll_Free(req);

    if (! rpl)
        return ERROR_NOT_ENOUGH_MEMORY;

    status = rpl->h.status;
    if (status == 0)
        status = __sys_NetApiBufferAllocate(rpl->data_len, buf_ptr);
    if (status == 0)
        memcpy(*buf_ptr, rpl->data, rpl->data_len);

    Dll_Free(rpl);

    return status;
}


#endif


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------


#if 0


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


typedef NTSTATUS (*P_I_NetlogonGetTrustRid)(
    void *Unknown1, void *Unknown2, void *RidStruct);

static P_I_NetlogonGetTrustRid      __sys_I_NetlogonGetTrustRid     = NULL;

static ULONG Lsa_I_NetlogonGetTrustRid(
    void *Unknown1, void *Unknown2, void *RidStruct);


//---------------------------------------------------------------------------
// Lsa_Init_NetApi
//---------------------------------------------------------------------------


ALIGNED BOOLEAN Lsa_Init_NetApi(HMODULE module)
{
    void *I_NetlogonGetTrustRid;

    if (! module)
        return;

    //
    // intercept NETAPI32 entry points
    //

    I_NetlogonGetTrustRid = (P_I_NetlogonGetTrustRid)
        GetProcAddress(module, "I_NetlogonGetTrustRid");

    SBIEDLL_HOOK(Lsa_,I_NetlogonGetTrustRid);

    return TRUE;
}


//---------------------------------------------------------------------------
// Lsa_LsaRegisterLogonProcess
//---------------------------------------------------------------------------


static ULONG Lsa_I_NetlogonGetTrustRid(
    void *Unknown1, void *Unknown2, void *RidStruct)
{
    //
    // assume we're have connectivity problems trying to logon to the domain
    //

    return ERROR_TRUSTED_RELATIONSHIP_FAILURE;
}


#endif
