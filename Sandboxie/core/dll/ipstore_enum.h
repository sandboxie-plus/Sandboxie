/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2021 David Xanatos, xanasoft.com
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



#include "ipstore_impl.h"


struct IEnumPStoreListElem
{
    LIST_ELEM list_elem;
    union {
        GUID guid;
        WCHAR name[1];
    } v;
};


struct IEnumPStoreList
{
    LIST list;
    ULONG refcount;
    BOOLEAN error;
};


class IEnumPStoreGeneric
{

public:

    void *operator new(size_t n);
    void operator delete(void *p);

protected:

    IEnumPStoreGeneric();
    IEnumPStoreGeneric(const IEnumPStoreGeneric *model);
    ~IEnumPStoreGeneric();

    HRESULT GenericSkip(DWORD celt);

    HRESULT GenericReset();

protected:

    IEnumPStoreList *m_list;
    IEnumPStoreListElem *m_current;
    ULONG m_refcount;
};


class IEnumPStoreTypesImpl :
    public IEnumPStoreGeneric, public IEnumPStoreTypes
{

public:

    static IEnumPStoreTypes *CreateEnumType(IPStoreImpl *pst, ULONG pst_key);
    static IEnumPStoreTypes *CreateEnumSubtype(
        IPStoreImpl *pst, ULONG pst_key, const GUID *type_guid);

protected:

    IEnumPStoreTypesImpl();
    IEnumPStoreTypesImpl(const IEnumPStoreTypesImpl *model);
    ~IEnumPStoreTypesImpl();

    void InsertSorted(GUID *guid);

    // IUnknown

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
        REFIID iid,
        void **ppvObject);

    virtual ULONG STDMETHODCALLTYPE AddRef();

    virtual ULONG STDMETHODCALLTYPE Release();

    // IEnumPStoreTypes

    virtual HRESULT STDMETHODCALLTYPE Next(
        DWORD celt,
        GUID *rgelt,
        DWORD *pceltFetched);

    virtual HRESULT STDMETHODCALLTYPE Skip(
        DWORD celt);

    virtual HRESULT STDMETHODCALLTYPE Reset(void);

    virtual HRESULT STDMETHODCALLTYPE Clone(
        IEnumPStoreTypes **ppenum);

};


class IEnumPStoreItemsImpl :
    public IEnumPStoreGeneric, public IEnumPStoreItems
{

public:

    static IEnumPStoreItems *CreateEnumItem(
        IPStoreImpl *pst, ULONG pst_key,
        const GUID *type_guid, const GUID *subtype_guid);

protected:

    IEnumPStoreItemsImpl(void *CoTaskMemAlloc);
    IEnumPStoreItemsImpl(const IEnumPStoreItemsImpl *model);
    ~IEnumPStoreItemsImpl();

    void InsertSorted(const WCHAR *name);

    void *m_CoTaskMemAlloc;

    // IUnknown

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
        REFIID iid,
        void **ppvObject);

    virtual ULONG STDMETHODCALLTYPE AddRef();

    virtual ULONG STDMETHODCALLTYPE Release();

    // IEnumPStoreTypes

    virtual HRESULT STDMETHODCALLTYPE Next(
        DWORD celt,
        WCHAR **rgelt,
        DWORD *pceltFetched);

    virtual HRESULT STDMETHODCALLTYPE Skip(
        DWORD celt);

    virtual HRESULT STDMETHODCALLTYPE Reset(void);

    virtual HRESULT STDMETHODCALLTYPE Clone(
        IEnumPStoreItems **ppenum);

};

