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


#ifndef _MY_SERVICEWIRE_H
#define _MY_SERVICEWIRE_H


#include "msgids.h"


//---------------------------------------------------------------------------
// Start Service
//---------------------------------------------------------------------------


struct tagSERVICE_START_REQ
{
    MSG_HEADER h;
    ULONG name_len;
    WCHAR name[1];
};

struct tagSERVICE_START_RPL
{
    MSG_HEADER h;                       // status is Win32 error
};

typedef struct tagSERVICE_START_REQ SERVICE_START_REQ;
typedef struct tagSERVICE_START_RPL SERVICE_START_RPL;

typedef struct tagSERVICE_START_REQ SERVICE_STOP_REQ;
typedef struct tagSERVICE_START_RPL SERVICE_STOP_RPL;


//---------------------------------------------------------------------------
// Query Service
//---------------------------------------------------------------------------


struct tagSERVICE_QUERY_REQ
{
    MSG_HEADER h;
    USHORT with_service_status;
    USHORT with_service_config;
    ULONG name_len;
    WCHAR name[1];
};

struct tagSERVICE_QUERY_RPL
{
    MSG_HEADER h;                       // status is Win32 error
    SERVICE_STATUS_PROCESS service_status;
    ULONG service_config_len;
    QUERY_SERVICE_CONFIG service_config;
};

typedef struct tagSERVICE_QUERY_REQ SERVICE_QUERY_REQ;
typedef struct tagSERVICE_QUERY_RPL SERVICE_QUERY_RPL;


//---------------------------------------------------------------------------
// List Services
//---------------------------------------------------------------------------


struct tagSERVICE_LIST_REQ
{
    MSG_HEADER h;
    ULONG type_filter;
    ULONG state_filter;
};

struct tagSERVICE_LIST_RPL
{
    MSG_HEADER h;                       // status is Win32 error
    WCHAR names[1];
};

typedef struct tagSERVICE_LIST_REQ SERVICE_LIST_REQ;
typedef struct tagSERVICE_LIST_RPL SERVICE_LIST_RPL;


//---------------------------------------------------------------------------
// Run Service
//---------------------------------------------------------------------------


struct tagSERVICE_RUN_REQ
{
    MSG_HEADER h;
    ULONG type;
    WCHAR name[64];
    WCHAR devmap[96];
    ULONG path_len;                     // in bytes, including NULL
    WCHAR path[1];
};

typedef struct tagSERVICE_RUN_REQ SERVICE_RUN_REQ;


//---------------------------------------------------------------------------
// UAC Service
//---------------------------------------------------------------------------


struct tagSERVICE_UAC_REQ
{
    MSG_HEADER h;
    WCHAR   devmap[96];
    ULONG64 uac_pkt_addr;
    ULONG   uac_pkt_len;
};

typedef struct tagSERVICE_UAC_REQ SERVICE_UAC_REQ;


//---------------------------------------------------------------------------
// WowSpl64 Service
//---------------------------------------------------------------------------


struct tagSERVICE_SPL_REQ
{
    MSG_HEADER h;
    WCHAR integrity_level;
    ULONG integrity_level_win7sp1;
};

typedef struct tagSERVICE_SPL_REQ SERVICE_SPL_REQ;


//---------------------------------------------------------------------------


#endif /* _MY_SERVICEWIRE_H */
