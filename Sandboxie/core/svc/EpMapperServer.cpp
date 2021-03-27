/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2021 David Xanatos, xanasoft.com
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
#include "core/drv/api_defs.h"

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

    HANDLE idProcess = (HANDLE)(ULONG_PTR)PipeServer::GetCallerProcessId();
    WCHAR boxname[48];
    if (!NT_SUCCESS(SbieApi_QueryProcess(idProcess, boxname, NULL, NULL, NULL)))
        return SHORT_REPLY(E_FAIL);

    const WCHAR* wstrSpooler = L"Spooler";
    /*const WCHAR* wstrWPAD = L"WinHttpAutoProxySvc";
    //const WCHAR* wstrBT = L"bthserv";
    //const WCHAR* wstrSSDP = L"ssdpsrv";
    RPC_IF_ID ifidGCS       = { {0x88abcbc3, 0x34EA, 0x76AE, { 0x82, 0x15, 0x76, 0x75, 0x20, 0x65, 0x5A, 0x23 }}, 0, 0 };   // {88ABCBC3-34EA-76AE-8215-767520655A23}
    RPC_IF_ID ifidSmartCard = { {0xC6B5235A, 0xE413, 0x481D, { 0x9A, 0xC8, 0x31, 0x68, 0x1B, 0x1F, 0xAA, 0xF5 }}, 1, 1 };   // {C6B5235A-E413-481D-9AC8-31681B1FAAF5}
    RPC_IF_ID ifidBluetooth = { {0x2ACB9D68, 0xB434, 0x4B3E, { 0xB9, 0x66, 0xE0, 0x6B, 0x4B, 0x3A, 0x84, 0xCB }}, 1, 0 };   // {2ACB9D68-B434-4B3E-B966-E06B4B3A84CB}
    RPC_IF_ID ifidSSDP      = { {0x4B112204, 0x0E19, 0x11D3, { 0xB4, 0x2B, 0x00, 0x00, 0xF8, 0x1F, 0xEB, 0x9F }}, 1, 0 };   // {4B112204-0E19-11D3-B42B-0000F81FEB9F}*/

    RPC_IF_ID ifidRequest;
    WCHAR pwszServiceName [81];
    *pwszServiceName = 0;

    if (_wcsicmp(req->wszPortId, SPOOLER_PORT_ID) == 0) {
        if (SbieApi_QueryConfBool(boxname, L"ClosePrintSpooler", FALSE)) 
            return SHORT_REPLY(E_ACCESSDENIED);
        wcscpy_s(pwszServiceName, 81, wstrSpooler);
    }
    /*else if (_wcsicmp(req->wszPortId, WPAD_PORT_ID) == 0) {
        wcscpy_s(pwszServiceName, 81, wstrWPAD);
    }
    else if (_wcsicmp(req->wszPortId, BT_PORT_ID) == 0) {
        if (!SbieApi_QueryConfBool(boxname, L"OpenBluetooth", FALSE)) 
            return SHORT_REPLY(E_ACCESSDENIED);
        //wcscpy_s(pwszServiceName, 81, wstrBT);
        memcpy(&ifidRequest, &ifidBluetooth, sizeof(RPC_IF_ID)); 
    }
    else if (_wcsicmp(req->wszPortId, SSDP_PORT_ID) == 0) {
        if (!SbieApi_QueryConfBool(boxname, L"OpenUPnP", FALSE)) 
            return SHORT_REPLY(E_ACCESSDENIED);
        //wcscpy_s(pwszServiceName, 81, wstrSSDP);
        memcpy(&ifidRequest, &ifidSSDP, sizeof(RPC_IF_ID));
    }
    else if (_wcsicmp(req->wszPortId, GAME_CONFIG_STORE_PORT_ID) == 0) {
        memcpy(&ifidRequest, &ifidGCS, sizeof(RPC_IF_ID));
    }
    else if (_wcsicmp(req->wszPortId, SMART_CARD_PORT_ID) == 0) {
        if (!SbieApi_QueryConfBool(boxname, L"OpenSmartCard", TRUE)) 
            return SHORT_REPLY(E_ACCESSDENIED);
        memcpy(&ifidRequest, &ifidSmartCard, sizeof(RPC_IF_ID));
    }
    else 
        return SHORT_REPLY(E_INVALIDARG);*/
    else
    {
        WCHAR buf[MAX_PATH];
        if (SbieDll_GetStringForStringList(req->wszPortId, boxname, L"RpcPortBindingIfId", buf, sizeof(buf)))
        {
            unsigned short uuid[37];
            wmemcpy((WCHAR*)uuid, buf + 1, 36); uuid[36] = 0;

            ifidRequest.VersMajor = 0;
            ifidRequest.VersMinor = 0;
            if(UuidFromString(uuid, &ifidRequest.Uuid) != RPC_S_OK)
                return SHORT_REPLY(E_INVALIDARG);
        }
        else if (SbieDll_GetStringForStringList(req->wszPortId, boxname, L"RpcPortBindingSvc", buf, sizeof(buf)))
        {
            wcscpy_s(pwszServiceName, 81, buf);
        }
        else
            return SHORT_REPLY(E_INVALIDARG);
    }

    EPMAPPER_GET_PORT_NAME_RPL *rpl = (EPMAPPER_GET_PORT_NAME_RPL *)LONG_REPLY(sizeof(EPMAPPER_GET_PORT_NAME_RPL));
    if (rpl == NULL)
        return SHORT_REPLY(E_OUTOFMEMORY);

    rpl->h.status = STATUS_NOT_FOUND;
    rpl->wszPortName[0] = L'\0';

    if (*pwszServiceName) {

        HANDLE hPid = NULL;

        // find the service process
        //ULONG error = 0;
        SC_HANDLE sc_handle = OpenSCManager(NULL, NULL, GENERIC_READ);
        if (sc_handle)
        {
            SC_HANDLE svc_handle = OpenService(sc_handle, pwszServiceName, SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG);
            if (svc_handle) 
            {
                SERVICE_STATUS_PROCESS service_status;
                ULONG len = sizeof(service_status);
                if (QueryServiceStatusEx(svc_handle, SC_STATUS_PROCESS_INFO, (BYTE*)&service_status, len, &len))
                    hPid = (HANDLE)service_status.dwProcessId;
                //else
                //    error = GetLastError();
                CloseServiceHandle(svc_handle);
            }
            //else
            //    error = GetLastError();
            CloseServiceHandle(sc_handle);
        }
        //else
        //    error = GetLastError();

        if (hPid)
        {
            // Param 1 is the service PID
            // Param 2 will return the port name with "\RPC Control\" prepended
            rpl->h.status = SbieApi_CallTwo(
                API_GET_DYNAMIC_PORT_FROM_PID,
                (ULONG_PTR)hPid,
                (ULONG_PTR)rpl->wszPortName);
        }
    }
    else {

        RPC_EP_INQ_HANDLE hContext = 0;

        // ask EpMapper for dynamic endpoint names for the desired RPC_IF_ID
        RPC_STATUS status = RpcMgmtEpEltInqBegin(NULL, RPC_C_EP_MATCH_BY_IF, &ifidRequest, RPC_C_VERS_ALL, NULL, &hContext);
        if (status == RPC_S_OK)
        {
            RPC_BINDING_HANDLE hBinding = 0;
            RPC_IF_ID ifidEndpoint;

            // return the 1st match that contains "LRPC-"
            while ((status = RpcMgmtEpEltInqNextW(hContext, &ifidEndpoint, &hBinding, NULL, NULL)) == RPC_S_OK)
            {
                WCHAR wstrPortName[DYNAMIC_PORT_NAME_CHARS];

                RPC_WSTR pwszPortName = NULL;
                RpcBindingToStringBindingW(hBinding, &pwszPortName);   // Get string port name. Format is "ncalrpc:[LRPC-f760d5b40689a98168]"
                if (pwszPortName == NULL)
                    continue;
                wcsncpy(wstrPortName, (wchar_t*)pwszPortName + 9, DYNAMIC_PORT_NAME_CHARS); // format is "ncalrpc:[LRPC-f760d5b40689a98168]" We only want actual port name
                wstrPortName[23] = 0;                                                       // Take off the ']'
                RpcStringFreeW(&pwszPortName);

                if (wcsncmp(wstrPortName, L"LRPC-", 5) == 0)
                {
                    _snwprintf(rpl->wszPortName, DYNAMIC_PORT_NAME_CHARS, L"\\RPC Control\\%s", wstrPortName);
                    rpl->h.status = STATUS_SUCCESS;
                    break;
                }
            }
            RpcMgmtEpEltInqDone(&hContext);
        }

        //rpl->hr = status;
    }

    if (rpl->h.status == STATUS_SUCCESS)
    {
        //
        // Note: it seams that chrome.exe resolves GAME_CONFIG_STORE_PORT in one process and accesses from an other
        // so since here we only open non critical ports we will use PID 0 to open them globally
        // instead of only for the one process. Todo: make it per sandbox instead
        //

        // Param 1 is dynamic port name (e.g. "LRPC-f760d5b40689a98168"), WCHAR[DYNAMIC_PORT_NAME_CHARS]
        // Param 2 is the process PID for which to open the port, can be 0 when port is special
        // Param 3 is the port type/identifier
        rpl->h.status = SbieApi_CallThree(API_OPEN_DYNAMIC_PORT,
            (ULONG_PTR)rpl->wszPortName,
            (ULONG_PTR)0, 
            (ULONG_PTR)req->wszPortId);
    }

    return (MSG_HEADER *)rpl;
}


