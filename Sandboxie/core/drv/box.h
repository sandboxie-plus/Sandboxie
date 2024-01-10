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
// Box Management
//---------------------------------------------------------------------------


#ifndef _MY_BOX_H
#define _MY_BOX_H


#include "driver.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _BOX             BOX;


struct _BOX {

    // a sandbox is identified by three components:
    // 1.  the name of this box

    WCHAR name[BOXNAME_COUNT];
    ULONG name_len;                     // in bytes, including NULL

    // 2.  the user account that started this sandbox

    WCHAR *sid;
    ULONG sid_len;                      // in bytes, including NULL

    // 3.  Terminal Services session number

    ULONG session_id;

    // parameters for Conf_Expand when used for this box

    CONF_EXPAND_ARGS *expand_args;

    // a sandbox has four paths:
    // 1.  the file system entry point into the sandbox
    // default:  \??\C:\Sandbox\%SID%\BoxName

    WCHAR *file_path;
    ULONG file_path_len;                // in bytes, including NULL

    // when the file_path was redirected by a reparse point, we keep the original 

    WCHAR *file_raw_path;
    ULONG file_raw_path_len;            // in bytes, including NULL

    // 2.  the registry entry point into the sandbox
    // (but note that the Registry.dat file resides below file_path)
    // default:  HKEY_CURRENT_USER\Sandbox\BoxName

    WCHAR *key_path;
    ULONG key_path_len;                 // in bytes, including NULL

    // 3.  the based named objects directory for the sandbox
    // default:  \Sandbox\%SID%\Session_%SESSION%\BoxName

    WCHAR *ipc_path;
    ULONG ipc_path_len;                 // in bytes, including NULL

    // 4.  same as ipc path, but backslashes translated to underscores

    WCHAR *pipe_path;
    ULONG pipe_path_len;                // in bytes, including NULL

    WCHAR *spooler_directory;
    ULONG spooler_directory_len;

    WCHAR *system_temp_path;
    ULONG system_temp_path_len;

    WCHAR *user_temp_path;
    ULONG user_temp_path_len;
};


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


// Returns TRUE if specified name is valid as a box name.
// This does not necessarily mean such a box exists.

BOOLEAN Box_IsValidName(const WCHAR *name);

// Create a BOX by specified name.
// Other components are queried from the current process.
// Path components are built using the configuration, but only
// if init_paths is TRUE.  Otherwise they are set to NULL.

BOX *Box_Create(POOL *pool, const WCHAR *boxname, BOOLEAN init_paths);

BOX *Box_CreateEx(
    POOL *pool, const WCHAR *boxname,
    const WCHAR *sidstring, ULONG session_id,
    BOOLEAN init_paths);

// Create a BOX by duplicating all the information from a model box,
// into a new BOX allocated from the specified pool.

BOX *Box_Clone(POOL *pool, const BOX *model);

// Free the box

void Box_Free(BOX *box);

// Returns true if UNICODE_STRING uni represents a path inside the
// sandbox for component m where m is 'file', 'key' or 'ipc'

#define Box_IsBoxedPath(box,m,uni)  \
    Box_IsBoxedPath_Helper((uni), ((box)->m##_path), ((box)->m##_path_len))

int Box_NlsStrCmp(const WCHAR *s1, const WCHAR *s2, ULONG len);

BOOLEAN Box_IsBoxedPath_Helper(
    UNICODE_STRING *uni, const WCHAR *box_path, ULONG box_path_len);


//---------------------------------------------------------------------------


#endif // _MY_BOX_H
