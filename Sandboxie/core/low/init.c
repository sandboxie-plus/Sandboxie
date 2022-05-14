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
// Low Level DLL initialization code
//---------------------------------------------------------------------------

#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

#include <windows.h>
#include "common/win32_ntddk.h"
#include "common/defines.h"
#include "core/drv/api_defs.h"
#include "lowdata.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define FILE_DEVICE_UNKNOWN             0x00000022
#define METHOD_NEITHER                  3
#define FILE_ANY_ACCESS                 0
#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

#ifdef _WIN64
// Pointer to 64-bit ProcessHeap is at offset 0x0030 of 64-bit PEB
#define GET_ADDR_OF_PEB __readgsqword(0x60)
#define GET_ADDR_OF_PROCESS_HEAP ((ULONG_PTR *)(GET_ADDR_OF_PEB + 0x30))


#else ! _WIN64

// Pointer to 32-bit ProcessHeap is at offset 0x0018 of 32-bit PEB
#define GET_ADDR_OF_PEB __readfsdword(0x30)
#define GET_ADDR_OF_PROCESS_HEAP ((ULONG_PTR *)(GET_ADDR_OF_PEB + 0x18))

#endif _WIN64

#define SBIELOW_CALL(x) ((P_##x)&data->x##_code)


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

#ifdef _WIN64
ULONG_PTR EntrypointC(SBIELOW_DATA *data, void *ActivationContext, void *SystemService, void * ActivationContext64);
extern void InitInject(SBIELOW_DATA *data, void *,void *);
#else
ULONG_PTR EntrypointC(SBIELOW_DATA *data, void *ActivationContext, void *SystemService);
extern void InitInject(SBIELOW_DATA *data, void *);
#endif
static void InitSyscalls(SBIELOW_DATA *data, void *);

static void InitConsole(SBIELOW_DATA *data);



NTSTATUS SystemServiceC(
    SBIELOW_DATA *data, ULONG syscall_index, ULONG_PTR *syscall_args);


//---------------------------------------------------------------------------
// SbieApi_Ioctl
//---------------------------------------------------------------------------


_FX NTSTATUS SbieApi_Ioctl(SBIELOW_DATA *data, void *parms)
{
    IO_STATUS_BLOCK MyIoStatusBlock;

#ifdef _WIN64
    ULONG MyIoStatusBlock32[2];
    *(ULONG_PTR *)&MyIoStatusBlock = (ULONG_PTR)MyIoStatusBlock32;
#endif _WIN64

    return SBIELOW_CALL(NtDeviceIoControlFile)(
        (HANDLE)data->api_device_handle, NULL, NULL, NULL, &MyIoStatusBlock,
        API_SBIEDRV_CTLCODE, parms, sizeof(ULONG64) * 8, NULL, 0);
}


//---------------------------------------------------------------------------
// SbieApi_QueryProcessInfo
//---------------------------------------------------------------------------


#ifdef _WIN64


_FX ULONG64 SbieApi_QueryProcessInfo(SBIELOW_DATA *data, ULONG info_type)
{
    NTSTATUS status;
    __declspec(align(8)) ULONG64 ResultValue;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_QUERY_PROCESS_INFO_ARGS *args = (API_QUERY_PROCESS_INFO_ARGS *)parms;

    memzero(parms, sizeof(parms));
    args->func_code             = API_QUERY_PROCESS_INFO;

    args->process_id.val64      = 0;
    args->info_type.val64       = (ULONG64)(ULONG_PTR)info_type;
    args->info_data.val64       = (ULONG64)(ULONG_PTR)&ResultValue;

    status = SbieApi_Ioctl(data, parms);
    if (! NT_SUCCESS(status))
        ResultValue = 0;

    return ResultValue;
}


#endif _WIN64


//---------------------------------------------------------------------------
// SbieApi_DebugPrint
//---------------------------------------------------------------------------


#if 0
_FX NTSTATUS SbieApi_DebugPrint(SBIELOW_DATA *data, const WCHAR *text)
{
#ifdef _WIN64
    NTSTATUS status;
    __declspec(align(8)) UNICODE_STRING64 msgtext;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_LOG_MESSAGE_ARGS *args = (API_LOG_MESSAGE_ARGS *)parms;

    ULONG len;
    for (len = 0; text[len]; ++len)
        ;

    msgtext.Buffer = (ULONG_PTR)text;
    msgtext.Length = (USHORT)(len * sizeof(WCHAR));
    msgtext.MaximumLength = msgtext.Length + sizeof(WCHAR);

    memzero(parms, sizeof(parms));
    args->func_code = API_LOG_MESSAGE;
    args->session_id.val = -1;
    args->msgid.val = 2101;
    args->msgtext.val = &msgtext;

    return SbieApi_Ioctl(data, parms);
#else ! _WIN64
    return 0;
#endif _WIN64
}
#endif


//---------------------------------------------------------------------------
// WaitForDebugger
//---------------------------------------------------------------------------


#if 0
_FX void WaitForDebugger(SBIELOW_DATA *data)
{
    volatile UCHAR *pIsDebuggerPresent = (UCHAR *)(GET_ADDR_OF_PEB + 2);

    while (! (*pIsDebuggerPresent)) {

        const P_NtDelayExecution NtDelayExecution =
            (P_NtDelayExecution) &data->NtDelayExecution_code;

        LARGE_INTEGER delay;
        delay.QuadPart = -SECONDS(3) / 100; // 0FFFFFFFFh 0FFFB6C20h

        NtDelayExecution(FALSE, &delay);
    }

    __debugbreak();
}
#endif


//---------------------------------------------------------------------------
// InitSyscalls
//---------------------------------------------------------------------------

#ifdef _WIN64

#define MAX_FUNC_SIZE 0x76

//Note any change to this function requires the same modification to the same function in sbiedll: see dllhook.c (findChromeTarget)
ULONGLONG * findChromeTarget(unsigned char* addr)
{
    int i = 0;
    ULONGLONG target;
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
				
                // special case when compiled using mingw toolchain
                // mov rcx,qword ptr [rax+offset] or mov rcx,qword ptr [rcx+offset]
                if ((*(USHORT *)&addr[i + 7] == 0x8B48)) 
                {
                    if (addr[i + 9] == 0x48 || addr[i + 9] == 0x49)
                        delta = addr[i + 10];
                    else if (addr[i + 9] == 0x88 || addr[i + 9] == 0x89)
                        delta = *(ULONG*)&addr[i + 10];
                    else
                        break;
                    target = (ULONGLONG)ChromeTarget + delta;
                    ChromeTarget = *(ULONGLONG **)target;
                }
				
                // }
                break;
            }
        }
    }
    return ChromeTarget;
}
#endif

void *Hook_CheckChromeHook(void *SourceFunc)
{
    if (!SourceFunc)
        return NULL;
    UCHAR *ZwXxxPtr = (UCHAR *)SourceFunc;
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
    ULONGLONG *chrome64Target = NULL;

    if (ZwXxxPtr[0] == 0x50 &&	//push rax
        ZwXxxPtr[1] == 0x48 &&	//mov rax,?
        ZwXxxPtr[2] == 0xb8) {
        ULONGLONG* longlongs = *(ULONGLONG**)&ZwXxxPtr[3];
        chrome64Target = findChromeTarget((unsigned char*)longlongs);
    }
    // Chrome 49+ 64bit hook
    // mov rax, <target> 
    // jmp rax 
    else if (ZwXxxPtr[0] == 0x48 && //mov rax,<target>
        ZwXxxPtr[1] == 0xb8 &&
        *(USHORT*)&ZwXxxPtr[10] == 0xe0ff)/* jmp rax */ {
        ULONGLONG* longlongs = *(ULONGLONG**)&ZwXxxPtr[2];
        chrome64Target = findChromeTarget((unsigned char*)longlongs);
    }
    if (chrome64Target != NULL) {
        ZwXxxPtr = (UCHAR *)chrome64Target;
    }
#endif 
    return ZwXxxPtr;
}

_FX void PrepSyscalls(SBIELOW_DATA *data, void * SystemService)
{
    UCHAR *SystemServiceAsm;
    void *RegionBase;
    SIZE_T RegionSize;
    ULONG OldProtect;

    const ULONG OFFSET_ULONG_PTR =
#ifdef _WIN64
    2;  // on 64-bit Windows, "mov rax, 0" instruction is two bytes
#else
    1;  // on 32-bit Windows, "mov edx, 0" instruction is one byte
#endif

    //
    // modify our assembly SystemService entry stub (in entry.asm)
    // to include the data area pointer
    //

    SystemServiceAsm = (UCHAR *)SystemService;
    RegionBase = (void *)(SystemServiceAsm + OFFSET_ULONG_PTR);
    RegionSize = sizeof(ULONG_PTR);

    SBIELOW_CALL(NtProtectVirtualMemory)(
        NtCurrentProcess(), &RegionBase, &RegionSize,
        PAGE_EXECUTE_READWRITE, &OldProtect);

    *(ULONG_PTR *)(SystemServiceAsm + OFFSET_ULONG_PTR) = (ULONG_PTR)data;

    SBIELOW_CALL(NtProtectVirtualMemory)(
        NtCurrentProcess(), &RegionBase, &RegionSize,
        OldProtect, &OldProtect);

    data->pSystemService = (ULONG64)SystemServiceAsm;
}

_FX void InitSyscalls(SBIELOW_DATA *data, void * SystemService)
{
    UCHAR *SystemServiceAsm, *ZwXxxPtr;
    ULONG *SyscallPtr;
    ULONG SyscallNum;
    void *RegionBase;
    SIZE_T RegionSize;
    ULONG OldProtect;

    SystemServiceAsm = (UCHAR *)SystemService;

    //
    // our syscall data area describes the ZwXxx functions in ntdll,
    // overwrite each export to jump to our assembly SystemService
    // see also core/drv/syscall.c and core/svc/DriverInjectAssist.cpp
    //

    SyscallPtr = (ULONG *)(data->syscall_data
                         + sizeof(ULONG)         // size of buffer
                         + sizeof(ULONG)         // offset to extra data
                         + (32 * 4));            // saved code from ntdll

    while (SyscallPtr[0] || SyscallPtr[1]) {

        //
        // the ULONG at SyscallPtr[1] gives the offset of the ZwXxx export
        // from the base address of ntdll
        //

        ZwXxxPtr = (UCHAR *)((ULONG_PTR)SyscallPtr[1] + data->ntdll_base);

        //
        // in a chrome sandbox process, some syscalls are already hooked
        // by chrome at this point, and we want to hook the syscall stub
        // saved by chrome, rather than the chrome hook itself (32-bit only)
        //

        ZwXxxPtr = Hook_CheckChromeHook(ZwXxxPtr);

        //
        // make the syscall address writable
        //

        RegionBase = ZwXxxPtr;

#ifdef _WIN64
        RegionSize = data->Sbie64bitJumpTable ? 13 : 14; // 16;
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

        if (data->Sbie64bitJumpTable) {
            // bytes overwriten /*16*/ 13;

            unsigned char * jTableTarget = (unsigned char *)&data->Sbie64bitJumpTable->entry[SyscallNum & 0x3ff]; // jump table is sized for up to 1024 entries
            // write new patch for jump table
            // The jTable is now injected in the same memory module with lowlevel; no need for a 64 bit long jump
            // mov r10, <4 byte SyscallNum>
            jTableTarget[0] = 0x49;
            jTableTarget[1] = 0xc7;
            jTableTarget[2] = 0xc2;
            *(ULONG *)&jTableTarget[3] = SyscallNum;
            // jmp <4 byte SystemServiceAsm>
            if (data->flags.is_win10) {
                jTableTarget[7] = 0x48;
                jTableTarget[8] = 0xe9;
                *(ULONG *)&jTableTarget[9] = (ULONG)(ULONG_PTR)(SystemServiceAsm - (jTableTarget + 13));
            }
            else {

                jTableTarget[7] = 0xe9;
                *(ULONG *)&jTableTarget[8] = (ULONG)(ULONG_PTR)(SystemServiceAsm - (jTableTarget + 12));

            }
            //  *(ULONGLONG *) &jTableTarget[-8] = 0x9090909090909090; /*patch location for sboxdll hook. jtable elements need to be at 0x18 in size for this*/
            // jump table: using push rcx instead of push rax to differentiate from the chrome sandbox hook
            // the sboxdll.dll needs this distinction to handle the chrome type hook properly and to not search
            // for the chrome target in a sbox 64 bit jtable hook

            // using ret is not compatible with CET - Hardware-enforced Stack Protection
            /*ZwXxxPtr[0] = 0x51;     //push rcx
            // mov rax,<8 byte address to jTableEntry>
            ZwXxxPtr[1] = 0x48;
            ZwXxxPtr[2] = 0xb8;
            *(ULONGLONG *)&ZwXxxPtr[3] = (ULONGLONG)jTableTarget;
            *(ULONG *)&ZwXxxPtr[11] = 0x24048948;       // mov [rsp],rax
            ZwXxxPtr[15] = 0xc3;    // ret*/

            // mov rax,<8 byte address to jTableEntry>
            ZwXxxPtr[0] = 0x90; // start with a nop so that it does not look lile a chrome hook
            ZwXxxPtr[1] = 0x48;
            ZwXxxPtr[2] = 0xb8;
            *(ULONGLONG*)&ZwXxxPtr[3] = (ULONGLONG)jTableTarget;
            // jmp rax
            ZwXxxPtr[11] = 0xFF;
            ZwXxxPtr[12] = 0xE0;
        }
        else {
            // bytes overwriten 14;

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
        }
#else ! _WIN64
        // bytes overwriten 10;

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
}


//---------------------------------------------------------------------------
// InitConsole
//---------------------------------------------------------------------------


#ifdef _WIN64


_FX void InitConsole(SBIELOW_DATA *data)
{
    ULONG64 addr64;
    ULONG   addr32;

    if (! data->flags.is_wow64)
        return;

    //
    // on Windows 7, Process_Low_InitConsole from core/drv/process_low.c
    // adjusts the ConsoleHandle field in the RTL_USER_PROCESS_PARAMETERS
    // structure to prevent a console connection during initialization of
    // kernel32.
    //
    // On 64-bit Windows, Process_Low_InitConsole updates the ConsoleHandle
    // field of the 64-bit version of RTL_USER_PROCESS_PARAMETERS.  But if
    // this is a Wow64 process, kernel32 initialization looks at the 32-bit
    // ConsoleHandle.
    //
    // this code complements Process_Low_InitConsole by adjusting the
    // ConsoleHandle in the 32-bit RTL_USER_PROCESS_PARAMETERS structure.
    //

    // get 64-bit PEB
    addr64 = __readgsqword(0x60);
    // get 64-bit RTL_USER_PROCESS_PARAMETERS
    addr64 = *(ULONG_PTR *)(addr64 + 0x20);
    // ConsoleHandle member is at offset 0x10
    addr64 += 0x10;

    if (*(ULONG64 *)addr64 == -1) {

        // get 32-bit TEB
        addr32 = __readgsdword(0);
        // get 32-bit PEB
        addr32 = *(ULONG *)(ULONG_PTR)(addr32 + 0x30);
        // get 32-bit RTL_USER_PROCESS_PARAMETERS
        addr32 = *(ULONG *)(ULONG_PTR)(addr32 + 0x10);
        // ConsoleHandle member is at offset 0x10
        addr32 += 0x10;

        // copy ConsoleHandle from 64-bit structure to 32-bit structure
        *(ULONG *)(ULONG_PTR)addr32 = *(ULONG *)addr64;
    }
}


#endif _WIN64


//---------------------------------------------------------------------------
// EntrypointC
//---------------------------------------------------------------------------


#ifdef _WIN64
_FX ULONG_PTR EntrypointC(SBIELOW_DATA *data,void *ActivationContext, void *SystemService,void * ActivationContext64)
#else
_FX ULONG_PTR EntrypointC(SBIELOW_DATA *data,void *ActivationContext, void *SystemService)
#endif
{
    //
    // use the ProcessHeap field in the PEB to synchronize multiple
    // threads running our low level initialization code:
    //
    // for the very first thread, ProcessHeap is zero on entry, and is
    // set to -1 while running our init code.  any other threads that
    // start (i.e. injected) at the same time wait while ProcessHeap is
    // still -1. when our init code finishes, it passes control to
    // LdrInitializeThunk which stores new value into ProcessHeap which
    // will not be -1, thus releasing any waiting threads
    //
    volatile ULONG_PTR *ProcessHeap = GET_ADDR_OF_PROCESS_HEAP;

    ULONG_PTR OldProcessHeap =
#ifdef _WIN64
        _InterlockedCompareExchange64(ProcessHeap, -1, 0);

#else ! _WIN64
                    _InterlockedCompareExchange(ProcessHeap, -1, 0);
#endif _WIN64

    if (OldProcessHeap == 0) {

        //
        // the first thread arrives here
        //

        // WaitForDebugger(data);

        PrepSyscalls(data, SystemService);
		if(!data->flags.bHostInject && !data->flags.bNoSysHooks)
			InitSyscalls(data, SystemService);

#ifdef _WIN64
		InitInject(data, ActivationContext, ActivationContext64);

		if (!data->flags.bNoConsole)
			InitConsole(data);
#else
		InitInject(data, ActivationContext);
#endif

    } else if (OldProcessHeap == -1) {

        //
        // any other threads arrive here and will wait for
        // the first thread to finish initializing SbieLow
        //

        LARGE_INTEGER delay;
        delay.QuadPart = -SECONDS(3) / 100; // 0FFFFFFFFh 0FFFB6C20h

        while (*ProcessHeap == -1) {

            const P_NtDelayExecution NtDelayExecution =
                (P_NtDelayExecution) &data->NtDelayExecution_code;

            NtDelayExecution(FALSE, &delay);
        }
    }

    //
    // when SbieLow initialization is done, pass control
    // to original LdrInitializeThunk code in ntdll
    //


    return (ULONG_PTR)&data->LdrInitializeThunk_tramp;
}

