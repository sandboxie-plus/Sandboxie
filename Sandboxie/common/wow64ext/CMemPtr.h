/**
 *
 * WOW64Ext Library
 *
 * Copyright (c) 2014 ReWolf
 * http://blog.rewolf.pl/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#pragma once

class CMemPtr
{
private:
    void** m_ptr;
    bool watchActive;

public:
    CMemPtr(void** ptr) : m_ptr(ptr), watchActive(true) {}

    ~CMemPtr()
    {
        if (*m_ptr && watchActive)
        { 
            HeapFree(GetProcessHeap(), 0, *m_ptr); 
            *m_ptr = 0; 
        } 
    }

    static void* Alloc(size_t size) {
        return HeapAlloc(GetProcessHeap(), 0, size);
    }

    void disableWatch() { watchActive = false; }
};

#define NEW(size) \
    CMemPtr::Alloc(size)

#define WATCH(ptr) \
    CMemPtr watch_##ptr((void**)&ptr)

#define DISABLE_WATCH(ptr) \
    watch_##ptr.disableWatch()
