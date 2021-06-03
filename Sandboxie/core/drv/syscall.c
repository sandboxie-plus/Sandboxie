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
// Syscall Management
//---------------------------------------------------------------------------


#include "syscall.h"
#include "dll.h"
#include "process.h"
#include "thread.h"
#include "obj.h"
#include "api.h"
#include "util.h"
#include "session.h"



//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN Syscall_Init_List(void);

static BOOLEAN Syscall_Init_Table(void);

static BOOLEAN Syscall_Init_ServiceData(void);

static void Syscall_ErrorForAsciiName(const UCHAR *name_a);


//---------------------------------------------------------------------------


static NTSTATUS Syscall_OpenHandle(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args);

static NTSTATUS Syscall_GetNextProcess(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args);

static NTSTATUS Syscall_DeviceIoControlFile(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args);

static NTSTATUS Syscall_DuplicateHandle(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args);

static BOOLEAN Syscall_QuerySystemInfo_SupportProcmonStack(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args);


//---------------------------------------------------------------------------


static NTSTATUS Syscall_Api_Query(PROCESS *proc, ULONG64 *parms);

static NTSTATUS Syscall_Api_Invoke(PROCESS *proc, ULONG64 *parms);


//---------------------------------------------------------------------------


static ULONG Syscall_GetIndexFromNtdll(
    UCHAR *code, const UCHAR *name, ULONG name_len);

static BOOLEAN Syscall_GetKernelAddr(
    ULONG index, void **pKernelAddr, ULONG *pParamCount);


//---------------------------------------------------------------------------


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, Syscall_Init)
#pragma alloc_text (INIT, Syscall_Init_List)
#pragma alloc_text (INIT, Syscall_Init_Table)
#pragma alloc_text (INIT, Syscall_Init_ServiceData)
#pragma alloc_text (INIT, Syscall_GetByName)
#pragma alloc_text (INIT, Syscall_Set1)
#pragma alloc_text (INIT, Syscall_Set2)
#pragma alloc_text (INIT, Syscall_ErrorForAsciiName)
#pragma alloc_text (INIT, Syscall_GetIndexFromNtdll)
#pragma alloc_text (INIT, Syscall_GetKernelAddr)
#ifdef _WIN64
// only needed for 32-bit gui_xp code
#pragma alloc_text (INIT, Syscall_GetServiceTable)
#endif _WIN64
#endif // ALLOC_PRAGMA


//---------------------------------------------------------------------------


typedef NTSTATUS (*P_SystemService00)(void);
typedef NTSTATUS (*P_SystemService01)(
    ULONG_PTR arg01);
typedef NTSTATUS (*P_SystemService02)(
    ULONG_PTR arg01, ULONG_PTR arg02);
typedef NTSTATUS (*P_SystemService03)(
    ULONG_PTR arg01, ULONG_PTR arg02, ULONG_PTR arg03);
typedef NTSTATUS (*P_SystemService04)(
    ULONG_PTR arg01, ULONG_PTR arg02, ULONG_PTR arg03, ULONG_PTR arg04);
typedef NTSTATUS (*P_SystemService05)(
    ULONG_PTR arg01, ULONG_PTR arg02, ULONG_PTR arg03, ULONG_PTR arg04,
    ULONG_PTR arg05);
typedef NTSTATUS (*P_SystemService06)(
    ULONG_PTR arg01, ULONG_PTR arg02, ULONG_PTR arg03, ULONG_PTR arg04,
    ULONG_PTR arg05, ULONG_PTR arg06);
typedef NTSTATUS (*P_SystemService07)(
    ULONG_PTR arg01, ULONG_PTR arg02, ULONG_PTR arg03, ULONG_PTR arg04,
    ULONG_PTR arg05, ULONG_PTR arg06, ULONG_PTR arg07);
typedef NTSTATUS (*P_SystemService08)(
    ULONG_PTR arg01, ULONG_PTR arg02, ULONG_PTR arg03, ULONG_PTR arg04,
    ULONG_PTR arg05, ULONG_PTR arg06, ULONG_PTR arg07, ULONG_PTR arg08);
typedef NTSTATUS (*P_SystemService09)(
    ULONG_PTR arg01, ULONG_PTR arg02, ULONG_PTR arg03, ULONG_PTR arg04,
    ULONG_PTR arg05, ULONG_PTR arg06, ULONG_PTR arg07, ULONG_PTR arg08,
    ULONG_PTR arg09);
typedef NTSTATUS (*P_SystemService10)(
    ULONG_PTR arg01, ULONG_PTR arg02, ULONG_PTR arg03, ULONG_PTR arg04,
    ULONG_PTR arg05, ULONG_PTR arg06, ULONG_PTR arg07, ULONG_PTR arg08,
    ULONG_PTR arg09, ULONG_PTR arg10);
typedef NTSTATUS (*P_SystemService11)(
    ULONG_PTR arg01, ULONG_PTR arg02, ULONG_PTR arg03, ULONG_PTR arg04,
    ULONG_PTR arg05, ULONG_PTR arg06, ULONG_PTR arg07, ULONG_PTR arg08,
    ULONG_PTR arg09, ULONG_PTR arg10, ULONG_PTR arg11);
typedef NTSTATUS (*P_SystemService12)(
    ULONG_PTR arg01, ULONG_PTR arg02, ULONG_PTR arg03, ULONG_PTR arg04,
    ULONG_PTR arg05, ULONG_PTR arg06, ULONG_PTR arg07, ULONG_PTR arg08,
    ULONG_PTR arg09, ULONG_PTR arg10, ULONG_PTR arg11, ULONG_PTR arg12);
typedef NTSTATUS (*P_SystemService13)(
    ULONG_PTR arg01, ULONG_PTR arg02, ULONG_PTR arg03, ULONG_PTR arg04,
    ULONG_PTR arg05, ULONG_PTR arg06, ULONG_PTR arg07, ULONG_PTR arg08,
    ULONG_PTR arg09, ULONG_PTR arg10, ULONG_PTR arg11, ULONG_PTR arg12,
    ULONG_PTR arg13);
typedef NTSTATUS (*P_SystemService14)(
    ULONG_PTR arg01, ULONG_PTR arg02, ULONG_PTR arg03, ULONG_PTR arg04,
    ULONG_PTR arg05, ULONG_PTR arg06, ULONG_PTR arg07, ULONG_PTR arg08,
    ULONG_PTR arg09, ULONG_PTR arg10, ULONG_PTR arg11, ULONG_PTR arg12,
    ULONG_PTR arg13, ULONG_PTR arg14);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static LIST Syscall_List;

static SYSCALL_ENTRY **Syscall_Table = NULL;

static ULONG Syscall_MaxIndex = 0;

static UCHAR *Syscall_NtdllSavedCode = NULL;

static SYSCALL_ENTRY *Syscall_SetInformationThread = NULL;


//---------------------------------------------------------------------------
// Syscall_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Syscall_Init(void)
{
    if (! Syscall_Init_List())
        return FALSE;

    if (! Syscall_Init_Table())
        return FALSE;

    if (! Syscall_Init_ServiceData())
        return FALSE;

    if (! Syscall_Set1("DuplicateObject", Syscall_DuplicateHandle))
        return FALSE;

    if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA) {
        if (!Syscall_Set1("GetNextProcess", Syscall_GetNextProcess))
            return FALSE;
    }

    if (!Syscall_Set1("DeviceIoControlFile", Syscall_DeviceIoControlFile))
        return FALSE;

    if (!Syscall_Set3("QuerySystemInformation", Syscall_QuerySystemInfo_SupportProcmonStack))
        return FALSE;

    //
    // set API handlers
    //

    Api_SetFunction(API_QUERY_SYSCALLS,     Syscall_Api_Query);
    Api_SetFunction(API_INVOKE_SYSCALL,     Syscall_Api_Invoke);

    //
    // finish
    //

    return TRUE;
}


//---------------------------------------------------------------------------
// Syscall_Init_List
//---------------------------------------------------------------------------


_FX BOOLEAN Syscall_Init_List(void)
{
    UCHAR *name, *ntdll_code;
    void *ntos_addr;
    DLL_ENTRY *dll;
    SYSCALL_ENTRY *entry;
    ULONG proc_index, proc_offset, syscall_index, param_count;
    ULONG name_len, entry_len;

    List_Init(&Syscall_List);

    //
    // scan each ZwXxx export in NTDLL
    //

    dll = Dll_Load(Dll_NTDLL);
    if (! dll)
        return FALSE;

    proc_offset = Dll_GetNextProc(dll, "Zw", &name, &proc_index);
    if (! proc_offset)
        return FALSE;

    while (proc_offset) {

        if (name[0] != 'Z' || name[1] != 'w')
            break;
        name += 2;                  // skip Zw prefix
        for (name_len = 0; (name_len < 64) && name[name_len]; ++name_len)
            ;

        //DbgPrint("    Found SysCall %s\n", name);

        entry = NULL;

        //
        // we don't hook ZwXxx services which do not or may not return to
        // caller. this is because the invocation of Syscall_Api_Invoke
        // goes through IopXxxControlFile (called by NtDeviceIoControlFile)
        // which takes a reference on the file object for our API device
        // object, and no return means there is no corresponding dereference
        // for the file object.  there could be other nasty side effects.
        //
        // ZwXxx services:   ZwContinue, ZwCallbackReturn, ZwRaiseException,
        // NtTerminateJobObject, NtTerminateProcess, NtTerminateThread
        //

#define IS_PROC_NAME(ln,nm) (name_len == ln && memcmp(name, nm, ln) == 0)

        if (    IS_PROC_NAME(8,  "Continue")
            ||  IS_PROC_NAME(10, "ContinueEx")
            ||  IS_PROC_NAME(14, "CallbackReturn")
            ||  IS_PROC_NAME(14, "RaiseException")
            ||  IS_PROC_NAME(18, "TerminateJobObject")
            ||  IS_PROC_NAME(16, "TerminateProcess")
            ||  IS_PROC_NAME(15, "TerminateThread")
            ||  IS_PROC_NAME(14, "YieldExecution")            // ICD-10607 - McAfee uses it to pass its own data in the stack. The call is not important to us. 

                                                            ) {
            goto next_zwxxx;
        }

        //
        // the Google Chrome "wow_helper" process expects NtMapViewOfSection
        // to not be already hooked.  although this is needed only on 64-bit
        // Vista, this particular syscall is not very important to us, so
        // for sake of consistency, we skip hooking it on all platforms
        //

        if (    IS_PROC_NAME(16,  "MapViewOfSection"))
            goto next_zwxxx;

        //
        // analyze each ZwXxx export to find the service index number
        //

        ntos_addr = NULL;
        param_count = 0;

        ntdll_code = Dll_RvaToAddr(dll, proc_offset);
        if (ntdll_code) {

            syscall_index =
                Syscall_GetIndexFromNtdll(ntdll_code, name, name_len);

            if (syscall_index == -2) {
                //
                // if ZwXxx export is not a real syscall, then skip it
                //
                goto next_zwxxx;
            }

            if (syscall_index != -1) {
                Syscall_GetKernelAddr(
                            syscall_index, &ntos_addr, &param_count);
            }
        }

        if (! ntos_addr) {

            Syscall_ErrorForAsciiName(name);
            return FALSE;
        }

        //
        // store an entry for the syscall in our list of syscalls
        //

        entry_len = sizeof(SYSCALL_ENTRY) + name_len + 1;
        entry = Mem_AllocEx(Driver_Pool, entry_len, TRUE);
        if (! entry)
            return FALSE;

        entry->syscall_index = (USHORT)syscall_index;
        entry->param_count = (USHORT)param_count;
        entry->ntdll_offset = proc_offset;
        entry->ntos_func = ntos_addr;
        entry->handler1_func = NULL;
        entry->handler2_func = NULL;
        entry->handler3_func_support_procmon = NULL;
        entry->name_len = (USHORT)name_len;
        memcpy(entry->name, name, name_len);
        entry->name[name_len] = '\0';

        List_Insert_After(&Syscall_List, NULL, entry);

        if (syscall_index > Syscall_MaxIndex)
            Syscall_MaxIndex = syscall_index;

        //
        // process next ZwXxx export
        //

next_zwxxx:

        proc_offset = Dll_GetNextProc(dll, NULL, &name, &proc_index);
    }

    //
    // report an error if we did not find a reasonable number of services
    //

    if (Syscall_MaxIndex < 100) {
        Log_Msg1(MSG_1113, L"100");
        return FALSE;
    }

    if (Syscall_MaxIndex >= 500) {
        Log_Msg1(MSG_1113, L"500");
        return FALSE;
    }

    //
    // workaround for Online Armor driver
    //

#ifndef _WIN64

    if (1) {
        extern void Syscall_HandleOADriver(void);
        Syscall_HandleOADriver();
    }

#endif ! _WIN64

    return TRUE;
}


//---------------------------------------------------------------------------
// Syscall_Init_Table
//---------------------------------------------------------------------------


_FX BOOLEAN Syscall_Init_Table(void)
{
    SYSCALL_ENTRY *entry;
    ULONG len, i;

    //
    // build the table which maps syscall index numbers to entries
    //

    len = sizeof(SYSCALL_ENTRY *) * (Syscall_MaxIndex + 1);
    Syscall_Table = Mem_AllocEx(Driver_Pool, len, TRUE);
    if (! Syscall_Table)
        return FALSE;

    for (i = 0; i <= Syscall_MaxIndex; ++i) {

        entry = List_Head(&Syscall_List);
        while (entry) {
            if (entry->syscall_index == i)
                break;
            entry = List_Next(entry);
        }

        Syscall_Table[i] = entry;
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Syscall_Init_ServiceData
//---------------------------------------------------------------------------


_FX BOOLEAN Syscall_Init_ServiceData(void)
{
    UCHAR *NtdllExports[] = {
        "DelayExecution", "DeviceIoControlFile", "FlushInstructionCache",
        "ProtectVirtualMemory"
    };
    SYSCALL_ENTRY *entry;
    DLL_ENTRY *dll;
    UCHAR *ntdll_code;
    ULONG i;

    //
    // allocate some space to save code from ntdll
    //

    Syscall_NtdllSavedCode = Mem_AllocEx(Driver_Pool, (32 * 4), TRUE);
    if (! Syscall_NtdllSavedCode)
        return FALSE;

    dll = Dll_Load(Dll_NTDLL);
    if (! dll)
        return FALSE;

    //
    // save a copy of some syscall functions in ntdll, we will pass
    // these to SbieSvc, which in turn will pass them to SbieLow
    // (see core/svc/DriverAssistInject.cpp and core/low/lowdata.h)
    //

    for (i = 0; i < 4; ++i) {

        entry = Syscall_GetByName(NtdllExports[i]);
        if (! entry)
            return FALSE;

        ntdll_code = Dll_RvaToAddr(dll, entry->ntdll_offset);
        if (! ntdll_code) {
            Syscall_ErrorForAsciiName(entry->name);
            return FALSE;
        }

        memcpy(Syscall_NtdllSavedCode + (i * 32), ntdll_code, 32);
    }

    //
    // get the syscall for NtSetInformationThread (see Syscall_Api_Invoke)
    //

    Syscall_SetInformationThread = Syscall_GetByName("SetInformationThread");

    //
    // finish
    //

    return TRUE;
}


//---------------------------------------------------------------------------
// Syscall_GetByName
//---------------------------------------------------------------------------


_FX SYSCALL_ENTRY *Syscall_GetByName(const UCHAR *name)
{
    ULONG name_len = strlen(name);

    SYSCALL_ENTRY *entry = List_Head(&Syscall_List);
    while (entry) {

        if (entry->name_len == name_len
                    && memcmp(entry->name, name, name_len) == 0) {

            return entry;
        }

        entry = List_Next(entry);
    }

    Syscall_ErrorForAsciiName(name);
    return NULL;
}


//---------------------------------------------------------------------------
// Syscall_Set1
//---------------------------------------------------------------------------


_FX BOOLEAN Syscall_Set1(const UCHAR *name, P_Syscall_Handler1 handler_func)
{
    SYSCALL_ENTRY *entry = Syscall_GetByName(name);
    if (! entry)
        return FALSE;
    entry->handler1_func = handler_func;
    return TRUE;
}

//---------------------------------------------------------------------------
// Syscall_Set3
//---------------------------------------------------------------------------


_FX BOOLEAN Syscall_Set3(const UCHAR *name, P_Syscall_Handler3_Support_Procmon_Stack handler_func)
{
    SYSCALL_ENTRY *entry = Syscall_GetByName(name);
    if (!entry)
        return FALSE;
    entry->handler3_func_support_procmon = handler_func;
    return TRUE;
}


//---------------------------------------------------------------------------
// Syscall_ErrorForAsciiName
//---------------------------------------------------------------------------


_FX void Syscall_ErrorForAsciiName(const UCHAR *name_a)
{
    WCHAR name_w[66];
    ULONG i;

    for (i = 0; (i < 64) && name_a[i]; ++i)
        name_w[i] = name_a[i];
    name_w[i] = '\0';

    Log_Msg1(MSG_1113, name_w);
}


//---------------------------------------------------------------------------
// Syscall_Invoke
//---------------------------------------------------------------------------
extern unsigned int g_TrapFrameOffset;

_FX NTSTATUS Syscall_Invoke(SYSCALL_ENTRY *entry, ULONG_PTR *stack)
{
    NTSTATUS status;

    __try {

        //DbgPrint("[syscall] request param count = %d\n", entry->param_count);

        if (entry->param_count == 0) {

            P_SystemService00 nt = (P_SystemService00)entry->ntos_func;
            status = nt();

        } else if (entry->param_count == 1) {

            P_SystemService01 nt = (P_SystemService01)entry->ntos_func;
            status = nt(stack[0]);

        } else if (entry->param_count == 2) {

            P_SystemService02 nt = (P_SystemService02)entry->ntos_func;
            status = nt(stack[0], stack[1]);

        } else if (entry->param_count == 3) {

            P_SystemService03 nt = (P_SystemService03)entry->ntos_func;
            status = nt(stack[0], stack[1], stack[2]);

        } else if (entry->param_count == 4) {

            P_SystemService04 nt = (P_SystemService04)entry->ntos_func;
            status = nt(stack[0], stack[1], stack[2], stack[3]);

        } else if (entry->param_count == 5) {

            P_SystemService05 nt = (P_SystemService05)entry->ntos_func;
            status = nt(stack[0], stack[1], stack[2], stack[3], stack[4]);

        } else if (entry->param_count == 6) {

            P_SystemService06 nt = (P_SystemService06)entry->ntos_func;
            status = nt(stack[0], stack[1], stack[2], stack[3], stack[4],
                        stack[5]);

        } else if (entry->param_count == 7) {

            P_SystemService07 nt = (P_SystemService07)entry->ntos_func;
            status = nt(stack[0], stack[1], stack[2], stack[3], stack[4],
                        stack[5], stack[6]);

        } else if (entry->param_count == 8) {

            P_SystemService08 nt = (P_SystemService08)entry->ntos_func;
            status = nt(stack[0], stack[1], stack[2], stack[3], stack[4],
                        stack[5], stack[6], stack[7]);

        } else if (entry->param_count == 9) {

            P_SystemService09 nt = (P_SystemService09)entry->ntos_func;
            status = nt(stack[0], stack[1], stack[2], stack[3], stack[4],
                        stack[5], stack[6], stack[7], stack[8]);

        } else if (entry->param_count == 10) {

            P_SystemService10 nt = (P_SystemService10)entry->ntos_func;
            status = nt(stack[0], stack[1], stack[2], stack[3], stack[4],
                        stack[5], stack[6], stack[7], stack[8], stack[9]);

        } else if (entry->param_count == 11) {

            P_SystemService11 nt = (P_SystemService11)entry->ntos_func;
            status = nt(stack[0], stack[1], stack[2], stack[3], stack[4],
                        stack[5], stack[6], stack[7], stack[8], stack[9],
                        stack[10]);

        } else if (entry->param_count == 12) {

            P_SystemService12 nt = (P_SystemService12)entry->ntos_func;
            status = nt(stack[0], stack[1], stack[2], stack[3], stack[4],
                        stack[5], stack[6], stack[7], stack[8], stack[9],
                        stack[10], stack[11]);

        } else if (entry->param_count == 13) {

            P_SystemService13 nt = (P_SystemService13)entry->ntos_func;
            status = nt(stack[0], stack[1], stack[2], stack[3], stack[4],
                        stack[5], stack[6], stack[7], stack[8], stack[9],
                        stack[10], stack[11], stack[12]);

        } else if (entry->param_count == 14) {

            P_SystemService14 nt = (P_SystemService14)entry->ntos_func;
            status = nt(stack[0], stack[1], stack[2], stack[3], stack[4],
                        stack[5], stack[6], stack[7], stack[8], stack[9],
                        stack[10], stack[11], stack[12], stack[13]);

        } else {

            status = STATUS_INVALID_SYSTEM_SERVICE;
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    return status;
}


//---------------------------------------------------------------------------
// Syscall_Api_Invoke
//---------------------------------------------------------------------------


_FX NTSTATUS Syscall_Api_Invoke(PROCESS *proc, ULONG64 *parms)
{
    ULONG_PTR *user_args;
    SYSCALL_ENTRY *entry;
    ULONG syscall_index;
    NTSTATUS status;
#ifdef _WIN64
    volatile ULONG_PTR ret = 0;
    volatile ULONG_PTR UserStack = 0;

    volatile PETHREAD pThread = PsGetCurrentThread();
    volatile PKTRAP_FRAME pTrapFrame = NULL;
#endif
    //int exception;

    //
    // caller must be sandboxed and service index must be known
    //

    if (! proc)
        return STATUS_NOT_IMPLEMENTED;

    syscall_index = (ULONG)parms[1];

    //DbgPrint("[syscall] request for service %d / %08X\n", syscall_index, syscall_index);

    if (Syscall_Table && (syscall_index <= Syscall_MaxIndex))
        entry = Syscall_Table[syscall_index];
    else
        entry = NULL;

    if (! entry)
        return STATUS_INVALID_SYSTEM_SERVICE;

    // DbgPrint("[syscall] request p=%06d t=%06d - BEGIN %s\n", PsGetCurrentProcessId(), PsGetCurrentThreadId(), entry->name);

    //
    // make sure the thread has sufficient access rights to itself
    // then impersonate the full access token for the thread or process
    //

    if (!Thread_AdjustGrantedAccess()) {

        Process_SetTerminated(proc, 5);
    }
    else {

        Thread_SetThreadToken(proc);        // may set proc->terminated
    }

    if (proc->terminated) {

        Process_CancelProcess(proc);
        return STATUS_PROCESS_IS_TERMINATING;
    }

    //
    // if we have a handler for this service, invoke it
    //

    user_args = (ULONG_PTR *)parms[2];

    __try {

        BOOLEAN traced = FALSE;
        const ULONG args_len = entry->param_count * sizeof(ULONG_PTR);
#ifdef _WIN64
        ProbeForRead(user_args, args_len, sizeof(ULONG_PTR));

        // default - support procmon stack if handler3_func_support_procmon is null.
        if (!entry->handler3_func_support_procmon
            || entry->handler3_func_support_procmon(proc, entry, user_args)
            )
        {
            if (g_TrapFrameOffset) {

                pTrapFrame = (PKTRAP_FRAME) *(ULONG_PTR*)((UCHAR*)pThread + g_TrapFrameOffset);
                if (pTrapFrame) {
                    ret = pTrapFrame->Rip;
                    UserStack = pTrapFrame->Rsp;
                    pTrapFrame->Rsp = pTrapFrame->Rbp; //*pRbp;
                    pTrapFrame->Rip = pTrapFrame->Rbx; //*pRbx;
                }
            }
            else
            {
                pTrapFrame = NULL;
            }
        }
        else
        {
            pTrapFrame = NULL;
        }

#else ! _WIN64
        ProbeForRead(user_args, args_len, sizeof(UCHAR));
#endif _WIN64

        
        //if (proc->ipc_trace & (TRACE_ALLOW | TRACE_DENY))
        //{
        //    if (strcmp(entry->name, "AlpcSendWaitReceivePort") == 0)
        //    {
        //        HANDLE  hConnection;
        //        hConnection = (HANDLE*)user_args[0];
        //        DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, "SBIE [syscall] p=%06d t=%06d - %s, handle = %X >>>>>>\n",
        //            PsGetCurrentProcessId(), PsGetCurrentThreadId(),
        //            entry->name,
        //            hConnection);
        //    }
        //}


        if (entry->handler1_func) {

            status = entry->handler1_func(proc, entry, user_args);

        } else {

            status = Syscall_Invoke(entry, user_args);
        }

        // Debug tip. Display all Alpc/Rpc here.

        if (proc->ipc_trace & (TRACE_ALLOW | TRACE_DENY))
        {
            HANDLE  hConnection = NULL;
            UNICODE_STRING* puStr = NULL;

            if ((strcmp(entry->name, "ConnectPort") == 0) ||
                (strcmp(entry->name, "AlpcConnectPort") == 0) )
            {
                hConnection = *(HANDLE*)user_args[0];
                puStr = (UNICODE_STRING*)user_args[1];

                //DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_INFO_LEVEL, "SBIE [syscall] p=%06d t=%06d - %s, '%.*S', status = 0x%X, handle = %X\n",
                //    PsGetCurrentProcessId(), PsGetCurrentThreadId(),
                //    entry->name,
                //    (puStr->Length / 2), puStr->Buffer,
                //    status, hConnection);
                //if (puStr && puStr->Buffer && wcsstr(puStr->Buffer, L"\\RPC Control\\LRPC-"))
                //{
                    //int i = 0;          // place breakpoint here if you want to debug a particular port
                //}
            }
            else if ( (strcmp(entry->name, "AlpcCreatePort") == 0) ||
                (strcmp(entry->name, "AlpcConnectPortEx") == 0) )
            {
                hConnection = *(HANDLE*)user_args[0];
                POBJECT_ATTRIBUTES  pObjectAttributes = (POBJECT_ATTRIBUTES)user_args[1];
                if (pObjectAttributes)
                    puStr = (UNICODE_STRING*)pObjectAttributes->ObjectName;

                //DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_INFO_LEVEL, "SBIE [syscall] p=%06d t=%06d - %s, '%.*S', status = 0x%X, handle = %X\n",
                //    PsGetCurrentProcessId(), PsGetCurrentThreadId(),
                //    entry->name,
                //    (puStr->Length / 2), puStr->Buffer,
                //    status, hConnection);
            }
            else if ((strcmp(entry->name, "ReplyWaitReceivePort") == 0) ||
                (strcmp(entry->name, "ReceiveMessagePort") == 0) ||
                (strcmp(entry->name, "AlpcSendReply") == 0) ||
                (strcmp(entry->name, "AlpcAcceptConnectPort") == 0) ||
                (strcmp(entry->name, "AlpcCreatePortSection") == 0) ||
                // these 2 APIs will generate a lot of output if we don't check status
                ((status != STATUS_SUCCESS) && ((strcmp(entry->name, "AlpcSendWaitReceivePort") == 0) || (strcmp(entry->name, "RequestWaitReplyPort") == 0))) )
            {
                hConnection = (HANDLE*)user_args[0];

                //DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_INFO_LEVEL, "SBIE [syscall] p=%06d t=%06d - %s, status = 0x%X, handle = %X\n",
                //    PsGetCurrentProcessId(), PsGetCurrentThreadId(),
                //    entry->name,
                //    status, hConnection);
            }

            if (hConnection)
            {
                WCHAR trace_str[128];
                RtlStringCbPrintfW(trace_str, sizeof(trace_str), L"[syscall] %.*S, status = 0x%X, handle = %X; ", //59 chars + entry->name
                    max(strlen(entry->name), 64), entry->name,
                    status, hConnection);
                const WCHAR* strings[3] = { trace_str, puStr ? puStr->Buffer : NULL, NULL };
                ULONG lengths[3] = { wcslen(trace_str), puStr ? puStr->Length / 2 : 0, 0 };
                Session_MonitorPutEx(MONITOR_IPC | MONITOR_TRACE, strings, lengths, PsGetCurrentProcessId(), PsGetCurrentThreadId());
                traced = TRUE;
            }
        }

        if (!traced && ((proc->call_trace & TRACE_ALLOW) || ((status != STATUS_SUCCESS) && (proc->call_trace & TRACE_DENY))))
        {
            WCHAR trace_str[128];
            RtlStringCbPrintfW(trace_str, sizeof(trace_str), L"[syscall] %.*S, status = 0x%X", //59 chars + entry->name
                max(strlen(entry->name), 64), entry->name,
                status);
            const WCHAR* strings[2] = { trace_str, NULL };
            Session_MonitorPutEx(MONITOR_SYSCALL | MONITOR_TRACE, strings, NULL, PsGetCurrentProcessId(), PsGetCurrentThreadId());
        }

#ifdef _WIN64
        if (g_TrapFrameOffset) {
            if (pTrapFrame) {
                pTrapFrame->Rip = ret;
                pTrapFrame->Rsp = UserStack;
            }
        }
#endif

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    /*exception = 0;
    if (status != 0 && _wcsicmp(proc->image_name, L"chrome.exe") == 0) {
        exception = 1;
        //if (strstr(entry->name, "Pipe") != 0) exception = 1;
        //if (strstr(entry->name, "FsControl") != 0) exception = 1;
    }
    if (exception) {
        DbgPrint("[syscall] request p=%06d t=%06d - END   (%0X)  %s\n", PsGetCurrentProcessId(), PsGetCurrentThreadId(), status, entry->name);
    }*/

    if (proc->terminated) {

        Process_CancelProcess(proc);
        return STATUS_PROCESS_IS_TERMINATING;
    }

    //
    // clear any thread impersonation set during the syscall, to restore
    // use of the highly restricted primary token in this thread
    //
    // note that in one special case we leave the impersonation token as
    // was set by Thread_SetInformationThread_ChangeNotifyToken, see there
    //

    if (status == STATUS_THREAD_NOT_IN_PROCESS
            && entry == Syscall_SetInformationThread
            && user_args[0] == (ULONG_PTR)NtCurrentThread()) {

        status = STATUS_SUCCESS;

    } else {

        Thread_ClearThreadToken();
    }

    /*if (! NT_SUCCESS(status)) {
        DbgPrint("Process %06d Syscall %04X Status %08X\n", proc->pid, syscall_index, status);
    }*/

    return status;
}


//---------------------------------------------------------------------------
// Syscall_Api_Query
//---------------------------------------------------------------------------


_FX NTSTATUS Syscall_Api_Query(PROCESS *proc, ULONG64 *parms)
{
    ULONG buf_len;
    ULONG *user_buf;
    ULONG *ptr;
    SYSCALL_ENTRY *entry;

    //
    // caller must be our service process
    //

    if (proc)// || (PsGetCurrentProcessId() != Api_ServiceProcessId))
        return STATUS_ACCESS_DENIED;

    //
    // allocate user mode space for syscall table
    //

    buf_len = sizeof(ULONG)         // size of buffer
            + sizeof(ULONG)         // offset to extra data (for SbieSvc)
            + (32 * 4)              // saved code from ntdll
            + List_Count(&Syscall_List) * sizeof(ULONG) * 2
            + sizeof(ULONG) * 2     // final terminator entry
            ;

    user_buf = (ULONG *)parms[1];

    ProbeForWrite(user_buf, buf_len, sizeof(ULONG));

    //
    // populate the buffer with syscall data.  first we store
    // the size of the buffer, and then we leave room for ULONG used
    // by SbieSvc, and for the code for four ZwXxx stub functions
    //

    ptr = user_buf;
    *ptr = buf_len;
    ++ptr;

    *ptr = 0;           // placeholder for offset to extra offset
    ++ptr;

    memcpy(ptr, Syscall_NtdllSavedCode, (32 * 4));
    ptr += (32 * 4) / sizeof(ULONG);

    //
    // store service index number and (only on 32-bit Windows) also
    // the parameter count for each syscall into one ULONG.
    // store corresponding offset within ntdll into the other ULONG
    //

    entry = List_Head(&Syscall_List);
    while (entry) {

        ULONG syscall_index = (ULONG)entry->syscall_index;
#ifndef _WIN64
        ULONG param_count = (ULONG)entry->param_count;
        syscall_index |= (param_count * 4) << 24;
#endif ! _WIN64

        *ptr = syscall_index;
        ++ptr;
        *ptr = entry->ntdll_offset;
        ++ptr;

        entry = List_Next(entry);
    }

    //
    // store a final zero terminator entry and return successfully
    //

    *ptr = 0;
    ++ptr;
    *ptr = 0;

    return STATUS_SUCCESS;
}

_FX BOOLEAN Syscall_QuerySystemInfo_SupportProcmonStack(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args)
{
    BOOLEAN bRet = TRUE;

    // In Win10, case 0xb9 triggers WbCreateWarbirdProcess/WbDispatchOperation/WbSetTrapFrame
    // and PspSetContextThreadInternal (Warbird operation?) to deliver a apc call in the current
    // thread in user mode. Warbird needs the real thread context.

    // It seems only NtQuerySystemInfomation is doing this.
    // Call Syscall_Set3 in Syscall_Init if we see a different syscall does this in the future.

    // It is safe to access the parameter. ProbeForRead has been done by the caller already.
    if (syscall_entry->param_count > 0)
    {
        if (user_args[0] == 0xb9)
        {
            bRet = FALSE;
        }
    }

    return bRet;
}


//---------------------------------------------------------------------------
// 32-bit and 64-bit code
//---------------------------------------------------------------------------


#ifdef _WIN64
#include "syscall_64.c"
#else ! _WIN64
#include "syscall_32.c"
#endif _WIN64

#include "syscall_open.c"
