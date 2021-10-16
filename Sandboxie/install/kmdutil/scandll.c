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


#include "stdafx.h"

#include "common/win32_ntddk.h"
#include "common/defines.h"
#include "common/my_version.h"
#include "core/dll/sbiedll.h"


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _PGM {

    ULONG process_id;
    ULONG session_id;
    WCHAR image[128];
    BOOLEAN skip;

} PGM;


typedef struct _NONCLIENTMETRICS_XP {

    UINT        cbSize;
    int         iBorderWidth;
    int         iScrollWidth;
    int         iScrollHeight;
    int         iCaptionWidth;
    int         iCaptionHeight;
    LOGFONTW    lfCaptionFont;
    int         iSmCaptionWidth;
    int         iSmCaptionHeight;
    LOGFONTW    lfSmCaptionFont;
    int         iMenuWidth;
    int         iMenuHeight;
    LOGFONTW    lfMenuFont;
    LOGFONTW    lfStatusFont;
    LOGFONTW    lfMessageFont;

} NONCLIENTMETRICS_XP;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static void EnableDebugPriv(void);
static void Kmd_ScanDll_2(ULONG *pids, PGM *pgms);
static ULONG Kmd_DoWindow(WCHAR *text, PGM *pgms, BOOLEAN silent);
static LRESULT Kmd_WindowProc(
                    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern void Kmd_Stop_Host_Injected_Svcs();

//---------------------------------------------------------------------------
// Kmd_ScanDll
//---------------------------------------------------------------------------


_FX void Kmd_ScanDll(BOOLEAN silent)
{
    ULONG *pids;
    PGM *pgms;
    ULONG len;
    ULONG exitcode = -1;

    EnableDebugPriv();

    pids = HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, 262144);
    pgms = HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, 1048576);

    while (exitcode == -1) {

        memzero(pids, 262144);
        memzero(pgms, 1048576);

        if (EnumProcesses(pids, 16384, &len))
            len /= sizeof(ULONG);
        else
            len = 0;

        pids[len] = -1;
        Kmd_ScanDll_2(pids, pgms);

        if (pgms[0].process_id)
            exitcode = Kmd_DoWindow((WCHAR *)pids, pgms, silent);
        else
            exitcode = 0;
    }

    HeapFree(GetProcessHeap(), 0, pgms);
    HeapFree(GetProcessHeap(), 0, pids);
    ExitProcess(exitcode);
}


//---------------------------------------------------------------------------
// Kmd_ScanDll_2
//---------------------------------------------------------------------------


_FX void Kmd_ScanDll_2(ULONG *pids, PGM *pgms)
{
    ULONG i, j, z;
    HANDLE hProcess;
    ULONG CurrentProcessId;
    HMODULE *mods;
    ULONG len;
    WCHAR name[64];

    z = 0;
    CurrentProcessId = GetCurrentProcessId();
    mods = HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, 262144);

    for (i = 0; pids[i] != -1; ++i) {

        if (pids[i] == CurrentProcessId || pids[i] <= 8)
            continue;

        hProcess = OpenProcess(
            PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pids[i]);
        if (! hProcess)
            continue;

        j = 0;
#ifdef _WIN64
        if (! j) {
            typedef BOOL (*P_EnumProcessModulesEx)(
                HANDLE hProcess, HMODULE *lphModule, DWORD cb,
                LPDWORD lpcbNeeded, DWORD dwFilterFlag);
            static P_EnumProcessModulesEx pEnumProcessModulesEx = NULL;
            if (! pEnumProcessModulesEx)
                pEnumProcessModulesEx = (P_EnumProcessModulesEx)
                    GetProcAddress(GetModuleHandle(L"psapi.dll"),
                                   "EnumProcessModulesEx");
            if (pEnumProcessModulesEx) {
                j = pEnumProcessModulesEx(
                        hProcess, mods, 262144, &len, LIST_MODULES_ALL);
            }
        }
#endif _WIN64
        if (! j)
            j = EnumProcessModules(hProcess, mods, 262144, &len);

        if (j) {

            len /= sizeof(HMODULE);
            for (j = 0; j < len; ++j) {

                if (GetModuleBaseName(hProcess, mods[j], name, 60)) {

                    if (_wcsicmp(name, SBIEDLL L".dll") == 0) {

                        pgms[z].process_id = pids[i];

                        if (! ProcessIdToSessionId(
                                pgms[z].process_id, &pgms[z].session_id))
                            pgms[z].session_id = 0;

                        if (! GetModuleBaseName(
                                hProcess, mods[0], pgms[z].image, 124)) {
                            pgms[z].image[0] = L'?';
                            pgms[z].image[1] = L'\0';
                        }

                        ++z;
                    }
                }
            }
        }

        CloseHandle(hProcess);
    }

    HeapFree(GetProcessHeap(), 0, mods);
    pgms[z].process_id = 0;
}


//---------------------------------------------------------------------------
// EnableDebugPriv
//---------------------------------------------------------------------------


_FX void EnableDebugPriv(void)
{
    WCHAR priv_space[64];
    TOKEN_PRIVILEGES *privs = (TOKEN_PRIVILEGES *)priv_space;
    HANDLE hToken;

    BOOL b = LookupPrivilegeValue(
                L"", SE_DEBUG_NAME, &privs->Privileges[0].Luid);
    if (b) {

        privs->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        privs->PrivilegeCount = 1;

        b = OpenProcessToken(
                GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken);
        if (b) {

            b = AdjustTokenPrivileges(hToken, FALSE, privs, 0, NULL, NULL);
            CloseHandle(hToken);
        }
    }
}


//---------------------------------------------------------------------------
// Kmd_DoWindow
//---------------------------------------------------------------------------


_FX ULONG Kmd_DoWindow(WCHAR *text, PGM *pgms, BOOLEAN silent)
{
    static ATOM atom = 0;
    RECT rc;
    int w, h, x, y, xOk, xCancel;
    HWND hWnd, hStatic, hList, hBtnOk, hBtnCancel;
    MSG msg;
    NONCLIENTMETRICS_XP ncm;
    HFONT hFontVar, hFontFixed;
    ULONG i;
    BOOL b, OkClicked;
    BOOLEAN rtl;

    //
    // get fonts
    //

    memzero(&ncm, sizeof(ncm));
    ncm.cbSize = sizeof(ncm);
    if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0))
        hFontVar = CreateFontIndirect(&ncm.lfMessageFont);
    else
        hFontVar = NULL;
    if (! hFontVar)
        hFontVar = GetStockObject(SYSTEM_FONT);

    hFontFixed = GetStockObject(ANSI_FIXED_FONT);

    //
    // create window
    //

    if (! atom) {

        WNDCLASS wc;
        memzero(&wc, sizeof(WNDCLASS));
        wc.lpfnWndProc = Kmd_WindowProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.hIcon = LoadIcon(NULL, IDI_INFORMATION);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.lpszClassName = SANDBOXIE L"SandboxieKmdUtilWindow";
        atom = RegisterClass(&wc);
    }

    GetWindowRect(GetDesktopWindow(), &rc);
    w = rc.right - rc.left;
    if (w >= 800) {
        w /= 2;
        x = w / 2;
    } else {
        w -= 32;
        x = 0;
    }
    h = rc.bottom - rc.top;
    if (h >= 600) {
        h /= 2;
        y = h / 2;
    } else {
        h -= 32;
        y = 0;
    }

    OkClicked = FALSE;

    hWnd = CreateWindowEx(
        WS_EX_APPWINDOW, (LPCWSTR)atom, SANDBOXIE,
        WS_POPUPWINDOW | WS_CAPTION,
        x, y, w, h, NULL, NULL, NULL, &OkClicked);

    //
    // create static text
    //

    SbieDll_GetLanguage(&rtl);
    i = rtl ? WS_EX_LAYOUTRTL : 0;

    hStatic = CreateWindowEx(
        i, L"STATIC", SbieDll_FormatMessage0(8106),
        SS_LEFT | WS_CHILD | WS_VISIBLE,
        5, 5, w - 15, 40, hWnd, NULL, NULL, NULL);

    SendMessage(hStatic, WM_SETFONT, (WPARAM)hFontVar, FALSE);

    //
    // create buttons
    //

    wcscpy(text, SbieDll_FormatMessage0(3001));
    for (i = 0; i < wcslen(text); ++i)
        if (text[i] == L'&')
            wmemmove(text + i, text + i + 1, wcslen(text + i));

    xOk = w / 2 - 150;
    xCancel = w / 2 + 50;
    y = h - 65;
    if (rtl) {
        x = xOk;
        xOk = xCancel;
        xCancel = x;
    }

    hBtnOk = CreateWindowEx(
        0, L"BUTTON", text,
        BS_PUSHBUTTON | WS_CHILD | WS_TABSTOP | WS_VISIBLE,
        xOk, y, 100, 25, hWnd, (HMENU)IDOK, NULL, NULL);

    SendMessage(hBtnOk, WM_SETFONT, (WPARAM)hFontVar, FALSE);

    wcscpy(text, SbieDll_FormatMessage0(3002));
    for (i = 0; i < wcslen(text); ++i)
        if (text[i] == L'&')
            wmemmove(text + i, text + i + 1, wcslen(text + i));

    hBtnCancel = CreateWindowEx(
        0, L"BUTTON", text,
        BS_PUSHBUTTON | WS_CHILD | WS_TABSTOP | WS_VISIBLE,
        xCancel, y, 100, 25, hWnd, (HMENU)IDCANCEL, NULL, NULL);

    SendMessage(hBtnCancel, WM_SETFONT, (WPARAM)hFontVar, FALSE);

    //
    // create list box and populate it
    //

    hList = CreateWindowEx(
        0, L"LISTBOX", NULL,
        LBS_NOINTEGRALHEIGHT | LBS_NOSEL
            | WS_HSCROLL | WS_VSCROLL | WS_BORDER
            | WS_CHILD | WS_TABSTOP | WS_VISIBLE,
        5, 45, w - 15, h - 115, hWnd, NULL, NULL, NULL);

    SendMessage(hList, WM_SETFONT, (WPARAM)hFontFixed, FALSE);

    SendMessage(hList, LB_SETHORIZONTALEXTENT, 800, 0);

    b = FALSE;

    for (i = 0; pgms[i].process_id; ++i) {

        pgms[i].skip = TRUE;

        if (_wcsicmp(pgms[i].image, SBIESVC_EXE) == 0)
            continue;

        if (_wcsicmp(pgms[i].image, SBIECTRL_EXE) == 0) {
            ULONG sessid;
            if (ProcessIdToSessionId(GetCurrentProcessId(), &sessid))
                if (pgms[i].session_id == sessid)
                    continue;
        }

        swprintf(text, L"%-48.48s  PID %5d  Session %d",
                 pgms[i].image, pgms[i].process_id, pgms[i].session_id);
        SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)text);

        pgms[i].skip = FALSE;

        b = TRUE;
    }

    if (! b)
        return 0;

    if (silent)
        goto terminate_and_return;

    //
    // show window and do message loop
    //

    ShowWindow(hWnd, SW_SHOWNORMAL);

    SetFocus(hBtnOk);

    while (1) {

        if (GetMessage(&msg, hWnd, 0, 0) <= 0)
            break;

        if (msg.message == WM_KEYDOWN) {

            if (msg.wParam == VK_TAB) {
                HWND hWndNew = GetFocus();
                if (hWndNew && (GetParent(hWndNew) == hWnd)) {
                    hWndNew = GetWindow(hWndNew, GW_HWNDNEXT);
                    if (hWndNew) {
                        ULONG style = GetWindowLong(hWndNew, GWL_STYLE);
                        if (! (style & WS_TABSTOP))
                            hWndNew = NULL;
                    }
                } else
                    hWndNew = NULL;
                if (! hWndNew)
                    hWndNew = hBtnOk;
                SetFocus(hWndNew);
            }

            if (msg.wParam == VK_ESCAPE)
                break;

            if (msg.wParam == VK_RETURN || msg.wParam == VK_SPACE) {
                HWND hWndFocus = GetFocus();
                if (hWndFocus != hBtnCancel)
                    OkClicked = TRUE;
                break;
            }
        }

        DispatchMessage(&msg);
    }

    DestroyWindow(hWnd);

    //
    // terminate programs if the OK button was clicked
    //

    if (! OkClicked)
        return 1;

terminate_and_return:

    // We don't want to call TerminateProcess on any host services that have been injected. It will create an event log entry and immediate restart.
    Kmd_Stop_Host_Injected_Svcs();

    for (i = 0; pgms[i].process_id; ++i) {
        if (! pgms[i].skip) {
            HANDLE hProcess = OpenProcess(
                PROCESS_TERMINATE, FALSE, pgms[i].process_id);
            if (hProcess) {
                TerminateProcess(hProcess, 1);
                CloseHandle(hProcess);
            }
        }
    }

    Sleep(1000);
    return -1;
}


//---------------------------------------------------------------------------
// Kmd_WindowProc
//---------------------------------------------------------------------------


_FX LRESULT Kmd_WindowProc(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static BOOL *pOkClicked = NULL;
    if (msg == WM_CREATE) {
        CREATESTRUCT *cs = (CREATESTRUCT *)lParam;
        pOkClicked = cs->lpCreateParams;
        return 0;
    }
    if (msg == WM_COMMAND && (wParam == IDOK || wParam == IDCANCEL)) {
        if (wParam == IDOK && pOkClicked)
            *pOkClicked = TRUE;
        PostMessage(hWnd, WM_KEYDOWN, VK_ESCAPE, 0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}
