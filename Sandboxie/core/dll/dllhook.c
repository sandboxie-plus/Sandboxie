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

typedef struct _TRACE_ENTRY {

  LIST_ELEM list_elem;

  char* name;
  UCHAR code[128];
} TRACE_ENTRY;

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
    LIST        trace;
} MODULE_HOOK;

LIST Dll_ModuleHooks;
CRITICAL_SECTION  Dll_ModuleHooks_CritSec;
BOOLEAN Dll_HookTrace = FALSE;

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

    Dll_HookTrace = SbieApi_QueryConfBool(NULL, L"HookTrace", FALSE);

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
        List_Init(&mod_hook->trace);
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

            // the table is not full
            if (ptrVTable->index < ptrVTable->maxEntries) {

                diff = (ULONG_PTR) & ((ULONG_PTR*)ptrVTable->offset)[ptrVTable->index];
                diff = diff - target;
                delta = diff;
                delta < 0 ? delta *= -1 : delta;

                // is DetourFunc in the jump range
                if (delta < maxDelta && ptrVTable->index <= ptrVTable->maxEntries) {
                    // found a good table, break and return it
                    break;
                }
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

        func = Hook_CheckChromeHook((void *)target, (void*)GET_PEB_IMAGE_BASE);
        if (func && func != (UCHAR*)-1) {
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

    MODULE_HOOK* mod_hook = SbieDll_GetModuleHookAndLock(module, tzuk | 0xFF); // 0xFF - executable
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
    MODULE_HOOK* mod_hook = SbieDll_GetModuleHookAndLock(module, (tzuk & 0xFFFFFF00) | 0xEC); // 0xEC - executable ARM64 Emulation Compatible
#else
    MODULE_HOOK* mod_hook = SbieDll_GetModuleHookAndLock(module, tzuk | 0xFF); // 0xFF - executable
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

    RegionBase = &func[0];
    RegionSize = 16;

    if (!VirtualProtect(RegionBase, RegionSize, PAGE_EXECUTE_READWRITE, &prot)) {
        ULONG err = GetLastError();
        SbieApi_Log(2303, _fmt2, SourceFuncName, 33, err);
        func = NULL;
        goto finish;
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
        // write branch to jump entry
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
    // restore protection and flush instruction cache
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
// SbieDll_HookFunc
//---------------------------------------------------------------------------

#define HOOK_STAT_CHROME        0x00000001
#define HOOK_STAT_CHROME_FAIL   0x00000002
#define HOOK_STAT_NO_FFS        0x00000004
#define HOOK_STAT_SKIPPED       0x00000008
#define HOOK_STAT_TRACE         0x00000100
#define HOOK_STAT_SYSCALL       0x00000200 // ARM64 EC only
#define HOOK_STAT_INTERESTING   0x000000FF

_FX void *SbieDll_HookFunc(
    const char *SourceFuncName, void *SourceFunc, void *DetourFunc, HMODULE module, DWORD* pHookStats)
{
    //
    // Chrome sandbox support
    //

    void* ChromeFunc = Hook_CheckChromeHook(SourceFunc, (void*)GET_PEB_IMAGE_BASE);
    if (ChromeFunc) {
        if (pHookStats) *pHookStats |= HOOK_STAT_CHROME;
        if (ChromeFunc != (void*)-1)
            SourceFunc = ChromeFunc;
        else {
            //SbieApi_Log(2328, _fmt1, SourceFuncName, 1);
            if (pHookStats) *pHookStats |= HOOK_STAT_CHROME_FAIL;
        }
    }

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
    // Note: this mechanism is only available during initialization as
    // at the end of Dll_Ordinal1 we dispose of the syscall/inject data area
    // therefore any Nt function hooks must be set up from the get go
    //

    extern ULONG* SbieApi_SyscallPtr;
    if (module == Dll_Ntdll && *(USHORT*)&SourceFuncName[0] == 'tN' && SbieApi_SyscallPtr) {

        if (pHookStats) *pHookStats |= HOOK_STAT_SYSCALL;

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
    {
        void* SourceFuncEC = Hook_GetFFSTarget(SourceFunc);
        if (SourceFuncEC) {
    
            return SbieDll_Hook_arm(SourceFuncName, SourceFuncEC, DetourFunc, module);
        }
        else
        {
            
            if(strcmp(SourceFuncName, "ShellExecuteExW") != 0) // for these functions its fine
                SbieApi_Log(2329, _fmt1, SourceFuncName, 1);
            if (pHookStats) *pHookStats |= HOOK_STAT_NO_FFS;
        }
    }

#endif
#ifdef _M_ARM64
    return SbieDll_Hook_arm(SourceFuncName, SourceFunc, DetourFunc, module);
#else
    return SbieDll_Hook_x86(SourceFuncName, SourceFunc, DetourFunc, module);
#endif
}


//---------------------------------------------------------------------------
// SbieDll_Hook
//---------------------------------------------------------------------------


_FX void *SbieDll_Hook(
    const char *SourceFuncName, void *SourceFunc, void *DetourFunc, HMODULE module)
{
    const WCHAR* ModuleName = NULL;
    DWORD HookStats = 0;
    void* func = NULL;

    if (SbieDll_FuncSkipHook(SourceFuncName)) {
        HookStats = HOOK_STAT_SKIPPED;
        func = SourceFunc;
        goto finish;
    }

    //
    // when hooking a function we can detour the detour and log the call
    //

    PDWORD64 pDetourFunc = NULL;
    if (Dll_ApiTrace) {

        if(!DetourFunc)
			HookStats |= HOOK_STAT_TRACE;

        ModuleName = Trace_FindModuleByAddress((void*)module);

#ifdef _M_ARM64EC
        MODULE_HOOK* mod_hook = SbieDll_GetModuleHookAndLock(module, (tzuk & 0xFFFFFF00) | 0xEC); // 0xEC - executable ARM64 Emulation Compatible
#else
        MODULE_HOOK* mod_hook = SbieDll_GetModuleHookAndLock(module, tzuk | 0xFF); // 0xFF - executable
#endif

        TRACE_ENTRY* pTrace = Pool_Alloc(mod_hook->pool, sizeof(TRACE_ENTRY));
        List_Insert_After(&mod_hook->trace, NULL, pTrace);

        UCHAR* NewDetour = pTrace->code;

        extern void ApiInstrumentationAsm(void);

        typedef union
        {
	        PBYTE pB;
	        PWORD  pW;
	        PDWORD pL;
	        PDWORD64 pQ;
        } TYPES;

        TYPES ip;
        ip.pB = NewDetour;

        // store detour address
        if (DetourFunc == NULL)
            pDetourFunc = ip.pQ++;
        else
            *ip.pQ++ = (ULONG_PTR)DetourFunc;

        // store full function name
        int len;
        if (ModuleName) {
            len = Sbie_snprintf(ip.pB, 96, "%S!%s", ModuleName, SourceFuncName);
            pTrace->name = ip.pB + wcslen(ModuleName) + 1;
        } else {
            len = Sbie_snprintf(ip.pB, 96, "%s", SourceFuncName);
            pTrace->name = ip.pB + 1;
        }
        ip.pB += len + 1;
        
        ULONG_PTR tmp = ((ULONG_PTR)ip.pB & 0x03);
        if (tmp != 0) // fix alignment, needed for ARM64
            ip.pB += 0x4 - tmp;

        // create new detour
        DetourFunc = ip.pB;
#if defined(_M_ARM64) || defined(_M_ARM64EC)
        //*ip.pL++ = 0xD43E0000;  // brk #0xF000
        //0
        *ip.pL++ = 0x580000b1;	// ldr x17, 20 - NewDetour
        *ip.pL++ = 0x58000050;	// ldr x16, 8 - ApiInstrumentationAsm
        *ip.pL++ = 0xD61F0200;	// br x16
        *ip.pQ++ = (ULONG_PTR)ApiInstrumentationAsm; 
        *ip.pQ++ = (ULONG_PTR)NewDetour;
        // 28
        FlushInstructionCache(GetCurrentProcess(), pTrace->code, 32);
#elif _WIN64
        //*ip.pB++ = 0xCC;      // int 3
        // 0
        *ip.pW++ = 0xB848;    // mov rax, NewDetour     // move data header pointer to rax
        *ip.pQ++ = (ULONG_PTR)NewDetour;
        *ip.pW++ = 0x25FF;    // jmp qword ptr [rip+0]  // jump to proxy function
        *ip.pL++ = 0x00000000;
        *ip.pQ++ = (ULONG_PTR)ApiInstrumentationAsm;
        // 24
#else
        //*ip.pB++ = 0xCC;      // int 3
        // 0
        *ip.pB++ = 0xB8;    // mov eax, NewDetour       // move data header pointer to eax
        *ip.pL++ = (ULONG_PTR)NewDetour;
        *ip.pB++ = 0xBA;    // mov edx, ApiInstrumentationAsm   // jump to proxy function
        *ip.pL++ = (ULONG_PTR)ApiInstrumentationAsm;
        *ip.pW++ = 0xE2FF;   // jmp edx
        // 12
#endif

        LeaveCriticalSection(&Dll_ModuleHooks_CritSec);
    }

    //
    // install the hook
    //

    func = SbieDll_HookFunc(SourceFuncName, SourceFunc, DetourFunc, module, &HookStats);

    //
    // when tracing API calls of functions that are not normally hooked,
    // we did not have an initial detour and have passed NULL
    // in this case we set the trampoline itself as final detour target
    //

    if (pDetourFunc) {
        *pDetourFunc = (DWORD64)func;
        func = NULL;
    }

finish:
    if (Dll_HookTrace || (HookStats & HOOK_STAT_INTERESTING) || !func) {

        if (!ModuleName)
            ModuleName = Trace_FindModuleByAddress((void*)module);

        WCHAR dbg[1024];
		WCHAR* dbg_ptr = dbg;
		size_t dbg_size = ARRAYSIZE(dbg);
        int len = Sbie_snwprintf(dbg_ptr, dbg_size, L"%sHooking%s: %s!%S", 
            !func ? L"FAILED " : (HookStats & HOOK_STAT_SKIPPED) ? L"Skipped " : L"", 
            (HookStats & HOOK_STAT_TRACE) ? L" (trace)" : L"", 
            ModuleName ? ModuleName : L"unknown", 
            SourceFuncName);
		dbg_ptr += len;
		dbg_size -= len;
        if (HookStats & HOOK_STAT_CHROME_FAIL) {
            len = Sbie_snwprintf(dbg_ptr, dbg_size, L" (Chrome Hook Unresolved)");
            dbg_ptr += len;
            dbg_size -= len;
        }
        else if (HookStats & HOOK_STAT_CHROME) {
            len = Sbie_snwprintf(dbg_ptr, dbg_size, L" (Chrome Hook Hooked)");
            dbg_ptr += len;
            dbg_size -= len;
        }
#ifdef _M_ARM64EC
        if (HookStats & HOOK_STAT_NO_FFS) {
            len = Sbie_snwprintf(dbg_ptr, dbg_size, L" FFS Target not found, hooked x86 code instead");
            dbg_ptr += len;
            dbg_size -= len;
        }
#endif
        SbieApi_MonitorPutMsg(MONITOR_HOOK | MONITOR_TRACE | ((HookStats & HOOK_STAT_SKIPPED) ? MONITOR_OPEN : 0), dbg);
    }

    return func;
}


//---------------------------------------------------------------------------
// SbieDll_IsTraced
//---------------------------------------------------------------------------


_FX BOOLEAN SbieDll_IsTraced(const char* name, MODULE_HOOK* mod_hook)
{
    TRACE_ENTRY* entry = List_Head(&mod_hook->trace);
    while (entry) {

        TRACE_ENTRY* next_entry = List_Next(entry);

        if (strcmp(entry->name, name) == 0)
            return TRUE;
            
        entry = next_entry;
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// SbieDll_TraceModule
//---------------------------------------------------------------------------

DWORD64 FindImagePosition(DWORD rva, void* pNTHeader, DWORD64 imageBase);

_FX void SbieDll_TraceModule(HMODULE module)
{
    if (Dll_ApiTrace) {

        //
        // check if this dll should be fully hooked
        //

        const WCHAR* ModuleName = Trace_FindModuleByAddress((void*)module);
        if (!SbieDll_CheckStringInList(ModuleName, NULL, L"ApiTraceDll"))
            return;

        //
        // Check the mod_hook->trace list and add tracing to functions which has not yet been hooked
        // 

        MODULE_HOOK* mod_hook = SbieDll_GetModuleHookAndLock(module, tzuk | 0xFF); // 0xFF - executable

        ULONG_PTR DllBase = (ULONG_PTR)module;

#ifdef _WIN64
        PIMAGE_NT_HEADERS64 nt_hdrs = ((IMAGE_NT_HEADERS64*)(DllBase + ((IMAGE_DOS_HEADER*)DllBase)->e_lfanew));
        IMAGE_OPTIONAL_HEADER64* opt_hdr = &nt_hdrs->OptionalHeader;
#else
        PIMAGE_NT_HEADERS32 nt_hdrs = ((IMAGE_NT_HEADERS32*)(DllBase + ((IMAGE_DOS_HEADER*)DllBase)->e_lfanew));
        IMAGE_OPTIONAL_HEADER32* opt_hdr = &nt_hdrs->OptionalHeader;
#endif

        if (opt_hdr->NumberOfRvaAndSizes) {

            IMAGE_DATA_DIRECTORY* dir0 = &opt_hdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

            if (dir0->VirtualAddress) {

                DWORD64 dir0Address = FindImagePosition(dir0->VirtualAddress, nt_hdrs, DllBase);

                IMAGE_EXPORT_DIRECTORY* exports = (IMAGE_EXPORT_DIRECTORY*)dir0Address;

                ULONG* names = (ULONG*)((DWORD64)exports + exports->AddressOfNames - dir0->VirtualAddress);
                USHORT* ordinals = (USHORT*)((DWORD64)exports + exports->AddressOfNameOrdinals - dir0->VirtualAddress);
                ULONG* functions = (ULONG*)((DWORD64)exports + exports->AddressOfFunctions - dir0->VirtualAddress);

                for (ULONG i = 0; i < exports->NumberOfNames; ++i) {

                    char* name = (char*)((DWORD64)exports + names[i] - dir0->VirtualAddress);
                    if (*name == '_' || *name == '?') continue;

                    if (SbieDll_IsTraced(name, mod_hook))
                        continue;

                    if (SbieDll_CheckStringInListA(name, NULL, L"ApiSkipTrace"))
                        continue;

                    if (ordinals[i] < exports->NumberOfFunctions) {

                        ULONG_PTR proc = DllBase + functions[ordinals[i]];

                        SbieDll_Hook(name, (void*)proc, NULL, module);
                    }
                }
            }
        }

        LeaveCriticalSection(&Dll_ModuleHooks_CritSec);
    }
}


//---------------------------------------------------------------------------
// SbieDll_UnHookModule
//---------------------------------------------------------------------------


_FX void SbieDll_UnHookModule(HMODULE module)
{
    EnterCriticalSection(&Dll_ModuleHooks_CritSec);

    MODULE_HOOK* mod_hook = List_Head(&Dll_ModuleHooks);
    while (mod_hook) {

        MODULE_HOOK* next_mod_hook = List_Next(mod_hook);

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

            // we may have more then one entry with different tags on ARM64
#ifndef _M_ARM64EC
            break;
#endif
        }

        mod_hook = next_mod_hook;
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

		SbieDll_GetSettingsForName(NULL, Dll_ImageName, L"SkipHook", HookText, sizeof(HookText), L"");
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
