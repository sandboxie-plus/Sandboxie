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
// Start Sandbox Monitoring for Current Process
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "common/win32_ntddk.h"
#include "core/dll/sbiedll.h"
#include "common/defines.h"
#include "core/svc/SbieIniWire.h"
#include "common/my_version.h"
#include "msgs/msgs.h"
#include "core/drv/api_defs.h"
#include <psapi.h>
#include <Shlwapi.h>
#include "common/SecDeskHelper.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define MyHeapAlloc(len) \
    (WCHAR*)HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, (len))

#define MyHeapFree(ptr) \
    HeapFree(GetProcessHeap(), 0, ptr);


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


void List_Process_Ids(void);
int Terminate_All_Processes(BOOL all_boxes);
int Unmount_All_Boxes(BOOL all_boxes);
int Delete_All_Sandboxes();

extern WCHAR *DoRunDialog(HINSTANCE hInstance);
extern WCHAR *DoBoxDialog(void);
extern bool DoAboutDialog(bool bReminder = false);
extern BOOL ResolveDirectory(WCHAR *PathW);
extern WCHAR *DoStartMenu(void);
extern BOOL WriteStartMenuResult(const WCHAR *MapName, const WCHAR *Command);
extern void DeleteSandbox(
    const WCHAR *BoxName, BOOL bLogoff, BOOL bSilent, int phase);
int SecureDialogFunc(HWND hWnd, void* param);


extern "C" {
    SBIEDLL_EXPORT NTSTATUS Key_GetName(
        HANDLE RootDirectory, UNICODE_STRING* ObjectName,
        WCHAR** OutTruePath, WCHAR** OutCopyPath, BOOLEAN* OutIsBoxedPath);

    SBIEDLL_EXPORT NTSTATUS File_GetName(
        HANDLE RootDirectory, UNICODE_STRING* ObjectName,
        WCHAR** OutTruePath, WCHAR** OutCopyPath, ULONG* OutFlags);
}

//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


WCHAR BoxName[BOXNAME_COUNT];
WCHAR BoxKey[128+1];

PWSTR ChildCmdLine = NULL;
PWSTR ChildWrkDir = NULL;
BOOL run_mail_agent = FALSE;
BOOL display_run_dialog = FALSE;
int display_start_menu = 0;
BOOL execute_auto_run = FALSE;
BOOL execute_open_with = FALSE;
BOOL run_elevated_2 = FALSE;
BOOL fake_admin = FALSE;
BOOL disable_force_on_this_program = FALSE;
BOOL force_children_on_this_program = FALSE;
BOOL auto_select_default_box = FALSE;
WCHAR *StartMenuSectionName = NULL;
BOOL run_silent = FALSE;
BOOL keep_alive = FALSE;
BOOL hide_window = FALSE;
BOOL wait_for_process = FALSE;
BOOLEAN layout_rtl = FALSE;
WORD GetStartupInfo_wShowWindow = SW_SHOWNORMAL;

WCHAR *Sandboxie_Start_Title = NULL;

ULONG Token_Elevation_Type = 0;

extern BOOL boxdlg_run_outside_sandbox;



//---------------------------------------------------------------------------
// die
//---------------------------------------------------------------------------


static ULONG die(ULONG rc)
{
    //
    // Comodo Firewall and perhaps others, trap NtTerminateProcess
    // and cause an exception for some unknown reason, so if
    // NtTerminateProcess fails, we take an alternate route and
    // use NtTerminateThread
    //

    __try {
        NtTerminateProcess(NtCurrentProcess(), rc);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        NtTerminateThread(NtCurrentThread(), rc);
    }

    return rc;
}


//---------------------------------------------------------------------------
// Show_Error
//---------------------------------------------------------------------------


void Show_Error(WCHAR *Descr)
{
    WCHAR msg[2048];

    DWORD ErrorCode = GetLastError();

    PWSTR ErrorText;
    DWORD FormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS;

    FormatMessage(FormatFlags, NULL, ErrorCode,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR)&ErrorText, 0, NULL);

    if (ErrorCode) {
        WCHAR *SysErrText = SbieDll_FormatMessage0(MSG_3206);
        wsprintf(msg, L"%s\n\n%s\n\n%s (%d)",
            Descr, SysErrText, ErrorText, ErrorCode);
        LocalFree(SysErrText);
    } else
        wcscpy(msg, Descr);

    if ((! run_silent) && (! hide_window)) {
        MessageBox(NULL, msg, Sandboxie_Start_Title,
                   MB_ICONEXCLAMATION | MB_OK | MB_SETFOREGROUND | MB_TOPMOST |
                   (layout_rtl ? MB_RTLREADING | MB_RIGHT : 0));
    }
}


//---------------------------------------------------------------------------
// MyCoInitialize
//---------------------------------------------------------------------------


void MyCoInitialize(void)
{
    static BOOL init = FALSE;
    if (! init) {
        extern HRESULT CoInitialize(void *);
        HRESULT hr = CoInitialize(NULL);
        if (hr != S_OK && hr != S_FALSE) {
            Show_Error(SbieDll_FormatMessage(MSG_3213, NULL));
            die(EXIT_FAILURE);
        }
        init = TRUE;
    }
}


//---------------------------------------------------------------------------
// CallSbieSvcGetUser
//---------------------------------------------------------------------------


void *CallSbieSvcGetUser(void)
{
    SBIE_INI_GET_USER_REQ req;
    void *rpl = NULL;
    ULONG retries;

    //
    // issue request to get current user section,
    // try to start SbieSvc until we get a reply
    //

    for (retries = 0; retries < 12; ++retries) {

        if (retries) {

            SbieDll_StartSbieSvc(FALSE);
            Sleep(500);
        }

        req.h.msgid = MSGID_SBIE_INI_GET_USER;
        req.h.length = sizeof(SBIE_INI_GET_USER_REQ);
        rpl = SbieDll_CallServer(&req.h);

        if (rpl)
            break;
    }

    return rpl;
}


//---------------------------------------------------------------------------
// Validate_Box_Name
//---------------------------------------------------------------------------


BOOL Validate_Box_Name(void)
{
    //
    // make sure Sandboxie components are all started
    //

    if (! CallSbieSvcGetUser()) {

        WCHAR *errmsg =
            SbieDll_FormatMessage1(MSG_2331, SbieDll_GetStartError());
        SetLastError(0);
        Show_Error(errmsg);

        return FALSE;
    }

    //
    // parse /box parameter
    //

    if (! disable_force_on_this_program) {

        if (display_start_menu != 2) {
            if (!DoAboutDialog(true))
                return die(EXIT_FAILURE);
        }

        if (_wcsicmp(BoxName, L"__ask__") == 0 ||
            _wcsicmp(BoxName, L"current") == 0) {

            if (auto_select_default_box) {
                if(!NT_SUCCESS(SbieApi_QueryConfAsIs(L"GlobalSettings", L"DefaultBox", 0, BoxName, sizeof(BoxName))) || *BoxName == L'\0')
                    wcscpy(BoxName, L"DefaultBox");
                if (SbieApi_IsBoxEnabled(BoxName) != STATUS_SUCCESS)
                    auto_select_default_box = FALSE;
            }
            if (! auto_select_default_box)
                wcscpy(BoxName, DoBoxDialog());

            if (! BoxName[0]) {
                if (disable_force_on_this_program) {
                    // might be set by DoBoxDialog
                    return FALSE;
                }
                return die(EXIT_FAILURE);
            }
        }

        if (SbieApi_IsBoxEnabled(BoxName) != STATUS_SUCCESS) {

            if (run_silent)
                ExitProcess(ERROR_UNKNOWN_PROPERTY);

            SetLastError(0);
            Show_Error(SbieDll_FormatMessage1(MSG_3204, BoxName));
            return die(EXIT_FAILURE);
        }
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Get_Default_Browser
//---------------------------------------------------------------------------


_FX WCHAR *Get_Default_Browser(void)
{
    const ULONG _cmdline = 10240;
    HRESULT rc;
    DWORD cmdlen;
    WCHAR *pgmname;
    BOOL  iexplore = FALSE;

    WCHAR *assoc_name = L".html";
    OSVERSIONINFO osvi;
    memzero(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx((OSVERSIONINFO *)&osvi);
    if ((osvi.dwMajorVersion == 10) || (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion >= 2)) // Win 10 or Win 8
    {
        assoc_name = L"http";
    }

    cmdlen = _cmdline;
    pgmname = MyHeapAlloc(cmdlen);

    cmdlen = _cmdline;
    rc = AssocQueryString(0, ASSOCSTR_EXECUTABLE,
                          assoc_name, NULL, pgmname, &cmdlen);

    if (rc != S_OK)
        iexplore = TRUE;
    else {
        WCHAR *backslash = wcsrchr(pgmname, L'\\');
        if (! backslash)
            iexplore = TRUE;
        else if (_wcsicmp(backslash + 1, L"notepad.exe") == 0)
            iexplore = TRUE;
        else if (_wcsicmp(backslash + 1, L"openwith.exe") == 0)
            iexplore = TRUE;
        else if (_wcsicmp(backslash + 1, L"LaunchWinApp.exe") == 0)     // MS Edge (Metro)
            iexplore = TRUE;
    }

    if (iexplore) {

        cmdlen = _cmdline;
        rc = AssocQueryString(ASSOCF_INIT_BYEXENAME, ASSOCSTR_EXECUTABLE,
                              L"iexplore", NULL, pgmname, &cmdlen);

        if (rc != S_OK) {

            rc = SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL,
                                 SHGFP_TYPE_CURRENT, pgmname);
            if (rc != S_OK) {

                memzero(pgmname, 16);
                GetSystemWindowsDirectory(pgmname, MAX_PATH);
                if (! pgmname[0])
                    wcscpy(pgmname, L"C:\\");
                wcscpy(pgmname + 3, L"Program Files");
            }

            wcscat(pgmname, L"\\Internet Explorer\\iexplore.exe");
        }
    }

    return pgmname;
}


//---------------------------------------------------------------------------
// Eat_String
//---------------------------------------------------------------------------


PWSTR Eat_String(PWSTR str)
{
    BOOL quoted = FALSE;

    while (*str == L' ')
        ++str;

    while (*str) {
        if (*str == L'\"')
            quoted = !quoted;
        else if (*str == L' ' && (! quoted))
            break;
        ++str;
    }

    while (*str == L' ')
        ++str;

    return str;
}


//---------------------------------------------------------------------------
// Parse_Command_Line
//---------------------------------------------------------------------------


BOOL Parse_Command_Line(void)
{
    static const WCHAR *default_browser   = L"default_browser";
    static const WCHAR *mail_agent        = L"mail_agent";
    static const WCHAR *run_dialog        = L"run_dialog";
    static const WCHAR *start_menu        = L"start_menu";
    static const WCHAR *about_dialog      = L"about_dialog";
    static const WCHAR *open_with         = L"open_with";
    static const WCHAR *auto_run          = L"auto_run";
    static const WCHAR *mount_hive        = L"mount_hive";
    static const WCHAR *delete_sandbox    = L"delete_sandbox";
    static const WCHAR *delete_all_sandboxes = L"delete_all_sandboxes";
    static const WCHAR *_logoff           = L"_logoff";
    static const WCHAR *_silent           = L"_silent";
    static const WCHAR *_phase            = L"_phase";
    static const WCHAR *disable_force     = L"disable_force";

    PWSTR tmp;

    // eat first parameter - the program name for Start.exe

    PWSTR cmd = GetCommandLine();
    cmd = Eat_String(cmd);

    //
    //
    //

    if (_wcsicmp(cmd, L"run_sbie_ctrl") == 0 || _wcsnicmp(cmd, L"open_agent", 10) == 0) {

        union {
            MSG_HEADER req;
            UCHAR buffer[128];
        };
        MSG_HEADER *rpl = NULL;
        if (CallSbieSvcGetUser()) {
            req.length = sizeof(req);
            req.msgid  = MSGID_SBIE_INI_RUN_SBIE_CTRL;

            if (_wcsnicmp(cmd, L"open_agent:", 11) == 0) {
                cmd += 11;
                tmp = Eat_String(cmd);
                if (*cmd == L'\"') {
                    cmd++;
                    tmp--;
                }
                ULONG len = ULONG(tmp - cmd) * sizeof(WCHAR);
                memcpy((WCHAR*)&buffer[req.length], cmd, len);
                req.length += len;
                *((WCHAR*)&buffer[req.length]) = 0;
                req.length += sizeof(WCHAR);
            }

            rpl = SbieDll_CallServer(&req);
        }
        ExitProcess((rpl && rpl->status == 0) ? EXIT_SUCCESS : EXIT_FAILURE);
    }

    //
    // parse command line switches before the program name to invoke
    //

    while (*cmd == L'/') {
        ++cmd;

        //
        // Command line switch /box
        //

        if (_wcsnicmp(cmd, L"box:", 4) == 0) {

            cmd += 4;

            tmp = cmd;

            if (*tmp == L'-' &&
                    (! SbieApi_QueryProcessInfo(
                        (HANDLE)(ULONG_PTR)GetCurrentProcessId(), 0))) {

                //
                // box parameter specifies pid number for model process
                //

                LONG model_pid = wcstol(cmd, &cmd, 10);

                if (*cmd != L' ' || model_pid >= 0 ||
                            (! SbieApi_QueryProcessInfo(
                                    (HANDLE)(ULONG_PTR)-model_pid, 0)))
                    cmd = tmp;

            } else {

                //
                // box parameter specifies boxname
                //

                while (*cmd && *cmd != L' ') {
                    if (iswalnum(*cmd) || *cmd == L'_')
                        ++cmd;
                    else {
                        cmd = tmp;
                        break;
                    }
                }
            }

            if (tmp == cmd || (cmd - tmp > (BOXNAME_COUNT - 2))) {

                if (run_silent)
                    ExitProcess(ERROR_UNKNOWN_PROPERTY);

                SetLastError(0);
                Show_Error(SbieDll_FormatMessage1(MSG_3204, tmp));
                return FALSE;
            }

            memzero(BoxName, sizeof(BoxName));
            wcsncpy(BoxName, tmp, (cmd - tmp));

        //
        // Command line switch /env:=Refresh
        //

        } else if (_wcsnicmp(cmd, L"env:=Refresh", 12) == 0) {

            if (SbieApi_QueryProcessInfo(
                        (HANDLE)(ULONG_PTR)GetCurrentProcessId(), 0)) {
                extern void Env_Refresh(const WCHAR *BoxName);
                Env_Refresh(BoxName);
            }

            cmd += 12;

        //
        // Command line switch /env:Name="Value"
        //

        } else if (_wcsnicmp(cmd, L"env:", 4) == 0) {

            WCHAR *save_cmd = cmd;
            WCHAR *env_name, *env_value;
            ULONG env_name_len, env_value_len;
            WCHAR *env_name_x, *env_value_x;

            env_name = cmd + 4;
            cmd = wcschr(env_name, L'=');
            if (cmd)
                env_name_len = (ULONG)(ULONG_PTR)(cmd - env_name);
            else
                env_name_len = 999;
            if (env_name_len > 128) {
                SetLastError(0);
                Show_Error(SbieDll_FormatMessage1(MSG_3202, save_cmd));
                return FALSE;
            }

            env_value = cmd + 1;
            cmd = Eat_String(env_value);
            if (cmd)
                env_value_len = (ULONG)(ULONG_PTR)(cmd - env_value);
            else
                env_value_len = 999;
            if (env_value_len > 128) {
                SetLastError(0);
                Show_Error(SbieDll_FormatMessage1(MSG_3202, save_cmd));
                return FALSE;
            }

            while (env_value[env_value_len - 1] == L' ')
                --env_value_len;
            if (env_value[env_value_len - 1] == L'\"')
                --env_value_len;
            if (env_value[0]                 == L'\"') {
                --env_value_len;
                ++env_value;
            }

            env_name_x = MyHeapAlloc((env_name_len + 1) * sizeof(WCHAR));
            wmemcpy(env_name_x, env_name, env_name_len);
            env_name_x[env_name_len] = L'\0';

            env_value_x = MyHeapAlloc((env_value_len + 1) * sizeof(WCHAR));
            wmemcpy(env_value_x, env_value, env_value_len);
            env_value_x[env_value_len] = L'\0';

            SetEnvironmentVariable(env_name_x, env_value_x);

        //
        // Command line switch /reload
        //

        } else if (_wcsnicmp(cmd, L"reload", 6) == 0) {

            SbieApi_ReloadConf(-1, 0);
            ExitProcess(0);

        //
        // Command line switch /terminate, /terminate:*, /terminate_all
        //

        } else if (_wcsnicmp(cmd, L"terminate:*", 11) == 0 ||
                   _wcsnicmp(cmd, L"terminate_all", 13) == 0) {

            return die(Terminate_All_Processes(TRUE));

        } else if (_wcsnicmp(cmd, L"terminate", 9) == 0) {

            return die(Terminate_All_Processes(FALSE));
            
        //
        // Command line switch /unmount, /unmount_all
        //

        } else if (_wcsnicmp(cmd, L"unmount_all", 11) == 0) {

            Terminate_All_Processes(TRUE);
            return die(Unmount_All_Boxes(TRUE));

        } else if (_wcsnicmp(cmd, L"unmount", 7) == 0) {

            Terminate_All_Processes(FALSE);
            return die(Unmount_All_Boxes(FALSE));

        //
        // Command line switch /key:[box password] /mount
        //

        } else if (_wcsnicmp(cmd, L"key:", 4) == 0) {

            cmd += 4;

            tmp = cmd;

            //
            // parameter specifies boxkey
            //

            WCHAR end = L' ';
            if (*tmp == L'\"') {
                ++tmp;
                ++cmd;
                end = L'\"';
            }

            while (*cmd && *cmd != end) {
                ++cmd;
            }

            if (tmp == cmd || (cmd - tmp > ARRAYSIZE(BoxKey)-1)) {

                if (run_silent)
                    ExitProcess(ERROR_UNKNOWN_PROPERTY);

                SetLastError(0);
                Show_Error(SbieDll_FormatMessage1(MSG_3202, tmp));
                return FALSE;
            }

            memzero(BoxKey, sizeof(BoxKey));
            wcsncpy(BoxKey, tmp, (cmd - tmp));

            if (end == L'\"')
                ++cmd;

        } else if (_wcsnicmp(cmd, L"mount_protected", 15) == 0) {

            Validate_Box_Name();
            return die(SbieDll_Mount(BoxName, BoxKey, TRUE) ? EXIT_SUCCESS : EXIT_FAILURE);

        } else if (_wcsnicmp(cmd, L"mount", 5) == 0) {

            Validate_Box_Name();
            return die(SbieDll_Mount(BoxName, BoxKey, FALSE) ? EXIT_SUCCESS : EXIT_FAILURE);

        //
        // Command line switch /listpids
        //

        } else if (_wcsnicmp(cmd, L"listpids", 8) == 0) {

            Validate_Box_Name();

            List_Process_Ids();
            return die(EXIT_SUCCESS);

        //
        // Command line switch /silent
        //

        } else if (_wcsnicmp(cmd, L"silent", 6) == 0) {

            cmd += 6;

            run_silent = TRUE;

        //
        // Command line switch /keep_alive
        //

        } else if (_wcsnicmp(cmd, L"keep_alive", 10) == 0) {

            cmd += 10;

            keep_alive = TRUE;

        //
        // Command line switch /elevate
        //

        } else if (_wcsnicmp(cmd, L"elevate", 7) == 0) {

            cmd = Eat_String(cmd);

            run_elevated_2 = TRUE;

        //
        // Command line switch /fake_admin
        //

        } else if (_wcsnicmp(cmd, L"fake_admin", 10) == 0) {

            cmd = Eat_String(cmd);

            fake_admin = TRUE;

        //
        // Command line switch /disable_force or /dfp
        //

        } else if (_wcsnicmp(cmd, L"disable_force", 13) == 0 ||
                   _wcsnicmp(cmd, L"dfp",            3) == 0) {

            cmd = Eat_String(cmd);

            disable_force_on_this_program = TRUE;

        //
        // Command line switch /force_children or /fcp
        //

        } else if (_wcsnicmp(cmd, L"force_children", 14) == 0 ||
                   _wcsnicmp(cmd, L"fcp",             3) == 0) {

            cmd = Eat_String(cmd);

            force_children_on_this_program = TRUE;

        //
        // Command line switch /hide_window
        //

        } else if (_wcsnicmp(cmd, L"hide_window", 11) == 0) {

            cmd = Eat_String(cmd);

            hide_window = TRUE;
            GetStartupInfo_wShowWindow = SW_HIDE;

        //
        // Command line switch /wait
        //

        } else if (_wcsnicmp(cmd, L"wait", 4) == 0) {

            cmd = Eat_String(cmd);

            wait_for_process = TRUE;

        //
        // unknown command line switch
        //

        } else {
            SetLastError(0);
            Show_Error(SbieDll_FormatMessage1(MSG_3202, cmd));
            return FALSE;
        }

        while (*cmd == L' ')
            ++cmd;
    }

    //
    // if rest is exactly "default_browser", then we run default browser
    // also if begins with http:// or https://
    //

    if (wcsncmp(cmd, default_browser, wcslen(default_browser)) == 0 ||
            _wcsnicmp(cmd, L"http://", 7) == 0 ||
            _wcsnicmp(cmd, L"https://", 8) == 0) {

        ULONG cmdlen;
        WCHAR *pgmname = Get_Default_Browser();
        if (wcsncmp(cmd, default_browser, wcslen(default_browser)) == 0)
            cmd += wcslen(default_browser);
        cmdlen = (wcslen(pgmname) + 64 + wcslen(cmd)) * sizeof(WCHAR);
        ChildCmdLine = MyHeapAlloc(cmdlen);
        wsprintf(ChildCmdLine, L"\"%s\" %s", pgmname, cmd);

        return TRUE;

    // if rest is exactly "mail_agent", then we run default mail agent

    } else if (wcsncmp(cmd, mail_agent, wcslen(mail_agent)) == 0) {

        DWORD cmdlen = 10240;
        PWSTR pgmname = MyHeapAlloc(cmdlen);

        HRESULT rc = AssocQueryString(
            0, ASSOCSTR_EXECUTABLE, L"mailto", NULL, pgmname, &cmdlen);

        if (rc != S_OK) {
            SetLastError(0);
            Show_Error(SbieDll_FormatMessage0(MSG_3209));
            return FALSE;
        }

        cmd += wcslen(mail_agent);
        cmdlen += wcslen(cmd) + 32;

        ChildCmdLine = MyHeapAlloc(cmdlen * sizeof(WCHAR));

        wsprintf(ChildCmdLine, L"\"%s\"%s", pgmname, cmd);

        return TRUE;

    // if rest is exactly "run_dialog", then we indicate to later
    // invoke the Run Sandboxed dialog

    } else if (wcsncmp(cmd, run_dialog, wcslen(run_dialog)) == 0) {

        ChildCmdLine = cmd;
        display_run_dialog = TRUE;

        return TRUE;

    // if rest is exactly "start_menu", then we indicate to later
    // invoke the Start Menu

    } else if (wcsncmp(cmd, start_menu, wcslen(start_menu)) == 0) {

        WCHAR *colon = cmd + wcslen(start_menu);
        display_start_menu = *colon == L':' ? 2 : 1; // 1 run from start menu, 2 create shortcut

        if (! SbieApi_QueryProcessInfo(
                        (HANDLE)(ULONG_PTR)GetCurrentProcessId(), 0)) {
            // this is the instance of Start.exe outside the sandbox
            // so just resend the start_menu command line to the
            // instance that will restart in the sandbox
            ChildCmdLine = cmd;

        } else {

            // this is the instance of Start.exe in the sandbox so we
            // need to parse the start_menu command line
            if (*colon == L':') {
                ULONG name_len = wcslen(colon + 1);
                StartMenuSectionName =
                    MyHeapAlloc((name_len + 4) * sizeof(WCHAR));
                if (StartMenuSectionName) {
                    wmemcpy(StartMenuSectionName, colon + 1, name_len);
                    StartMenuSectionName[name_len] = L'\0';
                }
                colon = wcschr(StartMenuSectionName, L':');
                if (colon) {
                    *colon = L'\0';
                    ChildCmdLine = colon + 1;
                }
            }
        }

        return TRUE;

    // show open with dialog

    } else if (wcsncmp(cmd, open_with, wcslen(open_with)) == 0) {

        /*if (! SbieApi_QueryProcessInfo(
                        (HANDLE)(ULONG_PTR)GetCurrentProcessId(), 0)) {
            // this is the instance of Start.exe outside the sandbox
            // so just resend the start_menu command line to the
            // instance that will restart in the sandbox
            ChildCmdLine = cmd;

        }*/

        execute_open_with = TRUE;

        DWORD len = wcslen(open_with) + 1;
        ChildCmdLine = MyHeapAlloc((wcslen(cmd) - len) * sizeof(WCHAR));
        wcscpy(ChildCmdLine, cmd + len);

        return TRUE;

    // show about dialog

    } else if (wcsncmp(cmd, about_dialog, wcslen(about_dialog)) == 0) {

        DoAboutDialog();

        return FALSE;

    // run auto start entries

    } else if (wcsncmp(cmd, auto_run, wcslen(auto_run)) == 0) {

        if (! SbieApi_QueryProcessInfo(
                        (HANDLE)(ULONG_PTR)GetCurrentProcessId(), 0)) {
            // this is the instance of Start.exe outside the sandbox
            // so just resend the start_menu command line to the
            // instance that will restart in the sandbox
            ChildCmdLine = cmd;

        }

        execute_auto_run = TRUE;

        return TRUE;

    // mount hive

    } else if (wcsncmp(cmd, mount_hive, wcslen(mount_hive)) == 0) {

        if (! SbieApi_QueryProcessInfo(
                        (HANDLE)(ULONG_PTR)GetCurrentProcessId(), 0)) {
            ChildCmdLine = cmd;

        }
        else {
            Sleep(5000);
        }

        return TRUE;

    // if rest is exactly "delete_sandbox", do that processing

    } else if (wcsncmp(cmd, delete_sandbox, wcslen(delete_sandbox)) == 0) {

        BOOL logoff = FALSE;
        BOOL silent = FALSE;
        int phase = 0;

        cmd += wcslen(delete_sandbox);
        while (1) {
            if (wcsncmp(cmd, _logoff, wcslen(_logoff)) == 0) {
                cmd += wcslen(_logoff);
                logoff = TRUE;
                continue;
            }
            if (wcsncmp(cmd, _silent, wcslen(_silent)) == 0) {
                cmd += wcslen(_silent);
                silent = TRUE;
                continue;
            }
            if (wcsncmp(cmd, _phase, wcslen(_phase)) == 0) {
                cmd += wcslen(_phase);
                if (*cmd == L'1')
                    phase = 1;
                else if (*cmd == L'2')
                    phase = 2;
                ++cmd;
                continue;
            }
            break;
        }

        if (phase != 2)
            Validate_Box_Name();

        DeleteSandbox(BoxName, logoff, silent, phase);
        // does not return

        return TRUE;

    // if rest is exactly "delete_all_sandboxes", do that processing

    } else if (wcsncmp(cmd, delete_all_sandboxes, wcslen(delete_all_sandboxes)) == 0) {

        return die(Delete_All_Sandboxes());

    //
    // if rest is exactly "disable_force" or "disable_force_off"
    // then process accordingly
    //

    } else if (wcsncmp(cmd, disable_force, 13) == 0) {

        ULONG NewState = TRUE;
        cmd += 13;
        if (_wcsnicmp(cmd, L"_off", 4) == 0)
            NewState = FALSE;
        SbieApi_DisableForceProcess(&NewState, NULL);
        ExitProcess(0);
    }

    //
    // handle uac_prompt
    //

    else if (wcsncmp(cmd, L"uac_prompt", 10) == 0) {

        cmd = Eat_String(cmd);

        wchar_t szPath[MAX_PATH];
        GetModuleFileNameW(NULL, szPath, ARRAYSIZE(szPath));
        *wcsrchr(szPath, L'\\') = L'\0';
        wcscat_s(szPath, MAX_PATH, L"\\SbieWallpaper.png");

        /*while (! IsDebuggerPresent())
            Sleep(500);
        __debugbreak();*/

        //
        // Open Sandboxie's own UAC Dialog
        // Note: When User Account Control (UAC) is configured to not use the secure desktop, sandboxie does the same.
        //

        int ret;
//#ifndef WITH_DEBUG
        if (SbieApi_QueryConfBool(NULL, L"PromptOnSecureDesktop", TRUE) && GetPromptOnSecureDesktop())
            ret = ShowSecureDialog(SecureDialogFunc, cmd, szPath);
        else
//#endif
            ret = SecureDialogFunc(NULL, cmd);

        ExitProcess(ret);
    }

    //
    // otherwise pass the rest of the command line as-is to the child
    //

    if (*cmd == L'\0') {
        SetLastError(0);
        Show_Error(SbieDll_FormatMessage0(MSG_3203));
        return FALSE;
    }

    //
    // if this is a link to start.exe with a box, extract the target command line
    //

    if (StrStrIW(cmd, L"\\start.exe") != 0) {
        wchar_t* tmp = StrStrIW(cmd, L"/box:");
        if (tmp) {
            tmp = StrStrIW(tmp, L" ");
            if (tmp)
                cmd = tmp + 1;
        }
    }

    ChildCmdLine = (WCHAR *)MyHeapAlloc(10240 * sizeof(WCHAR));
    wcscpy(ChildCmdLine, cmd);

    return TRUE;
}


//---------------------------------------------------------------------------
// UacGetParams
//---------------------------------------------------------------------------

const ULONG tzuk = 'xobs';

typedef struct _SECURE_UAC_PACKET {

    //
    // keep in sync with SbieSvc.exe / ServiceServer2
    //

    ULONG   tzuk;
    ULONG   len;
    ULONG   app_len;
    ULONG   app_ofs;
    ULONG   cmd_len;
    ULONG   cmd_ofs;
    ULONG   dir_len;
    ULONG   dir_ofs;
    ULONG   inv_len;
    ULONG64 hEvent;
    ULONG64 hResult;
    ULONG64 ret_code;
    WCHAR   text[1];

} SECURE_UAC_PACKET;

void UacGetParams(
    HANDLE idProcess, ULONG_PTR pkt_addr, ULONG pkt_len, WCHAR **app, WCHAR **cmd, WCHAR **dir)
{
    BOOL ok = TRUE;
    HANDLE hProcess = NULL;
    SECURE_UAC_PACKET *pkt = NULL;
    SIZE_T copy_len;

    //
    // open client process
    //

    if (ok) {
        hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE |
                                PROCESS_VM_OPERATION | PROCESS_DUP_HANDLE,
                                FALSE, (ULONG)(ULONG_PTR)idProcess);
        if (! hProcess)
            ok = FALSE;
    }

    //
    // get and validate request packet
    //

    if (ok) {
        pkt = (SECURE_UAC_PACKET *)HeapAlloc(GetProcessHeap(), 0, pkt_len);
        if (! pkt) {
            ok = FALSE;
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        }
    }

    if (ok) {
        ok = ReadProcessMemory(hProcess, (void *)(ULONG_PTR)pkt_addr, pkt,
                               pkt_len, &copy_len);
        if (ok && (copy_len  != pkt_len ||
                   pkt->tzuk != tzuk ||
                   pkt->len  != pkt_len ||
                   pkt_len   != ~pkt->inv_len)) {

            ok = FALSE;
            SetLastError(ERROR_INVALID_DATA);
        }
    }

    //
    // execute request and copy results back to caller
    //

    if (ok) {

        if (app && pkt->app_len < 1024) {
            *app = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, (pkt->app_len + 1) * sizeof(WCHAR));
            if (*app) {
                wmemcpy(*app, (WCHAR*)((UCHAR*)pkt + pkt->app_ofs), pkt->app_len);
                (*app)[pkt->app_len] = L'\0';
            }
        }

        if (cmd && pkt->cmd_len < 1024) {
            *cmd = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, (pkt->cmd_len + 1) * sizeof(WCHAR));
            if (*cmd) {
                wmemcpy(*cmd, (WCHAR*)((UCHAR*)pkt + pkt->cmd_ofs), pkt->cmd_len);
                (*cmd)[pkt->cmd_len] = L'\0';
            }
        }
        
        if (dir && pkt->dir_len < 1024) {
            *dir = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, (pkt->dir_len + 1) * sizeof(WCHAR));
            if (*dir) {
                wmemcpy(*dir, (WCHAR*)((UCHAR*)pkt + pkt->dir_ofs), pkt->dir_len);
                (*dir)[pkt->dir_len] = L'\0';
            }
        }
    }

    if (hProcess)
        CloseHandle(hProcess);

    if (pkt)
        HeapFree(GetProcessHeap(), 0, pkt);
}


//---------------------------------------------------------------------------
// TruncatePathMiddle
//---------------------------------------------------------------------------


void TruncatePathMiddle(const wchar_t* fullPath, wchar_t* outPath, size_t maxLen)
{
    if (!fullPath || !outPath || maxLen < 5) {
        if (outPath && maxLen > 0)
            outPath[0] = L'\0';
        return;
    }

    size_t fullLen = wcslen(fullPath);
    if (fullLen < maxLen) {
        wcsncpy(outPath, fullPath, maxLen);
        outPath[maxLen - 1] = L'\0';
        return;
    }

    // Find filename
    const wchar_t* filename = wcsrchr(fullPath, L'\\');
    filename = filename ? filename + 1 : fullPath;
    size_t fileLen = wcslen(filename);

    if (fileLen + 4 >= maxLen) {
        // Can't even fit filename
        wcsncpy(outPath, L"...", maxLen);
        wcsncat(outPath, filename + (fileLen - (maxLen - 4)), maxLen - 4);
        outPath[maxLen - 1] = L'\0';
        return;
    }

    // Allocate space for "..." and filename
    size_t remaining = maxLen - fileLen - 4;

    // Find folder split points
    const wchar_t* pathEnd = filename - 1;
    size_t prefixLen = 0;
    const wchar_t* prefixEnd = fullPath;

    while (*prefixEnd && prefixEnd < pathEnd) {
        if (*prefixEnd == L'\\')
            ++prefixLen;
        ++prefixEnd;
    }

    size_t suffixLen = 0;
    const wchar_t* suffixStart = pathEnd;
    while (suffixStart > fullPath && suffixLen < prefixLen) {
        if (*suffixStart == L'\\')
            ++suffixLen;
        --suffixStart;
    }

    // Now walk forward from start
    const wchar_t* prefixWalk = fullPath;
    size_t used = 0;
    wchar_t prefix[260] = L"";
    while (prefixWalk < pathEnd && used < remaining / 2) {
        const wchar_t* next = wcschr(prefixWalk, L'\\');
        if (!next || next >= pathEnd)
            break;

        size_t len = next - prefixWalk + 1;
        if (used + len >= remaining / 2)
            break;

        wcsncat(prefix, prefixWalk, len);
        used += len;
        prefixWalk = next + 1;
    }

    // Now walk backward from suffixStart
    const wchar_t* suffixWalk = pathEnd;
    wchar_t suffix[260] = L"";
    used = 0;

    while (suffixWalk > fullPath && used < remaining / 2) {
        const wchar_t* prev = suffixWalk;
        while (prev > fullPath && *(prev - 1) != L'\\')
            --prev;

        size_t len = suffixWalk - prev + 1;
        if (used + len >= remaining / 2)
            break;

        wchar_t tmp[260] = L"";
        wcsncat(tmp, prev, len);
        wcscat(tmp, suffix);
        wcscpy(suffix, tmp);

        used += len;
        if (prev == fullPath)
            break;
        suffixWalk = prev - 1;
    }

    // Build final string: prefix + "..." + suffix + filename
    wcsncpy(outPath, prefix, maxLen);
    wcscat(outPath, L"...");
    if (suffix[0] != L'\0') {
        wcsncat(outPath, suffix, maxLen - wcslen(outPath) - 1);
    } else if (outPath[wcslen(outPath) - 1] != L'\\') {
        // No suffix preserved, but we want "\filename"
        wcsncat(outPath, L"\\", maxLen - wcslen(outPath) - 1);
    }
    wcsncat(outPath, filename, maxLen - wcslen(outPath) - 1);
    outPath[maxLen - 1] = L'\0';
}


//---------------------------------------------------------------------------
// DrawTextWithFont
//---------------------------------------------------------------------------


int DrawTextWithFont(
    HDC hdc, int y, const WCHAR* txt, int width, HFONT hFont, COLORREF color, DWORD flags = DT_WORDBREAK)
{
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    SetTextColor(hdc, color);

    RECT rc = { 10, y, 10 + width, y + 4096 };
    // Get height
    DrawTextW(hdc, txt, -1, &rc, flags | DT_CALCRECT);
    // Actually draw
    DrawTextW(hdc, txt, -1, &rc, flags);
    SelectObject(hdc, hOldFont);
    return rc.bottom;
}


//---------------------------------------------------------------------------
// MeasureTextHeight
//---------------------------------------------------------------------------


int MeasureTextHeight(HDC hdc, const WCHAR* txt, int width, HFONT hFont, DWORD flags)
{
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    RECT rc = { 10, 0, 10 + width, 4096 };
    DrawTextW(hdc, txt, -1, &rc, flags | DT_CALCRECT);
    SelectObject(hdc, hOldFont);
    return rc.bottom;
}


//---------------------------------------------------------------------------
// UacPromptWndProc
//---------------------------------------------------------------------------

#define IDS_CANCEL 801
#define IDS_YES    805
#define IDS_NO     806

struct SDialogParams
{
    HANDLE          idProcess;
    ULARGE_INTEGER  pkt_addr;
    ULONG           pkt_len;
    int             ButtonY;
    HWND            hYes;
    HWND            hNo;
    HWND            hCancel;
    WCHAR           BoxName[BOXNAME_COUNT];
    WCHAR           ExeName[99];
    WCHAR           AppName[99];
    int             DialogResult;
	HFONT           hFontButtonText;
};

LRESULT UacPromptWndProc(
    HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    SDialogParams* pParams = (SDialogParams*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    switch (msg)
    {
    case WM_CREATE: {

        /*while (! IsDebuggerPresent())
            Sleep(500);
        __debugbreak();*/

        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        pParams = (SDialogParams*)pcs->lpCreateParams;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)pParams);

        SbieApi_QueryProcess(pParams->idProcess, pParams->BoxName, pParams->ExeName, NULL, NULL);

        if (pParams->pkt_len) {

            WCHAR* AppName;
            UacGetParams(pParams->idProcess, (ULONG_PTR)pParams->pkt_addr.QuadPart, pParams->pkt_len, &AppName, NULL, NULL);
            if (AppName) {

                if (memcmp(AppName, L"*MSI*", 5 * sizeof(WCHAR)) == 0)
                    wcscpy(pParams->AppName, L"Windows Installer");
                else
                    TruncatePathMiddle(AppName, pParams->AppName, 70);

                HeapFree(GetProcessHeap(), 0, AppName);
            }
        }

        //
        // Create Yes/No/Cancel buttons
        //

		HDC hdc = GetDC(hwnd);
		const int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
		const int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
		ReleaseDC(hwnd, hdc);

		const int buttonWidth = MulDiv(100, dpiX, 96);
		const int buttonHeight = MulDiv(30, dpiY, 96);
		const int buttonGapWidth = MulDiv(110, dpiX, 96);
        pParams->ButtonY = MulDiv(300, dpiY, 96);

		// Create a font for the button text.
		// This font has to be preserved until the window is destroyed.
		int heightButtonText = MulDiv(12, dpiY, 72);
		pParams->hFontButtonText = CreateFont(-heightButtonText, 0, 0, 0,
			FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH,
			L"Arial");

        //WCHAR bufYes[32], bufNo[32], bufCancel[32];
        //LoadStringW(GetModuleHandleW(L"user32.dll"), IDS_YES,    bufYes,    32);
        //LoadStringW(GetModuleHandleW(L"user32.dll"), IDS_NO,     bufNo,     32);
        //LoadStringW(GetModuleHandleW(L"user32.dll"), IDS_CANCEL, bufCancel, 32);

		int buttonX = MulDiv(140, dpiX, 96);
        WCHAR* pMsg = SbieDll_FormatMessage0(MSG_3115);
        pParams->hYes = CreateWindowW(L"BUTTON", pMsg,
            WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON,
            buttonX, pParams->ButtonY, buttonWidth, buttonHeight, hwnd, (HMENU)IDYES,
            GetModuleHandle(NULL), NULL);
        LocalFree(pMsg);
		SendMessageW(pParams->hYes, WM_SETFONT, (WPARAM)pParams->hFontButtonText, TRUE);
		buttonX += buttonGapWidth;

        pMsg = SbieDll_FormatMessage0(MSG_3116);
        pParams->hNo = CreateWindowW(L"BUTTON", pMsg,
            WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_DEFPUSHBUTTON,
            buttonX, pParams->ButtonY, buttonWidth, buttonHeight, hwnd, (HMENU)IDNO,
            GetModuleHandle(NULL), NULL);
        LocalFree(pMsg);
		SendMessageW(pParams->hNo, WM_SETFONT, (WPARAM)pParams->hFontButtonText, TRUE);
		buttonX += buttonGapWidth;

        pMsg = SbieDll_FormatMessage0(MSG_3117);
        pParams->hCancel = CreateWindowW(L"BUTTON", pMsg,
            WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON,
            buttonX, pParams->ButtonY, buttonWidth, buttonHeight, hwnd, (HMENU)IDCANCEL,
            GetModuleHandle(NULL), NULL);
        LocalFree(pMsg);
		SendMessageW(pParams->hCancel, WM_SETFONT, (WPARAM)pParams->hFontButtonText, TRUE);

        SetFocus(pParams->hNo);

        return 0;
    }

    case WM_SETCURSOR:
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        return 0;

    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDYES:
        case IDNO:
        case IDCANCEL:
            if (pParams) pParams->DialogResult = LOWORD(wParam);
            DestroyWindow(hwnd);
            break;
        }
        return 0;

    case WM_DESTROY:
		DeleteObject(pParams->hFontButtonText);
        PostQuitMessage(0);
        return 0;

    case WM_PAINT: {

        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

		const int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
		const int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);

        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        int width = clientRect.right - MulDiv(20, dpiX, 96);

        // Prepare fonts
        int heightTitle = MulDiv(12, dpiY, 72);
        HFONT hFontTitle = CreateFont(-heightTitle, 0, 0, 0,
            FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH,
            L"Arial");

        int heightNormal = MulDiv(9, dpiY, 72);
        HFONT hFontNormal = CreateFont(-heightNormal, 0, 0, 0,
            FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH,
            L"Arial");

        SetBkColor(hdc, 0x00404040);

        int y = MulDiv(10, dpiY, 96);
        WCHAR* pMsg = SbieDll_FormatMessage2(3244, pParams->ExeName, pParams->BoxName);
        y = DrawTextWithFont(hdc, y, pMsg, width, hFontTitle, 0x00FFFFFF);
        LocalFree(pMsg);

        //y += 20;
        //pMsg = SbieDll_FormatMessage0(3742);
        //y = DrawTextWithFont(hdc, y, pMsg, width, hFontTitle, 0x00FFFFFF, DT_SINGLELINE);
        //LocalFree(pMsg);
        y = DrawTextWithFont(hdc, y, pParams->BoxName, width, hFontTitle, 0x0080FFFF, DT_SINGLELINE);

        if (*pParams->AppName) {
            y += MulDiv(10, dpiY, 96);
            pMsg = SbieDll_FormatMessage0(3743);
            y = DrawTextWithFont(hdc, y, pMsg, width, hFontTitle, 0x00FFFFFF, DT_SINGLELINE);
            LocalFree(pMsg);
            y = DrawTextWithFont(hdc, y, pParams->AppName, width, hFontTitle, 0x0080FFFF, DT_SINGLELINE);
        }

        y += MulDiv(30, dpiY, 96);
        pMsg = SbieDll_FormatMessage0(3245);
        y = DrawTextWithFont(hdc, y, pMsg, width, hFontTitle, 0x00FFFFFF);
        LocalFree(pMsg);

        // Store for button placement, e.g.:
        //pParams->ButtonY = y + 10;

        // Now draw the last message aligned from the bottom
        pMsg = SbieDll_FormatMessage0(3246);
        y = clientRect.bottom - MeasureTextHeight(hdc, pMsg, width, hFontNormal, DT_WORDBREAK) - MulDiv(10, dpiY, 96);
        DrawTextWithFont(hdc, y, pMsg, width, hFontNormal, 0x00AAAAAA);
        LocalFree(pMsg);

        DeleteObject(hFontTitle);
        DeleteObject(hFontNormal);

        EndPaint(hwnd, &ps);
        return 0;
    }

    //case WM_SIZE:
    //{
    //    // Reposition buttons after resize/paint
    //    if (pParams)
    //    {
    //        RECT clientRect;
    //        GetClientRect(hwnd, &clientRect);
    //        int width = clientRect.right - 20;

    //        // Assume buttons are already created and stored somewhere (e.g., in pParams)
    //        int btnY = pParams->ButtonY;
    //        int btnW = 100, btnH = 30;
    //        int spacing = 10;

    //        // Center buttons as an example, or keep your layout
    //        int totalWidth = btnW * 3 + spacing * 2;
    //        int startX = (clientRect.right - totalWidth) / 2;

    //        SetWindowPos(pParams->hYes, NULL, startX, btnY, btnW, btnH, SWP_NOZORDER | SWP_SHOWWINDOW);
    //        SetWindowPos(pParams->hNo, NULL, startX + btnW + spacing, btnY, btnW, btnH, SWP_NOZORDER | SWP_SHOWWINDOW);
    //        SetWindowPos(pParams->hCancel, NULL, startX + (btnW + spacing) * 2, btnY, btnW, btnH, SWP_NOZORDER | SWP_SHOWWINDOW);

    //    }
    //}

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}


//---------------------------------------------------------------------------
// SecureDialogFunc
//---------------------------------------------------------------------------


int SecureDialogFunc(HWND hWndParent, void* param)
{
    SDialogParams Params;
    memset(&Params, 0, sizeof(Params));

    WCHAR* ptr = (WCHAR*)param;
    if (ptr) {
        Params.idProcess = (HANDLE)(ULONG_PTR)wcstol(ptr, &ptr, 16);
        if (*ptr == L'_') {
            Params.pkt_addr.HighPart = wcstoul(ptr + 1, &ptr, 16);
            if (*ptr == L'_') {
                Params.pkt_addr.LowPart = wcstoul(ptr + 1, &ptr, 16);
                if (*ptr == L'_')
                    Params.pkt_len = wcstol(ptr + 1, &ptr, 16);
            }
        }
    }

    Params.DialogResult = IDCANCEL;

    //
    // get UAC shield icon
    //

    HICON hShieldIcon = NULL;
    HICON hShieldIconSm = NULL;

    if (1) {

        typedef struct {
            DWORD cbSize;
            HICON hIcon;
            int iSysImageIndex;
            int iIcon;
            WCHAR szPath[MAX_PATH];
        } SHSTOCKICONINFO;
        typedef HRESULT(*P_SHGetStockIconInfo)(
            ULONG_PTR siid, UINT uFlags, SHSTOCKICONINFO* psii);
        const ULONG SIID_SHIELD = 77;
        const ULONG SHGSI_ICON = 0x000000100;
        const ULONG SHGSI_SMALLICON = 0x000000001;

        HMODULE hShell32 = LoadLibrary(L"shell32.dll");
        if (hShell32) {

            P_SHGetStockIconInfo pSHGetStockIconInfo = (P_SHGetStockIconInfo)
                GetProcAddress(hShell32, "SHGetStockIconInfo");
            if (pSHGetStockIconInfo) {

                SHSTOCKICONINFO sii;
                memzero(&sii, sizeof(SHSTOCKICONINFO));
                sii.cbSize = sizeof(SHSTOCKICONINFO);
                HRESULT hr = pSHGetStockIconInfo(
                    SIID_SHIELD, SHGSI_ICON, &sii);
                if (SUCCEEDED(hr))
                    hShieldIcon = sii.hIcon;

                hr = pSHGetStockIconInfo(SIID_SHIELD,
                    SHGSI_ICON | SHGSI_SMALLICON, &sii);
                if (SUCCEEDED(hr))
                    hShieldIconSm = sii.hIcon;
            }
        }
    }

    //
    // create window
    //

    WNDCLASSEX wc;
    memzero(&wc, sizeof(WNDCLASSEX));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0; CS_NOCLOSE;
    wc.lpfnWndProc = UacPromptWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hbrBackground = CreateSolidBrush(0x00404040);
    wc.lpszClassName = SANDBOXIE L"_UAC_WindowClass";
    wc.hIcon = hShieldIcon;
    wc.hIconSm = hShieldIconSm;
    ATOM atom = RegisterClassEx(&wc);

    BOOLEAN rtl;
    SbieDll_GetLanguage(&rtl);

	HDC hScreenDC = GetDC(NULL);

    const int winWidth = MulDiv(600, GetDeviceCaps(hScreenDC, LOGPIXELSX), 96);
    const int winHeight = MulDiv(500, GetDeviceCaps(hScreenDC, LOGPIXELSY), 96);
	ReleaseDC(NULL, hScreenDC);

    int screenX = (GetSystemMetrics(SM_CXSCREEN) - winWidth) / 2;
    int screenY = (GetSystemMetrics(SM_CYSCREEN) - winHeight) / 2;

    HWND hWnd = CreateWindowEx(WS_EX_TOPMOST |
                               (rtl ? WS_EX_LAYOUTRTL : 0),
                               (LPCWSTR)atom, SANDBOXIE,
                               WS_SYSMENU | WS_MINIMIZEBOX,
                               screenX, screenY, winWidth, winHeight,
                               hWndParent, NULL, NULL, &Params);

	// disable parent window so that the background window cannot be clicked and activated
	BOOL isParentEnabled = IsWindowEnabled(hWndParent);
	EnableWindow(hWndParent, FALSE);

    ShowWindow(hWnd, SW_SHOW);

    //
    // do message loop
    //

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        if (!IsDialogMessage(hWnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    //
    // Cleanup after window is closed
    //

	EnableWindow(hWndParent, isParentEnabled);
    if (wc.hbrBackground)
        DeleteObject(wc.hbrBackground);
    if (hShieldIcon)
        DestroyIcon(hShieldIcon);
    if (hShieldIconSm) 
        DestroyIcon(hShieldIconSm);
    if (atom)
        UnregisterClassW((LPCWSTR)atom, GetModuleHandle(NULL));

    return Params.DialogResult;
}


//---------------------------------------------------------------------------
// List_Process_Ids
//---------------------------------------------------------------------------


void List_Process_Ids(void)
{
    CHAR msg[32];
    ULONG len, i;
    ULONG* pids;

    ULONG pid_count = 0;
    SbieApi_EnumProcessEx(BoxName, FALSE, -1, NULL, &pid_count); // query count
    pid_count += 128;

    pids = (ULONG*)MyHeapAlloc(sizeof(ULONG) * pid_count);
    ULONG rc = SbieApi_EnumProcessEx(BoxName, FALSE, -1, pids, &pid_count); // query pids

    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    if (rc != 0 || (! out))
        return;

    sprintf(msg, "%d\r\n", pid_count);
    WriteFile(out, msg, strlen(msg), &len, NULL);

    for (i = 0; i < pid_count; ++i) {
        sprintf(msg, "%d\r\n", pids[i]);
        WriteFile(out, msg, strlen(msg), &len, NULL);
    }

    MyHeapFree(pids);
}


//---------------------------------------------------------------------------
// Terminate_All_Processes
//---------------------------------------------------------------------------


int Terminate_All_Processes(BOOL all_boxes)
{
    if (all_boxes) {

        int index = -1;
        while (1) {
            index = SbieApi_EnumBoxes(index, BoxName);
            if (index == -1)
                break;
            SbieDll_KillAll(-1, BoxName);
        }

    } else {

        Validate_Box_Name();
        SbieDll_KillAll(-1, BoxName);
    }

    return EXIT_SUCCESS;
}


//---------------------------------------------------------------------------
// Unmount_All_Boxes
//---------------------------------------------------------------------------


int Unmount_All_Boxes(BOOL all_boxes)
{
    if (all_boxes) {

        int index = -1;
        while (1) {
            index = SbieApi_EnumBoxes(index, BoxName);
            if (index == -1)
                break;
            SbieDll_Unmount(BoxName);
        }

    } else {

        Validate_Box_Name();
        SbieDll_Unmount(BoxName);
    }

    return EXIT_SUCCESS;
}


//---------------------------------------------------------------------------
// Program_Start
//---------------------------------------------------------------------------


int Program_Start(void)
{
    BOOL ok;
    ULONG err;
    WCHAR *curdir;
    WCHAR *cmdline, *expanded;
    ULONG cmdline_len;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    HANDLE hNewProcess = NULL;

    memzero(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    memzero(&pi, sizeof(PROCESS_INFORMATION));

    si.dwFlags = STARTF_FORCEOFFFEEDBACK;
    if (GetStartupInfo_wShowWindow != SW_SHOWNORMAL) {
        si.dwFlags |= STARTF_USESHOWWINDOW;
        si.wShowWindow = GetStartupInfo_wShowWindow;
    }

    //
    // copy input command line, minus leading and trailing spaces
    //

    while (ChildCmdLine && *ChildCmdLine == L' ')
        ++ChildCmdLine;

    cmdline_len = (wcslen(ChildCmdLine) + 32) * sizeof(WCHAR);
    cmdline = MyHeapAlloc(cmdline_len);
    memcpy(cmdline, ChildCmdLine, cmdline_len);

    cmdline_len = wcslen(cmdline);
    while (cmdline[cmdline_len - 1] == L' ') {
        --cmdline_len;
        cmdline[cmdline_len] = L'\0';
    }

    //
    // change to target directory
    //

    if (!ChildWrkDir) {
        curdir = (WCHAR*)MyHeapAlloc(1024 * sizeof(WCHAR));
        if (GetEnvironmentVariable(
            L"00000000_" SBIE L"_CURRENT_DIRECTORY", curdir, 1020)) {

            ChildWrkDir = curdir;
            SetEnvironmentVariable(
                L"00000000_" SBIE L"_CURRENT_DIRECTORY", NULL);
        }
    }

    if(ChildWrkDir)
        SetCurrentDirectory(ChildWrkDir);

    //
    // service programs expect to be started by CreateProcess, and may not
    // expect modifications to the command line by ShellExecuteEx.
    // if we are running a service (as started by ServiceServer::RunHandler
    // in SbieSvc), then use CreateProcess instead of ShellExecuteEx
    //

    if (GetEnvironmentVariable(
                            L"00000000_" SBIE L"_SERVICE_NAME", NULL, 0)) {

        expanded = MyHeapAlloc(8192 * sizeof(WCHAR));
        ExpandEnvironmentStrings(cmdline, expanded, 8192);

        //
        // When the service process has a manifest which requires elevated privileges,
        // CreateProcess will fail if we did not start with a elevated token.
        // To fix this issue we always fake being elevated when starting a service.
        //

        SbieDll_SetFakeAdmin(TRUE);

		//
		// If the command contains a space but no ", try to fix it
		//

		if (wcsstr(expanded, L" ") && !wcsstr(expanded, L"\"") && _waccess(expanded, 0) != -1)
		{
			wmemmove(expanded + 1, expanded, wcslen(expanded) + 1);
			expanded[0] = L'\"';
			wcscat(expanded, L"\"");
		}

        ok = CreateProcess(
            NULL, expanded, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

        return (ok ? EXIT_SUCCESS : EXIT_FAILURE);
    }

    //
    // start program
    //

    do {

        //
        // first pass the command line as is to the shell, in case this
        // is the path to a folder or to a document, or a single
        // executable file
        //

        SHELLEXECUTEINFO shExecInfo;

        shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
        shExecInfo.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_DOENVSUBST
                         | SEE_MASK_FLAG_DDEWAIT | SEE_MASK_NOZONECHECKS;
        if (wait_for_process || keep_alive || force_children_on_this_program)
            shExecInfo.fMask |= SEE_MASK_NOCLOSEPROCESS;
        shExecInfo.hwnd = NULL;
        shExecInfo.lpVerb = NULL;
        shExecInfo.lpFile = cmdline;
        shExecInfo.lpParameters = NULL;
        shExecInfo.lpDirectory = NULL;
        shExecInfo.nShow = GetStartupInfo_wShowWindow;
        shExecInfo.hInstApp = NULL;

        if (run_elevated_2) {

            //
            // do not use the runas verb if already administrator
            //

            if (Token_Elevation_Type != TokenElevationTypeFull) {

                //
                // do not use the runas verb if dropped rights is in effect
                //

                ULONG64 ProcessFlags = SbieApi_QueryProcessInfo(0, 0);
                if (! (ProcessFlags & SBIE_FLAG_DROP_RIGHTS)) {

                    shExecInfo.lpVerb = L"runas";
                }
            }
        }

        //
        // make sure AppHelp.dll is loaded, so third party software like
        // EMET which relies on injection through ShimEng/AppHelp can work.
        // note:  don't use a full path to load AppHelp.dll or otherwise
        // the ClosedFilePath setting from core/drv/file.c will apply
        //

        //
        // note: this workaround does not work the path is still blocked 
        // and we still get problems, hence now the derive has an exemption for start.exe
        //

        //LoadLibrary(L"apphelp.dll");

        /*if (1) {
            WCHAR *AppHelpPath = MyHeapAlloc(512 * sizeof(WCHAR));
            AppHelpPath[0] = L'\0';
            if (GetModuleFileName(
                    GetModuleHandle(L"kernel32.dll"), AppHelpPath, 510)) {
                WCHAR *backslash = wcsrchr(AppHelpPath, L'\\');
                if (backslash) {
                    wcscpy(backslash + 1, L"appHelp.dll");
                    OutputDebugString(AppHelpPath);
                    LoadLibrary(AppHelpPath);
                }
            }
        }*/

        //
        // otherwise try run the file normally
        //

        ok = ShellExecuteEx(&shExecInfo);
        err = GetLastError();

        if ((! ok) && (err == ERROR_CANCELLED))
            break;

        //
        // trying to run a document as administrator will fail, in this
        // case we use the runas verb to invoke a second instance of
        // Start.exe which will then open the document normally
        //

        /*if ((! ok) && (err == ERROR_NO_ASSOCIATION) && run_elevated_2) {

            WCHAR *start_exe = MyHeapAlloc((MAX_PATH + 8) * sizeof(WCHAR));

            GetModuleFileName(NULL, start_exe, MAX_PATH);

            shExecInfo.lpFile = start_exe;
            shExecInfo.lpParameters = cmdline;

            ok = ShellExecuteEx(&shExecInfo);
            err = GetLastError();

            shExecInfo.lpFile = cmdline;
            shExecInfo.lpParameters = NULL;
        }*/

        //
        // handle a possible DDE error by retrying without the DDE option
        //

        if ((! ok) && (err == ERROR_DDE_FAIL || err == ERROR_NO_ASSOCIATION)) {

            shExecInfo.fMask &= ~SEE_MASK_FLAG_DDEWAIT;

            ok = ShellExecuteEx(&shExecInfo);
            err = GetLastError();

            shExecInfo.fMask |= SEE_MASK_FLAG_DDEWAIT;
        }

        //
        // if that didn't work, split the single command
        // into a command and parameters, and try again
        //

        if (! ok) {

            WCHAR *parms = Eat_String(cmdline);
            if (parms && *parms) {

                WCHAR *cmd2 = MyHeapAlloc((cmdline_len + 1) * sizeof(WCHAR));
                WCHAR *arg2 = MyHeapAlloc((cmdline_len + 1) * sizeof(WCHAR));

                wcsncpy(cmd2, cmdline, parms - cmdline);
                cmd2[parms - cmdline] = L'\0';
                while (1) {
                    ULONG len = wcslen(cmd2);
                    if ((! len) || (cmd2[len - 1] != L' '))
                        break;
                    cmd2[len - 1] = L'\0';
                }

                wcscpy(arg2, cmdline + (parms - cmdline));

                shExecInfo.lpFile = cmd2;
                shExecInfo.lpParameters = arg2;

                ok = ShellExecuteEx(&shExecInfo);
                err = GetLastError();

                if ((! ok) && (err == ERROR_DDE_FAIL || err == ERROR_NO_ASSOCIATION)) {
                    shExecInfo.fMask &= ~SEE_MASK_FLAG_DDEWAIT;
                    ok = ShellExecuteEx(&shExecInfo);
                    err = GetLastError();
                }
            }
        }

        if (ok && (wait_for_process || keep_alive))
            hNewProcess = shExecInfo.hProcess;
        else if(ok && force_children_on_this_program)
            pi.dwProcessId = GetProcessId(shExecInfo.hProcess);

        if (! ok) {

            //
            // otherwise try to run it as a command line
            //

            expanded = MyHeapAlloc(8192 * sizeof(WCHAR));
            ExpandEnvironmentStrings(cmdline, expanded, 8192);

            ok = CreateProcess(
                NULL, expanded, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

            err = ok ? 0 : GetLastError();

            if (ok && (wait_for_process || keep_alive))
                hNewProcess = pi.hProcess;
        }

    } while (0);

    //
    // if successful, then wait for SandboxieRpcSs initialization to complete
    // because we don't want to release the ServiceInitComplete event before
    // we know for sure that SandboxieRpcSs has opened it
    //

    if (ok) {

        if (force_children_on_this_program) {

            SbieApi_Call(API_FORCE_CHILDREN, 2, pi.dwProcessId, BoxName);

        } else if (!disable_force_on_this_program) {

            SbieDll_StartCOM(FALSE);
        }
    }

    //
    // report error
    //

    if (! ok) {

        WCHAR *errmsg = SbieDll_FormatMessage1(MSG_3205, cmdline);
        SetLastError(err);
        Show_Error(errmsg);

        keep_alive = FALSE; // disable keep alive when the process can't be started in the first place
            
        return EXIT_FAILURE;

    } else if (hNewProcess) {

        if (WaitForSingleObject(hNewProcess, INFINITE) == WAIT_OBJECT_0) {

            ok = GetExitCodeProcess(hNewProcess, &err);
            if (ok) {
                if (keep_alive)
                    return err;
                ExitProcess(err);
            }
        }

    } 
    // $Workaround$ - 3rd party fix
    else if (GetModuleHandle(L"protect.dll")) {

        //
        // hack for FortKnox firewall -- keep Start.exe around for a few
        // more seconds in case the firewall tries to get our process name
        //

        Sleep(2 * 1000);
    }

    return EXIT_SUCCESS;
}


//---------------------------------------------------------------------------
// StartAutoRunKey
//---------------------------------------------------------------------------


void StartAutoRun(const WCHAR* Name, const WCHAR* Cmd)
{
    SbieDll_RunStartExe(Cmd, NULL);
}


//---------------------------------------------------------------------------
// StartAutoRunKey
//---------------------------------------------------------------------------


void StartAutoRunKey(LPCWSTR lpKey)
{
    WCHAR* OutTruePath;
    WCHAR* OutCopyPath;
    UNICODE_STRING objname;
    RtlInitUnicodeString(&objname, lpKey);
    Key_GetName(NULL, &objname, &OutTruePath, &OutCopyPath, NULL);


	OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING RegistryPath;
	HANDLE hKey = NULL;
    const ULONG BufferSize = sizeof(KEY_VALUE_FULL_INFORMATION) + MAX_PATH;
	PUCHAR Buffer[BufferSize];
    PKEY_VALUE_FULL_INFORMATION valueInfo = (PKEY_VALUE_FULL_INFORMATION)Buffer;
	ULONG RequiredSize;
	ULONG i = 0;
	UNICODE_STRING Name;
	UNICODE_STRING Data;
	NTSTATUS Status;

    // Get the native unhooked function in order to enumerate only the sandboxed entries
    P_NtOpenKey __sys_NtOpenKey = (P_NtOpenKey)SbieDll_GetSysFunction(L"NtOpenKey");
    P_NtEnumerateValueKey __sys_NtEnumerateValueKey = (P_NtEnumerateValueKey)SbieDll_GetSysFunction(L"NtEnumerateValueKey");

    RtlInitUnicodeString(&RegistryPath, OutCopyPath);
	InitializeObjectAttributes(&ObjectAttributes, &RegistryPath, 0, NULL, NULL);
	Status = __sys_NtOpenKey(&hKey, KEY_QUERY_VALUE, &ObjectAttributes);
	if (!NT_SUCCESS(Status))
		return;

	while (TRUE) {
		Status = __sys_NtEnumerateValueKey(hKey, i++, KeyValueFullInformation, valueInfo, sizeof(Buffer), &RequiredSize);
		if (Status == STATUS_BUFFER_OVERFLOW)
			continue;
		else if (!NT_SUCCESS(Status))
			break;
		else if (valueInfo->Type != REG_SZ)
			continue;

		Name.Length = Name.MaximumLength = (USHORT)valueInfo->NameLength;
		Name.Buffer = valueInfo->Name;

		Data.Length = Data.MaximumLength = (USHORT)valueInfo->DataLength;
		Data.Buffer = (PWCHAR)((ULONG_PTR)valueInfo + valueInfo->DataOffset);
		if (Data.Length > sizeof(WCHAR) && Data.Buffer[Data.Length / sizeof(WCHAR) - 1] == UNICODE_NULL)
			Data.Length -= sizeof(WCHAR);

		StartAutoRun(Name.Buffer, Data.Buffer);
	}

	NtClose(hKey);
}


//---------------------------------------------------------------------------
// StartAutoRunKey
//---------------------------------------------------------------------------


void StartLink(const WCHAR* Path, const WCHAR* Name)
{
    WCHAR path[MAX_PATH];
    wcscpy(path, Path + 4); // skip \??\ prefix
    if (path[wcslen(path)] != L'\\')
        wcscat(path, L"\\");
    wcscat(path, Name);

    /*WCHAR *ptr = wcsrchr(path, L'.');
    if (! ptr)
        return;
    if (_wcsicmp(ptr, L".lnk") != 0 /&& _wcsicmp(ptr, L".url") != 0/)
        return;

    IShellLink *pShellLink;
    IPersistFile *pPersistFile;
    void GetLinkInstance(const WCHAR * path, IShellLink * *ppShellLink, IPersistFile * *ppPersistFile, bool msi);
    GetLinkInstance(path, &pShellLink, &pPersistFile, false);
    if ((! pShellLink) || (! pPersistFile))
        return;

    DWORD buflen = 10240;
    WCHAR *buf = (WCHAR *)MyHeapAlloc(buflen);

    HRESULT hr = pShellLink->GetPath(buf, buflen / sizeof(WCHAR) - 1, NULL, 0);
    if (SUCCEEDED(hr)) {

        StartAutoRun(Name, buf);
    }

    MyHeapFree(buf);

    pPersistFile->Release();
    pShellLink->Release();*/

    StartAutoRun(Name, path); // we can use the link directly
}


//---------------------------------------------------------------------------
// StartAutoRunKey
//---------------------------------------------------------------------------


void StartAutoAutoFolder(LPCWSTR lpPath)
{
    WCHAR* OutTruePath;
    WCHAR* OutCopyPath;
    UNICODE_STRING objname;
    RtlInitUnicodeString(&objname, lpPath);
    File_GetName(NULL, &objname, &OutTruePath, &OutCopyPath, NULL);

    UNICODE_STRING RootDirectoryName;
	OBJECT_ATTRIBUTES RootDirectoryAttributes;
	NTSTATUS ntStatus = STATUS_SUCCESS;
	HANDLE RootDirectoryHandle;
	IO_STATUS_BLOCK Iosb;
	//HANDLE Event;
    PFILE_ID_BOTH_DIR_INFORMATION DirInformation;

    // Get the native unhooked function in order to enumerate only the sandboxed entries
    P_NtCreateFile __sys_NtCreateFile = (P_NtCreateFile)SbieDll_GetSysFunction(L"NtCreateFile");
    P_NtQueryDirectoryFile __sys_NtQueryDirectoryFile = (P_NtQueryDirectoryFile)SbieDll_GetSysFunction(L"NtQueryDirectoryFile");
	
	RtlInitUnicodeString(&RootDirectoryName, OutCopyPath);
	InitializeObjectAttributes(&RootDirectoryAttributes, &RootDirectoryName, OBJ_CASE_INSENSITIVE, 0, 0);
	ntStatus = __sys_NtCreateFile(&RootDirectoryHandle,
		GENERIC_READ, &RootDirectoryAttributes, &Iosb, 0, FILE_ATTRIBUTE_DIRECTORY,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, FILE_OPEN, FILE_DIRECTORY_FILE, 0, 0);
	if (!NT_SUCCESS(ntStatus))
		return;
	
	//ntStatus = NtCreateEvent(&Event, GENERIC_ALL, 0, NotificationEvent, FALSE);
	//if (!NT_SUCCESS(ntStatus))
	//	return;

    DirInformation = (PFILE_ID_BOTH_DIR_INFORMATION)MyHeapAlloc(65536);

	if (__sys_NtQueryDirectoryFile(RootDirectoryHandle, 0, //Event,
		0, 0, &Iosb, DirInformation, 65536, FileIdBothDirectoryInformation, FALSE, NULL, FALSE) == STATUS_PENDING)
	{
		//ntStatus = NtWaitForSingleobject(Event, TRUE, 0);
	}
    if (NT_SUCCESS(ntStatus))
    {
        while (1)
        {
            UNICODE_STRING EntryName;
            EntryName.MaximumLength = EntryName.Length = (USHORT)DirInformation->FileNameLength;
            EntryName.Buffer = &DirInformation->FileName[0];
            if ((DirInformation->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                StartLink(lpPath, EntryName.Buffer);
            }
            if (0 == DirInformation->NextEntryOffset)
                break;
            else
                DirInformation = (PFILE_ID_BOTH_DIR_INFORMATION)(((PUCHAR)DirInformation) + DirInformation->NextEntryOffset);
        }
    }
    MyHeapFree(DirInformation);

	NtClose(RootDirectoryHandle);
}


//---------------------------------------------------------------------------
// StartAllAutoRunEntries
//---------------------------------------------------------------------------


void StartAllAutoRunEntries()
{
    //while (! IsDebuggerPresent())
    //    Sleep(500);
    //__debugbreak();

    // Start boxed services
    // todo

    // start auto run registry
    StartAutoRunKey(L"\\REGISTRY\\Machine\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
    StartAutoRunKey(L"\\REGISTRY\\Machine\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce");
    //StartAutoRunKey(L"\\REGISTRY\\Machine\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx");
    StartAutoRunKey(L"\\REGISTRY\\Machine\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run");
    StartAutoRunKey(L"\\REGISTRY\\Machine\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\RunOnce");
    //StartAutoRunKey(L"\\REGISTRY\\Machine\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx");

    //HKLM\Software\Microsoft\Windows\CurrentVersion\RunServices
    //HKLM\Software\Microsoft\Windows\CurrentVersion\RunServicesOnce

    StartAutoRunKey(L"\\REGISTRY\\User\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
    StartAutoRunKey(L"\\REGISTRY\\User\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce");
    //StartAutoRunKey(L"\\REGISTRY\\User\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx");
    StartAutoRunKey(L"\\REGISTRY\\User\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run");
    StartAutoRunKey(L"\\REGISTRY\\User\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\RunOnce");
    //StartAutoRunKey(L"\\REGISTRY\\User\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx");
    //HKCU\Software\Microsoft\Windows NT\CurrentVersion\Windows\Run

    //HKCU\Software\Microsoft\Windows\CurrentVersion\RunServices
    //HKCU\Software\Microsoft\Windows\CurrentVersion\RunServicesOnce

    // start auto start menu
    WCHAR AutoStartPath[MAX_PATH + 1] = L"\\??\\";
    SHGetSpecialFolderPath(NULL, AutoStartPath + 4, CSIDL_COMMON_STARTUP, FALSE);
    StartAutoAutoFolder(AutoStartPath);
    SHGetSpecialFolderPath(NULL, AutoStartPath + 4, CSIDL_STARTUP, FALSE);
    StartAutoAutoFolder(AutoStartPath);
}


//---------------------------------------------------------------------------
// GetParentPIDAndName
//---------------------------------------------------------------------------

extern "C" WINBASEAPI BOOL WINAPI QueryFullProcessImageNameW(HANDLE hProcess, DWORD dwFlags, LPWSTR lpExeName, PDWORD lpdwSize);

DWORD GetParentPIDAndName(DWORD ProcessID, LPTSTR lpszBuffer_Parent_Name) 
{
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, ProcessID);
	if (!ProcessID) 
		return 0;

	PROCESS_BASIC_INFORMATION pbi;
	NTSTATUS status = NtQueryInformationProcess(hProcess, ProcessBasicInformation, (LPVOID)&pbi, sizeof(pbi), NULL);

	DWORD dwParentID = 0;
	if (NT_SUCCESS(status)) {
		
		dwParentID = (DWORD)pbi.InheritedFromUniqueProcessId;

		if (NULL != lpszBuffer_Parent_Name) {

			HANDLE hParentProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwParentID);
            if (hParentProcess) {

                DWORD dwSize;
                BOOL ret = QueryFullProcessImageNameW(hParentProcess, 0, lpszBuffer_Parent_Name, &dwSize);

                CloseHandle(hParentProcess);
            }
		}
	}

	CloseHandle(hProcess);
	return dwParentID;
}


//---------------------------------------------------------------------------
// RestartInSandbox
//---------------------------------------------------------------------------


ULONG RestartInSandbox(void)
{
    WCHAR *dir, *cmd, *ptr;
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    ULONG err;
    BOOL ok;

    memzero(&pi, sizeof(PROCESS_INFORMATION));
    memzero(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);

    si.dwFlags = STARTF_FORCEOFFFEEDBACK;
    if (GetStartupInfo_wShowWindow != SW_SHOWNORMAL) {
        si.dwFlags |= STARTF_USESHOWWINDOW;
        si.wShowWindow = GetStartupInfo_wShowWindow;
    }

    // we should have foreground rights at this point, but the second
    // instance of Start.exe that we run through SbieDll_RunSandboxed
    // does not inherit our foreground rights automatically

    AllowSetForegroundWindow(ASFW_ANY);

    //
    // build command line.  note that we pass the current directory as an
    // environment variable which will be queried by Program_Start.  this
    // is because SbieSvc ProcessServer (used by SbieDll_RunSandboxed)
    // does not necessarily share our dos device map, and will not be able
    // to change to a drive letter that isn't in its dos device map
    //

    dir = (WCHAR *)MyHeapAlloc(1024 * sizeof(WCHAR));
    GetCurrentDirectory(1020, dir);

    cmd = (WCHAR *)MyHeapAlloc(16384 * sizeof(WCHAR));
    cmd[0] = L'\"';
    GetModuleFileName(NULL, cmd + 1, MAX_PATH);
    ptr = cmd + wcslen(cmd);
    wcscpy(ptr, L"\" /env:00000000_" SBIE L"_CURRENT_DIRECTORY=\"");
    wcscat(ptr, dir);
    wcscat(ptr, L"\" /env:=Refresh ");
    ptr += wcslen(ptr);
    if (run_elevated_2) {
        wcscpy(ptr, L"/elevate ");
        ptr = cmd + wcslen(cmd);
    }
    if (wait_for_process) {
        wcscpy(ptr, L"/wait ");
        ptr = cmd + wcslen(cmd);
    }
    wcscpy(ptr, ChildCmdLine);

    SbieApi_GetHomePath(NULL, 0, dir, 1020);

    //
    //
    //

	if (SbieApi_QueryConfBool(BoxName, L"AlertBeforeStart", FALSE)) {

        WCHAR parent_image[1020] = L"";
		GetParentPIDAndName(GetCurrentProcessId(), parent_image);

		WCHAR* text = SbieDll_FormatMessage1(MSG_3198, BoxName);
		if (MessageBoxW(NULL, text, Sandboxie_Start_Title, MB_YESNO) == IDNO)
			return EXIT_FAILURE;

        if (_wcsnicmp(parent_image, dir, wcslen(dir)) != 0) {
            if (MessageBoxW(NULL, SbieDll_FormatMessage0(3199), Sandboxie_Start_Title, MB_YESNO) == IDNO)
                return EXIT_FAILURE;
        }
	}

    //
    //
    //

    ULONG crflags = 0;
    if (fake_admin)
        crflags |= CREATE_SECURE_PROCESS; // repurpose this flag for the fake admin as its not valid in a sandboxed context
    ok = SbieDll_RunSandboxed(BoxName, cmd, dir, crflags, &si, &pi);
    err = GetLastError();

    if (! ok) {

        if (err == ERROR_SERVER_DISABLED) {

            SetLastError(0);
            Show_Error(SbieDll_FormatMessage0(MSG_3212));
            return EXIT_FAILURE;

        } else {

            return EXIT_FAILURE;
        }
    }

    if (! ok) {
        WCHAR *errmsg = SbieDll_FormatMessage1(MSG_3205, ChildCmdLine);
        SetLastError(err);
        Show_Error(errmsg);

    } else {

        MSG msg;
        PostThreadMessage(GetCurrentThreadId(), WM_NULL, 0, 0);
        GetMessage(&msg, NULL, 0, 0);       // turn off "feedback cursor"

        if (WaitForSingleObject(pi.hProcess, INFINITE) == WAIT_OBJECT_0) {

            ok = GetExitCodeProcess(pi.hProcess, &err);
            if (ok)
                return err;
        }
    }

    return EXIT_FAILURE;
}


//---------------------------------------------------------------------------
// WinMain
//---------------------------------------------------------------------------


int __stdcall WinMainCRTStartup(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    int rc;
    USHORT KeyState;
    STARTUPINFO si;

    Sandboxie_Start_Title = SbieDll_FormatMessage0(MSG_3101);
    SbieDll_GetLanguage(&layout_rtl);

    if(!NT_SUCCESS(SbieApi_QueryConfAsIs(L"GlobalSettings", L"DefaultBox", 0, BoxName, sizeof(BoxName))) || *BoxName == L'\0')
        wcscpy(BoxName, L"DefaultBox");

    Token_Elevation_Type = SbieDll_GetTokenElevationType();

    //
    // keep the nShowCmd flag that was passed to us
    //

    si.cb = sizeof(STARTUPINFO);
    GetStartupInfo(&si);

    if (si.dwFlags & STARTF_USESHOWWINDOW)
        GetStartupInfo_wShowWindow = si.wShowWindow;

    //
    // check if disabled force mode is activated (Ctrl+Shift)
    //    or if DefaultBox auto selection (just Ctrl)
    // either applies only when started as a window or from a shortcut
    // then process command line
    //

    KeyState = GetAsyncKeyState(VK_CONTROL);
    if (KeyState & 0x8000) {

        if (si.dwFlags & (STARTF_USESHOWWINDOW | STARTF_TITLEISLINKNAME)) {

            KeyState = GetAsyncKeyState(VK_SHIFT);
            if (KeyState & 0x8000)
                disable_force_on_this_program = TRUE;
            else
                auto_select_default_box = TRUE;
        }
    }

    //
    // set explicit AppUserModelID for Windows 7 taskbar
    //

    if (1) {

        HMODULE shell32 = LoadLibrary(L"shell32.dll");
        if (shell32) {
            typedef HRESULT (*P_Set)(const WCHAR *AppId);
            P_Set pSet = (P_Set)GetProcAddress(
                shell32, "SetCurrentProcessExplicitAppUserModelID");
            if (pSet)
                pSet(SANDBOXIE L".Start");
        }
    }

    //
    // parse command line
    //

    if (! Parse_Command_Line())
        return die(EXIT_FAILURE);

    if (! SbieApi_QueryProcessInfo(
                    (HANDLE)(ULONG_PTR)GetCurrentProcessId(), 0)) {
        //
        // we are not yet sandboxed, restart ourselves sandboxed
        // or apply "disabled forced program" state
        //

        if (BoxName[0] != L'-') {

            if (! display_start_menu)
                boxdlg_run_outside_sandbox = TRUE;

            Validate_Box_Name();

            if (disable_force_on_this_program) {

                ULONG NewState = DISABLE_JUST_THIS_PROCESS;
                SbieApi_DisableForceProcess(&NewState, NULL);
            }
            if (disable_force_on_this_program || force_children_on_this_program)
                return die(Program_Start());
        }

        return die(RestartInSandbox());
    }

    //
    //
    //

    while (1) {

        if (display_run_dialog) {
            MyCoInitialize();
            ChildCmdLine = DoRunDialog(GetModuleHandle(NULL));
            if (ChildCmdLine)
            {
                PWSTR pBegin = ChildCmdLine;
                PWSTR pEnd = Eat_String(ChildCmdLine);
                if (*ChildCmdLine == L'\"' && pEnd - ChildCmdLine > 2)
                    pBegin++;
				while (pBegin < --pEnd && *pEnd != L'\\');
				
				ChildWrkDir = (WCHAR *)HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, (pEnd - pBegin + 2) * sizeof(WCHAR));
				wmemcpy(ChildWrkDir, pBegin, pEnd - pBegin + 1);
				ChildWrkDir[pEnd - pBegin] = L'\0';
            }
        } else if (execute_open_with) {
            MyCoInitialize();
            WCHAR* CmdLine = DoRunDialog(GetModuleHandle(NULL));
            if (!CmdLine) // !CmdLine -> cancel
                ChildCmdLine = CmdLine;
            else { // execute_open_with
                WCHAR* FilePath = ChildCmdLine;
                DWORD len = wcslen(CmdLine) + 1 + wcslen(FilePath) + 16;
                ChildCmdLine = MyHeapAlloc(len * sizeof(WCHAR));
                wsprintf(ChildCmdLine, L"%s %s", CmdLine, FilePath);
                MyHeapFree(CmdLine);
                MyHeapFree(FilePath);
            }
        } else if (display_start_menu) {
            if (! ChildCmdLine)
                ChildCmdLine = DoStartMenu();
        } else if (execute_auto_run) {
            StartAllAutoRunEntries();
        }

        if (! ChildCmdLine) {
            rc = EXIT_FAILURE;
            break;
        }

        if (StartMenuSectionName) {
            if (WriteStartMenuResult(StartMenuSectionName, ChildCmdLine))
                rc = EXIT_SUCCESS;
            else
                rc = EXIT_FAILURE;
            break;
        }

        if (! ResolveDirectory(ChildCmdLine)) {
            rc = EXIT_FAILURE;
            break;
        }

        UINT64 start;
        int retry = 0;

    run_program:

        start = ::GetTickCount();

        rc = Program_Start();

        // keep the process running unless it gracefully terminates
        if (keep_alive && rc != EXIT_SUCCESS && retry++ < 5) { // after to many initialization failures abort
            if (::GetTickCount() - start >= 5000) // if the process run for less than 5 seconds consider it a failure to initialize
                retry = 0; // reset failure counter on success ful start
            goto run_program;
        }

        if (! display_run_dialog || rc == EXIT_SUCCESS)
            break;
    }

    return die(rc);
}


int __stdcall WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    /*while (! IsDebuggerPresent())
        Sleep(500);
    __debugbreak();*/
    return WinMainCRTStartup(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}
