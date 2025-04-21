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
// GUI Services
//---------------------------------------------------------------------------

#include "dll.h"

#include "gui_p.h"
#include "core/svc/GuiWire.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static HWND Gui_GetWindow(HWND hWnd, UINT uCmd);

static HWND Gui_GetParent(HWND hWnd);

static HWND Gui_SetParent(HWND hWndChild, HWND hWndNewParent);

static BOOL Gui_SwapMouseButton(BOOL fSwap);

static BOOL Gui_SetDoubleClickTime(UINT uInterval);

static BOOL Gui_ClipCursor(const RECT *lpRect);

static BOOLEAN Gui_GrantHandle(ULONG_PTR handle);

static HCURSOR Gui_SetCursor(HCURSOR hCursor);

static BOOL Gui_GetIconInfo(HICON hIcon, PICONINFO piconinfo);

static BOOL Gui_SetCursorPos(int x, int y);

static BOOL Gui_SetForegroundWindow(HWND hWnd);

static HMONITOR Gui_MonitorFromWindow(HWND hWnd, DWORD dwFlags);

static BOOL Gui_BlockInput(BOOL fBlockIt);

static UINT Gui_SendInput(ULONG nInputs, LPINPUT pInputs, ULONG cbInput);

static HDESK Gui_OpenInputDesktop(
    DWORD dwFlags, BOOL fInherit, ACCESS_MASK dwDesiredAccess);

static BOOL Gui_GetUserObjectInformationW(
    HANDLE hObj, int nIndex,
    void *pvInfo, DWORD nLength, DWORD *lpnLengthNeeded);

static BOOL Gui_OpenClipboard(HWND hwnd);

static BOOL Gui_CloseClipboard(void);

static HANDLE Gui_SetClipboardData(UINT uFormat, HANDLE hMem);

static HANDLE Gui_GetClipboardData(UINT uFormat);

static void Gui_GetClipboardData_BMP(void *buf, ULONG fmt);

static void Gui_GetClipboardData_MF(void *buf, ULONG sz, ULONG fmt);

static void Gui_GetClipboardData_EnhMF(void *buf, ULONG sz, ULONG fmt);

static BOOL Gui_EmptyClipboard();

static int Gui_ShowCursor(BOOL bShow);

static HWND Gui_SetActiveWindow(HWND hWnd);

static BOOL  Gui_BringWindowToTop(HWND hWnd);

static void Gui_SwitchToThisWindow(HWND hWnd, BOOL fAlt);

static LONG Gui_ChangeDisplaySettingsExA(
    void *lpszDeviceName, void *lpDevMode, HWND hwnd,
    DWORD dwflags, void *lParam);

static LONG Gui_ChangeDisplaySettingsExW(
    void *lpszDeviceName, void *lpDevMode, HWND hwnd,
    DWORD dwflags, void *lParam);


static LONG Gui_GetRawInputDeviceInfoA(
    _In_opt_ HANDLE hDevice, _In_ UINT uiCommand,
    _Inout_ LPVOID pData, _Inout_ PUINT pcbSize);

static LONG Gui_GetRawInputDeviceInfoW(
    _In_opt_ HANDLE hDevice, _In_ UINT uiCommand,
    _Inout_ LPVOID pData, _Inout_ PUINT pcbSize);

static HDC Gui_GetDC(HWND hWnd);

static HDC Gui_GetWindowDC(HWND hWnd);

static HDC Gui_GetDCEx(HWND hWnd, HRGN hrgnClip, DWORD flags);

static BOOL Gui_PrintWindow(HWND hwnd, HDC hdcBlt, UINT nFlags);

static int Gui_ReleaseDC(HWND hWnd, HDC hDc);

static BOOL Gui_ShutdownBlockReasonCreate(HWND hWnd, LPCWSTR pwszReason);

static UINT_PTR Gui_SetTimer(HWND hWnd, UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc);

static DWORD Wimm_timeGetTime(); 

static MMRESULT Wimm_timeSetEvent(UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc, DWORD_PTR dwUser, UINT fuEvent);
//---------------------------------------------------------------------------


typedef BOOL (*P_GetUserObjectInformationW)(
    HANDLE hObj, int nIndex,
    void *pvInfo, DWORD nLength, DWORD *lpnLengthNeeded);

typedef HMETAFILE (*P_SetMetaFileBitsEx)(
    UINT nSize, const BYTE *lpData);

typedef HENHMETAFILE (*P_SetEnhMetaFileBits)(
    UINT nSize, const BYTE *lpData);

typedef int (*P_SetDIBits)
    (HDC hdc, HBITMAP hbm, UINT start, UINT cLines, CONST VOID *lpBits, CONST BITMAPINFO * lpbmi, UINT ColorUse);

typedef HDC (*P_GetDC)(
    HWND hWnd);

typedef int (*P_ReleaseDC)(
    HWND hWnd,
    HDC hDC);

static P_GetUserObjectInformationW __sys_GetUserObjectInformationW = NULL;

#ifndef _DPI_AWARENESS_CONTEXTS_
struct DPI_AWARENESS_CONTEXT__ { int unused; };
typedef DPI_AWARENESS_CONTEXT__ *DPI_AWARENESS_CONTEXT;
#endif

typedef DPI_AWARENESS_CONTEXT (WINAPI *P_GetThreadDpiAwarenessContext)(
    VOID);

static P_GetThreadDpiAwarenessContext __sys_GetThreadDpiAwarenessContext = NULL;

static ULONG64 Dll_FirsttimeGetTimeValue = 0;
//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static BOOLEAN Gui_ClipCursorActive = FALSE;

static HWND  Gui_OpenClipboard_hwnd = NULL;
static ULONG Gui_OpenClipboard_tid  = 0;
static ULONG Gui_OpenClipboard_seq  = -1;

static HANDLE Gui_DummyInputDesktopHandle = NULL;

       BOOLEAN Gui_BlockInterferenceControl = FALSE;

       BOOLEAN Gui_DontAllowCoverTaskbar = FALSE;


//---------------------------------------------------------------------------
// Gui_InitMisc
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_InitMisc(HMODULE module)
{
    if (! Gui_OpenAllWinClasses) {

        Gui_BlockInterferenceControl = SbieApi_QueryConfBool(NULL, L"BlockInterferenceControl", FALSE);
        if(Gui_BlockInterferenceControl)
            Gui_DontAllowCoverTaskbar = !SbieApi_QueryConfBool(NULL, L"AllowCoverTaskbar", FALSE);
        
        SBIEDLL_HOOK_GUI(SetParent);
        if (Gui_UseProxyService) {
            SBIEDLL_HOOK_GUI(GetWindow);
            SBIEDLL_HOOK_GUI(GetParent);
            
            SBIEDLL_HOOK_GUI(MonitorFromWindow);
        
            SBIEDLL_HOOK_GUI(SetCursor);
            SBIEDLL_HOOK_GUI(GetIconInfo);
            
        }
		SBIEDLL_HOOK_GUI(SetCursorPos);
		SBIEDLL_HOOK_GUI(SetForegroundWindow);
		SBIEDLL_HOOK_GUI(ClipCursor);
        SBIEDLL_HOOK_GUI(SwapMouseButton);
        SBIEDLL_HOOK_GUI(SetDoubleClickTime);
        SBIEDLL_HOOK_GUI(ShowCursor);
        SBIEDLL_HOOK_GUI(BringWindowToTop);
        SBIEDLL_HOOK_GUI(SwitchToThisWindow);
        SBIEDLL_HOOK_GUI(SetActiveWindow);
		
        if (Gui_UseBlockCapture) {
            SBIEDLL_HOOK_GUI(GetWindowDC);
            SBIEDLL_HOOK_GUI(GetDC);
            SBIEDLL_HOOK_GUI(GetDCEx);
            SBIEDLL_HOOK_GUI(PrintWindow);
            SBIEDLL_HOOK_GUI(ReleaseDC);
        }

        if (Dll_OsBuild >= 6000) {

            //
            // hook BlockInput and SendInput on Windows Vista because UIPI
            // is disabled if UAC is disabled.  if UAC is enabled, kernel
            // side UIPI will also protect these two APIs.  see also Gui_Init
            // in core/drv/gui.c and PostThreadMessage in core/dll/guimsg.c
            //

            SBIEDLL_HOOK_GUI(BlockInput);
            SBIEDLL_HOOK_GUI(SendInput);
        }
    }

	if (!Gui_UseProxyService)
		return TRUE;

    SBIEDLL_HOOK_GUI(OpenClipboard);
    SBIEDLL_HOOK_GUI(CloseClipboard);
    SBIEDLL_HOOK_GUI(SetClipboardData);
    SBIEDLL_HOOK_GUI(GetClipboardData);
    SBIEDLL_HOOK_GUI(EmptyClipboard);

    //
    // Chinese instant messenger QQ.exe (aka TM.exe) uses OpenInputDesktop,
    // GetThreadDesktop and GetUserObjectInformation to determine if the
    // desktop is locked, and if OpenInputDesktop fails, it assumes lock.
    // Fortunately it is enough to hook just OpenInputDesktop to fix this
    //
    // Google Chrome also uses OpenInputDesktop and GetUserObjectInformation
    // to check if the desktop is locked, and other programs might as well
    //

    if (!Dll_CompartmentMode) {

        typedef HDESK (*P_OpenInputDesktop)(
            DWORD dwFlags, BOOL fInherit, ACCESS_MASK dwDesiredAccess);

        P_OpenInputDesktop __sys_OpenInputDesktop = (P_OpenInputDesktop)
            Ldr_GetProcAddrNew(DllName_user32, L"OpenInputDesktop","OpenInputDesktop");

        __sys_GetUserObjectInformationW = (P_GetUserObjectInformationW)
            Ldr_GetProcAddrNew(DllName_user32, L"GetUserObjectInformationW","GetUserObjectInformationW");

        if (__sys_OpenInputDesktop) {
            SBIEDLL_HOOK_GUI(OpenInputDesktop);
        }

        if (__sys_GetUserObjectInformationW) {
            SBIEDLL_HOOK_GUI(GetUserObjectInformationW);
        }
    }

    //
    // ChangeDisplaySettingsEx on Windows 8
    //

    if (Dll_OsBuild >= 8400) {
        P_ChangeDisplaySettingsEx __sys_ChangeDisplaySettingsExA =
            Ldr_GetProcAddrNew(DllName_user32, L"ChangeDisplaySettingsExA","ChangeDisplaySettingsExA");
        P_ChangeDisplaySettingsEx __sys_ChangeDisplaySettingsExW =
            Ldr_GetProcAddrNew(DllName_user32, L"ChangeDisplaySettingsExW","ChangeDisplaySettingsExW");
        SBIEDLL_HOOK_GUI(ChangeDisplaySettingsExA);
        SBIEDLL_HOOK_GUI(ChangeDisplaySettingsExW);
    }

    SBIEDLL_HOOK_GUI(GetRawInputDeviceInfoA);
    SBIEDLL_HOOK_GUI(GetRawInputDeviceInfoW);

	__sys_GetThreadDpiAwarenessContext = (P_GetThreadDpiAwarenessContext)
		Ldr_GetProcAddrNew(DllName_user32, L"GetThreadDpiAwarenessContext","GetThreadDpiAwarenessContext");


    if (SbieApi_QueryConfBool(NULL, L"BlockInterferePower", FALSE)) {

        SBIEDLL_HOOK_GUI(ShutdownBlockReasonCreate);
    }
	
	if (SbieApi_QueryConfBool(NULL, L"UseChangeSpeed", FALSE)) 	{
		P_SetTimer SetTimer = Ldr_GetProcAddrNew(DllName_user32, L"SetTimer", "SetTimer");
        if (SetTimer) {
            SBIEDLL_HOOK(Gui_, SetTimer);
        }

        // hook winmm timeGetTime and timeSetEvent
        if(GetModuleHandle(DllName_winmm) == NULL)
        {
            LoadLibrary(DllName_winmm);
        }

        P_timeGetTime timeGetTime = (P_timeGetTime)Ldr_GetProcAddrNew(DllName_winmm, L"timeGetTime", "timeGetTime");
        if (timeGetTime) {
            SBIEDLL_HOOK(Wimm_, timeGetTime);
            Dll_FirsttimeGetTimeValue = __sys_timeGetTime();
        }

        P_timeSetEvent timeSetEvent = (P_timeSetEvent)Ldr_GetProcAddrNew(DllName_winmm, L"timeSetEvent", "timeSetEvent");
        if(timeSetEvent) {
            SBIEDLL_HOOK(Wimm_, timeSetEvent);
        }
	}
	
    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_GetWindow
//---------------------------------------------------------------------------


_FX HWND Gui_GetWindow(HWND hWnd, UINT uCmd)
{
    if (uCmd < 0x10)
        return Gui_GetWindowFromProxy(uCmd, hWnd);
    else
        return NULL;
}


//---------------------------------------------------------------------------
// Gui_GetParent
//---------------------------------------------------------------------------


_FX HWND Gui_GetParent(HWND hWnd)
{
    if (__sys_IsWindow(hWnd) || (! hWnd))
        return __sys_GetParent(hWnd);
    else
        return Gui_GetWindowFromProxy('prnt', hWnd);
}


//---------------------------------------------------------------------------
// Gui_SetParent
//---------------------------------------------------------------------------


_FX HWND Gui_SetParent(HWND hWndChild, HWND hWndNewParent)
{
    //
    // fail the request (with a warning) if either the child or the
    // parent windows are not accessible to this process.  but note that
    // we make a special exception of converting the desktop hwnd to NULL
    //

    if (! Gui_IsWindowAccessible(hWndChild)) {
        SetLastError(ERROR_INVALID_WINDOW_HANDLE);
        return NULL;
    }

    if (hWndNewParent && (! Gui_IsWindowAccessible(hWndNewParent))) {
        if (hWndNewParent == __sys_GetDesktopWindow())
            hWndNewParent = NULL;
        else {
            //SbieApi_Log(2205, L"SetParent");
            SetLastError(ERROR_INVALID_WINDOW_HANDLE);
            return NULL;
        }
    }

    return __sys_SetParent(hWndChild, hWndNewParent);
}


//---------------------------------------------------------------------------
// Gui_ClipCursor
//---------------------------------------------------------------------------


_FX BOOL Gui_ClipCursor(const RECT *lpRect)
{
	if (Gui_BlockInterferenceControl && lpRect) {
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	}
	
	if (!Gui_UseProxyService)
		return __sys_ClipCursor(lpRect);
	
    GUI_CLIP_CURSOR_REQ req;
    void *rpl;

    req.msgid = GUI_CLIP_CURSOR;
    if (lpRect) {
        req.have_rect = TRUE;
        memcpy(&req.rect, lpRect, sizeof(req.rect));
        Gui_ClipCursorActive = TRUE;
    } else {
        req.have_rect = FALSE;
        memzero(&req.rect, sizeof(req.rect));
        Gui_ClipCursorActive = FALSE;
    }
    req.dpi_awareness_ctx = __sys_GetThreadDpiAwarenessContext ? (LONG64)(LONG_PTR)__sys_GetThreadDpiAwarenessContext() : 0;

    rpl = Gui_CallProxy(&req, sizeof(req), sizeof(ULONG));
    if (rpl) {
        Dll_Free(rpl);
        return TRUE;
    } else {
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }
}


//---------------------------------------------------------------------------
// Gui_ResetClipCursor
//---------------------------------------------------------------------------


_FX void Gui_ResetClipCursor(void)
{
    //
    // programs that set a clip cursor (e.g. full screen games) tend to
    // remove the clip on alt-tab switch, but do not remove the clip when
    // terminating.  and the clip remains in effect probably because the
    // sandboxed process does not have WINSTA_WRITEATTRIBUTES access.
    // to work around this, we have DllMain call this function on exit.
    //

    if (Gui_ClipCursorActive)
        Gui_ClipCursor(NULL);
}


//---------------------------------------------------------------------------
// Gui_SwapMouseButton
//---------------------------------------------------------------------------


_FX BOOL Gui_SwapMouseButton(BOOL fSwap)
{
    SetLastError(ERROR_ACCESS_DENIED);
    return FALSE;
}


//---------------------------------------------------------------------------
// Gui_SetDoubleClickTime
//---------------------------------------------------------------------------


_FX BOOL Gui_SetDoubleClickTime(UINT uInterval)
{
    SetLastError(ERROR_ACCESS_DENIED);
    return FALSE;
}


//---------------------------------------------------------------------------
// Gui_GrantHandle
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_GrantHandle(ULONG_PTR handle)
{
    GUI_GRANT_HANDLE_REQ req;
    void *rpl;

    if (! handle)
        return FALSE;

    req.msgid = GUI_GRANT_HANDLE;
    req.handle_type = 1;        // bitmap/cursor/icon
    req.handle_value = (ULONG)handle;

    rpl = Gui_CallProxy(&req, sizeof(req), sizeof(ULONG));
    if (rpl) {
        Dll_Free(rpl);
        return TRUE;

    } else {
        SetLastError(ERROR_INVALID_CURSOR_HANDLE);
        return FALSE;
    }
}


//---------------------------------------------------------------------------
// Gui_SetCursor
//---------------------------------------------------------------------------


_FX HCURSOR Gui_SetCursor(HCURSOR hCursor)
{
    HCURSOR hCursorRet = __sys_SetCursor(hCursor);
    if ((! hCursorRet) && (GetLastError() == ERROR_INVALID_CURSOR_HANDLE)) {

        //
        // the cursor handle is probably not yet accessible to our
        // job object so ask SbieSvc GUI Proxy server to grant us access
        //

        if (Gui_GrantHandle((ULONG_PTR)hCursor)) {

            hCursorRet = __sys_SetCursor(hCursor);
        }
    }

    return hCursorRet;
}


//---------------------------------------------------------------------------
// Gui_GetIconInfo
//---------------------------------------------------------------------------


_FX BOOL Gui_GetIconInfo(HICON hIcon, PICONINFO piconinfo)
{
    BOOL ok = __sys_GetIconInfo(hIcon, piconinfo);
    if ((! ok) && (GetLastError() == ERROR_INVALID_CURSOR_HANDLE)) {

        //
        // the icon handle is probably not yet accessible to our
        // job object so ask SbieSvc GUI Proxy server to grant us access
        //

        if (Gui_GrantHandle((ULONG_PTR)hIcon)) {

            ok = __sys_GetIconInfo(hIcon, piconinfo);
        }
    }

    return ok;
}


//---------------------------------------------------------------------------
// Gui_SetCursorPos
//---------------------------------------------------------------------------


_FX BOOL Gui_SetCursorPos(int x, int y)
{
	if (Gui_BlockInterferenceControl)
		return FALSE;
	
	if (!Gui_UseProxyService)
		return __sys_SetCursorPos(x, y);
		
    GUI_SET_CURSOR_POS_REQ req;
    GUI_SET_CURSOR_POS_RPL *rpl;
    ULONG error;
    BOOL retval;

    req.msgid = GUI_SET_CURSOR_POS;
    req.error = GetLastError();
    req.x = x;
    req.y = y;
    req.dpi_awareness_ctx = __sys_GetThreadDpiAwarenessContext ? (LONG64)(LONG_PTR)__sys_GetThreadDpiAwarenessContext() : 0;
    rpl = Gui_CallProxyEx(&req, sizeof(req), sizeof(ULONG), TRUE);
    if (rpl) {
        retval = rpl->retval;
        error = rpl->error;
        Dll_Free(rpl);
    } else {
        error = ERROR_ACCESS_DENIED;
        retval = FALSE;
    }
    SetLastError(error);
    return retval;
}


//---------------------------------------------------------------------------
// Gui_SetForegroundWindow
//---------------------------------------------------------------------------


_FX BOOL Gui_SetForegroundWindow(HWND hWnd)
{
    GUI_SET_FOREGROUND_WINDOW_REQ req;
    void *rpl;

	if (Gui_BlockInterferenceControl)	{
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	}
	
    if (!Gui_UseProxyService || __sys_IsWindow(hWnd) || (! hWnd)) {
        // window is in the same sandbox (or is NULL), no need for GUI Proxy
        return __sys_SetForegroundWindow(hWnd);
    }

    if (__sys_SetForegroundWindow(hWnd))
        return TRUE;

    Gui_AllowSetForegroundWindow();

    req.msgid = GUI_SET_FOREGROUND_WINDOW;
    req.hwnd = (ULONG)(ULONG_PTR)hWnd;
    rpl = Gui_CallProxyEx(&req, sizeof(req), sizeof(ULONG), TRUE);
    if (rpl) {
        Dll_Free(rpl);
        return TRUE;
    } else {
        SetLastError(ERROR_INVALID_WINDOW_HANDLE);
        return FALSE;
    }
}


//---------------------------------------------------------------------------
// Gui_MonitorFromWindow
//---------------------------------------------------------------------------


_FX HMONITOR Gui_MonitorFromWindow(HWND hWnd, DWORD dwFlags)
{
    ULONG err = GetLastError();
    HMONITOR result = __sys_MonitorFromWindow(hWnd, dwFlags);
    if (! result) {

        GUI_MONITOR_FROM_WINDOW_REQ req;
        GUI_MONITOR_FROM_WINDOW_RPL *rpl;

        req.msgid = GUI_MONITOR_FROM_WINDOW;
        req.error = err;
        req.hwnd = (ULONG)(ULONG_PTR)hWnd;
        req.flags = dwFlags;

        rpl = Gui_CallProxy(&req, sizeof(req), sizeof(*rpl));
        if (! rpl)
            return FALSE;

        err = rpl->error;
        result = (HMONITOR)(LONG_PTR)(LONG)rpl->retval;
        Dll_Free(rpl);

        SetLastError(err);
    }

    return result;
}


//---------------------------------------------------------------------------
// Gui_BlockInput
//---------------------------------------------------------------------------


_FX BOOL Gui_BlockInput(BOOL fBlockIt)
{
    if (fBlockIt)
        return 0;
    else
        return __sys_BlockInput(fBlockIt);
}


//---------------------------------------------------------------------------
// Gui_SendInput
//---------------------------------------------------------------------------


_FX UINT Gui_SendInput(ULONG nInputs, LPINPUT pInputs, ULONG cbInput)
{
    //
    // we are going to pass the input to the system one INPUT structure
    // at a time, so we can check for change in the foreground window
    //

    ULONG retval = 0;
    while (nInputs && nInputs < 16) {

        //
        // if the foreground window is not sandboxed, stop
        //

        HWND hwnd = __sys_GetForegroundWindow();
        if (! hwnd)
            break;
        if (! Gui_IsSameBox(hwnd, NULL, NULL))
            break;

        //
        // otherwise pass one INPUT structure to the system, then advance
        // to the next INPUT structure.  note that we must get a return
        // of exactly 1
        //

        if (__sys_SendInput(1, pInputs, cbInput) != 1)
            break;

        ++retval;
        --nInputs;
        pInputs = (LPINPUT)((UCHAR *)pInputs + cbInput);
    }

    return retval;
}


//---------------------------------------------------------------------------
// Gui_OpenInputDesktop
//---------------------------------------------------------------------------


_FX HDESK Gui_OpenInputDesktop(
    DWORD dwFlags, BOOL fInherit, ACCESS_MASK dwDesiredAccess)
{
    if (! Gui_DummyInputDesktopHandle)
        Gui_DummyInputDesktopHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
    return Gui_DummyInputDesktopHandle;
}


//---------------------------------------------------------------------------
// Gui_GetUserObjectInformationW
//---------------------------------------------------------------------------


_FX BOOL Gui_GetUserObjectInformationW(
    HANDLE hObj, int nIndex,
    void *pvInfo, DWORD nLength, DWORD *lpnLengthNeeded)
{
    if (Gui_DummyInputDesktopHandle && hObj == Gui_DummyInputDesktopHandle && nIndex == UOI_NAME) {

        ULONG Default_len = (7 + 1) * sizeof(WCHAR);
        if (lpnLengthNeeded)
            *lpnLengthNeeded = Default_len;
        if (pvInfo && nLength >= Default_len) {
            memcpy(pvInfo, L"Default", Default_len);
            return TRUE;
        }
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    return __sys_GetUserObjectInformationW(
                        hObj, nIndex, pvInfo, nLength, lpnLengthNeeded);
}


//---------------------------------------------------------------------------
//
// Clipboard support functions
//
// The sandboxed process is running in a restricting job, so it can't
// get clipboard data that was copied by a process outside the sandbox.
//
// a second problem on UIPI systems is that an integrity level is given
// to each clipboard data item, so a process outside the sandbox copies
// data to the clipboard with integrity level zero.  this prevents a
// process outside the sandbox from pasting (perhaps a bug in win32k).
//
// to work around both issues we use the SbieSvc GUI Proxy.
//
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Gui_OpenClipboard
//---------------------------------------------------------------------------


_FX BOOL Gui_OpenClipboard(HWND hwnd)
{
    BOOL ok = __sys_OpenClipboard(hwnd);
    if (ok) {
        Gui_OpenClipboard_hwnd = hwnd;
        Gui_OpenClipboard_tid = GetCurrentThreadId();
        Gui_OpenClipboard_seq = __sys_GetClipboardSequenceNumber();
    }
    return ok;
}


//---------------------------------------------------------------------------
// Gui_CloseClipboard
//---------------------------------------------------------------------------


_FX BOOL Gui_CloseClipboard(void)
{
    //
    // this request is used on Windows Vista and later where we need to
    // adjust the internal integrity level numbers stored in the clipboard.
    // see CloseClipboardSlave in core/svc/GuiServer.cpp,
    // and also see core/drv/gui.c
    //
    // there is a potential problem if there is a clipboard viewer program
    // (registered via SetClipboardViewer or AddClipboardFormatListener),
    // in which case we have a race condition:  do we get to fix the
    // clipboard before the viewer gets the clipboard notification?
    // SbieSvc is an "above normal" priority process, so we rely on that.
    //

    BOOL ok = __sys_CloseClipboard();
    ULONG err = GetLastError();

    if (ok && Dll_OsBuild >= 6000) {

        ULONG new_seq = __sys_GetClipboardSequenceNumber();
        if (new_seq != Gui_OpenClipboard_seq) {

            ULONG req = GUI_CLOSE_CLIPBOARD;

            // While calling the Service we want to suppress
            // any informational msg boxes that the application
            // my through up. This is because we're bringing
            // forward the rendering of "Delay rendered"
            // clipboard types (e.g. the Enhanced Metafile type
            // from Office applications).
            // This causes some issues because sometimes
            // the application (e.g. Excel) will pop up message
            // box where it normally wouldn't do so. There
            // is also no option to turn off that informational
            // message. 
            THREAD_DATA *TlsData = Dll_GetTlsData(NULL);
            ULONG * rpl = NULL;

            if (TlsData)
                TlsData->gui_should_suppress_msgbox = TRUE;

            rpl = Gui_CallProxyEx(
                &req, sizeof(ULONG), sizeof(ULONG), TRUE);
            if (TlsData)
                TlsData->gui_should_suppress_msgbox = FALSE;

            if (rpl)
                Dll_Free(rpl);

            Gui_OpenClipboard_seq = new_seq;
        }
    }

    if (ok) {
        Gui_OpenClipboard_hwnd = NULL;
        Gui_OpenClipboard_tid  = 0;
    }

    SetLastError(err);
    return ok;
}


//---------------------------------------------------------------------------
// Gui_SetClipboardData
//---------------------------------------------------------------------------


_FX HANDLE Gui_SetClipboardData(UINT uFormat, HANDLE hMem)
{
    if (!SbieApi_QueryConfBool(NULL, L"OpenClipboard", TRUE))
        return NULL;

    return __sys_SetClipboardData(uFormat, hMem);
}


//---------------------------------------------------------------------------
// Gui_EmptyClipboard
//---------------------------------------------------------------------------


_FX BOOL Gui_EmptyClipboard()
{
    if (!SbieApi_QueryConfBool(NULL, L"OpenClipboard", TRUE))
        return FALSE;

    return __sys_EmptyClipboard();
}


//---------------------------------------------------------------------------
// Gui_GetClipboardData
//---------------------------------------------------------------------------


_FX HANDLE Gui_GetClipboardData(UINT uFormat)
{
    static HANDLE *_hGlobalClipData = NULL;
    HANDLE hGlobalRet;
    HWND OpenClipboardHwnd;
    GUI_GET_CLIPBOARD_DATA_REQ req;
    GUI_GET_CLIPBOARD_DATA_RPL *rpl;
    ULONG error;
    ULONG retry;

    //
    // first we try the system GetClipboardData.  this always fails with
    // ERROR_ACCESS_DENIED if the process is running in a job that has a
    // UIRestriction which prevents clipboard access.  it fails with
    // ERROR_INVALID_HANDLE if the job has UIRestriction which prevents
    // access to handles outside the job, and the clipboard data belongs
    // to a process outside the job
    //
    // in either case, we go to SbieSvc GUI Proxy Server to ask for the
    // clipboard data
    //

    if (!SbieApi_QueryConfBool(NULL, L"OpenClipboard", TRUE))
        return NULL;

    hGlobalRet = __sys_GetClipboardData(uFormat);
    if (hGlobalRet)
        return hGlobalRet;
    error = GetLastError();
    if (error != ERROR_ACCESS_DENIED && error != ERROR_INVALID_HANDLE)
        return hGlobalRet;

    //
    // we have to close the clipboard before another process can access it
    // (the other process being the SbieSvc GuiProxy process)
    //
    // if we can't close the clipboard (which means the thread has not
    // actually opened the clipboard), then abort
    //

    OpenClipboardHwnd = __sys_GetOpenClipboardWindow();

    if (OpenClipboardHwnd != Gui_OpenClipboard_hwnd ||
                    GetCurrentThreadId() != Gui_OpenClipboard_tid) {
        SetLastError(ERROR_ACCESS_DENIED);
        return NULL;
    }

    if (! __sys_CloseClipboard()) {
        SetLastError(ERROR_ACCESS_DENIED);
        return NULL;
    }

    //
    // ask SbieSvc to call GetClipboardData for us
    //

    req.msgid = GUI_GET_CLIPBOARD_DATA;
    req.format = uFormat;
    rpl = Gui_CallProxyEx(&req, sizeof(req), sizeof(*rpl), TRUE);
    if (! rpl)
        error = GetLastError();
    else {

        if (! rpl->result)
            error = rpl->error;
        else {

            //
            // if the request was successful, we were given a handle to a
            // section which contains the data from the clipboard, but we
            // have to convert this into an HGLOBAL
            //

            void *src = MapViewOfFileEx(
                                (HANDLE)(ULONG_PTR)rpl->section_handle,
                                FILE_MAP_READ, 0, 0, 0, NULL);
            if (! src)
                error = GetLastError();
            else {

                HGLOBAL hGlobal =
                    GlobalAlloc(GMEM_FIXED, (ULONG_PTR)rpl->section_length);
                if (hGlobal) {

                    void *dst = GlobalLock(hGlobal);
                    if (! dst)
                        error = GetLastError();
                    else {

                        memcpy(dst, src, (ULONG_PTR)rpl->section_length);

                        if (uFormat == CF_BITMAP)// || uFormat == CF_DIB)
                            Gui_GetClipboardData_BMP(dst, uFormat);

                        if (uFormat == CF_ENHMETAFILE) {
                            Gui_GetClipboardData_EnhMF(
                                dst, (ULONG)(ULONG_PTR)rpl->section_length,
                                uFormat);
                        }

                        if (uFormat == CF_METAFILEPICT) {
                            Gui_GetClipboardData_MF(
                              dst, (ULONG)(ULONG_PTR)rpl->section_length,
                              uFormat);
                        }

                        GlobalUnlock(hGlobal);

                        hGlobalRet = hGlobal;
                        error = ERROR_SUCCESS;
                    }
                }

                UnmapViewOfFile(src);
            }

            CloseHandle((HANDLE)(ULONG_PTR)rpl->section_handle);
        }

        Dll_Free(rpl);
    }

    //
    // applications aren't supposed to free clipboard HGLOBALs, so we have
    // to free the HGLOBAL we returned last time, and keep the HGLOBAL that
    // we return now, to free it on the next call to this function
    //

    if (hGlobalRet) {

        if (_hGlobalClipData)
        {   
            // reuse the previous returned data if it is the same
            BOOL    bReuse = FALSE;
            SIZE_T  nSizeOld = GlobalSize(_hGlobalClipData);
            SIZE_T  nSizeNew = GlobalSize(hGlobalRet);
            
            if (nSizeOld && nSizeOld == nSizeNew)
            {
                if (memcmp(hGlobalRet, _hGlobalClipData, nSizeOld) == 0)
                {
                    bReuse = TRUE;
                }
            }

            if (bReuse)
            {
                GlobalFree(hGlobalRet);
                hGlobalRet = _hGlobalClipData;
            }
            else
            {
                GlobalFree(_hGlobalClipData);
            }
        }
        _hGlobalClipData = hGlobalRet;

        //
        // if this is a bitmap paste, Gui_GetClipboardData_BMP should have
        // stored the HBITMAP handle at the top of the global data buffer
        //

        if (uFormat == CF_BITMAP) { // || uFormat == CF_DIB) {

            void *dst = GlobalLock(_hGlobalClipData);
            hGlobalRet = *(HANDLE *)dst;
            GlobalUnlock(_hGlobalClipData);
        }
        else if (uFormat == CF_ENHMETAFILE)
        {
            void *dst = GlobalLock(_hGlobalClipData);
            hGlobalRet = *(HANDLE *)dst;
            GlobalUnlock(_hGlobalClipData);
        }
    }

    //
    // grab ownership of the clipboard again before returning
    //

    for (retry = 0; retry < 5000; ++retry) {
        BOOL ok = Gui_OpenClipboard(OpenClipboardHwnd);
        if (ok)
            break;
        Sleep(1);
    }

    SetLastError(error);

    return hGlobalRet;
}


//---------------------------------------------------------------------------
// Gui_GetClipboardData_BMP
//---------------------------------------------------------------------------


_FX void Gui_GetClipboardData_BMP(void *buf, ULONG fmt)
{
    //
    // GetClipboardData(CF_BITMAP) returns an HBITMAP that is valid in the
    // context of SbieSvc.  GuiServer::GetClipboardBitmapSlave already sent
    // us the bitmap data in the reply buffer, we just need to turn it into
    // a local HBITMAP handle
    //

    P_CreateCompatibleBitmap    CreateCompatibleBitmap = NULL;
    P_SetDIBits                 SetDIBits = NULL;
    P_GetDC                     GetDC = NULL;
    P_ReleaseDC                 ReleaseDC = NULL;

    HBITMAP hBitmap = NULL;

    if (fmt != CF_BITMAP)
        SbieApi_Log(2205, L"Clipboard Bitmap (fmt %04X)", fmt);

    else {
        BITMAPINFO *pBitmapInfo = (BITMAPINFO *)buf;

        CreateCompatibleBitmap  = Ldr_GetProcAddrNew(DllName_gdi32, L"CreateCompatibleBitmap", "CreateCompatibleBitmap");
        SetDIBits               = Ldr_GetProcAddrNew(DllName_gdi32, L"SetDIBits", "SetDIBits");
        GetDC                   = Ldr_GetProcAddrNew(DllName_user32, L"GetDC", "GetDC");
        ReleaseDC               = Ldr_GetProcAddrNew(DllName_user32, L"ReleaseDC", "ReleaseDC");

        if (CreateCompatibleBitmap && GetDC && SetDIBits && ReleaseDC)
        {
            HDC hdc = GetDC(NULL);

            if(hdc)
            {
                hBitmap = CreateCompatibleBitmap(hdc, pBitmapInfo->bmiHeader.biWidth, pBitmapInfo->bmiHeader.biHeight);

                if (hBitmap)
                {
                    SetDIBits(hdc, hBitmap, 0, pBitmapInfo->bmiHeader.biHeight, (UCHAR *)buf + 128, pBitmapInfo, DIB_RGB_COLORS);
                }

                ReleaseDC(NULL, hdc);
            }
        }
    }
    

    *(HANDLE *)buf = hBitmap;
}


//---------------------------------------------------------------------------
// Gui_GetClipboardData_EnhMF
//---------------------------------------------------------------------------


_FX void Gui_GetClipboardData_EnhMF(void *buf, ULONG sz, ULONG fmt)
{
    //
    // GetClipboardData(CF_ENHMETAFILE) returns an HENHMETAFILE object.
    //
    P_SetEnhMetaFileBits SetEnhMetaFileBits;
    HENHMETAFILE hEnhMetaFile;

    if (fmt != CF_ENHMETAFILE) {
        SbieApi_Log(2205, L"Clipboard Enhanced MetaFile (fmt %04X sz %d)", fmt, sz);
        return;
    }

    SetEnhMetaFileBits = Ldr_GetProcAddrNew(DllName_gdi32, L"SetEnhMetaFileBits","SetEnhMetaFileBits");
    if (! SetEnhMetaFileBits)
        return;

    hEnhMetaFile = SetEnhMetaFileBits(sz, buf);
    *(HANDLE*)buf = hEnhMetaFile;
}


//---------------------------------------------------------------------------
// Gui_GetClipboardData_MF
//---------------------------------------------------------------------------


_FX void Gui_GetClipboardData_MF(void *buf, ULONG sz, ULONG fmt)
{
    GUI_GET_CLIPBOARD_DATA_REQ req;
    GUI_GET_CLIPBOARD_DATA_RPL *rpl;
    P_SetMetaFileBitsEx SetMetaFileBitsEx;

    //
    // GetClipboardData(CF_METAFILEPICT) returns a structure which contains
    // an HMETAFILE handle that is valid in the context of SbieSvc, so we
    // make a secondary call to SbieSvc to get the data bytes so that we can
    // create a local HMETAFILE handle
    //

    if (!SbieApi_QueryConfBool(NULL, L"OpenClipboard", TRUE))
        return;

    if ((fmt != CF_METAFILEPICT) || (sz != sizeof(METAFILEPICT))) {
        SbieApi_Log(2205, L"Clipboard MetaFile (fmt %04X sz %d)", fmt, sz);
        return;
    }

    SetMetaFileBitsEx = Ldr_GetProcAddrNew(DllName_gdi32, L"SetMetaFileBitsEx","SetMetaFileBitsEx");
    if (! SetMetaFileBitsEx)
        return;

    req.msgid = GUI_GET_CLIPBOARD_METAFILE;
    req.format = fmt;
    rpl = Gui_CallProxyEx(&req, sizeof(req), sizeof(*rpl), TRUE);
    if (! rpl)
        return;

    if (rpl->result) {

        //
        // if the request was successful, we were given a handle to a
        // section which contains the data from the clipboard, but we
        // have to convert this into an HGLOBAL
        //

        void *src = MapViewOfFileEx(
                            (HANDLE)(ULONG_PTR)rpl->section_handle,
                            FILE_MAP_READ, 0, 0, 0, NULL);
        if (src) {

            HGLOBAL hGlobal =
                GlobalAlloc(GMEM_FIXED, (ULONG_PTR)rpl->section_length);
            if (hGlobal) {

                void *dst = GlobalLock(hGlobal);
                if (dst) {

                    HMETAFILE hmf = SetMetaFileBitsEx(
                        (ULONG)(ULONG_PTR)rpl->section_length, src);

                    if (hmf) {

                        ((METAFILEPICT *)buf)->hMF = hmf;
                    }

                    GlobalUnlock(hGlobal);
                }
            }

            UnmapViewOfFile(src);
        }

        CloseHandle((HANDLE)(ULONG_PTR)rpl->section_handle);
    }

    Dll_Free(rpl);
}


//---------------------------------------------------------------------------
// Gui_ChangeDisplaySettingsEx_impl
//---------------------------------------------------------------------------


_FX LONG Gui_ChangeDisplaySettingsEx_impl(
    void* lpszDeviceName, void* lpDevMode, HWND hwnd,
    DWORD dwflags, void* lParam, BOOLEAN bUnicode)
{
    GUI_CHANGE_DISPLAY_SETTINGS_REQ req;
    GUI_CHANGE_DISPLAY_SETTINGS_RPL* rpl;

    if ((dwflags & ~(CDS_UNKNOWNFLAG | CDS_RESET | CDS_FULLSCREEN | CDS_TEST)) || lParam || hwnd) {
        SbieApi_Log(2205, L"ChangeDisplaySettingsEx %08X", dwflags);
        SetLastError(ERROR_ACCESS_DENIED);
        return DISP_CHANGE_FAILED;
    }

    req.msgid = GUI_CHANGE_DISPLAY_SETTINGS;
    req.flags = dwflags;
    req.unicode = bUnicode;

    if (lpszDeviceName) {
        if (bUnicode) {
            WCHAR* name = (WCHAR*)req.devname;
            ULONG len = wcslen(lpszDeviceName);
            if (len > 62)
                len = 62;
            wmemcpy(name, lpszDeviceName, len);
            name[len] = L'\0';
        }
        else {
            UCHAR* name = (UCHAR*)req.devname;
            ULONG len = strlen(lpszDeviceName);
            if (len > 62)
                len = 62;
            memcpy(name, lpszDeviceName, len);
            name[len] = L'\0';
        }
        req.have_devname = TRUE;
    }
    else {
        memzero(req.devname, sizeof(req.devname));
        req.have_devname = FALSE;
    }

    if (lpDevMode) {
        memcpy(&req.devmode, lpDevMode, bUnicode ? sizeof(DEVMODEW) : sizeof(DEVMODEA));
        req.have_devmode = TRUE;
    }
    else
        req.have_devmode = FALSE;

    rpl = Gui_CallProxy(&req, sizeof(req), sizeof(*rpl));
    if (!rpl)
        return DISP_CHANGE_FAILED;
    else {
        ULONG error = rpl->error;
        ULONG retval = rpl->retval;
        Dll_Free(rpl);
        SetLastError(error);
        return retval;
    }
}

//---------------------------------------------------------------------------
// Gui_ChangeDisplaySettingsExA
//---------------------------------------------------------------------------


_FX LONG Gui_ChangeDisplaySettingsExA(
    void *lpszDeviceName, void *lpDevMode, HWND hwnd,
    DWORD dwflags, void *lParam)
{
    return Gui_ChangeDisplaySettingsEx_impl(lpszDeviceName, lpDevMode, hwnd, dwflags, lParam, FALSE);
}


//---------------------------------------------------------------------------
// Gui_ChangeDisplaySettingsExW
//---------------------------------------------------------------------------


_FX LONG Gui_ChangeDisplaySettingsExW(
    void *lpszDeviceName, void *lpDevMode, HWND hwnd,
    DWORD dwflags, void *lParam)
{
    return Gui_ChangeDisplaySettingsEx_impl(lpszDeviceName, lpDevMode, hwnd, dwflags, lParam, TRUE);
}


//---------------------------------------------------------------------------
// Gui_GetRawInputDeviceInfo_impl
//---------------------------------------------------------------------------


_FX LONG Gui_GetRawInputDeviceInfo_impl(
    _In_opt_ HANDLE hDevice, _In_ UINT uiCommand,
    _Inout_ LPVOID pData, _Inout_ PUINT pcbSize, BOOLEAN bUnicode)
{
    GUI_GET_RAW_INPUT_DEVICE_INFO_REQ* req;
    GUI_GET_RAW_INPUT_DEVICE_INFO_RPL* rpl;

    ULONG lenData = 0;
    if (pData && pcbSize) {
        lenData = *pcbSize;
        if (uiCommand == RIDI_DEVICENAME && bUnicode) {
            lenData *= sizeof(WCHAR);
        }
    }

    ULONG reqSize = sizeof(GUI_GET_RAW_INPUT_DEVICE_INFO_REQ) + lenData + 10;
    req = Dll_Alloc(reqSize);

    LPVOID reqData = (BYTE*)req + sizeof(GUI_GET_RAW_INPUT_DEVICE_INFO_REQ);

    req->msgid = GUI_GET_RAW_INPUT_DEVICE_INFO;
    req->hDevice = (ULONG64)hDevice;
    req->uiCommand = uiCommand;
    req->unicode = bUnicode;
    req->hasData = !!pData;

    if (lenData)
        memcpy(reqData, pData, lenData);

    // GetRawInputDeviceInfoA accesses pcbSize without testing it for being not NULL 
    // hence if the caller passes NULL we use a dummy value so that we don't crash the helper service
    if (pcbSize)
        req->cbSize = *pcbSize;
    else
        req->cbSize = 0;

    rpl = Gui_CallProxy(req, reqSize, sizeof(*rpl));

    Dll_Free(req);

    if (!rpl)
        return -1;

    ULONG error = rpl->error;
    ULONG retval = rpl->retval;

    if (pcbSize)
        *pcbSize = rpl->cbSize;

    if (lenData) {
        LPVOID rplData = (BYTE*)rpl + sizeof(GUI_GET_RAW_INPUT_DEVICE_INFO_RPL);
        memcpy(pData, rplData, lenData);
    }

    Dll_Free(rpl);
    SetLastError(error);
    return retval;
}


//---------------------------------------------------------------------------
// Gui_GetRawInputDeviceInfoA
//---------------------------------------------------------------------------


_FX LONG Gui_GetRawInputDeviceInfoA(
    _In_opt_ HANDLE hDevice, _In_ UINT uiCommand,
    _Inout_ LPVOID pData, _Inout_ PUINT pcbSize)
{
    return Gui_GetRawInputDeviceInfo_impl(hDevice, uiCommand, pData, pcbSize, FALSE);
}


//---------------------------------------------------------------------------
// Gui_GetRawInputDeviceInfoW
//---------------------------------------------------------------------------


_FX LONG Gui_GetRawInputDeviceInfoW(
    _In_opt_ HANDLE hDevice, _In_ UINT uiCommand,
    _Inout_ LPVOID pData, _Inout_ PUINT pcbSize)
{
    return Gui_GetRawInputDeviceInfo_impl(hDevice, uiCommand, pData, pcbSize, TRUE);
}


//---------------------------------------------------------------------------
//
// IMM32 - ImmAssociateContext and ImmAssociateContextEx
//
// kernel side handlers for ImmAssociateContext and ImmAssociateContextEx
// check that the specified HWND and HIMC objects are on the same desktop.
// but the default HIMC is created during WIN32K thread creation, at which
// time the thread is connected to a dummy desktop (see more about this in
// Gui_ConnectToWindowStationAndDesktop).
//
// when a thread calls these functions it is already connected to the real
// desktop, so we can easily work around this problem by creating a new
// HIMC object in the real desktop and passing it instead.
//
//---------------------------------------------------------------------------


static ULONG_PTR Gui_ImmAssociateContext(ULONG_PTR hwnd, ULONG_PTR imc);

static BOOL Gui_ImmAssociateContextEx(
    ULONG_PTR hwnd, ULONG_PTR imc, ULONG flags);

typedef ULONG_PTR (*P_ImmAssociateContext)(ULONG_PTR hwnd, ULONG_PTR imc);

typedef BOOL (*P_ImmAssociateContextEx)(
                ULONG_PTR hwnd, ULONG_PTR imc, ULONG flags);

typedef ULONG_PTR (*P_ImmCreateContext)(void);

static P_ImmAssociateContext    __sys_ImmAssociateContext   = NULL;
static P_ImmAssociateContextEx  __sys_ImmAssociateContextEx = NULL;
static P_ImmCreateContext       __sys_ImmCreateContext      = NULL;


//---------------------------------------------------------------------------
// Gui_Init_IMM32
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_Init_IMM32(HMODULE module)
{
    // NoSbieDesk BEGIN
    if (Dll_CompartmentMode || SbieApi_QueryConfBool(NULL, L"NoSandboxieDesktop", FALSE))
        return TRUE;
	// NoSbieDesk END

    __sys_ImmAssociateContext = (P_ImmAssociateContext)
                GetProcAddress(module, "ImmAssociateContext");

    __sys_ImmAssociateContextEx = (P_ImmAssociateContextEx)
                GetProcAddress(module, "ImmAssociateContextEx");

    __sys_ImmCreateContext = (P_ImmCreateContext)
                GetProcAddress(module, "ImmCreateContext");

    if (__sys_ImmCreateContext) {

        if (__sys_ImmAssociateContext) {
            SBIEDLL_HOOK_GUI(ImmAssociateContext);
        }

        if (__sys_ImmAssociateContextEx) {
            SBIEDLL_HOOK_GUI(ImmAssociateContextEx);
        }
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_ImmAssociateContext
//---------------------------------------------------------------------------


_FX ULONG_PTR Gui_ImmAssociateContext(ULONG_PTR hwnd, ULONG_PTR imc)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    imc = __sys_ImmAssociateContext(hwnd, imc);
    if ((! imc) && (GetLastError() == ERROR_ACCESS_DENIED)) {

        imc = TlsData->gui_himc;
        if (! imc) {
            imc = __sys_ImmCreateContext();
            if (! imc) {
                SetLastError(ERROR_ACCESS_DENIED);
                return FALSE;
            }
            TlsData->gui_himc = imc;
        }

        imc = __sys_ImmAssociateContext(hwnd, imc);
    }

    return imc;
}


//---------------------------------------------------------------------------
// Gui_ImmAssociateContextEx
//---------------------------------------------------------------------------


_FX BOOL Gui_ImmAssociateContextEx(
    ULONG_PTR hwnd, ULONG_PTR imc, ULONG flags)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    BOOL ok = __sys_ImmAssociateContextEx(hwnd, imc, flags);
    if ((! ok) && (GetLastError() == ERROR_ACCESS_DENIED)) {

        imc = TlsData->gui_himc;
        if (! imc) {
            imc = __sys_ImmCreateContext();
            if (! imc) {
                SetLastError(ERROR_ACCESS_DENIED);
                return FALSE;
            }
            TlsData->gui_himc = imc;
        }

        ok = __sys_ImmAssociateContextEx(hwnd, imc, 0);
    }

    return ok;
}


//---------------------------------------------------------------------------
// Gui_GetDC
//---------------------------------------------------------------------------


_FX HDC Gui_GetDC(HWND hWnd)
{
	HDC ret = __sys_GetDC(hWnd);

	ULONG_PTR pid = 0, tid = 0;
	if (Gui_UseBlockCapture && (hWnd == NULL || hWnd == __sys_GetDesktopWindow() || !Gui_IsSameBox(hWnd, &pid, &tid))) {

        return Gdi_GetDummyDC(ret, hWnd);
	}

	return ret;
}


//---------------------------------------------------------------------------
// Gui_GetWindowDC
//---------------------------------------------------------------------------


_FX HDC Gui_GetWindowDC(HWND hWnd)
{
	HDC ret = __sys_GetWindowDC(hWnd);

	ULONG_PTR pid = 0, tid = 0;
	if (Gui_UseBlockCapture && (hWnd == NULL || hWnd == __sys_GetDesktopWindow() || !Gui_IsSameBox(hWnd, &pid, &tid))) {

		return Gdi_GetDummyDC(ret, hWnd);
	}

	return ret;
}


//---------------------------------------------------------------------------
// Gui_GetDCEx
//---------------------------------------------------------------------------


_FX HDC Gui_GetDCEx(HWND hWnd, HRGN hrgnClip, DWORD flags)
{
	HDC ret = __sys_GetDCEx(hWnd, hrgnClip, flags);

	ULONG_PTR pid = 0, tid = 0;
	if (Gui_UseBlockCapture && (hWnd == NULL || hWnd == __sys_GetDesktopWindow() || !Gui_IsSameBox(hWnd, &pid, &tid))) {

		return Gdi_GetDummyDC(ret, hWnd);
	}

	return ret;
}


//---------------------------------------------------------------------------
// Gui_PrintWindow
//---------------------------------------------------------------------------


_FX BOOL Gui_PrintWindow(HWND hwnd, HDC hdcBlt, UINT nFlags)
{
	if (Gui_UseBlockCapture) {
	
		if (hwnd == NULL || hwnd == __sys_GetDesktopWindow()) {
		
			SetLastError(ERROR_ACCESS_DENIED);
			return 0;
		}

		ULONG_PTR pid = 0, tid = 0;
		if (!Gui_IsSameBox(hwnd, &pid, &tid)) {
		
			SetLastError(ERROR_ACCESS_DENIED);
			return 0;
		}
	}
	return __sys_PrintWindow(hwnd, hdcBlt, nFlags);
}


//---------------------------------------------------------------------------
// Gui_ReleaseDC
//---------------------------------------------------------------------------


_FX int Gui_ReleaseDC(HWND hWnd, HDC hdc) 
{
    hdc = Gdi_OnFreeDC(hdc);
    if (!hdc) 
        return 1;
	return __sys_ReleaseDC(hWnd, hdc);
}


//---------------------------------------------------------------------------
// Gui_ShutdownBlockReasonCreate
//---------------------------------------------------------------------------


_FX BOOL Gui_ShutdownBlockReasonCreate(HWND hWnd, LPCWSTR pwszReason) 
{
	SetLastError(ERROR_ACCESS_DENIED);
	return 0;
	//return __sys_ShutdownBlockReasonCreate(hWnd, pwszReason);
}


//---------------------------------------------------------------------------
// Gui_SetTimer
//---------------------------------------------------------------------------


_FX UINT_PTR Gui_SetTimer(HWND hWnd, UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc)
{
	ULONG add = SbieApi_QueryConfNumber(NULL, L"AddTimerSpeed", 1), low = SbieApi_QueryConfNumber(NULL, L"LowTimerSpeed", 1);
	if (add != 0 && low != 0) {
        UINT64 newElapse = uElapse;
        newElapse = newElapse * low / add;
		return __sys_SetTimer(hWnd, nIDEvent, (UINT)newElapse, lpTimerFunc);
    }

	return 0;
}

//---------------------------------------------------------------------------
// WINMM_timeGetTime
//---------------------------------------------------------------------------


_FX DWORD Wimm_timeGetTime()
{
    ULONG add = SbieApi_QueryConfNumber(NULL, L"AddTimerSpeed", 1), low = SbieApi_QueryConfNumber(NULL, L"LowTimerSpeed", 1);
    ULONG64 time = __sys_timeGetTime();
    if(add != 0 && low != 0) {
        time = Dll_FirsttimeGetTimeValue + (time - Dll_FirsttimeGetTimeValue) * add / low; // multi
    }
    
    return (DWORD)time;
}


//---------------------------------------------------------------------------
// WINMM_timeSetEvent
//---------------------------------------------------------------------------


_FX MMRESULT Wimm_timeSetEvent(UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc, DWORD_PTR dwUser, UINT fuEvent)
{
	ULONG add = SbieApi_QueryConfNumber(NULL, L"AddTimerSpeed", 1), low = SbieApi_QueryConfNumber(NULL, L"LowTimerSpeed", 1);
	if (add != 0 && low != 0) {
        UINT64 newDelay = uDelay;
        newDelay = newDelay * low / add;
        return __sys_timeSetEvent((UINT)newDelay, uResolution, lpTimeProc, dwUser, fuEvent);
    }
	
    return 0;
}
//---------------------------------------------------------------------------
// Gui_ShowCursor
//---------------------------------------------------------------------------


_FX int Gui_ShowCursor(BOOL bShow) 
{
	if (Gui_BlockInterferenceControl && !bShow)
		return 0;
	return __sys_ShowCursor(bShow);
}


//---------------------------------------------------------------------------
// Gui_SetActiveWindow
//---------------------------------------------------------------------------


_FX HWND Gui_SetActiveWindow(HWND hWnd) 
{
	if (Gui_BlockInterferenceControl)
		return NULL;
	return __sys_SetActiveWindow(hWnd);
}


//---------------------------------------------------------------------------
// Gui_BringWindowToTop
//---------------------------------------------------------------------------


_FX BOOL  Gui_BringWindowToTop(HWND hWnd) 
{
	if (Gui_BlockInterferenceControl)
		return FALSE;
	return __sys_BringWindowToTop(hWnd);
}


//---------------------------------------------------------------------------
// Gui_SwitchToThisWindow
//---------------------------------------------------------------------------


_FX void Gui_SwitchToThisWindow(HWND hWnd, BOOL fAlt) 
{
	if (Gui_BlockInterferenceControl)
		return;
	__sys_SwitchToThisWindow(hWnd, fAlt);
}
