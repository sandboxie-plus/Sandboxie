/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2024 David Xanatos, xanasoft.com
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
#include "common/hook_util.c"

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
#ifdef _M_ARM64
#define GET_ADDR_OF_PEB (*((ULONG_PTR*)(__getReg(18) + 0x60)))
#else
#define GET_ADDR_OF_PEB __readgsqword(0x60)
#endif
//#define GET_ADDR_OF_PROCESS_HEAP ((ULONG_PTR *)(GET_ADDR_OF_PEB + 0x30))
#define GET_ADDR_OF_PROCESS_BASE (*(ULONG_PTR *)(GET_ADDR_OF_PEB + 0x10))

#else ! _WIN64

// Pointer to 32-bit ProcessHeap is at offset 0x0018 of 32-bit PEB
#define GET_ADDR_OF_PEB __readfsdword(0x30)
//#define GET_ADDR_OF_PROCESS_HEAP ((ULONG_PTR *)(GET_ADDR_OF_PEB + 0x18))
#define GET_ADDR_OF_PROCESS_BASE (*(ULONG_PTR *)(GET_ADDR_OF_PEB + 0x08))

#endif _WIN64

#define SBIELOW_CALL(x) ((P_##x)&data->x##_code)


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

#ifdef _M_ARM64

extern void* SystemServiceARM64;
extern void* NtDeviceIoControlFileEC;
extern ULONG DeviceIoControlSvc;
extern UINT_PTR EcExitThunkPtr;

extern void* DetourCodeARM64;

extern SBIELOW_DATA SbieLowData;

ULONG_PTR EntrypointC();
#else
ULONG_PTR EntrypointC(SBIELOW_DATA *data, void *DetourCode, void *SystemService);
#endif

extern void InitInject(SBIELOW_DATA *data, void *DetourCode);

static void InitSyscalls(SBIELOW_DATA *data, void *);

#ifdef _M_ARM64
static void DisableCHPE(SBIELOW_DATA *data);
#endif

static void InitConsoleWOW64(SBIELOW_DATA *data);



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


//#ifdef _WIN64
//_FX ULONG64 SbieApi_QueryProcessInfo(SBIELOW_DATA *data, ULONG info_type)
//{
//    NTSTATUS status;
//    __declspec(align(8)) ULONG64 ResultValue;
//    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
//    API_QUERY_PROCESS_INFO_ARGS *args = (API_QUERY_PROCESS_INFO_ARGS *)parms;
//
//    memzero(parms, sizeof(parms));
//    args->func_code             = API_QUERY_PROCESS_INFO;
//
//    args->process_id.val64      = 0;
//    args->info_type.val64       = (ULONG64)(ULONG_PTR)info_type;
//    args->info_data.val64       = (ULONG64)(ULONG_PTR)&ResultValue;
//
//    status = SbieApi_Ioctl(data, parms);
//    if (! NT_SUCCESS(status))
//        ResultValue = 0;
//
//    return ResultValue;
//}
//#endif _WIN64


//---------------------------------------------------------------------------
// SbieApi_LogMsg
//---------------------------------------------------------------------------


_FX NTSTATUS SbieApi_LogMsg(ULONG64 pNtDeviceIoControlFile, ULONG64 api_device_handle, ULONG code, const WCHAR *text)
{
    NTSTATUS status = 0;
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
    args->msgid.val = code;
    args->msgtext.val = &msgtext;
    //status = SbieApi_Ioctl(data, parms);

    IO_STATUS_BLOCK MyIoStatusBlock;
#ifdef _WIN64
    ULONG MyIoStatusBlock32[2];
    *(ULONG_PTR *)&MyIoStatusBlock = (ULONG_PTR)MyIoStatusBlock32;
#endif _WIN64
    return ((P_NtDeviceIoControlFile)pNtDeviceIoControlFile)(
        (HANDLE)api_device_handle, NULL, NULL, NULL, &MyIoStatusBlock,
        API_SBIEDRV_CTLCODE, parms, sizeof(ULONG64) * 8, NULL, 0);

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_DebugError
//---------------------------------------------------------------------------


_FX NTSTATUS SbieApi_DebugError(SBIELOW_DATA* data, ULONG error)
{
    // Note: A normal string like L"text" would not result in position independent code !!!
    // hence we create a string array and fill it byte by byte

    //wchar_t text[] = { 'L','o','w','L','e','v','e','l',' ','E','r','r','o','r',':',' ','0','x',0,0,0,0,0,0,0,0,0,0};
    wchar_t text[] = { '0','x',0,0,0,0,0,0,0,0,0,0};

    // convert ulong to hex string and copy it into the message array
    wchar_t* ptr = &text[2]; // 18 // point after L"...0x"
    wchar_t table[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
    for(int i=28; i >= 0; i-=4)
        *ptr++ = table[(error >> i) & 0xF];

    return SbieApi_LogMsg(data->NtDeviceIoControlFile, data->api_device_handle, 2180, text);
}


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
// WriteMemorySafe
//---------------------------------------------------------------------------


_FX void WriteMemorySafe(SBIELOW_DATA* data, void *Address, SIZE_T Size, void *Data)
{
    void *RegionBase = Address;
    SIZE_T RegionSize = Size;
    ULONG OldProtect;

    SBIELOW_CALL(NtProtectVirtualMemory)(
        NtCurrentProcess(), &RegionBase, &RegionSize,
        PAGE_EXECUTE_READWRITE, &OldProtect);

    // memcopy is not available, lets do our own
    switch (Size) {
    case 1: *(UCHAR*)Address = *(UCHAR*)Data;       break;
    case 2: *(USHORT*)Address = *(USHORT*)Data;     break;
    case 4: *(ULONG*)Address = *(ULONG*)Data;       break;
    case 8: *(ULONG64*)Address = *(ULONG64*)Data;   break;
    default:
        for (SIZE_T i = 0; i < Size; i++)
            ((UCHAR*)Address)[i] = ((UCHAR*)Data)[i];
    }

    SBIELOW_CALL(NtProtectVirtualMemory)(
        NtCurrentProcess(), &RegionBase, &RegionSize,
        OldProtect, &OldProtect);
}


//---------------------------------------------------------------------------
// PrepSyscalls
//---------------------------------------------------------------------------


_FX void PrepSyscalls(SBIELOW_DATA *data, void * SystemService)
{
#ifdef _M_ARM64
    if (data->flags.is_arm64ec) {

        //
        // Windows on arm64 uses a special syscall wrapper functions
        // when running in arm64ec / x64 mode hence we need to 
        // point our SystemService's NtDeviceIoControlFile to 
        // a replica of the #NtDeviceIoControlFile EC variant
        //

        ULONG64 pNtDeviceIoControlFileEC = (ULONG64)&NtDeviceIoControlFileEC;
        WriteMemorySafe(data, &data->NtDeviceIoControlFile, sizeof(ULONG64), &pNtDeviceIoControlFileEC);


        // 
        // On arm64 NtDeviceIoControlFile_code looks like this:
        //  svc #0x07
        //  ret
        // 
        // as DeviceIoControlSvc points to our svc instruction,
        // we can just copy the ULONG strait out of the native function
        //

        WriteMemorySafe(data, &DeviceIoControlSvc, sizeof(ULONG), &data->NtDeviceIoControlFile_code[0]);


        //
        // get the EcExitThunkPtr which points to
        // __os_arm64x_dispatch_call_no_redirect
        // 
        // syscall_ec_data[0] // total data length
        // syscall_ec_data[1] // extra data offset
        // 
        // EcExitThunkPtr is at (extra_data_offset - 8)
        // 

        ULONG* syscall_ec_data = (ULONG*)data->syscall_data;

        UINT_PTR pEcExitThunkPtr = *(UINT_PTR*)((UINT_PTR)syscall_ec_data + syscall_ec_data[1] - 8);
        WriteMemorySafe(data, &EcExitThunkPtr, sizeof(UINT_PTR), &pEcExitThunkPtr);
    }
    else
#endif
    {
        ULONG64 pNtDeviceIoControlFile = (ULONG64)&data->NtDeviceIoControlFile_code[0];
        WriteMemorySafe(data, &data->NtDeviceIoControlFile, sizeof(ULONG64), &pNtDeviceIoControlFile);
    }

    const LONG OFFSET_ULONG_PTR =
#ifdef _M_ARM64
    -(LONG)sizeof(ULONG_PTR); // on arm64 windows we use a prepended data region
#elif _WIN64
    2;  // on 64-bit Windows, "mov rax, 0" instruction is two bytes
#else
    1;  // on 32-bit Windows, "mov edx, 0" instruction is one byte
#endif

    //
    // modify our assembly SystemService entry stub (in entry.asm)
    // to include the data area pointer
    //

    WriteMemorySafe(data, ((UCHAR *)SystemService) + OFFSET_ULONG_PTR, sizeof(ULONG_PTR), &data);

    //
    // store the SystemService address in pSystemService
    //

    ULONG64 SystemServicePtr = (ULONG64)SystemService;
    WriteMemorySafe(data, &data->pSystemService, sizeof(ULONG64), &SystemServicePtr);
}


//---------------------------------------------------------------------------
// InitSyscalls
//---------------------------------------------------------------------------


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
                         + sizeof(ULONG)                                    // size of buffer
                         + sizeof(ULONG)                                    // offset to extra data
                         + (NATIVE_FUNCTION_SIZE * NATIVE_FUNCTION_COUNT)); // saved code from ntdll

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

        void* ChromeFunc = Hook_CheckChromeHook(ZwXxxPtr, (void*)GET_ADDR_OF_PROCESS_BASE);
        if (ChromeFunc != NULL) {
            if (ChromeFunc != (void*)-1)
                ZwXxxPtr = ChromeFunc;
            else {
                if(data->flags.hook_dbg)
                    SbieApi_DebugError(data, 0x60 | (SyscallPtr[0] << 16));
                //SyscallPtr += 2;
                //continue;
            }
        }

        //
        // make the syscall address writable
        //

        RegionBase = ZwXxxPtr;

#ifdef _M_ARM64
        RegionSize = 16;
#elif _WIN64
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

#ifdef _M_ARM64

        //
        // On arm64 syscall wrapper functions looks like this:
        //  svc #0xXXX
        //  ret
        //  DCD 0
        //  DCD 0
        // so we have only 16 bytes to use per detour
        //

        if (data->Sbie64bitJumpTable) {

            unsigned char * jTableTarget = (unsigned char *)&data->Sbie64bitJumpTable->entry[SyscallNum & 0x3ff]; // jump table is sized for up to 1024 entries

            ULONG* aCode = (ULONG*)jTableTarget;

            MOV mov;
            mov.OP   = 0xD2800011;  // mov x17, #0xFFFF
            mov.imm16 = (USHORT)SyscallNum; 
            *aCode++ = mov.OP;
	        *aCode++ = 0x58000048;	// ldr x8, 8
	        *aCode++ = 0xD61F0100;	// br x8
            *(ULONG_PTR*)aCode = (ULONG_PTR)SystemServiceAsm;   // DCQ &SystemServiceAsm
            // 20

            aCode = (ULONG*)ZwXxxPtr;
	        *aCode++ = 0x58000048;	// ldr x8, 8
	        *aCode++ = 0xD61F0100;	// br x8
            *(ULONG_PTR*)aCode = (ULONG_PTR)jTableTarget;       // DCQ &jTableTarget
            // 16
        }
        else {

            ULONG* aCode = (ULONG*)ZwXxxPtr;

            MOV mov;
            mov.OP   = 0xD2800011;  // mov x17, #0xFFFF
            mov.imm16 = (USHORT)SyscallNum; 
            *aCode++ = mov.OP;
	        *aCode++ = 0x18000048;	// ldr w8, 8
	        *aCode++ = 0xD61F0100;	// br x8
            *(ULONG*)aCode = (ULONG)(ULONG_PTR)SystemServiceAsm;// DCD &SystemServiceAsm
            // 16
        }

        SBIELOW_CALL(NtFlushInstructionCache)(
            NtCurrentProcess(), RegionBase, (ULONG)RegionSize);

#elif _WIN64

        if (data->Sbie64bitJumpTable) {
            // bytes overwritten /*16*/ 13;

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
            // bytes overwritten 14;

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
        // bytes overwritten 10;

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


#ifdef _M_ARM64

//---------------------------------------------------------------------------
// MyImageOptionsEx
//---------------------------------------------------------------------------


_FX NTSTATUS MyImageOptionsEx(PUNICODE_STRING SubKey, PCWSTR ValueName, 
    ULONG Type, PVOID Buffer, ULONG BufferSize, PULONG ReturnedLength, BOOLEAN Wow64, SBIELOW_DATA* data)
{
    // Note: A normal string like L"LoadCHPEBinaries" would not result in position independent code !!!
    wchar_t LoadCHPEBinaries[] = { 'L','o','a','d','C','H','P','E','B','i','n','a','r','i','e','s',0 }; 
    PCWSTR ptr = ValueName;
    for (PCWSTR tmp = LoadCHPEBinaries; *ptr && *tmp && *ptr == *tmp; ptr++, tmp++);
	if (*ptr == L'\0'){ //if (_wcsicmp(ValueName, L"LoadCHPEBinaries") == 0)
		*(ULONG*)Buffer = 0;
		return 0; // STATUS_SUCCESS
	}
    //return 0xC0000034; // STATUS_OBJECT_NAME_NOT_FOUND

    typedef (*P_ImageOptionsEx)(PUNICODE_STRING, PCWSTR, ULONG, PVOID, ULONG, PULONG, BOOLEAN);
    return ((P_ImageOptionsEx)data->RtlImageOptionsEx_tramp)(SubKey, ValueName, Type, Buffer, BufferSize, ReturnedLength, Wow64);
}


//---------------------------------------------------------------------------
// DisableCHPE
//---------------------------------------------------------------------------


_FX void DisableCHPE(SBIELOW_DATA* data)
{
    SYSCALL_DATA* syscall_data;
    SBIELOW_EXTRA_DATA *extra;
    ULONG uError = 0;

    //
    // Sandboxie on ARM64 requires x86 applications NOT to use the CHPE (Compiled Hybrid Portable Executable)
    // binaries as when hooking a hybrid binary it is required to hook the internal native functions.
    // 
    // This can be done quite easily for ARM64EC (x64 on ARM64) by compiling SbieDll.dll as ARM64EC
    // and resolving the FFS sequence targets, which then can be hooked with the native SbieDll.dll functions.
    // 
    // For CHPE MSFT how ever does not provide any public build tool chain, hence it would be required
    // to hand craft native detour targets and then properly transition to x86 code which is not documented.
    // When the use of hybrid binaries for x86 is disabled all loaded DLL's, except the native ntdll.dll
    // are pure x86 binaries and can be easily hooked with SbieDll.dll's x86 functions.
    // 
    // To prevent the kernel from loading the CHPE version of ntdll.dll we can pass PsAttributeChpe = 0
    // in the AttributeList of NtCreateUserProcess, however then the created process will still try to 
    // load the rest of the system dll's from the SyChpe32 directory and fail to initialize.
    // There for we have to hook LdrQueryImageFileExecutionOptionsEx and simulate the LoadCHPEBinaries = 0
    // in its "Image File Execution Options" key this way the process will continue with loading
    // the regular x86 binaries from the SysWOW64 directory and initialize properly.
    // 
    // Note: This hook affects only the native function and is only installed on x86 processes
    //          hence we install a similar hook in SbieDll.dll!Proc_Init which causes 
    //          CreateProcessInternalW to set PsAttributeChpe = 0 when creating new processes.
    //

    syscall_data = (SYSCALL_DATA *)data->syscall_data;

    extra = (SBIELOW_EXTRA_DATA *) (data->syscall_data + syscall_data->extra_data_offset);

    void* RtlImageOptionsEx = FindDllExport((void*)data->ntdll_base, 
                                    (UCHAR*)extra + extra->RtlImageOptionsEx_offset, &uError);
    if (!RtlImageOptionsEx)
        return;

    void *RegionBase;
    SIZE_T RegionSize;
    ULONG OldProtect;
    ULONG* aCode;

    //
    // backup target & create simple trampoline
    //

    RegionBase = (void*)data->RtlImageOptionsEx_tramp;
    RegionSize = sizeof(data->RtlImageOptionsEx_tramp);
    SBIELOW_CALL(NtProtectVirtualMemory)(
        NtCurrentProcess(), &RegionBase, &RegionSize,
        PAGE_EXECUTE_READWRITE, &OldProtect);

    ULONG DetourSize = 28;
    memcpy(data->RtlImageOptionsEx_tramp, RtlImageOptionsEx, DetourSize); 

    aCode = (ULONG*)(data->RtlImageOptionsEx_tramp + DetourSize); // 28
	aCode[0] = 0x58000048;	// ldr x8, 8 - Rest of RtlImageOptionsEx
	aCode[1] = 0xD61F0100;	// br x8
	*(DWORD64*)&aCode[2] = (DWORD64)RtlImageOptionsEx + DetourSize; 
    // 44

    SBIELOW_CALL(NtProtectVirtualMemory)(
        NtCurrentProcess(), &RegionBase, &RegionSize,
        OldProtect, &OldProtect);

    SBIELOW_CALL(NtFlushInstructionCache)(
        NtCurrentProcess(), RegionBase, (ULONG)RegionSize);

    //
    // make target writable & create detour
    //

    RegionBase = (void*)RtlImageOptionsEx;
    RegionSize = DetourSize;
    SBIELOW_CALL(NtProtectVirtualMemory)(
        NtCurrentProcess(), &RegionBase, &RegionSize,
        PAGE_EXECUTE_READWRITE, &OldProtect);

	aCode = (ULONG*)RtlImageOptionsEx;
    aCode[0] = 0x580000a7;	// ldr x7, 20 - data
	aCode[1] = 0x58000048;	// ldr x8, 8 - MyImageOptionsEx
	aCode[2] = 0xD61F0100;	// br x8
	*(DWORD64*)&aCode[3] = (DWORD64)MyImageOptionsEx; 
    *(DWORD64*)&aCode[5] = (DWORD64)data;
    //28

    SBIELOW_CALL(NtProtectVirtualMemory)(
        NtCurrentProcess(), &RegionBase, &RegionSize,
        OldProtect, &OldProtect);

    SBIELOW_CALL(NtFlushInstructionCache)(
        NtCurrentProcess(), RegionBase, (ULONG)RegionSize);
}
#endif


#ifdef _WIN64

//---------------------------------------------------------------------------
// InitConsoleWOW64
//---------------------------------------------------------------------------


_FX void InitConsoleWOW64(SBIELOW_DATA *data)
{
    ULONG64 addr64;
    ULONG   addr32;

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
    addr64 = GET_ADDR_OF_PEB;
    // get 64-bit RTL_USER_PROCESS_PARAMETERS
    addr64 = *(ULONG_PTR *)(addr64 + 0x20);
    // ConsoleHandle member is at offset 0x10
    addr64 += 0x10;

    if (*(ULONG64 *)addr64 == -1) {

        // get 32-bit TEB
#ifdef _M_ARM64
        addr32  = (ULONG)(__getReg(18));
        addr32 = *(ULONG *)(ULONG_PTR)(addr32 + 0x00);
#else
        addr32 = __readgsdword(0);
#endif
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


#ifdef _M_ARM64
ULONG_PTR EntrypointC() {
    SBIELOW_DATA* data = &SbieLowData;
    void* DetourCode = &DetourCodeARM64;
    void* SystemService = &SystemServiceARM64;
#else
ULONG_PTR EntrypointC(SBIELOW_DATA *data, void *DetourCode, void *SystemService) {
#endif

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
    //volatile ULONG_PTR *ProcessHeap = GET_ADDR_OF_PROCESS_HEAP;

    if(!data->Init_Done)
    {
        SYSCALL_DATA* syscall_data;
        SBIELOW_EXTRA_DATA *extra;

        //
        // Starting with Windows 11 build 26040 ProcessHeap must be 0
        // for the process initialisation to be successful, hence from
        // now on we use Init_Lock in the SBIELOW_EXTRA_DATA to synchronize
        // the execution of the init code, the first thread to arrive here
        // will encounter the initial 0 value and be allowed to execute
        // the value will be changed to -1 indicating initialization in
        // progress. Subsequent threads will wait, until the first thread,
        // once done changes the value to 1 indicating initialization completion.
        // 
        // Since SBIELOW_EXTRA_DATA is freed by Ldr_Inject_Entry hence we need
        // to also use Init_Done in SBIELOW_DATA its initially 0 and
        // once the initialization is completed will be set to 1, such
        // that later created threads can skip the initialization code.
        //

        syscall_data = (SYSCALL_DATA*)data->syscall_data;
      
        extra = (SBIELOW_EXTRA_DATA *) (data->syscall_data + syscall_data->extra_data_offset);

        volatile ULONG_PTR *Init_Lock = &extra->Init_Lock;

#ifdef _WIN64
        ULONG_PTR OldInit_Lock = _InterlockedCompareExchange64(Init_Lock, -1, 0);
#else ! _WIN64
        ULONG_PTR OldInit_Lock = _InterlockedCompareExchange(Init_Lock, -1, 0);
#endif _WIN64

        if (OldInit_Lock == 0) {

            //
            // the first thread arrives here
            //

            // WaitForDebugger(data);

            //wchar_t text[] = { 't','e','s','t',0 };
            //SbieApi_LogMsg(data->NtDeviceIoControlFile, data->api_device_handle, 1122, text);

            PrepSyscalls(data, SystemService);
            if (!data->flags.bHostInject && !data->flags.bNoSysHooks)
                InitSyscalls(data, SystemService);

		    InitInject(data, DetourCode);

#ifdef _WIN64
            if (data->flags.is_wow64) {

#ifdef _M_ARM64
                if(!data->flags.is_chpe32)
		            DisableCHPE(data);
#endif
                if (!data->flags.bNoConsole)
                    InitConsoleWOW64(data);
            }
#endif

            // Set Init_Done 
            UCHAR Init_Done = 1;
            WriteMemorySafe(data, &data->Init_Done, sizeof(UCHAR), &Init_Done);

            // Release waiting threads
#ifdef _WIN64
            _InterlockedExchange64(Init_Lock, 1);
#else ! _WIN64
            _InterlockedExchange(Init_Lock, 1);
#endif _WIN64

        } else if (OldInit_Lock == -1) {

            //
            // any other threads arrive here and will wait for
            // the first thread to finish initializing SbieLow
            //

            LARGE_INTEGER delay;
            delay.QuadPart = -SECONDS(3) / 100; // 0FFFFFFFFh 0FFFB6C20h

            while (*Init_Lock == -1) {

                const P_NtDelayExecution NtDelayExecution =
                    (P_NtDelayExecution) &data->NtDelayExecution_code;

                NtDelayExecution(FALSE, &delay);
            }
        }
    }

    //
    // when SbieLow initialization is done, pass control
    // to original LdrInitializeThunk code in ntdll
    //


    return (ULONG_PTR)&data->LdrInitializeThunk_tramp;
}
