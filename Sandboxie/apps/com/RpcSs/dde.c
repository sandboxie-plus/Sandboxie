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
// DDE Helper Window
//---------------------------------------------------------------------------


#include <windows.h>
#include <dde.h>
#include "core/dll/sbiedll.h"
#include "common/defines.h"
#include "common/my_version.h"


//---------------------------------------------------------------------------
//
// dummy window for DDEML support
//
// DDEML will call (internal) user32!ValidateHwnd to validate the window
// it sees in DDE messages, and if this window is outside the sandbox,
// the conversation fails.  this could be fixed with UserHandleGrantAccess
// if not for the integrity level difference between the process in the
// sandbox and the window outside the sandbox.
//
// to work around this, we use this dummy window which is in the sandbox
// and at the same integrity level as the DDE server process, so always
// accepted by user32!ValidateHwnd
//
// this dummy proxy window is used by the Gui_DDE_INITIATE_Received,
// Gui_DDE_ACK_Sending and Gui_DDE_COPYDATA_Received functions
// in file core/dll/guidde.c -- see there for more
//
// note the window class name is _DDE_ProxyClass1 and there is another
// proxy window called _DDE_ProxyClass2 created by DdeProxyThreadSlave
// in the SbieSvc GUI Proxy in file core/svc/GuiServer.cpp
//
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Dde_Thread
//---------------------------------------------------------------------------


_FX ULONG Dde_Thread(void *arg)
{
    ATOM atom;
    HWND hwnd;
    WNDCLASS wc;
    MSG msg;

    if (SbieApi_QueryProcessInfo(0, 0) & SBIE_FLAG_OPEN_ALL_WIN_CLASS)
        return 0;

    memzero(&wc, sizeof(wc));
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = SANDBOXIE L"_DDE_ProxyClass1";
    atom = RegisterClass(&wc);
    if (! atom)
        return 0;

    hwnd = CreateWindowEx(0, (LPCWSTR)atom, L"", 0,
                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                NULL, NULL, GetModuleHandle(NULL), NULL);
    if (! hwnd)
        return 0;

    while (1) {

        GetMessage(&msg, NULL, 0, 0);
        DispatchMessage(&msg);
    }

    return 0;
}
