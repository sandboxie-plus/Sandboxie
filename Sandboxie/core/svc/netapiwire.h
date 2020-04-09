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


#ifndef _MY_NETAPIWIRE_H
#define _MY_NETAPIWIRE_H


#include "msgids.h"


//---------------------------------------------------------------------------
// NetUseAdd
//---------------------------------------------------------------------------


struct tagNETAPI_USE_ADD_REQ
{
    MSG_HEADER h;
    UCHAR   level;
    ULONG   ui0_local_len;
    ULONG   ui0_remote_len;
    ULONG   ui1_password_len;
    ULONG   ui2_username_len;
    ULONG   ui2_domainname_len;
    ULONG   ui1_status;
    ULONG   ui1_asg_type;
    ULONG   ui1_refcount;
    ULONG   ui1_usecount;
    ULONG   ui3_flags;
    ULONG   ui4_auth_identity_length;
    UCHAR   ui4_auth_identity[2048+2];
    WCHAR   ui0_local[256+1];
    WCHAR   ui0_remote[256+1];
    WCHAR   ui1_password[256+1];
    WCHAR   ui2_username[256+1];
    WCHAR   ui2_domainname[256+1];
};


struct tagNETAPI_USE_ADD_RPL
{
    MSG_HEADER h;                       // status is NET_API_STATUS
    ULONG parm_index;
};

typedef struct tagNETAPI_USE_ADD_REQ NETAPI_USE_ADD_REQ;
typedef struct tagNETAPI_USE_ADD_RPL NETAPI_USE_ADD_RPL;


//---------------------------------------------------------------------------


#if 0


//---------------------------------------------------------------------------
// NetWkstaGetInfo
//---------------------------------------------------------------------------


struct tagNETAPI_WKSTA_GET_INFO_REQ
{
    MSG_HEADER h;
    ULONG level;
    ULONG name_len;                     // BYTE count, or -1 for NULL name
    WCHAR name[1];
};


struct tagNETAPI_WKSTA_GET_INFO_RPL
{
    MSG_HEADER h;                       // status is NET_API_STATUS
    ULONG data_len;                     // BYTE count
    UCHAR data[1];
};

typedef struct tagNETAPI_WKSTA_GET_INFO_REQ NETAPI_WKSTA_GET_INFO_REQ;
typedef struct tagNETAPI_WKSTA_GET_INFO_RPL NETAPI_WKSTA_GET_INFO_RPL;


//---------------------------------------------------------------------------
// NetServerGetInfo
//---------------------------------------------------------------------------


struct tagNETAPI_SERVER_GET_INFO_REQ
{
    MSG_HEADER h;
    ULONG level;
    ULONG name_len;                     // BYTE count, or -1 for NULL name
    WCHAR name[1];
};


struct tagNETAPI_SERVER_GET_INFO_RPL
{
    MSG_HEADER h;                       // status is NET_API_STATUS
    ULONG data_len;                     // BYTE count
    UCHAR data[1];
};

typedef struct tagNETAPI_SERVER_GET_INFO_REQ NETAPI_SERVER_GET_INFO_REQ;
typedef struct tagNETAPI_SERVER_GET_INFO_RPL NETAPI_SERVER_GET_INFO_RPL;


#endif


//---------------------------------------------------------------------------


#endif /* _MY_NETAPIWIRE_H */
