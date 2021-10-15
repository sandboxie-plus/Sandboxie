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


#include "stdafx.h"
#include "TabbingComboBox.h"


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CTabbingComboBox, CComboBox)

    ON_WM_CHAR()

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CTabbingComboBox::CTabbingComboBox()
    : CComboBox()
{
    m_pWndNext = NULL;
    m_pWndPrev = NULL;
}


//---------------------------------------------------------------------------
// Apply
//---------------------------------------------------------------------------


void CTabbingComboBox::SetTabbingWindows(CWnd *pWndNext, CWnd *pWndPrev)
{
    m_pWndNext = pWndNext;
    m_pWndPrev = pWndPrev;
}


//---------------------------------------------------------------------------
// OnChar
//---------------------------------------------------------------------------


void CTabbingComboBox::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar == VK_TAB) {
        CWnd *pWnd =
            (GetKeyState(VK_SHIFT) & 0x8000) ? m_pWndPrev : m_pWndNext;
        if (pWnd) {
            pWnd->SetFocus();
            return;
        }
    }
    const MSG *msg = GetCurrentMessage();
    DefWindowProc(msg->message, msg->wParam, msg->lParam);
}
