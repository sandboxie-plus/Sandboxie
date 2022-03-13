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
// LingerProcess / LeaderProcess management
//---------------------------------------------------------------------------

#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

#include <windows.h>
#include <stdlib.h>
#include "core/dll/sbiedll.h"
#include "common/win32_ntddk.h"
#include "common/my_version.h"
#include "common/defines.h"
#define WITHOUT_POOL
#include "common/map.c"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define MAX_LINGER_LEADER_COUNT     512


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _LINGER_LEADER {
    HANDLE orig_pid;
    HANDLE curr_pid;
    WCHAR image[1];
} LINGER_LEADER;

typedef struct _MONITOR_PID {
    HANDLE hProcHandle;
    HANDLE hProcWait;
} MONITOR_PID;


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


extern const WCHAR *_SandboxieRpcSs;
extern const WCHAR *_msiexec;

static const WCHAR *_SandboxieDcomLaunch = SANDBOXIE L"DcomLaunch.exe";
static const WCHAR *_SandboxieCrypto     = SANDBOXIE L"Crypto.exe";
static const WCHAR *_SandboxieBITS       = SANDBOXIE L"BITS.exe";
static const WCHAR *_SandboxieWUAU       = SANDBOXIE L"WUAU.exe";
static const WCHAR *_wuauclt             = L"wuauclt.exe";
static const WCHAR *_TrustedInstaller    = L"TrustedInstaller.exe";
static const WCHAR *_tiworker            = L"tiworker.exe";
static CRITICAL_SECTION ProcessCritSec;
static HANDLE heventProcessStart;
static HANDLE heventRpcSs;
static HASH_MAP pidMap;

void RemovePid(ULONG myPid);


//---------------------------------------------------------------------------
// WaitOrTimerCallback 
//---------------------------------------------------------------------------

VOID CALLBACK WaitOrTimerCallback( _In_  PVOID lpParameter, _In_  BOOLEAN TimerOrWaitFired )
{
    EnterCriticalSection(&ProcessCritSec);
    RemovePid((ULONG)(ULONG_PTR)lpParameter);
    SetEvent(heventRpcSs);
    LeaveCriticalSection(&ProcessCritSec);
}

//---------------------------------------------------------------------------
// RemovePid 
//---------------------------------------------------------------------------

void RemovePid(ULONG myPid)
{
    MONITOR_PID monitorPid;
    if (map_take(&pidMap, (void*)myPid, &monitorPid, sizeof(monitorPid))) {
        UnregisterWait(monitorPid.hProcWait);
        CloseHandle(monitorPid.hProcHandle);
    }
}

//---------------------------------------------------------------------------
// AddPid 
//---------------------------------------------------------------------------

void AddPid(ULONG *myPids, ULONG count)
{
    for (ULONG i = 0; i < count; i++)
    {
        if (map_get(&pidMap, (void*)myPids[i]) == NULL) { // not yet listed
            MONITOR_PID monitorPid;
            monitorPid.hProcHandle = OpenProcess(SYNCHRONIZE, FALSE, myPids[i]);
            if (monitorPid.hProcHandle) {
                RegisterWaitForSingleObject(&monitorPid.hProcWait, monitorPid.hProcHandle, WaitOrTimerCallback, (void *)myPids[i], INFINITE, WT_EXECUTEONLYONCE);
                map_insert(&pidMap, (void*)myPids[i], &monitorPid, sizeof(monitorPid));
            }
        }
    }
}


//---------------------------------------------------------------------------
// ProcessStartMonitor
//---------------------------------------------------------------------------


DWORD ProcessStartMonitor(void *arg) {
    DWORD rc;

    if (!heventProcessStart) {
        heventProcessStart = CreateEvent(0, FALSE, FALSE, SESSION_PROCESS);
        if (!heventProcessStart) {
            //Can't start monitor thread!
            return 0;
        }
    }
    while (1) {
        rc = WaitForSingleObject(heventProcessStart, INFINITE);
        EnterCriticalSection(&ProcessCritSec);

        ULONG pid_count = 0;
        SbieApi_EnumProcessEx(NULL, FALSE, -1, NULL, &pid_count); // query count
        pid_count += 128;

        ULONG* pids = HeapAlloc(
            GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, sizeof(ULONG) * pid_count);
        SbieApi_EnumProcessEx(NULL, FALSE, -1, pids, &pid_count); // query pids

        AddPid(pids, pid_count);

        HeapFree(GetProcessHeap(), 0, pids);

        LeaveCriticalSection(&ProcessCritSec);
    }
    return 0;
}


//---------------------------------------------------------------------------
// Add_LL_Entry
//---------------------------------------------------------------------------


_FX void Add_LL_Entry(
    LINGER_LEADER **lingers, ULONG *inout_count, const WCHAR *image)
{
    ULONG linger_count = *inout_count;

    if (linger_count >= MAX_LINGER_LEADER_COUNT)
        return;

    lingers[linger_count] = HeapAlloc(
        GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS,
        sizeof(LINGER_LEADER) + (wcslen(image) + 4) * sizeof(WCHAR));

    lingers[linger_count]->orig_pid = NULL;
    lingers[linger_count]->curr_pid = NULL;
    wcscpy(lingers[linger_count]->image, image);

    ++linger_count;

    *inout_count = linger_count;
}


//---------------------------------------------------------------------------
// LingerEnumWindowsProc
//---------------------------------------------------------------------------


_FX BOOL LingerEnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    if (GetWindowLong(hwnd, GWL_STYLE) & WS_VISIBLE) {
        ULONG *pids = (ULONG *)lParam;
        ULONG i;
        ULONG idProcess = 0;
        GetWindowThreadProcessId(hwnd, &idProcess);
        if (idProcess) {
            for (i = 0; pids[i] != -1; i++) {
                if (idProcess == pids[i])
                    return FALSE;
            }
        }
    }
    return TRUE;
}


//---------------------------------------------------------------------------
// DoLingerLeader
//---------------------------------------------------------------------------


int DoLingerLeader(void)
{
    ULONG i, j;
    HANDLE pids_i;
    WCHAR image[128];

    PROCESS_DATA *myData;
    LINGER_LEADER **lingers;
    LINGER_LEADER **leaders;
    ULONG linger_count, leader_count, process_count;
    LONG rc;
    BOOLEAN any_leaders;

    //
    // handle linger/leader process terminations
    //

    linger_count = 0;
    leader_count = 0;
    lingers = NULL;
    leaders = NULL;
    any_leaders = FALSE;

    map_init(&pidMap, NULL);
    
    InitializeCriticalSection(&ProcessCritSec);

    heventRpcSs = CreateEvent(0, FALSE, FALSE, NULL);
	HANDLE ThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ProcessStartMonitor, NULL, 0, NULL);
	if (ThreadHandle)
		CloseHandle(ThreadHandle);

    if (1) {
        //
        // read and store LingerProcess configuration
        //

        lingers = (LINGER_LEADER**)HeapAlloc(
            GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS,
            sizeof(LINGER_LEADER *) * MAX_LINGER_LEADER_COUNT);

        while (1)
        {
            rc = SbieApi_QueryConfAsIs(
                NULL, L"LingerProcess", linger_count,
                image, sizeof(WCHAR) * 120);
            if (rc != 0)
                break;

            Add_LL_Entry(lingers, &linger_count, image);
        }

        if (SbieApi_QueryConfBool(NULL, L"LingerLeniency", TRUE)) {

            //
            // see which of the LingerProcess programs were already active
            // before SandboxieRpcSs started.  they will not be considered
            // LingerProcess programs and will not be terminated
            //

            ULONG pid_count = 0;
            SbieApi_EnumProcessEx(NULL, FALSE, -1, NULL, &pid_count); // query count
            pid_count += 128;

            ULONG* pids = HeapAlloc(
                GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, sizeof(ULONG) * pid_count);
            SbieApi_EnumProcessEx(NULL, FALSE, -1, pids, &pid_count); // query pids

            AddPid(pids, pid_count);

            for (i = 0; i < pid_count; ++i) {

                pids_i = (HANDLE)(ULONG_PTR)pids[i];
                SbieApi_QueryProcess(pids_i, NULL, image, NULL, NULL);

                for (j = 0; j < linger_count; ++j) {
                    if (_wcsicmp(lingers[j]->image, image) == 0) {
                        lingers[j]->orig_pid = pids_i;
                        break;
                    }
                }
            }

            HeapFree(GetProcessHeap(), 0, pids);
        }

        //
        // add standard lingers.  note that we don't check if any of
        // the following processes was already active before we started
        //

        Add_LL_Entry(lingers, &linger_count, _SandboxieDcomLaunch);
        Add_LL_Entry(lingers, &linger_count, _SandboxieCrypto);
        Add_LL_Entry(lingers, &linger_count, _SandboxieBITS);
        Add_LL_Entry(lingers, &linger_count, _SandboxieWUAU);
        Add_LL_Entry(lingers, &linger_count, _wuauclt);
        Add_LL_Entry(lingers, &linger_count, _TrustedInstaller);
        Add_LL_Entry(lingers, &linger_count, _tiworker);

        //
        // read and store LeaderProcess configuration
        //

        leaders = HeapAlloc(
            GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS,
            sizeof(LINGER_LEADER *) * MAX_LINGER_LEADER_COUNT);

        while (1) {

            rc = SbieApi_QueryConfAsIs(
                NULL, L"LeaderProcess", leader_count,
                image, sizeof(WCHAR) * 120);
            if (rc != 0)
                break;

            Add_LL_Entry(leaders, &leader_count, image);
        }
    }

    //
    // do StartService and StartProgram settings
    //

    if (1)
    {

        //        extern DWORD ServiceStatus_CurrentState;

        myData = my_findProcessData(L"RPCSS", 0);
        if (!myData) {
            return 0;
        }
        WaitForSingleObject(myData->hStartLingerEvent, INFINITE);

        for (i = 0; ; ++i) {

            rc = SbieApi_QueryConfAsIs(
                NULL, L"StartService", i, image, sizeof(WCHAR) * 120);
            if (rc != 0)
                break;

            SbieDll_StartBoxedService(image, TRUE);
        }

        ULONG buf_len = 4096 * sizeof(WCHAR);
        WCHAR* buf1 = HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY, buf_len);

        for (i = 0; ; ++i) {

            rc = SbieApi_QueryConfAsIs(
                NULL, L"StartProgram", i, buf1, buf_len - 16);
            if (rc != 0)
                break;

            SbieDll_ExpandAndRunProgram(buf1);
        }

        HeapFree(GetProcessHeap(), 0, buf1);
    }

    //
    // now just wait until such time when our COM framework is
    // the only things left running in the sandbox, and then die
    //

    while (1)
    {
        BOOLEAN any_leaders_local = FALSE;
        BOOLEAN terminate_and_stop = TRUE;

        //
        // wait for the process exit event (my_event)
        //

        WaitForSingleObject(heventRpcSs, INFINITE);
        EnterCriticalSection(&ProcessCritSec);

        SbieApi_EnumProcessEx(NULL, FALSE, -1, NULL, &process_count); // query count
        process_count += 128;

        ULONG* pids = HeapAlloc(
            GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, sizeof(ULONG) * (process_count + 1)); // allocate oen more for the -1 marker
        SbieApi_EnumProcessEx(NULL, FALSE, -1, pids, &process_count); // query pids
        pids[process_count] = -1; // set the end marker

        //
        // query running processes
        //

        for (i = 0; i < process_count; ++i) {

            //
            // if the process in question was started by Start.exe,
            // then we do not treat the process as lingering.  except:
            //
            // - if the process is running as LocalSystem, it was probably
            // spawned using Start.exe by SbieSvc::ServiceServer.
            //
            // - if the process is SandboxieCrypto, which can get invoked
            // by Start.exe in some cases
            //

            BOOLEAN excluded_from_linger = FALSE;
            //BOOLEAN is_local_system_sid = FALSE;

            pids_i = (HANDLE)(ULONG_PTR)pids[i];
            image[0] = L'\0';

            if (0 == SbieApi_QueryProcess(pids_i, NULL, image, NULL, NULL)
                && _wcsicmp(image, _SandboxieCrypto) != 0) {

                //
                // check if this is a local system process 
                //

                // Note: since we normally no longer start services as system this is pointless

                //HANDLE ProcessHandle = SbieDll_OpenProcess(PROCESS_QUERY_INFORMATION, pids_i);
                //if (ProcessHandle) {
                //    if (SbieDll_CheckProcessLocalSystem(ProcessHandle))
                //        is_local_system_sid = TRUE;
                //    CloseHandle(ProcessHandle);
                //}
                //
                //if (!is_local_system_sid) 
                if (SbieApi_QueryConfBool(NULL, L"LingerLeniency", TRUE))
                {
                    //
                    // then check if the process was started explicitly
                    // (via forced mechanism or as a child of start.exe)
                    // and then don't terminate it as a linger
                    //
                    // (note that sevice processes running as local system
                    // are also children of start.exe, but in that case,
                    // is_local_system_sid would be TRUE and we would not
                    // reach this point.)
                    //
                    // fix-me: services are no longer started by default as system
                    //

                    ULONG64 ProcessFlags =
                                SbieApi_QueryProcessInfo(pids_i, 0);

                    if (ProcessFlags & (SBIE_FLAG_FORCED_PROCESS        |
                                        SBIE_FLAG_PARENT_WAS_START_EXE  |
                                        SBIE_FLAG_PROCESS_IS_START_EXE  )) {

                        excluded_from_linger = TRUE;
                    }
                }
            }

            //
            // ignore our own process
            //

            if (_wcsicmp(image, _SandboxieRpcSs) == 0)
                continue;

            //
            // find a leader process
            //

            for (j = 0; j < leader_count; ++j) {
                if (_wcsicmp(leaders[j]->image, image) == 0) {
                    any_leaders_local = TRUE;
                    break;
                }
            }

            //
            // for each process we find, check if the image appears on
            // the linger list.  if so, we update the last known pid
            // for that linger.  but if that process was already active
            // before RpcSs started, then don't kill any lingers
            //
            // alternatively, if the process is not on the linger list,
            // then we won't be killing any of the LingerProcess yet
            //

            for (j = 0; j < linger_count; ++j) {
                if (_wcsicmp(lingers[j]->image, image) == 0) {
                    lingers[j]->curr_pid = pids_i;
                    if (excluded_from_linger)
                        lingers[j]->orig_pid = pids_i;
                    if (pids_i == lingers[j]->orig_pid)
                        terminate_and_stop = FALSE;
                    break;
                }
            }

            if (j == linger_count) {

                //
                // we found an active process that is not a linger process,
                // so reset the flag to kill lingers
                //

                terminate_and_stop = FALSE;
            }
        }

        //
        // if leader processes ended (i.e. after being active for some time)
        // then kill all processes
        //

        if (any_leaders && (! any_leaders_local)) {

            terminate_and_stop = TRUE;
            goto do_kill_all;

        } else
            any_leaders = any_leaders_local;

        //
        // don't terminate if a lingering process has an open window
        //

        if (terminate_and_stop) {

            //
            // if a process in the PID list has a window LingerEnumWindowsProc will return FALSE
            // what causes the enumeration to abort and EnumWindows to return FALSE as well
            //

            BOOL ret = EnumWindows(LingerEnumWindowsProc, (LPARAM)pids);
            if (ret == FALSE)
                terminate_and_stop = FALSE;
        }

        //
        // don't terminate if a lingering process has just started recently
        //

        if (terminate_and_stop) {
            for (i = 0; i < process_count; ++i) {
                HANDLE hProcess = NULL;
                ULONG64 ProcessFlags = SbieApi_QueryProcessInfo(
                                (HANDLE) (ULONG_PTR) pids[i], 0);
                if (! (ProcessFlags & SBIE_FLAG_IMAGE_FROM_SBIE_DIR)) {
                    hProcess = OpenProcess(
                                PROCESS_QUERY_INFORMATION, FALSE, pids[i]);
                }
                if (hProcess) {
                    FILETIME time, time1, time2, time3;
                    BOOL ok = GetProcessTimes(
                            hProcess, &time, &time1, &time2, &time3);
                    if (ok) {
                        LARGE_INTEGER liProcess, liCurrent;
                        __int64 diff;
                        liProcess.LowPart = time.dwLowDateTime;
                        liProcess.HighPart = time.dwHighDateTime;
                        GetSystemTimeAsFileTime(&time);
                        liCurrent.LowPart = time.dwLowDateTime;
                        liCurrent.HighPart = time.dwHighDateTime;
                        diff = liCurrent.QuadPart - liProcess.QuadPart;
                        if (diff < SECONDS(5))
                            terminate_and_stop = FALSE;
                    }
                    CloseHandle(hProcess);
                }
                if (! terminate_and_stop)
                    break;
            }
        }

        //
        // kill all programs and quit
        //

        LeaveCriticalSection(&ProcessCritSec);
do_kill_all:

        HeapFree(GetProcessHeap(), 0, pids);

        if (terminate_and_stop) {
            SbieDll_KillAll(-1, NULL);
            break;
        }
    }

	// cleanup CS
	DeleteCriticalSection(&ProcessCritSec);

    // this process is no longer needed

    ExitProcess(0);
    return 0;
}
