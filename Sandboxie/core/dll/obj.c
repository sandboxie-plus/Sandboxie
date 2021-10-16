/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020 David Xanatos, xanasoft.com
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


#include "dll.h"
#include "obj.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static NTSTATUS Obj_NtQueryObject(
    HANDLE ObjectHandle,
    OBJECT_INFORMATION_CLASS ObjectInformationClass,
    void *ObjectInformation,
    ULONG Length,
    ULONG *ResultLength);

static NTSTATUS Obj_NtQueryVirtualMemory(
    HANDLE ProcessHandle,
    void *BaseAddress,
    MEMORY_INFORMATION_CLASS MemoryInformationClass,
    void *MemoryInformation,
    SIZE_T Length,
    SIZE_T *ResultLength);

static NTSTATUS Obj_NtQueryInformationProcess(
	HANDLE ProcessHandle,
	PROCESSINFOCLASS ProcessInformationClass,
	PVOID ProcessInformation,
	ULONG ProcessInformationLength,
	PULONG ReturnLength);

//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static P_NtQueryObject          __sys_NtQueryObject             = NULL;

       P_NtQueryVirtualMemory   __sys_NtQueryVirtualMemory      = NULL;

	   P_NtQueryObject          __sys_NtQueryInformationProcess = NULL;


//---------------------------------------------------------------------------
// Obj_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Obj_Init(void)
{
#if 0
    __sys_NtQueryObject = NtQueryObject;
#else
    SBIEDLL_HOOK(Obj_,NtQueryObject);
    SBIEDLL_HOOK(Obj_,NtQueryVirtualMemory);
	SBIEDLL_HOOK(Obj_,NtQueryInformationProcess);
#endif
    return TRUE;
}


//---------------------------------------------------------------------------
// Obj_GetObjectType
//---------------------------------------------------------------------------


_FX ULONG Obj_GetObjectType(HANDLE ObjectHandle)
{
    NTSTATUS status;
    PUBLIC_OBJECT_TYPE_INFORMATION  info;

    ULONG length = sizeof(info);

    status = __sys_NtQueryObject(ObjectHandle, ObjectTypeInformation, &info, length, &length);

    if (NT_SUCCESS(status)) {
        const WCHAR *TypeName = info.TypeName.Buffer;
        if (TypeName[0] == L'D' && _wcsicmp(TypeName, L"Directory") == 0)
            return OBJ_TYPE_DIRECTORY;
        if (TypeName[0] == L'F' && _wcsicmp(TypeName, L"File") == 0)
            return OBJ_TYPE_FILE;
        if (TypeName[0] == L'K' && _wcsicmp(TypeName, L"Key") == 0)
            return OBJ_TYPE_KEY;
        if (TypeName[0] == L'A' && _wcsicmp(TypeName, L"ALPC Port") == 0)
            return OBJ_TYPE_PORT;
        if (TypeName[0] == L'P' && _wcsicmp(TypeName, L"Port") == 0)
            return OBJ_TYPE_PORT;
        if (TypeName[0] == L'E' && _wcsicmp(TypeName, L"Event") == 0)
            return OBJ_TYPE_EVENT;
        if (TypeName[0] == L'M' && _wcsicmp(TypeName, L"Mutant") == 0)
            return OBJ_TYPE_MUTANT;
        if (TypeName[0] == L'S' && _wcsicmp(TypeName, L"Section") == 0)
            return OBJ_TYPE_SECTION;
        if (TypeName[0] == L'S' && _wcsicmp(TypeName, L"Semaphore") == 0)
            return OBJ_TYPE_SEMAPHORE;
        if (TypeName[0] == L'P' && _wcsicmp(TypeName, L"Process") == 0)
            return OBJ_TYPE_PROCESS;
    }
    return OBJ_TYPE_UNKNOWN;
}


//---------------------------------------------------------------------------
// Obj_GetObjectName
//---------------------------------------------------------------------------


_FX NTSTATUS Obj_GetObjectName(
    HANDLE ObjectHandle, void *ObjectName, ULONG *Length)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);
    NTSTATUS status;

    TlsData->obj_NtQueryObject_lock = TRUE;

    status = __sys_NtQueryObject(
        ObjectHandle, ObjectNameInformation, ObjectName, *Length, Length);

    TlsData->obj_NtQueryObject_lock = FALSE;

    return status;
}


//---------------------------------------------------------------------------
// Obj_NtQueryObject
//---------------------------------------------------------------------------


_FX NTSTATUS Obj_NtQueryObject(
    HANDLE ObjectHandle,
    OBJECT_INFORMATION_CLASS ObjectInformationClass,
    void *ObjectInformation,
    ULONG Length,
    ULONG *ResultLength)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    UNICODE_STRING *name;
    NTSTATUS status;
    ULONG type, maxlen, outlen;

    //
    // if the request is not for object name, or if this is a
    // recursive invocation, then call the system directly
    //

    if (TlsData->obj_NtQueryObject_lock ||
            ObjectInformationClass != ObjectNameInformation) {

        return __sys_NtQueryObject(
            ObjectHandle, ObjectInformationClass,
            ObjectInformation, Length, ResultLength);
    }

    //
    // determine object type
    //

    TlsData->obj_NtQueryObject_lock = TRUE;

    type = Obj_GetObjectType(ObjectHandle);

    if (type != OBJ_TYPE_FILE       && type != OBJ_TYPE_KEY         &&
        type != OBJ_TYPE_DIRECTORY  && type != OBJ_TYPE_PORT        &&
        type != OBJ_TYPE_EVENT      && type != OBJ_TYPE_MUTANT      &&
        type != OBJ_TYPE_SECTION    && type != OBJ_TYPE_SEMAPHORE   ) {

        status = __sys_NtQueryObject(
            ObjectHandle, ObjectInformationClass,
            ObjectInformation, Length, ResultLength);

        goto finish;
    }

    //
    // query name for object that is potentially inside the box
    //

    if (Length) {
        name = ObjectInformation;
        maxlen = Length & ~1;
    } else {
        maxlen = sizeof(OBJECT_NAME_INFORMATION) + 32;
        name = Dll_AllocTemp(maxlen);
    }

    status = __sys_NtQueryObject(
        ObjectHandle, ObjectNameInformation, name, maxlen, &outlen);

    if (status == STATUS_INFO_LENGTH_MISMATCH ||
        status == STATUS_BUFFER_OVERFLOW) {

        if (name != ObjectInformation)
            Dll_Free(name);
        maxlen = outlen;
        name = Dll_AllocTemp(maxlen);

        status = __sys_NtQueryObject(
            ObjectHandle, ObjectNameInformation, name, maxlen, &outlen);
    }

    if (! NT_SUCCESS(status)) {

        if (name != ObjectInformation)
            Dll_Free(name);

        status = __sys_NtQueryObject(
            ObjectHandle, ObjectInformationClass,
            ObjectInformation, Length, ResultLength);

        goto finish;
    }

    //
    // fix path to remove sandbox prefix
    //

    if (name->Length) {

        ULONG tmplen;

        if (type == OBJ_TYPE_FILE)
            tmplen = File_NtQueryObjectName(name, maxlen);

        else if (type == OBJ_TYPE_KEY)
            tmplen = Key_NtQueryObjectName(name, maxlen);

        else
            tmplen = Ipc_NtQueryObjectName(name, maxlen);

        if (tmplen)
            outlen = sizeof(UNICODE_STRING) + tmplen;
    }

    //
    // copy result to caller
    //

    if (ResultLength)
        *ResultLength = outlen;

    if (Length < outlen) {

        if (Length < sizeof(UNICODE_STRING))
            status = STATUS_INFO_LENGTH_MISMATCH;
        else
            status = STATUS_BUFFER_OVERFLOW;

    } else if (name != ObjectInformation) {

        UNICODE_STRING *out_name = (UNICODE_STRING *)ObjectInformation;
        memcpy(out_name, name, outlen);
        out_name->Buffer = (WCHAR *)(out_name + 1);
    }

    if (name != ObjectInformation)
        Dll_Free(name);

    //
    // finish
    //

finish:

    TlsData->obj_NtQueryObject_lock = FALSE;

    SetLastError(LastError);
    return status;
}


//---------------------------------------------------------------------------
// Obj_NtQueryVirtualMemory
//---------------------------------------------------------------------------


_FX NTSTATUS Obj_NtQueryVirtualMemory(
    HANDLE ProcessHandle,
    void *BaseAddress,
    MEMORY_INFORMATION_CLASS MemoryInformationClass,
    void *MemoryInformation,
    SIZE_T Length,
    SIZE_T *ResultLength)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    UNICODE_STRING *name;
    NTSTATUS status;
    SIZE_T maxlen, outlen;

    //
    // if the request is not for an object name, then call the system
    //

    if (MemoryInformationClass != MemoryMappedFilenameInformation) {

        return __sys_NtQueryVirtualMemory(
            ProcessHandle, BaseAddress, MemoryInformationClass,
            MemoryInformation, Length, ResultLength);
    }

    //
    // query name for object that is potentially inside the box
    //

    if (Length) {
        name = MemoryInformation;
        maxlen = Length & ~1;
    } else {
        maxlen = sizeof(OBJECT_NAME_INFORMATION) + 32;
        name = Dll_AllocTemp((ULONG)maxlen);
    }

    status = __sys_NtQueryVirtualMemory(
        ProcessHandle, BaseAddress, MemoryMappedFilenameInformation,
        name, maxlen, &outlen);

    if (status == STATUS_INFO_LENGTH_MISMATCH ||
        status == STATUS_BUFFER_OVERFLOW) {

        if (name != MemoryInformation)
            Dll_Free(name);
        maxlen = outlen;
        name = Dll_AllocTemp((ULONG)maxlen);

        status = __sys_NtQueryVirtualMemory(
            ProcessHandle, BaseAddress, MemoryMappedFilenameInformation,
            name, maxlen, &outlen);
    }

    if (! NT_SUCCESS(status)) {

        if (name != MemoryInformation)
            Dll_Free(name);

        status = __sys_NtQueryVirtualMemory(
            ProcessHandle, BaseAddress, MemoryInformationClass,
            MemoryInformation, Length, ResultLength);

        goto finish;
    }

    //
    // fix path to remove sandbox prefix
    //

    if (name->Length && maxlen < 0x01000000) {

        ULONG tmplen = File_NtQueryObjectName(name, (ULONG)maxlen);

        if (tmplen)
            outlen = sizeof(UNICODE_STRING) + tmplen;
    }

    //
    // copy result to caller
    //

    if (ResultLength)
        *ResultLength = outlen;

    if (Length < outlen) {

        if (Length < sizeof(UNICODE_STRING))
            status = STATUS_INFO_LENGTH_MISMATCH;
        else
            status = STATUS_BUFFER_OVERFLOW;

    } else if (name != MemoryInformation) {

        UNICODE_STRING *out_name = (UNICODE_STRING *)MemoryInformation;
        memcpy(out_name, name, outlen);
        out_name->Buffer = (WCHAR *)(out_name + 1);
    }

    if (name != MemoryInformation)
        Dll_Free(name);

    //
    // finish
    //

finish:

    SetLastError(LastError);
    return status;
}

//---------------------------------------------------------------------------
// Obj_NtQueryVirtualMemory
//---------------------------------------------------------------------------


_FX NTSTATUS Obj_NtQueryInformationProcess(
	HANDLE ProcessHandle,
	PROCESSINFOCLASS ProcessInformationClass,
	PVOID ProcessInformation,
	ULONG ProcessInformationLength,
	PULONG ReturnLength)
{
	NTSTATUS status;
	ULONG outlen;

	status = __sys_NtQueryInformationProcess(
		ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, &outlen);
	
	if (ProcessInformationClass == ProcessImageFileName)
	{
		//
		// since file paths are always shorter without the sandbox prefix we can keep this simple
		//

		ULONG tmplen;
		PUNICODE_STRING fileName = (PUNICODE_STRING)ProcessInformation;

		tmplen = File_NtQueryObjectName(fileName, fileName->MaximumLength);

		if (tmplen)
			outlen = sizeof(UNICODE_STRING) + tmplen;
	}

	if (ReturnLength)
		*ReturnLength = outlen;

	return status;
}