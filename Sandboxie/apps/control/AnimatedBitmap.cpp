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

#include "stdafx.h"
#include "AnimatedBitmap.h"
#include "apps/common/MyGdi.h"
#include <objbase.h>
#include <gdiplus.h>


//---------------------------------------------------------------------------
// MyImage class
//---------------------------------------------------------------------------


class CMyImage : public Gdiplus::Image
{

    IStream *m_pStream;

public:

    CMyImage()
        : Image() { nativeImage = NULL; m_pStream = NULL; }

    ~CMyImage()
    {
        if (m_pStream)
            m_pStream->Release();
    }

    bool SetImage(const WCHAR *rsrcName)
    {
        m_pStream = (IStream *)MyGdi_CreateResourceStream(rsrcName);
        if (! m_pStream)
            return false;

        lastResult = Gdiplus::DllExports::GdipLoadImageFromStream(
                                                m_pStream, &nativeImage);
        return (lastResult == 0 ? true : false);
    }

};


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CAnimatedBitmap, CStatic)

    ON_WM_PAINT()
    ON_WM_TIMER()

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CAnimatedBitmap::CAnimatedBitmap()
    : CStatic()
{
    m_image = NULL;
    m_property = NULL;
    m_frame_count = 0;
    m_frame_index = -1;
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CAnimatedBitmap::~CAnimatedBitmap()
{
    SetImage(NULL);
}


//---------------------------------------------------------------------------
// Init
//---------------------------------------------------------------------------


bool CAnimatedBitmap::Init(HWND hwndEdit)
{
    if (hwndEdit && SubclassWindow(hwndEdit)) {
        ModifyStyleEx(WS_EX_LAYOUTRTL, 0);
        return true;
    } else
        return false;
}


//---------------------------------------------------------------------------
// Init
//---------------------------------------------------------------------------


bool CAnimatedBitmap::Init(CDialog *dlg, UINT id)
{
    HWND hwndEdit;
    dlg->GetDlgItem(id, &hwndEdit);
    return Init(hwndEdit);
}


//---------------------------------------------------------------------------
// SetImage
//---------------------------------------------------------------------------


bool CAnimatedBitmap::SetImage(const WCHAR *name)
{
    //
    // delete resources for old image
    //

    CMyImage *image = (CMyImage *)m_image;
    if (image) {
        delete image;
        m_image = NULL;
    }

    if (m_property) {
        free(m_property);
        m_property = NULL;
    }

    m_frame_count = 0;
    m_frame_index = -1;

    //
    // load image file for new image
    //

    if (! name)
        return false;

    image = new CMyImage();
    if (! image)
        return false;

    if (! image->SetImage(name)) {
        delete image;
        return false;
    }

    //
    // get frame data from the image
    //

    UINT dim_count = image->GetFrameDimensionsCount();
    GUID *dim_ids = (GUID *)malloc(sizeof(GUID) * dim_count);
    if (! dim_ids) {
        delete image;
        return false;
    }

    if (image->GetFrameDimensionsList(dim_ids, dim_count) == 0) {

        m_guid = dim_ids[0];
        int frame_count = image->GetFrameCount(&m_guid);

        UINT item_size = 0;
        Gdiplus::PropertyItem *property = NULL;
        if (frame_count > 1)
            item_size = image->GetPropertyItemSize(PropertyTagFrameDelay);
        if (item_size)
            property = (Gdiplus::PropertyItem *)malloc(item_size);
        if (property) {

            if (image->GetPropertyItem(
                        PropertyTagFrameDelay, item_size, property) == 0) {

                free(dim_ids);

                m_image = image;
                m_property = property;

                m_frame_count = frame_count;

                return true;
            }
        }
    }

    free(dim_ids);
    delete image;
    return false;
}


//---------------------------------------------------------------------------
// Animate
//---------------------------------------------------------------------------


void CAnimatedBitmap::Animate(bool enable)
{
    if (! m_image)
        enable = false;

    if (enable && m_frame_index == -1) {

        SetTimer(26880, 10, NULL);
        m_frame_index = 0;
        m_frame_ticks = 0;

    } else if ((! enable) && m_frame_index != -1) {

        KillTimer(26880);
        m_frame_index = -1;
    }
}


//---------------------------------------------------------------------------
// DrawFrame
//---------------------------------------------------------------------------


void CAnimatedBitmap::DrawFrame(CDC *dc)
{
    //
    // create an off-screen buffer to paint into
    //

    CRect rc;
    GetClientRect(&rc);

    CBitmap mem_bmp;
    mem_bmp.CreateCompatibleBitmap(dc, rc.Width(), rc.Height());
    CDC mem_dc;
    mem_dc.CreateCompatibleDC(dc);
    CBitmap *old_bmp = mem_dc.SelectObject(&mem_bmp);

    //
    // paint the image into the off-screen buffer
    //

    CMyImage *image = (CMyImage *)m_image;
    const ULONG width = image->GetWidth();
    const ULONG height = image->GetHeight();

    Gdiplus::Graphics graphics(mem_dc.m_hDC);
    graphics.DrawImage(image, 0, 0, width, height);

    //
    // parent window can paint into off-screen buffer
    //

    CAnimatedBitmap_NM nm;
    nm.hwndFrom = m_hWnd;
    nm.idFrom = GetDlgCtrlID();
    nm.code = NM_CUSTOMDRAW;
    nm.dc = &mem_dc;
    GetParent()->SendMessage(WM_NOTIFY, nm.idFrom, (LPARAM)&nm);

    //
    // copy off-screen buffer into our window and release it
    //

    dc->BitBlt(0, 0, width, height, &mem_dc, 0, 0, SRCCOPY);

    mem_dc.SelectObject(old_bmp);
}


//---------------------------------------------------------------------------
// OnPaint
//---------------------------------------------------------------------------


void CAnimatedBitmap::OnPaint()
{
    if (IsWindowVisible()) {
        PAINTSTRUCT ps;
        CDC *dc = BeginPaint(&ps);
        DrawFrame(dc);
        EndPaint(&ps);
    }
}


//---------------------------------------------------------------------------
// OnTimer
//---------------------------------------------------------------------------


void CAnimatedBitmap::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == 26880 && m_image && m_frame_index != -1) {

        CMyImage *image = (CMyImage *)m_image;

        GUID page_guid = Gdiplus::FrameDimensionTime;
        image->SelectActiveFrame(&m_guid, m_frame_index);

        ULONG ticks_now = GetTickCount();

        if (m_frame_ticks == 0) {

            Gdiplus::PropertyItem *property =
                                        (Gdiplus::PropertyItem *)m_property;
            int frame_time = ((long *)property->value)[m_frame_index];
            m_frame_ticks = ticks_now + frame_time * 10;

            if (IsWindowVisible()) {
                CDC *dc = GetDC();
                if (dc) {
                    DrawFrame(dc);
                    ReleaseDC(dc);
                }
            }

        } else {

            if (m_frame_ticks <= ticks_now) {

                m_frame_ticks = 0;

                ++m_frame_index;
                if (m_frame_index == m_frame_count)
                    m_frame_index = 0;
            }
        }
    }

    CStatic::OnTimer(nIDEvent);
}
