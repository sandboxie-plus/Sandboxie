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


#ifndef _MY_APP_H
#define _MY_APP_H


#include "stdafx.h"
#include "resource.h"
#include "apps/common/MyMsg.h"
#include "BaseDialog.h"


#define WM_TRAYICON     (WM_APP + 7)


extern const CString _HideMessage;
extern const CString _GlobalSettings;
extern const CString _RunAsAdmin;
extern const CString _ExplorerExe;


class CMyApp : public CWinApp
{
public:

    BOOL InitInstance();
    int  ExitInstance();

    //
    // global variables
    //

    static bool m_Windows2000;
    static bool m_WindowsVista;
    static bool m_LayoutRTL;

    static HBRUSH m_background;

    static CFont m_fontFixed;

    static HICON m_icon;
    static ATOM m_atom;

    static HWND m_hwndTray;

    static int m_modal;

    static CString m_appTitle;

    static ULONG m_session_id;

    //
    // global utilities
    //

    static void RunStartExe(
        const CString &cmd, const CString &box,
        BOOL wait = FALSE, BOOL inherit = FALSE);

    static int MsgBox(CWnd *pWnd, const WCHAR *text, UINT flags);

    static int MsgBox(CWnd *pWnd, ULONG textid, UINT flags);

    static int MsgCheckBox(
        CWnd *pWnd, const CString &text, ULONG checkid, UINT flags);

    static int MsgCheckBox(
        CWnd *pWnd, ULONG textid, ULONG checkid, UINT flags);

    static CString InputBox(CWnd *pWnd, ULONG textid);
    static void InputBox(CWnd *pWnd, ULONG textid, CString &str);

    static void GetProgramFileName(
        CWnd *pParentWnd, CString &pgm1, CString &pgm2);

    static CMenu *MyLoadMenu(const WCHAR *MenuResName);

    static void CopyListBoxToClipboard(CWnd *wnd, int idListBox);

    //
    // tray icon
    //

    static UINT m_TaskbarCreated;

    static void CreateTrayIcon(CWnd *pParentWnd, HICON hIcon, CString info);
    static void ChangeTrayIcon(HICON hIcon, CString info);
    static void BalloonTrayIcon(CString text, ULONG timeout);
    static void DeleteTrayIcon();
};


#endif // _MY_APP_H
