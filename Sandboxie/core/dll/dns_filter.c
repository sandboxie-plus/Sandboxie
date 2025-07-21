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
// DNS Filter
//---------------------------------------------------------------------------

#define NOGDI
#include "dll.h"

#include <windows.h>
#include <wchar.h>
#include <oleauto.h>
#include <SvcGuid.h>
#include "common/my_wsa.h"
#include "common/netfw.h"
#include "common/map.h"
#include "wsa_defs.h"
#include "common/pattern.h"
#include "common/str_util.h"
#include "core/drv/api_defs.h"
#include "core/drv/verify.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static int WSA_WSALookupServiceBeginW(
    LPWSAQUERYSETW  lpqsRestrictions,
    DWORD           dwControlFlags,
    LPHANDLE        lphLookup);

static int WSA_WSALookupServiceNextW(
    HANDLE          hLookup,
    DWORD           dwControlFlags,
    LPDWORD         lpdwBufferLength,
    LPWSAQUERYSETW  lpqsResults);

static int WSA_WSALookupServiceEnd(HANDLE hLookup);


BOOLEAN WSA_GetIP(const short* addr, int addrlen, IP_ADDRESS* pIP);
void WSA_DumpIP(ADDRESS_FAMILY af, IP_ADDRESS* pIP, wchar_t* pStr);

//---------------------------------------------------------------------------


static P_WSALookupServiceBeginW __sys_WSALookupServiceBeginW = NULL;
static P_WSALookupServiceNextW __sys_WSALookupServiceNextW = NULL;
static P_WSALookupServiceEnd __sys_WSALookupServiceEnd = NULL;


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


extern POOL* Dll_Pool;

static LIST       WSA_FilterList;
static BOOLEAN    WSA_FilterEnabled = FALSE;

typedef struct _IP_ENTRY
{
    LIST_ELEM list_elem;

    USHORT Type;
    IP_ADDRESS IP;
} IP_ENTRY;

typedef struct _WSA_LOOKUP {
    LIST* pEntries;
    BOOLEAN NoMore;
    WCHAR* DomainName;       // Request Domain
    GUID* ServiceClassId;    // Request Class ID
    DWORD Namespace;         // Request Namespace
    BOOLEAN Filtered;        // Filter flag
} WSA_LOOKUP;

static HASH_MAP   WSA_LookupMap;

static BOOLEAN    WSA_DnsTraceFlag = FALSE;


//---------------------------------------------------------------------------
// WSA_GetLookup
//---------------------------------------------------------------------------


_FX WSA_LOOKUP* WSA_GetLookup(HANDLE h, BOOLEAN bCanAdd)
{
    WSA_LOOKUP* pLookup = (WSA_LOOKUP*)map_get(&WSA_LookupMap, h);
    if (pLookup == NULL && bCanAdd) {
        pLookup = (WSA_LOOKUP*)map_insert(&WSA_LookupMap, h, NULL, sizeof(WSA_LOOKUP));
        if (pLookup) {
            pLookup->pEntries = NULL;
            pLookup->NoMore = FALSE;
            pLookup->DomainName = NULL;
            pLookup->ServiceClassId = NULL;
            pLookup->Filtered = FALSE;
        }
    }
    return pLookup;
}

//---------------------------------------------------------------------------
// WSA_IsIPv6Query
//---------------------------------------------------------------------------

_FX BOOLEAN WSA_IsIPv6Query(LPGUID lpServiceClassId)
{
    if (lpServiceClassId) {
        if (memcmp(lpServiceClassId, &(GUID)SVCID_DNS_TYPE_AAAA, sizeof(GUID)) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

//---------------------------------------------------------------------------
// WSA_FillResponseStructure
//---------------------------------------------------------------------------

_FX BOOLEAN WSA_FillResponseStructure(
    WSA_LOOKUP* pLookup,
    LPWSAQUERYSETW lpqsResults,
    LPDWORD lpdwBufferLength)
{
    if (!pLookup || !pLookup->pEntries)
        return FALSE;

    if (!pLookup->DomainName)
        return FALSE;

    BOOLEAN isIPv6Query = WSA_IsIPv6Query(pLookup->ServiceClassId);

    // Calc buffer size needed
    DWORD neededSize = sizeof(WSAQUERYSETW);
    DWORD domainNameLen = (wcslen(pLookup->DomainName) + 1) * sizeof(WCHAR);
    neededSize += domainNameLen;

    // Calc IP size
    DWORD ipCount = 0;
    IP_ENTRY* entry;

    // Filter IP by type
    for (entry = (IP_ENTRY*)List_Head(pLookup->pEntries); entry; entry = (IP_ENTRY*)List_Next(entry)) {
        if ((isIPv6Query && entry->Type == AF_INET6) ||
            (!isIPv6Query && entry->Type == AF_INET)) {
            ipCount++;
        }
    }

    if (ipCount == 0) {
        SetLastError(WSA_E_NO_MORE);
        return FALSE;
    }

    DWORD csaddrSize = sizeof(CSADDR_INFO) * ipCount;
    neededSize += csaddrSize;

    DWORD sockaddrSize = 0;
    for (entry = (IP_ENTRY*)List_Head(pLookup->pEntries); entry; entry = (IP_ENTRY*)List_Next(entry)) {
        if ((isIPv6Query && entry->Type == AF_INET6) ||
            (!isIPv6Query && entry->Type == AF_INET)) {
            if (entry->Type == AF_INET)
                sockaddrSize += sizeof(SOCKADDR_IN) * 2;
            else if (entry->Type == AF_INET6)
                sockaddrSize += sizeof(SOCKADDR_IN6_LH) * 2;
        }
    }
    neededSize += sockaddrSize;

    // Buffer not enough, return error
    if (*lpdwBufferLength < neededSize) {
        *lpdwBufferLength = neededSize;
        SetLastError(WSAEFAULT);
        return FALSE;
    }

    memset(lpqsResults, 0, sizeof(WSAQUERYSETW));

    lpqsResults->dwSize = sizeof(WSAQUERYSETW);
    lpqsResults->dwNameSpace = pLookup->Namespace;

    BYTE* currentPtr = (BYTE*)lpqsResults + sizeof(WSAQUERYSETW);

    lpqsResults->lpszServiceInstanceName = (LPWSTR)currentPtr;
    wcscpy(lpqsResults->lpszServiceInstanceName, pLookup->DomainName);
    currentPtr += domainNameLen;

    lpqsResults->lpszQueryString = (LPWSTR)currentPtr;
    wcscpy(lpqsResults->lpszQueryString, pLookup->DomainName);
    currentPtr += domainNameLen;

    lpqsResults->dwNumberOfCsAddrs = ipCount;
    lpqsResults->lpcsaBuffer = (PCSADDR_INFO)currentPtr;
    currentPtr += csaddrSize;

    DWORD i = 0;
    for (entry = (IP_ENTRY*)List_Head(pLookup->pEntries); entry; entry = (IP_ENTRY*)List_Next(entry)) {
        if ((isIPv6Query && entry->Type != AF_INET6) ||
            (!isIPv6Query && entry->Type != AF_INET)) {
            continue;
        }

        PCSADDR_INFO csaInfo = &lpqsResults->lpcsaBuffer[i++];

        if (entry->Type == AF_INET) {
            SOCKADDR_IN* remoteAddr = (SOCKADDR_IN*)currentPtr;
            currentPtr += sizeof(SOCKADDR_IN);
            memset(remoteAddr, 0, sizeof(SOCKADDR_IN));

            remoteAddr->sin_family = AF_INET;
            remoteAddr->sin_port = 0x3500;
            remoteAddr->sin_addr.S_un.S_addr = entry->IP.Data32[3];

            csaInfo->RemoteAddr.lpSockaddr = (LPSOCKADDR)remoteAddr;
            csaInfo->RemoteAddr.iSockaddrLength = sizeof(SOCKADDR_IN);

            SOCKADDR_IN* localAddr = (SOCKADDR_IN*)currentPtr;
            currentPtr += sizeof(SOCKADDR_IN);
            memset(localAddr, 0, sizeof(SOCKADDR_IN));

            localAddr->sin_family = AF_INET;
            localAddr->sin_port = 0;
            localAddr->sin_addr.S_un.S_addr = 0;

            csaInfo->LocalAddr.lpSockaddr = (LPSOCKADDR)localAddr;
            csaInfo->LocalAddr.iSockaddrLength = sizeof(SOCKADDR_IN);


            csaInfo->iSocketType = SOCK_DGRAM;
            csaInfo->iProtocol = IPPROTO_UDP;
        }
        else if (entry->Type == AF_INET6) {
            SOCKADDR_IN6_LH* remoteAddr = (SOCKADDR_IN6_LH*)currentPtr;
            currentPtr += sizeof(SOCKADDR_IN6_LH);
            memset(remoteAddr, 0, sizeof(SOCKADDR_IN6_LH));

            remoteAddr->sin6_family = AF_INET6;
            remoteAddr->sin6_port = 0x3500;
            memcpy(remoteAddr->sin6_addr.u.Byte, entry->IP.Data, 16);

            csaInfo->RemoteAddr.lpSockaddr = (LPSOCKADDR)remoteAddr;
            csaInfo->RemoteAddr.iSockaddrLength = sizeof(SOCKADDR_IN6_LH);

            SOCKADDR_IN6_LH* localAddr = (SOCKADDR_IN6_LH*)currentPtr;
            currentPtr += sizeof(SOCKADDR_IN6_LH);
            memset(localAddr, 0, sizeof(SOCKADDR_IN6_LH));

            localAddr->sin6_family = AF_INET6;
            localAddr->sin6_port = 0;
            memset(localAddr->sin6_addr.u.Byte, 0, 16);

            csaInfo->LocalAddr.lpSockaddr = (LPSOCKADDR)localAddr;
            csaInfo->LocalAddr.iSockaddrLength = sizeof(SOCKADDR_IN6_LH);


            csaInfo->iSocketType = SOCK_RAW;
            // magic number returned by Windows
            csaInfo->iProtocol = 23;
        }
    }

    return TRUE;
}

//---------------------------------------------------------------------------
// WSA_InitNetDnsFilter
//---------------------------------------------------------------------------


_FX BOOLEAN WSA_InitNetDnsFilter(HMODULE module)
{
    P_WSALookupServiceBeginW WSALookupServiceBeginW;
    P_WSALookupServiceNextW WSALookupServiceNextW;
    P_WSALookupServiceEnd WSALookupServiceEnd;

    List_Init(&WSA_FilterList);

    //
    // Load filter rules
    //

    WCHAR conf_buf[256];
    for (ULONG index = 0; ; ++index) {

        NTSTATUS status = SbieApi_QueryConf(
            NULL, L"NetworkDnsFilter", index, conf_buf, sizeof(conf_buf) - 16 * sizeof(WCHAR));
        if (!NT_SUCCESS(status))
            break;

        ULONG level = -1;
        WCHAR* value = Config_MatchImageAndGetValue(conf_buf, Dll_ImageName, &level);
        if (!value)
            continue;

        WCHAR* domain_ip = wcschr(value, L':');
        if (domain_ip)
            *domain_ip++ = L'\0';

        PATTERN* pat = Pattern_Create(Dll_Pool, value, TRUE, level);

        if (domain_ip) {

            LIST* entries = (LIST*)Dll_Alloc(sizeof(LIST));
            List_Init(entries);

            BOOLEAN HasV6 = FALSE;

            const WCHAR* ip_value = domain_ip;
            ULONG ip_len = wcslen(domain_ip);
            for (const WCHAR* ip_end = ip_value + ip_len; ip_value < ip_end;) {
                const WCHAR* ip_str1;
                ULONG ip_len1;
                ip_value = SbieDll_GetTagValue(ip_value, ip_end, &ip_str1, &ip_len1, L';');

                IP_ENTRY* entry = (IP_ENTRY*)Dll_Alloc(sizeof(IP_ENTRY));
                if (_inet_xton(ip_str1, ip_len1, &entry->IP, &entry->Type) == 1) {
                    if (entry->Type == AF_INET6)
                        HasV6 = TRUE;
                    List_Insert_After(entries, NULL, entry);
                }
            }

            if (!HasV6) {

                //
                // when there are no IPv6 entries create mapped once from the v4 ips
                //

                for (IP_ENTRY* entry = (IP_ENTRY*)List_Head(entries); entry && entry->Type == AF_INET; entry = (IP_ENTRY*)List_Next(entry)) {

                    IP_ENTRY* entry6 = (IP_ENTRY*)Dll_Alloc(sizeof(IP_ENTRY));
                    entry6->Type = AF_INET6;

                    // IPv4 to IPv6 map
                    memset(entry6->IP.Data, 0, 10);
                    entry6->IP.Data[10] = 0xFF;
                    entry6->IP.Data[11] = 0xFF;
                    memcpy(&entry6->IP.Data[12], &entry->IP.Data32[3], 4);

                    List_Insert_After(entries, NULL, entry6);
                }
            }

            PVOID* aux = Pattern_Aux(pat);
            *aux = entries;
        }

        List_Insert_After(&WSA_FilterList, NULL, pat);
    }

    if (WSA_FilterList.count > 0) {

        WSA_FilterEnabled = TRUE;

        map_init(&WSA_LookupMap, Dll_Pool);

        SCertInfo CertInfo = { 0 };
        if (!NT_SUCCESS(SbieApi_QueryDrvInfo(-1, &CertInfo, sizeof(CertInfo))) || !(CertInfo.active && CertInfo.opt_net)) {

            const WCHAR* strings[] = { L"NetworkDnsFilter" , NULL };
            SbieApi_LogMsgExt(-1, 6009, strings);

            WSA_FilterEnabled = FALSE;
        }
    }

    //
    // Setup DNS hooks
    //

    WSALookupServiceBeginW = (P_WSALookupServiceBeginW)GetProcAddress(module, "WSALookupServiceBeginW");
    if (WSALookupServiceBeginW) {
        SBIEDLL_HOOK(WSA_, WSALookupServiceBeginW);
    }

    WSALookupServiceNextW = (P_WSALookupServiceNextW)GetProcAddress(module, "WSALookupServiceNextW");
    if (WSALookupServiceNextW) {
        SBIEDLL_HOOK(WSA_, WSALookupServiceNextW);
    }

    WSALookupServiceEnd = (P_WSALookupServiceEnd)GetProcAddress(module, "WSALookupServiceEnd");
    if (WSALookupServiceEnd) {
        SBIEDLL_HOOK(WSA_, WSALookupServiceEnd);
    }

    // If there are any DnsTrace options set, then output this debug string
    WCHAR wsTraceOptions[4];
    if (SbieApi_QueryConf(NULL, L"DnsTrace", 0, wsTraceOptions, sizeof(wsTraceOptions)) == STATUS_SUCCESS && wsTraceOptions[0] != L'\0')
        WSA_DnsTraceFlag = TRUE;

    return TRUE;
}


//---------------------------------------------------------------------------
// WSA_WSALookupServiceBeginW
//---------------------------------------------------------------------------


_FX int WSA_WSALookupServiceBeginW(
    LPWSAQUERYSETW  lpqsRestrictions,
    DWORD           dwControlFlags,
    LPHANDLE        lphLookup)
{
    if (WSA_FilterEnabled && lpqsRestrictions && lpqsRestrictions->lpszServiceInstanceName) {
        ULONG path_len = wcslen(lpqsRestrictions->lpszServiceInstanceName);
        WCHAR* path_lwr = (WCHAR*)Dll_AllocTemp((path_len + 4) * sizeof(WCHAR));
        wmemcpy(path_lwr, lpqsRestrictions->lpszServiceInstanceName, path_len);
        path_lwr[path_len] = L'\0';
        _wcslwr(path_lwr);

        PATTERN* found;
        if (Pattern_MatchPathList(path_lwr, path_len, &WSA_FilterList, NULL, NULL, NULL, &found) > 0) {
            HANDLE fakeHandle = (HANDLE)Dll_Alloc(sizeof(ULONG_PTR));
            if (!fakeHandle) {
                Dll_Free(path_lwr);
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return SOCKET_ERROR;
            }

            *lphLookup = fakeHandle;

            WSA_LOOKUP* pLookup = WSA_GetLookup(fakeHandle, TRUE);
            if (pLookup) {
                pLookup->Filtered = TRUE;

                pLookup->DomainName = Dll_Alloc((path_len + 1) * sizeof(WCHAR));
                if (pLookup->DomainName) {
                    wcscpy(pLookup->DomainName, lpqsRestrictions->lpszServiceInstanceName);
                }

                pLookup->Namespace = lpqsRestrictions->dwNameSpace;

                if (lpqsRestrictions->lpServiceClassId) {
                    pLookup->ServiceClassId = Dll_Alloc(sizeof(GUID));
                    if (pLookup->ServiceClassId) {
                        memcpy(pLookup->ServiceClassId, lpqsRestrictions->lpServiceClassId, sizeof(GUID));
                    }
                }

                PVOID* aux = Pattern_Aux(found);
                if (*aux)
                    pLookup->pEntries = (LIST*)*aux;
                else
                    pLookup->NoMore = TRUE;
            }

            if (WSA_DnsTraceFlag) {
                WCHAR ClsId[64] = { 0 };
                if (lpqsRestrictions->lpServiceClassId) {
                    Sbie_snwprintf(ClsId, 64, L" (ClsId: %08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX)",
                        lpqsRestrictions->lpServiceClassId->Data1, lpqsRestrictions->lpServiceClassId->Data2, lpqsRestrictions->lpServiceClassId->Data3,
                        lpqsRestrictions->lpServiceClassId->Data4[0], lpqsRestrictions->lpServiceClassId->Data4[1], lpqsRestrictions->lpServiceClassId->Data4[2], lpqsRestrictions->lpServiceClassId->Data4[3],
                        lpqsRestrictions->lpServiceClassId->Data4[4], lpqsRestrictions->lpServiceClassId->Data4[5], lpqsRestrictions->lpServiceClassId->Data4[6], lpqsRestrictions->lpServiceClassId->Data4[7]);
                }

                WCHAR msg[512];
                Sbie_snwprintf(msg, 512, L"DNS Request Intercepted: %s%s (NS: %d, Type: %s, Hdl: 0x%x) - Using filtered response",
                    lpqsRestrictions->lpszServiceInstanceName, ClsId, lpqsRestrictions->dwNameSpace,
                    WSA_IsIPv6Query(lpqsRestrictions->lpServiceClassId) ? L"IPv6" : L"IPv4", fakeHandle);
                SbieApi_MonitorPutMsg(MONITOR_DNS | MONITOR_DENY, msg);
            }

            Dll_Free(path_lwr);
            return NO_ERROR;
        }

        Dll_Free(path_lwr);
    }

    int ret = __sys_WSALookupServiceBeginW(lpqsRestrictions, dwControlFlags, lphLookup);

    if (WSA_DnsTraceFlag && lpqsRestrictions) {
        WCHAR ClsId[64] = { 0 };
        if (lpqsRestrictions->lpServiceClassId) {
            Sbie_snwprintf(ClsId, 64, L" (ClsId: %08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX)",
                lpqsRestrictions->lpServiceClassId->Data1, lpqsRestrictions->lpServiceClassId->Data2, lpqsRestrictions->lpServiceClassId->Data3,
                lpqsRestrictions->lpServiceClassId->Data4[0], lpqsRestrictions->lpServiceClassId->Data4[1], lpqsRestrictions->lpServiceClassId->Data4[2], lpqsRestrictions->lpServiceClassId->Data4[3],
                lpqsRestrictions->lpServiceClassId->Data4[4], lpqsRestrictions->lpServiceClassId->Data4[5], lpqsRestrictions->lpServiceClassId->Data4[6], lpqsRestrictions->lpServiceClassId->Data4[7]);
        }

        WCHAR msg[512];
        BOOLEAN isIPv6 = WSA_IsIPv6Query(lpqsRestrictions->lpServiceClassId);
        Sbie_snwprintf(msg, 512, L"DNS Request Begin: %s%s, NS: %d, Type: %s, Hdl: 0x%x, Err: %d)",
            lpqsRestrictions->lpszServiceInstanceName ? lpqsRestrictions->lpszServiceInstanceName : L"Unnamed",
            ClsId, lpqsRestrictions->dwNameSpace, isIPv6 ? L"IPv6" : L"IPv4",
            lphLookup ? *lphLookup : NULL, ret == SOCKET_ERROR ? GetLastError() : 0);
        SbieApi_MonitorPutMsg(MONITOR_DNS, msg);
    }

    return ret;
}


//---------------------------------------------------------------------------
// WSA_WSALookupServiceNextW
//---------------------------------------------------------------------------


_FX int WSA_WSALookupServiceNextW(
    HANDLE          hLookup,
    DWORD           dwControlFlags,
    LPDWORD         lpdwBufferLength,
    LPWSAQUERYSETW  lpqsResults)
{
    WSA_LOOKUP* pLookup = NULL;

    if (WSA_FilterEnabled) {
        pLookup = WSA_GetLookup(hLookup, FALSE);

        if (pLookup && pLookup->Filtered) {
            if (pLookup->NoMore || !pLookup->pEntries) {
                SetLastError(WSA_E_NO_MORE);
                return SOCKET_ERROR;
            }

            if (WSA_FillResponseStructure(pLookup, lpqsResults, lpdwBufferLength)) {
                pLookup->NoMore = TRUE;

                if (WSA_DnsTraceFlag) {
                    WCHAR msg[2048];
                    Sbie_snwprintf(msg, 512, L"DNS Filtered Response: %s (NS: %d, Type: %s, Hdl: 0x%x)",
                        pLookup->DomainName, lpqsResults->dwNameSpace,
                        WSA_IsIPv6Query(pLookup->ServiceClassId) ? L"IPv6" : L"IPv4", hLookup);

                    for (DWORD i = 0; i < lpqsResults->dwNumberOfCsAddrs; i++) {
                        IP_ADDRESS ip;
                        if (WSA_GetIP(lpqsResults->lpcsaBuffer[i].RemoteAddr.lpSockaddr,
                            lpqsResults->lpcsaBuffer[i].RemoteAddr.iSockaddrLength, &ip))
                            WSA_DumpIP(lpqsResults->lpcsaBuffer[i].RemoteAddr.lpSockaddr->sa_family, &ip, msg);
                    }

                    SbieApi_MonitorPutMsg(MONITOR_DNS, msg);
                }

                return NO_ERROR;
            }
            else {
                // GetLastError already set in WSA_FillResponseStructure
                return SOCKET_ERROR;
            }
        }

        if (pLookup && pLookup->NoMore) {

            SetLastError(WSA_E_NO_MORE);
            return SOCKET_ERROR;
        }
    }

    int ret = __sys_WSALookupServiceNextW(hLookup, dwControlFlags, lpdwBufferLength, lpqsResults);

    if (ret == NO_ERROR && pLookup && pLookup->pEntries) {

        //
        // This is a bit a simplified implementation, it assumes that all results are always of the same time
        // else it may truncate it early, also it can't return more results the have been found. 
        //

        if (lpqsResults->dwNumberOfCsAddrs > 0) {

            IP_ENTRY* entry = (IP_ENTRY*)List_Head(pLookup->pEntries);

            for (DWORD i = 0; i < lpqsResults->dwNumberOfCsAddrs; i++) {

                USHORT af = lpqsResults->lpcsaBuffer[i].RemoteAddr.lpSockaddr->sa_family;
                for (; entry && entry->Type != af; entry = (IP_ENTRY*)List_Next(entry)); // skip to an entry of the right type
                if (!entry) { // no more entries clear remaining results
                    lpqsResults->dwNumberOfCsAddrs = i;
                    break;
                }

                if (af == AF_INET6)
                    memcpy(((SOCKADDR_IN6_LH*)lpqsResults->lpcsaBuffer[i].RemoteAddr.lpSockaddr)->sin6_addr.u.Byte, entry->IP.Data, 16);
                else if (af == AF_INET)
                    ((SOCKADDR_IN*)lpqsResults->lpcsaBuffer[i].RemoteAddr.lpSockaddr)->sin_addr.S_un.S_addr = entry->IP.Data32[3];

                entry = (IP_ENTRY*)List_Next(entry);
            }
        }

        if (lpqsResults->lpBlob != NULL) {

            IP_ENTRY* entry = (IP_ENTRY*)List_Head(pLookup->pEntries);

            HOSTENT* hp = (HOSTENT*)lpqsResults->lpBlob->pBlobData;
            if (hp->h_addrtype == AF_INET6 || hp->h_addrtype == AF_INET) {

                for (PCHAR* Addr = (PCHAR*)(((UINT_PTR)hp->h_addr_list + (UINT_PTR)hp)); *Addr; Addr++) {

                    for (; entry && entry->Type != hp->h_addrtype; entry = (IP_ENTRY*)List_Next(entry)); // skip to an entry of the right type
                    if (!entry) { // no more entries clear remaining results
                        *Addr = 0;
                        continue;
                    }

                    PCHAR ptr = (PCHAR)(((UINT_PTR)*Addr + (UINT_PTR)hp));
                    if (hp->h_addrtype == AF_INET6)
                        memcpy(ptr, entry->IP.Data, 16);
                    else if (hp->h_addrtype == AF_INET)
                        *(DWORD*)ptr = entry->IP.Data32[3];

                    entry = (IP_ENTRY*)List_Next(entry);
                }
            }
        }

        pLookup->NoMore = TRUE;
    }

    if (WSA_DnsTraceFlag && ret == NO_ERROR) {
        WCHAR msg[2048];
        Sbie_snwprintf(msg, 512, L"DNS Request Found: %s (NS: %d, Hdl: 0x%x)",
            lpqsResults->lpszServiceInstanceName, lpqsResults->dwNameSpace, hLookup);

        for (DWORD i = 0; i < lpqsResults->dwNumberOfCsAddrs; i++) {
            IP_ADDRESS ip;
            if (WSA_GetIP(lpqsResults->lpcsaBuffer[i].RemoteAddr.lpSockaddr, lpqsResults->lpcsaBuffer[i].RemoteAddr.iSockaddrLength, &ip))
                WSA_DumpIP(lpqsResults->lpcsaBuffer[i].RemoteAddr.lpSockaddr->sa_family, &ip, msg);
        }

        if (lpqsResults->lpBlob != NULL) {

            HOSTENT* hp = (HOSTENT*)lpqsResults->lpBlob->pBlobData;
            if (hp->h_addrtype != AF_INET6 && hp->h_addrtype != AF_INET) {
                WSA_DumpIP(hp->h_addrtype, NULL, msg);
            }
            else if (hp->h_addr_list) {
                for (PCHAR* Addr = (PCHAR*)(((UINT_PTR)hp->h_addr_list + (UINT_PTR)hp)); *Addr; Addr++) {

                    PCHAR ptr = (PCHAR)(((UINT_PTR)*Addr + (UINT_PTR)hp));

                    IP_ADDRESS ip;
                    if (hp->h_addrtype == AF_INET6)
                        memcpy(ip.Data, ptr, 16);
                    else if (hp->h_addrtype == AF_INET)
                        ip.Data32[3] = *(DWORD*)ptr;
                    WSA_DumpIP(hp->h_addrtype, &ip, msg);
                }
            }
        }

        SbieApi_MonitorPutMsg(MONITOR_DNS, msg);
    }

    return ret;
}


//---------------------------------------------------------------------------
// WSA_WSALookupServiceEnd
//---------------------------------------------------------------------------


_FX int WSA_WSALookupServiceEnd(HANDLE hLookup)
{
    if (WSA_FilterEnabled) {
        WSA_LOOKUP* pLookup = WSA_GetLookup(hLookup, FALSE);

        if (pLookup && pLookup->Filtered) {
            if (pLookup->DomainName)
                Dll_Free(pLookup->DomainName);

            if (pLookup->ServiceClassId)
                Dll_Free(pLookup->ServiceClassId);

            map_remove(&WSA_LookupMap, hLookup);

            Dll_Free(hLookup);

            if (WSA_DnsTraceFlag) {
                WCHAR msg[256];
                Sbie_snwprintf(msg, 256, L"DNS Filtered Request End (Hdl: 0x%x)", hLookup);
                SbieApi_MonitorPutMsg(MONITOR_DNS, msg);
            }

            return NO_ERROR;
        }

        map_remove(&WSA_LookupMap, hLookup);
    }

    if (WSA_DnsTraceFlag) {
        WCHAR msg[256];
        Sbie_snwprintf(msg, 256, L"DNS Request End (Hdl: 0x%x)", hLookup);
        SbieApi_MonitorPutMsg(MONITOR_DNS, msg);
    }

    return __sys_WSALookupServiceEnd(hLookup);
}
