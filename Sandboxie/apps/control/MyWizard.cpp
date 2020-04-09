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
// My Generic Wizard
//---------------------------------------------------------------------------


#include "stdafx.h"
#include "MyApp.h"
#include "MyWizard.h"
#include "apps/common/FontStore.h"


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CMyWizard, CPropertySheet)

    ON_WM_SIZE()
    ON_WM_NCLBUTTONDOWN()

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// CMyWizard Constructor
//---------------------------------------------------------------------------


CMyWizard::CMyWizard(CWnd *pParentWnd, ULONG title_msgid)
{
    m_pBaseDialog = new CBaseDialog(NULL);

    CommonConstruct(pParentWnd, 0);
    m_psh.dwFlags |= PSH_WIZARD | PSH_WIZARD_LITE;

    m_title_msgid = title_msgid;
}


//---------------------------------------------------------------------------
// CMyWizard Destructor
//---------------------------------------------------------------------------


CMyWizard::~CMyWizard()
{
    while (! m_pages.IsEmpty()) {
        CMyWizardPage *page = (CMyWizardPage *)m_pages.RemoveHead();
        delete page;
    }

    delete m_pBaseDialog;
}


//---------------------------------------------------------------------------
// CMyWizard::AddPage
//---------------------------------------------------------------------------


void CMyWizard::AddPage(CMyWizardPage *page)
{
    m_pages.AddTail(page);
    CPropertySheet::AddPage(page);
}


//---------------------------------------------------------------------------
// CMyWizard::OnInitDialog
//---------------------------------------------------------------------------


BOOL CMyWizard::OnInitDialog()
{
    m_pBaseDialog->m_hWnd = m_hWnd;
    m_pBaseDialog->AddMinimizeButton();

    if (CMyApp::m_LayoutRTL)
        ModifyStyleEx(0, WS_EX_LAYOUTRTL, SWP_NOACTIVATE);

    CenterWindow(GetDesktopWindow());

    SetWindowText(CMyMsg(m_title_msgid));

    SetDlgItemText(IDCANCEL, CMyMsg(MSG_3004));

    return CPropertySheet::OnInitDialog();
}


//---------------------------------------------------------------------------
// CMyWizard::OnSize
//---------------------------------------------------------------------------


void CMyWizard::OnSize(UINT nType, int cx, int cy)
{
    if (m_pBaseDialog)
        m_pBaseDialog->OnSize(nType, cx, cy);
}


//---------------------------------------------------------------------------
// CMyWizard::OnNcLButtonDown
//---------------------------------------------------------------------------


void CMyWizard::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
    if (m_pBaseDialog)
        m_pBaseDialog->OnNcLButtonDown(nHitTest, point);
}


//---------------------------------------------------------------------------
// CMyWizardPage Constructor
//---------------------------------------------------------------------------


CMyWizardPage::CMyWizardPage(int page_num)
    : CLayoutPropertyPage(page_num)
{
    CLayoutPropertyPage::InitPage(
            0,
            CSize(50, 50), CSize(640, 384), CSize(0, 0),
            CMyApp::m_background);
}


//---------------------------------------------------------------------------
// CMyWizardPage::SetPageTitle
//---------------------------------------------------------------------------


void CMyWizardPage::SetPageTitle(const CString &text)
{
    CStatic *title = CreateStatic(text, CPoint(5, 5), CSize(80, 10));
    title->SetFont(CFontStore::Get(L"Tahoma", 12, FW_BOLD));

    CString num;
    CMyWizard *wizard = (CMyWizard *)GetParent();
    num.Format(L"%d/%d", m_page_num, wizard->GetPageCount());

    CStatic *page = CreateStatic(num, CPoint(90, 5), CSize(5, 10));
    page->SetFont(CFontStore::Get(L"Tahoma", 10));
}


//---------------------------------------------------------------------------
// CMyWizardPage::SetPageTitle
//---------------------------------------------------------------------------


void CMyWizardPage::SetPageTitle(ULONG msgid)
{
    return SetPageTitle(CMyMsg(msgid));
}


//---------------------------------------------------------------------------
// CMyWizardPage::CreateStatic
//---------------------------------------------------------------------------


CStatic *CMyWizardPage::CreateStatic(
    const CString &text, const CPoint &pos, const CPoint &size)
{
    CStatic *wnd = (CStatic *)CreateChild(-1, L"STATIC", 0, 0, pos, size);
    wnd->SetWindowText(text);
    wnd->SetFont(CFontStore::Get(L"Tahoma", 12));
    return wnd;
}


//---------------------------------------------------------------------------
// CMyWizardPage::CreateStatic
//---------------------------------------------------------------------------


CStatic *CMyWizardPage::CreateStatic(
    ULONG msgid, const CPoint &pos, const CPoint &size)
{
    return CreateStatic(CMyMsg(msgid), pos, size);
}
