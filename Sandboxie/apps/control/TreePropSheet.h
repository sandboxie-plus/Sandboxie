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


#if !defined(AFX_TREEPROPSHEET_H__50695CFB_FCE4_4188_ADB4_BF05A5488E41__INCLUDED_)
#define AFX_TREEPROPSHEET_H__50695CFB_FCE4_4188_ADB4_BF05A5488E41__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PropPageFrame.h"
#include "BaseDialog.h"
#include <afxtempl.h>

namespace TreePropSheet
{

/**
A property sheet, which can use a tree control instead of a tab
control, to give the user access to the different pages.

You can use it exactly the same way, as a CPropertySheet object.
Simply create CPropertyPage objects and add them via AddPage() to
the sheet. If you would like to use the tree view mode (default),
you can specify the path of the pages in the tree, by their name:
The names of the pages can contain
double colons ("::"), which will specify the path of that page in the
tree control. I.e. if you have three pages with the following names:
1. _T("Appearance::Toolbars")
2. _T("Appearance::Menus")
3. _T("Directories")
the tree would look as follow:
\verbatim
Appearance
|
+-Toolbars
|
+-Menus

Directories
\endverbatim
If you would like to use a double colon, which should not be
interpreted as a path seperator, prefix it with a backslash ("\\::").

To disable tree view mode and use the standard tabbed mode, call
the SetTreeViewMode() method. This also allows you, to enable page
captions and tree images for tree view mode. If you would like to
have images in the tree, but not all of your pages specify images or
there are tree view items, which are not attached to a page (only
parent items for real page items), you have to set default images
using the SetTreeDefaultImages() method -- otherwise their may appear
display errors.

If the user selects a tree view item, which does not belong to a page,
because it is just a parent item for real page items, no page will
be displayed, instead a message will be displayed, that can be set
via SetEmptyPageText().

@author Sven Wiegand
*/
class /*AFX_EXT_CLASS*/ CTreePropSheet : public CPropertySheet
{
    DECLARE_DYNAMIC(CTreePropSheet)

// Construction/Destruction
public:
    CTreePropSheet();
    CTreePropSheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
    CTreePropSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
    virtual ~CTreePropSheet();

// Operations
public:
    /**
    Call this method, if you would like to use a tree control to browse
    the pages, instead of the tab control.

    This method needs to becalled, before DoModal() or Create(). If the
    window has already been created, the method will fail.

    @param bTreeViewMode
        Pass TRUE to provide a tree view control instead of a tab control
        to browse the pages, pass FALSE to use the normal tab control.
    @param bPageCaption
        TRUE if a caption should be displayed for each page. The caption
        contains the page title and an icon if specified with the page.
        Ignored if bTreeViewMode is FALSE.
    @param bTreeImages
        TRUE if the page icons should be displayed in the page tree,
        FALSE if there should be no icons in the page tree. Ignored if
        bTreeViewMode is FALSE. If not all of your pages are containing
        icons, or if there will be empty pages (parent nodes without a
        related page, you need to call SetTreeDefaultImages() to avoid
        display errors.

    @return
        TRUE on success or FALSE, if the window has already been created.
    */
    BOOL SetTreeViewMode(BOOL bTreeViewMode = TRUE, BOOL bPageCaption = FALSE, BOOL bTreeImages = FALSE);

    /**
    Specifies the width of the tree control, when the sheet is in tree
    view mode. The default value (if this method is not called) is 150
    pixels.

    This method needs to be called, before DoModeal() or Create().
    Otherwise it will fail.

    @param nWidth
        The width in pixels for the page tree.

    @return
        TRUE on success, FALSE otherwise (if the window has already been
        created).
    */
    BOOL SetTreeWidth(int nWidth);

    /**
    Specifies the text to be drawn on empty pages (pages for tree view
    items, that are not related to a page, because they are only
    parents for other items). This is only needed in tree view mode.

    The specified text can contains a single "%s" placeholder which
    will be replaced with the title of the empty page.
    */
    void SetEmptyPageText(LPCTSTR lpszEmptyPageText);

    /**
    Allows you to specify, how the empty page message (see
    SetEmptyPageText()) should be drawn.

    @param dwFormat
        A combination of the DT_* flags available for the Win32-API
        function DrawText(), that should be used to draw the text.
        The default value is:
        \code
        DT_CENTER|DT_VCENTER|DT_NOPREFIX|DT_SINGLELINE
        \endcode

    @return
        The previous format.
    */
    DWORD SetEmptyPageTextFormat(DWORD dwFormat);

    //@{
    /**
    Defines the images, that should be used for pages without icons and
    for empty parent nodes. The list contains exactly to images:
    1. An image that should be used for parent tree nodes, without a
       page asignd.
    2. An image that should be used for pages, which are not specifying
       any icons.
    Standard image size is 16x16 Pixels, but if you call this method
    before creating the sheet, the size of image 0 in this list will
    be assumed as your preferred image size and all other icons must
    have the same size.

    @param pImages
        Pointer to an image list with exactly to images, that should be
        used as default images. The images are copied to an internal
        list, so that the given list can be deleted after this call.
    @param unBitmapID
        Resource identifier for the bitmap, that contains the default
        images. The resource should contain exactly to images.
    @param cx
        Width of a singe image in pixels.
    @param crMask
        Color that should be interpreted as transparent.

    @return
        TRUE on success, FALSE otherwise.
    */
    BOOL SetTreeDefaultImages(CImageList *pImages);
    BOOL SetTreeDefaultImages(UINT unBitmapID, int cx, COLORREF crMask);
    //@}

    /**
    Returns a pointer to the tree control, when the sheet is in
    tree view mode, NULL otherwise.
    */
    CTreeCtrl* GetPageTreeControl();

    /* Set text for checkbox */
    void SetAutoApply(const CString &text, BOOL def);

    /* TRUE if page is changing.  This can be used to tell if KillActive
       is called during page switch or for Apply or OK */
    BOOL IsSelChanging() const;

    /* TRUE if the checkbox is set, FALSE otherwise */
    BOOL IsAutoApplyChecked() const;

    /* Set RTL mode. */
    void SetLayoutRTL();

    /* Add minimize button */
    void AddMinimizeButton();


// Public helpers
public:
    //@{
    /**
    This helper allows you to easily set the icon of a property page.

    This static method does nothing more, than extracting the specified
    image as an icon from the given image list and assign the
    icon-handle to the hIcon property of the pages PROPSHEETPAGE
    structure (m_psp) and modify the structures flags, so that the
    image will be recognized.

    You need to call this method for a page, before adding the page
    to a property sheet.

    @important
    If you are using the CImageList-version, you are responsible for
    destroying the extracted icon with DestroyIcon() or the static
    DestroyPageIcon() method.

    @see DestroyPageIcon()

    @param pPage
        Property page to set the image for.
    @param hIcon
        Handle to icon that should be set for the page.
    @param unIconId
        Ressource identifier for the icon to set.
    @param Images
        Reference of the image list to extract the icon from.
    @param nImage
        Zero based index of the image in pImages, that should be used
        as an icon.

    @return
        TRUE on success, FALSE if an error occured.
    */
    static BOOL SetPageIcon(CPropertyPage *pPage, HICON hIcon);
    static BOOL SetPageIcon(CPropertyPage *pPage, UINT unIconId);
    static BOOL SetPageIcon(CPropertyPage *pPage, CImageList &Images, int nImage);
    //@}

    /**
    Checks, if the PSP_USEHICON flag is set in the PROPSHEETPAGE struct;
    If this is the case, the flag will be removed and the icon
    specified by the hIcon attribute of the PROPSHEETPAGE struct will
    be destroyed using DestroyIcon().

    @note
    You only have to call DestroyIcon() for icons, that have been
    created using CreateIconIndirect() (i.e. used by
    CImageList::ExtractIcon()).

    @return
        TRUE on success, FALSE if the PSP_USEHICON flag was not set or
        if the icon handle was NULL.
    */
    static BOOL DestroyPageIcon(CPropertyPage *pPage);

// Overridable implementation helpers
protected:
    /**
    Will be called to generate the message, that should be displayed on
    an empty page, when the sheet is in tree view mode

    This default implementation simply returns lpszEmptyPageMessage
    with the optional "%s" placeholder replaced by lpszCaption.

    @param lpszEmptyPageMessage
        The string, set by SetEmptyPageMessage(). This string may contain
        a "%s" placeholder.
    @param lpszCaption
        The title of the empty page.
    */
    virtual CString GenerateEmptyPageMessage(LPCTSTR lpszEmptyPageMessage, LPCTSTR lpszCaption);

    /**
    Will be called during creation process, to create the CTreeCtrl
    object (the object, not the window!).

    Allows you to inject your own CTreeCtrl-derived classes.

    This default implementation simply creates a CTreeCtrl with new
    and returns it.
    */
    virtual CTreeCtrl* CreatePageTreeObject();

    /**
    Will be called during creation process, to create the object, that
    is responsible for drawing the frame around the pages, drawing the
    empty page message and the caption.

    Allows you to inject your own CPropPageFrame-derived classes.

    This default implementation simply creates a CPropPageFrameTab with
    new and returns it.
    */
    virtual CPropPageFrame* CreatePageFrame();

// Implementation helpers
protected:
    /**
    Aligns the OK,Cancel,Apply buttons against the right edge
    */
    int AlignButtonsToRight();

    /**
    Moves all childs by the specified amount of pixels.

    @param nDx
        Pixels to move the childs in horizontal direction (can be
        negative).
    @param nDy
        Pixels to move the childs in vertical direction (can be
        negative).
    */
    void MoveChildWindows(int nDx, int nDy);

    /**
    Refills the tree that contains the entries for the several pages.
    */
    void RefillPageTree();

    /**
    Creates the specified path in the page tree and returns the handle
    of the most child item created.

    @param lpszPath
        Path of the item to create (see description of this class).
    @param hParentItem
        Handle of the item under which the path should be created or
        TVI_ROOT to start from the root.
    */
    HTREEITEM CreatePageTreeItem(LPCTSTR lpszPath, HTREEITEM hParent = TVI_ROOT);

    /**
    Splits the given path into the topmost item and the rest. See
    description of this class for detailed path information.

    I.e. when given the string "Appearance::Toolbars::Customize", the
    method will return "Appearance" and after the call strRest will
    be "Toolbars::Customize".
    */
    CString SplitPageTreePath(CString &strRest);

    /**
    Tries to deactivate the current page, and hides it if successfull,
    so that an empty page becomes visible.

    @return
        TRUE if the current page has been deactivated successfully,
        FALSE if the currently active page prevents a page change.
    */
    BOOL KillActiveCurrentPage();

    /**
    Returns the page tree item, that representates the specified page
    or NULL, if no such icon exists.

    @param nPage
        Zero based page index, for which the item to retrieve.
    @param hRoot
        Item to start the search at or TVI_ROOT to search the whole
        tree.
    */
    HTREEITEM GetPageTreeItem(int nPage, HTREEITEM hRoot = TVI_ROOT);

    /**
    Selects and shows the item, representing the specified page.

    @param nPage
        Zero based page index.

    @return
        TRUE on success, FALSE if no item does exist for the specified
        page.
    */
    BOOL SelectPageTreeItem(int nPage);

    /**
    Selects and shows the tree item for the currently active page.

    @return
        TRUE on success, FALSE if no item exists for the currently active
        page or if it was not possible to get information about the
        currently active page.
    */
    BOOL SelectCurrentPageTreeItem();

    /**
    Updates the caption for the currently selected page (if the caption
    is enabled).
    */
    void UpdateCaption();

    /**
    Activates the previous page in the page order or the last one, if
    the current one is the first.

    This method does never fail.
    */
    void ActivatePreviousPage();

    /**
    Activates the next page in the page order or the first one, if the
    current one is the last.

    This method does never fail.
    */
    void ActivateNextPage();

    /**
    Collapses all sub-trees except the one containing the newly selected
    item.
    */
    void CollapseAllTreeItems(HTREEITEM hTreeItemNew);


// Overridings
protected:
    //{{AFX_VIRTUAL(CTreePropSheet)
    public:
    virtual BOOL OnInitDialog();
    //}}AFX_VIRTUAL

// Message handlers
protected:
    //{{AFX_MSG(CTreePropSheet)
    afx_msg void OnDestroy();
    //}}AFX_MSG
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
    //
    afx_msg LRESULT OnAddPage(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnRemovePage(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSetCurSel(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSetCurSelId(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnIsDialogMessage(WPARAM wParam, LPARAM lParam);

    afx_msg void OnPageTreeSelChanging(NMHDR *pNotifyStruct, LRESULT *plResult);
    afx_msg void OnPageTreeSelChanged(NMHDR *pNotifyStruct, LRESULT *plResult);
    DECLARE_MESSAGE_MAP()

// Properties
private:
    /** TRUE if we should use the tree control instead of the tab ctrl. */
    BOOL m_bTreeViewMode;

    /** The tree control */
    CTreeCtrl *m_pwndPageTree;

    /** The frame around the pages */
    CPropPageFrame *m_pFrame;

    /**
    TRUE, if a tree item selection by OnPageTreeSelChanged() is
    performed currently.
    */
    BOOL m_bPageTreeSelChangedActive;

    /** TRUE if a page caption should be displayed, FALSE otherwise. */
    BOOL m_bPageCaption;

    /** TRUE if images should be displayed in the tree. */
    BOOL m_bTreeImages;

    /** Images to be displayed in the tree control. */
    CImageList m_Images;

    /** Default images. */
    CImageList m_DefaultImages;

    /**
    Message to be displayed on empty pages. May contain a "%s"
    placeholder which will be replaced by the caption of the empty
    page.
    */
    CString m_strEmptyPageMessage;

    /** The width of the page tree control in pixels. */
    int m_nPageTreeWidth;

    /** Checkbox control **/
    CButton m_checkbox;
    CString m_checkbox_text;
    BOOL m_checkbox_default;

    /** Layout direction **/
    BOOL m_bLayoutRTL;

    /** Base dialog for minimize buttons **/
    CBaseDialog *m_pBaseDialog;

// Static Properties
private:
    /** The id of the tree view control, that shows the pages. */
    static const UINT s_unPageTreeId;
};


} //namespace TreePropSheet

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_TREEPROPSHEET_H__50695CFB_FCE4_4188_ADB4_BF05A5488E41__INCLUDED_
