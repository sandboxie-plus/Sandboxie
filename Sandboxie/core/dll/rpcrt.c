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
// RPCRT4
//---------------------------------------------------------------------------


#include "dll.h"
#include "core/drv/api_defs.h"
#include "core/svc/ServiceWire.h"
#include "core/svc/EpMapperWire.h"
#include <objbase.h>
#include <psapi.h>
#define INITGUID
#include <guiddef.h>
#include "trace.h"
#include "common/str_util.h"

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static ULONG RpcRt_RpcBindingInqAuthClientEx(
    void *ClientBinding, void *Privs, void **ServerPrincName,
    ULONG *AuthnLevel, ULONG *AuthnSvc, ULONG *AuthzSvc, ULONG Flags);

static ULONG RpcRt_RpcBindingFromStringBindingW(
    __in const WCHAR* StringBinding,
    __out RPC_BINDING_HANDLE* OutBinding);

static ULONG RpcRt_RpcBindingFromStringBindingA(
    __in const CHAR* StringBinding,
    __out RPC_BINDING_HANDLE* OutBinding);

static RPC_STATUS RpcRt_RpcBindingCreateW(
    RPC_BINDING_HANDLE_TEMPLATE_V1_W * Template,
    RPC_BINDING_HANDLE_SECURITY_V1_W * Security,
    RPC_BINDING_HANDLE_OPTIONS_V1 * Options,
    RPC_BINDING_HANDLE * Binding);

static RPC_STATUS RpcRt_RpcBindingCreateA(
    RPC_BINDING_HANDLE_TEMPLATE_V1_A* Template,
    RPC_BINDING_HANDLE_SECURITY_V1_A* Security,
    RPC_BINDING_HANDLE_OPTIONS_V1* Options,
    RPC_BINDING_HANDLE* Binding);

static RPC_STATUS RpcRt_RpcStringBindingComposeW(
    TCHAR* ObjUuid,
    TCHAR* ProtSeq,
    TCHAR* NetworkAddr,
    TCHAR* EndPoint,
    TCHAR* Options,
    TCHAR** StringBinding);

static RPC_STATUS RpcRt_RpcBindingFree(
    _Inout_ RPC_BINDING_HANDLE* Binding);

static RPC_STATUS RpcRt_RpcBindingCopy(
    _In_ RPC_BINDING_HANDLE SourceBinding,
    _Out_ RPC_BINDING_HANDLE* DestinationBinding);


void *Scm_QueryServiceByName(
    const WCHAR *ServiceNm,
    ULONG with_service_status, ULONG with_service_config);

int Scm_Start_Sppsvc();


#ifdef _WIN64

extern ULONG_PTR __cdecl RpcRt_NdrClientCall2(
    PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING  pFormat, ...);

extern ULONG_PTR __cdecl RpcRt_NdrClientCall3(
    MIDL_STUBLESS_PROXY_INFO* pProxyInfo, ULONG nProcNum, void* pReturnValue, ...);

#else

extern ULONG_PTR __cdecl RpcRt_NdrClientCall(
    PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING  pFormat, ULONG_PTR* Args);

extern ULONG_PTR __cdecl RpcRt_NdrClientCall2(
    PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING  pFormat, ULONG_PTR* Args);

#endif _WIN64


#ifdef _WIN64

extern ULONG_PTR __cdecl RpcRt_NdrAsyncClientCall(
    PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING pFormat, ...);

extern ULONG_PTR __cdecl RpcRt_Ndr64AsyncClientCall(
    void* pProxyInfo, ULONG nProcNum, void* pReturnValue, ...);

#else

extern ULONG_PTR __cdecl RpcRt_NdrAsyncClientCall(
    void* pStubDescriptor, void* pFormat, ULONG_PTR* Args);

#endif _WIN64


static ULONG_PTR RpcRt_RpcAsyncCompleteCall(
    RPC_ASYNC_STATE* AsyncState, void* Reply);


BOOLEAN Secure_RpcAsyncCompleteCall(RPC_ASYNC_STATE* AsyncState, void* Reply);

BOOLEAN __cdecl Secure_CheckElevation(struct SECURE_UAC_ARGS* Args);


//---------------------------------------------------------------------------


typedef ULONG (*P_RpcBindingInqAuthClientEx)(
    void *ClientBinding, void *Privs, void **ServerPrincName,
    ULONG *AuthnLevel, ULONG *AuthnSvc, ULONG *AuthzSvc, ULONG Flags);

typedef ULONG (*P_RpcBindingFromStringBindingW)(
    const WCHAR *StringBinding, void **OutBinding);

typedef ULONG(*P_RpcBindingFromStringBindingA)(
    const CHAR* StringBinding, void** OutBinding);

typedef RPC_STATUS (*P_RpcBindingCreateW)(
    RPC_BINDING_HANDLE_TEMPLATE_V1_W * Template,
    RPC_BINDING_HANDLE_SECURITY_V1_W * Security,
    RPC_BINDING_HANDLE_OPTIONS_V1 * Options,
    RPC_BINDING_HANDLE * Binding);

typedef RPC_STATUS(*P_RpcBindingCreateA)(
    RPC_BINDING_HANDLE_TEMPLATE_V1_A* Template,
    RPC_BINDING_HANDLE_SECURITY_V1_A* Security,
    RPC_BINDING_HANDLE_OPTIONS_V1* Options,
    RPC_BINDING_HANDLE* Binding);

typedef RPC_STATUS (RPC_ENTRY *P_RpcStringBindingComposeW)( 
    TCHAR *ObjUuid, 
    TCHAR *ProtSeq, 
    TCHAR *NetworkAddr, 
    TCHAR *EndPoint, 
    TCHAR *Options, 
    TCHAR **StringBinding);

typedef RPC_STATUS(RPC_ENTRY* P_RpcBindingCopy)(
    _In_ RPC_BINDING_HANDLE SourceBinding,
    _Out_ RPC_BINDING_HANDLE* DestinationBinding);

typedef RPC_STATUS(RPC_ENTRY* P_RpcBindingFree)(
    _Inout_ RPC_BINDING_HANDLE* Binding);


typedef RPC_STATUS (RPC_ENTRY *P_UuidToStringW)(
    const UUID *Uuid, 
    RPC_WSTR *StringUuid);

typedef RPC_STATUS(RPC_ENTRY *P_RpcStringFreeW)(
    RPC_WSTR *String);

typedef RPC_STATUS (*P_RpcMgmtSetComTimeout)(
    RPC_BINDING_HANDLE Binding, 
    unsigned int Timeout);

typedef BOOL(WINAPI* P_GetModuleInformation)(
    _In_ HANDLE hProcess, 
    _In_ HMODULE hModule, 
    _Out_ LPMODULEINFO lpmodinfo, 
    _In_ DWORD cb);

typedef ULONG_PTR(*P_RpcAsyncCompleteCall)(
    RPC_ASYNC_STATE* AsyncState, void* Reply);


//---------------------------------------------------------------------------


P_RpcStringBindingComposeW  __sys_RpcStringBindingComposeW = NULL;

P_RpcBindingInqAuthClientEx __sys_RpcBindingInqAuthClientEx = NULL;

P_RpcBindingFromStringBindingW __sys_RpcBindingFromStringBindingW = NULL;

P_RpcBindingFromStringBindingA __sys_RpcBindingFromStringBindingA = NULL;

P_RpcBindingCreateW __sys_RpcBindingCreateW = NULL;
P_RpcBindingCreateA __sys_RpcBindingCreateA = NULL;

P_RpcBindingCopy    __sys_RpcBindingCopy = NULL;
P_RpcBindingFree    __sys_RpcBindingFree = NULL;

P_RpcMgmtSetComTimeout __sys_RpcMgmtSetComTimeout = NULL;
#define RPC_C_BINDING_TIMEOUT   4
BOOLEAN __use_RpcMgmtSetComTimeout = FALSE;

P_UuidToStringW __sys_UuidToStringW = NULL;

P_RpcStringFreeW __sys_RpcStringFreeW = NULL;

P_GetModuleInformation __sys_GetModuleInformation = NULL;

#ifdef _WIN64
VOID*             __sys_NdrClientCall2 = NULL;
VOID*             __sys_NdrClientCall3 = NULL;
#else
VOID*             __sys_NdrClientCall = NULL;
VOID*             __sys_NdrClientCall2 = NULL;
#endif

#ifdef _WIN64
VOID*             __sys_NdrAsyncClientCall = NULL;
VOID*             __sys_Ndr64AsyncClientCall = NULL;
#else
VOID*             __sys_NdrAsyncClientCall = NULL;
#endif _WIN64

static  P_RpcAsyncCompleteCall  __sys_RpcAsyncCompleteCall = NULL;

extern LIST Ipc_DynamicPortNames;

typedef struct _IPC_DYNAMIC_PORT {
    LIST_ELEM list_elem;

    WCHAR       wstrPortId[DYNAMIC_PORT_ID_CHARS];
    WCHAR       wstrPortName[DYNAMIC_PORT_NAME_CHARS];
} IPC_DYNAMIC_PORT;

BOOLEAN g_rpc_client_hooks = FALSE;

//---------------------------------------------------------------------------
// RpcRt_Init
//---------------------------------------------------------------------------


_FX BOOLEAN RpcRt_Init(HMODULE module)
{
    P_RpcBindingInqAuthClientEx RpcBindingInqAuthClientEx;

    RpcBindingInqAuthClientEx = (P_RpcBindingInqAuthClientEx)
        Ldr_GetProcAddrNew(DllName_rpcrt4, L"RpcBindingInqAuthClientExW","RpcBindingInqAuthClientExW");

    SBIEDLL_HOOK(RpcRt_,RpcBindingInqAuthClientEx);

    // these 2 are only used for debugging, so no need to hook
    __sys_UuidToStringW = Ldr_GetProcAddrNew(DllName_rpcrt4, L"UuidToStringW", "UuidToStringW");
    __sys_RpcStringFreeW = Ldr_GetProcAddrNew(DllName_rpcrt4, L"RpcStringFreeW", "RpcStringFreeW");

    //if (Dll_OsBuild < 6000)        // Vista
    //    return TRUE;

    if ( Dll_OsBuild >= 15063) {
        P_RpcStringBindingComposeW RpcStringBindingComposeW = (P_RpcStringBindingComposeW)
            Ldr_GetProcAddrNew(DllName_rpcrt4, L"RpcStringBindingComposeW","RpcStringBindingComposeW");
        SBIEDLL_HOOK(RpcRt_,RpcStringBindingComposeW);
    }

    P_RpcBindingFromStringBindingW RpcBindingFromStringBindingW = (P_RpcBindingFromStringBindingW)
        Ldr_GetProcAddrNew(DllName_rpcrt4, L"RpcBindingFromStringBindingW", "RpcBindingFromStringBindingW");
    SBIEDLL_HOOK(RpcRt_, RpcBindingFromStringBindingW);

    P_RpcBindingFromStringBindingA RpcBindingFromStringBindingA = (P_RpcBindingFromStringBindingA)
        Ldr_GetProcAddrNew(DllName_rpcrt4, L"RpcBindingFromStringBindingA", "RpcBindingFromStringBindingA");
    SBIEDLL_HOOK(RpcRt_, RpcBindingFromStringBindingA);

    P_RpcBindingCreateW RpcBindingCreateW = (P_RpcBindingCreateW)
        Ldr_GetProcAddrNew(DllName_rpcrt4, L"RpcBindingCreateW", "RpcBindingCreateW");
    SBIEDLL_HOOK(RpcRt_, RpcBindingCreateW);

    P_RpcBindingCreateA RpcBindingCreateA = (P_RpcBindingCreateA)
        Ldr_GetProcAddrNew(DllName_rpcrt4, L"RpcBindingCreateA", "RpcBindingCreateA");
    SBIEDLL_HOOK(RpcRt_, RpcBindingCreateA);

    P_RpcBindingCopy RpcBindingCopy = (P_RpcBindingCopy)
        Ldr_GetProcAddrNew(DllName_rpcrt4, L"RpcBindingCopy", "RpcBindingCopy");
    SBIEDLL_HOOK(RpcRt_, RpcBindingCopy);

    P_RpcBindingFree RpcBindingFree = (P_RpcBindingFree)
        Ldr_GetProcAddrNew(DllName_rpcrt4, L"RpcBindingFree", "RpcBindingFree");
    SBIEDLL_HOOK(RpcRt_, RpcBindingFree);


    __sys_RpcMgmtSetComTimeout = (P_RpcMgmtSetComTimeout)
        Ldr_GetProcAddrNew(DllName_rpcrt4, L"RpcMgmtSetComTimeout", "RpcMgmtSetComTimeout");
    __use_RpcMgmtSetComTimeout = Config_GetSettingsForImageName_bool(L"RpcMgmtSetComTimeout", TRUE);
    

    WCHAR wsTraceOptions[4];
    if (SbieApi_QueryConf(NULL, L"IpcTrace", 0, wsTraceOptions, sizeof(wsTraceOptions)) == STATUS_SUCCESS && wsTraceOptions[0] != L'\0')
    {
        g_rpc_client_hooks = TRUE;

#ifdef _WIN64

        void* NdrClientCall2 = Ldr_GetProcAddrNew(DllName_rpcrt4, L"NdrClientCall2", "NdrClientCall2");
        SBIEDLL_HOOK(RpcRt_, NdrClientCall2);

        void* NdrClientCall3 = Ldr_GetProcAddrNew(DllName_rpcrt4, L"NdrClientCall3", "NdrClientCall3");
        SBIEDLL_HOOK(RpcRt_, NdrClientCall3);

        //NdrClientCall2(PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING pFormat, ...)
        //NdrClientCall3(MIDL_STUBLESS_PROXY_INFO *pProxyInfo, unsigned int nProcNum, void *pReturnValue, ...)


        //Ndr64AsyncServerCall64(PRPC_MESSAGE pRpcMsg)
        //Ndr64AsyncServerCallAll(PRPC_MESSAGE pRpcMsg)
        //NdrAsyncServerCall(PRPC_MESSAGE pRpcMsg)
        //NdrServerCall2(PRPC_MESSAGE pRpcMsg)
        //NdrServerCallAll(PRPC_MESSAGE pRpcMsg)
        //NdrServerCallNdr64(PRPC_MESSAGE pRpcMsg)

#else
        
        void*  NdrClientCall = Ldr_GetProcAddrNew(DllName_rpcrt4, L"NdrClientCall", "NdrClientCall");
        SBIEDLL_HOOK(RpcRt_, NdrClientCall);

        void* NdrClientCall2 = Ldr_GetProcAddrNew(DllName_rpcrt4, L"NdrClientCall2", "NdrClientCall2");
        SBIEDLL_HOOK(RpcRt_, NdrClientCall2);

        //NdrClientCall(PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING pFormat, ...)
        //NdrClientCall2(PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING pFormat, ...)
        //NdrClientCall4(PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING pFormat, ...) -> NdrClientCall2


        //NdrAsyncServerCall(PRPC_MESSAGE pRpcMsg)
        //NdrServerCall2(PRPC_MESSAGE pRpcMsg)
        //NdrServerCall(PRPC_MESSAGE pRpcMsg)

#endif
    }

#ifdef _WIN64

    if (g_rpc_client_hooks) {
        void* NdrAsyncClientCall = Ldr_GetProcAddrNew(DllName_rpcrt4, L"NdrAsyncClientCall", "NdrAsyncClientCall");
        SBIEDLL_HOOK(RpcRt_, NdrAsyncClientCall);
    }

    // required for UAC
    void* Ndr64AsyncClientCall = Ldr_GetProcAddrNew(DllName_rpcrt4, L"Ndr64AsyncClientCall", "Ndr64AsyncClientCall");
    SBIEDLL_HOOK(RpcRt_, Ndr64AsyncClientCall);

    //NdrAsyncClientCall(PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING pFormat, ...)
    //Ndr64AsyncClientCall(MIDL_STUBLESS_PROXY_INFO* pProxyInfo, unsigned int nProcNum, void* pReturnValue, ...) <- uac hook


    //NdrDcomAsyncClientCall(PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING pFormat, ...)
    //Ndr64DcomAsyncClientCall(MIDL_STUBLESS_PROXY_INFO* pProxyInfo, unsigned int nProcNum, void* pReturnValue, ...)

#else ! _WIN64

    // required for UAC
    void* NdrAsyncClientCall = Ldr_GetProcAddrNew(DllName_rpcrt4, L"NdrAsyncClientCall", "NdrAsyncClientCall");
    SBIEDLL_HOOK(RpcRt_, NdrAsyncClientCall);

    //NdrAsyncClientCall(PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING pFormat, ...) <- uac hook
    //NdrAsyncClientCall2(PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING pFormat, ...) -> NdrAsyncClientCall


    //NdrDcomAsyncClientCall(PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING pFormat, ...)
    //NdrDcomAsyncClientCall2(PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING pFormat, ...)->NdrDcomAsyncClientCall

#endif _WIN64

    // required for UAC
    P_RpcAsyncCompleteCall RpcAsyncCompleteCall = (P_RpcAsyncCompleteCall)
        Ldr_GetProcAddrNew(DllName_rpcrt4, L"RpcAsyncCompleteCall", "RpcAsyncCompleteCall");
    SBIEDLL_HOOK(RpcRt_, RpcAsyncCompleteCall);

    return TRUE;
}


//---------------------------------------------------------------------------
// RpcRt_RpcBindingInqAuthClientEx
//---------------------------------------------------------------------------


_FX ULONG RpcRt_RpcBindingInqAuthClientEx(
    void *ClientBinding, void *Privs, void **ServerPrincName,
    ULONG *AuthnLevel, ULONG *AuthnSvc, ULONG *AuthzSvc, ULONG Flags)
{
    ULONG rc = __sys_RpcBindingInqAuthClientEx(
        ClientBinding, Privs, ServerPrincName,
        AuthnLevel, AuthnSvc, AuthzSvc, Flags);

    //
    // the toolbar component in Norton Internet Security calls
    // RpcBindingInqAuthClientW(binding,privs,NULL,&level,NULL,NULL)
    // and gets error RPC_S_INVALID_AUTH_IDENTITY inside the sandbox.
    // we fake successful return with level == RPC_C_AUTHN_LEVEL_PKT_PRIVACY
    //

    if (rc == RPC_S_INVALID_AUTH_IDENTITY && AuthnLevel &&
            ! (ServerPrincName || AuthnSvc || AuthzSvc || Flags)) {

        rc = RPC_S_OK;
        *AuthnLevel = RPC_C_AUTHN_LEVEL_PKT_PRIVACY;
    }

    return rc;
}


//---------------------------------------------------------------------------
// RpcRt_TestCallingModule
//---------------------------------------------------------------------------


BOOLEAN RpcRt_TestCallingModule(ULONG_PTR pRetAddr, ULONG_PTR hModule)
{
    if (hModule && (pRetAddr > hModule))
    {
        MODULEINFO modinfo;

        if (__sys_GetModuleInformation(GetCurrentProcess(), (HANDLE)hModule, &modinfo, sizeof(MODULEINFO)))
        {
            // return address within the module?
            if (pRetAddr < hModule + modinfo.SizeOfImage)
                return TRUE;
        }
    }
    return FALSE;
}


//---------------------------------------------------------------------------
// RpcRt_FindModulePreset
//---------------------------------------------------------------------------


_FX NTSTATUS RpcRt_FindModulePreset(
    const WCHAR* CallingModule, const WCHAR* Identifier, WCHAR* value, ULONG value_size)
{
    WCHAR conf_buf[2048];
    ULONG found_mode = -1;

    ULONG index = 0;
    while (1) {

        NTSTATUS status = SbieApi_QueryConf(
            NULL, L"RpcPortBinding", index, conf_buf, sizeof(conf_buf) - 16 * sizeof(WCHAR));
        if (!NT_SUCCESS(status))
            break;
        ++index;

        //
        // examples:
        //
        // RpcPortBinding=winspool.drv,'ncalrpc:[,Security=Impersonation Dynamic False]',Resolve=Spooler
        // RpcPortBinding=WinHttp.dll,ncalrpc:,Resolve=WinHttpAutoProxySvc,TimeOut=y
        //
        // RpcPortBinding=WinSCard.dll,{00000000-0000-0000-0000-000000000000},Resolve=SCard
        // RpcPortBindingIfId=SCard,{C6B5235A-E413-481D-9AC8-31681B1FAAF5}
        //

        ULONG mode = -1;
        const WCHAR* found_value = Config_MatchImageAndGetValue(conf_buf, CallingModule, &mode);
        if (!found_value || mode > found_mode)
            continue;

        WCHAR* test_value = NULL;
        ULONG test_len = 0;
        found_value = SbieDll_GetTagValue(found_value, NULL, &test_value, &test_len, L',');

        if (!test_value || !found_value || !*found_value) {
            SbieApi_Log(2207, L"RpcPortBinding");
            continue;
        }
        //test_value[test_len] = L'\0';

        if (*test_value == L'{') // is it a uuid?
        {
            if(_wcsnicmp(test_value + 1, Identifier, 36) != 0) // skip { and }
                continue;
        }
        else if(*test_value && !(test_value[0] == L'*' && test_value[1] == L'\0')) // test only for non wildcards and non empty strings
        {
            if (!Config_MatchImage(test_value, test_len, Identifier, 1))
                continue;
        }

        wcscpy_s(value, value_size / sizeof(WCHAR), found_value);
        found_mode = mode;
    }

    if (found_mode == -1)
        return STATUS_NO_MORE_ENTRIES;
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// StoreLpcPortName
//---------------------------------------------------------------------------


WCHAR* StoreLpcPortName(const WCHAR* wszPortId, const WCHAR* wszPortName)
{
    IPC_DYNAMIC_PORT* port = List_Head(&Ipc_DynamicPortNames);
    while (port)
    {
        if (_wcsicmp(wszPortId, port->wstrPortId) == 0)
        {
            wmemcpy(port->wstrPortName, wszPortName, DYNAMIC_PORT_NAME_CHARS);
            break;
        }

        port = List_Next(port);
    }

    if (port == NULL)
    {
        port = (IPC_DYNAMIC_PORT*)Dll_Alloc(sizeof(IPC_DYNAMIC_PORT));
        if (port)
        {
            wmemcpy(port->wstrPortId, wszPortId, DYNAMIC_PORT_ID_CHARS);
            wmemcpy(port->wstrPortName, wszPortName, DYNAMIC_PORT_NAME_CHARS);

            List_Insert_After(&Ipc_DynamicPortNames, NULL, port);
        }
    }
    return port ? port->wstrPortName : NULL;
}


//---------------------------------------------------------------------------
// GetDynamicLpcPortName
//---------------------------------------------------------------------------


WCHAR* GetDynamicLpcPortName(const WCHAR* wszPortId)
{
    EPMAPPER_GET_PORT_NAME_REQ req;
    EPMAPPER_GET_PORT_NAME_RPL* rpl;

    memset(&req, 0, sizeof(req));
    req.h.length = sizeof(EPMAPPER_GET_PORT_NAME_REQ);
    req.h.msgid = MSGID_EPMAPPER_GET_PORT_NAME;
    wcscpy(req.wszPortId, wszPortId);

    rpl = (EPMAPPER_GET_PORT_NAME_RPL*)SbieDll_CallServer(&req.h);

    WCHAR   wsTraceOptions[4];
    if (SbieApi_QueryConf(NULL, L"IpcTrace", 0, wsTraceOptions, sizeof(wsTraceOptions)) == STATUS_SUCCESS && wsTraceOptions[0] != L'\0')
    {
        WCHAR text[130];

        if (rpl && NT_SUCCESS(rpl->h.status))
            Sbie_snwprintf(text, 130, L"Resolved dynamic port: %s; endpoint: %s", req.wszPortId, rpl->wszPortName);
        else
            Sbie_snwprintf(text, 130, L"Failed to resolve dynamic port: %s; status: %08X", req.wszPortId, rpl ? rpl->h.status : 0);

        SbieApi_MonitorPutMsg(MONITOR_RPC | MONITOR_TRACE, text);
    }

    if (rpl && NT_SUCCESS(rpl->h.status))
    {
        WCHAR* port = StoreLpcPortName(req.wszPortId, rpl->wszPortName);
        Dll_Free(rpl);
        if(port)
            return port + 13; // skip "\\RPC Control\\"
    }

    return NULL;
}


//---------------------------------------------------------------------------
// RpcRt_IsDynamicPortOpen
//---------------------------------------------------------------------------


_FX BOOLEAN RpcRt_IsDynamicPortOpen(const WCHAR* wszPortName)
{
    IPC_DYNAMIC_PORT* port = List_Head(&Ipc_DynamicPortNames);
    while (port) 
    {    
        if (_wcsicmp(wszPortName, port->wstrPortName) == 0)
        {
            // see also RpcBindingFromStringBindingW in core/dll/rpcrt.c
            // and core/drv/ipc_spl.c
            return TRUE;
        }

        port = List_Next(port);
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// RpcRt_RpcBindingFromStringBindingA
//---------------------------------------------------------------------------


_FX ULONG RpcRt_RpcBindingFromStringBindingA(
    __in const CHAR* StringBinding,
    __out RPC_BINDING_HANDLE* OutBinding)
{
    NTSTATUS status = 0;
    THREAD_DATA* TlsData = NULL;

    TlsData = Dll_GetTlsData(NULL);

    TlsData->rpc_caller = (ULONG_PTR)_ReturnAddress();

    status = __sys_RpcBindingFromStringBindingA(StringBinding, OutBinding);

    TlsData->rpc_caller = 0;

    return status;
}


//---------------------------------------------------------------------------
// RpcRt_RpcBindingFromStringBindingW
//---------------------------------------------------------------------------


_FX ULONG RpcRt_RpcBindingFromStringBindingW(
    __in const WCHAR *StringBinding,
    __out RPC_BINDING_HANDLE* OutBinding)
{
    //
    // printing functions in winspool.drv talk to the Spooler service
    // (process spoolsv.exe).  prior to Windows 8.1, the service was
    // reachable via the fixed port name \RPC Control\spoolss.
    // in Windows 8.1, there is no longer a fixed endpoint name, instead
    // spoolsv.exe registers a dynamic endpoint with the system-wide
    // endpoint mapper (\RPC Control\epmapper, the rpcss service).
    // but our sandboxed epmapper has no knowledge of this endpoint.
    // this makes it impossible to connect to the spooler service.
    //
    // to work around this, we have use the driver helper function
    // API_GET_SPOOLER_PORT to find the port name of the spooler endpoint,
    // and then we inject this port name into the binding string that
    // we pass to the real RpcBindingFromStringBindingW.  this tells the
    // RPCRT4 logic to bypass endpoint resolution through (the sandboxed)
    // epmapper, and instead go directly to the specified port name.
    //
    // Ipc_NtAlpcConnectPort (core/dll/ipc.c) and Ipc_CheckGenericObject
    // (core/drv/ipc.c) are also adjusted to look for this port name and
    // treat it as a built in OpenIpcPath.
    //
    // to summarize, by finding the real port name used by spoolsv.exe
    // outside the sandbox, we bypass endpoint resolution and simulate
    // the pre-Windows 8.1 behavior of a fixed endpoint port name.
    //

    //StringBinding = 0x4 means a NULL value is set in the RpcBinding string
    //Microsoft adds 0x4 to the RpcBinding string member resulting in 
    //StringBinging = 0x4 if the RPCBindingString is NULL
    //and will result in a crash if it is passed to the system function.

    if(0x4 == (ULONG_PTR)StringBinding) {   
        return RPC_S_INVALID_ARG;
    }

    BOOLEAN use_RpcMgmtSetComTimeout = __use_RpcMgmtSetComTimeout;

    THREAD_DATA* TlsData = Dll_GetTlsData(NULL);

    ULONG_PTR pRetAddr = TlsData->rpc_caller ? TlsData->rpc_caller : (ULONG_PTR)_ReturnAddress();

    WCHAR wstrPortName[MAX_PATH];
    memset(wstrPortName, 0, sizeof(wstrPortName));

    /*if (wcsstr(StringBinding, L"epmapper") != NULL) {
        __debugbreak();
    }*/

    static const WCHAR* dynamicFalse = L"ncalrpc:[,Security=Impersonation Dynamic False]";
    //static const WCHAR* dynamicTrue = L"ncalrpc:[,Security=Impersonation Dynamic True]";

    if (_wcsicmp(StringBinding, dynamicFalse) == 0) {

        ULONG_PTR pWinSpool = (ULONG_PTR)GetModuleHandle(L"winspool.drv");

        if (RpcRt_TestCallingModule(pRetAddr, pWinSpool)) {

            WCHAR* pwszTempPortName = GetDynamicLpcPortName(SPOOLER_PORT_ID);

            if (pwszTempPortName == NULL)
                return RPC_S_ACCESS_DENIED;

            wcscpy(wstrPortName, L"ncalrpc:[");
            wcscpy(wstrPortName + 9, pwszTempPortName);
            wcscat(wstrPortName, dynamicFalse + 9);
        }
    }
    /*else if (_wcsicmp(StringBinding, L"0497b57d-2e66-424f-a0c6-157cd5d41700@ncalrpc:") == 0) { // Appinfo

        ULONG_PTR pkernel32 = (ULONG_PTR)GetModuleHandle(L"kernel32.dll");
        // kernel32.dll!AicpCreateBindingHandle
        // kernel32.dll!AicGetPackageActivationTokenForSxS
        // KernelBase.dll!CreateProcessInternalW
        // SbieDll.dll!Proc_CreateProcessInternalW_RS5

        if (RpcRt_TestCallingModule(pRetAddr, pkernel32)) {
            use_RpcMgmtSetComTimeout = TRUE;
        }
    }*/

    WCHAR* CallingModule = Trace_FindModuleByAddress((void*)pRetAddr);
    if (CallingModule)
    {
        // get the DLL specific preset if present
        use_RpcMgmtSetComTimeout = SbieDll_GetSettingsForName_bool(NULL, CallingModule, L"UseRpcMgmtSetComTimeout", use_RpcMgmtSetComTimeout);

        //
        // check for a "RpcPortBinding" entry 
        //

        WCHAR ModulePreset[256];
        if (NT_SUCCESS(RpcRt_FindModulePreset(CallingModule, StringBinding, ModulePreset, sizeof(ModulePreset)))) {
            
            WCHAR* pwszTempPortName = NULL;
            WCHAR tagValue[96];
            if (SbieDll_FindTagValue(ModulePreset, L"Resolve", tagValue, sizeof(tagValue), L'=', L',')) {
                pwszTempPortName = GetDynamicLpcPortName(tagValue);
            }
            
            if (!pwszTempPortName && SbieDll_FindTagValue(ModulePreset, L"IpcPort", tagValue, sizeof(tagValue), L'=', L',')) {
                pwszTempPortName = StoreLpcPortName(tagValue, tagValue);
            }

            if (pwszTempPortName != NULL)
            {
                WCHAR* ptr = wcsstr(StringBinding, L":");
                if (ptr)
                {
                    size_t len = ptr - StringBinding;
                    wcsncpy(wstrPortName, StringBinding, len);
                    wcscat(wstrPortName, L":[");
                    wcscat(wstrPortName, pwszTempPortName);
                    if (ptr[1] == L'[')
                        wcscat(wstrPortName, ptr + 2);
                    else
                        wcscat(wstrPortName, L"]");
                }
            }

            // the "RpcPortBinding" overwrites "UseRpcMgmtSetComTimeout"
            if (SbieDll_FindTagValue(ModulePreset, L"TimeOut", tagValue, sizeof(tagValue), L'=', L','))
                use_RpcMgmtSetComTimeout = Config_String2Bool(tagValue, use_RpcMgmtSetComTimeout);
        }
    }

    RPC_STATUS  status;
    status = __sys_RpcBindingFromStringBindingW(*wstrPortName ? wstrPortName : StringBinding, OutBinding);

    // If there are any IpcTrace options set, then output this debug string
    WCHAR   wsTraceOptions[4];
    if (SbieApi_QueryConf(NULL, L"IpcTrace", 0, wsTraceOptions, sizeof(wsTraceOptions)) == STATUS_SUCCESS && wsTraceOptions[0] != L'\0')
    {
        WCHAR msg[512];

        //Sbie_snwprintf(msg, 512, L"SBIE p=%06d t=%06d RpcBindingFromStringBindingW StringBinding = '%s', BindingHandle = 0x%X, status = 0x%X\n", GetCurrentProcessId(), GetCurrentThreadId(),
        Sbie_snwprintf(msg, 512, L"StringBinding = '%s', wstrPortName = '%s', BindingHandle = 0x%X, status = 0x%08X, timeout = %d, caller = '%s'",
            StringBinding, wstrPortName, *OutBinding, status, use_RpcMgmtSetComTimeout,
            CallingModule ? CallingModule : L"unknown");

        //OutputDebugString(msg);
        SbieApi_MonitorPutMsg(MONITOR_RPC | MONITOR_TRACE, msg);
    }

    if(use_RpcMgmtSetComTimeout) __sys_RpcMgmtSetComTimeout(*OutBinding, RPC_C_BINDING_TIMEOUT);
    return status;
}


//---------------------------------------------------------------------------
// RpcRt_RpcBindingCreateA
//---------------------------------------------------------------------------

_FX RPC_STATUS RpcRt_RpcBindingCreateA(
    __in RPC_BINDING_HANDLE_TEMPLATE_V1_A* Template,
    __in_opt RPC_BINDING_HANDLE_SECURITY_V1_A* Security,
    __in_opt RPC_BINDING_HANDLE_OPTIONS_V1* Options,
    __out RPC_BINDING_HANDLE* Binding)
{
    NTSTATUS status = 0;
    THREAD_DATA* TlsData = NULL;

    TlsData = Dll_GetTlsData(NULL);

    TlsData->rpc_caller = (ULONG_PTR)_ReturnAddress();

    status = __sys_RpcBindingCreateA(Template, Security, Options, Binding);

    TlsData->rpc_caller = 0;

    return status;
}

//---------------------------------------------------------------------------
// RpcRt_RpcBindingCreateW
//---------------------------------------------------------------------------


DEFINE_GUID(
    EMPTY_UUID,
    0x00000000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

// {906B0CE0-C70B-1067-B317-00DD010662DA}
DEFINE_GUID(
    MSDTC_UUID,
    0x906B0CE0, 0xC70B, 0x1067, 0xB3, 0x17, 0x00, 0xDD, 0x01, 0x06, 0x62, 0xDA);
    

_FX RPC_STATUS RpcRt_RpcBindingCreateW(
    __in RPC_BINDING_HANDLE_TEMPLATE_V1_W* Template,
    __in_opt RPC_BINDING_HANDLE_SECURITY_V1_W* Security,
    __in_opt RPC_BINDING_HANDLE_OPTIONS_V1* Options,
    __out RPC_BINDING_HANDLE* Binding)
{
    BOOLEAN use_RpcMgmtSetComTimeout = __use_RpcMgmtSetComTimeout;

    THREAD_DATA* TlsData = Dll_GetTlsData(NULL);

    ULONG_PTR pRetAddr = TlsData->rpc_caller ? TlsData->rpc_caller : (ULONG_PTR)_ReturnAddress();

    if ( (memcmp(&Template->ObjectUuid, &MSDTC_UUID, sizeof(GUID)) == 0) && RPC_PROTSEQ_LRPC == Template->ProtocolSequence && !Template->StringEndpoint)
    {
        Template->StringEndpoint = (unsigned short*)L"samss lpc";
    }

    RPC_WSTR   StringUuid;
    __sys_UuidToStringW(&Template->ObjectUuid, &StringUuid);

    WCHAR* CallingModule = Trace_FindModuleByAddress((void*)pRetAddr);
    if (CallingModule)
    {
        // get the DLL specific preset if present
        use_RpcMgmtSetComTimeout = SbieDll_GetSettingsForName_bool(NULL, CallingModule, L"UseRpcMgmtSetComTimeout", use_RpcMgmtSetComTimeout);

        //
        // check for a "RpcPortBinding" entry 
        //

        WCHAR ModulePreset[256];
        if (NT_SUCCESS(RpcRt_FindModulePreset(CallingModule, StringUuid, ModulePreset, sizeof(ModulePreset)))) {
            
            WCHAR tagValue[96];
            if (RPC_PROTSEQ_LRPC == Template->ProtocolSequence && !Template->StringEndpoint)
            {
                if (SbieDll_FindTagValue(ModulePreset, L"Resolve", tagValue, sizeof(tagValue), L'=', L',')) {
                    Template->StringEndpoint = GetDynamicLpcPortName(tagValue);
                }

                if (!Template->StringEndpoint && SbieDll_FindTagValue(ModulePreset, L"IpcPort", tagValue, sizeof(tagValue), L'=', L',')){
                    Template->StringEndpoint = StoreLpcPortName(tagValue, tagValue);
                }
            }

            // the "RpcPortBinding" overwrites "UseRpcMgmtSetComTimeout"
            if (SbieDll_FindTagValue(ModulePreset, L"TimeOut", tagValue, sizeof(tagValue), L'=', L','))
                use_RpcMgmtSetComTimeout = Config_String2Bool(tagValue, use_RpcMgmtSetComTimeout);
        }
    }

    RPC_STATUS  status;
    status = __sys_RpcBindingCreateW(Template, Security, Options, Binding);

    // If there are any IpcTrace options set, then output this debug string
    WCHAR   wsTraceOptions[4];
    if (SbieApi_QueryConf(NULL, L"IpcTrace", 0, wsTraceOptions, sizeof(wsTraceOptions)) == STATUS_SUCCESS && wsTraceOptions[0] != L'\0')
    {
        WCHAR msg[512];

        //Sbie_snwprintf(msg, 512, L"SBIE p=%06d t=%06d RpcBindingCreateW Endpoint = '%s', UUID = %s, status = 0x%X\n", GetCurrentProcessId(), GetCurrentThreadId(),
        Sbie_snwprintf(msg, 512, L"Endpoint = '%s', UUID = %s, BindingHandle = 0x%X, status = 0x%08X, timeout = %d, caller = '%s'", 
            Template && Template->StringEndpoint ? Template->StringEndpoint : L"null",
            StringUuid, *Binding, status, use_RpcMgmtSetComTimeout,
            CallingModule ? CallingModule : L"unknown");

        //OutputDebugString(msg);
        SbieApi_MonitorPutMsg(MONITOR_RPC | MONITOR_TRACE, msg);
    }

    __sys_RpcStringFreeW(&StringUuid);

    if (use_RpcMgmtSetComTimeout) __sys_RpcMgmtSetComTimeout(*Binding, RPC_C_BINDING_TIMEOUT);
    return status;
}


//---------------------------------------------------------------------------
// RpcRt_RpcStringBindingComposeW
//---------------------------------------------------------------------------


//#define UUID_UserMgrCli L"B18FBAB6-56F8-4702-84E0-41053293A869"

RPC_STATUS RPC_ENTRY RpcRt_RpcStringBindingComposeW(TCHAR *ObjUuid,TCHAR *ProtSeq,TCHAR *NetworkAddr,TCHAR *EndPoint,TCHAR *Options,TCHAR **StringBinding) 
{
    ULONG_PTR hSppc =  (ULONG_PTR)GetModuleHandle(L"sppc.dll");
    ULONG_PTR pRetAddr = (ULONG_PTR)_ReturnAddress();

    if (RpcRt_TestCallingModule(pRetAddr, hSppc))
    {
        EndPoint = L"SPPCTransportEndpoint-00001";
        Scm_Start_Sppsvc();
    }
    // we must block this in Win 10 to prevent r-click context menu hang in Explorer
    // note: this breaks otehr things but we need it, 
    // so instead we block the {470C0EBD-5D73-4D58-9CED-E91E22E23282} Pin To Start Screen verb handler;
    // inside Com_CoCreateInstance
    //else if (ObjUuid && (!_wcsicmp(ObjUuid, UUID_UserMgrCli)))
    //{
    //    return STATUS_ACCESS_DENIED;
    //}
    return __sys_RpcStringBindingComposeW(ObjUuid,ProtSeq,NetworkAddr,EndPoint,Options,StringBinding);
}


//---------------------------------------------------------------------------
// RpcRt_RpcBindingCopy
//---------------------------------------------------------------------------


RPC_STATUS RpcRt_RpcBindingCopy(
    _In_ RPC_BINDING_HANDLE SourceBinding,
    _Out_ RPC_BINDING_HANDLE* DestinationBinding)
{
    return __sys_RpcBindingCopy(SourceBinding, DestinationBinding);
}


//---------------------------------------------------------------------------
// RpcRt_RpcBindingFree
//---------------------------------------------------------------------------


static RPC_STATUS RpcRt_RpcBindingFree(
    _Inout_ RPC_BINDING_HANDLE* Binding)
{
    return __sys_RpcBindingFree(Binding);
}


//---------------------------------------------------------------------------
// RpcRt_NdrClientCallX
//---------------------------------------------------------------------------

void Sbie_StringFromGUID(const GUID* guid, WCHAR* str);

void RpcRt_NdrClientCallX(const WCHAR* Function, void* ReturnAddress, PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING pFormat, void** pStack)
{
    WCHAR text[512] = L"RpcRt_NdrClientCallX";

    const WCHAR* CallingModule = Trace_FindModuleByAddress(ReturnAddress);
    if (!CallingModule)
        CallingModule = L"unknown";

    __try
    {
        RPC_BINDING_HANDLE* pHandle = NULL;
        unsigned char HandleType = -1;
        if (pFormat) {
            HandleType = pFormat[0];
            if (!HandleType) { // explicit handle
                // pStack[0] explicit handle
                // pStack[1] Arg0
                // pStack[2] Arg1
                // ...
                // pStack[n+1] ArgN
                pHandle = (RPC_BINDING_HANDLE*)&pStack[0]; // first argument in the var arg list is the explicit handle
            }
            else { // implicit handle
                // pStack[0] Arg0
                // pStack[1] Arg1
                // ...
                // pStack[n] ArgN
                pHandle = (RPC_BINDING_HANDLE*)&pStubDescriptor->IMPLICIT_HANDLE_INFO.pPrimitiveHandle;
            }
        }

        if (pHandle && *pHandle == NULL) {
            pHandle = NULL;
        }

        PRPC_CLIENT_INTERFACE rpcInterface = (PRPC_CLIENT_INTERFACE)pStubDescriptor->RpcInterfaceInformation;
        if (rpcInterface)
        {
            WCHAR interfaceID[48];
            Sbie_StringFromGUID(&rpcInterface->InterfaceId.SyntaxGUID, interfaceID);

            Sbie_snwprintf(text, 512, L"Calling %s UUID = %s}, %d.%d, BindingHandle = 0x%X (%X), caller = '%s'", Function, interfaceID,
                rpcInterface->InterfaceId.SyntaxVersion.MajorVersion, rpcInterface->InterfaceId.SyntaxVersion.MinorVersion, pHandle ? *pHandle : NULL, HandleType, CallingModule);
        }
        else
        {
            Sbie_snwprintf(text, 512, L"Calling %s caller = '%s'", Function, CallingModule);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        Sbie_snwprintf(text, 512, L"Calling %s, caused log exception, caller = '%s'", Function, CallingModule);
    }

    SbieApi_MonitorPutMsg(MONITOR_RPC | MONITOR_TRACE, text);
}


#ifdef _WIN64

ULONG_PTR RpcRt_NdrClientCall2_x64(
    PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING pFormat, void** pStack)
{
    void* ReturnAddress = *(pStack - 3);

    RpcRt_NdrClientCallX(L"NdrClientCall2", ReturnAddress, pStubDescriptor, pFormat, pStack);

    return FALSE; // return TRUE to not call the trampoline upon return
}

ULONG_PTR RpcRt_NdrClientCall3_x64(
    MIDL_STUBLESS_PROXY_INFO* pProxyInfo, ULONG nProcNum, void* pReturnValue, void** pStack)
{
    void* ReturnAddress = *(pStack - 4);

    RpcRt_NdrClientCallX(L"NdrClientCall3", ReturnAddress, pProxyInfo->pStubDesc, pProxyInfo->ProcFormatString, pStack);

    return FALSE; // return TRUE to not call the trampoline upon return
}

#else

ULONG_PTR __cdecl RpcRt_NdrClientCall_x86(
    void* ReturnAddress,
    PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING pFormat, void** pStack)
{
    RpcRt_NdrClientCallX(L"NdrClientCall", ReturnAddress, pStubDescriptor, pFormat, pStack);

    return FALSE; // return TRUE to not call the trampoline upon return
}

ULONG_PTR __cdecl RpcRt_NdrClientCall2_x86(
    void* ReturnAddress,
    PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING pFormat, void** pStack)
{
    RpcRt_NdrClientCallX(L"NdrClientCall2", ReturnAddress, pStubDescriptor, pFormat, pStack);

    return FALSE; // return TRUE to not call the trampoline upon return
}

#endif _WIN64


//---------------------------------------------------------------------------
// RpcRt_NdrAsyncClientCall
//---------------------------------------------------------------------------

#ifdef _WIN64

ULONG_PTR RpcRt_NdrAsyncClientCall_x64(
    PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING pFormat, void** pStack)
{
    void* ReturnAddress = *(pStack - 3);

    // pStack[0] -> RPC_ASYNC_STATE
    // pStack[1] -> RPC_BINDING_HANDLE
    // other args

    RpcRt_NdrClientCallX(L"NdrAsyncClientCall", ReturnAddress, pStubDescriptor, pFormat, &pStack[1]);

    return FALSE; // return TRUE to not call the trampoline upon return
}

ALIGNED BOOLEAN __cdecl RpcRt_Ndr64AsyncClientCall_x64(
    MIDL_STUBLESS_PROXY_INFO* pProxyInfo, ULONG nProcNum, void* pReturnValue, void** pStack)
{
    void* ReturnAddress = *(pStack - 4);

    if (g_rpc_client_hooks) {
        RpcRt_NdrClientCallX(L"Ndr64AsyncClientCall", ReturnAddress, pProxyInfo->pStubDesc, pProxyInfo->ProcFormatString, &pStack[1]);
    }

    if (!SbieApi_QueryConfBool(NULL, L"NoUACProxy", FALSE))
    if (Dll_OsBuild >= 6000) {
        return Secure_CheckElevation((struct SECURE_UAC_ARGS*)pStack);
    }
    return FALSE; // return TRUE to not call the trampoline upon return
}

#else 

ALIGNED BOOLEAN __cdecl RpcRt_NdrAsyncClientCall_x86(
    void* ReturnAddress,
    PMIDL_STUB_DESC pStubDescriptor, void* pFormat, void** pStack)
{
    if (g_rpc_client_hooks) {
        RpcRt_NdrClientCallX(L"NdrAsyncClientCall", ReturnAddress, pStubDescriptor, pFormat, &pStack[1]);
    }

    if (!SbieApi_QueryConfBool(NULL, L"NoUACProxy", FALSE))
    if (Dll_OsBuild >= 6000) {
        return Secure_CheckElevation((struct SECURE_UAC_ARGS*)pStack);
    }
    return FALSE; // return TRUE to not call the trampoline upon return
}

#endif _WIN64


//---------------------------------------------------------------------------
// Secure_RpcAsyncCompleteCall
//---------------------------------------------------------------------------


ALIGNED ULONG_PTR RpcRt_RpcAsyncCompleteCall(
    RPC_ASYNC_STATE* AsyncState, void* Reply)
{
    if (Dll_OsBuild >= 6000) {
        if (Secure_RpcAsyncCompleteCall(AsyncState, Reply))
            return 0;
    }

    return __sys_RpcAsyncCompleteCall(AsyncState, Reply);
}