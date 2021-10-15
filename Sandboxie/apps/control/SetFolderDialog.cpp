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
// Set Sandbox Folder Dialog Box
//---------------------------------------------------------------------------


#include "stdafx.h"
#include "SetFolderDialog.h"

#include "SbieIni.h"
#include "Boxes.h"


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CSetFolderDialog, CBaseDialog)

    ON_CONTROL(LBN_SELCHANGE,   ID_SETFOLDER_DRIVES,    OnSelectDrive)
    //ON_COMMAND(ID_SETFOLDER_BROWSE,     OnBrowse)

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static const CString _FileRootPath(L"FileRootPath");
static const CString _BoxRootFolder(L"BoxRootFolder");


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CSetFolderDialog::CSetFolderDialog(CWnd *pParentWnd)
    : CBaseDialog(pParentWnd, L"SETFOLDER_DIALOG")
{
    DoModal();
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CSetFolderDialog::~CSetFolderDialog()
{
}


//---------------------------------------------------------------------------
// OnInitDialog
//---------------------------------------------------------------------------


BOOL CSetFolderDialog::OnInitDialog()
{
    SetWindowText(CMyMsg(MSG_3671));

    GetDlgItem(ID_SETFOLDER_EXPLAIN_1)->SetWindowText(CMyMsg(MSG_3672));
    GetDlgItem(ID_SETFOLDER_EXPLAIN_2)->SetWindowText(CMyMsg(MSG_3673));
    GetDlgItem(ID_SETFOLDER_EXPLAIN_3)->SetWindowText(CMyMsg(MSG_3674));

    GetDlgItem(IDOK)->SetWindowText(CMyMsg(MSG_3001));
    GetDlgItem(IDCANCEL)->SetWindowText(CMyMsg(MSG_3002));

    CBoxes &boxes = CBoxes::GetInstance();
    for (int i = (int)boxes.GetSize() - 1; i >= 1; --i) {
        if (! boxes.GetBox(i).GetBoxFile().IsEmpty()) {

            GetDlgItem(ID_SETFOLDER_WARN)->SetWindowText(CMyMsg(MSG_3675));
            GetDlgItem(ID_SETFOLDER_WARN)->ShowWindow(TRUE);
            break;
        }
    }

    MakeLTR(ID_SETFOLDER_PATH);
    CString path = GetFolder();
    GetDlgItem(ID_SETFOLDER_PATH)->SetWindowText(path);

    CListBox *list = (CListBox *)GetDlgItem(ID_SETFOLDER_DRIVES);
    list->ResetContent();
    for (WCHAR letter = L'A'; letter <= L'Z'; ++letter) {
        WCHAR path[8];
        path[0] = letter;
        path[1] = L':';
        path[2] = L'\\';
        path[3] = L'\0';
        UINT type = GetDriveType(path);
        if (type != DRIVE_UNKNOWN && type != DRIVE_NO_ROOT_DIR) {
            CMyMsg msg(MSG_3676, CString(letter));
            int index = list->AddString(msg);
            list->SetItemData(index, (LPARAM)letter);
        }
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// OnSelectDrive
//---------------------------------------------------------------------------


void CSetFolderDialog::OnSelectDrive()
{
    CListBox *list = (CListBox *)GetDlgItem(ID_SETFOLDER_DRIVES);
    int index = list->GetCurSel();
    if (index == LB_ERR)
        return;
    WCHAR letter = (WCHAR)list->GetItemData(index);
    CString path;
    path = letter + CString(L":\\Sandbox\\%USER%\\%SANDBOX%");
    GetDlgItem(ID_SETFOLDER_PATH)->SetWindowText(path);
}


//---------------------------------------------------------------------------
// OnOK
//---------------------------------------------------------------------------


void CSetFolderDialog::OnOK()
{
    CString path;
    GetDlgItem(ID_SETFOLDER_PATH)->GetWindowText(path);

    if (SetFolder(path)) {

        CMyApp::MsgBox(this, MSG_3513, MB_OK);
    }

    EndDialog(0);
}


//---------------------------------------------------------------------------
// GetFolder
//---------------------------------------------------------------------------


CString CSetFolderDialog::GetFolder()
{
    CSbieIni &ini = CSbieIni::GetInstance();
    CString path;
    ini.GetText(_GlobalSettings, _FileRootPath, path);
    if (path.IsEmpty()) {
        ini.GetText(_GlobalSettings, _BoxRootFolder, path);
        if (! path.IsEmpty())
            path += L"\\Sandbox";
    }
    if (path.IsEmpty()) {
        WCHAR windir[MAX_PATH + 8];
#if _MSC_VER == 1200        // Visual C++ 6.0
        GetWindowsDirectory(windir, MAX_PATH);
#else
        GetSystemWindowsDirectoryW(windir, MAX_PATH);
#endif
        path = windir[0] + CString(L":\\Sandbox\\%USER%\\%SANDBOX%");
    }
    return path;
}


//---------------------------------------------------------------------------
// SetFolder
//---------------------------------------------------------------------------


BOOL CSetFolderDialog::SetFolder(const CString &folder)
{
    CSbieIni &ini = CSbieIni::GetInstance();
    CString path2 = folder;
    if (path2.Find(L"%SANDBOX%") == -1)
        path2 += L"\\%SANDBOX%";
    BOOL ok = ini.SetText(_GlobalSettings, _BoxRootFolder, CString());
    if (ok)
        ok = ini.SetText(_GlobalSettings, _FileRootPath, path2);

    CBoxes::GetInstance().ReloadBoxes();

    return ok;
}
