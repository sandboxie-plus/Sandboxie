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
// Terminal Server -- using PipeServer
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "terminalserver.h"
#include "terminalwire.h"
#include "misc.h"
#include "core/dll/sbiedll.h"
#include "common/defines.h"
#include <wtsapi32.h>
#include "core/drv/api_defs.h"
#include "core/dll/sbiedll.h"


//---------------------------------------------------------------------------
// Imports from Windows headers
//---------------------------------------------------------------------------


#define SERVERNAME_CURRENT  ((HANDLE)NULL)

typedef enum _WINSTATIONINFOCLASS {
    WinStationInformation = 8
} WINSTATIONINFOCLASS;

typedef BOOLEAN (*P_WinStaQueryInformation)(
    HANDLE, ULONG, ULONG, PVOID, ULONG, PULONG);

typedef BOOLEAN (*P_WinStationIsSessionRemoteable)(
    HANDLE, ULONG, PBOOLEAN);

typedef BOOLEAN (*P_WinStationNameFromLogonId)(
    HANDLE, ULONG, WCHAR *);

typedef BOOLEAN (*P_WinStationGetConnectionProperty)(
    ULONG, GUID *, PVOID);

typedef BOOLEAN (*P_WinStationFreePropertyValue)(
    PVOID);

typedef BOOLEAN (*P_WinStationDisconnect)(
    HANDLE, ULONG, ULONG_PTR);


#define WTS_VALUE_TYPE_ULONG        1
#define WTS_VALUE_TYPE_STRING       2
#define WTS_VALUE_TYPE_BINARY       3
#define WTS_VALUE_TYPE_GUID         4


typedef struct __WTS_PROPERTY_VALUE {

    unsigned short Type;
    union {
        ULONG ulVal;
        struct {
            ULONG size;
            WCHAR *pstrVal;
        } strVal;
        struct {
            ULONG size;
            char *pbVal;
        } bVal;
        GUID guidVal;
    } u;

} WTS_PROPERTY_VALUE;


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


TerminalServer::TerminalServer(PipeServer *pipeServer)
{
    m_WinStaQueryInformation = NULL;
    m_WinStationIsSessionRemoteable = NULL;
    m_WinStationNameFromLogonId = NULL;
    m_WinStationGetConnectionProperty = NULL;
    m_WinStationFreePropertyValue = NULL;

    HMODULE _winsta = LoadLibrary(L"winsta.dll");
    if (_winsta) {
        m_WinStaQueryInformation =
            GetProcAddress(_winsta, "WinStationQueryInformationW");
        m_WinStationIsSessionRemoteable =
            GetProcAddress(_winsta, "WinStationIsSessionRemoteable");
        m_WinStationNameFromLogonId =
            GetProcAddress(_winsta, "WinStationNameFromLogonIdW");
        m_WinStationGetConnectionProperty =
            GetProcAddress(_winsta, "WinStationGetConnectionProperty");
        m_WinStationFreePropertyValue =
            GetProcAddress(_winsta, "WinStationFreePropertyValue");
        m_WinStationDisconnect =
            GetProcAddress(_winsta, "WinStationDisconnect");
    }

    pipeServer->Register(MSGID_TERMINAL, this, Handler);
}


//---------------------------------------------------------------------------
// Handler
//---------------------------------------------------------------------------


MSG_HEADER *TerminalServer::Handler(void *_this, MSG_HEADER *msg)
{
    TerminalServer *pThis = (TerminalServer *)_this;

    if (msg->msgid == MSGID_TERMINAL_QUERY_INFO)
        return pThis->QueryInfo(msg);

    if (msg->msgid == MSGID_TERMINAL_CHECK_TYPE)
        return pThis->CheckType(msg);

    if (msg->msgid == MSGID_TERMINAL_GET_NAME)
        return pThis->GetName(msg);

    if (msg->msgid == MSGID_TERMINAL_GET_PROPERTY)
        return pThis->GetProperty(msg);

    if (msg->msgid == MSGID_TERMINAL_DISCONNECT)
        return pThis->Disconnect(msg);

    if (msg->msgid == MSGID_TERMINAL_GET_USER_TOKEN)
        return pThis->GetUserToken(msg);

    return NULL;
}


//---------------------------------------------------------------------------
// GOTO_FINISH_IF_NO_API
//---------------------------------------------------------------------------


#define GOTO_FINISH_IF_NO_API(ApiName)                                      \
    if (! m_##ApiName) {                                                    \
        error = ERROR_SERVICE_DOES_NOT_EXIST;                               \
        goto finish;                                                        \
    }                                                                       \
    P_##ApiName p##ApiName = (P_##ApiName) m_##ApiName;


//---------------------------------------------------------------------------
// QueryInfo
//---------------------------------------------------------------------------


MSG_HEADER *TerminalServer::QueryInfo(MSG_HEADER *msg)
{
    TERMINAL_QUERY_INFO_RPL *rpl = NULL;
    ULONG error;

    GOTO_FINISH_IF_NO_API(WinStaQueryInformation);

    //
    // validate request
    //

    TERMINAL_QUERY_INFO_REQ *req = (TERMINAL_QUERY_INFO_REQ *)msg;
    if (req->h.length != sizeof(TERMINAL_QUERY_INFO_REQ)) {
        error = ERROR_INVALID_PARAMETER;
        goto finish;
    }
    if (req->data_len > PIPE_MAX_DATA_LEN) {
        error = ERROR_INVALID_PARAMETER;
        goto finish;
    }

    //
    // issue request
    //

    ULONG data_len = req->data_len;
    void *data = HeapAlloc(GetProcessHeap(), 0, data_len);
    if (! data)
        error = ERROR_OUTOFMEMORY;
    else {

        BOOLEAN ok = pWinStaQueryInformation(
                        SERVERNAME_CURRENT, req->session_id, req->info_class,
                        data, data_len, &data_len);
        if (! ok)
            error = GetLastError();
        else {

            //
            // prepare reply
            //

            ULONG rpl_len = sizeof(TERMINAL_QUERY_INFO_RPL) + data_len;
            rpl = (TERMINAL_QUERY_INFO_RPL *)LONG_REPLY(rpl_len);
            if (! rpl)
                error = ERROR_OUTOFMEMORY;
            else {

                rpl->data_len = data_len;
                memcpy(rpl->data, data, data_len);
                error = ERROR_SUCCESS;
            }
        }

        HeapFree(GetProcessHeap(), 0, data);
    }

    //
    // finish
    //

finish:

    if (! rpl)
        rpl = (TERMINAL_QUERY_INFO_RPL *)SHORT_REPLY(error);
    else
        rpl->h.status = error;

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// CheckType
//---------------------------------------------------------------------------


MSG_HEADER *TerminalServer::CheckType(MSG_HEADER *msg)
{
    TERMINAL_CHECK_TYPE_RPL *rpl = NULL;
    ULONG error;

    //
    // validate request
    //

    TERMINAL_CHECK_TYPE_REQ *req = (TERMINAL_CHECK_TYPE_REQ *)msg;
    if (req->h.length != sizeof(TERMINAL_CHECK_TYPE_REQ)) {
        error = ERROR_INVALID_PARAMETER;
        goto finish;
    }

    //
    // execute request
    //

    ULONG data_len = 0;
    ULONG data;
    BOOLEAN ok;

    if (req->check_is_remote) {

        GOTO_FINISH_IF_NO_API(WinStationIsSessionRemoteable);

        ok = pWinStationIsSessionRemoteable(
                SERVERNAME_CURRENT, req->session_id,
                (BOOLEAN *)&data);

        error = ok ? ERROR_SUCCESS : GetLastError();

        data_len = sizeof(UCHAR);

    } else
        error = ERROR_INVALID_PARAMETER;

    if (error)
        goto finish;

    //
    // prepare response
    //

    ULONG rpl_len = sizeof(TERMINAL_CHECK_TYPE_RPL) + data_len;
    rpl = (TERMINAL_CHECK_TYPE_RPL *)LONG_REPLY(rpl_len);
    if (! rpl)
        error = ERROR_OUTOFMEMORY;
    else {

        rpl->data_len = data_len;
        memcpy(rpl->data, &data, data_len);
    }

    //
    // finish
    //

finish:

    if (! rpl)
        rpl = (TERMINAL_CHECK_TYPE_RPL *)SHORT_REPLY(error);
    else
        rpl->h.status = error;

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// GetName
//---------------------------------------------------------------------------


MSG_HEADER *TerminalServer::GetName(MSG_HEADER *msg)
{
    TERMINAL_GET_NAME_RPL *rpl = NULL;
    ULONG error;

    GOTO_FINISH_IF_NO_API(WinStationNameFromLogonId);

    //
    // validate request
    //

    TERMINAL_GET_NAME_REQ *req = (TERMINAL_GET_NAME_REQ *)msg;
    if (req->h.length != sizeof(TERMINAL_GET_NAME_REQ)) {
        error = ERROR_INVALID_PARAMETER;
        goto finish;
    }

    //
    // issue request
    //

    WCHAR name[128];
    BOOLEAN ok = pWinStationNameFromLogonId(
                            SERVERNAME_CURRENT, req->session_id, name);
    if (! ok) {
        error = GetLastError();
        goto finish;
    }

    ULONG rpl_len = sizeof(TERMINAL_GET_NAME_RPL);
    rpl = (TERMINAL_GET_NAME_RPL *)LONG_REPLY(rpl_len);
    if (! rpl)
        error = ERROR_OUTOFMEMORY;
    else {

        wmemcpy(rpl->name, name, 120);
        name[120] = L'\0';
    }

    //
    // finish
    //

finish:

    if (! rpl)
        rpl = (TERMINAL_GET_NAME_RPL *)SHORT_REPLY(error);
    else
        rpl->h.status = error;

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// GetProperty
//---------------------------------------------------------------------------


MSG_HEADER *TerminalServer::GetProperty(MSG_HEADER *msg)
{
    TERMINAL_GET_PROPERTY_RPL *rpl = NULL;
    ULONG error;

    GOTO_FINISH_IF_NO_API(WinStationGetConnectionProperty);
    GOTO_FINISH_IF_NO_API(WinStationFreePropertyValue);

    //
    // validate request
    //

    TERMINAL_GET_PROPERTY_REQ *req = (TERMINAL_GET_PROPERTY_REQ *)msg;
    if (req->h.length != sizeof(TERMINAL_GET_PROPERTY_REQ)) {
        error = ERROR_INVALID_PARAMETER;
        goto finish;
    }

    //
    // issue request
    //

    WTS_PROPERTY_VALUE *value = NULL;
    BOOLEAN ok = pWinStationGetConnectionProperty(
                            req->session_id, &req->guid, (void *)&value);
    if (! ok) {
        error = GetLastError();
        goto finish;
    }

    ULONG data_len = 0;
    void *data_ptr = NULL;

    if (value->Type == WTS_VALUE_TYPE_ULONG) {
        data_len = sizeof(ULONG);
        data_ptr = &value->u.ulVal;

    } else if (value->Type == WTS_VALUE_TYPE_STRING) {
        data_len = value->u.strVal.size * sizeof(WCHAR);
        data_ptr = value->u.strVal.pstrVal;

    } else if (value->Type == WTS_VALUE_TYPE_BINARY) {
        data_len = value->u.bVal.size;
        data_ptr = value->u.bVal.pbVal;

    } else if (value->Type == WTS_VALUE_TYPE_GUID) {
        data_len = sizeof(GUID);
        data_ptr = &value->u.guidVal;

    } else
        error = ERROR_UNKNOWN_PROPERTY;

    if (data_ptr) {

        ULONG rpl_len = sizeof(TERMINAL_GET_PROPERTY_RPL) + data_len;
        rpl = (TERMINAL_GET_PROPERTY_RPL *)LONG_REPLY(rpl_len);
        if (! rpl)
            error = ERROR_OUTOFMEMORY;
        else {

            rpl->type = value->Type;
            rpl->len  = data_len;
            memcpy(&rpl->data, data_ptr, data_len);
            error = ERROR_SUCCESS;
        }
    }

    pWinStationFreePropertyValue(value);

    //
    // finish
    //

finish:

    if (! rpl)
        rpl = (TERMINAL_GET_PROPERTY_RPL *)SHORT_REPLY(error);
    else
        rpl->h.status = error;

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// Disconnect
//---------------------------------------------------------------------------


MSG_HEADER *TerminalServer::Disconnect(MSG_HEADER *msg)
{
    ULONG session_id;
    ULONG err;
    BOOLEAN ok = FALSE;

    HANDLE idProcess = (HANDLE)(ULONG_PTR)PipeServer::GetCallerProcessId();

    if (msg->length != sizeof(MSG_HEADER)) {

        err = ERROR_INVALID_PARAMETER;

    } else if (! m_WinStationDisconnect) {

        err = ERROR_SERVICE_DOES_NOT_EXIST;

    } else if (0 != SbieApi_QueryProcess(
                        idProcess, NULL, NULL, NULL, &session_id)) {

        err = ERROR_ACCESS_DENIED;

    } else {

        P_WinStationDisconnect pWinStationDisconnect =
                (P_WinStationDisconnect) m_WinStationDisconnect;

        ok = pWinStationDisconnect(SERVERNAME_CURRENT, session_id, 0);
        if (ok)
            err = 0;
        else
            err = GetLastError();
    }

    return SHORT_REPLY(err);
}


//---------------------------------------------------------------------------
// GetUserToken
//---------------------------------------------------------------------------


MSG_HEADER *TerminalServer::GetUserToken(MSG_HEADER *msg)
{
    ULONG session_id;
    ULONG err;
    BOOL ok = FALSE;

    HANDLE idProcess = (HANDLE)(ULONG_PTR)PipeServer::GetCallerProcessId();

    if (msg->length != sizeof(MSG_HEADER)) {

        err = ERROR_INVALID_PARAMETER;

    } else if (0 != SbieApi_QueryProcess(idProcess, NULL, NULL, NULL, &session_id)) {

        err = ERROR_ACCESS_DENIED;

    } else {

        WCHAR boxname[48] = { 0 };
        SbieApi_QueryProcess(idProcess, boxname, NULL, NULL, NULL);

        HANDLE hCallerProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, (ULONG)(ULONG_PTR)idProcess);
        if (!hCallerProcess)
            err = GetLastError();
        else {
            HANDLE pHandle;

            HANDLE hToken; //hToken = (HANDLE)SbieApi_QueryProcessInfoEx((HANDLE)idProcess, 'ptok', 0);
            if (WTSQueryUserToken(session_id, &hToken)) {
                
                HANDLE hFilteredToken = NULL;

                // OriginalToken BEGIN
                if (!SbieApi_QueryConfBool(boxname, L"NoSecurityIsolation", FALSE) && !SbieApi_QueryConfBool(boxname, L"OriginalToken", FALSE)
                // OriginalToken END
                // OpenToken BEGIN
                 && !SbieApi_QueryConfBool(boxname, L"OpenToken", FALSE) && !SbieApi_QueryConfBool(boxname, L"UnfilteredToken", FALSE))
	            // OpenToken END
                {
                    // of one of the above is true we handle unfiltered tokens
                    // if not we need to filter the token or else security checks in the driver wil fail!

                    if (!NT_SUCCESS(SbieApi_Call(API_FILTER_TOKEN, 3, (ULONG_PTR)idProcess, (ULONG_PTR)hToken, (ULONG_PTR)&hFilteredToken)))
                        goto fail;
                }

                ok = DuplicateHandle(GetCurrentProcess(), hFilteredToken ? hFilteredToken : hToken, hCallerProcess, &pHandle, TOKEN_ALL_ACCESS, FALSE, 0);

                if(hFilteredToken)
                    CloseHandle(hFilteredToken);
                
            fail:
                CloseHandle(hToken);
            }

            if (!ok)
                err = GetLastError();
            
            CloseHandle(hCallerProcess);

            if (ok) {
                ULONG rpl_len = sizeof(GET_USER_TOKEN_RPL);
                GET_USER_TOKEN_RPL* rpl = (GET_USER_TOKEN_RPL*)LONG_REPLY(rpl_len);
                if (!rpl) {
                    err = GetLastError();
                    CloseHandle(pHandle);
                }
                else {
                    rpl->hToken = pHandle;
                    return &rpl->h;
                }
            }
        }
    }

    return SHORT_REPLY(err);
}


