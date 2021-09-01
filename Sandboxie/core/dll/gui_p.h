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
// GUI Services Private Include
//---------------------------------------------------------------------------


#ifndef MY_GUI_P_H
#define MY_GUI_P_H


#include <windows.h>
#include "common/win32_ntddk.h"
#include "dll.h"


//---------------------------------------------------------------------------
// Debug Options
//---------------------------------------------------------------------------


#undef  DEBUG_CREATECLASSNAME
#undef  DEBUG_REGISTERCLASS
#undef  DEBUG_FINDWINDOW


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define XFF4                    ((LONG_PTR)(0x0000FFFFL))
#define XFF8                    ((LONG_PTR)(0xFFFFFFFFL))

#define WM_DDE_FIRST        0x03E0
#define WM_DDE_INITIATE     (WM_DDE_FIRST+0)
#define WM_DDE_TERMINATE    (WM_DDE_FIRST+1)
#define WM_DDE_ACK          (WM_DDE_FIRST+4)
#define WM_DDE_DATA         (WM_DDE_FIRST+5)
#define WM_DDE_REQUEST      (WM_DDE_FIRST+6)
#define WM_DDE_EXECUTE      (WM_DDE_FIRST+8)
#define WM_DDE_LAST         (WM_DDE_FIRST+8)


//---------------------------------------------------------------------------
// Prototypes
//---------------------------------------------------------------------------


typedef DWORD (*P_GetWindowThreadProcessId)(
    HWND hWnd, LPDWORD lpdwProcessId);

typedef HWND (*P_GetWindow)(HWND hWnd, UINT uCmd);

typedef HWND (*P_GetParent)(HWND hWnd);

typedef HWND (*P_SetParent)(HWND hWndChild, HWND hWndNewParent);

typedef HWND (*P_GetFocus)(void);

typedef BOOL (*P_IsWindow)(HWND hWnd);

typedef BOOL (*P_IsWindowUnicode)(HWND hWnd);

typedef BOOL (*P_IsWindowEnabled)(HWND hWnd);

typedef BOOL (*P_IsWindowVisible)(HWND hWnd);

typedef BOOL (*P_IsIconic)(HWND hWnd);

typedef BOOL (*P_IsZoomed)(HWND hWnd);

typedef BOOL (*P_ClipCursor)(const RECT *lpRect);

typedef BOOL (*P_GetClipCursor)(RECT *lpRect);

typedef BOOL (*P_GetCursorPos)(LPPOINT lpPoint);

typedef BOOL (*P_SetCursorPos)(int x, int y);

typedef BOOL (*P_OpenClipboard)(HWND hwnd);

typedef BOOL (*P_CloseClipboard)(void);

typedef HWND (*P_GetClipboardOwner)(void);

typedef HWND (*P_GetOpenClipboardWindow)(void);

typedef DWORD (*P_GetClipboardSequenceNumber)(void);

typedef HANDLE (*P_GetClipboardData)(UINT uFormat);

typedef int (*P_GetClipboardFormatName)(
    UINT format, void *lpszFormatName, int cchMaxCount);

typedef UINT (*P_RegisterClipboardFormat)(
    const void *lpszFormatName);

typedef LRESULT (*P_CallWindowProc)(
    WNDPROC lpWndProc, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef LONG(*P_GetRawInputDeviceInfoA)(
    _In_opt_ HANDLE hDevice, _In_ UINT uiCommand,
    _Inout_updates_bytes_to_opt_(*pcbSize, *pcbSize) LPVOID pData,
    _Inout_ PUINT pcbSize);

typedef LONG(*P_GetRawInputDeviceInfoW)(
    _In_opt_ HANDLE hDevice, _In_ UINT uiCommand,
    _Inout_updates_bytes_to_opt_(*pcbSize, *pcbSize) LPVOID pData,
    _Inout_ PUINT pcbSize);

//---------------------------------------------------------------------------

typedef BOOL (*P_ExitWindowsEx)(UINT uFlags, DWORD dwReason);

typedef BOOL (*P_EndTask)(
    HWND hWnd, BOOL fShutDown, BOOL fForce);

typedef BOOL (*P_ConsoleControl)(
    ULONG ctlcode, ULONG *data, ULONG_PTR unknown);

typedef BOOL (*P_AllocConsole)(void);

//---------------------------------------------------------------------------

typedef HWND (*P_CreateWindowEx)(
    DWORD dwExStyle, void *lpClassName, void *lpWindowName,
    DWORD dwStyle, int x, int y, int nWidth, int nHeight,
    HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);

typedef BOOL (*P_ShowWindow)(HWND hWnd, int nCmdShow);

typedef BOOL (*P_AnimateWindow)(HWND hwnd, ULONG time, ULONG flags);

//---------------------------------------------------------------------------

typedef BOOL (*P_MoveWindow)(
    HWND hWnd, int x, int y, int w, int h, BOOL bRepaint);

typedef BOOL (*P_SetWindowPos)(
    HWND hWnd, HWND hWndInsertAfter, int x, int y, int w, int h, UINT flags);

typedef BOOL (*P_MapWindowPoints)(
    HWND hWndFrom, HWND hWndTo, LPPOINT lpPoints, UINT cPoints);

typedef BOOL (*P_ClientToScreen)(HWND hWnd, LPPOINT lpPoint);

typedef BOOL (*P_ScreenToClient)(HWND hWnd, LPPOINT lpPoint);

typedef BOOL (*P_GetClientRect)(HWND hWnd, LPRECT lpRect);

typedef BOOL (*P_GetWindowRect)(HWND hWnd, LPRECT lpRect);

typedef BOOL (*P_GetWindowInfo)(HWND hWnd, PWINDOWINFO pwi);

//---------------------------------------------------------------------------

typedef LRESULT (*P_DefWindowProc)(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef LRESULT (*P_DefFrameProc)(
    HWND hWnd, HWND hWndMDIClient, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef LRESULT (*P_DefDlgProc)(
    HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

//---------------------------------------------------------------------------

#define P_InternalGetWindowText P_GetWindowText

#define P_RealGetWindowClass P_GetClassName

typedef int (*P_GetWindowText)(
    HWND hWnd, void *lpWindowTitle, int nMaxCount);

typedef BOOL (*P_SetWindowText)(HWND hWnd, void *lpWindowTitle);

#define P_GetConsoleOriginalTitle P_GetConsoleTitle

typedef int (*P_GetConsoleTitle)(
    void *lpConsoleTitle, int nMaxCount);

typedef BOOL (*P_SetConsoleTitle)(
    void *lpConsoleTitle);

//---------------------------------------------------------------------------

typedef ATOM (*P_RegisterClass)(void *lpwcx);

#define P_RegisterClassEx P_RegisterClass

typedef BOOL (*P_UnregisterClass)(
    const void *lpClassName, HINSTANCE hInstance);

typedef BOOL (*P_GetClassInfo)(
    HINSTANCE hInstance, const void *lpClassName, void *lpwcx);

#define P_GetClassInfoEx P_GetClassInfo

typedef int (*P_GetClassName)(HWND hWnd, void *lpClassName, int nMaxCount);

//---------------------------------------------------------------------------

typedef HANDLE (*P_CreateWindowStation) (void * lpwinsta, DWORD dwFlags, ACCESS_MASK dwDesiredAccess, LPSECURITY_ATTRIBUTES lpsa);

typedef BOOL (*P_EnumWindows)(WNDENUMPROC lpEnumFunc, LPARAM lParam);

typedef BOOL (*P_EnumChildWindows)(
    HWND hWndParent, WNDENUMPROC lpEnumFunc, LPARAM lParam);

typedef BOOL (*P_EnumThreadWindows)(
    DWORD dwThreadId, WNDENUMPROC lpEnumFunc, LPARAM lParam);

typedef BOOL (*P_EnumDesktopWindows)(
    HDESK hDesktop, WNDENUMPROC lpEnumFunc, LPARAM lParam);

typedef BOOL (*P_EnumDesktops)(
    HWINSTA hwinsta, DESKTOPENUMPROC lpEnumFunc, LPARAM lParam);

typedef HDESK (*P_OpenDesktop)(
    void *lpszDesktop, ULONG dwFlags, BOOL fInherit,
    ACCESS_MASK dwDesiredAccess);

typedef HDESK (*P_CreateDesktop)(
    void *lpszDesktop, void *lpszDevice, void *DevMode, ULONG dwFlags,
    ACCESS_MASK dwDesiredAccess, void *SecurityAttributes);

//---------------------------------------------------------------------------

typedef HWND (*P_FindWindow)(
    const void *lpClassName, const void *lpWindowName);

typedef HWND (*P_FindWindowEx)(
    HWND hwndParent, HWND hwndChildAfter,
    const void *lpClassName, const void *lpWindowName);

//---------------------------------------------------------------------------

typedef HWND (*P_GetDesktopWindow)(void);

typedef HWND (*P_GetShellWindow)(void);

//---------------------------------------------------------------------------

typedef HANDLE (*P_GetProp)(
    HWND hWnd, const void *lpString);

typedef BOOL (*P_SetProp)(
    HWND hWnd, const void *lpString, HANDLE hData);

typedef HANDLE (*P_RemoveProp)(
    HWND hWnd, const void *lpString);

typedef ULONG (*P_GetWindowLong)(HWND hWnd, int nIndex);

typedef ULONG (*P_SetWindowLong)(HWND hWnd, int nIndex, ULONG dwNew);

typedef ULONG_PTR (*P_SetWindowLong8)(
    HWND hWnd, int nIndex, ULONG_PTR dwNew, ULONG IsAscii);

typedef ULONG_PTR (*P_GetWindowLongPtr)(HWND hWnd, int nIndex);

typedef ULONG_PTR (*P_SetWindowLongPtr)(
    HWND hWnd, int nIndex, ULONG_PTR dwNew);

typedef ULONG_PTR (*P_SetWindowLongPtr8)(
    HWND hWnd, int nIndex, ULONG_PTR dwNew, ULONG IsAscii);

typedef ULONG (*P_GetClassLong)(HWND hWnd, int nIndex);

typedef ULONG_PTR (*P_GetClassLongPtr)(HWND hWnd, int nIndex);

//---------------------------------------------------------------------------

typedef LRESULT (*P_SendMessage)(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef LRESULT (*P_SendMessageTimeout)(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT fuFlags, UINT uTimeout, PDWORD_PTR lpdwResult);

typedef BOOL (*P_SendMessageCallback)(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    SENDASYNCPROC lpCallBack, ULONG_PTR dwData);

typedef LRESULT (*P_SendNotifyMessage)(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef LRESULT (*P_PostMessage)(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef BOOL (*P_PostThreadMessage)(
    ULONG idThread, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef LRESULT (*P_DispatchMessage)(const MSG *lpmsg);

typedef LRESULT (*P_DispatchMessage8)(const MSG *lpmsg, ULONG IsAscii);

typedef LRESULT (*P_PeekMessage)(
    LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax,
    UINT wRemoveMsg);

typedef LRESULT (*P_GetMessage)(
    LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);

typedef LRESULT (*P_MessageBoxW)(
    HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);

typedef LRESULT (*P_MessageBoxExW)(
    HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType, WORD wLanguageId);

typedef DWORD (*P_MsgWaitForMultipleObjects)(
    DWORD nCount, const HANDLE* pHandles,
    BOOL bWaitAll, DWORD dwMilliseconds, DWORD dwWakeMask);

typedef DWORD (*P_WaitForInputIdle)(HANDLE hProcess, DWORD dwMilliseconds);

typedef BOOL (*P_AttachThreadInput)(DWORD idAttach, DWORD idAttachTo, BOOL fAttach);

typedef ULONG_PTR (*P_ActivateKeyboardLayout)(ULONG_PTR hkl, UINT flags);

//---------------------------------------------------------------------------

typedef HHOOK (*P_SetWindowsHookEx)(
    int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId);

typedef BOOL (*P_UnhookWindowsHookEx)(HHOOK hhk);

//---------------------------------------------------------------------------

typedef HWND (*P_CreateDialogParam)(
    HINSTANCE hInstance, void *lpTemplateName, HWND hWndParent,
    DLGPROC lpDialogFunc, LPARAM dwInitParam);

typedef HWND (*P_CreateDialogIndirectParam)(
    HINSTANCE hInstance, void *lpTemplate, HWND hWndParent,
    DLGPROC lpDialogFunc, LPARAM dwInitParam);

typedef HWND (*P_CreateDialogIndirectParamAorW)(
    HINSTANCE hInstance, void *lpTemplate, HWND hWndParent,
    DLGPROC lpDialogFunc, LPARAM dwInitParam, UINT fAnsiFlag);

typedef INT_PTR (*P_DialogBoxParam)(
    HINSTANCE hInstance, void *lpTemplateName, HWND hWndParent,
    DLGPROC lpDialogFunc, LPARAM dwInitParam);

typedef INT_PTR (*P_DialogBoxIndirectParam)(
    HINSTANCE hInstance, void *lpTemplate, HWND hWndParent,
    DLGPROC lpDialogFunc, LPARAM dwInitParam);

typedef INT_PTR (*P_DialogBoxIndirectParamAorW)(
    HINSTANCE hInstance, void *lpTemplate, HWND hWndParent,
    DLGPROC lpDialogFunc, LPARAM dwInitParam, UINT fAnsiFlag);

//---------------------------------------------------------------------------

typedef ULONG_PTR (*P_RegisterDeviceNotification)(
    HANDLE hRecipient, LPVOID NotificationFilter, DWORD Flags);

typedef BOOL (*P_UnregisterDeviceNotification)(ULONG_PTR Handle);

//---------------------------------------------------------------------------

typedef BOOL (*P_SystemParametersInfo)(
    UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni);

//---------------------------------------------------------------------------

typedef BOOL (*P_SwapMouseButton)(BOOL fSwap);

typedef BOOL (*P_SetDoubleClickTime)(UINT uInterval);

typedef HCURSOR (*P_SetCursor)(HCURSOR hCursor);

typedef BOOL (*P_GetIconInfo)(HICON hIcon, PICONINFO piconinfo);

typedef HWND (*P_GetForegroundWindow)(void);

typedef BOOL (*P_SetForegroundWindow)(HWND hWnd);

//---------------------------------------------------------------------------

typedef BOOL (*P_BlockInput)(BOOL fBlockIt);

typedef UINT (*P_SendInput)(ULONG nInputs, LPINPUT pInputs, ULONG cbInput);

//---------------------------------------------------------------------------

typedef int (*P_LoadString)(
    HINSTANCE hInstance, UINT uID, void *lpBuffer, int nBufferMax);

//---------------------------------------------------------------------------

typedef BOOL (*P_SetProcessWindowStation)(HWINSTA hWinSta);

typedef BOOL (*P_SetThreadDesktop)(HDESK hDesktop);

typedef BOOL (*P_SwitchDesktop)(HDESK hDesktop);

typedef BOOL (*P_UserHandleGrantAccess)(
    HANDLE hUserHandle, HANDLE hJob, BOOL bGrant);

//---------------------------------------------------------------------------

typedef HMONITOR (*P_MonitorFromWindow)(HWND hWnd, DWORD dwFlags);

typedef UINT (*P_DdeInitialize)(
    ULONG_PTR pidInst, ULONG_PTR pfnCallback, ULONG afCmd, ULONG ulRes);

typedef LONG (*P_ChangeDisplaySettingsEx)(
    void *lpszDeviceName, void *lpDevMode, HWND hwnd,
    DWORD dwflags, void *lParam);
//---------------------------------------------------------------------------
//For d3d11 hook if needed

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

//---------------------------------------------------------------------------
// For Opeara's file dialog hook if needed
typedef BOOL(*P_GetOpenFileNameW)(LPVOID lpofn);

//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


extern BOOLEAN Gui_RenameClasses;
extern BOOLEAN Gui_OpenAllWinClasses;   // not running in a restricted job

extern BOOLEAN Gui_DisableTitle;
extern ULONG Gui_BoxNameTitleLen;
extern WCHAR* Gui_BoxNameTitleW;

extern ATOM Gui_DropTargetProp_Atom;
extern ATOM Gui_WindowProcOldW_Atom;
extern ATOM Gui_WindowProcOldA_Atom;


//---------------------------------------------------------------------------
// Undo Unicode defines from windows.h
//---------------------------------------------------------------------------


#undef CreateWindowStation
#undef CallWindowProc
#undef CreateDesktop
#undef CreateDialogIndirectParam
#undef CreateDialogParam
#undef CreateWindowEx
#undef DdeInitialize
#undef DefWindowProc
#undef DialogBoxIndirectParam
#undef DialogBoxParam
#undef DispatchMessage
#undef EnumDesktops
#undef GetClassInfo
#undef GetClassInfoEx
#undef GetClassLong
#undef GetClassLongPtr
#undef GetClassName
#undef GetClipboardFormatName
#undef GetProp
#undef GetWindowLong
#undef GetWindowLongPtr
#undef GetWindowText
#undef FindWindow
#undef FindWindowEx
#undef LoadString
#undef OpenDesktop
#undef PeekMessage
#undef MessageBox
#undef PostMessage
#undef PostThreadMessage
#undef RealGetWindowClass
#undef RegisterClass
#undef RegisterClassEx
#undef RegisterClipboardFormat
#undef RegisterDeviceNotification
#undef RemoveProp
#undef SendMessage
#undef SendMessageCallback
#undef SendMessageTimeout
#undef SendNotifyMessage
#undef SetProp
#undef SetWindowLong
#undef SetWindowLongPtr
#undef SetWindowsHookEx
#undef UnregisterClass


//---------------------------------------------------------------------------
// Function Pointers in USER32.DLL
//---------------------------------------------------------------------------


#ifdef GUI_DEFINE_SYS_VARIABLES
#define GUI_SYS_VAR(sysname)        P_##sysname __sys_##sysname = 0;
#define GUI_SYS_VAR_AW(sysname,aw)  P_##sysname __sys_##sysname##aw = 0;
#else
#define GUI_SYS_VAR(sysname)        extern P_##sysname __sys_##sysname;
#define GUI_SYS_VAR_AW(sysname,aw)  extern P_##sysname __sys_##sysname##aw;
#endif
#define GUI_SYS_VAR_2(nm)       GUI_SYS_VAR_AW(nm,A); GUI_SYS_VAR_AW(nm,W);


GUI_SYS_VAR(ClipCursor)
GUI_SYS_VAR(GetClipCursor)
GUI_SYS_VAR(GetCursorPos)
GUI_SYS_VAR(SetCursorPos)

GUI_SYS_VAR(SetCursor)
GUI_SYS_VAR(GetIconInfo)

GUI_SYS_VAR(GetWindow)
GUI_SYS_VAR(GetParent)
GUI_SYS_VAR(SetParent)

GUI_SYS_VAR(GetWindowThreadProcessId)

GUI_SYS_VAR(IsWindow)
GUI_SYS_VAR(IsWindowUnicode)
GUI_SYS_VAR(IsWindowEnabled)
GUI_SYS_VAR(IsWindowVisible)
GUI_SYS_VAR(IsIconic)
GUI_SYS_VAR(IsZoomed)

GUI_SYS_VAR_2(SendMessage)
GUI_SYS_VAR_2(SendMessageTimeout)
//GUI_SYS_VAR_2(SendMessageCallback)
GUI_SYS_VAR_2(SendNotifyMessage)
GUI_SYS_VAR_2(PostMessage)
GUI_SYS_VAR_2(PostThreadMessage)
GUI_SYS_VAR_2(DispatchMessage)

GUI_SYS_VAR(MapWindowPoints)
GUI_SYS_VAR(ClientToScreen)
GUI_SYS_VAR(ScreenToClient)
GUI_SYS_VAR(GetClientRect)
GUI_SYS_VAR(GetWindowRect)

GUI_SYS_VAR(GetForegroundWindow)
GUI_SYS_VAR(SetForegroundWindow)

GUI_SYS_VAR(MonitorFromWindow)
GUI_SYS_VAR_2(DdeInitialize)

GUI_SYS_VAR(BlockInput)
GUI_SYS_VAR(SendInput)

GUI_SYS_VAR(OpenClipboard)
GUI_SYS_VAR(CloseClipboard)
GUI_SYS_VAR(GetClipboardData);
GUI_SYS_VAR(GetClipboardOwner);
GUI_SYS_VAR(GetOpenClipboardWindow);
GUI_SYS_VAR(GetClipboardSequenceNumber);

GUI_SYS_VAR(GetRawInputDeviceInfoA);
GUI_SYS_VAR(GetRawInputDeviceInfoW);

GUI_SYS_VAR_2(OpenDesktop)
GUI_SYS_VAR_2(CreateDesktop)
GUI_SYS_VAR_2(CreateWindowStation)
#undef GUI_SYS_VAR_2
#undef GUI_SYS_VAR


//---------------------------------------------------------------------------


extern  P_ExitWindowsEx             __sys_ExitWindowsEx;

extern  P_EndTask                   __sys_EndTask;

extern  P_ConsoleControl            __sys_ConsoleControl;

extern  P_SwapMouseButton           __sys_SwapMouseButton;

extern  P_SetDoubleClickTime        __sys_SetDoubleClickTime;

extern  P_GetClipboardFormatName    __sys_GetClipboardFormatNameA;
extern  P_GetClipboardFormatName    __sys_GetClipboardFormatNameW;

extern  P_RegisterClipboardFormat   __sys_RegisterClipboardFormatA;
extern  P_RegisterClipboardFormat   __sys_RegisterClipboardFormatW;

extern  P_RealGetWindowClass        __sys_RealGetWindowClassA;
extern  P_RealGetWindowClass        __sys_RealGetWindowClassW;

extern  P_GetWindowText             __sys_GetWindowTextA;
extern  P_GetWindowText             __sys_GetWindowTextW;

extern  P_CallWindowProc            __sys_CallWindowProcA;
extern  P_CallWindowProc            __sys_CallWindowProcW;

extern  P_CreateWindowEx            __sys_CreateWindowExA;
extern  P_CreateWindowEx            __sys_CreateWindowExW;

extern  P_DefWindowProc             __sys_DefWindowProcA;
extern  P_DefWindowProc             __sys_DefWindowProcW;

extern  P_RegisterClass             __sys_RegisterClassA;
extern  P_RegisterClass             __sys_RegisterClassW;
extern  P_RegisterClass             __sys_RegisterClassExA;
extern  P_RegisterClass             __sys_RegisterClassExW;

extern  P_UnregisterClass           __sys_UnregisterClassA;
extern  P_UnregisterClass           __sys_UnregisterClassW;

extern  P_GetClassInfo              __sys_GetClassInfoA;
extern  P_GetClassInfo              __sys_GetClassInfoW;
extern  P_GetClassInfo              __sys_GetClassInfoExA;
extern  P_GetClassInfo              __sys_GetClassInfoExW;

extern  P_GetClassName              __sys_GetClassNameA;
extern  P_GetClassName              __sys_GetClassNameW;

extern  P_EnumWindows               __sys_EnumWindows;
extern  P_EnumChildWindows          __sys_EnumChildWindows;
extern  P_EnumThreadWindows         __sys_EnumThreadWindows;
extern  P_EnumDesktopWindows        __sys_EnumDesktopWindows;

extern  P_EnumDesktops              __sys_EnumDesktopsA;
extern  P_EnumDesktops              __sys_EnumDesktopsW;

extern  P_FindWindow                __sys_FindWindowA;
extern  P_FindWindow                __sys_FindWindowW;
extern  P_FindWindowEx              __sys_FindWindowExA;
extern  P_FindWindowEx              __sys_FindWindowExW;

extern  P_GetDesktopWindow          __sys_GetDesktopWindow;
extern  P_GetShellWindow            __sys_GetShellWindow;

extern  P_GetProp                   __sys_GetPropA;
extern  P_GetProp                   __sys_GetPropW;

extern  P_SetProp                   __sys_SetPropA;
extern  P_SetProp                   __sys_SetPropW;

extern  P_RemoveProp                __sys_RemovePropA;
extern  P_RemoveProp                __sys_RemovePropW;

extern  P_GetWindowLong             __sys_GetWindowLongA;
extern  P_GetWindowLong             __sys_GetWindowLongW;

extern  P_SetWindowLong             __sys_SetWindowLongA;
extern  P_SetWindowLong             __sys_SetWindowLongW;

extern  P_GetClassLong              __sys_GetClassLongA;
extern  P_GetClassLong              __sys_GetClassLongW;

#ifdef _WIN64

extern  P_GetWindowLongPtr          __sys_GetWindowLongPtrA;
extern  P_GetWindowLongPtr          __sys_GetWindowLongPtrW;

extern  P_SetWindowLongPtr          __sys_SetWindowLongPtrA;
extern  P_SetWindowLongPtr          __sys_SetWindowLongPtrW;

extern  P_GetClassLongPtr           __sys_GetClassLongPtrA;
extern  P_GetClassLongPtr           __sys_GetClassLongPtrW;

#else ! _WIN64

#define __sys_GetWindowLongPtrA     __sys_GetWindowLongA
#define __sys_GetWindowLongPtrW     __sys_GetWindowLongW

#define __sys_SetWindowLongPtrA     __sys_SetWindowLongA
#define __sys_SetWindowLongPtrW     __sys_SetWindowLongW

#endif _WIN64

extern  P_SetWindowsHookEx          __sys_SetWindowsHookExA;
extern  P_SetWindowsHookEx          __sys_SetWindowsHookExW;

extern  P_UnhookWindowsHookEx       __sys_UnhookWindowsHookEx;

extern  P_CreateDialogParam         __sys_CreateDialogParamA;
extern  P_CreateDialogParam         __sys_CreateDialogParamW;

extern  P_CreateDialogIndirectParam __sys_CreateDialogIndirectParamA;
extern  P_CreateDialogIndirectParam __sys_CreateDialogIndirectParamW;

extern  P_CreateDialogIndirectParamAorW
                                    __sys_CreateDialogIndirectParamAorW;

extern  P_DialogBoxParam            __sys_DialogBoxParamA;
extern  P_DialogBoxParam            __sys_DialogBoxParamW;

extern  P_DialogBoxIndirectParam    __sys_DialogBoxIndirectParamA;
extern  P_DialogBoxIndirectParam    __sys_DialogBoxIndirectParamW;

extern  P_DialogBoxIndirectParamAorW
                                    __sys_DialogBoxIndirectParamAorW;

extern  P_MsgWaitForMultipleObjects __sys_MsgWaitForMultipleObjects;

extern  P_PeekMessage               __sys_PeekMessageW;
extern  P_PeekMessage               __sys_PeekMessageA;

extern  P_MessageBoxW               __sys_MessageBoxW;
extern  P_MessageBoxExW             __sys_MessageBoxExW;

extern  P_LoadString                __sys_LoadStringW;

//extern         P_D3D11CreateDevice __sys_D3D11CreateDevice;
//extern         P_D3D11CreateDevice    D3D11CreateDevice;

//---------------------------------------------------------------------------
// SBIEDLL_HOOK_GUI
//---------------------------------------------------------------------------


#define SBIEDLL_HOOK_GUI(proc)                              \
    *(ULONG_PTR *)&__sys_##proc = (ULONG_PTR)               \
        SbieDll_Hook(#proc, __sys_##proc, Gui_##proc);      \
    if (! __sys_##proc) return FALSE;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


BOOLEAN Gui_IsSameBox(
    HWND hwnd, ULONG_PTR *out_idProcess, ULONG_PTR *out_idThread);

void Gui_Free(void *ptr);

LRESULT Gui_WindowProcW(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

LRESULT Gui_WindowProcA(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


//---------------------------------------------------------------------------


BOOLEAN Gui_InitClass(void);

void Gui_Hook_CREATESTRUCT_Handler(void);

UCHAR *Gui_CreateClassNameA(const UCHAR *istr);

WCHAR *Gui_CreateClassNameW(const WCHAR *istr);

BOOLEAN Gui_IsOpenClass(const WCHAR *ClassName);

const WCHAR *Gui_UnCreateClassName(const WCHAR *clsnm);

BOOLEAN Gui_IsWindowAccessible(HWND hWnd);

void Gui_CREATESTRUCT_Restore(LPARAM lParam);


//---------------------------------------------------------------------------


BOOLEAN Gui_InitTitle(void);

BOOLEAN Gui_ShouldCreateTitle(HWND hWnd);

UCHAR *Gui_CreateTitleA(const UCHAR *oldTitle);

WCHAR *Gui_CreateTitleW(const WCHAR *oldTitle);

int Gui_FixTitleW(HWND hWnd, WCHAR *lpWindowTitle, int len);

int Gui_FixTitleA(HWND hWnd, UCHAR *lpWindowTitle, int len);


//---------------------------------------------------------------------------


BOOLEAN Gui_InitEnum(void);


//---------------------------------------------------------------------------


BOOLEAN Gui_InitProp(void);

void Gui_SetWindowProc(HWND hWnd, BOOLEAN force);


//---------------------------------------------------------------------------


BOOLEAN Gui_InitMsg(void);


//---------------------------------------------------------------------------


BOOLEAN Gui_InitWinHooks(void);

LRESULT Gui_RegisterWinHook(DWORD dwThreadId, ULONG64 ghk);

LRESULT Gui_NotifyWinHooks(void);


//---------------------------------------------------------------------------


BOOLEAN Gui_InitDlgTmpl(void);


//---------------------------------------------------------------------------


BOOLEAN Ole_DoDragDrop(HWND hWnd, WPARAM wParam, LPARAM lParam);


//---------------------------------------------------------------------------


BOOLEAN Gui_InitMisc(void);


//---------------------------------------------------------------------------


HWND Gui_GetWindowFromProxy(ULONG which, HWND hwnd);

void *Gui_CallProxy(void *req, ULONG req_len, ULONG rpl_min_len);

void *Gui_CallProxyEx(
    void *req, ULONG req_len, ULONG rpl_min_len, BOOLEAN msgwait);


//---------------------------------------------------------------------------


BOOLEAN Gui_DDE_Init(void);

WPARAM Gui_DDE_INITIATE_Received(HWND hWnd, WPARAM wParam);

HWND Gui_DDE_ACK_Sending(HWND hWnd, WPARAM wParam);

BOOLEAN Gui_DDE_COPYDATA_Received(HWND hWnd, WPARAM wParam, LPARAM lParam);

BOOLEAN Gui_DDE_Post_In_Box(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

LRESULT Gui_DDE_DATA_Posting(HWND hWnd, WPARAM wParam, LPARAM lParam);

/*
static HRESULT Gui_D3D11CreateDevice(
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
//---------------------------------------------------------------------------


#endif // MY_GUI_P_H
