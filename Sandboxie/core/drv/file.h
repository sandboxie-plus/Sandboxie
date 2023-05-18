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
// File system
//---------------------------------------------------------------------------


#ifndef _MY_FILE_H
#define _MY_FILE_H


#include "driver.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


BOOLEAN File_Init(void);

void File_Unload(void);

NTSTATUS File_TranslateDosToNt(
    const WCHAR *dos_path, POOL *pool, WCHAR **out_path, ULONG *out_len);

WCHAR *File_TranslateReparsePoints(const WCHAR *path, POOL *pool);

BOOLEAN File_CreateBoxPath(PROCESS *proc);

BOOLEAN File_InitProcess(PROCESS *proc);


//---------------------------------------------------------------------------


NTSTATUS File_Api_Rename(PROCESS *proc, ULONG64 *parms);

NTSTATUS File_Api_GetName(PROCESS *proc, ULONG64 *parms);

NTSTATUS File_Api_RefreshPathList(PROCESS *proc, ULONG64 *parms);

NTSTATUS File_Api_Open(PROCESS *proc, ULONG64 *parms);

NTSTATUS File_Api_CheckInternetAccess(PROCESS *proc, ULONG64 *parms);

NTSTATUS File_Api_GetBlockedDll(PROCESS *proc, ULONG64 *parms);

NTSTATUS File_Api_ProtectRoot(PROCESS *proc, ULONG64 *parms);

NTSTATUS File_Api_UnprotectRoot(PROCESS *proc, ULONG64 *parms);


//---------------------------------------------------------------------------


#endif // _MY_FILE_H
