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


#include "obj.h"
#include "conf.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define PAD_LEN         (4 * sizeof(WCHAR))


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, Obj_Init)
#endif // ALLOC_PRAGMA


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static const WCHAR Obj_Unnamed_Name = L'\0';

const OBJECT_NAME_INFORMATION Obj_Unnamed = {
    { 0, 0, (WCHAR *)&Obj_Unnamed_Name } };


P_ObGetObjectType pObGetObjectType = NULL;
P_ObQueryNameInfo pObQueryNameInfo = NULL;

//---------------------------------------------------------------------------
// Include code for minifilter
//---------------------------------------------------------------------------


#include "obj_flt.c"

//---------------------------------------------------------------------------
// Include code for 32-bit Windows XP
//---------------------------------------------------------------------------


#ifdef XP_SUPPORT
#ifndef _WIN64
#include "obj_xp.c"
#endif _WIN64
#endif


//---------------------------------------------------------------------------
// Obj_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Obj_Init(void)
{
    //
    // register as a object filter on Vista SP1 and later
    //

    //if (Driver_OsVersion > DRIVER_WINDOWS_VISTA) {

    //    if (!Obj_Init_Filter())
    //        return FALSE;
    //}

    if (Driver_OsVersion >= DRIVER_WINDOWS_7) {

        //
        // get ObQueryNameInfo and ObGetObjectType
        //

        UNICODE_STRING uni;
        void *ptr;

        RtlInitUnicodeString(&uni, L"ObQueryNameInfo");
        ptr = MmGetSystemRoutineAddress(&uni);
        if (! ptr) {
            Log_Msg1(MSG_DLL_GET_PROC, uni.Buffer);
            return FALSE;
        }

        pObQueryNameInfo = (P_ObQueryNameInfo)ptr;

        RtlInitUnicodeString(&uni, L"ObGetObjectType");
        ptr = MmGetSystemRoutineAddress(&uni);
        if (! ptr) {
            Log_Msg1(MSG_DLL_GET_PROC, uni.Buffer);
            return FALSE;
        }

        pObGetObjectType = (P_ObGetObjectType)ptr;
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Obj_Unload
//---------------------------------------------------------------------------


_FX void Obj_Unload(void)
{
    //
    // deregister as a object filter on Vista SP1 and later
    //

    //if (Driver_OsVersion > DRIVER_WINDOWS_VISTA) {

    //    Obj_Unload_Filter();
    //}
}


//---------------------------------------------------------------------------
// Obj_GetName
//---------------------------------------------------------------------------


_FX NTSTATUS Obj_GetName(
    POOL *pool, void *Object,
    OBJECT_NAME_INFORMATION **Name, ULONG *NameLength)
{
    NTSTATUS status;
    UCHAR small_info_space[80];
    OBJECT_NAME_INFORMATION *small_info;
    ULONG len;
    OBJECT_NAME_INFORMATION *info;
    ULONG info_len;

    *Name = NULL;
    *NameLength = 0;

    //
    // invoke ObQueryNameString on the small buffer.  the small buffer
    // must be larger than sizeof(OBJECT_NAME_INFORMATION) even after
    // subtracting PAD_LEN bytes from it.  in other words, keep the
    // small buffer at least around 32 bytes (depending on PAD_LEN)
    //
    // sometimes ObQueryNameString gets confused and returns
    // STATUS_OBJECT_PATH_INVALID, in this case we just call with
    // a slightly smaller buffer
    //

    memzero(small_info_space, sizeof(small_info_space));
    small_info = (OBJECT_NAME_INFORMATION *)small_info_space;

    len = 0;        // must be initialized
    status = ObQueryNameString(
        Object, small_info, sizeof(small_info_space) - PAD_LEN, &len);

    if (status == STATUS_OBJECT_PATH_INVALID) {

        len = 0;        // must be initialized
        status = ObQueryNameString(
            Object, small_info,
            sizeof(small_info_space) - PAD_LEN * 2, &len);
    }

    if (NT_SUCCESS(status)) {

        //
        // we got the name completely into the small buffer, so we
        // allocate a pool buffer and copy the name string
        //

        if (small_info->Name.Length && small_info->Name.Buffer) {

            info_len = sizeof(OBJECT_NAME_INFORMATION)
                     + small_info->Name.Length + PAD_LEN * 2;
            info = (OBJECT_NAME_INFORMATION *)Mem_Alloc(pool, info_len);
            if (! info)
                return STATUS_INSUFFICIENT_RESOURCES;

            memzero(info, info_len);
            info->Name.Length        = small_info->Name.Length;
            info->Name.MaximumLength = small_info->Name.MaximumLength;
            info->Name.Buffer        =
                (WCHAR *)(((UCHAR *)info) + sizeof(UNICODE_STRING));
            memcpy(info->Name.Buffer,
                   small_info->Name.Buffer,
                   small_info->Name.Length);

        } else {

            info = NULL;
            info_len = 0;
        }

        goto finish;
    }

    if (status != STATUS_INFO_LENGTH_MISMATCH &&
        status != STATUS_BUFFER_OVERFLOW) {

        return status;
    }

    //
    // on Windows 2000, we may get STATUS_INFO_LENGTH_MISMATCH but
    // a result length of zero, in this case we must try again
    // with a larger buffer
    //

    info = NULL;
    info_len = 0;

    while (! len) {

        if (info)
            Mem_Free(info, info_len);
        info_len += 128;
        info = (OBJECT_NAME_INFORMATION *)Mem_Alloc(pool, info_len);
        if (! info)
            return STATUS_INSUFFICIENT_RESOURCES;

        memzero(info, info_len);
        len = 0;        // must be initialized
        status = ObQueryNameString(
            Object, info, info_len - PAD_LEN, &len);

        if (NT_SUCCESS(status))
            break;

        if (status == STATUS_OBJECT_PATH_INVALID) {

            len = 0;        // must be initialized
            status = ObQueryNameString(
                Object, info, info_len - PAD_LEN * 2, &len);
        }

        if (status != STATUS_INFO_LENGTH_MISMATCH &&
            status != STATUS_BUFFER_OVERFLOW) {

            Mem_Free(info, info_len);
            return status;
        }

        len = 0;
    }

    //
    // on Windows XP, we should have gotten a result length, and not
    // went into the loop above, so info is still NULL, and we need
    // to allocate it and query the name again
    //

    if (! info) {

        info_len = len + PAD_LEN * 2;
        info = (OBJECT_NAME_INFORMATION *)Mem_Alloc(pool, info_len);
        if (! info)
            return STATUS_INSUFFICIENT_RESOURCES;

        memzero(info, info_len);
        len = 0;        // must be initialized
        status = ObQueryNameString(
            Object, info, info_len - PAD_LEN, &len);

        if (! NT_SUCCESS(status)) {
            Mem_Free(info, info_len);
            return status;
        }
    }

    //
    // finally we only have to make sure that the name isn't empty
    //

finish:

    if (info && info->Name.Length && info->Name.Buffer) {

        //
        // On Windows 7, we may get two leading backslashes
        //

        if (Driver_OsVersion >= DRIVER_WINDOWS_7 &&
                info->Name.Length >= 2 * sizeof(WCHAR) &&
                info->Name.Buffer[0] == L'\\' &&
                info->Name.Buffer[1] == L'\\') {

            WCHAR *Buffer = info->Name.Buffer;
            USHORT Length = info->Name.Length;

            Length = Length / sizeof(WCHAR) - 1;
            wmemmove(Buffer, Buffer + 1, Length);
            Buffer[Length] = L'\0';

            info->Name.Length -= sizeof(WCHAR);
            info->Name.MaximumLength -= sizeof(WCHAR);
        }

        *Name = info;
        *NameLength = info_len;

    } else {

        if (info)
            Mem_Free(info, info_len);

        *Name = (OBJECT_NAME_INFORMATION *)&Obj_Unnamed;
        *NameLength = 0;
    }

    /*if (0) {
        OBJECT_NAME_INFORMATION *xname = *Name;
        WCHAR *xbuf = xname->Name.Buffer;
        ULONG xlen  = xname->Name.Length / sizeof(WCHAR);
        DbgPrint("Object Name: %*.*S\n", xlen, xlen, xbuf);
    }*/

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Obj_GetParseName
//---------------------------------------------------------------------------


_FX NTSTATUS Obj_GetParseName(
    POOL *pool, void *ParseObject, UNICODE_STRING *RemainingName,
    OBJECT_NAME_INFORMATION **Name, ULONG *NameLength)
{
    NTSTATUS status;
    OBJECT_NAME_INFORMATION *oldname, *newname;
    ULONG oldname_len, newname_len;
    WCHAR *ptr;

    status = Obj_GetName(pool, ParseObject, Name, NameLength);
    if ((! NT_SUCCESS(status)) || (*Name == &Obj_Unnamed))
        return status;

    oldname = *Name;
    oldname_len = *NameLength;

    newname_len = sizeof(OBJECT_NAME_INFORMATION)
                + oldname->Name.Length
                + sizeof(WCHAR)     // backslash
                + RemainingName->Length
                + PAD_LEN;
    newname = Mem_Alloc(pool, newname_len);
    if (! newname) {

        *Name = NULL;
        *NameLength = 0;
        status = STATUS_INSUFFICIENT_RESOURCES;

    } else {

        newname->Name.Buffer = (WCHAR *)(newname + 1);
        newname->Name.Length = oldname->Name.Length;

        ptr = newname->Name.Buffer;
        memcpy(ptr, oldname->Name.Buffer, oldname->Name.Length);
        ptr += oldname->Name.Length / sizeof(WCHAR);

        if (RemainingName->Length) {

            USHORT RemainingName_Length = RemainingName->Length;
            WCHAR *RemainingName_Buffer = RemainingName->Buffer;

            if (*RemainingName_Buffer != L'\\') {

                *ptr = L'\\';
                ++ptr;
                newname->Name.Length += sizeof(WCHAR);    // backslash

            } else {

                //
                // if the path was reparsed by the object manager, the
                // RemainingName may start with duplicate backslashes
                //

                while (RemainingName_Length >= sizeof(WCHAR) * 2
                        && RemainingName_Buffer[0] == L'\\'
                        && RemainingName_Buffer[1] == L'\\') {
                    ++RemainingName_Buffer;
                    RemainingName_Length -= sizeof(WCHAR);
                }
            }

            newname->Name.Length += RemainingName_Length;

            memcpy(ptr, RemainingName_Buffer, RemainingName_Length);
            ptr += RemainingName_Length / sizeof(WCHAR);
        }

        newname->Name.MaximumLength = newname->Name.Length + sizeof(WCHAR);

        memzero(ptr, PAD_LEN);

        *Name = newname;
        *NameLength = newname_len;
        status = STATUS_SUCCESS;
    }

    Mem_Free(oldname, oldname_len);
    return status;
}


//---------------------------------------------------------------------------
// Obj_GetNameOrFileName
//---------------------------------------------------------------------------


_FX NTSTATUS Obj_GetNameOrFileName(
    POOL *pool, void *Object,
    OBJECT_NAME_INFORMATION **Name, ULONG *NameLength)
{
    NTSTATUS status = Obj_GetName(pool, Object, Name, NameLength);

    if (    status == STATUS_OBJECT_PATH_INVALID
         || status == STATUS_PIPE_DISCONNECTED
         || status == STATUS_NOT_SUPPORTED
         || status == STATUS_UNSUCCESSFUL) {

        //
        // if we get an error on getting object name, and we determine
        // this is a file object, then try the alternate parse method,
        // where we query the object name for the device, and just
        // append the file name as it appears in the file object
        //

        void *ObjectType;
        if (Driver_OsVersion >= DRIVER_WINDOWS_7) {

            ObjectType = pObGetObjectType(Object);

        } else {

            OBJECT_HEADER *ObjectHeader = OBJECT_TO_OBJECT_HEADER(Object);

            if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA &&
                Driver_OsBuild > 6000) {

                ObjectType = (OBJECT_TYPE_VISTA_SP1 *)ObjectHeader->Type;

            } else {

                ObjectType = ObjectHeader->Type;
            }
        }

        if (ObjectType == *IoFileObjectType) {

            FILE_OBJECT *FileObject = (FILE_OBJECT *)Object;

            status = Obj_GetParseName(
                            pool, FileObject->DeviceObject,
                            &FileObject->FileName, Name, NameLength);
        }

    } else if (status != STATUS_SUCCESS) {

        Log_Status_Ex(MSG_2101, 0x99, status, Name != NULL ? (*Name)->Name.Buffer : L"Unnamed object");
    }

    return status;
}


//---------------------------------------------------------------------------
// Obj_GetTypeObjectType
//---------------------------------------------------------------------------


_FX POBJECT_TYPE Obj_GetTypeObjectType(void)
{
    static POBJECT_TYPE _TypeObjectType = NULL;

    //
    // on Windows 7 we need to find ObpTypeObjectType.  it is not
    // exported, but a new Windows 7 exported function, ObGetObjectType,
    // references an object type table with the following instruction:
    //          mov eax,dword ptr [nt!ObTypeIndexTable+eax*4]
    // ObpTypeObjectType is the third pointer in the table

    if ((Driver_OsVersion >= DRIVER_WINDOWS_7) && (! _TypeObjectType)) {

        const ULONG status = STATUS_UNSUCCESSFUL;
        static const WCHAR *TypeName = L"ObjectType";

        POBJECT_TYPE pType;
        UNICODE_STRING *Name;

        //
        // analyze ObGetObjectType to find ObTypeIndexTable
        //

        POBJECT_TYPE *pObTypeIndexTable = NULL;

        UCHAR *ptr = (UCHAR *)pObGetObjectType;
        if (ptr) {

            ULONG i;

#ifdef _WIN64
            if (Driver_OsVersion < DRIVER_WINDOWS_10) {
                for (i = 0; i < 16; ++i) {
                    if (ptr[i + 0] == 0x48 &&       // lea rcx,nt!ObTypeIndexTable
                        ptr[i + 1] == 0x8D &&
                        ptr[i + 2] == 0x0D)
                    {
                        LONG offset = *(LONG *)(ptr + i + 3);
                        pObTypeIndexTable =
                            (POBJECT_TYPE *)(ptr + i + 7 + offset);
                    }
                }
            }
            else {
                for (i = 0x1c; i < 0x2c; ++i) {
                    if (ptr[i + 0] == 0x48 &&       // lea rcx,nt!ObTypeIndexTable
                        ptr[i + 1] == 0x8D &&
                        ptr[i + 2] == 0x0D)
                    {
                        LONG offset = *(LONG *)(ptr + i + 3);
                        pObTypeIndexTable =
                            (POBJECT_TYPE *)(ptr + i + 7 + offset);
                    }
                }
                DbgPrint("pObTypeIndexTable = %p\n", pObTypeIndexTable);
            }
#else ! _WIN64
            UCHAR k = 0;
            if (Driver_OsVersion >= DRIVER_WINDOWS_10) {
                k = 0x1c;
            }

            for (i = k; i < (ULONG)16 + k; ++i) {
                if (ptr[i + 0] == 0x8B &&       // mov eax,[...+eax*4]
                    ptr[i + 1] == 0x04 &&
                    ptr[i + 2] == 0x85)
                {
                    ULONG_PTR *ptr2 = (ULONG_PTR *)(ptr + i + 3);
                    pObTypeIndexTable = (POBJECT_TYPE *)*ptr2;
                }
            }

#endif _WIN64

        }

        if (! pObTypeIndexTable) {
            Log_Status_Ex(MSG_OBJ_HOOK_ANY_PROC, 0x91, status, TypeName);
            return FALSE;
        }

        //
        // make sure ObTypeIndexTable[2] points to the "Type" object type
        //

        pType = pObTypeIndexTable[2];

#ifdef _WIN64

        if ((! pType) || ((ULONG_PTR)pType >= 0xFFFFFFFF00000000)) {
            Log_Status_Ex(MSG_OBJ_HOOK_ANY_PROC, 0x92, status, TypeName);
            return FALSE;
        }

#else ! _WIN64

        if ((! pType) || ((ULONG_PTR)pType >= 0xFFFF0000)) {
            Log_Status_Ex(MSG_OBJ_HOOK_ANY_PROC, 0x92, status, TypeName);
            return FALSE;
        }

#endif _WIN64

        Name = &((OBJECT_TYPE_VISTA_SP1 *)pType)->Name;
        if (Name->Length != 8 ||
                Name->Buffer[0] != L'T' || Name->Buffer[1] != L'y' ||
                Name->Buffer[2] != L'p' || Name->Buffer[3] != L'e') {
            Log_Status_Ex(MSG_OBJ_HOOK_ANY_PROC, 0x93, status, TypeName);
            return FALSE;
        }

        _TypeObjectType = pType;
    }

    return _TypeObjectType;
}
