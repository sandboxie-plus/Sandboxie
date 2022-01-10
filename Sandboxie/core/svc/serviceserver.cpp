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
// Service Server -- using PipeServer
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "serviceserver.h"
#include "servicewire.h"
#include "core/dll/sbiedll.h"
#include "common/defines.h"
#include "common/my_version.h"


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


ServiceServer::ServiceServer(PipeServer *pipeServer)
{
    pipeServer->Register(MSGID_SERVICE, this, Handler);
}


//---------------------------------------------------------------------------
// Handler
//---------------------------------------------------------------------------


MSG_HEADER *ServiceServer::Handler(void *_this, MSG_HEADER *msg)
{
    ServiceServer *pThis = (ServiceServer *)_this;

    if (PipeServer::ImpersonateCaller(&msg) != 0)
        return msg;

    if (msg->msgid == MSGID_SERVICE_QUERY)
        return pThis->QueryHandler(msg);

    if (msg->msgid == MSGID_SERVICE_LIST)
        return pThis->ListHandler(msg);

    HANDLE idProcess = (HANDLE)(ULONG_PTR)PipeServer::GetCallerProcessId();

    if (msg->msgid == MSGID_SERVICE_START) // start unboxed service on the host
        return pThis->StartHandler(msg, idProcess);

    if (msg->msgid == MSGID_SERVICE_RUN) // start a sandboxed service inside a particular box
        return pThis->RunHandler(msg, idProcess); 

    HANDLE idThread = (HANDLE)(ULONG_PTR)PipeServer::GetCallerThreadId();

    if (msg->msgid == MSGID_SERVICE_UAC)
        return pThis->UacHandler(msg, idProcess, idThread);

    return NULL;
}


//---------------------------------------------------------------------------
// StartHandler
//---------------------------------------------------------------------------


MSG_HEADER *ServiceServer::StartHandler(MSG_HEADER *msg, HANDLE idProcess)
{
    //
    // parse request packet
    //

    SERVICE_START_REQ *req = (SERVICE_START_REQ *)msg;
    if (req->h.length < sizeof(SERVICE_START_REQ))
        return SHORT_REPLY(ERROR_INVALID_PARAMETER);

    ULONG name_len = req->name_len * sizeof(WCHAR);
    if (name_len > PIPE_MAX_DATA_LEN)
        return SHORT_REPLY(ERROR_INVALID_PARAMETER);
    ULONG offset = FIELD_OFFSET(SERVICE_START_REQ, name);
    if (offset + name_len > req->h.length)
        return SHORT_REPLY(ERROR_INVALID_PARAMETER);

    /*x
    should do:
    if (! IsAdmin())
        return SHORT_REPLY(ERROR_ACCESS_DENIED)
    and NOT:
    IsAdmin
    if (! CanCallerDoElevation(idProcess, req->name, &idSession))
        return SHORT_REPLY(ERROR_ACCESS_DENIED);
    */

    //
    // execute request
    //

    ULONG status = 0;

    SC_HANDLE handle1 = OpenSCManager(NULL, NULL, GENERIC_READ);
    if (! handle1)
        status = GetLastError();
    else {
        SC_HANDLE handle2 = OpenService(handle1, req->name, SERVICE_START);
        if (! handle2)
            status = GetLastError();
        else {
            if (! StartService(handle2, 0, NULL))
                status = GetLastError();
            CloseServiceHandle(handle2);
        }
        CloseServiceHandle(handle1);
    }

    return SHORT_REPLY(status);
}


//---------------------------------------------------------------------------
// QueryHandler
//---------------------------------------------------------------------------


MSG_HEADER *ServiceServer::QueryHandler(MSG_HEADER *msg)
{
    ULONG error;
    ULONG cfg_len;

    //
    // parse request packet
    //

    SERVICE_QUERY_REQ *req = (SERVICE_QUERY_REQ *)msg;
    if (req->h.length < sizeof(SERVICE_QUERY_REQ))
        return SHORT_REPLY(ERROR_INVALID_PARAMETER);

    ULONG name_len = req->name_len * sizeof(WCHAR);
    if (name_len > PIPE_MAX_DATA_LEN)
        return SHORT_REPLY(ERROR_INVALID_PARAMETER);
    ULONG offset = FIELD_OFFSET(SERVICE_QUERY_REQ, name);
    if (offset + name_len > req->h.length)
        return SHORT_REPLY(ERROR_INVALID_PARAMETER);

    //
    // open connection to service manager
    //

    SC_HANDLE handle1 = OpenSCManager(NULL, NULL, GENERIC_READ);
    if (! handle1)
        return SHORT_REPLY(GetLastError());

    SC_HANDLE handle2 = OpenService(
        handle1, req->name, SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG);
    if (! handle2) {
        error = GetLastError();
        CloseServiceHandle(handle1);
        return SHORT_REPLY(error);
    }

    //
    // calculate length needed for reply
    //

    ULONG rpl_len = sizeof(SERVICE_QUERY_RPL);

    if (req->with_service_config) {

        if (req->with_service_config == (USHORT)-1)
            QueryServiceConfig(handle2, NULL, 0, &cfg_len);
        else {
            QueryServiceConfig2(
                handle2, req->with_service_config, NULL, 0, &cfg_len);
        }

        error = GetLastError();
        if (error != ERROR_INSUFFICIENT_BUFFER) {
            if (error == 0)
                error = ERROR_INVALID_DATA;
            CloseServiceHandle(handle2);
            CloseServiceHandle(handle1);
            return SHORT_REPLY(error);
        }

        rpl_len += cfg_len + 8;

    } else
        cfg_len = 0;

    //
    // execute request and build reply
    //

    error = 0;

    SERVICE_QUERY_RPL *rpl = (SERVICE_QUERY_RPL *)LONG_REPLY(rpl_len);
    if (rpl) {

		memzero(((UCHAR *)rpl) + sizeof(MSG_HEADER), rpl_len - sizeof(MSG_HEADER));

        if (req->with_service_status) {

            ULONG len = sizeof(rpl->service_status);
            if (! QueryServiceStatusEx(
                        handle2, SC_STATUS_PROCESS_INFO,
                        (BYTE *)&rpl->service_status, len, &len))
                error = GetLastError();
        }

        if ((! error) && req->with_service_config) {

            QUERY_SERVICE_CONFIG *cfg = &rpl->service_config;
            BOOL b;

            if (req->with_service_config == (USHORT)-1)
                b = QueryServiceConfig(
                        handle2, cfg, cfg_len, &rpl->service_config_len);
            else {
                b = QueryServiceConfig2(
                        handle2, req->with_service_config,
                        (BYTE *)cfg, cfg_len, &rpl->service_config_len);
            }

            if (! b)
                error = GetLastError();

#define ADJUST_MEMBER(x)    if (cfga->x) cfga->x -= (ULONG_PTR)cfga

            else if (req->with_service_config == (USHORT)-1) {

                QUERY_SERVICE_CONFIGA *cfga = (QUERY_SERVICE_CONFIGA *)cfg;
                ADJUST_MEMBER(lpBinaryPathName);
                ADJUST_MEMBER(lpLoadOrderGroup);
                ADJUST_MEMBER(lpDependencies);
                ADJUST_MEMBER(lpServiceStartName);
                ADJUST_MEMBER(lpDisplayName);

            } else if (req->with_service_config ==
                                            SERVICE_CONFIG_DESCRIPTION) {

                SERVICE_DESCRIPTIONA *cfga = (SERVICE_DESCRIPTIONA *)cfg;
                ADJUST_MEMBER(lpDescription);

            } else if (req->with_service_config ==
                                            SERVICE_CONFIG_FAILURE_ACTIONS) {

                SERVICE_FAILURE_ACTIONSA *cfga =
                                            (SERVICE_FAILURE_ACTIONSA *)cfg;
                ADJUST_MEMBER(lpRebootMsg);
                ADJUST_MEMBER(lpCommand);
                if (cfga->lpsaActions) {
                    UCHAR *lpsaActions = (UCHAR *)cfga->lpsaActions
                                       - (ULONG_PTR)cfga;
                    cfga->lpsaActions  = (SC_ACTION *)lpsaActions;
                }
            }

#undef ADJUST_MEMBER

        }
    }

    CloseServiceHandle(handle2);
    CloseServiceHandle(handle1);

    if (! rpl)
        return SHORT_REPLY(ERROR_NOT_ENOUGH_MEMORY);

    rpl->h.status = error;
    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// ListHandler
//---------------------------------------------------------------------------


MSG_HEADER *ServiceServer::ListHandler(MSG_HEADER *msg)
{
    //
    // parse request packet
    //

    SERVICE_LIST_REQ *req = (SERVICE_LIST_REQ *)msg;
    if (req->h.length < sizeof(SERVICE_LIST_REQ))
        return SHORT_REPLY(ERROR_INVALID_PARAMETER);

    //
    // open connection to service manager
    //

    SC_HANDLE handle1 = OpenSCManager(NULL, NULL, GENERIC_READ);
    if (! handle1)
        return SHORT_REPLY(GetLastError());

    //
    // enumerate services into a buffer that is large enough
    //

    ULONG buf_len = 0;
    ENUM_SERVICE_STATUS *buf = NULL;
    ULONG err = 0;
    ULONG num_services;
    ULONG i, j;

    while (1) {

        if (buf)
            HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, buf);
        if (err)
            break;
        buf_len += PAGE_SIZE * 4;
        buf = (ENUM_SERVICE_STATUS *)HeapAlloc(GetProcessHeap(), 0, buf_len);
        if (! buf) {
            err = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        i = j = 0;
        BOOL b = EnumServicesStatus(
            handle1, req->type_filter, req->state_filter, buf, buf_len,
            &i, &num_services, &j);
        if (b)
            break;

        err = GetLastError();
        if (err == ERROR_MORE_DATA)
            err = 0;
    }

    CloseServiceHandle(handle1);
    if (err)
        return SHORT_REPLY(err);

    //
    // calculate length needed for reply
    //

    ULONG rpl_len = sizeof(SERVICE_LIST_RPL) + sizeof(WCHAR);

    for (i = 0; i < num_services; ++i)
        rpl_len += (wcslen(buf[i].lpServiceName) + 1) * sizeof(WCHAR);

    SERVICE_LIST_RPL *rpl = (SERVICE_LIST_RPL *)LONG_REPLY(rpl_len);
    if (rpl) {

        rpl->h.status = 0;

        WCHAR *names = rpl->names;
        for (i = 0; i < num_services; ++i) {
            wcscpy(names, buf[i].lpServiceName);
            names += wcslen(names) + 1;
        }
        *names = L'\0';
    }

    HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, buf);

    if (! rpl)
        return SHORT_REPLY(ERROR_NOT_ENOUGH_MEMORY);

    return (MSG_HEADER *)rpl;
}
