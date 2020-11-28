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
//  unsigned char code[0x18]; //for jtable with hotpatch location for sboxdll hooking
    unsigned char code[0x10]; //for jtable
} SBIELOW_J_TABLE_ENTRY;

typedef struct _SBIELOW_J_TABLE
{
   SBIELOW_J_TABLE_ENTRY entry[1024];
} SBIELOW_J_TABLE;

#endif

typedef struct _SBIELOW_DATA {
    ULONG64     ntdll_base;
    ULONG64     syscall_data;

    ULONG64     api_device_handle;
    ULONG       api_sbiedrv_ctlcode;
    ULONG       api_invoke_syscall;

	BOOLEAN     is_wow64;
	BOOLEAN     long_diff;
	BOOLEAN     bHostInject;
	BOOLEAN     is_win10;

	/*union {
		ULONG	init_flags;
		struct {
			ULONG
				is_wow64		: 1,
				reservd_1		: 7,

				long_diff		: 1,
				reservd_2		: 7,

				bHostInject		: 1,
				bNoSysHooks		: 1,
				bNoConsole		: 1,
				reservd_3		: 5,

				is_win10		: 1,
				reservd_4		: 7;
		};
	};*/

    __declspec(align(16))
        UCHAR   LdrInitializeThunk_tramp[48];
    __declspec(align(16))
        UCHAR   NtDelayExecution_code[32];
    __declspec(align(16))
        UCHAR   NtDeviceIoControlFile_code[32];
    __declspec(align(16))
        UCHAR   NtFlushInstructionCache_code[32];
    __declspec(align(16))
        UCHAR   NtProtectVirtualMemory_code[32];
     ULONG64 RealNtDeviceIoControlFile;
#ifdef _WIN64
    SBIELOW_J_TABLE * Sbie64bitJumpTable;
	ULONG64     ntdll_wow64_base;
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
    ULONG KernelDll_offset;
    ULONG KernelDll_length;
    ULONG NativeSbieDll_offset;
    ULONG NativeSbieDll_length;
    ULONG Wow64SbieDll_offset;
    ULONG Wow64SbieDll_length;

} SBIELOW_EXTRA_DATA;


//---------------------------------------------------------------------------


#endif /* _MY_DRIVERASSIST_H */
