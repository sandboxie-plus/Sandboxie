/*
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
// Win32 Services
//---------------------------------------------------------------------------


#define NOGDI
#include "dll.h"
#include "debug.h"

#include "common\pattern.h"
#include "common\arm64_asm.h"
#include "core/drv/api_defs.h"
#include "core/low/lowdata.h"

#ifndef _WIN64
#include "common\dllimport.h"

NTSTATUS SbieApi_ProtectVirtualMemory(HANDLE hProcess, DWORD64 lpAddress, SIZE_T dwSize, ULONG flNewProtect, ULONG* lpflOldProtect);
NTSTATUS SbieApi_ReadVirtualMemory(HANDLE hProcess, DWORD64 lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, ULONG64* lpNumberOfBytesRead);
NTSTATUS SbieApi_WriteVirtualMemory(HANDLE hProcess, DWORD64 lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, ULONG64* lpNumberOfBytesWritten);
NTSTATUS SbieApi_FlushInstructionCache(HANDLE hProcess, DWORD64 lpBaseAddress, SIZE_T nSize);
NTSTATUS SbieApi_QueryVirtualMemory(HANDLE hProcess, DWORD64 BaseAddress, MEMORY_INFORMATION_CLASS MemoryInformationClass, PVOID MemoryInformation, SIZE_T MemoryInformationLength, PSIZE_T ReturnLength);

BOOLEAN Win32_HookWin32WoW64();
#endif

#ifdef _M_ARM64EC
ULONG Hook_GetSysCallFunc(ULONG* aCode, void** pHandleStubHijack);
ULONG* SbieDll_FindFirstSysCallFunc(ULONG* aCode, void** pHandleStubHijack);
void* SbieDll_GetEcExitThunk(void* HandleStubHijack);
#endif

ULONG SbieDll_GetSysCallOffset(const ULONG *SyscallPtr, ULONG syscall_index);

extern SBIELOW_DATA* SbieApi_data;
#define SBIELOW_CALL(x) ((P_##x)&data->x##_code)


//---------------------------------------------------------------------------
// Win32_HookWin32SysCalls
//---------------------------------------------------------------------------


_FX BOOLEAN Win32_HookWin32SysCalls(HMODULE win32u_base)
{
    UCHAR *SystemServiceAsm;
    ULONG *SyscallPtr;
    ULONG SyscallNum;
    void *RegionBase;
    SIZE_T RegionSize;
    ULONG OldProtect;
    
    SBIELOW_DATA* data = SbieApi_data;
    SystemServiceAsm = (UCHAR*)data->pSystemService;

	UCHAR* syscall_data = (UCHAR *)HeapAlloc(GetProcessHeap(), 0, 16384); // enough room for over 2000 syscalls
	if (!syscall_data)
		return FALSE;

    if (!NT_SUCCESS(SbieApi_Call(API_QUERY_SYSCALLS, 2, (ULONG_PTR)syscall_data, 1))) {
        HeapFree(GetProcessHeap(), 0, syscall_data);
        return FALSE;
    }

    //
    // our syscall data area describes the ZwXxx functions in ntdll,
    // overwrite each export to jump to our assembly SystemService
    // see also core/drv/syscall.c and core/svc/DriverInjectAssist.cpp
    //

    SyscallPtr = (ULONG *)(syscall_data
                         + sizeof(ULONG));         // size of buffer

#ifdef _M_ARM64EC

	ULONG* aCode = SbieDll_FindFirstSysCallFunc((ULONG*)win32u_base, NULL);
	if (aCode == NULL) {
        HeapFree(GetProcessHeap(), 0, syscall_data);
        return FALSE;
	}

	for (ULONG pos = 0; pos < 128; aCode++, pos += 4) {

		SyscallNum = Hook_GetSysCallFunc(aCode, NULL);
		if (SyscallNum == -1)
			continue;

		if (SbieDll_GetSysCallOffset(SyscallPtr, SyscallNum)) {

            RegionBase = aCode;

            RegionSize = 16;

            SBIELOW_CALL(NtProtectVirtualMemory)(
                NtCurrentProcess(), &RegionBase, &RegionSize,
                PAGE_EXECUTE_READWRITE, &OldProtect);

            MOV mov;
            mov.OP   = 0xD2800011;  // mov x17, #0xFFFF
            mov.imm16 = (USHORT)SyscallNum; 
            *aCode++ = mov.OP;
	        *aCode++ = 0x18000048;	// ldr w8, 8
	        *aCode++ = 0xD61F0100;	// br x8
            *(ULONG*)aCode = (ULONG)(ULONG_PTR)SystemServiceAsm;

            SBIELOW_CALL(NtFlushInstructionCache)(
                NtCurrentProcess(), RegionBase, (ULONG)RegionSize);

            SBIELOW_CALL(NtProtectVirtualMemory)(
                NtCurrentProcess(), &RegionBase, &RegionSize,
                OldProtect, &OldProtect);

		}

		//if (aCode[13] != 0 || aCode[14] != 0 || aCode[15] != 0 || aCode[16] != 0)
		//    break;
		//aCode += (13 + 3 + 3 + 1);
		aCode += 16;
		pos = 0; // reset
	}

#else

    while (SyscallPtr[0] || SyscallPtr[1]) {

        //
        // the ULONG at SyscallPtr[1] gives the offset of the ZwXxx export
        // from the base address of ntdll
        //

        UCHAR* ZwXxxPtr = (UCHAR *)((ULONG_PTR)SyscallPtr[1] + (UCHAR*)win32u_base);

        //
        // make the syscall address writable
        //
        RegionBase = ZwXxxPtr;

#ifdef _M_ARM64
        RegionSize = 16;
#elif _WIN64
        RegionSize = 14;
#else ! _WIN64
        RegionSize = 10;
#endif _WIN64

        //
        // SyscallPtr[0] specifies the syscall number in the low 24 bits.
        // on 32-bit Windows, also specifies the parameter count (in bytes)
        // in the high 8 bits.  this is used by SystemServiceAsm
        //
        // overwrite the ZwXxx export to call our SystemServiceAsm,
        // and then restore the original page protection
        //

        SBIELOW_CALL(NtProtectVirtualMemory)(
            NtCurrentProcess(), &RegionBase, &RegionSize,
            PAGE_EXECUTE_READWRITE, &OldProtect);

        SyscallNum = SyscallPtr[0];

#ifdef _M_ARM64

        ULONG* aCode = (ULONG*)ZwXxxPtr;

        MOV mov;
        mov.OP   = 0xD2800011;  // mov x17, #0xFFFF
        mov.imm16 = (USHORT)SyscallNum; 
        *aCode++ = mov.OP;
	    *aCode++ = 0x18000048;	// ldr w8, 8
	    *aCode++ = 0xD61F0100;	// br x8
        *(ULONG*)aCode = (ULONG)(ULONG_PTR)SystemServiceAsm;


        SBIELOW_CALL(NtFlushInstructionCache)(
            NtCurrentProcess(), RegionBase, (ULONG)RegionSize);

#elif _WIN64

        ZwXxxPtr[0] = 0x49;                     // mov r10, SyscallNumber
        ZwXxxPtr[1] = 0xC7;
        ZwXxxPtr[2] = 0xC2;
        *(ULONG *)&ZwXxxPtr[3] = SyscallNum;
        if (!data->flags.long_diff) {

            if (data->flags.is_win10) {
                ZwXxxPtr[7] = 0x48;             // jmp SystemServiceAsm
                ZwXxxPtr[8] = 0xE9;
                *(ULONG *)&ZwXxxPtr[9] = (ULONG)(ULONG_PTR)(SystemServiceAsm - (ZwXxxPtr + 13));
            }
            else {
                ZwXxxPtr[7] = 0xe9;             // jmp SystemServiceAsm
                *(ULONG *)&ZwXxxPtr[8] = (ULONG)(ULONG_PTR)(SystemServiceAsm - (ZwXxxPtr + 12));
            }
        }
        else {

            ZwXxxPtr[7] = 0xB8;                 // mov eax, SystemServiceAsm
            *(ULONG *)&ZwXxxPtr[8] = (ULONG)(ULONG_PTR)SystemServiceAsm;
            *(USHORT *)&ZwXxxPtr[12] = 0xE0FF;  // jmp rax
        }
        
#else ! _WIN64

        ZwXxxPtr[0] = 0xB8;                 // mov eax, SyscallNumber, with param count in the highest byte
        *(ULONG *)&ZwXxxPtr[1] = SyscallNum;
        ZwXxxPtr[5] = 0xE9;                 // jmp SystemServiceAsm
        *(ULONG *)&ZwXxxPtr[6] =
            (ULONG)(ULONG_PTR)(SystemServiceAsm - (ZwXxxPtr + 10));

#endif _WIN64

        SBIELOW_CALL(NtProtectVirtualMemory)(
            NtCurrentProcess(), &RegionBase, &RegionSize,
            OldProtect, &OldProtect);

        SyscallPtr += 2;
    }

#endif

    HeapFree(GetProcessHeap(), 0, syscall_data);
    return TRUE;
}


//---------------------------------------------------------------------------
// Win32_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Win32_Init(HMODULE hmodule)
{
	// In Windows 10 all Win32k.sys calls are located in win32u.dll
    if (Dll_OsBuild < 10041 || (Dll_ProcessFlags & SBIE_FLAG_WIN32K_HOOKABLE) == 0 || !SbieApi_QueryConfBool(NULL, L"EnableWin32kHooks", TRUE))
        return TRUE; // just return on older builds, or not enabled

    if (Dll_CompartmentMode || SbieApi_data->flags.bNoSysHooks)
        return TRUE; // no syscall hooking in comaprtment mode

    //
    // chrome needs for a working GPU acceleration the GdiDdDDI* win32k syscalls to have the right user token
    // this however with some other software causes issues, presumably as then other syscalls would need to have the same token
    //

    BOOLEAN useByDefualt = (Dll_ImageType == DLL_IMAGE_GOOGLE_CHROME);
    if (SbieDll_GetSettingsForName_bool(NULL, Dll_ImageName, L"UseWin32kHooks", useByDefualt)) {

        // disable Electron Workaround when we are ready to hook the required win32k syscalls
        extern BOOL Dll_ElectronWorkaround;
        Dll_ElectronWorkaround = FALSE; 

#ifndef _WIN64
        if (Dll_IsWow64) 
            Win32_HookWin32WoW64(); // WoW64 hooks
        else 
#endif
            Win32_HookWin32SysCalls(hmodule); // Native x86/x64 hooks
    }

	return TRUE;
}


#ifndef _WIN64

//---------------------------------------------------------------------------
// Win32_GetSysCallNumberWoW64
//---------------------------------------------------------------------------


ULONG Win32_GetSysCallNumberWoW64(DWORD64 pos, UCHAR* dll_data)
{
    // 4C 8B D1 - r10,rcx
    if (!(dll_data[pos + 0] == 0x4c && dll_data[pos + 1] == 0x8b && dll_data[pos + 2] == 0xd1))
        return 0;

    // B8 XX XX XX XX - mov eax,0xXXXX
    if (!(dll_data[pos + 3] == 0xb8))
        return 0;
    ULONG syscall_index = *(ULONG*)&dll_data[pos + 4];

    // F6 04 25 08 03 FE 7F 01 - test BYTE PTR ds:0x7ffe0308,0x1
    //

    // 75 03 - jne 0x15 (label_1)
    if (!(dll_data[pos + 16] == 0x75 && dll_data[pos + 17] == 0x03))
        return 0;

    // 0F 05 - syscall
    if (!(dll_data[pos + 18] == 0x0f && dll_data[pos + 19] == 0x05))
        return 0;
    // C3 - ret
    if (!(dll_data[pos + 20] == 0xc3))
        return 0;

    // label_1:
    // CD 2E - int 0x2e
    if (!(dll_data[pos + 21] == 0xcd && dll_data[pos + 22] == 0x2e))
        return 0;
    // C3 - ret
    if (!(dll_data[pos + 23] == 0xc3))
        return 0;

    //

    return syscall_index;
}


//---------------------------------------------------------------------------
// Win32_GetSysCallNumberArm64
//---------------------------------------------------------------------------


_FX ULONG Win32_GetSysCallNumberArm64(ULONG* aCode)
{
    //  0: e10000d4    svc  #0x1000
    SVC svc;
    svc.OP = aCode[0];
    if (!IS_SVC(svc) || svc.imm16 < 0x1000) // win32 syscalls are >= 0x1000
        return -1;

    //  1: c0035fd6    ret  
    if (aCode[1] != 0xd65f03c0)
        return -1;

    return svc.imm16;
}


//---------------------------------------------------------------------------
// Win32_HookWin32WoW64
//---------------------------------------------------------------------------


_FX BOOLEAN Win32_HookWin32WoW64()
{
    BOOLEAN ok = FALSE;
    UCHAR* dll_data = NULL;
    UCHAR* syscall_data = NULL;

    //
    // first set up the 64 bit capable NtQueryVirtualMemory64 for FindDllBase64
    //

    extern void* NtQueryVirtualMemory64;
    if (!NtQueryVirtualMemory64)
        NtQueryVirtualMemory64 = SbieApi_QueryVirtualMemory;

    //
    // get the 64-bit address of the wow64win.dll
    //

    DWORD64 BaseAddress = FindDllBase64(NtCurrentProcess(), L"wow64win.dll");

    SIZE_T SizeOfImage = 0x00100000; // 1 MB should be more than enough
    if (!BaseAddress) {
        SbieApi_Log(2303, L"win32k, wow64win.dll base not found");
        return FALSE;
    }

    dll_data = (UCHAR *)HeapAlloc(GetProcessHeap(), 0, (SIZE_T)SizeOfImage);
    if (!dll_data) {
        SbieApi_Log(2303, L"win32k, alloc failed (1)");
        goto finish;
    }

    ULONG64 SizeRead;
    if (!NT_SUCCESS(SbieApi_ReadVirtualMemory(NtCurrentProcess(), (PVOID64)BaseAddress, dll_data, SizeOfImage, &SizeRead))) {
        SbieApi_Log(2303, L"win32k, wow64win.dll read failed");
        goto finish;
    }

    DWORD64 SystemServiceAsm;
    ULONG* SyscallPtr;
    ULONG SyscallNum;
    DWORD64 RegionBase;
    SIZE_T RegionSize;
    ULONG OldProtect;

    SBIELOW_DATA* data = SbieApi_data;
    SystemServiceAsm = data->pSystemService;

    syscall_data = (UCHAR*)HeapAlloc(GetProcessHeap(), 0, 16384); // enough room for over 2000 syscalls
    if (!syscall_data) {
        SbieApi_Log(2303, L"win32k, alloc failed (2)");
        goto finish;
    }

    if (!NT_SUCCESS(SbieApi_Call(API_QUERY_SYSCALLS, 2, (ULONG_PTR)syscall_data, 1))) {
        SbieApi_Log(2303, L"win32k, syscall query failed");
        goto finish;
    }

    SyscallPtr = (ULONG*)(syscall_data + sizeof(ULONG)); // size of buffer

    if (Dll_IsXtAjit) { // x86 on arm64

        UCHAR* ZwXxxPtr;

        ULONG* aCode = (ULONG*)dll_data;

		for (ULONG64 pos = 0; pos < SizeRead; aCode++, pos += 4) {
            if (Win32_GetSysCallNumberArm64(aCode) != -1)
                break;
		}

	    if ((DWORD64)aCode >= (DWORD64)dll_data + SizeRead) {
            SbieApi_Log(2303, L"win32k, get first syscall wrapper failed");
            goto finish;
	    }

	    for (ULONG pos = 0; pos < 128; aCode++, pos += 4) {

		    ULONG SyscallNum = Win32_GetSysCallNumberArm64(aCode);
		    if (SyscallNum == -1)
			    continue;

		    if (SbieDll_GetSysCallOffset(SyscallPtr, SyscallNum)) {

                DWORD64 offset = (DWORD64)aCode - (DWORD64)dll_data;

                RegionBase = BaseAddress + offset;
                RegionSize = 16;

                //
                // prepare call to call our SystemServiceAsm
                //

                ZwXxxPtr = aCode;

                MOV mov;
                mov.OP   = 0xD2800011;  // mov x17, #0xFFFF
                mov.imm16 = (USHORT)SyscallNum; 
                *aCode++ = mov.OP;
	            *aCode++ = 0x18000048;	// ldr w8, 8
	            *aCode++ = 0xD61F0100;	// br x8
                *(ULONG*)aCode = (ULONG)(ULONG_PTR)SystemServiceAsm;

                //
                // overwrite the ZwXxx export to call our SystemServiceAsm,
                // and then restore the original page protection
                //

                if (!NT_SUCCESS(SbieApi_ProtectVirtualMemory(NtCurrentProcess(), RegionBase, RegionSize, PAGE_EXECUTE_READWRITE, &OldProtect))) {
                    SbieApi_Log(2303, L"win32k %d (1)", SyscallNum);
                    goto finish;
                }

                if (!NT_SUCCESS(SbieApi_WriteVirtualMemory(NtCurrentProcess(), RegionBase, ZwXxxPtr, RegionSize, &SizeRead))) {
                    SbieApi_Log(2303, L"win32k %d (2)", SyscallNum);
                    goto finish;
                }
                
                if (!NT_SUCCESS(SbieApi_FlushInstructionCache(NtCurrentProcess(), RegionBase, RegionSize))) {
                    SbieApi_Log(2303, L"win32k %d (3)", SyscallNum);
                    goto finish;
                }

                if (!NT_SUCCESS(SbieApi_ProtectVirtualMemory(NtCurrentProcess(), RegionBase, RegionSize, OldProtect, &OldProtect))) {
                    SbieApi_Log(2303, L"win32k %d (4)", SyscallNum);
                    goto finish;
                }

		    }

		    //if (aCode[2] != 0 || aCode[3] != 0)
		    //    break;
		    aCode += 4;
		    pos = 0; // reset
	    }

    }
    else { // x86 on x64

        UCHAR ZwXxxPtr[16];

        DWORD64 FuncTable = 0;

        for (DWORD64 pos = 0; pos < SizeRead - 0x20; pos++) {
            if (Win32_GetSysCallNumberWoW64(pos, dll_data) != 0) {
                FuncTable = pos;
                break;
            }
        }

        if (FuncTable == 0) {
            SbieApi_Log(2303, L"win32k, wow64win.dll sys call table not found");
            goto finish;
        }

        for (DWORD64 pos = FuncTable; pos < SizeRead - 0x20; )
        {
            SyscallNum = Win32_GetSysCallNumberWoW64(pos, dll_data);
            if (SyscallNum)
            {
                if (SbieDll_GetSysCallOffset(SyscallPtr, SyscallNum))
                {
                    RegionBase = BaseAddress + pos;
                    RegionSize = 14;

                    //
                    // prepare call to call our SystemServiceAsm
                    //

                    ZwXxxPtr[0] = 0x49;                     // mov r10, SyscallNumber
                    ZwXxxPtr[1] = 0xC7;
                    ZwXxxPtr[2] = 0xC2;
                    *(ULONG*)&ZwXxxPtr[3] = SyscallNum;
                    if (!data->flags.long_diff) {

                        if (data->flags.is_win10) {
                            ZwXxxPtr[7] = 0x48;             // jmp SystemServiceAsm
                            ZwXxxPtr[8] = 0xE9;
                            *(ULONG*)&ZwXxxPtr[9] = (ULONG)(ULONG_PTR)(SystemServiceAsm - (RegionBase + 13));
                        }
                        else {
                            ZwXxxPtr[7] = 0xe9;             // jmp SystemServiceAsm
                            *(ULONG*)&ZwXxxPtr[8] = (ULONG)(ULONG_PTR)(SystemServiceAsm - (RegionBase + 12));
                        }
                    }
                    else {

                        ZwXxxPtr[7] = 0xB8;                 // mov eax, SystemServiceAsm
                        *(ULONG*)&ZwXxxPtr[8] = (ULONG)(ULONG_PTR)SystemServiceAsm;
                        *(USHORT*)&ZwXxxPtr[12] = 0xE0FF;  // jmp rax
                    }

                    //
                    // overwrite the ZwXxx export to call our SystemServiceAsm,
                    // and then restore the original page protection
                    //

                    if (!NT_SUCCESS(SbieApi_ProtectVirtualMemory(NtCurrentProcess(), RegionBase, RegionSize, PAGE_EXECUTE_READWRITE, &OldProtect))) {
                        SbieApi_Log(2303, L"win32k %d (1)", SyscallNum);
                        goto finish;
                    }

                    if (!NT_SUCCESS(SbieApi_WriteVirtualMemory(NtCurrentProcess(), RegionBase, ZwXxxPtr, RegionSize, &SizeRead))) {
                        SbieApi_Log(2303, L"win32k %d (2)", SyscallNum);
                        goto finish;
                    }

                    if (!NT_SUCCESS(SbieApi_ProtectVirtualMemory(NtCurrentProcess(), RegionBase, RegionSize, OldProtect, &OldProtect))) {
                        SbieApi_Log(2303, L"win32k %d (3)", SyscallNum);
                        goto finish;
                    }
                }

                pos += 0x20;
            }
            else if (*((ULONG*)&dll_data[pos]) == 0xCCCCCCCC) { // int 3; int 3; int 3; int 3;
                ok = TRUE;
                break; // end of sys call function table
            }
            else
                pos++;
        }

    }

finish:
    
    if(syscall_data)
        HeapFree(GetProcessHeap(), 0, syscall_data);

    if(dll_data)
        HeapFree(GetProcessHeap(), 0, dll_data);

    return ok;
}


//
// Win32_HookWin32WoW64 needs to be able to read and write the 64-bit portion of the address space
// therefore we issue direct syscalls using 64 bit arguments to our driver's syscall interface
// The driver accepts function names and optionally returns the corresponding syscall index for later direct use
// This replaces the use of heaven's gate (wow64ext), as it is unavailable when running in emulation on arm64
//


//---------------------------------------------------------------------------
// SbieApi_ProtectVirtualMemory
//---------------------------------------------------------------------------


NTSTATUS SbieApi_ProtectVirtualMemory(HANDLE hProcess, DWORD64 lpAddress, SIZE_T dwSize, ULONG flNewProtect, ULONG* lpflOldProtect)
{
    ULONG64 BaseAddress = lpAddress;
    ULONG64 NumberOfBytesToProtect = dwSize;

    static SHORT syscall_index = 0xFFF;

	ULONG64 stack[17];
	stack[0] = hProcess;
	stack[1] = &BaseAddress;
	stack[2] = &NumberOfBytesToProtect;
	stack[3] = flNewProtect;
	stack[4] = lpflOldProtect;

    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    memset(parms, 0, sizeof(parms));
    parms[0] = API_INVOKE_SYSCALL;
	parms[1] = (ULONG64)(ULONG_PTR)syscall_index;
    parms[2] = (ULONG64)(ULONG_PTR)stack; // pointer to system service arguments on stack
    parms[3] = (ULONG64)(ULONG_PTR)"ProtectVirtualMemory";
    parms[4] = (ULONG64)(ULONG_PTR)&syscall_index;

    NTSTATUS status = SbieApi_Ioctl(parms);
    return status;
}


//---------------------------------------------------------------------------
// SbieApi_ReadVirtualMemory
//---------------------------------------------------------------------------


NTSTATUS SbieApi_ReadVirtualMemory(HANDLE hProcess, DWORD64 lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, ULONG64 *lpNumberOfBytesRead)
{
    static SHORT syscall_index = 0xFFF;

	ULONG64 stack[17];
	stack[0] = hProcess;
	stack[1] = lpBaseAddress;
	stack[2] = lpBuffer;
	stack[3] = nSize;
	stack[4] = lpNumberOfBytesRead;

    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    memset(parms, 0, sizeof(parms));
    parms[0] = API_INVOKE_SYSCALL;
	parms[1] = (ULONG64)(ULONG_PTR)syscall_index;
    parms[2] = (ULONG64)(ULONG_PTR)stack; // pointer to system service arguments on stack
    parms[3] = (ULONG64)(ULONG_PTR)"ReadVirtualMemory";
    parms[4] = (ULONG64)(ULONG_PTR)&syscall_index;

    NTSTATUS status = SbieApi_Ioctl(parms);
    if (status == STATUS_PARTIAL_COPY)
        status = STATUS_SUCCESS;
    return status;
}


//---------------------------------------------------------------------------
// SbieApi_WriteVirtualMemory
//---------------------------------------------------------------------------


NTSTATUS SbieApi_WriteVirtualMemory(HANDLE hProcess, DWORD64 lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, ULONG64 *lpNumberOfBytesWritten)
{
    static SHORT syscall_index = 0xFFF;

	ULONG64 stack[17];
	stack[0] = hProcess;
	stack[1] = lpBaseAddress;
	stack[2] = lpBuffer;
	stack[3] = nSize;
	stack[4] = lpNumberOfBytesWritten;

    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    memset(parms, 0, sizeof(parms));
    parms[0] = API_INVOKE_SYSCALL;
	parms[1] = (ULONG64)(ULONG_PTR)syscall_index;
    parms[2] = (ULONG64)(ULONG_PTR)stack; // pointer to system service arguments on stack
    parms[3] = (ULONG64)(ULONG_PTR)"WriteVirtualMemory";
    parms[4] = (ULONG64)(ULONG_PTR)&syscall_index;

    NTSTATUS status = SbieApi_Ioctl(parms);
    return status;
}


//---------------------------------------------------------------------------
// SbieApi_FlushInstructionCache
//---------------------------------------------------------------------------


NTSTATUS SbieApi_FlushInstructionCache(HANDLE hProcess, DWORD64 lpBaseAddress, SIZE_T nSize)
{
    static SHORT syscall_index = 0xFFF;

	ULONG64 stack[17];
	stack[0] = hProcess;
	stack[1] = lpBaseAddress;
	stack[2] = nSize;
	
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    memset(parms, 0, sizeof(parms));
    parms[0] = API_INVOKE_SYSCALL;
	parms[1] = (ULONG64)(ULONG_PTR)syscall_index;
    parms[2] = (ULONG64)(ULONG_PTR)stack; // pointer to system service arguments on stack
    parms[3] = (ULONG64)(ULONG_PTR)"FlushInstructionCache";
    parms[4] = (ULONG64)(ULONG_PTR)&syscall_index;

    NTSTATUS status = SbieApi_Ioctl(parms);
    return status;
}


//---------------------------------------------------------------------------
// SbieApi_QueryVirtualMemory
//---------------------------------------------------------------------------


NTSTATUS SbieApi_QueryVirtualMemory(HANDLE hProcess, DWORD64 BaseAddress, MEMORY_INFORMATION_CLASS MemoryInformationClass, PVOID MemoryInformation, SIZE_T MemoryInformationLength, PSIZE_T ReturnLength)
{
    DWORD64 ReturnLength64 = 0;

    static SHORT syscall_index = 0xFFF;

	ULONG64 stack[17];
	stack[0] = hProcess;
	stack[1] = BaseAddress;
	stack[2] = MemoryInformationClass;
	stack[3] = MemoryInformation;
	stack[4] = MemoryInformationLength;
    stack[5] = &ReturnLength64;

    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    memset(parms, 0, sizeof(parms));
    parms[0] = API_INVOKE_SYSCALL;
	parms[1] = (ULONG64)(ULONG_PTR)syscall_index;
    parms[2] = (ULONG64)(ULONG_PTR)stack; // pointer to system service arguments on stack
    parms[3] = (ULONG64)(ULONG_PTR)"QueryVirtualMemory";
    parms[4] = (ULONG64)(ULONG_PTR)&syscall_index;

    NTSTATUS status = SbieApi_Ioctl(parms);

    if(ReturnLength) *ReturnLength = (SIZE_T)ReturnLength64;

    return status;
}

#endif
