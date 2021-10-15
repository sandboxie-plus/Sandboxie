/*
 * Copyright 2020 David Xanatos, xanasoft.com
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
// Run Browser Dialog Box
//---------------------------------------------------------------------------


#ifndef _MY_UPDATEDIALOG_H
#define _MY_UPDATEDIALOG_H


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define ID_RUN_BROWSER_EXPLAIN  10111
#define ID_RUN_BROWSER_URL      10222


//---------------------------------------------------------------------------
// CUpdateDialog Class
//---------------------------------------------------------------------------


#ifndef RC_INVOKED


#include <afxcmn.h>


class CUpdateDialog : public CDialog
{
    DECLARE_MESSAGE_MAP()

    void *m_DlgTmplCopy;

    virtual BOOL OnInitDialog();

    afx_msg void OnNow();
    afx_msg void OnLater();
	afx_msg void OnNever();
	afx_msg void OnCancel();

	void CloseDialog();

public:

    CUpdateDialog(CWnd *pParentWnd);
    ~CUpdateDialog();

};


#endif // ! RC_INVOKED
#endif // _MY_UPDATEDIALOG_H
