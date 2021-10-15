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
// Delete Sandbox Dialog Box
//---------------------------------------------------------------------------

#include "stdafx.h"
#include "MyApp.h"
#include "DeleteDialog.h"

#include "Boxes.h"
#include "UserSettings.h"
#include "apps/common/MyGdi.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define ID_TIMER    1002


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CDeleteDialog, CBaseDialog)

    ON_WM_TIMER()

    ON_COMMAND(ID_RECOVER_SAME,         OnCmdRecoverSameAny)
    ON_COMMAND(ID_RECOVER_ANY,          OnCmdRecoverSameAny)
    ON_COMMAND(ID_RECOVER_ADD,          OnCmdAddFolder)

    ON_COMMAND(ID_RECOVER_SELECT_ALL,   OnSelectAll)

    ON_WM_CONTEXTMENU()

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CDeleteDialog::CDeleteDialog(
    CWnd *pParentWnd, const WCHAR *BoxName,
    BOOL WithDelete, BOOL AutoDelete,
    BOOL *DeleteSpawned)
    : CBaseDialog(pParentWnd, L"DELETE_DIALOG")
{
    m_WithDelete = WithDelete;
    m_AutoDelete = AutoDelete;
    m_filelist.SetBoxNameForDialogMode(BoxName);

    //
    //
    //

    m_empty = TRUE;
    m_files = 0;
    m_folders = 0;
    m_size = 0;

    m_spawned = FALSE;
    *DeleteSpawned = FALSE;

    CBox &box = CBoxes::GetInstance().GetBox(BoxName);
    CBoxFile &boxfile = box.GetBoxFile();

    if (m_WithDelete) {

        if (box.GetNeverDelete()) {

            CMyMsg msg(MSG_3051, BoxName);
            CMyApp::MsgBox(pParentWnd, msg, MB_OK);
            return;
        }

        boxfile.RebuildSkeletonTree();

        boxfile.ChangeFolder(L"\\");

        POSITION pos = boxfile.GetFolderHeadPosition();
        BOOL expanded;
        const WCHAR *folder_name = boxfile.GetNextFolder(pos, expanded);
        folder_name = boxfile.GetNextFolder(pos, expanded);

        m_stack.AddHead(CString(L"\\") + folder_name);
    }

    //
    // display dialog
    //

    DoModal();

    //
    // if empty box, display message
    //

    if (m_WithDelete && m_empty && ! m_AutoDelete) {

        CMyApp::MsgBox(pParentWnd, MSG_3730, MB_OK);
    }

    *DeleteSpawned = m_spawned;
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CDeleteDialog::~CDeleteDialog()
{
}


//---------------------------------------------------------------------------
// OnInitDialog
//---------------------------------------------------------------------------


BOOL CDeleteDialog::OnInitDialog()
{
    //
    // set title according to dialog mode
    //

    CString title = CMyMsg(MSG_3711);
    if (m_WithDelete)
        title = title.Mid(0, title.Find('|'));
    else
        title = title.Mid(title.Find('|') + 1);
    if (CMyApp::m_LayoutRTL)
        title = m_filelist.GetBoxNameForDialogMode() + L" - " + title;
    title += L" - " + m_filelist.GetBoxNameForDialogMode();
    SetWindowText(title);

    //
    // set dialog text
    //

    GetDlgItem(ID_DELETE_EXPLAIN_1)->SetWindowText(CMyMsg(MSG_3712));
    GetDlgItem(ID_DELETE_EXPLAIN_2)->SetWindowText(CMyMsg(MSG_3713));

    GetDlgItem(ID_RECOVER_SAME)->SetWindowText(CMyMsg(MSG_3720));
    GetDlgItem(ID_RECOVER_ANY)->SetWindowText(CMyMsg(MSG_3721));

    GetDlgItem(ID_RECOVER_ADD)->SetWindowText(CMyMsg(MSG_3358));

    GetDlgItem(IDOK)->SetWindowText(CMyMsg(MSG_3714));
    GetDlgItem(IDCANCEL)->SetWindowText(CMyMsg(MSG_3004));
    GetDlgItem(IDCLOSE)->SetWindowText(CMyMsg(MSG_3004));

    //
    //
    //

    CWnd *pCtl = GetDlgItem(ID_RECOVER_TREE);
    CRect rc;
    pCtl->GetWindowRect(&rc);
    ScreenToClient(&rc);

    m_filelist.Create(this);
    m_filelist.SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(),
                            SWP_NOZORDER | SWP_NOOWNERZORDER);
    m_filelist.ShowWindow(SW_SHOW);

    m_filelist.RefreshContent();
    m_filelist.SetFocus();

    //
    // prepare the select all button
    //

    static HBITMAP checkmark_bitmap = NULL;
    if (! checkmark_bitmap)
        checkmark_bitmap = MyGdi_CreateFromResource(L"BLUE_CHECKMARK");

    CButton *pButton = (CButton *)GetDlgItem(ID_RECOVER_SELECT_ALL);
    pButton->ModifyStyle(0, BS_BITMAP | BS_CENTER);
    pButton->SetBitmap(checkmark_bitmap);

    rc.top += 2;
    rc.bottom = rc.top + GetSystemMetrics(SM_CYVTHUMB) + 10;

    if (! CMyApp::m_LayoutRTL) {
        rc.right -= GetSystemMetrics(SM_CXHTHUMB) + 5;
        rc.left = rc.right - 25;
    } else {
        rc.left += GetSystemMetrics(SM_CXHTHUMB) + 5;
        rc.right = rc.left + 25;
        pButton->ModifyStyleEx(WS_EX_LAYOUTRTL, 0);
    }

    pButton->SetWindowPos(&wndTop,
        rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
        SWP_SHOWWINDOW);

    if (! SelectAllButton.m_hWnd) {
        CString tip = CMyMsg(MSG_3360);
        tip.Remove(L'&');
        SelectAllButton.Init(pButton->m_hWnd, tip);
    }

    if (! m_filelist.AnyQuickRecoverFiles())
        pButton->ShowWindow(SW_HIDE);

    //
    // hide buttons accoording to dialog mode
    //

    pCtl = GetDlgItem(IDCLOSE);

    if (m_WithDelete) {

        pCtl->ShowWindow(SW_HIDE);

    } else {

        pCtl->SetDlgCtrlID(IDCANCEL);
        //SetWindowLongPtr(pCtl->m_hWnd, GWLP_ID, IDCANCEL);

        pCtl = GetDlgItem(ID_RECOVER_GROUPBOX);
        pCtl->GetClientRect(&rc);
        int group_height = rc.Height();

        pCtl->ShowWindow(SW_HIDE);

        GetWindowRect(&rc);
        rc.bottom -= group_height;

        SetWindowPos(
            NULL, 0, 0, rc.Width(), rc.Height(),
            SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
    }

    //
    // start timer (to count contents) if manual delete mode
    //

    if (m_WithDelete) {

        OnTimer(ID_TIMER);

        if (m_empty) {
            EndDialog(0);
            return FALSE;
        }

        if (m_AutoDelete) {

            if (! m_filelist.AnyQuickRecoverFiles()) {
                OnOK();
                return FALSE;
            }

            m_DeleteButton.Init(this, IDOK);

            FlashTitle();
        }

        SetTimer(ID_TIMER, 10, NULL);
    }

    AddMinimizeButton();

    return FALSE;
}


//---------------------------------------------------------------------------
// OnTimer
//---------------------------------------------------------------------------


void CDeleteDialog::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent != ID_TIMER)
        return;

    if (m_stack.IsEmpty())
        return;

    const CString &BoxName = m_filelist.GetBoxNameForDialogMode();
    CBox &box = CBoxes::GetInstance().GetBox(BoxName);
    CBoxFile &boxfile = box.GetBoxFile();

    CString path = m_stack.RemoveHead();
    boxfile.SetIncludeDeleted(TRUE);
    if (boxfile.ChangeFolder(path, TRUE) == -1) {
        boxfile.SetIncludeDeleted(FALSE);
        return;
    }

    if (boxfile.GetFolderNumChildren() <= 0) {
        boxfile.SetIncludeDeleted(FALSE);
        return;
    }

    m_empty = FALSE;

    if (boxfile.IsPhysicalFolder())
        ++m_folders;

    POSITION pos = boxfile.GetFolderHeadPosition();
    while (pos) {
        BOOL expanded;
        const WCHAR *folder_name = boxfile.GetNextFolder(pos, expanded);
        if (! folder_name)
            break;
        m_stack.AddHead(path + L"\\" + folder_name);
    }

    pos = boxfile.GetFolderHeadPosition();
    while (pos) {
        ULONG64 size;
        const WCHAR *file_name = boxfile.GetNextFile(pos, size);
        if (! file_name)
            break;
        m_size += size;
        ++m_files;
    }

    boxfile.SetIncludeDeleted(FALSE);

    //
    // stringfy number of files.  note that if we have any files at all,
    // we will assume that the box also contains two RegHive files
    //

    CString strFiles;
    strFiles.Format(L"%d", m_files);

    CString strFolders;
    strFolders.Format(L"%d", m_folders);

    const WCHAR *units = L"";
    ULONG64 bytes64 = m_size;
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

    CString strBytes;
    if (CMyApp::m_LayoutRTL) {
        const ULONG U_LRO = 0x202D;   // Start of left-to-right override
        const ULONG U_PDF = 0x202C;   // Pop directional formatting
        strBytes.Format(L"%c%d %s%c", U_LRO, (ULONG)bytes64, units, U_PDF);
    } else
        strBytes.Format(L"%d %s", (ULONG)bytes64, units);

    CMyMsg str(MSG_3722, strFiles, strFolders, strBytes);
    SetDlgItemText(ID_DELETE_SIZE, str);
}


//---------------------------------------------------------------------------
// OnOK
//---------------------------------------------------------------------------


void CDeleteDialog::OnOK()
{
    if (! m_WithDelete)
        return;

    while (! m_stack.IsEmpty())
        m_stack.RemoveHead();

    const CString &BoxName = m_filelist.GetBoxNameForDialogMode();
    CBox &box = CBoxes::GetInstance().GetBox(BoxName);
    CBoxProc &boxproc = box.GetBoxProc();

    CWaitCursor waitcursor;

    SbieDll_KillAll(-1, BoxName);

    int i = 0;
    for (i = 0; i < 20; ++i) {
        boxproc.RefreshProcesses();
        if (boxproc.GetProcessCount() == 0) {
            Sleep(500);
            break;
        }
        Sleep(250);
    }

    if (i == 20) {

        CMyApp::MsgBox(this, MSG_3731, MB_OK);

    } else {

        WCHAR cmd[64];
        wcscpy(cmd, L"delete_sandbox");
        //if (m_AutoDelete)
                //wcscat(cmd, L"_silent");
        CMyApp::RunStartExe(cmd, BoxName, TRUE);

        box.GetBoxFile().RebuildSkeletonTree();

        m_spawned = TRUE;
    }

    EndDialog(0);
}


//---------------------------------------------------------------------------
// OnCmdRecoverSame
//---------------------------------------------------------------------------


void CDeleteDialog::OnCmdRecoverSameAny()
{
    int cmd = 0;
    const MSG *msg = GetCurrentMessage();
    if (LOWORD(msg->wParam) == ID_RECOVER_SAME)
        cmd = ID_FILE_RECOVER_SAME;
    else if (LOWORD(msg->wParam) == ID_RECOVER_ANY)
        cmd = ID_FILE_RECOVER_ANY;
    if (cmd)
        m_filelist.PostMessage(WM_COMMAND, cmd, 0);
}


//---------------------------------------------------------------------------
// OnCmdAddFolder
//---------------------------------------------------------------------------


void CDeleteDialog::OnCmdAddFolder()
{
    WCHAR path[MAX_PATH + 32];

    CMyMsg title(MSG_3726);

    BROWSEINFO bi;
    memzero(&bi, sizeof(BROWSEINFO));
    bi.hwndOwner = m_hWnd;
    bi.pszDisplayName = path;
    bi.lpszTitle = title;
    bi.ulFlags = BIF_UAHINT | BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl) {

        BOOL ok = SHGetPathFromIDList(pidl, path);
        CoTaskMemFree(pidl);
        if (ok) {

            //
            // add the new folder
            //

            const CString &BoxName = m_filelist.GetBoxNameForDialogMode();
            CBox &box = CBoxes::GetInstance().GetBox(BoxName);
            CBoxFile &boxfile = box.GetBoxFile();

            ok = box.AddOrRemoveQuickRecoveryFolder(path, TRUE);
            if (! ok)
                return;

            m_filelist.RefreshContent();

            if (m_filelist.AnyQuickRecoverFiles())
                GetDlgItem(ID_RECOVER_SELECT_ALL)->ShowWindow(SW_SHOW);

            //
            // check if the folder is empty
            //

            BOOL empty = TRUE;

            boxfile.ChangeFolder(L"\\");
            POSITION pos = boxfile.GetFolderHeadPosition();
            BOOL expanded;
            const WCHAR *folder_name = boxfile.GetNextFolder(pos, expanded);
            if (boxfile.ChangeFolder(folder_name) != -1) {
                pos = boxfile.GetFolderHeadPosition();
                while (pos) {
                    folder_name = boxfile.GetNextFolder(pos, expanded);
                    if (boxfile.ChangeFolder(folder_name) != -1) {
                        CString TruePath, CopyPath;
                        boxfile.GetFolderPaths(TruePath, CopyPath);
                        if (TruePath == path) {
                            empty = (boxfile.GetFolderNumChildren() == 0);
                            break;
                        }
                        boxfile.ChangeFolder(L"..");
                    }
                }
            }

            if (empty)
                CMyApp::MsgBox(this, MSG_3727, MB_OK);
        }
    }
}


//---------------------------------------------------------------------------
// OnSelectAll
//---------------------------------------------------------------------------


void CDeleteDialog::OnSelectAll()
{
    m_filelist.SelectAllItems(true);
    m_filelist.SetFocus();
}


//---------------------------------------------------------------------------
// OnContextMenu
//---------------------------------------------------------------------------


void CDeleteDialog::OnContextMenu(CWnd *pWnd, CPoint pt)
{
    if (pWnd == &m_filelist)
        m_filelist.OnContextMenu(pWnd, pt);
}
