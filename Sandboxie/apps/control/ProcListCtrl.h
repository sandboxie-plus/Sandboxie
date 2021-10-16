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
// Process List Control
//---------------------------------------------------------------------------


#ifndef _MY_PROC_LIST_CTRL_H
#define _MY_PROC_LIST_CTRL_H


#include "MyListCtrl.h"


class CProcListCtrl : public CMyListCtrl
{
    DECLARE_MESSAGE_MAP()

    int m_pid;
    void *m_box_order;
    void *m_box_order_selected;

    //
    //
    //

    virtual afx_msg void OnSize(UINT nType, int cx, int cy);

    virtual afx_msg void OnRefreshPosted();

    int RefreshOneBox(CBoxProc &boxproc, int ListIndex);

    int InsertProcessRow(
            int ListIndex, int ProcessId, const CString &ImageName,
            const CString &WindowTitle, int IconIndex);
    int InsertBoxRow(
            int ListIndex, const CString &BoxName, int IconIndex);

    afx_msg void OnCmdTerminate();
    afx_msg void OnCmdSettings();
    afx_msg void OnCmdResources();

    virtual afx_msg void OnClick(NMHDR *pNMHDR, LRESULT *pResult);

    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    void OnArrowKeyDown(UINT nChar);

    afx_msg void OnDestroy();

    //
    //
    //

    void RebuildCombo2(void *_order_entry, const CString &prefix);

    virtual afx_msg void OnComboSelect();

    //
    //
    //

public:

    BOOL Create(CWnd *pParentWnd);

    virtual void RefreshContent();

    void SelectByProcessId(int pid);
    int GetRightClickPid() const;

    virtual CBox &GetSelectedBox(CWnd *pWnd, CPoint pt) const;
    virtual void OnContextMenu(CWnd *pWnd, CPoint pt);

    void RebuildCombo();
};


#endif // _MY_PROC_LIST_CTRL_H
