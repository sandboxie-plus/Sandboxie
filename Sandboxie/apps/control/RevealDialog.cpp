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
// Reveal Sandbox Dialog Box
//---------------------------------------------------------------------------


#include "stdafx.h"
#include "RevealDialog.h"
#include "UserSettings.h"

#include "Boxes.h"


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CRevealDialog, CBaseDialog)

    ON_CONTROL(CBN_SELENDOK, ID_REVEAL_COMBO, OnCombo)
    ON_CONTROL(CBN_CLOSEUP,  ID_REVEAL_COMBO, OnCombo)

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CRevealDialog::CRevealDialog(CWnd *pParentWnd)
    : CBaseDialog(pParentWnd, L"REVEAL_DIALOG")
{
    m_modified = false;

    //
    // display dialog
    //

    DoModal();
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CRevealDialog::~CRevealDialog()
{
}


//---------------------------------------------------------------------------
// OnInitDialog
//---------------------------------------------------------------------------


BOOL CRevealDialog::OnInitDialog()
{
    SetWindowText(CMyMsg(MSG_3434));

    GetDlgItem(ID_REVEAL_EXPLAIN_1)->SetWindowText(CMyMsg(MSG_5122));
    GetDlgItem(ID_REVEAL_EXPLAIN_2)->SetWindowText(CMyMsg(MSG_5123));
    GetDlgItem(ID_REVEAL_EXPLAIN_3)->SetWindowText(
        CMyMsg(MSG_5124, CUserSettings::GetInstance().GetUserName()));

    GetDlgItem(IDOK)->SetWindowText(CMyMsg(MSG_3001));
    GetDlgItem(IDCANCEL)->SetWindowText(CMyMsg(MSG_3004));

    CStringList boxes;
    CBoxes::GetInstance().GetHiddenBoxes(boxes);
    if (boxes.IsEmpty()) {
        EndDialog(0);
        return FALSE;
    }

    MakeLTR(ID_REVEAL_EDIT);
    MakeLTR(ID_REVEAL_COMBO);
    CComboBox *pCombo = (CComboBox *)GetDlgItem(ID_REVEAL_COMBO);

    while (! boxes.IsEmpty())
        pCombo->AddString(boxes.RemoveHead());
    pCombo->SetCurSel(0);
    OnCombo();

    return TRUE;
}


//---------------------------------------------------------------------------
// OnCombo
//---------------------------------------------------------------------------


void CRevealDialog::OnCombo()
{
    CComboBox *pCombo = (CComboBox *)GetDlgItem(ID_REVEAL_COMBO);

    int index = pCombo->GetCurSel();
    if (index == LB_ERR)
        return;

    CString boxname;
    pCombo->GetLBText(index, boxname);

    CStringList users;
    CBox::GetSetUserAccounts(boxname, FALSE, users);

    CString text;
    while (! users.IsEmpty()) {
        if (text.GetLength())
            text += L", ";
        text += users.RemoveHead();
    }

    CEdit *pEdit = (CEdit *)GetDlgItem(ID_REVEAL_EDIT);
    pEdit->SetWindowText(text);
}


//---------------------------------------------------------------------------
// OnOK
//---------------------------------------------------------------------------


void CRevealDialog::OnOK()
{
    CComboBox *pCombo = (CComboBox *)GetDlgItem(ID_REVEAL_COMBO);

    int index = pCombo->GetCurSel();
    if (index == LB_ERR)
        return;

    CString boxname;
    pCombo->GetLBText(index, boxname);

    CStringList users;
    CBox::GetSetUserAccounts(boxname, FALSE, users);
    users.AddTail(CUserSettings::GetInstance().GetUserName());
    CBox::GetSetUserAccounts(boxname, TRUE, users);

    m_modified = true;
    EndDialog(0);
}


//---------------------------------------------------------------------------
// IsModified
//---------------------------------------------------------------------------


bool CRevealDialog::IsModified() const
{
    return m_modified;
}
