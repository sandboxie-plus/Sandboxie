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
// Set Layout Dialog Box
//---------------------------------------------------------------------------


#ifndef _MY_SETLAYOUTDIALOG_H
#define _MY_SETLAYOUTDIALOG_H


class CSetLayoutDialog : public CBaseDialog
{
    DECLARE_MESSAGE_MAP()

    static LRESULT MyKeyboardHook(int code, WPARAM wParam, LPARAM lParam);

    virtual BOOL OnInitDialog();

    void InitTree();
    void InitTree2(HTREEITEM hParent, void *_order_entry);

    virtual void OnOK();

    void ConvertToBoxOrder(HTREEITEM hParent, void *_order_entry);

    afx_msg void OnRightClick(NMHDR *pNMHDR, LRESULT *plResult);

    void CreateMoveMenu(CMenu *menu, CPtrArray *handles);
    void CreateMoveMenu2(
        CMenu *menu, CPtrArray *handles, HTREEITEM hItem, CString prefix);

    afx_msg void OnRestore();

    afx_msg void OnInsert();

    afx_msg void OnRename();

    afx_msg void OnLabelEdit(NMHDR *pNMHDR, LRESULT *plResult);

    afx_msg void OnDelete();

    afx_msg void OnMoveUp();

    afx_msg void OnMoveDown();

    afx_msg void OnMoveTo();

    HTREEITEM ReparentItem(HTREEITEM hItem, HTREEITEM hNewParentItem,
                           HTREEITEM hInsertAfterItem = NULL);

    void ReparentItemForDelete(HTREEITEM hItem, HTREEITEM hNewParentItem);

private:

    CTreeCtrl *m_tree;
    static HHOOK m_hhook;
    static bool m_inLabelEdit;

public:

    CSetLayoutDialog(CWnd *pParentWnd);
    ~CSetLayoutDialog();
};


#endif // _MY_SETLAYOUTDIALOG_H
