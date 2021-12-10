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
#include "taskbar.h"
#include "core/svc/GuiWire.h"
#include "common/my_version.h"
#include <stdio.h>


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOL Gui_ConnectConsole(ULONG ShowFlag);

static BOOL Gui_AllocConsole(void);

static void Gui_ConsoleApc(ULONG_PTR Parameter);

static void Gui_WaitForParentExitApc(ULONG_PTR Parameter);

static ULONG Gui_ConsoleThread(void *xHandles);

static BOOL Gui_SetConsoleTitleA(const UCHAR *lpConsoleTitle);

static BOOL Gui_SetConsoleTitleW(const WCHAR *lpConsoleTitle);

static int Gui_GetConsoleTitleW(WCHAR *lpConsoleTitle, int nMaxCount);

static int Gui_GetConsoleTitleA(UCHAR *lpConsoleTitle, int nMaxCount);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static HWND Gui_ConsoleHwnd = NULL;


//---------------------------------------------------------------------------


static P_SetConsoleTitle            __sys_SetConsoleTitleA          = NULL;
static P_SetConsoleTitle            __sys_SetConsoleTitleW          = NULL;
static P_GetConsoleTitle            __sys_GetConsoleTitleA          = NULL;
static P_GetConsoleTitle            __sys_GetConsoleTitleW          = NULL;

static P_GetMessage                 __sys_GetMessageW               = NULL;


//---------------------------------------------------------------------------
// Gui_InitConsole1
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_InitConsole1(void)
{
    // NoSbieCons BEGIN
    if ((Dll_ProcessFlags & SBIE_FLAG_APP_COMPARTMENT) != 0 || SbieApi_QueryConfBool(NULL, L"NoSandboxieConsole", FALSE)) {

        //
        // We need to set Gui_ConsoleHwnd in order for Gui_InitConsole2 to start up properly,
        // this functions starts a thread which listens for WM_DEVICECHANGE which we need
        // we could go for a different signaling method in future but for now we stick to this methos
        //

        Gui_ConsoleHwnd = GetConsoleWindow();

        return TRUE;
    }
	// NoSbieCons END

    //
    // on Windows 7 we may need to connect this process to a console
    // instance (conhost.exe) outside the sandbox
    //

    if (Dll_ProcessFlags & (   SBIE_FLAG_CREATE_CONSOLE_SHOW
                             | SBIE_FLAG_CREATE_CONSOLE_HIDE)) {

        Gui_ConnectConsole(-1);
    }

    //
    // if this is a console application, hook console functions
    //
    // note that this function may also be called for a non-console
    // application which has used Gui_SetWindowsHookEx before creating
    // any windows (which would be needed to apply the hooks)
    //

    Gui_ConsoleHwnd = GetConsoleWindow();
    if (! Gui_ConsoleHwnd) {

        P_AllocConsole __sys_AllocConsole;
        char *_AllocConsole = "AllocConsole";

        if (Dll_KernelBase) {
            __sys_AllocConsole = (P_AllocConsole)
                            GetProcAddress(Dll_KernelBase, _AllocConsole);
        } else
            __sys_AllocConsole = NULL;

        if (! __sys_AllocConsole) {
            __sys_AllocConsole = (P_AllocConsole)
                            GetProcAddress(Dll_Kernel32, _AllocConsole);
        }

        SBIEDLL_HOOK(Gui_,AllocConsole);

        return TRUE;
    }

    //
    // hook title APIs
    //

    SBIEDLL_HOOK(Gui_,SetConsoleTitleA);
    SBIEDLL_HOOK(Gui_,SetConsoleTitleW);

    SBIEDLL_HOOK(Gui_,GetConsoleTitleA);
    SBIEDLL_HOOK(Gui_,GetConsoleTitleW);

    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_ConnectConsole
//---------------------------------------------------------------------------


_FX BOOL Gui_ConnectConsole(ULONG ShowFlag)
{
    HANDLE ProcessToken;
    NTSTATUS status;

    //
    // on Windows 7, a console process tries to launch conhost.exe through
    // csrss.exe during initialization of kernel32.dll in the function
    // kernel32!ConnectConsoleInternal.  this will fail because the process
    // is using a highly restricted primary token.
    //
    // to work around this, we have function Process_Low_InitConsole in
    // core/drv/process_low.c clear the console handle in the
    // RTL_USER_PROCESS_PARAMETERS structure, and prevent console creation
    // during kernel32.dll initialization, and mark the process with the
    // flag SBIE_FLAG_CREATE_CONSOLE_SHOW or SBIE_FLAG_CREATE_CONSOLE_HIDE.
    //
    // control then reaches here, where we ask the SbieSvc GUI Proxy to
    // create a console for us, by starting a console helper process, and
    // then we connect to that console using the console helper process id.
    //

    status = NtOpenProcessToken(NtCurrentProcess(),
                    TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY,
                                &ProcessToken);
    if (NT_SUCCESS(status)) {

        STARTUPINFO si;
        GUI_CREATE_CONSOLE_REQ req;
        GUI_CREATE_CONSOLE_RPL *rpl;

        GetStartupInfo(&si);

        req.msgid = GUI_CREATE_CONSOLE;
        req.token = (ULONG64)(ULONG_PTR)ProcessToken;

        if (ShowFlag != -1)
            req.show_window = (USHORT)ShowFlag;
        else if (Dll_ProcessFlags & SBIE_FLAG_CREATE_CONSOLE_HIDE)
            req.show_window = SW_HIDE;
        else if (si.dwFlags & STARTF_USESHOWWINDOW)
            req.show_window = si.wShowWindow;
        else
            req.show_window = SW_SHOWDEFAULT;

        rpl = Gui_CallProxy(
                    &req, sizeof(req), sizeof(GUI_CREATE_CONSOLE_RPL));
        if (! rpl)
            status = STATUS_SERVER_DISABLED;
        else {
            status = rpl->status;
            if (NT_SUCCESS(status)) {

                //
                // SbieSvc console helper process was successfully
                // created, we can now connect to that console
                //

                typedef BOOL (*P_AttachConsole)(ULONG ProcessId);
                P_AttachConsole AttachConsole = (P_AttachConsole)
                    GetProcAddress(Dll_Kernel32, "AttachConsole");

                if (! AttachConsole(rpl->process_id))
                    status = STATUS_NOT_SAME_DEVICE;
            }

            Dll_Free(rpl);
        }

        NtClose(ProcessToken);
    }

    //
    // finish
    //

    if (! NT_SUCCESS(status)) {
        WCHAR errtxt[48];
        Sbie_snwprintf(errtxt, 48, L"ConsoleInit (%08X)", status);
        SbieApi_Log(2205, errtxt);
        return FALSE;
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_AllocConsole
//---------------------------------------------------------------------------


_FX BOOL Gui_AllocConsole(void)
{
    return Gui_ConnectConsole(SW_SHOW);
}


//---------------------------------------------------------------------------
// Gui_InitConsole2
//---------------------------------------------------------------------------


_FX void Gui_InitConsole2(void)
{
    HANDLE *Handles;
    HMODULE User32;

    //
    // hack:  the Kaspersky process klwtblfs.exe is protected from
    // termination through TerminateProcess, so make sure we terminate
    // voluntarily when the parent ends (typically SandboxieDcomLaunch)
    //

    if (_wcsicmp(Dll_ImageName, L"klwtblfs.exe") == 0) {

		HANDLE ThreadHandle = CreateThread(NULL, 0, Proc_WaitForParentExit, (void *)1, 0, NULL);
		if (ThreadHandle)
			CloseHandle(ThreadHandle); 
    }

    //
    // start an auxiliary thread to listen for WM_DEVICECHANGE
    // and window hook notifications
    //

    if (! Gui_ConsoleHwnd)
        return;

    User32 = GetModuleHandle(DllName_user32);

    Handles = Dll_Alloc(3 * sizeof(HANDLE));

    Handles[0] = OpenThread(SYNCHRONIZE, FALSE, GetCurrentThreadId());
    if (Handles[0]) {

        Handles[1] = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (Handles[1]) {

            Handles[2] = CreateThread(
                            NULL, 0, Gui_ConsoleThread, Handles, 0, NULL);
            if (Handles[2]) {

                WaitForMultipleObjects(3, Handles, FALSE, INFINITE);
                CloseHandle(Handles[2]);
            }

            CloseHandle(Handles[1]);
        }
    }

    //
    // if user32 was not loaded in this thread 0 before creating thread 1
    // (Gui_ConsoleThread), it means Gui_ConnectToWindowStationAndDesktop
    // was not yet called in thread 0.  thread 1 loaded user32, and we
    // want to make sure that thread 0 also connects to the desktop
    //

    if ((! User32) && GetModuleHandle(DllName_user32))
        Gui_ConnectToWindowStationAndDesktop(NULL);
}


//---------------------------------------------------------------------------
// Gui_ConsoleThread
//---------------------------------------------------------------------------


_FX ULONG Gui_ConsoleThread(void *xHandles)
{
    HANDLE *Handles = (HANDLE *)xHandles;
    WCHAR *title;
    WNDCLASS wc;
    ATOM atom;
    HWND hwnd;
    MSG msg;
    ULONG rc;

    //
    //
    //

    __sys_GetMessageW = (P_GetMessage)
        Ldr_GetProcAddrNew(DllName_user32, L"GetMessageW","GetMessageW");

    if (    (! __sys_MsgWaitForMultipleObjects)
        ||  (! __sys_PeekMessageW)
        ||  (! __sys_GetMessageW)
        ||  (! __sys_DispatchMessageW)
        ||  (! __sys_RegisterClassW)
        ||  (! __sys_CreateWindowExW)
        ||  (! __sys_DefWindowProcW)) {

        return 0;
    }

    //
    // reset window title
    //

    title = Dll_AllocTemp(512);
    if (GetConsoleTitle(title, 510))
        SetConsoleTitle(title);
    Dll_Free(title);

    //
    // create a window to receive WM_DEVICECHANGE notifications
    //

    memzero(&wc, sizeof(WNDCLASS));
    wc.lpfnWndProc = __sys_DefWindowProcW;
    wc.hInstance = Dll_Instance;
    wc.lpszClassName = SANDBOXIE L"HelperWindowClass";
    atom = __sys_RegisterClassW(&wc);

    hwnd = __sys_CreateWindowExW(0, (void *)atom, L"", WS_OVERLAPPEDWINDOW,
                                 1, 1, 1, 1, NULL, NULL, NULL, NULL);
    if (! hwnd)
        return 0;

    Gui_SetWindowProc(hwnd, TRUE);

    //
    // wait for messages and thread termination
    //

    SetEvent(Handles[1]);

    while (1) {

        //
        // this causes git.exe to hang also jumplists for a console process are pointless anyways
        // 
        //if (Gui_ConsoleHwnd && Dll_InitComplete) {
        //
        //    Taskbar_SetWindowAppUserModelId(Gui_ConsoleHwnd);
        //    Gui_ConsoleHwnd = NULL;
        //}

        while (__sys_PeekMessageW(&msg, NULL, 0, 0, PM_NOREMOVE)) {

            if (__sys_GetMessageW(&msg, NULL, 0, 0) != -1)
                __sys_DispatchMessageW(&msg);
        }

        rc = __sys_MsgWaitForMultipleObjects(
                    1, Handles, FALSE, INFINITE, QS_ALLINPUT);
        if (rc == WAIT_OBJECT_0)    // main thread terminated
            break;
    }

    return 0;
}


//---------------------------------------------------------------------------
// Gui_SetConsoleTitleW
//---------------------------------------------------------------------------


_FX BOOL Gui_SetConsoleTitleW(const WCHAR *lpConsoleTitle)
{
    WCHAR *newTitle = Gui_CreateTitleW(lpConsoleTitle);
    BOOL b = __sys_SetConsoleTitleW(newTitle);
    if (newTitle != lpConsoleTitle)
        Gui_Free(newTitle);
    return b;
}


//---------------------------------------------------------------------------
// Gui_SetConsoleTitleA
//---------------------------------------------------------------------------


_FX BOOL Gui_SetConsoleTitleA(const UCHAR *lpConsoleTitle)
{
    UCHAR *newTitle = Gui_CreateTitleA(lpConsoleTitle);
    BOOL b = __sys_SetConsoleTitleA(newTitle);
    if (newTitle != lpConsoleTitle)
        Gui_Free(newTitle);
    return b;
}


//---------------------------------------------------------------------------
// Gui_GetConsoleTitleW
//---------------------------------------------------------------------------


_FX int Gui_GetConsoleTitleW(WCHAR *lpConsoleTitle, int nMaxCount)
{
    int rc = __sys_GetConsoleTitleW(lpConsoleTitle, nMaxCount);
    return Gui_FixTitleW((HWND)(ULONG_PTR)tzuk, lpConsoleTitle, min(rc, nMaxCount));
}


//---------------------------------------------------------------------------
// Gui_GetConsoleTitleA
//---------------------------------------------------------------------------


_FX int Gui_GetConsoleTitleA(UCHAR *lpConsoleTitle, int nMaxCount)
{
    int rc = __sys_GetConsoleTitleA(lpConsoleTitle, nMaxCount);
    return Gui_FixTitleA((HWND)(ULONG_PTR)tzuk, lpConsoleTitle, min(rc, nMaxCount));
}
