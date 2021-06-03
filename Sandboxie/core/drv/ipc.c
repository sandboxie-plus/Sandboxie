/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
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
// Inter-Process Communication
//---------------------------------------------------------------------------


#include "ipc.h"
#include "syscall.h"
#include "process.h"
#include "session.h"
#include "obj.h"
#include "conf.h"
#include "util.h"
#include "api.h"
#include "common/my_version.h"
#include "common/pattern.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN Ipc_Init_Type(
    const WCHAR *TypeName, P_Syscall_Handler2 handler);

static BOOLEAN Ipc_InitPaths(PROCESS *proc);

static BOOLEAN Ipc_IsComServer(PROCESS *proc);


//---------------------------------------------------------------------------


static NTSTATUS Ipc_CheckGenericObject(
    PROCESS *proc, void *Object, UNICODE_STRING *Name,
    ACCESS_MASK GrantedAccess);

static NTSTATUS Ipc_CheckPortObject(
    PROCESS *proc, void *Object, UNICODE_STRING *Name,
    ACCESS_MASK GrantedAccess);

static NTSTATUS Ipc_CheckJobObject(
    PROCESS *proc, void *Object, UNICODE_STRING *Name,
    ACCESS_MASK GrantedAccess);

static NTSTATUS Ipc_CheckObjectName(HANDLE handle, KPROCESSOR_MODE mode);


//---------------------------------------------------------------------------


static NTSTATUS Ipc_Api_DuplicateObject(PROCESS *proc, ULONG64 *parms);

static NTSTATUS Ipc_Api_CreateDirOrLink(PROCESS *proc, ULONG64 *parms);

static NTSTATUS Ipc_Api_OpenDeviceMap(PROCESS *proc, ULONG64 *parms);

static NTSTATUS Ipc_Api_QuerySymbolicLink(PROCESS *proc, ULONG64 *parms);


//---------------------------------------------------------------------------


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, Ipc_Init)
#pragma alloc_text (INIT, Ipc_Init_Type)
#endif // ALLOC_PRAGMA


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static const WCHAR *Ipc_Event_TypeName      = L"Event";
static const WCHAR *Ipc_EventPair_TypeName  = L"EventPair";
static const WCHAR *Ipc_KeyedEvent_TypeName = L"KeyedEvent";
static const WCHAR *Ipc_Mutant_TypeName     = L"Mutant";
static const WCHAR *Ipc_Semaphore_TypeName  = L"Semaphore";
static const WCHAR *Ipc_Section_TypeName    = L"Section";
static const WCHAR *Ipc_JobObject_TypeName  = L"JobObject";


//---------------------------------------------------------------------------
// Ipc_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Ipc_Init(void)
{
    const UCHAR *_PortSyscallNames[] = {
        "ConnectPort", "SecureConnectPort", "CreatePort",
        "AlpcConnectPort", "AlpcCreatePort", NULL
    };
    const UCHAR **NamePtr;

    //
    // set object open handlers for generic objects
    //

#define Ipc_Init_Type_Generic(TypeName)                     \
    if (! Ipc_Init_Type(TypeName, Ipc_CheckGenericObject))  \
        return FALSE;

    Ipc_Init_Type_Generic(Ipc_Event_TypeName);
    Ipc_Init_Type_Generic(Ipc_EventPair_TypeName);
    Ipc_Init_Type_Generic(Ipc_KeyedEvent_TypeName);
    Ipc_Init_Type_Generic(Ipc_Mutant_TypeName);
    Ipc_Init_Type_Generic(Ipc_Semaphore_TypeName);
    Ipc_Init_Type_Generic(Ipc_Section_TypeName);

#undef Ipc_Init_Type_Generic

    if (! Ipc_Init_Type(Ipc_JobObject_TypeName, Ipc_CheckJobObject))
        return FALSE;

    //
    // set object open handlers for port objects
    //

    for (NamePtr = _PortSyscallNames; *NamePtr; ++NamePtr) {

        if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA ||
                            memcmp(*NamePtr, "Alpc", 4) != 0) {

            if (! Syscall_Set2(*NamePtr, Ipc_CheckPortObject))
                return FALSE;
        }
    }

    if (! Syscall_Set1("ImpersonateClientOfPort", Ipc_ImpersonatePort))
        return FALSE;

    if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA) {

        if (! Syscall_Set1(
                    "AlpcImpersonateClientOfPort", Ipc_ImpersonatePort))
            return FALSE;

        //
        // protect use of NtRequestPort, NtRequestWaitReplyPort, and
        // NtAlpcSendWaitReceivePort on Windows Vista to prevent use
        // of EndTask and NetUserChangePassword, when called through our
        // syscall interface.
        //
        // note that if a malicious program calls NtAlpcSendWaitReceivePort
        // directly (without using our syscall interface to elevate first)
        // to emulate NetUserChangePassword, then it gets status code
        // STATUS_PRIVILEGE_NOT_HELD because the restricted process token
        // does not include the change notify privilege.
        //
        // on Windows Vista, direct use of NtAlpcSendWaitReceivePort to
        // emulate EndTask (without using our syscall interface to elevate
        // first) will be blocked by UIPI.  (But note that applications in
        // other sandboxed will still be killable.)
        //
        // on Windows XP, the real NtRequestPort and NtRequestWaitReplyPort
        // in the kernel are already hooked by the gui_xp module.
        //

        if (! Syscall_Set1("RequestPort", Ipc_RequestPort))
            return FALSE;

        if (! Syscall_Set1("RequestWaitReplyPort", Ipc_RequestPort))
            return FALSE;

        if (! Syscall_Set1(
                    "AlpcSendWaitReceivePort", Ipc_AlpcSendWaitReceivePort))
            return FALSE;
    }

    //
    // set API handlers
    //

    Api_SetFunction(API_DUPLICATE_OBJECT,       Ipc_Api_DuplicateObject);
    Api_SetFunction(API_CREATE_DIR_OR_LINK,     Ipc_Api_CreateDirOrLink);
    Api_SetFunction(API_OPEN_DEVICE_MAP,        Ipc_Api_OpenDeviceMap);
    Api_SetFunction(API_QUERY_SYMBOLIC_LINK,    Ipc_Api_QuerySymbolicLink);
    //Api_SetFunction(API_ALLOW_SPOOLER_PRINT_TO_FILE, Ipc_Api_AllowSpoolerPrintToFile);

#ifndef _WIN64
    Api_SetFunction(API_SET_LSA_AUTH_PKG,       Ipc_Api_SetLsaAuthPkg);
#endif ! _WIN64

    Api_SetFunction(API_GET_DYNAMIC_PORT_FROM_PID, Ipc_Api_GetDynamicPortFromPid);
    Api_SetFunction(API_OPEN_DYNAMIC_PORT, Ipc_Api_OpenDynamicPort);

    //
    // prepare dynamic ports
    //
    
    if (!Mem_GetLockResource(&Ipc_Dynamic_Ports.pPortLock, TRUE))
        return FALSE;
    List_Init(&Ipc_Dynamic_Ports.Ports);

    //
    // finish
    //

    return TRUE;
}


//---------------------------------------------------------------------------
// Ipc_Init_Type
//---------------------------------------------------------------------------


_FX BOOLEAN Ipc_Init_Type(const WCHAR *TypeName, P_Syscall_Handler2 handler)
{
    WCHAR nameW[64];
    UCHAR nameA[64];
    ULONG i, n;

    wcscpy(nameW, L"Open");
    wcscat(nameW, TypeName);
    n = wcslen(nameW);
    for (i = 0; i <= n; ++i)
        nameA[i] = (UCHAR)nameW[i];

    if (! Syscall_Set2(nameA, handler))
        return FALSE;

    wcscpy(nameW, L"Create");
    wcscat(nameW, TypeName);
    n = wcslen(nameW);
    for (i = 0; i <= n; ++i)
        nameA[i] = (UCHAR)nameW[i];

    if (! Syscall_Set2(nameA, handler))
        return FALSE;

    return TRUE;
}


//---------------------------------------------------------------------------
// Ipc_CreateBoxPath
//---------------------------------------------------------------------------


_FX BOOLEAN Ipc_CreateBoxPath(PROCESS *proc)
{
    NTSTATUS status;
    UCHAR sd[128];
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    HANDLE handle;
    WCHAR *ptr;
    ULONG retries;

    //
    // initialize object attributes to create sandbox paths
    //

    RtlCreateSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    RtlSetDaclSecurityDescriptor(&sd, TRUE, NULL, FALSE);
    InitializeObjectAttributes(
        &objattrs, &objname,
        OBJ_CASE_INSENSITIVE | OBJ_PERMANENT | OBJ_KERNEL_HANDLE,
        NULL, &sd);

    //
    // main loop:  try to create sandbox path, removing last path component
    // each time we get STATUS_OBJECT_PATH_NOT_FOUND
    //

    ptr = NULL;

    retries = 0;
    while (retries < 64) {

        ++retries;
        RtlInitUnicodeString(&objname, proc->box->ipc_path);

        status = ZwCreateDirectoryObject(
            &handle, DIRECTORY_ALL_ACCESS, &objattrs);

        if (status == STATUS_OBJECT_PATH_NOT_FOUND) {

            //
            // if parent directories are missing, terminate the
            // path at the last backslash and retry the loop
            //

            WCHAR *ptr2 = wcsrchr(proc->box->ipc_path, L'\\');
            if (ptr)
                *ptr = L'\\';
            ptr = ptr2;
            if (ptr)
                *ptr = L'\0';

        } else {

            //
            // if we could create the path, we check if any backslashes
            // were zeroed out.  if so, we restart the loop from the top
            // using the full path.  otherwise, we're done
            //

            if (NT_SUCCESS(status))
                ZwClose(handle);

            if (status == STATUS_OBJECT_NAME_COLLISION)
                status = STATUS_SUCCESS;

            if (ptr) {

                *ptr = L'\\';
                ptr = NULL;

            } else
                break;
        }
    }

    if (retries >= 64)
        status = STATUS_UNSUCCESSFUL;

    if (! NT_SUCCESS(status)) {
		Log_Status_Ex_Process(MSG_IPC_CREATE_BOX_PATH, 0, status, proc->box->ipc_path, -1, proc->pid);
    }

    return (NT_SUCCESS(status));
}


//---------------------------------------------------------------------------
// Ipc_InitPaths
//---------------------------------------------------------------------------


_FX BOOLEAN Ipc_InitPaths(PROCESS *proc)
{
    static const WCHAR *_OpenPath = L"OpenIpcPath";
    static const WCHAR *_ClosedPath = L"ClosedIpcPath";
    static const WCHAR *openpaths[] = {
        L"\\Windows\\ApiPort",
        L"\\Sessions\\*\\Windows\\ApiPort",
        L"\\Sessions\\*\\Windows\\SharedSection",
        L"\\Sessions\\*\\BaseNamedObjects\\CrSharedMem_*",      // now required by Chromium browsers
        L"\\ThemeApiPort",
        L"\\KnownDlls\\*",              // see Ipc_Generic_MyOpenProc_2
#ifdef _WIN64
        L"\\KnownDlls32\\*",            // see Ipc_Generic_MyOpenProc_2
#endif _WIN64
        L"\\NLS\\*",
        L"*\\BaseNamedObjects*\\ShimCacheMutex",
        L"*\\BaseNamedObjects*\\ShimSharedMemory",
        L"*\\BaseNamedObjects*\\SHIMLIB_LOG_MUTEX",
        L"\\Security\\LSA_AUTHENTICATION_INITIALIZED",
        L"\\LsaAuthenticationPort",
        L"\\NlsCacheMutant",
        L"\\KernelObjects\\*",
        //
        // misc services
        //
        L"\\NLAPublicPort",
        L"\\RPC Control\\nlaapi",
        L"\\RPC Control\\tapsrvlpc",
        L"\\RPC Control\\senssvc",
        L"\\RPC Control\\samss lpc",
        L"*\\BaseNamedObjects*\\SENS Information Cache",
        L"*\\BaseNamedObjects*\\TabletHardwarePresent",
        L"*\\BaseNamedObjects*\\userenv: * Group Policy has been applied",
        L"*\\BaseNamedObjects*\\TermSrvReadyEvent",
        //
        // network
        //
        L"\\RPC Control\\dhcpcsvc",
        L"\\RPC Control\\dhcpcsvc6",
        L"\\RPC Control\\DNSResolver",
        L"\\RPC Control\\RasmanRpc",
        L"*\\BaseNamedObjects*\\WininetStartupMutex",
        L"*\\BaseNamedObjects*\\WininetConnectionMutex",
        L"*\\BaseNamedObjects*\\WininetProxyRegistryMutex",
        L"*\\BaseNamedObjects*\\RasPbFile",
        //
        // cicero multi-language subsystem
        //
        L"*\\BaseNamedObjects*\\CTF.*",
        L"*\\BaseNamedObjects*\\MSCTF.*",
        L"*\\BaseNamedObjects*\\MSUIM.*",
        L"*\\BaseNamedObjects*\\CtfmonInstMutex*",
        L"*\\BaseNamedObjects*\\CiceroSharedMemDefault*",
        L"*\\BaseNamedObjects*\\CicLoadWinStaWinSta*",
        //
        // sysinternals dbgview
        //
        L"*\\BaseNamedObjects*\\DBWinMutex",
        L"*\\BaseNamedObjects*\\DBWIN_BUFFER",
        L"*\\BaseNamedObjects*\\DBWIN_BUFFER_READY",
        L"*\\BaseNamedObjects*\\DBWIN_DATA_READY",
        //
        // multimedia
        //
        L"\\RPC Control\\AudioSrv",
        L"*\\BaseNamedObjects*\\mmGlobalPnpInfo",
        L"*\\BaseNamedObjects*\\Guard*mmGlobalPnpInfoGuard",
        L"*\\BaseNamedObjects*\\MidiMapper_modLongMessage_RefCnt",
        L"*\\BaseNamedObjects*\\MidiMapper_Configure",
        L"*\\BaseNamedObjects*\\SsiMidiDllCs",
        L"*\\BaseNamedObjects*\\StaccatoSynthCore11Mutex",
        L"*\\BaseNamedObjects*\\WDMAUD_Callbacks",
        L"*\\BaseNamedObjects*\\DirectSound*",
        L"*\\BaseNamedObjects*\\AMResourceMutex*",
        L"*\\BaseNamedObjects*\\AMResourceMapping*",
        L"*\\BaseNamedObjects*\\VideoRenderer",
        L"*\\BaseNamedObjects*\\VIDEOMEMORY",
        L"*\\BaseNamedObjects*\\mxrapi",
        L"*\\BaseNamedObjects*\\mixercallback",
        L"*\\BaseNamedObjects*\\hardwaremixercallback",
        L"*\\BaseNamedObjects*\\DINPUTWINMM",
        L"*\\BaseNamedObjects*\\DDrawDriverObjectListMutex",
        L"*\\BaseNamedObjects*\\__DDrawExclMode__",
        L"*\\BaseNamedObjects*\\__DDrawCheckExclMode__",
        L"*\\BaseNamedObjects*\\DDrawWindowListMutex",
        L"*\\BaseNamedObjects*\\DDrawCheckFullscreenSemaphore",
        L"*\\BaseNamedObjects*\\D3D9CheckFullscreenSemaphore",
        L"*\\BaseNamedObjects*\\WinMMConsoleAudioEvent",
        L"*\\BaseNamedObjects*\\SYSTEM_AUDIO_STREAM_*",     // AudioDg
        // following resource is needed on Windows Vista and later,
        // but also provided by the SRS Audio Sandbox program
        L"*\\BaseNamedObjects*\\AudioEngineDuplicateHandleApiPort*",
        // nComputing audio server/driver ?
        L"\\RPC Control\\NCWTSAudioServer",
        //
        // printer
        //
        L"\\RPC Control\\spoolss",
        L"*\\BaseNamedObjects*\\EPSON-PrgMtr-*",
        L"*\\BaseNamedObjects*\\RouterPreInitEvent",
        //
        // sandboxie service
        //
        L"\\RPC Control\\" SBIESVC L"Port",
        L"*\\BaseNamedObjects*\\" SANDBOXIE L"_StartMenu_WorkArea_*",
        //
        // third party
        //
        L"*\\BaseNamedObjects*\\ATITRAY_SMEM",
        L"*\\BaseNamedObjects*\\ATITRAY_OSDM",
        L"*\\BaseNamedObjects*\\AMCreateListenSock*",
        L"*\\BaseNamedObjects*\\AMIPC_*",       // Ad Muncher
        L"*\\BaseNamedObjects*\\devldr32",      // DevLdr32 sound card driver
        L"*\\BaseNamedObjects*\\ThreatfireApiHookIpc2Map",  // ThreatFire
        //
        // third party - key-logger-related
        //
        L"*\\BaseNamedObjects*\\00MemoryShareKeyloggerHunter",
        //
        // hardware - wacom tablet
        //
        L"*\\BaseNamedObjects*\\WacomNewFrontAppEventName",
        L"*\\BaseNamedObjects*\\WacomTouchingAppNameMutexName",
        //
        // end
        //
        L"\\...\\*",        // objects in an unnamed directory
        NULL
    };
    static const WCHAR *openpaths_vista[] = {
        //
        // misc services (vista)
        //
        L"\\RPC Control\\SLCTransportEndpoint-*",       // licensing
        L"\\RPC Control\\wpcsvc",                       // parental controls
        L"*\\BaseNamedObjects*\\BFE_Notify_Event_*",    // firewall
        //
        // multimedia (vista)
        //
        L"\\UxSmsApiPort",
        L"\\MmcssApiPort",
        L"*\\BaseNamedObjects*\\Dwm-*-ApiPort-*",       // aero
        L"*\\BaseNamedObjects*\\DwmDxBltEvent*",        // aero
        L"\\RPC Control\\AudioClientRpc",
#ifdef _WIN64
        //
        // 64-bit spooler for 32-bit programs
        //
        L"*\\BaseNamedObjects*\\WinSpl64To32Mutex*_0",
        L"\\RPC Control\\splwow64_*_0",
        L"\\RPC Control\\umpdproxy_*_0",
#endif _WIN64
        NULL
    };
    static const WCHAR *openpaths_windows7[] = {
        L"\\ConsoleEvent-0x*",
        L"*\\BaseNamedObjects*\\ConsoleEvent-0x*",
        L"\\RPC Control\\console-0x*-lpc-handle",
        L"\\RPC Control\\ConsoleEvent-0x*",
        L"\\RPC Control\\ConsoleLPC-0x*",
        L"\\RPC Control\\lsapolicylookup",
        L"\\RPC Control\\lsasspirpc",
        L"\\RPC Control\\LSARPC_ENDPOINT",
        L"\\RPC Control\\umpo",
        L"*\\BaseNamedObjects*\\FlipEx*",
        L"*\\BaseNamedObjects*\\FontCachePort",
        L"*\\BaseNamedObjects*\\FntCache-*",
        NULL
    };
    static const WCHAR *openpaths_windows8[] = {
        L"\\Windows\\Theme*",
        L"\\Sessions\\*\\Windows\\Theme*",
        L"\\Sessions\\*\\Windows\\DwmApiPort",
        L"*\\BaseNamedObjects*\\DWM_DX_FULLSCREEN_TRANSITION_EVENT",
#ifdef _WIN64
        //
        // 64-bit spooler for 32-bit programs
        //
        L"*\\BaseNamedObjects*\\WinSpl64To32Mutex*_2000",
        L"\\RPC Control\\splwow64_*_2000",
        L"\\RPC Control\\umpdproxy_*_2000",
#endif _WIN64
        NULL
    };
    static const WCHAR *openpaths_windows10[] = {
        L"*\\BaseNamedObjects*\\CoreMessagingRegistrar",
        L"\\RPC Control\\webcache_*",
        L"*\\BaseNamedObjects\\windows_webcache_counters_*",
        L"*\\BaseNamedObjects\\[CoreUI]-*",
        NULL
    };

    ULONG i;
    BOOLEAN ok;

    //
    // open paths
    //

    ok = Process_GetPaths(proc, &proc->open_ipc_paths, _OpenPath, FALSE);

    //
    // if configuration option OpenProtectedStorage applies,
    // then allow access to ProtectedStorage objects
    //

    if (ok && Conf_Get_Boolean(
                proc->box->name, Driver_OpenProtectedStorage, 0, FALSE)) {

        static const WCHAR *_PstEvent =
            L"*\\BaseNamedObjects*\\PS_SERVICE_STARTED";
        static const WCHAR *_PstPort =
            L"\\RPC Control\\protected_storage";

        ok = Process_AddPath(
            proc, &proc->open_ipc_paths, NULL, TRUE, _PstEvent, FALSE);
        if (ok) {
            ok = Process_AddPath(
                proc, &proc->open_ipc_paths, NULL, TRUE, _PstPort, FALSE);
        }
    }

    //
    // add default/built-in open paths
    //

    if (ok) {

        for (i = 0; openpaths[i] && ok; ++i) {
            ok = Process_AddPath(proc, &proc->open_ipc_paths, NULL,
                                 TRUE, openpaths[i], FALSE);
        }
    }

    if (ok && Driver_OsVersion >= DRIVER_WINDOWS_VISTA) {

        for (i = 0; openpaths_vista[i] && ok; ++i) {
            ok = Process_AddPath(proc, &proc->open_ipc_paths, NULL,
                                 TRUE, openpaths_vista[i], FALSE);
        }
    }

    if (ok && Driver_OsVersion >= DRIVER_WINDOWS_7) {

        for (i = 0; openpaths_windows7[i] && ok; ++i) {
            ok = Process_AddPath(proc, &proc->open_ipc_paths, NULL,
                                 TRUE, openpaths_windows7[i], FALSE);
        }
    }

    if (ok && Driver_OsVersion >= DRIVER_WINDOWS_8) {

        for (i = 0; openpaths_windows8[i] && ok; ++i) {
            ok = Process_AddPath(proc, &proc->open_ipc_paths, NULL,
                                 TRUE, openpaths_windows8[i], FALSE);
        }
    }

    if (ok && Driver_OsVersion >= DRIVER_WINDOWS_10) {

        for (i = 0; openpaths_windows10[i] && ok; ++i) {
            ok = Process_AddPath(proc, &proc->open_ipc_paths, NULL,
                TRUE, openpaths_windows10[i], FALSE);
        }
    }

    if (ok) {
        static const WCHAR *_ReturnToWelcome =
            L"*\\BaseNamedObjects*\\msgina: ReturnToWelcome";
        ok = Process_AddPath(proc, &proc->open_ipc_paths, NULL,
                             FALSE, _ReturnToWelcome, FALSE);
    }

    if (! ok) {
        Log_MsgP1(MSG_INIT_PATHS, _OpenPath, proc->pid);
        return FALSE;
    }

    //
    // closed paths
    //

    ok = Process_GetPaths(proc, &proc->closed_ipc_paths, _ClosedPath, FALSE);
    if (! ok) {
        Log_MsgP1(MSG_INIT_PATHS, _ClosedPath, proc->pid);
        return FALSE;
    }

    proc->ipc_warn_startrun = Conf_Get_Boolean(
        proc->box->name, L"NotifyStartRunAccessDenied", 0, TRUE);

    //
    // block password
    //

    proc->ipc_block_password =
        Conf_Get_Boolean(proc->box->name, L"BlockPassword", 0, TRUE); // OpenLsaSSPI (Security Support Provider Interface)

    proc->ipc_open_lsa_endpoint =
        Conf_Get_Boolean(proc->box->name, L"OpenLsaEndpoint", 0, FALSE);

    proc->ipc_open_sam_endpoint =
        Conf_Get_Boolean(proc->box->name, L"OpenSamEndpoint", 0, FALSE);

    proc->ipc_allowSpoolerPrintToFile =
        Conf_Get_Boolean(proc->box->name, L"AllowSpoolerPrintToFile", 0, FALSE);

    proc->ipc_openPrintSpooler =
        Conf_Get_Boolean(proc->box->name, L"OpenPrintSpooler", 0, FALSE);

    //
    // if process is launched as a COM server process by DcomLaunch service
    // outside the sandbox, then we will need to restart it as Start.exe
    // (see also Custom_ComServer in core/dll/custom.c)
    //

    if (ok && Ipc_IsComServer(proc)) {

        proc->untouchable = TRUE;
    }

    //
    // finish
    //

    return TRUE;
}


//---------------------------------------------------------------------------
// Ipc_IsComServer
//---------------------------------------------------------------------------


_FX BOOLEAN Ipc_IsComServer(PROCESS *proc)
{
    HANDLE ParentId;
    PROCESS *pproc;

    //
    // we conclude this is a COM server process if...
    //
    // - the process is forced
    //

    if (! proc->forced_process)
        return FALSE;

    //
    // - the executable image is iexplore.exe or wmplayer.exe or winamp.exe
    //   or kmplayer.exe    (from outside the sandbox)
    //

    if (proc->image_from_box)
        return FALSE;

    if (_wcsicmp(proc->image_name, L"iexplore.exe") != 0 &&
        _wcsicmp(proc->image_name, L"wmplayer.exe") != 0 &&
        _wcsicmp(proc->image_name, L"winamp.exe")   != 0 &&
        _wcsicmp(proc->image_name, L"kmplayer.exe") != 0) {

        return FALSE;
    }

    //
    // - parent is not sandboxed
    //

    MyGetParentId(&ParentId);
    if (! ParentId)
        return FALSE;

    pproc = Process_Find(ParentId, NULL);
    if (pproc)
        return FALSE;

    //
    // - parent process is a system process
    //

    if (! MyIsProcessRunningAsSystemAccount(ParentId))
        return FALSE;

    //
    // process is most likely a COM server process
    //

    return TRUE;
}


//---------------------------------------------------------------------------
// Ipc_InitProcess
//---------------------------------------------------------------------------


_FX BOOLEAN Ipc_InitProcess(PROCESS *proc)
{
    BOOLEAN ok = Ipc_InitPaths(proc);

    //
    // finish
    //

    return ok;
}


//---------------------------------------------------------------------------
// Ipc_IsRunRestricted
//---------------------------------------------------------------------------


_FX BOOLEAN Ipc_IsRunRestricted(PROCESS *proc)
{
    //
    // check Start/Run restrictions
    // issue message SBIE1308 when Start/Run restrictions apply
    //

    PATTERN *pattern = List_Head(&proc->closed_ipc_paths);
    while (pattern) {

        const WCHAR *source = Pattern_Source(pattern);
        if (source[0] == L'*' && source[1] == L'\0') {

            if (proc->ipc_warn_startrun) {

                Process_LogMessage(proc, MSG_STARTRUN_ACCESS_DENIED);
                proc->ipc_warn_startrun = FALSE;
            }

            return TRUE;
        }

        pattern = List_Next(pattern);
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// Ipc_CheckGenericObject
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_CheckGenericObject(
    PROCESS *proc, void *Object, UNICODE_STRING *Name,
    ACCESS_MASK GrantedAccess)
{
    NTSTATUS status;
    BOOLEAN IsBoxedPath;

    // If the client port object is unnamed, check the server port object. This happens with dynamic ports like the spooler and WPAD.
    // (and possibly others)

    if (!Name->Length)
    {
        OBJECT_NAME_INFORMATION *ServerPortName;
        ULONG NameLength;

        status = Obj_GetName(proc->pool, Object, &ServerPortName, &NameLength);

        if (ServerPortName && ServerPortName->Name.Buffer &&  ServerPortName->Name.Length)
        {
            //DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, "Ipc_CheckGenericObject Server Name = %S\n", ServerPortName->Name.Buffer);
            Name = &ServerPortName->Name;   // use the server name
        }
    }

    //
    // allow unconditional access to unnamed objects
    //

    if (! Name->Length)
        return STATUS_SUCCESS;

    ///
    // check if the specified path leads inside the box
    //

    IsBoxedPath = FALSE;

    if (Box_IsBoxedPath(proc->box, ipc, Name))
        IsBoxedPath = TRUE;

    //
    // allow/deny rules:
    // if path leads inside the sandbox, we allow access
    //

    status = STATUS_SUCCESS;

    if (! IsBoxedPath) {

        const WCHAR *pattern;
        BOOLEAN is_open, is_closed;

        //
        // deny access in two cases:
        // - if unsandboxed path matches a closed path
        // - if unsandboxed path does not match an open path
        //

        pattern = Process_MatchPath(
            proc->pool,
            Name->Buffer, Name->Length / sizeof(WCHAR),
            &proc->open_ipc_paths, &proc->closed_ipc_paths,
            &is_open, &is_closed);

        //
        // KnownDll objects:  prevent DELETE access
        //

        if (is_open && pattern[0] == L'\\' && pattern[1] == L'K'
                    && (wcsncmp(pattern, L"\\KnownDlls", 10) == 0)) { // L"\\KnownDlls\\*", L"\\KnownDlls32\\*",

            if (GrantedAccess & (DELETE | SECTION_EXTEND_SIZE))
                status = STATUS_ACCESS_DENIED;
        }


        else if (!is_open && !is_closed)
        {
            if (Ipc_Dynamic_Ports.pPortLock)
            {
                KeEnterCriticalRegion();
                ExAcquireResourceSharedLite(Ipc_Dynamic_Ports.pPortLock, TRUE);
        
                IPC_DYNAMIC_PORT* port = List_Head(&Ipc_Dynamic_Ports.Ports);
                while (port) 
                {    
                    if (_wcsicmp(Name->Buffer, port->wstrPortName) == 0)
                    {
                        // dynamic version of RPC ports, see also ipc_spl.c
                        // and RpcBindingFromStringBindingW in core/dll/rpcrt.c
                        is_open = TRUE;
                        break;
                    }

                    port = List_Next(port);
                }
        
                ExReleaseResourceLite(Ipc_Dynamic_Ports.pPortLock);
                KeLeaveCriticalRegion();
            }
        }

        if (is_closed || (! is_open))
            status = STATUS_ACCESS_DENIED;
    }

    //
    // trace the request if so desired
    //

    if (proc->ipc_trace & (TRACE_ALLOW | TRACE_DENY)) {

        WCHAR access_str[24];
        WCHAR letter;

        if ((! NT_SUCCESS(status)) && (proc->ipc_trace & TRACE_DENY))
            letter = L'D';
        else if (NT_SUCCESS(status) && (proc->ipc_trace & TRACE_ALLOW))
            letter = L'A';
        else
            letter = 0;

        if (letter) {
            WCHAR *backslash = wcsrchr(Name->Buffer, L'\\');
            if (backslash) {
                ++backslash;
                if (wcscmp(backslash, L"DBWinMutex")            == 0 ||
                    wcscmp(backslash, L"DBWIN_BUFFER")          == 0 ||
                    wcscmp(backslash, L"DBWIN_BUFFER_READY")    == 0 ||
                    wcscmp(backslash, L"DBWIN_DATA_READY")      == 0) {

                        letter = 0;
                }
            }
        }

        if (letter) {

            ULONG mon_type = MONITOR_IPC;
            if (!IsBoxedPath) {
                if (NT_SUCCESS(status))
                    mon_type |= MONITOR_OPEN;
                else
                    mon_type |= MONITOR_DENY;
            }

            RtlStringCbPrintfW(access_str, sizeof(access_str), L"(I%c) %08X", letter, GrantedAccess);
            Log_Debug_Msg(mon_type, access_str, Name->Buffer);
        }
    }

    else if (Session_MonitorCount) {

        ULONG mon_type = MONITOR_IPC;
        WCHAR *mon_name = Name->Buffer;
        if (IsBoxedPath)
            mon_name += proc->box->ipc_path_len / sizeof(WCHAR) - 1;
        else if (NT_SUCCESS(status))
            mon_type |= MONITOR_OPEN;
        else
            mon_type |= MONITOR_DENY;
        Session_MonitorPut(mon_type, mon_name, proc->pid);
    }

    // DbgPrint("Process <%06d> Status <%08X> Object <%S>\n", proc->pid, status, Name->Name.Buffer);

    return status;
}


//---------------------------------------------------------------------------
// Ipc_CheckPortObject
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_CheckPortObject(
    PROCESS *proc, void *Object, UNICODE_STRING *Name,
    ACCESS_MASK GrantedAccess)
{
    void *PortObject = Ipc_GetServerPort(Object);

    if (! PortObject)
        return STATUS_SUCCESS;

    return Ipc_CheckGenericObject(proc, PortObject, Name, GrantedAccess);
}


//---------------------------------------------------------------------------
// Ipc_CheckJobObject
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_CheckJobObject(
    PROCESS *proc, void *Object, UNICODE_STRING *Name,
    ACCESS_MASK GrantedAccess)
{
    //
    // we don't mind if a program in the sandbox creates or opens a job
    // object, but the job object must be named (as opposed to other IPC
    // objects), and must not include the right to assign to processes.
    // Ipc_CheckGenericObject additionally makes sure the job name path
    // is inside the sandbox
    //

    if (!Conf_Get_Boolean(proc->box->name, L"NoAddProcessToJob", 0, FALSE))
    if (GrantedAccess & (JOB_OBJECT_ASSIGN_PROCESS | JOB_OBJECT_TERMINATE))
        return STATUS_ACCESS_DENIED;

    if (! Name->Length)
        return STATUS_ACCESS_DENIED;

    return Ipc_CheckGenericObject(proc, Object, Name, GrantedAccess);
}


//---------------------------------------------------------------------------
// Ipc_Api_DuplicateObject
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_Api_DuplicateObject(PROCESS *proc, ULONG64 *parms)
{
    API_DUPLICATE_OBJECT_ARGS *args = (API_DUPLICATE_OBJECT_ARGS *)parms;
    PEPROCESS OtherProcessObject;
    HANDLE OtherProcessHandle;
    HANDLE SourceProcessHandle;
    HANDLE SourceHandle;
    HANDLE TargetProcessHandle;
    HANDLE *TargetHandle;
    HANDLE TestHandle;
    ULONG DesiredAccess;
    ULONG HandleAttributes;
    ULONG Options;
    NTSTATUS status;

    //
    // this API must be invoked by a sandboxed process
    //

    if (! proc)
        return STATUS_NOT_IMPLEMENTED;

    //
    // collect and verify parameters
    //

    SourceHandle        = args->source_handle.val;
    TargetHandle        = args->target_handle.val;
    OtherProcessHandle  = args->process_handle.val;

    ProbeForWrite(TargetHandle, sizeof(HANDLE), sizeof(ULONG_PTR));

    DesiredAccess       = args->desired_access.val;
    Options             = args->options.val;
    HandleAttributes    = 0;
    if (Options & DUPLICATE_INHERIT) {
        HandleAttributes |= OBJ_INHERIT;
        Options &= ~DUPLICATE_INHERIT;
    }

    //
    // other process must not be sandboxed.  this makes sure this API
    // is not used for communication by processes running in two
    // different sandboxes
    //

    status = ObReferenceObjectByHandle(OtherProcessHandle, 0, *PsProcessType,
                                       UserMode, &OtherProcessObject, NULL);

    if (! NT_SUCCESS(status))
        return status;

    if (Process_Find(PsGetProcessId(OtherProcessObject), NULL)) {
        // other process is sandboxed
        ObDereferenceObject(OtherProcessObject);
        return STATUS_ACCESS_DENIED;
    }

    //
    // if the caller holds SeDebugPrivilege, we allow duplication from
    // any other process.  otherwise, only from processes with same SID
    //

    if (! SeSinglePrivilegeCheck(
            RtlConvertLongToLuid(SE_DEBUG_PRIVILEGE), ExGetPreviousMode())) {

        UNICODE_STRING SidString;
        ULONG SessionId;

        status = Process_GetSidStringAndSessionId(
                        OtherProcessHandle, NULL, &SidString, &SessionId);
        if (NT_SUCCESS(status)) {

            if (_wcsicmp(proc->box->sid, SidString.Buffer) != 0)
                status = STATUS_ACCESS_DENIED;

            RtlFreeUnicodeString(&SidString);
        }

        if (! NT_SUCCESS(status)) {
            ObDereferenceObject(OtherProcessObject);
            return status;
        }
    }

    //
    // open the other process for PROCESS_DUP_HANDLE access
    //

    status = ObOpenObjectByPointer(
                OtherProcessObject, 0, NULL, PROCESS_DUP_HANDLE,
                *PsProcessType, UserMode, &OtherProcessHandle);

    ObDereferenceObject(OtherProcessObject);

    if (! NT_SUCCESS(status))
        return status;

    if (Options & DUPLICATE_INTO_OTHER) {

        SourceProcessHandle = NtCurrentProcess();
        TargetProcessHandle = OtherProcessHandle;
        Options &= ~DUPLICATE_INTO_OTHER;

    } else {

        SourceProcessHandle = OtherProcessHandle;
        TargetProcessHandle = NtCurrentProcess();
    }

    //
    // set up an exception catching block so we can always close
    // the powerful OtherProcessHandle
    //

    __try {

    //
    // if the source handle is in the current process, make sure it is
    // an unnamed object, before trying to duplicate it.  this makes sure
    // NtDuplicateObject is used for inter-process exchange of private
    // handles, and not to hijack access to objects outside the sandbox
    //

    if (IS_ARG_CURRENT_PROCESS(SourceProcessHandle)) {

        status = Ipc_CheckObjectName(SourceHandle, UserMode);

    //
    // if the source handle is in another process, we have to duplicate
    // it before we can examine it, so duplicate first without the
    // DUPLICATE_CLOSE_SOURCE option
    //

    } else if (IS_ARG_CURRENT_PROCESS(TargetProcessHandle)) {

        //
        // we duplicate the handle into kernel space such that that user 
        // wont be able to grab it while we are evaluaiting it
        //

        status = ZwDuplicateObject(
                        SourceProcessHandle, SourceHandle,
                        TargetProcessHandle, &TestHandle,
                        DesiredAccess, HandleAttributes,
                        Options & ~DUPLICATE_CLOSE_SOURCE);

        if (NT_SUCCESS(status)) {

            status = Ipc_CheckObjectName(TestHandle, KernelMode);

            ZwClose(TestHandle);
        }

    } else
        status = STATUS_INVALID_HANDLE;

    //
    // if all checks were passed duplicate the handle
    //

    if (NT_SUCCESS(status)) {

        status = NtDuplicateObject(
            SourceProcessHandle, SourceHandle,
            TargetProcessHandle, TargetHandle,
            DesiredAccess, HandleAttributes, Options);
    }

    //
    // end exception block and close OtherProcessHandle
    //

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    NtClose(OtherProcessHandle);

    return status;
}


//---------------------------------------------------------------------------
// Ipc_CheckObjectName
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_CheckObjectName(HANDLE handle, KPROCESSOR_MODE mode)
{
    NTSTATUS status;
    OBJECT_TYPE *object;
    USHORT TypeLength;
    WCHAR *TypeBuffer;

    status = ObReferenceObjectByHandle(
                    handle, 0, NULL, mode, &object, NULL);

    if (! NT_SUCCESS(status))
        return status;

    TypeLength = 0;
    TypeBuffer = NULL;

    if (Driver_OsVersion >= DRIVER_WINDOWS_7) {

        //
        // on Windows 7, the new ObQueryNameInfo API returns the offset
        // to the OBJECT_HEADER_NAME_INFO structure, or zero if the
        // object does not have a name
        //

        ULONG NameInfoOffset = pObQueryNameInfo(object);
        if (! NameInfoOffset) {

            //
            // use the new ObGetObjectType API to get the object type
            //

            OBJECT_TYPE_VISTA_SP1 *ObjectType = pObGetObjectType(object);
            TypeLength = ObjectType->Name.Length;
            TypeBuffer = ObjectType->Name.Buffer;
        }

    } else {

        //
        // on earlier versions of Windows, the object header precedes the
        // object body and contains an OBJECT_HEADER_NAME_INFO structure
        // and a direct pointer to the object type
        //

        OBJECT_HEADER *ObjectHeader = OBJECT_TO_OBJECT_HEADER(object);
        OBJECT_HEADER_NAME_INFO *NameInfo =
            OBJECT_HEADER_TO_NAME_INFO(ObjectHeader);

        if (! NameInfo) {

            if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA &&
                Driver_OsBuild > 6000) {

                OBJECT_TYPE_VISTA_SP1 *ObjectType =
                    (OBJECT_TYPE_VISTA_SP1 *)ObjectHeader->Type;

                TypeLength = ObjectType->Name.Length;
                TypeBuffer = ObjectType->Name.Buffer;

            } else {

                OBJECT_TYPE *ObjectType = ObjectHeader->Type;
                TypeLength = ObjectType->Name.Length;
                TypeBuffer = ObjectType->Name.Buffer;
            }
        }

        //DbgPrint("Object %08X Has NameInfo %08X TypeBuffer %*.*S\n", object, NameInfo, TypeLength/sizeof(WCHAR), TypeLength/sizeof(WCHAR), TypeBuffer);
    }

    //
    // if we have the type name here, it means the object is unnamed,
    // and we need to make sure the type name is acceptable
    //

    status = STATUS_ACCESS_DENIED;

    if (TypeLength && TypeBuffer) {

        #define IS_OBJECT_TYPE(l,n)                 \
            (TypeLength == l * sizeof(WCHAR) &&     \
             _wcsnicmp(TypeBuffer, n, l) == 0)

        if (IS_OBJECT_TYPE( 5,Ipc_Event_TypeName)       ||
            IS_OBJECT_TYPE( 9,Ipc_EventPair_TypeName)   ||
            IS_OBJECT_TYPE(10,Ipc_KeyedEvent_TypeName)  ||
            IS_OBJECT_TYPE( 6,Ipc_Mutant_TypeName)      ||
            IS_OBJECT_TYPE( 9,Ipc_Semaphore_TypeName)   ||
            IS_OBJECT_TYPE( 7,Ipc_Section_TypeName)      ) {

            status = STATUS_SUCCESS;
        }

        #undef IS_OBJECT_TYPE
    }

    //
    // finish
    //

    ObDereferenceObject(object);

    return status;
}


//---------------------------------------------------------------------------
// Ipc_Api_CreateDirOrLink
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_Api_CreateDirOrLink(PROCESS *proc, ULONG64 *parms)
{
    API_CREATE_DIR_OR_LINK_ARGS *args =
        (API_CREATE_DIR_OR_LINK_ARGS *)parms;
    NTSTATUS status;
    HANDLE handle;
    UNICODE_STRING64 *user_uni;
    WCHAR *user_buf, *objname_buf = NULL, *target_buf;
    ULONG user_len,  objname_len,  target_len;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname, target;

    //
    // this API must be invoked by a sandboxed process
    //

    if (! proc)
        return STATUS_NOT_IMPLEMENTED;

    status = STATUS_SUCCESS;

    //
    // copy first user parameter: objname
    //

    user_uni = (UNICODE_STRING64 *)args->objname.val;
    ProbeForRead(user_uni, sizeof(UNICODE_STRING64), sizeof(ULONG_PTR));
    user_len = user_uni->Length;
    user_buf = (WCHAR *)(ULONG_PTR)user_uni->Buffer;

    if (user_len >= sizeof(WCHAR) && user_len < 2048 && user_buf) {

        objname_len = user_len & ~1;
        ProbeForRead(user_buf, objname_len, sizeof(WCHAR));
        objname_buf = Mem_Alloc(proc->pool, objname_len + sizeof(WCHAR));
        if (! objname_buf)
            status = STATUS_INSUFFICIENT_RESOURCES;
        else {
            memcpy(objname_buf, user_buf, objname_len);
            objname_buf[objname_len / sizeof(WCHAR)] = L'\0';
        }

    } else
        status = STATUS_INVALID_PARAMETER;

    if (! NT_SUCCESS(status))
        return status;

    //
    // copy second user parameter: target.  note that if we fail here,
    // we don't even bother freeing up memory, since its allocated
    // to the pool of the sandboxed process
    //

    target_buf = NULL;
    target_len = 0;

    user_uni = (UNICODE_STRING64 *)args->target.val;
    if (user_uni) {

        ProbeForRead(user_uni, sizeof(UNICODE_STRING64), sizeof(ULONG_PTR));
        user_len = user_uni->Length;
        user_buf = (WCHAR *)(ULONG_PTR)user_uni->Buffer;

        if (user_len >= sizeof(WCHAR) && user_len < 2048 && user_buf) {

            target_len = user_len & ~1;
            ProbeForRead(user_buf, target_len, sizeof(WCHAR));
            target_buf = Mem_Alloc(proc->pool, target_len + sizeof(WCHAR));
            if (! target_buf)
                status = STATUS_INSUFFICIENT_RESOURCES;
            else {
                memcpy(target_buf, user_buf, target_len);
                target_buf[target_len / sizeof(WCHAR)] = L'\0';
            }

        } else
            status = STATUS_INVALID_PARAMETER;
    }

    if (! NT_SUCCESS(status)) {
        Mem_Free(objname_buf, objname_len + sizeof(WCHAR));
        return status;
    }

    //
    // make sure the paths are within the sandbox
    //

    InitializeObjectAttributes(
        &objattrs, &objname,
        OBJ_CASE_INSENSITIVE | OBJ_PERMANENT | OBJ_KERNEL_HANDLE,
        NULL, Driver_PublicSd);

    RtlInitUnicodeString(&objname, objname_buf);
    if (! Box_IsBoxedPath(proc->box, ipc, &objname))
        status = STATUS_ACCESS_DENIED;

    else if (target_buf) {

        RtlInitUnicodeString(&target, target_buf);
        if (! Box_IsBoxedPath(proc->box, ipc, &target))
            status = STATUS_ACCESS_DENIED;
        else {

            status = ZwCreateSymbolicLinkObject(
                &handle, SYMBOLIC_LINK_ALL_ACCESS, &objattrs, &target);

            if (NT_SUCCESS(status))
                ZwClose(handle);
        }

        Mem_Free(target_buf, target_len + sizeof(WCHAR));

    } else {

        if (Driver_LowLabelSd) {

            //
            // prior to version 3.68, we did not create object directories
            // with a low integrity label.  so to make sure migration is
            // smooth from earlier versions, we use the OBJ_OPENIF flag to
            // force the directory to always open successfully, so that we
            // can call ZwSetSecurityObject
            //
            // in later releases, when it is unlikely to still encounter
            // object directories created without the integrity label, it
            // would be ok to remove the OBJ_OPENIF flag, and only apply
            // the label when actually creating the object directory
            //

            objattrs.Attributes |= OBJ_OPENIF;
        }

        status = ZwCreateDirectoryObject(
            &handle, DIRECTORY_ALL_ACCESS, &objattrs);

        if (NT_SUCCESS(status)) {

            if (Driver_LowLabelSd) {

                ZwSetSecurityObject(
                    handle, LABEL_SECURITY_INFORMATION, Driver_LowLabelSd);
            }

            ZwClose(handle);
        }
    }

    Mem_Free(objname_buf, objname_len + sizeof(WCHAR));

    if (status == STATUS_OBJECT_NAME_COLLISION)
        status = STATUS_SUCCESS;

    return status;
}


//---------------------------------------------------------------------------
// Ipc_Api_OpenDeviceMap
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_Api_OpenDeviceMap(PROCESS *proc, ULONG64 *parms)
{
    API_OPEN_DEVICE_MAP_ARGS *args =
        (API_OPEN_DEVICE_MAP_ARGS *)parms;
    NTSTATUS status;
    HANDLE handle;
    UNICODE_STRING objname;
    OBJECT_ATTRIBUTES objattrs;

    //
    // this API must be invoked by a sandboxed process
    //

    if (! proc)
        return STATUS_NOT_IMPLEMENTED;

    //
    // open the device map for the current process
    //

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    RtlInitUnicodeString(&objname, L"\\??");

    status = ZwOpenDirectoryObject(&handle, 0, &objattrs);

    if (! NT_SUCCESS(status))
        return status;

    //
    // copy handle into user space
    //

    __try {

        HANDLE *user_handle = args->handle.val;
        ProbeForRead(user_handle, sizeof(HANDLE), sizeof(HANDLE));
        *user_handle = handle;

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    if (! NT_SUCCESS(status))
        NtClose(handle);

    return status;
}


//---------------------------------------------------------------------------
// Ipc_Api_QuerySymbolicLink
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_Api_QuerySymbolicLink(PROCESS *proc, ULONG64 *parms)
{
    API_QUERY_SYMBOLIC_LINK_ARGS *args =
        (API_QUERY_SYMBOLIC_LINK_ARGS *)parms;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    HANDLE handle;
    WCHAR *buf;
    WCHAR *user_buf;
    ULONG user_len;
    NTSTATUS status;

    //
    // this API must be invoked by a sandboxed process
    //

    if (! proc)
        return STATUS_NOT_IMPLEMENTED;

    //
    // check input buffers
    //

    user_buf = args->name_buf.val;
    user_len = args->name_len.val / sizeof(WCHAR);
    if ((! user_buf) || (! user_len) || (user_len > 4096))
        return STATUS_INVALID_PARAMETER;

    //
    // copy user object name into kernel buffer
    //

    buf = Mem_Alloc(proc->pool, (user_len + 8) * sizeof(WCHAR));
    if (! buf)
        return STATUS_INSUFFICIENT_RESOURCES;
    ProbeForRead(user_buf, sizeof(WCHAR) * user_len, sizeof(WCHAR));
    wmemcpy(buf, user_buf, user_len);
    buf[user_len] = L'\0';

    RtlInitUnicodeString(&objname, buf);

    InitializeObjectAttributes(&objattrs,
        &objname, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);

    // DbgPrint("Process %06d ||| Query <%S>\n", PsGetCurrentProcessId(), buf);

    //
    // open and query symbolic link into kernel buffer
    //

    status = ZwOpenSymbolicLinkObject(
                &handle, SYMBOLIC_LINK_QUERY, &objattrs);

    if (NT_SUCCESS(status)) {

        objname.Length = (USHORT)(user_len * sizeof(WCHAR));
        objname.MaximumLength = objname.Length;
        objname.Buffer = buf;
        status = ZwQuerySymbolicLinkObject(handle, &objname, NULL);

        ZwClose(handle);
    }

    //
    // write kernel buffer into user buffer
    //

    if (NT_SUCCESS(status)) {

        __try {

            ULONG len = objname.Length / sizeof(WCHAR);
            if (len >= user_len - 1)
                status = STATUS_BUFFER_TOO_SMALL;
            else {
                buf[len] = L'\0';
                ProbeForRead(
                    user_buf, sizeof(WCHAR) * (len + 1), sizeof(WCHAR));
                wmemcpy(user_buf, buf, len + 1);
            }

        } __except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
        }
    }

    // DbgPrint("Process %06d ||| Status <%08X>\n", PsGetCurrentProcessId(), status);

    Mem_Free(buf, (user_len + 8) * sizeof(WCHAR));

    return status;
}

//---------------------------------------------------------------------------
// Api_Unload
//---------------------------------------------------------------------------


_FX void Ipc_Unload(void)
{
    if (Ipc_Dynamic_Ports.pPortLock)
        Mem_FreeLockResource(&Ipc_Dynamic_Ports.pPortLock);
}