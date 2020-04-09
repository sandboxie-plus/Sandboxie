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
// Base Dialog Box
//---------------------------------------------------------------------------


#include "stdafx.h"
#include "MyApp.h"
#include "BaseDialog.h"
#include "apps/common/CommonUtils.h"


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CBaseDialog, CDialog)

    ON_WM_DESTROY()
    ON_WM_SIZE()
    ON_WM_NCLBUTTONDOWN()

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CBaseDialog::CBaseDialog(CWnd *pParentWnd, const WCHAR *TemplateName)
    : CDialog((UINT)0, pParentWnd)
{
    m_template_copy = NULL;
    SetDialogTemplate(TemplateName);
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CBaseDialog::~CBaseDialog()
{
    SetDialogTemplate(NULL);
}


//---------------------------------------------------------------------------
// SetDialogTemplate
//---------------------------------------------------------------------------


void CBaseDialog::SetDialogTemplate(const WCHAR *TemplateName)
{
    if (m_template_copy) {
        HeapFree(GetProcessHeap(), 0, m_template_copy);
        m_template_copy = NULL;
    }

    if (TemplateName && CMyApp::m_LayoutRTL) {

        m_template_copy =
            Common_DlgTmplRtl(AfxGetInstanceHandle(), TemplateName);

        if (m_template_copy)
            InitModalIndirect((LPCDLGTEMPLATE)m_template_copy, m_pParentWnd);
    }

    if (! m_template_copy)
        m_lpszTemplateName = TemplateName;
}


//---------------------------------------------------------------------------
// MakeLTR
//---------------------------------------------------------------------------


void CBaseDialog::MakeLTR(CWnd *pWnd)
{
    if (CMyApp::m_LayoutRTL) {
        const ULONG _flag = SWP_NOACTIVATE;
        pWnd->ModifyStyleEx(
            WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LAYOUTRTL, 0, _flag);
        pWnd->ModifyStyle(ES_RIGHT, 0, _flag);
    }
}


//---------------------------------------------------------------------------
// MakeLTR
//---------------------------------------------------------------------------


void CBaseDialog::MakeLTR(UINT id)
{
    MakeLTR(GetDlgItem(id));
}


//---------------------------------------------------------------------------
// AddMinimizeButton
//---------------------------------------------------------------------------


void CBaseDialog::AddMinimizeButton()
{
    ModifyStyle(0, WS_MINIMIZEBOX | WS_SYSMENU);
    ModifyStyleEx(0, WS_EX_APPWINDOW);
}


//---------------------------------------------------------------------------
// OnDestroy
//---------------------------------------------------------------------------


void CBaseDialog::OnDestroy()
{
    ShowOrHideAllWindows(false);
    CDialog::OnDestroy();
}


//---------------------------------------------------------------------------
// OnSize
//---------------------------------------------------------------------------


void CBaseDialog::OnSize(UINT nType, int cx, int cy)
{
    if (nType == SIZE_MINIMIZED)
        ShowOrHideAllWindows(true);
    else if (nType == SIZE_RESTORED)
        ShowOrHideAllWindows(false);
}


//---------------------------------------------------------------------------
// OnNcLButtonUp
//---------------------------------------------------------------------------


void CBaseDialog::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
    if (nHitTest == HTMINBUTTON)
        ShowWindow(SW_MINIMIZE);
    else
        CDialog::OnNcLButtonDown(nHitTest, point);
}


//---------------------------------------------------------------------------
// ShowOrHideAllWindows
//---------------------------------------------------------------------------


void CBaseDialog::ShowOrHideAllWindows(bool hide)
{
    if (hide) {

        if (! m_hidden_windows.IsEmpty())
            ShowOrHideAllWindows(false);

        EnumThreadWindows(
            GetCurrentThreadId(), ShowOrHideAllWindowsEnum, (LPARAM)this);

    } else {

        while (! m_hidden_windows.IsEmpty()) {
            HWND hwnd = (HWND)m_hidden_windows.RemoveHead();
            ::ShowWindow(hwnd, SW_SHOW);
        }
    }
}


//---------------------------------------------------------------------------
// ShowOrHideAllWindowsEnum
//---------------------------------------------------------------------------


BOOL CBaseDialog::ShowOrHideAllWindowsEnum(HWND hwnd, LPARAM lparam)
{
    CBaseDialog *pThis = (CBaseDialog *)lparam;
    bool HideWindow = false;

    if (hwnd != pThis->m_hWnd) {

        WCHAR clsnm[256];
        GetClassName(hwnd, clsnm, 250);
        clsnm[250] = L'\0';
        if (_wcsicmp(clsnm, L"#32770") == 0)
            HideWindow = true;
        else {
            CWnd *pMainWnd = AfxGetMainWnd();
            if (pMainWnd && pMainWnd->m_hWnd == hwnd)
                HideWindow = true;
        }
    }

    if (HideWindow && (GetWindowLong(hwnd, GWL_STYLE) & WS_VISIBLE)) {
        pThis->m_hidden_windows.AddTail(hwnd);
        ::ShowWindow(hwnd, SW_HIDE);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// GetInputWindow
//---------------------------------------------------------------------------


CWnd *CBaseDialog::GetInputWindow(bool *inModal)
{
    if (inModal)
        *inModal = false;

    CWnd *pWnd = AfxGetMainWnd();
    if (pWnd && pWnd->m_hWnd) {

        HWND hWnd = NULL;
        EnumThreadWindows(
            GetCurrentThreadId(), GetInputWindowEnum, (LPARAM)&hWnd);
        if (hWnd && hWnd != pWnd->m_hWnd) {
            pWnd = CWnd::FromHandle(hWnd);
            if (inModal)
                *inModal = true;
        }

    } else
        pWnd = NULL;

    return pWnd;
}


//---------------------------------------------------------------------------
// GetActiveWnd
//---------------------------------------------------------------------------


BOOL CBaseDialog::GetInputWindowEnum(HWND hwnd, LPARAM lparam)
{
    WCHAR clsnm[256];
    GetClassName(hwnd, clsnm, 250);
    clsnm[250] = L'\0';
    if (_wcsicmp(clsnm, L"#32770") == 0) {
        ULONG style = ::GetWindowLong(hwnd, GWL_STYLE);
        if ((style & (WS_VISIBLE | WS_DISABLED)) == WS_VISIBLE) {
            *(HWND *)lparam = hwnd;
            return FALSE;
        }
    }
    return TRUE;
}


//---------------------------------------------------------------------------
// InModalState
//---------------------------------------------------------------------------


bool CBaseDialog::InModalState()
{
    bool inModal;
    GetInputWindow(&inModal);
    return inModal;
}


//---------------------------------------------------------------------------
// FlashTitle
//---------------------------------------------------------------------------


void CBaseDialog::FlashTitle()
{
    FLASHWINFO flash;
    memzero(&flash, sizeof(FLASHWINFO));
    flash.cbSize = sizeof(FLASHWINFO);
    flash.hwnd = m_hWnd;
    flash.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
    ::FlashWindowEx(&flash);
}
