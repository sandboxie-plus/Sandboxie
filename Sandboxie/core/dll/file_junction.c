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

} FILE_JUNCTION_ENTRY;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static FILE_JUNCTION_ENTRY *File_Junction_FindForward(
    const WCHAR *Path, ULONG PathLen);

static FILE_JUNCTION_ENTRY *File_Junction_FindReverse(
    const WCHAR *Path, ULONG PathLen);

static BOOLEAN File_Junction_IsBoundary(
    const WCHAR *Path, ULONG PrefixLen);

static WCHAR *File_Junction_TrimString(WCHAR *str, ULONG *len);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static FILE_JUNCTION_ENTRY *File_JunctionEntries = NULL;
static ULONG File_JunctionCount = 0;
static BOOLEAN File_Junction_BlockRawAccess = TRUE;


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
// File_Junction_FindForward
//---------------------------------------------------------------------------


static FILE_JUNCTION_ENTRY *File_Junction_FindForward(
    const WCHAR *Path, ULONG PathLen)
{
    ULONG i;
    ULONG best_len = 0;
    FILE_JUNCTION_ENTRY *best = NULL;
    FILE_JUNCTION_ENTRY *entry;

    for (i = 0; i < File_JunctionCount; ++i) {

        entry = &File_JunctionEntries[i];

        if (entry->src_len > PathLen)
            continue;
        if (entry->src_len < best_len)
            continue;
        if (! File_Junction_IsBoundary(Path, entry->src_len))
            continue;
        if (_wcsnicmp(Path, entry->src, entry->src_len) != 0)
            continue;

        best = entry;
        best_len = entry->src_len;
    }

    return best;
}


//---------------------------------------------------------------------------
// File_Junction_FindReverse
//---------------------------------------------------------------------------


static FILE_JUNCTION_ENTRY *File_Junction_FindReverse(
    const WCHAR *Path, ULONG PathLen)
{
    ULONG i;
    ULONG best_len = 0;
    FILE_JUNCTION_ENTRY *best = NULL;
    FILE_JUNCTION_ENTRY *entry;

    if (PathLen < 3 || Path[1] != L':')
        return NULL;

    for (i = 0; i < File_JunctionCount; ++i) {

        entry = &File_JunctionEntries[i];

        if (entry->dst_len > PathLen)
            continue;
        if (entry->dst_len < best_len)
            continue;
        if (! File_Junction_IsBoundary(Path, entry->dst_len))
            continue;
        if (_wcsnicmp(Path, entry->dst, entry->dst_len) != 0)
            continue;

        best = entry;
        best_len = entry->dst_len;
    }

    return best;
}


//---------------------------------------------------------------------------
// File_Junction_BlockRawAccessPath
//---------------------------------------------------------------------------


_FX BOOLEAN File_Junction_BlockRawAccessPath(const WCHAR *Path, ULONG PathLen)
{
    if (! File_Junction_BlockRawAccess)
        return FALSE;
    return File_Junction_FindReverse(Path, PathLen) != NULL;
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
    }
}


//---------------------------------------------------------------------------
// File_ApplyJunctionMap
//---------------------------------------------------------------------------


_FX WCHAR *File_ApplyJunctionMap(THREAD_DATA *TlsData, WCHAR *TruePath)
{
    FILE_JUNCTION_ENTRY *best;
    ULONG TruePath_len;
    ULONG NewPath_len;
    WCHAR *NewPath;

    TruePath_len = wcslen(TruePath);

    best = File_Junction_FindForward(TruePath, TruePath_len);
    if (! best)
        return TruePath;

    NewPath_len = best->dst_len + (TruePath_len - best->src_len);

    NewPath = Dll_GetTlsNameBuffer(
        TlsData, TRUE_NAME_BUFFER, (NewPath_len + 1) * sizeof(WCHAR));

    wmemmove(NewPath + best->dst_len, TruePath + best->src_len,
                NewPath_len - best->dst_len + 1);
    wmemcpy(NewPath, best->dst, best->dst_len);

    return NewPath;
}


//---------------------------------------------------------------------------
// File_ApplyJunctionMapReverse
//---------------------------------------------------------------------------


_FX WCHAR *File_ApplyJunctionMapReverse(
    THREAD_DATA *TlsData, WCHAR *Path, ULONG Path_len)
{
    FILE_JUNCTION_ENTRY *best;
    ULONG NewPath_len;
    WCHAR *NewPath;

    best = File_Junction_FindReverse(Path, Path_len);
    if (! best)
        return NULL;

    NewPath_len = best->src_len + (Path_len - best->dst_len);

    NewPath = Dll_GetTlsNameBuffer(
        TlsData, TRUE_NAME_BUFFER, (NewPath_len + 1) * sizeof(WCHAR));

    wmemmove(NewPath + best->src_len, Path + best->dst_len,
                NewPath_len - best->src_len + 1);
    wmemcpy(NewPath, best->src, best->src_len);

    return NewPath;
}


//---------------------------------------------------------------------------
// File_ApplyJunctionMapReverseInPlace
//---------------------------------------------------------------------------


_FX ULONG File_ApplyJunctionMapReverseInPlace(
    WCHAR *Path, ULONG Path_len, ULONG MaxLen)
{
    FILE_JUNCTION_ENTRY *best;
    ULONG NewPath_len;

    best = File_Junction_FindReverse(Path, Path_len);
    if (! best)
        return 0;

    NewPath_len = best->src_len + (Path_len - best->dst_len);
    if (NewPath_len + 1 > MaxLen)
        return 0;

    memmove(Path + best->src_len, Path + best->dst_len,
                (Path_len - best->dst_len + 1) * sizeof(WCHAR));
    wmemcpy(Path, best->src, best->src_len);

    return NewPath_len;
}
