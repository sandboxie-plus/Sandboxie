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
#include "common/my_version.h"
#include <stdio.h>

#pragma auto_inline(off)


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static void Gui_InitPropAtoms(void);

static const void *Gui_ReplaceAtom(const void *lpString);

HANDLE Gui_GetPropCommon(
    HWND hWnd, const void *lpString, BOOLEAN unicode, ULONG LastError);

static HANDLE Gui_GetPropW(HWND hWnd, const WCHAR *lpString);

static HANDLE Gui_GetPropA(HWND hWnd, const UCHAR *lpString);

static BOOL Gui_SetPropW(HWND hWnd, const WCHAR *lpString, HANDLE hData);

static BOOL Gui_SetPropA(HWND hWnd, const UCHAR *lpString, HANDLE hData);

static HANDLE Gui_RemovePropW(HWND hWnd, const WCHAR *lpString);

static HANDLE Gui_RemovePropA(HWND hWnd, const UCHAR *lpString);

static ULONG_PTR Gui_GetLongCommon(HWND hWnd, int nIndex, ULONG which);

static ULONG_PTR Gui_GetWindowLong_DragDrop(
    HWND hWnd, int nIndex, ULONG_PTR value);

static ULONG_PTR Gui_SetWindowLong_DragDrop(
    HWND hWnd, int nIndex, ULONG_PTR dwNew, ULONG_PTR value);

static ULONG Gui_GetWindowLongW(HWND hWnd, int nIndex);

static ULONG Gui_GetWindowLongA(HWND hWnd, int nIndex);

static ULONG Gui_SetWindowLongW(HWND hWnd, int nIndex, ULONG dwNew);

static ULONG Gui_SetWindowLongA(HWND hWnd, int nIndex, ULONG dwNew);

static ULONG_PTR Gui_SetWindowLong8(
    HWND hWnd, int nIndex, ULONG_PTR dwNew, ULONG IsAscii);

static BOOLEAN Gui_Hook_SetWindowLong8(void);

static ULONG Gui_GetClassLongW(HWND hWnd, int nIndex);

static ULONG Gui_GetClassLongA(HWND hWnd, int nIndex);

#ifdef _WIN64

static ULONG_PTR Gui_GetWindowLongPtrW(HWND hWnd, int nIndex);

static ULONG_PTR Gui_GetWindowLongPtrA(HWND hWnd, int nIndex);

static ULONG_PTR Gui_SetWindowLongPtrW(
    HWND hWnd, int nIndex, ULONG_PTR dwNew);

static ULONG_PTR Gui_SetWindowLongPtrA(
    HWND hWnd, int nIndex, ULONG_PTR dwNew);

static ULONG_PTR Gui_SetWindowLongPtr8(
    HWND hWnd, int nIndex, ULONG_PTR dwNew, ULONG IsAscii);

static BOOLEAN Gui_Hook_SetWindowLongPtr8(void);

static ULONG_PTR Gui_GetClassLongPtrW(HWND hWnd, int nIndex);

static ULONG_PTR Gui_GetClassLongPtrA(HWND hWnd, int nIndex);

#endif _WIN64


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------

       ATOM Gui_DropTargetProp_Atom = 0;

       ATOM Gui_WindowProcOldW_Atom = 0;
       ATOM Gui_WindowProcOldA_Atom = 0;
static ATOM Gui_WindowProcNewW_Atom = 0;
static ATOM Gui_WindowProcNewA_Atom = 0;

static ATOM Gui_OleDropTargetInterface_Atom           = 0;
static ATOM Gui_OleDropTargetMarshalHwnd_Atom         = 0;
static ATOM Gui_OleEndPointID_Atom                    = 0;
static ATOM Gui_Sandbox_OleDropTargetInterface_Atom   = 0;
static ATOM Gui_Sandbox_OleDropTargetMarshalHwnd_Atom = 0;
static ATOM Gui_Sandbox_OleEndPointID_Atom            = 0;


#ifdef _WIN64

static P_SetWindowLong8            __sys_SetWindowLong8         = 0;
static P_SetWindowLongPtr8         __sys_SetWindowLongPtr8      = 0;

#endif _WIN64


//---------------------------------------------------------------------------
// Gui_InitProp
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_InitProp(void)
{
    //
    // initialize our Drag-n-Drop atoms
    //

    Gui_InitPropAtoms();

    //
    // hook functions
    //

    if (! SbieDll_IsOpenCOM()) {

        //
        // if there is access to the real COM epmapper, then don't
        // hook Prop functions that are used to hide drag/drop props
        //

        SBIEDLL_HOOK_GUI(GetPropA);
        SBIEDLL_HOOK_GUI(GetPropW);
        SBIEDLL_HOOK_GUI(SetPropA);
        SBIEDLL_HOOK_GUI(SetPropW);
        SBIEDLL_HOOK_GUI(RemovePropA);
        SBIEDLL_HOOK_GUI(RemovePropW);
    }

    if (! Gui_OpenAllWinClasses) {

        //
        // if not hooking window classes, don't install custom wndproc
        //

        SBIEDLL_HOOK_GUI(GetWindowLongA);
        SBIEDLL_HOOK_GUI(GetWindowLongW);

#ifdef _WIN64

        // new style hook on SetWindowLong on 64-bit Windows 8.1 and later
        if (Dll_OsBuild < 9600) {
            SBIEDLL_HOOK_GUI(SetWindowLongA);
            SBIEDLL_HOOK_GUI(SetWindowLongW);

        } else if (! Gui_Hook_SetWindowLong8())
            return FALSE;

#else ! _WIN64

        // otherwise old style hooks on SetWindowLongA and SetWindowLongW
        SBIEDLL_HOOK_GUI(SetWindowLongA);
        SBIEDLL_HOOK_GUI(SetWindowLongW);

#endif _WIN64

        SBIEDLL_HOOK_GUI(GetClassLongA);
        SBIEDLL_HOOK_GUI(GetClassLongW);

#ifdef _WIN64

        SBIEDLL_HOOK_GUI(GetWindowLongPtrA);
        SBIEDLL_HOOK_GUI(GetWindowLongPtrW);

        // special hook on SetWindowLongPtr on 64-bit Windows 8 and later
        if (Dll_OsBuild < 8400) {
            SBIEDLL_HOOK_GUI(SetWindowLongPtrA);
            SBIEDLL_HOOK_GUI(SetWindowLongPtrW);

        } else if (! Gui_Hook_SetWindowLongPtr8())
            return FALSE;

        SBIEDLL_HOOK_GUI(GetClassLongPtrA);
        SBIEDLL_HOOK_GUI(GetClassLongPtrW);

#endif _WIN64

    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_InitPropAtoms
//---------------------------------------------------------------------------


_FX void Gui_InitPropAtoms(void)
{
    //
    // drag drop props
    //

    static const WCHAR *_OleDropTargetInterface = L"OleDropTargetInterface";
    static const WCHAR *_OleDropTargetMarshalHwnd =
        L"OleDropTargetMarshalHwnd";
    static const WCHAR *_OleEndPointID =
        L"OleEndPointID";
    static const WCHAR *_format = L"Sandbox:%s:%s";
    WCHAR name[128];

    if (! Gui_OleDropTargetInterface_Atom) {

        Gui_OleDropTargetInterface_Atom =
            GlobalAddAtomW(_OleDropTargetInterface);

        if (Gui_OleDropTargetInterface_Atom) {
            Sbie_swprintf(name, _format, Dll_BoxName, _OleDropTargetInterface);
            Gui_Sandbox_OleDropTargetInterface_Atom = GlobalAddAtomW(name);
        }
    }

    if (! Gui_OleDropTargetMarshalHwnd_Atom) {

        Gui_OleDropTargetMarshalHwnd_Atom =
            GlobalAddAtomW(_OleDropTargetMarshalHwnd);

        if (Gui_OleDropTargetMarshalHwnd_Atom) {
            Sbie_swprintf(name, _format, Dll_BoxName, _OleDropTargetMarshalHwnd);
            Gui_Sandbox_OleDropTargetMarshalHwnd_Atom = GlobalAddAtomW(name);
        }
    }

    if (! Gui_OleEndPointID_Atom) {

        Gui_OleEndPointID_Atom =
            GlobalAddAtomW(_OleEndPointID);

        if (Gui_OleEndPointID_Atom) {
            Sbie_swprintf(name, _format, Dll_BoxName, _OleEndPointID);
            Gui_Sandbox_OleEndPointID_Atom = GlobalAddAtomW(name);
        }
    }

    //
    // window/dialog procedure props
    //

    if (! Gui_DropTargetProp_Atom)
        Gui_DropTargetProp_Atom = GlobalAddAtomW(SBIE L"_DropTarget");

    if (! Gui_WindowProcOldW_Atom)
        Gui_WindowProcOldW_Atom = GlobalAddAtomW(SBIE L"_WindowProcOldW");

    if (! Gui_WindowProcOldA_Atom)
        Gui_WindowProcOldA_Atom = GlobalAddAtomW(SBIE L"_WindowProcOldA");

    if (! Gui_WindowProcNewW_Atom)
        Gui_WindowProcNewW_Atom = GlobalAddAtomW(SBIE L"_WindowProcNewW");

    if (! Gui_WindowProcNewA_Atom)
        Gui_WindowProcNewA_Atom = GlobalAddAtomW(SBIE L"_WindowProcNewA");
}


//---------------------------------------------------------------------------
// Gui_ReplaceAtom
//---------------------------------------------------------------------------


_FX const void *Gui_ReplaceAtom(const void *lpString)
{
    Gui_InitPropAtoms();

    if (((LONG_PTR)lpString & ~0xFFFF) == 0) {
        LONG_PTR atom = (LONG_PTR)lpString & 0xFFFF;
        if (atom == Gui_OleDropTargetInterface_Atom)
            (ATOM)lpString = Gui_Sandbox_OleDropTargetInterface_Atom;
        if (atom == Gui_OleDropTargetMarshalHwnd_Atom)
            (ATOM)lpString = Gui_Sandbox_OleDropTargetMarshalHwnd_Atom;
        if (atom == Gui_OleEndPointID_Atom)
            (ATOM)lpString = Gui_Sandbox_OleEndPointID_Atom;
    }

    return lpString;
}


//---------------------------------------------------------------------------
// Gui_GetPropCommon
//---------------------------------------------------------------------------


_FX HANDLE Gui_GetPropCommon(
    HWND hWnd, const void *lpString, BOOLEAN unicode, ULONG LastError)
{
    GUI_GET_WINDOW_PROP_REQ req;
    GUI_GET_WINDOW_PROP_RPL *rpl;
    HANDLE handle;
    ULONG error;
    ULONG len;

    memzero(&req, sizeof(req));

    req.msgid = GUI_GET_WINDOW_PROP;
    req.error = LastError;
    req.hwnd = (ULONG)(ULONG_PTR)hWnd;
    req.unicode = unicode;

    if (((ULONG_PTR)lpString & 0xFFFF0000) == 0) {
        req.prop_atom = ((ULONG)(ULONG_PTR)lpString) & 0xFFFF;

    } else if (unicode) {
        len = wcslen((WCHAR *)lpString);
        if (len >= RTL_NUMBER_OF_V1(req.prop_text) - 1)
            return NULL;
        wmemcpy(req.prop_text, lpString, len);
        req.prop_is_text = TRUE;

    } else {
        len = strlen((char *)lpString);
        if (len >= RTL_NUMBER_OF_V1(req.prop_text) - 1)
            return NULL;
        memcpy(req.prop_text, lpString, len);
        req.prop_is_text = TRUE;
    }

    rpl = Gui_CallProxy(&req, sizeof(req), sizeof(GUI_GET_WINDOW_PROP_RPL));
    if (! rpl)
        return FALSE;

    handle = (HANDLE)(ULONG_PTR)rpl->result;
    error = rpl->error;
    Dll_Free(rpl);

    SetLastError(error);
    return handle;
}


//---------------------------------------------------------------------------
// Gui_GetPropW
//---------------------------------------------------------------------------


_FX HANDLE Gui_GetPropW(HWND hWnd, const WCHAR *lpString)
{
    if (! Gui_OpenAllWinClasses) {

        //
        // normal mode
        //

        ULONG LastError = GetLastError();

        if (! Gui_IsWindowAccessible(hWnd)) {
            if (hWnd != __sys_GetDesktopWindow()) {
                SetLastError(ERROR_INVALID_WINDOW_HANDLE);
                return NULL;
            }
        }

        lpString = Gui_ReplaceAtom(lpString);

        if (Gui_IsSameBox(hWnd, NULL, NULL))
            return __sys_GetPropW(hWnd, lpString);
        else
            return Gui_GetPropCommon(hWnd, lpString, TRUE, LastError);

    } else {

        //
        // OpenWinClass=* mode
        //

        lpString = Gui_ReplaceAtom(lpString);
        return __sys_GetPropW(hWnd, lpString);
    }
}


//---------------------------------------------------------------------------
// Gui_GetPropA
//---------------------------------------------------------------------------


_FX HANDLE Gui_GetPropA(HWND hWnd, const UCHAR *lpString)
{
    if (! Gui_OpenAllWinClasses) {

        //
        // normal mode
        //

        ULONG LastError = GetLastError();

        if (! Gui_IsWindowAccessible(hWnd)) {
            if (hWnd != __sys_GetDesktopWindow()) {
                SetLastError(ERROR_INVALID_WINDOW_HANDLE);
                return NULL;
            }
        }

        lpString = Gui_ReplaceAtom(lpString);

        if (Gui_IsSameBox(hWnd, NULL, NULL))
            return __sys_GetPropA(hWnd, lpString);
        else
            return Gui_GetPropCommon(hWnd, lpString, TRUE, LastError);

    } else {

        //
        // OpenWinClass=* mode
        //

        lpString = Gui_ReplaceAtom(lpString);
        return __sys_GetPropA(hWnd, lpString);
    }
}


//---------------------------------------------------------------------------
// Gui_SetPropW
//---------------------------------------------------------------------------


_FX BOOL Gui_SetPropW(HWND hWnd, const WCHAR *lpString, HANDLE hData)
{
    if (! Gui_OpenAllWinClasses) {

        if (! Gui_IsWindowAccessible(hWnd)) {
            SetLastError(ERROR_INVALID_WINDOW_HANDLE);
            return FALSE;
        }
    }

    lpString = Gui_ReplaceAtom(lpString);

    return __sys_SetPropW(hWnd, lpString, hData);
}


//---------------------------------------------------------------------------
// Gui_SetPropA
//---------------------------------------------------------------------------


_FX BOOL Gui_SetPropA(HWND hWnd, const UCHAR *lpString, HANDLE hData)
{
    if (! Gui_OpenAllWinClasses) {

        if (! Gui_IsWindowAccessible(hWnd)) {
            SetLastError(ERROR_INVALID_WINDOW_HANDLE);
            return FALSE;
        }
    }

    lpString = Gui_ReplaceAtom(lpString);

    return __sys_SetPropA(hWnd, lpString, hData);
}


//---------------------------------------------------------------------------
// Gui_RemovePropW
//---------------------------------------------------------------------------


_FX HANDLE Gui_RemovePropW(HWND hWnd, const WCHAR *lpString)
{
    if (! Gui_OpenAllWinClasses) {

        if (! Gui_IsWindowAccessible(hWnd)) {
            SetLastError(ERROR_INVALID_WINDOW_HANDLE);
            return NULL;
        }
    }

    lpString = Gui_ReplaceAtom(lpString);

    return __sys_RemovePropW(hWnd, lpString);
}


//---------------------------------------------------------------------------
// Gui_RemovePropA
//---------------------------------------------------------------------------


_FX HANDLE Gui_RemovePropA(HWND hWnd, const UCHAR *lpString)
{
    if (! Gui_OpenAllWinClasses) {

        if (! Gui_IsWindowAccessible(hWnd)) {
            SetLastError(ERROR_INVALID_WINDOW_HANDLE);
            return NULL;
        }
    }

    lpString = Gui_ReplaceAtom(lpString);

    return __sys_RemovePropA(hWnd, lpString);
}


//---------------------------------------------------------------------------
// Gui_GetLongCommon
//---------------------------------------------------------------------------


_FX ULONG_PTR Gui_GetLongCommon(HWND hWnd, int nIndex, ULONG which)
{
    ULONG_PTR result;
    ULONG LastError = GetLastError();

    if (Gui_IsSameBox(hWnd, NULL, NULL)) {

        //
        // if target window is in the same sandbox (i.e. within the
        // boundary of the job object) then issue a direct call
        //

        if (which == 'cl w')
            result = __sys_GetClassLongW(hWnd, nIndex);
        else if (which == 'cl a')
            result = __sys_GetClassLongA(hWnd, nIndex);

        else if (which == 'wl w')
            result = __sys_GetWindowLongW(hWnd, nIndex);
        else if (which == 'wl a')
            result = __sys_GetWindowLongA(hWnd, nIndex);

#ifdef _WIN64

        else if (which == 'clpw')
            result = __sys_GetClassLongPtrW(hWnd, nIndex);
        else if (which == 'clpa')
            result = __sys_GetClassLongPtrA(hWnd, nIndex);

        else if (which == 'wlpw')
            result = __sys_GetWindowLongPtrW(hWnd, nIndex);
        else if (which == 'wlpa')
            result = __sys_GetWindowLongPtrA(hWnd, nIndex);

#endif _WIN64

        else {
            result = 0;
            SetLastError(ERROR_INVALID_PARAMETER);
        }

    } else {

        //
        // if target window is outside the job, issue an indirect request
        //

        GUI_GET_WINDOW_LONG_REQ req;
        GUI_GET_WINDOW_LONG_RPL *rpl;
        ULONG error;

        req.msgid = GUI_GET_WINDOW_LONG;
        req.error = LastError;
        req.hwnd = (ULONG)(ULONG_PTR)hWnd;
        req.index = nIndex;
        req.which = which;
        rpl = Gui_CallProxy(
                &req, sizeof(req), sizeof(GUI_GET_WINDOW_LONG_RPL));
        if (! rpl)
            return FALSE;

        result = (ULONG_PTR)rpl->result;
        error = rpl->error;
        Dll_Free(rpl);
        SetLastError(error);
    }

    return result;
}


//---------------------------------------------------------------------------
// Gui_GetWindowLong_DragDrop
//---------------------------------------------------------------------------


_FX ULONG_PTR Gui_GetWindowLong_DragDrop(
    HWND hWnd, int nIndex, ULONG_PTR value)
{
    if (nIndex == GWL_EXSTYLE && (value & WS_EX_ACCEPTFILES)) {

        //
        // if the style WS_EX_ACCEPTFILES is there because we put it
        // there (see Ole_RegisterDragDrop) to make it possible for
        // unsandboxed windows to drop files in sandboxed windows,
        // then hide it from sandboxed processes (because they can
        // find and use the sandboxed IDropTarget)
        //

        void *pDropTarget =
                    __sys_GetPropW(hWnd, (LPCWSTR)Gui_DropTargetProp_Atom);
        if (pDropTarget)
            value &= ~WS_EX_ACCEPTFILES;
    }

    return value;
}


//---------------------------------------------------------------------------
// Gui_SetWindowLong_DragDrop
//---------------------------------------------------------------------------


_FX ULONG_PTR Gui_SetWindowLong_DragDrop(
    HWND hWnd, int nIndex, ULONG_PTR dwNew, ULONG_PTR value)
{
    if (nIndex == GWL_EXSTYLE) {

        if (dwNew & WS_EX_ACCEPTFILES) {

            //
            // if the app is explicitly setting the WS_EX_ACCEPTFILES flag
            // then make sure we are not trying to handle WM_DROPFILES on
            // its behalf
            //
            // Note:  Updated to not remove our property.  With Win10 Creator's
            //        update and Win8.1, drag-n-drop was not working because
            //        we were removing it below and thus the IDropTarget was
            //        not found in Ole_DoDragDrop().
            //

            if (Gui_DropTargetProp_Atom)
            {
                // __sys_RemovePropW(hWnd, (LPCWSTR)Gui_DropTargetProp_Atom);
            }

        } else {

            //
            // if the app does a SetWindowLong(GetWindowLong(), ...)
            // sequence on the window, it will clear our WS_EX_ACCEPTFILES
            // flag (per Gui_GetWindowLong_DragDrop) so apply it again
            //

            void *pDropTarget =
                    __sys_GetPropW(hWnd, (LPCWSTR)Gui_DropTargetProp_Atom);
            if (pDropTarget) {
               dwNew |= WS_EX_ACCEPTFILES;
                __sys_SetWindowLongW(hWnd, nIndex, (ULONG)dwNew);
            }
        }
    }

    return value;
}


//---------------------------------------------------------------------------
// Gui_GetWindowLongW
//---------------------------------------------------------------------------


_FX ULONG Gui_GetWindowLongW(HWND hWnd, int nIndex)
{
    ULONG value;
    Gui_InitPropAtoms();
    //value = __sys_GetWindowLongW(hWnd, nIndex);
    value = (ULONG)Gui_GetLongCommon(hWnd, nIndex, 'wl w');

#ifndef _WIN64

    if (Gui_RenameClasses && (nIndex == GWLP_WNDPROC)) {

        //
        // prevent caller from seeing our window procedure
        //

        WNDPROC w = __sys_GetPropW(hWnd, (LPCWSTR)Gui_WindowProcNewW_Atom);
        WNDPROC a = __sys_GetPropW(hWnd, (LPCWSTR)Gui_WindowProcNewA_Atom);

        if ((w && value == (ULONG_PTR)w) || (a && value == (ULONG_PTR)a)) {

            value = (ULONG_PTR)__sys_GetPropW(
                                    hWnd, (LPCWSTR)Gui_WindowProcOldW_Atom);
        }
    }

#endif ! _WIN64

    return (ULONG)Gui_GetWindowLong_DragDrop(hWnd, nIndex, value);
}


//---------------------------------------------------------------------------
// Gui_GetWindowLongA
//---------------------------------------------------------------------------


_FX ULONG Gui_GetWindowLongA(HWND hWnd, int nIndex)
{
    ULONG value;
    Gui_InitPropAtoms();
    //value = __sys_GetWindowLongA(hWnd, nIndex);
    value = (ULONG)Gui_GetLongCommon(hWnd, nIndex, 'wl a');

#ifndef _WIN64

    if (Gui_RenameClasses && (nIndex == GWLP_WNDPROC)) {

        //
        // prevent caller from seeing our window procedure
        //

        WNDPROC w = __sys_GetPropW(hWnd, (LPCWSTR)Gui_WindowProcNewW_Atom);
        WNDPROC a = __sys_GetPropW(hWnd, (LPCWSTR)Gui_WindowProcNewA_Atom);

        if ((w && value == (ULONG_PTR)w) || (a && value == (ULONG_PTR)a)) {

            value = (ULONG_PTR)__sys_GetPropW(
                                    hWnd, (LPCWSTR)Gui_WindowProcOldA_Atom);
        }
    }

#endif ! _WIN64

    return (ULONG)Gui_GetWindowLong_DragDrop(hWnd, nIndex, value);
}


//---------------------------------------------------------------------------
// Gui_SetWindowLongW
//---------------------------------------------------------------------------


_FX ULONG Gui_SetWindowLongW(HWND hWnd, int nIndex, ULONG dwNew)
{
    ULONG value;
    Gui_InitPropAtoms();
    value = __sys_SetWindowLongW(hWnd, nIndex, dwNew);

#ifndef _WIN64

    if (Gui_RenameClasses && (nIndex == GWLP_WNDPROC)) {

        //
        // replace window procedure, then override the new procedure
        //

        WNDPROC w = __sys_GetPropW(hWnd, (LPCWSTR)Gui_WindowProcNewW_Atom);
        WNDPROC a = __sys_GetPropW(hWnd, (LPCWSTR)Gui_WindowProcNewA_Atom);

        if ((w && value == (ULONG_PTR)w) || (a && value == (ULONG_PTR)a)) {

            value = (ULONG_PTR)__sys_GetPropW(
                                    hWnd, (LPCWSTR)Gui_WindowProcOldW_Atom);

            Gui_SetWindowProc(hWnd, TRUE);
        }
    }

#endif ! _WIN64

    return (ULONG)Gui_SetWindowLong_DragDrop(hWnd, nIndex, dwNew, value);
}


//---------------------------------------------------------------------------
// Gui_SetWindowLongA
//---------------------------------------------------------------------------


_FX ULONG Gui_SetWindowLongA(HWND hWnd, int nIndex, ULONG dwNew)
{
    ULONG value;
    Gui_InitPropAtoms();
    value = __sys_SetWindowLongA(hWnd, nIndex, dwNew);

#ifndef _WIN64

    if (Gui_RenameClasses && (nIndex == GWLP_WNDPROC)) {

        //
        // replace window procedure, then override the new procedure
        //

        WNDPROC w = __sys_GetPropW(hWnd, (LPCWSTR)Gui_WindowProcNewW_Atom);
        WNDPROC a = __sys_GetPropW(hWnd, (LPCWSTR)Gui_WindowProcNewA_Atom);

        if ((w && value == (ULONG_PTR)w) || (a && value == (ULONG_PTR)a)) {

            value = (ULONG_PTR)__sys_GetPropW(
                                    hWnd, (LPCWSTR)Gui_WindowProcOldA_Atom);

            Gui_SetWindowProc(hWnd, TRUE);
        }
    }

#endif ! _WIN64

    return (ULONG)Gui_SetWindowLong_DragDrop(hWnd, nIndex, dwNew, value);
}


//---------------------------------------------------------------------------
// Gui_GetClassLongW
//---------------------------------------------------------------------------


_FX ULONG Gui_GetClassLongW(HWND hWnd, int nIndex)
{
    return (ULONG)Gui_GetLongCommon(hWnd, nIndex, 'cl w');
}


//---------------------------------------------------------------------------
// Gui_GetClassLongA
//---------------------------------------------------------------------------


_FX ULONG Gui_GetClassLongA(HWND hWnd, int nIndex)
{
    return (ULONG)Gui_GetLongCommon(hWnd, nIndex, 'cl a');
}


//---------------------------------------------------------------------------
// Begin 64-bit Get/SetWindowLongPtr functions
//---------------------------------------------------------------------------


#ifdef _WIN64


//---------------------------------------------------------------------------
// Gui_GetWindowLongPtrW
//---------------------------------------------------------------------------


_FX ULONG_PTR Gui_GetWindowLongPtrW(HWND hWnd, int nIndex)
{
    ULONG_PTR value;
    Gui_InitPropAtoms();
    //value = (ULONG_PTR)__sys_GetWindowLongPtrW(hWnd, nIndex);
    value = (ULONG_PTR)Gui_GetLongCommon(hWnd, nIndex, 'wlpw');

    if (Gui_RenameClasses && (nIndex == GWLP_WNDPROC)) {

        //
        // prevent caller from seeing our window procedure
        //

        WNDPROC w = __sys_GetPropW(hWnd, (LPCWSTR)Gui_WindowProcNewW_Atom);
        WNDPROC a = __sys_GetPropW(hWnd, (LPCWSTR)Gui_WindowProcNewA_Atom);

        if ((w && value == (ULONG_PTR)w) || (a && value == (ULONG_PTR)a)) {

            value = (ULONG_PTR)__sys_GetPropW(
                                    hWnd, (LPCWSTR)Gui_WindowProcOldW_Atom);
        }
    }

    return Gui_GetWindowLong_DragDrop(hWnd, nIndex, value);
}


//---------------------------------------------------------------------------
// Gui_GetWindowLongPtrA
//---------------------------------------------------------------------------


_FX ULONG_PTR Gui_GetWindowLongPtrA(HWND hWnd, int nIndex)
{
    ULONG_PTR value;
    Gui_InitPropAtoms();
    //value = (ULONG_PTR)__sys_GetWindowLongPtrA(hWnd, nIndex);
    value = (ULONG_PTR)Gui_GetLongCommon(hWnd, nIndex, 'wlpa');

    if (Gui_RenameClasses && (nIndex == GWLP_WNDPROC)) {

        //
        // prevent caller from seeing our window procedure
        //

        WNDPROC w = __sys_GetPropW(hWnd, (LPCWSTR)Gui_WindowProcNewW_Atom);
        WNDPROC a = __sys_GetPropW(hWnd, (LPCWSTR)Gui_WindowProcNewA_Atom);

        if ((w && value == (ULONG_PTR)w) || (a && value == (ULONG_PTR)a)) {

            value = (ULONG_PTR)__sys_GetPropW(
                                    hWnd, (LPCWSTR)Gui_WindowProcOldA_Atom);
        }
    }

    return Gui_GetWindowLong_DragDrop(hWnd, nIndex, value);
}


//---------------------------------------------------------------------------
// Gui_SetWindowLongPtrW
//---------------------------------------------------------------------------


_FX ULONG_PTR Gui_SetWindowLongPtrW(HWND hWnd, int nIndex, ULONG_PTR dwNew)
{
    ULONG_PTR value;
    Gui_InitPropAtoms();
    value = __sys_SetWindowLongPtrW(hWnd, nIndex, dwNew);

    if (Gui_RenameClasses && (nIndex == GWLP_WNDPROC)) {

        //
        // replace window procedure, then override the new procedure
        //

        WNDPROC w = __sys_GetPropW(hWnd, (LPCWSTR)Gui_WindowProcNewW_Atom);
        WNDPROC a = __sys_GetPropW(hWnd, (LPCWSTR)Gui_WindowProcNewA_Atom);

        if ((w && value == (ULONG_PTR)w) || (a && value == (ULONG_PTR)a)) {

            value = (ULONG_PTR)__sys_GetPropW(
                                    hWnd, (LPCWSTR)Gui_WindowProcOldW_Atom);

            Gui_SetWindowProc(hWnd, TRUE);
        }
    }

    return Gui_SetWindowLong_DragDrop(hWnd, nIndex, dwNew, value);
}


//---------------------------------------------------------------------------
// Gui_SetWindowLongPtrA
//---------------------------------------------------------------------------


_FX ULONG_PTR Gui_SetWindowLongPtrA(HWND hWnd, int nIndex, ULONG_PTR dwNew)
{
    ULONG_PTR value;
    Gui_InitPropAtoms();
    value = __sys_SetWindowLongPtrA(hWnd, nIndex, dwNew);

    if (Gui_RenameClasses && (nIndex == GWLP_WNDPROC)) {

        //
        // replace window procedure, then override the new procedure
        //

        WNDPROC w = __sys_GetPropW(hWnd, (LPCWSTR)Gui_WindowProcNewW_Atom);
        WNDPROC a = __sys_GetPropW(hWnd, (LPCWSTR)Gui_WindowProcNewA_Atom);

        if ((w && value == (ULONG_PTR)w) || (a && value == (ULONG_PTR)a)) {

            value = (ULONG_PTR)__sys_GetPropW(
                                    hWnd, (LPCWSTR)Gui_WindowProcOldA_Atom);

            Gui_SetWindowProc(hWnd, TRUE);
        }
    }

    return Gui_SetWindowLong_DragDrop(hWnd, nIndex, dwNew, value);
}


//---------------------------------------------------------------------------
// Gui_GetClassLongPtrW
//---------------------------------------------------------------------------


_FX ULONG_PTR Gui_GetClassLongPtrW(HWND hWnd, int nIndex)
{
    return Gui_GetLongCommon(hWnd, nIndex, 'clpw');
}


//---------------------------------------------------------------------------
// Gui_GetClassLongPtrA
//---------------------------------------------------------------------------


_FX ULONG_PTR Gui_GetClassLongPtrA(HWND hWnd, int nIndex)
{
    return Gui_GetLongCommon(hWnd, nIndex, 'clpa');
}


//---------------------------------------------------------------------------
// Gui_SetWindowLong8
//---------------------------------------------------------------------------


_FX ULONG_PTR Gui_SetWindowLong8(
    HWND hWnd, int nIndex, ULONG_PTR dwNew, ULONG IsAscii)
{
    ULONG_PTR value;
    Gui_InitPropAtoms();
    value = __sys_SetWindowLong8(hWnd, nIndex, dwNew, IsAscii);

    if (Gui_RenameClasses && (nIndex == GWLP_WNDPROC)) {

        //
        // replace window procedure, then override the new procedure
        //

        WNDPROC w = __sys_GetPropW(hWnd, (LPCWSTR)Gui_WindowProcNewW_Atom);
        WNDPROC a = __sys_GetPropW(hWnd, (LPCWSTR)Gui_WindowProcNewA_Atom);

        if ((w && value == (ULONG_PTR)w) || (a && value == (ULONG_PTR)a)) {

            ATOM atom = IsAscii ? Gui_WindowProcOldA_Atom
                                : Gui_WindowProcOldW_Atom;
            value = (ULONG_PTR)__sys_GetPropW(hWnd, (LPCWSTR)atom);

            Gui_SetWindowProc(hWnd, TRUE);
        }
    }

    return Gui_SetWindowLong_DragDrop(hWnd, nIndex, dwNew, value);
}


//---------------------------------------------------------------------------
// Gui_Hook_SetWindowLong8
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_Hook_SetWindowLong8(void)
{
    //
    // on Windows 8.1, the SetWindowLongA and SetWindowLongW functions
    // are very short stubs that invoke user32!SetWindowLong.
    // the hook trampoline may override past the end of the short function,
    // and corrupt surrounding code.  to work around this, we analyze the
    // short functions in order to hook user32!SetWindowLong instead
    //
    // SetWindowLongA       mov r9d,1           41 B9 01 00 00 00
    //      (11 bytes)      jmp xxx             E9 xx xx xx xx
    // SetWindowLongW       xor r9d,r9d         45 33 C9
    //      (8 bytes)       jmp xxx             E9 xx xx xx xx
    //

    UCHAR *a = (UCHAR *)__sys_SetWindowLongA;
    UCHAR *w = (UCHAR *)__sys_SetWindowLongW;

    if (Dll_OsBuild >= 14942) {

        HMODULE hWin32u;

        hWin32u = GetModuleHandleA("win32u.dll");
        __sys_SetWindowLong8 = (P_SetWindowLong8) GetProcAddress(hWin32u, "NtUserSetWindowLong");
        SBIEDLL_HOOK_GUI(SetWindowLong8);
        return TRUE;
    }

    if (*(ULONG *)a == 0x0001B941 && *(ULONG *)w == 0xE9C93345) {

        LONG a_offset = *(LONG *)(a + 7);
        LONG w_offset = *(LONG *)(w + 4);

        if ((a + 11 + a_offset) == (w + 8 + w_offset)) {

            __sys_SetWindowLong8 = (P_SetWindowLong8)(w + 8 + w_offset);

            SBIEDLL_HOOK_GUI(SetWindowLong8);

            return TRUE;
        }
    }

    //windows 10 build 10147 no longer has the wrapper for SetWindowLongW.
    //SetWindowLongA remains the same as the previous version but points to a
    //separate function 
    else if (*(ULONG *)a == 0x0001B941) {
        LONG a_offset = *(LONG *)(a + 7);
        __sys_SetWindowLong8 = (P_SetWindowLong8)w;
        if ((a + 11 + a_offset) !=  w) {
            __sys_SetWindowLongA =  (P_SetWindowLong)(a + 11 + a_offset);
        }
        return TRUE;
    }
    
    SbieApi_Log(2205, L"SetWindowLong8");
    return FALSE;
}


//---------------------------------------------------------------------------
// Gui_SetWindowLongPtr8
//---------------------------------------------------------------------------


_FX ULONG_PTR Gui_SetWindowLongPtr8(
    HWND hWnd, int nIndex, ULONG_PTR dwNew, ULONG IsAscii)
{
    ULONG_PTR value;
    Gui_InitPropAtoms();
    value = __sys_SetWindowLongPtr8(hWnd, nIndex, dwNew, IsAscii);

    if (Gui_RenameClasses && (nIndex == GWLP_WNDPROC)) {

        //
        // replace window procedure, then override the new procedure
        //

        WNDPROC w = __sys_GetPropW(hWnd, (LPCWSTR)Gui_WindowProcNewW_Atom);
        WNDPROC a = __sys_GetPropW(hWnd, (LPCWSTR)Gui_WindowProcNewA_Atom);

        if ((w && value == (ULONG_PTR)w) || (a && value == (ULONG_PTR)a)) {

            ATOM atom = IsAscii ? Gui_WindowProcOldA_Atom
                                : Gui_WindowProcOldW_Atom;
            value = (ULONG_PTR)__sys_GetPropW(hWnd, (LPCWSTR)atom);

            Gui_SetWindowProc(hWnd, TRUE);
        }
    }

    return Gui_SetWindowLong_DragDrop(hWnd, nIndex, dwNew, value);
}


//---------------------------------------------------------------------------
// Gui_Hook_SetWindowLongPtr8
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_Hook_SetWindowLongPtr8(void)
{
    //
    // on Windows 8, the SetWindowLongPtrA and SetWindowLongPtrW functions
    // are very short stubs that invoke user32!SetWindowLongPtr.
    // the hook trampoline may override past the end of the short function,
    // and corrupt surrounding code.  to work around this, we analyze the
    // short functions in order to hook user32!SetWindowLongPtr instead
    //
    // SetWindowLongPtrA    mov r9d,1           41 B9 01 00 00 00
    //      (11 bytes)      jmp xxx             E9 xx xx xx xx
    // SetWindowLongPtrW    xor r9d,r9d         45 33 C9
    //      (8 bytes)       jmp xxx             E9 xx xx xx xx
    //

    UCHAR *a = (UCHAR *)__sys_SetWindowLongPtrA;
    UCHAR *w = (UCHAR *)__sys_SetWindowLongPtrW;

    if (Dll_OsBuild >= 14942) {
        HMODULE hWin32u;
        hWin32u = GetModuleHandleA("win32u.dll");
        __sys_SetWindowLongPtr8 = (P_SetWindowLongPtr8) GetProcAddress(hWin32u,"NtUserSetWindowLongPtr");
        SBIEDLL_HOOK_GUI(SetWindowLongPtr8);
        return TRUE;
    }

    if (*(ULONG *)a == 0x0001B941 && *(ULONG *)w == 0xE9C93345) {

        LONG a_offset = *(LONG *)(a + 7);
        LONG w_offset = *(LONG *)(w + 4);

        if ((a + 11 + a_offset) == (w + 8 + w_offset)) {

            __sys_SetWindowLongPtr8 =
                                (P_SetWindowLongPtr8)(w + 8 + w_offset);

            SBIEDLL_HOOK_GUI(SetWindowLongPtr8);

            return TRUE;
        }
    }

    SbieApi_Log(2205, L"SetWindowLongPtr8");
    return FALSE;
}


//---------------------------------------------------------------------------
// End 64-bit Get/SetWindowLongPtr functions
//---------------------------------------------------------------------------


#endif _WIN64


//---------------------------------------------------------------------------
// Gui_SetWindowProc
//---------------------------------------------------------------------------


_FX void Gui_SetWindowProc(HWND hWnd, BOOLEAN force)
{
    void *pWindowProcW;
    void *pWindowProcA;

    //
    // ignore windows that are not top level windows, unless forced
    //
    // note that SetWindowLongXxx functions specify the force parameter
    // when they re-apply our wndproc after the caller program requested
    // to replace our wndproc
    //

    if ((! force) && __sys_GetParent(hWnd))
        return;

    Gui_InitPropAtoms();

    //
    // replace window procedure while recording the results of
    // GetWindowLong A+W before and after the replace
    //

    pWindowProcW = (void *)__sys_GetWindowLongPtrW(hWnd, GWLP_WNDPROC);
    pWindowProcA = (void *)__sys_GetWindowLongPtrA(hWnd, GWLP_WNDPROC);

    if (pWindowProcW == Gui_WindowProcW || pWindowProcA == Gui_WindowProcA)
        return;

    __sys_SetPropW(hWnd, (LPCWSTR)Gui_WindowProcOldW_Atom, pWindowProcW);
    __sys_SetPropW(hWnd, (LPCWSTR)Gui_WindowProcOldA_Atom, pWindowProcA);

    if (__sys_IsWindowUnicode(hWnd)) {
        __sys_SetWindowLongPtrW(
                        hWnd, GWLP_WNDPROC, (ULONG_PTR)Gui_WindowProcW);
    } else {
        __sys_SetWindowLongPtrA(
                        hWnd, GWLP_WNDPROC, (ULONG_PTR)Gui_WindowProcA);
    }

    pWindowProcW = (void *)__sys_GetWindowLongPtrW(hWnd, GWLP_WNDPROC);
    pWindowProcA = (void *)__sys_GetWindowLongPtrA(hWnd, GWLP_WNDPROC);

    __sys_SetPropW(hWnd, (LPCWSTR)Gui_WindowProcNewW_Atom, pWindowProcW);
    __sys_SetPropW(hWnd, (LPCWSTR)Gui_WindowProcNewA_Atom, pWindowProcA);
}
