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
// Internet Explorer Server Hooking
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static IMyUnknown *IEServer_MyCreateInstance(REFIID riid);
static IMyUnknown *IEServer_MyCreateInstanceHelper(void);
static void *IEServer_MyQueryInterface(IMyUnknown *This, REFIID riid);


static HRESULT IEServer_IOleCommandTarget_QueryStatus(
    IOleCommandTarget *This, const GUID *pguidCmdGroup, ULONG cCmds,
    OLECMD prgCmds[], OLECMDTEXT *pCmdText);
static HRESULT IEServer_IOleCommandTarget_Exec(
    IOleCommandTarget *This, const GUID *pguidCmdGroup, DWORD nCmdID,
    DWORD nCmdexecopt, VARIANT *pvaIn, VARIANT *pvaOut);


static HRESULT IEServer_ITargetFramePriv_FindFrameDownwards(
    ITargetFramePriv *This,
    LPCWSTR pszTargetName, DWORD dwFlags, IUnknown **ppunkTargetFrame);
static HRESULT IEServer_ITargetFramePriv_FindFrameInContext(
    ITargetFramePriv *This,
    LPCWSTR pszTargetName, IUnknown *punkContextFrame,
    DWORD dwFlags, IUnknown **ppunkTargetFrame);
static HRESULT IEServer_ITargetFramePriv_OnChildFrameActivate(
    ITargetFramePriv *This,
    IUnknown *pUnkChildFrame);
static HRESULT IEServer_ITargetFramePriv_OnChildFrameDeactivate(
    ITargetFramePriv *This,
    IUnknown *pUnkChildFrame);
static HRESULT IEServer_ITargetFramePriv_NavigateHack(
    ITargetFramePriv *This,
    DWORD grfHLNF, LPBC pbc, IBindStatusCallback *pibsc,
    LPCWSTR pszTargetName, LPCWSTR pszUrl, LPCWSTR pszLocation);
static HRESULT IEServer_ITargetFramePriv_FindBrowserByIndex(
    ITargetFramePriv *This,
    DWORD dwID, IUnknown **ppunkBrowser);
static HRESULT IEServer_ITargetFramePriv2_AggregatedNavigation2(
    ITargetFramePriv2 *This,
    DWORD grfHLNF, LPBC pbc, IBindStatusCallback *pibsc,
    LPCWSTR pszTargetName, IUri *pUri, LPCWSTR pszLocation);

static HRESULT IEServer_IWebBrowser2_NotImpl(
    IWebBrowser2 *This);
static HRESULT IEServer_IWebBrowser2_Navigate(
    IWebBrowser2 *This,
    BSTR url, VARIANT *Flags, VARIANT *TargetFrameName,
    VARIANT *PostData, VARIANT *Headers);
static HRESULT IEServer_IWebBrowser2_put_Visible(
    IWebBrowser2 *This, VARIANT_BOOL Value);
static HRESULT IEServer_IWebBrowser2_Navigate2(
    IWebBrowser2 *This,
    VARIANT *URL, VARIANT *Flags, VARIANT *TargetFrameName,
    VARIANT *PostData, VARIANT *Headers);


static HRESULT IEServer_ITargetFrame2_NotImpl(ITargetFrame2 *This);
static HRESULT IEServer_ITargetFrame2_SetFrameName(
    ITargetFrame2 *This, LPCWSTR pszFrameName);
static HRESULT IEServer_ITargetFrame2_SetFrameSrc(
    ITargetFrame2 *This, LPCWSTR pszFrameSrc);


static HRESULT IEServer_IHlinkFrame_NotImpl(IHlinkFrame *This);
static HRESULT IEServer_IHlinkFrame_Navigate(
    IHlinkFrame *This, DWORD grfHLNF, LPBC pbc, IBindStatusCallback *pibsc,
    IHlink *pihlNavigate);


static HRESULT IEServer_IServiceProvider_QueryService(
    IHTMLWindow2 *This, REFGUID guidService, REFIID riid, void **ppvObject);


static HRESULT IEServer_IHTMLWindow2_NotImpl(IHTMLWindow2 *This);


static void IEServer_RestartProgram(const WCHAR *arg);
static WCHAR *IEServer_ResolveUrl(WCHAR *path);


//---------------------------------------------------------------------------
// GUIDs
//---------------------------------------------------------------------------


// {B722BCCB-4E68-101B-A2BC-00AA00404770}
static const GUID IID_IOleCommandTarget = {
    0xB722BCCB, 0x4E68, 0x101B,
        { 0xA2, 0xBC, 0x00, 0xAA, 0x00, 0x40, 0x47, 0x70 }
};

// {0002DF01-0000-0000-C000-000000000046}
static const GUID CLSID_InternetExplorer = {
    0x0002DF01, 0x0000, 0x0000,
        { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 }
};

// {B2C867E6-69D6-46F2-A611-DED9A4BD7FEF}
static const GUID IID_ITargetFramePriv2 = {
    0xB2C867E6, 0x69D6, 0x46F2,
        { 0xA6, 0x11, 0xDE, 0xD9, 0xA4, 0xBD, 0x7F, 0xEF }
};

// {000214D0-0000-0000-C000-000000000046}
static const GUID CGID_Explorer = {
    0x000214D0, 0x0000, 0x0000,
        { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 }
};


//---------------------------------------------------------------------------
// IEServer_MyCreateInstance
//---------------------------------------------------------------------------


_FX IMyUnknown *IEServer_MyCreateInstance(REFIID riid)
{
    if (IsEqualIID(riid, &IID_IUnknown)             ||
        IsEqualIID(riid, &IID_IOleCommandTarget)    ||
        IsEqualIID(riid, &IID_ITargetFramePriv)     ||
        IsEqualIID(riid, &IID_ITargetFramePriv2)    ||
        IsEqualIID(riid, &IID_IWebBrowserApp)       ||
        IsEqualIID(riid, &IID_IWebBrowser2)         ||
        IsEqualIID(riid, &IID_ITargetFrame2)        ||
        IsEqualIID(riid, &IID_IHlinkFrame)          ||
        IsEqualIID(riid, &IID_IServiceProvider)     ||
        0) {

        return IEServer_MyCreateInstanceHelper();
    }

    return NULL;
}


//---------------------------------------------------------------------------
// IEServer_MyCreateInstanceHelper
//---------------------------------------------------------------------------


_FX IMyUnknown *IEServer_MyCreateInstanceHelper(void)
{
    ULONG SizeofVtbls;
    IMyUnknown *This;
    ULONG_PTR *ptr;
    ULONG i;

    SizeofVtbls = 0
        + sizeof(ULONG_PTR) + sizeof(IOleCommandTargetVtbl)
        + sizeof(ULONG_PTR) + sizeof(ITargetFramePriv2Vtbl)
        + sizeof(ULONG_PTR) + sizeof(IWebBrowser2Vtbl)
        + sizeof(ULONG_PTR) + sizeof(ITargetFrame2Vtbl)
        + sizeof(ULONG_PTR) + sizeof(IHlinkFrameVtbl)
        + sizeof(ULONG_PTR) + sizeof(IServiceProviderVtbl)
        + sizeof(ULONG_PTR) + sizeof(IHTMLWindow2Vtbl);

    This = ComServer_MyUnknown_New(IEServer_MyQueryInterface, SizeofVtbls);

    ptr = (ULONG_PTR *)&This->VtblSpace;

    *ptr = (ULONG_PTR)This;
    ++ptr;
    This->Vtbls[1] = (ULONG_PTR)ptr;
    ptr[0] = (ULONG_PTR)ComServer_IUnknown_QueryInterface;
    ptr[1] = (ULONG_PTR)ComServer_IUnknown_AddRef;
    ptr[2] = (ULONG_PTR)ComServer_IUnknown_Release;
    ptr[3] = (ULONG_PTR)IEServer_IOleCommandTarget_QueryStatus;
    ptr[4] = (ULONG_PTR)IEServer_IOleCommandTarget_Exec;
    ptr += sizeof(IOleCommandTargetVtbl) / sizeof(ULONG_PTR);

    *ptr = (ULONG_PTR)This;
    ++ptr;
    This->Vtbls[2] = (ULONG_PTR)ptr;
    ptr[0] = (ULONG_PTR)ComServer_IUnknown_QueryInterface;
    ptr[1] = (ULONG_PTR)ComServer_IUnknown_AddRef;
    ptr[2] = (ULONG_PTR)ComServer_IUnknown_Release;
    ptr[3] = (ULONG_PTR)IEServer_ITargetFramePriv_FindFrameDownwards;
    ptr[4] = (ULONG_PTR)IEServer_ITargetFramePriv_FindFrameInContext;
    ptr[5] = (ULONG_PTR)IEServer_ITargetFramePriv_OnChildFrameActivate;
    ptr[6] = (ULONG_PTR)IEServer_ITargetFramePriv_OnChildFrameDeactivate;
    ptr[7] = (ULONG_PTR)IEServer_ITargetFramePriv_NavigateHack;
    ptr[8] = (ULONG_PTR)IEServer_ITargetFramePriv_FindBrowserByIndex;
    ptr[9] = (ULONG_PTR)IEServer_ITargetFramePriv2_AggregatedNavigation2;
    ptr += sizeof(ITargetFramePriv2Vtbl) / sizeof(ULONG_PTR);

    *ptr = (ULONG_PTR)This;
    ++ptr;
    This->Vtbls[3] = (ULONG_PTR)ptr;
    for (i = 0; i < sizeof(IWebBrowser2Vtbl) / sizeof(ULONG_PTR); ++i)
        ptr[i] = (ULONG_PTR)IEServer_IWebBrowser2_NotImpl;
    ptr[0] = (ULONG_PTR)ComServer_IUnknown_QueryInterface;
    ptr[1] = (ULONG_PTR)ComServer_IUnknown_AddRef;
    ptr[2] = (ULONG_PTR)ComServer_IUnknown_Release;
    ptr[11] = (ULONG_PTR)IEServer_IWebBrowser2_Navigate;
    ptr[41] = (ULONG_PTR)IEServer_IWebBrowser2_put_Visible;
    ptr[52] = (ULONG_PTR)IEServer_IWebBrowser2_Navigate2;
    ptr += sizeof(IWebBrowser2Vtbl) / sizeof(ULONG_PTR);

    *ptr = (ULONG_PTR)This;
    ++ptr;
    This->Vtbls[4] = (ULONG_PTR)ptr;
    for (i = 0; i < sizeof(ITargetFrame2Vtbl) / sizeof(ULONG_PTR); ++i)
        ptr[i] = (ULONG_PTR)IEServer_ITargetFrame2_NotImpl;
    ptr[0] = (ULONG_PTR)ComServer_IUnknown_QueryInterface;
    ptr[1] = (ULONG_PTR)ComServer_IUnknown_AddRef;
    ptr[2] = (ULONG_PTR)ComServer_IUnknown_Release;
    ptr[3] = (ULONG_PTR)IEServer_ITargetFrame2_SetFrameName;
    ptr[6] = (ULONG_PTR)IEServer_ITargetFrame2_SetFrameSrc;
    ptr += sizeof(ITargetFrame2Vtbl) / sizeof(ULONG_PTR);

    *ptr = (ULONG_PTR)This;
    ++ptr;
    This->Vtbls[5] = (ULONG_PTR)ptr;
    for (i = 0; i < sizeof(IHlinkFrameVtbl) / sizeof(ULONG_PTR); ++i)
        ptr[i] = (ULONG_PTR)IEServer_IHlinkFrame_NotImpl;
    ptr[0] = (ULONG_PTR)ComServer_IUnknown_QueryInterface;
    ptr[1] = (ULONG_PTR)ComServer_IUnknown_AddRef;
    ptr[2] = (ULONG_PTR)ComServer_IUnknown_Release;
    ptr[5] = (ULONG_PTR)IEServer_IHlinkFrame_Navigate;
    ptr += sizeof(IHlinkFrameVtbl) / sizeof(ULONG_PTR);

    *ptr = (ULONG_PTR)This;
    ++ptr;
    This->Vtbls[6] = (ULONG_PTR)ptr;
    ptr[0] = (ULONG_PTR)ComServer_IUnknown_QueryInterface;
    ptr[1] = (ULONG_PTR)ComServer_IUnknown_AddRef;
    ptr[2] = (ULONG_PTR)ComServer_IUnknown_Release;
    ptr[3] = (ULONG_PTR)IEServer_IServiceProvider_QueryService;
    ptr += sizeof(IServiceProviderVtbl) / sizeof(ULONG_PTR);

    *ptr = (ULONG_PTR)This;
    ++ptr;
    This->Vtbls[7] = (ULONG_PTR)ptr;
    for (i = 0; i < sizeof(IHTMLWindow2Vtbl) / sizeof(ULONG_PTR); ++i)
        ptr[i] = (ULONG_PTR)IEServer_IHTMLWindow2_NotImpl;
    ptr[0] = (ULONG_PTR)ComServer_IUnknown_QueryInterface;
    ptr[1] = (ULONG_PTR)ComServer_IUnknown_Release;
    ptr[2] = (ULONG_PTR)ComServer_IUnknown_Release;
    ptr += sizeof(IHTMLWindow2Vtbl) / sizeof(ULONG_PTR);

    return This;
}


//---------------------------------------------------------------------------
// IEServer_MyQueryInterface
//---------------------------------------------------------------------------


_FX void *IEServer_MyQueryInterface(IMyUnknown *This, REFIID riid)
{
    if (IsEqualIID(riid, &IID_IOleCommandTarget))
        return (void *)&This->Vtbls[1];

    if (IsEqualIID(riid, &IID_ITargetFramePriv) ||
        IsEqualIID(riid, &IID_ITargetFramePriv2))
        return (void *)&This->Vtbls[2];

    if (IsEqualIID(riid, &IID_IWebBrowserApp) ||
        IsEqualIID(riid, &IID_IWebBrowser2))
        return (void *)&This->Vtbls[3];

    if (IsEqualIID(riid, &IID_ITargetFrame2))
        return (void *)&This->Vtbls[4];

    if (IsEqualIID(riid, &IID_IHlinkFrame))
        return (void *)&This->Vtbls[5];

    if (IsEqualIID(riid, &IID_IServiceProvider))
        return (void *)&This->Vtbls[6];

    return NULL;
}


//---------------------------------------------------------------------------
// IEServer_IOleCommandTarget_QueryStatus
//---------------------------------------------------------------------------


_FX HRESULT IEServer_IOleCommandTarget_QueryStatus(
    IOleCommandTarget *This, const GUID *pguidCmdGroup, ULONG cCmds,
    OLECMD prgCmds[], OLECMDTEXT *pCmdText)
{
    // OutputDebugString(L"IEServer_IOleCommandTarget_QueryStatus\n");
    return E_FAIL;
}


//---------------------------------------------------------------------------
// IEServer_IOleCommandTarget_Exec
//---------------------------------------------------------------------------


_FX HRESULT IEServer_IOleCommandTarget_Exec(
    IOleCommandTarget *This, const GUID *pguidCmdGroup, DWORD nCmdID,
    DWORD nCmdexecopt, VARIANT *pvaIn, VARIANT *pvaOut)
{
    /*
        WCHAR txt[128];
        swprintf(txt, L"IEServer_IOleCommandTarget_Exec - CmdGroup=%08X-%08X-%08X-%08X CmdId=%08X CmdOpts=%08X\n",
        ((ULONG *)pguidCmdGroup)[0], ((ULONG *)pguidCmdGroup)[1],
        ((ULONG *)pguidCmdGroup)[2], ((ULONG *)pguidCmdGroup)[3],
        nCmdID, nCmdexecopt);
        OutputDebugString(txt);
    */

    if (! IsEqualIID(pguidCmdGroup, &CGID_Explorer))
        return OLECMDERR_E_UNKNOWNGROUP;
    if (nCmdID != 0x2D || (! pvaIn) || pvaOut || pvaIn->vt != VT_BSTR)
        return OLECMDERR_E_NOTSUPPORTED;

    IEServer_RestartProgram(pvaIn->bstrVal);
    return S_OK;
}


//---------------------------------------------------------------------------
// IEServer_ITargetFramePriv_FindFrameDownwards
//---------------------------------------------------------------------------


_FX HRESULT IEServer_ITargetFramePriv_FindFrameDownwards(
    ITargetFramePriv *This,
    LPCWSTR pszTargetName, DWORD dwFlags, IUnknown **ppunkTargetFrame)
{
#ifdef COMSERVER_DEBUG
    OutputDebugString(L"ITargetFramePriv::FindFrameDownwards\n");
#endif
    return E_NOTIMPL;
}


//---------------------------------------------------------------------------
// IEServer_ITargetFramePriv_FindFrameInContext
//---------------------------------------------------------------------------


_FX HRESULT IEServer_ITargetFramePriv_FindFrameInContext(
    ITargetFramePriv *This,
    LPCWSTR pszTargetName, IUnknown *punkContextFrame,
    DWORD dwFlags, IUnknown **ppunkTargetFrame)
{
#ifdef COMSERVER_DEBUG
    OutputDebugString(L"ITargetFramePriv::FindFrameInContext\n");
#endif
    return E_NOTIMPL;
}


//---------------------------------------------------------------------------
// IEServer_ITargetFramePriv_OnChildFrameActivate
//---------------------------------------------------------------------------


_FX HRESULT IEServer_ITargetFramePriv_OnChildFrameActivate(
    ITargetFramePriv *This,
    IUnknown *pUnkChildFrame)
{
#ifdef COMSERVER_DEBUG
    OutputDebugString(L"ITargetFramePriv::OnChildFrameActivate\n");
#endif
    return E_NOTIMPL;
}


//---------------------------------------------------------------------------
// IEServer_ITargetFramePriv_OnChildFrameDeactivate
//---------------------------------------------------------------------------


_FX HRESULT IEServer_ITargetFramePriv_OnChildFrameDeactivate(
    ITargetFramePriv *This,
    IUnknown *pUnkChildFrame)
{
#ifdef COMSERVER_DEBUG
    OutputDebugString(L"ITargetFramePriv::OnChildFrameDeactivate\n");
#endif
    return E_NOTIMPL;
}


//---------------------------------------------------------------------------
// IEServer_ITargetFramePriv_NavigateHack
//---------------------------------------------------------------------------


_FX HRESULT IEServer_ITargetFramePriv_NavigateHack(
    ITargetFramePriv *This,
    DWORD grfHLNF, LPBC pbc, IBindStatusCallback *pibsc,
    LPCWSTR pszTargetName, LPCWSTR pszUrl, LPCWSTR pszLocation)
{
#ifdef COMSERVER_DEBUG
    OutputDebugString(L"ITargetFramePriv::NavigateHack\n");
#endif
    if (pszLocation) {
        ULONG len = (wcslen(pszUrl) + wcslen(pszLocation) + 4)
                  * sizeof(WCHAR);
        WCHAR *url = Dll_Alloc(len);
        wcscpy(url, pszUrl);
        wcscat(url, pszLocation);
        pszUrl = url;
    }
    IEServer_RestartProgram(pszUrl);
    return S_OK;
}


//---------------------------------------------------------------------------
// IEServer_ITargetFramePriv_FindBrowserByIndex
//---------------------------------------------------------------------------


_FX HRESULT IEServer_ITargetFramePriv_FindBrowserByIndex(
    ITargetFramePriv *This,
    DWORD dwID, IUnknown **ppunkBrowser)
{
#ifdef COMSERVER_DEBUG
    OutputDebugString(L"ITargetFramePriv::FindBrowserByIndex\n");
#endif
    return E_NOTIMPL;
}


//---------------------------------------------------------------------------
// IEServer_ITargetFramePriv2_AggregatedNavigation2
//---------------------------------------------------------------------------


_FX HRESULT IEServer_ITargetFramePriv2_AggregatedNavigation2(
    ITargetFramePriv2 *This,
    DWORD grfHLNF, LPBC pbc, IBindStatusCallback *pibsc,
    LPCWSTR pszTargetName, IUri *pUri, LPCWSTR pszLocation)
{
    HRESULT hr;
    BSTR pszUrl;
#ifdef COMSERVER_DEBUG
    OutputDebugString(L"ITargetFramePriv2::AggregatedNavigation2\n");
#endif
    hr = IUri_GetRawUri(pUri, &pszUrl);
    if (SUCCEEDED(hr)) {
        hr = ITargetFramePriv2_NavigateHack(
            This, grfHLNF, pbc, pibsc,pszTargetName, pszUrl, pszLocation);
    }
    return hr;
}


//---------------------------------------------------------------------------
// IEServer_IWebBrowser2_NotImpl
//---------------------------------------------------------------------------


_FX HRESULT IEServer_IWebBrowser2_NotImpl(IWebBrowser2 *This)
{
#ifdef COMSERVER_DEBUG
    OutputDebugString(L"IWebBrowser2::NotImpl\n");
#endif
    return E_NOTIMPL;
}


//---------------------------------------------------------------------------
// IEServer_IWebBrowser2_Navigate
//---------------------------------------------------------------------------


_FX HRESULT IEServer_IWebBrowser2_Navigate(
    IWebBrowser2 *This,
    BSTR url, VARIANT *Flags, VARIANT *TargetFrameName,
    VARIANT *PostData, VARIANT *Headers)
{
    IEServer_RestartProgram(url);
    return S_OK;
}


//---------------------------------------------------------------------------
// IEServer_IWebBrowser2_put_Visible
//---------------------------------------------------------------------------


static HRESULT IEServer_IWebBrowser2_put_Visible(
    IWebBrowser2 *This, VARIANT_BOOL Value)
{
#ifdef COMSERVER_DEBUG
    OutputDebugString(L"IWebBrowser2::put_Visible\n");
#endif
    return S_OK;
}

//---------------------------------------------------------------------------
// IEServer_IWebBrowser2_Navigate2
//---------------------------------------------------------------------------


_FX HRESULT IEServer_IWebBrowser2_Navigate2(
    IWebBrowser2 *This,
    VARIANT *URL, VARIANT *Flags, VARIANT *TargetFrameName,
    VARIANT *PostData, VARIANT *Headers)
{
    IEServer_RestartProgram(URL->bstrVal);
    return S_OK;
}


//---------------------------------------------------------------------------
// IEServer_ITargetFrame2_NotImpl
//---------------------------------------------------------------------------


_FX HRESULT IEServer_ITargetFrame2_NotImpl(ITargetFrame2 *This)
{
#ifdef COMSERVER_DEBUG
    OutputDebugString(L"ITargetFrame2::NotImpl\n");
#endif
    return E_NOTIMPL;
}


//---------------------------------------------------------------------------
// IEServer_ITargetFrame2_SetFrameName
//---------------------------------------------------------------------------


_FX HRESULT IEServer_ITargetFrame2_SetFrameName(
    ITargetFrame2 *This, LPCWSTR pszFrameName)
{
    return S_OK;
}


//---------------------------------------------------------------------------
// IEServer_ITargetFrame2_SetFrameSrc
//---------------------------------------------------------------------------


_FX HRESULT IEServer_ITargetFrame2_SetFrameSrc(
    ITargetFrame2 *This, LPCWSTR pszFrameSrc)
{
    IEServer_RestartProgram(pszFrameSrc);
    return S_OK;
}


//---------------------------------------------------------------------------
// IEServer_IHlinkFrame_NotImpl
//---------------------------------------------------------------------------


_FX HRESULT IEServer_IHlinkFrame_NotImpl(IHlinkFrame *This)
{
#ifdef COMSERVER_DEBUG
    OutputDebugString(L"IHlinkFrame::NotImpl\n");
#endif
    return E_NOTIMPL;
}


//---------------------------------------------------------------------------
// IEServer_IHlinkFrame_Navigate
//---------------------------------------------------------------------------


_FX HRESULT IEServer_IHlinkFrame_Navigate(
    IHlinkFrame *This, DWORD grfHLNF, LPBC pbc, IBindStatusCallback *pibsc,
    IHlink *pihlNavigate)
{
#ifdef COMSERVER_DEBUG
    WCHAR txt[128];
    swprintf(txt, L"IEServer_IHlinkFrame_Navigate - %08X-%08X-%08X-%08X\n",
        grfHLNF, pbc, pibsc, pihlNavigate);
    OutputDebugString(txt);
#endif

    if (pihlNavigate) {

        WCHAR *tgt, *loc, *url;
        ULONG len;

        HRESULT hr = IHlink_GetStringReference(
            pihlNavigate, HLINKGETREF_ABSOLUTE, &tgt, &loc);
        if (SUCCEEDED(hr) && tgt) {

            len = wcslen(tgt) + 4;
            if (loc)
                len += 4 + wcslen(loc);
            url = Dll_Alloc(len * sizeof(WCHAR));
            wcscpy(url, tgt);
            if (loc) {
                WCHAR *ptr = url + wcslen(url);
                *ptr = L'#';
                wcscpy(ptr + 1, loc);
            }
            IEServer_RestartProgram(url);
            return S_OK;
        }
    }

    return E_NOTIMPL;
}


//---------------------------------------------------------------------------
// IEServer_IServiceProvider_QueryService
//---------------------------------------------------------------------------


_FX HRESULT IEServer_IServiceProvider_QueryService(
    IHTMLWindow2 *This, REFGUID guidService, REFIID riid, void **ppvObject)
{
    ULONG_PTR *Vtbl = *(ULONG_PTR **)((*(ULONG_PTR **)This) - 1);

#ifdef COMSERVER_DEBUG
    WCHAR txt[128];
    swprintf(txt, L"IEServer_IServiceProvider_QueryService - %08X-%08X-%08X-%08X\n",
        ((ULONG *)riid)[0], ((ULONG *)riid)[1], ((ULONG *)riid)[2], ((ULONG *)riid)[3]);
    OutputDebugString(txt);
#endif

    if (! ppvObject)
        return E_POINTER;

    *ppvObject = NULL;

    if (IsEqualIID(riid, &IID_IHTMLWindow2))
        *ppvObject = (void *)&Vtbl[7];

    if (*ppvObject) {

        InterlockedIncrement(&ComServer_Object_RefCount);
        return S_OK;

    } else {

#ifdef COMSERVER_DEBUG
        OutputDebugString(L"IEServer_IServiceProvider_QueryService - E_NOINTERFACE\n");
#endif
        return E_NOINTERFACE;
    }
}


//---------------------------------------------------------------------------
// IEServer_IHTMLWindow2_NotImpl
//---------------------------------------------------------------------------


_FX HRESULT IEServer_IHTMLWindow2_NotImpl(IHTMLWindow2 *This)
{
#ifdef COMSERVER_DEBUG
    OutputDebugString(L"IHTMLWindow2::NotImpl\n");
#endif
    return E_NOTIMPL;
}


//---------------------------------------------------------------------------
// IEServer_RestartProgram
//---------------------------------------------------------------------------


_FX void IEServer_RestartProgram(const WCHAR *arg)
{
#ifdef COMSERVER_DEBUG
    OutputDebugString(L"IEServer Restart Commmand Line:\n");
    OutputDebugString(arg);
#endif

    ComServer_RestartProgram(IEServer_ResolveUrl((WCHAR *)arg));
}


//---------------------------------------------------------------------------
// IEServer_ResolveUrl
//---------------------------------------------------------------------------


_FX WCHAR *IEServer_ResolveUrl(WCHAR *path)
{
    const ULONG INTERNET_MAX_PATH_LENGTH = 2048;
    WCHAR *dot, *buf, *buf2, *name;
    ULONG len;

    dot = wcsrchr(path, L'.');
    if (! dot)
        return path;
    if (_wcsnicmp(dot, L".url", 4) != 0)
        return path;

    buf = (WCHAR *)Dll_Alloc(8192);
    buf2 = (WCHAR *)((UCHAR *)buf + 4096);

    wcscpy(buf, path);
    path = buf;
    while (iswspace(*path))
        ++path;
    if (*path == L'\"') {
        path[wcslen(path) - 1] = L'\0';
        ++path;
    }

    len = GetFullPathName(path, 2000, buf2, &name);
    if (! len)
        return path;
    len = GetPrivateProfileString(L"InternetShortcut", L"URL", L"",
                                  buf, INTERNET_MAX_PATH_LENGTH, buf2);
    if (! len)
        return path;

    return buf;
}
