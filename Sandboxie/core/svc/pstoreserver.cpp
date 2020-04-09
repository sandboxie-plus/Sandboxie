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
// Protected Storage Server -- using PipeServer
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "pstoreserver.h"
#include "pstorewire.h"
#undef LOCK_EXCLUSIVE
#include "..\dll\pstore.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


typedef HRESULT (*P_PStoreCreateInstance)(
    IPStore **ppProvider, PST_PROVIDERID *pProviderID,
    void *pReserved, DWORD dwFlags);


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


PStoreServer::PStoreServer(PipeServer *pipeServer)
{
    m_pStore = NULL;

    pipeServer->Register(MSGID_PSTORE, this, Handler);

    QueueUserWorkItem(connectToPStore, this, WT_EXECUTELONGFUNCTION);
}


//---------------------------------------------------------------------------
// connectToPStore
//---------------------------------------------------------------------------


DWORD PStoreServer::connectToPStore(void *__this)
{
    PStoreServer *_this = (PStoreServer *)__this;

    Sleep(1000);

    HMODULE pstorec = LoadLibrary(L"pstorec.dll");
    if (pstorec) {
        P_PStoreCreateInstance func = (P_PStoreCreateInstance)
            GetProcAddress(pstorec, "PStoreCreateInstance");
        if (func) {
            IPStore *pStore;
            HRESULT hr = func(&pStore, NULL, NULL, 0);
            if (SUCCEEDED(hr)) {

                //
                // set the m_pStore member using interlocked instructions
                // to ensure a full memory barrier and no out-of-order
                // reads in other threads
                //

                InterlockedExchangePointer(&_this->m_pStore, pStore);
            }
        }
    }

    return 0;
}


//---------------------------------------------------------------------------
// Handler
//---------------------------------------------------------------------------


MSG_HEADER *PStoreServer::Handler(void *_this, MSG_HEADER *msg)
{
    PStoreServer *pThis = (PStoreServer *)_this;

    if (PipeServer::ImpersonateCaller(&msg) != 0)
        return msg;

    if (msg->msgid == MSGID_PSTORE_GET_TYPE_INFO)
        return pThis->GetTypeInfo(msg);

    if (msg->msgid == MSGID_PSTORE_GET_SUBTYPE_INFO)
        return pThis->GetSubtypeInfo(msg);

    if (msg->msgid == MSGID_PSTORE_READ_ITEM)
        return pThis->ReadItem(msg);

    if (msg->msgid == MSGID_PSTORE_ENUM_TYPES)
        return pThis->EnumTypes(msg);

    if (msg->msgid == MSGID_PSTORE_ENUM_ITEMS)
        return pThis->EnumItems(msg);

    return NULL;
}


//---------------------------------------------------------------------------
// GetTypeInfo
//---------------------------------------------------------------------------


MSG_HEADER *PStoreServer::GetTypeInfo(MSG_HEADER *msg)
{
    //
    // parse and execute request packet
    //

    PSTORE_GET_TYPE_INFO_REQ *req = (PSTORE_GET_TYPE_INFO_REQ *)msg;
    if (req->h.length != sizeof(PSTORE_GET_TYPE_INFO_REQ))
        return SHORT_REPLY(E_INVALIDARG);

    PST_TYPEINFO *pTypeInfo;
    HRESULT hr = PST_E_TYPE_NO_EXISTS;

    IPStore *pStore = (IPStore *)m_pStore;
    if (pStore) {
        hr = pStore->GetTypeInfo(
                    PST_KEY_CURRENT_USER, &req->type_id, (PPST_TYPEINFO**)&pTypeInfo, 0);
        if (hr != PST_E_OK) {
            hr = pStore->GetTypeInfo(
                    PST_KEY_LOCAL_MACHINE, &req->type_id, (PPST_TYPEINFO**)&pTypeInfo, 0);
        }
    }

    if (FAILED(hr))
        return SHORT_REPLY(hr);

    //
    // build reply packet
    //

    ULONG rpl_len = sizeof(PSTORE_GET_TYPE_INFO_RPL)
                  + (wcslen(pTypeInfo->szDisplayName) + 1) * sizeof(WCHAR);
    PSTORE_GET_TYPE_INFO_RPL *rpl =
        (PSTORE_GET_TYPE_INFO_RPL *)LONG_REPLY(rpl_len);

    if (rpl) {

        rpl->flags = 0;
        rpl->name_len = wcslen(pTypeInfo->szDisplayName);
        wcscpy(rpl->name, pTypeInfo->szDisplayName);
    }

    CoTaskMemFree(pTypeInfo);

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// GetSubtypeInfo
//---------------------------------------------------------------------------


MSG_HEADER *PStoreServer::GetSubtypeInfo(MSG_HEADER *msg)
{
    //
    // parse and execute request packet
    //

    PSTORE_GET_SUBTYPE_INFO_REQ *req = (PSTORE_GET_SUBTYPE_INFO_REQ *)msg;
    if (req->h.length != sizeof(PSTORE_GET_SUBTYPE_INFO_REQ))
        return SHORT_REPLY(E_INVALIDARG);

    PST_TYPEINFO *pTypeInfo;
    HRESULT hr = PST_E_TYPE_NO_EXISTS;

    IPStore *pStore = (IPStore *)m_pStore;
    if (pStore) {

        hr = pStore->GetSubtypeInfo(
            PST_KEY_CURRENT_USER, &req->type_id, &req->subtype_id,
            (PPST_TYPEINFO**)&pTypeInfo, 0);
        if (hr != PST_E_OK) {
            hr = pStore->GetSubtypeInfo(
                PST_KEY_LOCAL_MACHINE, &req->type_id, &req->subtype_id,
                (PPST_TYPEINFO**)&pTypeInfo, 0);
        }
    }

    if (FAILED(hr))
        return SHORT_REPLY(hr);

    //
    // build reply packet
    //

    ULONG rpl_len = sizeof(PSTORE_GET_SUBTYPE_INFO_RPL)
                  + (wcslen(pTypeInfo->szDisplayName) + 1) * sizeof(WCHAR);
    PSTORE_GET_SUBTYPE_INFO_RPL *rpl =
        (PSTORE_GET_SUBTYPE_INFO_RPL *)LONG_REPLY(rpl_len);

    if (rpl) {

        rpl->flags = 0;
        rpl->name_len = wcslen(pTypeInfo->szDisplayName);
        wcscpy(rpl->name, pTypeInfo->szDisplayName);
    }

    CoTaskMemFree(pTypeInfo);

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// ReadItem
//---------------------------------------------------------------------------

#define  PST_PF_NEVER_SHOW   0x00000002 

MSG_HEADER *PStoreServer::ReadItem(MSG_HEADER *msg)
{
    ;
    BYTE *data;
    ULONG data_len;

    //
    // parse and execute request packet
    //

    PSTORE_READ_ITEM_REQ *req = (PSTORE_READ_ITEM_REQ *)msg;
    if (req->h.length < sizeof(PSTORE_READ_ITEM_REQ))
        return SHORT_REPLY(E_INVALIDARG);

    ULONG name_len = req->name_len * sizeof(WCHAR);
    if (name_len > PIPE_MAX_DATA_LEN)
        return SHORT_REPLY(E_INVALIDARG);
    if (FIELD_OFFSET(PSTORE_READ_ITEM_REQ, name) + name_len > req->h.length)
        return SHORT_REPLY(E_INVALIDARG);
    name_len += sizeof(WCHAR);
    WCHAR *name = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, name_len);
    if (! name)
        return NULL;
    memset(name, 0, name_len);
    memcpy(name, req->name, name_len - sizeof(WCHAR));

    PST_PROMPTINFO aPromptInfo;
    aPromptInfo.cbSize = sizeof(PST_PROMPTINFO);
    aPromptInfo.dwPromptFlags = PST_PF_NEVER_SHOW;
    aPromptInfo.hwndApp = NULL;
    aPromptInfo.szPrompt = NULL;

    HRESULT hr = PST_E_ITEM_NO_EXISTS;

    IPStore *pStore = (IPStore *)m_pStore;
    if (pStore) {

        hr = pStore->ReadItem(
                PST_KEY_CURRENT_USER, &req->type_id, &req->subtype_id, name,
                &data_len, &data, &aPromptInfo, 0);
        if (hr != PST_E_OK) {
            hr = pStore->ReadItem(
                PST_KEY_LOCAL_MACHINE, &req->type_id, &req->subtype_id, name,
                &data_len, &data, &aPromptInfo, 0);
        }
    }

    HeapFree(GetProcessHeap(), 0, name);

    if (FAILED(hr))
        return SHORT_REPLY(hr);

    //
    // build reply packet
    //

    ULONG rpl_len = sizeof(PSTORE_READ_ITEM_RPL) + data_len;
    PSTORE_READ_ITEM_RPL *rpl = (PSTORE_READ_ITEM_RPL *)LONG_REPLY(rpl_len);

    if (rpl) {

        rpl->data_len = data_len;
        memcpy(rpl->data, data, data_len);
    }

    if (data)
        CoTaskMemFree(data);

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// EnumTypes
//---------------------------------------------------------------------------


MSG_HEADER *PStoreServer::EnumTypes(MSG_HEADER *msg)
{
    //
    // parse and execute request packet
    //

    PSTORE_ENUM_TYPES_REQ *req = (PSTORE_ENUM_TYPES_REQ *)msg;
    if (req->h.length != sizeof(PSTORE_ENUM_TYPES_REQ))
        return SHORT_REPLY(E_INVALIDARG);

    ULONG count = 0, n;
    IEnumPStoreTypes *pEnum = NULL;
    GUID guid;
    HRESULT hr = E_FAIL;

    IPStore *pStore = (IPStore *)m_pStore;
    if (pStore) {

        if (! req->enum_subtypes)
            hr = pStore->EnumTypes(req->pst_key, 0, &pEnum);
        else
            hr = pStore->EnumSubtypes(
                                   req->pst_key, &req->type_id, 0, &pEnum);

        while (SUCCEEDED(hr)) {
            hr = pEnum->Next(1, &guid, &n);
            count += n;
        }
    }

    if (FAILED(hr)) {
        if (pEnum)
            pEnum->Release();
        return SHORT_REPLY(hr);
    }

    //
    // build reply packet
    //

    ULONG rpl_len = sizeof(PSTORE_ENUM_TYPES_RPL) + count * sizeof(GUID);
    PSTORE_ENUM_TYPES_RPL *rpl =
        (PSTORE_ENUM_TYPES_RPL *)LONG_REPLY(rpl_len);

    if (rpl) {

        pEnum->Reset();

        hr = pEnum->Next(count, rpl->guids, &rpl->count);
        if (FAILED(hr)) {
            rpl->count = 0;
            rpl->h.status = hr;
        }
    }

    pEnum->Release();

    return S_OK;
}


//---------------------------------------------------------------------------
// EnumItems
//---------------------------------------------------------------------------


MSG_HEADER *PStoreServer::EnumItems(MSG_HEADER *msg)
{
    //
    // parse and execute request packet
    //

    PSTORE_ENUM_ITEMS_REQ *req = (PSTORE_ENUM_ITEMS_REQ *)msg;
    if (req->h.length != sizeof(PSTORE_ENUM_ITEMS_REQ))
        return SHORT_REPLY(E_INVALIDARG);

    ULONG rpl_len = sizeof(PSTORE_ENUM_ITEMS_RPL);
    ULONG count = 0, n;
    IEnumPStoreItems *pEnum = NULL;
    WCHAR *name;
    HRESULT hr = E_FAIL;

    IPStore *pStore = (IPStore *)m_pStore;
    if (pStore) {

        hr = pStore->EnumItems(
                req->pst_key, &req->type_id, &req->subtype_id, 0, &pEnum);

        while (SUCCEEDED(hr)) {
            hr = pEnum->Next(1, &name, &n);
            if (n) {
                rpl_len += (wcslen(name) + 4) * sizeof(WCHAR);
                CoTaskMemFree(name);
                ++count;
            }
        }
    }

    if (FAILED(hr)) {
        if (pEnum)
            pEnum->Release();
        return SHORT_REPLY(hr);
    }

    //
    // build reply packet
    //

    PSTORE_ENUM_ITEMS_RPL *rpl =
        (PSTORE_ENUM_ITEMS_RPL *)LONG_REPLY(rpl_len);

    if (rpl) {

        rpl->count = 0;

        WCHAR *out_name = &rpl->names[0];
        *out_name = L'\0';

        pEnum->Reset();

        while (1) {
            hr = pEnum->Next(1, &name, &n);
            if (SUCCEEDED(hr)) {
                wcscpy(out_name, name);
                out_name += wcslen(out_name) + 1;
                CoTaskMemFree(name);
                ++rpl->count;
            } else {
                rpl->h.status = hr;
                break;
            }
        }
    }

    pEnum->Release();
    return (MSG_HEADER *)rpl;
}
