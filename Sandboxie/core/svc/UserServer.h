/*
 * Copyright 2022-2023 David Xanatos, xanasoft.com
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
// User Proxy Server
//---------------------------------------------------------------------------


#ifndef _MY_USERSERVER_H
#define _MY_USERSERVER_H


#include "common/list.h"


class UserServer
{

public:

    static UserServer *GetInstance();

    static void RunWorker(const WCHAR *cmdline);

    ULONG StartWorker(ULONG session_id);

    ULONG StartAsync(ULONG session_id, HANDLE hEvent);

protected:

    UserServer();

	~UserServer();

    static void ReportError2336(
                        ULONG session_id, ULONG errlvl, ULONG status);

protected:

    bool CreateQueueWorker(const WCHAR *cmdline);

    static void QueueCallbackWorker(void *arg, BOOLEAN timeout);

    bool QueueCallbackWorker2(void);

protected:

    struct WorkerArgs {
        ULONG pid;
        ULONG req_len;
        ULONG rpl_len;
        void *req_buf;
        void *rpl_buf;
    };
    typedef ULONG (UserServer::*WorkerFunc)(WorkerArgs *args);
    WorkerFunc *m_WorkerFuncs;

    ULONG OpenFile(WorkerArgs *args);

    ULONG OpenDocument(WorkerArgs *args);

    //
    // access check utilities
    //

    ULONG GetProcessPathList(ULONG path_code, 
        ULONG pid, void **out_pool, LIST **out_list);

    //bool CheckProcessPathList(LIST *list, const WCHAR *str);

    //
    // data
    //

protected:

    CRITICAL_SECTION m_WorkersLock;
    LIST m_WorkersList;
    HANDLE m_QueueEvent;

    WCHAR *m_QueueName;
    ULONG m_ParentPid;
    ULONG m_SessionId;

};


#endif /* _MY_USERSERVER_H */
