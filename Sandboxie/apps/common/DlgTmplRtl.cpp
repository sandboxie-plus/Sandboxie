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
// Common Utility Functions
//---------------------------------------------------------------------------


#include "CommonUtils.h"


//---------------------------------------------------------------------------
// Common_DlgTmplRtl
//---------------------------------------------------------------------------


extern "C" void *Common_DlgTmplRtl(HINSTANCE hInst, const WCHAR *TmplName)
{
    HRSRC hResource = ::FindResource(hInst, TmplName, RT_DIALOG);
    HGLOBAL hGlobal = LoadResource(hInst, hResource);
    void  *DataPtr  = LockResource(hGlobal);

    if (DataPtr) {

        typedef struct {
            DWORD style;
            DWORD dwExtendedStyle;
        } DLGTMPL1;
        typedef struct {
            WORD dlgVer;
            WORD signature;
            DWORD helpID;
            DWORD exStyle;
            DWORD style;
        } DLGTMPL2;

        ULONG *exStyle;
        ULONG DataSize = SizeofResource(hInst, hResource);
        void *NewTmpl = HeapAlloc(
                GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, DataSize);
        memcpy(NewTmpl, DataPtr, DataSize);

        if (((DLGTMPL2 *)NewTmpl)->signature == 0xFFFF)
            exStyle = &((DLGTMPL2 *)NewTmpl)->exStyle;
        else
            exStyle = &((DLGTMPL1 *)NewTmpl)->dwExtendedStyle;
        (*exStyle) |= WS_EX_LAYOUTRTL;

        return NewTmpl;
    }

    return NULL;
}
