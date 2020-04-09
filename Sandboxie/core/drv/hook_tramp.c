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
// Hook Management:  Trampoline Construction
//---------------------------------------------------------------------------


#define HOOK_WITH_PRIVATE_PARTS
#include "hook.h"
#include "util.h"

BOOLEAN File_TrusteerLoaded(void);

//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _HOOK_TRAMP_PAGE {

    LIST_ELEM list_elem;
    UCHAR *mapped_base;
    UCHAR *next_tramp;

} HOOK_TRAMP_PAGE;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static void *Hook_Tramp_Get(ULONG TrampSize);

static BOOLEAN Hook_Tramp_CountBytes(
    void *SysProc, ULONG *ByteCount, BOOLEAN is64, BOOLEAN probe);

static BOOLEAN Hook_Tramp_Copy(
    HOOK_TRAMP *tramp, void *SourceFunc,
    ULONG ByteCount, BOOLEAN is64, BOOLEAN probe);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static LIST Hook_Tramp_Pages;
static BOOLEAN Hook_Tramp_Pages_Initialized = FALSE;


//---------------------------------------------------------------------------
// Hook_Tramp_Get
//---------------------------------------------------------------------------


_FX void *Hook_Tramp_Get(ULONG TrampSize)
{
    NTSTATUS status;
    HOOK_TRAMP_PAGE *page;
    void *tramp;

    if (! Hook_Tramp_Pages_Initialized) {
        List_Init(&Hook_Tramp_Pages);
        Hook_Tramp_Pages_Initialized = TRUE;
    }

    page = List_Head(&Hook_Tramp_Pages);
    while (page) {
        if (page->next_tramp < page->mapped_base + 0xF00)
            break;
        page = List_Next(page);
    }

    if (! page) {

        //
        // small pool allocations come from a shared pool of memory
        // where we can't assume ownership on the pages and can't
        // change their permissions, so we want to allocate whole pages
        //

        ULONG err;
        ULONG_PTR addr;
        PMDL mdl;

        page = ExAllocatePoolWithTag(PagedPool, PAGE_SIZE, tzuk);
        if (! page) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            err = 0x11;
            goto finish;
        }

        memzero(page, sizeof(HOOK_TRAMP_PAGE));

        mdl = IoAllocateMdl(page, PAGE_SIZE, FALSE, FALSE, NULL);
        if (! mdl) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            err = 0x12;
            goto finish;
        }

        __try {
            MmProbeAndLockPages(mdl, KernelMode, IoModifyAccess);
            status = STATUS_SUCCESS;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
        }
        if (! NT_SUCCESS(status)) {
            err = 0x13;
            goto finish;
        }

        page->mapped_base = MmMapLockedPagesSpecifyCache(
            mdl, KernelMode, MmCached, NULL, FALSE, NormalPagePriority);
        if (! page->mapped_base) {
            status = STATUS_UNSUCCESSFUL;
            err = 0x14;
            goto finish;
        }

#ifdef _WIN64

        status = MmProtectMdlSystemAddress(mdl, PAGE_EXECUTE_READWRITE);
        if (! NT_SUCCESS(status)) {
            err = 0x15;
            goto finish;
        }

#endif _WIN64

        addr = (ULONG_PTR)page->mapped_base + sizeof(HOOK_TRAMP_PAGE);
        addr = (addr + 15) & ~0x0F;
        page->next_tramp = (UCHAR *)addr;

        List_Insert_After(&Hook_Tramp_Pages, NULL, page);

        err = 0;

finish:
        if (err) {

            Log_Status(MSG_HOOK_TRAMP_GET, err, status);
            page = NULL;
        }
    }

    if (page) {

        tramp = page->next_tramp;

        TrampSize = (TrampSize + 15) & ~0x0F;
        page->next_tramp = ((UCHAR *)tramp) + TrampSize;

        *(ULONG *)tramp = tzuk;

    } else
        tramp = NULL;

    return tramp;
}


//---------------------------------------------------------------------------
// Hook_Tramp_CountBytes
//---------------------------------------------------------------------------


_FX BOOLEAN Hook_Tramp_CountBytes(
    void *SysProc, ULONG *ByteCount, BOOLEAN is64, BOOLEAN probe)
{
    UCHAR *addr = (UCHAR *)SysProc;
    ULONG needlen = (is64 == 9 ? 13 : (is64 ? 12 : (File_TrusteerLoaded()?6:5)));
    ULONG copylen = 0;

    // count at least the (needlen) bytes of instructions from the original
    // entry point to our stub, as we will overwrite that area later

    while (1) {

        HOOK_INST inst;
        BOOLEAN ok = Hook_Analyze(addr, probe, is64, &inst);
        if (! ok)
            return FALSE;

        if (inst.op1 == 0xFF && inst.op2 == 0x25
                                        && *(ULONG *)&addr[2] == 0) {
            // jmp dword/qword ptr [+00], so skip the following ULONG_PTR
            inst.len += sizeof(ULONG_PTR);
        }

        copylen += inst.len;
        if (copylen >= needlen)
            break;

        addr += inst.len;
    }

    *ByteCount = copylen;
    return TRUE;
}


//---------------------------------------------------------------------------
// Hook_Tramp_Copy
//---------------------------------------------------------------------------


_FX BOOLEAN Hook_Tramp_Copy(
    HOOK_TRAMP *tramp, void *SourceFunc,
    ULONG ByteCount, BOOLEAN is64, BOOLEAN probe)
{
    UCHAR *src = (UCHAR *)SourceFunc;
    UCHAR *code = tramp->code;
    ULONG code_len;
    BOOLEAN push_pop_rax = FALSE;

    tramp->eyecatcher = tzuk;
    tramp->target = src + ByteCount;

    // copy ByteCount bytes from the original source function into
    // the code area of the trampoline stub, adjustmenting it as needed
    // in 32-bit mode, we also relocate call/jump targets. (E8/E9)
    // in 64-bit mode, we also relocate rip-based displacements

    while ((ULONG_PTR)(src - (UCHAR *)SourceFunc) < ByteCount) {

        HOOK_INST inst;
        BOOLEAN ok = Hook_Analyze(src, probe, is64, &inst);
        if (! ok)
            return FALSE;

        memcpy(code, src, inst.len);
        code_len = inst.len;

        if ((inst.op1 >= 0x70 && inst.op1 <= 0x7F) ||
            (inst.op1 == 0x0F && (inst.op2 >= 0x80 && inst.op2 <= 0x8F))) {

            // conditional-jmp with 8-bit or 32-bit displacement

            UCHAR cond;
            if (inst.op1 == 0x0F)
                cond = inst.op2 & 0x0F;
            else
                cond = inst.op1 & 0x0F;

            if (is64) {

                // in 64-bit mode, rewrite as indirect jmp/call:
                //           jcc cont (opposite condition)
                //           jmp qword ptr [target64]
                // target64: dq  target
                // (cont):   ...

                code[0] = 0x70 | (cond ^ 1);    // jcc cont (rip+16)
                code[1] = 14;
                code[2] = 0xFF;                 // jmp [rip+6]
                code[3] = 0x25;
                *(LONG *)&code[4] = 0;
                *(ULONG_PTR *)&code[8] = (ULONG_PTR)inst.parm;

                code_len = 16;

            } else {

                // in 32-bit mode, fix jmp target relative to new eip,
                // and write out jcc disp32

                code[0] = 0x0F;                 // jcc disp32
                code[1] = 0x80 | cond;
                *(ULONG *)&code[2] =
                    (ULONG)(inst.parm - ((ULONG_PTR)code + 6));

                code_len = 6;
            }

        } else if (inst.op1 == 0xEB) {

            // unconditional short jump with 8-bit displacement

            if (is64) {

                // in 64-bit mode, rewrite as indirect jmp/call:
                //           jmp qword ptr [target64]
                // target64: dq  target

                code[0] = 0xFF;                 // jmp [rip+6]
                code[1] = 0x25;
                *(LONG *)&code[2] = 0;
                *(ULONG_PTR *)&code[6] = (ULONG_PTR)inst.parm;

                code_len = 14;

            } else {

                // in 32-bit mode, fix jmp target relative to new eip,
                // and write out jcc disp32

                code[0] = 0xE9;                 // jmp disp32
                *(ULONG *)&code[1] =
                    (ULONG)(inst.parm - ((ULONG_PTR)code + 5));

                code_len = 5;
            }

        } else if (inst.op1 == 0xE8 || inst.op1 == 0xE9) {

            // simple direct jmp/call with 32-bit displacement

            if (is64) {

                // in 64-bit mode, rewrite as indirect jmp/call:
                //           jmp qword ptr [target64]
                //           jmp cont
                // target64: dq  target
                // cont:     ...

                code[0] = 0xFF;         // jmp/call [rip+8]
                code[1] = (inst.op1 == 0xE8) ? 0x15 : 0x25;
                *(LONG *)&code[2] = 2;
                code[6] = 0xEB;         // jmp cont (rip+8)
                code[7] = 0x08;
                *(ULONG_PTR *)&code[8] = (ULONG_PTR)inst.parm;

                code_len = 16;

            } else {

                // in 32-bit mode, fix jmp target relative to new eip

                ULONG *rel32 = (ULONG *)(code + ((UCHAR *)inst.rel32 - src));
                *rel32 = (ULONG)(inst.parm - ((ULONG_PTR)code + inst.len));
            }

        } else if (inst.op1 == 0xFF && inst.op2 == 0x25 &&
                        *(ULONG *)&src[2] == 0) {

            //
            // indirect jmp to the immediately following address:
            //      jmp dword/qword ptr [+00]
            // typically found when the entry point is already hooked
            // by someone else.  we retrieve the jump target from the
            // instruction and use it for the trampoline jump target,
            // then break
            //

            if (is64)
                tramp->target = (void *)(*(ULONG64 *)&src[6]);
            else
                tramp->target = (void *)(ULONG_PTR)(*(ULONG *)&src[6]);

            break;

        } else if (is64 && inst.modrm && (inst.modrm[0] & 0xC7) == 5) {

            // RIP-relative addressing in 64-bit mode, when the
            // instruction contains modrm byte: mod==00b, rm==101b.
            // (high two bits are zero, low three bits have a value of 5)

            UCHAR *modrm;

            // we rewrite the original instruction to use RAX as the
            // base, rather than RIP, and prefix an instruction that
            // loads RAX with the original RIP.

            if (push_pop_rax) {
                *code = 0x50;           // push rax
                ++code;
            }

            code[0] = 0x48;             // mov rax, addr
            code[1] = 0xB8;
            *(ULONG_PTR *)&code[2] = ((ULONG_PTR)src) + inst.len;
            code += 10;

            memcpy(code, src, inst.len);
            modrm = code + (inst.modrm - src);
            *modrm = (*modrm | 0x80) & (~7);

            if (push_pop_rax) {
                code += inst.len;
                *code = 0x58;           // pop rax
                ++code;
                // we already advanced the code pointer to insert the pop
                // instruction so we don't want to advance it again:
                code_len = 0;
            }

        } else if (is64 && inst.op1 == 0x8B && inst.op2 == 0xC4) {

            // if rax is used in the prolog, we will need to push and
            // pop it before using it in the code generated above for
            // RIP-relative addressing in 64-bit mode

            push_pop_rax = TRUE;
        }

        src += inst.len;
        code += code_len;
    }

    // stub ends with a jump to the original entrypoint.  first qword
    // in the stub contains the address of the original entrypoint

    code[0] = 0xFF;                     // jmp dword/qword ptr [target]
    code[1] = 0x25;
    code += 2;
    if (is64) {
        *(LONG *)code = (LONG)((UCHAR *)&tramp->target - (code + 4));
        code += 8;
    } else {
        *(ULONG *)code = (ULONG)(ULONG_PTR)&tramp->target;
        code += 4;
    }

    code_len = (ULONG)(code - (UCHAR *)tramp);
    code_len = (code_len + 15) & (~0x0F);   // align to 16-byte boundary
    tramp->size = code_len;

    return TRUE;
}


//---------------------------------------------------------------------------
// Hook_BuildTramp
//---------------------------------------------------------------------------


_FX void *Hook_BuildTramp(
    void *SourceFunc, void *Trampoline, BOOLEAN is64, BOOLEAN probe)
{
    ULONG ByteCount;
    HOOK_TRAMP *tramp;

    if (SourceFunc) {
        if (! Hook_Tramp_CountBytes(SourceFunc, &ByteCount, is64, probe))
            return NULL;
    }

    if (Trampoline)
        tramp = (HOOK_TRAMP *)Trampoline;
    else {
        tramp = (HOOK_TRAMP *)Hook_Tramp_Get(sizeof(HOOK_TRAMP));
        if (! tramp)
            return NULL;
    }

    if (SourceFunc) {
        if (! Hook_Tramp_Copy(tramp, SourceFunc, ByteCount, is64, probe))
            return NULL;
    }

    return &tramp->code;
}


//---------------------------------------------------------------------------
// Hook_BuildJump
//---------------------------------------------------------------------------


_FX void Hook_BuildJump(
    void *WritableAddr, void *ExecutableAddr, void *JumpTarget)
{
    UCHAR *SourceAddr = (UCHAR *)WritableAddr;

    //
    // ideally, WritableAddr points at a writable page received through
    // MmGetSystemAddressForMdlSafe for the page at ExecutableAddr.
    // but in case it isn't and WritableAddr == ExecutableAddr then
    // we disable the CR0-WP bit.
    //

    DisableWriteProtect();

    //
    // if we detect JMP DWORD/QWORD PTR [+00], then replace the jump target
    //

    if (SourceAddr[0] == 0xFF && SourceAddr[1] == 0x25 &&
            *(ULONG *)&SourceAddr[2] == 0) {

        *(ULONG_PTR *)(SourceAddr + 6) = (ULONG_PTR)JumpTarget;

    } else {

        //
        // otherwise normal processing.  the cli instruction at the top
        // makes sure that no exceptions can be scheduled while running
        // the replacement code.  this is required on 64-bit because we
        // are probably overwriting the function epilog, and the replaced
        // code is different than the unwind (xdata) info associated with
        // the original function.  the target function should re-enable
        // interrupts, see Process_BuildHookEntry
        //

#ifdef _WIN64   // 64-bit

        SourceAddr[0] = 0xFA;           // cli
        SourceAddr[1] = 0x48;           // mov rax, my_func
        SourceAddr[2] = 0xB8;
        *(ULONG_PTR *)&SourceAddr[3] = (ULONG_PTR)JumpTarget;
        SourceAddr[11] = 0xFF;          // jmp rax
        SourceAddr[12] = 0xE0;

#else           // 32-bit

        if (SourceAddr[0] == 0x68 && SourceAddr[5] == 0xC3) {

            //
            // Rising Antivirus hooks by pointing SSDT to this stub:
            //      PUSH routine_address        0x68 xx xx xx xx
            //      RET                         0xC3
            // When unloading, overwrites "routine_address" with original
            // address from SSDT.  Thus if we hook by replacing PUSH with
            // JMP, we get a crash when it unloads.  To prevent that, we
            // use the same PUSH/RET method to transfer control
            //

            *(ULONG *)&SourceAddr[1] = (ULONG)JumpTarget;

        } else {

            SourceAddr[0] = 0xE9;           // JMP [addr]
            *(ULONG *)&SourceAddr[1] =
                (ULONG)((UCHAR *)JumpTarget - (UCHAR *)ExecutableAddr - 5);
        }

#endif          // 32-bit or 64-bit

    }

    EnableWriteProtect();
}
