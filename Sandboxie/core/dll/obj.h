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
// NtQueryObject
//---------------------------------------------------------------------------


#ifndef _MY_OBJ_H
#define _MY_OBJ_H


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


enum {

    OBJ_TYPE_UNKNOWN,
    OBJ_TYPE_DIRECTORY,
    OBJ_TYPE_FILE,
    OBJ_TYPE_KEY,
    OBJ_TYPE_PORT,
    OBJ_TYPE_EVENT,
    OBJ_TYPE_MUTANT,
    OBJ_TYPE_SECTION,
    OBJ_TYPE_SEMAPHORE,
    OBJ_TYPE_PROCESS,
    OBJ_TYPE_LAST

} OBJ_TYPE;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


BOOLEAN Obj_Init(void);

ULONG Obj_GetObjectType(HANDLE ObjectHandle);

NTSTATUS Obj_GetObjectName(
    HANDLE ObjectHandle, void *ObjectName, ULONG *Length);


//---------------------------------------------------------------------------


ULONG File_NtQueryObjectName(UNICODE_STRING *ObjectName, ULONG MaxLen);

ULONG Key_NtQueryObjectName(UNICODE_STRING *ObjectName, ULONG MaxLen);

ULONG Ipc_NtQueryObjectName(UNICODE_STRING *ObjectName, ULONG MaxLen);


//---------------------------------------------------------------------------


#endif /* _MY_OBJ_H */
