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
// Named Pipe Proxy Server -- using PipeServer
//---------------------------------------------------------------------------


#ifndef _MY_NAMEDPIPESERVER_H
#define _MY_NAMEDPIPESERVER_H


#include "PipeServer.h"
#include "ProxyHandle.h"


class NamedPipeServer
{

public:

    NamedPipeServer(PipeServer *pipeServer);

protected:

    static MSG_HEADER *Handler(void *_this, MSG_HEADER *msg);

    static void CloseCallback(void *context, void *data);

    void NotifyHandler(HANDLE idProcess);

    MSG_HEADER *OpenHandler(MSG_HEADER *msg, HANDLE idProcess);

    MSG_HEADER *CloseHandler(MSG_HEADER *msg, HANDLE idProcess);

    MSG_HEADER *SetHandler(MSG_HEADER *msg, HANDLE idProcess);

    MSG_HEADER *ReadHandler(MSG_HEADER *msg, HANDLE idProcess);

    MSG_HEADER *WriteHandler(MSG_HEADER *msg, HANDLE idProcess);

    MSG_HEADER *LpcConnectHandler(MSG_HEADER *msg, HANDLE idProcess);

    MSG_HEADER *LpcRequestHandler(MSG_HEADER *msg, HANDLE idProcess);

    MSG_HEADER *AlpcRequestHandler(MSG_HEADER *msg, HANDLE idProcess);

protected:

    ProxyHandle *m_ProxyHandle;

    void *m_pNtAlpcConnectPort;
    void *m_pNtAlpcSendWaitReceivePort;
    VOID *m_pNtAlpcImpersonateClientOfPort;

};


#endif /* _MY_NAMEDPIPESERVER_H */
