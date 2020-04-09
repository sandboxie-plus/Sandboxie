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
// Proxy Handle
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "common/list.h"
#include "ProxyHandle.h"


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _PROXY_HANDLE {

    LIST_ELEM list_elem;
    HANDLE process_id;
    ULONG unique_id;
    ULONG refcount;
    struct _PROXY_HANDLE *p_proxy_handle;
    ULONG_PTR data;

} PROXY_HANDLE;


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


ProxyHandle::ProxyHandle(HANDLE heap, ULONG size_of_data,
                         P_ProxyHandle_CloseCallback close_callback,
                         void *context_for_callback)
{
    InitializeCriticalSectionAndSpinCount(&m_lock, 1000);
    List_Init(&m_list);

    m_close_callback = close_callback;
    m_context_for_callback = context_for_callback;

    if (! heap)
        heap = GetProcessHeap();
    m_heap = heap;

    m_size_of_data = size_of_data;

    m_unique_id = 'sbox';
}


//---------------------------------------------------------------------------
// Create
//---------------------------------------------------------------------------


ULONG ProxyHandle::Create(HANDLE process_id, void *model_data)
{
    PROXY_HANDLE *proxy = (PROXY_HANDLE *)HeapAlloc(
        m_heap, 0, sizeof(PROXY_HANDLE) + m_size_of_data);

    if (! proxy) {
        m_close_callback(m_context_for_callback, model_data);
        return 0;
    }

    proxy->process_id = process_id;

    proxy->unique_id = 0;
    while (! proxy->unique_id)
        proxy->unique_id = InterlockedIncrement(&m_unique_id);

    proxy->refcount = 1;

    proxy->p_proxy_handle = proxy;

    memcpy(&proxy->data, model_data, m_size_of_data);

    EnterCriticalSection(&m_lock);

    List_Insert_After(&m_list, NULL, proxy);

    LeaveCriticalSection(&m_lock);

    // WCHAR msg[128];wsprintf(msg, L"Proxy Handle Created With Process = %d Index = %d\n", process_id, proxy->unique_id); OutputDebugString(msg);

    return proxy->unique_id;
}


//---------------------------------------------------------------------------
// Find
//---------------------------------------------------------------------------


void *ProxyHandle::Find(HANDLE process_id, ULONG unique_id)
{
    void *proxy_data = NULL;

    if (process_id && unique_id) {

        EnterCriticalSection(&m_lock);

        PROXY_HANDLE *proxy = (PROXY_HANDLE *)List_Head(&m_list);
        while (proxy) {

            if (proxy->process_id == process_id  &&
                proxy->unique_id  == unique_id) {

                ++proxy->refcount;
                proxy_data = (void *)&proxy->data;
            }

            proxy = (PROXY_HANDLE *)List_Next(proxy);
        }

        LeaveCriticalSection(&m_lock);
    }

    return proxy_data;
}


//---------------------------------------------------------------------------
// Close
//---------------------------------------------------------------------------


void ProxyHandle::Close(void *proxy_data)
{
    ULONG_PTR p_p_proxy_handle = (ULONG_PTR)proxy_data - sizeof(ULONG_PTR);
    PROXY_HANDLE *proxy = *(PROXY_HANDLE **)p_p_proxy_handle;

    EnterCriticalSection(&m_lock);

    // WCHAR msg[128];wsprintf(msg, L"Proxy Handle Closing With Process = %d Index = %d\n", proxy->process_id, proxy->unique_id); OutputDebugString(msg);

    proxy->unique_id = 0;

    //
    // first, release the ref count added by the Find call that lead
    // to this Close call.  then, release the ref count added by the
    // Create call that created this PROXY_HANDLE
    //

    Release(proxy_data);
    Release(proxy_data);

    LeaveCriticalSection(&m_lock);
}


//---------------------------------------------------------------------------
// Release
//---------------------------------------------------------------------------


void ProxyHandle::Release(void *proxy_data)
{
    ULONG_PTR p_p_proxy_handle = (ULONG_PTR)proxy_data - sizeof(ULONG_PTR);
    PROXY_HANDLE *proxy = *(PROXY_HANDLE **)p_p_proxy_handle;

    EnterCriticalSection(&m_lock);

    if (proxy->refcount != 0)
        --proxy->refcount;

    if (proxy->refcount == 0) {

        // WCHAR msg[128];wsprintf(msg, L"Proxy Handle Deleting1 With Process = %d Index = %d\n", proxy->process_id, proxy->unique_id); OutputDebugString(msg);

        m_close_callback(m_context_for_callback, &proxy->data);

        List_Remove(&m_list, proxy);
        HeapFree(m_heap, 0, proxy);
    }

    LeaveCriticalSection(&m_lock);
}


//---------------------------------------------------------------------------
// ReleaseProcess
//---------------------------------------------------------------------------


void ProxyHandle::ReleaseProcess(HANDLE process_id)
{
    EnterCriticalSection(&m_lock);

    PROXY_HANDLE *proxy = (PROXY_HANDLE *)List_Head(&m_list);
    while (proxy) {

        PROXY_HANDLE *proxy_next = (PROXY_HANDLE *)List_Next(proxy);
        if (proxy->process_id == process_id) {

            if (proxy->refcount != 0)
                --proxy->refcount;

            if (proxy->refcount == 0) {

                // WCHAR msg[128];wsprintf(msg, L"Proxy Handle Deleting2 With Process = %d Index = %d\n", proxy->process_id, proxy->unique_id); OutputDebugString(msg);

                m_close_callback(m_context_for_callback, &proxy->data);

                List_Remove(&m_list, proxy);
                HeapFree(m_heap, 0, proxy);
            }
        }

        proxy = proxy_next;
    }

    LeaveCriticalSection(&m_lock);
}
