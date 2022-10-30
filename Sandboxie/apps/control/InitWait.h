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
// Initialization Wait
//---------------------------------------------------------------------------


#ifndef _MY_INITWAIT_H
#define _MY_INITWAIT_H


#include "MessageDialog.h"


class CInitWait : public CWnd
{
    DECLARE_MESSAGE_MAP()

    CString m_app_ver;
    CString m_svc_ver;
    ULONG   m_svc_abi;
    CString m_drv_ver;
    ULONG   m_drv_abi;

    HICON m_hIcon1;
    HICON m_hIcon2;
    HICON m_hIconPtr;
    CMenu *m_pMenu;

    BOOL m_try_elevate;

    CMessageDialog *m_pMessageDialog;

    void GetVersions();

    afx_msg void OnDestroy();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg LRESULT OnTaskbarCreated(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnTrayIcon(WPARAM wParam, LPARAM lParam);
    afx_msg void OnShowErrors();

public:

    CInitWait(CWinApp *myApp);
    ~CInitWait();

};


#endif // _MY_INITWAIT_H