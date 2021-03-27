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
// GUI Proxy Server
//---------------------------------------------------------------------------


#ifndef _MY_GUISERVER_H
#define _MY_GUISERVER_H


#include "common/list.h"


class GuiServer
{

public:

    static GuiServer *GetInstance();

    bool InitProcess(HANDLE hProcess, ULONG process_id, ULONG session_id,
                     BOOLEAN add_to_job);

    bool InitConsole(HANDLE hProcess, ULONG process_id, ULONG session_id);

    static void RunSlave(const WCHAR *cmdline);

protected:

    GuiServer();

    ULONG StartSlave(ULONG session_id);

    ULONG SendMessageToSlave(ULONG session_id, ULONG process_id,
                             BOOLEAN add_to_job);

    static void ReportError2336(
                        ULONG session_id, ULONG errlvl, ULONG status);

    static void RunConsoleSlave(const WCHAR *evtname);

    static void ConsoleCallbackSlave(void *arg, BOOLEAN timeout);

    static void DropConsoleIntegrity();

    static void AdjustConsoleTaskbarButton();

    static ULONG DdeProxyThreadSlave(void *xDdeArgs);

protected:

    static void *GetNtUserQueryWindow(void);

    bool CreateQueueSlave(const WCHAR *cmdline);

    static void QueueCallbackSlave(void *arg, BOOLEAN timeout);

    bool QueueCallbackSlave2(void);

    HANDLE GetJobObjectForAssign(const WCHAR *boxname);

    HANDLE GetJobObjectForGrant(ULONG pid);

    bool GetWindowStationAndDesktopName(WCHAR *out_name);

protected:

    struct SlaveArgs {
        ULONG pid;
        ULONG req_len;
        ULONG rpl_len;
        void *req_buf;
        void *rpl_buf;
    };
    typedef ULONG (GuiServer::*SlaveFunc)(SlaveArgs *args);
    SlaveFunc *m_SlaveFuncs;

    ULONG InitProcessSlave(SlaveArgs *args);

    ULONG GetWindowStationSlave(SlaveArgs *args);

    ULONG CreateConsoleSlave(SlaveArgs *args);

    ULONG QueryWindowSlave(SlaveArgs *args);

    ULONG IsWindowSlave(SlaveArgs *args);

    ULONG GetWindowLongSlave(SlaveArgs *args);

    ULONG GetWindowPropSlave(SlaveArgs *args);

    ULONG GetWindowHandleSlave(SlaveArgs *args);

    ULONG GetClassNameSlave(SlaveArgs *args);

    ULONG GetWindowRectSlave(SlaveArgs *args);

    ULONG GetWindowInfoSlave(SlaveArgs *args);

    ULONG GrantHandleSlave(SlaveArgs *args);

    ULONG EnumWindowsSlave(SlaveArgs *args);

    ULONG EnumWindowsFilterSlave(ULONG pid, void *rpl_buf);

    static BOOL EnumWindowsSlaveEnumProc(HWND hwnd, LPARAM lParam);

    ULONG FindWindowSlave(SlaveArgs *args);

    ULONG MapWindowPointsSlave(SlaveArgs *args);

    ULONG SetWindowPosSlave(SlaveArgs *args);

    ULONG CloseClipboardSlave(SlaveArgs *args);

    ULONG GetClipboardDataSlave(SlaveArgs *args);

    HANDLE GetClipboardDataSlave2(ULONG pid, void *mem_ptr, SIZE_T mem_len);

    void *GetClipboardBitmapSlave(void *hBitmap);

    void *GetClipboardEnhMetaFileSlave(void *hEnhMetaFile);

    ULONG GetClipboardMetaFileSlave(SlaveArgs *args);

    ULONG SendPostMessageSlave(SlaveArgs *args);

    ULONG SendCopyDataSlave(SlaveArgs *args);

    ULONG ClipCursorSlave(SlaveArgs *args);

    ULONG SetForegroundWindowSlave(SlaveArgs *args);

    ULONG MonitorFromWindowSlave(SlaveArgs *args);

    ULONG SplWow64Slave(SlaveArgs *args);

    void  SplWow64SlaveWin8();

    ULONG ChangeDisplaySettingsSlave(SlaveArgs *args);

    ULONG SetCursorPosSlave(SlaveArgs *args);

    ULONG RemoveHostWindow(SlaveArgs *args);

    ULONG GetRawInputDeviceInfoSlave(SlaveArgs* args);

    ULONG WndHookNotifySlave(SlaveArgs* args);

    ULONG WndHookRegisterSlave(SlaveArgs* args);

    //
    // window access check utilities
    //

    ULONG GetProcessPathList(
        ULONG pid, void **out_pool, LIST **out_list);

    bool CheckProcessPathList(LIST *list, const WCHAR *str);

    bool CheckSameProcessBoxes(
        ULONG in_pid, WCHAR *boxname, HWND hwnd, ULONG *out_pid);

    bool CheckWindowAccessible(
        ULONG pid, WCHAR *boxname, LIST *list, HWND hwnd);

    bool CompareIntegrityLevels(ULONG src_pid, HWND dst_hwnd);

    bool ShouldIgnoreIntegrityLevels(ULONG pid, HWND hwnd);

    bool AllowSendPostMessage(
        ULONG pid, ULONG msg, bool IsSendMsg, HWND hwnd);

    //
    // data
    //

protected:

    CRITICAL_SECTION m_SlavesLock;
    LIST m_SlavesList;
    HANDLE m_QueueEvent;

    WCHAR *m_QueueName;
    ULONG m_ParentPid;
    ULONG m_SessionId;
    ULONG m_nOSVersion;

    LIST m_WndHooks;
};


#endif /* _MY_GUISERVER_H */
