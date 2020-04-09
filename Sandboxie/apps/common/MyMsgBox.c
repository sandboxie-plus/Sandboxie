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
// Message Box Utility
//---------------------------------------------------------------------------


#include "MyMsgBox.h"
#include "common/defines.h"


//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------


typedef struct _MY_MSG_BOX_DATA {

    const WCHAR *CheckText;
    HWND hwnd;
    WCHAR *EditText;
    ULONG EditStyle;

} MY_MSG_BOX_DATA;


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static MY_MSG_BOX_DATA *MyMsgBox_Data[4] = { NULL, NULL, NULL, NULL };
static volatile ULONG MyMsgBox_Level = 0;


#define MyMsgBox_CheckText   (MyMsgBox_Data[MyMsgBox_Level]->CheckText)
#define MyMsgBox_hwnd        (MyMsgBox_Data[MyMsgBox_Level]->hwnd)
#define MyInputBox_EditText  (MyMsgBox_Data[MyMsgBox_Level]->EditText)
#define MyInputBox_EditStyle (MyMsgBox_Data[MyMsgBox_Level]->EditStyle)


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static LRESULT CALLBACK MyMsgBox_Hook(
    int nCode, WPARAM wParam, LPARAM lParam);

static void MyMsgBox_InsertCheckBox(HWND hwnd);

static LRESULT CALLBACK MyInputBox_Hook(
    int nCode, WPARAM wParam, LPARAM lParam);

static void MyInputBox_InsertEditBox(HWND hwnd);

static void MyMsgBox_PushPop(BOOLEAN Push);


//---------------------------------------------------------------------------
// MyMsgBox_Hook
//---------------------------------------------------------------------------


_FX LRESULT CALLBACK MyMsgBox_Hook(
    int nCode, WPARAM wParam, LPARAM lParam)
{
    HWND hwnd = (HWND)wParam;

    if (MyMsgBox_hwnd != (HWND)-1) {

        if (nCode == HCBT_ACTIVATE && GetParent(hwnd) == MyMsgBox_hwnd) {

            MyMsgBox_hwnd = hwnd;
            MyMsgBox_InsertCheckBox(hwnd);
        }

        if (nCode == HCBT_DESTROYWND && hwnd == MyMsgBox_hwnd) {

            MyMsgBox_hwnd = (HWND)-1;
            if (BST_CHECKED == SendDlgItemMessage(
                                    hwnd, 'tz', BM_GETCHECK, 0, 0))
                MyMsgBox_CheckText = INVALID_HANDLE_VALUE;
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}


//---------------------------------------------------------------------------
// MyMsgBox_InsertCheckBox
//---------------------------------------------------------------------------


_FX void MyMsgBox_InsertCheckBox(HWND hwnd)
{
    RECT rc;
    SIZE sz;
    int width, height;
    HWND button, stat;
    HDC dc;
    HFONT font;

    //
    // create a button and measure its dimensions
    //

    button = CreateWindow(
        L"BUTTON", MyMsgBox_CheckText,
        BS_AUTOCHECKBOX | WS_CHILD | WS_VISIBLE,
        0, 0, -1, -1, hwnd, (HMENU)'tz', NULL, NULL);

    dc = GetDC(button);
    font = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
    if (font) {
        SelectObject(dc, font);
        SendMessage(button, WM_SETFONT, (WPARAM)font, 0);
    }

    GetTextExtentPoint32(
        dc, MyMsgBox_CheckText, wcslen(MyMsgBox_CheckText), &sz);

    ReleaseDC(button, dc);

    sz.cx += 40;

    //
    // resize message box window
    //

    GetWindowRect(hwnd, &rc);

    width = rc.right - rc.left;
    height = rc.bottom - rc.top;

    while (width < sz.cx) {
        rc.left -= 20;
        width += 20;
    }

    height += sz.cy * 2;

    MoveWindow(hwnd, rc.left, rc.top, width, height, TRUE);

    //
    // create empty static text window over the extended part of the
    // window, to make sure the color is consistent.  then reposition
    // the button on top of that static
    //

    GetClientRect(hwnd, &rc);

    stat = CreateWindow(
        L"STATIC", L"", WS_CHILD | WS_VISIBLE,
        0, rc.bottom - sz.cy * 2, rc.right, rc.bottom,
        hwnd, (HMENU)-1, NULL, NULL);

    SetWindowPos(button, stat, 10, rc.bottom - sz.cy * 2, sz.cx, sz.cy, 0);
}


//---------------------------------------------------------------------------
// MessageCheckBox
//---------------------------------------------------------------------------


_FX int MessageCheckBox(
    HWND hwnd, const WCHAR *text, const WCHAR *caption, UINT flags,
    const WCHAR *check)
{
    HHOOK hhk;
    int rv;

    MyMsgBox_PushPop(TRUE);

    MyMsgBox_CheckText = check;
    MyMsgBox_hwnd = hwnd;

    hhk = SetWindowsHookEx(
        WH_CBT, MyMsgBox_Hook, NULL, GetCurrentThreadId());

    rv = MessageBox(hwnd, text, caption, flags);

    if (MyMsgBox_CheckText == INVALID_HANDLE_VALUE)
        rv = -rv;

    MyMsgBox_CheckText = NULL;
    MyMsgBox_hwnd = (HWND)-1;

    if (hhk)
        UnhookWindowsHookEx(hhk);

    MyMsgBox_PushPop(FALSE);

    return rv;
}


//---------------------------------------------------------------------------
// MyInputBox_Hook
//---------------------------------------------------------------------------


_FX LRESULT CALLBACK MyInputBox_Hook(
    int nCode, WPARAM wParam, LPARAM lParam)
{
    HWND hwnd = (HWND)wParam;

    if (MyMsgBox_hwnd != (HWND)-1) {

        if (nCode == HCBT_ACTIVATE && GetParent(hwnd) == MyMsgBox_hwnd) {

            MyMsgBox_hwnd = hwnd;
            MyInputBox_InsertEditBox(hwnd);
        }

        if (nCode == HCBT_DESTROYWND && hwnd == MyMsgBox_hwnd) {

            int len = (int)SendDlgItemMessage(
                            hwnd, 'tz', WM_GETTEXTLENGTH, 0, 0);
            if (len) {
                MyInputBox_EditText =
                    LocalAlloc(LMEM_FIXED, (len + 4) * sizeof(WCHAR));
                SendDlgItemMessage(
                    hwnd, 'tz', WM_GETTEXT,
                    (WPARAM)(len + 2), (LPARAM)MyInputBox_EditText);
            }

            MyMsgBox_hwnd = (HWND)-1;
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}


//---------------------------------------------------------------------------
// MyInputBox_InsertEditBox
//---------------------------------------------------------------------------


_FX void MyInputBox_InsertEditBox(HWND hwnd)
{
    HWND child, edit;
    HDC dc;
    HFONT font;
    SIZE sz;
    RECT rc;
    POINT p;
    ULONG oldStyleEx;

    //
    // hide all child windows that are not buttons
    //

    child = GetWindow(hwnd, GW_CHILD);
    while (child) {
        int id = GetDlgCtrlID(child);
        if (id != IDOK && id != IDCANCEL)
            ShowWindow(child, SW_HIDE);
        child = GetWindow(child, GW_HWNDNEXT);
    }

    //
    // create an edit box
    //

    oldStyleEx = GetWindowLong(hwnd, GWL_EXSTYLE);
    SetWindowLong(hwnd, GWL_EXSTYLE, oldStyleEx | WS_EX_NOINHERITLAYOUT);

    edit = CreateWindowEx(
        WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | MyInputBox_EditStyle,
        0, 0, -1, -1, hwnd, (HMENU)'tz', NULL, NULL);

    SetWindowLong(hwnd, GWL_EXSTYLE, oldStyleEx);

    dc = GetDC(edit);
    font = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
    if (font) {
        SelectObject(dc, font);
        SendMessage(edit, WM_SETFONT, (WPARAM)font, 0);
    }

    GetTextExtentPoint32(dc, L"M", 1, &sz);
    sz.cy += sz.cy / 2;

    ReleaseDC(edit, dc);

    if (MyInputBox_EditText) {
        SetWindowText(edit, MyInputBox_EditText);
        MyInputBox_EditText = NULL;
    }

    //
    // get the size of the client area, excluding the buttons
    //

    GetWindowRect(GetDlgItem(hwnd, IDOK), &rc);
    p.x = 0;
    p.y = rc.top;
    ScreenToClient(hwnd, &p);

    GetClientRect(hwnd, &rc);

    MoveWindow(edit,
               20, (p.y - sz.cy) / 2,
               rc.right - 20 * 2, sz.cy,
               TRUE);

    SetFocus(edit);
}


//---------------------------------------------------------------------------
// InputBox
//---------------------------------------------------------------------------


_FX WCHAR *InputBox(HWND hwnd, const WCHAR *caption, UINT flags,
                const WCHAR *def)
{
    HHOOK hhk;
    int rv;
    WCHAR *input;
    int def_len;
    WCHAR *text;

    MyMsgBox_PushPop(TRUE);

    MyInputBox_EditStyle = ES_AUTOHSCROLL;
    if (flags & INPUTBOX_PASSWORD) {
        flags &= ~INPUTBOX_PASSWORD;
        MyInputBox_EditStyle |= ES_PASSWORD;
    }
    if (flags & INPUTBOX_READONLY) {
        flags &= ~INPUTBOX_READONLY;
        MyInputBox_EditStyle |= ES_READONLY;
    }

    MyInputBox_EditText = (WCHAR *)def;
    MyMsgBox_hwnd = hwnd;

    hhk = SetWindowsHookEx(
        WH_CBT, MyInputBox_Hook, NULL, GetCurrentThreadId());

    if (def)
        def_len = wcslen(def);
    else
        def_len = 0;
    text = LocalAlloc(LMEM_FIXED, (def_len + 50) * sizeof(WCHAR));
    wcscpy(text, L"MMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nM");
    while (def_len > 0) {
        wcscat(text, L"MMMMM");
        def_len -= 5;
    }

    rv = MessageBox(hwnd, text, caption, flags);

    LocalFree(text);

    if (rv == IDOK)
        input = MyInputBox_EditText;
    else {
        input = NULL;
        if (MyInputBox_EditText && MyInputBox_EditText != def)
            LocalFree(MyInputBox_EditText);
    }

    MyInputBox_EditText = NULL;
    MyMsgBox_hwnd = (HWND)-1;

    if (hhk)
        UnhookWindowsHookEx(hhk);

    MyMsgBox_PushPop(FALSE);

    return input;
}


//---------------------------------------------------------------------------
// MyMsgBox_PushPop
//---------------------------------------------------------------------------


_FX void MyMsgBox_PushPop(BOOLEAN Push)
{
    if (Push) {
        ++MyMsgBox_Level;
        if (! MyMsgBox_Data[MyMsgBox_Level]) {
            MyMsgBox_Data[MyMsgBox_Level] =
                LocalAlloc(LMEM_FIXED, sizeof(MY_MSG_BOX_DATA));
        }
        memzero(MyMsgBox_Data[MyMsgBox_Level], sizeof(MY_MSG_BOX_DATA));
    } else
        --MyMsgBox_Level;
}
