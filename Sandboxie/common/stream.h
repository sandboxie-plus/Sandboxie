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

#ifndef _MY_STREAM_H
#define _MY_STREAM_H

//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------

typedef struct STREAM STREAM;

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

#ifndef KERNEL_MODE

NTSTATUS Stream_Open(
    OUT STREAM **out_stream,
    IN  HANDLE FileHandle);

#else   // kernel mode stream

NTSTATUS Stream_Open(
    OUT STREAM **out_stream,
    IN  const WCHAR *FullPath,
    IN  ACCESS_MASK DesiredAccess,
    IN  ULONG FileAttributes,
    IN  ULONG ShareAccess,
    IN  ULONG CreateDisposition,
    IN  ULONG CreateOptions);

#endif

void Stream_Close(
    IN  STREAM *stream);

NTSTATUS Stream_Flush(
    IN  STREAM *stream);

NTSTATUS Stream_Read_Bytes(
    IN  STREAM *stream,
    IN  ULONG len,
    OUT UCHAR *v);

NTSTATUS Stream_Write_Bytes(
    IN  STREAM *stream,
    IN  ULONG len,
    IN  UCHAR *v);

NTSTATUS Stream_Read_Short(
    IN  STREAM *stream,
    OUT USHORT *v);

NTSTATUS Stream_Write_Short(
    IN  STREAM *stream,
    IN  USHORT v);

NTSTATUS Stream_Read_Long(
    IN  STREAM *stream,
    OUT ULONG *v);

NTSTATUS Stream_Write_Long(
    IN  STREAM *stream,
    IN  ULONG v);

//---------------------------------------------------------------------------

#endif // _MY_STREAM_H
