/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020 David Xanatos, xanasoft.com
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
// Handles ActivClient's acscmonitor.dll which crashes firefox in sandboxie.
//---------------------------------------------------------------------------

#include "dll.h"

//---------------------------------------------------------------------------
// Acscmonitor_LoadLibrary
//---------------------------------------------------------------------------

ULONG CALLBACK Acscmonitor_LoadLibrary(LPVOID lpParam)
{
    // Acscmonitor is a plugin to Firefox which create a thread on initialize.
    // Firefox has a habit of initializing the module right before it's about
    // to unload the DLL, which is causing the crash.
    // Our solution is to prevent the library from ever being removed by holding
    // a reference to it.
    LoadLibraryW(L"acscmonitor.dll");
    return 0;
}

//---------------------------------------------------------------------------
// Acscmonitor_Init
//---------------------------------------------------------------------------

_FX BOOLEAN Acscmonitor_Init(HMODULE hDll)
{
	HANDLE ThreadHandle = CreateThread(NULL, 0, Acscmonitor_LoadLibrary, (LPVOID)0, 0, NULL);
	if (ThreadHandle)
		CloseHandle(ThreadHandle); 
    return TRUE;
}
