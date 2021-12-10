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

/*NTSTATUS Win32_NtUserCreateWindowEx(
	DWORD dwExStyle,
	PUNICODE_STRING UnsafeClassName,
	LPCWSTR VersionedClass,
	PUNICODE_STRING UnsafeWindowName,
	DWORD dwStyle,
	LONG x,
	LONG y,
	LONG nWidth,
	LONG nHeight,
	HWND hWndParent,
	HMENU hMenu,
	HINSTANCE hInstance,
	LPVOID lpParam,
	DWORD dwShowMode,
	DWORD dwUnknown1,
	DWORD dwUnknown2,
	VOID* qwUnknown3);


typedef NTSTATUS (*P_NtUserCreateWindowEx)(
	DWORD dwExStyle,
	PUNICODE_STRING UnsafeClassName,
	LPCWSTR VersionedClass,
	PUNICODE_STRING UnsafeWindowName,
	DWORD dwStyle,
	LONG x,
	LONG y,
	LONG nWidth,
	LONG nHeight,
	HWND hWndParent,
	HMENU hMenu,
	HINSTANCE hInstance,
	LPVOID lpParam,
	DWORD dwShowMode,
	DWORD dwUnknown1,
	DWORD dwUnknown2,
	VOID* qwUnknown3);

P_NtUserCreateWindowEx		__sys_NtUserCreateWindowEx = NULL;*/

#define HOOK_WIN32K

#ifdef HOOK_WIN32K

#include "core/drv/api_defs.h"
#include "core/low/lowdata.h"

SBIELOW_DATA* SbieApi_data = NULL;
#define SBIELOW_CALL(x) ((P_##x)&data->x##_code)

/*
#ifdef _WIN64

#define MAX_FUNC_SIZE 0x76

//Note any change to this function requires the same modification to the same function in sbiedll: see dllhook.c (findChromeTarget)
ULONGLONG * findChromeTarget(unsigned char* addr)
{
    int i = 0;
    ULONGLONG target = 0;
    ULONG_PTR * ChromeTarget = NULL;
    if (!addr) return NULL;
    //Look for mov rcx,[target 4 byte offset] or in some cases mov rax,[target 4 byte offset]
    //So far the offset has been positive between 0xa00000 and 0xb00000 bytes;
    //This may change in a future version of chrome
    for (i = 0; i < MAX_FUNC_SIZE; i++) {
        if ((*(USHORT *)&addr[i] == 0x8b48)) {
            //Look for mov rcx,[target 4 byte offset] or in some cases mov rax,[target 4 byte offset]
            if ((addr[i + 2] == 0x0d || addr[i + 2] == 0x05)) {
                LONG delta;
                target = (ULONG_PTR)(addr + i + 7);
                delta = *(LONG *)&addr[i + 3];
                //check if offset is close to the expected value (is positive and less than 0x100000 as of chrome 64) 
                //  if (delta > 0 && delta < 0x100000 )  { //may need to check delta in a future version of chrome
                target += delta;
                ChromeTarget = *(ULONGLONG **)target;
                // }
                break;
            }
        }
    }
    return ChromeTarget;
}
#endif
*/

_FX BOOLEAN SbieDll_HookWin32SysCalls(HMODULE win32u_base)
{
    UCHAR *SystemServiceAsm, *ZwXxxPtr;
    ULONG *SyscallPtr;
    ULONG SyscallNum;
    void *RegionBase;
    SIZE_T RegionSize;
    ULONG OldProtect;

    const ULONG OFFSET_ULONG_PTR =
#ifdef _WIN64
    2;  // on 64-bit Windows, "mov rax, 0" instruction is two bytes
    //ULONGLONG *chrome64Target = NULL;
#else
    1;  // on 32-bit Windows, "mov edx, 0" instruction is one byte
#endif
    
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
        // in a chrome sandbox process, some syscalls are already hooked
        // by chrome at this point, and we want to hook the syscall stub
        // saved by chrome, rather than the chrome hook itself (32-bit only)
        //
        /*
#ifndef _WIN64 //if not _WIN64

        if (ZwXxxPtr[0] == 0xB8 &&                  // mov eax,?
            ZwXxxPtr[5] == 0xBA &&                  // mov edx,?
            *(USHORT *)&ZwXxxPtr[10] == 0xE2FF)		// jmp edx
        {
            ULONG i = 0;

            ULONG *longs = *(ULONG **)&ZwXxxPtr[6];
            for (i = 0; i < 20; i++, longs++)
            {
                if (longs[0] == 0x5208EC83 && longs[1] == 0x0C24548B &&
                    longs[2] == 0x08245489 && longs[3] == 0x0C2444C7 &&
                    longs[5] == 0x042444C7)
                {
                    ZwXxxPtr = (UCHAR *)longs[4];
                    break;
                }
            }
        }
#else // _WIN64
        if (ZwXxxPtr[0] == 0x50 &&	//push rax
            ZwXxxPtr[1] == 0x48 &&	//mov rax,?
            ZwXxxPtr[2] == 0xb8) {
            ULONGLONG *longlongs = *(ULONGLONG **)&ZwXxxPtr[3];
            chrome64Target = findChromeTarget((unsigned char *)longlongs);
        }
        // Chrome 49+ 64bit hook
        // mov rax, <target> 
        // jmp rax 
        else if (ZwXxxPtr[0] == 0x48 && //mov rax,<target>
            ZwXxxPtr[1] == 0xb8 &&
            *(USHORT *)&ZwXxxPtr[10] == 0xe0ff) { // jmp rax
            ULONGLONG *longlongs = *(ULONGLONG **)&ZwXxxPtr[2];
            chrome64Target = findChromeTarget((unsigned char *)longlongs);
        }
#endif 
        */

        //
        // make the syscall address writable
        //
        RegionBase = ZwXxxPtr;

#ifdef _WIN64
        RegionSize = 12;
        /*if (!chrome64Target) {
            chrome64Target = (ULONG_PTR*)ZwXxxPtr;
        }*/
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

        SyscallNum = SyscallPtr[0] | 0x1000;

#ifdef _WIN64
        /*if (chrome64Target && data->Sbie64bitJumpTable) {
            RegionSize = 16;
            ZwXxxPtr = (UCHAR *)chrome64Target;
            RegionBase = ZwXxxPtr;

            SBIELOW_CALL(NtProtectVirtualMemory)(
                NtCurrentProcess(), &RegionBase, &RegionSize,
                PAGE_EXECUTE_READWRITE, &OldProtect);

            //unsigned char * jTableTarget = (unsigned char *)&data->Sbie64bitJumpTable->entry[SyscallNum & 0x3ff];
            unsigned char * jTableTarget = (unsigned char *)&data->Sbie64bitJumpTable->entry[SyscallNum];
            // write new patch for jump table
            // The jTable is now injected in the same memory module with lowlevel; no need for a 64 bit long jump
            // mov r10, <4 byte SyscallNum>
            jTableTarget[0] = 0x49;
            jTableTarget[1] = 0xc7;
            jTableTarget[2] = 0xc2;
            *(ULONG *)&jTableTarget[3] = SyscallNum;
            // jmp <4 byte SystemServiceAsm>
            if (data->is_win10) {
                jTableTarget[7] = 0x48;
                jTableTarget[8] = 0xe9;
                *(ULONG *)&jTableTarget[9] = (ULONG)(ULONG_PTR)(SystemServiceAsm - (jTableTarget + 13));
            }
            else {

                jTableTarget[7] = 0xe9;
                *(ULONG *)&jTableTarget[8] = (ULONG)(ULONG_PTR)(SystemServiceAsm - (jTableTarget + 12));

            }
            //  *(ULONGLONG *) &jTableTarget[-8] = 0x9090909090909090; // patch location for sboxdll hook. jtable elements need to be at 0x18 in size for this
            // jump table: using push rcx instead of push rax to differentiate from the chrome sandbox hook
            // the sboxdll.dll needs this distinction to handle the chrome type hook properly and to not search
            // for the chrome target in a sbox 64 bit jtable hook

            // using ret is not compatible with CET - Hardware-enforced Stack Protection
            //ZwXxxPtr[0] = 0x51;     //push rcx
            //// mov rax,<8 byte address to jTableEntry>
            //ZwXxxPtr[1] = 0x48;
            //ZwXxxPtr[2] = 0xb8;
            //*(ULONGLONG *)&ZwXxxPtr[3] = (ULONGLONG)jTableTarget;
            //*(ULONG *)&ZwXxxPtr[11] = 0x24048948;       // mov [rsp],rax
            //ZwXxxPtr[15] = 0xc3;    // ret

            // mov rax,<8 byte address to jTableEntry>
            ZwXxxPtr[0] = 0x90; // start with a nop so that it does not look lile a chrome hook
            ZwXxxPtr[1] = 0x48;
            ZwXxxPtr[2] = 0xb8;
            *(ULONGLONG*)&ZwXxxPtr[3] = (ULONGLONG)jTableTarget;
            // jmp rax
            ZwXxxPtr[11] = 0xFF;
            ZwXxxPtr[12] = 0xE0;

            chrome64Target = NULL;
        }
        else*/
        {
            RegionBase = ZwXxxPtr;

            SBIELOW_CALL(NtProtectVirtualMemory)(
                NtCurrentProcess(), &RegionBase, &RegionSize,
                PAGE_EXECUTE_READWRITE, &OldProtect);

            ZwXxxPtr[0] = 0x49;                 // mov r10, SyscallNumber
            //ZwXxxPtr[0] = 0xCC;               // debug
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
        }
#else ! _WIN64

        SBIELOW_CALL(NtProtectVirtualMemory)(
            NtCurrentProcess(), &RegionBase, &RegionSize,
            PAGE_EXECUTE_READWRITE, &OldProtect);

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

#ifndef _WIN64
    // ToDo: add wow 64 support for win32 syscall hooking
    if (Dll_IsWow64)
        return TRUE;
#endif

    // disable Electron Workaround when we are ready to hook the required win32k syscalls
    extern BOOL Dll_ElectronWorkaround;
    Dll_ElectronWorkaround = FALSE; 

    //
    // chrome needs for a working GPU acceleration the GdiDdDDI* win32k syscalls to have the right user token
    //

    WCHAR* cmdline = GetCommandLine();

    if ((wcsstr(cmdline, L"--type=gpu-process") != NULL && wcsstr(cmdline, L"--gpu-preferences=") != NULL)
     || SbieApi_QueryConfBool(NULL, L"AlwaysUseWin32kHooks", FALSE)) {

        SbieDll_HookWin32SysCalls(hmodule);
    }

	//P_NtUserCreateWindowEx NtUserCreateWindowEx = (P_NtUserCreateWindowEx)GetProcAddress(hmodule, "NtUserCreateWindowEx");
    //SBIEDLL_HOOK(Win32_, NtUserCreateWindowEx);
#endif

	return TRUE;
}

/*_FX NTSTATUS Win32_NtUserCreateWindowEx(
	DWORD dwExStyle,
	PUNICODE_STRING UnsafeClassName,
	LPCWSTR VersionedClass,
	PUNICODE_STRING UnsafeWindowName,
	DWORD dwStyle,
	LONG x,
	LONG y,
	LONG nWidth,
	LONG nHeight,
	HWND hWndParent,
	HMENU hMenu,
	HINSTANCE hInstance,
	LPVOID lpParam,
	DWORD dwShowMode,
	DWORD dwUnknown1,
	DWORD dwUnknown2,
	VOID* qwUnknown3)
{
#if 1

	NTSTATUS ret = __sys_NtUserCreateWindowEx(
		dwExStyle,
		UnsafeClassName,
		VersionedClass,
		UnsafeWindowName,
		dwStyle,
		x,
		y,
		nWidth,
		nHeight,
		hWndParent,
		hMenu,
		hInstance,
		lpParam,
		dwShowMode,
		dwUnknown1,
		dwUnknown2,
		qwUnknown3);

	return ret;

#else
	extern HANDLE SbieApi_DeviceHandle;

	ULONG syscall_index = 0x1074; // NtUserCreateWindowEx in 21H2

	ULONG64 stack[17];

	stack[0] = dwExStyle;
	stack[1] = (ULONG64)UnsafeClassName;
	stack[2] = (ULONG64)VersionedClass;
	stack[3] = (ULONG64)UnsafeWindowName;
	stack[4] = dwStyle;
	stack[5] = x;
	stack[6] = y;
	stack[7] = nWidth;
	stack[8] = nHeight;
	stack[9] = (ULONG64)hWndParent;
	stack[10] = (ULONG64)hMenu;
	stack[11] = (ULONG64)hInstance;
	stack[12] = (ULONG64)lpParam;
	stack[13] = dwShowMode;
	stack[14] = dwUnknown1;
	stack[15] = dwUnknown2;
	stack[16] = (ULONG64)qwUnknown3;

	NTSTATUS status;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];

    memset(parms, 0, sizeof(parms));
    parms[0] = API_INVOKE_SYSCALL32;
	parms[1] = (ULONG64)(ULONG_PTR)syscall_index;
    parms[2] = (ULONG64)(ULONG_PTR)stack; // pointer to system service arguments on stack

	IO_STATUS_BLOCK MyIoStatusBlock;
	status = SbieApi_NtDeviceIoControlFile(
        SbieApi_DeviceHandle, NULL, NULL, NULL, &MyIoStatusBlock,
        API_SBIEDRV_CTLCODE, parms, sizeof(ULONG64) * 8, NULL, 0);

	return status;

#endif
}*/
