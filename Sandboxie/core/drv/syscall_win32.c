/*
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


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN Syscall_Init_List32(void);

static BOOLEAN Syscall_Init_Table32(void);

void Syscall_Update_Config32();


//---------------------------------------------------------------------------


static NTSTATUS Syscall_Api_Query32(PROCESS *proc, ULONG64 *parms);

static NTSTATUS Syscall_Api_Invoke32(PROCESS *proc, ULONG64 *parms);


//---------------------------------------------------------------------------


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, Syscall_Init_List32)
#pragma alloc_text (INIT, Syscall_Init_Table32)
#endif // ALLOC_PRAGMA



//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static LIST Syscall_List32;

static SYSCALL_ENTRY **Syscall_Table32 = NULL;

ULONG Syscall_MaxIndex32 = 0;




//---------------------------------------------------------------------------
// Syscall_GetWin32kAddr
//---------------------------------------------------------------------------

//struct _EX_FAST_REF
//{
//    void *Object;
//    unsigned long RefCnt:3;
//    unsigned long Value;
//};
//
//struct _EX_CALLBACK
//{
//    struct _EX_FAST_REF RoutineBlock;
//};
//
//struct _PS_WIN32K_GLOBALS
//{
//    struct _EX_CALLBACK Win32CallBack;
//    struct _KSERVICE_TABLE_DESCRIPTOR ServiceDescriptorTableShadow[2];
//    struct _KSERVICE_TABLE_DESCRIPTOR ServiceDescriptorTableFilter[2];
//};

typedef struct _KSERVICE_TABLE_DESCRIPTOR
{
    unsigned long *Base;
    unsigned long *Reserved1;
    unsigned long Limit;
    unsigned char *Number;
} KSERVICE_TABLE_DESCRIPTOR, *PKSERVICE_TABLE_DESCRIPTOR ;

#ifdef _WIN64
_FX BOOLEAN Syscall_GetWin32kAddr(KSERVICE_TABLE_DESCRIPTOR *ShadowTable, ULONG *Base_Copy, 
    ULONG index, void **pKernelAddr, ULONG *pParamCount)
{
    ULONG MaxSyscallIndexPlusOne = ShadowTable->Limit;
    if ((index >= 0x1000) &&
                ((index & 0xFFF) < MaxSyscallIndexPlusOne)) {

        long EntryValue = Base_Copy[index & 0xFFF];

        *pKernelAddr = (UCHAR *)ShadowTable->Base + (EntryValue >> 4);
#ifdef _M_ARM64
        *pParamCount = (ULONG)(EntryValue & 0x0F) + 8;
#else
        *pParamCount = (ULONG)(EntryValue & 0x0F) + 4;
#endif
        //DbgPrint("    SysCall32 offset: %d\r\n", (ULONG)(EntryValue >> 4));
        return TRUE;
    }

    Log_Msg1(MSG_1113, L"ADDRESS");
    return FALSE;
}
#else
_FX BOOLEAN Syscall_GetWin32kAddr(KSERVICE_TABLE_DESCRIPTOR *ShadowTable, ULONG *ProcTable, UCHAR *ParmTable,
    ULONG index, void **pKernelAddr, ULONG *pParamCount)
{
    ULONG MaxSyscallIndexPlusOne = ShadowTable->Limit;
    if ((index >= 0x1000) &&
                ((index & 0xFFF) < MaxSyscallIndexPlusOne)) {

        *pKernelAddr = (void *)ProcTable[index & 0xFFF];
        *pParamCount = ((ULONG)ParmTable[index & 0xFFF]) / 4;
        //DbgPrint("    SysCall32 offset: %d\r\n", (ULONG)(EntryValue >> 4));
        return TRUE;
    }

    Log_Msg1(MSG_1113, L"ADDRESS");
    return FALSE;
}
#endif

//---------------------------------------------------------------------------
// Syscall_Init_List32
//---------------------------------------------------------------------------


_FX BOOLEAN Syscall_Init_List32(void)
{
    BOOLEAN success = FALSE;
    UCHAR *name, *win32k_code;
    void *ntos_addr;
    DLL_ENTRY *dll;
    SYSCALL_ENTRY *entry;
    ULONG proc_index, proc_offset, syscall_index, param_count;
    ULONG name_len, entry_len;
    ULONG* base_copy = NULL;
#ifndef _WIN64
    UCHAR* number_copy = NULL;
#endif

    List_Init(&Syscall_List32);

    //
    // prepare the enabled/disabled lists
    //

    LIST enabled_hooks;
    LIST disabled_hooks;
    Syscall_LoadHookMap(L"EnableWin32Hook", &enabled_hooks);
    Syscall_LoadHookMap(L"DisableWin32Hook", &disabled_hooks);

    LIST approved_syscalls;
    Syscall_LoadHookMap(L"ApproveWin32SysCall", &approved_syscalls);

    BOOLEAN ignore_hook_blacklist = Conf_Get_Boolean(NULL, L"IgnoreWin32HookBlacklist", 0, FALSE);

    //
    // get the syscall table
    //

    KSERVICE_TABLE_DESCRIPTOR *ShadowTable = (KSERVICE_TABLE_DESCRIPTOR *)Syscall_GetServiceTable();
    if (!ShadowTable) {
        Log_Msg1(MSG_1113, L"SHADOW_TABLE");
        goto finish;
    }
    //DbgPrint(" ntoskrln.exe SysCalls: %d %p %p\n", ShadowTable->Limit, ShadowTable->Base, ShadowTable->Number);
    ShadowTable += 1; // ServiceDescriptorTableShadow[0] -> ntoskrnl.exe, ServiceDescriptorTableShadow[1] -> win32k.sys
    //DbgPrint(" win32k.sys SysCalls: %d %p %p\n", ShadowTable->Limit, ShadowTable->Base, ShadowTable->Number);

    if (ShadowTable->Limit > 0xFFF) { // not plausible
        Log_Msg1(MSG_1113, L"SHADOW_TABLE");
        goto finish;
    }
    
    //
    // We can not read ShadowTable->Base bemory, without being in a GUI thread
    // hence we grab csrss.exe and attach to it, create a copy of this memory
    // and use it in the loop below instead
    //

    HANDLE csrssId = Util_GetProcessPidByName(L"csrss.exe");
    if (csrssId == (HANDLE)-1) {
        Log_Msg1(MSG_1113, L"csrss.exe");
        goto finish;
    }

    base_copy = (ULONG*)Mem_AllocEx(Driver_Pool, ShadowTable->Limit * sizeof(long), TRUE);
    if (!base_copy)
        goto finish;
#ifndef _WIN64
    number_copy = (UCHAR*)Mem_AllocEx(Driver_Pool, ShadowTable->Limit * sizeof(char), TRUE);
    if (!number_copy)
        goto finish;
#endif

    PEPROCESS ProcessObject;
    if (NT_SUCCESS(PsLookupProcessByProcessId(csrssId,&ProcessObject))) {
        KAPC_STATE ApcState;
        KeStackAttachProcess(ProcessObject, &ApcState);
        if (MmIsAddressValid(ShadowTable->Base)
#ifndef _WIN64
             && MmIsAddressValid(ShadowTable->Number)
#endif
        ) {
            memcpy(base_copy, ShadowTable->Base, ShadowTable->Limit * sizeof(long));
#ifndef _WIN64
            memcpy(number_copy, ShadowTable->Number, ShadowTable->Limit * sizeof(char));
#endif
            success = TRUE;
        }
        KeUnstackDetachProcess(&ApcState);
        ObDereferenceObject(ProcessObject);
    }

    if (!success) {
        Log_Msg1(MSG_1113, L"WIN32K_TABLE");
        goto finish;
    }
    success = FALSE;

    //
    // scan each NtXxx export in WIN32U
    //

    dll = Dll_Load(Dll_WIN32U);
    if (!dll) 
        goto finish;

    proc_offset = Dll_GetNextProc(dll, "Nt", &name, &proc_index);
    if (! proc_offset)
        goto finish;

    while (proc_offset) {

        if (name[0] != 'N' || name[1] != 't')
            break;
        name += 2;                  // skip Nt prefix
        for (name_len = 0; (name_len < 64) && name[name_len]; ++name_len)
            ;
        
        //DbgPrint("    Found SysCall32 %s\n", name);

        entry = NULL;

        //
        // we don't hook UserCreateWindowEx as it uses callbacks into
        // user space from the kernel, for once this does not play well 
        // with our system call interface, but it would also be a security issue
        // to allow user code execution while we have restored the original token
        //
        // also some hooks cause BSOD's for now we just blacklist them
        //

        #define IS_PROC_NAME(ln,nm) (name_len == ln && memcmp(name, nm, ln) == 0)

        if (!ignore_hook_blacklist)
        if (    IS_PROC_NAME(18, "UserCreateWindowEx")

            ||  IS_PROC_NAME( 7, "GdiInit") // bsod
            ||  IS_PROC_NAME(12, "GdiInitSpool") // probably too

            ||  IS_PROC_NAME(27, "UserSetProcessWindowStation") // bsod
            ||  IS_PROC_NAME(33, "UserSetProcessDpiAwarenessContext") // bsod
            ||  IS_PROC_NAME(20, "UserSetThreadDesktop") // bsod
                                                            ) {

            //DbgPrint("    Win32k Hook disabled for %s (blacklisted)\n", name);
            goto next_ntxxx;
        }

        #define IS_PROC_PREFIX(ln,nm) (name_len >= ln && memcmp(name, nm, ln) == 0)

        //
        // Chrome and msedge need GdiDdDDI to be hooked in order for 
        // the HW acceleration to work.
        //

        BOOLEAN install_hook = IS_PROC_PREFIX(8, "GdiDdDDI");

        if (!Syscall_TestHookMap(name, name_len, &enabled_hooks, &disabled_hooks, install_hook)) {
            //DbgPrint("    Win32k Hook disabled for %s\n", name);
            goto next_ntxxx;
        }
        //DbgPrint("    Win32k Hook enabled for %s\n", name);

#undef IS_PROC_NAME
#undef IS_PROC_PREFIX

        //
        // analyze each NtXxx export to find the service index number
        //

        ntos_addr = NULL;
        param_count = 0;

        win32k_code = Dll_RvaToAddr(dll, proc_offset);
        if (win32k_code) {

            syscall_index = Syscall_GetIndexFromNtdll(win32k_code);

            if (syscall_index == -2) {
                //
                // if ZwXxx export is not a real syscall, then skip it
                //
                goto next_ntxxx;
            }

            if (syscall_index != -1) {

                Syscall_GetWin32kAddr(ShadowTable, base_copy,
#ifndef _WIN64
                                                               number_copy,
#endif
                            syscall_index, &ntos_addr, &param_count);

                //BOOLEAN test;
                //KeStackAttachProcess(pProcess, &ApcState);
                //test = MmIsAddressValid(ntos_addr);
                //KeUnstackDetachProcess(&ApcState);
                //DbgPrint("    Found SysCall32: %s, pcnt %d; idx: %d; addr: %p %s\r\n", name, param_count, syscall_index, ntos_addr, test ? "valid" : "invalid");
                //DbgPrint("    Found SysCall32: %s, pcnt %d; idx: %d; addr: %p\r\n", name, param_count, syscall_index, ntos_addr);
            }
        }

        if (! ntos_addr) {
            Syscall_ErrorForAsciiName(name);
            goto next_ntxxx;
            //goto finish;   
        }

        syscall_index = (syscall_index & 0xFFF);

        //
        // store an entry for the syscall in our list of syscalls
        //

        entry_len = sizeof(SYSCALL_ENTRY) + name_len + 1;
        entry = Mem_AllocEx(Driver_Pool, entry_len, TRUE);
        if (!entry)
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
        entry->approved = (Syscall_HookMapMatch(name, name_len, &approved_syscalls) != 0);
        entry->name_len = (USHORT)name_len;
        memcpy(entry->name, name, name_len);
        entry->name[name_len] = '\0';

        List_Insert_After(&Syscall_List32, NULL, entry);

        if (syscall_index > Syscall_MaxIndex32)
            Syscall_MaxIndex32 = syscall_index;

        //
        // process next ZwXxx export
        //
        
next_ntxxx:

        proc_offset = Dll_GetNextProc(dll, NULL, &name, &proc_index);

    }

    success = TRUE;

    //DbgPrint("Found %d win32 SysCalls\n", Syscall_MaxIndex32);

    //
    // report an error if we did not find a reasonable number of services
    //

    if (Syscall_MaxIndex32 < 100) {
        Log_Msg1(MSG_1113, L"100");
        success = FALSE;
    }

    if (Syscall_MaxIndex32 >= 2000) {
        Log_Msg1(MSG_1113, L"2000");
        success = FALSE;
    }

finish:

    if(!success)
        Syscall_MaxIndex32 = 0;

    Syscall_FreeHookMap(&enabled_hooks);
    Syscall_FreeHookMap(&disabled_hooks);

    Syscall_FreeHookMap(&approved_syscalls);

    if (base_copy)
        Mem_Free(base_copy, ShadowTable->Limit * sizeof(long));
#ifndef _WIN64
    if (number_copy)
        Mem_Free(number_copy, ShadowTable->Limit * sizeof(char));
#endif

    return success;
}


//---------------------------------------------------------------------------
// Syscall_Init_Table32
//---------------------------------------------------------------------------


_FX BOOLEAN Syscall_Init_Table32(void)
{
    SYSCALL_ENTRY *entry;
    ULONG len, i;

    //
    // build the table which maps syscall index numbers to entries
    //

    len = sizeof(SYSCALL_ENTRY *) * (Syscall_MaxIndex32 + 1);
    Syscall_Table32 = Mem_AllocEx(Driver_Pool, len, TRUE);
    if (! Syscall_Table32)
        return FALSE;

    for (i = 0; i <= Syscall_MaxIndex32; ++i) {

        entry = List_Head(&Syscall_List32);
        while (entry) {
            if (entry->syscall_index == i)
                break;
            entry = List_Next(entry);
        }

        Syscall_Table32[i] = entry;
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Syscall_Api_Invoke32
//---------------------------------------------------------------------------


_FX NTSTATUS Syscall_Api_Invoke32(PROCESS* proc, ULONG64* parms)
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

    //
    // caller must be sandboxed and service index must be known
    //

    if (! proc)
        return STATUS_NOT_IMPLEMENTED;

    syscall_index = (ULONG)parms[1];

    if ((syscall_index & 0x1000) == 0) 
        return STATUS_INVALID_SYSTEM_SERVICE;

    syscall_index = (syscall_index & 0xFFF);

    //DbgPrint("[syscall32] request for service %d / %08X\n", syscall_index | 0x1000, syscall_index | 0x1000);

    if (Syscall_Table32 && (syscall_index <= Syscall_MaxIndex32))
        entry = Syscall_Table32[syscall_index];
    else
        entry = NULL;

    if (! entry)
        return STATUS_INVALID_SYSTEM_SERVICE;

    //DbgPrint("[syscall] request %s\n", entry->name);

    // DbgPrint("[syscall] request p=%06d t=%06d - BEGIN %s\n", PsGetCurrentProcessId(), PsGetCurrentThreadId(), entry->name);

    if(!proc->is_locked_down || entry->approved)
        Thread_SetThreadToken(proc);        // may set proc->terminated // does nothing if !proc->primary_token

//    if (proc->terminated) {
//
//        Process_TerminateProcess(proc);
//        return STATUS_PROCESS_IS_TERMINATING;
//    }


    //
    // on first win32k.sys call we must convert this thread to a GUI thread
    //

    // todo: call KiConvertToGuiThread() or PsConvertToGuiThread()

    // note: once this is implemented the below check with MmIsAddressValid will be obsolete



    //
    // if we have a handler for this service, invoke it
    //

    user_args = (ULONG_PTR *)parms[2];

    __try {

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

        if (entry->handler1_func && !proc->open_all_nt) {

            status = entry->handler1_func(proc, entry, user_args);

        } else {

            //
            // we must validate the address as an application without being switched to
            // gui mode could still issue a syscall to a win32k and cause a BSOD otherwise
            //

            if (MmIsAddressValid(entry->ntos_func)) {

                //DbgPrint("   SysCall32 %d -> %p\r\n", syscall_index, entry->ntos_func);
                status = Syscall_Invoke(entry, user_args);

            } else {

                status = STATUS_INVALID_ADDRESS;

            }
        }

        if ((proc->call_trace & TRACE_ALLOW) || ((status != STATUS_SUCCESS) && (proc->call_trace & TRACE_DENY)))
        {
            WCHAR trace_str[128];
            RtlStringCbPrintfW(trace_str, sizeof(trace_str), L"%.*S, status = 0x%X", //59 chars + entry->name
                max(strlen(entry->name), 64), entry->name,
                status);
            const WCHAR* strings[3] = { trace_str, trace_str + (entry->name_len + 2), NULL };
            ULONG lengths[3] = {entry->name_len, wcslen(trace_str) - (entry->name_len + 2), 0 };
            Session_MonitorPutEx(MONITOR_SYSCALL | (entry->approved ? MONITOR_OPEN : MONITOR_TRACE), 
                strings, lengths, PsGetCurrentProcessId(), PsGetCurrentThreadId());
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


//    if (proc->terminated) {
//
//        Process_TerminateProcess(proc);
//        return STATUS_PROCESS_IS_TERMINATING;
//    }

    //
    // clear any thread impersonation set during the syscall, to restore
    // use of the highly restricted primary token in this thread
    //

    if(proc->primary_token)
        Thread_ClearThreadToken();

    /*if (! NT_SUCCESS(status)) {
        DbgPrint("Process %06d Syscall %04X Status %08X\n", proc->pid, syscall_index, status);
    }*/

    return status;
}


//---------------------------------------------------------------------------
// Syscall_Api_Query32
//---------------------------------------------------------------------------


_FX NTSTATUS Syscall_Api_Query32(PROCESS *proc, ULONG64 *parms)
{
    ULONG buf_len;
    ULONG *user_buf;
    ULONG *ptr;
    SYSCALL_ENTRY *entry;

    if (Syscall_MaxIndex32 == 0)
        return STATUS_NOT_IMPLEMENTED;

    BOOLEAN add_names = parms[3] != 0;

    //
    // allocate user mode space for syscall table
    //

    buf_len = sizeof(ULONG)         // size of buffer
            + List_Count(&Syscall_List32) * ((sizeof(ULONG) * 2) + (add_names ? 64 : 0))
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

    //
    // store service index number and (only on 32-bit Windows) also
    // the parameter count for each syscall into one ULONG.
    // store corresponding offset within ntdll into the other ULONG
    //

    entry = List_Head(&Syscall_List32);
    while (entry) {

        ULONG syscall_index = (ULONG)entry->syscall_index | 0x1000;
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

//---------------------------------------------------------------------------
// Syscall_Update_Config32
//---------------------------------------------------------------------------


_FX void Syscall_Update_Config32()
{
    SYSCALL_ENTRY *entry;

    LIST approved_syscalls;
    Syscall_LoadHookMap(L"ApproveWin32SysCall", &approved_syscalls);

    entry = List_Head(&Syscall_List32);
    while (entry) {

        entry->approved = (Syscall_HookMapMatch(entry->name, entry->name_len, &approved_syscalls) != 0);

        entry = List_Next(entry);
    }

    Syscall_FreeHookMap(&approved_syscalls);
}