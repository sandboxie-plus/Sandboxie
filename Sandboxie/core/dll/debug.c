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
// Debug Helper
//---------------------------------------------------------------------------


#include "debug.h"
#ifdef WITH_DEBUG


//---------------------------------------------------------------------------


#include "dll.h"
#include <stdio.h>


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define BREAK_STRING            "Generating random cookie"

//#define BREAK_PROC      "Install"
//#define BREAK_PROC      "_OptInMicrosoftUpdate@4"
//#define BREAK_PROC      "g1"
//#define BREAK_PROC      "ExecSecureObjects"
//#define BREAK_PROC      "SoftwareDirectorMsiErrorCheck"
//#define BREAK_PROC      "InstallDriverPackages"
#define BREAK_PROC        "MSIunzipcore"

#undef  HIDE_SBIEDLL


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static void Debug_RtlSetLastWin32Error(ULONG err);

static void Debug_OutputDebugStringW(const WCHAR *str);

static void Debug_OutputDebugStringA(const UCHAR *str);

static BOOL Debug_DebugActiveProcess(ULONG dwProcessId);

static BOOL Debug_WaitForDebugEvent(
    LPDEBUG_EVENT lpDebugEvent, DWORD dwMilliseconds);

#ifdef BREAK_PROC
static ULONG_PTR Debug_GetProcAddress(HMODULE hModule, const UCHAR *ProcName);
#endif

#ifdef HIDE_SBIEDLL
static NTSTATUS Debug_LdrGetDllHandle(
    WCHAR *DllPath, ULONG *DllCharacteristics,
    UNICODE_STRING *DllName, ULONG_PTR *DllHandle);
#endif


//---------------------------------------------------------------------------


typedef void (*P_RtlSetLastWin32Error)(ULONG err);
typedef void (*P_OutputDebugString)(const void *str);
typedef BOOL (*P_DebugActiveProcess)(ULONG dwProcessId);
typedef BOOL (*P_WaitForDebugEvent)(
    LPDEBUG_EVENT lpDebugEvent, DWORD dwMilliseconds);


static P_RtlSetLastWin32Error       __sys_RtlSetLastWin32Error      = NULL;
static P_OutputDebugString          __sys_OutputDebugStringW        = NULL;
static P_OutputDebugString          __sys_OutputDebugStringA        = NULL;
static P_DebugActiveProcess         __sys_DebugActiveProcess        = NULL;
static P_WaitForDebugEvent          __sys_WaitForDebugEvent         = NULL;


static Debug_dwProcessId = 0;


#ifdef BREAK_PROC
typedef ULONG_PTR (*P_GetProcAddress)(HMODULE hModule, const UCHAR *ProcName);
static P_GetProcAddress __sys_GetProcAddress            = NULL;
#endif

#ifdef HIDE_SBIEDLL
typedef NTSTATUS (*P_LdrGetDllHandle)(
    WCHAR *DllPath, ULONG *DllCharacteristics,
    UNICODE_STRING *DllName, ULONG_PTR *DllHandle);
static P_LdrGetDllHandle __sys_LdrGetDllHandle          = NULL;
__declspec(dllimport) NTSTATUS LdrGetDllHandle(
    WCHAR *DllPath, ULONG *DllCharacteristics,
    UNICODE_STRING *DllName, ULONG_PTR *DllHandle);
#endif


//---------------------------------------------------------------------------
// Debug_Init
//---------------------------------------------------------------------------


_FX int Debug_Init(void)
{
    P_OutputDebugString OutputDebugStringW;
    P_OutputDebugString OutputDebugStringA;
    P_RtlSetLastWin32Error RtlSetLastWin32Error;
    P_DebugActiveProcess DebugActiveProcess;
    P_WaitForDebugEvent  WaitForDebugEvent;

    //
    // intercept NTDLL entry points
    //

    RtlSetLastWin32Error = (P_RtlSetLastWin32Error)
        GetProcAddress(Dll_Ntdll, "RtlSetLastWin32Error");

    //SBIEDLL_HOOK(Debug_,RtlSetLastWin32Error);

    //
    // intercept KERNEL32 entry points
    //

    OutputDebugStringW = (P_OutputDebugString)
        GetProcAddress(Dll_Kernel32, "OutputDebugStringW");
    OutputDebugStringA = (P_OutputDebugString)
        GetProcAddress(Dll_Kernel32, "OutputDebugStringA");

    /*if (_wcsicmp(Dll_ImageName, L"msiexec.exe") == 0) {

        SBIEDLL_HOOK(Debug_,OutputDebugStringW);
        SBIEDLL_HOOK(Debug_,OutputDebugStringA);
    }*/

    DebugActiveProcess = (P_DebugActiveProcess)
        GetProcAddress(Dll_Kernel32, "DebugActiveProcess");

    WaitForDebugEvent = (P_WaitForDebugEvent)
        GetProcAddress(Dll_Kernel32, "WaitForDebugEvent");

    //SBIEDLL_HOOK(Debug_,DebugActiveProcess);
    //SBIEDLL_HOOK(Debug_,WaitForDebugEvent);

#ifdef BREAK_PROC
    SBIEDLL_HOOK(Debug_,GetProcAddress);
#endif

#ifdef HIDE_SBIEDLL
    SBIEDLL_HOOK(Debug_,LdrGetDllHandle);
#endif

    //
    //
    //

#if 0

    if (_wcsicmp(Dll_ImageName, L"006995CD.EXE") == 0 ||
        _wcsicmp(Dll_ImageName, L"WINDOWS.EXE") == 0) {

        UCHAR *ptr = (UCHAR *)0x00405168;
        if (*ptr == 0x55) {
            ULONG prot;
            VirtualProtect(ptr, 5, PAGE_EXECUTE_READWRITE, &prot);
            ptr[0] = 0x31;
            ptr[1] = 0xC0;
            ptr[2] = 0xC3;
        }

        /*UCHAR *call = (UCHAR *)0x0040bf3b;
        if (*call == 0xE8) {
            ULONG prot;
            VirtualProtect(call, 5, PAGE_EXECUTE_READWRITE, &prot);
            call[0] = 0x90;
            call[1] = 0x90;
            call[2] = 0x90;
            call[3] = 0x90;
            call[4] = 0x90;
        }*/
    }

#endif

    //
    // break
    //

	OutputDebugString(L"Dll_ImageName: ");
    OutputDebugString(Dll_ImageName);

    //if (_wcsicmp(Dll_ImageName, L"explorer.exe") == 0) {
    //if (_wcsicmp(Dll_ImageName, L"WISPTIS.exe") == 0) {
    //if (_wcsicmp(Dll_ImageName, L"TrustedInstaller.exe") == 0) {
    //if (_wcsicmp(Dll_ImageName, L"chrome.exe") == 0) {
    //if (_wcsicmp(Dll_ImageName, L"dllhost.exe") == 0) {
    //if ((! Dll_IsWow64) && _wcsicmp(Dll_ImageName, L"msiexec.exe") == 0) {
    //if (_wcsicmp(Dll_ImageName, L"SandboxieRpcSs.exe") == 0) {
    //if (_wcsicmp(Dll_ImageName, L"acrord32.exe") == 0) {
    //if (_wcsicmp(Dll_ImageName, L"conhost.exe") == 0) {
    //if (_wcsicmp(Dll_ImageName, L"IExplore.exe") == 0) {
    //if (_wcsicmp(Dll_ImageName, L"tiWorker.exe") == 0) {
    //if (0) {

#if defined(BREAK_IMAGE_1)
    if (_wcsicmp(Dll_ImageName, BREAK_IMAGE_1) == 0)
#else
    if (0)
#endif
    {
        while (! IsDebuggerPresent()) {
            OutputDebugString(L"BREAK\n");
            Sleep(500);
        }
        __debugbreak();
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Debug_RtlSetLastWin32Error
//---------------------------------------------------------------------------


ALIGNED void Debug_RtlSetLastWin32Error(ULONG err)
{
    static volatile ULONG InError = 0;
    if (err) {
        if (InterlockedIncrement(&InError) == 1) {
            WCHAR txt[64];
            Sbie_snwprintf(txt, 64, L"SetErr %d\n", err);
            OutputDebugString(txt);
            InterlockedDecrement(&InError);
        }
    }
    __sys_RtlSetLastWin32Error(err);
}


//---------------------------------------------------------------------------
// Debug_OutputDebugStringW
//---------------------------------------------------------------------------


ALIGNED void Debug_OutputDebugStringW(const WCHAR *str)
{
    if (wcsstr(str, TEXT(BREAK_STRING))) {
        __sys_OutputDebugStringW(L"***** BREAK STRING *****\n");
        while (! IsDebuggerPresent())
            Sleep(500);
        __debugbreak();
    }
    __sys_OutputDebugStringW(str);
}


//---------------------------------------------------------------------------
// Debug_OutputDebugStringA
//---------------------------------------------------------------------------


ALIGNED void Debug_OutputDebugStringA(const UCHAR *str)
{
    if (strstr(str, BREAK_STRING)) {
        __sys_OutputDebugStringA("***** BREAK STRING *****\n");
        while (! IsDebuggerPresent())
            Sleep(500);
        __debugbreak();
    }
    __sys_OutputDebugStringA(str);
}


//---------------------------------------------------------------------------
// Debug_DebugActiveProcess
//---------------------------------------------------------------------------


ALIGNED BOOL Debug_DebugActiveProcess(ULONG dwProcessId)
{
    BOOL ok;
    ULONG err;

    WCHAR txt[128];
    Sbie_snwprintf(txt, 128, L"Debug Active Process Id %d\n", dwProcessId);
    OutputDebugString(txt);
    Debug_dwProcessId = dwProcessId;

    ok = __sys_DebugActiveProcess(dwProcessId);
    err = GetLastError();

    DebugSetProcessKillOnExit(FALSE);

    SetLastError(err);
    return ok;
}


//---------------------------------------------------------------------------
// Debug_WaitForDebugEvent
//---------------------------------------------------------------------------


ALIGNED BOOL Debug_WaitForDebugEvent(
    LPDEBUG_EVENT lpDebugEvent, DWORD dwMilliseconds)
{
    WCHAR txt[256];
    BOOL ok;
    ULONG err;

    ok = __sys_WaitForDebugEvent(lpDebugEvent, dwMilliseconds);
    err = GetLastError();

    Sbie_snwprintf(txt, 256, L"Debug Event %d on Process Id %d\n",
        ok ? lpDebugEvent->dwDebugEventCode : 0,
        ok ? lpDebugEvent->dwProcessId : 0);
    OutputDebugString(txt);

    /*if (! ok) {
        if (Debug_dwProcessId) {
            OutputDebugString(L"Detaching debugger\n");
            DebugActiveProcessStop(Debug_dwProcessId);
            Debug_dwProcessId = 0;
        }
    }*/

    SetLastError(err);
    return ok;
}


//---------------------------------------------------------------------------
// Debug_GetProcAddress
//---------------------------------------------------------------------------


#ifdef BREAK_PROC
ALIGNED ULONG_PTR Debug_GetProcAddress(HMODULE hModule, const UCHAR *ProcName)
{
    const ULONG_PTR High = ((ULONG_PTR)ProcName) >> 16;
    if (High && _stricmp(ProcName, BREAK_PROC) == 0) {
        while (! IsDebuggerPresent()) {
            OutputDebugString(L"GetProcAddress Break\n");
            Sleep(500);
        }
        __debugbreak();
    }
    return __sys_GetProcAddress(hModule, ProcName);
}
#endif


//---------------------------------------------------------------------------
// Debug_GetProcAddress
//---------------------------------------------------------------------------


#ifdef HIDE_SBIEDLL
_FX NTSTATUS Debug_LdrGetDllHandle(
    WCHAR *DllPath, ULONG *DllCharacteristics,
    UNICODE_STRING *DllName, ULONG_PTR *DllHandle)
{
    NTSTATUS status = __sys_LdrGetDllHandle(
        DllPath, DllCharacteristics, DllName, DllHandle);
    if (Dll_InitComplete) {
        if (NT_SUCCESS(status) && DllHandle &&
                                    *DllHandle == (ULONG_PTR)Dll_Instance) {
            *DllHandle = 0;
            status = STATUS_DLL_NOT_FOUND;
        }
    }
    return status;
}
#endif


//---------------------------------------------------------------------------


#endif WITH_DEBUG
