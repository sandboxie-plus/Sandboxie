/* 
 * Copyright (c) 2020-2023, David Xanatos
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <winternl.h>

#include "helpers.h"
#include "dirent.h"

OSVERSIONINFOW g_osvi;

#pragma warning(disable : 4996)
void InitOsVersionInfo() 
{
	g_osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
	NTSTATUS(WINAPI * RtlGetVersion)(LPOSVERSIONINFOW);
	*(FARPROC*)&RtlGetVersion = GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlGetVersion");
	if (RtlGetVersion == NULL || !NT_SUCCESS(RtlGetVersion(&g_osvi)))
		GetVersionExW(&g_osvi);
}
		
std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> g_str_conv;

bool ListDir(std::wstring Path, std::vector<std::wstring>& Entries)
{
	if (Path.back() != L'\\')
		return false;

	//std::string narrow = g_str_conv.to_bytes(wide_utf16_source_string);
	//std::wstring wide = g_str_conv.from_bytes(narrow_utf8_source_string);
	std::string aPath = g_str_conv.to_bytes(Path);

	DIR* d = opendir (aPath.c_str());
	if (d == NULL)
		return false;

	dirent* e;
	while ((e = readdir(d)) != NULL) 
	{
		std::string aName = e->d_name;
		std::wstring Name = std::wstring(aName.begin(), aName.end());

		switch (e->d_type) 
		{
			case DT_DIR:
				if(Name.compare(L"..") == 0 || Name.compare(L".") == 0)
					continue;
				Entries.push_back(Path + Name + L"\\");
				break;
			default:
				Entries.push_back(Path + Name);
		}
	}
	closedir (d);
	return true;
}

std::wstring hexStr(unsigned char* data, int len)
{
	static constexpr wchar_t hexmap[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

	std::wstring s(len * 2, ' ');
	for (int i = 0; i < len; ++i) {
		s[2 * i] = hexmap[(data[i] & 0xF0) >> 4];
		s[2 * i + 1] = hexmap[data[i] & 0x0F];
	}
	return s;
}

bool FileExists(const wchar_t* path)
{
	if (GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES && GetLastError() == ERROR_FILE_NOT_FOUND)
		return false;
	return true;
}

void DbgPrint(const wchar_t* format, ...)
{
    va_list va_args;
    va_start(va_args, format);
    
    wchar_t tmp1[510];

    _vsnwprintf(tmp1, sizeof(tmp1), format, va_args);

    OutputDebugStringW(tmp1);

    va_end(va_args);
}