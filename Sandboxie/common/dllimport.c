/*
 * Copyright 2022 David Xanatos, xanasoft.com
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

#define WIN32_NO_STATUS
#include <windows.h>
#include "win32_ntddk.h"

// ntimage.h

typedef struct _IMAGE_CHPE_METADATA_X86 {
	ULONG  Version;
	ULONG  CHPECodeAddressRangeOffset;
	ULONG  CHPECodeAddressRangeCount;
	ULONG  WowA64ExceptionHandlerFunctionPointer;
	ULONG  WowA64DispatchCallFunctionPointer;
	ULONG  WowA64DispatchIndirectCallFunctionPointer;
	ULONG  WowA64DispatchIndirectCallCfgFunctionPointer;
	ULONG  WowA64DispatchRetFunctionPointer;
	ULONG  WowA64DispatchRetLeafFunctionPointer;
	ULONG  WowA64DispatchJumpFunctionPointer;
	ULONG  CompilerIATPointer;         // Present if Version >= 2
	ULONG  WowA64RdtscFunctionPointer; // Present if Version >= 3
} IMAGE_CHPE_METADATA_X86, * PIMAGE_CHPE_METADATA_X86;

typedef struct _IMAGE_CHPE_RANGE_ENTRY {
	union {
		ULONG StartOffset;
		struct {
			ULONG NativeCode : 1;
			ULONG AddressBits : 31;
		} DUMMYSTRUCTNAME;
	} DUMMYUNIONNAME;

	ULONG Length;
} IMAGE_CHPE_RANGE_ENTRY, * PIMAGE_CHPE_RANGE_ENTRY;

typedef struct _IMAGE_ARM64EC_METADATA {
	ULONG  Version;
	ULONG  CodeMap;
	ULONG  CodeMapCount;
	ULONG  CodeRangesToEntryPoints;
	ULONG  RedirectionMetadata;
	ULONG  tbd__os_arm64x_dispatch_call_no_redirect;
	ULONG  tbd__os_arm64x_dispatch_ret;
	ULONG  tbd__os_arm64x_dispatch_call;
	ULONG  tbd__os_arm64x_dispatch_icall;
	ULONG  tbd__os_arm64x_dispatch_icall_cfg;
	ULONG  AlternateEntryPoint;
	ULONG  AuxiliaryIAT;
	ULONG  CodeRangesToEntryPointsCount;
	ULONG  RedirectionMetadataCount;
	ULONG  GetX64InformationFunctionPointer;
	ULONG  SetX64InformationFunctionPointer;
	ULONG  ExtraRFETable;
	ULONG  ExtraRFETableSize;
	ULONG  __os_arm64x_dispatch_fptr;
	ULONG  AuxiliaryIATCopy;
} IMAGE_ARM64EC_METADATA;

typedef struct _IMAGE_ARM64EC_REDIRECTION_ENTRY {
	ULONG Source;
	ULONG Destination;
} IMAGE_ARM64EC_REDIRECTION_ENTRY;

// 

typedef NTSTATUS(__stdcall* P_NtQueryVirtualMemory64)(
	IN  HANDLE ProcessHandle,
	IN  DWORD64 BaseAddress,
	IN  MEMORY_INFORMATION_CLASS MemoryInformationClass,
	OUT PVOID MemoryInformation,
	IN  SIZE_T MemoryInformationLength,
	OUT PSIZE_T ReturnLength OPTIONAL);

typedef NTSTATUS(__stdcall* P_NtReadVirtualMemory64)(
    IN  HANDLE ProcessHandle,
    IN  DWORD64 BaseAddress,
    OUT PVOID Buffer,
    IN  SIZE_T BufferSize,
    OUT PSIZE_T NumberOfBytesRead OPTIONAL);

#ifdef _WIN64
P_NtQueryVirtualMemory64 NtQueryVirtualMemory64 = (P_NtQueryVirtualMemory64)NtQueryVirtualMemory;
P_NtReadVirtualMemory64 NtReadVirtualMemory64 = (P_NtReadVirtualMemory64)NtReadVirtualMemory;
#else
P_NtQueryVirtualMemory64 NtQueryVirtualMemory64 = NULL;
P_NtReadVirtualMemory64 NtReadVirtualMemory64 = NULL;
#endif

DWORD64 FindDllBase64(HANDLE hProcess, const WCHAR* dll)
{
	char buffer[512];
	ULONG len = wcslen(dll);

	if (!NtQueryVirtualMemory64)
		return -1;

	for (DWORD64 baseAddress = 0;;)
	{
		MEMORY_BASIC_INFORMATION64 basicInfo;
		if (!NT_SUCCESS(NtQueryVirtualMemory64(hProcess, baseAddress, MemoryBasicInformation, &basicInfo, sizeof(MEMORY_BASIC_INFORMATION64), NULL)))
			break;

        baseAddress = baseAddress + basicInfo.RegionSize;
		if (NT_SUCCESS(NtQueryVirtualMemory64(hProcess, basicInfo.AllocationBase, MemoryMappedFilenameInformation, buffer, sizeof(buffer), NULL)))
		{
			UNICODE_STRING64* FullImageName = (UNICODE_STRING*)buffer;
			if (FullImageName->Length > len * sizeof(WCHAR)) {

				WCHAR* path = (WCHAR*)((DWORD64)FullImageName->Buffer + FullImageName->Length - len * sizeof(WCHAR));
				if (_wcsicmp(path, dll) == 0) {

					return (DWORD64)basicInfo.AllocationBase;
				}
			}
		}
	}

	return 0;
}

BYTE* MapRemoteDll(HANDLE hProcess, DWORD64 DllBase) 
{
	DWORD64 MaxSize = 0;

	if (!NtQueryVirtualMemory64 || !NtReadVirtualMemory64)
		return (BYTE*)-1;

	for (DWORD64 baseAddress = 0;;)
	{
		MEMORY_BASIC_INFORMATION64 basicInfo;
		if (!NT_SUCCESS(NtQueryVirtualMemory64(hProcess, baseAddress, MemoryBasicInformation, &basicInfo, sizeof(MEMORY_BASIC_INFORMATION64), NULL)))
			break;

		baseAddress = baseAddress + basicInfo.RegionSize;
		if (basicInfo.AllocationBase == DllBase)
		{
			DWORD64 CurrentSize = (basicInfo.BaseAddress + basicInfo.RegionSize) - basicInfo.AllocationBase;
			if (MaxSize < CurrentSize)
				MaxSize = CurrentSize;
		}
	}

	if (MaxSize == 0)
		return NULL;

	BYTE* buffer = HeapAlloc(GetProcessHeap(), 0, (SIZE_T)MaxSize);
	if (!buffer)
		return NULL;

	BYTE* ptr = buffer;
	for (DWORD64 pos = DllBase; pos < (DllBase + MaxSize); pos += 0x1000) 
	{
		NtReadVirtualMemory64(hProcess, pos, ptr, 0x1000, NULL);
		ptr += 0x1000;
	}

	return buffer;
}

IMAGE_SECTION_HEADER* FindImageSection(DWORD rva, PIMAGE_NT_HEADERS32 pNTHeader)
{
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pNTHeader);
	for (ULONG i = 0; i < pNTHeader->FileHeader.NumberOfSections; i++, section++) {
		DWORD size = section->Misc.VirtualSize;
		if (size == 0)
			size = section->SizeOfRawData;
		if ((rva >= section->VirtualAddress) && (rva < (section->VirtualAddress + size)))
			return section;
	}
	return NULL;
}

DWORD64 FindImagePosition(DWORD rva, void* pNTHeader, DWORD64 imageBase)
{
	if (imageBase != 0) // live image
		return imageBase + rva;
	// file on disk
	IMAGE_SECTION_HEADER* pSectionHdr = FindImageSection(rva, pNTHeader);
	if (!pSectionHdr)
		return 0;
	DWORD delta = pSectionHdr->VirtualAddress - pSectionHdr->PointerToRawData;
	//return imageBase + rva - delta;
	return rva - delta;
}

static DWORD64 FindDllExport2(P_NtReadVirtualMemory64 ReadDll, HANDLE hProcess, DWORD64 DllBase, IMAGE_DATA_DIRECTORY *dir0, const char* ProcName, void* pNTHeader)
{
	NTSTATUS status;
	BYTE* buffer;
	DWORD64 proc = 0;

	DWORD64 dir0Address = FindImagePosition(dir0->VirtualAddress, pNTHeader, DllBase);

	buffer = HeapAlloc(GetProcessHeap(), 0, dir0->Size);
	status = ReadDll(hProcess, dir0Address, buffer, dir0->Size, NULL);

	IMAGE_EXPORT_DIRECTORY* exports = buffer;

	ULONG* names = (ULONG*)((DWORD64)buffer + exports->AddressOfNames - dir0->VirtualAddress);
	USHORT* ordinals = (USHORT*)((DWORD64)buffer + exports->AddressOfNameOrdinals - dir0->VirtualAddress);
	ULONG* functions = (ULONG*)((DWORD64)buffer + exports->AddressOfFunctions - dir0->VirtualAddress);

	for (ULONG i = 0; i < exports->NumberOfNames; ++i) {

		//BYTE* name = (BYTE*)DllBase + names[i];
		char* name = (char*)((DWORD64)exports + names[i] - dir0->VirtualAddress);
		
		if(strcmp(name, ProcName) == 0)
		{
			if (ordinals[i] < exports->NumberOfFunctions) {

				proc = DllBase + functions[ordinals[i]];

				// Note: if this is an arm32 image the real address has a 0x01 appended to indicate it uses the thumb instruction set
				//if (((PIMAGE_NT_HEADERS32)pNTHeader)->FileHeader.Machine == IMAGE_FILE_MACHINE_ARMNT)
				//	proc &= ~1;

				break;
			}
		}
	}

	if (proc && proc >= dir0Address && proc < dir0Address + dir0->Size) {

		//
		// if the export points inside the export table, then it is a
		// forwarder entry.  we don't handle these, because none of the
		// exports we need is a forwarder entry.  if this changes, we
		// might have to scan LDR tables to find the target dll
		//

		proc = 0;
	}

	HeapFree(GetProcessHeap(), 0, buffer);

	return proc;
}

DWORD64 ResolveWoWRedirection64(P_NtReadVirtualMemory64 ReadDll, HANDLE hProcess, DWORD64 DllBase, DWORD64 proc, DWORD64 CHPEMetadataPointer, void* pNTHeader)
{
	NTSTATUS status;

	IMAGE_ARM64EC_METADATA MetaData;
	status = ReadDll(hProcess, CHPEMetadataPointer, &MetaData, sizeof(MetaData), NULL);

	ULONG size = MetaData.RedirectionMetadataCount * sizeof(IMAGE_ARM64EC_REDIRECTION_ENTRY);
	BYTE* buffer = HeapAlloc(GetProcessHeap(), 0, size);
	status = ReadDll(hProcess, FindImagePosition(MetaData.RedirectionMetadata, pNTHeader, DllBase), buffer, size, NULL);
	IMAGE_ARM64EC_REDIRECTION_ENTRY* RedirectionMetadata = buffer;

	for (ULONG i = 0; i < MetaData.RedirectionMetadataCount; i++) {
		if ((proc - DllBase) == RedirectionMetadata[i].Source) {
			proc = DllBase + RedirectionMetadata[i].Destination;
			break;
		}
	}

	HeapFree(GetProcessHeap(), 0, buffer);

	return proc;
}

DWORD64 FindDllExport_impl(P_NtReadVirtualMemory64 ReadDll, HANDLE hProcess, DWORD64 DllBase, const char* ProcName)
{
	NTSTATUS status;
	DWORD64 proc = 0;

	IMAGE_DOS_HEADER* dos_hdr;
	IMAGE_NT_HEADERS* nt_hdrs;

	BYTE Buffer[0x10000];
	BYTE Buffer2[0x10000];

	status = ReadDll(hProcess, DllBase, Buffer, sizeof(Buffer), NULL);

	BOOLEAN resolve_ec = ProcName[0] == '#';
	if (resolve_ec)
		ProcName++;

	BOOLEAN resolve_exp = memcmp(ProcName, "EXP+", 4) == 0;
	if (resolve_exp)
		ProcName += 4;

	dos_hdr = Buffer;

	if (dos_hdr->e_magic != 'MZ' && dos_hdr->e_magic != 'ZM')
		return 0;
	nt_hdrs = (IMAGE_NT_HEADERS*)((BYTE*)dos_hdr + dos_hdr->e_lfanew);
	if (nt_hdrs->Signature != IMAGE_NT_SIGNATURE)     // 'PE\0\0'
		return 0;

	if (nt_hdrs->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {

		IMAGE_NT_HEADERS32* nt_hdrs_32 = (IMAGE_NT_HEADERS32*)nt_hdrs;
		IMAGE_OPTIONAL_HEADER32* opt_hdr_32 = &nt_hdrs_32->OptionalHeader;

		if (opt_hdr_32->NumberOfRvaAndSizes) {

			IMAGE_DATA_DIRECTORY* dir0 = &opt_hdr_32->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
			proc = FindDllExport2(ReadDll, hProcess, DllBase, dir0, ProcName, nt_hdrs_32);
		}
	}

	else if (nt_hdrs->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {

		IMAGE_NT_HEADERS64* nt_hdrs_64 = (IMAGE_NT_HEADERS64*)nt_hdrs;
		IMAGE_OPTIONAL_HEADER64* opt_hdr_64 = &nt_hdrs_64->OptionalHeader;

		if (opt_hdr_64->NumberOfRvaAndSizes) {

			IMAGE_LOAD_CONFIG_DIRECTORY64 LoadConfig;

			IMAGE_DATA_DIRECTORY* dir10 = &opt_hdr_64->DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG];
			if (resolve_ec && dir10->VirtualAddress && dir10->Size >= FIELD_OFFSET(IMAGE_LOAD_CONFIG_DIRECTORY64, CHPEMetadataPointer) + sizeof(ULONGLONG)) {

				status = ReadDll(hProcess, FindImagePosition(dir10->VirtualAddress, nt_hdrs_64, DllBase), &LoadConfig, min(sizeof(LoadConfig), dir10->Size), NULL);
			}

			typedef struct _DYN_RELOC_TABLE {
				ULONG Unknown1;
				ULONG Unknown2;
				ULONG Unknown3;
				ULONG Unknown4;
				ULONG TableSize;
				UCHAR Entries[];
			} DYN_RELOC_TABLE;
			
			DYN_RELOC_TABLE* DynamicValueRelocTable = NULL;

			if (DllBase == 0 && (resolve_ec || resolve_exp)) { // only for images on disk, on live images we take the actually used export directory

				PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nt_hdrs);
				nt_hdrs->FileHeader.NumberOfSections;

				section += (LoadConfig.DynamicValueRelocTableSection - 1);

				DWORD64 pos = FindImagePosition(section->VirtualAddress, nt_hdrs_64, DllBase);
				status = ReadDll(hProcess, pos, Buffer2, min(sizeof(Buffer2), section->Misc.VirtualSize), NULL);

				DynamicValueRelocTable = (DYN_RELOC_TABLE*)(Buffer2 + LoadConfig.DynamicValueRelocTableOffset);


				for (UCHAR* TablePtr = DynamicValueRelocTable->Entries; TablePtr < DynamicValueRelocTable->Entries + DynamicValueRelocTable->TableSize; ) {

					struct {
						ULONG Offset;
						ULONG Size;
					} *Section = TablePtr;
					TablePtr += 8;
					Section->Size -= 8;

					for (UCHAR* EntryPtr = TablePtr; TablePtr < EntryPtr + Section->Size; ) {

						struct {
							USHORT
								RVA : 12,
								Unknown : 1,
								Size : 3;
						} *Entry = TablePtr;
						TablePtr += 2;

						ULONGLONG Value = 0;
						memcpy(&Value, TablePtr, Entry->Size);
						TablePtr += Entry->Size;

						ULONG RVA = Section->Offset + Entry->RVA;

						// Apply the value relocs to our header buffer
						if (RVA < sizeof(Buffer)) {

							//DbgPrintf("%08x -> %08x\n", RVA, (ULONG)Value);
							memcpy(Buffer + RVA, &Value, Entry->Size);
						}
					}
				}
			}

			IMAGE_DATA_DIRECTORY* dir0 = &opt_hdr_64->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

			proc = FindDllExport2(ReadDll, hProcess, DllBase, dir0, ProcName, nt_hdrs_64);

			if(resolve_ec && LoadConfig.CHPEMetadataPointer) {
				if (DllBase == 0) // file on disk
					LoadConfig.CHPEMetadataPointer = FindImagePosition((DWORD)(LoadConfig.CHPEMetadataPointer - opt_hdr_64->ImageBase), nt_hdrs_64, DllBase);
				proc = ResolveWoWRedirection64(ReadDll, hProcess, DllBase, proc, LoadConfig.CHPEMetadataPointer, nt_hdrs_64);
			}
		}
	}

	return proc;
}

NTSTATUS __stdcall ReadLocalMemory(
	_In_ HANDLE Unused,
	_In_opt_ DWORD64 BaseAddress,
	_Out_writes_bytes_(BufferSize) PVOID Buffer,
	_In_ SIZE_T BufferSize,
	_Out_opt_ PSIZE_T pNumberOfBytesRead)
{
	memcpy(Buffer, (void*)BaseAddress, BufferSize);
	if (pNumberOfBytesRead) *pNumberOfBytesRead = BufferSize;
	return 0;
}

DWORD64 FindDllExportInMem(DWORD64 DllBase, const char* ProcName)
{
	return FindDllExport_impl(ReadLocalMemory, NULL, DllBase, ProcName);
}

DWORD64 FindRemoteDllExport(HANDLE hProcess, DWORD64 DllBase, const char* ProcName)
{
	if (NtReadVirtualMemory64 == NULL)
		return -1;

	return FindDllExport_impl(NtReadVirtualMemory64, hProcess, DllBase, ProcName);
}

NTSTATUS __stdcall ReadDllFile(
	_In_ HANDLE FileHandle,
	_In_opt_ DWORD64 BaseAddress,
	_Out_writes_bytes_(BufferSize) PVOID Buffer,
	_In_ SIZE_T BufferSize,
	_Out_opt_ PSIZE_T pNumberOfBytesRead)
{
	LARGE_INTEGER pos;
	pos.QuadPart = BaseAddress;
	SetFilePointerEx(FileHandle, pos, NULL, FILE_BEGIN);
	DWORD NumberOfBytesRead;
	BOOL ret = ReadFile(FileHandle, Buffer, (DWORD)BufferSize, &NumberOfBytesRead, NULL);
	if (pNumberOfBytesRead) *pNumberOfBytesRead = NumberOfBytesRead;
	return ret ? 0 : 0xC0000001L;
}

DWORD64 FindDllExportFromFile(const WCHAR* dll, const char* ProcName)
{
	DWORD64 proc;
	HANDLE hFile = CreateFileW(dll, GENERIC_READ, (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE), NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return 0;

	proc = FindDllExport_impl(ReadDllFile, hFile, 0, ProcName);

	CloseHandle(hFile);
	return proc;
}
