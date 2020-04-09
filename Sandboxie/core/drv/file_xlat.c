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
// File System (Translation Services)
//---------------------------------------------------------------------------


#include "file.h"
#include "obj.h"


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _CACHE_PATH
{
    LIST_ELEM list_elem;
    ULONG alloc_len;
    ULONGLONG time;
    ULONG src_len;      // in characters, excluding NULL
    ULONG dst_len;      // in characters, excluding NULL
    WCHAR *dst;
    WCHAR src[1];

} CACHE_PATH;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static NTSTATUS File_TranslateShares(
    const WCHAR *dos_path, POOL *pool, WCHAR **out_path, ULONG *out_len);

static NTSTATUS File_TranslateSymlinks(WCHAR *name, ULONG max_len);

void File_InitReparsePoints(BOOLEAN init);

static WCHAR *File_TranslateReparsePoints_2(
    const WCHAR *path, ULONG len, POOL *pool, ULONG PassNum);

static CACHE_PATH *File_TranslateReparsePoints_3(
    const WCHAR *path, ULONG path_len, POOL *pool, ULONG PassNum);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


extern const WCHAR *File_Redirector;
extern const ULONG  File_RedirectorLen;
extern const WCHAR *File_MupRedir;
extern const ULONG  File_MupRedirLen;
extern const WCHAR *File_DfsClientRedir;
extern const ULONG  File_DfsClientRedirLen;
extern const WCHAR *File_HgfsRedir;
extern const ULONG  File_HgfsRedirLen;
extern const WCHAR *File_Mup;
extern const ULONG  File_MupLen;
extern const WCHAR *File_NamedPipe;
extern const ULONG  File_NamedPipeLen;

static LIST File_ReparsePointsList;
static PERESOURCE File_ReparsePointsLock = NULL;
static ULONG64 File_ReparsePointsCleanupTime = 0;
static volatile LONG File_ReparsePointsBusy = 0;


//---------------------------------------------------------------------------
// File_TranslateDosToNt
//---------------------------------------------------------------------------


_FX NTSTATUS File_TranslateDosToNt(
    const WCHAR *dos_path, POOL *pool, WCHAR **out_path, ULONG *out_len)
{
    NTSTATUS status;
    WCHAR *name;
    const ULONG name_max_len = 256;

    *out_path = NULL;
    *out_len = 0;

    //
    // handle shares
    //

    status = File_TranslateShares(dos_path, pool, out_path, out_len);
    if (status != STATUS_BAD_INITIAL_PC)
        return status;

    //
    // the input dos path may begin with \??\, which we just ignore
    //

    if (dos_path[0] == L'\\' && dos_path[1] == L'?' &&
        dos_path[2] == L'?'  && dos_path[3] == L'\\')
    {
        dos_path += 4;
    }

    //
    // the input dos path must begin (or continue) with x:\ or
    // x: immediately followed by a null terminator,
    //

    if ((! dos_path[0]) || dos_path[1] != L':' ||
            (dos_path[2] != L'\\' && dos_path[2] != L'\0')) {

        return STATUS_OBJECT_PATH_SYNTAX_BAD;
    }

    // we are going to open the symbolic link object \??\x: and query
    // its target path, and do this iteratively as long as the target
    // path is a symbolic link itself.  here we initialize the loop

    name = Mem_Alloc(pool, (name_max_len + 2) * sizeof(WCHAR));
    if (! name)
        return STATUS_INSUFFICIENT_RESOURCES;

    name[0] = L'\\';
    name[1] = L'?';
    name[2] = L'?';
    name[3] = L'\\';
    name[4] = dos_path[0];
    name[5] = L':';
    name[6] = L'\0';

    status = File_TranslateSymlinks(name, name_max_len);

    // the loop ends due to error.  if it was STATUS_OBJECT_TYPE_MISMATCH,
    // it means we parsed symbolic links until we reached the actual device
    // object, which we can return.  any other status is an error

    if (NT_SUCCESS(status)) {

        ULONG name_len = wcslen(name);

        WCHAR *path;
        ULONG len = (name_len + wcslen(&dos_path[2]) + 1) * sizeof(WCHAR);
        if (! dos_path[2]) {
            // if the dos path was just x:, we'll add one backslash
            len += sizeof(WCHAR);
        }
        path = Mem_Alloc(pool, len);
        if (path) {

            wmemcpy(path, name, name_len);
            if (! dos_path[2])
                wcscpy(path + name_len, L"\\");
            else
                wcscpy(path + name_len, &dos_path[2]);

            status = File_TranslateShares(path, pool, out_path, out_len);
            if (status != STATUS_BAD_INITIAL_PC)
                Mem_Free(path, len);
            else {

                *out_path = path;
                *out_len = len;
                status = STATUS_SUCCESS;
            }

        } else
            status = STATUS_INSUFFICIENT_RESOURCES;
    }

    Mem_Free(name, (name_max_len + 2) * sizeof(WCHAR));
    return status;
}


//---------------------------------------------------------------------------
// File_TranslateShares
//---------------------------------------------------------------------------


_FX NTSTATUS File_TranslateShares(
    const WCHAR *dos_path, POOL *pool, WCHAR **out_path, ULONG *out_len)
{
    WCHAR *path;
    ULONG PrefixLen;
    ULONG len;

    //
    // if the path begins with the DOS-style UNC prefix of two backslashes,
    // then replace it with \Device\Mup
    //

    if (dos_path[0] == L'\\' && dos_path[1] == L'\\' &&
        dos_path[2] != L'\0' && dos_path[2] != L'\\') {

        len = (File_MupLen + wcslen(&dos_path[1]) + 1) * sizeof(WCHAR);
        path = Mem_Alloc(pool, len);
        if (! path)
            return STATUS_INSUFFICIENT_RESOURCES;

        wmemcpy(path, File_Mup, File_MupLen);
        wcscpy(path + File_MupLen, &dos_path[1]);

        *out_path = path;
        *out_len = len;
        return STATUS_SUCCESS;
    }

    //
    // if the path begins with \Device\LanmanRedirector prefix, change
    // to the \Device\Mup prefix
    //

    if (_wcsnicmp(dos_path, File_Redirector, File_RedirectorLen) == 0)
        PrefixLen = File_RedirectorLen;
    else if (_wcsnicmp(dos_path, File_MupRedir, File_MupRedirLen) == 0)
        PrefixLen = File_MupRedirLen;
    else if (_wcsnicmp(dos_path, File_DfsClientRedir, File_DfsClientRedirLen) == 0)
        PrefixLen = File_DfsClientRedirLen;
    else if (_wcsnicmp(dos_path, File_HgfsRedir, File_HgfsRedirLen) == 0)
        PrefixLen = File_HgfsRedirLen;
    else
        PrefixLen = 0;

    if (PrefixLen) {

        const WCHAR *ptr = dos_path + PrefixLen;
        if (ptr[0] == L'\\') {
            //
            // we need to skip a path component, if path has a semicolon:
            // \Device\LanmanRedirector\;Z:0000000000009c2c\server\share
            // otherwise we don't skip at all, as in such a path:
            // \Device\LanmanRedirector\server\share
            //
            if (ptr[1] == L';')
                ptr = wcschr(ptr + 2, L'\\');
        } else
            ptr = NULL;
        if (ptr && ptr[0] && ptr[1]) {

            len = (PrefixLen + wcslen(ptr) + 1) * sizeof(WCHAR);
            path = Mem_Alloc(pool, len);
            if (! path)
                return STATUS_INSUFFICIENT_RESOURCES;

            wmemcpy(path, File_Mup, File_MupLen);
            wcscpy(path + File_MupLen, ptr);

            *out_path = path;
            *out_len = len;
            return STATUS_SUCCESS;
        }
    }

    //
    // otherwise, return special value to indicate no share name translation
    //

    return STATUS_BAD_INITIAL_PC;
}


//---------------------------------------------------------------------------
// File_TranslateSymlinks
//---------------------------------------------------------------------------


_FX NTSTATUS File_TranslateSymlinks(WCHAR *name, ULONG max_len)
{
    NTSTATUS status;
    HANDLE handle;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING uni;
    ULONG len, rem, ofs;
    ULONG retry = 0;
    BOOLEAN at_least_one_symlink_found = FALSE;

    //DbgPrint("-------------------------------------------\n");

    InitializeObjectAttributes(&objattrs,
        &uni, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

    uni.Buffer = name;

    len = wcslen(name);

    retry = 0;

top:

    ++retry;
    if (retry == 8)
        return STATUS_REPARSE_POINT_NOT_RESOLVED;

    //DbgPrint("[Retry %d] Path <%S>\n", retry, name);

    //
    // try to open the longest symbolic link we find.  for instance,
    // if the object name is \??\PIPE\MyPipe, we will open the link
    // "\??\PIPE" even though "\??\" itself is also a link
    //

    while (1) {

        WCHAR save_char = name[len];
        name[len] = L'\0';

        if (len == 7 && _wcsicmp(name, L"\\Device") == 0) {

            status = STATUS_OBJECT_TYPE_MISMATCH;

            //DbgPrint("[Retry %d] Fake Status <%08X> for Open <%S>\n", retry, status, name);

        } else {

            uni.Length = (USHORT)(len * sizeof(WCHAR));
            uni.MaximumLength = (USHORT)(uni.Length + sizeof(WCHAR));

            status = ZwOpenSymbolicLinkObject(
                &handle, SYMBOLIC_LINK_QUERY, &objattrs);

            //DbgPrint("[Retry %d] Status <%08X> for Open <%S>\n", retry, status, name);
        }

        name[len] = save_char;

        if (NT_SUCCESS(status))
            break;

        handle = NULL;

        if (len <= 1)
            break;

        do {
            --len;
        } while (len && name[len] != L'\\');

        if (len <= 1)
            break;
    }

    //
    // if we couldn't locate a symbolic link then we're done
    //

    if (! handle) {

        if (status == STATUS_OBJECT_TYPE_MISMATCH) {

            if (at_least_one_symlink_found)
                status = STATUS_SUCCESS;
            else
                status = STATUS_OBJECT_PATH_SYNTAX_BAD;

        } else if (status == STATUS_OBJECT_NAME_NOT_FOUND ||
                   status == STATUS_OBJECT_PATH_NOT_FOUND)
                status = STATUS_OBJECT_PATH_SYNTAX_BAD;

        //DbgPrint("[Retry %d] Exit1 with Status <%08X>\n", retry, status);

        return status;
    }

    at_least_one_symlink_found = TRUE;

    //
    // move the suffix beyond the symbolic link prefix to the
    // end of the buffer, and then query the symbolic link into
    // the start of the buffer
    //

    rem = wcslen(name) - len;
    ofs = max_len - rem;
    //DbgPrint("[Retry %d] OldLen=%d Rem=%d Ofs=%d\n", retry, len, rem, ofs);
    if (rem)
        wmemmove(name + ofs, name + len, rem);

    uni.Length = (USHORT)((ofs - 1) * sizeof(WCHAR));
    uni.MaximumLength = (USHORT)(uni.Length + sizeof(WCHAR));

    status = ZwQuerySymbolicLinkObject(handle, &uni, NULL);

    ZwClose(handle);

    if (! NT_SUCCESS(status)) {
        //DbgPrint("[Retry %d] Exit2 with Status <%08X>\n", retry, status);
        return status;
    }

    len = uni.Length / sizeof(WCHAR);
    //DbgPrint("[Retry %d] NewLen=%d\n", retry, len);
    if (rem) {
        wmemmove(name + len, name + ofs, rem);
        len += rem;
    }
    name[len] = L'\0';

    goto top;
}


//---------------------------------------------------------------------------
// File_TranslateReparsePoints
//---------------------------------------------------------------------------


_FX void File_InitReparsePoints(BOOLEAN init)
{
    if (init) {

        List_Init(&File_ReparsePointsList);
        Mem_GetLockResource(&File_ReparsePointsLock, TRUE);

    } else {

        Mem_FreeLockResource(&File_ReparsePointsLock);
    }
}


//---------------------------------------------------------------------------
// File_TranslateReparsePoints
//---------------------------------------------------------------------------


_FX WCHAR *File_TranslateReparsePoints(const WCHAR *path, POOL *pool)
{
    WCHAR *ptr, *retpath;
    ULONG len;
    int index;

    if (! File_ReparsePointsLock)
        return NULL;

    //
    // if path is a network path, or does not start with \Device\, then exit
    //

    len = wcslen(path);
    if (len < 8 || _wcsnicmp(path, File_Mup, 8) != 0)
        return NULL;

    if (len > File_RedirectorLen && (   path[File_RedirectorLen] == L'\\'
                                     || path[File_RedirectorLen] == L'\0')
            && _wcsnicmp(path, File_Redirector, File_RedirectorLen) == 0)
        return NULL;

    if (len > File_MupLen        && (   path[File_MupLen] == L'\\'
                                     || path[File_MupLen] == L'\0')
            && _wcsnicmp(path, File_Mup, File_MupLen) == 0)
        return NULL;

    if (len > File_NamedPipeLen  && (   path[File_NamedPipeLen] == L'\\'
                                     || path[File_NamedPipeLen] == L'\0')
            && _wcsnicmp(path, File_NamedPipe, File_NamedPipeLen) == 0)
        return NULL;

    //
    // find the first index of a wildcard character (* or ?)
    //

    ptr = wcschr(path, L'*');
    if (ptr)
        index = (ULONG)(ULONG_PTR)(ptr - path);
    else
        index = -1;
    ptr = wcschr(path, L'?');
    if (ptr) {
        int index_2 = (ULONG)(ULONG_PTR)(ptr - path);
        if ((index == -1) || (index_2 < index))
            index = index_2;
    }

    //
    // if the wildcard immediately follows a backslash then we don't
    // bother with the two-pass check below
    //

    if (index >= 8 && path[index - 1] == L'\\') {
        len = index - 1;
        index = -1;
    }

    //
    // translate the path
    //

    if (index >= 8) {

        //
        // pass 1 for wildcards:  try to translate the string up to the point
        // where the wildcard was found.  For instance for input path
        // C:\PARENT\CHILD* we try to translate C:\PARENT\CHILD.
        //
        // during pass 1, if the path is not found, the return is NULL
        //

        retpath = File_TranslateReparsePoints_2(path, index, pool, 1);
        if (! retpath) {

            //
            // pass 2:  if the path was not found, chop off the last
            // component and try again.  for instance, after trying
            // C:\PARENT\CHILD failed, try C:\PARENT
            //

            ptr = (WCHAR *)(path + index);
            while (ptr > path) {
                --ptr;
                if (*ptr == L'\\')
                    break;
            }

            len = (ULONG)(ULONG_PTR)(ptr - path);
            retpath = File_TranslateReparsePoints_2(path, len, pool, 2);
        }

    } else if (index == -1) {

        //
        // no wildcard in the path:  try just one pass
        //

        retpath = File_TranslateReparsePoints_2(path, len, pool, 2);

    } else {

        //
        // chopping the wildcard created an invalid path
        //

        retpath = NULL;
    }

    return retpath;
}


//---------------------------------------------------------------------------
// File_TranslateReparsePoints_2
//---------------------------------------------------------------------------


_FX WCHAR *File_TranslateReparsePoints_2(
    const WCHAR *path, ULONG len, POOL *pool, ULONG PassNum)
{
    LARGE_INTEGER now;
    CACHE_PATH *entry;
    WCHAR *retpath;
    KIRQL irql;

    //
    // discard any trailing backslashes
    //

    while (len && path[len - 1] == L'\\')
        --len;

    if (! len)
        return NULL;

    //DbgPrint("Checking (%d) %*.*S (originally %S)\n", PassNum, len, len, path, path);

    //
    // try to acquire our lock, but if another thread has released the
    // lock in order to call File_TranslateReparsePoints_3 (see below)
    // then we want to keep waiting
    //

    while (1) {

        KeQuerySystemTime(&now);

        KeRaiseIrql(APC_LEVEL, &irql);
        ExAcquireResourceExclusiveLite(File_ReparsePointsLock, TRUE);

        if (! File_ReparsePointsBusy)
            break;

        ExReleaseResourceLite(File_ReparsePointsLock);
        KeLowerIrql(irql);
        ZwYieldExecution();
    }

    //
    // clean up entries that were created more than 10 seconds ago,
    // but don't check for old entries more than once per second
    //

    if (now.QuadPart - File_ReparsePointsCleanupTime > SECONDS(1)) {

        File_ReparsePointsCleanupTime = now.QuadPart;

        entry = List_Head(&File_ReparsePointsList);
        while (entry) {

            CACHE_PATH *next_entry = List_Next(entry);
            if (now.QuadPart - entry->time > SECONDS(10)) {

                List_Remove(&File_ReparsePointsList, entry);
                Mem_Free(entry, entry->alloc_len);

            }
            entry = next_entry;
        }
    }

    //
    // search for an exact match on the input path
    //

    entry = List_Head(&File_ReparsePointsList);
    while (entry) {
        if (entry->src_len == len && _wcsnicmp(entry->src, path, len) == 0)
            break;
        entry = List_Next(entry);
    }

    //
    // if we could not find a matching entry then create one.
    // note that we have to release the lock and lower IRQL because
    // File_TranslateReparsePoints_3 is doing I/O
    //

    if (! entry) {

        InterlockedIncrement(&File_ReparsePointsBusy);

        ExReleaseResourceLite(File_ReparsePointsLock);
        KeLowerIrql(irql);

        entry = File_TranslateReparsePoints_3(path, len, pool, PassNum);

        KeRaiseIrql(APC_LEVEL, &irql);
        ExAcquireResourceExclusiveLite(File_ReparsePointsLock, TRUE);

        InterlockedDecrement(&File_ReparsePointsBusy);

        if (entry) {
            entry->time = now.QuadPart;
            List_Insert_After(&File_ReparsePointsList, NULL, entry);
        }
    }

    //
    // finish
    //

    if (entry && entry->dst) {

        path += len;
        len = wcslen(path) + 1;
        retpath = Mem_Alloc(pool, (entry->dst_len + len) * sizeof(WCHAR));
        if (retpath) {

            wmemcpy(retpath, entry->dst, entry->dst_len);
            wmemcpy(retpath + entry->dst_len, path, len);
        }

    } else
        retpath = NULL;

    ExReleaseResourceLite(File_ReparsePointsLock);
    KeLowerIrql(irql);

    return retpath;
}


//---------------------------------------------------------------------------
// File_TranslateReparsePoints_3
//---------------------------------------------------------------------------


_FX CACHE_PATH *File_TranslateReparsePoints_3(
    const WCHAR *path, ULONG path_len, POOL *pool, ULONG PassNum)
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE handle;
    WCHAR *path2;
    OBJECT_NAME_INFORMATION *Name;
    ULONG NameLength;
    ULONG dst_len, alloc_len;
    CACHE_PATH *entry;

    //
    // try to open the specified path
    //

    path2 = Mem_Alloc(pool, (path_len + 1) * sizeof(WCHAR));
    if (! path2)
        return NULL;
    wmemcpy(path2, path, path_len);
    path2[path_len] = L'\0';

    InitializeObjectAttributes(&objattrs,
        &objname, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL, Driver_PublicSd);

    RtlInitUnicodeString(&objname, path2);

    status = ZwCreateFile(
        &handle,
        SYNCHRONIZE,
        &objattrs,
        &IoStatusBlock,
        NULL,                   // AllocationSize
        0,                      // FileAttributes
        FILE_SHARE_VALID_FLAGS, // ShareAccess
        FILE_OPEN,              // CreateDisposition
        FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
        NULL, 0);               // EaBuffer, EaLength

    //DbgPrint("Opened %S status %08X\n", path2, status);

    Mem_Free(path2, (path_len + 1) * sizeof(WCHAR));

    //
    // extract the full path
    //

    Name = NULL;
    dst_len = 0;

    if (NT_SUCCESS(status)) {

        FILE_OBJECT *object;
        status = ObReferenceObjectByHandle(
            handle, 0, *IoFileObjectType, KernelMode, &object, NULL);

        if (NT_SUCCESS(status)) {

            Obj_GetName(pool, object, &Name, &NameLength);

            if (Name && (Name != &Obj_Unnamed)) {

                WCHAR *path3 = Name->Name.Buffer;
                dst_len = wcslen(path3);
                while (dst_len && path3[dst_len - 1] == L'\\')
                    --dst_len;
            }

            ObDereferenceObject(object);
        }

        ZwClose(handle);
    }

    //
    // create the new cache entry
    //

    if (dst_len || (PassNum == 2)) {

        alloc_len = sizeof(CACHE_PATH)
                  + (path_len + 1) * sizeof(WCHAR)
                  + (dst_len + 1) * sizeof(WCHAR);
        entry = Mem_Alloc(Driver_Pool, alloc_len);
        if (entry) {

            entry->alloc_len = alloc_len;

            entry->src_len = path_len;
            entry->dst_len = dst_len;
            wmemcpy(entry->src, path, path_len);
            entry->src[path_len] = L'\0';
            if (dst_len) {
                entry->dst = entry->src + path_len + 1;
                wmemcpy(entry->dst, Name->Name.Buffer, dst_len);
                entry->dst[dst_len] = L'\0';
            } else
                entry->dst = NULL;

            //DbgPrint("alloc_len=%d fixed_part=%d src_len=%d dst_len=%d\n", entry->alloc_len, sizeof(CACHE_PATH), entry->src_len, entry->dst_len);
        }

    } else
        entry = NULL;

    //
    // finish
    //

    if (Name && (Name != &Obj_Unnamed))
        Mem_Free(Name, NameLength);

    return entry;
}
