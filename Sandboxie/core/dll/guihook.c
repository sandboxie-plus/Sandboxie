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
#include "core/drv/api_flags.h"
#include "core/svc/GuiWire.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define MAX_THREADS         1000


//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------


typedef struct _GUI_HOOK {

    LIST_ELEM list_elem;
    ULONG_PTR eyecatcher;
    CRITICAL_SECTION crit;
    int idHook;
    HOOKPROC lpfn;
    HINSTANCE hMod;
    BOOLEAN IsUnicode;
    LIST threads;

} GUI_HOOK;


typedef struct _GUI_THREAD {

    LIST_ELEM list_elem;
    HHOOK hhk;
    ULONG_PTR idThread;
    FILETIME CreationTime;

} GUI_THREAD;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN Gui_IsThreadInThisProcess(ULONG ThreadId);

static HHOOK Gui_SetWindowsHookExA(
    int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId);

static HHOOK Gui_SetWindowsHookExW(
    int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId);

static HHOOK Gui_SetWindowsHookEx(
    int idHook, HOOKPROC lpfn, HINSTANCE hMod, BOOLEAN IsUnicode);

static BOOL Gui_HookCollectThreads(HWND hwnd, LPARAM lParam);

static BOOLEAN Gui_HookThread(GUI_HOOK *ghk, ULONG_PTR idThread);

static BOOL Gui_UnhookWindowsHookEx(HHOOK hhk);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static CRITICAL_SECTION Gui_HooksCritSec;
static LIST Gui_Hooks;
static DWORD Gui_HookHelperThreadId = 0;
static int Gui_HookCount = 0;

//---------------------------------------------------------------------------
// Gui_InitWinHooks
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_InitWinHooks(void)
{
    InitializeCriticalSection(&Gui_HooksCritSec);
    List_Init(&Gui_Hooks);

    SBIEDLL_HOOK_GUI(SetWindowsHookExA);
    SBIEDLL_HOOK_GUI(SetWindowsHookExW);
    SBIEDLL_HOOK_GUI(UnhookWindowsHookEx);

    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_IsThreadInThisProcess
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_IsThreadInThisProcess(ULONG ThreadId)
{
    const ACCESS_MASK DesiredAccess = (Dll_OsBuild >= 6000
                                    ? THREAD_QUERY_LIMITED_INFORMATION
                                    : THREAD_QUERY_INFORMATION);
    NTSTATUS status;
    HANDLE ThreadHandle;
    CLIENT_ID ClientId;
    OBJECT_ATTRIBUTES objattrs;

    if (! ThreadId)
        return FALSE;

    InitializeObjectAttributes(&objattrs, NULL, 0, NULL, NULL);
    ClientId.UniqueProcess = NULL;
    ClientId.UniqueThread = (HANDLE)(ULONG_PTR)ThreadId;
    status = NtOpenThread(
                    &ThreadHandle, DesiredAccess, &objattrs, &ClientId);

    if (NT_SUCCESS(status)) {

        THREAD_BASIC_INFORMATION ThreadBasicInfo;
        ULONG len;

        status = NtQueryInformationThread(
                    ThreadHandle, ThreadBasicInformation,
                    &ThreadBasicInfo, sizeof(ThreadBasicInfo), &len);

        NtClose(ThreadHandle);

        if (NT_SUCCESS(status)) {

            HANDLE ProcessId = ThreadBasicInfo.ClientId.UniqueProcess;
            if ((ULONG)(ULONG_PTR)ProcessId == Dll_ProcessId) {

                return TRUE;
            }
        }
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// Gui_SetWindowsHookExA
//---------------------------------------------------------------------------


_FX HHOOK Gui_SetWindowsHookExA(
    int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId)
{
    HHOOK hhook;

    if (    idHook == WH_JOURNALRECORD || idHook == WH_JOURNALPLAYBACK ||
            idHook == WH_KEYBOARD_LL   || idHook == WH_MOUSE_LL        ||
            //Gui_IsThreadInThisProcess(dwThreadId)
            dwThreadId != 0 || hMod == NULL
        ) {

        //
        // if this is a non-injecting hook, or if an injecting hook
        // targets a thread in the same process, then let the system
        // take care of the hook
        //

        hhook = __sys_SetWindowsHookExA(idHook, lpfn, hMod, dwThreadId);

    } else {

        //
        // otherwise we manage the hook here
        //

        hhook = Gui_SetWindowsHookEx(idHook, lpfn, hMod, FALSE);
    }

    return hhook;
}


//---------------------------------------------------------------------------
// Gui_SetWindowsHookExW
//---------------------------------------------------------------------------


_FX HHOOK Gui_SetWindowsHookExW(
    int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId)
{
    //
    // the hook dll is not injected, and instead the hook notify message
    // is sent to the hooking thread, if the hook type is one of the
    // four listed below, so we don't care to protect these cases.
    //
    // we also don't care if the target thread is in the same process.
    //
    // in any other case, the hooker probablys expects their hook dll to
    // be injected into target threads, but this will not occur.
    //
    // on Vista and later, UIPI will prevent the dll from loading into
    // threads outside the sandbox.  (but note that it might be able to
    // load into threads in other sandboxes.)
    //
    // on XP we don't have UIPI so instead SetWindowsHook in win32k is
    // hooked to block hooks which would inject dlls outside the sandbox.
    //

    //
    // the boxed "global" hook mechanism should only be used for global hooks
    // using it for hooks that specify a thread ID like done in sbie 5.33.6 
    // and earlier results in a non standard conform behavioure:
    // the hook in handled as a global one and injectted into all processes.
    //
    // so we let the system handle hooks on a specified thread or once without 
    // a dll module handle always as those are not global
    //

    HHOOK hhook;

    if (    idHook == WH_JOURNALRECORD || idHook == WH_JOURNALPLAYBACK ||
            idHook == WH_KEYBOARD_LL   || idHook == WH_MOUSE_LL        ||
            //Gui_IsThreadInThisProcess(dwThreadId)
            dwThreadId != 0 || hMod == NULL
        ) {

        if (idHook == WH_MOUSE_LL && Dll_ImageType == DLL_IMAGE_WISPTIS) {

            // hack:  block hook by Microsoft WISPTIS (tablet input program)
            hhook = (HHOOK)(ULONG_PTR)0x12345678;

        } else {

            //
            // if this is a non-injecting hook, or if an injecting hook
            // targets a thread in the same process, then let the system
            // take care of the hook
            //

            hhook = __sys_SetWindowsHookExW(idHook, lpfn, hMod, dwThreadId);
        }

    } else {

        //
        // otherwise we manage the hook here
        //

        hhook = Gui_SetWindowsHookEx(idHook, lpfn, hMod, TRUE);
    }

    return hhook;
}


//---------------------------------------------------------------------------
// Gui_HookHelperProc
//---------------------------------------------------------------------------


ULONG CALLBACK Gui_HookHelperProc(LPVOID lpParam)
{
    //
    // global hooks are turned into thread specific hooks and managed 
    // by the service worker for each session, see GuiServer::WndHookNotifySlave
    //
    // whenever a window is created the service gets notified and instructs
    // the hooking pocess to hook the window's thread this is done using QueueUserAPC
    // targeting this helper thread, whenever a APC is scheduled the thread 
    // will resume and execute it, it being Gui_NotifyWinHooksAPC
    //

    DWORD MyThreadId = Gui_HookHelperThreadId;
    while (MyThreadId == Gui_HookHelperThreadId)
        SleepEx(10, TRUE); // be in a waitable state for he APC's

    return 0;
}


//---------------------------------------------------------------------------
// Gui_NotifyWinHooksAPC
//---------------------------------------------------------------------------


_FX VOID Gui_NotifyWinHooksAPC(ULONG_PTR idThread)
{
    GUI_HOOK *ghk;

    EnterCriticalSection(&Gui_HooksCritSec);

    ghk = List_Head(&Gui_Hooks);
    while (ghk) {
        Gui_HookThread(ghk, idThread);
        ghk = List_Next(ghk);
    }

    LeaveCriticalSection(&Gui_HooksCritSec);
}


//---------------------------------------------------------------------------
// Gui_SetWindowsHookEx
//---------------------------------------------------------------------------


_FX HHOOK Gui_SetWindowsHookEx(
    int idHook, HOOKPROC lpfn, HINSTANCE hMod, BOOLEAN IsUnicode)
{
    GUI_HOOK *ghk;
    LONG_PTR *threads;
    LONG_PTR i;
    BOOLEAN ok;
    ULONG err;

    //
    // allocate hook and apply it to all active threads in the sandbox
    //

    ghk = Dll_Alloc(sizeof(GUI_HOOK));
    ghk->eyecatcher = tzuk;
    ghk->idHook = idHook;
    ghk->lpfn = lpfn;
    ghk->hMod = hMod;
    ghk->IsUnicode = IsUnicode;
    List_Init(&ghk->threads);
    InitializeCriticalSection(&ghk->crit);

    threads = Dll_Alloc((MAX_THREADS + 4) * sizeof(LONG_PTR));
    threads[0] = 0;
	if(Gui_OpenAllWinClasses && __sys_EnumWindows)
		__sys_EnumWindows(Gui_HookCollectThreads, (LPARAM)threads);
	else
	    Gui_EnumWindows(Gui_HookCollectThreads, (LPARAM)threads);

    ok = FALSE;
    err = 0;
    for (i = 1; i <= threads[0]; ++i) {
        if (Gui_HookThread(ghk, threads[i]))
            ok = TRUE;
        else if (err == 0 || err == ERROR_ACCESS_DENIED)
            err = GetLastError();
    }

    Dll_Free(threads);

    if (! ok) {
        Dll_Free(ghk);
        SetLastError(err);
        return NULL;
    }

    EnterCriticalSection(&Gui_HooksCritSec);
    List_Insert_After(&Gui_Hooks, NULL, ghk);
    Gui_HookCount++;
    if (Gui_HookHelperThreadId == 0) {
        HANDLE HookHelperThread = CreateThread(NULL, 0, Gui_HookHelperProc, (LPVOID)0, CREATE_SUSPENDED, &Gui_HookHelperThreadId);
        ResumeThread(HookHelperThread);
        CloseHandle(HookHelperThread);
    }
    LeaveCriticalSection(&Gui_HooksCritSec);

    Gui_RegisterWinHook(Gui_HookHelperThreadId, (ULONG64)ghk);

    SetLastError(0);
    return (HHOOK)ghk;
}


//---------------------------------------------------------------------------
// Gui_RegisterWinHook
//---------------------------------------------------------------------------


_FX LRESULT Gui_RegisterWinHook(DWORD dwThreadId, ULONG64 ghk)
{
    GUI_WND_HOOK_REGISTER_REQ req;
    GUI_WND_HOOK_REGISTER_RPL* rpl;

    req.msgid = GUI_WND_HOOK_REGISTER;
    req.hthread = dwThreadId;
    req.hproc = dwThreadId ? (ULONG64)&Gui_NotifyWinHooksAPC : 0ull;
    req.hhook = ghk;

    rpl = Gui_CallProxy(&req, sizeof(req), sizeof(*rpl));
    if (rpl)
        Dll_Free(rpl);

    return 0;
}


//---------------------------------------------------------------------------
// Gui_HookCollectThreads
//---------------------------------------------------------------------------


_FX BOOL Gui_HookCollectThreads(HWND hwnd, LPARAM lParam)
{
    LONG_PTR *threads = (ULONG_PTR *)lParam;

    if (threads[0] < MAX_THREADS) {

        LONG_PTR i;
        LONG_PTR idProcess, idThread;

        if (Gui_IsSameBox(hwnd, &idProcess, &idThread)) {

            for (i = 1; i <= threads[0]; ++i)
                if (threads[i] == idThread)
                    return TRUE;

            threads[0] = i;
            threads[i] = idThread;
        }
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_HookThread
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_HookThread(GUI_HOOK *ghk, ULONG_PTR idThread)
{
    GUI_THREAD *thd;
    HANDLE hThread;
    FILETIME CreationTime, ExitTime, KernelTime, UserTime;
    BOOL b;

    //
    // get the thread creation time
    //

    hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, (ULONG)idThread);
    if (hThread) {

        b = GetThreadTimes(
                hThread, &CreationTime, &ExitTime, &KernelTime, &UserTime);

        CloseHandle(hThread);

    } else
        b = FALSE;

    if (! b)
        return FALSE;

    //
    // see if we already hooked the particular thread id with the same
    // thread creation time, in which case, we have nothing to do
    //

    EnterCriticalSection(&ghk->crit);

    thd = List_Head(&ghk->threads);
    while (thd) {
        if (thd->idThread == idThread &&
            thd->CreationTime.dwLowDateTime == CreationTime.dwLowDateTime &&
            thd->CreationTime.dwHighDateTime == CreationTime.dwHighDateTime)
            break;
        thd = List_Next(thd);
    }

    if (thd) {
        LeaveCriticalSection(&ghk->crit);
        return TRUE;
    }

    //
    // hook the thread, since we haven't hooked it before
    //

    thd = Dll_Alloc(sizeof(GUI_THREAD));

    if (ghk->IsUnicode) {
        thd->hhk = __sys_SetWindowsHookExW(
                        ghk->idHook, ghk->lpfn, ghk->hMod, (ULONG)idThread);
    } else {
        thd->hhk = __sys_SetWindowsHookExA(
                        ghk->idHook, ghk->lpfn, ghk->hMod, (ULONG)idThread);
    }

#ifdef DEBUG_HOOKS
    {   WCHAR msg[256];
        Sbie_snwprintf(msg, 256, L"2 SetWindowsHookEx%c idHook=%d lpfn=%08X hMod=%08X dwThreadId=%d ; hhook=%08X error=%d\n",
        ghk->IsUnicode ? L'W' : L'A',
        ghk->idHook, ghk->lpfn, ghk->hMod, idThread, thd->hhk, GetLastError());
        OutputDebugString(msg);
    }
#endif DEBUG_HOOKS

    if (! thd->hhk) {
        ULONG err = GetLastError();
        LeaveCriticalSection(&ghk->crit);
        Dll_Free(thd);

        if (err == 0)
            err = ERROR_ACCESS_DENIED;
        SetLastError(err);
        return FALSE;
    }

    thd->idThread = idThread;
    thd->CreationTime.dwLowDateTime = CreationTime.dwLowDateTime;
    thd->CreationTime.dwHighDateTime = CreationTime.dwHighDateTime;

    List_Insert_After(&ghk->threads, NULL, thd);

    LeaveCriticalSection(&ghk->crit);

    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_UnhookWindowsHookEx
//---------------------------------------------------------------------------

_FX BOOL Gui_UnhookWindowsHookEx(HHOOK hhk)
{
    GUI_HOOK *ghk=NULL;
    GUI_THREAD *thd;

    //
    // try to differentiate between a real HHOOK handle
    // and a fake handle that we produce in Gui_SetWindowsHookEx
    // The 'fake handle' is an allocated memory address pointing to a
    // GUI_HOOK data structure thus this value is aligned to the architecture
    // word size: 4 bytes on 32bit systems and 8 bytes on 64 bit systems.  
    // Check the memory alignment first to test for a real HHOOK handle.

    if (!hhk) {
        return TRUE;
    }

#ifdef _WIN64
    if ((ULONG_PTR)hhk % 8) {
#else
    if ((ULONG_PTR)hhk % 4) {
#endif
        return __sys_UnhookWindowsHookEx(hhk);
    }
    
    ghk = (GUI_HOOK *) hhk;
    __try {
        if (ghk->eyecatcher != tzuk) {
            ghk = NULL;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        ghk = NULL;
    }


    if (! ghk)
       return __sys_UnhookWindowsHookEx(hhk);

    //
    // unhook the psuedo-global hook that we set
    //

    EnterCriticalSection(&Gui_HooksCritSec);
    List_Remove(&Gui_Hooks, ghk);
    Gui_HookCount--;
    if (Gui_HookCount <= 0)
        Gui_HookHelperThreadId = 0;
    LeaveCriticalSection(&Gui_HooksCritSec);

    Gui_RegisterWinHook(0, (ULONG64)ghk);

    EnterCriticalSection(&ghk->crit);

    while (1) {
        thd = List_Head(&ghk->threads);
        if (! thd)
            break;
        __sys_UnhookWindowsHookEx(thd->hhk);
        List_Remove(&ghk->threads, thd);
        Dll_Free(thd);
    }

    LeaveCriticalSection(&ghk->crit);
    Dll_Free(ghk);

    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_NotifyWinHooks
//---------------------------------------------------------------------------


_FX LRESULT Gui_NotifyWinHooks()
{
    GUI_WND_HOOK_NOTIFY_REQ req;
    GUI_WND_HOOK_NOTIFY_RPL *rpl;

    req.msgid = GUI_WND_HOOK_NOTIFY;
    req.threadid = GetCurrentThreadId();
        
    rpl = Gui_CallProxy(&req, sizeof(req), sizeof(*rpl));
    if (rpl)
        Dll_Free(rpl);

    return 0;
}

