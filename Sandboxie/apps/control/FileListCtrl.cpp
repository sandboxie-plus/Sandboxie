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
// File Tree Ctrl
//---------------------------------------------------------------------------


#include "stdafx.h"
#include "MyApp.h"
#include "FileListCtrl.h"

#include "ShellDialog.h"
#include "Boxes.h"
#include "QuickRecover.h"
#include "SbieIni.h"
#include "common/my_version.h"


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CFileListCtrl, CMyListCtrl)

    ON_COMMAND(ID_FILE_RUN,              OnCmdRun)
    ON_COMMAND(ID_FILE_RECOVER_SAME,     OnCmdRecoverFileOrFolder)
    ON_COMMAND(ID_FILE_RECOVER_ANY,      OnCmdRecoverFileOrFolder)
    ON_COMMAND(ID_FILE_RECOVER_ADD,      OnCmdAddOrRemoveFolder)
    ON_COMMAND(ID_FILE_RECOVER_CUT,      OnCmdCut)
    ON_COMMAND(ID_FILE_RECOVER_REMOVE,   OnCmdAddOrRemoveFolder)
    ON_COMMAND(ID_FILE_CREATE_SHORTCUT,  OnCmdCreateShortcut)

    ON_WM_KEYDOWN()
    ON_WM_KEYUP()

    ON_WM_DESTROYCLIPBOARD()

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// SetBoxNameForDialogMode
//---------------------------------------------------------------------------


void CFileListCtrl::SetBoxNameForDialogMode(const WCHAR *BoxName)
{
    m_BoxName = BoxName;
}


//---------------------------------------------------------------------------
// GetBoxNameForDialogMode
//---------------------------------------------------------------------------


const CString &CFileListCtrl::GetBoxNameForDialogMode() const
{
    return m_BoxName;
}


//---------------------------------------------------------------------------
// AnyQuickRecoverFiles
//---------------------------------------------------------------------------


bool CFileListCtrl::AnyQuickRecoverFiles() const
{
    return (m_anyQuickRecoverFiles == 1) ? true : false;
}


//---------------------------------------------------------------------------
// Create
//---------------------------------------------------------------------------


BOOL CFileListCtrl::Create(CWnd *pParentWnd)
{
    m_anyQuickRecoverFiles = 0;
    m_RefreshCounter = 0;

    InspectClipboard();

    //
    // create list control
    //

    int style = WS_CLIPCHILDREN;
    if (! m_BoxName.IsEmpty())
        style |= WS_CLIPSIBLINGS | WS_BORDER
              |  LVS_SHOWSELALWAYS | LVS_NOCOLUMNHEADER;
    CMyListCtrl::Create(pParentWnd, style, L"ZFILE");

    CListCtrl::InsertColumn(0, L"",  LVCFMT_LEFT, 2500, 0);

    //
    // create button
    //

    CMyListCtrl::CreateComboButton();

    if (m_BoxName.IsEmpty())
        m_combo.ShowWindow(SW_SHOW);
    else
        m_combo.EnableWindow(FALSE);

    RebuildCombo();

    //
    //
    //

    return TRUE;
}


//---------------------------------------------------------------------------
// OnSize
//---------------------------------------------------------------------------


void CFileListCtrl::OnSize(UINT nType, int cx, int cy)
{
    if (nType != SIZE_MINIMIZED) {

        CMyListCtrl::ResizeComboButton();

        UpdateScrollBar();
    }
}


//---------------------------------------------------------------------------
// UpdateScrollBar
//---------------------------------------------------------------------------


void CFileListCtrl::UpdateScrollBar()
{
    RECT rc;
    GetWindowRect(&rc);
    SCROLLINFO si;
    CListCtrl::GetScrollInfo(SB_HORZ, &si, SIF_PAGE);
    si.nPage = (rc.right - rc.left) / 4;
    CListCtrl::SetScrollInfo(SB_HORZ, &si, TRUE);
}


//---------------------------------------------------------------------------
// SelectAllItems
//---------------------------------------------------------------------------


void CFileListCtrl::SelectAllItems(bool select)
{
    ULONG ListCount = CListCtrl::GetItemCount();
    ULONG ListIndex = 0;
    while (ListIndex < ListCount) {
        CListCtrl::SetItemState(
            ListIndex, select ? LVIS_SELECTED : 0, LVIS_SELECTED);
        ++ListIndex;
    }
}


//---------------------------------------------------------------------------
// OnKeyDown
//---------------------------------------------------------------------------


void CFileListCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    BOOL ctrl = ((GetKeyState(VK_CONTROL) & 0x8000) == 0x8000);

    if ((nChar == VK_LEFT || nChar == VK_RIGHT) && (! ctrl))
        OnArrowKeyDown(nChar);

    else if (nChar == VK_TAB)
        m_combo.SetFocus();

    else if (nChar == 'A' && ctrl)
        SelectAllItems(true);

    else if (nChar == 'X' && ctrl) {
        int ListIndex = CListCtrl::GetNextItem(-1, LVNI_FOCUSED);
        SelectAllItems(false);
        CMyListCtrl::SelectIndex(ListIndex);
        OnCmdCut();
    }

    else {
        if ((nChar == VK_LEFT || nChar == VK_RIGHT) && ctrl)
            UpdateScrollBar();

        CMyListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
    }
}


//---------------------------------------------------------------------------
// OnKeyUp
//---------------------------------------------------------------------------


void CFileListCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar == VK_UP || nChar == VK_DOWN || nChar == VK_PRIOR ||
            nChar == VK_NEXT || nChar == VK_HOME || nChar == VK_END) {

        int index = CListCtrl::GetNextItem(-1, LVNI_FOCUSED);
        if (index != -1)
            MyEnsureVisible(index);

        UpdateScrollBar();
    }
}


//---------------------------------------------------------------------------
// OnArrowKeyDown
//---------------------------------------------------------------------------


void CFileListCtrl::OnArrowKeyDown(UINT nChar)
{
    int index = CListCtrl::GetNextItem(-1, LVNI_FOCUSED);
    if (index == -1)
        return;

    CBox &box = CBoxes::GetInstance().GetBox(m_BoxName);
    CBoxFile &boxfile = box.GetBoxFile();

    LVITEM lvi;
    memzero(&lvi, sizeof(LVITEM));
    lvi.mask = LVIF_IMAGE | LVIF_PARAM;
    lvi.iItem = index;
    if (! CListCtrl::GetItem(&lvi))
        return;

    BOOL new_expand = (nChar == VK_RIGHT) ? TRUE : FALSE;
    BOOL old_expand = new_expand;

    CString *boxpath = (CString *)lvi.lParam;
    if (! boxpath)
        goto navigate;

    bool isFolder = (lvi.iImage == m_imgMinus || lvi.iImage == m_imgPlus);
    if (! isFolder)
        goto navigate;

    if (boxfile.ChangeFolder(*boxpath) == -1)
        return;

    old_expand = boxfile.IsFolderExpandedView();
    if (old_expand != new_expand) {
        boxfile.SetFolderExpandedView(new_expand);

        SelectAllItems(false);
        ULONG flags = LVIS_SELECTED | LVIS_FOCUSED;
        CListCtrl::SetItemState(index, flags, flags);
    }

navigate:

    if (old_expand == new_expand) {

        int new_index = index;
        if (new_expand && (index + 1) < CListCtrl::GetItemCount())
            ++new_index;
        else if ((! new_expand) && index > 0)
            --new_index;
        if (new_index != index) {
            SelectAllItems(false);
            ULONG flags = LVIS_SELECTED | LVIS_FOCUSED;
            CListCtrl::SetItemState(index, 0, flags);
            CListCtrl::SetItemState(new_index, flags, flags);
            index = new_index;
        }
    }

    MyEnsureVisible(index);
    CMyListCtrl::PostRefresh(index);
}


//---------------------------------------------------------------------------
// MyEnsureVisible
//---------------------------------------------------------------------------


void CFileListCtrl::MyEnsureVisible(int index)
{
    LVITEM lvi;
    memzero(&lvi, sizeof(LVITEM));
    lvi.mask = LVIF_INDENT | LVIF_IMAGE | LVIF_PARAM;
    lvi.iItem = index;
    if (! CListCtrl::GetItem(&lvi)) {
        CListCtrl::EnsureVisible(index, FALSE);
        return;
    }

    CListCtrl::EnsureVisible(index, TRUE);

    int thumbw = GetSystemMetrics(SM_CXICON);

    RECT rc;
    GetClientRect(&rc);
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;

    int scrollw = GetScrollPos(SB_HORZ);
    CSize sz;
    sz.cx = width / 2;
    sz.cy = 0;

    if (lvi.iIndent * thumbw - scrollw >= width * 65 / 100) {

        CListCtrl::Scroll(sz);

    } else if (lvi.iIndent * thumbw < scrollw) {

        sz.cx = lvi.iIndent * thumbw - scrollw;
        CListCtrl::Scroll(sz);
    }

    //
    //
    //

    if (lvi.lParam) {
        CString text = *(CString *)lvi.lParam;

        LVCOLUMN col;
        memzero(&col, sizeof(col));
        col.mask = LVCF_TEXT;
        col.pszText = (WCHAR *)(const WCHAR *)text;
        CListCtrl::SetColumn(0, &col);
    }
}


//---------------------------------------------------------------------------
// OnDestroyClipboard
//---------------------------------------------------------------------------


void CFileListCtrl::OnDestroyClipboard()
{
    while (! m_CutPaths.IsEmpty())
        m_CutPaths.RemoveHead();
    if (m_DontClearCutMarks)
        return;
    while (1) {
        int ListIndex = CListCtrl::GetNextItem(-1, LVNI_CUT);
        if (ListIndex == -1)
            break;
        CListCtrl::SetItemState(ListIndex, 0, LVIS_CUT);
    }
}


//---------------------------------------------------------------------------
// RefreshContent
//---------------------------------------------------------------------------


void CFileListCtrl::RefreshContent()
{
    UpdateScrollBar();

    //
    // refresh combo box
    //

    if (m_combo.IsWindowEnabled()) {

        int index = m_combo.GetCurSel();
        if (index == LB_ERR) {

            RebuildCombo();
            return;

        } else {

            CString *pBoxName = (CString *)m_combo.GetItemDataPtr(index);
            CBox &box = CBoxes::GetInstance().GetBox(*pBoxName);
            if (box.GetName() != *pBoxName) {

                RebuildCombo();
                return;
            }
        }

        CStringList OrderedBoxList;
        CMyListCtrl::GetOrderedBoxList(OrderedBoxList, NULL);

        for (index = 0; (! OrderedBoxList.IsEmpty()); ++index) {

            CString BoxName1 = OrderedBoxList.RemoveHead();
            CString *BoxName2 = (CString *)m_combo.GetItemDataPtr(index);
            if (BoxName2 == (CString *)-1)
                BoxName2 = NULL;
            if ((! BoxName2) || BoxName2->CompareNoCase(BoxName1) != 0) {

                RebuildCombo();
                return;
            }
        }
    }

    //
    // refresh files in sandbox
    //

    CBox &box = CBoxes::GetInstance().GetBox(m_BoxName);
    CBoxFile &boxfile = box.GetBoxFile();
    if (m_RefreshCounter != boxfile.GetBoxRefreshCounter())
        RebuildFiles();
}


//---------------------------------------------------------------------------
// OnComboSelect
//---------------------------------------------------------------------------


void CFileListCtrl::OnComboSelect()
{
    int index = m_combo.GetCurSel();
    if (index == LB_ERR)
        return;

    CString *pBoxName = (CString *)m_combo.GetItemDataPtr(index);
    CBox &box = CBoxes::GetInstance().GetBox(*pBoxName);
    if (box.GetName() == *pBoxName) {
        m_BoxName = *pBoxName;
        RebuildFiles();
    } else
        RebuildCombo();

    this->SetFocus();

    CMyListCtrl::PostRefresh(-1);
}


//---------------------------------------------------------------------------
// RebuildCombo
//---------------------------------------------------------------------------


void CFileListCtrl::RebuildCombo()
{
    int i;
    int index = -1;
    CString *pBoxName;

    for (i = m_combo.GetCount() - 1; i >= 0; --i) {
        pBoxName = (CString *)m_combo.GetItemDataPtr(i);
        delete pBoxName;
        m_combo.DeleteString(i);
    }

    CBoxes &boxes = CBoxes::GetInstance();
    if (boxes.GetSize() <= 1) {

        // no sandboxes available
        m_BoxName = CString();
        index = 0;

    } else {

        if (m_BoxName.IsEmpty())
            m_BoxName = L"DefaultBox";

        CStringList OrderedBoxList;
        CMyListCtrl::GetOrderedBoxList(OrderedBoxList, NULL);

        while (! OrderedBoxList.IsEmpty()) {

            CString BoxName = OrderedBoxList.RemoveHead();
            CBox &box = boxes.GetBox(BoxName);
            pBoxName = new CString(box.GetName());
            WCHAR *BoxNameText = SbieDll_FormatMessage1(MSG_3515, *pBoxName);
            i = m_combo.AddString(BoxNameText);
            LocalFree(BoxNameText);
            m_combo.SetItemDataPtr(i, pBoxName);
            if (pBoxName->CompareNoCase(m_BoxName) == 0)
                index = i;
        }

        if (index == -1) {
            index = 0;
            m_BoxName = boxes.GetBox(1).GetName();
        }
    }

    m_combo.SetCurSel(index);

    RebuildFiles();
}


//---------------------------------------------------------------------------
// RebuildFiles
//---------------------------------------------------------------------------


void CFileListCtrl::RebuildFiles()
{
    int i;
    CString *data;

    //
    // save path to item that has the focus
    //

    CString SaveItemPath;

    if (m_SaveBoxName == m_BoxName) {

        i = CListCtrl::GetNextItem(-1, LVNI_FOCUSED);
        if (i != -1) {
            data = (CString *)CListCtrl::GetItemData(i);
            if (! data)
                data = (CString *)CListCtrl::GetItemData(i - 1);
            if (data)
                SaveItemPath = *data;
        }
    }

    m_SaveBoxName = m_BoxName;

    //
    // delete all contents from the list view
    //

    for (i = CListCtrl::GetItemCount() - 1; i >= 0; --i)
        CMyListCtrl::MyDeleteItem(i);

    CMyListCtrl::SelectIndex(-1);

    if (m_BoxName.IsEmpty()) {
        m_RefreshCounter = 0;
        return;
    }

    //
    // populate the list view
    //

    CBox &box = CBoxes::GetInstance().GetBox(m_BoxName);
    CBoxFile &boxfile = box.GetBoxFile();

    if (! m_combo.IsWindowEnabled())
        boxfile.RebuildQuickRecoveryFolders();

    m_RefreshCounter = boxfile.GetBoxRefreshCounter();
    if (boxfile.ChangeFolder(L"\\") == -1)
        return;

    if (m_combo.IsWindowEnabled()) {

        //
        // frame mode: enumerate everything below the root folder
        //

        InsertFolder(boxfile, L"", 1, 0);

    } else {

        //
        // dialog mode: change into virtual folder \Quick Recovery Folders
        // and use that as the root folder
        //

        m_anyQuickRecoverFiles = -1;

        POSITION pos = boxfile.GetFolderHeadPosition();
        BOOL expanded;
        const WCHAR *folder_name = boxfile.GetNextFolder(pos, expanded);
        boxfile.ChangeFolder(folder_name, TRUE);
        InsertFolder(boxfile, CString(L"\\") + folder_name, 0, 0);

        if (m_anyQuickRecoverFiles == -1) {

            CMyMsg none(MSG_3732);

            LVITEM lvi;
            memzero(&lvi, sizeof(LVITEM));
            lvi.mask = LVIF_INDENT | LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM;
            lvi.iImage = m_imgEmpty;
            lvi.pszText = (WCHAR *)(const WCHAR *)none;
            CListCtrl::InsertItem(&lvi);
        }
    }

    SetFocusByPath(SaveItemPath);
    RebuildImageList();
}


//---------------------------------------------------------------------------
// InsertFolder
//---------------------------------------------------------------------------


int CFileListCtrl::InsertFolder(
    CBoxFile &boxfile, const CString &_boxpath,
    int ListIndex, int ListIndent)
{
    ULONG ListIndex0 = ListIndex;

    LVITEM lvi;
    memzero(&lvi, sizeof(LVITEM));
    lvi.mask = LVIF_INDENT | LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM
             | LVIF_STATE;
    lvi.iIndent = ListIndent;
    lvi.stateMask = LVIS_CUT;

    CString boxpath = _boxpath + L"\\";

    CString TruePath, CopyPath;
    boxfile.GetFolderPaths(TruePath, CopyPath);

    //
    //
    //

    POSITION pos = boxfile.GetFolderHeadPosition();
    while (pos) {

        BOOL expanded;
        const WCHAR *folder_name = boxfile.GetNextFolder(pos, expanded);
        if (! folder_name)
            break;
        const CString folder_boxpath = boxpath + folder_name;

        if (m_anyQuickRecoverFiles) {
            expanded = FALSE;
            if (boxfile.ChangeFolder(folder_name) != -1) {
                if (boxfile.GetFolderNumChildren() > 0) {
                    boxfile.SetFolderExpandedView(TRUE);
                    expanded = TRUE;
                }
                boxfile.ChangeFolder(L"..");
            }
        }

        CString FolderCopyPath = CopyPath + L"\\" + folder_name;

        lvi.iItem = ListIndex;
        lvi.iImage = expanded ? m_imgMinus : m_imgPlus;
        lvi.pszText = (WCHAR *)folder_name;
        lvi.lParam = (LPARAM)new CString(folder_boxpath);
        lvi.state = IsCutPath(FolderCopyPath) ? LVIS_CUT : 0;
        CListCtrl::InsertItem(&lvi);
        ++ListIndex;

        int oldListIndex = ListIndex;

        if (expanded) {
            if (boxfile.ChangeFolder(folder_name) != -1) {

                ListIndex = InsertFolder(
                    boxfile, folder_boxpath, ListIndex,
                        ListIndent + (m_combo.IsWindowEnabled() ? 1 : 2));
                boxfile.ChangeFolder(L"..");
            }
        }

        if (m_anyQuickRecoverFiles && ListIndex == oldListIndex) {

            ListIndex = oldListIndex - 1;
            CMyListCtrl::MyDeleteItem(ListIndex);
        }
    }

    //
    //
    //

    pos = boxfile.GetFolderHeadPosition();
    while (1) {

        ULONG64 bytes64;
        const WCHAR *file_name = boxfile.GetNextFile(pos, bytes64);
        if (! file_name)
            break;

        CString FileCopyPath = CopyPath + L"\\" + file_name;

        if (m_anyQuickRecoverFiles) {

            bool shouldShowFile = ShouldShowFileInQuickRecovery(
                                        file_name, bytes64, FileCopyPath);
            if (! shouldShowFile)
                continue;

        } else {

            //
            // in Files and Folders View, try to get most up to date file size
            //

            ULONG attrs;
            ULONG64 bytes64x;
            if (boxfile.QueryFileAttributes(FileCopyPath, &attrs, &bytes64x))
                bytes64 = bytes64x;
        }

        const CString file_box_path = boxpath + file_name;
        InsertFile(file_box_path, CopyPath, bytes64, ListIndex, ListIndent);
        ++ListIndex;

        if (m_anyQuickRecoverFiles == -1)
            m_anyQuickRecoverFiles = 1;
    }

    //
    //
    //

    if (ListIndex == ListIndex0 && (! m_anyQuickRecoverFiles)) {

        CMyMsg none(MSG_3769);

        lvi.iItem = ListIndex;
        lvi.iImage = m_imgEmpty;
        lvi.pszText = (WCHAR *)(const WCHAR *)none;
        lvi.lParam = 0;
        lvi.state = 0;
        CListCtrl::InsertItem(&lvi);
        ++ListIndex;
    }

    return ListIndex;
}


//---------------------------------------------------------------------------
// InsertFile
//---------------------------------------------------------------------------


void CFileListCtrl::InsertFile(
    const CString &VirtualPath, const CString &FolderCopyPath,
    ULONG64 bytes64, int ListIndex, int ListIndent)
{
    LVITEM lvi;
    memzero(&lvi, sizeof(LVITEM));
    lvi.mask = LVIF_IMAGE | LVIF_TEXT | LVIF_STATE;
    lvi.iItem = ListIndex;
    lvi.stateMask = LVIS_CUT;

    if (ListIndent != -1) {
        lvi.mask |= LVIF_INDENT | LVIF_PARAM;
        lvi.iIndent = ListIndent;
        lvi.lParam = (LPARAM)new CString(VirtualPath);
    }

    //
    //
    //

    const WCHAR *FileName = wcsrchr((const WCHAR *)VirtualPath, L'\\');
    const CString FileCopyPath = FolderCopyPath + FileName;

    SHFILEINFO shfi;
    ULONG_PTR rc = SHGetFileInfo(FileCopyPath, 0, &shfi, sizeof(shfi),
                                 SHGFI_ICON /*| SHGFI_SMALLICON*/);
    if (rc) {
        lvi.iImage = CMyListCtrl::AddToImageList(shfi.hIcon);
        DestroyIcon(shfi.hIcon);
    } else
        lvi.iImage = m_imgEmpty;

    lvi.state = IsCutPath(FileCopyPath) ? LVIS_CUT : 0;

    //
    //
    //

    const WCHAR *units = L"B";
    if (bytes64 >= 1024) {
        bytes64 /= 1024;
        units = L"KB";
        if (bytes64 >= 1024) {
            bytes64 /= 1024;
            units = L"MB";
            if (bytes64 >= 1024) {
                bytes64 /= 1024;
                units = L"GB";
            }
        }
    }

    CString label;
    label.Format(L"%s  (%d %s)", FileName + 1, (ULONG)bytes64, units);
    lvi.pszText = (WCHAR *)(const WCHAR *)label;

    //
    //
    //

    if (ListIndent == -1)
        CListCtrl::SetItem(&lvi);
    else
        CListCtrl::InsertItem(&lvi);
}


//---------------------------------------------------------------------------
// RemoveFolderItems
//---------------------------------------------------------------------------


void CFileListCtrl::RemoveFolderItems(int ListIndex, int ListIndent)
{
    LVITEM lvi;
    memzero(&lvi, sizeof(LVITEM));
    lvi.mask = LVIF_INDENT | LVIF_IMAGE | LVIF_PARAM;
    lvi.iItem = ListIndex + 1;
    while (1) {
        if (! CListCtrl::GetItem(&lvi))
            break;
        if (lvi.iIndent <= ListIndent)
            break;
        CMyListCtrl::MyDeleteItem(lvi.iItem);
    }
}


//---------------------------------------------------------------------------
// SetFocusByPath
//---------------------------------------------------------------------------


void CFileListCtrl::SetFocusByPath(CString SaveItemPath)
{
    CString ColumnText;

    if (! SaveItemPath.IsEmpty()) {

        int i, FoundIndex = 0;
        CString *data;
        while (1) {

            for (i = CListCtrl::GetItemCount() - 1; i >= 0; --i) {
                data = (CString *)CListCtrl::GetItemData(i);
                if (data && data->CompareNoCase(SaveItemPath) == 0) {

                    FoundIndex = i;
                    break;
                }
            }

            if (FoundIndex)
                break;
            int BackslashIndex = SaveItemPath.ReverseFind(L'\\');
            if (BackslashIndex <= 0)
                break;
            SaveItemPath = SaveItemPath.Left(BackslashIndex);
        }

        if (FoundIndex) {
            CMyListCtrl::SelectIndex(FoundIndex);
            ColumnText = *data;
        }
    }

    LVCOLUMN col;
    memzero(&col, sizeof(col));
    col.mask = LVCF_TEXT;
    col.pszText = (WCHAR *)(const WCHAR *)ColumnText;
    CListCtrl::SetColumn(0, &col);
}


//---------------------------------------------------------------------------
// OnRefreshPosted
//---------------------------------------------------------------------------


void CFileListCtrl::OnRefreshPosted()
{
    //
    //
    //

    int ListIndex = (int)GetCurrentMessage()->lParam;

    if (ListIndex == -1) {      // from OnComboSelect or OnScroll
        UpdateScrollBar();
        return;
    }

    LVITEM lvi;
    memzero(&lvi, sizeof(LVITEM));
    lvi.mask = LVIF_INDENT | LVIF_IMAGE | LVIF_PARAM;
    lvi.iItem = ListIndex;
    if (! CListCtrl::GetItem(&lvi))
        return;
    if (! lvi.lParam)
        return;
    int ListIndent = lvi.iIndent;

    SetRedraw(FALSE);

    bool isFolder = (lvi.iImage == m_imgMinus || lvi.iImage == m_imgPlus);
    if (isFolder) {

        RemoveFolderItems(ListIndex, ListIndent);
    }

    //
    //
    //

    CString FolderCopyPath;
    ULONG attrs = 0;
    ULONG64 bytes64 = 0;
    BOOL expanded = FALSE;

    const CString *VirtualPath = (CString *)lvi.lParam;
    CString VirtualFolderPath, FileName;
    if (isFolder)
        VirtualFolderPath = *VirtualPath;
    else {
        int BackslashIndex = VirtualPath->ReverseFind(L'\\');
        VirtualFolderPath = VirtualPath->Left(BackslashIndex);
        FileName = VirtualPath->Mid(BackslashIndex);
    }

    CBox &box = CBoxes::GetInstance().GetBox(m_BoxName);
    CBoxFile &boxfile = box.GetBoxFile();
    if (boxfile.ChangeFolder(VirtualFolderPath) != -1) {

        expanded = boxfile.IsFolderExpandedView();

        if (boxfile.IsPhysicalFolder()) {

            CString TruePath;
            boxfile.GetFolderPaths(TruePath, FolderCopyPath);
            CString FileCopyPath = FolderCopyPath + FileName;

            if (boxfile.QueryFileAttributes(
                                    FileCopyPath, &attrs, &bytes64)) {

                if (attrs & FILE_ATTRIBUTE_DIRECTORY)
                    attrs = FILE_ATTRIBUTE_DIRECTORY;
                else
                    attrs = FILE_ATTRIBUTE_NORMAL;
            }

        } else
            attrs = FILE_ATTRIBUTE_DIRECTORY;
    }

    //
    //
    //

    if (attrs & FILE_ATTRIBUTE_DIRECTORY) {

        if (! isFolder) {
            boxfile.ChangeFolder(VirtualFolderPath, TRUE);
            expanded = FALSE;
            VirtualFolderPath = *VirtualPath;
        }

        lvi.mask = LVIF_IMAGE | LVIF_STATE;
        lvi.stateMask = LVIS_CUT;
        lvi.state = 0;
        lvi.iImage = expanded ? m_imgMinus : m_imgPlus;
        if (! isFolder) {
            lvi.mask |= LVIF_TEXT;
            lvi.pszText = (WCHAR *)((const WCHAR *)FileName + 1);
        }

        if (! m_CutPaths.IsEmpty()) {
            CString CopyPath;
            if (IsPhysicalItem(ListIndex, NULL, NULL, &CopyPath)) {
                if (IsCutPath(CopyPath))
                    lvi.state = LVIS_CUT;
            }
        }

        CListCtrl::SetItem(&lvi);

        if (expanded &&
                boxfile.ChangeFolder(VirtualFolderPath, TRUE) != -1) {

            InsertFolder(
                boxfile, *VirtualPath, ListIndex + 1,
                    ListIndent + (m_combo.IsWindowEnabled() ? 1 : 2));
        }

    } else {

        if (attrs & FILE_ATTRIBUTE_NORMAL) {

            if (isFolder) {
                int BackslashIndex = FolderCopyPath.ReverseFind(L'\\');
                FolderCopyPath = FolderCopyPath.Left(BackslashIndex);
            }

            if (isFolder)
                CMyListCtrl::MyDeleteItem(ListIndex);
            else
                ListIndent = -1;    // replace

            InsertFile(*VirtualPath, FolderCopyPath, bytes64,
                       ListIndex, ListIndent);

        } else {

            CMyListCtrl::MyDeleteItem(ListIndex);
        }
    }

    SetRedraw(TRUE);
    UpdateScrollBar();
}


//---------------------------------------------------------------------------
// OnClick
//---------------------------------------------------------------------------


void CFileListCtrl::OnClick(NMHDR *pNMHDR, LRESULT *pResult)
{
    NMITEMACTIVATE *nm = (NMITEMACTIVATE *)pNMHDR;
    CString *boxpath = NULL;
    if (nm->iItem == -1)
        return;

    LVITEM lvi;
    memzero(&lvi, sizeof(LVITEM));
    lvi.mask = LVIF_INDENT | LVIF_IMAGE | LVIF_PARAM;
    lvi.iItem = nm->iItem;
    if (! CListCtrl::GetItem(&lvi))
        return;

    boxpath = (CString *)lvi.lParam;
    if (! boxpath)
        return;

    bool expand = false;
    bool isFolder = (lvi.iImage == m_imgMinus || lvi.iImage == m_imgPlus);
    if (isFolder) {
        if (nm->hdr.code == NM_DBLCLK ||
            GetScrollPos(SB_HORZ) + nm->ptAction.x < 32 * (lvi.iIndent + 1))
                expand = true;
    } else
        expand = true;

    if (expand) {

        CBox &box = CBoxes::GetInstance().GetBox(m_BoxName);
        CBoxFile &boxfile = box.GetBoxFile();
        if (boxfile.ChangeFolder(*boxpath) != -1) {
            BOOL expanded = boxfile.IsFolderExpandedView();
            boxfile.SetFolderExpandedView(! expanded);
        }
    }

    MyEnsureVisible(nm->iItem);
    CMyListCtrl::PostRefresh(nm->iItem);
}


//---------------------------------------------------------------------------
// GetSelectedBox
//---------------------------------------------------------------------------


CBox &CFileListCtrl::GetSelectedBox(CWnd *pWnd, CPoint pt) const
{
    if (m_combo.IsWindowEnabled()) {

        if (GetFocus() == &m_combo)
            return CBoxes::GetInstance().GetBox(m_BoxName);

        RECT rc;
        m_combo.GetWindowRect(&rc);
        if (pt.x >= rc.left && pt.x <= rc.right &&
                pt.y >= rc.top && pt.y <= rc.bottom) {

            return CBoxes::GetInstance().GetBox(m_BoxName);
        }
    }

    return CBoxes::GetInstance().GetBox(0);
}


//---------------------------------------------------------------------------
// GetSelectedItemPosition
//---------------------------------------------------------------------------


void CFileListCtrl::GetSelectedItemPosition(CPoint &pt) const
{
    if (m_combo.IsWindowEnabled() && GetFocus() == &m_combo) {

        RECT rc;
        m_combo.GetWindowRect(&rc);
        pt.x = rc.left + 20;
        pt.y = rc.top + 20;

    } else
        return CMyListCtrl::GetSelectedItemPosition(pt);
}


//---------------------------------------------------------------------------
// IsPhysicalItem
//---------------------------------------------------------------------------


bool CFileListCtrl::IsPhysicalItem(
    int ListIndex, bool *pIsFolder, CString *pTruePath, CString *pCopyPath)
{
    LVITEM lvi;
    memzero(&lvi, sizeof(LVITEM));
    lvi.mask = LVIF_IMAGE | LVIF_PARAM;
    lvi.iItem = ListIndex;
    if (! CListCtrl::GetItem(&lvi))
        return false;

    //
    // the virtual path of the item should have more than one backslash
    //

    if (! lvi.lParam)
        return false;
    CString *VirtualPath = (CString *)lvi.lParam;
    int FirstBackslashIndex = VirtualPath->Find(L'\\');
    if (FirstBackslashIndex == -1)
        return false;
    int LastBackslashIndex = VirtualPath->ReverseFind(L'\\');
    if (LastBackslashIndex == FirstBackslashIndex)
        return false;

    //
    // the virtual path of the item should translate to a physical folder
    //

    CString VirtualFolderPath;
    bool isFolder = (lvi.iImage == m_imgMinus || lvi.iImage == m_imgPlus);
    if (isFolder)
        VirtualFolderPath = *VirtualPath;
    else
        VirtualFolderPath = VirtualPath->Left(LastBackslashIndex);

    CBox &box = CBoxes::GetInstance().GetBox(m_BoxName);
    CBoxFile &boxfile = box.GetBoxFile();
    if (boxfile.ChangeFolder(VirtualFolderPath) == -1)
        return false;

    CString TruePath, CopyPath;
    boxfile.GetFolderPaths(TruePath, CopyPath);

    if (TruePath.IsEmpty())
        return false;

    //
    // return results to caller
    //

    if (pIsFolder)
        *pIsFolder = isFolder;

    if (pTruePath || pCopyPath) {

        if (! isFolder) {
            CString FileName;
            FileName = VirtualPath->Mid(LastBackslashIndex);
            TruePath += FileName;
            CopyPath += FileName;
        }

        if (pTruePath)
            *pTruePath = TruePath;
        if (pCopyPath)
            *pCopyPath = CopyPath;
    }

    return true;
}


//---------------------------------------------------------------------------
// IsCutPath
//---------------------------------------------------------------------------


bool CFileListCtrl::IsCutPath(const CString &CopyPath)
{
    POSITION pos = m_CutPaths.GetHeadPosition();
    while (pos) {
        const CString &path = m_CutPaths.GetNext(pos);
        if (path.CompareNoCase(CopyPath) == 0)
            return true;
    }
    return false;
}


//---------------------------------------------------------------------------
// OnContextMenu
//---------------------------------------------------------------------------


void CFileListCtrl::OnContextMenu(CWnd *pWnd, CPoint pt)
{
    //
    // get select item
    //

    if (pt.x == -1 && pt.y == -1)
        GetSelectedItemPosition(pt);
    ScreenToClient(&pt);
    int index = HitTest(pt, NULL);
    if (index == -1)
        return;

    MyEnsureVisible(index);

    //
    // proces selection and display context menu
    //

    CMenu *pMenu = m_pContextMenu->GetSubMenu(0);
    bool doMenu = false;

    if (CListCtrl::GetSelectedCount() > 1)
        doMenu = OnContextMenuMultiple(pMenu, pt);
    else
        doMenu = OnContextMenuSingle(index, pMenu, pt);

    if (doMenu) {

        ClientToScreen(&pt);
        pMenu->TrackPopupMenu(0, pt.x, pt.y, this, NULL);
    }
}


//---------------------------------------------------------------------------
// OnContextMenuMultiple
//---------------------------------------------------------------------------


bool CFileListCtrl::OnContextMenuMultiple(CMenu *pMenu, CPoint pt)
{
    //
    // make sure at least one selected item is a real file
    //

    bool any_physical = false;
    int index = -1;
    while (1) {
        index = CListCtrl::GetNextItem(index, LVNI_SELECTED);
        if (index == -1)
            break;
        if (IsPhysicalItem(index, NULL, NULL, NULL)) {
            any_physical = true;
            break;
        }
    }
    if (! any_physical)
        return false;

    //
    // turn on the recover buttons, turn everything else off
    //

    pMenu->EnableMenuItem(ID_FILE_RUN,              MF_DISABLED | MF_GRAYED);
    pMenu->EnableMenuItem(ID_FILE_RECOVER_SAME,     MF_ENABLED);
    pMenu->EnableMenuItem(ID_FILE_RECOVER_ANY,      MF_ENABLED);
    pMenu->EnableMenuItem(ID_FILE_RECOVER_CUT,      MF_ENABLED);
    pMenu->EnableMenuItem(ID_FILE_RECOVER_ADD,      MF_DISABLED | MF_GRAYED);
    pMenu->EnableMenuItem(ID_FILE_RECOVER_REMOVE,   MF_DISABLED | MF_GRAYED);
    pMenu->EnableMenuItem(ID_FILE_CREATE_SHORTCUT,  MF_DISABLED | MF_GRAYED);

    return true;
}


//---------------------------------------------------------------------------
// OnContextMenuSingle
//---------------------------------------------------------------------------


bool CFileListCtrl::OnContextMenuSingle(int index, CMenu *pMenu, CPoint pt)
{
    //
    // make sure at least one selected item is a real file
    //

    bool isFolder;
    CString TruePath;
    if (! IsPhysicalItem(index, &isFolder, &TruePath, NULL))
        return false;

    //
    // turn on the buttons to add/remove quick recovery folder
    //

    CBox &box = CBoxes::GetInstance().GetBox(m_BoxName);
    CBoxFile &boxfile = box.GetBoxFile();

    int state4, state5, state6;
    state4 = state5 = state6 = MF_DISABLED | MF_GRAYED;
    if (isFolder) {

        if (boxfile.IsQuickRecoverFolder()) {

            CString TruePath2 = TruePath;
            TruePath2.TrimRight(L'\\');

            CSbieIni &ini = CSbieIni::GetInstance();

            CStringList qrFolders;
            box.GetQuickRecoveryFolders(qrFolders, TRUE);

            while (! qrFolders.IsEmpty()) {

                CString qrFolder = qrFolders.RemoveHead();
                qrFolder =
                    CSbieIni::GetInstance().MakeSpecificPath(qrFolder);

                if (qrFolder.CompareNoCase(TruePath) == 0 ||
                        qrFolder.CompareNoCase(TruePath2) == 0) {
                    state5 = MF_ENABLED;
                    break;
                }
            }

        } else {
            state4 = MF_ENABLED;
            state6 = MF_ENABLED;
        }

    } else if (! boxfile.IsQuickRecoverFolder())
        state6 = MF_ENABLED;

    pMenu->EnableMenuItem(ID_FILE_RUN,              MF_ENABLED);
    pMenu->EnableMenuItem(ID_FILE_RECOVER_SAME,     MF_ENABLED);
    pMenu->EnableMenuItem(ID_FILE_RECOVER_ANY,      MF_ENABLED);
    pMenu->EnableMenuItem(ID_FILE_RECOVER_CUT,      MF_ENABLED);
    pMenu->EnableMenuItem(ID_FILE_RECOVER_ADD,      state4);
    pMenu->EnableMenuItem(ID_FILE_RECOVER_REMOVE,   state5);
    pMenu->EnableMenuItem(ID_FILE_CREATE_SHORTCUT,  state6);

    return true;
}


//---------------------------------------------------------------------------
// OnCmdRun
//---------------------------------------------------------------------------


void CFileListCtrl::OnCmdRun()
{
    if (CListCtrl::GetSelectedCount() > 1)
        return;
    int index = CListCtrl::GetNextItem(-1, LVNI_SELECTED);
    if (index == -1)
        return;

    CString TruePath;
    if (! IsPhysicalItem(index, NULL, &TruePath, NULL))
        return;

    CString QuotedTruePath = L"\"" + TruePath + L"\"";
    CMyApp::RunStartExe(QuotedTruePath, m_BoxName);
}


//---------------------------------------------------------------------------
// OnCmdCreateShortcut
//---------------------------------------------------------------------------


void CFileListCtrl::OnCmdCreateShortcut()
{
    if (CListCtrl::GetSelectedCount() > 1)
        return;
    int index = CListCtrl::GetNextItem(-1, LVNI_SELECTED);
    if (index == -1)
        return;

    CString TruePath;
    if (! IsPhysicalItem(index, NULL, &TruePath, NULL))
        return;

    CShellDialog::CreateShortcut(m_BoxName, TruePath);
}


//---------------------------------------------------------------------------
// OnCmdCut
//---------------------------------------------------------------------------


void CFileListCtrl::OnCmdCut()
{
    //
    // turn off selection on child items when the parent item is selected
    //

    LVITEM lvi;
    memzero(&lvi, sizeof(LVITEM));
    lvi.mask = LVIF_INDENT | LVIF_IMAGE | LVIF_PARAM;

    int ListIndex = -1;
    while (1) {
        ListIndex = CListCtrl::GetNextItem(ListIndex, LVNI_SELECTED);
        if (ListIndex == -1)
            break;

        lvi.iItem = ListIndex;
        if (CListCtrl::GetItem(&lvi)) {

            bool isFolder =
                    (lvi.iImage == m_imgMinus || lvi.iImage == m_imgPlus);
            if (isFolder) {

                lvi.iItem = ListIndex;
                int oldIndent = lvi.iIndent;
                while (1) {

                    ++lvi.iItem;
                    if (! CListCtrl::GetItem(&lvi))
                        break;
                    if (lvi.iIndent <= oldIndent)
                        break;

                    CListCtrl::SetItemState(lvi.iItem, 0, LVIS_SELECTED);
                }
            }
        }
    }

    //
    // extract each item that is still selected
    //

    CStringList ItemsToRecover;
    ULONG DropFiles_len = 0;

    bool clearCutMarks = true;

    ListIndex = -1;
    while (1) {
        ListIndex = CListCtrl::GetNextItem(ListIndex, LVNI_SELECTED);
        if (ListIndex == -1)
            break;

        CString CopyPath;
        if (IsPhysicalItem(ListIndex, NULL, NULL, &CopyPath)) {

            ItemsToRecover.AddTail(CopyPath);
            DropFiles_len += (CopyPath.GetLength() + 1) * sizeof(WCHAR);

            if (clearCutMarks) {
                OnDestroyClipboard();
                clearCutMarks = false;
            }

            CListCtrl::SetItemState(ListIndex, LVIS_CUT, LVIS_CUT);
        }
    }

    if (! DropFiles_len)
        return;

    //
    // build DROPFILES structure
    //

    DropFiles_len += sizeof(DROPFILES) + 4;
    HGLOBAL hDropFiles = GlobalAlloc(GMEM_MOVEABLE, DropFiles_len);
    if (! hDropFiles)
        return;

    DROPFILES *pDropFiles = (DROPFILES *)GlobalLock(hDropFiles);
    if (! pDropFiles) {
        GlobalFree(hDropFiles);
        return;
    }

    WCHAR *pNextFile = (WCHAR *)(pDropFiles + 1);

    memzero(pDropFiles, sizeof(DROPFILES));
    pDropFiles->pFiles =
        (ULONG)((ULONG_PTR)pNextFile - (ULONG_PTR)pDropFiles);
    pDropFiles->fWide = TRUE;

    POSITION pos = ItemsToRecover.GetHeadPosition();
    while (pos) {
        CString item = ItemsToRecover.GetNext(pos);
        wcscpy(pNextFile, (const WCHAR *)item);
        pNextFile += wcslen(pNextFile) + 1;
    }

    *pNextFile = L'\0';

    GlobalUnlock(hDropFiles);

    //
    // build move hint
    //

    HGLOBAL hMoveHint = GlobalAlloc(GMEM_MOVEABLE, sizeof(ULONG));
    if (hMoveHint) {
        ULONG *pMoveHint = (ULONG *)GlobalLock(hMoveHint);
        if (pMoveHint) {
            *pMoveHint = DROPEFFECT_MOVE;
            GlobalUnlock(hMoveHint);
        }
    }

    //
    // build Sandboxie Control hint
    //

    HGLOBAL hSbieHint = GlobalAlloc(GMEM_MOVEABLE, sizeof(ULONG));
    if (hSbieHint) {
        ULONG *pSbieHint = (ULONG *)GlobalLock(hSbieHint);
        if (pSbieHint) {
            *pSbieHint = (ULONG)'kuzt';
            GlobalUnlock(hSbieHint);
        }
    }

    //
    // set clipboard data and record paths
    //

    if (OpenClipboard()) {

        m_DontClearCutMarks = true;

        EmptyClipboard();
        SetClipboardData(CF_HDROP, hDropFiles);
        if (hMoveHint) {
            UINT fmt = RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
            SetClipboardData(fmt, hMoveHint);
        }
        if (hSbieHint) {
            UINT fmt = RegisterClipboardFormat(SANDBOXIE);
            SetClipboardData(fmt, hSbieHint);
        }
        CloseClipboard();

        while (! ItemsToRecover.IsEmpty()) {
            CString item = ItemsToRecover.RemoveHead();
            m_CutPaths.AddTail(item);
        }

        m_DontClearCutMarks = false;

    } else {

        GlobalFree(hDropFiles);
        if (hMoveHint)
            GlobalFree(hMoveHint);
        if (hSbieHint)
            GlobalFree(hSbieHint);
    }
}


//---------------------------------------------------------------------------
// OnCmdRecoverFileOrFolder
//---------------------------------------------------------------------------


void CFileListCtrl::OnCmdRecoverFileOrFolder()
{
    //
    // turn off selection on child items when the parent item is selected.
    // note that the child items are deleted
    //

    LVITEM lvi;
    memzero(&lvi, sizeof(LVITEM));
    lvi.mask = LVIF_INDENT | LVIF_IMAGE | LVIF_PARAM;

    int ListIndex = -1;
    while (1) {
        ListIndex = CListCtrl::GetNextItem(ListIndex, LVNI_SELECTED);
        if (ListIndex == -1)
            break;

        lvi.iItem = ListIndex;
        if (CListCtrl::GetItem(&lvi)) {

            bool isFolder =
                    (lvi.iImage == m_imgMinus || lvi.iImage == m_imgPlus);
            if (isFolder) {

                int oldIndent = lvi.iIndent;
                while (1) {

                    lvi.iItem = ListIndex + 1;
                    if (! CListCtrl::GetItem(&lvi))
                        break;
                    if (lvi.iIndent <= oldIndent)
                        break;

                    CMyListCtrl::MyDeleteItem(ListIndex + 1);
                }
            }
        }
    }

    //
    // extract each item that is still selected
    //

    lvi.mask = LVIF_IMAGE | LVIF_PARAM;

    CString PathToFocus;
    CStringList ItemsToRecover;

    while (1) {
        ListIndex = CListCtrl::GetNextItem(-1, LVNI_SELECTED);
        if (ListIndex == -1)
            break;

        lvi.iItem = ListIndex;
        if (! CListCtrl::GetItem(&lvi)) {
            lvi.iImage = 0;
            lvi.lParam = 0;
        }

        //
        //
        //

        CString CopyPath;
        if (IsPhysicalItem(ListIndex, NULL, NULL, &CopyPath)) {

            bool doAddItem = true;

            if ((! m_combo.IsWindowEnabled())) {

                bool isFolder =
                    (lvi.iImage == m_imgMinus || lvi.iImage == m_imgPlus);
                if (isFolder) {

                    CString RelativePath;
                    int index = CopyPath.ReverseFind(L'\\');
                    if (index != -1)
                        RelativePath = CopyPath.Mid(index);

                    AddItemsToRecoverRecursive(
                        ItemsToRecover, RelativePath,
                        *(CString *)lvi.lParam, CopyPath);
                    doAddItem = false;
                }
            }

            if (doAddItem)
                ItemsToRecover.AddTail(CopyPath);
        }

        //
        //
        //

        if (lvi.lParam && PathToFocus.IsEmpty())
            PathToFocus = *(CString *)lvi.lParam;

        ULONG flags = LVIS_SELECTED | LVIS_FOCUSED | LVIS_DROPHILITED;
        CListCtrl::SetItemState(ListIndex, LVIS_DROPHILITED, flags);
    }

    //
    // save all the folders which are expanded in the list
    //

    CStringList ExpandedFolders;

    int ListCount = CListCtrl::GetItemCount();
    ListIndex = 0;
    while (ListIndex < ListCount) {
        lvi.iItem = ListIndex;
        if (CListCtrl::GetItem(&lvi)
                                && lvi.lParam && lvi.iImage == m_imgMinus)
            ExpandedFolders.AddTail(*(CString *)lvi.lParam);
        ++ListIndex;
    }

    //
    // recover all extracted items
    //

    const MSG *msg = GetCurrentMessage();
    int qrMode =
        ((LOWORD(msg->wParam) == ID_FILE_RECOVER_ANY) ? QR_ANY : QR_SAME);

    if (ItemsToRecover.GetCount()) {

        CQuickRecover qr(GetParent(), m_BoxName, ItemsToRecover, qrMode);

        CBox &box = CBoxes::GetInstance().GetBox(m_BoxName);
        CBoxFile &boxfile = box.GetBoxFile();
        boxfile.RebuildSkeletonTree();

        while (! ExpandedFolders.IsEmpty()) {
            CString folder = ExpandedFolders.RemoveHead();
            if (boxfile.ChangeFolder(folder) != -1)
                boxfile.SetFolderExpandedView(TRUE);
        }
    }

    //
    //
    //

    while (1) {
        ListIndex = CListCtrl::GetNextItem(-1, LVNI_DROPHILITED);
        if (ListIndex == -1)
            break;
        CMyListCtrl::MyDeleteItem(ListIndex);
    }

    RebuildFiles();
    SetFocusByPath(PathToFocus);
}


//---------------------------------------------------------------------------
// OnCmdAddOrRemoveFolder
//---------------------------------------------------------------------------


void CFileListCtrl::OnCmdAddOrRemoveFolder()
{
    if (CListCtrl::GetSelectedCount() > 1)
        return;
    int index = CListCtrl::GetNextItem(-1, LVNI_SELECTED);
    if (index == -1)
        return;

    bool isFolder;
    CString TruePath;
    if (! IsPhysicalItem(index, &isFolder, &TruePath, NULL))
        return;

    const MSG *msg = GetCurrentMessage();
    BOOL add = (LOWORD(msg->wParam) == ID_FILE_RECOVER_ADD);

    CBox &box = CBoxes::GetInstance().GetBox(m_BoxName);
    box.AddOrRemoveQuickRecoveryFolder(TruePath, add);
    if (! add) {
        TruePath.TrimRight(L'\\');
        box.AddOrRemoveQuickRecoveryFolder(TruePath, add);
    }

    RebuildFiles();
}


//---------------------------------------------------------------------------
// AddItemsToRecoverRecursive
//---------------------------------------------------------------------------


void CFileListCtrl::AddItemsToRecoverRecursive(
    CStringList &ItemsToRecover, const CString &RelativePath,
    const CString &VirtualPath, const CString &CopyPath)
{
    CBox &box = CBoxes::GetInstance().GetBox(m_BoxName);
    CBoxFile &boxfile = box.GetBoxFile();

    if (boxfile.ChangeFolder(VirtualPath) == -1)
        return;

    POSITION pos = boxfile.GetFolderHeadPosition();
    while (pos) {
        ULONG64 bytes64;
        const WCHAR *file_name = boxfile.GetNextFile(pos, bytes64);
        if (! file_name)
            break;
        CString FileCopyPath = CopyPath + L"\\" + file_name;
        bool shouldShowFile = ShouldShowFileInQuickRecovery(
                                    file_name, bytes64, FileCopyPath);
        if (shouldShowFile) {
            if (! RelativePath.IsEmpty())
                FileCopyPath += L"|" + RelativePath;
            ItemsToRecover.AddTail(FileCopyPath);
        }
    }

    pos = boxfile.GetFolderHeadPosition();
    while (pos) {
        BOOL expanded;
        const WCHAR *folder_name = boxfile.GetNextFolder(pos, expanded);
        if (! folder_name)
            break;
        AddItemsToRecoverRecursive(
            ItemsToRecover,
            RelativePath + L"\\" + folder_name,
            VirtualPath + L"\\" + folder_name,
            CopyPath + L"\\" + folder_name);
    }
}


//---------------------------------------------------------------------------
// ShouldCompareFile
//---------------------------------------------------------------------------


bool CFileListCtrl::ShouldShowFileInQuickRecovery(
    const WCHAR *file_name, ULONG64 bytes64, const CString &FileCopyPath)
{
    //
    // files to ignore when displaying Quick Recovery dialog box
    //

    if (bytes64 == 0)
        return false;

    if (_wcsicmp(file_name, L"desktop.ini") == 0)
        return false;

    //
    // check if Microsoft Office document file
    //

    bool isOfficeFile = false;
    const WCHAR *dot = wcsrchr(file_name, L'.');
    if (dot) {
        ++dot;
        const WCHAR *_types[] = {
            L"doc", L"docx", L"ppt", L"pptx", L"xls", L"xlsx", L"rtf",
            NULL };
        const WCHAR **type = _types;
        while (*type) {
            if (_wcsicmp(dot, *type) == 0) {
                isOfficeFile = true;
                break;
            }
            ++type;
        }
    }

    //
    // ignore Microsoft Office temp files
    //

    if (isOfficeFile) {

        if (file_name[0] == L'~' && file_name[1] == L'$')
            return false;

        if (bytes64 <= 32 * 1024 * 1024 && CompareSameFiles(FileCopyPath))
            return false;
    }

    //
    // finish
    //

    return true;
}


//---------------------------------------------------------------------------
// CompareSameFiles
//---------------------------------------------------------------------------


bool CFileListCtrl::CompareSameFiles(const CString &CopyPath)
{
    bool same = false;

    CBox &box = CBoxes::GetInstance().GetBox(m_BoxName);
    CBoxFile &boxfile = box.GetBoxFile();

    CString TruePath = boxfile.GetTruePathForCopyPath(CopyPath);
    boxfile.TranslateNtToDosPath(TruePath);

    HANDLE hCopyFile = NULL;
    HANDLE hTrueFile = CreateFile(TruePath, FILE_GENERIC_READ, FILE_SHARE_READ,
                                  NULL, OPEN_EXISTING, 0, NULL);
    if (hTrueFile == INVALID_HANDLE_VALUE)
        hTrueFile = NULL;

    if (hTrueFile) {

        hCopyFile = CreateFile(CopyPath, FILE_GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, 0, NULL);
        if (hCopyFile == INVALID_HANDLE_VALUE)
            hCopyFile = NULL;
    }

    BOOL ok = TRUE;

    if (hTrueFile && hCopyFile) {

        LARGE_INTEGER TrueSize, CopySize;
        ok = GetFileSizeEx(hTrueFile, &TrueSize);
        if (ok)
            ok = GetFileSizeEx(hCopyFile, &CopySize);
        if (ok && TrueSize.QuadPart != CopySize.QuadPart)
            ok = FALSE;
    }

    if (ok) {

        const ULONG BufSize = 8 * 1024;

        void *TrueBuf = malloc(BufSize);
        void *CopyBuf = malloc(BufSize);
        if (TrueBuf && CopyBuf) {

            while (ok) {

                ULONG TrueCount, CopyCount;
                ok = ReadFile(hTrueFile, TrueBuf, BufSize, &TrueCount, NULL);
                if (ok) {
                    ok = ReadFile(
                            hCopyFile, CopyBuf, TrueCount, &CopyCount, NULL);
                    if (ok) {
                        if (TrueCount != CopyCount)
                            ok = FALSE;
                        else if (memcmp(TrueBuf, CopyBuf, CopyCount) != 0)
                            ok = FALSE;
                        else if (TrueCount < BufSize)
                            break;
                    }
                }
            }

            if (ok)
                same = true;
        }

        if (CopyBuf)
            free(CopyBuf);
        if (TrueBuf)
            free(TrueBuf);
    }

    if (hCopyFile)
        CloseHandle(hCopyFile);

    if (hTrueFile)
        CloseHandle(hTrueFile);

    return same;
}


//---------------------------------------------------------------------------
// InspectClipboard
//---------------------------------------------------------------------------


void CFileListCtrl::InspectClipboard()
{
    m_DontClearCutMarks = false;

    UINT fmt = RegisterClipboardFormat(SANDBOXIE);
    if (! IsClipboardFormatAvailable(fmt))
        return;

    HGLOBAL hDropFiles = NULL;
    DROPFILES *pDropFiles = NULL;
    WCHAR *pNextFile = NULL;
    SIZE_T hDropFiles_len = 0;

    OpenClipboard();
    if (IsClipboardFormatAvailable(fmt)) {
        hDropFiles = GetClipboardData(CF_HDROP);
        if (hDropFiles) {
            hDropFiles_len = GlobalSize(hDropFiles);
            pDropFiles = (DROPFILES *)GlobalLock(hDropFiles);
            if (pDropFiles && pDropFiles->fWide) {
                pNextFile =
                    (WCHAR *)(pDropFiles->pFiles + (ULONG_PTR)pDropFiles);
            }
        }
    }

    while (pNextFile && *pNextFile) {
        m_CutPaths.AddTail(pNextFile);
        pNextFile += wcslen(pNextFile) + 1;
        if (((ULONG_PTR)pNextFile - (ULONG_PTR)pDropFiles) >= hDropFiles_len)
            break;
    }

    if (pDropFiles)
        GlobalUnlock(hDropFiles);
    CloseClipboard();
}
