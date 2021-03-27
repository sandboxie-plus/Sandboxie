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
#include "core/svc/SbieIniWire.h"

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------

extern POOL* Dll_Pool;
extern POOL* Dll_PoolTemp;

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


_FX WCHAR* Config_MatchImageAndGetValue(WCHAR* value, const WCHAR* ImageName, ULONG* pMode)
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

        ULONG len = (ULONG)(tmp - value);
        if (len) {
            match = Config_MatchImage(value, len, ImageName, 1);
            if (inv)
                match = !match;
            if (!match)
                tmp = NULL;
            else if (pMode) {
                if (len == 1 && *value == L'*')
                    *pMode = 2; // 2 - match all 
                else
                    *pMode = inv ? 1 : 0; // 1 - match by negation, 0 - exact match
            }
        }

        value = tmp ? tmp + 1 : NULL;
    }
    else {

        if (pMode) *pMode = 2; // 2 - global default
    }

    return value;
}


//---------------------------------------------------------------------------
// Config_String2Bool
//---------------------------------------------------------------------------


BOOLEAN Config_String2Bool(const WCHAR* value, BOOLEAN defval)
{
    if (*value == L'y' || *value == L'Y')
        return TRUE;
    if (*value == L'n' || *value == L'N')
        return FALSE;
    return defval;
}


//---------------------------------------------------------------------------
// Config_GetSettingsForImageName_bool
//---------------------------------------------------------------------------


BOOLEAN Config_GetSettingsForImageName_bool(const WCHAR* setting, BOOLEAN defval)
{
    WCHAR value[16];
    Config_GetSettingsForImageName(setting, value, sizeof(value), NULL);
    return Config_String2Bool(value, defval);
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

        WCHAR* value = Config_MatchImageAndGetValue(conf_buf, Dll_ImageName, NULL);
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
        WCHAR* found_value = Config_MatchImageAndGetValue(conf_buf, Dll_ImageName, &mode);
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


//---------------------------------------------------------------------------
// SbieDll_GetBorderColor
//---------------------------------------------------------------------------


BOOLEAN SbieDll_GetBorderColor(const WCHAR* box_name, COLORREF* color, BOOL* title, int* width)
{
#ifndef RGB
#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
#endif //RGB

    *color = RGB(255, 255, 0);
    if (title) *title = FALSE;
    if (width) *width = 6;

    NTSTATUS status;
    WCHAR str[32];
    status = SbieApi_QueryConfAsIs(box_name, L"BorderColor", 0, str, sizeof(str) - sizeof(WCHAR)); // BorderColor=#00ffff,ttl,6

    if (!NT_SUCCESS(status) || wcslen(str) < 7 || str[0] != L'#')
        return FALSE;

    WCHAR* ptr = str;
    WCHAR* tmp = wcschr(ptr, L',');
    if (tmp != NULL) *tmp = L'\0';

    WCHAR* endptr;
    *color = wcstol(ptr + 1, &endptr, 16);
    if (*endptr != L'\0') {
        *color = RGB(255, 255, 0);
        return FALSE;
    }

    if (tmp == NULL) return TRUE;
    ptr = tmp + 1;
    tmp = wcschr(ptr, L',');
    if (tmp != NULL) *tmp = L'\0';

    if (_wcsicmp(ptr, L"ttl") == 0)
    {
        if (title) *title = TRUE;
    }
    else if (_wcsicmp(ptr, L"off") == 0)
        return FALSE;

    if (tmp == NULL) return TRUE;
    ptr = tmp + 1;
    tmp = wcschr(ptr, L',');
    if (tmp != NULL) *tmp = L'\0';

    if (width) *width = _wtoi(ptr);

    return TRUE;
}


//---------------------------------------------------------------------------
// SbieDll_MatchImage
//---------------------------------------------------------------------------


BOOLEAN SbieDll_MatchImage_Impl(const WCHAR* pat_str, ULONG pat_len, const WCHAR* test_str, const WCHAR* BoxName, ULONG depth)
{
    if (*pat_str == L'<') {

        ULONG index;
        WCHAR buf[CONF_LINE_LEN];

        if (depth >= 6)
            return FALSE;

        for (index = 0; ; ++index) {

            //
            // get next process group setting, compare to passed group name.
            // if the setting is <passed_group_name>= then we accept it.
            //

            NTSTATUS status = SbieApi_QueryConfAsIs(
                BoxName, L"ProcessGroup", index, buf, CONF_LINE_LEN * sizeof(WCHAR));
            if (!NT_SUCCESS(status))
                break;
            WCHAR* value = buf;

            ULONG value_len = wcslen(value);
            if (value_len <= pat_len + 1)
                continue;
            if (_wcsnicmp(value, pat_str, pat_len) != 0)
                continue;

            value += pat_len;
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

                    if (SbieDll_MatchImage_Impl(value, value_len, test_str, BoxName, depth + 1))
                        return TRUE;
                }

                value += value_len;
                while (*value == L',')
                    ++value;
            }
        }

    }
    else {

        ULONG test_len = wcslen(test_str);
        if (test_len == pat_len)
            return (_wcsnicmp(test_str, pat_str, test_len) == 0);

    }

    return FALSE;
}


BOOLEAN SbieDll_MatchImage(const WCHAR* pat_str, const WCHAR* test_str, const WCHAR* BoxName)
{
    ULONG pat_len = wcslen(pat_str);
    return SbieDll_MatchImage_Impl(pat_str, pat_len, test_str, BoxName, 1);
}


//---------------------------------------------------------------------------
// SbieDll_GetStringForStringList
//---------------------------------------------------------------------------


SBIEDLL_EXPORT BOOLEAN SbieDll_GetStringForStringList(const WCHAR* string, const WCHAR* boxname, const WCHAR* setting, WCHAR* value, ULONG value_size)
{
    WCHAR buf[CONF_LINE_LEN];
    ULONG index = 0;
    while (1) {
        NTSTATUS status = SbieApi_QueryConfAsIs(boxname, setting, index, buf, 64 * sizeof(WCHAR));
        ++index;
        if (NT_SUCCESS(status)) {
            WCHAR* ptr = wcschr(buf, L',');
            if (ptr) *ptr = L'\0';
            if (_wcsicmp(buf, string) == 0) {
                if (ptr++)
                    wcscpy_s(value, value_size / sizeof(WCHAR), ptr);
                else
                    *value = L'\0';
                return TRUE;
            }
        }
        else if (status != STATUS_BUFFER_TOO_SMALL)
            break;
    }
    return FALSE;
}


//---------------------------------------------------------------------------
// SbieDll_CheckStringInList
//---------------------------------------------------------------------------


BOOLEAN SbieDll_CheckStringInList(const WCHAR* string, const WCHAR* boxname, const WCHAR* setting)
{
    WCHAR buf[66];
    ULONG index = 0;
    while (1) {
        NTSTATUS status = SbieApi_QueryConfAsIs(boxname, setting, index, buf, 64 * sizeof(WCHAR));
        ++index;
        if (NT_SUCCESS(status)) {
            if (_wcsicmp(buf, string) == 0) {
                return TRUE;
            }
        }
        else if (status != STATUS_BUFFER_TOO_SMALL)
            break;
    }
    return FALSE;
}


//---------------------------------------------------------------------------
// SbieDll_GetBoolForStringFromList
//---------------------------------------------------------------------------


/*SBIEDLL_EXPORT BOOLEAN SbieDll_GetBoolForStringFromList(const WCHAR* string, const WCHAR* boxname, const WCHAR* setting, BOOLEAN def_found, BOOLEAN not_found)
{
    WCHAR buf[32];
    if (SbieDll_GetStringForStringList(string, boxname, setting, buf, sizeof(buf)))
        return Config_String2Bool(buf, def_found);
    return not_found;
}*/


//---------------------------------------------------------------------------
// Config_GetTagValue
//---------------------------------------------------------------------------


WCHAR* Config_GetTagValue(WCHAR* str, WCHAR** value, ULONG* len, WCHAR sep)
{
    *value = NULL;
    *len = 0;

    // skip whitespace
    //while (*str == L' ' || *str == L'\t') str++;

    WCHAR* tmp;
    BOOLEAN alt;
    // check if tag contains a string in quotations
    if ((alt = (*str == L'\"')) || (*str == L'\''))
    {
        WCHAR* end = wcschr(str + 1, alt ? L'\"' : L'\'');
        if (!end)
            return NULL; // error invalid string
        *value = str + 1;
        *len = (ULONG)(end - *value);
        end++;
        tmp = wcschr(end, sep);
        if (!tmp) tmp = wcschr(end, L'\0');
    }
    // else just look for separator
    else
    {
        tmp = wcschr(str, sep);
        if (!tmp) tmp = wcschr(str, L'\0');
        *value = str;
        *len = (ULONG)(tmp - *value);
    }

    if (tmp && *tmp) tmp++; // skip separator
    return tmp;
}


//---------------------------------------------------------------------------
// Config_FindTagValue
//---------------------------------------------------------------------------


BOOLEAN Config_FindTagValue(WCHAR* string, const WCHAR* tag_name, WCHAR* value, ULONG value_size, const WCHAR* deftext, WCHAR sep)
{
    ULONG tag_len = wcslen(tag_name);
    WCHAR* temp = string;
    WCHAR* tmp;
    WCHAR* name;
    ULONG len;
    WCHAR* found_value;
    ULONG found_len;
    while(*temp)
    {
        tmp = wcschr(temp, L'=');
        if (!tmp)
            break;
        name = temp;
        len = (ULONG)(tmp - temp);

        temp = Config_GetTagValue(tmp + 1, &found_value, &found_len, sep);
     
        if (tag_len == len && _wcsnicmp(tag_name, name, len) == 0)
        {
            wcsncpy_s(value, value_size / sizeof(WCHAR), found_value, found_len);
            return TRUE;
        }
    }

    return FALSE;
}