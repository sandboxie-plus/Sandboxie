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


#ifndef _MY_HOOK_H
#define _MY_HOOK_H


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


#ifdef HOOK_WITH_PRIVATE_PARTS


enum HOOK_KIND {

    INST_UNKNOWN    = 0,
    INST_MOVE,
    INST_CTLXFER,           // jmp/jcc/call with 32-bit disp
    INST_CTLXFER_REG,       // jmp/call reg or [reg]
    INST_CALL_MEM,          // call [mem]
    INST_JUMP_MEM,          // jmp  [mem]
    INST_SYSCALL,
    INST_RET
};


typedef struct _HOOK_INST {

    ULONG len;
    UCHAR kind;
    UCHAR op1, op2;
    ULONG64 parm;
    LONG *rel32;            // --> 32-bit relocation for control-xfer
    UCHAR *modrm;
    ULONG flags;

} HOOK_INST;


#endif // HOOK_WITH_PRIVATE_PARTS


//---------------------------------------------------------------------------
// HOOK_TRAMP
//---------------------------------------------------------------------------


#pragma pack(push)
#pragma pack(1)

typedef struct _HOOK_TRAMP {

    ULONG eyecatcher;                   // (0,4) signature

    ULONG size;                         // (4,4) tramp size (multiple of 16)

    void *target;                       // (8,4/8) branch target in source

    __declspec(align(16))
        UCHAR code[64];                 // (16) source code + extra jmp

	ULONG count;						// original length of relocated code

} HOOK_TRAMP;

#pragma pack(pop)


#define HOOK_TRAMP_CODE_TO_TRAMP_HEAD(x)    \
    (HOOK_TRAMP *)((ULONG_PTR)x - FIELD_OFFSET(HOOK_TRAMP, code))


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

// Hook_BuildTramp constructs a detour trampoline that will jump to
// SourceFunc + n, where n is the length, in bytes, of the first few
// instructions at SourceFunc that would be damaged by placing a jump
// instruction at the top of SourceFunc.  The jump instruction is assumed
// to be 5 bytes long in 32-bit mode and 12 bytes long in 64-bit mode.
// The damaged instructions are copied to the detour trampoline.
// Returns the address to the start of the code in the detour trampoline.
// The function being redirected to, through the overwriting jump
// instruction, should jump to the returned value, to invoke the
// original code.  The trampoline is built on the address specified
// in Trampoline.  Kernel-mode code should pass a NULL address to
// use a system space trampoline.

void *Hook_BuildTramp(
    void *SourceFunc, void *Trampoline, BOOLEAN is64, BOOLEAN probe);


// Hook_BuildJump1 will write a 'jmp DestFunc' instruction at
// the address specified by WritableAddr.  In case WritableAddr was
// generated using MmGetSystemAddressForMdlSafe, then ExecutableAddr
// specifies the corresponding "real" address which will be used to
// generate a 32-bit jump offset.  (Note that 64-bit code does not
// use jump offsets and disregards ExecutableAddr.)

void Hook_BuildJump(
    void *WritableAddr, void *ExecutableAddr, void *JumpTarget);


#ifdef HOOK_WITH_PRIVATE_PARTS


// Analyze a single instruction at the specified address

BOOLEAN Hook_Analyze(
    void *address,
    BOOLEAN probe_address,
    BOOLEAN is64,
    HOOK_INST *inst);

#endif // HOOK_WITH_PRIVATE_PARTS


//---------------------------------------------------------------------------


#endif // _MY_HOOK_H
