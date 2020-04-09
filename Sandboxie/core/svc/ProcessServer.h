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
// Process Server -- using PipeServer
//---------------------------------------------------------------------------


#ifndef _MY_PROCESSSERVER_H
#define _MY_PROCESSSERVER_H


#include "PipeServer.h"


class ProcessServer
{

public:

    ProcessServer(PipeServer *pipeServer);

protected:

    static MSG_HEADER *Handler(void *_this, MSG_HEADER *msg);

    MSG_HEADER *CheckInitCompleteHandler();

    BOOL KillProcess(ULONG ProcessId);

    MSG_HEADER *KillOneHandler(HANDLE CallerProcessId, MSG_HEADER *msg);

    MSG_HEADER *KillAllHandler(HANDLE CallerProcessId, MSG_HEADER *msg);

    NTSTATUS KillAllHelper(const WCHAR *BoxName, ULONG SessionId);

    MSG_HEADER *SetDeviceMap(HANDLE CallerProcessId, MSG_HEADER *msg);

    MSG_HEADER *OpenDeviceMap(HANDLE CallerProcessId, MSG_HEADER *msg);

    //
    // Run Sandboxed
    //

    MSG_HEADER *RunSandboxedHandler(MSG_HEADER *msg);
    WCHAR *RunSandboxedCopyString(MSG_HEADER *msg, ULONG ofs, ULONG len);
    HANDLE RunSandboxedGetToken(
            HANDLE CallerProcessHandle, bool CallerInSandbox,
            const WCHAR *BoxName);
    BOOL RunSandboxedSetDacl(
            HANDLE CallerProcessHandle, HANDLE NewTokenHandle);
    BOOL RunSandboxedStartProcess(
            HANDLE PrimaryTokenHandle, LONG_PTR BoxNameOrModelPid,
            ULONG CallerProcessId,
            WCHAR *cmd, const WCHAR *dir, WCHAR *env, ULONG *crflags,
            STARTUPINFO *si, PROCESS_INFORMATION *pi);
    WCHAR *RunSandboxedComServer(ULONG CallerProcessId);
    BOOL RunSandboxedDupAndCloseHandles(
            HANDLE CallerProcessHandle, ULONG crflags,
            PROCESS_INFORMATION *piInput, PROCESS_INFORMATION *piReply);

protected:

    CRITICAL_SECTION m_RunSandboxed_CritSec;

};


#endif /* _MY_PROCESSSERVER_H */
