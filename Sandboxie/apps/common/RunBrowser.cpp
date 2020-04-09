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
// Run Browser Dialog
//---------------------------------------------------------------------------

#include "..\control\stdafx.h"

#include "RunBrowser.h"
#include "CommonUtils.h"
#include "MyMsg.h"
#include "core/dll/sbiedll.h"


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CRunBrowser, CDialog)

    ON_COMMAND(IDYES,   OnYes)
    ON_COMMAND(IDNO,    OnNo)

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CRunBrowser::CRunBrowser(CWnd *pParentWnd, const CString &url)
    : CDialog((UINT)0, pParentWnd)
{
    m_url = url;
    m_lpszTemplateName = L"RUN_BROWSER_DIALOG";

    BOOLEAN LayoutRTL;
    SbieDll_GetLanguage(&LayoutRTL);
    if (LayoutRTL) {

        m_DlgTmplCopy =
            Common_DlgTmplRtl(AfxGetInstanceHandle(), m_lpszTemplateName);
        if (m_DlgTmplCopy) {
            m_lpszTemplateName = NULL;
            InitModalIndirect((LPCDLGTEMPLATE)m_DlgTmplCopy, m_pParentWnd);
        }

    } else
        m_DlgTmplCopy = NULL;

    DoModal();
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CRunBrowser::~CRunBrowser()
{
    if (m_DlgTmplCopy) {
        HeapFree(GetProcessHeap(), 0, m_DlgTmplCopy);
        m_DlgTmplCopy = NULL;
    }
}


//---------------------------------------------------------------------------
// OnInitDialog
//---------------------------------------------------------------------------


BOOL CRunBrowser::OnInitDialog()
{
    SetWindowText(CMyMsg(MSG_3641));

    GetDlgItem(ID_RUN_BROWSER_EXPLAIN)->SetWindowText(CMyMsg(MSG_3642));
    GetDlgItem(IDYES)->SetWindowText(CMyMsg(MSG_3643));
    GetDlgItem(IDCANCEL)->SetWindowText(CMyMsg(MSG_3002));
    GetDlgItem(IDNO)->SetWindowText(CMyMsg(MSG_3644));

    GetDlgItem(ID_RUN_BROWSER_URL)->SetWindowText(m_url);
    return TRUE;
}


//---------------------------------------------------------------------------
// OnYes
//---------------------------------------------------------------------------


void CRunBrowser::OnYes()
{
    Common_RunStartExe(m_url, CString());
    EndDialog(0);
}


//---------------------------------------------------------------------------
// OnNo
//---------------------------------------------------------------------------


void CRunBrowser::OnNo()
{
    ShellExecute(
        m_pParentWnd->m_hWnd, NULL, m_url, NULL, NULL, SW_SHOWNORMAL);
    EndDialog(0);
}


//---------------------------------------------------------------------------
// GetTopicUrl
//---------------------------------------------------------------------------


CString CRunBrowser::GetTopicUrl(const CString &topic)
{
    return L"https://www.sandboxie.com/index.php?" + topic;
}


//---------------------------------------------------------------------------
// OpenHelp
//---------------------------------------------------------------------------


void CRunBrowser::OpenHelp(CWnd *pParentWnd, const CString &topic)
{
    CRunBrowser x(pParentWnd, GetTopicUrl(topic));
}

//---------------------------------------------------------------------------
// OpenForum
//---------------------------------------------------------------------------


void CRunBrowser::OpenForum(CWnd *pParentWnd)
{
    CRunBrowser x(pParentWnd, L"http://forums.sandboxie.com/phpBB3/");
}
