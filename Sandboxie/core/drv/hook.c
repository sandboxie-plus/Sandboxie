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
// Hook Management
//---------------------------------------------------------------------------


#define HOOK_WITH_PRIVATE_PARTS
#include "hook.h"
#include "dll.h"

#include "util.h"
#define KERNEL_MODE
#include "../dll/hook_inst.c"
#include "../dll/hook_tramp.c"

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define NUM_TARGET_PAGES    4

#define MAX_DEPTH           10


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static LONG Hook_FindSyscall2(
    void *addr, int depth, ULONG_PTR *targets, BOOLEAN check_target,
    LONG eax, LONG *skip);
static LONG Hook_CheckSkippedSyscall(LONG eax, LONG *skip);


//---------------------------------------------------------------------------


#pragma alloc_text (INIT, Hook_GetServiceIndex)
#pragma alloc_text (INIT, Hook_FindSyscall2)


//---------------------------------------------------------------------------
// Hook_GetService
//---------------------------------------------------------------------------


_FX BOOLEAN Hook_GetService(
    void *DllProc, LONG *SkipIndexes, ULONG ParamCount,
    void **NtService, void **ZwService)
{
    LONG svc_num;
    void *svc_addr;

    if (NtService)
        *NtService = NULL;
    if (ZwService)
        *ZwService = NULL;

    if ((ULONG_PTR)DllProc < 0x10000) {

        svc_num = (ULONG)(ULONG_PTR)DllProc;

    } else {

        svc_num = Hook_GetServiceIndex(DllProc, SkipIndexes);
        if (svc_num == -1)
            return FALSE;
    }

    svc_addr = Hook_GetNtServiceInternal(svc_num, ParamCount);
    if (! svc_addr)
        return FALSE;

    if (NtService)
        *NtService = svc_addr;

    if (ZwService) {
        svc_addr = Hook_GetZwServiceInternal(svc_num);
        if (! svc_addr)
            return FALSE;
        *ZwService = svc_addr;
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Hook_GetServiceIndex
//---------------------------------------------------------------------------


_FX LONG Hook_GetServiceIndex(void *DllProc, LONG *SkipIndexes)
{
    static LONG SkipIndexes_default[1] = { 0 };
    ULONG_PTR *targets;
    LONG eax;

    if (! SkipIndexes)
        SkipIndexes = SkipIndexes_default;

    targets = (ULONG_PTR *)ExAllocatePoolWithTag(
        PagedPool, PAGE_SIZE * NUM_TARGET_PAGES, tzuk);
    if (! targets) {
        Log_Msg0(MSG_1104);
        return -1;
    }

    targets[0] = 0;
    eax = Hook_FindSyscall2(DllProc, 1, targets, TRUE, -1, SkipIndexes);

    ExFreePoolWithTag(targets, tzuk);

    if ((eax != -1) && (eax < 0x2000) && ((eax & 0xFFF) < 0x600))
        return eax;

    return -1;
}


//---------------------------------------------------------------------------
// Hook_FindSyscall2
//---------------------------------------------------------------------------


_FX LONG Hook_FindSyscall2(
    void *addr, int depth, ULONG_PTR *targets, BOOLEAN check_target,
    LONG eax, LONG *skip)
{
    HOOK_INST inst;
    ULONG edx;
    ULONG i;
    BOOLEAN is64;
    ULONG count;

#ifdef _WIN64           // 64-bit
    is64 = TRUE;
#else                   // 32-bit
    is64 = FALSE;
#endif                  // xx-bit

    if (depth > MAX_DEPTH)
        return -1;

    //
    // for each start address, we record it in our list of jump targets,
    // so we can ignore it the next time we have to process that address.
    //
    // note that check_target is FALSE when we enter a recursive call
    // after processing "call edx" or "call [edx]", see below.  in this
    // case we don't check jump targets because we want to re-analyse
    // the sequence leading to SYSENTER every time
    //

#ifndef _WIN64

    //
    // if the target is near the system limit, then it is probably
    // SharedUserData and we want to process it every time.
    // (again see below for processing of call edx)
    //

    if (check_target) {

        if (((ULONG)addr & 0xF0000000) >= 0x70000000)
            check_target = FALSE;
    }

#endif ! _WIN64

    if (check_target) {

        for (i = 0; i < (ULONG)targets[0]; ++i)
            if (targets[i] == (ULONG_PTR)addr)
                return -1;
        ++i;
        if (i > (PAGE_SIZE * NUM_TARGET_PAGES / sizeof(ULONG_PTR) - 8))
            return -1;
        targets[i] = (ULONG_PTR)addr;
        targets[0] = (ULONG)i;
    }

    check_target = TRUE;

    //
    // analyze instructions beginning at the passed address
    //

    for (count = 0; count < 1024; ++count) {

        BOOLEAN ok = Hook_Analyze(addr, TRUE, is64, &inst);
        // DbgPrint("%04d  At Address %p Ok=%d inst=%02X%02X inst.kind=%d parm=%p\n", depth, addr, ok, inst.op1, inst.op2, inst.kind, (void *)inst.parm);
        if ((! ok) || inst.kind == INST_RET || inst.kind == INST_JUMP_MEM)
            return -1;

        if (inst.kind == INST_SYSCALL) {
            eax = Hook_CheckSkippedSyscall(eax, skip);
            if (eax != -1)
                return eax;
        }

#ifndef _WIN64

        if (inst.kind == INST_CTLXFER_REG) {

            //
            // if this instruction is "call edx" or "call [edx]", and
            // last instruction we processed was "mov edx, imm32", then
            // branch to the following address.  (this is used in 32-bit
            // Windows code to jump to SharedUserData!SystemCallStub).
            //

            if (inst.op1 == 0xFF && inst.op2 == 0xD2) {

                inst.kind = INST_CTLXFER;
                inst.parm = edx;
                check_target = FALSE;

            } else if (inst.op1 == 0xFF && inst.op2 == 0x12 &&
                       ((ULONG)edx & 0xF0000000) >= 0x70000000) {

                __try {

                    ProbeForRead((ULONG *)edx, sizeof(ULONG), sizeof(ULONG));
                    inst.parm = *(ULONG *)edx;
                    inst.kind = INST_CTLXFER;
                    check_target = FALSE;

                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    inst.parm = 0;
                }
                if (! inst.parm)
                    return -1;

            } else
                return -1;
        }

#endif // ! _WIN64

        if (inst.kind == INST_CTLXFER) {

            /*
            if (((ULONG_PTR)inst.parm & 0xFFF00000) == 0x7FF && hookdbg) {
                //DbgPrint("%04d  Address %p Call %p\n", depth, addr, (void *)inst.parm);
            }
            */

            eax = Hook_FindSyscall2(
                (void *)inst.parm, depth + 1,
                targets, check_target,
                eax, skip);

            check_target = TRUE;

            eax = Hook_CheckSkippedSyscall(eax, skip);
            if (eax != -1)
                return eax;

            if (inst.op1 == 0xE9 ||     // if it was a jump, then stop
                inst.op1 == 0xEB)
                return -1;
        }

        edx = 0;                        // reset edx on every cycle

        if (inst.op1 == 0xB8)           // mov eax, imm32
            eax = (LONG)inst.parm;

        else if (inst.op1 == 0xBA)      // mov edx, imm32
            edx = (ULONG)inst.parm;

        addr = ((UCHAR *)addr) + inst.len;
    }

    return -1;
}


//---------------------------------------------------------------------------
// Hook_CheckSkippedSyscall
//---------------------------------------------------------------------------


_FX LONG Hook_CheckSkippedSyscall(LONG eax, LONG *skip)
{
    LONG i;
    for (i = 0; i < skip[0]; ++i)
        if (eax == skip[i + 1])
            return -1;
    return eax;
}


//---------------------------------------------------------------------------
// Hook_Api_Tramp
//---------------------------------------------------------------------------


_FX NTSTATUS Hook_Api_Tramp(PROCESS *proc, ULONG64 *parms)
{
    NTSTATUS status;
    void *Source;
    void *Trampoline;
    BOOLEAN is64;

    if (! proc)
        return STATUS_NOT_IMPLEMENTED;

    // Source should point to the beginning of a function

    Source = (void *)parms[1];
    if (! Source)
        return STATUS_INVALID_PARAMETER;

    // Trampoline is expected to point to a 96-byte writable buffer,
    // aligned on a 16-byte boundary.

    Trampoline = (void *)parms[2];
    if (! Trampoline)
        return STATUS_INVALID_PARAMETER;
    ProbeForWrite(Trampoline, 96 /* sizeof(HOOK_TRAMP) */, 16);

    //
    // build the trampoline
    //

#ifdef _WIN64
    is64 = ! IoIs32bitProcess(NULL);
#else
    is64 = FALSE;
#endif _WIN64

    if (Hook_BuildTramp(Source, Trampoline, is64, TRUE))
        status = STATUS_SUCCESS;
    else
        status = STATUS_UNSUCCESSFUL;

    return status;
}


//---------------------------------------------------------------------------
// 32-bit and 64-bit code
//---------------------------------------------------------------------------


#ifdef _WIN64
#include "hook_64.c"
#else ! _WIN64
#include "hook_32.c"
#endif _WIN64