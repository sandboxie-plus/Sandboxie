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
// Windows Media Player Server Hooking
//---------------------------------------------------------------------------


#include <shobjidl.h>
#include <shlobj.h>


//---------------------------------------------------------------------------
// IExecuteCommand from Windows 7 ShObjIdl.h
//---------------------------------------------------------------------------


typedef struct IExecuteCommand IExecuteCommand;

typedef struct IExecuteCommandVtbl
{
    BEGIN_INTERFACE

    HRESULT ( STDMETHODCALLTYPE *QueryInterface )(
         IExecuteCommand * This, REFIID riid, void **ppvObject);

    ULONG ( STDMETHODCALLTYPE *AddRef )(
         IExecuteCommand * This);

    ULONG ( STDMETHODCALLTYPE *Release )(
         IExecuteCommand * This);

    HRESULT ( STDMETHODCALLTYPE *SetKeyState )(
         IExecuteCommand * This,
        /* [in] */ DWORD grfKeyState);

    HRESULT ( STDMETHODCALLTYPE *SetParameters )(
         IExecuteCommand * This, LPCWSTR pszParameters);

    HRESULT ( STDMETHODCALLTYPE *SetPosition )(
         IExecuteCommand * This,
        /* [in] */ POINT pt);

    HRESULT ( STDMETHODCALLTYPE *SetShowWindow )(
         IExecuteCommand * This,
        /* [in] */ int nShow);

    HRESULT ( STDMETHODCALLTYPE *SetNoShowUI )(
         IExecuteCommand * This,
        /* [in] */ BOOL fNoShowUI);

    HRESULT ( STDMETHODCALLTYPE *SetDirectory )(
         IExecuteCommand * This, LPCWSTR pszDirectory);

    HRESULT ( STDMETHODCALLTYPE *Execute )(
         IExecuteCommand * This);

    END_INTERFACE
} IExecuteCommandVtbl;


static HRESULT WMPServer_IExecuteCommand_SetKeyState(
    IExecuteCommand *This, DWORD grfKeyState);
static HRESULT WMPServer_IExecuteCommand_SetParameters(
    IExecuteCommand *This, LPCWSTR pszParameters);
static HRESULT WMPServer_IExecuteCommand_SetPosition(
    IExecuteCommand *This, POINT pt);
static HRESULT WMPServer_IExecuteCommand_SetShowWindow(
    IExecuteCommand *This, int nShow);
static HRESULT WMPServer_IExecuteCommand_SetNoShowUI(
    IExecuteCommand *This, BOOL fNoShowUI);
static HRESULT WMPServer_IExecuteCommand_SetDirectory(
    IExecuteCommand *This, LPCWSTR pszDirectory);
static HRESULT WMPServer_IExecuteCommand_Execute(
    IExecuteCommand *This);


//---------------------------------------------------------------------------
// IShellItemArray from Windows 7 ShObjIdl.h
//---------------------------------------------------------------------------


typedef ULONG GETPROPERTYSTOREFLAGS;
typedef ULONG SIATTRIBFLAGS;
typedef void *REFPROPERTYKEY;
typedef void *IEnumShellItems;

typedef struct IShellItemArray IShellItemArray;

typedef struct IShellItemArrayVtbl
{
    BEGIN_INTERFACE

    HRESULT ( STDMETHODCALLTYPE *QueryInterface )(
        IShellItemArray * This, REFIID riid, void **ppvObject);

    ULONG ( STDMETHODCALLTYPE *AddRef )(
        IShellItemArray * This);

    ULONG ( STDMETHODCALLTYPE *Release )(
        IShellItemArray * This);

    HRESULT ( STDMETHODCALLTYPE *BindToHandler )(
        IShellItemArray * This, IBindCtx *pbc, REFGUID bhid,
        REFIID riid, void **ppvOut);

    HRESULT ( STDMETHODCALLTYPE *GetPropertyStore )(
        IShellItemArray * This, GETPROPERTYSTOREFLAGS flags,
        REFIID riid, void **ppv);

    HRESULT ( STDMETHODCALLTYPE *GetPropertyDescriptionList )(
        IShellItemArray * This, REFPROPERTYKEY keyType,
        REFIID riid, void **ppv);

    HRESULT ( STDMETHODCALLTYPE *GetAttributes )(
        IShellItemArray * This, SIATTRIBFLAGS AttribFlags,
        SFGAOF sfgaoMask, SFGAOF *psfgaoAttribs);

    HRESULT ( STDMETHODCALLTYPE *GetCount )(
        IShellItemArray * This, DWORD *pdwNumItems);

    HRESULT ( STDMETHODCALLTYPE *GetItemAt )(
        IShellItemArray * This, DWORD dwIndex, IShellItem **ppsi);

    HRESULT ( STDMETHODCALLTYPE *EnumItems )(
        IShellItemArray * This, IEnumShellItems **ppenumShellItems);

    END_INTERFACE
} IShellItemArrayVtbl;

interface IShellItemArray
{
    struct IShellItemArrayVtbl *lpVtbl;
};


//---------------------------------------------------------------------------
// IObjectWithSelection from Windows 7 ShObjIdl.h
//---------------------------------------------------------------------------


typedef struct IObjectWithSelection IObjectWithSelection;

typedef struct IObjectWithSelectionVtbl
{
    BEGIN_INTERFACE

    HRESULT ( STDMETHODCALLTYPE *QueryInterface )(
        IObjectWithSelection * This, REFIID riid, void **ppvObject);

    ULONG ( STDMETHODCALLTYPE *AddRef )(
        IObjectWithSelection * This);

    ULONG ( STDMETHODCALLTYPE *Release )(
        IObjectWithSelection * This);

    HRESULT ( STDMETHODCALLTYPE *SetSelection )(
        IObjectWithSelection * This,
        IShellItemArray *psia);

    HRESULT ( STDMETHODCALLTYPE *GetSelection )(
        IObjectWithSelection * This, REFIID riid, void **ppv);

    END_INTERFACE
} IObjectWithSelectionVtbl;


static HRESULT WMPServer_IObjectWithSelection_SetSelection(
    IObjectWithSelection *This, IShellItemArray *psia);

static HRESULT WMPServer_IObjectWithSelection_GetSelection(
    IObjectWithSelection *This, REFIID riid, void **ppv);


//---------------------------------------------------------------------------
// IDropTarget
//---------------------------------------------------------------------------


static HRESULT WMPServer_IDropTarget_DragEnter(
    IDropTarget *This, IDataObject *pDataObject,
    DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

static HRESULT WMPServer_IDropTarget_DragOver(
    IDropTarget *This, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

static HRESULT WMPServer_IDropTarget_DragLeave(IDropTarget *This);

static HRESULT WMPServer_IDropTarget_Drop(
    IDropTarget *This, IDataObject *pDataObject,
    DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static IMyUnknown *WMPServer_MyCreateInstance(REFIID riid);
static IMyUnknown *WMPServer_MyCreateInstanceHelper(void);
static void *WMPServer_MyQueryInterface(IMyUnknown *This, REFIID riid);


//---------------------------------------------------------------------------
// GUIDs
//---------------------------------------------------------------------------


// {ED1D0FDF-4414-470A-A56D-CFB68623FC58}
static const GUID CLSID_WindowsMediaPlayer_Play = {
    0xED1D0FDF, 0x4414, 0x470A,
        { 0xA5, 0x6D, 0xCF, 0xB6, 0x86, 0x23, 0xFC, 0x58 }
};

// {45597C98-80F6-4549-84FF-752CF55E2D29}
static const GUID CLSID_WindowsMediaPlayer_Enqueue = {
    0x45597C98, 0x80F6, 0x4549,
        { 0x84, 0xFF, 0x75, 0x2C, 0xF5, 0x5E, 0x2D, 0x29 }
};

// {46986115-84D6-459C-8F95-52DD653E532E}
static const GUID CLSID_WinAmp = {
    0x46986115, 0x84D6, 0x459C,
        { 0x8F, 0x95, 0x52, 0xDD, 0x65, 0x3E, 0x53, 0x2E }
};

// {9EB4C4CB-74C2-4BE9-AA5D-8249F16020AD}
static const GUID CLSID_KmPlayer = {
    0x9EB4C4CB, 0x74C2, 0x4BE9,
        { 0xAA, 0x5D, 0x82, 0x49, 0xF1, 0x60, 0x20, 0xAD }
};

// {7F9185B0-CB92-43C5-80A9-92277A4F7B54}
static const GUID IID_IExecuteCommand = {
    0x7F9185B0, 0xCB92, 0x43C5,
        { 0x80, 0xA9, 0x92, 0x27, 0x7A, 0x4F, 0x7B, 0x54 }
};

// {1C9CD5BB-98E9-4491-A60F-31AACC72B83C}
static const GUID IID_IObjectWithSelection = {
    0x1C9CD5BB, 0x98E9, 0x4491,
        { 0xA6, 0x0F, 0x31, 0xAA, 0xCC, 0x72, 0xB8, 0x3C }
};


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static WCHAR *WMPServer_Parameters = NULL;


//---------------------------------------------------------------------------
// WMPServer_MyCreateInstance
//---------------------------------------------------------------------------


_FX IMyUnknown *WMPServer_MyCreateInstance(REFIID riid)
{
    if (IsEqualIID(riid, &IID_IUnknown)             ||
        IsEqualIID(riid, &IID_IExecuteCommand)      ||
        IsEqualIID(riid, &IID_IObjectWithSelection) ||
        0) {

        return WMPServer_MyCreateInstanceHelper();
    }

    if (ComServer_ImageType == ComServer_ImageType_WINAMP ||
        ComServer_ImageType == ComServer_ImageType_KMPLAYER) {

        if (IsEqualIID(riid, &IID_IDropTarget)) {

            return WMPServer_MyCreateInstanceHelper();
        }
    }

    return NULL;
}


//---------------------------------------------------------------------------
// WMPServer_MyCreateInstanceHelper
//---------------------------------------------------------------------------


_FX IMyUnknown *WMPServer_MyCreateInstanceHelper(void)
{
    ULONG SizeofVtbls;
    IMyUnknown *This;
    ULONG_PTR *ptr;

    SizeofVtbls = 0
        + sizeof(ULONG_PTR) + sizeof(IExecuteCommandVtbl)
        + sizeof(ULONG_PTR) + sizeof(IObjectWithSelectionVtbl)
        + sizeof(ULONG_PTR) + sizeof(IDropTargetVtbl);

    This = ComServer_MyUnknown_New(WMPServer_MyQueryInterface, SizeofVtbls);

    ptr = (ULONG_PTR *)&This->VtblSpace;

    *ptr = (ULONG_PTR)This;
    ++ptr;
    This->Vtbls[1] = (ULONG_PTR)ptr;
    ptr[0] = (ULONG_PTR)ComServer_IUnknown_QueryInterface;
    ptr[1] = (ULONG_PTR)ComServer_IUnknown_AddRef;
    ptr[2] = (ULONG_PTR)ComServer_IUnknown_Release;
    ptr[3] = (ULONG_PTR)WMPServer_IExecuteCommand_SetKeyState;
    ptr[4] = (ULONG_PTR)WMPServer_IExecuteCommand_SetParameters;
    ptr[5] = (ULONG_PTR)WMPServer_IExecuteCommand_SetPosition;
    ptr[6] = (ULONG_PTR)WMPServer_IExecuteCommand_SetShowWindow;
    ptr[7] = (ULONG_PTR)WMPServer_IExecuteCommand_SetNoShowUI;
    ptr[8] = (ULONG_PTR)WMPServer_IExecuteCommand_SetDirectory;
    ptr[9] = (ULONG_PTR)WMPServer_IExecuteCommand_Execute;
    ptr += sizeof(IExecuteCommandVtbl) / sizeof(ULONG_PTR);

    *ptr = (ULONG_PTR)This;
    ++ptr;
    This->Vtbls[2] = (ULONG_PTR)ptr;
    ptr[0] = (ULONG_PTR)ComServer_IUnknown_QueryInterface;
    ptr[1] = (ULONG_PTR)ComServer_IUnknown_AddRef;
    ptr[2] = (ULONG_PTR)ComServer_IUnknown_Release;
    ptr[3] = (ULONG_PTR)WMPServer_IObjectWithSelection_SetSelection;
    ptr[4] = (ULONG_PTR)WMPServer_IObjectWithSelection_GetSelection;
    ptr += sizeof(IObjectWithSelectionVtbl) / sizeof(ULONG_PTR);

    *ptr = (ULONG_PTR)This;
    ++ptr;
    This->Vtbls[3] = (ULONG_PTR)ptr;
    ptr[0] = (ULONG_PTR)ComServer_IUnknown_QueryInterface;
    ptr[1] = (ULONG_PTR)ComServer_IUnknown_AddRef;
    ptr[2] = (ULONG_PTR)ComServer_IUnknown_Release;
    ptr[3] = (ULONG_PTR)WMPServer_IDropTarget_DragEnter;
    ptr[4] = (ULONG_PTR)WMPServer_IDropTarget_DragOver;
    ptr[5] = (ULONG_PTR)WMPServer_IDropTarget_DragLeave;
    ptr[6] = (ULONG_PTR)WMPServer_IDropTarget_Drop;
    ptr += sizeof(IDropTargetVtbl) / sizeof(ULONG_PTR);

    return This;
}



//---------------------------------------------------------------------------
// WMPServer_MyQueryInterface
//---------------------------------------------------------------------------


_FX void *WMPServer_MyQueryInterface(IMyUnknown *This, REFIID riid)
{
    if (IsEqualIID(riid, &IID_IUnknown))
        return (void *)&This->Vtbls[0];

    if (IsEqualIID(riid, &IID_IExecuteCommand))
        return (void *)&This->Vtbls[1];

    if (IsEqualIID(riid, &IID_IObjectWithSelection))
        return (void *)&This->Vtbls[2];

    if (ComServer_ImageType == ComServer_ImageType_WINAMP ||
        ComServer_ImageType == ComServer_ImageType_KMPLAYER) {

        if (IsEqualIID(riid, &IID_IDropTarget))
            return (void *)&This->Vtbls[3];
    }

    return NULL;
}


//---------------------------------------------------------------------------
// WMPServer_IExecuteCommand_SetKeyState
//---------------------------------------------------------------------------


_FX HRESULT WMPServer_IExecuteCommand_SetKeyState(
    IExecuteCommand *This, DWORD grfKeyState)
{
    return S_OK;
}


//---------------------------------------------------------------------------
// WMPServer_IExecuteCommand_SetParameters
//---------------------------------------------------------------------------


_FX HRESULT WMPServer_IExecuteCommand_SetParameters(
    IExecuteCommand *This, LPCWSTR pszParameters)
{
    if (pszParameters) {

        ULONG len = wcslen(pszParameters);
        WMPServer_Parameters = Dll_Alloc((len + 1) * sizeof(WCHAR));
        wmemcmp(WMPServer_Parameters, pszParameters, len);
        WMPServer_Parameters[len] = L'\0';

        while (*WMPServer_Parameters == L' ')
            ++WMPServer_Parameters;
        if (*WMPServer_Parameters == L'\0')
            WMPServer_Parameters = NULL;

    } else
        WMPServer_Parameters = NULL;

#ifdef COMSERVER_DEBUG
    { WCHAR txt[128];
    swprintf(txt, L"WMPServer_IExecuteCommand_SetParameters - <%s>\n", WMPServer_Parameters);
    OutputDebugString(txt); }
#endif

    return S_OK;
}


//---------------------------------------------------------------------------
// WMPServer_IExecuteCommand_SetPosition
//---------------------------------------------------------------------------


_FX HRESULT WMPServer_IExecuteCommand_SetPosition(
    IExecuteCommand *This, POINT pt)
{
    return S_OK;
}


//---------------------------------------------------------------------------
// WMPServer_IExecuteCommand_SetShowWindow
//---------------------------------------------------------------------------


_FX HRESULT WMPServer_IExecuteCommand_SetShowWindow(
    IExecuteCommand *This, int nShow)
{
    return S_OK;
}


//---------------------------------------------------------------------------
// WMPServer_IExecuteCommand_SetNoShowUI
//---------------------------------------------------------------------------


_FX HRESULT WMPServer_IExecuteCommand_SetNoShowUI(
    IExecuteCommand *This, BOOL fNoShowUI)
{
    return S_OK;
}


//---------------------------------------------------------------------------
// WMPServer_IExecuteCommand_SetDirectory
//---------------------------------------------------------------------------


_FX HRESULT WMPServer_IExecuteCommand_SetDirectory(
    IExecuteCommand *This, LPCWSTR pszDirectory)
{
#ifdef COMSERVER_DEBUG
    WCHAR txt[128];
    swprintf(txt, L"WMPServer_IExecuteCommand_SetDirectory - <%s>\n", pszDirectory);
    OutputDebugString(txt);
#endif

    SetCurrentDirectory(pszDirectory);
    return S_OK;
}


//---------------------------------------------------------------------------
// WMPServer_IExecuteCommand_Execute
//---------------------------------------------------------------------------


_FX HRESULT WMPServer_IExecuteCommand_Execute(
    IExecuteCommand *This)
{
    WCHAR *arg = WMPServer_Parameters;
    if (! arg)
        arg = L"";

    if (wcslen(arg) > 1 && arg[wcslen(arg) - 1] == L'\"')
        arg[wcslen(arg) - 1] = L'\0';
    if (*arg == L'\"')
        ++arg;

    ComServer_RestartProgram(arg);

    return S_OK;
}


//---------------------------------------------------------------------------
// WMPServer_IObjectWithSelection_SetSelection
//---------------------------------------------------------------------------


_FX HRESULT WMPServer_IObjectWithSelection_SetSelection(
    IObjectWithSelection *This, IShellItemArray *psia)
{
    ULONG index = 0;
    while (1) {

        WCHAR *path1, *path2;
        ULONG len;

        IShellItem *pShellItem;
        HRESULT hr = psia->lpVtbl->GetItemAt(psia, index, &pShellItem);
        if (FAILED(hr))
            break;
        ++index;

        hr = IShellItem_GetDisplayName(pShellItem, SIGDN_FILESYSPATH, &path1);
        if (SUCCEEDED(hr)) {

            if (WMPServer_Parameters)
                len = wcslen(WMPServer_Parameters);
            else
                len = 0;
            len += wcslen(path1) + 8;

            path2 = Dll_Alloc(len * sizeof(WCHAR));
            if (WMPServer_Parameters) {
                wcscpy(path2, WMPServer_Parameters);
                wcscat(path2, L" \"");
            } else
                wcscpy(path2, L"\"");
            wcscat(path2, path1);
            wcscat(path2, L"\"");

            WMPServer_Parameters = path2;
        }

        IShellItem_Release(pShellItem);
    }

#ifdef COMSERVER_DEBUG
    { WCHAR txt[512];
    swprintf(txt, L"WMPServer_IObjectWithSelection_SetSelection - <%s>\n", WMPServer_Parameters);
    OutputDebugString(txt); }
#endif

    return S_OK;
}


//---------------------------------------------------------------------------
// WMPServer_IObjectWithSelection_GetSelection
//---------------------------------------------------------------------------


_FX HRESULT WMPServer_IObjectWithSelection_GetSelection(
    IObjectWithSelection *This, REFIID riid, void **ppv)
{
    return S_OK;
}


//---------------------------------------------------------------------------
// WMPServer_IDropTarget_DragEnter
//---------------------------------------------------------------------------


_FX HRESULT WMPServer_IDropTarget_DragEnter(
    IDropTarget *This, IDataObject *pDataObject,
    DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
#ifdef COMSERVER_DEBUG
    { OutputDebugString(L"WMPServer_IDropTarget_DragEnter\n"); }
#endif
    *pdwEffect = DROPEFFECT_COPY;
    return S_OK;
}


//---------------------------------------------------------------------------
// WMPServer_IDropTarget_DragOver
//---------------------------------------------------------------------------


_FX HRESULT WMPServer_IDropTarget_DragOver(
    IDropTarget *This, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
#ifdef COMSERVER_DEBUG
    { OutputDebugString(L"WMPServer_IDropTarget_DragOver\n"); }
#endif
    *pdwEffect = DROPEFFECT_COPY;
    return S_OK;
}


//---------------------------------------------------------------------------
// WMPServer_IDropTarget_DragLeave
//---------------------------------------------------------------------------


_FX HRESULT WMPServer_IDropTarget_DragLeave(IDropTarget *This)
{
#ifdef COMSERVER_DEBUG
    { OutputDebugString(L"WMPServer_IDropTarget_DragLeave\n"); }
#endif
    return S_OK;
}


//---------------------------------------------------------------------------
// WMPServer_IDropTarget_Drop
//---------------------------------------------------------------------------


_FX HRESULT WMPServer_IDropTarget_Drop(
    IDropTarget *This, IDataObject *pDataObject,
    DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    HRESULT hr;
    FORMATETC format;
    STGMEDIUM medium;

#ifdef COMSERVER_DEBUG
    { OutputDebugString(L"WMPServer_IDropTarget_Drop\n"); }
#endif

    format.cfFormat = CF_HDROP;
    format.ptd = NULL;
    format.dwAspect = DVASPECT_CONTENT;
    format.lindex = -1;
    format.tymed = TYMED_HGLOBAL;

    memzero(&medium, sizeof(medium));

    hr = IDataObject_GetData(pDataObject, &format, &medium);

    if (FAILED(hr))
        return hr;

    if (medium.hGlobal) {

        DROPFILES *DropFiles = GlobalLock(medium.hGlobal);
        if (DropFiles->fWide) {

            WCHAR *path = (WCHAR *)((UCHAR *)DropFiles + DropFiles->pFiles);
            ComServer_RestartProgram(path);
        }

        GlobalUnlock(medium.hGlobal);
    }

    if (medium.pUnkForRelease)
        IUnknown_Release(medium.pUnkForRelease);
    else if (medium.hGlobal)
        GlobalFree(medium.hGlobal);

    *pdwEffect = DROPEFFECT_COPY;

    return S_OK;
}
