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
// DLL Initialization
//---------------------------------------------------------------------------


#include "dll.h"
#include "core/low/lowdata.h"
#include <stdio.h>


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#ifdef _M_ARM64


#define LDR_INJECT_SETTING_NAME             L"InjectDllARM64"
#define LDR_HOST_INJECT_SETTING_NAME        L"HostInjectDllARM64"
//#define LDR_INJECT_NUM_SAVE_BYTES   16
#define LDR_INJECT_NUM_SAVE_BYTES   20


#elif _WIN64


#define LDR_INJECT_SETTING_NAME             L"InjectDll64"
#define LDR_HOST_INJECT_SETTING_NAME        L"HostInjectDll64"
#define LDR_INJECT_NUM_SAVE_BYTES   12
//#define LDR_INJECT_NUM_SAVE_BYTES   19


#else ! _WIN64


#define LDR_INJECT_SETTING_NAME             L"InjectDll"
#define LDR_HOST_INJECT_SETTING_NAME        L"HostInjectDll"
#define LDR_INJECT_NUM_SAVE_BYTES   5


#endif _WIN64


#define LDRP_STATIC_LINK                        0x00000002
#define LDRP_IMAGE_DLL                          0x00000004
#define LDRP_ENTRY_PROCESSED                    0x00004000
#define LDRP_DONT_CALL_FOR_THREADS              0x00040000
#define LDRP_PROCESS_ATTACH_CALLED              0x00080000

#define LDRP_LOAD_IN_PROGRESS                   0x00001000
#define LDRP_UNLOAD_IN_PROGRESS                 0x00002000
#define LDRP_ENTRY_INSERTED                     0x00008000
#define LDRP_CURRENT_LOAD                       0x00010000
#define LDRP_FAILED_BUILTIN_LOAD                0x00020000
#define LDRP_DEBUG_SYMBOLS_LOADED               0x00100000
#define LDRP_IMAGE_NOT_AT_BASE                  0x00200000
#define LDRP_COR_IMAGE                          0x00400000
#define LDR_COR_OWNS_UNMAP                      0x00800000
#define LDRP_SYSTEM_MAPPED                      0x01000000
#define LDRP_IMAGE_VERIFYING                    0x02000000
#define LDRP_DRIVER_DEPENDENT_DLL               0x04000000
#define LDRP_ENTRY_NATIVE                       0x08000000
#define LDRP_REDIRECTED                         0x10000000
#define LDRP_NON_PAGED_DEBUG_INFO               0x20000000
#define LDRP_MM_LOADED                          0x40000000
#define LDRP_COMPAT_DATABASE_PROCESSED          0x80000000


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _PEB_LDR_DATA {
    UCHAR Reserved1[8];
    ULONG_PTR Reserved2;
    /*
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
    */
    ULONG_PTR InLoadOrder;
    ULONG_PTR InLoadOrder2;
    ULONG_PTR InMemoryOrder;    // --> LDR_MODULE_IN_MEMORY_ORDER
    ULONG_PTR InMemoryOrder2;
    ULONG_PTR InInitOrder;      // --> LDR_MODULE
    ULONG_PTR InInitOrder2;
} PEB_LDR_DATA;


typedef struct _LDR_MODULE {
    ULONG_PTR       Next;
    ULONG_PTR       Prev;
    ULONG_PTR       BaseAddress;
    ULONG_PTR       EntryPoint;
    ULONG           SizeOfImage;
    UNICODE_STRING  FullDllName;
    UNICODE_STRING  BaseDllName;
    ULONG           Flags;
    USHORT          LoadCount;
    USHORT          TlsIndex;
    LIST_ENTRY      HashTableEntry;
    ULONG           TimeDateStamp;
} LDR_MODULE;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


void Ldr_LoadInjectDlls(BOOLEAN bHostInject);

IMAGE_OPTIONAL_HEADER *Ldr_OptionalHeader(ULONG_PTR ImageBase);

static void Ldr_FixImagePath(void);

static WCHAR *Ldr_FixImagePath_2(void);

static void Ldr_FixImagePath_3(
    const WCHAR *ImageDosPath,
    UNICODE_STRING *FullName, UNICODE_STRING *BaseName);

static void Ldr_FixImagePath_DllPath(void);

void Ldr_Inject_Init(BOOLEAN bHostInject);


#ifdef _WIN64
extern void Ldr_Inject_Entry64(void);
#else ! _WIN64
extern void Ldr_Inject_Entry32(void);
#endif _WIN64


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static UCHAR Ldr_Inject_SaveBytes[LDR_INJECT_NUM_SAVE_BYTES];
static ULONG Ldr_Inject_OldProtect = 0;

static const WCHAR *Ldr_InjectDll       = LDR_INJECT_SETTING_NAME;
static const WCHAR *Ldr_HostInjectDll   = LDR_HOST_INJECT_SETTING_NAME;

static ULONG_PTR Ldr_ImageBase = 0;

BOOLEAN Ldr_BoxedImage = FALSE;

WCHAR *Ldr_ImageTruePath = NULL;


//---------------------------------------------------------------------------
// Ldr_LoadInjectDlls
//---------------------------------------------------------------------------


_FX void Ldr_LoadInjectDlls(BOOLEAN bHostInject)
{
    //
    // inject DLLs specified by the InjectDll setting
    // for each DLL, we load it.  if it exports an InjectDllMain
    // procedure, we also call that procedure
    //

    NTSTATUS status;
    HMODULE ModuleHandle;
    UNICODE_STRING DllUni;
    ULONG DllFlags;

    WCHAR *dllname = Dll_AllocTemp(MAX_PATH * 2 * sizeof(WCHAR));
    ULONG index = 0;

    //
    // We also end up here form host injection mode so Dll_HomeDosPath is not available
    //

    WCHAR *path = Dll_AllocTemp(1024 * sizeof(WCHAR));
    SbieApi_GetHomePath(NULL, 0, path, 1020);

    if (!__sys_LdrLoadDll)
        __sys_LdrLoadDll = (P_LdrLoadDll)GetProcAddress(Dll_Ntdll, "LdrLoadDll");

    while (1) {

        status = SbieApi_QueryConfAsIs(
                        NULL, (bHostInject? Ldr_HostInjectDll:Ldr_InjectDll), index,
                        dllname, (MAX_PATH * 2 - 2) * sizeof(WCHAR));
        ++index;
        if (! NT_SUCCESS(status)) {
            if (status == STATUS_BUFFER_TOO_SMALL)
                continue;
            break;
        }

        //
        // For security reasons we do not allow relative paths, or other files then .dll
        //

        wchar_t* ext = wcsrchr(dllname, L'.');
        if (!ext || _wcsicmp(ext, L".dll") != 0 || wcsstr(dllname, L"..") != NULL)
            continue;

		//
		// For expedient use we allow to enter the dll name without a path
		// starting with \ in that case the DLL is looked for in %SbieHome%
		//

        if (dllname[0] == L'\\' && wcslen(path) + wcslen(dllname) + 1 < MAX_PATH * 2)
        {
            wmemmove(dllname + wcslen(path), dllname, wcslen(dllname) + 1);
            wmemcpy(dllname, path, wcslen(path));
        }

        //
        // For security reasons we don't allow HostInjectDll to use an absolute path
        // Dll's to be injected into host processes must be located in sbies install dir
        //

        else if (bHostInject)
            continue;


        //
        // we have to prevent invocation of Ldr_CallDllCallbacks while
        // loading the DLL here, otherwise we will end up our per-dll
        // init functions (e.g. AdvApi_Init) which use GetProcAddress
        // and this messes up the ntdll initial load sequence.
        // we do this by bypassing Ldr_LdrLoadDll
        //

        ModuleHandle = 0;
        DllFlags = 0;
        RtlInitUnicodeString(&DllUni, dllname);
        status = __sys_LdrLoadDll(NULL, &DllFlags, &DllUni, &ModuleHandle);

        if (NT_SUCCESS(status) && ModuleHandle) {

            void *pInjectDllMain = Ldr_GetProcAddr_3(
                    (ULONG_PTR)ModuleHandle, L"InjectDllMain");

            if (! pInjectDllMain) {
                pInjectDllMain = Ldr_GetProcAddr_3(
                    (ULONG_PTR)ModuleHandle, L"_InjectDllMain@8");
            }

            if (pInjectDllMain)
                ProtectCall2(pInjectDllMain, (ULONG_PTR)Dll_Instance, 0);
        }
    }

    Dll_Free(dllname);
    Dll_Free(path);
}


//---------------------------------------------------------------------------
// Ldr_OptionalHeader
//---------------------------------------------------------------------------


_FX IMAGE_OPTIONAL_HEADER *Ldr_OptionalHeader(ULONG_PTR ImageBase)
{
    IMAGE_DOS_HEADER *dos_hdr =
        (IMAGE_DOS_HEADER *)ImageBase;

    IMAGE_NT_HEADERS *nt_hdrs =
        (IMAGE_NT_HEADERS *)(ImageBase + dos_hdr->e_lfanew);

    IMAGE_OPTIONAL_HEADER *opt_hdr =
        &nt_hdrs->OptionalHeader;

    return opt_hdr;
}


//---------------------------------------------------------------------------
// Ldr_MakeStaticDll
//---------------------------------------------------------------------------


_FX BOOLEAN Ldr_MakeStaticDll(ULONG_PTR BaseAddress)
{
    ULONG_PTR InInitOrderList = GET_PEB_LDR_DATA->InInitOrder;
    ULONG_PTR next = ((LDR_MODULE *)InInitOrderList)->Next;
    while (next != InInitOrderList) {
        LDR_MODULE *NtPtr = (LDR_MODULE *)next;
        if (NtPtr->BaseAddress == BaseAddress) {
            NtPtr->LoadCount = 0xFFFF;
            return TRUE;
        }
        next = NtPtr->Next;
    }
    return FALSE;
}


//---------------------------------------------------------------------------
// Ldr_SetDdagState_W8
//---------------------------------------------------------------------------


_FX ULONG Ldr_SetDdagState_W8(ULONG_PTR BaseAddress, ULONG NewState)
{
    //
    // Windows 8 seems to use new node graph structures for managing
    // modules.  the traditional LDR_MODULE entry points to a new
    // LDR_DDAG_NODE structure, which contains an LDR_DDAG_STATE field.
    //
    // State is less than 9 during module initialization, at which time
    // any use of GetModuleHandle or GetProcAddress on the corresponding
    // module will cause ntdll to load dependent DLLs for that module
    // and run their init routines.  this gets in the way of our DLL init
    // routines invoked from Ldr_CallOneDllCallback
    //
    // to work around this, we have to force a value of 9 into that State
    // field before and after running our init routines, see use in
    // Ldr_CallDllCallbacks
    //

#ifdef _WIN64
#define LDR_MODULE_BASE_ADDRESS_OFFSET      (0x30)
#define LDR_MODULE_DDAG_NODE_OFFSET         (0x98)
#define LDR_DDAG_STATE_OFFSET               (0x38)
#else ! _WIN64
#define LDR_MODULE_BASE_ADDRESS_OFFSET      (0x18)
#define LDR_MODULE_DDAG_NODE_OFFSET         (0x50)
#define LDR_DDAG_STATE_OFFSET               (0x20)
#endif _WIN64

    if (Dll_OsBuild >= 8400) {

        ULONG_PTR InLoadOrderList = GET_PEB_LDR_DATA->InLoadOrder;
        ULONG_PTR next = ((LDR_MODULE *)InLoadOrderList)->Next;
        while (next != InLoadOrderList) {

            ULONG_PTR LdrModuleBaseAddress =
                *(ULONG_PTR *)(next + LDR_MODULE_BASE_ADDRESS_OFFSET);

            if (LdrModuleBaseAddress == BaseAddress) {

                ULONG_PTR DdagNode =
                    *(ULONG_PTR *)(next + LDR_MODULE_DDAG_NODE_OFFSET);

                ULONG OldState =
                    *(ULONG *)(DdagNode + LDR_DDAG_STATE_OFFSET);

                *(ULONG *)(DdagNode + LDR_DDAG_STATE_OFFSET) = NewState;

                return OldState;
            }

            next = ((LDR_MODULE *)next)->Next;
        }
    }

    return 0;
}


//---------------------------------------------------------------------------
// Ldr_LoadAndFix_IsClosedFilePath
//---------------------------------------------------------------------------


_FX BOOLEAN Ldr_LoadAndFix_IsClosedFilePath(const WCHAR *DllName)
{
    WCHAR *ptr;
    ULONG mp_flags;

    //
    // check if dll name matches any ClosedFilePath settings
    // if no match, retry check with x\name.dll, so that we can
    // match ClosedFilePath=*\name.dll settings
    //

    mp_flags = SbieDll_MatchPath(L'f', DllName);
    if (PATH_IS_CLOSED(mp_flags))
        return TRUE;

    ptr = wcschr(DllName, L'\\');
    if (ptr)
        return FALSE;

    ptr = Dll_AllocTemp((wcslen(DllName) + 8) * sizeof(WCHAR));
    wcscpy(ptr, L"X\\");
    wcscat(ptr, DllName);
    mp_flags = SbieDll_MatchPath(L'f', ptr);
    Dll_Free(ptr);
    if (PATH_IS_CLOSED(mp_flags))
        return TRUE;

    return FALSE;
}


//---------------------------------------------------------------------------
// Ldr_FixImagePath
//---------------------------------------------------------------------------


_FX void Ldr_FixImagePath(void)
{
    //
    // first LDR entry in memory order represents the loaded image
    //

    typedef struct _LDR_MODULE_IN_MEMORY_ORDER {
        ULONG_PTR Next;
        ULONG_PTR Prev;
        LDR_MODULE LdrModule;
    } LDR_MODULE_IN_MEMORY_ORDER;

    LDR_MODULE_IN_MEMORY_ORDER *LdrModuleInMemoryOrder =
        (LDR_MODULE_IN_MEMORY_ORDER *)(GET_PEB_LDR_DATA->InMemoryOrder);

    LDR_MODULE *LdrModule = &LdrModuleInMemoryOrder->LdrModule;

    if (LdrModule->BaseAddress == Ldr_ImageBase) {

        WCHAR *KernelPath, *TruePathForBoxedPath;

        UNICODE_STRING *FullName = &LdrModule->FullDllName;
        UNICODE_STRING *BaseName = &LdrModule->BaseDllName;

        //
        // if this is a forced program, we need to get the image path from
        // the kernel (i.e. NtQueryVirtualMemory with MemoryMappedFilenameInformation),
        // in case the image path contains symbolic links or reparse points.
        // such links would have been translated by NtCreateFile if the
        // program was started by another program in the sandbox.  so if
        // this is a forced program, we want to have the same consistency,
        // by asking the kernel for a fully reparsed path
        //

        if (Dll_ProcessFlags & SBIE_FLAG_FORCED_PROCESS)
            KernelPath = Ldr_FixImagePath_2();
        else
            KernelPath = NULL;

        //
        // check if the image path is inside the sandbox.  if this isn't a
        // forced program, we use the module path recorded by NTDLL
        //

        TruePathForBoxedPath = File_GetTruePathForBoxedPath(
                (KernelPath ? KernelPath : FullName->Buffer), TRUE);

        if (TruePathForBoxedPath) {

            Ldr_FixImagePath_3(TruePathForBoxedPath, FullName, BaseName);

            Dll_Free(TruePathForBoxedPath);

            Ldr_BoxedImage = TRUE;

        } else if (KernelPath &&
                        _wcsicmp(KernelPath, FullName->Buffer) != 0) {

            //
            // if we have a path from the kernel (i.e. forced program)
            // which is different from the path stored by NTDLL, then
            // this is probably due to reparse points, and we want to
            // replace the stored path
            //

            Ldr_FixImagePath_3(KernelPath, FullName, BaseName);

        } else {

            Ldr_ImageTruePath = File_TranslateDosToNtPath(FullName->Buffer);
        }

        if (KernelPath)
            Dll_Free(KernelPath);
    }

    Ldr_FixImagePath_DllPath();
}


//---------------------------------------------------------------------------
// Ldr_FixImagePath_2
//---------------------------------------------------------------------------


_FX WCHAR *Ldr_FixImagePath_2(void)
{
    UNICODE_STRING *NameUni;
    //SIZE_T BufferLength;
    ULONG BufferLength;
    NTSTATUS status;

    //
    // Windows is caching loaded modules, when after being run a binary is moved
    // and run again, NtQueryVirtualMemory will return the original location
    // and not the valid up to date current location.
    // Hence we use NtQueryInformationProcess instead it also returns the reparsed path
    //

    //extern P_NtQueryVirtualMemory __sys_NtQueryVirtualMemory;
    //if (! __sys_NtQueryVirtualMemory)
    extern P_NtQueryInformationProcess __sys_NtQueryInformationProcess;
    if (! __sys_NtQueryInformationProcess)
        return NULL;

    BufferLength = 256;
    NameUni = Dll_AllocTemp((ULONG)BufferLength + sizeof(WCHAR) * 2);
    //status = __sys_NtQueryVirtualMemory(
    //    NtCurrentProcess(), (void *)Ldr_ImageBase, MemoryMappedFilenameInformation,
    //    NameUni, BufferLength, &BufferLength);
    status = __sys_NtQueryInformationProcess(
        NtCurrentProcess(), ProcessImageFileName,
        NameUni, BufferLength, &BufferLength);

    if (status == STATUS_INFO_LENGTH_MISMATCH ||
        status == STATUS_BUFFER_OVERFLOW) {

        Dll_Free(NameUni);
        NameUni = Dll_AllocTemp((ULONG)BufferLength + sizeof(WCHAR) * 2);
        //status = __sys_NtQueryVirtualMemory(
        //    NtCurrentProcess(), (void *)Ldr_ImageBase, MemoryMappedFilenameInformation,
        //    NameUni, BufferLength, &BufferLength);
        status = __sys_NtQueryInformationProcess(
            NtCurrentProcess(), ProcessImageFileName,
            NameUni, BufferLength, &BufferLength);
    }

    if (! NT_SUCCESS(status)) {
        Dll_Free(NameUni);
        return NULL;
    }

    NameUni->Buffer[NameUni->Length / sizeof(WCHAR)] = L'\0';
    if (! SbieDll_TranslateNtToDosPath(NameUni->Buffer)) {
        Dll_Free(NameUni);
        return NULL;
    }

    wmemmove((WCHAR *)NameUni, NameUni->Buffer, wcslen(NameUni->Buffer) + 1);
    return (WCHAR *)NameUni;
}


//---------------------------------------------------------------------------
// Ldr_FixImagePath_3
//---------------------------------------------------------------------------


_FX void Ldr_FixImagePath_3(
    const WCHAR *ImageDosPath,
    UNICODE_STRING *FullName, UNICODE_STRING *BaseName)
{
    WCHAR *Backslash;

    //
    // update FullName/BaseName in LDR entry for first load module
    //

    ULONG len1 = wcslen(ImageDosPath) * sizeof(WCHAR);
    ULONG len2 = len1 + sizeof(WCHAR);
    WCHAR *buf = (WCHAR *)HeapAlloc(
                    GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, len2);
    memcpy(buf, ImageDosPath, len2);

    FullName->Length        = (USHORT)len1;
    FullName->MaximumLength = (USHORT)len2;
    FullName->Buffer        = buf;

    Backslash = wcsrchr(buf, L'\\');
    if (Backslash)
        RtlInitUnicodeString(BaseName, Backslash + 1);
    else
        *BaseName = *FullName;

    //
    // update ImagePathName in RTL_USER_PROCESS_PARAMETERS
    //

    if (1) {

        RTL_USER_PROCESS_PARAMETERS *ProcessParms =
                                        Proc_GetRtlUserProcessParameters();
        if (ProcessParms) {

            buf = (WCHAR *)HeapAlloc(
                        GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, len2);
            memcpy(buf, ImageDosPath, len2);

            ProcessParms->ImagePathName.Length        = (USHORT)len1;
            ProcessParms->ImagePathName.MaximumLength = (USHORT)len2;
            ProcessParms->ImagePathName.Buffer        = buf;
        }
    }

    //
    // keep a copy of the NT form of the true path to the image
    //

    Ldr_ImageTruePath = File_TranslateDosToNtPath(ImageDosPath);
}


//---------------------------------------------------------------------------
// Ldr_FixImagePath_DllPath
//---------------------------------------------------------------------------


_FX void Ldr_FixImagePath_DllPath(void)
{
    //
    // if the first entry in the DllPath for the process starts with the
    // sandbox prefix, then we need to remove this prefix to prevent NTDLL
    // from associating module path C:\Sandbox\drive\c\test.dll rather than
    // just c:\test.dll during the initial load of static DLLs
    //

    RTL_USER_PROCESS_PARAMETERS *ProcessParms =
                                    Proc_GetRtlUserProcessParameters();
    if (ProcessParms) {

        WCHAR *DllPath_Buf = ProcessParms->DllPath.Buffer;
        ULONG DllPath_Len = ProcessParms->DllPath.Length / sizeof(WCHAR);

        ULONG DosFilePath_Len;
        WCHAR *DosFilePath =
                    Dll_AllocTemp((Dll_BoxFilePathLen + 64) * sizeof(WCHAR));
        wmemcpy(DosFilePath, Dll_BoxFilePath, Dll_BoxFilePathLen + 1);

        if (SbieDll_TranslateNtToDosPath(DosFilePath))
            DosFilePath_Len = wcslen(DosFilePath);
        else
            DosFilePath_Len = 0;

        if (DosFilePath_Len && DllPath_Buf && DllPath_Len > DosFilePath_Len
            && _wcsnicmp(DllPath_Buf, DosFilePath, DosFilePath_Len) == 0
            && DllPath_Buf[DosFilePath_Len] == L'\\') {

            //
            // the dll path starts with the sandbox prefix,
            // so extract the first component
            //

            ULONG Prefix_Len;
            WCHAR *Prefix_Buf;
            WCHAR *TruePath;

            WCHAR *EndPtr = wcschr(DllPath_Buf, L';');
            if (EndPtr)
                Prefix_Len = (ULONG)(EndPtr - DllPath_Buf);
            else {
                Prefix_Len = DllPath_Len;
                EndPtr = DllPath_Buf + DllPath_Len;
            }

            //
            // copy the first component into a new buffer
            // and get the true path for it
            //

            Prefix_Buf = Dll_AllocTemp((Prefix_Len + 1) * sizeof(WCHAR));
            wmemcpy(Prefix_Buf, DllPath_Buf, Prefix_Len);
            Prefix_Buf[Prefix_Len] = L'\0';

            TruePath = File_GetTruePathForBoxedPath(Prefix_Buf, TRUE);
            if (TruePath) {

                ULONG TruePath_Len = wcslen(TruePath);
                ULONG Suffix_Len = DllPath_Len + 1 - Prefix_Len;
                ULONG NewPath_Len = TruePath_Len + Suffix_Len;

                WCHAR *NewPath;

                if (NewPath_Len <= DllPath_Len) {

                    // new dll path fits in the original dll path buffer

                    NewPath = DllPath_Buf;

                } else {

                    // need to allocate a larger dll path buffer

                    NewPath = HeapAlloc(GetProcessHeap(), 0,
                                    (NewPath_Len + 1) * sizeof(WCHAR));
                }

                if (NewPath) {

                    wmemmove(NewPath + TruePath_Len,
                             DllPath_Buf + Prefix_Len,
                             Suffix_Len);

                    wmemcpy(NewPath, TruePath, TruePath_Len);

                    RtlInitUnicodeString(&ProcessParms->DllPath, NewPath);
                }

                Dll_Free(TruePath);
            }

            Dll_Free(Prefix_Buf);
        }

        Dll_Free(DosFilePath);
    }
}


//---------------------------------------------------------------------------
// Ldr_Inject_Init
//---------------------------------------------------------------------------
BOOLEAN g_bHostInject = FALSE;
ULONG_PTR g_entrypoint = 0;

_FX void Ldr_Inject_Init(BOOLEAN bHostInject)
{
    IMAGE_OPTIONAL_HEADER *opt_hdr;
    UCHAR *entrypoint;

    Ldr_ImageBase = (ULONG_PTR)GetModuleHandle(NULL);
    opt_hdr = Ldr_OptionalHeader(Ldr_ImageBase);
    g_bHostInject = bHostInject;

    //
    // if the image is in the sandbox, adjust its LDR entry
    //
    if (!bHostInject)
        Ldr_FixImagePath();

    //
    // inject call to Ldr_Inject_Entry at the image entrypoint
    //
    // note that some .NET programs have a zero entrypoint address
    //

    if (! opt_hdr->AddressOfEntryPoint)
        return;

    entrypoint = (UCHAR *)(Ldr_ImageBase + opt_hdr->AddressOfEntryPoint);
    g_entrypoint = (ULONG_PTR)entrypoint;

    memcpy(Ldr_Inject_SaveBytes, entrypoint, LDR_INJECT_NUM_SAVE_BYTES);

    if (VirtualProtect(entrypoint, LDR_INJECT_NUM_SAVE_BYTES,
                       PAGE_EXECUTE_READWRITE, &Ldr_Inject_OldProtect)) {

#ifdef _M_ARM64

        ULONG* aCode = (ULONG*)entrypoint;
        *aCode++ = 0x10000000;	// adr x0, 0 - copy pc to x0
	    *aCode++ = 0x58000048;	// ldr x8, 8
        *aCode++ = 0xD61F0100;	// br x8
        *(ULONG_PTR*)aCode = (ULONG_PTR)Ldr_Inject_Entry64;

        NtFlushInstructionCache(GetCurrentProcess(), entrypoint, LDR_INJECT_NUM_SAVE_BYTES);

#elif _WIN64

        entrypoint[0] = 0x48;           // mov rax, Ldr_Inject_Entry64
        entrypoint[1] = 0xB8;
        *(ULONG_PTR *)(entrypoint + 2) = (ULONG_PTR)Ldr_Inject_Entry64;

//        entrypoint[10] = 0xFF;          // call rax
//        entrypoint[11] = 0xD0;

        // using 19 bytes breaks Antidote11

        //entrypoint[10] = 0x48;          // lea rcx, [rip - 0x11]
        //entrypoint[11] = 0x8d;
        //entrypoint[12] = 0x0d;
        //*(ULONG*)(entrypoint + 13) = -0x11;
        //
        //entrypoint[17] = 0xFF;          // jmp rax
        //entrypoint[18] = 0xE0;

        entrypoint[10] = 0xFF;          // jmp rax
        entrypoint[11] = 0xE0;

#else ! _WIN64

        *entrypoint = 0xE8;             // call Ldr_Inject_Entry
        *(ULONG_PTR *)(entrypoint + 1) =
            (UCHAR *)Ldr_Inject_Entry32 - (entrypoint + 5);

#endif _WIN64

    }
}


//---------------------------------------------------------------------------
// Ldr_Inject_Entry
//---------------------------------------------------------------------------


//_FX void Ldr_Inject_Entry(ULONG_PTR *pRetAddr)
_FX void* Ldr_Inject_Entry(ULONG_PTR *pPtr)
{
    UCHAR *entrypoint;
    ULONG dummy_prot;

    //
    // restore correct code sequence at the entrypoint
    //

//#ifdef _M_ARM64
//    entrypoint = ((UCHAR *)*pRetAddr) - (LDR_INJECT_NUM_SAVE_BYTES - sizeof(ULONG_PTR)); // after blr comes the 64bit address
//#else
//    entrypoint = ((UCHAR *)*pRetAddr) - LDR_INJECT_NUM_SAVE_BYTES;
//#endif
//    *pRetAddr = (ULONG_PTR)entrypoint;
#ifdef _M_ARM64
    entrypoint = (UCHAR*)pPtr;
#elif _WIN64
    // entrypoint = (UCHAR*)pPtr;
    entrypoint = (UCHAR*)g_entrypoint;
#else // x86
    entrypoint = ((UCHAR *)*pPtr) - LDR_INJECT_NUM_SAVE_BYTES;
    *pPtr = (ULONG_PTR)entrypoint;
#endif

    // If entrypoint hook is different, need to adjust offset. Copying the original byets won't have the correct offset.
    // MS UEV also hooks exe entry.
    if (g_entrypoint != (ULONG_PTR)entrypoint)
    {
#ifdef _WIN64
        // We haven't seen the case yet. MS UEV may or may not have conflicts in this case.
#else
        ULONG_PTR   nDiff = (ULONG_PTR)entrypoint - g_entrypoint;
        ULONG_PTR*  pAddressOrig = (ULONG_PTR*)&Ldr_Inject_SaveBytes[1];
        *pAddressOrig = (*pAddressOrig) - nDiff;
#endif
    }

    VirtualProtect(entrypoint, LDR_INJECT_NUM_SAVE_BYTES,
                   PAGE_EXECUTE_READWRITE, &dummy_prot);

    memcpy(entrypoint, Ldr_Inject_SaveBytes, LDR_INJECT_NUM_SAVE_BYTES);

    VirtualProtect(entrypoint, LDR_INJECT_NUM_SAVE_BYTES,
                   Ldr_Inject_OldProtect, &dummy_prot);

#ifdef _M_ARM64
    NtFlushInstructionCache(GetCurrentProcess(), entrypoint, LDR_INJECT_NUM_SAVE_BYTES);
#endif

    if (!g_bHostInject)
    {

        // if we caused PEB.ReadImageFileExecOptions to be non-zero then restore
        // the zero value here
        if (Dll_OsBuild < 8400) {

            UCHAR *ReadImageFileExecOptions = (UCHAR *)(NtCurrentPeb() + 1);
            if (*ReadImageFileExecOptions == '*')
                *ReadImageFileExecOptions = 0;
        }

        //
        // do some post-LDR initialization
        //

        Ldr_LoadInjectDlls(g_bHostInject);

        Dll_InitExeEntry();
    }
    else
    {
        Ldr_LoadInjectDlls(g_bHostInject);
    }
    
	
    //
    // free the syscall/inject data area which is no longer needed
    //

#ifdef _M_ARM64EC
    extern ULONG* SbieApi_SyscallPtr;
    SbieApi_SyscallPtr = NULL;
#endif
    extern SBIELOW_DATA* SbieApi_data;
    VirtualFree((void*)SbieApi_data->syscall_data, 0, MEM_RELEASE);

    //
    // return original entry point address to jump to
    //

    return entrypoint;
}
