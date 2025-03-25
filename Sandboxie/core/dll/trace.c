/*
 * Copyright 2020-2024 DavidXanatos, xanasoft.com
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
// Trace Helper
//---------------------------------------------------------------------------

#include "dll.h"
#include "trace.h"
#include "core/low/lowdata.h"

#include <dbghelp.h>

#ifdef _M_ARM64EC
void* Hook_GetFFSTarget(void* ptr);
#endif

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static void Trace_RtlSetLastWin32Error(ULONG err);

static void Trace_OutputDebugStringW(const WCHAR *str);

static void Trace_OutputDebugStringA(const UCHAR *str);

static NTSTATUS InstallInstrumentationCallback();


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------

extern SBIELOW_DATA* SbieApi_data;


typedef void (*P_RtlSetLastWin32Error)(ULONG err);
typedef void (*P_OutputDebugString)(const void *str);

static P_RtlSetLastWin32Error       __sys_RtlSetLastWin32Error      = NULL;
static P_OutputDebugString          __sys_OutputDebugStringW        = NULL;
static P_OutputDebugString          __sys_OutputDebugStringA        = NULL;

BOOLEAN Dll_SbieTrace = FALSE;
BOOLEAN Dll_ApiTrace = FALSE;
BOOLEAN Dll_FileTrace = FALSE;


//---------------------------------------------------------------------------
// Trace_Init
//---------------------------------------------------------------------------


_FX int Trace_Init(void)
{
    Dll_SbieTrace = SbieApi_QueryConfBool(NULL, L"SbieTrace", FALSE);

    Dll_ApiTrace = Config_GetSettingsForImageName_bool(L"ApiTrace", FALSE);

    Dll_FileTrace = Config_GetSettingsForImageName_bool(L"FileTrace", FALSE);

    if (SbieApi_QueryConfBool(NULL, L"ErrorTrace", FALSE)) {

        //
        // intercept NTDLL entry points
        //
        HMODULE module = Dll_Ntdll;

        P_RtlSetLastWin32Error RtlSetLastWin32Error = (P_RtlSetLastWin32Error)
            GetProcAddress(Dll_Ntdll, "RtlSetLastWin32Error");
        SBIEDLL_HOOK(Trace_, RtlSetLastWin32Error);
    }

    if (SbieApi_QueryConfBool(NULL, L"DebugTrace", FALSE)) {

        //
        // intercept KERNEL32 entry points
        //
        HMODULE module = Dll_Kernel32;

        P_OutputDebugString OutputDebugStringW = (P_OutputDebugString)
            GetProcAddress(Dll_Kernel32, "OutputDebugStringW");
        SBIEDLL_HOOK(Trace_, OutputDebugStringW);

        P_OutputDebugString OutputDebugStringA = (P_OutputDebugString)
            GetProcAddress(Dll_Kernel32, "OutputDebugStringA");
        SBIEDLL_HOOK(Trace_, OutputDebugStringA);


        OutputDebugString(L"SbieDll injected...\n");
    }

    //
    // If there are any CallTrace options set, then install syscall instrumentation
    //

    WCHAR wsTraceOptions[4];
    if (SbieApi_QueryConf(NULL, L"CallTraceEx", 0, wsTraceOptions, sizeof(wsTraceOptions)) == STATUS_SUCCESS && wsTraceOptions[0] != L'\0') {
        if (!NT_SUCCESS(InstallInstrumentationCallback()))
            SbieApi_Log(2205, L"ProcessInstrumentationCallback");
    }

    return TRUE;
}

//---------------------------------------------------------------------------
// Trace_Init
//---------------------------------------------------------------------------


_FX void Trace_Entry(void)
{
#ifdef WITH_DEBUG
    DbgTrace("Dll_InitExeEntry completed");
#endif
}


//---------------------------------------------------------------------------
// Trace_RtlSetLastWin32Error
//---------------------------------------------------------------------------


ALIGNED void Trace_RtlSetLastWin32Error(ULONG err)
{
    if (err) {
		WCHAR strW[64];
        Sbie_snwprintf(strW, 64, L"SetError: %d\n", err);
		SbieApi_MonitorPutMsg(MONITOR_OTHER | MONITOR_TRACE, strW);
    }
    __sys_RtlSetLastWin32Error(err);
}


//---------------------------------------------------------------------------
// Trace_OutputDebugStringW
//---------------------------------------------------------------------------


ALIGNED void Trace_OutputDebugStringW(const WCHAR *strW)
{
	SbieApi_MonitorPutMsg(MONITOR_OTHER | MONITOR_TRACE, strW);

    __sys_OutputDebugStringW(strW);
}


//---------------------------------------------------------------------------
// Trace_OutputDebugStringA
//---------------------------------------------------------------------------


ALIGNED void Trace_OutputDebugStringA(const UCHAR *strA)
{
	WCHAR strW[256 + 1];
	Sbie_snwprintf(strW, 256 + 1, L"%S", strA); // convert to WCHAR
	SbieApi_MonitorPutMsg(MONITOR_OTHER | MONITOR_TRACE, strW);

    __sys_OutputDebugStringA(strA);
}


//---------------------------------------------------------------------------
// Trace_FindModuleByAddress
//---------------------------------------------------------------------------

#include "../../common/my_xeb.h"

WCHAR* Trace_FindModuleByAddress(void* address)
{
    WCHAR* found = NULL;

    PLIST_ENTRY Head, Next;
    PLDR_DATA_TABLE_ENTRY Entry;
    PPEB peb = (PPEB)NtCurrentPeb();

    EnterCriticalSection((PRTL_CRITICAL_SECTION)peb->LoaderLock);

    Head = &peb->Ldr->InLoadOrderModuleList;
    Next = Head->Flink;

    while (Next != Head)
    {
        Entry = CONTAINING_RECORD(Next, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

        if (Entry->DllBase <= address && (UINT_PTR)Entry->DllBase + Entry->SizeOfImage > (UINT_PTR)address)
        {
            found = Entry->BaseDllName.Buffer;
            break;
        }
        Next = Next->Flink;
    }

    LeaveCriticalSection((PRTL_CRITICAL_SECTION)peb->LoaderLock);

    return found;
}

BOOLEAN Trace_FindExportByAddress(void* address, WCHAR** pModule, char** pExport, void** pAddress)
{
    PLIST_ENTRY Head, Next;
    PLDR_DATA_TABLE_ENTRY Entry;
    PPEB peb = (PPEB)NtCurrentPeb();

    EnterCriticalSection((PRTL_CRITICAL_SECTION)peb->LoaderLock);

    Head = &peb->Ldr->InLoadOrderModuleList;
    Next = Head->Flink;

    while (Next != Head)
    {
        Entry = CONTAINING_RECORD(Next, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
        UINT_PTR moduleBase = (UINT_PTR)Entry->DllBase;
        UINT_PTR moduleEnd  = moduleBase + Entry->SizeOfImage;

        if ((UINT_PTR)address >= moduleBase && (UINT_PTR)address < moduleEnd)
        {
            PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)Entry->DllBase;
            if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
                goto nextModule;

            PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(moduleBase + dosHeader->e_lfanew);
            if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
                goto nextModule;

            IMAGE_DATA_DIRECTORY exportDirData = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
            if (exportDirData.VirtualAddress == 0)
                goto nextModule; 

            PIMAGE_EXPORT_DIRECTORY exportDir = (PIMAGE_EXPORT_DIRECTORY)(moduleBase + exportDirData.VirtualAddress);
            DWORD* functions = (DWORD*)(moduleBase + exportDir->AddressOfFunctions);
            DWORD* names     = (DWORD*)(moduleBase + exportDir->AddressOfNames);
            WORD*  ordinals  = (WORD*)(moduleBase + exportDir->AddressOfNameOrdinals);
            DWORD  numNames  = exportDir->NumberOfNames;

            UINT_PTR bestFuncAddr = 0;
            char* bestExportName = NULL;

            // First pass: find the export with the highest address that is <= given address.
            for (DWORD i = 0; i < numNames; i++)
            {
                DWORD funcRVA = functions[ordinals[i]];
                UINT_PTR funcAddr = moduleBase + funcRVA;
                if (funcAddr <= (UINT_PTR)address && funcAddr > bestFuncAddr)
                {
                    bestFuncAddr = funcAddr;
                    bestExportName = (char*)(moduleBase + names[i]);
                }
            }

            // Second pass: find the next function address after bestFuncAddr.
            UINT_PTR nextFuncAddr = (UINT_PTR)-1;
            for (DWORD i = 0; i < numNames; i++)
            {
                DWORD funcRVA = functions[ordinals[i]];
                UINT_PTR funcAddr = moduleBase + funcRVA;
                if (funcAddr > bestFuncAddr && funcAddr < nextFuncAddr)
                {
                    nextFuncAddr = funcAddr;
                }
            }

            // Tolerant check: if the given address is before the next export,
            // then we consider it part of the best matched function.
            if (bestExportName && ((UINT_PTR)address < nextFuncAddr))
            {
                LeaveCriticalSection((PRTL_CRITICAL_SECTION)peb->LoaderLock);

                if (pModule) *pModule = Entry->BaseDllName.Buffer;
                if (pExport) *pExport = bestExportName;
                if (pAddress) *pAddress = (void*)bestFuncAddr;
                return TRUE;
            }
        }
nextModule:
        Next = Next->Flink;
    }

    LeaveCriticalSection((PRTL_CRITICAL_SECTION)peb->LoaderLock);

    return FALSE;
}


//---------------------------------------------------------------------------
// SetInstrumentationCallbackHook
//---------------------------------------------------------------------------

typedef void(*CallbackFn)();

typedef struct _PROCESS_INSTRUMENTATION_CALLBACK_INFORMATION
{
    ULONG Version;
    ULONG Reserved;
    CallbackFn Callback;
} PROCESS_INSTRUMENTATION_CALLBACK_INFORMATION;

extern void InstrumentationCallbackAsm(void);

void* __sys_RtlCaptureContext = NULL;

// Code inspired by ScyllaHide - https://github.com/x64dbg/ScyllaHide/blob/master/HookLibrary/HookHelper.cpp
NTSTATUS InstallInstrumentationCallback()
{
    HANDLE ProcessHandle = NtCurrentProcess();

    // Windows 10
    PROCESS_INSTRUMENTATION_CALLBACK_INFORMATION CallbackInfo;
    CallbackInfo.Reserved = 0;
    CallbackInfo.Callback = InstrumentationCallbackAsm; // set NULL to disable
#ifdef _WIN64
    CallbackInfo.Version = 0; // 0 for x64 and ARM64, 1 for x86
#else
    // Windows 7-8.1 do not support x86/WOW64 instrumentation callbacks
    if (Dll_OsBuild < 10041)
        return STATUS_NOT_SUPPORTED;

    // Native x86 instrumentation callbacks don't work correctly
    BOOL Wow64Process = FALSE;
    if (!IsWow64Process(ProcessHandle, &Wow64Process) || !Wow64Process) {
        //CallbackInfo.Version = 1; // Value to use if they did
        return STATUS_NOT_SUPPORTED;
    }

    // WOW64: set the callback pointer in the version field
    CallbackInfo.Version = (ULONG)CallbackInfo.Callback;
#endif

    // Windows 7-8.1 require SE_DEBUG_PRIVILEGE for this to work, even on the current process
    if (Dll_OsBuild < 10041) // todo: use sbie drv or set privilege in compartment type boxes
        return STATUS_PRIVILEGE_NOT_HELD;

#if defined(_M_ARM64) || defined(_M_ARM64EC)
    __sys_RtlCaptureContext = GetProcAddress(Dll_Ntdll, "RtlCaptureContext");
    if (!__sys_RtlCaptureContext)
        return STATUS_NOT_SUPPORTED;
#ifdef _M_ARM64EC
    //__sys_RtlCaptureContext = Hook_GetFFSTarget(__sys_RtlCaptureContext);
    // TODO
    return STATUS_NOT_SUPPORTED;
#endif
#endif

    NTSTATUS status = NtSetInformationProcess(ProcessHandle, ProcessInstrumentationCallback, &CallbackInfo, sizeof(CallbackInfo));

    return status;
}


//---------------------------------------------------------------------------
// InstrumentationTrace
//---------------------------------------------------------------------------


VOID InstrumentationTrace(ULONG_PTR ReturnAddress, NTSTATUS ReturnStatus)
{
#if defined(_M_ARM64) || defined(_M_ARM64EC)
    ULONG_PTR NtFunction = ReturnAddress - 0x04;
#elif _WIN64
    ULONG_PTR NtFunction = ReturnAddress - 0x14;
    //ULONG SystemCallNumber = *(ULONG*)(ReturnAddress - 0x10);
#else
    ULONG_PTR NtFunction = ReturnAddress - 0x0c;
    //ULONG SystemCallNumber = *(ULONG*)(ReturnAddress - 0x0?);
#endif

#ifdef _WIN64
    // Note: this would also be needed in native 32 bit
    extern SBIELOW_DATA* SbieApi_data;
    if (NtFunction == (ULONG_PTR)SbieApi_data->NtDeviceIoControlFile_code)
        return; // this is used by our sys call interface, the driver will log this syscall
#endif
    
    HMODULE aModules[] = { Dll_Ntdll, Dll_Win32u, NULL }; // Note: Dll_Win32u must be last as it might be NULL

    for (HMODULE* ppModule = aModules; *ppModule; ppModule++) {

        HMODULE hModule = *ppModule;

#ifdef _WIN64
        PIMAGE_NT_HEADERS64 pHeader = ((IMAGE_NT_HEADERS64*)((ULONG_PTR)hModule + ((IMAGE_DOS_HEADER*)hModule)->e_lfanew));
#else
        PIMAGE_NT_HEADERS32 pHeader = ((IMAGE_NT_HEADERS32*)((ULONG_PTR)hModule + ((IMAGE_DOS_HEADER*)hModule)->e_lfanew));
#endif

        DWORD uSize = pHeader->OptionalHeader.SizeOfImage;
        if (((ULONG_PTR)hModule <= ReturnAddress && ReturnAddress <= (ULONG_PTR)hModule + uSize))
        {
            LPCSTR FunctionName = NULL;
            ULONG_PTR MatchFunction = 0;

            IMAGE_EXPORT_DIRECTORY* EXPORT = (IMAGE_EXPORT_DIRECTORY*)((ULONG_PTR)hModule + pHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
            for (DWORD i = 0; i < EXPORT->NumberOfNames; i++)
            {
                WORD Index = *(WORD*)((ULONG_PTR)hModule + EXPORT->AddressOfNameOrdinals + i * 2);
                ULONG_PTR Function = (ULONG_PTR)hModule + *(DWORD*)((ULONG_PTR)hModule + EXPORT->AddressOfFunctions + Index * 4);
                if (NtFunction == Function || (ReturnAddress - Function < ReturnAddress - MatchFunction)) {
                    MatchFunction = Function;
                    FunctionName = (LPCSTR)((ULONG_PTR)hModule + *(DWORD*)((ULONG_PTR)hModule + EXPORT->AddressOfNames + i * 4));
                    if(NtFunction == Function)
                        break;
                }
            }

            // Skip "Nt" Prefix
            if (FunctionName && _strnicmp(FunctionName, "Nt", 2) == 0)
                FunctionName += 2;

            WCHAR trace_str[128];
            ULONG len = Sbie_snwprintf(trace_str, 128, L"%S%S%cstatus = 0x%X", 
                FunctionName ? FunctionName : "Unknown Function", 
                NtFunction == MatchFunction ? "" : "(?)",
                '\0',
                ReturnStatus);
            SbieApi_MonitorPut2Ex(MONITOR_SYSCALL | MONITOR_TRACE, len, trace_str, FALSE, FALSE);

            //DbgPrint("[TRACKING] %s Called\n", FunctionName ? FunctionName : "Unknown Function");
            break;
        }
    }
}


//---------------------------------------------------------------------------
// InstrumentationCallback
//---------------------------------------------------------------------------


void InstrumentationCallback(
#ifdef _WIN64
    PCONTEXT ctx,
#endif
#if defined(_M_ARM64) //|| defined(_M_ARM64EC)
    ULONG_PTR lr,
#endif
    ULONG_PTR ReturnAddress,
    ULONG_PTR ReturnValue)
{
    TEB* pTEB = NtCurrentTeb();

#if defined(_M_ARM64) //|| defined(_M_ARM64EC)
    ctx->Pc = pTEB->InstrumentationCallbackPreviousPc;
    ctx->Sp = pTEB->InstrumentationCallbackPreviousSp;
    ctx->Lr = lr;
    ctx->X0 = ReturnValue;
#elif _M_X64
    ctx->Rip = pTEB->InstrumentationCallbackPreviousPc;
    ctx->Rsp = pTEB->InstrumentationCallbackPreviousSp;
    ctx->Rcx = ctx->R10;
    ctx->R10 = ctx->Rip;
#endif

    // Prevent recursion
    if (!pTEB->InstrumentationCallbackDisabled) {
        pTEB->InstrumentationCallbackDisabled = 1;

        InstrumentationTrace(ReturnAddress, (NTSTATUS)ReturnValue);

        pTEB->InstrumentationCallbackDisabled = 0;
    }

#ifdef _WIN64
    RtlRestoreContext(ctx, NULL);
#endif
}

#if 0
ULONG_PTR NTAPI InstrumentationCallback(
    _In_ ULONG_PTR ReturnAddress, // ECX/R10
    _Inout_ ULONG_PTR ReturnVal) // EAX/RAX
{
    // code
    return ReturnVal;
}
#endif


//---------------------------------------------------------------------------
// ApiInstrumentation
//---------------------------------------------------------------------------

#ifdef _WIN64
void ApiInstrumentation(const char* pName, void** pStack)
#else
void __fastcall ApiInstrumentation(const char* pName, void** pStack)
#endif
{
    void* ReturnAddress = *(pStack - 1);

    TEB* pTEB = NtCurrentTeb();
    ULONG_PTR* sbie_deph = (ULONG_PTR*)&pTEB->ReservedForDebuggerInstrumentation[15];
    if (*sbie_deph)
        return;
    *sbie_deph += 1;

    WCHAR trace_str[256];
    //ULONG len = Sbie_snwprintf(trace_str, 128, L"%S <-- %s%S", pName, CallingModule, pCaller ? pCaller : "");
    ULONG len = Sbie_snwprintf(trace_str, 128, L"%S", pName);
    SbieApi_MonitorPut2Ex(MONITOR_APICALL | MONITOR_TRACE, len, trace_str, FALSE, FALSE);

    *sbie_deph -= 1;
}


//---------------------------------------------------------------------------
// BufferToHexW
//---------------------------------------------------------------------------


void BufferToHexW(const void* lpBuffer, size_t nSize, wchar_t* outBuf, size_t outBufSize)
{
    const unsigned char* p = (const unsigned char*)lpBuffer;
    size_t offset = 0;

    for (size_t i = 0; i < nSize && offset + 4 < outBufSize; ++i)
    {
        int written = Sbie_snwprintf(outBuf + offset, outBufSize - offset, L"%02X", p[i]);
        if (written <= 0 || (size_t)written >= outBufSize - offset)
            break;
        offset += written;
    }

    outBuf[offset] = L'\0';
}

//---------------------------------------------------------------------------
// Trace_SbieDrvFunc2Str
//---------------------------------------------------------------------------

#include "core/drv/api_defs.h"

const wchar_t* Trace_SbieDrvFunc2Str(ULONG func)
{
    switch (func)
    {
        case API_GET_VERSION:                   return L"API_GET_VERSION";
        case API_LOG_MESSAGE:                   return L"API_LOG_MESSAGE";
        case API_QUERY_PROCESS:                 return L"API_QUERY_PROCESS";
        case API_QUERY_BOX_PATH:                return L"API_QUERY_BOX_PATH";
        case API_QUERY_PROCESS_PATH:            return L"API_QUERY_PROCESS_PATH";
        case API_QUERY_PATH_LIST:               return L"API_QUERY_PATH_LIST";
        case API_ENUM_PROCESSES:                return L"API_ENUM_PROCESSES";
        case API_DISABLE_FORCE_PROCESS:         return L"API_DISABLE_FORCE_PROCESS";
        case API_QUERY_CONF:                    return L"API_QUERY_CONF";
        case API_RELOAD_CONF:                   return L"API_RELOAD_CONF";
        case API_CREATE_DIR_OR_LINK:            return L"API_CREATE_DIR_OR_LINK";
        case API_DUPLICATE_OBJECT:              return L"API_DUPLICATE_OBJECT";
        case API_RENAME_FILE:                   return L"API_RENAME_FILE";
        case API_SET_USER_NAME:                 return L"API_SET_USER_NAME";
        case API_INIT_GUI:                      return L"API_INIT_GUI";
        case API_UNLOAD_DRIVER:                 return L"API_UNLOAD_DRIVER";
        case API_MONITOR_CONTROL:               return L"API_MONITOR_CONTROL";
        case API_GET_UNMOUNT_HIVE:              return L"API_GET_UNMOUNT_HIVE";
        case API_GET_FILE_NAME:                 return L"API_GET_FILE_NAME";
        case API_REFRESH_FILE_PATH_LIST:        return L"API_REFRESH_FILE_PATH_LIST";
        case API_SET_LSA_AUTH_PKG:              return L"API_SET_LSA_AUTH_PKG";
        case API_OPEN_FILE:                     return L"API_OPEN_FILE";
        case API_START_PROCESS:                 return L"API_START_PROCESS";
        case API_CHECK_INTERNET_ACCESS:         return L"API_CHECK_INTERNET_ACCESS";
        case API_GET_HOME_PATH:                 return L"API_GET_HOME_PATH";
        case API_GET_BLOCKED_DLL:               return L"API_GET_BLOCKED_DLL";
        case API_QUERY_LICENSE:                 return L"API_QUERY_LICENSE";
        case API_OPEN_DEVICE_MAP:               return L"API_OPEN_DEVICE_MAP";
        case API_OPEN_PROCESS:                  return L"API_OPEN_PROCESS";
        case API_QUERY_PROCESS_INFO:            return L"API_QUERY_PROCESS_INFO";
        case API_IS_BOX_ENABLED:                return L"API_IS_BOX_ENABLED";
        case API_SESSION_LEADER:                return L"API_SESSION_LEADER";
        case API_QUERY_SYMBOLIC_LINK:           return L"API_QUERY_SYMBOLIC_LINK";
        case API_OPEN_KEY:                      return L"API_OPEN_KEY";
        case API_SET_LOW_LABEL_KEY:             return L"API_SET_LOW_LABEL_KEY";
        case API_SET_SERVICE_PORT:              return L"API_SET_SERVICE_PORT";
        case API_INJECT_COMPLETE:               return L"API_INJECT_COMPLETE";
        case API_QUERY_SYSCALLS:                return L"API_QUERY_SYSCALLS";
        case API_INVOKE_SYSCALL:                return L"API_INVOKE_SYSCALL";
        case API_GUI_CLIPBOARD:                 return L"API_GUI_CLIPBOARD";
        case API_RELOAD_CONF2:                  return L"API_RELOAD_CONF2";
        case API_MONITOR_PUT2:                  return L"API_MONITOR_PUT2";
        case API_MONITOR_GET_EX:                return L"API_MONITOR_GET_EX";
        case API_GET_MESSAGE:                   return L"API_GET_MESSAGE";
        case API_PROCESS_EXEMPTION_CONTROL:     return L"API_PROCESS_EXEMPTION_CONTROL";
        case API_GET_DYNAMIC_PORT_FROM_PID:     return L"API_GET_DYNAMIC_PORT_FROM_PID";
        case API_OPEN_DYNAMIC_PORT:             return L"API_OPEN_DYNAMIC_PORT";
        case API_QUERY_DRIVER_INFO:             return L"API_QUERY_DRIVER_INFO";
        default:                                return L"API_UNKNOWN";
    }
}


//---------------------------------------------------------------------------
// Trace_SbieSvcFunc2Str
//---------------------------------------------------------------------------

#include "core/svc/msgids.h"

const wchar_t* Trace_SbieSvcFunc2Str(ULONG func)
{
    switch (func)
    {
        case MSGID_PSTORE_GET_TYPE_INFO:        return L"MSGID_PSTORE_GET_TYPE_INFO";
        case MSGID_PSTORE_GET_SUBTYPE_INFO:     return L"MSGID_PSTORE_GET_SUBTYPE_INFO";
        case MSGID_PSTORE_READ_ITEM:            return L"MSGID_PSTORE_READ_ITEM";
        case MSGID_PSTORE_ENUM_TYPES:           return L"MSGID_PSTORE_ENUM_TYPES";
        case MSGID_PSTORE_ENUM_ITEMS:           return L"MSGID_PSTORE_ENUM_ITEMS";

        case MSGID_PROCESS_CHECK_INIT_COMPLETE: return L"MSGID_PROCESS_CHECK_INIT_COMPLETE";
        case MSGID_PROCESS_GET_WORK_DEPRECATED: return L"MSGID_PROCESS_GET_WORK_DEPRECATED";
        case MSGID_PROCESS_KILL_ONE:            return L"MSGID_PROCESS_KILL_ONE";
        case MSGID_PROCESS_KILL_ALL:            return L"MSGID_PROCESS_KILL_ALL";
        case MSGID_PROCESS_RUN_SANDBOXED:       return L"MSGID_PROCESS_RUN_SANDBOXED";
        case MSGID_PROCESS_SET_DEVICE_MAP:      return L"MSGID_PROCESS_SET_DEVICE_MAP";
        case MSGID_PROCESS_OPEN_DEVICE_MAP:     return L"MSGID_PROCESS_OPEN_DEVICE_MAP";

        case MSGID_SERVICE_START:               return L"MSGID_SERVICE_START";
        case MSGID_SERVICE_QUERY:               return L"MSGID_SERVICE_QUERY";
        case MSGID_SERVICE_LIST:                return L"MSGID_SERVICE_LIST";
        case MSGID_SERVICE_RUN:                 return L"MSGID_SERVICE_RUN";
        case MSGID_SERVICE_UAC:                 return L"MSGID_SERVICE_UAC";

        case MSGID_TERMINAL_QUERY_INFO:         return L"MSGID_TERMINAL_QUERY_INFO";
        case MSGID_TERMINAL_CHECK_TYPE:         return L"MSGID_TERMINAL_CHECK_TYPE";
        case MSGID_TERMINAL_GET_NAME:           return L"MSGID_TERMINAL_GET_NAME";
        case MSGID_TERMINAL_GET_PROPERTY:       return L"MSGID_TERMINAL_GET_PROPERTY";
        case MSGID_TERMINAL_DISCONNECT:         return L"MSGID_TERMINAL_DISCONNECT";
        case MSGID_TERMINAL_GET_USER_TOKEN:     return L"MSGID_TERMINAL_GET_USER_TOKEN";

        case MSGID_NAMED_PIPE_OPEN:             return L"MSGID_NAMED_PIPE_OPEN";
        case MSGID_NAMED_PIPE_CLOSE:            return L"MSGID_NAMED_PIPE_CLOSE";
        case MSGID_NAMED_PIPE_SET:              return L"MSGID_NAMED_PIPE_SET";
        case MSGID_NAMED_PIPE_READ:             return L"MSGID_NAMED_PIPE_READ";
        case MSGID_NAMED_PIPE_WRITE:            return L"MSGID_NAMED_PIPE_WRITE";
        case MSGID_NAMED_PIPE_LPC_CONNECT:      return L"MSGID_NAMED_PIPE_LPC_CONNECT";
        case MSGID_NAMED_PIPE_LPC_REQUEST:      return L"MSGID_NAMED_PIPE_LPC_REQUEST";
        case MSGID_NAMED_PIPE_ALPC_REQUEST:     return L"MSGID_NAMED_PIPE_ALPC_REQUEST";
        case MSGID_NAMED_PIPE_NOTIFICATION:     return L"MSGID_NAMED_PIPE_NOTIFICATION";

        case MSGID_FILE_SET_ATTRIBUTES:         return L"MSGID_FILE_SET_ATTRIBUTES";
        case MSGID_FILE_SET_SHORT_NAME:         return L"MSGID_FILE_SET_SHORT_NAME";
        case MSGID_FILE_LOAD_KEY:               return L"MSGID_FILE_LOAD_KEY";
        case MSGID_FILE_GET_ALL_HANDLES:        return L"MSGID_FILE_GET_ALL_HANDLES";
        case MSGID_FILE_IMPERSONATION_REQUESTS: return L"MSGID_FILE_IMPERSONATION_REQUESTS";
        case MSGID_FILE_SET_REPARSE_POINT:      return L"MSGID_FILE_SET_REPARSE_POINT";
        case MSGID_FILE_OPEN_WOW64_KEY:         return L"MSGID_FILE_OPEN_WOW64_KEY";
        case MSGID_FILE_CHECK_KEY_EXISTS:       return L"MSGID_FILE_CHECK_KEY_EXISTS";

        case MSGID_SBIE_INI_GET_USER:           return L"MSGID_SBIE_INI_GET_USER";
        case MSGID_SBIE_INI_GET_PATH:           return L"MSGID_SBIE_INI_GET_PATH";
        case MSGID_SBIE_INI_TEMPLATE:           return L"MSGID_SBIE_INI_TEMPLATE";
        case MSGID_SBIE_INI_SET_PASSWORD:       return L"MSGID_SBIE_INI_SET_PASSWORD";
        case MSGID_SBIE_INI_TEST_PASSWORD:      return L"MSGID_SBIE_INI_TEST_PASSWORD";
        case MSGID_SBIE_INI_GET_SETTING:        return L"MSGID_SBIE_INI_GET_SETTING";
        case MSGID_SBIE_INI_SET_SETTING:        return L"MSGID_SBIE_INI_SET_SETTING";
        case MSGID_SBIE_INI_ADD_SETTING:        return L"MSGID_SBIE_INI_ADD_SETTING";
        case MSGID_SBIE_INI_INS_SETTING:        return L"MSGID_SBIE_INI_INS_SETTING";
        case MSGID_SBIE_INI_DEL_SETTING:        return L"MSGID_SBIE_INI_DEL_SETTING";
        case MSGID_SBIE_INI_GET_VERSION:        return L"MSGID_SBIE_INI_GET_VERSION";
        case MSGID_SBIE_INI_GET_WAIT_HANDLE:    return L"MSGID_SBIE_INI_GET_WAIT_HANDLE";
        case MSGID_SBIE_INI_RUN_SBIE_CTRL:      return L"MSGID_SBIE_INI_RUN_SBIE_CTRL";

        case MSGID_NETAPI_USE_ADD:              return L"MSGID_NETAPI_USE_ADD";
//        case MSGID_NETAPI_WKSTA_GET_INFO:       return L"MSGID_NETAPI_WKSTA_GET_INFO";
//        case MSGID_NETAPI_SERVER_GET_INFO:      return L"MSGID_NETAPI_SERVER_GET_INFO";

        case MSGID_COM_GET_CLASS_OBJECT:        return L"MSGID_COM_GET_CLASS_OBJECT";
        case MSGID_COM_CREATE_INSTANCE:         return L"MSGID_COM_CREATE_INSTANCE";
        case MSGID_COM_QUERY_INTERFACE:         return L"MSGID_COM_QUERY_INTERFACE";
        case MSGID_COM_ADD_REF_RELEASE:         return L"MSGID_COM_ADD_REF_RELEASE";
        case MSGID_COM_INVOKE_METHOD:           return L"MSGID_COM_INVOKE_METHOD";
        case MSGID_COM_UNMARSHAL_INTERFACE:     return L"MSGID_COM_UNMARSHAL_INTERFACE";
        case MSGID_COM_MARSHAL_INTERFACE:       return L"MSGID_COM_MARSHAL_INTERFACE";
        case MSGID_COM_QUERY_BLANKET:           return L"MSGID_COM_QUERY_BLANKET";
        case MSGID_COM_SET_BLANKET:             return L"MSGID_COM_SET_BLANKET";
        case MSGID_COM_COPY_PROXY:              return L"MSGID_COM_COPY_PROXY";
        case MSGID_COM_CRYPT_PROTECT_DATA:      return L"MSGID_COM_CRYPT_PROTECT_DATA";
        case MSGID_COM_NOTIFICATION:            return L"MSGID_COM_NOTIFICATION";

        case MSGID_IPHLP_CREATE_FILE:           return L"MSGID_IPHLP_CREATE_FILE";
        case MSGID_IPHLP_CLOSE_HANDLE:          return L"MSGID_IPHLP_CLOSE_HANDLE";
        case MSGID_IPHLP_SEND_ECHO:             return L"MSGID_IPHLP_SEND_ECHO";
        case MSGID_IPHLP_NOTIFICATION:          return L"MSGID_IPHLP_NOTIFICATION";

        case MSGID_QUEUE_CREATE:                return L"MSGID_QUEUE_CREATE";
        case MSGID_QUEUE_GETREQ:                return L"MSGID_QUEUE_GETREQ";
        case MSGID_QUEUE_PUTRPL:                return L"MSGID_QUEUE_PUTRPL";
        case MSGID_QUEUE_PUTREQ:                return L"MSGID_QUEUE_PUTREQ";
        case MSGID_QUEUE_GETRPL:                return L"MSGID_QUEUE_GETRPL";
        case MSGID_QUEUE_STARTUP:               return L"MSGID_QUEUE_STARTUP";
        case MSGID_QUEUE_NOTIFICATION:          return L"MSGID_QUEUE_NOTIFICATION";

        case MSGID_EPMAPPER_GET_PORT_NAME:      return L"MSGID_EPMAPPER_GET_PORT_NAME";

        default:                                return L"MSGID_UNKNOWN";
    }
}


//---------------------------------------------------------------------------
// Trace_SbieSvcFunc2Str
//---------------------------------------------------------------------------

#include "core/svc/GuiWire.h"

const wchar_t* Trace_SbieGuiFunc2Str(ULONG func)
{
    switch (func)
    {
        case GUI_INIT_PROCESS:                  return L"GUI_INIT_PROCESS";
        case GUI_GET_WINDOW_STATION:            return L"GUI_GET_WINDOW_STATION";
        case GUI_CREATE_CONSOLE:                return L"GUI_CREATE_CONSOLE";
        case GUI_QUERY_WINDOW:                  return L"GUI_QUERY_WINDOW";
        case GUI_IS_WINDOW:                     return L"GUI_IS_WINDOW";
        case GUI_GET_WINDOW_LONG:               return L"GUI_GET_WINDOW_LONG";
        case GUI_GET_WINDOW_PROP:               return L"GUI_GET_WINDOW_PROP";
        case GUI_GET_WINDOW_HANDLE:             return L"GUI_GET_WINDOW_HANDLE";
        case GUI_GET_CLASS_NAME:                return L"GUI_GET_CLASS_NAME";
        case GUI_GET_WINDOW_RECT:               return L"GUI_GET_WINDOW_RECT";
        case GUI_GET_WINDOW_INFO:               return L"GUI_GET_WINDOW_INFO";
        case GUI_GRANT_HANDLE:                  return L"GUI_GET_WINDOW_INFO";
        case GUI_ENUM_WINDOWS:                  return L"GUI_ENUM_WINDOWS";
        case GUI_FIND_WINDOW:                   return L"GUI_FIND_WINDOW";
        case GUI_MAP_WINDOW_POINTS:             return L"GUI_MAP_WINDOW_POINTS";
        case GUI_SET_WINDOW_POS:                return L"GUI_SET_WINDOW_POS";
        case GUI_CLOSE_CLIPBOARD:               return L"GUI_CLOSE_CLIPBOARD";
        case GUI_GET_CLIPBOARD_DATA:            return L"GUI_GET_CLIPBOARD_DATA";
        case GUI_SEND_POST_MESSAGE:             return L"GUI_SEND_POST_MESSAGE";
        case GUI_SEND_COPYDATA:                 return L"GUI_SEND_COPYDATA";
        case GUI_CLIP_CURSOR:                   return L"GUI_CLIP_CURSOR";
        case GUI_MONITOR_FROM_WINDOW:           return L"GUI_MONITOR_FROM_WINDOW";
        case GUI_SET_FOREGROUND_WINDOW:         return L"GUI_SET_FOREGROUND_WINDOW";
        case GUI_SPLWOW64:                      return L"GUI_SPLWOW64";
        case GUI_CHANGE_DISPLAY_SETTINGS:       return L"GUI_CHANGE_DISPLAY_SETTINGS";
        case GUI_SET_CURSOR_POS:                return L"GUI_SET_CURSOR_POS";
        case GUI_GET_CLIPBOARD_METAFILE:        return L"GUI_GET_CLIPBOARD_METAFILE";
        case GUI_REMOVE_HOST_WINDOW:            return L"GUI_REMOVE_HOST_WINDOW";
        case GUI_GET_RAW_INPUT_DEVICE_INFO:     return L"GUI_GET_RAW_INPUT_DEVICE_INFO";
        case GUI_WND_HOOK_NOTIFY:               return L"GUI_WND_HOOK_NOTIFY";
        case GUI_WND_HOOK_REGISTER:             return L"GUI_WND_HOOK_REGISTER";
        case GUI_KILL_JOB:                      return L"GUI_KILL_JOB";
        default:                                return L"GUI_UNKNOWN";
    }
}