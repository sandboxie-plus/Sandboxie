/*
 * Copyright 2022 David Xanatos, xanasoft.com
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
// User Proxy Server
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "PipeServer.h"
#include "UserServer.h"
#include "QueueWire.h"
#include "UserWire.h"
#include "core/dll/sbiedll.h"
#include "core/drv/api_defs.h"
#include "common/my_version.h"
#include "common/program_control_rule.h"
#include <stdlib.h>
#include <sddl.h>
#include <aclapi.h>
#include <wtsapi32.h>
#include <userenv.h>
#include <shellapi.h>
#include "misc.h"
#include "core/drv/verify.h"

#define PATTERN XPATTERN
extern "C" {
#include "common/pattern.h"
} // extern "C"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define MAX_RPL_BUF_SIZE    32768


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _USER_WORKER {

    LIST_ELEM list_elem;

    HANDLE hProcess;

    ULONG session_id;

} USER_WORKER;


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------

static int UserServer_MatchImage(const WCHAR* pattern, const WCHAR* imageName, void* context)
{
    (void)context;
    return SbieDll_MatchImage(pattern, imageName, NULL) ? 1 : 0;
}

static void UserServer_AdjustFolderRule(WCHAR* value, void* context)
{
    (void)context;
    if (value && *value != L'*')
        SbieDll_TranslateNtToDosPath(value);
}

static WCHAR* UserServer_MatchImageAndGetValue(WCHAR* value, const WCHAR* imageName, ULONG* pLevel)
{
    return ProgramControl_MatchImageScopeAndGetValueEx(
        value,
        imageName,
        UserServer_MatchImage,
        NULL,
        (unsigned long*)pLevel);
}

static BOOLEAN UserServer_UseRuleExtensions(const WCHAR* boxname, const WCHAR* setting)
{
    if (!ProgramControl_IsRuleExtensionSetting(setting))
        return TRUE;

    return SbieApi_QueryConfBool(boxname, L"UseForceBreakoutRuleExtensions", FALSE) ? TRUE : FALSE;
}

static BOOLEAN UserServer_GetBreakoutDocumentTarget(
    const WCHAR* boxname, const WCHAR* imageName, const WCHAR* path, ULONG length,
    WCHAR* outTarget, ULONG outTargetCch, LONG* outPriority)
{
    WCHAR conf_buf[2048];
    WCHAR* path_lwr;
    POOL* pool;
    ULONG best_level = (ULONG)-1;
    BOOLEAN has_match = FALSE;
    BOOLEAN has_target = FALSE;
    BOOLEAN best_has_priority = FALSE;
    LONG best_priority = -1;
    ULONG index = 0;
    ULONG path_len = (length + 1) * sizeof(WCHAR);
    BOOLEAN use_rule_extensions;

    if (!boxname || !imageName || !path || !outTarget || outTargetCch == 0)
        return FALSE;

    use_rule_extensions = UserServer_UseRuleExtensions(boxname, L"BreakoutDocument");

    outTarget[0] = L'\0';
    if (outPriority)
        *outPriority = -1;

    path_lwr = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, path_len);
    if (!path_lwr)
        return FALSE;

    memcpy(path_lwr, path, path_len);
    path_lwr[length] = L'\0';
    _wcslwr(path_lwr);

    pool = Pool_Create();
    if (!pool) {
        HeapFree(GetProcessHeap(), 0, path_lwr);
        return FALSE;
    }

    while (1) {

        NTSTATUS status = SbieApi_QueryConf(
            boxname, L"BreakoutDocument", index, conf_buf, sizeof(conf_buf) - 16 * sizeof(WCHAR));
        if (!NT_SUCCESS(status)) {
            if (status == STATUS_BUFFER_TOO_SMALL) {
                ++index;
                continue;
            }
            break;
        }
        ++index;

        ULONG level = (ULONG)-1;
        WCHAR* value = UserServer_MatchImageAndGetValue(conf_buf, imageName, &level);
        if (!value)
            continue;

        SBIE_NORMALIZED_RULE normalized;
        if (!ProgramControl_ParseRuleExtensionsInPlace(value, &normalized, use_rule_extensions ? 1 : 0))
            continue;

        value = normalized.base_rule;
        if (*value != L'*')
            SbieDll_TranslateNtToDosPath(value);

        PATTERN* pattern = Pattern_Create(pool, value, TRUE, level);
        if (!pattern)
            continue;

        if (Pattern_Match(pattern, path_lwr, length)) {
            if (ProgramControl_ShouldReplaceTargetMatch(
                    has_match,
                    best_has_priority,
                    best_priority,
                    best_level,
                    normalized.has_priority,
                    normalized.priority,
                    level)) {
                has_match = TRUE;
                best_has_priority = normalized.has_priority ? TRUE : FALSE;
                best_priority = normalized.priority;
                best_level = level;

                if (outPriority)
                    *outPriority = normalized.has_priority ? normalized.priority : -1;

                if (normalized.has_target_box && normalized.target_box && *normalized.target_box) {
                    wcscpy_s(outTarget, outTargetCch, normalized.target_box);
                    has_target = TRUE;
                }
                else {
                    outTarget[0] = L'\0';
                    has_target = FALSE;
                }
            }
        }

        Pattern_Free(pattern);
    }

    Pool_Delete(pool);
    HeapFree(GetProcessHeap(), 0, path_lwr);
    return has_target;
}

// Scans ForceFolder rules to find if any match the directory containing path.
// Returns TRUE if a match is found, and sets *outPriority to the Priority=N
// value of the best-matching rule (-1 if no Priority= was specified).
static BOOLEAN UserServer_GetForceFolderPriority(
    const WCHAR* boxname, const WCHAR* imageName, const WCHAR* path, ULONG path_len,
    LONG* outPriority)
{
    ULONG dir_len;
    WCHAR* dir_path;
    BOOLEAN use_rule_extensions;
    BOOLEAN has_priority = FALSE;
    LONG best_priority = -1;

    // Find the directory length (characters before the last backslash).
    dir_len = 0;
    {
        const WCHAR* p = path;
        const WCHAR* end = path + path_len;
        while (p < end && *p) {
            if (*p == L'\\')
                dir_len = (ULONG)(p - path);
            ++p;
        }
    }

    if (!dir_len)
        return FALSE;

    use_rule_extensions = UserServer_UseRuleExtensions(boxname, L"ForceFolder");

    if (outPriority)
        *outPriority = -1;

    dir_path = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, (dir_len + 1) * sizeof(WCHAR));
    if (!dir_path)
        return FALSE;

    memcpy(dir_path, path, dir_len * sizeof(WCHAR));
    dir_path[dir_len] = L'\0';

    BOOLEAN found = ProgramControl_CheckFolderSettingMatchFromConf(
        boxname,
        L"ForceFolder",
        imageName,
        dir_path,
        dir_len,
        1,
        &has_priority,
        &best_priority,
        use_rule_extensions ? 1 : 0,
        UserServer_MatchImage,
        NULL,
        UserServer_AdjustFolderRule,
        NULL) ? TRUE : FALSE;

    if (outPriority)
        *outPriority = has_priority ? best_priority : -1;

    HeapFree(GetProcessHeap(), 0, dir_path);
    return found;
}

// Scans BreakoutFolder rules to find if any match the handler directory.
// Returns TRUE if a match is found, and sets *outPriority to the Priority=N
// value of the best-matching rule (-1 if no Priority= was specified).
static BOOLEAN UserServer_GetBreakoutFolderPriority(
    const WCHAR* boxname, const WCHAR* imageName, const WCHAR* appPath, ULONG appDirLen,
    LONG* outPriority)
{
    BOOLEAN use_rule_extensions;
    BOOLEAN has_priority = FALSE;
    LONG best_priority = -1;
    BOOLEAN found;

    if (outPriority)
        *outPriority = -1;

    if (!boxname || !imageName || !*imageName || !appPath || !*appPath || !appDirLen)
        return FALSE;

    use_rule_extensions = UserServer_UseRuleExtensions(boxname, L"BreakoutFolder");

    found = ProgramControl_CheckFolderSettingMatchFromConf(
        boxname,
        L"BreakoutFolder",
        imageName,
        appPath,
        appDirLen,
        1,
        &has_priority,
        &best_priority,
        use_rule_extensions ? 1 : 0,
        UserServer_MatchImage,
        NULL,
        UserServer_AdjustFolderRule,
        NULL) ? TRUE : FALSE;

    if (outPriority)
        *outPriority = has_priority ? best_priority : -1;

    return found;
}

static BOOLEAN UserServer_GetBreakoutFolderTarget(
    const WCHAR* boxname, const WCHAR* imageName, const WCHAR* appPath, ULONG appDirLen,
    WCHAR* outTarget, ULONG outTargetCch)
{
    BOOLEAN use_rule_extensions;

    if (!outTarget || outTargetCch == 0)
        return FALSE;

    outTarget[0] = L'\0';

    if (!boxname || !imageName || !*imageName || !appPath || !*appPath || !appDirLen)
        return FALSE;

    use_rule_extensions = UserServer_UseRuleExtensions(boxname, L"BreakoutFolder");

    return ProgramControl_GetFolderTargetFromConf(
        boxname,
        imageName,
        appPath,
        appDirLen,
        outTarget,
        outTargetCch,
        NULL,
        NULL,
        use_rule_extensions ? 1 : 0,
        UserServer_MatchImage,
        NULL,
        UserServer_AdjustFolderRule,
        NULL) ? TRUE : FALSE;
}

// Scans ForceProcess rules to check if any match the given image name.
// Returns TRUE if a match is found, and sets *outPriority to the Priority=N
// value of the best-matching rule (-1 if no Priority= was specified).
static BOOLEAN UserServer_QueryImageRuleMatch(
    const WCHAR* boxname, const WCHAR* setting, const WCHAR* imageName,
    LONG* outPriority, WCHAR* outTarget, ULONG outTargetCch)
{
    WCHAR conf_buf[2048];
    BOOLEAN found = FALSE;
    BOOLEAN best_has_priority = FALSE;
    LONG best_priority = -1;
    ULONG index = 0;
    BOOLEAN use_rule_extensions;

    if (!boxname || !setting || !imageName)
        return FALSE;

    use_rule_extensions = UserServer_UseRuleExtensions(boxname, setting);

    if (outPriority)
        *outPriority = -1;
    if (outTarget && outTargetCch > 0)
        outTarget[0] = L'\0';

    while (1) {

        NTSTATUS status = SbieApi_QueryConf(
            boxname, setting, index, conf_buf, sizeof(conf_buf) - 16 * sizeof(WCHAR));
        if (!NT_SUCCESS(status)) {
            if (status == STATUS_BUFFER_TOO_SMALL) {
                ++index;
                continue;
            }
            break;
        }
        ++index;

        SBIE_NORMALIZED_RULE normalized;
        if (!ProgramControl_ParseRuleExtensionsInPlace(conf_buf, &normalized, use_rule_extensions ? 1 : 0))
            continue;

        if (!SbieDll_MatchImage(normalized.base_rule, imageName, NULL))
            continue;

        if (ProgramControl_ShouldReplaceTargetMatch(
                found,
                best_has_priority,
                best_priority,
                2,
                normalized.has_priority,
                normalized.priority,
                2)) {
            found = TRUE;
            best_has_priority = normalized.has_priority ? TRUE : FALSE;
            best_priority = normalized.priority;

            if (outTarget && outTargetCch > 0) {
                if (normalized.has_target_box && normalized.target_box && *normalized.target_box)
                    wcscpy_s(outTarget, outTargetCch, normalized.target_box);
                else
                    outTarget[0] = L'\0';
            }
        }
    }

    if (found && outPriority)
        *outPriority = best_has_priority ? best_priority : -1;

    return found;
}

static BOOLEAN UserServer_GetForceProcessPriority(
    const WCHAR* boxname, const WCHAR* imageName, LONG* outPriority)
{
    return UserServer_QueryImageRuleMatch(
        boxname, L"ForceProcess", imageName, outPriority, NULL, 0);
}

// Scans BreakoutProcess rules to check if any match the given image name.
// Returns TRUE if a match is found, and sets *outPriority to the Priority=N
// value of the best-matching rule (-1 if no Priority= was specified).
static BOOLEAN UserServer_GetBreakoutProcessPriority(
    const WCHAR* boxname, const WCHAR* imageName, LONG* outPriority,
    WCHAR* outTarget, ULONG outTargetCch)
{
    return UserServer_QueryImageRuleMatch(
        boxname, L"BreakoutProcess", imageName, outPriority, outTarget, outTargetCch);
}

static BOOLEAN UserServer_IsPrimaryPreferredByPriority(
    LONG primaryPriority,
    LONG secondaryPriority,
    BOOLEAN preferPrimaryOnTie)
{
    // Generic priority comparison helper.
    // Example 1: primary=2, secondary=5 -> TRUE (lower numeric priority wins).
    // Example 2: primary=-1, secondary=0 -> FALSE (explicit priority beats unset).
    // Example 3: primary=-1, secondary=-1, preferPrimaryOnTie=TRUE -> TRUE (tie goes primary).
    BOOLEAN primary_has_priority = (primaryPriority >= 0) ? TRUE : FALSE;
    BOOLEAN secondary_has_priority = (secondaryPriority >= 0) ? TRUE : FALSE;

    if (primary_has_priority && secondary_has_priority) {
        if (primaryPriority < secondaryPriority)
            return TRUE;
        if (primaryPriority > secondaryPriority)
            return FALSE;
        return preferPrimaryOnTie ? TRUE : FALSE;
    }

    if (primary_has_priority != secondary_has_priority)
        return primary_has_priority ? TRUE : FALSE;

    return preferPrimaryOnTie ? TRUE : FALSE;
}

static void UserServer_SelectBreakoutProcessFolderWinner(
    BOOLEAN bp_match,
    LONG bp_priority,
    const WCHAR* bp_targetBox,
    BOOLEAN bf_match,
    LONG bf_priority,
    BOOLEAN bf_has_target,
    const WCHAR* bf_targetBox,
    BOOLEAN* outMatch,
    LONG* outPriority,
    BOOLEAN* outHasTarget,
    WCHAR* outTargetBox,
    ULONG outTargetCch)
{
    if (!outMatch || !outPriority || !outHasTarget || !outTargetBox || outTargetCch == 0)
        return;

    *outMatch = FALSE;
    *outPriority = -1;
    *outHasTarget = FALSE;
    outTargetBox[0] = L'\0';

    if (!bp_match && !bf_match)
        return;

    *outMatch = TRUE;

    BOOLEAN winner_is_bp;
    if (bp_match && bf_match)
        winner_is_bp = UserServer_IsPrimaryPreferredByPriority(bp_priority, bf_priority, TRUE);
    else
        winner_is_bp = bp_match ? TRUE : FALSE;

    if (winner_is_bp) {
        *outPriority = bp_priority;
        if (bp_targetBox && *bp_targetBox) {
            *outHasTarget = TRUE;
            wcscpy_s(outTargetBox, outTargetCch, bp_targetBox);
        }
    }
    else {
        *outPriority = bf_priority;
        if (bf_has_target && bf_targetBox && *bf_targetBox) {
            *outHasTarget = TRUE;
            wcscpy_s(outTargetBox, outTargetCch, bf_targetBox);
        }
    }
}

static ULONG UserServer_MapLaunchFailureStatusFromLastError(void)
{
    DWORD err = GetLastError();

    if (err == ERROR_FILE_NOT_FOUND)
        return STATUS_OBJECT_NAME_NOT_FOUND;
    if (err == ERROR_PATH_NOT_FOUND || err == ERROR_INVALID_DRIVE)
        return STATUS_OBJECT_PATH_NOT_FOUND;
    if (err == ERROR_NO_ASSOCIATION)
        return (ULONG)HRESULT_FROM_WIN32(ERROR_NO_ASSOCIATION);

    return STATUS_UNSUCCESSFUL;
}

static BOOLEAN UserServer_ShouldFallbackToSourceOnLaunchFailure(ULONG status)
{
    return (
        status == STATUS_OBJECT_NAME_NOT_FOUND ||
        status == STATUS_OBJECT_PATH_NOT_FOUND ||
        status == (ULONG)HRESULT_FROM_WIN32(ERROR_NO_ASSOCIATION)
    ) ? TRUE : FALSE;
}

static BOOLEAN UserServer_IsLikelyFileSystemPath(const WCHAR* path)
{
    if (!path || !*path)
        return FALSE;

    // Drive-qualified DOS path, e.g. C:\foo
    if ((((path[0] >= L'A' && path[0] <= L'Z') || (path[0] >= L'a' && path[0] <= L'z')) &&
         path[1] == L':' &&
         (path[2] == L'\\' || path[2] == L'/')))
        return TRUE;

    // UNC path, e.g. \\server\share
    if ((path[0] == L'\\' && path[1] == L'\\') ||
        (path[0] == L'/' && path[1] == L'/'))
        return TRUE;

    // NT-style path prefixes used by some callers.
    if ((path[0] == L'\\' && path[1] == L'?' && path[2] == L'?' && path[3] == L'\\') ||
        (path[0] == L'\\' && path[1] == L'?' && path[2] == L'\\'))
        return TRUE;

    return FALSE;
}

static BOOLEAN UserServer_ShouldFallbackToSourceOnHostMissingPath(const WCHAR* path)
{
    DWORD attrs;
    DWORD err;

    if (!UserServer_IsLikelyFileSystemPath(path))
        return FALSE;

    attrs = GetFileAttributesW(path);
    if (attrs != INVALID_FILE_ATTRIBUTES)
        return FALSE;

    err = GetLastError();
    return (err == ERROR_FILE_NOT_FOUND ||
            err == ERROR_PATH_NOT_FOUND ||
            err == ERROR_INVALID_DRIVE) ? TRUE : FALSE;
}

static ULONG UserServer_OpenDocumentInTargetBox(const WCHAR* targetBox, const WCHAR* path, const WCHAR* lpDirectory)
{
    const DWORD TOKEN_RIGHTS = TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_ADJUST_DEFAULT | TOKEN_ADJUST_SESSIONID | TOKEN_ADJUST_GROUPS;
    WCHAR homePath[MAX_PATH];
    WCHAR startExe[MAX_PATH];
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    WCHAR* cmdline;
    size_t cmdlineCch;
    HANDLE hOldToken = NULL;
    HANDLE hNewToken = NULL;
    LPVOID env = NULL;
    BOOL ok;
    LONG status;

    status = SbieApi_GetHomePath(NULL, 0, homePath, MAX_PATH);
    if (status != 0)
        return status;

    if (swprintf_s(startExe, _countof(startExe), L"%s\\%s", homePath, START_EXE) < 0)
        return STATUS_BUFFER_TOO_SMALL;

    cmdlineCch = wcslen(startExe) + wcslen(targetBox) + wcslen(path) + 16;
    cmdline = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, cmdlineCch * sizeof(WCHAR));
    if (!cmdline)
        return STATUS_INSUFFICIENT_RESOURCES;

    if (swprintf_s(
            cmdline, cmdlineCch, L"\"%s\" /box:%s \"%s\"", startExe, targetBox, path) < 0) {
        HeapFree(GetProcessHeap(), 0, cmdline);
        return STATUS_BUFFER_TOO_SMALL;
    }

    ok = OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_DUPLICATE, &hOldToken);
    if (!ok) {
        HeapFree(GetProcessHeap(), 0, cmdline);
        return STATUS_ACCESS_DENIED;
    }

    ok = DuplicateTokenEx(
            hOldToken, TOKEN_RIGHTS, NULL, SecurityAnonymous, TokenPrimary, &hNewToken);
    if (!ok) {
        CloseHandle(hOldToken);
        HeapFree(GetProcessHeap(), 0, cmdline);
        return STATUS_ACCESS_DENIED;
    }

    ok = CreateEnvironmentBlock(&env, hNewToken, FALSE);
    if (!ok)
        env = NULL;

    memzero(&si, sizeof(STARTUPINFO));
    memzero(&pi, sizeof(PROCESS_INFORMATION));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_FORCEOFFFEEDBACK;
    si.lpDesktop = L"WinSta0\\Default";

    DWORD flags = 0;
    if (env)
        flags |= CREATE_UNICODE_ENVIRONMENT;
    ok = CreateProcessAsUser(
        hNewToken, startExe, cmdline, NULL, NULL, FALSE,
        flags, env, (lpDirectory && lpDirectory[0]) ? lpDirectory : homePath, &si, &pi);

    if (env)
        DestroyEnvironmentBlock(env);
    CloseHandle(hNewToken);
    CloseHandle(hOldToken);

    HeapFree(GetProcessHeap(), 0, cmdline);
    if (!ok)
        return UserServer_MapLaunchFailureStatusFromLastError();

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return STATUS_SUCCESS;
}

// Opens a document unboxed by adding this worker process to the DFP
// (Disable Force Process) list, then calling ShellExecuteEx. Any document
// handler (e.g. notepad++.exe) spawned as a direct child inherits DFP via
// Process_NotifyProcess_Create, so ForceProcess rules are skipped for it.
static ULONG UserServer_OpenDocumentUnboxed(const WCHAR* path, const WCHAR* lpDirectory)
{
    // Add this worker process to DFP. When ShellExecuteEx creates the document
    // handler (e.g. notepad++.exe) as a direct child, the driver propagates DFP
    // to it and skips any matching ForceProcess rules.
    ULONG dfp_state = DISABLE_JUST_THIS_PROCESS;
    SbieApi_DisableForceProcess(&dfp_state, NULL);

    SHELLEXECUTEINFO shex;
    memzero(&shex, sizeof(SHELLEXECUTEINFO));
    shex.cbSize = sizeof(SHELLEXECUTEINFO);
    shex.fMask = 0;
    shex.hwnd = NULL;
    shex.lpVerb = L"open";
    shex.lpFile = path;
    shex.lpParameters = NULL;
    shex.lpDirectory = lpDirectory;
    shex.nShow = SW_SHOWNORMAL;
    shex.hInstApp = NULL;

    typedef BOOL (*P_ShellExecuteEx)(void *);
    HMODULE shell32 = GetModuleHandleW(L"shell32.dll");
    if (!shell32)
        shell32 = LoadLibraryW(L"shell32.dll");
    P_ShellExecuteEx pShellExecuteEx = shell32
        ? (P_ShellExecuteEx)GetProcAddress(shell32, "ShellExecuteExW")
        : NULL;
    if (!pShellExecuteEx)
        return STATUS_ENTRYPOINT_NOT_FOUND;

    if (pShellExecuteEx(&shex))
        return STATUS_SUCCESS;
    return UserServer_MapLaunchFailureStatusFromLastError();
}

//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


UserServer::UserServer()
{
    InitializeCriticalSection(&m_WorkersLock);
    List_Init(&m_WorkersList);
    m_QueueEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    // worker data
    m_QueueName = NULL;
    m_ParentPid = 0;
    m_SessionId = 0;
}

UserServer::~UserServer()
{
	// cleanup CS
	DeleteCriticalSection(&m_WorkersLock);
}


//---------------------------------------------------------------------------
// UserServer
//---------------------------------------------------------------------------


UserServer *UserServer::GetInstance()
{
    static UserServer *_instance = NULL;
    if (! _instance)
        _instance = new UserServer();
    return _instance;
}


//---------------------------------------------------------------------------
// StartWorker
//---------------------------------------------------------------------------

volatile HANDLE UserServer__hParentProcess = NULL;

_FX VOID UserServer__APC(ULONG_PTR hParent)
{
    UserServer__hParentProcess = (HANDLE)hParent;
}

ULONG UserServer::StartWorker(ULONG session_id)
{
    const ULONG TOKEN_RIGHTS = TOKEN_QUERY          | TOKEN_DUPLICATE
                             | TOKEN_ADJUST_DEFAULT | TOKEN_ADJUST_SESSIONID
                             | TOKEN_ADJUST_GROUPS  | TOKEN_ASSIGN_PRIMARY;
    HANDLE hOldToken = NULL;
    HANDLE hNewToken = NULL;
    LPVOID env = NULL;
    ULONG status;
    BOOL ok = TRUE;

    //
    // terminate an existing worker process that stopped functioning
    //

    USER_WORKER *worker = (USER_WORKER *)List_Head(&m_WorkersList);
    while (worker) {

        USER_WORKER *worker_next = (USER_WORKER *)List_Next(worker);

        if (worker->session_id == session_id) {

            TerminateProcess(worker->hProcess, 1);
            CloseHandle(worker->hProcess);

            List_Remove(&m_WorkersList, worker);
            HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, worker);
        }

        worker = worker_next;
    }

    //
    // build the command line for the User Worker Proxy Server Process
    //

    WCHAR EventName[96];

    WCHAR *cmdline = (WCHAR *)HeapAlloc(
                                GetProcessHeap(), 0, 768 * sizeof(WCHAR));
    if (! cmdline)
        return STATUS_INSUFFICIENT_RESOURCES;

    cmdline[0] = L'\"';
    status = SbieApi_GetHomePath(NULL, 0, &cmdline[1], 512);
    if (status != 0)
        ok = FALSE;
    else {

        WCHAR *cmdptr = cmdline + wcslen(cmdline);
        wcscpy(cmdptr, L"\\" SBIESVC_EXE L"\" ");
        cmdptr += wcslen(cmdptr);
        wsprintf(cmdptr, L"%s_UserProxy_%08X,%d",
            SANDBOXIE, session_id, GetCurrentProcessId());

        wcscpy(EventName, L"Global\\");
        wcscat(EventName, cmdptr);
    }

    //
    // use the users security token for the worker
    //

    if (ok) {
        //ok = OpenProcessToken(GetCurrentProcess(), TOKEN_RIGHTS, &hOldToken);
        ok = WTSQueryUserToken(session_id, &hOldToken);
        if (! ok)
            status = 0x72000000 | GetLastError();
    }

    if (ok) {
        ok = DuplicateTokenEx(
                hOldToken, TOKEN_RIGHTS, NULL, SecurityAnonymous,
                TokenPrimary, &hNewToken);
        if (! ok)
            status = 0x73000000 | GetLastError();
    }

    if (ok) {
        ok = CreateEnvironmentBlock(&env, hNewToken, FALSE);
        if (!ok)
            env = NULL;
    }

    //if (ok) {
    //    ok = SetTokenInformation(
    //            hNewToken, TokenSessionId, &session_id, sizeof(ULONG));
    //    if (! ok)
    //        status = 0x74000000 | GetLastError();
    //}

    //
    // create an event object for the new User Worker process
    // the user process needs to be able to set this event
    // so set the appropriate security descriptor
    //

    SECURITY_DESCRIPTOR sd;
    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);

    SECURITY_ATTRIBUTES sa = {0};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = FALSE;
    sa.lpSecurityDescriptor = &sd;

    HANDLE EventHandle = CreateEvent(&sa, TRUE, FALSE, EventName);
    if (EventHandle)
        ResetEvent(EventHandle);
    else {
        status = 0x75000000 | GetLastError();
        ok = FALSE;
    }

    //
    // create the new process
    //

    if (ok) {

        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        memzero(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);
        si.dwFlags = STARTF_FORCEOFFFEEDBACK;
        si.lpDesktop = L"WinSta0\\Default";

        DWORD flags = ABOVE_NORMAL_PRIORITY_CLASS;
        if (env)
            flags |= CREATE_UNICODE_ENVIRONMENT;

        ok = CreateProcessAsUser(
            hNewToken, NULL, cmdline, NULL, NULL, FALSE,
            flags, env, NULL, &si, &pi);
        if (! ok)
            status = 0x76000000 | GetLastError();
        else {

            //
            // wait for the new process to signal the event
            //

            HANDLE WaitHandles[2];
            WaitHandles[0] = EventHandle;
            WaitHandles[1] = pi.hProcess;

            status = WaitForMultipleObjects(2, WaitHandles, FALSE, 15 * 1000);
            if (status != WAIT_OBJECT_0) {
                status = 0x77000000 | status;
                ok = FALSE;

            } else {

                //
                // create a new worker process element
                //

                worker = (USER_WORKER *)HeapAlloc(
                                        GetProcessHeap(), 0, sizeof(USER_WORKER));
                if (! worker) {

                    status = STATUS_INSUFFICIENT_RESOURCES;
                    ok = FALSE;

                } else {

                    worker->session_id = session_id;
                    worker->hProcess = pi.hProcess;

                    List_Insert_After(&m_WorkersList, NULL, worker);

                    status = 0;
                }
            }

            //
            // since the worker is running as user it can't open this service process, even for SYNCHRONIZE only
            // hence we duplicate the required token and use APC to pass it to our new worker.
            //

            HANDLE hThis;
            if(NT_SUCCESS(DuplicateHandle(NtCurrentProcess(), NtCurrentProcess(), pi.hProcess, &hThis, SYNCHRONIZE, FALSE, 0)))
                QueueUserAPC(UserServer__APC, pi.hThread, (ULONG_PTR)hThis);

            CloseHandle(pi.hThread);
            if (! ok)
                CloseHandle(pi.hProcess);
        }
    }

    if (EventHandle)
        CloseHandle(EventHandle);
    if (env)
        DestroyEnvironmentBlock(env);
    if (hNewToken)
        CloseHandle(hNewToken);
    if (hOldToken)
        CloseHandle(hOldToken);
    HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, cmdline);

    return status;
}

static SBIE_POLICY_DECISION UserServer_ResolveDocumentPolicy(
    BOOLEAN breakout_has_target,
    int force_process_match_input,
    int force_folder_match_input,
    BOOLEAN force_has_priority,
    LONG force_priority,
    BOOLEAN breakout_has_priority,
    LONG breakout_priority)
{
    SBIE_RULE_MATCH_SET matches;
    SBIE_POLICY_INPUT in;

    memzero(&matches, sizeof(matches));
    memzero(&in, sizeof(in));

    // force_process_match_input: set by caller based on whether the caller's image
    // (the document handler that will be re-launched) matches any ForceProcess rule.
    // force_folder_match_input: set by caller using the scope-aware ForceFolder check
    // (UserServer_GetForceFolderPriority) so image-scoped rules are respected.
    matches.force_process_match = force_process_match_input;
    matches.force_folder_match = force_folder_match_input;
    matches.force_children_match = 0;
    matches.breakout_process_match = 0;
    matches.breakout_folder_match = 0;
    matches.breakout_document_match = 1;
    matches.breakout_has_target = breakout_has_target ? 1 : 0;

    in.context_kind = SBIE_CTX_SANDBOXED_DOCUMENT_OPEN;
    in.caller_forced_by_children = 0;
    in.source_equals_candidate_box = 1;

    // Legacy default for BreakoutDocument (1.17.5): breakout wins when matched.
    // Priority-aware behavior overrides this default; equal-priority ties fall back
    // to this same legacy ordering.
    return SbiePolicy_ResolveWithPriorities(
        &in,
        &matches,
        TRUE,
        force_has_priority ? 1 : 0,
        force_priority,
        breakout_has_priority ? 1 : 0,
        breakout_priority);
}


//---------------------------------------------------------------------------
// StartAsync
//---------------------------------------------------------------------------

struct SStartupParam
{
    ULONG session_id;
    HANDLE hEvent;
};

ULONG UserServer__StartupWorker(void* _Param)
{
    SStartupParam* pParam = (SStartupParam*)_Param;

    //
    // thart the proxy process
    //

    UserServer::GetInstance()->StartWorker(pParam->session_id);

    //
    // notify the requesting party that the server is now up and running
    //

    SetEvent(pParam->hEvent);

    HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, pParam);
    return 0;
}

ULONG UserServer::StartAsync(ULONG session_id, HANDLE hEvent)
{
    SStartupParam* pParam = (SStartupParam*)HeapAlloc(GetProcessHeap(), 0, sizeof(SStartupParam));
    pParam->session_id = session_id;
    pParam->hEvent = hEvent;

    HANDLE hThread = CreateThread(NULL, 0, UserServer__StartupWorker, (void *)pParam, 0, NULL);
    if (!hThread) {
        HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, pParam);
        return STATUS_UNSUCCESSFUL;
    }
    CloseHandle(hThread);
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// ReportError2336
//---------------------------------------------------------------------------


void UserServer::ReportError2336(ULONG session_id, ULONG errlvl, ULONG status)
{
    SbieApi_LogEx(session_id, 2336, L"[%02X / %08X]", errlvl, status);
}


//---------------------------------------------------------------------------
//
// Worker Process
//
//---------------------------------------------------------------------------


typedef struct USER_JOB {

    LIST_ELEM list_elem;
    HANDLE handle;

} USER_JOB;


//---------------------------------------------------------------------------
// RunWorker
//---------------------------------------------------------------------------


void UserServer::RunWorker(const WCHAR *cmdline)
{
    //
    // select between a normal SbieSVC User Worker
    //

    UserServer *pThis = GetInstance();

    //
    // get process id for parent (which should be the main SbieSvc process)
    //

    NTSTATUS status;
    ULONG len;
    PROCESS_BASIC_INFORMATION info;

    status = NtQueryInformationProcess(
        NtCurrentProcess(), ProcessBasicInformation,
        &info, sizeof(PROCESS_BASIC_INFORMATION), &len);

    if (! NT_SUCCESS(status))
        return;

    pThis->m_ParentPid = (ULONG)info.InheritedFromUniqueProcessId;
    if (! pThis->m_ParentPid)
        return;

    //
    // create message queue and process incoming requests
    //

    if (! pThis->CreateQueueWorker(cmdline))
        return;

    //
    // exit when parent dies
    //

    while (UserServer__hParentProcess == NULL)
        SleepEx(10, TRUE); // be in a waitable state for he APC's

    //HANDLE hParentProcess =
    //                OpenProcess(SYNCHRONIZE, FALSE, pThis->m_ParentPid);
    //if (! hParentProcess)
    //    hParentProcess = NtCurrentProcess();
    status = WaitForSingleObject(UserServer__hParentProcess, INFINITE);
    if (status == WAIT_OBJECT_0)
        ExitProcess(0);
}


//---------------------------------------------------------------------------
// CreateQueueWorker
//---------------------------------------------------------------------------


bool UserServer::CreateQueueWorker(const WCHAR *cmdline)
{
    //
    // create a queue with the queue manager
    //

    WCHAR *ptr = (WCHAR*)wcsstr(cmdline, L"_UserProxy");
    if (! ptr)
        return false;
    ULONG len = (wcslen(ptr) + 1) * sizeof(WCHAR);
    m_QueueName = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, len);
    memcpy(m_QueueName, ptr, len);
    *m_QueueName = L'*';
    _wcsupr(m_QueueName);

    m_SessionId = wcstol(m_QueueName + 11, &ptr, 16);
    if (*ptr != L',')
        return false;
    *ptr = L'\0';           // terminate queue name

    m_ParentPid = wcstol(ptr + 1, &ptr, 10);
    if (*ptr != L'\0')
        return false;

    ULONG status = SbieDll_QueueCreate(m_QueueName, &m_QueueEvent);
    if (status != 0)
        return false;

    //
    // signal the event object
    //

    WCHAR EventName[96];
    wcscpy(EventName, L"Global\\");
    wcscat(EventName, cmdline);
    HANDLE EventHandle = OpenEvent(EVENT_MODIFY_STATE, FALSE, EventName);
    if (EventHandle) {
        SetEvent(EventHandle);
        CloseHandle(EventHandle);
    }

    //
    // prepare the dispatch table for incoming requests
    //

    const ULONG m_WorkerFuncs_len =
                    sizeof(WorkerFunc) * (USER_MAX_REQUEST_CODE + 4);
    m_WorkerFuncs = (WorkerFunc *)
                    HeapAlloc(GetProcessHeap(), 0, m_WorkerFuncs_len);
    memzero(m_WorkerFuncs, m_WorkerFuncs_len);

    m_WorkerFuncs[USER_OPEN_FILE]          = &UserServer::OpenFile;
    m_WorkerFuncs[USER_SHELL_EXEC]         = &UserServer::OpenDocument;
    


    //
    // register a worker thread to process incoming queue requests
    //

    HANDLE WaitHandle;
    if (! RegisterWaitForSingleObject(&WaitHandle, m_QueueEvent,
                                      QueueCallbackWorker, (void *)this,
                                      INFINITE, WT_EXECUTEDEFAULT))
        return false;

    return true;
}


//---------------------------------------------------------------------------
// QueueCallbackWorker
//---------------------------------------------------------------------------


void UserServer::QueueCallbackWorker(void *arg, BOOLEAN timeout)
{
    UserServer *pThis = (UserServer *)arg;
    while (1) {
        bool check_for_more_requests = pThis->QueueCallbackWorker2();
        if (! check_for_more_requests)
            break;
    }
}


//---------------------------------------------------------------------------
// QueueCallbackWorker2
//---------------------------------------------------------------------------


bool UserServer::QueueCallbackWorker2(void)
{
    //
    // get next request
    //
    // note that STATUS_END_OF_FILE here indicates there are no more requests
    // in the queue at this time and we should go resume waiting on the event
    //

    WorkerArgs args;
    ULONG request_id;
    ULONG data_len;
    ULONG rpl_len;
    void *data_ptr;
    ULONG rpl_buf[MAX_RPL_BUF_SIZE / sizeof(ULONG)];

    ULONG status = SbieDll_QueueGetReq(m_QueueName, &args.pid, NULL,
                                       &request_id, &data_ptr, &data_len);
    if (status != 0) {
        if (status != STATUS_END_OF_FILE)
            ReportError2336(-1, 0x81, status);
        return false;
    }

    //
    // process request
    //

    status = STATUS_INVALID_SYSTEM_SERVICE;
    rpl_len = sizeof(ULONG);

    ULONG msgid = *(ULONG *)data_ptr;

    if (msgid < USER_MAX_REQUEST_CODE) {

        WorkerFunc WorkerFuncPtr = m_WorkerFuncs[msgid];
        if (WorkerFuncPtr) {

            bool issue_request = true;

            //
            // issue request
            //

            if (issue_request) {

                args.req_len = data_len;
                args.req_buf = data_ptr;
                args.rpl_len = rpl_len;
                args.rpl_buf = rpl_buf;

                status = (this->*WorkerFuncPtr)(&args);
                if (status == 0)
                    rpl_len = args.rpl_len;
            }
        }
    }

    //
    // send reply
    //
    // note that STATUS_END_OF_FILE here indicates the calling process is no
    // longer there, in which case we still return true to process any other
    // requests from other processes which may be in the queue
    //

    *rpl_buf = status;
    status = SbieDll_QueuePutRpl(
                            m_QueueName, request_id, rpl_buf, rpl_len);

    SbieDll_FreeMem(data_ptr);

    if (status != 0 && status != STATUS_END_OF_FILE) {
        ReportError2336(-1, 0x82, status);
        return false;
    }

    return true;
}


//---------------------------------------------------------------------------
// OpenFile
//---------------------------------------------------------------------------


ULONG UserServer::OpenFile(WorkerArgs *args)
{
    USER_OPEN_FILE_REQ *req = (USER_OPEN_FILE_REQ *)args->req_buf;
    USER_OPEN_FILE_RPL *rpl = (USER_OPEN_FILE_RPL *)args->rpl_buf;

    if (args->req_len < sizeof(USER_OPEN_FILE_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    WCHAR* path_buff = (WCHAR*)(((UCHAR*)req) + req->FileNameOffset);

    //
    // check if the caller belongs to our session
    //

    ULONG session_id;
    WCHAR boxname[BOXNAME_COUNT];
    if (!NT_SUCCESS(SbieApi_QueryProcess((HANDLE)(ULONG_PTR)args->pid, boxname, NULL, NULL, &session_id))
     || session_id != m_SessionId 
     || !SbieApi_QueryConfBool(boxname, L"EnableEFS", FALSE)) {

        return STATUS_ACCESS_DENIED;
    }

    __declspec(align(8)) SCertInfo CertInfo = { 0 };
    if (!NT_SUCCESS(SbieApi_QueryDrvInfo(-1, &CertInfo, sizeof(CertInfo))) || !(CertInfo.active && CertInfo.opt_enc)) {
        const WCHAR* strings[] = { boxname, L"EnableEFS", NULL };
        SbieApi_LogMsgExt(session_id, 6004, strings);
        return STATUS_ACCESS_DENIED;
    } 

    //
    // check if operation is permitted, it must be for a file on a disk
    // and the file access rules must allow for the access
    //

    if(_wcsnicmp(path_buff, L"\\Device\\HarddiskVolume", 22) != 0)
        return STATUS_ACCESS_DENIED;
    
    BOOL write_access = FALSE;

#define FILE_DENIED_ACCESS ~(                                           \
    STANDARD_RIGHTS_READ | GENERIC_READ | SYNCHRONIZE | READ_CONTROL |  \
    FILE_READ_DATA | FILE_READ_EA | FILE_READ_ATTRIBUTES | FILE_EXECUTE)
            
    if (req->DesiredAccess & FILE_DENIED_ACCESS)
        write_access = TRUE;
            
    if (req->CreateDisposition != FILE_OPEN)
        write_access = TRUE;

    if (req->CreateOptions & FILE_DELETE_ON_CLOSE)
        write_access = TRUE;

#undef FILE_DENIED_ACCESS

    POOL *pool;
#ifdef USE_MATCH_PATH_EX
    LIST *normal_list, *open_list, *closed_list, *write_list, *read_list;
#else
    LIST *open_list, *closed_list, *write_list;
#endif

    if (!NT_SUCCESS(GetProcessPathList('fo', args->pid, (void **)&pool, &open_list))
     || !NT_SUCCESS(GetProcessPathList('fc', args->pid, (void **)&pool, &closed_list))
     || !NT_SUCCESS(GetProcessPathList('fr', args->pid, (void **)&pool, &write_list))
#ifdef USE_MATCH_PATH_EX
     || !NT_SUCCESS(GetProcessPathList('fn', args->pid, (void **)&pool, &normal_list))
     || !NT_SUCCESS(GetProcessPathList('fw', args->pid, (void **)&pool, &read_list))
#endif
        )  
        return STATUS_INTERNAL_ERROR;

#ifdef USE_MATCH_PATH_EX
    ULONG64 Dll_ProcessFlags = SbieApi_QueryProcessInfo((HANDLE)args->pid, 0);

    BOOLEAN use_rule_specificity = (Dll_ProcessFlags & SBIE_FLAG_RULE_SPECIFICITY) != 0;
    //BOOLEAN use_privacy_mode = (Dll_ProcessFlags & SBIE_FLAG_PRIVACY_MODE) != 0;

    //ULONG mp_flags = SbieDll_MatchPathImpl(use_rule_specificity, use_privacy_mode, path_buff, normal_list, open_list, closed_list, write_list, read_list);
    ULONG mp_flags = SbieDll_MatchPathImpl(use_rule_specificity, path_buff, normal_list, open_list, closed_list, write_list, read_list);
#else
    ULONG mp_flags = SbieDll_MatchPathImpl(path_buff, open_list, closed_list, write_list);
#endif

    Pool_Delete(pool);

    if(write_access && (!PATH_IS_OPEN(mp_flags) || PATH_IS_READ(mp_flags)))
        return STATUS_ACCESS_DENIED;
    if(PATH_IS_CLOSED(mp_flags) || PATH_IS_WRITE(mp_flags))
        return STATUS_ACCESS_DENIED;

    //
    // open the file on behalf of the caller
    //

    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    RtlInitUnicodeString(&objname, path_buff);

    LARGE_INTEGER AllocSize;
    AllocSize.QuadPart = req->AllocationSize;

    void* pEaBuff = NULL;
    if (req->EaBufferOffset != 0)
        pEaBuff = ((UCHAR*)req) + req->EaBufferOffset;

    HANDLE hFile;
    IO_STATUS_BLOCK IoStatusBlock;
    rpl->error = NtCreateFile(&hFile, req->DesiredAccess, &objattrs, &IoStatusBlock, AllocSize.QuadPart != 0 ? &AllocSize : NULL, 
        req->FileAttributes, req->ShareAccess, req->CreateDisposition, req->CreateOptions, pEaBuff, req->EaLength);
    rpl->Status = IoStatusBlock.Status;
    rpl->Information = IoStatusBlock.Information;

    if (NT_SUCCESS(rpl->error)) {

        //
        // duplicate the handle into the calling process, and close our own
        //

        HANDLE hProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, args->pid);
        if (hProcess) {
            DuplicateHandle(NtCurrentProcess(), hFile, hProcess, (HANDLE*)&rpl->FileHandle, req->DesiredAccess, FALSE, 0);
            CloseHandle(hProcess);
        }
        else
            rpl->error = STATUS_UNSUCCESSFUL;

        NtClose(hFile);
    }

    args->rpl_len = sizeof(USER_OPEN_FILE_RPL);
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// OpenFile
//---------------------------------------------------------------------------


// Fallback flag written to rpl_buf[1]: non-zero tells the DLL to fall back to
// opening the document in the source box instead of treating the call as handled.
#define USER_DOCUMENT_FALLBACK_TO_SOURCE 1

ULONG UserServer::OpenDocument(WorkerArgs *args)
{
    USER_SHELL_EXEC_REQ *req = (USER_SHELL_EXEC_REQ *)args->req_buf;

    if (args->req_len < sizeof(USER_SHELL_EXEC_REQ))
        return STATUS_INFO_LENGTH_MISMATCH;

    WCHAR* path_buff = (WCHAR*)(((UCHAR*)req) + req->FileNameOffset);

    // Always send two ULONGs: rpl_buf[0] = status, rpl_buf[1] = fallback flag.
    args->rpl_len = 2 * sizeof(ULONG);
    ((ULONG*)args->rpl_buf)[1] = 0;

    //
    // check if the caller belongs to our session
    //

    ULONG session_id;
    WCHAR boxname[BOXNAME_COUNT];
    WCHAR image[96];
    WCHAR sid[96];
    if (!NT_SUCCESS(SbieApi_QueryProcess((HANDLE)(ULONG_PTR)args->pid, boxname, image, sid, &session_id))
     || session_id != m_SessionId) {

        return STATUS_ACCESS_DENIED;
    }

    //
    // check the BreakoutDocument list and execute if ok
    //

    ULONG path_len = (ULONG)wcslen(path_buff);
    if (SbieDll_CheckPatternInList(path_buff, path_len, boxname, L"BreakoutDocument")) {

        LONG bd_priority = -1;
        LONG bf_priority = -1;
        LONG ff_priority = -1;
        LONG fp_priority = -1;
        LONG bp_priority = -1;

        WCHAR targetBox[BOXNAME_COUNT] = { 0 };
        BOOLEAN has_target = UserServer_GetBreakoutDocumentTarget(
            boxname, image, path_buff, path_len, targetBox, BOXNAME_COUNT, &bd_priority);

        // ForceFolder: check if document's directory matches any ForceFolder rule.
        // Capture the match result for use as force_folder_match (scope-aware).
        BOOLEAN ff_match = UserServer_GetForceFolderPriority(boxname, image, path_buff, path_len, &ff_priority);

        // ForceProcess: the document handler (e.g., notepad++.exe) is what ForceProcess
        // targets, not the caller (e.g., explorer.exe). Resolve the actual handler for
        // the document extension via shlwapi AssocQueryStringW so the priority comparison
        // uses the correct image name. Fall back to the caller's image if resolution fails.
        WCHAR handlerImage[96] = { 0 };
        WCHAR handlerPathBuf[MAX_PATH] = { 0 };
        const WCHAR* fpCheckImage = image;
        {
            typedef HRESULT (WINAPI *P_AssocQueryStringW)(DWORD, DWORD, LPCWSTR, LPCWSTR, LPWSTR, DWORD*);
            HMODULE hShlwapi = GetModuleHandleW(L"shlwapi.dll");
            if (!hShlwapi)
                hShlwapi = LoadLibraryW(L"shlwapi.dll");
            if (hShlwapi) {
                P_AssocQueryStringW pAssocQuery = (P_AssocQueryStringW)GetProcAddress(hShlwapi, "AssocQueryStringW");
                if (pAssocQuery) {
                    DWORD len = 0;
                    // ASSOCF_NONE = 0, ASSOCSTR_EXECUTABLE = 2
                    HRESULT hr = pAssocQuery(0, 2, path_buff, NULL, NULL, &len);
                    if ((hr == E_POINTER || hr == S_FALSE) && len > 0) {
                        WCHAR* handlerPath = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, len * sizeof(WCHAR));
                        if (handlerPath) {
                            hr = pAssocQuery(0, 2, path_buff, NULL, handlerPath, &len);
                            if (SUCCEEDED(hr) && handlerPath[0]) {
                                wcscpy_s(handlerPathBuf, _countof(handlerPathBuf), handlerPath);
                                const WCHAR* slash = wcsrchr(handlerPath, L'\\');
                                if (wcscpy_s(handlerImage, _countof(handlerImage), slash ? slash + 1 : handlerPath) == 0)
                                    fpCheckImage = handlerImage;
                            }
                            HeapFree(GetProcessHeap(), 0, handlerPath);
                        }
                    }
                }
            }
        }
        BOOLEAN fp_match = UserServer_GetForceProcessPriority(boxname, fpCheckImage, &fp_priority);

        // BreakoutFolder: check the resolved handler path against BreakoutFolder
        // rules, scoped by the source process image (same as process-start folder scope).
        WCHAR bf_targetBox[BOXNAME_COUNT] = { 0 };
        BOOLEAN bf_match = FALSE;
        BOOLEAN bf_has_target = FALSE;
        if (handlerPathBuf[0]) {
            const WCHAR* handlerSlash = wcsrchr(handlerPathBuf, L'\\');
            if (handlerSlash && handlerSlash > handlerPathBuf) {
                ULONG handlerDirLen = (ULONG)(handlerSlash - handlerPathBuf);
                bf_match = UserServer_GetBreakoutFolderPriority(
                    boxname,
                    image,
                    handlerPathBuf,
                    handlerDirLen,
                    &bf_priority);
                bf_has_target = UserServer_GetBreakoutFolderTarget(
                    boxname,
                    image,
                    handlerPathBuf,
                    handlerDirLen,
                    bf_targetBox,
                    BOXNAME_COUNT);
            }
        }

        // BreakoutProcess: check if the handler image also matches a BreakoutProcess
        // rule.  When BreakoutProcess has a lower priority number than BreakoutDocument,
        // BP wins the breakout-side contest and its TargetBox (if any) is used.
        WCHAR bp_targetBox[BOXNAME_COUNT] = { 0 };
        BOOLEAN bp_match = UserServer_GetBreakoutProcessPriority(boxname, fpCheckImage, &bp_priority, bp_targetBox, BOXNAME_COUNT);

        // First arbitrate within process-start breakout rules: BreakoutProcess vs BreakoutFolder.
        // Tie behavior follows process-start semantics: BreakoutProcess wins equal priority.
        BOOLEAN breakout_pf_match = FALSE;
        LONG breakout_pf_priority = -1;
        BOOLEAN breakout_pf_has_target = FALSE;
        WCHAR breakout_pf_targetBox[BOXNAME_COUNT] = { 0 };

        UserServer_SelectBreakoutProcessFolderWinner(
            bp_match,
            bp_priority,
            bp_targetBox,
            bf_match,
            bf_priority,
            bf_has_target,
            bf_targetBox,
            &breakout_pf_match,
            &breakout_pf_priority,
            &breakout_pf_has_target,
            breakout_pf_targetBox,
            BOXNAME_COUNT);

        // Arbitrate within the breakout side: BreakoutDocument vs the winning
        // process-start breakout rule (BreakoutProcess or BreakoutFolder).
        // No explicit priority (= -1) loses to any explicit priority (>= 0).
        // On equal priority, BreakoutDocument wins. Both unset priorities are
        // treated as an equal-priority tie.
        BOOLEAN bd_wins = TRUE;
        if (breakout_pf_match)
            bd_wins = UserServer_IsPrimaryPreferredByPriority(bd_priority, breakout_pf_priority, TRUE);

        // The effective breakout priority for comparison against force rules.
        LONG breakout_winner_priority = bd_wins ? bd_priority : breakout_pf_priority;

        // Apply target from the breakout-side winner only.
        BOOLEAN has_target_effective = FALSE;
        if (bd_wins)
            has_target_effective = has_target ? TRUE : FALSE;
        else if (breakout_pf_has_target && breakout_pf_targetBox[0] != L'\0') {
            wcscpy_s(targetBox, BOXNAME_COUNT, breakout_pf_targetBox);
            has_target_effective = TRUE;
        }

        // Combined force priority: take the most specific (lowest number) among
        // matching ForceFolder and ForceProcess rules that carry explicit Priority=N.
        LONG force_priority = -1;
        BOOLEAN force_has_priority = FALSE;
        if (ff_priority >= 0) {
            force_has_priority = TRUE;
            force_priority = ff_priority;
        }
        if (fp_priority >= 0) {
            if (!force_has_priority || fp_priority < force_priority) {
                force_has_priority = TRUE;
                force_priority = fp_priority;
            }
        }
        BOOLEAN breakout_has_priority = (breakout_winner_priority >= 0);

        // Default: breakout wins only when TargetBox is explicitly specified.
        // For BREAKOUT_UNBOXED (no TargetBox), ForceProcess/ForceFolder dominates
        // unless BreakoutDocument has an explicit lower Priority=N to override it.
        // has_target_effective and breakout_winner_priority reflect the breakout-side
        // winner across BreakoutDocument, BreakoutProcess, and BreakoutFolder.
        SBIE_POLICY_DECISION decision = UserServer_ResolveDocumentPolicy(
            has_target_effective,
            fp_match ? 1 : 0,        // force_process_match: handler image vs ForceProcess rules
            ff_match ? 1 : 0,        // force_folder_match: scope-aware result from GetForceFolderPriority
            force_has_priority,
            force_priority,
            breakout_has_priority,
            breakout_winner_priority);

        if (decision == SBIE_DECISION_FORCE_SAME_BOX ||
            decision == SBIE_DECISION_FORCE_OTHER_BOX ||
            decision == SBIE_DECISION_DENY_FALLBACK ||
            decision == SBIE_DECISION_NO_MATCH) {
            // Policy says keep document in source box; signal fallback without logging an error.
            ((ULONG*)args->rpl_buf)[1] = USER_DOCUMENT_FALLBACK_TO_SOURCE;
            return STATUS_SUCCESS;
        }

        // For box-only files (missing on host), breakout launch from the service
        // cannot resolve the document path. Fall back to source-box handling.
        if (UserServer_ShouldFallbackToSourceOnHostMissingPath(path_buff)) {
            ((ULONG*)args->rpl_buf)[1] = USER_DOCUMENT_FALLBACK_TO_SOURCE;
            return STATUS_SUCCESS;
        }

        // When BreakoutUseTargetDir is set, use the document's parent directory
        // as the working directory for the launched viewer, not the Launcher's home path.
        WCHAR doc_dir[MAX_PATH] = { 0 };
        if (SbieDll_GetSettingsForName_bool(boxname, image, L"BreakoutUseTargetDir", FALSE)) {
            const WCHAR* last_sep = wcsrchr(path_buff, L'\\');
            if (last_sep && last_sep > path_buff) {
                ULONG dir_len = (ULONG)(last_sep - path_buff + 1);
                if (dir_len < MAX_PATH) {
                    wmemcpy(doc_dir, path_buff, dir_len);
                    doc_dir[dir_len] = L'\0';
                }
            }
        }

        if (decision == SBIE_DECISION_BREAKOUT_TARGET_BOX) {

            if (!NT_SUCCESS(SbieApi_Call(
                    API_IS_BOX_ENABLED, 3,
                    (ULONG_PTR)targetBox, (ULONG_PTR)sid, (ULONG_PTR)session_id))) {
                // Target box not available; signal fallback without logging SBIE2203.
                ((ULONG*)args->rpl_buf)[1] = USER_DOCUMENT_FALLBACK_TO_SOURCE;
                return STATUS_SUCCESS;
            }

            ULONG launch_status = UserServer_OpenDocumentInTargetBox(targetBox, path_buff, doc_dir[0] ? doc_dir : NULL);
            if (!NT_SUCCESS(launch_status) && UserServer_ShouldFallbackToSourceOnLaunchFailure(launch_status)) {
                ((ULONG*)args->rpl_buf)[1] = USER_DOCUMENT_FALLBACK_TO_SOURCE;
                return STATUS_SUCCESS;
            }
            return launch_status;
        }

        // BREAKOUT_UNBOXED: open the document with DFP set on this worker so
        // the document handler inherits DFP and is not re-forced by ForceProcess.
        ULONG launch_status = UserServer_OpenDocumentUnboxed(path_buff, doc_dir[0] ? doc_dir : NULL);
        if (!NT_SUCCESS(launch_status) && UserServer_ShouldFallbackToSourceOnLaunchFailure(launch_status)) {
            ((ULONG*)args->rpl_buf)[1] = USER_DOCUMENT_FALLBACK_TO_SOURCE;
            return STATUS_SUCCESS;
        }
        return launch_status;
    }

    return STATUS_ACCESS_DENIED;
}


//---------------------------------------------------------------------------
// GetProcessPathList
//---------------------------------------------------------------------------


ULONG UserServer::GetProcessPathList(ULONG path_code,
    ULONG pid, void **out_pool, LIST **out_list)
{
    const HANDLE xpid = (HANDLE)(ULONG_PTR)pid;

    ULONG len;
    LONG status = SbieApi_QueryPathList(path_code, &len, NULL, xpid, TRUE);
    if (status != 0)
        return status;

    status = STATUS_INSUFFICIENT_RESOURCES;

    POOL *pool = Pool_Create();
    if (! pool)
        return status;

    WCHAR *path = (WCHAR *)Pool_Alloc(pool, len);
    LIST *list = (LIST *)Pool_Alloc(pool, sizeof(LIST));

    if (path && list)
        status = SbieApi_QueryPathList(path_code, NULL, path, xpid, TRUE);

    if (status != STATUS_SUCCESS) {
        Pool_Delete(pool);
        return status;
    }

    List_Init(list);
    while (*((ULONG*)path) != -1) {
        ULONG level = *((ULONG*)path);
        path += sizeof(ULONG)/sizeof(WCHAR);
        PATTERN *pattern = Pattern_Create(pool, path, TRUE, level);
        if (! pattern) {
            Pool_Delete(pool);
            return status;
        }
        List_Insert_After(list, NULL, pattern);
        path += wcslen(path) + 1;
    }

    *out_pool = pool;
    *out_list = list;

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// CheckProcessPathList
//---------------------------------------------------------------------------


//bool UserServer::CheckProcessPathList(LIST *list, const WCHAR *string)
//{
//    BOOLEAN ret = FALSE;
//
//    ULONG length = wcslen(string);
//    ULONG path_len = (length + 1) * sizeof(WCHAR);
//    WCHAR* path_lwr = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, path_len);
//    if (!path_lwr) {
//        SbieApi_Log(2305, NULL);
//        goto finish;
//    }
//    memcpy(path_lwr, string, path_len);
//    path_lwr[length] = L'\0';
//    _wcslwr(path_lwr);
//
//    PATTERN *pat = (PATTERN *)List_Head(list);
//    while (pat) {
//        if (Pattern_Match(pat, path_lwr, length)) {
//            ret = TRUE;
//            goto finish;
//        }
//        pat = (PATTERN *)List_Next(pat);
//    }
//
//finish:
//    HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, path_lwr);
//
//    return ret;
//}

