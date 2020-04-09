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
// EpMapperServer -- using PipeServer
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "EpMapperServer.h"
#include "EpMapperWire.h"
#include "core/dll/sbiedll.h"
#include "common/defines.h"


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


EpMapperServer::EpMapperServer(PipeServer *pipeServer)
{
    pipeServer->Register(MSGID_EPMAPPER, this, Handler);
}


//---------------------------------------------------------------------------
// Handler
//---------------------------------------------------------------------------


MSG_HEADER *EpMapperServer::Handler(void *_this, MSG_HEADER *msg)
{
    EpMapperServer *pThis = (EpMapperServer *)_this;

    if (msg->msgid == MSGID_EPMAPPER_GET_PORT_NAME)
        return pThis->EpmapperGetPortNameHandler(msg);

    return NULL;
}


//---------------------------------------------------------------------------
// EpmapperGetPortNameHandler
//---------------------------------------------------------------------------


MSG_HEADER *EpMapperServer::EpmapperGetPortNameHandler(MSG_HEADER *msg)
{
    EPMAPPER_GET_PORT_NAME_REQ *req = (EPMAPPER_GET_PORT_NAME_REQ *)msg;
    if (req->h.length < sizeof(EPMAPPER_GET_PORT_NAME_REQ))
        return SHORT_REPLY(E_INVALIDARG);

    EPMAPPER_GET_PORT_NAME_RPL *rpl =
        (EPMAPPER_GET_PORT_NAME_RPL *)LONG_REPLY(sizeof(EPMAPPER_GET_PORT_NAME_RPL));
    if (rpl == NULL)
        return SHORT_REPLY(E_OUTOFMEMORY);

    RPC_EP_INQ_HANDLE hContext = 0;

    // ask EpMapper for dynamic endpoint names for the desired RPC_IF_ID
    RPC_STATUS status = RpcMgmtEpEltInqBegin(NULL, RPC_C_EP_MATCH_BY_IF, &req->ifidRequest, RPC_C_VERS_ALL, NULL, &hContext);
    if (status == RPC_S_OK)
    {
        RPC_BINDING_HANDLE hBinding = 0;
        RPC_IF_ID ifidEndpoint;

        // return the 1st match that contains "LRPC-"
        while ((status = RpcMgmtEpEltInqNextW(hContext, &ifidEndpoint, &hBinding, NULL, NULL)) == RPC_S_OK)
        {
            RPC_WSTR pwszPortName = NULL;

            status = RpcBindingToStringBindingW(hBinding, &pwszPortName);   // Get string port name. Format is "ncalrpc:[LRPC-f760d5b40689a98168]"
            memset(rpl->wszPortName, 0, sizeof(rpl->wszPortName));
            wcsncpy(rpl->wszPortName, (wchar_t *)pwszPortName, DYNAMIC_PORT_NAME_CHARS);
            RpcStringFreeW(&pwszPortName);
            if (wcsstr(rpl->wszPortName, L"LRPC-"))
                break;
        }
        RpcMgmtEpEltInqDone(&hContext);
    }

    rpl->hr = status;
    return (MSG_HEADER *)rpl;
}


