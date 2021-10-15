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
// GUI Dialog Template Functions
//---------------------------------------------------------------------------

#include "dll.h"

#include "gui_p.h"
#include "taskbar.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define SCDLG_UNICODE 0
#define SCDLG_ANSI 2


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static HWND Gui_CreateDialogParamA(
    HINSTANCE hInstance,
    void *lpTemplateName,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam);

static HWND Gui_CreateDialogParamW(
    HINSTANCE hInstance,
    void *lpTemplateName,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam);

static HWND Gui_CreateDialogIndirectParamA(
    HINSTANCE hInstance,
    void *lpTemplate,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam);

static HWND Gui_CreateDialogIndirectParamW(
    HINSTANCE hInstance,
    void *lpTemplate,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam);

static HWND Gui_CreateDialogIndirectParamAorW(
    HINSTANCE hInstance,
    void *lpTemplate,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam,
    UINT fAnsiFlag);

static INT_PTR Gui_DialogBoxParamA(
    HINSTANCE hInstance,
    void *lpTemplateName,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam);

static INT_PTR Gui_DialogBoxParamW(
    HINSTANCE hInstance,
    void *lpTemplateName,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam);

static INT_PTR Gui_DialogBoxIndirectParamA(
    HINSTANCE hInstance,
    void *lpTemplate,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam);

static INT_PTR Gui_DialogBoxIndirectParamW(
    HINSTANCE hInstance,
    void *lpTemplate,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam);

static INT_PTR Gui_DialogBoxIndirectParamAorW(
    HINSTANCE hInstance,
    void *lpTemplate,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam,
    UINT fAnsiFlag);

static void *Gui_CreateDialogTemplate(WORD *tmpl);

static void *Gui_FindDialogTemplate(
    HINSTANCE hInstance, void *lpTemplateName, BOOLEAN IsUnicode);

static DLGPROC Gui_MyDialogProc1(DLGPROC OrigDlgProc, UINT fAnsiFlag);

static LRESULT Gui_MyDialogProc2(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


//---------------------------------------------------------------------------
// Gui_InitDlgTmpl
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_InitDlgTmpl(void)
{
    if (! Gui_RenameClasses)
        return TRUE;

    SBIEDLL_HOOK_GUI(CreateDialogParamA);
    SBIEDLL_HOOK_GUI(CreateDialogParamW);
    SBIEDLL_HOOK_GUI(CreateDialogIndirectParamA);
    SBIEDLL_HOOK_GUI(CreateDialogIndirectParamW);
    SBIEDLL_HOOK_GUI(CreateDialogIndirectParamAorW);

    SBIEDLL_HOOK_GUI(DialogBoxParamA);
    SBIEDLL_HOOK_GUI(DialogBoxParamW);
    SBIEDLL_HOOK_GUI(DialogBoxIndirectParamA);
    SBIEDLL_HOOK_GUI(DialogBoxIndirectParamW);
    SBIEDLL_HOOK_GUI(DialogBoxIndirectParamAorW);

    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_CreateDialogParamW
//---------------------------------------------------------------------------


_FX HWND Gui_CreateDialogParamW(
    HINSTANCE hInstance,
    void *lpTemplateName,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam)
{
    HWND rv;
    void *tmpl = Gui_FindDialogTemplate(hInstance, lpTemplateName, TRUE);
    if (tmpl)
        rv = Gui_CreateDialogIndirectParamW(
            hInstance, tmpl, hWndParent, lpDialogFunc, dwInitParam);
    else
        rv = NULL;
    return rv;
}


//---------------------------------------------------------------------------
// Gui_CreateDialogParamA
//---------------------------------------------------------------------------


_FX HWND Gui_CreateDialogParamA(
    HINSTANCE hInstance,
    void *lpTemplateName,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam)
{
    HWND rv;
    void *tmpl = Gui_FindDialogTemplate(hInstance, lpTemplateName, FALSE);
    if (tmpl)
        rv = Gui_CreateDialogIndirectParamA(
            hInstance, tmpl, hWndParent, lpDialogFunc, dwInitParam);
    else
        rv = NULL;
    return rv;
}


//---------------------------------------------------------------------------
// Gui_CreateDialogIndirectParamW
//---------------------------------------------------------------------------


_FX HWND Gui_CreateDialogIndirectParamW(
    HINSTANCE hInstance,
    void *lpTemplate,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam)
{
    return Gui_CreateDialogIndirectParamAorW(
        hInstance, lpTemplate, hWndParent, lpDialogFunc, dwInitParam,
        SCDLG_UNICODE);
}

//---------------------------------------------------------------------------
// Gui_CreateDialogIndirectParamW
//---------------------------------------------------------------------------


_FX HWND Gui_CreateDialogIndirectParamA(
    HINSTANCE hInstance,
    void *lpTemplate,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam)
{
    return Gui_CreateDialogIndirectParamAorW(
        hInstance, lpTemplate, hWndParent, lpDialogFunc, dwInitParam,
        SCDLG_ANSI);
}


//---------------------------------------------------------------------------
// Gui_CreateDialogIndirectParamAorW
//---------------------------------------------------------------------------


_FX HWND Gui_CreateDialogIndirectParamAorW(
    HINSTANCE hInstance,
    void *lpTemplate,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam,
    UINT fAnsiFlag)
{
    HWND hwnd;

    void *lpTemplate2 = Gui_CreateDialogTemplate(lpTemplate);
    if (lpTemplate2)
        lpTemplate = lpTemplate2;

    hwnd = __sys_CreateDialogIndirectParamAorW(
        hInstance, lpTemplate, hWndParent,
        Gui_MyDialogProc1(lpDialogFunc, fAnsiFlag), dwInitParam,
        fAnsiFlag);

    if (hwnd)
        Gui_SetWindowProc(hwnd, TRUE);

    if (lpTemplate2)
        Gui_Free(lpTemplate2);

    return hwnd;
}


//---------------------------------------------------------------------------
// Gui_DialogBoxParamW
//---------------------------------------------------------------------------


_FX INT_PTR Gui_DialogBoxParamW(
    HINSTANCE hInstance,
    void *lpTemplateName,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam)
{
    INT_PTR rv;
    void *tmpl = Gui_FindDialogTemplate(hInstance, lpTemplateName, TRUE);
    if (tmpl)
        rv = Gui_DialogBoxIndirectParamW(
            hInstance, tmpl, hWndParent, lpDialogFunc, dwInitParam);
    else
        rv = 0;
    return rv;
}


//---------------------------------------------------------------------------
// Gui_DialogBoxParamA
//---------------------------------------------------------------------------


_FX INT_PTR Gui_DialogBoxParamA(
    HINSTANCE hInstance,
    void *lpTemplateName,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam)
{
    INT_PTR rv;
    void *tmpl = Gui_FindDialogTemplate(hInstance, lpTemplateName, FALSE);
    if (tmpl)
        rv = Gui_DialogBoxIndirectParamA(
            hInstance, tmpl, hWndParent, lpDialogFunc, dwInitParam);
    else
        rv = 0;
    return rv;
}


//---------------------------------------------------------------------------
// Gui_DialogBoxIndirectParamW
//---------------------------------------------------------------------------


_FX INT_PTR Gui_DialogBoxIndirectParamW(
    HINSTANCE hInstance,
    void *lpTemplate,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam)
{
    return Gui_DialogBoxIndirectParamAorW(
        hInstance, lpTemplate, hWndParent, lpDialogFunc, dwInitParam,
        SCDLG_UNICODE);
}


//---------------------------------------------------------------------------
// Gui_DialogBoxIndirectParamA
//---------------------------------------------------------------------------


_FX INT_PTR Gui_DialogBoxIndirectParamA(
    HINSTANCE hInstance,
    void *lpTemplate,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam)
{
    return Gui_DialogBoxIndirectParamAorW(
        hInstance, lpTemplate, hWndParent, lpDialogFunc, dwInitParam,
        SCDLG_ANSI);
}


//---------------------------------------------------------------------------
// Gui_DialogBoxIndirectParamAorW
//---------------------------------------------------------------------------


_FX INT_PTR Gui_DialogBoxIndirectParamAorW(
    HINSTANCE hInstance,
    void *lpTemplate,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam,
    UINT fAnsiFlag)
{
    INT_PTR rv;

    void *lpTemplate2 = Gui_CreateDialogTemplate(lpTemplate);
    if (lpTemplate2)
        lpTemplate = lpTemplate2;

    rv = __sys_DialogBoxIndirectParamAorW(
        hInstance, lpTemplate, hWndParent,
        Gui_MyDialogProc1(lpDialogFunc, fAnsiFlag), dwInitParam,
        fAnsiFlag);

    if (lpTemplate2)
        Gui_Free(lpTemplate2);

    return rv;
}


//---------------------------------------------------------------------------
// Gui_CreateDialogTemplate
//---------------------------------------------------------------------------


#define GUI_DLG_VER 1
#include "guidlg.h"
#define GUI_DLG_VER 2
#include "guidlg.h"


_FX void *Gui_CreateDialogTemplate(WORD *tmpl)
{
    WORD *dlgver = &tmpl[0];
    WORD *signature = &tmpl[1];

    // select function according to DLGTEMPLATE/DLGTEMPLATEEX
    if (*dlgver == 1 && *signature == 0xFFFF)
        return GUI_CreateDialogTemplate2((void *)tmpl);
    else
        return GUI_CreateDialogTemplate1((void *)tmpl);
}


//---------------------------------------------------------------------------
// Gui_FindDialogTemplate
//---------------------------------------------------------------------------


_FX void *Gui_FindDialogTemplate(
    HINSTANCE hInstance, void *lpTemplateName, BOOLEAN IsUnicode)
{
    void *restype;
    HRSRC hrsrc;
    HGLOBAL hglobal;
    void *tmpl = NULL;

    if (IsUnicode) {
        restype = MAKEINTRESOURCEW(5);  // RT_DIALOG
        hrsrc = FindResourceW(hInstance, lpTemplateName, restype);
    } else {
        restype = MAKEINTRESOURCEA(5);  // RT_DIALOG
        hrsrc = FindResourceA(hInstance, lpTemplateName, restype);
    }
    if (hrsrc) {
        hglobal = LoadResource(hInstance, hrsrc);
        if (hglobal)
            tmpl = LockResource(hglobal);
    }
    return tmpl;
}


//---------------------------------------------------------------------------
// Gui_MyDialogProc1
//---------------------------------------------------------------------------


_FX DLGPROC Gui_MyDialogProc1(DLGPROC OrigDlgProc, UINT fAnsiFlag)
{
    return (DLGPROC)Dll_JumpStub(OrigDlgProc, Gui_MyDialogProc2, fAnsiFlag);
}


//---------------------------------------------------------------------------
// Gui_MyDialogProc2
//---------------------------------------------------------------------------


_FX LRESULT Gui_MyDialogProc2(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    ULONG_PTR *StubData = Dll_JumpStubData();
    LRESULT lResult;

    //
    // this dialog procedure is invoked from the stub created by
    // Gui_MyDialogProc1.  the stub jumps to this procedure.  this will
    // happen just once, as we will restore the original dialog procedure
    // (stored in StubData[1]) before returning.  we will also free the
    // stub itself (stored in StubData[0]).
    //
    // the purpose of this dialog procedure is only to install our
    // _window_ procedure (not dialog procedure) into the dialog window.
    //

    Gui_SetWindowProc(hWnd, TRUE);

    //
    // if a dialog box is the main window of an application,
    // we need to adjust the shell data for the taskbar button
    //

    if (! __sys_GetParent(hWnd)) {

        Taskbar_SetWindowAppUserModelId(hWnd);
    }

    //
    // restore original dialog procedure and invoke it before returning.
    // StubData[2] tells us if the dialog procedure is ANSI or UNICODE
    //

    if (StubData[2] != SCDLG_ANSI)
        __sys_SetWindowLongPtrW(hWnd, DWLP_DLGPROC, StubData[1]);
    else
        __sys_SetWindowLongPtrA(hWnd, DWLP_DLGPROC, StubData[1]);

    if (StubData[1])
        lResult = ProtectCall4(
                (void *)StubData[1], (ULONG_PTR)hWnd, uMsg, wParam, lParam);
    else
        lResult = 0;

    Dll_FreeCode128((void *)StubData[0]);

    return lResult;
}
