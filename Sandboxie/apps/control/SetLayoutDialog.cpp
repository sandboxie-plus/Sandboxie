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


#include "stdafx.h"
#include "SetLayoutDialog.h"

#include "apps/common/BoxOrder.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define ID_HANDLES      10000
#define ID_RESTORE      30001
#define ID_INSERT       30002
#define ID_RENAME       30003
#define ID_DELETE       30004
#define ID_MOVE_UP      30005
#define ID_MOVE_DOWN    30006
#define ID_MOVE_TO      30007


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CSetLayoutDialog, CBaseDialog)

    ON_NOTIFY(NM_RCLICK,            ID_SETLAYOUT_TREE,  OnRightClick)
    ON_NOTIFY(TVN_BEGINLABELEDIT,   ID_SETLAYOUT_TREE,  OnLabelEdit)
    ON_NOTIFY(TVN_ENDLABELEDIT,     ID_SETLAYOUT_TREE,  OnLabelEdit)

    ON_COMMAND(ID_RESTORE,      OnRestore)
    ON_COMMAND(ID_INSERT,       OnInsert)
    ON_COMMAND(ID_RENAME,       OnRename)
    ON_COMMAND(ID_DELETE,       OnDelete)
    ON_COMMAND(ID_MOVE_TO,      OnMoveTo)
    ON_COMMAND(ID_MOVE_UP,      OnMoveUp)
    ON_COMMAND(ID_MOVE_DOWN,    OnMoveDown)

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


HHOOK CSetLayoutDialog::m_hhook = NULL;
bool  CSetLayoutDialog::m_inLabelEdit = false;


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CSetLayoutDialog::CSetLayoutDialog(CWnd *pParentWnd)
    : CBaseDialog(pParentWnd, L"SETLAYOUT_DIALOG")
{
    m_tree = NULL;

    m_hhook = SetWindowsHookEx(
        WH_KEYBOARD, MyKeyboardHook, NULL, GetCurrentThreadId());

    DoModal();

    if (m_hhook)
        UnhookWindowsHookEx(m_hhook);
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CSetLayoutDialog::~CSetLayoutDialog()
{
    if (m_tree)
        delete m_tree;
}


//---------------------------------------------------------------------------
// MyKeyboardHook
//---------------------------------------------------------------------------


LRESULT CSetLayoutDialog::MyKeyboardHook(
    int code, WPARAM wParam, LPARAM lParam)
{
    //
    // dialog box closes if Enter/Esc pressed, so intercept and discard
    // http://support.microsoft.com/kb/216664
    //

    if (code >= 0 && m_inLabelEdit &&
                        (wParam == VK_RETURN || wParam == VK_ESCAPE)) {

        HWND hwnd = ::GetFocus();
        if (hwnd) {
            WCHAR clsnm[64];
            if (GetClassName(hwnd, clsnm, 60 * sizeof(WCHAR))
                                        && _wcsicmp(clsnm, L"Button") != 0)
                return 1;
        }
    }

    return CallNextHookEx(m_hhook, code, wParam, lParam);
}


//---------------------------------------------------------------------------
// OnInitDialog
//---------------------------------------------------------------------------


BOOL CSetLayoutDialog::OnInitDialog()
{
    SetWindowText(CMyMsg(MSG_3435));

    GetDlgItem(ID_SETLAYOUT_EXPLAIN_1)->SetWindowText(CMyMsg(MSG_5142));

    GetDlgItem(IDOK)->SetWindowText(CMyMsg(MSG_3001));
    GetDlgItem(IDCANCEL)->SetWindowText(CMyMsg(MSG_3002));

    InitTree();

    m_tree->SetFocus();
    return FALSE;
}


//---------------------------------------------------------------------------
// InitTree
//---------------------------------------------------------------------------


void CSetLayoutDialog::InitTree()
{
    const ULONG tree_style = TVS_HASBUTTONS | TVS_FULLROWSELECT
                           | TVS_EDITLABELS
                           | WS_CHILD | WS_CLIPSIBLINGS
                           | WS_VISIBLE | WS_BORDER;

    RECT rc;
    GetDlgItem(ID_SETLAYOUT_TREE_PLACEHOLDER)->GetWindowRect(&rc);
    GetDlgItem(ID_SETLAYOUT_TREE_PLACEHOLDER)->ShowWindow(SW_HIDE);

    ScreenToClient(&rc);
    m_tree = new CTreeCtrl;
    m_tree->Create(tree_style, rc, this, ID_SETLAYOUT_TREE);

    m_tree->SetIndent(m_tree->GetIndent() * 3 / 2);

    HTREEITEM hRoot = m_tree->InsertItem(CMyMsg(MSG_5143));
    if (hRoot) {

        BOX_ORDER_ENTRY *box_order = BoxOrder_Read();
        if (box_order) {
            if (box_order->children)
                InitTree2(hRoot, box_order->children);
            BoxOrder_Free(box_order);
        }
    }
}


//---------------------------------------------------------------------------
// InitTree2
//---------------------------------------------------------------------------


void CSetLayoutDialog::InitTree2(HTREEITEM hParent, void *_order_entry)
{
    BOX_ORDER_ENTRY *order_entry = (BOX_ORDER_ENTRY *)_order_entry;
    while (order_entry) {

        HTREEITEM hNewItem = m_tree->InsertItem(order_entry->name, hParent);
        if (hNewItem && order_entry->children)
            InitTree2(hNewItem, order_entry->children);

        order_entry = order_entry->next;
    }

    m_tree->SetItemState(hParent, TVIS_BOLD, TVIS_BOLD);
    m_tree->Expand(hParent, TVE_EXPAND);
}


//---------------------------------------------------------------------------
// OnOK
//---------------------------------------------------------------------------


void CSetLayoutDialog::OnOK()
{
    BOX_ORDER_ENTRY *root = BoxOrder_Alloc(L"");
    if (root) {
        HTREEITEM hItem = m_tree->GetRootItem();
        ConvertToBoxOrder(hItem, root);
        BoxOrder_Write(root);
        BoxOrder_Free(root);
    }

    EndDialog(0);
}


//---------------------------------------------------------------------------
// ConvertToBoxOrder
//---------------------------------------------------------------------------


void CSetLayoutDialog::ConvertToBoxOrder(
    HTREEITEM hParent, void *_order_entry)
{
    BOX_ORDER_ENTRY *order_entry = (BOX_ORDER_ENTRY *)_order_entry;

    HTREEITEM hItem = m_tree->GetChildItem(hParent);
    while (hItem) {

        CString text = m_tree->GetItemText(hItem);
        BOX_ORDER_ENTRY *new_entry = BoxOrder_Alloc(text);
        if (new_entry) {
            BoxOrder_Append(order_entry, new_entry);

            if (m_tree->GetItemState(hItem, TVIS_BOLD) & TVIS_BOLD)
                ConvertToBoxOrder(hItem, new_entry);
        }

        hItem = m_tree->GetNextSiblingItem(hItem);
    }
}


//---------------------------------------------------------------------------
// OnRightClick
//---------------------------------------------------------------------------


void CSetLayoutDialog::OnRightClick(NMHDR *pNMHDR, LRESULT *plResult)
{
    if (pNMHDR->code != NM_RCLICK)
        return;

    //
    // get clicked item and its position
    //

    POINT pt;
    GetCursorPos(&pt);
    m_tree->ScreenToClient(&pt);
    HTREEITEM hItem = m_tree->HitTest(pt, NULL);
    if (! hItem)
        return;

    RECT rc;
    if (! m_tree->GetItemRect(hItem, &rc, TRUE))
        return;
    m_tree->ClientToScreen(&rc);

    m_tree->SelectItem(hItem);

    //
    // determine item type:  1=root, 2=group, 3=sandbox
    //

    int ItemType;
    if (! m_tree->GetParentItem(hItem))
        ItemType = 1;
    else if (m_tree->GetItemState(hItem, TVIS_BOLD) & TVIS_BOLD)
        ItemType = 2;
    else
        ItemType = 3;

    //
    // prepare context menu
    //

    CPtrArray handles;
    CMenu menu;
    menu.CreatePopupMenu();

    if (ItemType <= 2) {

        menu.AppendMenu(MF_STRING, ID_INSERT, CMyMsg(MSG_5146));
    }

    if (ItemType == 1) {

        menu.AppendMenu(MF_STRING, ID_RESTORE, CMyMsg(MSG_5145));
    }

    if (ItemType == 2) {

        CMenu menu2;
        menu2.CreatePopupMenu();
        CreateMoveMenu(&menu2, &handles);
        menu.AppendMenu(MF_STRING | MF_POPUP,
                       (INT_PTR)menu2.GetSafeHmenu(),
                       CMyMsg(MSG_5151));

        menu.AppendMenu(MF_STRING, ID_RENAME, CMyMsg(MSG_5147));
        menu.AppendMenu(MF_STRING, ID_DELETE, CMyMsg(MSG_5148));

    }

    if (ItemType == 3) {

        CreateMoveMenu(&menu, &handles);
    }

    int id = menu.TrackPopupMenu(
                    TPM_RETURNCMD, rc.left, rc.bottom, this, NULL);

    //
    // execute command
    //

    if (id >= ID_RESTORE && id <= ID_MOVE_DOWN) {

        PostMessage(WM_COMMAND, id, NULL);

    } else if (id >= ID_HANDLES) {

        id -= ID_HANDLES;
        if (id < handles.GetSize()) {

            PostMessage(WM_COMMAND, ID_MOVE_TO, (LPARAM)handles.GetAt(id));
        }
    }
}


//---------------------------------------------------------------------------
// CreateMoveMenu
//---------------------------------------------------------------------------


void CSetLayoutDialog::CreateMoveMenu(CMenu *menu, CPtrArray *handles)
{
    CMenu menu2;
    menu2.CreatePopupMenu();
    CreateMoveMenu2(&menu2, handles, NULL, CString());

    menu->AppendMenu(MF_STRING, ID_MOVE_UP, CMyMsg(MSG_5152));
    menu->AppendMenu(MF_STRING, ID_MOVE_DOWN, CMyMsg(MSG_5153));
    menu->AppendMenu(MF_STRING | MF_POPUP,
                     (INT_PTR)menu2.GetSafeHmenu(),
                     CMyMsg(MSG_5154));
}


//---------------------------------------------------------------------------
// CreateMoveMenu2
//---------------------------------------------------------------------------


void CSetLayoutDialog::CreateMoveMenu2(
    CMenu *menu, CPtrArray *handles, HTREEITEM hItem, CString prefix)
{
    CString text, prefix2;
    if (hItem) {
        text = m_tree->GetItemText(hItem);
        prefix2 = prefix + text + L" > ";
    } else {
        hItem = m_tree->GetRootItem();
        text = CMyMsg(MSG_5143);
    }
    menu->AppendMenu(
        MF_STRING, ID_HANDLES + handles->Add(hItem), prefix + text);

    hItem = m_tree->GetChildItem(hItem);
    while (hItem) {
        if (m_tree->GetItemState(hItem, TVIS_BOLD) & TVIS_BOLD)
            CreateMoveMenu2(menu, handles, hItem, prefix2);
        hItem = m_tree->GetNextSiblingItem(hItem);
    }
}


//---------------------------------------------------------------------------
// OnRestore
//---------------------------------------------------------------------------


void CSetLayoutDialog::OnRestore()
{
    HTREEITEM hRoot = m_tree->GetRootItem();
    if (hRoot) {

        BOX_ORDER_ENTRY *box_order = BoxOrder_ReadDefault();
        if (box_order) {

            if (box_order->children) {

                while (1) {
                    HTREEITEM hChildItem = m_tree->GetChildItem(hRoot);
                    if (! hChildItem)
                        break;
                    m_tree->DeleteItem(hChildItem);
                }

                InitTree2(hRoot, box_order->children);
            }

            BoxOrder_Free(box_order);
        }

        m_tree->SelectSetFirstVisible(hRoot);
    }
}


//---------------------------------------------------------------------------
// OnInsert
//---------------------------------------------------------------------------


void CSetLayoutDialog::OnInsert()
{
    HTREEITEM hOldItem = m_tree->GetSelectedItem();
    HTREEITEM hNewItem = m_tree->InsertItem(L"", hOldItem);
    if (hNewItem) {
        m_tree->SetItemState(hNewItem, TVIS_BOLD, TVIS_BOLD);
        m_tree->SelectItem(hNewItem);
        m_tree->EditLabel(hNewItem);
    }
}


//---------------------------------------------------------------------------
// OnRename
//---------------------------------------------------------------------------


void CSetLayoutDialog::OnRename()
{
    HTREEITEM hItem = m_tree->GetSelectedItem();
    if (hItem)
        m_tree->EditLabel(hItem);
}


//---------------------------------------------------------------------------
// OnLabelEdit
//---------------------------------------------------------------------------


void CSetLayoutDialog::OnLabelEdit(NMHDR *pNMHDR, LRESULT *plResult)
{
    if (pNMHDR->code != TVN_BEGINLABELEDIT &&
        pNMHDR->code != TVN_ENDLABELEDIT)
        return;

    NMTVDISPINFO *pNM = (NMTVDISPINFO *)pNMHDR;

    HTREEITEM hItem = pNM->item.hItem;
    if (! hItem)
        return;
    BOOL is_group = (m_tree->GetItemState(hItem, TVIS_BOLD) & TVIS_BOLD);

    m_inLabelEdit = false;

    if (pNMHDR->code == TVN_BEGINLABELEDIT) {

        if (is_group) {
            *plResult = FALSE;  // allow edit
            m_inLabelEdit = true;
        } else
            *plResult = TRUE;   // cancel edit

    } else if (pNMHDR->code == TVN_ENDLABELEDIT) {

        if (is_group) {

            CString text;
            if (pNM->item.pszText)
                text = pNM->item.pszText;
            text.TrimLeft();
            text.TrimRight();
            if (text.IsEmpty()) {
                text = m_tree->GetItemText(hItem);
                text.TrimLeft();
                text.TrimRight();
            }

            if (! text.IsEmpty())
                m_tree->SetItemText(hItem, text);
            else {
                m_tree->SelectItem(hItem);
                PostMessage(WM_COMMAND, ID_DELETE, NULL);
            }
        }

        *plResult = FALSE;      // reject text
    }
}


//---------------------------------------------------------------------------
// OnDelete
//---------------------------------------------------------------------------


void CSetLayoutDialog::OnDelete()
{
    HTREEITEM hItem = m_tree->GetSelectedItem();
    if (hItem && (m_tree->GetItemState(hItem, TVIS_BOLD) & TVIS_BOLD)) {
        HTREEITEM hParentItem = m_tree->GetParentItem(hItem);
        if (hParentItem) {

            ReparentItemForDelete(hItem, hParentItem);
            m_tree->DeleteItem(hItem);
            m_tree->SelectItem(hParentItem);
        }
    }
}


//---------------------------------------------------------------------------
// OnMoveUp
//---------------------------------------------------------------------------


void CSetLayoutDialog::OnMoveUp()
{
    HTREEITEM hItem = m_tree->GetSelectedItem();
    if (hItem) {

        HTREEITEM hParentItem = m_tree->GetParentItem(hItem);
        HTREEITEM hPrevItem = m_tree->GetPrevSiblingItem(hItem);
        if (hPrevItem) {
            hPrevItem = m_tree->GetPrevSiblingItem(hPrevItem);
            if (! hPrevItem)
                hPrevItem = TVI_FIRST;
        }

        if (hParentItem && hPrevItem) {

            HTREEITEM hNewItem = ReparentItem(hItem, hParentItem, hPrevItem);
            if (hNewItem) {
                m_tree->DeleteItem(hItem);
                m_tree->SelectItem(hNewItem);
            }
        }
    }
}


//---------------------------------------------------------------------------
// OnMoveDown
//---------------------------------------------------------------------------


void CSetLayoutDialog::OnMoveDown()
{
    HTREEITEM hItem = m_tree->GetSelectedItem();
    if (hItem) {

        HTREEITEM hParentItem = m_tree->GetParentItem(hItem);
        HTREEITEM hNextItem = m_tree->GetNextSiblingItem(hItem);

        if (hParentItem && hNextItem) {

            HTREEITEM hNewItem = ReparentItem(hItem, hParentItem, hNextItem);
            if (hNewItem) {
                m_tree->DeleteItem(hItem);
                m_tree->SelectItem(hNewItem);
            }
        }
    }
}


//---------------------------------------------------------------------------
// OnMoveTo
//---------------------------------------------------------------------------


void CSetLayoutDialog::OnMoveTo()
{
    HTREEITEM hItem = m_tree->GetSelectedItem();
    if (hItem) {

        HTREEITEM hOldParentItem = m_tree->GetParentItem(hItem);
        HTREEITEM hNewParentItem = (HTREEITEM)GetCurrentMessage()->lParam;

        HTREEITEM hNewAncessorItem = hNewParentItem;
        while (hNewAncessorItem) {
            if (hNewAncessorItem == hItem)
                break;
            hNewAncessorItem = m_tree->GetParentItem(hNewAncessorItem);
        }

        if (hOldParentItem && (! hNewAncessorItem) &&
                                    (hOldParentItem != hNewParentItem)) {

            HTREEITEM hNewItem = ReparentItem(hItem, hNewParentItem);
            if (hNewItem) {
                m_tree->DeleteItem(hItem);
                m_tree->SelectItem(hNewItem);
            }
        }
    }
}


//---------------------------------------------------------------------------
// ReparentItem
//---------------------------------------------------------------------------


HTREEITEM CSetLayoutDialog::ReparentItem(
    HTREEITEM hItem, HTREEITEM hNewParentItem, HTREEITEM hInsertAfterItem)
{
    CString text = m_tree->GetItemText(hItem);
    int state = m_tree->GetItemState(hItem, TVIS_BOLD);

    HTREEITEM hNewItem =
                m_tree->InsertItem(text, hNewParentItem, hInsertAfterItem);
    if (hNewItem) {

        m_tree->SetItemState(hNewItem, TVIS_BOLD, state);

        HTREEITEM hChildItem = m_tree->GetChildItem(hItem);
        while (hChildItem) {

            ReparentItem(hChildItem, hNewItem);
            hChildItem = m_tree->GetNextSiblingItem(hChildItem);
        }

        if (m_tree->GetChildItem(hNewItem))
            m_tree->Expand(hNewItem, TVE_EXPAND);
    }

    return hNewItem;
}


//---------------------------------------------------------------------------
// ReparentItemForDelete
//---------------------------------------------------------------------------


void CSetLayoutDialog::ReparentItemForDelete(
    HTREEITEM hItem, HTREEITEM hNewParentItem)
{
    HTREEITEM hChildItem = m_tree->GetChildItem(hItem);
    while (hChildItem) {

        if (m_tree->GetItemState(hChildItem, TVIS_BOLD) & TVIS_BOLD) {

            ReparentItemForDelete(hChildItem, hNewParentItem);

        } else {

            ReparentItem(hChildItem, hNewParentItem);
        }

        hChildItem = m_tree->GetNextSiblingItem(hChildItem);
    }
}
