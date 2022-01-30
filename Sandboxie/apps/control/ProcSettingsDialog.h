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


#ifndef _MY_PROCESSSETTINGSDIALOG_H
#define _MY_PROCESSSETTINGSDIALOG_H


class CProcSettingsDialog : public CBaseDialog
{
    DECLARE_MESSAGE_MAP()

    CString m_boxname, m_process, m_internet_list, m_startrun_list;
    BOOL m_alert, m_force, m_linger, m_leader, m_internet, m_startrun;

    BOOL RemoveProgramFromList(CString &list);

    BOOL CheckIfBrackets(WCHAR type, UINT idButton);

    virtual BOOL OnInitDialog();
    virtual void OnOK();

    void ShowPage1(int nCmdShow);
    void ShowPage2(int nCmdShow);

    void UpdateAccess(const CString &list, ULONG idList, ULONG idButton,
                      ULONG msgidAll, ULONG msgidSome);
    void UpdateInternetAccess(void);
    void UpdateStartRunAccess(void);

    afx_msg void OnViewPage1();
    afx_msg void OnViewPage2();

    afx_msg void OnCmdAlert();
    afx_msg void OnCmdForce();

    afx_msg void OnCmdLinger();
    afx_msg void OnCmdLeader();

    afx_msg void OnCmdInternetAccess();
    afx_msg void OnCmdStartRunAccess();

public:

    CProcSettingsDialog(CWnd *pParentWnd, ULONG pid);
    ~CProcSettingsDialog();
};


#endif // _MY_PROCESSSETTINGSDIALOG_H
