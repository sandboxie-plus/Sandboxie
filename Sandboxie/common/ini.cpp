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

#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

//#include "targetver.h"

#include <windows.h>

#include "ini.h"


WCHAR* Ini_Prep_ConfigLine(WCHAR* iniDataPtr, WCHAR* &line, WCHAR* &end)
{
    line = iniDataPtr;        
    // trim leading whitespaces
    while (*line == L' ' || *line == L'\t' || *line == L'\r') line++; 
    // find the line ending
    end = wcschr(line, L'\n');
    // if there is no line ending its teh last line so fine the sring termiantor
    if (end == NULL) iniDataPtr = end = wcschr(line, L'\0');
    else iniDataPtr = end + 1;
    // trim tailing whitespaces
    while (end > line && (*(end-1) == L' ' || *(end-1) == L'\t' || *(end-1) == L'\r')) end--; 
    // return pointer to next line
    return iniDataPtr;
}

void Ini_Read_ConfigEntry(WCHAR* line, WCHAR* end, std::list<SIniEntry>& entries)
{
    if (*line == L'#') goto no_name;
    WCHAR* separator = wcschr(line, L'=');
    if (separator > end || separator == NULL)
    {
    no_name:
        entries.push_back(SIniEntry{std::wstring(), std::wstring(line, end - line)});
    }
    else 
    {
        WCHAR* name_start = line;
        WCHAR* name_end = separator;
        // trim tailing whitespaces
        while (name_end > name_start && (*(name_end-1) == L' ' || *(name_end-1) == L'\t')) name_end--; 

        WCHAR* value_start = separator+1;
        // trim leading whitespaces
        while (*value_start == L' ' || *value_start == L'\t') value_start++; 
        WCHAR* value_end = end;

        entries.push_back(SIniEntry{std::wstring(name_start, name_end - name_start), std::wstring(value_start, value_end - value_start)});
    }
}

void Ini_Read_ConfigSection(WCHAR* &iniDataPtr, std::list<SIniEntry>& entries)
{
    for (WCHAR* next = NULL;  *iniDataPtr != L'\0'; iniDataPtr = next)
    {
        WCHAR *line, *end;
        next = Ini_Prep_ConfigLine(iniDataPtr, line, end);

        if (*line == L'[')
            break;

        Ini_Read_ConfigEntry(line, end, entries);
    }
    
    while (entries.size() > 0 && entries.back().Name.size() == 0 && entries.back().Value.size() == 0)
        entries.pop_back(); // drop empty lines at the very end of a section
}

SIniSection* Ini_Read_SectionHeader(WCHAR*& iniDataPtr, SConfigIni* pIniConfig)
{
    WCHAR *line, *end;
    iniDataPtr = Ini_Prep_ConfigLine(iniDataPtr, line, end);

    if (*line == L'[') // section
    {
        WCHAR* start = line + 1;
        WCHAR* stop = wcschr(line, L']');
        if (stop == NULL || stop > end)
            stop = end;
        pIniConfig->Sections.push_back(SIniSection{std::wstring(start, stop - start)});
        return &pIniConfig->Sections.back();
    }
    return NULL;
}