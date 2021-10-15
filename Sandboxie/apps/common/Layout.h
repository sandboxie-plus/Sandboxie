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
// Layout Class
//---------------------------------------------------------------------------


#ifndef _MY_LAYOUT_H
#define _MY_LAYOUT_H


#include <afxcmn.h>
#include <afxdlgs.h>


//---------------------------------------------------------------------------
// CLayout Class
//---------------------------------------------------------------------------


class CLayout
{

protected:

    CWnd *m_wnd;
    CPtrList m_children;

protected:

    void CalculatePositionAndSize(void *_childobj, int w, int h,
                                  CPoint &out_pos, CSize &out_size);

    void CalculateOneWindow(void *_childobj);

public:

    CLayout(CWnd *wnd);
    ~CLayout();

    BOOL AttachChild(CWnd *child, const CPoint &pos, const CSize &size);

    BOOL ReplaceChild(CWnd *oldchild, CWnd *newchild);

    BOOL SetMinMaxChildObject(
            CWnd *wnd, const CSize &size_min, const CSize &size_max);

    CWnd *CreateChild(int ctrlid, const WCHAR *clsnm,
                      ULONG exstyle, ULONG style,
                      const CPoint &pos, const CSize &size);

    void DeleteChildren();

    BOOL DeleteChild(HWND hwnd);
    BOOL DeleteChild(CWnd *wnd);

    CWnd *GetChildByIndex(int index);

    void Calculate();

};


//---------------------------------------------------------------------------
// CLayoutWnd Class
//---------------------------------------------------------------------------


class CLayoutWnd : public CWnd
{

protected:

    DECLARE_MESSAGE_MAP()

    CLayout m_layout;

protected:

    afx_msg void OnSize(UINT nType, int cx, int cy);

    afx_msg void OnSizing(UINT nSide, LPRECT lpRect);

    afx_msg void OnDestroy();

    afx_msg void OnParentNotify(UINT message, LPARAM lParam);

    static LRESULT LayoutCtrlWndClass(
        HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:

    CWnd *CreateChild(int ctrlid, const WCHAR *clsnm,
                      ULONG exstyle, ULONG style,
                      const CPoint &pos, const CSize &size);

    CLayoutWnd *CreateLayoutCtrl(const CPoint &pos, const CSize &size);

    CWnd *GetChildByIndex(int index);

    CLayoutWnd();

};


//---------------------------------------------------------------------------
// CLayoutPropertyPage Class
//---------------------------------------------------------------------------


class CLayoutPropertyPage : public CPropertyPage
{

protected:

    DECLARE_MESSAGE_MAP()

    CLayout m_layout;

    ULONG m_dlgtmpl[16];

    HBRUSH m_hbrush;

    int m_page_num;

protected:

    virtual BOOL OnInitDialog();

    virtual BOOL OnSetActive();

    afx_msg BOOL OnEraseBkgnd(CDC *pDC);

    afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);

public:

    CWnd *CreateChild(int ctrlid, const WCHAR *clsnm,
                      ULONG exstyle, ULONG style,
                      const CPoint &pos, const CSize &size);

    CLayoutPropertyPage(int page_num);

    void InitPage(ULONG exstyle, const CSize &abs_size, HBRUSH hbrush);

    void InitPage(ULONG exstyle, const CSize &pct_size, const CSize &size_min, const CSize &size_max, HBRUSH hbrush);

};


//---------------------------------------------------------------------------


#endif // _MY_LAYOUT_H
