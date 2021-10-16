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
// Getting Started Wizard
//---------------------------------------------------------------------------


#include "stdafx.h"
#include "MyApp.h"
#include "GettingStartedWizard.h"
#include "Boxes.h"
#include "MessageDialog.h"
#include "DeleteDialog.h"
#include "MyFrame.h"
#include "apps/common/FontStore.h"
#include "apps/common/MyGdi.h"


//---------------------------------------------------------------------------
// CShadowWnd
//---------------------------------------------------------------------------


class CShadowWnd : public CWnd
{

protected:

    DECLARE_MESSAGE_MAP()

    afx_msg void OnPaint();

public:

    HRGN m_hrgn;

    CShadowWnd();

    void ExcludeWindows(HWND rectwnd, HWND circlewnd);

};


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define ID_TIMER            10001
#define ID_IMAGE_PAGE2      10201
#define ID_DESKTOP_PAGE3    10301
#define IDNEXT              0x3024


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CGettingStartedPage, CLayoutPropertyPage)

    ON_NOTIFY(NM_CUSTOMDRAW, ID_IMAGE_PAGE2, OnImageFrame_Page2)

    ON_COMMAND(ID_DESKTOP_PAGE3, OnShowDesktop_Page3)
    ON_WM_CTLCOLOR()
    ON_WM_TIMER()

    ON_COMMAND(ID_SANDBOX_MENU | ID_SANDBOX_RECOVER,
                                        OnImmediateRecover_Page4)

    ON_COMMAND(ID_SANDBOX_MENU | ID_SANDBOX_EXPLORE,
                                        OnShowBalloon_Page5)
    ON_COMMAND(ID_SANDBOX_MENU | ID_SANDBOX_TERMINATE,
                                        OnHideBalloon_Page5)
    ON_COMMAND(ID_SANDBOX_MENU | ID_SANDBOX_DELETE,
                                        OnDeleteContents_Page5)

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CGettingStartedPage::CGettingStartedPage(int page_num)
    : CMyWizardPage(page_num)
{
    memzero(&u, sizeof(u));
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CGettingStartedPage::~CGettingStartedPage()
{
    if (m_page_num == 2) {

        delete u.page2.m_legend_bitmap;
    }

    if (m_page_num == 3 || m_page_num == 5) {

        KillTimer(ID_TIMER);
    }

    if (m_page_num == 5) {

        CShadowWnd *shadow_wnd = (CShadowWnd *)u.page5.m_shadow_wnd;
        delete shadow_wnd;
    }
}


//---------------------------------------------------------------------------
// OnInitDialog
//---------------------------------------------------------------------------


BOOL CGettingStartedPage::OnInitDialog()
{
    typedef void (CGettingStartedPage::*P_OnInitDialogProc)(void);
    P_OnInitDialogProc init_procs[] = { NULL,
        &CGettingStartedPage::OnInitDialog_Page1,
        &CGettingStartedPage::OnInitDialog_Page2,
        &CGettingStartedPage::OnInitDialog_Page3,
        &CGettingStartedPage::OnInitDialog_Page4,
        &CGettingStartedPage::OnInitDialog_Page5,
        &CGettingStartedPage::OnInitDialog_Page6
    };
    (this->*(init_procs[m_page_num]))();

    return CMyWizardPage::OnInitDialog();
}


//---------------------------------------------------------------------------
// OnInitDialog_Page1
//---------------------------------------------------------------------------


void CGettingStartedPage::OnInitDialog_Page1()
{
    SetPageTitle(MSG_7851);

    CreateStatic(MSG_7852, CPoint(5, 15), CSize(90, 60));

    CBox &box = CBoxes::GetInstance().GetBox(CBoxes::m_DefaultBox);
    if (box.GetName().CompareNoCase(CBoxes::m_DefaultBox) != 0) {

        CreateStatic(MSG_7853, CPoint(5, 80), CSize(90, 20));
    }
}


//---------------------------------------------------------------------------
// OnInitDialog_Page2
//---------------------------------------------------------------------------


void CGettingStartedPage::OnInitDialog_Page2()
{
    SetPageTitle(MSG_7854);

    u.page2.m_legend_bitmap = new CBitmap();
    u.page2.m_legend_bitmap->Attach(
                                MyGdi_CreateFromResource(L"WELCOME_LEGEND"));

    CStatic *image = (CStatic *)CreateChild(
        ID_IMAGE_PAGE2, L"STATIC", 0, SS_CENTER,
        CPoint(15, 17), CSize(80, 80));

    u.page2.m_animation = new CAnimatedBitmap();
    u.page2.m_animation->Init(image->UnsubclassWindow());
    u.page2.m_animation->SetImage(L"WELCOME_ANIM");
    u.page2.m_animation->Animate(true);
    m_layout.ReplaceChild(image, u.page2.m_animation);
}


//---------------------------------------------------------------------------
// OnInitDialog_Page3
//---------------------------------------------------------------------------


void CGettingStartedPage::OnInitDialog_Page3()
{
    SetPageTitle(MSG_7855);

    CreateStatic(MSG_7856, CPoint(5, 15), CSize(90, 20));

    u.page3.m_runsandbox_icon =
                        ::LoadIcon(AfxGetInstanceHandle(), L"TRAYICON_FULL");
    CStatic *icon = (CStatic *)CreateChild(
                    -1, L"STATIC", 0, SS_ICON, CPoint(45, 35), CSize(1, 1));
    m_layout.SetMinMaxChildObject(icon, CSize(64, 64), CSize(64, 64));
    u.page3.m_runsandbox_pwnd = icon;

    CString link_name = CMyMsg(MSG_3698);
    int dot = link_name.ReverseFind(L'.');
    if (dot != -1)
        link_name = link_name.Left(dot);
    CStatic *icon_name = (CStatic *)CreateChild(
        -1, L"STATIC", 0, SS_CENTER, CPoint(5, 55), CSize(90, 20));
    icon_name->SetWindowText(link_name);
    icon_name->SetFont(CFontStore::Get(L"Tahoma", 12, FW_NORMAL));

    CButton *button = (CButton *)CreateChild(
        ID_DESKTOP_PAGE3, L"BUTTON", 0, BS_PUSHBUTTON | BS_CENTER,
        CPoint(15, 75), CSize(70, 10));
    button->SetWindowText(CMyMsg(MSG_7857));
    button->SetFont(CFontStore::Get(L"Tahoma", 10, FW_NORMAL));

    CBox &box = CBoxes::GetInstance().GetBox(CBoxes::m_DefaultBox);
    if (box.GetName().CompareNoCase(CBoxes::m_DefaultBox) == 0)
        SetTimer(ID_TIMER, 5 * 1000, NULL);
}


//---------------------------------------------------------------------------
// OnInitDialog_Page4
//---------------------------------------------------------------------------


void CGettingStartedPage::OnInitDialog_Page4()
{
    SetPageTitle(MSG_7858);

    CreateStatic(MSG_7859, CPoint(5, 15), CSize(90, 80));

    CMessageDialog::m_GettingStartedWindow = this;
}


//---------------------------------------------------------------------------
// OnInitDialog_Page5
//---------------------------------------------------------------------------


void CGettingStartedPage::OnInitDialog_Page5()
{
    SetPageTitle(MSG_7860);

    CreateStatic(MSG_7861, CPoint(5, 15), CSize(90, 40));

    CString msg = CMyMsg(MSG_7862) + L"\n\n" + CMyMsg(MSG_7863);
    CreateStatic(msg, CPoint(5, 50), CSize(90, 50));

    OSVERSIONINFO osvi;
    memzero(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx((OSVERSIONINFO *)&osvi);
    if ((osvi.dwMajorVersion == 10) || (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion >= 1))
        // Windows 7 and above
        u.page5.m_open_tray = true;

    u.page5.m_shadow_wnd = new CShadowWnd();

    CMyFrame::m_GettingStartedWindow = this;
    OnShowBalloon_Page5();
}


//---------------------------------------------------------------------------
// OnInitDialog_Page6
//---------------------------------------------------------------------------


void CGettingStartedPage::OnInitDialog_Page6()
{
    SetPageTitle(MSG_7864);

    CreateStatic(MSG_7865, CPoint(5, 15), CSize(90, 70));
}


//---------------------------------------------------------------------------
// OnKillActive
//---------------------------------------------------------------------------


BOOL CGettingStartedPage::OnKillActive()
{
    if (m_page_num == 5 && u.page5.m_shadow_wnd)
        u.page5.m_shadow_wnd->ShowWindow(SW_HIDE);

    if (CMyFrame::m_GettingStartedWindow)
        CMyApp::BalloonTrayIcon(CString(), 10);

    CMessageDialog::m_GettingStartedWindow = NULL;
    CMyFrame::m_GettingStartedWindow = NULL;

    KillTimer(ID_TIMER);

    CGettingStartedWizard *wizard = (CGettingStartedWizard *)GetParent();
    wizard->FlashNextButton(false);

    return CLayoutPropertyPage::OnKillActive();
}


//---------------------------------------------------------------------------
// OnKillActive
//---------------------------------------------------------------------------


void CGettingStartedPage::OnCancel()
{
    if (m_page_num == 5 && u.page5.m_shadow_wnd)
        u.page5.m_shadow_wnd->ShowWindow(SW_HIDE);

    if (CMessageDialog::m_GettingStartedWindow)
        CMessageDialog::m_GettingStartedWindow = NULL;

    if (CMyFrame::m_GettingStartedWindow) {
        CMyApp::BalloonTrayIcon(CString(), 10);
        CMyFrame::m_GettingStartedWindow = NULL;
    }

    return CLayoutPropertyPage::OnCancel();
}


//---------------------------------------------------------------------------
// OnImageFrame_Page2
//---------------------------------------------------------------------------


void CGettingStartedPage::OnImageFrame_Page2(NMHDR *pNMHDR, LRESULT *pResult)
{
    BITMAP info;
    if (u.page2.m_legend_bitmap->GetBitmap(&info)) {
        CAnimatedBitmap_NM *nm = (CAnimatedBitmap_NM *)pNMHDR;

        CDC bmp_dc;
        bmp_dc.CreateCompatibleDC(nm->dc);
        CBitmap *bmp_old = bmp_dc.SelectObject(u.page2.m_legend_bitmap);

        nm->dc->BitBlt(10, 10, info.bmWidth, info.bmHeight, &bmp_dc, 0, 0, SRCCOPY);

        bmp_dc.SelectObject(bmp_old);
    }
}


//---------------------------------------------------------------------------
// OnShowDesktop_Page3
//---------------------------------------------------------------------------


void CGettingStartedPage::OnShowDesktop_Page3()
{
    CWnd *traywnd = FindWindow(L"Shell_TrayWnd", NULL);
    if (traywnd)
        traywnd->SendMessage(WM_COMMAND, 407, 0);
}


//---------------------------------------------------------------------------
// OnCtlColor
//---------------------------------------------------------------------------


HBRUSH CGettingStartedPage::OnCtlColor(
    CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
    if (m_page_num == 3 && pWnd == u.page3.m_runsandbox_pwnd) {

        pDC->SetBkMode(TRANSPARENT);
        DrawIconEx(pDC->m_hDC, 0, 0, u.page3.m_runsandbox_icon,
                   64, 64, 0, NULL, DI_NORMAL);
        return (HBRUSH)::GetStockObject(NULL_BRUSH);
    }

    return CLayoutPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
}


//---------------------------------------------------------------------------
// OnTimer
//---------------------------------------------------------------------------


void CGettingStartedPage::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == ID_TIMER && m_page_num == 3) {

        int process_count =
                CBoxes::GetInstance().GetBox(CBoxes::m_DefaultBox)
                                        .GetBoxProc().GetProcessCount();
        if (process_count) {

            KillTimer(ID_TIMER);

            CGettingStartedWizard *wizard =
                            (CGettingStartedWizard *)GetParent();
            wizard->FlashNextButton(true);
        }
    }

    if (nIDEvent == ID_TIMER && m_page_num == 5) {

        OnShowBalloon_Page5();
    }

    return CLayoutPropertyPage::OnTimer(nIDEvent);
}


//---------------------------------------------------------------------------
// OnImmediateRecover_Page4
//---------------------------------------------------------------------------


void CGettingStartedPage::OnImmediateRecover_Page4()
{
    CMessageDialog::m_GettingStartedWindow = NULL;

    CGettingStartedWizard *wizard =
                    (CGettingStartedWizard *)GetParent();
    wizard->FlashNextButton(true);
}


//---------------------------------------------------------------------------
// OnShowBalloon_Page5
//---------------------------------------------------------------------------


void CGettingStartedPage::OnShowBalloon_Page5()
{
    if (CMyFrame::m_GettingStartedWindow) {

        if (! u.page5.m_tray_opened) {

            //
            // find the button on the tray that it used to show/expand
            // hidden notification icons and use that as a target for
            // the arrow window
            //

            HWND ButtonWnd = FindTrayButton(NULL, NULL);

            CShadowWnd *shadow_wnd = (CShadowWnd *)u.page5.m_shadow_wnd;
            shadow_wnd->ExcludeWindows(GetParent()->m_hWnd, ButtonWnd);

            //
            // on Windows 7, if the tray button is visible, then some
            // icons are hidden in the icon overflow area of the tray.
            // in case our icon is hidden, we fake a click on the button
            // so the icon overflow area is made visible
            //

            if (u.page5.m_open_tray &&
                    ButtonWnd && ::IsWindowVisible(ButtonWnd)) {

                bool OverflowIconsAlreadyVisible = false;
                HWND NotifyAreaWnd =
                            ::FindWindow(L"NotifyIconOverflowWindow", NULL);
                if (NotifyAreaWnd && ::IsWindowVisible(NotifyAreaWnd))
                    OverflowIconsAlreadyVisible = true;

                HWND TrayNotifyWnd = ::GetParent(ButtonWnd);
                WPARAM wParam = ::GetDlgCtrlID(ButtonWnd);
                LPARAM lParam = (LPARAM)ButtonWnd;
                ::SendMessage(TrayNotifyWnd, WM_COMMAND, wParam, lParam);
                if (OverflowIconsAlreadyVisible)
                    ::SendMessage(TrayNotifyWnd, WM_COMMAND, wParam, lParam);
            }

            //
            // request a secondary WM_TIMER which to show the balloon
            //

            u.page5.m_tray_opened = true;
            PostMessage(WM_TIMER, ID_TIMER, 0);

        } else {

            u.page5.m_tray_opened = false;

            SetTimer(ID_TIMER, 5 * 1000, NULL);

            CString tip = L"\n" + CMyMsg(MSG_7863);
            CMyApp::BalloonTrayIcon(tip, 10);

        }
    }
}


//---------------------------------------------------------------------------
// OnHideBalloon_Page5
//---------------------------------------------------------------------------


void CGettingStartedPage::OnHideBalloon_Page5()
{
    if (CMyFrame::m_GettingStartedWindow) {
        KillTimer(ID_TIMER);
        CMyApp::BalloonTrayIcon(CString(), 10);
    }
}


//---------------------------------------------------------------------------
// OnDeleteContents_Page5
//---------------------------------------------------------------------------


void CGettingStartedPage::OnDeleteContents_Page5()
{
    if (m_page_num == 5 && u.page5.m_shadow_wnd)
        u.page5.m_shadow_wnd->ShowWindow(SW_HIDE);

    CMyFrame::m_GettingStartedWindow = NULL;

    GetParent()->ShowWindow(SW_HIDE);
    BOOL spawned;
    CDeleteDialog dlg(this, CBoxes::m_DefaultBox, TRUE, FALSE, &spawned);
    if (! m_hWnd)       // in case window was destroyed by exit command
        return;
    GetParent()->ShowWindow(SW_SHOW);

    CGettingStartedWizard *wizard = (CGettingStartedWizard *)GetParent();
    wizard->FlashNextButton(true);
}


//---------------------------------------------------------------------------
// Find
//---------------------------------------------------------------------------


HWND CGettingStartedPage::FindTrayButton(HWND hwnd, LPARAM lParam)
{
    if (lParam) {

        WCHAR clsnm[64];
        int n = GetClassName(hwnd, clsnm, 64);
        if (n == 6 && _wcsicmp(clsnm, L"BUTTON") == 0) {
            int id = ::GetDlgCtrlID(hwnd);
            if (id == 0x5DE) {
                *(HWND *)lParam = hwnd;
                return NULL;
            }
        }
        return (HWND)TRUE;

    } else {

        HWND hwndResult = NULL;

        HWND ShellTrayWnd = ::FindWindow(L"Shell_TrayWnd", NULL);
        if (ShellTrayWnd) {
            HWND TrayNotifyWnd = ::FindWindowEx(
                        ShellTrayWnd, NULL, L"TrayNotifyWnd", NULL);
            if (TrayNotifyWnd) {
                EnumChildWindows(TrayNotifyWnd, (WNDENUMPROC)FindTrayButton,
                                 (LPARAM)&hwndResult);
            }
        }

        return hwndResult;
    }
}


//---------------------------------------------------------------------------
// CGettingStartedWizard Constructor
//---------------------------------------------------------------------------


CGettingStartedWizard::CGettingStartedWizard(CWnd *pParentWnd)
    : CMyWizard(pParentWnd, 7850)
{
    for (int i = 1; i <= 6; ++i)
        AddPage(new CGettingStartedPage(i));

    pParentWnd->ShowWindow(SW_HIDE);
    DoModal();
    pParentWnd->ShowWindow(SW_SHOW);

    CMessageDialog::m_GettingStartedWindow = NULL;
    CMyFrame::m_GettingStartedWindow = NULL;
}


//---------------------------------------------------------------------------
// FlashNextButton
//---------------------------------------------------------------------------


void CGettingStartedWizard::FlashNextButton(bool enable)
{
    if (enable) {
        ShowWindow(SW_RESTORE);
        SetForegroundWindow();
        m_pBaseDialog->FlashTitle();
    }

    if (! m_next_button.m_hWnd)
        m_next_button.Init(this, IDNEXT);

    m_next_button.EnableFlashing(enable);
}


//---------------------------------------------------------------------------
// CArrowWnd Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CShadowWnd, CWnd)

    ON_WM_PAINT()

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// CShadowWnd Constructor
//---------------------------------------------------------------------------


CShadowWnd::CShadowWnd()
{
    m_hrgn = NULL;

    CRect rc;
    GetDesktopWindow()->GetWindowRect(&rc);
    CWnd::CreateEx(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE
                                 | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
                   NULL, NULL, WS_POPUP | WS_CLIPCHILDREN | WS_VISIBLE,
                   2, 2, rc.Width() - 2 * 2, rc.Height() - 2 * 2,
                   NULL, NULL, NULL);
    ::SetLayeredWindowAttributes(m_hWnd, 0, 192, LWA_ALPHA);
}


//---------------------------------------------------------------------------
// CShadowWnd::ExcludeWindows
//---------------------------------------------------------------------------


void CShadowWnd::ExcludeWindows(HWND rectwnd, HWND circlewnd)
{
    CRect rc;
    GetDesktopWindow()->GetWindowRect(&rc);
    HRGN rgn1 = CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);
    int desk_right = rc.right;

    if (::IsWindowVisible(rectwnd) & (! ::IsIconic(rectwnd)))
        ::GetWindowRect(rectwnd, &rc);
    else
        memzero(&rc, sizeof(RECT));
    HRGN rgn2 = CreateRectRgn(
        rc.left - 4, rc.top - 4, rc.right + 4, rc.bottom + 4);

    if (circlewnd) {

        ::GetWindowRect(circlewnd, &rc);
        int rad = desk_right - rc.right;
        if (GetWindowLong(circlewnd, GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
            rad = rc.right;
        HRGN rgn3 = CreateEllipticRgn(rc.left - rad, rc.top - rad,
                                      rc.right + rad, rc.bottom + rad);
        CombineRgn(rgn2, rgn2, rgn3, RGN_OR);
        DeleteObject(rgn3);
    }

    CombineRgn(rgn1, rgn1, rgn2, RGN_XOR);
    DeleteObject(rgn2);

    SetWindowRgn(rgn1, TRUE);
}


//---------------------------------------------------------------------------
// CShadowWnd::OnPaint
//---------------------------------------------------------------------------


void CShadowWnd::OnPaint()
{
    CRect rc;
    GetClientRect(&rc);

    PAINTSTRUCT ps;
    CDC *dc = BeginPaint(&ps);
    dc->FillSolidRect(rc, 0);
    EndPaint(&ps);
}
