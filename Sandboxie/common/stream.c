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
// File Stream Support
//---------------------------------------------------------------------------

#ifndef KERNEL_MODE
#include "win32_ntddk.h"
#endif
#include "stream.h"
#include "defines.h"

//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------

#pragma pack(push,1)

struct STREAM {
    HANDLE handle;
    UCHAR *data_ptr;
    ULONG data_len;
    __declspec(align(8)) UCHAR data[0];
};

#pragma pack(pop)

#define STREAM_DATA_SIZE (PAGE_SIZE - sizeof(STREAM) - 8)

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

NTSTATUS Stream_Read_More(
    IN  STREAM *stream);

//---------------------------------------------------------------------------
// Stream_Open (user mode)
//---------------------------------------------------------------------------

#ifndef KERNEL_MODE

__declspec(align(16)) NTSTATUS Stream_Open(
    OUT STREAM **out_stream,
    IN  HANDLE Handle)
{
    STREAM *stream;

    *out_stream = NULL;

    stream = HeapAlloc(GetProcessHeap(), 0, PAGE_SIZE);
    if (! stream)
        return STATUS_INSUFFICIENT_RESOURCES;

    stream->handle = Handle;

    stream->data_len = 0;
    stream->data_ptr = &stream->data[0];
    *out_stream = stream;

    return STATUS_SUCCESS;
}

#endif

//---------------------------------------------------------------------------
// Stream_Open (kernel mode)
//---------------------------------------------------------------------------

#ifdef KERNEL_MODE

__declspec(align(16)) NTSTATUS Stream_Open(
    OUT STREAM **out_stream,
    IN  const WCHAR *FullPath,
    IN  ACCESS_MASK DesiredAccess,
    IN  ULONG FileAttributes,
    IN  ULONG ShareAccess,
    IN  ULONG CreateDisposition,
    IN  ULONG CreateOptions)
{
    NTSTATUS status;
    UNICODE_STRING uni;
    OBJECT_ATTRIBUTES objattrs;
    IO_STATUS_BLOCK MyIoStatusBlock;
    STREAM *stream;
    ULONG retries;

    *out_stream = NULL;

    stream = ExAllocatePoolWithTag(PagedPool, PAGE_SIZE, tzuk);
    if (! stream)
        return STATUS_INSUFFICIENT_RESOURCES;

    RtlInitUnicodeString(&uni, FullPath);
    InitializeObjectAttributes(&objattrs,
        &uni, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);

    for (retries = 0; retries < 10; ++retries) {

        status = IoCreateFile(
            &stream->handle,
            DesiredAccess | SYNCHRONIZE,
            &objattrs,
            &MyIoStatusBlock,
            NULL,                           // AllocationSize
            FileAttributes,
            ShareAccess,
            CreateDisposition,
            CreateOptions | FILE_SYNCHRONOUS_IO_NONALERT,
            NULL, 0,                        // EaBuffer, EaLength
            CreateFileTypeNone,             // CreateFileType
            NULL,                           // ExtraCreateParameters
            IO_NO_PARAMETER_CHECKING | IO_FORCE_ACCESS_CHECK);

        // we could get STATUS_USER_MAPPED_FILE (why??), in which
        // case we suspend for a short while, then retry

        if (status == STATUS_USER_MAPPED_FILE) {
            LARGE_INTEGER time;
            time.QuadPart = -5000000L;
            KeDelayExecutionThread(KernelMode, FALSE, &time);
        } else
            break;
    }

    if (! NT_SUCCESS(status)) {
        ExFreePoolWithTag(stream, tzuk);
        return status;
    }

    stream->data_len = 0;
    stream->data_ptr = &stream->data[0];
    *out_stream = stream;

    return status;
}

#endif

//---------------------------------------------------------------------------
// Stream_Close
//---------------------------------------------------------------------------

__declspec(align(16)) void Stream_Close(
    IN  STREAM *stream)
{
#ifndef KERNEL_MODE
    NtClose(stream->handle);
    HeapFree(GetProcessHeap(), 0, stream);
#else   // kernel mode stream
    ZwClose(stream->handle);
    ExFreePoolWithTag(stream, tzuk);
#endif
}

//---------------------------------------------------------------------------
// Stream_Read_More
//---------------------------------------------------------------------------

__declspec(align(16)) NTSTATUS Stream_Read_More(
    IN  STREAM *stream)
{
    NTSTATUS status;
    IO_STATUS_BLOCK MyIoStatusBlock;

#ifndef KERNEL_MODE
    status = NtReadFile(
#else
    status = ZwReadFile(
#endif
        stream->handle, NULL, NULL, NULL, &MyIoStatusBlock,
        &stream->data[0], STREAM_DATA_SIZE, NULL, NULL);

    if (NT_SUCCESS(status))
        stream->data_len = (ULONG)MyIoStatusBlock.Information;
    else
        stream->data_len = 0;

    stream->data_ptr = &stream->data[0];

    if (NT_SUCCESS(status) && stream->data_len == 0)
        status = STATUS_END_OF_FILE;

    return status;
}

//---------------------------------------------------------------------------
// Stream_Flush
//---------------------------------------------------------------------------

__declspec(align(16)) NTSTATUS Stream_Flush(
    IN  STREAM *stream)
{
    NTSTATUS status;
    IO_STATUS_BLOCK MyIoStatusBlock;

    if (stream->data_len != 0) {

#ifndef KERNEL_MODE
        status = NtWriteFile(
#else
        status = ZwWriteFile(
#endif
            stream->handle, NULL, NULL, NULL, &MyIoStatusBlock,
            &stream->data[0], stream->data_len, NULL, NULL);

        if (NT_SUCCESS(status) &&
            MyIoStatusBlock.Information != stream->data_len)
            status = STATUS_DISK_FULL;

        stream->data_len = 0;
        stream->data_ptr = &stream->data[0];

    } else
        status = STATUS_SUCCESS;

    return status;
}

//---------------------------------------------------------------------------
// Byte-oriented Access Macros
//---------------------------------------------------------------------------

#define STREAM_GET_BYTE(b) {                            \
    if (stream->data_len == 0) {                        \
        NTSTATUS status = Stream_Read_More(stream);     \
        if (! NT_SUCCESS(status))                       \
            return status;                              \
    }                                                   \
    (b) = *stream->data_ptr;                            \
    ++stream->data_ptr;                                 \
    --stream->data_len;                                 \
    }

#define STREAM_PUT_BYTE(b) {                            \
    if (stream->data_len == STREAM_DATA_SIZE) {         \
        NTSTATUS status = Stream_Flush(stream);         \
        if (! NT_SUCCESS(status))                       \
            return status;                              \
    }                                                   \
    *stream->data_ptr = (b);                            \
    ++stream->data_ptr;                                 \
    ++stream->data_len;                                 \
    }

//---------------------------------------------------------------------------
// Stream_Read_Bytes
//---------------------------------------------------------------------------

NTSTATUS Stream_Read_Bytes(
    IN  STREAM *stream,
    IN  ULONG len,
    OUT UCHAR *v)
{
    while (len) {
        STREAM_GET_BYTE(*v);
        ++v;
        --len;
    }
    return STATUS_SUCCESS;
}

//---------------------------------------------------------------------------
// Stream_Write_Bytes
//---------------------------------------------------------------------------

NTSTATUS Stream_Write_Bytes(
    IN  STREAM *stream,
    IN  ULONG len,
    OUT UCHAR *v)
{
    while (len) {
        STREAM_PUT_BYTE(*v);
        ++v;
        --len;
    }
    return STATUS_SUCCESS;
}

//---------------------------------------------------------------------------
// Stream_Read_Short
//---------------------------------------------------------------------------

NTSTATUS Stream_Read_Short(
    IN  STREAM *stream,
    OUT USHORT *v)
{
    UCHAR *b = (UCHAR *)v;
    STREAM_GET_BYTE(b[0]);
    STREAM_GET_BYTE(b[1]);
    return STATUS_SUCCESS;
}

//---------------------------------------------------------------------------
// Stream_Write_Short
//---------------------------------------------------------------------------

NTSTATUS Stream_Write_Short(
    IN  STREAM *stream,
    IN  USHORT v)
{
    UCHAR *b = (UCHAR *)&v;
    STREAM_PUT_BYTE(b[0]);
    STREAM_PUT_BYTE(b[1]);
    return STATUS_SUCCESS;
}

//---------------------------------------------------------------------------
// Stream_Read_Long
//---------------------------------------------------------------------------

NTSTATUS Stream_Read_Long(
    IN  STREAM *stream,
    OUT ULONG *v)
{
    UCHAR *b = (UCHAR *)v;
    STREAM_GET_BYTE(b[0]);
    STREAM_GET_BYTE(b[1]);
    STREAM_GET_BYTE(b[2]);
    STREAM_GET_BYTE(b[3]);
    return STATUS_SUCCESS;
}

//---------------------------------------------------------------------------
// Stream_Write_Long
//---------------------------------------------------------------------------

NTSTATUS Stream_Write_Long(
    IN  STREAM *stream,
    IN  ULONG v)
{
    UCHAR *b = (UCHAR *)&v;
    STREAM_PUT_BYTE(b[0]);
    STREAM_PUT_BYTE(b[1]);
    STREAM_PUT_BYTE(b[2]);
    STREAM_PUT_BYTE(b[3]);
    return STATUS_SUCCESS;
}
