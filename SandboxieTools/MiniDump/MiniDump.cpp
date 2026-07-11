/*
 * Copyright 2026 David Xanatos, xanasoft.com
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
// MiniDump - Out-of-process crash dump utility
//---------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>
#include <wtsapi32.h>
#include <shlobj.h>
#include <commdlg.h>
#include <aclapi.h>
#include <sddl.h>
#include <winhttp.h>
#include <tlhelp32.h>
#include <commctrl.h>
#include <psapi.h>
#include <stdio.h>
#include "resource.h"

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "wtsapi32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "comctl32.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

//---------------------------------------------------------------------------
// Defines and Types
//---------------------------------------------------------------------------

enum DumpType {
    DUMP_TRIAGE   = 0,
    DUMP_STANDARD = 1,
    DUMP_FULL     = 2
};

#define MDRESULT_SUCCESS       0
#define MDRESULT_ERROR         1
#define MDRESULT_PATH_REJECTED 2

#define MAX_COMMENT_LENGTH     4096

typedef struct {
    DWORD dwDumpType;
    BOOL  bSubmit;
    BOOL  bSave;
    WCHAR szPath[MAX_PATH];
    WCHAR szComment[MAX_COMMENT_LENGTH];
} MINIDUMP_UI_RESPONSE;

typedef struct {
    DWORD dwResult;
    DWORD dwError;
    WCHAR szMessage[256];
} MINIDUMP_RESULT;

// Triage dump flags (same as WerFault.exe)
#define DUMP_FLAGS_TRIAGE   (MiniDumpWithHandleData | \
                             MiniDumpWithUnloadedModules | \
                             MiniDumpFilterModulePaths | \
                             MiniDumpWithProcessThreadData | \
                             MiniDumpWithoutOptionalData | \
                             MiniDumpIgnoreInaccessibleMemory | \
                             MiniDumpFilterTriage)

// Standard dump with heap
#define DUMP_FLAGS_STANDARD (MiniDumpWithDataSegs | \
                             MiniDumpWithHandleData | \
                             MiniDumpWithUnloadedModules | \
                             MiniDumpWithProcessThreadData | \
                             MiniDumpWithPrivateReadWriteMemory | \
                             MiniDumpIgnoreInaccessibleMemory)

// Full memory dump
#define DUMP_FLAGS_FULL     (MiniDumpWithFullMemory | \
                             MiniDumpWithHandleData | \
                             MiniDumpWithThreadInfo | \
                             MiniDumpWithProcessThreadData | \
                             MiniDumpWithFullMemoryInfo | \
                             MiniDumpWithUnloadedModules | \
                             MiniDumpWithFullAuxiliaryState | \
                             MiniDumpIgnoreInaccessibleMemory | \
                             MiniDumpWithTokenInformation)

#define SUBMIT_URL_HOST     L"xanasoft.com"
#define SUBMIT_URL_PATH     L"/dumps/submit.php"

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------

static HINSTANCE g_hInstance = NULL;
static WCHAR g_szExePath[MAX_PATH] = {0};
static WCHAR g_szAppName[64] = L"Unknown";
static WCHAR g_szAppVersion[32] = L"";
static WCHAR g_szPipeName[128] = {0};

// Command line parameters
static BOOL g_bUiMode = FALSE;
static BOOL g_bStandaloneMode = FALSE;
static DWORD g_dwPid = 0;
static DWORD g_dwTid = 0;
static PVOID g_pExceptionPointers = NULL;
static DWORD g_dwDumpType = DUMP_TRIAGE;

//---------------------------------------------------------------------------
// Forward Declarations
//---------------------------------------------------------------------------

static BOOL ParseCommandLine(LPWSTR lpCmdLine);
static BOOL IsSession0AndSystem(void);
static int DoUserDump(void);
static int DoSession0Dump(void);
static int DoUiMode(void);
static int DoStandaloneMode(void);
static int DumpToTempFallback(void);
static BOOL WriteDumpToFile(LPCWSTR szPath, DWORD dwDumpType);
static BOOL WriteDumpForProcess(DWORD dwPid, LPCWSTR szPath, DWORD dwDumpType);
static BOOL IsPathSafe(LPCWSTR szPath);
static void GenerateDefaultPath(LPWSTR szPath, LPCWSTR szAppName);
static void GenerateTempPath(LPWSTR szPath, LPCWSTR szAppName, BOOL bSystemTemp);
static INT_PTR CALLBACK CrashDumpDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK ProcessListDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL BrowseForSaveLocation(HWND hParent, LPWSTR szPath);
static HANDLE CreateNamedPipeWithUserAccess(DWORD sessionId, LPWSTR szPipeName);
static BOOL SpawnUiProcess(DWORD sessionId, LPCWSTR pipeName);
static BOOL SubmitCrashDump(LPCWSTR szDumpPath, LPCWSTR szComment);
static BOOL GetMachineGuid(LPWSTR szGuid, DWORD cchGuid);
static void UpdateSaveControls(HWND hDlg, BOOL bSaveEnabled);
static void PopulateProcessList(HWND hListView);
static void SetDialogIcon(HWND hDlg);

//---------------------------------------------------------------------------
// Entry Point
//---------------------------------------------------------------------------

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(nCmdShow);

    g_hInstance = hInstance;
    GetModuleFileNameW(NULL, g_szExePath, MAX_PATH);

    // Initialize common controls for ListView
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(icex);
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    if (!ParseCommandLine(lpCmdLine)) {
        // No valid parameters - show standalone process list
        return DoStandaloneMode();
    }

    if (g_bUiMode) {
        return DoUiMode();
    }

    if (IsSession0AndSystem()) {
        return DoSession0Dump();
    } else {
        return DoUserDump();
    }
}

//---------------------------------------------------------------------------
// ParseCommandLine
//---------------------------------------------------------------------------

static BOOL ParseCommandLine(LPWSTR lpCmdLine)
{
    LPWSTR p = lpCmdLine;

    while (*p) {
        // Skip whitespace
        while (*p == L' ' || *p == L'\t') p++;
        if (*p == L'\0') break;

        if (*p == L'/') {
            p++;

            if (_wcsnicmp(p, L"ui", 2) == 0 && (p[2] == L' ' || p[2] == L'\0')) {
                g_bUiMode = TRUE;
                p += 2;
            }
            else if (_wcsnicmp(p, L"pid:", 4) == 0) {
                g_dwPid = wcstoul(p + 4, &p, 10);
            }
            else if (_wcsnicmp(p, L"tid:", 4) == 0) {
                g_dwTid = wcstoul(p + 4, &p, 10);
            }
            else if (_wcsnicmp(p, L"ep:", 3) == 0) {
                if (p[3] == L'0' && (p[4] == L'x' || p[4] == L'X')) {
                    g_pExceptionPointers = (PVOID)(ULONG_PTR)wcstoull(p + 5, &p, 16);
                } else {
                    g_pExceptionPointers = (PVOID)(ULONG_PTR)wcstoull(p + 3, &p, 16);
                }
            }
            else if (_wcsnicmp(p, L"name:", 5) == 0) {
                p += 5;
                LPWSTR start = p;
                while (*p && *p != L' ' && *p != L'\t') p++;
                size_t len = p - start;
                if (len > 63) len = 63;
                wcsncpy_s(g_szAppName, 64, start, len);

                // Extract version from name if present (e.g., "SandMan-v1.2.3")
                WCHAR* pVer = wcsstr(g_szAppName, L"-v");
                if (pVer) {
                    wcscpy_s(g_szAppVersion, 32, pVer + 2);
                }
            }
            else if (_wcsnicmp(p, L"type:", 5) == 0) {
                g_dwDumpType = wcstoul(p + 5, &p, 10);
                if (g_dwDumpType > DUMP_FULL) g_dwDumpType = DUMP_TRIAGE;
            }
            else if (_wcsnicmp(p, L"pipe:", 5) == 0) {
                p += 5;
                LPWSTR start = p;
                while (*p && *p != L' ' && *p != L'\t') p++;
                size_t len = p - start;
                if (len > 127) len = 127;
                wcsncpy_s(g_szPipeName, 128, start, len);
            }
            else {
                // Unknown parameter, skip to next space
                while (*p && *p != L' ' && *p != L'\t') p++;
            }
        }
        else {
            // Skip unknown token
            while (*p && *p != L' ' && *p != L'\t') p++;
        }
    }

    // Validate required parameters
    if (!g_bUiMode && (g_dwPid == 0 || g_dwTid == 0)) {
        return FALSE;
    }
    if (g_bUiMode && g_szPipeName[0] == L'\0') {
        return FALSE;
    }

    return TRUE;
}

//---------------------------------------------------------------------------
// IsSession0AndSystem
//---------------------------------------------------------------------------

static BOOL IsSession0AndSystem(void)
{
    DWORD sessionId = 0;
    HANDLE hToken = NULL;
    BYTE buffer[256];
    PTOKEN_USER pTokenUser;
    DWORD dwLen = 0;
    BOOL isSystem = FALSE;
    PSID pSystemSid = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    // Check session ID
    if (!ProcessIdToSessionId(GetCurrentProcessId(), &sessionId) || sessionId != 0) {
        return FALSE;
    }

    // Check if running as SYSTEM
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        return FALSE;
    }

    pTokenUser = (PTOKEN_USER)buffer;
    if (GetTokenInformation(hToken, TokenUser, pTokenUser, sizeof(buffer), &dwLen)) {
        if (AllocateAndInitializeSid(&ntAuthority, 1, SECURITY_LOCAL_SYSTEM_RID,
                0, 0, 0, 0, 0, 0, 0, &pSystemSid)) {
            isSystem = EqualSid(pTokenUser->User.Sid, pSystemSid);
            FreeSid(pSystemSid);
        }
    }

    CloseHandle(hToken);
    return isSystem;
}

//---------------------------------------------------------------------------
// DoUserDump - User session: show dialog directly, then dump
//---------------------------------------------------------------------------

static int DoUserDump(void)
{
    MINIDUMP_UI_RESPONSE response;
    INT_PTR dlgResult;
    WCHAR szTempPath[MAX_PATH];
    LPCWSTR szDumpPath;
    BOOL bDeleteAfter = FALSE;

    memset(&response, 0, sizeof(response));
    response.dwDumpType = g_dwDumpType;
    response.bSubmit = FALSE;
    response.bSave = TRUE;
    GenerateDefaultPath(response.szPath, g_szAppName);

    dlgResult = DialogBoxParamW(g_hInstance, MAKEINTRESOURCE(IDD_CRASHDUMP),
                                NULL, CrashDumpDlgProc, (LPARAM)&response);

    if (dlgResult != IDOK) {
        return 0;  // User cancelled
    }

    // Determine where to save
    if (response.bSave) {
        szDumpPath = response.szPath;
    } else if (response.bSubmit) {
        // Save to temp for submission, then delete
        GenerateTempPath(szTempPath, g_szAppName, FALSE);
        szDumpPath = szTempPath;
        bDeleteAfter = TRUE;
    } else {
        // Nothing to do
        return 0;
    }

    if (!WriteDumpToFile(szDumpPath, response.dwDumpType)) {
        MessageBoxW(NULL, L"Failed to write crash dump file.", L"Crash Dump Error", MB_ICONERROR);
        return 1;
    }

    // Submit if requested
    if (response.bSubmit) {
        while (!SubmitCrashDump(szDumpPath, response.szComment)) {
            int ret = MessageBoxW(NULL,
                L"Failed to submit crash dump to server.\n\nWould you like to retry?",
                L"Crash Dump", MB_RETRYCANCEL | MB_ICONWARNING);
            if (ret != IDRETRY) {
                break;
            }
        }
    }

    // Delete temp file if not saving
    if (bDeleteAfter) {
        DeleteFileW(szDumpPath);
    }

    return 0;
}

//---------------------------------------------------------------------------
// DoSession0Dump - Session 0: spawn UI in user session, get path via IPC
//---------------------------------------------------------------------------

static int DoSession0Dump(void)
{
    WCHAR szPipeName[64];
    DWORD activeSession;
    HANDLE hPipe;
    MINIDUMP_UI_RESPONSE response;
    MINIDUMP_RESULT result;
    DWORD dwRead = 0;
    DWORD dwWritten = 0;
    WCHAR szTempPath[MAX_PATH];
    LPCWSTR szDumpPath;
    BOOL bDeleteAfter = FALSE;

    // Get active console session
    activeSession = WTSGetActiveConsoleSessionId();
    if (activeSession == 0xFFFFFFFF) {
        return DumpToTempFallback();
    }

    // Create named pipe with user access
    hPipe = CreateNamedPipeWithUserAccess(activeSession, szPipeName);
    if (hPipe == INVALID_HANDLE_VALUE) {
        return DumpToTempFallback();
    }

    // Spawn UI process in user session
    if (!SpawnUiProcess(activeSession, szPipeName)) {
        CloseHandle(hPipe);
        return DumpToTempFallback();
    }

    // Wait for UI process to connect
    if (!ConnectNamedPipe(hPipe, NULL) && GetLastError() != ERROR_PIPE_CONNECTED) {
        CloseHandle(hPipe);
        return DumpToTempFallback();
    }

    // Read response from UI
    memset(&response, 0, sizeof(response));
    if (!ReadFile(hPipe, &response, sizeof(response), &dwRead, NULL) || dwRead < sizeof(response)) {
        CloseHandle(hPipe);
        return DumpToTempFallback();
    }

    // Check if user cancelled
    if (!response.bSave && !response.bSubmit) {
        CloseHandle(hPipe);
        return 0;  // User cancelled, no dump
    }

    // Determine where to save
    memset(&result, 0, sizeof(result));
    if (response.bSave) {
        // Validate path (safety check)
        if (!IsPathSafe(response.szPath)) {
            result.dwResult = MDRESULT_PATH_REJECTED;
            wcscpy_s(result.szMessage, 256, L"Path is not allowed (system directory or file exists)");
            WriteFile(hPipe, &result, sizeof(result), &dwWritten, NULL);
            CloseHandle(hPipe);
            return 0;
        }
        szDumpPath = response.szPath;
    } else {
        // Save to Windows temp for submission, then delete
        GenerateTempPath(szTempPath, g_szAppName, TRUE);
        szDumpPath = szTempPath;
        bDeleteAfter = TRUE;
    }

    // Write dump
    if (!WriteDumpToFile(szDumpPath, response.dwDumpType)) {
        result.dwResult = MDRESULT_ERROR;
        result.dwError = GetLastError();
        wcscpy_s(result.szMessage, 256, L"Failed to write crash dump file");
    } else {
        result.dwResult = MDRESULT_SUCCESS;

        // Submit if requested
        if (response.bSubmit) {
            SubmitCrashDump(szDumpPath, response.szComment);
        }

        // Delete temp file if not saving
        if (bDeleteAfter) {
            DeleteFileW(szDumpPath);
        }
    }

    // Send result to UI
    WriteFile(hPipe, &result, sizeof(result), &dwWritten, NULL);
    CloseHandle(hPipe);

    return (result.dwResult == MDRESULT_SUCCESS) ? 0 : 1;
}

//---------------------------------------------------------------------------
// DoUiMode - Show dialog and send result via pipe
//---------------------------------------------------------------------------

static int DoUiMode(void)
{
    HANDLE hPipe;
    DWORD dwMode;
    MINIDUMP_UI_RESPONSE response;
    INT_PTR dlgResult;
    DWORD dwWritten = 0;
    MINIDUMP_RESULT result;
    DWORD dwRead = 0;

    // Connect to pipe
    hPipe = CreateFileW(g_szPipeName, GENERIC_READ | GENERIC_WRITE, 0,
                        NULL, OPEN_EXISTING, 0, NULL);
    if (hPipe == INVALID_HANDLE_VALUE) {
        return 1;
    }

    // Set pipe to message mode
    dwMode = PIPE_READMODE_MESSAGE;
    SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL);

    // Prepare response
    memset(&response, 0, sizeof(response));
    response.dwDumpType = g_dwDumpType;
    response.bSubmit = FALSE;
    response.bSave = TRUE;
    GenerateDefaultPath(response.szPath, g_szAppName);

    // Show dialog
    dlgResult = DialogBoxParamW(g_hInstance, MAKEINTRESOURCE(IDD_CRASHDUMP),
                                NULL, CrashDumpDlgProc, (LPARAM)&response);

    if (dlgResult != IDOK) {
        response.bSave = FALSE;
        response.bSubmit = FALSE;
    }

    // Send response
    WriteFile(hPipe, &response, sizeof(response), &dwWritten, NULL);

    // Wait for result if not cancelled
    if (response.bSave || response.bSubmit) {
        memset(&result, 0, sizeof(result));
        if (ReadFile(hPipe, &result, sizeof(result), &dwRead, NULL) && dwRead >= sizeof(result)) {
            if (result.dwResult == MDRESULT_PATH_REJECTED) {
                MessageBoxW(NULL, result.szMessage, L"Crash Dump", MB_ICONWARNING);
            } else if (result.dwResult == MDRESULT_ERROR) {
                MessageBoxW(NULL, result.szMessage, L"Crash Dump Error", MB_ICONERROR);
            }
        }
    }

    CloseHandle(hPipe);
    return 0;
}

//---------------------------------------------------------------------------
// DumpToTempFallback - Fallback when no user session available
//---------------------------------------------------------------------------

static int DumpToTempFallback(void)
{
    WCHAR szPath[MAX_PATH];
    GenerateTempPath(szPath, g_szAppName, TRUE);
    return WriteDumpToFile(szPath, g_dwDumpType) ? 0 : 1;
}

//---------------------------------------------------------------------------
// WriteDumpToFile
//---------------------------------------------------------------------------

static BOOL WriteDumpToFile(LPCWSTR szPath, DWORD dwDumpType)
{
    HANDLE hProcess;
    HANDLE hFile;
    MINIDUMP_TYPE dumpFlags;
    MINIDUMP_EXCEPTION_INFORMATION mei;
    PMINIDUMP_EXCEPTION_INFORMATION pMei = NULL;
    BOOL success;

    // Open target process
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, g_dwPid);
    if (!hProcess) {
        return FALSE;
    }

    // Create output file
    hFile = CreateFileW(szPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        CloseHandle(hProcess);
        return FALSE;
    }

    // Determine dump flags
    switch (dwDumpType) {
        case DUMP_STANDARD:
            dumpFlags = (MINIDUMP_TYPE)DUMP_FLAGS_STANDARD;
            break;
        case DUMP_FULL:
            dumpFlags = (MINIDUMP_TYPE)DUMP_FLAGS_FULL;
            break;
        default:
            dumpFlags = (MINIDUMP_TYPE)DUMP_FLAGS_TRIAGE;
            break;
    }

    // Set up exception info if available
    if (g_pExceptionPointers) {
        mei.ThreadId = g_dwTid;
        mei.ExceptionPointers = (PEXCEPTION_POINTERS)g_pExceptionPointers;
        mei.ClientPointers = TRUE;
        pMei = &mei;
    }

    // Write dump
    success = MiniDumpWriteDump(hProcess, g_dwPid, hFile, dumpFlags, pMei, NULL, NULL);

    CloseHandle(hFile);
    CloseHandle(hProcess);

    if (!success) {
        DeleteFileW(szPath);
    }

    return success;
}

//---------------------------------------------------------------------------
// IsPathSafe - Validate path for Session 0 mode
//---------------------------------------------------------------------------

static BOOL IsPathSafe(LPCWSTR szPath)
{
    WCHAR szWindows[MAX_PATH];
    WCHAR szSystem32[MAX_PATH];
    WCHAR szProgramFiles[MAX_PATH];
    size_t len;
    DWORD attrs;

    if (!szPath || szPath[0] == L'\0') {
        return FALSE;
    }

    // Check not a device path
    if (wcsncmp(szPath, L"\\\\.\\", 4) == 0 || wcsncmp(szPath, L"\\\\?\\", 4) == 0) {
        return FALSE;
    }

    // Check not UNC path to avoid network writes
    if (wcsncmp(szPath, L"\\\\", 2) == 0) {
        return FALSE;
    }

    // Get Windows directory
    if (GetWindowsDirectoryW(szWindows, MAX_PATH)) {
        len = wcslen(szWindows);
        if (_wcsnicmp(szPath, szWindows, len) == 0 &&
            (szPath[len] == L'\\' || szPath[len] == L'\0')) {
            return FALSE;
        }
    }

    // Get System directory
    if (GetSystemDirectoryW(szSystem32, MAX_PATH)) {
        len = wcslen(szSystem32);
        if (_wcsnicmp(szPath, szSystem32, len) == 0 &&
            (szPath[len] == L'\\' || szPath[len] == L'\0')) {
            return FALSE;
        }
    }

    // Get Program Files directories
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROGRAM_FILES, NULL, 0, szProgramFiles))) {
        len = wcslen(szProgramFiles);
        if (_wcsnicmp(szPath, szProgramFiles, len) == 0 &&
            (szPath[len] == L'\\' || szPath[len] == L'\0')) {
            return FALSE;
        }
    }

    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROGRAM_FILESX86, NULL, 0, szProgramFiles))) {
        len = wcslen(szProgramFiles);
        if (_wcsnicmp(szPath, szProgramFiles, len) == 0 &&
            (szPath[len] == L'\\' || szPath[len] == L'\0')) {
            return FALSE;
        }
    }

    // Check file doesn't already exist
    attrs = GetFileAttributesW(szPath);
    if (attrs != INVALID_FILE_ATTRIBUTES) {
        return FALSE;  // File exists
    }

    return TRUE;
}

//---------------------------------------------------------------------------
// GenerateDefaultPath - Generate path in Documents folder
//---------------------------------------------------------------------------

static void GenerateDefaultPath(LPWSTR szPath, LPCWSTR szAppName)
{
    WCHAR szDocuments[MAX_PATH];
    SYSTEMTIME st;

    // Get Documents folder
    if (FAILED(SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, 0, szDocuments))) {
        // Fallback to temp
        GetTempPathW(MAX_PATH, szDocuments);
    }

    // Generate filename with timestamp
    GetLocalTime(&st);

    wsprintfW(szPath, L"%s\\%s_%04d-%02d-%02d_%02d-%02d-%02d.dmp",
              szDocuments, szAppName,
              st.wYear, st.wMonth, st.wDay,
              st.wHour, st.wMinute, st.wSecond);
}

//---------------------------------------------------------------------------
// GenerateTempPath - Generate path in temp folder
//---------------------------------------------------------------------------

static void GenerateTempPath(LPWSTR szPath, LPCWSTR szAppName, BOOL bSystemTemp)
{
    WCHAR szTempDir[MAX_PATH];
    SYSTEMTIME st;

    if (bSystemTemp) {
        // Windows temp folder
        GetWindowsDirectoryW(szTempDir, MAX_PATH);
        wcscat_s(szTempDir, MAX_PATH, L"\\Temp");
    } else {
        // User temp folder
        GetTempPathW(MAX_PATH, szTempDir);
        // Remove trailing backslash if present
        size_t len = wcslen(szTempDir);
        if (len > 0 && szTempDir[len - 1] == L'\\') {
            szTempDir[len - 1] = L'\0';
        }
    }

    // Generate filename with timestamp
    GetLocalTime(&st);

    wsprintfW(szPath, L"%s\\%s_%04d-%02d-%02d_%02d-%02d-%02d.dmp",
              szTempDir, szAppName,
              st.wYear, st.wMonth, st.wDay,
              st.wHour, st.wMinute, st.wSecond);
}

//---------------------------------------------------------------------------
// BrowseForSaveLocation
//---------------------------------------------------------------------------

static BOOL BrowseForSaveLocation(HWND hParent, LPWSTR szPath)
{
    OPENFILENAMEW ofn;
    WCHAR szFile[MAX_PATH];

    memset(&ofn, 0, sizeof(ofn));
    wcscpy_s(szFile, MAX_PATH, szPath);

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hParent;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Dump Files (*.dmp)\0*.dmp\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrDefExt = L"dmp";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

    if (GetSaveFileNameW(&ofn)) {
        wcscpy_s(szPath, MAX_PATH, szFile);
        return TRUE;
    }

    return FALSE;
}

//---------------------------------------------------------------------------
// GetMachineGuid - Read MachineGuid from registry
//---------------------------------------------------------------------------

static BOOL GetMachineGuid(LPWSTR szGuid, DWORD cchGuid)
{
    HKEY hKey = NULL;
    BOOL success = FALSE;
    DWORD dwType = 0;
    DWORD cbData;

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                      L"SOFTWARE\\Microsoft\\Cryptography",
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        cbData = cchGuid * sizeof(WCHAR);
        if (RegQueryValueExW(hKey, L"MachineGuid", NULL, &dwType,
                            (LPBYTE)szGuid, &cbData) == ERROR_SUCCESS) {
            success = TRUE;
        }
        RegCloseKey(hKey);
    }

    return success;
}

//---------------------------------------------------------------------------
// WideToUtf8 - Convert wide string to UTF-8
//---------------------------------------------------------------------------

static int WideToUtf8(LPCWSTR wstr, char* utf8, int utf8Size)
{
    if (!wstr) return 0;
    return WideCharToMultiByte(CP_UTF8, 0, wstr, -1, utf8, utf8Size, NULL, NULL);
}

//---------------------------------------------------------------------------
// SubmitCrashDump - Upload dump to server
//---------------------------------------------------------------------------

static BOOL SubmitCrashDump(LPCWSTR szDumpPath, LPCWSTR szComment)
{
    BOOL success = FALSE;
    HINTERNET hSession = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    BYTE* pFileData = NULL;
    DWORD dwFileSize = 0;
    DWORD dwRead = 0;
    WCHAR szMachineGuid[64];
    DWORD dwStatusCode = 0;
    DWORD dwSize = 0;
    char* pBody = NULL;

    szMachineGuid[0] = L'\0';

    // Read dump file
    hFile = CreateFileW(szDumpPath, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        goto cleanup;
    }

    dwFileSize = GetFileSize(hFile, NULL);
    if (dwFileSize == INVALID_FILE_SIZE || dwFileSize == 0) {
        goto cleanup;
    }

    pFileData = (BYTE*)malloc(dwFileSize);
    if (!pFileData) {
        goto cleanup;
    }

    if (!ReadFile(hFile, pFileData, dwFileSize, &dwRead, NULL) || dwRead != dwFileSize) {
        goto cleanup;
    }

    CloseHandle(hFile);
    hFile = INVALID_HANDLE_VALUE;

    // Get machine GUID
    GetMachineGuid(szMachineGuid, 64);

    // Open HTTP session
    hSession = WinHttpOpen(L"MiniDump/1.0",
                          WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                          WINHTTP_NO_PROXY_NAME,
                          WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {
        goto cleanup;
    }

    // Connect
    hConnect = WinHttpConnect(hSession, SUBMIT_URL_HOST, 443, 0);
    if (!hConnect) {
        goto cleanup;
    }

    // Open request
    hRequest = WinHttpOpenRequest(hConnect, L"POST", SUBMIT_URL_PATH,
                                  NULL, WINHTTP_NO_REFERER,
                                  WINHTTP_DEFAULT_ACCEPT_TYPES,
                                  WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        goto cleanup;
    }

    // Build multipart form data
    {
        char szBoundary[64];
        char szSoftwareUtf8[128];
        char szVersionUtf8[64];
        char szCommentUtf8[MAX_COMMENT_LENGTH * 3];
        char szGuidUtf8[128];
        char* pDash;
        WCHAR szContentType[128];
        size_t bodyCapacity;
        size_t bodyLen;

        wsprintfA(szBoundary, "----MiniDump%08X%08X", GetTickCount(), rand());

        wsprintfW(szContentType, L"Content-Type: multipart/form-data; boundary=%S", szBoundary);

        WinHttpAddRequestHeaders(hRequest, szContentType, (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD);

        // Build body
        bodyCapacity = dwFileSize + 8192;
        pBody = (char*)malloc(bodyCapacity);
        if (!pBody) {
            goto cleanup;
        }

        bodyLen = 0;

        // Field: report
        bodyLen += wsprintfA(pBody + bodyLen, "--%s\r\nContent-Disposition: form-data; name=\"report\"\r\n\r\n1\r\n", szBoundary);

        // Field: software
        WideToUtf8(g_szAppName, szSoftwareUtf8, sizeof(szSoftwareUtf8));
        // Remove version suffix for software name
        pDash = strchr(szSoftwareUtf8, '-');
        if (pDash) *pDash = '\0';
        bodyLen += wsprintfA(pBody + bodyLen, "--%s\r\nContent-Disposition: form-data; name=\"software\"\r\n\r\n%s\r\n", szBoundary, szSoftwareUtf8);

        // Field: version
        strcpy_s(szVersionUtf8, sizeof(szVersionUtf8), "unknown");
        if (g_szAppVersion[0]) {
            WideToUtf8(g_szAppVersion, szVersionUtf8, sizeof(szVersionUtf8));
        }
        bodyLen += wsprintfA(pBody + bodyLen, "--%s\r\nContent-Disposition: form-data; name=\"version\"\r\n\r\n%s\r\n", szBoundary, szVersionUtf8);

        // Field: comment
        WideToUtf8(szComment ? szComment : L"", szCommentUtf8, sizeof(szCommentUtf8));
        bodyLen += wsprintfA(pBody + bodyLen, "--%s\r\nContent-Disposition: form-data; name=\"comment\"\r\n\r\n%s\r\n", szBoundary, szCommentUtf8);

        // Field: randId
        WideToUtf8(szMachineGuid, szGuidUtf8, sizeof(szGuidUtf8));
        bodyLen += wsprintfA(pBody + bodyLen, "--%s\r\nContent-Disposition: form-data; name=\"randId\"\r\n\r\n%s\r\n", szBoundary, szGuidUtf8);

        // Field: dump (file)
        bodyLen += wsprintfA(pBody + bodyLen, "--%s\r\nContent-Disposition: form-data; name=\"dump\"; filename=\"MiniDump.dmp\"\r\nContent-Type: application/octet-stream\r\n\r\n", szBoundary);

        // Reallocate if needed
        if (bodyLen + dwFileSize + 64 > bodyCapacity) {
            char* pNewBody;
            bodyCapacity = bodyLen + dwFileSize + 64;
            pNewBody = (char*)realloc(pBody, bodyCapacity);
            if (!pNewBody) {
                goto cleanup;
            }
            pBody = pNewBody;
        }

        // Append file data
        memcpy(pBody + bodyLen, pFileData, dwFileSize);
        bodyLen += dwFileSize;

        // End boundary
        bodyLen += wsprintfA(pBody + bodyLen, "\r\n--%s--\r\n", szBoundary);

        // Send request
        if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                               pBody, (DWORD)bodyLen, (DWORD)bodyLen, 0)) {
            goto cleanup;
        }
    }

    // Receive response
    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        goto cleanup;
    }

    // Check status code
    dwSize = sizeof(dwStatusCode);
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                       NULL, &dwStatusCode, &dwSize, NULL);

    success = (dwStatusCode == 200);

cleanup:
    if (pBody) free(pBody);
    if (pFileData) free(pFileData);
    if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);

    return success;
}

//---------------------------------------------------------------------------
// UpdateSaveControls - Enable/disable save path controls
//---------------------------------------------------------------------------

static void UpdateSaveControls(HWND hDlg, BOOL bSaveEnabled)
{
    EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PATH), bSaveEnabled);
    EnableWindow(GetDlgItem(hDlg, IDC_BTN_BROWSE), bSaveEnabled);
}

//---------------------------------------------------------------------------
// CrashDumpDlgProc
//---------------------------------------------------------------------------

static INT_PTR CALLBACK CrashDumpDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static MINIDUMP_UI_RESPONSE* pResponse = NULL;

    switch (msg) {
    case WM_INITDIALOG:
        pResponse = (MINIDUMP_UI_RESPONSE*)lParam;

        // Set dialog icon (title bar and taskbar)
        SetDialogIcon(hDlg);

        // Load warning icon for the dialog content - use system warning icon
        {
            HICON hIcon = LoadIconW(NULL, IDI_WARNING);
            if (hIcon) {
                SendDlgItemMessageW(hDlg, IDC_ICON_WARNING, STM_SETICON, (WPARAM)hIcon, 0);
            }
        }

        // Set dialog title
        {
            WCHAR szTitle[128];
            wsprintfW(szTitle, L"Crash Dump - %s", g_szAppName);
            SetWindowTextW(hDlg, szTitle);
        }

        // Set info text
        {
            WCHAR szInfo[256];
            wsprintfW(szInfo, L"%s has crashed. A crash dump can be saved for debugging purposes.", g_szAppName);
            SetDlgItemTextW(hDlg, IDC_STATIC_INFO, szInfo);
        }

        // Comment edit starts disabled
        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_COMMENT), FALSE);

        // Populate dump type combo
        SendDlgItemMessageW(hDlg, IDC_COMBO_TYPE, CB_ADDSTRING, 0, (LPARAM)L"Triage (small, ~100 KB)");
        SendDlgItemMessageW(hDlg, IDC_COMBO_TYPE, CB_ADDSTRING, 0, (LPARAM)L"Standard (with heap, ~20 MB)");
        SendDlgItemMessageW(hDlg, IDC_COMBO_TYPE, CB_ADDSTRING, 0, (LPARAM)L"Full (complete memory, 100+ MB)");
        SendDlgItemMessageW(hDlg, IDC_COMBO_TYPE, CB_SETCURSEL, pResponse->dwDumpType, 0);

        // Save checkbox - default checked
        CheckDlgButton(hDlg, IDC_CHECK_SAVE, BST_CHECKED);

        // Set default path
        SetDlgItemTextW(hDlg, IDC_EDIT_PATH, pResponse->szPath);

        // Center dialog
        {
            RECT rc;
            GetWindowRect(hDlg, &rc);
            int x = (GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2;
            int y = (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2;
            SetWindowPos(hDlg, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }

        SetForegroundWindow(hDlg);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_CHECK_SUBMIT:
            {
                BOOL bChecked = (IsDlgButtonChecked(hDlg, IDC_CHECK_SUBMIT) == BST_CHECKED);
                EnableWindow(GetDlgItem(hDlg, IDC_EDIT_COMMENT), bChecked);
            }
            return TRUE;

        case IDC_CHECK_SAVE:
            {
                BOOL bChecked = (IsDlgButtonChecked(hDlg, IDC_CHECK_SAVE) == BST_CHECKED);
                UpdateSaveControls(hDlg, bChecked);
            }
            return TRUE;

        case IDC_BTN_BROWSE:
            {
                WCHAR szPath[MAX_PATH];
                GetDlgItemTextW(hDlg, IDC_EDIT_PATH, szPath, MAX_PATH);
                if (BrowseForSaveLocation(hDlg, szPath)) {
                    SetDlgItemTextW(hDlg, IDC_EDIT_PATH, szPath);
                }
            }
            return TRUE;

        case IDOK:
            pResponse->dwDumpType = (DWORD)SendDlgItemMessageW(hDlg, IDC_COMBO_TYPE, CB_GETCURSEL, 0, 0);
            pResponse->bSave = (IsDlgButtonChecked(hDlg, IDC_CHECK_SAVE) == BST_CHECKED);
            if (pResponse->bSave) {
                GetDlgItemTextW(hDlg, IDC_EDIT_PATH, pResponse->szPath, MAX_PATH);
            }
            pResponse->bSubmit = (IsDlgButtonChecked(hDlg, IDC_CHECK_SUBMIT) == BST_CHECKED);
            if (pResponse->bSubmit) {
                GetDlgItemTextW(hDlg, IDC_EDIT_COMMENT, pResponse->szComment, MAX_COMMENT_LENGTH);
            } else {
                pResponse->szComment[0] = L'\0';
            }
            EndDialog(hDlg, IDOK);
            return TRUE;

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;

    case WM_CLOSE:
        EndDialog(hDlg, IDCANCEL);
        return TRUE;
    }

    return FALSE;
}

//---------------------------------------------------------------------------
// CreateNamedPipeWithUserAccess
//---------------------------------------------------------------------------

static HANDLE CreateNamedPipeWithUserAccess(DWORD sessionId, LPWSTR szPipeName)
{
    HANDLE hUserToken = NULL;
    BYTE buffer[256];
    PTOKEN_USER pTokenUser;
    DWORD dwLen = 0;
    EXPLICIT_ACCESSW ea[2];
    PSID pSystemSid = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    PACL pAcl = NULL;
    SECURITY_DESCRIPTOR sd;
    SECURITY_ATTRIBUTES sa;
    HANDLE hPipe;

    // Generate pipe name
    wsprintfW(szPipeName, L"\\\\.\\pipe\\MiniDump_%08X", GetCurrentProcessId());

    // Get user token for the session
    if (!WTSQueryUserToken(sessionId, &hUserToken)) {
        return INVALID_HANDLE_VALUE;
    }

    // Get user SID
    pTokenUser = (PTOKEN_USER)buffer;
    if (!GetTokenInformation(hUserToken, TokenUser, pTokenUser, sizeof(buffer), &dwLen)) {
        CloseHandle(hUserToken);
        return INVALID_HANDLE_VALUE;
    }

    CloseHandle(hUserToken);

    // Build DACL
    memset(ea, 0, sizeof(ea));

    // SYSTEM full access
    if (!AllocateAndInitializeSid(&ntAuthority, 1, SECURITY_LOCAL_SYSTEM_RID,
            0, 0, 0, 0, 0, 0, 0, &pSystemSid)) {
        return INVALID_HANDLE_VALUE;
    }

    ea[0].grfAccessPermissions = GENERIC_ALL;
    ea[0].grfAccessMode = SET_ACCESS;
    ea[0].grfInheritance = NO_INHERITANCE;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea[0].Trustee.ptstrName = (LPWSTR)pSystemSid;

    // User read/write access
    ea[1].grfAccessPermissions = GENERIC_READ | GENERIC_WRITE;
    ea[1].grfAccessMode = SET_ACCESS;
    ea[1].grfInheritance = NO_INHERITANCE;
    ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[1].Trustee.TrusteeType = TRUSTEE_IS_USER;
    ea[1].Trustee.ptstrName = (LPWSTR)pTokenUser->User.Sid;

    if (SetEntriesInAclW(2, ea, NULL, &pAcl) != ERROR_SUCCESS) {
        FreeSid(pSystemSid);
        return INVALID_HANDLE_VALUE;
    }

    // Create security descriptor
    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        LocalFree(pAcl);
        FreeSid(pSystemSid);
        return INVALID_HANDLE_VALUE;
    }

    if (!SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE)) {
        LocalFree(pAcl);
        FreeSid(pSystemSid);
        return INVALID_HANDLE_VALUE;
    }

    memset(&sa, 0, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = &sd;
    sa.bInheritHandle = FALSE;

    // Create pipe
    hPipe = CreateNamedPipeW(
        szPipeName,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1,                  // Max instances
        sizeof(MINIDUMP_RESULT),
        sizeof(MINIDUMP_UI_RESPONSE),
        30000,              // Timeout ms
        &sa
    );

    LocalFree(pAcl);
    FreeSid(pSystemSid);

    return hPipe;
}

//---------------------------------------------------------------------------
// SpawnUiProcess
//---------------------------------------------------------------------------

static BOOL SpawnUiProcess(DWORD sessionId, LPCWSTR pipeName)
{
    const DWORD TOKEN_RIGHTS = TOKEN_QUERY | TOKEN_DUPLICATE
                             | TOKEN_ADJUST_DEFAULT | TOKEN_ADJUST_SESSIONID
                             | TOKEN_ADJUST_GROUPS | TOKEN_ASSIGN_PRIMARY;
    HANDLE hUserToken = NULL;
    HANDLE hNewToken = NULL;
    WCHAR cmdLine[512];
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    BOOL ok;

    // Get user token for the session
    if (!WTSQueryUserToken(sessionId, &hUserToken)) {
        return FALSE;
    }

    // Duplicate token
    if (!DuplicateTokenEx(hUserToken, TOKEN_RIGHTS, NULL, SecurityAnonymous,
                          TokenPrimary, &hNewToken)) {
        CloseHandle(hUserToken);
        return FALSE;
    }

    CloseHandle(hUserToken);

    // Build command line
    wsprintfW(cmdLine, L"\"%s\" /ui /pipe:%s /name:%s /type:%lu",
              g_szExePath, pipeName, g_szAppName, g_dwDumpType);

    // Set up startup info for user desktop
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    si.lpDesktop = (LPWSTR)L"WinSta0\\Default";
    si.dwFlags = STARTF_FORCEOFFFEEDBACK;

    memset(&pi, 0, sizeof(pi));
    ok = CreateProcessAsUserW(
        hNewToken,
        NULL,
        cmdLine,
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi
    );

    CloseHandle(hNewToken);

    if (ok) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }

    return ok;
}

//---------------------------------------------------------------------------
// SetDialogIcon - Set dialog icon helper
//---------------------------------------------------------------------------

static void SetDialogIcon(HWND hDlg)
{
    HICON hIconBig = (HICON)LoadImageW(g_hInstance, MAKEINTRESOURCEW(IDI_CRASHDUMP),
                                       IMAGE_ICON, GetSystemMetrics(SM_CXICON),
                                       GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
    HICON hIconSmall = (HICON)LoadImageW(g_hInstance, MAKEINTRESOURCEW(IDI_CRASHDUMP),
                                         IMAGE_ICON, GetSystemMetrics(SM_CXSMICON),
                                         GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
    if (hIconBig) {
        SendMessageW(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
    }
    if (hIconSmall) {
        SendMessageW(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
    }
}

//---------------------------------------------------------------------------
// WriteDumpForProcess - Write dump for a running process (no exception)
//---------------------------------------------------------------------------

static BOOL WriteDumpForProcess(DWORD dwPid, LPCWSTR szPath, DWORD dwDumpType)
{
    HANDLE hProcess;
    HANDLE hFile;
    MINIDUMP_TYPE dumpFlags;
    BOOL success;

    // Open target process
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPid);
    if (!hProcess) {
        return FALSE;
    }

    // Create output file
    hFile = CreateFileW(szPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        CloseHandle(hProcess);
        return FALSE;
    }

    // Determine dump flags
    switch (dwDumpType) {
        case DUMP_STANDARD:
            dumpFlags = (MINIDUMP_TYPE)DUMP_FLAGS_STANDARD;
            break;
        case DUMP_FULL:
            dumpFlags = (MINIDUMP_TYPE)DUMP_FLAGS_FULL;
            break;
        default:
            dumpFlags = (MINIDUMP_TYPE)DUMP_FLAGS_TRIAGE;
            break;
    }

    // Write dump without exception info
    success = MiniDumpWriteDump(hProcess, dwPid, hFile, dumpFlags, NULL, NULL, NULL);

    CloseHandle(hFile);
    CloseHandle(hProcess);

    if (!success) {
        DeleteFileW(szPath);
    }

    return success;
}

//---------------------------------------------------------------------------
// PopulateProcessList - Fill ListView with running processes
//---------------------------------------------------------------------------

static void PopulateProcessList(HWND hListView)
{
    HANDLE hSnapshot;
    PROCESSENTRY32W pe32;
    LVITEMW lvi;
    WCHAR szPid[16];
    int nItem;

    // Clear existing items
    ListView_DeleteAllItems(hListView);

    // Take snapshot of all processes
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return;
    }

    pe32.dwSize = sizeof(pe32);

    if (Process32FirstW(hSnapshot, &pe32)) {
        do {
            // Skip System Idle Process
            if (pe32.th32ProcessID == 0) {
                continue;
            }

            // Try to open the process to check if it's accessible
            HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                       FALSE, pe32.th32ProcessID);
            BOOL bAccessible = (hProc != NULL);
            if (hProc) {
                CloseHandle(hProc);
            }

            // Add to list
            memset(&lvi, 0, sizeof(lvi));
            lvi.mask = LVIF_TEXT | LVIF_PARAM;
            lvi.iItem = ListView_GetItemCount(hListView);
            lvi.pszText = pe32.szExeFile;
            lvi.lParam = (LPARAM)pe32.th32ProcessID;
            nItem = ListView_InsertItem(hListView, &lvi);

            // Set PID column
            wsprintfW(szPid, L"%lu", pe32.th32ProcessID);
            ListView_SetItemText(hListView, nItem, 1, szPid);

            // Set accessible column
            ListView_SetItemText(hListView, nItem, 2, bAccessible ? (LPWSTR)L"Yes" : (LPWSTR)L"No");

        } while (Process32NextW(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
}

//---------------------------------------------------------------------------
// ProcessListDlgProc - Process list dialog procedure
//---------------------------------------------------------------------------

static INT_PTR CALLBACK ProcessListDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HWND hListView = NULL;

    switch (msg) {
    case WM_INITDIALOG:
        {
            SetDialogIcon(hDlg);

            // Get ListView handle
            hListView = GetDlgItem(hDlg, IDC_LIST_PROCESSES);

            // Set extended styles
            ListView_SetExtendedListViewStyle(hListView,
                LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);

            // Add columns
            LVCOLUMNW lvc;
            memset(&lvc, 0, sizeof(lvc));
            lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

            lvc.iSubItem = 0;
            lvc.cx = 200;
            lvc.pszText = (LPWSTR)L"Process Name";
            ListView_InsertColumn(hListView, 0, &lvc);

            lvc.iSubItem = 1;
            lvc.cx = 70;
            lvc.pszText = (LPWSTR)L"PID";
            ListView_InsertColumn(hListView, 1, &lvc);

            lvc.iSubItem = 2;
            lvc.cx = 70;
            lvc.pszText = (LPWSTR)L"Access";
            ListView_InsertColumn(hListView, 2, &lvc);

            // Populate dump type combo
            SendDlgItemMessageW(hDlg, IDC_COMBO_TYPE, CB_ADDSTRING, 0, (LPARAM)L"Triage (small, ~100 KB)");
            SendDlgItemMessageW(hDlg, IDC_COMBO_TYPE, CB_ADDSTRING, 0, (LPARAM)L"Standard (with heap, ~20 MB)");
            SendDlgItemMessageW(hDlg, IDC_COMBO_TYPE, CB_ADDSTRING, 0, (LPARAM)L"Full (complete memory, 100+ MB)");
            SendDlgItemMessageW(hDlg, IDC_COMBO_TYPE, CB_SETCURSEL, DUMP_TRIAGE, 0);

            // Populate process list
            PopulateProcessList(hListView);

            // Center dialog
            RECT rc;
            GetWindowRect(hDlg, &rc);
            int x = (GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2;
            int y = (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2;
            SetWindowPos(hDlg, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BTN_REFRESH:
            PopulateProcessList(hListView);
            return TRUE;

        case IDC_BTN_DUMP:
            {
                // Get selected item
                int nSel = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
                if (nSel == -1) {
                    MessageBoxW(hDlg, L"Please select a process first.",
                                L"Process Dump", MB_ICONINFORMATION);
                    return TRUE;
                }

                // Get process ID from lParam
                LVITEMW lvi;
                memset(&lvi, 0, sizeof(lvi));
                lvi.mask = LVIF_PARAM;
                lvi.iItem = nSel;
                ListView_GetItem(hListView, &lvi);
                DWORD dwPid = (DWORD)lvi.lParam;

                // Get process name for filename
                WCHAR szProcName[MAX_PATH];
                ListView_GetItemText(hListView, nSel, 0, szProcName, MAX_PATH);

                // Remove .exe extension if present
                WCHAR* pExt = wcsrchr(szProcName, L'.');
                if (pExt && _wcsicmp(pExt, L".exe") == 0) {
                    *pExt = L'\0';
                }

                // Get dump type
                DWORD dwDumpType = (DWORD)SendDlgItemMessageW(hDlg, IDC_COMBO_TYPE,
                                                              CB_GETCURSEL, 0, 0);

                // Generate default path
                WCHAR szPath[MAX_PATH];
                GenerateDefaultPath(szPath, szProcName);

                // Show save dialog
                if (BrowseForSaveLocation(hDlg, szPath)) {
                    // Create dump
                    SetCursor(LoadCursor(NULL, IDC_WAIT));
                    BOOL bSuccess = WriteDumpForProcess(dwPid, szPath, dwDumpType);
                    SetCursor(LoadCursor(NULL, IDC_ARROW));

                    if (bSuccess) {
                        WCHAR szMsg[MAX_PATH + 64];
                        wsprintfW(szMsg, L"Dump created successfully:\n%s", szPath);
                        MessageBoxW(hDlg, szMsg, L"Process Dump", MB_ICONINFORMATION);
                    } else {
                        DWORD dwErr = GetLastError();
                        WCHAR szMsg[256];
                        wsprintfW(szMsg, L"Failed to create dump.\nError code: %lu", dwErr);
                        MessageBoxW(hDlg, szMsg, L"Process Dump Error", MB_ICONERROR);
                    }
                }
            }
            return TRUE;

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;

    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;
            if (pnmh->idFrom == IDC_LIST_PROCESSES && pnmh->code == NM_DBLCLK) {
                // Double-click acts like Dump button
                SendMessageW(hDlg, WM_COMMAND, MAKEWPARAM(IDC_BTN_DUMP, BN_CLICKED), 0);
                return TRUE;
            }
        }
        break;

    case WM_CLOSE:
        EndDialog(hDlg, IDCANCEL);
        return TRUE;
    }

    return FALSE;
}

//---------------------------------------------------------------------------
// DoStandaloneMode - Show process list when no parameters given
//---------------------------------------------------------------------------

static int DoStandaloneMode(void)
{
    DialogBoxParamW(g_hInstance, MAKEINTRESOURCE(IDD_PROCESSLIST),
                    NULL, ProcessListDlgProc, 0);
    return 0;
}
