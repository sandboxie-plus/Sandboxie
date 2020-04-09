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
// Alert Programs Dialog Box
//---------------------------------------------------------------------------


#ifndef _MY_ALERTDIALOG_H
#define _MY_ALERTDIALOG_H


class CAlertDialog : public CBaseDialog
{
    DECLARE_MESSAGE_MAP()

    virtual BOOL OnInitDialog();

    afx_msg void OnAdd();
    afx_msg void OnRemove();

    virtual void OnOK();

public:

    CAlertDialog(CWnd *pParentWnd);
    ~CAlertDialog();
};


#endif // _MY_ALERTDIALOG_H
