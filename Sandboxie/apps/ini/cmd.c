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
// SbieIni scripting utility
//---------------------------------------------------------------------------


#include "global.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define CMD_MAX_ARGS 99


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


WCHAR **CmdVerbs;


//---------------------------------------------------------------------------
// CmdEatString
//---------------------------------------------------------------------------


WCHAR *CmdEatString(WCHAR *str)
{
    BOOL quoted = FALSE;

    while (*str == L' ')
        ++str;

    while (*str) {
        if (*str == L'\"')
            quoted = !quoted;
        else if (*str == L' ' && (! quoted))
            break;
        ++str;
    }

    while (*str == L' ')
        ++str;

    return str;
}


//---------------------------------------------------------------------------
// CmdCopyString
//---------------------------------------------------------------------------


WCHAR *CmdCopyString(WCHAR *ptr, WCHAR *ptr2)
{
    WCHAR *copy;
    ULONG len;
    while (*ptr == L'\"' || *ptr == L' ')
        ++ptr;
    while ((ptr2 > ptr) && (ptr2[-1] == L'\"' || ptr2[-1] == L' '))
        --ptr2;
    len = (ULONG)(ULONG_PTR)(ptr2 - ptr);
    copy = HeapAlloc(GetProcessHeap(), 0, (len + 1) * sizeof(WCHAR));
    if (copy) {
        memcpy(copy, ptr, len * sizeof(WCHAR));
        copy[len] = L'\0';
    }
    return copy;
}


//---------------------------------------------------------------------------
// CmdParse
//---------------------------------------------------------------------------


void CmdParse(void)
{
    WCHAR **CmdOpts;
    WCHAR *ptr, *ptr2, *copy;
    ULONG len;
    ULONG num_verbs, num_opts;

    len = CMD_MAX_ARGS * 2 * sizeof(ULONG_PTR);
    CmdVerbs = HeapAlloc(GetProcessHeap(), 0, len);
    if (! CmdVerbs)
        return;
    CmdOpts = CmdVerbs + CMD_MAX_ARGS;
    num_verbs = num_opts = 0;

    ptr = GetCommandLine();
    ptr = CmdEatString(ptr);        // skip image name first argument

    while (*ptr) {

        ptr2 = CmdEatString(ptr);
        copy = CmdCopyString(ptr, ptr2);
        if (! copy) {
            CmdVerbs = NULL;
            return;
        }

        if (*copy == L'/') {

            WCHAR *ptr_co = wcschr(copy, L':');
            WCHAR *ptr_eq = wcschr(copy, L'=');
            if (ptr_eq) {
                if ((! ptr_co) || (ptr_co > ptr_eq))
                    ptr_co = ptr_eq;
            }
            if (ptr_co) {
                *ptr_co = L'\0';
                ++ptr_co;
                ptr_eq = CmdEatString(ptr_co);
                ptr_co = CmdCopyString(ptr_co, ptr_eq);
            }
            if (! ptr_co)
                ptr_co = L"";

            CmdOpts[num_opts] = copy + 1;
            ++num_opts;
            CmdOpts[num_opts] = ptr_co;
            ++num_opts;

        } else if (*copy) {

            CmdVerbs[num_verbs] = copy;
            ++num_verbs;
        }

        ptr = ptr2;
    }

    CmdVerbs[num_verbs] = NULL;
    CmdOpts[num_opts] = NULL;
}


//---------------------------------------------------------------------------
// CmdIs
//---------------------------------------------------------------------------


BOOL CmdIs(const WCHAR *val)
{
    if (CmdVerbs && CmdVerbs[0]) {
        if (_wcsicmp(CmdVerbs[0], val) == 0)
            return TRUE;
    }
    return FALSE;
}


//---------------------------------------------------------------------------
// CmdVerb
//---------------------------------------------------------------------------


WCHAR *CmdVerb(ULONG index)
{
    if (CmdVerbs) {
        ULONG index2 = 0;
        while (index && CmdVerbs[index2]) {
            ++index2;
            --index;
        }
        return CmdVerbs[index2];
    }
    return NULL;
}


//---------------------------------------------------------------------------
// CmdOpt
//---------------------------------------------------------------------------


WCHAR *CmdOpt(const WCHAR *name)
{
    if (CmdVerbs) {
        WCHAR **CmdOpts = CmdVerbs + CMD_MAX_ARGS;
        while (*CmdOpts) {
            if (_wcsicmp(*CmdOpts, name) == 0)
                return CmdOpts[1];
            CmdOpts += 2;
        }
    }
    return NULL;
}
