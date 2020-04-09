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
// SHFileOperation with Yes To All button
//---------------------------------------------------------------------------


#include <windows.h>
#include <shellapi.h>
#include "CommonUtils.h"
#include "common/defines.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static LRESULT CALLBACK MyFileOp_Hook(
    int nCode, WPARAM wParam, LPARAM lParam);

static LRESULT CALLBACK MyFileOp_NewWindowProc(
    HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static HWND MyFileOp_InsertYesToAllButton(HWND hwnd);

static BOOL MyFileOp_CountButtons(HWND hwnd, LPARAM lParam);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static HWND MyFileOp_main_hwnd = NULL;
static HWND MyFileOp_button_hwnd = NULL;
static const WCHAR *MyFileOp_button_text = NULL;

static BOOL *MyFileOp_pYesToAll = NULL;

WNDPROC MyFileOp_OldWindowProc = NULL;


//---------------------------------------------------------------------------
// MyFileOp_Hook
//---------------------------------------------------------------------------


_FX LRESULT CALLBACK MyFileOp_Hook(
    int nCode, WPARAM wParam, LPARAM lParam)
{
    HWND hwnd = (HWND)wParam;

    if (MyFileOp_main_hwnd != (HWND)-1) {

        if (nCode == HCBT_ACTIVATE
                    && GetParent(hwnd) == MyFileOp_main_hwnd) {

            MyFileOp_main_hwnd = hwnd;
            MyFileOp_OldWindowProc = (WNDPROC)SetWindowLongPtr(
                hwnd, GWLP_WNDPROC, (ULONG_PTR)MyFileOp_NewWindowProc);
        }

        if (nCode == HCBT_DESTROYWND && hwnd == MyFileOp_main_hwnd) {

            MyFileOp_main_hwnd = (HWND)-1;
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}


//---------------------------------------------------------------------------
// MyFileOp_NewWindowProc
//---------------------------------------------------------------------------


_FX LRESULT CALLBACK MyFileOp_NewWindowProc(
    HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (MyFileOp_main_hwnd && MyFileOp_pYesToAll) {

        if (msg == WM_WINDOWPOSCHANGED) {
            WINDOWPOS *pos = (WINDOWPOS *)lParam;
            if (pos->flags & SWP_SHOWWINDOW) {

                MyFileOp_button_hwnd = MyFileOp_InsertYesToAllButton(hwnd);
            }
        }

        if (msg == WM_COMMAND && HIWORD(wParam) == BN_CLICKED
                && lParam == (LPARAM)MyFileOp_button_hwnd) {

            *MyFileOp_pYesToAll = TRUE;
            msg = WM_CLOSE;
        }
    }

    return CallWindowProc(
                MyFileOp_OldWindowProc, hwnd, msg,  wParam, lParam);
}


//---------------------------------------------------------------------------
// MyFileOp_InsertYesToAllButton
//---------------------------------------------------------------------------


_FX HWND MyFileOp_InsertYesToAllButton(HWND hwnd)
{
    RECT rc;
    SIZE sz;
    int width, height;
    HWND button;
    HDC dc;
    HFONT font;
    ULONG ButtonCount;

    //
    // window with just one button is probably an error window
    //

    ButtonCount = 0;
    EnumChildWindows(hwnd, MyFileOp_CountButtons, (LPARAM)&ButtonCount);
    if (ButtonCount == 1)
        return NULL;

    //
    // create a button and measure its dimensions
    //

    button = CreateWindow(
        L"BUTTON", MyFileOp_button_text,
        BS_DEFPUSHBUTTON | WS_CHILD | WS_VISIBLE,
        0, 0, -1, -1, hwnd, (HMENU)'tz', NULL, NULL);

    dc = GetDC(button);
    font = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
    if (font) {
        SelectObject(dc, font);
        SendMessage(button, WM_SETFONT, (WPARAM)font, 0);
    }

    GetTextExtentPoint32(
        dc, MyFileOp_button_text, wcslen(MyFileOp_button_text), &sz);

    ReleaseDC(button, dc);

    sz.cx += 40;
    sz.cy += 40;

    //
    // resize dialog box window
    //

    GetWindowRect(hwnd, &rc);

    width = rc.right - rc.left;
    height = rc.bottom - rc.top;

    while (width < sz.cx) {
        rc.left -= 20;
        width += 20;
    }

    height += sz.cy + 10;

    MoveWindow(hwnd, rc.left, rc.top, width, height, TRUE);

    //
    // resize and move button
    //

    GetClientRect(hwnd, &rc);

    if (rc.right - rc.left > sz.cx + 20)
        sz.cx = rc.right - rc.left - 20;

    rc.bottom -= 10;
    rc.top = rc.bottom - sz.cy;
    rc.left = (rc.right - rc.left - sz.cx) / 2;
    rc.right = rc.left + sz.cx;

    SetWindowPos(button, NULL,
        rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 0);

    //
    // finish
    //

    return button;
}


//---------------------------------------------------------------------------
// MyFileOp_CountButtons
//---------------------------------------------------------------------------


_FX BOOL MyFileOp_CountButtons(HWND hwnd, LPARAM lParam)
{
    WCHAR clsnm[256];
    GetClassName(hwnd, clsnm, 250);
    clsnm[250] = L'\0';
    if (_wcsicmp(clsnm, L"BUTTON") == 0) {
        ULONG *ButtonCount = (ULONG *)lParam;
        ++(*ButtonCount);
    }
    return TRUE;
}


//---------------------------------------------------------------------------
// Common_SHFileOperation
//---------------------------------------------------------------------------


_FX int Common_SHFileOperation(void *lpSHFileOpStruct, BOOL *pYesToAll,
                               const WCHAR *ReplaceButtonText)
{
    SHFILEOPSTRUCT *shop = (SHFILEOPSTRUCT *)lpSHFileOpStruct;
    HHOOK hhk = NULL;
    int rv;

    MyFileOp_main_hwnd = NULL;
    MyFileOp_button_hwnd = NULL;
    MyFileOp_button_text = NULL;
    MyFileOp_pYesToAll = NULL;

    if (pYesToAll) {

        *pYesToAll = FALSE;

        if (! (shop->fFlags & FOF_NOCONFIRMATION)) {

            MyFileOp_main_hwnd = shop->hwnd;
            MyFileOp_button_hwnd = NULL;
            MyFileOp_button_text = ReplaceButtonText;
            MyFileOp_pYesToAll = pYesToAll;

            hhk = SetWindowsHookEx(
                WH_CBT, MyFileOp_Hook, NULL, GetCurrentThreadId());
        }
    }

    rv = SHFileOperation(lpSHFileOpStruct);

    if (hhk) {

        MyFileOp_main_hwnd = (HWND)-1;
        MyFileOp_button_hwnd = NULL;
        MyFileOp_button_text = NULL;
        MyFileOp_pYesToAll = NULL;

        UnhookWindowsHookEx(hhk);
    }

    return rv;
}
