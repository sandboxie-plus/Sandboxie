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
// Low Level Syscall Interface
//---------------------------------------------------------------------------


#include "process.h"
#include "api.h"
#include "util.h"
#include "conf.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static NTSTATUS Process_Low_Api_InjectComplete(
    PROCESS *proc, ULONG64 *parms);


//---------------------------------------------------------------------------


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, Process_Low_Init)
#endif // ALLOC_PRAGMA


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static KEVENT *Process_Low_Event = NULL;


//---------------------------------------------------------------------------
// Low_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Process_Low_Init(void)
{
    Process_Low_Event = ExAllocatePoolWithTag(NonPagedPool, sizeof(KEVENT), tzuk);
    if (! Process_Low_Event) {
        Log_Msg0(MSG_1104);
        return FALSE;
    }
    KeInitializeEvent(Process_Low_Event, SynchronizationEvent, FALSE);

    Api_SetFunction(API_INJECT_COMPLETE, Process_Low_Api_InjectComplete);

    return TRUE;
}


//---------------------------------------------------------------------------
// Process_Low_Unload
//---------------------------------------------------------------------------


_FX void Process_Low_Unload(void)
{
    if (Process_Low_Event) {
        ExFreePoolWithTag(Process_Low_Event, tzuk);
        Process_Low_Event = NULL;
    }
}


//---------------------------------------------------------------------------
// Process_Low_Inject
//---------------------------------------------------------------------------


_FX BOOLEAN Process_Low_Inject(
    HANDLE process_id, ULONG session_id, ULONG64 create_time,
    const WCHAR *image_name, BOOLEAN add_process_to_job, BOOLEAN bHostInject)
{
    PROCESS *proc;
    SVC_PROCESS_MSG msg;
    ULONG_PTR is_wow64 = 0;
    NTSTATUS status = STATUS_SUCCESS;
    BOOLEAN done = FALSE;
    KIRQL irql;

    //
    // query wow64 flag for new process
    //

#ifdef _WIN64

    if (1) {

        OBJECT_ATTRIBUTES objattrs;
        CLIENT_ID clientid;
        HANDLE ProcessHandle;
        ULONG x;

        InitializeObjectAttributes(&objattrs,
            NULL, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
        clientid.UniqueThread = NULL;
        clientid.UniqueProcess = process_id;

        status = ZwOpenProcess(&ProcessHandle, PROCESS_QUERY_INFORMATION,
                               &objattrs, &clientid);

        if (NT_SUCCESS(status)) {

            status = ZwQueryInformationProcess(
                                ProcessHandle, ProcessWow64Information,
                                &is_wow64, sizeof(is_wow64), &x);

            ZwClose(ProcessHandle);
        }
    }

#endif _WIN64

    //
    // send message to SbieSvc DriverAssist
    //

    if (NT_SUCCESS(status)) {

        ULONG len = wcslen(image_name);
        const ULONG max_len = sizeof(msg.process_name) / sizeof(WCHAR) - 1;
        if (len > max_len)
            len = max_len;
        wmemcpy(msg.process_name, image_name, len);
        msg.process_name[len] = L'\0';

        msg.process_id = (ULONG)(ULONG_PTR)process_id;
        msg.session_id = session_id;
        msg.create_time = create_time;
        msg.is_wow64 = (is_wow64 ? TRUE : FALSE);
        msg.add_to_job = add_process_to_job;
        msg.bHostInject = bHostInject;

        if (! Api_SendServiceMessage(SVC_INJECT_PROCESS, sizeof(msg), &msg))
            status = STATUS_SERVER_DISABLED;
    }

    //
    // wait for response from SbieSvc DriverAssist
    //
    // note that our original process structure may disappear at any
    // moment (as discussed in Process_NotifyProcess_Create) so we
    // check creation time to be sure it's still the same process
    // that we're expecting
    //

    if (NT_SUCCESS(status)) {

        LARGE_INTEGER time;
        ULONG retries = 0;

        while ((retries < 40 * 3) && (! Driver_Unloading)) {

            proc = Process_Find(process_id, &irql);

            if (proc && proc->create_time == create_time) {

                done = proc->sbielow_loaded || proc->terminated;

                if (! is_wow64)
                    proc->ntdll32_base = -1;
            }

            ExReleaseResourceLite(Process_ListLock);
            KeLowerIrql(irql);

            if (done)
                break;

            time.QuadPart = -(SECONDS(1) / 4); // 250ms*40 = 10s
            KeWaitForSingleObject(Process_Low_Event,
                                  Executive, KernelMode, FALSE, &time);
            ++retries;
        }

        if (! done)           // if no response from SbieSvc
            status = STATUS_TIMEOUT;
    }

    //
    // cancel process and report errors
    //

    if (! NT_SUCCESS(status) && !bHostInject) {

        proc = Process_Find(process_id, &irql);

        if (proc && proc->create_time == create_time) {

            Process_SetTerminated(proc, 3);
        }

        ExReleaseResourceLite(Process_ListLock);
        KeLowerIrql(irql);

        Log_Status_Ex_Process(MSG_1231, 0x22, status, image_name, session_id, process_id);

        return FALSE;
    }

    return TRUE; // sbielow_loaded;
}


//---------------------------------------------------------------------------
// Process_Low_Api_InjectComplete
//---------------------------------------------------------------------------


_FX NTSTATUS Process_Low_Api_InjectComplete(PROCESS *proc, ULONG64 *parms)
{
    NTSTATUS status;

    //
    // this API must be invoked by the Sandboxie service
    //

    if (proc || (PsGetCurrentProcessId() != Api_ServiceProcessId)) {

        status = STATUS_ACCESS_DENIED;

    } else {

        //
        // turn on the SbieLow loaded flag for the process
        //

        HANDLE ProcessId = (HANDLE)(ULONG_PTR)(ULONG)parms[1];

        KIRQL irql;
        PROCESS *proc = Process_Find(ProcessId, &irql);

        if (proc) {

            ULONG error = (ULONG)parms[3];
            if (error) 
                Process_SetTerminated(proc, 3);
            else
                proc->sbielow_loaded = TRUE;

            //
            // the service dynamically allocates a per box SID to be used,
            // if no SID is provided this feature is either disabled or failed
            // then we fall back to using the default anonymous SID
            //

            __try {

                PSID pSID = (PSID)(ULONG_PTR)parms[2];

                if (pSID) {

                    ProbeForRead(pSID, SECURITY_MAX_SID_SIZE, sizeof(UCHAR));

                    ULONG sid_length = RtlLengthSid(pSID);
                    proc->SandboxieLogonSid = Mem_Alloc(proc->pool, sid_length);
                    memcpy(proc->SandboxieLogonSid, pSID, sid_length);
                }

            } __except (EXCEPTION_EXECUTE_HANDLER) {
                status = GetExceptionCode();
            }
        }

        ExReleaseResourceLite(Process_ListLock);
        KeLowerIrql(irql);

        if (proc) {

            KeSetEvent(Process_Low_Event, 0, FALSE);
            status = STATUS_SUCCESS;

        } else
            status = STATUS_INVALID_CID;
    }

    return status;
}


//---------------------------------------------------------------------------
// Process_Low_InitConsole
//---------------------------------------------------------------------------


_FX BOOLEAN Process_Low_InitConsole(PROCESS *proc)
{
    NTSTATUS status;

	// NoSbieCons BEGIN
	if (proc->bAppCompartment || Conf_Get_Boolean(proc->box->name, L"NoSandboxieConsole", 0, FALSE))
		return TRUE;
	// NoSbieCons END

    //
    // on Windows 7, a console process tries to launch conhost.exe through
    // csrss.exe during initialization of kernel32.dll in the function
    // kernel32!ConnectConsoleInternal.  this will fail because the process
    // is using a highly restricted primary token.
    //
    // to work around this, we modify here the RTL_USER_PROCESS_PARAMETERS
    // structure as if the process was started with the CreateProcess flag
    // DETACHED_PROCESS so the process doesn't try to connect to a console.
    //
    // later on, SbieDll will ask SbieSvc to create conhost.exe on behalf
    // of the process in the sandbox, and then connect to that console.
    // see core/dll/guicon.c for more about this.
    //
    // this code should be invoked before the process token is replaced,
    // so SbieSvc can create the console process based on the real token
    //
    // note that none of this is needed prior to Windows 7
    //

    if (Driver_OsVersion < DRIVER_WINDOWS_7) {
        status = STATUS_SUCCESS;
        goto finish;
    }

    if (PsGetCurrentProcessId() != proc->pid) {
        status = STATUS_DEVICE_ALREADY_ATTACHED;
        goto finish;
    }

    __try {

        //
        // select PEB offsets according to system bitness
        //

#ifdef _WIN64
        const ULONG UserProcessParms_offset =   0x20;   // 64-bit
        const ULONG ImageSubsystem_offset   =   0x128;
#else ! _WIN64
        const ULONG UserProcessParms_offset =   0x10;   // 32-bit
        const ULONG ImageSubsystem_offset   =   0xB4;
#endif _WIN64
        const ULONG ConsoleHandle_offset    =   0x10;   // 32/64
        const ULONG PebLength = ImageSubsystem_offset + sizeof(ULONG_PTR);

        const UCHAR IMAGE_SUBSYSTEM_WINDOWS_CUI = 3;

        //
        // make sure PEB block is accessible
        //

        ULONG_PTR Peb = PsGetProcessPeb(PsGetCurrentProcess());

        ProbeForRead((void *)Peb, PebLength, sizeof(ULONG_PTR));

        // DbgPrint("Process %d Image Subsystem %d\n", proc->pid, *(UCHAR *)(Peb + ImageSubsystem_offset));

        if (*(UCHAR *)(Peb + ImageSubsystem_offset)
                                            == IMAGE_SUBSYSTEM_WINDOWS_CUI) {

            //
            // this is a console process.  we will need to open a new
            // console, unless the ConsoleHandle field in the
            // RTL_USER_PROCESS_PARAMETERS structure already specifies
            // an existing console pid
            //

            const HANDLE DETACHED_PROCESS   = (HANDLE) -1;
            const HANDLE CREATE_NEW_CONSOLE = (HANDLE) -2;
            const HANDLE CREATE_NO_WINDOW   = (HANDLE) -3;

            ULONG_PTR RtlUserProcessParms =
                            *(ULONG_PTR *)(Peb + UserProcessParms_offset);
            HANDLE *PtrConsoleHandle =
                    (HANDLE *)(RtlUserProcessParms + ConsoleHandle_offset);
            ProbeForRead(PtrConsoleHandle, sizeof(HANDLE), sizeof(HANDLE));

            // DbgPrint("Console Handle = %p\n", *PtrConsoleHandle);

            if (    *PtrConsoleHandle == 0
                 || *PtrConsoleHandle == CREATE_NEW_CONSOLE
                 || *PtrConsoleHandle == CREATE_NO_WINDOW) {

                if (*PtrConsoleHandle == CREATE_NO_WINDOW)
                    proc->create_console_flag = 'H';    // hide
                else
                    proc->create_console_flag = 'S';    // show

                *PtrConsoleHandle = DETACHED_PROCESS;

                //
                // note that if this a Wow64 process, InitConsole
                // in core/low/init.c will copy the DETACHED_PROCESS value
                // from the 64-bit ConsoleHandle that we adjusted here,
                // into the corresponding 32-bit ConsoleHandle field
                //
            }
        }

        status = STATUS_SUCCESS;

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    //
    //
    //

finish:

    if (! NT_SUCCESS(status)) {
		Log_Status_Ex_Process(MSG_1231, 0x66, status, proc->image_name, proc->box->session_id, proc->pid);
        return FALSE;
    }

    return TRUE;
}
