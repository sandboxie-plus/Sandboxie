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
// Quick Recover
//---------------------------------------------------------------------------


#ifndef _MY_QUICKRECOVER_H
#define _MY_QUICKRECOVER_H


#include "ToolTipButton.h"


#define QR_SAME                     10
#define QR_ANY                      20
#define QR_AUTO                     30
#define QR_LOG                      40


class CQuickRecover : public CBaseDialog
{
    DECLARE_MESSAGE_MAP()

    CString m_BoxName;
    CString m_SrcPath;
    CString m_DstPath;

    CStringList m_items;
    CStringList m_list;
    BOOL m_check;
    BOOL m_ready;
    BOOL m_replace;
    BOOL m_log;

    HWND m_hwndTree;
    CStatic m_static;
    CButton m_checkbox;
    CListBox m_listbox;

    ULONG m_log_count;

    CToolTipButton SelectAllButton;
    CToolTipButton RightClickButton;
    CToolTipButton CycleRecoverButton;

    int m_recover_mode;

    static ULONG m_TempFolderCounter;
    static CStringList m_past_files;

    static const WCHAR *ReplaceButtonText;

    afx_msg void OnDestroy();
    afx_msg void OnListSelect();
    afx_msg int  OnVKeyToItem(UINT nKey, CListBox *pListBox, UINT nIndex);
    afx_msg void OnSelectAll();
    afx_msg void OnContextMenu(CWnd *pWnd, CPoint pt);
    afx_msg void OnCheckBoxClick();
    afx_msg void OnCycleRecover();

    void InitFolderList(bool AddHead, int SelectIndex);
    void AddHeadFolderList(ULONG msgid);

    CString GetDestPlusRelative(
        const CString &DestPath, const CString &RelativePath);

    BOOL RecoverToAnyFolder(const CString &RelativePath = CString());

    static int CALLBACK BrowseCallback(
        HWND hwnd, UINT msg, LPARAM lParam, LPARAM lpData);

    void OnInitDialogAnyFolder();

    void AutoRecover();

    void MoveButtonIntoList(
        CButton *pButton, CListBox *pListBox, HBITMAP hBitmap);

    virtual BOOL OnInitDialog();

    virtual void OnOK();

    BOOL RecoverToSameFolder();

    BOOL RecoverFile(const CString &DestPath, BOOL SameFolder = FALSE);
    BOOL RecoverFile2(WCHAR *src, WCHAR *dst, BOOL SameFolder);
    BOOL RecoverFile3(WCHAR *src, WCHAR *dst, BOOL SameFolder);
    void RecoverFileX(WCHAR *src);

    BOOL RecoverFolder(BOOL SameFolder);

    //
    // recovery log
    //

    void RecoveryLog();

    BOOL OnInitDialogRecoveryLog();
    void OnOKRecoveryLog();
    afx_msg void OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized);
    afx_msg void OnRemoveFolders();

public:

    CQuickRecover(CWnd *pParentWnd, const CString &BoxName,
                  const CString &SrcPath, int qrMode);

    CQuickRecover(CWnd *pParentWnd, const CString &BoxName,
                  CStringList &list, int qrMode);

    ~CQuickRecover();

    void AddAutoRecoverItem(const CString &SrcPath);
};


#endif // _MY_QUICKRECOVER_H
