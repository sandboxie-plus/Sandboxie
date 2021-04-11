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
// DLL Load Monitor
//---------------------------------------------------------------------------


#include "dll.h"
#include "core/drv/api_flags.h"

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define LDR_NUM_CALLBACKS 8 

#define LDR_TOKEN_PRIMARY -4
#define LDR_TOKEN_IMPERSONATION -5
#define LDR_TOKEN_EFFECTIVE -6

//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct tagDLL {
    const WCHAR *nameW;
    const char  *nameA;
    BOOLEAN(*init_func)(HMODULE);
    int state;
} DLL;

typedef struct _LDR_DLL_LOADED_NOTIFICATION_DATA {
    ULONG Flags;                    //Reserved.
    PCUNICODE_STRING FullDllName;   //The full path name of the DLL module.
    PCUNICODE_STRING BaseDllName;   //The base file name of the DLL module.
    PVOID DllBase;                  //A pointer to the base address for the DLL in memory.
    ULONG SizeOfImage;              //The size of the DLL image, in bytes.
} LDR_DLL_LOADED_NOTIFICATION_DATA, *PLDR_DLL_LOADED_NOTIFICATION_DATA;

typedef struct _LDR_DLL_UNLOADED_NOTIFICATION_DATA {
    ULONG Flags;                    //Reserved.
    PCUNICODE_STRING FullDllName;   //The full path name of the DLL module.
    PCUNICODE_STRING BaseDllName;   //The base file name of the DLL module.
    PVOID DllBase;                  //A pointer to the base address for the DLL in memory.
    ULONG SizeOfImage;              //The size of the DLL image, in bytes.
} LDR_DLL_UNLOADED_NOTIFICATION_DATA, *PLDR_DLL_UNLOADED_NOTIFICATION_DATA;

typedef union _LDR_DLL_NOTIFICATION_DATA {
    LDR_DLL_LOADED_NOTIFICATION_DATA Loaded;
    LDR_DLL_UNLOADED_NOTIFICATION_DATA Unloaded;
} LDR_DLL_NOTIFICATION_DATA, *PLDR_DLL_NOTIFICATION_DATA;

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static void Ldr_CallOneDllCallback(const UCHAR *ImageNameA, ULONG_PTR ImageBase);
static void Ldr_CallOneDllCallbackXP(const UCHAR *ImageNameA, ULONG_PTR ImageBase);
static void Ldr_CallDllCallbacks(void);

static NTSTATUS Ldr_LdrLoadDll(WCHAR *PathString, ULONG *DllFlags, UNICODE_STRING *ModuleName, HANDLE *ModuleHandle);
static NTSTATUS Ldr_Win10_LdrLoadDll(WCHAR *PathString, ULONG *DllFlags, UNICODE_STRING *ModuleName, HANDLE *ModuleHandle);
static NTSTATUS Ldr_LdrUnloadDll(HANDLE ModuleHandle);

static ULONG_PTR Ldr_LdrResolveDelayLoadedAPI(
    HMODULE ImageBase, ULONG_PTR DelayImportDescriptor,
    ULONG_PTR UnknownParameter1, ULONG_PTR UnknownParameter2,
    ULONG_PTR UnknownParameter3, ULONG_PTR UnknownParameter4);

static NTSTATUS Ldr_LdrQueryImageFileExecutionOptions(
    UNICODE_STRING *SubKey, const WCHAR *ValueName, ULONG Type,
    void *Buffer, ULONG BufferSize, ULONG *ReturnedLength);

static ULONG_PTR Ldr_NtApphelpCacheControl(
    ULONG_PTR Unknown1, ULONG_PTR Unknown2);

void Ldr_MyDllCallbackA(const CHAR *ImageName, HMODULE ImageBase);
void Ldr_MyDllCallbackW(const WCHAR *ImageName, HMODULE ImageBase);
void Ldr_MyDllCallbackNew(const WCHAR *ImageName, HMODULE ImageBase);

static void *Ldr_GetProcAddr_2(const WCHAR *DllName, const WCHAR *ProcName);

static void *Ldr_GetProcAddr_3(ULONG_PTR DllBase, const WCHAR *ProcName);

static NTSTATUS Ldr_NtLoadDriver(UNICODE_STRING *RegistryPath);

static BOOL LdrCheckImmersive();

static NTSTATUS Ldr_LdrRegisterDllNotification(ULONG Flags, void * NotificationFunction, PVOID Context, PVOID *Cookie);
static NTSTATUS Ldr_LdrUnregisterDllNotification(void * Cookie);

static void CALLBACK Ldr_LdrDllNotification(ULONG NotificationReason, PLDR_DLL_NOTIFICATION_DATA NotificationData, void * Context);

static BOOL Ldr_NtOpenThreadToken(HANDLE ThreadHandle, DWORD  DesiredAccess, BOOL    OpenAsSelf, PHANDLE TokenHandle);
static BOOL Ldr_RtlEqualSid(void * sid1, void * sid2);

//---------------------------------------------------------------------------


ULONG Ldr_SetDdagState_W8(ULONG_PTR BaseAddress, ULONG NewState); // ldr_init

//---------------------------------------------------------------------------
typedef NTSTATUS(NTAPI *P_LdrRegisterDllNotification)(ULONG Flags, void * NotificationFunction, PVOID Context, PVOID *Cookie);
typedef NTSTATUS(NTAPI *P_LdrUnregisterDllNotification)(void * Cookie);

typedef NTSTATUS(*P_LdrLockLoaderLock)(ULONG Flags, ULONG *State, ULONG_PTR *Cookie);

typedef NTSTATUS(*P_LdrUnlockLoaderLock)(ULONG Flags, ULONG_PTR Cookie);

typedef NTSTATUS(*P_LdrLoadDll)(
    WCHAR *PathString, ULONG *DllFlags,
    UNICODE_STRING *ModuleName, HANDLE *ModuleHandle);

typedef NTSTATUS(*P_LdrUnloadDll)(
    HANDLE ModuleHandle);

typedef ULONG_PTR(*P_LdrResolveDelayLoadedAPI)(
    HMODULE ImageBase, ULONG_PTR DelayImportDescriptor,
    ULONG_PTR UnknownParameter1, ULONG_PTR UnknownParameter2,
    ULONG_PTR UnknownParameter3, ULONG_PTR UnknownParameter4);

typedef NTSTATUS(*P_LdrQueryImageFileExecutionOptions)(
    UNICODE_STRING *SubKey, const WCHAR *ValueName, ULONG Type,
    void *Buffer, ULONG BufferSize, ULONG *ReturnedLength);

typedef ULONG_PTR(*P_NtApphelpCacheControl)(
    ULONG_PTR Unknown1, ULONG_PTR Unknown2);

typedef NTSTATUS(*P_NtTerminateProcess)(HANDLE ProcessHandle, NTSTATUS ExitStatus);

typedef NTSTATUS(*P_NtLoadDriver)(UNICODE_STRING *RegistryPath);

typedef void(*P_LdrDllCallback)(const UCHAR *ImageName, HMODULE ImageBase);
typedef void(*P_LdrDllCallbackW)(const WCHAR *ImageName, HMODULE ImageBase);
typedef void(*P_Ldr_CallOneDllCallback)(const UCHAR *ImageNameA, ULONG_PTR ImageBase);

typedef  NTSTATUS(WINAPI *P_NtAccessCheckByType) (
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    PSID PrincipalSelfSid,
    HANDLE ClientToken,
    ACCESS_MASK DesiredAccess,
    POBJECT_TYPE_LIST ObjectTypeList,
    ULONG ObjectTypeListLength,
    PGENERIC_MAPPING GenericMapping,
    PPRIVILEGE_SET PrivilegeSet,
    PULONG PrivilegeSetLength,
    PACCESS_MASK GrantedAccess,
    PNTSTATUS AccessStatus);

typedef BOOL(*P_RtlEqualSid) (void * sid1, void * sid2);

//---------------------------------------------------------------------------

static P_LdrRegisterDllNotification __sys_LdrRegisterDllNotification = NULL;
static P_LdrUnregisterDllNotification __sys_LdrUnregisterDllNotification = NULL;

static P_NtTerminateProcess __sys_NtTerminateProcess = NULL;

static P_LdrLockLoaderLock      __sys_LdrLockLoaderLock = NULL;
static P_LdrUnlockLoaderLock    __sys_LdrUnlockLoaderLock = NULL;

static P_LdrLoadDll             __sys_LdrLoadDll = NULL;
static P_LdrUnloadDll           __sys_LdrUnloadDll = NULL;

static P_LdrResolveDelayLoadedAPI __sys_LdrResolveDelayLoadedAPI = NULL;

static P_LdrQueryImageFileExecutionOptions __sys_LdrQueryImageFileExecutionOptions = NULL;
static P_NtApphelpCacheControl  __sys_NtApphelpCacheControl = NULL;

static P_NtLoadDriver           __sys_NtLoadDriver = NULL;

P_LdrGetDllHandleEx      __sys_LdrGetDllHandleEx = NULL;

static P_Ldr_CallOneDllCallback __my_Ldr_CallOneDllCallback = NULL;

static P_NtOpenThreadToken  __sys_NtOpenThreadToken = NULL;

P_RtlEqualSid __sys_RtlEqualSid = NULL;

extern ULONG Dll_Windows;

extern BOOLEAN Secure_Is_IE_NtQueryInformationToken;

extern BOOLEAN Secure_FakeAdmin;

//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------

static DLL Ldr_Dlls[] = {
    { L"advapi32.dll",          "advapi32.dll",         AdvApi_Init,                    0},
    { L"crypt32.dll",           "crypt32.dll",          Crypt_Init,                     0},
    { L"hnetcfg.dll",           "hnetcfg.dll",          HNet_Init,                      0},
    { L"ws2_32.dll",            "ws2_32.dll",           WSA_Init,                       0},
    { L"iphlpapi.dll",          "iphlpapi.dll",         IpHlp_Init,                     0},
    { L"netapi32.dll",          "netapi32.dll",         NetApi_Init,                    0},
    { L"wkscli.dll",            "wkscli.dll",           NetApi_Init_WksCli,             0},
    { L"ole32.dll",             "ole32.dll",            Ole_Init,                       0},
    { L"combase.dll",           "combase.dll",          Com_Init_ComBase,               0},
    { L"pstorec.dll",           "pstorec.dll",          Pst_Init,                       0},
    { L"secur32.dll",           "secur32.dll",          Lsa_Init_Secur32,               0},
    { L"sspicli.dll",           "sspicli.dll",          Lsa_Init_SspiCli,               0},
    { L"setupapi.dll",          "setupapi.dll",         Setup_Init_SetupApi,            0},
    { L"cfgmgr32.dll",          "cfgmgr32.dll",         Setup_Init_CfgMgr32,            0},
    { L"shell32.dll",           "shell32.dll",          SH32_Init,                      0},
    { L"zipfldr.dll",           "zipfldr.dll",          SH32_Init_ZipFldr,              0},
    { L"uxtheme.dll",           "uxtheme.dll",          SH32_Init_UxTheme,              0},
    { L"shcore.dll",            "shcore.dll",           Taskbar_SHCore_Init,            0},
    { L"user32.dll",            "user32.dll",           Gui_Init,                       0},
    { L"imm32.dll",             "imm32.dll",            Gui_Init_IMM32,                 0},
    { L"d3d9.dll",              "d3d9.dll",             Gui_Init_D3D9,                  0},
    { L"sfc_os.dll",            "sfc_os.dll",           Sfc_Init,                       0},
    { L"wtsapi32.dll",          "wtsapi32.dll",         Terminal_Init_WtsApi,           0},
    { L"winsta.dll",            "winsta.dll",           Terminal_Init_WinSta,           0},
    { L"wevtapi.dll",           "wevtapi.dll",          EvtApi_Init,                    0},
    { L"sxs.dll",               "sxs.dll",              Sxs_Init,                       0},
    { L"gdi32.dll",             "gdi32.dll",            Gdi_Init,                       0},
    { L"gdi32full.dll",         "gdi32full.dll",        Gdi_Full_Init,                  0},
    { L"winspool.drv",          "winspool.drv",         Gdi_Init_Spool,                 0},
    { L"rpcrt4.dll",            "rpcrt4.dll",           RpcRt_Init,                     0},
    { L"userenv.dll",           "userenv.dll",          UserEnv_Init,                   0},
    { L"sechost.dll",           "sechost.dll",          Scm_SecHostDll,                 0},
    { L"msi.dll",               "msi.dll",              Scm_MsiDll,                     0},
    { L"osppc.dll",             "osppc.dll",            Scm_OsppcDll,                   0},
    { L"dwrite.dll",            "dwrite.dll",           Scm_DWriteDll,                  0},
    { L"mso.dll",               "mso.dll",              File_MsoDll,                    0},
    { L"advpack.dll",           "advpack.dll",          Proc_Init_AdvPack,              0},
    { L"agcore.dll",            "agcore.dll",           Custom_SilverlightAgCore,       0},
    { L"MsgPlusLive.dll",       "MsgPlusLive.dll",      Custom_MsgPlusLive,             0},
    { L"IDMIECC.dll",           "IDMIECC.dll",          Custom_InternetDownloadManager, 0},
    { L"snxhk.dll",             "snxhk.dll",            Custom_Avast_SnxHk,             0},
    { L"snxhk64.dll",           "snxhk64.dll",          Custom_Avast_SnxHk,             0},
    { L"emet.dll",              "emet.dll",             Custom_EMET_DLL,                0},
    { L"sysfer.dll",            "sysfer.dll",           Custom_SYSFER_DLL,              0},
    { L"MsCorEE.dll",           "MsCorEE.dll",          MsCorEE_Init,                   0},
    { L"Pdh.dll",               "Pdh.dll",              Pdh_Init,                       0},
    { L"winnsi.dll",            "winnsi.dll",           NsiRpc_Init,                    0},
    { L"acscmonitor.dll",       "acscmonitor.dll",      Acscmonitor_Init,               0},
    { L"ComDlg32.dll",          "ComDlg32.dll",         ComDlg32_Init,                  0},
    { L"ntmarta.dll",           "ntmarta.dll",          Ntmarta_Init,                   0},
#ifdef _WIN64
    { L"dgapi64.dll",           "dgapi64.dll",          DigitalGuardian_Init,           0},
#else
    { L"dgapi.dll",             "dgapi.dll",            DigitalGuardian_Init,           0},
#endif _WIN64
    { NULL,                     NULL ,                  NULL,                           0}
};

static ULONG_PTR *Ldr_Callbacks = 0;

static CRITICAL_SECTION Ldr_LoadedModules_CritSec;
static void *Ldr_LoadedModules = NULL;
static void *LdrLoaderCookie = NULL;
static volatile BOOLEAN Ldr_LdrLoadDll_Invoked = FALSE;

//---------------------------------------------------------------------------

#include "ldr_init.c"

NTSTATUS Ldr_NtAccessCheckByType(
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    PSID PrincipalSelfSid,
    HANDLE ClientToken,
    ACCESS_MASK DesiredAccess,
    POBJECT_TYPE_LIST ObjectTypeList,
    ULONG ObjectTypeListLength,
    PGENERIC_MAPPING GenericMapping,
    PPRIVILEGE_SET PrivilegeSet,
    PULONG PrivilegeSetLength,
    PACCESS_MASK GrantedAccess,
    PNTSTATUS AccessStatus
);

NTSTATUS  Ldr_NtAccessCheckByTypeResultList(
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    PSID PrincipalSelfSid,
    HANDLE ClientToken,
    ACCESS_MASK     DesiredAccess,
    POBJECT_TYPE_LIST ObjectTypeList,
    ULONG ObjectTypeListLength,
    PGENERIC_MAPPING GenericMapping,
    PPRIVILEGE_SET  PrivilegeSet,
    PULONG PrivilegeSetLength,
    PACCESS_MASK    GrantedAccess,
    PNTSTATUS   AccessStatus
);

NTSTATUS Ldr_NtAccessCheck(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN HANDLE               ClientToken,
    IN ACCESS_MASK          DesiredAccess,
    IN PGENERIC_MAPPING     GenericMapping OPTIONAL,
    OUT PPRIVILEGE_SET      RequiredPrivilegesBuffer,
    IN OUT PULONG           BufferLength,
    OUT PACCESS_MASK        GrantedAccess,
    OUT PNTSTATUS           AccessStatus);

NTSTATUS Ldr_NtQuerySecurityAttributesToken(
    IN HANDLE TokenHandle,
    IN PUNICODE_STRING Attributes,
    IN ULONG NumberOfAttributes,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG ReturnLength);

NTSTATUS Ldr_NtQueryInformationToken(
    HANDLE TokenHandle,
    TOKEN_INFORMATION_CLASS TokenInformationClass,
    void *TokenInformation,
    ULONG TokenInformationLength,
    ULONG *ReturnLength);

NTSTATUS Ldr_NtTerminateProcess(HANDLE  ProcessHandle, NTSTATUS ExitStatus);

static P_NtAccessCheckByType            __sys_NtAccessCheckByType = NULL;
static P_NtAccessCheck                  __sys_NtAccessCheck = NULL;
static P_NtQuerySecurityAttributesToken __sys_NtQuerySecurityAttributesToken = NULL;
static P_NtQueryInformationToken        __sys_NtQueryInformationToken = NULL;
static P_NtAccessCheckByTypeResultList  __sys_NtAccessCheckByTypeResultList = NULL;



NTSTATUS Ldr_NtTerminateProcess(HANDLE  ProcessHandle, NTSTATUS ExitStatus)
{
    NTSTATUS rc;

    // ProcessHandle is optional. Unregister callback when current process is terminating
    if (!ProcessHandle
        || ProcessHandle == NtCurrentProcess()
        || GetCurrentProcessId() == GetProcessId(ProcessHandle)
        )
    {
        __sys_LdrUnregisterDllNotification(LdrLoaderCookie);
    }

    rc = __sys_NtTerminateProcess(ProcessHandle, ExitStatus);

    return rc;
}

//---------------------------------------------------------------------------

void CALLBACK Ldr_LdrDllNotification(ULONG NotificationReason, PLDR_DLL_NOTIFICATION_DATA NotificationData, void * Context)
{
    ULONG_PTR LdrCookie = 0;
    NTSTATUS status = 0;


    if (NotificationReason == 1) {
        status = __sys_LdrLockLoaderLock(0, NULL, &LdrCookie);
        Ldr_MyDllCallbackNew(NotificationData->Loaded.BaseDllName->Buffer, (HMODULE)NotificationData->Loaded.DllBase);
        __sys_LdrUnlockLoaderLock(0, LdrCookie);

        return;
    }
    else if (NotificationReason == 2) {
        Ldr_MyDllCallbackNew(NotificationData->Unloaded.BaseDllName->Buffer, 0);
    }
    return;
}

//---------------------------------------------------------------------------

_FX NTSTATUS Ldr_LdrRegisterDllNotification(ULONG Flags, void * NotificationFunction, PVOID Context, PVOID *Cookie)
{
    NTSTATUS status = 0;
    status = __sys_LdrRegisterDllNotification(0, ((void *)Ldr_LdrDllNotification), NULL, Cookie);
    return status;
}

_FX NTSTATUS Ldr_LdrUnregisterDllNotification(void * Cookie)
{
    return STATUS_SUCCESS;
}

//---------------------------------------------------------------------------
// LdrCheckImmersive
//---------------------------------------------------------------------------

//A Microsoft update adds an extra level of integrity checking for "immersive processes" in the win32k driver for
//windows 8.0 and higher.  Microsoft determines a process as immersive by the existence of a PE Header 
//section name ".imrsiv".
//
//The following windows updates adds this functionality: KB3081455 for Windows 10 & KB3087039 Windows 8.x. 
//(see JIRA: SBIE-1)
//
//The LdrCheckImmersive function counters the new logic in win32k.sys by changing the .imrisv section name in an
//immersive process to .Imrisv.  This will bypass the forced integrity check for immersive processes in win32k.sys 
//so the sandboxie untrusted level will pass the integrity check.
//
//
//win32k.sys/win32kbase.sys code:
//PE header section name check in the following win32k.sys/win32kbase.sys function:
//UserProcessImmersiveType(_WIN32_PROCESS_CALLOUT_PARAMETERS *,void *,_PROCESS_IMMERSIVE_TYPE *,ulong *,int *)
//The reverse engineered code of the "immersive" PE section name check loop of this function is as follows:
//
//PE_Section_Name = GetFirstSectioName(PE)
//while (PE_SectionName) {                         
//  /*if PE_Section_Name == ".imrsiv" */
//  if(!RtlCompareMemory(PE_Section_Name,".imrsiv",8)) { 
//     if(integrityLevel < 0x2000) {
//       return 0xc0000001; /* this breaks sandboxie */
//     }
//     else {
//       return TRUE;
//     }
//   }
//   PE_Section_Name = GetNextSectioname();
//}
//return TRUE;
//; ---------------------------------------------------------------------------

BOOL LdrCheckImmersive()
{
    IMAGE_DOS_HEADER *dos_hdr = 0;
    IMAGE_NT_HEADERS *nt_hdrs = 0;
    IMAGE_SECTION_HEADER *section = 0;
    int i;

    UCHAR * bindata = (UCHAR *)GET_PEB_IMAGE_BASE;
    dos_hdr = (IMAGE_DOS_HEADER *)bindata;
    if (dos_hdr->e_magic == 'MZ' || dos_hdr->e_magic == 'ZM') {
        nt_hdrs = (IMAGE_NT_HEADERS *)((UCHAR *)dos_hdr + dos_hdr->e_lfanew);
        if (nt_hdrs->Signature != IMAGE_NT_SIGNATURE) {   // 'PE\0\0'
            return FALSE;
        }
    }

    section = IMAGE_FIRST_SECTION(nt_hdrs);

    for (i = 0; i < nt_hdrs->FileHeader.NumberOfSections; i++) {
        if (!memcmp(section[i].Name, ".imrsiv", 8)) {
            void *RegionBase = section[i].Name;
            SIZE_T RegionSize = 8;
            ULONG OldProtect;
            ULONG dummy_prot;

            NtProtectVirtualMemory(NtCurrentProcess(), &RegionBase, &RegionSize, PAGE_EXECUTE_READWRITE, &OldProtect);
            section[i].Name[1] -= 0x20;
            NtProtectVirtualMemory(NtCurrentProcess(), &RegionBase, &RegionSize, OldProtect, &dummy_prot);
            return TRUE;
        }
    }
    return TRUE;
}

//---------------------------------------------------------------------------
// Ldr_Init
//---------------------------------------------------------------------------

_FX BOOLEAN Ldr_Init()
{
    UCHAR *ReadImageFileExecOptions;

    //
    // get access to the loader lock manipulation routines,
    // available on Windows XP and later
    //


    if (Ldr_Callbacks) {
        return TRUE;
    }
    __sys_LdrLockLoaderLock = (P_LdrLockLoaderLock)GetProcAddress(Dll_Ntdll, "LdrLockLoaderLock");

    __sys_LdrUnlockLoaderLock = (P_LdrUnlockLoaderLock)GetProcAddress(Dll_Ntdll, "LdrUnlockLoaderLock");

    //
    // initialize data
    //

    InitializeCriticalSection(&Ldr_LoadedModules_CritSec);

    Ldr_Callbacks = Dll_Alloc(sizeof(ULONG_PTR) * LDR_NUM_CALLBACKS);
    memzero(Ldr_Callbacks, sizeof(ULONG_PTR) * LDR_NUM_CALLBACKS);

    if (Dll_OsBuild >= 6000) { // Windows Vista
        SbieDll_RegisterDllCallback(Ldr_MyDllCallbackA);
        __my_Ldr_CallOneDllCallback = Ldr_CallOneDllCallback;
    }
    else {
        SbieDll_RegisterDllCallback(Ldr_MyDllCallbackW);
        __my_Ldr_CallOneDllCallback = Ldr_CallOneDllCallbackXP;
    }

    //
    // hook entrypoints
    //

    void* RtlEqualSid = (P_RtlEqualSid)GetProcAddress(Dll_Ntdll, "RtlEqualSid");

    SBIEDLL_HOOK(Ldr_, RtlEqualSid);

    if (Secure_FakeAdmin || Dll_OsBuild >= 9600) {

        void* NtAccessCheckByType = GetProcAddress(Dll_Ntdll, "NtAccessCheckByType");
        void* NtAccessCheck = GetProcAddress(Dll_Ntdll, "NtAccessCheck");
        void* NtQuerySecurityAttributesToken = GetProcAddress(Dll_Ntdll, "NtQuerySecurityAttributesToken");
        void* NtQueryInformationToken = GetProcAddress(Dll_Ntdll, "NtQueryInformationToken");
        void* NtAccessCheckByTypeResultList = GetProcAddress(Dll_Ntdll, "NtAccessCheckByTypeResultList");
        

        SBIEDLL_HOOK(Ldr_, NtQuerySecurityAttributesToken);
        SBIEDLL_HOOK(Ldr_, NtAccessCheckByType);
        SBIEDLL_HOOK(Ldr_, NtAccessCheck);
        SBIEDLL_HOOK(Ldr_, NtAccessCheckByTypeResultList);
        SBIEDLL_HOOK(Ldr_, NtQueryInformationToken);
    }

    if (Dll_OsBuild >= 9600) { // Windows 8.1 and later
        NTSTATUS rc = 0;

        void *NtTerminateProcess = (P_NtTerminateProcess)GetProcAddress(Dll_Ntdll, "NtTerminateProcess");
        __sys_LdrRegisterDllNotification = (P_LdrRegisterDllNotification)GetProcAddress(Dll_Ntdll, "LdrRegisterDllNotification");
        __sys_LdrUnregisterDllNotification = (P_LdrUnregisterDllNotification)GetProcAddress(Dll_Ntdll, "LdrUnregisterDllNotification");

        /* needed for future update */
        //void *LdrRegisterDllNotification = (P_LdrRegisterDllNotification)GetProcAddress(Dll_Ntdll,"LdrRegisterDllNotification");
        //void *LdrUnregisterDllNotification = (P_LdrUnregisterDllNotification)GetProcAddress(Dll_Ntdll,"LdrUnregisterDllNotification");
        //SBIEDLL_HOOK(Ldr_,LdrRegisterDllNotification);
        //SBIEDLL_HOOK(Ldr_,LdrUnregisterDllNotification);
        if (__sys_LdrRegisterDllNotification) {
            rc = __sys_LdrRegisterDllNotification(0, ((void *)Ldr_LdrDllNotification), NULL, &LdrLoaderCookie);
        }
        if (rc) {
            return FALSE;
        }

        SBIEDLL_HOOK(Ldr_, NtTerminateProcess);
        SBIEDLL_HOOK(Ldr_Win10_, LdrLoadDll);
        SBIEDLL_HOOK(Ldr_, NtLoadDriver);
        if (DLL_IMAGE_GOOGLE_CHROME == Dll_ImageType) {
            SBIEDLL_HOOK(Ldr_, NtOpenThreadToken);
        }
    }
    else { // Windows 8 and before
        SBIEDLL_HOOK(Ldr_, LdrLoadDll);
        SBIEDLL_HOOK(Ldr_, LdrUnloadDll);
        SBIEDLL_HOOK(Ldr_, LdrQueryImageFileExecutionOptions);
        SBIEDLL_HOOK(Ldr_, NtLoadDriver);
        if (Dll_OsBuild >= 8400) {

            P_LdrResolveDelayLoadedAPI LdrResolveDelayLoadedAPI =
                (P_LdrResolveDelayLoadedAPI)GetProcAddress(
                    Dll_Ntdll, "LdrResolveDelayLoadedAPI");

            P_NtApphelpCacheControl NtApphelpCacheControl =
                (P_NtApphelpCacheControl)GetProcAddress(
                    Dll_Ntdll, "NtApphelpCacheControl");

            SBIEDLL_HOOK(Ldr_, LdrResolveDelayLoadedAPI);
            SBIEDLL_HOOK(Ldr_, NtApphelpCacheControl);
        }

    }
    //
    // set PEB.ReadImageFileExecOptions to non-zero to force ntdll to call
    // LdrQueryImageFileExecutionOptions so we can call Ldr_CallDllCallbacks
    //
    // on Windows 8, we use a hook on NtApphelpCacheControl instead
    //

    if (Dll_OsBuild < 8400) { // Windows 7 and older

        ReadImageFileExecOptions = (UCHAR *)(NtCurrentPeb() + 1);

        if (!(*ReadImageFileExecOptions))
            *ReadImageFileExecOptions = '*';
    }
    else {
        LdrCheckImmersive();
    }

    //
    // do some more initializations based on the executable image,
    // and inject code at the program entrypoint
    //


    // Ronen fix --
    //
    // load inject DLLs
    //

    // Ldr_LoadInjectDlls();

    //
    // initialize manifest
    //

    Ldr_Inject_Init(FALSE);
    Sxs_ActivateDefaultManifest((void *)Ldr_ImageBase);

    return TRUE;
}

//---------------------------------------------------------------------------
// SbieDll_RegisterDllCallback
//---------------------------------------------------------------------------

_FX BOOLEAN SbieDll_RegisterDllCallback(void *Callback)
{
    NTSTATUS status = 0;
    ULONG_PTR LdrCookie;
    ULONG i;
    BOOLEAN ok = FALSE;
    ULONG state = 2;

    if (Dll_OsBuild < 9600) {
        status = __sys_LdrLockLoaderLock(0, NULL, &LdrCookie);
    }
    if (NT_SUCCESS(status)) {

        for (i = 0; i < LDR_NUM_CALLBACKS; ++i) {

            if (!Ldr_Callbacks[i]) {

                Ldr_Callbacks[i] = (ULONG_PTR)Callback;
                ok = TRUE;

                break;
            }
        }

        if (Dll_OsBuild < 9600) {
            __sys_LdrUnlockLoaderLock(0, LdrCookie);
        }
    }
    return ok;
}


//---------------------------------------------------------------------------
// Ldr_CallOneDllCallback
//---------------------------------------------------------------------------

_FX void Ldr_CallOneDllCallback(const UCHAR *ImageNameA, ULONG_PTR ImageBase)
{
    ULONG i;

    for (i = 0; i < LDR_NUM_CALLBACKS; ++i) {
        ULONG_PTR callback = Ldr_Callbacks[i];
        if (!callback)
            break;
        __try {
            ((P_LdrDllCallback)callback)(ImageNameA, (HMODULE)ImageBase);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
        }
    }
}


_FX void Ldr_CallOneDllCallbackXP(const UCHAR *ImageNameA, ULONG_PTR ImageBase)
{
    ULONG i;

    WCHAR ImageNameW[512];
    WCHAR *NameW = ImageNameW;
    for (i = 0; i < 511 && ImageNameA[i]; ++i)
        NameW[i] = ImageNameA[i];
    NameW[i] = L'\0';

    for (i = 0; i < LDR_NUM_CALLBACKS; ++i) {

        ULONG_PTR callback = Ldr_Callbacks[i];
        if (!callback)
            break;

        __try {
            ((P_LdrDllCallbackW)callback)(ImageNameW, (HMODULE)ImageBase);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
        }
    }
}


//---------------------------------------------------------------------------
// Ldr_CallDllCallbacks
//---------------------------------------------------------------------------


_FX void Ldr_CallDllCallbacks(void)
{
    static ULONG CallDepth = 0;

    NTSTATUS status;
    void *buf;
    ULONG len1, len2;

    struct _SYSTEM_MODULE_INFORMATION *OldList, *NewList;
    ULONG i, iOld, iNew;
    MODULE_INFO *pOld, *pNew;
    BOOLEAN found;

    //
    // if this is a recursive call to Ldr_CallDllCallbacks, then a callback
    // function, which we already invoked through this function, has loaded
    // another DLL, causing control to reach this function again.  in this
    // case just let the other Ldr_CallDllCallbacks handle it
    //

    if (InterlockedIncrement(&CallDepth) != 1)
        return;

    //
    // get the most current list of modules loaded into the process
    //

    len1 = 1024 * 32;
    buf = NULL;

    //Ldr_inCallBack++;

    for (i = 0; i < 6; ++i) {

        buf = Dll_Alloc(len1);
        status = LdrQueryProcessModuleInformation(buf, len1, &len2);
        if (NT_SUCCESS(status))
            break;

        Dll_Free(buf);
        buf = NULL;

        if (status != STATUS_INFO_LENGTH_MISMATCH)
            break;

        len1 = len2 + 1024;
    }

    if (!buf) {
        SbieApi_Log(2205, L"LoadedModules [%08X]", status);
        return;
    }

    OldList = (struct _SYSTEM_MODULE_INFORMATION *)Ldr_LoadedModules;

    NewList = (struct _SYSTEM_MODULE_INFORMATION *)buf;

    //
    // check each module that appears in the 'old' list against
    // the 'new' list and invoke 'unload' callbacks
    //

    if (OldList) {

        for (iOld = 0; iOld < OldList->ModuleCount; ++iOld) {

            pOld = &OldList->ModuleInfo[iOld];

            found = FALSE;

            for (iNew = 0; iNew < NewList->ModuleCount; ++iNew) {

                pNew = &NewList->ModuleInfo[iNew];

                if (pOld->ImageBaseAddress == pNew->ImageBaseAddress &&
                    strcmp(pOld->Path, pNew->Path) == 0) {

                    found = TRUE;
                    break;
                }
            }

            if (!found) {

                __my_Ldr_CallOneDllCallback(pOld->Path + pOld->NameOffset, 0);
            }
        }
    }

    //
    // check each module that appears in the 'new' list against
    // the 'old' list and invoke 'load' callbacks
    //

    for (iNew = 0; iNew < NewList->ModuleCount; ++iNew) {

        pNew = &NewList->ModuleInfo[iNew];

        found = FALSE;

        if (OldList) {

            for (iOld = 0; iOld < OldList->ModuleCount; ++iOld) {

                pOld = &OldList->ModuleInfo[iOld];

                if (pOld->ImageBaseAddress == pNew->ImageBaseAddress &&
                    strcmp(pOld->Path, pNew->Path) == 0) {

                    found = TRUE;
                    break;
                }
            }
        }

        if (!found) {

            ULONG OldState = Ldr_SetDdagState_W8(pNew->ImageBaseAddress, 9);

            ANSI_STRING ansi;
            UNICODE_STRING uni;
            RtlInitString(&ansi, pNew->Path);
            RtlAnsiStringToUnicodeString(&uni, &ansi, TRUE);
            if (iNew == 0 && *uni.Buffer)
                *uni.Buffer = L'*';
            _wcslwr(uni.Buffer);
            SbieApi_MonitorPut(MONITOR_IMAGE, uni.Buffer);
            RtlFreeUnicodeString(&uni);

            __my_Ldr_CallOneDllCallback(pNew->Path + pNew->NameOffset,
                pNew->ImageBaseAddress);

            if (OldState)
                Ldr_SetDdagState_W8(pNew->ImageBaseAddress, OldState);
        }
    }

    //
    // finish.  note that if CallDepth > 1 at this point, then we had a
    // recursive invocation that did not do anything, and we should check
    // if any new modules have been loaded
    //

    EnterCriticalSection(&Ldr_LoadedModules_CritSec);

    Ldr_LoadedModules = NewList;

    LeaveCriticalSection(&Ldr_LoadedModules_CritSec);

    if (OldList && OldList != NewList)
        Dll_Free(OldList);

    if (InterlockedExchange(&CallDepth, 0) > 1) {

        Ldr_CallDllCallbacks();
    }
}


//---------------------------------------------------------------------------
// Ldr_CallDllCallbacks_WithLock
//---------------------------------------------------------------------------


_FX void Ldr_CallDllCallbacks_WithLock(void)
{
    ULONG_PTR LdrCookie = 0;
    NTSTATUS status = 0;
    ULONG state = 2;

    status = __sys_LdrLockLoaderLock(0, NULL, &LdrCookie);
    if (NT_SUCCESS(status)) {
        Ldr_CallDllCallbacks();
        __sys_LdrUnlockLoaderLock(0, LdrCookie);
    }
}


//---------------------------------------------------------------------------
// Ldr_LdrLoadDll
//---------------------------------------------------------------------------


_FX NTSTATUS Ldr_LdrLoadDll(
    WCHAR *PathString,
    ULONG *DllFlags,
    UNICODE_STRING *ModuleName,
    HANDLE *ModuleHandle)
{
    //
    // load the DLL and invoke callbacks
    //

    ULONG_PTR LdrCookie = 0;
    NTSTATUS status = 0;
    ULONG state = 2;

    status = __sys_LdrLockLoaderLock(0, &state, &LdrCookie);
    if (NT_SUCCESS(status)) {

        status = __sys_LdrLoadDll(PathString, DllFlags, ModuleName, ModuleHandle);

        if (NT_SUCCESS(status)) {
            Ldr_CallDllCallbacks();
            Ldr_LdrLoadDll_Invoked = TRUE;
        }
        __sys_LdrUnlockLoaderLock(0, LdrCookie);
    }
    Scm_SecHostDll_W8();
    return status;
}


//---------------------------------------------------------------------------
// Ldr_Win10_LdrLoadDll
//---------------------------------------------------------------------------


_FX NTSTATUS Ldr_Win10_LdrLoadDll(
    WCHAR *PathString,
    ULONG *DllFlags,
    UNICODE_STRING *ModuleName,
    HANDLE *ModuleHandle)
{
    //
    // load the DLL 
    //
    NTSTATUS status = 0;

    status = __sys_LdrLoadDll(PathString, DllFlags, ModuleName, ModuleHandle);
    Scm_SecHostDll_W8();
    return status;
}

//---------------------------------------------------------------------------
// Ldr_LdrUnloadDll
//---------------------------------------------------------------------------


_FX NTSTATUS Ldr_LdrUnloadDll(HANDLE ModuleHandle)
{
    NTSTATUS status = 0;
    ULONG_PTR LdrCookie = 0;
    ULONG state = 2;
    DWORD tid;

    //
    // prevent unloading of SbieDll
    //

    tid = GetCurrentThreadId();
    if (ModuleHandle == Dll_Instance)
        return STATUS_SUCCESS;
    //
    // unload the DLL
    //
    status = __sys_LdrLockLoaderLock(0, NULL, &LdrCookie);

    if (NT_SUCCESS(status)) {

        status = __sys_LdrUnloadDll(ModuleHandle);

        if (Ldr_LdrLoadDll_Invoked) {

            //
            // since static dlls can't be freed, there is no real point
            // to call Ldr_CallDllCallbacks before any dynamic dll has
            // ever loaded
            //
            // more importantly, apphelp.dll (the shim engine) can call
            // LdrUnloadDll on aclayers.dll, before ntdll starts to
            // initialize static dlls, and that is too early to call
            // Ldr_CallDllCallbacks and run our module init routines
            //

            Ldr_CallDllCallbacks();
        }

        __sys_LdrUnlockLoaderLock(0, LdrCookie);
    }

    return status;
}


//---------------------------------------------------------------------------
// Ldr_LdrResolveDelayLoadedAPI
//---------------------------------------------------------------------------


_FX ULONG_PTR Ldr_LdrResolveDelayLoadedAPI(
    HMODULE ImageBase, ULONG_PTR DelayImportDescriptor,
    ULONG_PTR UnknownParameter1, ULONG_PTR UnknownParameter2,
    ULONG_PTR UnknownParameter3, ULONG_PTR UnknownParameter4)
{
    //
    // load the DLL and invoke callbacks
    //
    ULONG_PTR result;

    result = __sys_LdrResolveDelayLoadedAPI(
        ImageBase, DelayImportDescriptor, UnknownParameter1,
        UnknownParameter2, UnknownParameter3, UnknownParameter4);

    Ldr_CallDllCallbacks_WithLock();

    Scm_SecHostDll_W8();

    return result;
}


//---------------------------------------------------------------------------
// Ldr_LdrQueryImageFileExecutionOptions
//---------------------------------------------------------------------------


_FX NTSTATUS Ldr_LdrQueryImageFileExecutionOptions(
    UNICODE_STRING *SubKey, const WCHAR *ValueName, ULONG Type,
    void *Buffer, ULONG BufferSize, ULONG *ReturnedLength)
{

    NTSTATUS rc;
    //
    // our Ldr_CallDllCallbacks misses static import DLLs because these are
    // loaded using the unexported LdrpLoadDll function which we can't hook.
    // however LdrpRunInitializeRoutines (also unexported) calls the exported
    // LdrQueryImageFileExecutionOptions before running init routines, so
    // this is a good opportunity to call our DLL callbacks
    //

    Ldr_CallDllCallbacks();     // loader lock held by ntdll

    //
    // if we caused PEB.ReadImageFileExecOptions to be non-zero, we will restore
    // the zero value in Ldr_Inject_Entry and return STATUS_OBJECT_NAME_NOT_FOUND
    // here to prevent recursive call in __sys_LdrQueryImageFileExecutionOptions
    //
    if (Dll_OsBuild < 8400) {

        UCHAR *ReadImageFileExecOptions = (UCHAR *)(NtCurrentPeb() + 1);
        if (*ReadImageFileExecOptions == '*')
            return STATUS_OBJECT_NAME_NOT_FOUND;
    }

    rc = __sys_LdrQueryImageFileExecutionOptions(
        SubKey, ValueName, Type, Buffer, BufferSize, ReturnedLength);
    return  rc;
}


//---------------------------------------------------------------------------
// Ldr_NtApphelpCacheControl
//---------------------------------------------------------------------------


_FX ULONG_PTR Ldr_NtApphelpCacheControl(
    ULONG_PTR Unknown1, ULONG_PTR Unknown2)
{
    ULONG_PTR rc;
    //
    // invoke DLL callbacks
    //
    Ldr_CallDllCallbacks_WithLock();
    rc = __sys_NtApphelpCacheControl(Unknown1, Unknown2);
    return  rc;
}


//---------------------------------------------------------------------------
// Ldr_MyDllCallbacks (A,W,New)
//---------------------------------------------------------------------------


_FX void Ldr_MyDllCallbackA(const CHAR *ImageName, HMODULE ImageBase)
{
    //
    // invoke our sub-modules as necessary
    //
    if (ImageBase) {

        DLL *dll = Ldr_Dlls;
        while (dll->nameA) {
            if (_stricmp(ImageName, dll->nameA) == 0) {
                BOOLEAN ok = dll->init_func(ImageBase);
                if (!ok)
                    SbieApi_Log(2318, dll->nameW);
                break;
            }
            ++dll;
        }
    }
}

_FX void Ldr_MyDllCallbackW(const WCHAR *ImageName, HMODULE ImageBase)
{
    //
    // invoke our sub-modules as necessary
    //
    if (ImageBase) {

        DLL *dll = Ldr_Dlls;
        while (dll->nameW) {
            if (_wcsicmp(ImageName, dll->nameW) == 0) {
                BOOLEAN ok = dll->init_func(ImageBase);
                if (!ok)
                    SbieApi_Log(2318, dll->nameW);

                break;
            }

            ++dll;
        }
    }
}


_FX void Ldr_MyDllCallbackNew(const WCHAR *ImageName, HMODULE ImageBase)
{
    //
    // invoke our sub-modules as necessary
    //
    DLL *dll = Ldr_Dlls;

    while (dll->nameW) {
        BOOLEAN ok;
        if (_wcsicmp(ImageName, dll->nameW) == 0) {
            if (ImageBase && !dll->state) {
                EnterCriticalSection(&Ldr_LoadedModules_CritSec);
                dll->state = 1;
                LeaveCriticalSection(&Ldr_LoadedModules_CritSec);
                ok = dll->init_func(ImageBase);
                if (!ok)
                    SbieApi_Log(2318, dll->nameW);
                break;
            }
            else {
                EnterCriticalSection(&Ldr_LoadedModules_CritSec);
                dll->state = 0;
                LeaveCriticalSection(&Ldr_LoadedModules_CritSec);
            }
        }
        ++dll;
    }
}

//---------------------------------------------------------------------------
// Ldr_GetProcAddr
//---------------------------------------------------------------------------


_FX void *Ldr_GetProcAddrOld(const WCHAR *DllName, const WCHAR *ProcNameW)
{
    NTSTATUS status;
    void *proc = Ldr_GetProcAddr_2(DllName, ProcNameW);

    if (!proc) {
        //      if (Dll_Windows < 10) {
        ULONG_PTR LdrCookie;
        ULONG state = 0, i = 0;

        for (i = 0; i < 20; ++i) {
            status = __sys_LdrLockLoaderLock(2 /*try*/, &state, &LdrCookie);
            if (NT_SUCCESS(status) && (state <= 1))
                break;
            else {
                NtYieldExecution();
            }
        }

        if (i < 20 && state <= 1) {
            HMODULE DllBase;
            DllBase = GetModuleHandle(DllName);
            if (!DllBase) {
                DllBase = LoadLibrary(DllName);
            }
            if (DllBase) {

                proc = Ldr_GetProcAddr_3((ULONG_PTR)DllBase, ProcNameW);
            }

            if (state == 1)                 // if lock was really taken
                __sys_LdrUnlockLoaderLock(0, LdrCookie);
        }

    }

    return proc;
}


_FX void *Ldr_GetProcAddrNew(const WCHAR *DllName, const WCHAR *ProcNameW, char * ProcNameA)
{
    NTSTATUS status;
    void *proc;
    //  char buffer[768];
    //  sprintf(buffer,"GetProcAddrNew: DllName = %S, ProcW = %S, ProcA = %s\n",DllName,ProcNameW,ProcNameA);
    //  OutputDebugStringA(buffer);

    if (Dll_OsBuild < 9600) {
        proc = Ldr_GetProcAddr_2(DllName, ProcNameW);
        if (!proc) {
            ULONG_PTR LdrCookie;
            ULONG state = 0, i = 0;

            for (i = 0; i < 20; ++i) {
                status = __sys_LdrLockLoaderLock(2 /*try*/, &state, &LdrCookie);
                if (NT_SUCCESS(status) && (state <= 1))
                    break;
                else {
                    NtYieldExecution();
                }
            }

            if (i < 20 && state <= 1) {
                HMODULE DllBase;
                DllBase = GetModuleHandle(DllName);
                if (!DllBase) {
                    DllBase = LoadLibrary(DllName);
                }
                if (DllBase) {

                    proc = Ldr_GetProcAddr_3((ULONG_PTR)DllBase, ProcNameW);
                }

                if (state == 1)                 // if lock was really taken
                    __sys_LdrUnlockLoaderLock(0, LdrCookie);
            }
        }
    }
    else {
        HMODULE DllBase;
        DllBase = GetModuleHandle(DllName);
        if (!DllBase) {
            DllBase = LoadLibrary(DllName);
        }
        if (DllBase) {
            proc = GetProcAddress(DllBase, ProcNameA);
        }

    }
    return proc;
}


//---------------------------------------------------------------------------
// Ldr_GetProcAddr_2
//---------------------------------------------------------------------------


_FX void *Ldr_GetProcAddr_2(const WCHAR *DllName, const WCHAR *ProcName)
{
    void *proc = NULL;
    struct _SYSTEM_MODULE_INFORMATION *list;
    ULONG i, j;
    const ULONG n = wcslen(DllName);

    EnterCriticalSection(&Ldr_LoadedModules_CritSec);

    list = (struct _SYSTEM_MODULE_INFORMATION *)Ldr_LoadedModules;
    if (list) {

        for (i = 0; i < list->ModuleCount; ++i) {

            const MODULE_INFO *mod = &list->ModuleInfo[i];
            const UCHAR *name = &mod->Path[mod->NameOffset];
            for (j = 0; j < n; ++j) {
                if (tolower(name[j]) != towlower(DllName[j]))
                    break;
            }
            if (j == n) {

                proc = Ldr_GetProcAddr_3(mod->ImageBaseAddress, ProcName);
                break;
            }
        }
    }

    LeaveCriticalSection(&Ldr_LoadedModules_CritSec);

    return proc;
}


//---------------------------------------------------------------------------
// Ldr_GetProcAddr_3
//---------------------------------------------------------------------------


_FX void *Ldr_GetProcAddr_3(ULONG_PTR DllBase, const WCHAR *ProcName)
{
    void *proc = NULL;
    ULONG i, j;
    const ULONG n = wcslen(ProcName);

    IMAGE_OPTIONAL_HEADER *OptHdr = Ldr_OptionalHeader(DllBase);
    if (OptHdr->NumberOfRvaAndSizes) {

        const IMAGE_DATA_DIRECTORY *dir0 = &OptHdr->DataDirectory[0];
        if (dir0->VirtualAddress && dir0->Size) {

            IMAGE_EXPORT_DIRECTORY *exports = (IMAGE_EXPORT_DIRECTORY *)
                ((UCHAR *)DllBase + dir0->VirtualAddress);

            ULONG *names = (ULONG *)
                ((UCHAR *)DllBase + exports->AddressOfNames);

            for (i = 0; i < exports->NumberOfNames; ++i) {

                UCHAR *name = (UCHAR *)DllBase + names[i];
                for (j = 0; j < n; ++j) {
                    if (tolower(name[j]) != towlower(ProcName[j]))
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
                && (ULONG_PTR)proc < (ULONG_PTR)exports + dir0->Size) {

                //
                // don't handle forwarded exports
                //

                SbieApi_Log(2205, L"EXPORT %s", proc);
                proc = NULL;
            }
        }
    }
    return proc;
}

//---------------------------------------------------------------------------
// Ldr_NtLoadDriver
//---------------------------------------------------------------------------


_FX NTSTATUS Ldr_NtLoadDriver(UNICODE_STRING *RegistryPath)
{
    NTSTATUS status;

    status = __sys_NtLoadDriver(RegistryPath);
    if (status == STATUS_PRIVILEGE_NOT_HELD) {

        WCHAR *DriverName = NULL;

        // if we got a legitimate UNICODE_STRING then try to display
        // the driver name.  note that Kerio Personal Firewall passes
        // an invalid parameter (RegistryPath = 0xFFFFFFxx)

        if (RegistryPath &&
            (((ULONG)(ULONG_PTR)RegistryPath) & 0xFFFFFF00) != 0xFFFFFF00
            && RegistryPath->Buffer) {

            DriverName = wcsrchr(RegistryPath->Buffer, L'\\');
            if (DriverName)
                ++DriverName;
        }

        if (DriverName)
            SbieApi_Log(2103, L"%S [%S] (NtLoadDriver)", DriverName, Dll_BoxName);
    }

    return status;
}

//---------------------------------------------------------------------------

_FX void Ldr_TestToken(HANDLE token, PHANDLE hTokenReal)
{
    if (Dll_OsBuild < 9600) // this magic values are available only from windows 8.1 onwards
        return;

    if ((LONG_PTR)token == LDR_TOKEN_PRIMARY) {
        NtOpenProcessToken(NtCurrentProcess(), TOKEN_QUERY, hTokenReal);
    }
    else if ((LONG_PTR)token == LDR_TOKEN_IMPERSONATION) {
        NtOpenThreadToken(NtCurrentThread(), TOKEN_QUERY, FALSE, hTokenReal);
    }
    else if ((LONG_PTR)token <= LDR_TOKEN_EFFECTIVE) {
        NtOpenThreadToken(NtCurrentThread(), TOKEN_QUERY, FALSE, hTokenReal);
        if (!hTokenReal) {
            NtOpenProcessToken(NtCurrentProcess(), TOKEN_QUERY, hTokenReal);
        }
    }
    return;
}

_FX NTSTATUS Ldr_NtQueryInformationToken(
    HANDLE TokenHandle,
    TOKEN_INFORMATION_CLASS TokenInformationClass,
    void *TokenInformation,
    ULONG TokenInformationLength,
    ULONG *ReturnLength)
{
    NTSTATUS status = 0;
    THREAD_DATA *TlsData = NULL;
    HANDLE hTokenReal = NULL;
    BOOLEAN FakeAdmin = FALSE;

    Ldr_TestToken(TokenHandle, &hTokenReal);

    status = __sys_NtQueryInformationToken(
        hTokenReal ? hTokenReal : TokenHandle, TokenInformationClass,
        TokenInformation, TokenInformationLength, ReturnLength);

    if (hTokenReal)
    {
        NtClose(hTokenReal);
    }

    //
    // To make the process think we need to chage here a fe values
    // we also ensure that tha token belongs to the current process
    //

    if (Secure_FakeAdmin && (SbieApi_QueryProcessInfoEx(0, 'ippt', (LONG_PTR)hTokenReal))) 
    {
        FakeAdmin = TRUE;
    }

    //
    // NtQueryInformationToken is hooked for Internet Explorer.
    //
    // if the check occurs during CreateProcess, then return the real
    // information, so UAC elevation may occur for the new process.
    //
    // otherwise, this check is related to Protected Mode, so pretend
    // we are running as Administrator
    //

    TlsData = Dll_GetTlsData(NULL);

    if (Secure_Is_IE_NtQueryInformationToken && !TlsData->proc_create_process)
    {
        FakeAdmin = TRUE;
    }

    if (NT_SUCCESS(status) && FakeAdmin) {

        if (TokenInformationClass == TokenElevation) {

            *(BOOLEAN *)TokenInformation = TRUE;
        }

        else if (TokenInformationClass == TokenElevationType) {

            //
            // on Vista, fake a return value for a full token
            //

            *(ULONG *)TokenInformation = TokenElevationTypeFull;
        }

        else if (TokenInformationClass == TokenIntegrityLevel) {

            //
            // on Vista, fake a high integrity level
            //

#include "pshpack4.h"

            typedef struct {

                ULONG_PTR Pointer;
                ULONG_PTR Sixty;
                ULONG     OneOhOne;
                ULONG     HighBitSet;
                ULONG     ThreeK;

            } TOKEN_INTEGRITY_LEVEL;

#include "poppack.h"

            if (TokenInformationLength >= sizeof(TOKEN_INTEGRITY_LEVEL)) {

                TOKEN_INTEGRITY_LEVEL *Info =
                    (TOKEN_INTEGRITY_LEVEL *)TokenInformation;

                Info->Pointer = (ULONG_PTR)TokenInformation
                    + sizeof(ULONG_PTR) * 2;
                Info->Sixty = 0x60;
                Info->OneOhOne = 0x101;
                Info->HighBitSet = 0x10000000;
                Info->ThreeK = 0x3000;

                if (ReturnLength)
                    *ReturnLength = sizeof(TOKEN_INTEGRITY_LEVEL);
            }
        }
    }

    return status;
}

_FX NTSTATUS Ldr_NtQuerySecurityAttributesToken(HANDLE TokenHandle, PUNICODE_STRING Attributes, ULONG NumberOfAttributes, PVOID Buffer, ULONG Length, PULONG ReturnLength)
{
    NTSTATUS status = 0;
    HANDLE hTokenReal = NULL;

    Ldr_TestToken(TokenHandle, &hTokenReal);

    status = __sys_NtQuerySecurityAttributesToken(hTokenReal ? hTokenReal : TokenHandle, Attributes, NumberOfAttributes, Buffer, Length, ReturnLength);

    if (hTokenReal) {
        NtClose(hTokenReal);
    }
    return status;
}

NTSTATUS Ldr_NtAccessCheckByType(PSECURITY_DESCRIPTOR SecurityDescriptor, PSID PrincipalSelfSid, HANDLE ClientToken, ACCESS_MASK DesiredAccess, POBJECT_TYPE_LIST ObjectTypeList, ULONG ObjectTypeListLength, PGENERIC_MAPPING GenericMapping, PPRIVILEGE_SET PrivilegeSet, PULONG PrivilegeSetLength, PACCESS_MASK GrantedAccess, PNTSTATUS AccessStatus)
{
    NTSTATUS rc;
    HANDLE hTokenReal = NULL;

    if (Dll_ImageType == DLL_IMAGE_SANDBOXIE_BITS ||
        Dll_ImageType == DLL_IMAGE_SANDBOXIE_WUAU ||
        Dll_ImageType == DLL_IMAGE_WUAUCLT) {
        *GrantedAccess = 0xFFFFFFFF;
        *AccessStatus = TRUE;
        SetLastError(0);
        return TRUE;
    }

    Ldr_TestToken(ClientToken, &hTokenReal);

    rc = __sys_NtAccessCheckByType(SecurityDescriptor, PrincipalSelfSid, hTokenReal ? hTokenReal : ClientToken, DesiredAccess, ObjectTypeList, ObjectTypeListLength, GenericMapping, PrivilegeSet, PrivilegeSetLength, GrantedAccess, AccessStatus);

    if (hTokenReal) {
        NtClose(hTokenReal);
    }

    return rc;
}


_FX NTSTATUS Ldr_NtAccessCheck(PSECURITY_DESCRIPTOR SecurityDescriptor, HANDLE ClientToken, ACCESS_MASK DesiredAccess, PGENERIC_MAPPING GenericMapping, PPRIVILEGE_SET RequiredPrivilegesBuffer, PULONG BufferLength, PACCESS_MASK GrantedAccess, PNTSTATUS AccessStatus)
{
    NTSTATUS status = 0;
    HANDLE hTokenReal = NULL;

    Ldr_TestToken(ClientToken, &hTokenReal);

    status = __sys_NtAccessCheck(SecurityDescriptor, hTokenReal ? hTokenReal : ClientToken, DesiredAccess, GenericMapping, RequiredPrivilegesBuffer, BufferLength, GrantedAccess, AccessStatus);
    
    if (hTokenReal) {
        NtClose(hTokenReal);
    }
    return status;
}

_FX NTSTATUS Ldr_NtAccessCheckByTypeResultList(PSECURITY_DESCRIPTOR SecurityDescriptor, PSID PrincipalSelfSid, HANDLE ClientToken, ACCESS_MASK  DesiredAccess, POBJECT_TYPE_LIST ObjectTypeList, ULONG ObjectTypeListLength, PGENERIC_MAPPING GenericMapping, PPRIVILEGE_SET    PrivilegeSet, PULONG PrivilegeSetLength, PACCESS_MASK   GrantedAccess, PNTSTATUS    AccessStatus)
{
    NTSTATUS status = 0;
    HANDLE hTokenReal = NULL;

    Ldr_TestToken(ClientToken, &hTokenReal);

    status = __sys_NtAccessCheckByTypeResultList(SecurityDescriptor, PrincipalSelfSid, ClientToken, DesiredAccess, ObjectTypeList, ObjectTypeListLength, GenericMapping, PrivilegeSet, PrivilegeSetLength, GrantedAccess, AccessStatus);

    if (hTokenReal) {
        NtClose(hTokenReal);
    }
    return status;
}

BOOL Ldr_NtOpenThreadToken(HANDLE ThreadHandle, DWORD  DesiredAccess, BOOL    OpenAsSelf, PHANDLE TokenHandle)
{
    BOOL rc;

    rc = __sys_NtOpenThreadToken(ThreadHandle, DesiredAccess, OpenAsSelf, TokenHandle);
    if (rc == STATUS_ACCESS_DENIED && OpenAsSelf) {
        rc = __sys_NtOpenThreadToken(ThreadHandle, DesiredAccess, 0, TokenHandle);
    }
    return rc;
}

BOOL Ldr_RtlEqualSid(void * sid1, void * sid2)
{
    if (!sid1 || !sid2) {
        return FALSE;
    }
    return __sys_RtlEqualSid(sid1, sid2);
}
