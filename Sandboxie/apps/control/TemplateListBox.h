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
// Template List Box
//---------------------------------------------------------------------------


#ifndef _MY_TEMPLATELISTBOX_H
#define _MY_TEMPLATELISTBOX_H


class CTemplateListBox
{

public:

    static CString Prefix2;
    static CString Prefix1;
    static CString Prefix0;

    static CString GetTemplateTitle(const CString &TemplateName);
    static CString GetTemplateClass(const CString &TemplateName);

    static void Decorate(CString &text, BOOL enable, BOOL force);
    static void DecorateAster(CString &text);
    static BOOL IsAster(const CString &text);
    static BOOL IsCheck(const CString &text);
    static BOOL IsClear(const CString &text);

    static BOOL OnAddRemove(CWnd *wnd, BOOL enable, BOOL toggle = FALSE);
};


#endif // _MY_TEMPLATELISTBOX_H
