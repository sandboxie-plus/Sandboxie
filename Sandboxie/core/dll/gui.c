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

#define GUI_DEFINE_SYS_VARIABLES
#include "gui_p.h"
#include "taskbar.h"
#include "core/svc/GuiWire.h"
#include "common/my_version.h"
#include "core/drv/api_defs.h"
#include <stdio.h>
#include <psapi.h>


//---------------------------------------------------------------------------
// Function Pointers in USER32.DLL
//---------------------------------------------------------------------------


        P_SetThreadDesktop          __sys_SetThreadDesktop          = NULL;
        P_SwitchDesktop             __sys_SwitchDesktop             = NULL;

        P_UserHandleGrantAccess     __sys_UserHandleGrantAccess     = NULL;

        P_GetFocus                  __sys_GetFocus                  = NULL;

        P_ShowWindow                __sys_ShowWindow                = NULL;

        P_ExitWindowsEx             __sys_ExitWindowsEx             = NULL;

        P_EndTask                   __sys_EndTask                   = NULL;

        P_ConsoleControl            __sys_ConsoleControl            = NULL;

        P_SwapMouseButton           __sys_SwapMouseButton           = NULL;
        P_SetDoubleClickTime        __sys_SetDoubleClickTime        = NULL;

        P_AnimateWindow             __sys_AnimateWindow             = NULL;

        P_GetClipboardFormatName    __sys_GetClipboardFormatNameA   = NULL;
        P_GetClipboardFormatName    __sys_GetClipboardFormatNameW   = NULL;

        P_RegisterClipboardFormat   __sys_RegisterClipboardFormatA  = NULL;
        P_RegisterClipboardFormat   __sys_RegisterClipboardFormatW  = NULL;

        P_RealGetWindowClass        __sys_RealGetWindowClassA       = NULL;
        P_RealGetWindowClass        __sys_RealGetWindowClassW       = NULL;

        P_GetWindowText             __sys_GetWindowTextA            = NULL;
        P_GetWindowText             __sys_GetWindowTextW            = NULL;

        P_CallWindowProc            __sys_CallWindowProcA           = NULL;
        P_CallWindowProc            __sys_CallWindowProcW           = NULL;

        P_CreateWindowEx            __sys_CreateWindowExA           = NULL;
        P_CreateWindowEx            __sys_CreateWindowExW           = NULL;

        P_DefWindowProc             __sys_DefWindowProcA            = NULL;
        P_DefWindowProc             __sys_DefWindowProcW            = NULL;

        P_ActivateKeyboardLayout    __sys_ActivateKeyboardLayout    = NULL;

        P_MoveWindow                __sys_MoveWindow                = NULL;
        P_SetWindowPos              __sys_SetWindowPos              = NULL;

        P_GetWindowInfo             __sys_GetWindowInfo             = NULL;

        P_RegisterClass             __sys_RegisterClassA            = NULL;
        P_RegisterClass             __sys_RegisterClassW            = NULL;
        P_RegisterClass             __sys_RegisterClassExA          = NULL;
        P_RegisterClass             __sys_RegisterClassExW          = NULL;

        P_UnregisterClass           __sys_UnregisterClassA          = NULL;
        P_UnregisterClass           __sys_UnregisterClassW          = NULL;

        P_GetClassInfo              __sys_GetClassInfoA             = NULL;
        P_GetClassInfo              __sys_GetClassInfoW             = NULL;
        P_GetClassInfo              __sys_GetClassInfoExA           = NULL;
        P_GetClassInfo              __sys_GetClassInfoExW           = NULL;

        P_GetClassName              __sys_GetClassNameA             = NULL;
        P_GetClassName              __sys_GetClassNameW             = NULL;

        P_EnumWindows               __sys_EnumWindows               = NULL;
        P_EnumChildWindows          __sys_EnumChildWindows          = NULL;
        P_EnumThreadWindows         __sys_EnumThreadWindows         = NULL;
        P_EnumDesktopWindows        __sys_EnumDesktopWindows        = NULL;

        P_EnumDesktops              __sys_EnumDesktopsA             = NULL;
        P_EnumDesktops              __sys_EnumDesktopsW             = NULL;

        P_FindWindow                __sys_FindWindowA               = NULL;
        P_FindWindow                __sys_FindWindowW               = NULL;

        P_FindWindowEx              __sys_FindWindowExA             = NULL;
        P_FindWindowEx              __sys_FindWindowExW             = NULL;

        P_GetDesktopWindow          __sys_GetDesktopWindow          = NULL;
        P_GetShellWindow            __sys_GetShellWindow            = NULL;

        P_GetProp                   __sys_GetPropA                  = NULL;
        P_GetProp                   __sys_GetPropW                  = NULL;

        P_SetProp                   __sys_SetPropA                  = NULL;
        P_SetProp                   __sys_SetPropW                  = NULL;

        P_RemoveProp                __sys_RemovePropA               = NULL;
        P_RemoveProp                __sys_RemovePropW               = NULL;

        P_GetWindowLong             __sys_GetWindowLongA            = NULL;
        P_GetWindowLong             __sys_GetWindowLongW            = NULL;

        P_SetWindowLong             __sys_SetWindowLongA            = NULL;
        P_SetWindowLong             __sys_SetWindowLongW            = NULL;

        P_GetClassLong              __sys_GetClassLongA             = NULL;
        P_GetClassLong              __sys_GetClassLongW             = NULL;

#ifdef _WIN64

        P_GetWindowLongPtr          __sys_GetWindowLongPtrA         = NULL;
        P_GetWindowLongPtr          __sys_GetWindowLongPtrW         = NULL;

        P_SetWindowLongPtr          __sys_SetWindowLongPtrA         = NULL;
        P_SetWindowLongPtr          __sys_SetWindowLongPtrW         = NULL;

        P_GetClassLongPtr           __sys_GetClassLongPtrA          = NULL;
        P_GetClassLongPtr           __sys_GetClassLongPtrW          = NULL;

#endif _WIN64

        P_SetWindowsHookEx          __sys_SetWindowsHookExA         = NULL;
        P_SetWindowsHookEx          __sys_SetWindowsHookExW         = NULL;

        P_UnhookWindowsHookEx       __sys_UnhookWindowsHookEx       = NULL;

        P_CreateDialogParam         __sys_CreateDialogParamA        = NULL;
        P_CreateDialogParam         __sys_CreateDialogParamW        = NULL;

        P_CreateDialogIndirectParam __sys_CreateDialogIndirectParamA
                                                                    = NULL;
        P_CreateDialogIndirectParam __sys_CreateDialogIndirectParamW
                                                                    = NULL;
        P_CreateDialogIndirectParamAorW
                                    __sys_CreateDialogIndirectParamAorW
                                                                    = NULL;

        P_DialogBoxParam            __sys_DialogBoxParamA           = NULL;
        P_DialogBoxParam            __sys_DialogBoxParamW           = NULL;

        P_DialogBoxIndirectParam    __sys_DialogBoxIndirectParamA   = NULL;
        P_DialogBoxIndirectParam    __sys_DialogBoxIndirectParamW   = NULL;

        P_DialogBoxIndirectParamAorW
                                    __sys_DialogBoxIndirectParamAorW
                                                                    = NULL;

        P_LoadString                __sys_LoadStringW               = NULL;

        P_RegisterDeviceNotification
                                    __sys_RegisterDeviceNotificationA
                                                                    = NULL;
        P_RegisterDeviceNotification
                                    __sys_RegisterDeviceNotificationW
                                                                    = NULL;
        P_UnregisterDeviceNotification
                                    __sys_UnregisterDeviceNotification
                                                                    = NULL;
        P_MsgWaitForMultipleObjects __sys_MsgWaitForMultipleObjects = NULL;

        P_PeekMessage               __sys_PeekMessageA              = NULL;
        P_PeekMessage               __sys_PeekMessageW              = NULL;

        P_MessageBoxW               __sys_MessageBoxW               = NULL;
        P_MessageBoxExW             __sys_MessageBoxExW             = NULL;

        P_WaitForInputIdle          __sys_WaitForInputIdle          = NULL;

        P_AttachThreadInput         __sys_AttachThreadInput         = NULL;

        P_GetOpenFileNameW          __sys_GetOpenFileNameW          = NULL;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN Gui_Init2(void);

static BOOLEAN Gui_Init3(void);

static BOOL Gui_SetThreadDesktop(HDESK hDesktop);

static BOOL Gui_SwitchDesktop(HDESK hDesktop);

static BOOL Gui_UserHandleGrantAccess(
    HANDLE hUserHandle, HANDLE hJob, BOOL bGrant);

static HWND Gui_CreateDummyParentWindow(void);

static HWND Gui_CreateWindowExA(
    DWORD dwExStyle,
    UCHAR *lpClassName,
    void *lpWindowName,
    DWORD dwStyle,
    int x,
    int y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam);

static HWND Gui_CreateWindowExW(
    DWORD dwExStyle,
    void *lpClassName,
    void *lpWindowName,
    DWORD dwStyle,
    int x,
    int y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam);

static BOOLEAN Gui_CanForwardMsg(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam/*, LRESULT* plResult*/);

static LRESULT Gui_DefWindowProcA(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static LRESULT Gui_DefWindowProcW(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static ULONG_PTR Gui_ActivateKeyboardLayout(ULONG_PTR hkl, UINT flags);

static BOOL Gui_ExitWindowsEx(UINT uFlags, DWORD dwReason);

static BOOL Gui_IsWindowCommon(HWND hwnd, UCHAR which);

static BOOL Gui_IsWindow(HWND hWnd);

static BOOL Gui_IsWindowEnabled(HWND hWnd);

BOOL Gui_IsWindowVisible(HWND hWnd);

BOOL Gui_IsWindowUnicode(HWND hWnd);

BOOL Gui_IsIconic(HWND hWnd);

BOOL Gui_IsZoomed(HWND hWnd);

static BOOL Gui_MoveWindow(
    HWND hWnd, int x, int y, int w, int h, BOOL bRepaint);

static BOOL Gui_SetWindowPos(
    HWND hWnd, HWND hWndInsertAfter, int x, int y, int w, int h, UINT flags);

static int Gui_MessageBoxW(
    HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);

static int Gui_MessageBoxExW(
    HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType, WORD wLanguageId);    

static BOOL Gui_EndTask(
    HWND hWnd, BOOL fShutDown, BOOL fForce);

static BOOL Gui_ConsoleControl(
    ULONG ctlcode, ULONG *data, ULONG_PTR unknown);

static ULONG_PTR Gui_RegisterDeviceNotificationA(
    HANDLE hRecipient, LPVOID NotificationFilter, DWORD Flags);

static ULONG_PTR Gui_RegisterDeviceNotificationW(
    HANDLE hRecipient, LPVOID NotificationFilter, DWORD Flags);

static BOOL Gui_UnregisterDeviceNotification(ULONG_PTR Handle);

static int Gui_MapWindowPoints(
    HWND hWndFrom, HWND hWndTo, LPPOINT lpPoints, UINT cPoints);

static BOOL Gui_ClientToScreen(HWND hWnd, LPPOINT lpPoint);

static BOOL Gui_ScreenToClient(HWND hWnd, LPPOINT lpPoint);

static int Gui_GetClientRect(HWND hWnd, LPRECT lpRect);

static int Gui_GetWindowRect(HWND hWnd, LPRECT lpRect);

static int Gui_GetWindowInfo(HWND hWnd, PWINDOWINFO pwi);

static BOOL Gui_AnimateWindow(HWND hwnd, ULONG time, ULONG flags);

static DWORD Gui_WaitForInputIdle(HANDLE hProcess, DWORD dwMilliseconds);

static BOOL Gui_AttachThreadInput(DWORD idAttach, DWORD idAttachTo, BOOL fAttach);


//---------------------------------------------------------------------------
// GUI_IMPORT
//---------------------------------------------------------------------------


#define GUI_IMPORT_(base,suffix) {                                      \
    ProcName = #base#suffix;                                            \
    __sys_##base##suffix =                                              \
        (P_##base)GetProcAddress(module, #base#suffix);                 \
    if (! __sys_##base##suffix)                                         \
        goto import_fail;                                               \
    }

#define GUI_IMPORT___(base) GUI_IMPORT_(base,)
#define GUI_IMPORT__W(base) GUI_IMPORT_(base,W)
#define GUI_IMPORT_AW(base) GUI_IMPORT_(base,A) GUI_IMPORT_(base,W)


//---------------------------------------------------------------------------
// Gui_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_Init(HMODULE module)
{
    //
    // import functions
    //

    BOOLEAN ok = TRUE;

    const UCHAR *ProcName;

    if (! Gdi_InitZero())       // only if Gdi_Init was not called yet
        return FALSE;

    GUI_IMPORT___(GetWindowThreadProcessId);
    GUI_IMPORT___(SetThreadDesktop);
    GUI_IMPORT___(SwitchDesktop);
    GUI_IMPORT___(UserHandleGrantAccess);
    GUI_IMPORT___(GetWindow);
    GUI_IMPORT___(GetParent);
    GUI_IMPORT___(SetParent);
    GUI_IMPORT___(GetFocus);
    GUI_IMPORT___(GetForegroundWindow);
    GUI_IMPORT___(IsWindow);
    GUI_IMPORT___(IsWindowEnabled);
    GUI_IMPORT___(IsWindowVisible);
    GUI_IMPORT___(IsWindowUnicode);
    GUI_IMPORT___(IsIconic);
    GUI_IMPORT___(IsZoomed);
    GUI_IMPORT___(ShowWindow);
    GUI_IMPORT___(ClipCursor);
    GUI_IMPORT___(GetClipCursor);
    GUI_IMPORT___(GetCursorPos);
    GUI_IMPORT___(SetCursorPos);

    GUI_IMPORT___(MsgWaitForMultipleObjects);
    GUI_IMPORT_AW(PeekMessage);
    GUI_IMPORT___(MessageBoxW);
    GUI_IMPORT___(MessageBoxExW);
    GUI_IMPORT___(WaitForInputIdle);

    GUI_IMPORT___(OpenClipboard);
    GUI_IMPORT___(CloseClipboard);
    GUI_IMPORT___(GetClipboardOwner);
    GUI_IMPORT___(GetOpenClipboardWindow);
    GUI_IMPORT___(GetClipboardSequenceNumber);
    GUI_IMPORT_AW(GetClipboardFormatName);
    GUI_IMPORT_AW(RegisterClipboardFormat);
    GUI_IMPORT___(GetClipboardData);

    GUI_IMPORT___(GetRawInputDeviceInfoA);
    GUI_IMPORT___(GetRawInputDeviceInfoW);
    
    GUI_IMPORT___(ExitWindowsEx);
    GUI_IMPORT___(EndTask);
    if (Dll_OsBuild >= 8400) {
        GUI_IMPORT___(ConsoleControl);
    }

    GUI_IMPORT_AW(CallWindowProc);

    GUI_IMPORT_AW(CreateWindowEx);

    GUI_IMPORT_AW(DefWindowProc);
    GUI_IMPORT___(ActivateKeyboardLayout);

    GUI_IMPORT_AW(RealGetWindowClass);
    GUI_IMPORT_AW(GetWindowText);

    GUI_IMPORT___(MoveWindow);
    GUI_IMPORT___(SetWindowPos);
    GUI_IMPORT___(SwapMouseButton);
    GUI_IMPORT___(SetDoubleClickTime);
    GUI_IMPORT___(SetCursor);
    GUI_IMPORT___(GetIconInfo);
    GUI_IMPORT___(AnimateWindow);

    GUI_IMPORT___(BlockInput);
    GUI_IMPORT___(SendInput);

    GUI_IMPORT___(MapWindowPoints);
    GUI_IMPORT___(ClientToScreen);
    GUI_IMPORT___(ScreenToClient);
    GUI_IMPORT___(GetClientRect);
    GUI_IMPORT___(GetWindowRect);
    GUI_IMPORT___(GetWindowInfo);

    GUI_IMPORT_AW(RegisterDeviceNotification);
    GUI_IMPORT___(UnregisterDeviceNotification);

    GUI_IMPORT_AW(RegisterClass);
    GUI_IMPORT_AW(RegisterClassEx);
    GUI_IMPORT_AW(UnregisterClass);
    GUI_IMPORT_AW(GetClassInfo);
    GUI_IMPORT_AW(GetClassInfoEx);
    GUI_IMPORT_AW(GetClassName);

    GUI_IMPORT___(EnumWindows);
    GUI_IMPORT___(EnumChildWindows);
    GUI_IMPORT___(EnumThreadWindows);
    GUI_IMPORT___(EnumDesktopWindows);
    GUI_IMPORT_AW(EnumDesktops);
    GUI_IMPORT_AW(OpenDesktop);
    GUI_IMPORT_AW(CreateDesktop);
    GUI_IMPORT_AW(CreateWindowStation);

    GUI_IMPORT_AW(FindWindow);
    GUI_IMPORT_AW(FindWindowEx);

    GUI_IMPORT___(GetDesktopWindow);
    GUI_IMPORT___(GetShellWindow);

    GUI_IMPORT_AW(GetProp);
    GUI_IMPORT_AW(SetProp);
    GUI_IMPORT_AW(RemoveProp);
    GUI_IMPORT_AW(GetWindowLong);
    GUI_IMPORT_AW(SetWindowLong);
    GUI_IMPORT_AW(GetClassLong);

#ifdef _WIN64

    GUI_IMPORT_AW(GetWindowLongPtr);
    GUI_IMPORT_AW(SetWindowLongPtr);
    GUI_IMPORT_AW(GetClassLongPtr);

#endif _WIN64

    GUI_IMPORT_AW(SendMessage);
    GUI_IMPORT_AW(SendMessageTimeout);
    //GUI_IMPORT_AW(SendMessageCallback);
    GUI_IMPORT_AW(SendNotifyMessage);
    GUI_IMPORT_AW(PostMessage);
    GUI_IMPORT_AW(PostThreadMessage);
    GUI_IMPORT_AW(DispatchMessage);

    GUI_IMPORT_AW(SetWindowsHookEx);
    GUI_IMPORT___(UnhookWindowsHookEx);

    GUI_IMPORT_AW(CreateDialogParam);
    GUI_IMPORT_AW(CreateDialogIndirectParam);
    GUI_IMPORT___(CreateDialogIndirectParamAorW);

    GUI_IMPORT_AW(DialogBoxParam);
    GUI_IMPORT_AW(DialogBoxIndirectParam);
    GUI_IMPORT___(DialogBoxIndirectParamAorW);

    GUI_IMPORT__W(LoadString);
    GUI_IMPORT___(SetForegroundWindow);
    GUI_IMPORT___(MonitorFromWindow);
    GUI_IMPORT_AW(DdeInitialize)

    GUI_IMPORT___(AttachThreadInput);

    ProcName = NULL;

import_fail:

    if (ProcName)
        SbieApi_Log(2303, L"%s (0)", ProcName);

    //
    // initialize sub-modules
    //

    ok = TRUE;

    if (ok)
        ok = Gui_InitClass();

    if (ok)
        ok = Gui_InitTitle();

    if (ok)
        ok = Gui_Init2();

    if (ok)
        ok = Gui_InitEnum();

    if (ok)
        ok = Gui_InitProp();

    if (ok)
        ok = Gui_InitMsg();

    if (ok)
        ok = Gui_InitWinHooks();

    if (ok)
        ok = Gui_InitDlgTmpl();

    if (ok)
        ok = Gui_Init3();

	// NoSbieDesk BEGIN
	if (SbieApi_QueryConfBool(NULL, L"NoSandboxieDesktop", FALSE))
		return ok;
	// NoSbieDesk END

    SBIEDLL_HOOK_GUI(AttachThreadInput);

    return ok;
}


//---------------------------------------------------------------------------
// Gui_Init2
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_Init2(void)
{
    SBIEDLL_HOOK_GUI(ExitWindowsEx);
    SBIEDLL_HOOK_GUI(EndTask);
    if (__sys_ConsoleControl) {
        SBIEDLL_HOOK_GUI(ConsoleControl);
    }

    //if (Gui_RenameClasses) {
    if (! Dll_SkipHook(L"createwin")) {

        SBIEDLL_HOOK_GUI(CreateWindowExA);
        SBIEDLL_HOOK_GUI(CreateWindowExW);
    }

    if (! Dll_KernelBase) {

        //
        // see Gui_InitWindows7
        //

        SBIEDLL_HOOK_GUI(DefWindowProcA);
        SBIEDLL_HOOK_GUI(DefWindowProcW);
    }


    SBIEDLL_HOOK_GUI(SetThreadDesktop);
    SBIEDLL_HOOK_GUI(SwitchDesktop);

    SBIEDLL_HOOK_GUI(MessageBoxW);
    SBIEDLL_HOOK_GUI(MessageBoxExW);

    if (! Gui_OpenAllWinClasses) {

        SBIEDLL_HOOK_GUI(UserHandleGrantAccess);
        SBIEDLL_HOOK_GUI(IsWindow);
        SBIEDLL_HOOK_GUI(IsWindowEnabled);
        SBIEDLL_HOOK_GUI(IsWindowVisible);
        SBIEDLL_HOOK_GUI(IsWindowUnicode);
        SBIEDLL_HOOK_GUI(IsIconic);
        SBIEDLL_HOOK_GUI(IsZoomed);
        SBIEDLL_HOOK_GUI(MoveWindow);
        SBIEDLL_HOOK_GUI(SetWindowPos);
        SBIEDLL_HOOK_GUI(MapWindowPoints);
        SBIEDLL_HOOK_GUI(ClientToScreen);
        SBIEDLL_HOOK_GUI(ScreenToClient);
        SBIEDLL_HOOK_GUI(GetClientRect);
        SBIEDLL_HOOK_GUI(GetWindowRect);
        SBIEDLL_HOOK_GUI(GetWindowInfo);
        SBIEDLL_HOOK_GUI(AnimateWindow);
        SBIEDLL_HOOK_GUI(WaitForInputIdle);
        SBIEDLL_HOOK_GUI(ActivateKeyboardLayout);
    }

    if (! Gui_InitMisc())
        return FALSE;

    if (! Gui_DDE_Init())
        return FALSE;

    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_Init3
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_Init3(void)
{
    //
    // expect that both RegisterDeviceNotificationA and
    // RegisterDeviceNotificationW might point at the same location
    //

    if (__sys_RegisterDeviceNotificationA ==
                                        __sys_RegisterDeviceNotificationW) {

        SBIEDLL_HOOK_GUI(RegisterDeviceNotificationW);

    } else {

        SBIEDLL_HOOK_GUI(RegisterDeviceNotificationA);
        SBIEDLL_HOOK_GUI(RegisterDeviceNotificationW);
    }

    SBIEDLL_HOOK_GUI(UnregisterDeviceNotification);

    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_InitWindows7
//---------------------------------------------------------------------------


_FX void Gui_InitWindows7(void)
{
    if (Dll_KernelBase) {

        //
        // on Windows 7, doing GetProcAddress for user32!DefWindowProcX
        // actually returns the address for ntdll!NtdllDefWindowProc_X,
        // which points at just one instruction:
        //      jmp dword/qword ptr [x]
        // where x is initially a dummy function in ntdll.  when the user32
        // dll is initialized, the value at x is redirected to the real
        // function in user32.
        //
        // Gui_Init2 is typically invoked before user32 was initialized,
        // therefore we postpone hooking DefWindowProc to this function,
        // which is called after user32 has been initialized, and can see
        // the real function in user32.
        //

        ULONG i;
        for (i = 0; i < 2; ++i) {

            BOOLEAN ok = FALSE;
            ULONG_PTR *pSourceFunc = 0;
            UCHAR *SourceFunc;
            UCHAR *DetourFunc;
            const UCHAR *FuncName;

            if (i == 0) {
                pSourceFunc = (ULONG_PTR *)&__sys_DefWindowProcA;
                DetourFunc = (UCHAR *)Gui_DefWindowProcA;
                FuncName = "DefWindowProcA";
            } else if (i == 1) {
                pSourceFunc = (ULONG_PTR *)&__sys_DefWindowProcW;
                DetourFunc = (UCHAR *)Gui_DefWindowProcW;
                FuncName = "DefWindowProcW";
            }

            if (! pSourceFunc)
                continue;
            SourceFunc = (UCHAR *)(*pSourceFunc);
            if (! SourceFunc)
                continue;

            //
            // confirm the function starts with an indirect jmp,
            // and try to replace the value at [x]
            //

#ifdef _WIN64

            if (*(UCHAR *)SourceFunc == 0x48 &&
                    *(USHORT *)((UCHAR *)SourceFunc + 1) == 0x25FF) {
                // 4825FF is same as 25FF
                SourceFunc = (UCHAR *)SourceFunc + 1;
            }

#endif _WIN64

            if (*(ULONG *)SourceFunc == 0x25FFFF8B) {
                // skip prefix "mov edi,edi"
                SourceFunc = (UCHAR *)SourceFunc + 2;
            }

            if (*(USHORT *)SourceFunc == 0x25FF) {

                ULONG_PTR target;
#ifdef _WIN64
                LONG_PTR diff = *(LONG *)((ULONG_PTR)SourceFunc + 2);
                target = (ULONG_PTR)SourceFunc + 6 + diff;
#else
                target = *(ULONG_PTR *)((ULONG_PTR)SourceFunc + 2);
#endif _WIN64
                *pSourceFunc = *(ULONG_PTR *)target;
            }

            *pSourceFunc = (ULONG_PTR)SbieDll_Hook(
                FuncName, (void *)(*pSourceFunc), DetourFunc);
        }
    }
}

// Processthreadsapi.h from Windows 8 See
// https://msdn.microsoft.com/en-us/library/windows/desktop/hh769085%28v=vs.85%29.aspx
// https://msdn.microsoft.com/en-us/library/windows/desktop/hh871472(v=vs.85).aspx
#if 0
typedef enum _PROCESS_MITIGATION_POLICY { 
  ProcessDEPPolicy                  = 0,
  ProcessASLRPolicy                 = 1,
  ProcessReserved1MitigationPolicy  = 2,
  ProcessStrictHandleCheckPolicy    = 3,
  ProcessSystemCallDisablePolicy    = 4,
  MaxProcessMitigationPolicy        = 5
} PROCESS_MITIGATION_POLICY, *PPROCESS_MITIGATION_POLICY;

typedef struct _PROCESS_MITIGATION_SYSTEM_CALL_DISABLE_POLICY {
  union {
      DWORD  Flags;
      struct {
          DWORD DisallowWin32kSystemCalls : 1;
          DWORD ReservedFlags : 31;
      };
  };
} PROCESS_MITIGATION_SYSTEM_CALL_DISABLE_POLICY, *PPROCESS_MITIGATION_SYSTEM_CALL_DISABLE_POLICY;
#endif

typedef BOOL (WINAPI * pGetProcessMitigationPolicy)(
    HANDLE hProcess,
    PROCESS_MITIGATION_POLICY MitigationPolicy,
    PVOID lpBuffer,
    SIZE_T dwLength
);

_FX BOOL DisallowWin32kSystemCallsIsOn()
{
    BOOL bRet = FALSE;

    if (Dll_OsBuild >= 8400)    // win8
    {
        pGetProcessMitigationPolicy fn = (pGetProcessMitigationPolicy) GetProcAddress(GetModuleHandle(L"kernel32.dll"), "GetProcessMitigationPolicy");
        
        if (fn)
        {
            PROCESS_MITIGATION_SYSTEM_CALL_DISABLE_POLICY pol = {0};

            if (fn(GetCurrentProcess(), ProcessSystemCallDisablePolicy, &pol, sizeof(pol)))
            {
                if (pol.Flags & pol.DisallowWin32kSystemCalls)
                {
                    bRet = TRUE;
                }
            }
        }
    }

    return bRet;
}

//---------------------------------------------------------------------------
// Gui_ConnectToWindowStationAndDesktop
//---------------------------------------------------------------------------

extern P_NtSetInformationThread __sys_NtSetInformationThread;

_FX BOOLEAN Gui_ConnectToWindowStationAndDesktop(HMODULE User32)
{
    static HDESK _ProcessDesktop = NULL;

    RTL_USER_PROCESS_PARAMETERS *ProcessParms;
    ULONG_PTR rc = 0;
    ULONG errlvl = 0;

    //
    // process is already connected to window station, connect to desktop
    //

    if (_ProcessDesktop)
        goto ConnectThread;

    //
    // on first call from Gdi_GdiDllInitialize_Common in gdi.c, the User32
    // parameter should be non-zero.  if we get here and User32 is zero,
    // then this is a call from DllMain for a non-GUI process, and we quit
    //

    if (! User32)
        return FALSE;

    if (DisallowWin32kSystemCallsIsOn())
    {
        return FALSE;
    }

    //
    // the first win32k service call (i.e. service number >= 0x1000)
    // triggers "thread GUI conversion".  the kernel system service
    // handler nt!KiSystemService calls nt!PsConvertToGuiThread which
    // calls some initialization function in win32k which connects the
    // calling process to a window station object, then connects the
    // calling thread to a desktop object.
    //
    // note that this thread conversion code path occurs for the first
    // win32k syscall without regard to which specific syscall it is.
    //
    // the difficulty with this is that SbieLow does not hook win32k
    // syscalls, so the window station/desktop connection is done with
    // the highly restricted process token, and this fails.  to solve
    // this problem, the SbieSvc GUI Proxy Server creates dummy window
    // station and desktop objects with a NULL DACL which can be accessed
    // even with our restricted token.  SbieSvc also gives us duplicated
    // handles with full access to the real window station and desktop
    // objects that we need to use.
    //
    // we place the name of the dummy window station object into the
    // lpDesktop member of the STARTUPINFO structure.  we then call
    // SetProcessWindowStation.  which means the thread conversion code
    // will successfully connect to the dummy window station, and then
    // control passes to SetProcessWindowStation which connects us to
    // the real window station before returning control.
    //

    ProcessParms = Proc_GetRtlUserProcessParameters();
    if (! ProcessParms)
        errlvl = 1;
    else {

        //
        // ask SbieSvc for window station and desktop handles
        //

        ULONG req = GUI_GET_WINDOW_STATION;
        GUI_GET_WINDOW_STATION_RPL *rpl = Gui_CallProxyEx(
                    &req, sizeof(ULONG), sizeof(*rpl), FALSE);

        if (! rpl)
            errlvl = 2;
        else {

            //
            // locate windowstation and desktop functions in user32 dll
            //

            P_SetProcessWindowStation _SetProcessWindowStation =
                (P_SetProcessWindowStation)
                    GetProcAddress(User32, "SetProcessWindowStation");

            if (! __sys_SetThreadDesktop) {
                // in the special case when USER32 is loaded before GDI32, as
                // discussed in Gdi_InitZero, SetThreadDesktop is still zero
                __sys_SetThreadDesktop = (P_SetThreadDesktop)
                    GetProcAddress(User32, "SetThreadDesktop");
            }

            if ((! _SetProcessWindowStation) || (! __sys_SetThreadDesktop))
                errlvl = 3;
            else {

                //
                // set DesktopName in ProcessParms to point to our dummy
                // window station so the initial default connection can
                // be made to a workstation that is accessible
                //

                UNICODE_STRING SaveDesktopName;
#ifndef _WIN64
                UNICODE_STRING64 SaveDesktopName64;
                UNICODE_STRING64 *DesktopName64;
#endif ! _WIN64

                memcpy(&SaveDesktopName, &ProcessParms->DesktopName,
                       sizeof(UNICODE_STRING));

                RtlInitUnicodeString(
                    &ProcessParms->DesktopName, rpl->name);

#ifndef _WIN64
                //
                // in a 32-bit process on 64-bit Windows, we actually need
                // to change the DesktopName member in the 64-bit
                // RTL_USER_PROCESS_PARAMETERS structure and not the
                // 32-bit version of the structure.
                //
                // note that the 64-bit PEB will be in the lower 32-bits in
                // a 32-bit process, so it is accessible, but its address is
                // not available to us.   but the SbieSvc GUI Proxy process
                // is 64-bit so it can send us the address of the 64-bit PEB
                // in the reply datagram
                //

                if (Dll_IsWow64) {

                    //
                    // 64-bit PEB offset 0x20 -> RTL_USER_PROCESS_PARAMETERS
                    // RTL_USER_PROCESS_PARAMETERS offset 0xC0 is DesktopName
                    //

                    ULONG ProcessParms64 = *(ULONG *)(rpl->peb64 + 0x20);
                    DesktopName64 =
                            (UNICODE_STRING64 *)(ProcessParms64 + 0xC0);

                    memcpy(&SaveDesktopName64,
                           DesktopName64, sizeof(UNICODE_STRING64));

                    DesktopName64->Length = ProcessParms->DesktopName.Length;
                    DesktopName64->MaximumLength =
                                     ProcessParms->DesktopName.MaximumLength;
                    DesktopName64->Buffer =
                                     (ULONG)ProcessParms->DesktopName.Buffer;
                }
#endif ! _WIN64

                //
                // note also that the default \Windows object directory
                // (where the WindowStations object directory is located)
                // grants access to Everyone, but this is not true for
                // the per-session object directories \Sessions\N.
                //
                // our process token does not include the change notify
                // privilege, so access to the window station object
                // would have to validate each object directory in the
                // path, and this would fail with our process token.
                //
                // to work around this, we issue a special request to
                // SbieDrv through NtSetInformationThread which causes
                // it to return with an impersonation token that includes
                // the change notify privilege but is otherwise restricted
                //
                // see also:  file core/drv/thread_token.c function
                // Thread_SetInformationThread_ChangeNotifyToken
                //

                rc = (ULONG_PTR)NtCurrentThread();

				// OriginalToken BEGIN
				if (SbieApi_QueryConfBool(NULL, L"OriginalToken", FALSE))
					rc = 0;
				else
				// OriginalToken END
                if (__sys_NtSetInformationThread)
                {
                    rc = __sys_NtSetInformationThread(NtCurrentThread(),
                        ThreadImpersonationToken, &rc, sizeof(rc));
                }
                else
                {
                    rc = NtSetInformationThread(NtCurrentThread(),
                                ThreadImpersonationToken, &rc, sizeof(rc));
                }

                if (rc != 0)
                    errlvl = 4;

                //
                // invoking SetProcessWindowStation will first connect
                // to the default (dummy) window station as part of
                // initial thread by PsConvertToGuiThread, then when
                // control finally arrives in SetProcessWindowStation,
                // the connection to the real window station is made
                //

                else if (! _SetProcessWindowStation(
                                                (HWINSTA)rpl->hwinsta)) {
                    errlvl = 5;
                    rc = GetLastError();

                } else
                    _ProcessDesktop = (HDESK)rpl->hdesk;

                //
                // restore the original contents of the DesktopName field
                //

                memcpy(&ProcessParms->DesktopName, &SaveDesktopName,
                       sizeof(UNICODE_STRING));
#ifndef _WIN64
                if (Dll_IsWow64) {
                    memcpy(DesktopName64, &SaveDesktopName64,
                           sizeof(UNICODE_STRING64));
                }
#endif ! _WIN64
            }

            Dll_Free(rpl);
        }
    }

    //
    // the first thread, as well as any subsequent new thread, has to
    // explicitly connect to a desktop object by handle, for the same
    // reason that we need explicit connection to window station by
    // handle:  because the process token is not useful to connect by
    // object name, which is what thread conversion logic tries to do
    //

ConnectThread:

    if (errlvl == 0) {

        if (! __sys_SetThreadDesktop(_ProcessDesktop)) {
            errlvl = 6;
            rc = GetLastError();
        }
    }

    if (errlvl) {
        WCHAR errtxt[48];
        Sbie_snwprintf(errtxt, 48, L"Win32Init.%d (%08p)", errlvl, (void*)rc);
        SbieApi_Log(2205, errtxt);
    }

    return (rc == 0) ? TRUE : FALSE;
}


//---------------------------------------------------------------------------
// Gui_SetThreadDesktop
//---------------------------------------------------------------------------


_FX BOOL Gui_SetThreadDesktop(HDESK hDesktop)
{
    SetLastError(ERROR_SUCCESS);
    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_SwitchDesktop
//---------------------------------------------------------------------------


_FX BOOL Gui_SwitchDesktop(HDESK hDesktop)
{
    SetLastError(ERROR_SUCCESS);
    return TRUE;
}

//---------------------------------------------------------------------------
// Gui_UserHandleGrantAccess
//---------------------------------------------------------------------------


_FX BOOL Gui_UserHandleGrantAccess(
    HANDLE hUserHandle, HANDLE hJob, BOOL bGrant)
{
    SetLastError(ERROR_SUCCESS);
    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_IsSameBox
//---------------------------------------------------------------------------
// only check sandboxed process

_FX BOOLEAN Gui_IsSameBox(
    HWND hwnd, ULONG_PTR *out_idProcess, ULONG_PTR *out_idThread)
{
    ULONG idProcess, idThread;
    NTSTATUS status;
    WCHAR boxname[48];
    ULONG session_id;

    idProcess = 0;
    idThread = __sys_GetWindowThreadProcessId(hwnd, &idProcess);
    if (out_idProcess)
        *out_idProcess = idProcess;
    if (out_idThread)
        *out_idThread = idThread;

    if ((! idThread) || (! idProcess)) {

#ifndef _WIN64

        if (((ULONG_PTR)hwnd & 0xFFFF0000) == 0
                && (! out_idProcess) && (! out_idThread)) {
            //
            // hWnd could be an index into some internal table (possibly
            // only under WOW64), in which case the GetWindowThreadProcessId
            // call fails.  if the caller doesn't care about pid and tid,
            // then we just assume the window is in the current process
            //
            return TRUE;
        }

#endif ! _WIN64

        return FALSE;
    }

    if (idProcess == Dll_ProcessId)
        return TRUE;

    // not a valid sandboxed process. host injected process has boxname.
    if ( (SbieApi_QueryProcessInfo((HANDLE)(ULONG_PTR)idProcess, 0) & SBIE_FLAG_VALID_PROCESS) != SBIE_FLAG_VALID_PROCESS )
        return FALSE;

    status = SbieApi_QueryProcess((HANDLE)(ULONG_PTR)idProcess,
                                  boxname, NULL, NULL, &session_id);
    if (! NT_SUCCESS(status))
        return FALSE;
    if (session_id != Dll_SessionId)
        return FALSE;
    if (_wcsicmp(boxname, Dll_BoxName) != 0)
        return FALSE;

    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_Free
//---------------------------------------------------------------------------


_FX void Gui_Free(void *ptr)
{
    DWORD err = GetLastError();
    Dll_Free(ptr);
    SetLastError(err);
}


//---------------------------------------------------------------------------
// Gui_CreateDummyParentWindow
//---------------------------------------------------------------------------


_FX HWND Gui_CreateDummyParentWindow(void)
{
    static HWND _hwnd = NULL;
    static ATOM _atom = 0;

    //
    // an Internet Explorer tab process creates a child window (WS_CHILD)
    // with the desktop as a parent.  (and then asks the main IE process
    // to reparent the window.)  the desktop window is inaccessile, so we
    // have to create a dummy window to serve as the parent window
    //

    if (_hwnd) {
        if (__sys_IsWindow(_hwnd))
            return _hwnd;
        _hwnd = NULL;
    }

    if (! _atom) {

        WCHAR clsnm[64], *boxed_clsnm;
        WNDCLASS wc;

        Sbie_snwprintf(clsnm, 64, L"%s-DUMMY-%d-%d",
                 SBIE, Dll_ProcessId, GetTickCount());
        boxed_clsnm = Gui_CreateClassNameW(clsnm);

        memzero(&wc, sizeof(wc));
        wc.lpfnWndProc = __sys_DefWindowProcW;
        wc.hInstance = Dll_Instance;
        wc.lpszClassName = boxed_clsnm;
        _atom = __sys_RegisterClassW(&wc);

        if (boxed_clsnm != clsnm)
            Gui_Free(boxed_clsnm);
    }

    if (! _atom)
        return NULL;

    _hwnd = __sys_CreateWindowExW(
                0, (void *)_atom, L"", 0,
                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                NULL, NULL, Dll_Instance, NULL);
    return _hwnd;
}


//---------------------------------------------------------------------------
// Gui_CreateWindowExW
//---------------------------------------------------------------------------


_FX HWND Gui_CreateWindowExW(
    DWORD dwExStyle,
    void *lpClassName,
    void *lpWindowName,
    DWORD dwStyle,
    int x,
    int y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);
    void *new_WindowName;
    WCHAR *clsnm;
    HWND hwndResult;

    //
    // under Sandboxie 4 the Chrome sandbox child process gets confused
    // (reason not known) and creates some top level windows, for which it
    // does not process messages.  this causes DDE message broadcast to
    // hang for several seconds.  to workaround this, we cause the windows
    // to be created as message-only windows
    //
    // note:  the desktop window was made accessible in early v4 builds
    // but this code is still here to handle any other parent windows
    //
    // note:  this code breaks chrome hw acceleration, so its no longer used
    //

    /*if (Dll_ChromeSandbox) { 
        dwStyle |= WS_CHILD;
        hWndParent = HWND_MESSAGE;
    }*/

    //
    // replace title on windows that have no parent
    // replace class name
    // replace parent
    //

    if ((! Gui_DisableTitle) &&
            lpWindowName && (dwStyle & WS_CAPTION) == WS_CAPTION &&
            ((! hWndParent) || (dwStyle & WS_CHILD) == 0))
        new_WindowName = Gui_CreateTitleW((WCHAR *)lpWindowName);
    else
        new_WindowName = lpWindowName;

    if (! Gui_RenameClasses)
        clsnm = lpClassName;
    else
        clsnm = Gui_CreateClassNameW(lpClassName);

    if (hWndParent && (hWndParent != HWND_MESSAGE)
                            && (! __sys_IsWindow(hWndParent))) {
        if (dwStyle & WS_CHILD)
            hWndParent = Gui_CreateDummyParentWindow();
        else
            hWndParent = NULL;
    }

    //
    // create window
    //

    ++TlsData->gui_create_window;
    if (TlsData->gui_create_window == 1) {

        if (!TlsData->gui_hooks_installed) {
            Gui_NotifyWinHooks();
            TlsData->gui_hooks_installed = TRUE;
        }

        Taskbar_SetProcessAppUserModelId();
    }

    hwndResult = __sys_CreateWindowExW(
        dwExStyle, clsnm, new_WindowName, dwStyle, x, y,
        nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

    if ((! hwndResult) && (clsnm != lpClassName)) {

        ULONG LastError = GetLastError();
        if (LastError == ERROR_CANNOT_FIND_WND_CLASS ||
            LastError == ERROR_INVALID_WINDOW_HANDLE) {

            hwndResult = __sys_CreateWindowExW(
                dwExStyle, lpClassName, new_WindowName, dwStyle, x, y,
                nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
        }
    }

    --TlsData->gui_create_window;

    //
    // replace window procedure
    //

    if (hwndResult) {

        Gui_SetWindowProc(hwndResult, FALSE);

        if ((! hWndParent) && (! (dwExStyle & WS_EX_TOOLWINDOW)))
            Taskbar_SetWindowAppUserModelId(hwndResult);
    }

    //
    // finish
    //

    if (clsnm != lpClassName)
        Gui_Free(clsnm);
    if (new_WindowName != lpWindowName)
        Gui_Free((void *)new_WindowName);

    return hwndResult;
}


//---------------------------------------------------------------------------
// Gui_CreateWindowExA
//---------------------------------------------------------------------------


_FX HWND Gui_CreateWindowExA(
    DWORD dwExStyle,
    UCHAR *lpClassName,
    void *lpWindowName,
    DWORD dwStyle,
    int x,
    int y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);
    void *new_WindowName;
    UCHAR *clsnm;
    HWND hwndResult;

    //
    // replace title on windows that have no parent
    // replace class name
    // replace parent
    //

    if ((! Gui_DisableTitle) &&
            lpWindowName && (dwStyle & WS_CAPTION) == WS_CAPTION &&
            ((! hWndParent) || (dwStyle & WS_CHILD) == 0))
        new_WindowName = Gui_CreateTitleA((UCHAR *)lpWindowName);
    else
        new_WindowName = lpWindowName;

    if (! Gui_RenameClasses)
        clsnm = lpClassName;
    else
        clsnm = Gui_CreateClassNameA(lpClassName);

    if (hWndParent && (hWndParent != HWND_MESSAGE)
                            && (! __sys_IsWindow(hWndParent))) {
        if (dwStyle & WS_CHILD)
            hWndParent = Gui_CreateDummyParentWindow();
        else
            hWndParent = NULL;
    }

    //
    // create window
    //

    ++TlsData->gui_create_window;
    if (TlsData->gui_create_window == 1) {
        
        if (!TlsData->gui_hooks_installed) {
            Gui_NotifyWinHooks();
            TlsData->gui_hooks_installed = TRUE;
        }

        Taskbar_SetProcessAppUserModelId();
    }

    hwndResult = __sys_CreateWindowExA(
        dwExStyle, clsnm, new_WindowName, dwStyle, x, y,
        nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

    if ((! hwndResult) && (clsnm != lpClassName)) {

        ULONG LastError = GetLastError();
        if (LastError == ERROR_CANNOT_FIND_WND_CLASS ||
            LastError == ERROR_INVALID_WINDOW_HANDLE) {

            hwndResult = __sys_CreateWindowExA(
                dwExStyle, lpClassName, new_WindowName, dwStyle, x, y,
                nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
        }
    }

    --TlsData->gui_create_window;

    //
    // replace window procedure
    //

    if (hwndResult) {

        Gui_SetWindowProc(hwndResult, FALSE);

        if ((! hWndParent) && (! (dwExStyle & WS_EX_TOOLWINDOW)))
            Taskbar_SetWindowAppUserModelId(hwndResult);
    }

    //
    // finish
    //

    if (clsnm != lpClassName)
        Gui_Free(clsnm);
    if (new_WindowName != lpWindowName)
        Gui_Free((void *)new_WindowName);

    return hwndResult;
}


//---------------------------------------------------------------------------
// Gui_CanForwardMsg
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_CanForwardMsg(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam/*, LRESULT* plResult*/)
{
    //*plResult = 0;
    if (uMsg == WM_DROPFILES) {

        if (Ole_DoDragDrop(hWnd, wParam, lParam))
            return FALSE;

    } else if (uMsg == WM_DEVICECHANGE) {

        SbieDll_DeviceChange(wParam, lParam);

    } else if (uMsg == WM_COPYDATA) {

        if (Gui_DDE_COPYDATA_Received(hWnd, wParam, lParam))
            return FALSE;

    }
    else if (uMsg == WM_DDE_INITIATE) {

    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_WindowProcW
//---------------------------------------------------------------------------


_FX LRESULT Gui_WindowProcW(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WNDPROC wndproc;
    LRESULT lResult;
    LPARAM new_lParam;
    THREAD_DATA * TlsData = Dll_GetTlsData(NULL);
    BOOLEAN bIgnore = FALSE;

    if (! Gui_CanForwardMsg(hWnd, uMsg, wParam, lParam/*, &lResult*/))
        return 0; //lResult;

    if (uMsg == WM_DDE_INITIATE)
        wParam = Gui_DDE_INITIATE_Received(hWnd, wParam);

    if (uMsg == WM_SETTEXT && Gui_ShouldCreateTitle(hWnd))
        new_lParam = (LPARAM)Gui_CreateTitleW((WCHAR *)lParam);
    else
        new_lParam = lParam;

    wndproc = __sys_GetPropW(hWnd, (LPCWSTR)Gui_WindowProcOldW_Atom);
    if (DLL_IMAGE_OFFICE_EXCEL == Dll_ImageType) {

        if (WM_RENDERFORMAT == uMsg)
        {
            TlsData = Dll_GetTlsData(NULL);

            // Ignore rendering when clipboard is closing.
            if (TlsData && TlsData->gui_should_suppress_msgbox)
            {
                bIgnore = TRUE;
            }
        }

        if (!bIgnore)
        {
            lResult = __sys_CallWindowProcW(wndproc, hWnd, uMsg, wParam, new_lParam);
        }
        else
        {
            lResult = 0;
        }
    }
    else {
        lResult = __sys_CallWindowProcW(wndproc, hWnd, uMsg, wParam, new_lParam);
    }

    if (new_lParam != lParam)
        Gui_Free((void *)new_lParam);
    return lResult;
}


//---------------------------------------------------------------------------
// Gui_WindowProcA
//---------------------------------------------------------------------------


_FX LRESULT Gui_WindowProcA(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WNDPROC wndproc;
    LRESULT lResult;
    LPARAM new_lParam;

    if (! Gui_CanForwardMsg(hWnd, uMsg, wParam, lParam/*, &lResult*/))
        return 0; //lResult;

    if (uMsg == WM_DDE_INITIATE)
        wParam = Gui_DDE_INITIATE_Received(hWnd, wParam);

    if (uMsg == WM_SETTEXT && Gui_ShouldCreateTitle(hWnd))
        new_lParam = (LPARAM)Gui_CreateTitleA((UCHAR *)lParam);
    else
        new_lParam = lParam;

    wndproc = __sys_GetPropW(hWnd, (LPCWSTR)Gui_WindowProcOldA_Atom);
    lResult = __sys_CallWindowProcA(wndproc, hWnd, uMsg, wParam, new_lParam);

    if (new_lParam != lParam)
        Gui_Free((void *)new_lParam);
    return lResult;
}


//---------------------------------------------------------------------------
// Gui_DefWindowProcW
//---------------------------------------------------------------------------


_FX LRESULT Gui_DefWindowProcW(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPARAM new_lParam = lParam;
    LRESULT lResult;

    if (uMsg == WM_SETTEXT && Gui_ShouldCreateTitle(hWnd))
        new_lParam = (LPARAM)Gui_CreateTitleW((WCHAR *)lParam);

    else if (uMsg == WM_CREATE || uMsg == WM_NCCREATE)
        Gui_CREATESTRUCT_Restore(lParam);

    lResult = __sys_DefWindowProcW(hWnd, uMsg, wParam, new_lParam);

    if (new_lParam != lParam)
        Gui_Free((void *)new_lParam);

    return lResult;
}


//---------------------------------------------------------------------------
// Gui_DefWindowProcA
//---------------------------------------------------------------------------


_FX LRESULT Gui_DefWindowProcA(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPARAM new_lParam = lParam;
    LRESULT lResult;

    if (uMsg == WM_SETTEXT && Gui_ShouldCreateTitle(hWnd))
        new_lParam = (LPARAM)Gui_CreateTitleA((UCHAR *)lParam);

    else if (uMsg == WM_CREATE || uMsg == WM_NCCREATE)
        Gui_CREATESTRUCT_Restore(lParam);

    lResult = __sys_DefWindowProcA(hWnd, uMsg, wParam, new_lParam);

    if (new_lParam != lParam)
        Gui_Free((void *)new_lParam);

    return lResult;
}


//---------------------------------------------------------------------------
// Gui_ActivateKeyboardLayout
//---------------------------------------------------------------------------


_FX ULONG_PTR Gui_ActivateKeyboardLayout(ULONG_PTR hkl, UINT flags)
{
    //
    // direct invocation ActivateKeyboardLayout is blocked because our
    // process is restricted, but we can simulate it by sending a message
    //

    return __sys_DefWindowProcW(__sys_GetFocus(), WM_INPUTLANGCHANGEREQUEST,
                                INPUTLANGCHANGE_SYSCHARSET, hkl);
}


//---------------------------------------------------------------------------
// Gui_ExitWindowsEx
//---------------------------------------------------------------------------


_FX BOOL Gui_ExitWindowsEx(UINT uFlags, DWORD dwReason)
{
    SbieApi_Log(2104, L"%S [%S]", Dll_ImageName, Dll_BoxName);
    SetLastError(ERROR_PRIVILEGE_NOT_HELD);
    return FALSE;
}


//---------------------------------------------------------------------------
// Gui_IsWindowCommon
//---------------------------------------------------------------------------


_FX BOOL Gui_IsWindowCommon(HWND hwnd, UCHAR which)
{
    GUI_IS_WINDOW_REQ req;
    GUI_IS_WINDOW_RPL *rpl;
    ULONG err;
    BOOL retval;

    //
    // first try to issue the command in the context of this process
    //

    err = GetLastError();

    if (__sys_IsWindow(hwnd)) {

        if (which == 'w')
            return TRUE;
        if (which == 'e')
            return __sys_IsWindowEnabled(hwnd);
        if (which == 'v')
            return __sys_IsWindowVisible(hwnd);
        if (which == 'u')
            return __sys_IsWindowUnicode(hwnd);
        if (which == 'i')
            return __sys_IsIconic(hwnd);
        if (which == 'z')
            return __sys_IsZoomed(hwnd);

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;

    } else if (! hwnd)
        return FALSE;

    //
    // if the specified hwnd isn't valid (which is probably due to
    // job restrictions), then go through SbieSvc
    //

    req.msgid = GUI_IS_WINDOW;
    req.error = err;
    req.hwnd = (ULONG)(ULONG_PTR)hwnd;

    rpl = Gui_CallProxy(&req, sizeof(req), sizeof(*rpl));
    if (! rpl)
        return FALSE;

    err = rpl->error;
    if (which == 'w')
        retval = rpl->window;
    else if (which == 'e')
        retval = rpl->enabled;
    else if (which == 'v')
        retval = rpl->visible;
    else if (which == 'u')
        retval = rpl->unicode;
    else if (which == 'i')
        retval = rpl->iconic;
    else if (which == 'z')
        retval = rpl->zoomed;
    else {
        retval = FALSE;
        err = ERROR_INVALID_PARAMETER;
    }

    Dll_Free(rpl);
    SetLastError(err);
    return retval;
}


//---------------------------------------------------------------------------
// Gui_IsWindow
//---------------------------------------------------------------------------


_FX BOOL Gui_IsWindow(HWND hWnd)
{
    return Gui_IsWindowCommon(hWnd, 'w');
}


//---------------------------------------------------------------------------
// Gui_IsWindowEnabled
//---------------------------------------------------------------------------


_FX BOOL Gui_IsWindowEnabled(HWND hWnd)
{
    return Gui_IsWindowCommon(hWnd, 'e');
}


//---------------------------------------------------------------------------
// Gui_IsWindowVisible
//---------------------------------------------------------------------------


_FX BOOL Gui_IsWindowVisible(HWND hWnd)
{
    return Gui_IsWindowCommon(hWnd, 'v');
}


//---------------------------------------------------------------------------
// Gui_IsWindowUnicode
//---------------------------------------------------------------------------


_FX BOOL Gui_IsWindowUnicode(HWND hWnd)
{
    return Gui_IsWindowCommon(hWnd, 'u');
}


//---------------------------------------------------------------------------
// Gui_IsIconic
//---------------------------------------------------------------------------


_FX BOOL Gui_IsIconic(HWND hWnd)
{
    return Gui_IsWindowCommon(hWnd, 'i');
}


//---------------------------------------------------------------------------
// Gui_IsZoomed
//---------------------------------------------------------------------------


_FX BOOL Gui_IsZoomed(HWND hWnd)
{
    return Gui_IsWindowCommon(hWnd, 'z');
}


//---------------------------------------------------------------------------
// Gui_MoveWindow
//---------------------------------------------------------------------------


_FX BOOL Gui_MoveWindow(
    HWND hWnd, int x, int y, int w, int h, BOOL bRepaint)
{
    if (! Gui_IsWindowAccessible(hWnd)) {
        SetLastError(ERROR_INVALID_WINDOW_HANDLE);
        return FALSE;
    }
    return __sys_MoveWindow(hWnd, x, y, w, h, bRepaint);
}


//---------------------------------------------------------------------------
// Gui_SetWindowPos
//---------------------------------------------------------------------------


_FX BOOL Gui_SetWindowPos(
    HWND hWnd, HWND hWndInsertAfter, int x, int y, int w, int h, UINT flags)
{
    ULONG error = GetLastError();

    if (! Gui_IsWindowAccessible(hWnd)) {
        SetLastError(ERROR_INVALID_WINDOW_HANDLE);
        return FALSE;
    }

    //
    // use SbieSvc GUI Proxy if hWnd is accessible but outside the sandbox
    //

    if (! Gui_IsSameBox(hWnd, NULL, NULL)) {

        GUI_SET_WINDOW_POS_REQ req;
        GUI_SET_WINDOW_POS_RPL *rpl;
        ULONG retval;

        req.msgid = GUI_SET_WINDOW_POS;
        req.error = error;
        req.hwnd = (ULONG)(ULONG_PTR)hWnd;
        req.hwnd_insert_after = (ULONG)(ULONG_PTR)hWndInsertAfter;
        req.x = x;
        req.y = y;
        req.w = w;
        req.h = h;
        req.flags = flags;

        rpl = Gui_CallProxyEx(&req, sizeof(req), sizeof(*rpl), TRUE);
        if (! rpl)
            return FALSE;

        retval = rpl->retval;
        error = rpl->error;
        Dll_Free(rpl);
        SetLastError(error);
        return retval;
    }

    //
    // for SWP_FRAMECHANGED request, refresh the window title
    //

    if ((! Gui_DisableTitle) && (flags & SWP_FRAMECHANGED)) {

        static volatile LONG SetWindowPosDepth = 0;
        if (InterlockedIncrement(&SetWindowPosDepth) == 1) {

            if (Gui_ShouldCreateTitle(hWnd)) {

                LRESULT lResult1;
                WCHAR *title = Dll_AllocTemp(512);
                if (__sys_GetWindowTextW(hWnd, title, 510)) {
                    __sys_SendMessageTimeoutW(
                        hWnd, WM_SETTEXT, 0, (LPARAM)title,
                        SMTO_BLOCK | SMTO_ABORTIFHUNG, 1000, &lResult1);
                }
                Dll_Free(title);
            }

            InterlockedDecrement(&SetWindowPosDepth);
        }
    }

    return __sys_SetWindowPos(hWnd, hWndInsertAfter, x, y, w, h, flags);
}

//---------------------------------------------------------------------------
// MsgBoxShouldContinue
//---------------------------------------------------------------------------

static BOOL MsgBoxShouldContinue(UINT uType)
{
    THREAD_DATA * TlsData = Dll_GetTlsData(NULL);
    // Condition is that we're performing an operation
    // (gui_should_suppress_msgbox is TRUE) and
    // the message box is informational (i.e. only has OK to click on).
    // In this case we simply ignore this and return OK.
    if (TlsData &&
        TlsData->gui_should_suppress_msgbox &&
        (uType & 0xF) == MB_OK)
    {
        return FALSE;
    }
    return TRUE;
}

//---------------------------------------------------------------------------
// Gui_MessageBoxW
//---------------------------------------------------------------------------

_FX int Gui_MessageBoxW(
    HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
    if (!MsgBoxShouldContinue(uType))
    {
        return IDOK;
    }
    return (int)__sys_MessageBoxW(hWnd, lpText, lpCaption, uType);
}

//---------------------------------------------------------------------------
// Gui_MessageBoxW
//---------------------------------------------------------------------------

_FX int Gui_MessageBoxExW(
    HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType, WORD wLanguageId)
{
    if (!MsgBoxShouldContinue(uType))
    {
        return IDOK;
    }

    // DarkComet is using a trick to get csrss.exe to display the MessageBox and bypass the Sbie window border.
    // Disable these here.
    uType &= ~(MB_DEFAULT_DESKTOP_ONLY | MB_SERVICE_NOTIFICATION);
    return (int)__sys_MessageBoxExW(hWnd, lpText, lpCaption, uType, wLanguageId);
}

//---------------------------------------------------------------------------
// Gui_EndTask
//---------------------------------------------------------------------------


_FX BOOL Gui_EndTask(HWND hWnd, BOOL fShutDown, BOOL fForce)
{
    LRESULT lResult1;
    LRESULT lResult2 = __sys_SendMessageTimeoutW(
        hWnd, WM_CLOSE, 0, 0,
        SMTO_BLOCK | SMTO_ABORTIFHUNG, 1000, &lResult1);
    SetLastError(ERROR_SUCCESS);
    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_ConsoleControl
//---------------------------------------------------------------------------


_FX BOOL Gui_ConsoleControl(ULONG ctlcode, ULONG *data, ULONG_PTR unknown)
{
    if (ctlcode == 7) {
        //
        // in Windows 8, conhost.exe uses ConsoleControl with
        // ctlcode 7 to terminate counterpart process.
        // data[0] specifies pid, data[1] specifies HWND
        //
        BOOLEAN ok = SbieDll_KillOne(*data);
        if (ok)
            return STATUS_SUCCESS;
        SbieApi_Log(2205, L"ConsoleControl");
    }
    return __sys_ConsoleControl(ctlcode, data, unknown);
}


//---------------------------------------------------------------------------
// Gui_RegisterDeviceNotificationA
//---------------------------------------------------------------------------


_FX ULONG_PTR Gui_RegisterDeviceNotificationA(
    HANDLE hRecipient, LPVOID NotificationFilter, DWORD Flags)
{
    SetLastError(0);
    return (ULONG_PTR)0x12345678;
}


//---------------------------------------------------------------------------
// Gui_RegisterDeviceNotificationW
//---------------------------------------------------------------------------


_FX ULONG_PTR Gui_RegisterDeviceNotificationW(
    HANDLE hRecipient, LPVOID NotificationFilter, DWORD Flags)
{
    SetLastError(0);
    return (ULONG_PTR)0x12345678;
}


//---------------------------------------------------------------------------
// Gui_UnregisterDeviceNotification
//---------------------------------------------------------------------------


_FX BOOL Gui_UnregisterDeviceNotification(ULONG_PTR Handle)
{
    SetLastError(0);
    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_AllowSetForegroundWindow
//---------------------------------------------------------------------------


_FX void Gui_AllowSetForegroundWindow(void)
{
    //
    // this function is typically used prior to calling SbieDll_CallServer
    // in a scenario where SbieSvc might display a pop up window and we
    // want to let this window go to the foreground
    //

    typedef (*P_AllowSetForegroundWindow)(ULONG dwProcessId);
    P_AllowSetForegroundWindow pAllowSetForegroundWindow =
        (P_AllowSetForegroundWindow)
            Ldr_GetProcAddrNew(DllName_user32, L"AllowSetForegroundWindow","AllowSetForegroundWindow");
    if (pAllowSetForegroundWindow)
        pAllowSetForegroundWindow(ASFW_ANY);
}


//---------------------------------------------------------------------------
// Gui_MapWindowPoints
//---------------------------------------------------------------------------


int Gui_MapWindowPoints(
    HWND hWndFrom, HWND hWndTo, LPPOINT lpPoints, UINT cPoints)
{
    GUI_MAP_WINDOW_POINTS_REQ *req;
    GUI_MAP_WINDOW_POINTS_RPL *rpl;
    ULONG req_len, err;
    ULONG retval;

    //
    // first try to execute the command in the context of this process
    //

    retval = __sys_MapWindowPoints(
                        hWndFrom, hWndTo, lpPoints, cPoints);
    if (retval)
        return retval;

    /*if (Gui_IsSameBox(hWndFrom, NULL, NULL)) {
        if (Gui_IsSameBox(hWndTo, NULL, NULL)) {
            return __sys_MapWindowPoints(
                        hWndFrom, hWndTo, lpPoints, cPoints);
        }
    }*/

    //
    // otherwise handle the request through SbieSvc GUI Proxy server
    //

    req_len = sizeof(GUI_MAP_WINDOW_POINTS_REQ) + cPoints * sizeof(POINT);
    req = Dll_AllocTemp(req_len);
    req->msgid = GUI_MAP_WINDOW_POINTS;
    req->hwnd_from = (ULONG)(ULONG_PTR)hWndFrom;
    req->hwnd_to = (ULONG)(ULONG_PTR)hWndTo;
    req->num_pts = cPoints;
    memcpy(req->pts, lpPoints, cPoints * sizeof(POINT));

    rpl = Gui_CallProxy(req, req_len, sizeof(*rpl));
    if (! rpl)
        return 0;

    err = rpl->error;
    retval = rpl->retval;
    memcpy(lpPoints, rpl->pts, (ULONG)rpl->num_pts * sizeof(POINT));
    Dll_Free(rpl);

    SetLastError(err);
    return retval;
}


//---------------------------------------------------------------------------
// Gui_ClientToScreen
//---------------------------------------------------------------------------


_FX BOOL Gui_ClientToScreen(HWND hWnd, LPPOINT lpPoint)
{
    GUI_MAP_WINDOW_POINTS_REQ req;
    GUI_MAP_WINDOW_POINTS_RPL *rpl;
    ULONG err;
    ULONG retval;

    //
    // first try to execute the command in the context of this process
    //

    retval = __sys_ClientToScreen(hWnd, lpPoint);
    if (retval)
        return retval;

    //
    // otherwise handle the request through SbieSvc GUI Proxy server
    //

    req.msgid = GUI_MAP_WINDOW_POINTS;
    req.hwnd_from = (ULONG)(ULONG_PTR)hWnd;
    req.num_pts = (0xFF000000 | 'c2s');
    memcpy(req.pts, lpPoint, sizeof(POINT));

    rpl = Gui_CallProxy(&req, sizeof(req), sizeof(*rpl));
    if (! rpl)
        return 0;

    err = rpl->error;
    retval = rpl->retval;
    memcpy(lpPoint, rpl->pts, sizeof(POINT));
    Dll_Free(rpl);

    SetLastError(err);
    return retval;
}


//---------------------------------------------------------------------------
// Gui_ScreenToClient
//---------------------------------------------------------------------------


_FX BOOL Gui_ScreenToClient(HWND hWnd, LPPOINT lpPoint)
{
    GUI_MAP_WINDOW_POINTS_REQ req;
    GUI_MAP_WINDOW_POINTS_RPL *rpl;
    ULONG err;
    ULONG retval;

    //
    // first try to execute the command in the context of this process
    //

    retval = __sys_ScreenToClient(hWnd, lpPoint);
    if (retval)
        return retval;

    //
    // otherwise handle the request through SbieSvc GUI Proxy server
    //

    req.msgid = GUI_MAP_WINDOW_POINTS;
    req.hwnd_to = (ULONG)(ULONG_PTR)hWnd;
    req.num_pts = (0xFF000000 | 's2c');
    memcpy(req.pts, lpPoint, sizeof(POINT));

    rpl = Gui_CallProxy(&req, sizeof(req), sizeof(*rpl));
    if (! rpl)
        return 0;

    err = rpl->error;
    retval = rpl->retval;
    memcpy(lpPoint, rpl->pts, sizeof(POINT));
    Dll_Free(rpl);

    SetLastError(err);
    return retval;
}


//---------------------------------------------------------------------------
// Gui_AnimateWindow
//---------------------------------------------------------------------------


_FX BOOL Gui_AnimateWindow(HWND hwnd, ULONG time, ULONG flags)
{
    ULONG nShowCmd;

    //
    // the AnimateWindow API manipulates both the window in question and
    // its parent window.  for windows like the ComboLBox which have the
    // desktop as their parent, the call will fail
    //
    //

    if (Gui_IsSameBox(hwnd, NULL, NULL)) {
        HWND parent = __sys_GetParent(hwnd);
        if (Gui_IsSameBox(parent, NULL, NULL)) {

            return __sys_AnimateWindow(hwnd, time, flags);
        }
    }

    //
    // we can't use the SbieSvc GUI Proxy to invoke the AnimateWindow API
    // because that API causes messages to be sent to the window proc,
    // and our thread would be stuck in Gui_CallProxy and would not process
    // any window messages.  so we just do a basic show/hide instead
    //

    if (flags & AW_HIDE)
        nShowCmd = SW_HIDE;
    else if (flags & AW_ACTIVATE)
        nShowCmd = SW_SHOW;
    else
        nShowCmd = SW_SHOWNOACTIVATE;
    return __sys_ShowWindow(hwnd, nShowCmd);
}


//---------------------------------------------------------------------------
// Gui_GetClientRect
//---------------------------------------------------------------------------


_FX BOOL Gui_GetClientRect(HWND hWnd, LPRECT lpRect)
{
    GUI_GET_WINDOW_RECT_REQ req;
    GUI_GET_WINDOW_RECT_RPL *rpl;
    ULONG err;
    BOOL ok;

    err = GetLastError();
    if (__sys_GetClientRect(hWnd, lpRect))
        return TRUE;

    req.msgid = GUI_GET_WINDOW_RECT;
    req.error = err;
    req.which = 'clnt';
    req.hwnd = (ULONG)(ULONG_PTR)hWnd;

    rpl = Gui_CallProxy(&req, sizeof(req), sizeof(*rpl));
    if (! rpl)
        return FALSE;

    err = rpl->error;
    ok = rpl->result;
    memcpy(lpRect, &rpl->rect, sizeof(RECT));
    Dll_Free(rpl);

    SetLastError(err);
    return ok;
}


//---------------------------------------------------------------------------
// Gui_GetWindowRect
//---------------------------------------------------------------------------


_FX BOOL Gui_GetWindowRect(HWND hWnd, LPRECT lpRect)
{
    GUI_GET_WINDOW_RECT_REQ req;
    GUI_GET_WINDOW_RECT_RPL *rpl;
    ULONG err;
    BOOL ok;

    err = GetLastError();
    if (__sys_GetWindowRect(hWnd, lpRect))
        return TRUE;

    req.msgid = GUI_GET_WINDOW_RECT;
    req.error = err;
    req.which = 'wind';
    req.hwnd = (ULONG)(ULONG_PTR)hWnd;

    rpl = Gui_CallProxy(&req, sizeof(req), sizeof(*rpl));
    if (! rpl)
        return FALSE;

    err = rpl->error;
    ok = rpl->result;
    memcpy(lpRect, &rpl->rect, sizeof(RECT));
    Dll_Free(rpl);

    SetLastError(err);
    return ok;
}


//---------------------------------------------------------------------------
// Gui_GetWindowInfo
//---------------------------------------------------------------------------


_FX BOOL Gui_GetWindowInfo(HWND hWnd, PWINDOWINFO pwi)
{
    GUI_GET_WINDOW_INFO_REQ req;
    GUI_GET_WINDOW_INFO_RPL *rpl;
    ULONG err;
    BOOL ok;

    err = GetLastError();
    if (__sys_GetWindowInfo(hWnd, pwi))
        return TRUE;

    req.msgid = GUI_GET_WINDOW_INFO;
    req.error = err;
    req.hwnd = (ULONG)(ULONG_PTR)hWnd;

    rpl = Gui_CallProxy(&req, sizeof(req), sizeof(*rpl));
    if (! rpl)
        return FALSE;

    err = rpl->error;
    ok = rpl->result;
    memcpy(pwi, &rpl->info, sizeof(WINDOWINFO));
    Dll_Free(rpl);

    SetLastError(err);
    return ok;
}


//---------------------------------------------------------------------------
// Gui_WaitForInputIdle
//---------------------------------------------------------------------------


_FX DWORD Gui_WaitForInputIdle(HANDLE hProcess, DWORD dwMilliseconds)
{
    //
    // WaitForInput waits until the specified process is waiting for input
    // messages with an empty message queue.  initialization of a process
    // in the sandbox can involve some uses of Gui_CallProxyEx(msgwait=TRUE).
    // this might cause WaitForInput calls to return prematurely, because
    // MsgWaitForMultipleObjects triggers the "input idle" state.  to work
    // around this, we check that the process is idle 50 times in a row
    //

    ULONG StartTicks = GetTickCount();
    ULONG IdleCount = 0;
    ULONG TicksPassed;
    ULONG RetVal;
    while (1) {
        RetVal = __sys_WaitForInputIdle(hProcess, dwMilliseconds);
        if ((RetVal != 0) || (IdleCount > 50))
            break;
        TicksPassed = GetTickCount() - StartTicks;
        if (TicksPassed >= dwMilliseconds)
            break;
        dwMilliseconds -= TicksPassed;
        ++IdleCount;
        Sleep(1);
    }
    return RetVal;
}

_FX BOOL Gui_AttachThreadInput(DWORD idAttach, DWORD idAttachTo, BOOL fAttach)
{
    BOOL bRet = FALSE;

    if (fAttach)
    {
        GUI_REMOVE_HOST_WINDOW_REQ req;
        GUI_REMOVE_HOST_WINDOW_RPL *rpl = NULL;

        req.msgid = GUI_REMOVE_HOST_WINDOW;
        req.threadid = idAttachTo;

        rpl = Gui_CallProxy(&req, sizeof(req), sizeof(*rpl));
        
        if (rpl)
            Dll_Free(rpl);
    }

    bRet = __sys_AttachThreadInput(idAttach, idAttachTo, fAttach);

    return bRet;
}


//---------------------------------------------------------------------------
// Gui_GetWindowFromProxy
//---------------------------------------------------------------------------


_FX HWND Gui_GetWindowFromProxy(ULONG which, HWND hwnd)
{
    GUI_GET_WINDOW_HANDLE_REQ req;
    GUI_GET_WINDOW_HANDLE_RPL *rpl;
    HWND hwnd_ret;
    ULONG err;

    //
    // first try to issue the command in the context of this process
    //

    err = GetLastError();
    SetLastError(0);
    if (which == 'prnt')
        hwnd_ret = __sys_GetParent(hwnd);
    else if (which < 0x10)
        hwnd_ret = __sys_GetWindow(hwnd, which);
    else
        hwnd_ret = NULL;

    //
    // if there was an error (probably ERROR_INVALID_WINDOW_HANDLE)
    // the request was probably denied by job restrictions, so go
    // through SbieSvc.  otherwise just return
    //

    if (GetLastError() == 0)
        return hwnd_ret;

    req.msgid = GUI_GET_WINDOW_HANDLE;
    req.error = err;
    req.which = which;
    req.hwnd = (ULONG)(ULONG_PTR)hwnd;

    rpl = Gui_CallProxy(&req, sizeof(req), sizeof(*rpl));
    if (! rpl)
        return NULL;

    err = rpl->error;
    hwnd_ret = (HWND)(ULONG_PTR)rpl->hwnd;
    Dll_Free(rpl);

    SetLastError(err);
    return hwnd_ret;
}


//---------------------------------------------------------------------------
// Gui_CallProxy
//---------------------------------------------------------------------------


_FX void *Gui_CallProxy(void *req, ULONG req_len, ULONG rpl_min_len)
{
    return Gui_CallProxyEx(req, req_len, rpl_min_len, FALSE);
}


//---------------------------------------------------------------------------
// Gui_CallProxyEx
//---------------------------------------------------------------------------


_FX void *Gui_CallProxyEx(
    void *req, ULONG req_len, ULONG rpl_min_len, BOOLEAN msgwait)
{
    //static ULONG _Ticks = 0;
    //static ULONG _Ticks1 = 0;
    static WCHAR *_QueueName = NULL;
    NTSTATUS status;
    ULONG req_id;
    ULONG data_len;
    void *data;
    HANDLE event;

    //ULONG Ticks0 = GetTickCount();

    if (! _QueueName) {
        _QueueName = Dll_Alloc(32 * sizeof(WCHAR));
        Sbie_snwprintf(_QueueName, 32, L"*GUIPROXY_%08X", Dll_SessionId);
        //_Ticks = 0;
    }

    /*if (1) {
        WCHAR txt[128];
        Sbie_snwprintf(txt, 128, L"Request command is %08X\n", *(ULONG *)req);
        OutputDebugString(txt);
    }*/

    //
    // if this is a forced program, it may have been started as part of
    // a DDE conversation, which means Windows Explorer may now be waiting
    // in WaitForInputIdle and if we call MsgWaitForMultipleObjects then
    // Windows Explorer will send the DDE message too early for us.
    //
    // to avoid this, we don't use MsgWaitForMultipleObjects in forced
    // processes until DispatchMessage has been called, which is usually
    // late enough that the process already created its DDE server window
    //

    if ((Dll_ProcessFlags & SBIE_FLAG_FORCED_PROCESS) && msgwait) {
        extern BOOLEAN Gui_DispatchMessageCalled; // guimsg.c
        if (! Gui_DispatchMessageCalled)
            msgwait = FALSE;
    }

    // Opera's file dialog hook is there.
    if (msgwait && __sys_GetOpenFileNameW)
    {
        THREAD_DATA *data = Dll_GetTlsData(NULL);
        
        if (data->bOperaFileDlgThread)
        {
            msgwait = FALSE;
        }
    }

    status = SbieDll_QueuePutReq(_QueueName, req, req_len, &req_id, &event);
    if (NT_SUCCESS(status)) {

        if (msgwait) {

            //
            // wait for a reply while processing window messages sent to
            // this thread, for example if waiting for a response for a
            // GUI_GET_CLIPBOARD_DATA request, and the calling thread owns
            // the clipboard, then we have to respond to window messages
            // sent due to SbieSvc GUI Proxy calling GetClipboardData
            //

            ULONG StartTime = GetTickCount();

            if (__sys_MsgWaitForMultipleObjects && __sys_PeekMessageW) {

                while (1) {

                    if ((GetTickCount() - StartTime) > (10 * 1000))
                        status = WAIT_TIMEOUT;
                    else {
#ifdef _WIN64
                        status = __sys_MsgWaitForMultipleObjects(
                            1, &event, FALSE, (10 * 1000), QS_SENDMESSAGE);
#else ! _WIN64
                        // Gui_MsgWaitForMultipleObjects aligns stack
                        // before calling __sys_MsgWaitForMultipleObjects
                        // in case the caller gave us a bad stack
                        extern Gui_MsgWaitForMultipleObjects(
                            ULONG a, HANDLE *b, ULONG c, ULONG d, ULONG e);
                        status = Gui_MsgWaitForMultipleObjects(
                            1, &event, FALSE, (10 * 1000), QS_SENDMESSAGE);
#endif _WIN64
                    }

                    if (status != 1) {  // status 1 means message queue
                        if (status == -1)
                            status = 0xFF000000 | GetLastError();
                        break;
                    } else if (WaitForSingleObject(event, 0) == 0) {
                        // MsgWaitForMultipleObjects checks the message
                        // queue before checking our event, but if it
                        // was signalled, we prefer to just return
                        status = 0;
                        break;
                    } else {
                        // call PeekMessage to process sent messages
                        MSG msg;
                        __sys_PeekMessageW(&msg, NULL, 0, 0, PM_NOREMOVE);
                    }
                }

            } else
                status = STATUS_ENTRYPOINT_NOT_FOUND;

        } else {

            //
            // wait for a reply on the queue without processing window
            // messages, this is the simpler case and is preferable in most
            // scenarios where a window message is not expected
            //

            if (WaitForSingleObject(event, 10 * 1000) != 0)
                status = STATUS_TIMEOUT;
        }

        CloseHandle(event);
    }

    if (status == 0) {

        status = SbieDll_QueueGetRpl(_QueueName, req_id, &data, &data_len);

        if (NT_SUCCESS(status)) {

            if (data_len >= sizeof(ULONG) && *(ULONG *)data) {

                status = *(ULONG *)data;

            } else if (data_len >= rpl_min_len) {

                /*_Ticks += GetTickCount() - Ticks0;
                if (_Ticks > _Ticks1 + 1000) {
                    WCHAR txt[128];
                    Sbie_snwprintf(txt, 128, L"Already spent %d ticks in gui\n", _Ticks);
                    OutputDebugString(txt);
                    _Ticks1 = _Ticks;
                }*/

                return data;

            } else
                status = STATUS_INFO_LENGTH_MISMATCH;

            Dll_Free(data);
        }
    }

    SbieApi_Log(2203, L"%S; MsgId: %d - %S [%08X]", _QueueName, *(ULONG*)req, Dll_ImageName, status);
    SetLastError(ERROR_SERVER_DISABLED);
    return NULL;
}

_FX NTSTATUS ComDlg32_GetOpenFileNameW(LPVOID lpofn)
{
    BOOL bRet = FALSE;
    THREAD_DATA *data = Dll_GetTlsData(NULL);

    if (data)
        data->bOperaFileDlgThread = TRUE;
    
    bRet = __sys_GetOpenFileNameW(lpofn);

    if (data)
        data->bOperaFileDlgThread = FALSE;

    return bRet;
}

_FX BOOLEAN ComDlg32_Init(HMODULE hModule)
{
    //if (_wcsicmp(Dll_ImageName, L"opera.exe") == 0)
    //{
        void *GetOpenFileNameW = GetProcAddress(hModule, "GetOpenFileNameW");
        SBIEDLL_HOOK(ComDlg32_, GetOpenFileNameW);
    //}

    return TRUE;
}