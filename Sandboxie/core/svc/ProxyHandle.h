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


#ifndef _MY_PROXYHANDLE_H
#define _MY_PROXYHANDLE_H


typedef void (*P_ProxyHandle_CloseCallback)(void *context, void *proxy_data);


class ProxyHandle
{

public:

    ProxyHandle(HANDLE heap, ULONG size_of_data,
                P_ProxyHandle_CloseCallback close_callback,
                void *context_for_callback);

    ULONG Create(HANDLE process_id, void *model_data);

    void *Find(HANDLE process_id, ULONG unique_id);

    void Close(void *proxy_data);

    void Release(void *proxy_data);

    void ReleaseProcess(HANDLE process_id);

protected:

    CRITICAL_SECTION m_lock;
    LIST m_list;

    P_ProxyHandle_CloseCallback m_close_callback;
    void *m_context_for_callback;

    HANDLE m_heap;

    ULONG m_size_of_data;

    volatile LONG m_unique_id;

};


#endif /* _MY_PROXYHANDLE_H */
