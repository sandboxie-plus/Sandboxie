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
// Handling NT Objects:  32-bit Code for Windows XP
//---------------------------------------------------------------------------


#include "process.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN Obj_HookAnyProc(
    const WCHAR *TypeName, ULONG ProcOffset, ULONG NumArgs,
    ULONG_PTR NewProc, ULONG_PTR *OldProc,
    ULONG *IncPtr, ULONG_PTR *pHookEntry);


//---------------------------------------------------------------------------


void *_AddressOfReturnAddress(void);
#pragma intrinsic(_AddressOfReturnAddress)


//---------------------------------------------------------------------------


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, Obj_HookParseProc)
#pragma alloc_text (INIT, Obj_HookAnyProc)
#endif // ALLOC_PRAGMA


//---------------------------------------------------------------------------
// Obj_HookParseProc
//---------------------------------------------------------------------------


_FX BOOLEAN Obj_HookParseProc(
    const WCHAR *Type, OB_PARSE_METHOD NewFunc, OB_PARSE_METHOD *OldFunc,
    ULONG_PTR *HookEntry)
{
    ULONG ProcOffset;

    ProcOffset = FIELD_OFFSET(OBJECT_TYPE_INITIALIZER, ParseProcedure);
    /*
    if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA && Driver_OsBuild > 6000) {
        // Vista SP1 introduces changes to object type structures
        ProcOffset =
            FIELD_OFFSET(OBJECT_TYPE_INITIALIZER_VISTA_SP1, ParseProcedure);
    }*/

    return Obj_HookAnyProc(
        Type, ProcOffset, 10,
        (ULONG_PTR)NewFunc, (ULONG_PTR *)OldFunc, NULL, HookEntry);
}


//---------------------------------------------------------------------------
// Obj_HookAnyProc
//---------------------------------------------------------------------------


_FX BOOLEAN Obj_HookAnyProc(
    const WCHAR *TypeName, ULONG ProcOffset, ULONG NumArgs,
    ULONG_PTR NewProc, ULONG_PTR *OldProc,
    ULONG *IncPtr, ULONG_PTR *pHookEntry)
{
    NTSTATUS status;
    WCHAR ObjectName[64];
    UNICODE_STRING uni;
    OBJECT_ATTRIBUTES objattrs;
    HANDLE handle;
    OBJECT_TYPE *object;
    ULONG_PTR HookEntry;
    ULONG_PTR ProcPtr;

    //
    // hook object type procedure
    //

    wcscpy(ObjectName, L"\\ObjectTypes\\");
    wcscat(ObjectName, TypeName);
    RtlInitUnicodeString(&uni, ObjectName);
    InitializeObjectAttributes(&objattrs,
        &uni, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

    status = ObOpenObjectByName(
                    &objattrs, NULL, KernelMode, NULL, 0, NULL, &handle);
    if (! NT_SUCCESS(status)) {
        Log_Status_Ex(MSG_OBJ_HOOK_ANY_PROC, 0x11, status, TypeName);
        return FALSE;
    }

    status = ObReferenceObjectByHandle(
        handle, 0, NULL, KernelMode, &object, NULL);

    if (! NT_SUCCESS(status))
        Log_Status_Ex(MSG_OBJ_HOOK_ANY_PROC, 0x22, status, TypeName);

    else {

        ProcPtr = (ULONG_PTR)(((UCHAR *)&object->TypeInfo) + ProcOffset);

        *OldProc = *(ULONG_PTR *)ProcPtr;

        //
        // create a hook entry which will jump to OldProc or NewProc.
        // if OldProc is null, we pass the number of arguments it would
        // take, so the hook entry can handle that case
        //

        HookEntry = Process_BuildHookEntry(
            NewProc, (*OldProc ? *OldProc : NumArgs), IncPtr);

        if (! HookEntry) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            Log_Status_Ex(MSG_OBJ_HOOK_ANY_PROC, 0x33, status, TypeName);

        } else {

            *pHookEntry = HookEntry;

            KeMemoryBarrier();

            InterlockedExchangePointer((void **)ProcPtr, (void *)HookEntry);
        }

        ObDereferenceObject(object);
    }

    ZwClose(handle);

    return (NT_SUCCESS(status));
}
