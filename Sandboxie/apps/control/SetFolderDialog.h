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


#ifndef _MY_SETFOLDERDIALOG_H
#define _MY_SETFOLDERDIALOG_H


class CSetFolderDialog : public CBaseDialog
{
    DECLARE_MESSAGE_MAP()

    virtual BOOL OnInitDialog();

    afx_msg void OnSelectDrive();

    virtual void OnOK();

    CString GetFolder();
    BOOL SetFolder(const CString &folder);

public:

    CSetFolderDialog(CWnd *pParentWnd);
    ~CSetFolderDialog();
};


#endif // _MY_SETFOLDERDIALOG_H
