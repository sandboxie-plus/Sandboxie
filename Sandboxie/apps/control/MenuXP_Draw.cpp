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

///////////////////////////////////////////////////////////////////////////////
//
// Draw.cpp : implementation file
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MenuXP_Tools.h"
#include "MenuXP_Draw.h"

/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
*/

///////////////////////////////////////////////////////////////////////////////
HLSCOLOR RGB2HLS (COLORREF rgb)
{
    unsigned char minval = min(GetRValue(rgb), min(GetGValue(rgb), GetBValue(rgb)));
    unsigned char maxval = max(GetRValue(rgb), max(GetGValue(rgb), GetBValue(rgb)));
    float mdiff  = float(maxval) - float(minval);
    float msum   = float(maxval) + float(minval);

    float luminance = msum / 510.0f;
    float saturation = 0.0f;
    float hue = 0.0f;

    if ( maxval != minval )
    {
        float rnorm = (maxval - GetRValue(rgb)  ) / mdiff;
        float gnorm = (maxval - GetGValue(rgb)) / mdiff;
        float bnorm = (maxval - GetBValue(rgb) ) / mdiff;

        saturation = (luminance <= 0.5f) ? (mdiff / msum) : (mdiff / (510.0f - msum));

        if (GetRValue(rgb) == maxval) hue = 60.0f * (6.0f + bnorm - gnorm);
        if (GetGValue(rgb) == maxval) hue = 60.0f * (2.0f + rnorm - bnorm);
        if (GetBValue(rgb) == maxval) hue = 60.0f * (4.0f + gnorm - rnorm);
        if (hue > 360.0f) hue = hue - 360.0f;
    }
    return HLS ((hue*255)/360, luminance*255, saturation*255);
}

///////////////////////////////////////////////////////////////////////////////
static BYTE _ToRGB (float rm1, float rm2, float rh)
{
  if      (rh > 360.0f) rh -= 360.0f;
  else if (rh <   0.0f) rh += 360.0f;

  if      (rh <  60.0f) rm1 = rm1 + (rm2 - rm1) * rh / 60.0f;
  else if (rh < 180.0f) rm1 = rm2;
  else if (rh < 240.0f) rm1 = rm1 + (rm2 - rm1) * (240.0f - rh) / 60.0f;

  return (BYTE)(rm1 * 255);
}

///////////////////////////////////////////////////////////////////////////////
COLORREF HLS2RGB (HLSCOLOR hls)
{
    float hue        = ((int)HLS_H(hls)*360)/255.0f;
    float luminance  = HLS_L(hls)/255.0f;
    float saturation = HLS_S(hls)/255.0f;

    if ( saturation == 0.0f )
    {
        return RGB (HLS_L(hls), HLS_L(hls), HLS_L(hls));
    }
    float rm1, rm2;

    if ( luminance <= 0.5f ) rm2 = luminance + luminance * saturation;
    else                     rm2 = luminance + saturation - luminance * saturation;
    rm1 = 2.0f * luminance - rm2;
    BYTE red   = _ToRGB (rm1, rm2, hue + 120.0f);
    BYTE green = _ToRGB (rm1, rm2, hue);
    BYTE blue  = _ToRGB (rm1, rm2, hue - 120.0f);

    return RGB (red, green, blue);
}

///////////////////////////////////////////////////////////////////////////////
COLORREF HLS_TRANSFORM (COLORREF rgb, int percent_L, int percent_S)
{
    HLSCOLOR hls = RGB2HLS (rgb);
    BYTE h = HLS_H(hls);
    BYTE l = HLS_L(hls);
    BYTE s = HLS_S(hls);

    if ( percent_L > 0 )
    {
        l = BYTE(l + ((255 - l) * percent_L) / 100);
    }
    else if ( percent_L < 0 )
    {
        l = BYTE((l * (100+percent_L)) / 100);
    }
    if ( percent_S > 0 )
    {
        s = BYTE(s + ((255 - s) * percent_S) / 100);
    }
    else if ( percent_S < 0 )
    {
        s = BYTE((s * (100+percent_S)) / 100);
    }
    return HLS2RGB (HLS(h, l, s));
}

/////////////////////////////////////////////////////////////////////////////
/*HBITMAP WINAPI GetScreenBitmap (LPCRECT pRect)
{
    HDC     hDC;
    HDC     hMemDC;
    HBITMAP hNewBitmap = NULL;

    if ( (hDC = ::GetDC (NULL)) != NULL )
    {
        if ( (hMemDC = ::CreateCompatibleDC (hDC)) != NULL )
        {
            if ( (hNewBitmap = ::CreateCompatibleBitmap (hDC, pRect->right - pRect->left, pRect->bottom - pRect->top)) != NULL )
            {
                HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hMemDC, hNewBitmap);
                ::BitBlt (hMemDC, 0, 0, pRect->right - pRect->left, pRect->bottom - pRect->top,
                          hDC, pRect->left, pRect->top, SRCCOPY);
                ::SelectObject (hMemDC, (HGDIOBJ) hOldBitmap);
            }
            ::DeleteDC (hMemDC);
        }
        ::ReleaseDC (NULL, hDC);
    }
    return hNewBitmap;
}*/


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
CBufferDC::CBufferDC (HDC hDestDC, const CRect& rcPaint) : m_hDestDC (hDestDC)
{
    if ( rcPaint.IsRectEmpty() )
    {
        ::GetClipBox (m_hDestDC, m_rect);
    }
    else
    {
        m_rect = rcPaint;
    }
    VERIFY(Attach (::CreateCompatibleDC (m_hDestDC)));
    m_bitmap.Attach (::CreateCompatibleBitmap (m_hDestDC, m_rect.right, m_rect.bottom));
    m_hOldBitmap = ::SelectObject (m_hDC, m_bitmap);

    if ( m_rect.top > 0 )
    {
        ExcludeClipRect (0, 0, m_rect.right, m_rect.top);
    }
    if ( m_rect.left > 0 )
    {
        ExcludeClipRect (0, m_rect.top, m_rect.left, m_rect.bottom);
    }
}

///////////////////////////////////////////////////////////////////////////////
CBufferDC::~CBufferDC ()
{
    VERIFY(::BitBlt (m_hDestDC, m_rect.left, m_rect.top, m_rect.Width(), m_rect.Height(), m_hDC, m_rect.left, m_rect.top, SRCCOPY));
    ::SelectObject (m_hDC, m_hOldBitmap);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
CPenDC::CPenDC (HDC hDC, COLORREF crColor) : m_hDC (hDC)
{
    VERIFY(m_pen.CreatePen (PS_SOLID, 1, crColor));
    m_hOldPen = (HPEN)::SelectObject (m_hDC, m_pen);
}

///////////////////////////////////////////////////////////////////////////////
CPenDC::~CPenDC ()
{
    ::SelectObject (m_hDC, m_hOldPen);
}

///////////////////////////////////////////////////////////////////////////////
void CPenDC::Color (COLORREF crColor)
{
    ::SelectObject (m_hDC, m_hOldPen);
    VERIFY(m_pen.DeleteObject());
    VERIFY(m_pen.CreatePen (PS_SOLID, 1, crColor));
    m_hOldPen = (HPEN)::SelectObject (m_hDC, m_pen);
}

///////////////////////////////////////////////////////////////////////////////
COLORREF CPenDC::Color () const
{
    LOGPEN logPen;

    ((CPenDC*)this)->m_pen.GetLogPen (&logPen);

    return logPen.lopnColor;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
CBrushDC::CBrushDC (HDC hDC, COLORREF crColor) : m_hDC (hDC)
{
    if ( crColor == CLR_NONE ) VERIFY(m_brush.CreateStockObject (NULL_BRUSH));
    else                       VERIFY(m_brush.CreateSolidBrush (crColor));
    m_hOldBrush = (HBRUSH)::SelectObject (m_hDC, m_brush);
}

///////////////////////////////////////////////////////////////////////////////
CBrushDC::~CBrushDC ()
{
    ::SelectObject (m_hDC, m_hOldBrush);
}

///////////////////////////////////////////////////////////////////////////////
void CBrushDC::Color (COLORREF crColor)
{
    ::SelectObject (m_hDC, m_hOldBrush);
    VERIFY(m_brush.DeleteObject());
    if ( crColor == CLR_NONE ) VERIFY(m_brush.CreateStockObject (NULL_BRUSH));
    else                       VERIFY(m_brush.CreateSolidBrush (crColor));
    m_hOldBrush = (HBRUSH)::SelectObject (m_hDC, m_brush);
}

///////////////////////////////////////////////////////////////////////////////
COLORREF CBrushDC::Color () const
{
    LOGBRUSH logBrush;

    ((CBrushDC*)this)->m_brush.GetLogBrush (&logBrush);

    return logBrush.lbColor;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CFontDC::CFontDC (HDC hDC, LPCTSTR sFaceName, COLORREF crText)
        : m_hDC (hDC), m_hFont (NULL), m_hDefFont (NULL), m_crTextOld (CLR_NONE)
{
    *this = sFaceName;

    if ( crText != CLR_DEFAULT )
    {
        *this = crText;
    }
}

/////////////////////////////////////////////////////////////////////////////
CFontDC::CFontDC (HDC hDC, BYTE nStockFont, COLORREF crText)
        : m_hDC (hDC), m_hFont (NULL), m_hDefFont (NULL), m_crTextOld (CLR_NONE)
{
    *this = nStockFont;

    if ( crText != CLR_DEFAULT )
    {
        *this = crText;
    }
}

/////////////////////////////////////////////////////////////////////////////
CFontDC::CFontDC (HDC hDC, HFONT hFont, COLORREF crText)
        : m_hDC (hDC), m_hFont (NULL), m_hDefFont (NULL), m_crTextOld (CLR_NONE)
{
    *this = hFont;

    if ( crText != CLR_DEFAULT )
    {
        *this = crText;
    }
}

/////////////////////////////////////////////////////////////////////////////
CFontDC::~CFontDC ()
{
    if ( m_hDefFont != NULL )
    {
        ::SelectObject (m_hDC, m_hDefFont);
        DeleteObject (m_hFont);
    }
    if ( m_crTextOld != CLR_NONE )
    {
        ::SetTextColor (m_hDC, m_crTextOld);
    }
}

/////////////////////////////////////////////////////////////////////////////
const CFontDC& CFontDC::operator = (LPCTSTR sFaceName)
{
    LOGFONT lf;

    ::GetObject (::GetCurrentObject (m_hDC, OBJ_FONT), sizeof(LOGFONT), &lf);

    if ( _tcsicmp (sFaceName, lf.lfFaceName) )
    {
        if ( m_hDefFont != NULL )
        {
            ::SelectObject (m_hDC, m_hDefFont);
            DeleteObject (m_hFont);
        }
        _tcscpy (lf.lfFaceName, sFaceName);
        m_hFont = ::CreateFontIndirect (&lf);
        m_hDefFont = (HFONT)::SelectObject (m_hDC, m_hFont);
    }
    return *this;
}

/////////////////////////////////////////////////////////////////////////////
const CFontDC& CFontDC::operator = (BYTE nStockFont)
{
    if ( m_hDefFont != NULL )
    {
        ::SelectObject (m_hDC, m_hDefFont);
        DeleteObject (m_hFont);
    }

    if (nStockFont == DEFAULT_GUI_FONT) {

        NONCLIENTMETRICS NonClientMetrics;
        memset(&NonClientMetrics, 0, sizeof(NONCLIENTMETRICS));
        NonClientMetrics.cbSize = sizeof(NONCLIENTMETRICS);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS,
                             sizeof(NONCLIENTMETRICS), &NonClientMetrics, 0);
        m_hFont = ::CreateFontIndirect(&NonClientMetrics.lfMenuFont);

    } else
        m_hFont = (HFONT)::GetStockObject (nStockFont);

    m_hDefFont = (HFONT)::SelectObject (m_hDC, m_hFont);

    return *this;
}

/////////////////////////////////////////////////////////////////////////////
const CFontDC& CFontDC::operator = (HFONT hFont)
{
    if ( m_hDefFont != NULL )
    {
        ::SelectObject (m_hDC, m_hDefFont);
        DeleteObject (m_hFont);
    }
    m_hFont = hFont;
    m_hDefFont = (HFONT)::SelectObject (m_hDC, m_hFont);

    return *this;
}

/////////////////////////////////////////////////////////////////////////////
const CFontDC& CFontDC::operator = (COLORREF crText)
{
    if ( m_crTextOld == CLR_NONE )
    {
        m_crTextOld = ::GetTextColor (m_hDC);
    }
    ::SetTextColor (m_hDC, crText);

    return *this;
}

/////////////////////////////////////////////////////////////////////////////
CFontDC::operator LPCTSTR ()
{
    static LOGFONT lf;

    ::GetObject (::GetCurrentObject (m_hDC, OBJ_FONT), sizeof(LOGFONT), &lf);

    return lf.lfFaceName;
}

/////////////////////////////////////////////////////////////////////////////
CFontDC::operator COLORREF ()
{
    return ::GetTextColor (m_hDC);
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CBoldDC::CBoldDC (HDC hDC, bool bBold) : m_hDC (hDC), m_hDefFont (NULL)
{
    HFONT hFont = (HFONT)::GetCurrentObject(m_hDC, OBJ_FONT);
    if (hFont) {

        CFont *pFont = CFont::FromHandle(hFont);
        if (pFont) {

            LOGFONT lf;
            if (pFont->GetLogFont (&lf)) {

                if ( ( bBold && lf.lfWeight != FW_BOLD) ||
                     (!bBold && lf.lfWeight == FW_BOLD) )
                {
                    lf.lfWeight = bBold ? FW_BOLD : FW_NORMAL;

                    m_fontBold.CreateFontIndirect (&lf);
                    m_hDefFont = (HFONT)::SelectObject (m_hDC, m_fontBold);
                }
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
CBoldDC::~CBoldDC ()
{
    if ( m_hDefFont != NULL )
    {
        ::SelectObject (m_hDC, m_hDefFont);
    }
}
