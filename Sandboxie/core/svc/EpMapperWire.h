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
// EpMapper Server -- using PipeServer
//---------------------------------------------------------------------------


#ifndef _MY_EPMAPPERWIRE_H
#define _MY_EPMAPPERWIRE_H


#undef NTDDI_VERSION
#define NTDDI_VERSION NTDDI_VISTA
#define __RPC_FAR
#include <rpc.h>

#include "common/defines.h"

//---------------------------------------------------------------------------
// EPMapper Get Port Name Service
//---------------------------------------------------------------------------


struct tagEPMAPPER_GET_PORT_NAME_REQ
{
    MSG_HEADER h;
    RPC_IF_ID   ifidRequest;
};

struct tagEPMAPPER_GET_PORT_NAME_RPL
{
    MSG_HEADER h;           // h.status is RPC_STATUS
    HRESULT hr;
    WCHAR wszPortName[DYNAMIC_PORT_NAME_CHARS];
};

typedef struct tagEPMAPPER_GET_PORT_NAME_REQ    EPMAPPER_GET_PORT_NAME_REQ;
typedef struct tagEPMAPPER_GET_PORT_NAME_RPL    EPMAPPER_GET_PORT_NAME_RPL;


#endif  // _MY_EPMAPPERWIRE_H