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
// Com Server -- using PipeServer
//---------------------------------------------------------------------------


#ifndef _MY_COMWIRE_H
#define _MY_COMWIRE_H


#include "common/defines.h"
#include "msgids.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define COM_MAX_UNMARSHAL_BUF_LEN 2048


//---------------------------------------------------------------------------
// COM_RPC_MESSAGE
//---------------------------------------------------------------------------


typedef struct tagCOM_RPC_SYNTAX_IDENTIFIER
{
    GUID SyntaxGUID;
    unsigned short MajorVersion;
    unsigned short MinorVersion;

} COM_RPC_SYNTAX_IDENTIFIER;

typedef struct tagCOM_RPC_MESSAGE
{
    void *Handle;
    unsigned long DataRepresentation;
    void *Buffer;
    unsigned int BufferLength;
    unsigned int ProcNum;
    COM_RPC_SYNTAX_IDENTIFIER *TransferSyntax;
    void *RpcInterfaceInformation;
    void *ReservedForRuntime;
    void *ManagerEpv;
    void *ImportContext;
    unsigned long RpcFlags;

} COM_RPC_MESSAGE;


//---------------------------------------------------------------------------
// Get Class Object Service
//---------------------------------------------------------------------------


struct tagCOM_GET_CLASS_OBJECT_REQ
{
    MSG_HEADER h;
    GUID clsid;
    GUID iid;
    BOOLEAN elevate;
};

struct tagCOM_GET_CLASS_OBJECT_RPL
{
    MSG_HEADER h;                       // status is RPC exception
    HRESULT hr;
    ULONG objidx;
};

typedef struct tagCOM_GET_CLASS_OBJECT_REQ COM_GET_CLASS_OBJECT_REQ;
typedef struct tagCOM_GET_CLASS_OBJECT_RPL COM_GET_CLASS_OBJECT_RPL;


//---------------------------------------------------------------------------
// Create Instance Service
//---------------------------------------------------------------------------


struct tagCOM_CREATE_INSTANCE_REQ
{
    MSG_HEADER h;
    ULONG objidx;
    GUID iid;
};

struct tagCOM_CREATE_INSTANCE_RPL
{
    MSG_HEADER h;                       // status is RPC exception
    HRESULT hr;
    ULONG objidx;
};

typedef struct tagCOM_CREATE_INSTANCE_REQ COM_CREATE_INSTANCE_REQ;
typedef struct tagCOM_CREATE_INSTANCE_RPL COM_CREATE_INSTANCE_RPL;


//---------------------------------------------------------------------------
// Query Interface Service
//---------------------------------------------------------------------------


struct tagCOM_QUERY_INTERFACE_REQ
{
    MSG_HEADER h;
    ULONG objidx;
    GUID iid;
};

struct tagCOM_QUERY_INTERFACE_RPL
{
    MSG_HEADER h;                       // status is RPC exception
    HRESULT hr;
    ULONG objidx;
};

typedef struct tagCOM_QUERY_INTERFACE_REQ COM_QUERY_INTERFACE_REQ;
typedef struct tagCOM_QUERY_INTERFACE_RPL COM_QUERY_INTERFACE_RPL;


//---------------------------------------------------------------------------
// AddRef/Release Service
//---------------------------------------------------------------------------


struct tagCOM_ADD_REF_RELEASE_REQ
{
    MSG_HEADER h;
    ULONG objidx;
    UCHAR op;
};

struct tagCOM_ADD_REF_RELEASE_RPL
{
    MSG_HEADER h;                       // status is RPC exception
    HRESULT hr;
    ULONG refcount;
};

typedef struct tagCOM_ADD_REF_RELEASE_REQ COM_ADD_REF_RELEASE_REQ;
typedef struct tagCOM_ADD_REF_RELEASE_RPL COM_ADD_REF_RELEASE_RPL;


//---------------------------------------------------------------------------
// Invoke Method Service
//---------------------------------------------------------------------------


struct tagCOM_INVOKE_METHOD_REQ
{
    MSG_HEADER h;
    ULONG objidx;
    ULONG DataRepresentation;
    ULONG ProcNum;
    ULONG BufferLength;
    WCHAR Buffer[1];
};

struct tagCOM_INVOKE_METHOD_RPL
{
    MSG_HEADER h;                       // status is RPC exception
    HRESULT hr;
    ULONG DataRepresentation;
    ULONG BufferLength;
    WCHAR Buffer[1];
};

typedef struct tagCOM_INVOKE_METHOD_REQ COM_INVOKE_METHOD_REQ;
typedef struct tagCOM_INVOKE_METHOD_RPL COM_INVOKE_METHOD_RPL;


//---------------------------------------------------------------------------
// Unmarshal Interface Service
//---------------------------------------------------------------------------


struct tagCOM_UNMARSHAL_INTERFACE_REQ
{
    MSG_HEADER h;
    GUID iid;
    ULONG BufferLength;
    UCHAR Buffer[1];
};

struct tagCOM_UNMARSHAL_INTERFACE_RPL
{
    MSG_HEADER h;                       // status is RPC exception
    HRESULT hr;
    ULONG objidx;
};

typedef struct tagCOM_UNMARSHAL_INTERFACE_REQ COM_UNMARSHAL_INTERFACE_REQ;
typedef struct tagCOM_UNMARSHAL_INTERFACE_RPL COM_UNMARSHAL_INTERFACE_RPL;


//---------------------------------------------------------------------------
// Marshal Interface Service
//---------------------------------------------------------------------------


struct tagCOM_MARSHAL_INTERFACE_REQ
{
    MSG_HEADER h;
    ULONG objidx;
    GUID iid;
    ULONG destctx;
    ULONG mshlflags;
};

struct tagCOM_MARSHAL_INTERFACE_RPL
{
    MSG_HEADER h;                       // status is RPC exception
    HRESULT hr;
    ULONG BufferLength;
    WCHAR Buffer[1];
};

typedef struct tagCOM_MARSHAL_INTERFACE_REQ COM_MARSHAL_INTERFACE_REQ;
typedef struct tagCOM_MARSHAL_INTERFACE_RPL COM_MARSHAL_INTERFACE_RPL;


//---------------------------------------------------------------------------
// Query Security Blanket Service
//---------------------------------------------------------------------------


struct tagCOM_QUERY_BLANKET_REQ
{
    MSG_HEADER h;
    ULONG objidx;
};

struct tagCOM_QUERY_BLANKET_RPL
{
    MSG_HEADER h;                       // status is RPC exception
    HRESULT hr;
    ULONG AuthnSvc;
    ULONG AuthzSvc;
    ULONG AuthnLevel;
    ULONG ImpLevel;
    ULONG Capabilities;
    WCHAR ServerPrincName[128];
};

typedef struct tagCOM_QUERY_BLANKET_REQ COM_QUERY_BLANKET_REQ;
typedef struct tagCOM_QUERY_BLANKET_RPL COM_QUERY_BLANKET_RPL;


//---------------------------------------------------------------------------
// Copy Proxy Service
//---------------------------------------------------------------------------


struct tagCOM_COPY_PROXY_REQ
{
    MSG_HEADER h;
    ULONG objidx;
};

struct tagCOM_COPY_PROXY_RPL
{
    MSG_HEADER h;                       // status is RPC exception
    HRESULT hr;
    ULONG objidx;
};

typedef struct tagCOM_COPY_PROXY_REQ COM_COPY_PROXY_REQ;
typedef struct tagCOM_COPY_PROXY_RPL COM_COPY_PROXY_RPL;


//---------------------------------------------------------------------------
// Set Security Blanket Service
//---------------------------------------------------------------------------


struct tagCOM_SET_BLANKET_REQ
{
    MSG_HEADER h;
    ULONG objidx;
    ULONG AuthnSvc;
    ULONG AuthzSvc;
    ULONG AuthnLevel;
    ULONG ImpLevel;
    ULONG Capabilities;
    WCHAR ServerPrincName[128];
    BOOLEAN DefaultServerPrincName;
};

struct tagCOM_SET_BLANKET_RPL
{
    MSG_HEADER h;                       // status is RPC exception
    HRESULT hr;
};

typedef struct tagCOM_SET_BLANKET_REQ COM_SET_BLANKET_REQ;
typedef struct tagCOM_SET_BLANKET_RPL COM_SET_BLANKET_RPL;


//---------------------------------------------------------------------------
// Crypt Protect/Unprotect Data Service
//---------------------------------------------------------------------------


struct tagCOM_CRYPT_PROTECT_DATA_REQ
{
    MSG_HEADER h;
    UCHAR mode;
    ULONG flags;
    ULONG data_len;
    ULONG entropy_len;
    ULONG descr_len;
    ULONG prompt_flags;
    ALIGNED ULONG64 prompt_hwnd;
    WCHAR prompt_text[96];
    UCHAR data[1];
    // WCHAR entropy[1];
    // WCHAR descr[1];
};

struct tagCOM_CRYPT_PROTECT_DATA_RPL
{
    MSG_HEADER h;                       // status is win32 error
    ULONG data_len;
    ULONG descr_len;
    UCHAR data[1];
    // WCHAR descr[1];
};

typedef struct tagCOM_CRYPT_PROTECT_DATA_REQ COM_CRYPT_PROTECT_DATA_REQ;
typedef struct tagCOM_CRYPT_PROTECT_DATA_RPL COM_CRYPT_PROTECT_DATA_RPL;



//---------------------------------------------------------------------------


#endif /* _MY_COMWIRE_H */
