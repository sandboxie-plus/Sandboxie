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
// File (Link)
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


struct _FILE_DRIVE {

    WCHAR letter;
    WCHAR sn[10];
    BOOLEAN subst;
    ULONG len;          // in characters, excluding NULL
    WCHAR path[0];

};


struct _FILE_LINK {

    LIST_ELEM list_elem;
    ULONG ticks;
    BOOLEAN same;
    BOOLEAN stop;
    ULONG src_len;      // in characters, excluding NULL
    ULONG dst_len;      // in characters, excluding NULL
    WCHAR *dst;
    WCHAR src[1];

};


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static const FILE_DRIVE *File_GetDriveForPath(
    const WCHAR *Path, ULONG PathLen);

static const FILE_DRIVE *File_GetDriveForUncPath(
    const WCHAR *Path, ULONG PathLen, ULONG *OutPrefixLen);

static FILE_DRIVE *File_GetDriveForLetter(WCHAR drive_letter);

static const FILE_DRIVE *File_GetDriveAndLinkForPath(
    const WCHAR *Path, ULONG PathLen, const FILE_LINK **OutLink);

static BOOLEAN File_AddLink(
    BOOLEAN PermLink, const WCHAR *Src, const WCHAR *Dst);

static void File_RemovePermLinks(const WCHAR *path);

static WCHAR *File_TranslateTempLinks(
    WCHAR *TruePath, BOOLEAN StripLastPathComponent);

static WCHAR *File_TranslateTempLinks_2(WCHAR *input_str, ULONG input_len);

static FILE_LINK *File_AddTempLink(WCHAR *path);

static ULONG File_FixPermLinksForTempLink(
    WCHAR *name, ULONG name_len, ULONG max_len);

static FILE_LINK *File_FindPermLinksForMatchPath(
    const WCHAR *name, ULONG name_len);

static WCHAR *File_FixPermLinksForMatchPath(const WCHAR *name);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static CRITICAL_SECTION *File_DrivesAndLinks_CritSec = NULL;

static FILE_DRIVE **File_Drives = NULL;

static LIST *File_PermLinks = NULL;
static LIST *File_TempLinks = NULL;


//---------------------------------------------------------------------------
// File_GetDriveForPath
//---------------------------------------------------------------------------


_FX const FILE_DRIVE *File_GetDriveForPath(const WCHAR *Path, ULONG PathLen)
{
    const FILE_DRIVE *drive;
    ULONG i;

    EnterCriticalSection(File_DrivesAndLinks_CritSec);

    for (i = 0; i < 26; ++i) {

        drive = File_Drives[i];
        if (drive) {

            if (PathLen >= drive->len
                    && _wcsnicmp(Path, drive->path, drive->len) == 0) {

                //
                // make sure access to \Device\HarddiskVolume10 (for M:),
                // for instance, is not matched by \Device\HarddiskVolume1
                // (for C:), by requiring a backslash or null character
                // to follow the matching drive path
                //

                const WCHAR *ptr = Path + drive->len;
                if (*ptr == L'\\' || *ptr == L'\0')
                    break;
            }

            drive = NULL;
        }
    }

    if (! drive)
        LeaveCriticalSection(File_DrivesAndLinks_CritSec);

    return drive;
}


//---------------------------------------------------------------------------
// File_GetDriveForUncPath
//---------------------------------------------------------------------------


_FX const FILE_DRIVE *File_GetDriveForUncPath(
    const WCHAR *Path, ULONG PathLen, ULONG *OutPrefixLen)
{
    const FILE_DRIVE *drive;
    WCHAR *ptr1a;
    ULONG pfx_len, sfx_len;
    ULONG i;

    //
    // given an input path like
    // \Device\LanmanRedirector\server\share\file.txt
    // find the corresponding drive even if it is stored as
    // \Device\LanmanRedirector\;Q:0000000000001234\server\share
    //

    *OutPrefixLen = 0;

    if (PathLen <= 8 || _wcsnicmp(Path, File_Mup, 8) != 0)
        return NULL;
    ptr1a = wcschr(Path + 8, L'\\');
    if (! ptr1a)
        return NULL;
    pfx_len = (ULONG)(ptr1a - Path);
    sfx_len = wcslen(ptr1a);

    //
    //
    //

    EnterCriticalSection(File_DrivesAndLinks_CritSec);

    for (i = 0; i < 26; ++i) {

        drive = File_Drives[i];
        if (drive && drive->len > 8 && // make sure it starts with \device
                _wcsnicmp(drive->path, File_Mup, 8) == 0) {

            //
            // make sure the device component is the same
            //

            const WCHAR *ptr1b = wcschr(drive->path + 8, L'\\');
            if (ptr1b && (ptr1b - drive->path) == pfx_len
                    && *(ptr1b + 1) == L';'
                    && _wcsnicmp(Path, drive->path, pfx_len) == 0) {

                const WCHAR *ptr2b = wcschr(ptr1b + 1, L'\\');
                if (ptr2b) {

                    ULONG sfx_len_2 = wcslen(ptr2b);
                    if (sfx_len >= sfx_len_2
                            && _wcsnicmp(ptr1a, ptr2b, sfx_len_2) == 0) {

                        const WCHAR *ptr2a = ptr1a + sfx_len_2;
                        if (*ptr2a == L'\\' || *ptr2a == L'\0') {

                            *OutPrefixLen = (ULONG)(ptr2a - Path);
                            break;
                        }
                    }
                }
            }

            drive = NULL;
        }
    }

    if (! drive)
        LeaveCriticalSection(File_DrivesAndLinks_CritSec);

    return drive;
}


//---------------------------------------------------------------------------
// File_GetDriveForLetter
//---------------------------------------------------------------------------


_FX FILE_DRIVE *File_GetDriveForLetter(WCHAR drive_letter)
{
    FILE_DRIVE *drive;

    EnterCriticalSection(File_DrivesAndLinks_CritSec);

    if (drive_letter >= L'A' && drive_letter <= L'Z')
        drive = File_Drives[drive_letter - L'A'];
    else if (drive_letter >= L'a' && drive_letter <= L'z')
        drive = File_Drives[drive_letter - L'a'];
    else
        drive = NULL;

    if (! drive)
        LeaveCriticalSection(File_DrivesAndLinks_CritSec);

    return drive;
}


//---------------------------------------------------------------------------
// File_AddLink
//---------------------------------------------------------------------------


_FX BOOLEAN File_AddLink(
    BOOLEAN PermLink, const WCHAR *Src, const WCHAR *Dst)
{
    ULONG src_len;
    ULONG dst_len;
    ULONG drive_letter;
    ULONG alloc_len;
    FILE_DRIVE *src_drive;
    FILE_DRIVE *dst_drive;
    FILE_LINK *link, *old_link;

    //
    // prepare a new link structure
    //

    src_drive = NULL;
    src_len = wcslen(Src);
    if (Src[1] == L':') {
        drive_letter = Src[0];
        if (drive_letter >= L'A' && drive_letter <= L'Z')
            drive_letter = drive_letter - L'A' + L'a';
        if (drive_letter >= L'a' && drive_letter <= L'z') {
            src_drive = File_Drives[drive_letter - L'a'];
            if (src_drive)
                src_len += src_drive->len;
        }
    }

    dst_drive = NULL;
    dst_len = wcslen(Dst);
    if (Dst[1] == L':') {
        drive_letter = Dst[0];
        if (drive_letter >= L'A' && drive_letter <= L'Z')
            drive_letter = drive_letter - L'A' + L'a';
        if (drive_letter >= L'a' && drive_letter <= L'z') {
            dst_drive = File_Drives[drive_letter - L'a'];
            if (dst_drive)
                dst_len += dst_drive->len;
        }
    }

    alloc_len = sizeof(FILE_LINK)
              + (src_len + 4) * sizeof(WCHAR)
              + (dst_len + 4) * sizeof(WCHAR);
    link = Dll_Alloc(alloc_len);

    //
    // prepare src path
    //

    if (src_drive) {
        wcscpy(link->src, src_drive->path);
        wcscat(link->src, Src + 2);
    } else
        wcscpy(link->src, Src);
    link->src_len = wcslen(link->src);

    while (link->src_len && link->src[link->src_len - 1] == L'\\') {
        link->src[link->src_len - 1] = L'\0';
        --link->src_len;
    }

    //
    // prepare dst path
    //

    link->dst = link->src + link->src_len + 1;
    *link->dst = L'\0';
    ++link->dst;

    if (dst_drive) {
        wcscpy(link->dst, dst_drive->path);
        wcscat(link->dst, Dst + 2);
    } else
        wcscpy(link->dst, Dst);
    link->dst_len = wcslen(link->dst);

    while (link->dst_len && link->dst[link->dst_len - 1] == L'\\') {
        link->dst[link->dst_len - 1] = L'\0';
        --link->dst_len;
    }

    //
    // abort if src or dst are null
    //

    if ((! link->src_len) || (! link->dst_len)) {

        Dll_Free(link);
        return FALSE;
    }

    //
    // abort if src and dst are the same
    //

    if (PermLink && link->src_len == link->dst_len
                 && _wcsicmp(link->src, link->dst) == 0) {

        Dll_Free(link);
        return FALSE;
    }

    //
    // abort if entry was already added for this src.
    //
    // in the case of a volume not mounted as a drive letter,
    // the specified src can be the destination of an old entry
    //
    // if not duplicate, add the new link
    //

    EnterCriticalSection(File_DrivesAndLinks_CritSec);

    if (PermLink) {

        old_link = List_Head(File_PermLinks);
        while (old_link) {

            if (    (old_link->src_len == link->src_len &&
                        _wcsicmp(old_link->src, link->src) == 0)
                ||  (old_link->dst_len == link->src_len &&
                        _wcsicmp(old_link->dst, link->src) == 0)) {

                LeaveCriticalSection(File_DrivesAndLinks_CritSec);

                Dll_Free(link);
                return FALSE;
            }

            old_link = List_Next(old_link);
        }

        link->ticks = 0;
        link->same = FALSE;

        List_Insert_After(File_PermLinks, NULL, link);

    } else {

        link->ticks = GetTickCount();
        if (link->src_len == link->dst_len &&
                _wcsicmp(link->src, link->dst) == 0)
            link->same = TRUE;
        else
            link->same = FALSE;

        List_Insert_Before(File_TempLinks, NULL, link);
    }

    LeaveCriticalSection(File_DrivesAndLinks_CritSec);

    return TRUE;
}


//---------------------------------------------------------------------------
// File_RemovePermLinks
//---------------------------------------------------------------------------


_FX void File_RemovePermLinks(const WCHAR *path)
{
    ULONG path_len;
    FILE_LINK *old_link;

    //
    // if a drive is being unmounted (or remounted), scan the list of
    // reparse links and remove any references to its related device.
    //

    path_len = wcslen(path);

    old_link = List_Head(File_PermLinks);
    while (old_link) {

        FILE_LINK *next_link = List_Next(old_link);

        const ULONG src_len = old_link->src_len;
        const WCHAR *src    = old_link->src;
        if (src_len >= path_len &&
                (src[path_len] == L'\\' || src[path_len] == L'\0') &&
                _wcsnicmp(path, src, path_len) == 0) {

            List_Remove(File_PermLinks, old_link);
            Dll_Free(old_link);
        }

        old_link = next_link;
    }
}


//---------------------------------------------------------------------------
// FILE_IS_REDIRECTOR_OR_MUP
//---------------------------------------------------------------------------


#define FILE_IS_REDIRECTOR_OR_MUP(str,len)                              \
   (    (len >= File_RedirectorLen &&                                   \
            0 == _wcsnicmp(str, File_Redirector, File_RedirectorLen))   \
    ||  (len >= File_MupLen &&                                          \
            0 == _wcsnicmp(str, File_Mup, File_MupLen)))


//---------------------------------------------------------------------------
// File_TranslateTempLinks
//---------------------------------------------------------------------------


_FX WCHAR *File_TranslateTempLinks(
    WCHAR *TruePath, BOOLEAN StripLastPathComponent)
{
    static ULONG cleanup_ticks = 0;
    FILE_LINK *link;
    WCHAR *ret;
    ULONG TruePath_len, ret_len;

    //
    // entry
    //

    ULONG ticks = GetTickCount();
    /*static ULONG TimeSpentHere = 0;
    static ULONG TimeSpentHereLastReport = 0;*/

    //
    // initialize length of input path
    //

    if (StripLastPathComponent) {
        WCHAR *backslash = wcsrchr(TruePath, L'\\');
        if (! backslash)
            return NULL;
        TruePath_len = (ULONG)(ULONG_PTR)(backslash - TruePath);
    } else {
        TruePath_len = wcslen(TruePath);
        while (TruePath_len && TruePath[TruePath_len - 1] == L'\\')
            --TruePath_len;
        if (! TruePath_len)
            return NULL;
    }

    //
    // make sure the path is for a local drive
    //

    if (1) {
        const FILE_DRIVE *drive;
        if (FILE_IS_REDIRECTOR_OR_MUP(TruePath, TruePath_len))
            drive = NULL;
        else
            drive = File_GetDriveForPath(TruePath, TruePath_len);
        if (drive) {
            if (drive->len == TruePath_len)
                drive = NULL;
            LeaveCriticalSection(File_DrivesAndLinks_CritSec);
        }
        if (! drive)
            return NULL;
    }

    ret = NULL;

    //
    // on first loop throughout this function, clean old entries
    //

    EnterCriticalSection(File_DrivesAndLinks_CritSec);

    if (ticks - cleanup_ticks > 1000) {

        cleanup_ticks = ticks;

        link = List_Head(File_TempLinks);
        while (link) {
            FILE_LINK *next_link = List_Next(link);
            if (ticks - link->ticks > 10 * 1000) {
                List_Remove(File_TempLinks, link);
                Dll_Free(link);
            }
            link = next_link;
        }
    }

    //
    // look for an exact match in the list of temporary links
    //

    link = List_Head(File_TempLinks);
    while (link) {

        if (link->src_len == TruePath_len
                && 0 == _wcsnicmp(link->src, TruePath, TruePath_len)) {

            if (! link->same) {

                //
                // link->dst is different from link->src, so we need to
                // append the last component to link->dst
                //

                ULONG rem = wcslen(TruePath) - TruePath_len + 1;
                ret = Dll_AllocTemp((link->dst_len + rem) * sizeof(WCHAR));
                wmemcpy(ret, link->dst, link->dst_len);
                wmemcpy(ret + link->dst_len, TruePath + TruePath_len, rem);
            }

            goto finish;
        }

        link = List_Next(link);
    }

    //
    // if there was no exact match then process the path
    //

    if (StripLastPathComponent)
        TruePath[TruePath_len] = L'\0';

    ret = File_TranslateTempLinks_2(TruePath, TruePath_len);

    if (StripLastPathComponent)
        TruePath[TruePath_len] = L'\\';

    //
    // add a link from the original true path to the final result
    //

    link = List_Head(File_TempLinks);
    while (link) {
        if (link->src_len == TruePath_len
                && 0 == _wcsnicmp(link->src, TruePath, TruePath_len))
            break;
        link = List_Next(link);
    }

    if (! link) {

        if (StripLastPathComponent)
            TruePath[TruePath_len] = L'\0';

        File_AddLink(FALSE, TruePath, ret);

        if (StripLastPathComponent)
            TruePath[TruePath_len] = L'\\';
    }

    //
    // if result is same as input, then we can return null
    //

    ret_len = wcslen(ret);

    if (ret_len == TruePath_len && 0 == _wcsnicmp(ret, TruePath, ret_len)) {

        Dll_Free(ret);
        ret = NULL;
        goto finish;
    }

    //
    // append the last component to the return path
    //

    if (StripLastPathComponent) {

        ULONG  rem_len = wcslen(TruePath) - TruePath_len + 1;
        WCHAR *tmp_ret = Dll_AllocTemp((ret_len + rem_len) * sizeof(WCHAR));
        wmemcpy(tmp_ret, ret, ret_len);
        wmemcpy(tmp_ret + ret_len, TruePath + TruePath_len, rem_len);
        Dll_Free(ret);
        ret = tmp_ret;
    }

    //
    // finish
    //

finish:

    /*TimeSpentHere += GetTickCount() - ticks;
    if (TimeSpentHere - TimeSpentHereLastReport > 5000) {
        WCHAR txt[256];
        Sbie_snwprintf(txt, 256, L"Time Spent On Links = %d\n", TimeSpentHere);
        OutputDebugString(txt);
        TimeSpentHereLastReport = TimeSpentHere;
    }*/

    LeaveCriticalSection(File_DrivesAndLinks_CritSec);
    return ret;
}


//---------------------------------------------------------------------------
// File_TranslateTempLinks_2
//---------------------------------------------------------------------------


_FX WCHAR *File_TranslateTempLinks_2(WCHAR *input_str, ULONG input_len)
{
    FILE_LINK *link, *best_link;
    WCHAR *work_str;
    ULONG prefix_len, work_len;

    //
    // duplicate the input string as a work area
    //

    work_str = Dll_AllocTemp((input_len + 1) * sizeof(WCHAR));
    wmemcpy(work_str, input_str, input_len);
    work_str[input_len] = L'\0';
    work_len = input_len;

    //
    // main loop
    //

    while (1) {

        //
        // find longest matching prefix from the list of temporary links
        //

        prefix_len = 0;
        best_link = NULL;

        link = List_Head(File_TempLinks);
        while (link) {

            if (        link->src_len <= work_len
                    &&  link->src_len > prefix_len
                    &&  (   work_str[link->src_len] == L'\\'
                         || work_str[link->src_len] == L'\0')
                    &&  0 == _wcsnicmp(link->src, work_str, link->src_len)) {

                prefix_len = link->src_len;
                best_link = link;
            }

            //
            // keep trying
            //

            link = List_Next(link);
        }

        link = best_link;

        //
        // if we found a prefix, combine it with rest of string, then
        // restart the loop in case we have a link for the combined path
        //

        if (link) {

            if (! link->same) {

                const ULONG dst_len = link->dst_len;
                ULONG  rem_len = work_len - prefix_len + 1;
                WCHAR *new_str =
                    Dll_AllocTemp((dst_len + rem_len) * sizeof(WCHAR));
                wmemcpy(new_str, link->dst, dst_len);
                wmemcpy(new_str + dst_len, work_str + prefix_len, rem_len);

                Dll_Free(work_str);
                work_str = new_str;
                work_len = dst_len + rem_len - 1;

                if ((! link->stop) && (rem_len > 1))
                    continue;

                break;

            } else if (work_len == prefix_len || link->stop)
                break;

        } else {

            //
            // otherwise make sure we are dealing with a local drive
            //

            const FILE_DRIVE *drive;
            if (FILE_IS_REDIRECTOR_OR_MUP(work_str, work_len))
                drive = NULL;
            else {
                drive = File_GetDriveForPath(work_str, work_len);
                if (drive) {
                    prefix_len = drive->len;
                    LeaveCriticalSection(File_DrivesAndLinks_CritSec);
                    if (prefix_len == work_len)
                        drive = NULL;
                }
            }

            if (! drive)
                break;
        }

        //
        // at this point we either did not find a prefix, or we found a
        // prefix where the destination is the same as the source, i.e.
        // a normal directory path, so check the next directory on the path
        //

        if (1) {

            ULONG save_index;
            WCHAR save_char;
            WCHAR *backslash = wcschr(work_str + prefix_len + 1, L'\\');
            if (backslash)
                save_index = (ULONG)(ULONG_PTR)(backslash - work_str);
            else
                save_index = work_len;

            save_char = work_str[save_index];
            work_str[save_index] = L'\0';
            link = File_AddTempLink(work_str);
            work_str[save_index] = save_char;

            if (! link) // this can only happen due to an internal error
                break;
        }
    }

    //
    // finish
    //

    return work_str;
}


//---------------------------------------------------------------------------
// File_AddTempLink
//---------------------------------------------------------------------------


_FX FILE_LINK *File_AddTempLink(WCHAR *path)
{
    NTSTATUS status;
    HANDLE handle;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_LINK *link;
    WCHAR *newpath;
    BOOLEAN stop;
    BOOLEAN bPermLinkPath = FALSE;

    //
    // try to open the path
    //

    P_NtCreateFile pNtCreateFile = __sys_NtCreateFile;
    P_NtClose pNtClose = __sys_NtClose;
    if (! pNtCreateFile)
        pNtCreateFile = NtCreateFile;
    if (! pNtClose)
        pNtClose = NtClose;

    stop = TRUE;

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    RtlInitUnicodeString(&objname, path);

    status = pNtCreateFile(
        &handle, FILE_READ_ATTRIBUTES | SYNCHRONIZE, &objattrs,
        &IoStatusBlock, NULL, 0, FILE_SHARE_VALID_FLAGS,
        FILE_OPEN, FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
        NULL, 0);

    if (NT_SUCCESS(status)) {

        //
        // get the reparsed absolute path
        //

        const ULONG PATH_BUF_LEN = 1024;
        newpath = Dll_AllocTemp(PATH_BUF_LEN);

        status = SbieApi_GetFileName(handle, PATH_BUF_LEN - 4, newpath);
        if (NT_SUCCESS(status)) {

            //
            // make sure path does not contain duplicate backslashes
            //

            ULONG len = wcslen(newpath);
            WCHAR *name = newpath;
            while (name[0]) {
                if (name[0] == L'\\' && name[1] == L'\\') {
                    ULONG move_len = len - (ULONG)(name - newpath) + 1;
                    wmemmove(name, name + 1, move_len);
                    --len;
                } else
                    ++name;
            }

            //
            // convert permanent links (i.e. drive mount points)
            //

            len = File_FixPermLinksForTempLink(
                newpath, len, (PATH_BUF_LEN - 4) / sizeof(WCHAR));

            //
            // verify the link points to a local drive
            //

            if (File_FindPermLinksForMatchPath(newpath, len))
            {
                // release lock by File_FindPermLinksForMatchPath
                LeaveCriticalSection(File_DrivesAndLinks_CritSec);
                bPermLinkPath = TRUE;
            }

            if (! FILE_IS_REDIRECTOR_OR_MUP(newpath, len) && !bPermLinkPath) {
                const FILE_DRIVE *drive = File_GetDriveForPath(newpath, len);
                if (drive) {
                    LeaveCriticalSection(File_DrivesAndLinks_CritSec);
                    stop = FALSE;
                }
            }

        } else {
            Dll_Free(newpath);
            newpath = NULL;
        }

        pNtClose(handle);

    } else
        newpath = NULL;

    //
    // add the new link and return
    //

    if (! newpath)
        newpath = path;

    if (File_AddLink(FALSE, path, newpath)) {
        link = List_Head(File_TempLinks);
        if (link)
            link->stop = stop;
    } else
        link = NULL;

    if (newpath != path)
        Dll_Free(newpath);

    return link;
}


//---------------------------------------------------------------------------
// File_FixPermLinksForTempLink
//---------------------------------------------------------------------------


_FX ULONG File_FixPermLinksForTempLink(
    WCHAR *name, ULONG name_len, ULONG max_len)
{
    //
    // given a path 'name' of length 'name_len'
    // in a buffer with a maximum length of 'max_len'
    // replace any perm links at the start of the path
    //

    const FILE_LINK *link;
    ULONG retries = 0;

    link = List_Head(File_PermLinks);
    while (link) {

        const ULONG src_len = link->src_len;

        if (
#ifdef WOW64_FS_REDIR
            link != File_Wow64FileLink &&
#endif WOW64_FS_REDIR
            name_len >= src_len &&
                (name[src_len] == L'\\' || name[src_len] == L'\0') &&
                _wcsnicmp(name, link->src, src_len) == 0) {

            const ULONG dst_len = link->dst_len;
            if (dst_len + name_len - src_len <= max_len) {

                if (src_len != dst_len)
                    wmemmove(name + dst_len,
                             name + src_len,
                             name_len - src_len + 1);
                wmemcpy(name, link->dst, dst_len);
                name_len -= src_len;
                name_len += dst_len;

                link = List_Head(File_PermLinks);
                ++retries;
                if (retries == 16)
                    break;
                else
                    continue;
            }
        }

        link = List_Next(link);
    }

    return name_len;
}


//---------------------------------------------------------------------------
// File_GetDriveAndLinkForPath
//---------------------------------------------------------------------------


_FX const FILE_DRIVE *File_GetDriveAndLinkForPath(
    const WCHAR *Path, ULONG PathLen, const FILE_LINK **OutLink)
{
    //
    // given a path 'name' of length 'name_len'
    // in a buffer with a maximum length of 'max_len'
    // replace any perm links at the start of the path
    //

    const FILE_DRIVE *drive;
    const FILE_LINK *link;

    *OutLink = NULL;

    EnterCriticalSection(File_DrivesAndLinks_CritSec);

    link = List_Head(File_PermLinks);
    while (link) {

        const ULONG src_len = link->src_len;

        if (
#ifdef WOW64_FS_REDIR
            link != File_Wow64FileLink &&
#endif WOW64_FS_REDIR
            PathLen >= src_len &&
                (Path[PathLen] == L'\\' || Path[PathLen] == L'\0') &&
                _wcsnicmp(Path, link->src, src_len) == 0) {

            *OutLink = link;
            break;
        }

        link = List_Next(link);
    }

    drive = File_GetDriveForPath(Path, PathLen);
    if (drive)
        LeaveCriticalSection(File_DrivesAndLinks_CritSec);

    // on exit, File_DrivesAndLinks_CritSec is locked just once

    return drive;
}


//---------------------------------------------------------------------------
// File_FindPermLinksForMatchPath
//---------------------------------------------------------------------------


_FX FILE_LINK *File_FindPermLinksForMatchPath(
    const WCHAR *name, ULONG name_len)
{
    //
    // given a path 'name' with length 'name_len'
    // find any perm links at the start of the path
    //

    FILE_LINK *link;

    EnterCriticalSection(File_DrivesAndLinks_CritSec);

    link = List_Head(File_PermLinks);
    while (link) {

        const ULONG dst_len = link->dst_len;

        if (
#ifdef WOW64_FS_REDIR
            link != File_Wow64FileLink &&
#endif WOW64_FS_REDIR
            name_len >= dst_len &&
                (name[dst_len] == L'\\' || name[dst_len] == L'\0') &&
                _wcsnicmp(name, link->dst, dst_len) == 0) {

            return link;
        }

        link = List_Next(link);
    }

    LeaveCriticalSection(File_DrivesAndLinks_CritSec);

    return NULL;
}


//---------------------------------------------------------------------------
// File_FixPermLinksForMatchPath
//---------------------------------------------------------------------------


_FX WCHAR *File_FixPermLinksForMatchPath(const WCHAR *name)
{
    //
    // given a path 'name' replace any perm links at the start of the path
    //

    FILE_LINK *link;
    ULONG name_len = wcslen(name);

    link = File_FindPermLinksForMatchPath(name, name_len);
    if (link) {

        const ULONG dst_len = link->dst_len;
        const ULONG src_len = link->src_len;

        const ULONG rem_len = name_len - dst_len + 1;

        WCHAR *name2 = Dll_AllocTemp((src_len + rem_len) * sizeof(WCHAR));
        wmemcpy(name2, link->src, src_len);
        wmemcpy(name2 + src_len, name + dst_len, rem_len);

        LeaveCriticalSection(File_DrivesAndLinks_CritSec);

        return name2;

    } else
        return NULL;
}
