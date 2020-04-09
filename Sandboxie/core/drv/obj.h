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
// Handling NT Objects
//---------------------------------------------------------------------------


#ifndef _MY_OBJ_H
#define _MY_OBJ_H


#include <ntifs.h>
#include "common/pool.h"
#include "process.h"


//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------


typedef void *(*P_ObGetObjectType)(void *Object);
typedef ULONG (*P_ObQueryNameInfo)(void *Object);


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


BOOLEAN Obj_Init(void);

NTSTATUS Obj_GetName(
    POOL *pool, void *Object,
    OBJECT_NAME_INFORMATION **Name, ULONG *NameLength);

NTSTATUS Obj_GetParseName(
    POOL *pool, void *ParseObject, UNICODE_STRING *RemainingName,
    OBJECT_NAME_INFORMATION **Name, ULONG *NameLength);

NTSTATUS Obj_GetNameOrFileName(
    POOL *pool, void *Object,
    OBJECT_NAME_INFORMATION **Name, ULONG *NameLength);

POBJECT_TYPE Obj_GetTypeObjectType(void);


//---------------------------------------------------------------------------


#ifndef _WIN64


BOOLEAN Obj_HookParseProc(
    const WCHAR *Type, OB_PARSE_METHOD NewFunc, OB_PARSE_METHOD *OldFunc,
    ULONG_PTR *HookEntry);


#endif


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


extern const OBJECT_NAME_INFORMATION Obj_Unnamed;

extern P_ObGetObjectType pObGetObjectType;
extern P_ObQueryNameInfo pObQueryNameInfo;


//---------------------------------------------------------------------------
// Macros Related to ParseProcedure
//---------------------------------------------------------------------------


#define OBJ_PARSE_PROC_ARGS                     \
    PVOID ParseObject,                          \
    PVOID ObjectType,                           \
    PACCESS_STATE AccessState,                  \
    KPROCESSOR_MODE AccessMode,                 \
    ULONG Attributes,                           \
    PUNICODE_STRING CompleteName,               \
    PUNICODE_STRING RemainingName,              \
    PVOID Context,                              \
    PSECURITY_QUALITY_OF_SERVICE SecurityQos,   \
    PVOID *Object


#define CALL_PARSE_PROC(name)                                   \
    name(ParseObject, ObjectType, AccessState,                  \
         AccessMode, Attributes, CompleteName, RemainingName,   \
         Context, SecurityQos, Object)


#define OBJ_PARSE_PROC_ARGS_2                   \
    PROCESS *proc,                              \
    PVOID ParseObject,                          \
    PVOID ObjectType,                           \
    PACCESS_STATE AccessState,                  \
    KPROCESSOR_MODE AccessMode,                 \
    ULONG Attributes,                           \
    PUNICODE_STRING CompleteName,               \
    PUNICODE_STRING RemainingName,              \
    PVOID Context,                              \
    PSECURITY_QUALITY_OF_SERVICE SecurityQos,   \
    PVOID *Object


#define CALL_PARSE_PROC_2(name)                                 \
    name(proc, ParseObject, ObjectType, AccessState,            \
         AccessMode, Attributes, CompleteName, RemainingName,   \
         Context, SecurityQos, Object)


// call the system OB_PARSE_METHOD p, if the status was set to zero

#define OBJ_CALL_SYSTEM_PARSE_PROC(p)                                   \
    if (NT_SUCCESS(status) && p) {                                      \
        status = p(ParseObject, ObjectType, AccessState, AccessMode,    \
                    Attributes, CompleteName, RemainingName, Context,   \
                    SecurityQos, Object);                               \
    }                                                                   \
    return status;


//---------------------------------------------------------------------------


#endif // _MY_OBJ_H
