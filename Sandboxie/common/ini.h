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

#ifndef CONFIG_INI_H
#define CONFIG_INI_H

#include <string>
#include <list>

// Note: we don't use maps in order to preserve the order of the ini file

struct SIniEntry
{
    std::wstring Name;
    std::wstring Value;
};

struct SIniSection
{
    std::wstring Name;
    std::list<SIniEntry> Entries;
};

struct SConfigIni
{
    ULONG Encoding;
    std::list<SIniSection> Sections;
};


WCHAR* Ini_Prep_ConfigLine(WCHAR* iniDataPtr, WCHAR*& line, WCHAR*& end);
void Ini_Read_ConfigEntry(WCHAR* line, WCHAR* end, std::list<SIniEntry>& entries);
void Ini_Read_ConfigSection(WCHAR*& iniDataPtr, std::list<SIniEntry>& entries);
SIniSection* Ini_Read_SectionHeader(WCHAR*& iniDataPtr, SConfigIni* pIniConfig);

#endif // CONFIG_INI_H