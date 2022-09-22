/*
 * Copyright 2020 DavidXanatos, xanasoft.com
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

#include <windows.h>
#include "win32_ntddk.h"

//
// Do not include any external CRT into sboxdll (SbieDll.dll) project !!!
// This DLL is injected early into the process start up sequence and adding dependencies may break Sandboxie. 
// Normally we link directly to the CRT build into ntdll.dll.
//
// As more recent versions of ntdll.lib are not offering many CRT functions we have to create own libs.
// See NtCRT_x64.def and NtCRT_x86.def respectively for the functions, we use a custom build step 
// "lib /def:%(FullPath) /out:$(SolutionDir)Bin\$(PlatformName)\$(Configuration)\NtCRT.lib /machine:x64" 
// on the appropriate def file, for x86 we use the switch /machine:x86 instead.
//
// For the x86 build we also need _except_handler3 which we have recreated in except_handler3.asm
//
// Alternatively we could link all the required functions dynamically from InitMyNtDll
//

int(*P_vsnwprintf)(wchar_t *_Buffer, size_t Count, const wchar_t * const, va_list Args) = NULL;
int(*P_vsnprintf)(char *_Buffer, size_t Count, const char * const, va_list Args) = NULL;

void InitMyNtDll(HMODULE Ntdll)
{
	*(FARPROC*)&P_vsnwprintf = GetProcAddress(Ntdll, "_vsnwprintf");
	*(FARPROC*)&P_vsnprintf = GetProcAddress(Ntdll, "_vsnprintf");
}
