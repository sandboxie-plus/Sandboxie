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
// Run Sandboxed Dialog
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "resource.h"
#include "msgs/msgs.h"
#include "common/list.c"
#define COBJMACROS
#include "core/dll/sbiedll.h"
#include "core/dll/pstore.h"
#include "common/defines.h"
#include "apps/common/CommonUtils.h"
#include "common/my_version.h"
#define INITGUID
#include <guiddef.h>
#include <commdlg.h>


//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------


typedef struct _tag_HIST_ITEM {
    LIST_ELEM list_elem;
    ULONG_PTR idx;
    WCHAR cmd[1];
} HIST_ITEM;


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static HICON hProgramIcon;

static WCHAR *editbox_space;
static WCHAR *editbox;
static int editbox_len;
static HICON hDeleteIcon;
static WCHAR *msg_3308;

static IPStore *pst;
static LIST history;

static BOOL initialized = FALSE;

DEFINE_GUID(GUID_SANDBOXIE,
    0xBE6FB014L, 0x7308, 0x47F5, 0x85,0x16, 0x80,0x2E,0xBF,0x98,0x7F,0xD2);

static const WCHAR *RunHistory = L"RunHistory";

const WCHAR *Sandboxie = SANDBOXIE;

extern BOOL run_elevated_2;

extern BOOLEAN layout_rtl;


//---------------------------------------------------------------------------
// LoadHistory
//---------------------------------------------------------------------------


void LoadHistory(void)
{
    HRESULT hr;
    BYTE *data;
    DWORD len;
    WCHAR *ptr;
    HIST_ITEM *hist_item;

    List_Init(&history);
    pst = (IPStore*)SbieDll_InitPStore();
    if (! pst)
        return;

    hr = pst->ReadItem(
        PST_KEY_CURRENT_USER,
        &GUID_SANDBOXIE, &GUID_SANDBOXIE, RunHistory, &len, &data, NULL, 0);
    if (FAILED(hr) || len == 0)
        return;

    ptr = (WCHAR *)data;
    while (*ptr) {
        len = sizeof(HIST_ITEM) + (wcslen(ptr) + 1) * sizeof(WCHAR);
        hist_item = (HIST_ITEM *)HeapAlloc(
            GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, len);
        wcscpy(hist_item->cmd, ptr);
        List_Insert_After(&history, NULL, hist_item);
        ptr += wcslen(ptr) + 1;
    }

    CoTaskMemFree(data);
}


//---------------------------------------------------------------------------
// SaveHistory
//---------------------------------------------------------------------------


void SaveHistory(void)
{
    HRESULT hr;
    DWORD len;
    BYTE *data;
    HIST_ITEM *hist_item, *next_item;
    WCHAR *ptr;
    PST_TYPEINFO typeinfo;

    if (! pst)
        return;

    // create a new head element using the contents of the edit box,
    // then loop through the items, deleting items that are the same
    // as the editbox, and counting the length of all others

    len = sizeof(HIST_ITEM);
    if (editbox[0]) {
        len += (wcslen(editbox) + 1) * sizeof(WCHAR);
        hist_item = (HIST_ITEM *)HeapAlloc(
            GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, len);
        wcscpy(hist_item->cmd, editbox);
        List_Insert_Before(&history, NULL, hist_item);
    }

    hist_item = (HIST_ITEM*)List_Head(&history);
    if (hist_item)
        next_item = (HIST_ITEM *)List_Next(hist_item);
    else
        next_item = NULL;
    while (next_item) {
        hist_item = next_item;
        next_item = (HIST_ITEM *)List_Next(hist_item);
        if (wcscmp(hist_item->cmd, editbox) == 0)
            List_Remove(&history, hist_item);
        else
            len += (wcslen(hist_item->cmd) + 1) * sizeof(WCHAR);
    }
    len += sizeof(WCHAR);       // terminator null

    // allocate an appropriately-sized output buffer,
    // then loop through the items, copying them into the buffer

    data = (BYTE *)HeapAlloc(
        GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY, len);
    ptr = (WCHAR *)data;
    hist_item = (HIST_ITEM*)List_Head(&history);
    while (hist_item) {
        wcscpy(ptr, hist_item->cmd);
        ptr += wcslen(ptr) + 1;
        hist_item = (HIST_ITEM *)List_Next(hist_item);
    }
    *ptr = L'\0';               // terminator null
    len = (DWORD)((BYTE *)ptr + sizeof(WCHAR) - data);

    // write out the data buffer.  create any necessary types

    hr = pst->WriteItem(
        PST_KEY_CURRENT_USER,
        &GUID_SANDBOXIE, &GUID_SANDBOXIE, RunHistory, len, data, NULL, 0, 0);
    if (FAILED(hr)) {

        typeinfo.cbSize = sizeof(typeinfo);
        typeinfo.szDisplayName = (WCHAR *)Sandboxie;
        hr = pst->CreateType(
            PST_KEY_CURRENT_USER, &GUID_SANDBOXIE, &typeinfo, 0);
        hr = pst->CreateSubtype(
            PST_KEY_CURRENT_USER,
            &GUID_SANDBOXIE, &GUID_SANDBOXIE, &typeinfo, NULL, 0);

        hr = pst->WriteItem(
            PST_KEY_CURRENT_USER,
            &GUID_SANDBOXIE, &GUID_SANDBOXIE, RunHistory,
            len, data, NULL, 0, 0);
    }
}


//---------------------------------------------------------------------------
// CreateHistoryCombo
//---------------------------------------------------------------------------


HWND CreateHistoryCombo(
    HWND hwnd)
{
    HWND ctrl;
    RECT rc;
    LRESULT lResult;
    HIST_ITEM *hist_item;
    COMBOBOXEXITEM cbei;

    ctrl = GetDlgItem(hwnd, IDC_COMBO);
    GetWindowRect(ctrl, &rc);
    if (layout_rtl) {
        SetWindowLong(hwnd, GWL_EXSTYLE,
                      GetWindowLong(hwnd, GWL_EXSTYLE) & ~WS_EX_LAYOUTRTL);
    }
    MapWindowPoints(NULL, hwnd, (LPPOINT)&rc, 2);

    ctrl = CreateWindowEx(
        0, WC_COMBOBOXEX, L"", CBS_DROPDOWN | CBS_AUTOHSCROLL |
        WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_TABSTOP,
        rc.left, rc.top, rc.right - rc.left, 300, hwnd, NULL, NULL, NULL);

    if (layout_rtl) {
        SetWindowLong(hwnd, GWL_EXSTYLE,
                      GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYOUTRTL);
    }

    if (! ctrl)
        return NULL;

    do {
        lResult = SendMessage(ctrl, CBEM_DELETEITEM, 0, 0);
    } while (lResult != CB_ERR && lResult != 0);

    hist_item = (HIST_ITEM*)List_Head(&history);
    while (hist_item) {
        cbei.mask = CBEIF_TEXT;
        cbei.iItem = -1;
        cbei.pszText = hist_item->cmd;
        cbei.cchTextMax = wcslen(hist_item->cmd);
        hist_item->idx =
            SendMessage(ctrl, CBEM_INSERTITEM, 0, (LPARAM)&cbei);
        hist_item = (HIST_ITEM *)List_Next(hist_item);
    }

    return ctrl;
}


//---------------------------------------------------------------------------
// DeleteHistoryItem
//---------------------------------------------------------------------------


void DeleteHistoryItem(
    HWND hwndCombo)
{
    HIST_ITEM *hist_item, *next_item;
    COMBOBOXEXITEM cbei;
    WCHAR text[512];

    next_item = (HIST_ITEM*)List_Head(&history);
    while (next_item) {
        hist_item = next_item;
        next_item = (HIST_ITEM *)List_Next(hist_item);
        if (wcscmp(hist_item->cmd, editbox) == 0) {
            List_Remove(&history, hist_item);
            break;
        }
    }

    cbei.mask = CBEIF_TEXT;
    cbei.iItem = 0;
    while (1) {
        cbei.pszText = text;
        cbei.cchTextMax = sizeof(text) / sizeof(WCHAR) - 1;
        if (SendMessage(hwndCombo, CBEM_GETITEM, 0, (LPARAM)&cbei) == 0)
            break;
        if (wcscmp(text, editbox) == 0) {
            SendMessage(hwndCombo, CBEM_DELETEITEM, cbei.iItem, 0);
            break;
        }
        ++cbei.iItem;
    }
}


//---------------------------------------------------------------------------
// PrepareRunAsAdmin
//---------------------------------------------------------------------------


void PrepareRunAsAdmin(HWND hwnd, const WCHAR *BoxName, BOOLEAN JustAdmin)
{
    RECT rc;
    ULONG width, height;
    HWND ctrl;
    BOOL disable_button = FALSE;

    //
    // check if elevation is possible and if it is in effect
    //

    extern ULONG Token_Elevation_Type;

    if (Token_Elevation_Type == TokenElevationTypeNone)
        return;

    if (Token_Elevation_Type == TokenElevationTypeFull) {

        run_elevated_2 = TRUE;
        disable_button = TRUE;
    }

    //
    // disable button if dropped rights is in effect
    //

    if (BoxName) {

        if ((BoxName == (WCHAR *)-1) ||
                SbieApi_QueryConfBool(BoxName, L"UseSecurityMode", FALSE) ||
                SbieApi_QueryConfBool(BoxName, L"DropAdminRights", FALSE)) {

            run_elevated_2 = FALSE;
            disable_button = TRUE;
        }

    } else {

        ULONG64 ProcessFlags = SbieApi_QueryProcessInfo(0, 0);
        if (ProcessFlags & SBIE_FLAG_DROP_RIGHTS) {

            run_elevated_2 = FALSE;
            disable_button = TRUE;
        }
    }

    //
    //
    //

    if (JustAdmin) {

        ctrl = GetDlgItem(hwnd, IDRUNADMIN);
        if (ctrl) {

            SendMessage(ctrl, BM_SETCHECK,
                run_elevated_2 ? BST_CHECKED : BST_UNCHECKED, 0);
            EnableWindow(ctrl, (! disable_button));
            ShowWindow(ctrl, SW_SHOW);
        }

        return;
    }

    //
    //
    //

    GetWindowRect(hwnd, &rc);
    width = rc.right - rc.left + 1;
    height = rc.bottom - rc.top + 1 + 25;
    MoveWindow(hwnd, rc.left, rc.top, width, height, TRUE);

    ctrl = GetDlgItem(hwnd, IDOK);
    if (ctrl) {
        GetWindowRect(ctrl, &rc);
        MapWindowPoints(NULL, hwnd, (POINT *)&rc, 2);
        width = rc.right - rc.left + 1;
        height = rc.bottom - rc.top + 1;
        rc.top += 25;
        MoveWindow(ctrl, rc.left, rc.top, width, height, TRUE);
    }

    ctrl = GetDlgItem(hwnd, IDCANCEL);
    if (ctrl) {
        GetWindowRect(ctrl, &rc);
        MapWindowPoints(NULL, hwnd, (POINT *)&rc, 2);
        width = rc.right - rc.left + 1;
        height = rc.bottom - rc.top + 1;
        rc.top += 25;
        MoveWindow(ctrl, rc.left, rc.top, width, height, TRUE);
    }

    ctrl = GetDlgItem(hwnd, IDBROWSE);
    if (ctrl) {
        GetWindowRect(ctrl, &rc);
        MapWindowPoints(NULL, hwnd, (POINT *)&rc, 2);
        width = rc.right - rc.left + 1;
        height = rc.bottom - rc.top + 1;
        rc.top += 25;
        MoveWindow(ctrl, rc.left, rc.top, width, height, TRUE);
    }

    ctrl = GetDlgItem(hwnd, IDRUNADMIN);
    if (ctrl) {
        SetWindowText(ctrl, SbieDll_FormatMessage0(MSG_3414));
        if (run_elevated_2)
            SendMessage(ctrl, BM_SETCHECK, BST_CHECKED, 0);
        if (disable_button)
            EnableWindow(ctrl, FALSE);
        ShowWindow(ctrl, SW_SHOW);
    }
}


//---------------------------------------------------------------------------
// ClickRunAsAdmin
//---------------------------------------------------------------------------


void ClickRunAsAdmin(HWND hwnd)
{
    HWND ctrl;
    LRESULT lResult;

    ctrl = GetDlgItem(hwnd, IDRUNADMIN);
    lResult = SendMessage(ctrl, BM_GETCHECK, 0, 0);
    if (lResult == BST_CHECKED) {
        run_elevated_2 = FALSE;
        lResult = BST_UNCHECKED;
    } else {
        run_elevated_2 = TRUE;
        lResult = BST_CHECKED;
    }
    SendMessage(ctrl, BM_SETCHECK, lResult, 0);
}


//---------------------------------------------------------------------------
// AddToolTipForRunAsAdmin
//---------------------------------------------------------------------------


void AddToolTipForRunAsAdmin(HWND hDialog, HWND hwndToolTip)
{
    HWND ctrl = GetDlgItem(hDialog, IDRUNADMIN);
    if (ctrl) {

        TOOLINFO ti;
        memzero(&ti, sizeof(TOOLINFO));
        ti.cbSize = sizeof(TOOLINFO);
        ti.hwnd = hDialog;
        ti.uId = 1111;

        SendMessage(hwndToolTip, TTM_DELTOOL, 0, (LPARAM)&ti);

        if (! IsWindowEnabled(ctrl)) {

            ULONG msg;
            if (BST_CHECKED == SendMessage(ctrl, BM_GETCHECK, 0, 0))
                msg = MSG_3256;
            else
                msg = MSG_3255;

            ti.uFlags = TTF_SUBCLASS | TTF_CENTERTIP;
            GetWindowRect(ctrl, &ti.rect);
            MapWindowPoints(NULL, hDialog, (LPPOINT)&ti.rect, 2);
            ti.lpszText = SbieDll_FormatMessage0(msg);

            SendMessage(hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);


        }
    }
}


//---------------------------------------------------------------------------
// RunDialogProc
//---------------------------------------------------------------------------


INT_PTR RunDialogProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    static HWND hwndCombo = NULL;
    static HWND hwndEdit = NULL;
    static HWND hwndToolTip = NULL;
    HWND ctrl;
    OPENFILENAME ofn;
    WCHAR boxname[BOXNAME_COUNT];
    WCHAR title[128];
    TOOLINFO ti;

    switch (uMsg) {

        //
        // initialize dialog
        //

        case WM_INITDIALOG:

            //
            // assign text strings
            //

            SetWindowText(hwnd, SbieDll_FormatMessage0(MSG_3052));

            SetDlgItemText(hwnd, IDOK, SbieDll_FormatMessage0(MSG_3001));
            SetDlgItemText(hwnd, IDCANCEL, SbieDll_FormatMessage0(MSG_3002));
            SetDlgItemText(hwnd, IDBROWSE, SbieDll_FormatMessage0(MSG_3003));

            extern BOOL execute_open_with;
            if (execute_open_with)
            {
                SetDlgItemText(hwnd, IDRUNDLGTEXT,
                           SbieDll_FormatMessage0(MSG_3107));

                extern PWSTR ChildCmdLine;
                SetDlgItemText(hwnd, IDRUNDLGTEXT2, ChildCmdLine);
            }
            else
            {
                SetDlgItemText(hwnd, IDRUNDLGTEXT,
                           SbieDll_FormatMessage0(MSG_3103));

                SetDlgItemText(hwnd, IDRUNDLGTEXT2,
                           SbieDll_FormatMessage0(MSG_3104));
            }

            //
            // position window
            //

            SetWindowPos(hwnd, HWND_TOPMOST, 100, 100, 0, 0, SWP_NOSIZE);
            SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hProgramIcon);
            SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)NULL);

            //
            // create edit/history combo box
            //

            hwndCombo = CreateHistoryCombo(hwnd);
            if (hwndCombo) {
                SetFocus(hwndCombo);
                hwndEdit = (HWND)SendMessage(
                    hwndCombo, CBEM_GETEDITCONTROL, 0, 0);
                SHAutoComplete(hwndEdit, SHACF_FILESYSTEM |
                                         SHACF_AUTOSUGGEST_FORCE_ON |
                                         SHACF_AUTOAPPEND_FORCE_ON);

            } else
                EndDialog(hwnd, 0);

            //
            // set window title to include box name, unless BoxNameTitle=y
            //

            if (! SbieApi_QueryConfBool(NULL, L"BoxNameTitle", FALSE)) {

                SbieApi_QueryProcess(NULL, boxname, NULL, NULL, NULL);

                if (layout_rtl) {
                    wcscpy(title, boxname);
                    wcscat(title, L" - ");
                    GetWindowText(hwnd, title + wcslen(title), 100);
                } else {
                    GetWindowText(hwnd, title, 100);
                    wcscat(title, L" - ");
                    wcscat(title, boxname);
                }
                SetWindowText(hwnd, title);
            }

            //
            // delete button: assign image and create tooltip
            //

            ctrl = GetDlgItem(hwnd, IDDELETE);

            SendMessage(ctrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hDeleteIcon);

            hwndToolTip = CreateWindowEx(
                WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
                WS_POPUP | TTS_NOPREFIX | TTS_NOANIMATE | TTS_NOFADE,
                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                hwnd, NULL, (HINSTANCE)lParam, NULL);

            ti.cbSize = sizeof(TOOLINFO);
            ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
            ti.hwnd = hwnd;
            ti.uId = (UINT_PTR)ctrl;
            ti.hinst = (HINSTANCE)lParam;
            ti.lpszText = SbieDll_FormatMessage0(MSG_3105);
            ti.lParam = 0;

            SendMessage(hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);

            //
            // run as admin
            //

            PrepareRunAsAdmin(hwnd, NULL, FALSE);

            AddToolTipForRunAsAdmin(hwnd, hwndToolTip);

            //
            // end dialog initialization
            //

            SetForegroundWindow(hwnd);  // explicitly go to foreground

            return FALSE;   // don't set focus, we already did

        //
        // handle buttons
        //

        case WM_COMMAND:

            if (LOWORD(wParam) == IDOK) {

                GetWindowText(hwndEdit, editbox, editbox_len - 1);
                EndDialog(hwnd, IDOK);

            } else if (LOWORD(wParam) == IDCANCEL) {

                editbox[0] = L'\0';
                EndDialog(hwnd, IDCANCEL);

            } else if (LOWORD(wParam) == IDBROWSE) {

                WCHAR editbox_char0 = editbox[0];
                editbox[0] = L'\0';

                ZeroMemory(&ofn, sizeof(OPENFILENAME));
                ofn.lStructSize  = sizeof(OPENFILENAME);
                ofn.hwndOwner    = hwnd;
                ofn.lpstrFilter  = msg_3308;
                ofn.lpstrFile    = editbox;
                ofn.nFilterIndex = 1;
                ofn.nMaxFile     = editbox_len;
                ofn.Flags        = OFN_DONTADDTORECENT
                                 | OFN_NODEREFERENCELINKS
                                 | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
                if (GetOpenFileName(&ofn)) {

                    if (wcschr(editbox, L' ')) {
                        ULONG len = wcslen(editbox);
                        wmemmove(editbox + 1, editbox, len);
                        editbox[0] = L'\"';
                        editbox[len + 1] = L'\"';
                        editbox[len + 2] = L'\0';
                    }
                    SetWindowText(hwndEdit, editbox);

                } else
                    editbox[0] = editbox_char0;

            } else if (LOWORD(wParam) == IDDELETE) {

                GetWindowText(hwndEdit, editbox, editbox_len - 1);
                DeleteHistoryItem(hwndCombo);
                SetWindowText(hwndEdit, L"");
                SetFocus(hwndCombo);

                if (hwndToolTip) {

                    //
                    // if a dialog box has only one control with a tool tip
                    // then the tool tip stops appearing after the control is
                    // used, the following workaround fixes that
                    //

                    SendMessage(hwndToolTip, TTM_ACTIVATE, FALSE, 0);
                    SendMessage(hwndToolTip, TTM_ACTIVATE, TRUE, 0);
                }

            } else if (LOWORD(wParam) == IDRUNADMIN) {

                ClickRunAsAdmin(hwnd);
            }

            break;
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// DoRunDialog
//---------------------------------------------------------------------------


WCHAR *DoRunDialog(HINSTANCE hInstance)
{
    INITCOMMONCONTROLSEX icc;
    INT_PTR r;

    if (! initialized) {

        icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icc.dwICC = ICC_USEREX_CLASSES | ICC_TAB_CLASSES;
        InitCommonControlsEx(&icc);

        hProgramIcon = (HICON)LoadImage(
            hInstance, MAKEINTRESOURCE(IDICON), IMAGE_ICON,
            0, 0, LR_DEFAULTSIZE);

        editbox_len = 10240;
        editbox_space = (WCHAR *)HeapAlloc(
            GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS,
            editbox_len * sizeof(WCHAR));

        hDeleteIcon = (HICON)LoadImage(
            hInstance, MAKEINTRESOURCE(IDDELETE), IMAGE_ICON,
            16, 16, 0);

        LoadHistory();

        msg_3308 = SbieDll_FormatMessage0(MSG_3308);
        while (1) {
            WCHAR *ptr = wcsrchr(msg_3308, L'#');
            if (! ptr)
                break;
            *ptr = L'\0';
        }

        initialized = TRUE;
    }

    editbox = editbox_space;
    editbox[0] = L'\0';

    if (layout_rtl) {

        LPCDLGTEMPLATE tmpl = (LPCDLGTEMPLATE)Common_DlgTmplRtl(
                                    hInstance, MAKEINTRESOURCE(RUN_DIALOG));
        r = DialogBoxIndirectParam(hInstance, tmpl,
                           NULL, RunDialogProc, (LPARAM)hInstance);

    } else {

        r = DialogBoxParam(hInstance, MAKEINTRESOURCE(RUN_DIALOG),
                           NULL, RunDialogProc, (LPARAM)hInstance);
    }

    if (r == IDOK)
        SaveHistory();
    if (r != IDOK || ! editbox[0])
        editbox = NULL;

    return editbox;
}
