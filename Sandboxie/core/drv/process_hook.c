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
// Process Hooks
//---------------------------------------------------------------------------


#include "process.h"
#include "hook.h"


//---------------------------------------------------------------------------
// Functions from util.asm
//---------------------------------------------------------------------------


#ifdef _WIN64
extern ULONG_PTR Process_FindSandboxed64(void);
#endif


//---------------------------------------------------------------------------
// Process_BuildHookEntry
//---------------------------------------------------------------------------


_FX ULONG_PTR Process_BuildHookEntry(
    ULONG_PTR NewProc, ULONG_PTR OldProc, ULONG *IncPtr)
{
    HOOK_TRAMP *tramp;
    UCHAR *code;
    ULONG_PTR *pOldProc;

    tramp = Hook_BuildTramp(NULL, NULL, FALSE, FALSE);
    if (! tramp)
        return 0;
    tramp = HOOK_TRAMP_CODE_TO_TRAMP_HEAD(tramp);

    tramp->eyecatcher = tzuk;
    tramp->target = (void *)NewProc;

    code = &tramp->code[0];
    pOldProc = (ULONG_PTR *)&tramp->code[sizeof(tramp->code) - 8];

#ifdef _WIN64
    // sti instruction to re-enable interrupts; see Hook_BuildJump
    code[0] = 0xFB;
    ++code;
#endif

    // mov eax,eax
    code[0] = 0x8B;
    code[1] = 0xC0;
    code += 2;

#ifdef _WIN64
#define PREFIX64() code[0] = 0x48; ++code;
#else
#define PREFIX64()
#endif

    if (IncPtr) {
        // nop, indicates the optional inc
        code[0] = 0x90;
        ++code;
        // mov r/eax, IncPtr
        PREFIX64();
        code[0] = 0xB8;
        *(ULONG_PTR *)&code[1] = (ULONG_PTR)IncPtr;
        code += 1 + sizeof(ULONG_PTR);
        // inc dword ptr [r/eax]
        code[0] = 0xFF;
        code[1] = 0x00;
        code += 2;
    }

    // mov r/eax, Process_Find
    PREFIX64();
    code[0] = 0xB8;
#ifdef _WIN64
    *(ULONG_PTR *)&code[1] = (ULONG_PTR)Process_FindSandboxed64;
#else
    *(ULONG_PTR *)&code[1] = (ULONG_PTR)Process_FindSandboxed;
#endif
    code += 1 + sizeof(ULONG_PTR);

#ifndef _WIN64
    // push 0
    code[0] = 0x6A;
    code[1] = 0x00;
    code += 2;
    // push 0
    code[0] = 0x6A;
    code[1] = 0x00;
    code += 2;
#endif

    // call eax
    code[0] = 0xFF;
    code[1] = 0xD0;
    code += 2;

    // test eax, eax
    PREFIX64();
    code[0] = 0x85;
    code[1] = 0xC0;
    code += 2;
    // jnz +6
    code[0] = 0x75;
    code[1] = 6;
    code += 2;

    if (OldProc < 0x100) {
#ifdef _WIN64
        // ret
        code[0] = 0xC3;
        code[1] = 0x90;
        code[2] = 0x90;
#else
        // ret NumArgs
        code[0] = 0xC2;
        *(USHORT *)&code[1] = (USHORT)OldProc * 4;
#endif
        code[3] = 0x90;
        code[4] = 0x90;
        code[5] = 0x90;
    } else {
        *pOldProc = OldProc;
        // jmp q/dword ptr [AddressOfOldProc]
        code[0] = 0xff;
        code[1] = 0x25;
#ifdef _WIN64
        *(ULONG_PTR *)&code[2] = (ULONG_PTR)((UCHAR *)pOldProc - code - 6);
#else
        *(ULONG_PTR *)&code[2] = (ULONG_PTR)pOldProc;
#endif
    }
    code += 6;

    // jmp q/dword ptr [AddressOfNewProc]
    code[0] = 0xff;
    code[1] = 0x25;
#ifdef _WIN64
    *(ULONG_PTR *)&code[2] = (ULONG_PTR)((UCHAR *)&tramp->target - code - 6);
#else
    *(ULONG_PTR *)&code[2] = (ULONG_PTR)&tramp->target;
#endif
    code += 6;

#undef PREFIX64

    return (ULONG_PTR)&tramp->code;
}


//---------------------------------------------------------------------------
// Process_DisableHookEntry
//---------------------------------------------------------------------------


_FX void Process_DisableHookEntry(ULONG_PTR HookEntry)
{
    HOOK_TRAMP *tramp = HOOK_TRAMP_CODE_TO_TRAMP_HEAD(HookEntry);
    UCHAR *code = &tramp->code[0];
    UCHAR *test;
    UCHAR hotpatch[2];

    //
    // change 'test eax,eax' into 'xor eax,eax'.  this effectively
    // disables our hook by always jumping to the system hook
    //

#ifdef _WIN64
#define PREFIX64 1
#else
#define PREFIX64 0
#endif

#ifdef _WIN64
    // sti (64-bit only)
    ++code;
#endif

    test = code;
    // mov eax, eax
    test += 2;
    // optional inc dword ptr [eax]
    if (*test == 0x90)
        test += 1 + PREFIX64 + 1 + sizeof(ULONG_PTR) + 2;
    // mov eax, Process_Find
    test += PREFIX64 + 1 + sizeof(ULONG_PTR);
#ifndef _WIN64
    // (32-bit only) push, push
    test += 2 + 2;
#endif
    // call
    test += 2;

    // test -> xor
    *(test + PREFIX64) = 0x33;

#undef PREFIX64

    //
    // now change the 'mov eax,eax' at the top to jump directly
    // to the modified 'xor eax,eax' instruction
    //

    hotpatch[0] = 0xEB;
    hotpatch[1] = (UCHAR)(test - (code + 2));
    *(USHORT *)code = *(USHORT *)&hotpatch[0];
}
