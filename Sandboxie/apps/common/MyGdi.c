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
// GDI Plus Wrappers
//---------------------------------------------------------------------------


#define COBJMACROS
#include <objbase.h>

#include <windows.h>
#include "MyGdi.h"

#include <common/defines.h>
#if _MSC_VER == 1200        // Visual C++ 6.0
typedef ULONG ULONG_PTR;
#endif


//---------------------------------------------------------------------------
// GdiPlus Imports
//---------------------------------------------------------------------------


struct GdiplusStartupInput
{
    UINT32 GdiplusVersion;             // Must be 1
    ULONG_PTR      DebugEventCallback; // Ignored on free builds
    BOOL SuppressBackgroundThread;     // FALSE unless you're prepared to call
                                       // the hook/unhook functions properly
    BOOL SuppressExternalCodecs;       // FALSE unless you want GDI+ only to use
                                       // its internal image codecs.
};

ULONG GdiplusStartup(
    ULONG_PTR *token, struct GdiplusStartupInput *input, ULONG_PTR *output);

typedef ULONG_PTR GpImage;
typedef ULONG_PTR GpBitmap;

ULONG GdipCreateBitmapFromStream(IStream* stream, GpBitmap **bitmap);
ULONG GdipCreateHBITMAPFromBitmap(
    GpBitmap* bitmap, HBITMAP* hbmReturn, ULONG background);
ULONG GdipDisposeImage(GpImage *image);


//---------------------------------------------------------------------------
// MyGdiInit
//---------------------------------------------------------------------------


void MyGdi_Init(void)
{
    static ULONG_PTR token = 0;
    struct GdiplusStartupInput input;
    memzero(&input, sizeof(input));
    input.GdiplusVersion = 1;
    GdiplusStartup(&token, &input, NULL);
}


//---------------------------------------------------------------------------
// MyGdi_CreateResourceStream
//---------------------------------------------------------------------------


void *MyGdi_CreateResourceStream(const WCHAR *rsrcName)
{
    HINSTANCE hInst;
    HRSRC hResource;
    ULONG rsrcSize;
    const void *rsrcData;
    HGLOBAL hGlobal;
    void *pGlobal;
    IStream *pStream;

    hInst = GetModuleHandle(NULL);

    hResource = FindResource(hInst, rsrcName, L"IMAGE");
    if (! hResource)
        return NULL;

    rsrcSize = SizeofResource(hInst, hResource);
    if (! rsrcSize)
        return NULL;

    rsrcData = LockResource(LoadResource(hInst, hResource));
    if (! rsrcData)
        return NULL;

    hGlobal = GlobalAlloc(GMEM_MOVEABLE, rsrcSize);
    if (! hGlobal)
        return NULL;
    pGlobal = GlobalLock(hGlobal);
    if (! pGlobal) {
        GlobalFree(hGlobal);
        return NULL;
    }
    memcpy(pGlobal, rsrcData, rsrcSize);
    GlobalUnlock(hGlobal);

    if (CreateStreamOnHGlobal(hGlobal, TRUE, &pStream) != S_OK) {
        GlobalFree(hGlobal);
        pStream = NULL;
    }

    return pStream;
}


//---------------------------------------------------------------------------
// MyGdi_CreateFromResource
//---------------------------------------------------------------------------


HBITMAP MyGdi_CreateFromResource(const WCHAR *rsrcName)
{
    HBITMAP hBitmap = NULL;

    IStream *pStream = (IStream *)MyGdi_CreateResourceStream(rsrcName);
    if (pStream) {

        GpBitmap *gdiBitmap;
        if (GdipCreateBitmapFromStream(pStream, &gdiBitmap) == S_OK) {

            if (GdipCreateHBITMAPFromBitmap(gdiBitmap, &hBitmap, -1) != S_OK)
                hBitmap = NULL;

            GdipDisposeImage((GpImage *)gdiBitmap);
        }

        IStream_Release(pStream);
    }

    return hBitmap;
}
