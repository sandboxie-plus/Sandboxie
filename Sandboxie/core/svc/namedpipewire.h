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
// Named Pipe Proxy Server -- using PipeServer
//---------------------------------------------------------------------------


#ifndef _MY_NAMEDPIPEWIRE_H
#define _MY_NAMEDPIPEWIRE_H


#include "msgids.h"


//---------------------------------------------------------------------------
// IO_STATUS_BLOCK
//---------------------------------------------------------------------------


typedef struct tagNAMED_PIPE_IOSB {

    //
    // if SbieSvc is 64-bit, it returns a 64-bit IO_STATUS_BLOCK even if
    // the caller is a 32-bit process.  the following structure keeps
    // everyone on a 64-bit IO_STATUS_BLOCK, always
    //

    ULONG64 status;
    ULONG64 information;

} NAMED_PIPE_IOSB;


//---------------------------------------------------------------------------
// Open Service
//---------------------------------------------------------------------------


struct tagNAMED_PIPE_OPEN_REQ
{
    MSG_HEADER h;
    ULONG create_options;
    WCHAR name[64];
    WCHAR server[48];
};

struct tagNAMED_PIPE_OPEN_RPL
{
    MSG_HEADER h;                       // status is NTSTATUS
    ULONG handle;
    NAMED_PIPE_IOSB iosb;
};

typedef struct tagNAMED_PIPE_OPEN_REQ NAMED_PIPE_OPEN_REQ;
typedef struct tagNAMED_PIPE_OPEN_RPL NAMED_PIPE_OPEN_RPL;


//---------------------------------------------------------------------------
// Close Service
//---------------------------------------------------------------------------


struct tagNAMED_PIPE_CLOSE_REQ
{
    MSG_HEADER h;
    ULONG handle;
};

struct tagNAMED_PIPE_CLOSE_RPL
{
    MSG_HEADER h;                       // status is NTSTATUS
};

typedef struct tagNAMED_PIPE_CLOSE_REQ NAMED_PIPE_CLOSE_REQ;
typedef struct tagNAMED_PIPE_CLOSE_RPL NAMED_PIPE_CLOSE_RPL;


//---------------------------------------------------------------------------
// Set Information Service
//---------------------------------------------------------------------------


struct tagNAMED_PIPE_SET_REQ
{
    MSG_HEADER h;
    ULONG handle;
    ULONG data_len;
    UCHAR data[1];
};

struct tagNAMED_PIPE_SET_RPL
{
    MSG_HEADER h;                       // status is NTSTATUS
    ULONG handle;
    NAMED_PIPE_IOSB iosb;
};

typedef struct tagNAMED_PIPE_SET_REQ NAMED_PIPE_SET_REQ;
typedef struct tagNAMED_PIPE_SET_RPL NAMED_PIPE_SET_RPL;


//---------------------------------------------------------------------------
// Read Service
//---------------------------------------------------------------------------


struct tagNAMED_PIPE_READ_REQ
{
    MSG_HEADER h;
    ULONG handle;
    ULONG read_len;
};

struct tagNAMED_PIPE_READ_RPL
{
    MSG_HEADER h;                       // status is NTSTATUS
    NAMED_PIPE_IOSB iosb;
    ULONG data_len;
    UCHAR data[1];
};


typedef struct tagNAMED_PIPE_READ_REQ NAMED_PIPE_READ_REQ;
typedef struct tagNAMED_PIPE_READ_RPL NAMED_PIPE_READ_RPL;


//---------------------------------------------------------------------------
// Write Service
//---------------------------------------------------------------------------


struct tagNAMED_PIPE_WRITE_REQ
{
    MSG_HEADER h;
    ULONG handle;
    ULONG data_len;
    UCHAR data[1];
};

struct tagNAMED_PIPE_WRITE_RPL
{
    MSG_HEADER h;                       // status is NTSTATUS
    NAMED_PIPE_IOSB iosb;
};


typedef struct tagNAMED_PIPE_WRITE_REQ NAMED_PIPE_WRITE_REQ;
typedef struct tagNAMED_PIPE_WRITE_RPL NAMED_PIPE_WRITE_RPL;


//---------------------------------------------------------------------------
// Lpc Connect Service
//---------------------------------------------------------------------------


struct tagNAMED_PIPE_LPC_CONNECT_REQ
{
    MSG_HEADER h;
    WCHAR name[64];
    ULONG max_msg_len;
    ULONG info_len;
    UCHAR info_data[1];
};

struct tagNAMED_PIPE_LPC_CONNECT_RPL
{
    MSG_HEADER h;                       // status is NTSTATUS
    ULONG handle;
    ULONG max_msg_len;
    ULONG info_len;
    UCHAR info_data[1];
};


typedef struct tagNAMED_PIPE_LPC_CONNECT_REQ NAMED_PIPE_LPC_CONNECT_REQ;
typedef struct tagNAMED_PIPE_LPC_CONNECT_RPL NAMED_PIPE_LPC_CONNECT_RPL;


//---------------------------------------------------------------------------
// Lpc Request Service
//---------------------------------------------------------------------------


struct tagNAMED_PIPE_LPC_REQUEST_REQ
{
    MSG_HEADER h;
    ULONG handle;
    UCHAR data[328];                    // MAX_PORTMSG_LENGTH
    UCHAR info[1];
};

struct tagNAMED_PIPE_LPC_REQUEST_RPL
{
    MSG_HEADER h;                       // status is NTSTATUS
    UCHAR data[328];                    // MAX_PORTMSG_LENGTH
    UCHAR info[1];
};


typedef struct tagNAMED_PIPE_LPC_REQUEST_REQ NAMED_PIPE_LPC_REQUEST_REQ;
typedef struct tagNAMED_PIPE_LPC_REQUEST_RPL NAMED_PIPE_LPC_REQUEST_RPL;


//---------------------------------------------------------------------------
// ALpc Request Service
//---------------------------------------------------------------------------


struct tagNAMED_PIPE_ALPC_REQUEST_REQ
{
    MSG_HEADER h;
    ULONG handle;
    ULONG msg_len;
    ULONG view[2];                      // some ALPC_MESSAGE_VIEW data
    UCHAR data[24];                     // at least sizeof(PORT_MESSAGE)
};

struct tagNAMED_PIPE_ALPC_REQUEST_RPL
{
    MSG_HEADER h;                       // status is NTSTATUS
    ULONG msg_len;
    ULONG view[3];                      // some ALPC_MESSAGE_VIEW data
    UCHAR data[1];
};


typedef struct tagNAMED_PIPE_ALPC_REQUEST_REQ NAMED_PIPE_ALPC_REQUEST_REQ;
typedef struct tagNAMED_PIPE_ALPC_REQUEST_RPL NAMED_PIPE_ALPC_REQUEST_RPL;


//---------------------------------------------------------------------------
// Thread Impersonate Service
//---------------------------------------------------------------------------


struct tagNAMED_PIPE_IMPERSONATE_REQ
{
    MSG_HEADER h;
    ULONG type;
    ULONG64 handle;
    UCHAR data[328];                    // MAX_PORTMSG_LENGTH
};


typedef struct tagNAMED_PIPE_IMPERSONATE_REQ NAMED_PIPE_IMPERSONATE_REQ;


//---------------------------------------------------------------------------


#endif /* _MY_NAMEDPIPEWIRE_H */
