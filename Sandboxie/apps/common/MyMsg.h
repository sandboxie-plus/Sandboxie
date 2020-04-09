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


#ifndef _MY_MYMSG_H
#define _MY_MYMSG_H


#include "msgs/msgs.h"


class CMyMsg : public CString
{
    static const CString &m_unknown;

    void Construct(WCHAR *str);

public:

    CMyMsg(ULONG msgid);

    CMyMsg(ULONG msgid, const WCHAR *p1);

    CMyMsg(ULONG msgid, const WCHAR *p1, const WCHAR *p2);

    CMyMsg(ULONG msgid, const WCHAR *p1, const WCHAR *p2, const WCHAR *p3);
};


#endif // _MY_MYMSG_H
