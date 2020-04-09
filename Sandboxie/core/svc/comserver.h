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
// COM Proxy Server -- using PipeServer
//---------------------------------------------------------------------------


#ifndef _MY_COMSERVER_H
#define _MY_COMSERVER_H


#include "PipeServer.h"


class ComServer
{

public:

    ComServer(PipeServer *pipeServer);

    void DeleteAllSlaves();

protected:

    static MSG_HEADER *Handler(void *_this, MSG_HEADER *msg);

    MSG_HEADER *GetClassObjectHandler(
        MSG_HEADER *msg, void *_slave, BOOLEAN *deleted);

    MSG_HEADER *CreateInstanceHandler(
        MSG_HEADER *msg, void *_slave, BOOLEAN *deleted);

    MSG_HEADER *QueryInterfaceHandler(
        MSG_HEADER *msg, void *_slave, BOOLEAN *deleted);

    MSG_HEADER *AddRefReleaseHandler(
        MSG_HEADER *msg, void *_slave, BOOLEAN *deleted);

    MSG_HEADER *InvokeMethodHandler(
        MSG_HEADER *msg, void *_slave, BOOLEAN *deleted);

    MSG_HEADER *UnmarshalInterfaceHandler(
        MSG_HEADER *msg, void *_slave, BOOLEAN *deleted);

    MSG_HEADER *MarshalInterfaceHandler(
        MSG_HEADER *msg, void *_slave, BOOLEAN *deleted);

    MSG_HEADER *QueryBlanketHandler(
        MSG_HEADER *msg, void *_slave, BOOLEAN *deleted);

    MSG_HEADER *SetBlanketHandler(
        MSG_HEADER *msg, void *_slave, BOOLEAN *deleted);

    MSG_HEADER *CopyProxyHandler(
        MSG_HEADER *msg, void *_slave, BOOLEAN *deleted);

    MSG_HEADER *NonComHandler(
        MSG_HEADER *msg, void *_slave, BOOLEAN *deleted);

    void *LockSlave(HANDLE idProcess, ULONG msgid);

    ULONG CallSlave(void *_slave, ULONG callid, BOOLEAN *deleted);

    void DeleteSlave(void *_slave);

    void NotifyAllSlaves(HANDLE idProcess);

    static void NotifyAllSlaves2(ULONG64 *ThreadData);

    void NotifyAllSlaves3(HANDLE idProcess);

    void LogErr(ULONG session, ULONG n1, ULONG n2);

    void LogErr(void *_slave, ULONG n1, ULONG n2);

    //
    // Slave Process
    //

public:

    static void RunSlave(const WCHAR *cmdline);

protected:

    static void *FindSlaveObject(void *_map, LIST *ObjectsList,
                                    ULONG *exc, HRESULT *hr);

    static void *RefOrAllocSlaveObject(ULONG idProcess, void *pUnknown,
                                       LIST *ObjectsList,
                                       ULONG *exc, HRESULT *hr);

    static void DeleteSlaveObject(void *_obj, LIST *ObjectsList);

    static void GetClassObjectSlave(void *_map, LIST *ObjectsList,
                                    ULONG *exc, HRESULT *hr);

    static void CreateInstanceSlave(void *_map, LIST *ObjectsList,
                                    ULONG *exc, HRESULT *hr);

    static void AddRefReleaseSlave(void *_map, LIST *ObjectsList,
                                   ULONG *exc, HRESULT *hr);

    static void QueryInterfaceSlave(void *_map, LIST *ObjectsList,
                                    ULONG *exc, HRESULT *hr);

    static void InvokeMethodSlave(void *_map, LIST *ObjectsList,
                                  ULONG *exc, HRESULT *hr);

    static void UnmarshalInterfaceSlave(void *_map, LIST *ObjectsList,
                                        ULONG *exc, HRESULT *hr);

    static void MarshalInterfaceSlave(void *_map, LIST *ProxiesList,
                                      ULONG *exc, HRESULT *hr);

    static void QueryBlanketSlave(void *_map, LIST *ProxiesList,
                                  ULONG *exc, HRESULT *hr);

    static void SetBlanketSlave(void *_map, LIST *ProxiesList,
                                ULONG *exc, HRESULT *hr);

    static void CopyProxySlave(void *_map, LIST *ProxiesList,
                               ULONG *exc, HRESULT *hr);

    static void ProcessNotifySlave(void *_map, LIST *ObjectsList,
                                   ULONG *exc, HRESULT *hr);

    static BOOLEAN CheckWmiExecMethod(UCHAR *BufPtr, ULONG BufLen);

    //
    // ComServer2
    //

    static void *FindOrCreateDummySlaveObject(void *_map, LIST *ObjectsList,
                                              ULONG *exc, HRESULT *hr);

    static void *CreateDummySlaveObject(GUID *riid);

    static ULONG CryptProtectDataSlave(void *Buffer);

    //
    // variables
    //

protected:

    CRITICAL_SECTION m_SlavesLock;
    LIST m_SlavesList;
    HANDLE m_SlaveReleasedEvent;

    static volatile LONG m_ObjIdx;

    static HANDLE m_heap;

    static bool m_AnySlaveObjectCreated;

};


#endif /* _MY_COMSERVER_H */
