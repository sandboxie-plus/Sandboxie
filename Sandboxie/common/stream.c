/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2021 David Xanatos, xanasoft.com
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
    ULONG encoding;
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

NTSTATUS Stream_Open(
    OUT STREAM **out_stream,
    IN  HANDLE Handle)
{
    STREAM *stream;

    *out_stream = NULL;

    stream = (STREAM*)HeapAlloc(GetProcessHeap(), 0, PAGE_SIZE);
    if (! stream)
        return STATUS_INSUFFICIENT_RESOURCES;

    stream->handle = Handle;

    stream->data_len = 0;
    stream->data_ptr = &stream->data[0];
    stream->encoding = 0;
    *out_stream = stream;

    return STATUS_SUCCESS;
}

#endif

//---------------------------------------------------------------------------
// Stream_Open (kernel mode)
//---------------------------------------------------------------------------

#ifdef KERNEL_MODE

NTSTATUS Stream_Open(
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
    stream->encoding = 0;
    *out_stream = stream;

    return status;
}

#endif

//---------------------------------------------------------------------------
// Stream_Close
//---------------------------------------------------------------------------

void Stream_Close(
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

NTSTATUS Stream_Read_More(
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

NTSTATUS Stream_Flush(
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

//---------------------------------------------------------------------------
// Read_BOM
//---------------------------------------------------------------------------

ULONG Read_BOM(UCHAR** data, ULONG* len)
{
    ULONG encoding;

    if (*len >= 3 && (*data)[0] == 0xEF && (*data)[1] == 0xBB && (*data)[2] == 0xBF) 
    {
        *data += 3;
        *len -= 3;

        encoding = 1;
        //DbgPrint("sbie read ini, found UTF-8 Signature\n");
    }
    else if (*len >= 2 && (*data)[0] == 0xFF && (*data)[1] == 0xFE)
    {
        *data += 2;
        *len -= 2;

        encoding = 0;
        //DbgPrint("sbie read ini, found Unicode (UTF-16 LE) BOM\n");
    }
    else if (*len >= 2 && (*data)[0] == 0xFE && (*data)[1] == 0xFF)
    {
        *data += 2;
        *len -= 2;

        encoding = 2;
        //DbgPrint("sbie read ini, found Unicode (UTF-16 BE) BOM\n");
    }
    else
    {
        // If there is no BOM/Signature try to detect the file type
        // Unicode Litle Endian (windows wchar_t) will have the n*2+1 bytes 0 as long, as no higher unicode chrakters are used
        BOOLEAN LooksUnicodeLE = TRUE;
        // similrly Unicode Big Endian (byte swaped) will have the n*2 bytes 0 as long
        BOOLEAN LooksUnicodeBE = TRUE;
        // UTF-8 shouldn't have null bytes
        for (ULONG pos = 0; (pos + 1) < min(*len, 16); pos += 2) // check first 8 char16's
        {
            if ((*data)[pos] != 0)
                LooksUnicodeBE = FALSE;
            if ((*data)[pos + 1] != 0)
                LooksUnicodeLE = FALSE;
        }

        if (!LooksUnicodeLE && !LooksUnicodeBE)
        {
            encoding = 1;
            //DbgPrint("sbie read ini, looks UTF-8 encoded\n");
        }
        else if (!LooksUnicodeLE && LooksUnicodeBE)
        {
            encoding = 2;
            //DbgPrint("sbie read ini, looks Unicode (UTF-16 BE) encoded\n");
        }
        else
        {
            encoding = 0;
            //if (LooksUnicodeLE && !LooksUnicodeBE)
            //  DbgPrint("sbie read ini, looks Unicode (UTF-16 LE) encoded\n");
            //else
            //  DbgPrint("sbie read ini, encoding looks broken, assuming (UTF-16 LE)\n");
        }
    }

    return encoding;
}

//---------------------------------------------------------------------------
// Stream_Read_BOM
//---------------------------------------------------------------------------

NTSTATUS Stream_Read_BOM(
    IN  STREAM* stream,
    ULONG* encoding)
{
    if (stream->data_len == 0) 
    {
        NTSTATUS status = Stream_Read_More(stream);
        if (!NT_SUCCESS(status))
            return status;
    }

    stream->encoding = Read_BOM(&stream->data_ptr, &stream->data_len);

    if (encoding) *encoding = stream->encoding;

    return STATUS_SUCCESS;
}

//---------------------------------------------------------------------------
// Stream_Read_Wchar
//---------------------------------------------------------------------------

NTSTATUS Stream_Read_Wchar(
    IN  STREAM* stream,
    OUT USHORT* v)
{
    if (stream->encoding == 0) // Unicode Litle Endian
    {
        UCHAR* b = (UCHAR*)v;
        STREAM_GET_BYTE(b[0]);
        STREAM_GET_BYTE(b[1]);
    }
    else if (stream->encoding == 2) // Unicode Big Endian
    {
        UCHAR* b = (UCHAR*)v;
        STREAM_GET_BYTE(b[1]);
        STREAM_GET_BYTE(b[0]);
    }
    else if (stream->encoding == 1) // utf 8
    {
        UCHAR cur_byte;

    read_next:
        STREAM_GET_BYTE(cur_byte);

        //Figure out the current code unit to determine the range. It is split into 6 main groups, each of which handles the data
        //differently from one another.
        if (cur_byte < 0x80) {
            //0..127, the ASCII range.

            //We directly plug in the values to the UTF-16 code unit.
            *v = (wchar_t)(cur_byte);
        }
        else if (cur_byte < 0xC0) {
            //0x80..0xBF, we ignore. These are reserved for UTF-8 encoding.
            goto read_next;
        }
        else if (cur_byte < 0xE0) {
            //128..2047, the extended ASCII range, and into the Basic Multilingual Plane.

            //Work on the first code unit.
            wchar_t highShort = (wchar_t)(cur_byte & 0x1F);
            //Increment the current code unit pointer to the next code unit
            STREAM_GET_BYTE(cur_byte);
            //Work on the second code unit.
            wchar_t lowShort = (wchar_t)(cur_byte & 0x3F);

            //Create the UTF-16 code unit, then increment the iterator
            int unicode = (highShort << 8) | lowShort;

            //Check to make sure the "unicode" is in the range [0..D7FF] and [E000..FFFF].
            if ((0 <= unicode && unicode <= 0xD7FF) || (0xE000 <= unicode && unicode <= 0xFFFF)) {
                //Directly set the value to the UTF-16 code unit.
                *v = (wchar_t)unicode;
            }
        }
        else if (cur_byte < 0xF0) {
            //2048..65535, the remaining Basic Multilingual Plane.

            //Work on the UTF-8 code units one by one.
            //If drawn out, it would be 1110aaaa 10bbbbcc 10ccdddd
            //Where a is 4th byte, b is 3rd byte, c is 2nd byte, and d is 1st byte.
            wchar_t fourthChar = (wchar_t)(cur_byte & 0xF);
            STREAM_GET_BYTE(cur_byte);
            wchar_t thirdChar = (wchar_t)(cur_byte & 0x3C) >> 2;
            wchar_t secondCharHigh = (wchar_t)(cur_byte & 0x3);
            STREAM_GET_BYTE(cur_byte);
            wchar_t secondCharLow = (wchar_t)(cur_byte & 0x30) >> 4;
            wchar_t firstChar = (wchar_t)(cur_byte & 0xF);

            //Create the resulting UTF-16 code unit, then increment the iterator.
            int unicode = (fourthChar << 12) | (thirdChar << 8) | (secondCharHigh << 6) | (secondCharLow << 4) | firstChar;

            //Check to make sure the "unicode" is in the range [0..D7FF] and [E000..FFFF].
            //According to math, UTF-8 encoded "unicode" should always fall within these two ranges.
            if ((0 <= unicode && unicode <= 0xD7FF) || (0xE000 <= unicode && unicode <= 0xFFFF)) {
                //Directly set the value to the UTF-16 code unit.
                *v = (wchar_t)unicode;
            }
        }
        else if (cur_byte < 0xF8) {
            //65536..10FFFF, the Unicode UTF range

            //Work on the UTF-8 code units one by one.
            //If drawn out, it would be 11110abb 10bbcccc 10ddddee 10eeffff
            //Where a is 6th byte, b is 5th byte, c is 4th byte, and so on.
            wchar_t sixthChar = (wchar_t)(cur_byte & 0x4) >> 2;
            wchar_t fifthCharHigh = (wchar_t)(cur_byte & 0x3);
            STREAM_GET_BYTE(cur_byte);
            wchar_t fifthCharLow = (wchar_t)(cur_byte & 0x30) >> 4;
            wchar_t fourthChar = (wchar_t)(cur_byte & 0xF);
            STREAM_GET_BYTE(cur_byte);
            wchar_t thirdChar = (wchar_t)(cur_byte & 0x3C) >> 2;
            wchar_t secondCharHigh = (wchar_t)(cur_byte & 0x3);
            STREAM_GET_BYTE(cur_byte);
            wchar_t secondCharLow = (wchar_t)(cur_byte & 0x30) >> 4;
            wchar_t firstChar = (wchar_t)(cur_byte & 0xF);

            int unicode = (sixthChar << 4) | (fifthCharHigh << 2) | fifthCharLow | (fourthChar << 12) | (thirdChar << 8) | (secondCharHigh << 6) | (secondCharLow << 4) | firstChar;
            wchar_t highSurrogate = (unicode - 0x10000) / 0x400 + 0xD800;
            wchar_t lowSurrogate = (unicode - 0x10000) % 0x400 + 0xDC00;

            //Set the UTF-16 code units
            //*v1 = lowSurrogate;
            //*v2 = highSurrogate;

            *v = L'_';
        }
        else {
            goto read_next;
        }
    }
    else
        return STATUS_INVALID_PARAMETER;

    return STATUS_SUCCESS;
}