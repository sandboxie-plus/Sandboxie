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
// DLL Management
//---------------------------------------------------------------------------


#ifndef _MY_DLL_H
#define _MY_DLL_H


#include "driver.h"


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


struct _DLL_ENTRY;
typedef struct _DLL_ENTRY DLL_ENTRY;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


BOOLEAN Dll_Init(void);

void Dll_Unload(void);

DLL_ENTRY *Dll_Load(const WCHAR *DllBaseName);

void *Dll_RvaToAddr(DLL_ENTRY *dll, ULONG rva);

void *Dll_GetProc(
    const WCHAR *DllName, const UCHAR *ProcName, BOOLEAN returnOffset);

ULONG Dll_GetNextProc(
    DLL_ENTRY *dll, const UCHAR *SearchName,
    UCHAR **FoundName, ULONG *FoundIndex);


//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


extern const WCHAR *Dll_NTDLL;
#ifdef XP_SUPPORT
extern const WCHAR *Dll_USER;
#endif

//---------------------------------------------------------------------------


#endif // _MY_DLL_H
