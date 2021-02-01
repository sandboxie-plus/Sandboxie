/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC
 * Copyright 2020-2021 David Xanatos, xanasoft.com
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
 // Config
 //---------------------------------------------------------------------------

#define NOGDI
#include "dll.h"
#include "common/pool.h"
#include "common\pattern.h"

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------

extern POOL* Dll_Pool;
extern POOL* Dll_PoolTemp;

static BOOLEAN Config_MatchImageGroup(
    const WCHAR* group, ULONG group_len, const WCHAR* test_str,
    ULONG depth);

static BOOLEAN Config_MatchImage(
    const WCHAR* pat_str, ULONG pat_len, const WCHAR* test_str,
    ULONG depth);


//---------------------------------------------------------------------------
// Config_MatchImage
//---------------------------------------------------------------------------


_FX BOOLEAN Config_MatchImage(
    const WCHAR* pat_str, ULONG pat_len, const WCHAR* test_str,
    ULONG depth)
{
    PATTERN* pat;
    WCHAR* tmp;
    ULONG tmp_len;
    BOOLEAN ok;

    //
    // if pat_len was specified, we should create the match pattern
    // using only the first pat_len characters of pat_str
    //

    if (pat_len) {

        tmp_len = (pat_len + 1) * sizeof(WCHAR);
        tmp = Dll_AllocTemp(tmp_len);
        if (!tmp)
            return FALSE;

        wcsncpy(tmp, pat_str, pat_len);
        tmp[pat_len] = L'\0';

        pat = Pattern_Create(Dll_PoolTemp, tmp, TRUE);

        Dll_Free(tmp);

    }
    else {

        pat = Pattern_Create(Dll_PoolTemp, pat_str, TRUE);
    }

    if (!pat)
        return FALSE;

    //
    //
    //

    if (*pat_str == L'<') {

        ok = Config_MatchImageGroup(
            Pattern_Source(pat), 0, test_str, depth + 1);

        Pattern_Free(pat);

        return ok;
    }

    //
    // create a lower-case copy of test_str
    //

    ok = FALSE;

    tmp_len = (wcslen(test_str) + 1) * sizeof(WCHAR);
    tmp = Dll_AllocTemp(tmp_len);
    if (tmp) {

        memcpy(tmp, test_str, tmp_len);
        _wcslwr(tmp);

        ok = Pattern_Match(pat, tmp, wcslen(tmp));

        Dll_Free(tmp);
    }

    Pattern_Free(pat);

    return ok;
}


//---------------------------------------------------------------------------
// Config_MatchImageGroup
//---------------------------------------------------------------------------


_FX BOOLEAN Config_MatchImageGroup(
    const WCHAR* group, ULONG group_len, const WCHAR* test_str,
    ULONG depth)
{
    ULONG index;
    BOOLEAN match = FALSE;
    WCHAR conf_buf[2048];

    if (!group_len)
        group_len = wcslen(group);

    for (index = 0; (!match); ++index) {

        //
        // get next process group setting, compare to passed group name.
        // if the setting is <passed_group_name>= then we accept it.
        //

        NTSTATUS status = SbieApi_QueryConf(
            NULL, L"ProcessGroup", index, conf_buf, sizeof(conf_buf) - 16 * sizeof(WCHAR));
        if (!NT_SUCCESS(status))
            break;

        ULONG value_len;
        const WCHAR* value = conf_buf;

        value_len = wcslen(value);
        if (value_len <= group_len + 1)
            continue;
        if (_wcsnicmp(value, group, group_len) != 0)
            continue;

        value += group_len;
        if (*value != L',')
            continue;
        ++value;

        //
        // value now points at the comma-separated
        // list of processes in this process group
        //

        while (*value) {
            WCHAR* ptr = wcschr(value, L',');
            if (ptr)
                value_len = (ULONG)(ULONG_PTR)(ptr - value);
            else
                value_len = wcslen(value);

            if (value_len) {
                if (*value != L'<') {
                    match = Config_MatchImage(
                        value, value_len, test_str, depth + 1);
                }
                else if (depth < 6) {
                    match = Config_MatchImageGroup(
                        value, value_len, test_str, depth + 1);
                }
                if (match)
                    break;
            }

            value += value_len;
            while (*value == L',')
                ++value;
        }
    }

    return match;
}


//---------------------------------------------------------------------------
// Config_MatchImageAndGetValue
//---------------------------------------------------------------------------


_FX WCHAR* Config_MatchImageAndGetValue(WCHAR* value, ULONG* pMode)
{
    //
    // if the setting indicates an image name followed by a comma,
    // then match the image name against the executing process.
    //

    WCHAR* tmp = wcschr(value, L',');
    if (tmp) {

        BOOLEAN inv, match;

        //
        // exclamation marks negates the matching
        //

        if (*value == L'!') {
            inv = TRUE;
            ++value;
        }
        else
            inv = FALSE;

        if (pMode) *pMode = inv ? 1 : 0; // 1 - match by negation, 0 - exact match

        ULONG len = (ULONG)(tmp - value);
        if (len) {
            match = Config_MatchImage(value, len, Dll_ImageName, 1);
            if (inv)
                match = !match;
            if (!match)
                tmp = NULL;
        }

        value = tmp ? tmp + 1 : NULL;
    }
    else {

        if (pMode) *pMode = 2; // 2 - global default
    }

    return value;
}


//---------------------------------------------------------------------------
// Config_GetSettingsForImageName_bool
//---------------------------------------------------------------------------


BOOLEAN Config_GetSettingsForImageName_bool(const WCHAR* setting, BOOLEAN defval)
{
    WCHAR value[16];
    Config_GetSettingsForImageName(setting, value, sizeof(value), NULL);
    if (*value == L'y' || *value == L'Y')
        return TRUE;
    if (*value == L'n' || *value == L'N')
        return FALSE;
    return defval;
}


//---------------------------------------------------------------------------
// Config_InitPatternList
//---------------------------------------------------------------------------


_FX BOOLEAN Config_InitPatternList(const WCHAR* setting, LIST* list)
{
    WCHAR conf_buf[2048];

    PATTERN* pat;

    ULONG index = 0;
    while (1) {

        NTSTATUS status = SbieApi_QueryConf(
            NULL, setting, index, conf_buf, sizeof(conf_buf) - 16 * sizeof(WCHAR));
        if (!NT_SUCCESS(status))
            break;
        ++index;

        WCHAR* value = Config_MatchImageAndGetValue(conf_buf, NULL);
        if (value)
        {
            pat = Pattern_Create(Dll_Pool, value, TRUE);

            List_Insert_After(list, NULL, pat);
        }
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Config_GetSettingsForImageName
//---------------------------------------------------------------------------


_FX NTSTATUS Config_GetSettingsForImageName(
    const WCHAR* setting, WCHAR* value, ULONG value_size, const WCHAR* deftext)
{
    WCHAR conf_buf[2048];
    ULONG found_mode = -1;

    ULONG index = 0;
    while (1) {

        NTSTATUS status = SbieApi_QueryConf(
            NULL, setting, index, conf_buf, sizeof(conf_buf) - 16 * sizeof(WCHAR));
        if (!NT_SUCCESS(status))
            break;
        ++index;

        ULONG mode = -1;
        WCHAR* found_value = Config_MatchImageAndGetValue(conf_buf, &mode);
        if (!found_value || mode > found_mode)
            continue;
        //if (found_value) {
        //    SbieApi_Log(2302, L"%S - %S [%S]", setting, Dll_ImageName, Dll_BoxName);
        //    break;
        //}
        wcscpy_s(value, value_size / sizeof(WCHAR), found_value);
        found_mode = mode;
    }

    if (found_mode == -1) {
        if (deftext) wcscpy_s(value, value_size / sizeof(WCHAR), deftext);
        else value[0] = L'\0';
    }

    return STATUS_SUCCESS;
}