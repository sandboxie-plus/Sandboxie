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


#include "stdafx.h"
#include "MyApp.h"
#include "MyListCtrl.h"

#include "UserSettings.h"
#include "apps/common/MyGdi.h"
#include "apps/common/BoxOrder.h"


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CMyListCtrl, CListCtrl)

    ON_COMMAND(ID_SHOW_WINDOW,                  OnRefreshPosted)

    ON_NOTIFY_REFLECT(NM_CLICK,                 OnClick)
    ON_NOTIFY_REFLECT(NM_DBLCLK,                OnClick)

    ON_CONTROL(CBN_SELENDOK, ID_GROUP_COMBO,    OnComboSelect)
    ON_CONTROL(CBN_CLOSEUP,  ID_GROUP_COMBO,    OnComboSelect)

    ON_WM_SIZE()

    ON_MENUXP_MESSAGES()

END_MESSAGE_MAP()


IMPLEMENT_MENUXP(CMyListCtrl, CListCtrl)


//---------------------------------------------------------------------------
// Create
//---------------------------------------------------------------------------


BOOL CMyListCtrl::Create(CWnd *pParentWnd, int style, const WCHAR *prefix)
{
    //
    // create list view control
    //

    if (pParentWnd && CMyApp::m_LayoutRTL)
        pParentWnd->ModifyStyleEx(0, WS_EX_NOINHERITLAYOUT);

    style |= LVS_REPORT | WS_VSCROLL | WS_HSCROLL | WS_CHILD | WS_TABSTOP;
    BOOL ok = CListCtrl::Create(style, CRect(), pParentWnd, 0);

    if (pParentWnd && CMyApp::m_LayoutRTL)
        pParentWnd->ModifyStyleEx(WS_EX_NOINHERITLAYOUT, 0);

    if (! ok)
        return FALSE;

    CListCtrl::SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);

    //
    // set appearance for list view control
    //

    CListCtrl::SetTextColor(RGB(0,0,0));
    CListCtrl::SetTextBkColor(CLR_NONE);

    CListCtrl::SetBkImage(MyGdi_CreateFromResource(L"BACKGROUND"));

    //
    // initialize right-click menu
    //

    m_pContextMenu = CMyApp::MyLoadMenu(prefix + CString(L"_MENU"));

    //
    // initialize image list
    //

    InitImageList(prefix + CString(L"_ICON_"));

    return TRUE;
}


//---------------------------------------------------------------------------
// CreateComboButton
//---------------------------------------------------------------------------


void CMyListCtrl::CreateComboButton(void)
{
    m_combo.Create(CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
                   CRect(), this, ID_GROUP_COMBO);
    m_combo.SetFont(CListCtrl::GetFont());
    m_combo.SetExtendedUI(TRUE);

    m_combo.SetTabbingWindows(this, this);
}


//---------------------------------------------------------------------------
// ResizeComboButton
//---------------------------------------------------------------------------


void CMyListCtrl::ResizeComboButton(void)
{
    if (m_combo.m_hWnd && m_combo.IsWindowEnabled()) {

        RECT rc;
        GetWindowRect(&rc);
        int cx = rc.right - rc.left;
        int thumbw = GetSystemMetrics(SM_CXHTHUMB) * 2;
        if (cx > thumbw)
            cx -= thumbw;

        CDC *dc = GetDC();
        CSize sz = dc->GetTextExtent(L"Mp", 2);
        ReleaseDC(dc);
        int h = sz.cy * 4;
        int w = sz.cx * 12;

        int x = cx - w;
        if (cx < w + 100)
            x = w = cx / 2;

        int y = 20;
        CWnd *pHeader = CListCtrl::GetHeaderCtrl();
        if (pHeader) {
            pHeader->GetClientRect(&rc);
            y = rc.bottom;
        }
        y += 5;

        m_combo.SetWindowPos(&wndTop, x, y, w, h, SWP_NOACTIVATE);
    }
}


//---------------------------------------------------------------------------
// InitImageList
//---------------------------------------------------------------------------


void CMyListCtrl::InitImageList(const CString &prefix)
{
    //
    // create image list
    //

    int cxIndex = SM_CXICON;
    int cyIndex = SM_CYICON;
    BOOL SmallIcons;
    CUserSettings::GetInstance().GetBool(L"SmallIcons", SmallIcons);
    if (SmallIcons) {
        cxIndex = SM_CXSMICON;
        cyIndex = SM_CYSMICON;
    }
    int cx = GetSystemMetrics(cxIndex);
    int cy = GetSystemMetrics(cyIndex);
    if (cx > 34)
        cx = 34;
    if (cy > 34)
        cy = 34;

    CImageList imglist;
    imglist.Create(cx, cy, ILC_COLORDDB | ILC_MASK, 8, 8);
    imglist.SetBkColor(CLR_NONE);

    CListCtrl::SetImageList(&imglist, LVSIL_SMALL);

    imglist.Detach();

    //
    // load standard icons
    //

    HICON iconEmpty = (HICON)LoadImage(
        AfxGetInstanceHandle(), prefix + L"EMPTY", IMAGE_ICON, 0, 0, 0);

    HICON iconMinus = (HICON)LoadImage(
        AfxGetInstanceHandle(), prefix + L"MINUS", IMAGE_ICON, 0, 0, 0);

    HICON iconPlus = (HICON)LoadImage(
        AfxGetInstanceHandle(), prefix + L"PLUS",  IMAGE_ICON, 0, 0, 0);

    m_imgEmpty = AddToImageList(iconEmpty);
    m_imgMinus = AddToImageList(iconMinus);
    m_imgPlus  = AddToImageList(iconPlus);

    //
    // initialize refresh timer
    //

    m_LastImageTicks = GetTickCount();
    m_LastImageCount = CListCtrl::GetImageList(LVSIL_SMALL)->GetImageCount();
}


//---------------------------------------------------------------------------
// AddToImageList
//---------------------------------------------------------------------------


int CMyListCtrl::AddToImageList(HICON icon)
{
    int imglist_width, imglist_height;
    CImageList *imglist = CListCtrl::GetImageList(LVSIL_SMALL);
    if (ImageList_GetIconSize(
            imglist->m_hImageList, &imglist_width, &imglist_height)) {

        ICONINFO icon_info;
        if (GetIconInfo(icon, &icon_info)) {

            CBitmap bmp_clr, bmp_msk;
            bmp_clr.Attach(icon_info.hbmColor);
            bmp_msk.Attach(icon_info.hbmMask);

            bool ok_clr = AddToImageList2(
                            &bmp_clr, imglist_width, imglist_height, false);
            bool ok_msk = AddToImageList2(
                            &bmp_msk, imglist_width, imglist_height, true);

            if (ok_clr || ok_msk) {

                icon_info.hbmColor = (HBITMAP)bmp_clr.m_hObject;
                icon_info.hbmMask  = (HBITMAP)bmp_msk.m_hObject;
                icon = ::CreateIconIndirect(&icon_info);
            }
        }
    }

    return imglist->Add(icon);
}


//---------------------------------------------------------------------------
// AddToImageList2
//---------------------------------------------------------------------------


bool CMyListCtrl::AddToImageList2(CBitmap *bmp, int w, int h, bool mask)
{
    bool replaced = false;

    BITMAP info;
    if (bmp->GetBitmap(&info) &&
                (info.bmWidth < w || info.bmHeight < h)) {

        CDC *dc = GetDC();

        CDC dc1;
        dc1.CreateCompatibleDC(NULL);
        CBitmap *old_bmp_1 = dc1.SelectObject(bmp);

        CDC dc2;
        dc2.CreateCompatibleDC(NULL);
        CBitmap bmp2;
        bmp2.CreateCompatibleBitmap(mask ? &dc2 : &dc1, w, h);
        CBitmap *old_bmp_2 = dc2.SelectObject(&bmp2);

        dc2.FillSolidRect(0, 0, w, h, RGB(255,255,255));
        int x = (w - info.bmWidth) / 2;
        int y = (h - info.bmHeight) / 2;
        dc2.BitBlt(x, y, info.bmWidth, info.bmHeight, &dc1, 0, 0, SRCCOPY);

        dc2.SelectObject(old_bmp_2);
        dc1.SelectObject(old_bmp_1);

        bmp->DeleteObject();
        bmp->Attach(bmp2.Detach());

        replaced = true;
    }

    return replaced;
}


//---------------------------------------------------------------------------
// MyDeleteItem
//---------------------------------------------------------------------------


BOOL CMyListCtrl::MyDeleteItem(int ListIndex)
{
    LONG_PTR DataPtr = CListCtrl::GetItemData(ListIndex);

    if (! CListCtrl::DeleteItem(ListIndex))
        return FALSE;

    if (DataPtr > 0)
        delete (CString *)DataPtr;

    return TRUE;
}


//---------------------------------------------------------------------------
// RebuildImageList
//---------------------------------------------------------------------------


void CMyListCtrl::RebuildImageList()
{
    ULONG TicksNow = GetTickCount();
    if (TicksNow - m_LastImageTicks < 5 * 1000)
        return;
    m_LastImageTicks = TicksNow;

    CImageList *ImageList = CListCtrl::GetImageList(LVSIL_SMALL);

    int ImageCount = ImageList->GetImageCount();
    if (ImageCount == m_LastImageCount)
        return;

    //
    //
    //

    int ListCount = CListCtrl::GetItemCount();
    int ListIndex;

    CPtrArray Icons;
    Icons.SetSize(ListCount);

    LVITEM lvi;
    memzero(&lvi, sizeof(LVITEM));
    lvi.mask = LVIF_IMAGE;

    for (ListIndex = 0; ListIndex < ListCount; ++ListIndex) {
        lvi.iItem = ListIndex;
        if (CListCtrl::GetItem(&lvi) && lvi.iImage != m_imgEmpty &&
                lvi.iImage != m_imgMinus && lvi.iImage != m_imgPlus) {

            HICON hIcon = ImageList->ExtractIcon(lvi.iImage);
            Icons.SetAt(ListIndex, hIcon);
        }
    }

    //
    //
    //

    SetRedraw(FALSE);

    int ImageIndex;
    for (ImageIndex = ImageCount - 1; ImageIndex >= 0; --ImageIndex) {
        if (ImageIndex != m_imgEmpty &&
                ImageIndex != m_imgMinus && ImageIndex != m_imgPlus) {
            ImageList->Remove(ImageIndex);
        }
    }

    //
    //
    //

    for (ListIndex = 0; ListIndex < ListCount; ++ListIndex) {

        HICON hIcon = (HICON)Icons.GetAt(ListIndex);
        if (hIcon) {
            lvi.iItem = ListIndex;
            lvi.iImage = ImageList->Add(hIcon);
            CListCtrl::SetItem(&lvi);
            DestroyIcon(hIcon);
        }
    }

    m_LastImageCount = ImageList->GetImageCount();

    SetRedraw(TRUE);
}


//---------------------------------------------------------------------------
// SelectIndex
//---------------------------------------------------------------------------


void CMyListCtrl::SelectIndex(int ListIndex)
{
    if (ListIndex != -1)
        CListCtrl::EnsureVisible(ListIndex, FALSE);
    CListCtrl::SetItemState(ListIndex, LVIS_SELECTED | LVIS_FOCUSED,
                                       LVIS_SELECTED | LVIS_FOCUSED);
}


//---------------------------------------------------------------------------
// PostRefresh
//---------------------------------------------------------------------------


void CMyListCtrl::PostRefresh(LPARAM lParam)
{
    PostMessage(WM_COMMAND, ID_SHOW_WINDOW, lParam);
}


//---------------------------------------------------------------------------
// GetSelectedItemPosition
//---------------------------------------------------------------------------


void CMyListCtrl::GetSelectedItemPosition(CPoint &pt) const
{
    BOOL ok = FALSE;
    int index = GetNextItem(-1, LVIS_SELECTED);
    if (index != -1) {
        ok = GetItemPosition(index, &pt);
        if (ok) {
            ClientToScreen(&pt);
            pt.x += 50;
            pt.y += 20;
        }
    }
    if (! ok) {
        pt.x = 0;
        pt.y = 0;
    }
}


//---------------------------------------------------------------------------
// GetOrderedBoxList
//---------------------------------------------------------------------------


void CMyListCtrl::GetOrderedBoxList(CStringList &BoxList, void *BoxOrder)
{
    BOX_ORDER_ENTRY *order_entry, *orig_order_entry;
    if (BoxOrder) {
        order_entry = (BOX_ORDER_ENTRY *)BoxOrder;
        orig_order_entry = NULL;
    } else
        orig_order_entry = order_entry = BoxOrder_Read();

    while (order_entry) {

        if (order_entry->children)
            GetOrderedBoxList(BoxList, order_entry->children);
        else {

            bool Duplicate = false;

            POSITION pos = BoxList.GetHeadPosition();
            while (pos) {
                CString &OldName = BoxList.GetNext(pos);
                if (OldName.CompareNoCase(order_entry->name) == 0) {
                    Duplicate = true;
                    break;
                }
            }

            if (! Duplicate)
                BoxList.AddTail(order_entry->name);
        }

        order_entry = order_entry->next;
    }

    if (orig_order_entry)
        BoxOrder_Free(orig_order_entry);
}


//---------------------------------------------------------------------------
// ShowComboBox
//---------------------------------------------------------------------------


void CMyListCtrl::ShowComboBox(bool visible)
{
    if (m_combo.m_hWnd)
        m_combo.ShowWindow(visible ? SW_SHOW : SW_HIDE);
}
