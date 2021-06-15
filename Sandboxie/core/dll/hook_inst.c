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
// Hook Management:  Instruction Analyzer
//---------------------------------------------------------------------------


#ifndef KERNEL_MODE
#include "dll.h"
#include "msgs/msgs.h"
#define MSG_HOOK_ANALYZE    MSG_1151
#define Log_Msg1			SbieApi_Log
#define HOOK_WITH_PRIVATE_PARTS
#include "hook.h"
#endif


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define _F_DATA_SIZE(f)     (((f) & 0xFF))
#define _F_ADDR_SIZE(f)     (((f) & 0xFFFF) >> 8)
#define _F_REX(f)           (((f) & 0xFFFFFF) >> 16)
#define F_DATA_SIZE()       (_F_DATA_SIZE(inst->flags))
#define F_ADDR_SIZE()       (_F_ADDR_SIZE(inst->flags))
#define F_REX()             (_F_REX(inst->flags))
#define F_64_BIT            (0xF0 << 16)

#define F_MAKE_COMBO(dataSize, addrSize, rex)   \
    (                                           \
        (((dataSize) & 0xFF)) |                 \
        (((addrSize) & 0xFF) << 8) |            \
        (((rex) & 0xFF) << 16)                  \
    )

#define F_REG(f)            ((f & 0x38) >> 3)


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static UCHAR *Hook_Analyze_Inst(UCHAR *addr, HOOK_INST *inst);

static UCHAR *Hook_Analyze_CtlXfer(UCHAR *addr, HOOK_INST *inst);

static UCHAR *Hook_Analyze_ModRM(
    UCHAR *modrm, HOOK_INST *inst);

static UCHAR *Hook_Analyze_Prefix(
    UCHAR *addr, BOOLEAN is64, ULONG *flags);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


/* Legend: */
/* 0 - invalid or unhandled opcode, or handled outside of this table */
/* 1 - opcode, modrm, optional disp */
/* 2 - opcode, immediate 8 */
/* 3 - opcode, immediate 16/32 */
/* 4 - opcode, modrm, immediate 8 */
/* 5 - opcode, modrm, immediate 16/32 */
/* 6 - opcode, nothing else */
/* 7 - opcode, address 32/64 (selected by F_ADDR_SIZE) */

static const UCHAR Hook_Opcodes_1[256] = {

            /*   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
        /* 0 */  1, 1, 1, 1, 2, 3, 6, 6, 1, 1, 1, 1, 2, 3, 6, 0,
        /* 1 */  1, 1, 1, 1, 2, 3, 6, 6, 1, 1, 1, 1, 2, 3, 6, 6,
        /* 2 */  1, 1, 1, 1, 2, 3, 0, 6, 1, 1, 1, 1, 2, 3, 0, 6,
        /* 3 */  1, 1, 1, 1, 2, 3, 0, 6, 1, 1, 1, 1, 2, 3, 0, 6,
        /* 4 */  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        /* 5 */  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        /* 6 */  6, 6, 1, 1, 1, 0, 0, 0, 3, 0, 2, 0, 6, 6, 6, 6,
        /* 7 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 8 */  4, 5, 4, 4, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 1,
        /* 9 */  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 6, 6, 6, 6, 6,
        /* A */  7, 7, 7, 7, 6, 6, 6, 6, 2, 3, 6, 6, 6, 6, 6, 6,
        /* B */  2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
        /* C */  4, 4, 3, 6, 0, 0, 4, 5, 0, 6, 3, 6, 6, 2, 6, 6,
        /* D */  1, 1, 1, 1, 2, 2, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0,
        /* E */  2, 2, 2, 2, 2, 2, 2, 2, 3, 0, 0, 0, 6, 6, 6, 6,
        /* F */  0, 0, 0, 0, 6, 6, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0
};

// arithmetic 0x80..0x83:  three middle bits of op2 decide the operation:
//      000=add  001=or   010=adc  011=sbb
//      100=and  101=sub  110=xor  111=cmp

// shifts 0xC0..0xC1, 0xD0..0xD3: three middle bits decide:
//      000=rol  001=ror  010=rcl  011=rcr
//      100=shl  101=shr  110=???  111=sar

// extended 0xFF:  three middle bits of op2 decide the operation:
//      000=inc  001=dec  010=call 011=call
//      100=jmp  101=jmp  110=push 111=n/a


//---------------------------------------------------------------------------
// Hook_Analyze
//---------------------------------------------------------------------------


ALIGNED BOOLEAN Hook_Analyze(
    void *address,
    BOOLEAN probe_address,
    BOOLEAN is64,
    HOOK_INST *inst)
{
    UCHAR *addr;
    WCHAR text[64];

    memzero(inst, sizeof(HOOK_INST));

    __try {

#ifdef KERNEL_MODE
        if (probe_address)
            ProbeForRead(address, 16, sizeof(UCHAR));
#endif

        /*if (1) {
            UCHAR *z = (UCHAR *)address;
            DbgPrint("%p - %02X %02X %02X %02X %02X %02X %02X %02X\n", z, z[0], z[1], z[2], z[3], z[4], z[5], z[6], z[7]);
            z += 8;
            DbgPrint("%p - %02X %02X %02X %02X %02X %02X %02X %02X\n", z, z[0], z[1], z[2], z[3], z[4], z[5], z[6], z[7]);
        }*/
        // DbgPrint("*** Analyzing pfx  at %08X\n", address);
        addr = Hook_Analyze_Prefix(address, is64, &inst->flags);
        inst->op1 = addr[0];
        inst->op2 = addr[1];
        // DbgPrint("    Analyzing inst at %08X\n", addr);
        addr = Hook_Analyze_Inst(addr, inst);
        if (! addr) {
            addr = address;
#ifdef KERNEL_MODE
			RtlStringCbPrintfW(text, 64,
#else
			Sbie_snwprintf(text, 64, 
#endif
				L"%08p:  %02X,%02X,%02X,%02X,"
                L"%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X", addr,
                addr[0],  addr[1],  addr[2],  addr[3],  addr[4],  addr[5],
                addr[6],  addr[7],  addr[8],  addr[9],  addr[10], addr[11]);
            Log_Msg1(MSG_HOOK_ANALYZE, text);
            addr = NULL;
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        addr = NULL;
#ifdef KERNEL_MODE
		RtlStringCbPrintfW(text, 64,
#else
		Sbie_snwprintf(text, 64,
#endif
			L"(fault at %p)", address);
        Log_Msg1(MSG_HOOK_ANALYZE, text);

    }

    if (! addr)
        return FALSE;

    inst->len = (ULONG)(addr - (UCHAR *)address);
    return TRUE;
}


//---------------------------------------------------------------------------
// Hook_Analyze_Inst
//---------------------------------------------------------------------------


ALIGNED UCHAR *Hook_Analyze_Inst(UCHAR *addr, HOOK_INST *inst)
{
    UCHAR *addr2;
    UCHAR op, op2;

    addr2 = Hook_Analyze_CtlXfer(addr, inst);
    if (addr2 != addr)
        return addr2;

    op = addr[0];
    op2 = addr[1];

    switch (Hook_Opcodes_1[op]) {

        case 1:
            return Hook_Analyze_ModRM(addr + 1, inst);

        case 2:
            return addr + 2;

        case 3:
            if (F_DATA_SIZE() == 8) {
                inst->parm = *(ULONG64 *)(addr + 1);
                return addr + 1 + 8;
            }
            if (F_DATA_SIZE() == 4) {
                inst->parm = (ULONG64)*(ULONG *)(addr + 1);
                return addr + 1 + 4;
            }
            if (F_DATA_SIZE() == 2) {
                inst->parm = (ULONG64)*(USHORT *)(addr + 1);
                return addr + 1 + 2;
            }
            break;

        case 4:
            return Hook_Analyze_ModRM(addr + 1, inst) + 1;

        case 5:
            return Hook_Analyze_ModRM(addr + 1, inst) + F_DATA_SIZE();

        case 6:
            return addr + 1;

        case 7:
            return addr + 1 + F_ADDR_SIZE();
    }

    // special opcodes

    if (op == 0xF6 || op == 0xF7) {
        if (F_REG(op2) == 0) {          // F6/F7 test opcode
            if (op == 0xF6)             // 8-bit
                return Hook_Analyze_ModRM(addr + 1, inst) + 1;
            else                        // 16/32-bit
                return Hook_Analyze_ModRM(addr + 1, inst) + F_DATA_SIZE();
        } else {
            // div, idiv, mul, imul, neg, not
            return Hook_Analyze_ModRM(addr + 1, inst);
        }
    }

    // double-byte opcodes escaped by FE

    if (op == 0xFE) {

        if (F_REG(op2) == 0 || F_REG(op2) == 1)         // inc/dec
            return Hook_Analyze_ModRM(addr + 1, inst);

    }

    // double-byte opcodes escaped by FF

    if (op == 0xFF) {

        if (F_REG(op2) == 0 || F_REG(op2) == 1 ||       // inc/dec
            F_REG(op2) == 6)                            // push
            return Hook_Analyze_ModRM(addr + 1, inst);

    }

    // double-byte opcodes escaped by 0F

    if (op == 0x0F) {

        if ((op2 >= 0x19 && op2 <= 0x1F) ||     // nop r/m
            (op2 >= 0x40 && op2 <= 0x4F) ||     // cmovxx r/m
            (op2 >= 0x90 && op2 <= 0x9F))       // setxx r/m
            return Hook_Analyze_ModRM(addr + 2, inst);

        switch (op2) {

            case 0x10:                  // movups xmm?
                return addr + 4;
            case 0x57:                  // xorps
                return addr + 3;
            case 0xA0:                  // push fs
            case 0xA8:                  // push gs
                return addr + 2;

            case 0xA5:                  // shld r/m
            case 0xAD:                  // shrd r/m
            case 0xAF:                  // imul r/m
            case 0xB6:                  // movzx r/m
            case 0xB7:                  // movzx r/m
            case 0xBE:                  // movsx r/m
            case 0xBF:                  // movsx r/m
            case 0xA3:                  // bt   r/m
            case 0xAB:                  // bts  r/m
            case 0xB3:                  // btr  r/m
            case 0xBB:                  // btc  r/m
                return Hook_Analyze_ModRM(addr + 2, inst);

            case 0xBA:                  // bt, bts, btr, btc imm8
            case 0xA4:                  // shld r/m, imm8
            case 0xAC:                  // shrd r/m, imm8
                return Hook_Analyze_ModRM(addr + 2, inst) + 1;
        }

        // special instruction

        // DbgPrint("Inst = %02X/%02X\n", op, op2);

        if (op2 == 0x00 || op2 == 0x01) {       // 0F 00/01

            UCHAR op3 = addr[2];
            op3 = ((op2 & 1) << 7) | F_REG(op3);

            //DbgPrint("Addr[] = %02X/%02X --> %02X\n", op2, addr[2], op3);

            if (op3 == 0x00 ||                          // sldt r/m
                op3 == 0x01 ||                          // str r/m
                op3 == 0x80 ||                          // sgdt mem
                op3 == 0x81) {                          // sidt mem

                return Hook_Analyze_ModRM(addr + 2, inst);
            }
        }

    }

    // unhandled opcode

    return NULL;
}


//---------------------------------------------------------------------------
// Hook_Analyze_CtlXfer
//---------------------------------------------------------------------------


ALIGNED UCHAR *Hook_Analyze_CtlXfer(UCHAR *addr, HOOK_INST *inst)
{
    UCHAR op1, op2;
    LONG rel32;
    BOOLEAN have_rel32;

    op1 = addr[0];
    op2 = addr[1];
    rel32 = 0;
    have_rel32 = FALSE;

    // system control transfers

    if ((op1 == 0x0F && op2 == 0x05) ||                 // syscall
        (op1 == 0x0F && op2 == 0x34) ||                 // sysenter
        (op1 == 0xCD && op2 == 0x2E)) {                 // int 2e

        inst->kind = INST_SYSCALL;
        addr += 2;
    }

    if (op1 == 0xCC) {
        inst->kind = INST_RET;
        ++addr;
    }

    // return control transfer

    if (op1 == 0xC3 || op1 == 0xCB ||                   // ret, retf
        op1 == 0xC2 || op1 == 0xCA) {                   // ret, retf nnn

        inst->kind = INST_RET;
        if (op1 & 1)
            ++addr;
        else
            addr += 3;
    }

    // short jumps with an 8-bit displacement

    if ((op1 >= 0x70 && op1 <= 0x7F) ||                 // jcc disp8
        (op1 >= 0xE0 && op1 <= 0xE3) ||                 // loop/jcxz disp8
        (op1 == 0xEB)) {                                // jmp disp8

        have_rel32 = TRUE;
        rel32 = (LONG)*(CHAR *)(addr + 1);
        addr += 2;
    }

    // control transfer to a near 32-bit displacement

    if (op1 == 0x0F && (op2 & 0xF0) == 0x80) {          // jcc disp32

        have_rel32 = TRUE;
        rel32 = *(LONG *)(addr + 2);
        addr += 6;
    }

    if (op1 == 0xE8 || op1 == 0xE9) {                   // jmp/call disp32
        have_rel32 = TRUE;
        inst->rel32 = (LONG *)(addr + 1);
        rel32 = *inst->rel32;
        addr += 5;
    }

    // double-byte instructions

    if (op1 == 0xFF) {

        // control transfer using a register, no displacement

        if ((op2 >= 0xD0 && op2 <= 0xD7) ||             // call reg
            (op2 >= 0xE0 && op2 <= 0xE7)) {             // jmp reg

            inst->kind = INST_CTLXFER_REG;
            inst->parm = op2 & 0x0F;
            addr += 2;

        } else if (op2 == 0x12) {

            // control transfer to [edx]

            inst->kind = INST_CTLXFER_REG;
            inst->parm = 0x80 | (op2 & 0x0F);
            addr += 2;

        } else {

            // control transfer using a modrm field

            if ((op2 & 0x38) == 0x10 ||                     // call
                (op2 & 0x38) == 0x18 ||                     // call
                (op2 & 0x38) == 0x20 ||                     // jmp
                (op2 & 0x38) == 0x28) {                     // jmp

                inst->kind = (op2 >= 0x20) ? INST_JUMP_MEM : INST_CALL_MEM;
                addr = Hook_Analyze_ModRM(addr + 1, inst);
            }
        }

    }

    // far jump through call gate to absolute address

    if (op1 == 0xEA) {

        USHORT seg = *(USHORT *)(addr + 5);
        if (seg == 0x001B || seg == 0x0023) {   // application CS == 0x001B
            inst->kind = INST_CTLXFER;          // on 64-bit, also 0023 ?
            inst->parm = *(ULONG *)(addr + 1);
            addr += 7;
        }
    }

    // fix relative target address

    if (have_rel32) {
        inst->kind = INST_CTLXFER;
        inst->parm = (ULONG_PTR)addr + rel32;
    }

    return addr;
}


//---------------------------------------------------------------------------
// Hook_Analyze_ModRM
//---------------------------------------------------------------------------


ALIGNED UCHAR *Hook_Analyze_ModRM(UCHAR *modrm, HOOK_INST *inst)
{
    UCHAR disp;

    // if opcode implies a modrm field, we need to skip one or
    // two bytes of modrm field, and possibly additional one
    // or four bytes of displacement

    inst->modrm = modrm;
    disp = 0xFF;                // indicate no displacement

    // mod == 11b:  one-byte modrm, no displacements
    if ((modrm[0] & 0xC0) == 0xC0)
        ++modrm;                // skip one byte of modrm

    // (mod != 11b) && (rm == 100b) && (! 16-bit):  two-byte modrm.
    // displacement is governed by mod and base fields,
    // where base is low 3 bits of second modrm byte (aka SIB).
    // interpretation of mod+base is same as that of
    // mod+rm for purposes of deciding size of displacement
    else if ((F_ADDR_SIZE() != 2) && (modrm[0] & 7) == 4) {
        disp = (modrm[0] & 0xC0) | (modrm[1] & 7);
        modrm += 2;             // skip two bytes of modrm

    // (mod != 11b) && (rm != 100b):  one-byte modrm.
    // displacement is governed by mod+rm fields, see
    // interpretation below
    } else {
        disp = (modrm[0] & 0xC0) | (modrm[0] & 7);
        ++modrm;                // skip one byte of modrm
    }

    // displacement is one byte if mod == 01b,
    // four bytes if mod == 10b (option 1), or
    // four bytes if mod == 00b and rm/base == 101b (option 2)
    if (disp != 0xFF) {
        if ((disp & 0xC0) == 0x40)
            ++modrm;            // skip 8-bit displacement
        else if ((disp & 0xC0) == 0x80 ||
                 (disp & 7) == 5)
            modrm += 4;         // skip 32-bit displacement
    }

    return modrm;
}


//---------------------------------------------------------------------------
// Hook_Analyze_Prefix
//---------------------------------------------------------------------------


ALIGNED UCHAR *Hook_Analyze_Prefix(UCHAR *addr, BOOLEAN is64, ULONG *flags)
{
    ULONG rex, osize, asize;
    UCHAR b;

    rex = 0;
    osize = 4;
    asize = is64 ? 8 : 4;

    while (1) {

        b = *addr;
        ++addr;

        if (b == 0x2E ||                // CS: override prefix 2E
            b == 0x3E ||                // DS: override prefix 3E
            b == 0x26 ||                // ES: override prefix 26
            b == 0x64 ||                // FS: override prefix 64
            b == 0x65 ||                // GS: override prefix 65
            b == 0x36 ||                // SS: override prefix 36
            b == 0xF0 ||                // LOCK prefix
            b == 0xF3 ||                // REP or REPZ prefix
            b == 0xF2)                  // REPNZ prefix
            continue;

        if (b == 0x66) {                // Operand-size override prefix 66
            osize = 2;
            continue;
        }

        if (b == 0x67) {                // Address-size override prefix 67
            asize = is64 ? 4 : 2;
            continue;
        }

        if (is64 && (b & 0xF0) == 0x40) {   // REX prefix 4x
            rex = b & 0x0F;
            continue;
        }

        --addr;                         // Not a prefix byte
        break;
    }

    if (is64 && (rex & 8))              // REX.W prefix
        osize = 8;

    // in 64-bit mode, 64-bits of addr or data are usually sign-extended
    // from only 32-bits encoded in the instruction.  only in two cases
    // an instruction actually encodes full 64-bits of information:
    //
    // - moving to/from accum register:                 opcodes A0..A3
    //   - this is the default, unless overriden
    //     by prefix 67
    //
    // - moving immediate value to any register:        opcodes B8..BF
    //   - prefix 48, REX.W, must be specified for this

    if (asize == 8 && (b < 0xA0 || b > 0xA3))
        asize = 4;
    if (osize == 8 && (b < 0xB8 || b > 0xBF))
        osize = 4;

    // encode prefix information into the flags

    *flags = F_MAKE_COMBO(osize, asize, rex);
    if (is64)
        *flags |= F_64_BIT;

    return addr;
}
