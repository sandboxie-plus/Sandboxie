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
// Base List Control
//---------------------------------------------------------------------------


#ifndef _MY_LIST_CTRL_H
#define _MY_LIST_CTRL_H


#include "MenuXP.h"
#include "Box.h"
#include "TabbingComboBox.h"


class CMyListCtrl : public CListCtrl
{

protected:

    DECLARE_MESSAGE_MAP()
    DECLARE_MENUXP()

    CMenu *m_pContextMenu;

    CMapPtrToWord m_iconmap;
    int m_imgEmpty, m_imgMinus, m_imgPlus;

    ULONG m_LastImageTicks;
    int m_LastImageCount;

    CTabbingComboBox m_combo;

    //
    //
    //

    BOOL Create(CWnd *pParentWnd, int style, const WCHAR *prefix);

    void CreateComboButton(void);

    void ResizeComboButton(void);

    void InitImageList(const CString &prefix);

    int AddToImageList(HICON icon);

    bool AddToImageList2(CBitmap *bmp, int w, int h, bool mask);

    void RebuildImageList();

    void SelectIndex(int ListIndex);

    void PostRefresh(LPARAM lParam = 0);

    BOOL MyDeleteItem(int ListIndex);

    void GetOrderedBoxList(CStringList &BoxList, void *BoxOrder);

    //
    //
    //

    virtual afx_msg void OnRefreshPosted() = 0;

    virtual afx_msg void OnClick(NMHDR *pNMHDR, LRESULT *pResult) = 0;

    virtual afx_msg void OnSize(UINT nType, int cx, int cy) = 0;

    virtual afx_msg void OnComboSelect() = 0;

    //
    //
    //

public:

    virtual CBox &GetSelectedBox(CWnd *pWnd, CPoint pt) const = 0;
    virtual void GetSelectedItemPosition(CPoint &pt) const;
    virtual void OnContextMenu(CWnd *pWnd, CPoint pt) = 0;

    virtual void RefreshContent() = 0;

    void ShowComboBox(bool visible);
};


#endif // _MY_LIST_CTRL_H
