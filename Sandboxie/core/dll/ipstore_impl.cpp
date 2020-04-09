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

#define STREAM_USER_MODE
extern "C" {
#include "dll.h"
#include "common/stream.h"
}

#include <stdio.h>
#include "ipstore_impl.h"
#include "ipstore_enum.h"
#include "common/win32_ntddk.h"
#include "core/svc/PStoreWire.h"
#include "common/my_version.h"
#include "msgs/msgs.h"


#define HeapAllocN(n) HeapAlloc(heap, HEAP_GENERATE_EXCEPTIONS, n)
#define HeapAllocType(st) (st *)HeapAllocN(sizeof(st))

typedef LPVOID (*P_CoTaskMemAlloc)(ULONG cb);

#define PSTORE_VERSION_2 0xA1B2C3D4

#define FLAG_DELETED 1

static const WCHAR *_MutexName   = SBIE L"_ProtectedStorage_Mutex";
static const WCHAR *_SectionName = SBIE L"_ProtectedStorage_Section";



IPStoreImpl::IPStoreImpl(void *ptrCoTaskMemAlloc)
{
    refcount = 1;

    CoTaskMemAlloc = ptrCoTaskMemAlloc;

    mutex = NULL;
    section = NULL;
    heap = NULL;

    List_Init(&types);

    local_timestamp = 0;
    global_timestamp = NULL;

    mutex = CreateMutex(NULL, FALSE, _MutexName);
    if (! mutex)
        return;
    WaitForSingleObject(mutex, INFINITE);

    bool section_created = false;

    section = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, _SectionName);
    if (! section) {
        section = CreateFileMapping(
            INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 8, _SectionName);
        if (section)
            section_created = true;
    }
    if (section) {
        global_timestamp = (__int64 *)
            MapViewOfFile(section, FILE_MAP_ALL_ACCESS, 0, 0, 8);
        if (section_created)
            *global_timestamp = local_timestamp + 1;
    }

    ReleaseMutex(mutex);
}


IPStoreImpl::~IPStoreImpl()
{
    if (heap)
        HeapDestroy(heap);
    if (section)
        CloseHandle(section);
    if (mutex)
        CloseHandle(mutex);
}


void *IPStoreImpl::operator new(size_t n)
{
    return HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, n);
}


void IPStoreImpl::operator delete(void *p)
{
    HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, p);
}


void *IPStoreImpl::open_stream(bool forWrite)
{
    HANDLE handle;
    ULONG desired_access;
    ULONG share_mode;
    ULONG create_disp;
    ULONG file_attrs;
    STREAM *stream;
    WCHAR filename[MAX_PATH + 64];

    ZeroMemory(filename, sizeof(filename));
    GetWindowsDirectory(filename, MAX_PATH);
#ifdef SBIE_PST_DAT
    wcscat(filename, L"\\" SBIE_PST_DAT);
#else
    wcscat(filename, L"\\SbiePst.dat");
#endif SBIE_PST_DAT

    if (forWrite) {

        desired_access = GENERIC_WRITE;
        share_mode = 0;
        create_disp = CREATE_ALWAYS;
        file_attrs = FILE_ATTRIBUTE_HIDDEN;

    } else {

        desired_access = GENERIC_READ;
        share_mode = FILE_SHARE_READ;
        create_disp = OPEN_EXISTING;
        file_attrs = FILE_ATTRIBUTE_NORMAL;
    }

    handle = CreateFile(
        filename, desired_access, share_mode,
        NULL, create_disp, file_attrs, NULL);

    if ((handle == INVALID_HANDLE_VALUE) && (GetLastError() == 5) &&
            forWrite) {

        create_disp = TRUNCATE_EXISTING;
        handle = CreateFile(
            filename, desired_access, share_mode,
            NULL, create_disp, file_attrs, NULL);
    }

    if (handle != INVALID_HANDLE_VALUE) {

        if (Stream_Open(&stream, handle) == STATUS_SUCCESS)
            return stream;
    }

    return NULL;
}


bool IPStoreImpl::read()
{
    if (! global_timestamp)
        return false;

    WaitForSingleObject(mutex, INFINITE);

    if (local_timestamp != *global_timestamp) {

        local_timestamp = *global_timestamp;

        if (heap)
            HeapDestroy(heap);
        heap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 4096, 0);

        List_Init(&types);

        STREAM *stream = (STREAM *)open_stream(false);
        if (stream) {

            bool ok = true;

            ULONG count;
            if (Stream_Read_Long(stream, &count) == STATUS_SUCCESS) {

                // count may contain the version 2 indicator, which
                // means the real count is one ULONG later, and it
                // also means we have to read the flags members

                m_readFlags = false;
                if (count == PSTORE_VERSION_2) {
                    if (Stream_Read_Long(stream, &count) != STATUS_SUCCESS)
                        count = 0;
                    m_readFlags = true;
                }

                while (count > 0) {
                    if (! read_type(stream)) {
                        ok = false;
                        break;
                    }
                    --count;
                }
            }

            if (! ok)
                SbieApi_Log(2332, NULL);

            Stream_Close(stream);
        }
    }

    ReleaseMutex(mutex);
    return true;
}


bool IPStoreImpl::read_type(void *_stream)
{
    STREAM *stream = (STREAM *)_stream;

    PsType *type = HeapAllocType(PsType);

    if (Stream_Read_Bytes(
            stream, sizeof(GUID), (UCHAR *)&type->type_id)
                != STATUS_SUCCESS)
        return false;
    int name_len;
    if (! read_block(stream, (void **)&type->name, &name_len))
        return false;

    type->flags = 0;
    if (m_readFlags) {
        if (Stream_Read_Long(stream, &type->flags) != STATUS_SUCCESS)
            return false;
    }

    DWORD count;
    if (Stream_Read_Long(stream, &count) != STATUS_SUCCESS)
        return false;

    List_Init(&type->subtypes);

    while (count != 0) {
        if (! read_subtype(stream, type))
            return false;
        --count;
    }

    List_Insert_After(&types, NULL, type);

    return true;
}


bool IPStoreImpl::read_subtype(void *_stream, PsType *type)
{
    STREAM *stream = (STREAM *)_stream;

    PsSubtype *subtype = HeapAllocType(PsSubtype);

    if (Stream_Read_Bytes(
            stream, sizeof(GUID), (UCHAR *)&subtype->subtype_id)
                != STATUS_SUCCESS)
        return false;
    int name_len;
    if (! read_block(stream, (void **)&subtype->name, &name_len))
        return false;

    subtype->flags = 0;
    if (m_readFlags) {
        if (Stream_Read_Long(stream, &subtype->flags) != STATUS_SUCCESS)
            return false;
    }

    DWORD count;
    if (Stream_Read_Long(stream, &count) != STATUS_SUCCESS)
        return false;

    List_Init(&subtype->items);

    while (count != 0) {
        if (! read_item(stream, subtype))
            return false;
        --count;
    }

    List_Insert_After(&type->subtypes, NULL, subtype);

    return true;
}


bool IPStoreImpl::read_item(void *_stream, PsSubtype *subtype)
{
    STREAM *stream = (STREAM *)_stream;

    PsItem *item = HeapAllocType(PsItem);

    int name_len;
    if (! read_block(stream, (void **)&item->name, &name_len))
        return false;

    item->flags = 0;
    if (m_readFlags) {
        if (Stream_Read_Long(stream, &item->flags) != STATUS_SUCCESS)
            return false;
    }

    if (! read_block(stream, &item->value, &item->value_len))
        return false;

    List_Insert_After(&subtype->items, NULL, item);

    return true;
}



bool IPStoreImpl::read_block(void *_stream, void **data, int *len)
{
    STREAM *stream = (STREAM *)_stream;
    DWORD n1, n2;
    int i;

    if (Stream_Read_Long(stream, &n1) != STATUS_SUCCESS)
        return false;
    *data = HeapAllocN(n1);
    if (Stream_Read_Bytes(stream, n1, (UCHAR *)*data) != STATUS_SUCCESS)
        return false;
    if (Stream_Read_Long(stream, &n2) != STATUS_SUCCESS)
        return false;
    if ((~n2) != n1)
        return false;
    *len = n1;
    for (i = 0; i < *len; ++i)
        (*(UCHAR **)data)[i] ^= (UCHAR)i;
    return true;
}


void IPStoreImpl::write()
{
    if (! global_timestamp)
        return;

    WaitForSingleObject(mutex, INFINITE);

    STREAM *stream = (STREAM *)open_stream(true);
    if (stream) {

        Stream_Write_Long(stream, PSTORE_VERSION_2);

        Stream_Write_Long(stream, List_Count(&types));

        PsType *type = (PsType *)List_Head(&types);
        while (type) {
            if (! write_type(stream, type))
                break;
            type = (PsType *)List_Next(type);
        }

        Stream_Flush(stream);
        Stream_Close(stream);
    }

    ReleaseMutex(mutex);
};


bool IPStoreImpl::write_type(void *_stream, const PsType *type)
{
    STREAM *stream = (STREAM *)_stream;

    if (Stream_Write_Bytes(
            stream, sizeof(GUID), (UCHAR *)&type->type_id)
                != STATUS_SUCCESS)
        return false;
    int name_len = (wcslen(type->name) + 1) * sizeof(WCHAR);
    if (! write_block(stream, type->name, name_len))
        return false;

    if (Stream_Write_Long(stream, type->flags) != STATUS_SUCCESS)
        return false;

    DWORD count = List_Count(&type->subtypes);
    if (Stream_Write_Long(stream, count) != STATUS_SUCCESS)
        return false;

    PsSubtype *subtype = (PsSubtype *)List_Head(&type->subtypes);
    while (subtype) {
        if (! write_subtype(stream, subtype))
            return false;
        subtype = (PsSubtype *)List_Next(subtype);
    }

    return true;
}


bool IPStoreImpl::write_subtype(void *_stream, const PsSubtype *subtype)
{
    STREAM *stream = (STREAM *)_stream;

    if (Stream_Write_Bytes(
            stream, sizeof(GUID), (UCHAR *)&subtype->subtype_id)
                != STATUS_SUCCESS)
        return false;
    int name_len = (wcslen(subtype->name) + 1) * sizeof(WCHAR);
    if (! write_block(stream, subtype->name, name_len))
        return false;

    if (Stream_Write_Long(stream, subtype->flags) != STATUS_SUCCESS)
        return false;

    DWORD count = List_Count(&subtype->items);
    if (Stream_Write_Long(stream, count) != STATUS_SUCCESS)
        return false;

    PsItem *item = (PsItem *)List_Head(&subtype->items);
    while (item) {
        if (! write_item(stream, item))
            return false;
        item = (PsItem *)List_Next(item);
    }

    return true;
}


bool IPStoreImpl::write_item(void *_stream, const PsItem *item)
{
    STREAM *stream = (STREAM *)_stream;

    int name_len = (wcslen(item->name) + 1) * sizeof(WCHAR);
    if (! write_block(stream, item->name, name_len))
        return false;

    if (Stream_Write_Long(stream, item->flags) != STATUS_SUCCESS)
        return false;

    if (! write_block(stream, item->value, item->value_len))
        return false;

    return true;
}


bool IPStoreImpl::write_block(void *_stream, void *data, int len)
{
    STREAM *stream = (STREAM *)_stream;
    int i;
    bool ok;

    if (Stream_Write_Long(stream, len) != STATUS_SUCCESS)
        return false;
    for (i = 0; i < len; ++i)
        ((UCHAR *)data)[i] ^= (UCHAR)i;
    ok = Stream_Write_Bytes(stream, len, (UCHAR *)data) == STATUS_SUCCESS;
    for (i = 0; i < len; ++i)
        ((UCHAR *)data)[i] ^= (UCHAR)i;
    if (! ok)
        return false;
    if (Stream_Write_Long(stream, ~len) != STATUS_SUCCESS)
        return false;
    return true;
}


IPStoreImpl::PsType *IPStoreImpl::create_type(
    const GUID *pType, const PST_TYPEINFO *pInfo, DWORD dwFlags)
{
    PsType *type = HeapAllocType(PsType);

    type->type_id = *pType;

    DWORD len = (wcslen(pInfo->szDisplayName) + 1) * sizeof(WCHAR);
    type->name = (WCHAR *)HeapAllocN(len);
    CopyMemory(type->name, pInfo->szDisplayName, len);

    type->flags = 0;

    List_Init(&type->subtypes);

    List_Insert_Before(&types, NULL, type);

    return type;
}


IPStoreImpl::PsSubtype *IPStoreImpl::create_subtype(
    IPStoreImpl::PsType *type, const GUID *pSubtype,
    const PST_TYPEINFO *pInfo, DWORD dwFlags)
{
    PsSubtype *subtype = HeapAllocType(PsSubtype);

    subtype->subtype_id = *pSubtype;

    DWORD len = (wcslen(pInfo->szDisplayName) + 1) * sizeof(WCHAR);
    subtype->name = (WCHAR *)HeapAllocN(len);
    CopyMemory(subtype->name, pInfo->szDisplayName, len);

    subtype->flags = 0;

    List_Init(&subtype->items);

    List_Insert_Before(&type->subtypes, NULL, subtype);

    return subtype;
}

IPStoreImpl::PsItem *IPStoreImpl::create_item(
    IPStoreImpl::PsSubtype *subtype, IPStoreImpl::PsItem *item,
    const WCHAR *szItemName, DWORD cbData, void *pbData)
{
    if (! item) {
        item = HeapAllocType(PsItem);
        item->flags = 0;
        List_Insert_Before(&subtype->items, NULL, item);
    }

    DWORD len = (wcslen(szItemName) + 1) * sizeof(WCHAR);
    item->name = (WCHAR *)HeapAllocN(len);
    CopyMemory(item->name, szItemName, len);

    item->value_len = cbData;
    if (item->value_len) {
        item->value = HeapAllocN(item->value_len);
        CopyMemory(item->value, pbData, item->value_len);
    } else
        item->value = NULL;

    return item;
}

IPStoreImpl::PsType *IPStoreImpl::find_type(const GUID *guid)
{
    // find the type in the sandboxed pstore

    PsType *type = (PsType *)List_Head(&types);
    while (type) {
        if (IsEqualGUID(type->type_id, *guid)) {
            if (type->flags & FLAG_DELETED)
                type = NULL;
            return type;
        }
        type = (PsType *)List_Next(type);
    }

    // otherwise ask server if it knows this type

    PSTORE_GET_TYPE_INFO_REQ req;
    req.h.length = sizeof(PSTORE_GET_TYPE_INFO_REQ);
    req.h.msgid = MSGID_PSTORE_GET_TYPE_INFO;
    req.type_id = *guid;

    PSTORE_GET_TYPE_INFO_RPL *rpl =
        (PSTORE_GET_TYPE_INFO_RPL *)SbieDll_CallServer(&req.h);

    if (rpl && rpl->h.status == 0) {

        PST_TYPEINFO aTypeInfo;
        aTypeInfo.cbSize = sizeof(PST_TYPEINFO);
        aTypeInfo.szDisplayName = rpl->name;

        type = create_type(guid, &aTypeInfo, rpl->flags);
    }

    if (rpl)
        Dll_Free(rpl);

    return type;
}


IPStoreImpl::PsSubtype *IPStoreImpl::find_subtype(
    IPStoreImpl::PsType *type, const GUID *guid)
{
    // find the subtype in the sandboxed pstore

    PsSubtype *subtype = (PsSubtype *)List_Head(&type->subtypes);
    while (subtype) {
        if (IsEqualGUID(subtype->subtype_id, *guid)) {
            if (subtype->flags & FLAG_DELETED)
                subtype = NULL;
            return subtype;
        }
        subtype = (PsSubtype *)List_Next(subtype);
    }

    // otherwise ask server if it knows this subtype

    PSTORE_GET_SUBTYPE_INFO_REQ req;
    req.h.length = sizeof(PSTORE_GET_SUBTYPE_INFO_REQ);
    req.h.msgid = MSGID_PSTORE_GET_SUBTYPE_INFO;
    req.type_id = type->type_id;
    req.subtype_id = *guid;

    PSTORE_GET_SUBTYPE_INFO_RPL *rpl =
        (PSTORE_GET_SUBTYPE_INFO_RPL *)SbieDll_CallServer(&req.h);

    if (rpl && rpl->h.status == 0) {

        PST_TYPEINFO aTypeInfo;
        aTypeInfo.cbSize = sizeof(PST_TYPEINFO);
        aTypeInfo.szDisplayName = rpl->name;

        subtype = create_subtype(type, guid, &aTypeInfo, rpl->flags);
    }

    if (rpl)
        Dll_Free(rpl);

    return subtype;
}


IPStoreImpl::PsItem *IPStoreImpl::find_item(
    IPStoreImpl::PsType *type, IPStoreImpl::PsSubtype *subtype,
    const WCHAR *name)
{
    // find the item in the sandboxed pstore

    PsItem *item = (PsItem *)List_Head(&subtype->items);
    while (item) {
        if (wcscmp(item->name, name) == 0) {
            if (item->flags & FLAG_DELETED)
                item = NULL;
            return item;
        }
        item = (PsItem *)List_Next(item);
    }

    // otherwise ask server if it knows this item

    ULONG req_len = sizeof(PSTORE_READ_ITEM_REQ)
                  + (wcslen(name) + 1) * sizeof(WCHAR);
    PSTORE_READ_ITEM_REQ *req = (PSTORE_READ_ITEM_REQ *)Dll_Alloc(req_len);
    req->h.length = req_len;
    req->h.msgid = MSGID_PSTORE_READ_ITEM;
    req->type_id = type->type_id;
    req->subtype_id = subtype->subtype_id;
    req->name_len = wcslen(name);
    wcscpy(req->name, name);

    PSTORE_READ_ITEM_RPL *rpl =
        (PSTORE_READ_ITEM_RPL *)SbieDll_CallServer(&req->h);

    if (rpl && rpl->h.status == 0) {

        item = create_item(subtype, NULL, name, rpl->data_len, rpl->data);
    }

    Dll_Free(req);
    if (rpl)
        Dll_Free(rpl);

    return item;
}


HRESULT IPStoreImpl::not_impl(const WCHAR *func)
{
    SbieApi_Log(2205, L"IPStore::%S", func);
    return E_FAIL;
}


HRESULT IPStoreImpl::GetInfo(
    /* [in] */ PPST_PROVIDERINFO __RPC_FAR *ppProperties)
{
    return not_impl(L"GetInfo");
}


HRESULT IPStoreImpl::GetProvParam(
    /* [in] */ DWORD dwParam,
    /* [out] */ DWORD __RPC_FAR *pcbData,
    /* [out] */ BYTE __RPC_FAR *__RPC_FAR *ppbData,
    /* [in] */ DWORD dwFlags)
{
    return not_impl(L"GetProvParam");
}


HRESULT IPStoreImpl::SetProvParam(
    /* [in] */ DWORD dwParam,
    /* [in] */ DWORD cbData,
    /* [in] */ BYTE __RPC_FAR *pbData,
    /* [in] */ DWORD __RPC_FAR *dwFlags)
{
    return not_impl(L"SetProvParam");
}


HRESULT IPStoreImpl::CreateType(
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID __RPC_FAR *pType,
    /* [in] */ PPST_TYPEINFO pInfo,
    /* [in] */ DWORD dwFlags)
{
    if (! read())
        return E_FAIL;

    PsType *type = find_type(pType);
    if (type)
        return PST_E_TYPE_EXISTS;

    create_type(pType, pInfo, dwFlags);
    write();

    return PST_E_OK;
}


HRESULT IPStoreImpl::GetTypeInfo(
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID __RPC_FAR *pType,
    /* [in] */ PPST_TYPEINFO __RPC_FAR *__RPC_FAR *ppInfo,
    /* [in] */ DWORD dwFlags)
{
    return not_impl(L"GetTypeInfo");
}


HRESULT IPStoreImpl::DeleteType(
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID __RPC_FAR *pType,
    /* [in] */ DWORD dwFlags)
{
    if (! read())
        return E_FAIL;

    PsType *type = find_type(pType);
    if (! type)
        return PST_E_TYPE_NO_EXISTS;

    type->flags |= FLAG_DELETED;

    write();

    return PST_E_OK;
}


HRESULT IPStoreImpl::CreateSubtype(
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID __RPC_FAR *pType,
    /* [in] */ const GUID __RPC_FAR *pSubtype,
    /* [in] */ PPST_TYPEINFO pInfo,
    /* [in] */ PPST_ACCESSRULESET pRules,
    /* [in] */ DWORD dwFlags)
{
    if (! read())
        return E_FAIL;

    PsType *type = find_type(pType);
    if (! type)
        return PST_E_TYPE_NO_EXISTS;

    PsSubtype *subtype = find_subtype(type, pSubtype);
    if (subtype)
        return PST_E_TYPE_EXISTS;

    create_subtype(type, pSubtype, pInfo, dwFlags);
    write();

    return PST_E_OK;
}


HRESULT IPStoreImpl::GetSubtypeInfo(
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID __RPC_FAR *pType,
    /* [in] */ const GUID __RPC_FAR *pSubtype,
    /* [in] */ PPST_TYPEINFO __RPC_FAR *__RPC_FAR *ppInfo,
    /* [in] */ DWORD dwFlags)
{
    return not_impl(L"GetSubtypeInfo");
}


HRESULT IPStoreImpl::DeleteSubtype(
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID __RPC_FAR *pType,
    /* [in] */ const GUID __RPC_FAR *pSubtype,
    /* [in] */ DWORD dwFlags)
{
    if (! read())
        return E_FAIL;

    PsType *type = find_type(pType);
    if (! type)
        return PST_E_TYPE_NO_EXISTS;

    PsSubtype *subtype = find_subtype(type, pSubtype);
    if (! subtype)
        return PST_E_TYPE_NO_EXISTS;

    subtype->flags |= FLAG_DELETED;

    write();

    return PST_E_OK;
}


HRESULT IPStoreImpl::ReadAccessRuleset(
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID __RPC_FAR *pType,
    /* [in] */ const GUID __RPC_FAR *pSubtype,
    /* [in] */ PPST_TYPEINFO pInfo,
    /* [in] */ PPST_ACCESSRULESET __RPC_FAR *__RPC_FAR *ppRules,
    /* [in] */ DWORD dwFlags)
{
    return not_impl(L"ReadAccessRuleset");
}


HRESULT IPStoreImpl::WriteAccessRuleset(
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID __RPC_FAR *pType,
    /* [in] */ const GUID __RPC_FAR *pSubtype,
    /* [in] */ PPST_TYPEINFO pInfo,
    /* [in] */ PPST_ACCESSRULESET pRules,
    /* [in] */ DWORD dwFlags)
{
    return not_impl(L"WriteAccessRuleset");
}


HRESULT IPStoreImpl::DeleteItem(
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID __RPC_FAR *pItemType,
    /* [in] */ const GUID __RPC_FAR *pItemSubtype,
    /* [in] */ LPCWSTR szItemName,
    /* [in] */ PPST_PROMPTINFO pPromptInfo,
    /* [in] */ DWORD dwFlags)
{
    if (! read())
        return E_FAIL;

    PsType *type = find_type(pItemType);
    if (! type)
        return PST_E_TYPE_NO_EXISTS;

    PsSubtype *subtype = find_subtype(type, pItemSubtype);
    if (! subtype)
        return PST_E_TYPE_NO_EXISTS;

    PsItem *item = find_item(type, subtype, szItemName);
    if (! item)
        return PST_E_ITEM_NO_EXISTS;

    item->flags |= FLAG_DELETED;

    write();

    return PST_E_OK;
}


HRESULT IPStoreImpl::ReadItem(
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID __RPC_FAR *pItemType,
    /* [in] */ const GUID __RPC_FAR *pItemSubtype,
    /* [in] */ LPCWSTR szItemName,
    /* [in] */ DWORD __RPC_FAR *cbData,
    /* [in] */ BYTE __RPC_FAR *__RPC_FAR *ppbData,
    /* [in] */ PPST_PROMPTINFO pPromptInfo,
    /* [in] */ DWORD dwFlags)
{
    if (! read())
        return E_FAIL;

    PsType *type = find_type(pItemType);
    if (! type)
        return PST_E_TYPE_NO_EXISTS;

    PsSubtype *subtype = find_subtype(type, pItemSubtype);
    if (! subtype)
        return PST_E_TYPE_NO_EXISTS;

    PsItem *item = find_item(type, subtype, szItemName);
    if (! item)
        return PST_E_ITEM_NO_EXISTS;

    *cbData = item->value_len;
    if (item->value_len) {
        *ppbData = (BYTE *)
            ((P_CoTaskMemAlloc)CoTaskMemAlloc)(item->value_len);
        CopyMemory(*ppbData, item->value, item->value_len);
    } else
        *ppbData = NULL;

    return PST_E_OK;
}

HRESULT IPStoreImpl::WriteItem(
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID __RPC_FAR *pItemType,
    /* [in] */ const GUID __RPC_FAR *pItemSubtype,
    /* [in] */ LPCWSTR szItemName,
    /* [in] */ DWORD cbData,
    /* [size_is][in] */ BYTE __RPC_FAR *pbData,
    /* [in] */ PPST_PROMPTINFO pPromptInfo,
    /* [in] */ DWORD dwDefaultConfirmationStyle,
    /* [in] */ DWORD dwFlags)
{
    if (! read())
        return E_FAIL;

    PsType *type = find_type(pItemType);
    if (! type)
        return PST_E_TYPE_NO_EXISTS;

    PsSubtype *subtype = find_subtype(type, pItemSubtype);
    if (! subtype)
        return PST_E_TYPE_NO_EXISTS;

    PsItem *item = find_item(type, subtype, szItemName);
    if (item) {
        if (dwFlags & PST_NO_OVERWRITE)
            return PST_E_ITEM_EXISTS;
        if (item->value) {
            HeapFree(heap, HEAP_GENERATE_EXCEPTIONS, item->value);
            item->value = NULL;
            item->value_len = 0;
        }
    }

    create_item(subtype, item, szItemName, cbData, pbData);
    write();

    return PST_E_OK;
}

HRESULT IPStoreImpl::OpenItem(
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID __RPC_FAR *pItemType,
    /* [in] */ const GUID __RPC_FAR *pItemSubtype,
    /* [in] */ LPCWSTR szItemName,
    /* [in] */ PST_ACCESSMODE ModeFlags,
    /* [in] */ PPST_PROMPTINFO pProomptInfo,
    /* [in] */ DWORD dwFlags)
{
    // from MSDN:  The OpenItem method opens an item for multiple accesses.
    // looks like this can be implemented as a no-op ?
    return PST_E_OK;
    // return not_impl(L"OpenItem");
}


HRESULT IPStoreImpl::CloseItem(
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID __RPC_FAR *pItemType,
    /* [in] */ const GUID __RPC_FAR *pItemSubtype,
    /* [in] */ LPCWSTR __RPC_FAR *szItemName,
    /* [in] */ DWORD dwFlags)
{
    // from MSDN:  The CloseItem method closes a specified data item from
    //             protected storage.
    // see OpenItem
    return PST_E_OK;
    //return not_impl(L"CloseItem");
}


//---------------------------------------------------------------------------
// IPStore::EnumTypes
//---------------------------------------------------------------------------


HRESULT IPStoreImpl::EnumTypes(
    /* [in] */ PST_KEY Key,
    /* [in] */ DWORD dwFlags,
    /* [in] */ IEnumPStoreTypes __RPC_FAR *__RPC_FAR *ppenum)
{
    if (! read())
        return E_FAIL;
    *ppenum = IEnumPStoreTypesImpl::CreateEnumType(this, Key);
    if (! *ppenum)
        return E_OUTOFMEMORY;
    return S_OK;
}


//---------------------------------------------------------------------------
// IPStore::EnumSubtypes
//---------------------------------------------------------------------------


HRESULT IPStoreImpl::EnumSubtypes(
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID __RPC_FAR *pType,
    /* [in] */ DWORD dwFlags,
    /* [in] */ IEnumPStoreTypes __RPC_FAR *__RPC_FAR *ppenum)
{
    if (! read())
        return E_FAIL;
    *ppenum = IEnumPStoreTypesImpl::CreateEnumSubtype(this, Key, pType);
    if (! *ppenum)
        return E_OUTOFMEMORY;
    return S_OK;
}


//---------------------------------------------------------------------------
// IPStore::EnumItems
//---------------------------------------------------------------------------


HRESULT IPStoreImpl::EnumItems(
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID __RPC_FAR *pItemType,
    /* [in] */ const GUID __RPC_FAR *pItemSubtype,
    /* [in] */ DWORD dwFlags,
    /* [in] */ IEnumPStoreItems __RPC_FAR *__RPC_FAR *ppenum)
{
    if (! read())
        return E_FAIL;
    *ppenum = IEnumPStoreItemsImpl::CreateEnumItem(
        this, Key, pItemType, pItemSubtype);
    if (! *ppenum)
        return E_OUTOFMEMORY;
    return S_OK;
}


//---------------------------------------------------------------------------
// IUnknown::QueryInterface
//---------------------------------------------------------------------------


ALIGNED HRESULT IPStoreImpl::QueryInterface(
    REFIID iid,
    void **ppvObject)
{
    this->AddRef();
    *ppvObject = this;
    return S_OK;
}


//---------------------------------------------------------------------------
// IUnknown::AddRef
//---------------------------------------------------------------------------


ALIGNED ULONG IPStoreImpl::AddRef()
{
    ++refcount;
    return refcount;
}


//---------------------------------------------------------------------------
// IUnknown::Release
//---------------------------------------------------------------------------


ALIGNED ULONG IPStoreImpl::Release()
{
    --refcount;
    if (refcount)
        return refcount;
    delete this;
    return 0;
}
