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
#include "core/svc/ComWire.h"
#include "common/my_version.h"
#include <stdio.h>


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define SB_SETTEXTW         (WM_USER+11)
#define SBT_OWNERDRAW       0x1000


//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------


typedef struct _GUI_ENUM_PROC_PARM {

    ULONG which;
    UINT uMsg;
    WPARAM wParam;
    LPARAM lParam;
    P_SendMessage __sys_SendOrPostMessageX;
    P_SendMessageTimeout __sys_SendMessageTimeoutX;
    UINT fuFlags;
    UINT uTimeout;
    PDWORD_PTR lpdwResult;

} GUI_ENUM_PROC_PARM;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static LRESULT Gui_SendMessageA(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static LRESULT Gui_SendMessageW(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static LRESULT Gui_SendMessageTimeoutA(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT fuFlags, UINT uTimeout, PDWORD_PTR lpdwResult);

static LRESULT Gui_SendMessageTimeoutW(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT fuFlags, UINT uTimeout, PDWORD_PTR lpdwResult);

static LRESULT Gui_SendNotifyMessageA(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static LRESULT Gui_SendNotifyMessageW(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static LRESULT Gui_PostMessageA(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static LRESULT Gui_PostMessageW(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static LRESULT Gui_SendPostMessageCommon(
    ULONG which, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT fuFlags, UINT uTimeout, PDWORD_PTR lpdwResult);

static LRESULT Gui_SendPostMessageMulti(
    ULONG which, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT fuFlags, UINT uTimeout, PDWORD_PTR lpdwResult);

static BOOL Gui_SendPostMessageMultiCallback(HWND hWnd, LPARAM lParam);

static LRESULT Gui_SendCopyData(
    ULONG which, HWND hWnd, WPARAM wParam, LPARAM lParam,
    UINT fuFlags, UINT uTimeout, PDWORD_PTR lpdwResult);

static BOOL Gui_PostThreadMessageA(
    ULONG idThread, UINT uMsg, WPARAM wParam, LPARAM lParam);

static BOOL Gui_PostThreadMessageW(
    ULONG idThread, UINT uMsg, WPARAM wParam, LPARAM lParam);

static BOOLEAN Gui_PostThreadMessage_Check(ULONG idThread, UINT uMsg);

static LRESULT Gui_DispatchMessageA(const MSG *lpmsg);

static LRESULT Gui_DispatchMessageW(const MSG *lpmsg);

static BOOLEAN Gui_DoStatusBarMessage(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plResult);

static int Gui_LoadStringW(
    HINSTANCE hInstance, UINT uID, void *lpBuffer, int nBufferMax);

static LRESULT Gui_SendMessageW_MdiCreate(HWND hWnd, LPARAM lParam);

static LRESULT Gui_SendMessageA_MdiCreate(HWND hWnd, LPARAM lParam);


//---------------------------------------------------------------------------


#ifdef _WIN64

static LRESULT Gui_DispatchMessage8(const MSG *lpmsg, ULONG IsAscii);

#ifndef _M_ARM64
static BOOLEAN Gui_Hook_DispatchMessage8(HMODULE module);
#endif

static P_DispatchMessage8          __sys_DispatchMessage8       = 0;

#endif _WIN64


//---------------------------------------------------------------------------


typedef ULONG (*P_GetProcessIdOfThread)(HANDLE Thread);

static P_GetProcessIdOfThread       __sys_GetProcessIdOfThread  = NULL;


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


BOOLEAN Gui_DispatchMessageCalled = FALSE;


//---------------------------------------------------------------------------
// Gui_InitMsg
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_InitMsg(HMODULE module)
{
    //
    // hook SendMessage and PostMessage family of functions
    //

    if (! Gui_OpenAllWinClasses) {

        SBIEDLL_HOOK_GUI(SendMessageA);
        SBIEDLL_HOOK_GUI(SendMessageW);

        SBIEDLL_HOOK_GUI(SendMessageTimeoutA);
        SBIEDLL_HOOK_GUI(SendMessageTimeoutW);

        SBIEDLL_HOOK_GUI(SendNotifyMessageA);
        SBIEDLL_HOOK_GUI(SendNotifyMessageW);

        SBIEDLL_HOOK_GUI(PostMessageA);
        SBIEDLL_HOOK_GUI(PostMessageW);

        //
        // hook PostThreadMessage on Vista, see Gui_PostThreadMessage_Check
        //

        if (Dll_OsBuild >= 6000) {

            __sys_GetProcessIdOfThread = (P_GetProcessIdOfThread)
                GetProcAddress(Dll_Kernel32, "GetProcessIdOfThread");
            if (__sys_GetProcessIdOfThread) {

                SBIEDLL_HOOK_GUI(PostThreadMessageA);
                SBIEDLL_HOOK_GUI(PostThreadMessageW);
            }
        }
    }

    //
    // DispatchMessage hook requires special handling on 64-bit Windows 8
    //

#ifndef _WIN64

    SBIEDLL_HOOK_GUI(DispatchMessageA);
    SBIEDLL_HOOK_GUI(DispatchMessageW);

#else _WIN64

    if (Dll_OsBuild >= 14942) { // Windows 10 1703 preview #7
        HMODULE hWin32u = GetModuleHandleA("win32u.dll");
        __sys_DispatchMessage8 = (P_DispatchMessage8)GetProcAddress(hWin32u, "NtUserDispatchMessage");
        SBIEDLL_HOOK_GUI(DispatchMessage8);
    }
    else if (Dll_OsBuild < 8400) {
        SBIEDLL_HOOK_GUI(DispatchMessageA);
        SBIEDLL_HOOK_GUI(DispatchMessageW);
    }
    else 
#ifndef _M_ARM64
    if (!Gui_Hook_DispatchMessage8(module))
#endif
    {
        SbieApi_Log(2205, L"DispatchMessage8");
        return FALSE;
    }
   

#endif _WIN64

    //
    // hook LoadString for Internet Explorer
    //

    if (SH_GetInternetExplorerVersion() >= 9) {
        SBIEDLL_HOOK_GUI(LoadStringW);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_SendMessageA
//---------------------------------------------------------------------------


_FX LRESULT Gui_SendMessageA(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    if ((ULONG_PTR)hWnd != XFF4 && (ULONG_PTR)hWnd != XFF8) {

        if (uMsg == WM_MDICREATE && Dll_OsBuild >= 8400)
            return Gui_SendMessageA_MdiCreate(hWnd, lParam);

        if (uMsg == WM_DDE_ACK)
            hWnd = Gui_DDE_ACK_Sending(hWnd, wParam);
    }

    lResult = Gui_SendPostMessageCommon(
                        'sm a', hWnd, uMsg, wParam, lParam, 0, 0, NULL);

    if ((ULONG_PTR)hWnd != XFF4 && (ULONG_PTR)hWnd != XFF8) {

        if (uMsg == WM_GETTEXT)
            lResult = Gui_FixTitleA(hWnd, (UCHAR *)lParam, (int)lResult);
    }

    return lResult;
}


//---------------------------------------------------------------------------
// Gui_SendMessageW
//---------------------------------------------------------------------------


_FX LRESULT Gui_SendMessageW(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    if ((ULONG_PTR)hWnd != XFF4 && (ULONG_PTR)hWnd != XFF8) {

        if (uMsg == SB_SETTEXTW) {
            BOOL handled =
                Gui_DoStatusBarMessage(hWnd, uMsg, wParam, lParam, &lResult);
            if (handled)
                return lResult;
        }

        if (uMsg == WM_MDICREATE && Dll_OsBuild >= 8400)
            return Gui_SendMessageW_MdiCreate(hWnd, lParam);

        if (uMsg == WM_DDE_ACK)
            hWnd = Gui_DDE_ACK_Sending(hWnd, wParam);
    }

    lResult = Gui_SendPostMessageCommon(
                        'sm w', hWnd, uMsg, wParam, lParam, 0, 0, NULL);

    if ((ULONG_PTR)hWnd != XFF4 && (ULONG_PTR)hWnd != XFF8) {

        if (uMsg == WM_GETTEXT)
            lResult = Gui_FixTitleW(hWnd, (WCHAR *)lParam, (int)lResult);
    }

    return lResult;
}


//---------------------------------------------------------------------------
// Gui_SendMessageTimeoutA
//---------------------------------------------------------------------------


_FX LRESULT Gui_SendMessageTimeoutA(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT fuFlags, UINT uTimeout, PDWORD_PTR lpdwResult)
{
    LRESULT lResult;

    lResult = Gui_SendPostMessageCommon(
        'smta', hWnd, uMsg, wParam, lParam, fuFlags, uTimeout, lpdwResult);

    return lResult;
}


//---------------------------------------------------------------------------
// Gui_SendMessageTimeoutW
//---------------------------------------------------------------------------


_FX LRESULT Gui_SendMessageTimeoutW(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT fuFlags, UINT uTimeout, PDWORD_PTR lpdwResult)
{
    LRESULT lResult;

    lResult = Gui_SendPostMessageCommon(
        'smtw', hWnd, uMsg, wParam, lParam, fuFlags, uTimeout, lpdwResult);

    if ((ULONG_PTR)hWnd != XFF4 && (ULONG_PTR)hWnd != XFF8) {

        if ((! lResult)
                && Dll_ImageType == DLL_IMAGE_SHELL_EXPLORER
                && uMsg == WM_NULL && wParam == 0 && lParam == 0
                && (fuFlags & SMTO_ABORTIFHUNG)) {

            //
            // special case:  Windows Explorer sends a WM_NULL message to
            // the window of the owner of the clipboard data, in order
            // to test if the window is not hung, and display or hide
            // the Paste menu command accordingly.  make sure the
            // command is displayed even for windows outside the sandbox
            //

            if (hWnd == __sys_GetClipboardOwner()) {

                *lpdwResult = 0;
                lResult = 1;
            }
        }
    }

    return lResult;
}


//---------------------------------------------------------------------------
// Gui_SendNotifyMessageA
//---------------------------------------------------------------------------


_FX LRESULT Gui_SendNotifyMessageA(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return Gui_SendPostMessageCommon(
                    'snma', hWnd, uMsg, wParam, lParam, 0, 0, NULL);
}


//---------------------------------------------------------------------------
// Gui_SendNotifyMessageW
//---------------------------------------------------------------------------


_FX LRESULT Gui_SendNotifyMessageW(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return Gui_SendPostMessageCommon(
                    'snmw', hWnd, uMsg, wParam, lParam, 0, 0, NULL);
}


//---------------------------------------------------------------------------
// Gui_PostMessageA
//---------------------------------------------------------------------------


_FX LRESULT Gui_PostMessageA(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    if (Gui_UseProxyService && uMsg == WM_DDE_DATA)
        lResult = Gui_DDE_DATA_Posting(hWnd, wParam, lParam);
    else {

        lResult = Gui_SendPostMessageCommon(
                        'pm a', hWnd, uMsg, wParam, lParam, 0, 0, NULL);
    }

    if (Gui_UseProxyService && (ULONG_PTR)hWnd != XFF4 && (ULONG_PTR)hWnd != XFF8) {

        //
        // for some messages, we have to pretend the post was successful,
        // otherwise some applications react badly
        //

        if ((! lResult) && (uMsg == WM_DROPFILES ||
                            uMsg == WM_DDE_ACK   ||
                            uMsg == WM_DDE_TERMINATE)) {

            SetLastError(0);
            lResult = 1;
        }
    }

    return lResult;
}


//---------------------------------------------------------------------------
// Gui_PostMessageW
//---------------------------------------------------------------------------


_FX LRESULT Gui_PostMessageW(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult;

    if (Gui_UseProxyService && uMsg == WM_DDE_DATA)
        lResult = Gui_DDE_DATA_Posting(hWnd, wParam, lParam);
    else {

        lResult = Gui_SendPostMessageCommon(
                        'pm w', hWnd, uMsg, wParam, lParam, 0, 0, NULL);
    }

    if (Gui_UseProxyService && (ULONG_PTR)hWnd != XFF4 && (ULONG_PTR)hWnd != XFF8) {

        //
        // for some messages, we have to pretend the post was successful,
        // otherwise some applications react badly
        //

        if ((! lResult) && (uMsg == WM_DROPFILES ||
                            uMsg == WM_DDE_ACK   ||
                            uMsg == WM_DDE_TERMINATE)) {

            SetLastError(0);
            lResult = 1;
        }
    }

    return lResult;
}


//---------------------------------------------------------------------------
// Gui_SendPostMessageCommon
//---------------------------------------------------------------------------


_FX LRESULT Gui_SendPostMessageCommon(
    ULONG which, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT fuFlags, UINT uTimeout, PDWORD_PTR lpdwResult)
{
    GUI_SEND_POST_MESSAGE_REQ req;
    GUI_SEND_POST_MESSAGE_RPL *rpl;
    LRESULT result;
    ULONG error;

    /*if (uMsg >= WM_DDE_FIRST && uMsg <= WM_DDE_LAST) {
        WCHAR txt[128];
        UCHAR *ptr = (UCHAR *)&which;
        Sbie_snwprintf(txt, 128, L"Doing (%c%c%c%c) MSG=%04X to HWND=%08X from WPARAM=%08X\n",
                ptr[3],ptr[2],ptr[1],ptr[0], uMsg, hWnd, wParam);
        OutputDebugString(txt);
    }*/

    //
    // if this is a broadcast message, then we need to enumerate windows
    //

    if ((ULONG_PTR)hWnd == XFF4 || (ULONG_PTR)hWnd == XFF8) {

        return Gui_SendPostMessageMulti(which, hWnd, uMsg, wParam, lParam,
                                        fuFlags, uTimeout, lpdwResult);
    }

    //
    // if target window is in the same sandbox (i.e. within the
    // boundary of the job object) then issue a direct call
    //

    if ((! hWnd) || !Gui_UseProxyService || Gui_IsSameBox(hWnd, NULL, NULL)) {

        if (hWnd && (which == 'pm w' || which == 'pm a')
                 && (uMsg >= WM_DDE_FIRST && uMsg <= WM_DDE_LAST)) {

            if (! Gui_DDE_Post_In_Box(hWnd, uMsg, wParam, lParam))
                return 0;
            return 1;

        } else if (which == 'pm w')
            return __sys_PostMessageW(hWnd, uMsg, wParam, lParam);
        else if (which == 'pm a')
            return __sys_PostMessageA(hWnd, uMsg, wParam, lParam);

        else if (which == 'sm w')
            return __sys_SendMessageW(hWnd, uMsg, wParam, lParam);
        else if (which == 'sm a')
            return __sys_SendMessageA(hWnd, uMsg, wParam, lParam);

        else if (which == 'snmw')
            return __sys_SendNotifyMessageW(hWnd, uMsg, wParam, lParam);
        else if (which == 'snma')
            return __sys_SendNotifyMessageA(hWnd, uMsg, wParam, lParam);

        else if (which == 'smtw')
            return __sys_SendMessageTimeoutW(hWnd, uMsg, wParam, lParam,
                                             fuFlags, uTimeout, lpdwResult);
        else if (which == 'smta')
            return __sys_SendMessageTimeoutA(hWnd, uMsg, wParam, lParam,
                                             fuFlags, uTimeout, lpdwResult);

        else {
            SetLastError(ERROR_INVALID_PARAMETER);
            return 0;
        }
    }

    //
    // if target window is outside the job, issue an indirect request
    //

    if (uMsg == WM_COPYDATA) {
        return Gui_SendCopyData(
                which, hWnd, wParam, lParam, fuFlags, uTimeout, lpdwResult);
    }

    req.msgid = GUI_SEND_POST_MESSAGE;
    req.which = which;
    req.hwnd = (ULONG)(ULONG_PTR)hWnd;
    req.msg = uMsg;
    req.wparam = (ULONG64)(ULONG_PTR)wParam;
    req.lparam = (ULONG64)(ULONG_PTR)lParam;
    req.flags = fuFlags;
    req.timeout = uTimeout;

    rpl = Gui_CallProxyEx(&req, sizeof(req), sizeof(*rpl), TRUE);
    if (! rpl)
        return 0;

    result = (ULONG_PTR)rpl->lresult1;
    if ((which == 'smtw' || which == 'smta') && lpdwResult && result)
        *lpdwResult = (ULONG_PTR)rpl->lresult2;

    error = rpl->error;
    Dll_Free(rpl);

    SetLastError(error);
    return result;
}


//---------------------------------------------------------------------------
// Gui_SendPostMessageMulti
//---------------------------------------------------------------------------


_FX LRESULT Gui_SendPostMessageMulti(
    ULONG which, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT fuFlags, UINT uTimeout, PDWORD_PTR lpdwResult)
{
    GUI_ENUM_PROC_PARM parm;

    memzero(&parm, sizeof(parm));
    parm.which = which;
    parm.uMsg = uMsg;
    parm.wParam = wParam;
    parm.lParam = lParam;
    parm.fuFlags = fuFlags;
    parm.uTimeout = uTimeout;
    parm.lpdwResult = lpdwResult;

    if(!Gui_UseProxyService && __sys_EnumWindows)
		__sys_EnumWindows(Gui_SendPostMessageMultiCallback, (LPARAM)&parm);
    else
        Gui_EnumWindows(Gui_SendPostMessageMultiCallback, (LPARAM)&parm);

    return 1;
}


//---------------------------------------------------------------------------
// Gui_SendPostMessageMultiCallback
//---------------------------------------------------------------------------


_FX BOOL Gui_SendPostMessageMultiCallback(HWND hWnd, LPARAM lParam)
{
    GUI_ENUM_PROC_PARM *parm = (GUI_ENUM_PROC_PARM *)lParam;

    if ((ULONG_PTR)hWnd == XFF4 || (ULONG_PTR)hWnd == XFF8) {
        // this shouldn't happen, but make sure we don't get recursive
        return TRUE;
    }

    Gui_SendPostMessageCommon(
        parm->which, hWnd, parm->uMsg, parm->wParam, parm->lParam,
        parm->fuFlags, parm->uTimeout, parm->lpdwResult);

    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_SendCopyData
//---------------------------------------------------------------------------


_FX LRESULT Gui_SendCopyData(
    ULONG which, HWND hWnd, WPARAM wParam, LPARAM lParam,
    UINT fuFlags, UINT uTimeout, PDWORD_PTR lpdwResult)
{
    COPYDATASTRUCT *cds;
    GUI_SEND_COPYDATA_REQ *req;
    GUI_SEND_COPYDATA_RPL *rpl;
    LRESULT result;
    ULONG error;
    ULONG req_len, cds_len;

    //
    // we support WM_COPYDATA for SendMessage and SendMessageTimeout
    //

    if (which != 'sm w' && which != 'sm a' &&
        which != 'smtw' && which != 'smta') {

        SbieApi_Log(2205, L"WM_COPYDATA (1)");
        return FALSE;
    }

    //
    // make sure the buffer to copy is not too large
    //

    cds = (COPYDATASTRUCT *)lParam;
    cds_len = cds->cbData;
    if (cds_len > 1024*1024) {

        SbieApi_Log(2205, L"WM_COPYDATA (2)");
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    //
    // initialize a request buffer and send it
    //

    req_len = sizeof(GUI_SEND_COPYDATA_REQ) + cds_len;
    req = Dll_AllocTemp(req_len);

    req->msgid = GUI_SEND_COPYDATA;
    req->which = which;
    req->hwnd = (ULONG)(ULONG_PTR)hWnd;
    req->wparam = (ULONG64)(ULONG_PTR)wParam;
    req->flags = fuFlags;
    req->timeout = uTimeout;

    req->cds_key = (ULONG64)(ULONG_PTR)cds->dwData;
    req->cds_len = cds_len;
    if (cds_len)
        memcpy(req->cds_buf, cds->lpData, cds_len);

    rpl = Gui_CallProxyEx(req, req_len, sizeof(*rpl), TRUE);
    if (! rpl) {
        error = GetLastError();
        result = 0;
    } else {

        result = (ULONG_PTR)rpl->lresult1;
        if ((which == 'smtw' || which == 'smta') && result)
            *lpdwResult = (ULONG_PTR)rpl->lresult2;

        error = rpl->error;
        Dll_Free(rpl);
    }

    Dll_Free(req);

    SetLastError(error);
    return result;
}


//---------------------------------------------------------------------------
// Gui_PostThreadMessageA
//---------------------------------------------------------------------------


_FX BOOL Gui_PostThreadMessageA(
    ULONG idThread, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (Gui_PostThreadMessage_Check(idThread, uMsg))
        return __sys_PostThreadMessageA(idThread, uMsg, wParam, lParam);
    SetLastError(ERROR_ACCESS_DENIED);
    return FALSE;
}


//---------------------------------------------------------------------------
// Gui_PostThreadMessageW
//---------------------------------------------------------------------------


_FX BOOL Gui_PostThreadMessageW(
    ULONG idThread, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (Gui_PostThreadMessage_Check(idThread, uMsg))
        return __sys_PostThreadMessageW(idThread, uMsg, wParam, lParam);
    SetLastError(ERROR_ACCESS_DENIED);
    return FALSE;
}


//---------------------------------------------------------------------------
// Gui_PostThreadMessage_Check
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_PostThreadMessage_Check(ULONG idThread, UINT uMsg)
{
    //
    // on Windows XP, we have a kernel hook on PostThreadMessage.
    // on Vista and later, we rely on UAC/UIPI to block sending messages
    // outside the sandbox (because the process in the sandbox is running
    // at untrusted integrity).  but UIPI is disabled when UAC is disabled
    // so we also use an application level hook on PostThreadMessage
    //

    HANDLE ThreadHandle;
    WCHAR name[48];
    NTSTATUS status;
    ULONG ProcessId;
    ULONG session_id;

    if (! idThread)
        return TRUE;
    ThreadHandle = OpenThread(
                        THREAD_QUERY_LIMITED_INFORMATION, FALSE, idThread);
    if (! ThreadHandle)
        goto fail;

    ProcessId = __sys_GetProcessIdOfThread(ThreadHandle);
    CloseHandle(ThreadHandle);
    if (! ProcessId)
        goto fail;
    if (ProcessId == Dll_ProcessId)
        return TRUE;

    status = SbieApi_QueryProcess((HANDLE)(ULONG_PTR)ProcessId,
                                  name, NULL, NULL, &session_id);
    if (! NT_SUCCESS(status))
        goto fail;
    if (session_id != Dll_SessionId)
        goto fail;
    if (_wcsicmp(name, Dll_BoxName) != 0)
        goto fail;

    return TRUE;

    //
    // log the message that we blocked and fail
    //

fail:

    if (Dll_OsBuild >= 8400 &&
            __sys_GetClipboardFormatNameW((ATOM)uMsg, name, 40) &&
            _wcsicmp(name, L"MSUIM.Msg.LangBarModal") == 0) {

        //
        // on Windows 8, winkey+space pops up an input language dialog box,
        // managed by Windows Explorer, which disappears upon releasing the
        // space, but only if the process sends a thread message.  we want
        // to allow this special message
        //

        return TRUE;
    }

    Sbie_snwprintf(name, 48, L"$:TID=%08X:MSG=%08X", idThread, uMsg);
    SbieApi_MonitorPut2(MONITOR_WINCLASS | MONITOR_DENY, name, FALSE);

    return FALSE;
}


//---------------------------------------------------------------------------
// Gui_DispatchMessageA
//---------------------------------------------------------------------------


_FX LRESULT Gui_DispatchMessageA(const MSG *lpmsg)
{
    Gui_DispatchMessageCalled = TRUE;
    File_DoAutoRecover(FALSE);

    return __sys_DispatchMessageA(lpmsg);
}


//---------------------------------------------------------------------------
// Gui_DispatchMessageW
//---------------------------------------------------------------------------


_FX LRESULT Gui_DispatchMessageW(const MSG *lpmsg)
{
    Gui_DispatchMessageCalled = TRUE;
    File_DoAutoRecover(FALSE);

    return __sys_DispatchMessageW(lpmsg);
}


//---------------------------------------------------------------------------
// Gui_DispatchMessage8
//---------------------------------------------------------------------------


#ifdef _WIN64


_FX LRESULT Gui_DispatchMessage8(const MSG *lpmsg, ULONG IsAscii)
{
    Gui_DispatchMessageCalled = TRUE;
    File_DoAutoRecover(FALSE);

    return __sys_DispatchMessage8(lpmsg, IsAscii);
}


//---------------------------------------------------------------------------
// Gui_Hook_DispatchMessage8
//---------------------------------------------------------------------------

#ifndef _M_ARM64
// $HookHack$ - Custom, not automated, Hook
_FX BOOLEAN Gui_Hook_DispatchMessage8(HMODULE module)
{
    //
    // on Windows 8, the DispatchMessageA and DispatchMessageW functions
    // are very short stubs that invoke user32!DispatchMessageWorker.
    // the hook trampoline may override past the end of the short function,
    // and corrupt surrounding code.  to work around this, we analyze the
    // short functions in order to hook user32!DispatchMessageWorker instead
    //
    // DispatchMessageA     mov edx,1           BA 01 00 00 00
    //      (10 bytes)      jmp xxx             E9 xx xx xx xx
    // DispatchMessageW     xor edx,edx         33 D2
    //      (6 bytes)       jmp short xxx       EB xx
    //

    UCHAR *a = (UCHAR *)__sys_DispatchMessageA;
    UCHAR *w = (UCHAR *)__sys_DispatchMessageW;

    if (*(ULONG *)a == 0x000001BA && *(USHORT *)w == 0xD233) {

        LONG  a_offset;
        LONG  w_offset;

        if (a[5] == 0xEB)                       // short jmp
            a_offset = *(CHAR *)(a + 6) + 7;
        else if (a[5] == 0xE9)                  // normal jmp
            a_offset = *(LONG *)(a + 6) + 10;

        if (w[2] == 0xEB)                       // short jmp
            w_offset = *(CHAR *)(w + 3) + 4;
        else if (w[2] == 0xE9)                  // normal jmp
            w_offset = *(LONG *)(w + 3) + 7;
        else
            w_offset = 0;

        if ((a + a_offset) == (w + w_offset)) {

            __sys_DispatchMessage8 = (P_DispatchMessage8)(w + w_offset);

            SBIEDLL_HOOK_GUI(DispatchMessage8);

            return TRUE;
        }
    }

    return FALSE;
}
#endif

#endif _WIN64


//---------------------------------------------------------------------------
// Select text to replace "Protected Mode: Off" in Internet Explorer
//---------------------------------------------------------------------------


#ifndef PROTECTED_MODE_TEXT
#define PROTECTED_MODE_TEXT SANDBOXIE
#endif


//---------------------------------------------------------------------------
// Gui_DoStatusBarMessage
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_DoStatusBarMessage(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plResult)
{
    WCHAR clsnm[32];
    WCHAR *text, *ptr1, *ptr2, *mytext;
    ULONG len1, len2;
    ULONG LastError;

    if (Dll_ImageType != DLL_IMAGE_INTERNET_EXPLORER)
        return FALSE;

    if (Gui_DisableTitle)
        return FALSE;

    //
    // message must be directed at a status bar window
    //

    if (__sys_RealGetWindowClassW(hWnd, clsnm, 30) != 18)
        return FALSE;
    if (_wcsicmp(clsnm, L"msctls_statusbar32") != 0)
        return FALSE;

    //
    // message must be directed to status bar part 10,
    // and not specify any flags (most importantly not SBT_OWNERDRAW)
    //
    // if matches, replace Off with Sandboxie in text like
    //      Internet | Protected Mode: Off
    //
    // this only works in Internet Explorer 8, see also Gui_LoadStringW
    //

    if (wParam != 10)
        return FALSE;

    text = (WCHAR *)lParam;

    ptr1 = wcsstr(text, L" | ");
    if (! ptr1)
        return FALSE;
    ptr1 += 3;
    ptr2 = wcsstr(ptr1, L": ");
    if (! ptr2)
        return FALSE;

    len1 = (ULONG)(ULONG_PTR)(ptr1 - text);
    len2 = wcslen(PROTECTED_MODE_TEXT) + 1;
    mytext = Dll_Alloc((len1 + len2 + 4) * sizeof(WCHAR));
    wmemcpy(mytext, text, len1);
    wmemcpy(mytext + len1, PROTECTED_MODE_TEXT, len2);

    *plResult = __sys_SendMessageW(hWnd, uMsg, wParam, (LPARAM)mytext);

    LastError = GetLastError();
    Dll_Free(mytext);
    SetLastError(LastError);

    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_LoadStringW
//---------------------------------------------------------------------------


_FX int Gui_LoadStringW(
    HINSTANCE hInstance, UINT uID, void *lpBuffer, int nBufferMax)
{
    //
    // Internet Explorer 9 does not display the protected mode detail
    // in the status bar, but in a HTML property sheet dialog box created
    // by MSHTML.  Intercept the use of LoadString in MSHTML function
    //     MSHTML!CHTMLDlg::_ConcatenateZoneNameWithProtectedModeIfNeeded
    // which loads string #12940, text of which is "Protected Mode: Off"
    //

    int rv;

    if (uID == 12940 && nBufferMax >= 0x80 &&
            hInstance == GetModuleHandle(L"mshtml.dll")) {

        wcscpy((WCHAR *)lpBuffer, PROTECTED_MODE_TEXT);
        rv = wcslen((WCHAR *)lpBuffer);

    } else {

        rv = __sys_LoadStringW(hInstance, uID, lpBuffer, nBufferMax);
    }

    return rv;
}


//---------------------------------------------------------------------------
// Gui_SendMessageW_MdiCreate
//---------------------------------------------------------------------------


_FX LRESULT Gui_SendMessageW_MdiCreate(HWND hWnd, LPARAM lParam)
{
    //
    // the Windows 8 default handler for WM_MDICREATE
    // (in USER32!DefMDIChildProcW) does not call CreateWindowEx as in
    // earlier versions of Windows, but rather calls the internal
    // _CreateWindowEx that the public CreateWindowEx uses.
    //
    // this means the window class name in MDICREATESTRUCT don't get
    // sandboxed properly.  to work around this, we intercept the
    // SendMessage(WM_MDICREATE) request and replace MDICREATESTRUCT
    //

    MDICREATESTRUCT *mdics = (MDICREATESTRUCT *)lParam;

    WCHAR *clsnm = Gui_CreateClassNameW(mdics->szClass);

    LRESULT lResult;

    if (clsnm && clsnm != mdics->szClass) {

        mdics = Dll_AllocTemp(sizeof(MDICREATESTRUCT));
        memcpy(mdics, (void *)lParam, sizeof(MDICREATESTRUCT));
        mdics->szClass = clsnm;
    }

    lResult = __sys_SendMessageW(hWnd, WM_MDICREATE, 0, (LPARAM)mdics);

    if ((ULONG_PTR)mdics != (ULONG_PTR)lParam) {

        Gui_Free(mdics);
        Gui_Free(clsnm);
    }

    return lResult;
}


//---------------------------------------------------------------------------
// Gui_SendMessageA_MdiCreate
//---------------------------------------------------------------------------


_FX LRESULT Gui_SendMessageA_MdiCreate(HWND hWnd, LPARAM lParam)
{
    //
    // see comment above in Gui_SendMessageW_MdiCreate
    //

    MDICREATESTRUCTA *mdics = (MDICREATESTRUCTA *)lParam;

    char *clsnm = Gui_CreateClassNameA(mdics->szClass);

    LRESULT lResult;

    if (clsnm && clsnm != mdics->szClass) {

        mdics = Dll_AllocTemp(sizeof(MDICREATESTRUCTA));
        memcpy(mdics, (void *)lParam, sizeof(MDICREATESTRUCTA));
        mdics->szClass = clsnm;
    }

    lResult = __sys_SendMessageA(hWnd, WM_MDICREATE, 0, (LPARAM)mdics);

    if ((ULONG_PTR)mdics != (ULONG_PTR)lParam) {

        Gui_Free(mdics);
        Gui_Free(clsnm);
    }

    return lResult;
}
