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
#include <stdlib.h>


//---------------------------------------------------------------------------
//
// SUPPORT FOR DDE CONVERSATIONS
//
// There seems to be a bug in kernel win32k related to processes with a
// restricted token and DDE conversations.  GetMessage/PeekMessage by the
// receiving process ends up in win32k!xxxDDETrackGetMessageHook which
// calls win32k!HMValidateHandleNoRipNoIL to validate a dde object.
// if the thread is restricted, win32k!HMValidateHandleNoRipNoIL calls
// win32k!ValidateHandleSecure with a second parameter (value 1) which
// causes the validation to always fail.  this failure causes
// xxxDDETrackGetMessageHook to clear the message number and lParam
// value in the MSG structure it returns to the caller.
//
// note that this only affects posted DDE messages and not sent messages,
// and only occurs on Windows with UIPI, i.e. Vista and later.  but for
// sake of consistenty, this code is used on Windows XP as well.
//
// the workaround includes one dummy proxy window in the sandbox and
// a second, smarter proxy window in the SbieSvc GUI Proxy process.
//
// the sequence of events for a DDE client out of the sandbox:
//
// 1.   DDE client sends WM_DDE_INITIATE to all windows, this is
//      intercepted by Gui_DDE_INITIATE_Received which replaces the
//      wParam (which says the client window outside the sandbox)
//      with the HWND of a dummy/proxy window in RPCSS, and stores
//      the original wParam in thread local storage (TLS).
//
//      note that this is done for all windows that receives message
//      WM_DDE_INITIATE whether or not they will respond to it.
//
//      we use a proxy window in RPCSS to work around the case where
//      the process uses DDEML to drive DDE.  DDEML calls ValidateHwnd
//      on the window from WM_DDE_INITIATE and this will fail, due to
//      differences in IL, if the client is outside the sandbox.
//
// 2.   one of the windows responds to WM_DDE_INITIATE by sending a
//      reply of WM_DDE_ACK to the client window it knows about, which
//      is the proxy window in RPCSS.  function Gui_DDE_ACK_Sending
//      uses the TLS data saved by Gui_DDE_INITIATE_Received to replace
//      the target HWND on the message being sent
//
// 3.   the request to send WM_DDE_ACK goes through SbieSvc GUI Proxy,
//      and GuiServer::SendPostMessageSlave in core/svc/GuiServer.cpp
//      intercepts that and instead creates a proxy and a proxy window
//      in a work thread (see DdeProxyThreadSlave there).
//
//      that proxy window in SbieSvc will actually send the WM_DDE_ACK
//      to the client, and will present itself as the server for the
//      DDE conversation, so it can receive any posted WM_DDE_EXECUTE
//      messages without IL limitations (per the introduction above).
//
// 4.   the proxy window in SbieSvc GUI Proxy receives the WM_DDE_EXECUTE
//      message, extracts the command text given in the message LPARAM,
//      and sends a WM_COPYDATA to the server window in the sandbox.
//      the Gui_DDE_COPYDATA_Received function gets this message through
//      the Gui_CanForwardMsg function.
//
//      Gui_DDE_COPYDATA_Received posts one of the WM_DDE_* messages
//      to the window which received the WM_COPYDATA, and then discards
//      the WM_COPYDATA message itself
//
// 5.   after handling the WM_DDE_EXECUTE, the server posts a WM_DDE_ACK
//      message to the client window of the conversation, which is really
//      the proxy window in SbieSvc GUI Proxy.  the message is permitted
//      by SbieSvc because it always allows posting WM_DDE_ACK messages
//      (see GuiServer::AllowSendPostMessage).  in response, the proxy
//      window posts a WM_DDE_ACK to the real client of the conversation.
//
//
// for another changed related to DDE, see Gui_CallProxyEx and
// Gui_DispatchMessageCalled.
//
//
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOL Gui_DDE_HookMotherWindow(HWND hwnd, LPARAM lParam);

static UINT Gui_DdeInitializeW(
    ULONG_PTR pidInst, ULONG_PTR pfnCallback, ULONG afCmd, ULONG ulRes);

static UINT Gui_DdeInitializeA(
    ULONG_PTR pidInst, ULONG_PTR pfnCallback, ULONG afCmd, ULONG ulRes);


//---------------------------------------------------------------------------


HWND Gui_FindWindowW(
    const WCHAR *lpClassName, const WCHAR *lpWindowName);

HANDLE Gui_GetPropCommon(
    HWND hWnd, const void *lpString, BOOLEAN unicode, ULONG LastError);


//---------------------------------------------------------------------------


typedef LPARAM (*P_PackDDElParam)(UINT msg, UINT_PTR uiLo, UINT_PTR uiHi);
typedef BOOL (*P_UnpackDDElParam)(UINT msg, LPARAM lParam,
                                  PUINT_PTR uiLo, PUINT_PTR uiHi);

static P_PackDDElParam              __sys_PackDDElParam         = NULL;
static P_UnpackDDElParam            __sys_UnpackDDElParam       = NULL;


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static const WCHAR *Gui_DDE_TERM = L"//" SANDBOXIE L"//DDE//TERM";
static const WCHAR *Gui_DDE_ACK  = L"//" SANDBOXIE L"//DDE//ACK";
static const WCHAR *Gui_DDE_REQ  = L"//" SANDBOXIE L"//DDE//REQ//";

static ULONG Gui_DDE_TERM_Len;
static ULONG Gui_DDE_ACK_Len;
static ULONG Gui_DDE_REQ_Len;


//---------------------------------------------------------------------------
// Gui_DDE_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_DDE_Init(HMODULE module)
{
    __sys_PackDDElParam =
                Ldr_GetProcAddrNew(DllName_user32, L"PackDDElParam","PackDDElParam");
    __sys_UnpackDDElParam =
                Ldr_GetProcAddrNew(DllName_user32, L"UnpackDDElParam","UnpackDDElParam");

    Gui_DDE_TERM_Len = wcslen(Gui_DDE_TERM);
    Gui_DDE_ACK_Len  = wcslen(Gui_DDE_ACK);
    Gui_DDE_REQ_Len  = wcslen(Gui_DDE_REQ);

    SBIEDLL_HOOK_GUI(DdeInitializeW);
    SBIEDLL_HOOK_GUI(DdeInitializeA);
    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_DdeInitializeW
//---------------------------------------------------------------------------


_FX UINT Gui_DdeInitializeW(
    ULONG_PTR pidInst, ULONG_PTR pfnCallback, ULONG afCmd, ULONG ulRes)
{
    UINT rc = __sys_DdeInitializeW(pidInst, pfnCallback, afCmd, ulRes);
    ULONG err = GetLastError();
    Gui_DDE_HookMotherWindow(NULL, 0);
    SetLastError(err);
    return rc;
}


//---------------------------------------------------------------------------
// Gui_DdeInitializeA
//---------------------------------------------------------------------------


_FX UINT Gui_DdeInitializeA(
    ULONG_PTR pidInst, ULONG_PTR pfnCallback, ULONG afCmd, ULONG ulRes)
{
    UINT rc = __sys_DdeInitializeA(pidInst, pfnCallback, afCmd, ulRes);
    ULONG err = GetLastError();
    Gui_DDE_HookMotherWindow(NULL, 0);
    SetLastError(err);
    return rc;
}


//---------------------------------------------------------------------------
// Gui_DDE_HookMotherWindow
//---------------------------------------------------------------------------


_FX BOOL Gui_DDE_HookMotherWindow(HWND hwnd, LPARAM lParam)
{
    //
    // we need to hook the DDEMLMom window so our Gui_CanForwardMsg
    // front end can intercept our WM_COPYDATA messages for DDE.
    // however DDEMLMom is created using an internal version of
    // CreateWindowEx.  to work around this, we hook DdeInitialize
    // and then explicitly look for the DDEMLMom window
    //

    ULONG tid = (ULONG)lParam;
    if (! tid) {
        if (__sys_EnumThreadWindows && __sys_GetClassNameW) {
            tid = GetCurrentThreadId();
            if (tid)
                __sys_EnumThreadWindows(tid, Gui_DDE_HookMotherWindow, tid);
        }
    } else {
        WCHAR clsnm[32];
        if (__sys_GetClassNameW(hwnd, clsnm, 30)) {
            clsnm[30] = L'\0';
            if (wcscmp(clsnm, L"DDEMLMom") == 0) {
                Gui_SetWindowProc(hwnd, TRUE);
                return FALSE;
            }
        }
    }
    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_DDE_INITIATE_Received
//---------------------------------------------------------------------------


_FX WPARAM Gui_DDE_INITIATE_Received(HWND hWnd, WPARAM wParam)
{
    static HWND hProxyWnd = NULL;

    if (Gui_OpenAllWinClasses || Dll_ImageType == DLL_IMAGE_SANDBOXIE_RPCSS)
        return wParam;

    if (! wParam)
        return wParam;

    if (Gui_IsSameBox((HWND)wParam, NULL, NULL)) {

        THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

        TlsData->gui_dde_client_hwnd = (HWND)wParam;
        TlsData->gui_dde_proxy_hwnd  = NULL;
        return wParam;
    }

    if (! hProxyWnd) {

        ULONG retry;
        for (retry = 0; retry < 10; ++retry) {
            hProxyWnd = Gui_FindWindowW(SANDBOXIE L"_DDE_ProxyClass1", NULL);
            if (hProxyWnd)
                break;
            Sleep(500);
        }
    }

    if (! hProxyWnd)
        return wParam;

    else {

        THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

        TlsData->gui_dde_client_hwnd = (HWND)wParam;
        TlsData->gui_dde_proxy_hwnd  = hProxyWnd;

        return (WPARAM)hProxyWnd;
    }
}


//---------------------------------------------------------------------------
// Gui_DDE_ACK_Sending
//---------------------------------------------------------------------------


_FX HWND Gui_DDE_ACK_Sending(HWND hWnd, WPARAM wParam)
{
    if (Gui_OpenAllWinClasses)
        return hWnd;

    if (hWnd) {

        THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

        // in case window is a worker window created by DDEML,
        // hook its window proc so we can process WM_COPYDATA
        // for Gui_DDE_COPYDATA_Received
        Gui_SetWindowProc((HWND)wParam, TRUE);

        if (hWnd == TlsData->gui_dde_proxy_hwnd)
            hWnd = TlsData->gui_dde_client_hwnd;
    }

    return hWnd;
}


//---------------------------------------------------------------------------
// Gui_DDE_COPYDATA_Received
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_DDE_COPYDATA_Received(
    HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    WCHAR prop_name[64];
    HWND hClientWnd;
    HGLOBAL hGlobal;
    void *pGlobal;
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    //
    // a WM_COPYDATA message is received
    //
    // the WPARAM should specify the proxy window from DdeProxyThreadSlave
    // in SbieSvc GUI Proxy, which we can check by looking for a special
    // property on the window.
    //
    // if the calling thread has gui_dde_client_hwnd is -1, indicating an
    // in-sandbox DDE conversation, as set by Gui_DDE_Post_In_Box, then
    // we don't expect the "other" window to have this special property
    //

    COPYDATASTRUCT *cds = (COPYDATASTRUCT *)lParam;
    if (cds->dwData != tzuk)
        return FALSE;

    Sbie_snwprintf(prop_name, 64, SBIE L"_DDE_%08p", (void*)hWnd);
    hClientWnd = Gui_GetPropCommon((HWND)wParam, prop_name, TRUE, 0);
    if (TlsData->gui_dde_client_hwnd != (HWND)-1) {
#ifdef _WIN64
		if ((!hClientWnd) || (hClientWnd != TlsData->gui_dde_client_hwnd))
			return FALSE;
#else
		// When a *.xlsx file is double clicked, excel 2016 will run, but Gui_GetPropCommon will return empty.
		// This is a problem for Office 32-bit only
		if (hClientWnd == 0)
			hClientWnd = TlsData->gui_dde_client_hwnd;
		else if (hClientWnd != TlsData->gui_dde_client_hwnd)
			return FALSE;
#endif
    }

    //
    // WM_DDE_ACK sent as specific text in WM_COPYDATA that we recognize
    //

    if (cds->cbData == (Gui_DDE_ACK_Len + 1) * sizeof(WCHAR)
            && wcscmp(cds->lpData, Gui_DDE_ACK) == 0) {

        const ULONG_PTR _DDEACK_fAck = 0x8000;

        if (TlsData->gui_dde_client_hwnd == (HWND)-1
                && (! TlsData->gui_dde_proxy_hwnd)
                && (WPARAM)hWnd == TlsData->gui_dde_post_wparam) {
            // if variables are as set by the WM_DDE_EXECUTE code
            // in Gui_DDE_Post_In_Box (see below), then restore
            // lParam originally specified by caller
            hGlobal = (HGLOBAL)TlsData->gui_dde_post_lparam;
        } else {
            // otherwise we create a dummy lParam
            hGlobal = GlobalAlloc(GMEM_DDESHARE | GMEM_FIXED, 8);
        }

        lParam = __sys_PackDDElParam(
                            WM_DDE_ACK, _DDEACK_fAck, (UINT_PTR)hGlobal);
        __sys_PostMessageW(hWnd, WM_DDE_ACK, wParam, lParam);
        return TRUE;
    }

    //
    // WM_DDE_TERMINATE as WM_DDE_ACK above
    //

    if (cds->cbData == (Gui_DDE_TERM_Len + 1) * sizeof(WCHAR)
            && wcscmp(cds->lpData, Gui_DDE_TERM) == 0) {

        __sys_PostMessageW(hWnd, WM_DDE_TERMINATE, wParam, 0);
        return TRUE;
    }

    //
    // WM_DDE_REQUEST sent as specific text.  the LPARAM that was sent in
    // the DDE message is embedded in the command text
    //

    if (cds->cbData > (Gui_DDE_REQ_Len + 1) * sizeof(WCHAR)
            && wcsncmp(cds->lpData, Gui_DDE_REQ, Gui_DDE_REQ_Len) == 0) {

        WCHAR *endptr = (WCHAR *)cds->lpData + Gui_DDE_REQ_Len;
        lParam = wcstoul(endptr, &endptr, 16);

        if (TlsData->gui_dde_proxy_hwnd)
            hClientWnd = (HWND)TlsData->gui_dde_proxy_hwnd;

        __sys_PostMessageW(hWnd, WM_DDE_REQUEST, (WPARAM)hClientWnd, lParam);
        return TRUE;
    }

    //
    // otherwise this is text we don't recognize so this is WM_DDE_EXECUTE,
    // this may be ANSI text sent to a UNICODE server that we need to fix.
    // e.g. Opera uses UNICODE DDE windows but sends ANSI DDE EXECUTE text
    //

    if (cds->cbData >= sizeof(ULONG) &&
                        _wcsicmp(Dll_ImageName, L"opera.exe") == 0) {

        // if starts with a quote and there are no zero bytes as would
        // be typical in a UNICODE string, then assume ANSI string
        UCHAR *bytes = (UCHAR *)cds->lpData;
        if (bytes[0] == L'\"' && bytes[1] && bytes[2] && bytes[3]) {

            hGlobal = GlobalAlloc(GMEM_DDESHARE | GMEM_FIXED,
                                  cds->cbData * sizeof(WCHAR));
            if (hGlobal) {
                pGlobal = GlobalLock(hGlobal);
                if (pGlobal) {
                    ULONG i;
                    for (i = 0; i < cds->cbData; ++i)
                        ((WCHAR *)pGlobal)[i] = ((UCHAR *)cds->lpData)[i];
                    GlobalUnlock(hGlobal);

                    if (TlsData->gui_dde_proxy_hwnd)
                        hClientWnd = (HWND)TlsData->gui_dde_proxy_hwnd;

                    __sys_PostMessageW(hWnd, WM_DDE_EXECUTE,
                                       (WPARAM)hClientWnd, (LPARAM)hGlobal);
                    return TRUE;
                }
            }

            return FALSE;
        }
    }

    //
    // otherwise this is text we don't recognize so normal WM_DDE_EXECUTE
    //

    hGlobal = GlobalAlloc(GMEM_DDESHARE | GMEM_FIXED, cds->cbData);
    if (hGlobal) {
        pGlobal = GlobalLock(hGlobal);
        if (pGlobal) {
            memcpy(pGlobal, cds->lpData, cds->cbData);
            GlobalUnlock(hGlobal);

            if (TlsData->gui_dde_proxy_hwnd)
                hClientWnd = (HWND)TlsData->gui_dde_proxy_hwnd;

            __sys_PostMessageW(
                hWnd, WM_DDE_EXECUTE, (WPARAM)hClientWnd, (LPARAM)hGlobal);
            return TRUE;
        }
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// Gui_DDE_Post_In_Box
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_DDE_Post_In_Box(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //
    // support for DDE conversation between two programs in the sandbox
    //

    COPYDATASTRUCT cds;
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    WCHAR prop_name[64];
    Sbie_snwprintf(prop_name, 64, SBIE L"_DDE_%08p", (void*)hWnd);
    __sys_SetPropW((HWND)wParam, prop_name, (HWND)wParam);

    cds.dwData = tzuk;

    //
    // WM_DDE_EXECUTE, extract command text and send WM_COPYDATA
    //

    if (uMsg == WM_DDE_EXECUTE) {

        TlsData->gui_dde_client_hwnd = (HWND)-1;
        TlsData->gui_dde_proxy_hwnd  = NULL;
        TlsData->gui_dde_post_wparam = wParam;
        TlsData->gui_dde_post_lparam = lParam;

        cds.cbData = (ULONG)(ULONG_PTR)GlobalSize((HGLOBAL)lParam);
        cds.lpData = GlobalLock((HGLOBAL)lParam);
        if (cds.lpData) {

            __sys_SendMessageW(hWnd, WM_COPYDATA, wParam, (LPARAM)&cds);
            GlobalUnlock((HGLOBAL)lParam);
        }

        Gui_SetWindowProc((HWND)wParam, TRUE);

    //
    // WM_DDE_ACK and WM_DDE_TERMINATE, send specific text via WM_COPYDATA
    //

    } else if (uMsg == WM_DDE_ACK) {

        if (hWnd == TlsData->gui_dde_proxy_hwnd) {
            //
            // trying to post WM_DDE_ACK to the proxy window in RPCSS won't
            // do anything, but it probably means the server is expecting to
            // end the conversation, so simulate a WM_DDE_TERMINATE
            //
            __sys_PostMessageW(
                        (HWND)wParam, WM_DDE_TERMINATE, (WPARAM)hWnd, 0);
            return TRUE;
        }

        cds.lpData = (void *)Gui_DDE_ACK;
        cds.cbData = (Gui_DDE_ACK_Len + 1) * sizeof(WCHAR);
        __sys_SendMessageW(hWnd, WM_COPYDATA, wParam, (LPARAM)&cds);

    } else if (uMsg == WM_DDE_TERMINATE) {

        cds.lpData = (void *)Gui_DDE_TERM;
        cds.cbData = (Gui_DDE_TERM_Len + 1) * sizeof(WCHAR);
        __sys_SendMessageW(hWnd, WM_COPYDATA, wParam, (LPARAM)&cds);

    } else {

        SbieApi_Log(2205, L"DDE %08X", uMsg);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_DDE_DATA_Posting
//---------------------------------------------------------------------------


_FX LRESULT Gui_DDE_DATA_Posting(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    //
    // support for posting a WM_DDE_DATA reply in response to an earlier
    // WM_DDE_REQUEST message.  this is intended primarily for support
    // for the Bookmark Buddy utility, running outside the sandbox.
    //
    // WM_DDE_REQUEST/WM_DDE_DATA conversations between two programs in
    // the sandbox is not supported.
    //

    typedef struct {
        unsigned short  unused:12,
                        fResponse:1,
                        fRelease:1,
                        reserved:1,
                        fAckReq:1;
        short           cfFormat;
        BYTE            Value[1];
    } DDEDATA;

    LRESULT result = 0;
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    if (TlsData->gui_dde_proxy_hwnd && TlsData->gui_dde_client_hwnd &&
            TlsData->gui_dde_proxy_hwnd == (HWND)hWnd) {

        ULONG_PTR lo, hi;
        if (__sys_UnpackDDElParam(WM_DDE_DATA, lParam, &lo, &hi)) {

            DDEDATA *DdeData = GlobalLock((HGLOBAL)lo);
            ULONG DdeDataLen = 0;
            if (DdeData) {
                if (DdeData->fResponse) // reply to WM_DDE_REQUEST
                    DdeDataLen = (ULONG)GlobalSize((HGLOBAL)lo);
            }

            if (DdeDataLen) {

                GUI_SEND_COPYDATA_REQ *req;
                GUI_SEND_COPYDATA_RPL *rpl;
                ULONG req_len;

                req_len = sizeof(GUI_SEND_COPYDATA_REQ) + DdeDataLen;
                req = Dll_AllocTemp(req_len);

                req->msgid = GUI_SEND_COPYDATA;
                req->which = 'dde ';
                req->hwnd = (ULONG)(ULONG_PTR)TlsData->gui_dde_client_hwnd;
                req->wparam = 0;
                req->flags = 0;
                req->timeout = 0;

                req->cds_key = (ULONG64)(ULONG_PTR)hi;
                req->cds_len = DdeDataLen;
                if (DdeDataLen)
                    memcpy(req->cds_buf, DdeData, DdeDataLen);

                rpl = Gui_CallProxyEx(req, req_len, sizeof(*rpl), TRUE);
                if (rpl) {
                    result = (LRESULT)rpl->lresult1;
                    Dll_Free(rpl);
                }

                Dll_Free(req);
            }

            if (DdeData)
                GlobalUnlock(DdeData);
        }
    }

    if (! result)
        SbieApi_Log(2205, L"DDE DATA");
    return result;
}
