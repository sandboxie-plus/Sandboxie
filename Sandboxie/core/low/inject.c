/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2023 David Xanatos, xanasoft.com
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
// inject
//---------------------------------------------------------------------------

#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

#include <windows.h>
#include "common/win32_ntddk.h"
#include "common/defines.h"
#include "lowdata.h"
#include "core/drv/api_defs.h"

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

_FX NTSTATUS SbieApi_LogMsg(ULONG64 pNtDeviceIoControlFile, ULONG64 api_device_handle, ULONG code, const WCHAR* text);
_FX NTSTATUS SbieApi_DebugError(SBIELOW_DATA* data, ULONG error);

UCHAR *FindDllExport(void *DllBase, const UCHAR *ProcName, ULONG *pErr);

#ifdef _M_ARM64
void* Hook_GetFFSTarget(UCHAR* SourceFunc);
#endif

//static void InitInjectWow64(SBIELOW_DATA *data);

//---------------------------------------------------------------------------

typedef NTSTATUS(*P_LdrLoadDll)(
    WCHAR *PathString, ULONG *DllFlags,
    UNICODE_STRING *ModuleName, HANDLE *ModuleHandle);

typedef NTSTATUS (*P_LdrGetProcedureAddress)(
    HANDLE ModuleHandle, ANSI_STRING *ProcName, ULONG ProcNum,
    ULONG_PTR *Address);

typedef NTSTATUS (*P_NtProtectVirtualMemory)(
    HANDLE ProcessHandle, PVOID *BaseAddress,
    PSIZE_T RegionSize, ULONG NewProtect, PULONG OldProtect);

typedef NTSTATUS (*P_NtRaiseHardError)(
    NTSTATUS ErrorStatus, ULONG NumberOfParameters, ULONG UnicodeBitMask,
    ULONG_PTR *Parameters, ULONG ErrorOption, ULONG *ErrorReturn);

//---------------------------------------------------------------------------

#define SBIELOW_CALL(x) ((P_##x)&data->x##_code)


//---------------------------------------------------------------------------
// DetourFunc
//---------------------------------------------------------------------------


ULONG_PTR DetourFunc(INJECT_DATA *inject) 
{
    //
    // Note: this function is invoked from the detour code, hence when running in WoW64,
    // the used instance of this function will be from the 32 bit version,
    // in which case we are unable to use SBIELOW_CALL and need to have a
    // pointer to the appropriate 32 bit function
    // 
    // Furthermore, on ARM64 the SBIELOW_DATA will be allocated past the 4 GB boundary 
    // hence in 32 bit mode we can not access it, only INJECT_DATA is available
    //

    NTSTATUS status;
    UNICODE_STRING* pDllPath;
    HANDLE ModuleHandle;
    typedef VOID(*P_Dll_Ordinal1)(INJECT_DATA* inject);
    P_Dll_Ordinal1 SbieDllOrdinal1;
    void *RegionBase;
    SIZE_T RegionSize;
    ULONG OldProtect;

#ifdef _WIN64
    SBIELOW_DATA* data = (SBIELOW_DATA*)inject->sbielow_data;
#endif

    //
    // restore original function
    //

    RegionBase = (void*)inject->RtlFindActCtx;
#ifdef _WIN64
#ifdef _M_ARM64
    RegionSize = 16;
    memcpy((void*)inject->RtlFindActCtx, inject->RtlFindActCtx_Bytes, 16);

    SBIELOW_CALL(NtFlushInstructionCache)(
        NtCurrentProcess(), (void*)inject->RtlFindActCtx, 16);
#else
    RegionSize = 12;
    memcpy((void*)inject->RtlFindActCtx, inject->RtlFindActCtx_Bytes, 12);
#endif

    SBIELOW_CALL(NtProtectVirtualMemory)(
        NtCurrentProcess(), &RegionBase, &RegionSize,
        inject->RtlFindActCtx_Protect, &OldProtect);
#else
    RegionSize = 5;
    memcpy((void*)inject->RtlFindActCtx, inject->RtlFindActCtx_Bytes, 5);

    ((P_NtProtectVirtualMemory)inject->NtProtectVirtualMemory)(
        NtCurrentProcess(), &RegionBase, &RegionSize,
        inject->RtlFindActCtx_Protect, &OldProtect);
#endif

    //
    // load kernel32.dll
    //

    pDllPath = (UNICODE_STRING*)&inject->KernelDll;

    status = ((P_LdrLoadDll)inject->LdrLoadDll)(NULL, 0, pDllPath, &ModuleHandle);
    
    //
    // load sbiedll.dll
    //

    if (status == 0) {

        pDllPath = (UNICODE_STRING*)&inject->SbieDll;

        status = ((P_LdrLoadDll)inject->LdrLoadDll)(NULL, 0, pDllPath, &ModuleHandle);
    }

    //
    // get ordinal 1 from sbiedll
    //

    if (status == 0) {

        status = ((P_LdrGetProcedureAddress)inject->LdrGetProcAddr)(ModuleHandle, NULL, 1, (ULONG_PTR*)&SbieDllOrdinal1);
#ifdef _M_ARM64
        //
        // on ARM64EC we hook the native code, hence we need to obtain the address of the native ordinal 1 from our SbieDll.dll
        // instead of the FFS sequence as given by NtGetProcedureAddress when in ARM64EC mode
        //

        if (data->flags.is_arm64ec && status >= 0) {
            SbieDllOrdinal1 = (P_Dll_Ordinal1)Hook_GetFFSTarget((UCHAR*)SbieDllOrdinal1);
            //if (!SbieDllOrdinal1)
            //    status = STATUS_ENTRYPOINT_NOT_FOUND;
        }
#endif
    }

    //
    // call ordinal 1 of sbiedll.dll
    //

    if (status == 0) {

        SbieDllOrdinal1(inject);
    }

    //
    // or report error if one occurred instead
    //

    else {

        wchar_t text[] = { 0 };
        SbieApi_LogMsg(inject->NtDeviceIoControlFile, inject->api_device_handle, 2181, text);

        status = 0xC0000142; // = STATUS_DLL_INIT_FAILED
        ULONG_PTR Parameters[1] = { (ULONG_PTR)pDllPath };
        ULONG ErrorReturn;
        ((P_NtRaiseHardError)inject->NtRaiseHardError)(
            status | 0x10000000, // | FORCE_ERROR_MESSAGE_BOX
            1, 1, Parameters, 1, &ErrorReturn);
    }

    return status;
}


//---------------------------------------------------------------------------
// InitInject
//---------------------------------------------------------------------------


_FX void InitInject(SBIELOW_DATA *data, void *DetourCode)
{
    void *ntdll_base;
    SYSCALL_DATA* syscall_data;
    INJECT_DATA *inject;
    SBIELOW_EXTRA_DATA *extra;
    UCHAR *HookTarget, *HookCode;
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
        // (Prior to Windows 8, the base address of ntdll32 is recorded
        // in offset 0x036C of the KUSER_SHARED_DATA structure, which always
        // has a fixed base address of 0x7FFE0000. This is not available
        // in Windows 8, so we have to rely on the driver to track this
        // base address via Process_NotifyImage in core/drv/process.c.
        // So we might as well use this approach for all 64-bit Windows.)
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

    inject->LdrLoadDll = (ULONG_PTR)FindDllExport(ntdll_base,
                            (UCHAR *)extra + extra->LdrLoadDll_offset, &uError);
#ifdef _M_ARM64
    if (inject->LdrLoadDll && data->flags.is_arm64ec)
        inject->LdrLoadDll = (ULONG_PTR)Hook_GetFFSTarget((UCHAR*)inject->LdrLoadDll);
#endif
    if (!inject->LdrLoadDll) {
        SbieApi_DebugError(data, (0x01 << 4) | uError);
        return;
    }

    inject->LdrGetProcAddr = (ULONG_PTR)FindDllExport(ntdll_base,
                                (UCHAR *)extra + extra->LdrGetProcAddr_offset, &uError);
#ifdef _M_ARM64
    if (inject->LdrGetProcAddr && data->flags.is_arm64ec)
        inject->LdrGetProcAddr = (ULONG_PTR)Hook_GetFFSTarget((UCHAR*)inject->LdrGetProcAddr);
#endif
    if (!inject->LdrGetProcAddr) {
        SbieApi_DebugError(data, (0x02 << 4) | uError);
        return;
    }

#ifdef _WIN64
    if (data->flags.is_wow64) {

        inject->NtProtectVirtualMemory = (ULONG_PTR)FindDllExport(ntdll_base,
                                                (UCHAR*)extra + extra->NtProtectVirtualMemory_offset, &uError);
        if (!inject->NtProtectVirtualMemory) {
            SbieApi_DebugError(data, (0x03 << 4) | uError);
            return;
        }

        inject->NtRaiseHardError = (ULONG_PTR)FindDllExport(ntdll_base,
                                                (UCHAR*)extra + extra->NtRaiseHardError_offset, &uError);
        if (!inject->NtRaiseHardError) {
            SbieApi_DebugError(data, (0x04 << 4) | uError);
            return;
        }

        inject->NtDeviceIoControlFile = (ULONG_PTR)FindDllExport(ntdll_base,
                                                (UCHAR*)extra + extra->NtDeviceIoControlFile_offset, &uError);
        if (!inject->NtDeviceIoControlFile) {
            SbieApi_DebugError(data, (0x05 << 4) | uError);
            return;
        }
    }
    else
#endif
    {
        //
        // for ARM64EC we need native functions, FindDllExport can manage FFS
        // however this does not work for syscalls, hence we use the native function directly
        //

        inject->NtProtectVirtualMemory = data->NativeNtProtectVirtualMemory;
        inject->NtRaiseHardError = data->NativeNtRaiseHardError;
        inject->NtDeviceIoControlFile = data->NtDeviceIoControlFile;
    }
    inject->api_device_handle = data->api_device_handle;

#ifdef _M_ARM64

    //
    // when hooking on arm64, go for LdrLoadDll 
    // instead of RtlFindActivationContextSectionString
    // for ARM64 both work, but for ARM64EC hooking RtlFindActCtx fails
    //

    if (!data->flags.is_wow64)
        HookTarget = (UCHAR*)inject->LdrLoadDll;
    else
#endif
    {
        HookTarget = FindDllExport(ntdll_base,
                        (UCHAR *)extra + extra->RtlFindActCtx_offset, &uError);
        if (!HookTarget) {
            SbieApi_DebugError(data, (0x05 << 4) | uError);
            return;
        }
    }
    inject->RtlFindActCtx = (ULONG_PTR)HookTarget;


    //
    // prepare unicode strings
    //

    inject->KernelDll.Length = (USHORT)extra->KernelDll_length;
    inject->KernelDll.MaxLen = inject->KernelDll.Length + sizeof(WCHAR);
    inject->KernelDll.Buf32   =
                    (ULONG)((ULONG_PTR)extra + extra->KernelDll_offset);
#ifdef _WIN64
    inject->KernelDll.Buf64   =
                  (ULONG64)((ULONG_PTR)extra + extra->KernelDll_offset);
#endif

    //
    // select the right version of SbieDll.dll
    //

#ifdef _M_ARM64
    if (data->flags.is_arm64ec) {

        inject->SbieDll.Length = (SHORT)extra->Arm64ecSbieDll_length;
        inject->SbieDll.MaxLen = inject->SbieDll.Length + sizeof(WCHAR);
        inject->SbieDll.Buf64 =
            (ULONG64)((ULONG_PTR)extra + extra->Arm64ecSbieDll_offset);
    }
    else
#endif
#ifdef _WIN64
    if (data->flags.is_wow64) 
    {
        inject->SbieDll.Length = (SHORT)extra->Wow64SbieDll_length;
        inject->SbieDll.MaxLen = inject->SbieDll.Length + sizeof(WCHAR);
        inject->SbieDll.Buf32  =
                        (ULONG)((ULONG_PTR)extra + extra->Wow64SbieDll_offset);
    }
    else
#endif
    {
        inject->SbieDll.Length = (SHORT)extra->NativeSbieDll_length;
        inject->SbieDll.MaxLen = inject->SbieDll.Length + sizeof(WCHAR);
#ifdef _WIN64
        inject->SbieDll.Buf64 =
            (ULONG64)((ULONG_PTR)extra + extra->NativeSbieDll_offset);
#else
        inject->SbieDll.Buf32  =
            (ULONG)((ULONG_PTR)extra + extra->NativeSbieDll_offset);
#endif
    }


    //
    // modify our detour code in entry.asm to include a hard coded pointer to the inject data area.
    //
    
#ifdef _WIN64
    if (!data->flags.is_wow64) {

#ifdef _M_ARM64

        HookCode = (UCHAR*)DetourCode;
        RegionBase = (void*)(HookCode - 8);
        RegionSize = sizeof(ULONG_PTR);
        SBIELOW_CALL(NtProtectVirtualMemory)(
            NtCurrentProcess(), &RegionBase, &RegionSize,
            PAGE_EXECUTE_READWRITE, &OldProtect);

        *(ULONG_PTR*)(HookCode - 8) = (ULONG_PTR)inject;

        SBIELOW_CALL(NtProtectVirtualMemory)(
            NtCurrentProcess(), &RegionBase, &RegionSize,
            OldProtect, &OldProtect);

        RegionBase = (void*)&HookTarget[0]; // RtlFindActCtx
        RegionSize = 16;
        SBIELOW_CALL(NtProtectVirtualMemory)(
            NtCurrentProcess(), &RegionBase, &RegionSize,
            PAGE_EXECUTE_READWRITE, &inject->RtlFindActCtx_Protect);
        memcpy(inject->RtlFindActCtx_Bytes, HookTarget, 16);

        ULONG* aCode = (ULONG*)HookTarget;
        *aCode++ = 0x58000048;	// ldr x8, 8
        *aCode++ = 0xD61F0100;	// br x8
        *(DWORD64*)aCode = (DWORD64)HookCode;

        SBIELOW_CALL(NtFlushInstructionCache)(
            NtCurrentProcess(), RegionBase, (ULONG)RegionSize);

#else

        HookCode = (UCHAR*)DetourCode;
        RegionBase = (void*)(HookCode - 8);
        RegionSize = sizeof(ULONG_PTR);
        SBIELOW_CALL(NtProtectVirtualMemory)(
            NtCurrentProcess(), &RegionBase, &RegionSize,
            PAGE_EXECUTE_READWRITE, &OldProtect);

        *(ULONG_PTR*)(HookCode - 8) = (ULONG_PTR)inject;

        SBIELOW_CALL(NtProtectVirtualMemory)(
            NtCurrentProcess(), &RegionBase, &RegionSize,
            OldProtect, &OldProtect);

        RegionBase = (void*)&HookTarget[0]; // RtlFindActCtx
        RegionSize = 12;
        SBIELOW_CALL(NtProtectVirtualMemory)(
            NtCurrentProcess(), &RegionBase, &RegionSize,
            PAGE_EXECUTE_READWRITE, &inject->RtlFindActCtx_Protect);
        memcpy(inject->RtlFindActCtx_Bytes, HookTarget, 12);

        HookTarget[0] = 0x48;
        HookTarget[1] = 0xb8;
        *(ULONG_PTR*)&HookTarget[2] = (ULONG_PTR)HookCode;
        HookTarget[10] = 0xff;
        HookTarget[11] = 0xe0;

#endif

    }
    else
    {
        HookCode = (UCHAR*)data->ptr_32bit_detour;
#else
    {
        HookCode = (UCHAR*)DetourCode;
#endif
        RegionBase = (void*)(HookCode + 1);
        RegionSize = sizeof(ULONG_PTR);

        SBIELOW_CALL(NtProtectVirtualMemory)(
            NtCurrentProcess(), &RegionBase, &RegionSize,
            PAGE_EXECUTE_READWRITE, &OldProtect);

        *(ULONG*)(HookCode + 1) = (ULONG)(ULONG_PTR)inject;

        SBIELOW_CALL(NtProtectVirtualMemory)(
            NtCurrentProcess(), &RegionBase, &RegionSize,
            OldProtect, &OldProtect);

        RegionBase = (void*)HookTarget; // RtlFindActCtx
        RegionSize = 5;

        SBIELOW_CALL(NtProtectVirtualMemory)(
            NtCurrentProcess(), &RegionBase, &RegionSize,
            PAGE_EXECUTE_READWRITE, &inject->RtlFindActCtx_Protect);

        memcpy(inject->RtlFindActCtx_Bytes, HookTarget, 5);

        HookTarget[0] = 0xE9;
        *(ULONG*)&HookTarget[1] = (ULONG)(HookCode - (HookTarget + 5));
    }
}
