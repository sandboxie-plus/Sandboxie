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


#include "driver.h"
#include "mem.h"


//---------------------------------------------------------------------------
// Mem_AllocEx
//---------------------------------------------------------------------------


_FX void *Mem_AllocEx(POOL *pool, ULONG size, BOOLEAN InitMsg)
{

#ifdef DEBUG_MEMORY

    UCHAR *ptr = Pool_Alloc(pool, size + 64 * 2);
    if (ptr) {

        memset(ptr,         0xCC, 64);
        ptr += 64;
        memset(ptr + size,  0xCC, 64);
    }

#else ! DEBUG_MEMORY

    void *ptr = Pool_Alloc(pool, size);

#endif DEBUG_MEMORY

    // We can't log memory failures because the log functions alloc mem and we will get into a recursive loop
    //if (! ptr) {

    //    if (InitMsg) {

    //        Log_Msg0(MSG_1104);

    //    } else {

    //        Log_Msg_Session(MSG_1201, NULL, NULL, -1);
    //    }
    //}

    return ptr;
}


//---------------------------------------------------------------------------
// Mem_Free
//---------------------------------------------------------------------------


#ifdef DEBUG_MEMORY


_FX void Mem_Free(void *ptr, ULONG size)
{
    int i;
    UCHAR *pre, *post;

    pre  = ((UCHAR *)ptr) - 64;
    post = ((UCHAR *)ptr) + size;

    for (i = 0; i < 64; ++i)
        if (pre[i] != 0xCC || post[i] != 0xCC) {
            DbgPrint("Corruption detected in pool block %08X\n", ptr);
            __debugbreak();
        }

    Pool_Free(pre, size + 64 * 2);
}


#endif DEBUG_MEMORY


//---------------------------------------------------------------------------
// Mem_AllocStringEx
//---------------------------------------------------------------------------


_FX WCHAR *Mem_AllocStringEx(
    POOL *pool, const WCHAR *model_string, BOOLEAN InitMsg)
{
    WCHAR *str;
    ULONG num_bytes = (wcslen(model_string) + 1) * sizeof(WCHAR);
    str = Mem_AllocEx(pool, num_bytes, InitMsg);
    if (str)
        memcpy(str, model_string, num_bytes);
    return str;
}


//---------------------------------------------------------------------------
// Mem_FreeString
//---------------------------------------------------------------------------


_FX void Mem_FreeString(WCHAR *string)
{
    ULONG num_bytes = (wcslen(string) + 1) * sizeof(WCHAR);
    Mem_Free(string, num_bytes);
}


//---------------------------------------------------------------------------
// Mem_GetLockResource
//---------------------------------------------------------------------------


_FX BOOLEAN Mem_GetLockResource(PERESOURCE *ppResource, BOOLEAN InitMsg)
{
    *ppResource = ExAllocatePoolWithTag(NonPagedPool, sizeof(ERESOURCE), tzuk);
    if (*ppResource) {
        ExInitializeResourceLite(*ppResource);
        return TRUE;
    } else {
        Log_Msg0(InitMsg ? MSG_1104 : MSG_1201);
        return FALSE;
    }
}


//---------------------------------------------------------------------------
// Mem_FreeLockResource
//---------------------------------------------------------------------------


_FX void Mem_FreeLockResource(PERESOURCE *ppResource)
{
    if (*ppResource) {
        ExDeleteResourceLite(*ppResource);
        ExFreePoolWithTag(*ppResource, tzuk);
        *ppResource = NULL;
    }
}

//---------------------------------------------------------------------------
//
// Fix for the WindowsKernelModeDriver10.0 not creating Win 7 32bit compatible code
//
//---------------------------------------------------------------------------

#ifndef _WIN64
int __cdecl memcmp(
	_In_reads_bytes_(_Size) void const* _Buf1,
	_In_reads_bytes_(_Size) void const* _Buf2,
	_In_                    size_t      _Size
)
{
	return (RtlCompareMemory(_Buf1, _Buf2, _Size) == _Size) ? 0 : 1;
}
#endif

