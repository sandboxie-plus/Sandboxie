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
// Process List Ctrl
//---------------------------------------------------------------------------


#include "stdafx.h"
#include "MyApp.h"
#include "ProcListCtrl.h"

#include "Boxes.h"
#include "UserSettings.h"
#include "ProcSettingsDialog.h"
#include "BoxPage.h"
#include "apps/common/BoxOrder.h"


//---------------------------------------------------------------------------
// Strings
//---------------------------------------------------------------------------


static const WCHAR *_Column0      = L"ColWidthProcName";
static const WCHAR *_Column1      = L"ColWidthProcId";
static const WCHAR *_Column2      = L"ColWidthProcTitle";
static const WCHAR *_ColumnWidths = L"ProcessViewColumnWidths";


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CProcListCtrl, CMyListCtrl)

    ON_COMMAND(ID_PROCESS_TERMINATE,            OnCmdTerminate)
    ON_COMMAND(ID_PROCESS_SETTINGS,             OnCmdSettings)
    ON_COMMAND(ID_PROCESS_RESOURCES,            OnCmdResources)

    ON_WM_KEYDOWN()
    ON_WM_DESTROY()

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// Create
//---------------------------------------------------------------------------


BOOL CProcListCtrl::Create(CWnd *pParentWnd)
{
    m_pid = 0;
    m_box_order = NULL;
    m_box_order_selected = NULL;

    CMyListCtrl::Create(
        pParentWnd, WS_CLIPCHILDREN | LVS_SINGLESEL, L"PROCESS");

    //
    // set column widths
    //

    int width0, width1, width2;

    CStringList widths;
    CUserSettings::GetInstance().GetTextCsv(_ColumnWidths, widths);
    if (widths.GetCount() >= 3) {

        width0 = _wtoi(widths.RemoveHead());
        width1 = _wtoi(widths.RemoveHead());
        width2 = _wtoi(widths.RemoveHead());

        if (! width0)
            width0 = 250;
        if (! width1)
            width1 = 70;
        if (! width2)
            width1 = 300;

    } else {

        CUserSettings::GetInstance().GetNum(_Column0, width0, 250);
        CUserSettings::GetInstance().GetNum(_Column1, width1, 70);
        CUserSettings::GetInstance().GetNum(_Column2, width2, 300);
    }

    CListCtrl::InsertColumn(0, CMyMsg(MSG_3517), LVCFMT_LEFT, width0, 0);
    CListCtrl::InsertColumn(1, CMyMsg(MSG_3518), LVCFMT_LEFT, width1, 0);
    CListCtrl::InsertColumn(2, CMyMsg(MSG_3519), LVCFMT_LEFT, width2, 0);

    //
    //
    //

    CMyListCtrl::CreateComboButton();

    RebuildCombo();

    //
    //
    //

    return TRUE;
}


//---------------------------------------------------------------------------
// OnDestory
//---------------------------------------------------------------------------


void CProcListCtrl::OnDestroy()
{
    int width0 = GetColumnWidth(0);
    int width1 = GetColumnWidth(1);
    int width2 = GetColumnWidth(2);

    CString widths;
    widths.Format(L"%d,%d,%d", width0, width1, width2);
    BOOL ok = CUserSettings::GetInstance().SetText(_ColumnWidths, widths);

    if (ok) {
        CUserSettings::GetInstance().SetText(_Column0, CString());
        CUserSettings::GetInstance().SetText(_Column1, CString());
        CUserSettings::GetInstance().SetText(_Column2, CString());
    }
}


//---------------------------------------------------------------------------
// OnSize
//---------------------------------------------------------------------------


void CProcListCtrl::OnSize(UINT nType, int cx, int cy)
{
    if (nType != SIZE_MINIMIZED) {

        CMyListCtrl::ResizeComboButton();
    }
}


//---------------------------------------------------------------------------
// OnRefreshPosted
//---------------------------------------------------------------------------


void CProcListCtrl::OnRefreshPosted()
{
    SetRedraw(FALSE);
    RefreshContent();
    SetRedraw(TRUE);
}


//---------------------------------------------------------------------------
// RefreshContent
//---------------------------------------------------------------------------


void CProcListCtrl::RefreshContent()
{
    int ListIndex = 0;

    BOX_ORDER_ENTRY *box_order = (BOX_ORDER_ENTRY *)m_box_order_selected;
    if (! box_order)
        box_order = (BOX_ORDER_ENTRY *)m_box_order;
    if (box_order)
        box_order = box_order->children;

    CStringList OrderedBoxList;
    CMyListCtrl::GetOrderedBoxList(OrderedBoxList, box_order);

    CBoxes &boxes = CBoxes::GetInstance();

    while (! OrderedBoxList.IsEmpty()) {

        CString BoxName = OrderedBoxList.RemoveHead();
        CBox &box = boxes.GetBox(BoxName);

        int IconIndex;
        if (box.GetBoxProc().GetProcessCount() == 0)
            IconIndex = m_imgEmpty;
        else if (box.IsExpandedView())
            IconIndex = m_imgMinus;
        else
            IconIndex = m_imgPlus;

        ListIndex = InsertBoxRow(ListIndex, box.GetName(), IconIndex);
        if (IconIndex == m_imgMinus)
            ListIndex = RefreshOneBox(box.GetBoxProc(), ListIndex);
    }

    //
    // delete any left over items
    //

    for (int i = CListCtrl::GetItemCount() - 1; i >= ListIndex; --i)
        CMyListCtrl::MyDeleteItem(i);

    RebuildImageList();
}


//---------------------------------------------------------------------------
// RefreshOneBox
//---------------------------------------------------------------------------


int CProcListCtrl::RefreshOneBox(CBoxProc &boxproc, int ListIndex)
{
    int ProcIndex = -1;
    bool have1 = true;
    bool have2 = true;

    LVITEM lvi;
    memzero(&lvi, sizeof(LVITEM));

    while (1) {

        if (have1) {
            ++ProcIndex;
            if (ProcIndex >= boxproc.GetProcessCount())
                have1 = false;
        }

        if (have2) {
            lvi.mask = LVIF_IMAGE | LVIF_PARAM;
            lvi.iItem = ListIndex;
            if (! CListCtrl::GetItem(&lvi))
                have2 = false;
            else if (lvi.lParam >= 0)
                have2 = false;
        }

        if (have1) {

            int pid1 = boxproc.GetProcessId(ProcIndex);
            const CString &image1 = boxproc.GetProcessImageName(ProcIndex);
            const CString &title1 = boxproc.GetProcessWindowTitle(ProcIndex);

            if (have2) {

                int pid2 = -(int)lvi.lParam;
                CString &image2 = CListCtrl::GetItemText(ListIndex, 0);
                CString &title2 = CListCtrl::GetItemText(ListIndex, 2);
                if (pid1 == pid2 && image1 == image2 && title1 == title2) {

                    ++ListIndex;

                } else {

                    CMyListCtrl::MyDeleteItem(ListIndex);
                    --ProcIndex;
                }

            } else {

                int icon1 = CMyListCtrl::AddToImageList(
                                boxproc.GetProcessWindowIcon(ProcIndex));
                ListIndex =
                    InsertProcessRow(ListIndex, pid1, image1, title1, icon1);
            }

        } else {

            if (have2) {

                lvi.mask = LVIF_PARAM;
                lvi.iItem = ListIndex;
                while (1) {
                    if (! CMyListCtrl::MyDeleteItem(ListIndex))
                        break;
                    if (! CListCtrl::GetItem(&lvi))
                        break;
                    if (lvi.lParam >= 0)
                        break;
                }
            }

            break;
        }
    }

    return ListIndex;
}


//---------------------------------------------------------------------------
// InsertProcessRow
//---------------------------------------------------------------------------


int CProcListCtrl::InsertProcessRow(
    int ListIndex, int ProcessId, const CString &ImageName,
    const CString &WindowTitle, int IconIndex)
{
    LVITEM lvi;
    memzero(&lvi, sizeof(LVITEM));
    lvi.iItem = ListIndex;

    lvi.mask = LVIF_INDENT | LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM;
    lvi.iIndent = 1;
    lvi.iImage = IconIndex;
    lvi.pszText = (WCHAR *)(const WCHAR *)ImageName;
    lvi.lParam = -ProcessId;
    ListIndex = CListCtrl::InsertItem(&lvi);

    WCHAR pidtext[32];
    wsprintf(pidtext, L"%d", ProcessId);
    lvi.iItem = ListIndex;
    lvi.iSubItem = 1;
    lvi.mask = LVIF_TEXT;
    lvi.pszText = pidtext;
    CListCtrl::SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = (WCHAR *)(const WCHAR *)WindowTitle;
    CListCtrl::SetItem(&lvi);

    return ListIndex + 1;
}


//---------------------------------------------------------------------------
// InsertBoxRow
//---------------------------------------------------------------------------


int CProcListCtrl::InsertBoxRow(
    int ListIndex, const CString &BoxName, int IconIndex)
{
    bool DoSelect = false;

    LVITEM lvi;
    memzero(&lvi, sizeof(LVITEM));
    lvi.iItem = ListIndex;
    lvi.mask = LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
    lvi.stateMask = LVIS_SELECTED;
    while (1) {
        if (! CListCtrl::GetItem(&lvi))
            break;
        if (lvi.lParam > 0) {
            CString *pBoxName = (CString *)lvi.lParam;
            if (BoxName == *pBoxName && lvi.iImage == IconIndex) {

                return ListIndex + 1;

            } else {

                if ((! DoSelect) && (lvi.state & LVIS_SELECTED))
                    DoSelect = true;
            }
        } else
            lvi.lParam = 0;
        CMyListCtrl::MyDeleteItem(ListIndex);
    }

    //
    //
    //

    WCHAR *BoxNameText = SbieDll_FormatMessage1(MSG_3515, BoxName);

    lvi.mask = LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM;
    lvi.iImage = IconIndex;
    lvi.pszText = BoxNameText;
    lvi.lParam = (LPARAM)(new CString(BoxName));
    ListIndex = CListCtrl::InsertItem(&lvi);

    LocalFree(BoxNameText);

    if (IconIndex != m_imgEmpty) {

        WCHAR *ActiveText = SbieDll_FormatMessage0(MSG_3516);

        lvi.iItem = ListIndex;
        lvi.iSubItem = 1;
        lvi.mask = LVIF_TEXT;
        lvi.pszText = ActiveText;
        CListCtrl::SetItem(&lvi);

        LocalFree(ActiveText);
    }

    if (DoSelect)
        CMyListCtrl::SelectIndex(ListIndex);

    return ListIndex + 1;
}


//---------------------------------------------------------------------------
// SelectByProcessId
//---------------------------------------------------------------------------


void CProcListCtrl::SelectByProcessId(int pid)
{
    CBoxes &boxes = CBoxes::GetInstance();
    CBox &box = boxes.GetBoxByProcessId(pid);
    box.SetExpandedView(TRUE);
    boxes.WriteExpandedView();

    LVFINDINFO lvfi;
    memzero(&lvfi, sizeof(LVFINDINFO));
    lvfi.flags = LVFI_PARAM;
    lvfi.lParam = -pid;
    int ListIndex = CListCtrl::FindItem(&lvfi);
    if (ListIndex != -1)
        CMyListCtrl::SelectIndex(ListIndex);

    SetFocus();
    RefreshContent();
}


//---------------------------------------------------------------------------
// GetSelectedBox
//---------------------------------------------------------------------------


CBox &CProcListCtrl::GetSelectedBox(CWnd *pWnd, CPoint pt) const
{
    int SelectIndex = GetNextItem(-1, LVIS_SELECTED);
    if (SelectIndex != -1) {
        LPARAM lParam = GetItemData(SelectIndex);
        if (lParam > 0) {
            CString *BoxName = (CString *)lParam;
            return CBoxes::GetInstance().GetBox(*BoxName);
        }
    }

    return CBoxes::GetInstance().GetBox(0);
}


//---------------------------------------------------------------------------
// GetRightClickPid
//---------------------------------------------------------------------------


int CProcListCtrl::GetRightClickPid() const
{
    return m_pid;
}


//---------------------------------------------------------------------------
// OnContextMenu
//---------------------------------------------------------------------------


void CProcListCtrl::OnContextMenu(CWnd *pWnd, CPoint pt)
{
    if (pt.x == -1 && pt.y == -1)
        GetSelectedItemPosition(pt);
    ScreenToClient(&pt);
    int index = HitTest(pt, NULL);
    if (index == -1)
        return;
    LPARAM lParam = GetItemData(index);
    if (lParam >= 0)
        return;

    m_pid = (int)-lParam;
    ClientToScreen(&pt);
    m_pContextMenu->GetSubMenu(0)->TrackPopupMenu(0, pt.x, pt.y, this, NULL);
}


//---------------------------------------------------------------------------
// OnCmdTerminate
//---------------------------------------------------------------------------


void CProcListCtrl::OnCmdTerminate()
{
    GetParent()->SendMessage(WM_COMMAND, ID_PROCESS_TERMINATE, 0);
}


//---------------------------------------------------------------------------
// OnCmdSettings
//---------------------------------------------------------------------------


void CProcListCtrl::OnCmdSettings()
{
    CProcSettingsDialog dlg(this, m_pid);
}


//---------------------------------------------------------------------------
// OnCmdResources
//---------------------------------------------------------------------------


void CProcListCtrl::OnCmdResources()
{
    CBox &box = CBoxes::GetInstance().GetBoxByProcessId(m_pid);
    const CString &boxName = box.GetName();
    if (! boxName.IsEmpty()) {
        CBoxProc &boxProc = box.GetBoxProc();
        const CString &pgm = boxProc.GetProcessImageName(
                                    boxProc.GetIndexForProcessId(m_pid));
        CBoxPage::DoPropertySheet(this, boxName, NULL, pgm);
    }
}


//---------------------------------------------------------------------------
// OnClick
//---------------------------------------------------------------------------


void CProcListCtrl::OnClick(NMHDR *pNMHDR, LRESULT *pResult)
{
    NMITEMACTIVATE *nm = (NMITEMACTIVATE *)pNMHDR;
    LPARAM lParam = 0;
    if (nm->iItem != -1) {
        CListCtrl::EnsureVisible(nm->iItem, FALSE);
        if (nm->iSubItem == 0)
            lParam = GetItemData(nm->iItem);
    }

    if (lParam > 0) {

        if (nm->hdr.code == NM_DBLCLK ||
            GetScrollPos(SB_HORZ) + nm->ptAction.x < 32) {

            CString *BoxName = (CString *)lParam;

            if (nm->uKeyFlags & LVKF_ALT) {

                int index = CBoxes::GetInstance().GetBoxIndex(*BoxName);
                if (index) {

                    index *= ID_SANDBOX_MENU_SIZE;
                    index += ID_SANDBOX_SETTINGS;
                    index += ID_SANDBOX_MENU;
                    GetParent()->SendMessage(WM_COMMAND, index, 0);
                }

            } else {

                CBoxes &boxes = CBoxes::GetInstance();
                CBox &box = boxes.GetBox(*BoxName);
                if (box.IsExpandedView())
                    box.SetExpandedView(FALSE);
                else
                    box.SetExpandedView(TRUE);
                boxes.WriteExpandedView();

                CMyListCtrl::PostRefresh();
            }
        }

    } else if (lParam < 0) {

        if (nm->hdr.code == NM_DBLCLK ||
            GetScrollPos(SB_HORZ) + nm->ptAction.x < 32) {

            if (nm->uKeyFlags & LVKF_ALT) {

                m_pid = (int)-lParam;
                OnCmdSettings();
            }
        }
    }
}


//---------------------------------------------------------------------------
// OnKeyDown
//---------------------------------------------------------------------------


void CProcListCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    BOOL ctrl = ((GetKeyState(VK_CONTROL) & 0x8000) == 0x8000);

    if ((nChar == VK_LEFT || nChar == VK_RIGHT) && (! ctrl))
        OnArrowKeyDown(nChar);

    else if (nChar == VK_TAB)
        m_combo.SetFocus();

    else
        CMyListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}


//---------------------------------------------------------------------------
// OnArrowKeyDown
//---------------------------------------------------------------------------


void CProcListCtrl::OnArrowKeyDown(UINT nChar)
{
    int index = CListCtrl::GetNextItem(-1, LVNI_FOCUSED);
    if (index == -1)
        return;

    LPARAM lParam = GetItemData(index);
    if (lParam <= 0)
        return;
    CString *BoxName = (CString *)lParam;

    CBoxes &boxes = CBoxes::GetInstance();
    CBox &box = boxes.GetBox(*BoxName);

    BOOL new_expand = (nChar == VK_RIGHT) ? TRUE : FALSE;
    BOOL old_expand = box.IsExpandedView();
    if (new_expand == old_expand)
        return;

    box.SetExpandedView(new_expand);
    boxes.WriteExpandedView();

    CMyListCtrl::PostRefresh();
}


//---------------------------------------------------------------------------
// RebuildCombo
//---------------------------------------------------------------------------


void CProcListCtrl::RebuildCombo()
{
    CString selection;
    int index = m_combo.GetCurSel();
    if (index != LB_ERR)
        m_combo.GetLBText(index, selection);

    m_combo.ResetContent();
    if (m_box_order)
        BoxOrder_Free((BOX_ORDER_ENTRY *)m_box_order);

    m_box_order = BoxOrder_Read();
    RebuildCombo2(m_box_order, CString());

    int count = m_combo.GetCount();
    BOOL enabled = (count > 1) ? TRUE : FALSE;
    m_combo.EnableWindow(enabled);
    if (enabled)
        CMyListCtrl::ResizeComboButton();
    m_combo.ShowWindow(enabled ? SW_SHOW : SW_HIDE);

    if (enabled && selection.GetLength()) {
        for (index = 0; index < count; ++index) {
            CString text;
            m_combo.GetLBText(index, text);
            if (text.CompareNoCase(selection) == 0)
                break;
        }
        if (index == count)
            index = 0;
    } else
        index = 0;

    m_combo.SetCurSel(index);
    m_box_order_selected = m_combo.GetItemDataPtr(index);
    if (m_box_order_selected == (void *)-1)
        m_box_order_selected = NULL;
}


//---------------------------------------------------------------------------
// RebuildCombo2
//---------------------------------------------------------------------------


void CProcListCtrl::RebuildCombo2(void *_order_entry, const CString &prefix)
{
    BOX_ORDER_ENTRY *order_entry = (BOX_ORDER_ENTRY *)_order_entry;
    while (order_entry) {
        if (order_entry->children) {

            CString name, prefix2;
            if (prefix.GetLength() == 0 && m_combo.GetCount() == 0) {
                name = CMyMsg(MSG_5143);
                prefix2 = CString();
            } else {
                name = prefix + order_entry->name;
                prefix2 = name + L" > ";
            }

            int i = m_combo.AddString(name);
            m_combo.SetItemDataPtr(i, order_entry);
            RebuildCombo2(order_entry->children, prefix2);
        }

        order_entry = order_entry->next;
    }
}


//---------------------------------------------------------------------------
// OnComboSelect
//---------------------------------------------------------------------------


void CProcListCtrl::OnComboSelect()
{
    void *box_order = m_box_order;
    int index = m_combo.GetCurSel();
    if (index != LB_ERR) {
        box_order = m_combo.GetItemDataPtr(index);
        if (m_box_order == (void *)-1)
            m_box_order = NULL;
    }
    m_box_order_selected = box_order;

    this->SetFocus();

    CMyListCtrl::PostRefresh(-1);
}
