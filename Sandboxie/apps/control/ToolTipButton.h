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
// ToolTip Button
//---------------------------------------------------------------------------


#ifndef _MY_TOOLTIPBUTTON_H
#define _MY_TOOLTIPBUTTON_H


#include <afxcmn.h>


class CToolTipButton : public CButton
{
    DECLARE_MESSAGE_MAP()

    CToolTipCtrl m_tip;
    BOOL m_tip_active;

protected:

    virtual BOOL PreTranslateMessage(MSG *msg);

    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

public:

    bool Init(CDialog *dlg, UINT id, const CString &text = CString());
    bool Init(HWND hwndButton, const CString &text = CString());
    void SetText(const CString &text);

    CToolTipButton();
    ~CToolTipButton();
};


#endif // _MY_TOOLTIPBUTTON_H
