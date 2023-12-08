/*
ImDisk Proxy Services.

Copyright (C) 2005-2007 Olof Lagerkvist.

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _INC_IMDPROXY_
#define _INC_IMDPROXY_

#if !defined(_WIN32) && !defined(_NTDDK_)
typedef int32_t LONG;
typedef uint32_t ULONG;
typedef int64_t LONGLONG;
typedef uint64_t ULONGLONG;
typedef u_short WCHAR;
#endif

#define IMDPROXY_SVC                    L"ImDskSvc"
#define IMDPROXY_SVC_PIPE_DOSDEV_NAME   L"\\\\.\\PIPE\\" IMDPROXY_SVC
#define IMDPROXY_SVC_PIPE_NATIVE_NAME   L"\\Device\\NamedPipe\\" IMDPROXY_SVC

#define IMDPROXY_FLAG_RO                0x01 // Read-only
#define IMDPROXY_FLAG_SUPPORTS_UNMAP    0x02 // Unmap/TRIM ranges
#define IMDPROXY_FLAG_SUPPORTS_ZERO     0x04 // Zero-fill ranges
#define IMDPROXY_FLAG_SUPPORTS_SCSI     0x08 // SCSI SRB operations
#define IMDPROXY_FLAG_SUPPORTS_SHARED   0x10 // Shared image access with reservations

typedef enum _IMDPROXY_REQ
{
    IMDPROXY_REQ_NULL,
    IMDPROXY_REQ_INFO,
    IMDPROXY_REQ_READ,
    IMDPROXY_REQ_WRITE,
    IMDPROXY_REQ_CONNECT,
    IMDPROXY_REQ_CLOSE,
    IMDPROXY_REQ_UNMAP,
    IMDPROXY_REQ_ZERO,
    IMDPROXY_REQ_SCSI,
    IMDPROXY_REQ_SHARED
} IMDPROXY_REQ, *PIMDPROXY_REQ;

typedef struct _IMDPROXY_CONNECT_REQ
{
    ULONGLONG request_code;
    ULONGLONG flags;
    ULONGLONG length;
} IMDPROXY_CONNECT_REQ, *PIMDPROXY_CONNECT_REQ;

typedef struct _IMDPROXY_CONNECT_RESP
{
    ULONGLONG error_code;
    ULONGLONG object_ptr;
} IMDPROXY_CONNECT_RESP, *PIMDPROXY_CONNECT_RESP;

typedef struct _IMDPROXY_INFO_RESP
{
    ULONGLONG file_size;
    ULONGLONG req_alignment;
    ULONGLONG flags;
} IMDPROXY_INFO_RESP, *PIMDPROXY_INFO_RESP;

typedef struct _IMDPROXY_READ_REQ
{
    ULONGLONG request_code;
    ULONGLONG offset;
    ULONGLONG length;
} IMDPROXY_READ_REQ, *PIMDPROXY_READ_REQ;

typedef struct _IMDPROXY_READ_RESP
{
    ULONGLONG errorno;
    ULONGLONG length;
} IMDPROXY_READ_RESP, *PIMDPROXY_READ_RESP;

typedef struct _IMDPROXY_WRITE_REQ
{
    ULONGLONG request_code;
    ULONGLONG offset;
    ULONGLONG length;
} IMDPROXY_WRITE_REQ, *PIMDPROXY_WRITE_REQ;

typedef struct _IMDPROXY_WRITE_RESP
{
    ULONGLONG errorno;
    ULONGLONG length;
} IMDPROXY_WRITE_RESP, *PIMDPROXY_WRITE_RESP;

typedef struct _IMDPROXY_UNMAP_REQ
{
    ULONGLONG request_code;
    ULONGLONG length;
} IMDPROXY_UNMAP_REQ, *PIMDPROXY_UNMAP_REQ;

typedef struct _IMDPROXY_UNMAP_RESP
{
    ULONGLONG errorno;
} IMDPROXY_UNMAP_RESP, *PIMDPROXY_UNMAP_RESP;

typedef struct _IMDPROXY_ZERO_REQ
{
    ULONGLONG request_code;
    ULONGLONG length;
} IMDPROXY_ZERO_REQ, *PIMDPROXY_ZERO_REQ;

typedef struct _IMDPROXY_ZERO_RESP
{
    ULONGLONG errorno;
} IMDPROXY_ZERO_RESP, *PIMDPROXY_ZERO_RESP;

typedef struct _IMDPROXY_SCSI_REQ
{
    ULONGLONG request_code;
    UCHAR cdb[16];
    ULONGLONG request_length;
    ULONGLONG max_response_length;
} IMDPROXY_SCSI_REQ, *PIMDPROXY_SCSI_REQ;

typedef struct _IMDPROXY_SCSI_RESP
{
    ULONGLONG errorno;
    ULONGLONG length;
} IMDPROXY_SCSI_RESP, *PIMDPROXY_SCSI_RESP;

typedef struct _IMDPROXY_SHARED_REQ
{
    ULONGLONG request_code;
    ULONGLONG operation_code;
    ULONGLONG reserve_scope;
    ULONGLONG reserve_type;
    ULONGLONG existing_reservation_key;
    ULONGLONG current_channel_key;
    ULONGLONG operation_channel_key;
} IMDPROXY_SHARED_REQ, *PIMDPROXY_SHARED_REQ;

typedef struct _IMDPROXY_SHARED_RESP
{
    ULONGLONG errorno;
    UCHAR unique_id[16];
    ULONGLONG channel_key;
    ULONGLONG reservation_key;
    ULONGLONG reservation_scope;
    ULONGLONG reservation_type;
    ULONGLONG length;
} IMDPROXY_SHARED_RESP, *PIMDPROXY_SHARED_RESP;

#define IMDPROXY_RESERVATION_KEY_ANY MAXULONGLONG

typedef enum _IMDPROXY_SHARED_OP_CODE
{
    GetUniqueId,
    ReadKeys,
    Register,
    ClearKeys,
    Reserve,
    Release,
    Preempt
} IMDPROXY_SHARED_OP_CODE, *PIMDPROXY_SHARED_OP_CODE;

typedef enum _IMDPROXY_SHARED_RESP_CODE
{
    NoError,
    ReservationCollision,
    InvalidParameter,
    IOError
} IMDPROXY_SHARED_RESP_CODE, *PIMDPROXY_SHARED_RESP_CODE;

// For shared memory proxy communication only. Offset to data area in
// shared memory.
#define IMDPROXY_HEADER_SIZE 4096

#endif // _INC_IMDPROXY_
