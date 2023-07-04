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
// Run Browser Dialog Box
//---------------------------------------------------------------------------


#ifndef _MY_RUNBROWSER_H
#define _MY_RUNBROWSER_H


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define ID_RUN_BROWSER_EXPLAIN  10111
#define ID_RUN_BROWSER_URL      10222


//---------------------------------------------------------------------------
// CRunBrowser Class
//---------------------------------------------------------------------------


#ifndef RC_INVOKED


#include <afxcmn.h>


class CRunBrowser : public CDialog
{
    DECLARE_MESSAGE_MAP()

    CString m_url;

    void *m_DlgTmplCopy;

    virtual BOOL OnInitDialog();

    afx_msg void OnYes();
    afx_msg void OnNo();

public:

    CRunBrowser(CWnd *pParentWnd, const CString &url);
    ~CRunBrowser();

    static CString GetTopicUrl(const CString &topic);

    static void OpenHelp(CWnd *pParentWnd, const CString &topic);

    static CString EscapeForURL(const CString& value);
};


#endif // ! RC_INVOKED
#endif // _MY_RUNBROWSER_H
