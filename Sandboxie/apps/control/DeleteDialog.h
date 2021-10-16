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
// Delete Sandbox Dialog Box
//---------------------------------------------------------------------------


#ifndef _MY_DELETEDIALOG_H
#define _MY_DELETEDIALOG_H


#include "FileListCtrl.h"
#include "FlashingButton.h"
#include "ToolTipButton.h"


class CDeleteDialog : public CBaseDialog
{
    DECLARE_MESSAGE_MAP()

    BOOL m_WithDelete;
    BOOL m_AutoDelete;

    CFileListCtrl m_filelist;
    CFlashingButton m_DeleteButton;

    CToolTipButton SelectAllButton;

    BOOL m_empty;
    int m_files;
    int m_folders;
    ULONG64 m_size;
    CStringList m_stack;

    BOOL m_spawned;

    virtual BOOL OnInitDialog();

    afx_msg void OnTimer(UINT_PTR nIDEvent);

    virtual void OnOK();

    afx_msg void OnCmdAddFolder();
    afx_msg void OnCmdRecoverSameAny();

    afx_msg void OnSelectAll();

    afx_msg void OnContextMenu(CWnd *pWnd, CPoint pt);

public:

    CDeleteDialog(CWnd *pParentWnd, const WCHAR *BoxName,
                  BOOL WithDelete, BOOL AutoDelete,
                  BOOL *DeleteSpawned);
    ~CDeleteDialog();
};


#endif // _MY_DELETEDIALOG_H
