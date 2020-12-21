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


#include "ipc.h"
#include "obj.h"
#include "api.h"
#include "thread.h"


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


typedef struct _LSA_MESSAGE_XP {

    PORT_MESSAGE port_msg;
    ULONG api_code;
    ULONG status;
    ULONG auth_pkg_code;
    ULONG *buf;
    ULONG buf_len;

} LSA_MESSAGE_XP;

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


static NTSTATUS Ipc_CheckPortRequest(
    PROCESS *proc, HANDLE PortHandle, PORT_MESSAGE *msg);

static NTSTATUS Ipc_CheckPortRequest_WinApi(
    PROCESS *proc, OBJECT_NAME_INFORMATION *Name, PORT_MESSAGE *msg);

static NTSTATUS Ipc_CheckPortRequest_Lsa(
    PROCESS *proc, OBJECT_NAME_INFORMATION *Name, PORT_MESSAGE *msg);

static NTSTATUS Ipc_CheckPortRequest_PowerManagement(
    PROCESS *proc, OBJECT_NAME_INFORMATION *Name, PORT_MESSAGE *msg);

//static NTSTATUS Ipc_CheckPortRequest_SpoolerPort(
//  PROCESS *proc, OBJECT_NAME_INFORMATION *Name, PORT_MESSAGE *msg);



//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static ULONG Ipc_MSV10_AuthPackageNumber = 0;


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

    if (Driver_OsVersion == DRIVER_WINDOWS_XP ||
        Driver_OsVersion == DRIVER_WINDOWS_2003) {

        port_object =
            ((struct LPC_PORT_OBJECT_XP_2003 *)Object)->ConnectionPort;

    } else if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA) {

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
        status = Ipc_CheckPortRequest_PowerManagement(proc, Name, msg);
    //if (status == STATUS_BAD_INITIAL_PC)
        //status = Ipc_CheckPortRequest_SpoolerPort(proc, Name, msg);
    if (status == STATUS_BAD_INITIAL_PC)
        status = STATUS_SUCCESS;

    //if (! NT_SUCCESS(status)) {
    //if (SearchUnicodeString(Name, L"spool", FALSE))
        //DbgPrint("Status <%08X> on Port <%*.*S>\n", status, Name->Name.Length/sizeof(WCHAR), Name->Name.Length/sizeof(WCHAR), Name->Name.Buffer);
    //}

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
// Ipc_CheckPortRequest_SpoolerPort
//---------------------------------------------------------------------------

// This routine is currently not used.  We chose to block spooler CreateFile in the minifilter instead.  But I (Curt) am keeping this code
// around because it demonstrates how to examine & filter RPC requests going to the spooler.
/*
_FX NTSTATUS Ipc_CheckPortRequest_SpoolerPort(
    PROCESS *proc, OBJECT_NAME_INFORMATION *Name, PORT_MESSAGE *msg)
{
    NTSTATUS status;

    if (! proc->ipc_block_password)
        return STATUS_BAD_INITIAL_PC;

    //
    // check that it is the spooler port
    //

    if (Driver_OsVersion >= DRIVER_WINDOWS_81) {
        if (Name->Name.Length < 13 * sizeof(WCHAR))
            return STATUS_BAD_INITIAL_PC;

        if (_wcsicmp(Name->Name.Buffer + 13, Ipc_SpoolerPort) != 0)
            return STATUS_BAD_INITIAL_PC;
    }
    else if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA) {
        if (_wcsicmp(Name->Name.Buffer, L"\\RPC Control\\spoolss") != 0)
            return STATUS_BAD_INITIAL_PC;

    } else
        return STATUS_BAD_INITIAL_PC;

    //
    // examine message
    //

    if (proc->m_boolAllowSpoolerPrintToFile)        // see if we are allowing print to file
        return STATUS_BAD_INITIAL_PC;

    status = STATUS_SUCCESS;

    __try {

        ProbeForRead(msg, sizeof(PORT_MESSAGE), sizeof(ULONG_PTR));

        if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA) {

            //
            //

            ULONG  len = msg->u1.s1.DataLength;
            UCHAR *ptr = (UCHAR *)((UCHAR *)msg + sizeof(PORT_MESSAGE));
            int i = 0;
            int rc = -2;

            ProbeForRead(ptr, len, sizeof(WCHAR));

            if (ptr[20] == 17) {        // RpcStartDocPrinter = Opnum 17
                status = STATUS_ACCESS_DENIED;
                //for (i = 20; i < len - 12; i++)
                //{
                //  rc = memcmp((void*)&(ptr[i]), "\4\0\0\0\0\0\0\0\4\0\0\0\0", 12);    // search for marshaled "RAW" field length bytes
                //  if (rc == 0)
                //  {
                //      rc = _wcsnicmp((void*)&(ptr[i + 12]), L"raw", 3);       // search for case insensitive "RAW"
                //      if (rc == 0)
                //          status = STATUS_ACCESS_DENIED;
                //  }
                //}
            }

        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    if (status == STATUS_ACCESS_DENIED)
        Log_MsgP0(MSG_1319, proc->pid);

    return status;
}
*/

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
            if ( ((Driver_OsVersion == DRIVER_WINDOWS_XP || Driver_OsVersion == DRIVER_WINDOWS_VISTA) && msg2->api_code == WINAPI_SRVDEVICEEVENT) ||
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
// Ipc_CheckPortRequest_Lsa
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_CheckPortRequest_Lsa(
    PROCESS *proc, OBJECT_NAME_INFORMATION *Name, PORT_MESSAGE *msg)
{
    NTSTATUS status;

    if (! proc->ipc_block_password)
        return STATUS_BAD_INITIAL_PC;

    //
    // check that it is \LsaAuthenticationPort
    // or that it is \RPC Control\lsasspirpc (Windows 7 variant)
    //

    if (Name->Name.Length == 22 * sizeof(WCHAR)) {

        if (_wcsicmp(Name->Name.Buffer, L"\\LsaAuthenticationPort") != 0)
            return STATUS_BAD_INITIAL_PC;

    } else if (Name->Name.Length == 23 * sizeof(WCHAR)) {

        if (_wcsicmp(Name->Name.Buffer, L"\\RPC Control\\lsasspirpc") != 0)
            return STATUS_BAD_INITIAL_PC;

    } else
        return STATUS_BAD_INITIAL_PC;

    //
    // examine message
    //

    status = STATUS_SUCCESS;

    __try {

        ProbeForRead(msg, sizeof(PORT_MESSAGE), sizeof(ULONG_PTR));

        if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA) {

            //
            // in Windows Vista and Windows 7, a password change request
            // includes the WCHAR string Negotiate immediately followed
            // by a non-zero WCHAR
            //

            WCHAR *ptr = (WCHAR *)((UCHAR *)msg + sizeof(PORT_MESSAGE));
            ULONG  len = msg->u1.s1.DataLength;

            ProbeForRead(ptr, len, sizeof(WCHAR));
            len /= sizeof(WCHAR);

            while (len > 9 + 1) {

                if (ptr[0] == L'N' && ptr[9] != 0
                        && wmemcmp(ptr, L"Negotiate", 9) == 0) {

                    status = STATUS_ACCESS_DENIED;
                    break;
                }

                ++ptr;
                --len;
            }

        } else {

            //
            // prior to Windows Vista, we have a 'call package' api
            // call (value 2), which identifies the MSV10 auth package,
            // and a change password sub code (value 5)
            //

            if (msg->u1.s1.TotalLength >= sizeof(LSA_MESSAGE_XP)) {

                LSA_MESSAGE_XP *msg2 = (LSA_MESSAGE_XP *)msg;

                ProbeForRead(
                    msg2, sizeof(LSA_MESSAGE_XP), sizeof(ULONG_PTR));

                if (msg2->api_code == 2 &&  // LsaCallAuthenticationPackage
                    msg2->auth_pkg_code == Ipc_MSV10_AuthPackageNumber &&
                    msg2->buf_len >= sizeof(ULONG)) {

                    ULONG *buf = msg2->buf;
                    ProbeForRead(buf, sizeof(ULONG), sizeof(ULONG));

                    if (*buf == 5) {            // change password

                        status = STATUS_ACCESS_DENIED;
                    }
                }
            }
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    if (status == STATUS_ACCESS_DENIED)
		Log_Msg_Process(MSG_PASSWORD_CHANGE_DENIED, NULL, NULL, -1, proc->pid);

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
// Ipc_Api_SetLsaAuthPkg
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_Api_SetLsaAuthPkg(PROCESS *proc, ULONG64 *parms)
{
    //
    // caller must be our service process
    //

    if (proc || (PsGetCurrentProcessId() != Api_ServiceProcessId))
        return STATUS_ACCESS_DENIED;

    //
    // collect msv10 auth package number
    //

    if (Ipc_MSV10_AuthPackageNumber)
        return STATUS_ACCESS_DENIED;

    Ipc_MSV10_AuthPackageNumber = (ULONG)parms[1];
    return STATUS_SUCCESS;
}


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
