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

#include "dll.h"

#include <windows.h>
#include <wchar.h>
#include <oleauto.h>
#include "core/svc/IpHlpWire.h"
#include "core/svc/netapiwire.h"
#include "common/my_wsa.h"


//---------------------------------------------------------------------------
//
// Windows Home Network Configuration
//
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static HRESULT HNet_IcfOpenDynamicFwPort(void *p1, void *p2, void *p3);


//---------------------------------------------------------------------------


typedef HRESULT (*P_IcfOpenDynamicFwPort)(void *p1, void *p2, void *p3);


//---------------------------------------------------------------------------


static P_IcfOpenDynamicFwPort   __sys_IcfOpenDynamicFwPort  = NULL;


//---------------------------------------------------------------------------
// IcfOpenDynamicFwPort
//---------------------------------------------------------------------------


_FX HRESULT HNet_IcfOpenDynamicFwPort(void *p1, void *p2, void *p3)
{
    // bind (Winsock 2) calls WSPBind, then HNetCfg.IcfOpenDynamicFwPort.
    // IcfOpenDynamicFwPort tries to talk to the Firewall service,
    // which is probably only accessible as a COM object (as opposed to
    // a well known port name), so it fails.
    // Here we intercept IcfOpenDynamicFwPort and return a success status,
    // while not actually talking to the Windows Firewall

    return 0;
}


//---------------------------------------------------------------------------
// HNet_Init
//---------------------------------------------------------------------------


_FX BOOLEAN HNet_Init(HMODULE module)
{
    P_IcfOpenDynamicFwPort IcfOpenDynamicFwPort;

    IcfOpenDynamicFwPort = (P_IcfOpenDynamicFwPort)
        GetProcAddress(module, "IcfOpenDynamicFwPort");

    if (IcfOpenDynamicFwPort) {
        SBIEDLL_HOOK(HNet_,IcfOpenDynamicFwPort);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
//
// Network Shares
//
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _USE_INFO_0 {
    WCHAR      *ui0_local;
    WCHAR      *ui0_remote;
} USE_INFO_0;

typedef struct _USE_INFO_1 {
    USE_INFO_0  ui1_ui0;
    WCHAR      *ui1_password;
    ULONG       ui1_status;
    ULONG       ui1_asg_type;
    ULONG       ui1_refcount;
    ULONG       ui1_usecount;
} USE_INFO_1;

typedef struct _USE_INFO_2 {
    USE_INFO_1  ui2_ui1;
    WCHAR      *ui2_username;
    WCHAR      *ui2_domainname;
} USE_INFO_2;

typedef struct _USE_INFO_3 {
    USE_INFO_2  ui3_ui2;
    ULONG       ui3_flags;
} USE_INFO_3;

typedef struct _USE_INFO_4 {
    USE_INFO_3  ui4_ui3;
    ULONG       ui4_auth_identity_length;
    UCHAR      *ui4_auth_identity;
} USE_INFO_4;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN NetApi_Hook_NetUseAdd(HMODULE module);

static ULONG NetApi_NetUseAdd(
    WCHAR *ServerName, ULONG Level, UCHAR *Buf, ULONG *ParmError);


//---------------------------------------------------------------------------


typedef ULONG (*P_NetUseAdd)(
    WCHAR *ServerName, ULONG Level, UCHAR *Buf, ULONG *ParmError);


//---------------------------------------------------------------------------
// NetApi_Init
//---------------------------------------------------------------------------


_FX BOOLEAN NetApi_Init(HMODULE module)
{
    if (Dll_OsBuild >= 2600 && Dll_OsBuild < 7600)  // XP, Vista
        return NetApi_Hook_NetUseAdd(module);
    else
        return TRUE;
}


//---------------------------------------------------------------------------
// NetApi_Init_WksCli
//---------------------------------------------------------------------------


_FX BOOLEAN NetApi_Init_WksCli(HMODULE module)
{
    if (Dll_OsBuild >= 7600)    // Windows 7
        return NetApi_Hook_NetUseAdd(module);
    else
        return TRUE;
}


//---------------------------------------------------------------------------
// NetApi_Hook_NetUseAdd
//---------------------------------------------------------------------------


_FX BOOLEAN NetApi_Hook_NetUseAdd(HMODULE module)
{
    P_NetUseAdd NetUseAdd;

    //
    // if \Device\Mup is closed, don't hook NetUseAdd, which would cause
    // use of the function to fail due to the use of RestrictToken in SbieSvc
    //

    if (1) {

        extern const WCHAR *File_Mup;
        ULONG mp_flags = SbieDll_MatchPath(L'f', File_Mup);
        if (PATH_IS_CLOSED(mp_flags))
            return TRUE;
    }

    //
    // otherwise hook NetUseAdd
    //

    NetUseAdd = (P_NetUseAdd)GetProcAddress(module, "NetUseAdd");
    if (NetUseAdd) {
        P_NetUseAdd __sys_NetUseAdd;
        SBIEDLL_HOOK(NetApi_,NetUseAdd);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// NetApi_NetUseAdd
//---------------------------------------------------------------------------


_FX ULONG NetApi_NetUseAdd(
    WCHAR *ServerName, ULONG Level, UCHAR *Buf, ULONG *ParmError)
{
    NETAPI_USE_ADD_REQ *req;
    NETAPI_USE_ADD_RPL *rpl;
    USE_INFO_0 *info0;
    USE_INFO_1 *info1;
    USE_INFO_2 *info2;
    USE_INFO_3 *info3;
    USE_INFO_4 *info4;
    ULONG err;
    ULONG drive_number;

    //
    // validate parameters and build a request packet
    //

    if (ServerName || (Level > 4)) {
        req = NULL;
        goto abort;
    }

    info0 = (USE_INFO_0 *)Buf;
    info1 = (USE_INFO_1 *)Buf;
    info2 = (USE_INFO_2 *)Buf;
    info3 = (USE_INFO_3 *)Buf;
    info4 = (USE_INFO_4 *)Buf;

    req = Dll_AllocTemp(sizeof(NETAPI_USE_ADD_REQ));
    req->h.length = sizeof(NETAPI_USE_ADD_REQ);
    req->h.msgid = MSGID_NETAPI_USE_ADD;

    req->level = (UCHAR)Level;

    if (req->level >= 0) {

        if (info0->ui0_local) {
            req->ui0_local_len = wcslen(info0->ui0_local);
            if (req->ui0_local_len > 256)
                goto abort;
            wmemcpy(req->ui0_local, info0->ui0_local,
                    req->ui0_local_len);
        } else
            req->ui0_local_len = -1;

        if (info0->ui0_remote) {
            req->ui0_remote_len = wcslen(info0->ui0_remote);
            if (req->ui0_remote_len > 256)
                goto abort;
            wmemcpy(req->ui0_remote, info0->ui0_remote,
                    req->ui0_remote_len);
        } else
            req->ui0_remote_len = -1;
    }

    if (req->level >= 1) {

        if (info1->ui1_password) {
            req->ui1_password_len = wcslen(info1->ui1_password);
            if (req->ui1_password_len > 256)
                goto abort;
            wmemcpy(req->ui1_password, info1->ui1_password,
                    req->ui1_password_len);
        } else
            req->ui1_password_len = -1;

        req->ui1_status   = info1->ui1_status;
        req->ui1_asg_type = info1->ui1_asg_type;
        req->ui1_refcount = info1->ui1_refcount;
        req->ui1_usecount = info1->ui1_usecount;
    }

    if (req->level >= 2) {

        if (info2->ui2_username) {
            req->ui2_username_len = wcslen(info2->ui2_username);
            if (req->ui2_username_len > 256)
                goto abort;
            wmemcpy(req->ui2_username, info2->ui2_username,
                    req->ui2_username_len);
        } else
            req->ui2_username_len = -1;

        if (info2->ui2_domainname) {
            req->ui2_domainname_len = wcslen(info2->ui2_domainname);
            if (req->ui2_domainname_len > 256)
                goto abort;
            wmemcpy(req->ui2_domainname, info2->ui2_domainname,
                    req->ui2_domainname_len);
        } else
            req->ui2_domainname_len = -1;
    }

    if (req->level >= 3) {

        req->ui3_flags = info3->ui3_flags;
    }

    if (req->level >= 4) {

        if (info4->ui4_auth_identity_length > 2048)
            goto abort;
        if (info4->ui4_auth_identity_length && info4->ui4_auth_identity) {
            req->ui4_auth_identity_length = info4->ui4_auth_identity_length;
            memcpy(req->ui4_auth_identity, info4->ui4_auth_identity,
                   req->ui4_auth_identity_length);
        } else
            req->ui4_auth_identity_length = -1;
    }

    //
    // send request to SbieSvc and examine reply
    // note that SbieSvc will call DefineDosDevice outside the sandbox,
    // see SbieSvc!NetApiServer::LaunchSlave
    //

    drive_number = 0;
    if (req->ui0_local_len != -1 && req->ui0_local_len >= 2 &&
            req->ui0_local[1] == L':') {
        WCHAR drive_letter = req->ui0_local[0];
        if (drive_letter >= L'a' && drive_letter <= L'z')
            drive_number = (drive_letter - L'a');
        else if (drive_letter >= L'A' && drive_letter <= L'Z')
            drive_number = (drive_letter - L'A');
    }

    rpl = (NETAPI_USE_ADD_RPL *)SbieDll_CallServer(&req->h);

    Dll_Free(req);

    if (! rpl)
        err = ERROR_NOT_ENOUGH_MEMORY;
    else {
        err = rpl->h.status;
        if (ParmError && err == ERROR_INVALID_PARAMETER &&
                rpl->h.length >= sizeof(NETAPI_USE_ADD_RPL))
            *ParmError = rpl->parm_index;
        Dll_Free(rpl);
    }

    if ((! err) && drive_number)
        SbieDll_DeviceChange(0xAA00 + drive_number, tzuk);

    return err;

abort:

    if (req)
        Dll_Free(req);
    SbieApi_Log(2205, L"NetUseAdd");
    return ERROR_ACCESS_DENIED;
}




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
