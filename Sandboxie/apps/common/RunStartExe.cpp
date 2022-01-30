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
// Common Utility Functions
//---------------------------------------------------------------------------

#include "..\control\stdafx.h"

#include "CommonUtils.h"
#include "common/defines.h"
#include "common/my_version.h"
#include "core/dll/sbiedll.h"


//---------------------------------------------------------------------------
// Common_RunStartExe
//---------------------------------------------------------------------------


void Common_RunStartExe(
    const CString &cmd, const CString &box, BOOL wait, BOOL inherit)
{
    WCHAR *cmdline = (WCHAR *)LocalAlloc(LMEM_FIXED, 2048 * sizeof(WCHAR));

    wcscpy(cmdline, L"/box:");
    if (box.IsEmpty())
        wcscat(cmdline, L"__ask__");
    else
        wcscat(cmdline, box);
    wcscat(cmdline, L" ");
    wcscat(cmdline, cmd);

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    memzero(&si, sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    if (inherit)
        si.lpReserved = (LPTSTR)1;

    if (! SbieDll_RunFromHome(START_EXE, cmdline, &si, &pi)) {

        ULONG ErrorCode = GetLastError();
        WCHAR *msg = (WCHAR *)LocalAlloc(LMEM_FIXED, 2560 * sizeof(WCHAR));
        DWORD FormatFlags = FORMAT_MESSAGE_FROM_SYSTEM |
                            FORMAT_MESSAGE_IGNORE_INSERTS;
        FormatMessage(FormatFlags, NULL, ErrorCode,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      (LPTSTR)msg, 500, NULL);
        wcscat(msg, L"\r\n\r\n" START_EXE L" ");
        wcscat(msg, cmdline);

        MessageBox(NULL, msg, SANDBOXIE L" " START_EXE L" Failed",
                   MB_OK | MB_TOPMOST);

        LocalFree(msg);

    } else {

        CWaitCursor waitcursor;

        while (wait) {

            waitcursor.Restore();
            ULONG rc = MsgWaitForMultipleObjects(
                        1, &pi.hProcess, FALSE, INFINITE, QS_ALLEVENTS);
            if (rc == WAIT_OBJECT_0)
                break;

            MSG msg;
            while (1) {
                if (! PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                    break;
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
            }
        }

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    LocalFree(cmdline);
}
