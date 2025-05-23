/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2025 David Xanatos, xanasoft.com
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
// Network Related
//---------------------------------------------------------------------------

#include "dll.h"

#include <windows.h>
#include <wchar.h>
#include <oleauto.h>
#include "gui_p.h"
#include "common/my_wsa.h"
#include "common/netfw.h"
#include "common/map.h"
#include "common/str_util.h"
#include "wsa_defs.h"
#include "core/svc/sbieiniwire.h"
#include "common/base64.c"
#include "core/drv/api_defs.h"
#include "core/drv/verify.h"
#include "common/rbtree.h"
#define _WINSOCK2API_
#include <IPTypes.h>


//---------------------------------------------------------------------------
//
// Windows Sockets 2
//
//---------------------------------------------------------------------------

#define SIO_GET_EXTENSION_FUNCTION_POINTER 0xC8000006

#define WSAID_CONNECTEX \
    {0x25a207b9,0xddf3,0x4660,{0x8e,0xe9,0x76,0xe5,0x8c,0x74,0x06,0x3e}}

#define WSAID_ACCEPTEX \
    {0xb5367df1,0xcbac,0x11cf,{0x95,0xca,0x00,0x80,0x5f,0x48,0xa1,0x92}}

//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------

typedef struct NETPROXY_RULE NETPROXY_RULE;

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

static void WSA_InitNetFwRules();

static BOOLEAN WSA_ParseNetProxy(NETPROXY_RULE* proxy, const WCHAR* RuleStr);

static BOOLEAN WSA_InitNetProxy();

static BOOLEAN WSA_InitBindIP();

BOOLEAN WSA_InitNetDnsFilter(HMODULE module);

static int WSA_IsBlockedTraffic(const short *addr, int addrlen, int protocol);

static int WSA_WSAStartup(
    WORD wVersionRequested,
    void* lpWSAData);
    
static int WSA_WSACleanup(void);


static int WSA_WSAIoctl(
    SOCKET                             s,
    DWORD                              dwIoControlCode,
    LPVOID                             lpvInBuffer,
    DWORD                              cbInBuffer,
    LPVOID                             lpvOutBuffer,
    DWORD                              cbOutBuffer,
    LPDWORD                            lpcbBytesReturned,
    LPWSAOVERLAPPED                    lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

static int WSA_ioctlsocket(
    SOCKET  s,
    long    cmd,
    ULONG*  argp);

static int WSA_WSAAsyncSelect(
    SOCKET  s,
    HWND    hWnd,
    UINT    wMsg,
    long    lEvent);

static int WSA_WSAEventSelect(
    SOCKET  s,
    void*   hEventObject,
    long    lNetworkEvents);

static int WSA_WSAEnumNetworkEvents(
    SOCKET  s,
    void*   hEventObject,
    LPWSANETWORKEVENTS lpNetworkEvents);

static int WSA_WSANSPIoctl(
    HANDLE          hLookup,
    DWORD           dwControlCode,
    LPVOID          lpvInBuffer,
    DWORD           cbInBuffer,
    LPVOID          lpvOutBuffer,
    DWORD           cbOutBuffer,
    LPDWORD         lpcbBytesReturned,
    LPWSACOMPLETION lpCompletion);

static int WSA_GetAddrInfoW(
    PCWSTR pNodeName,
    PCWSTR pServiceName,
    const ADDRINFOW* pHints,
    PADDRINFOW* ppResult);

static void WSA_FreeAddrInfoW(
    PADDRINFOW      pAddrInfo);

static SOCKET WSA_WSASocketW(
    int                 af,
    int                 type,
    int                 protocol,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,
    unsigned int        g,
    DWORD               dwFlags);

static int WSA_bind(
    SOCKET         s,
    const void     *name,
    int            namelen);

static int WSA_connect(
    SOCKET         s,
    const void     *name,
    int            namelen);

static int WSA_WSAConnect(
    SOCKET         s,
    const void     *name,
    int            namelen,
    LPWSABUF       lpCallerData,
    LPWSABUF       lpCalleeData,
    LPQOS          lpSQOS,
    LPQOS          lpGQOS);

static int WSA_ConnectEx(
    SOCKET          s,
    const void      *name,
    int             namelen,
    PVOID           lpSendBuffer,
    DWORD           dwSendDataLength,
    LPDWORD         lpdwBytesSent,
    LPOVERLAPPED    lpOverlapped);

/*static int WSA_listen(
    SOCKET          s,
    int             backlog);

static SOCKET WSA_accept(
    SOCKET   s,
    void     *addr,
    int      *addrlen);

static SOCKET WSA_WSAAccept(
    SOCKET          s,
    void            *addr,
    LPINT           addrlen,
    LPCONDITIONPROC lpfnCondition,
    DWORD_PTR       dwCallbackData);

static int WSA_AcceptEx(
    SOCKET       sListenSocket,
    SOCKET       sAcceptSocket,
    PVOID        lpOutputBuffer,
    DWORD        dwReceiveDataLength,
    DWORD        dwLocalAddressLength,
    DWORD        dwRemoteAddressLength,
    LPDWORD      lpdwBytesReceived,
    LPOVERLAPPED lpOverlapped);*/

static int WSA_sendto(
    SOCKET         s,
    const char     *buf,
    int            len,
    int            flags,
    const void     *to,
    int            tolen);

static int WSA_WSASendTo(
    SOCKET                             s,
    LPWSABUF                           lpBuffers,
    DWORD                              dwBufferCount,
    LPDWORD                            lpNumberOfBytesSent,
    DWORD                              dwFlags,
    const void                         *lpTo,
    int                                iTolen,
    LPWSAOVERLAPPED                    lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

static int WSA_recvfrom(
    SOCKET   s,
    char     *buf,
    int      len,
    int      flags,
    void     *from,
    int      *fromlen);

static int WSA_WSARecvFrom(
    SOCKET                             s,
    LPWSABUF                           lpBuffers,
    DWORD                              dwBufferCount,
    LPDWORD                            lpNumberOfBytesRecvd,
    LPDWORD                            lpFlags,
    void                               *lpFrom,
    LPINT                              lpFromlen,
    LPWSAOVERLAPPED                    lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

static int WSA_closesocket(SOCKET s);


BOOLEAN socks5_handshake(SOCKET s, BOOLEAN auth, WCHAR login[256], WCHAR pass[256]);

char socks5_request(SOCKET s, const SOCKADDR* addr);

USHORT start_socks5_relay(const SOCKADDR* addr, const SOCKADDR* proxy, BOOLEAN auth, WCHAR login[256], WCHAR pass[256]);


//---------------------------------------------------------------------------

static P_WSAStartup         __sys_WSAStartup        = NULL;

static P_WSACleanup         __sys_WSACleanup        = NULL;

       P_socket             __sys_socket            = NULL;

static P_WSAIoctl           __sys_WSAIoctl          = NULL;

static P_WSAGetLastError    __sys_WSAGetLastError   = NULL;
static P_WSASetLastError    __sys_WSASetLastError   = NULL;

static P_ioctlsocket        __sys_ioctlsocket       = NULL;
static P_WSAAsyncSelect     __sys_WSAAsyncSelect    = NULL;
static P_WSAEventSelect     __sys_WSAEventSelect    = NULL;

       P_select             __sys_select            = NULL;

static P_WSAEnumNetworkEvents __sys_WSAEnumNetworkEvents = NULL;

static P_WSANSPIoctl        __sys_WSANSPIoctl       = NULL;

static P_WSASocketW         __sys_WSASocketW        = NULL;

       P_bind               __sys_bind              = NULL;

       P_getsockname        __sys_getsockname       = NULL;

       P_WSAFDIsSet         __sys_WSAFDIsSet        = NULL;

       P_connect            __sys_connect           = NULL;
static P_WSAConnect         __sys_WSAConnect        = NULL;
static P_ConnectEx          __sys_ConnectEx         = NULL;

       P_listen             __sys_listen            = NULL;

       P_accept             __sys_accept            = NULL;
/*static P_WSAAccept          __sys_WSAAccept         = NULL;
static P_AcceptEx           __sys_AcceptEx          = NULL;*/

       P_recv               __sys_recv              = NULL;

       P_send               __sys_send              = NULL;
static P_sendto             __sys_sendto            = NULL;
static P_WSASendTo          __sys_WSASendTo         = NULL;

static P_recvfrom           __sys_recvfrom          = NULL;
static P_WSARecvFrom        __sys_WSARecvFrom       = NULL;

static P_shutdown           __sys_shutdown          = NULL;

       P_closesocket        __sys_closesocket       = NULL;

static P_GetAddrInfoW       __sys_GetAddrInfoW      = NULL;

static P_FreeAddrInfoW      __sys_FreeAddrInfoW     = NULL;

       P_inet_ntop          __sys_inet_ntop         = NULL;

//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------

struct NETPROXY_RULE {
    int used;

    SOCKADDR_IN     WSA_ProxyAddr;
    SOCKADDR_IN6_LH WSA_ProxyAddr6;

    BOOLEAN auth;
    WCHAR   login[255];
    WCHAR   pass[255];

    rbtree_t ip_map;
};

extern POOL*            Dll_Pool;

static LIST             WSA_FwList;

static BOOLEAN          WSA_WFPisEnabled      = FALSE;
static BOOLEAN          WSA_WFPisBlocking     = FALSE;

static BOOLEAN          WSA_TraceFlag         = FALSE;

static BOOLEAN          WSA_StartupDone       = FALSE;
static BOOLEAN          WSA_ProxyEnabled      = FALSE;
static BOOLEAN          WSA_ProxyThread       = FALSE;
static BOOLEAN          WSA_ProxyHack         = FALSE;
static NETPROXY_RULE*   WSA_Proxy4            = NULL;
static NETPROXY_RULE*   WSA_Proxy6            = NULL;
#ifdef PROXY_RESOLVE_HOST_NAMES
       HASH_MAP         DNS_LookupMap;
#endif

static BOOLEAN          WSA_BindIP            = FALSE;
static SOCKADDR_IN      WSA_BindIP4           = {0};
static SOCKADDR_IN6_LH  WSA_BindIP6           = {0};

typedef struct _WSA_SOCK {

    USHORT af;

    ULONG NonBlocking;

    BOOLEAN ConnectedSet;

    // WSAAsyncSelect
    HWND    hWnd;
    UINT    wMsg;
    long    lEvent;

    // WSAEventSelect
    void*   hEventObject;
    long    lNetworkEvents;

    // Binding
    BOOLEAN Bound;

} WSA_SOCK;

static HASH_MAP   WSA_SockMap;



//---------------------------------------------------------------------------
// WSA_GetSock
//---------------------------------------------------------------------------


_FX WSA_SOCK* WSA_GetSock(SOCKET s, BOOLEAN bCanAdd)
{
    WSA_SOCK* pSock = (WSA_SOCK*)map_get(&WSA_SockMap, (void*)s);
    if (pSock == NULL && bCanAdd)
        pSock = (WSA_SOCK*)map_insert(&WSA_SockMap, (void*)s, NULL, sizeof(WSA_SOCK)); // returns a MemZero'ed object
    return pSock;
}


//---------------------------------------------------------------------------
// WSA_WSAStartup
//---------------------------------------------------------------------------


_FX int WSA_WSAStartup(
    WORD wVersionRequested,
    void* lpWSAData)
{
    int ret = __sys_WSAStartup(wVersionRequested, lpWSAData);
    if (ret != 0)
        return ret;

    if (WSA_StartupDone)
        return 0;
    WSA_StartupDone = TRUE;

    //
    // Initialize network proxy
    //

    if (WSA_InitNetProxy()) {

        WSA_ProxyEnabled = TRUE;

        SCertInfo CertInfo = { 0 }; // experimental not yet for public
        if (NT_SUCCESS(SbieApi_QueryDrvInfo(-1, &CertInfo, sizeof(CertInfo))) && (CertInfo.type == eCertDeveloper || CERT_IS_TYPE(CertInfo, eCertEternal)))
            WSA_ProxyThread = SbieApi_QueryConfBool(NULL, L"UseProxyThreads", FALSE);

        if (!WSA_ProxyThread)
            WSA_ProxyHack = TRUE;
    }

    //
    // Initialize bind
    //

    if (WSA_InitBindIP()) {

        WSA_BindIP = TRUE;
    }

    //
    // Init helper map if needed
    //

    if (WSA_ProxyHack || WSA_BindIP) {

        map_init(&WSA_SockMap, Dll_Pool);
    }

    return 0;
}


//---------------------------------------------------------------------------
// WSA_WSACleanup
//---------------------------------------------------------------------------


_FX int WSA_WSACleanup(void)
{
    return __sys_WSACleanup();
}


//---------------------------------------------------------------------------
// WSAIoctl
//---------------------------------------------------------------------------


_FX int WSA_WSAIoctl(
    SOCKET                             s,
    DWORD                              dwIoControlCode,
    LPVOID                             lpvInBuffer,
    DWORD                              cbInBuffer,
    LPVOID                             lpvOutBuffer,
    DWORD                              cbOutBuffer,
    LPDWORD                            lpcbBytesReturned,
    LPWSAOVERLAPPED                    lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
    int rv = __sys_WSAIoctl(s, dwIoControlCode, lpvInBuffer, cbInBuffer, lpvOutBuffer, cbOutBuffer, lpcbBytesReturned, lpOverlapped, lpCompletionRoutine);

    if (rv == 0 && dwIoControlCode == SIO_GET_EXTENSION_FUNCTION_POINTER 
        && lpvInBuffer && cbInBuffer >= sizeof(GUID) && lpvOutBuffer && cbOutBuffer >= sizeof(void*)) {

        GUID guidConnectEx = WSAID_CONNECTEX;
        //GUID guidAcceptEx = WSAID_ACCEPTEX;

        if (memcmp(lpvInBuffer, &guidConnectEx, sizeof(guidConnectEx)) == 0)
        {
            memcpy(&__sys_ConnectEx, lpvOutBuffer, sizeof(void*)); // save the original function address
            void* detour_func = WSA_ConnectEx;
            memcpy(lpvOutBuffer, &detour_func, sizeof(void*)); // and return our detour function instead
        }
        /*else if (memcmp(lpvInBuffer, &guidAcceptEx, sizeof(guidAcceptEx)) == 0)
        {
            memcpy(&__sys_AcceptEx, lpvOutBuffer, sizeof(void*)); // save the original function address
            void* detour_func = WSA_AcceptEx;
            memcpy(lpvOutBuffer, &detour_func, sizeof(void*)); // and return our detour function instead
        }*/
    }

    return rv;
}


//---------------------------------------------------------------------------
// WSA_ioctlsocket
//---------------------------------------------------------------------------


_FX int WSA_ioctlsocket(
    SOCKET  s,
    long    cmd,
    ULONG*  argp)
{
    //
    // Note: on 64 bit windows this function just calls WSAIoctl
    // on 32 bit windows it does its own thing
    // for now we will only monitor ioctlsocket
    //

    if (WSA_ProxyHack && (cmd == FIONBIO) && argp) 
        WSA_GetSock(s, TRUE)->NonBlocking = *argp;

    return __sys_ioctlsocket(s, cmd, argp);
}


//---------------------------------------------------------------------------
// WSA_WSAAsyncSelect
//---------------------------------------------------------------------------


_FX int WSA_WSAAsyncSelect(
    SOCKET  s,
    HWND    hWnd,
    UINT    wMsg,
    long    lEvent)
{
    if (WSA_ProxyHack) {
        WSA_SOCK* pSock = WSA_GetSock(s, TRUE);

        pSock->hWnd = hWnd;
        pSock->wMsg = wMsg;
        pSock->lEvent = lEvent;

        pSock->NonBlocking = TRUE;
    }

    return __sys_WSAAsyncSelect(s, hWnd, wMsg, lEvent);
}


//---------------------------------------------------------------------------
// WSA_WSAEventSelect
//---------------------------------------------------------------------------


_FX int WSA_WSAEventSelect(
    SOCKET  s,
    void*   hEventObject,
    long    lNetworkEvents)
{
    if (WSA_ProxyHack) {
        WSA_SOCK* pSock = WSA_GetSock(s, TRUE);

        pSock->hEventObject = hEventObject;
        pSock->lNetworkEvents = lNetworkEvents;

        pSock->NonBlocking = TRUE;
    }

    return __sys_WSAEventSelect(s, hEventObject, lNetworkEvents);
}


//---------------------------------------------------------------------------
// WSA_WSAEnumNetworkEvents
//---------------------------------------------------------------------------


_FX int WSA_WSAEnumNetworkEvents(
    SOCKET  s,
    void*   hEventObject,
    LPWSANETWORKEVENTS lpNetworkEvents)
{
    int ret = __sys_WSAEnumNetworkEvents(s, hEventObject, lpNetworkEvents);

    if (WSA_ProxyHack) {
        WSA_SOCK* pSock = WSA_GetSock(s, TRUE);

        if (pSock->ConnectedSet) {
            lpNetworkEvents->lNetworkEvents |= FD_CONNECT;
            lpNetworkEvents->iErrorCode[FD_CONNECT_BIT] = 0;
            pSock->ConnectedSet = FALSE;
        }
    }

    return ret;
}


//---------------------------------------------------------------------------
// WSANSPIoctl
//---------------------------------------------------------------------------


_FX int WSA_WSANSPIoctl(
    HANDLE          hLookup,
    DWORD           dwControlCode,
    LPVOID          lpvInBuffer,
    DWORD           cbInBuffer,
    LPVOID          lpvOutBuffer,
    DWORD           cbOutBuffer,
    LPDWORD         lpcbBytesReturned,
    LPWSACOMPLETION lpCompletion)
{
    // the process of automatic proxy detection involves WinInet issuing
    // interleaved WSALookupServiceNext and WSNASPIoctl to request
    // change notifications from the Network Location Awareness (NLA)
    // Service Providers.  Outside the sandbox the WSANSPIoctl() call
    // returns SOCKET_ERROR/WSA_IO_PENDING, but inside the sandbox
    // WSANSPIoctl returns zero error.  Perhaps NLA can't access the
    // event handle specified in the lpCompletion?  In any case, we fake
    // the WSA_IO_PENDING error here and hope for the best.

    int rv = __sys_WSANSPIoctl(
        hLookup, dwControlCode,
        lpvInBuffer, cbInBuffer,
        lpvOutBuffer, cbOutBuffer,
        lpcbBytesReturned, lpCompletion);

    if (rv == 0 && dwControlCode == SIO_NSP_NOTIFY_CHANGE && lpCompletion) {
        SetLastError(WSA_IO_PENDING);
        rv = SOCKET_ERROR;
    }

    return rv;
}


//---------------------------------------------------------------------------
// WSA_WSASocketW
//---------------------------------------------------------------------------

const BOOLEAN File_InternetBlockade_ManualBypass();

static SOCKET WSA_WSASocketW(
  int                 af,
  int                 type,
  int                 protocol,
  LPWSAPROTOCOL_INFOW lpProtocolInfo,
  unsigned int        g,
  DWORD               dwFlags)
{
    // Note: mswsock.dll!WSPSocket is not exported

    if (WSA_WFPisBlocking) {

        //
        // check if the internet is still blocked, the driver will check the setting 
        // and if a runtime exception has been granted to check the WFP state
        // we pass NULL instead of a device name as the block is not device based
        //

        BOOLEAN prompt = SbieApi_QueryConfBool(NULL, L"PromptForInternetAccess", FALSE);
        if (SbieApi_CheckInternetAccess(0, NULL, !prompt) == STATUS_ACCESS_DENIED
            && (!prompt || !File_InternetBlockade_ManualBypass())) {

            //
            // Note: we don't care for the result and we don't want to fail this call
            // we invoke File_InternetBlockade_ManualBypass to give the box manager
            // a chance to allow the network access in the driver
            // 
            // the actual enforcement of the preset is done by the driver
            // 
            // to not make the process crash or behave unexpectedly we always allow 
            // for the socket to be created successfully
            //

        }
        else {

            //
            // don't ask again on success
            //

            WSA_WFPisBlocking = FALSE;
        }
    }

    SOCKET s = __sys_WSASocketW(af, type, protocol, lpProtocolInfo, g, dwFlags);

    if (WSA_ProxyHack || WSA_BindIP)
        WSA_GetSock(s, TRUE)->af = af;

    return s;
}


//---------------------------------------------------------------------------
// WSA_HandleAfUnix
//---------------------------------------------------------------------------


_FX BOOLEAN WSA_HandleAfUnix(const short** paddr, int* paddrlen)
{
    if (!(*paddrlen >= sizeof(SOCKADDR_UN) && *paddr && (*paddr)[0] == AF_UNIX))
        return FALSE; // not AF_UNIX nothing to do

    BOOLEAN ret = FALSE;
    HANDLE handle = INVALID_HANDLE_VALUE;
    WCHAR* path = NULL;

    //
    // use create file to get the proper sandboxed file path, take care of resource access settings 
    // and encure a box path exists if needed
    //

    handle = CreateFileA(((SOCKADDR_UN*)*paddr)->path, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD err = GetLastError();
    if (handle == INVALID_HANDLE_VALUE)
        goto finish;
    
    //
    // if the file was created we need to delete it again
    //

    if (err == 0) { // != ERROR_ALREADY_EXISTS
        
        NTSTATUS File_SetDisposition(
            HANDLE FileHandle, IO_STATUS_BLOCK * IoStatusBlock,
            void* FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass);

        IO_STATUS_BLOCK Iosb;
        FILE_DISPOSITION_INFORMATION fdi;
        fdi.DeleteFileOnClose = TRUE;
        File_SetDisposition(handle, &Iosb, &fdi, sizeof(FILE_DISPOSITION_INFORMATION), FileDispositionInformation);
    }

    //
    // get the path form the handle and translate it to Dos
    //

    path = Dll_Alloc(sizeof(WCHAR) * 8192);

    BOOLEAN IsBoxedPath;
    NTSTATUS status = SbieDll_GetHandlePath(handle, path, &IsBoxedPath);
    if (!NT_SUCCESS(status))
        goto finish;

    if (!SbieDll_TranslateNtToDosPath(path))
        goto finish;
    
    //
    // create a new addr with the new path
    //

    ULONG len = wcslen(path) * 2;
    *paddr = Dll_Alloc(sizeof(SOCKADDR_UN) + len);

    SOCKADDR_UN* un_addr = (SOCKADDR_UN*)*paddr;
    un_addr->family = AF_UNIX;

    len = WideCharToMultiByte(CP_ACP, 0, path, wcslen(path) + 1, un_addr->path, len, NULL, NULL);

    *paddrlen = sizeof(SOCKADDR_UN) + len;

    ret = TRUE;

finish:
    if (handle != INVALID_HANDLE_VALUE) 
        NtClose(handle);
    if (path)
        Dll_Free(path);

    return ret;
}


//---------------------------------------------------------------------------
// WSA_bind
//---------------------------------------------------------------------------

_FX int WSA_bind(
    SOCKET         s,
    const void     *name,
    int            namelen)
{
    BOOLEAN new_name = WSA_HandleAfUnix(&name, &namelen);

    if (WSA_BindIP) {

        if (namelen >= sizeof(SOCKADDR_IN) && name && ((short*)name)[0] == AF_INET) 
        {
            SOCKADDR_IN* addr_in = (SOCKADDR_IN*)name;
            memcpy(&addr_in->sin_addr, &WSA_BindIP4.sin_addr, sizeof(WSA_BindIP4.sin_addr));
            WSA_GetSock(s, TRUE)->Bound = TRUE;
        }
        else if (namelen >= sizeof(SOCKADDR_IN6_LH) && name && ((short*)name)[0] == AF_INET6) {

            SOCKADDR_IN6_LH* addr6_in = (SOCKADDR_IN6_LH*)name;
            memcpy(&addr6_in->sin6_addr, &WSA_BindIP6.sin6_addr, sizeof(WSA_BindIP6.sin6_addr));
            WSA_GetSock(s, TRUE)->Bound = TRUE;
        }
    }

    int ret = __sys_bind(s, name, namelen);

    if (new_name) Dll_Free((void*)name);

    return ret;
}


//---------------------------------------------------------------------------
// WSA_begin_connect
//---------------------------------------------------------------------------


_FX void WSA_bind_ip(
    SOCKET         s/*,
    const void     *name,
    int            namelen*/)
{
    WSA_SOCK* pSock = WSA_GetSock(s, TRUE);
    if (pSock->Bound)
        return;

    //if (namelen >= sizeof(SOCKADDR_IN) && name && ((short*)name)[0] == AF_INET) {
    if (pSock->af == AF_INET){

        __sys_bind(s, &WSA_BindIP4, sizeof(WSA_BindIP4));
        pSock->Bound = TRUE;
    }
    //else if (namelen >= sizeof(SOCKADDR_IN6_LH) && name && ((short*)name)[0] == AF_INET6) {
    else if (pSock->af == AF_INET6){

        __sys_bind(s, &WSA_BindIP6, sizeof(WSA_BindIP6));
        pSock->Bound = TRUE;
    }
}


//---------------------------------------------------------------------------
// WSA_GetIP
//---------------------------------------------------------------------------


_FX BOOLEAN WSA_GetIP(const short* addr, int addrlen, IP_ADDRESS* pIP)
{
    //pIP->Type = (BYTE)addr[0];
    if ((BYTE)addr[0] == AF_INET6 && addrlen >= sizeof(SOCKADDR_IN6_LH)) {
        memcpy(pIP->Data, ((SOCKADDR_IN6_LH*)addr)->sin6_addr.u.Byte, 16);
    }
    else  if ((BYTE)addr[0] == AF_INET && addrlen >= sizeof(SOCKADDR_IN)) {
        // IPv4-mapped IPv6 addresses, eg. ::FFFF:192.168.0.1
        pIP->Data32[0] = 0;
        pIP->Data32[1] = 0;
        pIP->Data32[2] = 0xFFFF0000;
        pIP->Data32[3] = ((SOCKADDR_IN*)addr)->sin_addr.S_un.S_addr;
    }
    else // something's wrong
        return FALSE;
    return TRUE;
}


//---------------------------------------------------------------------------
// WSA_DumpIP
//---------------------------------------------------------------------------


_FX void WSA_DumpIP(ADDRESS_FAMILY af, IP_ADDRESS* pIP, wchar_t* pStr)
{
    pStr = wcschr(pStr, L'\0');

    if (af == AF_INET6 && pIP->Data32[0] == 0 && pIP->Data32[1] == 0 && pIP->Data32[2] == 0xFFFF0000)
        af = AF_INET; // print mapped ipv4 addresses natively

    if (af == AF_INET6) {
		Sbie_snwprintf(pStr, 45+10, L"; IPv6: %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
			pIP->Data[0], pIP->Data[1], pIP->Data[2], pIP->Data[3], pIP->Data[4], pIP->Data[5], pIP->Data[6], pIP->Data[7],
			pIP->Data[8], pIP->Data[9], pIP->Data[10], pIP->Data[11], pIP->Data[12], pIP->Data[13], pIP->Data[14], pIP->Data[15]);
	}
	else if (af == AF_INET) {
		Sbie_snwprintf(pStr, 15+10, L"; IPv4: %d.%d.%d.%d",
            pIP->Data[12], pIP->Data[13], pIP->Data[14], pIP->Data[15]);
	}
    else
        Sbie_snwprintf(pStr, 5 + 10, L"; %d: ???", af);
}


//---------------------------------------------------------------------------
// WSA_IsBlockedTraffic
//---------------------------------------------------------------------------


_FX int WSA_IsBlockedTraffic(const short *addr, int addrlen, int protocol)
{

    if (WSA_FwList.count > 0 && addrlen >= sizeof(USHORT) * 2 && addr && (addr[0] == AF_INET || addr[0] == AF_INET6)) {

        USHORT port = _ntohs(addr[1]);

        IP_ADDRESS ip;
        if(!WSA_GetIP(addr, addrlen, &ip))
            return 1;  // lets block it

        BOOLEAN block = NetFw_BlockTraffic(&WSA_FwList, &ip, port, protocol);

        if (WSA_TraceFlag){
            WCHAR msg[256];
            Sbie_snwprintf(msg, 256, L"Network Traffic; Port: %u; Prot: %u", port, protocol);
            WSA_DumpIP(addr[0], &ip, msg);
            SbieApi_MonitorPutMsg(MONITOR_NETFW | (block ? MONITOR_DENY : MONITOR_OPEN), msg);
        }

        if (block) {

            SetLastError(WSAECONNREFUSED);
            return 1;
        }
    }

    return 0;
}


//---------------------------------------------------------------------------
// WSA_BypassProxy
//---------------------------------------------------------------------------

#define NETFW_MATCH_NONE	0

ULONG NetFw_MatchAddress(rbtree_t* ip_map, IP_ADDRESS* ip);

_FX BOOLEAN WSA_BypassProxyImpl(NETPROXY_RULE* WSA_Proxy, const short* addr, int addrlen)
{
    if (!WSA_Proxy)
        return FALSE;

    // check if there are any IPs specified for which we should bypass the proxy
    if (WSA_Proxy->ip_map.count > 0) {
        IP_ADDRESS ip;
        if (WSA_GetIP(addr, addrlen, &ip)) {
            if (NetFw_MatchAddress(&WSA_Proxy->ip_map, &ip) != NETFW_MATCH_NONE)
                return TRUE;
        }
    }

    return FALSE;
}

_FX BOOLEAN WSA_BypassProxy(const short *addr, int addrlen)
{
    const SOCKADDR* name = (SOCKADDR*)addr;
    if (name->sa_family == AF_INET) {
        if (WSA_BypassProxyImpl(WSA_Proxy4, addr, addrlen))
            return TRUE;
    } else if (name->sa_family == AF_INET6) {
        if (WSA_BypassProxyImpl(WSA_Proxy6, addr, addrlen))
            return TRUE;
    }
    return FALSE;
}


//---------------------------------------------------------------------------
// WSA_GetProxy
//---------------------------------------------------------------------------


_FX BOOLEAN WSA_GetProxy(const short *addr, int addrlen, void** proxy, int* proxylen, NETPROXY_RULE** pWSA_Proxy)
{
    //
    // First try finding an available proxy of the same IP type
    // if non is matching take ipv6 when prsent else ipv4
    //

    const SOCKADDR* name = (SOCKADDR*)addr;
    if (name->sa_family == AF_INET && WSA_Proxy4) {
        if (WSA_ProxyThread) {
            *pWSA_Proxy = NULL; // indicate thread mode

            USHORT port = start_socks5_relay(addr, &WSA_Proxy4->WSA_ProxyAddr, WSA_Proxy4->auth, WSA_Proxy4->login, WSA_Proxy4->pass);
            if (!port)
                return FALSE;

            *proxy = (void*)addr;
            *proxylen = addrlen;

            SOCKADDR_IN* v4 = *proxy;
            v4->sin_addr.S_un.S_addr = _ntohl(0x7F000001); // 127.0.0.1
            v4->sin_port = port;
        }
        else {
            *pWSA_Proxy = WSA_Proxy4;
            *proxy = &WSA_Proxy4->WSA_ProxyAddr;
            *proxylen = sizeof(SOCKADDR_IN);
        }
    }
    else if (name->sa_family == AF_INET6 && WSA_Proxy6) {
        if (WSA_ProxyThread) {
            *pWSA_Proxy = NULL; // indicate thread mode

            USHORT port = start_socks5_relay(addr, &WSA_Proxy6->WSA_ProxyAddr, WSA_Proxy6->auth, WSA_Proxy6->login, WSA_Proxy6->pass);
            if (!port)
                return FALSE;

            *proxy = (void*)addr;
            *proxylen = addrlen;

            SOCKADDR_IN6_LH* v6 = *proxy;
            v6->sin6_addr = (struct in6_addr){{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}}; // ::1
            v6->sin6_port = port;
        }
        else {
            *pWSA_Proxy = WSA_Proxy6;
            *proxy = &WSA_Proxy6->WSA_ProxyAddr6;
            *proxylen = sizeof(SOCKADDR_IN6_LH);
        }
    } 
    else 
        return FALSE; // no proxy found
    return TRUE;
}


//---------------------------------------------------------------------------
// WSA_begin_connect
//---------------------------------------------------------------------------


_FX void WSA_begin_connect(WSA_SOCK* pSock, SOCKET s)
{
    if (pSock->lEvent)
        __sys_WSAAsyncSelect(s, pSock->hWnd, 0, 0);

    if (pSock->lNetworkEvents)
        __sys_WSAEventSelect(s, NULL, 0);

    if (pSock->NonBlocking) {
        ULONG nb = FALSE;
        __sys_ioctlsocket(s, FIONBIO, &nb);
    }
}


//---------------------------------------------------------------------------
// WSA_end_connect
//---------------------------------------------------------------------------


_FX int WSA_end_connect(WSA_SOCK* pSock, SOCKET s, int ret)
{
    int err = __sys_WSAGetLastError();

    if (pSock->NonBlocking) {
        ULONG nb = pSock->NonBlocking;
        __sys_ioctlsocket(s, FIONBIO, &nb);
    }

    if ((pSock->lNetworkEvents & FD_CONNECT) != 0) {
        pSock->lNetworkEvents &= ~FD_CONNECT;

        SetEvent(pSock->hEventObject);

        ret = -1;
        err = WSAEWOULDBLOCK;
    }
    if(pSock->lNetworkEvents)
        __sys_WSAEventSelect(s, pSock->hEventObject, pSock->lNetworkEvents);

    if ((pSock->lEvent & FD_CONNECT) != 0) {
        pSock->lEvent &= ~FD_CONNECT;
        
        static P_PostMessage __sys_PostMessageW = NULL;
        if(__sys_PostMessageW == NULL)
            __sys_PostMessageW = Ldr_GetProcAddrNew(DllName_user32, L"PostMessageW","PostMessageW");
        __sys_PostMessageW(pSock->hWnd, pSock->wMsg, s, FD_CONNECT);

        ret = -1;
        err = WSAEWOULDBLOCK;
    }   
    if (pSock->lEvent)
        __sys_WSAAsyncSelect(s, pSock->hWnd, pSock->wMsg, pSock->lEvent);

    pSock->ConnectedSet = TRUE;

    __sys_WSASetLastError(err);
    return ret;
}


//---------------------------------------------------------------------------
// WSA_connect
//---------------------------------------------------------------------------


_FX int WSA_connect(
    SOCKET         s,
    const void     *name,
    int            namelen)
{
    if (WSA_IsBlockedTraffic(name, namelen, IPPROTO_TCP))
        return SOCKET_ERROR;

    void* proxy;
    int proxylen;
    NETPROXY_RULE* WSA_Proxy;

    if (WSA_BindIP) {

        WSA_bind_ip(s/*, name, namelen*/);
    }

    if (WSA_ProxyEnabled && !is_localhost(name) && !WSA_BypassProxy(name, namelen)) {

        int ret = SOCKET_ERROR;

        if (WSA_GetProxy(name, namelen, &proxy, &proxylen, &WSA_Proxy)) {

            WSA_SOCK* pSock = WSA_GetSock(s, FALSE);
            if (WSA_ProxyHack && pSock) WSA_begin_connect(pSock, s);

            ret = __sys_connect(s, proxy, proxylen);
            if (WSA_Proxy && ret == SOCKS_SUCCESS) {

                if (!socks5_handshake(s, WSA_Proxy->auth, WSA_Proxy->login, WSA_Proxy->pass) || socks5_request(s, name) != SOCKS_SUCCESS)
                    ret = SOCKET_ERROR;

                if (ret != SOCKS_SUCCESS)
                    __sys_shutdown(s, SD_BOTH);
            }

            if (WSA_ProxyHack && pSock) ret = WSA_end_connect(pSock, s, ret);
        }
        else
            __sys_WSASetLastError(WSAECONNREFUSED);

        return ret;
    }

    BOOLEAN new_name = WSA_HandleAfUnix(&name, &namelen);

    int ret = __sys_connect(s, name, namelen);

    if (new_name) Dll_Free((void*)name);

    return ret;
}


//---------------------------------------------------------------------------
// WSA_WSAConnect
//---------------------------------------------------------------------------


_FX int WSA_WSAConnect(
    SOCKET         s,
    const void     *name,
    int            namelen,
    LPWSABUF       lpCallerData,
    LPWSABUF       lpCalleeData,
    LPQOS          lpSQOS,
    LPQOS          lpGQOS)
{
    if (WSA_IsBlockedTraffic(name, namelen, IPPROTO_TCP))
        return SOCKET_ERROR;

    void* proxy;
    int proxylen;
    NETPROXY_RULE* WSA_Proxy;

    if (WSA_BindIP) {

        WSA_bind_ip(s/*, name, namelen*/);
    }

    if (WSA_ProxyEnabled && !is_localhost(name) && !WSA_BypassProxy(name, namelen)) {

        int ret = SOCKET_ERROR;

        if (WSA_GetProxy(name, namelen, &proxy, &proxylen, &WSA_Proxy)) {

            WSA_SOCK* pSock = WSA_GetSock(s, FALSE);
            if (WSA_ProxyHack && pSock) WSA_begin_connect(pSock, s);

            ret = __sys_WSAConnect(s, proxy, proxylen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS);
            if (WSA_Proxy && ret == SOCKS_SUCCESS) {

                if (!socks5_handshake(s, WSA_Proxy->auth, WSA_Proxy->login, WSA_Proxy->pass) || socks5_request(s, name) != SOCKS_SUCCESS)
                    ret = SOCKET_ERROR;

                if (ret != SOCKS_SUCCESS)
                    __sys_shutdown(s, SD_BOTH);
            }

            if (WSA_ProxyHack && pSock) ret = WSA_end_connect(pSock, s, ret);
        }
        else
            __sys_WSASetLastError(WSAECONNREFUSED);

        return ret;
    }

    BOOLEAN new_name = WSA_HandleAfUnix(&name, &namelen);

    int ret = __sys_WSAConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS);

    if (new_name) Dll_Free((void*)name);

    return ret;
}


//---------------------------------------------------------------------------
// WSA_ConnectEx
//---------------------------------------------------------------------------


_FX int WSA_ConnectEx(  
    SOCKET s,
    const void *name,
    int namelen,
    PVOID lpSendBuffer,
    DWORD dwSendDataLength,
    LPDWORD lpdwBytesSent,
    LPOVERLAPPED lpOverlapped)
{
    if (WSA_IsBlockedTraffic(name, namelen, IPPROTO_TCP))
        return SOCKET_ERROR;

    void* proxy;
    int proxylen;
    NETPROXY_RULE* WSA_Proxy;

    if (WSA_BindIP) {

        WSA_bind_ip(s/*, name, namelen*/);
    }

    if (WSA_ProxyEnabled && !is_localhost(name) && !WSA_BypassProxy(name, namelen)) {

        int ret = SOCKET_ERROR;

        if (WSA_GetProxy(name, namelen, &proxy, &proxylen, &WSA_Proxy)) {

            WSA_SOCK* pSock = WSA_GetSock(s, FALSE);
            if (WSA_ProxyHack && pSock) WSA_begin_connect(pSock, s);

            if (WSA_Proxy) {

                ret = __sys_connect(s, proxy, proxylen);
                if (ret == SOCKS_SUCCESS) {

                    if (!socks5_handshake(s, WSA_Proxy->auth, WSA_Proxy->login, WSA_Proxy->pass) || socks5_request(s, name) != SOCKS_SUCCESS)
                        ret = SOCKET_ERROR;

                    if (ret == 0 && lpSendBuffer) {
                        ret = __sys_send(s, (const char*)lpSendBuffer, dwSendDataLength, 0);
                        if (ret != SOCKET_ERROR) {
                            *lpdwBytesSent = ret;
                            ret = SOCKS_SUCCESS;
                        }
                    }

                    if (ret != SOCKS_SUCCESS)
                        __sys_shutdown(s, SD_BOTH);
                }
            } 
            else {

                if (__sys_ConnectEx(s, proxy, proxylen, lpSendBuffer, dwSendDataLength, lpdwBytesSent, lpOverlapped))
                    ret = SOCKS_SUCCESS;
            }

            if (WSA_ProxyHack && pSock) ret = WSA_end_connect(pSock, s, ret);
        }
        else
            __sys_WSASetLastError(WSAECONNREFUSED);

        return ret == SOCKS_SUCCESS;
    }

    BOOLEAN new_name = WSA_HandleAfUnix(&name, &namelen);

    int ret = __sys_ConnectEx(s, name, namelen, lpSendBuffer, dwSendDataLength, lpdwBytesSent, lpOverlapped);

    if (new_name) Dll_Free((void*)name);

    return ret;
}

/*
//---------------------------------------------------------------------------
// WSA_accept
//---------------------------------------------------------------------------

static int WSA_listen(
    SOCKET          s,
    int             backlog)
{

    return  __sys_listen(s, backlog);
}


//---------------------------------------------------------------------------
// WSA_accept
//---------------------------------------------------------------------------


_FX SOCKET WSA_accept(
    SOCKET   s,
    void     *addr,
    int      *addrlen)
{
    if (WSA_IsBlockedTraffic(addr, addrlen, IPPROTO_TCP)) {
        __sys_closesocket(s);
        return SOCKET_ERROR;
    }
    return __sys_accept(s, addr, addrlen);
}


//---------------------------------------------------------------------------
// WSA_WSAAccept
//---------------------------------------------------------------------------


_FX SOCKET WSA_WSAAccept(
    SOCKET          s,
    void            *addr,
    LPINT           addrlen,
    LPCONDITIONPROC lpfnCondition,
    DWORD_PTR       dwCallbackData)
{
    if (WSA_IsBlockedTraffic(addr, addrlen, IPPROTO_TCP)) {
        __sys_closesocket(s);
        return SOCKET_ERROR;
    }
    return __sys_WSAAccept(s, addr, addrlen, lpfnCondition, dwCallbackData);
}


//---------------------------------------------------------------------------
// WSA_WSAAccept
//---------------------------------------------------------------------------


_FX int WSA_AcceptEx(
    SOCKET       sListenSocket,
    SOCKET       sAcceptSocket,
    PVOID        lpOutputBuffer,
    DWORD        dwReceiveDataLength,
    DWORD        dwLocalAddressLength,
    DWORD        dwRemoteAddressLength,
    LPDWORD      lpdwBytesReceived,
    LPOVERLAPPED lpOverlapped)
{
    //
    // this call can operate asynchronously, hence we can not simply filter here the incoming connection
    // as we have a proper WFP filter in the driver for now this usermode filtering implementation 
    // will not filter incoming traffic at all, normally users are behind NATs or other firewall so 
    // blocking only outgoing connections should be good enough
    //

    //if (WSA_IsBlockedTraffic(addr, addrlen, IPPROTO_TCP)) {
    //    __sys_closesocket(sAcceptSocket);
    //    return SOCKET_ERROR;
    //}
    return __sys_AcceptEx(sListenSocket, sAcceptSocket, lpOutputBuffer, dwReceiveDataLength,
        dwLocalAddressLength, dwRemoteAddressLength, lpdwBytesReceived, lpOverlapped);
}
*/

//---------------------------------------------------------------------------
// WSA_sendto
//---------------------------------------------------------------------------


_FX int WSA_sendto(
    SOCKET         s,
    const char     *buf,
    int            len,
    int            flags,
    const void     *to,
    int            tolen)
{
    if (WSA_IsBlockedTraffic(to, tolen, IPPROTO_UDP))
        return SOCKET_ERROR;

    if (WSA_BindIP) {

        WSA_bind_ip(s/*, to, tolen*/);
    }

    return __sys_sendto(s, buf, len, flags, to, tolen);
}


//---------------------------------------------------------------------------
// WSA_WSASendTo
//---------------------------------------------------------------------------


_FX int WSA_WSASendTo(
    SOCKET                             s,
    LPWSABUF                           lpBuffers,
    DWORD                              dwBufferCount,
    LPDWORD                            lpNumberOfBytesSent,
    DWORD                              dwFlags,
    const void                         *lpTo,
    int                                iTolen,
    LPWSAOVERLAPPED                    lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
    if (WSA_IsBlockedTraffic(lpTo, iTolen, IPPROTO_UDP))
        return SOCKET_ERROR;

    if (WSA_BindIP) {

        WSA_bind_ip(s/*, lpTo, iTolen*/);
    }

    return __sys_WSASendTo(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent,
        dwFlags, lpTo, iTolen, lpOverlapped, lpCompletionRoutine);
}


//---------------------------------------------------------------------------
// WSA_recvfrom
//---------------------------------------------------------------------------


_FX int WSA_recvfrom(
    SOCKET   s,
    char     *buf,
    int      len,
    int      flags,
    void     *from,
    int      *fromlen)
{
    if (WSA_BindIP) {

        WSA_bind_ip(s);
    }

    char buffer[128];
    int buffer_len = sizeof(buffer);
    if (!from) {
        from = buffer;
        fromlen = &buffer_len;
    }

    int ret = __sys_recvfrom(s, buf, len, flags, from, fromlen);

    if (WSA_IsBlockedTraffic(from, *fromlen, IPPROTO_UDP))
        return SOCKET_ERROR;

    return ret;
}


//---------------------------------------------------------------------------
// WSA_WSARecvFrom
//---------------------------------------------------------------------------


_FX int WSA_WSARecvFrom(
    SOCKET                             s,
    LPWSABUF                           lpBuffers,
    DWORD                              dwBufferCount,
    LPDWORD                            lpNumberOfBytesRecvd,
    LPDWORD                            lpFlags,
    void                               *lpFrom,
    LPINT                              lpFromlen,
    LPWSAOVERLAPPED                    lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
    if (WSA_BindIP) {

        WSA_bind_ip(s);
    }

    char buffer[128];
    int buffer_len = sizeof(buffer);
    if (!lpFrom) {
        lpFrom = buffer;
        lpFromlen = &buffer_len;
    }

    int ret = __sys_WSARecvFrom(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd,
        lpFlags, lpFrom, lpFromlen, lpOverlapped, lpCompletionRoutine);

    if (WSA_IsBlockedTraffic(lpFrom, *lpFromlen, IPPROTO_UDP))
        return SOCKET_ERROR;

    return ret;
}


//---------------------------------------------------------------------------
// WSA_closesocket
//---------------------------------------------------------------------------


_FX int WSA_closesocket(SOCKET s)
{
    if (WSA_ProxyHack || WSA_BindIP)
        map_remove(&WSA_SockMap, (void*)s);
    return __sys_closesocket(s);
}


//---------------------------------------------------------------------------
// WSA_InitNetFwRules
//---------------------------------------------------------------------------


_FX void WSA_InitNetFwRules()
{
    WCHAR conf_buf[2048];

    for (ULONG index = 0; ; ++index) {

        NTSTATUS status = SbieApi_QueryConf(
            NULL, L"NetworkAccess", index, conf_buf, sizeof(conf_buf) - 16 * sizeof(WCHAR));
        if (!NT_SUCCESS(status))
            break;

        ULONG level = -1;
        WCHAR* value = Config_MatchImageAndGetValue(conf_buf, Dll_ImageName, &level);
        if (!value)
            continue;

        NETFW_RULE* rule = NetFw_AllocRule(Dll_Pool, level);
        if (!rule) {
            SbieApi_Log(2305, NULL);
            continue;
        }

		NetFw_ParseRule(rule, value);

        NetFw_AddRule(&WSA_FwList, rule);
    }
}


//---------------------------------------------------------------------------
// WSA_ParseNetProxy
//---------------------------------------------------------------------------

void NetFw_RuleAddIpRange(rbtree_t* tree, IP_ADDRESS* IpBegin, IP_ADDRESS* IpEnd, POOL* pool);

const WCHAR* wcsnchr(const WCHAR* str, size_t max, WCHAR ch);

BOOLEAN WSA_ParseNetProxy(NETPROXY_RULE* proxy, const WCHAR* found_value)
{
    // NetworkUseProxy=explorer.exe,Address=198.98.55.77;Port=40000;Auth=No;Login=l2sxbnjqR5JJAAoCnA;Password=12OxyLTW9nma5HbNjC
    // NetworkUseProxy=*,Address=abcd:0123:ffff:1d5c;Port=24;Auth=No

    WCHAR* addr_value;
    ULONG addr_len;
    if (SbieDll_FindTagValuePtr(found_value, L"Address", &addr_value, &addr_len, L'=', L';')) {
        addr_value[addr_len] = L'\0';
        if (_inet_aton(addr_value, &proxy->WSA_ProxyAddr.sin_addr) == 1) {
            proxy->WSA_ProxyAddr.sin_family = AF_INET;
        }
        else if (_inet_pton(AF_INET6, addr_value, &proxy->WSA_ProxyAddr6.sin6_addr) == 1) {
            proxy->WSA_ProxyAddr6.sin6_family = AF_INET6;
        } 
        else { // host name?
        
            ADDRINFOW hints = { 0 };
            hints.ai_family = AF_UNSPEC;       // Allow IPv4 or IPv6
            hints.ai_socktype = SOCK_STREAM;   // TCP
            hints.ai_protocol = IPPROTO_TCP;

            ADDRINFOW* res = NULL;
            int ret = __sys_GetAddrInfoW(addr_value, NULL, &hints, &res);
            if (ret == SOCKS_SUCCESS) {

                for (ADDRINFOW* ptr = res; ptr != NULL; ptr = ptr->ai_next) {
                    if (ptr->ai_family == AF_INET && proxy->WSA_ProxyAddr.sin_family == 0) {
                        memcpy(&proxy->WSA_ProxyAddr, ptr->ai_addr, sizeof(SOCKADDR_IN));
                    }
                    else if (ptr->ai_family == AF_INET6 && proxy->WSA_ProxyAddr6.sin6_family == 0) {
                        memcpy(&proxy->WSA_ProxyAddr6, ptr->ai_addr, sizeof(SOCKADDR_IN6_LH));
                    }
                }

                __sys_FreeAddrInfoW(res);
            }

            if(proxy->WSA_ProxyAddr.sin_family != AF_INET && proxy->WSA_ProxyAddr6.sin6_family != AF_INET6)
                return FALSE; // no ip found
        }
        addr_value[addr_len] = L';';
    } 
    else
        return FALSE;

    WCHAR* port_value;
    ULONG port_len;
    if (SbieDll_FindTagValuePtr(found_value, L"Port", &port_value, &port_len, L'=', L';')) {
        int temp = _wntoi(port_value, port_len);
        if (temp < 0 || temp > 0xFFFF)
            return FALSE;

        USHORT port = _ntohs((USHORT)temp);
        proxy->WSA_ProxyAddr6.sin6_port = port;
        proxy->WSA_ProxyAddr.sin_port = port;
    } 
    else
        return FALSE;

    const WCHAR* ip_value;
    ULONG ip_len;
    if (SbieDll_FindTagValuePtr(found_value, L"Bypass", &ip_value, &ip_len, L'=', L';')) {

        for (const WCHAR* ip_end = ip_value + ip_len; ip_value < ip_end;) {
            const WCHAR* ip_str1;
            ULONG ip_len1;
            ip_value = SbieDll_GetTagValue(ip_value, ip_end, &ip_str1, &ip_len1, L',');

            const WCHAR* ip_str2 = wcsnchr(ip_str1, ip_len1, L'-');
            if (ip_str2) {
                ip_len1 = (ULONG)(ip_str2 - ip_str1);
                ip_str2++; // skip dash
                ULONG ip_len2 = (ULONG)(ip_value - ip_str2);

                IP_ADDRESS ip1;
                _inet_xton(ip_str1, ip_len1, &ip1, NULL);
                IP_ADDRESS ip2;
                _inet_xton(ip_str2, ip_len2, &ip2, NULL);

                NetFw_RuleAddIpRange(&proxy->ip_map, &ip1, &ip2, Dll_Pool);
            }
            else
            {
                IP_ADDRESS ip;
                _inet_xton(ip_str1, ip_len1, &ip, NULL);
                NetFw_RuleAddIpRange(&proxy->ip_map, &ip, &ip, Dll_Pool);
            }
        }
    }

    WCHAR* auth_value;
    ULONG auth_len;
    if (SbieDll_FindTagValuePtr(found_value, L"Auth", &auth_value, &auth_len, L'=', L';'))
        proxy->auth = _wcsnicmp(auth_value, L"Yes", auth_len) == 0;

    if (!proxy->auth) return TRUE;

    WCHAR* login_value;
    ULONG login_len;
    if (SbieDll_FindTagValuePtr(found_value, L"Login", &login_value, &login_len, L'=', L';')) {
        if (login_len > 255)
            return FALSE;
        wmemcpy(proxy->login, login_value, login_len);
    }

    WCHAR* pass_value;
    ULONG pass_len;
    BOOLEAN ok = SbieDll_FindTagValuePtr(found_value, L"Password", &pass_value, &pass_len, L'=', L';');
    if (ok) {
        if (pass_len > 255)
            return FALSE;
        wmemcpy(proxy->pass, pass_value, pass_len);
        proxy->pass[pass_len] = L'\0';
    }
    else {
        ok = SbieDll_FindTagValuePtr(found_value, L"EncryptedPW", &pass_value, &pass_len, L'=', L';');
        if (ok) {

            SBIE_INI_RC4_CRYPT_REQ req;
            SBIE_INI_RC4_CRYPT_RPL *rpl;

            pass_value[pass_len] = L'\0';

            req.h.length = sizeof(SBIE_INI_RC4_CRYPT_REQ) + 255;
            req.h.msgid = MSGID_SBIE_INI_RC4_CRYPT;
            req.value_len = b64_decoded_size(pass_value);
            b64_decode(pass_value, req.value, req.value_len);

            pass_value[pass_len] = L'\0';

            rpl = (SBIE_INI_RC4_CRYPT_RPL *)SbieDll_CallServer(&req.h);
            if (rpl){

                pass_len = rpl->value_len / sizeof(wchar_t);
                if (pass_len > 255)
                    return FALSE;
                wmemcpy(proxy->pass, rpl->value, pass_len);
                proxy->pass[pass_len] = L'\0';

                Dll_Free(rpl);
            }
        }
    }

    return TRUE;
}

//---------------------------------------------------------------------------
// WSA_InitNetProxy
//---------------------------------------------------------------------------


#define NETPROXY_MATCH_GLOBAL   2
#define NETPROXY_MATCH_EXACT    0

int NetFw_IpCmp(const void* l, const void* r);

_FX BOOLEAN WSA_InitNetProxy()
{
    WCHAR proxy_buf[2048];
    
    ULONG FoundLevel4 = -1;
    ULONG FoundLevel6 = -1;

    for (ULONG index = 0; ; ++index) {
        NTSTATUS status = SbieApi_QueryConf(
            NULL, L"NetworkUseProxy", index, proxy_buf, sizeof(proxy_buf) - 16 * sizeof(WCHAR));
        if (!NT_SUCCESS(status))
            break;

        ULONG level = -1;
        WCHAR* value = Config_MatchImageAndGetValue(proxy_buf, Dll_ImageName, &level);
        if (!value || (level != NETPROXY_MATCH_EXACT && level != NETPROXY_MATCH_GLOBAL))
            continue;

        NETPROXY_RULE* WSA_Proxy = Dll_Alloc(sizeof(NETPROXY_RULE));
        if (!WSA_Proxy) break;
        memset(WSA_Proxy, 0, sizeof(NETPROXY_RULE));
        rbtree_init(&WSA_Proxy->ip_map, NetFw_IpCmp);

        if (WSA_ParseNetProxy(WSA_Proxy, value)) {

            if (WSA_Proxy->WSA_ProxyAddr.sin_family == AF_INET) {
                if (FoundLevel4 > level) {
                    if(WSA_Proxy4 && --WSA_Proxy4->used == 0)
                        Dll_Free(WSA_Proxy4);
                    WSA_Proxy4 = WSA_Proxy;
                    WSA_Proxy4->used++;
                    FoundLevel4 = level;
                }
            }

            if (WSA_Proxy->WSA_ProxyAddr6.sin6_family == AF_INET6) {
                if (FoundLevel6 > level) {
                    if(WSA_Proxy6 && --WSA_Proxy6->used == 0)
                        Dll_Free(WSA_Proxy6);
                    WSA_Proxy6 = WSA_Proxy;
                    WSA_Proxy6->used++;
                    FoundLevel6 = level;
                }
            }
        }

        if(WSA_Proxy->used == 0)
            Dll_Free(WSA_Proxy);
    }

    if (FoundLevel4 == -1 && FoundLevel6 == -1)
        return FALSE;
    //
    // even if no proxies were set up due to config error, if any were configured 
    // enable proxy and fail connections to prevent accidental ip leakage
    //

    SCertInfo CertInfo = { 0 };
    if (!NT_SUCCESS(SbieApi_QueryDrvInfo(-1, &CertInfo, sizeof(CertInfo))) || !(CertInfo.active && CertInfo.opt_net)) {

        const WCHAR* strings[] = { L"NetworkUseProxy" , NULL };
        SbieApi_LogMsgExt(-1, 6009, strings);

        WSA_Proxy4 = NULL;
        WSA_Proxy6 = NULL;
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// WSA_InitBindIP
//---------------------------------------------------------------------------


_FX BOOLEAN WSA_InitBindIP()
{
    WCHAR conf_buf[2048];

    IP_ADAPTER_ADDRESSES* adapters = NULL;

    ULONG FoundLevel = -1;
    for (ULONG index = 0; ; ++index) {

        NTSTATUS status = SbieApi_QueryConf(
            NULL, L"BindAdapter", index, conf_buf, sizeof(conf_buf) - 16 * sizeof(WCHAR));
        if (!NT_SUCCESS(status))
            break;

        ULONG level = -1;
        WCHAR* value = Config_MatchImageAndGetValue(conf_buf, Dll_ImageName, &level);
        if (!value)
            continue;

        if (FoundLevel < level)
            continue;

        if (!adapters) {
            HMODULE Iphlpapi = LoadLibraryW(L"Iphlpapi.dll");
            if (Iphlpapi) {
                P_GetAdaptersAddresses GetAdaptersAddresses = (P_GetAdaptersAddresses)GetProcAddress(Iphlpapi, "GetAdaptersAddresses");
                if (GetAdaptersAddresses) {
                    ULONG bufferSize = 0;
                    GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, NULL, &bufferSize);
                    adapters = Dll_Alloc(bufferSize * 10 / 8);
                    if (adapters) {
                        if (GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, adapters, &bufferSize) != NO_ERROR) {
                            Dll_Free(adapters);
                            adapters = NULL;
                        }
                    }
                }
            }
            FreeLibrary(Iphlpapi);
            if (!adapters) break;
        }

        for (IP_ADAPTER_ADDRESSES* adapter = adapters; adapter != NULL; adapter = adapter->Next) {

            if (_wcsicmp(adapter->FriendlyName, value) != 0)
                continue;

            for (IP_ADAPTER_UNICAST_ADDRESS* unicast = adapter->FirstUnicastAddress; unicast != NULL; unicast = unicast->Next) {

                if (unicast->Address.lpSockaddr->sa_family == AF_INET) {
                    memcpy(&WSA_BindIP4, unicast->Address.lpSockaddr, sizeof(SOCKADDR_IN));
                }
                else if (unicast->Address.lpSockaddr->sa_family == AF_INET6) {
                    memcpy(&WSA_BindIP6, unicast->Address.lpSockaddr, sizeof(SOCKADDR_IN6_LH));
                }

            }

            FoundLevel = level;
        }
    }

    if (adapters)
        Dll_Free(adapters);
    
    if (FoundLevel != -1)
        return TRUE;

    ULONG FoundLevel4 = -1;
    ULONG FoundLevel6 = -1;
    for (ULONG index = 0; ; ++index) {

        NTSTATUS status = SbieApi_QueryConf(
            NULL, L"BindAdapterIP", index, conf_buf, sizeof(conf_buf) - 16 * sizeof(WCHAR));
        if (!NT_SUCCESS(status))
            break;

        ULONG level = -1;
        WCHAR* value = Config_MatchImageAndGetValue(conf_buf, Dll_ImageName, &level);
        if (!value)
            continue;

        if (_inet_aton(value, &WSA_BindIP4.sin_addr) == 1) {
            if (FoundLevel4 > level) {
                WSA_BindIP4.sin_family = AF_INET;
                FoundLevel4 = level;
            }
        }
        else if(_inet_pton(AF_INET6, value, &WSA_BindIP6.sin6_addr) == 1){
            if (FoundLevel6 > level) {
                WSA_BindIP6.sin6_family = AF_INET6;
                FoundLevel6 = level;
            }
        }
    }

    return FoundLevel4 != -1 || FoundLevel6 != -1;
}


//---------------------------------------------------------------------------
// WSA_Init
//---------------------------------------------------------------------------


_FX BOOLEAN WSA_Init(HMODULE module)
{
    P_WSAStartup        WSAStartup;
    P_WSACleanup        WSACleanup;

    P_WSAIoctl          WSAIoctl;

    P_ioctlsocket       ioctlsocket;
    P_WSAAsyncSelect    WSAAsyncSelect;
    P_WSAEventSelect    WSAEventSelect;

    P_WSAEnumNetworkEvents WSAEnumNetworkEvents;

    P_WSANSPIoctl       WSANSPIoctl;

    P_WSASocketW        WSASocketW;

    P_bind              bind;

    P_connect           connect;
    P_WSAConnect        WSAConnect;

    /*P_listen            listen;

    P_accept            accept;
    P_WSAAccept         WSAAccept;*/

    P_sendto            sendto;
    P_WSASendTo         WSASendTo;
    P_recvfrom          recvfrom;
    P_WSARecvFrom       WSARecvFrom;
    P_closesocket       closesocket;

#ifdef PROXY_RESOLVE_HOST_NAMES
    P_GetAddrInfoW      GetAddrInfoW;
#endif

    //
    // initialize the network firewall rule list and hook the relevant functions
    //

    List_Init(&WSA_FwList);

    WSA_WFPisEnabled = (Dll_DriverFlags & SBIE_FEATURE_FLAG_WFP) != 0;
    if(WSA_WFPisEnabled)
        WSA_WFPisBlocking = !Config_GetSettingsForImageName_bool(L"AllowNetworkAccess", TRUE);
    else // load rules only when the driver is not doing the filtering
        WSA_InitNetFwRules();

    //
    // hook required WS2 functions
    //

    WSAStartup = (P_WSAStartup)GetProcAddress(module, "WSAStartup"); 
    if (WSAStartup) {
        SBIEDLL_HOOK(WSA_, WSAStartup);
    }

    WSACleanup = (P_WSACleanup)GetProcAddress(module, "WSACleanup"); 
    if (WSACleanup) {
        SBIEDLL_HOOK(WSA_, WSACleanup);
    }

    WSAIoctl = (P_WSAIoctl)GetProcAddress(module, "WSAIoctl");
    if (WSAIoctl) {
        SBIEDLL_HOOK(WSA_,WSAIoctl);
    }

    __sys_WSAGetLastError = (P_WSAGetLastError)GetProcAddress(module, "WSAGetLastError");
    __sys_WSASetLastError = (P_WSASetLastError)GetProcAddress(module, "WSASetLastError");

    //
    // Note: for our proxy implementation we need to have the sockets in blocking mode
    // unfortunately windows does not provide a way to query the blocking flag
    // not even when asking the driver directly :(
    // Hence we need to monitor the below calls and maintain and cache the blocking state
    //
  
    //int InputBuffer[] = { 2,0,0,0 };
    //((char*)&InputBuffer[2])[0] = 1;
    //Status = NtDeviceIoControlFile((HANDLE)sock, NULL, 0i64, 0i64, &IoStatusBlock, /*IOCTL_AFD_SET_INFORMATION*/ 0x1203B, InputBuffer, 0x10u, 0i64, 0);               // Set blocking -> STATUS_SUCCESS
    //Status = NtDeviceIoControlFile((HANDLE)sock, NULL, 0i64, 0i64, &IoStatusBlock, /*IOCTL_AFD_SET_INFORMATION*/ 0x1207b, InputBuffer, 0x10u, InputBuffer, 0x10u);    // Get blocking -> STATUS_INVALID_PARAMETER :(

    ioctlsocket = (P_ioctlsocket)GetProcAddress(module, "ioctlsocket");
    if (ioctlsocket) {
        SBIEDLL_HOOK(WSA_, ioctlsocket);
    }

    WSAAsyncSelect = (P_WSAAsyncSelect)GetProcAddress(module, "WSAAsyncSelect");
    if (WSAAsyncSelect) {
        SBIEDLL_HOOK(WSA_, WSAAsyncSelect);
    }

    WSAEventSelect = (P_WSAEventSelect)GetProcAddress(module, "WSAEventSelect");
    if (WSAEventSelect) {
        SBIEDLL_HOOK(WSA_, WSAEventSelect);
    }

    WSAEnumNetworkEvents = (P_WSAEnumNetworkEvents)GetProcAddress(module, "WSAEnumNetworkEvents");
    if (WSAEnumNetworkEvents) {
        SBIEDLL_HOOK(WSA_, WSAEnumNetworkEvents);
    }

    // +++

    __sys_recv = (P_recv)GetProcAddress(module, "recv");

    __sys_send = (P_send)GetProcAddress(module, "send");

    if (!Dll_CompartmentMode) {
        WSANSPIoctl = (P_WSANSPIoctl)GetProcAddress(module, "WSANSPIoctl");
        if (WSANSPIoctl) {
            SBIEDLL_HOOK(WSA_, WSANSPIoctl);
        }
    }

    bind = (P_bind)GetProcAddress(module, "bind");
    if (bind) {
        SBIEDLL_HOOK(WSA_,bind);
    }
    
    __sys_socket = (P_socket)GetProcAddress(module, "socket"); // uses WSASocketW
    
    WSASocketW = (P_WSASocketW)GetProcAddress(module, "WSASocketW");
    if (WSASocketW) {
        SBIEDLL_HOOK(WSA_,WSASocketW);
    }

    __sys_getsockname = (P_getsockname)GetProcAddress(module, "getsockname");

    __sys_WSAFDIsSet = (P_WSAFDIsSet )GetProcAddress(module, "__WSAFDIsSet");

    __sys_select = (P_select)GetProcAddress(module, "select");

    // TCP
    //if (! Dll_SkipHook(L"wsaconn")) {
   
    connect = (P_connect)GetProcAddress(module, "connect");
    if (connect) {
        SBIEDLL_HOOK(WSA_,connect);
    }

    WSAConnect = (P_WSAConnect)GetProcAddress(module, "WSAConnect");
    if (WSAConnect) {
        SBIEDLL_HOOK(WSA_,WSAConnect);
    }

    //}

    __sys_listen = (P_listen)GetProcAddress(module, "listen");

    __sys_accept = (P_accept)GetProcAddress(module, "accept");

    /*listen = (P_listen)GetProcAddress(module, "listen");
    if (listen) {
        SBIEDLL_HOOK(WSA_,listen);
    }

    accept = (P_accept)GetProcAddress(module, "accept");
    if (accept) {
        SBIEDLL_HOOK(WSA_,accept);
    }

    WSAAccept = (P_WSAAccept)GetProcAddress(module, "WSAAccept");
    if (WSAAccept) {
        SBIEDLL_HOOK(WSA_,WSAAccept);
    }*/
    //

    // UDP
    sendto = (P_sendto)GetProcAddress(module, "sendto");
    if (sendto) {
        SBIEDLL_HOOK(WSA_,sendto);
    }

    WSASendTo = (P_WSASendTo)GetProcAddress(module, "WSASendTo");
    if (WSASendTo) {
        SBIEDLL_HOOK(WSA_,WSASendTo);
    }

    recvfrom = (P_recvfrom)GetProcAddress(module, "recvfrom");
    if (recvfrom) {
        SBIEDLL_HOOK(WSA_,recvfrom);
    }

    WSARecvFrom = (P_WSARecvFrom)GetProcAddress(module, "WSARecvFrom");
    if (WSARecvFrom) {
        SBIEDLL_HOOK(WSA_,WSARecvFrom);
    }
    //
        
    __sys_shutdown = (P_shutdown)GetProcAddress(module, "shutdown");

    __sys_inet_ntop = (P_inet_ntop)GetProcAddress(module, "inet_ntop");

    __sys_GetAddrInfoW = (P_GetAddrInfoW)GetProcAddress(module, "GetAddrInfoW"); 

    __sys_FreeAddrInfoW = (P_FreeAddrInfoW)GetProcAddress(module, "FreeAddrInfoW");

#ifdef PROXY_RESOLVE_HOST_NAMES
    if(WSA_ProxyEnabled && SbieApi_QueryConfBool(NULL, L"NetworkProxyResolveHostnames", FALSE)) {
	
        map_init(&DNS_LookupMap, Dll_Pool);

        GetAddrInfoW = (P_GetAddrInfoW)GetProcAddress(module, "GetAddrInfoW");
        if (GetAddrInfoW) {
            SBIEDLL_HOOK(WSA_,GetAddrInfoW);
        }
    }
#endif

    closesocket = (P_closesocket)GetProcAddress(module, "closesocket");
    if (closesocket) {
        SBIEDLL_HOOK(WSA_,closesocket);
    }

    // If there are any NetFwTrace options set, then output this debug string
    WCHAR wsTraceOptions[4];
    if (SbieApi_QueryConf(NULL, L"NetFwTrace", 0, wsTraceOptions, sizeof(wsTraceOptions)) == STATUS_SUCCESS && wsTraceOptions[0] != L'\0')
        WSA_TraceFlag = TRUE;
    
    //
    // Initialize DNS filter
    //

    WSA_InitNetDnsFilter(module);

    return TRUE;
}



//---------------------------------------------------------------------------
//
// Windows Firewall Integration
//
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


//static ULONG_PTR Net_INetFwAuthorizedApplication_orig_ImageNameGet = 0;
static ULONG_PTR Net_INetFwAuthorizedApplication_orig_ImageNamePut = 0;

//static ULONG_PTR Net_INetFwRule_orig_ImageNameGet = 0;
static ULONG_PTR Net_INetFwRule_orig_ImageNamePut = 0;


//---------------------------------------------------------------------------


typedef (*P_INetFwXxx_get_ImageName)(void *pUnknown, BSTR *imageFileName);

typedef (*P_INetFwXxx_put_ImageName)(void *pUnknown, BSTR imageFileName);


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


/*static HRESULT Net_INetFwAuthorizedApplication_ImageNameGet(
    void *pUnknown, BSTR *imageFileName);*/

static HRESULT Net_INetFwAuthorizedApplication_ImageNamePut(
    void *pUnknown, BSTR imageFileName);

/*static HRESULT Net_INetFwRule_ImageNameGet(
    void *pUnknown, BSTR *imageFileName);*/

static HRESULT Net_INetFwRule_ImageNamePut(
    void *pUnknown, BSTR imageFileName);

/*static HRESULT Net_Common_ImageNameGet(
    void *pUnknown, BSTR *imageFileName, ULONG_PTR pOrigImageNameGet);*/

static HRESULT Net_Common_ImageNamePut(
    void *pUnknown, BSTR imageFileName, ULONG_PTR pOrigImageNamePut);


//---------------------------------------------------------------------------
// Net_Firewall_Hook
//---------------------------------------------------------------------------


_FX void Net_Firewall_Hook(ULONG type, void *pUnknown)
{
    ULONG prot;
    ULONG_PTR pNewPut;
    ULONG_PTR *pOldPut;
    ULONG_PTR *lpVtbl = *(ULONG_PTR **)pUnknown;

    if (type == 1) {

        //
        // IID_INetFwAuthorizedApplication
        // get_ProcessImageFileName=9, put_ProcessImageFileName=10
        //

        lpVtbl += 9;
        //pOldGet = &Net_INetFwAuthorizedApplication_orig_ImageNameGet;
        pOldPut = &Net_INetFwAuthorizedApplication_orig_ImageNamePut;
        //pNewGet = (ULONG_PTR)Net_INetFwAuthorizedApplication_ImageNameGet;
        pNewPut = (ULONG_PTR)Net_INetFwAuthorizedApplication_ImageNamePut;

    } else if (type == 2) {

        //
        // INetFwRule
        // get_ApplicationName=11, put_ApplicationName=12
        //

        lpVtbl += 11;
        //pOldGet = &Net_INetFwRule_orig_ImageNameGet;
        pOldPut = &Net_INetFwRule_orig_ImageNamePut;
        //pNewGet = (ULONG_PTR)Net_INetFwRule_ImageNameGet;
        pNewPut = (ULONG_PTR)Net_INetFwRule_ImageNamePut;

    } else
        return;

    if (VirtualProtect(
            lpVtbl, sizeof(ULONG_PTR) * 2, PAGE_READWRITE, &prot)) {

        /*if (lpVtbl[0] != pNewGet) {

            *pOldGet = lpVtbl[0];
            lpVtbl[0] = pNewGet;
        }*/

        if (lpVtbl[1] != pNewPut) {

            *pOldPut = lpVtbl[1];
            lpVtbl[1] = pNewPut;
        }

        VirtualProtect(lpVtbl, sizeof(ULONG_PTR) * 2, prot, &prot);
    }
}


//---------------------------------------------------------------------------
// Net_INetFwAuthorizedApplication_ImageNameGet
//---------------------------------------------------------------------------


/*_FX HRESULT Net_INetFwAuthorizedApplication_ImageNameGet(
    void *pUnknown, BSTR *imageFileName)
{
    return Net_Common_ImageNameGet(pUnknown, imageFileName,
                       Net_INetFwAuthorizedApplication_orig_ImageNameGet);
}*/


//---------------------------------------------------------------------------
// Net_INetFwAuthorizedApplication_ImageNamePut
//---------------------------------------------------------------------------


_FX HRESULT Net_INetFwAuthorizedApplication_ImageNamePut(
    void *pUnknown, BSTR imageFileName)
{
    return Net_Common_ImageNamePut(pUnknown, imageFileName,
                       Net_INetFwAuthorizedApplication_orig_ImageNamePut);
}


//---------------------------------------------------------------------------
// Net_INetFwRule_ImageNameGet
//---------------------------------------------------------------------------


/*_FX HRESULT Net_INetFwRule_ImageNameGet(
    void *pUnknown, BSTR *imageFileName)
{
    return Net_Common_ImageNameGet(pUnknown, imageFileName,
                                   Net_INetFwRule_orig_ImageNameGet);
}*/


//---------------------------------------------------------------------------
// Net_INetFwRule_ImageNamePut
//---------------------------------------------------------------------------


_FX HRESULT Net_INetFwRule_ImageNamePut(
    void *pUnknown, BSTR imageFileName)
{
    return Net_Common_ImageNamePut(pUnknown, imageFileName,
                                   Net_INetFwRule_orig_ImageNamePut);
}


//---------------------------------------------------------------------------
// Net_Common_ImageNameGet
//---------------------------------------------------------------------------


/*_FX HRESULT Net_Common_ImageNameGet(
    void *pUnknown, BSTR *imageFileName, ULONG_PTR pOrigImageNameGet)
{
    HRESULT hr;

    hr = ((P_INetFwXxx_get_ImageName)pOrigImageNameGet)(
                                                    pUnknown, imageFileName);

    return hr;
}*/


//---------------------------------------------------------------------------
// Net_Common_ImageNamePut
//---------------------------------------------------------------------------


_FX HRESULT Net_Common_ImageNamePut(
    void *pUnknown, BSTR imageFileName, ULONG_PTR pOrigImageNamePut)
{
    HANDLE hFile;
    ULONG len;
    WCHAR *path;
    HRESULT hr;

    len = *(((ULONG *)imageFileName) - 1);
    if (len > 4096)
        len = 4096;
    len /= sizeof(WCHAR);
    path = Dll_AllocTemp(8192);
    wmemcpy(path, imageFileName, len);
    path[len] = L'\0';

    hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_VALID_FLAGS,
                       NULL, OPEN_EXISTING, 0, NULL);

    if (hFile != INVALID_HANDLE_VALUE) {

        BOOLEAN IsBoxedPath;
        NTSTATUS status = SbieDll_GetHandlePath(hFile, path, &IsBoxedPath);
        if (NT_SUCCESS(status) && IsBoxedPath) {
            if (SbieDll_TranslateNtToDosPath(path)) {

                ULONG len = wcslen(path);
                wmemmove(path + 2, path, len + 1);
                *(ULONG *)path = len * sizeof(WCHAR);

                imageFileName = (BSTR)(path + 2);
            }
        }

        CloseHandle(hFile);
    }

    hr = ((P_INetFwXxx_put_ImageName)pOrigImageNamePut)(
                                                    pUnknown, imageFileName);

    Dll_Free(path);

    return hr;
}


//---------------------------------------------------------------------------
// WSA_GetAddrInfoW
//---------------------------------------------------------------------------

#ifdef PROXY_RESOLVE_HOST_NAMES
_FX int WSA_GetAddrInfoW(
    PCWSTR pNodeName,
    PCWSTR pServiceName,
    const ADDRINFOW* pHints,
    PADDRINFOW* ppResult) 
{
    int ret = __sys_GetAddrInfoW(pNodeName, pServiceName, pHints, ppResult);
    if (ret == 0 && pNodeName && ppResult) {
        size_t len = wcslen(pNodeName) + 1;
        char* host = Dll_AllocTemp(len * 2);
        if (!host) {
            SbieApi_Log(2305, NULL);
            return ret;
        }
        wcstombs(host, pNodeName, len);
        host[len - 1] = '\0';

        ADDRINFOW* pResult = *ppResult;
        if (pResult->ai_family == AF_INET) {
            SOCKADDR_IN* pAddr = (SOCKADDR_IN*)pResult->ai_addr;
            map_insert(&DNS_LookupMap, (void*)pAddr->sin_addr.s_addr, host, len);
        } else if (pResult->ai_family == AF_INET6) {
            SOCKADDR_IN6_LH* pAddr = (SOCKADDR_IN6_LH*)pResult->ai_addr;
            map_insert(&DNS_LookupMap, (void*)pAddr->sin6_addr.s6_addr, host, len);
        }
        Dll_Free(host);
    }
    return ret; 
}
#endif