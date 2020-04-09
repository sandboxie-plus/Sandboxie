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
// Memory management
//---------------------------------------------------------------------------


#ifndef _MY_MEM_H
#define _MY_MEM_H


//---------------------------------------------------------------------------
// Debug
//---------------------------------------------------------------------------


#undef  DEBUG_MEMORY


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


void *Mem_AllocEx(POOL *pool, ULONG size, BOOLEAN InitMsg);

#define Mem_Alloc(pool,size) Mem_AllocEx((pool),(size),FALSE)

#ifdef DEBUG_MEMORY

void Mem_Free(void *ptr, ULONG size);

#else ! DEBUG_MEMORY

#define Mem_Free Pool_Free

#endif DEBUG_MEMORY


//---------------------------------------------------------------------------


WCHAR *Mem_AllocStringEx(
    POOL *pool, const WCHAR *model_string, BOOLEAN InitMsg);

#define Mem_AllocString(pool,model) Mem_AllocStringEx((pool),(model),FALSE)

void Mem_FreeString(WCHAR *string);


//---------------------------------------------------------------------------


BOOLEAN Mem_GetLockResource(PERESOURCE *ppResource, BOOLEAN InitMsg);

void Mem_FreeLockResource(PERESOURCE *ppResource);


//---------------------------------------------------------------------------


#endif // _MY_MEM_H
