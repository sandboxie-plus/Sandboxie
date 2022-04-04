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
// Queue Server -- using PipeServer
//---------------------------------------------------------------------------


#ifndef _MY_QUEUESERVER_H
#define _MY_QUEUESERVER_H


#include "PipeServer.h"


class QueueServer
{

public:

    QueueServer(PipeServer *pipeServer);
	~QueueServer();

protected:

    static MSG_HEADER *Handler(void *_this, MSG_HEADER *msg);

    static void CloseCallback(void *context, void *data);

    MSG_HEADER *CreateHandler(MSG_HEADER *msg, HANDLE idProcess);

    MSG_HEADER *GetReqHandler(MSG_HEADER *msg, HANDLE idProcess);

    MSG_HEADER *PutRplHandler(MSG_HEADER *msg, HANDLE idProcess);

    MSG_HEADER *PutReqHandler(MSG_HEADER *msg, HANDLE idProcess);

    MSG_HEADER *GetRplHandler(MSG_HEADER *msg, HANDLE idProcess);

    void NotifyHandler(HANDLE idProcess);

    LONG OpenProcess(HANDLE idProcess, HANDLE *out_hProcess,
                     ULONG DesiredAccess = PROCESS_DUP_HANDLE);

    LONG DuplicateEvent(HANDLE hProcess, ULONG64 hEvent, HANDLE *out_hEvent);

    WCHAR *MakeQueueName(
                    HANDLE idProcess, WCHAR *req_name, ULONG *out_status);

    void *FindQueueObj(const WCHAR *QueueName);

    void DeleteQueueObj(void *_QueueObj);

    void DeleteRequestObj(LIST *RequestsList, void *_RequestObj);

protected:

    HANDLE m_heap;

    CRITICAL_SECTION m_lock;
    LIST m_queues;

    volatile LONG m_RequestId;
};


#endif /* _MY_QUEUESERVER_H */
