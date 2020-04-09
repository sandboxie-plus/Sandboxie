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
// IpHelper Server -- using PipeServer
//---------------------------------------------------------------------------


#ifndef _MY_IPHLPWIRE_H
#define _MY_IPHLPWIRE_H


#include "msgids.h"
#include <ipexport.h>
#include <icmpapi.h>


//---------------------------------------------------------------------------
// Create File
//---------------------------------------------------------------------------


struct tagIPHLP_CREATE_FILE_REQ
{
    MSG_HEADER h;
    BOOLEAN ip6;
};

struct tagIPHLP_CREATE_FILE_RPL
{
    MSG_HEADER h;                       // status is Win32 error code
    ULONG handle;
};

typedef struct tagIPHLP_CREATE_FILE_REQ IPHLP_CREATE_FILE_REQ;
typedef struct tagIPHLP_CREATE_FILE_RPL IPHLP_CREATE_FILE_RPL;


//---------------------------------------------------------------------------
// Close Handle
//---------------------------------------------------------------------------


struct tagIPHLP_CLOSE_HANDLE_REQ
{
    MSG_HEADER h;
    ULONG handle;
};

typedef struct tagIPHLP_CLOSE_HANDLE_REQ IPHLP_CLOSE_HANDLE_REQ;


//---------------------------------------------------------------------------
// Send Echo
//---------------------------------------------------------------------------


struct tagIPHLP_SEND_ECHO_REQ
{
    MSG_HEADER h;

    BOOLEAN iswow64;                    // TRUE if caller is a Wow64 process

    BOOLEAN ip6;                        // TRUE for Icmp6SendEcho2, else:
    BOOLEAN ex2;                        // TRUE for IcmpSendEcho2Ex, else:
                                        //          IcmpSendEcho2

    BOOLEAN ipopt_valid;
    UCHAR ipopt_ttl;
    UCHAR ipopt_tos;
    UCHAR ipopt_flags;

    ULONG handle;

    ULONG timeout;

    UCHAR src_addr[32];                 // IPAddr or struct sockaddr_in6
    UCHAR dst_addr[32];                 // IPAddr or struct sockaddr_in6

    ULONG reply_size;
    ULONG request_size;
    UCHAR request_data[1];

};

struct tagIPHLP_SEND_ECHO_RPL
{
    MSG_HEADER h;                       // status is Win32 error code
    ULONG num_replies;
    ULONG reply_size;
    UCHAR reply_data[1];

};

typedef struct tagIPHLP_SEND_ECHO_REQ IPHLP_SEND_ECHO_REQ;
typedef struct tagIPHLP_SEND_ECHO_RPL IPHLP_SEND_ECHO_RPL;


//---------------------------------------------------------------------------


#endif /* _MY_IPHLPWIRE_H */
