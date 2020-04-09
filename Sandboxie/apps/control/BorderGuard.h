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
// Border Guard
//---------------------------------------------------------------------------


#ifndef _MY_BORDERGUARD_H
#define _MY_BORDERGUARD_H


class CBorderGuard
{
    static CBorderGuard *m_instance;

    static void *m_DwmIsCompositionEnabled;
    static void *m_DwmGetWindowAttribute;

    static void MyTimerProc(
        HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

    void RefreshBorder(
        HWND hwnd, ULONG style, RECT *rect, const WCHAR *boxname);

    void RefreshConf2();

    UINT_PTR m_timer_id;
    int m_fast_timer_start_ticks;

    CPtrArray m_boxes;

    ULONG m_active_pid;
    HWND m_active_hwnd;
    RECT m_active_rect;
    RECT m_title_rect;
    int  m_title_mode;

    HWND m_border_hwnd;
    BOOL m_border_visible;
    HBRUSH m_border_brush;
    COLORREF m_border_brush_color;

    int m_thumb_width;
    int m_thumb_height;

    //ULONG m_border_width;

    bool m_windows_8;

public:

    CBorderGuard();

    ~CBorderGuard();

    void Refresh();

    static void RefreshConf();
};


#endif // _MY_BORDERGUARD_H
