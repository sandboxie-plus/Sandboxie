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


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static ULONG RpcRt_RpcBindingInqAuthClientEx(
    void *ClientBinding, void *Privs, void **ServerPrincName,
    ULONG *AuthnLevel, ULONG *AuthnSvc, ULONG *AuthzSvc, ULONG Flags);

static ULONG RpcRt_RpcBindingFromStringBindingW(
    const WCHAR *StringBinding, void **OutBinding);

static RPC_STATUS NsiRpc_NsiRpcRegisterChangeNotification(
    LPVOID  p1, LPVOID  p2, LPVOID  p3, LPVOID  p4, LPVOID  p5, LPVOID  p6, LPVOID  p7);

static RPC_STATUS RpcRt_RpcBindingCreateW(
    RPC_BINDING_HANDLE_TEMPLATE_V1_W * Template,
    RPC_BINDING_HANDLE_SECURITY_V1_W * Security,
    RPC_BINDING_HANDLE_OPTIONS_V1 * Options,
    RPC_BINDING_HANDLE * Binding);

void *Scm_QueryServiceByName(
    const WCHAR *ServiceNm,
    ULONG with_service_status, ULONG with_service_config);

int Scm_Start_Sppsvc();


#ifdef _WIN64

extern ULONG_PTR __cdecl RpcRt_NdrClientCall3(
    MIDL_STUBLESS_PROXY_INFO* pProxyInfo, ULONG nProcNum, void* pReturnValue, ...);

#else

//extern ULONG_PTR __cdecl RpcRt_NdrClientCall3(
//    MIDL_STUBLESS_PROXY_INFO* pProxyInfo, ULONG nProcNum, void* pReturnValue, ULONG_PTR* Args);

extern ULONG_PTR __cdecl RpcRt_NdrClientCall2(
    PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING  pFormat, ULONG_PTR* Args);

#endif _WIN64


//---------------------------------------------------------------------------


typedef ULONG (*P_RpcBindingInqAuthClientEx)(
    void *ClientBinding, void *Privs, void **ServerPrincName,
    ULONG *AuthnLevel, ULONG *AuthnSvc, ULONG *AuthzSvc, ULONG Flags);

typedef ULONG (*P_RpcBindingFromStringBindingW)(
    const WCHAR *StringBinding, void **OutBinding);

typedef RPC_STATUS (*P_NsiRpcRegisterChangeNotification)(
    LPVOID  p1, LPVOID  p2, LPVOID  p3, LPVOID  p4, LPVOID  p5, LPVOID  p6, LPVOID  p7);

typedef RPC_STATUS (*P_RpcBindingCreateW)(
    RPC_BINDING_HANDLE_TEMPLATE_V1_W * Template,
    RPC_BINDING_HANDLE_SECURITY_V1_W * Security,
    RPC_BINDING_HANDLE_OPTIONS_V1 * Options,
    RPC_BINDING_HANDLE * Binding);

typedef RPC_STATUS (RPC_ENTRY *P_RpcStringBindingComposeW)( TCHAR *ObjUuid, TCHAR *ProtSeq, TCHAR *NetworkAddr, TCHAR *EndPoint, TCHAR *Options, TCHAR **StringBinding);
RPC_STATUS RPC_ENTRY RpcRt_RpcStringBindingComposeW(
   TCHAR *ObjUuid,
   TCHAR *ProtSeq,
   TCHAR *NetworkAddr,
   TCHAR *EndPoint,
   TCHAR *Options,
   TCHAR **StringBinding
);

typedef RPC_STATUS (RPC_ENTRY *P_UuidToStringW)(const UUID *Uuid, RPC_WSTR *StringUuid);

typedef RPC_STATUS(RPC_ENTRY *P_RpcStringFreeW)(RPC_WSTR *String);

typedef RPC_STATUS (*P_RpcMgmtSetComTimeout)(RPC_BINDING_HANDLE Binding, unsigned int __RPC_FAR Timeout);

typedef BOOL(WINAPI* P_GetModuleInformation)(_In_ HANDLE hProcess, _In_ HMODULE hModule, _Out_ LPMODULEINFO lpmodinfo, _In_ DWORD cb);

typedef ULONG_PTR(__cdecl* P_NdrClientCall3)(
    MIDL_STUBLESS_PROXY_INFO* pProxyInfo, ULONG nProcNum, void* pReturnValue, ...);

typedef ULONG_PTR(__cdecl* P_NdrClientCall2)(
    PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING  pFormat, ...);

//---------------------------------------------------------------------------


P_RpcStringBindingComposeW  __sys_RpcStringBindingComposeW = NULL;

P_RpcBindingInqAuthClientEx __sys_RpcBindingInqAuthClientEx = NULL;

P_RpcBindingFromStringBindingW __sys_RpcBindingFromStringBindingW = NULL;

P_NsiRpcRegisterChangeNotification __sys_NsiRpcRegisterChangeNotification = NULL;

P_RpcBindingCreateW __sys_RpcBindingCreateW = NULL;

P_RpcMgmtSetComTimeout __sys_RpcMgmtSetComTimeout = NULL;
#define RPC_C_BINDING_TIMEOUT   4

P_UuidToStringW __sys_UuidToStringW = NULL;

P_RpcStringFreeW __sys_RpcStringFreeW = NULL;

P_GetModuleInformation __sys_GetModuleInformation = NULL;

#ifdef _WIN64
P_NdrClientCall3  __sys_NdrClientCall3 = NULL;
#else
P_NdrClientCall2  __sys_NdrClientCall2 = NULL;
#endif

extern WCHAR* g_Ipc_DynamicPortNames[NUM_DYNAMIC_PORTS];


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

    if (Dll_OsBuild >= 6000)        // Vista
    {
        P_RpcBindingFromStringBindingW  RpcBindingFromStringBindingW;
        P_RpcBindingCreateW     RpcBindingCreateW;

        if ( Dll_OsBuild >= 15063) {
            void *RpcStringBindingComposeW;
            RpcStringBindingComposeW =  (void *) Ldr_GetProcAddrNew(DllName_rpcrt4, L"RpcStringBindingComposeW","RpcStringBindingComposeW");
            SBIEDLL_HOOK(RpcRt_,RpcStringBindingComposeW);
        }

        RpcBindingFromStringBindingW = (P_RpcBindingFromStringBindingW)
            Ldr_GetProcAddrNew(DllName_rpcrt4, L"RpcBindingFromStringBindingW", "RpcBindingFromStringBindingW");

        SBIEDLL_HOOK(RpcRt_, RpcBindingFromStringBindingW);

        RpcBindingCreateW = (P_RpcBindingCreateW)
            Ldr_GetProcAddrNew(DllName_rpcrt4, L"RpcBindingCreateW", "RpcBindingCreateW");

        SBIEDLL_HOOK(RpcRt_, RpcBindingCreateW);

        if(Config_GetSettingsForImageName_bool(L"RpcMgmtSetComTimeout", TRUE))
            __sys_RpcMgmtSetComTimeout = (P_RpcMgmtSetComTimeout)Ldr_GetProcAddrNew(DllName_rpcrt4, L"RpcMgmtSetComTimeout", "RpcMgmtSetComTimeout");
    }

    WCHAR   wsTraceOptions[4];
    if ((Dll_OsBuild >= 8400) // win8 and above
    && SbieApi_QueryConf(NULL, L"IpcTrace", 0, wsTraceOptions, sizeof(wsTraceOptions)) == STATUS_SUCCESS && wsTraceOptions[0] != L'\0')
    {
#ifdef _WIN64

        P_NdrClientCall3 NdrClientCall3;
        NdrClientCall3 = (P_NdrClientCall3)Ldr_GetProcAddrNew(DllName_rpcrt4, L"NdrClientCall3", "NdrClientCall3");
        SBIEDLL_HOOK(RpcRt_, NdrClientCall3);

#else

        P_NdrClientCall2 NdrClientCall2;
        NdrClientCall2 = (P_NdrClientCall2)Ldr_GetProcAddrNew(DllName_rpcrt4, L"NdrClientCall2", "NdrClientCall2");
        SBIEDLL_HOOK(RpcRt_, NdrClientCall2);

#endif
    }

    return Secure_Init_Elevation(module);
}


//---------------------------------------------------------------------------
// NsiRpc_Init
//---------------------------------------------------------------------------

_FX BOOLEAN NsiRpc_Init(HMODULE module)
{
    P_NsiRpcRegisterChangeNotification NsiRpcRegisterChangeNotification;

    NsiRpcRegisterChangeNotification = (P_NsiRpcRegisterChangeNotification)
        Ldr_GetProcAddrNew(DllName_winnsi, L"NsiRpcRegisterChangeNotification","NsiRpcRegisterChangeNotification");

    SBIEDLL_HOOK(NsiRpc_,NsiRpcRegisterChangeNotification);

    return TRUE;
}

//  In Win8.1 WININET initialization needs to register network change events into NSI service (Network Store Interface Service).
//  The communication between guest and NSI service is via epmapper. We currently block epmapper and it blocks guest and NSI service.
//  This causes IE to pop up a dialog "Revocation information for the security certificate for this site is not available. Do you want to proceed?"
//  The fix can be either - 
//  1.  Allowing guest to talk to NSI service by fixing RpcBindingCreateW like what we did for keyiso-crypto and Smart Card service.
//      ( We don't fully know what we actually open to allow guest to talk to NSI service and if it is needed. It has been blocked. )
//  2. Hooking NsiRpcRegisterChangeNotification and silently returning NO_ERROR from the hook.
//      ( Guest app won't get Network Change notification. I am not sure if this is needed for the APP we support. )
//  We choose Fix 2 here. We may need fix 1 if see a need in the future.


_FX RPC_STATUS NsiRpc_NsiRpcRegisterChangeNotification(LPVOID  p1, LPVOID  p2, LPVOID  p3, LPVOID  p4, LPVOID  p5, LPVOID  p6, LPVOID  p7)
{
    RPC_STATUS ret = __sys_NsiRpcRegisterChangeNotification(p1, p2, p3, p4, p5, p6, p7);

    if (EPT_S_NOT_REGISTERED == ret)
    {
        ret = NO_ERROR;
    }
    return ret;
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
// GetDynamicLpcPortName
//---------------------------------------------------------------------------


WCHAR* GetDynamicLpcPortName(ENUM_DYNAMIC_PORT_TYPE portType)
{
    EPMAPPER_GET_PORT_NAME_REQ req;
    EPMAPPER_GET_PORT_NAME_RPL* rpl;

    memset(&req, 0, sizeof(req));
    req.h.length = sizeof(EPMAPPER_GET_PORT_NAME_REQ);
    req.h.msgid = MSGID_EPMAPPER_GET_PORT_NAME;
    req.portType = portType;

    rpl = (EPMAPPER_GET_PORT_NAME_RPL*)SbieDll_CallServer(&req.h);

    WCHAR   wsTraceOptions[4];
    if (SbieApi_QueryConf(NULL, L"IpcTrace", 0, wsTraceOptions, sizeof(wsTraceOptions)) == STATUS_SUCCESS && wsTraceOptions[0] != L'\0')
    {
        WCHAR text[130];

        if (rpl && NT_SUCCESS(rpl->h.status))
            Sbie_snwprintf(text, 130, L"Resolved dynamic port: %d; endpoint: %s", req.portType, rpl->wszPortName);
        else
            Sbie_snwprintf(text, 130, L"Failed to resolve dynamic port: %d; status: %08X", req.portType, rpl ? rpl->h.status : 0);

        SbieApi_MonitorPut2(MONITOR_IPC | MONITOR_TRACE, text, FALSE);
    }

    if (rpl && NT_SUCCESS(rpl->h.status))
    {
        wcsncpy(g_Ipc_DynamicPortNames[portType], rpl->wszPortName, DYNAMIC_PORT_NAME_CHARS);

        Dll_Free(rpl);
        return g_Ipc_DynamicPortNames[portType] + 13; // skip "\\RPC Control\\"
    }

    return NULL;
}


//---------------------------------------------------------------------------
// RpcRt_RpcBindingFromStringBindingW
//---------------------------------------------------------------------------


_FX ULONG RpcRt_RpcBindingFromStringBindingW(
    const WCHAR *StringBinding, void **OutBinding)
{
    static const WCHAR *_old =
        L"ncalrpc:[,Security=Impersonation Dynamic False]";

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

    WCHAR wstrPortName[DYNAMIC_PORT_NAME_CHARS];
    memset(wstrPortName, 0, sizeof(wstrPortName));

    if (_wcsicmp(StringBinding, _old) == 0) {

        ULONG_PTR pWinSpool = (ULONG_PTR)GetModuleHandle(L"winspool.drv");
        ULONG_PTR pRetAddr = (ULONG_PTR)_ReturnAddress();

        if (RpcRt_TestCallingModule(pRetAddr, pWinSpool)) {

            WCHAR* pwszTempPortName = GetDynamicLpcPortName(SPOOLER_PORT);

            if (pwszTempPortName == NULL)
                return RPC_S_ACCESS_DENIED;

            wcscpy(wstrPortName, L"ncalrpc:[");
            wcscpy(wstrPortName + 9, pwszTempPortName);
            wcscat(wstrPortName, _old + 9);
        }
    }

    else if (_wcsicmp(StringBinding, L"ncalrpc:") == 0) {
    
        WCHAR pwszEmpty[] = L"";
        WCHAR* pwszTempPortName = pwszEmpty;
    
        ULONG_PTR hWinHttp = (ULONG_PTR)GetModuleHandle(L"WinHttp.dll");
        ULONG_PTR hBtApi = (ULONG_PTR)GetModuleHandle(L"BluetoothApis.dll");
        ULONG_PTR pRetAddr = (ULONG_PTR)_ReturnAddress();

        if (RpcRt_TestCallingModule(pRetAddr, hWinHttp))
        {
            // WPAD (Windows Proxy Auto Discovery) uses dynamic RPC endpoints starting in Win 10 Anniv.
            pwszTempPortName = GetDynamicLpcPortName(WPAD_PORT);
        }
        else if (RpcRt_TestCallingModule(pRetAddr, hBtApi))
        {
            // Bluetooth support service
            pwszTempPortName = GetDynamicLpcPortName(BT_PORT);
        }
        
        if (pwszTempPortName != pwszEmpty) {

            if (pwszTempPortName == NULL)
                return RPC_S_ACCESS_DENIED;

            wcscpy(wstrPortName, L"ncalrpc:[");
            wcscpy(wstrPortName + 9, pwszTempPortName);
            wcscat(wstrPortName, L"]");
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
        Sbie_snwprintf(msg, 512, L"StringBinding = '%s', wstrPortName = '%s', BindingHandle = 0x%X, status = 0x%08X",
            StringBinding, wstrPortName, OutBinding, status);

        //OutputDebugString(msg);
        SbieApi_MonitorPut2(MONITOR_IPC | MONITOR_TRACE, msg, FALSE);
    }
    if(__sys_RpcMgmtSetComTimeout) __sys_RpcMgmtSetComTimeout(*OutBinding, RPC_C_BINDING_TIMEOUT); 
    return status;
}


DEFINE_GUID(
    EMPTY_UUID,
    0x00000000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

// {906B0CE0-C70B-1067-B317-00DD010662DA}
DEFINE_GUID(
    MSDTC_UUID,
    0x906B0CE0, 0xC70B, 0x1067, 0xB3, 0x17, 0x00, 0xDD, 0x01, 0x06, 0x62, 0xDA);

_FX RPC_STATUS RpcRt_RpcBindingCreateW(
    __in RPC_BINDING_HANDLE_TEMPLATE_V1_W * Template,
    __in_opt RPC_BINDING_HANDLE_SECURITY_V1_W * Security,
    __in_opt RPC_BINDING_HANDLE_OPTIONS_V1 * Options,
    __out RPC_BINDING_HANDLE * Binding)
{
    if ( (memcmp(&Template->ObjectUuid, &MSDTC_UUID, sizeof(GUID)) == 0) && RPC_PROTSEQ_LRPC == Template->ProtocolSequence && !Template->StringEndpoint)
    {
        Template->StringEndpoint = (unsigned short*)L"samss lpc";
    }

    else if ( (memcmp(&Template->ObjectUuid, &EMPTY_UUID, sizeof(GUID)) == 0) &&
        RPC_PROTSEQ_LRPC == Template->ProtocolSequence &&
        !Template->StringEndpoint)
    {
        ULONG_PTR hWinSCard = (ULONG_PTR)GetModuleHandle(L"WinSCard.dll");
        ULONG_PTR hResourcePolicyClient = (ULONG_PTR)GetModuleHandle(L"resourcepolicyclient.dll");
        ULONG_PTR pRetAddr = (ULONG_PTR)_ReturnAddress();

        if (RpcRt_TestCallingModule(pRetAddr, hWinSCard))
        {
            // smart card interface {C6B5235A-E413-481D-9AC8-31681B1FAAF5}  
            Template->StringEndpoint = GetDynamicLpcPortName(SMART_CARD_PORT);
        }
        else if (RpcRt_TestCallingModule(pRetAddr, hResourcePolicyClient))
        {
            // Win 10 AU WinRT interface - {88ABCBC3-34EA-76AE-8215-767520655A23}
            Template->StringEndpoint = GetDynamicLpcPortName(GAME_CONFIG_STORE_PORT);
        }
    }

    RPC_STATUS  status;
    status = __sys_RpcBindingCreateW(Template, Security, Options, Binding);
    // If there are any IpcTrace options set, then output this debug string
    WCHAR   wsTraceOptions[4];
    if (SbieApi_QueryConf(NULL, L"IpcTrace", 0, wsTraceOptions, sizeof(wsTraceOptions)) == STATUS_SUCCESS && wsTraceOptions[0] != L'\0')
    {
        WCHAR msg[512];
        RPC_CSTR   StringUuid;

        __sys_UuidToStringW(&Template->ObjectUuid, &StringUuid);
        //Sbie_snwprintf(msg, 512, L"SBIE p=%06d t=%06d RpcBindingCreateW Endpoint = '%s', UUID = %s, status = 0x%X\n", GetCurrentProcessId(), GetCurrentThreadId(),
        Sbie_snwprintf(msg, 512, L"Endpoint = '%s', UUID = %s, status = 0x%08X", 
            Template && Template->StringEndpoint ? Template->StringEndpoint : L"null",
            StringUuid, status);
        __sys_RpcStringFreeW(&StringUuid);

        //OutputDebugString(msg);
        SbieApi_MonitorPut2(MONITOR_IPC | MONITOR_TRACE, msg, FALSE);
    }
    if (__sys_RpcMgmtSetComTimeout) __sys_RpcMgmtSetComTimeout(*Binding, RPC_C_BINDING_TIMEOUT);
    return status;
}


#define UUID_UserMgrCli L"B18FBAB6-56F8-4702-84E0-41053293A869"

RPC_STATUS RPC_ENTRY RpcRt_RpcStringBindingComposeW(TCHAR *ObjUuid,TCHAR *ProtSeq,TCHAR *NetworkAddr,TCHAR *EndPoint,TCHAR *Options,TCHAR **StringBinding) {
    ULONG_PTR hSppc =  (ULONG_PTR)GetModuleHandle(L"sppc.dll");
    ULONG_PTR pRetAddr = (ULONG_PTR)_ReturnAddress();

    if (RpcRt_TestCallingModule(pRetAddr, hSppc))
    {
        EndPoint = L"SPPCTransportEndpoint-00001";
        Scm_Start_Sppsvc();
    }
    // we must block this in Win 10 to prevent r-click context menu hang in Explorer
    else if (ObjUuid && (!_wcsicmp(ObjUuid, UUID_UserMgrCli)))
    {
        return STATUS_ACCESS_DENIED;
    }
    return __sys_RpcStringBindingComposeW(ObjUuid,ProtSeq,NetworkAddr,EndPoint,Options,StringBinding);
}

void Sbie_StringFromGUID(const GUID* guid, WCHAR* str);

#ifdef _WIN64

ULONG_PTR RpcRt_NdrClientCall3_x64(
    MIDL_STUBLESS_PROXY_INFO* pProxyInfo, ULONG nProcNum, void* pReturnValue, va_list vl
)
{
    void* ReturnAddress = (void*)*(__int64*)(vl - (4 * 8));
    
    __try
    {
        PRPC_CLIENT_INTERFACE rpcInterface = (PRPC_CLIENT_INTERFACE)pProxyInfo->pStubDesc->RpcInterfaceInformation;

        WCHAR interfaceID[48];
        Sbie_StringFromGUID(&rpcInterface->InterfaceId.SyntaxGUID, interfaceID);

        WCHAR text[130];
        Sbie_snwprintf(text, 130, L"Calling NdrClientCall3 for interface %s}, %d.%d", interfaceID,
            rpcInterface->InterfaceId.SyntaxVersion.MajorVersion, rpcInterface->InterfaceId.SyntaxVersion.MinorVersion);

        SbieApi_MonitorPut2(MONITOR_IPC | MONITOR_TRACE, text, FALSE);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}

    return FALSE; // return TRUE to not call the trampoline upon return
}

#else

ULONG_PTR __cdecl RpcRt_NdrClientCall2_x86(
    void* ReturnAddress,
    PMIDL_STUB_DESC pStubDescriptor, PFORMAT_STRING pFormat, ...)
{
    //va_list vl;
    //va_start(vl, pFormat);
    
    __try
    {
        PRPC_CLIENT_INTERFACE rpcInterface = (PRPC_CLIENT_INTERFACE)pStubDescriptor->RpcInterfaceInformation;

        WCHAR interfaceID[48];
        Sbie_StringFromGUID(&rpcInterface->InterfaceId.SyntaxGUID, interfaceID);

        WCHAR text[130];
        Sbie_snwprintf(text, 130, L"Calling NdrClientCall2 for interface %s}, %d.%d", interfaceID,
            rpcInterface->InterfaceId.SyntaxVersion.MajorVersion, rpcInterface->InterfaceId.SyntaxVersion.MinorVersion);

        SbieApi_MonitorPut2(MONITOR_IPC | MONITOR_TRACE, text, FALSE);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}

    //va_end(vl);

    return FALSE; // return TRUE to not call the trampoline upon return
}

#endif _WIN64