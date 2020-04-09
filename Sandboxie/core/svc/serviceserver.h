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
// Service Server -- using PipeServer
//---------------------------------------------------------------------------

#ifndef _MY_SERVICESERVER_H
#define _MY_SERVICESERVER_H


#include "PipeServer.h"


class ServiceServer
{

public:

    ServiceServer(PipeServer *pipeServer);

protected:

    static MSG_HEADER *Handler(void *_this, MSG_HEADER *msg);

    static MSG_HEADER *StartHandler(MSG_HEADER *msg, HANDLE idProcess);

    static MSG_HEADER *QueryHandler(MSG_HEADER *msg);

    static MSG_HEADER *ListHandler(MSG_HEADER *msg);

    //
    // ServicesServer2
    //

public:

    static void RunUacSlave(const WCHAR *cmdline);

private:

    bool CanCallerDoElevation(
            HANDLE idProcess, const WCHAR *ServiceName, ULONG *pSessionId);

    static void ReportError2218(HANDLE idProcess, ULONG errlvl);

    static WCHAR *BuildPathForStartExe(
        HANDLE idProcess, const WCHAR *devmap, const WCHAR *svcname,
        const WCHAR *InArgs, WCHAR **OutArgs);

    MSG_HEADER *RunHandler(MSG_HEADER *msg, HANDLE idProcess);

    ULONG RunHandler2(
        HANDLE idProcess, ULONG idSession,
        const WCHAR *devmap, const WCHAR *svcname, const WCHAR *path);

    void SetTokenDefaultDacl(HANDLE hNewToken, HANDLE idProcess);

    MSG_HEADER *UacHandler(
        MSG_HEADER *msg, HANDLE idProcess, HANDLE idThread);

    ULONG UacHandler2(
        HANDLE idProcess, HANDLE idThread, const WCHAR *devmap,
        ULONG_PTR pkt_addr, ULONG pkt_len);

    static void RunUacSlave2(ULONG_PTR *ThreadArgs);

    static LRESULT RunUacSlave2WndProc(
        HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    static int RunUacSlave2WndProcTextOut(
        HDC hdc, int y, int height, int msgid);

    static ULONG RunUacSlave2Thread1(void *lpParameters);

    static ULONG RunUacSlave2Thread2(void *lpParameters);

    static void RunUacSlave3(
        HANDLE idProcess, ULONG_PTR pkt_addr, ULONG pkt_len,
        bool JustFail, WCHAR **OutAppName);

    static bool RunUacSlave4(
        HANDLE hClientProcess, void *xpkt, WCHAR **OutAppName);

    static bool CallUacDialogHook(
        ULONG_PTR *ThreadArgs, HANDLE idProcess,
        const WCHAR *cmdline, bool isAdmin);

    static bool IsAdmin();

};


#endif /* _MY_SERVICESERVER_H */
