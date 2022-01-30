/*
 * Copyright 2021 DavidXanatos, xanasoft.com
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
// Dump Helper
//---------------------------------------------------------------------------

#include "dll.h"
#include "dump.h"

#include <dbghelp.h>

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


typedef BOOL (__stdcall *P_MiniDumpWriteDump)(
  IN HANDLE hProcess,
  IN DWORD ProcessId,
  IN HANDLE hFile,
  IN MINIDUMP_TYPE DumpType,
  IN CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, OPTIONAL
  IN CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, OPTIONAL
  IN CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam OPTIONAL
  );

typedef LPTOP_LEVEL_EXCEPTION_FILTER(*P_SetUnhandledExceptionFilter)(
    _In_opt_ LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter);

//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static P_MiniDumpWriteDump __sys_MiniDumpWriteDump;
static HMODULE Dump_DbgHelpMod;

#define DUMP_FLAGS_DEFAULT  MiniDumpWithHandleData | \
                            MiniDumpWithUnloadedModules | \
                            MiniDumpFilterModulePaths | \
                            MiniDumpWithProcessThreadData | \
                            MiniDumpWithoutOptionalData | \
                            MiniDumpIgnoreInaccessibleMemory | \
                            MiniDumpFilterTriage //0x001205a4  thats what WerFault.exe uses

#define DUMP_FLAGS_EXTENDED MiniDumpWithFullMemory | \
                            MiniDumpWithHandleData | \
                            MiniDumpWithThreadInfo | \
                            MiniDumpWithProcessThreadData | \
                            MiniDumpWithFullMemoryInfo | \
                            MiniDumpWithUnloadedModules | \
                            MiniDumpWithFullAuxiliaryState | \
                            MiniDumpIgnoreInaccessibleMemory | \
                            MiniDumpWithTokenInformation

static MINIDUMP_TYPE Dump_Flags = DUMP_FLAGS_DEFAULT;
static P_SetUnhandledExceptionFilter __sys_SetUnhandledExceptionFilter = NULL;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Dump_CrashHandlerExceptionFilter
//---------------------------------------------------------------------------

LONGLONG GetCurCycle()
{
	LONGLONG freq, now;
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	QueryPerformanceCounter((LARGE_INTEGER*)&now);
	LONGLONG dwNow = ((now * 1000000) / freq); 
	return dwNow; // returns time since system start in us
}

static LONG __stdcall Dump_CrashHandlerExceptionFilter(EXCEPTION_POINTERS* pEx)
{
#ifdef _M_IX86
    if (pEx->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
    {
        // be sure that we have enought space...
        static char MyStack[1024 * 128];
        // it assumes that DS and SS are the same!!! (this is the case for Win32)
        // change the stack only if the selectors are the same (this is the case for Win32)
        //__asm push offset MyStack[1024*128];
        //__asm pop esp;
        __asm mov eax, offset MyStack[1024 * 128];
        __asm mov esp, eax;
    }
#endif

    SbieApi_Log(2224, L"%S [%S]", Dll_ImageName, Dll_BoxName);

    BOOLEAN bSuccess = FALSE;
    HANDLE hFile;

    wchar_t szMiniDumpFileName[128];
    Sbie_snwprintf(szMiniDumpFileName, 128, L"%s.%lld.dmp", Dll_ImageName, GetCurCycle);

    wchar_t szMiniDumpFilePath[MAX_PATH] = { 0 };
    Sbie_snwprintf(szMiniDumpFilePath, MAX_PATH, L"%s\\%s", Dll_BoxFilePath, szMiniDumpFileName);
    //hFile = CreateFile(szMiniDumpFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    UNICODE_STRING uni;
    OBJECT_ATTRIBUTES attr;
    RtlInitUnicodeString(&uni, szMiniDumpFilePath);
    InitializeObjectAttributes(&attr, &uni, OBJ_CASE_INSENSITIVE, NULL, 0);

    IO_STATUS_BLOCK Iosb;
    extern P_NtCreateFile __sys_NtCreateFile;
    NTSTATUS status = __sys_NtCreateFile(&hFile, FILE_GENERIC_WRITE, &attr, &Iosb, NULL, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ, FILE_OPEN_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

    //if (hFile != INVALID_HANDLE_VALUE)
    if (NT_SUCCESS(status))
    {
        MINIDUMP_EXCEPTION_INFORMATION stMDEI;
        stMDEI.ThreadId = GetCurrentThreadId();
        stMDEI.ExceptionPointers = pEx;
        stMDEI.ClientPointers = TRUE;
        // try to create an miniDump:
        if (__sys_MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, Dump_Flags, &stMDEI, NULL, NULL))
        {
            bSuccess = TRUE;
        }
        //CloseHandle(hFile);
        extern P_NtClose __sys_NtClose;
        __sys_NtClose(hFile);
    }

    wchar_t szMiniDumpMessage[256];
    if (!bSuccess)
        Sbie_snwprintf(szMiniDumpMessage, 256, L"%s crashed!\r\nCrashdump creation failed.", Dll_ImageName);
    else
        Sbie_snwprintf(szMiniDumpMessage, 256, L"%s crashed!\r\nCrashdump saved to \"%s\".", Dll_ImageName, szMiniDumpFileName);
    SbieApi_MonitorPut2(MONITOR_OTHER | MONITOR_TRACE, szMiniDumpMessage, FALSE);

    // or return one of the following:
    // - EXCEPTION_CONTINUE_SEARCH      // this will trigger the "normal" OS error-dialog
    // - EXCEPTION_CONTINUE_EXECUTION
    // - EXCEPTION_EXECUTE_HANDLER      // this will prevent the invocation of WerFault.exe
    return EXCEPTION_EXECUTE_HANDLER; 
}


//---------------------------------------------------------------------------
// Dump_SetUnhandledExceptionFilter
//---------------------------------------------------------------------------


ALIGNED LPTOP_LEVEL_EXCEPTION_FILTER Dump_SetUnhandledExceptionFilter(
    _In_opt_ LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter
)
{
    return NULL;
}


//---------------------------------------------------------------------------
// Dump_SetUnhandledExceptionFilter
//---------------------------------------------------------------------------


/*ALIGNED BOOLEAN Dump_MiniDumpWriteDump(
    _In_ HANDLE hProcess,
    _In_ DWORD ProcessId,
    _In_ HANDLE hFile,
    _In_ MINIDUMP_TYPE DumpType,
    _In_opt_ PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
    _In_opt_ PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
    _In_opt_ PMINIDUMP_CALLBACK_INFORMATION CallbackParam
)
{
    while (! IsDebuggerPresent()) {
        OutputDebugString(L"BREAK\n");
        Sleep(500);
    }
    __debugbreak();
    
    return __sys_MiniDumpWriteDump(hProcess, ProcessId, hFile, DumpType, ExceptionParam, UserStreamParam, CallbackParam);
}*/


//---------------------------------------------------------------------------
// Dump_Init
//---------------------------------------------------------------------------


_FX int Dump_Init(void)
{
    if (Dump_DbgHelpMod != NULL)
        return 2;

    // Initialize the member, so we do not load the dll after the exception has occured
    // which might be not possible anymore...
    Dump_DbgHelpMod = LoadLibrary(L"dbghelp.dll");
    if (!Dump_DbgHelpMod )
        return 0;

    /*if (_wcsicmp(Dll_ImageName, L"WerFault.exe") == 0) // fre experimenting only
    {
        P_MiniDumpWriteDump MiniDumpWriteDump = (P_MiniDumpWriteDump)GetProcAddress(Dump_DbgHelpMod, "MiniDumpWriteDump");
        SBIEDLL_HOOK(Dump_, MiniDumpWriteDump);
        return 1;
    }*/

    __sys_MiniDumpWriteDump = (P_MiniDumpWriteDump)GetProcAddress(Dump_DbgHelpMod, "MiniDumpWriteDump");

    // get the default preset
    WCHAR str[32];
    if (NT_SUCCESS(SbieApi_QueryConfAsIs(NULL, L"MiniDumpFlags", 0, str, sizeof(str) - sizeof(WCHAR)))) {
        if (_wcsnicmp(str, L"0x", 2) == 0)
            Dump_Flags = wcstol(str + 2, NULL, 16);
        else if (_wcsnicmp(str, L"extended", 3) == 0) // check only first 3 letters
            Dump_Flags = DUMP_FLAGS_EXTENDED;
    }

    // Register Unhandled Exception-Filter:
    SetUnhandledExceptionFilter(Dump_CrashHandlerExceptionFilter);

    // Additional call "PreventSetUnhandledExceptionFilter"...
    // See also: "SetUnhandledExceptionFilter" and VC8 (and later)
    // http://blog.kalmbachnet.de/?postid=75

    SBIEDLL_HOOK(Dump_, SetUnhandledExceptionFilter);

    //SbieApi_MonitorPut2(MONITOR_OTHER | MONITOR_TRACE, L"Minidump enabled", FALSE);
    return 1;
}