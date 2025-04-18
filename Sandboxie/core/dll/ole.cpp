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
// Ole32 -- Clipboard Management
//---------------------------------------------------------------------------


extern "C" {
#include "dll.h"
#include "gui_p.h"
#include "gdi.h"
#include "common/my_version.h"
}

#include <shellapi.h>
#include <shlobj.h>
#include <new>
#include <stdlib.h>


#undef   WITH_PRINT_FORMAT


//---------------------------------------------------------------------------
// Function Pointers
//---------------------------------------------------------------------------


typedef HRESULT (*P_OleSetClipboard)(IDataObject *pDataObject);

typedef HRESULT (*P_ReleaseStgMedium)(STGMEDIUM *pmedium);

typedef HRESULT (*P_RegisterDragDrop)(HWND hwnd, IDropTarget *pDropTarget);

typedef HRESULT (*P_RevokeDragDrop)(HWND hwnd);

typedef void (*P_DragFinish)(HDROP hDrop);

typedef UINT (*P_DragQueryFile)(
    HDROP hDrop, UINT iFile, WCHAR *lpszFile, UINT cch);

typedef LPITEMIDLIST (*P_ILCombine)(
    LPCITEMIDLIST pidl1, LPCITEMIDLIST pid2);

typedef UINT (*P_ILGetSize)(LPCITEMIDLIST pidl);

typedef BOOL (*P_SHGetPathFromIDList)(
    LPCITEMIDLIST pidl, void *pszPath);

typedef HRESULT (*P_SHILCreateFromPath)(
    void *pszPath, LPITEMIDLIST *ppidl, ULONG *rgflnOut);

typedef void (*P_SHFree)(void *pv);


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static HRESULT Ole_OleSetClipboard(IDataObject *pDataObject);

static void Ole_ReleaseStgMedium(STGMEDIUM *pmedium);

static HDROP Ole_IsVirtualFile(IDataObject *pDataObject);

static BOOLEAN Ole_WriteStreamToFile(
    IStream *pStream, ULONG StreamSize, WCHAR *Path);

static HRESULT Ole_RegisterDragDrop(HWND hwnd, IDropTarget *pDropTarget);

static HRESULT Ole_RevokeDragDrop(HWND hwnd);

static ULONG Ole_DoDragDrop_2(void *ThreadParam);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static ULONG Ole_CF_FileNameA = 0;
static ULONG Ole_CF_FileNameW = 0;
static ULONG Ole_CF_ShellIdList = 0;


//---------------------------------------------------------------------------


static P_OleSetClipboard        __sys_OleSetClipboard           = NULL;
static P_ReleaseStgMedium       __sys_ReleaseStgMedium          = NULL;
static P_RegisterDragDrop       __sys_RegisterDragDrop          = NULL;
static P_RevokeDragDrop         __sys_RevokeDragDrop            = NULL;
static P_DragFinish             __sys_DragFinish                = NULL;

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


extern "C" _FX BOOLEAN Ole_Init(HMODULE module)
{
    void *OleSetClipboard;
    void *ReleaseStgMedium;
    void *RegisterDragDrop;
    void *RevokeDragDrop;

    // DisableComProxy BEGIN
    if (!SbieApi_QueryConfBool(NULL, L"DisableComProxy", FALSE))
    // DisableComProxy END
    if (! SbieDll_IsOpenCOM()) {

        Com_Init_Ole32(module);

        RegisterDragDrop = GetProcAddress(module, "RegisterDragDrop");
        RevokeDragDrop   = GetProcAddress(module, "RevokeDragDrop");

        if (!Gui_OpenAllWinClasses && Config_GetSettingsForImageName_bool(L"UseDragDropHack", TRUE)) {

            //
            // don't hook drag and drop if using OpenWinClass=*
            // because we have neither our Get/SetWindowLong hooks
            // which maintain Gui_DropTargetProp_Atom, nor the window
            // procedure hook to invoke Ole_DoDragDrop
            //

            SBIEDLL_HOOK(Ole_,RegisterDragDrop);
            SBIEDLL_HOOK(Ole_,RevokeDragDrop);
        }
    }

    ReleaseStgMedium = (P_ReleaseStgMedium)
        GetProcAddress(module, "ReleaseStgMedium");

    SBIEDLL_HOOK(Ole_, ReleaseStgMedium);

    if (Dll_ImageType == DLL_IMAGE_SHELL_EXPLORER) {

        OleSetClipboard = GetProcAddress(module, "OleSetClipboard");

        SBIEDLL_HOOK(Ole_,OleSetClipboard);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// XEnumFormatEtc Class
//---------------------------------------------------------------------------


class XEnumFormatEtc : public IEnumFORMATETC
{

public:

    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObject);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    STDMETHOD(Next)(ULONG celt, FORMATETC *rgelt, ULONG *pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(IEnumFORMATETC **ppenum);

    BOOL m_eof;

};


//---------------------------------------------------------------------------
// XEnumFormatEtc::IUnknown::QueryInterface
//---------------------------------------------------------------------------


_FX HRESULT XEnumFormatEtc::QueryInterface(REFIID iid, void **ppvObject)
{
    if (IsEqualIID(iid, IID_IUnknown) || IsEqualIID(iid, IID_IEnumFORMATETC))
        *ppvObject = this;
    else
        return E_NOINTERFACE;
    ((IUnknown *)(*ppvObject))->AddRef();
    return S_OK;
}


//---------------------------------------------------------------------------
// XEnumFormatEtc::IUnknown::AddRef
//---------------------------------------------------------------------------


_FX ULONG XEnumFormatEtc::AddRef()
{
    return 1;
}


//---------------------------------------------------------------------------
// XEnumFormatEtc::IUnknown::Release
//---------------------------------------------------------------------------


_FX ULONG XEnumFormatEtc::Release()
{
    return 1;
}


//---------------------------------------------------------------------------
// XEnumFormatEtc::IUnknown::Next
//---------------------------------------------------------------------------


_FX HRESULT XEnumFormatEtc::Next(
    ULONG celt, FORMATETC *rgelt, ULONG *pceltFetched)
{
    if (m_eof) {

        if (pceltFetched)
            *pceltFetched = 0;

    } else if (celt >= 1) {

        m_eof = TRUE;
        rgelt->cfFormat = CF_HDROP;
        rgelt->ptd = NULL;
        rgelt->dwAspect = DVASPECT_CONTENT;
        rgelt->lindex = -1;
        rgelt->tymed = TYMED_HGLOBAL;
        if (pceltFetched)
            *pceltFetched = 1;
        if (celt == 1)
            return S_OK;
    }

    return S_FALSE;
}


//---------------------------------------------------------------------------
// XEnumFormatEtc::IUnknown::Skip
//---------------------------------------------------------------------------


_FX HRESULT XEnumFormatEtc::Skip(ULONG celt)
{
    if (m_eof || celt != 1)
        return S_FALSE;
    return S_OK;
}


//---------------------------------------------------------------------------
// XEnumFormatEtc::IUnknown::Reset
//---------------------------------------------------------------------------


_FX HRESULT XEnumFormatEtc::Reset(void)
{
    m_eof = FALSE;
    return S_OK;
}


//---------------------------------------------------------------------------
// XEnumFormatEtc::IUnknown::Clone
//---------------------------------------------------------------------------


_FX HRESULT XEnumFormatEtc::Clone(IEnumFORMATETC **ppenum)
{
    *ppenum = this;
    return S_OK;
}


//---------------------------------------------------------------------------
// XDataObject Class
//---------------------------------------------------------------------------


class XDataObject : public IDataObject
{
public:

    XDataObject(IDataObject *pDataObject, HDROP hDrop);

protected:

    void PrintFormat(const WCHAR *FuncName, const FORMATETC *pFormatetc)
#ifndef WITH_PRINT_FORMAT
    {}
#endif
    ;

    BOOLEAN IsValidFormat(const FORMATETC *pFormatetc);

    HRESULT GetDataCommon(FORMATETC *format, STGMEDIUM *medium);

    HGLOBAL InitFormatHDrop(HGLOBAL hData);

    HANDLE OpenFileFromHDrop(HGLOBAL hData);

    HGLOBAL InitFormatFileNameA(HGLOBAL hData);

    HGLOBAL InitFormatFileNameW(HGLOBAL hData);

    HGLOBAL InitFormatIdList(HGLOBAL hData);


//---------------------------------------------------------------------------
// IUnknown Methods
//---------------------------------------------------------------------------


public:

    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObject);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();


//---------------------------------------------------------------------------
// IDataObject Methods
//---------------------------------------------------------------------------


protected:

    STDMETHOD(DAdvise)(
        FORMATETC *pFormatetc, DWORD advf, IAdviseSink *pAdvSink,
        DWORD *pdwConnection);
    STDMETHOD(DUnadvise)(DWORD dwConnection);
    STDMETHOD(EnumDAdvise)(IEnumSTATDATA **ppenumAdvise);
    STDMETHOD(EnumFormatEtc)(
        DWORD dwDirection, IEnumFORMATETC **ppenumFormatetc);
    STDMETHOD(GetCanonicalFormatEtc)(
        FORMATETC *pFormatetcIn, FORMATETC *pFormatetcOut);
    STDMETHOD(GetData)(FORMATETC *pFormatetc, STGMEDIUM *pmedium);
    STDMETHOD(GetDataHere)(FORMATETC *pFormatetc, STGMEDIUM *pmedium);
    STDMETHOD(QueryGetData)(FORMATETC *pFormatetc);
    STDMETHOD(SetData)(
        FORMATETC *pFormatetc, STGMEDIUM *pmedium, BOOL fRelease);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


protected:

    IDataObject *m_pDataObject;
    HDROP m_hDrop;
    ULONG m_fileCount;
    ULONG m_refCount;

    XEnumFormatEtc m_EnumFormatEtc;

    HMODULE __shell32;
    P_DragQueryFile pDragQueryFileW;
    P_ILCombine pILCombine;
    P_ILGetSize pILGetSize;
    P_SHGetPathFromIDList pSHGetPathFromIDList;
    P_SHILCreateFromPath pSHILCreateFromPath;
    P_SHFree pSHFree;

};


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


_FX XDataObject::XDataObject(IDataObject *pDataObject, HDROP hDrop)
{
    //
    //
    //

    __shell32 = LoadLibrary(L"shell32.dll");
    if (__shell32) {

        pDragQueryFileW = (P_DragQueryFile)
            GetProcAddress(__shell32, "DragQueryFileW");

        pILCombine = (P_ILCombine)GetProcAddress(__shell32, "ILCombine");

        pILGetSize = (P_ILGetSize)GetProcAddress(__shell32, "ILGetSize");

        pSHGetPathFromIDList = (P_SHGetPathFromIDList)
            GetProcAddress(__shell32, "SHGetPathFromIDListW");

        pSHILCreateFromPath = (P_SHILCreateFromPath)
            GetProcAddress(__shell32, "SHILCreateFromPath");

        pSHFree = (P_SHFree)GetProcAddress(__shell32, "SHFree");

        if (! Ole_CF_FileNameA) {
            Ole_CF_FileNameA =
                __sys_RegisterClipboardFormatW(CFSTR_FILENAMEA);
        }
        if (! Ole_CF_FileNameW) {
            Ole_CF_FileNameW =
                __sys_RegisterClipboardFormatW(CFSTR_FILENAMEW);
        }

        if (! Ole_CF_ShellIdList) {
            Ole_CF_ShellIdList =
                __sys_RegisterClipboardFormatW(CFSTR_SHELLIDLIST);
        }

    } else {

        pDragQueryFileW = NULL;
        pSHGetPathFromIDList = NULL;
        pSHILCreateFromPath = NULL;
    }

    //
    //
    //

    m_pDataObject = pDataObject;
    if (m_pDataObject)
        pDataObject->AddRef();

    m_hDrop = hDrop;
    if (m_hDrop && pDragQueryFileW) {
        m_fileCount = pDragQueryFileW(m_hDrop, 0xFFFFFFFF, NULL, 0);
        if (m_fileCount == -1)
            m_fileCount = 0;
    } else
        m_fileCount = 0;

    m_refCount = 1;
}


//---------------------------------------------------------------------------
// XDataObject::IUnknown::QueryInterface
//---------------------------------------------------------------------------


_FX HRESULT XDataObject::QueryInterface(REFIID iid, void **ppvObject)
{
    if (IsEqualIID(iid, IID_IUnknown) || IsEqualIID(iid, IID_IDataObject))
        *ppvObject = this;
    else
        return E_NOINTERFACE;
    ((IUnknown *)(*ppvObject))->AddRef();
    return S_OK;
}


//---------------------------------------------------------------------------
// XDataObject::IUnknown::AddRef
//---------------------------------------------------------------------------


_FX ULONG XDataObject::AddRef()
{
    return ++m_refCount;
}


//---------------------------------------------------------------------------
// XDataObject::IUnknown::Release
//---------------------------------------------------------------------------


_FX ULONG XDataObject::Release()
{
    --m_refCount;
    if (m_refCount != 0)
        return m_refCount;

    if (m_pDataObject)
        m_pDataObject->Release();

    if (__shell32)
        FreeLibrary(__shell32);

    Dll_Free(this);

    return 0;
}


//---------------------------------------------------------------------------
// IDataObject::DAdvise
//---------------------------------------------------------------------------


_FX HRESULT XDataObject::DAdvise(
    FORMATETC *pFormatetc, DWORD advf, IAdviseSink *pAdvSink,
    DWORD *pdwConnection)
{
    if (! m_pDataObject)
        return OLE_E_ADVISENOTSUPPORTED;
    return m_pDataObject->DAdvise(pFormatetc, advf, pAdvSink, pdwConnection);
}


//---------------------------------------------------------------------------
// IDataObject::DUnadvise
//---------------------------------------------------------------------------


_FX HRESULT XDataObject::DUnadvise(DWORD dwConnection)
{
    if (! m_pDataObject)
        return OLE_E_ADVISENOTSUPPORTED;
    return m_pDataObject->DUnadvise(dwConnection);
}


//---------------------------------------------------------------------------
// IDataObject::EnumDAdvise
//---------------------------------------------------------------------------


_FX HRESULT XDataObject::EnumDAdvise(IEnumSTATDATA **ppenumAdvise)
{
    if (! m_pDataObject)
        return OLE_E_ADVISENOTSUPPORTED;
    return m_pDataObject->EnumDAdvise(ppenumAdvise);
}


//---------------------------------------------------------------------------
// IDataObject::EnumFormatEtc
//---------------------------------------------------------------------------


_FX HRESULT XDataObject::EnumFormatEtc(
    DWORD dwDirection, IEnumFORMATETC **ppenumFormatetc)
{
    if (! m_pDataObject) {
        m_EnumFormatEtc.Reset();
        *ppenumFormatetc = &m_EnumFormatEtc;
        return S_OK;
    }

    return m_pDataObject->EnumFormatEtc(dwDirection, ppenumFormatetc);
}


//---------------------------------------------------------------------------
// IDataObject::GetCanonicalFormatEtc
//---------------------------------------------------------------------------


_FX HRESULT XDataObject::GetCanonicalFormatEtc(
    FORMATETC *pFormatetcIn, FORMATETC *pFormatetcOut)
{
    if (! m_pDataObject) {
        memzero(pFormatetcOut, sizeof(FORMATETC));
        return DATA_S_SAMEFORMATETC;
    }
    return m_pDataObject->GetCanonicalFormatEtc(pFormatetcIn, pFormatetcOut);
}


//---------------------------------------------------------------------------
// IDataObject::GetData
//---------------------------------------------------------------------------


_FX HRESULT XDataObject::GetData(
    FORMATETC *pFormatetc, STGMEDIUM *pmedium)
{
    HRESULT hr;

    PrintFormat(L"GetData", pFormatetc);

    if (m_pDataObject)
        hr = m_pDataObject->GetData(pFormatetc, pmedium);

    else if (! IsValidFormat(pFormatetc))
        hr = DV_E_FORMATETC;
    else if ((pFormatetc->tymed & TYMED_HGLOBAL) == 0)
        hr = DV_E_TYMED;
    else if (pFormatetc->lindex != -1)
        hr = DV_E_LINDEX;
    else if (pFormatetc->dwAspect != DVASPECT_CONTENT)
        hr = DV_E_DVASPECT;
    else {

        SIZE_T hDropLen = GlobalSize(m_hDrop);
        HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, hDropLen);
        if (! hGlobal)
            hr = E_OUTOFMEMORY;
        else {

            void *ptrSrc = GlobalLock(m_hDrop);
            void *ptrDst = GlobalLock(hGlobal);
            memcpy(ptrDst, ptrSrc, hDropLen);
            GlobalUnlock(m_hDrop);
            GlobalUnlock(hGlobal);

            pmedium->tymed = TYMED_HGLOBAL;
            pmedium->hGlobal = hGlobal;
            pmedium->pUnkForRelease = NULL;
            hr = S_OK;
        }
    }

    if (SUCCEEDED(hr))
        hr = GetDataCommon(pFormatetc, pmedium);
    return hr;
}


//---------------------------------------------------------------------------
// IDataObject::GetDataHere
//---------------------------------------------------------------------------


_FX HRESULT XDataObject::GetDataHere(
    FORMATETC *pFormatetc, STGMEDIUM *pmedium)
{
    if (! m_pDataObject)
        return E_FAIL;
    HRESULT hr = m_pDataObject->GetDataHere(pFormatetc, pmedium);
    if (SUCCEEDED(hr))
        hr = GetDataCommon(pFormatetc, pmedium);
    return hr;
}


//---------------------------------------------------------------------------
// IDataObject::QueryGetData
//---------------------------------------------------------------------------


_FX HRESULT XDataObject::QueryGetData(FORMATETC *pFormatetc)
{
    HRESULT hr;

    PrintFormat(L"QueryGetData", pFormatetc);

    if (m_pDataObject)
        return m_pDataObject->QueryGetData(pFormatetc);

    else if (! IsValidFormat(pFormatetc))
        hr = DV_E_FORMATETC;
    else if ((pFormatetc->tymed & TYMED_HGLOBAL) == 0)
        hr = DV_E_TYMED;
    else if (pFormatetc->lindex != -1)
        hr = DV_E_LINDEX;
    else if (pFormatetc->dwAspect != DVASPECT_CONTENT)
        hr = DV_E_DVASPECT;
    else
        hr = S_OK;
    return hr;
}


//---------------------------------------------------------------------------
// IDataObject::SetData
//---------------------------------------------------------------------------


_FX HRESULT XDataObject::SetData(
    FORMATETC *pFormatetc, STGMEDIUM *pmedium, BOOL fRelease)
{
    if (! m_pDataObject)
        return E_NOTIMPL;
    return m_pDataObject->SetData(pFormatetc, pmedium, fRelease);
}


//---------------------------------------------------------------------------
// PrintFormat
//---------------------------------------------------------------------------


#ifdef WITH_PRINT_FORMAT

#include <stdio.h>

_FX void XDataObject::PrintFormat(
    const WCHAR *FuncName, const FORMATETC *pFormatetc)
{
    const ULONG fmt = (ULONG)pFormatetc->cfFormat;
    WCHAR text[128];
    Sbie_snwprintf(text, 128, L"%-32.32s - <", FuncName);
    if (fmt >= 0xC000 && fmt <= 0xFFFF)
        __sys_GetClipboardFormatNameW(fmt, text + 36, 60);
    else
        Sbie_snwprintf(text + 36, 128 - 36, L"%08X", fmt);
    wcscat(text, L">\n");
    OutputDebugString(text);
}


#endif WITH_PRINT_FORMAT


//---------------------------------------------------------------------------
// IsValidFormat
//---------------------------------------------------------------------------


_FX BOOLEAN XDataObject::IsValidFormat(const FORMATETC *pFormatetc)
{
    const UINT fmt = pFormatetc->cfFormat;
    if (m_fileCount) {

        BOOLEAN IsFileNameA = (Ole_CF_FileNameA && fmt == Ole_CF_FileNameA);
        BOOLEAN IsFileNameW = (Ole_CF_FileNameW && fmt == Ole_CF_FileNameW);

        if (IsFileNameA || IsFileNameW) {

            if (m_fileCount == 1)
                return TRUE;
            else
                return FALSE;
        }

        if (fmt == CF_HDROP)
            return TRUE;
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// GetDataCommon
//---------------------------------------------------------------------------


_FX HRESULT XDataObject::GetDataCommon(FORMATETC *format, STGMEDIUM *medium)
{
    HRESULT hr = S_OK;

    if (medium->tymed != TYMED_HGLOBAL)
        return hr;

    HGLOBAL hGlobal = NULL;
    const UINT fmt = format->cfFormat;

    PrintFormat(L"GetDataCommon", format);

    if (fmt == CF_HDROP) {

        hGlobal = InitFormatHDrop(medium->hGlobal);

    } else {

        if (Ole_CF_ShellIdList && fmt == Ole_CF_ShellIdList) {
            hGlobal = InitFormatIdList(medium->hGlobal);
            if (hGlobal == (HGLOBAL)-1) {
                hGlobal = NULL;
                hr = DV_E_FORMATETC;
            }
        }

        else if (Ole_CF_FileNameA && fmt == Ole_CF_FileNameA)
            hGlobal = InitFormatFileNameA(medium->hGlobal);

        else if (Ole_CF_FileNameW && fmt == Ole_CF_FileNameW)
            hGlobal = InitFormatFileNameW(medium->hGlobal);
    }

    if (hGlobal || hr != S_OK) {

        if (medium->pUnkForRelease) {
            medium->pUnkForRelease->Release();
            medium->pUnkForRelease = NULL;
        } else
            GlobalFree(medium->hGlobal);
        medium->hGlobal = hGlobal;
    }

    return hr;
}


//---------------------------------------------------------------------------
// InitFormatHDrop
//---------------------------------------------------------------------------


_FX HGLOBAL XDataObject::InitFormatHDrop(HGLOBAL hData)
{
    if (! pDragQueryFileW)
        return NULL;

    //
    // translate each path in the DROPFILES structure to a full
    // path into the sandbox, if applicable.  this is in case the
    // paste operation is going to be done by a program running
    // outside the sandbox
    //

    HDROP hDrop = (HDROP)hData;
    bool sandboxed = false;

    UINT count = pDragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);
    if (count == -1)
        count = 0;

    ULONG DropFiles_len = (count + 8) * MAX_PATH * 2 * sizeof(WCHAR);
    DROPFILES *DropFiles = (DROPFILES *)Dll_Alloc(DropFiles_len);
    memzero(DropFiles, DropFiles_len);
    WCHAR *DropName = (WCHAR *)(DropFiles + 1);
    DropFiles->pFiles = (DWORD)((UCHAR *)DropName - (UCHAR *)DropFiles);
    DropFiles->fWide = TRUE;

    for (UINT i = 0; i < count; ++i) {

        pDragQueryFileW(hDrop, i, DropName, 510);

        HANDLE hFile = CreateFileW(DropName,
            GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {

            BOOLEAN is_copy;
            LONG status = SbieDll_GetHandlePath(hFile, DropName, &is_copy);
            if (status == 0 && is_copy) {
                SbieDll_TranslateNtToDosPath(DropName);
                sandboxed = true;
            } else {
                wmemzero(DropName, 512);
                pDragQueryFileW(hDrop, i, DropName, 510);
            }

            CloseHandle(hFile);
        }

        DropName += wcslen(DropName) + 1;
    }

    *DropName = L'\0';
    ++DropName;

    //
    // if we found any sandboxed files to translate to full paths, then
    // create our own HGLOBAL data object
    //

    if (! sandboxed) {

        hData = NULL;

    } else {

        ULONG_PTR len = (UCHAR *)DropName - (UCHAR *)DropFiles;
        hData = GlobalAlloc(GMEM_MOVEABLE, len);
        if (hData) {
            void *ptr = GlobalLock(hData);
            memcpy(ptr, DropFiles, len);
            GlobalUnlock(hData);
        }
    }

    Dll_Free(DropFiles);
    return hData;
}


//---------------------------------------------------------------------------
// OpenFileFromHDrop
//---------------------------------------------------------------------------


_FX HANDLE XDataObject::OpenFileFromHDrop(HGLOBAL hData)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;

    WCHAR *FileName = (WCHAR *)Dll_Alloc(1024 * sizeof(WCHAR));
    UINT count = pDragQueryFileW((HDROP)hData, 0, FileName, 1000);
    if (count > 0 && count < 1000) {

        FileName[count] = L'\0';
        hFile = CreateFileW(FileName,
            GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS, NULL);
    }

    Dll_Free(FileName);
    return hFile;
}


//---------------------------------------------------------------------------
// InitFormatFileNameA
//---------------------------------------------------------------------------


_FX HGLOBAL XDataObject::InitFormatFileNameA(HGLOBAL hData)
{
    HGLOBAL hDataRet = NULL;
    HANDLE hFile;

    if (m_pDataObject) {

        //
        // hData is CF_FILENAMEA
        //

        char *FileNameA = (char *)GlobalLock(hData);

        hFile = CreateFileA(FileNameA,
            GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS, NULL);

        GlobalUnlock(hData);

    } else {

        //
        // hData is CF_HDROP
        //

        hFile = OpenFileFromHDrop(hData);
    }

    if (hFile != INVALID_HANDLE_VALUE) {

        BOOLEAN is_copy;
        WCHAR *name = (WCHAR *)Dll_Alloc(8192);
        LONG status = SbieDll_GetHandlePath(hFile, name, &is_copy);

        if (! m_pDataObject) {
            // hData was CF_HDROP so we don't have a default CF_FILENAMEA
            // to return.  we need to always create it regardless of is_copy
            is_copy = TRUE;
        }

        if (status == 0 && is_copy) {

            SbieDll_TranslateNtToDosPath(name);

            UNICODE_STRING uni;
            ANSI_STRING ansi;

            RtlInitUnicodeString(&uni, name);
            ansi.Length = 0;
            ansi.MaximumLength = uni.Length / sizeof(WCHAR) + 1;
            hDataRet = GlobalAlloc(GMEM_MOVEABLE, ansi.MaximumLength);
            if (hDataRet) {

                ansi.Buffer = (UCHAR *)GlobalLock(hDataRet);
                RtlUnicodeStringToAnsiString(&ansi, &uni, FALSE);
                GlobalUnlock(hDataRet);
            }
        }

        Dll_Free(name);
        CloseHandle(hFile);
    }

    return hDataRet;
}


//---------------------------------------------------------------------------
// InitFormatFileNameW
//---------------------------------------------------------------------------


_FX HGLOBAL XDataObject::InitFormatFileNameW(HGLOBAL hData)
{
    HGLOBAL hDataRet = NULL;
    HANDLE hFile;

    if (m_pDataObject) {

        //
        // hData is CF_FILENAMEW
        //

        WCHAR *FileNameW = (WCHAR *)GlobalLock(hData);

        hFile = CreateFileW(FileNameW,
            GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS, NULL);

        GlobalUnlock(hData);

    } else {

        //
        // hData is CF_HDROP
        //

        hFile = OpenFileFromHDrop(hData);
    }

    if (hFile != INVALID_HANDLE_VALUE) {

        BOOLEAN is_copy;
        WCHAR *name = (WCHAR *)Dll_Alloc(8192);
        LONG status = SbieDll_GetHandlePath(hFile, name, &is_copy);

        if (! m_pDataObject) {
            // hData was CF_HDROP so we don't have a default CF_FILENAMEW
            // to return.  we need to always create it regardless of is_copy
            is_copy = TRUE;
        }

        if (status == 0 && is_copy) {

            SbieDll_TranslateNtToDosPath(name);

            ULONG len = (wcslen(name) + 1) * sizeof(WCHAR);
            hDataRet = GlobalAlloc(GMEM_MOVEABLE, len);
            if (hDataRet) {

                WCHAR *ptr = (WCHAR *)GlobalLock(hDataRet);
                memcpy(ptr, name, len);
                GlobalUnlock(hDataRet);
            }
        }

        Dll_Free(name);
        CloseHandle(hFile);
    }

    return hDataRet;
}


//---------------------------------------------------------------------------
// InitFormatIdList
//---------------------------------------------------------------------------


_FX HGLOBAL XDataObject::InitFormatIdList(HGLOBAL hData)
{
#define GetPidl(i) (LPCITEMIDLIST)(((LPBYTE)pIdList)+(pIdList)->aoffset[i])

    if (! m_pDataObject) {
        // if we don't have an underlying data object,
        // then we shouldn't return an ID List
        return (HGLOBAL)-1;
    }

    if (        (! pILCombine)  ||  (!  pILGetSize)  ||  (! pSHFree)
            ||  (! pSHILCreateFromPath)  ||  (! pSHGetPathFromIDList))
        return NULL;

    CIDA *pIdList = (CIDA *)GlobalLock(hData);
    if (! pIdList)
        return NULL;

    BOOL ok;
    HANDLE hFile;
    WCHAR *path = (WCHAR *)Dll_AllocTemp(8192);

    //
    // check how many of the files in the Shell ID List Array
    // have copies in the sandbox.  we will prepare a new array
    // only if all files have a copy in the sandbox
    //

    UINT num_in_box = 0;
    UINT count;
    for (count = 1; count <= pIdList->cidl; ++count) {

        LPCITEMIDLIST pidl = pILCombine(GetPidl(0), GetPidl(count));
        if (pidl) {

            ok = pSHGetPathFromIDList(pidl, path);
            if (ok) {

                hFile = CreateFileW(path,
                    GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                    FILE_FLAG_BACKUP_SEMANTICS, NULL);

                if (hFile != INVALID_HANDLE_VALUE) {

                    BOOLEAN is_copy;
                    LONG rc = SbieDll_GetHandlePath(hFile, path, &is_copy);
                    if (rc == 0 && is_copy)
                        ++num_in_box;

                    CloseHandle(hFile);
                }
            }

            pSHFree((void *)pidl);
        }
    }

    //
    // if none of the files are in the sandbox, if all of the files are
    // in the sandbox then we create a new Shell ID List Array
    //
    // if some files are in the sandbox and others outside the sandbox
    // then we can't return a Shell ID List Array and hopefully the
    // caller can fallback on HDROP
    //

    HGLOBAL hDataRet = NULL;

    if (num_in_box == 0)
        goto finish;

    if (num_in_box != pIdList->cidl) {
        hDataRet = (HGLOBAL)-1;
        goto finish;
    }

    //
    // get the pidl for the parent folder in the sandbox
    //

    ok = pSHGetPathFromIDList(GetPidl(0), path);
    if (!ok)
        goto finish;

    hFile = CreateFileW(path,
        GENERIC_WRITE, FILE_SHARE_VALID_FLAGS, NULL,
        OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        goto finish;

    BOOLEAN is_copy;
    LONG rc = SbieDll_GetHandlePath(hFile, path, &is_copy);

    CloseHandle(hFile);

    if (rc == STATUS_BAD_INITIAL_PC) {

        //
        // if the parent folder is a root folder, we have to open
        // the file and get rid of the last path component
        //

        LPCITEMIDLIST pidl = pILCombine(GetPidl(0), GetPidl(1));
        if (pidl) {

            ok = pSHGetPathFromIDList(pidl, path);
            if (ok) {

                hFile = CreateFileW(path,
                    GENERIC_WRITE, FILE_SHARE_VALID_FLAGS, NULL,
                    OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

                LONG rc2 = SbieDll_GetHandlePath(hFile, path, &is_copy);

                CloseHandle(hFile);

                if (rc2 == 0 && is_copy) {

                    WCHAR *ptr = wcsrchr(path, L'\\');
                    if (ptr) {

                        *ptr = L'\0';
                        rc = 0;
                    }
                }
            }

            pSHFree((void *)pidl);
        }
    }

    if (rc != 0 || (! is_copy))
        goto finish;

    //
    // create a new pidl for the parent folder path in the sandbox
    //

    SbieDll_TranslateNtToDosPath(path);

	HRESULT hr;
    LPITEMIDLIST pidl;
    ULONG flags = 0;
    hr = pSHILCreateFromPath(path, &pidl, &flags);
    if (FAILED(hr))
        goto finish;

    //
    // create a new array where the pidl for the parent folder
    // references the corresponding folder in the sandbox
    //

    ULONG new_idlist_len = (pIdList->cidl + 2) * sizeof(UINT)
                         + pILGetSize(pidl);
    for (count = 1; count <= pIdList->cidl; ++count)
        new_idlist_len += pILGetSize(GetPidl(count));

    hDataRet = GlobalAlloc(GMEM_MOVEABLE, new_idlist_len);
    if (hDataRet) {

        UCHAR *ptr0 = (UCHAR *)GlobalLock(hDataRet);
        ((CIDA *)ptr0)->cidl = pIdList->cidl;
        UINT *offsets = ((CIDA *)ptr0)->aoffset;

        UCHAR *ptr = (UCHAR *)&offsets[pIdList->cidl + 1];
        ULONG pidl_len = pILGetSize(pidl);
        memcpy(ptr, pidl, pidl_len);
        offsets[0] = (USHORT)(ULONG_PTR)(ptr - ptr0);
        ptr += pidl_len;

        for (count = 1; count <= pIdList->cidl; ++count) {
            pidl_len = pILGetSize(GetPidl(count));
            memcpy(ptr, pidl, pidl_len);
            offsets[count] = (USHORT)(ULONG_PTR)(ptr - ptr0);
            ptr += pidl_len;
        }

        GlobalUnlock(hDataRet);
    }

    pSHFree(pidl);

finish:

    Dll_Free(path);
    GlobalUnlock(hData);
    return hDataRet;

#undef GetPidl

}


//---------------------------------------------------------------------------
// Ole_ReleaseStgMedium
//---------------------------------------------------------------------------


_FX void Ole_ReleaseStgMedium(STGMEDIUM *pmedium)
{
    if (pmedium != NULL)
    {
        if (((pmedium->tymed == TYMED_ENHMF && pmedium->hEnhMetaFile != 0 && pmedium->pUnkForRelease == 0) ||
            (__sys_GetEnhMetaFileBits != 0 && 0 != __sys_GetEnhMetaFileBits(pmedium->hEnhMetaFile, 0, NULL))) &&
            __sys_DeleteEnhMetaFile != 0)
        {
            __sys_DeleteEnhMetaFile(pmedium->hEnhMetaFile);
            pmedium->tymed = TYMED_NULL;
            return;
        }
        if (((pmedium->tymed == TYMED_GDI && pmedium->hBitmap != 0 && pmedium->pUnkForRelease == 0) ||
            (__sys_GetBitmapBits != 0 && 0 != __sys_GetBitmapBits(pmedium->hBitmap, 0, NULL))) &&
            __sys_DeleteObject != 0)
        {
            __sys_DeleteObject(pmedium->hBitmap);
            pmedium->tymed = TYMED_NULL;
            return;
        }
    }
    ((P_ReleaseStgMedium)__sys_ReleaseStgMedium)(pmedium);
}


//---------------------------------------------------------------------------
// Ole_OleSetClipboard
//---------------------------------------------------------------------------


_FX HRESULT Ole_OleSetClipboard(IDataObject *pDataObject)
{
    if (pDataObject) {

        ULONG len = sizeof(XDataObject);
        void *buf = Dll_Alloc(len);

        XDataObject *xDataObject;

        HDROP hDrop = Ole_IsVirtualFile(pDataObject);
        if (hDrop)
            pDataObject = NULL;

        xDataObject = new (buf) XDataObject(pDataObject, hDrop);
        pDataObject = xDataObject;
    }

    return ((P_OleSetClipboard)__sys_OleSetClipboard)(pDataObject);
}


//---------------------------------------------------------------------------
// Ole_IsVirtualFile
//---------------------------------------------------------------------------

// this code doesn't seem to be necessary and causes tmp files to be created in the sandbox

_FX HDROP Ole_IsVirtualFile(IDataObject *pDataObject)
{
    return NULL;
}

    /*
    //
    // check for the clipboard formats that zip files publish:
    // FileGroupDescriptorW and FileContents
    //

    if (! __sys_ReleaseStgMedium)
        return NULL;

    IEnumFORMATETC *pEnum;
    HRESULT hr = pDataObject->EnumFormatEtc(DATADIR_GET, &pEnum);
    if (FAILED(hr))
        return NULL;

    CLIPFORMAT cfFileDescriptor = 0;
    CLIPFORMAT cfFileContents = 0;
    FORMATETC format;

    pEnum->Reset();
    while (1) {
        ULONG n;
        hr = pEnum->Next(1, &format, &n);
        if (FAILED(hr) || n == 0)
            break;

        WCHAR name[64];
        CLIPFORMAT fmt = format.cfFormat;
        name[0] = L'\0';
        if (fmt >= 0xC000 && fmt <= 0xFFFF) {

            __sys_GetClipboardFormatNameW(fmt, name, 60);

            if (_wcsicmp(name, CFSTR_FILEDESCRIPTORW) == 0)
                cfFileDescriptor = fmt;
            else if (_wcsicmp(name, CFSTR_FILECONTENTS) == 0)
                cfFileContents = fmt;
        }
    }

    pEnum->Release();
    if ((! cfFileDescriptor) || (! cfFileContents))
        return NULL;

    //
    // get temporary output folder:
    // C:\TEMP\SbieTemp\HHHHHHHHLLLLLLLL
    //

    WCHAR *TempPath = (WCHAR *)Dll_Alloc((MAX_PATH + 128) * sizeof(WCHAR));
    wmemzero(TempPath, MAX_PATH + 128);

    GetTempPath(MAX_PATH, TempPath);
    wcscat(TempPath, SBIE L"_Temp\\");
    CreateDirectory(TempPath, NULL);
    WCHAR *TempFile = TempPath + wcslen(TempPath);
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    _itow(ft.dwHighDateTime, TempFile, 16);
    TempFile += wcslen(TempFile);
    _itow(ft.dwLowDateTime, TempFile, 16);
    TempFile += wcslen(TempFile);
    CreateDirectory(TempPath, NULL);
    *TempFile = L'\\';
    ++TempFile;

    //
    // prepare the HDROP / DROPFILES output buffer
    //

    format.cfFormat = cfFileDescriptor;
    format.ptd = NULL;
    format.dwAspect = DVASPECT_CONTENT;
    format.lindex = -1;
    format.tymed = TYMED_HGLOBAL;

    STGMEDIUM mediumFileDescriptor;
    memzero(&mediumFileDescriptor, sizeof(mediumFileDescriptor));
    hr = pDataObject->GetData(&format, &mediumFileDescriptor);

    if (FAILED(hr) || mediumFileDescriptor.tymed != TYMED_HGLOBAL) {
        Dll_Free(TempPath);
        return NULL;
    }

    FILEGROUPDESCRIPTOR *Group =
        (FILEGROUPDESCRIPTOR *)GlobalLock(mediumFileDescriptor.hGlobal);

    ULONG DropFiles_len =
        ((ULONG)Group->cItems) * MAX_PATH * 2 * sizeof(WCHAR);
    DROPFILES *DropFiles = (DROPFILES *)Dll_Alloc(DropFiles_len);
    memzero(DropFiles, DropFiles_len);
    WCHAR *DropName = (WCHAR *)(DropFiles + 1);
    DropFiles->pFiles = (DWORD)((UCHAR *)DropName - (UCHAR *)DropFiles);
    DropFiles->fWide = TRUE;

    //
    // extract each file into the temporary folder
    //

    const WCHAR *ParentDirPtr = NULL;
    ULONG ParentDirLen = 0;

    BOOLEAN AtLeastOne = FALSE;

    for (UINT i = 0; i < Group->cItems; ++i) {

        const WCHAR *cFileName = Group->fgd[i].cFileName;

        //
        // if the item is a directory, we're going to add it as a directory
        // to DROPFILES, and make sure its children are extracted from
        // the virtual container, but are not added to the DROPFILES
        //

        if ((Group->fgd[i].dwFlags & FD_ATTRIBUTES) &&
             (Group->fgd[i].dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

            wcscpy(TempFile, cFileName);
            CreateDirectory(TempPath, NULL);

            //
            // don't add the file explicitly to DROPFILES if its
            // parent directory has already been added
            //

            BOOLEAN add = TRUE;

            if (ParentDirLen &&
                    cFileName[ParentDirLen] == L'\\' &&
                    wcsncmp(cFileName, ParentDirPtr, ParentDirLen) == 0)
                add = FALSE;

            if (add) {

                wcscpy(DropName, TempPath);
                DropName += wcslen(DropName) + 1;

                AtLeastOne = TRUE;

                ParentDirPtr = cFileName;
                ParentDirLen = wcslen(ParentDirPtr);
            }

            continue;
        }

        //
        // otherwise the item is a file, extract only if not too large
        //

        if ((Group->fgd[i].dwFlags & FD_FILESIZE) == 0)
            continue;
        if (Group->fgd[i].nFileSizeHigh)
            continue;
        ULONG FileSize = Group->fgd[i].nFileSizeLow;

        format.cfFormat = cfFileContents;
        format.ptd = NULL;
        format.dwAspect = DVASPECT_CONTENT;
        format.lindex = i;
        format.tymed = TYMED_ISTREAM;

        STGMEDIUM medium;
        memzero(&medium, sizeof(medium));

        hr = pDataObject->GetData(&format, &medium);
        if (FAILED(hr))
            continue;

        //
        //
        //

        BOOLEAN reset = TRUE;

        if (medium.tymed == TYMED_ISTREAM) {

            wcscpy(TempFile, cFileName);
            if (Ole_WriteStreamToFile(medium.pstm, FileSize, TempPath)) {

                //
                // don't add the file explicitly to DROPFILES if its
                // parent directory has already been added
                //

                if (ParentDirLen &&
                        cFileName[ParentDirLen] == L'\\' &&
                        wcsncmp(cFileName, ParentDirPtr, ParentDirLen) == 0) {

                    reset = FALSE;

                } else {

                    wcscpy(DropName, TempPath);
                    DropName += wcslen(DropName) + 1;

                    AtLeastOne = TRUE;
                }
            }
        }

        if (reset) {
            ParentDirPtr = NULL;
            ParentDirLen = 0;
        }

        __sys_ReleaseStgMedium(&medium);
    }

    //
    // create HGLOBAL / HDROP for the DROPFILES buffer
    //

    HGLOBAL hData = NULL;

    if (AtLeastOne) {

        *DropName = L'\0';
        ++DropName;

        ULONG_PTR len = (UCHAR *)DropName - (UCHAR *)DropFiles;
        hData = GlobalAlloc(GMEM_MOVEABLE, len);
        if (hData) {
            void *ptr = GlobalLock(hData);
            memcpy(ptr, DropFiles, len);
            GlobalUnlock(hData);
        }
    }

    GlobalUnlock(mediumFileDescriptor.hGlobal);
    __sys_ReleaseStgMedium(&mediumFileDescriptor);

    Dll_Free(DropFiles);
    Dll_Free(TempPath);
    return (HDROP)hData;
}


//---------------------------------------------------------------------------
// Ole_WriteStreamToFile
//---------------------------------------------------------------------------


_FX BOOLEAN Ole_WriteStreamToFile(
    IStream *pStream, ULONG StreamSize, WCHAR *Path)
{
    HANDLE hFile = CreateFileW(
        Path, GENERIC_WRITE, FILE_SHARE_VALID_FLAGS, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    BOOLEAN ok = TRUE;
    UCHAR *Buffer = (UCHAR *)Dll_Alloc(16384);
    while (1) {

        ULONG BytesRead = 0;
        pStream->Read(Buffer, 16384, &BytesRead);
        if (! BytesRead)
            break;

        ULONG BytesWritten = 0;
        WriteFile(hFile, Buffer, BytesRead, &BytesWritten, NULL);
        if (BytesWritten != BytesRead) {
            ok = FALSE;
            break;
        }
    }

    Dll_Free(Buffer);
    CloseHandle(hFile);
    return ok;
}

*/

//---------------------------------------------------------------------------
// Ole_RegisterDragDrop
//---------------------------------------------------------------------------


_FX HRESULT Ole_RegisterDragDrop(HWND hwnd, IDropTarget *pDropTarget)
{
    HRESULT hr = __sys_RegisterDragDrop(hwnd, pDropTarget);

    if (SUCCEEDED(hr) && Gui_DropTargetProp_Atom) {

        //
        // our hook on SetProp renamed the DropTarget window properties
        // set by the real RegisterDragDrop, and we enable the
        // WS_EX_ACCEPTFILES style flag so we get the WM_DROPFILES message
        // and turn it into an OLE drop operation in Ole_DoDragDrop
        //

        ULONG exstyle = __sys_GetWindowLongW(hwnd, GWL_EXSTYLE);
        if ((exstyle & WS_EX_ACCEPTFILES) == 0) {

            exstyle |= WS_EX_ACCEPTFILES;
            __sys_SetWindowLongW(hwnd, GWL_EXSTYLE, exstyle);

            __sys_SetPropW(hwnd, (LPCWSTR)Gui_DropTargetProp_Atom, pDropTarget);

            if (! __sys_DragFinish) {

                HMODULE shell32 = LoadLibrary(L"shell32.dll");
                __sys_DragFinish =
                    (P_DragFinish)GetProcAddress(shell32, "DragFinish");
            }

            Gui_SetWindowProc(hwnd, TRUE);
        }
    }

    return hr;
}


//---------------------------------------------------------------------------
// Ole_RevokeDragDrop
//---------------------------------------------------------------------------


_FX HRESULT Ole_RevokeDragDrop(HWND hwnd)
{
    HRESULT hr = __sys_RevokeDragDrop(hwnd);

    if (SUCCEEDED(hr) && Gui_DropTargetProp_Atom) {

        //
        // if we enabled the WS_EX_ACCEPTFILES style in Ole_RegisterDragDrop
        // then we should now clear that style bit
        //

        void *pDropTarget =
                __sys_RemovePropW(hwnd, (LPCWSTR)Gui_DropTargetProp_Atom);
        if (pDropTarget) {

            ULONG exstyle = __sys_GetWindowLongW(hwnd, GWL_EXSTYLE);
            if ((exstyle & WS_EX_ACCEPTFILES) == WS_EX_ACCEPTFILES) {

                exstyle &= ~WS_EX_ACCEPTFILES;
                __sys_SetWindowLongW(hwnd, GWL_EXSTYLE, exstyle);
            }
        }
    }

    return hr;
}


//---------------------------------------------------------------------------
// Ole_DoDragDrop
//---------------------------------------------------------------------------


extern "C" _FX BOOLEAN Ole_DoDragDrop(
    HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    if (Gui_OpenAllWinClasses)
        return FALSE;

    if (wParam == tzuk) {

        Ole_DoDragDrop_2((void *)lParam);
        return TRUE;
    }

    //
    //
    //

    IDropTarget *pDropTarget = NULL;

    if (wParam) {

        pDropTarget = (IDropTarget *)__sys_GetPropW(
                            hWnd, (LPCWSTR)Gui_DropTargetProp_Atom);
    }

    if (! pDropTarget)
        return FALSE;

    ULONG len = sizeof(XDataObject);
    void *buf = Dll_Alloc(len);
    XDataObject *xDataObject =
                        new (buf) XDataObject(NULL, (HDROP)wParam);

    POINT ptx;
    __sys_GetCursorPos(&ptx);
    POINTL pt;
    pt.x = ptx.x;
    pt.y = ptx.y;

    ULONG effect = DROPEFFECT_COPY;
    BOOLEAN disregardEffect = FALSE;

    HRESULT hr = pDropTarget->DragEnter(xDataObject, 0, pt, &effect);
    if (SUCCEEDED(hr)) {

        if (Dll_ImageType == DLL_IMAGE_INTERNET_EXPLORER ||
            Dll_ImageType == DLL_IMAGE_MOZILLA_FIREFOX   ||
            Dll_ImageType == DLL_IMAGE_GOOGLE_CHROME) {

            ULONG_PTR *Args =
                (ULONG_PTR *)Dll_Alloc(sizeof(ULONG_PTR) * 8);
            Args[0] = (ULONG_PTR)Args;
            Args[1] = (ULONG_PTR)hWnd;
            Args[2] = (ULONG_PTR)wParam;
            Args[3] = (ULONG_PTR)pDropTarget;
            Args[4] = (ULONG_PTR)xDataObject;
            Args[5] = pt.x;
            Args[6] = pt.y;
            Args[7] = 1;

            if (QueueUserWorkItem(
                    Ole_DoDragDrop_2, Args, WT_EXECUTELONGFUNCTION))
                return TRUE;

            Dll_Free(Args);
        }

        hr = pDropTarget->DragOver(0, pt, &effect);
        if (effect == 0 && Dll_ImageType == DLL_IMAGE_WINDOWS_MEDIA_PLAYER)
            disregardEffect = TRUE;
    }

    if (SUCCEEDED(hr) && (effect || disregardEffect))
        pDropTarget->Drop(xDataObject, 0, pt, &effect);

    xDataObject->Release();

    __sys_DragFinish((HDROP)wParam);
    return TRUE;
}


//---------------------------------------------------------------------------
// Ole_DoDragDrop_2
//---------------------------------------------------------------------------


_FX ULONG Ole_DoDragDrop_2(void *ThreadArgs)
{
    ULONG_PTR *Args = (ULONG_PTR *)ThreadArgs;
    __try {
        if (*Args != (ULONG_PTR)Args)
            Args = 0;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        Args = 0;
    }
    if (! Args)
        return 0;

    HWND hWnd = (HWND)Args[1];
    HDROP hDrop = (HDROP)Args[2];
    IDropTarget *pDropTarget = (IDropTarget *)Args[3];
    XDataObject *xDataObject = (XDataObject *)Args[4];
    POINTL pt;
    pt.x = (LONG)Args[5];
    pt.y = (LONG)Args[6];
    ULONG *step = (ULONG *)&Args[7];

    //
    // step 1:  invoked by Ole_DoDragDrop after getting the first
    // WM_DROPFILES message.  record ThreadArgs in a window property,
    // sleep for a short while, and post the second WM_DROPFILES
    //

    if (*step == 1) {

        Sleep(250);
        ++(*step);
        if (__sys_PostMessageW(hWnd, WM_DROPFILES, tzuk, (LPARAM)Args))
            return 0;
    }

    //
    // step 2:  invoked by the second WM_DROPFILES
    //

    if (*step < 10) {

        ULONG effect = DROPEFFECT_COPY;
        HRESULT hr = pDropTarget->DragOver(0, pt, &effect);
        if (SUCCEEDED(hr)) {
            Sleep(100);
            ++(*step);
            if ((*step) & 1) {
                ++Args[6];
                ++Args[7];
            } else {
                --Args[6];
                --Args[7];
            }
            if (__sys_PostMessageW(hWnd, WM_DROPFILES, tzuk, (LPARAM)Args))
                return 0;
        }
    }

    //
    // step 3:  invoked by the third WM_DROPFILES
    //

    if (*step >= 10) {

        ULONG effect = DROPEFFECT_COPY;
        pDropTarget->Drop(xDataObject, 0, pt, &effect);

        xDataObject->Release();
        __sys_DragFinish(hDrop);
    }

    //
    // clean up work data
    //

    Dll_Free(Args);
    return 0;
}


//---------------------------------------------------------------------------
// Ole_XDataObject_From_IDataObject
//---------------------------------------------------------------------------


extern "C" _FX IDataObject *Ole_XDataObject_From_IDataObject(
    IDataObject *pDataObject)
{
    ULONG len = sizeof(XDataObject);
    void *buf = Dll_Alloc(len);
    XDataObject *xDataObject = new (buf) XDataObject(pDataObject, NULL);
    return xDataObject;
}
