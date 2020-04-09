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
// Terminal Server -- using PipeServer
//---------------------------------------------------------------------------


#ifndef _MY_TERMINALSERVER_H
#define _MY_TERMINALSERVER_H


#include "PipeServer.h"


class TerminalServer
{

public:

    TerminalServer(PipeServer *pipeServer);

protected:

    static MSG_HEADER *Handler(void *_this, MSG_HEADER *msg);

    MSG_HEADER *QueryInfo(MSG_HEADER *msg);

    MSG_HEADER *CheckType(MSG_HEADER *msg);

    MSG_HEADER *GetName(MSG_HEADER *msg);

    MSG_HEADER *GetProperty(MSG_HEADER *msg);

    MSG_HEADER *Disconnect(MSG_HEADER *msg);

protected:

    void *m_WinStaQueryInformation;
    void *m_WinStationIsSessionRemoteable;
    void *m_WinStationNameFromLogonId;
    void *m_WinStationGetConnectionProperty;
    void *m_WinStationFreePropertyValue;
    void *m_WinStationDisconnect;

};


#endif /* _MY_TERMINALSERVER_H */
