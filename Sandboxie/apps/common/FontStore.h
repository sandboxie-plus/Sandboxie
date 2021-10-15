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
// Font Store
//---------------------------------------------------------------------------


#ifndef _MY_FONTSTORE_H
#define _MY_FONTSTORE_H


#include <afxcmn.h>


class CFontStore : public CMapStringToPtr
{

    static CFontStore *m_instance;

public:

    static CFont *Get(const CString &TypeFaceName, int PointSize,
                      int Weight = FW_NORMAL);

};


#endif // _MY_FONTSTORE_H
