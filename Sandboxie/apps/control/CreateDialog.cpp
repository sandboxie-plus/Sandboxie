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
// Create Sandbox Dialog Box
//---------------------------------------------------------------------------


#include "MyApp.h"
#include "CreateDialog.h"

#include "Boxes.h"
#include "SbieIni.h"
#include "common/win32_ntddk.h"


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CCreateDialog, CBaseDialog)

    ON_CONTROL(EN_CHANGE, ID_CREATE_NAME, OnNameChange)

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CCreateDialog::CCreateDialog(CWnd *pParentWnd, const CString origName)
    : CBaseDialog(pParentWnd, L"CREATE_DIALOG")
{
    //
    // display dialog
    //

    if (origName.IsEmpty()) {
        m_title_id = MSG_3665;
        m_explain_id = MSG_3666;
    } else {
        m_name = origName;
        m_title_id = MSG_3476;
        m_explain_id = MSG_3528;
    }

    DoModal();
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CCreateDialog::~CCreateDialog()
{
}


//---------------------------------------------------------------------------
// OnInitDialog
//---------------------------------------------------------------------------


BOOL CCreateDialog::OnInitDialog()
{
    CString text = CMyMsg(m_title_id);
    text.Remove(L'&');
    SetWindowText(text);

    GetDlgItem(ID_CREATE_EXPLAIN)->SetWindowText(CMyMsg(m_explain_id));

    GetDlgItem(IDOK)->SetWindowText(CMyMsg(MSG_3001));
    GetDlgItem(IDCANCEL)->SetWindowText(CMyMsg(MSG_3002));

    if (m_name.IsEmpty()) {

        GetDlgItem(ID_CREATE_COPY_TEXT)->SetWindowText(CMyMsg(MSG_3669));

        MakeLTR(ID_CREATE_NAME);
        MakeLTR(ID_CREATE_COPY_COMBO);
        CComboBox *pCombo = (CComboBox *)GetDlgItem(ID_CREATE_COPY_COMBO);
        pCombo->AddString(CMyMsg(MSG_3769));

        CBoxes &boxes = CBoxes::GetInstance();
        for (int i = 1; i < boxes.GetSize(); ++i)
            pCombo->AddString(boxes.GetBox(i).GetName());
        pCombo->SetCurSel(0);

    } else {

        GetDlgItem(ID_CREATE_COPY_TEXT)->ShowWindow(SW_HIDE);
        GetDlgItem(ID_CREATE_COPY_COMBO)->ShowWindow(SW_HIDE);

    }

    return TRUE;
}


//---------------------------------------------------------------------------
// OnNameChange
//---------------------------------------------------------------------------


void CCreateDialog::OnNameChange()
{
    if (m_hide) {
        GetDlgItem(ID_CREATE_ERROR)->ShowWindow(SW_HIDE);
        m_hide = FALSE;
    }
}


//---------------------------------------------------------------------------
// OnOK
//---------------------------------------------------------------------------


void CCreateDialog::OnOK()
{
    CWnd *edit = GetDlgItem(ID_CREATE_NAME);
    CString name;
    edit->GetWindowText(name);

    //
    // make sure name is valid and unique
    //

    ULONG errmsg = 0;

    int i = 0;
    int len = name.GetLength();
    if (len <= 32) {
        for (; i < len; ++i) {
            if (name[i] >= L'0' && name[i] <= L'9')
                continue;
            if (name[i] >= L'A' && name[i] <= L'Z')
                continue;
            if (name[i] >= L'a' && name[i] <= L'z')
                continue;
            if (name[i] == L'_')
                continue;
            break;
        }
    }

    if (i == 0 || i != len)
        errmsg = MSG_3667;
    else {
        LONG rc = SbieApi_IsBoxEnabled(name);
        if (rc == STATUS_OBJECT_TYPE_MISMATCH)
            errmsg = MSG_3667;
        else if (rc == STATUS_ACCOUNT_RESTRICTION)
            errmsg = MSG_4665;
        else if (rc != STATUS_OBJECT_NAME_NOT_FOUND)
            errmsg = MSG_3668;
    }

    if (!errmsg && len <= 8) {
        if(SbieDll_IsReservedFileName(name))
            errmsg = MSG_3667;
    }

    if (!errmsg) {
        if (_wcsicmp(name, L"GlobalSettings") == 0
         || _wcsnicmp(name, L"UserSettings_", 13) == 0) {
            errmsg = MSG_3667;
        }
    }

    if (errmsg) {

        GetDlgItem(ID_CREATE_ERROR)->SetWindowText(CMyMsg(errmsg));
        GetDlgItem(ID_CREATE_ERROR)->ShowWindow(SW_SHOW);
        edit->SetFocus();
        m_hide = TRUE;
        return;
    }

    //
    // finish
    //

    m_name = name;

    CComboBox *pCombo = (CComboBox *)GetDlgItem(ID_CREATE_COPY_COMBO);
    i = pCombo->GetCurSel();
    if (i != 0 && i != CB_ERR)
        pCombo->GetLBText(i, m_oldname);

    EndDialog(0);
}


//---------------------------------------------------------------------------
// GetName
//---------------------------------------------------------------------------


CString CCreateDialog::GetName() const
{
    return m_name;
}


//---------------------------------------------------------------------------
// GetOldName
//---------------------------------------------------------------------------


CString CCreateDialog::GetOldName() const
{
    return m_oldname;
}
