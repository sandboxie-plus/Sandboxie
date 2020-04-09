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

#include "stdafx.h"
#include "ToolTipButton.h"


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CToolTipButton, CButton)

    ON_WM_LBUTTONUP()

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CToolTipButton::CToolTipButton()
{
    m_tip_active = FALSE;
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CToolTipButton::~CToolTipButton()
{
}


//---------------------------------------------------------------------------
// Init
//---------------------------------------------------------------------------


bool CToolTipButton::Init(CDialog *dlg, UINT id, const CString &text)
{
    HWND hwndButton;
    dlg->GetDlgItem(id, &hwndButton);
    return Init(hwndButton, text);
}


//---------------------------------------------------------------------------
// Init
//---------------------------------------------------------------------------


bool CToolTipButton::Init(HWND hwndButton, const CString &text)
{
    if (hwndButton) {
        if (SubclassWindow(hwndButton)) {
            SetText(text);
            return true;
        }
    }
    return false;
}


//---------------------------------------------------------------------------
// SetText
//---------------------------------------------------------------------------


void CToolTipButton::SetText(const CString &text)
{
    if (! m_tip.m_hWnd) {

        if (text.IsEmpty())
            return;

        CRect rc;
        GetClientRect(&rc);

        m_tip.Create(this);
        m_tip.AddTool(this, text, rc, 1);

    } else {

        m_tip.UpdateTipText(text, this, 1);
    }

    m_tip_active = ! text.IsEmpty();
    m_tip.Activate(m_tip_active);
}


//---------------------------------------------------------------------------
// PreTranslateMessage
//---------------------------------------------------------------------------


BOOL CToolTipButton::PreTranslateMessage(MSG *msg)
{
    if (m_tip.m_hWnd)
        m_tip.RelayEvent(msg);

    return CButton::PreTranslateMessage(msg);
}


//---------------------------------------------------------------------------
// OnLButtonUp
//---------------------------------------------------------------------------


void CToolTipButton::OnLButtonUp(UINT nFlags, CPoint point)
{
    CButton::OnLButtonUp(nFlags, point);
    if (m_tip_active) {

        //
        // if a dialog box has only one control with a tool tip then
        // the tool tip stops appearing after the control is used,
        // the following workaround fixes that
        //

        m_tip.Activate(FALSE);
        m_tip.Activate(TRUE);
    }
}
