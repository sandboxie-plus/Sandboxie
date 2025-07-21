/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2023 David Xanatos, xanasoft.com
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
// COM Services
//---------------------------------------------------------------------------


#include "dll.h"
#include "advapi.h"
//#include "iat.h"
#include "core/svc/ComWire.h"
#include "core/drv/api_flags.h"
#include <stdio.h>

#define COBJMACROS
#include <objbase.h>

#pragma auto_inline(off)


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define Com_RpcRaiseException(ExceptionCode)    \
    //RaiseException(ExceptionCode, EXCEPTION_NONCONTINUABLE, 0, NULL)

#define GETPROCADDR_DEF(name) \
    P_##name name = (P_##name) GetProcAddress(module, #name)

#define GETPROCADDR_SYS(name) \
    __sys_##name = (P_##name) GetProcAddress(module, #name)
extern ULONG Dll_Windows;
//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------


typedef struct _COM_IUNKNOWN {

    void *pVtbl;
    volatile ULONG RefCount;
    ULONG ObjIdx;
    ULONG Flags;
    GUID Guid;
    void *Vtbl[3];          // QueryInterface, AddRef, Release, ...

} COM_IUNKNOWN;


#define FLAG_REMOTE_REF         1
#define FLAG_PPROXY_AT_VTBL3    2


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

#ifndef _WIN64
BOOLEAN SbieDll_IsWow64();
#endif

static BOOLEAN Com_IsFirewallClsid(REFCLSID rclsid, const WCHAR *BoxName);

static BOOLEAN Com_IsClosedClsid(REFCLSID rclsid);

static HRESULT Com_CoGetClassObject(
    REFCLSID rclsid, ULONG clsctx, void *pServerInfo, REFIID riid,
    void **ppv);

static HRESULT Com_CoGetObject(
    const WCHAR *pszName, void *pBindOptions, REFIID riid, void **ppv);

static HRESULT Com_CoCreateInstance(
    REFCLSID rclsid, void *pUnkOuter, ULONG clsctx, REFIID riid, void **ppv);

static HRESULT Com_CoCreateInstanceEx(
    REFCLSID rclsid, void *pUnkOuter, ULONG clsctx, void *pServerInfo,
    ULONG cmq, MULTI_QI *pmqs);

#ifndef _M_ARM64
static BOOLEAN Com_Hook_CoUnmarshalInterface_W8(UCHAR *code, HMODULE module);

static HRESULT __fastcall Com_CoUnmarshalInterface_W8(
    ULONG_PTR StreamAddr, ULONG64 zero, REFIID riid, void **ppv);

static HRESULT __fastcall Com_CoUnmarshalInterface_W81(
    ULONG_PTR StreamAddr, ULONG zero, REFIID riid, void **ppv);
#endif

static HRESULT Com_CoUnmarshalInterface(
    IStream *pStream, REFIID riid, void **ppv);

static HRESULT Com_CoUnmarshalInterface_Common(
    IStream *pStream, REFIID riid, void **ppv, LARGE_INTEGER *seekpos);

static HRESULT Com_CoMarshalInterface(
    IStream *pStream, REFIID riid, IUnknown *pUnknown,
    DWORD dwDestContext, void *pvDestContext, DWORD mshlflags);

static HRESULT Com_CreateProxyStubFactoryBuffer(
    REFIID riid, IPSFactoryBuffer **pFactory, HMODULE *hProxyDll,
    ITypeInfo **pTypeInfo);

static HRESULT Com_CreateTypeInfo(REFIID riid, ITypeInfo **pTypeInfo);

static HRESULT Com_CreateProxy(REFIID riid, ULONG objidx, void **ppUnknown);

static HRESULT Com_IUnknown_New(
    ULONG objidx, ULONG methods, ULONG flags, COM_IUNKNOWN **ppv);

static HRESULT Com_IClassFactory_New(
    REFCLSID rclsid, const WCHAR *StringGUID, void **ppv);

static HRESULT Com_OuterIUnknown_New(ULONG objidx, REFIID iid, void **ppv);

static HRESULT Com_IRpcChannelBuffer_New(
    ULONG objidx, REFIID iid, void **ppv);

static HRESULT Com_IMarshal_New(ULONG objidx, void **ppv);

static HRESULT Com_IClientSecurity_New(ULONG objidx, REFIID iid, void **ppv);

static HRESULT Com_IRpcOptions_New(ULONG objidx, REFIID iid, void **ppv);

static void Com_IConnectionPoint_Hook(void *pUnknown);

static HRESULT Com_IConnectionPoint_Advise(
    void *pUnknown, void *pSink, ULONG *pCookie);

static HRESULT Com_IConnectionPoint_Unadvise(void *pUnknown, ULONG Cookie);

static void *Com_Alloc(ULONG len);

static void Com_Free(void *ptr);

static void Com_Trace(
    const WCHAR *TraceType, REFCLSID rclsid, REFIID riid,
    ULONG ProcNum, HRESULT hr);

static void Com_Trace2(
    const WCHAR* TraceType, const WCHAR* pszName, REFCLSID rclsid, REFIID riid,
    ULONG ProcNum, ULONG clsctx, HRESULT hr, ULONG monflag);

static void Com_Monitor(REFCLSID rclsid, ULONG monflag);

#define HSTRING void*
static HRESULT Com_RoGetActivationFactory(HSTRING activatableClassId, REFIID  iid, void** factory);

//static HRESULT Com_IClassFactoryEx_New(
//    REFCLSID rclsid, const WCHAR* StringGUID, void** ppv);

//---------------------------------------------------------------------------


typedef HRESULT (*P_CoGetClassObject)(
    REFCLSID rclsid, ULONG clsctx, void *pServerInfo, REFIID riid,
    void **ppv);

typedef ULONG (*P_CoGetObject)(
    const WCHAR *pszName, void *pBindOptions, REFIID riid, void **ppv);

typedef ULONG (*P_CoCreateInstance)(
    REFCLSID rclsid, void *pUnkOuter, ULONG dwClsContext, REFIID riid,
    void **ppv);

typedef ULONG (*P_CoCreateInstanceEx)(
    REFCLSID rclsid, void *pUnkOuter, ULONG dwClsContext,
    void *pServerInfo, ULONG cmq, MULTI_QI *pResults);

typedef ULONG (*P_CoUnmarshalInterface)(
    IStream *pStream, REFIID riid, void **ppv);

typedef ULONG (__fastcall *P_CoUnmarshalInterface_W8)(
    ULONG_PTR StreamAddr, ULONG64 zero, REFIID riid, void **ppv);

typedef ULONG (__fastcall *P_CoUnmarshalInterface_W81)(
    ULONG_PTR StreamAddr, ULONG zero, REFIID riid, void **ppv);


typedef ULONG (*P_CoMarshalInterface)(
    IStream *pStream, REFIID riid, IUnknown *pUnknown,
    DWORD dwDestContext, void *pvDestContext, DWORD mshlflags);

typedef ULONG (*P_CoGetPSClsid)(REFIID riid, CLSID *pclsid);

typedef ULONG (*P_StringFromGUID2)(REFGUID guid, void *lpsz, int cchMax);

typedef HRESULT (*P_IIDFromString)(LPCOLESTR lpsz, IID *lpiid);

typedef HRESULT (*P_LoadTypeLibEx)(
    LPCOLESTR szFile, REGKIND regkind, ITypeLib **pptlib);

typedef HRESULT (*P_CreateProxyFromTypeInfo)(
    ITypeInfo *pTypeInfo, IUnknown *pUnkOuter, REFIID riid,
    IRpcProxyBuffer **ppProxy, void **ppv);

typedef HRESULT (*P_CreateStubFromTypeInfo)(
    ITypeInfo *pTypeInfo, REFIID riid, IUnknown *pUnkServer,
    IRpcStubBuffer **ppStub);

typedef void *(*P_CoTaskMemAlloc)(ULONG cb);

typedef HRESULT (*P_RoGetActivationFactory)(HSTRING activatableClassId, REFIID  iid, void** factory);

typedef const wchar_t* (*P_WindowsGetStringRawBuffer)(HSTRING string, unsigned int* length);

//---------------------------------------------------------------------------


P_CoGetClassObject          __sys_CoGetClassObject          = NULL;
P_CoGetObject               __sys_CoGetObject               = NULL;
P_CoCreateInstance          __sys_CoCreateInstance          = NULL;
P_CoCreateInstanceEx        __sys_CoCreateInstanceEx        = NULL;
P_CoUnmarshalInterface      __sys_CoUnmarshalInterface      = NULL;
#ifndef _M_ARM64
P_CoUnmarshalInterface_W8   __sys_CoUnmarshalInterface_W8   = NULL;
P_CoUnmarshalInterface_W81  __sys_CoUnmarshalInterface_W81  = NULL;
#endif
P_CoMarshalInterface        __sys_CoMarshalInterface        = NULL;

P_CoGetPSClsid              __sys_CoGetPSClsid              = NULL;
P_StringFromGUID2           __sys_StringFromGUID2           = NULL;

P_LoadTypeLibEx             __sys_LoadTypeLibEx             = NULL;

P_CreateProxyFromTypeInfo   __sys_CreateProxyFromTypeInfo   = NULL;
P_CreateStubFromTypeInfo    __sys_CreateStubFromTypeInfo    = NULL;

P_CoTaskMemAlloc            __sys_CoTaskMemAlloc            = NULL;
P_IIDFromString             __sys_IIDFromString             = NULL;

P_RoGetActivationFactory    __sys_RoGetActivationFactory    = NULL;
P_WindowsGetStringRawBuffer __sys_WindowsGetStringRawBuffer = NULL;

//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------

BOOLEAN Ipc_OpenCOM = FALSE;

static ULONG Com_NumOpenClsids = -1;
static GUID *Com_OpenClsids = NULL;

static ULONG Com_NumClosedClsids = -1;
static GUID *Com_ClosedClsids = NULL;

static WCHAR* Com_ClosedRT = NULL;

static BOOLEAN Com_TraceFlag = FALSE;

static const WCHAR *Com_Mem_Trace = NULL;


EXTERN_C const IID GUID_NULL;
EXTERN_C const IID IID_IUnknown;
EXTERN_C const IID IID_IClassFactory;
EXTERN_C const IID IID_IMarshal;
EXTERN_C const IID IID_IPSFactoryBuffer;
EXTERN_C const IID IID_IRpcChannelBuffer;
EXTERN_C const IID IID_IConnectionPoint;
EXTERN_C const IID IID_IContextMenu;
EXTERN_C const CLSID CLSID_StdMarshal;


static const GUID IID_IStdIdentity = {
    0x0000001B, 0x0000, 0x0000,
        { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };

static const GUID IID_INetFwAuthorizedApplication = {
    0xB5E64FFA, 0xC2C5, 0x444E,
        { 0xA3, 0x01, 0xFB, 0x5E, 0x00, 0x01, 0x80, 0x50 } };

static const GUID IID_INetFwRule = {
    0xAF230D27, 0xBABA, 0x4E42,
        { 0xAC, 0xED, 0xF5, 0x24, 0xF2, 0x2C, 0xFC, 0xE2 } };

//static const GUID IID_VirtualDesktopManager = {
//    0xAA509086, 0x5CA9, 0x4C25,
//        { 0x8F, 0x95, 0x58, 0x9D, 0x3C, 0x07, 0xB4, 0x8A } };


//---------------------------------------------------------------------------
// Com_LoadClsidList
//---------------------------------------------------------------------------


_FX void Com_LoadClsidList(const WCHAR* setting, GUID** pClsids, ULONG* pNumClsids, const WCHAR* BoxName)
{
    //
    // for SbieSvc, re-create the list every time, in case
    // the configuration was reloaded
    //
    if (BoxName) {
        if (*pClsids) {

            Com_Free(*pClsids);
            *pClsids = NULL;
        }

        *pNumClsids = -1;
    }


    if (*pNumClsids != -1)
        return; // already done

    ULONG index;
    ULONG counter;
    GUID* guid;
    NTSTATUS status;
    WCHAR wbuf[196];
    WCHAR* buf = wbuf;
    const WCHAR* ImageName = NULL;
    if (BoxName == NULL) // sandboxed process not in service
        ImageName = Dll_ImageName;

    counter = 0;
    for (index = 0; ; ++index) {
        status = SbieApi_QueryConfAsIs(
            BoxName, setting, index, buf, 190 * sizeof(WCHAR));
        if (!NT_SUCCESS(status))
            break;

        WCHAR* ptr = wcschr(buf, L',');
        if (ptr) {
            *ptr = L'\0';
            if (ImageName && !SbieDll_MatchImage(buf, ImageName, BoxName))
                continue;
            buf = ptr + 1;
        }

        if (*buf != L'{') continue;

        ++counter;
    }

    if (counter) {

        *pClsids = Com_Alloc(sizeof(GUID) * counter);
        if (!*pClsids)
            counter = 0;
    }

    *pNumClsids = counter;

    if (counter) {

        for (index = 0; counter > 0; ++index) {

            status = SbieApi_QueryConfAsIs(
                BoxName, setting, index, buf, 190 * sizeof(WCHAR));
            if (!NT_SUCCESS(status))
                break;

            WCHAR* ptr = wcschr(buf, L',');
            if (ptr) {
                *ptr = L'\0';
                if (ImageName && !SbieDll_MatchImage(buf, ImageName, BoxName))
                    continue;
                buf = ptr + 1;
            }

            if (*buf != L'{') continue;

            --counter;

            WCHAR* space = wcschr(buf, L' ');
            if (space)
                *space = L'\0';

            guid = &(*pClsids)[index];

            if (!__sys_IIDFromString) {
                // if called from SbieSvc
                HMODULE module =
                    GetModuleHandle(DllName_ole32_or_combase);
                GETPROCADDR_SYS(IIDFromString);
            }

            if ((!__sys_IIDFromString) ||
                __sys_IIDFromString(buf, guid) != 0) {

                memzero(guid, sizeof(GUID));
            }
        }
    }
  
}


//---------------------------------------------------------------------------
// SbieDll_IsOpenClsid
//---------------------------------------------------------------------------


_FX BOOLEAN SbieDll_IsOpenClsid(
    REFCLSID rclsid, ULONG clsctx, const WCHAR *BoxName)
{
    static const GUID CLSID_WinMgmt = {
        0x8BC3F05E, 0xD86B, 0x11D0,
                        { 0xA0, 0x75, 0x00, 0xC0, 0x4F, 0xB6, 0x88, 0x20 } };

    static const GUID CLSID_NetworkListManager = {
        0xA47979D2, 0xC419, 0x11D9,
                        { 0xA5, 0xB4, 0x00, 0x11, 0x85, 0xAD, 0x2B, 0x89 } };

    static const GUID CLSID_ShellServiceHostBrokerProvider = {
        0x3480A401, 0xBDE9, 0x4407,
                        { 0xBC, 0x02, 0x79, 0x8A, 0x86, 0x6A, 0xC0, 0x51 } };

    //
    // open the null clsid to open all
    //

    static const GUID CLSID_Null = {
        0x00000000, 0x0000, 0x0000,
                        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };

    //if (clsctx & CLSCTX_LOCAL_SERVER) {

        ULONG index;
        GUID *guid;

        //
        // check against list of built-in CLSID exclusions
        //

        if (memcmp(rclsid, &CLSID_WinMgmt,              sizeof(GUID)) == 0 ||
            memcmp(rclsid, &CLSID_NetworkListManager,   sizeof(GUID)) == 0 ||
            memcmp(rclsid, &CLSID_ShellServiceHostBrokerProvider, sizeof(GUID)) == 0)
        {

            return TRUE;
        }

        //
        // initialize list of user-configured CLSID exclusions
        //

        static const WCHAR* setting = L"OpenClsid";
        Com_LoadClsidList(setting , &Com_OpenClsids, &Com_NumOpenClsids, BoxName);

        //
        // check against list of user-configured CLSID exclusions
        //

        for (index = 0; index < Com_NumOpenClsids; ++index) {
            guid = &Com_OpenClsids[index];
            if (memcmp(guid, rclsid, sizeof(GUID)) == 0 || memcmp(guid, &CLSID_Null, sizeof(GUID)) == 0)
                return TRUE;
        }
    //}

    if (Com_IsFirewallClsid(rclsid, BoxName))
        return TRUE;

    return FALSE;
}


//---------------------------------------------------------------------------
// Com_IsFirewallClsid
//---------------------------------------------------------------------------


_FX BOOLEAN Com_IsFirewallClsid(REFCLSID rclsid, const WCHAR *BoxName)
{
    // {0CA545C6-37AD-4A6C-BF92-9F7610067EF5} HNetCfg.FwOpenPort
    // {2C5BC43E-3369-4C33-AB0C-BE9469677AF4} HNetCfg.FwRule
    // {304CE942-6E39-40D8-943A-B913C40C9CD4} HNetCfg.FwMgr
    // {9D745ED8-C514-4D1D-BF42-751FED2D5AC7} HNetCfg.FwProduct
    // {CC19079B-8272-4D73-BB70-CDB533527B61} HNetCfg.FwProducts
    // {E2B3C97F-6AE1-41AC-817A-F6F92166D7DD} HNetCfg.FwPolicy2
    // {EC9846B3-2762-4A6B-A214-6ACB603462D2} HNetCfg.FwAuthorizedApplication

    static const GUID CLSID_Firewall[7] = {
        { 0x0CA545C6, 0x37AD, 0x4A6C,
                        { 0xBF, 0x92, 0x9F, 0x76, 0x10, 0x06, 0x7E, 0xF5 } },
        { 0x2C5BC43E, 0x3369, 0x4C33,
                        { 0xAB, 0x0C, 0xBE, 0x94, 0x69, 0x67, 0x7A, 0xF4 } },
        { 0x304CE942, 0x6E39, 0x40D8,
                        { 0x94, 0x3A, 0xB9, 0x13, 0xC4, 0x0C, 0x9C, 0xD4 } },
        { 0x9D745ED8, 0xC514, 0x4D1D,
                        { 0xBF, 0x42, 0x75, 0x1F, 0xED, 0x2D, 0x5A, 0xC7 } },
        { 0xCC19079B, 0x8272, 0x4D73,
                        { 0xBB, 0x70, 0xCD, 0xB5, 0x33, 0x52, 0x7B, 0x61 } },
        { 0xE2B3C97F, 0x6AE1, 0x41AC,
                        { 0x81, 0x7A, 0xF6, 0xF9, 0x21, 0x66, 0xD7, 0xDD } },
        { 0xEC9846B3, 0x2762, 0x4A6B,
                        { 0xA2, 0x14, 0x6A, 0xCB, 0x60, 0x34, 0x62, 0xD2 } }
    };
    ULONG i;

    for (i = 0; i < 7; ++i) {
        if (memcmp(rclsid, &CLSID_Firewall[i], sizeof(GUID)) == 0) {
            if (! File_IsBlockedNetParam(BoxName))
                return TRUE;
            /*if ((! BoxName) && (*(ULONG *)rclsid != 0xE2B3C97F))
                SbieApi_Log(1314, Dll_ImageName);*/
            break;
        }
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// Com_IsClosedClsid
//---------------------------------------------------------------------------


_FX BOOLEAN Com_IsClosedClsid(REFCLSID rclsid)
{
    static const UCHAR EventSystem[16] = {
        0xa2, 0xfb, 0x14, 0x4e, 0x22, 0x2e, 0xd1, 0x11,
        0x99, 0x64, 0x00, 0xc0, 0x4f, 0xbb, 0xb3, 0x45 };

    static const UCHAR EventSystemTier2[16] = {
        0x66, 0xf7, 0xe1, 0x1b, 0x36, 0x55, 0xd1, 0x11,
        0xb7, 0x26, 0x00, 0xc0, 0x4f, 0xb9, 0x26, 0xaf };

    // moved to templates.ini
    //static const UCHAR PinToStartScreen[16] = { // {470C0EBD-5D73-4D58-9CED-E91E22E23282} 
    //    0xbd, 0x0e, 0x0c, 0x47, 0x73, 0x5d, 0x58, 0x4d, 
    //    0x9c, 0xed, 0xe9, 0x1e, 0x22, 0xe2, 0x32, 0x82
    //};

    static const UCHAR* ClosedIDs[] = { EventSystem, EventSystemTier2/*, PinToStartScreen*/ };

    ULONG index;
    GUID* guid;

    for (index = 0; index < ARRAYSIZE(ClosedIDs); ++index) {
        if (memcmp(rclsid, ClosedIDs[index], 16) == 0)
            return TRUE;
    }

    //
    // initialize list of user-configured CLSID blocks
    // Note: the service threads everything not explicitly open as closed anyways
    //

    static const WCHAR* setting = L"ClosedClsid";
    Com_LoadClsidList(setting, &Com_ClosedClsids, &Com_NumClosedClsids, NULL);

    //
    // check against list of user-configured CLSID blocks
    //

    for (index = 0; index < Com_NumClosedClsids; ++index) {
        guid = &Com_ClosedClsids[index];
        if (memcmp(guid, rclsid, sizeof(GUID)) == 0)
            return TRUE;
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// Com_CoGetClassObject
//---------------------------------------------------------------------------


_FX HRESULT Com_CoGetClassObject(
    REFCLSID rclsid, ULONG clsctx, void *pServerInfo, REFIID riid,
    void **ppv)
{
    static const WCHAR *TraceType = L"GETCLS";
    HRESULT hr;
    ULONG monflag = 0;

    // debug tip. You can stop the debugger on a COM object load (instantiation) by uncommenting lines below.

    //static const GUID CLSID_BJPlugin = 
        //{ 0xEDFC7D0E, 0x7290, 0x50fa,
    //{ 0xaa, 0x13, 0x27, 0x17, 0x09, 0x38, 0x91, 0xc1 }};

    if (Com_IsClosedClsid(rclsid)) {
        *ppv = NULL;
        Com_Trace2(TraceType, NULL, rclsid, riid, 0, clsctx, 0, MONITOR_DENY);
        if (!Com_TraceFlag) Com_Monitor(rclsid, MONITOR_DENY);
        return E_ACCESSDENIED;
    }

    if (!Ipc_OpenCOM && (! pServerInfo) && SbieDll_IsOpenClsid(rclsid, clsctx, NULL)) {

        hr = Com_IClassFactory_New(rclsid, NULL, ppv);

        if (SUCCEEDED(hr))
            monflag |= MONITOR_OPEN;
        else
            monflag |= MONITOR_DENY;

    } else {
        //if (memcmp(rclsid, &CLSID_BJPlugin, sizeof(GUID)) == 0) {
            //while(!IsDebuggerPresent()) Sleep(500); __debugbreak();
        //}
        hr = __sys_CoGetClassObject(rclsid, clsctx, pServerInfo, riid, ppv);
    }

    Com_Trace2(TraceType, NULL, rclsid, riid, 0, clsctx, hr, monflag);
    if ((clsctx & CLSCTX_LOCAL_SERVER) != 0 || monflag) {
        if(!Com_TraceFlag) Com_Monitor(rclsid, monflag);
    }

    return hr;
}


//---------------------------------------------------------------------------
// Com_CoGetObject
//---------------------------------------------------------------------------


_FX HRESULT Com_CoGetObject(
    const WCHAR *pszName, void *pBindOptions, REFIID riid, void **ppv)
{
    static const WCHAR *TraceType = L"GETOBJ";
    GUID clsid;
    HRESULT hr;
    IClassFactory *pFactory;
    ULONG monflag = 0;
    BOOLEAN IsOpenClsid = FALSE;

    if (!Ipc_OpenCOM) {
        if (_wcsnicmp(pszName, L"Elevation:Administrator!new:", 28) == 0) {
            if (__sys_IIDFromString(pszName + 28, &clsid) == 0) {
                if (SbieDll_IsOpenClsid(&clsid, CLSCTX_LOCAL_SERVER, NULL))
                    IsOpenClsid = TRUE;
            }
        }
    }

    if (IsOpenClsid) {

        hr = Com_IClassFactory_New(&clsid, pszName + 28, (void **)&pFactory);

        if (SUCCEEDED(hr)) {

            hr = IClassFactory_CreateInstance(pFactory, NULL, riid, ppv);

            IClassFactory_Release(pFactory);
        }

        if (SUCCEEDED(hr))
            monflag |= MONITOR_OPEN;
        else
            monflag |= MONITOR_DENY;

    } else {

        hr = __sys_CoGetObject(pszName, pBindOptions, riid, ppv);
    }

    Com_Trace2(TraceType, pszName, NULL, riid, 0, 0, hr, monflag);
    if (!Com_TraceFlag) Com_Monitor(&clsid, monflag);

    return hr;
}


//---------------------------------------------------------------------------
// Com_CoCreateInstance
//---------------------------------------------------------------------------


_FX HRESULT Com_CoCreateInstance(
    REFCLSID rclsid, void *pUnkOuter, ULONG clsctx, REFIID riid, void **ppv)
{
    static const WCHAR *TraceType = L"CRE-IN";
    HRESULT hr;
    IClassFactory *pFactory;
    ULONG monflag = 0;

    if (Com_IsClosedClsid(rclsid)) {
        *ppv = NULL;
        Com_Trace2(TraceType, NULL, rclsid, riid, 0, clsctx, 0, MONITOR_DENY);
        if (!Com_TraceFlag) Com_Monitor(rclsid, MONITOR_DENY);
        return E_ACCESSDENIED;
    }

    /*if (memcmp(rclsid, &IID_VirtualDesktopManager, 16) == 0) {

        hr = Com_IClassFactoryEx_New(rclsid, NULL, (void **)&pFactory);

        if (SUCCEEDED(hr)) {

            hr = IClassFactory_CreateInstance(
                                        pFactory, pUnkOuter, riid, ppv);

            IClassFactory_Release(pFactory);
        }

    } else*/ if (!Ipc_OpenCOM && SbieDll_IsOpenClsid(rclsid, clsctx, NULL)) {

        hr = Com_IClassFactory_New(rclsid, NULL, (void **)&pFactory);

        if (SUCCEEDED(hr)) {

            hr = IClassFactory_CreateInstance(
                                        pFactory, pUnkOuter, riid, ppv);

            IClassFactory_Release(pFactory);
        }

        if (SUCCEEDED(hr))
            monflag |= MONITOR_OPEN;
        else
            monflag |= MONITOR_DENY;

    } else {

        hr = __sys_CoCreateInstance(rclsid, pUnkOuter, clsctx, riid, ppv);
    }

    Com_Trace2(TraceType, NULL, rclsid, riid, 0, clsctx, hr, monflag);
    if ((clsctx & CLSCTX_LOCAL_SERVER) != 0 || monflag) {
        if (!Com_TraceFlag) Com_Monitor(rclsid, monflag);
    }

    //
    // hook for IContextMenu used in Windows Explorer
    //

    if (SUCCEEDED(hr) && Dll_ImageType == DLL_IMAGE_SHELL_EXPLORER &&
            memcmp(riid, &IID_IContextMenu, sizeof(GUID)) == 0) {

        extern void SH32_IContextMenu_Hook(REFCLSID, void *);
        SH32_IContextMenu_Hook(rclsid, *ppv);
    }

    //
    // taskbar hooks
    //

    /*if (SUCCEEDED(hr) && (clsctx & CLSCTX_INPROC_SERVER))
        Taskbar_CoCreateInstance(riid, ppv);*/

    return hr;
}


//---------------------------------------------------------------------------
// Com_CoCreateInstanceEx
//---------------------------------------------------------------------------


_FX HRESULT Com_CoCreateInstanceEx(
    REFCLSID rclsid, void *pUnkOuter, ULONG clsctx, void *pServerInfo,
    ULONG cmq, MULTI_QI *pmqs)
{
    static const WCHAR *TraceType = L"CRE-EX";
    HRESULT hr;
    IClassFactory *pFactory;
    ULONG i;
    ULONG monflag = 0;

    //
    // special cases
    //

    if (Com_IsClosedClsid(rclsid)) {
        Com_Trace2(TraceType, NULL, rclsid, NULL, 0, clsctx, 0, MONITOR_DENY);
        if (!Com_TraceFlag) Com_Monitor(rclsid, MONITOR_DENY);
        return E_ACCESSDENIED;
    }

    if (Dll_ImageType == DLL_IMAGE_WINDOWS_LIVE_MAIL) {

        static const UCHAR BitsClsid[16] = {
            0x4b, 0xd3, 0x91, 0x49, 0xa1, 0x80, 0x91, 0x42,
            0x83, 0xb6, 0x33, 0x28, 0x36, 0x6b, 0x90, 0x97 };
        if (memcmp(rclsid, BitsClsid, 16) == 0) {

            return E_ACCESSDENIED;
        }
    }

    //
    // otherwise normal processing
    //

    if (!Ipc_OpenCOM && SbieDll_IsOpenClsid(rclsid, clsctx, NULL)) {

        hr = Com_IClassFactory_New(rclsid, NULL, (void **)&pFactory);
        if (SUCCEEDED(hr)) {

            ULONG good = 0;
            ULONG bad = 0;

            for (i = 0; i < cmq; ++i) {

                MULTI_QI *mqi = &pmqs[i];
                REFIID riid = mqi->pIID;
                void **ppv = &mqi->pItf;

                mqi->hr = IClassFactory_CreateInstance(
                                        pFactory, pUnkOuter, riid, ppv);
                if (FAILED(mqi->hr))
                    ++bad;
                else
                    ++good;
            }

            if (good == cmq)
                hr = S_OK;
            else if (bad == cmq)
                hr = E_NOINTERFACE;
            else
                hr = CO_S_NOTALLINTERFACES;

            IClassFactory_Release(pFactory);
        }

        if (SUCCEEDED(hr))
            monflag |= MONITOR_OPEN;
        else
            monflag |= MONITOR_DENY;

    } else {

        hr = __sys_CoCreateInstanceEx(
                            rclsid, pUnkOuter, clsctx, pServerInfo, cmq, pmqs);
    }

    
    for (i = 0; i < cmq; ++i) {
        MULTI_QI *mqi = &pmqs[i];
        Com_Trace2(TraceType, NULL, rclsid, mqi->pIID, 0, clsctx, mqi->hr, monflag);
        if ((clsctx & CLSCTX_LOCAL_SERVER) != 0 || monflag) {
            if (!Com_TraceFlag) Com_Monitor(rclsid, monflag);
        }
    }

    return hr;
}


//---------------------------------------------------------------------------
// Com_Hook_CoUnmarshalInterface_W8
//---------------------------------------------------------------------------

#ifndef _M_ARM64
_FX BOOLEAN Com_Hook_CoUnmarshalInterface_W8(UCHAR *code, HMODULE module)
{

    //
    // on Windows 8, parameter unmarshalling code in NdrpClientCall2 calls
    // an internal combase!_CoUnmarshalInterface directly rather than use
    // the public CoUnmarshalInterface.  and the public CoUnmarshalInterface
    // is a stub that calls _CoUnmarshalInterface.
    //
    // so we want to hook the internal _CoUnmarshalInterface rather than
    // the public stub version.  this function determines the address of
    // the internal function and hooks it.
    //
    // note that on 32-bit Windows, the internal _CoUnmarshalInterface
    // expects the first argument in ECX, rather than on the stack, so we
    // have to do some __fastcall magic.  see Com_CoUnmarshalInterface_W8
    //

#ifdef _WIN64

    if (Dll_OsBuild >= 15002) { // Windows 10 1703 preview

        if (code[0x18] == 0xe9) {

            LONG_PTR jump_target = (LONG_PTR)code + (*(LONG *)(code + 0x19)) + 0x1d;
            P_CoUnmarshalInterface_W8 CoUnmarshalInterface_W8 =
                (P_CoUnmarshalInterface_W8)jump_target;
            SBIEDLL_HOOK(Com_, CoUnmarshalInterface_W8);
            return TRUE;
        }
    }

    else if (Dll_OsBuild >= 14942) { // Windows 10 1703 preview #7

        if (code[0x20] == 0xe9) {
            LONG_PTR jump_target = (LONG_PTR)code + (*(LONG *)(code + 0x21)) + 0x25;
            P_CoUnmarshalInterface_W8 CoUnmarshalInterface_W8 =
                (P_CoUnmarshalInterface_W8)jump_target;
            SBIEDLL_HOOK(Com_, CoUnmarshalInterface_W8);
            return TRUE;
        }
    }

    if (*(USHORT *)(code + 6) == 0xD233     // xor edx, edx
            && code[8] == 0xE9) {           // jmp

        LONG_PTR jump_target = (LONG_PTR)code + (*(LONG *)(code + 9)) + 13;

        P_CoUnmarshalInterface_W8 CoUnmarshalInterface_W8 =
            (P_CoUnmarshalInterface_W8) jump_target;

        SBIEDLL_HOOK(Com_,CoUnmarshalInterface_W8);

        return TRUE;
    }

    if (*(USHORT *)(code + 6) == 0xD233     // xor edx, edx
            && code[8] == 0xEB) {           // jmp

        LONG_PTR jump_target = (LONG_PTR)code + (*(char *)(code + 9)) + 10;

        P_CoUnmarshalInterface_W8 CoUnmarshalInterface_W8 =
            (P_CoUnmarshalInterface_W8) jump_target;

        SBIEDLL_HOOK(Com_,CoUnmarshalInterface_W8);

        return TRUE;
    }
#else ! _WIN64

    if (*(USHORT *)(code + 14) == 0x006A    // push 0
            && code[16] == 0xE8) {          // call

        ULONG jump_target = (ULONG)code + (*(ULONG *)(code + 17)) + 21;

        P_CoUnmarshalInterface_W8 CoUnmarshalInterface_W8 =
            (P_CoUnmarshalInterface_W8) jump_target;

        SBIEDLL_HOOK(Com_,CoUnmarshalInterface_W8);

        return TRUE;
    }

    if (*(USHORT *)(code + 11) == 0xD232    // xor dl,dl
            && code[16] == 0xE8) {          // call

        ULONG jump_target = (ULONG)code + (*(ULONG *)(code + 17)) + 21;

        P_CoUnmarshalInterface_W81 CoUnmarshalInterface_W81 =
            (P_CoUnmarshalInterface_W81) jump_target;

        SBIEDLL_HOOK(Com_,CoUnmarshalInterface_W81);

        return TRUE;
    }

#endif _WIN64

    SbieApi_Log(2205, L"CoUnmarshalInterface (W8)");
    return FALSE;
}


//---------------------------------------------------------------------------
// Com_CoUnmarshalInterface_W8
//---------------------------------------------------------------------------


_FX HRESULT __fastcall Com_CoUnmarshalInterface_W8(
    ULONG_PTR StreamAddr, ULONG64 zero, REFIID riid, void **ppv)
{
    const HRESULT HR_OR_INVALID_OXID =
        MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,OR_INVALID_OXID);
    HRESULT hr;
    LARGE_INTEGER posl;
    ULARGE_INTEGER posu;
    ULONG_PTR riid_arg;
    ULONG_PTR ppv_arg;

    //
    // on 32-bit Windows 8, the internal _CoUnmarshalInterface function
    // expects the first argument (StreamAddr) in ECX rather than on the
    // stack, and adds a new unknown second parameter.  to avoid external
    // assembly wrappers, we use __fastcall, which tells the compiler
    // that the first two arguments pass in ECX and EDX.  but the caller
    // only passes the first parameter in ECX, and passes the second
    // parameter in the stack and not in EDX.  so we define the second
    // parameter as ULONG64 which tells the compiler that part of the
    // second parameter will still pass in the stack.
    //
    // on 64-bit Windows 8 the internal _CoUnmarshalInterface function
    // uses standard calling convention so we don't have to do any tricks.
    // the compiler silent ignores __fastcall on 64-bit compilation.
    //

    IStream *pStream = (IStream *)StreamAddr;

#ifdef _WIN64

    riid_arg = (ULONG_PTR)riid;
    ppv_arg = (ULONG_PTR)ppv;

#else ! _WIN64

    //
    // the optimizer gets confused and overwrites our arguments,
    // so save them here for the call to Com_CoUnmarshalInterface_Common
    //

    __asm {
        mov eax, [ebp+0x0C]     // riid
        mov riid_arg, eax
        mov eax, [ebp+0x10]     // ppv
        mov ppv_arg, eax
    }

#endif

    //
    // first invoke the COM unmarshaller.  it returns OR_INVALID_OXID
    // when the interface was marshalled in SbieSvc, because SbieSvc
    // uses a different epmapper than the sandboxed epmapper
    //

    posl.QuadPart = 0;
    hr = IStream_Seek(pStream, posl, STREAM_SEEK_CUR, &posu);
    if (FAILED(hr))
        return hr;

    hr = __sys_CoUnmarshalInterface_W8(StreamAddr, zero, riid, ppv);
    if (hr != HR_OR_INVALID_OXID)
        return hr;

    posl.QuadPart = posu.QuadPart;
    hr = IStream_Seek(pStream, posl, STREAM_SEEK_SET, &posu);
    if (FAILED(hr))
        return hr;

    return Com_CoUnmarshalInterface_Common(
                pStream, (REFIID)riid_arg, (void **)ppv_arg, &posl);
}


//---------------------------------------------------------------------------
// Com_CoUnmarshalInterface_W81
//---------------------------------------------------------------------------


_FX HRESULT __fastcall Com_CoUnmarshalInterface_W81(
    ULONG_PTR StreamAddr, ULONG zero, REFIID riid, void **ppv)
{
    const HRESULT HR_OR_INVALID_OXID =
        MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,OR_INVALID_OXID);
    HRESULT hr;
    LARGE_INTEGER posl;
    ULARGE_INTEGER posu;

    //
    // on 32-bit Windows 8.1, combase!_CoUnmarshalInterface is a true
    // fastcall function which gets its second argument in edx.  this is
    // unlike the Windows 8 version which gets the second argument on the
    // stack, which requires a small trick with ULONG64 (see above)
    //

    IStream *pStream = (IStream *)StreamAddr;

    //
    // first invoke the COM unmarshaller.  it returns OR_INVALID_OXID
    // when the interface was marshalled in SbieSvc, because SbieSvc
    // uses a different epmapper than the sandboxed epmapper
    //

    posl.QuadPart = 0;
    hr = IStream_Seek(pStream, posl, STREAM_SEEK_CUR, &posu);
    if (FAILED(hr))
        return hr;

    hr = __sys_CoUnmarshalInterface_W81(StreamAddr, zero, riid, ppv);
    if (hr != HR_OR_INVALID_OXID)
        return hr;

    posl.QuadPart = posu.QuadPart;
    hr = IStream_Seek(pStream, posl, STREAM_SEEK_SET, &posu);
    if (FAILED(hr))
        return hr;

    return Com_CoUnmarshalInterface_Common(pStream, riid, ppv, &posl);
}
#endif

//---------------------------------------------------------------------------
// Com_CoUnmarshalInterface
//---------------------------------------------------------------------------


_FX HRESULT Com_CoUnmarshalInterface(
    IStream *pStream, REFIID riid, void **ppv)
{
    const HRESULT HR_OR_INVALID_OXID =
        MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,OR_INVALID_OXID);
    HRESULT hr;
    LARGE_INTEGER posl;
    ULARGE_INTEGER posu;

    //
    // first invoke the COM unmarshaller.  it returns OR_INVALID_OXID
    // when the interface was marshalled in SbieSvc, because SbieSvc
    // uses a different epmapper than the sandboxed epmapper
    //

    posl.QuadPart = 0;
    hr = IStream_Seek(pStream, posl, STREAM_SEEK_CUR, &posu);
    if (FAILED(hr))
        return hr;

    hr = __sys_CoUnmarshalInterface(pStream, riid, ppv);
    if (hr != HR_OR_INVALID_OXID)
        return hr;

    posl.QuadPart = posu.QuadPart;
    hr = IStream_Seek(pStream, posl, STREAM_SEEK_SET, &posu);
    if (FAILED(hr))
        return hr;

    return Com_CoUnmarshalInterface_Common(pStream, riid, ppv, &posl);
}


//---------------------------------------------------------------------------
// Com_CoUnmarshalInterface_Common
//---------------------------------------------------------------------------


_FX HRESULT Com_CoUnmarshalInterface_Common(
    IStream *pStream, REFIID riid, void **ppv, LARGE_INTEGER *seekpos)
{
    static const WCHAR *TraceType = L"UNMRSH";
    const HRESULT HR_OR_INVALID_OXID =
        MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,OR_INVALID_OXID);
    HRESULT hr;
    ULARGE_INTEGER posu;
    COM_UNMARSHAL_INTERFACE_REQ *req;
    COM_UNMARSHAL_INTERFACE_RPL *rpl;
    GUID iid;
    ULONG len;

    //
    // allocate a large request buffer to hold a potentially
    // large unmarshalling buffer, up to some maximum bytes
    //

    req = (COM_UNMARSHAL_INTERFACE_REQ *)
                    Com_Alloc(128 + COM_MAX_UNMARSHAL_BUF_LEN);
    if (! req)
        return E_OUTOFMEMORY;

    //
    // read the entire stream into the request
    // the stream should begin with 0x18 bytes:
    // ULONG 'MEOW' ; ULONG type ; GUID iid
    //

    hr = IStream_Read(pStream, req->Buffer, COM_MAX_UNMARSHAL_BUF_LEN, &len);
    if (hr == S_FALSE)
        hr = S_OK;
    if (SUCCEEDED(hr) && len < 0x18)
        hr = RPC_E_INVALID_OBJREF;
    if (FAILED(hr)) {
        Com_Free(req);
        return hr;
    }

    //
    // construct a request to unmarshal the interface in SbieSvc,
    // where the real epmapper is accessible
    //

    req->h.length = sizeof(COM_UNMARSHAL_INTERFACE_REQ) + len;
    req->h.msgid = MSGID_COM_UNMARSHAL_INTERFACE;

    memcpy(&iid, req->Buffer + 8, sizeof(GUID));
    memcpy(&req->iid, &iid, sizeof(GUID));

    req->BufferLength = len;

    //
    // execute request and build our specialized proxy which
    // can invoke the remote interface unmarshalled in SbieSvc
    //

    rpl = (COM_UNMARSHAL_INTERFACE_RPL *)SbieDll_CallServer(&req->h);

    Com_Free(req);

    if (rpl) {
        hr = rpl->h.status;
        if (hr != S_OK)
            Com_Free(rpl);
    } else
        hr = RPC_S_SERVER_UNAVAILABLE;

    if (hr != S_OK) {
        Com_Trace(TraceType, NULL, riid, 0, hr);
        if (hr == RPC_S_SERVER_UNAVAILABLE) {
            IStream_Seek(pStream, *seekpos, STREAM_SEEK_SET, &posu);
            return HR_OR_INVALID_OXID;
        } else {
            Com_RpcRaiseException(hr);
            return E_ABORT;
        }
    }

    hr = rpl->hr;
    if (SUCCEEDED(hr) && (! rpl->objidx))
        hr = E_FAIL;
    if (SUCCEEDED(hr)) {
        if (memcmp(riid, &GUID_NULL, sizeof(GUID)) != 0)
            memcpy(&iid, riid, sizeof(GUID));
        hr = Com_CreateProxy(&iid, rpl->objidx, ppv);
    }

    Com_Trace(TraceType, NULL, &iid, 0, hr);

    Com_Free(rpl);
    return hr;
}


//---------------------------------------------------------------------------
// Com_MarshalInterface
//---------------------------------------------------------------------------


static HRESULT Com_CoMarshalInterface(
    IStream *pStream, REFIID riid, IUnknown *pUnknown,
    DWORD dwDestContext, void *pvDestContext, DWORD mshlflags)
{
    static const GUID IID_ISearchNotifyInlineSite =
        { 0xB5702E61, 0xE75C, 0x4B64,
                        { 0x82, 0xA1, 0x6C, 0xB4, 0xF8, 0x32, 0xFC, 0xCF } };
    HRESULT hr;

    //
    // special handling for ISearchNotifyInlineSite objects,
    // normal handling otherwise
    //

    if (dwDestContext == MSHCTX_INPROC ||
            memcmp(riid, &IID_ISearchNotifyInlineSite, sizeof(GUID)) != 0) {

        hr = __sys_CoMarshalInterface(
                    pStream, riid, pUnknown,
                    dwDestContext, pvDestContext, mshlflags);

    } else {

        //
        // create a dummy object in SbieSvc, marshal it in SbieSvc,
        // and write the marshal buffer into the stream
        //

        static const WCHAR *TraceType = L"MRSH-X";

        COM_MARSHAL_INTERFACE_REQ req;
        COM_MARSHAL_INTERFACE_RPL *rpl;

        req.h.length = sizeof(COM_MARSHAL_INTERFACE_REQ);
        req.h.msgid = MSGID_COM_MARSHAL_INTERFACE;
        req.objidx = -1;
        memcpy(&req.iid, riid, sizeof(GUID));
        req.destctx = dwDestContext;
        req.mshlflags = mshlflags;

        rpl = (COM_MARSHAL_INTERFACE_RPL *)SbieDll_CallServer(&req.h);

        if (rpl) {
            hr = rpl->h.status;
            if (hr != S_OK)
                Com_Free(rpl);
        } else
            hr = RPC_S_SERVER_UNAVAILABLE;
        if (hr != S_OK) {
            Com_Trace(TraceType, NULL, riid, 0, hr);
            Com_RpcRaiseException(hr);
            return E_ABORT;
        }

        hr = rpl->hr;
        if (SUCCEEDED(hr)) {

            ULONG count = 0;
            hr = IStream_Write(
                    pStream, rpl->Buffer, rpl->BufferLength, &count);
        }

        Com_Trace(TraceType, NULL, riid, 0, hr);

        Com_Free(rpl);
    }

    //
    // finish
    //

    return hr;
}


//---------------------------------------------------------------------------
// Com_Init_ComBase
//---------------------------------------------------------------------------


_FX BOOLEAN Com_Init_ComBase(HMODULE module)
{
    //
    // on Windows 8, these core COM functions are hooked when combase.dll
    // is loaded, invoked by Ldr_LoadDll.  in earlier versions of Windows,
    // this function is invoked by Com_Init_Ole32
    //

    GETPROCADDR_DEF(CoGetClassObject);
    GETPROCADDR_DEF(CoCreateInstance);
    GETPROCADDR_DEF(CoCreateInstanceEx);
    GETPROCADDR_DEF(CoUnmarshalInterface);
    GETPROCADDR_DEF(CoMarshalInterface);

    GETPROCADDR_SYS(CoTaskMemAlloc);
    GETPROCADDR_SYS(IIDFromString);

    if (SbieDll_IsOpenCOM()
    // DisableComProxy BEGIN
    || SbieApi_QueryConfBool(NULL, L"DisableComProxy", FALSE))
    // DisableComProxy END
        Ipc_OpenCOM = TRUE;
    
    
    SBIEDLL_HOOK(Com_, CoGetClassObject);
    if (!Dll_SkipHook(L"cocreate")) {
        SBIEDLL_HOOK(Com_, CoCreateInstance);
        SBIEDLL_HOOK(Com_, CoCreateInstanceEx);
    }

    if (!Ipc_OpenCOM) {
        if (Dll_OsBuild >= 8400) {
            CoUnmarshalInterface = (P_CoUnmarshalInterface)GetProcAddress(GetModuleHandle(L"combase.dll"), "CoUnmarshalInterface");
#ifndef _M_ARM64
            // $HookHack$ - Custom, not automated, Hook for windows 8 and 8.1 and early windows 10 builds
            if (!CoUnmarshalInterface) {
                if (!Com_Hook_CoUnmarshalInterface_W8((UCHAR*)CoUnmarshalInterface, module))
                    return FALSE;
            }
#endif
        }
        SBIEDLL_HOOK(Com_, CoUnmarshalInterface);

        SBIEDLL_HOOK(Com_, CoMarshalInterface);
        SbieDll_IsOpenClsid(&IID_IUnknown, CLSCTX_LOCAL_SERVER, NULL); // trigger list loading
    }

    if (Dll_OsBuild >= 8400) { // win8 and above
        __sys_WindowsGetStringRawBuffer = (P_WindowsGetStringRawBuffer)GetProcAddress(module, "WindowsGetStringRawBuffer");
        P_RoGetActivationFactory RoGetActivationFactory = (P_RoGetActivationFactory)GetProcAddress(module, "RoGetActivationFactory");
        if (RoGetActivationFactory) {
            SBIEDLL_HOOK(Com_, RoGetActivationFactory);
        }
    }

    // If there are any ClsidTrace options set, then output this debug string
    WCHAR wsTraceOptions[4];
    if (SbieApi_QueryConf(NULL, L"ClsidTrace", 0, wsTraceOptions, sizeof(wsTraceOptions)) == STATUS_SUCCESS && wsTraceOptions[0] != L'\0')
        Com_TraceFlag = TRUE;

    if (!__sys_RegOpenKeyExW) {
        HMODULE module = Dll_KernelBase ? Dll_KernelBase : LoadLibrary(DllName_advapi32);
        __sys_RegOpenKeyExW = (P_RegOpenKeyEx)GetProcAddress(module, "RegOpenKeyExW");
        GETPROCADDR_SYS(RegCloseKey);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Com_Init_Ole32
//---------------------------------------------------------------------------


_FX BOOLEAN Com_Init_Ole32(HMODULE module)
{
    GETPROCADDR_DEF(CoGetObject);

    if (Dll_OsBuild < 8400) {

        //
        // on Windows 8, core COM functions are in combase.dll which is
        // initialized separately.  on earlier versions of Windows, the
        // core COM functions are part of ole32.dll
        //

        if (! Com_Init_ComBase(module))
            return FALSE;
    }

    //
    // other functions are still in ole32, even on Windows 8
    //

    SBIEDLL_HOOK(Com_,CoGetObject);

    return TRUE;
}


//---------------------------------------------------------------------------
// Com_CreateProxyStubFactoryBuffer
//---------------------------------------------------------------------------


_FX HRESULT Com_CreateProxyStubFactoryBuffer(
    REFIID riid, IPSFactoryBuffer **pFactory, HMODULE *hProxyDll,
    ITypeInfo **pTypeInfo)
{
    static const GUID IID_PSOAInterface =
        { 0x00020424, 0x0000, 0x0000,
                        { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
    HRESULT hr;
    LONG rc;
    CLSID ProxyClsid;
    ULONG buf_len;
    WCHAR *path;
    HKEY hkey;
    LPFNGETCLASSOBJECT DllGetClassObject;

    //
    // initialize function pointers
    //

    if (! __sys_CoGetPSClsid) {

        HMODULE module = GetModuleHandle(DllName_ole32_or_combase);
        GETPROCADDR_SYS(CoGetPSClsid);
        GETPROCADDR_SYS(StringFromGUID2);
    }

    if (! __sys_RegOpenKeyExW) {

        HMODULE module = LoadLibrary(DllName_advapi32);
        __sys_RegOpenKeyExW = (P_RegOpenKeyEx)
            GetProcAddress(module, "RegOpenKeyExW");
        GETPROCADDR_SYS(RegCloseKey);
    }

    if (    (! __sys_CoGetPSClsid)
        ||  (! __sys_StringFromGUID2)
        ||  (! __sys_RegOpenKeyExW)
        ||  (! __sys_RegCloseKey)) {

        SbieApi_Log(2205, L"CreateProxyStubFactoryBuffer");
        return E_FAIL;
    }

    //
    // create a client-side in-proc handler proxy for an interface.
    // we do this the same way CoUnmarshalInterface creates a proxy
    // interface:  by loading the proxy DLL and invoking its
    // IPSFactoryBuffer::CreateProxy method.
    //

    *pFactory = NULL;
    *pTypeInfo = NULL;
    *hProxyDll = NULL;

    //
    // step 1, the Classes\Interface\<IID>\ProxyStubClsid32 key
    // specifies the CLSID of the proxy DLL implementing the
    // desired interface
    //

    if (memcmp(riid, &IID_IUnknown, sizeof(GUID)) == 0) {

        static const GUID CLSID_PSFactoryBuffer =
            { 0x00000320, 0x0000, 0x0000,
                            { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };

        memcpy(&ProxyClsid, &CLSID_PSFactoryBuffer, sizeof(GUID));
        hr = S_OK;

    } else

        hr = __sys_CoGetPSClsid(riid, &ProxyClsid);

    if (FAILED(hr))
        return hr;

    if (memcmp(&ProxyClsid, &IID_PSOAInterface, sizeof(GUID)) == 0) {

        //
        // oleautomation interfaces are associated with a TypeLib
        // rather than a DLL, we need to extract a TypeInfo which
        // can be used to generate a proxy or stub
        //
        // IID_PSOAInterface is the standard marshaller for a TypeLib
        //

        return Com_CreateTypeInfo(riid, pTypeInfo);
    }

    //
    // step 2, the Classes\CLSID\<CLSID>\InProcServer32 key specifies
    // the path to the proxy DLL
    //

    buf_len = MAX_PATH + 64;
    path = Com_Alloc(buf_len);
    if (! path)
        return E_OUTOFMEMORY;

    wcscpy(path, L"CLSID\\");
    __sys_StringFromGUID2(&ProxyClsid, path + wcslen(path), 48);
    wcscat(path, L"\\InProcServer32");

    rc = __sys_RegOpenKeyExW(HKEY_CLASSES_ROOT, path, 0, KEY_READ, &hkey);
    if (rc == 0) {

        ULONG len;
        UNICODE_STRING ValueName;
        KEY_VALUE_PARTIAL_INFORMATION *info =
            (KEY_VALUE_PARTIAL_INFORMATION *)path;

        RtlInitUnicodeString(&ValueName, L"");

        rc = NtQueryValueKey(
                hkey, &ValueName, KeyValuePartialInformation,
                info, buf_len - 8, &len);

        if (rc == 0) {

            len = info->DataLength / sizeof(WCHAR);
            wmemmove(path, (WCHAR*)info->Data, len);
            path[len] = L'\0';
        }

        __sys_RegCloseKey(hkey);
    }

    //
    // step 3, load the proxy DLL
    //

    if (rc != 0) {

        *hProxyDll = NULL;
        hr = REGDB_E_CLASSNOTREG;

    } else {

        WCHAR *path2 = Dll_AllocTemp(MAX_PATH * 2 * sizeof(WCHAR));
        wmemzero(path2, MAX_PATH * 2);
        ExpandEnvironmentStrings(path, path2, MAX_PATH * 2 - 8);

        *hProxyDll = LoadLibraryW(path2);
        if (! *hProxyDll)
            hr = CO_E_DLLNOTFOUND;

        Dll_Free(path2);
    }

    Com_Free(path);

    if (FAILED(hr))
        return hr;

    //
    // step 4, invoke DllGetClassObject to get IPSFactoryBuffer
    //

    DllGetClassObject = (LPFNGETCLASSOBJECT)
        GetProcAddress(*hProxyDll, "DllGetClassObject");

    if (! DllGetClassObject)
        hr = CO_E_ERRORINDLL;
    else
        hr = DllGetClassObject(&ProxyClsid, &IID_IPSFactoryBuffer, pFactory);

    if (FAILED(hr)) {
        FreeLibrary(*hProxyDll);
        *hProxyDll = NULL;
    }

    return hr;
}


//---------------------------------------------------------------------------
// Com_CreateTypeInfo
//---------------------------------------------------------------------------


_FX HRESULT Com_CreateTypeInfo(REFIID riid, ITypeInfo **pTypeInfo)
{
    static const WCHAR *_TypeLib = L"TypeLib";
    HRESULT hr;
    LONG rc;
    ULONG buf_len;
    WCHAR *path;
    HKEY hkey;
    ULONG len;
    UNICODE_STRING ValueName;
    KEY_VALUE_PARTIAL_INFORMATION *info;

    //
    // initialize function pointers
    //

    if (! __sys_LoadTypeLibEx) {

        HMODULE module = LoadLibrary(L"oleaut32.dll");
        GETPROCADDR_SYS(LoadTypeLibEx);
    }

    if (! __sys_CreateProxyFromTypeInfo) {

        HMODULE module = GetModuleHandle(L"rpcrt4.dll");
        GETPROCADDR_SYS(CreateProxyFromTypeInfo);
        GETPROCADDR_SYS(CreateStubFromTypeInfo);
    }

    if (    (! __sys_LoadTypeLibEx)
        ||  (! __sys_CreateProxyFromTypeInfo)
        ||  (! __sys_CreateStubFromTypeInfo)) {

        SbieApi_Log(2205, L"CreateTypeInfo");
        return E_FAIL;
    }

    //
    // step 2, the Interface\IID\TypeLib key specifies the GUID and
    // version for the associated type library, use these to open
    // the registry key in the TypeLib subtree and extract the path
    //

    buf_len = MAX_PATH + 64;
    path = Com_Alloc(buf_len);
    if (! path)
        return E_OUTOFMEMORY;

    wcscpy(path, L"Interface\\");
    __sys_StringFromGUID2(riid, path + wcslen(path), 48);
    wcscat(path, L"\\");
    wcscat(path, _TypeLib);

    rc = __sys_RegOpenKeyExW(HKEY_CLASSES_ROOT, path, 0, KEY_READ, &hkey);

#ifndef _WIN64

    // 32-bit app can read x64 registry related to interface and typelib.
    if (rc == ERROR_FILE_NOT_FOUND && SbieDll_IsWow64())
    {
        rc = __sys_RegOpenKeyExW(HKEY_CLASSES_ROOT, path, 0, KEY_READ | KEY_WOW64_64KEY, &hkey);
    }

#endif

    if (rc == 0) {

        RtlInitUnicodeString(&ValueName, L"");

        info = (KEY_VALUE_PARTIAL_INFORMATION *)path;

        rc = NtQueryValueKey(
                hkey, &ValueName, KeyValuePartialInformation,
                info, buf_len - 8, &len);

        if (rc == 0) {

            WCHAR *ptr;
            ULONG info_space[16];

            //
            // copy the TypeLib GUID over the path
            //

            len = info->DataLength / sizeof(WCHAR);
            ptr = path + wcslen(_TypeLib) + 1;
            wmemmove(ptr, (WCHAR*)info->Data, len);
            wcscpy(path, _TypeLib);
            ptr[-1] = L'\\';
            ptr[38] = L'\\';
            ptr += 39;

            //
            // query and append the Version value to the path,
            // followed by a final suffix \0\win32
            //

            RtlInitUnicodeString(&ValueName, L"Version");

            info = (KEY_VALUE_PARTIAL_INFORMATION *)info_space;

            rc = NtQueryValueKey(
                    hkey, &ValueName, KeyValuePartialInformation,
                    info, sizeof(info_space), &len);

            if (rc == 0) {

                len = info->DataLength / sizeof(WCHAR);
                wmemcpy(ptr, (WCHAR*)info->Data, len);
                while (len && ptr[len - 1] == L'\0')
                    --len;
                wcscpy(&ptr[len], L"\\0\\win32");
            }
        }

        __sys_RegCloseKey(hkey);
    }

    //
    // now that we have a a path like TypeLib\GUID\Version\0\win32,
    // find the path to the type library file
    //

    if (rc == 0)
    {
        rc = __sys_RegOpenKeyExW(HKEY_CLASSES_ROOT, path, 0, KEY_READ, &hkey);

#ifndef _WIN64

        // 32-bit app can read x64 registry related to interface and typelib.
        if (rc == ERROR_FILE_NOT_FOUND && SbieDll_IsWow64())
        {
            rc = __sys_RegOpenKeyExW(HKEY_CLASSES_ROOT, path, 0, KEY_READ | KEY_WOW64_64KEY, &hkey);

            if (rc == ERROR_FILE_NOT_FOUND)
            {
                // TypeLib\GUID\Version\0\win64. Change win32 to win64
                size_t len = wcslen(path);
                if (len > 3)
                {
                    path[len - 2] = '6';
                    path[len - 1] = '4';
                }

                rc = __sys_RegOpenKeyExW(HKEY_CLASSES_ROOT, path, 0, KEY_READ | KEY_WOW64_64KEY, &hkey);
            }
        }

#endif
    }

    if (rc == 0) {

        RtlInitUnicodeString(&ValueName, L"");

        info = (KEY_VALUE_PARTIAL_INFORMATION *)path;

        rc = NtQueryValueKey(
                hkey, &ValueName, KeyValuePartialInformation,
                info, buf_len - 8, &len);

        if (rc == 0) {

            len = info->DataLength / sizeof(WCHAR);
            wmemmove(path, (WCHAR*)info->Data, len);
            path[len] = L'\0';
        }

        __sys_RegCloseKey(hkey);
    }

    //
    // step 3, load the type library
    //

    if (rc != 0) {

        hr = COMADMIN_E_BADREGISTRYLIBID;

    } else {

        ITypeLib *pTypeLib;

        WCHAR *path2 = Dll_AllocTemp(MAX_PATH * 2 * sizeof(WCHAR));
        wmemzero(path2, MAX_PATH * 2);
        ExpandEnvironmentStrings(path, path2, MAX_PATH * 2 - 8);

        hr = __sys_LoadTypeLibEx(path2, REGKIND_NONE, &pTypeLib);

        Dll_Free(path2);

        if (SUCCEEDED(hr)) {

            hr = ITypeLib_GetTypeInfoOfGuid(pTypeLib, riid, pTypeInfo);

            ITypeLib_Release(pTypeLib);
        }
    }

    Com_Free(path);

    return hr;
}


//---------------------------------------------------------------------------
// SbieDll_ComCreateProxy
//---------------------------------------------------------------------------


_FX HRESULT SbieDll_ComCreateProxy(
    REFIID riid, void *pUnkOuter, void *pChannel, void **ppUnknown)
{
    HRESULT hr;
    IPSFactoryBuffer *pFactory;
    ITypeInfo *pTypeInfo;
    HMODULE hProxyDll;

    //
    // create a standard proxy for an interface that has an associated
    // DLL or typelib.  this means it isn't possible to create such
    // a proxy for the IUnknown interface itself
    //

    hr = Com_CreateProxyStubFactoryBuffer(
                            riid, &pFactory, &hProxyDll, &pTypeInfo);
    if (SUCCEEDED(hr)) {

        //
        // step 4, invoke IPSFactoryBuffer::CreateProxy
        //     or, invoke CreateProxyFromTypeInfo for PSOAInterface's
        //
        // we might get a zero pUnknown from IPSFactoryBuffer::CreateProxy,
        // see also step 5 (this happens for the IDispatch interface)
        //

        IUnknown *pUnknown;
        IRpcProxyBuffer *pProxy;

        if (pTypeInfo) {

            hr = __sys_CreateProxyFromTypeInfo(
                        pTypeInfo, pUnkOuter, riid, &pProxy, &pUnknown);

        } else {

            hr = IPSFactoryBuffer_CreateProxy(
                        pFactory, pUnkOuter, riid, &pProxy, &pUnknown);
        }

        if (SUCCEEDED(hr)) {

            //
            // step 5, connect proxy to our channel.
            //
            // if we got a zero pUnknown from IPSFactoryBuffer::CreateProxy
            // it means the final proxy interface will be returned after
            // the proxy is connected to the channel, using QueryInterface.
            //

            hr = IRpcProxyBuffer_Connect(pProxy, pChannel);

            if (SUCCEEDED(hr) && (! pUnknown)) {

                hr = IRpcProxyBuffer_QueryInterface(pProxy, riid, &pUnknown);
            }

            if (SUCCEEDED(hr)) {

                ((COM_IUNKNOWN *)pUnkOuter)->Vtbl[3] = pProxy;
                ((COM_IUNKNOWN *)pUnkOuter)->Vtbl[4] = pUnknown;
                *ppUnknown = pUnknown;

                if (! pUnknown)
                    SbieApi_Log(2205, L"Zero Proxy");

            } else {

                IRpcProxyBuffer_Release(pProxy);
                if (pUnknown)
                    IUnknown_Release(pUnknown);
            }
        }

        if (FAILED(hr) && hProxyDll)
            FreeLibrary(hProxyDll);
    }

    if (pFactory)
        IPSFactoryBuffer_Release(pFactory);
    if (pTypeInfo)
        ITypeInfo_Release(pTypeInfo);

    if (FAILED(hr))
        *ppUnknown = NULL;

    return hr;
}


//---------------------------------------------------------------------------
// Com_CreateProxy
//---------------------------------------------------------------------------


_FX HRESULT Com_CreateProxy(
    REFIID riid, ULONG objidx, void **ppUnknown)
{
    HRESULT hr;
    IUnknown *pUnkOuter;

    hr = Com_OuterIUnknown_New(objidx, riid, &pUnkOuter);

    if (SUCCEEDED(hr)) {

        //
        // special case for a proxy for IUnknown
        //

        if (memcmp(riid, &IID_IUnknown, sizeof(GUID)) == 0) {

            ((COM_IUNKNOWN *)pUnkOuter)->Vtbl[4] = pUnkOuter;

            *ppUnknown = pUnkOuter;
            hr = S_OK;

        } else {

            IRpcChannelBuffer *pChannel;

            hr = Com_IRpcChannelBuffer_New(objidx, riid, &pChannel);

            if (SUCCEEDED(hr)) {

                hr = SbieDll_ComCreateProxy(
                            riid, pUnkOuter, pChannel, ppUnknown);

                if (pChannel)
                    IRpcChannelBuffer_Release(pChannel);

                //
                // special cases if this is IConnectionPoint or INetFwXxx
                //

                if (SUCCEEDED(hr)) {

                    extern Net_Firewall_Hook(ULONG type, void *pUnknown);

                    if (memcmp(riid, &IID_IConnectionPoint,
                               sizeof(GUID)) == 0) {

                        Com_IConnectionPoint_Hook(*ppUnknown);

                    } else if (memcmp(riid, &IID_INetFwAuthorizedApplication,
                                      sizeof(GUID)) == 0) {

                        Net_Firewall_Hook(1, *ppUnknown);

                    } else if (memcmp(riid, &IID_INetFwRule,
                                      sizeof(GUID)) == 0) {

                        Net_Firewall_Hook(2, *ppUnknown);
                    }
                }
            }

            IUnknown_Release(pUnkOuter);
        }
    }

    return hr;
}


//---------------------------------------------------------------------------
// SbieDll_ComCreateStub
//---------------------------------------------------------------------------


_FX HRESULT SbieDll_ComCreateStub(
    REFIID riid, void *pUnknown, void **ppStub, void **ppChannel)
{
    HRESULT hr;
    IPSFactoryBuffer *pFactory;
    ITypeInfo *pTypeInfo;
    HMODULE hProxyDll;

    //
    // special case for a stub for IUnknown
    //

    if (memcmp(riid, &IID_IUnknown, sizeof(GUID)) == 0) {

        //
        // our proxies for IUnknown do not use the channel marshalling
        // architecture, instead issuing alternate calls, thus there
        // is no need for an IRpcStubBuffer stubs for these proxies
        //

        *ppStub = NULL;
        *ppChannel = NULL;

        return S_OK;
    }

    //
    // otherwise create a standard stub
    //

    hr = Com_CreateProxyStubFactoryBuffer(
                            riid, &pFactory, &hProxyDll, &pTypeInfo);
    if (SUCCEEDED(hr)) {

        //
        // step 5, invoke IPSFactoryBuffer::CreateStub
        //     or, invoke CreateStubFromTypeInfo for PSOAInterface's
        //

        IRpcStubBuffer *pStub;

        if (pTypeInfo) {

            hr = __sys_CreateStubFromTypeInfo(pTypeInfo, riid, NULL, &pStub);

        } else {

            hr = IPSFactoryBuffer_CreateStub(pFactory, riid, NULL, &pStub);
        }

        if (SUCCEEDED(hr)) {

            //
            // step 6, create channel and connect stub to interface
            //

            IRpcChannelBuffer *pChannel;
            hr = Com_IRpcChannelBuffer_New(0, riid, &pChannel);

            if (SUCCEEDED(hr)) {

                hr = IRpcStubBuffer_Connect(pStub, (IUnknown *)pUnknown);

                if (SUCCEEDED(hr)) {

                    *ppStub = pStub;
                    *ppChannel = pChannel;

                    if (! pStub)
                        SbieApi_Log(2205, L"Zero Stub");

                } else
                    IRpcChannelBuffer_Release(pChannel);
            }

            if (FAILED(hr))
                IRpcStubBuffer_Release(pStub);
        }

        if (FAILED(hr) && hProxyDll)
            FreeLibrary(hProxyDll);
    }

    if (pFactory)
        IPSFactoryBuffer_Release(pFactory);
    if (pTypeInfo)
        ITypeInfo_Release(pTypeInfo);

    if (FAILED(hr)) {
        *ppStub = NULL;
        *ppChannel = NULL;
    }

    return hr;
}


//---------------------------------------------------------------------------
// Com_IUnknown_Add_Ref_Release
//---------------------------------------------------------------------------


_FX void Com_IUnknown_Add_Ref_Release(COM_IUNKNOWN *This, UCHAR op)
{
    HRESULT hr;
    COM_ADD_REF_RELEASE_REQ *req;
    COM_ADD_REF_RELEASE_RPL *rpl;
    ULONG len;

    len = sizeof(COM_ADD_REF_RELEASE_REQ);
    req = (COM_ADD_REF_RELEASE_REQ *)Com_Alloc(len);
    if (! req)
        return;
    req->h.length = len;
    req->h.msgid = MSGID_COM_ADD_REF_RELEASE;
    req->objidx = This->ObjIdx;
    req->op = op;

    rpl = (COM_ADD_REF_RELEASE_RPL *)SbieDll_CallServer(&req->h);

    Com_Free(req);

    if (rpl) {
        hr = rpl->h.status;
        Com_Free(rpl);
    } else
        hr = RPC_S_SERVER_UNAVAILABLE;
    if (hr != S_OK)
        Com_RpcRaiseException(hr);
}


//---------------------------------------------------------------------------
// Com_IUnknown_AddRef
//---------------------------------------------------------------------------


_FX ULONG Com_IUnknown_AddRef(COM_IUNKNOWN *This)
{
    if (This->Flags & FLAG_REMOTE_REF)
        Com_IUnknown_Add_Ref_Release(This, 'a');
    return InterlockedIncrement(&This->RefCount);
}


//---------------------------------------------------------------------------
// Com_IUnknown_Release
//---------------------------------------------------------------------------


_FX ULONG Com_IUnknown_Release(COM_IUNKNOWN *This)
{
    ULONG refcount;

    if (This->Flags & FLAG_REMOTE_REF)
       Com_IUnknown_Add_Ref_Release(This, 'r');
    refcount = InterlockedDecrement(&This->RefCount);

    if (! refcount) {

        if (This->Flags & FLAG_PPROXY_AT_VTBL3) {
            IRpcProxyBuffer *pProxy = (IRpcProxyBuffer *)This->Vtbl[3];
            if (pProxy) {
                IRpcProxyBuffer_Disconnect(pProxy);
                IRpcProxyBuffer_Release(pProxy);
            }
        }

        //Com_Mem_Trace = L"IUnknown";
        Com_Free(This);
    }

    return refcount;
}


//---------------------------------------------------------------------------
// Com_IUnknown_New
//---------------------------------------------------------------------------


_FX HRESULT Com_IUnknown_New(
    ULONG objidx, ULONG methods, ULONG flags, COM_IUNKNOWN **ppv)
{
    COM_IUNKNOWN *This;
    ULONG len;

    len = sizeof(COM_IUNKNOWN) + sizeof(ULONG_PTR) * methods;
    This = Com_Alloc(len);
    *ppv = This;
    if (! This)
        return E_OUTOFMEMORY;

    This->pVtbl = This->Vtbl;
    This->RefCount = 1;
    This->ObjIdx = objidx;
    This->Flags = flags;
    This->Vtbl[1] = Com_IUnknown_AddRef;
    This->Vtbl[2] = Com_IUnknown_Release;

    return S_OK;
}


//---------------------------------------------------------------------------
// Com_IClassFactory_QueryInterface
//---------------------------------------------------------------------------


_FX HRESULT Com_IClassFactory_QueryInterface(
    COM_IUNKNOWN *This, REFIID riid, void **ppv)
{
    HRESULT hr;

    if (memcmp(riid, &IID_IUnknown, sizeof(GUID)) == 0 ||
        memcmp(riid, &IID_IClassFactory, sizeof(GUID)) == 0) {

        IUnknown_AddRef((IUnknown *)This);
        *ppv = This;
        hr = S_OK;

    } else {

        *ppv = NULL;
        hr = E_NOINTERFACE;
    }

    return hr;
}


//---------------------------------------------------------------------------
// Com_IClassFactory_CreateInstance
//---------------------------------------------------------------------------


_FX HRESULT Com_IClassFactory_CreateInstance(
    COM_IUNKNOWN *This, IUnknown *pUnkOuter, REFIID riid, void **ppv)
{
    static const WCHAR *TraceType = L"CRE-CF";
    COM_CREATE_INSTANCE_REQ *req;
    COM_CREATE_INSTANCE_RPL *rpl;
    ULONG len;
    ULONG hr;

    if (pUnkOuter)
        SbieApi_Log(2205, L"IClassFactory::CreateInstance");

    *ppv = NULL;

    len = sizeof(COM_CREATE_INSTANCE_REQ);
    req = (COM_CREATE_INSTANCE_REQ *)Com_Alloc(len);
    if (! req)
        return E_OUTOFMEMORY;
    req->h.length = len;
    req->h.msgid = MSGID_COM_CREATE_INSTANCE;
    req->objidx = This->ObjIdx;
    memcpy(&req->iid, riid, sizeof(GUID));

    rpl = (COM_CREATE_INSTANCE_RPL *)SbieDll_CallServer(&req->h);

    Com_Free(req);

    if (rpl) {
        hr = rpl->h.status;
        if (hr != S_OK)
            Com_Free(rpl);
    } else
        hr = RPC_S_SERVER_UNAVAILABLE;

    if (hr != S_OK) {
        Com_Trace(TraceType, &This->Guid, riid, 0, hr);
        Com_RpcRaiseException(hr);
        return E_ABORT;
    }

    hr = rpl->hr;
    if (SUCCEEDED(hr) && (! rpl->objidx))
        hr = E_FAIL;
    if (SUCCEEDED(hr))
        hr = Com_CreateProxy(riid, rpl->objidx, ppv);

    Com_Trace(TraceType, &This->Guid, riid, 0, hr);

    Com_Free(rpl);
    return hr;
}


//---------------------------------------------------------------------------
// Com_IClassFactory_LockServer
//---------------------------------------------------------------------------


_FX HRESULT Com_IClassFactory_LockServer(
    COM_IUNKNOWN *This, BOOL fLock)
{
    SbieApi_Log(2205, L"IClassFactory::LockServer");
    return 0;
}


//---------------------------------------------------------------------------
// Com_IClassFactory_New
//---------------------------------------------------------------------------


_FX HRESULT Com_IClassFactory_New(
    REFCLSID rclsid, const WCHAR *StringGUID, void **ppv)
{
    COM_IUNKNOWN *This;
    ULONG len;
    COM_GET_CLASS_OBJECT_REQ *req;
    COM_GET_CLASS_OBJECT_RPL *rpl;
    ULONG hr;

    *ppv = NULL;

    len = sizeof(COM_GET_CLASS_OBJECT_REQ);
    req = (COM_GET_CLASS_OBJECT_REQ *)Com_Alloc(len);
    if (! req)
        return E_OUTOFMEMORY;
    req->h.length = len;
    req->h.msgid = MSGID_COM_GET_CLASS_OBJECT;
    memcpy(&req->clsid, rclsid, sizeof(GUID));
    memcpy(&req->iid, &IID_IClassFactory, sizeof(GUID));
    req->elevate = (StringGUID != NULL);

    rpl = (COM_GET_CLASS_OBJECT_RPL *)SbieDll_CallServer(&req->h);

    Com_Free(req);

    if (rpl) {
        hr = rpl->h.status;
        if (hr != S_OK) {
            Com_Free(rpl);
            if (hr == ERROR_ELEVATION_REQUIRED && StringGUID) {
                SbieApi_Log(2214, StringGUID);
                SbieApi_Log(2219, L"%S [%S]", Dll_ImageName, Dll_BoxName);
                hr = RPC_S_SERVER_UNAVAILABLE;
            }
        }
    } else
        hr = RPC_S_SERVER_UNAVAILABLE;
    if (hr != S_OK) {
        Com_RpcRaiseException(hr);
        return E_ABORT;
    }

    hr = rpl->hr;
    if (SUCCEEDED(hr)) {

        //Com_Mem_Trace = L"IClassFactory";
        hr = Com_IUnknown_New(rpl->objidx, 2, FLAG_REMOTE_REF, &This);
        if (SUCCEEDED(hr)) {

            This->Vtbl[0] = Com_IClassFactory_QueryInterface;
            This->Vtbl[3] = Com_IClassFactory_CreateInstance;
            This->Vtbl[4] = Com_IClassFactory_LockServer;
            memcpy(&This->Guid, rclsid, sizeof(GUID));

            *ppv = This;
        }
    }

    Com_Free(rpl);
    return hr;
}


//---------------------------------------------------------------------------
// Com_OuterIUnknown_QueryInterface
//---------------------------------------------------------------------------


_FX HRESULT Com_OuterIUnknown_QueryInterface(
    COM_IUNKNOWN *This, REFIID riid, void **ppv)
{
    static const WCHAR *TraceType = L"QRYINT";
    COM_QUERY_INTERFACE_REQ *req;
    COM_QUERY_INTERFACE_RPL *rpl;
    ULONG len;
    ULONG hr;

    if (memcmp(riid, &IID_IUnknown, sizeof(GUID)) == 0 ||
        memcmp(riid, &This->Guid, sizeof(GUID)) == 0) {

        IUnknown *pUnknown = (IUnknown *)This->Vtbl[4];
        IUnknown_AddRef(pUnknown);
        *ppv = pUnknown;
        return S_OK;
    }

    if (memcmp(riid, &IID_IMarshal, sizeof(GUID)) == 0) {

        return Com_IMarshal_New(This->ObjIdx, ppv);
    }

    if (memcmp(riid, &IID_IClientSecurity, sizeof(GUID)) == 0) {

        return Com_IClientSecurity_New(This->ObjIdx, &This->Guid, ppv);
    }

    if (memcmp(riid, &IID_IRpcOptions, sizeof(GUID)) == 0) {

        return Com_IRpcOptions_New(This->ObjIdx, &This->Guid, ppv);
    }

    *ppv = NULL;

    if (memcmp(riid, &IID_IStdIdentity, sizeof(GUID)) == 0) {

        //
        // IID_IStdIdentity is an internal COM interface used by
        // IGlobalInterfaceTable::RegisterInterfaceInGlobal and
        // perhaps other places.  if we issue a "query interface"
        // request to SbieSvc for this interface, it fails during
        // SbieDll_ComCreateStub and might mess up the proxy object.
        //

        return E_NOINTERFACE;
    }

    len = sizeof(COM_QUERY_INTERFACE_REQ);
    req = (COM_QUERY_INTERFACE_REQ *)Com_Alloc(len);
    if (! req)
        return E_OUTOFMEMORY;
    req->h.length = len;
    req->h.msgid = MSGID_COM_QUERY_INTERFACE;
    req->objidx = This->ObjIdx;
    memcpy(&req->iid, riid, sizeof(GUID));

    rpl = (COM_QUERY_INTERFACE_RPL *)SbieDll_CallServer(&req->h);

    Com_Free(req);

    if (rpl) {
        hr = rpl->h.status;
        if (hr != S_OK)
            Com_Free(rpl);
    } else
        hr = RPC_S_SERVER_UNAVAILABLE;
    if (hr != S_OK) {
        Com_Trace(TraceType, &This->Guid, riid, 0, hr);
#ifndef REGHIVE_ALWAYS_MOUNT_NEVER_UNMOUNT // if not sbox build
        Com_RpcRaiseException(hr);
#endif ! REGHIVE_ALWAYS_MOUNT_NEVER_UNMOUNT
        return E_ABORT;
    }

    hr = rpl->hr;
    if (SUCCEEDED(hr) && (! rpl->objidx))
        hr = E_FAIL;
    if (SUCCEEDED(hr))
        hr = Com_CreateProxy(riid, rpl->objidx, ppv);

    Com_Trace(TraceType, &This->Guid, riid, 0, hr);

    Com_Free(rpl);
    return hr;
}


//---------------------------------------------------------------------------
// Com_OuterIUnknown_New
//---------------------------------------------------------------------------


_FX HRESULT Com_OuterIUnknown_New(ULONG objidx, REFIID iid, void **ppv)
{
    HRESULT hr;
    COM_IUNKNOWN *This;

    //Com_Mem_Trace = L"OuterIUnknown";
    hr = Com_IUnknown_New(
            objidx, 3, FLAG_REMOTE_REF | FLAG_PPROXY_AT_VTBL3, &This);
    if (SUCCEEDED(hr)) {

        This->Vtbl[0] = Com_OuterIUnknown_QueryInterface;
        This->Vtbl[3] = NULL;       // to be filled with pProxy
        This->Vtbl[4] = NULL;       // to be filled with pUnknown
        This->Vtbl[5] = NULL;       // to be filled with pStub
        memcpy(&This->Guid, iid, sizeof(GUID));

        *ppv = This;
    }

    return hr;
}


//---------------------------------------------------------------------------
// Com_IRpcChannelBuffer_QueryInterface
//---------------------------------------------------------------------------


_FX HRESULT Com_IRpcChannelBuffer_QueryInterface(
    COM_IUNKNOWN *This, REFIID riid, void **ppv)
{
    HRESULT hr;

    if (memcmp(riid, &IID_IUnknown, sizeof(GUID)) == 0 ||
        memcmp(riid, &IID_IRpcChannelBuffer, sizeof(GUID)) == 0) {

        IUnknown_AddRef((IUnknown *)This);
        *ppv = This;
        hr = S_OK;

    } else {

        *ppv = NULL;
        hr = E_NOINTERFACE;
    }

    return hr;
}


//---------------------------------------------------------------------------
// Com_IRpcChannelBuffer_GetBuffer
//---------------------------------------------------------------------------


_FX HRESULT Com_IRpcChannelBuffer_GetBuffer(
    COM_IUNKNOWN *This, COM_RPC_MESSAGE *pMessage, REFIID riid)
{
    HRESULT hr = S_OK;
    pMessage->Buffer = Com_Alloc(pMessage->BufferLength);
    if (! pMessage->Buffer)
        hr = E_OUTOFMEMORY;
    return hr;
}


//---------------------------------------------------------------------------
// Com_IRpcChannelBuffer_FreeBuffer
//---------------------------------------------------------------------------


_FX HRESULT Com_IRpcChannelBuffer_FreeBuffer(
    COM_IUNKNOWN *This, COM_RPC_MESSAGE *pMessage)
{
    void *Buffer = pMessage->Buffer;
    pMessage->Buffer = NULL;
    if (Buffer)
        Com_Free(Buffer);
    return S_OK;
}


//---------------------------------------------------------------------------
// Com_IRpcChannelBuffer_GetDestCtx
//---------------------------------------------------------------------------


_FX HRESULT Com_IRpcChannelBuffer_GetDestCtx(
    COM_IUNKNOWN *This, ULONG *pdwDestContext, void **ppvDestContext)
{
    *pdwDestContext = MSHCTX_NOSHAREDMEM;
    return S_OK;
}


//---------------------------------------------------------------------------
// Com_IRpcChannelBuffer_IsConnected
//---------------------------------------------------------------------------


_FX HRESULT Com_IRpcChannelBuffer_IsConnected(COM_IUNKNOWN *This)
{
    return S_OK;
}


//---------------------------------------------------------------------------
// Com_IRpcChannelBuffer_SendReceive
//---------------------------------------------------------------------------


_FX HRESULT Com_IRpcChannelBuffer_SendReceive(
    COM_IUNKNOWN *This, COM_RPC_MESSAGE *pMessage, ULONG *pStatus)
{
    static const WCHAR *TraceType = L"INVOKE";
    HRESULT hr;
    ULONG len;
    COM_INVOKE_METHOD_REQ *req;
    COM_INVOKE_METHOD_RPL *rpl;

    ULONG ProcNum = pMessage->ProcNum;

    len = sizeof(COM_INVOKE_METHOD_REQ) + pMessage->BufferLength;
    req = (COM_INVOKE_METHOD_REQ *)Com_Alloc(len);
    if (! req)
        return E_OUTOFMEMORY;

    req->h.length = len;
    req->h.msgid = MSGID_COM_INVOKE_METHOD;
    req->objidx = This->ObjIdx;
    req->DataRepresentation = pMessage->DataRepresentation;
    req->ProcNum = ProcNum;
    req->BufferLength = pMessage->BufferLength;
    memcpy(req->Buffer, pMessage->Buffer, pMessage->BufferLength);

    rpl = (COM_INVOKE_METHOD_RPL *)SbieDll_CallServer(&req->h);

    Com_Free(req);

    if (rpl) {
        hr = rpl->h.status;
        if (hr != S_OK)
            Com_Free(rpl);
    } else
        hr = RPC_S_SERVER_UNAVAILABLE;
    if (pStatus)
        *pStatus = hr;
    if (hr != S_OK) {
        Com_Trace(TraceType, NULL, &This->Guid, ProcNum, hr);
        return E_ABORT;
    }

    hr = rpl->hr;
    if (SUCCEEDED(hr)) {

        IRpcChannelBuffer_FreeBuffer(
            (IRpcChannelBuffer *)This, (RPCOLEMESSAGE *)pMessage);

        pMessage->DataRepresentation = rpl->DataRepresentation;
        pMessage->BufferLength = rpl->BufferLength;
        hr = IRpcChannelBuffer_GetBuffer(
            (IRpcChannelBuffer *)This, (RPCOLEMESSAGE *)pMessage, NULL);
        if (SUCCEEDED(hr))
            memcpy(pMessage->Buffer, rpl->Buffer, rpl->BufferLength);
    }

    Com_Trace(TraceType, NULL, &This->Guid, ProcNum, hr);

    Com_Free(rpl);
    return hr;
}


//---------------------------------------------------------------------------
// SbieDll_IRpcChannelBuffer_New
//---------------------------------------------------------------------------


_FX HRESULT Com_IRpcChannelBuffer_New(
    ULONG objidx, REFIID iid, void **ppv)
{
    HRESULT hr;
    COM_IUNKNOWN *This;

    //Com_Mem_Trace = L"IRpcChannelBuffer";
    hr = Com_IUnknown_New(objidx, 5, 0, &This);
    *ppv = This;
    if (! This)
        return E_OUTOFMEMORY;

    This->Vtbl[0] = Com_IRpcChannelBuffer_QueryInterface;
    This->Vtbl[3] = Com_IRpcChannelBuffer_GetBuffer;
    This->Vtbl[4] = Com_IRpcChannelBuffer_SendReceive;
    This->Vtbl[5] = Com_IRpcChannelBuffer_FreeBuffer;
    This->Vtbl[6] = Com_IRpcChannelBuffer_GetDestCtx;
    This->Vtbl[7] = Com_IRpcChannelBuffer_IsConnected;
    memcpy(&This->Guid, iid, sizeof(GUID));

    return S_OK;
}


//---------------------------------------------------------------------------
// Com_IMarshal_QueryInterface
//---------------------------------------------------------------------------


_FX HRESULT Com_IMarshal_QueryInterface(
    COM_IUNKNOWN *This, REFIID riid, void **ppv)
{
    HRESULT hr;

    if (memcmp(riid, &IID_IUnknown, sizeof(GUID)) == 0 ||
        memcmp(riid, &IID_IMarshal, sizeof(GUID)) == 0) {

        IUnknown_AddRef((IUnknown *)This);
        *ppv = This;
        hr = S_OK;

    } else {

        *ppv = NULL;
        hr = E_NOINTERFACE;
    }

    return hr;
}


//---------------------------------------------------------------------------
// Com_IMarshal_GetUnmarshalClass
//---------------------------------------------------------------------------


_FX HRESULT Com_IMarshal_GetUnmarshalClass(
    COM_IUNKNOWN *This, REFIID riid, void *pv,
    ULONG dwDestContext, void *pvDestContext, ULONG mshlflags,
    CLSID *pCid)
{
    memcpy(pCid, &CLSID_StdMarshal, sizeof(GUID));
    return S_OK;
}


//---------------------------------------------------------------------------
// Com_IMarshal_GetMarshalSizeMax
//---------------------------------------------------------------------------


_FX HRESULT Com_IMarshal_GetMarshalSizeMax(
    COM_IUNKNOWN *This, REFIID riid, void *pv,
    ULONG dwDestContext, void *pvDestContext, ULONG mshlflags,
    ULONG *pSize)
{
    *pSize = 1024;
    return S_OK;
}


//---------------------------------------------------------------------------
// Com_IMarshal_MarshalInterface
//---------------------------------------------------------------------------


_FX HRESULT Com_IMarshal_MarshalInterface(
    COM_IUNKNOWN *This, IStream *pStm, REFIID riid,
    void *pv, ULONG dwDestContext, void *pvDestContext, ULONG mshlflags)
{
    static const WCHAR *TraceType = L"MARSHL";
    HRESULT hr;
    COM_MARSHAL_INTERFACE_REQ req;
    COM_MARSHAL_INTERFACE_RPL *rpl;

    req.h.length = sizeof(COM_MARSHAL_INTERFACE_REQ);
    req.h.msgid = MSGID_COM_MARSHAL_INTERFACE;
    req.objidx = This->ObjIdx;
    memcpy(&req.iid, riid, sizeof(GUID));
    req.destctx = dwDestContext;
    req.mshlflags = mshlflags;

    rpl = (COM_MARSHAL_INTERFACE_RPL *)SbieDll_CallServer(&req.h);

    if (rpl) {
        hr = rpl->h.status;
        if (hr != S_OK)
            Com_Free(rpl);
    } else
        hr = RPC_S_SERVER_UNAVAILABLE;
    if (hr != S_OK) {
        Com_Trace(TraceType, NULL, &This->Guid, 0, hr);
        Com_RpcRaiseException(hr);
        return E_ABORT;
    }

    hr = rpl->hr;
    if (SUCCEEDED(hr)) {

        ULONG count = 0;
        hr = IStream_Write(pStm, rpl->Buffer, rpl->BufferLength, &count);
    }

    Com_Trace(TraceType, NULL, riid, 0, hr);

    Com_Free(rpl);
    return hr;
}


//---------------------------------------------------------------------------
// Com_IMarshal_UnmarshalInterface
//---------------------------------------------------------------------------


_FX HRESULT Com_IMarshal_UnmarshalInterface(
    COM_IUNKNOWN *This, IStream *pStm, REFIID riid, void **pv)
{
    SbieApi_Log(2205, L"IMarshal::UnmarshalInterface");
    return E_NOTIMPL;
}


//---------------------------------------------------------------------------
// Com_IMarshal_ReleaseMarshalData
//---------------------------------------------------------------------------


_FX HRESULT Com_IMarshal_ReleaseMarshalData(
    COM_IUNKNOWN *This, IStream *pStm)
{
    SbieApi_Log(2205, L"IMarshal::ReleaseMarshalData");
    return S_OK;
}


//---------------------------------------------------------------------------
// Com_IMarshal_DisconnectObject
//---------------------------------------------------------------------------


_FX HRESULT Com_IMarshal_DisconnectObject(
    COM_IUNKNOWN *This, ULONG dwReserved)
{
    SbieApi_Log(2205, L"IMarshal::DisconnectObject");
    return S_OK;
}


//---------------------------------------------------------------------------
// Com_IMarshal_New
//---------------------------------------------------------------------------


_FX HRESULT Com_IMarshal_New(ULONG objidx, void **ppv)
{
    HRESULT hr;
    COM_IUNKNOWN *This;

    //Com_Mem_Trace = L"IMarshal";
    hr = Com_IUnknown_New(objidx, 6, 0, &This);
    *ppv = This;
    if (! This)
        return E_OUTOFMEMORY;

    This->Vtbl[0] = Com_IMarshal_QueryInterface;
    This->Vtbl[3] = Com_IMarshal_GetUnmarshalClass;
    This->Vtbl[4] = Com_IMarshal_GetMarshalSizeMax;
    This->Vtbl[5] = Com_IMarshal_MarshalInterface;
    This->Vtbl[6] = Com_IMarshal_UnmarshalInterface;
    This->Vtbl[7] = Com_IMarshal_ReleaseMarshalData;
    This->Vtbl[8] = Com_IMarshal_DisconnectObject;

    return S_OK;
}


//---------------------------------------------------------------------------
// Com_IClientSecurity_QueryInterface
//---------------------------------------------------------------------------


_FX HRESULT Com_IClientSecurity_QueryInterface(
    COM_IUNKNOWN *This, REFIID riid, void **ppv)
{
    HRESULT hr;

    if (memcmp(riid, &IID_IUnknown, sizeof(GUID)) == 0 ||
        memcmp(riid, &IID_IClientSecurity, sizeof(GUID)) == 0) {

        IUnknown_AddRef((IUnknown *)This);
        *ppv = This;
        hr = S_OK;

    } else {

        *ppv = NULL;
        hr = E_NOINTERFACE;
    }

    return hr;
}


//---------------------------------------------------------------------------
// Com_IClientSecurity_QueryBlanket
//---------------------------------------------------------------------------


_FX HRESULT Com_IClientSecurity_QueryBlanket(
    COM_IUNKNOWN *This, IUnknown *pProxy,
    ULONG *pAuthnSvc, ULONG *pAuthzSvc, WCHAR **pServerPrincName,
    ULONG *pAuthnLevel, ULONG *pImpLevel,
    RPC_AUTH_IDENTITY_HANDLE *pAuthInfo, ULONG *pCapabilities)
{
    static const WCHAR *TraceType = L"Q-BLKT";
    HRESULT hr;
    COM_QUERY_BLANKET_REQ req;
    COM_QUERY_BLANKET_RPL *rpl;

    req.h.length = sizeof(COM_QUERY_BLANKET_REQ);
    req.h.msgid = MSGID_COM_QUERY_BLANKET;
    req.objidx = This->ObjIdx;

    rpl = (COM_QUERY_BLANKET_RPL *)SbieDll_CallServer(&req.h);

    if (rpl) {
        hr = rpl->h.status;
        if (hr != S_OK)
            Com_Free(rpl);
    } else
        hr = RPC_S_SERVER_UNAVAILABLE;
    if (hr != S_OK) {
        Com_Trace(TraceType, NULL, &This->Guid, 0, hr);
        Com_RpcRaiseException(hr);
        return E_ABORT;
    }

    hr = rpl->hr;
    if (SUCCEEDED(hr)) {

        *pAuthnSvc = rpl->AuthnSvc;
        if (pAuthzSvc)
            *pAuthzSvc = rpl->AuthzSvc;
        if (pServerPrincName) {
            ULONG len = (wcslen(rpl->ServerPrincName) + 1) * sizeof(WCHAR);
            *pServerPrincName = (WCHAR *)__sys_CoTaskMemAlloc(len);
            memcpy(*pServerPrincName, rpl->ServerPrincName, len);
        }
        if (pAuthnLevel)
            *pAuthnLevel = rpl->AuthnLevel;
        if (pImpLevel)
            *pImpLevel = rpl->ImpLevel;
        if (pAuthInfo)
            *pAuthInfo = NULL;
        if (pCapabilities)
            *pCapabilities = rpl->Capabilities;
    }

    Com_Trace(TraceType, NULL, &This->Guid, 0, hr);

    Com_Free(rpl);
    return hr;
}


//---------------------------------------------------------------------------
// Com_IClientSecurity_SetBlanket
//---------------------------------------------------------------------------


_FX HRESULT Com_IClientSecurity_SetBlanket(
    COM_IUNKNOWN *This, IUnknown *pProxy,
    ULONG dwAuthnSvc, ULONG dwAuthzSvc, WCHAR *pServerPrincName,
    ULONG dwAuthnLevel, ULONG dwImpLevel,
    RPC_AUTH_IDENTITY_HANDLE pAuthInfo, ULONG dwCapabilities)
{
    static const WCHAR *TraceType = L"S-BLKT";
    HRESULT hr;
    COM_SET_BLANKET_REQ req;
    COM_SET_BLANKET_RPL *rpl;

    req.h.length = sizeof(COM_SET_BLANKET_REQ);
    req.h.msgid = MSGID_COM_SET_BLANKET;
    req.objidx = This->ObjIdx;

    req.AuthnSvc     = dwAuthnSvc;
    req.AuthzSvc     = dwAuthzSvc;
    req.AuthnLevel   = dwAuthnLevel;
    req.ImpLevel     = dwImpLevel;
    req.Capabilities = dwCapabilities;

    req.ServerPrincName[0] = L'\0';
    req.DefaultServerPrincName = FALSE;

    if (pServerPrincName == COLE_DEFAULT_PRINCIPAL) {

        req.DefaultServerPrincName = TRUE;

    } else if (pServerPrincName) {

        ULONG copy_len = wcslen(pServerPrincName) * sizeof(WCHAR);
        ULONG max_len = sizeof(req.ServerPrincName) - sizeof(WCHAR);
        if (copy_len > max_len)
            copy_len = max_len;
        memcpy(req.ServerPrincName, pServerPrincName, copy_len);
        req.ServerPrincName[copy_len / sizeof(WCHAR)] = L'\0';
    }

    rpl = (COM_SET_BLANKET_RPL *)SbieDll_CallServer(&req.h);

    if (rpl) {
        hr = rpl->h.status;
        if (hr != S_OK)
            Com_Free(rpl);
    } else
        hr = RPC_S_SERVER_UNAVAILABLE;
    if (hr != S_OK) {
        Com_Trace(TraceType, NULL, &This->Guid, 0, hr);
        Com_RpcRaiseException(hr);
        return E_ABORT;
    }

    Com_Trace(TraceType, NULL, &This->Guid, 0, hr);

    Com_Free(rpl);
    return hr;
}


//---------------------------------------------------------------------------
// Com_IClientSecurity_CopyProxy
//---------------------------------------------------------------------------


_FX HRESULT Com_IClientSecurity_CopyProxy(
    COM_IUNKNOWN *This, IUnknown *pProxy, IUnknown **ppCopy)
{
    static const WCHAR *TraceType = L"CPYPRX";
    HRESULT hr;
    COM_COPY_PROXY_REQ req;
    COM_COPY_PROXY_RPL *rpl;

    req.h.length = sizeof(COM_COPY_PROXY_REQ);
    req.h.msgid = MSGID_COM_COPY_PROXY;
    req.objidx = This->ObjIdx;

    rpl = (COM_COPY_PROXY_RPL *)SbieDll_CallServer(&req.h);

    if (rpl) {
        hr = rpl->h.status;
        if (hr != S_OK)
            Com_Free(rpl);
    } else
        hr = RPC_S_SERVER_UNAVAILABLE;
    if (hr != S_OK) {
        Com_Trace(TraceType, NULL, &This->Guid, 0, hr);
        Com_RpcRaiseException(hr);
        return E_ABORT;
    }

    hr = rpl->hr;
    if (SUCCEEDED(hr) && (! rpl->objidx))
        hr = E_FAIL;
    if (SUCCEEDED(hr))
        hr = Com_CreateProxy(&This->Guid, rpl->objidx, ppCopy);

    Com_Trace(TraceType, NULL, &This->Guid, 0, hr);

    Com_Free(rpl);
    return hr;
}


//---------------------------------------------------------------------------
// Com_IClientSecurity_New
//---------------------------------------------------------------------------


_FX HRESULT Com_IClientSecurity_New(ULONG objidx, REFIID iid, void **ppv)
{
    HRESULT hr;
    COM_IUNKNOWN *This;

    //Com_Mem_Trace = L"IClientSecurity";
    hr = Com_IUnknown_New(objidx, 3, 0, &This);
    *ppv = This;
    if (! This)
        return E_OUTOFMEMORY;

    This->Vtbl[0] = Com_IClientSecurity_QueryInterface;
    This->Vtbl[3] = Com_IClientSecurity_QueryBlanket;
    This->Vtbl[4] = Com_IClientSecurity_SetBlanket;
    This->Vtbl[5] = Com_IClientSecurity_CopyProxy;
    memcpy(&This->Guid, iid, sizeof(GUID));

    return S_OK;
}


//---------------------------------------------------------------------------
// Com_IRpcOptions_QueryInterface
//---------------------------------------------------------------------------


_FX HRESULT Com_IRpcOptions_QueryInterface(
    COM_IUNKNOWN *This, REFIID riid, void **ppv)
{
    HRESULT hr;

    if (memcmp(riid, &IID_IUnknown, sizeof(GUID)) == 0 ||
        memcmp(riid, &IID_IRpcOptions, sizeof(GUID)) == 0) {

        IUnknown_AddRef((IUnknown *)This);
        *ppv = This;
        hr = S_OK;

    } else {

        *ppv = NULL;
        hr = E_NOINTERFACE;
    }

    return hr;
}


//---------------------------------------------------------------------------
// Com_IRpcOptions_Set
//---------------------------------------------------------------------------


_FX HRESULT Com_IRpcOptions_Set(
    COM_IUNKNOWN *This, IUnknown *pPrx, DWORD prop, ULONG_PTR value)
{
    if (prop != COMBND_RPCTIMEOUT)
        return E_INVALIDARG;
    if (value != RPC_C_BINDING_INFINITE_TIMEOUT &&
        value != RPC_C_BINDING_MIN_TIMEOUT &&
        value != RPC_C_BINDING_DEFAULT_TIMEOUT &&
        value != RPC_C_BINDING_MAX_TIMEOUT)
        return E_INVALIDARG;
    This->Vtbl[5] = (void *)value;
    return S_OK;
}


//---------------------------------------------------------------------------
// Com_IRpcOptions_Query
//---------------------------------------------------------------------------


_FX HRESULT Com_IRpcOptions_Query(
    COM_IUNKNOWN *This, IUnknown *pPrx, DWORD prop, ULONG_PTR *value)
{
    if (prop == COMBND_RPCTIMEOUT)
        *value = (ULONG_PTR)This->Vtbl[5];
    else if (prop == COMBND_SERVER_LOCALITY)
        *value = SERVER_LOCALITY_MACHINE_LOCAL;
    else
        return E_INVALIDARG;
    return S_OK;
}


//---------------------------------------------------------------------------
// Com_IRpcOptions_New
//---------------------------------------------------------------------------


_FX HRESULT Com_IRpcOptions_New(ULONG objidx, REFIID iid, void **ppv)
{
    HRESULT hr;
    COM_IUNKNOWN *This;

    //Com_Mem_Trace = L"IRpcOptions";
    hr = Com_IUnknown_New(objidx, 3, 0, &This);
    *ppv = This;
    if (! This)
        return E_OUTOFMEMORY;

    This->Vtbl[0] = Com_IRpcOptions_QueryInterface;
    This->Vtbl[3] = Com_IRpcOptions_Set;
    This->Vtbl[4] = Com_IRpcOptions_Query;
    This->Vtbl[5] = (void *)RPC_C_BINDING_DEFAULT_TIMEOUT;
    memcpy(&This->Guid, iid, sizeof(GUID));

    return S_OK;
}


//---------------------------------------------------------------------------
// Com_IConnectionPoint_Hook
//---------------------------------------------------------------------------


_FX void Com_IConnectionPoint_Hook(void *pUnknown)
{
    ULONG_PTR **lpVtbl;
    ULONG_PTR *NewVtbl;

    lpVtbl = (ULONG_PTR **)pUnknown;
    NewVtbl = Com_Alloc(sizeof(ULONG_PTR) * 8);
    memcpy(NewVtbl, *lpVtbl, sizeof(ULONG_PTR) * 8);
    NewVtbl[5] = (ULONG_PTR)Com_IConnectionPoint_Advise;
    NewVtbl[6] = (ULONG_PTR)Com_IConnectionPoint_Unadvise;
    *lpVtbl = NewVtbl;
}


//---------------------------------------------------------------------------
// Com_IConnectionPoint_Advise
//---------------------------------------------------------------------------


_FX HRESULT Com_IConnectionPoint_Advise(
    void *pUnknown, void *pSink, ULONG *pCookie)
{
    if ((! pSink) || (! pCookie))
        return E_POINTER;
    *pCookie = tzuk;
    return S_OK;
}


//---------------------------------------------------------------------------
// Com_IConnectionPoint_Unadvise
//---------------------------------------------------------------------------


_FX HRESULT Com_IConnectionPoint_Unadvise(void *pUnknown, ULONG Cookie)
{
    if (Cookie != tzuk)
        return E_POINTER;
    return S_OK;
}


//---------------------------------------------------------------------------
// Com_Alloc
//---------------------------------------------------------------------------


_FX void *Com_Alloc(ULONG len)
{
    void *ptr;
    if (1 || Dll_BoxName)
        ptr = Dll_Alloc(len);
    else
        ptr = HeapAlloc(GetProcessHeap(), 0, len);

    /*if (Com_Mem_Trace) {
        WCHAR txt[128];
        Sbie_snwprintf(txt, 128, L"ALLOC <%s> AT <%08X>\n", Com_Mem_Trace, ptr);
        OutputDebugString(txt);
        Com_Mem_Trace = NULL;
    }*/

    return ptr;
}


//---------------------------------------------------------------------------
// Com_Free
//---------------------------------------------------------------------------


_FX void Com_Free(void *ptr)
{
    /*if (Com_Mem_Trace) {
        WCHAR txt[128];
        Sbie_snwprintf(txt, 128, L"FREE  <%s> AT <%08X>\n", Com_Mem_Trace, ptr);
        OutputDebugString(txt);
        Com_Mem_Trace = NULL;
    }*/

    if (1 || Dll_BoxName)
        Dll_Free(ptr);
    else
        HeapFree(GetProcessHeap(), 0, ptr);
}


//---------------------------------------------------------------------------
// Com_Trace_Guid
//---------------------------------------------------------------------------


_FX void Com_Trace_Guid(
    WCHAR *text, REFGUID guid, WCHAR *subkey)
{
    WCHAR *ptr;
    HKEY hkey;
    LONG rc;

    if (! __sys_StringFromGUID2) {

        HMODULE module = GetModuleHandle(DllName_ole32);
        GETPROCADDR_SYS(StringFromGUID2);

        if (!__sys_StringFromGUID2)
        {
            wcscpy(text, L"unknown");
            return;
        }
    }

    wcscpy(text, subkey);
    ptr = text + wcslen(text);
    *ptr = L'\\';
    ++ptr;
    __sys_StringFromGUID2(guid, ptr, 48);

    rc = __sys_RegOpenKeyExW(HKEY_CLASSES_ROOT, text, 0, KEY_READ, &hkey);

    wmemmove(text, ptr, 48);
    ptr = text + wcslen(text);
    *ptr = L' ';
    ++ptr;

    if (rc == 0) {

        ULONG len;
        UNICODE_STRING ValueName;
        KEY_VALUE_PARTIAL_INFORMATION *info;
        ULONG_PTR info_ptr;

        info_ptr = (ULONG_PTR)ptr;
        if (info_ptr & 3)
            info_ptr = (info_ptr + 3) & (~3);
        info = (KEY_VALUE_PARTIAL_INFORMATION *)info_ptr;

        RtlInitUnicodeString(&ValueName, L"");

        rc = NtQueryValueKey(
                hkey, &ValueName, KeyValuePartialInformation,
                info, 128, &len);

        if (rc == 0) {

            len = info->DataLength / sizeof(WCHAR);
            wmemmove(ptr, (WCHAR*)info->Data, len);
            ptr[len] = L'\0';
        }

        __sys_RegCloseKey(hkey);
    }

    if (rc != 0) {
        wcscpy(ptr, L"IUnknown");
    }
}


//---------------------------------------------------------------------------
// Com_Trace
//---------------------------------------------------------------------------


_FX void Com_Trace(
    const WCHAR* TraceType, REFCLSID rclsid, REFIID riid,
    ULONG ProcNum, HRESULT hr)
{
    Com_Trace2(TraceType, NULL, rclsid, riid, ProcNum, 0, hr, MONITOR_TRACE);
}

_FX void Com_Trace2(
    const WCHAR* TraceType, const WCHAR* pszName, REFCLSID rclsid, REFIID riid,
    ULONG ProcNum, ULONG clsctx, HRESULT hr, ULONG monflag)
{
    WCHAR *text;
    WCHAR *ptr;

    if (! Com_TraceFlag)
        return;

    text = Com_Alloc(1024 * sizeof(WCHAR));
    ptr = text + Sbie_snwprintf(text, 1024, L"COM <%08X> %s <%08X> ", clsctx, TraceType, hr);

    if (pszName) {
        wcscpy(ptr, L"NAME: ");
        ptr += 6; // wcslen(ptr);
        wcscpy(ptr, pszName);
        ptr += wcslen(ptr);
    }

    if (rclsid) {
        wcscpy(ptr, L"CLSID: ");
        ptr += 7; // wcslen(ptr);
        Com_Trace_Guid(ptr, rclsid, L"CLSID");
        ptr += wcslen(ptr);
    }

    if (riid) {
        if (rclsid) {
            *ptr++ = L';';
            *ptr++ = L' ';
        }
        wcscpy(ptr, L"Interface: ");
        ptr += 11; // wcslen(ptr);
        Com_Trace_Guid(ptr, riid, L"Interface");
        ptr += wcslen(ptr);

        if (! rclsid) {
            *ptr = L' ';
            ++ptr;
            _itow(ProcNum, ptr, 10);
            ptr += wcslen(ptr);
        }
    }

    //ptr[0] = L'\n';
    //ptr[1] = L'\0';
    //OutputDebugString(text);
    *ptr = L'\0';
    SbieApi_MonitorPut2(MONITOR_COMCLASS | monflag, text, FALSE);

    Com_Free(text);
}


//---------------------------------------------------------------------------
// Com_Monitor
//---------------------------------------------------------------------------


_FX void Com_Monitor(REFCLSID rclsid, ULONG monflag)
{
    if (Dll_BoxName) {

        WCHAR text[160];
        Com_Trace_Guid(text, rclsid, L"CLSID");
        SbieApi_MonitorPut2(MONITOR_COMCLASS | monflag, text, FALSE);
    }
}


//---------------------------------------------------------------------------
// Com_LoadRTList
//---------------------------------------------------------------------------


_FX void Com_LoadRTList(const WCHAR* setting, WCHAR** pNames)
{
    if (*pNames != NULL)
        return; // already done

    ULONG index;
    ULONG total_len;
    ULONG cur_pos;
    NTSTATUS status;
    WCHAR wbuf[196];
    WCHAR* buf = wbuf;
    const WCHAR* ImageName = Dll_ImageName;

    total_len = 1;
    for (index = 0; ; ++index) {
        status = SbieApi_QueryConfAsIs(
            NULL, setting, index, buf, 190 * sizeof(WCHAR));
        if (!NT_SUCCESS(status))
            break;

        WCHAR* ptr = wcschr(buf, L',');
        if (ptr) {
            *ptr = L'\0';
            if (ImageName && !SbieDll_MatchImage(buf, ImageName, NULL))
                continue;
            buf = ptr + 1;
        }

        if (*buf != L'\0') continue;

        total_len += wcslen(buf) + 1;
    }

    //
    // We store the data as a list of \0 separated strings the first empty string indicated the end
    //

    *pNames = Com_Alloc(sizeof(WCHAR) * total_len);
    if (!*pNames)
        return;
    
    cur_pos = 0;
    for (index = 0; total_len > cur_pos; ++index) {

        status = SbieApi_QueryConfAsIs(
            NULL, setting, index, buf, 190 * sizeof(WCHAR));
        if (!NT_SUCCESS(status))
            break;

        WCHAR* ptr = wcschr(buf, L',');
        if (ptr) {
            *ptr = L'\0';
            if (ImageName && !SbieDll_MatchImage(buf, ImageName, NULL))
                continue;
            buf = ptr + 1;
        }

        if (*buf != L'\0') continue;

        wcscpy((*pNames) + cur_pos, buf);

        cur_pos += wcslen(buf) + 1;
    }

    (*pNames)[total_len - 1] = L'\0'; // indicated the end of data
}


//---------------------------------------------------------------------------
// Com_IsClosedRT
//---------------------------------------------------------------------------


_FX BOOLEAN Com_IsClosedRT(const wchar_t* strClassId)
{
    //
    // Even in compartment mode, these things are functional only in combination with open COM
    //

    if (!(Ipc_OpenCOM && Dll_CompartmentMode) && !SbieApi_QueryConfBool(NULL, L"DisableRTBlacklist", FALSE)) {

        //
        // Chrome uses the FindAppUriHandlersAsync, which fails returning a NULL value when we don't have com open and more rights
        // than we should have. Chrome does not check for this failure mode and dereferences it, resulting in a fatal crash.
        // Since we don't support modern app features anyways, the simplest workaround is to block this interface.
        //

        if (Dll_ImageType == DLL_IMAGE_GOOGLE_CHROME) {

            if (wcscmp(strClassId, L"Windows.System.Launcher") == 0)
                return TRUE;
        }

        //
        // ToastNotificationManager requires open com and original token, with boxed com this causes a deadlock
        //

        if (wcscmp(strClassId, L"Windows.UI.Notifications.ToastNotificationManager") == 0)
            return TRUE;
    }

    static const WCHAR* setting = L"ClosedRT";
    Com_LoadRTList(setting, &Com_ClosedRT);

    for (const WCHAR* pName = Com_ClosedRT; pName && *pName; pName += wcslen(pName) + 1) {

        if (wcscmp(strClassId, pName) == 0 || wcscmp(pName, L"*") == 0)
            return TRUE; 
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// Com_RoGetActivationFactory
//---------------------------------------------------------------------------


_FX HRESULT Com_RoGetActivationFactory(HSTRING activatableClassId, REFIID  iid, void** factory)
{
    if (__sys_WindowsGetStringRawBuffer == NULL)
        return E_NOTIMPL; // if we can't inspect this modern RT stuff just fail.

    const wchar_t* strClassId = __sys_WindowsGetStringRawBuffer(activatableClassId, NULL);

    if (Com_IsClosedRT(strClassId)) {
        SbieApi_MonitorPut2(MONITOR_RTCLASS | MONITOR_DENY, strClassId, FALSE);
        return E_ACCESSDENIED;
    }

    SbieApi_MonitorPut2(MONITOR_RTCLASS, strClassId, FALSE);
    return __sys_RoGetActivationFactory(activatableClassId, iid, factory);
}


/*
//---------------------------------------------------------------------------
// Com_IClassFactoryEx_CreateInstance
//---------------------------------------------------------------------------

_FX HRESULT Com_OuterIUnknown_QueryInterface_NotImpl(
    COM_IUNKNOWN* This, REFIID riid, void** ppv)
{
    SbieApi_Log(2205, L"IUnknown::QueryInterface");
    return E_NOTIMPL;
}

_FX HRESULT Com_IsWindowOnCurrentVirtualDesktop(COM_IUNKNOWN* This, __RPC__in HWND topLevelWindow, __RPC__out BOOL* onCurrentDesktop) {
    return E_NOTIMPL;
}
_FX HRESULT Com_GetWindowDesktopId(COM_IUNKNOWN* This, __RPC__in HWND topLevelWindow, __RPC__out GUID *desktopId) {
    return E_NOTIMPL;
}
_FX HRESULT Com_MoveWindowToDesktop(COM_IUNKNOWN* This, __RPC__in HWND topLevelWindow, __RPC__in REFGUID desktopId) {
    return E_NOTIMPL;
}

void* Com_VirtualDesktopManager_vtbl[] = { Com_IsWindowOnCurrentVirtualDesktop,Com_GetWindowDesktopId,Com_MoveWindowToDesktop };

typedef struct _COM_ITF {
    const GUID* Guid;
    void** vtbl;
    int vtblCnt;
} COM_ITF;

COM_ITF Com_Interfaces[] = { {&IID_VirtualDesktopManager, Com_VirtualDesktopManager_vtbl, ARRAYSIZE(Com_VirtualDesktopManager_vtbl)} };


_FX HRESULT Com_IClassFactoryEx_CreateInstance(
    COM_IUNKNOWN *This, IUnknown *pUnkOuter, REFIID riid, void **ppv)
{
    ULONG hr;
    COM_IUNKNOWN *pUnknown;

    if (pUnkOuter)
        SbieApi_Log(2205, L"IClassFactory::CreateInstance");

    for (ULONG i = 0; i < ARRAYSIZE(Com_Interfaces); i++) {

        if (memcmp(&This->Guid, Com_Interfaces[i].Guid, 16) == 0) {

            hr = Com_IUnknown_New(-1, 3, FLAG_REMOTE_REF | FLAG_PPROXY_AT_VTBL3, &pUnknown);
            if (SUCCEEDED(hr)) {

                pUnknown->Vtbl[0] = Com_OuterIUnknown_QueryInterface_NotImpl;
                memcpy(&pUnknown->Vtbl[3], Com_Interfaces[i].vtbl, sizeof(void*) * Com_Interfaces[i].vtblCnt);
                memcpy(&pUnknown->Guid, riid, sizeof(GUID));

                *ppv = pUnknown;
            }
            return hr;
        }
    }

    return E_NOTIMPL;
}


//---------------------------------------------------------------------------
// Com_IClassFactoryEx_New
//---------------------------------------------------------------------------


_FX HRESULT Com_IClassFactoryEx_New(
    REFCLSID rclsid, const WCHAR* StringGUID, void** ppv)
{
    HRESULT hr;
    COM_IUNKNOWN *This;
    hr = Com_IUnknown_New(-1, 2, FLAG_REMOTE_REF, &This);
    if (SUCCEEDED(hr)) {

        This->Vtbl[0] = Com_IClassFactory_QueryInterface;
        This->Vtbl[3] = Com_IClassFactoryEx_CreateInstance;
        This->Vtbl[4] = Com_IClassFactory_LockServer;
        memcpy(&This->Guid, rclsid, sizeof(GUID));

        *ppv = This;
    }
    return hr;
}
*/
