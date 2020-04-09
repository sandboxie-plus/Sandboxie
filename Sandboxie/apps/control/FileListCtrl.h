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
// File Tree Control
//---------------------------------------------------------------------------


#ifndef _MY_FILE_TREE_CTRL_H
#define _MY_FILE_TREE_CTRL_H


#include "MyListCtrl.h"


class CFileListCtrl : public CMyListCtrl
{
    DECLARE_MESSAGE_MAP()

    //
    //
    //

    CString m_BoxName;
    CString m_SaveBoxName;
    CStringList m_CutPaths;
    int m_anyQuickRecoverFiles;
    int m_RefreshCounter;
    bool m_DontClearCutMarks;

    //
    //
    //

    virtual afx_msg void OnSize(UINT nType, int cx, int cy);

    void UpdateScrollBar();

    virtual afx_msg void OnComboSelect();

    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

    void OnArrowKeyDown(UINT nChar);

    void MyEnsureVisible(int index);

    afx_msg void OnDestroyClipboard();

    void RebuildCombo();
    void RebuildFiles();

    int InsertFolder(CBoxFile &boxfile, const CString &_boxpath,
                     int ListIndex, int ListIndent);
    void InsertFile(const CString &VirtualPath,
                    const CString &FolderCopyPath, ULONG64 bytes64,
                    int ListIndex, int ListIndent);
    void RemoveFolderItems(int ListIndex, int ListIndent);

    void SetFocusByPath(CString SaveItemPath);

    virtual afx_msg void OnRefreshPosted();

    virtual afx_msg void OnClick(NMHDR *pNMHDR, LRESULT *pResult);

    bool IsPhysicalItem(int ListIndex, bool *pIsFolder,
                        CString *pTruePath, CString *pCopyPath);

    bool IsCutPath(const CString &CopyPath);

    bool OnContextMenuMultiple(CMenu *pMenu, CPoint pt);
    bool OnContextMenuSingle(int index, CMenu *pMenu, CPoint pt);

    afx_msg void OnCmdRun();
    afx_msg void OnCmdCreateShortcut();
    afx_msg void OnCmdCut();
    afx_msg void OnCmdRecoverFileOrFolder();
    afx_msg void OnCmdAddOrRemoveFolder();

    void AddItemsToRecoverRecursive(
        CStringList &ItemsToRecover, const CString &RelativePath,
        const CString &VirtualPath, const CString &CopyPath);

    bool ShouldShowFileInQuickRecovery(
        const WCHAR *file_name, ULONG64 bytes64,
        const CString &FileCopyPath);
    bool CompareSameFiles(const CString &CopyPath);

    void InspectClipboard();

public:

    void SetBoxNameForDialogMode(const WCHAR *BoxName);
    const CString &GetBoxNameForDialogMode() const;
    bool AnyQuickRecoverFiles() const;

    BOOL Create(CWnd *pParentWnd);

    virtual void RefreshContent();

    virtual CBox &GetSelectedBox(CWnd *pWnd, CPoint pt) const;
    virtual void GetSelectedItemPosition(CPoint &pt) const;
    void OnContextMenu(CWnd *pWnd, CPoint pt);

    void SelectAllItems(bool select);

};


#endif // _MY_FILE_TREE_CTRL_H
