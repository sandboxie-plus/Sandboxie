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
// Lock
//---------------------------------------------------------------------------


#ifndef __MY_LOCK_H
#define __MY_LOCK_H


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define LOCK_FREE      0x00000000L

#define LOCK_EXCLUSIVE 0x80000000L


//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------


typedef volatile ULONG LOCK;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


#ifdef __cplusplus
extern "C" {
#endif


void Lock_Exclusive(LOCK *lockword, const WCHAR *LockName);

void Lock_Share(LOCK *lockword, const WCHAR *LockName);

void Lock_Unlock(LOCK *lockword, const WCHAR *LockName);


#ifdef __cplusplus
} // extern "C"
#endif


//---------------------------------------------------------------------------


#endif // __MY_LOCK_H
