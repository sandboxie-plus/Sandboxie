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
#include "util.h"
#include "session.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static NTSTATUS Ipc_Api_GetRpcPortName_2(PEPROCESS ProcessObject, WCHAR *pDstPortName);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------

IPC_DYNAMIC_PORTS Ipc_Dynamic_Ports[NUM_DYNAMIC_PORTS];

static const WCHAR *_rpc_control = L"\\RPC Control";

//---------------------------------------------------------------------------
// Ipc_Api_AllowSpoolerPrintToFile
//---------------------------------------------------------------------------

//_FX NTSTATUS Ipc_Api_AllowSpoolerPrintToFile(PROCESS *proc, ULONG64 *parms)
//{
//    API_ALLOW_SPOOLER_PRINT_TO_FILE_ARGS *pArgs = (API_ALLOW_SPOOLER_PRINT_TO_FILE_ARGS *)parms;
//
//    if (Process_Find(NULL, NULL)) {     // is caller sandboxed?
//        return STATUS_ACCESS_DENIED;
//    }
//
//    if (pArgs->process_id.val > 0)
//    {
//        PROCESS *proc = Process_Find(pArgs->process_id.val, NULL);
//        if (proc && proc != PROCESS_TERMINATED)
//            proc->ipc_allowSpoolerPrintToFile = TRUE;
//    }
//    return 0;
//}


//---------------------------------------------------------------------------
// Ipc_Api_OpenDynamicPort
//---------------------------------------------------------------------------

// Param 1 is dynamic port name (e.g. "\RPC Control\LRPC-f760d5b40689a98168"), WCHAR[DYNAMIC_PORT_NAME_CHARS]
// Param 2 is the process PID for which to open the port, can be 0 when port is special
// Param 3 is the port type/identifier, can be -1 indicating non special port

_FX NTSTATUS Ipc_Api_OpenDynamicPort(PROCESS* proc, ULONG64* parms)
{
    NTSTATUS status = STATUS_SUCCESS;
    //KIRQL irql;
    API_OPEN_DYNAMIC_PORT_ARGS* pArgs = (API_OPEN_DYNAMIC_PORT_ARGS*)parms;
    WCHAR portName[DYNAMIC_PORT_NAME_CHARS];

    if (proc) // is caller sandboxed?
        return STATUS_ACCESS_DENIED;

    //if (PsGetCurrentProcessId() != Api_ServiceProcessId)
    //    return STATUS_ACCESS_DENIED;

    ENUM_DYNAMIC_PORT_TYPE ePortType = NUM_DYNAMIC_PORTS;
    //if (pArgs->port_type.val == -1)
    //    ePortType = NUM_DYNAMIC_PORTS;
    //else 
    if (pArgs->port_type.val <= NUM_DYNAMIC_PORTS)
        ePortType = (ENUM_DYNAMIC_PORT_TYPE)pArgs->port_type.val;
    //else
    //    return STATUS_INVALID_PARAMETER;

    if(pArgs->port_name.val == NULL)
        return STATUS_INVALID_PARAMETER;
    try {
        ProbeForRead(pArgs->port_name.val, sizeof(WCHAR) * DYNAMIC_PORT_NAME_CHARS, sizeof(WCHAR));
        wmemcpy(portName, pArgs->port_name.val, DYNAMIC_PORT_NAME_CHARS - 1);
        portName[DYNAMIC_PORT_NAME_CHARS - 1] = L'\0';
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }
    if (!NT_SUCCESS(status))
        return status;

    //
    // When this is a special port save it our global Ipc_Dynamic_Ports structure
    //

    if (ePortType != NUM_DYNAMIC_PORTS && Ipc_Dynamic_Ports[ePortType].pPortLock)
    {
        KeEnterCriticalRegion();
        ExAcquireResourceExclusiveLite(Ipc_Dynamic_Ports[ePortType].pPortLock, TRUE);

        wmemcpy(Ipc_Dynamic_Ports[ePortType].wstrPortName, portName, DYNAMIC_PORT_NAME_CHARS);

        ExReleaseResourceLite(Ipc_Dynamic_Ports[ePortType].pPortLock);
        KeLeaveCriticalRegion();
    }

    //
    // Open the port for the selected process
    //

    if (pArgs->process_id.val != 0)
    {
        //proc = Process_Find(pArgs->process_id.val, &irql);
        proc = Process_Find(pArgs->process_id.val, NULL);
        if (proc && (proc != PROCESS_TERMINATED))
        {
            KIRQL irql2;

            KeRaiseIrql(APC_LEVEL, &irql2);
            ExAcquireResourceExclusiveLite(proc->ipc_lock, TRUE);

            Process_AddPath(proc, &proc->open_ipc_paths, NULL, FALSE, portName, FALSE);

            ExReleaseResourceLite(proc->ipc_lock);
            KeLowerIrql(irql2);
        }
        else
            status = STATUS_NOT_FOUND;
        //ExReleaseResourceLite(Process_ListLock);
        //KeLowerIrql(irql);
    }

    return status;
}


//---------------------------------------------------------------------------
// Ipc_Api_GetDynamicPortFromPid
//---------------------------------------------------------------------------

// Param 1 is the service PID
// Param 2 will return the port name with "\RPC Control\" prepended

_FX NTSTATUS Ipc_Api_GetDynamicPortFromPid(PROCESS *proc, ULONG64 *parms)
{
    NTSTATUS status;
    PEPROCESS ProcessObject;
    //BOOLEAN done = FALSE;
    API_GET_DYNAMIC_PORT_FROM_PID_ARGS *pArgs = (API_GET_DYNAMIC_PORT_FROM_PID_ARGS *)parms;

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


_FX NTSTATUS Ipc_Api_GetRpcPortName_2(PEPROCESS ProcessObject, WCHAR *pDstPortName)
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

                        __try {

                            if (pDstPortName)
                            {
                                ProbeForWrite(pDstPortName, sizeof(WCHAR) * DYNAMIC_PORT_NAME_CHARS, sizeof(WCHAR));
                                wmemcpy(pDstPortName, name, DYNAMIC_PORT_NAME_CHARS - 1);
                                pDstPortName[DYNAMIC_PORT_NAME_CHARS - 1] = L'\0';
                            }

                        } __except (EXCEPTION_EXECUTE_HANDLER) {
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


//---------------------------------------------------------------------------
// Ipc_Filter_Spooler_Msg
//---------------------------------------------------------------------------


_FX BOOLEAN Ipc_Filter_Spooler_Msg(UCHAR uMsg)
{
    BOOLEAN filter = FALSE;

    switch (uMsg)
    {
    //case 0x00:	//EnumPrinters
    
    //case 0x02:	//SetJob
    //case 0x03:	//GetJob
    //case 0x04:	//EnumJobs
    case 0x05:		//AddPrinter
    case 0x06:		//DeletePrinter
    //case 0x07:	//SetPrinter
    //case 0x08:	//GetPrinter
    
    //case 0x0A:	//EnumPrinterDrivers
    //case 0x0B:	//CallDrvDocumentEventThunk
    //case 0x0C:	//GetPrinterDriverDirectory
    case 0x0D:		//DeletePrinterDriver
    case 0x0E:		//AddPrintProcessor
    //case 0x0F:	//EnumPrintProcessors
    //case 0x10:	//GetPrintProcessorDirectory
    //case 0x11:	//StartDocPrinter
    //case 0x12:	//StartPagePrinter
    //case 0x13:	//FlushBuffer
    //case 0x14:	//EndPagePrinter
    //case 0x15:	//AbortPrinter
    //case 0x16:	//ReadPrinter
    //case 0x17:	//EndDocPrinter
    //case 0x18:	//AddJob
    //case 0x19:	//ScheduleJobWorker
    //case 0x1A:	//GetPrinterData
    case 0x1B:		//SetPrinterData
    //case 0x1C:	//WaitForPrinterChange
    //case 0x1D:	//ClosePrinterContextHandle
    case 0x1E:		//AddForm
    case 0x1F:		//DeleteForm
    //case 0x20:	//GetForm
    case 0x21:		//SetForm
    //case 0x22:	//EnumForms
    //case 0x23:	//EnumPorts
    //case 0x24:	//EnumMonitors
    case 0x25:		//AddPort
    case 0x26:		//ConfigurePort
    case 0x27:		//DeletePort
    case 0x28:		//CreatePrinterIC
    case 0x29:		//PlayGdiScriptOnPrinterIC
    case 0x2A:		//DeletePrinterIC
    
    //case 0x2C:	//DeletePrinterConnection
    
    case 0x2E:		//AddMonitor
    case 0x2F:		//DeleteMonitor
    case 0x30:		//DeletePrintProcessor
    case 0x31:		//AddPrintProvidor
    case 0x32:		//DeletePrintProvidor
    //case 0x33:	//EnumPrintProcessorDatatypes
    
    //case 0x35:	//GetPrinterDriver
    //case 0x36:	//FindFirstPrinterChangeNotificationWorker
    //case 0x37:	//FindNextPrinterChangeNotification
    //case 0x38:	//FindClosePrinterChangeNotificationWorker
    

    case 0x3D: 		//AddPortEx
    

    //case 0x40: 	//ResetPrinter


    case 0x47: 	    //SetPort
    //case 0x48: 	//EnumPrinterData
    case 0x49: 	    //DeletePrinterData


    case 0x4D: 	    //SetPrinterDataEx
    //case 0x4E: 	//GetPrinterDataEx
    //case 0x4F: 	//EnumPrinterDataEx
    //case 0x50: 	//EnumPrinterKey
    case 0x51: 		//DeletePrinterDataEx
    case 0x52: 		//DeletePrinterKey
    //case 0x53: 	//SeekPrinter
    case 0x54: 		//DeletePrinterDriverEx
    case 0x55: 		//AddPerMachineConnection
    case 0x56: 		//DeletePerMachineConnection
    //case 0x57: 	//EnumPerMachineConnections
    //case 0x58: 	//GetMonitorUI
    case 0x59: 		//AddPrinterDriverEx
    //case 0x5A: 	//OpenPrinterRPC


    //case 0x5D: 	//GetSpoolFileHandle
    //case 0x5E: 	//CommitSpoolData
    //case 0x5F: 	//CloseSpoolFileHandle
    //case 0x60: 	//FlushPrinter
    //case 0x61: 	//SendRecvBidiData
    
    case 0x63: 		//AddPrinterConnection
    case 0x64: 		//InstallPrinterDriverFromPackage
    case 0x65: 		//UploadPrinterDriverPackage
    //case 0x66: 	//GetCorePrinterDrivers
    //case 0x67: 	//CorePrinterDriverInstalled
    //case 0x68: 	//GetPrinterDriverPackagePath
    case 0x69: 		//DeletePrinterDriverPackage
    //case 0x6A: 	//FindCompatibleDriver
    //case 0x6B: 	//ReportJobProcessingProgress
    case 0x6C: 		//SpoolerSetPolicy
    //case 0x6D: 	//GetPrinterDriver
    //case 0x6E: 	//GetJobNamedPropertyValue
    //case 0x6F: 	//SetJobNamedProperty
    //case 0x70: 	//DeleteJobNamedProperty
    //case 0x71: 	//EnumJobNamedProperties
    //case 0x72: 	//ConnectToLd64In32ServerWorker
    //case 0x73: 	//GetUserPropertyBag
    //case 0x74: 	//LogJobInfoForBranchOffice
    //case 0x75: 	//RegeneratePrintDeviceCapabilities

    //case 0xEF:    //Unknown
        filter = TRUE;
    }
    
    if (Session_MonitorCount) {

        WCHAR access_str[24];
        swprintf(access_str, L" Msg: %02X", (ULONG)uMsg);
        const WCHAR* strings[3] = { L"\\RPC Control\\spoolss", access_str, NULL };
        Session_MonitorPutEx(MONITOR_IPC | (filter ? MONITOR_DENY : MONITOR_OPEN), strings, NULL, PsGetCurrentProcessId());
    }

    return filter;
}
