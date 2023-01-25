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
// Functions
//---------------------------------------------------------------------------

_FX NTSTATUS SbieApi_DebugError(SBIELOW_DATA* data, ULONG error);

UCHAR *FindDllExport(void *DllBase, const UCHAR *ProcName, ULONG *pErr);

static UCHAR *FindDllExport2(
    void *DllBase, IMAGE_DATA_DIRECTORY *dir0, const UCHAR *ProcName, ULONG *pErr);

#ifdef _M_ARM64
void* Hook_GetFFSTarget(UCHAR* SourceFunc);
#endif

static void InitInjectWow64(SBIELOW_DATA *data);

//---------------------------------------------------------------------------


#define SBIELOW_CALL(x) ((P_##x)&data->x##_code)


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
        *pErr = 0xa;
        return NULL;
    }
    nt_hdrs = (IMAGE_NT_HEADERS *)((UCHAR *)dos_hdr + dos_hdr->e_lfanew);
    if (nt_hdrs->Signature != IMAGE_NT_SIGNATURE) {     // 'PE\0\0'
        *pErr = 0xb;
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


//---------------------------------------------------------------------------
// FindDllExport2
//---------------------------------------------------------------------------


_FX UCHAR *FindDllExport2(
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

            *pErr = 0xc;
            proc = NULL;
        }
    }

    return proc;
}

#ifdef _M_ARM64

//---------------------------------------------------------------------------
// MyGetProcedureAddress
//---------------------------------------------------------------------------


_FX NTSTATUS MyGetProcedureAddress(HMODULE ModuleHandle, PANSI_STRING FunctionName, WORD Ordinal, PVOID*FunctionAddress, INJECT_DATA *inject)
{
    SBIELOW_DATA* data = (SBIELOW_DATA*)*(ULONG64*)inject;

    typedef (*P_LdrGetProcedureAddress)(HMODULE, PANSI_STRING, WORD, PVOID*);
    NTSTATUS status = ((P_LdrGetProcedureAddress)inject->LdrGetProcAddr)(ModuleHandle, FunctionName, Ordinal, FunctionAddress);

    //
    // in ARM64EC mode unwrap the FFS and return the native function
    //

    if (data->flags.is_arm64ec && status >= 0) {
        *FunctionAddress = Hook_GetFFSTarget(*FunctionAddress);
        if (!*FunctionAddress)
            return STATUS_ENTRYPOINT_NOT_FOUND;
    }

    return status;
}

#endif

//---------------------------------------------------------------------------
// InitInject
//---------------------------------------------------------------------------


_FX void InitInject(SBIELOW_DATA *data, void *DetourCode)
{
    void *ntdll_base;
    SYSCALL_DATA* syscall_data;
    INJECT_DATA *inject;
    SBIELOW_EXTRA_DATA *extra;
    UCHAR *LdrCode, *MyHookCode;
    void *RegionBase;
    SIZE_T RegionSize;
    ULONG OldProtect;
    ULONG uError = 0;

    //
    // now that syscalls were intercepted, we can use the top of the syscall
    // data area as a work area for injecting our dll, but we still need the
    // information from the extra area at the bottom of the data area
    //

    syscall_data = (SYSCALL_DATA *)data->syscall_data;

    extra = (SBIELOW_EXTRA_DATA *) (data->syscall_data + syscall_data->extra_data_offset);

    inject = (INJECT_DATA *) ((UCHAR *)extra + extra->InjectData_offset);

    inject->sbielow_data = (ULONG64)data;

    //
    // in a 32-bit program on 64-bit Windows, we need to hook the
    // 32-bit ntdll (ntdll32) rather than the 64-bit ntdll, and we can
    // ask our driver for the base address of the 32-bit ntdll
    //

    ntdll_base = (void *)data->ntdll_base;

#ifdef _WIN64

    if (data->flags.is_wow64) {

		//
		// Instead of requiring the driver for this task, we can simplify it
		// and use NtQueryVirtualMemory to find the mapped image directly.
		// We do that in the injector, but we could also have done it here ourselves.
		//

		ntdll_base = (void *)data->ntdll_wow64_base;

        //extern ULONG64 SbieApi_QueryProcessInfo(
        //                            SBIELOW_DATA *data, ULONG info_type);
		//
        //ULONG ntdll32_base = (ULONG)SbieApi_QueryProcessInfo(data, 'nt32');
		//
        //ntdll_base = (void *)(ULONG_PTR)ntdll32_base;

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
                    (UCHAR *)extra + extra->LdrLoadDll_offset, &uError);
    if (!LdrCode) {
        SbieApi_DebugError(data, (0x01 << 4) | uError);
        return;
    }
#ifdef _M_ARM64
    if (data->flags.is_arm64ec)
        LdrCode = Hook_GetFFSTarget(LdrCode);
#endif
    if (!LdrCode) {
        SbieApi_DebugError(data, 0x01d);
        return;
    }
    inject->LdrLoadDll = (ULONG_PTR)LdrCode;

    LdrCode = FindDllExport(ntdll_base,
                    (UCHAR *)extra + extra->LdrGetProcAddr_offset, &uError);
    if (!LdrCode) {
        SbieApi_DebugError(data, (0x02 << 4) | uError);
        return;
    }
#ifdef _M_ARM64
    if (data->flags.is_arm64ec)
        LdrCode = Hook_GetFFSTarget(LdrCode);
#endif
    if (!LdrCode) {
        SbieApi_DebugError(data, 0x02d);
        return;
    }
    inject->LdrGetProcAddr = (ULONG_PTR)LdrCode;

#ifdef _M_ARM64

    //
    // on ARM64EC we hook the native code hence we need the custom MyGetProcedureAddress
    // to obtain the address of the native original 1 from our SbieDll.dll
    // instead of the FFS sequence as given by NtGetProcedureAddress
    //

    inject->MyGetProcAddr = (ULONG_PTR)MyGetProcedureAddress;
#endif

#ifdef _WIN64
    if (data->flags.is_wow64) {
        LdrCode = FindDllExport(ntdll_base,
            (UCHAR*)extra + extra->NtRaiseHardError_offset, &uError);
        if (!LdrCode) {
            SbieApi_DebugError(data, (0x03 << 4) | uError);
            return;
        }
        inject->NtRaiseHardError = (ULONG_PTR)LdrCode;
    }
    else
#endif

    //
    // for ARM64EC we need native functions, FindDllExport can manage FFS's
    // however this does not work for syscalls, hence we use the native function directly
    //
        inject->NtRaiseHardError = data->NativeNtRaiseHardError;

#ifdef _M_ARM64

    //
    // when hooking on arm64, go for LdrLoadDll 
    // instead of RtlFindActivationContextSectionString
    // for ARM64 both work, but for ARM64EC hooking RtlFindActCtx fails
    //

    if (!data->flags.is_wow64)
        LdrCode = (UCHAR*)inject->LdrLoadDll;
    else
#endif
    {
        LdrCode = FindDllExport(ntdll_base,
                        (UCHAR *)extra + extra->RtlFindActCtx_offset, &uError);
        if (!LdrCode) {
            SbieApi_DebugError(data, (0x04 << 4) | uError);
            return;
        }
    }
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
    if (data->flags.is_wow64) {

        InitInjectWow64(data);
        return;
    }
#endif _WIN64

#ifdef _M_ARM64
    if (data->flags.is_arm64ec) {

        inject->SbieDll_Length = (SHORT)extra->Arm64ecSbieDll_length;
        inject->SbieDll_MaxLen = inject->SbieDll_Length + sizeof(WCHAR);
        inject->SbieDll_Buf64 =
            (ULONG64)((ULONG_PTR)extra + extra->Arm64ecSbieDll_offset);
    }
    else
#endif
    {
        inject->SbieDll_Length = (SHORT)extra->NativeSbieDll_length;
        inject->SbieDll_MaxLen = inject->SbieDll_Length + sizeof(WCHAR);
        inject->SbieDll_Buf32  =
            (ULONG)((ULONG_PTR)extra + extra->NativeSbieDll_offset);
        inject->SbieDll_Buf64 =
            (ULONG64)((ULONG_PTR)extra + extra->NativeSbieDll_offset);
    }


    //
    // select version of RtlFindActivationContextSectionString detour code:
    // because both the 32-bit and 64-bit versions of this SbieLow code must
    // handle 32-bit programs, both versions include the 32-bit detour code.
    // (see entry.asm)
    //


    //
    // modify our RtlFindActivationContextSectionString detour code in
    // entry.asm to include a hard coded pointer to the inject data area.
    
#ifdef _M_ARM64

    MyHookCode = (UCHAR *) DetourCode;
    RegionBase = (void *)(MyHookCode -  8);
    RegionSize = sizeof(ULONG_PTR);
    SBIELOW_CALL(NtProtectVirtualMemory)(
        NtCurrentProcess(), &RegionBase, &RegionSize,
        PAGE_EXECUTE_READWRITE, &OldProtect);

    *(ULONG_PTR *)(MyHookCode - 8) = (ULONG_PTR)inject;

    SBIELOW_CALL(NtProtectVirtualMemory)(
        NtCurrentProcess(), &RegionBase, &RegionSize,
        OldProtect, &OldProtect);

    RegionBase = (void *)&LdrCode[0]; // RtlFindActCtx
    RegionSize = 16;
    SBIELOW_CALL(NtProtectVirtualMemory)(
        NtCurrentProcess(), &RegionBase, &RegionSize,
        PAGE_EXECUTE_READWRITE, &inject->RtlFindActCtx_Protect);
    memcpy(&inject->RtlFindActCtx_Bytes, LdrCode, 16);

	ULONG* aCode = (ULONG*)LdrCode;
	*aCode++ = 0x58000048;	// ldr x8, 8
	*aCode++ = 0xD61F0100;	// br x8
	*(DWORD64*)aCode = (DWORD64)MyHookCode;

    SBIELOW_CALL(NtFlushInstructionCache)(
        NtCurrentProcess(), RegionBase, (ULONG)RegionSize);

#elif _WIN64

    MyHookCode = (UCHAR *) DetourCode;
    RegionBase = (void *)(MyHookCode -  8);
    RegionSize = sizeof(ULONG_PTR);
    SBIELOW_CALL(NtProtectVirtualMemory)(
        NtCurrentProcess(), &RegionBase, &RegionSize,
        PAGE_EXECUTE_READWRITE, &OldProtect);

    *(ULONG_PTR *)(MyHookCode - 8) = (ULONG_PTR)inject;

    SBIELOW_CALL(NtProtectVirtualMemory)(
        NtCurrentProcess(), &RegionBase, &RegionSize,
        OldProtect, &OldProtect);

    RegionBase = (void *)&LdrCode[0]; // RtlFindActCtx
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

    MyHookCode = (UCHAR *)DetourCode;
    RegionBase = (void *)(MyHookCode + 1);
    RegionSize = sizeof(ULONG_PTR);

    SBIELOW_CALL(NtProtectVirtualMemory)(
        NtCurrentProcess(), &RegionBase, &RegionSize,
        PAGE_EXECUTE_READWRITE, &OldProtect);

    *(ULONG *)(MyHookCode + 1) = (ULONG)(ULONG_PTR)inject;

    SBIELOW_CALL(NtProtectVirtualMemory)(
        NtCurrentProcess(), &RegionBase, &RegionSize,
        OldProtect, &OldProtect);

    RegionBase = (void *)LdrCode; // RtlFindActCtx
    RegionSize = 5;

    SBIELOW_CALL(NtProtectVirtualMemory)(
        NtCurrentProcess(), &RegionBase, &RegionSize,
        PAGE_EXECUTE_READWRITE, &inject->RtlFindActCtx_Protect);

    memcpy(&inject->RtlFindActCtx_Bytes, LdrCode, 5);

    LdrCode[0] = 0xE9;
    *(ULONG *)&LdrCode[1] = (ULONG)(MyHookCode - (LdrCode + 5));

#endif
}


//---------------------------------------------------------------------------
// InitInjectWow64
//---------------------------------------------------------------------------


#ifdef _WIN64
_FX void InitInjectWow64(SBIELOW_DATA *data)
{
    SYSCALL_DATA* syscall_data;
    INJECT_DATA *inject;
    SBIELOW_EXTRA_DATA *extra;
    UCHAR *LdrCode, *MyCode;
    void *RegionBase;
    SIZE_T RegionSize;

    //
    // find inject and extra data areas, same as in InitInject()
    //

    syscall_data = (SYSCALL_DATA *)data->syscall_data;

    extra = (SBIELOW_EXTRA_DATA *) (data->syscall_data + syscall_data->extra_data_offset);

    inject = (INJECT_DATA *) ((UCHAR *)extra + extra->InjectData_offset);

    //
    // prepare unicode strings
    //

    inject->SbieDll_Length = (SHORT)extra->Wow64SbieDll_length;
    inject->SbieDll_MaxLen = inject->SbieDll_Length + sizeof(WCHAR);
    inject->SbieDll_Buf32  =
                    (ULONG)((ULONG_PTR)extra + extra->Wow64SbieDll_offset);

    //
    // the service fills INJECT_DATA.DetourCode_x86 with the right non native code
    //

    MyCode = inject->DetourCode_x86;

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
