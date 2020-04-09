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
// SbieIni scripting utility
//---------------------------------------------------------------------------


#include <windows.h>
#include <stdio.h>


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define ERRLVL_CMDLINE 1


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _SECTION_NAME
{
    struct _SECTION_NAME *next;
    WCHAR name[1];
} SECTION_NAME;


//---------------------------------------------------------------------------
// Cmd
//---------------------------------------------------------------------------


void CmdParse(void);
BOOL CmdIs(const WCHAR *val);
WCHAR *CmdVerb(ULONG index);
WCHAR *CmdOpt(const WCHAR *name);


//---------------------------------------------------------------------------
// Main
//---------------------------------------------------------------------------


void UsageError(const WCHAR *text);


//---------------------------------------------------------------------------
// Query
//---------------------------------------------------------------------------


int DoQuery(void);


//---------------------------------------------------------------------------
// Update
//---------------------------------------------------------------------------


int DoUpdate(void);
