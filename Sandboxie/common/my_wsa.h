/*
 * Copyright 2021-2024 DavidXanatos, xanasoft.com
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
// IP Helpers
//---------------------------------------------------------------------------


#ifndef _MY_WSA_H
#define _MY_WSA_H

#ifndef NO_IP_DEFS

#include <inaddr.h>
#include <in6addr.h>

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#define SOCKS_SUCCESS           0
#define SOCKS_GENERAL_FAILURE   1

#define IOC_IN                  0x80000000      /* copy in parameters */
#define _WSAIOW(x,y)            (IOC_IN|(x)|(y))
#define IOC_WS2                 0x08000000
#define SIO_NSP_NOTIFY_CHANGE   _WSAIOW(IOC_WS2,25)

#define WSA_IO_PENDING          (ERROR_IO_PENDING)

#define AF_UNSPEC               0               /* unspecified */
#define AF_UNIX                 1               /* unix socket available since windows build 17063 */
#define AF_INET                 2               /* internetwork: UDP, TCP, etc. */
#define AF_INET6                23              /* internetwork v6: UDP, TCP, etc. */
#define SOCKET                  ULONG_PTR

#define IPPROTO_ICMP            1               /* control message protocol */
#define IPPROTO_TCP             6               /* tcp */
#define IPPROTO_UDP             17              /* user datagram protocol */

#define SOCK_STREAM             1               /* stream socket */
#define SOCK_DGRAM              2               /* datagram socket */
#define SOCK_RAW                3               /* raw-protocol interface */
#define SOCK_RDM                4               /* reliably-delivered message */
#define SOCK_SEQPACKET          5               /* sequenced packet stream */

#define IPPROTO_ANY	            256

#define SD_RECEIVE              0x00
#define SD_SEND                 0x01
#define SD_BOTH                 0x02

#define MSG_WAITALL             0x8             /* do not complete until packet is completely filled */

#define FIONBIO                 0x8004667e


/*
 * WinSock 2 extension -- bit values and indices for FD_XXX network events
 */
#define FD_READ_BIT      0
#define FD_READ          (1 << FD_READ_BIT)

#define FD_WRITE_BIT     1
#define FD_WRITE         (1 << FD_WRITE_BIT)

#define FD_OOB_BIT       2
#define FD_OOB           (1 << FD_OOB_BIT)

#define FD_ACCEPT_BIT    3
#define FD_ACCEPT        (1 << FD_ACCEPT_BIT)

#define FD_CONNECT_BIT   4
#define FD_CONNECT       (1 << FD_CONNECT_BIT)

#define FD_CLOSE_BIT     5
#define FD_CLOSE         (1 << FD_CLOSE_BIT)

#define FD_QOS_BIT       6
#define FD_QOS           (1 << FD_QOS_BIT)

#define FD_GROUP_QOS_BIT 7
#define FD_GROUP_QOS     (1 << FD_GROUP_QOS_BIT)

#define FD_ROUTING_INTERFACE_CHANGE_BIT 8
#define FD_ROUTING_INTERFACE_CHANGE     (1 << FD_ROUTING_INTERFACE_CHANGE_BIT)

#define FD_ADDRESS_LIST_CHANGE_BIT 9
#define FD_ADDRESS_LIST_CHANGE     (1 << FD_ADDRESS_LIST_CHANGE_BIT)

#define FD_MAX_EVENTS    10
#define FD_ALL_EVENTS    ((1 << FD_MAX_EVENTS) - 1)

//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef void* LPWSACOMPLETION;
typedef void* LPWSABUF;
typedef void* LPWSAOVERLAPPED;
typedef void* LPWSAOVERLAPPED_COMPLETION_ROUTINE;
typedef void* LPCONDITIONPROC;
typedef void* LPWSAPROTOCOL_INFOW;
typedef void* LPQOS;



typedef USHORT ADDRESS_FAMILY;

typedef struct {
    union {
        struct {
            ULONG Zone : 28;
            ULONG Level : 4;
        };
        ULONG Value;
    };
} SCOPE_ID, *PSCOPE_ID;

typedef struct sockaddr {

    ADDRESS_FAMILY sa_family;           // Address family.

    CHAR sa_data[14];                   // Up to 14 bytes of direct address.
} SOCKADDR, *PSOCKADDR, FAR *LPSOCKADDR;

typedef struct sockaddr_in {

    ADDRESS_FAMILY sin_family;

    USHORT sin_port;
    IN_ADDR sin_addr;
    CHAR sin_zero[8];
} SOCKADDR_IN, *PSOCKADDR_IN;

typedef struct sockaddr_in6 {
    ADDRESS_FAMILY sin6_family; // AF_INET6.
    USHORT sin6_port;           // Transport level port number.
    ULONG  sin6_flowinfo;       // IPv6 flow information.
    IN6_ADDR sin6_addr;         // IPv6 address.
    union {
        ULONG sin6_scope_id;    // Set of interfaces for a scope.
        SCOPE_ID sin6_scope_struct;
    };
} SOCKADDR_IN6_LH, *PSOCKADDR_IN6_LH, FAR *LPSOCKADDR_IN6_LH;

typedef struct sockaddr_un {
    ADDRESS_FAMILY  family;     // AF_UNIX
    char            path[1];    // Pathname
} SOCKADDR_UN;

typedef void (*PIPFORWARD_CHANGE_CALLBACK)
    (void *CallerContext, void *Row, ULONG NotificationType);

typedef struct _WSANETWORKEVENTS {
       long lNetworkEvents;
       int iErrorCode[FD_MAX_EVENTS];
} WSANETWORKEVENTS, FAR * LPWSANETWORKEVENTS;

typedef unsigned int u_int;

#ifndef FD_SETSIZE
#define FD_SETSIZE      64
#endif /* FD_SETSIZE */

typedef struct fd_set {
        u_int fd_count;          /* how many are SET? */
        SOCKET  fd_array[FD_SETSIZE];   /* an array of SOCKETs */
} fd_set;

#define FD_CLR(fd, set) do { \
    u_int __i; \
    for (__i = 0; __i < ((fd_set FAR *)(set))->fd_count ; __i++) { \
        if (((fd_set FAR *)(set))->fd_array[__i] == fd) { \
            while (__i < ((fd_set FAR *)(set))->fd_count-1) { \
                ((fd_set FAR *)(set))->fd_array[__i] = \
                    ((fd_set FAR *)(set))->fd_array[__i+1]; \
                __i++; \
            } \
            ((fd_set FAR *)(set))->fd_count--; \
            break; \
        } \
    } \
} while(0)

#define FD_SET(fd, set) do { \
    u_int __i; \
    for (__i = 0; __i < ((fd_set FAR *)(set))->fd_count; __i++) { \
        if (((fd_set FAR *)(set))->fd_array[__i] == (fd)) { \
            break; \
        } \
    } \
    if (__i == ((fd_set FAR *)(set))->fd_count) { \
        if (((fd_set FAR *)(set))->fd_count < FD_SETSIZE) { \
            ((fd_set FAR *)(set))->fd_array[__i] = (fd); \
            ((fd_set FAR *)(set))->fd_count++; \
        } \
    } \
} while(0)

#define FD_ZERO(set) (((fd_set FAR *)(set))->fd_count=0)

#define FD_ISSET(fd, set) __sys_WSAFDIsSet((SOCKET)(fd), (fd_set FAR *)(set))

struct timeval {
        long    tv_sec;         /* seconds */
        long    tv_usec;        /* and microseconds */
};


#endif

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

__inline ULONG _ntohl(ULONG IP)
{
	ULONG PI;
	((UCHAR*)&PI)[0] = ((UCHAR*)&IP)[3];
	((UCHAR*)&PI)[1] = ((UCHAR*)&IP)[2];
	((UCHAR*)&PI)[2] = ((UCHAR*)&IP)[1];
	((UCHAR*)&PI)[3] = ((UCHAR*)&IP)[0];
	return PI;
}

__inline USHORT _ntohs(USHORT PT)
{
	USHORT TP;
	((UCHAR*)&TP)[0] = ((UCHAR*)&PT)[1];
	((UCHAR*)&TP)[1] = ((UCHAR*)&PT)[0];
	return TP;
}


#endif /* _MY_WSA_H */
