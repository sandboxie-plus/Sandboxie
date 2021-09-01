/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2021 David Xanatos, xanasoft.com
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


#include "ipc.h"
#include "obj.h"
#include "api.h"
#include "thread.h"
#include "session.h"


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


#define PORT_TYPE                           0x0000000F
#define SERVER_CONNECTION_PORT              0x00000001
#define UNCONNECTED_COMMUNICATION_PORT      0x00000002
#define SERVER_COMMUNICATION_PORT           0x00000003
#define CLIENT_COMMUNICATION_PORT           0x00000004
#define PORT_WAITABLE                       0x20000000
#define PORT_NAME_DELETED                   0x40000000
#define PORT_DYNAMIC_SECURITY               0x80000000
#define PORT_DELETED                        0x10000000


struct LPC_PORT_OBJECT_2K {
    ULONG Length;
    ULONG Flags;
    struct _LPCP_PORT_OBJECT *ConnectionPort;
    // ...  more
};


struct LPC_PORT_OBJECT_XP_2003 {
    struct _LPCP_PORT_OBJECT *ConnectionPort;
    // ...  more
};


struct ALPC_PORT_OBJECT_VISTA {
    ULONG_PTR unknown1;
    ULONG_PTR unknown2;
    struct _LPCP_PORT_OBJECT **ConnectionPortPtr;
    // ...  more
};


#define KERNEL_CHECKVDM                     0x00010005
#define KERNEL_DEFINEDOSDEVICE              0x00010017
#define KERNEL_DEFINEDOSDEVICE_VISTA        0x00010014
#define WINAPI_EXITWINDOWSEX                0x00030400
#define WINAPI_ENDTASK                      0x00030401
#define WINAPI_SRVDEVICEEVENT               0x00030406
#define WINAPI_SRVDEVICEEVENT_WIN7          0x00030404


typedef struct _WINAPI_MESSAGE {

    PORT_MESSAGE port_msg;
    void *unknown;
    ULONG api_code;

} WINAPI_MESSAGE;

typedef struct _POWER_API_MESSAGE
{
    PORT_MESSAGE port_msg;

    ULONG start1;           // Only seen this as 0 or 1 - A 1 indicates the start of an operation
    ULONG start2;
    ULONG start3;

    ULONG seq_number;       // Sequential number incremented after the starting operation message (1..n)

    ULONG unknown1;

    ULONG op_type;

    ULONG unknown2;
    ULONG unknown3;

    //
    // What remains is unknown - Perhaps a bit more of the header and then
    // the remaining data.  I've seen GUIDs in certain messages.
    //
} POWER_API_MESSAGE;

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


NTSTATUS Ipc_CheckPortRequest(
    PROCESS *proc, HANDLE PortHandle, PORT_MESSAGE *msg);

NTSTATUS Ipc_CheckPortRequest_WinApi(
    PROCESS *proc, OBJECT_NAME_INFORMATION *Name, PORT_MESSAGE *msg);

NTSTATUS Ipc_CheckPortRequest_Lsa(
    PROCESS *proc, OBJECT_NAME_INFORMATION *Name, PORT_MESSAGE *msg);

NTSTATUS Ipc_CheckPortRequest_LsaEP(
    PROCESS* proc, OBJECT_NAME_INFORMATION* Name, PORT_MESSAGE* msg);

NTSTATUS Ipc_CheckPortRequest_Sam(
    PROCESS* proc, OBJECT_NAME_INFORMATION* Name, PORT_MESSAGE* msg);

NTSTATUS Ipc_CheckPortRequest_PowerManagement(
    PROCESS *proc, OBJECT_NAME_INFORMATION *Name, PORT_MESSAGE *msg);

NTSTATUS Ipc_CheckPortRequest_SpoolerPort(
    PROCESS *proc, OBJECT_NAME_INFORMATION *Name, PORT_MESSAGE *msg);


static NTSTATUS Ipc_Api_GetRpcPortName_2(
    PEPROCESS ProcessObject, WCHAR* pDstPortName);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


IPC_DYNAMIC_PORTS Ipc_Dynamic_Ports;

static const WCHAR* _rpc_control = L"\\RPC Control";


//---------------------------------------------------------------------------
// Ipc_GetServerPort
//---------------------------------------------------------------------------


_FX void *Ipc_GetServerPort(void *Object)
{
    void *port_object = NULL;

    //
    // when a server process creates a port, we get a PORT_OBJECT
    //    with a ConnectionPort member that points to itself.
    //
    // when a client process connects to a port, we get a PORT_OBJECT
    //    with a ConnectionPort member that points to the server port.
    // *  NtConnectPort (in the client process) references the server
    //    port object.  dereference is done only when client port is
    //    closed.  so we don't have to worry about the server port object
    //    (in ConnectionPort) disappearing while we're working with it.
    //

#ifdef XP_SUPPORT
    if (Driver_OsVersion == DRIVER_WINDOWS_XP ||
        Driver_OsVersion == DRIVER_WINDOWS_2003) {

        port_object =
            ((struct LPC_PORT_OBJECT_XP_2003 *)Object)->ConnectionPort;

    } else 
#endif
    if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA) {

        port_object =
            *(((struct ALPC_PORT_OBJECT_VISTA *)Object)->ConnectionPortPtr);
    }

    return port_object;
}


//---------------------------------------------------------------------------
// Ipc_CheckPortRequest
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_CheckPortRequest(
    PROCESS *proc, HANDLE PortHandle, PORT_MESSAGE *msg)
{
    NTSTATUS status;
    void *client_port_object;
    void *server_port_object;
    OBJECT_NAME_INFORMATION *Name;
    ULONG NameLength;

    if (! msg)
        return STATUS_SUCCESS;

    client_port_object = NULL;
    Name = NULL;

    //
    // get the name of the server port for this client connection port
    //

    status = ObReferenceObjectByHandle(
                PortHandle, 0, *LpcPortObjectType, UserMode,
                &client_port_object, NULL);
    if (! NT_SUCCESS(status))
        goto finish;

    server_port_object = Ipc_GetServerPort(client_port_object);
    if (! server_port_object) {
        status = STATUS_ACCESS_DENIED;
        goto finish;
    }

    status = Obj_GetName(proc->pool, server_port_object, &Name, &NameLength);
    if (! NT_SUCCESS(status))
        goto finish;
    if (Name == &Obj_Unnamed) {
        Name = NULL;
        goto finish;
    }

    //
    // examine port message and block specific api_codes
    // each checker returns STATUS_BAD_INITIAL_PC for a port object it
    // does not handle, otherwise STATUS_SUCCESS or STATUS_ACCESS_DENIED
    //

    status = Ipc_CheckPortRequest_WinApi(proc, Name, msg);
    if (status == STATUS_BAD_INITIAL_PC)
        status = Ipc_CheckPortRequest_Lsa(proc, Name, msg);
    if (status == STATUS_BAD_INITIAL_PC)
        status = Ipc_CheckPortRequest_LsaEP(proc, Name, msg);
    if (status == STATUS_BAD_INITIAL_PC)
        status = Ipc_CheckPortRequest_Sam(proc, Name, msg);
    if (status == STATUS_BAD_INITIAL_PC)
        status = Ipc_CheckPortRequest_PowerManagement(proc, Name, msg);
    if (status == STATUS_BAD_INITIAL_PC)
        status = Ipc_CheckPortRequest_SpoolerPort(proc, Name, msg);
    if (status == STATUS_BAD_INITIAL_PC)
        status = STATUS_SUCCESS;

    //if (! NT_SUCCESS(status)) {
    //if (SearchUnicodeString(Name, L"spool", FALSE))
        //DbgPrint("Status <%08X> on Port <%*.*S>\n", status, Name->Name.Length/sizeof(WCHAR), Name->Name.Length/sizeof(WCHAR), Name->Name.Buffer);
    //}

    /*if (Session_MonitorCount)// && (proc->ipc_trace & (TRACE_ALLOW | TRACE_DENY))) 
    {
        WCHAR msg_str[256];
        RtlStringCbPrintfW(msg_str, sizeof(msg_str), L"CheckPortRequest, Status <%08X> on Port <%*.*s>\n", status, Name->Name.Length / sizeof(WCHAR), Name->Name.Length / sizeof(WCHAR), Name->Name.Buffer);
        Log_Debug_Msg(MONITOR_IPC, msg_str, NULL);
    }*/

    //
    // finish
    //

finish:

    if (Name)
        Mem_Free(Name, NameLength);
    if (client_port_object)
        ObDereferenceObject(client_port_object);

    return status;
}


//---------------------------------------------------------------------------
// Ipc_DisplayPowerMsg
//---------------------------------------------------------------------------

void Ipc_DisplayPowerMsg(POWER_API_MESSAGE *pMsg)
{
#ifdef DISPLAY_POWER_MSG_DATA

    //
    // Used to help reverse engineer the messaging - Probably a good idea
    // to keep around for future support.
    //

    int i;
    char szBuffer[1024];

    DbgPrint(" \n");
    DbgPrint("DataLength = [%d]  -  TotalLength = [%d]\n", pMsg->port_msg.u1.s1.DataLength, pMsg->port_msg.u1.s1.TotalLength);
    DbgPrint("Type = [%d]  -  DataInfoOffset = [%d]\n", pMsg->port_msg.u2.s2.Type, pMsg->port_msg.u2.s2.DataInfoOffset);
    DbgPrint("MessageID = [%ld]\n", pMsg->port_msg.MessageId);

    if (pMsg->port_msg.u1.s1.DataLength < (1024 / 3))
    {
        USHORT uDataLength = pMsg->port_msg.u1.s1.DataLength;
        unsigned char* pData = (unsigned char*)pMsg + sizeof(PORT_MESSAGE);

        for (i = 0; i < uDataLength; i++)
        {
            sprintf(szBuffer + (i * 3), " %02x", pData[i]);
        }

        DbgPrint("Buffer: [%s]\n", szBuffer);
    }
    else
    {
        DbgPrint("*** Error:  DisplayPowerMsg() szBuffer is too small! ***\n");
    }

    DbgPrint("Start1 = [0x%02x]\n", pMsg->start1);
    DbgPrint("Start2 = [0x%02x]\n", pMsg->start2);
    DbgPrint("Start3 = [0x%02x]\n", pMsg->start3);
    DbgPrint("SequenceNumber = [0x%02x]\n", pMsg->seq_number);
    DbgPrint("Unknown1 = [0x%02x]\n", pMsg->unknown1);
    DbgPrint("OpType = [0x%02x]\n", pMsg->op_type);
    DbgPrint("Unknown2 = [0x%02x]\n", pMsg->unknown2);
    DbgPrint("Unknown3 = [0x%02x]\n", pMsg->unknown3);

#endif
}

//---------------------------------------------------------------------------
// Ipc_ShouldAllowPowerOperation
//---------------------------------------------------------------------------

NTSTATUS Ipc_ShouldAllowPowerOperation(POWER_API_MESSAGE *pPowerMsg)
{
    // White-list approach -> Default to Access Denied
    NTSTATUS Status = STATUS_ACCESS_DENIED;

    if (!pPowerMsg)
    {
        return Status;
    }

    if (pPowerMsg->start1 == 1 && pPowerMsg->port_msg.u2.s2.Type == 16384)
    {
        // Start of Power Operation -> Allow
        Status = STATUS_SUCCESS;
        // DbgPrint("Start of Power Operation...\n");
    }
    else if (Driver_OsVersion <= DRIVER_WINDOWS_7)
    {
        //
        // Allow read-only operations.  It's unknown what exactly the allowed operations below pertain to,
        // otherwise #define's would have be used to be more descriptive.
        //

        switch (pPowerMsg->op_type)
        {
        case 0x00:
        case 0x01:
        case 0x09:
        case 0x13:
        {
            Status = STATUS_SUCCESS;
        }
        break;
        }
    }
    else
    {
        // Windows 8 and above

        switch (pPowerMsg->op_type)
        {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x0a:
        case 0x12:
        {
            Status = STATUS_SUCCESS;
        }
        break;
        }
    }

    /*
    if (Status == STATUS_ACCESS_DENIED)
    {
        DbgPrint("Denying Access to Operation [0x%02x]\n", pPowerMsg->op_type);
    }
    */

    return Status;
}

//---------------------------------------------------------------------------
// Ipc_CheckPortRequest_PowerManagement
//---------------------------------------------------------------------------

_FX NTSTATUS Ipc_CheckPortRequest_PowerManagement(
    PROCESS *proc, OBJECT_NAME_INFORMATION *Name, PORT_MESSAGE *msg)
{
    NTSTATUS Status;
    UNICODE_STRING usPowerPort;

    RtlInitUnicodeString(&usPowerPort, L"\\RPC Control\\umpo");

    if (RtlCompareUnicodeString(&usPowerPort, &Name->Name, FALSE) != 0)
    {
        return STATUS_BAD_INITIAL_PC;
    }

    Status = STATUS_ACCESS_DENIED;

    __try
    {
        ProbeForRead(msg, sizeof(PORT_MESSAGE), sizeof(ULONG_PTR));

        if (msg->u1.s1.TotalLength >= sizeof(POWER_API_MESSAGE))
        {
            POWER_API_MESSAGE *pPowerMsg = (POWER_API_MESSAGE *)msg;

            ProbeForRead(pPowerMsg, sizeof(POWER_API_MESSAGE), sizeof(ULONG_PTR));

            Ipc_DisplayPowerMsg(pPowerMsg);

            Status = Ipc_ShouldAllowPowerOperation(pPowerMsg);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Status = GetExceptionCode();
    }

    return Status;
}


//---------------------------------------------------------------------------
// Ipc_CheckPortRequest_WinApi
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_CheckPortRequest_WinApi(
    PROCESS *proc, OBJECT_NAME_INFORMATION *Name, PORT_MESSAGE *msg)
{
    static const WCHAR *_Windows_ApiPort = L"\\Windows\\ApiPort";
    NTSTATUS status;
    WCHAR *name_ptr;

    //
    // check that it is \Windows\ApiPort or \Sessions\*\Windows\ApiPort
    //

    if (Name->Name.Length < 16 * sizeof(WCHAR))
        return STATUS_BAD_INITIAL_PC;

    name_ptr = Name->Name.Buffer
             + Name->Name.Length / sizeof(WCHAR)
             - 16;
    if (_wcsicmp(name_ptr, _Windows_ApiPort) != 0)
        return STATUS_BAD_INITIAL_PC;

    if (Name->Name.Length > 16 * sizeof(WCHAR)) {
        if (_wcsnicmp(Name->Name.Buffer, L"\\Sessions\\", 10) != 0)
            return STATUS_BAD_INITIAL_PC;
    }

    //
    // examine message
    //

    status = STATUS_SUCCESS;

    __try {

        ProbeForRead(msg, sizeof(PORT_MESSAGE), sizeof(ULONG_PTR));

        if (msg->u1.s1.TotalLength >= sizeof(WINAPI_MESSAGE)) {

            WINAPI_MESSAGE *msg2 = (WINAPI_MESSAGE *)msg;

            ProbeForRead(msg2, sizeof(WINAPI_MESSAGE), sizeof(ULONG_PTR));

            if (msg2->api_code == WINAPI_EXITWINDOWSEX ||
                msg2->api_code == WINAPI_ENDTASK) {

                status = STATUS_ACCESS_DENIED;
            }

            if (msg2->api_code == KERNEL_CHECKVDM) {
                Log_MsgP0(MSG_BLOCKED_16_BIT, proc->pid);
                status = STATUS_ACCESS_DENIED;
            }

            if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA) {
                if (msg2->api_code == KERNEL_DEFINEDOSDEVICE_VISTA)
                    status = STATUS_ACCESS_DENIED;
            } else {
                if (msg2->api_code == KERNEL_DEFINEDOSDEVICE)
                    status = STATUS_ACCESS_DENIED;
            }

            // MS11-063
            if ( 
#ifdef XP_SUPPORT
                ((Driver_OsVersion == DRIVER_WINDOWS_XP || Driver_OsVersion == DRIVER_WINDOWS_VISTA) && msg2->api_code == WINAPI_SRVDEVICEEVENT) ||
#endif
                 (Driver_OsVersion == DRIVER_WINDOWS_7 && msg2->api_code == WINAPI_SRVDEVICEEVENT_WIN7) ) {

                Log_MsgP0(MSG_1316, proc->pid);
                status = STATUS_ACCESS_DENIED;
            }
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    return status;
}


//---------------------------------------------------------------------------
// Ipc_ImpersonatePort
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_ImpersonatePort(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args)
{
    NTSTATUS status = Syscall_Invoke(syscall_entry, user_args);

    if (NT_SUCCESS(status) && proc->primary_token) {

        status = Thread_StoreThreadToken(proc);
    }

    return status;
}


//---------------------------------------------------------------------------
// Ipc_RequestPort
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_RequestPort(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args)
{
    HANDLE PortHandle = (HANDLE *)user_args[0];
    PORT_MESSAGE *Message = (PORT_MESSAGE *)user_args[1];

    NTSTATUS status = Ipc_CheckPortRequest(proc, PortHandle, Message);

    if (NT_SUCCESS(status)) {

        status = Syscall_Invoke(syscall_entry, user_args);
    }

    return status;
}


//---------------------------------------------------------------------------
// Ipc_AlpcSendWaitReceivePort
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_AlpcSendWaitReceivePort(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args)
{
    HANDLE PortHandle = (HANDLE *)user_args[0];
    PORT_MESSAGE *Message = (PORT_MESSAGE *)user_args[2];

    NTSTATUS status = Ipc_CheckPortRequest(proc, PortHandle, Message);

    if (NT_SUCCESS(status)) {

        status = Syscall_Invoke(syscall_entry, user_args);
    }

    return status;
}


//---------------------------------------------------------------------------
// Ipc_Api_OpenDynamicPort
//---------------------------------------------------------------------------

// Param 1 is dynamic port name (e.g. "\RPC Control\LRPC-f760d5b40689a98168"), WCHAR[DYNAMIC_PORT_NAME_CHARS]
// Param 2 is the process PID for which to open the port, can be 0 when port is special
// Param 3 is the port type/identifier

_FX NTSTATUS Ipc_Api_OpenDynamicPort(PROCESS* proc, ULONG64* parms)
{
    NTSTATUS status = STATUS_SUCCESS;
    KIRQL irql;
    API_OPEN_DYNAMIC_PORT_ARGS* pArgs = (API_OPEN_DYNAMIC_PORT_ARGS*)parms;
    WCHAR portName[DYNAMIC_PORT_NAME_CHARS];
    WCHAR portId[DYNAMIC_PORT_ID_CHARS];

    if (proc) // is caller sandboxed?
        return STATUS_NOT_IMPLEMENTED;

    if (PsGetCurrentProcessId() != Api_ServiceProcessId)
        return STATUS_ACCESS_DENIED;

    if (pArgs->port_name.val == NULL)
        return STATUS_INVALID_PARAMETER;
    try {
        ProbeForRead(pArgs->port_name.val, sizeof(WCHAR) * DYNAMIC_PORT_NAME_CHARS, sizeof(WCHAR));
        wmemcpy(portName, pArgs->port_name.val, DYNAMIC_PORT_NAME_CHARS - 1);
        portName[DYNAMIC_PORT_NAME_CHARS - 1] = L'\0';

        if (pArgs->port_id.val == NULL)
            __leave;
        ProbeForRead(pArgs->port_id.val, sizeof(WCHAR) * DYNAMIC_PORT_ID_CHARS, sizeof(WCHAR));
        wmemcpy(portId, pArgs->port_id.val, DYNAMIC_PORT_ID_CHARS - 1);
        portId[DYNAMIC_PORT_ID_CHARS - 1] = L'\0';
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }
    if (!NT_SUCCESS(status))
        return status;

    //
    // When this is a special port save it our global Ipc_Dynamic_Ports structure
    //

    if (pArgs->port_id.val != NULL && Ipc_Dynamic_Ports.pPortLock)
    {
        KeEnterCriticalRegion();
        ExAcquireResourceExclusiveLite(Ipc_Dynamic_Ports.pPortLock, TRUE);

        IPC_DYNAMIC_PORT* port = List_Head(&Ipc_Dynamic_Ports.Ports);
        while (port) 
        {    
            if (_wcsicmp(portId, port->wstrPortId) == 0)
            {
                wmemcpy(port->wstrPortName, portName, DYNAMIC_PORT_NAME_CHARS);
                break;
            }

            port = List_Next(port);
        }

        if (port == NULL) 
        {
            port = Mem_AllocEx(Driver_Pool, sizeof(IPC_DYNAMIC_PORT), TRUE);
            if (!port)
                Log_Msg0(MSG_1104);
            else
            {
                wmemcpy(port->wstrPortId, portId, DYNAMIC_PORT_ID_CHARS);
                wmemcpy(port->wstrPortName, portName, DYNAMIC_PORT_NAME_CHARS);

                if (_wcsicmp(port->wstrPortId, L"spooler") == 0)
                    Ipc_Dynamic_Ports.pSpoolerPort = port;

                List_Insert_After(&Ipc_Dynamic_Ports.Ports, NULL, port);
            }
        }

        ExReleaseResourceLite(Ipc_Dynamic_Ports.pPortLock);
        KeLeaveCriticalRegion();
    }

    //
    // Open the port for the selected process
    //

    if (pArgs->process_id.val != 0)
    {
        proc = Process_Find(pArgs->process_id.val, &irql);
        if (proc && (proc != PROCESS_TERMINATED))
        {
            ExAcquireResourceExclusiveLite(proc->ipc_lock, TRUE);

            Process_AddPath(proc, &proc->open_ipc_paths, NULL, FALSE, portName, FALSE);

            ExReleaseResourceLite(proc->ipc_lock);
        }
        else
            status = STATUS_NOT_FOUND;
        ExReleaseResourceLite(Process_ListLock);
        KeLowerIrql(irql);
    }

    return status;
}


//---------------------------------------------------------------------------
// Ipc_Api_GetDynamicPortFromPid
//---------------------------------------------------------------------------

// Param 1 is the service PID
// Param 2 will return the port name with "\RPC Control\" prepended

_FX NTSTATUS Ipc_Api_GetDynamicPortFromPid(PROCESS* proc, ULONG64* parms)
{
    NTSTATUS status;
    PEPROCESS ProcessObject;
    //BOOLEAN done = FALSE;
    API_GET_DYNAMIC_PORT_FROM_PID_ARGS* pArgs = (API_GET_DYNAMIC_PORT_FROM_PID_ARGS*)parms;

    if (proc) // is caller sandboxed?
        return STATUS_ACCESS_DENIED;

    //
    // this function determines the dynamic RPC endpoint that is used by a service/process
    //

    status = PsLookupProcessByProcessId(pArgs->process_id.val, &ProcessObject);

    if (NT_SUCCESS(status)) {

        //if (PsGetProcessSessionId(ProcessObject) == 0) {
        //
        //    void *nbuf;
        //    ULONG nlen;
        //    WCHAR *nptr;
        //
        //    Process_GetProcessName(
        //        Driver_Pool, (ULONG_PTR)pArgs->process_id.val, &nbuf, &nlen, &nptr);
        //
        //    if (nbuf) {
        //
        //        if (_wcsicmp(nptr, pArgs->exe_name.val) == 0
        //            && MyIsProcessRunningAsSystemAccount(pArgs->process_id.val)) {

        status = Ipc_Api_GetRpcPortName_2(ProcessObject, pArgs->full_port_name.val);

        //            done = TRUE;
        //        }
        //
        //        Mem_Free(nbuf, nlen);
        //    }
        //}

        ObDereferenceObject(ProcessObject);
    }

    return status;
}


//---------------------------------------------------------------------------
// Ipc_Api_GetRpcPortName_2
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_Api_GetRpcPortName_2(PEPROCESS ProcessObject, WCHAR* pDstPortName)
{
    NTSTATUS status;
    ULONG len, dummy_len;
    ULONG context;
    HANDLE handle;
    OBJECT_DIRECTORY_INFORMATION* info;
    void* buf, * PortObject;
    UNICODE_STRING objname;
    OBJECT_ATTRIBUTES objattrs;
    WCHAR name[DYNAMIC_PORT_NAME_CHARS];

    InitializeObjectAttributes(&objattrs,
        &objname, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

    RtlInitUnicodeString(&objname, _rpc_control);

    status = ZwOpenDirectoryObject(&handle, 0, &objattrs);

    if (!NT_SUCCESS(status))
        return status;

    //
    // get a list of all objects in the system
    //

    len = 0;
    while (1) {

        len += PAGE_SIZE * 2;
        buf = ExAllocatePoolWithTag(PagedPool, len, tzuk);
        if (!buf) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        dummy_len = 0;
        status = ZwQueryDirectoryObject(
            handle, buf, len, FALSE, TRUE, &context, &dummy_len);

        if (status == STATUS_MORE_ENTRIES || status == STATUS_INFO_LENGTH_MISMATCH)
        {
            ExFreePoolWithTag(buf, tzuk);
            continue;
        }

        break;
    }

    if (!NT_SUCCESS(status))
        return status;

    //
    // go through list looking for LRPC-* objects of type ALPC Port
    //

    info = (OBJECT_DIRECTORY_INFORMATION*)buf;
    while (1) {

        UNICODE_STRING* ObjName = &info->ObjectName;
        UNICODE_STRING* TypeName = &info->ObjectTypeName;

        if ((!ObjName->Buffer) && (!TypeName->Buffer))
            break;

        if (TypeName->Length == 9 * sizeof(WCHAR) && TypeName->Buffer
            && _wcsicmp(TypeName->Buffer, L"ALPC Port") == 0) {

            if ((ObjName->Length > 5 * sizeof(WCHAR)) &&
                (ObjName->Length < 64 * sizeof(WCHAR)) &&
                _wcsnicmp(ObjName->Buffer, L"LRPC-", 5) == 0) {

                RtlStringCbPrintfW(name, sizeof(name), L"%s\\%s", _rpc_control, ObjName->Buffer);

                RtlInitUnicodeString(&objname, name);

                status = ObReferenceObjectByName(
                    &objname, OBJ_CASE_INSENSITIVE, NULL, 0,
                    *LpcPortObjectType, KernelMode, NULL,
                    &PortObject);

                if (NT_SUCCESS(status)) {

                    //
                    // make sure the owner process for the LRPC-* port
                    // is the process that was specified as a parameter
                    //

                    struct {
                        LIST_ENTRY PortListEntry;
                        void* CommunicationInfo;
                        PEPROCESS OwnerProcess;
                    } *AlpcPortObject = PortObject;

                    if (AlpcPortObject->OwnerProcess == ProcessObject) {

                        __try {

                            if (pDstPortName)
                            {
                                ProbeForWrite(pDstPortName, sizeof(WCHAR) * DYNAMIC_PORT_NAME_CHARS, sizeof(WCHAR));
                                wmemcpy(pDstPortName, name, DYNAMIC_PORT_NAME_CHARS - 1);
                                pDstPortName[DYNAMIC_PORT_NAME_CHARS - 1] = L'\0';
                            }

                        }
                        __except (EXCEPTION_EXECUTE_HANDLER) {
                            status = GetExceptionCode();
                        }

                        ObDereferenceObject(PortObject);
                        break;
                    }

                    ObDereferenceObject(PortObject);
                }
            }
        }

        ++info;
    }

    //
    // release storage
    //

    ExFreePoolWithTag(buf, tzuk);

    ZwClose(handle);

    return status;
}


#ifdef XP_SUPPORT

//---------------------------------------------------------------------------
//
// 32-bit hooks for Windows XP
//
// These hooks protect use of NtRequestPort and NtRequestWaitReplyPort
// in case a malicious program tries to bypass user mode hooks on EndTask
// or invokes
//
//
//---------------------------------------------------------------------------


#ifndef _WIN64


//---------------------------------------------------------------------------
// IPC_PORT_HEADER
//---------------------------------------------------------------------------


#define IPC_PORT_HEADER(Message)                                            \
    NTSTATUS status;                                                        \
    PROCESS *proc = Process_GetCurrent();                                   \
    if (proc != PROCESS_TERMINATED)                                         \
        status = Ipc_CheckPortRequest(proc, PortHandle, Message);           \
    else                                                                    \
        status = STATUS_PROCESS_IS_TERMINATING;


//---------------------------------------------------------------------------
// Ipc_NtRequestPort
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtRequestPort(
    HANDLE PortHandle, void *RequestMessage)
{
    IPC_PORT_HEADER(RequestMessage);
    if (NT_SUCCESS(status))
        status = __sys_NtRequestPort(PortHandle, RequestMessage);
    return status;
}


//---------------------------------------------------------------------------
// Ipc_NtRequestWaitReplyPort
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtRequestWaitReplyPort(
    HANDLE PortHandle, void *RequestMessage, void *ReplyMessage)
{
    IPC_PORT_HEADER(RequestMessage);
    if (NT_SUCCESS(status))
        status = __sys_NtRequestWaitReplyPort(
                    PortHandle, RequestMessage, ReplyMessage);
    return status;
}


#endif _WIN64
#endif