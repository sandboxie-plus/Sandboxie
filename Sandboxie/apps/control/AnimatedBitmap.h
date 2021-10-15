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
// Animated Bitmap
//---------------------------------------------------------------------------


#ifndef _MY_ANIMATEDBITMAP_H
#define _MY_ANIMATEDBITMAP_H


#include <afxcmn.h>


struct CAnimatedBitmap_NM : public NMHDR
{
    CDC *dc;
};


class CAnimatedBitmap : public CStatic
{

protected:

    DECLARE_MESSAGE_MAP()

    void *m_image;
    GUID m_guid;
    void *m_property;
    int m_frame_count;

    int m_frame_index;
    ULONG m_frame_ticks;

protected:

    void DrawFrame(CDC *dc);
    afx_msg void OnPaint();
    afx_msg void OnTimer(UINT_PTR nIDEvent);

public:

    bool Init(HWND hwndEdit);
    bool Init(CDialog *dlg, UINT id);
    bool SetImage(const WCHAR *name);
    void Animate(bool enable);

    CAnimatedBitmap();
    ~CAnimatedBitmap();
};


#endif // _MY_ANIMATEDBITMAP_H
