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
// Popup Messages Dialog Box
//---------------------------------------------------------------------------


#ifndef _MY_MESSAGEDIALOG_H
#define _MY_MESSAGEDIALOG_H


#include "QuickRecover.h"
#include "Box.h"
#include "FlashingButton.h"


#define MSGDLG_NORMAL           1
#define MSGDLG_EVENT_VIEWER     2


class CMessageDialog : public CBaseDialog
{
    DECLARE_MESSAGE_MAP()

    CFlashingButton m_HelpButton;

    BOOL m_firsttime;
    BOOL m_visible;
    BOOL m_can_hide;

    ULONG m_buf_len;
    WCHAR *m_buf;

    CPtrList m_queue;

    CListBox *m_listbox;

    CQuickRecover *m_qr;
    CString m_qr_box;

    CPtrArray m_hidden;

    BOOL IsHiddenMessage(
        ULONG code, const WCHAR *detail_1, const WCHAR *detail_2);

    void DoPopups();
    void DoRecovery();
    void DoAutoPlay(const CBox &box, const WCHAR drive);

    void MyDoModal(CDialog *pDialog);

    virtual BOOL OnInitDialog();

    afx_msg void OnHelp();
    afx_msg void OnHide();
    afx_msg void OnCopy();

    afx_msg void OnListDoubleClick();

    CString GetSBIExxxx(ULONG *pCode, CString *pDetail);
    void DiscardMessages(const CString &sbie, const CString &detail);

    void ReadStartError();
    void ReadEventLog();

public:

    CMessageDialog(CWnd *pParentWnd, int mode);
    ~CMessageDialog();

    void ReloadConf();

    void OnTimer();

    static CWnd *m_GettingStartedWindow;
};


#endif // _MY_MESSAGEDIALOG_H
