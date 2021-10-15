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
// MenuXP.cpp : implementation file
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MenuXP.h"
#include "MenuXP_Tools.h"
#include "MenuXP_Draw.h"

/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
*/

//#ifndef DWORD_PTR
//    typedef DWORD* DWORD_PTR;
//    typedef long* LONG_PTR;
//#endif

#ifndef ODS_HOTLIGHT
    #define ODS_HOTLIGHT 0x0040
#endif
#ifndef ODS_INACTIVE
    #define ODS_INACTIVE 0x0080
#endif

#ifndef ODS_NOACCEL
    #define ODS_NOACCEL 0x0100
#endif

#ifndef DT_HIDEPREFIX
    #define DT_HIDEPREFIX 0x00100000
#endif

#ifndef SPI_GETKEYBOARDCUES
    #define SPI_GETKEYBOARDCUES 0x100A
#endif

// From <winuser.h>
#ifndef OBM_CHECK
    #define OBM_CHECK 32760
#endif

#define IMGWIDTH 16
#define IMGHEIGHT 16
#define IMGPADDING 6
#define TEXTPADDING 8
#define TEXTPADDING_MNUBR 4
#define SM_CXSHADOW 4

#define DPISCALE(x) ((int)(CMenuXP::m_dpiScale * (x)))


///////////////////////////////////////////////////////////////////////////////
// Menu item management class
//
class CMenuItem
{
protected:
    MENUITEMINFO m_miInfo;
    CString      m_sCaption;
    CImgDesc     m_ImgDesc;
    HIMAGELIST   m_hImgList;
    int          m_nIndex;

public:
    CMenuItem ();
    CMenuItem (HMENU hMenu, UINT uItem, bool fByPosition = true);
   ~CMenuItem ();

// Properties
public:
    int   GetCaption   (CString& sCaption) const;
    int   GetShortCut  (CString& sShortCut) const;
    bool  GetSeparator () const;
    bool  GetChecked   () const;
    bool  GetRadio     () const;
    bool  GetDisabled  () const;
    bool  GetDefault   () const;
    HMENU GetPopup     () const;
    UINT  GetID        () const;

// Methods
public:
    int  GetCaptionWidth  (CDC* pDC) const;
    int  GetShortCutWidth (CDC* pDC) const;
    int  GetHeight        (CDC* pDC) const;
    bool Draw             (CDC* pDC, LPCRECT pRect, bool bSelected, bool bMenuBar = false, bool bHotLight = false, bool bInactive = false, bool bNoAccel = false) const;

public:
    static BYTE ms_nCheck;
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BYTE CMenuItem::ms_nCheck = 0;

///////////////////////////////////////////////////////////////////////////////
CMenuItem::CMenuItem ()
{
    memset (&m_miInfo, 0, sizeof(MENUITEMINFO));
}

///////////////////////////////////////////////////////////////////////////////
CMenuItem::CMenuItem (HMENU hMenu, UINT uItem, bool fByPosition)
{
    memset (&m_miInfo, 0, sizeof(MENUITEMINFO));
    m_miInfo.cbSize = sizeof(MENUITEMINFO);
    m_miInfo.fMask = MIIM_STATE|MIIM_SUBMENU|MIIM_TYPE|MIIM_DATA|MIIM_ID;
    VERIFY (::GetMenuItemInfo (hMenu, uItem, fByPosition, &m_miInfo));

    if ( !(m_miInfo.fType & MFT_SEPARATOR) )
    {
        if ( m_miInfo.hSubMenu != NULL )
        {
            CMenuXP::ms_sSubMenuCaptions.Lookup (m_miInfo.hSubMenu, m_sCaption);
            CMenuXP::ms_SubMenuImages.Lookup (m_miInfo.hSubMenu, m_ImgDesc);
        }
        else
        {
            CMenuXP::ms_sCaptions.Lookup (m_miInfo.wID, m_sCaption);
            CMenuXP::ms_Images.Lookup (m_miInfo.wID, m_ImgDesc);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
CMenuItem::~CMenuItem ()
{
}

///////////////////////////////////////////////////////////////////////////////
int CMenuItem::GetCaption (CString& sCaption) const
{
    ASSERT(m_miInfo.fMask & MIIM_TYPE);
    sCaption = m_sCaption;

    int nShortCutPos = sCaption.Find ('\t');

    if ( nShortCutPos == -1 )
    {
        return sCaption.GetLength();
    }
    sCaption = sCaption.Left (nShortCutPos);

    return nShortCutPos;
}

///////////////////////////////////////////////////////////////////////////////
int CMenuItem::GetShortCut (CString& sShortCut) const
{
    ASSERT(m_miInfo.fMask & MIIM_TYPE);
    CString sCaption = m_sCaption;

    int nShortCutPos = sCaption.Find ('\t');

    if ( nShortCutPos == -1 )
    {
        sShortCut = "";
        return 0;
    }
    int nLength = sCaption.GetLength()-nShortCutPos-1;

    sShortCut = sCaption.Right (nLength);

    return nLength;
}

///////////////////////////////////////////////////////////////////////////////
bool CMenuItem::GetSeparator () const
{
    ASSERT(m_miInfo.fMask & MIIM_TYPE);
    return (m_miInfo.fType & MFT_SEPARATOR) == MFT_SEPARATOR;
}

///////////////////////////////////////////////////////////////////////////////
bool CMenuItem::GetChecked () const
{
    ASSERT(m_miInfo.fMask & MIIM_STATE);
    return (m_miInfo.fState & MFS_CHECKED) == MFS_CHECKED;
}

///////////////////////////////////////////////////////////////////////////////
bool CMenuItem::GetRadio () const
{
    ASSERT(m_miInfo.fMask & MIIM_TYPE);
    return (m_miInfo.fType & MFT_RADIOCHECK) == MFT_RADIOCHECK;
}

///////////////////////////////////////////////////////////////////////////////
bool CMenuItem::GetDisabled () const
{
    ASSERT(m_miInfo.fMask & MIIM_STATE);
    return (m_miInfo.fState & MFS_GRAYED) == MFS_GRAYED;
}

///////////////////////////////////////////////////////////////////////////////
bool CMenuItem::GetDefault () const
{
    ASSERT(m_miInfo.fMask & MIIM_STATE);
    return (m_miInfo.fState & MFS_DEFAULT) == MFS_DEFAULT;
}

///////////////////////////////////////////////////////////////////////////////
HMENU CMenuItem::GetPopup () const
{
    ASSERT(m_miInfo.fMask & MIIM_SUBMENU);
    return m_miInfo.hSubMenu;
}

///////////////////////////////////////////////////////////////////////////////
UINT CMenuItem::GetID () const
{
    ASSERT(m_miInfo.fMask & MIIM_ID);
    return m_miInfo.wID;
}

///////////////////////////////////////////////////////////////////////////////
int CMenuItem::GetCaptionWidth (CDC* pDC) const
{
    if ( GetSeparator() )
    {
        return 0;
    }
    CString sCaption;
    int nLength = 0;

    if ( GetCaption (sCaption) > 0 )
    {
        int nPos = sCaption.Find ('&');
        CBoldDC bold (*pDC, GetDefault());

        if ( nPos >= 0 )
        {
            sCaption = sCaption.Left (nPos) + sCaption.Right (sCaption.GetLength()-nPos-1);
        }
        nLength = pDC->GetTextExtent (sCaption).cx;
    }
    return nLength;
}

///////////////////////////////////////////////////////////////////////////////
int CMenuItem::GetShortCutWidth (CDC* pDC) const
{
    if ( GetSeparator() )
    {
        return 0;
    }
    CString sShortCut;
    int nLength = 0;

    if ( GetShortCut (sShortCut) > 0 )
    {
        CBoldDC bold (*pDC, GetDefault());

        nLength = pDC->GetTextExtent (sShortCut).cx;
    }
    return nLength;
}

///////////////////////////////////////////////////////////////////////////////
int CMenuItem::GetHeight (CDC* pDC) const
{
    TEXTMETRIC tm;

    pDC->GetTextMetrics (&tm);

    return GetSeparator() ? tm.tmHeight/2+2 : tm.tmHeight+4;
}

///////////////////////////////////////////////////////////////////////////////
bool CMenuItem::Draw (CDC* pDC, LPCRECT pRect, bool bSelected, bool bMenuBar, bool bHotLight, bool bInactive, bool bNoAccel) const
{
    COLORREF crMenuBkgnd;
    BOOL bFlatMenus;
    SystemParametersInfo(SPI_GETFLATMENU, 0, &bFlatMenus, 0);
    if (bFlatMenus)
        crMenuBkgnd = ::GetSysColor (COLOR_MENUBAR);
    else
        crMenuBkgnd = ::GetSysColor (COLOR_MENU);

    COLORREF crBackImg = CLR_NONE;
    bool bMenuBarItemSelected = false;

    if ( bMenuBar && bSelected )
    {
        CRect rc (pRect);
        CPenDC pen (*pDC, ::GetSysColor (COLOR_3DDKSHADOW));
        CBrushDC brush (*pDC, HLS_TRANSFORM (crMenuBkgnd, +20, 0));

        rc.right -= TEXTPADDING_MNUBR;
        bMenuBarItemSelected = true;
        pDC->Rectangle (rc);
        rc.left = rc.right;
        rc.right += TEXTPADDING_MNUBR;
        pDC->FillSolidRect (rc, crMenuBkgnd);

        for ( int x = 0; x < SM_CXSHADOW; x++ )
        {
            for ( int y = ( x < 2 ) ? 2-x : 0; y < rc.Height()-x-((x>0)?1:2); y++ )
            {
                int nMakeSpec = 78+(3-(y==0?0:(y==1?(x<2?0:1):(y==2?(x<2?x:2):x))))*5;
                COLORREF cr = pDC->GetPixel (rc.right-x-1, rc.top+y+SM_CXSHADOW);
                COLORREF cr2 = RGB(((nMakeSpec * int(GetRValue(cr))) / 100),
                                   ((nMakeSpec * int(GetGValue(cr))) / 100),
                                   ((nMakeSpec * int(GetBValue(cr))) / 100));
                pDC->SetPixel (rc.right-x-1, rc.top+y+SM_CXSHADOW, cr2);
            }
        }
    }
    else if ( bSelected || (bHotLight && !bInactive) )
    {
        COLORREF crHighLight = ::GetSysColor (COLOR_HIGHLIGHT);
        CPenDC pen (*pDC, crHighLight);
        CBrushDC brush (*pDC, crBackImg = GetDisabled() ? HLS_TRANSFORM (::GetSysColor (COLOR_MENU), +73, 0) : HLS_TRANSFORM (crHighLight, +70, -57));

        if ( bMenuBar )
        {
            CRect rc (pRect);

            rc.right -= TEXTPADDING_MNUBR;
            pDC->Rectangle (rc);
            rc.left = rc.right;
            rc.right += TEXTPADDING_MNUBR;
            pDC->FillSolidRect (rc, crMenuBkgnd);
        }
        else
        {
            pDC->Rectangle (pRect);
        }
    }
    else if ( !bMenuBar )
    {
        CRect rc (pRect);

        rc.right = DPISCALE(IMGWIDTH+IMGPADDING);
        pDC->FillSolidRect (rc, HLS_TRANSFORM (::GetSysColor (COLOR_3DFACE), +20, 0));
        rc.left = rc.right;
        rc.right = pRect->right;
        pDC->FillSolidRect (rc, ::GetSysColor (COLOR_MENU));
    }
    else
    {
        pDC->FillSolidRect (pRect, crMenuBkgnd);
    }
    if ( GetSeparator() )
    {
        int SepY = (pRect->top+pRect->bottom)/2;
        CPenDC pen (*pDC, 0x808080);
        pDC->MoveTo (pRect->left+IMGWIDTH+IMGPADDING+TEXTPADDING,  SepY);
        pDC->LineTo (pRect->right-1, SepY);

        ++SepY;
        pen.Color (0xFFFFFF);
        pDC->MoveTo (pRect->left+IMGWIDTH+IMGPADDING+TEXTPADDING,  SepY);
        pDC->LineTo (pRect->right-1, SepY);
    }
    else
    {
        CRect rc (pRect);
        CString sCaption;

        if ( GetCaption (sCaption) > 0 )
        {
            pDC->SetTextColor (bInactive ? ::GetSysColor (COLOR_3DSHADOW) : (GetDisabled() ? HLS_TRANSFORM (::GetSysColor (COLOR_3DFACE), -18, 0) : ::GetSysColor (COLOR_MENUTEXT)));
            pDC->SetBkMode (TRANSPARENT);

            BOOL bKeyboardCues = true;
            ::SystemParametersInfo (SPI_GETKEYBOARDCUES, 0, &bKeyboardCues, 0);
            DWORD dwHidePrefix = ( bNoAccel && !bKeyboardCues ) ? DT_HIDEPREFIX : 0;

            if ( bMenuBar )
            {
                rc.right -= TEXTPADDING_MNUBR;
                pDC->DrawText (sCaption, rc, DT_SINGLELINE|DT_VCENTER|DT_CENTER|dwHidePrefix);
            }
            else
            {
                CBoldDC bold (*pDC, GetDefault());

                rc.left = DPISCALE(IMGWIDTH+IMGPADDING+TEXTPADDING);
                pDC->DrawText (sCaption, rc, DT_SINGLELINE|DT_VCENTER|DT_LEFT|dwHidePrefix);

                CString sShortCut;

                if ( GetShortCut (sShortCut) > 0 )
                {
                    rc.right -= DPISCALE(TEXTPADDING+5);
                    pDC->DrawText (sShortCut, rc, DT_SINGLELINE|DT_VCENTER|DT_RIGHT);
                }
                if ( GetChecked() )
                {
                    COLORREF crHighLight = ::GetSysColor (COLOR_HIGHLIGHT);
                    CPenDC pen (*pDC, crHighLight);
                    CBrushDC brush (*pDC, crBackImg = GetDisabled() ? HLS_TRANSFORM (::GetSysColor (COLOR_MENU), +73, 0) :
                                                                      (bSelected ? HLS_TRANSFORM (crHighLight, +50, -50) : HLS_TRANSFORM (crHighLight, +70, -57)));
                    pDC->Rectangle (CRect (pRect->left+1, pRect->top+1, pRect->left+DPISCALE(IMGWIDTH+4), pRect->bottom-1));
                }
                if ( m_ImgDesc.m_hImgList != NULL && m_ImgDesc.m_nIndex != -1 )
                {
                    bool bOver = !GetDisabled() && bSelected;

                    if ( GetDisabled() || (bSelected && !GetChecked()) )
                    {
                        HICON hIcon = ImageList_ExtractIcon (NULL, m_ImgDesc.m_hImgList, m_ImgDesc.m_nIndex);
                        pDC->DrawState (CPoint (pRect->left + ( bOver ? 4 : 3 ), rc.top + ( bOver ? 4 : 3 )),
                                        CSize (IMGWIDTH, IMGHEIGHT), hIcon, DSS_MONO,
                                        CBrush (bOver ? HLS_TRANSFORM (::GetSysColor (COLOR_HIGHLIGHT), +50, -66) : HLS_TRANSFORM (::GetSysColor (COLOR_MENU), -27, 0)));
                        DestroyIcon (hIcon);
                    }
                    if ( !GetDisabled() )
                    {
                        ::ImageList_Draw (m_ImgDesc.m_hImgList, m_ImgDesc.m_nIndex, pDC->m_hDC,
                                          pRect->left+( (bSelected && !GetChecked()) ? 2 : 3 ), rc.top+( (bSelected && !GetChecked()) ? 2 : 3 ), ILD_TRANSPARENT);
                    }
                }
                else if ( GetChecked() )
                {
                    // Draw the check mark
                    ULONG oldLayout = pDC->GetLayout();
                    pDC->SetLayout(oldLayout | LAYOUT_BITMAPORIENTATIONPRESERVED);

                    if (oldLayout & LAYOUT_RTL)
                        rc.left = pRect->left - IMGWIDTH/4+1;
                    else
                        rc.left = pRect->left + IMGWIDTH/4+1;
                    rc.right = rc.left + DPISCALE(IMGWIDTH)-2;

                    if ( GetRadio() )
                    {
                        CPoint ptCenter = rc.CenterPoint();
                        COLORREF crBullet = GetDisabled() ? HLS_TRANSFORM (::GetSysColor (COLOR_MENU), -27, 0) : ::GetSysColor (COLOR_MENUTEXT);
                        CPenDC pen (*pDC, crBullet);
                        CBrushDC brush (*pDC, crBullet);

                        pDC->Ellipse (CRect (ptCenter.x-4, ptCenter.y-3, ptCenter.x+3, ptCenter.y+4));
                        pDC->SetPixel (ptCenter.x+1, ptCenter.y+2, crBackImg);
                    }
                    else
                    {
                        pDC->SetBkColor (crBackImg);
                        HBITMAP hBmp = LoadBitmap (NULL, MAKEINTRESOURCE(OBM_CHECK));
                        pDC->DrawState (CPoint (rc.left,rc.top+3), CSize(rc.Size()), hBmp, DSS_NORMAL, (HBRUSH)NULL);
                        DeleteObject (hBmp);
                    }

                    pDC->SetLayout(oldLayout);
                }
            }
        }
    }
    return bMenuBarItemSelected;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
CMap <int, int, CString, CString&> CMenuXP::ms_sCaptions;
CMap <HMENU, HMENU, CString, CString&> CMenuXP::ms_sSubMenuCaptions;
CMap <int, int, CImgDesc, CImgDesc&> CMenuXP::ms_Images;
CMap <HMENU, HMENU, CImgDesc, CImgDesc&> CMenuXP::ms_SubMenuImages;
double CMenuXP::m_dpiScale;

///////////////////////////////////////////////////////////////////////////////
void CMenuXP::SetXPLookNFeel (CWnd* pWnd, HMENU hMenu, bool bXPLook, bool bMenuBar)
{
    if ( !bXPLook )
    {
        // TODO: Remove the ownerdraw style ?
        return;
    }

    CDC *pDC = pWnd->GetDC();
    m_dpiScale = pDC->GetDeviceCaps(LOGPIXELSX) / 96.0;
    pWnd->ReleaseDC(pDC);

//    TRACE(_T("Referenced captions : %i\n"), ms_sCaptions.GetCount()+ms_sSubMenuCaptions.GetCount());
    // Clean up old references...
    // ... for captions
    POSITION pos = ms_sSubMenuCaptions.GetStartPosition();

    while ( pos != NULL )
    {
        HMENU hSubMenu;
        CString sBuff;

        ms_sSubMenuCaptions.GetNextAssoc (pos, hSubMenu, sBuff);

        if ( !::IsMenu (hSubMenu) )
        {
            ms_sSubMenuCaptions.RemoveKey (hSubMenu);
        }
    }
    // ... for images
    pos = ms_SubMenuImages.GetStartPosition();

    while ( pos != NULL )
    {
        HMENU hSubMenu;
        CImgDesc ImgDesc;

        ms_SubMenuImages.GetNextAssoc (pos, hSubMenu, ImgDesc);

        if ( !::IsMenu (hSubMenu) )
        {
            ms_SubMenuImages.RemoveKey (hSubMenu);
        }
    }
    ASSERT(hMenu != NULL);
    int nItemCount = ::GetMenuItemCount (hMenu);
    MENUITEMINFO mii = { sizeof MENUITEMINFO, MIIM_ID|MIIM_TYPE|MIIM_SUBMENU };
    CClientDC cDC (AfxGetMainWnd());
    TEXTMETRIC tm;
    CFontDC font (cDC, DEFAULT_GUI_FONT);

    cDC.GetTextMetrics (&tm);

    int nHeight = max(tm.tmHeight+2,IMGHEIGHT)+4;
    int nSepHeight = tm.tmHeight/2+2;
    int nCaptionLength = 0;
    int nShortCutLength = 0;
    CPtrList* pListControlBars = NULL;

    if ( pWnd != NULL && !bMenuBar )
    {
        if ( pWnd->IsKindOf (RUNTIME_CLASS(CFrameWnd)) && !((CFrameWnd*)pWnd)->m_listControlBars.IsEmpty() )
        {
            pListControlBars = &((CFrameWnd*)pWnd)->m_listControlBars;
        }
        else
        {
            CFrameWnd* pFrame = pWnd->GetParentFrame();
            if ( pFrame != NULL )
            {
                pListControlBars = &pFrame->m_listControlBars;
            }
        }
    }

    ULONG LastSeenId = 0;

    for ( int i = 0; i < nItemCount; i++ )
    {
        TCHAR sCaption[256] = _T("");
        mii.dwTypeData = sCaption;
        mii.cch = 255;
        mii.fMask &= ~MIIM_DATA;
        ::GetMenuItemInfo (hMenu, i, true, &mii);

        if ( (mii.fType & MFT_OWNERDRAW) == 0 && (!bMenuBar || (mii.fType & MFT_BITMAP) == 0) )
        {
            mii.fType |= MFT_OWNERDRAW;

            if ( bMenuBar )
            {
                CRect rcText (0, 0, 1000, 0);

                cDC.DrawText (sCaption, (int)_tcslen (sCaption), rcText, DT_SINGLELINE|DT_LEFT|DT_CALCRECT);
                mii.dwItemData = MAKELONG(MAKEWORD(0, 0), rcText.Width());
                mii.fMask |= MIIM_DATA;
            }

            if (LastSeenId && mii.wID == LastSeenId) {
                // the SuRun utility adds two consecutive entries with the same
                // ID number, this confuses our id-based  ownerdraw system, so
                // we get rid of the second entry, which should be a separator
                ::DeleteMenu(hMenu, i, MF_BYPOSITION);
                --i;
                --nItemCount;
                continue;
            }
            LastSeenId = mii.wID;

            ::SetMenuItemInfo (hMenu, i, true, &mii);

            if ( (mii.fType & MFT_SEPARATOR) == 0 )
            {
                CString sBuff(sCaption);

                if ( mii.hSubMenu != NULL )
                {
                    ms_sSubMenuCaptions.SetAt (mii.hSubMenu, sBuff);
                }
                else
                {
                    ms_sCaptions.SetAt (mii.wID, sBuff);
                }
                if ( pListControlBars != NULL )
                {
                    POSITION pos = pListControlBars->GetHeadPosition();

                    while ( pos != NULL )
                    {
                        CControlBar* pBar = (CControlBar*)pListControlBars->GetNext (pos);
                        ASSERT(pBar != NULL);
                        TCHAR sClassName[256];

                        ::GetClassName (pBar->m_hWnd, sClassName, lengthof (sClassName));

                        if ( !_tcsicmp (sClassName, _T("ToolbarWindow32")) )
                        {
                            TBBUTTONINFO tbbi = { sizeof(TBBUTTONINFO), TBIF_COMMAND|TBIF_IMAGE };

                            if ( pBar->SendMessage (TB_GETBUTTONINFO, mii.wID, (LPARAM)&tbbi) != -1 &&
                                 (UINT)tbbi.idCommand == mii.wID && tbbi.iImage != -1 )
                            {
                                CImgDesc imgDesc ((HIMAGELIST)pBar->SendMessage (TB_GETIMAGELIST, 0, 0), tbbi.iImage);

                                if ( mii.hSubMenu != NULL )
                                {
                                    ms_SubMenuImages.SetAt (mii.hSubMenu, imgDesc);
                                }
                                else
                                {
                                    ms_Images.SetAt (mii.wID, imgDesc);
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
        if ( !bMenuBar )
        {
            CMenuItem mnuItem (hMenu, i);
            int       nWidth = mnuItem.GetCaptionWidth (&cDC);

            if ( nWidth > nCaptionLength )
            {
                nCaptionLength = nWidth;
            }
            nWidth = mnuItem.GetShortCutWidth (&cDC);

            if ( nWidth > nShortCutLength )
            {
                nShortCutLength = nWidth;
            }
        }
    }
    if ( !bMenuBar )
    {
        for ( int j = 0; j < nItemCount; j++ )
        {
            mii.fMask = MIIM_TYPE;
            ::GetMenuItemInfo (hMenu, j, true, &mii);

            if ( (mii.fType & MFT_SEPARATOR) == 0 )
            {
                mii.dwItemData = MAKELONG(MAKEWORD(nHeight, nShortCutLength), nCaptionLength);
            }
            else
            {
                mii.dwItemData = nSepHeight;
            }
            mii.fMask = MIIM_DATA;
            ::SetMenuItemInfo (hMenu, j, true, &mii);
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
void CMenuXP::SetXPLookNFeelRecursive (CWnd* pWnd, HMENU hMenu, bool bXPLook, bool bMenuBar)
{
    MENUITEMINFO mii = { sizeof MENUITEMINFO, MIIM_SUBMENU };

    int nItemCount = ::GetMenuItemCount (hMenu);
    for ( int i = 0; i < nItemCount; i++ )
    {
        ::GetMenuItemInfo (hMenu, i, true, &mii);
        if ( mii.hSubMenu )
        {
            SetXPLookNFeelRecursive (pWnd, mii.hSubMenu, bXPLook, bMenuBar);
        }
    }

    SetXPLookNFeel(pWnd, hMenu, bXPLook, bMenuBar);
}


///////////////////////////////////////////////////////////////////////////////
bool CMenuXP::IsOwnerDrawn (HMENU hMenu)
{
    MENUITEMINFO mii = { sizeof MENUITEMINFO, MIIM_TYPE };

    ::GetMenuItemInfo (hMenu, 0, true, &mii);

    return (mii.fType & MFT_OWNERDRAW) != 0;
}

///////////////////////////////////////////////////////////////////////////////
void CMenuXP::SetMenuItemImage (UINT nID, HIMAGELIST hImgList, int nIndex)
{
    CImgDesc imgDesc (hImgList, nIndex);
    ms_Images.SetAt (nID, imgDesc);
}

///////////////////////////////////////////////////////////////////////////////
void CMenuXP::OnMeasureItem (MEASUREITEMSTRUCT* pMeasureItemStruct)
{
    if ( pMeasureItemStruct->CtlType == ODT_MENU )
    {
        pMeasureItemStruct->itemHeight = LOBYTE(LOWORD(pMeasureItemStruct->itemData));

        if ( pMeasureItemStruct->itemHeight == 0 )
        {
            // This is a menubar item
            pMeasureItemStruct->itemWidth = HIWORD(pMeasureItemStruct->itemData) + TEXTPADDING_MNUBR;
        }
        else
        {
            pMeasureItemStruct->itemWidth = IMGWIDTH + IMGPADDING + HIWORD(pMeasureItemStruct->itemData) + TEXTPADDING + HIBYTE(LOWORD(pMeasureItemStruct->itemData)) + TEXTPADDING + 4;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
bool CMenuXP::OnDrawItem (DRAWITEMSTRUCT* pDrawItemStruct, HWND hWnd)
{
    if ( pDrawItemStruct->CtlType != ODT_MENU )
    {
        return false;
    }
    ASSERT (pDrawItemStruct->CtlType == ODT_MENU);
    CBufferDC cDC (pDrawItemStruct->hDC, pDrawItemStruct->rcItem);
    CMenuItem item ((HMENU)pDrawItemStruct->hwndItem, pDrawItemStruct->itemID, false);
    CFontDC font (cDC, DEFAULT_GUI_FONT);

    if ( item.Draw (&cDC, &pDrawItemStruct->rcItem, (pDrawItemStruct->itemState&ODS_SELECTED)!=0, LOBYTE(LOWORD(pDrawItemStruct->itemData))==0, (pDrawItemStruct->itemState&ODS_HOTLIGHT)!=0, (pDrawItemStruct->itemState&ODS_INACTIVE)!=0, (pDrawItemStruct->itemState&ODS_NOACCEL)!=0) )
    {
        CRect rc;
        ::GetMenuItemRect (hWnd, (HMENU)pDrawItemStruct->hwndItem, 0, rc);
    }
    CMenuItem::ms_nCheck++;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
LRESULT CMenuXP::OnMenuChar (HMENU hMenu, UINT nChar, UINT nFlags)
{
    if ( (nFlags & (MF_POPUP|MF_SYSMENU)) == MF_POPUP || nFlags == 0 )
    {
        int nItemCount = ::GetMenuItemCount (hMenu);

        nChar = toupper (nChar);

        for ( int i = 0; i < nItemCount; i++ )
        {
            CMenuItem mnuItem (hMenu, i);
            CString sCaption;

            mnuItem.GetCaption (sCaption);
            sCaption.MakeUpper();

            for ( int nPos = sCaption.GetLength()-2; nPos >= 0; nPos-- )
            {
                if ( sCaption[nPos] == '&' && (UINT)toupper (sCaption[nPos+1]) == nChar &&
                     (nPos == 0 || sCaption[nPos-1] != '&') )
                {
                    return MAKELRESULT(i,2);
                }
            }
        }
    }
    return 0;
}
