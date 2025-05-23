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
// Pool Memory Manager
//---------------------------------------------------------------------------


#include "pool.h"
#include "list.h"
#include "lock.h"
#include "defines.h"

#ifndef KERNEL_MODE
#ifdef _M_ARM64EC
P_NtAllocateVirtualMemoryEx __sys_NtAllocateVirtualMemoryEx = NULL;
#endif
#endif

//---------------------------------------------------------------------------
// Parameters
//---------------------------------------------------------------------------


#ifndef POOL_TAG
static const ULONG Pool_Tag = 'loop';
#define POOL_TAG                Pool_Tag
#endif


// page size and cell size;
// cell size must be a power of two
#if defined(KERNEL_MODE)

#define POOL_PAGE_SIZE          4096            // system page size
#define POOL_CELL_SIZE          16
#ifdef _WIN64
#define POOL_MASK_LEFT          0xFFFFFFFFFFFFF000UL
#else
#define POOL_MASK_LEFT          0xFFFFF000UL
#endif
#define POOL_MASK_RIGHT         0xFFF


#else /* ! KERNEL_MODE */

#define POOL_PAGE_SIZE          65536           // VirtualAlloc granularity
#define POOL_CELL_SIZE          128
#ifdef _WIN64
#define POOL_MASK_LEFT          0xFFFFFFFFFFFF0000UL
#else
#define POOL_MASK_LEFT          0xFFFF0000UL
#endif
#define POOL_MASK_RIGHT         0xFFFF
#endif


// requests for more than this amount will allocate a LARGE_CHUNK.
// should be close to PAGE_SIZE, because LARGE_CHUNKs are allocated
// using page granularity
#define LARGE_CHUNK_MINIMUM     (POOL_PAGE_SIZE * 3 / 4)


// page that have less than this many free cells, are not considered
// during allocation requests
#define FULL_PAGE_THRESHOLD     4


#ifndef POOL_DEBUG
#define POOL_DEBUG 0
#endif


#ifndef POOL_TIMING
#define POOL_TIMING 0
#endif


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


// pad lengths
#define PAD_8(p)        (((p) + 7) & ~7)
#define PAD_CELL(p)     (((p) + POOL_CELL_SIZE - 1) & ~(POOL_CELL_SIZE - 1))
#define NUM_CELLS(n)    (PAD_CELL(n) / POOL_CELL_SIZE)


// size of header part of a pool page, padded to cell size
#define PAGE_HEADER_SIZE    PAD_CELL(sizeof(PAGE))

// calculate how many bytes would be needed for a bitmap,
// if the bitmap itself was not part of the page, padded to cell-size
#define PAGE_BITMAP_SIZE    \
    PAD_CELL(((POOL_PAGE_SIZE - PAGE_HEADER_SIZE) / POOL_CELL_SIZE + 7) / 8)

// number of cells in a single pool page
#define NUM_PAGE_CELLS      \
    ((POOL_PAGE_SIZE - PAGE_HEADER_SIZE - PAGE_BITMAP_SIZE) / POOL_CELL_SIZE)


// size of LARGE_CHUNK structure, padded to 8-bytes
#define LARGE_CHUNK_SIZE    PAD_8(sizeof(LARGE_CHUNK))


// Maximum possible size for a large chunk
#define LARGE_CHUNK_MAXIMUM     (0xFFFFFFFFu - (LARGE_CHUNK_SIZE + POOL_PAGE_SIZE))


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct PAGE PAGE;
typedef struct LARGE_CHUNK LARGE_CHUNK;


#pragma pack(push)
#pragma pack(1)


struct PAGE {

    LIST_ELEM list_elem;
    PAGE *next;
    POOL *pool;
    ULONG eyecatcher;
    USHORT num_free;                    // estimated, not accurate
};


struct POOL {

    ULONG eyecatcher;

#ifdef POOL_USE_CUSTOM_LOCK

    LOCK pages_lock;
    LOCK large_chunks_lock;

#elif defined(KERNEL_MODE)

    PERESOURCE lock;

#else /* ! KERNEL_MODE */

    volatile LONG thread;
    CRITICAL_SECTION lock;

#endif

    LIST pages;                         // pages searched during allocation
    LIST full_pages;                    // full pages that are not searched
    LIST large_chunks;

    UCHAR initial_bitmap[PAGE_BITMAP_SIZE];
};


struct LARGE_CHUNK {

    LIST_ELEM list_elem;
    ULONG eyecatcher;
    POOL *pool;
    void *ptr;
    ULONG size;                         // multiple of POOL_PAGE_SIZE

};


#pragma pack(pop)


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static void *Pool_Alloc_Mem(ULONG size, ULONG tag);
static void Pool_Free_Mem(void *ptr, ULONG tag);

static PAGE *Pool_Alloc_Page(POOL *pool, ULONG tag);

static ULONG Pool_Find_Cells(PAGE *page, ULONG size);

static void *Pool_Get_Cells(POOL *pool, ULONG size);
static void Pool_Free_Cells(void *ptr, ULONG size);

static void *Pool_Get_Large_Chunk(POOL *pool, ULONG size);
static void Pool_Free_Large_Chunk(void *ptr, ULONG size);


//---------------------------------------------------------------------------
// ABEND
//---------------------------------------------------------------------------


#if defined(KERNEL_MODE) && (POOL_DEBUG == 0)
#define ABEND(why) \
    KeBugCheckEx(DRIVER_CORRUPTED_MMPOOL, (ULONG_PTR)POOL_TAG, why, 0, 0)
#else /* ! KERNEL_MODE || POOL_DEBUG */
#define ABEND(why) { OutputDebugString(L"pool abend " L#why L"\n"); __debugbreak();  }
#endif


enum {
    FIRST_ABEND                                             = 0,
    POOL_FREE_SIZE_MISMATCH                                 = 1,
    POOL_FREE_NULL_PTR_OR_ZERO_SIZE                         = 2,
    POOL_FREE_LARGE_CHUNK_FIRST_EYECATCHER_MISMATCH         = 3,
    POOL_FREE_LARGE_CHUNK_SECOND_EYECATCHER_MISMATCH        = 4,
    POOL_GET_CELLS_EYECATCHER_MISMATCH                      = 5,
    POOL_FREE_CELLS_EYECATCHER_MISMATCH                     = 6,
    LAST_ABEND
};


//---------------------------------------------------------------------------
// Timing
//---------------------------------------------------------------------------


#if POOL_TIMING


static __int64 Pool_Alloc_Time = 0;
static __int64 Pool_Alloc_Mem_Time = 0;
static __int64 Pool_Alloc_Page_Time = 0;
static __int64 Pool_Find_Cells_Time = 0;
static __int64 Pool_Get_Cells_Time = 0;
static __int64 Pool_Get_Cells_1_Time = 0;
static __int64 Pool_Get_Cells_2_Time = 0;

static __int64 Pool_Free_Time = 0;
static __int64 Pool_Free_Mem_Time = 0;
static __int64 Pool_Free_Cells_Time = 0;


ALIGNED void Pool_Timing(__int64 *timer)
{
    static __int64 TimerQueue[16];
    static int TimerLevel = 0;
    FILETIME ftNow;
    LARGE_INTEGER liNow;
    __int64 now;

    GetSystemTimeAsFileTime(&ftNow);
    liNow.LowPart = ftNow.dwLowDateTime;
    liNow.HighPart = ftNow.dwHighDateTime;
    now = liNow.QuadPart;

    if (! timer) {
        TimerQueue[TimerLevel] = now;
        ++TimerLevel;
    } else {
        --TimerLevel;
        (*timer) += now - TimerQueue[TimerLevel];
    }
}


ALIGNED void Pool_Print_Timing(void)
{
    printf("Pool_Alloc_Time = %f\n", Pool_Alloc_Time / 1000.0);
    printf("Pool_Alloc_Mem_Time = %f\n", Pool_Alloc_Mem_Time / 1000.0);
    printf("Pool_Alloc_Page_Time = %f\n", Pool_Alloc_Page_Time / 1000.0);
    printf("Pool_Find_Cells_Time = %f\n", Pool_Find_Cells_Time / 1000.0);
    printf("Pool_Get_Cells_Time = %f\n", Pool_Get_Cells_Time / 1000.0);
    printf("Pool_Get_Cells_1_Time = %f\n", Pool_Get_Cells_1_Time / 1000.0);
    printf("Pool_Get_Cells_2_Time = %f\n", Pool_Get_Cells_2_Time / 1000.0);
}


#else POOL_TIMING


#define Pool_Timing(timer)
#define Pool_Print_Timing()


#endif


//---------------------------------------------------------------------------
// Locks
//---------------------------------------------------------------------------


#ifdef POOL_USE_CUSTOM_LOCK


static const WCHAR *Pool_pages_lock_Name        = L"PoolLockP";
static const WCHAR *Pool_large_chunks_lock_Name = L"PoolLockL";


//#define POOL_LOCK(lock)
//#define POOL_UNLOCK(lock)

#define POOL_DECLARE_IRQL


#if 1

#if defined(KERNEL_MODE)
#undef  POOL_DECLARE_IRQL
#define POOL_DECLARE_IRQL KIRQL irql;
#define POOL_RAISE_IRQL KeRaiseIrql(APC_LEVEL, &irql);
#define POOL_LOWER_IRQL KeLowerIrql(irql);
#else
#define POOL_RAISE_IRQL
#define POOL_LOWER_IRQL
#endif

#define POOL_LOCK(lock)                                 \
    POOL_RAISE_IRQL                                     \
    Lock_Exclusive(&pool->lock, Pool_##lock##_Name);

#define POOL_UNLOCK(lock)                               \
    Lock_Unlock(&pool->lock, Pool_##lock##_Name);       \
    POOL_LOWER_IRQL

#endif


#elif defined(KERNEL_MODE)


#define POOL_DECLARE_IRQL KIRQL irql;
#define POOL_LOCK(dummylockname)                        \
    KeRaiseIrql(APC_LEVEL, &irql);                      \
    ExAcquireResourceExclusiveLite(pool->lock, TRUE);
#define POOL_UNLOCK(dummylockname)                      \
    ExReleaseResourceLite(pool->lock);                  \
    KeLowerIrql(irql);


#else /* ! KERNEL_MODE */

#define POOL_DECLARE_IRQL                               \
    const LONG _tid = (LONG)GetCurrentThreadId();       \
    BOOLEAN locked;

#define POOL_LOCK(dummylockname)                        \
    if (InterlockedCompareExchange(&pool->thread, _tid, 0) != _tid) {   \
        locked = TRUE;                                  \
        EnterCriticalSection(&pool->lock);              \
        pool->thread = _tid;                            \
    } else                                              \
        locked = FALSE;

#define POOL_UNLOCK(dummylockname)                      \
    if (locked) {                                       \
        pool->thread = _tid;                            \
        LeaveCriticalSection(&pool->lock);              \
        InterlockedExchange(&pool->thread, 0);          \
    }

#endif


//---------------------------------------------------------------------------
// Pool_Alloc_Mem
//---------------------------------------------------------------------------


ALIGNED void *Pool_Alloc_Mem(ULONG size, ULONG tag)
{
    void *ptr = NULL;

    Pool_Timing(NULL);

    // size parameter will be a multiple of POOL_PAGE_SIZE, and this routine
    // must return memory allocated with page-alignment
#ifdef KERNEL_MODE
#if (NTDDI_VERSION >= NTDDI_WIN10_VB)
    ptr = ExAllocatePool2(POOL_FLAG_PAGED, size, tag);
#else
    ptr = ExAllocatePoolWithTag(PagedPool, size, tag);
#endif
#else
    SIZE_T RegionSize = size;
#ifdef _M_ARM64EC
    if ((UCHAR)tag == 0xEC) {

        //
        // this pool is designated for native ARM code in a EC process
        //

        MEM_EXTENDED_PARAMETER Parameter = { 0 };
	    Parameter.Type = MemExtendedParameterAttributeFlags;
	    Parameter.ULong64 = MEM_EXTENDED_PARAMETER_EC_CODE;

        __sys_NtAllocateVirtualMemoryEx(NtCurrentProcess(), &ptr, &RegionSize, MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN,
            PAGE_EXECUTE_READWRITE, &Parameter, 1);
    }
    else
#endif
        NtAllocateVirtualMemory(NtCurrentProcess(), &ptr, 0, &RegionSize, MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN,
            ((UCHAR)tag == 0xFF ? PAGE_EXECUTE_READWRITE : PAGE_READWRITE));
#endif
    // printf("Allocated %d bytes at %08X\n", size, ptr);

    Pool_Timing(&Pool_Alloc_Mem_Time);

    return ptr;
}


//---------------------------------------------------------------------------
// Pool_Free_Mem
//---------------------------------------------------------------------------


ALIGNED void Pool_Free_Mem(void *ptr, ULONG tag)
{
    // printf("Freeing at %08X\n", ptr);
#ifdef KERNEL_MODE
    ExFreePoolWithTag(ptr, tag);
#else
    if (! VirtualFree(ptr, 0, MEM_RELEASE)) {
        RaiseException(
            STATUS_ACCESS_VIOLATION,
            EXCEPTION_NONCONTINUABLE_EXCEPTION, 0, NULL);
        ExitProcess(-1);
    }
#endif
}

//---------------------------------------------------------------------------
// Pool_Alloc_Page
//---------------------------------------------------------------------------


ALIGNED PAGE *Pool_Alloc_Page(POOL *pool, ULONG tag)
{
    PAGE *page;

    Pool_Timing(NULL);

    page = (PAGE*)Pool_Alloc_Mem(POOL_PAGE_SIZE, tag);
    if (page) {

        page->eyecatcher = tag;
        page->next = NULL;
        page->num_free = NUM_PAGE_CELLS;

        if (pool) {
            UCHAR *bitmap = (UCHAR *)page + PAGE_HEADER_SIZE;
            memcpy(bitmap, pool->initial_bitmap, PAGE_BITMAP_SIZE);
            page->pool = pool;
            List_Insert_Before(&pool->pages, NULL, page);
        }
    }

    Pool_Timing(&Pool_Alloc_Page_Time);

    return page;
}


//---------------------------------------------------------------------------
// Pool_Create
//---------------------------------------------------------------------------


ALIGNED POOL *Pool_Create(void)
{
    return Pool_CreateTagged(POOL_TAG);
}


//---------------------------------------------------------------------------
// Pool_CreateTagged
//---------------------------------------------------------------------------


ALIGNED POOL *Pool_CreateTagged(ULONG tag)
{
    PAGE *page;
    UCHAR *bitmap;
    ULONG i, ofs, bit;
    POOL *pool;

    // allocate the first pool page

    page = Pool_Alloc_Page(NULL, tag);
    if (! page)
        return NULL;

    // prepare the initial bitmap for the page.  first reset the entire
    // cell-size-padded cell bitmap to zeroes.  then set to one just
    // those bits that are used for padding and don't represent real cells

    bitmap = (UCHAR *)page + PAGE_HEADER_SIZE;
    memzero(bitmap, PAGE_BITMAP_SIZE);
    i = NUM_PAGE_CELLS;
    while (1) {
        ofs = i / 8;
        bit = 1 << (i & 7);
        if (ofs >= PAGE_BITMAP_SIZE)
            break;
        if (bit == 1) {
            bitmap[ofs] = 0xFF;
            i += 8;
        } else {
            bitmap[ofs] |= bit;
            ++i;
        }
    }

    // allocate the pool structure just past the page bitmap,
    // then initialize it

    pool = (POOL *)((UCHAR *)page + PAGE_HEADER_SIZE + PAGE_BITMAP_SIZE);

    page->pool = pool;

    pool->eyecatcher = tag;

#ifdef POOL_USE_CUSTOM_LOCK

    pool->pages_lock        = LOCK_FREE;
    pool->large_chunks_lock = LOCK_FREE;

#elif defined(KERNEL_MODE)

#if (NTDDI_VERSION >= NTDDI_WIN10_VB)
    pool->lock = ExAllocatePool2(
                                POOL_FLAG_NON_PAGED, sizeof(ERESOURCE), tag);
#else
    pool->lock = ExAllocatePoolWithTag(
                                NonPagedPool, sizeof(ERESOURCE), tag);
#endif
    if (! pool->lock) {
        Pool_Free_Mem(page, tag);
        return NULL;
    }
    ExInitializeResourceLite(pool->lock);

#else /* ! KERNEL_MODE */

    InterlockedExchange(&pool->thread, 0);
    InitializeCriticalSectionAndSpinCount(&pool->lock, 1000);

#endif

    memcpy(pool->initial_bitmap, bitmap, PAGE_BITMAP_SIZE);

    List_Init(&pool->pages);
    List_Insert_Before(&pool->pages, NULL, page);

    List_Init(&pool->full_pages);
    List_Init(&pool->large_chunks);

    // mark in the bitmap, the cells allocated to the pool structure

    i = 0;
    while (i < NUM_CELLS(sizeof(POOL))) {
        ofs = i / 8;
        bit = 1 << (i & 7);
        bitmap[ofs] |= bit;
        ++i;
    }

    return pool;
}


//---------------------------------------------------------------------------
// Pool_Delete
//---------------------------------------------------------------------------


ALIGNED ULONG Pool_Delete(POOL *pool)
{
    LARGE_CHUNK *large_chunk;
    PAGE *page;
    ULONG num_pages = 0;

    // if this pool tracks any allocated large chunks, free them now

    large_chunk = (LARGE_CHUNK*)List_Head(&pool->large_chunks);
    while (large_chunk) {
        LARGE_CHUNK *next_large_chunk = (LARGE_CHUNK*)List_Next(large_chunk);
        num_pages += large_chunk->size / POOL_PAGE_SIZE;
        Pool_Free_Mem(large_chunk->ptr, large_chunk->eyecatcher);
        large_chunk = next_large_chunk;
    }

    // free pages used by this pool, but make sure not to free the page that
    // contains the pool, until all other pages have been freed

    page = (PAGE*)List_Head(&pool->pages);
    while (page) {
        PAGE *next_page = (PAGE*)List_Next(page);
        if (((ULONG_PTR)pool & POOL_MASK_LEFT) != (ULONG_PTR)page)
            Pool_Free_Mem(page, page->eyecatcher);
        ++num_pages;
        page = next_page;
    }

    page = (PAGE*)List_Head(&pool->full_pages);
    while (page) {
        PAGE *next_page = (PAGE*)List_Next(page);
        if (((ULONG_PTR)pool & POOL_MASK_LEFT) != (ULONG_PTR)page)
            Pool_Free_Mem(page, page->eyecatcher);
        ++num_pages;
        page = next_page;
    }

#ifdef POOL_USE_CUSTOM_LOCK
#elif defined(KERNEL_MODE)
    ExDeleteResourceLite(pool->lock);
    ExFreePoolWithTag(pool->lock, pool->eyecatcher);
#else /* ! KERNEL_MODE */
    DeleteCriticalSection(&pool->lock);
#endif

    page = (PAGE *)((ULONG_PTR)pool & POOL_MASK_LEFT);
    Pool_Free_Mem(page, page->eyecatcher);

    Pool_Print_Timing();

    return num_pages;
}


//---------------------------------------------------------------------------
// Pool_Alloc
//---------------------------------------------------------------------------


ALIGNED void *Pool_Alloc(POOL *pool, ULONG size)
{
    if (size >= LARGE_CHUNK_MAXIMUM)
        return NULL;

    void *ptr = NULL;
    if (size) {

#if POOL_DEBUG
        ULONG caller_size = size;
        size += sizeof(ULONG);
#endif

        if (size > LARGE_CHUNK_MINIMUM)
            ptr = Pool_Get_Large_Chunk(pool, size);
        else
            ptr = Pool_Get_Cells(pool, NUM_CELLS(size));

#if POOL_DEBUG
        if (ptr) {
            *(ULONG *)((UCHAR *)ptr + caller_size) = size;
            // printf("put %d (at %08X) for %08X\n", size, (ULONG *)((UCHAR *)ptr + caller_size), ptr);
        }
#endif
    }

#if 0
#if defined(KERNEL_MODE)
    DbgPrint("Pool_Alloc (Pool=%08X) %08X (%d)\n", pool, ptr, size);
#endif
#endif

    return ptr;
}


//---------------------------------------------------------------------------
// Pool_Free
//---------------------------------------------------------------------------


ALIGNED void Pool_Free(void *ptr, ULONG size)
{
    if (ptr && size) {

#if POOL_DEBUG
        ULONG caller_size = size;
        size += sizeof(ULONG);
        if (*(ULONG *)((UCHAR *)ptr + caller_size) != size) {
            // printf("got %d (at %08X), expected %d for %08X\n", *(ULONG *)((UCHAR *)ptr + caller_size), (ULONG *)((UCHAR *)ptr + caller_size), size, ptr);
            ABEND(POOL_FREE_SIZE_MISMATCH);
        }
#endif

        // if the pointer is page-aligned, this must be a LARGE_CHUNK
        if (((ULONG_PTR)ptr & (POOL_PAGE_SIZE - 1)) == 0)
            Pool_Free_Large_Chunk(ptr, size);
        else
            Pool_Free_Cells(ptr, NUM_CELLS(size));

#if 0
#if defined(KERNEL_MODE)
    DbgPrint("Pool_Free                 %08X (%d)\n", ptr, size);
#endif
#endif

    } else
        ABEND(POOL_FREE_NULL_PTR_OR_ZERO_SIZE);
}


//---------------------------------------------------------------------------
// Pool_Print_Page
//---------------------------------------------------------------------------


#if 1
#define Pool_Print_Page(page)
#else


ALIGNED void Pool_Print_Page(PAGE *page)
{
    UCHAR *bitmap = (UCHAR *)page + PAGE_HEADER_SIZE;
    ULONG i;

    bitmap = (UCHAR *)page + PAGE_HEADER_SIZE;
    for (i = 0; i < NUM_PAGE_CELLS; ++i) {
        char ch;
        ULONG ofs = (i / 8);
        ULONG bit = 1 << (i & 7);
        if (bitmap[ofs] & bit)
            ch = '1';
        else
            ch = '0';
        if (i > 0 && (i % 60) == 0)
            printf("\n");
        printf("%c", ch);
    }
    printf("\n");
}


#endif


//---------------------------------------------------------------------------
// Pool_Find_Cells
//---------------------------------------------------------------------------


ALIGNED ULONG Pool_Find_Cells(PAGE *page, ULONG size)
{
    UCHAR *bitmap = (UCHAR *)page + PAGE_HEADER_SIZE;
    ULONG mask = 1;
    ULONG i = 0;

    Pool_Timing(NULL);

    // find the

    while (i < NUM_PAGE_CELLS) {
        ULONG bit = *bitmap & mask;
        ULONG j = i + 1;
        mask <<= 1;
        mask = (mask & 0xFF) | (mask >> 8);
        bitmap += mask & 1;
        if (bit) {
            while (j < NUM_PAGE_CELLS) {
                if (mask == 1 && *bitmap == 0xFF) {
                    j += 8;
                    ++bitmap;
                } else {
                    if ((*bitmap & mask) == 0)
                        break;
                    ++j;
                    mask <<= 1;
                    mask = (mask & 0xFF) | (mask >> 8);
                    bitmap += mask & 1;
                }
            }
        } else {
            while (j < NUM_PAGE_CELLS && j - i < size) {
                if (mask == 1 && *bitmap == 0) {
                    j += 8;
                    ++bitmap;
                } else {
                    if ((*bitmap & mask) != 0)
                        break;
                    ++j;
                    mask <<= 1;
                    mask = (mask & 0xFF) | (mask >> 8);
                    bitmap += mask & 1;
                }
            }
            if (j - i >= size) {
                // printf("Page %08X has %d consecutive free cells, starting at %d:\n", page, size, i);
                Pool_Print_Page(page);
                Pool_Timing(&Pool_Find_Cells_Time);
                return i;
            }
        }
        i = j;
    }

    Pool_Timing(&Pool_Find_Cells_Time);

    return -1;
}


//---------------------------------------------------------------------------
// Pool_Get_Cells
//---------------------------------------------------------------------------


ALIGNED void *Pool_Get_Cells(POOL *pool, ULONG size)
{
    POOL_DECLARE_IRQL
    PAGE *page, *next_page;
    UCHAR *bitmap;
    ULONG mask;
    UCHAR *ptr;
    ULONG index = -1;

    Pool_Timing(NULL);

    // look for a page that has enough free cells to satisfy the request

    Pool_Timing(NULL);

    POOL_LOCK(pages_lock);

    page = (PAGE*)List_Head(&pool->pages);
    while (page) {
        next_page = (PAGE*)List_Next(page);

        if (page->eyecatcher != pool->eyecatcher)
            ABEND(POOL_GET_CELLS_EYECATCHER_MISMATCH);

        if (page->num_free >= size) {

            // the page says it has at least the number of cells we need.
            // if we find a block of enough free cells, break here

            index = Pool_Find_Cells(page, size);
            if (index != -1)
                break;

            // if there were not enough free cells, then adjust the page
            // to say it has at most one less than what we just asked for

            page->num_free = (USHORT)(size - 1);

            // after adjusting, if the page crosses the threshold and has
            // too few free cells to bother with it again, we move it to
            // the list of full pages that are not traversed

            if (page->num_free < FULL_PAGE_THRESHOLD) {

                List_Remove(&pool->pages, page);
                List_Insert_Before(&pool->full_pages, NULL, page);
            }
        }

        page = next_page;
    }

    Pool_Timing(&Pool_Get_Cells_1_Time);

    // if we couldn't find a page, we have to allocate a new page.

    if (! page) {

        page = Pool_Alloc_Page(pool, pool->eyecatcher);
        if (! page) {
            POOL_UNLOCK(pages_lock);
            return NULL;
        }

        index = 0;
    }

    // after allocation, if the page crosses the threshold and has
    // too few free cells to bother with it again, we move it to
    // the list of full pages that are not traversed

    page->num_free = (USHORT)(page->num_free - size);

    if (page->num_free < FULL_PAGE_THRESHOLD) {

        List_Remove(&pool->pages, page);
        List_Insert_Before(&pool->full_pages, NULL, page);
    }

    // we have a block of consecutive free cells of at least 'size' cells,
    // mark it in use (or at least some of it)

    Pool_Timing(NULL);

    // printf("Satisfied request for %d cells ", size);

    bitmap = (UCHAR *)page + PAGE_HEADER_SIZE + (index / 8);
    mask = 1 << (index & 7);
    while (size) {
        if (mask == 1 && size > 8) {
            *bitmap = 0xFF;
            ++bitmap;
            size -= 8;
        } else {
            *bitmap |= mask;
            mask <<= 1;
            mask = (mask & 0xFF) | (mask >> 8);
            bitmap += mask & 1;
            --size;
        }
    }

    Pool_Timing(&Pool_Get_Cells_2_Time);

    ptr = (UCHAR *)page + PAGE_HEADER_SIZE + PAGE_BITMAP_SIZE
                        + index * POOL_CELL_SIZE;

    // printf(" at index %d address %08X (pool = %08X)\n", index, ptr, pool);
    // Pool_Print_Page(page);

    Pool_Timing(&Pool_Get_Cells_Time);

    // unlock and return

    POOL_UNLOCK(pages_lock);

    return ptr;
}


//---------------------------------------------------------------------------
// Pool_Free_Cells
//---------------------------------------------------------------------------


ALIGNED void Pool_Free_Cells(void *ptr, ULONG size)
{
    POOL_DECLARE_IRQL
    PAGE *page = (PAGE *)((ULONG_PTR)ptr & POOL_MASK_LEFT);
    ULONG index =
        (((ULONG)(ULONG_PTR)ptr & POOL_MASK_RIGHT)
            - PAGE_HEADER_SIZE - PAGE_BITMAP_SIZE)
        / POOL_CELL_SIZE;
    UCHAR *bitmap = (UCHAR *)page + PAGE_HEADER_SIZE + index / 8;
    ULONG mask = 1 << (index & 7);
    // printf("freeing ptr = %08X idx = %d (%d %d)\n", ptr, index, PAGE_HEADER_SIZE, PAGE_BITMAP_SIZE);

    POOL *pool = page->pool;

    //if ((page->eyecatcher != POOL_TAG) || (pool->eyecatcher != POOL_TAG))
    if (page->eyecatcher != pool->eyecatcher)
        ABEND(POOL_FREE_CELLS_EYECATCHER_MISMATCH);

    POOL_LOCK(pages_lock);

    // if after de-allocation, a full page crosses threshold in reverse,
    // we move it to the list of usable pages

    if (page->num_free < FULL_PAGE_THRESHOLD &&
            page->num_free + size >= FULL_PAGE_THRESHOLD) {

        List_Remove(&pool->full_pages, page);
        List_Insert_Before(&pool->pages, NULL, page);
    }

    page->num_free = (USHORT)(page->num_free + size);

    // indicate the cells are now free

    while (size) {
        if (mask == 1 && size > 8) {
            *bitmap = 0;
            ++bitmap;
            size -= 8;
        } else {
            *bitmap &= ~mask;
            mask <<= 1;
            mask = (mask & 0xFF) | (mask >> 8);
            bitmap += mask & 1;
            --size;
        }
    }

    POOL_UNLOCK(pages_lock);
}


//---------------------------------------------------------------------------
// Pool_Get_Large_Chunk
//---------------------------------------------------------------------------


ALIGNED void *Pool_Get_Large_Chunk(POOL *pool, ULONG size)
{
    POOL_DECLARE_IRQL
    ULONG large_chunk_size;
    void *ptr;
    LARGE_CHUNK *large_chunk;

    // pad the requested size to a multiple of POOL_PAGE_SIZE, after
    // adding the size of the (padded) LARGE_CHUNK structure
    large_chunk_size = (size + LARGE_CHUNK_SIZE + POOL_PAGE_SIZE - 1)
                     & ~(POOL_PAGE_SIZE - 1);

    ptr = Pool_Alloc_Mem(large_chunk_size, pool->eyecatcher);
    if (! ptr)
        return NULL;

    // position the LARGE_CHUNK structure at the end of the last page
    large_chunk = (LARGE_CHUNK *)
        ((UCHAR *)ptr + large_chunk_size - LARGE_CHUNK_SIZE);

    large_chunk->eyecatcher = pool->eyecatcher;
    large_chunk->pool = pool;
    large_chunk->ptr  = ptr;
    large_chunk->size = large_chunk_size;

    // lock the pool and add the allocated large chunk

    POOL_LOCK(large_chunks_lock);

    List_Insert_Before(&pool->large_chunks, NULL, large_chunk);

    POOL_UNLOCK(large_chunks_lock);

    return ptr;
}


//---------------------------------------------------------------------------
// Pool_Free_Large_Chunk
//---------------------------------------------------------------------------


ALIGNED void Pool_Free_Large_Chunk(void *ptr, ULONG size)
{
    POOL_DECLARE_IRQL
    ULONG large_chunk_size;
    LARGE_CHUNK *large_chunk;
    POOL *pool;

    // pad the requested size to a multiple of POOL_PAGE_SIZE, after
    // adding the size of the (padded) LARGE_CHUNK structure
    large_chunk_size = (size + LARGE_CHUNK_SIZE + POOL_PAGE_SIZE - 1)
                     & ~(POOL_PAGE_SIZE - 1);

    // LARGE_CHUNK structure is positioned at the end of the last page
    large_chunk = (LARGE_CHUNK *)
        ((UCHAR *)ptr + large_chunk_size - LARGE_CHUNK_SIZE);

    //if (large_chunk->eyecatcher != POOL_TAG)
    //    ABEND(POOL_FREE_LARGE_CHUNK_FIRST_EYECATCHER_MISMATCH);
    //pool = large_chunk->pool;
    //if (pool->eyecatcher != POOL_TAG)
    //    ABEND(POOL_FREE_LARGE_CHUNK_SECOND_EYECATCHER_MISMATCH);
    pool = large_chunk->pool;
    if (large_chunk->eyecatcher != pool->eyecatcher)
        ABEND(POOL_FREE_LARGE_CHUNK_SECOND_EYECATCHER_MISMATCH);

    // lock the pool and remove the freed large chunk

    POOL_LOCK(large_chunks_lock);

    List_Remove(&pool->large_chunks, large_chunk);

    POOL_UNLOCK(large_chunks_lock);

    Pool_Free_Mem(ptr, large_chunk->eyecatcher);
}
