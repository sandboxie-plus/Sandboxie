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
#include "util.h"
#include "session.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


BOOLEAN Ipc_Filter_Spooler_Msg(PROCESS* proc, UCHAR uMsg);


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
// Ipc_CheckPortRequest_SpoolerPort
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_CheckPortRequest_SpoolerPort(
    PROCESS* proc, OBJECT_NAME_INFORMATION* Name, PORT_MESSAGE* msg)
{
    NTSTATUS status;

    if (proc->ipc_openPrintSpooler)        // see if we are not filtering spooler requests
        return STATUS_BAD_INITIAL_PC;

    //
    // check that it is the spooler port
    //

    if (Driver_OsVersion >= DRIVER_WINDOWS_81) {

        if (Name->Name.Length < 13 * sizeof(WCHAR))
            return STATUS_BAD_INITIAL_PC;

        BOOLEAN is_spooler = FALSE;

        if (Ipc_Dynamic_Ports.pPortLock)
        {
            KeEnterCriticalRegion();
            ExAcquireResourceSharedLite(Ipc_Dynamic_Ports.pPortLock, TRUE);

            if (Ipc_Dynamic_Ports.pSpoolerPort 
                && _wcsicmp(Name->Name.Buffer, Ipc_Dynamic_Ports.pSpoolerPort->wstrPortName) == 0)
            {
                // dynamic version of RPC ports, see also ipc_spl.c
                // and RpcBindingFromStringBindingW in core/dll/rpcrt.c
                is_spooler = TRUE;
            }

            ExReleaseResourceLite(Ipc_Dynamic_Ports.pPortLock);
            KeLeaveCriticalRegion();
        }

        if (!is_spooler)
            return STATUS_BAD_INITIAL_PC;
    }
    else if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA) {

        if (_wcsicmp(Name->Name.Buffer, L"\\RPC Control\\spoolss") != 0)
            return STATUS_BAD_INITIAL_PC;

    }
    else
        return STATUS_BAD_INITIAL_PC;

    //
    // examine message
    //

    status = STATUS_SUCCESS;

    __try {

        ProbeForRead(msg, sizeof(PORT_MESSAGE), sizeof(ULONG_PTR));

        if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA) {

            //
            //

            ULONG  len = msg->u1.s1.DataLength;
            UCHAR* ptr = (UCHAR*)((UCHAR*)msg + sizeof(PORT_MESSAGE));
            int i = 0;
            int rc = -2;

            ProbeForRead(ptr, len, sizeof(WCHAR));

            /*if (ptr[20] == 17) {        // RpcStartDocPrinter = Opnum 17

                if (!proc->ipc_allowSpoolerPrintToFile)
                {
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

                if (status == STATUS_ACCESS_DENIED)
                    Log_MsgP0(MSG_1319, proc->pid);
            }
            else*/

            if (Ipc_Filter_Spooler_Msg(proc, ptr[20]))
                status = STATUS_ACCESS_DENIED;

            //DbgPrint("Spooler IPC Port message ID: %d\n", (int)ptr[20]);

        }

    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    return status;
}


//---------------------------------------------------------------------------
// Ipc_Filter_Spooler_Msg
//---------------------------------------------------------------------------


_FX BOOLEAN Ipc_Filter_Spooler_Msg(PROCESS* proc, UCHAR uMsg)
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
    
    if (Session_MonitorCount && (proc->ipc_trace & (TRACE_ALLOW | TRACE_DENY))) {

        ULONG mon_type = MONITOR_IPC;

        if (filter && (proc->ipc_trace & TRACE_DENY))
            mon_type |= MONITOR_DENY;
        else if (!filter && (proc->ipc_trace & TRACE_ALLOW))
            mon_type |= MONITOR_OPEN;
        else
            mon_type = 0;

        if (mon_type) {
            WCHAR msg_str[24];
            RtlStringCbPrintfW(msg_str, sizeof(msg_str), L"Msg: %02X", (ULONG)uMsg);
            Log_Debug_Msg(mon_type, L"\\RPC Control\\spoolss", msg_str);
        }
    }

    return filter;
}
