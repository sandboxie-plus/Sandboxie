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
// Functions
//---------------------------------------------------------------------------

#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

#include <windows.h>
#include "common/win32_ntddk.h"
#include "common/defines.h"
#include "lowdata.h"

//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _INJECT_DATA {

    ULONG syscall_data_len;
    ULONG extra_data_offset;

    ULONG64 LdrLoadDll;
    ULONG64 LdrGetProcAddr;
    ULONG64 NtRaiseHardError;
    ULONG64 RtlFindActCtx;

    ULONG RtlFindActCtx_Protect;
    UCHAR RtlFindActCtx_Bytes[12];

    USHORT  KernelDll_Length;
    USHORT  KernelDll_MaxLen;
    ULONG   KerneDll_Buf32;
    ULONG64 KerneDll_Buf64;

    USHORT  SbieDll_Length;
    USHORT  SbieDll_MaxLen;
    ULONG   SbieDll_Buf32;
    ULONG64 SbieDll_Buf64;

    ULONG64 ModuleHandle;
    ULONG64 SbieDllOrdinal1;

#ifdef _WIN64

    UCHAR   DetourCode[128];

#endif _WIN64

} INJECT_DATA;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static UCHAR *FindDllExport(void *DllBase, const UCHAR *ProcName);

static UCHAR *FindDllExport2(
    void *DllBase, IMAGE_DATA_DIRECTORY *dir0, const UCHAR *ProcName);

static void InitInjectWow64(SBIELOW_DATA *data, void* RtlFindActivationContextSectionString );


//---------------------------------------------------------------------------


#define SBIELOW_CALL(x) ((P_##x)&data->x##_code)


//---------------------------------------------------------------------------
// FindDllExport
//---------------------------------------------------------------------------


_FX UCHAR *FindDllExport(void *DllBase, const UCHAR *ProcName)
{
    IMAGE_DOS_HEADER *dos_hdr;
    IMAGE_NT_HEADERS *nt_hdrs;

    //
    // find the DllMain entrypoint for the dll
    //

    dos_hdr = (void *)DllBase;
    if (dos_hdr->e_magic != 'MZ' && dos_hdr->e_magic != 'ZM')
        return NULL;
    nt_hdrs = (IMAGE_NT_HEADERS *)((UCHAR *)dos_hdr + dos_hdr->e_lfanew);
    if (nt_hdrs->Signature != IMAGE_NT_SIGNATURE)     // 'PE\0\0'
        return NULL;

    if (nt_hdrs->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {

        IMAGE_NT_HEADERS32 *nt_hdrs_32 = (IMAGE_NT_HEADERS32 *)nt_hdrs;
        IMAGE_OPTIONAL_HEADER32 *opt_hdr_32 =
            &nt_hdrs_32->OptionalHeader;

        if (opt_hdr_32->NumberOfRvaAndSizes) {

            IMAGE_DATA_DIRECTORY *dir0 = &opt_hdr_32->DataDirectory[0];
            return FindDllExport2(DllBase, dir0, ProcName);
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
            return FindDllExport2(DllBase, dir0, ProcName);
        }
    }

#endif _WIN64

    return NULL;
}


//---------------------------------------------------------------------------
// FindDllExport2
//---------------------------------------------------------------------------


_FX UCHAR *FindDllExport2(
    void *DllBase, IMAGE_DATA_DIRECTORY *dir0, const UCHAR *ProcName)
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

            proc = NULL;
        }
    }

    return proc;
}


//---------------------------------------------------------------------------
// InitInject
//---------------------------------------------------------------------------

#ifdef _WIN64

_FX void InitInject(SBIELOW_DATA *data, void * RtlFindActivationContextSectionString,void * RtlFindActivationContextSectionString64)
#else
_FX void InitInject(SBIELOW_DATA *data, void * RtlFindActivationContextSectionString)
#endif
{
    void *ntdll_base;
    INJECT_DATA *inject;
    SBIELOW_EXTRA_DATA *extra;
    UCHAR *LdrCode, *MyHookCode;
    void *RegionBase;
    SIZE_T RegionSize;
    ULONG OldProtect;

    //
    // now that syscalls were intercepted, we can use the top of the syscall
    // data area as a work area for injecting our dll, but we still need the
    // information from the extra area at the bottom of the data area
    //

    inject = (INJECT_DATA *)data->syscall_data;

    extra = (SBIELOW_EXTRA_DATA *)
                    (data->syscall_data + inject->extra_data_offset);

    //
    // in a 32-bit program on 64-bit Windows, we need to hook the
    // 32-bit ntdll (ntdll32) rather than the 64-bit ntdll, and we can
    // ask our driver for the base address of the 32-bit ntdll
    //

    ntdll_base = (void *)data->ntdll_base;

#ifdef _WIN64

    if (data->is_wow64) {

        extern ULONG64 SbieApi_QueryProcessInfo(
                                    SBIELOW_DATA *data, ULONG info_type);

        ULONG ntdll32_base = (ULONG)SbieApi_QueryProcessInfo(data, 'nt32');

        ntdll_base = (void *)(ULONG_PTR)ntdll32_base;

        //
        // (prior to Windows 8, the base address of ntdll32 is recorded
        // in offset 0x036C of the KUSER_SHARED_DATA structure, which always
        // has a fixed base address of 0x7FFE0000.  this is not available
        // in Windows 8, so we have to rely on the driver to track this
        // base address via Process_NotifyImage in core/drv/process.c.
        // so we might as well use this approach for all 64-bit Windows.)
        //
        //ULONG ntdll32_base = *(ULONG *)(0x7FFE0000 + 0x036C);
        //ntdll_base = (void *)(ULONG_PTR)ntdll32_base;
        //ntdll_base = (void *)0x77300000;
    }

#endif _WIN64

    //
    // find the addresses of the ntdll (or ntdll32) functions:
    // LdrLoadDll, LdrGetProcedureAddress, NtRaiseHardError,
    // and RtlFindActivationContextSectionString
    //
    LdrCode = FindDllExport(ntdll_base,
                    (UCHAR *)extra + extra->LdrLoadDll_offset);
    if (! LdrCode)
        return;
    inject->LdrLoadDll = (ULONG_PTR)LdrCode;

    LdrCode = FindDllExport(ntdll_base,
                    (UCHAR *)extra + extra->LdrGetProcAddr_offset);
    if (! LdrCode)
        return;
    inject->LdrGetProcAddr = (ULONG_PTR)LdrCode;

    LdrCode = FindDllExport(ntdll_base,
                    (UCHAR *)extra + extra->NtRaiseHardError_offset);
    if (! LdrCode)
        return;
    inject->NtRaiseHardError = (ULONG_PTR)LdrCode;

    LdrCode = FindDllExport(ntdll_base,
                    (UCHAR *)extra + extra->RtlFindActCtx_offset);
    if (! LdrCode)
        return;
    inject->RtlFindActCtx = (ULONG_PTR)LdrCode;

    //
    // prepare unicode strings
    //

    inject->KernelDll_Length = (USHORT)extra->KernelDll_length;
    inject->KernelDll_MaxLen = inject->KernelDll_Length + sizeof(WCHAR);
    inject->KerneDll_Buf32   =
                    (ULONG)((ULONG_PTR)extra + extra->KernelDll_offset);
    inject->KerneDll_Buf64   =
                  (ULONG64)((ULONG_PTR)extra + extra->KernelDll_offset);

#ifdef _WIN64

    if (data->is_wow64) {

        InitInjectWow64(data,RtlFindActivationContextSectionString);
        goto store_sbielow_address;
    }

#endif _WIN64

    inject->SbieDll_Length = (SHORT)extra->NativeSbieDll_length;
    inject->SbieDll_MaxLen = inject->SbieDll_Length + sizeof(WCHAR);
    inject->SbieDll_Buf32  =
                    (ULONG)((ULONG_PTR)extra + extra->NativeSbieDll_offset);
    inject->SbieDll_Buf64  =
                    (ULONG)((ULONG_PTR)extra + extra->NativeSbieDll_offset);

    //
    // select version of RtlFindActivationContextSectionString detour code:
    // because both the 32-bit and 64-bit versions of this SbieLow code must
    // handle 32-bit programs, both versions include the 32-bit detour code.
    // (see entry.asm)
    //



    //
    // modify our RtlFindActivationContextSectionString detour code in
    // entry.asm to include a hard coded pointer to the inject data area.
    
#ifdef _WIN64
    MyHookCode = (UCHAR *) RtlFindActivationContextSectionString64;
    RegionBase = (void *)(MyHookCode -  8);
    RegionSize = sizeof(ULONG_PTR);
    SBIELOW_CALL(NtProtectVirtualMemory)(
        NtCurrentProcess(), &RegionBase, &RegionSize,
        PAGE_EXECUTE_READWRITE, &OldProtect);

    *(ULONG_PTR *)(MyHookCode - 8) = (ULONG_PTR)inject;

    SBIELOW_CALL(NtProtectVirtualMemory)(
        NtCurrentProcess(), &RegionBase, &RegionSize,
        OldProtect, &OldProtect);

    RegionBase = (void *)&LdrCode[0];
    RegionSize = 12;
    SBIELOW_CALL(NtProtectVirtualMemory)(
        NtCurrentProcess(), &RegionBase, &RegionSize,
        PAGE_EXECUTE_READWRITE, &inject->RtlFindActCtx_Protect);
    memcpy(&inject->RtlFindActCtx_Bytes, LdrCode, 12);

    LdrCode[0] = 0x48;
    LdrCode[1] = 0xb8;
    *(ULONG_PTR *)&LdrCode[2] = (ULONG_PTR)MyHookCode;
    LdrCode[10] = 0xff;
    LdrCode[11] = 0xe0;


#else
    MyHookCode = (UCHAR *)RtlFindActivationContextSectionString;
    RegionBase = (void *)(MyHookCode + 1);
    RegionSize = sizeof(ULONG_PTR);

    SBIELOW_CALL(NtProtectVirtualMemory)(
        NtCurrentProcess(), &RegionBase, &RegionSize,
        PAGE_EXECUTE_READWRITE, &OldProtect);

    *(ULONG *)(MyHookCode + 1) = (ULONG)(ULONG_PTR)inject;

    SBIELOW_CALL(NtProtectVirtualMemory)(
        NtCurrentProcess(), &RegionBase, &RegionSize,
        OldProtect, &OldProtect);

    RegionBase = (void *)LdrCode;
    RegionSize = 5;

    SBIELOW_CALL(NtProtectVirtualMemory)(
        NtCurrentProcess(), &RegionBase, &RegionSize,
        PAGE_EXECUTE_READWRITE, &inject->RtlFindActCtx_Protect);

    memcpy(&inject->RtlFindActCtx_Bytes, LdrCode, 5);

    LdrCode[0] = 0xE9;
    *(ULONG *)&LdrCode[1] = (ULONG)(MyHookCode - (LdrCode + 5));
#endif

    //
    // at this point we no longer need the data at top of the
    // syscall/inject data area, so store a pointer back to SbieLow
    // data area, for use by SbieDll ordinal 1
    //


#ifdef _WIN64
store_sbielow_address:
#endif _WIN64

    *(ULONG64 *)inject = (ULONG64)data;
}


//---------------------------------------------------------------------------
// InitInjectWow64
//---------------------------------------------------------------------------


#ifdef _WIN64


_FX void InitInjectWow64(SBIELOW_DATA *data, void * RtlFindActivationContextSectionString)
{
    INJECT_DATA *inject;
    SBIELOW_EXTRA_DATA *extra;
    UCHAR *LdrCode, *MyCode;
    void *RegionBase;
    SIZE_T RegionSize;
    ULONG i;

    //
    // find inject and extra data areas, same as in InitInject()
    //

    inject = (INJECT_DATA *)data->syscall_data;

    extra = (SBIELOW_EXTRA_DATA *) (data->syscall_data + inject->extra_data_offset);

    //
    // prepare unicode strings
    //

    inject->SbieDll_Length = (SHORT)extra->Wow64SbieDll_length;
    inject->SbieDll_MaxLen = inject->SbieDll_Length + sizeof(WCHAR);
    inject->SbieDll_Buf32  =
                    (ULONG)((ULONG_PTR)extra + extra->Wow64SbieDll_offset);

    //
    // select the 32-bit version of RtlFindActivationContextSectionString
    // detour code, and copy into the syscall data area which is always
    // allocated in the low 32-bit of the address space in a 32-bit context
    //
    // currently copies 128 bytes of the detour code, make sure to keep
    // this in sync with the code in entry.asm
    //


    MyCode = (UCHAR *) RtlFindActivationContextSectionString;
    for (i = 0; i < 128 / sizeof(ULONG64); ++i)
        ((ULONG64 *)inject->DetourCode)[i] = ((ULONG64 *)MyCode)[i];

    MyCode = inject->DetourCode;

    //
    // modify our copied detour code to include a hard coded pointer to
    // the inject data area (which is the syscall data area)
    //

    *(ULONG *)(MyCode + 1) = (ULONG)(ULONG_PTR)inject;

    //
    // hook the top of RtlFindActivationContextSectionString
    // to jump to our copied detour
    //

    LdrCode = (UCHAR *)inject->RtlFindActCtx;

    RegionBase = (void *)LdrCode;
    RegionSize = 5;

    SBIELOW_CALL(NtProtectVirtualMemory)(
        NtCurrentProcess(), &RegionBase, &RegionSize,
        PAGE_EXECUTE_READWRITE, &inject->RtlFindActCtx_Protect);

    memcpy(&inject->RtlFindActCtx_Bytes, LdrCode, 5);

    LdrCode[0] = 0xE9;
    *(ULONG *)&LdrCode[1] = (ULONG)(MyCode - (LdrCode + 5));
}


#endif _WIN64
