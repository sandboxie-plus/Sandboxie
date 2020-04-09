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
// Process Settings Dialog Box
//---------------------------------------------------------------------------


#include "stdafx.h"
#include "MyApp.h"
#include "ProcSettingsDialog.h"

#include "Boxes.h"
#include "SbieIni.h"
#include "UserSettings.h"


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CProcSettingsDialog, CBaseDialog)

    ON_COMMAND(ID_SETTINGS_PAGE_1,          OnViewPage1)
    ON_COMMAND(ID_SETTINGS_PAGE_2,          OnViewPage2)

    ON_COMMAND(ID_SETTINGS_ALERT,           OnCmdAlert)
    ON_COMMAND(ID_SETTINGS_FORCE,           OnCmdForce)

    ON_COMMAND(ID_SETTINGS_LINGER,          OnCmdLinger)
    ON_COMMAND(ID_SETTINGS_LEADER,          OnCmdLeader)

    ON_COMMAND(ID_SETTINGS_INTERNET,        OnCmdInternetAccess)
    ON_COMMAND(ID_SETTINGS_STARTRUN,        OnCmdStartRunAccess)

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


extern const WCHAR *_ProcSettingsNotify;


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CProcSettingsDialog::CProcSettingsDialog(CWnd *pParentWnd, ULONG pid)
    : CBaseDialog(pParentWnd, L"PROC_SETTINGS_DIALOG")
{
    CBox &box = CBoxes::GetInstance().GetBoxByProcessId(pid);
    m_boxname = box.GetName();

    CBoxProc &boxproc = box.GetBoxProc();
    m_process =
        boxproc.GetProcessImageName(boxproc.GetIndexForProcessId(pid));

    m_alert  = box.GetProcessSetting(L'A', m_process);
    m_force  = box.GetProcessSetting(L'F', m_process);
    m_linger = box.GetProcessSetting(L'L', m_process);
    m_leader = box.GetProcessSetting(L'R', m_process);

    m_internet_list = box.GetInternetProcess();
    m_startrun_list = box.GetStartRunProcess();
    m_internet = RemoveProgramFromList(m_internet_list);
    m_startrun = RemoveProgramFromList(m_startrun_list);

    //
    // display dialog
    //

    DoModal();
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CProcSettingsDialog::~CProcSettingsDialog()
{
}


//---------------------------------------------------------------------------
// RemoveProgramFromList
//---------------------------------------------------------------------------


BOOL CProcSettingsDialog::RemoveProgramFromList(CString &list)
{
    CString list2;
    BOOL first = TRUE;
    BOOL removed = FALSE;

    int i = 0;
    while (i < list.GetLength()) {
        int j = list.Find(L',', i);
        if (j == -1)
            j = list.GetLength();
        CString token = list.Mid(i, j - i);
        token.TrimLeft();
        token.TrimRight();
        if (token.CompareNoCase(m_process) == 0)
            removed = TRUE;
        else {
            if (first)
                first = FALSE;
            else
                list2 += ",";
            list2 += token;
        }
        i = j + 1;
    }

    list = list2;
    return removed;
}


//---------------------------------------------------------------------------
// CheckIfBrackets
//---------------------------------------------------------------------------


BOOL CProcSettingsDialog::CheckIfBrackets(WCHAR type, UINT idButton)
{
    CBox &box = CBoxes::GetInstance().GetBox(m_boxname);

    CStringList list;
    box.GetProcessList(type, list, TRUE);

    while (! list.IsEmpty()) {
        CString image = list.RemoveHead();
        if (CSbieIni::GetInstance().StripBrackets(image)) {
            if (image.CompareNoCase(m_process) == 0) {

                if (idButton) {

                    CButton *pButton = (CButton *)GetDlgItem(idButton);
                    int check = pButton->GetCheck();
                    if (check == BST_UNCHECKED)
                        check = BST_CHECKED;
                    else
                        check = BST_UNCHECKED;
                    pButton->SetCheck(check);
                }

                CMyApp::MsgBox(this, MSG_4222, MB_OK);

                return TRUE;
            }
        }
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// OnInitDialog
//---------------------------------------------------------------------------


BOOL CProcSettingsDialog::OnInitDialog()
{
    //
    // set dialog text
    //

    SetWindowText(CMyMsg(MSG_3741));

    GetDlgItem(ID_SETTINGS_SANDBOX_LABEL)->SetWindowText(CMyMsg(MSG_3742));
    GetDlgItem(ID_SETTINGS_SANDBOX)->SetWindowText(m_boxname);
    GetDlgItem(ID_SETTINGS_PROGRAM_LABEL)->SetWindowText(CMyMsg(MSG_3743));
    GetDlgItem(ID_SETTINGS_PROGRAM)->SetWindowText(m_process);

    ((CButton *)GetDlgItem(ID_SETTINGS_PAGE_1))->SetCheck(BST_CHECKED);
    GetDlgItem(ID_SETTINGS_PAGE_1)->SetWindowText(CMyMsg(MSG_3744));
    GetDlgItem(ID_SETTINGS_PAGE_2)->SetWindowText(CMyMsg(MSG_3745));

    GetDlgItem(IDOK)->SetWindowText(CMyMsg(MSG_3001));
    GetDlgItem(IDCANCEL)->SetWindowText(CMyMsg(MSG_3002));

    //
    // page 1 texts
    //

    GetDlgItem(ID_SETTINGS_GROUP1P1)->SetWindowText(CMyMsg(MSG_3751));
    GetDlgItem(ID_SETTINGS_LABEL_P1)->SetWindowText(CMyMsg(MSG_3752));
    GetDlgItem(ID_SETTINGS_ALERT)->SetWindowText(CMyMsg(MSG_3753));
    GetDlgItem(ID_SETTINGS_FORCE)->SetWindowText(CMyMsg(MSG_3754));

    GetDlgItem(ID_SETTINGS_GROUP2P1)->SetWindowText(CMyMsg(MSG_3755));
    GetDlgItem(ID_SETTINGS_LINGER)->SetWindowText(CMyMsg(MSG_3756));
    GetDlgItem(ID_SETTINGS_LEADER)->SetWindowText(CMyMsg(MSG_3757));

    //
    // page 2 texts
    //

    GetDlgItem(ID_SETTINGS_GROUP1P2)->SetWindowText(CMyMsg(MSG_3761));
    GetDlgItem(ID_SETTINGS_INTERNET_LIST)->SetWindowText(CMyMsg(MSG_3762));
    GetDlgItem(ID_SETTINGS_INTERNET)->SetWindowText(CMyMsg(MSG_3764));

    GetDlgItem(ID_SETTINGS_GROUP2P2)->SetWindowText(CMyMsg(MSG_3765));
    GetDlgItem(ID_SETTINGS_STARTRUN_LIST)->SetWindowText(CMyMsg(MSG_3766));
    GetDlgItem(ID_SETTINGS_STARTRUN)->SetWindowText(CMyMsg(MSG_3768));

    RECT rc;
    GetDlgItem(ID_SETTINGS_GROUP1P1)->GetWindowRect(&rc);
    ScreenToClient(&rc);
    rc.bottom -= 15;
    GetDlgItem(ID_SETTINGS_GROUP1P1)->MoveWindow(&rc);

    //
    // populate check and radio buttons
    //

    CButton *pButton;

    if (m_alert) {
        pButton = (CButton *)GetDlgItem(ID_SETTINGS_ALERT);
        pButton->SetCheck(TRUE);
    }

    if (m_force) {
        pButton = (CButton *)GetDlgItem(ID_SETTINGS_FORCE);
        pButton->SetCheck(TRUE);
    }

    if (m_linger) {
        pButton = (CButton *)GetDlgItem(ID_SETTINGS_LINGER);
        pButton->SetCheck(TRUE);
    }

    if (m_leader) {
        pButton = (CButton *)GetDlgItem(ID_SETTINGS_LEADER);
        pButton->SetCheck(TRUE);
    }

    if (m_internet) {
        pButton = (CButton *)GetDlgItem(ID_SETTINGS_INTERNET);
        pButton->SetCheck(TRUE);
    }

    if (m_startrun) {
        pButton = (CButton *)GetDlgItem(ID_SETTINGS_STARTRUN);
        pButton->SetCheck(TRUE);
    }

    UpdateInternetAccess();
    UpdateStartRunAccess();

    //
    // select page 1
    //

    ShowPage1(SW_SHOW);
    return TRUE;
}


//---------------------------------------------------------------------------
// ShowPage1
//---------------------------------------------------------------------------


void CProcSettingsDialog::ShowPage1(int nCmdShow)
{
    GetDlgItem(ID_SETTINGS_GROUP1P1)->ShowWindow(nCmdShow);
    GetDlgItem(ID_SETTINGS_LABEL_P1)->ShowWindow(nCmdShow);
    GetDlgItem(ID_SETTINGS_ALERT)->ShowWindow(nCmdShow);
    GetDlgItem(ID_SETTINGS_FORCE)->ShowWindow(nCmdShow);
    if (! GetDlgItem(ID_SETTINGS_FORCE)->IsWindowEnabled())
        GetDlgItem(ID_SETTINGS_REGISTER)->ShowWindow(nCmdShow);

    GetDlgItem(ID_SETTINGS_GROUP2P1)->ShowWindow(nCmdShow);
    GetDlgItem(ID_SETTINGS_LINGER)->ShowWindow(nCmdShow);
    GetDlgItem(ID_SETTINGS_LEADER)->ShowWindow(nCmdShow);
}


//---------------------------------------------------------------------------
// ShowPage2
//---------------------------------------------------------------------------


void CProcSettingsDialog::ShowPage2(int nCmdShow)
{
    GetDlgItem(ID_SETTINGS_GROUP1P2)->ShowWindow(nCmdShow);
    GetDlgItem(ID_SETTINGS_INTERNET_LIST)->ShowWindow(nCmdShow);
    GetDlgItem(ID_SETTINGS_INTERNET)->ShowWindow(nCmdShow);

    GetDlgItem(ID_SETTINGS_GROUP2P2)->ShowWindow(nCmdShow);
    GetDlgItem(ID_SETTINGS_STARTRUN_LIST)->ShowWindow(nCmdShow);
    GetDlgItem(ID_SETTINGS_STARTRUN)->ShowWindow(nCmdShow);
}


//---------------------------------------------------------------------------
// UpdateAccess
//---------------------------------------------------------------------------


void CProcSettingsDialog::UpdateAccess(
    const CString &list, ULONG idList, ULONG idButton,
    ULONG msgidAll, ULONG msgidSome)
{
    CWnd *pList = GetDlgItem(idList);
    CButton *pButton = (CButton *)GetDlgItem(idButton);
    BOOL checked = (pButton->GetCheck() == BST_CHECKED) ? TRUE : FALSE;

    CString text = CMyMsg(msgidSome);
    text += L"  ";

    if (list == L"*") {

        if (checked)
            text += m_process;
        else
            text += CMyMsg(MSG_3769);

    } else {

        BOOL first = TRUE;
        int i = 0;
        while (i < list.GetLength()) {
            int j = list.Find(L',', i);
            if (j == -1)
                j = list.GetLength();
            CString token = list.Mid(i, j - i);
            token.TrimLeft();
            token.TrimRight();
            if (first)
                first = FALSE;
            else
                text += L", ";
            text += token;
            i = j + 1;
        }

        if (checked) {
            if (first)
                first = FALSE;
            else
                text += L", ";
            text += m_process;
        }

        if (first)
            text = CMyMsg(msgidAll);
    }

    pList->SetWindowText(text);
}


//---------------------------------------------------------------------------
// UpdateInternetAccess
//---------------------------------------------------------------------------


void CProcSettingsDialog::UpdateInternetAccess(void)
{
    UpdateAccess(m_internet_list,
                 ID_SETTINGS_INTERNET_LIST, ID_SETTINGS_INTERNET,
                 MSG_3762, MSG_3763);
}


//---------------------------------------------------------------------------
// UpdateStartRunAccess
//---------------------------------------------------------------------------


void CProcSettingsDialog::UpdateStartRunAccess(void)
{
    UpdateAccess(m_startrun_list,
                 ID_SETTINGS_STARTRUN_LIST, ID_SETTINGS_STARTRUN,
                 MSG_3766, MSG_3767);
}


//---------------------------------------------------------------------------
// OnViewPage1
//---------------------------------------------------------------------------


void CProcSettingsDialog::OnViewPage1()
{
    ShowPage2(SW_HIDE);
    ShowPage1(SW_SHOW);
}


//---------------------------------------------------------------------------
// OnViewPage2
//---------------------------------------------------------------------------


void CProcSettingsDialog::OnViewPage2()
{
    ShowPage1(SW_HIDE);
    ShowPage2(SW_SHOW);
}


//---------------------------------------------------------------------------
// OnCmdAlert
//---------------------------------------------------------------------------


void CProcSettingsDialog::OnCmdAlert()
{
    if (CheckIfBrackets(L'A', ID_SETTINGS_ALERT))
        return;
    if (CheckIfBrackets(L'F', ID_SETTINGS_ALERT))
        return;

    CButton *pButton = (CButton *)GetDlgItem(ID_SETTINGS_FORCE);
    pButton->SetCheck(BST_UNCHECKED);
}


//---------------------------------------------------------------------------
// OnCmdForce
//---------------------------------------------------------------------------


void CProcSettingsDialog::OnCmdForce()
{
    if (CheckIfBrackets(L'A', ID_SETTINGS_FORCE))
        return;
    if (CheckIfBrackets(L'F', ID_SETTINGS_FORCE))
        return;

    CButton *pButton = (CButton *)GetDlgItem(ID_SETTINGS_ALERT);
    pButton->SetCheck(FALSE);

    pButton = (CButton *)GetDlgItem(ID_SETTINGS_FORCE);
    if (pButton->GetCheck() == BST_CHECKED &&
            m_process.CompareNoCase(_ExplorerExe) == 0) {

        CMyMsg msg(MSG_3772, m_process);
        int rv = CMyApp::MsgBox(this, msg, MB_YESNO);
        if (rv != IDYES)
            pButton->SetCheck(BST_UNCHECKED);
    }
}


//---------------------------------------------------------------------------
// OnCmdLinger
//---------------------------------------------------------------------------


void CProcSettingsDialog::OnCmdLinger()
{
    if (CheckIfBrackets(L'L', ID_SETTINGS_LINGER))
        return;
    if (CheckIfBrackets(L'R', ID_SETTINGS_LINGER))
        return;

    CButton *pButton = (CButton *)GetDlgItem(ID_SETTINGS_LEADER);
    pButton->SetCheck(BST_UNCHECKED);
}


//---------------------------------------------------------------------------
// OnCmdLeader
//---------------------------------------------------------------------------


void CProcSettingsDialog::OnCmdLeader()
{
    if (CheckIfBrackets(L'R', ID_SETTINGS_LEADER))
        return;
    if (CheckIfBrackets(L'L', ID_SETTINGS_LEADER))
        return;

    CButton *pButton = (CButton *)GetDlgItem(ID_SETTINGS_LINGER);
    pButton->SetCheck(BST_UNCHECKED);
}


//---------------------------------------------------------------------------
// OnCmdInternetAccess
//---------------------------------------------------------------------------


void CProcSettingsDialog::OnCmdInternetAccess()
{
    UpdateInternetAccess();
}


//---------------------------------------------------------------------------
// OnCmdStartRunAccess
//---------------------------------------------------------------------------


void CProcSettingsDialog::OnCmdStartRunAccess()
{
    UpdateStartRunAccess();
}


//---------------------------------------------------------------------------
// OnOK
//---------------------------------------------------------------------------


void CProcSettingsDialog::OnOK()
{
    BOOL ok = TRUE;
    BOOL changes = FALSE;
    BOOL val;
    CButton *pButton;
    CBox &box = CBoxes::GetInstance().GetBox(m_boxname);

    //
    // Alert
    //

    pButton = (CButton *)GetDlgItem(ID_SETTINGS_ALERT);
    val = (pButton->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
    if (ok && (val != m_alert)) {
        ok = box.SetProcessSetting(L'A', m_process, val);
        if (ok)
            changes = TRUE;
    }

    //
    // Force
    //

    pButton = (CButton *)GetDlgItem(ID_SETTINGS_FORCE);
    val = (pButton->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
    if (ok && (val != m_force)) {
        ok = box.SetProcessSetting(L'F', m_process, val);
        if (ok)
            changes = TRUE;
    }

    //
    // Linger
    //

    pButton = (CButton *)GetDlgItem(ID_SETTINGS_LINGER);
    val = (pButton->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
    if (ok && (val != m_linger)) {
        ok = box.SetProcessSetting(L'L', m_process, val);
        if (ok)
            changes = TRUE;
    }

    //
    // Leader
    //

    pButton = (CButton *)GetDlgItem(ID_SETTINGS_LEADER);
    val = (pButton->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
    if (ok && (val != m_leader)) {
        ok = box.SetProcessSetting(L'R', m_process, val);
        if (ok)
            changes = TRUE;
    }

    //
    // Internet
    //

    pButton = (CButton *)GetDlgItem(ID_SETTINGS_INTERNET);
    val = (pButton->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
    if (ok && (val != m_internet)) {
        if (val) {
            if (m_internet_list == L"*")
                m_internet_list = L"";
            if (! m_internet_list.IsEmpty())
                m_internet_list += L",";
            m_internet_list += m_process;
        }
        ok = box.SetInternetProcess(m_internet_list);
        if (ok)
            changes = TRUE;
    }

    //
    // StartRun
    //

    pButton = (CButton *)GetDlgItem(ID_SETTINGS_STARTRUN);
    val = (pButton->GetCheck() == BST_CHECKED) ? TRUE : FALSE;
    if (ok && (val != m_startrun)) {
        if (val) {
            if (m_startrun_list == L"*")
                m_startrun_list = L"";
            if (! m_startrun_list.IsEmpty())
                m_startrun_list += L",";
            m_startrun_list += m_process;
        }
        ok = box.SetStartRunProcess(m_startrun_list);
        if (ok)
            changes = TRUE;
    }

    //
    // Tip
    //

    if (changes) {

        CUserSettings &settings = CUserSettings::GetInstance();

        BOOL tip;
        settings.GetBool(_ProcSettingsNotify, tip, TRUE);

        if (tip) {
            int rv = CMyApp::MsgCheckBox(this, MSG_3770, 0, MB_OK);
            if (rv < 0)
                settings.SetBool(_ProcSettingsNotify, FALSE);
        }
    }

    EndDialog(0);
}
