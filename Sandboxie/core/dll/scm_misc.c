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

#include "advapi.h"

//---------------------------------------------------------------------------
// Service Control Manager
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static ULONG_PTR Scm_SubscribeServiceChangeNotifications(
    ULONG_PTR Unknown1, ULONG_PTR Unknown2, ULONG_PTR Unknown3,
    ULONG_PTR Unknown4, ULONG_PTR Unknown5);


//---------------------------------------------------------------------------


typedef ULONG_PTR (*P_SubscribeServiceChangeNotifications)(
    ULONG_PTR Unknown1, ULONG_PTR Unknown2, ULONG_PTR Unknown3,
    ULONG_PTR Unknown4, ULONG_PTR Unknown5); // ret 14h

static P_SubscribeServiceChangeNotifications
                            __sys_SubscribeServiceChangeNotifications = NULL;


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static BOOLEAN Scm_SecHostDll_W8_ShouldRun = FALSE;


//---------------------------------------------------------------------------
// Scm_SecHostDll
//---------------------------------------------------------------------------

typedef struct _FuncNamesAW {
    UCHAR       *FuncNameA;
    WCHAR       *FuncNameW;
    ULONG_PTR   *pSyscall;
} FuncNamesAW;

#define FUNCNAME( n ) { #n, L#n, NULL }
#define FUNCNAME2( n ) { #n, L#n, (ULONG_PTR*)&__sys_##n }

_FX BOOLEAN Scm_SecHostDll(HMODULE module)
{
    static FuncNamesAW funcNamesAW[] = { 
        FUNCNAME(ChangeServiceConfigA),
        FUNCNAME(ChangeServiceConfigW),
        FUNCNAME(ChangeServiceConfig2A),
        FUNCNAME(ChangeServiceConfig2W),
        FUNCNAME(CloseServiceHandle),
        FUNCNAME(ControlService),
        FUNCNAME(CreateServiceA),
        FUNCNAME(CreateServiceW),
        FUNCNAME(DeleteService),
        FUNCNAME(OpenSCManagerA),
        FUNCNAME(OpenSCManagerW),
        FUNCNAME(OpenServiceA),
        FUNCNAME(OpenServiceW),
        FUNCNAME(QueryServiceConfigA),
        FUNCNAME(QueryServiceConfigW),
        FUNCNAME(QueryServiceConfig2A),
        FUNCNAME(QueryServiceConfig2W),
        FUNCNAME(QueryServiceObjectSecurity),
        FUNCNAME(QueryServiceStatus),
        FUNCNAME(QueryServiceStatusEx),
        FUNCNAME(RegisterServiceCtrlHandlerA),
        FUNCNAME(RegisterServiceCtrlHandlerW),
        FUNCNAME(RegisterServiceCtrlHandlerExA),
        FUNCNAME(RegisterServiceCtrlHandlerExW),
        FUNCNAME(SetServiceObjectSecurity),
        FUNCNAME(NotifyServiceStatusChangeA),
        FUNCNAME(NotifyServiceStatusChangeW),
        FUNCNAME(SetServiceStatus),
        FUNCNAME(StartServiceA),
        FUNCNAME(StartServiceW),
        FUNCNAME(StartServiceCtrlDispatcherA),
        FUNCNAME(StartServiceCtrlDispatcherW),
        { NULL, NULL }
    };
    ULONG i;
    HMODULE advapi;

    //
    // Windows 8 may not let us load advapi32.dll here, perhaps due to
    // checking circular links (sechost -> advapi32 -> sechost), so set
    // a flag for Scm_SecHostDll_W8
    //

    if (Dll_OsBuild >= 8400) {

        if ((module != (HMODULE)(ULONG_PTR)tzuk)
                                    || Scm_SecHostDll_W8_ShouldRun) {

            if (module)
                Scm_SecHostDll_W8_ShouldRun = TRUE;

            return TRUE;
        }

        module = GetModuleHandle(DllName_sechost);

        if (! module) {
            Scm_SecHostDll_W8_ShouldRun = TRUE;
            return TRUE;
        }
    }

    //
    // support for Windows 7 SecHost:
    // for every function that our AdvApi32 module hooks,
    // we route the corresponding function in SecHost to AdvApi32
    //

    advapi = LoadLibrary(DllName_advapi32);
    if (! advapi) {
        if (Dll_OsBuild >= 8400) {
            Scm_SecHostDll_W8_ShouldRun = TRUE;
            return TRUE;
        } else {
            SbieApi_Log(2205, L"SecHost");
            return FALSE;
        }
    }

    for (i = 0; funcNamesAW[i].FuncNameA; ++i) {

        void *ResPtr;
        void *SecPtr = Ldr_GetProcAddrNew(DllName_sechost, funcNamesAW[i].FuncNameW,(char *)funcNamesAW[i].FuncNameA);
        void *AdvPtr = Ldr_GetProcAddrNew(DllName_advapi32, funcNamesAW[i].FuncNameW,(char *)funcNamesAW[i].FuncNameA);
        if ((! SecPtr) || (! AdvPtr)) {
            SbieApi_Log(2303, L"%s (SEC)", funcNamesAW[i].FuncNameA);
            return FALSE;
        }

        ResPtr = SbieDll_Hook((char *)funcNamesAW[i].FuncNameA, SecPtr, AdvPtr, module);
        if (! ResPtr)
            return FALSE;
    }

    //
    // on Windows 8, hook sechost!SubscribeServiceChangeNotifications
    //

    if (Dll_OsBuild >= 8400) {

        static FuncNamesAW funcRedir[] = {
            FUNCNAME2(CredWriteA),
            FUNCNAME2(CredWriteW),
            FUNCNAME2(CredReadA),
            FUNCNAME2(CredReadW),
            FUNCNAME2(CredWriteDomainCredentialsA),
            FUNCNAME2(CredWriteDomainCredentialsW),
            FUNCNAME2(CredReadDomainCredentialsA),
            FUNCNAME2(CredReadDomainCredentialsW),
            FUNCNAME2(CredRenameA),
            FUNCNAME2(CredRenameW),
            FUNCNAME2(CredDeleteA),
            FUNCNAME2(CredDeleteW),
            FUNCNAME2(CredEnumerateA),
            FUNCNAME2(CredEnumerateW),
            { NULL, NULL }
        };

        for (i = 0; funcRedir[i].FuncNameA; ++i)
        {
            void *SecPtr = Ldr_GetProcAddrNew(DllName_sechost, funcRedir[i].FuncNameW, (char *)funcRedir[i].FuncNameA);
            if (SecPtr)
            {
                *funcRedir[i].pSyscall = *(ULONG_PTR*)&SecPtr;
            }
        }

        SCM_IMPORT_W8___(SubscribeServiceChangeNotifications);
        SBIEDLL_HOOK_SCM(SubscribeServiceChangeNotifications);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Scm_SecHostDll_W8
//---------------------------------------------------------------------------


_FX void Scm_SecHostDll_W8(void)
{
    if (Scm_SecHostDll_W8_ShouldRun) {
        Scm_SecHostDll_W8_ShouldRun = FALSE;
        Scm_SecHostDll((HMODULE)(ULONG_PTR)tzuk);
    }
}


//---------------------------------------------------------------------------
// Scm_SubscribeServiceChangeNotifications
//---------------------------------------------------------------------------


_FX ULONG_PTR Scm_SubscribeServiceChangeNotifications(
    ULONG_PTR Unknown1, ULONG_PTR Unknown2, ULONG_PTR Unknown3,
    ULONG_PTR Unknown4, ULONG_PTR Unknown5)
{
    //
    // fake success for new unknown function in Windows 8,
    // SubscribeServiceChangeNotifications
    //

    return 0;
}


//---------------------------------------------------------------------------
// Scm_DllHack
//---------------------------------------------------------------------------


_FX BOOLEAN Scm_DllHack(HMODULE module, const WCHAR *svcname)
{
    ULONG state;
    SC_HANDLE hService;
    SERVICE_QUERY_RPL *rpl;

    //
    // hack:  make sure the given service is running
    //

    if (! module)
        return TRUE;

    if (Scm_IsBoxedService(svcname))
        return TRUE;

    rpl = (SERVICE_QUERY_RPL *)Scm_QueryServiceByName(svcname, TRUE, 0);
    if (! rpl)
        return TRUE;

    state = rpl->service_status.dwCurrentState;
    Dll_Free(rpl);
    if (state != SERVICE_STOPPED)
        return TRUE;

    hService = Scm_OpenServiceW(
                    HANDLE_SERVICE_MANAGER, svcname, SERVICE_START);
    if (hService) {

        if (Scm_StartServiceW(hService, 0, NULL))
            Sleep(500);

        Scm_CloseServiceHandle(hService);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Scm_OsppcDll
//---------------------------------------------------------------------------


_FX BOOLEAN Scm_OsppcDll(HMODULE module)
{
    Custom_OsppcDll(module);

    //
    // hack for Office 2010 osppc.dll:  make sure osppsvc service is running
    //

    return Scm_DllHack(module, L"osppsvc");
}


//---------------------------------------------------------------------------
// Scm_DWriteDll
//---------------------------------------------------------------------------


_FX BOOLEAN Scm_DWriteDll(HMODULE module)
{
    //
    // hack for IE 9 DWrite.dll:  make sure FontCache service is running
    //

    return Scm_DllHack(module, L"FontCache");
}