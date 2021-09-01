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
// DLL Management
//---------------------------------------------------------------------------


#include "dll.h"
#include <ntimage.h>


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


struct _DLL_ENTRY {

    LIST_ELEM list_elem;
    WCHAR name[32];
    HANDLE hFile;
    HANDLE hSection;
    ULONG_PTR ImageBase;
    ULONG SizeOfImage;
    UCHAR *base;
    IMAGE_NT_HEADERS *nthdr;
    IMAGE_EXPORT_DIRECTORY *exports;

};


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static void *Dll_GetProc2(
    DLL_ENTRY *dll, const UCHAR *ProcName, BOOLEAN returnOffset);


//---------------------------------------------------------------------------


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, Dll_Init)
#pragma alloc_text (INIT, Dll_RvaToAddr)
#pragma alloc_text (INIT, Dll_GetProc)
#pragma alloc_text (INIT, Dll_GetNextProc)
#endif // ALLOC_PRAGMA


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static LIST Dll_List;
static BOOLEAN Dll_List_Initialized = FALSE;

const WCHAR *Dll_NTDLL = L"NTDLL";
const WCHAR *Dll_USER = L"USER32";


//---------------------------------------------------------------------------
// Dll_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Dll_Init(void)
{
    List_Init(&Dll_List);
    Dll_List_Initialized = TRUE;

    if (! Dll_Load(Dll_NTDLL)) // ntoskrnl.exe - ntdll.dll
        return FALSE;
    if (! Dll_Load(Dll_USER)) // win32k.sys - w10: win32u.dll - w7: user32.dll & gdi32.dll
        return FALSE;

    return TRUE;
}


//---------------------------------------------------------------------------
// Dll_Unload
//---------------------------------------------------------------------------


_FX void Dll_Unload(void)
{
    DLL_ENTRY *dll;

    if (! Dll_List_Initialized)
        return;

    while (1) {
        dll = List_Head(&Dll_List);
        if (! dll)
            break;
        List_Remove(&Dll_List, dll);

        if (dll->base)
            ZwUnmapViewOfSection(NtCurrentProcess(), dll->base);
        if (dll->hSection)
            ZwClose(dll->hSection);
        if (dll->hFile)
            ZwClose(dll->hFile);
    }
}


//---------------------------------------------------------------------------
// Dll_Load
//---------------------------------------------------------------------------


_FX DLL_ENTRY *Dll_Load(const WCHAR *DllBaseName)
{
    static const WCHAR *_DotDll = L".dll";
    NTSTATUS status;
    DLL_ENTRY *dll;
    WCHAR path[128];
    UNICODE_STRING uni;
    OBJECT_ATTRIBUTES objattrs;
    IO_STATUS_BLOCK MyIoStatusBlock;
    FILE_STANDARD_INFORMATION info;
    IMAGE_DOS_HEADER *dos_hdr;
    IMAGE_DATA_DIRECTORY *data_dirs;
    ULONG err;
    SIZE_T mapsize;

    //
    // if we already have a loaded dll instance, return that
    //

    dll = List_Head(&Dll_List);
    while (dll) {
        if (_wcsicmp(dll->name, DllBaseName) == 0)
            return dll;
        dll = List_Next(dll);
    }

    //
    // otherwise we allocate a dll instance and attempt to initialize it.
    // if any error occurs during initialization, the return dll->base
    // and dll->exports will be null, and the instance should not be used
    //

    dll = Mem_Alloc(Driver_Pool, sizeof(DLL_ENTRY));
    if (! dll) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        err = 0x11;
        goto finish;
    }

    memzero(dll, sizeof(DLL_ENTRY));
    wcscpy(dll->name, DllBaseName);

    //
    // open the dll file and query its on-disk size
    //

    RtlStringCbPrintfW(path, sizeof(path), L"\\SystemRoot\\System32\\%s%s", DllBaseName, _DotDll);

#ifdef _WIN64

    /* if we have to open a 32-bit DLL on 64-bit Windows, adjust path
       if (DllBaseName == Dll_NTDLL_32) {
        wmemcpy(path + 15, L"Wow64", 5);
        wmemcpy(path + 26, _DotDll, 5);
    }*/

#endif _WIN64

    RtlInitUnicodeString(&uni, path);
    InitializeObjectAttributes(
        &objattrs, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = ZwCreateFile(
        &dll->hFile, FILE_GENERIC_READ, &objattrs, &MyIoStatusBlock, NULL,
        0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN, FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
        NULL, 0);
    if (! NT_SUCCESS(status)) {
        dll->hFile = NULL;
        err = 0x12;
        goto finish;
    }

    status = ZwQueryInformationFile(
        dll->hFile, &MyIoStatusBlock, &info,
        sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);
    if (! NT_SUCCESS(status)) {
        err = 0x13;
        goto finish;
    }

    // create and map the dll according to its on-disk size

    status = ZwCreateSection(
        &dll->hSection, SECTION_MAP_READ | SECTION_QUERY, NULL,
        &info.EndOfFile, PAGE_READONLY, SEC_RESERVE, dll->hFile);
    if (! NT_SUCCESS(status)) {
        dll->hSection = NULL;
        err = 0x14;
        goto finish;
    }

    dll->base = NULL;
    mapsize = (SIZE_T)info.EndOfFile.QuadPart;
    status = ZwMapViewOfSection(
        dll->hSection, NtCurrentProcess(), &dll->base, 0, 0, 0,
        &mapsize, ViewUnmap, 0, PAGE_READONLY);
    if (! NT_SUCCESS(status)) {
        dll->base = NULL;
        err = 0x15;
        goto finish;
    }

    //
    // find the image exports directory in the mapped dll instance
    //

    data_dirs = NULL;

    dos_hdr = (IMAGE_DOS_HEADER *)dll->base;
    if (dos_hdr->e_magic == 'MZ' || dos_hdr->e_magic == 'ZM') {

        IMAGE_NT_HEADERS *nt_hdrs =
            (IMAGE_NT_HEADERS *)((UCHAR *)dos_hdr + dos_hdr->e_lfanew);
        if (nt_hdrs->Signature == IMAGE_NT_SIGNATURE) {   // 'PE\0\0'

            dll->nthdr = nt_hdrs;

            if (nt_hdrs->OptionalHeader.Magic ==
                IMAGE_NT_OPTIONAL_HDR32_MAGIC) {

                IMAGE_NT_HEADERS32 *nt_hdrs_32 =
                    (IMAGE_NT_HEADERS32 *)nt_hdrs;
                IMAGE_OPTIONAL_HEADER32 *opt_hdr_32 =
                    &nt_hdrs_32->OptionalHeader;
                data_dirs = &opt_hdr_32->DataDirectory[0];
                dll->ImageBase = opt_hdr_32->ImageBase;
                dll->SizeOfImage = opt_hdr_32->SizeOfImage;

            } else if (nt_hdrs->OptionalHeader.Magic ==
                       IMAGE_NT_OPTIONAL_HDR64_MAGIC) {

                IMAGE_NT_HEADERS64 *nt_hdrs_64 =
                    (IMAGE_NT_HEADERS64 *)nt_hdrs;
                IMAGE_OPTIONAL_HEADER64 *opt_hdr_64 =
                    &nt_hdrs_64->OptionalHeader;
                data_dirs = &opt_hdr_64->DataDirectory[0];
                dll->ImageBase = (ULONG_PTR)opt_hdr_64->ImageBase;
                dll->SizeOfImage = opt_hdr_64->SizeOfImage;
            }
        }
    }

    if (! data_dirs) {
        status = STATUS_UNSUCCESSFUL;
        err = 0x16;     // data_dirs still null, must be an invalid image
        goto finish;
    }

    dll->exports = (IMAGE_EXPORT_DIRECTORY *)Dll_RvaToAddr(
        dll, data_dirs[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

    if (! dll->exports) {
        err = 0x17;     // could not translate rva to addr
        goto finish;
    }

    //
    // finalize
    //

    List_Insert_After(&Dll_List, NULL, dll);
    err = 0;

finish:
    if (err) {
        Log_Status_Ex(MSG_DLL_LOAD, err, status, DllBaseName);
        dll = NULL;
    }

    return dll;
}


//---------------------------------------------------------------------------
// Dll_RvaToAddr
//---------------------------------------------------------------------------


_FX void *Dll_RvaToAddr(DLL_ENTRY *dll, ULONG rva)
{
    IMAGE_SECTION_HEADER *section;
    ULONG i;

    section = IMAGE_FIRST_SECTION(dll->nthdr);
    for (i = 0; i < dll->nthdr->FileHeader.NumberOfSections; ++i) {

        if (rva >= section->VirtualAddress &&
            rva <  section->VirtualAddress + section->SizeOfRawData) {

            void *addr = dll->base
                + rva - section->VirtualAddress + section->PointerToRawData;
            return addr;
        }

        ++section;
    }

    return NULL;
}


//---------------------------------------------------------------------------
// Dll_GetProc
//---------------------------------------------------------------------------


_FX void *Dll_GetProc(
    const WCHAR *DllName, const UCHAR *ProcName, BOOLEAN returnOffset)
{
    void *proc = NULL;

    DLL_ENTRY *dll = Dll_Load(DllName);
    if (dll) {

        UCHAR *name;
        ULONG index;
        ULONG offset = Dll_GetNextProc(dll, ProcName, &name, &index);

        if (offset && strcmp(name, ProcName) == 0) {

            if (returnOffset)
                proc = (UCHAR *)(ULONG_PTR)offset;
            else
                proc = Dll_RvaToAddr(dll, offset);
        }
    }

    if (! proc) {
        WCHAR dll_proc_name[96];
        RtlStringCbPrintfW(dll_proc_name, sizeof(dll_proc_name), L"%s.%S", DllName, ProcName);
        Log_Msg1(MSG_DLL_GET_PROC, dll_proc_name);
    }

    return proc;
}


//---------------------------------------------------------------------------
// Dll_GetNextProc
//---------------------------------------------------------------------------


_FX ULONG Dll_GetNextProc(
    DLL_ENTRY *dll, const UCHAR *SearchName,
    UCHAR **FoundName, ULONG *FoundIndex)
{
    ULONG *names = Dll_RvaToAddr(dll, dll->exports->AddressOfNames);
    ULONG *addrs = Dll_RvaToAddr(dll, dll->exports->AddressOfFunctions);
    USHORT *ordis = Dll_RvaToAddr(dll, dll->exports->AddressOfNameOrdinals);

    ULONG i;
    ULONG dll_offset = 0;
    UCHAR *dll_name = NULL;

    if (names && addrs && ordis) {

        if (SearchName) {

            for (i = 0; i < dll->exports->NumberOfNames; ++i) {

                dll_name = Dll_RvaToAddr(dll, names[i]);
                if (dll_name && strcmp(dll_name, SearchName) >= 0) {

                    dll_offset = addrs[ordis[i]];
                    break;
                }
            }

            if (! dll_offset) {

                WCHAR dll_proc_name[96];
                RtlStringCbPrintfW(dll_proc_name, sizeof(dll_proc_name), L"%s.%S", dll->name, SearchName);
                Log_Msg1(MSG_1112, dll_proc_name);
            }

        } else {

            i = *FoundIndex + 1;
            if (i < dll->exports->NumberOfNames) {

                dll_offset = addrs[ordis[i]];
                dll_name = Dll_RvaToAddr(dll, names[i]);
            }
        }

        *FoundName = dll_name;
        *FoundIndex = i;
    }

    return dll_offset;
}
