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
// Shell Integration Dialog Box
//---------------------------------------------------------------------------


#ifndef _MY_SHELLDIALOG_H
#define _MY_SHELLDIALOG_H


class CShellDialog : public CBaseDialog
{
    DECLARE_MESSAGE_MAP()

    static BOOL m_SpawnedMenu;

    virtual BOOL OnInitDialog();

    virtual void OnOK();

    afx_msg void OnShortcuts();

    afx_msg void OnCloseOrCancel();

    static void SyncRunLogon();
    static void SyncSendToMenuVista();

    static void CreateAssoc(WCHAR *path, const WCHAR *classname);
    static void DeleteAssoc(WCHAR *path, const WCHAR *classname);

    static void SyncBrowserIcon(
        const CString &setting, int nFolder, const CString &subdir);

    static BOOL GetStartMenuShortcut(
        CString &BoxName, CString &LinkPath,
        CString &IconPath, ULONG &IconIndex,
        CString &WorkDir);

    static void CreateDesktopShortcut(
        const CString &BoxName, const CString &LinkPath,
        const CString &IconPath, ULONG IconIndex,
        const CString &WorkDir);

    static BOOL CreateShortcut(
        const CString &LinkPath, const CString &LinkName,
        const CString &boxname, const WCHAR *pgmArgs,
        const CString &iconPath, int iconIndex, const CString &workdir,
        BOOL run_elevated = FALSE);

public:

    CShellDialog(CWnd *pParentWnd);
    ~CShellDialog();

    static void Sync();
    static void SyncContextMenu();
    static void SyncSendToMenu();

    static void CreateShortcut(CString BoxName, CString LinkPath);

};


#endif // _MY_SHELLDIALOG_H
