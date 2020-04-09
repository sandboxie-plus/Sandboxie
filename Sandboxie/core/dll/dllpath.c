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
// Path List Management
//---------------------------------------------------------------------------


#define NOGDI
#include "dll.h"
#include "common/pool.h"
#include "common/pattern.c"
#include "common/my_version.h"
#include "core/drv/api_defs.h"


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _PATH_LIST_ANCHOR {

    POOL *pool;
    BOOLEAN file_paths_initialized;
    BOOLEAN key_paths_initialized;
    BOOLEAN ipc_paths_initialized;
    BOOLEAN win_classes_initialized;
    LIST open_file_path;
    LIST closed_file_path;
    LIST write_file_path;
    LIST open_key_path;
    LIST closed_key_path;
    LIST write_key_path;
    LIST open_ipc_path;
    LIST closed_ipc_path;
    LIST open_win_classes;

} PATH_LIST_ANCHOR;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN Dll_InitPathList2(
    ULONG path_code, LIST *open, LIST *closed, LIST *write);

static BOOLEAN Dll_InitPathList3(
    POOL *pool, ULONG path_code, LIST *list);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static PATH_LIST_ANCHOR *Dll_PathListAnchor = NULL;

static CRITICAL_SECTION Dll_FilePathListCritSec;


//---------------------------------------------------------------------------
// Dll_InitPathList
//---------------------------------------------------------------------------


_FX BOOLEAN Dll_InitPathList(void)
{
    PATH_LIST_ANCHOR *anchor;
    POOL *pool;

    InitializeCriticalSectionAndSpinCount(&Dll_FilePathListCritSec, 1000);

    pool = Pool_Create();
    if (! pool) {
        SbieApi_Log(2305, NULL);
        return FALSE;
    }

    anchor = Pool_Alloc(pool, sizeof(PATH_LIST_ANCHOR));
    if (! anchor) {
        SbieApi_Log(2305, NULL);
        return FALSE;
    }
    Dll_PathListAnchor = anchor;

    memzero(anchor, sizeof(PATH_LIST_ANCHOR));
    anchor->pool = pool;

    return TRUE;
}


//---------------------------------------------------------------------------
// Dll_InitPathList2
//---------------------------------------------------------------------------


_FX BOOLEAN Dll_InitPathList2(
    ULONG path_code, LIST *open, LIST *closed, LIST *write)
{
    BOOLEAN ok = TRUE;

    if (ok && open) {
        path_code = (path_code & 0xFF00) | 'o';
        ok = Dll_InitPathList3(Dll_PathListAnchor->pool, path_code, open);
    }

    if (ok && closed) {
        path_code = (path_code & 0xFF00) | 'c';
        ok = Dll_InitPathList3(Dll_PathListAnchor->pool, path_code, closed);
    }

    if (ok && write) {
        path_code = (path_code & 0xFF00) | 'w';
        ok = Dll_InitPathList3(Dll_PathListAnchor->pool, path_code, write);
    }

    if (! ok) {

        WCHAR str[2];
        str[0] = (WCHAR)((path_code & 0xFF00) >> 8);
        str[1] = L'\0';
        SbieApi_Log(2317, str);

        if (open)
            List_Init(open);
        if (closed)
            List_Init(closed);
        if (write)
            List_Init(write);
    }

    return ok;
}


//---------------------------------------------------------------------------
// Dll_InitPathList3
//---------------------------------------------------------------------------


_FX BOOLEAN Dll_InitPathList3(POOL *pool, ULONG path_code, LIST *list)
{
    LONG status;
    ULONG len;
    WCHAR *path;
    WCHAR *ptr;
    PATTERN *pat;
    BOOLEAN ok;

    status = SbieApi_QueryPathList(path_code, &len, NULL, NULL);
    if (status != STATUS_SUCCESS)
        return FALSE;

    path = Dll_AllocTemp(len);
    status = SbieApi_QueryPathList(path_code, NULL, path, NULL);
    if (status != STATUS_SUCCESS) {
        Dll_Free(path);
        return FALSE;
    }

    ok = TRUE;

    ptr = path;
    while (*ptr) {
        pat = Pattern_Create(pool, ptr, TRUE);
        if (! pat) {
            ok = FALSE;
            break;
        }
        List_Insert_After(list, NULL, pat);
        ptr += wcslen(ptr) + 1;
    }

    Dll_Free(path);
    return ok;
}

//---------------------------------------------------------------------------
// SbieDll_MatchPath
//---------------------------------------------------------------------------


_FX ULONG SbieDll_MatchPath(WCHAR path_code, const WCHAR *path)
{
    return SbieDll_MatchPath2(path_code, path, TRUE, TRUE);
}

//---------------------------------------------------------------------------
// SbieDll_MatchPath2
//---------------------------------------------------------------------------


_FX ULONG SbieDll_MatchPath2(WCHAR path_code, const WCHAR *path, BOOLEAN bCheckObjectExists, BOOLEAN bMonitorLog)
{
    LIST *open_list, *closed_list, *write_list;
    PATTERN *pat;
    WCHAR *path_lwr;
    ULONG path_len;
    ULONG mp_flags;
    USHORT monflag;

    mp_flags = 0;

    if (path == (const WCHAR *)-1) {
        path = NULL;
        path_len = 0;
    } else {
        path_len = wcslen(path);
        if (! path_len)
            return 0;
    }

    if (path_code == L'p') {
        path_code = L'f';
        monflag = MONITOR_PIPE;
    } else if (path_code == L'i')
        monflag = MONITOR_IPC;
    else
        monflag = 0;

    //
    // select path list
    //

    if (path_code == L'f') {

        EnterCriticalSection(&Dll_FilePathListCritSec);

        open_list   = &Dll_PathListAnchor->open_file_path;
        closed_list = &Dll_PathListAnchor->closed_file_path;
        write_list  = &Dll_PathListAnchor->write_file_path;

        if (! Dll_PathListAnchor->file_paths_initialized) {
            Dll_InitPathList2('fx', open_list, closed_list, write_list);
            Dll_PathListAnchor->file_paths_initialized = TRUE;
        }

        if (monflag == MONITOR_PIPE)    // if path_code was L'p' then
            write_list = NULL;          // don't check write-only paths

    } else if (path_code == L'k') {

        open_list   = &Dll_PathListAnchor->open_key_path;
        closed_list = &Dll_PathListAnchor->closed_key_path;
        write_list  = &Dll_PathListAnchor->write_key_path;

        if (! Dll_PathListAnchor->key_paths_initialized) {
            Dll_InitPathList2('kx', open_list, closed_list, write_list);
            Dll_PathListAnchor->key_paths_initialized = TRUE;
        }

    } else if (path_code == L'i') {

        open_list   = &Dll_PathListAnchor->open_ipc_path;
        closed_list = &Dll_PathListAnchor->closed_ipc_path;
        write_list  = NULL;

        if (! Dll_PathListAnchor->ipc_paths_initialized) {
            Dll_InitPathList2('ix', open_list, closed_list, NULL);
            Dll_PathListAnchor->ipc_paths_initialized = TRUE;
        }

    } else if (path_code == L'w') {

        open_list   = &Dll_PathListAnchor->open_win_classes;
        closed_list = NULL;
        write_list  = NULL;

        if (! Dll_PathListAnchor->win_classes_initialized) {
            Dll_InitPathList2('wx', open_list, NULL, NULL);
            Dll_PathListAnchor->win_classes_initialized = TRUE;
        }

    } else
        return 0;

    //
    // scan paths list.  if the path to match does not already end with
    // a backslash character, we will check it twice, second time with
    // a suffixing backslash.  this will make sure we match C:\X even
    // even when {Open,Closed}XxxPath=C:\X\ (with a backslash suffix)
    //

    path_lwr = Dll_AllocTemp((path_len + 4) * sizeof(WCHAR));

    wmemcpy(path_lwr, path, path_len);
    path_lwr[path_len]     = L'\0';
    path_lwr[path_len + 1] = L'\0';
    _wcslwr(path_lwr);

    //
    // ClosedXxxPath
    //

    if (closed_list && path_len) {

        pat = List_Head(closed_list);
        while (pat) {

            if (Pattern_Match(pat, path_lwr, path_len)) {
                mp_flags |= PATH_CLOSED_FLAG;
                break;
            }

            if (path_lwr[path_len - 1] != L'\\') {
                path_lwr[path_len] = L'\\';
                if (Pattern_Match(pat, path_lwr, path_len + 1)) {

                    path_lwr[path_len] = L'\0';
                    mp_flags |= PATH_CLOSED_FLAG;
                    break;
                }
                path_lwr[path_len] = L'\0';
            }

            pat = List_Next(pat);
        }
    }

    //
    // WriteXxxPath - only if ClosedXxxPath matched
    //

    if (write_list && mp_flags && path_len) {

        pat = List_Head(write_list);
        while (pat) {

            if (Pattern_Match(pat, path_lwr, path_len)) {
                mp_flags &= ~PATH_CLOSED_FLAG;
                mp_flags |= PATH_WRITE_FLAG;
                break;
            }

            if (path_lwr[path_len - 1] != L'\\') {
                path_lwr[path_len] = L'\\';
                if (Pattern_Match(pat, path_lwr, path_len + 1)) {

                    path_lwr[path_len] = L'\0';
                    mp_flags &= ~PATH_CLOSED_FLAG;
                    mp_flags |= PATH_WRITE_FLAG;
                    break;
                }
                path_lwr[path_len] = L'\0';
            }

            pat = List_Next(pat);
        }
    }

    //
    // OpenXxxPath - only if no setting matched
    //

    if (open_list && (! mp_flags)  && path_len) {

        pat = List_Head(open_list);
        while (pat) {

            if (Pattern_Match(pat, path_lwr, path_len)) {
                mp_flags |= PATH_OPEN_FLAG;
                break;
            }

            if (path_lwr[path_len - 1] != L'\\') {
                path_lwr[path_len] = L'\\';
                if (Pattern_Match(pat, path_lwr, path_len + 1)) {

                    path_lwr[path_len] = L'\0';
                    mp_flags |= PATH_OPEN_FLAG;
                    break;
                }
                path_lwr[path_len] = L'\0';
            }

            pat = List_Next(pat);
        }
    }

    if (path_code == L'f')
        LeaveCriticalSection(&Dll_FilePathListCritSec);

    //
    // make sure that Sandboxie resources marked "always in box"
    // will not match any OpenIpcPath or ClosedIpcPath settings
    //

    if (path_code == L'i' && mp_flags && path) {

        WCHAR *LastBackSlash = wcsrchr(path, L'\\');
        if (LastBackSlash && wcsncmp(LastBackSlash + 1,
                                SBIE_BOXED_, SBIE_BOXED_LEN) == 0) {

            mp_flags = 0;
        }
    }

    //
    // log access request in the resource access monitor
    //

    if (monflag) {

        if (! monflag)
            monflag = MONITOR_IPC;
        if (PATH_IS_CLOSED(mp_flags))
            monflag |= MONITOR_DENY;
        else if (PATH_IS_OPEN(mp_flags))
            monflag |= MONITOR_OPEN;

        if (bMonitorLog)
        {
            SbieApi_MonitorPut2(monflag, path_lwr, bCheckObjectExists);
        }
    }

    Dll_Free(path_lwr);

    return mp_flags;
}


//---------------------------------------------------------------------------
// Dll_RefreshPathList
//---------------------------------------------------------------------------


_FX void Dll_RefreshPathList(void)
{
    if (! Dll_PathListAnchor)
        return;

    EnterCriticalSection(&Dll_FilePathListCritSec);

    if (SbieApi_CallZero(API_REFRESH_FILE_PATH_LIST) == STATUS_SUCCESS) {

        LIST open_paths, closed_paths, write_paths;

        List_Init(&open_paths);
        List_Init(&closed_paths);
        List_Init(&write_paths);

        if (Dll_InitPathList2('fx',
                    &open_paths, &closed_paths, &write_paths)) {

            memcpy(&Dll_PathListAnchor->open_file_path,     &open_paths,
                   sizeof(LIST));
            memcpy(&Dll_PathListAnchor->closed_file_path,   &closed_paths,
                   sizeof(LIST));
            memcpy(&Dll_PathListAnchor->write_file_path,    &write_paths,
                   sizeof(LIST));

            Dll_PathListAnchor->file_paths_initialized = TRUE;
        }
    }

    LeaveCriticalSection(&Dll_FilePathListCritSec);
}
