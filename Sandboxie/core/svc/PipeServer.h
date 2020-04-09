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
// Pipe Server
//---------------------------------------------------------------------------


#ifndef _MY_PIPESERVER_H
#define _MY_PIPESERVER_H


#include <windows.h>
#include "common/win32_ntddk.h"
#include "common/list.h"
#include "common/pool.h"
#include "msgids.h"


/* Recommended maximum length for any single element in a request packet. */

#define PIPE_MAX_DATA_LEN   0x00FFFFFF

#define LONG_REPLY(ln)  (PipeServer::GetPipeServer()->AllocMsg(ln))
#define SHORT_REPLY(st) (PipeServer::GetPipeServer()->AllocShortMsg(st))


extern "C" const ULONG tzuk;


class PipeServer
{
public:

    /*
     * Return the instance for the global PipeServer
     */

    static PipeServer *GetPipeServer();

    /*
     * Destructor
     */

    ~PipeServer();

    /*
     * Start the pipe server once all registrations are complete
     */

    bool Start();

    /*
     * Handler function prototype for sub-servers
     */

    typedef MSG_HEADER *(*Handler)(void *context, MSG_HEADER *msg);

    /*
     * Register handler function for a known request message id.
     * if impersonate = TRUE, the sub-server handler will be invoked
     * after impersonating the calling client
     */

    void Register(
        ULONG serverId, void *context, Handler handler);

    /*
     * Manufacture a short reply message with error
     * Sub-servers can use SHORT_REPLY macro
     */

    MSG_HEADER *AllocShortMsg(ULONG status);

    /*
     * Allocate request/reply message buffer
     */

    MSG_HEADER *AllocMsg(ULONG length);

    /*
     * Free request/reply message buffer
     */

    void FreeMsg(MSG_HEADER *msg);

    /*
     * Get process id for caller
     */

    static ULONG GetCallerProcessId();

    /*
     * Get thread id for caller
     */

    static ULONG GetCallerThreadId();

    /*
     * Get session id for caller
     */

    static ULONG GetCallerSessionId();

    /*
     * Impersonate caller security context
     */

    static ULONG ImpersonateCaller(MSG_HEADER **pmsg = NULL);

    /*
     * Process a message in the context of the calling thread
     */

    MSG_HEADER *Call(MSG_HEADER *msg);

protected:

    /*
     * Private constructor
     */

    PipeServer();

    /*
     * Static wrapper for thread function
     */

    static void ThreadStub(void *parm);

    /*
     * Thread function for listening on hServerPort
     */

    void Thread(void);

    /*
     * Port Connect
     */

    void PortConnect(PORT_MESSAGE *msg);

    /*
     * Port Disconnect
     */

    void PortDisconnect(PORT_MESSAGE *msg);

    /*
     * Port Disconnect using process creation time
     */

    void PortDisconnectByCreateTime(LARGE_INTEGER *CreateTime);

    /*
     * Port Request
     */

    void PortRequest(
        HANDLE PortHandle, PORT_MESSAGE *msg, void *voidClient);

    /*
     * Port Reply
     */

    void PortReply(PORT_MESSAGE *msg, void *voidClient);

    /*
     * Port Find Client
     */

    void *PortFindClient(PORT_MESSAGE *msg);

    /*
     * Call a registered sub-server
     */

    MSG_HEADER *CallTarget(
        MSG_HEADER *msg, HANDLE PortHandle, PORT_MESSAGE *PortMessage);

    /*
     * Notify registered sub-servers of an ended process
     */

    void NotifyTargets(HANDLE idProcess);

protected:

    LIST m_targets;
    LIST m_clients;
    CRITICAL_SECTION m_lock;
    POOL *m_pool;
    ULONG m_TlsIndex;

    volatile HANDLE m_hServerPort;
    HANDLE *m_Threads;

    static PipeServer *m_instance;
};


#endif /* _MY_PIPESERVER_H */
