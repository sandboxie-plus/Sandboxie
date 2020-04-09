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
// GDI+ Dll Private Include
//---------------------------------------------------------------------------

#ifndef GDI_H_INCLUDED
#define GDI_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

typedef UINT (*P_GetEnhMetaFileBits)(
    HENHMETAFILE hemf,
    UINT cbBuffer,
    LPBYTE lpbBuffer
    );

typedef UINT (*P_GetBitmapBits)(
    HBITMAP hBitmap,
    UINT cbBuffer,
    LPBYTE lpbBuffer
    );


typedef BOOL (*P_DeleteObject)(
    HGDIOBJ hObject);

typedef BOOL (*P_DeleteEnhMetaFile)(
    HENHMETAFILE hemf );

extern P_GetEnhMetaFileBits         __sys_GetEnhMetaFileBits;
extern P_GetBitmapBits              __sys_GetBitmapBits;
extern P_DeleteObject               __sys_DeleteObject;
extern P_DeleteEnhMetaFile          __sys_DeleteEnhMetaFile;

#ifdef __cplusplus
}; // extern "C"
#endif

#endif // GDI_H_INCLUDED
