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


#define NOGDI
#include "dll.h"

#include "common\pattern.h"

#include "core/drv/api_defs.h"
#include "core/low/lowdata.h"

extern SBIELOW_DATA* SbieApi_data;
#define SBIELOW_CALL(x) ((P_##x)&data->x##_code)


//---------------------------------------------------------------------------
// SbieDll_HookWin32SysCalls
//---------------------------------------------------------------------------


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
        RegionSize = 14;
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

        ZwXxxPtr[0] = 0x49;                     // mov r10, SyscallNumber
        ZwXxxPtr[1] = 0xC7;
        ZwXxxPtr[2] = 0xC2;
        *(ULONG *)&ZwXxxPtr[3] = SyscallNum;
        if (!data->flags.long_diff) {

            if (data->flags.is_win10) {
                ZwXxxPtr[7] = 0x48;             // jmp SystemServiceAsm
                ZwXxxPtr[8] = 0xE9;
                *(ULONG *)&ZwXxxPtr[9] = (ULONG)(ULONG_PTR)(SystemServiceAsm - (ZwXxxPtr + 13));
            }
            else {
                ZwXxxPtr[7] = 0xe9;             // jmp SystemServiceAsm
                *(ULONG *)&ZwXxxPtr[8] = (ULONG)(ULONG_PTR)(SystemServiceAsm - (ZwXxxPtr + 12));
            }
        }
        else {

            ZwXxxPtr[7] = 0xB8;                 // mov eax, SystemServiceAsm
            *(ULONG *)&ZwXxxPtr[8] = (ULONG)(ULONG_PTR)SystemServiceAsm;
            *(USHORT *)&ZwXxxPtr[12] = 0xE0FF;  // jmp rax
        }
        
#else ! _WIN64

        ZwXxxPtr[0] = 0xB8;                 // mov eax, SyscallNumber, with param count in the highest byte
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


//---------------------------------------------------------------------------
// Win32_WoW64_GetSysCallNumber
//---------------------------------------------------------------------------

#ifndef _WIN64
ULONG Win32_WoW64_GetSysCallNumber(DWORD64 pos, UCHAR* dll_data)
{
    // 4C 8B D1 - r10,rcx
    if (!(dll_data[pos + 0] == 0x4c && dll_data[pos + 1] == 0x8b && dll_data[pos + 2] == 0xd1))
        return 0;

    // B8 XX XX XX XX - mov eax,0xXXXX
    if (!(dll_data[pos + 3] == 0xb8))
        return 0;
    ULONG syscall_index = *(ULONG*)&dll_data[pos + 4];

    // F6 04 25 08 03 FE 7F 01 - test BYTE PTR ds:0x7ffe0308,0x1
    //

    // 75 03 - jne 0x15 (label_1)
    if (!(dll_data[pos + 16] == 0x75 && dll_data[pos + 17] == 0x03))
        return 0;

    // 0F 05 - syscall
    if (!(dll_data[pos + 18] == 0x0f && dll_data[pos + 19] == 0x05))
        return 0;
    // C3 - ret
    if (!(dll_data[pos + 20] == 0xc3))
        return 0;

    // label_1:
    // CD 2E - int 0x2e
    if (!(dll_data[pos + 21] == 0xcd && dll_data[pos + 22] == 0x2e))
        return 0;
    // C3 - ret
    if (!(dll_data[pos + 23] == 0xc3))
        return 0;

    //

    return syscall_index;
}


//---------------------------------------------------------------------------
// SbieDll_HasSysCallHook
//---------------------------------------------------------------------------


_FX BOOLEAN SbieDll_HasSysCallHook(UCHAR* syscall_data, ULONG syscall_index)
{
    ULONG *SyscallPtr;
    ULONG SyscallNum;

    SyscallPtr = (ULONG *)(syscall_data + sizeof(ULONG)); // size of buffer

    while (SyscallPtr[0] || SyscallPtr[1]) {
            
        SyscallNum = SyscallPtr[0];
            
        SyscallNum &= 0xFFFF; // clear the not needed param count
            
        if (SyscallNum == syscall_index)
            return TRUE;

        SyscallPtr += 2;
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// SbieDll_HookWin32WoW64
//---------------------------------------------------------------------------

//#include "../../common/wow64ext/wow64ext.h"
DWORD64 __cdecl X64Call(DWORD64 func, int argC, ...);
DWORD64 __cdecl GetModuleHandle64(const wchar_t* lpModuleName);
BOOL __cdecl VirtualProtectEx64(HANDLE hProcess, DWORD64 lpAddress, SIZE_T dwSize, DWORD flNewProtect, DWORD* lpflOldProtect);
BOOL __cdecl ReadProcessMemory64(HANDLE hProcess, DWORD64 lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, SIZE_T *lpNumberOfBytesRead);
BOOL __cdecl WriteProcessMemory64(HANDLE hProcess, DWORD64 lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, SIZE_T *lpNumberOfBytesWritten);

_FX BOOLEAN SbieDll_HookWin32WoW64()
{
    BOOLEAN ok = FALSE;
    UCHAR* dll_data = NULL;
    UCHAR* syscall_data = NULL;

    DWORD64 BaseAddress = GetModuleHandle64(L"wow64win.dll");
    SIZE_T SizeOfImage = 0x00100000; // 1 MB should be more than enough
    if (!BaseAddress) {
        SbieApi_Log(2303, L"win32k, wow64win.dll base not found");
        return FALSE;
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE,  GetCurrentProcessId());
    if (hProcess == INVALID_HANDLE_VALUE) {
        SbieApi_Log(2303, L"win32k, can't open process");
        return FALSE;
    }

    dll_data = (UCHAR *)HeapAlloc(GetProcessHeap(), 0, (SIZE_T)SizeOfImage);
    if (!dll_data) {
        SbieApi_Log(2303, L"win32k, alloc failed (1)");
        goto finish;
    }

    SIZE_T SizeRead;
    if (!ReadProcessMemory64(hProcess, (PVOID64)BaseAddress, dll_data, SizeOfImage, &SizeRead)) {
        SbieApi_Log(2303, L"win32k, wow64win.dll read failed");
        goto finish;
    }

    DWORD64 FuncTable = 0;

    for (DWORD64 pos = 0; pos < SizeRead-0x20; pos++) {
        if (Win32_WoW64_GetSysCallNumber(pos, dll_data) != 0){
            FuncTable = pos;
            break;
        }
    }
    
    if (FuncTable == 0) {
        SbieApi_Log(2303, L"win32k, wow64win.dll sys call table not found");
        goto finish;
    }

    DWORD64 SystemServiceAsm;
    UCHAR ZwXxxPtr[16];
    ULONG SyscallNum;
    DWORD64 RegionBase;
    SIZE_T RegionSize;
    ULONG OldProtect;
    
    SBIELOW_DATA* data = SbieApi_data;
    SystemServiceAsm = data->pSystemService;

	syscall_data = (UCHAR *)HeapAlloc(GetProcessHeap(), 0, 16384); // enough room for over 2000 syscalls
    if (!syscall_data) {
        SbieApi_Log(2303, L"win32k, alloc failed (2)");
        goto finish;
    }

    if (!NT_SUCCESS(SbieApi_Call(API_QUERY_SYSCALLS, 2, (ULONG_PTR)syscall_data, 1))) {
        SbieApi_Log(2303, L"win32k, syscall query failed");
        goto finish;
    }

    for (DWORD64 pos = FuncTable; pos < SizeRead - 0x20; ) 
    {
        SyscallNum = Win32_WoW64_GetSysCallNumber(pos, dll_data);
        if (SyscallNum) 
        {
            if(SbieDll_HasSysCallHook(syscall_data, SyscallNum))
            {
                RegionBase = BaseAddress + pos;
                RegionSize = 14;
                
                //
                // prepare call to call our SystemServiceAsm
                //

                ZwXxxPtr[0] = 0x49;                     // mov r10, SyscallNumber
                ZwXxxPtr[1] = 0xC7;
                ZwXxxPtr[2] = 0xC2;
                *(ULONG *)&ZwXxxPtr[3] = SyscallNum;
                if (!data->flags.long_diff) {

                    if (data->flags.is_win10) {
                        ZwXxxPtr[7] = 0x48;             // jmp SystemServiceAsm
                        ZwXxxPtr[8] = 0xE9;
                        *(ULONG *)&ZwXxxPtr[9] = (ULONG)(ULONG_PTR)(SystemServiceAsm - (RegionBase + 13));
                    }
                    else {
                        ZwXxxPtr[7] = 0xe9;             // jmp SystemServiceAsm
                        *(ULONG *)&ZwXxxPtr[8] = (ULONG)(ULONG_PTR)(SystemServiceAsm - (RegionBase + 12));
                    }
                }
                else {

                    ZwXxxPtr[7] = 0xB8;                 // mov eax, SystemServiceAsm
                    *(ULONG *)&ZwXxxPtr[8] = (ULONG)(ULONG_PTR)SystemServiceAsm;
                    *(USHORT *)&ZwXxxPtr[12] = 0xE0FF;  // jmp rax
                }
                

                //
                // overwrite the ZwXxx export to call our SystemServiceAsm,
                // and then restore the original page protection
                //

                if (!VirtualProtectEx64(hProcess, RegionBase, RegionSize, PAGE_EXECUTE_READWRITE, &OldProtect)) {
                    SbieApi_Log(2303, L"win32k %d (1)", SyscallNum);
                    goto finish;
                }

                if (!WriteProcessMemory64(hProcess, RegionBase, ZwXxxPtr, RegionSize, &SizeRead)) {
                    SbieApi_Log(2303, L"win32k %d (2)", SyscallNum);
                    goto finish;
                }

                if (!VirtualProtectEx64(hProcess, RegionBase, RegionSize, OldProtect, &OldProtect)) {
                    SbieApi_Log(2303, L"win32k %d (3)", SyscallNum);
                    goto finish;
                }
            }

            pos += 0x20;
        }
        else if (*((ULONG*)&dll_data[pos]) == 0xCCCCCCCC) { // int 3; int 3; int 3; int 3;
            ok = TRUE;
            break; // end of sys call function table
        }
        else
            pos++;
    }

finish:
    
    if(syscall_data)
        HeapFree(GetProcessHeap(), 0, syscall_data);

    if(dll_data)
        HeapFree(GetProcessHeap(), 0, dll_data);

    CloseHandle(hProcess);

    return ok;
}
#endif

//---------------------------------------------------------------------------
// Win32_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Win32_Init(HMODULE hmodule)
{
	// In Windows 10 all Win32k.sys calls are located in win32u.dll
    if (Dll_OsBuild < 10041 || (Dll_ProcessFlags & SBIE_FLAG_WIN32K_HOOKABLE) == 0 || !SbieApi_QueryConfBool(NULL, L"EnableWin32kHooks", TRUE))
        return TRUE; // just return on older builds, or not enabled

    if ((Dll_ProcessFlags & SBIE_FLAG_APP_COMPARTMENT) != 0 || SbieApi_data->flags.bNoSysHooks)
        return TRUE;

    // disable Electron Workaround when we are ready to hook the required win32k syscalls
    extern BOOL Dll_ElectronWorkaround;
    Dll_ElectronWorkaround = FALSE; 

    //
    // chrome needs for a working GPU acceleration the GdiDdDDI* win32k syscalls to have the right user token
    //

    WCHAR* cmdline = GetCommandLine();

    if ((wcsstr(cmdline, L"--type=gpu-process") != NULL && wcsstr(cmdline, L"--gpu-preferences=") != NULL)
     || SbieDll_GetSettingsForName_bool(NULL, Dll_ImageName, L"AlwaysUseWin32kHooks", FALSE)) {

#ifndef _WIN64
        if (Dll_IsWow64) 
            SbieDll_HookWin32WoW64(); // WoW64 hooks
        else 
#endif
            SbieDll_HookWin32SysCalls(hmodule); // Native x86/x64 hooks
    }

	return TRUE;
}