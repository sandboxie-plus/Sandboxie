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
// IP Helper API Server -- using PipeServer
//---------------------------------------------------------------------------


#ifndef _MY_IPHLPSERVER_H
#define _MY_IPHLPSERVER_H


#include "PipeServer.h"
#include "ProxyHandle.h"


class IpHlpServer
{

public:

    IpHlpServer(PipeServer *pipeServer);

protected:

    static MSG_HEADER *Handler(void *_this, MSG_HEADER *msg);

    static void CloseCallback(void *context, void *data);

    MSG_HEADER *CreateHandler(MSG_HEADER *msg, HANDLE idProcess);

    MSG_HEADER *CloseHandler(MSG_HEADER *msg, HANDLE idProcess);

    MSG_HEADER *SendEchoHandler(MSG_HEADER *msg, HANDLE idProcess);

    void NotifyHandler(HANDLE idProcess);

protected:

    ProxyHandle *m_ProxyHandle;

    void *m_IcmpCreateFile;
    void *m_Icmp6CreateFile;
    void *m_IcmpCloseHandle;

    void *m_IcmpSendEcho2;
    void *m_IcmpSendEcho2Ex;
    void *m_Icmp6SendEcho2;

};


#endif /* _MY_IPHLPSERVER_H */
