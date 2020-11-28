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
// Queue Server -- using PipeServer
//---------------------------------------------------------------------------


#ifndef _MY_QUEUEWIRE_H
#define _MY_QUEUEWIRE_H


#include "../../common/defines.h"
#include "msgids.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define QUEUE_NAME_MAXLEN   64


//---------------------------------------------------------------------------
// Create Queue
//---------------------------------------------------------------------------


struct tagQUEUE_CREATE_REQ
{
    MSG_HEADER h;
    WCHAR queue_name[QUEUE_NAME_MAXLEN];
    __declspec(align(8)) ULONG64 event_handle;
};

struct tagQUEUE_CREATE_RPL
{
    MSG_HEADER h;                       // status is NTSTATUS
};

typedef struct tagQUEUE_CREATE_REQ QUEUE_CREATE_REQ;
typedef struct tagQUEUE_CREATE_RPL QUEUE_CREATE_RPL;


//---------------------------------------------------------------------------
// Get Request from Queue
//---------------------------------------------------------------------------


struct tagQUEUE_GETREQ_REQ
{
    MSG_HEADER h;
    WCHAR queue_name[QUEUE_NAME_MAXLEN];
};

struct tagQUEUE_GETREQ_RPL
{
    MSG_HEADER h;                       // status is NTSTATUS
    ULONG client_pid;
    ULONG client_tid;
    ULONG req_id;
    ULONG data_len;
    UCHAR data[1];
};

typedef struct tagQUEUE_GETREQ_REQ QUEUE_GETREQ_REQ;
typedef struct tagQUEUE_GETREQ_RPL QUEUE_GETREQ_RPL;


//---------------------------------------------------------------------------
// Put Reply in Queue
//---------------------------------------------------------------------------


struct tagQUEUE_PUTRPL_REQ
{
    MSG_HEADER h;
    WCHAR queue_name[QUEUE_NAME_MAXLEN];
    ULONG req_id;
    ULONG data_len;
    UCHAR data[1];
};

struct tagQUEUE_PUTRPL_RPL
{
    MSG_HEADER h;                       // status is NTSTATUS
};

typedef struct tagQUEUE_PUTRPL_REQ QUEUE_PUTRPL_REQ;
typedef struct tagQUEUE_PUTRPL_RPL QUEUE_PUTRPL_RPL;


//---------------------------------------------------------------------------
// Put Request in Queue
//---------------------------------------------------------------------------


struct tagQUEUE_PUTREQ_REQ
{
    MSG_HEADER h;
    WCHAR queue_name[QUEUE_NAME_MAXLEN];
    __declspec(align(8)) ULONG64 event_handle;
    ULONG data_len;
    UCHAR data[1];
};

struct tagQUEUE_PUTREQ_RPL
{
    MSG_HEADER h;                       // status is NTSTATUS
    ULONG req_id;
};

typedef struct tagQUEUE_PUTREQ_REQ QUEUE_PUTREQ_REQ;
typedef struct tagQUEUE_PUTREQ_RPL QUEUE_PUTREQ_RPL;


//---------------------------------------------------------------------------
// Get Reply from Queue
//---------------------------------------------------------------------------


struct tagQUEUE_GETRPL_REQ
{
    MSG_HEADER h;
    WCHAR queue_name[QUEUE_NAME_MAXLEN];
    ULONG req_id;
};

struct tagQUEUE_GETRPL_RPL
{
    MSG_HEADER h;                       // status is NTSTATUS
    ULONG data_len;
    UCHAR data[1];
};

typedef struct tagQUEUE_GETRPL_REQ QUEUE_GETRPL_REQ;
typedef struct tagQUEUE_GETRPL_RPL QUEUE_GETRPL_RPL;


//---------------------------------------------------------------------------


#endif /* _MY_QUEUEWIRE_H */
