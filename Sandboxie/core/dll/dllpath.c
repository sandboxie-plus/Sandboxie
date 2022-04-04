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
#include "core/drv/api_flags.h"

#define USE_MATCH_PATH_EX

//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _PATH_LIST_ANCHOR {

    POOL *pool;

    BOOLEAN file_paths_initialized;
    BOOLEAN key_paths_initialized;
    BOOLEAN ipc_paths_initialized;
    BOOLEAN win_classes_initialized;

#ifdef USE_MATCH_PATH_EX
    LIST normal_file_path;
#endif
    LIST open_file_path;
    LIST closed_file_path;
    LIST write_file_path;
#ifdef USE_MATCH_PATH_EX
    LIST read_file_path;
#endif

#ifdef USE_MATCH_PATH_EX
    LIST normal_key_path;
#endif
    LIST open_key_path;
    LIST closed_key_path;
    LIST write_key_path;
#ifdef USE_MATCH_PATH_EX
    LIST read_key_path;
#endif

#ifdef USE_MATCH_PATH_EX
    LIST normal_ipc_path;
#endif
    LIST open_ipc_path;
    LIST closed_ipc_path;

    LIST open_win_classes;

} PATH_LIST_ANCHOR;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


#ifdef USE_MATCH_PATH_EX
static BOOLEAN Dll_InitPathList2(
    ULONG path_code, LIST *normal, LIST *open, LIST *closed, LIST *write, LIST *read);
#else
static BOOLEAN Dll_InitPathList2(
    ULONG path_code, LIST *open, LIST *closed, LIST *write);
#endif

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

#ifdef USE_MATCH_PATH_EX
_FX BOOLEAN Dll_InitPathList2(
    ULONG path_code, LIST *normal, LIST *open, LIST *closed, LIST *write, LIST *read)
#else
_FX BOOLEAN Dll_InitPathList2(
    ULONG path_code, LIST *open, LIST *closed, LIST *write)
#endif
{
    BOOLEAN ok = TRUE;

#ifdef USE_MATCH_PATH_EX
    if (ok && normal) {
        path_code = (path_code & 0xFF00) | 'n';
        ok = Dll_InitPathList3(Dll_PathListAnchor->pool, path_code, normal);
    }
#endif

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

#ifdef USE_MATCH_PATH_EX
    if (ok && read) {
        path_code = (path_code & 0xFF00) | 'r';
        ok = Dll_InitPathList3(Dll_PathListAnchor->pool, path_code, read);
    }
#endif

    if (! ok) {

        WCHAR str[2];
        str[0] = (WCHAR)((path_code & 0xFF00) >> 8);
        str[1] = L'\0';
        SbieApi_Log(2317, str);

#ifdef USE_MATCH_PATH_EX
        if (normal)
            List_Init(normal);
#endif
        if (open)
            List_Init(open);
        if (closed)
            List_Init(closed);
        if (write)
            List_Init(write);
#ifdef USE_MATCH_PATH_EX
        if (read)
            List_Init(read);
#endif
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

    status = SbieApi_QueryPathList(path_code, &len, NULL, NULL, TRUE);
    if (status != STATUS_SUCCESS)
        return FALSE;

    path = Dll_AllocTemp(len);
    status = SbieApi_QueryPathList(path_code, NULL, path, NULL, TRUE);
    if (status != STATUS_SUCCESS) {
        Dll_Free(path);
        return FALSE;
    }

    ok = TRUE;

    ptr = path;
    // while (*ptr) {
    while (*((ULONG*)ptr) != -1) {
        ULONG level = *((ULONG*)ptr);
        ptr += sizeof(ULONG)/sizeof(WCHAR);
        pat = Pattern_Create(pool, ptr, TRUE, level);
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

#ifdef USE_MATCH_PATH_EX

//---------------------------------------------------------------------------
// SbieDll_GetReadablePaths
//---------------------------------------------------------------------------


_FX void SbieDll_GetReadablePaths(WCHAR path_code, LIST **lists)
{
    if (path_code == L'f') {

        EnterCriticalSection(&Dll_FilePathListCritSec);

        lists[0] = &Dll_PathListAnchor->normal_file_path;
        lists[1] = &Dll_PathListAnchor->open_file_path;
        lists[2] = &Dll_PathListAnchor->read_file_path;
        lists[3] = NULL;

    } else if (path_code == L'k') {

        lists[0] = &Dll_PathListAnchor->normal_key_path;
        lists[1] = &Dll_PathListAnchor->open_key_path;
        lists[2] = &Dll_PathListAnchor->read_key_path;
        lists[3] = NULL;

    } else if (path_code == L'i') {

        lists[0] = &Dll_PathListAnchor->normal_ipc_path;
        lists[1] = &Dll_PathListAnchor->open_ipc_path;
        lists[2] = NULL;

    }
}

_FX void SbieDll_ReleaseFilePathLock()
{
    LeaveCriticalSection(&Dll_FilePathListCritSec);
}


//---------------------------------------------------------------------------
// Process_MatchPathList
//---------------------------------------------------------------------------


_FX int Process_MatchPathList(
    WCHAR *path_lwr, ULONG path_len, LIST *list, ULONG* plevel, const WCHAR** patsrc)
{
    PATTERN *pat;
    int match_len = 0;
    ULONG level = plevel ? *plevel : -1; // lower is better, 3 is max value

    pat = List_Head(list);
    while (pat) {

        ULONG cur_level = Pattern_Level(pat);
        if (cur_level > level)
            goto next; // no point testing patters with a to weak level

        int cur_len = Pattern_MatchX(pat, path_lwr, path_len);
        if (cur_len > match_len) {
            match_len = cur_len;
            level = cur_level;
            if (patsrc) *patsrc = Pattern_Source(pat);
            
            // we need to test all entries to find the best match, so we dont break here
        }

        //
        // if we have a pattern like C:\Windows\,
        // we still want it to match a path like C:\Windows,
        // hence we add a L'\\' to the path and check again
        //

        else if (path_lwr[path_len - 1] != L'\\') { 
            path_lwr[path_len] = L'\\';
            cur_len = Pattern_MatchX(pat, path_lwr, path_len + 1);
            path_lwr[path_len] = L'\0';
            if (cur_len > match_len) {
                match_len = cur_len;
                level = cur_level;
                if (patsrc) *patsrc = Pattern_Source(pat);
            }
        }

    next:
        pat = List_Next(pat);
    }

    if (plevel) *plevel = level;
    return match_len;
}
#endif


//---------------------------------------------------------------------------
// SbieDll_MatchPath2
//---------------------------------------------------------------------------


_FX ULONG SbieDll_MatchPath2(WCHAR path_code, const WCHAR *path, BOOLEAN bCheckObjectExists, BOOLEAN bMonitorLog)
{
#ifdef USE_MATCH_PATH_EX
    LIST *normal_list, *open_list, *closed_list, *write_list, *read_list;
#else
    LIST *open_list, *closed_list, *write_list;
    PATTERN *pat;
#endif
    WCHAR *path_lwr;
    ULONG path_len;
    ULONG mp_flags;
    ULONG monflag;

    mp_flags = 0;

    if (path == (const WCHAR *)-1) {
        path = NULL;
        path_len = 0;
    } else {
        path_len = wcslen(path);
        if (! path_len)
            return 0;
    }

    if (path_code == L'f') {
        monflag = MONITOR_FILE;
    } else if (path_code == L'k') {
        monflag = MONITOR_KEY;
    } else if (path_code == L'p') {
        path_code = L'f';
        monflag = MONITOR_PIPE;
    } else if (path_code == L'i') {
        monflag = MONITOR_IPC;
        if (path && path[0] == L'\\' && path[1] == L'K'
          && (wcsncmp(path, L"\\KnownDlls", 10) == 0)) // this will be traced by the driver
            monflag = 0;
    } else if (path_code == L'w') {
        monflag = MONITOR_WINCLASS;
    } else
        monflag = MONITOR_OTHER;

    //
    // select path list
    //

    if (path_code == L'f') {

        EnterCriticalSection(&Dll_FilePathListCritSec);

#ifdef USE_MATCH_PATH_EX
        normal_list = &Dll_PathListAnchor->normal_file_path;
#endif
        open_list   = &Dll_PathListAnchor->open_file_path;
        closed_list = &Dll_PathListAnchor->closed_file_path;
        write_list  = &Dll_PathListAnchor->write_file_path;
#ifdef USE_MATCH_PATH_EX
        read_list   = &Dll_PathListAnchor->read_file_path;
#endif

        if (! Dll_PathListAnchor->file_paths_initialized) {
#ifdef USE_MATCH_PATH_EX
            Dll_InitPathList2('fx', normal_list, open_list, closed_list, write_list, read_list);
#else
            Dll_InitPathList2('fx', open_list, closed_list, write_list);
#endif
            Dll_PathListAnchor->file_paths_initialized = TRUE;
        }

        if (monflag == MONITOR_PIPE)    // if path_code was L'p' then
            write_list = NULL;          // don't check write-only paths

    } else if (path_code == L'k') {

#ifdef USE_MATCH_PATH_EX
        normal_list = &Dll_PathListAnchor->normal_key_path;
#endif
        open_list   = &Dll_PathListAnchor->open_key_path;
        closed_list = &Dll_PathListAnchor->closed_key_path;
        write_list  = &Dll_PathListAnchor->write_key_path;
#ifdef USE_MATCH_PATH_EX
        read_list   = &Dll_PathListAnchor->read_key_path;
#endif

        if (! Dll_PathListAnchor->key_paths_initialized) {
#ifdef USE_MATCH_PATH_EX
            Dll_InitPathList2('kx', normal_list, open_list, closed_list, write_list, read_list);
#else
            Dll_InitPathList2('kx', open_list, closed_list, write_list);
#endif
            Dll_PathListAnchor->key_paths_initialized = TRUE;
        }

    } else if (path_code == L'i') {

#ifdef USE_MATCH_PATH_EX
        normal_list = &Dll_PathListAnchor->normal_ipc_path;
#endif
        open_list   = &Dll_PathListAnchor->open_ipc_path;
        closed_list = &Dll_PathListAnchor->closed_ipc_path;
        write_list  = NULL;
#ifdef USE_MATCH_PATH_EX
        read_list   = NULL;
#endif

        if (! Dll_PathListAnchor->ipc_paths_initialized) {
#ifdef USE_MATCH_PATH_EX
            Dll_InitPathList2('ix', normal_list, open_list, closed_list, NULL, NULL);
#else
            Dll_InitPathList2('ix', open_list, closed_list, NULL);
#endif
            Dll_PathListAnchor->ipc_paths_initialized = TRUE;
        }

    } else if (path_code == L'w') {

#ifdef USE_MATCH_PATH_EX
        normal_list = NULL;
#endif
        open_list   = &Dll_PathListAnchor->open_win_classes;
        closed_list = NULL;
        write_list  = NULL;
#ifdef USE_MATCH_PATH_EX
        read_list   = NULL;
#endif

        if (! Dll_PathListAnchor->win_classes_initialized) {
#ifdef USE_MATCH_PATH_EX
            Dll_InitPathList2('wx', NULL, open_list, NULL, NULL, NULL);
#else
            Dll_InitPathList2('wx', open_list, NULL, NULL);
#endif
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

#ifdef USE_MATCH_PATH_EX
  
    //const WCHAR* curpat;
    ULONG cur_level;
    int cur_len;
    int match_len;
    ULONG level;

    BOOLEAN use_rule_specificity = (path_code == L'f' || path_code == L'k' || path_code == L'i') && (Dll_ProcessFlags & SBIE_FLAG_RULE_SPECIFICITY) != 0;

    //
    // set default behavioure 
    //

    level = 3; // 3 - global default - lower is better, 3 is max value
    match_len = 0;
    if ((path_code == L'f' || path_code == L'k' || path_code == L'i') && (Dll_ProcessFlags & SBIE_FLAG_PRIVACY_MODE) != 0) {

        mp_flags = PATH_WRITE_FLAG; // write path mode
    }
    else {

        mp_flags = 0; // normal mode
    }

    //
    // ClosedXxxPath
    //
    
    if (closed_list && path_len) {
        cur_level = level;
        cur_len = Process_MatchPathList(path_lwr, path_len, closed_list, &cur_level, NULL);// &curpat);
        if (cur_level <= level && cur_len > match_len) {
            level = cur_level;
            match_len = cur_len;
            //if (patsrc) *patsrc = curpat;

            mp_flags = PATH_CLOSED_FLAG;
            if (!use_rule_specificity) goto finish;
        }
    }
    
    //
    // WriteXxxPath
    //
    
    if (write_list && path_len) {
        cur_level = level;
        cur_len = Process_MatchPathList(path_lwr, path_len, write_list, &cur_level, NULL);// &curpat);
        if (cur_level <= level && cur_len > match_len) {
            level = cur_level;
            match_len = cur_len;
            //if (patsrc) *patsrc = curpat;

            mp_flags = PATH_WRITE_FLAG;
            if (!use_rule_specificity) goto finish;
        }
    }
    
    //
    // ReadXxxPath
    //
    
    if (read_list && path_len) {
        cur_level = level;
        cur_len = Process_MatchPathList(path_lwr, path_len,read_list, &cur_level, NULL);// &curpat);
        if (cur_level <= level && cur_len > match_len) {
            level = cur_level;
            match_len = cur_len;
            //if (patsrc) *patsrc = curpat;

            mp_flags = PATH_OPEN_FLAG; // say its open and let the driver deny the write access
            if (!use_rule_specificity) goto finish;
        }
    }
    
    //
    // NormalXxxPath
    //
    
    if (normal_list && path_len) {
        cur_level = level;
        cur_len = Process_MatchPathList(path_lwr, path_len, normal_list, &cur_level, NULL);// &curpat);
        if (cur_level <= level && cur_len > match_len) {
            level = cur_level;
            match_len = cur_len;
            //if (patsrc) *patsrc = curpat;

            mp_flags = 0;
            // dont goto finish as open can overwrite this 
        }
    }

    //
    // OpenXxxPath
    //
    
    if (open_list && path_len) {
        cur_level = level;
        cur_len = Process_MatchPathList(path_lwr, path_len, open_list, &cur_level, NULL);// &curpat);
        if (cur_level <= level && cur_len > match_len) {
            level = cur_level;
            match_len = cur_len;
            //if (patsrc) *patsrc = curpat;

            mp_flags = PATH_OPEN_FLAG;
        }
    }

finish:

#else
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

    if (write_list /*&& mp_flags*/ && path_len) {

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
#endif

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

    if (path && monflag) {

        if (PATH_IS_CLOSED(mp_flags))
            monflag |= MONITOR_DENY;
        // If hts file or key it will be logged by the driver's trace facility
        // we only have to log closed events as those never reach the driver
        else if (monflag == MONITOR_FILE || monflag == MONITOR_KEY)
            bMonitorLog = FALSE;
        else if (PATH_IS_OPEN(mp_flags))
            monflag |= MONITOR_OPEN;

        if (bMonitorLog)
        {
            SbieApi_MonitorPut2(monflag, path, bCheckObjectExists);
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

    if (SbieApi_Call(API_REFRESH_FILE_PATH_LIST, 0) == STATUS_SUCCESS) {

#ifdef USE_MATCH_PATH_EX
        LIST normal_paths, open_paths, closed_paths, write_paths, read_paths;
#else
        LIST open_paths, closed_paths, write_paths;
#endif

#ifdef USE_MATCH_PATH_EX
        List_Init(&normal_paths);
#endif
        List_Init(&open_paths);
        List_Init(&closed_paths);
        List_Init(&write_paths);
#ifdef USE_MATCH_PATH_EX
        List_Init(&read_paths);
#endif

#ifdef USE_MATCH_PATH_EX
        if (Dll_InitPathList2('fx',
                    &normal_paths, &open_paths, &closed_paths, &write_paths, &read_paths)) {
#else
        if (Dll_InitPathList2('fx',
                    &open_paths, &closed_paths, &write_paths)) {
#endif

#ifdef USE_MATCH_PATH_EX
            memcpy(&Dll_PathListAnchor->normal_file_path,   &normal_paths,
                   sizeof(LIST));
#endif
            memcpy(&Dll_PathListAnchor->open_file_path,     &open_paths,
                   sizeof(LIST));
            memcpy(&Dll_PathListAnchor->closed_file_path,   &closed_paths,
                   sizeof(LIST));
            memcpy(&Dll_PathListAnchor->write_file_path,    &write_paths,
                   sizeof(LIST));
#ifdef USE_MATCH_PATH_EX
            memcpy(&Dll_PathListAnchor->read_file_path,     &read_paths,
                   sizeof(LIST));
#endif

            Dll_PathListAnchor->file_paths_initialized = TRUE;
        }
    }

    LeaveCriticalSection(&Dll_FilePathListCritSec);
}


//---------------------------------------------------------------------------
// SbieDll_IsParentReadable
//---------------------------------------------------------------------------


_FX BOOLEAN SbieDll_HasReadableSubPath(WCHAR path_code, const WCHAR* TruePath)
{
    BOOLEAN FoundReadable = FALSE;

    LIST* lists[4];
    SbieDll_GetReadablePaths(path_code, lists);

    ULONG TruePathLen = wcslen(TruePath);
    if (TruePathLen > 1 && TruePath[TruePathLen - 1] == L'\\')
        TruePathLen--; // never take last \ into account

    for (int i=0; lists[i] != NULL; i++) {

        PATTERN* pat = List_Head(lists[i]);
        while (pat) {

            const WCHAR* patstr = Pattern_Source(pat);

            if (_wcsnicmp(TruePath, patstr, TruePathLen) == 0 && patstr[TruePathLen] == L'\\'){

                FoundReadable = TRUE;
                break;
            }

            pat = List_Next(pat);
        }
    }

    if (path_code == L'f')
        SbieDll_ReleaseFilePathLock();

    return FoundReadable;
}