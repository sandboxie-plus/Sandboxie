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
// Driver Assistant
//---------------------------------------------------------------------------


#ifndef _MY_DRIVERASSIST_H
#define _MY_DRIVERASSIST_H

#include <map>
#include <vector>
#include <string>

class DriverAssist
{

    //
    // main and driver initialization functions
    //

public:

    static bool Initialize();

    static void Shutdown();

    static bool IsDriverReady();

    static bool LookupSidCached(const PSID pSid, 
        WCHAR *UserName, ULONG* UserNameLen);

private:

    DriverAssist();

	~DriverAssist();

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
    // mounted registry hive
    //

    void HiveMounted(void *_msg);

    //
    // unmount registry hive
    //

    void UnmountHive(void *_msg);

    //
    // log messages to file
    //

    void LogMessage(void *_msg);

    void LogMessage_Single(ULONG code, wchar_t* data, ULONG pid);
    void LogMessage_Multi(ULONG msgid, const WCHAR *path, const WCHAR *text);
    void LogMessage_Write(const WCHAR *path, const WCHAR *text);

    //
    // functions to inject low level code layer into new process
    //

	bool InjectLow_Init();

    void InjectLow(void *_msg);
	HANDLE InjectLow_OpenProcess(void *_msg);

    //
    // SbieLogin
    //

    void InitSIDs();

    bool GetSandboxieSID(const WCHAR* boxname, UCHAR* SandboxieLogonSid, DWORD dwSidSize);

    void CleanUpSIDs();

    //
    // data
    //

private:

    static DriverAssist *m_instance;

    volatile HANDLE m_PortHandle;
    HANDLE *m_Threads;

    volatile bool m_DriverReady;

	ULONG m_last_message_number;

    std::map<std::wstring, std::wstring> m_SidCache;

    //
    // critical sections
    //

    CRITICAL_SECTION m_LogMessage_CritSec;
    CRITICAL_SECTION m_critSecHostInjectedSvcs;
    CRITICAL_SECTION m_SidCache_CritSec;
};


#endif /* _MY_DRIVERASSIST_H */
