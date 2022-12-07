/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2022 David Xanatos, xanasoft.com
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
// SbieDLL Hook Management
//---------------------------------------------------------------------------


#define NOGDI
#include "dll.h"
#include "hook.h"
#include "debug.h"
#include "trace.h"
#include "common/pool.h"
#include "common/pattern.h"
#if defined(_M_ARM64) || defined(_M_ARM64EC)
#include "common/arm64_asm.h"
#endif
#include "common/hook_util.c"

//#include <stdio.h>


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


ULONG_PTR  DLL_FindWow64Target(ULONG_PTR address);

BOOLEAN SbieDll_FuncSkipHook(const char* func);

//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


#ifdef _WIN64

typedef struct _VECTOR_TABLE {

  LIST_ELEM list_elem;

  void * offset;
  int index;
  int maxEntries;
} VECTOR_TABLE;

#define VTABLE_SIZE 0x4000 //16k enough for 2048 8 byte entries

#endif _WIN64
extern ULONG Dll_Windows;

typedef struct _MODULE_HOOK {

    LIST_ELEM   list_elem;

    HMODULE     module;
#ifdef _M_ARM64EC
    ULONG       tag;
#endif
    POOL*       pool;
#ifdef _WIN64
    LIST        vTables;
#endif
} MODULE_HOOK;

LIST Dll_ModuleHooks;
CRITICAL_SECTION  Dll_ModuleHooks_CritSec;

#ifdef _M_ARM64EC
P_NtAllocateVirtualMemoryEx __sys_NtAllocateVirtualMemoryEx = NULL;
#endif

static const WCHAR *_fmt1 = L"%s (%d)";
static const WCHAR *_fmt2 = L"%s (%d, %d)";


//---------------------------------------------------------------------------
// SbieApi_HookInit
//---------------------------------------------------------------------------


_FX void SbieDll_HookInit()
{
    InitializeCriticalSection(&Dll_ModuleHooks_CritSec);
    List_Init(&Dll_ModuleHooks);

#ifdef _M_ARM64EC
    __sys_NtAllocateVirtualMemoryEx = (P_NtAllocateVirtualMemoryEx)GetProcAddress(Dll_Ntdll, "NtAllocateVirtualMemoryEx");
#endif
}


//---------------------------------------------------------------------------
// SbieDll_GetModuleHookAndLock
//---------------------------------------------------------------------------


_FX MODULE_HOOK* SbieDll_GetModuleHookAndLock(HMODULE module, ULONG tag)
{
    //
    // Get the module hook resource for this module, if module is NULL
    // its NTDLL or a special case
    //

    EnterCriticalSection(&Dll_ModuleHooks_CritSec);

    MODULE_HOOK* mod_hook = List_Head(&Dll_ModuleHooks);
    while (mod_hook) {

        if (mod_hook->module == module
#ifdef _M_ARM64EC
            && mod_hook->tag == tag
#endif
        ) 
            break;

        mod_hook = List_Next(mod_hook);
    }

    if (!mod_hook) {
        mod_hook = Dll_Alloc(sizeof(MODULE_HOOK));
        if (!mod_hook) 
            return NULL;
        mod_hook->module = module;
#ifdef _M_ARM64EC
        mod_hook->tag = tag;
#endif
        mod_hook->pool = NULL;
#ifdef _WIN64
        List_Init(&mod_hook->vTables);
#endif
        List_Insert_Before(&Dll_ModuleHooks, NULL, mod_hook); // insert first as we probably will use it often in the next few calls
    }
    if (!mod_hook->pool) {
        mod_hook->pool = Pool_CreateTagged(tag);
        if (!mod_hook->pool) 
            return NULL;
    }
    
    return mod_hook;
}

//---------------------------------------------------------------------------
// SbieDll_GetHookTable
//---------------------------------------------------------------------------


#ifdef _WIN64
_FX VECTOR_TABLE* SbieDll_GetHookTable(MODULE_HOOK* mod_hook, ULONG_PTR target, __int64 maxDelta, BOOLEAN longRange)
{
    ULONG_PTR diff;
    __int64 delta;
    BOOLEAN defaultRange = FALSE;

    VECTOR_TABLE* ptrVTable = List_Head(&mod_hook->vTables);
    for (;;) {
        if (!ptrVTable || !ptrVTable->offset) { // if there is no vtable create it
            ULONG_PTR tempAddr;
            ULONG_PTR step = 0x20000;// + VTABLE_SIZE;
            ULONG_PTR max_attempts = 0x4000000 / step;

            // optimization for windows 7 and low memory DLL's
            if (target < 0x80000000 && (target > 0x4000000)) {
                step = 0x200000;
            }
            // optimization for windows 8.1
            else if (target < 0x4000000) {
                step *= -1;
            }
            else if (target < 0x10000000000) {
                step *= -1;
            }
            else if(longRange) {
                defaultRange = TRUE;
            }

            // sprintf(buffer,"VTable Alloc: target = %p, step = %p, default = %d\n",target,step,defaultRange);
            // OutputDebugStringA(buffer);
            tempAddr = (target & 0xfffffffffffe0000) - (step << 2);

            if (defaultRange) {
                tempAddr -= 0x20000000;
            }

            if (!ptrVTable) {

                ptrVTable = Pool_Alloc(mod_hook->pool, sizeof(VECTOR_TABLE));
                if (!ptrVTable) {
                    //SbieApi_Log(2303, _fmt1, SourceFuncName, 53);
                    break;
                }

                memset(ptrVTable, 0, sizeof(VECTOR_TABLE));

                List_Insert_After(&mod_hook->vTables, NULL, ptrVTable);
            }

            for (; !ptrVTable->offset && max_attempts; tempAddr -= step, max_attempts--) {

                PVOID RegionBase = (PVOID)tempAddr;
                SIZE_T RegionSize = VTABLE_SIZE;
#ifdef  _M_ARM64EC
                MEM_EXTENDED_PARAMETER Parameter = { 0 };
	            Parameter.Type = MemExtendedParameterAttributeFlags;
	            Parameter.ULong64 = MEM_EXTENDED_PARAMETER_EC_CODE;

                if (NT_SUCCESS(__sys_NtAllocateVirtualMemoryEx(NtCurrentProcess(), &RegionBase, &RegionSize, MEM_COMMIT | MEM_RESERVE | MEM_TOP_DOWN, PAGE_EXECUTE_READWRITE, &Parameter, 1)))
                    ptrVTable->offset = RegionBase;
#else
                if (NT_SUCCESS(NtAllocateVirtualMemory(NtCurrentProcess(), &RegionBase, 0, &RegionSize, MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN, PAGE_EXECUTE_READWRITE)))
                    ptrVTable->offset = RegionBase;
#endif
                //ptrVTable->offset = VirtualAlloc((void*)tempAddr, VTABLE_SIZE, MEM_COMMIT | MEM_RESERVE | MEM_TOP_DOWN, PAGE_EXECUTE_READWRITE); // PAGE_READWRITE
                //  sprintf(buffer,"VTable Offset: target = %p, offset = %p, tryAddress = %p, attempt = 0x%x\n",target,ptrVTable->offset,tempAddr,max_attempts);
                //  OutputDebugStringA(buffer);
            }

            // brute force fallback
            if (!ptrVTable->offset) {

                step = VTABLE_SIZE;
                max_attempts = 0x40000000 / step; // 1 gig
                ULONG_PTR cur_attempt = 0;

                tempAddr = ((ULONG_PTR)target & 0xfffffffffffe0000);

                for (; !ptrVTable->offset && cur_attempt < (max_attempts * 2); cur_attempt++) {

                    ULONG_PTR curAddr = tempAddr + (((cur_attempt + 2)/2) * step);
                    if((cur_attempt % 2) == 0) // search both directions alternating
                        curAddr *= -1;

                    PVOID RegionBase = (PVOID)curAddr;
                    SIZE_T RegionSize = VTABLE_SIZE;
#ifdef  _M_ARM64EC
                    MEM_EXTENDED_PARAMETER Parameter = { 0 };
	                Parameter.Type = MemExtendedParameterAttributeFlags;
	                Parameter.ULong64 = MEM_EXTENDED_PARAMETER_EC_CODE;

                    if (NT_SUCCESS(__sys_NtAllocateVirtualMemoryEx(NtCurrentProcess(), &RegionBase, &RegionSize, MEM_COMMIT | MEM_RESERVE | MEM_TOP_DOWN, PAGE_EXECUTE_READWRITE, &Parameter, 1)))
                        ptrVTable->offset = RegionBase;
#else
                    if (NT_SUCCESS(NtAllocateVirtualMemory(NtCurrentProcess(), &RegionBase, 0, &RegionSize, MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN, PAGE_EXECUTE_READWRITE)))
                        ptrVTable->offset = RegionBase;
#endif
                }
            }

            ptrVTable->index = 0;
            ptrVTable->maxEntries = VTABLE_SIZE / sizeof(void*);
        }

        if (ptrVTable && ptrVTable->offset) { // check if we have a vtable

            diff = (ULONG_PTR) &((ULONG_PTR *)ptrVTable->offset)[ptrVTable->index];
            diff = diff - target;
            delta = diff;
            delta < 0 ? delta *= -1 : delta;

            // is DetourFunc in the jump range
            if (delta < maxDelta && ptrVTable->index <= ptrVTable->maxEntries) {
                // found a good table, break and return it
                break;
            }
        }
        else { // fail and disable vtable if it could not be initialized
            //SbieApi_Log(2303, _fmt1, SourceFuncName, 888);
            ptrVTable = NULL;
            break;
        }

        ptrVTable = List_Next(ptrVTable);

    }

    return ptrVTable;
}
#endif _WIN64


#ifndef _M_ARM64

//---------------------------------------------------------------------------
// SbieApi_HookTramp
//---------------------------------------------------------------------------


_FX LONG SbieApi_HookTramp(void *Source, void *Trampoline)
{
	NTSTATUS status;
#ifdef _WIN64
	BOOLEAN is64 = TRUE;
#else
	BOOLEAN is64 = FALSE;
#endif _WIN64

	if (Hook_BuildTramp(Source, Trampoline, is64, TRUE))
		status = STATUS_SUCCESS;
	else
		status = STATUS_UNSUCCESSFUL;

	return status;
}


//---------------------------------------------------------------------------
// SbieDll_Hook_x86
//---------------------------------------------------------------------------


_FX void *SbieDll_Hook_x86(
    const char *SourceFuncName, void *SourceFunc, void *DetourFunc, HMODULE module)
{
    UCHAR *tramp, *func = NULL;
    void* RegionBase;
    SIZE_T RegionSize;
    ULONG prot, dummy_prot;
    ULONG_PTR diff;
    ULONG_PTR target;
#ifdef _WIN64
    long long delta;
    BOOLEAN CallInstruction64 = FALSE;
#endif _WIN64

    //
    // validate parameters
    //

    if (! SourceFunc) {
        SbieApi_Log(2303, _fmt1, SourceFuncName, 1);
        return NULL;
    }

    //
    // if the source function begins with relative jump EB xx, it means
    // someone else has already hooked it, so we try to hook the hook
    // (this helps to co-exist with Cisco Security Agent)
    //

    if (*(UCHAR *)SourceFunc == 0xEB) { // jmp xx;
        signed char offset = *((signed char *)SourceFunc + 1);
        SourceFunc = (UCHAR *)SourceFunc + offset + 2;
    }

    //
    // if the source function begins with a near jump E9 xx xx xx xx
    // then follow the jump to its destination.  if the destination
    // turns out to be the same as DetourFunc, then abort,
    // otherwise (for 32-bit code) just replace the jump target
    //

    while (*(UCHAR *)SourceFunc == 0xE9) { // jmp xx xx xx xx;

        diff = *(LONG *)((ULONG_PTR)SourceFunc + 1);
        target = (ULONG_PTR)SourceFunc + diff + 5;
        if (target == (ULONG_PTR)DetourFunc) {
            SbieApi_Log(2303, _fmt1, SourceFuncName, 4);
            return NULL;
        }

#ifdef _WIN64

        SourceFunc = (void *)target;

#else ! WIN_64

        func = Hook_CheckChromeHook((void *)target);
        if (func != (void *)target) {
            SourceFunc = func;
            goto skip_e9_rewrite;
        }

        func = (UCHAR *)SourceFunc;
        diff = (UCHAR *)DetourFunc - (func + 5);
        ++func;
        if (! VirtualProtect(func, 4, PAGE_EXECUTE_READWRITE, &prot)) {
            ULONG err = GetLastError();
            SbieApi_Log(2303, _fmt2, SourceFuncName, 31, err);
            return NULL;
        }
        *(ULONG *)func = (ULONG)diff;
        VirtualProtect(func, 4, prot, &dummy_prot);

        return (void *)target;

skip_e9_rewrite: ;

#endif _WIN64

    }

#ifdef _WIN64

    //
    // 64-bit only:  avast snxhk64.dll compatibility:  the function
    // begins with nop+jmp (90,E9), and the jump target is a second
    // jump instruction 'jmp qword ptr [x]', then advance the pointer
    // to the second jump instruction so the next block of code
    // can process it
    //

    if (*(USHORT *)SourceFunc == 0xE990) { // nop; jmp xx xx xx xx;
        diff = *(LONG *)((ULONG_PTR)SourceFunc + 2);
        target = (ULONG_PTR)SourceFunc + diff + 6;
        if (*(USHORT *)target == 0x25FF) // jmp QWORD PTR [rip+xx xx xx xx];
            SourceFunc = (void *)target;
    }

	//
	// DX: this simplification fails for delay loaded libraries, see comments about SetSecurityInfo,
	// resulting in an endless loop, so just don't do that 
	//

#if 0
    //
    // 64-bit only:  if the function begins with 'jmp qword ptr [x]'
    // (6 bytes) then replace the value at x, rather than overwrite
    // 12 bytes.
    //

    if (*(UCHAR *)SourceFunc == 0x48 && // rex.W
            *(USHORT *)((UCHAR *)SourceFunc + 1) == 0x25FF) { // jmp QWORD PTR [rip+xx xx xx xx];
        // 48 FF 25 is same as FF 25
        SourceFunc = (UCHAR *)SourceFunc + 1;
    }

    if (*(USHORT *)SourceFunc == 0x25FF) { // jmp QWORD PTR [rip+xx xx xx xx];

        void *orig_addr;
        /*
        sprintf(buffer,"0x25FF Hook: %s\n",SourceFuncName);
        OutputDebugStringA(buffer);
        */
        diff = *(LONG *)((ULONG_PTR)SourceFunc + 2);
        target = (ULONG_PTR)SourceFunc + 6 + diff;
        orig_addr = (void *)*(ULONG_PTR *)target;
        if (orig_addr == DetourFunc) {
            SbieApi_Log(2303, _fmt1, SourceFuncName, 4);
            return NULL;
        }

        func = (UCHAR *)target;
        if (! VirtualProtect(func, 8, PAGE_EXECUTE_READWRITE, &prot)) {
            ULONG err = GetLastError();
            SbieApi_Log(2303, _fmt2, SourceFuncName, 32, err);
            return NULL;
        }
        *(ULONG_PTR *)target = (ULONG_PTR)DetourFunc;
        VirtualProtect(func, 8, prot, &dummy_prot);

        return orig_addr;
    }
#endif

#endif _WIN64

    //
    // Get the module hook object and obtain lock on critical section
    //

    MODULE_HOOK* mod_hook = SbieDll_GetModuleHookAndLock(module, tzuk | 0xFF);
    if (!mod_hook) {
        SbieApi_Log(2303, _fmt1, SourceFuncName, 5);
        goto finish;
    }
    
    //
    // 64-bit only:  if the function begins with 'call qword ptr [x]'
    // (6 bytes) then overwrite at the call target address.
    //

#ifdef _WIN64

    if (*(USHORT *)SourceFunc == 0x15FF) { // call QWORD PTR [rip+xx xx xx xx];

        //
        // the call instruction pushes a qword into the stack, we need
        // to remove this qword before calling our detour function
        //

        //UCHAR *NewDetour = Dll_AllocCode128();
        UCHAR *NewDetour = Pool_Alloc(mod_hook->pool, 128);
        if (! NewDetour) {
            SbieApi_Log(2305, NULL);
            goto finish;
        }

        NewDetour[0] = 0x58;        // pop rax
        NewDetour[1] = 0x48;        // mov rax, DetourFunc
        NewDetour[2] = 0xB8;
        *(ULONG_PTR *)(&NewDetour[3]) = (ULONG_PTR)DetourFunc;
        NewDetour[11] = 0xFF;       // jmp rax
        NewDetour[12] = 0xE0;

        DetourFunc = NewDetour;

        //
        // when our detour function calls the trampoline to invoke the
        // original code, we have to push the qword back into the stack,
        // because this is what the original code expects
        //

        NewDetour[16] = 0x48;       // mov rax, SourceFunc+6
        NewDetour[17] = 0xB8;
        *(ULONG_PTR *)(&NewDetour[18]) = (ULONG_PTR)SourceFunc + 6;
        NewDetour[26] = 0x50;       // push rax
        NewDetour[27] = 0x48;       // mov rax, trampoline code
        NewDetour[28] = 0xB8;
        *(ULONG_PTR *)(&NewDetour[29]) = 0;
        NewDetour[37] = 0xFF;       // jmp rax
        NewDetour[38] = 0xE0;

        CallInstruction64 = TRUE;

        //
        // overwrite the code at the target of the call instruction
        //

        diff = *(LONG *)((ULONG_PTR)SourceFunc + 2);
        target = (ULONG_PTR)SourceFunc + 6 + diff;
        SourceFunc = (void *)*(ULONG_PTR *)target;
    }

#endif _WIN64

    //
    // create the trampoline
    //

    //tramp = Dll_AllocCode128();
    tramp = Pool_Alloc(mod_hook->pool, 128);
    if (! tramp /*|| !VirtualProtect(tramp, 128, PAGE_EXECUTE_READWRITE, &dummy_prot)*/) {
        SbieApi_Log(2305, NULL);
        goto finish;
    }

    if (SbieApi_HookTramp(SourceFunc, tramp) != 0) {
        SbieApi_Log(2303, _fmt1, SourceFuncName, 2);
        goto finish;
    }

	//ULONG ByteCount = *(ULONG*)(tramp + 80);
	//ULONG UsedCount = 0;
	
    //
    // create the detour
    //

    func = (UCHAR *)SourceFunc;

    RegionBase = &func[-8]; // -8 for hotpatch area if present
    RegionSize = 20;

    if (!VirtualProtect(RegionBase, RegionSize, PAGE_EXECUTE_READWRITE, &prot)) {

        //
        // on windows 7 hooking NdrClientCall2 in 32bit (WoW64) mode fails
        // because the memory area starts at -6 and not -8
        // this area could be a hot patch reagion which we don't use
        // hence if that fails just start at the exact offset and try again
        //

        RegionBase = &func[0];
        RegionSize = 12;

        if (!VirtualProtect(RegionBase, RegionSize, PAGE_EXECUTE_READWRITE, &prot)) {
            ULONG err = GetLastError();
            SbieApi_Log(2303, _fmt2, SourceFuncName, 33, err);
            func = NULL;
            goto finish;
        }
    }

    //
    // hook the source function
    //

#ifdef _WIN64
    if (Dll_Windows >= 10) {
        target = (ULONG_PTR)&func[6];
    }
    else {
        target = (ULONG_PTR)&func[5];
    }

    diff = (ULONG_PTR)((ULONG_PTR)DetourFunc - target);
    delta = diff;
    delta < 0 ? delta *= -1 : delta;

    //is DetourFunc in 32bit jump range
    if (delta < 0x80000000) {
        /*
        sprintf(buffer,"32 bit Hook: %s\n",SourceFuncName);
        OutputDebugStringA(buffer);
        */
        if (Dll_Windows >= 10) {
            func[0] = 0x48;             // 32bit relative rex.W JMP DetourFunc
            func[1] = 0xE9;
            *(ULONG *)(&func[2]) = (ULONG)diff;
			//UsedCount = 1 + 1 + 4;
        }
        else {
            func[0] = 0xE9;             // 32bit relative JMP DetourFunc
            *(ULONG *)(&func[1]) = (ULONG)diff;
			//UsedCount = 1 + 4;
        }
    }

    //is DetourFunc in 64bit jump range
    /*else if (1) {

        func[0] = 0x48;
        func[1] = 0xb8;
        *(ULONG_PTR *)&func[2] = (ULONG_PTR)DetourFunc;
        func[10] = 0xff;
        func[11] = 0xe0;
    }*/

    else {
        
        target = (ULONG_PTR)&func[6];

        VECTOR_TABLE* ptrVTable = SbieDll_GetHookTable(mod_hook, target, 0x80000000, TRUE);
        if (!ptrVTable) {
            // OutputDebugStringA("Memory alloc failed: 12 Byte Patch Disabled\n");
            SbieApi_Log(2303, _fmt1, SourceFuncName, 999);
            func = NULL;
            goto finish;
        }

        diff = (ULONG_PTR) &((ULONG_PTR *)ptrVTable->offset)[ptrVTable->index];
        diff = diff - target;

        ((ULONG_PTR *)ptrVTable->offset)[ptrVTable->index] = (ULONG_PTR)DetourFunc;
        *(USHORT *)&func[0] = 0x25ff;       // jmp QWORD PTR [rip+diff];
        *(ULONG *)&func[2] = (ULONG)diff;
		//UsedCount = 2 + 4;
        ptrVTable->index++;
    }

#else
    diff = (UCHAR *)DetourFunc - (func + 5);
    func[0] = 0xE9;             // JMP DetourFunc
    *(ULONG *)(&func[1]) = (ULONG)diff;
	//UsedCount = 1 + 4;
#endif

	// just in case nop out the rest of the code we moved to the trampoline
	// ToDo: why does this break unity games
	//for(; UsedCount < ByteCount; UsedCount++)
	//	func[UsedCount] = 0x90; // nop

	VirtualProtect(RegionBase, RegionSize, prot, &dummy_prot);

    // the trampoline code begins at trampoline + 16 bytes
    func = (UCHAR *)(ULONG_PTR)(tramp + 16);

    //
    // 64-bit only:  if we are hooking a function that started with a
    // call instruction, then we have to return a secondary trampoline
    //

#ifdef _WIN64

    if (CallInstruction64) {

        UCHAR *NewDetour = (UCHAR *)DetourFunc;
        *(ULONG_PTR *)(&NewDetour[29]) = (ULONG_PTR)func;
        func = NewDetour + 16;
    }

#endif _WIN64

finish:
    LeaveCriticalSection(&Dll_ModuleHooks_CritSec);

    return func;
}

#endif

#if defined(_M_ARM64) || defined(_M_ARM64EC)

//---------------------------------------------------------------------------
// Detours
//---------------------------------------------------------------------------

#include "../../common/Detours/detours.h"

#ifdef _M_ARM64EC

PVOID WINAPI DetourCopyInstructionARM64(_In_opt_ PVOID pDst,            \
                                      _Inout_opt_ PVOID *ppDstPool,     \
                                      _In_ PVOID pSrc,                  \
                                      _Out_opt_ PVOID *ppTarget,        \
                                      _Out_opt_ LONG *plExtra);         \

#endif

#ifdef DETOURS_ARM

#define DETOURS_PFUNC_TO_PBYTE(p)  ((PBYTE)(((ULONG_PTR)(p)) & ~(ULONG_PTR)1))
#define DETOURS_PBYTE_TO_PFUNC(p)  ((PBYTE)(((ULONG_PTR)(p)) | (ULONG_PTR)1))

#endif // DETOURS_ARM

typedef struct _DETOUR_ALIGN
{
    BYTE    obTarget        : 3;
    BYTE    obTrampoline    : 5;
} DETOUR_ALIGN;

//C_ASSERT(sizeof(_DETOUR_ALIGN) == 1);

typedef struct _DETOUR_TRAMPOLINE
{
    // An ARM64 instruction is 4 bytes long.
    //
    // The overwrite is always 2 instructions plus a literal, so 16 bytes, 4 instructions.
    //
    // Copied instructions can expand.
    //
    // The scheme using MovImmediate can cause an instruction
    // to grow as much as 6 times.
    // That would be Bcc or Tbz with a large address space:
    //   4 instructions to form immediate
    //   inverted tbz/bcc
    //   br
    //
    // An expansion of 4 is not uncommon -- bl/blr and small address space:
    //   3 instructions to form immediate
    //   br or brl
    //
    // A theoretical maximum for rbCode is therefore 4*4*6 + 16 = 112 (another 16 for jmp to pbRemain).
    //
    // With literals, the maximum expansion is 5, including the literals: 4*4*5 + 16 = 96.
    //
    // The number is rounded up to 128. m_rbScratchDst should match this.
    //
    BYTE            rbCode[128];    // target code + jmp to pbRemain
    BYTE            cbCode;         // size of moved target code.
    BYTE            cbCodeBreak[3]; // padding to make debugging easier.
    BYTE            rbRestore[24];  // original target code.
    BYTE            cbRestore;      // size of original target code.
    BYTE            cbRestoreBreak[3]; // padding to make debugging easier.
    DETOUR_ALIGN    rAlign[8];      // instruction alignment array.
    PBYTE           pbRemain;       // first instruction after moved code. [free list]
    PBYTE           pbDetour;       // first instruction of detour function.
} DETOUR_TRAMPOLINE, *PDETOUR_TRAMPOLINE;

//C_ASSERT(sizeof(_DETOUR_TRAMPOLINE) == 184);

enum {
    SIZE_OF_JMP = 16
};


#define DETOUR_TRACE(x)


void arm64_write_opcode(PBYTE *pbCode, ULONG Opcode)
{
    *(ULONG *)*pbCode = Opcode;
    *pbCode += 4;
}

ULONG arm64_fetch_opcode(PBYTE pbCode)
{
    return *(ULONG *)pbCode;
}

BOOL arm64_detour_does_code_end_function(PBYTE pbCode)
{
    ULONG Opcode = arm64_fetch_opcode(pbCode);
    if ((Opcode & 0xfffffc1f) == 0xd61f0000 ||      // br <reg>
        (Opcode & 0xfc000000) == 0x14000000) {      // b <imm26>
        return TRUE;
    }
    return FALSE;
}

ULONG arm64_detour_is_code_filler(PBYTE pbCode)
{
    if (*(ULONG *)pbCode == 0xd503201f) {   // nop.
        return 4;
    }
    if (*(ULONG *)pbCode == 0x00000000) {   // zero-filled padding.
        return 4;
    }
    return 0;
}

PBYTE arm64_detour_gen_jmp_immediate(PBYTE pbCode, PBYTE* ppPool, PBYTE pbJmpVal, BOOL tryUsePatchSpace)
{
    PBYTE pbLiteral;
    if (ppPool != NULL) {
        *ppPool = *ppPool - 8;
        pbLiteral = *ppPool;
    }
    else if (tryUsePatchSpace && *(UINT_PTR*)(pbCode - 8) == 0) { // use the provided empty space
        pbLiteral = pbCode - 8;
    } 
    else {
        pbLiteral = pbCode + 8;
    }

    *((PBYTE*)pbLiteral) = pbJmpVal;
    LONG delta = (LONG)(pbLiteral - pbCode);

    arm64_write_opcode(&pbCode, 0x58000011 | (((delta / 4) & 0x7FFFF) << 5));  // LDR X17,[PC+n]
    arm64_write_opcode(&pbCode, 0xd61f0000 | (17 << 5));                       // BR X17

    if (ppPool == NULL) {
        pbCode += 8;
    }
    return pbCode;
}

PBYTE arm64_detour_gen_brk(PBYTE pbCode, PBYTE pbLimit)
{
    while (pbCode < pbLimit) {
        arm64_write_opcode(&pbCode, 0xd4100000 | (0xf000 << 5));
    }
    return pbCode;
}


//---------------------------------------------------------------------------
// SbieDll_Hook_arm
//---------------------------------------------------------------------------


void* SbieDll_Hook_arm(
    const char* SourceFuncName, void* SourceFunc, void* DetourFunc, HMODULE module)
{
    UCHAR *tramp, *func = NULL;
    void* RegionBase;
    SIZE_T RegionSize;
    ULONG prot, dummy_prot;
    ULONG_PTR diff;
    ULONG_PTR target;

    //
    // validate parameters
    //

    if (! SourceFunc) {
        SbieApi_Log(2303, _fmt1, SourceFuncName, 1);
        return NULL;
    }

    //
    // Get the module hook object and obtain lock on critical section
    //

#ifdef _M_ARM64EC
    MODULE_HOOK* mod_hook = SbieDll_GetModuleHookAndLock(module, (tzuk & 0xFFFFFF00) | 0xEC);
#else
    MODULE_HOOK* mod_hook = SbieDll_GetModuleHookAndLock(module, tzuk | 0xFF);
#endif
    if (!mod_hook) {
        SbieApi_Log(2303, _fmt1, SourceFuncName, 5);
        goto finish;
    }

    //
    // create the trampoline
    //
    tramp = Pool_Alloc(mod_hook->pool, sizeof(DETOUR_TRAMPOLINE));
    if (! tramp /*|| !VirtualProtect(tramp, sizeof(DETOUR_TRAMPOLINE), PAGE_EXECUTE_READWRITE, &dummy_prot)*/) {
        SbieApi_Log(2305, NULL);
        goto finish;
    }

    //
    // From MS detours begin:
    //

    PVOID pDetour = DetourFunc;
    PVOID* ppPointer = &SourceFunc;
    PBYTE pbTarget = (PBYTE)*ppPointer;
    PDETOUR_TRAMPOLINE pTrampoline = tramp;

#ifdef _M_ARM64
    pbTarget = (PBYTE)DetourCodeFromPointer(pbTarget, NULL);
    pDetour = DetourCodeFromPointer(pDetour, NULL);
#endif
    
    DETOUR_TRACE(("detours: pbTramp=%p, pDetour=%p\n", pTrampoline, pDetour));

    memset(pTrampoline->rAlign, 0, sizeof(pTrampoline->rAlign));

    // Determine the number of movable target instructions.
    PBYTE pbSrc = pbTarget;
    PBYTE pbTrampoline = pTrampoline->rbCode;
    PBYTE pbPool = pbTrampoline + sizeof(pTrampoline->rbCode);
    ULONG cbTarget = 0;
    ULONG cbJump = SIZE_OF_JMP;
    ULONG nAlign = 0;

#ifdef DETOURS_ARM
    // On ARM, we need an extra instruction when the function isn't 32-bit aligned.
    // Check if the existing code is another detour (or at least a similar
    // "ldr pc, [PC+0]" jump.
    if ((ULONG)pbTarget & 2) {
        cbJump += 2;

        ULONG op = fetch_thumb_opcode(pbSrc);
        if (op == 0xbf00) {
            op = fetch_thumb_opcode(pbSrc + 2);
            if (op == 0xf8dff000) { // LDR PC,[PC]
                *((PUSHORT&)pbTrampoline)++ = *((PUSHORT&)pbSrc)++;
                *((PULONG&)pbTrampoline)++ = *((PULONG&)pbSrc)++;
                *((PULONG&)pbTrampoline)++ = *((PULONG&)pbSrc)++;
                cbTarget = (LONG)(pbSrc - pbTarget);
                // We will fall through the "while" because cbTarget is now >= cbJump.
            }
        }
    }
    else {
        ULONG op = fetch_thumb_opcode(pbSrc);
        if (op == 0xf8dff000) { // LDR PC,[PC]
            *((PULONG&)pbTrampoline)++ = *((PULONG&)pbSrc)++;
            *((PULONG&)pbTrampoline)++ = *((PULONG&)pbSrc)++;
            cbTarget = (LONG)(pbSrc - pbTarget);
            // We will fall through the "while" because cbTarget is now >= cbJump.
        }
    }
#endif

    while (cbTarget < cbJump) {
        PBYTE pbOp = pbSrc;
        LONG lExtra = 0;

        DETOUR_TRACE((" DetourCopyInstruction(%p,%p)\n",
                      pbTrampoline, pbSrc));
        pbSrc = (PBYTE)
#ifdef _M_ARM64EC
            DetourCopyInstructionARM64(pbTrampoline, (PVOID*)&pbPool, pbSrc, NULL, &lExtra);
#else
            DetourCopyInstruction(pbTrampoline, (PVOID*)&pbPool, pbSrc, NULL, &lExtra);
#endif
        DETOUR_TRACE((" DetourCopyInstruction() = %p (%d bytes)\n",
                      pbSrc, (int)(pbSrc - pbOp)));
        pbTrampoline += (pbSrc - pbOp) + lExtra;
        cbTarget = (ULONG)(pbSrc - pbTarget);
        pTrampoline->rAlign[nAlign].obTarget = (BYTE)cbTarget;
        pTrampoline->rAlign[nAlign].obTrampoline = (BYTE)(pbTrampoline - pTrampoline->rbCode);
        nAlign++;

        if (nAlign >= ARRAYSIZE(pTrampoline->rAlign)) {
            break;
        }

        if (arm64_detour_does_code_end_function(pbOp)) {
            break;
        }
    }

    // Consume, but don't duplicate padding if it is needed and available.
    while (cbTarget < cbJump) {
        LONG cFiller = arm64_detour_is_code_filler(pbSrc);
        if (cFiller == 0) {
            break;
        }

        pbSrc += cFiller;
        cbTarget = (LONG)(pbSrc - pbTarget);
    }

#if DETOUR_DEBUG
    {
        DETOUR_TRACE((" detours: rAlign ["));
        LONG n = 0;
        for (n = 0; n < ARRAYSIZE(pTrampoline->rAlign); n++) {
            if (pTrampoline->rAlign[n].obTarget == 0 &&
                pTrampoline->rAlign[n].obTrampoline == 0) {
                break;
            }
            DETOUR_TRACE((" %d/%d",
                          pTrampoline->rAlign[n].obTarget,
                          pTrampoline->rAlign[n].obTrampoline
                          ));

        }
        DETOUR_TRACE((" ]\n"));
    }
#endif

    //if (cbTarget < cbJump || nAlign > ARRAYSIZE(pTrampoline->rAlign)) {
    if (nAlign > ARRAYSIZE(pTrampoline->rAlign)) {
        // Too few instructions.
        SbieApi_Log(2303, _fmt1, SourceFuncName, 2);
        goto finish;
    }

    pTrampoline->cbCode = (BYTE)(pbTrampoline - pTrampoline->rbCode);
    pTrampoline->cbRestore = (BYTE)cbTarget;
    CopyMemory(pTrampoline->rbRestore, pbTarget, cbTarget);

    if (cbTarget > sizeof(pTrampoline->rbCode) - cbJump) {
        // Too many instructions.
        SbieApi_Log(2303, _fmt1, SourceFuncName, 3);
        goto finish;
    }

    pTrampoline->pbRemain = pbTarget + cbTarget;
    pTrampoline->pbDetour = (PBYTE)pDetour;


    pbTrampoline = pTrampoline->rbCode + pTrampoline->cbCode;
    pbTrampoline = arm64_detour_gen_jmp_immediate(pbTrampoline, &pbPool, pTrampoline->pbRemain, FALSE);
    pbTrampoline = arm64_detour_gen_brk(pbTrampoline, pbPool);

    //
    // end
    //


    //
    // create the detour
    //

    func = (UCHAR *)pbTarget;

    RegionBase = &func[-8]; // -8 for hotpatch area if present
    RegionSize = 24;

    if (!VirtualProtect(RegionBase, RegionSize, PAGE_EXECUTE_READWRITE, &prot)) {

        //
        // if that fails just start at the exact offset and try again
        // without the hot patch area which we don't use anyways
        //

        RegionBase = &func[0];
        RegionSize = 16;

        if (!VirtualProtect(RegionBase, RegionSize, PAGE_EXECUTE_READWRITE, &prot)) {
            ULONG err = GetLastError();
            SbieApi_Log(2303, _fmt2, SourceFuncName, 33, err);
            func = NULL;
            goto finish;
        }
    }

    //
    // hook the source function
    //

    if (cbTarget >= cbJump) {

        //
        // enough space for a normal detour, we have SIZE_OF_JMP available or more
        //

        //PBYTE pbCode = arm64_detour_gen_jmp_immediate(pbTarget, NULL, pTrampoline->pbDetour, TRUE);
        PBYTE pbCode = arm64_detour_gen_jmp_immediate(pbTarget, NULL, pTrampoline->pbDetour, FALSE);
        pbCode = arm64_detour_gen_brk(pbCode, pTrampoline->pbRemain);
#ifdef DETOURS_ARM
        *o->ppbPointer = DETOURS_PBYTE_TO_PFUNC(o->pTrampoline->rbCode);
#else
        *ppPointer = pTrampoline->rbCode;
#endif

    } 

    else {

        //
        // if we don't have SIZE_OF_JMP available, we use a jump table located
        // within +/- 128MB which we can reach with a single branch instruction
        //

        target = (ULONG_PTR)&func[0];

        VECTOR_TABLE* ptrVTable = SbieDll_GetHookTable(mod_hook, target, 0x08000000, FALSE); // +/-128MB
        if (!ptrVTable) {
            // OutputDebugStringA("Memory alloc failed: 12 Byte Patch Disabled\n");
            SbieApi_Log(2303, _fmt1, SourceFuncName, 999);
            func = NULL;
            goto finish;
        }
        
        //
        // write jump entry
        //

        PBYTE pbJump = (PBYTE)&((ULONG_PTR *)ptrVTable->offset)[ptrVTable->index];

        PBYTE pbCode = arm64_detour_gen_jmp_immediate(pbJump, NULL, pTrampoline->pbDetour, FALSE);
        //pbCode = arm64_detour_gen_brk(pbCode, pTrampoline->pbRemain);
        FlushInstructionCache(GetCurrentProcess(), pbJump, SIZE_OF_JMP);
#ifdef DETOURS_ARM
        *o->ppbPointer = DETOURS_PBYTE_TO_PFUNC(o->pTrampoline->rbCode);
#else
        *ppPointer = pTrampoline->rbCode;
#endif

        //
        // write branche to jump entry
        //

        diff = (ULONG_PTR) & ((ULONG_PTR*)ptrVTable->offset)[ptrVTable->index];
        diff = diff - target;

        B b;
        b.OP = 0x14000000;
        b.imm26 = (ULONG)(diff >> 2);
        arm64_write_opcode(&pbTarget, b.OP);

        ptrVTable->index += SIZE_OF_JMP / sizeof(ULONG_PTR); // +16 bytes
    }

    //
    // restore protection and fluch instruction cache
    //

	VirtualProtect(RegionBase, RegionSize, prot, &dummy_prot);
    FlushInstructionCache(GetCurrentProcess(), RegionBase, RegionSize);

    func = *ppPointer;
    FlushInstructionCache(GetCurrentProcess(), pTrampoline->rbCode, pTrampoline->cbCode + SIZE_OF_JMP);

finish:
    LeaveCriticalSection(&Dll_ModuleHooks_CritSec);

    return func;
}

#endif


//---------------------------------------------------------------------------
// SbieDll_Hook
//---------------------------------------------------------------------------


_FX void *SbieDll_Hook(
    const char *SourceFuncName, void *SourceFunc, void *DetourFunc, HMODULE module)
{
    if (SbieDll_FuncSkipHook(SourceFuncName))
        return SourceFunc;

    //if (Dll_SbieTrace) {
    //    WCHAR* ModuleName = Trace_FindModuleByAddress((void*)module);
    //    DbgPrint("Hooking: %S!%s\r\n", ModuleName, SourceFuncName);
    //}

    //
    // Chrome sandbox support
    //

    //void* OldSourceFunc = SourceFunc;

    SourceFunc = Hook_CheckChromeHook(SourceFunc);

    //if (OldSourceFunc != SourceFunc) {
    //    WCHAR* ModuleName = Trace_FindModuleByAddress((void*)module);
    //    DbgPrint("Found Chrome Hook on: %S!%s\r\n", ModuleName, SourceFuncName);
    //}
    

#ifdef _M_ARM64EC

    //
    // Check if the function is a Fast Forward Sequence, if so
    // get the target address of the EC function and hook it instead
    // this way we can intercept also internal function calls within a dll
    // like CreateProcessInternalW when called from CreateProcessW
    //

    extern ULONG* SbieApi_SyscallPtr;
    if (module == Dll_Ntdll && *(USHORT*)&SourceFuncName[0] == 'tN' && SbieApi_SyscallPtr) {

        USHORT index = Hook_GetSysCallIndex(SourceFunc);
        if (index != 0xFFFF) {

            ULONG SbieDll_GetSysCallOffset(const ULONG *SyscallPtr, ULONG syscall_index);
            ULONG offset = SbieDll_GetSysCallOffset(SbieApi_SyscallPtr, index);
            if (offset) {

                void* SourceFuncEC = (void*)((UINT_PTR)Dll_Ntdll + offset);
                return SbieDll_Hook_arm(SourceFuncName, SourceFuncEC, DetourFunc, module);
            }
            //else // hook disabled in driver like NtTerminateProcess
            //    SbieApi_Log(2303, _fmt2, SourceFuncName, 69, 2);
        }
        else
            SbieApi_Log(2303, _fmt2, SourceFuncName, 69, 1);
    }
    else

    // 
    // if module is -1 than we comes from the api redirection in Scm_SecHostDll
    // as there we hook with other x64 code we use the regular x86 hook routime
    //

    if (module != (HMODULE)-1) {

        void* SourceFuncEC = Hook_GetFFSTarget(SourceFunc);
        if (SourceFuncEC) {

            return SbieDll_Hook_arm(SourceFuncName, SourceFuncEC, DetourFunc, module);
        }
        else
            SbieApi_Log(2303, _fmt1, SourceFuncName, 69);
    }

#endif
#ifdef _M_ARM64
    return SbieDll_Hook_arm(SourceFuncName, SourceFunc, DetourFunc, module);
#else
    return SbieDll_Hook_x86(SourceFuncName, SourceFunc, DetourFunc, module);
#endif
}


//---------------------------------------------------------------------------
// SbieDll_UnHookModule
//---------------------------------------------------------------------------


_FX void SbieDll_UnHookModule(HMODULE module)
{
    EnterCriticalSection(&Dll_ModuleHooks_CritSec);

    MODULE_HOOK* mod_hook = List_Head(&Dll_ModuleHooks);
    while (mod_hook) {

        if (mod_hook->module == module) {

            List_Remove(&Dll_ModuleHooks, mod_hook);

#ifdef _WIN64
            VECTOR_TABLE* ptrVTable = List_Head(&mod_hook->vTables);
            while (ptrVTable) {

                if (ptrVTable->offset)
                    VirtualFree(ptrVTable->offset, 0, MEM_RELEASE);

                ptrVTable = List_Next(ptrVTable);
            }
#endif

            Pool_Delete(mod_hook->pool);
            Dll_Free(mod_hook);

            break;
        }

        mod_hook = List_Next(mod_hook);
    }

    LeaveCriticalSection(&Dll_ModuleHooks_CritSec);
}


//---------------------------------------------------------------------------
// SbieDll_FuncSkipHook
//---------------------------------------------------------------------------


BOOLEAN SbieDll_FuncSkipHook(const char* func)
{
    static const WCHAR* setting = L"FuncSkipHook";

    static BOOLEAN Disable = FALSE;
    if (Disable) return FALSE;

    WCHAR buf[66];
    ULONG index = 0;
    while (1) {
        NTSTATUS status = SbieApi_QueryConfAsIs(NULL, setting, index, buf, 64 * sizeof(WCHAR));
        if (NT_SUCCESS(status)) {
            WCHAR* ptr = buf;
            for (const char* tmp = func; *ptr && *tmp && *ptr == *tmp; ptr++, tmp++);
            if (*ptr == L'\0') //if (_wcsicmp(buf, func) == 0)
                return TRUE;
        }
        else if (status != STATUS_BUFFER_TOO_SMALL)
            break;
        ++index;
    }

    // if there are no configured functions to skip, disable the check
    if (index == 0) Disable = TRUE;

    return FALSE;
}


//---------------------------------------------------------------------------
// Dll_GetSettingsForImageName
//---------------------------------------------------------------------------


_FX NTSTATUS Dll_GetSettingsForImageName(
    const WCHAR *setting, WCHAR* value, ULONG value_size, const WCHAR *deftext)
{
    POOL *pool;
    WCHAR *text, *image_lwr, *buf;
    ULONG text_len, image_len;
    ULONG index;
    BOOLEAN match = FALSE;

    //
    //
    //

    pool = Pool_Create();
    if (! pool)
        goto outofmem;

    //
    //
    //

    if (deftext)
        text_len = wcslen(deftext);
    else
        text_len = 0;
    text = Pool_Alloc(pool, (text_len + 1) * sizeof(WCHAR));
    if (! text)
        goto outofmem;
    wmemcpy(text, deftext, text_len);
    text[text_len] = L'\0';

    //
    //
    //

    image_len = (wcslen(Dll_ImageName) + 1) * sizeof(WCHAR);
    image_lwr = Pool_Alloc(pool, image_len);
    if (! image_lwr)
        goto outofmem;
    memcpy(image_lwr, Dll_ImageName, image_len);
    _wcslwr(image_lwr);
    image_len = wcslen(image_lwr);

    //
    //
    //

    buf = Pool_Alloc(pool, 1024 * sizeof(WCHAR));
    if (! buf)
        goto outofmem;

    index = 0;
    while (1) {

        WCHAR *ptr, *buf_ptr;
        PATTERN *image_pat;

        NTSTATUS status = SbieApi_QueryConfAsIs(
                    NULL, setting, index, buf, 1020 * sizeof(WCHAR));
        if (! NT_SUCCESS(status))
            break;
        ++index;

        ptr = wcschr(buf, L',');
        if (!ptr) {
            ptr = buf;
            goto skip_match; // if there is no L',' it means any image
        }
        *ptr = L'\0';

        if (buf[0] == L'/' && buf[1] == L'/' &&
                (Dll_ProcessFlags & SBIE_FLAG_IMAGE_FROM_SBIE_DIR)) {
            // a // prefix in the image name (such as //start.exe) matches
            // only if the image resides in our installation directory
            buf_ptr = buf + 2;
        } else
            buf_ptr = buf;

        image_pat = Pattern_Create(pool, buf_ptr, TRUE, 0);
        if (Pattern_Match(image_pat, image_lwr, image_len)) {

            match = TRUE;
        }

        Pattern_Free(image_pat);

        if (!match)
            continue;

        if (text_len)
            *ptr = L',';    // restore comma if text is not empty
        else
            ++ptr;          // or skip comma if text is empty

        skip_match:
        {
            ULONG ptr_len;
            WCHAR* new_text;
            ptr_len = wcslen(ptr);
            new_text = Pool_Alloc(pool,
                (text_len + ptr_len + 1) * sizeof(WCHAR));
            if (!new_text)
                goto outofmem;
            wmemcpy(new_text, text, text_len);
            wmemcpy(new_text + text_len, ptr, ptr_len + 1);
            text = new_text;
            text_len = text_len + ptr_len;
        }
        break;
    }

    //
    // finish
    //

    wcscpy_s(value, value_size / sizeof(WCHAR), text);

    Pool_Delete(pool);

    return STATUS_SUCCESS;

outofmem:

    SbieApi_Log(2305, NULL);
    ExitProcess(-1);
    return STATUS_INSUFFICIENT_RESOURCES;
}


//---------------------------------------------------------------------------
// Dll_SkipHook
//---------------------------------------------------------------------------


_FX BOOLEAN Dll_SkipHook(const WCHAR *HookName)
{
    static WCHAR HookText[256];
    static BOOLEAN HookTextInit = TRUE;
    BOOLEAN found = FALSE;

    //
    // initialize hook text based on image name
    //

    if (HookTextInit) {

        HookTextInit = FALSE;

        Dll_GetSettingsForImageName(L"SkipHook", HookText, sizeof(HookText), NULL);
    } 
    
    //
    // query for a specific hook
    //

    if (HookName && *HookText) {

        ULONG len = wcslen(HookName);
        WCHAR *ptr = HookText;
        while (ptr) {
            while (*ptr == L',')
                ++ptr;
            if (_wcsnicmp(ptr, HookName, len) == 0) {
                found = TRUE;
                break;
            }
            ptr = wcschr(ptr, L',');
        }
    }

    return found;
}


//---------------------------------------------------------------------------
// Dll_JumpStub
//---------------------------------------------------------------------------


_FX void *Dll_JumpStub(void *OldCode, void *NewCode, ULONG_PTR Arg)
{
    UCHAR *code, *ptr;

    code = Dll_AllocCode128();
    ptr = code;

    //
    // build stub which loads eax with (code + 32) then jumps to NewCode
    //

#ifdef _M_ARM64

    *(ULONG*)ptr = 0x10000111;  // adr x17, 32 ; load pc register + 32 to x17
    ptr += sizeof(ULONG);

    //
    // write jump to NewCode
    //

    *(ULONG*)ptr = 0x58000048;	// ldr x8, 8
    ptr += sizeof(ULONG);
    *(ULONG*)ptr = 0xD61F0100;	// br x8
    ptr += sizeof(ULONG);
    *(ULONG_PTR *)ptr = (ULONG_PTR)NewCode;
    ptr += sizeof(ULONG_PTR);

#else

#ifdef _M_ARM64EC

    //
    // The emulator does not preserve RAX hence we use R9 instead
    // and save R9's value to the StubData area
    //

    *ptr = 0x4C;                // mov [rip+24], r9
    ++ptr;
    *(USHORT *)ptr = 0x0d89;
    ptr += sizeof(USHORT);
    *(ULONG *)ptr = (56-7);     // save 4th argument (r9) at StubData[3]
    ptr += sizeof(ULONG);   

    *(USHORT *)ptr = 0xB949;    // movabs r9
    ptr += sizeof(USHORT);
#elif _WIN64
    *(USHORT *)ptr = 0xB848;    // mov rax
    ptr += sizeof(USHORT);
#else ! _WIN64
    *ptr = 0xB8;                // mov eax
    ++ptr;
#endif _WIN64
    *(ULONG_PTR *)ptr = (ULONG_PTR)(code + 32);
    ptr += sizeof(ULONG_PTR);

    //
    // write jump to NewCode
    //

    *(USHORT *)ptr = 0x25FF;    // jmp dword/qword ptr [rip+6]
    ptr += sizeof(USHORT);
#ifdef _WIN64
    *(ULONG *)ptr = 0;
#else ! _WIN64
    *(ULONG *)ptr = (ULONG)(ptr + 4);
#endif _WIN64
    ptr += sizeof(ULONG);
    *(ULONG_PTR *)ptr = (ULONG_PTR)NewCode;
    ptr += sizeof(ULONG_PTR);

#endif

    //
    // write data at (code + 32)
    //

    ptr = code + 32;
    *(ULONG_PTR *)ptr = (ULONG_PTR)code;
    ptr += sizeof(ULONG_PTR);
    *(ULONG_PTR *)ptr = (ULONG_PTR)OldCode;
    ptr += sizeof(ULONG_PTR);
    *(ULONG_PTR *)ptr = (ULONG_PTR)Arg;
    ptr += sizeof(ULONG_PTR);
    // 4th argument goes here

    //
    // write eyecatcher at (code + 64)
    //

    *(ULONG *)(code + 64) = tzuk;

    return code;
}


//---------------------------------------------------------------------------
// Dll_JumpStubData
//---------------------------------------------------------------------------


#if !defined(_M_ARM64) && !defined(_M_ARM64EC)
#pragma warning(push)
#pragma warning(disable : 4716) // function must return a value
_FX ULONG_PTR *Dll_JumpStubData(void)
{
    //
    // returns pointer to StubData which is stored in eax at the time
    // when the replacement code is entered.  use as first statement
    // to make sure the value in eax is not modified
    //
    // StubData[0] = detour stub.  use with Dll_FreeCode128:
    //               Dll_FreeCode128((void *)StubData[0])
    //
    // StubData[1] = OldCode value passed to Dll_JumpStub()
    //
    // StubData[2] = Arg value passed to Dll_JumpStub()
    //
    //
}
#pragma warning(pop)
#endif


//---------------------------------------------------------------------------
// Dll_JumpStubDataForCode
//---------------------------------------------------------------------------


_FX ULONG_PTR *Dll_JumpStubDataForCode(void *StubCode)
{
    //
    // if StubCode identifies a function created by Dll_JumpStub then
    // return the StubData for that stub.  otherwise return NULL
    //

    ULONG_PTR rv = 0;
    __try {
        if (*(ULONG *)((UCHAR *)StubCode + 64) == tzuk)
            rv = (ULONG_PTR)StubCode + 32;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
    return (ULONG_PTR *)rv;
}


#ifndef _WIN64

#define WOW_SIZE 0x53
#define WOW_PATCH_SIZE 7

//---------------------------------------------------------------------------
// Dll_FixWow64Syscall
//---------------------------------------------------------------------------

extern ULONG Dll_Windows;
#define GET_ADDR_OF_PEB __readfsdword(0x30)
#define GET_PEB_IMAGE_BUILD (*(USHORT *)(GET_ADDR_OF_PEB + 0xac))

_FX void Dll_FixWow64Syscall(void)
{
    static UCHAR *_code = NULL;

    // NoSysCallHooks BEGIN
    if(Dll_CompartmentMode || SbieApi_QueryConfBool(NULL, L"NoSysCallHooks", FALSE))
        return;
    // NoSysCallHooks END

    //
    // the Wow64 thunking layer for syscalls in ntdll32 has several thunks:
    // thunk 0 calls the corresponding NtXxx export in the 64-bit ntdll.
    // other thunks issue the syscall instruction directly and are probably
    // intended as an optimization.  we want all 32-bit syscalls to go
    // through our SbieLow syscall interface, so we need to always force
    // use of thunk 0 rather than the optimization thunks.
    //
    // fs:[0xC0] stores the address of wow64cpu!X86SwitchTo64BitMode which
    // switches into 64-bit mode, we want to replace this with a small stub
    // which zeroes ecx (the thunk selector) and calls to the original code.
    //
    // note that fs:[0xC0] is thread specific so we have to fix this address
    // in every thread, so we are called from DllMain on DLL_THREAD_ATTACH
    //
    // note that on Windows 8, the thunk is selected by the high 16-bits of
    // eax, rather than by ecx, so we also adjust the value of eax
    //
    // Windows 10:
    // The wow64cpu!X86SwitchTo64BitMode has been removed.  Instead the switch
    // to 64 bit is done in 32 bit dll's in a function named Wow64SystemServiceCall
    // (ntdll.dll, user32.dll, gdi.dll,... etc).
    // The function uses a special jmp, e.g., (jmp far ptr 33h:$06).  The segment jmp using 
    // segment 0x33 will switch the cpu to 64bit (long mode).

    //; ---------------------------------------------------------------------------
    //_Wow64SystemServiceCall@0 proc near   ; CODE XREF: NtAccessCheck(x,x,x,x,x,x,x,x)
    //            mov     edx, large fs:30h
    //            mov     edx, [edx+254h]
    //            test    edx, 2 //if wow64
    //            jz      short IS_WOW64 
    //            int     2Eh               ; 32 bit syscall DOS 2+ internal - EXECUTE COMMAND
    //                                      ; DS:SI -> counted CR-terminated command string
    //            retn
    //; ---------------------------------------------------------------------------
    //            IS_WOW64:                 ; CODE XREF: Wow64SystemServiceCall()+13
    //            jmp     far ptr 33h:CPU_64 //cpu is switched to 64 bit mode @CPU_64
    //            CPU_64:
    //            jmp     dword ptr [r15+0F8h] //jmp to wow64cpu dispatch function
    //_Wow64SystemServiceCall@0 endp
    //; ---------------------------------------------------------------------------
    //In order to dispatch Wow64 in windows 10 correctly it is necessary to patch
    //wow64cpu.dll dispatch function directly.  This is done using fs:[0xc0] as a reference
    //,since it still points to a function in wow64cpu, to find this dispatch function.  
    //The following function finds and hooks this function for windows 10.


    if (Dll_IsWow64) {

        if (Dll_Windows >= 10) {
            if (!_code) {
                UCHAR *myAddress;
                ULONG_PTR wow64cpu = 0;
                void *RegionBase;
                SIZE_T RegionSize;
                ULONG OldProtect;
                ULONG dummy_prot;
                int i;

                //logic to find win64cpu dispatcher function by referencing the location
                //in fs:[0xc0] and looking for the function start signature (0x49 0x87 0xe6 : xch rsp,14)

                //end of previous function
                //48 CF     iretq
                //start of wow64 dispatch function
                //49 87 E6    xchg    rsp, r14
                //45 8B 06    mov     r8d, [r14]

                wow64cpu = (__readfsdword(0xC0) & 0xfffff0000);
                wow64cpu = DLL_FindWow64Target(wow64cpu);
                if (!wow64cpu) {
                    return;
                }
                wow64cpu -= 0x50;
                _try{
                    for (i = 0; i < 0x50 && *(ULONG *)wow64cpu != 0x45e68749; i++, wow64cpu++);
                // Look for end of previous function 
                if (*(USHORT *)(wow64cpu - 2) != 0xcf48) {
                    wow64cpu = 0;
                }
                }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                    wow64cpu = 0;
                }

                if (!wow64cpu) {
                    wow64cpu = 0;
                    return;
                }
                myAddress = (UCHAR *)VirtualAlloc(NULL, 0x100, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
                if (!myAddress) {
                    return;
                }

                //wow64cpu hook for windows 10
                memcpy(myAddress + WOW_PATCH_SIZE, (void *)wow64cpu, WOW_SIZE);
                *(ULONG*)(myAddress + 0) = 0xFF25c933;
                myAddress[4] = 0xff;
                myAddress[5] = 0;
                myAddress[6] = 0;
                RegionBase = (void *)wow64cpu;
                RegionSize = 0x10;
                _code = (UCHAR *)wow64cpu;

                NtProtectVirtualMemory(NtCurrentProcess(), &RegionBase, &RegionSize, PAGE_EXECUTE_READWRITE, &OldProtect);
                _code[0] = 0x51;        //push rcx
                // mov rcx,<8 byte address to myAddress>
                _code[1] = 0x48;
                _code[2] = 0xb9;
                *(ULONGLONG *)&_code[3] = (ULONGLONG)myAddress;
                *(ULONG *)&_code[11] = 0x240C8948;  // mov [rsp],rcx
                _code[15] = 0xc3;       // ret
                NtProtectVirtualMemory(NtCurrentProcess(), &RegionBase, &RegionSize, OldProtect, &dummy_prot);
                return;
            }
        }
        else {
            if (!_code) {
                ULONG X86SwitchTo64BitMode = __readfsdword(0xC0);
                _code = (UCHAR *)Dll_AllocCode128();
                // and eax, 0xFFFF ; xor ecx, ecx ; jmp xxx ( push xxx; ret; )
                *(ULONG *)(_code + 0) = 0xFF25C933;
                *(ULONG *)(_code + 4) = 0x680000FF;
                *(ULONG *)(_code + 8) = X86SwitchTo64BitMode;
                *(UCHAR *)(_code + 12) = 0xc3;
            }
            __writefsdword(0xC0, (ULONG)_code);
            return;
        }
    }
    return;
}


_FX ULONG_PTR  DLL_FindWow64Target(ULONG_PTR address)
{
    IMAGE_DOS_HEADER *dos_hdr = 0;
    IMAGE_NT_HEADERS *nt_hdrs = 0;
    IMAGE_SECTION_HEADER *section = 0;
    IMAGE_DATA_DIRECTORY *data_dirs = 0;
    ULONG_PTR ExportDirectoryVA;
    IMAGE_EXPORT_DIRECTORY *ExportDirectory = NULL;
    ULONG_PTR* names;
    ULONG_PTR* functions;
    ULONG_PTR imageBase = 0;
    DWORD numNames;

    dos_hdr = (IMAGE_DOS_HEADER *)address;

    if (dos_hdr->e_magic == 'MZ' || dos_hdr->e_magic == 'ZM')
    {
        nt_hdrs = (IMAGE_NT_HEADERS *)((UCHAR *)dos_hdr + dos_hdr->e_lfanew);

        if (nt_hdrs->Signature == IMAGE_NT_SIGNATURE)       // 'PE\0\0'
        {
            if (nt_hdrs->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
            {
                IMAGE_NT_HEADERS64 *nt_hdrs_64 = (IMAGE_NT_HEADERS64 *)nt_hdrs;
                IMAGE_OPTIONAL_HEADER64 *opt_hdr_64 = &nt_hdrs_64->OptionalHeader;
                data_dirs = &opt_hdr_64->DataDirectory[0];
                imageBase = (ULONG_PTR)opt_hdr_64->ImageBase;
            }
        }
    }

    if (!data_dirs[IMAGE_DIRECTORY_ENTRY_EXPORT].Size)
        return 0;

    ExportDirectoryVA = data_dirs[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

    if (!ExportDirectoryVA)
        return 0;

    ExportDirectory = (IMAGE_EXPORT_DIRECTORY *)(ExportDirectoryVA + address);
    names = (ULONG_PTR *)(ExportDirectory->AddressOfNames + address);
    functions = (ULONG_PTR *)(ExportDirectory->AddressOfFunctions + address);

    for (numNames = ExportDirectory->NumberOfNames; numNames; numNames--)
    {
        if (!_stricmp((const char *)(names[numNames - 1] + address), "TurboDispatchJumpAddressStart"))
            return functions[numNames - 1] + address;
    }

    return 0;
}

#endif ! _WIN64


_FX VOID SbieDll_DumpProlog(void* addr, const wchar_t* func)
{
    wchar_t buffer[1024];
    wchar_t* ptr = buffer;
    ptr += Sbie_snwprintf(ptr, 256, L"%S: %p ", func, addr);
    for (int i = 0; i < 48; i++)
        ptr += Sbie_snwprintf(ptr, 16, L"%02x", ((BYTE*)addr)[i]);
    wcscat(buffer, L"\r\n");
    OutputDebugStringW(buffer);
}
