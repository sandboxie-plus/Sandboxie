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
// Protected Storage Implementation
//---------------------------------------------------------------------------


#ifndef MY_IPSTORE_IMPL_H
#define MY_IPSTORE_IMPL_H


#include "pstore.h"
#include "common/list.h"


//---------------------------------------------------------------------------
// IPStoreImpl
//---------------------------------------------------------------------------


class IPStoreImpl : public IPStore
{

    friend class IEnumPStoreTypesImpl;
    friend class IEnumPStoreItemsImpl;

private:

    typedef struct _tagPsType {
        LIST_ELEM list_elem;
        GUID type_id;
        WCHAR *name;
        ULONG flags;
        LIST subtypes;
    } PsType;

    typedef struct _tagPsSubtype {
        LIST_ELEM list_elem;
        GUID subtype_id;
        WCHAR *name;
        ULONG flags;
        LIST items;
    } PsSubtype;

    typedef struct _tagPsItem {
        LIST_ELEM list_elem;
        WCHAR *name;
        ULONG flags;
        void *value;
        int value_len;
    } PsItem;

    int refcount;

    void *CoTaskMemAlloc;

    HANDLE mutex;
    HANDLE section;
    HANDLE heap;
    LIST types;
    __int64 local_timestamp;
    __int64 *global_timestamp;

    bool m_readFlags;

private:

    void reset();

    void *open_stream(bool forWrite);

    bool read();
    bool read_type(void *_stream);
    bool read_subtype(void *_stream, PsType *type);
    bool read_item(void *_stream, PsSubtype *subtype);
    bool read_block(void *_stream, void **data, int *len);
    void write();
    bool write_type(void *_stream, const PsType *type);
    bool write_subtype(void *_stream, const PsSubtype *subtype);
    bool write_item(void *_stream, const PsItem *item);
    bool write_block(void *_stream, void *data, int len);

    PsType *create_type(
        const GUID *pType, const PST_TYPEINFO *pInfo, DWORD dwFlags);
    PsSubtype *create_subtype(
        PsType *type, const GUID *pSubtype,
        const PST_TYPEINFO *pInfo, DWORD dwFlags);
    PsItem *create_item(
        PsSubtype *subtype, PsItem *item, const WCHAR *szItemName,
        DWORD cbData, void *pbData);

    PsType *find_type(const GUID *guid);
    PsSubtype *find_subtype(PsType *type, const GUID *guid);
    PsItem *find_item(PsType *type, PsSubtype *subtype, const WCHAR *item);

    HRESULT not_impl(const WCHAR *func);

public:

    IPStoreImpl(void *ptrCoTaskMemAlloc);
    ~IPStoreImpl();

    // operator new

    void *operator new(size_t n);
    void operator delete(void *p);

    // IUnknown

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
        REFIID iid,
        void **ppvObject);

    virtual ULONG STDMETHODCALLTYPE AddRef();

    virtual ULONG STDMETHODCALLTYPE Release();

    // IPStore

    virtual HRESULT STDMETHODCALLTYPE GetInfo(
        /* [in] */ PPST_PROVIDERINFO __RPC_FAR *ppProperties);

    virtual HRESULT STDMETHODCALLTYPE GetProvParam(
        /* [in] */ DWORD dwParam,
        /* [out] */ DWORD __RPC_FAR *pcbData,
        /* [out] */ BYTE __RPC_FAR *__RPC_FAR *ppbData,
        /* [in] */ DWORD dwFlags);

    virtual HRESULT STDMETHODCALLTYPE SetProvParam(
        /* [in] */ DWORD dwParam,
        /* [in] */ DWORD cbData,
        /* [in] */ BYTE __RPC_FAR *pbData,
        /* [in] */ DWORD __RPC_FAR *dwFlags);

    virtual HRESULT STDMETHODCALLTYPE CreateType(
        /* [in] */ PST_KEY Key,
        /* [in] */ const GUID __RPC_FAR *pType,
        /* [in] */ PPST_TYPEINFO pInfo,
        /* [in] */ DWORD dwFlags);

    virtual HRESULT STDMETHODCALLTYPE GetTypeInfo(
        /* [in] */ PST_KEY Key,
        /* [in] */ const GUID __RPC_FAR *pType,
        /* [in] */ PPST_TYPEINFO __RPC_FAR *__RPC_FAR *ppInfo,
        /* [in] */ DWORD dwFlags);

    virtual HRESULT STDMETHODCALLTYPE DeleteType(
        /* [in] */ PST_KEY Key,
        /* [in] */ const GUID __RPC_FAR *pType,
        /* [in] */ DWORD dwFlags);

    virtual HRESULT STDMETHODCALLTYPE CreateSubtype(
        /* [in] */ PST_KEY Key,
        /* [in] */ const GUID __RPC_FAR *pType,
        /* [in] */ const GUID __RPC_FAR *pSubtype,
        /* [in] */ PPST_TYPEINFO pInfo,
        /* [in] */ PPST_ACCESSRULESET pRules,
        /* [in] */ DWORD dwFlags);

    virtual HRESULT STDMETHODCALLTYPE GetSubtypeInfo(
        /* [in] */ PST_KEY Key,
        /* [in] */ const GUID __RPC_FAR *pType,
        /* [in] */ const GUID __RPC_FAR *pSubtype,
        /* [in] */ PPST_TYPEINFO __RPC_FAR *__RPC_FAR *ppInfo,
        /* [in] */ DWORD dwFlags);

    virtual HRESULT STDMETHODCALLTYPE DeleteSubtype(
        /* [in] */ PST_KEY Key,
        /* [in] */ const GUID __RPC_FAR *pType,
        /* [in] */ const GUID __RPC_FAR *pSubtype,
        /* [in] */ DWORD dwFlags);

    virtual HRESULT STDMETHODCALLTYPE ReadAccessRuleset(
        /* [in] */ PST_KEY Key,
        /* [in] */ const GUID __RPC_FAR *pType,
        /* [in] */ const GUID __RPC_FAR *pSubtype,
        /* [in] */ PPST_TYPEINFO pInfo,
        /* [in] */ PPST_ACCESSRULESET __RPC_FAR *__RPC_FAR *ppRules,
        /* [in] */ DWORD dwFlags);

    virtual HRESULT STDMETHODCALLTYPE WriteAccessRuleset(
        /* [in] */ PST_KEY Key,
        /* [in] */ const GUID __RPC_FAR *pType,
        /* [in] */ const GUID __RPC_FAR *pSubtype,
        /* [in] */ PPST_TYPEINFO pInfo,
        /* [in] */ PPST_ACCESSRULESET pRules,
        /* [in] */ DWORD dwFlags);

    virtual HRESULT STDMETHODCALLTYPE EnumTypes(
        /* [in] */ PST_KEY Key,
        /* [in] */ DWORD dwFlags,
        /* [in] */ IEnumPStoreTypes __RPC_FAR *__RPC_FAR *ppenum);

    virtual HRESULT STDMETHODCALLTYPE EnumSubtypes(
        /* [in] */ PST_KEY Key,
        /* [in] */ const GUID __RPC_FAR *pType,
        /* [in] */ DWORD dwFlags,
        /* [in] */ IEnumPStoreTypes __RPC_FAR *__RPC_FAR *ppenum);

    virtual HRESULT STDMETHODCALLTYPE DeleteItem(
        /* [in] */ PST_KEY Key,
        /* [in] */ const GUID __RPC_FAR *pItemType,
        /* [in] */ const GUID __RPC_FAR *pItemSubtype,
        /* [in] */ LPCWSTR szItemName,
        /* [in] */ PPST_PROMPTINFO pPromptInfo,
        /* [in] */ DWORD dwFlags);

    virtual HRESULT STDMETHODCALLTYPE ReadItem(
        /* [in] */ PST_KEY Key,
        /* [in] */ const GUID __RPC_FAR *pItemType,
        /* [in] */ const GUID __RPC_FAR *pItemSubtype,
        /* [in] */ LPCWSTR szItemName,
        /* [in] */ DWORD __RPC_FAR *cbData,
        /* [in] */ BYTE __RPC_FAR *__RPC_FAR *ppbData,
        /* [in] */ PPST_PROMPTINFO pPromptInfo,
        /* [in] */ DWORD dwFlags);

    virtual HRESULT STDMETHODCALLTYPE WriteItem(
        /* [in] */ PST_KEY Key,
        /* [in] */ const GUID __RPC_FAR *pItemType,
        /* [in] */ const GUID __RPC_FAR *pItemSubtype,
        /* [in] */ LPCWSTR szItemName,
        /* [in] */ DWORD cbData,
        /* [size_is][in] */ BYTE __RPC_FAR *pbData,
        /* [in] */ PPST_PROMPTINFO pPromptInfo,
        /* [in] */ DWORD dwDefaultConfirmationStyle,
        /* [in] */ DWORD dwFlags);

    virtual HRESULT STDMETHODCALLTYPE OpenItem(
        /* [in] */ PST_KEY Key,
        /* [in] */ const GUID __RPC_FAR *pItemType,
        /* [in] */ const GUID __RPC_FAR *pItemSubtype,
        /* [in] */ LPCWSTR szItemName,
        /* [in] */ PST_ACCESSMODE ModeFlags,
        /* [in] */ PPST_PROMPTINFO pProomptInfo,
        /* [in] */ DWORD dwFlags);

    virtual HRESULT STDMETHODCALLTYPE CloseItem(
        /* [in] */ PST_KEY Key,
        /* [in] */ const GUID __RPC_FAR *pItemType,
        /* [in] */ const GUID __RPC_FAR *pItemSubtype,
        /* [in] */ LPCWSTR __RPC_FAR *szItemName,
        /* [in] */ DWORD dwFlags);

    virtual HRESULT STDMETHODCALLTYPE EnumItems(
        /* [in] */ PST_KEY Key,
        /* [in] */ const GUID __RPC_FAR *pItemType,
        /* [in] */ const GUID __RPC_FAR *pItemSubtype,
        /* [in] */ DWORD dwFlags,
        /* [in] */ IEnumPStoreItems __RPC_FAR *__RPC_FAR *ppenum);

};


//---------------------------------------------------------------------------


#endif // MY_IPSTORE_IMPL_H
