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

#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

//#include "targetver.h"

#include <windows.h>

#include "defines.h"
#include "ini.h"
#include "bom.c"

CIniFile::CIniFile()
{
	m_Encoding = 0;
}

NTSTATUS CIniFile::LoadIni(const WCHAR* IniPath)
{
    m_Sections.clear();

    WCHAR* iniData = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    WCHAR* iniDataPtr;
    ULONG encoding;
    SIniSection* pSection;

    hFile = CreateFileW(
        IniPath, FILE_GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        if (err == ERROR_FILE_NOT_FOUND) {
            InitComment(); // set a ini header with a descriptive comment
            status = STATUS_SUCCESS; // the file does not exist that's ok
        } else
            status = STATUS_OPEN_FAILED;
        goto finish;
    }

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize) || fileSize.QuadPart >= 384*1024*1024) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto finish;
    }

    if (fileSize.QuadPart == 0) {
        status = STATUS_SUCCESS;
        goto finish; // nothing to do
    }

    iniData = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, (SIZE_T)(fileSize.QuadPart + 128));
    if (!iniData) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto finish;
    }

    DWORD bytesRead;
    if (!ReadFile(hFile, iniData, (DWORD)fileSize.QuadPart, &bytesRead, NULL) || bytesRead != (DWORD)fileSize.QuadPart) {
        status = STATUS_NOT_READ_FROM_COPY;
        goto finish;
    }

    iniDataPtr = iniData;

    // Decode the BOM if present and andance the iniDataPtr accordingly
    encoding = Read_BOM((UCHAR**)&iniDataPtr, &bytesRead);

    if (encoding == 1) { // UTF-8 Signature
        // decode utf8
        int ByteSize = MultiByteToWideChar(CP_UTF8, 0, (char*)iniDataPtr, bytesRead, NULL, 0) + 1;
        WCHAR* tmpData = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, ByteSize * sizeof(wchar_t));
        if (!tmpData) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto finish;
        }
        bytesRead = MultiByteToWideChar(CP_UTF8, 0, (char*)iniDataPtr, bytesRead, tmpData, ByteSize);
        // swap buffers
        HeapFree(GetProcessHeap(), 0, iniData);
        iniDataPtr = iniData = tmpData;
    }
    else {
        if (encoding == 2) { //Unicode (UTF-16 BE) BOM
            // swap all bytes
            UCHAR* tmpData = (UCHAR*)iniDataPtr;
            for (DWORD i = 0; i < bytesRead - 1; i += 2) {
                UCHAR tmp = tmpData[i + 1];
                tmpData[i + 1] = tmpData[i];
                tmpData[i] = tmp;
            }
        }
        //else if (encoding == 0) //Unicode (UTF-16 LE) BOM
        bytesRead /= sizeof(wchar_t);
    }

    iniDataPtr[bytesRead] = L'\0';

    m_Encoding = encoding;

    m_Sections.push_back(SIniSection{});
    pSection = &m_Sections.back();
    while(*iniDataPtr != L'\0' && pSection != NULL)
    {
        ReadSection(iniDataPtr, pSection->Entries);
        if (*iniDataPtr == L'\0')
            break;

        pSection = ReadHeader(iniDataPtr);
    }

    status = STATUS_SUCCESS;

finish:
    if(iniData != NULL)
        HeapFree(GetProcessHeap(), 0, iniData);

    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);

    return status;
}

void CIniFile::InitComment()
{
    m_Sections.push_back(SIniSection{ L"" });
    m_Sections.back().Entries.push_back(SIniEntry{ L"", L"#" });
    m_Sections.back().Entries.push_back(SIniEntry{ L"", L"# Sandboxie configuration file" });
    m_Sections.back().Entries.push_back(SIniEntry{ L"", L"#" });

    m_Sections.push_back(SIniSection{ L"GlobalSettings" });
}

NTSTATUS CIniFile::GetValue(const WCHAR* section, const WCHAR* setting, std::wstring& value)
{
    SIniSection* pSection = GetSection(section, false);
    if (!pSection)
        return STATUS_OBJECT_NAME_NOT_FOUND;

    for (auto I = pSection->Entries.begin(); I != pSection->Entries.end(); ++I)
    {
        if (*setting == L'\0') { // get section
            if(I->Name.size() > 0)
                value += I->Name + L"=";
            value += I->Value + L"\r\n";
        }
        else if (_wcsicmp(I->Name.c_str(), setting) == 0) {
            if(!value.empty()) // string list
                //iniData.push_back(L'\0');
                value.push_back(L'\n');
            value += I->Value;
        }
    }
    return STATUS_SUCCESS;
}

NTSTATUS CIniFile::SetValue(const WCHAR* section, const WCHAR* setting, const WCHAR* value, ULONG value_len)
{
	if (value_len == -1)
		value_len = (ULONG)wcslen(value);
    BOOLEAN have_value = (value_len != 0);

    SIniSection* pSection = GetSection(section, true);

    //
    // Check if this is a replace section request and if so execute it
    //

    if (wcslen(setting) == 0 && have_value) 
    {
        std::list<SIniEntry> entries;

        WCHAR* iniDataPtr = (WCHAR*)value;
		ReadSection(iniDataPtr, entries);
        if (*iniDataPtr != L'\0') // there must be no sections inside another section
            return STATUS_INVALID_PARAMETER;

        pSection->Entries = entries;
        return STATUS_SUCCESS;
    }

    //
    // remove old values and set the new once
    //

    std::list<SIniEntry>::iterator pos = pSection->Entries.end();
    for (auto I = pSection->Entries.begin(); I != pSection->Entries.end();)
    {
        if (_wcsicmp(I->Name.c_str(), setting) == 0) {
            I = pSection->Entries.erase(I);
            pos = I;
        }
        else
            ++I;
    }

    //
    // set the value(s) if present
    //

    if (have_value) 
    {
        /*for (WCHAR* value = value; value_len > 0 && *value != L'\0';)
        {
            pSection->Entries.insert(pos, SIniEntry{ setting, value });

            ULONG len = wcslen(value);
            value_len -= len;
            if (value_len > 0) {
                value_len -= 1;
                value += len + 1;
            }
        }*/

        //
        // Note: SbieCtrl passes a \n separated list to replace all values in a string list
        //

        for (const WCHAR* ptr = value; *ptr != L'\0'; ) 
        {
            ULONG cpylen, skiplen;
            const WCHAR *cr = wcschr(ptr, L'\n');
            if (cr) {
                cpylen = (ULONG)(cr - ptr);
                skiplen = cpylen + 1;
            } else {
                cpylen = wcslen(ptr);
                skiplen = cpylen;
            }
            if (cpylen > CONF_LINE_LEN)
                cpylen = CONF_LINE_LEN;

            pSection->Entries.insert(pos, SIniEntry{ setting, std::wstring(ptr, cpylen) });

            ptr += skiplen;
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS CIniFile::AddValue(const WCHAR* section, const WCHAR* setting, const WCHAR* value, bool bInsert)
{
    //
    // Get the relevant ini section object
    //

    SIniSection* pSection = GetSection(section, true);

    //
    // Find the right place to add the value
    //

    std::list<SIniEntry>::iterator pos = pSection->Entries.end();
    for (auto I = pSection->Entries.begin(); I != pSection->Entries.end();++I)
    {
        if (_wcsicmp(I->Name.c_str(), setting) == 0) {
            // !insert -> append -> find last entry
            if (!bInsert || pos == pSection->Entries.end()) {
                pos = I;
                if (!bInsert) pos++;
            }
            //if (_wcsicmp(I->Value.c_str(), value) == 0) {
            //    // this value is already present, so let's abort right here
            //    return STATUS_SUCCESS;
            //}
        }
    }

    //
    // add the value to the string list
    //

    pSection->Entries.insert(pos, SIniEntry{ setting, value });

    return STATUS_SUCCESS;
}

NTSTATUS CIniFile::RemoveValue(const WCHAR* section, const WCHAR* setting, const WCHAR* value)
{
    //
    // Get the relevant ini section object
    //

    SIniSection* pSection = GetSection(section, false);
    if (!pSection)
        return STATUS_SUCCESS;

    //
    // discard setting with the matching the value
    //

    for (auto I = pSection->Entries.begin(); I != pSection->Entries.end();)
    {
        if (_wcsicmp(I->Name.c_str(), setting) == 0 && (!value || !*value || _wcsicmp(I->Value.c_str(), value) == 0)) {
            I = pSection->Entries.erase(I);
            // Note: we could break here, but let's finish in case there is a duplicate
        }
        else
            ++I;
    }

    return STATUS_SUCCESS;
}

NTSTATUS CIniFile::RemoveSection(const WCHAR* section)
{
    for (auto I = m_Sections.begin(); I != m_Sections.end(); ++I)
    {
        if (_wcsicmp(I->Name.c_str(), section) == 0) {
            m_Sections.erase(I);
            break;
        }
    }
    return STATUS_SUCCESS;
}

CIniFile::SIniSection* CIniFile::GetSection(const WCHAR* section, bool bCanAdd)
{
    SIniSection* pSection = NULL;
    for (auto I = m_Sections.begin(); I != m_Sections.end(); ++I)
    {
        if (_wcsicmp(I->Name.c_str(), section) == 0) {
            pSection = &(*I);
            break;
        }
    }

    if (!pSection && bCanAdd) {
        m_Sections.push_back(SIniSection{section});
        pSection = &m_Sections.back();
    }
    return pSection;
}


WCHAR* CIniFile::PrepLine(WCHAR* iniDataPtr, WCHAR* &line, WCHAR* &end)
{
    line = iniDataPtr;        
    // trim leading whitespaces
    while (*line == L' ' || *line == L'\t' || *line == L'\r') line++; 

    // find the line ending
    end = wcschr(line, L'\n');
    // it is the last line if there is no line ending, so find the string terminator
    if (end == NULL) iniDataPtr = end = wcschr(line, L'\0');
    else iniDataPtr = end + 1;

    // trim trailing whitespaces
    while (end > line && (*(end-1) == L' ' || *(end-1) == L'\t' || *(end-1) == L'\r')) end--; 

    // return pointer to next line
    return iniDataPtr;
}

void CIniFile::ReadEntry(WCHAR* line, WCHAR* end, std::list<SIniEntry>& entries)
{
    WCHAR* separator;
    if (*line == L'#') 
        goto no_name;
    separator = wcschr(line, L'=');
    if (separator > end || separator == NULL)
    {
    no_name:
        entries.push_back(SIniEntry{std::wstring(), std::wstring(line, end - line)});
    }
    else 
    {
        WCHAR* name_start = line;
        WCHAR* name_end = separator;
        // trim trailing whitespaces
        while (name_end > name_start && (*(name_end-1) == L' ' || *(name_end-1) == L'\t')) name_end--; 

        WCHAR* value_start = separator+1;
        // trim leading whitespaces
        while (*value_start == L' ' || *value_start == L'\t') value_start++; 
        WCHAR* value_end = end;

        entries.push_back(SIniEntry{std::wstring(name_start, name_end - name_start), std::wstring(value_start, value_end - value_start)});
    }
}

void CIniFile::ReadSection(WCHAR* &iniDataPtr, std::list<SIniEntry>& entries)
{
    for (WCHAR* next = NULL;  *iniDataPtr != L'\0'; iniDataPtr = next)
    {
        WCHAR *line, *end;
        next = PrepLine(iniDataPtr, line, end);

        if (*line == L'[')
            break;

        ReadEntry(line, end, entries);
    }
    
    while (entries.size() > 0 && entries.back().Name.size() == 0 && entries.back().Value.size() == 0)
        entries.pop_back(); // drop empty lines at the very end of a section
}

CIniFile::SIniSection* CIniFile::ReadHeader(WCHAR*& iniDataPtr)
{
    WCHAR *line, *end;
    iniDataPtr = PrepLine(iniDataPtr, line, end);

    if (*line == L'[') // section
    {
        WCHAR* start = line + 1;
        WCHAR* stop = wcschr(line, L']');
        if (stop == NULL || stop > end)
            stop = end;
        m_Sections.push_back(SIniSection{std::wstring(start, stop - start)});
        return &m_Sections.back();
    }
    return NULL;
}

NTSTATUS CIniFile::SaveIni(const WCHAR* IniPath)
{
    NTSTATUS status;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    std::wstring iniData;

    int retryCnt = 0;
retry:
    hFile = CreateFileW(
        IniPath, FILE_GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_SHARING_VIOLATION && retryCnt++ < 10) {
            Sleep(100);
            goto retry;
        }
        return STATUS_OPEN_FAILED;
    }

    //
    // rebuild the ini from the cache with new values, if present, 
    // and keeping comments and most of the formatting
    //

    for (auto I = m_Sections.begin(); I != m_Sections.end(); ++I)
    {
        if (I->Name.size() > 0)
            iniData += L"[" + I->Name + L"]\r\n";

        for (auto J = I->Entries.begin(); J != I->Entries.end(); ++J)
        {
            if (J->Value.size() > 0) {
                if (J->Name.size() > 0)
                    iniData += J->Name + L"=";
                iniData += J->Value;
            }
            iniData += L"\r\n";
        }
        iniData += L"\r\n";
    }

    ULONG lenWritten = 0;
	if (m_Encoding == 1) { // UTF-8 Signature EF BB BF
        static const UCHAR bom[3] = { 0xEF, 0xBB, 0xBF };
        WriteFile(hFile, bom, sizeof(bom), &lenWritten, NULL);
    }
	else { // UNICODE Byte Order Mark (little endian) FF FE
        static const UCHAR bom[2] = { 0xFF, 0xFE };
        WriteFile(hFile, bom, sizeof(bom), &lenWritten, NULL);
    }

    ULONG lenToWrite = wcslen(iniData.c_str()) * sizeof(WCHAR);

    char* text_utf8 = NULL;
    if (m_Encoding == 1)
    {
        ULONG utf8_len = WideCharToMultiByte(CP_UTF8, 0, iniData.c_str(), lenToWrite / sizeof(WCHAR), NULL, 0, NULL, NULL);
        text_utf8 = (char*)HeapAlloc(GetProcessHeap(), 0, utf8_len);
        lenToWrite = WideCharToMultiByte(CP_UTF8, 0, iniData.c_str(), lenToWrite / sizeof(WCHAR), text_utf8, utf8_len, NULL, NULL);
    }

    if (! WriteFile(hFile, text_utf8 ? (void*)text_utf8 : (void*)iniData.c_str(), lenToWrite, &lenWritten, NULL))
        lenWritten = -1;

    if(text_utf8)
        HeapFree(GetProcessHeap(), 0, text_utf8);

    if (lenWritten != lenToWrite)
        status = STATUS_UNEXPECTED_IO_ERROR;
    else if (! SetEndOfFile(hFile))
        status = STATUS_INVALID_OFFSET_ALIGNMENT;
    else
        status = STATUS_SUCCESS;

    CloseHandle(hFile);

    return status;
}