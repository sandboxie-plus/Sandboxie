/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
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

// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "SboxHostDll.h"

using namespace std;

HANDLE  g_hInjectedGlobalNameObj = NULL;

BOOLEAN InitHook( HINSTANCE hSbieDll );

extern "C" __declspec(dllexport) void InjectDllMain(HINSTANCE hSbieDll, ULONG_PTR UnusedParameter)
{
    InitHook(hSbieDll);
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    BOOL bRet = TRUE;

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:

        WCHAR wszDLLPath[MAX_PATH + 1];
        if (GetModuleFileName(GetModuleHandle(NULL), wszDLLPath, MAX_PATH))
        {
            if (wcsstr(wszDLLPath, L"OfficeClickToRun.exe"))
            {
                // You can use this global name object to check if the dll loaded.
                g_hInjectedGlobalNameObj = CreateMutex(NULL, FALSE, SBOX_HOST_DLL_LOADED);

                bRet = TRUE;
            }
            else
            {
                // injected to wrong process.
                bRet = FALSE;
            }
        }
        break;

    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:

        if (g_hInjectedGlobalNameObj)
        {
            CloseHandle(g_hInjectedGlobalNameObj);
        }
        break;
    }
    return bRet;
}

