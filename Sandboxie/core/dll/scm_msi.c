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

#include "handle.h"

//---------------------------------------------------------------------------
// Service Control Manager
//---------------------------------------------------------------------------


//#define ErrorMessageBox(txt)
//
//#define HOOK_WIN32(func) {                                                  \
//    const char *FuncName = #func;                                           \
//    void *SourceFunc = Ldr_GetProcAddrNew(DllName_advapi32, L#func, #func); \
//    if (!SourceFunc && Dll_KernelBase) {                                    \
//        SourceFunc = GetProcAddress(Dll_KernelBase, FuncName);              \
//    }                                                                       \
//    __sys_##func =                                                          \
//        (ULONG_PTR)SbieDll_Hook(FuncName, SourceFunc, my_##func);           \
//    if (! __sys_##func)                                                     \
//        hook_success = FALSE;                                               \
//    }
//
//
//#include "../../apps/com/privs.h"

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

static HANDLE Scm_CreateWaitableTimerW(
    LPSECURITY_ATTRIBUTES lpTimerAttributes,
    BOOL bManualReset, LPCWSTR lpTimerName);

static BOOL Scm_OpenProcessToken(
    _In_ HANDLE ProcessHandle,
    _In_ DWORD DesiredAccess,
    _Outptr_ PHANDLE TokenHandle
    );

static BOOL Scm_OpenThreadToken(
    _In_ HANDLE  ThreadHandle,
    _In_ DWORD   DesiredAccess,
    _In_ BOOL    OpenAsSelf,
    _Outptr_ PHANDLE TokenHandle
    );

static BOOL Scm_GetTokenInformation(
    _In_ HANDLE TokenHandle,
    _In_ TOKEN_INFORMATION_CLASS TokenInformationClass,
    _Out_writes_bytes_to_opt_(TokenInformationLength,*ReturnLength) LPVOID TokenInformation,
    _In_ DWORD TokenInformationLength,
    _Out_ PDWORD ReturnLength
    );

//---------------------------------------------------------------------------
// Prototypes
//---------------------------------------------------------------------------


typedef HANDLE (*P_CreateWaitableTimerW)(
    LPSECURITY_ATTRIBUTES lpTimerAttributes,
    BOOL bManualReset, LPCWSTR lpTimerName);

typedef BOOL (*P_OpenProcessToken)(
    _In_ HANDLE ProcessHandle,
    _In_ DWORD DesiredAccess,
    _Outptr_ PHANDLE TokenHandle
    );

typedef BOOL (*P_OpenThreadToken)(
    _In_ HANDLE  ThreadHandle,
    _In_ DWORD   DesiredAccess,
    _In_ BOOL    OpenAsSelf,
    _Outptr_ PHANDLE TokenHandle
    );

typedef BOOL (*P_GetTokenInformation)(
    _In_ HANDLE TokenHandle,
    _In_ TOKEN_INFORMATION_CLASS TokenInformationClass,
    _Out_writes_bytes_to_opt_(TokenInformationLength,*ReturnLength) LPVOID TokenInformation,
    _In_ DWORD TokenInformationLength,
    _Out_ PDWORD ReturnLength
    );

//---------------------------------------------------------------------------
// Pointers
//---------------------------------------------------------------------------


static P_CreateWaitableTimerW       __sys_CreateWaitableTimerW = NULL;

static P_OpenProcessToken           __sys_OpenProcessToken = NULL;
static P_OpenThreadToken            __sys_OpenThreadToken = NULL;

static P_GetTokenInformation        __sys_GetTokenInformation = NULL;

//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------

static volatile BOOLEAN Scm_IsMsiServer = FALSE;
BOOLEAN Scm_MsiServer_Systemless = FALSE;

static const WCHAR *_MsiServerInUseEventName = SBIE L"_WindowsInstallerInUse";

static HANDLE   Msi_ServerInUseEvent = NULL;


//---------------------------------------------------------------------------
// Scm_SetupMsiHooks
//---------------------------------------------------------------------------


_FX BOOLEAN Scm_SetupMsiHooks()
{
    HMODULE module = NULL;

    //while (!IsDebuggerPresent())
    //    Sleep(500);
    //__debugbreak();

    P_CreateWaitableTimerW CreateWaitableTimerW = (P_CreateWaitableTimerW)GetProcAddress(Dll_Kernel32, "CreateWaitableTimerW");
    SBIEDLL_HOOK(Scm_, CreateWaitableTimerW);


    // MSIServer without system - fake running as system
    if (!SbieDll_CheckProcessLocalSystem(GetCurrentProcess()))
    {
        Scm_MsiServer_Systemless = TRUE;

        //
        // To run MSIServer without system privileges we need to make it think it is running as system
        // we do that by hooking OpenProcessToken and if it opened the current process caching the resulting token handle
        // than in GetTokenInformation when asked for TokenUser for this handle we return the system SID
        // finally on NtClose we clear the cached token value in case it gets reused later
        //

        /*
        msi.dll!RunningAsLocalSystem
        v2 = GetCurrentProcess();
        if ( OpenProcessToken(v2, 8u, &hObject) )
        {
            v3 = IsLocalSystemToken(hObject);
        ...
    
        msi.dll!IsLocalSystemToken
        if ( GetUserSID(a1, Sid) )
            return 0;
        StringSid = 0i64;
        if ( !ConvertSidToStringSidW(Sid, &StringSid) )
            return 0;
        v2 = L"S-1-5-18";
        wcscmp...
    

        msi.dll!GetUserSID
        if ( GetTokenInformation(a1, TokenUser, TokenInformation, 0x58u, ReturnLength) )
        {
            if ( CopySid(0x48u, a2, TokenInformation[0]) )
        ...
        */

        HMODULE hAdvapi32 = LoadLibrary(L"Advapi32.dll");

        void* OpenProcessToken = (P_OpenProcessToken)GetProcAddress(hAdvapi32, "OpenProcessToken");
        SBIEDLL_HOOK(Scm_, OpenProcessToken);

        //void* OpenThreadToken = (P_OpenThreadToken)GetProcAddress(hAdvapi32, "OpenThreadToken");
        //SBIEDLL_HOOK(Scm_, OpenThreadToken);


        void* GetTokenInformation = (P_GetTokenInformation)GetProcAddress(hAdvapi32, "GetTokenInformation");
        SBIEDLL_HOOK(Scm_, GetTokenInformation);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Scm_CreateWaitableTimerW
//---------------------------------------------------------------------------


_FX HANDLE Scm_CreateWaitableTimerW(
    LPSECURITY_ATTRIBUTES lpTimerAttributes,
    BOOL bManualReset, LPCWSTR lpTimerName)
{

    //
    // When MsiServer is not started as system (missing the admin groupe) this call fails
    //

    lpTimerAttributes = NULL;

    return __sys_CreateWaitableTimerW(lpTimerAttributes, bManualReset, lpTimerName);
}


//---------------------------------------------------------------------------
// Scm_TokenCloseHandler
//---------------------------------------------------------------------------


_FX VOID Scm_TokenCloseHandler(HANDLE Handle) 
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    if (TlsData->scm_last_own_token == Handle)
        TlsData->scm_last_own_token = NULL;
}


//---------------------------------------------------------------------------
// Scm_OpenProcessToken
//---------------------------------------------------------------------------


_FX BOOL Scm_OpenProcessToken(HANDLE ProcessHandle, DWORD DesiredAccess, PHANDLE phTokenOut)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    NTSTATUS status = __sys_OpenProcessToken(ProcessHandle, DesiredAccess, phTokenOut);

    if (NT_SUCCESS(status) && ProcessHandle == GetCurrentProcess()) {

        Handle_RegisterCloseHandler(*phTokenOut, Scm_TokenCloseHandler);
        TlsData->scm_last_own_token = *phTokenOut;
    }

    return status;
}


//---------------------------------------------------------------------------
// Scm_OpenThreadToken
//---------------------------------------------------------------------------


_FX BOOL Scm_OpenThreadToken(HANDLE ThreadHandle, DWORD DesiredAccess, BOOL OpenAsSelf, PHANDLE phTokenOut)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    NTSTATUS status = __sys_OpenThreadToken(ThreadHandle, DesiredAccess, OpenAsSelf, phTokenOut);

    if (NT_SUCCESS(status) && ThreadHandle == GetCurrentThread()) {

        Handle_RegisterCloseHandler(*phTokenOut, Scm_TokenCloseHandler);
        TlsData->scm_last_own_token = *phTokenOut;
    }

    return status;
}


//---------------------------------------------------------------------------
// Scm_GetTokenInformation
//---------------------------------------------------------------------------


_FX BOOL Scm_GetTokenInformation(HANDLE TokenHandle, TOKEN_INFORMATION_CLASS TokenInformationClass,
    LPVOID TokenInformation, DWORD TokenInformationLength, PDWORD ReturnLength)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    static const UCHAR sid[12] = {
    1,                                      // Revision
    1,                                      // SubAuthorityCount
    0,0,0,0,0,5, // SECURITY_NT_AUTHORITY   // IdentifierAuthority
    SECURITY_LOCAL_SYSTEM_RID               // SubAuthority
    };

    if (TokenInformationClass == TokenUser && TlsData->scm_last_own_token == TokenHandle 
        && TokenInformationLength >= sizeof(TOKEN_USER) + sizeof(sid))
    {
        PTOKEN_USER token_user = (PTOKEN_USER)TokenInformation;
        token_user->User.Sid = (PSID)(((UCHAR*)TokenInformation) + sizeof(TOKEN_USER));
        memcpy(token_user->User.Sid, sid, sizeof(sid));

        *ReturnLength = sizeof(TOKEN_USER) + sizeof(sid);
        return TRUE;
    }

    return __sys_GetTokenInformation(TokenHandle, TokenInformationClass, TokenInformation, TokenInformationLength, ReturnLength);
}


//---------------------------------------------------------------------------
// Scm_SetupMsiWaiter
//---------------------------------------------------------------------------


_FX void Scm_SetupMsiWaiter()
{
    //
    // release our hold on the MSI Server in-use flag
    //

    if (Msi_ServerInUseEvent) {
        CloseHandle(Msi_ServerInUseEvent);
        Msi_ServerInUseEvent = NULL;
    }

    //
    // end the service as soon as the in-use flag is gone
    //

    while (1) {

        HANDLE hEvent = OpenEvent(
            EVENT_MODIFY_STATE, FALSE, _MsiServerInUseEventName);

        if (!hEvent) {
            ExitProcess(0);
            break;
        }

        CloseHandle(hEvent);
        Sleep(2000);
    }
}


//---------------------------------------------------------------------------
// Scm_MsiDll
//---------------------------------------------------------------------------


_FX BOOLEAN Scm_MsiDll(HMODULE module)
{
    //
    // MSI library unloading
    //      XXX - Ldr module no longer does unload notifications
    //            so we might rely on MsiCloseHandle instead
    //

    /* if (! module) {

        if (Msi_ServerInUseEvent) {
            CloseHandle(Msi_ServerInUseEvent);
            Msi_ServerInUseEvent = FALSE;
        }

        return TRUE;
    }*/

    //
    // indicate we are one more process that is using the MSI Server
    //

    if (Dll_ImageType != DLL_IMAGE_SANDBOXIE_RPCSS &&
        Dll_ImageType != DLL_IMAGE_SANDBOXIE_DCOMLAUNCH) {

        if ((!Msi_ServerInUseEvent) && (!Scm_IsMsiServer)) {
            Msi_ServerInUseEvent = CreateEvent(
                NULL, TRUE, FALSE, _MsiServerInUseEventName);
        }
    }

    return TRUE;
}
