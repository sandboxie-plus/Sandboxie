/*
 * Copyright 2022 David Xanatos, xanasoft.com
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
// Network Proxy
//---------------------------------------------------------------------------

#include "dll.h"

#include <windows.h>
#include <wchar.h>
#include <oleauto.h>
#include "common/my_wsa.h"
#include "common/netfw.h"
#include "common/map.h"
#include "wsa_defs.h"


#define SOCKS_VERSION               0x05
#define SOCKS_SUBVERSION            0x01

// authentication methods
#define SOCKS_NO_AUTHENTICATION     0x00
#define SOCKS_USERNAME_PASSWORD     0x02
#define SOCKS_METHOD_NONE           0xFF

// response codes
//#define SOCKS_SUCCESS               0x00
#define SOCKS_SERVER_FAILURE        0x01
#define SOCKS_DENIED                0x02
#define SOCKS_NETWORK_UNREACHABLE   0x03
#define SOCKS_HOST_UNREACHABLE      0x04
#define SOCKS_CONNECTION_REFUSED    0x05
#define SOCKS_TTL_EXPIRED           0x06

// address types
#define SOCKS_CONNECT               0x01
#define SOCKS_IPV4                  0x01
#define SOCKS_DOMAINNAME            0x03
#define SOCKS_IPV6                  0x04

#define SOCKS_RESPONSE_MAX_SIZE     512
#define SOCKS_REQUEST_MAX_SIZE      264
#define SOCKS_AUTH_MAX_SIZE         255

#define HOST_NAME_MAX               256
#define INET_ADDRSTRLEN             16
#define INET6_ADDRSTRLEN            46

extern P_recv       __sys_recv;
extern P_send       __sys_send;
extern P_inet_ntop  __sys_inet_ntop;
#ifdef PROXY_RESOLVE_HOST_NAMES
extern HASH_MAP     DNS_LookupMap;
#endif

//---------------------------------------------------------------------------
// socks5_handshake
//---------------------------------------------------------------------------


_FX BOOLEAN socks5_handshake(SOCKET s, BOOLEAN auth, WCHAR login[SOCKS_AUTH_MAX_SIZE], WCHAR pass[SOCKS_AUTH_MAX_SIZE])
{
    char req[4] = { SOCKS_VERSION, 1 + auth, SOCKS_NO_AUTHENTICATION, 0 };

    if (auth) 
        req[3] = SOCKS_USERNAME_PASSWORD;

    if (__sys_send(s, req, (3 + auth), 0) != (3 + auth))
        goto on_error;

    char res[2];
    if (__sys_recv(s, res, sizeof(res), MSG_WAITALL) != sizeof(res))
        goto on_error;

    if (res[0] != SOCKS_VERSION) {
        SbieApi_Log(2360, L"SOCKS version mismatch: expected '%d', got '%d'", SOCKS_VERSION, res[0]);
        goto on_error;
    }

    switch (res[1]) {
    case SOCKS_NO_AUTHENTICATION:
        return TRUE;
    case SOCKS_USERNAME_PASSWORD:
        if (!auth || !login || !pass) {
            SbieApi_Log(2360, L"authentication required, but no credentials provided");
            goto on_error;
        }
        char l[SOCKS_AUTH_MAX_SIZE];
        char p[SOCKS_AUTH_MAX_SIZE];
        size_t login_len = wcstombs(l, login, SOCKS_AUTH_MAX_SIZE);
        size_t pass_len = wcstombs(p, pass, SOCKS_AUTH_MAX_SIZE);

        size_t auth_buf_len = 1 + 1 + login_len + 1 + pass_len;
        char* auth_buf = Dll_AllocTemp(auth_buf_len);
        if (!auth_buf) {
            SbieApi_Log(2305, NULL);
            goto on_error;
        }

        size_t offset = 0;
        auth_buf[offset++] = SOCKS_SUBVERSION;
        auth_buf[offset++] = login_len;
        memcpy(auth_buf + offset, l, login_len);
        offset += login_len;
        auth_buf[offset++] = (char)pass_len;
        memcpy(auth_buf + offset, p, pass_len);
        offset += pass_len;

        if (__sys_send(s, auth_buf, auth_buf_len , 0) != auth_buf_len) {
            Dll_Free(auth_buf);
            goto on_error;
        }
        Dll_Free(auth_buf);

        if (__sys_recv(s, res, sizeof(res), MSG_WAITALL) != sizeof(res))
            goto on_error;

        if (res[0] != SOCKS_SUBVERSION) {
            SbieApi_Log(2360, L"subnegotiation version mismatch: expected '%d', got '%d'", SOCKS_SUBVERSION, res[0]);
            goto on_error;
        }

        if (res[1] != SOCKS_SUCCESS) {
            SbieApi_Log(2360, L"authentication failed");
            goto on_error;
        }

        return TRUE;
    default:
        SbieApi_Log(2360, L"no acceptable authentication method");
        break;
    }

on_error:
    return FALSE;
}


//---------------------------------------------------------------------------
// socks5_request_send
//---------------------------------------------------------------------------


static char socks5_request_send(SOCKET s, char* buf, size_t size)
{
    if (__sys_send(s, buf, size, 0) != size)
        return SOCKS_GENERAL_FAILURE;

    char res[SOCKS_RESPONSE_MAX_SIZE] = { 0 };
    if (__sys_recv(s, res, 4, 0) == SOCKET_ERROR)
        return SOCKS_GENERAL_FAILURE;

    if (res[1] != SOCKS_SUCCESS)
        return res[1];

    if (res[3] == SOCKS_IPV4) {
        if (__sys_recv(s, res + 4, 6, MSG_WAITALL) == SOCKET_ERROR)
            return SOCKS_GENERAL_FAILURE;
    }
    else if (res[3] == SOCKS_IPV6) {
        if (__sys_recv(s, res + 4, 18, MSG_WAITALL) == SOCKET_ERROR)
            return SOCKS_GENERAL_FAILURE;
    }
    else {
        return SOCKS_GENERAL_FAILURE;
    }

    return SOCKS_SUCCESS;
}

//---------------------------------------------------------------------------
// socks5_report_error
//---------------------------------------------------------------------------

_FX void socks5_report_error(int code, const char* buf)
{
    char* host = NULL;
    USHORT port = 0;
    if (buf[3] == SOCKS_IPV4) {
        host = Dll_AllocTemp(INET_ADDRSTRLEN);
        if (!host) return;
        const IN_ADDR* v4 = (const IN_ADDR*)(buf + 4);
        __sys_inet_ntop(AF_INET, v4, host, INET_ADDRSTRLEN);
        port = _ntohs(*((USHORT*)(buf + 8)));
    }
    else if (buf[3] == SOCKS_IPV6) {
        host = Dll_AllocTemp(INET6_ADDRSTRLEN);
        if (!host) return;
        const IN6_ADDR* v6 = (const IN6_ADDR*)(buf + 4);
        __sys_inet_ntop(AF_INET6, v6, host, INET6_ADDRSTRLEN);
        port = _ntohs(*((USHORT*)(buf + 20)));
    }
    else if (buf[3] == SOCKS_DOMAINNAME) {
        size_t domain_len = buf[4];
        host = Dll_AllocTemp(domain_len + 1);
        if (!host) return;
        memcpy(host, buf + 5, domain_len);
        host[domain_len] = '\0';
        port = _ntohs(*((USHORT*)(buf + 5 + domain_len)));
    }
    if (!host) return;

    switch (code) {
    case SOCKS_SERVER_FAILURE:
        SbieApi_Log(2360, L"general server failure (%s:%hu)", host, port);
        break;
    case SOCKS_DENIED:
        SbieApi_Log(2360, L"connection denied by server ruleset (%s:%hu)", host, port);
        break;
    case SOCKS_NETWORK_UNREACHABLE:
        SbieApi_Log(2360, L"network unreachable (%s:%hu)", host, port);
        break;
    case SOCKS_HOST_UNREACHABLE:
        SbieApi_Log(2360, L"host unreachable (%s:%hu)", host, port);
        break;
    case SOCKS_CONNECTION_REFUSED:
        SbieApi_Log(2360, L"connection refused (%s:%hu)", host, port);
        break;
    case SOCKS_TTL_EXPIRED:
        SbieApi_Log(2360, L"TTL expired (%s:%hu)", host, port);
        break;
    default:
        SbieApi_Log(2360, L"request failed with status %d (%s:%hu)", code, host, port);
        break;
    }

    Dll_Free(host);
}

//---------------------------------------------------------------------------
// socks5_request
//---------------------------------------------------------------------------

_FX char socks5_request(SOCKET s, const SOCKADDR* addr)
{
    char req[SOCKS_REQUEST_MAX_SIZE] = { SOCKS_VERSION, SOCKS_CONNECT, 0 };

    char* ptr = req + 3;
    if (addr->sa_family == AF_INET) {
        const SOCKADDR_IN* v4 = (const SOCKADDR_IN*)addr;
#ifdef PROXY_RESOLVE_HOST_NAMES
        char* domain = (char*)map_get(&DNS_LookupMap, (void*)v4->sin_addr.s_addr);
        if (domain) {
            *ptr++ = SOCKS_DOMAINNAME;
            *ptr++ = strlen(domain);
            memcpy(ptr, domain, strlen(domain));
            ptr += strlen(domain);
            *((USHORT*)ptr) = v4->sin_port;
            ptr += sizeof(USHORT);
        } 
        else 
#endif
        {
            *ptr++ = SOCKS_IPV4;
            *((ULONG*)ptr) = v4->sin_addr.s_addr;
            ptr += sizeof(ULONG);
            *((USHORT*)ptr) = v4->sin_port;
            ptr += sizeof(USHORT);
        }
    }
    else if (addr->sa_family == AF_INET6) {
        const SOCKADDR_IN6_LH* v6 = (const SOCKADDR_IN6_LH*)addr;
#ifdef PROXY_RESOLVE_HOST_NAMES
        char* domain = (char*)map_get(&DNS_LookupMap, (void*)&v6->sin6_addr.s6_addr);
        if (domain) {
            *ptr++ = SOCKS_DOMAINNAME;
            *ptr++ = strlen(domain);
            memcpy(ptr, domain, strlen(domain));
            ptr += strlen(domain);
            *((USHORT*)ptr) = v6->sin6_port;
            ptr += sizeof(USHORT);
        }
        else 
#endif
        {
            *ptr++ = SOCKS_IPV6;
            memcpy(ptr, &v6->sin6_addr, sizeof(v6->sin6_addr));
            ptr += sizeof(v6->sin6_addr);
            *((USHORT*)ptr) = v6->sin6_port;
            ptr += sizeof(USHORT);
        }
    }
    else {
        return SOCKS_GENERAL_FAILURE;
    }

    int ret = socks5_request_send(s, req, ptr - req);
    if (ret != SOCKS_SUCCESS)
        socks5_report_error(ret, req);
    return ret;
}
