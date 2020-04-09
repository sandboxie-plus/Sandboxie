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



extern "C" {
#include "dll.h"
}
#include <stdio.h>
#include "ipstore_enum.h"
#include "core/svc/PStoreWire.h"


typedef LPVOID (*P_CoTaskMemAlloc)(ULONG cb);


//---------------------------------------------------------------------------
// IEnumPStoreGeneric Default Constructor
//---------------------------------------------------------------------------


IEnumPStoreGeneric::IEnumPStoreGeneric()
{
    m_list = (IEnumPStoreList *)Dll_Alloc(sizeof(IEnumPStoreList));
    List_Init(&m_list->list);
    m_list->refcount = 1;
    m_list->error = FALSE;

    m_current = NULL;

    m_refcount = 1;
}


//---------------------------------------------------------------------------
// IEnumPStoreGeneric Copy Constructor
//---------------------------------------------------------------------------


IEnumPStoreGeneric::IEnumPStoreGeneric(
    const IEnumPStoreGeneric *model)
{
    m_list = model->m_list;
    ++m_list->refcount;

    m_current = model->m_current;

    m_refcount = 1;
}


//---------------------------------------------------------------------------
// IEnumPStoreGeneric Destructor
//---------------------------------------------------------------------------


IEnumPStoreGeneric::~IEnumPStoreGeneric()
{
    --m_list->refcount;
    if (m_list->refcount == 0) {
        while (1) {
            IEnumPStoreListElem *elem =
                (IEnumPStoreListElem *)List_Head(&m_list->list);
            if (! elem)
                break;
            List_Remove(&m_list->list, elem);
            Dll_Free(elem);
        }
        Dll_Free(m_list);
    }
}


//---------------------------------------------------------------------------
// IEnumPStoreGeneric::GenericSkip
//---------------------------------------------------------------------------


HRESULT IEnumPStoreGeneric::GenericSkip(DWORD celt)
{
    if (m_list->error)
        return PST_E_TYPE_NO_EXISTS;
    if (celt) {
        if (List_Count(&m_list->list) == 0)
            return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        while (celt) {
            if (m_current) {
                m_current = (IEnumPStoreListElem *)List_Next(m_current);
                --celt;
            } else
                return HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
        }
    }
    return S_OK;
}


//---------------------------------------------------------------------------
// IEnumPStoreGeneric::GenericReset
//---------------------------------------------------------------------------


HRESULT IEnumPStoreGeneric::GenericReset()
{
    if (m_list->error)
        return PST_E_TYPE_NO_EXISTS;
    m_current = (IEnumPStoreListElem *)List_Head(&m_list->list);
    return S_OK;
}


//---------------------------------------------------------------------------
// IEnumPStoreGeneric Operator new
//---------------------------------------------------------------------------


void *IEnumPStoreGeneric::operator new(size_t n)
{
    return Dll_Alloc(n);
}


//---------------------------------------------------------------------------
// IEnumPStoreGeneric Operator delete
//---------------------------------------------------------------------------


void IEnumPStoreGeneric::operator delete(void *p)
{
    return Dll_Free(p);
}


//---------------------------------------------------------------------------
// IEnumPStoreTypesImpl::CreateEnumType
//---------------------------------------------------------------------------


IEnumPStoreTypes *IEnumPStoreTypesImpl::CreateEnumType(
    IPStoreImpl *pst, ULONG pst_key)
{
    IEnumPStoreTypesImpl *penum = new IEnumPStoreTypesImpl();
    ULONG i;

    PSTORE_ENUM_TYPES_REQ req;
    memset(&req, 0, sizeof(req));
    req.h.length = sizeof(PSTORE_ENUM_TYPES_REQ);
    req.h.msgid = MSGID_PSTORE_ENUM_TYPES;

    req.pst_key = pst_key;

    PSTORE_ENUM_TYPES_RPL *rpl =
        (PSTORE_ENUM_TYPES_RPL *)SbieDll_CallServer(&req.h);

    if (rpl && rpl->h.status == 0) {

        for (i = 0; i < rpl->count; ++i)
            penum->InsertSorted(&rpl->guids[i]);
    }

    if (rpl)
        Dll_Free(rpl);

    if (pst_key == PST_KEY_CURRENT_USER) {

        IPStoreImpl::PsType *elem =
            (IPStoreImpl::PsType *)List_Head(&pst->types);
        while (elem) {
            penum->InsertSorted(&elem->type_id);
            elem = (IPStoreImpl::PsType *)List_Next(elem);
        }
    }

    penum->GenericReset();
    return penum;
}


//---------------------------------------------------------------------------
// IEnumPStoreTypesImpl::CreateEnumSubtype
//---------------------------------------------------------------------------


IEnumPStoreTypes *IEnumPStoreTypesImpl::CreateEnumSubtype(
    IPStoreImpl *pst, ULONG pst_key, const GUID *type_guid)
{
    IEnumPStoreTypesImpl *penum = new IEnumPStoreTypesImpl();
    ULONG i;

    IPStoreImpl::PsType *type_elem = pst->find_type(type_guid);
    if (! type_elem) {
        penum->m_list->error = TRUE;
        return penum;
    }

    PSTORE_ENUM_TYPES_REQ req;
    memset(&req, 0, sizeof(req));
    req.h.length = sizeof(PSTORE_ENUM_TYPES_REQ);
    req.h.msgid = MSGID_PSTORE_ENUM_TYPES;

    req.pst_key = pst_key;
    req.type_id = *type_guid;
    req.enum_subtypes = TRUE;

    PSTORE_ENUM_TYPES_RPL *rpl =
        (PSTORE_ENUM_TYPES_RPL *)SbieDll_CallServer(&req.h);

    if (rpl && rpl->h.status == 0) {

        for (i = 0; i < rpl->count; ++i)
            penum->InsertSorted(&rpl->guids[i]);
    }

    if (rpl)
        Dll_Free(rpl);

    if (pst_key == PST_KEY_CURRENT_USER) {

        IPStoreImpl::PsSubtype *elem =
            (IPStoreImpl::PsSubtype *)List_Head(&type_elem->subtypes);
        while (elem) {
            penum->InsertSorted(&elem->subtype_id);
            elem = (IPStoreImpl::PsSubtype *)List_Next(elem);
        }
    }

    penum->GenericReset();
    return penum;
}


//---------------------------------------------------------------------------
// IEnumPStoreTypesImpl Defeault Constructor
//---------------------------------------------------------------------------


IEnumPStoreTypesImpl::IEnumPStoreTypesImpl()
: IEnumPStoreGeneric()
{
}


//---------------------------------------------------------------------------
// IEnumPStoreTypesImpl Copy Constructor
//---------------------------------------------------------------------------


IEnumPStoreTypesImpl::IEnumPStoreTypesImpl(
    const IEnumPStoreTypesImpl *model)
: IEnumPStoreGeneric(model)
{
}


//---------------------------------------------------------------------------
// IEnumPStoreTypesImpl Destructor
//---------------------------------------------------------------------------


IEnumPStoreTypesImpl::~IEnumPStoreTypesImpl(void)
{
}


//---------------------------------------------------------------------------
// IEnumPStoreTypesImpl::StringFromGUID
//---------------------------------------------------------------------------


void IEnumPStoreTypesImpl::StringFromGUID(const GUID *guid, WCHAR *str)
{
    struct _s {
        ULONG a;
        USHORT b;
        USHORT c;
        UCHAR x1;
        UCHAR x2;
        UCHAR y1;
        UCHAR y2;
        UCHAR y3;
        UCHAR y4;
        UCHAR y5;
        UCHAR y6;
    } *s = (_s *)guid;
    Sbie_swprintf(str, L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        s->a, s->b, s->c,
        s->x1, s->x2,
        s->y1, s->y2, s->y3, s->y4, s->y5, s->y6);
}


//---------------------------------------------------------------------------
// IEnumPStoreTypesImpl::InsertSorted
//---------------------------------------------------------------------------


void IEnumPStoreTypesImpl::InsertSorted(GUID *guid)
{
    WCHAR guidL[48], guidR[48];

    StringFromGUID(guid, guidR);
    IEnumPStoreListElem *elem =
        (IEnumPStoreListElem *)List_Head(&m_list->list);
    while (elem) {
        StringFromGUID(&elem->v.guid, guidL);
        int c = wcscmp(guidL, guidR);
        if (c == 0)
            return;
        if (c > 0)
            break;
        elem = (IEnumPStoreListElem *)List_Next(elem);
    }

    IEnumPStoreListElem *new_elem =
        (IEnumPStoreListElem *)Dll_Alloc(sizeof(IEnumPStoreListElem));
    new_elem->v.guid = *guid;
    if (elem)
        List_Insert_Before(&m_list->list, elem, new_elem);
    else
        List_Insert_After(&m_list->list, NULL, new_elem);

}


//---------------------------------------------------------------------------
// IEnumPStoreTypesImpl::QueryInterface
//---------------------------------------------------------------------------


HRESULT IEnumPStoreTypesImpl::QueryInterface(REFIID iid, void **ppvObject)
{
    this->AddRef();
    *ppvObject = this;
    return S_OK;
}


//---------------------------------------------------------------------------
// IEnumPStoreTypesImpl::AddRef
//---------------------------------------------------------------------------


ULONG IEnumPStoreTypesImpl::AddRef()
{
    ++m_refcount;
    return m_refcount;
}


//---------------------------------------------------------------------------
// IEnumPStoreTypesImpl::Release
//---------------------------------------------------------------------------


ULONG IEnumPStoreTypesImpl::Release()
{
    --m_refcount;
    if (m_refcount)
        return m_refcount;
    delete this;
    return 0;
}


//---------------------------------------------------------------------------
// IEnumPStoreTypesImpl::Next
//---------------------------------------------------------------------------


HRESULT IEnumPStoreTypesImpl::Next(
    DWORD celt, GUID *rgelt, DWORD *pceltFetched)
{
    if (pceltFetched)
        *pceltFetched = 0;
    if (m_list->error)
        return PST_E_TYPE_NO_EXISTS;
    if (celt) {
        if (List_Count(&m_list->list) == 0)
            return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        while (celt) {
            if (m_current) {
                *rgelt = m_current->v.guid;
                ++rgelt;
                if (pceltFetched)
                    ++*pceltFetched;
                m_current = (IEnumPStoreListElem *)List_Next(m_current);
                --celt;
            } else
                return HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
        }
    }
    return S_OK;
}


//---------------------------------------------------------------------------
// IEnumPStoreTypesImpl::Skip
//---------------------------------------------------------------------------


HRESULT IEnumPStoreTypesImpl::Skip(DWORD celt)
{
    return GenericSkip(celt);
}


//---------------------------------------------------------------------------
// IEnumPStoreTypesImpl::Reset
//---------------------------------------------------------------------------


HRESULT IEnumPStoreTypesImpl::Reset()
{
    return GenericReset();
}


//---------------------------------------------------------------------------
// IEnumPStoreTypesImpl::Clone
//---------------------------------------------------------------------------


HRESULT IEnumPStoreTypesImpl::Clone(IEnumPStoreTypes **ppenum)
{
    *ppenum = new IEnumPStoreTypesImpl(this);
    return S_OK;
}

//---------------------------------------------------------------------------
// IEnumPStoreItemsImpl::CreateEnumItem
//---------------------------------------------------------------------------


IEnumPStoreItems *IEnumPStoreItemsImpl::CreateEnumItem(
    IPStoreImpl *pst, ULONG pst_key,
    const GUID *type_guid, const GUID *subtype_guid)
{
    IEnumPStoreItemsImpl *penum =
        new IEnumPStoreItemsImpl(pst->CoTaskMemAlloc);
    ULONG i;

    IPStoreImpl::PsType *type_elem = pst->find_type(type_guid);
    if (! type_elem) {
        penum->m_list->error = TRUE;
        return penum;
    }
    IPStoreImpl::PsSubtype *subtype_elem =
        pst->find_subtype(type_elem, subtype_guid);
    if (! subtype_elem) {
        penum->m_list->error = TRUE;
        return penum;
    }

    PSTORE_ENUM_ITEMS_REQ req;
    memset(&req, 0, sizeof(req));
    req.h.length = sizeof(PSTORE_ENUM_ITEMS_REQ);
    req.h.msgid = MSGID_PSTORE_ENUM_ITEMS;

    req.pst_key = pst_key;
    req.type_id = *type_guid;
    req.subtype_id = *subtype_guid;

    PSTORE_ENUM_ITEMS_RPL *rpl =
        (PSTORE_ENUM_ITEMS_RPL *)SbieDll_CallServer(&req.h);

    if (rpl && rpl->h.status == 0) {

        WCHAR *name = &rpl->names[0];
        for (i = 0; i < rpl->count; ++i) {
            penum->InsertSorted(name);
            name += wcslen(name) + 1;
        }
    }

    if (rpl)
        Dll_Free(rpl);

    if (pst_key == PST_KEY_CURRENT_USER) {

        IPStoreImpl::PsItem *elem =
            (IPStoreImpl::PsItem *)List_Head(&subtype_elem->items);
        while (elem) {
            penum->InsertSorted(elem->name);
            elem = (IPStoreImpl::PsItem *)List_Next(elem);
        }
    }

    penum->GenericReset();
    return penum;
}


//---------------------------------------------------------------------------
// IEnumPStoreItemsImpl Constructor
//---------------------------------------------------------------------------


IEnumPStoreItemsImpl::IEnumPStoreItemsImpl(void *CoTaskMemAlloc)
: IEnumPStoreGeneric()
{
    m_CoTaskMemAlloc = CoTaskMemAlloc;
}


//---------------------------------------------------------------------------
// IEnumPStoreItemsImpl Copy Constructor
//---------------------------------------------------------------------------


IEnumPStoreItemsImpl::IEnumPStoreItemsImpl(
    const IEnumPStoreItemsImpl *model)
: IEnumPStoreGeneric(model)
{
    m_CoTaskMemAlloc = model->m_CoTaskMemAlloc;
}


//---------------------------------------------------------------------------
// IEnumPStoreItemsImpl Destructor
//---------------------------------------------------------------------------


IEnumPStoreItemsImpl::~IEnumPStoreItemsImpl(void)
{
}


//---------------------------------------------------------------------------
// IEnumPStoreItemsImpl::InsertSorted
//---------------------------------------------------------------------------


void IEnumPStoreItemsImpl::InsertSorted(const WCHAR *name)
{
    IEnumPStoreListElem *elem =
        (IEnumPStoreListElem *)List_Head(&m_list->list);
    while (elem) {
        int c = _wcsicmp(elem->v.name, name);
        if (c == 0)
            return;
        if (c > 0)
            break;
        elem = (IEnumPStoreListElem *)List_Next(elem);
    }

    ULONG len = sizeof(IEnumPStoreListElem)
              + (wcslen(name) + 1) * sizeof(WCHAR);
    IEnumPStoreListElem *new_elem = (IEnumPStoreListElem *)Dll_Alloc(len);
    wcscpy(new_elem->v.name, name);
    if (elem)
        List_Insert_Before(&m_list->list, elem, new_elem);
    else
        List_Insert_After(&m_list->list, NULL, new_elem);
}


//---------------------------------------------------------------------------
// IEnumPStoreItemsImpl::QueryInterface
//---------------------------------------------------------------------------


HRESULT IEnumPStoreItemsImpl::QueryInterface(REFIID iid, void **ppvObject)
{
    this->AddRef();
    *ppvObject = this;
    return S_OK;
}


//---------------------------------------------------------------------------
// IEnumPStoreItemsImpl::AddRef
//---------------------------------------------------------------------------


ULONG IEnumPStoreItemsImpl::AddRef()
{
    ++m_refcount;
    return m_refcount;
}


//---------------------------------------------------------------------------
// IEnumPStoreItemsImpl::Release
//---------------------------------------------------------------------------


ULONG IEnumPStoreItemsImpl::Release()
{
    --m_refcount;
    if (m_refcount)
        return m_refcount;
    delete this;
    return 0;
}


//---------------------------------------------------------------------------
// IEnumPStoreItemsImpl::Next
//---------------------------------------------------------------------------


HRESULT IEnumPStoreItemsImpl::Next(
    DWORD celt, WCHAR **rgelt, DWORD *pceltFetched)
{
    if (pceltFetched)
        *pceltFetched = 0;
    if (m_list->error)
        return PST_E_TYPE_NO_EXISTS;
    while (celt) {
        if (m_current) {
            ULONG len = (wcslen(m_current->v.name) + 1) * sizeof(WCHAR);
            WCHAR *out =
                (WCHAR *)((P_CoTaskMemAlloc)m_CoTaskMemAlloc)(len);
            if (! out)
                return E_OUTOFMEMORY;
            wcscpy(out, m_current->v.name);
            *rgelt = out;
            ++rgelt;
            if (pceltFetched)
                ++*pceltFetched;
            m_current = (IEnumPStoreListElem *)List_Next(m_current);
            --celt;
        } else
            return HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
    }
    return S_OK;
}


//---------------------------------------------------------------------------
// IEnumPStoreItemsImpl::Skip
//---------------------------------------------------------------------------


HRESULT IEnumPStoreItemsImpl::Skip(DWORD celt)
{
    return GenericSkip(celt);
}


//---------------------------------------------------------------------------
// IEnumPStoreItemsImpl::Reset
//---------------------------------------------------------------------------


HRESULT IEnumPStoreItemsImpl::Reset()
{
    return GenericReset();
}


//---------------------------------------------------------------------------
// IEnumPStoreItemsImpl::Clone
//---------------------------------------------------------------------------


HRESULT IEnumPStoreItemsImpl::Clone(IEnumPStoreItems **ppenum)
{
    *ppenum = new IEnumPStoreItemsImpl(this);
    return S_OK;
}
