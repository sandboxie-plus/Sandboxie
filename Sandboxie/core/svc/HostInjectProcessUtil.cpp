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
// Driver Assistant, host injected process utilities
//---------------------------------------------------------------------------

// warning C4652: compiler option 'C++ Exception Handling Unwinding' inconsistent with precompiled header; current command-line option will override that defined in the precompiled header
#pragma warning(disable : 4652)

#include "stdafx.h"

#include <psapi.h>
#include <stdio.h>
#include <string>
#include <set>

typedef long NTSTATUS;
#undef WIN32_NO_STATUS
#include <ntstatus.h>

#include "common/my_version.h"
#include "core/dll/sbieapi.h"


#define CONF_LINE_LEN               2000    // keep in sync with drv/conf.c

using namespace std;

set<wstring>        g_setSvcNames;

// build a map of all services that are HosInjectProcess'es

void BuildSvcSet()
{
    WCHAR   wszConfigLine[CONF_LINE_LEN];
    int i = 0;

    g_setSvcNames.clear();

    // Loop through every section, searching for "HostInjectProcess", and add the service names to the set.
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
                    g_setSvcNames.insert(wstring(wszSvcName));
                }
            }
        }
    }
}


BOOL IsSvcInjected(DWORD dwPid)
{
    HANDLE hProcess;
    HMODULE hMods[1024];
    BOOL res = FALSE;

    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPid);
    if (hProcess != NULL)
    {
        DWORD dwSize;
        // search through all loaded modules and see if we are injected 
        if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &dwSize))
        {
            DWORD n;
            for (n = 0; n < (dwSize / sizeof(HMODULE)); n++)
            {
                WCHAR   wszModName[MAX_PATH];
                if (GetModuleBaseNameW(hProcess, hMods[n], wszModName, sizeof(wszModName) / sizeof(WCHAR)))
                {
                    if (_wcsicmp(wszModName, SBIEDLL L".dll") == 0)
                    {
                        res = TRUE;
                        break;
                    }
                }
            }
        } // if (EnumProcessModules())

        CloseHandle(hProcess);
    } // if (OpenProcess())

    return res;
}


void RestartService(SC_HANDLE hScm, WCHAR *wszServiceName)
{
    SC_HANDLE hService;
    BOOL res;

    hService = OpenService(hScm, wszServiceName, SERVICE_ALL_ACCESS);
    res = GetLastError();
    if (hService != NULL)
    {
        SERVICE_STATUS stServiceStatus;
        res = ControlService(hService, SERVICE_CONTROL_STOP, &stServiceStatus);
        res = StartServiceW(hService, 0, NULL);
        CloseServiceHandle(hService);
    }
}


void RestartHostInjectedSvcs()
{
    BuildSvcSet();

    SC_HANDLE hScm = OpenSCManagerW(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);
    if (!hScm)
        return;

    const DWORD dwProcBufSize = 32 * 1024;
    LPBYTE pProcBuf = new BYTE[dwProcBufSize];

    DWORD   dwBytesNeeded = 0;
    DWORD   dwServicesReturned = 0;
    DWORD   dwResumeHandle = 0;

    // get a list of all active services
    BOOL b = EnumServicesStatusExW(hScm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_ACTIVE, pProcBuf, dwProcBufSize,
        &dwBytesNeeded, &dwServicesReturned, &dwResumeHandle, NULL);
    DWORD res = GetLastError();

    DWORD i = 0;
    for (i = 0; i < dwServicesReturned; i++)
    {
        ENUM_SERVICE_STATUS_PROCESS     *pService = &((ENUM_SERVICE_STATUS_PROCESS*)pProcBuf)[i];

        if ((_wcsicmp(pService->lpServiceName, SBIESVC) == 0) )         // ignore the Sbie service
            continue;

        set<wstring>::iterator itr = g_setSvcNames.find(pService->lpServiceName);   // is service name in the map?

        if (itr != g_setSvcNames.end())
        {
            // if we are not injected and we should be, restart svc
            if (!IsSvcInjected(pService->ServiceStatusProcess.dwProcessId))
                RestartService(hScm, pService->lpServiceName);
        }
        else
        {
            // if we are injected and we should not be, restart svc
            if (IsSvcInjected(pService->ServiceStatusProcess.dwProcessId))
                RestartService(hScm, pService->lpServiceName);
        }
    }

    CloseServiceHandle(hScm);
    delete pProcBuf;
}

