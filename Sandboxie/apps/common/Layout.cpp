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

#include "stdafx.h"
#include "Layout.h"
#include "common/defines.h"
#include <math.h>


//---------------------------------------------------------------------------
// CLayoutChild Class
//---------------------------------------------------------------------------


struct CLayoutChild
{
    CWnd *m_wnd;
    CPoint m_pos;               // in percent
    CSize m_size;               // in percent
    CSize m_size_min;           // in pixels
    CSize m_size_max;           // in pixels

    CLayoutChild(CWnd *wnd, const CPoint &pos, const CSize &size)
    {
        m_wnd = wnd; m_pos = pos; m_size = size;
        m_size_min.cx = 0; m_size_min.cy = 0;
        m_size_max.cx = 0; m_size_max.cy = 0;
    }
};


//---------------------------------------------------------------------------
// CLayout Constructor
//---------------------------------------------------------------------------


CLayout::CLayout(CWnd *wnd)
{
    m_wnd = wnd;
}


//---------------------------------------------------------------------------
// CLayout Destructor
//---------------------------------------------------------------------------


CLayout::~CLayout()
{
    DeleteChildren();
}


//---------------------------------------------------------------------------
// CLayout::AttachChild
//---------------------------------------------------------------------------


BOOL CLayout::AttachChild(
    CWnd *wnd, const CPoint &point, const CSize &size)
{
    CLayoutChild *childobj = new CLayoutChild(wnd, point, size);
    if (! childobj)
        return FALSE;
    m_children.AddTail(childobj);
    if (m_wnd->IsWindowVisible())
        Calculate();
    return TRUE;
}


//---------------------------------------------------------------------------
// CLayout::ReplaceChild
//---------------------------------------------------------------------------


BOOL CLayout::ReplaceChild(CWnd *oldchild, CWnd *newchild)
{
    POSITION pos = m_children.GetHeadPosition();
    while (pos) {
        POSITION save_pos = pos;
        CLayoutChild *childobj = (CLayoutChild *)m_children.GetNext(pos);
        if (childobj->m_wnd == oldchild) {
            delete oldchild;
            childobj->m_wnd = newchild;
            return TRUE;
        }
    }
    return FALSE;
}


//---------------------------------------------------------------------------
// CLayout::SetMinMaxChildObject
//---------------------------------------------------------------------------


BOOL CLayout::SetMinMaxChildObject(
        CWnd *wnd, const CSize &size_min, const CSize &size_max)
{
    POSITION pos = m_children.GetHeadPosition();
    while (pos) {
        CLayoutChild *childobj = (CLayoutChild *)m_children.GetNext(pos);
        if (childobj->m_wnd == wnd) {
            childobj->m_size_min = size_min;
            childobj->m_size_max = size_max;
            return TRUE;
        }
    }
    return FALSE;
}


//---------------------------------------------------------------------------
// CLayout::CreateChild
//---------------------------------------------------------------------------


CWnd *CLayout::CreateChild(
    int ctrlid, const WCHAR *clsnm, ULONG exstyle, ULONG style,
    const CPoint &pos, const CSize &size)
{
    CWnd *wnd = new CWnd();
    if (wnd->CreateEx(exstyle, clsnm, L"", (style | WS_CHILD | WS_VISIBLE),
                      CRect(), m_wnd, ctrlid, NULL)) {

        if (AttachChild(wnd, pos, size)) {

            return wnd;
        }
    }

    delete wnd;
    return NULL;
}


//---------------------------------------------------------------------------
// CLayout::DeleteChild
//---------------------------------------------------------------------------


BOOL CLayout::DeleteChild(HWND hwnd)
{
    POSITION pos = m_children.GetHeadPosition();
    while (pos) {
        POSITION save_pos = pos;
        CLayoutChild *childobj = (CLayoutChild *)m_children.GetNext(pos);
        if (childobj->m_wnd->m_hWnd == hwnd) {
            m_children.RemoveAt(save_pos);
            delete childobj->m_wnd;
            delete childobj;
            return TRUE;
        }
    }
    return FALSE;
}


//---------------------------------------------------------------------------
// CLayout::DeleteChild
//---------------------------------------------------------------------------


BOOL CLayout::DeleteChild(CWnd *wnd)
{
    return DeleteChild(wnd->m_hWnd);
}


//---------------------------------------------------------------------------
// CLayout::DeleteChildren
//---------------------------------------------------------------------------


void CLayout::DeleteChildren()
{
    while (! m_children.IsEmpty()) {
        CLayoutChild *childobj = (CLayoutChild *)m_children.RemoveHead();
        delete childobj->m_wnd;
        delete childobj;
    }
}


//---------------------------------------------------------------------------
// CLayout::GetChildByIndex
//---------------------------------------------------------------------------


CWnd *CLayout::GetChildByIndex(int index)
{
    POSITION pos = m_children.GetHeadPosition();
    while (pos) {
        CLayoutChild *childobj = (CLayoutChild *)m_children.GetNext(pos);
        if (index == 0)
            return childobj->m_wnd;
        --index;
    }
    return NULL;
}


//---------------------------------------------------------------------------
// CLayout::CalculatePositionAndSize
//---------------------------------------------------------------------------


void CLayout::CalculatePositionAndSize(void *_childobj, int w, int h,
                                       CPoint &out_pos, CSize &out_size)
{
    CLayoutChild *childobj = (CLayoutChild *)_childobj;

    int x  = (int)floorf(childobj->m_pos.x   * w / 100.0f + 0.5f);
    int y  = (int)floorf(childobj->m_pos.y   * h / 100.0f + 0.5f);
    int cx = (int)floorf(childobj->m_size.cx * w / 100.0f + 0.5f);
    int cy = (int)floorf(childobj->m_size.cy * h / 100.0f + 0.5f);

    if (childobj->m_size_min.cx && cx < childobj->m_size_min.cx)
        cx = childobj->m_size_min.cx;
    if (childobj->m_size_min.cy && cy < childobj->m_size_min.cy)
        cy = childobj->m_size_min.cy;
    if (childobj->m_size_max.cx && cx > childobj->m_size_max.cx)
        cx = childobj->m_size_max.cx;
    if (childobj->m_size_max.cy && cy > childobj->m_size_max.cy)
        cy = childobj->m_size_max.cy;

    out_pos.x = x;
    out_pos.y = y;
    out_size.cx = cx;
    out_size.cy = cy;
}


//---------------------------------------------------------------------------
// CLayout::CalculateOneWindow
//---------------------------------------------------------------------------


void CLayout::CalculateOneWindow(void *_childobj)
{
    CLayoutChild *childobj = (CLayoutChild *)_childobj;

    CRect rc;
    m_wnd->GetClientRect(&rc);

    CPoint pos;
    CSize size;
    CalculatePositionAndSize(
        childobj, rc.Width(), rc.Height(), pos, size);

    childobj->m_wnd->SetWindowPos(NULL, pos.x, pos.y, size.cx, size.cy,
        SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
}


//---------------------------------------------------------------------------
// CLayout::Calculate
//---------------------------------------------------------------------------


void CLayout::Calculate()
{
    int num_children = (int)m_children.GetCount();
    if (num_children <= 1) {
        if (num_children == 1)
            CalculateOneWindow(m_children.GetHead());
        return;
    }

    CRect rc;
    m_wnd->GetClientRect(&rc);
    int w = rc.Width();
    int h = rc.Height();

    HDWP hdwp = BeginDeferWindowPos(num_children + 2);

    POSITION pos = m_children.GetHeadPosition();
    while (pos) {

        CLayoutChild *childobj = (CLayoutChild *)m_children.GetNext(pos);

        CPoint pos;
        CSize size;
        CalculatePositionAndSize(
            childobj, rc.Width(), rc.Height(), pos, size);

        hdwp = DeferWindowPos(
            hdwp, childobj->m_wnd->m_hWnd, 0,
            pos.x, pos.y, size.cx, size.cy,
            SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
    }

    EndDeferWindowPos(hdwp);
}


//---------------------------------------------------------------------------
// CLayoutWnd Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CLayoutWnd, CWnd)

    ON_WM_SIZE()
    ON_WM_SIZING()
    ON_WM_DESTROY()
    ON_WM_PARENTNOTIFY()

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// CLayoutWnd Constructor
//---------------------------------------------------------------------------


#pragma warning(push)
#pragma warning(disable: 4355)

CLayoutWnd::CLayoutWnd()
    : m_layout(this)
{
}

#pragma warning(pop)


//---------------------------------------------------------------------------
// CLayoutWnd::CreateChild
//---------------------------------------------------------------------------


CWnd *CLayoutWnd::CreateChild(
    int ctrlid, const WCHAR *clsnm, ULONG exstyle, ULONG style,
    const CPoint &pos, const CSize &size)
{
    return m_layout.CreateChild(ctrlid, clsnm, exstyle, style, pos, size);
}


//---------------------------------------------------------------------------
// CLayoutWnd::CreateLayoutCtrl
//---------------------------------------------------------------------------


CLayoutWnd *CLayoutWnd::CreateLayoutCtrl(
    const CPoint &pos, const CSize &size)
{
    static ATOM _layout_ctrl_atom = NULL;

    if (! _layout_ctrl_atom) {

        WNDCLASSEX wc;
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_GLOBALCLASS;
        wc.lpfnWndProc = LayoutCtrlWndClass;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = AfxGetInstanceHandle();
        wc.hIcon = NULL;
        wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = NULL;
        wc.lpszMenuName = NULL;
        wc.lpszClassName = L"LayoutCtrlClass";
        wc.hIconSm = NULL;

        _layout_ctrl_atom = RegisterClassEx(&wc);
        if (! _layout_ctrl_atom)
            return FALSE;
    }

    CLayoutWnd *ctrl = new CLayoutWnd();
    if (ctrl) {

        const ULONG exstyle = 0;
        const ULONG style   = WS_CHILD | WS_VISIBLE;

        if (ctrl->CreateEx(exstyle, (LPCTSTR)_layout_ctrl_atom, L"",
                           style, CRect(), this, -1, NULL)) {

            if (m_layout.AttachChild(ctrl, pos, size)) {

                return ctrl;
            }

            ctrl->DestroyWindow();
        }

        delete ctrl;
    }

    return NULL;
}


//---------------------------------------------------------------------------
// CLayoutWnd::LayoutCtrlWndClass
//---------------------------------------------------------------------------


LRESULT CLayoutWnd::LayoutCtrlWndClass(
    HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if ((uMsg >= WM_CTLCOLORMSGBOX && uMsg <= WM_CTLCOLORSTATIC) ||
        uMsg == WM_COMMAND) {

        hwnd = ::GetParent(hwnd);
        return ::SendMessage(hwnd, uMsg, wParam, lParam);

    } else
        return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}


//---------------------------------------------------------------------------
// CLayoutWnd::GetChildByIndex
//---------------------------------------------------------------------------


CWnd *CLayoutWnd::GetChildByIndex(int index)
{
    return m_layout.GetChildByIndex(index);
}


//---------------------------------------------------------------------------
// CLayoutWnd::OnSize
//---------------------------------------------------------------------------


void CLayoutWnd::OnSize(UINT nType, int cx, int cy)
{
    if (nType == SIZE_MAXIMIZED || nType == SIZE_RESTORED)
        m_layout.Calculate();
    CWnd::OnSize(nType, cx, cy);
}


//---------------------------------------------------------------------------
// CLayoutWnd::OnSizing
//---------------------------------------------------------------------------


void CLayoutWnd::OnSizing(UINT nSide, LPRECT lpRect)
{
    m_layout.Calculate();
    CWnd::OnSizing(nSide, lpRect);
}


//---------------------------------------------------------------------------
// CLayoutWnd::OnDestroy
//---------------------------------------------------------------------------


void CLayoutWnd::OnDestroy()
{
    m_layout.DeleteChildren();
    CWnd::OnDestroy();
}


//---------------------------------------------------------------------------
// CLayoutWnd::OnParentNotify
//---------------------------------------------------------------------------


void CLayoutWnd::OnParentNotify(UINT message, LPARAM lParam)
{
    if (message == WM_DESTROY)
        m_layout.DeleteChild(CWnd::FromHandle((HWND)lParam));
    CWnd::OnParentNotify(message, lParam);
}


//---------------------------------------------------------------------------
// CLayoutPropertyPage Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CLayoutPropertyPage, CPropertyPage)

    ON_WM_ERASEBKGND()
    ON_WM_CTLCOLOR()

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// CLayoutPropertyPage Constructor
//---------------------------------------------------------------------------


#pragma warning(push)
#pragma warning(disable: 4355)

CLayoutPropertyPage::CLayoutPropertyPage(int page_num)
    : m_layout(this), m_page_num(page_num)
{
}

#pragma warning(pop)


//---------------------------------------------------------------------------
// CLayoutPropertyPage::InitPage
//---------------------------------------------------------------------------


void CLayoutPropertyPage::InitPage(
    ULONG exstyle, const CSize &pct_size, const CSize &size_min,
    const CSize &size_max, HBRUSH hbrush)
{
    //
    // pct_x and pct_y specify the desired size of the dialog box as
    // a percent of the desktop screen size
    //

    CRect rc;
    GetDesktopWindow()->GetWindowRect(&rc);
    int cx = (int)floorf(pct_size.cx * rc.Width()  / 100.0f + 0.5f);
    int cy = (int)floorf(pct_size.cy * rc.Height() / 100.0f + 0.5f);

    if (size_min.cx && cx < size_min.cx)
        cx = size_min.cx;
    if (size_min.cy && cy < size_min.cy)
        cy = size_min.cy;
    if (size_max.cx && cx > size_max.cx)
        cx = size_max.cx;
    if (size_max.cy && cy > size_max.cy)
        cy = size_max.cy;

    return InitPage(exstyle, CSize(cx, cy), hbrush);
}


//---------------------------------------------------------------------------
// CLayoutPropertyPage::InitPage
//---------------------------------------------------------------------------


void CLayoutPropertyPage::InitPage(
    ULONG exstyle, const CSize &abs_size, HBRUSH hbrush)
{
    //
    // we need to convert screen units to dialog units, which are the
    // average character size in the font.  the font is the system default
    // dialog font because we don't override that in the dialog template
    //

    UINT cxFont = 8, cyFont = 16;
    CFont font;
    if (font.CreateStockObject(DEFAULT_GUI_FONT)) {

        static const WCHAR *_alphabet =
            L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

        HDC hdc = ::GetDC(NULL);
        HFONT oldfont = (HFONT)SelectObject(hdc, font);

        SIZE size;
        TEXTMETRIC tm;
        GetTextMetrics(hdc, &tm);
        cyFont = tm.tmHeight + tm.tmExternalLeading;
        ::GetTextExtentPoint32(hdc, _alphabet, 52, &size);
        cxFont = (size.cx + 26) / 52;

        ::SelectObject(hdc, oldfont);
        ::ReleaseDC(NULL, hdc);
    }

    //
    // construct a trivial and empty dialog box template
    //

    typedef struct {
        DWORD style;
        DWORD dwExtendedStyle;
        WORD cdit;
        short x;
        short y;
        short cx;
        short cy;
        WORD id;
    } DLGTMPL1;

    memzero(&m_dlgtmpl, sizeof(m_dlgtmpl));
    DLGTMPL1 *tmpl = (DLGTMPL1 *)m_dlgtmpl;

    tmpl->style = WS_CHILD | WS_DISABLED | WS_CAPTION;
    tmpl->dwExtendedStyle = exstyle;
    tmpl->cdit = 0;
    tmpl->cx = (short)(abs_size.cx * 4 / cxFont);
    tmpl->cy = (short)(abs_size.cy * 8 / cyFont);

    m_psp.pResource = (LPCDLGTEMPLATE)tmpl;
    m_psp.dwFlags |= PSP_DLGINDIRECT;

    //
    // set background brush
    //

    m_hbrush = hbrush;
}


//---------------------------------------------------------------------------
// CLayoutPropertyPage::CreateChild
//---------------------------------------------------------------------------


CWnd *CLayoutPropertyPage::CreateChild(
    int ctrlid, const WCHAR *clsnm, ULONG exstyle, ULONG style,
    const CPoint &pos, const CSize &size)
{
    CWnd *wnd = m_layout.CreateChild(
                                ctrlid, clsnm, exstyle, style, pos, size);
    wnd->SetFont(GetFont());
    return wnd;
}


//---------------------------------------------------------------------------
// CLayoutPropertyPage::OnEraseBkgnd
//---------------------------------------------------------------------------


BOOL CLayoutPropertyPage::OnEraseBkgnd(CDC *pDC)
{
    if (! m_hbrush)
        return CPropertyPage::OnEraseBkgnd(pDC);

    CRect rc;
    GetClientRect(&rc);
    pDC->SelectObject(m_hbrush);
    pDC->PatBlt(0, 0, rc.Width(), rc.Height(), PATCOPY);
    return TRUE;
}


//---------------------------------------------------------------------------
// CLayoutPropertyPage::OnCtlColor
//---------------------------------------------------------------------------


HBRUSH CLayoutPropertyPage::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);

    if (m_hbrush && nCtlColor == CTLCOLOR_STATIC) {

        bool MakeTransparent = true;
        WCHAR clsnm[130];
        if (GetClassName(pWnd->m_hWnd, clsnm, 128 * sizeof(WCHAR)) &&
                _wcsicmp(clsnm, L"Edit") == 0) {
            //
            // WM_CTLCOLORSTATIC (nCtlColor == CTLCOLOR_STATIC)
            // is also received by read-only/disabled edit controls,
            // but we don't want to change those
            //
            MakeTransparent = false;
        }

        if (MakeTransparent) {

            hbr = (HBRUSH)::GetStockObject(NULL_BRUSH);

            pDC->SetBkMode(TRANSPARENT);

            COLORREF rgb = 0;
            CString text;
            pWnd->GetWindowText(text);
            if (text.Left(7).Compare(L"http://") == 0)
                rgb = RGB(0,0,255);
            pDC->SetTextColor(rgb);
        }
    }

    return hbr;
}


//---------------------------------------------------------------------------
// CLayoutPropertyPage::OnInitDialog
//---------------------------------------------------------------------------


BOOL CLayoutPropertyPage::OnInitDialog()
{
    m_layout.Calculate();
    return TRUE;
}


//---------------------------------------------------------------------------
// CLayoutPropertyPage::OnSetActive
//---------------------------------------------------------------------------


BOOL CLayoutPropertyPage::OnSetActive()
{
    SetWindowPos(NULL, 0, 0, 0, 0,  // fixes a problem with WS_EX_LAYOUTRTL
                 SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

    CPropertySheet *sheet = (CPropertySheet *)GetParent();

    ULONG flags = 0;
    if (m_page_num > 1)
        flags |= PSWIZB_BACK;
    if (m_page_num == sheet->GetPageCount())
        flags |= PSWIZB_FINISH;
    else
        flags |= PSWIZB_NEXT;

    sheet->SetWizardButtons(flags);

    return CPropertyPage::OnSetActive();
}
