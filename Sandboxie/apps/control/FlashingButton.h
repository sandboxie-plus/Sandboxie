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
// Flashing Button
//---------------------------------------------------------------------------


#ifndef _MY_FLASHINGBUTTON_H
#define _MY_FLASHINGBUTTON_H


#include <afxcmn.h>


class CFlashingButton : public CButton
{
    DECLARE_MESSAGE_MAP()

    ULONG m_width;
    ULONG m_height;
    ULONG_PTR m_timer;
    int m_invert;
    bool m_enable;

protected:

    afx_msg void OnPaint();

    afx_msg void OnMouseMove(UINT nFlags, CPoint point);

    afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);

    afx_msg void OnTimer(UINT_PTR nIDEvent);

public:

    bool Init(HWND hwndButton);
    bool Init(CWnd *dlg, UINT id);

    void EnableFlashing(bool enable);

    CFlashingButton();
    ~CFlashingButton();
};


#endif // _MY_FLASHINGBUTTON_H
