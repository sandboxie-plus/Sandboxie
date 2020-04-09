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
// Protected Storage Server -- using PipeServer
//---------------------------------------------------------------------------

#ifndef _MY_PSTOREWIRE_H
#define _MY_PSTOREWIRE_H


#include "msgids.h"


//---------------------------------------------------------------------------
// Get Type Info
//---------------------------------------------------------------------------


struct tagPSTORE_GET_TYPE_INFO_REQ
{
    MSG_HEADER h;
    GUID type_id;
};

struct tagPSTORE_GET_TYPE_INFO_RPL
{
    MSG_HEADER h;                       // status is HRESULT
    ULONG flags;
    ULONG name_len;
    WCHAR name[1];
};

typedef struct tagPSTORE_GET_TYPE_INFO_REQ PSTORE_GET_TYPE_INFO_REQ;
typedef struct tagPSTORE_GET_TYPE_INFO_RPL PSTORE_GET_TYPE_INFO_RPL;


//---------------------------------------------------------------------------
// Get Subtype Info
//---------------------------------------------------------------------------


struct tagPSTORE_GET_SUBTYPE_INFO_REQ
{
    MSG_HEADER h;
    GUID type_id;
    GUID subtype_id;
};

struct tagPSTORE_GET_SUBTYPE_INFO_RPL
{
    MSG_HEADER h;                       // status is HRESULT
    ULONG flags;
    ULONG name_len;
    WCHAR name[1];
};

typedef struct tagPSTORE_GET_SUBTYPE_INFO_REQ PSTORE_GET_SUBTYPE_INFO_REQ;
typedef struct tagPSTORE_GET_SUBTYPE_INFO_RPL PSTORE_GET_SUBTYPE_INFO_RPL;


//---------------------------------------------------------------------------
// Read Item
//---------------------------------------------------------------------------


struct tagPSTORE_READ_ITEM_REQ
{
    MSG_HEADER h;
    GUID type_id;
    GUID subtype_id;
    ULONG name_len;
    WCHAR name[1];
};

struct tagPSTORE_READ_ITEM_RPL
{
    MSG_HEADER h;                       // status is HRESULT
    ULONG data_len;
    UCHAR data[1];
};

typedef struct tagPSTORE_READ_ITEM_REQ PSTORE_READ_ITEM_REQ;
typedef struct tagPSTORE_READ_ITEM_RPL PSTORE_READ_ITEM_RPL;


//---------------------------------------------------------------------------
// Enumerate Types and Subtypes
//---------------------------------------------------------------------------


struct tagPSTORE_ENUM_TYPES_REQ
{
    MSG_HEADER h;
    ULONG pst_key;
    GUID type_id;
    BOOLEAN enum_subtypes;
};

struct tagPSTORE_ENUM_TYPES_RPL
{
    MSG_HEADER h;                       // status is HRESULT
    ULONG count;
    GUID guids[1];
};

typedef struct tagPSTORE_ENUM_TYPES_REQ PSTORE_ENUM_TYPES_REQ;
typedef struct tagPSTORE_ENUM_TYPES_RPL PSTORE_ENUM_TYPES_RPL;


//---------------------------------------------------------------------------
// Enumerate Items
//---------------------------------------------------------------------------


struct tagPSTORE_ENUM_ITEMS_REQ
{
    MSG_HEADER h;
    ULONG pst_key;
    GUID type_id;
    GUID subtype_id;
};

struct tagPSTORE_ENUM_ITEMS_RPL
{
    MSG_HEADER h;                       // status is HRESULT
    ULONG count;
    WCHAR names[1];
};

typedef struct tagPSTORE_ENUM_ITEMS_REQ PSTORE_ENUM_ITEMS_REQ;
typedef struct tagPSTORE_ENUM_ITEMS_RPL PSTORE_ENUM_ITEMS_RPL;


//---------------------------------------------------------------------------

#endif /* _MY_PSTOREWIRE_H */
