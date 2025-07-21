/*
 * Copyright 2022 David Xanatos, xanasoft.com
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


#ifndef DLLIMPORT_H
#define DLLIMPORT_H
#pragma once

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

DWORD64 FindDllBase64(HANDLE hProcess, const WCHAR* dll);
BYTE* MapRemoteDll(HANDLE hProcess, DWORD64 DllBase);
DWORD64 FindDllExportInMem(DWORD64 DllBase, const char* ProcName);
DWORD64 FindRemoteDllExport(HANDLE hProcess, DWORD64 DllBase, const char* ProcName);
DWORD64 FindDllExportFromFile(const WCHAR* dll, const char* ProcName);

#ifdef __cplusplus
}
#endif

#endif /* DLLIMPORT_H */
