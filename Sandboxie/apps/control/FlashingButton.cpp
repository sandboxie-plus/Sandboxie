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

#include "stdafx.h"
#include "FlashingButton.h"


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CFlashingButton, CButton)

    ON_WM_PAINT()
    ON_WM_MOUSEMOVE()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
    ON_WM_TIMER()

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CFlashingButton::CFlashingButton()
    : CButton()
{
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CFlashingButton::~CFlashingButton()
{
    if (m_timer)
        KillTimer(m_timer);
}


//---------------------------------------------------------------------------
// Init
//---------------------------------------------------------------------------


bool CFlashingButton::Init(HWND hwndButton)
{
    if (hwndButton) {
        if (SubclassWindow(hwndButton)) {

            RECT rc;
            ::GetWindowRect(hwndButton, &rc);
            m_width = rc.right - rc.left + 1;
            m_height = rc.bottom - rc.top + 1;

            m_enable = true;
            m_invert = 1;
            m_timer = SetTimer(1234, 750, 0);

            return true;
        }
    }

    return false;
}


//---------------------------------------------------------------------------
// Init
//---------------------------------------------------------------------------


bool CFlashingButton::Init(CWnd *dlg, UINT id)
{
    HWND hwndButton;
    dlg->GetDlgItem(id, &hwndButton);
    return Init(hwndButton);
}


//---------------------------------------------------------------------------
// EnableFlashing
//---------------------------------------------------------------------------


void CFlashingButton::EnableFlashing(bool enable)
{
    m_enable = enable;
}


//---------------------------------------------------------------------------
// OnPaint
//---------------------------------------------------------------------------


void CFlashingButton::OnPaint()
{
    if (m_enable && m_invert == 1) {

        PAINTSTRUCT ps;
        HDC hWinDC = ::BeginPaint(m_hWnd, &ps);
        HDC hMemDC = CreateCompatibleDC(hWinDC);
        HBITMAP hBitmap = CreateCompatibleBitmap(hWinDC, m_width, m_height);
        SelectObject(hMemDC, hBitmap);

        DefWindowProc(WM_PAINT, (WPARAM)hMemDC, NULL);
        BitBlt(hWinDC, 0, 0, m_width, m_height, hMemDC, 0, 0, NOTSRCCOPY);

        DeleteObject(hBitmap);
        DeleteDC(hMemDC);
        ::EndPaint(m_hWnd, &ps);

    } else {

        DefWindowProc(WM_PAINT, NULL, NULL);
    }
}


//---------------------------------------------------------------------------
// OnMouseMove
//---------------------------------------------------------------------------


void CFlashingButton::OnMouseMove(UINT nFlags, CPoint point)
{
    if (m_invert) {

        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = m_hWnd;

        if (_TrackMouseEvent(&tme)) {
            m_invert = 0;
            Invalidate();
        }
    }

    DefWindowProc(WM_MOUSEMOVE, nFlags, MAKELPARAM(point.x, point.y));
}


//---------------------------------------------------------------------------
// OnMouseLeave
//---------------------------------------------------------------------------


LRESULT CFlashingButton::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
    m_invert = 1;
    Invalidate();
    return 0;
}


//---------------------------------------------------------------------------
// OnTimer
//---------------------------------------------------------------------------


void CFlashingButton::OnTimer(UINT_PTR nIDEvent)
{
    if (m_invert) {
        m_invert = -m_invert;
        Invalidate();
    }
}
