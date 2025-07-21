/*
 * Copyright 2022-2025 David Xanatos, xanasoft.com
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

extern P_socket     __sys_socket;
extern P_bind       __sys_bind;
extern P_getsockname __sys_getsockname;
extern P_WSAFDIsSet __sys_WSAFDIsSet;
extern P_connect    __sys_connect;
extern P_recv       __sys_recv;
extern P_send       __sys_send;
extern P_inet_ntop  __sys_inet_ntop;
extern P_select     __sys_select;
extern P_listen     __sys_listen;
extern P_accept     __sys_accept;
extern P_closesocket __sys_closesocket;
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

//---------------------------------------------------------------------------
// RELAY_CONFIG
//---------------------------------------------------------------------------

typedef struct {
    SOCKET listen_sock;
    union {
        SOCKADDR        addr;
        SOCKADDR_IN     addr4;
        SOCKADDR_IN6_LH addr6;
    };
    union {
        SOCKADDR        proxy;
        SOCKADDR_IN     proxy4;
        SOCKADDR_IN6_LH proxy6;
    };
    BOOLEAN auth;
    WCHAR login[SOCKS_AUTH_MAX_SIZE];
    WCHAR pass[SOCKS_AUTH_MAX_SIZE];
} RELAY_CONFIG,* PRELAY_CONFIG;


//---------------------------------------------------------------------------
// run_relay_loop
//---------------------------------------------------------------------------

#define BUF_SIZE     4096

VOID run_relay_loop(SOCKET a, SOCKET b)
{
    char buffer[BUF_SIZE];
    fd_set readSet;
    while (1) {
        FD_ZERO(&readSet);
        FD_SET(a, &readSet);
        FD_SET(b, &readSet);
        SOCKET maxfd = (a > b ? a : b) + 1;
        int ready = __sys_select((int)maxfd, &readSet, NULL, NULL, NULL);
        if (ready <= 0) break;
        if (FD_ISSET(a, &readSet)) {
            int bytes = __sys_recv(a, buffer, BUF_SIZE, 0);
            if (bytes <= 0) break;
            int sent = 0;
            while (sent < bytes) {
                int n = __sys_send(b, buffer + sent, bytes - sent, 0);
                if (n <= 0) return;
                sent += n;
            }
        }
        if (FD_ISSET(b, &readSet)) {
            int bytes = __sys_recv(b, buffer, BUF_SIZE, 0);
            if (bytes <= 0) break;
            int sent = 0;
            while (sent < bytes) {
                int n = __sys_send(a, buffer + sent, bytes - sent, 0);
                if (n <= 0) return;
                sent += n;
            }
        }
    }
}

//---------------------------------------------------------------------------
// connect_to_proxy
//---------------------------------------------------------------------------

BOOLEAN connect_to_proxy(PRELAY_CONFIG relay_config, SOCKET* proxy)
{
    *proxy = __sys_socket(relay_config->proxy.sa_family, SOCK_STREAM, IPPROTO_TCP);
    if (*proxy != INVALID_SOCKET) 
    {
        int proxylen = relay_config->proxy.sa_family == AF_INET ? sizeof(SOCKADDR_IN) : sizeof(SOCKADDR_IN6_LH);
        if (__sys_connect(*proxy, &relay_config->proxy, proxylen) == SOCKS_SUCCESS)
        {
            if (socks5_handshake(*proxy, relay_config->auth, relay_config->login, relay_config->pass)
             && socks5_request(*proxy, &relay_config->addr) == SOCKS_SUCCESS)
                return TRUE;
        }

        __sys_closesocket(*proxy);
    }
    return FALSE;
}

//---------------------------------------------------------------------------
// proxy_handle_relay
//---------------------------------------------------------------------------

DWORD WINAPI proxy_handle_relay(LPVOID param)
{
    PRELAY_CONFIG relay_config = (RELAY_CONFIG*)param;

    SOCKET client = INVALID_SOCKET;
    SOCKET proxy = INVALID_SOCKET;

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(relay_config->listen_sock, &fds);

    struct timeval tv;
    tv.tv_sec = 5;       // 5 second timeout
    tv.tv_usec = 0;

    int ret = __sys_select((int)relay_config->listen_sock + 1, &fds, NULL, NULL, &tv);
    if (ret > 0) { // handle clien        
        client = __sys_accept(relay_config->listen_sock, NULL, NULL);
    }
    /*else if (ret == 0) { // timeout occurred
    
    }
    else { // error

    }*/
    __sys_closesocket(relay_config->listen_sock);

    if (client != INVALID_SOCKET)
    {
        if (connect_to_proxy(relay_config, &proxy)) 
        {
            Dll_Free(relay_config);
            relay_config = NULL;
            
            run_relay_loop(client, proxy);

            __sys_closesocket(proxy);
        }

        __sys_closesocket(client);
    }
    
    if(relay_config)
        Dll_Free(relay_config);
    return 0;
}

//---------------------------------------------------------------------------
// start_socks5_relay
//---------------------------------------------------------------------------

USHORT start_socks5_relay(const SOCKADDR* addr, const SOCKADDR* proxy, BOOLEAN auth, WCHAR login[SOCKS_AUTH_MAX_SIZE], WCHAR pass[SOCKS_AUTH_MAX_SIZE])
{
    PRELAY_CONFIG relay_config = Dll_Alloc(sizeof(RELAY_CONFIG));
    if (!relay_config)
        return 0; // fail
    relay_config->listen_sock = INVALID_SOCKET;

    if (addr->sa_family == AF_INET)
    {
        memcpy(&relay_config->addr4, addr, sizeof(SOCKADDR_IN));
        memcpy(&relay_config->proxy4, proxy, sizeof(SOCKADDR_IN));
    }
    else if (addr->sa_family == AF_INET6)
    {
        memcpy(&relay_config->addr6, addr, sizeof(SOCKADDR_IN6_LH));
        memcpy(&relay_config->proxy6, proxy, sizeof(SOCKADDR_IN6_LH));
    }
    else
        goto fail;

    relay_config->auth = auth;
    if (auth) {
        wcscpy_s(relay_config->login, SOCKS_AUTH_MAX_SIZE, login);
        wcscpy_s(relay_config->pass, SOCKS_AUTH_MAX_SIZE, pass);
    }

    relay_config->listen_sock = __sys_socket(addr->sa_family, SOCK_STREAM, IPPROTO_TCP);
    if (relay_config->listen_sock == INVALID_SOCKET)
        goto fail;

    //DWORD off = 0; // enable IPv4 on IPV6
    //setsockopt(relay_config->listen_sock, /*IPPROTO_IPV6*/ 41, /*IPV6_V6ONLY*/ 27, (char*)&off, sizeof(off));

    if (addr->sa_family == AF_INET)
    {
        struct sockaddr_in addr4 = { 0 };
        addr4.sin_family = AF_INET;
        addr4.sin_addr.s_addr = _ntohl(0x7F000001); // 127.0.0.1
        addr4.sin_port = 0;
        if (__sys_bind(relay_config->listen_sock, (struct sockaddr*)&addr4, sizeof(addr4)) != 0)
            goto fail;
    }
    else if (addr->sa_family == AF_INET6)
    {
        struct sockaddr_in6 addr6 = { 0 };
        addr6.sin6_family = AF_INET6;
        addr6.sin6_addr = (struct in6_addr){{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}}; // ::1
        addr6.sin6_port = 0;
        if (__sys_bind(relay_config->listen_sock, (struct sockaddr*)&addr6, sizeof(addr6)) != 0)
            goto fail;
    }

    if (__sys_listen(relay_config->listen_sock, 1) != 0) // backlog = 1 we only look for one connection
        goto fail;

    struct sockaddr_in6 bound_addr;
    int addr_len = sizeof(bound_addr);
    if (__sys_getsockname(relay_config->listen_sock, (struct sockaddr*)&bound_addr, &addr_len) != 0)
        goto fail;

    CreateThread(NULL, 0, proxy_handle_relay, relay_config, 0, NULL);

    return bound_addr.sin6_port;

fail:

    if(relay_config->listen_sock != INVALID_SOCKET)
        __sys_closesocket(relay_config->listen_sock);

    if(relay_config)
        Dll_Free(relay_config);

    return 0;
}