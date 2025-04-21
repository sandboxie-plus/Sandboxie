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
// Sandboxie DLL
//---------------------------------------------------------------------------


#include "dll.h"
#include "obj.h"
#include "trace.h"
#include "debug.h"
#include "dump.h"
#include "core/low/lowdata.h"
#include "common/my_version.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

static void Dll_InitGeneric(HINSTANCE hInstance);

static void Dll_InitInjected(void);

static void Dll_SelectImageType(void);

void Ldr_Inject_Init(BOOLEAN bHostInject);

void Dll_Wow64DisableTurboThunks(void);

//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


const ULONG tzuk = 'xobs';

SBIELOW_DATA* SbieApi_data = NULL;
#ifdef _M_ARM64EC
ULONG* SbieApi_SyscallPtr = NULL;
#endif

HINSTANCE Dll_Instance = NULL;
HMODULE Dll_Ntdll = NULL;
HMODULE Dll_Kernel32 = NULL;
HMODULE Dll_KernelBase = NULL;
HMODULE Dll_Win32u = NULL;
// $Workaround$ - 3rd party fix
HMODULE Dll_DigitalGuardian = NULL;

const WCHAR *Dll_BoxName = NULL;
const WCHAR *Dll_ImageName = NULL;
const WCHAR *Dll_SidString = NULL;

const WCHAR *Dll_HomeNtPath = NULL;
ULONG Dll_HomeNtPathLen = 0;
const WCHAR *Dll_HomeDosPath = NULL;
//ULONG Dll_HomeDosPathLen = 0;

const WCHAR *Dll_BoxFilePath = NULL;
const WCHAR *Dll_BoxFileRawPath = NULL;
const WCHAR *Dll_BoxFileDosPath = NULL;
const WCHAR *Dll_BoxKeyPath = NULL;
const WCHAR *Dll_BoxIpcPath = NULL;

ULONG Dll_BoxFilePathLen = 0;
ULONG Dll_BoxFileRawPathLen = 0;
ULONG Dll_BoxFileDosPathLen = 0;
ULONG Dll_BoxKeyPathLen = 0;
ULONG Dll_BoxIpcPathLen = 0;
ULONG Dll_SidStringLen = 0;

ULONG Dll_ProcessId = 0;
ULONG Dll_SessionId = 0;

ULONG Dll_DriverFlags = 0;
ULONG64 Dll_ProcessFlags = 0;

#ifndef _WIN64
BOOLEAN Dll_IsWow64 = FALSE;
#endif
#ifdef _M_ARM64EC
BOOLEAN Dll_IsArm64ec = FALSE;
void* Dll_xtajit64 = NULL;
#endif
#ifndef _WIN64
BOOLEAN Dll_IsXtAjit = FALSE;
#endif
BOOLEAN Dll_IsSystemSid = FALSE;
BOOLEAN Dll_InitComplete = FALSE;
BOOLEAN Dll_EntryComplete = FALSE;
BOOLEAN Dll_RestrictedToken = FALSE;
BOOLEAN Dll_AppContainerToken = FALSE;
BOOLEAN Dll_ChromeSandbox = FALSE;
BOOLEAN Dll_FirstProcessInBox = FALSE;
BOOLEAN Dll_CompartmentMode = FALSE;
//BOOLEAN Dll_AlernateIpcNaming = FALSE;

ULONG Dll_ImageType = DLL_IMAGE_UNSPECIFIED;

ULONG Dll_OsBuild = 0;
ULONG Dll_Windows = 0;

const UCHAR *SbieDll_Version = MY_VERSION_STRING;


//extern ULONG64 __security_cookie = 0;


//---------------------------------------------------------------------------


static WCHAR *Dll_BoxNameSpace;
static WCHAR *Dll_ImageNameSpace;
static WCHAR *Dll_SidStringSpace;


//---------------------------------------------------------------------------


const WCHAR *DllName_advapi32   = L"advapi32.dll";
const WCHAR *DllName_combase    = L"combase.dll";
const WCHAR *DllName_kernel32   = L"kernel32.dll";
const WCHAR *DllName_kernelbase = L"kernelbase.dll";
const WCHAR *DllName_ole32      = L"ole32.dll";
const WCHAR *DllName_oleaut32   = L"oleaut32.dll";
const WCHAR *DllName_user32     = L"user32.dll";
const WCHAR *DllName_rpcrt4     = L"rpcrt4.dll";
const WCHAR *DllName_winnsi     = L"winnsi.dll";
const WCHAR *DllName_shell32    = L"shell32.dll";
const WCHAR *DllName_sechost    = L"sechost.dll";
const WCHAR *DllName_gdi32      = L"gdi32.dll";
const WCHAR *DllName_secur32    = L"secur32.dll";
const WCHAR *DllName_sspicli    = L"sspicli.dll";
const WCHAR *DllName_mscoree    = L"mscoree.dll";
const WCHAR *DllName_ntmarta    = L"ntmarta.dll";
const WCHAR *DllName_winmm      = L"winmm.dll";


//---------------------------------------------------------------------------
// DllMain
//---------------------------------------------------------------------------


_FX BOOL WINAPI DllMain(
    HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    if (dwReason == DLL_THREAD_ATTACH) {

        if (Dll_BoxName) {
            if (Dll_Windows < 10)
                Dll_FixWow64Syscall();
            Gui_ConnectToWindowStationAndDesktop(NULL);
        }

    } else if (dwReason == DLL_THREAD_DETACH) {

        Dll_FreeTlsData();

    } else if (dwReason == DLL_PROCESS_ATTACH) {

        // $Workaround$ - 3rd party fix
#ifdef _WIN64
        Dll_DigitalGuardian = GetModuleHandleA("DgApi64.dll");
#else
        Dll_DigitalGuardian = GetModuleHandleA("DgApi.dll");
#endif

        Dll_OsBuild = GET_PEB_IMAGE_BUILD;

        //if (GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueueApcThreadEx2"))  // Somewhere around windows insider build 19603
        //    Dll_Windows = 11;
        //else if (GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtGetCurrentProcessorNumberEx"))  
        //else 
        if (GetProcAddress(GetModuleHandleA("ntdll.dll"), "LdrFastFailInLoaderCallout")) 
            Dll_Windows = 10;
        else //if (GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtAlpcConnectPortEx")) 
            Dll_Windows = 8;
        // else
        //    Dll_Windows = 7;

        ProcessIdToSessionId(GetCurrentProcessId(), &Dll_SessionId);

        Dll_InitGeneric(hInstance);
        SbieDll_HookInit();

    } else if (dwReason == DLL_PROCESS_DETACH) {

        if (Dll_InitComplete && Dll_BoxName) {

            File_DoAutoRecover(TRUE);
            Gui_ResetClipCursor();
        }

    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Dll_InitGeneric
//---------------------------------------------------------------------------


_FX void Dll_InitGeneric(HINSTANCE hInstance)
{
    //
    // Dll_InitGeneric initializes SbieDll in a general way, suitable
    // for a program which may or may not be in the sandbox
    //

    Dll_Instance = hInstance;

    Dll_Ntdll = GetModuleHandle(L"ntdll.dll");
    Dll_Kernel32 = GetModuleHandle(DllName_kernel32);
    Dll_KernelBase = GetModuleHandle(DllName_kernelbase);

	extern void InitMyNtDll(HMODULE Ntdll);
	InitMyNtDll(Dll_Ntdll);

	extern FARPROC __sys_GetModuleInformation;
	__sys_GetModuleInformation = GetProcAddress(LoadLibraryW(L"psapi.dll"), "GetModuleInformation");

    if (! Dll_InitMem()) {
        SbieApi_Log(2305, NULL);
        ExitProcess(-1);
    }
}


//---------------------------------------------------------------------------
// Dll_InitInjected
//---------------------------------------------------------------------------


_FX void Dll_InitInjected(void)
{
	//
	// Dll_InitInjected is executed by Dll_Ordinal1 in the context
	// of a program that is running in the sandbox
	//

	LONG status;
	BOOLEAN ok;
	ULONG BoxFilePathLen;
	ULONG BoxKeyPathLen;
	ULONG BoxIpcPathLen;

    //
    // confirm the process is sandboxed before going further
    //

    Dll_BoxNameSpace        = Dll_Alloc(BOXNAME_COUNT * sizeof(WCHAR));
    memzero(Dll_BoxNameSpace,           BOXNAME_COUNT * sizeof(WCHAR));

    Dll_ImageNameSpace      = Dll_Alloc(256 * sizeof(WCHAR));
    memzero(Dll_ImageNameSpace,         256 * sizeof(WCHAR));

    Dll_SidStringSpace      = Dll_Alloc( 96 * sizeof(WCHAR));
    memzero(Dll_SidStringSpace,          96 * sizeof(WCHAR));

    Dll_ProcessId = (ULONG)(ULONG_PTR)GetCurrentProcessId();

    status = SbieApi_QueryProcessEx2( // sets proc->sbiedll_loaded = TRUE; in the driver
        (HANDLE)(ULONG_PTR)Dll_ProcessId, 255,
        Dll_BoxNameSpace, Dll_ImageNameSpace, Dll_SidStringSpace,
        &Dll_SessionId, NULL);

    if (status != 0) {
        SbieApi_Log(2304, Dll_ImageName);
        ExitProcess(-1);
    }

    Dll_BoxName = (const WCHAR *)Dll_BoxNameSpace;
    Dll_ImageName = (const WCHAR *)Dll_ImageNameSpace;
    Dll_SidString = (const WCHAR *)Dll_SidStringSpace;

    Dll_SidStringLen = wcslen(Dll_SidString);


    //
    // break for the debugger, as soon as we have Dll_ImageName
    //

    Debug_Wait();

    Trace_Init();

    //
    // query Sandboxie home folder
    //

    Dll_HomeNtPath = Dll_AllocTemp(1024 * sizeof(WCHAR));
    Dll_HomeDosPath = Dll_AllocTemp(1024 * sizeof(WCHAR));

    SbieApi_GetHomePath((WCHAR*)Dll_HomeNtPath, 1020, (WCHAR*)Dll_HomeDosPath, 1020);

    Dll_HomeNtPathLen = wcslen(Dll_HomeNtPath);
    //Dll_HomeDosPathLen = wcslen(Dll_HomeDosPath);

    //
    // get features flags
    //

    SbieApi_QueryDrvInfo(0, &Dll_DriverFlags, sizeof(Dll_DriverFlags));

    //
    // get process type and flags
    //

    Dll_ProcessFlags = SbieApi_QueryProcessInfo(0, 0);

    Dll_CompartmentMode = (Dll_ProcessFlags & SBIE_FLAG_APP_COMPARTMENT) != 0;

    //
    // check for restricted token types
    //

    Dll_RestrictedToken = Secure_IsRestrictedToken(FALSE);

    Dll_AppContainerToken = Secure_IsAppContainerToken(NULL);

    Dll_SelectImageType();

    //
    // query the box paths
    //

    BoxFilePathLen = 0;
    BoxKeyPathLen = 0;
    BoxIpcPathLen = 0;

    status = SbieApi_QueryBoxPath(
        NULL, NULL, NULL, NULL,
        &BoxFilePathLen, &BoxKeyPathLen, &BoxIpcPathLen);
    if (status != 0) {
        SbieApi_Log(2304, Dll_ImageName);
        ExitProcess(-1);
    }

    Dll_BoxFilePath = Dll_Alloc(BoxFilePathLen);
    Dll_BoxKeyPath = Dll_Alloc(BoxKeyPathLen);
    Dll_BoxIpcPath = Dll_Alloc(BoxIpcPathLen);

    status = SbieApi_QueryBoxPath(
        NULL,
        (WCHAR *)Dll_BoxFilePath,
        (WCHAR *)Dll_BoxKeyPath,
        (WCHAR *)Dll_BoxIpcPath,
        &BoxFilePathLen, &BoxKeyPathLen, &BoxIpcPathLen);
    if (status != 0) {
        SbieApi_Log(2304, Dll_ImageName);
        ExitProcess(-1);
    }

    Dll_BoxFilePathLen = wcslen(Dll_BoxFilePath);
    Dll_BoxKeyPathLen = wcslen(Dll_BoxKeyPath);
    Dll_BoxIpcPathLen = wcslen(Dll_BoxIpcPath);

  //  Dll_AlernateIpcNaming = SbieApi_QueryConfBool(NULL, L"UseAlernateIpcNaming", FALSE);
  //  if (Dll_AlernateIpcNaming) {
  //
  //      //
  //      // instead of using a separate namespace
  //		// just replace all \ with _ and use it as a suffix rather then an actual path
  //      // similar to what is done for named pipes already
  //      // this approach can help to reduce the footprint when running in portable mode
  //      // alternatively we could create volatile entries under AppContainerNamedObjects 
  //      //
  //
  //      WCHAR* ptr = (WCHAR*)Dll_BoxIpcPath;
  //      while (*ptr) {
  //          WCHAR *ptr2 = wcschr(ptr, L'\\');
  //          if (ptr2) {
  //              ptr = ptr2;
  //              *ptr = L'_';
  //          } else
  //              ptr += wcslen(ptr);
  //      }
  //  }


#ifdef WITH_DEBUG
    if (SbieApi_QueryConfBool(NULL, L"DisableSbieDll", FALSE)) {
        Dll_InitComplete = TRUE;
        return;
    }
#endif

    //
    // check if process SID is LocalSystem
    //

    Dll_IsSystemSid = Secure_IsLocalSystemToken(FALSE);

    //
    // create a security descriptor granting access to everyone
    //

    Secure_InitSecurityDescriptors();

    //
    // initialize sandboxed process, first the basic NTDLL hooks
    //

    ok = Dll_InitPathList();

#ifndef _WIN64
    if (ok) {
        if (Dll_Windows < 10) 
            Dll_FixWow64Syscall();
        else
            Dll_Wow64DisableTurboThunks();
    }
#endif

    if (ok)
        ok = Handle_Init();

    if (ok)
        ok = Obj_Init();

    if (ok) {

        //
        // check if we are the first process in the sandbox
        // (for AutoExec function in custom module)
        //

        ULONG pid_count = 0;
        if (NT_SUCCESS(SbieApi_EnumProcessEx(NULL,FALSE,-1,NULL,&pid_count)) && pid_count == 1)
            Dll_FirstProcessInBox = TRUE;

        WCHAR str[32];
        if (NT_SUCCESS(SbieApi_QueryConfAsIs(NULL, L"ProcessLimit", 0, str, sizeof(str) - sizeof(WCHAR)))) {
            ULONG num = _wtoi(str);
            if (num > 0) {
                if (num < pid_count)
                    ExitProcess(-1);
                if ((num * 8 / 10) < pid_count)
                    Sleep(3000);
            }
        }
    }

    if (ok) {

        //
        // ipc must be initialized before anything else to make delete v2 work
        //

        ok = Ipc_Init();
    }

    if (ok) {

        //
        // Key should be initialized first, to prevent key requests
        // with MAXIMUM_ALLOWED access from failing
        //

        ok = Key_Init();

        //
        // on Windows 8.1, we may get some crashes errors while Chrome
        // is shutting down, so just quit any WerFault.exe process
        //

        //if (ok && Dll_OsBuild >= 9600 &&
        //        _wcsicmp(Dll_ImageName, L"WerFault.exe") == 0) {

        //    ExitProcess(0);
        //}
    }

    if (ok)
        ok = File_Init();

    if (ok)
        ok = Secure_Init();

    if (ok)
        ok = SysInfo_Init();

    if (ok)
        ok = Sxs_InitKernel32();

    if (ok)
        ok = Proc_Init();

    if (ok)
        ok = Kernel_Init();

    if (ok)
        ok = Gui_InitConsole1();

    if (ok) // Note: Ldr_Init may cause rpcss to be started early
        ok = Ldr_Init();            // last to initialize

    //
    // finish
    //

#ifdef WITH_DEBUG
    if (ok) 
        ok = Debug_Init();
#endif WITH_DEBUG

    if (! ok) {
        SbieApi_Log(2304, Dll_ImageName);
        ExitProcess(-1);
    }

    //
    // Setup soft resource restrictions
    //

    WCHAR str[32];
    if (NT_SUCCESS(SbieApi_QueryConfAsIs(NULL, L"CpuAffinityMask", 0, str, sizeof(str) - sizeof(WCHAR))) && str[0] == L'0' && (str[1] == L'x' || str[1] == L'X')){

        WCHAR* endptr;
        KAFFINITY AffinityMask = wcstoul(str + 2, &endptr, 16); // note we only support core 0-31 as wcstoull is not exported by ntdll
        if (AffinityMask)
            NtSetInformationProcess(GetCurrentProcess(), ProcessAffinityMask, &AffinityMask, sizeof(KAFFINITY));
    }

    Dll_InitComplete = TRUE;

    if (! Dll_RestrictedToken)
        CustomizeSandbox();
}


//---------------------------------------------------------------------------
// Dll_InitExeEntry
//---------------------------------------------------------------------------


_FX void Dll_InitExeEntry(void)
{
    //
    // Dll_InitInjected is executed by Ldr_Inject_Entry after NTDLL has
    // finished initializing the process (loading static import DLLs, etc)
    //

    //
    // hook DefWindowProc on Windows 7, after USER32 has been initialized
    //

    Gui_InitWindows7();

    //
    // hook the console window, if applicable
    //

    Gui_InitConsole2();

    //
    // if we are SplWow64, register our pid with SbieSvc GUI Proxy
    //

    Gdi_SplWow64(TRUE);

    //
    // check if running as a forced COM server process
    // note:  it does not return if this is the case
    //

    Custom_ComServer();

    //
    // force load of UxTheme in a Google Chrome sandbox process
    //

    // Note: this does not seem to be needed anymore for modern Chrome builds, also it breaks Vivaldi browser

    //Custom_Load_UxTheme(); 

    UserEnv_InitVer(Dll_OsBuild >= 7600 ? Dll_KernelBase : Dll_Kernel32); // in KernelBase since Win 7

    //
    // Windows 8.1:  hook UserEnv-related entrypoint in KernelBase
    //

    if (Dll_OsBuild >= 9600)
        UserEnv_Init(Dll_KernelBase);

    //
    // start SandboxieRpcSs
    //

    SbieDll_StartCOM(TRUE);

    //
    // setup own top level exception handler
    //

    if(Config_GetSettingsForImageName_bool(L"EnableMiniDump", FALSE))
        Dump_Init();

    //
    // once we return here the process images entrypoint will be called
    //

    Trace_Entry();
    Dll_EntryComplete = TRUE;
}


//---------------------------------------------------------------------------
// Dll_GetImageType
//---------------------------------------------------------------------------


_FX ULONG Dll_GetImageType(const WCHAR *ImageName)
{
    ULONG ImageType = DLL_IMAGE_UNSPECIFIED;

    //
    // check for custom configured special images
    //

    ULONG index;
    NTSTATUS status;
    WCHAR wbuf[96];
    WCHAR* buf = wbuf;

    for (index = 0; ; ++index) {
        status = SbieApi_QueryConfAsIs(
            NULL, L"SpecialImage", index, buf, 90 * sizeof(WCHAR));
        if (!NT_SUCCESS(status))
            break;

        WCHAR* ptr = wcschr(buf, L',');
        if (!ptr) continue;

        *ptr++ = L'\0';

        if (_wcsicmp(ImageName, ptr) == 0) {

            if (_wcsicmp(L"chrome", buf) == 0)
                ImageType = DLL_IMAGE_GOOGLE_CHROME;
            else if (_wcsicmp(L"firefox", buf) == 0)
                ImageType = DLL_IMAGE_MOZILLA_FIREFOX;
            else if (_wcsicmp(L"thunderbird", buf) == 0)
                ImageType = DLL_IMAGE_MOZILLA_THUNDERBIRD;
            else if (_wcsicmp(L"browser", buf) == 0)
                ImageType = DLL_IMAGE_OTHER_WEB_BROWSER;
            else if (_wcsicmp(L"mail", buf) == 0)
                ImageType = DLL_IMAGE_OTHER_MAIL_CLIENT;
            else if (_wcsicmp(L"plugin", buf) == 0)
                ImageType = DLL_IMAGE_PLUGIN_CONTAINER;
            else
                ImageType = DLL_IMAGE_LAST; // invalid type set place holder such that we keep this image uncustomized

            break;
        }
    }

    //
    // keep image names in sync with enum at top of dll.h
    //

    static const WCHAR *_ImageNames[] = {

        SANDBOXIE L"RpcSs.exe",     (WCHAR *)DLL_IMAGE_SANDBOXIE_RPCSS,
        SANDBOXIE L"DcomLaunch.exe",(WCHAR *)DLL_IMAGE_SANDBOXIE_DCOMLAUNCH,
        SANDBOXIE L"Crypto.exe",    (WCHAR *)DLL_IMAGE_SANDBOXIE_CRYPTO,
        SANDBOXIE L"WUAU.exe",      (WCHAR *)DLL_IMAGE_SANDBOXIE_WUAU,
        SANDBOXIE L"BITS.exe",      (WCHAR *)DLL_IMAGE_SANDBOXIE_BITS,
        SBIESVC_EXE,                (WCHAR *)DLL_IMAGE_SANDBOXIE_SBIESVC,

        L"msiexec.exe",             (WCHAR *)DLL_IMAGE_MSI_INSTALLER,
        L"TrustedInstaller.exe",    (WCHAR *)DLL_IMAGE_TRUSTED_INSTALLER,
        L"TiWorker.exe",            (WCHAR *)DLL_IMAGE_TRUSTED_INSTALLER,
        L"wuauclt.exe",             (WCHAR *)DLL_IMAGE_WUAUCLT,
        L"explorer.exe",            (WCHAR *)DLL_IMAGE_SHELL_EXPLORER,
        L"rundll32.exe",            (WCHAR *)DLL_IMAGE_RUNDLL32,
        L"dllhost.exe",             (WCHAR *)DLL_IMAGE_DLLHOST,
        L"ServiceModelReg.exe",     (WCHAR *)DLL_IMAGE_SERVICE_MODEL_REG,

        L"iexplore.exe",            (WCHAR *)DLL_IMAGE_INTERNET_EXPLORER,

        L"wmplayer.exe",            (WCHAR *)DLL_IMAGE_WINDOWS_MEDIA_PLAYER,
        L"winamp.exe",              (WCHAR *)DLL_IMAGE_NULLSOFT_WINAMP,
        L"kmplayer.exe",            (WCHAR *)DLL_IMAGE_PANDORA_KMPLAYER,
        L"wlmail.exe",              (WCHAR *)DLL_IMAGE_WINDOWS_LIVE_MAIL,
        L"wisptis.exe",             (WCHAR *)DLL_IMAGE_WISPTIS,

        L"GoogleUpdate.exe",        (WCHAR *)DLL_IMAGE_GOOGLE_UPDATE,

        L"AcroRd32.exe",            (WCHAR *)DLL_IMAGE_ACROBAT_READER,
        L"Acrobat.exe",             (WCHAR *)DLL_IMAGE_ACROBAT_READER,
        L"plugin-container.exe",    (WCHAR *)DLL_IMAGE_PLUGIN_CONTAINER,
        L"Outlook.exe",             (WCHAR *)DLL_IMAGE_OFFICE_OUTLOOK,
        L"Excel.exe",               (WCHAR *)DLL_IMAGE_OFFICE_EXCEL,

        NULL,                       NULL
    };

    if (ImageType == DLL_IMAGE_UNSPECIFIED) {

        for (int i = 0; _ImageNames[i]; i += 2) {
            if (_wcsicmp(ImageName, _ImageNames[i]) == 0) {
                ImageType = (ULONG)(ULONG_PTR)_ImageNames[i + 1];
                break;
            }
        }
    }

    return ImageType;
}

//---------------------------------------------------------------------------
// Dll_SelectImageType
//---------------------------------------------------------------------------


_FX void Dll_SelectImageType(void)
{
    Dll_ImageType = Dll_GetImageType(Dll_ImageName);

    //if (Dll_ImageType == DLL_IMAGE_UNSPECIFIED &&
    //        _wcsnicmp(Dll_ImageName, L"FlashPlayerPlugin_", 18) == 0)
    //    Dll_ImageType = DLL_IMAGE_FLASH_PLAYER_SANDBOX;

    if (Dll_ImageType == DLL_IMAGE_DLLHOST) {

        const WCHAR *CmdLine = GetCommandLine();
        if (CmdLine) {
            if (wcsstr(CmdLine, L"{3EB3C877-1F16-487C-9050-104DBCD66683}"))
                Dll_ImageType = DLL_IMAGE_DLLHOST_WININET_CACHE;
        }
    }

    //
    // issue a warning for some known programs
    //

    if (Dll_ImageType == DLL_IMAGE_UNSPECIFIED && (
            _wcsicmp(Dll_ImageName, L"SchTasks.exe") == 0
        ||  _wcsicmp(Dll_ImageName, L"cvh.exe") == 0    // Office 2010 virt
        ||  0)) {

        SbieApi_Log(2205, Dll_ImageName);
    }

    if (Dll_ImageType == DLL_IMAGE_LAST)
        Dll_ImageType = DLL_IMAGE_UNSPECIFIED;

    SbieApi_QueryProcessInfoEx(0, 'spit', Dll_ImageType);

    //
    // we have some special cases for programs running under a restricted
    // token, such as a Chromium sandbox processes, or Microsoft Office 2010
    // programs running as embedded previewers within Outlook
    //

    if (Dll_RestrictedToken || Dll_AppContainerToken) {

        if (Dll_ImageType == DLL_IMAGE_GOOGLE_CHROME ||
            Dll_ImageType == DLL_IMAGE_MOZILLA_FIREFOX ||
            //Dll_ImageType == DLL_IMAGE_FLASH_PLAYER_SANDBOX
            Dll_ImageType == DLL_IMAGE_ACROBAT_READER) {

            Dll_ChromeSandbox = TRUE;
        }
    }

    Dll_SkipHook(NULL);
}


//---------------------------------------------------------------------------
// Dll_Ordinal1
//---------------------------------------------------------------------------


_FX VOID Dll_Ordinal1(INJECT_DATA * inject)
{
    SBIELOW_DATA *data = (SBIELOW_DATA *)inject->sbielow_data;
    BOOLEAN bHostInject = FALSE;

    SbieApi_data = data;
#ifdef _M_ARM64EC
    // get the pointer to sys_call_list in the SYSCALL_DATA struct
    SbieApi_SyscallPtr = (ULONG*)((ULONG64)data->syscall_data + sizeof(ULONG) + sizeof(ULONG) + (NATIVE_FUNCTION_SIZE * NATIVE_FUNCTION_COUNT));
#endif

    extern HANDLE SbieApi_DeviceHandle;
    SbieApi_DeviceHandle = (HANDLE)data->api_device_handle;

    //
    // the SbieLow data area includes values that are useful to us
    // so we copy them into dedicated variables if we are going to use them more often
    //

    bHostInject = data->flags.bHostInject == 1;

#ifndef _WIN64
    Dll_IsWow64 = data->flags.is_wow64 == 1; // x86 on x64 or arm64
#endif
#ifdef _M_ARM64EC
    Dll_IsArm64ec = data->flags.is_arm64ec == 1; // x64 on arm64
	Dll_xtajit64 = GetModuleHandle(L"xtajit64.dll");
#endif
#ifndef _WIN64
    Dll_IsXtAjit = data->flags.is_xtajit == 1; // x86 on arm64
#endif


    if (!bHostInject)
    {
        //
        // SbieDll was already partially initialized in Dll_InitGeneric,
        // complete the initialization for a sandboxed process
        //
        HANDLE heventProcessStart = 0;

        Dll_InitInjected(); // install required hooks (Dll_InitInjected -> Ldr_Init -> Ldr_Inject_Init(FALSE))

        //
        // notify RPCSS that a new process was created in the current sandbox
        //

        if (Dll_ImageType != DLL_IMAGE_SANDBOXIE_RPCSS) {
            heventProcessStart = CreateEvent(0, FALSE, FALSE, SESSION_PROCESS);
            if (heventProcessStart) {
                SetEvent(heventProcessStart);
                CloseHandle(heventProcessStart);
            }
        }

        //
        // workaround for Program Compatibility Assistant (PCA), we have
        // to start a second instance of this process outside the PCA job,
        // see also Proc_RestartProcessOutOfPcaJob
        //

        int MustRestartProcess = 0;
        if (Dll_ProcessFlags & SBIE_FLAG_PROCESS_IN_PCA_JOB) {
            if (!SbieApi_QueryConfBool(NULL, L"NoRestartOnPCA", FALSE))
                MustRestartProcess = 1;
        }

        else if (Dll_ProcessFlags & SBIE_FLAG_FORCED_PROCESS) {
            if (SbieApi_QueryConfBool(NULL, L"ForceRestartAll", FALSE)
             || SbieDll_CheckStringInList(Dll_ImageName, NULL, L"ForceRestart"))
                MustRestartProcess = 2;
        }

        if (MustRestartProcess) {

            WCHAR text[128];
            Sbie_snwprintf(text, 128, L"Cleanly restarting forced process, reason %d", MustRestartProcess);
            SbieApi_MonitorPutMsg(MONITOR_OTHER, text);

            extern void Proc_RestartProcessOutOfPcaJob(void);
            Proc_RestartProcessOutOfPcaJob();
            // does not return
        }

        //
        // explorer needs sandboxed COM to show a warning and terminate when COM is not sandboxed
        //

        if (Dll_ImageType == DLL_IMAGE_SHELL_EXPLORER && SbieDll_IsOpenCOM()) {

            SbieApi_Log(2195, NULL);
            ExitProcess(0);
        }

        //
        // msi installer requires COM to be sandboxed, else the installation will be done outside the sandbox
        //

        if (Dll_ImageType == DLL_IMAGE_MSI_INSTALLER) {

            if (SbieDll_IsOpenCOM()) {
                SbieApi_Log(2196, NULL);
                ExitProcess(0);
            }

            if (!SbieApi_QueryConfBool(NULL, L"MsiInstallerExemptions", FALSE) && SbieApi_QueryConfBool(NULL, L"NotifyMsiInstaller", TRUE)) {
                SbieApi_Log(2194, L"MsiInstallerExemptions=y");
            }
        }
    }
    else
    {
        Ldr_Inject_Init(TRUE);
    }
}


//---------------------------------------------------------------------------
// Dll_Wow64DisableTurboThunks
//---------------------------------------------------------------------------

#ifndef _WIN64

typedef enum _WOW64_FUNCTION {
    Wow64Function64Nop,
    Wow64FunctionQueryProcessDebugInfo,
    Wow64FunctionTurboThunkControl,
    Wow64FunctionCfgDispatchControl,
    Wow64FunctionOptimizeChpeImportThunks,
} WOW64_FUNCTION;

typedef NTSTATUS(*P_NtWow64CallFunction64)(
    _In_ WOW64_FUNCTION Wow64Function,
    _In_ ULONG Flags,
    _In_ ULONG InputBufferLength,
    _In_reads_bytes_opt_(InputBufferLength) PVOID InputBuffer,
    _In_ ULONG OutputBufferLength,
    _Out_writes_bytes_opt_(OutputBufferLength) PVOID OutputBuffer,
    _Out_opt_ PULONG ReturnLength
);

#define WOW64_TURBO_THUNK_DISABLE 0
#define WOW64_TURBO_THUNK_ENABLE  1   // STATUS_NOT_SUPPORTED :(

_FX void Dll_Wow64DisableTurboThunks(void)
{
    if (!Dll_IsWow64)
        return;

    //
    // Dll_FixWow64Syscall (dllhooks.c):
    // the Wow64 thunking layer for syscalls in ntdll32 has several thunks:
    // thunk 0 calls the corresponding NtXxx export in the 64-bit ntdll.
    // other thunks issue the syscall instruction directly and are probably
    // intended as an optimization.  we want all 32-bit syscalls to go
    // through our SbieLow syscall interface, so we need to always force
    // use of thunk 0 rather than the optimization thunks.
    //
    // Windows 10: 
    // Starting with windows 10 we can use Wow64FunctionTurboThunkControl
    // to disable this "optimization" properly.
    // 
    // Windows on ARM64:
    // On ARM64 there is no wow64cpu.dll and Wow64FunctionTurboThunkControl
    // returns STATUS_NOT_IMPLEMENTED, and no use of Turbo Thunks has been 
    // yet observed hence we can sattle on doing nothing for the time being.
    //
    
    if (Dll_IsXtAjit)
        return;

    NTSTATUS status = STATUS_NOT_SUPPORTED;

    P_NtWow64CallFunction64 NtWow64CallFunction64 = (P_NtWow64CallFunction64)GetProcAddress(Dll_Ntdll, "NtWow64CallFunction64");
    if (NtWow64CallFunction64) {
        ULONG ThunkInput = WOW64_TURBO_THUNK_DISABLE;
        status = NtWow64CallFunction64(Wow64FunctionTurboThunkControl, 0, sizeof(ThunkInput), &ThunkInput, 0, NULL, NULL);
    }

    if(!NT_SUCCESS(status))
        SbieApi_Log(2205, L"Wow64FunctionTurboThunkControl %08X", status);
}

#endif
