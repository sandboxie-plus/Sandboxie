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
// Key Merge
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _KEY_MERGE {

    LIST_ELEM list_elem;

    HANDLE handle;
    ULONG ticks;
    BOOLEAN cant_merge;

    BOOLEAN subkeys_merged;
    LARGE_INTEGER last_write_time;
    LIST subkeys;

    ULONG last_index;
    void *last_subkey;

    BOOLEAN values_merged;
    LIST values;

    ULONG name_len;     // in bytes, excluding NULL
    WCHAR name[0];

} KEY_MERGE;


typedef struct _KEY_MERGE_SUBKEY {

    LIST_ELEM list_elem;
    ULONG name_len;     // in bytes, excluding NULL
    LARGE_INTEGER LastWriteTime;
    BOOLEAN TitleOrClass;
    WCHAR name[0];

} KEY_MERGE_SUBKEY;


typedef struct _KEY_MERGE_VALUE {

    LIST_ELEM list_elem;
    ULONG data_type;
    ULONG data_len;
    void *data_ptr;
    ULONG name_len;     // in bytes, excluding NULL
    WCHAR name[0];
    // WCHAR data[0];

} KEY_MERGE_VALUE;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static NTSTATUS Key_Merge(
    HANDLE KeyHandle, const WCHAR *TruePath, const WCHAR *CopyPath,
    BOOLEAN want_subkeys, BOOLEAN want_values, KEY_MERGE **out_merge);

static NTSTATUS Key_OpenForMerge(
    HANDLE KeyHandle, const WCHAR *TruePath, const WCHAR *CopyPath,
    KEY_MERGE **out_TrueMerge, HANDLE *out_CopyHandle);

static BOOLEAN Key_ShouldNotMerge(
    const WCHAR *TruePath, const WCHAR *CopyPath);

static NTSTATUS Key_MergeCache(
    HANDLE TrueHandle, LARGE_INTEGER *LastWriteTime,
    const WCHAR *TruePath, KEY_MERGE **out_TrueMerge);

static NTSTATUS Key_MergeCacheSubkeys(KEY_MERGE *merge, HANDLE TrueHandle);

static NTSTATUS Key_MergeCacheValues(KEY_MERGE *merge, HANDLE TrueHandle);

static NTSTATUS Key_MergeSubkeys(
    KEY_MERGE *merge, KEY_MERGE *TrueMerge, HANDLE CopyHandle);

static NTSTATUS Key_MergeValues(
    KEY_MERGE *merge, KEY_MERGE *TrueMerge, HANDLE CopyHandle);

static void Key_MergeFree(KEY_MERGE *merge, BOOLEAN FreeMergeItself);

static NTSTATUS Key_GetMergedValue(
    KEY_MERGE_VALUE *value,
    KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    void *KeyValueInformation,
    ULONG Length,
    ULONG *ResultLength);

static void Key_DiscardMergeByHandle(
    THREAD_DATA *TlsData, HANDLE KeyHandle, BOOLEAN Recurse);

NTSTATUS File_NtCloseImpl(HANDLE FileHandle);

//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static LIST Key_Handles;
static LIST Key_MergeCacheList;
static CRITICAL_SECTION Key_Handles_CritSec;


//---------------------------------------------------------------------------
// Key_Merge
//---------------------------------------------------------------------------


_FX NTSTATUS Key_Merge(
    HANDLE KeyHandle, const WCHAR *TruePath, const WCHAR *CopyPath,
    BOOLEAN want_subkeys, BOOLEAN want_values, KEY_MERGE **out_merge)
{
    NTSTATUS status;
    ULONG ticks_now;
    ULONG TruePath_len;
    KEY_MERGE *merge;
    KEY_MERGE *TrueMerge;
    HANDLE CopyHandle;

    //
    // if we have information cached for this handle, return it
    //

    ticks_now = GetTickCount();
    TruePath_len = wcslen(TruePath) * sizeof(WCHAR);

    EnterCriticalSection(&Key_Handles_CritSec);

    merge = List_Head(&Key_Handles);
    while (merge) {

        KEY_MERGE *next = List_Next(merge);

            /*
        if (1) {
            WCHAR txt[512];
            Sbie_snwprintf(txt, 512, L"Merge at %08X Handle %08X Name (%d) %s Cmp => %d\n",
                merge, merge->handle, merge->name_len, merge->name,
                merge->name_len == TruePath_len ?
                    RtlEqualMemory(merge->name, TruePath, TruePath_len)
                : -1);
            OutputDebugString(txt);
        }
            */

        if (merge->handle != KeyHandle) {

            //
            // this isn't the cached entry we want, but if we find
            // an entry that is too old, we must discard it.  the
            // timeout should be fairly small, so that changes that
            // are made to this key outside this process, will become
            // visible even if the merge was cached
            //

            if (ticks_now - merge->ticks <= 5 * 1000) {
                merge = next;
                continue;
            }

        } else if (merge->name_len == TruePath_len &&
                        _wcsnicmp(merge->name, TruePath,
                                  TruePath_len / sizeof(WCHAR)) == 0) {

            //
            // we found a cached entry for the same handle, and
            // the same key path, so we are going to use it.
            //

            break;
        }

        //
        // if we got here, we need to discard the stale entry
        //

        List_Remove(&Key_Handles, merge);
        Key_MergeFree(merge, TRUE);

        merge = next;
    }

    //
    // if we don't have a merge entry, create one.  it is inserted
    // at the end of the list, so the loop above always gets to
    // look at all existing merge entries and discard stale ones
    //

    if (! merge) {

        merge = Dll_Alloc(sizeof(KEY_MERGE) + TruePath_len + sizeof(WCHAR));
        memzero(merge, sizeof(KEY_MERGE));

        merge->handle = KeyHandle;
        merge->ticks = ticks_now;
        // merge->cant_merge = FALSE;       // memzero takes care of this

        merge->name_len = TruePath_len;
        memcpy(merge->name, TruePath, TruePath_len + sizeof(WCHAR));

        List_Insert_Before(&Key_Handles, NULL, merge);
    }

    //
    // if cant_merge is set, then we already know that either TruePath
    // or CopyPath exist, but not both, so return special status
    //

    if (merge->cant_merge) {

        LeaveCriticalSection(&Key_Handles_CritSec);
        return STATUS_BAD_INITIAL_PC;
    }

    //
    // merge the subkeys and values, if caller asked for them.
    // we will first need to open the copy key and get the true merge
    //

    if (    (want_subkeys && (! merge->subkeys_merged)) ||
            (want_values  && (! merge->values_merged)))
    {
        status = Key_OpenForMerge(
            KeyHandle, TruePath, CopyPath, &TrueMerge, &CopyHandle);

        if (! NT_SUCCESS(status)) {

            //
            // if we get STATUS_BAD_INITIAL_PC, then set cant_merge
            // (so next time through this function for the same handle
            // will skip some overhead).  we will also return this status
            // without doing anything else
            //

            if (status == STATUS_BAD_INITIAL_PC)
                merge->cant_merge = TRUE;

            TrueMerge  = NULL;
            CopyHandle = NULL;
        }

    } else {

        TrueMerge  = NULL;
        CopyHandle = NULL;
        status = STATUS_SUCCESS;
    }

    if (NT_SUCCESS(status) && want_subkeys && (! merge->subkeys_merged)) {
        status = Key_MergeSubkeys(merge, TrueMerge, CopyHandle);
        merge->subkeys_merged = TRUE;
    }

    if (NT_SUCCESS(status) && want_values && (! merge->values_merged)) {
        status = Key_MergeValues(merge, TrueMerge, CopyHandle);
        merge->values_merged = TRUE;
    }

    //
    // finish.  IMPORTANT:  if we return STATUS_SUCCESS, we return
    // without unlocking Key_Handles_CritSec !
    //

    if (CopyHandle)
        File_NtCloseImpl(CopyHandle);

    if (! NT_SUCCESS(status))
        LeaveCriticalSection(&Key_Handles_CritSec);

    *out_merge = merge;
    return status;
}


//---------------------------------------------------------------------------
// Key_OpenForMerge
//---------------------------------------------------------------------------


_FX NTSTATUS Key_OpenForMerge(
    HANDLE KeyHandle, const WCHAR *TruePath, const WCHAR *CopyPath,
    KEY_MERGE **out_TrueMerge, HANDLE *out_CopyHandle)
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    KEY_BASIC_INFORMATION info;
    ULONG len;
    HANDLE TrueHandle;
    ULONG mp_flags;

    *out_TrueMerge  = NULL;
    *out_CopyHandle = NULL;

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    //
    // for some keys, we want to prevent merging to increase performance
    //

    if (Key_ShouldNotMerge(TruePath, CopyPath))
        return STATUS_BAD_INITIAL_PC;

    //
    // check for open and closed paths
    //

    mp_flags = SbieDll_MatchPath(L'k', TruePath);

    if (PATH_IS_CLOSED(mp_flags))
        return STATUS_ACCESS_DENIED;
    else if (PATH_IS_OPEN(mp_flags))
        return STATUS_BAD_INITIAL_PC;

    //
    // open CopyPath for KEY_READ access.  we use __sys_NtOpenKey
    // because we want to know if there is a CopyPath key, or not,
    // or if it is an open path.
    //

    RtlInitUnicodeString(&objname, CopyPath);

    status = __sys_NtOpenKey(out_CopyHandle, KEY_READ, &objattrs);

    if (NT_SUCCESS(status)) {

        //
        // if we could open the copy key, make sure it isn't deleted
        //

        status = __sys_NtQueryKey(
            *out_CopyHandle, KeyBasicInformation,
            &info, sizeof(KEY_BASIC_INFORMATION), &len);

        if (NT_SUCCESS(status) || status == STATUS_BUFFER_OVERFLOW) {
            if (IS_DELETE_MARK(&info.LastWriteTime))
                status = STATUS_KEY_DELETED;
            else
                status = STATUS_SUCCESS;
        }

        if (! NT_SUCCESS(status))
            File_NtCloseImpl(*out_CopyHandle);

    } else if (status == STATUS_OBJECT_NAME_NOT_FOUND ||
               status == STATUS_OBJECT_PATH_NOT_FOUND ||
               status == STATUS_ACCESS_DENIED)
    {
        //
        // if we couldn't find a copy key, indicate there is nothing to merge
        //

        status = STATUS_BAD_INITIAL_PC;
    }

    if (! NT_SUCCESS(status)) {
        *out_CopyHandle = NULL;
        return status;
    }

    //
    // open TruePath for KEY_READ access.  we use __sys_NtOpenKey
    // because we really want the TruePath key, even if there is
    // a counterpart boxed CopyPath key.
    //

    RtlInitUnicodeString(&objname, TruePath);

    if (PATH_IS_WRITE(mp_flags))
        status = STATUS_OBJECT_NAME_NOT_FOUND;
    else {
        status = __sys_NtOpenKey(
            &TrueHandle, Key_GetWow64Flag(TruePath, KEY_READ), &objattrs);
    }

    if (NT_SUCCESS(status)) {

        //
        // if we could open the true key, get the true merge for it
        //

        status = __sys_NtQueryKey(
            TrueHandle, KeyBasicInformation,
            &info, sizeof(KEY_BASIC_INFORMATION), &len);

        if (NT_SUCCESS(status) || status == STATUS_BUFFER_OVERFLOW) {

            status = Key_MergeCache(
                TrueHandle, &info.LastWriteTime, TruePath, out_TrueMerge);
        }

        File_NtCloseImpl(TrueHandle);

    } else if (status == STATUS_OBJECT_NAME_NOT_FOUND ||
               status == STATUS_OBJECT_PATH_NOT_FOUND)
    {
        //
        // we couldn't find a true key, but we still have to "merge"
        // the copy key (in order to remove deleted keys/values)
        //

        status = STATUS_SUCCESS;
    }

    if (! NT_SUCCESS(status)) {
        File_NtCloseImpl(*out_CopyHandle);
        *out_TrueMerge  = NULL;
        *out_CopyHandle = NULL;
        return status;
    }

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Key_ShouldNotMerge
//---------------------------------------------------------------------------


_FX BOOLEAN Key_ShouldNotMerge(const WCHAR *TruePath, const WCHAR *CopyPath)
{
    static BOOLEAN HaveHklmDomainsKey = -1;
    static BOOLEAN HaveHkcuDomainsKey = -1;
    static const WCHAR *_Domains =
        L"Software\\Microsoft\\Windows\\CurrentVersion"
            L"\\Internet Settings\\ZoneMap\\Domains";
    static ULONG _Domains_Len = 75;
    const WCHAR *ptr;
    BOOLEAN hklm;

    //
    // hack:  there can be a large number of subkeys below the registry key
    //      HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\
    //                                  Internet Settings\ZoneMap\Domains
    //
    // these are typically created by the Immunize feature of SpyBot S&D
    // and cause the SHLWAPI!ZoneCheckUrlEx function to take a long time
    // to enumerate all these keys because all those registry keys have
    // to be merged every time they are used.
    //
    // to work around this problem, we check just once if the Domains key
    // exists in the sandbox, in the normal case it does not and then we
    // can short-circuit the merge process.
    //
    // we use SbieSvc to check if the key exists because in the case of
    // Adobe Reader X, the NtOpenKey function in OpenForMerge is hooked
    // and sent to the Adobe Reader broker process and will succeed even
    // if there is no sandboxed registry key.
    //

    ptr = TruePath;
    if (_wcsnicmp(ptr, Key_System, 17) == 0) {      // \registry\machine
        ptr += 17 + 1;
        hklm = TRUE;
    } else if (_wcsnicmp(ptr, Key_CurrentUser, Key_CurrentUserLen) == 0) {
        ptr += Key_CurrentUserLen + 1;
        hklm = FALSE;
    } else
        return FALSE;

    if (_wcsnicmp(ptr, _Domains, _Domains_Len) != 0)
        return FALSE;
    if (ptr[_Domains_Len] && ptr[_Domains_Len] != L'\\')
        return FALSE;

    if ((   hklm  && HaveHklmDomainsKey == (BOOLEAN)-1) ||
        ((! hklm) && HaveHkcuDomainsKey == (BOOLEAN)-1)) {

        WCHAR *ptr;
        MSG_HEADER *rpl;
        FILE_CHECK_KEY_EXISTS_REQ *req;
        ULONG req_len, path_len;

        path_len = Dll_BoxKeyPathLen;
        if (hklm)
            path_len += 8;                      // \machine
        else
            path_len += Key_UserCurrentLen;     // \user\current
        path_len += 1 + _Domains_Len;           // \Software\Microsoft\...

        req_len = sizeof(FILE_CHECK_KEY_EXISTS_REQ)
                + path_len * sizeof(WCHAR);
        req = Dll_AllocTemp(req_len);

        req->h.length = req_len;
        req->h.msgid = MSGID_FILE_CHECK_KEY_EXISTS;
        req->KeyPath_len = path_len;
        wmemcpy(req->KeyPath, Dll_BoxKeyPath, Dll_BoxKeyPathLen);
        ptr = req->KeyPath + Dll_BoxKeyPathLen;
        if (hklm) {
            wmemcpy(ptr, Key_System + 9, 8);    // \machine
            ptr += 8;
        } else {
            wmemcpy(ptr, Key_UserCurrent, Key_UserCurrentLen);
            ptr += Key_UserCurrentLen;
        }
        *ptr = L'\\';
        wmemcpy(ptr + 1, _Domains, _Domains_Len + 1);

        if (hklm)
            HaveHklmDomainsKey = TRUE;
        else
            HaveHkcuDomainsKey = TRUE;

        rpl = SbieDll_CallServer((MSG_HEADER *)req);
        Dll_Free(req);
        if (rpl) {

            if (rpl->status == STATUS_OBJECT_NAME_NOT_FOUND ||
                rpl->status == STATUS_OBJECT_PATH_NOT_FOUND) {

                if (hklm)
                    HaveHklmDomainsKey = FALSE;
                else
                    HaveHkcuDomainsKey = FALSE;
            }

            Dll_Free(rpl);
        }
    }

    if ((   hklm  && HaveHklmDomainsKey == FALSE) ||
        ((! hklm) && HaveHkcuDomainsKey == FALSE)) {

        return TRUE;
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// Key_MergeCache
//---------------------------------------------------------------------------


_FX NTSTATUS Key_MergeCache(
    HANDLE TrueHandle, LARGE_INTEGER *LastWriteTime,
    const WCHAR *TruePath, KEY_MERGE **out_TrueMerge)
{
    NTSTATUS status;
    ULONG ticks_now;
    ULONG TruePath_len;
    KEY_MERGE *merge;

    //
    // this function returns (possibly first creating) a cached KEY_MERGE
    // which represents only the true key for a particular key path.
    // this makes a noticable performance difference, because most true
    // keys don't change during the lifetime of a sandboxed process,
    // but they still need to be repeatedly merged with copy keys
    //
    // first, scan the list to see if we have an existing merge
    //

    ticks_now = GetTickCount();
    TruePath_len = wcslen(TruePath) * sizeof(WCHAR);

    merge = List_Head(&Key_MergeCacheList);
    while (merge) {

        KEY_MERGE *next = List_Next(merge);

        if (ticks_now - merge->ticks <= 30 * 1000) {

            //
            // we hit a merge that hasn't been in use for 30 seconds,
            // then release the storage for it
            //

            List_Remove(&Key_MergeCacheList, merge);
            Key_MergeFree(merge, TRUE);

        } else if (merge->name_len == TruePath_len &&
                        _wcsnicmp(merge->name, TruePath,
                                  TruePath_len / sizeof(WCHAR)) == 0) {

            //
            // we found an existing cached entry for a true merge with
            // the same true path
            //

            break;
        }

        merge = next;
    }

    //
    // if we found a merge, make sure it is not outdated with respect
    // to the real key, before returning it.  if outdated, rebuild it
    //

    if (merge) {

        if (LastWriteTime->QuadPart == merge->last_write_time.QuadPart) {
            *out_TrueMerge = merge;
            return STATUS_SUCCESS;
        }

        Key_MergeFree(merge, FALSE);

    } else {

        //
        // no merge found, create the merge structure
        //

        merge = Dll_Alloc(sizeof(KEY_MERGE) + TruePath_len + sizeof(WCHAR));
        memzero(merge, sizeof(KEY_MERGE));

        merge->name_len = TruePath_len;
        memcpy(merge->name, TruePath, TruePath_len + sizeof(WCHAR));

        List_Insert_After(&Key_MergeCacheList, NULL, merge);
    }

    merge->last_write_time.QuadPart = LastWriteTime->QuadPart;

    //
    // build the subkeys and values in the true merge
    //

    status = Key_MergeCacheSubkeys(merge, TrueHandle);
    if (NT_SUCCESS(status))
        status = Key_MergeCacheValues(merge, TrueHandle);
    if (NT_SUCCESS(status))
        *out_TrueMerge = merge;
    else {
        List_Remove(&Key_MergeCacheList, merge);
        Key_MergeFree(merge, TRUE);
    }

    return status;
}


//---------------------------------------------------------------------------
// Key_MergeCacheSubkeys
//---------------------------------------------------------------------------


_FX NTSTATUS Key_MergeCacheSubkeys(KEY_MERGE *merge, HANDLE TrueHandle)
{
    NTSTATUS status;
    ULONG info_len;
    ULONG len;
    KEY_NODE_INFORMATION *info;
    ULONG index;
    KEY_MERGE_SUBKEY *subkey, *subkey2;

    //
    // get the subkeys from the TrueHandle and insert them,
    // in sorted alphabetical order, into the merge
    //

    info_len = 128;         // at least sizeof(KEY_NODE_INFORMATION)
    info = Dll_Alloc(info_len);

    index = 0;

    while (1) {

        status = __sys_NtEnumerateKey(
            TrueHandle, index, KeyNodeInformation, info, info_len, &len);

        if (status == STATUS_BUFFER_OVERFLOW) {

            Dll_Free(info);
            if (len > info_len)
                info_len = len;
            else
                info_len += 128;
            info = Dll_Alloc(info_len);
            continue;
        }

        if (status == STATUS_NO_MORE_ENTRIES)
            break;

        if (! NT_SUCCESS(status)) {
            Dll_Free(info);
            return status;
        }

        //
        // create the subkey
        //

        len = sizeof(KEY_MERGE_SUBKEY) + info->NameLength + sizeof(WCHAR);
        subkey = Dll_Alloc(len);

        subkey->name_len = info->NameLength;
        memcpy(subkey->name, info->Name, subkey->name_len);
        subkey->name[subkey->name_len / sizeof(WCHAR)] = L'\0';

        subkey->LastWriteTime = info->LastWriteTime;

        subkey->TitleOrClass = (info->TitleIndex ||
                                info->ClassOffset != -1 ||
                                info->ClassLength);

        //
        // find where to insert it.  if the new key is already larger than
        // our last key in the sorted list, instead directly at the end
        //

        subkey2 = List_Tail(&merge->subkeys);
        if (subkey2 && _wcsicmp(subkey2->name, subkey->name) < 0)
            subkey2 = NULL;
        else {
            subkey2 = List_Head(&merge->subkeys);
            while (subkey2) {
                if (_wcsicmp(subkey2->name, subkey->name) > 0)
                    break;
                subkey2 = List_Next(subkey2);
            }
        }

        if (subkey2)
            List_Insert_Before(&merge->subkeys, subkey2, subkey);
        else
            List_Insert_After(&merge->subkeys, NULL, subkey);

        ++index;
    }

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Key_MergeCacheValues
//---------------------------------------------------------------------------


_FX NTSTATUS Key_MergeCacheValues(KEY_MERGE *merge, HANDLE TrueHandle)
{
    NTSTATUS status;
    ULONG info_len;
    ULONG len;
    KEY_VALUE_FULL_INFORMATION *info;
    ULONG index;
    KEY_MERGE_VALUE *value, *value2;

    //
    // get the values from the TrueHandle and insert them,
    // in sorted alphabetical order, into the merge
    //

    info_len = 128;         // at least sizeof(KEY_VALUE_FULL_INFORMATION)
    info = Dll_Alloc(info_len);

    index = 0;

    while (1) {

        status = __sys_NtEnumerateValueKey(
            TrueHandle, index, KeyValueFullInformation,
            info, info_len, &len);

        if (status == STATUS_BUFFER_OVERFLOW) {

            Dll_Free(info);
            if (len > info_len)
                info_len = len;
            else
                info_len += 128;
            info = Dll_Alloc(info_len);
            continue;
        }

        if (status == STATUS_NO_MORE_ENTRIES)
            break;

        if (! NT_SUCCESS(status)) {
            Dll_Free(info);
            return status;
        }

        //
        // create the value
        //

        len = sizeof(KEY_MERGE_VALUE)
            + info->NameLength + sizeof(WCHAR)
            + info->DataLength;
        value = Dll_Alloc(len);

        value->name_len = info->NameLength;
        memcpy(value->name, info->Name, value->name_len);
        value->name[value->name_len / sizeof(WCHAR)] = L'\0';

        value->data_type = info->Type;
        value->data_ptr = (UCHAR *)value->name
                        + value->name_len + sizeof(WCHAR);
        value->data_len = info->DataLength;
        memcpy(value->data_ptr,
               (UCHAR *)info + info->DataOffset, info->DataLength);

        //
        // find where to insert it
        //

        value2 = List_Head(&merge->values);
        while (value2) {
            if (_wcsicmp(value2->name, value->name) > 0)
                break;
            value2 = List_Next(value2);
        }
        if (value2)
            List_Insert_Before(&merge->values, value2, value);
        else
            List_Insert_After(&merge->values, NULL, value);

        ++index;
    }

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Key_MergeSubkeys
//---------------------------------------------------------------------------


_FX NTSTATUS Key_MergeSubkeys(
    KEY_MERGE *merge, KEY_MERGE *TrueMerge, HANDLE CopyHandle)
{
    NTSTATUS status;
    ULONG info_len;
    ULONG len;
    KEY_NODE_INFORMATION *info;
    ULONG index;
    KEY_MERGE_SUBKEY *subkey, *subkey2;
    BOOLEAN subkey_deleted;

    //
    // get the latest of the two LastWriteTime fields
    //

    info_len = 128;         // at least sizeof(KEY_NODE_INFORMATION)
    info = Dll_Alloc(info_len);

    status = __sys_NtQueryKey(
        CopyHandle, KeyBasicInformation, info, info_len, &len);

    if ((! NT_SUCCESS(status)) && status != STATUS_BUFFER_OVERFLOW) {
        Dll_Free(info);
        return status;
    }

    merge->last_write_time.QuadPart = info->LastWriteTime.QuadPart;

    if (! TrueMerge)
        goto TrueHandleFinish;

    if (TrueMerge->last_write_time.QuadPart >
                                            merge->last_write_time.QuadPart)
        merge->last_write_time.QuadPart = info->LastWriteTime.QuadPart;

    //
    // the true merge contains the list of all subkeys in the true key,
    // already sorted in alphabetical order.  copy the list into our merge
    //

    subkey2 = List_Head(&TrueMerge->subkeys);
    while (subkey2) {

        len = sizeof(KEY_MERGE_SUBKEY) + subkey2->name_len + sizeof(WCHAR);
        subkey = Dll_Alloc(len);

        subkey->name_len = subkey2->name_len;
        memcpy(subkey->name, subkey2->name, subkey->name_len);
        subkey->name[subkey->name_len / sizeof(WCHAR)] = L'\0';

        List_Insert_After(&merge->subkeys, NULL, subkey);

        subkey2 = List_Next(subkey2);
    }

TrueHandleFinish:
    ;

    //
    // next, get the subkeys from CopyHandle.  subkeys that are
    // marked deleted are removed from the merge.  other subkeys
    // are insterted in sorted alphabetical order
    //

    index = 0;

    while (1) {

        status = __sys_NtEnumerateKey(
            CopyHandle, index, KeyNodeInformation, info, info_len, &len);

        if (status == STATUS_BUFFER_OVERFLOW) {

            Dll_Free(info);
            if (len > info_len)
                info_len = len;
            else
                info_len += 128;
            info = Dll_Alloc(info_len);
            continue;
        }

        if (status == STATUS_NO_MORE_ENTRIES)
            break;

        if (! NT_SUCCESS(status)) {
            Dll_Free(info);
            return status;
        }

        //
        // create the subkey
        //

        len = sizeof(KEY_MERGE_SUBKEY) + info->NameLength + sizeof(WCHAR);
        subkey = Dll_Alloc(len);

        subkey->name_len = info->NameLength;
        memcpy(subkey->name, info->Name, info->NameLength);
        subkey->name[info->NameLength / sizeof(WCHAR)] = L'\0';

        subkey->LastWriteTime = info->LastWriteTime;

        subkey->TitleOrClass = (info->TitleIndex ||
                                info->ClassOffset != -1 ||
                                info->ClassLength);

        if (IS_DELETE_MARK(&info->LastWriteTime))
            subkey_deleted = TRUE;
        else
            subkey_deleted = FALSE;

        //
        // find where we would insert the new subkey.  if we find
        // the same name already in the merge, check for delete mark
        //

        subkey2 = List_Head(&merge->subkeys);
        while (subkey2) {
            int cmp = _wcsicmp(subkey2->name, subkey->name);
            if (cmp > 0)
                break;

            if (cmp == 0) {
                if (subkey_deleted) {
                    List_Remove(&merge->subkeys, subkey2);
                    Dll_Free(subkey2);
                    subkey2 = NULL;
                } else {
                    subkey2->LastWriteTime = subkey->LastWriteTime;
                    if (subkey->TitleOrClass)
                        subkey2->TitleOrClass = subkey->TitleOrClass;
                }
                subkey_deleted = TRUE;
                break;
            }

            subkey2 = List_Next(subkey2);
        }

        if (subkey) {
            if (subkey_deleted)
                Dll_Free(subkey);
            else if (subkey2)
                List_Insert_Before(&merge->subkeys, subkey2, subkey);
            else
                List_Insert_After(&merge->subkeys, NULL, subkey);
        }

        ++index;
    }

        /*{WCHAR txt[128]; Sbie_snwprintf(txt, 128, L"Merge %s has %d subkeys: \n", wcsrchr(merge->name, L'\\'), List_Count(&merge->subkeys)); OutputDebugString(txt);
        subkey2 = List_Head(&merge->subkeys);
        while (subkey2) {
            Sbie_snwprintf(txt, 128, L"  ==>  %s\n", subkey2->name);  OutputDebugString(txt);
            subkey2 = List_Next(subkey2);
        }}*/

    Dll_Free(info);
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Key_MergeValues
//---------------------------------------------------------------------------


_FX NTSTATUS Key_MergeValues(
    KEY_MERGE *merge, KEY_MERGE *TrueMerge, HANDLE CopyHandle)
{
    NTSTATUS status;
    ULONG info_len;
    ULONG len;
    KEY_VALUE_FULL_INFORMATION *info;
    ULONG index;
    KEY_MERGE_VALUE *value, *value2;
    BOOLEAN value_deleted;

    info_len = 128;         // at least sizeof(KEY_VALUE_FULL_INFORMATION)
    info = Dll_Alloc(info_len);

    if (! TrueMerge)
        goto TrueHandleFinish;

    //
    // the true merge contains the list of all values in the true key,
    // already sorted in alphabetical order.  copy the list into our merge
    //

    value2 = List_Head(&TrueMerge->values);
    while (value2) {

        len = sizeof(KEY_MERGE_VALUE)
            + value2->name_len + sizeof(WCHAR)
            + value2->data_len;
        value = Dll_Alloc(len);

        value->name_len = value2->name_len;
        memcpy(value->name, value2->name, value->name_len);
        value->name[value->name_len / sizeof(WCHAR)] = L'\0';

        value->data_type = value2->data_type;
        value->data_ptr = (UCHAR *)value->name
                        + value->name_len + sizeof(WCHAR);
        value->data_len = value2->data_len;
        memcpy(value->data_ptr, value2->data_ptr, value->data_len);

        List_Insert_After(&merge->values, NULL, value);

        value2 = List_Next(value2);
    }

TrueHandleFinish:
    ;

    //
    // next, get the values from CopyHandle.  values that are
    // marked deleted are removed from the merge.  other values
    // are insterted in sorted alphabetical order
    //

    index = 0;

    while (1) {

        status = __sys_NtEnumerateValueKey(
            CopyHandle, index, KeyValueFullInformation,
            info, info_len, &len);

        if (status == STATUS_BUFFER_OVERFLOW) {

            Dll_Free(info);
            if (len > info_len)
                info_len = len;
            else
                info_len += 128;
            info = Dll_Alloc(info_len);
            continue;
        }

        if (status == STATUS_NO_MORE_ENTRIES)
            break;

        if (! NT_SUCCESS(status)) {
            Dll_Free(info);
            return status;
        }

        //
        // create the value
        //

        len = sizeof(KEY_MERGE_VALUE)
            + info->NameLength + sizeof(WCHAR)
            + info->DataLength;
        value = Dll_Alloc(len);

        value->name_len = info->NameLength;
        memcpy(value->name, info->Name, info->NameLength);
        value->name[info->NameLength / sizeof(WCHAR)] = L'\0';

        value->data_type = info->Type;
        value->data_ptr = (UCHAR *)value->name
                        + info->NameLength + sizeof(WCHAR);
        value->data_len = info->DataLength;
        memcpy(value->data_ptr,
               (UCHAR *)info + info->DataOffset, value->data_len);

        if (info->Type == tzuk)
            value_deleted = TRUE;
        else
            value_deleted = FALSE;

        //
        // find where we would insert the new value.  if we find
        // the same name already in the merge, then copy value must
        // replace true value. unless copy value is marked deleted,
        // in which case delete true value without adding copy value
        //

        value2 = List_Head(&merge->values);
        while (value2) {
            int cmp = _wcsicmp(value2->name, value->name);
            if (cmp > 0)
                break;

            if (cmp == 0) {
                if (! value_deleted) {
                    // if not delete mark, add copy value after true value
                    List_Insert_After(&merge->values, value2, value);
                    value = NULL;
                }
                // remove and delete true value
                List_Remove(&merge->values, value2);
                Dll_Free(value2);
                value2 = NULL;
                break;
            }

            value2 = List_Next(value2);
        }

        if (value) {
            if (value_deleted)
                Dll_Free(value);
            else if (value2)
                List_Insert_Before(&merge->values, value2, value);
            else
                List_Insert_After(&merge->values, NULL, value);
        }

        ++index;
    }

        /*{WCHAR txt[128]; Sbie_snwprintf(txt, 128, L"Merge %s has %d values: \n", wcsrchr(merge->name, L'\\'), List_Count(&merge->values)); OutputDebugString(txt);
        value2 = List_Head(&merge->values);
        while (value2) {
            Sbie_snwprintf(txt, 128, L"  ==>  %s\n", value2->name);  OutputDebugString(txt);
            value2 = List_Next(value2);
        }}*/

    Dll_Free(info);
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Key_MergeFree
//---------------------------------------------------------------------------


_FX void Key_MergeFree(
    KEY_MERGE *merge, BOOLEAN FreeMergeItself)
{
    while (1) {
        KEY_MERGE_SUBKEY *subkey = List_Head(&merge->subkeys);
        if (! subkey)
            break;
        List_Remove(&merge->subkeys, subkey);
        Dll_Free(subkey);
    }

    while (1) {
        KEY_MERGE_VALUE *value = List_Head(&merge->values);
        if (! value)
            break;
        List_Remove(&merge->values, value);
        Dll_Free(value);
    }

    if (FreeMergeItself)
        Dll_Free(merge);
}


//---------------------------------------------------------------------------
// Key_GetMergedValue
//---------------------------------------------------------------------------


_FX NTSTATUS Key_GetMergedValue(
    KEY_MERGE_VALUE *value,
    KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    void *KeyValueInformation,
    ULONG Length,
    ULONG *ResultLength)
{
    NTSTATUS status;
    ULONG info_min_len, info_max_len;
    UCHAR *info_buf;

    info_buf = NULL;

    __try {

    //
    // allocate a temporary buffer for complete output
    //

    if (KeyValueInformationClass == KeyValueBasicInformation) {

        info_min_len = FIELD_OFFSET(KEY_VALUE_BASIC_INFORMATION, Name);
        info_max_len = info_min_len + value->name_len;

    } else if (KeyValueInformationClass == KeyValueFullInformation) {

        info_min_len = FIELD_OFFSET(KEY_VALUE_FULL_INFORMATION, Name);
        info_max_len = info_min_len + value->name_len + value->data_len;

    } else if (KeyValueInformationClass == KeyValuePartialInformation) {

        info_min_len = FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data);
        info_max_len = info_min_len + value->data_len;

    } else if (KeyValueInformationClass==KeyValuePartialInformationAlign64) {

        info_min_len =
            FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION_ALIGN64, Data);
        info_max_len = info_min_len + value->data_len;

    } else {

        status = STATUS_INVALID_PARAMETER;
        __leave;
    }

    *ResultLength = info_max_len;
    if (Length < info_min_len) {
        status = STATUS_BUFFER_TOO_SMALL;
        __leave;
    }

    info_buf = Dll_Alloc(info_max_len);
    *(ULONG *)info_buf = 0;                 // reset TitleIndex

    //
    // fill the temporary buffer with complete information
    //

    if (KeyValueInformationClass == KeyValueBasicInformation) {

        KEY_VALUE_BASIC_INFORMATION *info =
            (KEY_VALUE_BASIC_INFORMATION *)info_buf;

        info->Type = value->data_type;
        info->NameLength = value->name_len;
        memcpy(info->Name, value->name, value->name_len);

    } else if (KeyValueInformationClass == KeyValueFullInformation) {

        KEY_VALUE_FULL_INFORMATION *info =
            (KEY_VALUE_FULL_INFORMATION *)info_buf;

        info->Type = value->data_type;
        info->DataOffset = FIELD_OFFSET(KEY_VALUE_FULL_INFORMATION, Name)
                         + value->name_len;
        info->DataLength = value->data_len;
        info->NameLength = value->name_len;
        memcpy(info->Name, value->name, value->name_len);
        memcpy(
            info_buf + info->DataOffset, value->data_ptr, value->data_len);

    } else if (KeyValueInformationClass == KeyValuePartialInformation) {

        KEY_VALUE_PARTIAL_INFORMATION *info =
            (KEY_VALUE_PARTIAL_INFORMATION *)info_buf;

        info->Type = value->data_type;
        info->DataLength = value->data_len;
        memcpy(info->Data, value->data_ptr, value->data_len);

    } else if (KeyValueInformationClass==KeyValuePartialInformationAlign64) {

        KEY_VALUE_PARTIAL_INFORMATION_ALIGN64 *info =
            (KEY_VALUE_PARTIAL_INFORMATION_ALIGN64 *)info_buf;

        info->Type = value->data_type;
        info->DataLength = value->data_len;
        memcpy(info->Data, value->data_ptr, value->data_len);

    }

    //
    // copy the information into the caller's buffer
    //

    if (Length >= info_max_len) {
        Length = info_max_len;
        status = STATUS_SUCCESS;
    } else
        status = STATUS_BUFFER_OVERFLOW;

    memcpy(KeyValueInformation, info_buf, Length);

    //
    // finish
    //

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    if (info_buf)
        Dll_Free(info_buf);

    return status;
}


//---------------------------------------------------------------------------
// Key_DiscardMergeByPath
//---------------------------------------------------------------------------


_FX void Key_DiscardMergeByPath(const WCHAR *TruePath, BOOLEAN Recurse)
{
    ULONG TruePath_len;
    KEY_MERGE *merge;

    TruePath_len = wcslen(TruePath) * sizeof(WCHAR);

    if (! TryEnterCriticalSection(&Key_Handles_CritSec))
        return;

    merge = List_Head(&Key_Handles);
    while (merge) {

        KEY_MERGE *next = List_Next(merge);

        if (merge->name_len == TruePath_len && _wcsnicmp(
                merge->name, TruePath, TruePath_len / sizeof(WCHAR)) == 0) {

            if (Recurse) {
                WCHAR *backslash = wcsrchr(merge->name, L'\\');
                if (backslash) {
                    *backslash = L'\0';
                    Key_DiscardMergeByPath(merge->name, FALSE);
                    *backslash = L'\\';
                    next = List_Next(merge);
                }
            }

            List_Remove(&Key_Handles, merge);
            Key_MergeFree(merge, TRUE);
        }

        merge = next;
    }

    LeaveCriticalSection(&Key_Handles_CritSec);
}


//---------------------------------------------------------------------------
// Key_DiscardMergeByHandle
//---------------------------------------------------------------------------


_FX void Key_DiscardMergeByHandle(
    THREAD_DATA *TlsData, HANDLE KeyHandle, BOOLEAN Recurse)
{
    NTSTATUS status;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    RtlInitUnicodeString(&objname, L"");

    status = Key_GetName(KeyHandle, &objname, &TruePath, &CopyPath, NULL);
    if (NT_SUCCESS(status))
        Key_DiscardMergeByPath(TruePath, Recurse);

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    Dll_PopTlsNameBuffer(TlsData);
}


//---------------------------------------------------------------------------
// Key_NtClose
//---------------------------------------------------------------------------


_FX void Key_NtClose(HANDLE KeyHandle)
{
    KEY_MERGE *merge;

    //
    // this routine should be called from NtClose or NtDuplicateObject.
    // it marks the merge associated with KeyHandle as unused,
    // causing it to be deleted by Key_Merge at some later point
    //

    if (! TryEnterCriticalSection(&Key_Handles_CritSec))
        return;

    merge = List_Head(&Key_Handles);
    while (merge) {
        if (merge->handle == KeyHandle) {
            merge->handle = (HANDLE)-1;
            merge->ticks = 0;
            break;
        }
        merge = List_Next(merge);;
    }

    LeaveCriticalSection(&Key_Handles_CritSec);
}
