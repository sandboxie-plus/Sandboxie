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
// Base Dialog Box
//---------------------------------------------------------------------------


#ifndef _MY_BASEDIALOG_H
#define _MY_BASEDIALOG_H


class CBaseDialog : public CDialog
{
protected:

    DECLARE_MESSAGE_MAP()

    void *m_template_copy;

    CPtrList m_hidden_windows;

    void ShowOrHideAllWindows(bool hide);
    static BOOL ShowOrHideAllWindowsEnum(HWND hwnd, LPARAM lparam);

public:

    afx_msg void OnDestroy();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);

public:

    void SetDialogTemplate(const WCHAR *TemplateName);

    static void MakeLTR(CWnd *pWnd);
    void MakeLTR(UINT id);

    void AddMinimizeButton();

    void FlashTitle();

    CBaseDialog(CWnd *pParentWnd, const WCHAR *TemplateName = NULL);
    ~CBaseDialog();

    static CWnd *GetInputWindow(bool *inModal = NULL);
    static BOOL GetInputWindowEnum(HWND hwnd, LPARAM lparam);
    static bool InModalState();
};


#endif // _MY_DELETEDIALOG_H
