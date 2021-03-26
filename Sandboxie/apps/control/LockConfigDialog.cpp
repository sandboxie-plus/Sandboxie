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
// Lock Configuration Dialog Box
//---------------------------------------------------------------------------


#include "stdafx.h"
#include "MyApp.h"
#include "LockConfigDialog.h"

#include "SbieIni.h"
#include "UserSettings.h"
#include "apps/common/RunBrowser.h"


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CLockConfigDialog, CBaseDialog)

    ON_COMMAND(ID_LOCK_ENABLE_PASSWORD,     OnEnablePassword)
    ON_COMMAND(ID_LOCK_CHANGE_PASSWORD,     OnChangePassword)

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CLockConfigDialog::CLockConfigDialog(CWnd *pParentWnd)
    : CBaseDialog(pParentWnd, L"LOCK_CONFIG_DIALOG")
{
    *m_NewPassword = L'\0';
    m_RemovePassword = FALSE;

    DoModal();

    SecureZeroMemory(m_NewPassword, sizeof(m_NewPassword));
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CLockConfigDialog::~CLockConfigDialog()
{
}


//---------------------------------------------------------------------------
// OnInitDialog
//---------------------------------------------------------------------------


BOOL CLockConfigDialog::OnInitDialog()
{
    //
    // prepare dialog text
    //

    SetWindowText(CMyMsg(MSG_4261));

    GetDlgItem(ID_PAGE_LABEL_1)->SetWindowText(CMyMsg(MSG_4262));

    GetDlgItem(ID_LOCK_ADMIN_ONLY)->SetWindowText(CMyMsg(MSG_4263));
    GetDlgItem(ID_LOCK_FORCE_ADMIN_ONLY)->SetWindowText(CMyMsg(MSG_4267));

    if (! CUserSettings::GetInstance().IsAdmin()) {

        GetDlgItem(ID_LOCK_ADMIN_ONLY)->EnableWindow(FALSE);
        GetDlgItem(ID_LOCK_FORCE_ADMIN_ONLY)->EnableWindow(FALSE);
    }

    GetDlgItem(ID_LOCK_ENABLE_PASSWORD)->SetWindowText(CMyMsg(MSG_4264));
    GetDlgItem(ID_LOCK_CHANGE_PASSWORD)->SetWindowText(CMyMsg(MSG_4266));
    GetDlgItem(ID_LOCK_FORGET_PASSWORD)->SetWindowText(CMyMsg(MSG_4265));

    GetDlgItem(IDOK)->SetWindowText(CMyMsg(MSG_3001));
    GetDlgItem(IDCANCEL)->SetWindowText(CMyMsg(MSG_3002));

    //
    // set checkboxes
    //

    CButton *pButton;

    BOOL isEditAdminOnly, isForceDisableAdminOnly;
    BOOL isEditPassword, isHavePassword, isForgetPassword;
    CSbieIni::GetInstance().GetRestrictions(
        isEditAdminOnly, isForceDisableAdminOnly,
        isEditPassword, isHavePassword, isForgetPassword);

    if (isEditAdminOnly) {
        pButton = (CButton *)GetDlgItem(ID_LOCK_ADMIN_ONLY);
        pButton->SetCheck(BST_CHECKED);
    }

    if (isEditPassword) {
        pButton = (CButton *)GetDlgItem(ID_LOCK_ENABLE_PASSWORD);
        pButton->SetCheck(BST_CHECKED);
        pButton = (CButton *)GetDlgItem(ID_LOCK_CHANGE_PASSWORD);
        pButton->ShowWindow(SW_SHOW);
    }

    if (isForgetPassword) {
        pButton = (CButton *)GetDlgItem(ID_LOCK_FORGET_PASSWORD);
        pButton->SetCheck(BST_CHECKED);
    }

    if (isForceDisableAdminOnly) {
        pButton = (CButton *)GetDlgItem(ID_LOCK_FORCE_ADMIN_ONLY);
        pButton->SetCheck(BST_CHECKED);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// OnEnablePassword
//---------------------------------------------------------------------------


void CLockConfigDialog::OnEnablePassword()
{
    CButton *pButton1 = (CButton *)GetDlgItem(ID_LOCK_ENABLE_PASSWORD);
    CButton *pButton2 = (CButton *)GetDlgItem(ID_LOCK_CHANGE_PASSWORD);

    if (pButton1->GetCheck() == BST_CHECKED) {

        pButton1->SetCheck(BST_UNCHECKED);
        pButton2->ShowWindow(SW_HIDE);

        m_RemovePassword = TRUE;
        *m_NewPassword = L'\0';

    } else {

        pButton1->SetCheck(BST_CHECKED);
        pButton2->ShowWindow(SW_SHOW);

        m_RemovePassword = FALSE;
        if (! *m_NewPassword)
            OnChangePassword();

        if (! *m_NewPassword) {

            pButton1->SetCheck(BST_UNCHECKED);
            pButton2->ShowWindow(SW_HIDE);

            m_RemovePassword = TRUE;
        }
    }
}


//---------------------------------------------------------------------------
// OnChangePassword
//---------------------------------------------------------------------------


void CLockConfigDialog::OnChangePassword()
{
    CSbieIni &ini = CSbieIni::GetInstance();
    if (! ini.SetOrTestPassword(NULL))
        return;

    WCHAR Pass1[66], Pass2[66];

retry:

    if (! ini.InputPassword(MSG_4272, Pass1))
        return;
    if (! ini.InputPassword(MSG_4273, Pass2))
        return;

    if (wcscmp(Pass1, Pass2) == 0)
        wcscpy(m_NewPassword, Pass1);
    else {
        int rc = CMyApp::MsgBox(this, MSG_4274, MB_YESNO);
        if (rc == IDYES)
            goto retry;
    }

    SecureZeroMemory(Pass1, sizeof(Pass1));
    SecureZeroMemory(Pass2, sizeof(Pass2));
}


//---------------------------------------------------------------------------
// OnOK
//---------------------------------------------------------------------------


void CLockConfigDialog::OnOK()
{
    CSbieIni &ini = CSbieIni::GetInstance();

    if (! ini.SetOrTestPassword(NULL))
        return;

    if (*m_NewPassword || m_RemovePassword) {
        if (m_RemovePassword)
            *m_NewPassword = L'\0';
        ini.SetOrTestPassword(m_NewPassword);
    }

    BOOL isEditAdminOnly = FALSE;
    BOOL isForceDisableAdminOnly = FALSE;
    BOOL isForgetPassword = FALSE;

    CButton *pButton = (CButton *)GetDlgItem(ID_LOCK_ADMIN_ONLY);
    if (pButton->GetCheck() == BST_CHECKED)
        isEditAdminOnly = TRUE;

    pButton = (CButton *)GetDlgItem(ID_LOCK_FORCE_ADMIN_ONLY);
    if (pButton->GetCheck() == BST_CHECKED)
        isForceDisableAdminOnly = TRUE;

    pButton = (CButton *)GetDlgItem(ID_LOCK_FORGET_PASSWORD);
    if (pButton->GetCheck() == BST_CHECKED)
        isForgetPassword = TRUE;

    ini.SetRestrictions(
        isEditAdminOnly, isForceDisableAdminOnly, isForgetPassword);

    //if ((*m_NewPassword) || isEditAdminOnly) {
    //    int rv = CMyApp::MsgBox(this, MSG_4269, MB_YESNO);
    //    if (rv == IDYES)
    //        CRunBrowser::OpenHelp(this, L"ConfigurationProtection");
    //}

    EndDialog(0);
}
