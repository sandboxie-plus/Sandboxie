/*
 * Copyright 2021 David Xanatos, xanasoft.com
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
// Syscall Management
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Syscall_LoadHookMap
//---------------------------------------------------------------------------


_FX BOOLEAN Syscall_LoadHookMap(const WCHAR* setting_name, LIST *list)
{
    ULONG index;
    const WCHAR *value;
    PATTERN* pat;

    const WCHAR* _SysCallPresets = L"SysCallPresets";

    List_Init(list);

    Conf_AdjustUseCount(TRUE);

    for (index = 0; ; ++index) {

        //
        // get next configuration setting for this path list
        //

        value = Conf_Get(_SysCallPresets, setting_name, index);
        if (! value)
            break;

        //
        // create pattern and add to list
        //

        pat = Pattern_Create(Driver_Pool, value, FALSE, 0);
        if (pat) {
            List_Insert_After(list, NULL, pat);
        }
    }

    Conf_AdjustUseCount(FALSE);

    return TRUE;
}


//---------------------------------------------------------------------------
// Syscall_HookMapMatch
//---------------------------------------------------------------------------


_FX int Syscall_HookMapMatch(const UCHAR *name, ULONG name_len, LIST *list)
{
    PATTERN* pat;
    int match_len = 0;

    WCHAR wname[68];
    ULONG i;
    for (i = 0; i < max(name_len, 64); i++)
        wname[i] = name[i];
    wname[i] = 0;

    pat = List_Head(list);
    while (pat) {

        int cur_len = Pattern_MatchX(pat, wname, name_len);
        if (cur_len > match_len) {
            match_len = cur_len;
        }

        pat = List_Next(pat);
    }

    return match_len;
}


//---------------------------------------------------------------------------
// Syscall_TestHookMap
//---------------------------------------------------------------------------


_FX BOOLEAN Syscall_TestHookMap(const UCHAR* name, ULONG name_len, LIST* enabled_hooks, LIST* disabled_hooks, BOOLEAN default_action)
{
    if(disabled_hooks->count == 0 && enabled_hooks->count == 0)
        return default_action;

    int disabe_match = Syscall_HookMapMatch(name, name_len, disabled_hooks);
    int enable_match = Syscall_HookMapMatch(name, name_len, enabled_hooks);
    if (disabe_match != 0 && disabe_match >= enable_match)
        return FALSE;
    if (enable_match != 0)
        return TRUE;
    return default_action;
}


//---------------------------------------------------------------------------
// Syscall_FreeHookMap
//---------------------------------------------------------------------------


_FX VOID Syscall_FreeHookMap(LIST *list)
{
    PATTERN* pat;
    while (1) {
        pat = List_Head(list);
        if (! pat)
            break;
        List_Remove(list, pat);
        Pattern_Free(pat);
    }
}
