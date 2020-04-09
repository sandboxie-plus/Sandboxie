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
// Driver Assistant
//---------------------------------------------------------------------------


#ifndef _MY_DRIVERASSIST_H
#define _MY_DRIVERASSIST_H


class DriverAssist
{

    //
    // main and driver initialization functions
    //

public:

    static bool Initialize();

    static void Shutdown();

    static bool IsDriverReady();


private:

    DriverAssist();

    bool InitializePortAndThreads();

    void ShutdownPortAndThreads();

    static ULONG StartDriverAsync(void *arg);

    static void InitClipboard();

    //
    // main listening & message worker threads
    //

    static void ThreadStub(void *parm);

    static DWORD MsgWorkerThreadStub(void * msg);

    void MsgWorkerThread(void * msg);

    void Thread();

    //
    // function to translate SID strings to user account names
    //

    void LookupSid(void *_msg);
    void LookupSid2(const WCHAR *SidString,
                    WCHAR *UserName, ULONG UserNameLen);

    //
    // cancel process in response to request from driver
    //

    void CancelProcess(void *_msg);

    //
    // restart HostInjectProcess's that are services in response to request from driver
    //

    void RestartHostInjectedSvcs();

    //
    // unmount registry hive
    //

    void UnmountHive(void *_msg);

    //
    // log messages to file
    //

    void LogMessage();

    void LogMessage_Single(void *data);
    void LogMessage_Multi(ULONG msgid, const WCHAR *path, const WCHAR *text);
    void LogMessage_Write(const WCHAR *path, const WCHAR *text);

    //
    // functions to inject low level code layer into new process
    //

    bool InjectLow_Init();
    bool InjectLow_InitHelper(ULONG *errlvl);
    static ULONG_PTR InjectLow_FindHighestAddress();
    ULONG InjectLow_InitSyscalls();

    void InjectLow(void *_msg);
    HANDLE InjectLow_OpenProcess(void *_msg);
    HANDLE InjectLow_SendHandle(HANDLE hProcess);
    void *InjectLow_CopyCode(HANDLE hProcess, BOOLEAN iswow64,
                             UCHAR *code, ULONG code_len);
    bool InjectLow_BuildTramp(
            void *_msg, BOOLEAN long_diff, UCHAR *code, ULONG_PTR addr);
    void *InjectLow_CopySyscalls(HANDLE hProcess);
    bool InjectLow_CopyData(
            HANDLE hProcess, void *remote_addr, void *local_data);
#ifdef _WIN64
    void * InjectLow_getPage(HANDLE hProcess,void *remote_addr);
    bool DriverAssist::Has32BitJumpHorizon(void * target, void * detour);
#endif
    bool InjectLow_WriteJump(
            HANDLE hProcess, void *remote_addr, BOOLEAN long_diff, void * local_data);

    //
    // data
    //

private:

    static DriverAssist *m_instance;

    volatile HANDLE m_PortHandle;
    HANDLE *m_Threads;

    volatile bool m_DriverReady;

    //
    // critical sections
    //

    CRITICAL_SECTION m_LogMessage_CritSec;
    CRITICAL_SECTION m_critSecHostInjectedSvcs;

    //
    // DriverAssistInject data
    //

    void *m_sbielow_ptr;
    //adding two offsets variables to replace the "head" and "tail" dependency
    ULONG m_sbielow_start_offset;
    ULONG m_sbielow_data_offset;

    ULONG m_sbielow_len;

    ULONG *m_syscall_data;

    ULONG_PTR m_LdrInitializeThunk;

};


#endif /* _MY_DRIVERASSIST_H */
