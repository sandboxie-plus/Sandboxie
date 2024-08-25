/*
 * Copyright 2023 David Xanatos, xanasoft.com
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
// Box Manager
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "BoxManager.h"
#include "ProcessServer.h"
#include "MountManager.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------

/*typedef struct _LINGER_LEADER {
    HANDLE orig_pid;
    HANDLE curr_pid;
    WCHAR image[1];
} LINGER_LEADER;*/

struct BOX_INSTANCE
{
    std::wstring BoxName;
    //bool HiveMounted = false;

    std::map<ULONG, BOXED_PROCESS*> ProcessMap;

    /*ULONG linger_count;
    ULONG leader_count;
    LINGER_LEADER **lingers;
    LINGER_LEADER **leaders;
    BOOLEAN any_leaders;*/
};

struct BOXED_PROCESS
{
    ULONG ProcessId;
    ULONG SessionId;
    BOX_INSTANCE* pBox;
    
    HANDLE hProcHandle;
    HANDLE hProcWait;
};


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------



BoxManager::BoxManager()
{
    InitializeCriticalSection(&m_CritSec);

    LoadProcesses();
}


//---------------------------------------------------------------------------
// BoxManager
//---------------------------------------------------------------------------


BoxManager *BoxManager::GetInstance()
{
    static BoxManager *_instance = NULL;
    if (! _instance)
        _instance = new BoxManager();
    return _instance;
}


void BoxManager::LoadProcesses()
{
    ULONG pid_count = 0;
    SbieApi_EnumProcessEx(NULL, TRUE, -1, NULL, &pid_count); // query count
        pid_count += 128;

    std::vector<ULONG> pids;
    pids.resize(pid_count + 1); // allocate oen more for the -1 marker
    SbieApi_EnumProcessEx(NULL, TRUE, -1, &pids.front(), &pid_count); // query pids

    ULONG reg_root_len = 256;
    WCHAR* reg_root_path = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, reg_root_len + 16);;

    for (ULONG i = 0; i <= pid_count; ++i) {

        WCHAR boxname[34];
        ULONG session_id;
        SbieApi_QueryProcess((HANDLE)(ULONG_PTR)pids[i], boxname, NULL, NULL, &session_id);

        ULONG reg_len = reg_root_len;
        if (!NT_SUCCESS(SbieApi_QueryProcessPath((HANDLE)(ULONG_PTR)pids[i], NULL, NULL, NULL, NULL, &reg_len, NULL)))
            continue;
        if (reg_len > reg_root_len) {
            reg_root_len = reg_len;
            reg_root_path = (WCHAR*)HeapReAlloc(GetProcessHeap(), 0, reg_root_path, reg_root_len + 16);
            if (!reg_root_path)
                continue;
        }
        
        if (!NT_SUCCESS(SbieApi_QueryProcessPath((HANDLE)(ULONG_PTR)pids[i], NULL, reg_root_path, NULL, NULL, &reg_len, NULL)))
            continue;

        ProcessCreated(pids[i], boxname, reg_root_path, session_id);
    }

    if (reg_root_path) 
        HeapFree(GetProcessHeap(), 0, reg_root_path);
}

//---------------------------------------------------------------------------
// WaitOrTimerCallback 
//---------------------------------------------------------------------------

VOID CALLBACK WaitOrTimerCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
    BOXED_PROCESS* pProcess = (BOXED_PROCESS*)lpParameter;

    UnregisterWait(pProcess->hProcWait);
    CloseHandle(pProcess->hProcHandle);

    EnterCriticalSection(&BoxManager::GetInstance()->m_CritSec);

    if (pProcess->pBox) {
        pProcess->pBox->ProcessMap.erase(pProcess->ProcessId);
#if 0
        BoxManager::GetInstance()->CheckLinger(pProcess->pBox, pProcess->SessionId);
#endif
    }

    LeaveCriticalSection(&BoxManager::GetInstance()->m_CritSec);

    delete pProcess;
}

extern "C" {
    WINBASEAPI DWORD WINAPI GetFinalPathNameByHandleW(
        _In_ HANDLE hFile,
        _Out_writes_(cchFilePath) LPWSTR lpszFilePath,
        _In_ DWORD cchFilePath,
        _In_ DWORD dwFlags
    );
}

//std::wstring BoxManager__GetFinalPath(const WCHAR* file_root_path)
//{
//    std::wstring FileRootPath;
//    FileRootPath.resize(8192);
//
//    //
//    // get the final file path by opening it and retrieving it from the handle
//    //
//
//    UNICODE_STRING objname;
//    RtlInitUnicodeString(&objname, file_root_path);
//
//    OBJECT_ATTRIBUTES objattrs;
//    InitializeObjectAttributes(
//        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);
//
//    HANDLE handle = INVALID_HANDLE_VALUE;
//    IO_STATUS_BLOCK IoStatusBlock;
//    NTSTATUS status = NtCreateFile(
//        &handle, GENERIC_READ, &objattrs, &IoStatusBlock, NULL, 0,
//        FILE_SHARE_VALID_FLAGS, FILE_OPEN, 0, NULL, 0);
//
//    DWORD dwRet = GetFinalPathNameByHandleW(handle, (WCHAR*)FileRootPath.c_str(), FileRootPath.size(), VOLUME_NAME_NT);
//    //if (dwRet == 0 || dwRet > FileRootPath.size()) // failed || buffer to small
//    //    goto finish;
//        
//    if (handle != INVALID_HANDLE_VALUE) 
//        NtClose(handle);
//
//    return FileRootPath;
//}

bool BoxManager::ProcessCreated(ULONG ProcessId, const WCHAR* boxname, const WCHAR* reg_root, ULONG session_id)
{
    bool IsFirst = false;

    EnterCriticalSection(&m_CritSec);

    BOX_INSTANCE* &pBox = m_BoxMap[reg_root];

    if (!pBox) {
        pBox = new BOX_INSTANCE;
        pBox->BoxName = std::wstring(boxname);

        SbieApi_LogEx(session_id, 2201, L"BoxCreated %S", pBox->BoxName.c_str());

#if 0
        InitLinger(pBox);
#endif

        IsFirst = true;
    }

    BOXED_PROCESS* &pProcess = pBox->ProcessMap[ProcessId];
    if (!pProcess) {
        pProcess = new BOXED_PROCESS;

        pProcess->ProcessId = ProcessId;
        pProcess->SessionId = session_id;
        pProcess->pBox = pBox;

        pProcess->hProcHandle = OpenProcess(SYNCHRONIZE, FALSE, ProcessId);
        if (pProcess->hProcHandle) {
            RegisterWaitForSingleObject(&pProcess->hProcWait, pProcess->hProcHandle, WaitOrTimerCallback, (void*)pProcess, INFINITE, WT_EXECUTEONLYONCE);
        }
    }

    LeaveCriticalSection(&m_CritSec);

    return IsFirst;
}

void BoxManager::BoxOpened(const WCHAR* reg_root, ULONG session_id)
{
    EnterCriticalSection(&m_CritSec);

    auto I = m_BoxMap.find(reg_root);
    if (I != m_BoxMap.end()){
        SbieApi_LogEx(session_id, 2201, L"BoxOpened %S", I->second->BoxName.c_str());
        //I->second->HiveMounted = TRUE;
    }

    LeaveCriticalSection(&m_CritSec);
}

void BoxManager::BoxClosed(const WCHAR* reg_root, ULONG session_id)
{
    EnterCriticalSection(&m_CritSec);

    auto I = m_BoxMap.find(reg_root);
    if (I != m_BoxMap.end()){
        SbieApi_LogEx(session_id, 2201, L"BoxClosed %S", I->second->BoxName.c_str());

        // the process map should be empty
        for (auto J = I->second->ProcessMap.begin(); J != I->second->ProcessMap.end(); ++J)
        {
            // keep the process around untill the event fires, but set the box pointer to NULL 
            J->second->pBox = NULL;
            //delete pProcess;
        }

#if 0
        for (ULONG i = 0; i < pBox->linger_count; ++i) {
            HeapFree(GetProcessHeap(), 0, pBox->lingers[i]);
        }
        HeapFree(GetProcessHeap(), 0, pBox->lingers);

        for (ULONG i = 0; i < pBox->leader_count; ++i) {
            HeapFree(GetProcessHeap(), 0, pBox->leaders[i]);
        }
        HeapFree(GetProcessHeap(), 0, pBox->leaders);
#endif

        delete I->second;
        m_BoxMap.erase(I);
    }

    LeaveCriticalSection(&m_CritSec);
}



#if 0
#include "../../common/my_version.h"


#define MAX_LINGER_LEADER_COUNT     512

static const WCHAR *_SandboxieRpcSs      = SANDBOXIE L"RpcSs.exe";
static const WCHAR *_msiexec             = L"msiexec.exe";

static const WCHAR *_SandboxieDcomLaunch = SANDBOXIE L"DcomLaunch.exe";
static const WCHAR *_SandboxieCrypto     = SANDBOXIE L"Crypto.exe";
static const WCHAR *_SandboxieBITS       = SANDBOXIE L"BITS.exe";
static const WCHAR *_SandboxieWUAU       = SANDBOXIE L"WUAU.exe";
static const WCHAR *_wuauclt             = L"wuauclt.exe";
static const WCHAR *_TrustedInstaller    = L"TrustedInstaller.exe";
static const WCHAR *_tiworker            = L"tiworker.exe";



_FX void Add_LL_Entry(
    LINGER_LEADER **lingers, ULONG *inout_count, const WCHAR *image)
{
    ULONG linger_count = *inout_count;

    if (linger_count >= MAX_LINGER_LEADER_COUNT)
        return;

    lingers[linger_count] = (LINGER_LEADER*)HeapAlloc(
        GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS,
        sizeof(LINGER_LEADER) + (wcslen(image) + 4) * sizeof(WCHAR));

    lingers[linger_count]->orig_pid = NULL;
    lingers[linger_count]->curr_pid = NULL;
    wcscpy(lingers[linger_count]->image, image);

    ++linger_count;

    *inout_count = linger_count;
}

void BoxManager::InitLinger(BOX_INSTANCE* pBox)
{
//    ULONG i, j;
//    HANDLE pids_i;
    WCHAR image[128];
//    PROCESS_DATA *myData;
    LONG rc;


    if (1) {

        //
        // read and store LingerProcess configuration
        //

        pBox->lingers = (LINGER_LEADER**)HeapAlloc(
            GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS,
            sizeof(LINGER_LEADER *) * MAX_LINGER_LEADER_COUNT);

        while (1)
        {
            rc = SbieApi_QueryConfAsIs(
                NULL, L"LingerProcess", pBox->linger_count,
                image, sizeof(WCHAR) * 120);
            if (rc != 0)
                break;

            Add_LL_Entry(pBox->lingers, &pBox->linger_count, image);
        }

        //
        // see which of the LingerProcess programs were already active
        // before SandboxieRpcSs started.  they will not be considered
        // LingerProcess programs and will not be terminated
        //

        /*ULONG pid_count = 0;
        SbieApi_EnumProcessEx(NULL, FALSE, -1, NULL, &pid_count); // query count
        pid_count += 128;

        ULONG* pids = (ULONG*)HeapAlloc(
            GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, sizeof(ULONG) * pid_count);
        SbieApi_EnumProcessEx(NULL, FALSE, -1, pids, &pid_count); // query pids

        AddPid(pids, pid_count);

        for (i = 0; i <= pid_count; ++i) {

            pids_i = (HANDLE) (ULONG_PTR) pids[i];
            SbieApi_QueryProcess(pids_i, NULL, image, NULL, NULL);

            for (j = 0; j < linger_count; ++j) {
                if (_wcsicmp(lingers[j]->image, image) == 0) {
                    lingers[j]->orig_pid = pids_i;
                    break;
                }
            }
        }

        HeapFree(GetProcessHeap(), 0, pids);*/

        //
        // add standard lingers.  note that we don't check if any of
        // the following processes was already active before we started
        //

        Add_LL_Entry(pBox->lingers, &pBox->linger_count, _SandboxieDcomLaunch);
        Add_LL_Entry(pBox->lingers, &pBox->linger_count, _SandboxieCrypto);
        Add_LL_Entry(pBox->lingers, &pBox->linger_count, _SandboxieBITS);
        Add_LL_Entry(pBox->lingers, &pBox->linger_count, _SandboxieWUAU);
        Add_LL_Entry(pBox->lingers, &pBox->linger_count, _wuauclt);
        Add_LL_Entry(pBox->lingers, &pBox->linger_count, _TrustedInstaller);
        Add_LL_Entry(pBox->lingers, &pBox->linger_count, _tiworker);

        //
        // read and store LeaderProcess configuration
        //

        pBox->leaders = (LINGER_LEADER**)HeapAlloc(
            GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS,
            sizeof(LINGER_LEADER *) * MAX_LINGER_LEADER_COUNT);

        while (1) {

            rc = SbieApi_QueryConfAsIs(
                NULL, L"LeaderProcess", pBox->leader_count,
                image, sizeof(WCHAR) * 120);
            if (rc != 0)
                break;

            Add_LL_Entry(pBox->leaders, &pBox->leader_count, image);
        }
    }

}

void BoxManager::CheckLinger(BOX_INSTANCE* pBox, ULONG SessionId)
{
//    ULONG i, j;
    ULONG j;
    WCHAR image[128];
    ULONG cur_session_id;

   {
        BOOLEAN any_leaders_local = FALSE;
        BOOLEAN terminate_and_stop = TRUE;

        //
        // wait for the process exit event (my_event)
        //

//        WaitForSingleObject(heventRpcSs, INFINITE);
//        EnterCriticalSection(&ProcessCritSec);

//        SbieApi_EnumProcessEx(NULL, FALSE, -1, NULL, &process_count); // query count
//        process_count += 128;

//        ULONG* pids = HeapAlloc(
//            GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, sizeof(ULONG) * (process_count + 1)); // allocate oen more for the -1 marker
//        SbieApi_EnumProcessEx(NULL, FALSE, -1, pids, &process_count); // query pids
//        pids[process_count] = -1; // set the end marker

        //
        // query running processes
        //

        map_iter_t iter = map_iter();
        while (map_next(&pBox->process_map, &iter)) {
            BOXED_PROCESS* pProcess = (BOXED_PROCESS*)iter.value;
//        for (i = 0; i < process_count; ++i) {

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
            BOOLEAN is_local_system_sid = FALSE;

            //pids_i = (HANDLE)(ULONG_PTR)pids[i];
            HANDLE pids_i = (HANDLE)(ULONG_PTR)pProcess->ProcessId;
            image[0] = L'\0';

            if (0 == SbieApi_QueryProcess(pids_i, NULL, image, NULL, &cur_session_id)
                && _wcsicmp(image, _SandboxieCrypto) != 0) {

                //
                // check if this is a local system process
                //

                HANDLE ProcessHandle = SbieDll_OpenProcess(PROCESS_QUERY_INFORMATION, pids_i);
                if (ProcessHandle) {
                    if (SbieDll_CheckProcessLocalSystem(ProcessHandle))
                        is_local_system_sid = TRUE;
                    CloseHandle(ProcessHandle);
                }

                if (!is_local_system_sid) {

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
            // ignore processes fron other sessions
            //

            if (cur_session_id != SessionId)
                continue;

            //
            // ignore rpcss process
            //

            if (_wcsicmp(image, _SandboxieRpcSs) == 0)
                continue;

            //
            // find a leader process
            //

            for (j = 0; j < pBox->leader_count; ++j) {
                if (_wcsicmp(pBox->leaders[j]->image, image) == 0) {
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

            for (j = 0; j < pBox->linger_count; ++j) {
                if (_wcsicmp(pBox->lingers[j]->image, image) == 0) {
                    pBox->lingers[j]->curr_pid = pids_i;
                    if (excluded_from_linger)
                        pBox->lingers[j]->orig_pid = pids_i;
                    if (pids_i == pBox->lingers[j]->orig_pid)
                        terminate_and_stop = FALSE;
                    break;
                }
            }

            if (j == pBox->linger_count) {

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

        if (pBox->any_leaders && (! any_leaders_local)) {

            terminate_and_stop = TRUE;
            goto do_kill_all;

        } else
            pBox->any_leaders = any_leaders_local;

        //
        // don't terminate if a lingering process has an open window
        //

        /*if (terminate_and_stop) {

            //
            // if a process in the PID list has a window LingerEnumWindowsProc will return FALSE
            // what causes the enumeration to abort and EnumWindows to return FALSE as well
            //

            BOOL ret = EnumWindows(LingerEnumWindowsProc, (LPARAM)pids);
            if (ret == FALSE)
                terminate_and_stop = FALSE;
        }*/

        //
        // don't terminate if a lingering process has just started recently
        //

        /*if (terminate_and_stop) {
            for (i = 0; i < process_count; ++i) {
                HANDLE hProcess = NULL;
                ULONG64 ProcessFlags = SbieApi_QueryProcessInfo(
                                pids_i, 0);
                if (! (ProcessFlags & SBIE_FLAG_IMAGE_FROM_SBIE_DIR)) {
                    hProcess = OpenProcess(
                                PROCESS_QUERY_INFORMATION, FALSE, (DWORD)(UINT_PTR)pids_i);
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
        }*/

        //
        // kill all programs and quit
        //

//        LeaveCriticalSection(&ProcessCritSec);
do_kill_all:

//        HeapFree(GetProcessHeap(), 0, pids);

        if (terminate_and_stop) {
            BOOLEAN TerminateJob = FALSE; // todo
            extern ProcessServer* pProcessServer;
            pProcessServer->KillAllHelper(pBox->boxname, SessionId, TerminateJob);
        }
    }
}
#endif