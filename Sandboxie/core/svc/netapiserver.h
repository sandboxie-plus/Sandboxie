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
// Net Api Services Server -- using PipeServer
//---------------------------------------------------------------------------


#ifndef _MY_NETAPISERVER_H
#define _MY_NETAPISERVER_H


#include "PipeServer.h"


class NetApiServer
{

public:

    NetApiServer(PipeServer *pipeServer);

    static void RunSlave(const WCHAR *cmdline);

protected:

    static MSG_HEADER *Handler(void *_this, MSG_HEADER *msg);

    MSG_HEADER *UseAdd(MSG_HEADER *msg);

    void LaunchSlave(ULONG len, const WCHAR *drive);

#if 0

    MSG_HEADER *WkstaGetInfo(MSG_HEADER *msg);

    MSG_HEADER *ServerGetInfo(MSG_HEADER *msg);

#endif

};


#endif /* _MY_NETAPISERVER_H */
