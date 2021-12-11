/*
 * Copyright 2020-2021 David Xanatos, xanasoft.com
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


#include "dll.h"

#define HOOK_WIN32K

#ifdef HOOK_WIN32K

#include "core/drv/api_defs.h"
#include "core/low/lowdata.h"

SBIELOW_DATA* SbieApi_data = NULL;
#define SBIELOW_CALL(x) ((P_##x)&data->x##_code)

_FX BOOLEAN SbieDll_HookWin32SysCalls(HMODULE win32u_base)
{
    UCHAR *SystemServiceAsm, *ZwXxxPtr;
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


    while (SyscallPtr[0] || SyscallPtr[1]) {

        //
        // the ULONG at SyscallPtr[1] gives the offset of the ZwXxx export
        // from the base address of ntdll
        //

        ZwXxxPtr = (UCHAR *)((ULONG_PTR)SyscallPtr[1] + (UCHAR*)win32u_base);

        //
        // make the syscall address writable
        //
        RegionBase = ZwXxxPtr;

#ifdef _WIN64
        RegionSize = 12;
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

#ifdef _WIN64
        SyscallNum &= 0xFFFF;   // clear the not needed param count

        ZwXxxPtr[0] = 0x49;                 // mov r10, SyscallNumber
        ZwXxxPtr[1] = 0xC7;
        ZwXxxPtr[2] = 0xC2;
        *(ULONG *)&ZwXxxPtr[3] = SyscallNum;
        if (!data->flags.long_diff) {

            if (data->flags.is_win10) {
                ZwXxxPtr[7] = 0x48;             // jmp SystemServiceAsm
                ZwXxxPtr[8] = 0xE9;             // jmp SystemServiceAsm
                *(ULONG *)&ZwXxxPtr[9] = (ULONG)(ULONG_PTR)(SystemServiceAsm - (ZwXxxPtr + 13));
            }
            else {
                ZwXxxPtr[7] = 0xe9;             // jmp SystemServiceAsm
                *(ULONG *)&ZwXxxPtr[8] = (ULONG)(ULONG_PTR)(SystemServiceAsm - (ZwXxxPtr + 12));
            }
        }
        else {

            ZwXxxPtr[7] = 0xB8;             // mov eax, SystemServiceAsm
            *(ULONG *)&ZwXxxPtr[8] = (ULONG)(ULONG_PTR)SystemServiceAsm;
            *(USHORT *)&ZwXxxPtr[12] = 0xE0FF;  // jmp rax
        }
        
#else ! _WIN64

        ZwXxxPtr[0] = 0xB8;                 // mov eax, SyscallNumber
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

    HeapFree(GetProcessHeap(), 0, syscall_data);
    return TRUE;
}

#ifndef _WIN64
_FX NTSTATUS SbieDll_WoW64SysCall(ULONG syscall, ULONG* args)
{
	extern HANDLE SbieApi_DeviceHandle;

    ULONG argc = (syscall >> 24);
    syscall &= 0xFFFF;

	ULONG64 stack[19];
    for (ULONG i = 0; i < argc; i++)
        stack[i] = args[i];

	NTSTATUS status;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];

    memset(parms, 0, sizeof(parms));
    parms[0] = API_INVOKE_SYSCALL;
	parms[1] = (ULONG64)(ULONG_PTR)syscall;
    parms[2] = (ULONG64)(ULONG_PTR)stack; // pointer to system service arguments on stack

	IO_STATUS_BLOCK MyIoStatusBlock;
	status = NtDeviceIoControlFile(
        SbieApi_DeviceHandle, NULL, NULL, NULL, &MyIoStatusBlock,
        API_SBIEDRV_CTLCODE, parms, sizeof(ULONG64) * 8, NULL, 0);

	return status;
}

#pragma runtime_checks( "", off )
_FX NTSTATUS SbieDll_WoW64SysCallProc(ULONG arg0)
{
    //push        ebp  
    //mov         ebp,esp  
    //sub         esp,40h
    //push        ebx  
    //push        esi  
    //push        edi  
    __asm
    {
         mov         ecx,eax  // get the syscall number
         //return SbieDll_WoW64SysCall(syscall, 19, &arg0);
         lea         eax,[ebp+8]   // get address of arg0
         push        eax  // args
         push        ecx  // syscall
         call        SbieDll_WoW64SysCall
    }
    //pop         edi  
    //pop         esi  
    //pop         ebx  
    //mov         esp,ebp  
    //pop         ebp  
    //ret         4 
}
#pragma runtime_checks( "", restore )

_FX BOOLEAN SbieDll_HookWoW64SysCalls(HMODULE win32u_base)
{
    UCHAR *SystemServiceAsm, *ZwXxxPtr;
    ULONG *SyscallPtr;
    ULONG SyscallNum;
    char FuncName[68] = "Nt";
    void *RegionBase;
    SIZE_T RegionSize;
    ULONG OldProtect;
    
    SystemServiceAsm = (UCHAR*)SbieDll_WoW64SysCallProc;

	UCHAR* syscall_data = (UCHAR *)HeapAlloc(GetProcessHeap(), 0, 144000); // enough room for 2000 syscalls with names
	if (!syscall_data)
		return FALSE;

    NTSTATUS status = SbieApi_Call(API_QUERY_SYSCALLS, 3, (ULONG_PTR)syscall_data, 1, 1);

    if (!NT_SUCCESS(status)) {
        HeapFree(GetProcessHeap(), 0, syscall_data);
        return FALSE;
    }

    SyscallPtr = (ULONG *)(syscall_data
                         + sizeof(ULONG));         // size of buffer

    while (SyscallPtr[0] || SyscallPtr[1]) {

        SyscallNum = SyscallPtr[0] | 0x1000;
        strcpy(FuncName + 2, (char*)&SyscallPtr[2]);
        ZwXxxPtr = (UCHAR*)GetProcAddress(win32u_base, FuncName);
        if (!ZwXxxPtr)
            return FALSE;


        RegionBase = ZwXxxPtr;
        RegionSize = 10;

        NtProtectVirtualMemory(
            NtCurrentProcess(), &RegionBase, &RegionSize,
            PAGE_EXECUTE_READWRITE, &OldProtect);

        ZwXxxPtr[0] = 0xB8;                 // mov eax, SyscallNumber
        *(ULONG *)&ZwXxxPtr[1] = SyscallNum;
        ZwXxxPtr[5] = 0xE9;                 // jmp SystemServiceAsm
        *(ULONG *)&ZwXxxPtr[6] =
            (ULONG)(ULONG_PTR)(SystemServiceAsm - (ZwXxxPtr + 10));

        NtProtectVirtualMemory(
            NtCurrentProcess(), &RegionBase, &RegionSize,
            OldProtect, &OldProtect);
        

        SyscallPtr += 2 + 16;
    }

    HeapFree(GetProcessHeap(), 0, syscall_data);
    return TRUE;
}
#endif

#endif

_FX BOOLEAN Win32_Init(HMODULE hmodule)
{
#ifdef HOOK_WIN32K
	// In Windows 10 all Win32k.sys calls are located in win32u.dll
    if (Dll_OsBuild < 10041 || !SbieApi_QueryConfBool(NULL, L"EnableWin32kHooks", FALSE))
        return TRUE; // just return on older builds

    // NoSysCallHooks BEGIN
    if ((Dll_ProcessFlags & SBIE_FLAG_APP_COMPARTMENT) != 0 || SbieApi_QueryConfBool(NULL, L"NoSysCallHooks", FALSE))
        return TRUE;
    // NoSysCallHooks END

    // disable Electron Workaround when we are ready to hook the required win32k syscalls
    extern BOOL Dll_ElectronWorkaround;
    Dll_ElectronWorkaround = FALSE; 

    //
    // chrome needs for a working GPU acceleration the GdiDdDDI* win32k syscalls to have the right user token
    //

    WCHAR* cmdline = GetCommandLine();

    if ((wcsstr(cmdline, L"--type=gpu-process") != NULL && wcsstr(cmdline, L"--gpu-preferences=") != NULL)
     || SbieApi_QueryConfBool(NULL, L"AlwaysUseWin32kHooks", FALSE)) {

#ifndef _WIN64
        if (Dll_IsWow64) 
            SbieDll_HookWoW64SysCalls(hmodule);
        else 
#endif
            SbieDll_HookWin32SysCalls(hmodule);
    }

#endif

	return TRUE;
}
