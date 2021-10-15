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

/********************************************************************
*
* Copyright (c) 2002 Sven Wiegand <mail@sven-wiegand.de>
*
* You can use this and modify this in any way you want,
* BUT LEAVE THIS HEADER INTACT.
*
* Redistribution is appreciated.
*
* $Workfile:$
* $Revision:$
* $Modtime:$
* $Author:$
*
* Revision History:
*   $History:$
*
*********************************************************************/

#include "stdafx.h"
#include "PropPageFrameDefault.h"


namespace TreePropSheet
{


//uncomment the following line, if you don't have installed the
//new platform SDK
#define XPSUPPORT

#ifdef XPSUPPORT
#if _MSC_VER == 1200        // Visual C++ 6.0
#include "c:\work\psdk\include\uxtheme.h"
//#include "c:\work\psdk\include\tmschema.h"
#else
#include <uxtheme.h>
//#include <tmschema.h>
#endif
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-------------------------------------------------------------------
// class CThemeLib
//-------------------------------------------------------------------

#define THEMEAPITYPE(f)                 typedef HRESULT (__stdcall *_##f)
#define THEMEAPITYPE_(t, f)         typedef t (__stdcall *_##f)
#define THEMEAPIPTR(f)                  _##f m_p##f

#ifdef XPSUPPORT
    #define THEMECALL(f)                        return (*m_p##f)
    #define GETTHEMECALL(f)                 m_p##f = (_##f)GetProcAddress(m_hThemeLib, #f)
#else
    void ThemeDummy(...) {ASSERT(FALSE);}
    #define HTHEME                                  void*
    #define TABP_PANE                               0
    #define THEMECALL(f)                        return 0; ThemeDummy
    #define GETTHEMECALL(f)                 m_p##f = NULL
#endif


/**
Helper class for loading the uxtheme DLL and providing their
functions.

One global object of this class exists.

@author Sven Wiegand
*/
class CThemeLib
{
// construction/destruction
public:
    CThemeLib();
    ~CThemeLib();

// operations
public:
    /**
    Returns TRUE if the call wrappers are available, FALSE otherwise.
    */
    BOOL IsAvailable() const;

// call wrappers
public:
    BOOL IsThemeActive()
    {THEMECALL(IsThemeActive)();}

    HTHEME OpenThemeData(HWND hwnd, LPCWSTR pszClassList)
    {THEMECALL(OpenThemeData)(hwnd, pszClassList);}

    HRESULT CloseThemeData(HTHEME hTheme)
    {THEMECALL(CloseThemeData)(hTheme);}

    HRESULT GetThemeBackgroundContentRect(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, int iStateId,  const RECT *pBoundingRect, OUT RECT *pContentRect)
    {THEMECALL(GetThemeBackgroundContentRect)(hTheme, hdc, iPartId, iStateId, pBoundingRect, pContentRect);}

    HRESULT DrawThemeBackground(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, OPTIONAL const RECT *pClipRect)
    {THEMECALL(DrawThemeBackground)(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);}

// function pointers
private:
#ifdef XPSUPPORT
    THEMEAPITYPE_(BOOL, IsThemeActive)();
    THEMEAPIPTR(IsThemeActive);

    THEMEAPITYPE_(HTHEME, OpenThemeData)(HWND hwnd, LPCWSTR pszClassList);
    THEMEAPIPTR(OpenThemeData);

    THEMEAPITYPE(CloseThemeData)(HTHEME hTheme);
    THEMEAPIPTR(CloseThemeData);

    THEMEAPITYPE(GetThemeBackgroundContentRect)(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, int iStateId,  const RECT *pBoundingRect, OUT RECT *pContentRect);
    THEMEAPIPTR(GetThemeBackgroundContentRect);

    THEMEAPITYPE(DrawThemeBackground)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, OPTIONAL const RECT *pClipRect);
    THEMEAPIPTR(DrawThemeBackground);
#endif

// properties
private:
    /** instance handle to the library or NULL. */
    HINSTANCE m_hThemeLib;
};

/**
One and only instance of CThemeLib.
*/
static CThemeLib g_ThemeLib;


CThemeLib::CThemeLib()
:   m_hThemeLib(NULL)
{
#ifdef XPSUPPORT
    m_hThemeLib = LoadLibrary(_T("uxtheme.dll"));
    if (!m_hThemeLib)
        return;

    GETTHEMECALL(IsThemeActive);
    GETTHEMECALL(OpenThemeData);
    GETTHEMECALL(CloseThemeData);
    GETTHEMECALL(GetThemeBackgroundContentRect);
    GETTHEMECALL(DrawThemeBackground);
#endif
}


CThemeLib::~CThemeLib()
{
    if (m_hThemeLib)
        FreeLibrary(m_hThemeLib);
}


BOOL CThemeLib::IsAvailable() const
{
    return m_hThemeLib!=NULL;
}


//-------------------------------------------------------------------
// class CPropPageFrameDefault
//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CPropPageFrameDefault, CWnd)
    //{{AFX_MSG_MAP(CPropPageFrameDefault)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


CPropPageFrameDefault::CPropPageFrameDefault()
{
}


CPropPageFrameDefault::~CPropPageFrameDefault()
{
    if (m_Images.GetSafeHandle())
        m_Images.DeleteImageList();
}


/////////////////////////////////////////////////////////////////////
// Overridings

BOOL CPropPageFrameDefault::Create(DWORD dwWindowStyle, const RECT &rect, CWnd *pwndParent, UINT nID)
{
    return CWnd::Create(
        AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW, AfxGetApp()->LoadStandardCursor(IDC_ARROW), GetSysColorBrush(COLOR_3DFACE)),
        _T("Page Frame"),
        dwWindowStyle, rect, pwndParent, nID);
}


CWnd* CPropPageFrameDefault::GetWnd()
{
    return static_cast<CWnd*>(this);
}


void CPropPageFrameDefault::SetCaption(LPCTSTR lpszCaption, HICON hIcon /*= NULL*/)
{
    CPropPageFrame::SetCaption(lpszCaption, hIcon);

    // build image list
    if (m_Images.GetSafeHandle())
        m_Images.DeleteImageList();
    if (hIcon)
    {
        ICONINFO    ii;
        if (!GetIconInfo(hIcon, &ii))
            return;

        CBitmap bmMask;
        bmMask.Attach(ii.hbmMask);
        if (ii.hbmColor) DeleteObject(ii.hbmColor);

        BITMAP  bm;
        bmMask.GetBitmap(&bm);

        if (!m_Images.Create(bm.bmWidth, bm.bmHeight, ILC_COLOR32|ILC_MASK, 0, 1))
            return;

        if (m_Images.Add(hIcon) == -1)
            m_Images.DeleteImageList();
    }
}


CRect CPropPageFrameDefault::CalcMsgArea()
{
    CRect   rect;
    GetClientRect(rect);
    if (g_ThemeLib.IsAvailable() && g_ThemeLib.IsThemeActive())
    {
        HTHEME  hTheme = g_ThemeLib.OpenThemeData(m_hWnd, L"Tab");
        if (hTheme)
        {
            CRect   rectContent;
            CDC     *pDc = GetDC();
            g_ThemeLib.GetThemeBackgroundContentRect(hTheme, pDc->m_hDC, TABP_PANE, 0, rect, rectContent);
            ReleaseDC(pDc);
            g_ThemeLib.CloseThemeData(hTheme);

            if (GetShowCaption())
                rectContent.top = rect.top+GetCaptionHeight()+1;
            rect = rectContent;
        }
    }
    else if (GetShowCaption())
        rect.top+= GetCaptionHeight()+1;

    return rect;
}


CRect CPropPageFrameDefault::CalcCaptionArea()
{
    CRect   rect;
    GetClientRect(rect);
    if (g_ThemeLib.IsAvailable() && g_ThemeLib.IsThemeActive())
    {
        HTHEME  hTheme = g_ThemeLib.OpenThemeData(m_hWnd, L"Tab");
        if (hTheme)
        {
            CRect   rectContent;
            CDC     *pDc = GetDC();
            g_ThemeLib.GetThemeBackgroundContentRect(hTheme, pDc->m_hDC, TABP_PANE, 0, rect, rectContent);
            ReleaseDC(pDc);
            g_ThemeLib.CloseThemeData(hTheme);

            if (GetShowCaption())
                rectContent.bottom = rect.top+GetCaptionHeight();
            else
                rectContent.bottom = rectContent.top;

            rect = rectContent;
        }
    }
    else
    {
        if (GetShowCaption())
            rect.bottom = rect.top+GetCaptionHeight();
        else
            rect.bottom = rect.top;
    }

    return rect;
}

void CPropPageFrameDefault::DrawCaption(CDC *pDc, CRect rect, LPCTSTR lpszCaption, HICON hIcon)
{
    COLORREF    clrLeft = GetSysColor(COLOR_INACTIVECAPTION);
    COLORREF    clrRight = pDc->GetPixel(rect.right-1, rect.top);
    FillGradientRectH(pDc, rect, clrLeft, clrRight);

    // draw icon
    if (hIcon && m_Images.GetSafeHandle() && m_Images.GetImageCount() == 1)
    {
        IMAGEINFO   ii;
        m_Images.GetImageInfo(0, &ii);
        CPoint      pt(3, rect.CenterPoint().y - (ii.rcImage.bottom-ii.rcImage.top)/2);
        m_Images.Draw(pDc, 0, pt, ILD_TRANSPARENT);
        rect.left+= (ii.rcImage.right-ii.rcImage.left) + 3;
    }

    // draw text
    rect.left+= 2;

    COLORREF    clrPrev = pDc->SetTextColor(GetSysColor(COLOR_CAPTIONTEXT));
    int             nBkStyle = pDc->SetBkMode(TRANSPARENT);
    CFont           *pFont = (CFont*)pDc->SelectStockObject(SYSTEM_FONT);

    pDc->DrawText(lpszCaption, rect, DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);

    pDc->SetTextColor(clrPrev);
    pDc->SetBkMode(nBkStyle);
    pDc->SelectObject(pFont);
}


/////////////////////////////////////////////////////////////////////
// Implementation helpers

void CPropPageFrameDefault::FillGradientRectH(CDC *pDc, const RECT &rect, COLORREF clrLeft, COLORREF clrRight)
{
    // pre calculation
    int nSteps = rect.right-rect.left;
    int nRRange = GetRValue(clrRight)-GetRValue(clrLeft);
    int nGRange = GetGValue(clrRight)-GetGValue(clrLeft);
    int nBRange = GetBValue(clrRight)-GetBValue(clrLeft);

    double  dRStep = (double)nRRange/(double)nSteps;
    double  dGStep = (double)nGRange/(double)nSteps;
    double  dBStep = (double)nBRange/(double)nSteps;

    double  dR = (double)GetRValue(clrLeft);
    double  dG = (double)GetGValue(clrLeft);
    double  dB = (double)GetBValue(clrLeft);

    CPen    *pPrevPen = NULL;
    for (int x = rect.left; x <= rect.right; ++x)
    {
        CPen    Pen(PS_SOLID, 1, RGB((BYTE)dR, (BYTE)dG, (BYTE)dB));
        pPrevPen = pDc->SelectObject(&Pen);
        pDc->MoveTo(x, rect.top);
        pDc->LineTo(x, rect.bottom);
        pDc->SelectObject(pPrevPen);

        dR+= dRStep;
        dG+= dGStep;
        dB+= dBStep;
    }
}


/////////////////////////////////////////////////////////////////////
// message handlers

void CPropPageFrameDefault::OnPaint()
{
    CPaintDC dc(this);
    Draw(&dc);
}


BOOL CPropPageFrameDefault::OnEraseBkgnd(CDC* pDC)
{
    if (g_ThemeLib.IsAvailable() && g_ThemeLib.IsThemeActive())
    {
        HTHEME  hTheme = g_ThemeLib.OpenThemeData(m_hWnd, L"Tab");
        if (hTheme)
        {
            CRect   rect;
            GetClientRect(rect);
            g_ThemeLib.DrawThemeBackground(hTheme, pDC->m_hDC, TABP_PANE, 0, rect, NULL);

            g_ThemeLib.CloseThemeData(hTheme);
        }
        return TRUE;
    }
    else
    {
        return CWnd::OnEraseBkgnd(pDC);
    }
}



} //namespace TreePropSheet
