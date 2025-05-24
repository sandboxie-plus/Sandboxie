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
 // Sandboxie Low Level code injection mechanism moved here from SbieSvc
 //---------------------------------------------------------------------------

#include "dll.h"

#include <windows.h>
#include <stdio.h>

#include "common/my_version.h"
#include "core/low/lowdata.h"
#include "common/win32_ntddk.h"
#include "core/drv/api_defs.h"
#include "common/dllimport.h"
#include "common/arm64_asm.h"



//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _MY_TARGETS {
	unsigned long long entry;
	unsigned long long data;
	unsigned long long detour;
} MY_TARGETS;

#ifdef _M_ARM64

WINBASEAPI BOOL WINAPI IsWow64Process2(
	HANDLE hProcess,
	USHORT* pProcessMachine,
	USHORT* pNativeMachine
	);

WINBASEAPI BOOL WINAPI GetProcessInformation(
    HANDLE hProcess,
    PROCESS_INFORMATION_CLASS ProcessInformationClass,
    LPVOID ProcessInformation,
    DWORD ProcessInformationSize
    );

typedef PVOID (*P_VirtualAlloc2)(
    HANDLE Process,
    PVOID BaseAddress,
    SIZE_T Size,
    ULONG AllocationType,
    ULONG PageProtection,
    MEM_EXTENDED_PARAMETER* ExtendedParameters,
    ULONG ParameterCount
	);
	
#endif


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

SBIEDLL_EXPORT  HANDLE SbieDll_InjectLow_SendHandle(HANDLE hProcess);

void *SbieDll_InjectLow_CopyCode(
	HANDLE hProcess, SIZE_T total_size, SIZE_T lowLevel_size, const void* lowLevel_ptr
#ifdef _M_ARM64
	, BOOLEAN use_arm64ec
#endif
);
BOOLEAN SbieDll_InjectLow_BuildTramp(
	BOOLEAN long_diff, UCHAR *code, ULONG_PTR addr
#ifdef _M_ARM64
	, BOOLEAN use_arm64ec
#endif
);
void *SbieDll_InjectLow_CopySyscalls(HANDLE hProcess, BOOLEAN is_wow64
#ifdef _M_ARM64
	, BOOLEAN use_arm64ec
#endif
);
void* InjectLow_AllocMemory(HANDLE hProcess, SIZE_T size, BOOLEAN executable
#ifdef _M_ARM64
	, BOOLEAN use_arm64ec
#endif
);
BOOLEAN SbieDll_InjectLow_CopyData(
	HANDLE hProcess, void *remote_addr, void *local_data);
#ifdef _WIN64
BOOLEAN SbieDll_Has32BitJumpHorizon(void * target, void * detour);
void * SbieDll_InjectLow_getPage(HANDLE hProcess, void *remote_addr);
#endif
BOOLEAN SbieDll_InjectLow_WriteJump(
	HANDLE hProcess, void *remote_addr, BOOLEAN long_diff
#ifdef _M_ARM64
	, BOOLEAN use_arm64ec
#endif
);

#if defined(_M_ARM64) || defined(_M_ARM64EC)
ULONG Hook_GetSysCallFunc(ULONG* aCode, void** pHandleStubHijack);
#endif

//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


void *m_sbielow_ptr = NULL;
ULONG m_sbielow_len = 0;
//adding two offsets variables to replace the "head" and "tail" dependency
ULONG m_sbielow_start_offset = 0;
ULONG m_sbielow_data_offset = 0;

#ifdef _WIN64
void *m_sbielow32_ptr = NULL;
ULONG m_sbielow32_len = 0;
ULONG m_sbielow32_detour_offset = 0;
#endif

ULONG *m_syscall_data = NULL;

ULONG_PTR m_LdrInitializeThunk = 0;

#ifdef _M_ARM64

ULONG *m_syscall_ec_data = NULL;

ULONG_PTR m_LdrInitializeThunkEC = 0;

P_VirtualAlloc2 __sys_VirtualAlloc2 = NULL;

#endif


//---------------------------------------------------------------------------
// SbieDll_InjectLow_LoadLow
//---------------------------------------------------------------------------


_FX ULONG SbieDll_InjectLow_LoadLow(BOOLEAN arch_64bit, void **sbielow_ptr, ULONG *sbielow_len, ULONG *start_offset, ULONG* data_offset, ULONG* detour_offset)
{
    //
    // lock the SbieLow resource (embedded within the SbieSvc executable,
    // see lowlevel.rc) and find the offset to executable code, and length
    //

    IMAGE_DOS_HEADER *dos_hdr = 0;
    IMAGE_NT_HEADERS *nt_hdrs = 0;
    IMAGE_SECTION_HEADER *section = 0;
    IMAGE_DATA_DIRECTORY *data_dirs = 0;
    ULONG_PTR imageBase = 0;
    MY_TARGETS *targets = 0;

	ULONG errlvl = 0x11;

	HRSRC hrsrc = FindResource(Dll_Instance, arch_64bit ? L"LOWLEVEL64" : L"LOWLEVEL32", RT_RCDATA);
	if (! hrsrc)
        return errlvl;

    ULONG binsize = SizeofResource(Dll_Instance, hrsrc);
    if (! binsize)
		return errlvl;

    HGLOBAL hglob = LoadResource(Dll_Instance, hrsrc);
    if (! hglob)
		return errlvl;

    UCHAR *bindata = (UCHAR *)LockResource(hglob);
    if (! bindata)
		return errlvl;

	errlvl = 0x22;
    dos_hdr = (IMAGE_DOS_HEADER *)bindata;

    if (dos_hdr->e_magic == 'MZ' || dos_hdr->e_magic == 'ZM') {
        nt_hdrs = (IMAGE_NT_HEADERS *)((UCHAR *)dos_hdr + dos_hdr->e_lfanew);

		if (nt_hdrs->Signature != IMAGE_NT_SIGNATURE)   // 'PE\0\0'
			return errlvl;
		if (nt_hdrs->OptionalHeader.Magic != (arch_64bit ? IMAGE_NT_OPTIONAL_HDR64_MAGIC : IMAGE_NT_OPTIONAL_HDR32_MAGIC)) 
			return errlvl;

        if (!arch_64bit) {
            IMAGE_NT_HEADERS32 *nt_hdrs_32 = (IMAGE_NT_HEADERS32 *)nt_hdrs;
            IMAGE_OPTIONAL_HEADER32 *opt_hdr_32 = &nt_hdrs_32->OptionalHeader;
            data_dirs = &opt_hdr_32->DataDirectory[0];
            imageBase = opt_hdr_32->ImageBase;
        }
		else {
            IMAGE_NT_HEADERS64 *nt_hdrs_64 = (IMAGE_NT_HEADERS64 *)nt_hdrs;
            IMAGE_OPTIONAL_HEADER64 *opt_hdr_64 = &nt_hdrs_64->OptionalHeader;
            data_dirs = &opt_hdr_64->DataDirectory[0];
            imageBase = (ULONG_PTR)opt_hdr_64->ImageBase;
        }
    }

	ULONG zzzzz = 1;
#ifdef _M_ARM64
	if (arch_64bit) 
		zzzzz = 4; // ARM64 only
	else
#endif
	if (imageBase != 0) // x64 or x86
		return errlvl;

    section = IMAGE_FIRST_SECTION(nt_hdrs);
    if (nt_hdrs->FileHeader.NumberOfSections < 2) return errlvl;
    if (strncmp((char *)section[0].Name, SBIELOW_INJECTION_SECTION, strlen(SBIELOW_INJECTION_SECTION)) ||
        strncmp((char *)section[zzzzz].Name, SBIELOW_SYMBOL_SECTION, strlen(SBIELOW_SYMBOL_SECTION))) {
		return errlvl;
    }


    targets = (MY_TARGETS *)& bindata[section[zzzzz].PointerToRawData];
    if(start_offset) *start_offset = (ULONG)(targets->entry - imageBase - section[0].VirtualAddress);
    if(data_offset) *data_offset = (ULONG)(targets->data - imageBase - section[0].VirtualAddress);
	if(detour_offset) *detour_offset = (ULONG)(targets->detour - imageBase - section[0].VirtualAddress);

    *sbielow_ptr = bindata + section[0].PointerToRawData; //Old version: head;
    *sbielow_len = section[0].SizeOfRawData; //Old version: (ULONG)(ULONG_PTR)(tail - head);

	return 0;
}


//---------------------------------------------------------------------------
// InjectLow_InitHelper
//---------------------------------------------------------------------------


_FX ULONG SbieDll_InjectLow_InitHelper()
{
#ifdef _WIN64
	ULONG errlvl = SbieDll_InjectLow_LoadLow(TRUE, &m_sbielow_ptr, &m_sbielow_len, &m_sbielow_start_offset, &m_sbielow_data_offset, NULL);
	if(!errlvl)
		errlvl = SbieDll_InjectLow_LoadLow(FALSE, &m_sbielow32_ptr, &m_sbielow32_len, NULL, NULL, &m_sbielow32_detour_offset);
#else
	ULONG errlvl = SbieDll_InjectLow_LoadLow(FALSE, &m_sbielow_ptr, &m_sbielow_len, &m_sbielow_start_offset, &m_sbielow_data_offset, NULL);
#endif
	if (errlvl)
		return errlvl;

    //
    // record information about ntdll and the virtual memory system
    //

	errlvl = 0x33;

    m_LdrInitializeThunk = (ULONG_PTR) GetProcAddress(Dll_Ntdll, "LdrInitializeThunk");

    if (! m_LdrInitializeThunk)
		return errlvl;

#ifdef _M_ARM64

	//
	// for x64 on arm64 we need the EC version of LdrInitializeThunk as well as VirtualAlloc2, 
	// if those are missing we will only fail SbieDll_InjectLow for x64 processes on arm64
	//

	WCHAR path[MAX_PATH];
	if (! GetSystemDirectory(path, MAX_PATH))
        wcscpy(path, L"C:\\Windows\\System32");
	wcscat(path, L"\\ntdll.dll");

	m_LdrInitializeThunkEC = (ULONG_PTR)Dll_Ntdll + FindDllExportFromFile(path, "#LdrInitializeThunk");

	__sys_VirtualAlloc2 = (P_VirtualAlloc2)GetProcAddress(Dll_KernelBase, "VirtualAlloc2");

#elif _WIN64
    if (Dll_Windows >= 10) {
        unsigned char * code;
        code = (unsigned char *)m_LdrInitializeThunk;
        if (*(ULONG *)&code[0] == 0x24048b48 && code[0xa] == 0x48) {
            m_LdrInitializeThunk += 0xa;
        }
    }
#endif

    return 0;
}

#if defined(_M_ARM64) || defined(_M_ARM64EC)

//---------------------------------------------------------------------------
// SbieDll_FindFirstSysCallFunc
//---------------------------------------------------------------------------


_FX ULONG* SbieDll_FindFirstSysCallFunc(ULONG* aCode, void** pHandleStubHijack)
{
	//
	// Scan the ntdll.dll/win32u.dll from its base up to 16mb max 
	// in search of the first ec syscall wrapper function
	//

	__try {
		for (ULONG pos = 0; pos < 16 * 1024 * 1024; aCode++, pos += 4) {
			if (Hook_GetSysCallFunc(aCode, pHandleStubHijack) != -1)
				return aCode;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {}

	return NULL;
}

//---------------------------------------------------------------------------
// SbieDll_GetEcExitThunkPtr
//---------------------------------------------------------------------------


_FX void* SbieDll_GetEcExitThunkPtr(void* HandleStubHijack)
{
    UCHAR* ptr = (UCHAR*)HandleStubHijack;

	//
	// Analyze the HandleStubHijack function to find the address of
	// __os_arm64x_dispatch_call_no_redirect which points to
	// xtajit64.dll!#BinaryTranslatorEcExitThunkCallX64 in EC processes
	//

    for (ULONG i = 0; i < 0x40; i += 4, ptr += 4) {

        ADRP adrp;
        adrp.OP = *(ULONG*)ptr;

        if (IS_ADRP(adrp) && adrp.Rd == 16) { // adrp x16, 0x180379000

            LDR ldr;
            ldr.OP = *(ULONG*)(ptr + 4);

            if (IS_LDR(ldr) && ldr.Rn == 16 && ldr.Rt == 16) { // ldr x16, [x16, #0xae0]

                LONG delta = (adrp.immHi << 2 | adrp.immLo) << 12;
                delta += (ldr.imm12 << ldr.size);

                // Note: ADRP clears the lower 12 bits of the PC
                return (void*)(((ULONG_PTR)ptr & ~0xFFF) + delta);
            }
        }
    }

    return NULL;
}

#endif

//---------------------------------------------------------------------------
// SbieDll_GetSysCallOffset
//---------------------------------------------------------------------------


_FX ULONG SbieDll_GetSysCallOffset(const ULONG *SyscallPtr, ULONG syscall_index)
{
    ULONG SyscallNum;

    while (SyscallPtr[0] || SyscallPtr[1]) {
            
        SyscallNum = SyscallPtr[0];
            
        SyscallNum &= 0xFFFF; // clear the not needed param count
            
        if (SyscallNum == syscall_index)
            return SyscallPtr[1];

        SyscallPtr += 2;
    }

    return 0;
}


//---------------------------------------------------------------------------
// InjectLow_InitSyscalls
//---------------------------------------------------------------------------


_FX ULONG SbieDll_InjectLow_InitSyscalls(BOOLEAN drv_init)
{
	const WCHAR *_SbieDll = L"\\" SBIEDLL L".dll";
	WCHAR sbie_home[512];
	ULONG status;
	ULONG len;
	SBIELOW_EXTRA_DATA *extra;
	WCHAR *ptr;
	ULONG *syscall_data;
#ifdef _M_ARM64
	ULONG *syscall_ec_data;
#endif

	//
	// Get the SbieDll Location
	//

	/*if (1) {
		GetSystemDirectory(sbie_home, 512);
	}
	else */
	if (drv_init)
	{
		status = SbieApi_GetHomePath(NULL, 0, sbie_home, 512);
		if (status != 0)
			return status;
	}
	else
	{
		GetModuleFileName(Dll_Instance, sbie_home, 512);
		ptr = wcsrchr(sbie_home, L'\\');
		if (ptr)
			*ptr = L'\0';
	}

#define ULONG_DIFF(b,a) ((ULONG)((ULONG_PTR)(b) - (ULONG_PTR)(a)))

	//
	// get the list of syscall from the driver
	//

	if (!m_syscall_data) {
		syscall_data = (ULONG *)HeapAlloc(GetProcessHeap(), 0, 8192);
		if (!syscall_data)
			return STATUS_INSUFFICIENT_RESOURCES;
		*syscall_data = 0;
	}
	else
		syscall_data = m_syscall_data;

	if (drv_init)
	{
		//
		// Get a full sys call list from the driver
		//

		status = SbieApi_Call(API_QUERY_SYSCALLS, 1, (ULONG_PTR)syscall_data);
		if (status != 0)
			return status;

		len = *syscall_data;
		if ((!len) || (len & 3) || (len > 4096))
			return STATUS_INVALID_IMAGE_FORMAT;
	}
	else
	{
		//
		// Create a minimalistic driverless data
		//

		*syscall_data = sizeof(ULONG) + sizeof(ULONG) + (NATIVE_FUNCTION_SIZE * NATIVE_FUNCTION_COUNT); // + 248; // total_size 4, extra_data_offset 4, work area sizeof(INJECT_DATA)

		const char* NtdllExports[] = NATIVE_FUNCTION_NAMES;
		for (ULONG i = 0; i < NATIVE_FUNCTION_COUNT; ++i) {
			void* func_ptr = GetProcAddress(Dll_Ntdll, NtdllExports[i]);
			memcpy((void*)((ULONG_PTR)syscall_data + (sizeof(ULONG) + sizeof(ULONG)) + (NATIVE_FUNCTION_SIZE * i)), func_ptr, NATIVE_FUNCTION_SIZE);
		}

		len = *syscall_data;
	}

	/*
	struct SYSCALL_DATA {
		ULONG size_of_buffer;
		ULONG offset_to_extra_data;

		struct _CODE {
			UCHAR bytes[NATIVE_FUNCTION_SIZE]
		}			saved_code[NATIVE_FUNCTION_COUNT];

		struct SYSCALL_INFO {
			ULONG number;
			ULONG offset;
		}			sys_call_list[n + 1];

		// the above is provided by the driver, the below we have to construct

		ULONG64 EcExitThunkPtr;

		struct SBIELOW_EXTRA_DATA {
			// ...
		}			extra_data; // 52 bytes

		char LdrLoadDll_str[16]			= "LdrLoadDll";
		char LdrGetProcAddr_str[28]		= "LdrGetProcedureAddress";
		char NtRaiseHardError_str[20]	= "NtRaiseHardError";
		char RtlFindActCtx_str[44]		= "RtlFindActivationContextSectionString";

		wchar_t KernelDll_str[13]		= L"kernel32.dll";

		wchar_t NativeSbieDll_str[]		= L"...\\SbieDll.dll";
		wchar_t Arm64ecSbieDll_str[]	= L"...\\64\\SbieDll.dll";
		wchar_t Wow64SbieDll_str[]		= L"...\\32\\SbieDll.dll";

		struct INJECT_DATA {
			// ...
		}			InjectData;
	}
	*/

	//
	// the second ULONG in syscall_data points to extra data appended
	// by us here on top of what the driver returned
	//

	extra = (SBIELOW_EXTRA_DATA *)((ULONG_PTR)syscall_data + len);

	syscall_data[1] = len;

	//
	// write an ASCII string for LdrLoadDll (see core/low/inject.c)
	//

	ptr = (WCHAR *)((ULONG_PTR)extra + sizeof(SBIELOW_EXTRA_DATA));

	strcpy((char *)ptr, "LdrLoadDll");

	extra->LdrLoadDll_offset = ULONG_DIFF(ptr, extra);
	ptr += 16 / sizeof(WCHAR);

	//
	// write an ASCII string for LdrGetProcedureAddress
	//

	strcpy((char *)ptr, "LdrGetProcedureAddress");

	extra->LdrGetProcAddr_offset = ULONG_DIFF(ptr, extra);
	ptr += 28 / sizeof(WCHAR);

	//
	// write an ASCII string for NtProtectVirtualMemory
	//

	strcpy((char *)ptr, "NtProtectVirtualMemory");

	extra->NtProtectVirtualMemory_offset = ULONG_DIFF(ptr, extra);
	ptr += 28 / sizeof(WCHAR);

	//
	// write an ASCII string for NtRaiseHardError
	//

	strcpy((char *)ptr, "NtRaiseHardError");

	extra->NtRaiseHardError_offset = ULONG_DIFF(ptr, extra);
	ptr += 20 / sizeof(WCHAR);

	//
	// write an ASCII string for NtDeviceIoControlFile
	//

	strcpy((char *)ptr, "NtDeviceIoControlFile");

	extra->NtDeviceIoControlFile_offset = ULONG_DIFF(ptr, extra);
	ptr += 28 / sizeof(WCHAR);

	//
	// write an ASCII string for RtlFindActivationContextSectionString
	//

	strcpy((char *)ptr, "RtlFindActivationContextSectionString");

	extra->RtlFindActCtx_offset = ULONG_DIFF(ptr, extra);
	ptr += 44 / sizeof(WCHAR);

#ifdef _M_ARM64
	//
	// write an ASCII string for LdrQueryImageFileExecutionOptionsEx
	//

	strcpy((char *)ptr, "LdrQueryImageFileExecutionOptionsEx");

	extra->RtlImageOptionsEx_offset = ULONG_DIFF(ptr, extra);
	ptr += 40 / sizeof(WCHAR);
#endif

	//
	// ntdll loads kernel32 without a path, we will do the same
	// in our hook for RtlFindActivationContextSectionString,
	// see entry.asm
	//

	wcscpy(ptr, L"kernel32.dll");

	len = (ULONG)wcslen(ptr);
	extra->KernelDll_offset = ULONG_DIFF(ptr, extra);
	extra->KernelDll_length = len * sizeof(WCHAR);
	ptr += len + 1;

	//
	// append paths for native and wow64 SbieDll to the syscall buffer
	//

	wcscpy(ptr, sbie_home);
	wcscat(ptr, _SbieDll);

	len = (ULONG)wcslen(ptr);
	extra->NativeSbieDll_offset = ULONG_DIFF(ptr, extra);
	extra->NativeSbieDll_length = len * sizeof(WCHAR);
	ptr += len + 1;

#ifdef _M_ARM64
	wcscpy(ptr, sbie_home);
	wcscat(ptr, L"\\64");
	wcscat(ptr, _SbieDll);

	len = (ULONG)wcslen(ptr);
	extra->Arm64ecSbieDll_offset = ULONG_DIFF(ptr, extra);
	extra->Arm64ecSbieDll_length = len * sizeof(WCHAR);
	ptr += len + 1;
#endif

#ifdef _WIN64
	wcscpy(ptr, sbie_home);
	wcscat(ptr, L"\\32");
	wcscat(ptr, _SbieDll);

	len = (ULONG)wcslen(ptr);
	extra->Wow64SbieDll_offset = ULONG_DIFF(ptr, extra);
	extra->Wow64SbieDll_length = len * sizeof(WCHAR);
	ptr += len + 1;
#endif _WIN64


	// 
	// Note: the work area was now moved after the extra area 
	// hence the syscall_data is no longer overwritten
	//

	extra->InjectData_offset = ULONG_DIFF(ptr, extra);


	extra->Init_Lock = 0;


	//
	// adjust size of syscall buffer to include path strings
	//

	*syscall_data = ULONG_DIFF(ptr, syscall_data) + sizeof(INJECT_DATA);

	m_syscall_data = syscall_data;


#ifdef _M_ARM64

	//
	// create m_syscall_ec_data for arm64 ec
	//

	if (!m_syscall_ec_data) {
		syscall_ec_data = (ULONG *)HeapAlloc(GetProcessHeap(), 0, 8192);
		if (!syscall_ec_data)
			return STATUS_INSUFFICIENT_RESOURCES;
		*syscall_ec_data = 0;
	}
	else
		syscall_ec_data = m_syscall_ec_data;

	if (drv_init)
	{
		//
		// Search the ntdll.dll for the unexported syscall wrapper functions
		//

		void* HandleStubHijack = NULL;
		ULONG* aCode = SbieDll_FindFirstSysCallFunc((ULONG*)Dll_Ntdll, &HandleStubHijack);

		if (aCode == NULL) {
			SbieApi_Log(2303, L"syscall, wrappers not found");
			return STATUS_SUCCESS; // we will fail process creation for ec processes later
		}

		void* EcExitThunkPtr = SbieDll_GetEcExitThunkPtr(HandleStubHijack);

		//
		// create the syscall EC data from the ntdll.sll's syscall wrapper functions
		//

		const ULONG* SyscallPtr = (ULONG*)((ULONG64)syscall_data + sizeof(ULONG) + sizeof(ULONG) + (NATIVE_FUNCTION_SIZE * NATIVE_FUNCTION_COUNT));
		ULONG* SyscallPtrEC = (ULONG*)((ULONG64)syscall_ec_data + sizeof(ULONG) + sizeof(ULONG) + (NATIVE_FUNCTION_SIZE * NATIVE_FUNCTION_COUNT));

		for (ULONG pos = 0; pos < 128; aCode++, pos += 4) {

			ULONG SyscallNum = Hook_GetSysCallFunc(aCode, NULL);
			if (SyscallNum == -1)
				continue;

			if (SbieDll_GetSysCallOffset(SyscallPtr, SyscallNum)) {

				SyscallPtrEC[0] = SyscallNum;
				SyscallPtrEC[1] = ULONG_DIFF(aCode, Dll_Ntdll);
				SyscallPtrEC += 2;
			}

			//if (aCode[13] != 0 || aCode[14] != 0 || aCode[15] != 0 || aCode[16] != 0)
			//    break;
			//aCode += (13 + 3 + 3 + 1);
			aCode += 16;
			pos = 0; // reset
		}

		// end marker
		SyscallPtrEC[0] = 0;
		SyscallPtrEC[1] = 0;
		SyscallPtrEC += 2;

		// put the EcExitThunkPtr at (extra_data_offset - 8)
		*(ULONG64*)SyscallPtrEC = (ULONG64)EcExitThunkPtr;
		SyscallPtrEC += sizeof(ULONG64) / sizeof(ULONG);

		*syscall_ec_data = ULONG_DIFF(SyscallPtrEC, syscall_ec_data);
	}
	else
	{
		*syscall_ec_data = sizeof(ULONG) + sizeof(ULONG); // +248; // total_size 4, extra_data_offset 4, work area sizeof(INJECT_DATA)
	}

	//
	// copy the required functions duplicates to the ec data
	//

	memcpy(syscall_ec_data + 2, syscall_data + 2, NATIVE_FUNCTION_SIZE * NATIVE_FUNCTION_COUNT);

	//
	// copy the extra data section
	//

	len = *syscall_ec_data;
	ULONG extra_len = syscall_data[0] - syscall_data[1];

	memcpy((UCHAR*)syscall_ec_data + len, extra, extra_len);


	//
	// adjust length and extra offset
	//

	syscall_ec_data[1] = len;
	syscall_ec_data[0] = len + extra_len;

	m_syscall_ec_data = syscall_ec_data;
#endif

#undef ULONG_DIFF

	return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// InjectLow
//---------------------------------------------------------------------------


_FX ULONG SbieDll_InjectLow(HANDLE hProcess, ULONG init_flags, BOOLEAN dup_drv_handle)
{
	//SVC_PROCESS_MSG *msg = (SVC_PROCESS_MSG *)_msg;
	ULONG errlvl = 0;

	SBIELOW_DATA lowdata;
	memset(&lowdata, 0, sizeof(lowdata));

	lowdata.flags.init_flags = init_flags;

#ifdef _M_ARM64
	
	USHORT ProcessMachine = 0xFFFF;
	if (Dll_OsBuild >= 22000) { // Win 11
		PROCESS_MACHINE_INFORMATION info;
		if(GetProcessInformation(hProcess, (PROCESS_INFORMATION_CLASS)ProcessMachineTypeInfo, &info, sizeof(info)))
			ProcessMachine = info.ProcessMachine;
	} 
	else {  // Win 10
		IsWow64Process2(hProcess, &ProcessMachine, NULL);
	}

	if (ProcessMachine != 0xFFFF) {
        
		//
        // Currently 32-bit ARM processes are not supported,
        // there doesn't seam to be a significant amount of these out there anyways
        //

        if (ProcessMachine == IMAGE_FILE_MACHINE_ARMNT) {
            lowdata.flags.is_wow64 = 1;
			SetLastError(ERROR_NOT_SUPPORTED);
			errlvl = -1;
			goto finish;
        }
        else if (ProcessMachine == IMAGE_FILE_MACHINE_AMD64) {
            lowdata.flags.is_arm64ec = 1;
            lowdata.flags.is_xtajit = 1;
        }
        else if (ProcessMachine == IMAGE_FILE_MACHINE_I386) {
            lowdata.flags.is_wow64 = 1;
            lowdata.flags.is_xtajit = 1;
        }
	}
	else {

		SetLastError(ERROR_INVALID_FUNCTION);
		errlvl = 0xCC;
		goto finish;
	}

#endif

	//
	// verify all aspects of initialization were successful
	//
	if ((!m_sbielow_ptr) || (!m_syscall_data)
#ifdef _M_ARM64
			|| (lowdata.flags.is_arm64ec && ((!m_LdrInitializeThunkEC) || (!__sys_VirtualAlloc2) || (!m_syscall_ec_data)))
#endif
		) {

		SetLastError(ERROR_NOT_READY);
		errlvl = 0xFF;
		goto finish;
	}

	//
	// prepare the lowdata parameters area to copy into target process
	//

	lowdata.ntdll_base = (ULONG64)(ULONG_PTR)Dll_Ntdll;

#ifdef _WIN64
	static const WCHAR *_Ntdll32 = L"\\syswow64\\ntdll.dll";    // 19 chars
#ifdef _M_ARM64
	static const WCHAR *_NChpe32 = L"\\SyChpe32\\ntdll.dll";    // 19 chars

	//
	// Sandboxie requires CHPE to be disabled for x86 applications
	// for simplicity and compatibility with forced processes we disable CHPE globally using the Wow64\x86\xtajit key
	// alternatively we can hook LdrQueryImageFileExecutionOptionsEx from LowLevel.dll
	// and return 0 for the L"LoadCHPEBinaries" option
	// 
	// HKLM\SOFTWARE\Microsoft\Wow64\x86\xtajit -> LoadCHPEBinaries = 0
	// HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\{image_name} -> LoadCHPEBinaries = 0
	// HKCU\Software\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers -> {full_image_path} = "~ ARM64CHPEDISABLED"
	//  
#endif

	if (lowdata.flags.is_wow64) {
		lowdata.ntdll_wow64_base = FindDllBase64(hProcess, _Ntdll32);
#ifdef _M_ARM64
		if (!lowdata.ntdll_wow64_base) {
			//lowdata.ntdll_wow64_base = FindDllBase64(hProcess, _NChpe32);
			//if (lowdata.ntdll_wow64_base) 
			//	lowdata.flags.is_chpe32 = 1;
			//else {
				SetLastError(ERROR_NOT_SUPPORTED);
				errlvl = -2;
				goto finish;
			//}
		}
#endif
	}
#endif

	lowdata.RealNtDeviceIoControlFile = (ULONG64)GetProcAddress((HMODULE)lowdata.ntdll_base, "NtDeviceIoControlFile");
	lowdata.NativeNtProtectVirtualMemory = (ULONG64)GetProcAddress((HMODULE)lowdata.ntdll_base, "NtProtectVirtualMemory");
	lowdata.NativeNtRaiseHardError = (ULONG64)GetProcAddress((HMODULE)lowdata.ntdll_base, "NtRaiseHardError");

	//
	// on 64-bit Windows 8, there might be a difference of more than
	// 2GB bytes between ntdll and the injected SbieLow, which requires
	// use of longer jump sequences than the 5-byte 0xE9 relative jump
	//
	if (Dll_Windows >= 10) {
		lowdata.flags.is_win10 = 1;
	}

#ifdef _M_ARM64
	//
	// Windows on Arm64 offers only 16 bytes per syscall for our detour,
	// not enough to store the syscall number (4), load (4) a 64 bit address (8) and jump to it (4).
	// 
	// Hence we allocate the memory containing SystemServiceAsm in the lower 4 GB's
	// this way we save 4 bytes on the address and can fit in the 16 bytes available
	// 
	// An alternative aproche is to use the jump table as 16 bytes are just enough for 
	// a 64 bit detour, and in the jump table we have as much space per syscall as we need.
	//
#endif

	SIZE_T lowLevel_size;
#ifdef _WIN64 
	BOOLEAN use_jump_Table = FALSE;
	if(use_jump_Table)
		lowLevel_size = m_sbielow_len + sizeof(SBIELOW_J_TABLE) + 0x400;
	else
#endif
		lowLevel_size = m_sbielow_len;
		
	void *remote_addr = SbieDll_InjectLow_CopyCode(hProcess, lowLevel_size, m_sbielow_len, m_sbielow_ptr
#ifdef _M_ARM64
		, (BOOLEAN)lowdata.flags.is_arm64ec
#endif
	);

	if (remote_addr) {

		void* pLdrInitializeThunk = (void*)m_LdrInitializeThunk;
#ifdef _M_ARM64
		if (lowdata.flags.is_arm64ec) 
			pLdrInitializeThunk = (void*)m_LdrInitializeThunkEC;
#endif

		//
		// copy code at LdrInitializeThunk from new process
		//

		SIZE_T len1 = sizeof(lowdata.LdrInitializeThunk_tramp);
		SIZE_T len2 = 0;
		/*
		sprintf(buffer,"CopyCode: copy ldr size %d\n",code_len);
		OutputDebugStringA(buffer);
		*/
		BOOL vm_ok = ReadProcessMemory(
			hProcess, pLdrInitializeThunk, lowdata.LdrInitializeThunk_tramp,
			len1, &len2);

		if (!vm_ok || len1 != len2) {

			remote_addr = NULL;
		}
	}

	if (!remote_addr) {
		errlvl = 0x33;
		goto finish;
	}

#ifdef _WIN64 
	if (lowdata.flags.is_wow64) {

		//
		// when this is a 32 bit process running under WoW64, we need to inject also some 32 bit code
		//

		void* remote_addr32 = SbieDll_InjectLow_CopyCode(hProcess, m_sbielow32_len, m_sbielow32_len, m_sbielow32_ptr
#ifdef _M_ARM64
			, FALSE
#endif
		);

		if (remote_addr32) {

			ULONG protect;
			BOOL vm_ok = VirtualProtectEx(hProcess, remote_addr32, m_sbielow32_len,
				PAGE_EXECUTE_READ, &protect);
			if (vm_ok) {
				lowdata.ptr_32bit_detour = (ULONG64)((UCHAR*)remote_addr32 + m_sbielow32_detour_offset);
			}
		}

		if (!lowdata.ptr_32bit_detour) {
			errlvl = 0x88;
			goto finish;
		}
	}
#endif

#ifndef _M_ARM64
#ifdef _WIN64
	lowdata.flags.long_diff = 1;
	if (SbieDll_Has32BitJumpHorizon((void *)m_LdrInitializeThunk, remote_addr)) {
		lowdata.flags.long_diff = 0;
	}
#else
	lowdata.flags.long_diff = 0;
#endif
#endif

	if (dup_drv_handle)
	{
		//
		// duplicate the SbieDrv API file device handle into target process
		//

		lowdata.api_device_handle = (ULONG64)(ULONG_PTR)
			SbieDll_InjectLow_SendHandle(hProcess);
		if (!lowdata.api_device_handle) {

			errlvl = 0x22;
			goto finish;
		}

		lowdata.api_sbiedrv_ctlcode = API_SBIEDRV_CTLCODE;

		lowdata.api_invoke_syscall = API_INVOKE_SYSCALL;
	}

	//
	// the driver sent us a copy of some functions from ntdll in the
	// syscall_data buffer, copy that code to SbieLow through the
	// lowdata area (see also core/drv/syscall.c and core/low/lowdata.h)
	//

	memcpy(lowdata.NtDelayExecution_code, &m_syscall_data[2], (NATIVE_FUNCTION_SIZE * NATIVE_FUNCTION_COUNT));
	// lowdata.NtDelayExecution_code
	// lowdata.NtDeviceIoControlFile_code
	// lowdata.NtFlushInstructionCache_code
	// lowdata.NtProtectVirtualMemory_code

	//
	// allocate space for and write the lowlevel (SbieLow) code
	// and copy code at LdrInitializeThunk from target process
	//

#ifdef _WIN64
	if(use_jump_Table)
		lowdata.Sbie64bitJumpTable = (SBIELOW_J_TABLE *)((ULONG_PTR)remote_addr + m_sbielow_len + 0x400); //(0x400 - (m_sbielow_len & 0x3ff))+ m_sbielow_len;
#endif

	//
	// remove hard coded data block offset 
	ULONG_PTR tramp_remote_addr =   // calculate address in remote process
		(ULONG_PTR)remote_addr
		+ m_sbielow_data_offset     // offset of args area
		+ FIELD_OFFSET(SBIELOW_DATA, LdrInitializeThunk_tramp);

	if (!SbieDll_InjectLow_BuildTramp(lowdata.flags.long_diff == 1,
		lowdata.LdrInitializeThunk_tramp, tramp_remote_addr
#ifdef _M_ARM64
			, (BOOLEAN)lowdata.flags.is_arm64ec
#endif
		)) {

		//UCHAR *code = lowdata.LdrInitializeThunk_tramp;
		//SbieApi_LogEx(msg->session_id, 2335,
		//	L"%S [%02X %02X %02X %02X %02X %02X"
		//	L" %02X %02X %02X %02X %02X %02X]",
		//	msg->process_name,
		//	code[0], code[1], code[2], code[3], code[4], code[5],
		//	code[6], code[7], code[8], code[9], code[10], code[11]);

		SetLastError(ERROR_UNKNOWN_PRODUCT);

		errlvl = 0x44;
		goto finish;
	}

	//
	// copy the syscall data buffer (m_syscall_data) to target process
	//

	void* remote_syscall_data = SbieDll_InjectLow_CopySyscalls(hProcess, (BOOLEAN)lowdata.flags.is_wow64
#ifdef _M_ARM64
		, (BOOLEAN)lowdata.flags.is_arm64ec
#endif
	);
	if (!remote_syscall_data) {

		errlvl = 0x55;
		goto finish;
	}

	lowdata.syscall_data = (ULONG64)(ULONG_PTR)remote_syscall_data;

	//
	// write lowdata parameter area, including the converted trampoline
	// code, into target process, and make it execute-read
	//
	if (!SbieDll_InjectLow_CopyData(hProcess, remote_addr, &lowdata)) {

		errlvl = 0x66;
		goto finish;
	}

	//
	// overwrite the top of LdrInitializeThunk to jump to injected code
	// note that we have to skip the 8-byte signature (.HEAD.00)
	//
	// Removed hard coded dependency on (.HEAD.00). No longer need to add 8 to
	// the remote_addr
	if (!SbieDll_InjectLow_WriteJump(hProcess, (UCHAR *)remote_addr + m_sbielow_start_offset, lowdata.flags.long_diff == 1
#ifdef _M_ARM64
		, (BOOLEAN)lowdata.flags.is_arm64ec
#endif
	)) {
		errlvl = 0x77;
		goto finish;
	}

	//
	// finish
	//

finish:

	return errlvl;
}


//---------------------------------------------------------------------------
// InjectLow_AllocMemory
//---------------------------------------------------------------------------


_FX void* InjectLow_AllocMemory(HANDLE hProcess, SIZE_T size, BOOLEAN executable
#ifdef _M_ARM64
	, BOOLEAN use_arm64ec
#endif
) {
	SIZE_T region_size = size;
	void *remote_addr = NULL;

#ifdef _M_ARM64
	if (use_arm64ec && executable) {

		MEM_EXTENDED_PARAMETER Parameter = { 0 };
		Parameter.Type = MemExtendedParameterAttributeFlags;
		Parameter.ULong64 = MEM_EXTENDED_PARAMETER_EC_CODE;

		for (UINT_PTR base_addr = 0x10000; !remote_addr; base_addr <<= 1) {

			remote_addr = __sys_VirtualAlloc2(hProcess, (void*)base_addr, region_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE, &Parameter, 1);
		}

		return remote_addr;
	} 
#endif

	//
	// allocate virtual memory somewhere in the process.  to force an
	// address in the low 24-bits of the address space, we have to use
	// NtAllocateVirtalMemory and specify ZeroBits = 8 (32 - 8 = 24)
	//

	//for (int i = 8; !remote_addr && i > 2; i--) {
	for (int i = 8; !remote_addr && i >= 0; i--) {
		NTSTATUS status = NtAllocateVirtualMemory(hProcess, &remote_addr, i, &region_size, MEM_COMMIT | MEM_RESERVE, executable ? PAGE_EXECUTE_READWRITE : PAGE_READWRITE);
		if (!NT_SUCCESS(status)) {
			SetLastError(RtlNtStatusToDosError(status));
			remote_addr = NULL;
			region_size = size;
		}
	}

	return remote_addr;
}


//---------------------------------------------------------------------------
// InjectLow_SendHandle
//---------------------------------------------------------------------------


_FX HANDLE SbieDll_InjectLow_SendHandle(HANDLE hProcess)
{
	NTSTATUS status;
	HANDLE HandleLocal, HandleRemote;
	UNICODE_STRING uni;
	OBJECT_ATTRIBUTES objattrs;
	IO_STATUS_BLOCK MyIoStatusBlock;

	//
	// open the Sandboxie driver API file handle
	//

	RtlInitUnicodeString(&uni, API_DEVICE_NAME);
	InitializeObjectAttributes(
		&objattrs, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

	status = NtOpenFile(
		&HandleLocal, FILE_GENERIC_READ, &objattrs, &MyIoStatusBlock,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0);

	if (NT_SUCCESS(status)) {

		//
		// duplicate opened handle into new process
		//

		BOOL ok = DuplicateHandle(NtCurrentProcess(), HandleLocal,
			hProcess, &HandleRemote, 0, FALSE,
			DUPLICATE_SAME_ACCESS);

		CloseHandle(HandleLocal);

		if (ok) {

			return HandleRemote;
		}
	}

	return NULL;
}


//---------------------------------------------------------------------------
// InjectLow_CopyCode
//---------------------------------------------------------------------------


_FX void *SbieDll_InjectLow_CopyCode(HANDLE hProcess, SIZE_T total_size, SIZE_T lowLevel_size, const void* lowLevel_ptr
#ifdef _M_ARM64
	, BOOLEAN use_arm64ec
#endif
) {

	void* remote_addr = InjectLow_AllocMemory(hProcess, total_size, TRUE
#ifdef _M_ARM64
		, use_arm64ec
#endif
	);

	if (remote_addr) {

		//
		// copy SbieLow into the allocated region in the new process
		//

		SIZE_T len1 = lowLevel_size;
		SIZE_T len2 = 0;
		BOOL vm_ok = WriteProcessMemory(
			hProcess, remote_addr, lowLevel_ptr,
			len1, &len2);

		if (vm_ok && len1 == len2) {

			return remote_addr;
		}
	}

	return NULL;
}


//---------------------------------------------------------------------------
// InjectLow_BuildTramp
//---------------------------------------------------------------------------


_FX BOOLEAN SbieDll_InjectLow_BuildTramp(
	BOOLEAN long_diff, UCHAR *code, ULONG_PTR addr
#ifdef _M_ARM64
	, BOOLEAN use_arm64ec
#endif
) {

#ifdef _M_ARM64

	//
	// code contains already a copy of LdrInitializeThunk (48 bytes)
	// here we need to add the jump to the original at the right location
	//

	ULONG code_len = 16; // the length of the jump to detour code

	ULONG* aCode = (ULONG*)(code + code_len);
	*aCode++ = 0x58000048;	// ldr x8, 8
	*aCode++ = 0xD61F0100;	// br x8
	*(DWORD64*)aCode = (use_arm64ec ? m_LdrInitializeThunkEC : m_LdrInitializeThunk) + code_len;

#else

#define IS_1BYTE(a)     (                 code[offset + 0] == (a))
#define IS_2BYTE(a,b)   (IS_1BYTE(a)   && code[offset + 1] == (b))
#define IS_3BYTE(a,b,c) (IS_2BYTE(a,b) && code[offset + 2] == (c))

	//
	// skip past several bytes in the code copied from the top of the
	// LdrInitializeThunk function, where we will inject a jmp sequence.
	//
	// a simple E9 relative JMP five byte instruction in most cases,
	// a slightly longer seven byte version in case there is a long
	// distance between ntdll and SbieLow, i.e. on 64-bit Windows 8
	//

#ifdef _WIN64
	ULONG code_len = (long_diff ? 7 : ((Dll_Windows >= 10) ? 6 : 5));
#else
	ULONG code_len = 5;
#endif

	ULONG offset = 0;
	while (offset < code_len) {

		ULONG inst_len = 0;

		if (0)
			;

		// push ebp
		else if (IS_1BYTE(0x55))
			inst_len = 1;

		// mov ebp, esp
		else if (IS_2BYTE(0x8B, 0xEC))
			inst_len = 2;

		// mov edi, edi
		else if (IS_2BYTE(0x8B, 0xFF))
			inst_len = 2;

		// push ebx
		else if (IS_2BYTE(0xFF, 0xF3))
			inst_len = 2;

		// push rbx (Windows 8.1)
		else if (IS_2BYTE(0x40, 0x53))
			inst_len = 2;

		// mov dword ptr [esp+imm8],eax
		else if (IS_3BYTE(0x89, 0x44, 0x24))
			inst_len = 4;

		// lea eax, esp+imm8
		else if (IS_3BYTE(0x8D, 0x44, 0x24))
			inst_len = 4;

		// sub rsp, imm8
		else if (IS_3BYTE(0x48, 0x83, 0xEC))
			inst_len = 4;

		// mov rbx, rcx
		else if (IS_3BYTE(0x48, 0x8B, 0xD9))
			inst_len = 3;

		/*
		else if (IS_3BYTE(0x48, 0x8B, 0x04))
			inst_len = 4;
		*/
		//
		// abort if we don't recognize the instruction
		//

		if (!inst_len) {

			return FALSE;
		}

		offset += inst_len;
	}

#undef IS_3BYTE
#undef IS_2BYTE
#undef IS_1BYTE

	//
	// append a jump instruction at the bottom of our trampoline for
	// LdrInitializeThunk, which jumps back to the real LdrInitializeThunk
	//
	// note that on Windows 8 the difference between the address of
	// LdrInitializeThunk in the 64-bit ntdll and where SbieLow was copied
	// may be greater than 32-bit, so we use JMP QWORD rather than the
	// 5-byte 0xE9 relative JMP
	//
#ifdef _WIN64
	if (!long_diff) {

		if (Dll_Windows >= 10) {
			code[offset] = 0x48;
			code[offset + 1] = 0xE9;					// jmp
			*(ULONG *)&code[offset + 2] = (ULONG)
				(m_LdrInitializeThunk + offset - (addr + offset + 6));

		}
		else {
			code[offset] = 0xe9;
			*(ULONG *)&code[offset + 1] = (ULONG)
				(m_LdrInitializeThunk + offset - (addr + offset + 5));
		}
	}
	else {

		*(USHORT *)&code[offset] = 0x25FF;              // jmp qword ptr
		*(ULONG *)&code[offset + 2] = 0;
		*(ULONG64 *)&code[offset + 6] = m_LdrInitializeThunk + offset;
	}
#else
	code[offset] = 0xE9;								// jmp 

	*(ULONG *)&code[offset + 1] = (ULONG)
		(m_LdrInitializeThunk + offset - (addr + offset + 5));
#endif

#endif

	return TRUE;
}


//---------------------------------------------------------------------------
// InjectLow_CopySyscalls
//---------------------------------------------------------------------------


_FX void *SbieDll_InjectLow_CopySyscalls(HANDLE hProcess, BOOLEAN is_wow64
#ifdef _M_ARM64
	, BOOLEAN use_arm64ec
#endif
) {
	void *remote_addr = NULL;
	ULONG* data;
#ifdef _M_ARM64
	if(use_arm64ec)
		data = m_syscall_ec_data;
	else
#endif
		data = m_syscall_data;
	SIZE_T region_size = *data;

	remote_addr = InjectLow_AllocMemory(hProcess, region_size , FALSE
#ifdef _M_ARM64
		, FALSE
#endif
	);

	if (remote_addr) {

		//
		// copy the syscall data buffer into the new process
		//

		SIZE_T len1 = *data;
		SIZE_T len2 = 0;
		BOOL vm_ok = WriteProcessMemory(
			hProcess, remote_addr, data, len1, &len2);
		if (vm_ok && len1 == len2) {

			return remote_addr;
		}
	}

	return NULL;
}


//---------------------------------------------------------------------------
// InjectLow_CopyData
//---------------------------------------------------------------------------


_FX BOOLEAN SbieDll_InjectLow_CopyData(
	HANDLE hProcess, void *remote_addr, void *local_data)
{
	//
	// copy SBIELOW_DATA data into the area reserved within SbieLow
	// (i.e. at offset SBIELOW_DATA_OFFSET) in the new process
	//

	void *data_addr = (void *)((ULONG_PTR)remote_addr + m_sbielow_data_offset);
	SIZE_T len1 = sizeof(SBIELOW_DATA);
	SIZE_T len2 = 0;
	BOOL vm_ok = WriteProcessMemory(
		hProcess, data_addr, local_data, len1, &len2);
	if (vm_ok && len1 == len2) {

		ULONG protect;
		vm_ok = VirtualProtectEx(hProcess, remote_addr, m_sbielow_len,
			PAGE_EXECUTE_READ, &protect);
		if (vm_ok) {
			return TRUE;
		}
	}

	return FALSE;
}

#ifndef _M_ARM64
#ifdef _WIN64

_FX BOOLEAN SbieDll_Has32BitJumpHorizon(void * target, void * detour)
{
	ULONG_PTR diff;
	long long delta;
	diff = (ULONG_PTR)((ULONG_PTR)target - (ULONG_PTR)detour);
	delta = diff;
	delta < 0 ? delta *= -1 : delta;
	//is DetourFunc in 32bit jump range
	if (delta < 0x80000000) {
		return TRUE;
	}
	return FALSE;
}

_FX void * SbieDll_InjectLow_getPage(HANDLE hProcess, void *remote_addr)
{
	SIZE_T mySize;
	ULONG_PTR tempAddr;
	void * myTable = 0;
	UCHAR *func;
	ULONG myProtect;
	short myBuffer[1024];
	SIZE_T readSize;
	BOOL myVM;
	HANDLE myKernel32;
	HANDLE myNtDll;
	//HANDLE myTestDll;
	func = (UCHAR *)((ULONG_PTR)m_LdrInitializeThunk);
	myKernel32 = GetModuleHandleA("kernel32.dll");
	myNtDll = GetModuleHandleA("ntdll.dll");
	//  myTestDll = 0; 

	/*
	if(myTestDll) {
		//for testing remove this code!
		sprintf(buffer,"Dll Collision Test: address %p\n",myTestDll);
		OutputDebugStringA(buffer);
		myTable = VirtualAllocEx(hProcess,myTestDll,0x100, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
	}
	else {
	*/
	tempAddr = ((ULONG_PTR)myNtDll < (ULONG_PTR)myKernel32 ? (ULONG_PTR)myNtDll : (ULONG_PTR)myKernel32) - 0x10000;
	myTable = VirtualAllocEx(hProcess, (void *)tempAddr, 0x100, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	//  }
	/*
	else {
		//use hack if all else fails
		//OutputDebugStringA("Unable to allocate page!\n");
		max_attempts = 0;
	}
	*/

	if (myTable) {
		mySize = 0;
		if (SbieDll_Has32BitJumpHorizon(myTable, func)) {
			WriteProcessMemory(hProcess, myTable, &remote_addr, 8, &mySize);
			/*
			sprintf(buffer,"myPage = %p, kernel32 = %p, ntdll = %p\n",myTable,myKernel32,myNtDll);
			OutputDebugStringA(buffer);
			*/
			if (mySize == 8) {
				return myTable;
			}
		}
	}
	/*
	sprintf(buffer,"Failed to find table for target address %p, func = %p\n",myTable,func);
	OutputDebugStringA(buffer);
	*/

	readSize = 1;
	tempAddr = (ULONG_PTR)func - 8;
	ReadProcessMemory(hProcess, (void *)((ULONG_PTR)tempAddr), &myBuffer, 8, &readSize);

	// if hot patch area
	if (*((ULONG_PTR *)&myBuffer) == 0x9090909090909090 || *((ULONG_PTR *)&myBuffer) == 0xcccccccccccccccc) {
		//OutputDebugStringA("Using hotpatch area\n");
		myTable = (void *)tempAddr;
	}
	else { //not hot patch area: This is a hack
		//patch area in .rdata section of ntdll
		ReadProcessMemory(hProcess, (void *)((ULONG_PTR)tempAddr + 0x100000), myBuffer, sizeof(myBuffer), &readSize);
		if (readSize != sizeof(myBuffer)) {
			//OutputDebugStringA("Error reading Memory\n");
			return NULL;
		}
		for (int i = 0; i < sizeof(myBuffer) && !myTable; i++) {
			if (*((ULONG_PTR*)&myBuffer[i]) == 0x9090909090909090 ||
				*((ULONG_PTR*)&myBuffer[i]) == 0xcccccccccccccccc) {
				myTable = (void *)((ULONG_PTR)tempAddr + i);
				/*
				sprintf(buffer,"HACK: table found at %p, index %x\n",myTable, i);
				OutputDebugStringA(buffer);
				*/
			}
		}
		if (!myTable) {
			//OutputDebugStringA("Table not found\n");
			return NULL;
		}

	} //end else not hotpatch area

	myVM = VirtualProtectEx(hProcess, myTable, sizeof(void *), PAGE_READWRITE, &myProtect);
	if (myVM) {
		SIZE_T len2 = 0;
		myVM = WriteProcessMemory(hProcess, myTable, &remote_addr, 8, &len2);
		if (myVM && 8 == len2) {
			myVM = VirtualProtectEx(hProcess, myTable, 8, myProtect, &myProtect);
			if (myVM) {
				return myTable;
			}
		}
	}

	return NULL;
}
#endif  //#ifdef _WIN64
#endif //#ifndef _M_ARM64

//---------------------------------------------------------------------------
// InjectLow_WriteJump
//---------------------------------------------------------------------------
_FX BOOLEAN SbieDll_InjectLow_WriteJump(HANDLE hProcess, void *remote_addr, BOOLEAN long_diff
#ifdef _M_ARM64
	, BOOLEAN use_arm64ec
#endif
) {
	//
	// prepare a short prolog code that jumps to the injected SbieLow
	//
	UCHAR jump_code[20];
	void * detour = (void *)remote_addr;
	UCHAR *func = (UCHAR *)((ULONG_PTR)m_LdrInitializeThunk);
	SIZE_T len1;
	BOOL myVM;
	ULONG myProtect;

#ifdef _M_ARM64

	if(use_arm64ec)
		func = (UCHAR *)((ULONG_PTR)m_LdrInitializeThunkEC);

	ULONG* aCode = (ULONG*)jump_code;
	//*aCode++ = 0xD43E0000;	// brk #0xF000
	*aCode++ = 0x58000048;	// ldr x8, 8
	*aCode++ = 0xD61F0100;	// br x8
	*(DWORD64*)aCode = (DWORD64)detour; aCode += 2;

	len1 = (UCHAR*)aCode - jump_code;

#elif _WIN64
	if (!long_diff) {
		if (Dll_Windows >= 10) {
			len1 = 6;
			jump_code[0] = 0x48; //jump to entry code in entry.asm
			jump_code[1] = 0xE9; //jump to entry code in entry.asm
			*(ULONG *)(jump_code + 2) = (ULONG)((ULONG_PTR)detour - (m_LdrInitializeThunk + 6));
			//remote_addr = (void *)m_LdrInitializeThunk;
		}
		else {
			len1 = 5;
			jump_code[0] = 0xe9; //jump to entry code in entry.asm
			*(ULONG *)(jump_code + 1) = (ULONG)((ULONG_PTR)detour - (m_LdrInitializeThunk + 5));
		}
	}

	else {
		void * myTable = 0;
		if (!((ULONG_PTR)remote_addr & 0xffffffff00000000)) {
			len1 = 7;
			jump_code[0] = 0xB8;
			*(ULONG *)(jump_code + 1) = (ULONG)(ULONG_PTR)remote_addr;
			*(USHORT *)(jump_code + 5) = 0xE0FF;        // jmp rax
		}
		else {
			ULONG_PTR target;
			ULONG_PTR diff;

			len1 = 6;
			target = (ULONG_PTR)&func[6];
			myTable = SbieDll_InjectLow_getPage(hProcess, remote_addr);
			if (!myTable) {
				//OutputDebugStringA("Error: Table not set!\n");
				return FALSE;
			}

			diff = (ULONG_PTR) &((ULONG_PTR *)myTable)[0];
			diff = diff - target;

			*(USHORT *)&jump_code[0] = 0x25ff;
			*(ULONG *)&jump_code[2] = (ULONG)diff;
		}
	}
#else
	len1 = 5;
	jump_code[0] = 0xE9; //jump to entry code in entry.asm
	*(ULONG *)(jump_code + 1) = (ULONG)((ULONG_PTR)detour - (m_LdrInitializeThunk + 5));
	//remote_addr = (void *)m_LdrInitializeThunk;
#endif

	 //
	 // modify the bytes at LdrInitializeThunk with the prolog code
	 //

	myVM = VirtualProtectEx(hProcess, func, len1, PAGE_READWRITE, &myProtect);
	if (myVM) {
		SIZE_T len2 = 0;
		myVM = WriteProcessMemory(hProcess, func, jump_code, len1, &len2);
		/*
		sprintf(buffer,"WriteJump: len2 = %d\n",len2);
		OutputDebugStringA(buffer);
		*/
		if (myVM && len1 == len2) {
			myVM = VirtualProtectEx(hProcess, func, len1, myProtect, &myProtect);
			if (myVM) {
				return TRUE;
			}
		}
	}
	return FALSE;
}
