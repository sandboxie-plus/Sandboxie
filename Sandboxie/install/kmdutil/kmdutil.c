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
// KmdUtil
//---------------------------------------------------------------------------

#include "stdafx.h"

#include <shellapi.h>
#include <stdlib.h>
#include "common/defines.h"
#include "common/my_version.h"

extern void Kmd_ScanDll(BOOLEAN silent);


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

typedef enum _COMMAND {
    CMD_ERROR,
    CMD_DELETE,
    CMD_INSTALL,
    CMD_START,
    CMD_STOP,
    CMD_SCANDLL,
    CMD_SCANDLL_SILENT
} COMMAND;

typedef enum _OPTIONS {
    OPT_NONE = 0,
    OPT_SYSTEM_START = 1,
    OPT_AUTO_START = 2,
    OPT_DEMAND_START = 4,
    OPT_OWN_TYPE = 8,
    OPT_KERNEL_TYPE = 16,
    OPT_DISPLAY = 128,
    OPT_MSGFILE = 256,
    OPT_ALTITUDE = 512,
    OPT_GROUP = 1024,
    OPT_ERROR = 0xFFFFFFFF
} OPTIONS;

//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------

SC_HANDLE ScMgr;

static ULONG LangParm = 0;

//---------------------------------------------------------------------------
// SbieDll_GetLanguage
//---------------------------------------------------------------------------

ULONG SbieDll_GetLanguage(BOOLEAN *rtl)
{
    if (rtl) {
        if (LangParm == 1037 || LangParm == 1025)
            *rtl = TRUE;
        else
            *rtl = FALSE;
    }
    return LangParm;
}

#define SBIEDLL_FORMATMESSAGE_ONLY

#define SBIEDLL_GET_SBIE_MSG_DLL                        \
    WCHAR path[MAX_PATH + 8], *ptr;                     \
    memzero(path, sizeof(path));                        \
    GetModuleFileName(NULL, path, MAX_PATH);            \
    ptr = wcsrchr(path, L'\\');                         \
    if (ptr) {                                          \
        wcscpy(ptr + 1, SBIEMSG_DLL);                   \
        SbieMsgDll = LoadLibrary(path);                 \
    }

#include "core/dll/support.c"

//---------------------------------------------------------------------------
// Display_Error
//---------------------------------------------------------------------------

void Display_Error(PWSTR SubFuncName, DWORD LastError)
{
    wchar_t Text[512];
    PWSTR ErrorText;
    DWORD FormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS;

    if (! LastError)
        LastError = GetLastError();

    if (LastError == ERROR_SERVICE_MARKED_FOR_DELETE) {

        wcscpy(Text, SbieDll_FormatMessage1(8101, SubFuncName));

    } else if (LastError) {

        FormatMessage(FormatFlags, NULL, LastError,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      (LPTSTR)&ErrorText, 0, NULL);

        swprintf(Text, L"%s: %s (%d)", SubFuncName, ErrorText, LastError);

    } else {

        wcscpy(Text, SubFuncName);
    }

    MessageBox(NULL, Text, L"KmdUtil", MB_ICONEXCLAMATION | MB_OK);
}

//---------------------------------------------------------------------------
// Parse_Option
//---------------------------------------------------------------------------

OPTIONS Parse_Option(PWSTR str)
{
    if (_wcsicmp(str, L"start=system") == 0)
        return OPT_SYSTEM_START;
    else if (_wcsicmp(str, L"start=auto") == 0)
        return OPT_AUTO_START;
    else if (_wcsicmp(str, L"start=demand") == 0)
        return OPT_DEMAND_START;
    else if (_wcsicmp(str, L"type=own") == 0)
        return OPT_OWN_TYPE;
    else if (_wcsicmp(str, L"type=kernel") == 0)
        return OPT_KERNEL_TYPE;
    else if (_wcsnicmp(str, L"display=", 8) == 0)
        return OPT_DISPLAY;
    else if (_wcsnicmp(str, L"msgfile=", 8) == 0)
        return OPT_MSGFILE;
    else if (_wcsnicmp(str, L"altitude=", 9) == 0)
        return OPT_ALTITUDE;
    else if (_wcsnicmp(str, L"group=", 6) == 0)
        return OPT_GROUP;
    else
        return OPT_ERROR;
}

//---------------------------------------------------------------------------
// Parse_Command_Line
//---------------------------------------------------------------------------

BOOL Parse_Command_Line(
    COMMAND *Command,
    PWSTR *Driver_Name,
    PWSTR *Driver_Path,
    PWSTR *Driver_Display,
    PWSTR *Driver_MsgFile,
    PWSTR *Driver_Altitude,
    PWSTR *Driver_Group,
    OPTIONS *Options)
{
    PWSTR *xargs;
    WCHAR *args[128];
    int num_args_given;
    int num_args_needed;
    int next_arg;

    xargs = CommandLineToArgvW(GetCommandLine(), &num_args_given);

    memcpy(args, xargs, sizeof(ULONG_PTR) * num_args_given);

    if (num_args_given > 1 && _wcsnicmp(args[1], L"/lang=", 6) == 0) {
        LangParm = _wtoi(args[1] + 6);
        --num_args_given;
        memcpy(&args[1], &xargs[2],
               sizeof(ULONG_PTR) * (num_args_given - 1));
    }

    if (num_args_given == 1) {
        *Command = CMD_ERROR;
        MessageBox(NULL, L"Missing command", L"KmdUtil",
                   MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }

    if (_wcsnicmp(args[1], L"scandll", 7) == 0) {
        *Command = CMD_SCANDLL;
        if (_wcsicmp(args[1] + 7, L"_silent") == 0)
            *Command = CMD_SCANDLL_SILENT;
        num_args_needed = 0;

    } else if (_wcsicmp(args[1], L"delete") == 0) {
        *Command = CMD_DELETE;
        num_args_needed = 1;

    } else if (_wcsicmp(args[1], L"install") == 0) {
        *Command = CMD_INSTALL;
        num_args_needed = 2;

    } else if (_wcsicmp(args[1], L"start") == 0) {
        *Command = CMD_START;
        num_args_needed = 1;

    } else if (_wcsicmp(args[1], L"stop") == 0) {
        *Command = CMD_STOP;
        num_args_needed = 1;

    } else {
        *Command = CMD_ERROR;
        MessageBox(NULL, L"Invalid command", L"KmdUtil",
                   MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }

    if ((num_args_given - 2) < num_args_needed) {
        *Command = CMD_ERROR;
        MessageBox(NULL, L"Not enough arguments", L"KmdUtil",
                   MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }

    if (num_args_needed >= 1)
        *Driver_Name = args[2];
    if (num_args_needed >= 2)
        *Driver_Path = args[3];

    *Options = OPT_NONE;
    next_arg = num_args_needed + 2;
    while (next_arg < num_args_given) {
        WCHAR *arg = args[next_arg];
        OPTIONS opt = Parse_Option(arg);
        if (opt & OPT_DISPLAY)
            *Driver_Display = arg + 8;
        else if (opt & OPT_MSGFILE)
            *Driver_MsgFile = arg + 8;
        else if (opt & OPT_ALTITUDE)
            *Driver_Altitude = arg + 9;
        else if (opt & OPT_GROUP)
            *Driver_Group = arg + 6;
        else
            (*Options) |= opt;
        ++next_arg;
    }

    if (*Options == OPT_ERROR) {
        *Command = CMD_ERROR;
        MessageBox(NULL, L"Invalid option", L"KmdUtil",
                   MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }

    return TRUE;
}

//---------------------------------------------------------------------------
// Kmd_Open_Services_Key
//---------------------------------------------------------------------------

HKEY Kmd_Open_Services_Key(const WCHAR *subkey)
{
    WCHAR str[512];
    ULONG retcode;
    HKEY hkey;

    wcscpy(str, L"System\\CurrentControlSet\\Services\\");
    wcscat(str, subkey);

    retcode = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE, str, 0, KEY_READ | KEY_WRITE, &hkey);

    if (retcode != 0) {

        wcscpy(str, L"RegOpenKeyEx (Services\\");
        wcscat(str, subkey);
        wcscat(str, L")");
        Display_Error(str, retcode);

        hkey = NULL;
    }

    return hkey;
}

//---------------------------------------------------------------------------
// Kmd_Delete_Service
//---------------------------------------------------------------------------

BOOL Kmd_Delete_Service(
    const wchar_t *Driver_Name)
{
    SC_HANDLE service;

    // try to open the service for the driver

    service = OpenService(
        ScMgr, Driver_Name, SERVICE_INTERROGATE | DELETE);

    if (! service) {

        if (GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST)
            return TRUE;

        Display_Error(L"OpenService", 0);
        return FALSE;
    }

    // if the service opened successfully, we must delete it

    if (! DeleteService(service)) {
        if (GetLastError() != ERROR_SERVICE_MARKED_FOR_DELETE) {
            Display_Error(L"DeleteService", 0);
            return FALSE;
        }
    }

    return TRUE;
}

//---------------------------------------------------------------------------
// Kmd_Unregister_Event_Source
//---------------------------------------------------------------------------

BOOL Kmd_Unregister_Event_Source(
    const wchar_t *Driver_Name)
{
    HKEY system_key, driver_key;
    LONG retcode;

    system_key = Kmd_Open_Services_Key(L"EventLog\\System");
    if (! system_key)
        return FALSE;

    retcode = RegOpenKeyEx(
        system_key, Driver_Name, 0, KEY_READ | KEY_WRITE, &driver_key);

    if (retcode != 0) {

        if (retcode == ERROR_FILE_NOT_FOUND ||
            retcode == ERROR_SERVICE_DOES_NOT_EXIST ||
            retcode == ERROR_RESOURCE_TYPE_NOT_FOUND)
            return TRUE;

        Display_Error(L"RegOpenKeyEx (DriverKey)", retcode);
        return FALSE;
    }

    RegCloseKey(driver_key);

    retcode = RegDeleteKey(system_key, Driver_Name);
    if (retcode != 0) {
        Display_Error(L"RegDeleteKey (DriverKey)", retcode);
        return FALSE;
    }

    return TRUE;
}

//---------------------------------------------------------------------------
// Kmd_Install_Service
//---------------------------------------------------------------------------

BOOL Kmd_Install_Service(
    const wchar_t *Service_Name,
    const wchar_t *Service_Path,
    const wchar_t *Service_Display,
    const wchar_t *Service_Group,
    OPTIONS Options)
{
    SC_HANDLE service;

    // convert options into specific flags

    DWORD ServiceType;
    DWORD StartType;

    if (Options & OPT_KERNEL_TYPE)
        ServiceType = SERVICE_KERNEL_DRIVER;
    else
        ServiceType = SERVICE_WIN32_OWN_PROCESS;

    if (Options & OPT_SYSTEM_START)
        StartType = SERVICE_SYSTEM_START;
    else if (Options & OPT_AUTO_START)
        StartType = SERVICE_AUTO_START;
    else
        StartType = SERVICE_DEMAND_START;

    // create a service for the driver

    if (! Service_Display)
        Service_Display = Service_Name;

    service = CreateService(
        ScMgr,
        Service_Name, Service_Display,
        SERVICE_ALL_ACCESS,
        ServiceType,
        StartType,
        SERVICE_ERROR_NORMAL,
        Service_Path,
        Service_Group,
        NULL,                       // no tag identifier
        NULL,                       // no dependencies
        NULL,                       // LocalSystem account
        NULL);

    if (! service) {
        if (GetLastError() != ERROR_SERVICE_EXISTS) {
            Display_Error(L"CreateService", 0);
            return FALSE;
        }
    }

    return TRUE;
}

//---------------------------------------------------------------------------
// Kmd_Register_Event_Source
//---------------------------------------------------------------------------

BOOL Kmd_Register_Event_Source(
    const wchar_t *Driver_Name,
    const wchar_t *Driver_Path)
{
    static DWORD types_supported = 7;
    HKEY system_key, driver_key;
    LONG retcode;
    int retries = 0;

    system_key = Kmd_Open_Services_Key(L"EventLog\\System");
    if (! system_key)
        return FALSE;

    do {
        retcode = RegCreateKeyEx(
            system_key, Driver_Name, 0, NULL, 0,
            KEY_READ | KEY_WRITE, NULL, &driver_key, NULL);
        // sometimes this fails because
        // "an overlapped I/O operation is in progress"
        if (retcode == ERROR_IO_PENDING) {
            Sleep(500);
            ++retries;
            if (retries < 6)
                continue;
        }
    } while (0);

    if (retcode != 0) {
        Display_Error(L"RegCreateKeyEx (DriverKey)", retcode);
        return FALSE;
    }

    retcode = RegSetValueEx(
        driver_key, L"EventMessageFile", 0, REG_EXPAND_SZ,
        (BYTE *)Driver_Path, (wcslen(Driver_Path) + 1) * sizeof(WCHAR));
    if (retcode != 0) {
        Display_Error(L"RegSetValueEx (EventMessageFile)", retcode);
        return FALSE;
    }

    retcode = RegSetValueEx(
        driver_key, L"TypesSupported", 0, REG_DWORD,
        (BYTE *)&types_supported, sizeof(types_supported));
    if (retcode != 0) {
        Display_Error(L"RegSetValueEx (TypesSupported)", retcode);
        return FALSE;
    }

    return TRUE;
}

//---------------------------------------------------------------------------
// Kmd_Register_MiniFilter
//---------------------------------------------------------------------------

BOOL Kmd_Register_MiniFilter(
    const wchar_t *Driver_Name,
    const wchar_t *Driver_Altitude)
{
    WCHAR InstanceName[128];
    HKEY driver_key, instances_key, an_instance_key;
    LONG retcode, zero;

    driver_key = Kmd_Open_Services_Key(Driver_Name);
    if (! driver_key)
        return FALSE;

    retcode = RegSetValueEx(
        driver_key, L"DependsOnService", 0, REG_MULTI_SZ,
        (BYTE *)L"FltMgr\0\0", 8 * sizeof(WCHAR));
    if (retcode != 0) {
        Display_Error(L"RegSetValueEx (DependsOnService)", retcode);
        return FALSE;
    }

    retcode = RegCreateKeyEx(
        driver_key, L"Instances", 0, NULL, 0,
        KEY_READ | KEY_WRITE, NULL, &instances_key, NULL);
    if (retcode != 0) {
        Display_Error(L"RegCreateKeyEx (InstancesKey)", retcode);
        return FALSE;
    }

    wcscpy(InstanceName, Driver_Name);
    wcscat(InstanceName, L" Instance");

    retcode = RegSetValueEx(
        instances_key, L"DefaultInstance", 0, REG_SZ,
        (BYTE *)InstanceName, (wcslen(InstanceName) + 1) * sizeof(WCHAR));
    if (retcode != 0) {
        Display_Error(L"RegSetValueEx (DefaultInstance)", retcode);
        return FALSE;
    }

    retcode = RegCreateKeyEx(
        instances_key, InstanceName, 0, NULL, 0,
        KEY_READ | KEY_WRITE, NULL, &an_instance_key, NULL);
    if (retcode != 0) {
        Display_Error(L"RegCreateKeyEx (AnInstanceKey)", retcode);
        return FALSE;
    }

    retcode = RegSetValueEx(
        an_instance_key, L"Altitude", 0, REG_SZ,
        (BYTE *)Driver_Altitude, (wcslen(Driver_Altitude) + 1) * sizeof(WCHAR));
    if (retcode != 0) {
        Display_Error(L"RegSetValueEx (Altitude)", retcode);
        return FALSE;
    }

    zero = 0;
    retcode = RegSetValueEx(
        an_instance_key, L"Flags", 0, REG_DWORD,
        (BYTE *)&zero, sizeof(DWORD));
    if (retcode != 0) {
        Display_Error(L"RegSetValueEx (Altitude)", retcode);
        return FALSE;
    }

    return TRUE;
}

//---------------------------------------------------------------------------
// Kmd_Start_Service
//---------------------------------------------------------------------------

BOOL Kmd_Start_Service(
    const wchar_t *Driver_Name)
{
    SC_HANDLE service;
    SERVICE_STATUS service_status;

    // try to open the service for the driver

    service = OpenService(
        ScMgr, Driver_Name,
        SERVICE_INTERROGATE | SERVICE_START);
    if (! service) {
        Display_Error(L"OpenService", 0);
        return FALSE;
    }

    // stop the service

    if (! ControlService(
            service,
            SERVICE_CONTROL_INTERROGATE, &service_status)) {

        if (GetLastError() == ERROR_SERVICE_NOT_ACTIVE)
            service_status.dwCurrentState = SERVICE_STOPPED;

        else {
            Display_Error(L"ControlService Interrogate", 0);
            return FALSE;
        }
    }

    if (service_status.dwCurrentState == SERVICE_STOPPED) {

        if (! StartService(service, 0, NULL)) {
            Display_Error(L"ControlService Start", 0);
            return FALSE;
        }
    }

    return TRUE;
}

//---------------------------------------------------------------------------
// Kmd_Stop_Service
//---------------------------------------------------------------------------

BOOL Kmd_Stop_Service(
    const wchar_t *Driver_Name)
{
    extern BOOLEAN Kmd_Stop_SbieDrv(void);
    SC_HANDLE service;
    SERVICE_STATUS service_status;
    ULONG retries;

    if (_wcsicmp(Driver_Name, SBIEDRV) == 0) {
        // stop the driver
        if (! Kmd_Stop_SbieDrv())
            return FALSE;

        // fallback to stopping through SCM, otherwise the
        // driver registry key does not always disappear
    }

    // try to open the service for the driver

    service = OpenService(
        ScMgr, Driver_Name,
        SERVICE_INTERROGATE | SERVICE_STOP);

    if (! service) {

        if (GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST)
            return TRUE;

        Display_Error(L"OpenService", 0);
        return FALSE;
    }

    //
    // stop the service if it's active
    //

    for (retries = 0; ; ++retries) {

        if (retries) {

            WCHAR Text[384];

            wcscpy(Text, SbieDll_FormatMessage1(8102, Driver_Name));
            wcscat(Text, L"\n\n");
            wcscat(Text, SbieDll_FormatMessage0(8102 + retries));

            MessageBox(NULL, Text, L"KmdUtil", MB_ICONEXCLAMATION | MB_OK);
        }

        if (! ControlService(
                service,
                SERVICE_CONTROL_INTERROGATE, &service_status)) {

            if (GetLastError() == ERROR_SERVICE_NOT_ACTIVE)
                return TRUE;

            if ((GetLastError() == ERROR_SERVICE_REQUEST_TIMEOUT ||
                 GetLastError() == ERROR_PIPE_BUSY) && retries < 3)
                    continue;

            Display_Error(L"ControlService Interrogate", 0);
            return FALSE;
        }

        if (service_status.dwCurrentState != SERVICE_STOPPED) {

            if (! ControlService(
                    service,
                    SERVICE_CONTROL_STOP, &service_status)) {

                if ((GetLastError() == ERROR_SERVICE_REQUEST_TIMEOUT ||
                     GetLastError() == ERROR_PIPE_BUSY) && retries < 3)
                        continue;

                Display_Error(L"ControlService Stop", 0);
                return FALSE;
            }
        }

        return TRUE;
    }

    return FALSE;
}

//---------------------------------------------------------------------------
// WinMain
//---------------------------------------------------------------------------

int __stdcall WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    COMMAND Command;
    PWSTR Driver_Name = NULL;
    PWSTR Driver_Path = NULL;
    PWSTR Driver_Display = NULL;
    PWSTR Driver_MsgFile = NULL;
    PWSTR Driver_Altitude = NULL;
    PWSTR Driver_Group = NULL;
    OPTIONS Options;
    BOOL ok;

    if (! Parse_Command_Line(
            &Command, &Driver_Name, &Driver_Path,
            &Driver_Display, &Driver_MsgFile,
            &Driver_Altitude, &Driver_Group,
            &Options))
        return EXIT_FAILURE;

    ScMgr = OpenSCManager(
        NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CREATE_SERVICE);

    if (! ScMgr) {
        Display_Error(L"OpenSCManager", 0);
        return EXIT_FAILURE;
    }

    if ((Command == CMD_SCANDLL) || (Command == CMD_SCANDLL_SILENT)) {
        Kmd_ScanDll(Command == CMD_SCANDLL_SILENT);
        ok = TRUE;
    }

    if (Command == CMD_DELETE) {
        ok = Kmd_Delete_Service(Driver_Name);
        if (ok)
            ok = Kmd_Unregister_Event_Source(Driver_Name);
        ok = TRUE;  // don't let the calling installer fail
    }

    if (Command == CMD_INSTALL) {
        ok = Kmd_Install_Service(
            Driver_Name, Driver_Path, Driver_Display, Driver_Group, Options);
        if (ok) {
            if (! Driver_MsgFile)
                Driver_MsgFile = Driver_Path;
            ok = Kmd_Register_Event_Source(Driver_Name, Driver_MsgFile);
            if (ok && Driver_Altitude)
                ok = Kmd_Register_MiniFilter(Driver_Name, Driver_Altitude);
            if (! ok) {
                Kmd_Unregister_Event_Source(Driver_Name);
                Kmd_Delete_Service(Driver_Name);
            }
        }
    }

    if (Command == CMD_START)
        ok = Kmd_Start_Service(Driver_Name);

    if (Command == CMD_STOP)
        ok = Kmd_Stop_Service(Driver_Name);

    if (! ok)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
