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
// Temporarily Disable Forced Programs Dialog Box
//---------------------------------------------------------------------------

#include "stdafx.h"
#include "MyApp.h"
#include "DisableForceDialog.h"

#include "Boxes.h"
#include "SbieIni.h"


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static const CString _ForceDisableSeconds(L"ForceDisableSeconds");


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CDisableForceDialog::CDisableForceDialog(CWnd *pParentWnd)
    : CBaseDialog(pParentWnd, L"DISABLE_FORCE_DIALOG")
{
    m_endedok = FALSE;

    DoModal();
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CDisableForceDialog::~CDisableForceDialog()
{
}


//---------------------------------------------------------------------------
// OnInitDialog
//---------------------------------------------------------------------------


BOOL CDisableForceDialog::OnInitDialog()
{
    SetWindowText(CMyMsg(MSG_3651));

    GetDlgItem(ID_DISABLE_FORCE_LABEL_1)->SetWindowText(CMyMsg(MSG_3652));
    GetDlgItem(ID_DISABLE_FORCE_LABEL_2)->SetWindowText(CMyMsg(MSG_3653));
    GetDlgItem(ID_DISABLE_FORCE_LABEL_3)->SetWindowText(CMyMsg(MSG_3654));
    GetDlgItem(ID_DISABLE_FORCE_LABEL_4)->SetWindowText(CMyMsg(MSG_3599));

    GetDlgItem(IDOK)->SetWindowText(CMyMsg(MSG_3001));
    GetDlgItem(IDCANCEL)->SetWindowText(CMyMsg(MSG_3002));

    int seconds;
    CSbieIni::GetInstance().GetNum(
        _GlobalSettings, _ForceDisableSeconds, seconds, 10);
    SetDlgItemInt(ID_DISABLE_FORCE_EDIT, seconds);

    CSpinButtonCtrl *spin =
        (CSpinButtonCtrl *)GetDlgItem(ID_DISABLE_FORCE_SPIN);
    spin->SetRange(0, 29999);

    return TRUE;
}


//---------------------------------------------------------------------------
// OnOK
//---------------------------------------------------------------------------


void CDisableForceDialog::OnOK()
{
    int seconds = GetDlgItemInt(ID_DISABLE_FORCE_EDIT);
    BOOL ok = CSbieIni::GetInstance().SetNum(
        _GlobalSettings, _ForceDisableSeconds, seconds);
    if (! ok) {
        CSbieIni::GetInstance().GetNum(
            _GlobalSettings, _ForceDisableSeconds, seconds, 10);
        SetDlgItemInt(ID_DISABLE_FORCE_EDIT, seconds);
        ((CEdit *)GetDlgItem(ID_DISABLE_FORCE_EDIT))->SetSel(0, -1);
        GetDlgItem(ID_DISABLE_FORCE_EDIT)->SetFocus();

        return;
    }

    m_endedok = TRUE;
    EndDialog(0);
}


//---------------------------------------------------------------------------
// EndedOk
//---------------------------------------------------------------------------


BOOL CDisableForceDialog::EndedOk() const
{
    return m_endedok;
}
