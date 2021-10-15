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
// Alert Programs Dialog Box
//---------------------------------------------------------------------------

#include "stdafx.h"
#include "MyApp.h"
#include "AlertDialog.h"

#include "Boxes.h"
#include "SbieIni.h"
#include "ProgramSelector.h"


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CAlertDialog, CBaseDialog)

    ON_COMMAND(ID_ALERT_ADD,        OnAdd)
    ON_COMMAND(ID_ALERT_REMOVE,     OnRemove)

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CAlertDialog::CAlertDialog(CWnd *pParentWnd)
    : CBaseDialog(pParentWnd, L"ALERT_DIALOG")
{
    DoModal();
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CAlertDialog::~CAlertDialog()
{
}


//---------------------------------------------------------------------------
// OnInitDialog
//---------------------------------------------------------------------------


BOOL CAlertDialog::OnInitDialog()
{
    SetWindowText(CMyMsg(MSG_3681));

    GetDlgItem(ID_ALERT_EXPLAIN)->SetWindowText(CMyMsg(MSG_3682));

    GetDlgItem(ID_ALERT_ADD)->SetWindowText(CMyMsg(MSG_3355));
    GetDlgItem(ID_ALERT_REMOVE)->SetWindowText(CMyMsg(MSG_3351));

    GetDlgItem(IDOK)->SetWindowText(CMyMsg(MSG_3001));
    GetDlgItem(IDCANCEL)->SetWindowText(CMyMsg(MSG_3002));

    MakeLTR(ID_ALERT_LIST);
    CListBox *listbox = (CListBox *)GetDlgItem(ID_ALERT_LIST);
    listbox->SetFont(&CMyApp::m_fontFixed);

    CStringList list;
    CBoxes::GetInstance().GetBox(0).GetProcessList(L'A', list);
    while (! list.IsEmpty()) {
        CString pgm = list.RemoveHead();
        listbox->AddString(pgm);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// OnAddName
//---------------------------------------------------------------------------


void CAlertDialog::OnAdd()
{
    CProgramSelector pgmsel(this, NULL, false);
    if (! pgmsel.m_pgm1.IsEmpty()) {
        CListBox *pListBox = (CListBox *)GetDlgItem(ID_ALERT_LIST);
        pListBox->AddString(pgmsel.m_pgm1);
        if (! pgmsel.m_pgm2.IsEmpty())
            pListBox->AddString(pgmsel.m_pgm2);
    }
}


//---------------------------------------------------------------------------
// OnRemove
//---------------------------------------------------------------------------


void CAlertDialog::OnRemove()
{
    CListBox *listbox = (CListBox *)GetDlgItem(ID_ALERT_LIST);
    int index = listbox->GetCurSel();
    if (index == LB_ERR)
        return;
    listbox->DeleteString(index);
}


//---------------------------------------------------------------------------
// OnOK
//---------------------------------------------------------------------------


void CAlertDialog::OnOK()
{
    CBox &box = CBoxes::GetInstance().GetBox(0);

    BOOL ok = TRUE;

    CStringList list;
    box.GetProcessList(L'A', list);

    CListBox *listbox = (CListBox *)GetDlgItem(ID_ALERT_LIST);
    int size = listbox->GetCount();

    //
    // remove alerts that are no longer in our list
    //

    POSITION pos = list.GetHeadPosition();
    while (pos && ok) {
        CString pgm1 = list.GetNext(pos);

        BOOL found = FALSE;
        for (int index = 0; index < size; ++index) {
            CString pgm2;
            listbox->GetText(index, pgm2);
            if (pgm1 == pgm2) {
                found = TRUE;
                break;
            }
        }

        if (! found)
            ok = box.SetProcessSetting(L'A', pgm1, FALSE);
    }

    //
    // add new alerts
    //

    for (int index = 0; (index < size) && ok; ++index) {
        CString pgm1;
        listbox->GetText(index, pgm1);

        BOOL found = FALSE;
        pos = list.GetHeadPosition();
        while (pos) {
            CString pgm2 = list.GetNext(pos);
            if (pgm1 == pgm2) {
                found = TRUE;
                break;
            }
        }

        if (! found)
            ok = box.SetProcessSetting(L'A', pgm1, TRUE);
    }

    EndDialog(0);
}
