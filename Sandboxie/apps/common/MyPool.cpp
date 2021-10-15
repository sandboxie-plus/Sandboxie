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
// MyPool Memory Management
//---------------------------------------------------------------------------


#include <windows.h>
#include "MyPool.h"
#include "common/pool.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define DEBUG_MEMORY


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CMyPool::CMyPool()
{
    m_pool = Pool_Create();
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CMyPool::~CMyPool()
{
    if (m_pool)
        Pool_Delete((POOL *)m_pool);
}


//---------------------------------------------------------------------------
// Alloc
//---------------------------------------------------------------------------


void *CMyPool::Alloc(int size)
{
    UCHAR *ptr;

#ifdef DEBUG_MEMORY
    size += 64 * 2;
#endif // DEBUG_MEMORY

    size += sizeof(ULONG_PTR);
    ptr = (UCHAR *)Pool_Alloc((POOL *)m_pool, size);
    if (! ptr)
        __debugbreak();

#ifdef DEBUG_MEMORY
    memset(ptr,             0xCC, 64);
    memset(ptr + size - 64, 0xCC, 64);
    ptr += 64;
#endif // DEBUG_MEMORY

    *(ULONG_PTR *)ptr = size;
    ptr += sizeof(ULONG_PTR);
    return ptr;
}


//---------------------------------------------------------------------------
// Free
//---------------------------------------------------------------------------


void CMyPool::Free(void *ptr)
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
            wsprintf(txt, L"Memory corrupted, ptr=%p\n", ptr);
            OutputDebugString(txt);
            __debugbreak();
        }

    ptr2 -= 64;
    }
#endif // DEBUG_MEMORY

    Pool_Free(ptr2, size);
}
