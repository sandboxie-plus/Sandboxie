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
// Protected Storage Server -- using PipeServer
//---------------------------------------------------------------------------

#ifndef _MY_PSTORESERVER_H
#define _MY_PSTORESERVER_H


#include "PipeServer.h"


class PStoreServer
{

public:

    PStoreServer(PipeServer *pipeServer);

protected:

    static MSG_HEADER *Handler(void *_this, MSG_HEADER *msg);

    MSG_HEADER *GetTypeInfo(MSG_HEADER *msg);

    MSG_HEADER *GetSubtypeInfo(MSG_HEADER *msg);

    MSG_HEADER *ReadItem(MSG_HEADER *msg);

    MSG_HEADER *EnumTypes(MSG_HEADER *msg);

    MSG_HEADER *EnumItems(MSG_HEADER *msg);

    static DWORD connectToPStore(void *__this);

protected:

    void *m_pStore;

};


#endif /* _MY_PSTORESERVER_H */
