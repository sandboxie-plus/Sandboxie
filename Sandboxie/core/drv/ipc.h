/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC
 * Copyright 2020 David Xanatos, xanasoft.com 
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
// Inter-Process Communication
//---------------------------------------------------------------------------


#ifndef _MY_IPC_H
#define _MY_IPC_H


#include "driver.h"
#include "syscall.h"


typedef struct _IPC_DYNAMIC_PORT {
    LIST_ELEM list_elem;

    WCHAR       wstrPortId[DYNAMIC_PORT_ID_CHARS];
    WCHAR       wstrPortName[DYNAMIC_PORT_NAME_CHARS];
} IPC_DYNAMIC_PORT;

typedef struct _IPC_DYNAMIC_PORTS {
    PERESOURCE  pPortLock;
    
    LIST        Ports;

    IPC_DYNAMIC_PORT*  pSpoolerPort;
} IPC_DYNAMIC_PORTS;

extern IPC_DYNAMIC_PORTS Ipc_Dynamic_Ports;


//---------------------------------------------------------------------------
// Functions Prototypes
//---------------------------------------------------------------------------


typedef NTSTATUS (*P_NtRequestPort)(
    HANDLE PortHandle, void *RequestMessage);

typedef NTSTATUS (*P_NtRequestWaitReplyPort)(
    HANDLE PortHandle, void *RequestMessage, void *ReplyMessage);


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


BOOLEAN Ipc_Init(void);

void Ipc_Unload(void);

BOOLEAN Ipc_CreateBoxPath(PROCESS *proc);

void *Ipc_GetServerPort(void *Object);

BOOLEAN Ipc_InitProcess(PROCESS *proc);

BOOLEAN Ipc_IsRunRestricted(PROCESS *proc);


//---------------------------------------------------------------------------


NTSTATUS Ipc_ImpersonatePort(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args);

NTSTATUS Ipc_RequestPort(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args);

NTSTATUS Ipc_AlpcSendWaitReceivePort(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args);

NTSTATUS Ipc_NtRequestPort(
    HANDLE PortHandle, void *RequestMessage);

NTSTATUS Ipc_NtRequestWaitReplyPort(
    HANDLE PortHandle, void *RequestMessage, void *ReplyMessage);

NTSTATUS Ipc_Api_SetLsaAuthPkg(PROCESS *proc, ULONG64 *parms);

//NTSTATUS Ipc_Api_AllowSpoolerPrintToFile(PROCESS *proc, ULONG64 *parms);

NTSTATUS Ipc_Api_OpenDynamicPort(PROCESS* proc, ULONG64* parms);

NTSTATUS Ipc_Api_GetDynamicPortFromPid(PROCESS *proc, ULONG64 *parms);

//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


extern P_NtRequestPort              __sys_NtRequestPort;
extern P_NtRequestWaitReplyPort     __sys_NtRequestWaitReplyPort;


//---------------------------------------------------------------------------


#endif // _MY_IPC_H
