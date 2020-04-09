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
// Tabbing Combo Box
//---------------------------------------------------------------------------


#ifndef _MY_TABBING_COMBO_BOX_H
#define _MY_TABBING_COMBO_BOX_H


#include <afxcmn.h>


class CTabbingComboBox : public CComboBox
{

private:

    DECLARE_MESSAGE_MAP()

    void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);

    CWnd *m_pWndPrev;
    CWnd *m_pWndNext;

public:

    CTabbingComboBox();

    void SetTabbingWindows(CWnd *pWndNext, CWnd *pWndPrev);
};


#endif // _MY_TABBING_COMBO_BOX_H
