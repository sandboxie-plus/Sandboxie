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
// Inter-Process Communication
//---------------------------------------------------------------------------


#include "ipc.h"
#include "obj.h"
#include "api.h"
#include "util.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static NTSTATUS Ipc_Api_GetRpcPortName_2(enum ENUM_DYNAMIC_PORT_TYPE ePortType, PEPROCESS ProcessObject, WCHAR *pDstPortName);

static NTSTATUS Ipc_Api_CopyRpcPortName(enum ENUM_DYNAMIC_PORT_TYPE ePortType, WCHAR *pDstPortName, WCHAR *pSrcPortName);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------

IPC_DYNAMIC_PORTS Ipc_Dynamic_Ports[NUM_DYNAMIC_PORTS];

static const WCHAR *_rpc_control = L"\\RPC Control";

//---------------------------------------------------------------------------
// Ipc_Api_AllowSpoolerPrintToFile
//---------------------------------------------------------------------------

_FX NTSTATUS Ipc_Api_AllowSpoolerPrintToFile(PROCESS *proc, ULONG64 *parms)
{
    API_ALLOW_SPOOLER_PRINT_TO_FILE_ARGS *pArgs = (API_ALLOW_SPOOLER_PRINT_TO_FILE_ARGS *)parms;

    if (Process_Find(NULL, NULL)) {     // is caller sandboxed?
        return STATUS_ACCESS_DENIED;
    }

    if (pArgs->process_id.val > 0)
    {
        PROCESS *proc = Process_Find(pArgs->process_id.val, NULL);
        if (proc && proc != PROCESS_TERMINATED)
            proc->m_boolAllowSpoolerPrintToFile = TRUE;
    }
    return 0;
}


_FX NTSTATUS Ipc_Api_GetSpoolerPortFromPid(PROCESS *proc, ULONG64 *parms)
{
    return Ipc_Api_GetRpcPortFromPid(SPOOLER_PORT, proc, parms);
}


_FX NTSTATUS Ipc_Api_GetWpadPortFromPid(PROCESS *proc, ULONG64 *parms)
{
    return Ipc_Api_GetRpcPortFromPid(WPAD_PORT, proc, parms);
}

_FX NTSTATUS Ipc_Api_GetSmartCardPortFromPid(PROCESS *proc, ULONG64 *parms)
{
    return Ipc_Api_GetRpcPortFromPid(SMART_CARD_PORT, proc, parms);
}


// Param 1 is dynamic port name (e.g. "LRPC-f760d5b40689a98168")
// Param 2 will return the port name with "\RPC Control\" prepended

_FX NTSTATUS Ipc_Api_SetGameConfigStorePort(PROCESS *proc, ULONG64 *parms)
{
    WCHAR name[DYNAMIC_PORT_NAME_CHARS];
    API_SET_GAME_CONFIG_STORE_PORT_ARGS *pArgs = (API_SET_GAME_CONFIG_STORE_PORT_ARGS *)parms;

    swprintf(name, L"%s\\%s", _rpc_control, pArgs->port_name);

    return Ipc_Api_CopyRpcPortName(GAME_CONFIG_STORE_PORT, pArgs->full_port_name.val, name);
}


_FX NTSTATUS Ipc_Api_SetSmartCardPort(PROCESS *proc, ULONG64 *parms)
{
    WCHAR name[DYNAMIC_PORT_NAME_CHARS];
    API_SET_SMART_CARD_PORT_ARGS *pArgs = (API_SET_SMART_CARD_PORT_ARGS *)parms;

    swprintf(name, L"%s\\%s", _rpc_control, pArgs->port_name);

    return Ipc_Api_CopyRpcPortName(SMART_CARD_PORT, pArgs->full_port_name.val, name);
}



//---------------------------------------------------------------------------
// Ipc_Api_GetRpcPortNameFromPid
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_Api_GetRpcPortFromPid(enum ENUM_DYNAMIC_PORT_TYPE ePortType, PROCESS *proc, ULONG64 *parms)
{
    NTSTATUS status;
    PEPROCESS ProcessObject;
    BOOLEAN done = FALSE;
    API_GET_DYNAMIC_PORT_FROM_PID_ARGS *pArgs = (API_GET_DYNAMIC_PORT_FROM_PID_ARGS *)parms;

    //
    // this function determines the dynamic RPC endpoint that is used by a service/process
    //

    status = PsLookupProcessByProcessId(pArgs->process_id.val, &ProcessObject);

    if (NT_SUCCESS(status)) {

        if (PsGetProcessSessionId(ProcessObject) == 0) {

            void *nbuf;
            ULONG nlen;
            WCHAR *nptr;

            Process_GetProcessName(
                Driver_Pool, (ULONG_PTR)pArgs->process_id.val, &nbuf, &nlen, &nptr);

            if (nbuf) {

                if (_wcsicmp(nptr, pArgs->exe_name.val) == 0
                    && MyIsProcessRunningAsSystemAccount(pArgs->process_id.val)) {

                    status = Ipc_Api_GetRpcPortName_2(ePortType, ProcessObject, pArgs->port_name.val);

                    done = TRUE;
                }

                Mem_Free(nbuf, nlen);
            }
        }

        ObDereferenceObject(ProcessObject);
    }

    return status;
}


//---------------------------------------------------------------------------
// Ipc_Api_GetRpcPortName_2
//---------------------------------------------------------------------------

_FX NTSTATUS Ipc_Api_GetRpcPortName_2(enum ENUM_DYNAMIC_PORT_TYPE ePortType, PEPROCESS ProcessObject, WCHAR *pDstPortName)
{
    NTSTATUS status;
    ULONG len, dummy_len;
    ULONG context;
    HANDLE handle;
    OBJECT_DIRECTORY_INFORMATION *info;
    void *buf, *PortObject;
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
    // get a list of all processes in the system
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

    info = (OBJECT_DIRECTORY_INFORMATION *)buf;
    while (1) {

        UNICODE_STRING *ObjName = &info->ObjectName;
        UNICODE_STRING *TypeName = &info->ObjectTypeName;

        if ((!ObjName->Buffer) && (!TypeName->Buffer))
            break;

        if (TypeName->Length == 9 * sizeof(WCHAR) && TypeName->Buffer
            && _wcsicmp(TypeName->Buffer, L"ALPC Port") == 0) {

            if ((ObjName->Length > 5 * sizeof(WCHAR)) &&
                (ObjName->Length < 64 * sizeof(WCHAR)) &&
                _wcsnicmp(ObjName->Buffer, L"LRPC-", 5) == 0) {

                swprintf(name, L"%s\\%s", _rpc_control, ObjName->Buffer);

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
                        void *CommunicationInfo;
                        PEPROCESS OwnerProcess;
                    } *AlpcPortObject = PortObject;

                    if (AlpcPortObject->OwnerProcess == ProcessObject) {
                        Ipc_Api_CopyRpcPortName(ePortType, pDstPortName, name);
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


//---------------------------------------------------------------------------
// Ipc_Api_CopyRpcPortName
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_Api_CopyRpcPortName(enum ENUM_DYNAMIC_PORT_TYPE ePortType, WCHAR *pDstPortName, WCHAR *pSrcPortName)
{
    NTSTATUS status;

    if (Ipc_Dynamic_Ports[ePortType].pPortLock) {

        KeEnterCriticalRegion();
        ExAcquireResourceExclusive(Ipc_Dynamic_Ports[ePortType].pPortLock, TRUE);

        if (pSrcPortName && (*pSrcPortName))
        {
            __try {

                if (pDstPortName)
                {
                    ProbeForWrite(pDstPortName, sizeof(WCHAR) * DYNAMIC_PORT_NAME_CHARS, sizeof(WCHAR));
                    wmemcpy(pDstPortName, pSrcPortName, DYNAMIC_PORT_NAME_CHARS - 1);
                    pDstPortName[DYNAMIC_PORT_NAME_CHARS - 1] = L'\0';
                }

                // save port name in our global Ipc_Dynamic_Ports structure
                wmemcpy(Ipc_Dynamic_Ports[ePortType].wstrPortName, pSrcPortName, DYNAMIC_PORT_NAME_CHARS - 1);
                Ipc_Dynamic_Ports[ePortType].wstrPortName[DYNAMIC_PORT_NAME_CHARS - 1] = L'\0';

                status = STATUS_SUCCESS;

            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = GetExceptionCode();
            }
        }

        ExReleaseResourceLite(Ipc_Dynamic_Ports[ePortType].pPortLock);
        KeLeaveCriticalRegion();
    }

    return status;
}
