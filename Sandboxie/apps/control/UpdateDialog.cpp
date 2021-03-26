/*
 * Copyright 2020 David Xanatos, xanasoft.com
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

#include "stdafx.h"

#include "UpdateDialog.h"
#include "Updater.h"
#include "common/CommonUtils.h"
#include "common/MyMsg.h"
#include "core/dll/sbiedll.h"
#include "UserSettings.h"

//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CUpdateDialog, CDialog)

    ON_COMMAND(IDOK,			OnNow)
	ON_COMMAND(ID_UPDATE_LATER,	OnLater)
	ON_COMMAND(ID_UPDATE_NEVER,	OnNever)
    ON_COMMAND(IDCANCEL,		OnCancel)

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CUpdateDialog::CUpdateDialog(CWnd *pParentWnd)
    : CDialog((UINT)0, pParentWnd)
{
    m_lpszTemplateName = L"UPDATE_DIALOG";

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
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CUpdateDialog::~CUpdateDialog()
{
    if (m_DlgTmplCopy) {
        HeapFree(GetProcessHeap(), 0, m_DlgTmplCopy);
        m_DlgTmplCopy = NULL;
    }
}


//---------------------------------------------------------------------------
// OnInitDialog
//---------------------------------------------------------------------------


BOOL CUpdateDialog::OnInitDialog()
{
    SetWindowText(CMyMsg(MSG_3621));

    GetDlgItem(ID_UPDATE_EXPLAIN_1)->SetWindowText(CMyMsg(MSG_3622));
	GetDlgItem(IDOK)->SetWindowText(CMyMsg(MSG_3623));
    GetDlgItem(ID_UPDATE_LATER)->SetWindowText(CMyMsg(MSG_3624));
	GetDlgItem(ID_UPDATE_NEVER)->SetWindowText(CMyMsg(MSG_3625));
    GetDlgItem(IDCANCEL)->SetWindowText(CMyMsg(MSG_3002));
	GetDlgItem(ID_UPDATE_SILENT)->SetWindowText(CMyMsg(MSG_3626));
	GetDlgItem(ID_UPDATE_EXPLAIN_2)->SetWindowText(CMyMsg(MSG_3627));

	BOOL UpdateCheckNotify;
	CUserSettings::GetInstance().GetBool(_UpdateCheckNotify, UpdateCheckNotify, FALSE);
	CButton *pCheckBox1 = (CButton *)GetDlgItem(ID_UPDATE_SILENT);
	pCheckBox1->SetCheck(!UpdateCheckNotify ? BST_CHECKED : BST_UNCHECKED);

    return TRUE;
}


//---------------------------------------------------------------------------
// OnNow
//---------------------------------------------------------------------------


void CUpdateDialog::OnNow()
{
	if(!CUpdater::GetInstance().CheckUpdates(GetParent()))
		CMyApp::MsgBox(NULL, MSG_3628, MB_OK);
	CloseDialog();
}


//---------------------------------------------------------------------------
// OnLater
//---------------------------------------------------------------------------


void CUpdateDialog::OnLater()
{
	CUserSettings::GetInstance().SetNum64(_NextUpdateCheck, time(NULL) + 7 * 24 * 60 * 60);
	CloseDialog();
}


//---------------------------------------------------------------------------
// OnNever
//---------------------------------------------------------------------------


void CUpdateDialog::OnNever()
{
	CUserSettings::GetInstance().SetNum64(_NextUpdateCheck, -1);
	CloseDialog();
}


//---------------------------------------------------------------------------
// OnCancel
//---------------------------------------------------------------------------


void CUpdateDialog::OnCancel()
{
	EndDialog(0);
}


//---------------------------------------------------------------------------
// EndDialog
//---------------------------------------------------------------------------


void CUpdateDialog::CloseDialog()
{
	CButton *pCheckBox1 = (CButton *)GetDlgItem(ID_UPDATE_SILENT);
	CUserSettings::GetInstance().SetBool(_UpdateCheckNotify, !pCheckBox1->GetCheck() == BST_CHECKED);

	EndDialog(1);
}