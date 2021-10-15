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
//
// Search Dummy Object
//
//---------------------------------------------------------------------------


#include <new>


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static const GUID IID_ISearchNotifyInlineSite =
    { 0xB5702E61, 0xE75C, 0x4B64,
                    { 0x82, 0xA1, 0x6C, 0xB4, 0xF8, 0x32, 0xFC, 0xCF } };


//---------------------------------------------------------------------------
// FindOrCreateDummySlaveObject
//---------------------------------------------------------------------------


void *ComServer::FindOrCreateDummySlaveObject(void *_map, LIST *ObjectsList,
                                              ULONG *exc, HRESULT *hr)
{
    COM_SLAVE_MAP *pMap = (COM_SLAVE_MAP *)_map;

    COM_OBJECT *obj = (COM_OBJECT *)List_Head(ObjectsList);
    while (obj) {
        if (pMap->idProcess == obj->idProcess &&
            pMap->objidx    == -1 &&
            memcmp(pMap->Buffer, &obj->iid, sizeof(GUID)) == 0)
        {
            break;
        }
        obj = (COM_OBJECT *)List_Next(obj);
    }

    if (! obj) {

        void *pUnknown = CreateDummySlaveObject((GUID *)pMap->Buffer);

        if (pUnknown) {

            obj = (COM_OBJECT *)HeapAlloc(m_heap, 0, sizeof(COM_OBJECT));
            if (obj) {

                memzero(obj, sizeof(COM_OBJECT));

                obj->pUnknown = (IUnknown *)pUnknown;

                obj->idProcess = pMap->idProcess;

                obj->objidx   = -1;

                memcpy(&obj->iid, pMap->Buffer, sizeof(GUID));

                List_Insert_After(ObjectsList, NULL, obj);

            } else {

                ((IUnknown *)pUnknown)->Release();
                *exc = RPC_S_OUT_OF_RESOURCES;
                *hr = E_OUTOFMEMORY;
            }

        } else {

            *exc = RPC_S_OBJECT_NOT_FOUND;
            *hr = E_ABORT;
        }
    }

    return obj;
}


//---------------------------------------------------------------------------
//
// ISearchNotifyInlineSite
//
//---------------------------------------------------------------------------


enum _SEARCH_INDEXING_PHASE {
    SEARCH_INDEXING_PHASE_GATHERER  = 0,
    SEARCH_INDEXING_PHASE_QUERYABLE = 1,
    SEARCH_INDEXING_PHASE_PERSISTED = 2
}   SEARCH_INDEXING_PHASE;


typedef struct _SEARCH_ITEM_INDEXING_STATUS
{
    DWORD dwDocID;
    HRESULT hrIndexingStatus;
} SEARCH_ITEM_INDEXING_STATUS;


class MySearchNotifyInlineSite : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
        REFIID riid, void **ppvObject)
    {
        if (memcmp(&riid, &IID_ISearchNotifyInlineSite, sizeof(GUID)) == 0 ||
            memcmp(&riid, &IID_IUnknown, sizeof(GUID)) == 0)
        {
            *ppvObject = this;
            return S_OK;
        } else {
            *ppvObject = NULL;
            return E_NOINTERFACE;
        }
    }

    virtual ULONG STDMETHODCALLTYPE AddRef(void)
    {
        return 1;
    }

    virtual ULONG STDMETHODCALLTYPE Release(void)
    {
        return 1;
    }

    virtual HRESULT STDMETHODCALLTYPE OnItemIndexedStatusChange(
        ULONG sipStatus,
        ULONG dwNumEntries,
        void *rgItemStatusEntries)
    {
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE OnCatalogStatusChange(
        REFGUID guidCatalogResetSignature,
        REFGUID guidCheckPointSignature,
        ULONG dwLastCheckPointNumber)
    {
        return S_OK;
    }
};


//---------------------------------------------------------------------------
// CreateDummySlaveObject
//---------------------------------------------------------------------------


void *ComServer::CreateDummySlaveObject(GUID *riid)
{
    void *pUnknown = NULL;

    if (memcmp(riid, &IID_ISearchNotifyInlineSite, sizeof(GUID)) == 0) {

        pUnknown = HeapAlloc(m_heap, 0, sizeof(MySearchNotifyInlineSite));
        pUnknown = new (pUnknown) MySearchNotifyInlineSite();
    }

    return pUnknown;
}


//---------------------------------------------------------------------------
//
// Crypt Protect/Unprotect Data Service
//
//---------------------------------------------------------------------------


#include <wincrypt.h>


//---------------------------------------------------------------------------
// CryptProtectDataSlave
//---------------------------------------------------------------------------


ULONG ComServer::CryptProtectDataSlave(void *Buffer)
{
    COM_CRYPT_PROTECT_DATA_REQ *req = (COM_CRYPT_PROTECT_DATA_REQ *)Buffer;
    ULONG req_len, rpl_len, descr_len, offset;
    CRYPTPROTECT_PROMPTSTRUCT PromptStruct;
    DATA_BLOB DataIn, DataOut;
    DATA_BLOB Entropy, *pEntropy;
    WCHAR *DataDescr;
    BOOL ok;

    req_len = req->h.length;
    if (req_len < sizeof(COM_CRYPT_PROTECT_DATA_REQ))
        return ERROR_INVALID_PARAMETER;

    //
    // prompt struct
    //

    PromptStruct.cbSize = sizeof(CRYPTPROTECT_PROMPTSTRUCT);
    PromptStruct.dwPromptFlags = req->prompt_flags;
    PromptStruct.hwndApp = NULL; // (HWND)(ULONG_PTR)req->prompt_hwnd;
    PromptStruct.szPrompt = req->prompt_text;
    req->prompt_text[95] = L'\0';

    //
    // data field
    //

    offset = FIELD_OFFSET(COM_CRYPT_PROTECT_DATA_REQ, data);
    if (offset + req->data_len > req_len)
        return ERROR_INVALID_PARAMETER;

    DataIn.cbData = req->data_len;
    DataIn.pbData = (UCHAR *)req + offset;

    //
    // entropy field
    //

    offset += req->data_len;
    if (offset + req->entropy_len > req_len)
        return ERROR_INVALID_PARAMETER;

    Entropy.cbData = req->entropy_len;
    Entropy.pbData = (UCHAR *)req + offset;

    if (req->entropy_len)
        pEntropy = &Entropy;
    else
        pEntropy = NULL;

    //
    // parse descr field (for CryptProtectData call)
    // then invoke CryptProtectData or CryptUnprotectData
    //

    DataDescr = NULL;

    if (req->mode == L'P') {

        offset += req->entropy_len;
        if (offset + req->descr_len * sizeof(WCHAR) > req_len)
            return ERROR_INVALID_PARAMETER;

        if (req->descr_len) {

            DataDescr = (WCHAR *)((UCHAR *)req + offset);
            DataDescr[req->descr_len] = L'\0';
        }

        ok = CryptProtectData(&DataIn, DataDescr, pEntropy, NULL,
                              &PromptStruct, req->flags, &DataOut);

        DataDescr = NULL;

    } else if (req->mode == L'U') {

        ok = CryptUnprotectData(&DataIn, &DataDescr, pEntropy, NULL,
                                &PromptStruct, req->flags, &DataOut);

    } else
        return ERROR_INVALID_PARAMETER;

    if (! ok)
        return GetLastError();

    //
    // copy result data
    //

    rpl_len = sizeof(COM_CRYPT_PROTECT_DATA_RPL)
            + DataOut.cbData;

    if (DataDescr) {
        descr_len = wcslen(DataDescr);
        rpl_len += (descr_len + 1) * sizeof(WCHAR);
    } else
        descr_len = 0;

    if (rpl_len < MAX_MAP_BUFFER_LENGTH) {

        COM_CRYPT_PROTECT_DATA_RPL *rpl =
            (COM_CRYPT_PROTECT_DATA_RPL *)Buffer;

        rpl->h.length = rpl_len;
        rpl->h.status = 0;

        rpl->data_len = DataOut.cbData;
        memcpy(rpl->data, DataOut.pbData, rpl->data_len);

        rpl->descr_len = descr_len;
        memcpy(rpl->data + rpl->data_len, DataDescr, descr_len);
    }

    if (DataDescr)
        LocalFree(DataDescr);
    if (DataOut.pbData)
        LocalFree(DataOut.pbData);

    return (rpl_len < MAX_MAP_BUFFER_LENGTH ? 0 : ERROR_SECRET_TOO_LONG);
}
