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
// Network Related
//---------------------------------------------------------------------------

#include "dll.h"

#include <windows.h>
#include <wchar.h>
#include <oleauto.h>
#include "common/my_wsa.h"
#include "common/netfw.h"


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
// Functions
//---------------------------------------------------------------------------

static void WSA_InitNetFwRules();

static int WSA_IsBlockedTraffic(const short *addr, int addrlen, int protocol);

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

static int WSA_WSANSPIoctl(
    HANDLE          hLookup,
    DWORD           dwControlCode,
    LPVOID          lpvInBuffer,
    DWORD           cbInBuffer,
    LPVOID          lpvOutBuffer,
    DWORD           cbOutBuffer,
    LPDWORD         lpcbBytesReturned,
    LPWSACOMPLETION lpCompletion);

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

/*static SOCKET WSA_accept(
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

/*static int WSA_recvfrom(
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

static int WSA_closesocket(SOCKET s);*/


//---------------------------------------------------------------------------

typedef int (*P_WSAIoctl)(
    SOCKET                             s,
    DWORD                              dwIoControlCode,
    LPVOID                             lpvInBuffer,
    DWORD                              cbInBuffer,
    LPVOID                             lpvOutBuffer,
    DWORD                              cbOutBuffer,
    LPDWORD                            lpcbBytesReturned,
    LPWSAOVERLAPPED                    lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

typedef int (*P_WSANSPIoctl)(
    HANDLE          hLookup,
    DWORD           dwControlCode,
    LPVOID          lpvInBuffer,
    DWORD           cbInBuffer,
    LPVOID          lpvOutBuffer,
    DWORD           cbOutBuffer,
    LPDWORD         lpcbBytesReturned,
    LPWSACOMPLETION lpCompletion);

typedef int (*P_WSASocketW)(
    int                 af,
    int                 type,
    int                 protocol,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,
    unsigned int        g,
    DWORD               dwFlags);

typedef int (*P_bind)(
    SOCKET         s,
    const void     *name,
    int            namelen);

typedef int (*P_connect)(
    SOCKET         s,
    const void     *name,
    int            namelen);

typedef int (*P_WSAConnect)(
    SOCKET         s,
    const void     *name,
    int            namelen,
    LPWSABUF       lpCallerData,
    LPWSABUF       lpCalleeData,
    LPQOS          lpSQOS,
    LPQOS          lpGQOS);

typedef int (*P_ConnectEx) (
    SOCKET          s,
    const void      *name,
    int             namelen,
    PVOID           lpSendBuffer,
    DWORD           dwSendDataLength,
    LPDWORD         lpdwBytesSent,
    LPOVERLAPPED    lpOverlapped);

typedef SOCKET (*P_accept)(
    SOCKET   s,
    void     *addr,
    int      *addrlen);

typedef SOCKET (*P_WSAAccept)(
    SOCKET          s,
    void            *addr,
    LPINT           addrlen,
    LPCONDITIONPROC lpfnCondition,
    DWORD_PTR       dwCallbackData);

typedef int (*P_AcceptEx)(
    SOCKET       sListenSocket,
    SOCKET       sAcceptSocket,
    PVOID        lpOutputBuffer,
    DWORD        dwReceiveDataLength,
    DWORD        dwLocalAddressLength,
    DWORD        dwRemoteAddressLength,
    LPDWORD      lpdwBytesReceived,
    LPOVERLAPPED lpOverlapped);

typedef int (*P_sendto)(
    SOCKET         s,
    const char     *buf,
    int            len,
    int            flags,
    const void     *to,
    int            tolen);

typedef int (*P_WSASendTo)(
    SOCKET                             s,
    LPWSABUF                           lpBuffers,
    DWORD                              dwBufferCount,
    LPDWORD                            lpNumberOfBytesSent,
    DWORD                              dwFlags,
    const void                         *lpTo,
    int                                iTolen,
    LPWSAOVERLAPPED                    lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

typedef int (*P_recvfrom)(
    SOCKET   s,
    char     *buf,
    int      len,
    int      flags,
    void     *from,
    int      *fromlen);

typedef int (*P_WSARecvFrom)(
    SOCKET                             s,
    LPWSABUF                           lpBuffers,
    DWORD                              dwBufferCount,
    LPDWORD                            lpNumberOfBytesRecvd,
    LPDWORD                            lpFlags,
    void                               *lpFrom,
    LPINT                              lpFromlen,
    LPWSAOVERLAPPED                    lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

typedef int (*P_closesocket)(SOCKET s);

//---------------------------------------------------------------------------


static P_WSAIoctl           __sys_WSAIoctl          = NULL;
static P_WSANSPIoctl        __sys_WSANSPIoctl       = NULL;

static P_WSASocketW         __sys_WSASocketW        = NULL;

static P_bind               __sys_bind              = NULL;

static P_connect            __sys_connect           = NULL;
static P_WSAConnect         __sys_WSAConnect        = NULL;
static P_ConnectEx          __sys_ConnectEx         = NULL;

/*static P_accept             __sys_accept            = NULL;
static P_WSAAccept          __sys_WSAAccept         = NULL;
static P_AcceptEx           __sys_AcceptEx          = NULL;*/

static P_sendto             __sys_sendto            = NULL;
static P_WSASendTo          __sys_WSASendTo         = NULL;

/*static P_recvfrom           __sys_recvfrom          = NULL;
static P_WSARecvFrom        __sys_WSARecvFrom       = NULL;

static P_closesocket        __sys_closesocket       = NULL;*/

//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------

static LIST       WSA_FwList;

extern POOL*      Dll_Pool;

static BOOLEAN    WSA_WFPisEnabled   = FALSE;
static BOOLEAN    WSA_WFPisBlocking   = FALSE;

static BOOLEAN    WSA_TraceFlag = FALSE;

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
            // dont ask again on success
            //

            WSA_WFPisBlocking = FALSE;
        }
    }

    return __sys_WSASocketW(af, type, protocol, lpProtocolInfo, g, dwFlags);
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

    int ret = __sys_bind(s, name, namelen);

    if (new_name) Dll_Free((void*)name);

    return ret;
}


//---------------------------------------------------------------------------
// WSA_IsBlockedTraffic
//---------------------------------------------------------------------------


_FX int WSA_IsBlockedTraffic(const short *addr, int addrlen, int protocol)
{

    if (WSA_FwList.count > 0 && addrlen >= sizeof(USHORT) * 2 && addr && (addr[0] == AF_INET || addr[0] == AF_INET6)) {

        USHORT port = _ntohs(addr[1]);

        IP_ADDRESS ip;
        //ip.Type = (BYTE)addr[0];
        if ((BYTE)addr[0] == AF_INET6 && addrlen >= sizeof(SOCKADDR_IN6_LH)) {
            memcpy(ip.Data, ((SOCKADDR_IN6_LH*)addr)->sin6_addr.u.Byte, 16);
        }
        else  if ((BYTE)addr[0] == AF_INET && addrlen >= sizeof(SOCKADDR_IN)) {
            // IPv4-mapped IPv6 addresses, eg. ::FFFF:192.168.0.1
            ip.Data32[0] = 0;
            ip.Data32[1] = 0;
            ip.Data32[2] = 0xFFFF0000;
            ip.Data32[3] = ((SOCKADDR_IN*)addr)->sin_addr.S_un.S_addr;
            //*((ULONG*)ip.Data) = ((SOCKADDR_IN*)addr)->sin_addr.S_un.S_addr;
        }
        else // something's wrong
            return 1; // lets block it

        BOOLEAN block = NetFw_BlockTraffic(&WSA_FwList, &ip, port, protocol);

        if (WSA_TraceFlag){
            WCHAR msg[256];
			if ((BYTE)addr[0] == AF_INET6) {
				Sbie_snwprintf(msg, 256, L"Network Traffic; Port: %u; Prot: %u; IPv6: %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x", port, protocol,
					ip.Data[0], ip.Data[1], ip.Data[2], ip.Data[3], ip.Data[4], ip.Data[5], ip.Data[6], ip.Data[7],
					ip.Data[8], ip.Data[9], ip.Data[10], ip.Data[11], ip.Data[12], ip.Data[13], ip.Data[14], ip.Data[15]);
			}
			else {
				Sbie_snwprintf(msg, 256, L"Network Traffic; Port: %u; Prot: %u; IPv4: %d.%d.%d.%d", port, protocol, 
                    ip.Data[12], ip.Data[13], ip.Data[14], ip.Data[15]);
			}
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
// WSA_connect
//---------------------------------------------------------------------------


_FX int WSA_connect(
    SOCKET         s,
    const void     *name,
    int            namelen)
{
    if (WSA_IsBlockedTraffic(name, namelen, IPPROTO_TCP))
        return SOCKET_ERROR;

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

    BOOLEAN new_name = WSA_HandleAfUnix(&name, &namelen);

    int ret = __sys_WSAConnect(
        s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS);

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

    BOOLEAN new_name = WSA_HandleAfUnix(&name, &namelen);

    int ret = __sys_ConnectEx(
        s, name, namelen, lpSendBuffer, dwSendDataLength, lpdwBytesSent, lpOverlapped);

    if (new_name) Dll_Free((void*)name);

    return ret;
}

/*
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
    // this call can operate asynchroniusly, hence we can not simply filter here the incomming connection
    // as we have a proepr WFP filter in the driver for now this usermode filtering implementation 
    // will not filter incomming traffic at all, normally users ate beind NATs or other firewall so 
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
    return __sys_WSASendTo(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent,
        dwFlags, lpTo, iTolen, lpOverlapped, lpCompletionRoutine);
}

/*
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
    if (WSA_IsBlockedTraffic(from, *fromlen, IPPROTO_UDP))
        return SOCKET_ERROR;
    return __sys_recvfrom(s, buf, len, flags, from, fromlen);
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
    if (WSA_IsBlockedTraffic(lpFrom, *lpFromlen, IPPROTO_UDP))
        return SOCKET_ERROR;
    return __sys_WSARecvFrom(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd,
        lpFlags, lpFrom, lpFromlen, lpOverlapped, lpCompletionRoutine);
}
*/

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
        WCHAR* found_value = Config_MatchImageAndGetValue(conf_buf, Dll_ImageName, &level);
        if (!found_value)
            continue;

        NETFW_RULE* rule = NetFw_AllocRule(Dll_Pool, level);
        if (!rule) {
            SbieApi_Log(2305, NULL);
            continue;
        }

		NetFw_ParseRule(rule, found_value);

        NetFw_AddRule(&WSA_FwList, rule);
    }
}


//---------------------------------------------------------------------------
// WSA_Init
//---------------------------------------------------------------------------


_FX BOOLEAN WSA_Init(HMODULE module)
{
    P_WSAIoctl          WSAIoctl;
    P_WSANSPIoctl       WSANSPIoctl;

    P_WSASocketW        WSASocketW;

    P_bind              bind;

    P_connect           connect;
    P_WSAConnect        WSAConnect;
    /*P_accept            accept;
    P_WSAAccept         WSAAccept;*/

    P_sendto            sendto;
    P_WSASendTo         WSASendTo;
    /*P_recvfrom          recvfrom;
    P_WSARecvFrom       WSARecvFrom;*/


    WSAIoctl = (P_WSAIoctl)GetProcAddress(module, "WSAIoctl");
    if (WSAIoctl) {
        SBIEDLL_HOOK(WSA_,WSAIoctl);
    }

    WSANSPIoctl = (P_WSANSPIoctl)GetProcAddress(module, "WSANSPIoctl");
    if (WSANSPIoctl) {
        SBIEDLL_HOOK(WSA_,WSANSPIoctl);
    }


    bind = (P_bind)GetProcAddress(module, "bind");
    if (bind) {
        SBIEDLL_HOOK(WSA_,bind);
    }


    //
    // initialize the network firewall rule list and hook the relevant functions
    //

    List_Init(&WSA_FwList);

    WSA_WFPisEnabled = SbieApi_QueryConfBool(NULL, L"NetworkEnableWFP", FALSE);
    if(WSA_WFPisEnabled)
        WSA_WFPisBlocking = !Config_GetSettingsForImageName_bool(L"AllowNetworkAccess", TRUE);
    else // load rules only when the driver is not doing the filtering
        WSA_InitNetFwRules();


    if (! Dll_SkipHook(L"wsaconn")) {

        WSASocketW = (P_WSASocketW)GetProcAddress(module, "WSASocketW");
        if (WSASocketW) {
            SBIEDLL_HOOK(WSA_,WSASocketW);
        }

        // TCP
        connect = (P_connect)GetProcAddress(module, "connect");
        if (connect) {
            SBIEDLL_HOOK(WSA_,connect);
        }

        WSAConnect = (P_WSAConnect)GetProcAddress(module, "WSAConnect");
        if (WSAConnect) {
            SBIEDLL_HOOK(WSA_,WSAConnect);
        }

        /*accept = (P_accept)GetProcAddress(module, "accept");
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

        /*recvfrom = (P_recvfrom)GetProcAddress(module, "recvfrom");
        if (recvfrom) {
            SBIEDLL_HOOK(WSA_,recvfrom);
        }

        WSARecvFrom = (P_WSARecvFrom)GetProcAddress(module, "WSARecvFrom");
        if (WSARecvFrom) {
            SBIEDLL_HOOK(WSA_,WSARecvFrom);
        }*/
        //
        
        // used for accept
        //__sys_closesocket = (P_closesocket)GetProcAddress(module, "closesocket");
    }

    {
        // If there are any NetFwTrace options set, then output this debug string
        WCHAR wsTraceOptions[4];
        if (SbieApi_QueryConf(NULL, L"NetFwTrace", 0, wsTraceOptions, sizeof(wsTraceOptions)) == STATUS_SUCCESS && wsTraceOptions[0] != L'\0')
            WSA_TraceFlag = TRUE;
    }

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