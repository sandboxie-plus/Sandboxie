/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020 David Xanatos, xanasoft.com
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
// Shell (Taskbar)
//---------------------------------------------------------------------------

#include "dll.h"

#include <windows.h>
#include <stdio.h>
#include "dll.h"
#include "taskbar.h"
#define PROPSYS_INITGUID
#include "propsys.h"
#include "common/my_version.h"


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _TASKBAR_UNKNOWN {

    void *pVtbl;
    IUnknown *pReal;
    volatile ULONG RefCount;
    void *Vtbl[3];          // QueryInterface, AddRef, Release, ...

} TASKBAR_UNKNOWN;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN Taskbar_Init_2(HMODULE module);

static BOOLEAN Taskbar_ShouldOverrideAppUserModelId(void);

static HRESULT Taskbar_SetCurrentProcessExplicitAppUserModelID(
    const WCHAR *AppId);

static HRESULT Taskbar_GetCurrentProcessExplicitAppUserModelID(
    WCHAR **AppId);

static WCHAR *Taskbar_CreateAppUserModelId(const WCHAR *AppId);

static void Taskbar_GetProgramName(WCHAR **name, WCHAR **path);

static WCHAR *Taskbar_GetStartExeCommand(const WCHAR *args);

static WCHAR *Taskbar_GetNameWithBoxPrefix(const WCHAR *name);

static WCHAR *Taskbar_AllocBSTR(const WCHAR *model);

static void Taskbar_FreeBSTR(WCHAR *bstr);

static TASKBAR_UNKNOWN *Taskbar_AllocUnknown(ULONG methods, IUnknown *pReal);

static HRESULT Taskbar_Unknown_QueryInterface(
    IPropertyStore *This, REFIID riid, void **ppv);

static ULONG Taskbar_Unknown_AddRef(IPropertyStore *This);

static ULONG Taskbar_Unknown_Release(IPropertyStore *This);

static HRESULT Taskbar_SHGetPropertyStoreForWindow(
    HWND hwnd, REFIID riid, void **ppv);

static HRESULT Taskbar_IPropertyStore_GetCount(
    IPropertyStore *This, DWORD *cProps);

static HRESULT Taskbar_IPropertyStore_GetAt(
    IPropertyStore *This, DWORD iProp, PROPERTYKEY *pkey);

static HRESULT Taskbar_IPropertyStore_GetValue(
    IPropertyStore *This, REFPROPERTYKEY key, PROPVARIANT *pv);

static HRESULT Taskbar_IPropertyStore_SetValue(
    IPropertyStore *This, REFPROPERTYKEY key, REFPROPVARIANT propvar);

static HRESULT Taskbar_IPropertyStore_Commit(IPropertyStore *This);


//---------------------------------------------------------------------------


typedef HRESULT (*P_SetCurrentProcessExplicitAppUserModelID)(
    const WCHAR *AppId);

typedef HRESULT (*P_GetCurrentProcessExplicitAppUserModelID)(
    WCHAR **AppId);

typedef HRESULT (*P_SHGetPropertyStoreForWindow)(
    HWND hwnd, REFIID riid, void **ppv);

typedef HRESULT (*P_SHGetKnownFolderPath)(
    const GUID *folderid, DWORD dwFlags, HANDLE hToken, PWSTR *ppszPath);

typedef void *(*P_CoTaskMemAlloc)(ULONG cb);

typedef void (*P_CoTaskMemFree)(void *pv);


//---------------------------------------------------------------------------


static P_SetCurrentProcessExplicitAppUserModelID
                        __sys_SetCurrentProcessExplicitAppUserModelID = NULL;

static P_GetCurrentProcessExplicitAppUserModelID
                        __sys_GetCurrentProcessExplicitAppUserModelID = NULL;

static P_SHGetPropertyStoreForWindow
                        __sys_SHGetPropertyStoreForWindow = NULL;

extern P_CoTaskMemAlloc __sys_CoTaskMemAlloc;           // from com.c


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static WCHAR *Taskbar_BoxPrefix = NULL;

static ULONG Taskbar_BoxPrefix_Len = 0;

static WCHAR *Taskbar_SavedAppUserModelId = NULL;


//---------------------------------------------------------------------------
// GUIDs
//---------------------------------------------------------------------------


static const GUID IID_IPropertyStore = {
    0x886D8EEB, 0x8CF2, 0x4446,
        { 0x8D, 0x02, 0xCD, 0xBA, 0x1D, 0xBD, 0xCF, 0x99 } };

static const GUID FOLDERID_Recent = {
    0xAE50C081, 0xEBD2, 0x438A,
        { 0x86, 0x55, 0x8A, 0x09, 0x2E, 0x34, 0x98, 0x7A } };

/*static const GUID IID_ICustomDestinationList = {
    0x6332DEBF, 0x87B5, 0x4670,
        { 0x90, 0xC0, 0x5E, 0x57, 0xB4, 0x08, 0xA4, 0x9E } };

EXTERN_C const CLSID CLSID_EnumerableObjectCollection;*/


//---------------------------------------------------------------------------
// Taskbar_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Taskbar_Init(HMODULE module)
{
    static const WCHAR *Sandbox = L"Sandbox";
    //WCHAR Version[16];
    ULONG len;

    P_SHGetPropertyStoreForWindow SHGetPropertyStoreForWindow;

    if (Dll_OsBuild < 7600)
        return TRUE;

    //
    // initialize BoxPrefix variables
    //

    //if (SbieApi_GetVersion(Version) == 0) {
    //    while (1) {
    //        WCHAR *ptr = wcschr(Version, L'.');
    //        if (! ptr)
    //            break;
    //        *ptr = L'x';
    //    }
    //} else
    //    *Version = L'\0';

#ifdef CUSTOM_APPUSERMODELID
    SbieApi_QueryConf(
                NULL, L"TaskbarUnique", 0, Version, 14 * sizeof(WCHAR));
#endif CUSTOM_APPUSERMODELID

    len = (wcslen(Sandbox) + wcslen(Dll_BoxName) + 32) * sizeof(WCHAR);
    Taskbar_BoxPrefix = Dll_Alloc(len);
    //Sbie_snwprintf(Taskbar_BoxPrefix, len / sizeof(WCHAR), L"%s%s.%s.", Sandbox, Version, Dll_BoxName);
	Sbie_snwprintf(Taskbar_BoxPrefix, len / sizeof(WCHAR), L"%s.%s.", Sandbox, Dll_BoxName);
    /*Sbie_snwprintf(Taskbar_BoxPrefix, len / sizeof(WCHAR), L"%s.%s.%08X.",
                                Sandbox, Dll_BoxName, GetTickCount());*/
    Taskbar_BoxPrefix_Len = wcslen(Taskbar_BoxPrefix);

    //
    // hook taskbar services
    //

    SHGetPropertyStoreForWindow =
        (P_SHGetPropertyStoreForWindow) GetProcAddress(
            module, "SHGetPropertyStoreForWindow");

    if (Dll_OsBuild < 7600) {
        if (! Taskbar_Init_2(module))   // module == shell32
            return FALSE;
    }

    if (SHGetPropertyStoreForWindow) {
        SBIEDLL_HOOK(Taskbar_,SHGetPropertyStoreForWindow);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Taskbar_Init_2
//---------------------------------------------------------------------------


_FX BOOLEAN Taskbar_Init_2(HMODULE module)
{
    P_SetCurrentProcessExplicitAppUserModelID
        SetCurrentProcessExplicitAppUserModelID;
    P_GetCurrentProcessExplicitAppUserModelID
        GetCurrentProcessExplicitAppUserModelID;

    SetCurrentProcessExplicitAppUserModelID =
        (P_SetCurrentProcessExplicitAppUserModelID) GetProcAddress(
            module, "SetCurrentProcessExplicitAppUserModelID");

    GetCurrentProcessExplicitAppUserModelID =
        (P_GetCurrentProcessExplicitAppUserModelID) GetProcAddress(
            module, "GetCurrentProcessExplicitAppUserModelID");

    if (SetCurrentProcessExplicitAppUserModelID) {
        SBIEDLL_HOOK(Taskbar_,SetCurrentProcessExplicitAppUserModelID);
    }

    if (GetCurrentProcessExplicitAppUserModelID) {
        SBIEDLL_HOOK(Taskbar_,GetCurrentProcessExplicitAppUserModelID);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Taskbar_SHCore_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Taskbar_SHCore_Init(HMODULE module)
{
    //
    // Windows 8 has GetCurrentProcessExplicitAppUserModelID and
    // SetCurrentProcessExplicitAppUserModelID in a separate SHCORE.DLL
    // rather than in SHELL32.DLL as in Windows 7
    //

    if (Dll_OsBuild >= 8400) {
        Taskbar_Init_2(module);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Taskbar_ShouldOverrideAppUserModelId
//---------------------------------------------------------------------------


_FX BOOLEAN Taskbar_ShouldOverrideAppUserModelId(void)
{
    static BOOLEAN _checked = FALSE;
    static BOOLEAN _dohook = FALSE;

    const ULONG KF_FLAG_DONT_VERIFY = 0x00004000;
    const ULONG KF_FLAG_DONT_UNEXPAND = 0x00002000;

    P_SHGetKnownFolderPath SHGetKnownFolderPath;
    WCHAR *path;

    //
    //
    //

    if (_checked || (! Dll_InitComplete))
        return _dohook;

    _checked = TRUE;

    if (Dll_ProcessFlags & SBIE_FLAG_FORCED_PROCESS)
        return FALSE;

    if (Dll_ImageType == DLL_IMAGE_INTERNET_EXPLORER) {
        ULONG len;
        PROCESS_BASIC_INFORMATION info;
        if (0 == NtQueryInformationProcess(
                    NtCurrentProcess(), ProcessBasicInformation,
                    &info, sizeof(PROCESS_BASIC_INFORMATION), &len)) {
            ULONG64 ParentProcessFlags = SbieApi_QueryProcessInfo(
                        (HANDLE)info.InheritedFromUniqueProcessId, 0);
            if (ParentProcessFlags & SBIE_FLAG_FORCED_PROCESS)
                return FALSE;
        }
    }

    _dohook = TRUE;

    //
    //
    //

    SHGetKnownFolderPath = (P_SHGetKnownFolderPath) Ldr_GetProcAddrNew(DllName_shell32, L"SHGetKnownFolderPath","SHGetKnownFolderPath");

    if (! SHGetKnownFolderPath)
        path = NULL;
    else if (SHGetKnownFolderPath(&FOLDERID_Recent,
                        KF_FLAG_DONT_VERIFY | KF_FLAG_DONT_UNEXPAND,
                        NULL, &path) != 0)
        path = NULL;

    if (path) {

        WCHAR *NtPath;
        WCHAR *DosPath = Dll_AllocTemp((wcslen(path) + 64) * sizeof(WCHAR));
        wcscpy(DosPath, path);
        wcscat(DosPath, L"\\CustomDestinations\\x.customDestinations-ms");
        NtPath = File_TranslateDosToNtPath(DosPath);
        if (NtPath) {

            ULONG mp_flags = SbieDll_MatchPath(L'f', NtPath);
            if (PATH_IS_OPEN(mp_flags)) {

                _dohook = FALSE;
            }

            Dll_Free(NtPath);
        }
        Dll_Free(DosPath);
    }

    if (path) {

        P_CoTaskMemFree CoTaskMemFree = (P_CoTaskMemFree)
            Ldr_GetProcAddrNew(DllName_ole32_or_combase, L"CoTaskMemFree","CoTaskMemFree");

        if (CoTaskMemFree)
            CoTaskMemFree(path);
    }

    return _dohook;
}


//---------------------------------------------------------------------------
// Taskbar_SetCurrentProcessExplicitAppUserModelID
//---------------------------------------------------------------------------


_FX HRESULT Taskbar_SetCurrentProcessExplicitAppUserModelID(
    const WCHAR *AppId)
{
    HRESULT hr;
    WCHAR *NewId;

    if (! Taskbar_ShouldOverrideAppUserModelId())
        return __sys_SetCurrentProcessExplicitAppUserModelID(AppId);

    if (Taskbar_SavedAppUserModelId) {
        Dll_Free(Taskbar_SavedAppUserModelId);
        Taskbar_SavedAppUserModelId = NULL;
    }

    if (AppId) {

        ULONG len = wcslen(AppId);
        Taskbar_SavedAppUserModelId = Dll_Alloc((len + 1) * sizeof(WCHAR));
        wmemcpy(Taskbar_SavedAppUserModelId, AppId, len + 1);

        NewId = Taskbar_CreateAppUserModelId(AppId);

    } else
        NewId = NULL;

    hr = __sys_SetCurrentProcessExplicitAppUserModelID(NewId);

    if (NewId && NewId != AppId)
        Dll_Free(NewId);

    return hr;
}


//---------------------------------------------------------------------------
// Taskbar_GetCurrentProcessExplicitAppUserModelID
//---------------------------------------------------------------------------


_FX HRESULT Taskbar_GetCurrentProcessExplicitAppUserModelID(WCHAR **AppId)
{
    HRESULT hr;

    if (! Taskbar_ShouldOverrideAppUserModelId())
        return __sys_GetCurrentProcessExplicitAppUserModelID(AppId);

    hr = E_FAIL;

    if (Taskbar_SavedAppUserModelId && __sys_CoTaskMemAlloc) {
        ULONG len = wcslen(Taskbar_SavedAppUserModelId);
        WCHAR *copy = __sys_CoTaskMemAlloc((len + 1) * sizeof(WCHAR));
        if (copy) {
            wmemcpy(copy, Taskbar_SavedAppUserModelId, len + 1);
            *AppId = copy;
            hr = S_OK;
        }
    }

    return hr;
}


//---------------------------------------------------------------------------
// Taskbar_CreateAppUserModelId
//---------------------------------------------------------------------------


_FX WCHAR *Taskbar_CreateAppUserModelId(const WCHAR *AppId)
{
    int len;
    WCHAR *NewId;

    if (wcsncmp(AppId, Taskbar_BoxPrefix, Taskbar_BoxPrefix_Len) == 0)
        return (WCHAR *)AppId;

    len = (Taskbar_BoxPrefix_Len + wcslen(AppId) + 1) * sizeof(WCHAR);
    NewId = Dll_Alloc(len);
    if (NewId) {

        WCHAR *ptr = NewId;
        wmemcpy(ptr, Taskbar_BoxPrefix, Taskbar_BoxPrefix_Len);
        ptr += Taskbar_BoxPrefix_Len;
        while (*AppId) {
            if (*AppId == L'.')
                *ptr = L'_';
            else
                *ptr = *AppId;
            ++AppId;
            ++ptr;
        }
        *ptr = L'\0';
    }

    return NewId;
}


//---------------------------------------------------------------------------
// Taskbar_SetProcessAppUserModelId
//---------------------------------------------------------------------------

//extern ULONG Dll_Windows;
_FX void Taskbar_SetProcessAppUserModelId(void)
{
    static BOOLEAN _done = FALSE;

    P_SetCurrentProcessExplicitAppUserModelID
        SetCurrentProcessExplicitAppUserModelID;

    if ((Dll_OsBuild < 7600) || (! Dll_InitComplete))
        return;

    if (Taskbar_SavedAppUserModelId || _done)
        return;

    //
    // make sure the necessary function from shell32 is available
    //
//  if(Dll_Windows < 10) {
    SetCurrentProcessExplicitAppUserModelID =
        (P_SetCurrentProcessExplicitAppUserModelID) Ldr_GetProcAddrNew(DllName_shell32, L"SetCurrentProcessExplicitAppUserModelID","SetCurrentProcessExplicitAppUserModelID");

    //  }
    /*
    else {

    SetCurrentProcessExplicitAppUserModelID = (P_SetCurrentProcessExplicitAppUserModelID)
        GetProcAddress(LoadLibraryW(DllName_shell32),"SetCurrentProcessExplicitAppUserModelID");
    }
    */
    if (! SetCurrentProcessExplicitAppUserModelID)
        return;

    //
    // override AppUserModelId
    //

    _done = TRUE;

    if (Taskbar_ShouldOverrideAppUserModelId()) {

        SetCurrentProcessExplicitAppUserModelID(Dll_ImageName);
    }
}


//---------------------------------------------------------------------------
// Taskbar_SetWindowAppUserModelId
//---------------------------------------------------------------------------


_FX void Taskbar_SetWindowAppUserModelId(HWND hwnd)
{
    IPropertyStore *pPropertyStore;
    const WCHAR *AppId;
    WCHAR *ExeDescr, *ExePath;
    PROPVARIANT v;
    HRESULT hr;

    P_SHGetPropertyStoreForWindow SHGetPropertyStoreForWindow;

    if ((Dll_OsBuild < 7600) || (! Dll_InitComplete))
        return;

    //
    // make sure the necessary function from shell32 is available
    //

    SHGetPropertyStoreForWindow =
        (P_SHGetPropertyStoreForWindow) Ldr_GetProcAddrNew(
            DllName_shell32, L"SHGetPropertyStoreForWindow","SHGetPropertyStoreForWindow");

    if (! SHGetPropertyStoreForWindow)
        return;

    if (! Taskbar_ShouldOverrideAppUserModelId())
        return;

    //
    // get the property store interface for the window
    //

    hr = SHGetPropertyStoreForWindow(
                hwnd, &IID_IPropertyStore, &pPropertyStore);

    if (! SUCCEEDED(hr))
        return;

    //
    // set explicit AppUserModelID for the window
    //

    AppId = Taskbar_SavedAppUserModelId;
    if (! AppId)
        AppId = Dll_ImageName;

    v.vt = VT_BSTR;
    v.bstrVal = Taskbar_AllocBSTR(AppId);
    pPropertyStore->lpVtbl->SetValue(
        pPropertyStore, &PKEY_AppUserModel_ID, &v);
    Taskbar_FreeBSTR(v.bstrVal);

    //
    // override display name
    //

    Taskbar_GetProgramName(&ExeDescr, &ExePath);

    if (ExeDescr) {

        v.vt = VT_BSTR;
        v.bstrVal = Taskbar_AllocBSTR(ExeDescr);
        pPropertyStore->lpVtbl->SetValue(
            pPropertyStore,
            &PKEY_AppUserModel_RelaunchDisplayNameResource, &v);
        Taskbar_FreeBSTR(v.bstrVal);
    }

    //
    // override command line
    //

    if (ExePath) {

        v.vt = VT_BSTR;
        v.bstrVal = Taskbar_AllocBSTR(ExePath);
        pPropertyStore->lpVtbl->SetValue(
            pPropertyStore, &PKEY_AppUserModel_RelaunchCommand, &v);
        Taskbar_FreeBSTR(v.bstrVal);
    }

    //
    // override icon
    //

    if (ExePath) {

        WCHAR *IconPath =
            Dll_AllocTemp((wcslen(ExePath) + 8) * sizeof(WCHAR));
        wcscpy(IconPath, ExePath);
        wcscat(IconPath, L",-0");

        v.vt = VT_BSTR;
        v.bstrVal = Taskbar_AllocBSTR(IconPath);
        pPropertyStore->lpVtbl->SetValue(
            pPropertyStore, &PKEY_AppUserModel_RelaunchIconResource, &v);
        Taskbar_FreeBSTR(v.bstrVal);

        Dll_Free(IconPath);
    }

    //
    // cleanup
    //

    pPropertyStore->lpVtbl->Release(pPropertyStore);
}


//---------------------------------------------------------------------------
// Taskbar_GetProgramName
//---------------------------------------------------------------------------


_FX void Taskbar_GetProgramName(WCHAR **name, WCHAR **path)
{
    static WCHAR *_name = NULL;
    static WCHAR *_path = NULL;

    const ULONG ASSOCF_INIT_BYEXENAME = 2;
    const ULONG ASSOCSTR_FRIENDLYAPPNAME = 4;
    typedef HRESULT (*P_AssocQueryString)(
        ULONG_PTR flags, ULONG_PTR str, LPCTSTR pszAssoc,
        LPCTSTR pszExtra, LPTSTR pszOut, DWORD *pcchOut);
    P_AssocQueryString AssocQueryString;
    ULONG len;

    HANDLE hFile;

    *name = _name;
    *path = _path;
    if (_name)
        return;

    AssocQueryString = (P_AssocQueryString)
        Ldr_GetProcAddrNew(L"shlwapi.dll", L"AssocQueryStringW","AssocQueryStringW");
    if (! AssocQueryString)
        return;

    _path = Dll_Alloc((wcslen(Ldr_ImageTruePath) + 4) * sizeof(WCHAR));
    wcscpy(_path, Ldr_ImageTruePath);
    SbieDll_TranslateNtToDosPath(_path);

    hFile = CreateFile(_path, GENERIC_READ, FILE_SHARE_VALID_FLAGS,
                       NULL, OPEN_EXISTING, 0, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        BOOLEAN IsBoxedPath;
        WCHAR *path1 = Dll_Alloc(8192);
        NTSTATUS status = SbieDll_GetHandlePath(hFile, path1, &IsBoxedPath);
        if (NT_SUCCESS(status) && IsBoxedPath) {
            SbieDll_TranslateNtToDosPath(path1);
            Dll_Free(_path);
            _path = path1;
        } else
            Dll_Free(path1);
        CloseHandle(hFile);
    }

    len = MAX_PATH;
    _name = Dll_Alloc((len + 8) * sizeof(WCHAR));
    if (0 != AssocQueryString(
                ASSOCF_INIT_BYEXENAME, ASSOCSTR_FRIENDLYAPPNAME,
                _path, NULL, _name, &len)) {

        wcscpy(_name, Dll_ImageName);
    }

    *name = _name;
    *path = _path;
}


//---------------------------------------------------------------------------
// Taskbar_GetStartExeCommand
//---------------------------------------------------------------------------


_FX WCHAR *Taskbar_GetStartExeCommand(const WCHAR *args)
{
    ULONG len = (wcslen(args) + wcslen(Dll_BoxName) + MAX_PATH + 128);
    WCHAR *command = Dll_AllocTemp(len * sizeof(WCHAR));
    command[0] = L'\"';

#ifdef CUSTOM_APPUSERMODELID

    SbieApi_QueryConf(
        NULL, L"TaskbarProgram", 0, command + 1, MAX_PATH * sizeof(WCHAR));
    SbieDll_TranslateNtToDosPath(command + 1);
    wcscat(command, L"\" ");

#else ! CUSTOM_APPUSERMODELID

    SbieApi_GetHomePath(NULL, 0, &command[1], MAX_PATH);
    wcscat(command, L"\\" START_EXE L"\" /box:");
    wcscat(command, Dll_BoxName);
    wcscat(command, L" ");

#endif CUSTOM_APPUSERMODELID

    while (*args == L' ')
        ++args;
    if (*args == L'\"')
        wcscat(command, args);
    else {
        wcscat(command, L"\"");
        wcscat(command, args);
        wcscat(command, L"\"");
    }

    return command;
}


//---------------------------------------------------------------------------
// Taskbar_GetNameWithBoxPrefix
//---------------------------------------------------------------------------


_FX WCHAR *Taskbar_GetNameWithBoxPrefix(const WCHAR *name)
{
#ifdef CUSTOM_APPUSERMODELID
    ULONG len = wcslen(name) + MAX_PATH;
    WCHAR *out = Dll_AllocTemp(len * sizeof(WCHAR));
    out[0] = '[';

    SbieApi_QueryConf(
        NULL, L"TaskbarName", 0, out + 1, MAX_PATH * sizeof(WCHAR));
    wcscat(out, L"] ");
    wcscat(out, name);

#else ! CUSTOM_APPUSERMODELID
    ULONG len = wcslen(Dll_BoxName) + wcslen(name) + 8;
    WCHAR *out = Dll_AllocTemp(len * sizeof(WCHAR));
    Sbie_snwprintf(out, len, L"[%s] %s", Dll_BoxName, name);
#endif CUSTOM_APPUSERMODELID

    return out;
}


//---------------------------------------------------------------------------
// Taskbar_CoCreateInstance
//---------------------------------------------------------------------------


/*_FX void Taskbar_CoCreateInstance(const void *riid, void **ppv)
{
    if (memcmp(riid, &IID_ICustomDestinationList, sizeof(GUID)) == 0) {
        if (IsDebuggerPresent()) __debugbreak();
    }
}*/


//---------------------------------------------------------------------------
// Taskbar_AllocBSTR
//---------------------------------------------------------------------------


_FX WCHAR *Taskbar_AllocBSTR(const WCHAR *model)
{
    ULONG len = wcslen(model);
    WCHAR *result = Dll_Alloc(sizeof(ULONG) + (len + 1) * sizeof(WCHAR));
    wmemcpy(result + 2, model, len + 1);
    *(ULONG *)result = len * sizeof(WCHAR);
    return (WCHAR *)(result + 2);
}


//---------------------------------------------------------------------------
// Taskbar_FreeBSTR
//---------------------------------------------------------------------------


_FX void Taskbar_FreeBSTR(WCHAR *bstr)
{
    Dll_Free(bstr - 2);
}


//---------------------------------------------------------------------------
// Taskbar_AllocUnknown
//---------------------------------------------------------------------------


_FX TASKBAR_UNKNOWN *Taskbar_AllocUnknown(ULONG methods, IUnknown *pReal)
{
    TASKBAR_UNKNOWN *This;
    ULONG len;

    len = sizeof(TASKBAR_UNKNOWN) + sizeof(ULONG_PTR) * methods;
    This = Dll_Alloc(len);

    This->pVtbl = This->Vtbl;
    This->pReal = pReal;
    This->RefCount = 1;
    This->Vtbl[0] = Taskbar_Unknown_QueryInterface;
    This->Vtbl[1] = Taskbar_Unknown_AddRef;
    This->Vtbl[2] = Taskbar_Unknown_Release;

    return This;
}


//---------------------------------------------------------------------------
// Taskbar_Unknown_QueryInterface
//---------------------------------------------------------------------------


_FX HRESULT Taskbar_Unknown_QueryInterface(
    IPropertyStore *This, REFIID riid, void **ppv)
{
    if (memcmp(riid, &IID_IPropertyStore, sizeof(GUID)) != 0) {
        SbieApi_Log(2205, L"SHGetPropertyStoreForWindow");
        return E_NOINTERFACE;
    }
    This->lpVtbl->AddRef(This);
    *ppv = This;
    return S_OK;
}


//---------------------------------------------------------------------------
// Taskbar_Unknown_AddRef
//---------------------------------------------------------------------------


_FX ULONG Taskbar_Unknown_AddRef(IPropertyStore *This)
{
    TASKBAR_UNKNOWN *pThis = (TASKBAR_UNKNOWN *)This;
    IPropertyStore *pReal = (IPropertyStore *)pThis->pReal;

    pReal->lpVtbl->AddRef(pReal);
    return InterlockedIncrement(&pThis->RefCount);
}


//---------------------------------------------------------------------------
// Taskbar_Unknown_Release
//---------------------------------------------------------------------------


_FX ULONG Taskbar_Unknown_Release(IPropertyStore *This)
{
    TASKBAR_UNKNOWN *pThis = (TASKBAR_UNKNOWN *)This;
    IPropertyStore *pReal = (IPropertyStore *)pThis->pReal;
    ULONG refcount;

    pReal->lpVtbl->Release(pReal);
    refcount = InterlockedDecrement(&pThis->RefCount);
    if (! refcount)
        Dll_Free(This);
    return refcount;
}


//---------------------------------------------------------------------------
// Taskbar_SHGetPropertyStoreForWindow
//---------------------------------------------------------------------------


HRESULT Taskbar_SHGetPropertyStoreForWindow(
    HWND hwnd, REFIID riid, void **ppv)
{
    TASKBAR_UNKNOWN *pMyStore;

    HRESULT hr = __sys_SHGetPropertyStoreForWindow(hwnd, riid, ppv);
    if (FAILED(hr) || (! Taskbar_ShouldOverrideAppUserModelId()))
        return hr;
    if (memcmp(riid, &IID_IPropertyStore, sizeof(GUID)) != 0) {
        SbieApi_Log(2205, L"SHGetPropertyStoreForWindow");
        return hr;
    }

    pMyStore = Taskbar_AllocUnknown(5, *(IUnknown **)ppv);
    if (pMyStore) {

        pMyStore->Vtbl[3] = Taskbar_IPropertyStore_GetCount;
        pMyStore->Vtbl[4] = Taskbar_IPropertyStore_GetAt;
        pMyStore->Vtbl[5] = Taskbar_IPropertyStore_GetValue;
        pMyStore->Vtbl[6] = Taskbar_IPropertyStore_SetValue;
        pMyStore->Vtbl[7] = Taskbar_IPropertyStore_Commit;

        *(IPropertyStore **)ppv = (IPropertyStore *)pMyStore;
    }

    return hr;
}


//---------------------------------------------------------------------------
// Taskbar_IPropertyStore_GetCount
//---------------------------------------------------------------------------


_FX HRESULT Taskbar_IPropertyStore_GetCount(
    IPropertyStore *This, DWORD *cProps)
{
    TASKBAR_UNKNOWN *pThis = (TASKBAR_UNKNOWN *)This;
    IPropertyStore *pReal = (IPropertyStore *)pThis->pReal;

    return pReal->lpVtbl->GetCount(pReal, cProps);
}


//---------------------------------------------------------------------------
// Taskbar_IPropertyStore_GetAt
//---------------------------------------------------------------------------


_FX HRESULT Taskbar_IPropertyStore_GetAt(
    IPropertyStore *This, DWORD iProp, PROPERTYKEY *pkey)
{
    TASKBAR_UNKNOWN *pThis = (TASKBAR_UNKNOWN *)This;
    IPropertyStore *pReal = (IPropertyStore *)pThis->pReal;

    return pReal->lpVtbl->GetAt(pReal, iProp, pkey);
}


//---------------------------------------------------------------------------
// Taskbar_IPropertyStore_GetValue
//---------------------------------------------------------------------------


_FX HRESULT Taskbar_IPropertyStore_GetValue(
    IPropertyStore *This, REFPROPERTYKEY key, PROPVARIANT *pv)
{
    TASKBAR_UNKNOWN *pThis = (TASKBAR_UNKNOWN *)This;
    IPropertyStore *pReal = (IPropertyStore *)pThis->pReal;

    return pReal->lpVtbl->GetValue(pReal, key, pv);
}


//---------------------------------------------------------------------------
// Taskbar_IPropertyStore_SetValue
//---------------------------------------------------------------------------


_FX HRESULT Taskbar_IPropertyStore_SetValue(
    IPropertyStore *This, REFPROPERTYKEY key, REFPROPVARIANT propvar)
{
    TASKBAR_UNKNOWN *pThis = (TASKBAR_UNKNOWN *)This;
    IPropertyStore *pReal = (IPropertyStore *)pThis->pReal;

    //
    // replace caller-specified value as appropriate
    //

    WCHAR *NewValue = NULL;

    if (memcmp(key, &PKEY_AppUserModel_ID, sizeof(PROPERTYKEY)) == 0) {

        const WCHAR *AppId = Dll_ImageName;
        if (propvar->vt == VT_BSTR && propvar->bstrVal)
            AppId = propvar->bstrVal;
        NewValue = Taskbar_CreateAppUserModelId(AppId);
        if (NewValue == AppId)
            NewValue = NULL;

    } else if (memcmp(key, &PKEY_AppUserModel_RelaunchCommand,
                                            sizeof(PROPERTYKEY)) == 0) {

        if (propvar->vt == VT_BSTR && propvar->bstrVal)
            NewValue = Taskbar_GetStartExeCommand(propvar->bstrVal);
        else
            return S_OK;

    } else if (memcmp(key, &PKEY_AppUserModel_RelaunchDisplayNameResource,
                                            sizeof(PROPERTYKEY)) == 0) {

        if (propvar->vt == VT_BSTR && propvar->bstrVal)
            NewValue = Taskbar_GetNameWithBoxPrefix(propvar->bstrVal);
        else
            return S_OK;
    }

    //
    // set the property on the window
    //

    if (NewValue) {

        PROPVARIANT v;
        HRESULT hr;

        v.vt = VT_BSTR;
        v.bstrVal = Taskbar_AllocBSTR(NewValue);
        hr = pReal->lpVtbl->SetValue(pReal, key, &v);

        Taskbar_FreeBSTR(v.bstrVal);
        Dll_Free(NewValue);

        return hr;

    } else {

        return pReal->lpVtbl->SetValue(pReal, key, propvar);
    }
}


//---------------------------------------------------------------------------
// Taskbar_IPropertyStore_Commit
//---------------------------------------------------------------------------


_FX HRESULT Taskbar_IPropertyStore_Commit(IPropertyStore *This)
{
    TASKBAR_UNKNOWN *pThis = (TASKBAR_UNKNOWN *)This;
    IPropertyStore *pReal = (IPropertyStore *)pThis->pReal;

    return pReal->lpVtbl->Commit(pReal);
}
