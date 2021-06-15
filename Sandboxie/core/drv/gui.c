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


#include "gui.h"
#include "conf.h"
#include "process.h"
#include "log.h"
#include "api.h"
#include "util.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static NTSTATUS Gui_Api_Init(PROCESS *proc, ULONG64 *parms);

static NTSTATUS Gui_Api_Clipboard(PROCESS *proc, ULONG64 *parms);


//---------------------------------------------------------------------------


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, Gui_Init)
#endif // ALLOC_PRAGMA


//---------------------------------------------------------------------------
// Include code for 32-bit Windows XP
//---------------------------------------------------------------------------


#ifndef _WIN64
#include "gui_xp.c"
#endif _WIN64


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static const WCHAR *Gui_OpenClass_Name = L"OpenWinClass";


//---------------------------------------------------------------------------
// Gui_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_Init(void)
{
#ifndef _WIN64

    if (Driver_OsVersion < DRIVER_WINDOWS_VISTA) {

        //
        // on Windows Vista, UIPI reasonably protects use of
        // PostThreadMessage, SendInput, and SetWindowsHookEx because
        // the primary access token of the process will have an
        // untrusted integrity level.
        //
        // but note that UIPI is turned off when UAC is turned off !
        //
        // on Windows XP, we have to hook these functions in win32k
        // to provide adequate protection.
        //

        if (! Gui_Init_XpHook())
            return FALSE;
    }

#endif ! _WIN64

    Api_SetFunction(API_INIT_GUI,       Gui_Api_Init);
    Api_SetFunction(API_GUI_CLIPBOARD,  Gui_Api_Clipboard);

    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_Api_Init
//---------------------------------------------------------------------------


_FX NTSTATUS Gui_Api_Init(PROCESS *proc, ULONG64 *parms)
{
    NTSTATUS status = STATUS_SUCCESS;

#ifndef _WIN64

    if (Driver_OsVersion < DRIVER_WINDOWS_VISTA) {

        status = Gui_Api_Init_XpHook(proc, parms);
    }

#endif ! _WIN64

    if (NT_SUCCESS(status) && (! Process_ReadyToSandbox)) {

        Process_ReadyToSandbox = TRUE;
    }

    return status;
}


//---------------------------------------------------------------------------
// Gui_Unload
//---------------------------------------------------------------------------


_FX void Gui_Unload(void)
{
#ifndef _WIN64

    if (Driver_OsVersion < DRIVER_WINDOWS_VISTA) {

        Gui_Unload_XpHook();
    }

#endif ! _WIN64
}


//---------------------------------------------------------------------------
// Gui_InitProcess
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_InitProcess(PROCESS *proc)
{
    //static const WCHAR *_OpenClass = L"OpenWinClass";
    //static const WCHAR *_Asterisk  = L"*";
    static const WCHAR *openclasses[] = {
        L"Shell_TrayWnd",
        L"TrayNotifyWnd",
        L"SystemTray_Main",                 // Power Meter dialog box
        L"Connections Tray",
        L"MS_WebcheckMonitor",
        L"PrintTray_Notify_WndClass",
        L"CicLoaderWndClass",
        L"CicMarshalWndClass",
        L"Credential Dialog Xaml Host",
        //
        // hardware
        //
        L"Logitech Wingman Internal Message Router",    // joystick
        L"devldr",                          // devldr32 sound card driver
        //
        // third party window classes
        //
        L"CTouchPadSynchronizer",
        L"Type32_Main_Window",
        L"TForm_AshampooFirewall",
        L"WinVNC desktop sink",             // Tight VNC
        L"Afx:400000:0",                    // Download Accelerator Plus
        L"NVIDIA TwinView Window",          // nVidia nView
        L"SWFlash_PlaceHolderX",
        L"MdiClass",                        // PowerPoint
        NULL
    };
    ULONG i;
    BOOLEAN ok;

    ok = Process_GetPaths(
            proc, &proc->open_win_classes, Gui_OpenClass_Name, FALSE);

    if (ok) {
        for (i = 0; openclasses[i] && ok; ++i) {
            ok = Process_AddPath(proc, &proc->open_win_classes, NULL,
                                 TRUE, openclasses[i], FALSE);
        }
    }

    if (ok) {
        BOOLEAN AddMSTaskSwWClass = FALSE;
        if (Driver_OsVersion >= DRIVER_WINDOWS_7) {
            ok = Process_AddPath(
                    proc, &proc->open_win_classes, NULL,
                    TRUE, L"Sandbox:*:ConsoleWindowClass", FALSE);
            AddMSTaskSwWClass = TRUE;
        } else if ((! proc->image_from_box) &&
                (  _wcsicmp(proc->image_name, L"excel.exe")    == 0
                || _wcsicmp(proc->image_name, L"powerpnt.exe") == 0))
            AddMSTaskSwWClass = TRUE;
        if (ok && AddMSTaskSwWClass) {
            ok = Process_AddPath(proc, &proc->open_win_classes, NULL,
                                 TRUE, L"MSTaskSwWClass", FALSE);
        }
    }

    /*if (ok) {
        BOOLEAN is_closed;
        Process_MatchPath(
            proc->pool, _Asterisk, 1, &proc->open_win_classes, NULL,
            &proc->open_all_win_classes, &is_closed);
    }*/

    if (! ok)
        Log_MsgP1(MSG_INIT_PATHS, Gui_OpenClass_Name, proc->pid);

    return ok;
}


//---------------------------------------------------------------------------
// Gui_Check_OpenWinClass
//---------------------------------------------------------------------------


_FX void Gui_Check_OpenWinClass(PROCESS *proc)
{
    ULONG index = 0;

    Conf_AdjustUseCount(TRUE);

    while (1) {

        const WCHAR *value = Conf_Get(proc->box->name, Gui_OpenClass_Name,
                (index | CONF_GET_NO_EXPAND | CONF_GET_NO_TEMPLS));
        if (! value)
            break;

        if (value[0] == L'*' && value[1] == L'\0') {

            proc->open_all_win_classes = TRUE;
            break;
        }

        ++index;
    }

    Conf_AdjustUseCount(FALSE);
}


//---------------------------------------------------------------------------
//
// Workaround for bug in Windows Vista and later concerning clipboard which
// prevents a process outside the sandbox from accessing data copied into
// the clipboard by a process in the sandbox.
//
// The clipboard data area, within the window station object, contains
// a list of clipboard items.  In addition to format type and pointer,
// UIPI also stores the integrity level of the process for each item.
// There seems to be a bug in win32k!FindClipFormat when the item IL = 0,
// which would be the case for a process in the sandbox.
//
// to work around this, we have an API call (API_GUI_CLIPBOARD) that fixes
// the IL for each clipboard item.  function CloseClipboardSlave in file
// core/svc/GuiServer.cpp invokes this API in response to a request from
// Gui_CloseClipboard in file core/dll/gui.c.
//
// unfortunately the structure of the internal clipboard item structure
// varies randomly between versions of Windows, so we have SbieSvc put
// some data on the clipboard and make a special request which allows us
// to look at the clipboard contents and figure it out.
//
//---------------------------------------------------------------------------


typedef struct _GUI_CLIPBOARD {
    ULONG *items;
    ULONG count;
} GUI_CLIPBOARD;

static GUI_CLIPBOARD *Gui_GetClipboard(void);
static void Gui_InitClipboard();
static void Gui_FixClipboard(ULONG integrity);

static ULONG Gui_ClipboardItemLength = 0;
static ULONG Gui_ClipboardIntegrityIndex = 0;


//---------------------------------------------------------------------------
// Gui_GetClipboard
//---------------------------------------------------------------------------


_FX GUI_CLIPBOARD *Gui_GetClipboard(void)
{
    HANDLE WinStaHandle;
    void *WinStaObject;
    GUI_CLIPBOARD *Clipboard;
    NTSTATUS status;

    //
    // Clipboard offset can be found in win32k!FindClipFormat
    // In windows 10 find the offset in win32kfull!FindClipFormat

    ULONG Clipboard_Offset = 0;

    // Hard Offset Dependency

#ifdef _WIN64
    if (Driver_OsVersion <= DRIVER_WINDOWS_7) {
        Clipboard_Offset = 0x58;
    }
    else if (Driver_OsBuild < 18980) {      // Covers Win 8 up through Win 10-18980
        Clipboard_Offset = 0x60;
    }
    else
        Clipboard_Offset = 0x80;

#else ! _WIN64
    if (Driver_OsVersion <= DRIVER_WINDOWS_7) {
        Clipboard_Offset = 0x2c;
    }
    else if (Driver_OsBuild < 18980) {      // Covers Win 8 up through Win 10-18980
        Clipboard_Offset = 0x30;
    }
    else
        Clipboard_Offset = 0x40;
#endif _WIN64

    //
    // get the window station object to which caller is connected
    //

    WinStaHandle = PsGetProcessWin32WindowStation(PsGetCurrentProcess());
    if (! WinStaHandle)
        return NULL;

    status = ObReferenceObjectByHandle(
                WinStaHandle, 0, *ExWindowStationObjectType, KernelMode,
                &WinStaObject, NULL);

    if (! NT_SUCCESS(status))
        return NULL;

    ObDereferenceObject(WinStaObject);

    //
    // get the clipboard data in the window station object
    //

    Clipboard = (GUI_CLIPBOARD *) ((ULONG_PTR)WinStaObject + Clipboard_Offset);

    if (Clipboard->items && Clipboard->count)
        return Clipboard;

    return NULL;
}


//---------------------------------------------------------------------------
// Gui_InitClipboard
//---------------------------------------------------------------------------


_FX void Gui_InitClipboard(void)
{
    ULONG *ptr;
    ULONG x2, x3, x4, i;

    GUI_CLIPBOARD *Clipboard = Gui_GetClipboard();
    if (! Clipboard)
        return;

    //
    // analyze the structure of the clipboard item area.  InitClipboard
    // in core/svc/DriverAssistStart.cpp has placed four unique items
    // on the clipboard.
    //

    if (Clipboard->count < 4)
        return;

    ptr = Clipboard->items;
    if (*ptr != 0x111111)
        return;

    //
    // after we make sure the data area begins with 0x111111, the first
    // clipboard item format, scan ahead to see where we find 0x222222,
    // the format for the second clipboard item
    //
    // we do the same for the third and fourth items, and if all the
    // lengths match, we can assume we found the length of an item entry
    //

    for (x2 = 0; (x2 < 12) && (*ptr != 0x222222); ++x2, ++ptr)
        ;
    if (*ptr != 0x222222)
        return;

    for (x3 = 0; (x3 < 12) && (*ptr != 0x333333); ++x3, ++ptr)
        ;
    if (*ptr != 0x333333)
        return;

    for (x4 = 0; (x4 < 12) && (*ptr != 0x444444); ++x4, ++ptr)
        ;
    if (*ptr != 0x444444)
        return;

    if (x2 != x3 || x3 != x4)
        return;

    //
    // now we need to scan the data area to see which ULONG contains
    // the integrity level number.  it should be 0x4000 because we are
    // called from SbieSvc.  we make sure it is 0x4000 in all four
    // entries, then we can assume we have the right ULONG
    //

    ptr = Clipboard->items;
    for (i = 0; (i < 12) && (*ptr != 0x4000); ++i, ++ptr)
        ;
    if (*ptr != 0x4000) {                   // 0x111111

        //
        // we have a valid item length, but cannot find integrity level,
        // probably means that UAC/UIPI is turned off
        //

        Gui_ClipboardItemLength = x2;
        Gui_ClipboardIntegrityIndex = -1;

        return;
    }

    ptr += x2;
    if (*ptr != 0x4000)                     // 0x222222
        return;
    ptr += x2;
    if (*ptr != 0x4000)                     // 0x333333
        return;
    ptr += x2;
    if (*ptr != 0x4000)                     // 0x444444
        return;

    //
    // finish
    //

    Gui_ClipboardItemLength = x2;
    Gui_ClipboardIntegrityIndex = i;
}


//---------------------------------------------------------------------------
// Gui_FixClipboard
//---------------------------------------------------------------------------


_FX void Gui_FixClipboard(ULONG integrity)
{
    if (Gui_ClipboardIntegrityIndex != -1) {    // do nothing if UIPI is off

        ULONG i;
        ULONG *ptr;

        GUI_CLIPBOARD *Clipboard = Gui_GetClipboard();
        if (! Clipboard)
            return;

        ptr = Clipboard->items;
        for (i = 0; i < Clipboard->count; ++i) {
            const ULONG il = ptr[Gui_ClipboardIntegrityIndex] & ~1;
            if ((il == 0x0000) || (il == 0x1000) || (il == 0x2000)
                               || (il == 0x3000) || (il == 0x4000)) {
                ptr[Gui_ClipboardIntegrityIndex] = integrity;
            }
            ptr += Gui_ClipboardItemLength;
        }
    }
}


//---------------------------------------------------------------------------
// Gui_Api_Clipboard
//---------------------------------------------------------------------------


_FX NTSTATUS Gui_Api_Clipboard(PROCESS *proc, ULONG64 *parms)
{
    //
    // make sure we are on Windows Vista or later
    //

    if (Driver_OsVersion < DRIVER_WINDOWS_VISTA)
        return STATUS_NOT_SUPPORTED;

    //
    // make sure we are called from an SbieSvc process, which would be either
    // the main process (for the init call in core/svc/DriverAssistStart.cpp)
    // or the GUI proxy process (for the call from core/svc/GuiServer.cpp)
    //
    // the reason for this is we don't have any way to lock the clipboard
    // data in the window station object while we're using it, so we have
    // to rely on the calling process to open (and lock) the clipboard before
    // calling us here, and we can only rely on our own service process
    //

    if (proc || (! MyIsCallerMyServiceProcess()))
        return STATUS_ACCESS_DENIED;

    //
    // handle call according to parameters
    //

    if ((ULONG_PTR)parms[1] == -1) {

        if (! Gui_ClipboardItemLength)
            Gui_InitClipboard();

    } else {

        if (Gui_ClipboardItemLength && Gui_ClipboardIntegrityIndex)
            Gui_FixClipboard((ULONG)parms[1]);
        else
            return STATUS_UNKNOWN_REVISION;
    }

    return STATUS_SUCCESS;
}
