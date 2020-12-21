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
// Memory Management
//---------------------------------------------------------------------------


#include "dll.h"
#include "common/pool.h"
#include <stdio.h>


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#undef  DEBUG_MEMORY


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static void *Dll_AllocFromPool(POOL *pool, ULONG size);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static POOL *Dll_Pool = NULL;
static POOL *Dll_PoolTemp = NULL;
static POOL *Dll_PoolCode = NULL;

static ULONG Dll_TlsIndex = TLS_OUT_OF_INDEXES;

#ifdef DEBUG_MEMORY
static volatile ULONG Dll_MemUsage = 0;
static BOOLEAN Dll_MemTrace = FALSE;
#endif


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


_FX BOOLEAN Dll_InitMem(void)
{
    if (! Dll_Pool) {
        Dll_Pool = Pool_Create();
        if (! Dll_Pool)
            return FALSE;
    }

    if (! Dll_PoolTemp) {
        Dll_PoolTemp = Pool_Create();
        if (! Dll_PoolTemp)
            return FALSE;
    }

    if (! Dll_PoolCode) {
        Dll_PoolCode = Pool_CreateTagged(tzuk | 0xFF);
        if (! Dll_PoolCode)
            return FALSE;
    }

    if (Dll_TlsIndex == TLS_OUT_OF_INDEXES) {
        Dll_TlsIndex = TlsAlloc();
        if (Dll_TlsIndex == TLS_OUT_OF_INDEXES)
            return FALSE;
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Dll_AllocFromPool
//---------------------------------------------------------------------------


_FX void *Dll_AllocFromPool(POOL *pool, ULONG size)
{
    UCHAR *ptr;

#ifdef DEBUG_MEMORY
    size += 64 * 2;
#endif // DEBUG_MEMORY

    size += sizeof(ULONG_PTR);
    ptr = Pool_Alloc(pool, size);
    if (! ptr) {
        if (! Dll_BoxName)
            return NULL;
        SbieApi_Log(2305, NULL);
        ExitProcess(-1);
    }

#ifdef DEBUG_MEMORY
    memset(ptr,             0xCC, 64);
    memset(ptr + size - 64, 0xCC, 64);
    //{
    //WCHAR txt[64]; Sbie_snwprintf(txt, 64, L"Dll_Alloc for %-6d, block at %08X (%08X)\n", size, ptr, ptr + 64);
    //OutputDebugString(txt);
    //}
    ptr += 64;
#endif // DEBUG_MEMORY

#ifdef DEBUG_MEMORY
    InterlockedExchangeAdd(&Dll_MemUsage, size);
    if (Dll_MemTrace) {
        WCHAR txt[128];
        Sbie_snwprintf(txt, 128, L"ALLOC %d POOL %s\n", size, (pool == Dll_Pool) ? L"Main" : (pool == Dll_PoolTemp) ? L"Temp" : L"?");
        OutputDebugString(txt);
    }

#endif // DEBUG_MEMORY

    *(ULONG_PTR *)ptr = size;
    ptr += sizeof(ULONG_PTR);
    return ptr;
}


//---------------------------------------------------------------------------
// Dll_Alloc
//---------------------------------------------------------------------------


_FX void *Dll_Alloc(ULONG size)
{
    return Dll_AllocFromPool(Dll_Pool, size);
}


//---------------------------------------------------------------------------
// Dll_AllocTemp
//---------------------------------------------------------------------------


_FX void *Dll_AllocTemp(ULONG size)
{
    return Dll_AllocFromPool(Dll_PoolTemp, size);
}


//---------------------------------------------------------------------------
// Dll_Free
//---------------------------------------------------------------------------


_FX void Dll_Free(void *ptr)
{
    UCHAR *ptr2 = ((UCHAR *)ptr) - sizeof(ULONG_PTR);
    ULONG size = (ULONG)(*(ULONG_PTR *)ptr2);

#ifdef DEBUG_MEMORY
    {
    //WCHAR txt[64];
    int i;
    UCHAR *pre  = ptr2 - 64;
    UCHAR *post = ptr2 + size - 64 * 2;
    for (i = 0; i < 64; ++i)
        if (pre[i] != 0xCC || post[i] != 0xCC) {
            WCHAR txt[64];
            Sbie_snwprintf(txt, 64, L"Memory corrupted, ptr=%p\n", ptr);
            OutputDebugString(txt);
            SbieApi_Log(2316, NULL);
            while (! IsDebuggerPresent()) Sleep(500);
            __debugbreak();
        }

    ptr2 -= 64;
    //Sbie_snwprintf(txt, 64, L"Dll_Free  for %-6d        at %08X\n", size, ptr2);
    //OutputDebugString(txt);
    }
#endif // DEBUG_MEMORY

    Pool_Free(ptr2, size);

#ifdef DEBUG_MEMORY
    InterlockedExchangeAdd(&Dll_MemUsage, -(LONG)size);
    if (Dll_MemTrace) {
        WCHAR txt[128];
        Sbie_snwprintf(txt, 128, L"FREE  %d\n", size);
        OutputDebugString(txt);
    }
#endif // DEBUG_MEMORY
}


//---------------------------------------------------------------------------
// Dll_AllocCode128
//---------------------------------------------------------------------------


_FX void *Dll_AllocCode128(void)
{
    //
    // note that a pool cell is 128 bytes
    //

    UCHAR *ptr = Pool_Alloc(Dll_PoolCode, 128);
    if (! ptr) {
        SbieApi_Log(2305, NULL);
        ExitProcess(-1);
    }
    return ptr;
}


//---------------------------------------------------------------------------
// Dll_FreeCode128
//---------------------------------------------------------------------------


_FX void Dll_FreeCode128(void *ptr)
{
    Pool_Free(ptr, 128);
}


//---------------------------------------------------------------------------
// Dll_GetTlsData
//---------------------------------------------------------------------------


_FX THREAD_DATA *Dll_GetTlsData(ULONG *pLastError)
{
    THREAD_DATA *data;

    if (Dll_TlsIndex == TLS_OUT_OF_INDEXES)
        data = NULL;
    else {

        ULONG LastError = GetLastError();

        data = TlsGetValue(Dll_TlsIndex);

        if (! data) {

            data = Dll_Alloc(sizeof(THREAD_DATA));
            memzero(data, sizeof(THREAD_DATA));

            TlsSetValue(Dll_TlsIndex, data);
        }

        SetLastError(LastError);
        if (pLastError)
            *pLastError = LastError;
    }

    return data;
}


//---------------------------------------------------------------------------
// Dll_FreeTlsData
//---------------------------------------------------------------------------


_FX void Dll_FreeTlsData(void)
{
    THREAD_DATA *data;
    ULONG depth;

    if (Dll_TlsIndex == TLS_OUT_OF_INDEXES)
        data = NULL;
    else
        data = TlsGetValue(Dll_TlsIndex);
    if (! data)
        return;

    TlsSetValue(Dll_TlsIndex, NULL);

    for (depth = 0; depth < NAME_BUFFER_DEPTH; ++depth) {

        WCHAR *buf = data->name_buffer[TRUE_NAME_BUFFER][depth];
        if (buf)
            Dll_Free(buf);
        data->name_buffer[TRUE_NAME_BUFFER][depth] = NULL;

        buf = data->name_buffer[COPY_NAME_BUFFER][depth];
        if (buf)
            Dll_Free(buf);
        data->name_buffer[COPY_NAME_BUFFER][depth] = NULL;

		buf = data->name_buffer[TMPL_NAME_BUFFER][depth];
		if (buf)
			Dll_Free(buf);
		data->name_buffer[TMPL_NAME_BUFFER][depth] = NULL;
    }

    Dll_Free(data);
}


//---------------------------------------------------------------------------
// Dll_GetTlsNameBuffer
//---------------------------------------------------------------------------


ALIGNED WCHAR *Dll_GetTlsNameBuffer(
    THREAD_DATA *data, ULONG which, ULONG size)
{
    WCHAR *old_name_buffer;
    ULONG old_name_buffer_len;
    WCHAR **name_buffer;
    ULONG *name_buffer_len;

    name_buffer     = &data->name_buffer    [which][data->depth];
    name_buffer_len = &data->name_buffer_len[which][data->depth];

    //
    // round up the requested size (+ extra padding of some bytes)
    // to a multiple of PAGE_SIZE.
    //

    size = (size + 64 + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    if (size > *name_buffer_len) {

        //
        // requested size is larger than current name buffer, so we
        // allocate a larger buffer, and move the contents over to it
        //

        old_name_buffer     = *name_buffer;
        old_name_buffer_len = *name_buffer_len;

        *name_buffer_len = size;
        *name_buffer = Dll_Alloc(*name_buffer_len);

        if (old_name_buffer) {
            memcpy(*name_buffer, old_name_buffer, old_name_buffer_len);
            Dll_Free(old_name_buffer);
        }
    }

    //
    // debug checks:  the name buffer is allocated at least 64 bytes
    // more than needed.  fill these with 0xCC, andd check that later
    //

#ifdef DEBUG_MEMORY

    {
    UCHAR *debug_area = ((UCHAR *)(*name_buffer)) + *name_buffer_len - 64;
    memset(debug_area, 0xCC, 64);
    }

#endif // DEBUG_MEMORY

    return *name_buffer;
}


//---------------------------------------------------------------------------
// Dll_PushTlsNameBuffer
//---------------------------------------------------------------------------


ALIGNED void Dll_PushTlsNameBuffer(THREAD_DATA *data)
{
    ++data->depth;
    if (data->depth > NAME_BUFFER_DEPTH - 4)
        SbieApi_Log(2310, L"%d", data->depth);
    if (data->depth >= NAME_BUFFER_DEPTH) {
        ExitProcess(-1);
    }
}


//---------------------------------------------------------------------------
// Dll_PopTlsNameBuffer
//---------------------------------------------------------------------------


_FX void Dll_PopTlsNameBuffer(THREAD_DATA *data)
{
    //
    // debug checks:  the name buffer is allocated at least 64 bytes
    // more than needed.  fill these with 0xCC, andd check that later
    //

#ifdef DEBUG_MEMORY

    {

    WCHAR **name_buffer;
    ULONG *name_buffer_len;
    UCHAR *debug_area;
    ULONG i;
    ULONG which;
    WCHAR txt[128];

    which = TRUE_NAME_BUFFER;
    name_buffer     = &data->name_buffer    [which][data->depth];
    name_buffer_len = &data->name_buffer_len[which][data->depth];
    debug_area      = ((UCHAR *)(*name_buffer)) + *name_buffer_len - 64;
    for (i = 0; i < 64 && (*name_buffer); ++i)
        if (debug_area[i] != 0xCC) {
            Sbie_snwprintf(txt, 128, L"Buffer %d corrupted.  Buffer=%08X Length=%d Corrupt=%08X\n",
                which, *name_buffer, *name_buffer_len, &debug_area[i]);
            OutputDebugString(txt);
            __debugbreak();
        }

    which = COPY_NAME_BUFFER;
    name_buffer     = &data->name_buffer    [which][data->depth];
    name_buffer_len = &data->name_buffer_len[which][data->depth];
    debug_area      = ((UCHAR *)(*name_buffer)) + *name_buffer_len - 64;
    for (i = 0; i < 64 && (*name_buffer); ++i)
        if (debug_area[i] != 0xCC) {
            Sbie_snwprintf(txt, 128, L"Buffer %d corrupted.  Buffer=%08X Length=%d Corrupt=%08X\n",
                which, *name_buffer, *name_buffer_len, &debug_area[i]);
            OutputDebugString(txt);
            __debugbreak();
        }

	// todo: snapshots TMPL_NAME_BUFFER

    }

#endif // DEBUG_MEMORY

    --data->depth;
    if (data->depth < 0) {
        SbieApi_Log(2310, L"%d", data->depth);
    }
}
