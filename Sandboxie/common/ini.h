/*
 * Copyright 2021-2025 David Xanatos, xanasoft.com
 *
 * This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 3 of the License, or (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CONFIG_INI_H
#define CONFIG_INI_H

#include <string>
#include <list>

// Note: we don't use maps in order to preserve the order of the ini file


class CIniFile
{
public:
    CIniFile();
    virtual ~CIniFile() {}

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

    NTSTATUS LoadIni(const WCHAR* IniPath);

	NTSTATUS GetValue(const WCHAR* section, const WCHAR* setting, std::wstring& value);

    NTSTATUS SetValue(const WCHAR* section, const WCHAR* setting, const WCHAR* value, ULONG value_len = -1);

    NTSTATUS AddValue(const WCHAR* section, const WCHAR* setting, const WCHAR* value, bool bInsert = false);

    NTSTATUS RemoveValue(const WCHAR* section, const WCHAR* setting, const WCHAR* value);

	NTSTATUS RemoveSection(const WCHAR* section);

    NTSTATUS SaveIni(const WCHAR* IniPath);

protected:
    void InitComment();
    SIniSection* GetSection(const WCHAR* section, bool bCanAdd);
    void ReadSection(WCHAR* &iniDataPtr, std::list<SIniEntry>& entries);
    void ReadEntry(WCHAR* line, WCHAR* end, std::list<SIniEntry>& entries);
    SIniSection* ReadHeader(WCHAR*& iniDataPtr);
    WCHAR* PrepLine(WCHAR* iniDataPtr, WCHAR* &line, WCHAR* &end);

    ULONG m_Encoding;
    std::list<SIniSection> m_Sections;
};


#endif // CONFIG_INI_H