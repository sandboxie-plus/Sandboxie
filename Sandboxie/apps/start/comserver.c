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
// COM Server Hooking
//---------------------------------------------------------------------------


#define COBJMACROS
#include <objbase.h>
#include <docobj.h>
#include <htiface.h>
#include "htiface7.h"
#include <htiframe.h>
#include <hlink.h>
#include <exdisp.h>
#include <mshtmlc.h>
#include <stdio.h>
#include "common/defines.h"
#include "common/my_version.h"
#include "core/dll/sbiedll.h"
#include "core/drv/api_flags.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define COMSERVER_DEBUG


//---------------------------------------------------------------------------
// IMyUnknown / IMyClassFactory
//---------------------------------------------------------------------------


typedef struct _IMyUnknown IMyUnknown;

typedef IMyUnknown *(*P_MyCreateInstance)(REFIID riid);
typedef void *(*P_MyQueryInterface)(IMyUnknown *This, REFIID riid);


typedef struct _IMyUnknown {

    ULONG_PTR Vtbls[16];
    P_MyQueryInterface pMyQueryInterface;
    struct {
        ULONG_PTR pBackPointerToObject;
        ULONG_PTR pIUnknownQueryInterface;
        ULONG_PTR pIUnknownAddRef;
        ULONG_PTR pIUnknownRelease;
    } IUnknownVtbl;
    ULONG_PTR VtblSpace;
    // followed by more vtbls

} IMyUnknown;


typedef struct _IMyClassFactory {

    struct IClassFactoryVtbl *lpVtbl;       // struct IClassFactory
    P_MyCreateInstance pMyCreateInstance;
    IClassFactoryVtbl VtblSpace;

} IMyClassFactory;


static IUnknown *ComServer_MyClassFactory_New(
    P_MyCreateInstance pMyCreateInstance);


static HRESULT ComServer_IClassFactory_QueryInterface(
    IClassFactory *This, REFIID riid, void **ppvObject);
static ULONG ComServer_IClassFactory_AddRef(IClassFactory *This);
static ULONG ComServer_IClassFactory_Release(IClassFactory *This);
static HRESULT ComServer_IClassFactory_CreateInstance(
    IClassFactory *This, IUnknown *pUnkOuter, REFIID riid, void **ppvObject);
static HRESULT ComServer_IClassFactory_LockServer(
    IClassFactory *This, BOOL fLock);


static IMyUnknown *ComServer_MyUnknown_New(
    P_MyQueryInterface pMyQueryInterface, ULONG SizeofVtbls);


static HRESULT ComServer_IUnknown_QueryInterface(
    IUnknown *This, REFIID riid, void **ppvObject);
static ULONG ComServer_IUnknown_AddRef(IUnknown *This);
static ULONG ComServer_IUnknown_Release(IUnknown *This);


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static void ComServer_RestartProgram(const WCHAR *arg);
static void *Dll_Alloc(ULONG size);


//---------------------------------------------------------------------------
// GUIDs
//---------------------------------------------------------------------------


// {00000000-0000-0000-C000-000000000046}
static const GUID IID_IUnknown = {
    0x00000000, 0x0000, 0x0000,
        { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 }
};

// {00000001-0000-0000-C000-000000000046}
static const GUID IID_IClassFactory = {
    0x00000001, 0x0000, 0x0000,
        { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 }
};


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static volatile LONG ComServer_Factory_RefCount = 0;
static volatile BOOL ComServer_Factory_Locked = FALSE;
static volatile LONG ComServer_Object_RefCount = 0;

static WCHAR *ComServer_CommandLine = NULL;

enum {
    ComServer_ImageType_Unknown,
    ComServer_ImageType_IE,
    ComServer_ImageType_WMP,
    ComServer_ImageType_WINAMP,
    ComServer_ImageType_KMPLAYER,
    ComServer_ImageType_Last
};
static ULONG ComServer_ImageType = ComServer_ImageType_Unknown;


//---------------------------------------------------------------------------
// Sub modules
//---------------------------------------------------------------------------


#include "comserver_ie.c"
#include "comserver_wmp.c"


//---------------------------------------------------------------------------
// ComServer_Main
//---------------------------------------------------------------------------


_FX void *ComServer_Main(void)
{
    MSG msg;
    DWORD cookie;
    HRESULT hr;
    int hr_lvl;
    WCHAR *pgm;

    //
    //
    //

    WCHAR *exepath = Dll_Alloc((MAX_PATH + 4) * sizeof(WCHAR));
    WCHAR *cmdline = Dll_Alloc(8192 * sizeof(WCHAR));
    GetEnvironmentVariable(ENV_VAR_PFX L"COMSRV_EXE", exepath, MAX_PATH);
    exepath[MAX_PATH] = L'\0';
    GetEnvironmentVariable(ENV_VAR_PFX L"COMSRV_CMD", cmdline, 8190);
    cmdline[8190] = L'\0';
    OutputDebugString(L"COM SERVER EXE PATH\n");
    OutputDebugString(exepath);
    OutputDebugString(L"COM SERVER CMD LINE\n");
    OutputDebugString(cmdline);

    //
    //
    //

    pgm = wcsrchr(exepath, L'\\');
    if (pgm) {
        ++pgm;
        if (_wcsicmp(pgm, L"iexplore.exe") == 0)
            ComServer_ImageType = ComServer_ImageType_IE;
        else if (_wcsicmp(pgm, L"wmplayer.exe") == 0)
            ComServer_ImageType = ComServer_ImageType_WMP;
        else if (_wcsicmp(pgm, L"winamp.exe") == 0)
            ComServer_ImageType = ComServer_ImageType_WMP;
        else if (_wcsicmp(pgm, L"kmplayer.exe") == 0)
            ComServer_ImageType = ComServer_ImageType_WMP;
    }
    if (ComServer_ImageType == ComServer_ImageType_Unknown) {
        hr_lvl = 9;
        hr = -1;
        goto abort;
    }

    //
    // register with the real system COM as the provider
    // for objects with CLSID_InternetExplorer
    //

    hr_lvl = 1;
    hr = CoInitialize(0);
    if (SUCCEEDED(hr)) {

        const GUID *pClsid;
        P_MyCreateInstance pMyCreateInstance;

        if (ComServer_ImageType == ComServer_ImageType_IE) {

            pClsid = &CLSID_InternetExplorer;
            pMyCreateInstance = IEServer_MyCreateInstance;

        } else if (ComServer_ImageType == ComServer_ImageType_WMP) {

            pClsid = &CLSID_WindowsMediaPlayer_Play;
            if (cmdline && wcsstr(cmdline, L"/Enqueue"))
                pClsid = &CLSID_WindowsMediaPlayer_Enqueue;
            pMyCreateInstance = WMPServer_MyCreateInstance;

        } else if (ComServer_ImageType == ComServer_ImageType_WINAMP) {

            pClsid = &CLSID_WinAmp;
            pMyCreateInstance = WMPServer_MyCreateInstance;

        } else if (ComServer_ImageType == ComServer_ImageType_KMPLAYER) {

            pClsid = &CLSID_KmPlayer;
            pMyCreateInstance = WMPServer_MyCreateInstance;
        }

        hr_lvl = 2;
        hr = CoRegisterClassObject(
            pClsid, ComServer_MyClassFactory_New(pMyCreateInstance),
            CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE, &cookie);
    }

abort:

    if (FAILED(hr)) {
        SbieApi_Log(2314, L"%S [HR=%08X/%d]", pgm, hr, hr_lvl);
        ExitProcess(0);
    }

    //
    // message loop to handle COM messages
    //

#if 1
    SetTimer(NULL, 0, 2000, NULL);
#else
    while (! IsDebuggerPresent()) Sleep(500);
    __debugbreak();
#endif

    while (1) {
        BOOL b = GetMessage(&msg, 0, 0, 0);
        if (b == 0)
            break;
        if (b != -1) {
            if (msg.message == WM_TIMER) {
                if (ComServer_Object_RefCount == 0 &&
                    ComServer_Factory_RefCount <= 1 &&
                    ComServer_Factory_Locked == FALSE) {
#ifdef COMSERVER_DEBUG
                        OutputDebugString(L"End by WM_TIMER\n");
#endif
                        break;
                }
#ifdef COMSERVER_DEBUG
                        OutputDebugString(L"Ignoring WM_TIMER\n");
#endif
            }
            DispatchMessage(&msg);
        }
    }

#ifdef COMSERVER_DEBUG
    OutputDebugString(L"COM Server Shutting Down\n");
#endif

    //
    // done
    //

    CoRevokeClassObject(cookie);

    if (ComServer_CommandLine)
        return ComServer_CommandLine;

    ExitProcess(0);
}


//---------------------------------------------------------------------------
// ComServer_RestartProgram
//---------------------------------------------------------------------------


_FX void ComServer_RestartProgram(const WCHAR *arg)
{
    ULONG len = (wcslen(arg) + 1) * sizeof(WCHAR);
    WCHAR *cmd = Dll_Alloc(len);
    memcpy(cmd, arg, len);
    ComServer_CommandLine = cmd;

#ifdef COMSERVER_DEBUG
    OutputDebugString(L"ComServer Restart Commmand Line:\n");
    OutputDebugString(ComServer_CommandLine);
#endif

    PostQuitMessage(0);
}


//---------------------------------------------------------------------------
// ComServer_MyClassFactory_New
//---------------------------------------------------------------------------


_FX IUnknown *ComServer_MyClassFactory_New(
    P_MyCreateInstance pMyCreateInstance)
{
    IMyClassFactory *This;
    ULONG_PTR *ptr;

    This = Dll_Alloc(sizeof(IMyClassFactory));

    This->pMyCreateInstance = pMyCreateInstance;

    This->lpVtbl = &This->VtblSpace;
    ptr = (ULONG_PTR *)This->lpVtbl;
    ptr[0] = (ULONG_PTR)ComServer_IClassFactory_QueryInterface;
    ptr[1] = (ULONG_PTR)ComServer_IClassFactory_AddRef;
    ptr[2] = (ULONG_PTR)ComServer_IClassFactory_Release;
    ptr[3] = (ULONG_PTR)ComServer_IClassFactory_CreateInstance;
    ptr[4] = (ULONG_PTR)ComServer_IClassFactory_LockServer;

    return (IUnknown *)This;
}


//---------------------------------------------------------------------------
// ComServer_IClassFactory_QueryInterface
//---------------------------------------------------------------------------


_FX HRESULT ComServer_IClassFactory_QueryInterface(
    IClassFactory *This, REFIID riid, void **ppvObject)
{
#ifdef COMSERVER_DEBUG
    WCHAR txt[128];
    swprintf(txt, L"ComServer_IClassFactory_QueryInterface - %08X-%08X-%08X-%08X\n",
        ((ULONG *)riid)[0], ((ULONG *)riid)[1], ((ULONG *)riid)[2], ((ULONG *)riid)[3]);
    OutputDebugString(txt);
#endif

    if (! ppvObject)
        return E_POINTER;

    if (IsEqualIID(riid, &IID_IUnknown) ||
        IsEqualIID(riid, &IID_IClassFactory)) {

        InterlockedIncrement(&ComServer_Factory_RefCount);
        *ppvObject = This;
        return S_OK;
    }

    *ppvObject = NULL;
    return E_NOINTERFACE;
}


//---------------------------------------------------------------------------
// ComServer_IClassFactory_AddRef
//---------------------------------------------------------------------------


_FX ULONG ComServer_IClassFactory_AddRef(IClassFactory *This)
{
#ifdef COMSERVER_DEBUG
    WCHAR txt[128];
    swprintf(txt, L"ComServer_IClassFactory_AddRef - %d\n", ComServer_Factory_RefCount + 1);
    OutputDebugString(txt);
#endif

    InterlockedIncrement(&ComServer_Factory_RefCount);
    return ComServer_Factory_RefCount;
}


//---------------------------------------------------------------------------
// ComServer_IClassFactory_Release
//---------------------------------------------------------------------------


_FX ULONG ComServer_IClassFactory_Release(IClassFactory *This)
{
#ifdef COMSERVER_DEBUG
    WCHAR txt[128];
    swprintf(txt, L"ComServer_IClassFactory_Release - %d\n", ComServer_Factory_RefCount - 1);
    OutputDebugString(txt);
#endif

    InterlockedDecrement(&ComServer_Factory_RefCount);
    return ComServer_Factory_RefCount;
}


//---------------------------------------------------------------------------
// ComServer_IClassFactory_CreateInstance
//---------------------------------------------------------------------------


_FX HRESULT ComServer_IClassFactory_CreateInstance(
    IClassFactory *This, IUnknown *pUnkOuter, REFIID riid, void **ppvObject)
{
    HRESULT hr;
    IUnknown *obj;

#ifdef COMSERVER_DEBUG
    WCHAR txt[128];
    swprintf(txt, L"ComServer_IClassFactory_CreateInstance - %08X-%08X-%08X-%08X (pUnkOuter=%08X)\n",
        ((ULONG *)riid)[0], ((ULONG *)riid)[1], ((ULONG *)riid)[2], ((ULONG *)riid)[3], pUnkOuter);
    OutputDebugString(txt);
#endif

    if (! ppvObject)
        return E_POINTER;
    if (pUnkOuter) {
        *ppvObject = NULL;
        return CLASS_E_NOAGGREGATION;
    }

    obj = (IUnknown *)(((IMyClassFactory *)This)->pMyCreateInstance(riid));

    if (obj) {

        hr = IUnknown_QueryInterface(obj, riid, ppvObject);

        if (FAILED(hr)) {
            //Dll_Free(obj);
            *ppvObject = NULL;
        }

    } else
        hr = E_NOINTERFACE;

    return S_OK;
}


//---------------------------------------------------------------------------
// ComServer_IClassFactory_LockServer
//---------------------------------------------------------------------------


_FX HRESULT ComServer_IClassFactory_LockServer(
    IClassFactory *This, BOOL fLock)
{
    ComServer_Factory_Locked = fLock;
    return S_OK;
}


//---------------------------------------------------------------------------
// ComServer_MyUnknown_New
//---------------------------------------------------------------------------


_FX IMyUnknown *ComServer_MyUnknown_New(
    P_MyQueryInterface pMyQueryInterface, ULONG SizeofVtbls)
{
    ULONG len;
    IMyUnknown *This;
    ULONG_PTR *ptr;

    len = sizeof(IMyUnknown) + SizeofVtbls;
    This = Dll_Alloc(len);
    memzero(This, len);

    This->pMyQueryInterface = pMyQueryInterface;

    ptr = (ULONG_PTR *)&This->IUnknownVtbl;
    *ptr = (ULONG_PTR)This;                         // pBackPointerToObject
    ++ptr;
    This->Vtbls[0] = (ULONG_PTR)ptr;
    ptr[0] = (ULONG_PTR)ComServer_IUnknown_QueryInterface;
    ptr[1] = (ULONG_PTR)ComServer_IUnknown_AddRef;
    ptr[2] = (ULONG_PTR)ComServer_IUnknown_Release;
    ptr += sizeof(IUnknownVtbl) / sizeof(ULONG_PTR);

    return This;
}


//---------------------------------------------------------------------------
// ComServer_IUnknown_QueryInterface
//---------------------------------------------------------------------------


_FX HRESULT ComServer_IUnknown_QueryInterface(
    IUnknown *This, REFIID riid, void **ppvObject)
{
    void *xThis = *(ULONG_PTR **)((*(ULONG_PTR **)This) - 1);
    IMyUnknown *MyThis = (IMyUnknown *)xThis;

#ifdef COMSERVER_DEBUG
    WCHAR txt[128];
    swprintf(txt, L"ComServer_IUnknown_QueryInterface - %08X-%08X-%08X-%08X\n",
        ((ULONG *)riid)[0], ((ULONG *)riid)[1], ((ULONG *)riid)[2], ((ULONG *)riid)[3]);
    OutputDebugString(txt);
#endif

    if (! ppvObject)
        return E_POINTER;

    if (IsEqualIID(riid, &IID_IUnknown)) {

        *ppvObject = (void *)&MyThis->Vtbls[0];

    } else {

        *ppvObject = MyThis->pMyQueryInterface(MyThis, riid);
    }

    if (*ppvObject) {

        InterlockedIncrement(&ComServer_Object_RefCount);
        return S_OK;

    } else {

#ifdef COMSERVER_DEBUG
        OutputDebugString(L"ComServer_IUnknown_QueryInterface - E_NOINTERFACE\n");
#endif
        return E_NOINTERFACE;
    }
}


//---------------------------------------------------------------------------
// ComServer_IUnknown_AddRef
//---------------------------------------------------------------------------


_FX ULONG ComServer_IUnknown_AddRef(IUnknown *This)
{
#ifdef COMSERVER_DEBUG
    WCHAR txt[128];
    swprintf(txt, L"ComServer_IUnknown_AddRef - %d\n", ComServer_Object_RefCount + 1);
    OutputDebugString(txt);
#endif

    InterlockedIncrement(&ComServer_Object_RefCount);
    return ComServer_Object_RefCount;
}


//---------------------------------------------------------------------------
// ComServer_IUnknown_Release
//---------------------------------------------------------------------------


_FX ULONG ComServer_IUnknown_Release(IUnknown *This)
{
#ifdef COMSERVER_DEBUG
    WCHAR txt[128];
    swprintf(txt, L"ComServer_IUnknown_Release - %d\n", ComServer_Object_RefCount - 1);
    OutputDebugString(txt);
#endif

    InterlockedDecrement(&ComServer_Object_RefCount);
    return ComServer_Object_RefCount;
}


//---------------------------------------------------------------------------
// Dll_Alloc
//---------------------------------------------------------------------------


_FX void *Dll_Alloc(ULONG size)
{
    return HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, size);
}
