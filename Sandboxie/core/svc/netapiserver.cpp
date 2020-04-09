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
// Net Api Services Server -- using PipeServer
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "netapiserver.h"
#include "netapiwire.h"
#include "msgids.h"
#include "core/dll/sbiedll.h"
#include "common/my_version.h"
#include "common/defines.h"
#include <lm.h>


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


NetApiServer::NetApiServer(PipeServer *pipeServer)
{
    pipeServer->Register(MSGID_NETAPI, this, Handler);
}


//---------------------------------------------------------------------------
// Handler
//---------------------------------------------------------------------------


MSG_HEADER *NetApiServer::Handler(void *_this, MSG_HEADER *msg)
{
    NetApiServer *pThis = (NetApiServer *)_this;

    if (msg->msgid == MSGID_NETAPI_USE_ADD)
        return pThis->UseAdd(msg);

#if 0

    if (msg->msgid == MSGID_NETAPI_WKSTA_GET_INFO)
        return pThis->WkstaGetInfo(msg);

    if (msg->msgid == MSGID_NETAPI_SERVER_GET_INFO)
        return pThis->ServerGetInfo(msg);

#endif

    return NULL;
}


//---------------------------------------------------------------------------
// UseAdd
//---------------------------------------------------------------------------


MSG_HEADER *NetApiServer::UseAdd(MSG_HEADER *msg)
{
    typedef struct _USE_INFO_4 {
        USE_INFO_3  ui4_ui3;
        ULONG       ui4_auth_identity_length;
        UCHAR      *ui4_auth_identity;
    } USE_INFO_4;

    USE_INFO_4 info;
    ULONG parm_index = 0;
    ULONG error_code = 0;

    //
    // validate request structure
    //

    NETAPI_USE_ADD_REQ *req = (NETAPI_USE_ADD_REQ *)msg;
    if (req->h.length != sizeof(NETAPI_USE_ADD_REQ)) {
        error_code = ERROR_INVALID_PARAMETER;
        goto finish;
    }
    if (req->level > 4) {
        error_code = ERROR_INVALID_PARAMETER;
        goto finish;
    }

    if (req->level >= 0) {

        USE_INFO_0 *info0 = (USE_INFO_0 *)&info;

        if (req->ui0_local_len == -1)
            info0->ui0_local = NULL;
        else if (req->ui0_local_len > 256)
            parm_index = USE_LOCAL_PARMNUM;
        else {
            req->ui0_local[256] = L'\0';
            req->ui0_local[req->ui0_local_len] = L'\0';
            info0->ui0_local = req->ui0_local;
        }

        if (req->ui0_remote_len == -1)
            info0->ui0_remote = NULL;
        else if (req->ui0_remote_len > 256)
            parm_index = USE_REMOTE_PARMNUM;
        else {
            req->ui0_remote[256] = L'\0';
            req->ui0_remote[req->ui0_remote_len] = L'\0';
            info0->ui0_remote = req->ui0_remote;
        }
    }

    if (req->level >= 1) {

        USE_INFO_1 *info1 = (USE_INFO_1 *)&info;

        if (req->ui1_password_len == -1)
            info1->ui1_password = NULL;
        else if (req->ui1_password_len > 256)
            parm_index = USE_PASSWORD_PARMNUM;
        else {
            req->ui1_password[256] = L'\0';
            req->ui1_password[req->ui1_password_len] = L'\0';
            info1->ui1_password = req->ui1_password;
        }

        info1->ui1_status   = req->ui1_status;
        info1->ui1_asg_type = req->ui1_asg_type;
        info1->ui1_refcount = req->ui1_refcount;
        info1->ui1_usecount = req->ui1_usecount;
    }

    if (req->level >= 2) {

        USE_INFO_2 *info2 = (USE_INFO_2 *)&info;

        if (req->ui2_username_len == -1)
            info2->ui2_username = NULL;
        else if (req->ui2_username_len > 256)
            parm_index = USE_USERNAME_PARMNUM;
        else {
            req->ui2_username[256] = L'\0';
            req->ui2_username[req->ui2_username_len] = L'\0';
            info2->ui2_username = req->ui2_username;
        }

        if (req->ui2_domainname_len == -1)
            info2->ui2_domainname = NULL;
        else if (req->ui2_domainname_len > 256)
            parm_index = USE_DOMAINNAME_PARMNUM;
        else {
            req->ui2_domainname[256] = L'\0';
            req->ui2_domainname[req->ui2_domainname_len] = L'\0';
            info2->ui2_domainname = req->ui2_domainname;
        }
    }

    if (req->level >= 3) {

        USE_INFO_3 *info3 = (USE_INFO_3 *)&info;

        info3->ui3_flags = req->ui3_flags;
    }

    if (req->level >= 4) {

        USE_INFO_4 *info4 = (USE_INFO_4 *)&info;

        if (req->ui4_auth_identity_length == -1) {
            info4->ui4_auth_identity_length = 0;
            info4->ui4_auth_identity = NULL;
        } else if (req->ui4_auth_identity_length > 2048)
            error_code = ERROR_INVALID_PARAMETER;
        else {
            req->ui4_auth_identity[req->ui4_auth_identity_length + 0] = 0;
            req->ui4_auth_identity[req->ui4_auth_identity_length + 1] = 0;
            info4->ui4_auth_identity = req->ui4_auth_identity;
            info4->ui4_auth_identity_length = req->ui4_auth_identity_length;
        }
    }

    if (parm_index) {
        error_code = ERROR_INVALID_PARAMETER;
        goto finish;
    }

    //
    // invoke command on behalf of caller
    //

    error_code = PipeServer::ImpersonateCaller();
    if (error_code != 0) {
        error_code = RtlNtStatusToDosError(error_code);
        goto finish;
    }

    error_code = NetUseAdd(NULL, req->level, (UCHAR *)&info, &parm_index);

finish:

    NETAPI_USE_ADD_RPL *rpl =
        (NETAPI_USE_ADD_RPL *)LONG_REPLY(sizeof(NETAPI_USE_ADD_RPL));

    rpl->h.status = error_code;
    rpl->parm_index = parm_index;

    if (error_code == 0)
        LaunchSlave(req->ui0_local_len, req->ui0_local);

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// LaunchSlave
//---------------------------------------------------------------------------


void NetApiServer::LaunchSlave(ULONG len, const WCHAR *drive)
{
    //
    // outside the sandbox, Windows Explorer invokes DefineDosDevice after a
    // call to NetUseAdd, to broadcast the WM_DEVICECHANGE message to all
    // windows on the same desktop.  but since DefineDosDevice is blocked in
    // the sandbox, we run the NetApiServer SbieSvc slave to call it outside
    // the sandbox on the same desktop as our calling process
    //

    if (len != 2 || drive[1] != L':')
        return;

    STARTUPINFO si;
    WCHAR cmdline[128];
    wsprintf(cmdline, L"%s_NetProxy:Use=%c", SANDBOXIE, drive[0]);
    if (! SbieDll_RunFromHome(SBIESVC_EXE, cmdline, &si, NULL))
        return;
    WCHAR *ExePath = (WCHAR *)si.lpReserved;

    const ULONG TOKEN_RIGHTS =
        TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY;
    HANDLE hOldToken;
    if (OpenThreadToken(
            GetCurrentThread(), TOKEN_RIGHTS, FALSE, &hOldToken)) {

        HANDLE hNewToken;
        if (DuplicateTokenEx(
                hOldToken, TOKEN_RIGHTS, NULL, SecurityAnonymous,
                TokenPrimary, &hNewToken)) {

            PROCESS_INFORMATION pi;

            memzero(&si, sizeof(STARTUPINFO));
            si.cb = sizeof(STARTUPINFO);
            si.dwFlags = STARTF_FORCEOFFFEEDBACK;

            if (CreateProcessAsUser(
                    hNewToken, NULL, ExePath, NULL, NULL,
                    FALSE, 0, NULL, NULL, &si, &pi)) {

                WaitForSingleObject(pi.hProcess, 5000);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            }

            CloseHandle(hNewToken);
        }

        CloseHandle(hOldToken);
    }
    if (ExePath)
        HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, ExePath);
}


//---------------------------------------------------------------------------
// RunSlave
//---------------------------------------------------------------------------


void NetApiServer::RunSlave(const WCHAR *cmdline)
{
    cmdline = wcschr(cmdline, L':');
    if (cmdline && wmemcmp(cmdline, L":Use=", 5) == 0) {

        WCHAR device[8];
        device[0] = towupper(cmdline[5]);
        device[1] = L':';
        device[2] = L'\0';
        DefineDosDevice(DDD_LUID_BROADCAST_DRIVE, device, NULL);
    }

    ExitProcess(0);
}











#if 0


//---------------------------------------------------------------------------
// WkstaGetInfo
//---------------------------------------------------------------------------


MSG_HEADER *NetApiServer::WkstaGetInfo(MSG_HEADER *msg)
{
    //
    // validate request structure
    //

    NETAPI_WKSTA_GET_INFO_REQ *req = (NETAPI_WKSTA_GET_INFO_REQ *)msg;
    if (req->h.length < sizeof(NETAPI_WKSTA_GET_INFO_REQ))
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);
    if (req->name_len > PIPE_MAX_DATA_LEN)
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);
    ULONG offset = FIELD_OFFSET(NETAPI_WKSTA_GET_INFO_REQ, name);
    if ((req->name_len != -1) && (offset + req->name_len > req->h.length))
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);

    //
    // issue request
    //

    WCHAR *servername = NULL;
    if (req->name_len != -1)
        servername = req->name;

    BYTE *buf;
    NET_API_STATUS status = NetWkstaGetInfo(servername, req->level, &buf);
    if (status != 0)
        return SHORT_REPLY(status);

    ULONG buf_len;
    status = NetApiBufferSize(buf, &buf_len);
    if (status != 0) {
        NetApiBufferFree(buf);
        return SHORT_REPLY(status);
    }

    ULONG rpl_len = sizeof(NETAPI_WKSTA_GET_INFO_RPL) + buf_len;
    NETAPI_WKSTA_GET_INFO_RPL *rpl =
        (NETAPI_WKSTA_GET_INFO_RPL *)LONG_REPLY(rpl_len);

    if (rpl) {
        rpl->h.status = status;
        rpl->data_len = buf_len;
        memcpy(rpl->data, buf, buf_len);

        NetApiBufferFree(buf);
    }

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// ServerGetInfo
//---------------------------------------------------------------------------


MSG_HEADER *NetApiServer::ServerGetInfo(MSG_HEADER *msg)
{
    //
    // validate request structure
    //

    NETAPI_SERVER_GET_INFO_REQ *req = (NETAPI_SERVER_GET_INFO_REQ *)msg;
    if (req->h.length < sizeof(NETAPI_SERVER_GET_INFO_REQ))
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);
    if (req->name_len > PIPE_MAX_DATA_LEN)
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);
    ULONG offset = FIELD_OFFSET(NETAPI_SERVER_GET_INFO_REQ, name);
    if ((req->name_len != -1) && (offset + req->name_len > req->h.length))
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);

    //
    // issue request
    //

    WCHAR *servername = NULL;
    if (req->name_len != -1)
        servername = req->name;

    BYTE *buf;
    NET_API_STATUS status = NetServerGetInfo(servername, req->level, &buf);
    if (status != 0)
        return SHORT_REPLY(status);

    ULONG buf_len;
    status = NetApiBufferSize(buf, &buf_len);
    if (status != 0) {
        NetApiBufferFree(buf);
        return SHORT_REPLY(status);
    }

    ULONG rpl_len = sizeof(NETAPI_SERVER_GET_INFO_RPL) + buf_len;
    NETAPI_SERVER_GET_INFO_RPL *rpl =
        (NETAPI_SERVER_GET_INFO_RPL *)LONG_REPLY(rpl_len);

    if (rpl) {
        rpl->h.status = status;
        rpl->data_len = buf_len;
        memcpy(rpl->data, buf, buf_len);

        NetApiBufferFree(buf);
    }

    return (MSG_HEADER *)rpl;
}


#endif
