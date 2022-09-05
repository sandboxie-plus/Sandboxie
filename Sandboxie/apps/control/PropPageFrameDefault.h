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
*	$History:$
*
*********************************************************************/


#if !defined(AFX_PROPPAGEFRAMEDEFAULT_H__5C5B7AC9_2DF5_4E8C_8F5E_DE2CC04BBED7__INCLUDED_)
#define AFX_PROPPAGEFRAMEDEFAULT_H__5C5B7AC9_2DF5_4E8C_8F5E_DE2CC04BBED7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PropPageFrame.h"


namespace TreePropSheet
{


/**
An implementation of CPropPageFrame, that works well for Windows XP
styled systems and older windows versions (without themes).

@author Sven Wiegand
*/
class /*AFX_EXT_CLASS*/ CPropPageFrameDefault : public CWnd,
                                            public CPropPageFrame
{
// construction/destruction
public:
	CPropPageFrameDefault();
	virtual ~CPropPageFrameDefault();

// operations
public:

// overridings
public:
	virtual BOOL Create(DWORD dwWindowStyle, const RECT &rect, CWnd *pwndParent, UINT nID);
	virtual CWnd* GetWnd();
	virtual void SetCaption(LPCTSTR lpszCaption, HICON hIcon = NULL);
	
protected:
	virtual CRect CalcMsgArea();
	virtual CRect CalcCaptionArea();
	virtual void DrawCaption(CDC *pDc, CRect rect, LPCTSTR lpszCaption, HICON hIcon);

// Implementation helpers
protected:
	/**
	Fills a rectangular area with a gradient color starting at the left
	side with the color clrLeft and ending at the right sight with the
	color clrRight.

	@param pDc
		Device context to draw the rectangle in.
	@param rect
		Rectangular area to fill.
	@param clrLeft
		Color on the left side.
	@param clrRight
		Color on the right side.
	*/
	void FillGradientRectH(CDC *pDc, const RECT &rect, COLORREF clrLeft, COLORREF clrRight);

	/**
	Returns TRUE if Windows XP theme support is available, FALSE 
	otherwise.
	*/
	BOOL ThemeSupport() const;

protected:
	//{{AFX_VIRTUAL(CPropPageFrameDefault)
	//}}AFX_VIRTUAL

// message handlers
protected:
	//{{AFX_MSG(CPropPageFrameDefault)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// attributes
protected:
	/** 
	Image list that contains only the current icon or nothing if there
	is no icon.
	*/
	CImageList m_Images;
};


} //namespace TreePropSheet


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_PROPPAGEFRAMEDEFAULT_H__5C5B7AC9_2DF5_4E8C_8F5E_DE2CC04BBED7__INCLUDED_
