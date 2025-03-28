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
// Various generic hooking helpers
//---------------------------------------------------------------------------


/******************************************************
* WARNING: This code must be position independent !!! *
*          It is used by the LowLevel.dll shell code! *
*******************************************************/


#ifdef _WIN64
#define SET_LAST_ERROR(val) __writegsdword(0x68, (val))
#else
#define SET_LAST_ERROR(val) __readfsdword(0x34, (val))
#endif

//---------------------------------------------------------------------------
// FindDllExport2
//---------------------------------------------------------------------------


static UCHAR *FindDllExport2(
    void *DllBase, IMAGE_DATA_DIRECTORY *dir0, const UCHAR *ProcName, ULONG* pErr)
{
    void *proc = NULL;
    ULONG i, j, n;

    if (dir0->VirtualAddress && dir0->Size) {

        IMAGE_EXPORT_DIRECTORY *exports = (IMAGE_EXPORT_DIRECTORY *)
            ((UCHAR *)DllBase + dir0->VirtualAddress);

        ULONG *names = (ULONG *)
            ((UCHAR *)DllBase + exports->AddressOfNames);

        for (n = 0; ProcName[n]; ++n)
            ;

        for (i = 0; i < exports->NumberOfNames; ++i) {

            UCHAR *name = (UCHAR *)DllBase + names[i];
            for (j = 0; j < n; ++j) {
                if (name[j] != ProcName[j])
                    break;
            }
            if (j == n) {

                USHORT *ordinals = (USHORT *)
                    ((UCHAR *)DllBase + exports->AddressOfNameOrdinals);
                if (ordinals[i] < exports->NumberOfFunctions) {

                    ULONG *functions = (ULONG *)
                        ((UCHAR *)DllBase + exports->AddressOfFunctions);

                    proc = (UCHAR *)DllBase + functions[ordinals[i]];
                    break;
                }
            }
        }

        if (proc && (ULONG_PTR)proc >= (ULONG_PTR)exports
                 && (ULONG_PTR)proc <  (ULONG_PTR)exports + dir0->Size) {

            //
            // if the export points inside the export table, then it is a
            // forwarder entry.  we don't handle these, because none of the
            // exports we need is a forwarder entry.  if this changes, we
            // might have to scan LDR tables to find the target dll
            //

            if (pErr) *pErr = 0xc;
            proc = NULL;
        }
    }

    return proc;
}


//---------------------------------------------------------------------------
// FindDllExport
//---------------------------------------------------------------------------


_FX UCHAR *FindDllExport(void *DllBase, const UCHAR *ProcName, ULONG* pErr)
{
    IMAGE_DOS_HEADER *dos_hdr;
    IMAGE_NT_HEADERS *nt_hdrs;
    UCHAR* func_ptr = NULL;

    //
    // find the DllMain entrypoint for the dll
    //

    dos_hdr = (void *)DllBase;
    if (dos_hdr->e_magic != 'MZ' && dos_hdr->e_magic != 'ZM') {
        if (pErr) *pErr = 0xa;
        return NULL;
    }
    nt_hdrs = (IMAGE_NT_HEADERS *)((UCHAR *)dos_hdr + dos_hdr->e_lfanew);
    if (nt_hdrs->Signature != IMAGE_NT_SIGNATURE) {     // 'PE\0\0'
        if (pErr) *pErr = 0xb;
        return NULL;
    }

    if (nt_hdrs->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {

        IMAGE_NT_HEADERS32 *nt_hdrs_32 = (IMAGE_NT_HEADERS32 *)nt_hdrs;
        IMAGE_OPTIONAL_HEADER32 *opt_hdr_32 =
            &nt_hdrs_32->OptionalHeader;

        if (opt_hdr_32->NumberOfRvaAndSizes) {

            IMAGE_DATA_DIRECTORY *dir0 = &opt_hdr_32->DataDirectory[0];
            func_ptr = FindDllExport2(DllBase, dir0, ProcName, pErr);
        }
    }

#ifdef _WIN64

    else if (nt_hdrs->OptionalHeader.Magic ==
                                         IMAGE_NT_OPTIONAL_HDR64_MAGIC) {

        IMAGE_NT_HEADERS64 *nt_hdrs_64 = (IMAGE_NT_HEADERS64 *)nt_hdrs;
        IMAGE_OPTIONAL_HEADER64 *opt_hdr_64 =
            &nt_hdrs_64->OptionalHeader;

        if (opt_hdr_64->NumberOfRvaAndSizes) {

            IMAGE_DATA_DIRECTORY *dir0 = &opt_hdr_64->DataDirectory[0];
            func_ptr = FindDllExport2(DllBase, dir0, ProcName, pErr);
        }
    }

#endif _WIN64

    return func_ptr;
}


#if defined(_M_ARM64) || defined(_M_ARM64EC)
#include "common/arm64_asm.h"


//---------------------------------------------------------------------------
// Hook_GetXipTarget
//---------------------------------------------------------------------------


_FX void* Hook_GetXipTarget(void* ptr, int mode) 
{
    void* addr = ptr;

	ADRP adrp;
	adrp.OP = ((ULONG*)ptr)[0];
	if (!IS_ADRP(adrp) || adrp.Rd != 16) // adrp x16, #0x4c000 
		return ptr;

    LONG delta = (adrp.immHi << 2 | adrp.immLo) << 12;

    if (mode == 0) // default import jump mode
    {
        LDR ldr;
        ldr.OP = ((ULONG*)ptr)[1];
        if (!IS_LDR(ldr) || ldr.Rn != 16 || ldr.Rt != 16) // ldr  x16, [x16, #0xa8]
            return ptr;

        delta += (ldr.imm12 << ldr.size);

        addr = *((void**)(((UINT_PTR)ptr & ~0xFFF) + delta));
    }
    else if (mode == 1)
    {
        ADD add;
        add.OP = ((ULONG*)ptr)[1];
        if(!IS_ADD(add) || add.Rn != 16 || add.Rd != 16)
            return ptr;

        delta += (add.imm12 << add.shift);

        addr = ((void*)(((UINT_PTR)ptr & ~0xFFF) + delta));
    }
    //else if (mode == 2)
    //{
    //    LDUR ldur;
    //    ldur.OP = ((ULONG*)ptr)[1];
    //    if (!IS_LDUR(ldur) || ldur.Rn != 16 || ldur.Rt != 16) // ldur  x16, [x16, #0xa9]
	//        return ptr;
    // 
    //    delta += (ldr.imm12 << ldr.size);
    //
    //    addr = *((void**)(((UINT_PTR)ptr & ~0xFFF) + delta));
    //}

	BR br;
	br.OP = ((ULONG*)ptr)[2];
	if (!IS_BR(br) || br.rn != 16) // br   x16
		return ptr;

	return addr;
}


//---------------------------------------------------------------------------
// Hook_GetFFSTargetOld
//---------------------------------------------------------------------------


_FX void* Hook_GetFFSTargetOld(UCHAR* SourceFunc)
{
    //
    // FFS Sequence: Win10 & Win11 RTM
    // 
    //  48 8B FF            mov         rdi,rdi  
    //  55                  push        rbp  
    //  48 8B EC            mov         rbp,rsp  
    //  5D                  pop         rbp  
    //  90                  nop  
    //  E9 02 48 18 00      jmp         #__GSHandlerCheck_SEH_AMD64+138h (07FFB572B8190h) 
    //

    if (*(UCHAR *)SourceFunc == 0x48 && // mov         rdi,rdi  
        *(USHORT *)((UCHAR *)SourceFunc + 1) == 0xFF8B) 
        SourceFunc = (UCHAR *)SourceFunc + 3;
    if (*(UCHAR *)SourceFunc == 0x55)   // push        rbp
        SourceFunc = (UCHAR *)SourceFunc + 1;
    if (*(UCHAR *)SourceFunc == 0x48 && // mov         rbp,rsp 
        *(USHORT *)((UCHAR *)SourceFunc + 1) == 0xEC8B)
        SourceFunc = (UCHAR *)SourceFunc + 3;
    if (*(UCHAR *)SourceFunc == 0x5D)   // pop         rbp 
        SourceFunc = (UCHAR *)SourceFunc + 1;
    if (*(UCHAR *)SourceFunc == 0x90)   // nop
        SourceFunc = (UCHAR *)SourceFunc + 1;
    if (*(UCHAR *)SourceFunc == 0xE9) {  // jmp        07FFB572B8190h

        LONG diff = *(LONG*)(SourceFunc + 1);
        return (UCHAR*)SourceFunc + 5 + diff;
    }

    return NULL;
}


//---------------------------------------------------------------------------
// Hook_GetFFSTargetNew
//---------------------------------------------------------------------------


_FX void* Hook_GetFFSTargetNew(UCHAR* SourceFunc)
{
    //
    // FFS Sequence: Win11 build >= 22621.819 (or 22621.382)
    // 
    //  48 8B C4             mov         rax,rsp  
    //  48 89 58 20          mov         qword ptr [rax+20h],rbx  
    //  55                   push        rbp  
    //  5D                   pop         rbp  
    //  E9 E2 9E 17 00       jmp         #LdrLoadDll (07FFECCB748E0h)  
    //

    if (*(UCHAR *)SourceFunc == 0x48 && // mov         rax,rsp
        *(USHORT *)((UCHAR *)SourceFunc + 1) == 0xC48B) 
        SourceFunc = (UCHAR *)SourceFunc + 3;
    if (*(ULONG *)SourceFunc == 0x20588948) // mov     qword ptr [rax+20h],rbx 
        SourceFunc = (UCHAR *)SourceFunc + 4;
    if (*(UCHAR *)SourceFunc == 0x55)   // push        rbp
        SourceFunc = (UCHAR *)SourceFunc + 1;
    if (*(UCHAR *)SourceFunc == 0x5D)   // pop         rbp 
        SourceFunc = (UCHAR *)SourceFunc + 1;
    if (*(UCHAR *)SourceFunc == 0xE9) {  // jmp        07FFB572B8190h

        LONG diff = *(LONG*)(SourceFunc + 1);
        return (UCHAR*)SourceFunc + 5 + diff;
    }

    return NULL;
}


//---------------------------------------------------------------------------
// Hook_GetFFSTarget
//---------------------------------------------------------------------------


_FX void* Hook_GetFFSTarget(UCHAR* SourceFunc)
{
    //
    // if we first have a jump to the FFS sequence, follow it
    //

    if (*(UCHAR *)SourceFunc == 0x48 && // rex.W
            *(USHORT *)((UCHAR *)SourceFunc + 1) == 0x25FF) { // jmp QWORD PTR [rip+xx xx xx xx];
        // 48 FF 25 is same as FF 25
        SourceFunc = (UCHAR *)SourceFunc + 1;
    }

    if (*(USHORT *)SourceFunc == 0x25FF) { // jmp QWORD PTR [rip+xx xx xx xx];

        LONG diff = *(LONG *)((ULONG_PTR)SourceFunc + 2);
        ULONG_PTR target = (ULONG_PTR)SourceFunc + 6 + diff;

        SourceFunc = (void *)*(ULONG_PTR *)target;
    }

    //
    // check if the function is a FFS sequence and if so 
    // return the address of the target native function
    //

    void* pTarget = Hook_GetFFSTargetOld(SourceFunc);
    if (!pTarget)
        pTarget = Hook_GetFFSTargetNew(SourceFunc);
    return pTarget;
}


//---------------------------------------------------------------------------
// Hook_GetSysCallIndex
//---------------------------------------------------------------------------


_FX USHORT Hook_GetSysCallIndex(UCHAR* SourceFunc)
{

    //
    // Standard syscall function
    //
    //  4C 8B D1                mov     r10, rcx
    //  B8 19000000             mov     eax, 0x19
    //  F6 04 25 0803FE7F 01    test    byte [0x7ffe0308], 0x1
    //  75 03                   jne     +5
    //
    //  0F 05                   syscall 
    //  C3                      ret
    //
    //  CD 2E                   int     0x2e
    //  C3                      ret    
    //

    USHORT index = -1;

    if (*(UCHAR *)SourceFunc == 0x4C && // mov         r10,rcx  
            *(USHORT *)((UCHAR *)SourceFunc + 1) == 0xD18B) 
        SourceFunc = (UCHAR *)SourceFunc + 3;

    if (*(UCHAR *)SourceFunc == 0xB8) { // mov         eax, 0x55
        
        LONG value = *(LONG*)(SourceFunc + 1);
        if((value & 0xFFFF0000) == 0)
            index = (SHORT)value;
    }

    return index;
}


//---------------------------------------------------------------------------
// Hook_GetSysCallFunc
//---------------------------------------------------------------------------


_FX ULONG Hook_GetSysCallFunc(ULONG* aCode, void** pHandleStubHijack)
{
    //  0: ff4300d1    sub  sp, sp, #0x10
    if (aCode[0] != 0xd10043ff)
        return -1;

    //  1: 10000090    adrp x16, #0xffffffffffffe000       ; data_180165740
    //  2: 10021d91    add  x16, x16, #0x740               ; data_180165740

    //  3: f00300f9    str  x16, [sp]
    if (aCode[3] != 0xf90003f0)
        return -1;

    //  4: e10000d4    svc  #0x07
    SVC svc;
    svc.OP = aCode[4];
    if (!IS_SVC(svc))
        return -1;

    // 5: e90340f9    ldr  x9, [sp]
    if (aCode[5] != 0xf94003e9)
        return -1;

    // 6: 10000090    adrp x16, #0xffffffffffffe000       ; data_180165740
    // 7: 10021d91    add  x16, x16, #0x740               ; data_180165740

    // 8: 100209eb    subs x16, x16, x9
    if (aCode[8] != 0xeb090210)
        return -1;

    // 9: ff430091    add  sp, sp, #0x10
    if (aCode[9] != 0x910043ff)
        return -1;

    //  10: c1eaff54    b.ne #0xffffffffffffe520            ; HandleStubHijack

    B_COND b_cond;
    b_cond.OP = aCode[10];
    if (!IS_B_COND(b_cond) && b_cond.cond != 1)
        return -1;
    if (pHandleStubHijack) {
        LONG offset = b_cond.imm19 << 2;
        if (offset & (1 << 20)) // if this is negative
            offset |= 0xFFF00000; // make it properly negative
        *pHandleStubHijack = (void*)((UINT_PTR)(aCode + 10) + offset);
    }

    //  11: 090080d2    movz x9, #0
    if (aCode[11] != 0xd2800009)
        return -1;

    //  12: c0035fd6    ret  
    if (aCode[12] != 0xd65f03c0)
        return -1;

    return svc.imm16;
}

#endif


//---------------------------------------------------------------------------
// Hook_CheckChromeHook
//---------------------------------------------------------------------------


#ifdef _M_ARM64

#define MAX_FUNC_OPS (0x80/4)

ULONGLONG* findChromeTarget(unsigned char* addr)
{
    int i, j;
    ULONGLONG target;
    ULONGLONG * ChromeTarget = NULL;
    if (!addr) return NULL;
    // look for ADRP to some register followed (not imminently) by an LDR for and with the same register
    for (i = 0; i < MAX_FUNC_OPS && !ChromeTarget; i++) {
        ADRP adrp;
        adrp.OP = ((ULONG*)addr)[i];
        if (IS_ADRP(adrp)) {
            for (j = i + 1; j < MAX_FUNC_OPS && !ChromeTarget; j++) {
                LDR ldr;
                ldr.OP = ((ULONG*)addr)[j];
                if (IS_LDR(ldr) && ldr.Rn == adrp.Rd) { // ldr.Rt can be different ideally x0 or its same as adrp.Rd
                    LONG delta = (adrp.immHi << 2 | adrp.immLo) << 12;
                    delta += (ldr.imm12 << ldr.size);
                    target = ((((UINT_PTR) & ((ULONG*)addr)[i]) & ~0xFFF) + delta);
                    ChromeTarget = *(ULONGLONG **)target;
                }
            }
        }
    }
    return ChromeTarget;
}

#elif _WIN64

#define MAX_FUNC_SIZE 0x76

ULONGLONG * findChromeTarget(unsigned char* addr)
{
    int i = 0;
    ULONGLONG target;
    ULONGLONG * ChromeTarget = NULL;
    if (!addr) return NULL;

    //Look for mov rcx,[target 4 byte offset] or in some cases mov rax,[target 4 byte offset]
    //So far the offset has been positive between 0xa00000 and 0xb00000 bytes;
    //This may change in a future version of chrome
    for (i = 0; i < MAX_FUNC_SIZE; i++) {

        // some chromium 90+ derivatives replace the function with a return 1 stub
        // b8 01 00 00 00   mov eax,1
        // c3               ret
        // cc               int 3
        if (addr[i] == 0xB8                         && addr[i + 5] == 0xC3 && addr[i + 6] == 0xCC)
            return NULL;

        // vivaldi browser variation
        // 66 B8 01 00      mov ax,1  
        // C3               ret  
        // CC               int 3
        if (addr[i] == 0x66 && addr[i + 1] == 0xB8  && addr[i + 4] == 0xC3 && addr[i + 5] == 0xCC)
            return NULL;

        if ((*(USHORT *)&addr[i] == 0x8b48)) {

            //Look for mov rcx,[target 4 byte offset] or in some cases mov rax,[target 4 byte offset]
            if ((addr[i + 2] == 0x0d || addr[i + 2] == 0x05)) {
                LONG delta;
                target = (ULONG_PTR)(addr + i + 7);
                delta = *(LONG *)&addr[i + 3];

                //check if offset is close to the expected value (is positive and less than 0x100000 as of chrome 64) 
        //      if (delta > 0 && delta < 0x100000 )  { //may need to check delta in a future version of chrome
                target += delta;
                ChromeTarget = *(ULONGLONG **)target;

                // special case when compiled using mingw toolchain
                // mov rcx,qword ptr [rax+offset] or mov rcx,qword ptr [rcx+offset]
                if ((*(USHORT *)&addr[i + 7] == 0x8B48)) 
                {
                    if (addr[i + 9] == 0x48 || addr[i + 9] == 0x49)
                        delta = addr[i + 10];
                    else if (addr[i + 9] == 0x88 || addr[i + 9] == 0x89)
                        delta = *(ULONG*)&addr[i + 10];
                    else
                        break;
                    target = (ULONGLONG)ChromeTarget + delta;
                    ChromeTarget = *(ULONGLONG **)target;
                }

        //      }
                break;
            }
        }
    }

    return ChromeTarget;
}

ULONGLONG * findFirefoxTarget(unsigned char* addr, unsigned char* g_originals)
{
    if (!addr) 
        return NULL;

    ULONGLONG * ChromeTarget = NULL;

    //
    // Look for one fo the following opcodes
    // mov rcx,[target 4 byte offset] 
    // mov rax,[target 4 byte offset]
    // mov rdi,[target 4 byte offset]
    // mov r15,[target 4 byte offset]
    // and check if they target it within the exported g_originals variable
    // 
    // This may change in a future version of firefox
    //

    for (int i = 0; i < MAX_FUNC_SIZE; i++) {

        if ((*(USHORT *)&addr[i] == 0x8b48) || (*(USHORT *)&addr[i] == 0x8b4c)) {

            if ((addr[i + 2] == 0x0d) || (addr[i + 2] == 0x05) || (addr[i + 2] == 0x3d)) {

                ULONG_PTR target = (ULONG_PTR)(addr + i + 7);
                LONG delta = *(LONG *)&addr[i + 3];
                target += delta;

                // must point into g_originals which has INTERCEPTOR_MAX_ID entries (as of FF 138 it 53)
                if (target >= (ULONG_PTR)g_originals && target <= (ULONG_PTR)(g_originals + sizeof(ULONG_PTR) * 60)) {

                    ChromeTarget = *(ULONGLONG**)target;
                    break;
                }
            }
        }
    }

    return ChromeTarget;
}
#endif

_FX void* Hook_CheckChromeHook(void *SourceFunc, void* ProcBase)
{
    if (!SourceFunc)
        return NULL;
#ifdef _M_ARM64
    ULONG *func = (ULONG *)SourceFunc;

    if (func[0] == 0x58000050       // ldr         xip0,ZwCreateFile+8h (07FF99A6FF8C8h)  
     && func[1] == 0xD61F0200) {    // ldr         br          xip0

        ULONGLONG *longlongs = *(ULONGLONG **)&func[2];
        ULONGLONG *chrome64Target = findChromeTarget((unsigned char *)longlongs);

        if (!chrome64Target)
            return (void*)-1; // failure
        return chrome64Target;
    }
#elif _WIN64
    UCHAR *func = (UCHAR *)SourceFunc;
    ULONGLONG *longlongs = NULL;
    ULONGLONG *chrome64Target = NULL;

    if (func[0] == 0x50 &&	//push rax
        func[1] == 0x48 &&	//mov rax,?
        func[2] == 0xb8) 
    {
        longlongs = *(ULONGLONG **)&func[3];
        chrome64Target = findChromeTarget((unsigned char *)longlongs);
    }
    // Chrome 49+ 64bit hook
    // mov rax, <target> 
    // jmp rax 
    else if (func[0] == 0x48 && //mov rax,<target>
        func[1] == 0xb8 &&
        *(USHORT *)&func[10] == 0xe0ff) //jmp rax
    {
        longlongs = *(ULONGLONG **)&func[2];

        ULONG uError = 0;
        // Note: A normal string like L"text" would not result in position independent code !!!
        // hence we create a string array and fill it byte by byte
        wchar_t s_originals[] = { 'g', '_', 'o', 'r', 'i', 'g', 'i', 'n', 'a', 'l', 's', 0};
        UCHAR* g_originals = FindDllExport(ProcBase, s_originals, &uError);
#ifdef _DEBUG
        if (!g_originals) {
            SET_LAST_ERROR(uError);
            __debugbreak();
        }
#endif
        if (g_originals)
            chrome64Target = findFirefoxTarget((unsigned char*)longlongs, g_originals);
        else
            chrome64Target = findChromeTarget((unsigned char *)longlongs);
    }
    // firefox sometimes
    /*else if (func[0] == 0x49 && //mov r11,<target>
        func[1] == 0xBB &&
        func[10] == 0x41 && //jmp r11
        func[11] == 0xFF &&
        func[12] == 0xE3) 
    {
        longlongs = *(ULONGLONG **)&func[2];
        chrome64Target = findChromeTarget((unsigned char *)longlongs);
    }*/
    
    /*sboxie 64bit jtable hook signature */
        /* // use this to hook jtable location (useful for debugging)
        //else if(func[0] == 0x51 && func[1] == 0x48 && func[2] == 0xb8 ) {
        else if(func[0] == 0x90 && func[1] == 0x48 && func[2] == 0xb8 ) {
            long long addr;
            addr = (ULONG_PTR) *(ULONGLONG **)&func[3] ;
            SourceFunc = (void *) addr;
        }
        */

    if (longlongs) {
        if (!chrome64Target)
            return (void*)-1; // failure
        return chrome64Target;
    }
#else
    UCHAR *func = (UCHAR *)SourceFunc;
    ULONGLONG* chromeTarge = NULL;

    if (func[0] == 0xB8 &&                  // mov eax,?
        func[5] == 0xBA &&                  // mov edx,?
        *(USHORT *)&func[10] == 0xE2FF)     // jmp edx
    {
        ULONG i = 0;
        ULONG *longs = *(ULONG **)&func[6];

        for (i = 0; i < 20; i++, longs++)
        {
            if (longs[0] == 0x5208EC83 && longs[1] == 0x0C24548B &&
                longs[2] == 0x08245489 && longs[3] == 0x0C2444C7 &&
                longs[5] == 0x042444C7)
            {
                chromeTarge = (void *)longs[4];
                break;
            }
        }

        if (!chromeTarge)
            return (void*)-1; // failure
        return chromeTarge;
    }
#endif ! _WIN64
    return NULL;
}
