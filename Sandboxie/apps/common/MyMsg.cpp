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
// MyMsg Internationalization Utility
//---------------------------------------------------------------------------

#include "..\control\stdafx.h"

#include <afxcmn.h>
#include "MyMsg.h"
#include "core/dll/sbiedll.h"
#include "common/defines.h"


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


const CString &CMyMsg::m_unknown = CString(L"???");


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CMyMsg::CMyMsg(ULONG msgid)
{
    Construct(SbieDll_FormatMessage0(msgid));
}


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CMyMsg::CMyMsg(ULONG msgid, const WCHAR *p1)
{
    Construct(SbieDll_FormatMessage1(msgid, p1));
}


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CMyMsg::CMyMsg(ULONG msgid, const WCHAR *p1, const WCHAR *p2)
{
    Construct(SbieDll_FormatMessage2(msgid, p1, p2));
}


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CMyMsg::CMyMsg(
    ULONG msgid, const WCHAR *p1, const WCHAR *p2, const WCHAR *p3)
{
    const WCHAR *ins[6];
    memzero((WCHAR *)ins, sizeof(ins));
    ins[1] = (WCHAR *)p1;
    ins[2] = (WCHAR *)p2;
    ins[3] = (WCHAR *)p3;
    Construct(SbieDll_FormatMessage(msgid, ins));
}


//---------------------------------------------------------------------------
// Construct
//---------------------------------------------------------------------------


void CMyMsg::Construct(WCHAR *str)
{
    if (str) {
        CString::operator=(str);
        LocalFree(str);
    } else
        CString::operator=(m_unknown);
}
