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
// Window Title Map
//---------------------------------------------------------------------------


#ifndef _MY_WINDOWTITLEMAP_H
#define _MY_WINDOWTITLEMAP_H


class CWindowTitleMap : public CMapPtrToPtr
{
    static CWindowTitleMap *m_instance;

    ULONG m_counter;
    HICON m_nullIcon;
    ULONG m_pids[512];

    void *m_pGetWindowText;

    CWindowTitleMap();

    void Clear();

    BOOL ShouldIgnoreProcess(ULONG pid);

    static BOOL EnumProc(HWND hwnd, LPARAM lParam);

    void RefreshIcons();

public:

    ~CWindowTitleMap();

    static CWindowTitleMap &GetInstance();

    void Refresh();

    const WCHAR *Get(ULONG pid, HICON &icon);
};


#endif // _MY_WINDOWTITLEMAP_H
