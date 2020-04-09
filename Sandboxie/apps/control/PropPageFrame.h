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


#if !defined(AFX_PROPPAGEFRAME_H__B968548B_F0B4_4C35_85DD_C44242A9D368__INCLUDED_)
#define AFX_PROPPAGEFRAME_H__B968548B_F0B4_4C35_85DD_C44242A9D368__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


namespace TreePropSheet
{


/**
An object of an CPropertyPageFrame-derived class is used by 
CTreePropSheet to display a frame for the property pages.

Derived classes have to implement at least the Create() and the 
GetWnd() method, which
returns a pointer to the CWnd-obect of the window, that has been
created by the call to the Create() method. An implementation can
provide the CWnd-object by using it as a property or by deriving
from CWnd or a more specialiced class. This way has been choosen 
instead of deriving CPropPageFrame from CWnd, to allow derived class
to derive from more specialized classes than CWnd 
(i.e. CStatic, etc.). From the WM_PAINT-handler of your derived class
you have to call the Draw()-method.

Most implementations will also implement the DrawCaption() and 
DrawMsg() methods, to provide custom drawing functionality.

@author Sven Wiegand
*/
class /*AFX_EXT_CLASS*/ CPropPageFrame
{
// Construction/Destruction
public:
	CPropPageFrame();
	virtual ~CPropPageFrame();

// Operations
public:
	/**
	Has to create a window with the specified properties.

	@param dwWindowStyle
		Standard window styles, to apply to the window to create.
	@param rect
		Position and size of the window to create.
	@param pwndParent
		Parent window. (Never be NULL).
	@param nID
		Window id.

	@return
		TRUE on success, FALSE otherwise.
	*/
	virtual BOOL Create(DWORD dwWindowStyle, const RECT &rect, CWnd *pwndParent, UINT nID) = 0;

	/**
	Returns a pointer to the window object, that represents the frame.
	*/
	virtual CWnd* GetWnd() = 0;

	/**
	Enables or disables page caption.

	This default implementation calls the SafeUpdateWindow() method
	with the caption rectangle, to force it to be redrawn.
	*/
	virtual void ShowCaption(BOOL bEnable);

	/**
	Returns TRUE if captions are enabled, FALSE otherwise.
	*/
	BOOL GetShowCaption() const;

	/**
	Sets the height of the caption in pixels. This value is ignored 
	until the caption is enabled by ShowCaption(TRUE).

	This default implementation calls the SafeUpdateWindow() method
	with the caption rectangle, to force it to be redrawn.
	*/
	virtual void SetCaptionHeight(int nCaptionHeight);

	/**
	Returns the caption height, that has been most recently set by a
	call to the SetCaptionHeight() method.
	*/
	int GetCaptionHeight() const;

	/**
	Sets caption text an icon.

	This default implementation calls the SafeUpdateWindow() method
	with the caption rectangle, to force it to be redrawn.

	@param lpszCaption
		Text to display for the caption.
	@param hIcon
		Icon to display for the caption.
	*/
	virtual void SetCaption(LPCTSTR lpszCaption, HICON hIcon = NULL);

	/**
	Returns the caption, that has been set most recently using the
	SetCaption() method.

	@param pIcon
		Pointer to a HICON variable, that should receive the handle of
		the currently set icon or NULL, if there is no icon or a NULL
		pointer, if the caller is not interested in the icon.

	@return
		The text that has been set most recently using the SetCaption()
		method.
	*/
	CString GetCaption(HICON *pIcon = NULL) const;

	/**
	Sets the text to display.

	This default implementation calls the SafeUpdateWindow() method
	with the message rectangle, to force it to be redrawn.
	*/
	virtual void SetMsgText(LPCTSTR lpszMsg);

	/**
	Returns the text currently displayed.
	*/
	CString GetMsgText() const;

	/**
	Specifies the format to draw the text with, set by SetMsgText().

	This default implementation calls the SafeUpdateWindow() method
	with the message rectangle, to force it to be redrawn.

	@param dwFormat
		Combination of the DT_* flags, specified by the Win32 function
		DrawText().
	*/
	virtual void SetMsgFormat(DWORD dwFormat);

	/**
	Returns the format to draw the text with, set by SetMsgText().

	@see SetMsgFormat()
	*/
	DWORD GetMsgFormat() const;

// Overridable implementation helpers
protected:
	/**
	Draws the whole frame including caption (if enabled) and message.

	This method calculates the rectangles for the message area and
	the caption area using the CalcMsgArea() and the CalcCaptionArea()
	methods, draws then the caption calling the DrawCaption() method
	(only if caption is enabled) and afterwards the message calling
	the DrawMsg() method.

	You should call this method from the WM_PAINT-handler of your
	derived class.

	@param pDc
		Device context to draw in.
	*/
	virtual void Draw(CDC *pDc);

	/**
	Calculates the area, the message, set by SetMsgText() should be
	displayed in. The returned rectangle (client coordinates) will be
	passed to DrawMsg().

	This default implementation calls the CalcCaptionArea() method,
	substracts the returned rectangle from the client area and returns
	the result.
	*/
	virtual CRect CalcMsgArea();

	/**
	Draws the message with the specified format.

	This default implementation draws the given msg using the specified
	properties.

	@param pDc
		Device context to draw in.
	@param rect
		Rectangle to draw the message in.
	@param lpszMsg
		Message to draw.
	@param dwFormat.
		Combination of the DT_* flags, specified by the Win32 function
		DrawText() to draw the message with.
	*/
	virtual void DrawMsg(CDC *pDc, CRect rect, LPCTSTR lpszMsg, DWORD dwFormat);

	/**
	Calculates the caption area. The caption area is the rectangular
	range, the current page's caption should be drawn in.

	The caption can be enabled or disabled using the ShowCaption()
	method. This method should return an empty rect, if the caption
	is disabled. If the caption is enabled the height of the rect 
	should be as large, as specified by the latest call to the 
	SetCaptionHeight() method.

	The rectangle, returned by this method will be passed to the
	DrawCaption() method to draw the caption.

	If the caption is enabled currently, this default implementation
	returns a rectangle, that has the width of the client area and
	the height, specified by the latest call to SetCaptionHeight(),
	starting and y-position 0. If the caption is disabled at the 
	moment, this method returns an empty rectangle with the width of 
	the client area.
	*/
	virtual CRect CalcCaptionArea();

	/**
	Draws the caption.

	This default implementation draws nothing.

	@param pDc
		Device context to draw in.
	@param rect
		Rectangle to draw the caption in.
	@param lpszCaption
		Text to display in the caption.
	@param hIcon
		Icon to display in the caption.
	*/
	virtual void DrawCaption(CDC *pDc, CRect rect, LPCTSTR lpszCaption, HICON hIcon);

// Implementation helpers
protected:
	/**
	If the m_hWnd property of the CWnd-object returend by GetWnd() 
	specifies a valid window, this method will invalidate the specified
	rectangle, to schedule it for repaint, otherwise the method will
	return without doing anything.

	@param lpRect
		Pointer to the rectangular area, that should be marked for 
		repainting or NULL, if the whole client area should be marked
		for repainting.
	*/
	void SafeUpdateWindow(LPCRECT lpRect = NULL);

// Properties
private:
	/** TRUE if the caption should be drawn, FALSE otherwise. */
	BOOL m_bShowCaption;
		
	/** Height of the caption in pixels, if it is enabled. */
	int m_nCaptionHeight;

	/** Text to display in the caption. */
	CString m_strCaption;

	/** 
	Icon to display in the caption or NULL if no icon should be 
	displayed.
	*/
	HICON m_hCaptionIcon;

	/** Message text to display. */
	CString m_strMsg;

	/** Style to use when drawing the message text m_strMsg. */
	DWORD m_dwMsgFormat;
};


} //namespace TreePropSheet


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_PROPPAGEFRAME_H__B968548B_F0B4_4C35_85DD_C44242A9D368__INCLUDED_
