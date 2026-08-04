/*
 * Copyright 2022 DavidXanatos, xanasoft.com
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


#ifndef _WSA_DEFS_H
#define _WSA_DEFS_H

//---------------------------------------------------------------------------
// Prototypes
//---------------------------------------------------------------------------

typedef int (*P_WSAStartup)(
    WORD wVersionRequested,
    void* lpWSAData);
    
typedef int (*P_WSACleanup)(void);

typedef int (*P_socket)(
  int af,
  int type,
  int protocol);

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

typedef int (*P_ioctlsocket)(
    SOCKET  s,
    long    cmd,
    ULONG*  argp);

typedef int (*P_select)(
    int nfds,
    void *readfds,
    void *writefds,
    void *exceptfds,
    const void *timeout);

typedef int (*P_WSAAsyncSelect)(
    SOCKET  s,
    HWND    hWnd,
    UINT    wMsg,
    long    lEvent);

typedef int (*P_WSAEventSelect)(
    SOCKET  s,
    void*   hEventObject,
    long    lNetworkEvents);

typedef int (*P_WSAEnumNetworkEvents)(
    SOCKET  s,
    void*   hEventObject,
    void*   lpNetworkEvents
);

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

typedef int (*P_WSAGetLastError)();

typedef int (*P_WSASetLastError)(int err);

typedef int (*P_bind)(
    SOCKET         s,
    const void     *name,
    int            namelen);

typedef int (*P_getsockname)(
    SOCKET         s,
    const void     *name,
    int            *namelen);

typedef int (*P_WSAFDIsSet)(
  SOCKET unnamedParam1,
  void *unnamedParam2);

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

typedef int (*P_listen)(
    SOCKET         s,
    int            backlog);

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

typedef int (*P_recv)(
    SOCKET      s,
    char*       buf,
    int         len,
    int         flags);

typedef int (*P_send)(
    SOCKET      s,
    const char* buf,
    int         len,
    int         flags);

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

typedef int (*P_shutdown)(SOCKET s, int how);

typedef int (*P_closesocket)(SOCKET s);





typedef enum _WSAEcomparator
{
    COMP_EQUAL = 0,
    COMP_NOTLESS
} WSAECOMPARATOR, *PWSAECOMPARATOR, *LPWSAECOMPARATOR;

typedef struct _WSAVersion
{
    DWORD           dwVersion;
    WSAECOMPARATOR  ecHow;
}WSAVERSION, *PWSAVERSION, *LPWSAVERSION;

typedef struct _AFPROTOCOLS {
    INT iAddressFamily;
    INT iProtocol;
} AFPROTOCOLS, *PAFPROTOCOLS, *LPAFPROTOCOLS;

typedef struct _SOCKET_ADDRESS {
    LPSOCKADDR lpSockaddr;
    INT iSockaddrLength;
} SOCKET_ADDRESS, *PSOCKET_ADDRESS, *LPSOCKET_ADDRESS;

typedef struct _CSADDR_INFO {
    SOCKET_ADDRESS LocalAddr ;
    SOCKET_ADDRESS RemoteAddr ;
    INT iSocketType ;
    INT iProtocol ;
} CSADDR_INFO, *PCSADDR_INFO, FAR * LPCSADDR_INFO ;

typedef struct _WSAQuerySetW
{
    DWORD           dwSize;
    LPWSTR          lpszServiceInstanceName;
    LPGUID          lpServiceClassId;
    LPWSAVERSION    lpVersion;
    LPWSTR          lpszComment;
    DWORD           dwNameSpace;
    LPGUID          lpNSProviderId;
    LPWSTR          lpszContext;
    DWORD           dwNumberOfProtocols;
    LPAFPROTOCOLS   lpafpProtocols;
    LPWSTR          lpszQueryString;
    DWORD           dwNumberOfCsAddrs;
    LPCSADDR_INFO   lpcsaBuffer;
    DWORD           dwOutputFlags;
    LPBLOB          lpBlob;
} WSAQUERYSETW, *PWSAQUERYSETW, *LPWSAQUERYSETW;

struct  hostent {
        char    FAR * h_name;           /* official name of host */
        char    FAR * FAR * h_aliases;  /* alias list */
        short   h_addrtype;             /* host address type */
        short   h_length;               /* length of address */
        char    FAR * FAR * h_addr_list; /* list of addresses */
#define h_addr  h_addr_list[0]          /* address, for backward compat */
};

typedef struct hostent HOSTENT;

typedef int (*P_WSALookupServiceBeginW)(
    LPWSAQUERYSETW  lpqsRestrictions,
    DWORD           dwControlFlags,
    LPHANDLE        lphLookup);

typedef int (*P_WSALookupServiceNextW)(
    HANDLE          hLookup,
    DWORD           dwControlFlags,
    LPDWORD         lpdwBufferLength,
    LPWSAQUERYSETW  lpqsResults);

typedef int (*P_WSALookupServiceEnd)(HANDLE  hLookup);

typedef struct addrinfoW {
    int     ai_flags;
    int     ai_family;
    int     ai_socktype;
    int     ai_protocol;
    size_t  ai_addrlen;
    PWSTR   ai_canonname;
    struct sockaddr *ai_addr;
    struct addrinfoW *ai_next;
} ADDRINFOW, *PADDRINFOW;

typedef int (*P_GetAddrInfoW)(
    PCWSTR          pNodeName,
    PCWSTR          pServiceName,
    const ADDRINFOW *pHints,
    PADDRINFOW      *ppResult);

typedef void (*P_FreeAddrInfoW)(
    PADDRINFOW      pAddrInfo);

typedef PCSTR (*P_inet_ntop)(
    int            family,
    const void     *pAddr,
    PSTR           pStringBuf,
    size_t         StringBufSize);

typedef ULONG (*P_GetAdaptersAddresses)(
    ULONG Family,
    ULONG Flags,
    PVOID Reserved,
    void* AdapterAddresses,
    PULONG SizePointer);

#endif _WSA_DEFS_H