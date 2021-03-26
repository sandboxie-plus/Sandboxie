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
// Initialization Wait
//---------------------------------------------------------------------------


#include "stdafx.h"
#include "MyApp.h"
#include "InitWait.h"

#include "core/svc/SbieIniWire.h"
#include "common/my_version.h"
#include "common/win32_ntddk.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define ID_TIMER    1003


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CInitWait, CWnd)

    ON_COMMAND(ID_EXIT,                     OnDestroy)
    ON_COMMAND(ID_SHOW_ERRORS,              OnShowErrors)

    ON_WM_DESTROY()
    ON_WM_TIMER()

    ON_REGISTERED_MESSAGE(CMyApp::m_TaskbarCreated,     OnTaskbarCreated)
    ON_MESSAGE(WM_TRAYICON,                             OnTrayIcon)

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CInitWait::CInitWait(CWinApp *myApp)
{
    //
    // on entry, compare version against driver and service
    //

    m_pMessageDialog = NULL;

    m_hIconPtr = NULL;

    m_pMenu = NULL;

    m_app_ver.Format(L"%S", MY_VERSION_COMPAT);
    m_svc_ver = L"?";
    m_drv_ver = L"?";

    m_try_elevate = CMyApp::m_WindowsVista;

    GetVersions();

    if (m_app_ver == m_svc_ver && m_app_ver == m_drv_ver)
        return;

    //
    // if we got here, it means either service or driver is
    // not yet ready, so create window and tray icon for the wait
    //

    CreateEx(   0, (LPCTSTR)CMyApp::m_atom, CMyApp::m_appTitle,
                WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU,
                0, 0, 0, 0, NULL, NULL, NULL);

    myApp->m_pMainWnd = this;

    //
    //
    //

    HINSTANCE hInstance = AfxGetInstanceHandle();

    m_hIcon1 = ::LoadIcon(hInstance, L"INITWAIT1");
    m_hIcon2 = ::LoadIcon(hInstance, L"INITWAIT2");
    m_hIconPtr = m_hIcon1;

    m_pMenu = CMyApp::MyLoadMenu(L"WAIT_MENU");

    OnTaskbarCreated(0, 0);

    SetTimer(ID_TIMER, 1000, NULL);

    //
    // start SbieSvc
    //

    SbieDll_StartSbieSvc(FALSE);

    //
    // temporary message loop while we initialize
    //

    while (m_hIconPtr) {

        MSG msg;
        BOOL b = ::GetMessage(&msg, NULL, 0, 0);
        if (! b)
            break;

        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }

    if (m_app_ver != m_svc_ver || m_app_ver != m_drv_ver)
        exit(0);

    KillTimer(ID_TIMER);

    CMyApp::DeleteTrayIcon();
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CInitWait::~CInitWait()
{
}


//---------------------------------------------------------------------------
// GetVersions
//---------------------------------------------------------------------------


void CInitWait::GetVersions()
{
    BOOL fail = FALSE;
    WCHAR drv_ver[16];

    if (1) {
        bool init_complete = false;
        MSG_HEADER req;
        req.length = sizeof(MSG_HEADER);
        req.msgid = MSGID_PROCESS_CHECK_INIT_COMPLETE;
        MSG_HEADER *rpl = SbieDll_CallServer(&req);
        if (rpl) {
            if (rpl->status == 0)
                init_complete = true;
            SbieDll_FreeMem(rpl);
        }
        if (! init_complete)
            return;
    }

    if (m_svc_ver.GetAt(0) == L'?') {

        SBIE_INI_GET_VERSION_REQ req;
        req.h.length = sizeof(SBIE_INI_GET_VERSION_REQ);
        req.h.msgid = MSGID_SBIE_INI_GET_VERSION;

        SBIE_INI_GET_VERSION_RPL *rpl =
            (SBIE_INI_GET_VERSION_RPL *)SbieDll_CallServer(&req.h);
        if (rpl) {
            if (rpl->h.status == 0 && rpl->version[0]) {
                m_svc_ver = rpl->version;
                if (m_svc_ver != m_app_ver)
                    fail = TRUE;
            }
            SbieDll_FreeMem(rpl);
        }
    }

    if (m_drv_ver.GetAt(0) == L'?') {

        SbieApi_GetVersion(drv_ver);
        if (drv_ver[0] && _wcsicmp(drv_ver, L"unknown") != 0) {
            m_drv_ver = drv_ver;
            if (m_drv_ver != m_app_ver)
                fail = TRUE;
        }
    }

    if (fail) {
        CMyMsg msg(MSG_3304, m_app_ver, m_svc_ver, m_drv_ver);
        CMyApp::MsgBox(NULL, msg, MB_OK);
        exit(0);
    }
}


//---------------------------------------------------------------------------
// OnDestroy
//---------------------------------------------------------------------------


void CInitWait::OnDestroy()
{
    CMyApp::DeleteTrayIcon();
    if (m_hIconPtr)
        exit(0);
}



//---------------------------------------------------------------------------
// OnTimer
//---------------------------------------------------------------------------


void CInitWait::OnTimer(UINT_PTR nIDEvent)
{
    if (! m_hIconPtr)
        return;

    if (m_hIconPtr == m_hIcon1)
        m_hIconPtr = m_hIcon2;
    else
        m_hIconPtr = m_hIcon1;
    CMyApp::ChangeTrayIcon(m_hIconPtr, CString());

    GetVersions();
    if (m_app_ver == m_svc_ver && m_app_ver == m_drv_ver)
        m_hIconPtr = NULL;

    else if (m_try_elevate) {

        //
        // on Windows Vista, elevate to start the service
        //

        const WCHAR *StartError = SbieDll_GetStartError();
        if (StartError && wcsstr(StartError, L"[22 / 5]")) {

            m_try_elevate = FALSE;

            WCHAR path[512];
            GetSystemWindowsDirectoryW(path, MAX_PATH);
            wcscat(path, L"\\system32\\sc.exe");

            SHELLEXECUTEINFO shex;
            memzero(&shex, sizeof(SHELLEXECUTEINFO));
            shex.cbSize = sizeof(SHELLEXECUTEINFO);
            shex.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
            shex.hwnd = NULL;
            shex.lpFile = path;
            shex.lpParameters = L"start " SBIESVC;
            shex.nShow = SW_SHOWNORMAL;
            shex.lpVerb = L"runas";

            ShellExecuteEx(&shex);
        }
    }
}


//---------------------------------------------------------------------------
// OnTaskbarCreated
//---------------------------------------------------------------------------


LRESULT CInitWait::OnTaskbarCreated(WPARAM wParam, LPARAM lParam)
{
    CMyApp::CreateTrayIcon(this, m_hIconPtr, CMyMsg(MSG_3303));
    return 0;
}


//---------------------------------------------------------------------------
// OnTrayIcon
//---------------------------------------------------------------------------


LRESULT CInitWait::OnTrayIcon(WPARAM wParam, LPARAM lParam)
{
    if (LOWORD(lParam) == WM_RBUTTONUP) {

        CPoint pt = (CPoint)GetMessagePos();
        SetForegroundWindow();
        m_pMenu->GetSubMenu(0)->TrackPopupMenu(0, pt.x, pt.y, this, NULL);
        PostMessage(WM_NULL, 0, 0);
        return 0;
    }

    return 1;
}


//---------------------------------------------------------------------------
// OnShowErrors
//---------------------------------------------------------------------------


void CInitWait::OnShowErrors()
{
    CMessageDialog msgdlg(this, MSGDLG_EVENT_VIEWER);
}
