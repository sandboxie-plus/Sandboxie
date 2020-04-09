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
// MenuXP.h : header file
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <afxtempl.h>

///////////////////////////////////////////////////////////////////////////////
// Menu item image management class
class CImgDesc
{
public:
    HIMAGELIST m_hImgList;
    int        m_nIndex;

    CImgDesc (HIMAGELIST hImgList = NULL, int nIndex = 0)
        : m_hImgList (hImgList), m_nIndex (nIndex)
    {
    }
};

///////////////////////////////////////////////////////////////////////////////
class CMenuXP
{
// Operations
public:
    static void SetXPLookNFeel (CWnd* pWnd, HMENU hMenu, bool bXPLook = true, bool bMenuBar = false);
    static void SetXPLookNFeelRecursive (CWnd* pWnd, HMENU hMenu, bool bXPLook = true, bool bMenuBar = false);
    static bool IsOwnerDrawn (HMENU hMenu);
    static void SetMenuItemImage (UINT nID, HIMAGELIST hImgList, int nIndex);
    static void OnMeasureItem (MEASUREITEMSTRUCT* pMeasureItemStruct);
    static bool OnDrawItem (DRAWITEMSTRUCT* pDrawItemStruct, HWND hWnd);
    static LRESULT OnMenuChar (HMENU hMenu, UINT nChar, UINT nFlags);

// Attributes
protected:
    static CMap <int, int, CString, CString&> ms_sCaptions;
    static CMap <HMENU, HMENU, CString, CString&> ms_sSubMenuCaptions;
    static CMap <int, int, CImgDesc, CImgDesc&> ms_Images;
    static CMap <HMENU, HMENU, CImgDesc, CImgDesc&> ms_SubMenuImages;
    static double m_dpiScale;

friend class CMenuItem;
};

///////////////////////////////////////////////////////////////////////////////
#define DECLARE_MENUXP()                                                             \
    protected:                                                                       \
    afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);     \
    afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct); \
    afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);          \
    afx_msg LRESULT OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu);

///////////////////////////////////////////////////////////////////////////////
#define ON_MENUXP_MESSAGES() \
    ON_WM_INITMENUPOPUP()    \
    ON_WM_MEASUREITEM()      \
    ON_WM_DRAWITEM()         \
    ON_WM_MENUCHAR()

///////////////////////////////////////////////////////////////////////////////
/*
#define IMPLEMENT_MENUXP(theClass, baseClass)                                      \
    IMPLEMENT_MENUXP_(theClass, baseClass, CMenuXP::GetXPLookNFeel (this))
*/
#define IMPLEMENT_MENUXP(theClass, baseClass)                                      \
    IMPLEMENT_MENUXP_(theClass, baseClass, TRUE)

///////////////////////////////////////////////////////////////////////////////
#define IMPLEMENT_MENUXP_(theClass, baseClass, bFlag)                              \
    void theClass::OnInitMenuPopup (CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) \
    {                                                                              \
        baseClass::OnInitMenuPopup (pPopupMenu, nIndex, bSysMenu);                 \
        CMenuXP::SetXPLookNFeel (this, pPopupMenu->m_hMenu,                        \
                                 bFlag/* && !bSysMenu*/);                          \
    }                                                                              \
    void theClass::OnMeasureItem (int, LPMEASUREITEMSTRUCT lpMeasureItemStruct)    \
    {                                                                              \
        CMenuXP::OnMeasureItem (lpMeasureItemStruct);                              \
    }                                                                              \
    void theClass::OnDrawItem (int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)      \
    {                                                                              \
        if ( !CMenuXP::OnDrawItem (lpDrawItemStruct, m_hWnd) )                     \
        {                                                                          \
            baseClass::OnDrawItem (nIDCtl, lpDrawItemStruct);                      \
        }                                                                          \
    }                                                                              \
    LRESULT theClass::OnMenuChar (UINT nChar, UINT nFlags, CMenu* pMenu)           \
    {                                                                              \
        if ( CMenuXP::IsOwnerDrawn (pMenu->m_hMenu) )                              \
        {                                                                          \
            return CMenuXP::OnMenuChar (pMenu->m_hMenu, nChar, nFlags);            \
        }                                                                          \
        return baseClass::OnMenuChar (nChar, nFlags, pMenu);                       \
    }

///////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
