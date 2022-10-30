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
// Low Level DLL data area
//---------------------------------------------------------------------------


#ifndef _MY_LOWDATA_H
#define _MY_LOWDATA_H


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


#ifdef _WIN64

typedef struct _SBIELOW_J_TABLE_ENTRY 
{
#ifdef _M_ARM64
//  unsigned char code[0x28]; //for jtable with hotpatch location for sboxdll hooking
    unsigned char code[0x20]; //for jtable
#else
//  unsigned char code[0x18]; //for jtable with hotpatch location for sboxdll hooking
    unsigned char code[0x10]; //for jtable
#endif
} SBIELOW_J_TABLE_ENTRY;

typedef struct _SBIELOW_J_TABLE
{
   SBIELOW_J_TABLE_ENTRY entry[1024];
} SBIELOW_J_TABLE;

#endif


#define NATIVE_FUNCTION_NAMES   { "NtDelayExecution", "NtDeviceIoControlFile", "NtFlushInstructionCache", "NtProtectVirtualMemory" }
#define NATIVE_FUNCTION_COUNT   4
#define NATIVE_FUNCTION_SIZE    32


typedef union _SBIELOW_FLAGS {
	ULONG	init_flags;
	struct {
		ULONG
            is_wow64		: 1,
            is_arm64ec		: 1,
            is_xtajit       : 1,
            is_chpe32       : 1,
			reservd_1		: 4,

			long_diff		: 1,
			reservd_2		: 7,

			bHostInject		: 1,
			bNoSysHooks		: 1,
			bNoConsole		: 1,
			reservd_3		: 5,

			is_win10		: 1,
			reservd_4		: 7;
	};
} SBIELOW_FLAGS;

typedef struct _SBIELOW_DATA {
    ULONG64     ntdll_base;
    ULONG64     syscall_data;

    ULONG64     api_device_handle;
    ULONG       api_sbiedrv_ctlcode;
    ULONG       api_invoke_syscall;

    SBIELOW_FLAGS flags;

    __declspec(align(16))
        UCHAR   LdrInitializeThunk_tramp[48];

    __declspec(align(16))
        UCHAR   NtDelayExecution_code[NATIVE_FUNCTION_SIZE];
    __declspec(align(16))
        UCHAR   NtDeviceIoControlFile_code[NATIVE_FUNCTION_SIZE];     // offset 128
    __declspec(align(16))
        UCHAR   NtFlushInstructionCache_code[NATIVE_FUNCTION_SIZE];   // offset 160
    __declspec(align(16))
        UCHAR   NtProtectVirtualMemory_code[NATIVE_FUNCTION_SIZE];

    ULONG64     RealNtDeviceIoControlFile;          // offset 224
    ULONG64     NtDeviceIoControlFile; // for ARM64 // offset 232
    ULONG64     NativeNtRaiseHardError;             // offset 240

    ULONG64     pSystemService;

    ULONG64     DebugData[16];

#ifdef _WIN64
    SBIELOW_J_TABLE * Sbie64bitJumpTable;

	ULONG64     ntdll_wow64_base;
#endif

#ifdef _M_ARM64
    __declspec(align(16))
        UCHAR   RtlImageOptionsEx_tramp[48];
#endif

} SBIELOW_DATA;


//
// SBIELOW_DATA symbol is in the "zzzz" section of lowlevel that 
// points to location in the code section ".text", as defined in 
// entry.asm label SbieLowData
//
// hard coded Data offset dependency removed 
#define SBIELOW_INJECTION_SECTION ".text"
#define SBIELOW_SYMBOL_SECTION     "zzzz"

//
// additional strings that are used to inject SbieDll are passed in
// the syscall data area.  the second ULONG in the syscall data area
// specifies the offset to this extra data structure
//

typedef struct _SBIELOW_EXTRA_DATA {

    ULONG LdrLoadDll_offset;
    ULONG LdrGetProcAddr_offset;
    ULONG NtRaiseHardError_offset;
    ULONG RtlFindActCtx_offset;
#ifdef _M_ARM64
    ULONG RtlImageOptionsEx_offset;
#endif

    ULONG KernelDll_offset;
    ULONG KernelDll_length;

    ULONG NativeSbieDll_offset;
    ULONG NativeSbieDll_length;
#ifdef _M_ARM64
    ULONG Arm64ecSbieDll_offset;
    ULONG Arm64ecSbieDll_length;
#endif
    ULONG Wow64SbieDll_offset;
    ULONG Wow64SbieDll_length;

    ULONG InjectData_offset;

} SBIELOW_EXTRA_DATA;


//
// Syscall data as provided by the driver
//

typedef struct _SYSCALL_INFO {

    ULONG SyscallNum;
    ULONG SyscallOffset;

} SYSCALL_INFO;

typedef struct _SYSCALL_INFO_EX {

    ULONG SyscallNum;
    ULONG SyscallOffset;

    char SyscallName[64];

} SYSCALL_INFO_EX;


typedef struct _SYSCALL_DATA { // ntdll.dll

    ULONG syscall_data_len;
    ULONG extra_data_offset;

    UCHAR NtdllSavedCode[NATIVE_FUNCTION_SIZE * NATIVE_FUNCTION_COUNT];

    SYSCALL_INFO syscall_data[];

} SYSCALL_DATA;


typedef struct _SYSCALL_DATA32 { // win32u.dll

    ULONG syscall_data_len;

    SYSCALL_INFO syscall_data[];

} SYSCALL_DATA32;


//
// temporary data used by the Detour Code any changed to 
// this structure must be synchronized with all 3 versions of the
// Detour Code as well as with the binary copies of the x86 and x64 code
// 
// entry_asm.asm, entry_arm.asm and lowlevel_code.c
//

typedef struct _INJECT_DATA {

    ULONG64 sbielow_data;           // 0

    union {
        ULONG64 LdrLoadDll;         // 8
        ULONG64 RtlFindActCtx_SavedArg1; // todo: split this
    };
    ULONG64 LdrGetProcAddr;         // 16
    ULONG64 NtRaiseHardError;       // 24
    ULONG64 RtlFindActCtx;          // 32

    ULONG RtlFindActCtx_Protect;    // 40
    UCHAR RtlFindActCtx_Bytes[20];  // 44

    USHORT  KernelDll_Length;       // 64
    USHORT  KernelDll_MaxLen;
    ULONG   KerneDll_Buf32;
    ULONG64 KerneDll_Buf64;

    USHORT  SbieDll_Length;         // 80
    USHORT  SbieDll_MaxLen;
    ULONG   SbieDll_Buf32;
    ULONG64 SbieDll_Buf64;

    ULONG64 ModuleHandle;           // 96
    ULONG64 SbieDllOrdinal1;        // 104

    ULONG64 MyGetProcAddr;          // 112

#ifdef _WIN64
    UCHAR   DetourCode_x86[128];    // 120
#endif _WIN64

} INJECT_DATA;

//---------------------------------------------------------------------------


#endif /* _MY_DRIVERASSIST_H */
