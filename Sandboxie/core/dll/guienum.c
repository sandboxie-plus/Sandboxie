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
// GUI Services
//---------------------------------------------------------------------------

#include "dll.h"

#include "gui_p.h"
#include "core/svc/GuiWire.h"
#include <stdio.h>


//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------


typedef struct _GUI_ENUM_PROC_PARM {

    WNDENUMPROC lpEnumFunc;
    LPARAM lParam;

} GUI_ENUM_PROC_PARM;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN Gui_HookQueryWindow(void);

static ULONG_PTR Gui_NtUserQueryWindow(HWND hWnd, ULONG_PTR type);

//---------------------------------------------------------------------------

static BOOL Gui_EnumProc(HWND hWnd, LPARAM lParam);

static BOOL Gui_EnumCommon(
    UCHAR which, ULONG_PTR arg, WNDENUMPROC lpEnumFunc, LPARAM lParam);

static BOOL Gui_EnumChildWindows(
    HWND hWndParent, WNDENUMPROC lpEnumFunc, LPARAM lParam);

static BOOL Gui_EnumThreadWindows(
    DWORD dwThreadId, WNDENUMPROC lpEnumFunc, LPARAM lParam);

static BOOL Gui_EnumDesktopWindows(
    HDESK hDesktop, WNDENUMPROC lpEnumFunc, LPARAM lParam);

static BOOL Gui_EnumDesktopsW(
    HWINSTA hwinsta, DESKTOPENUMPROC lpEnumFunc, LPARAM lParam);

static BOOL Gui_EnumDesktopsA(
    HWINSTA hwinsta, DESKTOPENUMPROC lpEnumFunc, LPARAM lParam);

static HDESK Gui_OpenDesktopW(
    void *lpszDesktop, ULONG dwFlags, BOOL fInherit,
    ACCESS_MASK dwDesiredAccess);

static HDESK Gui_OpenDesktopA(
    void *lpszDesktop, ULONG dwFlags, BOOL fInherit,
    ACCESS_MASK dwDesiredAccess);

static HDESK Gui_CreateDesktopW(
    void *lpszDesktop, void *lpszDevice, void *DevMode, ULONG dwFlags,
    ACCESS_MASK dwDesiredAccess, void *SecurityAttributes);

static HDESK Gui_CreateDesktopA(
    void *lpszDesktop, void *lpszDevice, void *DevMode, ULONG dwFlags,
    ACCESS_MASK dwDesiredAccess, void *SecurityAttributes);

static HANDLE Gui_CreateWindowStationW(void *lpwinsta, DWORD dwFlags, ACCESS_MASK dwDesiredAccess, LPSECURITY_ATTRIBUTES lpsa);

static HANDLE Gui_CreateWindowStationA(void *lpwinsta, DWORD dwFlags, ACCESS_MASK dwDesiredAccess, LPSECURITY_ATTRIBUTES lpsa);
//---------------------------------------------------------------------------

static HWND Gui_FindWindowCommon(
    ULONG which, HWND hwndParent, HWND hwndChildAfter,
    const void *lpClassName, const void *lpWindowName);

HWND Gui_FindWindowW(
    const WCHAR *lpClassName, const WCHAR *lpWindowName);

static HWND Gui_FindWindowA(
    const UCHAR *lpClassName, const UCHAR *lpWindowName);

static HWND Gui_FindWindowExW(
    HWND hwndParent, HWND hwndChildAfter,
    const WCHAR *lpClassName, const WCHAR *lpWindowName);

static HWND Gui_FindWindowExA(
    HWND hwndParent, HWND hwndChildAfter,
    const UCHAR *lpClassName, const UCHAR *lpWindowName);

//---------------------------------------------------------------------------

static void Gui_MonitorW(const WCHAR *clsnm, ULONG monflag, HWND hwnd);

static void Gui_MonitorA(const UCHAR *clsnm, ULONG monflag, HWND hwnd);

//---------------------------------------------------------------------------

static HWND Gui_GetShellWindow(void);

//---------------------------------------------------------------------------

typedef ULONG_PTR (*P_NtUserQueryWindow)(HWND hWnd, ULONG_PTR type);

static P_NtUserQueryWindow __sys_NtUserQueryWindow = NULL;

/*
HRESULT D3D11CreateDevice(
  _In_opt_        IDXGIAdapter        *pAdapter,
                  D3D_DRIVER_TYPE     DriverType,
                  HMODULE             Software,
                  UINT                Flags,
  _In_opt_  const D3D_FEATURE_LEVEL   *pFeatureLevels,
                  UINT                FeatureLevels,
                  UINT                SDKVersion,
  _Out_opt_       ID3D11Device        **ppDevice,
  _Out_opt_       D3D_FEATURE_LEVEL   *pFeatureLevel,
  _Out_opt_       ID3D11DeviceContext **ppImmediateContext
);
*/
/*
typedef HRESULT (* P_D3D11CreateDevice)(
    void *pAdapter, 
    DWORD DriverType,
    HMODULE Software,
    UINT Flags,
    void *pFeatureLevels,
    UINT FeatureLevels,
    UINT SDKVersion,
    void *ppDevice,
    void *pFeatureLevel,
    void *ppImmediateContext
    );
    */


//d3d11
/*
P_D3D11CreateDevice __sys_D3D11CreateDevice = NULL;
P_D3D11CreateDevice D3D11CreateDevice = NULL;
*/
/*
extern P_D3D11CreateDevice D3D11CreateDevice;
*/
//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static BOOLEAN Gui_D3D9_Loaded = FALSE;

static ULONG Gui_GetShellWindow_LastTicks = 0;


//---------------------------------------------------------------------------
// Gui_InitEnum
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_InitEnum(void)
{
    //
    // hook EnumWindow* and FindWindow* family of functions
    //

    if (! Gui_OpenAllWinClasses) {

        if (! Gui_HookQueryWindow())
            return FALSE;

        if (! Dll_SkipHook(L"enumwin")) {

            SBIEDLL_HOOK_GUI(EnumWindows);
            SBIEDLL_HOOK_GUI(EnumChildWindows);
            SBIEDLL_HOOK_GUI(EnumThreadWindows);
            SBIEDLL_HOOK_GUI(EnumDesktopWindows);
        }

        if (! Dll_SkipHook(L"findwin")) {

            SBIEDLL_HOOK_GUI(FindWindowA);
            SBIEDLL_HOOK_GUI(FindWindowW);
            SBIEDLL_HOOK_GUI(FindWindowExA);
            SBIEDLL_HOOK_GUI(FindWindowExW);

            SBIEDLL_HOOK_GUI(GetShellWindow);
        }

    } else {

        //
        // even if OpenWinClass=* is in effect, we want to hook
        // GetShellWindow to force Windows Explorer to start in the
        // sandbox.   otherwise it just sends a message to the real
        // desktop Explorer to open a new folder window, and quits
        //

        if (Dll_ImageType == DLL_IMAGE_SHELL_EXPLORER) {

            SBIEDLL_HOOK_GUI(GetShellWindow);
        }
    }

    //
    // hook desktop APIs
    //

    SBIEDLL_HOOK_GUI(EnumDesktopsW);
    SBIEDLL_HOOK_GUI(EnumDesktopsA);
    SBIEDLL_HOOK_GUI(OpenDesktopW);
    SBIEDLL_HOOK_GUI(OpenDesktopA);

    // Chrome 52+ now requires the CreateDesktop call for
    // the chrome sandbox desktop. Note:  the sandboxie hook
    // raises an error when CreateDesktop is call.  This hook
    // is removed for chrome.  See advapi.c: AdvApi_GetSecurityInfo

    if (!Config_GetSettingsForImageName_bool(L"UseSbieWndStation", FALSE) && 
        (Dll_ImageType != DLL_IMAGE_GOOGLE_CHROME) &&
        (Dll_ImageType != DLL_IMAGE_MOZILLA_FIREFOX)) {
        SBIEDLL_HOOK_GUI(CreateDesktopW);
        SBIEDLL_HOOK_GUI(CreateDesktopA);
    }
    else {
        SBIEDLL_HOOK_GUI(CreateWindowStationW);
        SBIEDLL_HOOK_GUI(CreateWindowStationA);
    }    

    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_HookQueryWindow
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_HookQueryWindow(void)
{
    static const WCHAR *_ProcName = L"IsHungAppWindow";
    static char *_ProcNameA = "IsHungAppWindow";
    UCHAR *code;
    BOOLEAN ok = FALSE;

    code = Ldr_GetProcAddrNew(DllName_user32, _ProcName,_ProcNameA);
    if (code) {

        //
        // IsHungAppWindow should start with a call to NtUserQueryWindow
        //

#ifdef _WIN64
        const ULONG _E8_Offset = 9;
#else ! _WIN64
        const ULONG _E8_Offset = 10;
#endif _WIN64

        if (code[_E8_Offset] == 0xE8) {

            code = code + _E8_Offset + 5
                + (LONG_PTR)*(LONG *)(code + _E8_Offset + 1);

            //
            // make sure the address we think is NtUserQueryWindow
            // actually looks like a syscall stub
            //

#ifdef _WIN64

            if (    *(ULONG *)code == 0xB8D18B4C
                && (*(USHORT *)(code + 8) == 0x050F) || *(USHORT *)(code + 8) == 0x04F6)
                ok = TRUE;

#else ! _WIN64

            if (Dll_IsWow64) {

                // 64-bit prior Windows 7
                if ( (code[0] == 0xb8) && (code[5] == 0xba) && (*(USHORT *)(code + 10) == 0xd2ff)) {
                    ok = TRUE;
                }
                else if (code[0] == 0xB8 && code[5] == 0x8D && code[9] == 0xB9 && code[14] == 0x64) {
                    ok = TRUE;
                }
                // 64-bit prior Windows 7
                else if (code[0] == 0xB8 && code[5] == 0xB9 && code[10] == 0x8D && code[14] == 0x64) {
                    ok = TRUE;
                }
                // 64-bit Windows 8
                else if (code[0] == 0xB8 && code[12] == 0xC2 && *(USHORT *)(code + 5) == 0xFF64) {
                    ok = TRUE;
                }
                // 64-bit Windows 10
                else if(code[0] == 0xB8 && code[12] == 0xC2 && *(USHORT *)(code + 5) == 0x40BA) {
                    ok = TRUE;
                }

            } else {
                //OutputDebugStringA("32 bit code\n");
                // 32-bit prior to Windows 8
                if (    code[0] == 0xB8 && code[5] == 0xBA
                    && *(USHORT *)(code + 10) == 0x12FF)
                    ok = TRUE;

                // 32-bit Windows 8
                if (    code[0] == 0xB8 && code[5] == 0xE8
                    && *(SHORT *)(code + 10) == 0x08C2)
                    ok = TRUE;

                // 32-bit Windows 10
                if (    code[0] == 0xB8 && code[5] == 0xBA
                    && *(USHORT *)(code + 10) == 0xD2FF)
                    ok = TRUE;
            }

#endif _WIN64

        }
    }

    if (! ok) {
        //Windows 10 RS1 now exports win32k functions in win32u.dll
        code = Ldr_GetProcAddrNew(L"win32u.dll", L"NtUserQueryWindow","NtUserQueryWindow");
        if (code) {
            ok = TRUE;
        }
    }
    if (!ok ){
        SbieApi_Log(2303, L"%S (0)", _ProcName);
    }
    __sys_NtUserQueryWindow = (P_NtUserQueryWindow)code;
    SBIEDLL_HOOK_GUI(NtUserQueryWindow);

    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_NtUserQueryWindow
//---------------------------------------------------------------------------


_FX ULONG_PTR Gui_NtUserQueryWindow(HWND hWnd, ULONG_PTR type)
{
    GUI_QUERY_WINDOW_REQ req;
    GUI_QUERY_WINDOW_RPL *rpl;
    ULONG_PTR result;
    ULONG error;

    if (type <= 2) {

        //
        // if querying the process id (types 0, 1) or thread id (type 2),
        // assume this would be for a window in the sandbox, which would be
        // accessible to this process, and try the direct route first
        //

        result = __sys_NtUserQueryWindow(hWnd, type);
        if (result)
            return result;

    } else if (hWnd && __sys_IsWindow(hWnd)) {

        return __sys_NtUserQueryWindow(hWnd, type);
    }

    //
    // if this isn't type 0,1,2 or the window is not accessible the process,
    // take the indirect route through SbieSvc GUI Proxy
    //

    req.msgid = GUI_QUERY_WINDOW;
    req.error = GetLastError();
    req.hwnd = (ULONG)(ULONG_PTR)hWnd;
    req.type = (ULONG)type;
    rpl = Gui_CallProxy(&req, sizeof(req), sizeof(*rpl));
    if (! rpl)
        return FALSE;

    result = (ULONG_PTR)rpl->result;
    error = rpl->error;
    Dll_Free(rpl);
    SetLastError(error);
    return result;
}


//---------------------------------------------------------------------------
// Gui_EnumProc
//---------------------------------------------------------------------------


_FX BOOL Gui_EnumProc(HWND hWnd, LPARAM lParam)
{
    BOOLEAN allow = Gui_IsWindowAccessible(hWnd);
    if (allow) {

        GUI_ENUM_PROC_PARM *p = (GUI_ENUM_PROC_PARM *)lParam;
        ULONG_PTR result =
                    ProtectCall2(p->lpEnumFunc, (ULONG_PTR)hWnd, p->lParam);
        return (BOOL)result;
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_EnumCommon
//---------------------------------------------------------------------------


_FX BOOL Gui_EnumCommon(
    UCHAR which, ULONG_PTR arg, WNDENUMPROC lpEnumFunc, LPARAM lParam)
{
    GUI_ENUM_WINDOWS_REQ req;
    GUI_ENUM_WINDOWS_RPL *rpl;
    ULONG err, i;
    BOOL ok;

    req.msgid = GUI_ENUM_WINDOWS;
    req.which = which;
    req.arg = (ULONG)arg;
    rpl = Gui_CallProxyEx(&req, sizeof(req), sizeof(*rpl), TRUE);
    if (! rpl)
        return FALSE;

    if (! rpl->ok) {
        err = rpl->error;
        ok = FALSE;
    } else {

        ok = TRUE;
        for (i = 0; i < rpl->num_hwnds; ++i) {
            HWND hwnd = (HWND)(LONG_PTR)(LONG)rpl->hwnds[i];
            ULONG_PTR result =
                        ProtectCall2(lpEnumFunc, (ULONG_PTR)hwnd, lParam);
            if (! result) {
                ok = FALSE;
                break;
            }
        }
    }

    Dll_Free(rpl);
    return ok;
}


//---------------------------------------------------------------------------
// EnumWindows
//---------------------------------------------------------------------------


_FX BOOL Gui_EnumWindows(WNDENUMPROC lpEnumFunc, LPARAM lParam)
{
    return Gui_EnumCommon('W', 0, lpEnumFunc, lParam);
}


//---------------------------------------------------------------------------
// EnumChildWindows
//---------------------------------------------------------------------------


_FX BOOL Gui_EnumChildWindows(
    HWND hWndParent, WNDENUMPROC lpEnumFunc, LPARAM lParam)
{
    if (__sys_IsWindow(hWndParent) && hWndParent != __sys_GetDesktopWindow())
        return __sys_EnumChildWindows(hWndParent, lpEnumFunc, lParam);
    return Gui_EnumCommon('C', (ULONG_PTR)hWndParent, lpEnumFunc, lParam);
}


//---------------------------------------------------------------------------
// EnumThreadWindows
//---------------------------------------------------------------------------


_FX BOOL Gui_EnumThreadWindows(
    DWORD dwThreadId, WNDENUMPROC lpEnumFunc, LPARAM lParam)
{
    if (dwThreadId == GetCurrentThreadId())
        return __sys_EnumThreadWindows(dwThreadId, lpEnumFunc, lParam);
    return Gui_EnumCommon('T', (ULONG_PTR)dwThreadId, lpEnumFunc, lParam);
}


//---------------------------------------------------------------------------
// EnumDesktopWindows
//---------------------------------------------------------------------------


_FX BOOL Gui_EnumDesktopWindows(
    HDESK hDesktop, WNDENUMPROC lpEnumFunc, LPARAM lParam)
{
    // same as Gui_EnumWindows
    return Gui_EnumCommon('W', 0, lpEnumFunc, lParam);
}


//---------------------------------------------------------------------------
// Gui_EnumDesktopsW
//---------------------------------------------------------------------------


_FX BOOL Gui_EnumDesktopsW(
    HWINSTA hwinsta, DESKTOPENUMPROC lpEnumFunc, LPARAM lParam)
{
    static const WCHAR *_DefaultW = L"Default";
    return (BOOL)ProtectCall2(lpEnumFunc, (ULONG_PTR)_DefaultW, lParam);
}


//---------------------------------------------------------------------------
// Gui_EnumDesktopsA
//---------------------------------------------------------------------------


_FX BOOL Gui_EnumDesktopsA(
    HWINSTA hwinsta, DESKTOPENUMPROC lpEnumFunc, LPARAM lParam)
{
    static const char *_DefaultA = "Default";
    return (BOOL)ProtectCall2(lpEnumFunc, (ULONG_PTR)_DefaultA, lParam);
}


//---------------------------------------------------------------------------
// Gui_OpenDesktopW
//---------------------------------------------------------------------------


_FX HDESK Gui_OpenDesktopW(
    void *lpszDesktop, ULONG dwFlags, BOOL fInherit,
    ACCESS_MASK dwDesiredAccess)
{
    return CreateEvent(NULL, FALSE, FALSE, NULL);
}


//---------------------------------------------------------------------------
// Gui_OpenDesktopA
//---------------------------------------------------------------------------


_FX HDESK Gui_OpenDesktopA(
    void *lpszDesktop, ULONG dwFlags, BOOL fInherit,
    ACCESS_MASK dwDesiredAccess)
{
    return CreateEvent(NULL, FALSE, FALSE, NULL);
}

//---------------------------------------------------------------------------
//Gui_CreateWindowStationW
//---------------------------------------------------------------------------
extern HANDLE Sandboxie_WinSta ;

_FX HANDLE Gui_CreateWindowStationW (void *lpwinsta, DWORD dwFlags, ACCESS_MASK dwDesiredAccess, LPSECURITY_ATTRIBUTES lpsa) {
    HANDLE myHandle = 0;

    myHandle =  __sys_CreateWindowStationW(lpwinsta, dwFlags, dwDesiredAccess, lpsa);
    if (!myHandle) {
        return Sandboxie_WinSta;
    }
    return myHandle;
}

_FX HANDLE Gui_CreateWindowStationA (void *lpwinsta, DWORD dwFlags, ACCESS_MASK dwDesiredAccess, LPSECURITY_ATTRIBUTES lpsa) {
    HANDLE myHandle = 0;

    myHandle =  __sys_CreateWindowStationA(lpwinsta, dwFlags, dwDesiredAccess, lpsa);
    if (!myHandle) {
        return Sandboxie_WinSta;
    }
    return myHandle;
}
//---------------------------------------------------------------------------
// Gui_CreateDesktopW
//---------------------------------------------------------------------------


_FX HDESK Gui_CreateDesktopW(
    void *lpszDesktop, void *lpszDevice, void *DevMode, ULONG dwFlags,
    ACCESS_MASK dwDesiredAccess, void *SecurityAttributes)
{
    HANDLE rc = 0;
    //Call the system CreateDesktopW without a security context. 
    //This works in tandem with the Ntmarta_GetSecurityInfo hook (see in advapi.c).

    //Also see comment in Ntmarta_Init at SBIEDLL_HOOK2(Ntmarta_,GetSecurityInfo) for
    //Acrobat Reader.  This is needed to allow this process to create a desktop with
    //the sandboxie restricted token by dropping the security context.  This won't
    //work without the GetSecrityInfo hook.
    rc = __sys_CreateDesktopW(lpszDesktop, NULL, NULL, dwFlags, dwDesiredAccess, NULL);
    if (rc) {
        return rc;
    }

    SbieApi_Log(2205, L"CreateDesktop");
    return CreateEvent(NULL, FALSE, FALSE, NULL);
}


//---------------------------------------------------------------------------
// Gui_CreateDesktopA
//---------------------------------------------------------------------------


_FX HDESK Gui_CreateDesktopA(
    void *lpszDesktop, void *lpszDevice, void *DevMode, ULONG dwFlags,
    ACCESS_MASK dwDesiredAccess, void *SecurityAttributes)
{
    HANDLE rc = 0;
    rc = __sys_CreateDesktopA(lpszDesktop, NULL, NULL, dwFlags, dwDesiredAccess, NULL);
    if (rc) {
        return rc;
    }

    SbieApi_Log(2205, L"CreateDesktop");
    return CreateEvent(NULL, FALSE, FALSE, NULL);
}


//---------------------------------------------------------------------------
// Gui_FindWindowCommon
//---------------------------------------------------------------------------


_FX HWND Gui_FindWindowCommon(
    ULONG which, HWND hwndParent, HWND hwndChildAfter,
    const void *lpClassName, const void *lpWindowName)
{
    GUI_FIND_WINDOW_REQ req;
    GUI_FIND_WINDOW_RPL *rpl;
    HWND hwnd;
    ULONG len;

    memzero(&req, sizeof(req));

    req.msgid = GUI_FIND_WINDOW;
    req.which = which;
    req.hwnd_parent = (ULONG)(ULONG_PTR)hwndParent;
    req.hwnd_child = (ULONG)(ULONG_PTR)hwndChildAfter;

    if (((ULONG_PTR)lpClassName & 0xFFFF0000) == 0) {
        req.class_atom = ((ULONG)(ULONG_PTR)lpClassName) & 0xFFFF;

    } else if (which == 'fw w' || which == 'fwxw') {
        len = wcslen((WCHAR *)lpClassName);
        if (len >= RTL_NUMBER_OF_V1(req.class_text) - 1)
            return NULL;
        wmemcpy(req.class_text, lpClassName, len);
        req.class_is_text = TRUE;

    } else if (which == 'fw a' || which == 'fwxa') {
        len = strlen((char *)lpClassName);
        if (len >= RTL_NUMBER_OF_V1(req.class_text) - 1)
            return NULL;
        memcpy(req.class_text, lpClassName, len);
        req.class_is_text = TRUE;

    } else
        return NULL;

    if (lpWindowName) {

        if (which == 'fw w' || which == 'fwxw') {
            len = wcslen((WCHAR *)lpWindowName);
            if (len >= RTL_NUMBER_OF_V1(req.title_text) - 1)
                return NULL;
            wmemcpy(req.title_text, lpWindowName, len);
            req.title_is_text = TRUE;

        } else if (which == 'fw a' || which == 'fwxa') {
            len = strlen((char *)lpWindowName);
            if (len >= RTL_NUMBER_OF_V1(req.title_text) - 1)
                return NULL;
            memcpy(req.title_text, lpWindowName, len);
            req.title_is_text = TRUE;

        } else
            return NULL;
    }

    rpl = Gui_CallProxyEx(&req, sizeof(req), sizeof(*rpl), TRUE);
    if (! rpl)
        return FALSE;

    hwnd = (HWND)(LONG_PTR)(LONG)rpl->hwnd;
    Dll_Free(rpl);
    return hwnd;
}


//---------------------------------------------------------------------------
// Gui_FindWindowW
//---------------------------------------------------------------------------


_FX HWND Gui_FindWindowW(
    const WCHAR *lpClassName, const WCHAR *lpWindowName)
{
    WCHAR *clsnm;
    WCHAR *winnm;
    HWND hwndResult;
    ULONG monflag = 0;

#ifdef DEBUG_FINDWINDOW
    WCHAR txt[256];

    if (((ULONG_PTR)lpClassName & 0xFFFF0000) != 0)
        Sbie_snwprintf(txt, 256, L"FindWindowW   - %s\n", lpClassName);
    else
        Sbie_snwprintf(txt, 256, L"FindWindowW   - %X\n", lpClassName);
    OutputDebugString(txt);
#endif

    clsnm = Gui_CreateClassNameW(lpClassName);
    if (clsnm == lpClassName)
        monflag |= MONITOR_OPEN;

    hwndResult = __sys_FindWindowW(clsnm, lpWindowName);

    if (! hwndResult && lpWindowName) {
        winnm = Gui_CreateTitleW(lpWindowName);
        if (winnm != lpWindowName) {
            hwndResult = __sys_FindWindowW(clsnm, winnm);
            Gui_Free((void *)winnm);
        }
    }

    if (! hwndResult) {
        hwndResult = Gui_FindWindowCommon(
                        'fw w', NULL, NULL, lpClassName, lpWindowName);
    }

    if (hwndResult && (! Gui_IsWindowAccessible(hwndResult)))
        hwndResult = NULL;

    Gui_MonitorW(clsnm, monflag, hwndResult);

    if (clsnm != lpClassName)
        Gui_Free(clsnm);

#ifdef DEBUG_FINDWINDOW
    Sbie_snwprintf(txt, 256, L"FindWindowW   - Result HWND %X\n", hwndResult);
    OutputDebugString(txt);
#endif

    return hwndResult;
}


//---------------------------------------------------------------------------
// Gui_FindWindowA
//---------------------------------------------------------------------------


_FX HWND Gui_FindWindowA(
    const UCHAR *lpClassName, const UCHAR *lpWindowName)
{
    UCHAR *clsnm;
    UCHAR *winnm;
    HWND hwndResult;
    ULONG monflag = 0;

#ifdef DEBUG_FINDWINDOW
    WCHAR txt[256];
    if (((ULONG_PTR)lpClassName & 0xFFFF0000) != 0)
        Sbie_snwprintf(txt, 256, L"FindWindowA   - %S\n", lpClassName);
    else
		Sbie_snwprintf(txt, 256, L"FindWindowA   - %X\n", lpClassName);
    OutputDebugString(txt);
#endif

    clsnm = Gui_CreateClassNameA(lpClassName);
    if (clsnm == lpClassName)
        monflag |= MONITOR_OPEN;

    hwndResult = __sys_FindWindowA(clsnm, lpWindowName);

    if (! hwndResult && lpWindowName) {
        winnm = Gui_CreateTitleA(lpWindowName);
        if (winnm != lpWindowName) {
            hwndResult = __sys_FindWindowA(clsnm, winnm);
            Gui_Free((void *)winnm);
        }
    }

    if (! hwndResult) {
        hwndResult = Gui_FindWindowCommon(
                        'fw a', NULL, NULL, lpClassName, lpWindowName);
    }

    if (hwndResult && (! Gui_IsWindowAccessible(hwndResult)))
        hwndResult = NULL;

    Gui_MonitorA(clsnm, monflag, hwndResult);

    if (clsnm != lpClassName)
        Gui_Free(clsnm);

#ifdef DEBUG_FINDWINDOW
	Sbie_snwprintf(txt, 256, L"FindWindowA   - Result HWND %X\n", hwndResult);
    OutputDebugString(txt);
#endif

    return hwndResult;
}


//---------------------------------------------------------------------------
// Gui_FindWindowExW
//---------------------------------------------------------------------------


_FX HWND Gui_FindWindowExW(
    HWND hwndParent, HWND hwndChildAfter,
    const WCHAR *lpClassName, const WCHAR *lpWindowName)
{
    WCHAR *clsnm;
    WCHAR *winnm;
    HWND hwndResult;
    ULONG monflag = 0;

#ifdef DEBUG_FINDWINDOW
    WCHAR txt[256];
    if (((ULONG_PTR)lpClassName & 0xFFFF0000) != 0)
		Sbie_snwprintf(txt, 256, L"FindWindowExW - %s\n", lpClassName);
    else
		Sbie_snwprintf(txt, 256, L"FindWindowExW - %X\n", lpClassName);
    OutputDebugString(txt);
#endif

    clsnm = Gui_CreateClassNameW(lpClassName);
    if (clsnm == lpClassName)
        monflag |= MONITOR_OPEN;

    hwndResult = __sys_FindWindowExW(
        hwndParent, hwndChildAfter, clsnm, lpWindowName);

    if (! hwndResult && lpWindowName) {
        winnm = Gui_CreateTitleW(lpWindowName);
        if (winnm != lpWindowName) {
            hwndResult = __sys_FindWindowExW(
                hwndParent, hwndChildAfter, clsnm, winnm);
            Gui_Free((void *)winnm);
        }
    }

    if (! hwndResult) {
        hwndResult = Gui_FindWindowCommon(
            'fwxw', hwndParent, hwndChildAfter, lpClassName, lpWindowName);
    }

    if (hwndResult && (! Gui_IsWindowAccessible(hwndResult)))
        hwndResult = NULL;

    Gui_MonitorW(clsnm, monflag, hwndResult);

    if (clsnm != lpClassName)
        Gui_Free(clsnm);

#ifdef DEBUG_FINDWINDOW
	Sbie_snwprintf(txt, 256, L"FindWindowExW - Result HWND %X\n", hwndResult);
    OutputDebugString(txt);
#endif

    return hwndResult;
}


//---------------------------------------------------------------------------
// Gui_FindWindowExA
//---------------------------------------------------------------------------


_FX HWND Gui_FindWindowExA(
    HWND hwndParent, HWND hwndChildAfter,
    const UCHAR *lpClassName, const UCHAR *lpWindowName)
{
    UCHAR *clsnm;
    UCHAR *winnm;
    HWND hwndResult;
    ULONG monflag = 0;

#ifdef DEBUG_FINDWINDOW
    WCHAR txt[256];
    if (((ULONG_PTR)lpClassName & 0xFFFF0000) != 0)
		Sbie_snwprintf(txt, 256, L"FindWindowExA - %S\n", lpClassName);
    else
		Sbie_snwprintf(txt, 256, L"FindWindowExA - %X\n", lpClassName);
    OutputDebugString(txt);
#endif

    clsnm = Gui_CreateClassNameA(lpClassName);
    if (clsnm == lpClassName)
        monflag |= MONITOR_OPEN;

    hwndResult = __sys_FindWindowExA(
        hwndParent, hwndChildAfter, clsnm, lpWindowName);

    if (! hwndResult && lpWindowName) {
        winnm = Gui_CreateTitleA(lpWindowName);
        if (winnm != lpWindowName) {
            hwndResult = __sys_FindWindowExA(
                hwndParent, hwndChildAfter, clsnm, winnm);
            Gui_Free((void *)winnm);
        }
    }

    if (! hwndResult) {
        hwndResult = Gui_FindWindowCommon(
            'fwxa', hwndParent, hwndChildAfter, lpClassName, lpWindowName);
    }

    if (hwndResult && (! Gui_IsWindowAccessible(hwndResult)))
        hwndResult = NULL;

    Gui_MonitorA(clsnm, monflag, hwndResult);

    if (clsnm != lpClassName)
        Gui_Free(clsnm);

#ifdef DEBUG_FINDWINDOW
	Sbie_snwprintf(txt, 256, L"FindWindowExA - Result HWND %X\n", hwndResult);
    OutputDebugString(txt);
#endif

    return hwndResult;
}


//---------------------------------------------------------------------------
// Gui_MonitorW
//---------------------------------------------------------------------------


_FX void Gui_MonitorW(const WCHAR *clsnm, ULONG monflag, HWND hwnd)
{
    WCHAR text[130];
    if (((ULONG_PTR)clsnm & (LONG_PTR)0xFFFF0000) != 0) {
        wcsncpy(text, Gui_UnCreateClassName(clsnm), 128);
        text[128] = L'\0';
    } else
        Sbie_snwprintf(text, 130, L"#%d", PtrToUlong(clsnm) & 0xFFFF);
    if ((! hwnd) && (! monflag))
        monflag |= MONITOR_DENY;
    SbieApi_MonitorPut(MONITOR_WINCLASS | monflag, text);
}


//---------------------------------------------------------------------------
// Gui_MonitorA
//---------------------------------------------------------------------------


_FX void Gui_MonitorA(const UCHAR *clsnm, ULONG monflag, HWND hwnd)
{
    if (((ULONG_PTR)clsnm & (LONG_PTR)0xFFFF0000) != 0) {
        NTSTATUS status;
        UNICODE_STRING uni;
        ANSI_STRING ansi;
        RtlInitString(&ansi, clsnm);
        status = RtlAnsiStringToUnicodeString(&uni, &ansi, TRUE);
        if (NT_SUCCESS(status)) {
            Gui_MonitorW(uni.Buffer, monflag, hwnd);
            RtlFreeUnicodeString(&uni);
        }
    } else
        Gui_MonitorW((WCHAR *)clsnm, monflag, hwnd);
}


//---------------------------------------------------------------------------
// Gui_GetShellWindow
//---------------------------------------------------------------------------


_FX HWND Gui_GetShellWindow(void)
{
    static const WCHAR *_Progman = L"Progman";
    static HWND _LastHwnd = NULL;
    HWND hwnd;

    ULONG TicksNow = GetTickCount();
    if (TicksNow - Gui_GetShellWindow_LastTicks <= 5000)
        return _LastHwnd;
    Gui_GetShellWindow_LastTicks = TicksNow;

    hwnd = NULL;
    if (Gui_RenameClasses)
        hwnd = Gui_FindWindowW(_Progman, NULL);
    if ((! hwnd) && Gui_D3D9_Loaded && __sys_FindWindowW)
        hwnd = Gui_FindWindowCommon('fw w', NULL, NULL, _Progman, NULL);
    _LastHwnd = hwnd;
    return hwnd;
}


//---------------------------------------------------------------------------
// Gui_Init_D3D9
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_Init_D3D9(HMODULE module)
{
    //
    // some programs use GetShellWindow as the hFocusWindow argument for
    // the call to IDirect3D9::CreateDevice or IDirect3D9Ex::CreateDeviceEx
    // and if GetShellWindow returns NULL then the device creation fails.
    // work around that by returning the real GetShellWindow for programs
    // that load d3d9.dll
    //

    Gui_D3D9_Loaded = TRUE;
    // force Gui_GetShellWindow to refresh cached value
    Gui_GetShellWindow_LastTicks = 0;
    return TRUE;
}


/* //test code for d3d11
extern ULONG Dll_Windows;
UCHAR myTramp[128];

_FX BOOLEAN Gui_Init_D3D11(HMODULE module)
{
#ifndef _WIN64
    if ( Dll_ImageType == DLL_IMAGE_MOZILLA_FIREFOX) {
        if (Dll_Windows >= 10) {
            char buffer[256];
            UCHAR *tramp = myTramp;
            DWORD region_size;
            void * region_base;
            ULONG OldProtect;
            NTSTATUS status;
            LONG_PTR src;
            LONG_PTR target;
            UCHAR *code;

            D3D11CreateDevice = (P_D3D11CreateDevice) GetProcAddress(module,"D3D11CreateDevice");

            sprintf(buffer,"D3D11CreateDevice = %p \n",D3D11CreateDevice);
            OutputDebugStringA(buffer);

            //if(D3D11CreateDevice) {
    //      region_size = 0x400;
            //  tramp = Dll_AllocCode128();
    //      status = NtAllocateVirtualMemory( NtCurrentProcess(),  &tramp, 0, &region_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            //setup tramp

            region_size = 128;
            region_base = tramp;
            status = NtProtectVirtualMemory( NtCurrentProcess(),  &region_base, &region_size,
                PAGE_EXECUTE_READWRITE, &OldProtect);

            region_base = (void *) D3D11CreateDevice;
            region_size = 5;

            status = NtProtectVirtualMemory( NtCurrentProcess(), &region_base, &region_size, PAGE_EXECUTE_READWRITE, &OldProtect);
            region_size = 5;
            memcpy(tramp,D3D11CreateDevice,5);
            tramp[5] = 0xe9;
            src = (LONG_PTR) &tramp[10];
            target = (LONG_PTR) D3D11CreateDevice;
            target + 5;
            *((LONG *) (&tramp[6])) = target - src;
            //SBIEDLL_HOOK_GUI(D3D11CreateDevice);
            __sys_D3D11CreateDevice = (P_D3D11CreateDevice) tramp;
            sprintf(buffer,"__sys_D3D11CreateDevice = %p, tramp = %p,delta = %p \n",__sys_D3D11CreateDevice, tramp, target - src);
            OutputDebugStringA(buffer);
            // end setup tramp
            //}
            src = (LONG_PTR) D3D11CreateDevice;
            src +=5;
            target = (LONG_PTR)Gui_D3D11CreateDevice;
            code = (UCHAR *)D3D11CreateDevice;

            code[0] = 0xe9;
            *((LONG *) (&code[1])) = target - src;

            sprintf(buffer,"Hook: src = %p, target = %p,delta = %p \n",src, target, target - src);
            OutputDebugStringA(buffer);

        }       
    }
#endif
    return TRUE;
}
#ifndef _WIN64
_FX HRESULT Gui_D3D11CreateDevice(
    void *pAdapter, 
    DWORD DriverType,
    HMODULE Software,
    UINT Flags,
    void *pFeatureLevels,
    UINT FeatureLevels,
    UINT SDKVersion,
    void *ppDevice,
    void *pFeatureLevel,
    void *ppImmediateContext
    ) {
    if ( Dll_ImageType == DLL_IMAGE_MOZILLA_FIREFOX) {
    OutputDebugStringA("Disable D3D11CreateDevice for wow64");
     return 0x80070005;
    }
    __asm {
        jmp __sys_D3D11CreateDevice
    }

 }
#endif
*/