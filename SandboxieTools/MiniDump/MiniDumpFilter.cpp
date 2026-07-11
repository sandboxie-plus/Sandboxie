/*
 * Copyright 2024 David Xanatos, xanasoft.com
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
// MiniDumpFilter - Reusable exception filter library
//---------------------------------------------------------------------------

#include "MiniDumpFilter.h"

//---------------------------------------------------------------------------
// Constants
//---------------------------------------------------------------------------

#define MDF_CLEANUP_TIMEOUT_MS  5000    // 5 second timeout for cleanup callback

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------

static WCHAR g_szDumperPath[MAX_PATH] = {0};
static WCHAR g_szAppName[64] = L"Unknown";
static DWORD g_dwDumpType = MDF_TYPE_TRIAGE;
static MDF_CLEANUP_CALLBACK g_pfnCleanup = NULL;
static LPTOP_LEVEL_EXCEPTION_FILTER g_pOldFilter = NULL;
static BOOL g_bInitialized = FALSE;

//---------------------------------------------------------------------------
// Forward Declarations
//---------------------------------------------------------------------------

static LONG WINAPI MiniDumpFilter_ExceptionHandler(EXCEPTION_POINTERS* pEx);
static void SafeRunCleanupCallback(void);

//---------------------------------------------------------------------------
// MiniDumpFilter_Init
//---------------------------------------------------------------------------

BOOL MiniDumpFilter_Init(LPCWSTR pszDumperPath, LPCWSTR pszAppName, DWORD dwDumpType,
                         MDF_CLEANUP_CALLBACK pfnCleanup)
{
    if (g_bInitialized) {
        return TRUE;  // Already initialized
    }

    // Save dump type
    g_dwDumpType = dwDumpType;
    if (g_dwDumpType > MDF_TYPE_FULL) {
        g_dwDumpType = MDF_TYPE_TRIAGE;
    }

    // Save cleanup callback
    g_pfnCleanup = pfnCleanup;

    // Copy app name
    if (pszAppName && pszAppName[0]) {
        wcsncpy_s(g_szAppName, 64, pszAppName, _TRUNCATE);
    } else {
        wcscpy_s(g_szAppName, 64, L"Unknown");
    }

    // Get dumper path
    if (pszDumperPath && pszDumperPath[0]) {
        wcscpy_s(g_szDumperPath, MAX_PATH, pszDumperPath);
    } else {
        // Default: MiniDump.exe next to current executable
        if (!GetModuleFileNameW(NULL, g_szDumperPath, MAX_PATH)) {
            return FALSE;
        }

        // Find last backslash and replace filename
        WCHAR* p = wcsrchr(g_szDumperPath, L'\\');
        if (p) {
            wcscpy_s(p + 1, MAX_PATH - (p - g_szDumperPath) - 1, L"MiniDump.exe");
        } else {
            wcscpy_s(g_szDumperPath, MAX_PATH, L"MiniDump.exe");
        }
    }

    // Install exception filter
    g_pOldFilter = SetUnhandledExceptionFilter(MiniDumpFilter_ExceptionHandler);
    g_bInitialized = TRUE;

    return TRUE;
}

//---------------------------------------------------------------------------
// MiniDumpFilter_Shutdown
//---------------------------------------------------------------------------

void MiniDumpFilter_Shutdown(void)
{
    if (!g_bInitialized) {
        return;
    }

    // Restore old filter
    SetUnhandledExceptionFilter(g_pOldFilter);
    g_pOldFilter = NULL;
    g_bInitialized = FALSE;
}

//---------------------------------------------------------------------------
// CleanupThreadProc - Thread procedure for safe cleanup execution
//---------------------------------------------------------------------------

static DWORD WINAPI CleanupThreadProc(LPVOID lpParam)
{
    MDF_CLEANUP_CALLBACK pfnCallback = (MDF_CLEANUP_CALLBACK)lpParam;

    // Execute cleanup callback inside __try/__except
    // This ensures that even if the cleanup crashes, we catch it
    __try {
        pfnCallback();
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        // Cleanup crashed - ignore and continue
        // The dump will still be created
    }

    return 0;
}

//---------------------------------------------------------------------------
// SafeRunCleanupCallback - Run cleanup callback safely in a separate thread
//---------------------------------------------------------------------------

static void SafeRunCleanupCallback(void)
{
    if (!g_pfnCleanup) {
        return;  // No callback registered
    }

    // Create a thread to run the cleanup
    // This isolates the cleanup from the crash handler's stack/state
    HANDLE hThread = CreateThread(
        NULL,               // Default security
        64 * 1024,          // 64 KB stack (minimal)
        CleanupThreadProc,
        (LPVOID)g_pfnCleanup,
        0,                  // Run immediately
        NULL
    );

    if (hThread) {
        // Wait for cleanup to complete with timeout
        // If it hangs or takes too long, we proceed anyway
        WaitForSingleObject(hThread, MDF_CLEANUP_TIMEOUT_MS);

        // Don't call TerminateThread - it's too dangerous
        // Just close handle and proceed; the thread may still be running
        // but MiniDump.exe will capture the current state
        CloseHandle(hThread);
    }

    // Small delay to let memory operations complete
    Sleep(100);
}

//---------------------------------------------------------------------------
// MiniDumpFilter_ExceptionHandler
//---------------------------------------------------------------------------

static LONG WINAPI MiniDumpFilter_ExceptionHandler(EXCEPTION_POINTERS* pEx)
{
#ifdef _M_IX86
    // Handle stack overflow on x86 by switching to a static stack
    // This allows us to run code even when the stack is exhausted
    if (pEx->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW) {
        static char s_emergencyStack[1024 * 128];  // 128 KB static stack
        // Switch stack pointer to emergency stack
        __asm {
            mov eax, offset s_emergencyStack[1024 * 128]
            mov esp, eax
        }
    }
#endif

    // Skip debug exceptions - these are not real crashes
    if (pEx->ExceptionRecord->ExceptionCode == DBG_PRINTEXCEPTION_C ||
        pEx->ExceptionRecord->ExceptionCode == DBG_PRINTEXCEPTION_WIDE_C) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    // Run cleanup callback to clear sensitive data BEFORE creating dump
    // This runs in a protected context - if it crashes, we still proceed
    SafeRunCleanupCallback();

    // Build command line with minimal heap usage (use stack buffer)
    // Format: "path\MiniDump.exe" /pid:N /tid:N /ep:0xN /name:Name /type:N
    WCHAR cmdLine[512];
    wsprintfW(cmdLine, L"\"%s\" /pid:%lu /tid:%lu /ep:0x%p /name:%s /type:%lu",
              g_szDumperPath,
              GetCurrentProcessId(),
              GetCurrentThreadId(),
              pEx,
              g_szAppName,
              g_dwDumpType);

    // Spawn MiniDump.exe
    STARTUPINFOW si = {0};
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi = {0};

    if (CreateProcessW(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        // Wait for MiniDump.exe to complete (with timeout)
        WaitForSingleObject(pi.hProcess, 60000);  // 60 second timeout

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    // Let other handlers run (including WER if configured)
    return EXCEPTION_CONTINUE_SEARCH;
}
