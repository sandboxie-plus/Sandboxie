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
 // Sandboxie Low Level code injection mechanism moved here from SbieSvc
 //---------------------------------------------------------------------------

#include "dll.h"

#include <windows.h>
#include <stdio.h>

#include "common/my_version.h"
#include "core/low/lowdata.h"
#include "common/win32_ntddk.h"
#include "core/drv/api_defs.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _MY_TARGETS {
	unsigned long long entry;
	unsigned long long data;
} MY_TARGETS;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

SBIEDLL_EXPORT  HANDLE SbieDll_InjectLow_SendHandle(HANDLE hProcess);

SBIEDLL_EXPORT  void *SbieDll_InjectLow_CopyCode(
	HANDLE hProcess, BOOLEAN iswow64, UCHAR *code, ULONG code_len);
SBIEDLL_EXPORT  BOOLEAN SbieDll_InjectLow_BuildTramp(
	BOOLEAN long_diff, UCHAR *code, ULONG_PTR addr);
SBIEDLL_EXPORT  void *SbieDll_InjectLow_CopySyscalls(HANDLE hProcess);
SBIEDLL_EXPORT  BOOLEAN SbieDll_InjectLow_CopyData(
	HANDLE hProcess, void *remote_addr, void *local_data);
#ifdef _WIN64
SBIEDLL_EXPORT BOOLEAN SbieDll_Has32BitJumpHorizon(void * target, void * detour);
SBIEDLL_EXPORT void * SbieDll_InjectLow_getPage(HANDLE hProcess, void *remote_addr);
#endif
SBIEDLL_EXPORT  BOOLEAN SbieDll_InjectLow_WriteJump(
	HANDLE hProcess, void *remote_addr, BOOLEAN long_diff, void *localdata);

//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


void *m_sbielow_ptr = NULL;
//adding two offsets variables to replace the "head" and "tail" dependency
ULONG m_sbielow_start_offset = 0;
ULONG m_sbielow_data_offset = 0;

ULONG m_sbielow_len = 0;

ULONG *m_syscall_data = NULL;

ULONG_PTR m_LdrInitializeThunk = 0;


//---------------------------------------------------------------------------
// InjectLow_InitHelper
//---------------------------------------------------------------------------


_FX ULONG SbieDll_InjectLow_InitHelper()
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

    HRSRC hrsrc = FindResource(Dll_Instance, L"LOWLEVEL", RT_RCDATA);
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

        if (nt_hdrs->Signature == IMAGE_NT_SIGNATURE) {   // 'PE\0\0'
#ifndef _WIN64
            if (nt_hdrs->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
                IMAGE_NT_HEADERS32 *nt_hdrs_32 = (IMAGE_NT_HEADERS32 *)nt_hdrs;
                IMAGE_OPTIONAL_HEADER32 *opt_hdr_32 = &nt_hdrs_32->OptionalHeader;
                data_dirs = &opt_hdr_32->DataDirectory[0];
                imageBase = opt_hdr_32->ImageBase;
            }
#else
            if (nt_hdrs->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
                IMAGE_NT_HEADERS64 *nt_hdrs_64 = (IMAGE_NT_HEADERS64 *)nt_hdrs;
                IMAGE_OPTIONAL_HEADER64 *opt_hdr_64 = &nt_hdrs_64->OptionalHeader;
                data_dirs = &opt_hdr_64->DataDirectory[0];
                imageBase = (ULONG_PTR)opt_hdr_64->ImageBase;
            }
#endif
            else {

				return errlvl;
            }
        }
        else {

			return errlvl;
        }
    }

    if (imageBase) {
		return errlvl;
    }

    section = IMAGE_FIRST_SECTION(nt_hdrs);
    if (nt_hdrs->FileHeader.NumberOfSections < 2) return errlvl;
    if (strncmp((char *)section[0].Name, SBIELOW_INJECTION_SECTION, strlen(SBIELOW_INJECTION_SECTION)) ||
        strncmp((char *)section[1].Name, SBIELOW_SYMBOL_SECTION, strlen(SBIELOW_SYMBOL_SECTION))) {
		return errlvl;
    }


    targets = (MY_TARGETS *)& bindata[section[1].PointerToRawData];
    m_sbielow_start_offset = (ULONG)targets->entry - section[0].VirtualAddress;
    m_sbielow_data_offset = (ULONG)targets->data - section[0].VirtualAddress;

    m_sbielow_ptr = bindata + section[0].PointerToRawData; //Old version: head;
    m_sbielow_len = section[0].SizeOfRawData; //Old version: (ULONG)(ULONG_PTR)(tail - head);

    if ((!m_sbielow_start_offset) || (!m_sbielow_data_offset))
		return errlvl;

    //
    // record information about ntdll and the virtual memory system
    //

	errlvl = 0x33;

    m_LdrInitializeThunk = (ULONG_PTR) GetProcAddress(Dll_Ntdll, "LdrInitializeThunk");

    if (! m_LdrInitializeThunk)
		return errlvl;

#ifdef _WIN64
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

	//
	// Get the SbieDll Location
	//

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

		status = SbieApi_CallOne(API_QUERY_SYSCALLS, (ULONG_PTR)syscall_data);
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

		*syscall_data = 4 + 4 + 232; // size 4, offset 4, work area sizeof(INJECT_DATA)

		const char* NtdllExports[] = { "NtDelayExecution", "NtDeviceIoControlFile", "NtFlushInstructionCache", "NtProtectVirtualMemory" };
		for (ULONG i = 0; i < 4; ++i) {
			void* func_ptr = GetProcAddress(Dll_Ntdll, NtdllExports[i]);
			memcpy((void*)((ULONG_PTR)syscall_data + (4 + 4) + (32 * i)), func_ptr, 32);
		}

		len = *syscall_data;
	}

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
	// write an ASCII string for NtRaiseHardError
	//

	strcpy((char *)ptr, "NtRaiseHardError");

	extra->NtRaiseHardError_offset = ULONG_DIFF(ptr, extra);
	ptr += 20 / sizeof(WCHAR);

	//
	// write an ASCII string for RtlFindActivationContextSectionString
	//

	strcpy((char *)ptr, "RtlFindActivationContextSectionString");

	extra->RtlFindActCtx_offset = ULONG_DIFF(ptr, extra);
	ptr += 44 / sizeof(WCHAR);

	//
	// ntdll loads kernel32 without a path, we will do the same
	// in our hook for RtlFindActivationContextSectionString,
	// see entry.asm
	//

	wcscpy(ptr, L"kernel32.dll");

	len = wcslen(ptr);
	extra->KernelDll_offset = ULONG_DIFF(ptr, extra);;
	extra->KernelDll_length = len * sizeof(WCHAR);
	ptr += len + 1;

	//
	// append paths for native and wow64 SbieDll to the syscall buffer
	//

	wcscpy(ptr, sbie_home);
	wcscat(ptr, _SbieDll);

	len = wcslen(ptr);
	extra->NativeSbieDll_offset = ULONG_DIFF(ptr, extra);;
	extra->NativeSbieDll_length = len * sizeof(WCHAR);
	ptr += len + 1;

#ifdef _WIN64

	wcscpy(ptr, sbie_home);
	wcscat(ptr, L"\\32");
	wcscat(ptr, _SbieDll);

	len = wcslen(ptr);
	extra->Wow64SbieDll_offset = ULONG_DIFF(ptr, extra);
	extra->Wow64SbieDll_length = len * sizeof(WCHAR);
	ptr += len + 1;

#endif _WIN64

	//
	// adjust size of syscall buffer to include path strings
	//

	*syscall_data = ULONG_DIFF(ptr, syscall_data);

#undef ULONG_DIFF

	m_syscall_data = syscall_data;

	return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// FindWOW64_Ntdll
//---------------------------------------------------------------------------


ULONG64 SbieDll_FindWOW64_Ntdll(_In_ HANDLE ProcessHandle)
{
	/*typedef NTSTATUS(*P_NtQueryVirtualMemory)(
		IN  HANDLE ProcessHandle,
		IN  PVOID BaseAddress,
		IN  MEMORY_INFORMATION_CLASS MemoryInformationClass,
		OUT PVOID MemoryInformation,
		IN  SIZE_T MemoryInformationLength,
		OUT PSIZE_T ReturnLength OPTIONAL);

	P_NtQueryVirtualMemory __sys_NtQueryVirtualMemory = (P_NtQueryVirtualMemory)GetProcAddress(_Ntdll, "NtQueryVirtualMemory");*/

	static const WCHAR *_Ntdll32 = L"\\syswow64\\ntdll.dll";    // 19 chars

	char buffer[512];

	for (PVOID baseAddress = NULL;;)
	{
		MEMORY_BASIC_INFORMATION basicInfo;
		if (!NT_SUCCESS(NtQueryVirtualMemory(
			ProcessHandle,
			baseAddress,
			MemoryBasicInformation,
			&basicInfo,
			sizeof(MEMORY_BASIC_INFORMATION),
			NULL
		)))
		{
			break;
		}

		baseAddress = (PVOID)((ULONG_PTR)(baseAddress)+(ULONG_PTR)(basicInfo.RegionSize));

		if (NT_SUCCESS(NtQueryVirtualMemory(
			ProcessHandle,
			basicInfo.AllocationBase,
			MemoryMappedFilenameInformation,
			buffer,
			sizeof(buffer),
			NULL
		)))
		{
			UNICODE_STRING *FullImageName = (UNICODE_STRING*)buffer;
			if (FullImageName->Length > 19 * sizeof(WCHAR)) {

				WCHAR *path = FullImageName->Buffer
					+ FullImageName->Length / sizeof(WCHAR)
					- 19;
				if (_wcsicmp(path, _Ntdll32) == 0) {

					return (ULONG64)basicInfo.AllocationBase;
				}
			}
		}
	}

	return 0;
}


//---------------------------------------------------------------------------
// InjectLow
//---------------------------------------------------------------------------


_FX ULONG SbieDll_InjectLow(HANDLE hProcess, BOOLEAN is_wow64, BOOLEAN bHostInject, BOOLEAN dup_drv_handle)
{
	//SVC_PROCESS_MSG *msg = (SVC_PROCESS_MSG *)_msg;
	ULONG errlvl = 0;

	//
	// verify all aspects of initialization were successful
	//
	if ((!m_sbielow_ptr) || (!m_syscall_data)) {

		SetLastError(ERROR_NOT_READY);
		errlvl = 0xFF;
		goto finish;
	}

	//
	// prepare the lowdata paramters area to copy into target process
	//

	SBIELOW_DATA lowdata;
	memzero(&lowdata, sizeof(lowdata));

#ifdef _WIN64
	if (is_wow64)//(msg->is_wow64)
		lowdata.ntdll_wow64_base = SbieDll_FindWOW64_Ntdll(hProcess);
#endif
	lowdata.ntdll_base = (ULONG64)(ULONG_PTR)Dll_Ntdll;

	lowdata.is_wow64 = is_wow64; //msg->is_wow64;
	lowdata.bHostInject = bHostInject; //msg->bHostInject;

	lowdata.RealNtDeviceIoControlFile = (ULONG64)GetProcAddress((HMODULE)lowdata.ntdll_base, "NtDeviceIoControlFile");
	//
	// on 64-bit Windows 8, there might be a difference of more than
	// 2GB bytes between ntdll and the injected SbieLow, which requires
	// use of longer jump sequences than the 5-byte 0xE9 relative jump
	//
	if (Dll_Windows >= 10) {
		lowdata.is_win10 = TRUE;
	}

	void *remote_addr = SbieDll_InjectLow_CopyCode(hProcess, lowdata.is_wow64, lowdata.LdrInitializeThunk_tramp, sizeof(lowdata.LdrInitializeThunk_tramp));
	if (!remote_addr) {
		errlvl = 0x33;
		goto finish;
	}
	//   if (lowdata.is_wow64 && (m_addr_high != m_addr_high_32))
#ifdef _WIN64
	lowdata.long_diff = TRUE;
	if (SbieDll_Has32BitJumpHorizon((void *)m_LdrInitializeThunk, remote_addr)) {
		lowdata.long_diff = FALSE;
	}
#else
	lowdata.long_diff = FALSE;
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

	memcpy(lowdata.NtDelayExecution_code, &m_syscall_data[2], (32 * 4));

	//
	// allocate space for and write the lowlevel (SbieLow) code
	// and copy code at LdrInitializeThunk from target process
	//

#ifdef _WIN64
	lowdata.Sbie64bitJumpTable = (SBIELOW_J_TABLE *)((ULONG_PTR)remote_addr + m_sbielow_len + 0x400); //(0x400 - (m_sbielow_len & 0x3ff))+ m_sbielow_len;
#endif

	//
	// remove hard coded data block offset 
	ULONG_PTR tramp_remote_addr =   // calculate address in remote process
		(ULONG_PTR)remote_addr
		+ m_sbielow_data_offset     // offset of args area
		+ FIELD_OFFSET(SBIELOW_DATA, LdrInitializeThunk_tramp);

	if (!SbieDll_InjectLow_BuildTramp(lowdata.long_diff,
		lowdata.LdrInitializeThunk_tramp, tramp_remote_addr)) {

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

	void *remote_syscall_data = SbieDll_InjectLow_CopySyscalls(hProcess);
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
	if (!SbieDll_InjectLow_WriteJump(hProcess, (UCHAR *)remote_addr + m_sbielow_start_offset, lowdata.long_diff, &lowdata)) {
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


_FX void *SbieDll_InjectLow_CopyCode(HANDLE hProcess, BOOLEAN iswow64, UCHAR *code, ULONG code_len)
{
	SIZE_T region_size;
	SIZE_T lowLevel_size;
	void *remote_addr = NULL;
#ifdef _WIN64 
	lowLevel_size = m_sbielow_len + sizeof(SBIELOW_J_TABLE) + 0x400;
#else
	lowLevel_size = m_sbielow_len;
#endif
	region_size = lowLevel_size;

	for (int i = 8; !remote_addr && i > 2; i--) {
		NTSTATUS status = NtAllocateVirtualMemory(hProcess, &remote_addr, i, &region_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (!NT_SUCCESS(status)) {
			remote_addr = NULL;
			region_size = lowLevel_size;
		}
	}
	if (remote_addr) {

		//
		// copy SbieLow into the allocated region in the new process
		//

		SIZE_T len1 = m_sbielow_len;
		SIZE_T len2 = 0;
		BOOL vm_ok = WriteProcessMemory(
			hProcess, remote_addr, m_sbielow_ptr,
			len1, &len2);

		if (vm_ok && len1 == len2) {

			//
			// copy code at LdrInitializeThunk from new process
			//

			len1 = code_len;
			len2 = 0;
			/*
			sprintf(buffer,"CopyCode: copy ldr size %d\n",code_len);
			OutputDebugStringA(buffer);
			*/
			vm_ok = ReadProcessMemory(
				hProcess, (void *)m_LdrInitializeThunk, code,
				len1, &len2);

			if (vm_ok && len1 == len2) {

				return remote_addr;
			}
		}
	}

	return NULL;
}


//---------------------------------------------------------------------------
// InjectLow_BuildTramp
//---------------------------------------------------------------------------


_FX BOOLEAN SbieDll_InjectLow_BuildTramp(
	BOOLEAN long_diff, UCHAR *code, ULONG_PTR addr)
{

#define IS_1BYTE(a)     (                 code[offset + 0] == (a))
#define IS_2BYTE(a,b)   (IS_1BYTE(a)   && code[offset + 1] == (b))
#define IS_3BYTE(a,b,c) (IS_2BYTE(a,b) && code[offset + 2] == (c))

	//
	// skip past several bytes in the code copied from the top of the
	// LdrInitializeThunk fuction, where we will inject a jmp sequence.
	//
	// a simple E9 relative JMP five byte instruction in most cases,
	// a slightly longer seven byte version in case there is a long
	// distance between ntdll and SbieLow, i.e. on 64-bit Windows 8
	//

	ULONG code_len = (long_diff ? 7 : 5);

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
			code[offset + 1] = 0xE9;
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
	code[offset] = 0xE9;

	*(ULONG *)&code[offset + 1] = (ULONG)
		(m_LdrInitializeThunk + offset - (addr + offset + 5));
#endif
	return TRUE;
}


//---------------------------------------------------------------------------
// InjectLow_CopySyscalls
//---------------------------------------------------------------------------


_FX void *SbieDll_InjectLow_CopySyscalls(HANDLE hProcess)
{
	//
	// allocate virtual memory somewhere in the process.  to force an
	// address in the low 24-bits of the address space, we have to use
	// NtAllocateVirtalMemory and specify ZeroBits = 8 (32 - 8 = 24)
	//

	void *remote_addr = NULL;
	SIZE_T region_size = *m_syscall_data;
	int i;
	NTSTATUS status;

	for (i = 8; !remote_addr && i > 2; i--) {
		status = NtAllocateVirtualMemory(hProcess, &remote_addr, i, &region_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (!NT_SUCCESS(status)) {
			remote_addr = NULL;
			region_size = *m_syscall_data;
		}
	}

	if (!NT_SUCCESS(status)) {
		remote_addr = NULL;
		SetLastError(RtlNtStatusToDosError(status));
	}
	if (remote_addr) {

		//
		// copy the syscall data buffer into the new process
		//

		SIZE_T len1 = *m_syscall_data;
		SIZE_T len2 = 0;
		BOOL vm_ok = WriteProcessMemory(
			hProcess, remote_addr, m_syscall_data, len1, &len2);
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

//---------------------------------------------------------------------------
// InjectLow_WriteJump
//---------------------------------------------------------------------------
_FX BOOLEAN SbieDll_InjectLow_WriteJump(HANDLE hProcess, void *remote_addr, BOOLEAN long_diff, void *localdata)
{
	//
	// prepare a short prolog code that jumps to the injected SbieLow
	//
	UCHAR jump_code[16];
	void * detour = (void *)remote_addr;
	UCHAR *func = (UCHAR *)((ULONG_PTR)m_LdrInitializeThunk);;
	SIZE_T len1;
	BOOL myVM;
	ULONG myProtect;

#ifdef _WIN64
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
