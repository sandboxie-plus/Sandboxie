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

//---------------------------------------------------------------------------
// SandboxieInstall.exe Installation Package
//---------------------------------------------------------------------------

#include <windows.h>

//---------------------------------------------------------------------------
// GetResourceId
//---------------------------------------------------------------------------

int GetResourceId(void)
{
    typedef BOOL (*P_IsWow64Process)(HANDLE, BOOL *);

    HMODULE kernel32 = LoadLibrary(L"kernel32.dll");

    P_IsWow64Process __sys_IsWow64Process =
        (P_IsWow64Process)GetProcAddress(kernel32, "IsWow64Process");

    if (__sys_IsWow64Process) {

        BOOL x64;
        if (! __sys_IsWow64Process(GetCurrentProcess(), &x64))
            x64 = FALSE;
        if (x64)
            return 6464;
    }

    return 3232;
}

//---------------------------------------------------------------------------
// WinMain
//---------------------------------------------------------------------------

int __stdcall WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    UINT exitcode;

    int rsid;
    HRSRC hrsrc;
    ULONG exesize;
    HGLOBAL hglob;
    void *exedata;
    WCHAR path[800], *name;
    HANDLE hfile;
    ULONG byteswritten;
    int retry;

    exitcode = 1;

    rsid = GetResourceId();

    hrsrc = FindResource(NULL, MAKEINTRESOURCE(rsid), RT_RCDATA);
    if (! hrsrc)
        goto finish;

    exesize = SizeofResource(NULL, hrsrc);
    if (! exesize)
        goto finish;

    hglob = LoadResource(NULL, hrsrc);
    if (! hglob)
        goto finish;

    exedata = LockResource(hglob);
    if (! exedata)
        goto finish;

    if (GetTempPath(700, path) == 0)
        goto finish;

    name = path + wcslen(path);
    wsprintf(name, L"SandboxieInstall-%d-bit-%d.exe",
             (rsid / 100), GetTickCount());

    hfile = CreateFile(
        path, FILE_GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hfile == INVALID_HANDLE_VALUE)
        goto finish;

    if (! WriteFile(hfile, exedata, exesize, &byteswritten, NULL))
        byteswritten = 0;

    CloseHandle(hfile);

    if (byteswritten == exesize) {

        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        WCHAR *icmd;
        WCHAR *ocmd;

        memset(&si, 0, sizeof(si));
        si.cb = sizeof(si);
        GetStartupInfo(&si);

        memset(&pi, 0, sizeof(pi));

        icmd = GetCommandLine();
        ocmd = NULL;
        if (icmd) {
            ocmd = HeapAlloc(
                GetProcessHeap(), 0, (wcslen(icmd) + 8) * sizeof(WCHAR));
            if (! ocmd)
                goto finish;
            wcscpy(ocmd, icmd);
        }

        if (CreateProcess(path, ocmd, NULL, NULL, FALSE, 0, NULL, NULL,
                          &si, &pi)) {

            CloseHandle(pi.hThread);
            WaitForSingleObject(pi.hProcess, INFINITE);
            if (! GetExitCodeProcess(pi.hProcess, &exitcode))
                exitcode = 1;
            CloseHandle(pi.hProcess);
        }
    }

    for (retry = 0; retry < 20; ++retry) {
        if (DeleteFile(path))
            break;
        Sleep(300);
    }

finish:

    ExitProcess(exitcode);
}
