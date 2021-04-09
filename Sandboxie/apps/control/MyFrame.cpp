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
// Frame
//---------------------------------------------------------------------------


#include "stdafx.h"
#include "MyApp.h"
#include "MyFrame.h"

#include "DisableForceDialog.h"
#include "MonitorDialog.h"
#include "CreateDialog.h"
#include "DeleteDialog.h"
#include "SetFolderDialog.h"
#include "SetLayoutDialog.h"
#include "AlertDialog.h"
#include "ShellDialog.h"
#include "ThirdPartyDialog.h"
#include "AboutDialog.h"
#include "Boxes.h"
#include "UserSettings.h"
#include "SbieIni.h"
#include "BoxPage.h"
#include "BorderGuard.h"
#include "LockConfigDialog.h"
#include "RevealDialog.h"
#include "GettingStartedWizard.h"
#include "apps/common/RunBrowser.h"
#include "apps/common/BoxOrder.h"
#include "common/my_version.h"
#include "Updater.h"
#include "UpdateDialog.h"

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


enum {
    ID_FIRST        = 1000,
    ID_TIMER,
    ID_LAST
};


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static const WCHAR *_ShowWelcome                = L"ShowWelcome";
static const WCHAR *_AlwaysOnTop                = L"AlwaysOnTop";

static const WCHAR *_WindowCoords               = L"WindowCoords";
static const WCHAR *_WindowLeft                 = L"WindowLeft";
static const WCHAR *_WindowTop                  = L"WindowTop";
static const WCHAR *_WindowWidth                = L"WindowWidth";
static const WCHAR *_WindowHeight               = L"WindowHeight";
static const WCHAR *_ActiveView                 = L"ActiveView";

static const WCHAR *_TerminateNotify            = L"TerminateNotify";
static const WCHAR *_TerminateWarn              = L"TerminateWarn";
static const WCHAR *_ExplorerNotify             = L"ExplorerNotify";
static const WCHAR *_ExplorerWarn               = L"ExplorerWarn";
static const WCHAR *_EditConfNotify             = L"EditConfNotify";
static const WCHAR *_ReloadConfNotify           = L"ReloadConfNotify";
static const WCHAR *_HideWindowNotify           = L"HideWindowNotify";
       const WCHAR *_ProcSettingsNotify         = L"ProcSettingsNotify";
       const WCHAR *_SettingChangeNotify        = L"SettingChangeNotify";
       const WCHAR *_ShortcutNotify             = L"ShortcutNotify";
       const WCHAR *_UpdateCheckNotify          = L"UpdateCheckNotify";
static const WCHAR *_ShouldDeleteNotify         = L"ShouldDeleteNotify";

	   const WCHAR *_NextUpdateCheck            = L"NextUpdateCheck";

BOOL CMyFrame::m_inTimer   = FALSE;
BOOL CMyFrame::m_destroyed = FALSE;

CWnd *CMyFrame::m_GettingStartedWindow = NULL;


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CMyFrame, CFrameWnd)

    ON_COMMAND(ID_EXIT,                         OnCmdExit)
    ON_COMMAND(ID_SHOW_WINDOW,                  OnCmdShowWindow)
    ON_COMMAND(ID_TERMINATE_ALL,                OnCmdTermAll)
    ON_COMMAND(ID_DISABLE_FORCE,                OnCmdDisableForce)
    ON_COMMAND(ID_RESOURCE_MONITOR,             OnCmdResourceMonitor)

    ON_WM_ENTERIDLE()

    ON_COMMAND(ID_VIEW_CONTEXT,                 OnCmdViewContext)
    ON_COMMAND(ID_VIEW_PROCESS,                 OnCmdViewProcess)
    ON_COMMAND(ID_VIEW_FILES,                   OnCmdViewFiles)
    ON_COMMAND(ID_VIEW_RECOVERY_LOG,            OnCmdViewRecoveryLog)
    ON_COMMAND(ID_VIEW_TOPMOST,                 OnCmdViewTopmost)

    ON_COMMAND(ID_SANDBOX_REVEAL,               OnCmdSandboxReveal)
    ON_COMMAND(ID_SANDBOX_SET_LAYOUT,           OnCmdSandboxSetLayout)
    ON_COMMAND(ID_SANDBOX_SET_FOLDER,           OnCmdSandboxSetFolder)
    ON_COMMAND(ID_SANDBOX_CREATE_NEW,           OnCmdSandboxCreateNew)
    ON_COMMAND_RANGE(
        ID_SANDBOX_MENU, ID_SANDBOX_MENU_LAST_ID,   OnCmdSandbox)

    ON_COMMAND(ID_FINDER_OPEN,                  OnCmdFinderOpen)
    ON_COMMAND(ID_FINDER_CLOSE,                 OnCmdFinderClose)

    ON_COMMAND(ID_CONF_ALERT,                   OnCmdConfAlert)
    ON_COMMAND(ID_CONF_SHELL,                   OnCmdConfShell)
    ON_COMMAND(ID_CONF_THIRD_PARTY,             OnCmdThirdParty)
    ON_COMMAND(ID_CONF_THIRD_PARTY_ALERT,       OnCmdThirdParty)
    ON_COMMAND(ID_CONF_HIDDEN_MSG,              OnCmdConfHiddenMsg)
    ON_COMMAND(ID_CONF_SHOW_ALL_TIPS,           OnCmdConfShowHideTips)
    ON_COMMAND(ID_CONF_HIDE_ALL_TIPS,           OnCmdConfShowHideTips)
    ON_COMMAND(ID_CONF_LOCK,                    OnCmdConfLock)
    ON_COMMAND(ID_CONF_EDIT,                    OnCmdConfEdit)
    ON_COMMAND(ID_CONF_RELOAD,                  OnCmdConfReload)

	ON_COMMAND(ID_HELP_SUPPORT,                 OnCmdHelpSupport)
    ON_COMMAND(ID_HELP_TOPICS,                  OnCmdHelpTopics)
    ON_COMMAND(ID_HELP_TUTORIAL,                OnCmdHelpTutorial)
    ON_COMMAND(ID_HELP_FORUM,                   OnCmdHelpForum)
	ON_COMMAND(ID_HELP_UPDATE,                  OnCmdHelpUpdate)
    ON_COMMAND(ID_HELP_ABOUT,                   OnCmdHelpAbout)

	//ON_MESSAGE(WM_UPDATERESULT,					OnUpdateResult)

    ON_COMMAND(ID_PROCESS_TERMINATE,            OnCmdTerminateProcess)

    ON_UPDATE_COMMAND_UI(ID_DISABLE_FORCE,      OnUpdDisableForce)
    ON_UPDATE_COMMAND_UI(ID_VIEW_TOPMOST,       OnUpdViewMenu)
    ON_UPDATE_COMMAND_UI(ID_VIEW_CONTEXT,       OnUpdViewMenu)
    ON_UPDATE_COMMAND_UI(ID_VIEW_PROCESS,       OnUpdViewMenu)
    ON_UPDATE_COMMAND_UI(ID_VIEW_FILES,         OnUpdViewMenu)

    ON_WM_INITMENU()
    ON_WM_CONTEXTMENU()

    ON_REGISTERED_MESSAGE(CMyApp::m_TaskbarCreated,     OnTaskbarCreated)
    ON_MESSAGE(WM_TRAYICON,                             OnTrayIcon)

    ON_WM_CLOSE()
    ON_WM_DESTROY()
    ON_WM_QUERYENDSESSION()
    ON_WM_ERASEBKGND()
    ON_WM_SIZE()
    ON_WM_SIZING()
    ON_WM_TIMER()
    ON_WM_SETFOCUS()
    ON_WM_SHOWWINDOW()
    ON_WM_WINDOWPOSCHANGED()

    ON_WM_DROPFILES()

    ON_WM_DEVICECHANGE()

    ON_MENUXP_MESSAGES()

END_MESSAGE_MAP()


IMPLEMENT_MENUXP(CMyFrame, CFrameWnd)


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CMyFrame::CMyFrame(BOOL ForceVisible, BOOL ForceSync)
{
    m_mondlg = NULL;
    m_msgdlg = NULL;
    m_finder = NULL;
    m_view = m_view_old = 0;
    m_hidden = FALSE;

    //CUserSettings::GetInstance().GetBool(_ShowWelcome, m_ShowWelcome, TRUE);
    CUserSettings::GetInstance().GetBool(_AlwaysOnTop, m_AlwaysOnTop, FALSE);

    m_ReSyncShortcuts = ForceSync;

    m_AutoRunSoftCompat = CThirdPartyDialog::AutoRunSoftCompat();

    //
    // create main window
    //

    int left, top, width, height;

    CStringList coords;
    CUserSettings::GetInstance().GetTextCsv(_WindowCoords, coords);
    if (coords.GetCount() >= 4) {

        left   = _wtoi(coords.RemoveHead());
        top    = _wtoi(coords.RemoveHead());
        width  = _wtoi(coords.RemoveHead());
        height = _wtoi(coords.RemoveHead());

    } else {

        CUserSettings::GetInstance().GetNum(_WindowLeft,   left);
        CUserSettings::GetInstance().GetNum(_WindowTop,    top);
        CUserSettings::GetInstance().GetNum(_WindowWidth,  width);
        CUserSettings::GetInstance().GetNum(_WindowHeight, height);
    }

    AdjustSizePosition(left, top, width, height);

    ULONG exStyle = (CMyApp::m_LayoutRTL) ? WS_EX_LAYOUTRTL : 0;
	CString strTitle = CMyApp::m_appTitle + " - " MY_COMPANY_NAME_STRING;
    CreateEx(   exStyle, (LPCTSTR)CMyApp::m_atom, strTitle,
                WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU,
                left, top, width, height,
                NULL, NULL, NULL);

    InitMenus();

    DragAcceptFiles();

    //
    // create viewers
    //

    m_proclist.Create(this);
    m_filelist.Create(this);

    m_view = ID_VIEW_PROCESS;
    int view;
    CUserSettings::GetInstance().GetNum(_ActiveView, view, 0);
    if (view == ID_VIEW_PROCESS || view == ID_VIEW_FILES)
        m_view = view;
    OnSize(SIZE_RESTORED, 0, 0);

    //
    // customize menus
    //

    m_pTrayMenu = CMyApp::MyLoadMenu(L"TRAY_MENU");

    m_pTraySandboxMenu = CMyApp::MyLoadMenu(L"TRAY_SANDBOX_MENU");

    if (! CUserSettings::GetInstance().CanDisableForce()) {
        CMenu *pMenu = m_pTrayMenu->GetSubMenu(0);
        pMenu->DeleteMenu(ID_DISABLE_FORCE, MF_BYCOMMAND);
        pMenu = GetMenu()->GetSubMenu(0);
        pMenu->DeleteMenu(ID_DISABLE_FORCE, MF_BYCOMMAND);
    }

    if (! CUserSettings::GetInstance().CanDisableForce()) {
        CMenu *pMenu = m_pTrayMenu->GetSubMenu(0);
        pMenu->DeleteMenu(ID_DISABLE_FORCE, MF_BYCOMMAND);
        pMenu = GetMenu()->GetSubMenu(0);
        pMenu->DeleteMenu(ID_DISABLE_FORCE, MF_BYCOMMAND);
    }

    //
    // create tray icon
    //

    m_TrayActive = FALSE;
    m_TrayCommand = FALSE;

    HINSTANCE hInstance = AfxGetInstanceHandle();
    m_hIconFull     = ::LoadIcon(hInstance, L"TRAYICON_FULL");
    m_hIconEmpty    = ::LoadIcon(hInstance, L"TRAYICON_EMPTY");
    m_hIconFullDfp  = ::LoadIcon(hInstance, L"TRAYICON_FULL_DFP");
    m_hIconEmptyDfp = ::LoadIcon(hInstance, L"TRAYICON_EMPTY_DFP");
    m_hIconDelete   = ::LoadIcon(hInstance, L"TRAYICON_DELETE");
    m_hIconPtr      = 0;

    RefreshTrayIcon();

    OnTaskbarCreated(0, 0);

    //
    // create message dialog
    //

    m_msgdlg = new CMessageDialog(this, MSGDLG_NORMAL);

    //
    // start timer
    //

    SetTimer(ID_TIMER, 1000, NULL);

    //
    // set window state, minimized or normal
    //

    if (ForceVisible || (! CMyApp::m_hwndTray))
        m_hidden = FALSE;
    else
        m_hidden = TRUE;

    if (m_hidden)
        ShowWindow(SW_HIDE);
    else
        ShowWindow(SW_SHOWNORMAL);

    //
    // initialize border guard
    //

    CBorderGuard::RefreshConf();

    //
    // initialize Sandboxie service watchdog
    //

    CSbieIni::MonitorService(this, ID_EXIT);

    if (m_AlwaysOnTop)
        SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CMyFrame::~CMyFrame()
{
    if (! m_inTimer)
        delete m_msgdlg;
    m_destroyed = TRUE;
}


//---------------------------------------------------------------------------
// GetMyListCtrl
//---------------------------------------------------------------------------


CMyListCtrl *CMyFrame::GetMyListCtrl()
{
    CMyListCtrl *pMyListCtrl = NULL;
    if (m_view == ID_VIEW_PROCESS)
        pMyListCtrl = &m_proclist;
    else if (m_view == ID_VIEW_FILES)
        pMyListCtrl = &m_filelist;
    return pMyListCtrl;
}


//---------------------------------------------------------------------------
// AdjustPosition
//---------------------------------------------------------------------------


bool CMyFrame::AdjustSizePosition(
    int &left, int &top, int &width, int &height)
{
    bool no_corresponding_monitor = true;

    RECT rc;
    rc.left = left;
    rc.top = top;
    rc.right = left + width - 1;
    rc.bottom = top + height - 1;
    HMONITOR hmonitor = MonitorFromRect(&rc, MONITOR_DEFAULTTONULL);
    if ((! hmonitor) || (width <= 10) || (height  <= 10)) {

        GetDesktopWindow()->GetWindowRect(&rc);
        double xScale = rc.right / 1024.0;
        double yScale = rc.bottom / 768.0;

        left = 200;
        top = 150;
        width = (int)(660 * xScale);
        height = (int)(450 * yScale);

        return true;
    }

    return false;
}


//---------------------------------------------------------------------------
// InitMenus
//---------------------------------------------------------------------------


void CMyFrame::InitMenus(void)
{
    //
    // create and customize main menu
    //

    CMenu *pMenu = CMyApp::MyLoadMenu(L"TOP_MENU");


    //
    // activate main menu
    //

    SetMenu(pMenu);

    CMenuXP::SetXPLookNFeel(this, *pMenu, true, true);

    //
    // prepare Sandbox menu
    //

    m_SandboxMenu = CMyApp::MyLoadMenu(L"SANDBOX_MENU");
}


//---------------------------------------------------------------------------
// SaveSettings
//---------------------------------------------------------------------------


void CMyFrame::SaveSettings()
{
    WINDOWPLACEMENT wndpl;
    wndpl.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(&wndpl);
    CRect rc = wndpl.rcNormalPosition;

    CString coords;
    coords.Format(L"%d,%d,%d,%d", rc.left, rc.top, rc.Width(), rc.Height());
    BOOL ok = CUserSettings::GetInstance().SetText(_WindowCoords, coords);

    if (ok)
        ok = CUserSettings::GetInstance().SetNum(_ActiveView, m_view);

    if (ok) {
        CUserSettings::GetInstance().SetText(_WindowLeft,    CString());
        CUserSettings::GetInstance().SetText(_WindowTop,     CString());
        CUserSettings::GetInstance().SetText(_WindowWidth,   CString());
        CUserSettings::GetInstance().SetText(_WindowHeight,  CString());
    }
}


//---------------------------------------------------------------------------
// OnDestroy
//---------------------------------------------------------------------------


void CMyFrame::OnDestroy()
{
    KillTimer(ID_TIMER);
    CMyApp::DeleteTrayIcon();
    SaveSettings();
    PostQuitMessage(0);
}


//---------------------------------------------------------------------------
// OnQueryEndSession
//---------------------------------------------------------------------------


BOOL CMyFrame::OnQueryEndSession()
{
    SaveSettings();
    return TRUE;
}


//---------------------------------------------------------------------------
// PostNcDestroy
//---------------------------------------------------------------------------


void CMyFrame::PostNcDestroy()
{
    Detach();
    delete this;
}


//---------------------------------------------------------------------------
// OnCmdExit
//---------------------------------------------------------------------------


void CMyFrame::OnCmdExit()
{
    DestroyWindow();
}


//---------------------------------------------------------------------------
// OnCmdShowWindow
//---------------------------------------------------------------------------


void CMyFrame::OnCmdShowWindow()
{
    //
    // if we're displaying a dialog box then just switch hidden/visible flag
    // then this function isn't invoked by OnShowWindow but only by a direct
    // use of the tray icon, so we show the active dialog box
    //

    bool inModal;
    CWnd *pInputWnd = CBaseDialog::GetInputWindow(&inModal);
    if (inModal) {

        if (! IsWindowVisible()) {
            if (pInputWnd->IsIconic())
                pInputWnd->ShowWindow(SW_RESTORE);
            else {
                //
                // special case:  if the main window is hidden and
                // the dialog box is not minimized, then restore the
                // main window when the tray icon is clicked
                //
                goto normal;
            }
            m_hidden = FALSE;

        } else if (pInputWnd->GetStyle() & WS_MINIMIZEBOX) {

            m_hidden = TRUE;
            if (! pInputWnd->IsIconic())
                pInputWnd->ShowWindow(SW_MINIMIZE);
        }

        return;
    }

    //
    // otherwise normal operation
    //

normal:

    if (m_hidden) {

        CRect rc;
        GetWindowRect(&rc);
        int left = rc.left;
        int top = rc.top;
        int width = rc.Width();
        int height = rc.Height();
        if (AdjustSizePosition(left, top, width, height))
            MoveWindow(left, top, width, height, FALSE);

        m_hidden = FALSE;
        OnSetFocus(NULL);
        ShowWindow(SW_RESTORE);
        SetForegroundWindow();

    } else {

        m_hidden = TRUE;
        ShowWindow(SW_HIDE);
        CSbieIni::GetInstance().ForgetPasswordNow();
    }
}


//---------------------------------------------------------------------------
// OnCmdViewContext
//---------------------------------------------------------------------------


void CMyFrame::OnCmdViewContext()
{
    CPoint pt;
    CMyListCtrl *pMyListCtrl = GetMyListCtrl();
    if (pMyListCtrl) {
        pMyListCtrl->GetSelectedItemPosition(pt);
        OnContextMenu(this, pt);
    }
}


//---------------------------------------------------------------------------
// OnCmdViewProcess
//---------------------------------------------------------------------------


void CMyFrame::OnCmdViewProcess()
{
    m_view_old = m_view;
    m_view = ID_VIEW_PROCESS;
    OnSize(SIZE_RESTORED, 0, 0);
}


//---------------------------------------------------------------------------
// OnCmdViewFiles
//---------------------------------------------------------------------------


void CMyFrame::OnCmdViewFiles()
{
    m_view_old = m_view;
    m_view = ID_VIEW_FILES;
    OnSize(SIZE_RESTORED, 0, 0);
}


//---------------------------------------------------------------------------
// OnCmdViewFiles
//---------------------------------------------------------------------------


void CMyFrame::OnCmdViewRecoveryLog()
{
    CQuickRecover(this, CString(), CString(), QR_LOG);
}


void CMyFrame::OnCmdViewTopmost()
{
    m_AlwaysOnTop = !m_AlwaysOnTop;
    CUserSettings::GetInstance().SetBool(_AlwaysOnTop, m_AlwaysOnTop);
    SetWindowPos(m_AlwaysOnTop ? &CWnd::wndTopMost : &CWnd::wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}


//---------------------------------------------------------------------------
// OnUpdViewMenu
//---------------------------------------------------------------------------


void CMyFrame::OnUpdViewMenu(CCmdUI *pCmdUI)
{
    if (pCmdUI->m_nID == ID_VIEW_CONTEXT) {

        BOOL enable = FALSE;
        CMyListCtrl *pMyListCtrl = GetMyListCtrl();
        if (pMyListCtrl) {
            CPoint pt;
            pMyListCtrl->GetSelectedItemPosition(pt);
            if (pt.x != 0 || pt.y != 0)
                enable = TRUE;
        }
        pCmdUI->Enable(enable);

    } else {

        pCmdUI->SetCheck(m_view == pCmdUI->m_nID);
    }

    if (pCmdUI->m_nID == ID_VIEW_TOPMOST)
        pCmdUI->SetCheck(m_AlwaysOnTop);

}


//---------------------------------------------------------------------------
// OnCmdFinderOpen
//---------------------------------------------------------------------------


void CMyFrame::OnCmdFinderOpen()
{
    ShowWindow(SW_HIDE);

    if (m_finder)
        return;

    m_finder = new CFinderDialog(this, ID_FINDER_CLOSE);
}


//---------------------------------------------------------------------------
// OnCmdFinderClose
//---------------------------------------------------------------------------


void CMyFrame::OnCmdFinderClose()
{
    if (! m_finder)
        return;

    ShowWindow(SW_SHOW);
    m_hidden = FALSE;

    ULONG pid = m_finder->GetProcessId();
    if (pid)
        m_proclist.SelectByProcessId(pid);

    delete m_finder;
    m_finder = NULL;

    if (pid)
        OnCmdViewProcess();
}


//---------------------------------------------------------------------------
// OnCmdConfAlert
//---------------------------------------------------------------------------


void CMyFrame::OnCmdConfAlert()
{
    CAlertDialog dlg(this);
}


//---------------------------------------------------------------------------
// OnCmdConfShell
//---------------------------------------------------------------------------


void CMyFrame::OnCmdConfShell()
{
    CShellDialog dlg(this);
}


//---------------------------------------------------------------------------
// OnCmdThirdParty
//---------------------------------------------------------------------------


void CMyFrame::OnCmdThirdParty()
{
    m_AutoRunSoftCompat = FALSE;

    if (GetCurrentMessage()->message == WM_COMMAND &&
            GetCurrentMessage()->wParam == ID_CONF_THIRD_PARTY_ALERT) {
        if (CThirdPartyDialog::AutoRunSoftCompat())
            m_AutoRunSoftCompat = TRUE; // for next WM_TIMER
        return;
    }

    BOOL autorun = (GetCurrentMessage()->message == WM_TIMER);
    CThirdPartyDialog dlg(this, autorun, ID_CONF_THIRD_PARTY_ALERT);
}


//---------------------------------------------------------------------------
// OnCmdConfHiddenMsg
//---------------------------------------------------------------------------


void CMyFrame::OnCmdConfHiddenMsg()
{
    CUserSettings::GetInstance().SetText(_HideMessage, CString());
    m_msgdlg->ReloadConf();

    CMyApp::MsgBox(this, MSG_3536, MB_OK);
}


//---------------------------------------------------------------------------
// OnCmdConfShowHideTips
//---------------------------------------------------------------------------


void CMyFrame::OnCmdConfShowHideTips()
{
    BOOL val;
    ULONG msg;
    if (GetCurrentMessage()->wParam == ID_CONF_SHOW_ALL_TIPS) {
        val = TRUE;
        msg = MSG_3537;
    } else if (GetCurrentMessage()->wParam == ID_CONF_HIDE_ALL_TIPS) {
        val = FALSE;
        msg = MSG_3538;
    } else
        return;

    CUserSettings &ini = CUserSettings::GetInstance();
    BOOL ok = TRUE;
    if (ok)
        ok = ini.SetBool(_TerminateNotify,       val);
    if (ok)
        ok = ini.SetBool(_TerminateWarn,         val);
    if (ok)
        ok = ini.SetBool(_ExplorerNotify,        val);
    if (ok)
        ok = ini.SetBool(_ExplorerWarn,          val);
    if (ok)
        ok = ini.SetBool(_EditConfNotify,        val);
    if (ok)
        ok = ini.SetBool(_ReloadConfNotify,      val);
    if (ok)
        ok = ini.SetBool(_HideWindowNotify,      val);
    if (ok)
        ok = ini.SetBool(_ProcSettingsNotify,    val);
    if (ok)
        ok = ini.SetBool(_SettingChangeNotify,   val);
    if (ok)
        ok = ini.SetBool(_ShortcutNotify,        val);
    if (ok)
        ok = ini.SetBool(_UpdateCheckNotify,     val);
    if (ok)
        ok = ini.SetBool(_ShouldDeleteNotify,    val);
    if (ok)
        ok = ini.SetBool(_ShowWelcome,           val);

    if (ok)
        CMyApp::MsgBox(this, msg, MB_OK);
}


//---------------------------------------------------------------------------
// OnCmdConfLock
//---------------------------------------------------------------------------


void CMyFrame::OnCmdConfLock()
{
    CLockConfigDialog dlg(this);
}


//---------------------------------------------------------------------------
// OnCmdConfEdit
//---------------------------------------------------------------------------


void CMyFrame::OnCmdConfEdit()
{
    BOOL isEditAdminOnly, isForceDisableAdminOnly;
    BOOL isEditPassword, isHavePassword, isForgetPassword;
    CSbieIni::GetInstance().GetRestrictions(
        isEditAdminOnly, isForceDisableAdminOnly,
        isEditPassword, isHavePassword, isForgetPassword);

    if (isEditPassword) {

        CMyApp::MsgBox(this, MSG_3540, MB_OK);
        return;
    }

    //
    // launch editor
    //

    CString path = CSbieIni::GetInstance().GetPath();
    path = L"\"" + path + L"\"";

    SHELLEXECUTEINFO shex;
    memzero(&shex, sizeof(SHELLEXECUTEINFO));
    shex.cbSize = sizeof(SHELLEXECUTEINFO);
    shex.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
    shex.hwnd = m_hWnd;
    shex.lpFile = L"notepad";
    shex.lpParameters = path;
    shex.nShow = SW_SHOWNORMAL;

    shex.lpVerb = NULL;
    if (SbieDll_GetTokenElevationType() != TokenElevationTypeNone)
        shex.lpVerb = L"runas";

    if (! ShellExecuteEx(&shex))
        return;

    if (shex.hProcess) {

        ULONG_PTR *handles = (ULONG_PTR *)malloc(sizeof(ULONG_PTR) * 3);
        handles[0] = (ULONG_PTR)shex.hProcess;
        handles[1] = (ULONG_PTR)m_hWnd;
        AfxBeginThread(OnCmdConfEditThread, handles);
    }

    CUserSettings &settings = CUserSettings::GetInstance();
    BOOL tip;
    settings.GetBool(_EditConfNotify, tip, TRUE);

    if (tip) {
        int rv = CMyApp::MsgCheckBox(this, MSG_3512, 0, MB_OK);
        if (rv < 0) {
            rv = -rv;
            settings.SetBool(_EditConfNotify, FALSE);
        }
    }
}


//---------------------------------------------------------------------------
// OnCmdConfEditThread
//---------------------------------------------------------------------------


UINT AFX_CDECL CMyFrame::OnCmdConfEditThread(LPVOID parm)
{
    ULONG_PTR *handles = (ULONG_PTR *)parm;
    HANDLE hProcess = (HANDLE)handles[0];
    HWND hWnd = (HWND)handles[1];
    free(parm);
    WaitForSingleObject(hProcess, INFINITE);
    ::PostMessage(hWnd, WM_COMMAND, ID_CONF_RELOAD, 0);
    AfxEndThread(0);
    return 0;
}


//---------------------------------------------------------------------------
// OnCmdConfReload
//---------------------------------------------------------------------------


void CMyFrame::OnCmdConfReload()
{
    if (SbieApi_ReloadConf(-1) == 0) {

        CBoxes::GetInstance().ReloadBoxes();
        CBoxes::GetInstance().RefreshProcesses();
        m_msgdlg->ReloadConf();
        m_proclist.RebuildCombo();
        CBorderGuard::RefreshConf();
        OnSetFocus(NULL);
        CShellDialog::SyncSendToMenu();

        /*
        BOOL any = FALSE;
        CBoxes &boxes = CBoxes::GetInstance();
        for (int i = 1; i < boxes.GetSize(); ++i) {
            CBox &box = boxes.Get(i);
            if (box.GetProcessCount()) {
                any = TRUE;
                break;
            }
        }

        const WCHAR *setting = any ? _ReloadConfNotify2 : _ReloadConfNotify1;
        */

        CUserSettings &settings = CUserSettings::GetInstance();

        BOOL tip;
        settings.GetBool(_ReloadConfNotify, tip, TRUE);
        if (tip) {

            int rv = CMyApp::MsgCheckBox(this, MSG_3513, 0, MB_OK);

            if (rv < 0) {
                rv = -rv;
                settings.SetBool(_ReloadConfNotify, FALSE);
            }
        }

    } else {

        ++m_inTimer;
        m_msgdlg->OnTimer();
        --m_inTimer;
        if (! m_destroyed)
            CMyApp::MsgBox(this, MSG_3514, MB_OK);
    }
}


//---------------------------------------------------------------------------
// OnCmdHelpSupport
//---------------------------------------------------------------------------


void CMyFrame::OnCmdHelpSupport()
{
	CRunBrowser x(this, L"https://sandboxie-plus.com/go.php?to=donate");
}


//---------------------------------------------------------------------------
// OnCmdHelpTopics
//---------------------------------------------------------------------------


void CMyFrame::OnCmdHelpTopics()
{
    CRunBrowser::OpenHelp(this, L"HelpTopics");
}


//---------------------------------------------------------------------------
// OnCmdHelpTutorial
//---------------------------------------------------------------------------


void CMyFrame::OnCmdHelpTutorial()
{
    if (m_AutoRunSoftCompat) {
        Sleep(250);
        PostMessage(WM_COMMAND, ID_HELP_TUTORIAL, 0);
    } else
        CGettingStartedWizard wizard(this);
}

//---------------------------------------------------------------------------
// OnCmdHelpForum
//---------------------------------------------------------------------------


void CMyFrame::OnCmdHelpForum()
{
    CRunBrowser::OpenForum(this);
}

//---------------------------------------------------------------------------
// OnCmdHelpUpdate
//---------------------------------------------------------------------------


void CMyFrame::OnCmdHelpUpdate()
{
	CUpdateDialog dlg(this);
	dlg.DoModal();
}

//---------------------------------------------------------------------------
// OnCmdHelpAbout
//---------------------------------------------------------------------------


void CMyFrame::OnCmdHelpAbout()
{
    CAboutDialog dlg(this);
}


//---------------------------------------------------------------------------
// OnCmdDisableForce
//---------------------------------------------------------------------------


void CMyFrame::OnCmdDisableForce()
{
    ULONG state;
    SbieApi_DisableForceProcess(NULL, &state);
    if (state) {

        state = FALSE;
        SbieApi_DisableForceProcess(&state, NULL);

    } else {

        state = TRUE;
        const MSG *msg = GetCurrentMessage();
        if (HIWORD(msg->wParam) != 'tz') {
            CDisableForceDialog dlg(this);
            if (! dlg.EndedOk())
                state = FALSE;
        }
        if (state)
            SbieApi_DisableForceProcess(&state, NULL);
    }

    RefreshTrayIcon();
}


//---------------------------------------------------------------------------
// OnUpdDisableForce
//---------------------------------------------------------------------------


void CMyFrame::OnUpdDisableForce(CCmdUI *pCmdUI)
{
    ULONG state;
    SbieApi_DisableForceProcess(NULL, &state);
    pCmdUI->SetCheck(state != 0);
}


//---------------------------------------------------------------------------
// OnCmdResourceMonitor
//---------------------------------------------------------------------------


void CMyFrame::OnCmdResourceMonitor()
{
    if (m_mondlg)
        return;
    m_mondlg = new CMonitorDialog(this);

    m_mondlg->DoModal();

    delete m_mondlg;
    m_mondlg = NULL;
}


//---------------------------------------------------------------------------
// OnEnterIdle
//---------------------------------------------------------------------------


void CMyFrame::OnEnterIdle(UINT nWhy, CWnd *pWho)
{
    if (nWhy == MSGF_DIALOGBOX  && pWho == m_mondlg)
        m_mondlg->OnIdle();
}


//---------------------------------------------------------------------------
// OnCmdTermAll
//---------------------------------------------------------------------------


void CMyFrame::OnCmdTermAll()
{
    BOOL warn = TRUE;
    CBoxes &boxes = CBoxes::GetInstance();
    for (int i = (int)boxes.GetSize() - 1; i >= 1; --i) {
        CBox &box = boxes.GetBox(i);
        if (box.GetBoxProc().GetProcessCount()) {
            if (! TerminateProcesses(box, warn))
                break;
            warn = FALSE;
        }
    }
}


//---------------------------------------------------------------------------
// OnCmdSandboxReveal
//---------------------------------------------------------------------------


void CMyFrame::OnCmdSandboxReveal()
{
    CRevealDialog dlg(this);
    if (dlg.IsModified()) {
        CBoxes::GetInstance().ReloadBoxes();
        CShellDialog::SyncSendToMenu();
        m_proclist.RebuildCombo();
    }
    if (m_hWnd)
        OnSetFocus(NULL);
}


//---------------------------------------------------------------------------
// OnCmdSandboxSetFolder
//---------------------------------------------------------------------------


void CMyFrame::OnCmdSandboxSetFolder()
{
    CSetFolderDialog dlg(this);
    if (m_hWnd)
        OnSetFocus(NULL);
}


//---------------------------------------------------------------------------
// OnCmdSandboxSetLayout
//---------------------------------------------------------------------------


void CMyFrame::OnCmdSandboxSetLayout()
{
    CSetLayoutDialog dlg(this);
    if (m_hWnd) {
        m_proclist.RebuildCombo();
        OnSetFocus(NULL);
    }
}


//---------------------------------------------------------------------------
// OnCmdSandboxCreateNew
//---------------------------------------------------------------------------


void CMyFrame::OnCmdSandboxCreateNew()
{
    CCreateDialog dlg(this);
    CString name = dlg.GetName();
    if (name.IsEmpty())
        return;

    CString oldname = dlg.GetOldName();
    if (oldname.IsEmpty())
        CBox::SetEnabled(name);
    else
        CSbieIni::GetInstance().RenameOrCopySection(oldname, name, false);

    if (! m_hWnd)       // in case window was destroyed by exit command
        return;

    CBoxes &boxes = CBoxes::GetInstance();
    boxes.ReloadBoxes();
    CBox &box = boxes.GetBox(name);
    if (! box.GetName().IsEmpty()) {
        box.SetExpandedView(TRUE);
        boxes.WriteExpandedView();
    }

    m_proclist.RebuildCombo();
    OnSetFocus(NULL);
    CShellDialog::SyncSendToMenu();
    CBorderGuard::RefreshConf();
}


//---------------------------------------------------------------------------
// OnCmdSandbox
//---------------------------------------------------------------------------


void CMyFrame::OnCmdSandbox(UINT nID)
{
    int sandbox = (nID - ID_SANDBOX_MENU) / ID_SANDBOX_MENU_SIZE;
    int command = (nID - ID_SANDBOX_MENU) % ID_SANDBOX_MENU_SIZE;

    CBoxes &boxes = CBoxes::GetInstance();
    CBox &box = boxes.GetBox(sandbox);
    if (box.GetName().IsEmpty())
        return;

    bool ReloadBoxes = false;

    switch (command) {

        case ID_SANDBOX_RUN_BROWSER:
            CMyApp::RunStartExe(L"default_browser", box.GetName());
            break;

        case ID_SANDBOX_RUN_MAILER:
            CMyApp::RunStartExe(L"mail_agent", box.GetName());
            break;

        case ID_SANDBOX_RUN_ANY:
            CMyApp::RunStartExe(L"run_dialog", box.GetName());
            break;

        case ID_SANDBOX_RUN_MENU:
            CMyApp::RunStartExe(L"start_menu", box.GetName());
            break;

        case ID_SANDBOX_RUN_EXPLORER:
            RunExplorer(box, TRUE);
            break;

        case ID_SANDBOX_TERMINATE:
            TerminateProcesses(box, TRUE);
            break;

        case ID_SANDBOX_RECOVER:
        case ID_SANDBOX_DELETE:
            if (! CBaseDialog::InModalState()) {
                BOOL spawned;
                BOOL WithDelete =
                    (command == ID_SANDBOX_DELETE) ? TRUE : FALSE;
                CDeleteDialog dlg(
                    this, box.GetName(), WithDelete, FALSE, &spawned);
                if (spawned)
                    RefreshTrayIcon(TRUE);
            }
            break;

        case ID_SANDBOX_EXPLORE:
            RunExplorer(box, FALSE);
            break;

        case ID_SANDBOX_SETTINGS:
            if (! CBaseDialog::InModalState()) {
                CBoxPage::DoPropertySheet(
                    this, box.GetName(), &ReloadBoxes, CString());
            }
            break;

        case ID_SANDBOX_RENAME:
            RemoveOrRename(box, FALSE, TRUE, &ReloadBoxes);
            break;

        case ID_SANDBOX_REMOVE:
            RemoveOrRename(box, TRUE, FALSE, &ReloadBoxes);
            break;
    }

    if (! m_hWnd)       // in case window was destroyed by exit command
        return;

    if (ReloadBoxes) {
        boxes.ReloadBoxes();
        boxes.WriteExpandedView();
        m_proclist.RebuildCombo();
        CShellDialog::SyncSendToMenu();
        CBorderGuard::RefreshConf();
    }

    const MSG *msg = GetCurrentMessage();
    if (HIWORD(msg->wParam) != 'tz') {
        if (command == ID_SANDBOX_EXPLORE)
            ::SetFocus(NULL);
        else
            OnSetFocus(NULL);
    }
}


//---------------------------------------------------------------------------
// RunExplorer
//---------------------------------------------------------------------------


void CMyFrame::RunExplorer(CBox &box, BOOL sandboxed)
{
    CUserSettings &settings = CUserSettings::GetInstance();
    const WCHAR *setting = (sandboxed ? _ExplorerNotify : _ExplorerWarn);

    BOOL tip;
    settings.GetBool(setting, tip, TRUE);
    if (tip) {

        int rv = CMyApp::MsgCheckBox(this,
                (sandboxed ? MSG_3533 : MSG_3534),
                0, MB_OK);
        if (rv < 0) {
            rv = -rv;
            settings.SetBool(setting, FALSE);
        }
    }

    if (sandboxed) {

        //
        // run Windows Explorer sandboxed
        //

        CMyApp::RunStartExe(
            L"explorer.exe /e,::{20D04FE0-3AEA-1069-A2D8-08002B30309D}",
            box.GetName(), FALSE);

    } else {

        //
        // open unsandboxed folder window
        //

        CString BoxPath = box.GetBoxFile().GetPathDos();
        if (! BoxPath.IsEmpty()) {

            const ULONG ShareFlags =
                (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE);

            HANDLE hFile = INVALID_HANDLE_VALUE;

            while (1) {

                hFile = CreateFile(
                    BoxPath, FILE_GENERIC_READ, ShareFlags, NULL,
                    OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
                if (hFile && hFile != INVALID_HANDLE_VALUE)
                    break;

                int backslash = BoxPath.ReverseFind(L'\\');
                if (backslash == -1)
                    break;

                BoxPath = BoxPath.Mid(0, backslash);
            }

            if (hFile == INVALID_HANDLE_VALUE)
                BoxPath = CString();
            else
                CloseHandle(hFile);
        }

        if (! BoxPath.IsEmpty()) {

            int x = (UINT)(ULONG_PTR)ShellExecute(
                m_hWnd, NULL, BoxPath, NULL, NULL, SW_SHOWNORMAL);
            if (x <= 32)
                BoxPath = CString();
        }

        if (BoxPath.IsEmpty()) {

            CMyMsg text(MSG_3535);
            MessageBox(text, CMyApp::m_appTitle, MB_OK);
        }
    }
}


//---------------------------------------------------------------------------
// OnCmdTerminateProcess
//---------------------------------------------------------------------------


void CMyFrame::OnCmdTerminateProcess()
{
    int pid = m_proclist.GetRightClickPid();

    CUserSettings &settings = CUserSettings::GetInstance();

    int rv = 0;
    BOOL warn;

    settings.GetBool(_TerminateWarn, warn, TRUE);
    if (! warn)
        rv = IDYES;
    else {

        rv = CMyApp::MsgCheckBox(this, MSG_3531, MSG_3532, MB_YESNO);
        if (rv < 0) {
            rv = -rv;
            settings.SetBool(_TerminateWarn, FALSE);
        }
    }

    if (rv == IDYES) {

        SbieDll_KillOne(pid);
        OnSetFocus(NULL);
    }
}


//---------------------------------------------------------------------------
// TerminateProcesses
//---------------------------------------------------------------------------


BOOL CMyFrame::TerminateProcesses(CBox &box, BOOL warn)
{
    CUserSettings &settings = CUserSettings::GetInstance();

    int rv = 0;
    BOOL abort = FALSE;

    if (box.GetBoxProc().GetProcessCount() == 0) {

        if (warn)
            settings.GetBool(_TerminateNotify, warn, TRUE);
        if (warn) {

            rv = CMyApp::MsgCheckBox(this, MSG_3530, 0, MB_OK);
            if (rv < 0) {
                rv = -rv;
                settings.SetBool(_TerminateNotify, FALSE);
            }
        }

    } else {

        if (warn)
            settings.GetBool(_TerminateWarn, warn, TRUE);
        if (! warn)
            rv = IDYES;
        else {

            rv = CMyApp::MsgCheckBox(this, MSG_3531, MSG_3532, MB_YESNO);
            if (rv < 0) {
                rv = -rv;
                settings.SetBool(_TerminateWarn, FALSE);
            }
        }

        if (rv == IDYES)
            SbieDll_KillAll(-1, box.GetName());
        else
            abort = TRUE;
    }

    return (! abort);
}


//---------------------------------------------------------------------------
// RemoveOrRename
//---------------------------------------------------------------------------


void CMyFrame::RemoveOrRename(
    CBox &box, BOOL remove, BOOL rename, bool *ReloadBoxes)
{
    if (box.GetNeverDelete()) {

        CMyMsg msg(MSG_3051, box.GetName());
        CMyApp::MsgBox(this, msg, MB_OK);

    } else if (! box.GetBoxFile().IsEmpty()) {

        CMyMsg msg(MSG_3527, box.GetName());
        CMyApp::MsgBox(this, msg, MB_OK);

    } else {

        CBoxes &boxes = CBoxes::GetInstance();

        if (rename) {

            CCreateDialog dlg(this, box.GetName());
            CString newName = dlg.GetName();
            if (newName.IsEmpty() || newName == box.GetName())
                return;

            CSbieIni::GetInstance().RenameOrCopySection(
                box.GetName(), newName, true);

        } else if (remove) {

            CMyMsg msg(MSG_3529, box.GetName());
            if (CMyApp::MsgBox(this, msg, MB_YESNO) != IDYES)
                return;

            CSbieIni::GetInstance().RemoveSection(box.GetName());
        }

        *ReloadBoxes = true;
    }
}


//---------------------------------------------------------------------------
// InitSandboxMenu
//---------------------------------------------------------------------------


void CMyFrame::InitSandboxMenu(CMenu *pMenu, int posChild, int posInsert)
{
    CMenu *sandbox = pMenu->GetSubMenu(posChild);
    int i = 0;
    while (i < (int)sandbox->GetMenuItemCount()) {
        if (sandbox->GetSubMenu(i))
            sandbox->DeleteMenu(i, MF_BYPOSITION);
        else
            ++i;
    }

    BOX_ORDER_ENTRY *box_order = BoxOrder_Read();
    if (box_order) {

        if (box_order->children) {
            CMenu *model =
                (posChild == 0) ? m_pTraySandboxMenu : m_SandboxMenu;
            InitSandboxMenu1(
                box_order->children, model, sandbox, posInsert);
        }

        BoxOrder_Free(box_order);
    }

    sandbox->DeleteMenu(ID_SANDBOX_REVEAL, MF_BYCOMMAND);
    if ((posChild != 0) && CBoxes::GetInstance().AnyHiddenBoxes())
        sandbox->AppendMenu(MF_STRING, ID_SANDBOX_REVEAL, CMyMsg(MSG_3434));
}


//---------------------------------------------------------------------------
// InitSandboxMenu1
//---------------------------------------------------------------------------


void CMyFrame::InitSandboxMenu1(
    void *_box_order, CMenu *ModelMenu, CMenu *SandboxMenu, int posInsert)
{
    BOX_ORDER_ENTRY *order_entry = (BOX_ORDER_ENTRY *)_box_order;
    CBoxes &boxes = CBoxes::GetInstance();
    int MenuIndex = 1;

    while (order_entry) {

        CMenu ChildMenu;
        CString title;

        if (order_entry->children) {

            ChildMenu.CreatePopupMenu();
            InitSandboxMenu1(
                order_entry->children, ModelMenu, &ChildMenu, 0);
            title = order_entry->name;

        } else {

            int i = boxes.GetBoxIndex(order_entry->name);
            if (i) {

                title = boxes.GetBox(i).GetName();

                InitSandboxMenu2(ModelMenu, &ChildMenu,
                             i * ID_SANDBOX_MENU_SIZE + ID_SANDBOX_MENU);
            }
        }

        if (MenuIndex <= 36) {
            title += L"\t          &";
            if (MenuIndex >= 1 && MenuIndex <= 10)
                title += (WCHAR)(L'0' + (MenuIndex % 10));
            else if (MenuIndex >= 11 && MenuIndex <= 36)
                title += (WCHAR)(L'A' + MenuIndex - 11);
            ++MenuIndex;
        }

        SandboxMenu->InsertMenu(
            posInsert, MF_BYPOSITION |
            MF_STRING | MF_POPUP, (INT_PTR)ChildMenu.GetSafeHmenu(), title);
        ++posInsert;

        ChildMenu.Detach();

        order_entry = order_entry->next;
    }
}


//---------------------------------------------------------------------------
// InitSandboxMenu2
//---------------------------------------------------------------------------


void CMyFrame::InitSandboxMenu2(CMenu *model, CMenu *child, UINT BaseId)
{
    CString title;

    child->CreatePopupMenu();
    for (int j = 0; j < model->GetMenuItemCount(); ++j) {

        model->GetMenuString(j, title, MF_BYPOSITION);

        int id = model->GetMenuItemID(j);
        if (id == -1) {

            CMenu child2;
            InitSandboxMenu2(model->GetSubMenu(j), &child2, BaseId);

            child->AppendMenu(
                MF_STRING | MF_POPUP,
                (INT_PTR)child2.GetSafeHmenu(), title);

            child2.Detach();

        } else if (id == 0) {

            child->AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)nullptr);

        } else {

            child->AppendMenu(MF_STRING, BaseId + id, title);
        }
    }
}


//---------------------------------------------------------------------------
// OnInitMenu
//---------------------------------------------------------------------------


void CMyFrame::OnInitMenu(CMenu *pMenu)
{
    if (GetMenu()->m_hMenu != pMenu->m_hMenu)
        return;

    InitSandboxMenu(pMenu, 2, 0);
}


//---------------------------------------------------------------------------
// OnContextMenu
//---------------------------------------------------------------------------


void CMyFrame::OnContextMenu(CWnd *pWnd, CPoint pt)
{
    //
    // if the right-click is on a sandbox title, record the name,
    // otherwise invoke the right-click on the child control
    //

    CBox *pBox = NULL;

    CMyListCtrl *pMyListCtrl = GetMyListCtrl();
    if (pMyListCtrl) {

        CBox &box = pMyListCtrl->GetSelectedBox(pWnd, pt);
        if (box.GetName().IsEmpty()) {

            pMyListCtrl->OnContextMenu(pWnd, pt);
            return;
        }

        pBox = &box;
    }

    //
    //
    //

    if (pBox) {

        if (pMyListCtrl && (pt.x <= 0 || pt.y <= 0))
            pMyListCtrl->GetSelectedItemPosition(pt);

        CBoxes &boxes = CBoxes::GetInstance();
        int index;
        for (index = 1; index < boxes.GetSize(); ++index)
            if (boxes.GetBox(index).GetName() == pBox->GetName())
                break;

        CMenu child;
        InitSandboxMenu2(m_SandboxMenu, &child,
                         index * ID_SANDBOX_MENU_SIZE + ID_SANDBOX_MENU);

        child.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
    }
}


//---------------------------------------------------------------------------
// OnTaskbarCreated
//---------------------------------------------------------------------------


LRESULT CMyFrame::OnTaskbarCreated(WPARAM wParam, LPARAM lParam)
{
    CMyApp::CreateTrayIcon(this, m_hIconPtr, CString());
    return 0;
}


//---------------------------------------------------------------------------
// OnTrayIcon
//---------------------------------------------------------------------------


LRESULT CMyFrame::OnTrayIcon(WPARAM wParam, LPARAM lParam)
{
    if (m_finder)
        return 0;

    WORD msg = LOWORD(lParam);

    //
    // getting started wizard
    //

    if (m_GettingStartedWindow && (msg == WM_LBUTTONDOWN
                || msg == WM_LBUTTONUP || msg == WM_LBUTTONDBLCLK)) {

        m_GettingStartedWindow->PostMessage(
            WM_COMMAND, ID_SANDBOX_MENU | ID_SANDBOX_EXPLORE);

        return 0;
    }

    //
    // right button
    //

    if (msg == WM_RBUTTONDOWN || msg == WM_MOUSEMOVE) {

        if (! GetCursorPos(&m_TrayPoint))
            m_TrayPoint = (CPoint)GetMessagePos();
    }

    if (msg == WM_RBUTTONUP) {

        if (m_TrayActive)
            return 0;
        m_TrayActive = TRUE;

        //
        // prepare menu
        //

        InitSandboxMenu(m_pTrayMenu, 0, 2);

        CMenu *pMenu = m_pTrayMenu->GetSubMenu(0);
        CString msg;
        if (m_hidden)
            msg = CMyMsg(MSG_3485); // Show Window
        else
            msg = CMyMsg(MSG_3486); // Hide Window
        pMenu->ModifyMenu(ID_SHOW_WINDOW, MF_STRING, ID_SHOW_WINDOW, msg);
        pMenu->SetDefaultItem(0, TRUE);

        CCmdUI cmdUI;
        cmdUI.m_pMenu = pMenu;
        cmdUI.m_nID = ID_EXIT;
        while (1) {

            UINT id = pMenu->GetMenuItemID(cmdUI.m_nIndex);
            if (id == ID_DISABLE_FORCE)
            {
                cmdUI.m_nID = id;
                cmdUI.m_nIndexMax = pMenu->GetMenuItemCount();
                if (id == ID_DISABLE_FORCE)
                    OnUpdDisableForce(&cmdUI);
            } else if (id == ID_EXIT)
                break;

            ++cmdUI.m_nIndex;
        }
        cmdUI.m_nIndexMax = pMenu->GetMenuItemCount();

        //
        //
        //

        bool apply_right_order = false;

        HMONITOR hMonitor =
                        MonitorFromPoint(m_TrayPoint, MONITOR_DEFAULTTONULL);
        if (hMonitor) {
            MONITORINFO mi;
            memzero(&mi, sizeof(MONITORINFO));
            mi.cbSize = sizeof(MONITORINFO);
            if (GetMonitorInfo(hMonitor, &mi)) {
                int ll = mi.rcMonitor.left, rr = mi.rcMonitor.right;
                if ((m_TrayPoint.x - ll) > ((rr - ll) * 2 / 3))
                    apply_right_order = true;
            }
        }

        if (apply_right_order) {
            int count = pMenu->GetMenuItemCount();
            MENUITEMINFO mii;
            memzero(&mii, sizeof(MENUITEMINFO));
            mii.cbSize = sizeof(MENUITEMINFO);
            for (int i = 0; i < count; ++i) {
                mii.fMask = MIIM_FTYPE | MIIM_SUBMENU;
                if (::GetMenuItemInfo(pMenu->m_hMenu, i, TRUE, &mii)) {
                    if (mii.hSubMenu) {
                        mii.fMask = MIIM_FTYPE;
                        mii.fType |= MFT_RIGHTORDER;
                        ::SetMenuItemInfo(pMenu->m_hMenu, i, TRUE, &mii);
                    }
                }
            }
        }

        //
        // apply visual style
        //

        CMenuXP::SetXPLookNFeelRecursive(this, *pMenu, true, false);

        //
        // display menu
        //

        if (m_GettingStartedWindow) {
            m_GettingStartedWindow->PostMessage(
                WM_COMMAND, ID_SANDBOX_MENU | ID_SANDBOX_TERMINATE);
        }

        SetForegroundWindow();

        UINT nID = (UINT)pMenu->TrackPopupMenu(
            TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD,
            m_TrayPoint.x, m_TrayPoint.y, this, NULL);

        m_TrayActive = FALSE;

        //
        // invoke selected command
        //

        if (m_GettingStartedWindow && nID != ID_EXIT) {

            //
            // if we have a getting started window, send it either
            // a "delete defaultbox" or an "invalid command" message
            //

            WORD msg = ID_SANDBOX_EXPLORE;
            int sandbox = (nID - ID_SANDBOX_MENU) / ID_SANDBOX_MENU_SIZE;
            int command = (nID - ID_SANDBOX_MENU) % ID_SANDBOX_MENU_SIZE;
            if (command == ID_SANDBOX_DELETE) {
                CBoxes &boxes = CBoxes::GetInstance();
                CBox &box = boxes.GetBox(sandbox);
                if (box.GetName().CompareNoCase(CBoxes::m_DefaultBox) == 0)
                    msg = ID_SANDBOX_DELETE;
            }
            m_GettingStartedWindow->PostMessage(
                WM_COMMAND, ID_SANDBOX_MENU | msg);

        } else if (nID) {

            PostMessage(WM_COMMAND, MAKEWPARAM(nID, 'tz'), 0);
        }

        return 0;
    }

    //
    // left button double click
    //

    if (msg == WM_LBUTTONDBLCLK) {

        OnCmdShowWindow();

        return 0;
    }

    return 1;
}


//---------------------------------------------------------------------------
// RefreshTrayIcon
//---------------------------------------------------------------------------


void CMyFrame::RefreshTrayIcon(BOOL ForceDeleteIcon)
{
    HICON hIconPtr = 0;
    CString info;

    //
    // check if delete sandbox is in progress
    //

    CString EventName;
    EventName.Format(
        SANDBOXIE L"_Delete_Sandbox_Session_%d", CMyApp::m_session_id);
    HANDLE hEvent = OpenEvent(GENERIC_READ, FALSE, EventName);
    if (hEvent) {
        CloseHandle(hEvent);
        ForceDeleteIcon = TRUE;
    }

    if (ForceDeleteIcon) {

        hIconPtr = m_hIconDelete;
        info = CMyMsg(MSG_3309);
    }

    if (! hIconPtr) {

        //
        // otherwise check sandbox activity and disable force process
        //

        ULONG stateDfp;
        SbieApi_DisableForceProcess(NULL, &stateDfp);

        if (CBoxes::GetInstance().AnyActivity()) {
            if (stateDfp)
                hIconPtr = m_hIconFullDfp;
            else
                hIconPtr = m_hIconFull;
        } else {
            if (stateDfp)
                hIconPtr = m_hIconEmptyDfp;
            else
                hIconPtr = m_hIconEmpty;
        }
    }

    //
    // refresh icon
    //

    if (hIconPtr != m_hIconPtr) {

        m_hIconPtr = hIconPtr;
        CMyApp::ChangeTrayIcon(m_hIconPtr, info);
    }
}


//---------------------------------------------------------------------------
// OnTimer
//---------------------------------------------------------------------------


void CMyFrame::OnTimer(UINT_PTR nIDEvent)
{
    static ULONG _counter = 0;

    if (nIDEvent == ID_TIMER) {

        bool inModalState = CBaseDialog::InModalState();

        //
        // run software compatibility?
        //

        if (m_AutoRunSoftCompat && (! inModalState)) {

            OnCmdThirdParty();
            return;
        }

        //
        // first time?
        //

        /*if (m_ShowWelcome && (! inModalState)) {

            m_ShowWelcome = FALSE;
            CUserSettings::GetInstance().SetBool(_ShowWelcome, FALSE);

            m_ReSyncShortcuts = FALSE;
            CShellDialog::Sync();

            CGettingStartedWizard wizard(this);
            return;
        }*/

        //
        // resync shortcuts?  usually Sandboxie Control does not resync
        // the Run Sandboxed shortcuts on startup, except when the
        // /sync parameter is passed by the installer
        //

        if (m_ReSyncShortcuts) {

            m_ReSyncShortcuts = FALSE;
            CShellDialog::Sync();
        }

        //
        // save settings every 10 minutes
        //

        ++_counter;
        if ((_counter % 600) == 0)
            SaveSettings();

		//
		// update check
		//

		if (! m_hidden)
		{
			__int64 NextUpdateCheck;
			CUserSettings::GetInstance().GetNum64(_NextUpdateCheck, NextUpdateCheck, 0);
			if(NextUpdateCheck == 0)
				CUserSettings::GetInstance().SetNum64(_NextUpdateCheck, time(NULL) + 7 * 24 * 60 * 60);
			else if(NextUpdateCheck != -1 && time(NULL) >= NextUpdateCheck)
			{
				BOOL UpdateCheckNotify;
				CUserSettings::GetInstance().GetBool(_UpdateCheckNotify, UpdateCheckNotify, TRUE);
				if (UpdateCheckNotify)
				{
					static BOOLEAN update_dlg_open = FALSE;
					if (!update_dlg_open) {
						update_dlg_open = TRUE;
						CUpdateDialog dlg(this);
						if(dlg.DoModal() == 0)
							CUserSettings::GetInstance().SetNum64(_NextUpdateCheck, time(NULL) + 1 * 24 * 60 * 60);
						update_dlg_open = FALSE;
					}
				}
				else
				{
					CUserSettings::GetInstance().SetNum64(_NextUpdateCheck, time(NULL) + 1 * 24 * 60 * 60);
					CUpdater::GetInstance().CheckUpdates(this, false);
				}
			}
		}

        //
        // refresh processes
        //

        CBoxes &boxes = CBoxes::GetInstance();
        boxes.RefreshProcesses();

        //
        // reflect state in tray icon
        //

        RefreshTrayIcon();

        //
        // refresh message dialog
        //

        ++m_inTimer;
        m_msgdlg->OnTimer();
        --m_inTimer;

        if (m_destroyed)
            return;

        //
        // refresh views if not minimized
        //

        if (! m_finder && ! m_hidden) {

            if (m_view == ID_VIEW_PROCESS)
                m_proclist.RefreshContent();
        }

        //
        // prepare for auto delete
        //

        for (int index = 1; index < boxes.GetSize(); ++index) {
            CBox &box = boxes.GetBox(index);
            CBoxProc &boxproc = box.GetBoxProc();
            bool NoActivityNow = (boxproc.GetProcessCount() == 0);
            bool HadActivityBefore = (boxproc.GetOldProcessCount() != 0);
            if (NoActivityNow && HadActivityBefore) {

                //
                // re-enable Immediate Recovery, if was disabled
                //

                box.SetImmediateRecoveryState(TRUE, TRUE);

                //
                // delete contents if needed, otherwise remind about
                // period deletion
                //

                if (! box.GetNeverDelete()) {

                    if (box.GetAutoDelete())
                        m_DeleteList.AddTail(box.GetName());
                    else
                        CheckShouldDelete(box);
                }
            }
        }

        //
        // invoke auto delete
        //

        if (! inModalState) {

            while (! m_DeleteList.IsEmpty()) {
                CString &boxName = m_DeleteList.RemoveHead();
                CBox &box = CBoxes::GetInstance().GetBox(boxName);
                if (box.GetName() == boxName) {
                    BOOL spawned;
                    CDeleteDialog dlg(this, boxName, TRUE, TRUE, &spawned);
                    if (spawned)
                        RefreshTrayIcon(TRUE);
                }

                while (1) {
                    POSITION pos = m_DeleteList.Find(boxName);
                    if (! pos)
                        break;
                    m_DeleteList.RemoveAt(pos);
                }
            }
        }
    }
}


//---------------------------------------------------------------------------
// OnEraseBkgnd
//---------------------------------------------------------------------------


BOOL CMyFrame::OnEraseBkgnd(CDC *pDC)
{
    return TRUE;
}


//---------------------------------------------------------------------------
// OnSize
//---------------------------------------------------------------------------


void CMyFrame::OnSize(UINT nType, int cx, int cy)
{
    if ((nType == SIZE_RESTORED && IsWindowVisible()) ||
            nType == SIZE_MAXIMIZED || nType == SIZE_MINIMIZED)
        m_hidden = FALSE;

    if (! m_view)
        return;

    if (m_view != m_view_old) {

        if (m_view == ID_VIEW_PROCESS)
            m_filelist.ShowWindow(SW_HIDE);

        if (m_view == ID_VIEW_FILES)
            m_proclist.ShowWindow(SW_HIDE);

        m_view_old = m_view;
    }

    CMyListCtrl *pMyListCtrl = GetMyListCtrl();
    if (pMyListCtrl) {

        pMyListCtrl->RefreshContent();

        CRect rc;
        GetClientRect(&rc);
        pMyListCtrl->SetWindowPos(
            NULL, rc.left, rc.top, rc.right, rc.bottom,
            SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);

        pMyListCtrl->ShowWindow(SW_SHOW);
        pMyListCtrl->SetFocus();
    }
}


//---------------------------------------------------------------------------
// OnSizing
//---------------------------------------------------------------------------


void CMyFrame::OnSizing(UINT nSide, LPRECT lpRect)
{
    OnSize(SIZE_RESTORED, 0, 0);
}


//---------------------------------------------------------------------------
// OnClose
//---------------------------------------------------------------------------


void CMyFrame::OnClose()
{
    CUserSettings &settings = CUserSettings::GetInstance();

    if (! CMyApp::m_hwndTray) {
        OnTaskbarCreated(0, 0);
        if (! CMyApp::m_hwndTray) {
            ShowWindow(SW_MINIMIZE);
            CSbieIni::GetInstance().ForgetPasswordNow();
            return;
        }
    }

    BOOL tip;
    settings.GetBool(_HideWindowNotify, tip, TRUE);
    if (tip) {

        int rv = CMyApp::MsgCheckBox(this, MSG_3511, 0, MB_OK);

        if (rv < 0) {
            rv = -rv;
            settings.SetBool(_HideWindowNotify, FALSE);
        }
    }

    m_hidden = TRUE;
    ShowWindow(SW_HIDE);
    CSbieIni::GetInstance().ForgetPasswordNow();
}


//---------------------------------------------------------------------------
// OnSetFocus
//---------------------------------------------------------------------------


void CMyFrame::OnSetFocus(CWnd *pOldWnd)
{
    if (! m_finder && ! m_hidden) {
        CMyListCtrl *pMyListCtrl = GetMyListCtrl();
        if (pMyListCtrl) {
            pMyListCtrl->RefreshContent();
            pMyListCtrl->SetFocus();
        }
    }
}


//---------------------------------------------------------------------------
// OnShowWindow
//---------------------------------------------------------------------------


void CMyFrame::OnShowWindow(BOOL bShow, UINT nStatus)
{
    if (bShow && (! m_hidden))
        return;
    if ((! bShow) && m_hidden)
        return;
    if (CBaseDialog::InModalState()) {
        m_hidden = ! bShow;
        return;
    }
    OnCmdShowWindow();
}


//---------------------------------------------------------------------------
// OnWindowPosChanged
//---------------------------------------------------------------------------


void CMyFrame::OnWindowPosChanged(WINDOWPOS *lpwndpos)
{
    // if we're supposed to be topmost, and a child popup turned it off, turn it back on
    if (m_AlwaysOnTop && (GetWindowLongW(m_hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST) == 0)
    {
        SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
    CWnd::OnWindowPosChanged(lpwndpos);
}


//---------------------------------------------------------------------------
// OnDropFiles
//---------------------------------------------------------------------------


void CMyFrame::OnDropFiles(HDROP hDrop)
{
    WCHAR path[512];
    if (! DragQueryFile(hDrop, 0, path, sizeof(path) / sizeof(WCHAR) - 1))
        path[0] = L'\0';
    if (path[0]) {
        CString QuotedPath = L"\"" + CString(path) + L"\"";
        CMyApp::RunStartExe(QuotedPath, CString());
    }
}


//---------------------------------------------------------------------------
// OnDeviceChange
//---------------------------------------------------------------------------


BOOL CMyFrame::OnDeviceChange(UINT nEventType, DWORD_PTR dwData)
{
    SbieDll_DeviceChange(nEventType, dwData);
    return TRUE;
}


//---------------------------------------------------------------------------
// OnDeviceChange
//---------------------------------------------------------------------------


void CMyFrame::CheckShouldDelete(CBox &box)
{
    static ULONG last_ticks = 0;

    if (last_ticks && (GetTickCount() - last_ticks <= 1000 * 60 * 60)) {
        // don't warn more than once an hour
        return;
    }
    last_ticks = GetTickCount();

    FILETIME ft;
    ULARGE_INTEGER ul;

    if (box.GetBoxFile().GetBoxCreationTime(&ft)) {
        ul.LowPart  = ft.dwLowDateTime;
        ul.HighPart = ft.dwHighDateTime;
        ULONG DaysThen = (ULONG)(ul.QuadPart / SECONDS(86400));

        GetSystemTimeAsFileTime(&ft);
        ul.LowPart  = ft.dwLowDateTime;
        ul.HighPart = ft.dwHighDateTime;
        ULONG DaysNow = (ULONG)(ul.QuadPart / SECONDS(86400));

        //WCHAR txt[128]; wsprintf(txt, L"Then = %d Now = %d Diff = %d\n", DaysThen, DaysNow, DaysNow - DaysThen);
        //OutputDebugString(txt);

        if (DaysNow > (DaysThen + 7)) {

            CUserSettings &settings = CUserSettings::GetInstance();
            BOOL tip;
            settings.GetBool(_ShouldDeleteNotify, tip, TRUE);

            if (tip) {

                CString num;
                num.Format(L"%d", DaysNow - DaysThen);
                CMyMsg text(MSG_3539, box.GetName(), num);
                int rv = CMyApp::MsgCheckBox(this, text, 0, MB_OK);
                if (rv < 0)
                    settings.SetBool(_ShouldDeleteNotify, FALSE);
            }
        }
    }
}

/*LRESULT CMyFrame::OnUpdateResult(WPARAM wParam, LPARAM lParam)
{

	return 0;
}*/
