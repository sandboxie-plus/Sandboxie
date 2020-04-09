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
// Quick Recover
//---------------------------------------------------------------------------


#include "stdafx.h"
#include "QuickRecover.h"

#include <shlobj.h>
#include "common/win32_ntddk.h"
#include "UserSettings.h"
#include "Boxes.h"
#include "apps/common/MyGdi.h"
#include "apps/common/CommonUtils.h"


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static const CString _RecoverTarget(L"RecoverTarget");
static const CString _SaveRecoverTargets(L"SaveRecoverTargets");

static const CString _LongPathPrefix(L"\\\\?\\");

ULONG CQuickRecover::m_TempFolderCounter = 0;

CStringList CQuickRecover::m_past_files;

const WCHAR *CQuickRecover::ReplaceButtonText = NULL;


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CQuickRecover, CBaseDialog)

    ON_WM_VKEYTOITEM()
    ON_CONTROL(LBN_SELCHANGE,   ID_RECOVER_FOLDERS, OnListSelect)
    ON_CONTROL(LBN_DBLCLK,      ID_RECOVER_FOLDERS, OnListSelect)
    ON_COMMAND(ID_RECOVER_SELECT_ALL,               OnSelectAll)
    ON_COMMAND(ID_RECOVER_DISABLE,                  OnCheckBoxClick)
    ON_COMMAND(ID_RECOVER_REMOVE,                   OnRemoveFolders)
    ON_COMMAND(ID_RECOVER_CYCLE,                    OnCycleRecover)
    ON_WM_CONTEXTMENU()
    ON_WM_DESTROY()
    ON_WM_ACTIVATE()

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CQuickRecover::CQuickRecover(
    CWnd *pParentWnd, const CString &BoxName,
    const CString &SrcPath, int qrMode)
    : CBaseDialog(pParentWnd)
{
    m_BoxName = BoxName;
    m_SrcPath = SrcPath;

    m_ready = FALSE;
    m_log = FALSE;
    m_replace = FALSE;
    m_recover_mode = 0;

    if (qrMode == QR_SAME)
        RecoverToSameFolder();
    else if (qrMode == QR_ANY)
        RecoverToAnyFolder();
    else if (qrMode == QR_AUTO)
        AutoRecover();
    else if (qrMode == QR_LOG)
        RecoveryLog();
}


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------

CQuickRecover::CQuickRecover(
    CWnd *pParentWnd, const CString &BoxName,
    CStringList &SrcPaths, int qrMode)
    : CBaseDialog(pParentWnd)
{
    m_BoxName = BoxName;

    m_ready = FALSE;
    m_log = FALSE;
    m_replace = FALSE;
    m_recover_mode = 0;

    bool first = true;
    BOOL ok;
    CString SaveDstPath;

    POSITION pos = SrcPaths.GetHeadPosition();
    while (pos) {

        CString RelativePath;
        m_SrcPath = SrcPaths.GetNext(pos);
        int index = m_SrcPath.ReverseFind(L'|');
        if (index != -1) {
            RelativePath = m_SrcPath.Mid(index + 1);
            m_SrcPath = m_SrcPath.Left(index);
        }

        if (qrMode == QR_SAME)
            ok = RecoverToSameFolder();
        else if (first) {
            ok = RecoverToAnyFolder(RelativePath);
            SaveDstPath = m_DstPath;
            first = false;
        } else
            ok = RecoverFile(GetDestPlusRelative(SaveDstPath, RelativePath));
        if ((! ok) || m_DstPath.IsEmpty())
            break;
    }
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CQuickRecover::~CQuickRecover()
{
}


//---------------------------------------------------------------------------
// OnDestroy
//---------------------------------------------------------------------------


void CQuickRecover::OnDestroy()
{
    if (m_static.m_hWnd)
        m_static.UnsubclassWindow();
    if (m_listbox.m_hWnd)
        m_listbox.UnsubclassWindow();
    if (m_checkbox.m_hWnd)
        m_checkbox.UnsubclassWindow();
}


//---------------------------------------------------------------------------
// OnListSelect
//---------------------------------------------------------------------------


void CQuickRecover::OnListSelect()
{
    if (m_ready) {

        POSITION pos = 0;
        int index = m_listbox.GetCurSel();
        if (index != LB_ERR)
            pos = m_list.FindIndex(index);
        if (pos) {

            if (m_hwndTree) {

                //
                // SHBrowseForFolder dialog
                //

                CString &path = m_list.GetAt(pos);
                SendMessage(
                    BFFM_SETSELECTION, TRUE, (LPARAM)(const WCHAR *)path);

                const MSG *msg = GetCurrentMessage();
                if (HIWORD(msg->wParam) == LBN_DBLCLK)
                    SendMessage(WM_COMMAND, IDOK, 0);
                else
                    ::SetFocus(m_hwndTree);

            } else {

                //
                // Immediate Recovery dialog
                //

                if (m_log)
                    GetDlgItem(IDOK)->EnableWindow(TRUE);

                const MSG *msg = GetCurrentMessage();
                if (HIWORD(msg->wParam) == LBN_DBLCLK && (! m_check))
                    SendMessage(WM_COMMAND, IDOK, 0);
            }
        }
    }
}


//---------------------------------------------------------------------------
// OnVKeyToItem
//---------------------------------------------------------------------------


int CQuickRecover::OnVKeyToItem(UINT nKey, CListBox *pListBox, UINT nIndex)
{
    if (m_log || m_hwndTree || pListBox->GetDlgCtrlID() != ID_RECOVER_ITEMS)
        return -1;
    if (nKey != L'A' || (GetKeyState(VK_CONTROL) & 0x8000) == 0)
        return -1;

    pListBox->SelItemRange(TRUE, 0, pListBox->GetCount());
    return -2;
}


//---------------------------------------------------------------------------
// OnSelectAll
//---------------------------------------------------------------------------


void CQuickRecover::OnSelectAll()
{
    CListBox *pListBox = (CListBox *)GetDlgItem(ID_RECOVER_ITEMS);
    pListBox->SelItemRange(TRUE, 0, pListBox->GetCount());
}


//---------------------------------------------------------------------------
// OnContextMenu
//---------------------------------------------------------------------------


void CQuickRecover::OnContextMenu(CWnd *pWnd, CPoint pt)
{
    //
    // display a context menu to remove items from the list of folders
    //

    if (m_hwndTree)
        return;
    if (pWnd->GetDlgCtrlID() != ID_RECOVER_FOLDERS)
        return;
    CListBox *pListBox = (CListBox *)GetDlgItem(ID_RECOVER_FOLDERS);

    CPoint ptListBox = pt;
    pListBox->ScreenToClient(&ptListBox);

    BOOL outside = FALSE;
    ULONG FolderIndex = pListBox->ItemFromPoint(ptListBox, outside);
    if (outside)
        return;
    pListBox->SetCurSel(FolderIndex);
    if (m_hwndTree)
        OnListSelect();

    ULONG gray = 0;
    if ((! m_hwndTree) && (FolderIndex <= 1))
        gray = MF_GRAYED;

    CString msg3981 = CMyMsg(MSG_3981);
    msg3981.Remove(L'&');

    CMenu menu;
    menu.CreatePopupMenu();
    menu.AppendMenu(MF_STRING | gray, 3981, msg3981);
    menu.AppendMenu(MF_STRING | gray, 3982, CMyMsg(MSG_3982));
    menu.AppendMenu(MF_STRING,        3983, CMyMsg(MSG_3983));
    ULONG menucmd = menu.TrackPopupMenu(
        TPM_NONOTIFY | TPM_RETURNCMD | TPM_TOPALIGN, pt.x, pt.y, this);

    //
    // process menu selection
    //

    bool UpdateList = false;

    if (menucmd == 3981) {

        CString path = m_list.GetAt(m_list.FindIndex(FolderIndex));
        ShellExecute(m_hWnd, NULL, path, NULL, NULL, SW_SHOWNORMAL);

    } else if (menucmd == 3982) {

        //
        // delete the selected item
        //

        POSITION pos = m_list.FindIndex(FolderIndex);
        m_list.RemoveAt(pos);
        pListBox->DeleteString(FolderIndex);
        UpdateList = true;

    } else if (menucmd == 3983) {

        //
        // delete everything, except the items Recovery to Same/Any Folder
        //

        ULONG FolderCount = (m_hwndTree) ? 0 : 2;
        while (m_list.GetCount() != FolderCount) {
            POSITION pos = m_list.FindIndex(FolderCount);
            m_list.RemoveAt(pos);
            pListBox->DeleteString(FolderCount);
        }
        UpdateList = true;
    }

    //
    // store the new list, except the items Recovery Same/Any Folder
    //

    if (UpdateList) {

        if (! gray)
            pListBox->SetCurSel(0);

        CUserSettings &ini = CUserSettings::GetInstance();

        POSITION pos = m_list.GetHeadPosition();
        if (! m_hwndTree) {
            m_list.GetNext(pos);    // Recover to Same Folder
            m_list.GetNext(pos);    // Recover to Any Folder
        }

        CString value;
        while (pos) {
            if (! value.IsEmpty())
                value += L"\n";
            value += m_list.GetNext(pos);
        }

        ini.SetText(_RecoverTarget, value);

        int FolderCount = (m_hwndTree) ? 0 : 2;
        GetDlgItem(ID_RECOVER_RIGHT_CLICK)->ShowWindow(
            (m_list.GetCount() > FolderCount) ? SW_SHOW : SW_HIDE);
    }
}


//---------------------------------------------------------------------------
// OnCheckBoxClick
//---------------------------------------------------------------------------


void CQuickRecover::OnCheckBoxClick()
{
    m_check = ! m_check;

    CButton *pCheckBox = (CButton *)GetDlgItem(ID_RECOVER_DISABLE);
    pCheckBox->SetCheck(m_check ? BST_CHECKED : BST_UNCHECKED);

    if (! m_hwndTree) {
        GetDlgItem(IDOK)->EnableWindow(! m_check);
        CBox &box = CBoxes::GetInstance().GetBox(m_BoxName);
        box.SetImmediateRecoveryState(! m_check, TRUE);
    }
}


//---------------------------------------------------------------------------
// OnCycleRecover
//---------------------------------------------------------------------------


void CQuickRecover::OnCycleRecover()
{
    ULONG msgid1, msgid2;
    ++m_recover_mode;
    if (m_recover_mode == 3) {
        msgid1 = 3718;
        msgid2 = 3736;
        m_recover_mode = 0;
    } else if (m_recover_mode == 2) {
        msgid1 = 3737;
        msgid2 = 3718;
    } else if (m_recover_mode == 1) {
        msgid1 = 3736;
        msgid2 = 3737;
    }
    GetDlgItem(IDOK)->SetWindowText(CMyMsg(msgid1));
    if (CycleRecoverButton.m_hWnd)
        CycleRecoverButton.SetText(CMyMsg(msgid2));
}


//---------------------------------------------------------------------------
// InitFolderList
//---------------------------------------------------------------------------


void CQuickRecover::InitFolderList(bool AddHead, int SelectIndex)
{
    //
    // ReadFolderList
    //

    m_list.RemoveAll();
    m_listbox.ResetContent();

    CUserSettings &ini = CUserSettings::GetInstance();
    ini.GetBool(_SaveRecoverTargets, m_check, TRUE);
    if (m_check) {
        ini.GetTextList(_RecoverTarget, m_list);
        while (m_list.GetCount() > 10)
            m_list.RemoveTail();
    }

    if (AddHead) {

        AddHeadFolderList(MSG_3721);
        AddHeadFolderList(MSG_3720);
    }

    //
    // ShowFolderList
    //

    POSITION pos = m_list.GetHeadPosition();
    while (pos) {
        CString text = m_list.GetNext(pos);
        int index = 0;
        while (text.GetLength() - index > 45) {
            int index2 = text.Find(L'\\', index);
            if (index2 == -1)
                break;
            index = index2 + 1;
        }
        if (index)
            text = CString(L"...\\") + text.Mid(index);
        m_listbox.AddString(text);
    }

    m_listbox.SetCurSel(SelectIndex);
}


//---------------------------------------------------------------------------
// AddHeadFolderList
//---------------------------------------------------------------------------


void CQuickRecover::AddHeadFolderList(ULONG msgid)
{
    CString text = CMyMsg(msgid);

    int index = text.Find(L'(');
    if (index != -1)
        text = text.Left(index);
    text.Remove(L'&');
    text.Remove(L'\r');
    text.Replace(L'\n', L' ');
    text.TrimLeft();
    text.TrimRight();

    m_list.AddHead(text);
}


//---------------------------------------------------------------------------
// GetDestPlusRelative
//---------------------------------------------------------------------------


CString CQuickRecover::GetDestPlusRelative(
    const CString &DestPath, const CString &RelativePath)
{
    //
    // combine destination and relative paths.  but if the
    //

    CString Result = DestPath;

    if (RelativePath.GetLength() > 1) {

        int dIndex = DestPath.ReverseFind(L'\\');
        if (dIndex != -1) {

            int rIndex = RelativePath.Find(L'\\', 1);
            if (rIndex == -1)
                rIndex = RelativePath.GetLength();

            CString LastPart = DestPath.Mid(dIndex);
            CString FirstPart = RelativePath.Left(rIndex);

            if (FirstPart.CompareNoCase(LastPart) == 0) {

                Result = DestPath.Left(dIndex);
            }
        }

        Result += RelativePath;

        SHCreateDirectory(m_pParentWnd->m_hWnd, Result);
    }

    return Result;
}


//---------------------------------------------------------------------------
// RecoverToAnyFolder
//---------------------------------------------------------------------------


BOOL CQuickRecover::RecoverToAnyFolder(const CString &RelativePath)
{
    BOOL ok = FALSE;

    //
    // invoke Browse For Folder dialog box
    //

    WCHAR path[MAX_PATH + 32];

    CMyMsg title(MSG_3723);

    BROWSEINFO bi;
    memzero(&bi, sizeof(BROWSEINFO));
    bi.hwndOwner = m_pParentWnd->m_hWnd;
    bi.pszDisplayName = path;
    bi.lpszTitle = title;
    bi.ulFlags = BIF_RETURNONLYFSDIRS;
    bi.lpfn = BrowseCallback;
    bi.lParam = (LPARAM)this;
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl) {

        ok = SHGetPathFromIDList(pidl, path);
        CoTaskMemFree(pidl);

        //
        // update list of stored folders
        //

        if (ok) {

            CUserSettings &ini = CUserSettings::GetInstance();

            if (m_check) {

                CString value = path;
                while (! m_list.IsEmpty()) {
                    CString add = m_list.RemoveHead();
                    if (add.CompareNoCase(path) != 0)
                        value += L"\n" + add;
                }
                ini.SetText(_RecoverTarget, value);
            }

            ini.SetBool(_SaveRecoverTargets, m_check);

            ok = RecoverFile(GetDestPlusRelative(path, RelativePath));
        }

        if (ok)
            m_DstPath = path;
    }

    if (! ok)
        m_DstPath = CString();

    return ok;
}


//---------------------------------------------------------------------------
// BrowseCallback
//---------------------------------------------------------------------------


int CALLBACK CQuickRecover::BrowseCallback(
    HWND hwnd, UINT msg, LPARAM lParam, LPARAM lpData)
{
    if (msg == BFFM_INITIALIZED) {
        CQuickRecover *pThis = (CQuickRecover *)lpData;
        pThis->SubclassWindow(hwnd);
        pThis->OnInitDialogAnyFolder();
    }
    return 0;
}


//---------------------------------------------------------------------------
// OnInitDialogAnyFolder
//---------------------------------------------------------------------------


void CQuickRecover::OnInitDialogAnyFolder()
{
    //
    // add box name in title bar
    //

    CString caption;
    GetWindowText(caption);
    caption += L" - " + m_BoxName;
    SetWindowText(caption);

    //
    // remove context help button from dialog caption
    //

    ModifyStyle(DS_CONTEXTHELP, 0);
    ModifyStyleEx(WS_EX_CONTEXTHELP, 0);

    //
    // find the folder tree and make dialog larger
    //

    //m_hwndTree = ::FindWindowEx(m_hWnd, NULL, L"SHBrowseForFolder ShellNameSpace Control", NULL);
    m_hwndTree = ::FindWindowEx(m_hWnd, NULL, L"SysTreeView32", NULL);
    if (! m_hwndTree)
        return;

    //
    // add static text below the folder tree
    //

    CRect rc, rc2;

    ::GetWindowRect(m_hwndTree, &rc);
    ScreenToClient(&rc);
    ::GetWindowRect(GetDlgItem(IDOK)->m_hWnd, &rc2);
    ScreenToClient(&rc2);
    rc.top = rc2.bottom + 10;
    rc.bottom = rc.top + 25;

    m_static.Create(CMyMsg(MSG_3724), SS_SIMPLE | WS_CHILD | WS_VISIBLE,
                    rc, this, IDC_STATIC);
    m_static.SetFont(GetFont());

    //
    // add listbox below the static text
    //

    rc.top += 30;
    rc.bottom = rc.top + 140;

    m_listbox.Create(WS_TABSTOP | WS_BORDER | WS_CHILD | WS_VISIBLE |
                     WS_HSCROLL | WS_VSCROLL | LBS_NOTIFY,
                     rc, this, ID_RECOVER_FOLDERS);
    m_listbox.ModifyStyleEx(0, WS_EX_CLIENTEDGE);
    m_listbox.SetFont(GetFont());

    m_listbox.SetItemHeight(0, m_listbox.GetItemHeight(0) * 3 / 2);

    InitFolderList(false, -1);

    //
    // add checkbox below the listbox
    //

    m_listbox.GetWindowRect(&rc);
    ScreenToClient(&rc);
    rc.top = rc.bottom + 10;
    rc.bottom = rc.top + 30;

    m_checkbox.Create(CMyMsg(MSG_3725),
                      BS_CHECKBOX | WS_TABSTOP | WS_CHILD | WS_VISIBLE,
                      rc, this, ID_RECOVER_DISABLE);
    m_checkbox.SetFont(GetFont());

    if (m_check)
        m_checkbox.SetCheck(BST_CHECKED);

    //
    // resize dialog vertically
    //

    GetWindowRect(&rc2);
    ScreenToClient(&rc2);
    rc2.bottom = rc.bottom + 10;
    ClientToScreen(&rc2);
    MoveWindow(rc2, TRUE);

    //
    // enable list interaction
    //

    m_ready = TRUE;
}


//---------------------------------------------------------------------------
// AutoRecover
//---------------------------------------------------------------------------


void CQuickRecover::AutoRecover()
{
    SetDialogTemplate(L"AUTORECOVER_DIALOG");
}


//---------------------------------------------------------------------------
// AddAutoRecoverItem
//---------------------------------------------------------------------------


void CQuickRecover::AddAutoRecoverItem(const CString &SrcPath)
{
    CBox &box = CBoxes::GetInstance().GetBox(m_BoxName);
    CBoxFile &boxfile = box.GetBoxFile();
    CString CopyPath = boxfile.GetCopyPathForTruePath(SrcPath);

    POSITION pos = m_items.GetHeadPosition();
    while (pos) {
        const CString &OldCopyPath = m_items.GetNext(pos);
        if (OldCopyPath == CopyPath)
            return;
    }

    m_items.AddTail(CopyPath);

    GetDlgItem(ID_RECOVER_SELECT_ALL)->ShowWindow(
        (m_items.GetCount() > 1) ? SW_SHOW : SW_HIDE);

    //
    //
    //

    CString TruePath = SrcPath;
    boxfile.TranslateNtToDosPath(TruePath);

    CString text = TruePath;
    int index = 0;
    while (text.GetLength() - index > 65) {
        int index2 = text.Find(L'\\', index);
        if (index2 == -1)
            break;
        index = index2 + 1;
    }
    if (index)
        text = CString(L"...\\") + text.Mid(index);

    CListBox *items = (CListBox *)GetDlgItem(ID_RECOVER_ITEMS);
    items->AddString(text);
}


//---------------------------------------------------------------------------
// MoveButtonIntoList
//---------------------------------------------------------------------------


void CQuickRecover::MoveButtonIntoList(
    CButton *pButton, CListBox *pListBox, HBITMAP hBitmap)
{
    pButton->ModifyStyle(0, BS_BITMAP | BS_CENTER);
    pButton->SetBitmap(hBitmap);

    RECT rc;
    pListBox->GetWindowRect(&rc);
    ScreenToClient(&rc);
    rc.top = rc.bottom - 25;

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
    pListBox->ModifyStyle(0, WS_CLIPSIBLINGS);
}


//---------------------------------------------------------------------------
// OnInitDialog
//---------------------------------------------------------------------------


BOOL CQuickRecover::OnInitDialog()
{
    //
    // init bitmap resources
    //

    static HBITMAP checkmark_bitmap = NULL;
    static HBITMAP questionmark_bitmap = NULL;
    static HBITMAP cycle_bitmap = NULL;
    if (! checkmark_bitmap)
        checkmark_bitmap = MyGdi_CreateFromResource(L"BLUE_CHECKMARK");
    if (! questionmark_bitmap)
        questionmark_bitmap = MyGdi_CreateFromResource(L"BLUE_QUESTIONMARK");
    if (! cycle_bitmap)
        cycle_bitmap = MyGdi_CreateFromResource(L"BLUE_CYCLE");

    //
    // we're not SHBrowseForFolder
    //

    m_hwndTree = NULL;

    AddMinimizeButton();

    //
    // branch to Recovery Log if necessary
    //

    if (m_BoxName.IsEmpty())
        return OnInitDialogRecoveryLog();

    //
    // set dialog texts and adjust dialog position
    //

    SetWindowText(CMyMsg(MSG_3715));

    GetDlgItem(ID_DELETE_EXPLAIN_1)->SetWindowText(CMyMsg(MSG_3716));
    GetDlgItem(ID_DELETE_EXPLAIN_2)->SetWindowText(CMyMsg(MSG_3717));

    GetDlgItem(IDOK)->SetWindowText(CMyMsg(MSG_3718));
    GetDlgItem(IDCANCEL)->SetWindowText(CMyMsg(MSG_3004));

    GetDlgItem(ID_RECOVER_DISABLE)->SetWindowText(CMyMsg(MSG_3719));

    SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    //
    // add box name in title bar
    //

    CString caption;
    GetWindowText(caption);
    caption += L" - " + m_BoxName;
    SetWindowText(caption);

    //
    // populate items and show the Select All button
    //

    MakeLTR(ID_RECOVER_ITEMS);

    CListBox *pListBox = (CListBox *)GetDlgItem(ID_RECOVER_ITEMS);
    CButton *pButton = (CButton *)GetDlgItem(ID_RECOVER_SELECT_ALL);

    MoveButtonIntoList(pButton, pListBox, checkmark_bitmap);

    AddAutoRecoverItem(m_SrcPath);
    pListBox->SetSel(0, TRUE);

    if (! SelectAllButton.m_hWnd) {
        CString tip = CMyMsg(MSG_3360);
        tip.Remove(L'&');
        SelectAllButton.Init(pButton->m_hWnd, tip);
    }

    //
    // populate folder list
    //

    MakeLTR(ID_RECOVER_FOLDERS);

    HWND hwnd = GetDlgItem(ID_RECOVER_FOLDERS)->m_hWnd;
    m_listbox.Attach(hwnd);

    m_listbox.SetItemHeight(0, m_listbox.GetItemHeight(0) * 3 / 2);

    InitFolderList(true, 0);

    //
    // prepare the folder list tip button and the recover tool tip
    //

    pListBox = (CListBox *)GetDlgItem(ID_RECOVER_FOLDERS);
    pButton = (CButton *)GetDlgItem(ID_RECOVER_RIGHT_CLICK);

    MoveButtonIntoList(pButton, pListBox, questionmark_bitmap);
    if (! RightClickButton.m_hWnd)
        RightClickButton.Init(pButton->m_hWnd, CMyMsg(MSG_3734));

    if (m_list.GetCount() <= 2)
        pButton->ShowWindow(SW_HIDE);

    //
    // prepare the cycle button for the recover button
    //

    pButton = (CButton *)GetDlgItem(ID_RECOVER_CYCLE);
    pButton->SetBitmap(cycle_bitmap);
    pButton->SetWindowPos(NULL, 0, 0, 25, 25, SWP_NOMOVE | SWP_SHOWWINDOW);

    if (! CycleRecoverButton.m_hWnd)
        CycleRecoverButton.Init(pButton->m_hWnd, CMyMsg(MSG_3736));

    //
    // make sure the checkbox is reset
    //

    m_check = FALSE;

    //
    // enable list interaction
    //

    m_ready = TRUE;

    //
    // flash window
    //

    FlashTitle();

    GetDlgItem(IDOK)->SetFocus();
    return FALSE;
}


//---------------------------------------------------------------------------
// OnOK
//---------------------------------------------------------------------------


void CQuickRecover::OnOK()
{
    //
    // if this is OnOK for the customized SHBrowseForFolder, do nothing
    //

    if (m_hwndTree) {
        CWnd::Default();
        return;
    }

    if (m_log) {
        OnOKRecoveryLog();
        return;
    }

    //
    // get destination folder
    //

    BOOL ReorderFolders = FALSE;

    CListBox *list = (CListBox *)GetDlgItem(ID_RECOVER_FOLDERS);
    int index = list->GetCurSel();
    if (index == LB_ERR)
        return;
    BOOL sameFolder = FALSE;
    CString destFolder;
    if (index == 0)
        sameFolder = TRUE;
    else if (index != 1) {

        destFolder = m_list.GetAt(m_list.FindIndex(index));

        //
        // if selected from folder history, move folder to top
        //

        if (! destFolder.IsEmpty()) {

            CUserSettings &ini = CUserSettings::GetInstance();

            POSITION pos = m_list.GetHeadPosition();
            m_list.GetNext(pos);    // Recover to Same Folder
            m_list.GetNext(pos);    // Recover to Any Folder

            CString value = destFolder;
            while (pos) {
                const CString &add = m_list.GetNext(pos);
                if (add.CompareNoCase(destFolder) != 0)
                    value += L"\n" + add;
            }

            ini.SetText(_RecoverTarget, value);

            ReorderFolders = TRUE;
        }
    }

    //
    // recover selected items
    //

    BOOL AnySelected = FALSE;

    list = (CListBox *)GetDlgItem(ID_RECOVER_ITEMS);
    while (1) {

        int size = list->GetCount();

        for (index = 0; index < size; ++index) {
            if (list->GetSel(index) <= 0)
                continue;
            AnySelected = TRUE;
            POSITION pos = m_items.FindIndex(index);
            if (! pos)
                continue;
            m_SrcPath = m_items.GetAt(pos);

            BOOL ok;

            if (sameFolder) {

                ok = RecoverToSameFolder();

            } else {

                if (destFolder.IsEmpty()) {

                    CQuickRecover *qr2 = new CQuickRecover(
                            this, m_BoxName, m_SrcPath, QR_ANY);
                    destFolder = qr2->m_DstPath;
                    m_replace = qr2->m_replace;
                    delete qr2;

                    if (destFolder.IsEmpty())
                        return;

                    ReorderFolders = TRUE;
                }

                if (ReorderFolders) {

                    InitFolderList(true, 1);
                    GetDlgItem(ID_RECOVER_RIGHT_CLICK)->ShowWindow(
                        (m_list.GetCount() > 2) ? SW_SHOW : SW_HIDE);

                    m_check = FALSE;

                    ReorderFolders = FALSE;
                }

                ok = RecoverFile(destFolder);
            }

            if (ok) {

                m_items.RemoveAt(pos);
                GetDlgItem(ID_RECOVER_SELECT_ALL)->ShowWindow(
                    (m_items.GetCount() > 1) ? SW_SHOW : SW_HIDE);
                list->DeleteString(index);

            } else
                list->SetSel(index, FALSE);
            break;
        }

        if (index == size)
            break;
    }

    //
    // done
    //

    if (! AnySelected)
        CMyApp::MsgBox(this, MSG_3728, MB_OK);

    if (list->GetCount() == 0)
        EndDialog(0);
}


//---------------------------------------------------------------------------
// RecoverToSameFolder
//---------------------------------------------------------------------------


BOOL CQuickRecover::RecoverToSameFolder()
{
    CBox &box = CBoxes::GetInstance().GetBox(m_BoxName);
    CBoxFile &boxfile = box.GetBoxFile();

    CString CopyPath = m_SrcPath;
    CString TruePath = boxfile.GetTruePathForCopyPath(CopyPath);

    bool JustCreateDir = false;
    int index = TruePath.ReverseFind(L'\\');
    if (index != -1) {
        if (TruePath.Mid(index + 1) == L"\\*?|") // see RecoverFolder
            JustCreateDir = true;
        TruePath = TruePath.Mid(0, index);
    }

    CString TruePathDos = TruePath;
    boxfile.TranslateNtToDosPath(TruePathDos);
    if (TruePathDos.GetLength() > 3)
        SHCreateDirectory(m_pParentWnd->m_hWnd, TruePathDos);

    if (JustCreateDir)
        return TRUE;

    return RecoverFile(TruePath, TRUE);
}


//---------------------------------------------------------------------------
// RecoverFile
//---------------------------------------------------------------------------


BOOL CQuickRecover::RecoverFile(const CString &DestPath, BOOL SameFolder)
{
    //
    // prepare double-NULL-terminated versions of the
    // source and target paths
    //

    ULONG src_len = wcslen(m_SrcPath);
    WCHAR *src = malloc_WCHAR((src_len + 8) * sizeof(WCHAR));
    wmemcpy(src, m_SrcPath, src_len);
    wmemzero(src + src_len, 4);
    SbieDll_TranslateNtToDosPath(src);
    wmemzero(src + wcslen(src), 4);

    ULONG dst_len = wcslen(DestPath);
    WCHAR *dst = malloc_WCHAR((dst_len + 8) * sizeof(WCHAR));
    wmemcpy(dst, DestPath, dst_len);
    wmemzero(dst + dst_len, 4);
    SbieDll_TranslateNtToDosPath(dst);
    wmemzero(dst + wcslen(dst), 4);

    //
    // if destination path is too long, abort
    // if source path is too long, rename to a shorter folder
    //

    BOOL ok = FALSE;

    WCHAR *ptr = wcsrchr(src, L'\\');
    if (ptr) {

        if (wcslen(dst) + wcslen(ptr) >= 250) {

            CWnd *pWnd = m_hWnd ? this : m_pParentWnd;
            CMyApp::MsgBox(pWnd, CMyMsg(MSG_3733), MB_OK);

        } else {

            m_DstPath = DestPath;

            if (wcslen(src) < 250)
                ok = RecoverFile2(src, dst, SameFolder);
            else
                ok = RecoverFile3(src, dst, SameFolder);
        }
    }

    if (! ok)
        m_DstPath = CString();

    free(dst);
    free(src);

    return ok;
}


//---------------------------------------------------------------------------
// RecoverFile2
//---------------------------------------------------------------------------


BOOL CQuickRecover::RecoverFile2(WCHAR *src, WCHAR *dst, BOOL SameFolder)
{
    //
    // handle missing files and directories
    //

    ULONG attrs = GetFileAttributes(src);

    if (attrs & FILE_ATTRIBUTE_DIRECTORY) {

        BOOL ok;

        if (attrs == INVALID_FILE_ATTRIBUTES) {

            ULONG LastError = GetLastError();
            ok = (LastError == 0 ||
                  LastError == ERROR_FILE_NOT_FOUND ||
                  LastError == ERROR_PATH_NOT_FOUND);

        } else {

            ok = RecoverFolder(SameFolder);
        }

        return ok;
    }

    //
    // recover the file by moving it out of the sandbox
    //

    CWnd *pWnd = m_hWnd ? this : m_pParentWnd;

    SHFILEOPSTRUCT op;
    memzero(&op, sizeof(SHFILEOPSTRUCT));

    op.hwnd = pWnd->m_hWnd;
    op.wFunc = FO_MOVE;
    op.fFlags = FOF_NOCONFIRMMKDIR | FOF_SILENT
              | FOF_NOCOPYSECURITYATTRIBS;
    if (m_replace)
        op.fFlags |= FOF_NOCONFIRMATION;
    op.pFrom = src;
    op.pTo = dst;

    // The default operation is to Move the file -> Check if we have Delete access
    // If not, we'll switch to a Copy operation.  This allows Office or Adobe files
    // to be recovered while they are still in use.
    HANDLE hDeleteTest = CreateFile(src, DELETE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDeleteTest == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_SHARING_VIOLATION)
        {
            op.wFunc = FO_COPY;
        }
    }
    else
    {
        CloseHandle(hDeleteTest);
        hDeleteTest = NULL;
    }


    if (! ReplaceButtonText)
        ReplaceButtonText = _wcsdup(CMyMsg(MSG_3735));

    BOOL YesToAll;
    int rc = Common_SHFileOperation(&op, &YesToAll, ReplaceButtonText);
    if (YesToAll) {
        m_replace = TRUE;
        op.fFlags |= FOF_NOCONFIRMATION;
        rc = Common_SHFileOperation(&op, &YesToAll, ReplaceButtonText);
    }

    //
    // complain (on error) or add to log (on success)
    //

    if (GetFileAttributes(dst) == INVALID_FILE_ATTRIBUTES) {
        rc = 1;
        op.fAnyOperationsAborted = FALSE;
    }
    if ((rc != 0) && (! op.fAnyOperationsAborted))
        RecoverFileX(src);

    if ((rc == 0) && (! op.fAnyOperationsAborted)) {

        const WCHAR *backslash = wcsrchr(src, L'\\');
        if (backslash) {
            CString item = CString(dst) + CString(backslash);
            if (GetFileAttributes(item) != INVALID_FILE_ATTRIBUTES) {

                m_past_files.AddTail(item);

                if (m_recover_mode) {
                    const WCHAR *path = (m_recover_mode == 1) ? dst : item;
                    ShellExecute(
                        m_hWnd, NULL, path, NULL, NULL, SW_SHOWNORMAL);
                }
            }
        }
    }

    return (rc == 0 ? TRUE : FALSE);
}


//---------------------------------------------------------------------------
// RecoverFile3
//---------------------------------------------------------------------------


BOOL CQuickRecover::RecoverFile3(WCHAR *src, WCHAR *dst, BOOL SameFolder)
{
    BOOL complain = TRUE;
    BOOL ok;

    CBox &box = CBoxes::GetInstance().GetBox(m_BoxName);
    CBoxFile &boxfile = box.GetBoxFile();

    //
    // the source path is too long, rename it to something shorter.
    // first, create a temporary directory
    //

    ++m_TempFolderCounter;
    CString StringCounter;
    StringCounter.Format(L"\\temp-%04X", m_TempFolderCounter % 0x10000);

    CString TempFolder = boxfile.GetPathDos() + StringCounter;

    int SrcIndex = m_SrcPath.ReverseFind(L'\\');
    if (SrcIndex == -1)
        return FALSE;
    CString NewSrcPath = TempFolder + m_SrcPath.Mid(SrcIndex);
    if (NewSrcPath.GetLength() >= 250) {
        RecoverFileX(src);
        return FALSE;
    }

    ok = CreateDirectory(TempFolder, NULL);
    if ((! ok) && GetLastError() == ERROR_ALREADY_EXISTS)
        ok = TRUE;

    if (ok) {

        //
        // move the source file into the temporary directory
        //

        ok = MoveFile(_LongPathPrefix + src, NewSrcPath);
        if (ok) {

            ULONG src2_len = NewSrcPath.GetLength();
            WCHAR *src2 = malloc_WCHAR((src2_len + 4) * sizeof(WCHAR));
            wmemcpy(src2, NewSrcPath, src2_len);
            wmemzero(src2 + src2_len, 4);

            CString SaveSrcPath(m_SrcPath);
            m_SrcPath = boxfile.GetPathDos()
                      + StringCounter
                      + SaveSrcPath.Mid(SrcIndex);

            complain = FALSE;
            ok = RecoverFile2(src2, dst, SameFolder);

            m_SrcPath = SaveSrcPath;

            if (! ok) {

                //
                // if recovery failed, move the file back
                //

                MoveFile(NewSrcPath, _LongPathPrefix + src);
            }

            free(src2);
        }

        //
        // remove the temporary directory
        //

        RemoveDirectory(TempFolder);
    }

    if ((! ok) && complain)
        RecoverFileX(src);

    return ok;
}


//---------------------------------------------------------------------------
// RecoverFileX
//---------------------------------------------------------------------------


NOINLINE void CQuickRecover::RecoverFileX(WCHAR *src)
{
    CWnd *pWnd = m_hWnd ? this : m_pParentWnd;

    CString msg = CMyMsg(MSG_3729);
    msg += L"\n\n";
    msg += src;
    CMyApp::MsgBox(pWnd, msg, MB_OK);
}


//---------------------------------------------------------------------------
// RecoverFolder
//---------------------------------------------------------------------------


BOOL CQuickRecover::RecoverFolder(BOOL SameFolder)
{
    if (! SameFolder) {

        int SrcIndex = m_SrcPath.ReverseFind(L'\\');
        int DstIndex = m_DstPath.ReverseFind(L'\\');
        if (SrcIndex != -1 && DstIndex != -1) {

            CString SrcName = m_SrcPath.Mid(SrcIndex);
            CString DstName = m_DstPath.Mid(DstIndex);

            if (SrcName.CompareNoCase(DstName) != 0) {

                m_DstPath += SrcName;
                SHCreateDirectory(m_pParentWnd->m_hWnd, m_DstPath);
            }
        }
    }

    CString SaveSrcPath(m_SrcPath);
    CString SaveDstPath(m_DstPath);

    CString SearchString;
    if (m_SrcPath.GetLength() >= 250)
        SearchString = _LongPathPrefix;
    SearchString += SaveSrcPath + L"\\*";

    bool FoundAtLeastOneFile = false;

    WIN32_FIND_DATA data;
    HANDLE handle = FindFirstFile(SearchString, &data);
    while (handle != INVALID_HANDLE_VALUE) {

        CString FileName(data.cFileName);
        if (FileName != L"." && FileName != L".." &&
                (data.ftCreationTime.dwHighDateTime != 0x01B01234 ||
                 data.ftCreationTime.dwLowDateTime  != 0xDEAD44A0)) {

            m_SrcPath += L"\\" + FileName;

            BOOL ok;
            if (SameFolder) {

                FoundAtLeastOneFile = true;

                ok = RecoverToSameFolder();

            } else if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

                m_DstPath += L"\\" + FileName;

                SHCreateDirectory(m_pParentWnd->m_hWnd, m_DstPath);

                ok = RecoverFolder(FALSE);

                m_DstPath = SaveDstPath;

            } else {

                ok = RecoverFile(m_DstPath);

            }

            m_SrcPath = SaveSrcPath;

            if (! ok) {
                FindClose(handle);
                return FALSE;
            }
        }

        if (! FindNextFile(handle, &data)) {
            FindClose(handle);
            handle = INVALID_HANDLE_VALUE;
        }
    }

    if (SameFolder && (! FoundAtLeastOneFile)) {
        // make sure the folder itself is created even when it
        // doesn't contain any files or sub-folders
        m_SrcPath += L"\\*?|";
        RecoverToSameFolder();
        m_SrcPath = SaveSrcPath;
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// RecoveryLog
//---------------------------------------------------------------------------


void CQuickRecover::RecoveryLog()
{
    AutoRecover();  // just for SetDialogTemplate

    DoModal();
}


//---------------------------------------------------------------------------
// RecoveryLog
//---------------------------------------------------------------------------


BOOL CQuickRecover::OnInitDialogRecoveryLog()
{
    //
    // set dialog texts and adjust dialog position
    //

    SetWindowText(CMyMsg(MSG_3986));

    GetDlgItem(ID_DELETE_EXPLAIN_1)->SetWindowText(CMyMsg(MSG_3987));
    GetDlgItem(ID_DELETE_EXPLAIN_2)->ShowWindow(SW_HIDE);

    GetDlgItem(ID_RECOVER_DISABLE)->ShowWindow(SW_HIDE);

    //
    // stretch items list on top of folders list
    //

    CListBox *pListBox1 = (CListBox *)GetDlgItem(ID_RECOVER_ITEMS);
    CListBox *pListBox2 = (CListBox *)GetDlgItem(ID_RECOVER_FOLDERS);

    RECT rc1, rc2, rc3;
    pListBox1->GetWindowRect(&rc1);
    pListBox2->GetWindowRect(&rc2);
    rc1.bottom = rc2.bottom;
    ScreenToClient(&rc1);

    pListBox2->ShowWindow(SW_HIDE);
    pListBox1->SetWindowPos(NULL,
        rc1.left, rc1.top, rc1.right - rc1.left, rc1.bottom - rc1.top, 0);

    pListBox1->SetItemHeight(0, pListBox1->GetItemHeight(0) * 3 / 2);

    pListBox1->ModifyStyle(LBS_EXTENDEDSEL, 0);

    //
    // reposition the CLOSE button
    //

    CButton *pButton1 = (CButton *)GetDlgItem(IDOK);
    CButton *pButton2 = (CButton *)GetDlgItem(IDCANCEL);
    CButton *pButton3 = (CButton *)GetDlgItem(ID_RECOVER_REMOVE);

    pButton1->GetWindowRect(&rc1);
    pButton2->GetWindowRect(&rc2);
    rc2.top = rc1.bottom + 5;
    rc2.bottom = rc2.top + (rc1.bottom - rc1.top);
    rc1.right = rc2.right;
    rc1.top -= 5;
    rc1.bottom -= 5;
    ScreenToClient(&rc1);
    ScreenToClient(&rc2);
    rc3.left = rc1.left;
    rc3.top = rc2.top;
    rc3.right = rc2.left - 15;
    rc3.bottom = rc2.bottom;

    pButton1->SetWindowPos(NULL,
        rc1.left, rc1.top, rc1.right - rc1.left, rc1.bottom - rc1.top, 0);
    pButton2->SetWindowPos(NULL,
        rc2.left, rc2.top, rc2.right - rc2.left, rc2.bottom - rc2.top, 0);
    pButton3->SetWindowPos(NULL,
        rc3.left, rc3.top, rc3.right - rc3.left, rc3.bottom - rc3.top,
        SWP_SHOWWINDOW);

    pButton1->SetWindowText(CMyMsg(MSG_3981));
    pButton2->SetWindowText(CMyMsg(MSG_3004));
    pButton3->SetWindowText(CMyMsg(MSG_3983));

    //
    // populate list of recently recovered files
    //

    POSITION pos = m_past_files.GetHeadPosition();
    while (pos) {
        CString &item = m_past_files.GetNext(pos);
        pListBox1->InsertString(0, item);
    }

    m_log_count = (ULONG)(ULONG_PTR)m_past_files.GetCount();
    if (! m_log_count) {
        pButton1->EnableWindow(FALSE);
        pButton3->EnableWindow(FALSE);
    } else
        pListBox1->SetCurSel(0);

    //
    // set recovery log mode of operation
    //

    m_log = TRUE;
    m_ready = TRUE;

    return TRUE;
}


//---------------------------------------------------------------------------
// OnOKRecoveryLog
//---------------------------------------------------------------------------


void CQuickRecover::OnOKRecoveryLog()
{
    CListBox *list = (CListBox *)GetDlgItem(ID_RECOVER_ITEMS);
    int index = list->GetCurSel();
    if (index == LB_ERR)
        return;

    CString path = m_past_files.GetAt(m_past_files.FindIndex(index));
    index = path.ReverseFind(L'\\');
    if (index != -1) {
        path = path.Mid(0, index);
        ShellExecute(m_hWnd, NULL, path, NULL, NULL, SW_SHOWNORMAL);
    }
}



//---------------------------------------------------------------------------
// OnActivateRecoveryLog
//---------------------------------------------------------------------------


void CQuickRecover::OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized)
{
    if (m_log && nState != WA_INACTIVE &&
            m_log_count != (ULONG)(ULONG_PTR)m_past_files.GetCount()) {

        CListBox *pListBox1 = (CListBox *)GetDlgItem(ID_RECOVER_ITEMS);

        POSITION pos = m_past_files.FindIndex(m_log_count);
        while (pos) {
            CString &item = m_past_files.GetNext(pos);
            pListBox1->InsertString(0, item);
        }

        m_log_count = (ULONG)(ULONG_PTR)m_past_files.GetCount();
        if (m_log_count) {
            GetDlgItem(IDOK)->EnableWindow(TRUE);
            GetDlgItem(ID_RECOVER_REMOVE)->EnableWindow(TRUE);
        }
    }
}


//---------------------------------------------------------------------------
// OnRemoveFolders
//---------------------------------------------------------------------------


void CQuickRecover::OnRemoveFolders()
{
    m_past_files.RemoveAll();
    m_log_count = 0;
    ((CListBox *)GetDlgItem(ID_RECOVER_ITEMS))->ResetContent();
    GetDlgItem(IDOK)->EnableWindow(FALSE);
    GetDlgItem(ID_RECOVER_REMOVE)->EnableWindow(FALSE);
}


//---------------------------------------------------------------------------
// Split Button Stuff
//---------------------------------------------------------------------------


#if 0


#define BS_DEFSPLITBUTTON       0x0000000DL
#define BCN_DROPDOWN            (BCN_FIRST + 0x0002)
#define BCM_SETSPLITINFO        (BCM_FIRST + 0x0007)
// SPLIT BUTTON INFO mask flags
#define BCSIF_GLYPH             0x0001
#define BCSIF_IMAGE             0x0002
#define BCSIF_STYLE             0x0004
#define BCSIF_SIZE              0x0008
// SPLIT BUTTON STYLE flags
#define BCSS_NOSPLIT            0x0001
#define BCSS_STRETCH            0x0002
#define BCSS_ALIGNLEFT          0x0004
#define BCSS_IMAGE              0x0008


ON_NOTIFY(BCN_DROPDOWN,     IDOK,               OnDropDownRecover)


void CQuickRecover::OnDropDownRecover(NMHDR *pNMHDR, LRESULT *pResult)
{
    if (! CMyApp::m_WindowsVista)
        return;

    typedef struct tagNMBCDROPDOWN {
        NMHDR   hdr;
        RECT    rcButton;
    } NMBCDROPDOWN;
    NMBCDROPDOWN *pDropDown = (NMBCDROPDOWN *)pNMHDR;

    POINT pt;
    pt.x = pDropDown->rcButton.left;
    pt.y = pDropDown->rcButton.bottom;
    ::ClientToScreen(pDropDown->hdr.hwndFrom, &pt);

    CMenu menu;
    menu.CreatePopupMenu();
    menu.AppendMenu(MF_STRING, 1, CMyMsg(MSG_3718));
    menu.AppendMenu(MF_STRING, 2, CMyMsg(MSG_3736));
    menu.AppendMenu(MF_STRING, 3, CMyMsg(MSG_3737));
    ULONG menucmd = menu.TrackPopupMenu(
        TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN,
        pt.x, pt.y, this);

    while (menucmd >= 1 && menucmd <= 3 && m_recover_mode != menucmd)
        OnCycleRecover();
}


    //
    // prepare the cycle button for the recover button
    //

    if (CMyApp::m_WindowsVista) {

        pButton = (CButton *)GetDlgItem(IDOK);
        pButton->ModifyStyle(0, BS_DEFSPLITBUTTON);

        CImageList imglist;
        imglist.Create(25, 25, ILC_COLORDDB | ILC_MASK, 8, 8);
        imglist.SetBkColor(CLR_NONE);
        imglist.Add(CBitmap::FromHandle(cycle_bitmap), RGB(0,0,0));

        struct {
            UINT mask;
            HIMAGELIST himlGlyph;
            UINT uSplitStyle;
            SIZE size;
        } SplitInfo;
        memzero(&SplitInfo, sizeof(SplitInfo));
        SplitInfo.mask = BCSIF_IMAGE | BCSIF_STYLE;
        SplitInfo.himlGlyph = imglist.Detach();
        SplitInfo.uSplitStyle = BCSS_STRETCH | BCSS_IMAGE;
        pButton->SendMessage(BCM_SETSPLITINFO, 0, (LPARAM)&SplitInfo);
    }

    pButton = (CButton *)GetDlgItem(ID_RECOVER_CYCLE);
    pButton->SetBitmap(cycle_bitmap);
    pButton->SetWindowPos(NULL, 0, 0, 25, 25, SWP_NOMOVE | SWP_SHOWWINDOW);

    if (! CycleRecoverButton.m_hWnd)
        CycleRecoverButton.Init(pButton->m_hWnd, CMyMsg(MSG_3736));


#endif
