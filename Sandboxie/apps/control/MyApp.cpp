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
// Application
//---------------------------------------------------------------------------


#include "stdafx.h"
#include "MyApp.h"

#include "MyFrame.h"
#include "Boxes.h"
#include "WindowTitleMap.h"
#include "UserSettings.h"
#include "SbieIni.h"
#include "InitWait.h"
#include "ShellDialog.h"
#include "AutoPlay.h"
#include "apps/common/MyMsgBox.h"
#include "apps/common/MyGdi.h"
#include "apps/common/CommonUtils.h"
#include "apps/common/RunStartExe.h"
#include "common/win32_ntddk.h"
#include "common/my_version.h"
#include "core/drv/api_defs.h"


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


CMyApp theApp;

bool CMyApp::m_Windows2000  = false;
bool CMyApp::m_WindowsVista = false;
bool CMyApp::m_LayoutRTL    = false;

HBRUSH CMyApp::m_background = NULL;

CFont CMyApp::m_fontFixed;

ATOM CMyApp::m_atom = NULL;

UINT CMyApp::m_TaskbarCreated = 0;

HWND CMyApp::m_hwndTray = NULL;

int CMyApp::m_modal = 0;

CString CMyApp::m_appTitle;

ULONG CMyApp::m_session_id = 0;

const CString _HideMessage(L"HideMessage");
const CString _GlobalSettings(L"GlobalSettings");
const CString _ExplorerExe(L"explorer.exe");


//---------------------------------------------------------------------------
// InitInstance
//---------------------------------------------------------------------------


BOOL CMyApp::InitInstance()
{
    static const WCHAR *WindowClassName = SANDBOXIE_CONTROL L"WndClass";

    BOOL ForceVisible = FALSE;
    BOOL ForceSync    = FALSE;
    WCHAR *CommandLine = GetCommandLine();
    if (CommandLine) {
        if (wcsstr(CommandLine, L"/open"))
            ForceVisible = TRUE;
        if (wcsstr(CommandLine, L"/sync"))
            ForceSync    = TRUE;
        if (wcsstr(CommandLine, L"/uninstall")) {
            CShellDialog::Sync(TRUE);
            return TRUE;
        }
    }

    //
    // abort early if the application mutex already exists
    //

    WCHAR *InstanceMutexName = SANDBOXIE L"_SingleInstanceMutex_Control";
    HANDLE hInstanceMutex =
        OpenMutex(MUTEX_ALL_ACCESS, FALSE, InstanceMutexName);
    if (hInstanceMutex) {
        if (ForceVisible) {
            HWND hwnd = FindWindow(WindowClassName, NULL);
            if (hwnd) {
                ShowWindow(hwnd, SW_SHOWNORMAL);
                SetForegroundWindow(hwnd);
            }
        }
        return FALSE;
    }
    hInstanceMutex = CreateMutex(NULL, FALSE, InstanceMutexName);
    if (! hInstanceMutex)
        return FALSE;

    //
    // change to Sandboxie home directory
    //

    WCHAR *home_path = (WCHAR *)HeapAlloc(
        GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, 1024 * sizeof(WCHAR));
    SbieApi_GetHomePath(NULL, 0, home_path, 1020);
    SetCurrentDirectory(home_path);
    HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, home_path);

    //
    // check if Windows XP APIs are available
    //

    OSVERSIONINFO osvi;
    memzero(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx((OSVERSIONINFO *)&osvi);
    if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
        m_Windows2000 = true;
    if (osvi.dwMajorVersion >= 6)
        m_WindowsVista = true;

    //
    // initialize misc
    //

    CoInitialize(NULL);

// these have been deprecated
//#ifdef _AFXDLL
//    Enable3dControls();
//#else
//    Enable3dControlsStatic();
//#endif

    BOOLEAN LayoutRTL;
    SbieDll_GetLanguage(&LayoutRTL);
    if (LayoutRTL)
        m_LayoutRTL = true;

    m_appTitle = CMyMsg(MSG_3301);

#if _MSC_VER != 1200        // if not Visual C++ 6.0
    if (! ProcessIdToSessionId(GetCurrentProcessId(), &m_session_id))
        m_session_id = 0;
#endif

    SbieDll_GetDrivePath(-1);

    //
    // initialize graphical elements
    //

    MyGdi_Init();

    m_background =
        CreatePatternBrush(MyGdi_CreateFromResource(L"BACKGROUND"));

    if (1) {

        CFont font1;
        LOGFONT data1;
        font1.CreateStockObject(ANSI_FIXED_FONT);
        if (font1.GetLogFont(&data1)) {

            HDC hdc = GetDC(NULL);
            LONG NewHeight =
                -MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72);
            ReleaseDC(NULL, hdc);
            if (abs(NewHeight) > abs(data1.lfHeight)) {
                data1.lfHeight = NewHeight;
                data1.lfWidth = 0;
            }

            HDC hDC = ::GetDC(NULL);
            int dpi = GetDeviceCaps(hDC, LOGPIXELSY);
            if (dpi > 96)
                data1.lfWeight = FW_BOLD;
            ::ReleaseDC(NULL, hDC);

            m_fontFixed.CreateFontIndirect(&data1);

        } else
            m_fontFixed.CreateStockObject(ANSI_FIXED_FONT);
    }

    //
    // register main window class
    //

    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_GLOBALCLASS;
    wc.lpfnWndProc = ::DefWindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = AfxGetInstanceHandle();
    wc.hIcon = ::LoadIcon(wc.hInstance, L"AAAPPICON");
    wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = WindowClassName;
    wc.hIconSm = NULL;

    m_atom = RegisterClassEx(&wc);
    if (! m_atom)
        return FALSE;

    //
    // register TaskbarCreated message
    //

    m_TaskbarCreated = RegisterWindowMessage(L"TaskbarCreated");

    //
    // wait for Sandboxie to finish initialization
    //

    CInitWait initwait(this);

    //
    // take over as the Sandboxie session leader
    //

    SbieApi_SessionLeader(0, NULL);

    //
    // refresh processes
    //

    CBoxes::GetInstance().RefreshProcesses();

    //
    // setup autoplay cancellation
    //

    CAutoPlay::Install();

    //
    // create main window
    //

    m_pMainWnd = new CMyFrame(ForceVisible, ForceSync);
    m_pMainWnd->UpdateWindow();

    return TRUE;
}


//---------------------------------------------------------------------------
// ExitInstance
//---------------------------------------------------------------------------


int CMyApp::ExitInstance()
{
    CAutoPlay::Remove();
    delete &CBoxes::GetInstance();
    delete &CWindowTitleMap::GetInstance();
    delete &CUserSettings::GetInstance();
    delete &CSbieIni::GetInstance();

    return 0;
}


//---------------------------------------------------------------------------
// RunStartExe
//---------------------------------------------------------------------------


void CMyApp::RunStartExe(
    const CString &cmd, const CString &box, BOOL wait, BOOL inherit)
{
    Common_RunStartExe(cmd, box, wait, inherit);
}


//---------------------------------------------------------------------------
// MsgBox
//---------------------------------------------------------------------------


int CMyApp::MsgBox(CWnd *pWnd, const WCHAR *text, UINT flags)
{
    if (! pWnd)
        pWnd = CBaseDialog::GetActiveWindow();
    HWND hwnd = pWnd ? pWnd->m_hWnd : NULL;
    CString caption;
    if (hwnd)
        pWnd->GetWindowText(caption);
    else
        caption = CMyApp::m_appTitle;
    if (m_LayoutRTL)
        flags |= MB_RTLREADING | MB_RIGHT;

    return MessageBox(hwnd, text, caption, flags);
}


//---------------------------------------------------------------------------
// MsgBox
//---------------------------------------------------------------------------


int CMyApp::MsgBox(CWnd *pWnd, ULONG textid, UINT flags)
{
    return MsgBox(pWnd, CMyMsg(textid), flags);
}


//---------------------------------------------------------------------------
// MsgCheckBox
//---------------------------------------------------------------------------


int CMyApp::MsgCheckBox(
    CWnd *pWnd, const CString &text, ULONG checkid, UINT flags)
{
    if (! pWnd)
        pWnd = CBaseDialog::GetActiveWindow();
    HWND hwnd = pWnd ? pWnd->m_hWnd : NULL;
    CString caption;
    if (hwnd)
        pWnd->GetWindowText(caption);
    else
        caption = CMyApp::m_appTitle;
    if (m_LayoutRTL)
        flags |= MB_RTLREADING | MB_RIGHT;

    CMyMsg check(checkid ? checkid : MSG_3306);
    return MessageCheckBox(hwnd, text, caption, flags, check);
}


//---------------------------------------------------------------------------
// MsgCheckBox
//---------------------------------------------------------------------------


int CMyApp::MsgCheckBox(
    CWnd *pWnd, ULONG textid, ULONG checkid, UINT flags)
{
    CMyMsg textstr(textid);
    return MsgCheckBox(pWnd, textstr, checkid, flags);
}


//---------------------------------------------------------------------------
// InputBox
//---------------------------------------------------------------------------


CString CMyApp::InputBox(CWnd *pWnd, ULONG textid)
{
    ULONG flags = MB_OKCANCEL;
    if (m_LayoutRTL)
        flags |= MB_RTLREADING | MB_RIGHT;
    if (! pWnd)
        pWnd = CBaseDialog::GetActiveWindow();
    HWND hwnd = pWnd ? pWnd->m_hWnd : NULL;
    CMyMsg caption(textid);
    WCHAR *edit = ::InputBox(hwnd, caption, flags, NULL);
    CString retval;
    if (edit) {
        retval = edit;
        LocalFree(edit);
    }
    return retval;
}


//---------------------------------------------------------------------------
// InputBox
//---------------------------------------------------------------------------


void CMyApp::InputBox(CWnd *pWnd, ULONG textid, CString &str)
{
    ULONG flags = MB_OKCANCEL;
    if (m_LayoutRTL)
        flags |= MB_RTLREADING | MB_RIGHT;
    if (! pWnd)
        pWnd = CBaseDialog::GetActiveWindow();
    HWND hwnd = pWnd ? pWnd->m_hWnd : NULL;
    CMyMsg caption(textid);
    WCHAR *edit = ::InputBox(hwnd, caption, flags, str);
    if (edit) {
        str = edit;
        LocalFree(edit);
    }
}


//---------------------------------------------------------------------------
// GetProgramFileName
//---------------------------------------------------------------------------


void CMyApp::GetProgramFileName(
    CWnd *pParentWnd, CString &pgm1, CString &pgm2)
{
    pgm1 = CString();
    pgm2 = CString();

    WCHAR buf[MAX_PATH + 8];
    buf[0] = L'\0';

    WCHAR filter[64];
    wcscpy(filter, CMyMsg(MSG_3308));
    while (1) {
        WCHAR *ptr = wcsrchr(filter, L'#');
        if (! ptr)
            break;
        *ptr = L'\0';
    }

    OPENFILENAME ofn;
    memzero(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize  = sizeof(OPENFILENAME);
    ofn.hwndOwner    = pParentWnd->m_hWnd;
    ofn.nFilterIndex = 1;
    ofn.lpstrFilter  = filter;
    ofn.lpstrFile    = buf;
    ofn.nMaxFile     = MAX_PATH;
    ofn.Flags        = OFN_DONTADDTORECENT
                     | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (GetOpenFileName(&ofn)) {

        pgm1 = buf;
        int index = pgm1.ReverseFind(L'\\');
        if (index != -1)
            pgm1 = pgm1.Mid(index + 1);
        pgm1.MakeLower();

        DWORD retval = GetShortPathName(buf, buf, MAX_PATH + 4);
        if (retval && retval <= MAX_PATH) {

            pgm2 = buf;
            index = pgm2.ReverseFind(L'\\');
            if (index != -1) {
                pgm2 = pgm2.Mid(index + 1);
                pgm2.MakeLower();
            }
            if (index == -1 || pgm1 == pgm2)
                pgm2 = CString();
        }
    }
}


//---------------------------------------------------------------------------
// MyLoadMenu
//---------------------------------------------------------------------------


CMenu *CMyApp::MyLoadMenu(const WCHAR *MenuResName)
{
    CMenu *pMenu1;
    MENUITEMINFO mii;
    WCHAR text[128];
    int i, j, k;
    UINT id;

    //
    // create and load menu
    //

    pMenu1 = new CMenu();
    pMenu1->LoadMenu(MenuResName);

    //
    // convert message IDs to localized text
    //

    for (i = 0; i < (int)pMenu1->GetMenuItemCount(); ++i) {

        memzero(&mii, sizeof(MENUITEMINFO));
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_ID | MIIM_STRING | MIIM_SUBMENU;
        mii.dwTypeData = text;
        mii.cch = 120;

        ::GetMenuItemInfo(*pMenu1, i, TRUE, &mii);
        id = _wtoi(text);
        if (id) {

            CMyMsg msg(id);
            mii.dwTypeData = (WCHAR *)(const WCHAR *)msg;
            ::SetMenuItemInfo(*pMenu1, i, TRUE, &mii);
        }

        if (mii.hSubMenu) {

            CMenu *pMenu2 = pMenu1->GetSubMenu(i);

            for (j = 0; j < (int)pMenu2->GetMenuItemCount(); ++j) {

                memzero(&mii, sizeof(MENUITEMINFO));
                mii.cbSize = sizeof(MENUITEMINFO);
                mii.fMask = MIIM_ID | MIIM_STRING | MIIM_SUBMENU;
                mii.dwTypeData = text;
                mii.cch = 120;

                ::GetMenuItemInfo(*pMenu2, j, TRUE, &mii);
                id = _wtoi(text);
                if (id) {

                    CMyMsg msg(id, L"\t");
                    mii.dwTypeData = (WCHAR *)(const WCHAR *)msg;
                    ::SetMenuItemInfo(*pMenu2, j, TRUE, &mii);
                }

                if (mii.hSubMenu) {

                    CMenu *pMenu3 = pMenu2->GetSubMenu(j);

                    for (k = 0; k < (int)pMenu3->GetMenuItemCount(); ++k) {

                        memzero(&mii, sizeof(MENUITEMINFO));
                        mii.cbSize = sizeof(MENUITEMINFO);
                        mii.fMask = MIIM_ID | MIIM_STRING | MIIM_SUBMENU;
                        mii.dwTypeData = text;
                        mii.cch = 120;

                        ::GetMenuItemInfo(*pMenu3, k, TRUE, &mii);
                        id = _wtoi(text);
                        if (id) {

                            CMyMsg msg(id, L"\t");
                            mii.dwTypeData = (WCHAR *)(const WCHAR *)msg;
                            ::SetMenuItemInfo(*pMenu3, k, TRUE, &mii);
                        }
                    }
                }
            }
        }
    }

    return pMenu1;
}


//---------------------------------------------------------------------------
// CopyListBoxToClipboard
//---------------------------------------------------------------------------


void CMyApp::CopyListBoxToClipboard(CWnd *wnd, int idListBox)
{
    CListBox *listbox = (CListBox *)(wnd->GetDlgItem(idListBox));
    ULONG count = listbox->GetCount();

    ULONG len = 0;
    ULONG i;

    for (i = 0; i < count; ++i)
        len += listbox->GetTextLen(i) + 2;

    if (len) {

        HANDLE hGlobal =
                    GlobalAlloc(GMEM_MOVEABLE, (len + 4) * sizeof(WCHAR));
        if (hGlobal) {

            WCHAR *mem_ptr = (WCHAR *)GlobalLock(hGlobal);

            for (i = 0; i < count; ++i) {
                len = listbox->GetTextLen(i);
                listbox->GetText(i, mem_ptr);
                mem_ptr += len;
                *mem_ptr = L'\r';
                ++mem_ptr;
                *mem_ptr = L'\n';
                ++mem_ptr;
            }

            *mem_ptr = L'\0';
            GlobalUnlock(hGlobal);

            if (wnd->OpenClipboard()) {

                EmptyClipboard();
                SetClipboardData(CF_UNICODETEXT, hGlobal);
                CloseClipboard();

            } else
                GlobalFree(hGlobal);
        }
    }
}


//---------------------------------------------------------------------------
// CreateTrayIcon
//---------------------------------------------------------------------------


void CMyApp::CreateTrayIcon(CWnd *pParentWnd, HICON hIcon, CString info)
{
    m_hwndTray = pParentWnd->m_hWnd;

    for (int retry = 0; retry < 60; ++retry) {

        NOTIFYICONDATA nid;
        memzero(&nid, sizeof(NOTIFYICONDATA));
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = m_hwndTray;
        nid.uID = 1;
        nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
        nid.uCallbackMessage = WM_TRAYICON;
        nid.hIcon = hIcon;
        wcscpy(nid.szTip, CMyApp::m_appTitle);
        wcscat(nid.szTip, info);

        BOOL ok = Shell_NotifyIcon(NIM_ADD, &nid);
        if (ok)
            return;

        Sleep(500);
    }
}


//---------------------------------------------------------------------------
// ChangeTrayIcon
//---------------------------------------------------------------------------


void CMyApp::ChangeTrayIcon(HICON hIcon, CString info)
{
    if (m_hwndTray) {

        NOTIFYICONDATA nid;
        memzero(&nid, sizeof(NOTIFYICONDATA));
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = m_hwndTray;
        nid.uID = 1;
        nid.uFlags = NIF_ICON | NIF_TIP;
        nid.hIcon = hIcon;
        wcscpy(nid.szTip, CMyApp::m_appTitle);
        wcscat(nid.szTip, info);
        Shell_NotifyIcon(NIM_MODIFY, &nid);
    }
}


//---------------------------------------------------------------------------
// BalloonTrayIcon
//---------------------------------------------------------------------------


void CMyApp::BalloonTrayIcon(CString text, ULONG timeout)
{
    if (m_hwndTray) {

        NOTIFYICONDATA nid;
        memzero(&nid, sizeof(NOTIFYICONDATA));
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = m_hwndTray;
        nid.uID = 1;
        nid.uFlags = NIF_INFO;
        wcscpy(nid.szInfoTitle, CMyApp::m_appTitle);
        wcscpy(nid.szInfo, text);
        nid.dwInfoFlags = NIIF_NOSOUND;
        nid.uTimeout = timeout * 1000;
        Shell_NotifyIcon(NIM_MODIFY, &nid);
    }
}


//---------------------------------------------------------------------------
// DeleteTrayIcon
//---------------------------------------------------------------------------


void CMyApp::DeleteTrayIcon()
{
    if (m_hwndTray) {

        NOTIFYICONDATA nid;
        memzero(&nid, sizeof(NOTIFYICONDATA));
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = m_hwndTray;
        nid.uID = 1;
        nid.uFlags = 0;
        Shell_NotifyIcon(NIM_DELETE, &nid);

        m_hwndTray = NULL;
    }
}
