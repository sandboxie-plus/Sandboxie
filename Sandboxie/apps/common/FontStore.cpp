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

#include "stdafx.h"

#include "FontStore.h"
#include "common/defines.h"


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


CFontStore *CFontStore::m_instance = NULL;


//---------------------------------------------------------------------------
// Get
//---------------------------------------------------------------------------


CFont *CFontStore::Get(
    const CString &TypeFaceName, int PointSize, int Weight)
{
    if (! m_instance)
        m_instance = new CFontStore();

    CString key;
    key.Format(L"%s:%d:%d", TypeFaceName, PointSize, Weight);

    void *ptr;
    if (m_instance->Lookup(key, ptr))
        return (CFont *)ptr;

    HDC hDC = ::GetDC(NULL);
    int dpi = GetDeviceCaps(hDC, LOGPIXELSY);
    if (dpi > 96) {
        // reduce one point size for every 25% increase over the base 96 DPI
        PointSize -= (dpi - 96) / 24;
    }
    ::ReleaseDC(NULL, hDC);

    LOGFONT logfont;
    memzero(&logfont, sizeof(LOGFONT));
    logfont.lfCharSet = DEFAULT_CHARSET;
    logfont.lfHeight = PointSize * 10;  // see CreatePointFontIndirect in MFC
    logfont.lfWeight = Weight;
    int len = max(TypeFaceName.GetLength(), 30);
    wmemcpy(logfont.lfFaceName, TypeFaceName, len);
    logfont.lfFaceName[len] = L'\0';

    CFont *font = new CFont();
    font->CreatePointFontIndirect(&logfont, NULL);

    m_instance->SetAt(key, font);

    return font;
}
