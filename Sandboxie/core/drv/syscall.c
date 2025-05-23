/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2024 David Xanatos, xanasoft.com
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
#include "conf.h"
#include "common/pattern.h"
#include "core/low/lowdata.h"
#include "dyn_data.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN Syscall_Init_List(void);

static BOOLEAN Syscall_Init_Table(void);

static BOOLEAN Syscall_Init_ServiceData(void);

static void Syscall_ErrorForAsciiName(const UCHAR *name_a);

void Syscall_Update_Config();


//---------------------------------------------------------------------------


static NTSTATUS Syscall_OpenHandle(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args);

static NTSTATUS Syscall_GetNextProcess(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args);

static NTSTATUS Syscall_GetNextThread(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args);

static NTSTATUS Syscall_DeviceIoControlFile(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args);

static NTSTATUS Syscall_DuplicateHandle(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args);

#ifdef _M_AMD64
static BOOLEAN Syscall_QuerySystemInfo_SupportProcmonStack(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args);
#endif


//---------------------------------------------------------------------------


static NTSTATUS Syscall_Api_Query(PROCESS *proc, ULONG64 *parms);

static NTSTATUS Syscall_Api_Invoke(PROCESS *proc, ULONG64 *parms);


//---------------------------------------------------------------------------


static ULONG Syscall_GetIndexFromNtdll(UCHAR *code);

static BOOLEAN Syscall_GetKernelAddr(
    ULONG index, void **pKernelAddr, ULONG *pParamCount);


//---------------------------------------------------------------------------


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, Syscall_Init)
#pragma alloc_text (INIT, Syscall_Init_List)
#pragma alloc_text (INIT, Syscall_Init_Table)
#pragma alloc_text (INIT, Syscall_Init_ServiceData)
#pragma alloc_text (INIT, Syscall_Set1)
#pragma alloc_text (INIT, Syscall_Set2)
#pragma alloc_text (INIT, Syscall_ErrorForAsciiName)
#pragma alloc_text (INIT, Syscall_GetIndexFromNtdll)
#pragma alloc_text (INIT, Syscall_GetKernelAddr)
#pragma alloc_text (INIT, Syscall_GetServiceTable)
#endif // ALLOC_PRAGMA

#include "syscall_util.c"

#ifdef HOOK_WIN32K
#include "syscall_win32.c"
#endif


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

#ifdef HOOK_WIN32K
    if (Driver_OsBuild >= 14393 && Conf_Get_Boolean(NULL, L"EnableWin32kHooks", 0, TRUE)) {

        if (!Syscall_Init_List32())
            return FALSE;

        if (!Syscall_Init_Table32())
            return FALSE;
    }
#endif

    if (! Syscall_Set1("DuplicateObject", Syscall_DuplicateHandle))
        return FALSE;

    if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA) {
        if (!Syscall_Set1("GetNextProcess", Syscall_GetNextProcess))
            return FALSE;

        if (!Syscall_Set1("GetNextThread", Syscall_GetNextThread))
            return FALSE;
    }

    if (!Syscall_Set1("DeviceIoControlFile", Syscall_DeviceIoControlFile))
        return FALSE;

#ifdef _M_AMD64
    if (!Syscall_Set3("QuerySystemInformation", Syscall_QuerySystemInfo_SupportProcmonStack))
        return FALSE;
#endif

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
    BOOLEAN success = FALSE;
    UCHAR *name, *ntdll_code;
    void *ntos_addr;
    DLL_ENTRY *dll;
    SYSCALL_ENTRY *entry;
    ULONG proc_index, proc_offset, syscall_index, param_count;
    ULONG name_len, entry_len;

    List_Init(&Syscall_List);

    //
    // prepare the approve and disabled lists
    //

    LIST disabled_hooks;
    Syscall_LoadHookMap(L"DisableWinNtHook", &disabled_hooks);

    LIST approved_syscalls;
    Syscall_LoadHookMap(L"ApproveWinNtSysCall", &approved_syscalls);

    //
    // scan each ZwXxx export in NTDLL
    //

    dll = Dll_Load(Dll_NTDLL);
    if (! dll)
        goto finish;

    proc_offset = Dll_GetNextProc(dll, "Zw", &name, &proc_index);
    if (! proc_offset)
        goto finish;

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

                                                            ) {
            goto next_zwxxx;
        }

        //
        // on 64-bit Windows, some syscalls are fake, and should be skipped
        //

        if (    IS_PROC_NAME(15, "QuerySystemTime"))
              goto next_zwxxx;


        // ICD-10607 - McAfee uses it to pass its own data in the stack. The call is not important to us. 
        //if (    IS_PROC_NAME(14, "YieldExecution")) // $Workaround$ - 3rd party fix
        //    goto next_zwxxx;

        //
        // the Google Chrome "wow_helper" process expects NtMapViewOfSection
        // to not be already hooked.  although this is needed only on 64-bit
        // Vista, this particular syscall is not very important to us, so
        // for sake of consistency, we skip hooking it on all platforms
        //
        //if (    IS_PROC_NAME(16,  "MapViewOfSection")) // $Workaround$ - 3rd party fix
        //    goto next_zwxxx;

        //if (Syscall_HookMapMatch(name, name_len, &disabled_hooks))
        //    goto next_zwxxx;

#undef IS_PROC_NAME

        //
        // analyze each ZwXxx export to find the service index number
        //

        ntos_addr = NULL;
        param_count = 0;

        ntdll_code = Dll_RvaToAddr(dll, proc_offset);
        if (ntdll_code) {

            syscall_index = Syscall_GetIndexFromNtdll(ntdll_code);

            if (syscall_index == -2) {
                //
                // if ZwXxx export is not a real syscall, then skip it
                //
                goto next_zwxxx;
            }

            if (syscall_index != -1) {
                Syscall_GetKernelAddr(
                            syscall_index, &ntos_addr, &param_count);

                //DbgPrint("    Found SysCall: %s, pcnt %d; idx: %d\r\n", name, param_count, syscall_index);
            }
        }

        if (! ntos_addr) {

            Syscall_ErrorForAsciiName(name);
            goto finish;
        }

        //
        // store an entry for the syscall in our list of syscalls
        //

        entry_len = sizeof(SYSCALL_ENTRY) + name_len + 1;
        entry = Mem_AllocEx(Driver_Pool, entry_len, TRUE);
        if (! entry)
            goto finish;

        entry->syscall_index = (USHORT)syscall_index;
        entry->param_count = (USHORT)param_count;
        entry->ntdll_offset = proc_offset;
        entry->ntos_func = ntos_addr;
        entry->handler1_func = NULL;
        entry->handler2_func = NULL;
#ifdef _M_AMD64
        entry->handler3_func_support_procmon = NULL;
#endif
        entry->disabled = (Syscall_HookMapMatch(name, name_len, &disabled_hooks) != 0);
        entry->approved = (Syscall_HookMapMatch(name, name_len, &approved_syscalls) != 0);
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

    success = TRUE;

    //
    // report an error if we did not find a reasonable number of services
    //

    if (Syscall_MaxIndex < 100) {
        Log_Msg1(MSG_1113, L"100");
        success = FALSE;
    }

    if (Syscall_MaxIndex >= 500) {
        Log_Msg1(MSG_1113, L"500");
        success = FALSE;
    }

finish:

    if(!success)
        Syscall_MaxIndex = 0;

    Syscall_FreeHookMap(&disabled_hooks);

    Syscall_FreeHookMap(&approved_syscalls);

    return success;
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
    UCHAR *NtdllExports[] = NATIVE_FUNCTION_NAMES;
    SYSCALL_ENTRY *entry;
    DLL_ENTRY *dll;
    UCHAR *ntdll_code;
    ULONG i;

    //
    // allocate some space to save code from ntdll
    //

    Syscall_NtdllSavedCode = Mem_AllocEx(Driver_Pool, (NATIVE_FUNCTION_SIZE * NATIVE_FUNCTION_COUNT), TRUE);
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

    for (i = 0; i < NATIVE_FUNCTION_COUNT; ++i) {

        entry = Syscall_GetByName(NtdllExports[i] + 2); // +2 skip Nt prefix
        if (! entry)
            return FALSE;

        ntdll_code = Dll_RvaToAddr(dll, entry->ntdll_offset);
        if (! ntdll_code) {
            Syscall_ErrorForAsciiName(entry->name);
            return FALSE;
        }

        memcpy(Syscall_NtdllSavedCode + (i * NATIVE_FUNCTION_SIZE), ntdll_code, NATIVE_FUNCTION_SIZE);
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

#ifdef _M_AMD64
_FX BOOLEAN Syscall_Set3(const UCHAR *name, P_Syscall_Handler3_Support_Procmon_Stack handler_func)
{
    SYSCALL_ENTRY *entry = Syscall_GetByName(name);
    if (!entry)
        return FALSE;
    entry->handler3_func_support_procmon = handler_func;
    return TRUE;
}
#endif

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


NTSTATUS Sbie_InvokeSyscall_asm(void* func, ULONG count, void* args);

_FX NTSTATUS Syscall_Invoke(SYSCALL_ENTRY *entry, ULONG_PTR *stack)
{
    NTSTATUS status;

    //
    // Note: when directly calling win32k functions with "Core Isolation" (HVCI) enabled
    //  the nt!guard_dispatch_icall will cause a bugcheck!
    //  Hence we use a call proxy Sbie_InvokeSyscall_asm instead of a direct call
    //  alternatively we could disable "Control Flow Guard" for this file
    //

    __try {

        //DbgPrint("[syscall] request param count = %d\n", entry->param_count);

        status = Sbie_InvokeSyscall_asm(entry->ntos_func, entry->param_count, stack);

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
#ifdef _M_AMD64
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

#ifdef HOOK_WIN32K
    if ((syscall_index & 0x1000) != 0) { // win32k syscall
        return Syscall_Api_Invoke32(proc, parms);
    }
#endif

    syscall_index = (syscall_index & 0xFFF);

    //DbgPrint("[syscall] request for service %d / %08X\n", syscall_index, syscall_index);

    //
    // use direct syscalls to access 64 bit memory from a wow process 
    // instead of using heaven's gate / wow64ext
    //

    if (syscall_index == 0xFFF && parms[3] != 0) {
        __try {

            entry = Syscall_GetByName((UCHAR*)parms[3]);

            if(parms[4]) // return found index to the caller to be re used later
                *(USHORT*)parms[4] = entry->syscall_index; 

        } __except (EXCEPTION_EXECUTE_HANDLER) {
            entry = NULL;
        }
    }
    else 
        
    //

    if (Syscall_Table && (syscall_index <= Syscall_MaxIndex))
        entry = Syscall_Table[syscall_index];
    else
        entry = NULL;

    if (! entry)
        return STATUS_INVALID_SYSTEM_SERVICE;

    // DbgPrint("[syscall] request p=%06d t=%06d - BEGIN %s\n", PsGetCurrentProcessId(), PsGetCurrentThreadId(), entry->name);

#ifdef XP_SUPPORT
    //
    // make sure the thread has sufficient access rights to itself
    // then impersonate the full access token for the thread or process
    //

    if (!Thread_AdjustGrantedAccess()) {

        Process_SetTerminated(proc, 5);
    }
    else
#endif
    if(!proc->is_locked_down || entry->approved)
        Thread_SetThreadToken(proc);        // may set proc->terminated // does nothing if !proc->primary_token

    if (proc->terminated) {

        Process_TerminateProcess(proc);
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
#else ! _WIN64
        ProbeForRead(user_args, args_len, sizeof(UCHAR));
#endif _WIN64
#ifdef _M_AMD64
        // default - support procmon stack if handler3_func_support_procmon is null.
        if (!entry->handler3_func_support_procmon
            || entry->handler3_func_support_procmon(proc, entry, user_args)
            )
        {
            // $Offset$
            if (Dyndata_Active && Dyndata_Config.TrapFrame_offset) {

                pTrapFrame = (PKTRAP_FRAME) *(ULONG_PTR*)((UCHAR*)pThread + Dyndata_Config.TrapFrame_offset);
                if (pTrapFrame) {
                    ret = pTrapFrame->Rip;
                    UserStack = pTrapFrame->Rsp;
                    pTrapFrame->Rsp = pTrapFrame->Rdi; //*pRbp;
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
#endif
        
        //if (proc->ipc_trace & (TRACE_ALLOW | TRACE_DENY))
        //{
        //    if (strcmp(entry->name, "AlpcSendWaitReceivePort") == 0)
        //    {
        //        HANDLE  hHandle;
        //        hHandle = (HANDLE*)user_args[0];
        //        DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, "SBIE [syscall] p=%06d t=%06d - %s, handle = %X >>>>>>\n",
        //            PsGetCurrentProcessId(), PsGetCurrentThreadId(),
        //            entry->name,
        //            hHandle);
        //    }
        //}


        if (entry->handler1_func && !proc->open_all_nt) {

            status = entry->handler1_func(proc, entry, user_args);

        } else {

            status = Syscall_Invoke(entry, user_args);
        }

        // Debug tip. Display all Alpc/Rpc here.

        HANDLE  hHandle = NULL;
        UNICODE_STRING* puStr = NULL;

        if (proc->ipc_trace & (TRACE_ALLOW | TRACE_DENY))
        {
            if ((strcmp(entry->name, "ConnectPort") == 0) ||
                (strcmp(entry->name, "AlpcConnectPort") == 0) )
            {
                hHandle = *(HANDLE*)user_args[0];
                puStr = (UNICODE_STRING*)user_args[1];

                //DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_INFO_LEVEL, "SBIE [syscall] p=%06d t=%06d - %s, '%.*S', status = 0x%X, handle = %X\n",
                //    PsGetCurrentProcessId(), PsGetCurrentThreadId(),
                //    entry->name,
                //    (puStr->Length / 2), puStr->Buffer,
                //    status, hHandle);
                //if (puStr && puStr->Buffer && wcsstr(puStr->Buffer, L"\\RPC Control\\LRPC-"))
                //{
                    //int i = 0;          // place breakpoint here if you want to debug a particular port
                //}
            }
            else if ( (strcmp(entry->name, "AlpcCreatePort") == 0) ||
                (strcmp(entry->name, "AlpcConnectPortEx") == 0) )
            {
                hHandle = *(HANDLE*)user_args[0];
                POBJECT_ATTRIBUTES  pObjectAttributes = (POBJECT_ATTRIBUTES)user_args[1];
                if (pObjectAttributes)
                    puStr = (UNICODE_STRING*)pObjectAttributes->ObjectName;

                //DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_INFO_LEVEL, "SBIE [syscall] p=%06d t=%06d - %s, '%.*S', status = 0x%X, handle = %X\n",
                //    PsGetCurrentProcessId(), PsGetCurrentThreadId(),
                //    entry->name,
                //    (puStr->Length / 2), puStr->Buffer,
                //    status, hHandle);
            }
            else if ((strcmp(entry->name, "ReplyWaitReceivePort") == 0) ||
                (strcmp(entry->name, "ReceiveMessagePort") == 0) ||
                (strcmp(entry->name, "AlpcSendReply") == 0) ||
                (strcmp(entry->name, "AlpcAcceptConnectPort") == 0) ||
                (strcmp(entry->name, "AlpcCreatePortSection") == 0) ||
                // these 2 APIs will generate a lot of output if we don't check status
                ((status != STATUS_SUCCESS) && ((strcmp(entry->name, "AlpcSendWaitReceivePort") == 0) || (strcmp(entry->name, "RequestWaitReplyPort") == 0))) )
            {
                hHandle = (HANDLE*)user_args[0];

                //DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_INFO_LEVEL, "SBIE [syscall] p=%06d t=%06d - %s, status = 0x%X, handle = %X\n",
                //    PsGetCurrentProcessId(), PsGetCurrentThreadId(),
                //    entry->name,
                //    status, hHandle);
            }
            else if (strcmp(entry->name, "OpenDirectoryObject") == 0)
            {
                POBJECT_ATTRIBUTES  pObjectAttributes = (POBJECT_ATTRIBUTES)user_args[2];
                if (pObjectAttributes)
                    puStr = (UNICODE_STRING*)pObjectAttributes->ObjectName;
            }
        }

        if (proc->file_trace & (TRACE_ALLOW | TRACE_DENY))
        {
            if (strcmp(entry->name, "QueryFullAttributesFile") == 0)
            {
                POBJECT_ATTRIBUTES  pObjectAttributes = (POBJECT_ATTRIBUTES)user_args[0];
                if (pObjectAttributes)
                    puStr = (UNICODE_STRING*)pObjectAttributes->ObjectName;
            }
            else if (strcmp(entry->name, "QueryInformationFile") == 0)
            {
                hHandle = (HANDLE*)user_args[0];
            }
            else if (strcmp(entry->name, "CreateFile") == 0 || strcmp(entry->name, "OpenFile") == 0)
            {
                hHandle = *(HANDLE*)user_args[0];
                POBJECT_ATTRIBUTES  pObjectAttributes = (POBJECT_ATTRIBUTES)user_args[2];
                if (pObjectAttributes)
                    puStr = (UNICODE_STRING*)pObjectAttributes->ObjectName;
            }
            else if (strcmp(entry->name, "FsControlFile") == 0 || strcmp(entry->name, "Close") == 0)
            {
                hHandle = (HANDLE*)user_args[0];
            }
        }

        if (puStr || hHandle)
        {
            WCHAR trace_str[128];
            if (hHandle) {
                RtlStringCbPrintfW(trace_str, sizeof(trace_str), L"%.*S, status = 0x%X, handle = %X; ", //59 chars + entry->name
                    max(strlen(entry->name), 64), entry->name, status, hHandle);
            }
            else {
                RtlStringCbPrintfW(trace_str, sizeof(trace_str), L"%.*S, status = 0x%X; ", //59 chars + entry->name
                    max(strlen(entry->name), 64), entry->name, status);
            }
            const WCHAR* strings[4] = { trace_str, trace_str + (entry->name_len + 2), puStr ? puStr->Buffer : NULL, NULL };
            ULONG lengths[4] = {entry->name_len, wcslen(trace_str) - (entry->name_len + 4), puStr ? puStr->Length / 2 : 0, 0 };
            Session_MonitorPutEx(MONITOR_SYSCALL | (entry->approved ? MONITOR_OPEN : MONITOR_TRACE), 
                strings, lengths, PsGetCurrentProcessId(), PsGetCurrentThreadId());

            traced = TRUE;
        }

        if (!traced && ((proc->call_trace & TRACE_ALLOW) || ((status != STATUS_SUCCESS) && (proc->call_trace & TRACE_DENY))))
        {
            // Suppress Sbie's own calls to DeviceIoControlFile
            if ((strcmp(entry->name, "DeviceIoControlFile") != 0) || user_args[5] != API_SBIEDRV_CTLCODE)
            {
                WCHAR trace_str[128];
                RtlStringCbPrintfW(trace_str, sizeof(trace_str), L"%.*S, status = 0x%X", //59 chars + entry->name
                    max(strlen(entry->name), 64), entry->name,
                    status);
                const WCHAR* strings[3] = { trace_str, trace_str + (entry->name_len + 2), NULL };
                ULONG lengths[3] = { entry->name_len, wcslen(trace_str) - (entry->name_len + 2), 0 };
                Session_MonitorPutEx(MONITOR_SYSCALL | (entry->approved ? MONITOR_OPEN : MONITOR_TRACE),
                    strings, lengths, PsGetCurrentProcessId(), PsGetCurrentThreadId());
            }
        }

#ifdef _M_AMD64
        if (Dyndata_Active && Dyndata_Config.TrapFrame_offset) {
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

        Process_TerminateProcess(proc);
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

    } else if(proc->primary_token) {

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

#ifdef HOOK_WIN32K
    if (parms[2] == 1) { // 1 - win32k
        return Syscall_Api_Query32(proc, parms);
    }
    else if (parms[2] != 0) { // 0 - ntoskrnl
        return STATUS_INVALID_PARAMETER;
    }
#endif

    BOOLEAN add_names = parms[3] != 0;

    //
    // caller must be our service process
    //

    if (proc || (PsGetCurrentProcessId() != Api_ServiceProcessId))
        return STATUS_ACCESS_DENIED;

    //
    // allocate user mode space for syscall table
    //

    buf_len = sizeof(ULONG)         // size of buffer
            + sizeof(ULONG)         // offset to extra data (for SbieSvc)
            + (NATIVE_FUNCTION_SIZE * NATIVE_FUNCTION_COUNT) // saved code from ntdll
            + List_Count(&Syscall_List) * ((sizeof(ULONG) * 2) + (add_names ? 64 : 0))
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

    memcpy(ptr, Syscall_NtdllSavedCode, (NATIVE_FUNCTION_SIZE * NATIVE_FUNCTION_COUNT));
    ptr += (NATIVE_FUNCTION_SIZE * NATIVE_FUNCTION_COUNT) / sizeof(ULONG);

    //
    // store service index number and (only on 32-bit Windows) also
    // the parameter count for each syscall into one ULONG.
    // store corresponding offset within ntdll into the other ULONG
    //

    for (entry = List_Head(&Syscall_List); entry; entry = List_Next(entry)) {

        if (entry->disabled)
            continue;

        ULONG syscall_index = (ULONG)entry->syscall_index;
#ifndef _WIN64
        ULONG param_count = (ULONG)entry->param_count;
        syscall_index |= (param_count * 4) << 24;
#endif ! _WIN64

        *ptr = syscall_index;
        ++ptr;
        *ptr = entry->ntdll_offset;
        ++ptr;
        if (add_names) {
            memcpy(ptr, entry->name, entry->name_len);
            ((char*)ptr)[entry->name_len] = 0;
            ptr += 16; // 16 * sizeog(ULONG) = 64
        }
    }

    //
    // store a final zero terminator entry and return successfully
    //

    *ptr = 0;
    ++ptr;
    *ptr = 0;

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Syscall_Update_Config
//---------------------------------------------------------------------------


_FX void Syscall_Update_Config()
{
    SYSCALL_ENTRY *entry;

#ifdef HOOK_WIN32K
    Syscall_Update_Config32();
#endif

    LIST disabled_hooks;
    Syscall_LoadHookMap(L"DisableWinNtHook", &disabled_hooks);

    LIST approved_syscalls;
    Syscall_LoadHookMap(L"ApproveWinNtSysCall", &approved_syscalls);

    entry = List_Head(&Syscall_List);
    while (entry) {

        entry->disabled = (Syscall_HookMapMatch(entry->name, entry->name_len, &disabled_hooks) != 0);
        entry->approved = (Syscall_HookMapMatch(entry->name, entry->name_len, &approved_syscalls) != 0);

        entry = List_Next(entry);
    }

    Syscall_FreeHookMap(&disabled_hooks);

    Syscall_FreeHookMap(&approved_syscalls);
}


//---------------------------------------------------------------------------
// Syscall_QuerySystemInfo_SupportProcmonStack
//---------------------------------------------------------------------------

#ifdef _M_AMD64
_FX BOOLEAN Syscall_QuerySystemInfo_SupportProcmonStack(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args)
{
    BOOLEAN bRet = TRUE;

    // In Win10, case 0xb9 triggers WbCreateWarbirdProcess/WbDispatchOperation/WbSetTrapFrame
    // and PspSetContextThreadInternal (Warbird operation?) to deliver a apc call in the current
    // thread in user mode. Warbird needs the real thread context.
    // https://github.com/xpn/warbird_exploit
    // this exploit only works on x86 windows but can still crash a x64 one

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
#endif

//---------------------------------------------------------------------------
// 32-bit and 64-bit code
//---------------------------------------------------------------------------


#ifdef _WIN64
#include "syscall_64.c"
#else ! _WIN64
#include "syscall_32.c"
#endif _WIN64

#include "syscall_open.c"
