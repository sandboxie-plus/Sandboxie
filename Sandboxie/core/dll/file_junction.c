/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2026 David Xanatos, xanasoft.com
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
// File (Junction)
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _FILE_JUNCTION_ENTRY {

    WCHAR *src;                 // virtual (source) path, in DOS form
    ULONG src_len;              // in characters, excluding NULL
    WCHAR *dst;                 // real (target) path, in DOS form
    ULONG dst_len;              // in characters, excluding NULL

    WCHAR *src_nt;              // virtual (source) path, in NT device form
    ULONG src_nt_len;           // in characters, excluding NULL
    WCHAR *dst_nt;              // real (target) path, in NT device form
    ULONG dst_nt_len;           // in characters, excluding NULL

} FILE_JUNCTION_ENTRY;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static FILE_JUNCTION_ENTRY *File_Junction_FindForward(
    const WCHAR *Path, ULONG PathLen, BOOLEAN *pMatchedDos);

static FILE_JUNCTION_ENTRY *File_Junction_FindReverse(
    const WCHAR *Path, ULONG PathLen, BOOLEAN *pMatchedDos);

static BOOLEAN File_Junction_IsBoundary(
    const WCHAR *Path, ULONG PrefixLen);

static WCHAR *File_Junction_TrimString(WCHAR *str, ULONG *len);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static FILE_JUNCTION_ENTRY *File_JunctionEntries = NULL;
static ULONG File_JunctionCount = 0;
static BOOLEAN File_Junction_BlockRawAccess = TRUE;
static ULONG File_Junction_PolicyVersion = 0;


//---------------------------------------------------------------------------
// File_Junction_IsBoundary
//---------------------------------------------------------------------------


static BOOLEAN File_Junction_IsBoundary(const WCHAR *Path, ULONG PrefixLen)
{
    //
    // a drive root prefix, like C:\, matches any path on that drive
    //

    if (PrefixLen >= 3 && Path[PrefixLen - 1] == L'\\' &&
            Path[PrefixLen - 2] == L':')
        return TRUE;

    if (Path[PrefixLen] == L'\\' || Path[PrefixLen] == L'\0')
        return TRUE;
    return FALSE;
}


//---------------------------------------------------------------------------
// File_Junction_TrimString
//---------------------------------------------------------------------------


static WCHAR *File_Junction_TrimString(WCHAR *str, ULONG *len)
{
    ULONG l;

    while (*str == L' ' || *str == L'\t')
        ++str;

    l = wcslen(str);
    while (l > 0 && (str[l - 1] == L' ' || str[l - 1] == L'\t'))
        str[--l] = L'\0';

    *len = l;
    return str;
}


//---------------------------------------------------------------------------
// File_Junction_IsDosPath
//---------------------------------------------------------------------------


static BOOLEAN File_Junction_IsDosPath(const WCHAR *Path, ULONG PathLen)
{
    return PathLen >= 3 && Path[1] == L':';
}


//---------------------------------------------------------------------------
// File_Junction_FindForward
//---------------------------------------------------------------------------


static FILE_JUNCTION_ENTRY *File_Junction_FindForward(
    const WCHAR *Path, ULONG PathLen, BOOLEAN *pMatchedDos)
{
    ULONG i;
    ULONG best_len = 0;
    BOOLEAN best_dos = FALSE;
    FILE_JUNCTION_ENTRY *best = NULL;
    FILE_JUNCTION_ENTRY *entry;

    for (i = 0; i < File_JunctionCount; ++i) {

        entry = &File_JunctionEntries[i];

        //
        // try the DOS form of the source path
        //

        if (entry->src && entry->src_len <= PathLen &&
                entry->src_len >= best_len &&
                File_Junction_IsBoundary(Path, entry->src_len) &&
                _wcsnicmp(Path, entry->src, entry->src_len) == 0) {

            best = entry;
            best_len = entry->src_len;
            best_dos = TRUE;
        }

        //
        // try the NT device form of the source path
        //

        if (entry->src_nt && entry->src_nt_len <= PathLen &&
                entry->src_nt_len >= best_len &&
                File_Junction_IsBoundary(Path, entry->src_nt_len) &&
                _wcsnicmp(Path, entry->src_nt, entry->src_nt_len) == 0) {

            best = entry;
            best_len = entry->src_nt_len;
            best_dos = FALSE;
        }
    }

    if (pMatchedDos)
        *pMatchedDos = best_dos;

    return best;
}


//---------------------------------------------------------------------------
// File_Junction_FindReverse
//---------------------------------------------------------------------------


static FILE_JUNCTION_ENTRY *File_Junction_FindReverse(
    const WCHAR *Path, ULONG PathLen, BOOLEAN *pMatchedDos)
{
    ULONG i;
    ULONG best_len = 0;
    BOOLEAN best_dos = FALSE;
    FILE_JUNCTION_ENTRY *best = NULL;
    FILE_JUNCTION_ENTRY *entry;

    for (i = 0; i < File_JunctionCount; ++i) {

        entry = &File_JunctionEntries[i];

        //
        // try the DOS form of the target path
        //

        if (entry->dst && entry->dst_len <= PathLen &&
                entry->dst_len >= best_len &&
                File_Junction_IsBoundary(Path, entry->dst_len) &&
                _wcsnicmp(Path, entry->dst, entry->dst_len) == 0) {

            best = entry;
            best_len = entry->dst_len;
            best_dos = TRUE;
        }

        //
        // try the NT device form of the target path
        //

        if (entry->dst_nt && entry->dst_nt_len <= PathLen &&
                entry->dst_nt_len >= best_len &&
                File_Junction_IsBoundary(Path, entry->dst_nt_len) &&
                _wcsnicmp(Path, entry->dst_nt, entry->dst_nt_len) == 0) {

            best = entry;
            best_len = entry->dst_nt_len;
            best_dos = FALSE;
        }
    }

    if (pMatchedDos)
        *pMatchedDos = best_dos;

    return best;
}


//---------------------------------------------------------------------------
// File_Junction_BlockRawAccessPath
//---------------------------------------------------------------------------


_FX BOOLEAN File_Junction_BlockRawAccessPath(const WCHAR *Path, ULONG PathLen)
{
    BOOLEAN matched_dos;

    if (! File_Junction_BlockRawAccess)
        return FALSE;
    return File_Junction_FindReverse(Path, PathLen, &matched_dos) != NULL;
}


//---------------------------------------------------------------------------
// File_Junction_IsMappedSrc
//---------------------------------------------------------------------------


_FX BOOLEAN File_Junction_IsMappedSrc(const WCHAR *Path, ULONG PathLen)
{
    BOOLEAN matched_dos;

    return File_Junction_FindForward(Path, PathLen, &matched_dos) != NULL;
}


//---------------------------------------------------------------------------
// File_Junction_GetPolicyVersion
//---------------------------------------------------------------------------


_FX ULONG File_Junction_GetPolicyVersion(void)
{
    return File_Junction_PolicyVersion;
}


//---------------------------------------------------------------------------
// File_Junction_IsHomePath
//---------------------------------------------------------------------------


_FX BOOLEAN File_Junction_IsHomePath(const WCHAR *Path, ULONG PathLen)
{
    const WCHAR *home = File_Junction_IsDosPath(Path, PathLen)
                            ? Dll_HomeDosPath : Dll_HomeNtPath;
    ULONG home_len;

    if (! home)
        return FALSE;

    home_len = wcslen(home);

    if (home_len > PathLen)
        return FALSE;
    if (_wcsnicmp(Path, home, home_len) != 0)
        return FALSE;

    if (home_len == PathLen)
        return TRUE;
    if (Path[home_len] == L'\\')
        return TRUE;
    return FALSE;
}


//---------------------------------------------------------------------------
// File_InitJunctions
//---------------------------------------------------------------------------


_FX void File_InitJunctions(void)
{
    ULONG index;
    WCHAR buf[CONF_LINE_LEN];
    WCHAR *comma;
    WCHAR *src;
    WCHAR *dst;
    ULONG src_len;
    ULONG dst_len;
    NTSTATUS status;
    FILE_JUNCTION_ENTRY *entry;
    FILE_JUNCTION_ENTRY *table;

    if (File_JunctionEntries) {

        for (index = 0; index < File_JunctionCount; ++index) {
            Dll_Free(File_JunctionEntries[index].src);
            Dll_Free(File_JunctionEntries[index].dst);
            Dll_Free(File_JunctionEntries[index].src_nt);
            Dll_Free(File_JunctionEntries[index].dst_nt);
        }

        Dll_Free(File_JunctionEntries);
        File_JunctionEntries = NULL;
        File_JunctionCount = 0;
    }

    //
    // determine whether raw (reverse) access to junction destination
    // paths should be blocked;  defaults to yes, disable with
    // JunctionBlockRawAccess=n
    //

    File_Junction_BlockRawAccess = SbieApi_QueryConfBool(
        Dll_BoxName, L"JunctionBlockRawAccess", TRUE);

    //
    // policy version:  0 (default) denies process creation from a
    // junction source folder, since the kernel may fail to read the
    // image through the virtual path.  version 1 overrides (disables)
    // JunctionBlockRawAccess and instead remaps process images started
    // from a junction source folder to the junction target at process
    // creation time.
    //

    File_Junction_PolicyVersion = 0;

    {
        WCHAR val[CONF_LINE_LEN];

        if (NT_SUCCESS(SbieApi_QueryConf(
                Dll_BoxName, L"JunctionPolicyVersion", 0,
                val, sizeof(val) - sizeof(WCHAR)))) {

            File_Junction_PolicyVersion = (ULONG)_wtoi(val);

            if (File_Junction_PolicyVersion >= 1)
                File_Junction_BlockRawAccess = FALSE;
        }
    }

    index = 0;

    while (1) {

        status = SbieApi_QueryConf(
            Dll_BoxName, L"JunctionPath", index, buf, sizeof(buf) - sizeof(WCHAR));
        if (! NT_SUCCESS(status))
            break;

        ++index;

        comma = wcschr(buf, L',');
        if (! comma)
            continue;

        *comma = L'\0';

        src = File_Junction_TrimString(buf, &src_len);
        dst = File_Junction_TrimString(comma + 1, &dst_len);

        if (! src_len || ! dst_len)
            continue;

        while (src_len > 3 && src[src_len - 1] == L'\\')
            src[--src_len] = L'\0';

        while (dst_len > 3 && dst[dst_len - 1] == L'\\')
            dst[--dst_len] = L'\0';

        table = Dll_Alloc((File_JunctionCount + 1) * sizeof(FILE_JUNCTION_ENTRY));
        if (! table)
            break;

        if (File_JunctionCount) {
            memcpy(table, File_JunctionEntries,
                    File_JunctionCount * sizeof(FILE_JUNCTION_ENTRY));
            Dll_Free(File_JunctionEntries);
        }

        File_JunctionEntries = table;
        entry = &File_JunctionEntries[File_JunctionCount++];

        entry->src = Dll_Alloc((src_len + 1) * sizeof(WCHAR));
        entry->dst = Dll_Alloc((dst_len + 1) * sizeof(WCHAR));
        if (! entry->src || ! entry->dst) {
            File_JunctionCount = index - 1;
            break;
        }

        wmemcpy(entry->src, src, src_len + 1);
        wmemcpy(entry->dst, dst, dst_len + 1);
        entry->src_len = src_len;
        entry->dst_len = dst_len;

        //
        // also record the NT device form of the paths, because the
        // junction mapping in File_GetName operates on NT device paths
        // while process creation checks may pass DOS paths.  if a path
        // can't be converted (for example an unknown drive letter) the
        // NT form is left NULL and only the DOS form matches.
        //

        entry->src_nt = NULL;
        entry->dst_nt = NULL;
        entry->src_nt_len = 0;
        entry->dst_nt_len = 0;

        {
            WCHAR *src_nt = File_TranslateDosToNtPath(entry->src);
            if (src_nt) {
                ULONG src_nt_len = wcslen(src_nt);
                entry->src_nt = Dll_Alloc((src_nt_len + 1) * sizeof(WCHAR));
                if (entry->src_nt) {
                    wmemcpy(entry->src_nt, src_nt, src_nt_len + 1);
                    entry->src_nt_len = src_nt_len;
                }
                Dll_Free(src_nt);
            }

            WCHAR *dst_nt = File_TranslateDosToNtPath(entry->dst);
            if (dst_nt) {
                ULONG dst_nt_len = wcslen(dst_nt);
                entry->dst_nt = Dll_Alloc((dst_nt_len + 1) * sizeof(WCHAR));
                if (entry->dst_nt) {
                    wmemcpy(entry->dst_nt, dst_nt, dst_nt_len + 1);
                    entry->dst_nt_len = dst_nt_len;
                }
                Dll_Free(dst_nt);
            }
        }
    }

    //
    // diagnostic:  report the loaded junction entries so that a failure
    // to map can be traced back to the entries or the path forms
    //

    if (File_JunctionCount) {

        FILE_JUNCTION_ENTRY *e = &File_JunctionEntries[0];
        WCHAR dbg[CONF_LINE_LEN * 2 + 96];

        Sbie_snwprintf(dbg, CONF_LINE_LEN * 2 + 96,
            L"junction: %u entries, src=%s dst=%s src_nt=%s dst_nt=%s",
            File_JunctionCount,
            e->src ? e->src : L"<null>",
            e->dst ? e->dst : L"<null>",
            e->src_nt ? e->src_nt : L"<null>",
            e->dst_nt ? e->dst_nt : L"<null>");

        SbieApi_MonitorPut(MONITOR_FILE | MONITOR_OPEN, dbg);
    }
}


//---------------------------------------------------------------------------
// File_ApplyJunctionMap
//---------------------------------------------------------------------------


_FX WCHAR *File_ApplyJunctionMap(THREAD_DATA *TlsData, WCHAR *TruePath)
{
    FILE_JUNCTION_ENTRY *best;
    BOOLEAN matched_dos;
    const WCHAR *src, *dst;
    ULONG src_len, dst_len;
    ULONG TruePath_len;
    ULONG NewPath_len;
    WCHAR *NewPath;

    TruePath_len = wcslen(TruePath);

    best = File_Junction_FindForward(TruePath, TruePath_len, &matched_dos);
    if (! best)
        return TruePath;

    if (matched_dos) {
        src = best->src;
        src_len = best->src_len;
        dst = best->dst;
        dst_len = best->dst_len;
    }
    else {
        src = best->src_nt;
        src_len = best->src_nt_len;
        dst = best->dst_nt;
        dst_len = best->dst_nt_len;
    }

    NewPath_len = dst_len + (TruePath_len - src_len);

    NewPath = Dll_AllocTemp((NewPath_len + 1) * sizeof(WCHAR));

    wmemmove(NewPath + dst_len, TruePath + src_len,
                NewPath_len - dst_len + 1);
    wmemcpy(NewPath, dst, dst_len);

    return NewPath;
}


//---------------------------------------------------------------------------
// File_ApplyJunctionMapReverse
//---------------------------------------------------------------------------


_FX WCHAR *File_ApplyJunctionMapReverse(
    THREAD_DATA *TlsData, WCHAR *Path, ULONG Path_len)
{
    FILE_JUNCTION_ENTRY *best;
    BOOLEAN matched_dos;
    const WCHAR *src, *dst;
    ULONG src_len, dst_len;
    ULONG NewPath_len;
    WCHAR *NewPath;

    best = File_Junction_FindReverse(Path, Path_len, &matched_dos);
    if (! best)
        return NULL;

    if (matched_dos) {
        src = best->src;
        src_len = best->src_len;
        dst = best->dst;
        dst_len = best->dst_len;
    }
    else {
        src = best->src_nt;
        src_len = best->src_nt_len;
        dst = best->dst_nt;
        dst_len = best->dst_nt_len;
    }

    NewPath_len = src_len + (Path_len - dst_len);

    NewPath = Dll_AllocTemp((NewPath_len + 1) * sizeof(WCHAR));

    wmemmove(NewPath + src_len, Path + dst_len,
                NewPath_len - src_len + 1);
    wmemcpy(NewPath, src, src_len);

    return NewPath;
}


//---------------------------------------------------------------------------
// File_ApplyJunctionMapReverseInPlace
//---------------------------------------------------------------------------


_FX ULONG File_ApplyJunctionMapReverseInPlace(
    WCHAR *Path, ULONG Path_len, ULONG MaxLen)
{
    FILE_JUNCTION_ENTRY *best;
    BOOLEAN matched_dos;
    const WCHAR *src, *dst;
    ULONG src_len, dst_len;
    ULONG NewPath_len;

    best = File_Junction_FindReverse(Path, Path_len, &matched_dos);
    if (! best)
        return 0;

    if (matched_dos) {
        src = best->src;
        src_len = best->src_len;
        dst = best->dst;
        dst_len = best->dst_len;
    }
    else {
        src = best->src_nt;
        src_len = best->src_nt_len;
        dst = best->dst_nt;
        dst_len = best->dst_nt_len;
    }

    NewPath_len = src_len + (Path_len - dst_len);
    if (NewPath_len + 1 > MaxLen)
        return 0;

    memmove(Path + src_len, Path + dst_len,
                (Path_len - dst_len + 1) * sizeof(WCHAR));
    wmemcpy(Path, src, src_len);

    return NewPath_len;
}
