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
// File Server -- using PipeServer
//---------------------------------------------------------------------------


#ifndef _MY_FILEWIRE_H
#define _MY_FILEWIRE_H


#include "msgids.h"


//---------------------------------------------------------------------------
// Set Attributes
//---------------------------------------------------------------------------


struct tagFILE_SET_ATTRIBUTES_REQ
{
    MSG_HEADER h;
    FILE_BASIC_INFORMATION info;
    ULONG path_len;                     // BYTE count
    WCHAR path[1];
};

struct tagFILE_SET_ATTRIBUTES_RPL
{
    MSG_HEADER h;                       // status is NTSTATUS
};

typedef struct tagFILE_SET_ATTRIBUTES_REQ FILE_SET_ATTRIBUTES_REQ;
typedef struct tagFILE_SET_ATTRIBUTES_RPL FILE_SET_ATTRIBUTES_RPL;


//---------------------------------------------------------------------------
// Set Short Name
//---------------------------------------------------------------------------


struct tagFILE_SET_SHORT_NAME_REQ
{
    MSG_HEADER h;
    FILE_SHORT_NAME_INFORMATION info;
    ULONG path_len;                     // BYTE count
    WCHAR path[1];
};

struct tagFILE_SET_SHORT_NAME_RPL
{
    MSG_HEADER h;                       // status is NTSTATUS
};

typedef struct tagFILE_SET_SHORT_NAME_REQ FILE_SET_SHORT_NAME_REQ;
typedef struct tagFILE_SET_SHORT_NAME_RPL FILE_SET_SHORT_NAME_RPL;


//---------------------------------------------------------------------------
// Set Reparse Point
//---------------------------------------------------------------------------


struct tagFILE_SET_REPARSE_POINT_REQ
{
    MSG_HEADER h;
    ULONG src_path_len;                 // BYTE count
    ULONG dst_path_len;                 // BYTE count
    ULONG dst_path_ofs;                 // BYTE offset
    WCHAR src_path[1];
    // WCHAR dst_path[1];
};

typedef struct tagFILE_SET_REPARSE_POINT_REQ FILE_SET_REPARSE_POINT_REQ;


//---------------------------------------------------------------------------
// Load/Mount Registry Key
//---------------------------------------------------------------------------


struct tagFILE_LOAD_KEY_REQ
{
    MSG_HEADER h;
    WCHAR KeyPath[128];
    WCHAR FilePath[128];
};

typedef struct tagFILE_LOAD_KEY_REQ FILE_LOAD_KEY_REQ;


//---------------------------------------------------------------------------
// Open Wow64 Registry Key
//---------------------------------------------------------------------------


struct tagFILE_OPEN_WOW64_KEY_REQ
{
    MSG_HEADER h;
    ULONG Wow64DesiredAccess;           // KEY_WOW64_32KEY or KEY_WOW64_64KEY
    ULONG KeyPath_len;                  // BYTE count
    WCHAR KeyPath[1];
};

struct tagFILE_OPEN_WOW64_KEY_RPL
{
    MSG_HEADER h;                       // status is NTSTATUS
    WCHAR KeyPath[1];
};

typedef struct tagFILE_OPEN_WOW64_KEY_REQ FILE_OPEN_WOW64_KEY_REQ;
typedef struct tagFILE_OPEN_WOW64_KEY_RPL FILE_OPEN_WOW64_KEY_RPL;


//---------------------------------------------------------------------------
// Check Registry Key
//---------------------------------------------------------------------------


struct tagFILE_CHECK_KEY_EXISTS_REQ
{
    MSG_HEADER h;
    ULONG KeyPath_len;                  // BYTE count
    WCHAR KeyPath[1];
};

typedef struct tagFILE_CHECK_KEY_EXISTS_REQ FILE_CHECK_KEY_EXISTS_REQ;


//---------------------------------------------------------------------------
// Get All Handles
//---------------------------------------------------------------------------


struct tagFILE_GET_ALL_HANDLES_RPL
{
    MSG_HEADER h;                       // status is NTSTATUS
    ULONG num_handles;
    ULONG handles[1];
};

typedef struct tagFILE_GET_ALL_HANDLES_RPL FILE_GET_ALL_HANDLES_RPL;


//---------------------------------------------------------------------------


#endif /* _MY_FILEWIRE_H */
