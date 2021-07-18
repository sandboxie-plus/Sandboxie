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

#include "common/str_util.h"
#include "common/defines.h"


//---------------------------------------------------------------------------
// Config_SkipWhiteSpace
//---------------------------------------------------------------------------


VOID Config_SkipWhiteSpace(const WCHAR** pstr) {
    while ((*pstr)[0] == L' ' 
        || (*pstr)[0] == L'\r' 
        || (*pstr)[0] == L'\n' 
        || (*pstr)[0] == L'\t')
        (*pstr)++;
}


//---------------------------------------------------------------------------
// Config_TrimWhiteSpace
//---------------------------------------------------------------------------


VOID Config_TrimWhiteSpace(const WCHAR** pstr, ULONG* len) {
    while (*len > 0 
        && ((*pstr)[*len-1] == L' ' 
        || (*pstr)[*len-1] == L'\r'
        //|| (*pstr)[*len-1] == L'\n'
        || (*pstr)[*len-1] == L'\t')) 
        (*len)--;
}


//---------------------------------------------------------------------------
// SbieDll_GetTagValue
//---------------------------------------------------------------------------


const WCHAR* SbieDll_GetTagValue(const WCHAR* str, const WCHAR* strEnd, const WCHAR** value, ULONG* len, WCHAR sep)
{
    *value = NULL;
    *len = 0;

    Config_SkipWhiteSpace(&str);

    const WCHAR* tmp;
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
        if (strEnd) {
            if (!tmp || tmp > strEnd) tmp = strEnd;
        }
        else if (!tmp) tmp = wcschr(end, L'\0');
    }
    // else just look for separator
    else
    {
        tmp = wcschr(str, sep);
        if (strEnd) { 
            if (!tmp || tmp > strEnd) tmp = strEnd;
        }
        else if (!tmp) tmp = wcschr(str, L'\0');
        *value = str;
        *len = (ULONG)(tmp - *value);

        Config_TrimWhiteSpace(value, len);
    }

    if (tmp && *tmp && (!strEnd || tmp < strEnd)) tmp++; // skip separator
    return tmp;
}


//---------------------------------------------------------------------------
// SbieDll_EnumTagValues
//---------------------------------------------------------------------------


VOID SbieDll_EnumTagValues(const WCHAR* string, SbieDll_TagEnumProc enumProc, void* param, WCHAR eq, WCHAR sep)
{
    const WCHAR* str_ptr = string;
    const WCHAR* tmp;
    const WCHAR* name;
    ULONG len;
    const WCHAR* found_value;
    ULONG found_len;
    while(*str_ptr)
    {
        Config_SkipWhiteSpace(&str_ptr);

        tmp = wcschr(str_ptr, eq);
        if (!tmp)
            break;
        name = str_ptr;
        len = (ULONG)(tmp - str_ptr);
        Config_TrimWhiteSpace(&name, &len);

        str_ptr = SbieDll_GetTagValue(tmp + 1, NULL, &found_value, &found_len, sep);
     
        if (!str_ptr || !enumProc(name, len, found_value, found_len, param))
            break;
    }
}


//---------------------------------------------------------------------------
// SbieDll_FindTagValue
//---------------------------------------------------------------------------

typedef struct
{
    const WCHAR* tag_name;
    ULONG tag_len;
    const WCHAR* found_value;
    ULONG found_len;
} TagFindProcParam;

static BOOLEAN Config_TagFindProc(const WCHAR* name, ULONG name_len, const WCHAR* value, ULONG value_len, void* param)
{
    TagFindProcParam* tagFindProcParam = (TagFindProcParam*)param;
    if (tagFindProcParam->tag_len == name_len && _wcsnicmp(tagFindProcParam->tag_name, name, name_len) == 0)
    {
        tagFindProcParam->found_value = value;
        tagFindProcParam->found_len = value_len;
        return FALSE; // break
    }
    return TRUE; // continue
}

BOOLEAN SbieDll_FindTagValuePtr(const WCHAR* string, const WCHAR* tag_name, const WCHAR** value, ULONG* value_len, WCHAR eq, WCHAR sep)
{
    if (!string)
        return FALSE;

    TagFindProcParam tagFindProcParam = 
    {
        tag_name,
        wcslen(tag_name),
        NULL,
        0
    };

    SbieDll_EnumTagValues(string, &Config_TagFindProc, &tagFindProcParam, eq, sep);
    if (!tagFindProcParam.found_value)
        return FALSE;
    *value = tagFindProcParam.found_value;
    *value_len = tagFindProcParam.found_len;
    return TRUE;
}

BOOLEAN SbieDll_FindTagValue(const WCHAR* string, const WCHAR* tag_name, WCHAR* value, ULONG value_size, WCHAR eq, WCHAR sep)
{
    WCHAR* value_ptr;
    ULONG value_len;
    if (!SbieDll_FindTagValuePtr(string, tag_name, &value_ptr, &value_len, eq, sep))
        return FALSE;
    wcsncpy_s(value, value_size / sizeof(WCHAR), value_ptr, value_len);
    return TRUE;
}