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
// Setup Server -- using PipeServer
//---------------------------------------------------------------------------


#ifndef _MY_TERMINALWIRE_H
#define _MY_TERMINALWIRE_H


#include "msgids.h"


//---------------------------------------------------------------------------
// Query Information
//---------------------------------------------------------------------------


typedef struct _TERMINAL_QUERY_INFO_REQ {

    MSG_HEADER h;
    ULONG session_id;
    ULONG info_class;
    ULONG data_len;

} TERMINAL_QUERY_INFO_REQ;


typedef struct _TERMINAL_QUERY_INFO_RPL {

    MSG_HEADER h;                       // status is Win32 error
    ULONG data_len;
    UCHAR data[1];

} TERMINAL_QUERY_INFO_RPL;


//---------------------------------------------------------------------------
// Check Type
//---------------------------------------------------------------------------


typedef struct _TERMINAL_CHECK_TYPE_REQ {

    MSG_HEADER h;
    ULONG session_id;
    BOOLEAN check_is_remote;

} TERMINAL_CHECK_TYPE_REQ;


typedef struct _TERMINAL_CHECK_TYPE_RPL {

    MSG_HEADER h;                       // status is Win32 error
    ULONG data_len;
    UCHAR data[1];

} TERMINAL_CHECK_TYPE_RPL;


//---------------------------------------------------------------------------
// Get Property
//---------------------------------------------------------------------------


typedef struct _TERMINAL_GET_NAME_REQ {

    MSG_HEADER h;
    ULONG session_id;

} TERMINAL_GET_NAME_REQ;


typedef struct _TERMINAL_GET_NAME_RPL {

    MSG_HEADER h;                       // status is Win32 error
    WCHAR name[128];

} TERMINAL_GET_NAME_RPL;


//---------------------------------------------------------------------------
// Get Property
//---------------------------------------------------------------------------


typedef struct _TERMINAL_GET_PROPERTY_REQ {

    MSG_HEADER h;
    ULONG session_id;
    GUID guid;

} TERMINAL_GET_PROPERTY_REQ;


typedef struct _TERMINAL_GET_PROPERTY_RPL {

    MSG_HEADER h;                       // status is Win32 error
    ULONG type;
    ULONG len;
    UCHAR data[1];

} TERMINAL_GET_PROPERTY_RPL;


//---------------------------------------------------------------------------


#endif /* _MY_TERMINALWIRE_H */
