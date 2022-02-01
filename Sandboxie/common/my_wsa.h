/*
 * Copyright 2021 DavidXanatos, xanasoft.com
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


#define SOCKET_ERROR            (-1)

#define IOC_IN                  0x80000000      /* copy in parameters */
#define _WSAIOW(x,y)            (IOC_IN|(x)|(y))
#define IOC_WS2                 0x08000000
#define SIO_NSP_NOTIFY_CHANGE   _WSAIOW(IOC_WS2,25)

#define WSA_IO_PENDING          (ERROR_IO_PENDING)

#define AF_UNIX                 1               /* unix socket available since windows build 17063 */
#define AF_INET                 2               /* internetwork: UDP, TCP, etc. */
#define AF_INET6                23              /* internetwork v6: UDP, TCP, etc. */
#define SOCKET                  ULONG_PTR

#define IPPROTO_ICMP            1               /* control message protocol */
#define IPPROTO_TCP             6               /* tcp */
#define IPPROTO_UDP             17              /* user datagram protocol */

#define IPPROTO_ANY	            256


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
