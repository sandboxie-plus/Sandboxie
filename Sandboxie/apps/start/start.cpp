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
#include "core/svc/SbieIniWire.h"
#include "common/my_version.h"
#include "common/defines.h"
#include "msgs/msgs.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define MyHeapAlloc(len) \
    (WCHAR*)HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, (len))


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


void List_Process_Ids(void);
int Terminate_All_Processes(BOOL all_boxes);
BOOLEAN Register_Process(void);

extern WCHAR *DoRunDialog(HINSTANCE hInstance);
extern WCHAR *DoBoxDialog(void);
extern BOOL ResolveDirectory(WCHAR *PathW);
extern WCHAR *DoStartMenu(void);
extern BOOL WriteStartMenuResult(const WCHAR *MapName, const WCHAR *Command);
extern void DeleteSandbox(
    const WCHAR *BoxName, BOOL bLogoff, BOOL bSilent, int phase);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


WCHAR BoxName[34];

PWSTR ChildCmdLine = NULL;
BOOL run_mail_agent = FALSE;
BOOL display_run_dialog = FALSE;
BOOL display_start_menu = FALSE;
BOOL run_elevated_2 = FALSE;
BOOL disable_force_on_this_program = FALSE;
BOOL auto_select_default_box = FALSE;
WCHAR *StartMenuSectionName = NULL;
BOOL run_silent = FALSE;
BOOL dont_start_sbie_ctrl = FALSE;
BOOL process_was_registered = FALSE;
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

        if (_wcsicmp(BoxName, L"__ask__") == 0 ||
            _wcsicmp(BoxName, L"current") == 0) {

            if (auto_select_default_box) {
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
    static const WCHAR *delete_sandbox    = L"delete_sandbox";
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

    if (_wcsicmp(cmd, L"run_sbie_ctrl") == 0) {

        MSG_HEADER req, *rpl = NULL;
        if (CallSbieSvcGetUser()) {
            req.length = sizeof(req);
            req.msgid  = MSGID_SBIE_INI_RUN_SBIE_CTRL;
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

            if (tmp == cmd || (cmd - tmp > 32)) {

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
        // Command line switch /nosbiectrl
        //

        } else if (_wcsnicmp(cmd, L"nosbiectrl", 10) == 0) {

            cmd += 10;

            dont_start_sbie_ctrl = TRUE;

        //
        // Command line switch /reload
        //

        } else if (_wcsnicmp(cmd, L"reload", 6) == 0) {

            SbieApi_ReloadConf(-1);
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
        // Command line switch /elevate
        //

        } else if (_wcsnicmp(cmd, L"elevate", 7) == 0) {

            cmd = Eat_String(cmd);

            run_elevated_2 = TRUE;

        //
        // Command line switch /disable_force or /dfp
        //

        } else if (_wcsnicmp(cmd, L"disable_force", 13) == 0 ||
                   _wcsnicmp(cmd, L"dfp",            3) == 0) {

            cmd = Eat_String(cmd);

            disable_force_on_this_program = TRUE;

        //
        // Command line switch /elevate
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

        if (! SbieApi_QueryProcessInfo(
                        (HANDLE)(ULONG_PTR)GetCurrentProcessId(), 0)) {
            // this is the instance of Start.exe outside the sandbox
            // so just resend the start_menu command line to the
            // instance that will restart in the sandbox
            ChildCmdLine = cmd;

        } else {

            // this is the instance of Start.exe in the sandbox so we
            // need to parse the start_menu command line
            WCHAR *colon = cmd + wcslen(start_menu);
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

        display_start_menu = TRUE;

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
    // otherwise pass the rest of the command line as-is to the child
    //

    if (*cmd == L'\0') {
        SetLastError(0);
        Show_Error(SbieDll_FormatMessage0(MSG_3203));
        return FALSE;
    }

    ChildCmdLine = (WCHAR *)MyHeapAlloc(10240 * sizeof(WCHAR));
    wcscpy(ChildCmdLine, cmd);

    return TRUE;
}


//---------------------------------------------------------------------------
// List_Process_Ids
//---------------------------------------------------------------------------


void List_Process_Ids(void)
{
    CHAR msg[32];
    ULONG len, i;
    ULONG pids[512];

    ULONG rc = SbieApi_EnumProcess(BoxName, pids);
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    if (rc != 0 || (! out))
        return;

    sprintf(msg, "%d\r\n", pids[0]);
    WriteFile(out, msg, strlen(msg), &len, NULL);

    for (i = 1; i <= pids[0]; ++i) {
        sprintf(msg, "%d\r\n", pids[i]);
        WriteFile(out, msg, strlen(msg), &len, NULL);
    }
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
    // copy input command line, minus leading and tailing spaces
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

    curdir = (WCHAR *)MyHeapAlloc(1024 * sizeof(WCHAR));
    if (GetEnvironmentVariable(
                    L"00000000_" SBIE L"_CURRENT_DIRECTORY", curdir, 1020)) {

        SetCurrentDirectory(curdir);
        SetEnvironmentVariable(
                    L"00000000_" SBIE L"_CURRENT_DIRECTORY", NULL);
    }

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
        if (wait_for_process)
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
        // if Online Armor is part of the process, then invoke Start.exe
        // recursively, that somehow fixes the problem
        //

        if (process_was_registered && GetModuleHandle(L"oawatch.dll")) {

            if (SbieDll_RunFromHome(START_EXE, cmdline, &si, &pi)) {

                ok = TRUE;
                err = 0;

                break;
            }
        }

        //
        // make sure AppHelp.dll is loaded, so third party software like
        // EMET which relies on injection through ShimEng/AppHelp can work.
        // note:  don't use a full path to load AppHelp.dll or otherwise
        // the ClosedFilePath setting from core/drv/file.c will apply
        //

        LoadLibrary(L"apphelp.dll");

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

        if ((! ok) && (err == ERROR_NO_ASSOCIATION) &&
            process_was_registered && run_elevated_2) {

            WCHAR *start_exe = MyHeapAlloc((MAX_PATH + 8) * sizeof(WCHAR));

            GetModuleFileName(NULL, start_exe, MAX_PATH);

            shExecInfo.lpFile = start_exe;
            shExecInfo.lpParameters = cmdline;

            ok = ShellExecuteEx(&shExecInfo);
            err = GetLastError();

            shExecInfo.lpFile = cmdline;
            shExecInfo.lpParameters = NULL;
        }

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

                WCHAR *cmd2 = MyHeapAlloc(cmdline_len * sizeof(WCHAR));
                WCHAR *arg2 = MyHeapAlloc(cmdline_len * sizeof(WCHAR));

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

        if (ok && wait_for_process)
            hNewProcess = shExecInfo.hProcess;

        if (! ok) {

            //
            // otherwise try to run it as a command line
            //

            expanded = MyHeapAlloc(8192 * sizeof(WCHAR));
            ExpandEnvironmentStrings(cmdline, expanded, 8192);

            ok = CreateProcess(
                NULL, expanded, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

            err = ok ? 0 : GetLastError();

            if (ok && wait_for_process)
                hNewProcess = pi.hProcess;
        }

    } while (0);

    //
    // if successful, then wait for SandboxieRpcSs initialization to complete
    // because we don't want to release the ServiceInitComplete event before
    // we know for sure that SandboxieRpcSs has opened it
    //

    if (ok && (! disable_force_on_this_program)) {

        SbieDll_StartCOM(FALSE);
    }

    //
    // report error
    //

    if (! ok) {

        WCHAR *errmsg = SbieDll_FormatMessage1(MSG_3205, cmdline);
        SetLastError(err);
        Show_Error(errmsg);

        return EXIT_FAILURE;

    } else if (hNewProcess) {

        if (WaitForSingleObject(hNewProcess, INFINITE) == WAIT_OBJECT_0) {

            ok = GetExitCodeProcess(hNewProcess, &err);
            if (ok)
                ExitProcess(err);
        }

    } else if (GetModuleHandle(L"protect.dll")) {

        //
        // hack for FortKnox firewall -- keep Start.exe around for a few
        // more seconds in case the firewall tries to get our process name
        //

        Sleep(2 * 1000);
    }

    return EXIT_SUCCESS;
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
    // does not necssarily share our dos device map, and will not be able
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

    ok = SbieDll_RunSandboxed(BoxName, cmd, dir, 0, &si, &pi);
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
                return die(Program_Start());
            }
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
        } else if (display_start_menu) {
            if (! ChildCmdLine)
                ChildCmdLine = DoStartMenu();
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

        rc = Program_Start();

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
    return WinMainCRTStartup(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}
