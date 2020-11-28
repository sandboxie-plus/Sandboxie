/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020 David Xanatos, xanasoft.com
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
#include "dll.h"
#include "core/svc/netapiwire.h"


//---------------------------------------------------------------------------
//
// Windows Home Network Configuration
//
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static HRESULT HNet_IcfOpenDynamicFwPort(void *p1, void *p2, void *p3);


//---------------------------------------------------------------------------


typedef HRESULT (*P_IcfOpenDynamicFwPort)(void *p1, void *p2, void *p3);


//---------------------------------------------------------------------------


static P_IcfOpenDynamicFwPort   __sys_IcfOpenDynamicFwPort  = NULL;


//---------------------------------------------------------------------------
// IcfOpenDynamicFwPort
//---------------------------------------------------------------------------


_FX HRESULT HNet_IcfOpenDynamicFwPort(void *p1, void *p2, void *p3)
{
    // bind (Winsock 2) calls WSPBind, then HNetCfg.IcfOpenDynamicFwPort.
    // IcfOpenDynamicFwPort tries to talk to the Firewall service,
    // which is probably only accessible as a COM object (as opposed to
    // a well known port name), so it fails.
    // Here we intercept IcfOpenDynamicFwPort and return a success status,
    // while not actually talking to the Windows Firewall

    return 0;
}


//---------------------------------------------------------------------------
// HNet_Init
//---------------------------------------------------------------------------


_FX BOOLEAN HNet_Init(HMODULE module)
{
    P_IcfOpenDynamicFwPort IcfOpenDynamicFwPort;

    IcfOpenDynamicFwPort = (P_IcfOpenDynamicFwPort)
        GetProcAddress(module, "IcfOpenDynamicFwPort");

    if (IcfOpenDynamicFwPort) {
        SBIEDLL_HOOK(HNet_,IcfOpenDynamicFwPort);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
//
// Windows Sockets 2
//
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define SOCKET_ERROR            (-1)

#define IOC_IN          0x80000000      /* copy in parameters */
#define _WSAIOW(x,y)                  (IOC_IN|(x)|(y))
#define IOC_WS2                       0x08000000
#define SIO_NSP_NOTIFY_CHANGE         _WSAIOW(IOC_WS2,25)

#define WSA_IO_PENDING          (ERROR_IO_PENDING)

#define AF_INET         2               /* internetwork: UDP, TCP, etc. */
#define SOCKET                  ULONG_PTR


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef void WSACOMPLETION;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static int WSA_WSANSPIoctl(
    HANDLE hLookup, DWORD dwControlCode,
    void *lpvInBuffer, DWORD cbInBuffer,
    void *lpvOutBuffer, DWORD cbOutBuffer,
    DWORD *lpcbBytesReturned,
    WSACOMPLETION *lpCompletion);

static int WSA_IsBlockedPort(const short *addr, int addrlen);

static int WSA_connect(SOCKET s, const void *name, int namelen);

static int WSA_WSAConnect(
    SOCKET s, const void *name, int namelen,
    void *lpCallerData, void *lpCalleeData, void *lpSQOS, void *lpGQOS);


//---------------------------------------------------------------------------


typedef int (*P_WSANSPIoctl)(
    HANDLE hLookup, DWORD dwControlCode,
    void *lpvInBuffer, DWORD cbInBuffer,
    void *lpvOutBuffer, DWORD cbOutBuffer,
    DWORD *lpcbBytesReturned, WSACOMPLETION *lpCompletion);

typedef int (*P_connect)(SOCKET s, const struct sockaddr *name, int namelen);

typedef int (*P_WSAConnect)(
    SOCKET s, const struct sockaddr *name, int namelen,
    void *lpCallerData, void *lpCalleeData, void *lpSQOS, void *lpGQOS);


//---------------------------------------------------------------------------


static P_WSANSPIoctl        __sys_WSANSPIoctl       = NULL;
static P_connect            __sys_connect           = NULL;
static P_WSAConnect         __sys_WSAConnect        = NULL;


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static ULONG_PTR *WSA_BlockedPorts    = NULL;
static ULONG      WSA_MaxBlockedPorts = 0;


//---------------------------------------------------------------------------
// WSANSPIoctl
//---------------------------------------------------------------------------


_FX int WSA_WSANSPIoctl(
    HANDLE hLookup, DWORD dwControlCode,
    void *lpvInBuffer, DWORD cbInBuffer,
    void *lpvOutBuffer, DWORD cbOutBuffer,
    DWORD *lpcbBytesReturned,
    WSACOMPLETION *lpCompletion)
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
// WSA_IsBlockedPort
//---------------------------------------------------------------------------


_FX int WSA_IsBlockedPort(const short *addr, int addrlen)
{
    if (addrlen >= sizeof(USHORT) * 2 && addr && addr[0] == AF_INET) {

        USHORT portnum = ((addr[1] & 0xFF) << 8) | ((addr[1] & 0xFF00) >> 8);
        ULONG  index = portnum / 512;
        if (index < WSA_MaxBlockedPorts) {

            UCHAR *bitmask = (UCHAR *)WSA_BlockedPorts[index];
            ULONG byteNum = (portnum % 512) / 8;
            ULONG bitNum  = (portnum % 512) % 8;
            if (bitmask[byteNum] & (1 << bitNum)) {

                SetLastError(WSAECONNREFUSED);
                return 1;
            }
        }
    }

    return 0;
}


//---------------------------------------------------------------------------
// WSA_connect
//---------------------------------------------------------------------------


_FX int WSA_connect(SOCKET s, const void *name, int namelen)
{
    if (WSA_IsBlockedPort(name, namelen))
        return SOCKET_ERROR;
    return __sys_connect(s, name, namelen);
}


//---------------------------------------------------------------------------
// WSA_WSAConnect
//---------------------------------------------------------------------------


_FX int WSA_WSAConnect(
    SOCKET s, const void *name, int namelen,
    void *lpCallerData, void *lpCalleeData, void *lpSQOS, void *lpGQOS)
{
    if (WSA_IsBlockedPort(name, namelen))
        return SOCKET_ERROR;
    return __sys_WSAConnect(
        s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS);
}


//---------------------------------------------------------------------------
// WSA_InitBlockedPorts_AddPort
//---------------------------------------------------------------------------


_FX void WSA_InitBlockedPorts_AddPort(
    USHORT port1, USHORT port2, BOOLEAN setOrClear)
{
    ULONG index;

    //{WCHAR txt[128];Sbie_snwprintf(txt, 128, L"Adding port range %d - %d as %d\n", port1, port2, setOrClear); OutputDebugString(txt); }

    index = port2 / 512;
    if (index >= WSA_MaxBlockedPorts) {

        ULONG_PTR *NewTable = Dll_Alloc(sizeof(ULONG_PTR) * (index + 1));
        memzero(NewTable, sizeof(ULONG_PTR) * (index + 1));
        if (WSA_BlockedPorts) {
            memcpy(NewTable, WSA_BlockedPorts,
                   WSA_MaxBlockedPorts * sizeof(ULONG_PTR));
        }
        WSA_BlockedPorts    = NewTable;
        WSA_MaxBlockedPorts = index + 1;

        for (index = 0; index < WSA_MaxBlockedPorts; ++index) {
            if (! WSA_BlockedPorts[index]) {
                UCHAR *bitmask = Dll_Alloc(64);
                memzero(bitmask, 64);
                WSA_BlockedPorts[index] = (ULONG_PTR)bitmask;
            }
        }
    }

    for (index = port1; index <= port2; ++index) {
        UCHAR *bitmask = (UCHAR *)WSA_BlockedPorts[index / 512];
        ULONG byteNum = (index % 512) / 8;
        ULONG bitNum  = (index % 512) % 8;
        if (setOrClear)
            bitmask[byteNum] |= (1 << bitNum);
        else
            bitmask[byteNum] &= ~(1 << bitNum);
    }
}


//---------------------------------------------------------------------------
// WSA_InitBlockedPorts
//---------------------------------------------------------------------------


_FX void WSA_InitBlockedPorts(void)
{
    ULONG index = 0;
    WCHAR *text = Dll_AllocTemp(1024 * sizeof(WCHAR));
    WCHAR *ptr, *ptr2;
    BOOLEAN setAll = FALSE;

    while (1) {

        BOOLEAN setOrClear = TRUE;

        NTSTATUS status = SbieApi_QueryConf(
            NULL, L"BlockPort", index, text, 1020 * sizeof(WCHAR));
        ++index;
        if (! NT_SUCCESS(status)) {
            if (status == STATUS_BUFFER_TOO_SMALL)
                continue;
            break;
        }

        text[1020] = L'\0';
        ptr = text;
        while (iswspace(*ptr))
            ++ptr;
        if (*ptr == L'*') {
            if (! setAll) {
                WSA_InitBlockedPorts_AddPort((USHORT)0, (USHORT)65535, TRUE);
                setAll = TRUE;
            }
            setOrClear = FALSE;
            ++ptr;
            while (iswspace(*ptr))
                ++ptr;
            if (*ptr != L',')
                continue;
            ++ptr;
        }

        while (1) {

            int port1, port2;

            while (iswspace(*ptr))
                ++ptr;
            port1 = wcstol(ptr, &ptr2, 10);
            if (ptr2 == ptr)
                break;
            ptr = ptr2;
            if (port1 < 0)
                port1 = 0;
            else if (port1 > 65535)
                port1 = 65535;

            port2 = port1;
            while (iswspace(*ptr))
                ++ptr;
            if (*ptr == L'-') {
                ++ptr;
                while (iswspace(*ptr))
                    ++ptr;
                port2 = wcstol(ptr, &ptr2, 10);
                if (ptr2 == ptr)
                    break;
                ptr = ptr2;
                if (port2 < 0)
                    port2 = 0;
                else if (port2 > 65535)
                    port2 = 65535;
                if (port2 < port1)
                    port2 = port1;
                while (*ptr >= L'0' && *ptr <= L'9')
                    ++ptr;
            }

            WSA_InitBlockedPorts_AddPort((USHORT)port1, (USHORT)port2, setOrClear);

            while (iswspace(*ptr))
                ++ptr;
            if (*ptr != L',')
                break;
            ++ptr;
        }
    }

#if 0
    for (index = 0; index < WSA_MaxBlockedPorts; ++index) {
        int other;
        Sbie_snwprintf(text, 1024, L"%04d - ", index);
        ptr = text + wcslen(text);
        for (other = 0; other < 64; ++other) {
            Sbie_snwprintf(ptr, 1024 - wcslen(text), L"%02X/", ((UCHAR *)(WSA_BlockedPorts[index]))[other]);
            ptr += wcslen(ptr);
        }
        OutputDebugString(text);
    }
#endif

	Dll_Free(text);
}


//---------------------------------------------------------------------------
// WSA_Init
//---------------------------------------------------------------------------


_FX BOOLEAN WSA_Init(HMODULE module)
{
    P_WSANSPIoctl       WSANSPIoctl;
    P_connect           connect;
    P_WSAConnect        WSAConnect;

    WSANSPIoctl = (P_WSANSPIoctl)GetProcAddress(module, "WSANSPIoctl");
    if (WSANSPIoctl) {
        SBIEDLL_HOOK(WSA_,WSANSPIoctl);
    }

    //
    // initialize the list of blocked ports and hook the connect functions
    //

    WSA_InitBlockedPorts();

    if (! Dll_SkipHook(L"wsaconn")) {

        connect = (P_connect)GetProcAddress(module, "connect");
        if (connect) {
            SBIEDLL_HOOK(WSA_,connect);
        }

        WSAConnect = (P_WSAConnect)GetProcAddress(module, "WSAConnect");
        if (WSAConnect) {
            SBIEDLL_HOOK(WSA_,WSAConnect);
        }
    }

    return TRUE;
}


//---------------------------------------------------------------------------
//
// Network Shares
//
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _USE_INFO_0 {
    WCHAR      *ui0_local;
    WCHAR      *ui0_remote;
} USE_INFO_0;

typedef struct _USE_INFO_1 {
    USE_INFO_0  ui1_ui0;
    WCHAR      *ui1_password;
    ULONG       ui1_status;
    ULONG       ui1_asg_type;
    ULONG       ui1_refcount;
    ULONG       ui1_usecount;
} USE_INFO_1;

typedef struct _USE_INFO_2 {
    USE_INFO_1  ui2_ui1;
    WCHAR      *ui2_username;
    WCHAR      *ui2_domainname;
} USE_INFO_2;

typedef struct _USE_INFO_3 {
    USE_INFO_2  ui3_ui2;
    ULONG       ui3_flags;
} USE_INFO_3;

typedef struct _USE_INFO_4 {
    USE_INFO_3  ui4_ui3;
    ULONG       ui4_auth_identity_length;
    UCHAR      *ui4_auth_identity;
} USE_INFO_4;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN NetApi_Hook_NetUseAdd(HMODULE module);

static ULONG NetApi_NetUseAdd(
    WCHAR *ServerName, ULONG Level, UCHAR *Buf, ULONG *ParmError);


//---------------------------------------------------------------------------


typedef ULONG (*P_NetUseAdd)(
    WCHAR *ServerName, ULONG Level, UCHAR *Buf, ULONG *ParmError);


//---------------------------------------------------------------------------
// NetApi_Init
//---------------------------------------------------------------------------


_FX BOOLEAN NetApi_Init(HMODULE module)
{
    if (Dll_OsBuild >= 2600 && Dll_OsBuild < 7600)  // XP, Vista
        return NetApi_Hook_NetUseAdd(module);
    else
        return TRUE;
}


//---------------------------------------------------------------------------
// NetApi_Init_WksCli
//---------------------------------------------------------------------------


_FX BOOLEAN NetApi_Init_WksCli(HMODULE module)
{
    if (Dll_OsBuild >= 7600)    // Windows 7
        return NetApi_Hook_NetUseAdd(module);
    else
        return TRUE;
}


//---------------------------------------------------------------------------
// NetApi_Hook_NetUseAdd
//---------------------------------------------------------------------------


_FX BOOLEAN NetApi_Hook_NetUseAdd(HMODULE module)
{
    P_NetUseAdd NetUseAdd;

    //
    // if \Device\Mup is closed, don't hook NetUseAdd, which would cause
    // use of the function to fail due to the use of RestrictToken in SbieSvc
    //

    if (1) {

        extern const WCHAR *File_Mup;
        ULONG mp_flags = SbieDll_MatchPath(L'f', File_Mup);
        if (PATH_IS_CLOSED(mp_flags))
            return TRUE;
    }

    //
    // otherwise hook NetUseAdd
    //

    NetUseAdd = (P_NetUseAdd)GetProcAddress(module, "NetUseAdd");
    if (NetUseAdd) {
        P_NetUseAdd __sys_NetUseAdd;
        SBIEDLL_HOOK(NetApi_,NetUseAdd);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// NetApi_NetUseAdd
//---------------------------------------------------------------------------


_FX ULONG NetApi_NetUseAdd(
    WCHAR *ServerName, ULONG Level, UCHAR *Buf, ULONG *ParmError)
{
    NETAPI_USE_ADD_REQ *req;
    NETAPI_USE_ADD_RPL *rpl;
    USE_INFO_0 *info0;
    USE_INFO_1 *info1;
    USE_INFO_2 *info2;
    USE_INFO_3 *info3;
    USE_INFO_4 *info4;
    ULONG err;
    ULONG drive_number;

    //
    // validate parameters and build a request packet
    //

    if (ServerName || (Level > 4)) {
        req = NULL;
        goto abort;
    }

    info0 = (USE_INFO_0 *)Buf;
    info1 = (USE_INFO_1 *)Buf;
    info2 = (USE_INFO_2 *)Buf;
    info3 = (USE_INFO_3 *)Buf;
    info4 = (USE_INFO_4 *)Buf;

    req = Dll_AllocTemp(sizeof(NETAPI_USE_ADD_REQ));
    req->h.length = sizeof(NETAPI_USE_ADD_REQ);
    req->h.msgid = MSGID_NETAPI_USE_ADD;

    req->level = (UCHAR)Level;

    if (req->level >= 0) {

        if (info0->ui0_local) {
            req->ui0_local_len = wcslen(info0->ui0_local);
            if (req->ui0_local_len > 256)
                goto abort;
            wmemcpy(req->ui0_local, info0->ui0_local,
                    req->ui0_local_len);
        } else
            req->ui0_local_len = -1;

        if (info0->ui0_remote) {
            req->ui0_remote_len = wcslen(info0->ui0_remote);
            if (req->ui0_remote_len > 256)
                goto abort;
            wmemcpy(req->ui0_remote, info0->ui0_remote,
                    req->ui0_remote_len);
        } else
            req->ui0_remote_len = -1;
    }

    if (req->level >= 1) {

        if (info1->ui1_password) {
            req->ui1_password_len = wcslen(info1->ui1_password);
            if (req->ui1_password_len > 256)
                goto abort;
            wmemcpy(req->ui1_password, info1->ui1_password,
                    req->ui1_password_len);
        } else
            req->ui1_password_len = -1;

        req->ui1_status   = info1->ui1_status;
        req->ui1_asg_type = info1->ui1_asg_type;
        req->ui1_refcount = info1->ui1_refcount;
        req->ui1_usecount = info1->ui1_usecount;
    }

    if (req->level >= 2) {

        if (info2->ui2_username) {
            req->ui2_username_len = wcslen(info2->ui2_username);
            if (req->ui2_username_len > 256)
                goto abort;
            wmemcpy(req->ui2_username, info2->ui2_username,
                    req->ui2_username_len);
        } else
            req->ui2_username_len = -1;

        if (info2->ui2_domainname) {
            req->ui2_domainname_len = wcslen(info2->ui2_domainname);
            if (req->ui2_domainname_len > 256)
                goto abort;
            wmemcpy(req->ui2_domainname, info2->ui2_domainname,
                    req->ui2_domainname_len);
        } else
            req->ui2_domainname_len = -1;
    }

    if (req->level >= 3) {

        req->ui3_flags = info3->ui3_flags;
    }

    if (req->level >= 4) {

        if (info4->ui4_auth_identity_length > 2048)
            goto abort;
        if (info4->ui4_auth_identity_length && info4->ui4_auth_identity) {
            req->ui4_auth_identity_length = info4->ui4_auth_identity_length;
            memcpy(req->ui4_auth_identity, info4->ui4_auth_identity,
                   req->ui4_auth_identity_length);
        } else
            req->ui4_auth_identity_length = -1;
    }

    //
    // send request to SbieSvc and examine reply
    // note that SbieSvc will call DefineDosDevice outside the sandbox,
    // see SbieSvc!NetApiServer::LaunchSlave
    //

    drive_number = 0;
    if (req->ui0_local_len != -1 && req->ui0_local_len >= 2 &&
            req->ui0_local[1] == L':') {
        WCHAR drive_letter = req->ui0_local[0];
        if (drive_letter >= L'a' && drive_letter <= L'z')
            drive_number = (drive_letter - L'a');
        else if (drive_letter >= L'A' && drive_letter <= L'Z')
            drive_number = (drive_letter - L'A');
    }

    rpl = (NETAPI_USE_ADD_RPL *)SbieDll_CallServer(&req->h);

    Dll_Free(req);

    if (! rpl)
        err = ERROR_NOT_ENOUGH_MEMORY;
    else {
        err = rpl->h.status;
        if (ParmError && err == ERROR_INVALID_PARAMETER &&
                rpl->h.length >= sizeof(NETAPI_USE_ADD_RPL))
            *ParmError = rpl->parm_index;
        Dll_Free(rpl);
    }

    if ((! err) && drive_number)
        SbieDll_DeviceChange(0xAA00 + drive_number, tzuk);

    return err;

abort:

    if (req)
        Dll_Free(req);
    SbieApi_Log(2205, L"NetUseAdd");
    return ERROR_ACCESS_DENIED;
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
