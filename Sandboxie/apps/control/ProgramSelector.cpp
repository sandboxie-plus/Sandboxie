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
// Program Selector Dialog Box
//---------------------------------------------------------------------------


#include "stdafx.h"
#include "MyApp.h"
#include "ProgramSelector.h"


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CProgramSelector, CBaseDialog)

    ON_CONTROL(LBN_SELCHANGE, ID_PGMSEL_LIST1,  OnListSelected)
    ON_CONTROL(LBN_SELCHANGE, ID_PGMSEL_LIST2,  OnListSelected)
    ON_CONTROL(LBN_SELCHANGE, ID_PGMSEL_LIST3,  OnListSelected)

    ON_CONTROL(EN_CHANGE,     ID_PGMSEL_INPUT,  OnEditChanged)

    ON_COMMAND(ID_PGMSEL_OPEN,                  OnOpenFile)

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CProgramSelector::CProgramSelector(
    CWnd *pParentWnd, CBox *box,
    bool withProgramGroups, const CString &groupName)
    : CBaseDialog(pParentWnd, L"PROGRAM_SELECTOR")
{
    CBoxProc::AddRecentExesToList(m_list1);

    if (box) {

        box->GetProcessGroupsInBox(m_list3);

        GetProgramNamesFromSettings(*box);

        if (withProgramGroups) {
            while (1) {
                POSITION pos = m_list3.Find(groupName);
                if (! pos)
                    break;
                m_list3.RemoveAt(pos);
            }
        } else
            m_list3.RemoveAll();
    }

    //
    // display dialog
    //

    DoModal();
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CProgramSelector::~CProgramSelector()
{
}


//---------------------------------------------------------------------------
// GetProgramNamesFromSettings
//---------------------------------------------------------------------------


void CProgramSelector::GetProgramNamesFromSettings(CBox &box)
{
    CStringList &pgms = m_list2;

    //
    // add various settings that name processes
    //

    for (int retry = 0; retry < 5; ++retry) {

        CStringList procs;

        if (retry == 4) {

            GetProgramNamesFromGroups(box, m_list3, procs);

        } else if (retry == 3) {

            GetProgramNamesFromPathLists(box, procs);

        } else {

            WCHAR type = 0;
            if (retry == 0)
                type = L'A';
            else if (retry == 1)
                type = L'F';
            else if (retry == 2)
                type = L'L';
            box.GetProcessList(type, procs);
        }

        POSITION pos1 = procs.GetHeadPosition();
        while (pos1) {

            CString pgm1 = procs.GetNext(pos1);
            if (pgm1.GetLength() == 0)
                continue;
            if (pgm1 == L'*')
                continue;
            if (pgm1.GetAt(0) == L'!')
                pgm1.Delete(0, 1);
            if (pgm1.GetAt(0) == L'<')
                continue;

            BOOL add = TRUE;

            POSITION pos2 = pgms.GetHeadPosition();
            while (pos2) {
                CString &pgm2 = pgms.GetNext(pos2);
                if (pgm2.CompareNoCase(pgm1) == 0) {
                    add = FALSE;
                    break;
                }
            }

            if (add) {
                pgm1.MakeLower();
                pgms.AddTail(pgm1);
            }
        }
    }
}


//---------------------------------------------------------------------------
// GetProgramNamesFromGroups
//---------------------------------------------------------------------------


void CProgramSelector::GetProgramNamesFromGroups(
    CBox &box, const CStringList &groups, CStringList &out_pgms)
{
    POSITION pos = groups.GetHeadPosition();
    while (pos) {
        CString group = groups.GetNext(pos);
        CString pgms = box.GetProcessGroup(group);
        int i = 0;
        while (i < pgms.GetLength()) {
            int j = pgms.Find(L',', i);
            if (j == -1)
                j = pgms.GetLength();
            CString token = pgms.Mid(i, j - i);
            token.TrimLeft();
            token.TrimRight();
            out_pgms.AddTail(token);
            i = j + 1;
        }
    }
}


//---------------------------------------------------------------------------
// GetProgramNamesFromPathLists
//---------------------------------------------------------------------------


void CProgramSelector::GetProgramNamesFromPathLists(
    CBox &box, CStringList &pgms)
{
    CStringList list;

    CStringList tmp;
    box.GetXxxPathList('fo', tmp, TRUE);
    list.AddTail(&tmp);

    tmp.RemoveAll();
    box.GetXxxPathList('po', tmp, TRUE);
    list.AddTail(&tmp);

    tmp.RemoveAll();
    box.GetXxxPathList('fc', tmp, TRUE);
    list.AddTail(&tmp);

    tmp.RemoveAll();
    box.GetXxxPathList('fr', tmp, TRUE);
    list.AddTail(&tmp);

    tmp.RemoveAll();
    box.GetXxxPathList('ko', tmp, TRUE);
    list.AddTail(&tmp);

    tmp.RemoveAll();
    box.GetXxxPathList('kc', tmp, TRUE);
    list.AddTail(&tmp);

    tmp.RemoveAll();
    box.GetXxxPathList('kr', tmp, TRUE);
    list.AddTail(&tmp);

    tmp.RemoveAll();
    box.GetXxxPathList('io', tmp, TRUE);
    list.AddTail(&tmp);

    tmp.RemoveAll();
    box.GetXxxPathList('ic', tmp, TRUE);
    list.AddTail(&tmp);

    tmp.RemoveAll();
    box.GetXxxPathList('wo', tmp, TRUE);
    list.AddTail(&tmp);

    tmp.RemoveAll();
    box.GetXxxPathList('co', tmp, TRUE);
    list.AddTail(&tmp);

    while (! list.IsEmpty()) {
        CString str = list.RemoveHead();
        int index = str.Find(L',');
        if (index == -1)
            continue;
        str = str.Left(index);
        if (str.GetLength() > 1 && str.GetAt(0) == L'[')
            str.Delete(0, 1);
        if (str.GetLength() > 1 && str.GetAt(0) == L'<')
            continue;
        else if (str.Left(2) == L"!<")
            continue;
        str.TrimLeft();
        str.TrimRight();
        if (str.IsEmpty())
            continue;
        pgms.AddTail(str);
    }
}


//---------------------------------------------------------------------------
// PrepareListBox
//---------------------------------------------------------------------------


void CProgramSelector::PrepareListBox(int idCtrl, CStringList &list)
{
    MakeLTR(idCtrl);
    CListBox *pListBox = (CListBox *)GetDlgItem(idCtrl);
    pListBox->SetFont(&CMyApp::m_fontFixed);
    while (! list.IsEmpty()) {
        CString entry = list.RemoveHead();
        pListBox->AddString(entry);
    }
}


//---------------------------------------------------------------------------
// OnInitDialog
//---------------------------------------------------------------------------


BOOL CProgramSelector::OnInitDialog()
{
    ULONG titleMsgId = MSG_4282;
    if (m_list3.IsEmpty())
        titleMsgId = MSG_4281;
    SetWindowText(CMyMsg(titleMsgId));

    GetDlgItem(ID_PGMSEL_TEXT1)->SetWindowText(CMyMsg(MSG_4283));
    GetDlgItem(ID_PGMSEL_TEXT2)->SetWindowText(CMyMsg(MSG_4284));
    GetDlgItem(ID_PGMSEL_TEXT3)->SetWindowText(CMyMsg(MSG_4285));

    GetDlgItem(ID_PGMSEL_TEXT4)->SetWindowText(CMyMsg(MSG_4286));
    GetDlgItem(ID_PGMSEL_INPUT)->SetFont(&CMyApp::m_fontFixed);
    GetDlgItem(ID_PGMSEL_OPEN)->SetWindowText(CMyMsg(MSG_4287));

    GetDlgItem(IDOK)->SetWindowText(CMyMsg(MSG_3001));
    GetDlgItem(IDCANCEL)->SetWindowText(CMyMsg(MSG_3002));

    BOOL emptyProgramGroups = m_list3.IsEmpty();

    PrepareListBox(ID_PGMSEL_LIST1, m_list1);
    PrepareListBox(ID_PGMSEL_LIST2, m_list2);
    PrepareListBox(ID_PGMSEL_LIST3, m_list3);

    if (emptyProgramGroups) {

        GetDlgItem(ID_PGMSEL_TEXT3)->ShowWindow(SW_HIDE);
        GetDlgItem(ID_PGMSEL_LIST3)->ShowWindow(SW_HIDE);
    }

    GetDlgItem(ID_PGMSEL_INPUT)->SetFocus();
    return FALSE;
}


//---------------------------------------------------------------------------
// OnListSelected
//---------------------------------------------------------------------------


void CProgramSelector::OnListSelected()
{
    CString pgm;

    CListBox *pListBox1 = (CListBox *)GetDlgItem(ID_PGMSEL_LIST1);
    CListBox *pListBox2 = (CListBox *)GetDlgItem(ID_PGMSEL_LIST2);
    CListBox *pListBox3 = (CListBox *)GetDlgItem(ID_PGMSEL_LIST3);

    ULONG idCtrl = LOWORD(GetCurrentMessage()->wParam);
    if (idCtrl == ID_PGMSEL_LIST1) {

        int index = pListBox1->GetCurSel();
        if (index != LB_ERR) {
            pListBox1->GetText(index, pgm);
            pListBox2->SetCurSel(-1);
            pListBox3->SetCurSel(-1);
        }

    } else if (idCtrl == ID_PGMSEL_LIST2) {

        int index = pListBox2->GetCurSel();
        if (index != LB_ERR) {
            pListBox2->GetText(index, pgm);
            pListBox1->SetCurSel(-1);
            pListBox3->SetCurSel(-1);
        }

    } else if (idCtrl == ID_PGMSEL_LIST3) {

        int index = pListBox3->GetCurSel();
        if (index != LB_ERR) {
            pListBox3->GetText(index, pgm);
            pListBox1->SetCurSel(-1);
            pListBox2->SetCurSel(-1);
        }
    }

    GetDlgItem(ID_PGMSEL_INPUT)->SetWindowText(pgm);
    m_pgm1 = pgm;
    m_pgm2 = CString();
}


//---------------------------------------------------------------------------
// OnEditChanged
//---------------------------------------------------------------------------


void CProgramSelector::OnEditChanged()
{
    GetDlgItem(ID_PGMSEL_INPUT)->GetWindowText(m_pgm1);
    m_pgm2 = CString();
}


//---------------------------------------------------------------------------
// OnOK
//---------------------------------------------------------------------------


void CProgramSelector::OnOpenFile()
{
    CString pgm1;
    CString pgm2;
    CMyApp::GetProgramFileName(this, pgm1, pgm2);
    if (! pgm1.IsEmpty()) {

        GetDlgItem(ID_PGMSEL_INPUT)->SetWindowText(pgm1);
        m_pgm1 = pgm1;
        m_pgm2 = pgm2;
    }
}


//---------------------------------------------------------------------------
// OnOK
//---------------------------------------------------------------------------


void CProgramSelector::OnOK()
{
    m_pgm1.TrimLeft();
    m_pgm1.TrimRight();
    m_pgm1.MakeLower();

    m_pgm2.TrimLeft();
    m_pgm2.TrimRight();
    m_pgm2.MakeLower();

    if (! m_pgm1.IsEmpty()) {

        m_pgm1.Remove(L',');
        m_pgm1.Remove(L'[');
        m_pgm1.Remove(L']');

        CListBox *pListBox = (CListBox *)GetDlgItem(ID_PGMSEL_LIST3);
        bool found = false;
        int num = pListBox->GetCount();
        for (int index = 0; index < num; ++index) {
            CString str;
            pListBox->GetText(index, str);
            if (str.CompareNoCase(m_pgm1) == 0) {
                m_pgm1 = str;
                found = true;
                break;
            }
        }

        if (! found) {

            bool isProgramGroup =
                ((m_pgm1.GetLength() > 1 && m_pgm1.GetAt(0) == L'<')) ||
                ((m_pgm1.GetLength() > 2 && m_pgm1.Left(2) == L"!<"));

            if (! isProgramGroup) {

                int index = m_pgm1.Find(L'.');
                if (index == -1)
                    m_pgm1 += L".exe";
            }
        }
    }

    EndDialog(0);
}


//---------------------------------------------------------------------------
// OnCancel
//---------------------------------------------------------------------------


void CProgramSelector::OnCancel()
{
    m_pgm1 = CString();
    m_pgm2 = CString();

    CDialog::OnCancel();
}
