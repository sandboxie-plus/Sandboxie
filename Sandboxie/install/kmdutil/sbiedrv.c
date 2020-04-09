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


#include "stdafx.h"

#include "common/win32_ntddk.h"
#include "common/defines.h"
#include "core/drv/api_defs.h"
#include "core/dll/sbieapi.h"
#include "common/my_version.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static void EnablePriv(void);


//---------------------------------------------------------------------------
// EnablePriv
//---------------------------------------------------------------------------


ALIGNED void EnablePriv(void)
{
    //
    // enable SeRestorePrivilege so SetShortName() can do its job
    //

    WCHAR priv_space[64];
    TOKEN_PRIVILEGES *privs = (TOKEN_PRIVILEGES *)priv_space;
    HANDLE hToken;

    BOOL b = LookupPrivilegeValue(
                L"", SE_LOAD_DRIVER_NAME, &privs->Privileges[0].Luid);
    if (b) {

        privs->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        privs->PrivilegeCount = 1;

        b = OpenProcessToken(
                GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken);
        if (b) {

            b = AdjustTokenPrivileges(hToken, FALSE, privs, 0, NULL, NULL);
            CloseHandle(hToken);
        }
    }
}


//---------------------------------------------------------------------------
// Kmd_Stop_SbieDrv
//---------------------------------------------------------------------------


ALIGNED BOOLEAN Kmd_Stop_SbieDrv(void)
{
    //extern void Display_Error(PWSTR SubFuncName, DWORD LastError);

    WCHAR driver_version[16];
    UNICODE_STRING uni;
    LONG rc;

    rc = SbieApi_GetVersion(driver_version);
    if (rc == 0) {

        rc = SbieApi_CallZero(API_UNLOAD_DRIVER);
        if (rc == STATUS_CONNECTION_IN_USE) {
            Sleep(2500);
            rc = SbieApi_CallZero(API_UNLOAD_DRIVER);
        }
        if (rc == STATUS_CONNECTION_IN_USE) {
            Sleep(2500);
            rc = SbieApi_CallZero(API_UNLOAD_DRIVER);
        }
    }

    EnablePriv();

    RtlInitUnicodeString(&uni,
        L"\\Registry\\Machine\\System\\CurrentControlSet"
        L"\\Services\\" SBIEDRV);
    rc = NtUnloadDriver(&uni);
    if (rc == 0 || rc == STATUS_OBJECT_NAME_NOT_FOUND)
        return TRUE;

    return FALSE;
}


//---------------------------------------------------------------------------
// Kmd_Stop_Host_Injected_Svcs
//---------------------------------------------------------------------------

extern SC_HANDLE ScMgr;
#define CONF_LINE_LEN               2000    // keep in sync with drv/conf.c

void Kmd_Stop_Host_Injected_Svcs()
{
    WCHAR	wszConfigLine[CONF_LINE_LEN];
    int i = 0;

    // Loop through every section, searching for "HostInjectProcess", and stop them if they are services.
    while (SbieApi_QueryConf(NULL, NULL, i++, wszConfigLine, sizeof(wszConfigLine)) == STATUS_SUCCESS)
    {
        if (SbieApi_IsBoxEnabled(wszConfigLine) == STATUS_SUCCESS)
        {
            if (SbieApi_QueryConf(wszConfigLine, L"HostInjectProcess", 0, wszConfigLine, sizeof(wszConfigLine)) == STATUS_SUCCESS)
            {
                WCHAR   wszProcName[261];
                WCHAR   wszSvcName[261];
                if (swscanf(wszConfigLine, L"%260[^'|']|%260s", wszProcName, wszSvcName) == 2)
                {
                    SC_HANDLE hService;
                    BOOL res;
                    hService = OpenService(ScMgr, wszSvcName, SERVICE_INTERROGATE | SERVICE_STOP);
                    if (hService)
                    {
                        SERVICE_STATUS stServiceStatus;
                        res = ControlService(hService, SERVICE_CONTROL_STOP, &stServiceStatus);
                        CloseServiceHandle(hService);
                    }
                }
            }
        }
    }
}
