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
// Create Sandbox Dialog Box
//---------------------------------------------------------------------------


#ifndef _MY_CREATEDIALOG_H
#define _MY_CREATEDIALOG_H


class CCreateDialog : public CBaseDialog
{
    DECLARE_MESSAGE_MAP()

    BOOL m_hide;
    ULONG m_title_id;
    ULONG m_explain_id;
    CString m_name;
    CString m_oldname;

    virtual BOOL OnInitDialog();

    afx_msg void OnNameChange();

    virtual void OnOK();

public:

    CString GetName() const;
    CString GetOldName() const;

    CCreateDialog(CWnd *pParentWnd, const CString origName = CString());
    ~CCreateDialog();
};


#endif // _MY_CREATEDIALOG_H
