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

#include "stdafx.h"

#include <sddl.h>
#include <stdio.h>
#include <psapi.h>
#include <winioctl.h>
#include <ioapiset.h>

#include "misc.h"
#include "DriverAssist.h"
#include "common/defines.h"
#include "common/my_version.h"
#include "core/dll/sbiedll.h"
#include "core/drv/api_defs.h"
#include "sbieiniserver.h"
#include "MountManager.h"

//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------

typedef struct _MSG_DATA
{
    void *ClassContext;
    UCHAR msg[MAX_PORTMSG_LENGTH];
} MSG_DATA;


DriverAssist *DriverAssist::m_instance = NULL;


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


DriverAssist::DriverAssist()
{
    m_PortHandle = NULL;
    m_Threads = NULL;
    m_DriverReady = false;

	m_last_message_number = 0;

    InitializeCriticalSection(&m_LogMessage_CritSec);
    InitializeCriticalSection(&m_critSecHostInjectedSvcs);
    InitializeCriticalSection(&m_SidCache_CritSec);
}

DriverAssist::~DriverAssist()
{
	DeleteCriticalSection(&m_LogMessage_CritSec);
	DeleteCriticalSection(&m_critSecHostInjectedSvcs);
	DeleteCriticalSection(&m_SidCache_CritSec);
}


//---------------------------------------------------------------------------
// Initialize
//---------------------------------------------------------------------------


bool DriverAssist::Initialize()
{
    m_instance = new DriverAssist();
    ULONG tid;
    HANDLE hThread;

    if (!m_instance) {
        return false;
    }

    if (!m_instance->InjectLow_Init()) {
        return false;
    }
    if (!m_instance->InitializePortAndThreads()) {
        return false;
    }

    hThread = CreateThread(NULL, 0,
        (LPTHREAD_START_ROUTINE)StartDriverAsync, m_instance, 0, &tid);
	CloseHandle(hThread);

    return true;
}


//---------------------------------------------------------------------------
// InitializePortAndThreads
//---------------------------------------------------------------------------


bool DriverAssist::InitializePortAndThreads()
{
    NTSTATUS status;
    UNICODE_STRING objname;
    OBJECT_ATTRIBUTES objattrs;
    WCHAR PortName[64];
    PSECURITY_DESCRIPTOR sd;
    ULONG i, n;

    InitSIDs();

    //
    // create a security descriptor with a limited DACL
    // owner:system, group:system, dacl(allow;generic_all;system)
    //

    if (! ConvertStringSecurityDescriptorToSecurityDescriptor(
            L"O:SYG:SYD:(A;;GA;;;SY)", SDDL_REVISION_1, &sd, NULL)) {
        LogEvent(MSG_9234, 0x9244, GetLastError());
        return false;
    }

    //
    // create LPC port which the driver will use to send us messages
    // the port must have a name, or LpcRequestPort in SbieDrv will fail
    //

    wsprintf(PortName, L"%s-internal-%d",
             SbieDll_PortName(), GetTickCount());
    RtlInitUnicodeString(&objname, PortName);

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, sd);

    status = NtCreatePort(
        (HANDLE *)&m_PortHandle, &objattrs, 0, MAX_PORTMSG_LENGTH, NULL);

    if (! NT_SUCCESS(status)) {
        LogEvent(MSG_9234, 0x9254, status);
        return false;
    }

    LocalFree(sd);

    //
    // make sure threads on other CPUs will see the port
    //

    InterlockedExchangePointer(&m_PortHandle, m_PortHandle);

    //
    // create the worker threads
    //

    n = (NUMBER_OF_THREADS) * sizeof(HANDLE);
    m_Threads = (HANDLE *)HeapAlloc(GetProcessHeap(), 0, n);
    if (! m_Threads) {
        LogEvent(MSG_9234, 0x9251, GetLastError());
        return false;
    }
    memzero(m_Threads, n);

    for (i = 0; i < NUMBER_OF_THREADS; ++i) {

        m_Threads[i] = CreateThread(
            NULL, 0, (LPTHREAD_START_ROUTINE)ThreadStub, this, 0, &n);
        if (! m_Threads[i]) {
            LogEvent(MSG_9234, 0x9253, GetLastError());
            return false;
        }
    }

    return true;
}


//---------------------------------------------------------------------------
// Shutdown
//---------------------------------------------------------------------------


void DriverAssist::Shutdown()
{
    if (m_instance) {

        m_instance->ShutdownPortAndThreads();

        delete m_instance;
        m_instance = NULL;
    }
}


//---------------------------------------------------------------------------
// ShutdownPortAndThreads
//---------------------------------------------------------------------------


void DriverAssist::ShutdownPortAndThreads()
{
    ULONG i;

    HANDLE PortHandle = InterlockedExchangePointer(&m_PortHandle, NULL);

    if (PortHandle) {

        UCHAR space[MAX_PORTMSG_LENGTH];

        for (i = 0; i < NUMBER_OF_THREADS; ++i) {
            PORT_MESSAGE *msg = (PORT_MESSAGE *)space;
            memzero(msg, MAX_PORTMSG_LENGTH);
            msg->u1.s1.TotalLength = (USHORT)sizeof(PORT_MESSAGE);
            NtRequestPort(PortHandle, msg);
        }
    }

    if (m_Threads) {

        if (WAIT_TIMEOUT == WaitForMultipleObjects(
                                NUMBER_OF_THREADS, m_Threads, TRUE, 5000)) {

            for (i = 0; i < NUMBER_OF_THREADS; ++i)
                TerminateThread(m_Threads[i], 0);
            WaitForMultipleObjects(NUMBER_OF_THREADS, m_Threads, TRUE, 5000);
        }
    }

    if (PortHandle)
        NtClose(PortHandle);

    CleanUpSIDs();
}


//---------------------------------------------------------------------------
// ThreadStub
//---------------------------------------------------------------------------


void DriverAssist::ThreadStub(void *parm)
{
    ((DriverAssist *)parm)->Thread();
}


//---------------------------------------------------------------------------
// Thread
//---------------------------------------------------------------------------

void DriverAssist::MsgWorkerThread(void *MyMsg)
{
    PORT_MESSAGE *msg = (PORT_MESSAGE *)MyMsg;
    //Null pointer checked by caller
    if (msg->u2.s2.Type != LPC_DATAGRAM) {
        return;
    }
    ULONG data_len = msg->u1.s1.DataLength;
    if (data_len < sizeof(ULONG)) {
        return;
    }
    data_len -= sizeof(ULONG);

    ULONG *data_ptr = (ULONG *)((UCHAR *)msg + sizeof(PORT_MESSAGE));
    ULONG msgid = *data_ptr;
    ++data_ptr;

    if (msgid == SVC_LOOKUP_SID) {

        LookupSid(data_ptr);

    }
    else if (msgid == SVC_INJECT_PROCESS) {

        InjectLow(data_ptr);

    }
    else if (msgid == SVC_CANCEL_PROCESS) {

        CancelProcess(data_ptr);

    }
    else if (msgid == SVC_MOUNTED_HIVE) {

        HiveMounted(data_ptr);

    }
    else if (msgid == SVC_UNMOUNT_HIVE) {

        UnmountHive(data_ptr);

    }
    else if (msgid == SVC_LOG_MESSAGE) {

        LogMessage(data_ptr);

    }
    else if (msgid == SVC_CONFIG_UPDATED) {

#ifdef NEW_INI_MODE

        //
        // In case the ini was edited externally, i.e. by notepad.exe 
        // we update the ini cache each time the driver reloads the ini file.
        // 
        // In newer builds the driver tells us which process issued the reload
        // if we did it we don't need to purge the cached ini data
        //

        if(data_len < sizeof(ULONG) || *(ULONG*)data_ptr != GetCurrentProcessId())
            SbieIniServer::NotifyConfigReloaded();
#endif

        SbieDll_InjectLow_InitSyscalls(TRUE);

        RestartHostInjectedSvcs();
    }
}

DWORD DriverAssist::MsgWorkerThreadStub(void *MyMsg)
{
    if (!MyMsg) {
        return -1;
    }

    MSG_DATA* MsgData = (MSG_DATA*)MyMsg;
    ((DriverAssist *)(MsgData->ClassContext))->MsgWorkerThread(&MsgData->msg[0]);
    //Memory allocated in parent thread
    VirtualFree(MyMsg, 0, MEM_RELEASE);

    return NO_ERROR;
}

void DriverAssist::Thread()
{
    NTSTATUS status;
	HANDLE hThread;
    DWORD threadId;
    MSG_DATA *MsgData;

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    while (1) {

        MsgData = (MSG_DATA*)VirtualAlloc(0, sizeof(MSG_DATA), MEM_COMMIT, PAGE_READWRITE);
        if (!MsgData) {
            break;  // out of memory
        }

        status = NtReplyWaitReceivePort(m_PortHandle, NULL, NULL, (PORT_MESSAGE *)MsgData->msg);

        if (!m_PortHandle) {    // service is shutting down
            VirtualFree(MsgData, 0, MEM_RELEASE);
            break;
        }

        MsgData->ClassContext = this;
		hThread = CreateThread(NULL, 0, MsgWorkerThreadStub, (void *)MsgData, 0, &threadId);
		if (hThread)
			CloseHandle(hThread);
		else
			VirtualFree(MsgData, 0, MEM_RELEASE);
    }
}


//---------------------------------------------------------------------------
// LookupSidCached
//---------------------------------------------------------------------------


bool DriverAssist::LookupSidCached(const PSID pSid, WCHAR* UserName, ULONG* UserNameLen)
{
    bool ok = false;
    WCHAR domain[256];
    ULONG domain_len = sizeof(domain) / sizeof(WCHAR) - 4;
    SID_NAME_USE use;

    LPWSTR pStr;
    if (!ConvertSidToStringSid(pSid, &pStr))
        return false;


    EnterCriticalSection(&m_instance->m_SidCache_CritSec);

    auto I = m_instance->m_SidCache.find(pStr);
    if (I != m_instance->m_SidCache.end())
    {
        wcscpy_s(UserName, *UserNameLen, I->second.c_str());
        *UserNameLen = I->second.length();
        ok = true;
    }
    
    LeaveCriticalSection(&m_instance->m_SidCache_CritSec);


    if (!ok) {

        ok = LookupAccountSid(NULL, pSid, UserName, UserNameLen, domain, &domain_len, &use);

        if (ok) {

            EnterCriticalSection(&m_instance->m_SidCache_CritSec);

            m_instance->m_SidCache[pStr] = UserName;

            LeaveCriticalSection(&m_instance->m_SidCache_CritSec);
        }
    }


    LocalFree(pStr);

    return ok;
}


//---------------------------------------------------------------------------
// LookupSid
//---------------------------------------------------------------------------


void DriverAssist::LookupSid(void *_msg)
{
    SVC_LOOKUP_SID_MSG *msg = (SVC_LOOKUP_SID_MSG *)_msg;

    PSID pSid;
    BOOL b = ConvertStringSidToSid(msg->sid_string, &pSid);
    if (! b) {
        SbieApi_LogEx(msg->session_id, 2209, L"[11 / %d]", GetLastError());
        return;
    }

    WCHAR username[256];
    ULONG username_len = sizeof(username) / sizeof(WCHAR) - 4;
    //WCHAR domain[256];
    //ULONG domain_len = sizeof(domain) / sizeof(WCHAR) - 4;
    //SID_NAME_USE use;

    username[0] = L'\0';

    //b = LookupAccountSid(
    //    NULL, pSid, username, &username_len, domain, &domain_len, &use);
    b = LookupSidCached(pSid, username, &username_len);

    if ((! b) && GetLastError() == ERROR_NONE_MAPPED) {

        username_len = sizeof(username) / sizeof(WCHAR) - 4;
        username[0] = L'\0';
        LookupSid2(msg->sid_string, username, username_len);
        if (username[0])
            b = TRUE;
        else
            SetLastError(ERROR_NONE_MAPPED);
    }

    if ((! b) || (! username[0])) {
        //SbieApi_LogEx(msg->session_id, 2209, L": %S [22 / %d]", msg->sid_string, GetLastError());
        wcscpy(username, L"*?*?*?*");
    }

    LocalFree(pSid);

    username[sizeof(username) / sizeof(WCHAR) - 4] = L'\0';

    LONG rc = SbieApi_SetUserName(msg->sid_string, username);
    if (rc != 0)
        SbieApi_LogEx(msg->session_id, 2209, L"[33 / %08X]", rc);
}


//---------------------------------------------------------------------------
// LookupSid2
//---------------------------------------------------------------------------


void DriverAssist::LookupSid2(
    const WCHAR *SidString, WCHAR *UserName, ULONG UserNameLen)
{
    WCHAR *KeyPath = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, 1024);
    if (! KeyPath)
        return;
    wcscpy(KeyPath, SidString);
    wcscat(KeyPath,
                L"\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer");

    HKEY hKey;
    LONG rc = RegOpenKeyEx(HKEY_USERS, KeyPath, 0, KEY_READ, &hKey);
    if (rc == 0) {

        ULONG type, len = UserNameLen;
        rc = RegQueryValueEx(hKey, L"Logon User Name", NULL,
                             &type, (LPBYTE)UserName, &len);
        if (rc != 0 || type != REG_SZ)
            UserName[0] = L'\0';

        RegCloseKey(hKey);
    }

    HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, KeyPath);
}


//---------------------------------------------------------------------------
// CancelProcess
//---------------------------------------------------------------------------


void DriverAssist::CancelProcess(void *_msg)
{
    //
    // cancel process in response to request from driver
    //

    SVC_PROCESS_MSG *msg = (SVC_PROCESS_MSG *)_msg;

    const ULONG _DesiredAccess = PROCESS_TERMINATE
                               | PROCESS_QUERY_INFORMATION;

    HANDLE hProcess = OpenProcess(_DesiredAccess, FALSE, msg->process_id);

    if (hProcess) {

        FILETIME time, time1, time2, time3;
        BOOL ok = GetProcessTimes(hProcess, &time, &time1, &time2, &time3);
        if (ok && *(ULONG64 *)&time.dwLowDateTime == msg->create_time) {

            TerminateProcess(hProcess, 1);
        }

        CloseHandle(hProcess);
    }

    if (msg->reason == 0)
        SbieApi_LogEx(msg->session_id, 2314, msg->process_name);
	else if (msg->reason != -1) // in this case we have SBIE1308 and don't want any other messages
		SbieApi_LogEx(msg->session_id, 2314, L"%S [%d / %d]", msg->process_name, msg->process_id, msg->reason);
}


extern void RestartHostInjectedSvcs();

void DriverAssist::RestartHostInjectedSvcs()
{
    //
    // SbieCtrl issues a refresh on every setting change,
    // resulting in this function getting triggered way to often, 
    // hence we implement a small workaround.
    // The first thread to hit this monitors how many 
    // calls go in and waits until the last one,
    // then it starts the Job.
    //

    static volatile ULONG JobCounter = 0;
    if (InterlockedIncrement(&JobCounter) == 1) {
        do {
            Sleep(250);
        } while (JobCounter > 1);
        EnterCriticalSection(&m_critSecHostInjectedSvcs);
        ::RestartHostInjectedSvcs();
        LeaveCriticalSection(&m_critSecHostInjectedSvcs);
    }
    InterlockedDecrement(&JobCounter);
}


//---------------------------------------------------------------------------
// VolHas8dot3Support
//---------------------------------------------------------------------------

// VolumeFlags bit values (see FILE_FS_PERSISTENT_VOLUME_INFORMATION.VolumeFlags)
#define PERSISTENT_VOLUME_STATE_SHORT_NAME_CREATION_DISABLED 0x00000001		// No 8.3 name creation on this volume

//
// Structure for FSCTL_SET_PERSISTENT_VOLUME_STATE and FSCTL_GET_PERSISTENT_VOLUME_STATE
// The initial version will be 1.0
//
typedef struct _FILE_FS_PERSISTENT_VOLUME_INFORMATION {

    ULONG VolumeFlags;
    ULONG FlagMask;
    ULONG Version;
    ULONG Reserved;

} FILE_FS_PERSISTENT_VOLUME_INFORMATION, *PFILE_FS_PERSISTENT_VOLUME_INFORMATION;

// FltFsControlFile or ZwFsControlFile call # to query persistent volume info (if used in DDK)
// CAN ALSO USE WITH: DeviceIOControl (which is what we will do)
#define FSCTL_QUERY_PERSISTENT_VOLUME_STATE CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 143, METHOD_BUFFERED, FILE_ANY_ACCESS)  // FILE_FS_PERSISTENT_VOLUME_INFORMATION

BOOL VolHas8dot3Support(WCHAR* path)
{
    BOOL is8Dot3 = TRUE;

    HANDLE hFile;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING uni;
    //NTSTATUS status;
    //IO_STATUS_BLOCK MyIoStatusBlock;

    RtlInitUnicodeString(&uni, path);
    InitializeObjectAttributes(&objattrs, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

    //status = NtOpenFile(&hFile, GENERIC_READ, &objattrs, &MyIoStatusBlock, FILE_SHARE_READ | FILE_SHARE_WRITE, 0);
    //if (NT_SUCCESS(status)) {
    WCHAR device[] = L"\\\\.\\X:";
    device[4] = path[0];
    hFile = CreateFile(device, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, (HANDLE)NULL);
    if (hFile != INVALID_HANDLE_VALUE) {

        FILE_FS_PERSISTENT_VOLUME_INFORMATION inbuf = { 0 };
	    FILE_FS_PERSISTENT_VOLUME_INFORMATION outbuf = { 0 };
	    inbuf.FlagMask = PERSISTENT_VOLUME_STATE_SHORT_NAME_CREATION_DISABLED;
	    inbuf.Version = 1;
        
        //status = NtFsControlFile(hFile, NULL, NULL, NULL, &MyIoStatusBlock, FSCTL_QUERY_PERSISTENT_VOLUME_STATE, &inbuf, sizeof(inbuf), &outbuf, sizeof(outbuf));
        //if(NT_SUCCESS(status)) {
        DWORD BytesReturned;
        if(DeviceIoControl(hFile, FSCTL_QUERY_PERSISTENT_VOLUME_STATE, &inbuf, sizeof(inbuf), &outbuf, sizeof(outbuf), &BytesReturned, 0)){

		    is8Dot3 = (outbuf.VolumeFlags & PERSISTENT_VOLUME_STATE_SHORT_NAME_CREATION_DISABLED) ? FALSE : TRUE;
	    }

        NtClose(hFile);
    }

    return is8Dot3;
}


//---------------------------------------------------------------------------
// HiveMounted
//---------------------------------------------------------------------------


void DriverAssist::HiveMounted(void *_msg)
{
    SVC_REGHIVE_MSG *msg = (SVC_REGHIVE_MSG *)_msg;

	ULONG errlvl = 0;
    WCHAR* file_root_path = NULL;
    WCHAR* reg_root_path = NULL;

    ULONG file_len = 0;
    ULONG reg_len = 0;
    if (!NT_SUCCESS(SbieApi_QueryProcessPath((HANDLE)msg->process_id, NULL, NULL, NULL, &file_len, &reg_len, NULL))) {
        errlvl = 0x12;
        goto finish;
    }
    file_root_path = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, file_len + 16);
    reg_root_path = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, reg_len + 16);
    if (!file_root_path || !reg_root_path) {
        errlvl = 0x13;
        goto finish;
    }
    if (!NT_SUCCESS(SbieApi_QueryProcessPath((HANDLE)msg->process_id, file_root_path, reg_root_path, NULL, &file_len, &reg_len, NULL))) {
        errlvl = 0x14;
        goto finish;
    }

    //
    // lock box root if present
    //

    MountManager::GetInstance()->LockBoxRoot(reg_root_path, msg->session_id);

    //
    // check if the box is located on a volume without 8.3 naming
    // as this may cause issues with old installers
    //

    if (SbieApi_QueryConfBool(msg->boxname, L"EnableVerboseChecks", FALSE)) {

        if (SbieDll_TranslateNtToDosPath(reg_root_path)) { // wcslen(reg_root_path) > 22 &&

            if (!VolHas8dot3Support(reg_root_path)) {

                SbieApi_LogEx(msg->session_id, 2227, L"%S (%S)", msg->boxname, reg_root_path);
            }
        }
    }

    //
    // finish
    //

finish:
    if (file_root_path) 
        HeapFree(GetProcessHeap(), 0, file_root_path);
    if (reg_root_path) 
        HeapFree(GetProcessHeap(), 0, reg_root_path);
}


//---------------------------------------------------------------------------
// UnmountHive
//---------------------------------------------------------------------------


void DriverAssist::UnmountHive(void *_msg)
{
    SVC_REGHIVE_MSG *msg = (SVC_REGHIVE_MSG *)_msg;
    ULONG rc, retries;

    //
    // we got a message that specifies the pid of the last process in
    // a box, we're going to wait until that process disappears
    //

    bool ended = false;

    HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, msg->process_id);
    if (hProcess) {

        if (WaitForSingleObject(hProcess, 2 * 1000) == STATUS_SUCCESS) {

            ended = true;
        }

        CloseHandle(hProcess);
    }

    if (! ended) {

        for (retries = 0; retries < 20; ++retries) {

            rc = SbieApi_QueryProcess((HANDLE)(ULONG_PTR)msg->process_id,
                                      NULL, NULL, NULL, NULL);
            if (rc != 0)
                break;

            Sleep(100);
        }
    }

    //
    // it could be that we are invoked because Start.exe terminated, but
    // its spawned child process has not yet asked to mount the registry
    // for it.  I.e.,  the registry use count has dropped to zero, even
    // though another process is going to ask for that registry very soon.
    //
    // to make sure we don't unmount in this case, only to re-mount,
    // we check that the sandbox is empty, before issuing the unmount
    //

    bool ShouldUnmount = false;

    for (retries = 0; retries < 20; ++retries) {

        ULONG count = 0;
        rc = SbieApi_EnumProcessEx(
                            msg->boxname, FALSE, msg->session_id, NULL, &count);
        if (rc == 0 && count == 0) {

            ShouldUnmount = true;
            break;
        }

        Sleep(100);
    }


    //
    // unmount.  on Windows 2000, the process may appear to disappear
    // even before its handles were all closed (in particular, registry
    // handles), which could lead to SBIE2208 being reported.  so we
    // retry the operation
    //

    while (ShouldUnmount) {

        WCHAR root_path[MAX_REG_ROOT_LEN];
        UNICODE_STRING root_uni;
        OBJECT_ATTRIBUTES root_objattrs;
        HANDLE root_key;

        SbieApi_GetUnmountHive(root_path);
        if (! root_path[0])
            break;

        RtlInitUnicodeString(&root_uni, root_path);
        InitializeObjectAttributes(&root_objattrs,
            &root_uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

        for (retries = 0; retries < 25; ++retries) {

            rc = NtUnloadKey(&root_objattrs);
            if (rc == 0)
                break;

            Sleep(100);

            rc = NtOpenKey(&root_key, KEY_READ, &root_objattrs);
            if (rc == STATUS_OBJECT_NAME_NOT_FOUND ||
                rc == STATUS_OBJECT_PATH_NOT_FOUND)
                break;
            if (rc == 0)
                NtClose(root_key);
        }

        if (rc == 0) {

            //
            // unmount box container if present
            //

            MountManager::GetInstance()->ReleaseBoxRoot(root_path, false, msg->session_id);
        }

        if (rc != 0)
            SbieApi_LogEx(msg->session_id, 2208, L"[%08X]", rc);

        break;
    }
}


//---------------------------------------------------------------------------
// SbieLow Injection
//---------------------------------------------------------------------------


#include "DriverAssistStart.cpp"
#include "DriverAssistInject.cpp"
#include "DriverAssistLog.cpp"
