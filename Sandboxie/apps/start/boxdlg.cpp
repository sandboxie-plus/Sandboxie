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
// Select Sandbox Dialog
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "common/defines.h"
#include "core/dll/sbiedll.h"
#include "core/svc/SbieIniWire.h"
#include "resource.h"
#include "msgs/msgs.h"
#include "apps/common/CommonUtils.h"
#include "apps/common/BoxOrder.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


void PrepareRunAsAdmin(HWND hwnd, const WCHAR *BoxName, BOOLEAN JustAdmin);
void AddToolTipForRunAsAdmin(HWND hDialog, HWND hwndToolTip);
void ClickRunAsAdmin(HWND hwnd);
void Adjust_Window_Height(HWND hwnd);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static HICON hProgramIcon;

static WCHAR boxname[64];

static BOOL initialized = FALSE;

static int dfp_index = LB_ERR;

BOOL boxdlg_run_outside_sandbox = FALSE;

extern BOOL disable_force_on_this_program;

extern BOOLEAN layout_rtl;


//---------------------------------------------------------------------------
// CanDisableForce
//---------------------------------------------------------------------------


BOOLEAN CanDisableForce(void)
{
    static BOOLEAN init = FALSE;
    static BOOLEAN retval = FALSE;

    if (! init) {

        if (SbieApi_QueryConfBool(NULL, L"ForceDisableAdminOnly", FALSE)) {

            extern void *CallSbieSvcGetUser(void);
            SBIE_INI_GET_USER_RPL *rpl =
                (SBIE_INI_GET_USER_RPL *)CallSbieSvcGetUser();
            if (rpl) {
                if (rpl->admin)
                    retval = TRUE;
                SbieDll_FreeMem(rpl);
            }

        } else
            retval = TRUE;

        init = TRUE;
    }

    return retval;
}


//---------------------------------------------------------------------------
// GetBoxName
//---------------------------------------------------------------------------


BOOL GetBoxName(HWND listbox, int index, WCHAR *boxname)
{
    WCHAR *text;
    BOOL rv = FALSE;

    if (dfp_index != LB_ERR && index >= dfp_index - 1)
        text = NULL;
    else
        text = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, 512 * sizeof(WCHAR));

    if (text) {
        LRESULT lResult =
            SendMessage(listbox, LB_GETTEXT, index, (LPARAM)text);
        if (lResult != LB_ERR) {

            WCHAR *ptr = text;
            while (*ptr == L' ')
                ++ptr;
            if (*ptr != L'(') {

                wcscpy(boxname, ptr);
                rv = TRUE;
            }
        }
        HeapFree(GetProcessHeap(), 0, text);
    }

    if (! rv)
        boxname[0] = L'\0';

    return rv;
}


//---------------------------------------------------------------------------
// TrackToolTip
//---------------------------------------------------------------------------


void TrackToolTip(HWND hwnd, int first_time)
{
    static LRESULT last_index;
    static HWND htip = NULL, last_hlist = NULL;
    static int tip_x = 0, tip_y = 0;
    static WCHAR *boxtext = NULL;

    LRESULT index, lResult;
    WCHAR boxname[64];
    TOOLINFO ti;
    RECT rect;
    POINT pt;

    BOOLEAN just_hide = TRUE;

    //
    // get a handle to the listbox, or use the recorded handle
    //

    HWND hlist = NULL;
    if (hwnd)
        hlist = GetDlgItem(hwnd, IDBOXLIST);
    if (! hlist)
        hlist = last_hlist;
    if (! hlist)
        return;

    //
    // get current list box item index, text and rect.
    // abort if the index is the same as the last time
    //

    if (first_time) {

        last_index = LB_ERR;

        if (first_time == 9)
            goto dotooltip;

        if (first_time == 10) {
            if (htip) {
                DestroyWindow(htip);
                htip = NULL;
                tip_x = 0;
                tip_y = 0;
            }
            return;
        }
    }

    lResult = index = SendMessage(hlist, LB_GETCURSEL, 0, 0);
    if (lResult != LB_ERR) {
        if (index == last_index)
            return;
        if (index != dfp_index) {
            if (! GetBoxName(hlist, (int)index, boxname)) {
                PrepareRunAsAdmin(hwnd, (WCHAR *)-1, TRUE);
                lResult = LB_ERR;
            }
        }
    }

    if (lResult == LB_ERR) {
        last_index = LB_ERR;
        goto dotooltip;
    }

    if (last_index == index)
        return;
    last_index = index;

    //
    // hack, update checkbox Run as UAC Administrator
    //

    PrepareRunAsAdmin(hwnd, boxname, TRUE);

    //
    // extract the box text
    //

    if (! boxtext) {
        boxtext = (WCHAR *)HeapAlloc(GetProcessHeap(),
                                     HEAP_GENERATE_EXCEPTIONS,
                                     (2048 + 8) * sizeof(WCHAR));
    }

    if (dfp_index != LB_ERR && index == dfp_index) {

        WCHAR *text = SbieDll_FormatMessage0(MSG_3252);
        wcscpy(boxtext, text);
        LocalFree(text);
        SendMessage(hlist, LB_GETTEXT, (int)index, (LPARAM)boxname);

    } else if (0 != SbieApi_QueryConfAsIs(boxname, L"Description", 0,
                                          boxtext, 2048 * sizeof(WCHAR))) {

        boxtext[0] = L'\0';

        //
        // if running interactively, and there is no description for
        // DefaultBox, then show the tip about holding down the Ctrl key
        //

        if (_wcsicmp(boxname, L"DefaultBox") == 0) {

            STARTUPINFO si;
            si.cb = sizeof(STARTUPINFO);
            GetStartupInfo(&si);

            if (si.dwFlags &
                    (STARTF_USESHOWWINDOW | STARTF_TITLEISLINKNAME)) {

                WCHAR *text = SbieDll_FormatMessage0(MSG_3254);
                wcscpy(boxtext, text);
                LocalFree(text);
            }
        }
    }

    if (boxtext[0])
        just_hide = FALSE;

    if (! just_hide) {

        WCHAR *ptr = boxtext;
        while (ptr) {
            ptr = wcschr(ptr, L'<');
            if (! ptr)
                break;
            if (_wcsnicmp(ptr, L"<br>", 4) != 0) {
                ++ptr;
                continue;
            }
            ptr[0] = L'\r';
            ptr[1] = L'\n';
            wmemmove(ptr + 2, ptr + 4, wcslen(ptr + 4) + 1);
        }
    }

    //
    // create a tooltip control if we don't already have one
    //

dotooltip:

    memzero(&ti, sizeof(ti));
    ti.cbSize = sizeof(ti);
    ti.uId = 1;

    if (htip && last_hlist && hlist != last_hlist) {

        ti.hwnd = last_hlist;
        SendMessage(htip, TTM_DELTOOL, 0, (LPARAM)&ti);
        last_hlist = NULL;
    }

    if (! htip) {

        ULONG exStyle = WS_EX_TOPMOST | WS_EX_TOOLWINDOW;
        if (layout_rtl)
            exStyle |= WS_EX_LAYOUTRTL;
        htip = CreateWindowEx(exStyle, TOOLTIPS_CLASS, NULL,
                              WS_POPUP | TTS_NOPREFIX | TTS_BALLOON |
                              TTS_NOANIMATE | TTS_NOFADE,
                              0, 0, 0, 0, NULL,
                              NULL, GetModuleHandle(NULL), NULL);

        GetWindowRect(GetDesktopWindow(), &rect);
        pt.x = (rect.right - rect.left) / 3;
        SendMessage(htip, TTM_SETMAXTIPWIDTH, 0, pt.x);
    }

    if (! last_hlist) {

        last_hlist = hlist;

        ti.uFlags = TTF_TRACK | TTF_ABSOLUTE;
        ti.hwnd = hlist;
        SendMessage(htip, TTM_ADDTOOL, 0, (LPARAM)&ti);

        tip_x = 0;
        tip_y = 0;
    }

    if ((tip_x == 0 && tip_y == 0) || (first_time == 8)) {

        int top_idx = (int)SendMessage(hlist, LB_GETTOPINDEX, 0, 0);
        if (top_idx == LB_ERR)
            top_idx = 0;
        SendMessage(hlist, LB_GETITEMRECT, top_idx, (LPARAM)&rect);
        if (layout_rtl)
            pt.x = rect.left - GetSystemMetrics(SM_CXHTHUMB);
        else
            pt.x = rect.right + GetSystemMetrics(SM_CXHTHUMB);
        pt.y = rect.top;
        ClientToScreen(hlist, &pt);
        tip_x = pt.x;
        tip_y = pt.y;
    }

    //
    // update the tooltip control
    //

    ti.hwnd = hlist;

    if (first_time != 8)
        SendMessage(htip, TTM_TRACKACTIVATE, FALSE, (LPARAM)&ti);

    AddToolTipForRunAsAdmin(hwnd, htip);

    ti.lpszText = boxtext;
    SendMessage(htip, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
    SendMessage(htip, TTM_SETTITLE, 0, (LPARAM)boxname);

    if (just_hide)
        return;

    SendMessage(htip, TTM_TRACKPOSITION, 0, MAKELONG(tip_x, tip_y));

    if (first_time != 8)
        SendMessage(htip, TTM_TRACKACTIVATE, TRUE, (LPARAM)&ti);
}


//---------------------------------------------------------------------------
// AddBoxOrder
//---------------------------------------------------------------------------


int AddBoxOrder(
        BOX_ORDER_ENTRY *order_entry, HWND hwnd, int indent,
        const WCHAR *compare_text)
{
    WCHAR *text;
    int i, rv = -1;

    text = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, (indent + 256) * sizeof(WCHAR));
    if (! text)
        return -1;

    for (i = 0; i < indent; ++i)
        text[i] = L' ';

    while (order_entry) {

        if (order_entry->children) {

            text[indent] = L'(';
            wcscpy(&text[indent + 1], order_entry->name);
            i = wcslen(text);
            text[i] = L')';
            text[i + 1] = L'\0';

            SendDlgItemMessage(
                    hwnd, IDBOXLIST, LB_ADDSTRING, 0, (LPARAM)text);

            i = AddBoxOrder(order_entry->children, hwnd, indent + 12,
                            compare_text);
            if (rv == -1)
                rv = i;

        } else {

            wcscpy(&text[indent], order_entry->name);

            i = (int)SendDlgItemMessage(
                    hwnd, IDBOXLIST, LB_ADDSTRING, 0, (LPARAM)text);

            if (rv == -1 && i != LB_ERR &&
                            _wcsicmp(order_entry->name, compare_text) == 0)
                rv = i;
        }

        order_entry = order_entry->next;
    }

    HeapFree(GetProcessHeap(), 0, text);
    return rv;
}


//---------------------------------------------------------------------------
// ListBoxProc
//---------------------------------------------------------------------------


INT_PTR ListBoxProc(
    HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_CHAR) {

        WCHAR ch = towlower((WCHAR)wParam);

        WCHAR *text = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, 512 * sizeof(WCHAR));

        int list_index = (int)SendMessage(hwnd, LB_GETCURSEL, 0, 0);
        int list_count = (int)SendMessage(hwnd, LB_GETCOUNT, 0, 0);
        if (list_index < 0)
            list_index = 0;
        if (! text)
            list_count = 0;

        if (list_count > 1) {

            LRESULT lResult;
            int index = list_index;
            do {

                ++index;
                if (index >= list_count)
                    index = 0;

                lResult = SendMessage(hwnd, LB_GETTEXT, index, (LPARAM)text);
                if (lResult != LB_ERR) {

                    WCHAR *ptr = text;
                    while (*ptr == L' ')
                        ++ptr;
                    if (*ptr == L'(')
                        ++ptr;

                    if (towlower(*ptr) == ch) {

                        SendMessage(hwnd, LB_SETCURSEL, index, 0);
                        TrackToolTip(hwnd, 0);
                        return 0;
                    }
                }

            } while (index != list_index);
        }

        if (text)
            HeapFree(GetProcessHeap(), 0, text);
    }

    if (1) {

        WNDPROC proc = (WNDPROC)GetProp(hwnd, L"Ptr6392");
        if (proc)
            return CallWindowProc(proc, hwnd, uMsg, wParam, lParam);
    }

    return 0;
}


//---------------------------------------------------------------------------
// BoxDialogProc
//---------------------------------------------------------------------------


INT_PTR BoxDialogProc(
    HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static BOOL hide_tool_tip;
    static BOOL ok_disabled;
    int index;
    BOOL set_sel, is_box;
    RECT rc;
    int x, y;

    switch (uMsg) {

        //
        // initialize dialog
        //

        case WM_INITDIALOG:

            dfp_index = LB_ERR;

            hide_tool_tip = FALSE;
            ok_disabled   = FALSE;

            //
            // assign text strings
            //

            SetWindowText(hwnd, SbieDll_FormatMessage0(MSG_3052));

            SetDlgItemText(hwnd, IDOK, SbieDll_FormatMessage0(MSG_3001));
            SetDlgItemText(hwnd, IDCANCEL, SbieDll_FormatMessage0(MSG_3002));

            SetDlgItemText(hwnd, IDRUNDLGTEXT,
                           SbieDll_FormatMessage0(MSG_3106));

            //
            // position window in the middle of the screen
            //

            GetClientRect(GetDesktopWindow(), &rc);
            x = (rc.left + rc.right) / 2;
            y = (rc.top + rc.bottom) / 2;
            GetClientRect(hwnd, &rc);
            x -= rc.right / 2;
            y -= rc.bottom / 2;

            SetWindowPos(hwnd, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
            SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hProgramIcon);
            SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)NULL);

            //
            // populate list of sandboxes
            //

            index = -1;
            set_sel = TRUE;

            if (0 == SbieApi_QueryProcess(
                        (HANDLE)(ULONG_PTR)GetCurrentProcessId(),
                        boxname, NULL, NULL, NULL)) {

                SendDlgItemMessage(
                    hwnd, IDBOXLIST, LB_ADDSTRING, 0, (LPARAM)boxname);

            } else {

                //
                // add other sandboxes
                //

                BOX_ORDER_ENTRY *box_order = BoxOrder_Read();
                if (box_order) {
                    if (box_order->children) {
                        index = AddBoxOrder(box_order->children, hwnd, 0,
                                            L"DefaultBox");
                    }
                    BoxOrder_Free(box_order);
                }

                if (index != -1) {
                    SendDlgItemMessage(
                            hwnd, IDBOXLIST, LB_SETCURSEL, index, 0);
                    set_sel = FALSE;
                }

                index = (int)SendDlgItemMessage(
                                        hwnd, IDBOXLIST, LB_GETCOUNT, 0, 0);
                if (index <= 0)
                    EndDialog(hwnd, 0);

                //
                // add "Run Outside Sandbox" option
                //

                if (boxdlg_run_outside_sandbox && CanDisableForce()) {

                    WCHAR *text = SbieDll_FormatMessage0(MSG_3251);
                    for (x = 0; x < 40; ++x)
                        boxname[x] = 8212;      // separator line
                    boxname[40] = 0;
                    SendDlgItemMessage(hwnd,
                        IDBOXLIST, LB_INSERTSTRING, -1, (LPARAM)boxname);
                    dfp_index = (int)SendDlgItemMessage(hwnd,
                        IDBOXLIST, LB_INSERTSTRING, -1, (LPARAM)text);
                }
            }

            if (set_sel)
                SendDlgItemMessage(hwnd, IDBOXLIST, LB_SETCURSEL, 0, 0);

            //
            // set focus to the listbox
            //

            Adjust_Window_Height(hwnd);

            SetFocus(GetDlgItem(hwnd, IDBOXLIST));

            TrackToolTip(hwnd, 1);

            //
            // run as admin
            //

            PrepareRunAsAdmin(hwnd, NULL, FALSE);

            //
            // subclass listbox
            //

            if (1) {

                HWND hListBox = GetDlgItem(hwnd, IDBOXLIST);
                WNDPROC oldproc =
                    (WNDPROC)GetWindowLongPtr(hListBox, GWLP_WNDPROC);
                SetProp(hListBox, L"Ptr6392", oldproc);
                SetWindowLongPtr(
                    hListBox, GWLP_WNDPROC, (ULONG_PTR)ListBoxProc);
            }

            //
            // call ShowWindow twice to the ignore startup nShowCmd flag
            //

            ShowWindow(hwnd, SW_SHOWNORMAL);
            ShowWindow(hwnd, SW_SHOWNORMAL);

            break;

        //
        // if Start.exe started with the SW_MAXIMIZE startup nShowCmd flag,
        // the system will ask what windows size to use for maximize
        //

        case WM_GETMINMAXINFO:

            GetWindowRect(hwnd, &rc);
            if (1) {
                MINMAXINFO *mnmx = (MINMAXINFO *)lParam;
                mnmx->ptMaxSize.x = rc.right - rc.left + 1;
                mnmx->ptMaxSize.y = rc.bottom - rc.top + 1;
                mnmx->ptMaxPosition.x = rc.left;
                mnmx->ptMaxPosition.y = rc.top;
            }
            return 0;

        //
        // handle size/move:
        //

        case WM_DESTROY:
        case WM_KILLFOCUS:
        case WM_SETFOCUS:
        case WM_ACTIVATE:
        case WM_WINDOWPOSCHANGED:

            if (hide_tool_tip ||
                    uMsg == WM_DESTROY || uMsg == WM_KILLFOCUS ||
                    (uMsg == WM_ACTIVATE && LOWORD(wParam) == WA_INACTIVE))
                x = 9;
            else if (uMsg == WM_WINDOWPOSCHANGED)
                x = 8;
            else
                x = 1;

            TrackToolTip(hwnd, x);

            break;

        //
        // handle buttons
        //

        case WM_COMMAND:

            index = (int)SendDlgItemMessage(
                                hwnd, IDBOXLIST, LB_GETCURSEL, 0, 0);
            if (index == LB_ERR)
                is_box = FALSE;
            else
                is_box = GetBoxName(
                            GetDlgItem(hwnd, IDBOXLIST), index, boxname);

            if (LOWORD(wParam) == IDBOXLIST &&
                    HIWORD(wParam) == LBN_SELCHANGE) {

                TrackToolTip(hwnd, 0);

                if (dfp_index != LB_ERR && index == dfp_index)
                    is_box = TRUE;
                if (! is_box) {
                    EnableWindow(GetDlgItem(hwnd, IDOK), FALSE);
                    ok_disabled = TRUE;
                } else if (ok_disabled) {
                    EnableWindow(GetDlgItem(hwnd, IDOK), TRUE);
                    ok_disabled = FALSE;
                }

            } else if (LOWORD(wParam) == IDOK
                        ||  (LOWORD(wParam) == IDBOXLIST &&
                             HIWORD(wParam) == LBN_DBLCLK)) {

                if (dfp_index != LB_ERR && index == dfp_index) {

                    extern WCHAR *Sandboxie_Start_Title;
                    extern BOOL disable_force_on_this_program;
                    int rc;

                    hide_tool_tip = TRUE;

                    rc = MessageBox(hwnd,
                                    SbieDll_FormatMessage0(MSG_3253),
                                    Sandboxie_Start_Title,
                                    MB_ICONQUESTION | MB_YESNO |
                                (layout_rtl ? MB_RTLREADING | MB_RIGHT : 0));

                    if (rc != IDYES) {
                        hide_tool_tip = FALSE;
                        TrackToolTip(hwnd, 1);
                        break;
                    }

                    disable_force_on_this_program = TRUE;
                    boxname[0] = L'\0';

                } else if (! is_box) {

                    break;

                } else {

                    TrackToolTip(hwnd, 10);
                }

                EndDialog(hwnd, 0);
                return TRUE;

            } else if (LOWORD(wParam) == IDCANCEL) {

                boxname[0] = L'\0';     // before destroying window
                EndDialog(hwnd, 0);
                boxname[0] = L'\0';     // after destroying window
                return TRUE;

            } else if (LOWORD(wParam) == IDRUNADMIN) {

                ClickRunAsAdmin(hwnd);
            }

            break;
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// Adjust_Window_Height
//---------------------------------------------------------------------------


void Adjust_Window_Height(HWND hwnd)
{
    static int ControlIDs[] = { IDRUNADMIN, IDOK, IDCANCEL, 0 };
    HWND hlist, hctrl;
    int item_height;
    int num_items;
    int num_visible;
    int max_items;
    int diff;
    int i;
    RECT rc;
    POINT pt;

    //
    // calculate how many items are visible in the list box
    //

    hlist = GetDlgItem(hwnd, IDBOXLIST);
    item_height = (int)SendMessage(hlist, LB_GETITEMHEIGHT, 0, 0);
    GetClientRect(hlist, &rc);

    num_items = (int)SendMessage(hlist, LB_GETCOUNT, 0, 0);
    ++num_items;        // +1 for some spare vertical pixels
    num_visible = (rc.bottom - rc.top + 1) / item_height;

    if (num_visible >= num_items)
        return;

    //
    // extend the height of the listbox to try to make all items
    // visible, but don't take over the entire height of the screen
    //

    GetWindowRect(GetDesktopWindow(), &rc);
    max_items = (rc.bottom - rc.top + 1) / 2 / item_height;
    if (num_items > max_items)
        num_items = max_items;

    diff = (num_items - num_visible) / 2 + 1;

    //
    // extend the top and height of the window
    //

    GetWindowRect(hwnd, &rc);
    rc.top -= diff * item_height;
    pt.y = (rc.bottom + diff * item_height) - rc.top;
    SetWindowPos(hwnd, HWND_TOPMOST,
                 rc.left, rc.top, rc.right - rc.left + 1, pt.y,
                 0);

    //
    // extend the height of the listbox
    //

    GetWindowRect(hlist, &rc);
    rc.bottom += diff * item_height * 2;
    SetWindowPos(hlist, 0, 0, 0,
                 rc.right - rc.left + 1, rc.bottom - rc.top + 1,
                 SWP_NOMOVE);

    //
    // move all other controls towards the new bottom of the window
    //

    for (i = 0; ControlIDs[i]; ++i) {
        hctrl = GetDlgItem(hwnd, ControlIDs[i]);
        GetWindowRect(hctrl, &rc);
        pt.x = rc.left;
        pt.y = rc.top + diff * item_height * 2;
        ScreenToClient(hwnd, &pt);
        SetWindowPos(hctrl, 0,
                     pt.x, pt.y,
                     rc.right - rc.left + 1, rc.bottom - rc.top + 1,
                     0);
    }
}


//---------------------------------------------------------------------------
// DoBoxDialog
//---------------------------------------------------------------------------


WCHAR *DoBoxDialog(void)
{
    HINSTANCE hInstance = GetModuleHandle(NULL);
    INT_PTR r;

    if (! initialized) {

        INITCOMMONCONTROLSEX icc;
        icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icc.dwICC = ICC_STANDARD_CLASSES | ICC_TAB_CLASSES;
        InitCommonControlsEx(&icc);

        hProgramIcon = (HICON)LoadImage(
            hInstance, MAKEINTRESOURCE(IDICON), IMAGE_ICON,
            0, 0, LR_DEFAULTSIZE);

        initialized = TRUE;
    }

    boxname[0] = L'\0';

    if (layout_rtl) {

        LPCDLGTEMPLATE tmpl = (LPCDLGTEMPLATE)Common_DlgTmplRtl(
                                    hInstance, MAKEINTRESOURCE(BOX_DIALOG));
        r = DialogBoxIndirectParam(hInstance, tmpl, NULL,
                           BoxDialogProc, (LPARAM)hInstance);

    } else {

        r = DialogBoxParam(hInstance, MAKEINTRESOURCE(BOX_DIALOG), NULL,
                           BoxDialogProc, (LPARAM)hInstance);
    }

    return boxname;
}
