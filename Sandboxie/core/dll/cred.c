/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2023 David Xanatos, xanasoft.com
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
// Credentials
//---------------------------------------------------------------------------


#include "dll.h"
#include <stdio.h>
#include <wincred.h>
#define COBJMACROS
#include "core/dll/pstore.h"
#define INITGUID
#include <guiddef.h>
#include "advapi.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define CRED_TYPE_DELETED       0x40000000


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN Cred_PreparePStore(void);

static WCHAR *Cred_GetName(
    const WCHAR *DomainName, const WCHAR *TargetName, ULONG Type);

static void *Cred_Serialize(CREDENTIALW *cred, ULONG *out_len,
                            BOOLEAN OverrideLastWriteTime);

static BOOLEAN Cred_UnserializeHelper(void *in_cred, CREDENTIALW *cred);

static CREDENTIALW *Cred_Unserialize1(void *mrshcred);

static CREDENTIALW **Cred_UnserializeN(void **mrshcreds, ULONG *pCount);

static BOOL Cred_WriteItem(const WCHAR *name, void *data, ULONG len);

static BOOL Cred_CredWriteW(void *pCredential, ULONG Flags);
static BOOL Cred_CredWriteA(void *pCredential, ULONG Flags);

static BOOL Cred_CredReadW(
    const wchar_t *TargetName, ULONG Type, ULONG Flags, void **ppCredential);
static BOOL Cred_CredReadA(
    const char *TargetName, ULONG Type, ULONG Flags, void **ppCredential);

static BOOL Cred_CredWriteDomainCredentialsW(
    void *pTargetInfo, void *pCredential, ULONG Flags);
static BOOL Cred_CredWriteDomainCredentialsA(
    void *pTargetInfo, void *pCredential, ULONG Flags);

static BOOL Cred_CredReadDomainCredentialsW(
    void *pTargetInfo, ULONG Flags, ULONG *pCount, void ***ppCredentials);
static BOOL Cred_CredReadDomainCredentialsA(
    void *pTargetInfo, ULONG Flags, ULONG *pCount, void ***ppCredentials);

static BOOL Cred_CredGetTargetInfoW(
    void *pTargetName, ULONG Flags, void **pTargetInfo);
static BOOL Cred_CredGetTargetInfoA(
    void *pTargetName, ULONG Flags, void **pTargetInfo);

static BOOL Cred_CredRenameW(
    void *OldTargetName, void *NewTargetName, ULONG Type, ULONG Flags);
static BOOL Cred_CredRenameA(
    void *OldTargetName, void *NewTargetName, ULONG Type, ULONG Flags);

static BOOL Cred_CredDeleteW(const wchar_t *TargetName, ULONG Type, ULONG Flags);
static BOOL Cred_CredDeleteA(const char *TargetName, ULONG Type, ULONG Flags);

static BOOL Cred_CredEnumerateW(
    void *pFilter, ULONG Flags, ULONG *pCount, void ***ppCredentials);
static BOOL Cred_CredEnumerateA(
    void *pFilter, ULONG Flags, ULONG *pCount, void ***ppCredentials);


//---------------------------------------------------------------------------
// Prototypes
//---------------------------------------------------------------------------

typedef void (*P_CoTaskMemFree)(void *pv);

//---------------------------------------------------------------------------
// Pointers
//---------------------------------------------------------------------------


P_CredWrite              __sys_CredWriteA            = NULL;
P_CredWrite              __sys_CredWriteW            = NULL;

P_CredRead               __sys_CredReadA             = NULL;
P_CredRead               __sys_CredReadW             = NULL;

P_CredWriteDomainCredentials __sys_CredWriteDomainCredentialsW = NULL;
P_CredWriteDomainCredentials __sys_CredWriteDomainCredentialsA = NULL;

P_CredReadDomainCredentials __sys_CredReadDomainCredentialsW = NULL;
P_CredReadDomainCredentials __sys_CredReadDomainCredentialsA = NULL;

//P_CredGetTargetInfo      __sys_CredGetTargetInfoA    = NULL;
//P_CredGetTargetInfo      __sys_CredGetTargetInfoW    = NULL;

P_CredRename             __sys_CredRenameA           = NULL;
P_CredRename             __sys_CredRenameW           = NULL;

P_CredDelete             __sys_CredDeleteA           = NULL;
P_CredDelete             __sys_CredDeleteW           = NULL;

P_CredEnumerate          __sys_CredEnumerateA        = NULL;
P_CredEnumerate          __sys_CredEnumerateW        = NULL;

//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static IPStore *Cred_PStore = NULL;

static P_CoTaskMemFree Cred_CoTaskMemFree = NULL;

DEFINE_GUID(GUID_SANDBOXIE,
    0xBE6FB014L, 0x7308, 0x47F5, 0x85,0x16, 0x80,0x2E,0xBF,0x98,0x7F,0xD2);

static const WCHAR *Cred_SimpleCred = L"SimpleCred-";
static const WCHAR *Cred_DomainCred = L"DomainCred-";


//static BOOLEAN Cred_Trace = FALSE;

//---------------------------------------------------------------------------
// SBIEDLL_HOOK_CRED
//---------------------------------------------------------------------------


#define SBIEDLL_HOOK_CRED(proc)                                             \
    *(ULONG_PTR *)&__sys_##proc = (ULONG_PTR)Ldr_GetProcAddrNew(DllName_advapi32, L#proc, #proc); \
    if (*(ULONG_PTR *)&__sys_##proc) {                                      \
        *(ULONG_PTR *)&__sys_##proc = (ULONG_PTR)                           \
            SbieDll_Hook(#proc, __sys_##proc, Cred_##proc, module);         \
        if (! __sys_##proc) return FALSE;                                   \
    }


//---------------------------------------------------------------------------
// Cred_Init_AdvApi
//---------------------------------------------------------------------------


_FX BOOLEAN Cred_Init_AdvApi(HMODULE module)
{
    //
    // if OpenProtectedStorage or OpenCredentials is specified,
    // don't hook anything
    //

    ULONG mp_flags = SbieDll_MatchPath(L'i', L"\\RPC Control\\protected_storage");
    if (PATH_IS_OPEN(mp_flags))
        return TRUE;

    if (SbieApi_QueryConfBool(NULL, L"OpenCredentials", FALSE))
        return TRUE;

    //Cred_Trace = SbieApi_QueryConfBool(NULL, L"CredTrace", FALSE);

//    __sys_CredMarshalCredential = (P_CredMarshalCredential)
//        GetProcAddress(module, "CredMarshalCredential");

    //
    // otherwise hook Cred APIs
    //

    SBIEDLL_HOOK_CRED(CredWriteA);
    SBIEDLL_HOOK_CRED(CredWriteW);

    SBIEDLL_HOOK_CRED(CredReadA);
    SBIEDLL_HOOK_CRED(CredReadW);

    SBIEDLL_HOOK_CRED(CredWriteDomainCredentialsA);
    SBIEDLL_HOOK_CRED(CredWriteDomainCredentialsW);

    SBIEDLL_HOOK_CRED(CredReadDomainCredentialsA);
    SBIEDLL_HOOK_CRED(CredReadDomainCredentialsW);

    //SBIEDLL_HOOK_CRED(CredGetTargetInfoA);
    //SBIEDLL_HOOK_CRED(CredGetTargetInfoW);

    SBIEDLL_HOOK_CRED(CredRenameA);
    SBIEDLL_HOOK_CRED(CredRenameW);

    SBIEDLL_HOOK_CRED(CredDeleteA);
    SBIEDLL_HOOK_CRED(CredDeleteW);

    SBIEDLL_HOOK_CRED(CredEnumerateA);
    SBIEDLL_HOOK_CRED(CredEnumerateW);

    return TRUE;
}


//---------------------------------------------------------------------------
// Cred_PreparePStore
//---------------------------------------------------------------------------


_FX BOOLEAN Cred_PreparePStore(void)
{
    HMODULE module;

    if (Cred_PStore)
        return TRUE;

    module = LoadLibrary(DllName_ole32);
    if (module) {

        Cred_CoTaskMemFree =
            (P_CoTaskMemFree)GetProcAddress(module, "CoTaskMemFree");

        if (Cred_CoTaskMemFree) {

            Cred_PStore = SbieDll_InitPStore();
            if (Cred_PStore)
                return TRUE;
        }
    }

    SbieApi_Log(2213, NULL);
    SetLastError(ERROR_NO_SUCH_LOGON_SESSION);
    return FALSE;
}


//---------------------------------------------------------------------------
// Cred_GetName
//---------------------------------------------------------------------------


_FX WCHAR *Cred_GetName(
    const WCHAR *DomainName, const WCHAR *TargetName, ULONG Type)
{
    ULONG len;
    WCHAR *name;

    len = 24;
    if (DomainName)
        len += wcslen(DomainName);
    if (TargetName)
        //len += wcslen(TargetName) + 10;
        len += wcslen(TargetName);
    else
        TargetName = L"?";

    name = Dll_Alloc(len * sizeof(WCHAR));

    if (DomainName)
        Sbie_snwprintf(name, len, L"%s%s-%s", Cred_DomainCred, DomainName, TargetName);
    else
        //Sbie_snwprintf(name, len, L"%s%08X-%s", Cred_SimpleCred, Type, TargetName);
        Sbie_snwprintf(name, len, L"%s-%s", Cred_SimpleCred, TargetName);

    return name;
}


//---------------------------------------------------------------------------
// Cred_Serialize
//---------------------------------------------------------------------------


_FX void *Cred_Serialize(CREDENTIALW *cred, ULONG *out_len,
                         BOOLEAN OverrideLastWriteTime)
{
    ULONG len;
    ULONG i;
    void *buffer;
    UCHAR *ptr;
    FILETIME ft;

    //
    // compute length of credential
    //

    len = sizeof(ULONG)         // length prefix
        + sizeof(ULONG)         // version/eyecatcher
        + sizeof(CREDENTIALW);

    if (cred->TargetName)
        len += (wcslen(cred->TargetName) + 1) * sizeof(WCHAR);

    if (cred->Comment)
        len += (wcslen(cred->Comment) + 1) * sizeof(WCHAR);

    if (cred->TargetAlias)
        len += (wcslen(cred->TargetAlias) + 1) * sizeof(WCHAR);

    if (cred->UserName)
        len += (wcslen(cred->UserName) + 1) * sizeof(WCHAR);

    len += cred->CredentialBlobSize;

    for (i = 0; i < cred->AttributeCount; ++i) {
        len += sizeof(CREDENTIAL_ATTRIBUTEW);
        if (cred->Attributes[i].Keyword)
            len += (wcslen(cred->Attributes[i].Keyword) + 1) * sizeof(WCHAR);
        len += cred->Attributes[i].ValueSize;
    }

    //
    // build serialized block
    //

    buffer = Dll_Alloc(len);
    ptr = (UCHAR *)buffer;

    *(ULONG *)ptr = len;
    ptr += sizeof(ULONG);

    *(ULONG *)ptr = tzuk;
    ptr += sizeof(ULONG);

    *(ULONG *)ptr = cred->Flags;
    ptr += sizeof(ULONG);

    *(ULONG *)ptr = cred->Type;
    ptr += sizeof(ULONG);

    if (OverrideLastWriteTime)
        GetSystemTimeAsFileTime(&ft);
    else {
        ft.dwLowDateTime  = cred->LastWritten.dwLowDateTime;
        ft.dwHighDateTime = cred->LastWritten.dwHighDateTime;
    }
    *(ULONG *)ptr = ft.dwLowDateTime;
    ptr += sizeof(ULONG);
    *(ULONG *)ptr = ft.dwHighDateTime;
    ptr += sizeof(ULONG);

    *(ULONG *)ptr = cred->Persist;
    ptr += sizeof(ULONG);

    if (cred->TargetName) {
        *(ULONG *)ptr = wcslen(cred->TargetName);
        ptr += sizeof(ULONG);
        wcscpy((WCHAR *)ptr, cred->TargetName);
        ptr += wcslen((WCHAR *)ptr) * sizeof(WCHAR);
    } else {
        *(ULONG *)ptr = 0;
        ptr += sizeof(ULONG);
    }

    if (cred->Comment) {
        *(ULONG *)ptr = wcslen(cred->Comment);
        ptr += sizeof(ULONG);
        wcscpy((WCHAR *)ptr, cred->Comment);
        ptr += wcslen((WCHAR *)ptr) * sizeof(WCHAR);
    } else {
        *(ULONG *)ptr = 0;
        ptr += sizeof(ULONG);
    }

    if (cred->TargetAlias) {
        *(ULONG *)ptr = wcslen(cred->TargetAlias);
        ptr += sizeof(ULONG);
        wcscpy((WCHAR *)ptr, cred->TargetAlias);
        ptr += wcslen((WCHAR *)ptr) * sizeof(WCHAR);
    } else {
        *(ULONG *)ptr = 0;
        ptr += sizeof(ULONG);
    }

    if (cred->UserName) {
        *(ULONG *)ptr = wcslen(cred->UserName);
        ptr += sizeof(ULONG);
        wcscpy((WCHAR *)ptr, cred->UserName);
        ptr += wcslen((WCHAR *)ptr) * sizeof(WCHAR);
    } else {
        *(ULONG *)ptr = 0;
        ptr += sizeof(ULONG);
    }

    *(ULONG *)ptr = cred->CredentialBlobSize;
    ptr += sizeof(ULONG);
    if (cred->CredentialBlobSize) {
        memcpy(ptr, cred->CredentialBlob, cred->CredentialBlobSize);
        ptr += cred->CredentialBlobSize;
    }

    *(ULONG *)ptr = cred->AttributeCount;
    ptr += sizeof(ULONG);

    for (i = 0; i < cred->AttributeCount; ++i) {

        *(ULONG *)ptr = cred->Attributes[i].Flags;
        ptr += sizeof(ULONG);

        if (cred->Attributes[i].Keyword) {
            *(ULONG *)ptr = wcslen(cred->Attributes[i].Keyword);
            ptr += sizeof(ULONG);
            wcscpy((WCHAR *)ptr, cred->Attributes[i].Keyword);
            ptr += wcslen((WCHAR *)ptr) * sizeof(WCHAR);
        } else {
            *(ULONG *)ptr = 0;
            ptr += sizeof(ULONG);
        }

        *(ULONG *)ptr = cred->Attributes[i].ValueSize;
        ptr += sizeof(ULONG);
        if (cred->Attributes[i].ValueSize) {
            memcpy(ptr, cred->Attributes[i].Value,
                        cred->Attributes[i].ValueSize);
            ptr += cred->Attributes[i].ValueSize;
        }
    }

    //
    // finish
    //

    *out_len = len;
    return buffer;
}


//---------------------------------------------------------------------------
// Cred_UnserializeHelper
//---------------------------------------------------------------------------


_FX BOOLEAN Cred_UnserializeHelper(void *in_cred, CREDENTIALW *cred)
{
    UCHAR *ptr;
    WCHAR *ptr2;
    ULONG len2;
    ULONG i;

    ptr = (UCHAR *)in_cred;
    ptr += sizeof(ULONG);

    if (*(ULONG *)ptr != tzuk)
        return FALSE;
    ptr += sizeof(ULONG);

    ptr2 = (WCHAR *)((UCHAR *)cred + sizeof(CREDENTIALW));

    cred->Flags = *(ULONG *)ptr;
    ptr += sizeof(ULONG);

    cred->Type = *(ULONG *)ptr;
    ptr += sizeof(ULONG);

    cred->LastWritten.dwLowDateTime = *(ULONG *)ptr;
    ptr += sizeof(ULONG);
    cred->LastWritten.dwHighDateTime = *(ULONG *)ptr;
    ptr += sizeof(ULONG);

    cred->Persist = *(ULONG *)ptr;
    ptr += sizeof(ULONG);

    len2 = *(ULONG *)ptr;
    ptr += sizeof(ULONG);
    if (len2) {
        cred->TargetName = ptr2;
        wmemcpy(ptr2, (WCHAR*)ptr, len2);
        ptr += len2 * sizeof(WCHAR);
        ptr2 += len2;
        *ptr2 = L'\0';
        ++ptr2;
    } else
        cred->TargetName = NULL;

    len2 = *(ULONG *)ptr;
    ptr += sizeof(ULONG);
    if (len2) {
        cred->Comment = ptr2;
        wmemcpy(ptr2, (WCHAR*)ptr, len2);
        ptr += len2 * sizeof(WCHAR);
        ptr2 += len2;
        *ptr2 = L'\0';
        ++ptr2;
    } else
        cred->Comment = NULL;

    len2 = *(ULONG *)ptr;
    ptr += sizeof(ULONG);
    if (len2) {
        cred->TargetAlias = ptr2;
        wmemcpy(ptr2, (WCHAR*)ptr, len2);
        ptr += len2 * sizeof(WCHAR);
        ptr2 += len2;
        *ptr2 = L'\0';
        ++ptr2;
    } else
        cred->TargetAlias = NULL;

    len2 = *(ULONG *)ptr;
    ptr += sizeof(ULONG);
    if (len2) {
        cred->UserName = ptr2;
        wmemcpy(ptr2, (WCHAR*)ptr, len2);
        ptr += len2 * sizeof(WCHAR);
        ptr2 += len2;
        *ptr2 = L'\0';
        ++ptr2;
    } else
        cred->UserName = NULL;

    len2 = *(ULONG *)ptr;
    ptr += sizeof(ULONG);
    cred->CredentialBlobSize = len2;
    if (len2) {
        cred->CredentialBlob = (UCHAR *)ptr2;
        memcpy((UCHAR *)ptr2, ptr, len2);
        ptr += len2;
        ptr2 = (WCHAR *)((UCHAR *)ptr2 + len2);
    } else
        cred->CredentialBlob = NULL;

    cred->AttributeCount = *(ULONG *)ptr;
    ptr += sizeof(ULONG);
    if (cred->AttributeCount) {
        cred->Attributes = (CREDENTIAL_ATTRIBUTEW *)ptr2;
        ptr2 = (WCHAR *)((UCHAR *)ptr2 +
            cred->AttributeCount * sizeof(CREDENTIAL_ATTRIBUTEW));

        for (i = 0; i < cred->AttributeCount; ++i) {

            cred->Attributes[i].Flags = *(ULONG *)ptr;
            ptr += sizeof(ULONG);

            len2 = *(ULONG *)ptr;
            ptr += sizeof(ULONG);
            if (len2) {
                cred->Attributes[i].Keyword = ptr2;
                wmemcpy(ptr2, (WCHAR*)ptr, len2);
                ptr += len2 * sizeof(WCHAR);;
                ptr2 += len2;
                *ptr2 = L'\0';
                ++ptr2;
            } else
                cred->Attributes[i].Keyword = NULL;

            len2 = *(ULONG *)ptr;
            ptr += sizeof(ULONG);
            cred->Attributes[i].ValueSize = len2;
            if (len2) {
                cred->Attributes[i].Value = (UCHAR *)ptr2;
                memcpy((UCHAR *)ptr2, ptr, len2);
                ptr += len2;
                ptr2 = (WCHAR *)((UCHAR *)ptr2 + len2);
            } else
                cred->Attributes[i].Value = NULL;
        }

    } else
        cred->Attributes = NULL;

    return TRUE;
}


//---------------------------------------------------------------------------
// Cred_Unserialize1
//---------------------------------------------------------------------------


_FX CREDENTIALW *Cred_Unserialize1(void *mrshcred)
{
    ULONG len;
    CREDENTIALW *cred;

    len = *(ULONG *)mrshcred;
    cred = LocalAlloc(LMEM_FIXED, len);
    if (cred) {
        BOOLEAN ok = Cred_UnserializeHelper(mrshcred, cred);
        if (! ok) {
            LocalFree(cred);
            cred = NULL;
        }
    }

    return cred;
}


//---------------------------------------------------------------------------
// Cred_UnserializeN
//---------------------------------------------------------------------------


_FX CREDENTIALW **Cred_UnserializeN(void **mrshcreds, ULONG *pCount)
{
    ULONG len;
    ULONG i, j;
    CREDENTIALW **creds;
    UCHAR *ptr1, *ptr2;
    void *mrshcred;

    len = 0;
    for (i = 0; mrshcreds[i]; ++i) {
        mrshcred = mrshcreds[i];
        len += *(ULONG *)mrshcred;
    }
    len += sizeof(ULONG_PTR) * i;

    creds = LocalAlloc(LMEM_FIXED, len);
    if (! creds)
        return NULL;

    ptr1 = (UCHAR *)creds;
    ptr2 = ptr1 + sizeof(ULONG_PTR) * i;

    memzero(ptr1, i * sizeof(ULONG_PTR));
    *pCount = 0;

    for (j = 0; j < i; ++j) {

        BOOLEAN ok;

        mrshcred = mrshcreds[j];
        len = *(ULONG *)mrshcred;
        ok = Cred_UnserializeHelper(mrshcred, (CREDENTIALW *)ptr2);
        if (ok) {

            CREDENTIALW *newCred = (CREDENTIALW *)ptr2;
            ULONG_PTR *ptr0 = (ULONG_PTR *)creds;
            while ((UCHAR *)ptr0 != ptr1) {
                CREDENTIALW *oldCred = *(CREDENTIALW **)ptr0;
                if (_wcsicmp(oldCred->TargetName, newCred->TargetName) == 0)
                    ok = FALSE;
                ++ptr0;
            }

            if (ok) {

                if (newCred->LastWritten.dwLowDateTime == 0 &&
                    newCred->LastWritten.dwHighDateTime == 0)
                    GetSystemTimeAsFileTime(&newCred->LastWritten);

                *(ULONG_PTR *)ptr1 = (ULONG_PTR)ptr2;
                ptr1 += sizeof(ULONG_PTR);
                ptr2 += len;
                *pCount = *pCount + 1;
            }
        }
    }

    return creds;
}


//---------------------------------------------------------------------------
// Cred_WriteItem
//---------------------------------------------------------------------------


_FX BOOL Cred_WriteItem(const WCHAR *name, void *data, ULONG len)
{
    HRESULT hr;

    hr = IPStore_WriteItem(
        Cred_PStore, PST_KEY_CURRENT_USER,
        &GUID_SANDBOXIE, &GUID_SANDBOXIE, name,
        len, data, NULL, 0, 0);

    if (FAILED(hr)) {

        static const WCHAR *Sandboxie = L"Sandboxie";
        PST_TYPEINFO typeinfo;

        typeinfo.cbSize = sizeof(typeinfo);
        typeinfo.szDisplayName = (WCHAR *)Sandboxie;
        hr = IPStore_CreateType(
            Cred_PStore, PST_KEY_CURRENT_USER,
            &GUID_SANDBOXIE, &typeinfo, 0);

        hr = IPStore_CreateSubtype(
            Cred_PStore, PST_KEY_CURRENT_USER,
            &GUID_SANDBOXIE, &GUID_SANDBOXIE, &typeinfo, NULL, 0);

        hr = IPStore_WriteItem(
            Cred_PStore, PST_KEY_CURRENT_USER,
            &GUID_SANDBOXIE, &GUID_SANDBOXIE, name,
            len, data, NULL, 0, 0);
    }

    if (FAILED(hr)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    SetLastError(ERROR_SUCCESS);
    return TRUE;
}


//---------------------------------------------------------------------------
// Cred_CredWriteW
//---------------------------------------------------------------------------


_FX BOOL Cred_CredWriteW(void *pCredential, ULONG Flags)
{
    CREDENTIALW *cred = (CREDENTIALW *)pCredential;
    void *mrshcred;
    ULONG mrshcred_len;
    WCHAR *name;
    ULONG err;
    BOOL ok;

    if (! Cred_PreparePStore())
        return FALSE;

    mrshcred = Cred_Serialize(cred, &mrshcred_len, TRUE);

    if (! mrshcred) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    /*if (Cred_Trace) {
        WCHAR msg[1024];
        Sbie_snwprintf(msg, 1024, L"CredWriteW: %s (%d)", cred->TargetName, cred->Type);
        SbieApi_MonitorPutMsg(MONITOR_OTHER | MONITOR_TRACE, msg);
    }*/

    name = Cred_GetName(NULL, cred->TargetName, cred->Type);

    ok = Cred_WriteItem(name, mrshcred, mrshcred_len);
    err = GetLastError();

    Dll_Free(name);
    Dll_Free(mrshcred);

    SetLastError(err);
    return ok;
}


//---------------------------------------------------------------------------
// Cred_CredReadW
//---------------------------------------------------------------------------


_FX BOOL Cred_CredReadW(
    const wchar_t* TargetName, ULONG Type, ULONG Flags, void** ppCredential)
{
    HRESULT hr;
    WCHAR *name;
    UCHAR *mrshcred;
    ULONG mrshcred_len;
    ULONG err;
    BOOL ok;

    if (! Cred_PreparePStore())
        return FALSE;

    name = Cred_GetName(NULL, TargetName, Type);

    hr = IPStore_ReadItem(
        Cred_PStore, PST_KEY_CURRENT_USER,
        &GUID_SANDBOXIE, &GUID_SANDBOXIE, name,
        &mrshcred_len, &mrshcred, NULL, 0);

    Dll_Free(name);

    if (FAILED(hr) || mrshcred_len <= sizeof(ULONG)) {

        if (SUCCEEDED(hr))
            Cred_CoTaskMemFree(mrshcred);

        ok = __sys_CredReadW(TargetName, Type, Flags, ppCredential);
    }
    else {

        *ppCredential = Cred_Unserialize1(mrshcred);

        Cred_CoTaskMemFree(mrshcred);

        if (*ppCredential) {
            err = 0;
            ok = TRUE;
        }
        else {
            err = ERROR_NOT_FOUND;
            ok = FALSE;
        }

        SetLastError(err);
    }

    /*if (Cred_Trace) {
        WCHAR msg[1024];
        Sbie_snwprintf(msg, 1024, L"CredReadW: %s (%d) = %d", TargetName, Type, FAILED(hr) ? ok : (ok ? 2 : -1));
        SbieApi_MonitorPutMsg(MONITOR_OTHER | MONITOR_TRACE, msg);
    }*/

    return ok;
}


//---------------------------------------------------------------------------
// Cred_CredWriteDomainCredentialsW
//---------------------------------------------------------------------------


_FX BOOL Cred_CredWriteDomainCredentialsW(
    void *pTargetInfo, void *pCredential, ULONG Flags)
{
    CREDENTIAL_TARGET_INFORMATIONW *TargetInfo =
        (CREDENTIAL_TARGET_INFORMATIONW *)pTargetInfo;
    CREDENTIALW *cred = (CREDENTIALW *)pCredential;
    void *mrshcred;
    ULONG mrshcred_len;
    ULONG i;
    ULONG err;
    BOOL ok;

    if (! Cred_PreparePStore())
        return FALSE;

    mrshcred = Cred_Serialize(cred, &mrshcred_len, TRUE);

    if (! mrshcred) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    ok = TRUE;

    for (i = 0; i < 5 && ok; ++i) {

        const WCHAR *DomainName;
        if (i == 0)
            DomainName = TargetInfo->NetbiosServerName;
        else if (i == 1)
            DomainName = TargetInfo->DnsServerName;
        else if (i == 2)
            DomainName = TargetInfo->NetbiosDomainName;
        else if (i == 3)
            DomainName = TargetInfo->DnsDomainName;
        else if (i == 4)
            DomainName = TargetInfo->DnsTreeName;

        if (DomainName) {

            WCHAR *name = Cred_GetName(
                                DomainName, TargetInfo->TargetName, 0);

            ok = Cred_WriteItem(name, mrshcred, mrshcred_len);
            err = GetLastError();

            Dll_Free(name);
        }
    }

    Dll_Free(mrshcred);

    SetLastError(err);
    return ok;
}


//---------------------------------------------------------------------------
// Cred_CredReadDomainCredentialsW
//---------------------------------------------------------------------------


_FX BOOL Cred_CredReadDomainCredentialsW(
    void *pTargetInfo, ULONG Flags, ULONG *pCount, void ***ppCredentials)
{
    CREDENTIAL_TARGET_INFORMATIONW *TargetInfo =
        (CREDENTIAL_TARGET_INFORMATIONW *)pTargetInfo;
    HRESULT hr;
    ULONG i;
    ULONG err;
    BOOL ok;

    if (! Cred_PreparePStore())
        return FALSE;

    ok = FALSE;

    for (i = 0; i < 5 && (! ok); ++i) {

        const WCHAR *DomainName;
        if (i == 0)
            DomainName = TargetInfo->NetbiosServerName;
        else if (i == 1)
            DomainName = TargetInfo->DnsServerName;
        else if (i == 2)
            DomainName = TargetInfo->NetbiosDomainName;
        else if (i == 3)
            DomainName = TargetInfo->DnsDomainName;
        else if (i == 4)
            DomainName = TargetInfo->DnsTreeName;

        if (DomainName) {

            UCHAR *mrshcred;
            ULONG mrshcred_len;
            WCHAR *name = Cred_GetName(
                                DomainName, TargetInfo->TargetName, 0);

            hr = IPStore_ReadItem(
                Cred_PStore, PST_KEY_CURRENT_USER,
                &GUID_SANDBOXIE, &GUID_SANDBOXIE, name,
                &mrshcred_len, &mrshcred, NULL, 0);

            Dll_Free(name);

            if (SUCCEEDED(hr)) {

                void *mrshcreds[2];
                mrshcreds[0] = mrshcred;
                mrshcreds[1] = NULL;
                *ppCredentials = Cred_UnserializeN(mrshcreds, pCount);

                Cred_CoTaskMemFree(mrshcred);

                if (*ppCredentials) {
                    err = 0;
                    ok = TRUE;
                } else {
                    err = ERROR_NOT_FOUND;
                    ok = FALSE;
                }
            }
        }
    }

    if (! ok) {
        return __sys_CredReadDomainCredentialsW(
                                pTargetInfo, Flags, pCount, ppCredentials);
    }

    SetLastError(ERROR_SUCCESS);
    return TRUE;
}


//---------------------------------------------------------------------------
// Cred_CredDeleteW
//---------------------------------------------------------------------------


_FX BOOL Cred_CredDeleteW(const wchar_t *TargetName, ULONG Type, ULONG Flags)
{
    WCHAR *name;
    ULONG zero;
    CREDENTIALW *cred;

    if (!Cred_PreparePStore()) {
        SetLastError(ERROR_NOT_FOUND);
        return FALSE;
    }

    if (Cred_CredReadW(TargetName, Type, Flags, &cred)) {
        if (!cred) {
            SetLastError(ERROR_NOT_FOUND);
            return FALSE;
        }
        if (cred->CredentialBlobSize == sizeof(ULONG) && *(ULONG *)cred->CredentialBlob == 0) {
            LocalFree(cred);
            SetLastError(ERROR_NOT_FOUND);
            return FALSE;
        }
    }
    else {
        SetLastError(ERROR_NOT_FOUND);
        return FALSE;
    }
    LocalFree(cred);
    zero = 0;

    name = Cred_GetName(NULL, TargetName, Type);

    if (!Cred_WriteItem(name, &zero, sizeof(ULONG))) {
        Dll_Free(name);
        SetLastError(ERROR_NOT_FOUND);
        return FALSE;
    }
    Dll_Free(name);
    return TRUE;
}


//---------------------------------------------------------------------------
// Cred_CredEnumerateW
//---------------------------------------------------------------------------


_FX BOOL Cred_CredEnumerateW(
    void *pFilter, ULONG Flags, ULONG *pCount, void ***ppCredentials)
{
    HRESULT hr;
    IEnumPStoreItems *pEnum;
    void **mrshcreds;
    ULONG num_mrshcreds;
    ULONG len;
    ULONG i;
    ULONG err;
    BOOL ok;
    BOOLEAN exact;

    if (! Cred_PreparePStore())
        return FALSE;

    exact = FALSE;
    if (pFilter) {
        len = wcslen(pFilter);
        if (len && ((WCHAR *)pFilter)[len - 1] == L'*')
            --len;
        else
            exact = TRUE;
    } else
        len = 0;

    mrshcreds = Dll_Alloc(sizeof(ULONG_PTR) * 512);
    num_mrshcreds = 0;

    hr = IPStore_EnumItems(
        Cred_PStore, PST_KEY_CURRENT_USER,
        &GUID_SANDBOXIE, &GUID_SANDBOXIE, 0, &pEnum);

    if (SUCCEEDED(hr)) {

        WCHAR *name = NULL;
        ULONG fetched;
        UCHAR *mrshcred;
        ULONG mrshcred_len;

        while (1) {

            if (name) {
                Cred_CoTaskMemFree(name);
                name = NULL;
            }

            hr = IEnumPStoreItems_Next(pEnum, 1, &name, &fetched);
            if (hr != S_OK)
                break;

            if (wcsncmp(name, Cred_SimpleCred, 11) != 0)
                continue;

            if (len) {

                // name+20 is past the SimpleCred-12345678- prefix

                if (_wcsnicmp(name + 20, pFilter, len) != 0)
                    continue;

                if (exact && *(name + 20 + len) != L'\0')
                    continue;
            }

            hr = IPStore_ReadItem(
                Cred_PStore, PST_KEY_CURRENT_USER,
                &GUID_SANDBOXIE, &GUID_SANDBOXIE, name,
                &mrshcred_len, &mrshcred, NULL, 0);

            if (SUCCEEDED(hr) && num_mrshcreds < 510) {

                mrshcreds[num_mrshcreds] = mrshcred;
                ++num_mrshcreds;
            }
        }

        if (name)
            Cred_CoTaskMemFree(name);
    }

    ok = __sys_CredEnumerateW(pFilter, Flags, pCount, ppCredentials);
    err = GetLastError();

    if (num_mrshcreds) {

        if (ok) {

            void **in_creds = *ppCredentials;
            for (i = 0; i < *pCount; ++i) {
                if (num_mrshcreds < 510) {
                    mrshcreds[num_mrshcreds] =
                                    Cred_Serialize(in_creds[i], &len, FALSE);
                    ++num_mrshcreds;
                }
            }

            LocalFree(in_creds);
        }

        mrshcreds[num_mrshcreds] = NULL;
        *ppCredentials = Cred_UnserializeN(mrshcreds, pCount);

        if (*ppCredentials) {
            err = 0;
            ok = TRUE;
        } else {
            err = ERROR_NOT_FOUND;
            ok = FALSE;
        }
    }

    for (i = 0; i < num_mrshcreds; ++i)
        LocalFree(mrshcreds[i]);
    Dll_Free(mrshcreds);

    SetLastError(err);
    return ok;
}


//---------------------------------------------------------------------------
// Cred_CopyW2A
//---------------------------------------------------------------------------


_FX char* Cred_CopyW2A(char** pStrA, const WCHAR* strW)
{
    if (!strW)
        return NULL;
    ULONG i = 0;
    WCHAR* strA = *pStrA;
    for (; strW[i]; i++)
        strA[i] = strW[i];
    strA[i++] = '\0';
    *pStrA += i;
    return strA;
}


//---------------------------------------------------------------------------
// Cred_StrASizeW
//---------------------------------------------------------------------------


_FX size_t Cred_StrASizeW(const WCHAR* strW)
{
    if (!strW)
        return 0;
    return wcslen(strW) + 1;
}


//---------------------------------------------------------------------------
// Cred_CopyA2W
//---------------------------------------------------------------------------


_FX WCHAR* Cred_CopyA2W(WCHAR** pStrW, const char* strA)
{
    if (!strA)
        return NULL;
    ULONG i = 0;
    WCHAR* strW = *pStrW;
    for (; strA[i]; i++)
        strW[i] = strA[i];
    strW[i++] = L'\0';
    *pStrW += i;
    return strW;
}


//---------------------------------------------------------------------------
// Cred_StrWSizeA
//---------------------------------------------------------------------------


_FX size_t Cred_StrWSizeA(const char* strA)
{
    if (!strA)
        return 0;
    return (strlen(strA) + 1) * sizeof(WCHAR);
}


//---------------------------------------------------------------------------
// Cred_STRA2W
//---------------------------------------------------------------------------


_FX const WCHAR* Cred_STRA2W(const char* strA)
{
    if (!strA)
        return NULL;
    WCHAR* strW = LocalAlloc(LMEM_FIXED, Cred_StrWSizeA(strA));
    return Cred_CopyA2W(&strW, strA);
}


//---------------------------------------------------------------------------
// Cred_CREDENTIALA2W
//---------------------------------------------------------------------------


_FX CREDENTIALW* Cred_CREDENTIALA2W(CREDENTIALA* credA)
{
    ULONG size = sizeof(CREDENTIALW);
    size += Cred_StrWSizeA(credA->TargetName);  // CRED_MAX_DOMAIN_TARGET_NAME_LENGTH
    size += Cred_StrWSizeA(credA->Comment);     // CRED_MAX_STRING_LENGTH
    for (DWORD i = 0; i < credA->AttributeCount; i++) {         // CRED_MAX_ATTRIBUTES
        size += sizeof(CREDENTIAL_ATTRIBUTEA);
        size += Cred_StrWSizeA(credA->Attributes[i].Keyword);   // CRED_MAX_STRING_LENGTH
    }                                                           // CRED_MAX_VALUE_SIZE 
    size += Cred_StrWSizeA(credA->TargetAlias); // CRED_MAX_STRING_LENGTH
    size += Cred_StrWSizeA(credA->UserName);    // CRED_MAX_USERNAME_LENGTH

    CREDENTIALW* credW = LocalAlloc(LMEM_FIXED, size);

    WCHAR* ptr = (WCHAR*)(((char*)credW) + sizeof(CREDENTIALW));

    credW->Flags = credA->Flags;
    credW->Type = credA->Type;
    credW->TargetName = Cred_CopyA2W(&ptr, credA->TargetName);
    credW->Comment = Cred_CopyA2W(&ptr, credA->Comment);
    credW->LastWritten = credA->LastWritten;
    credW->CredentialBlobSize = credA->CredentialBlobSize;
    credW->CredentialBlob = credA->CredentialBlob;              // CRED_MAX_CREDENTIAL_BLOB_SIZE 
    credW->Persist = credA->Persist;
    credW->AttributeCount = credA->AttributeCount;
    credW->Attributes = (PCREDENTIAL_ATTRIBUTEW)ptr;
    ptr = (WCHAR*)(((char*)ptr) + (sizeof(PCREDENTIAL_ATTRIBUTEW) * credW->AttributeCount));
    for (DWORD i = 0; i < credA->AttributeCount; i++) {
        credW->Attributes[i].Keyword = Cred_CopyA2W(&ptr, credA->Attributes[i].Keyword);
        credW->Attributes[i].Flags = credA->Attributes[i].Flags;
        credW->Attributes[i].ValueSize = credA->Attributes[i].ValueSize;
        credW->Attributes[i].Value = credA->Attributes[i].Value;
    }   
    credW->TargetAlias = Cred_CopyA2W(&ptr, credA->TargetAlias);
    credW->UserName = Cred_CopyA2W(&ptr, credA->UserName);

    return credW;
}


//---------------------------------------------------------------------------
// Cred_CREDENTIALW2A
//---------------------------------------------------------------------------


_FX CREDENTIALA* Cred_CREDENTIALW2A(CREDENTIALW* credW)
{
    ULONG size = sizeof(CREDENTIALA);
    size += Cred_StrASizeW(credW->TargetName);  // CRED_MAX_DOMAIN_TARGET_NAME_LENGTH
    size += Cred_StrASizeW(credW->Comment);     // CRED_MAX_STRING_LENGTH
    for (DWORD i = 0; i < credW->AttributeCount; i++) {         // CRED_MAX_ATTRIBUTES
        size += sizeof(CREDENTIAL_ATTRIBUTEA);
        size += Cred_StrASizeW(credW->Attributes[i].Keyword);   // CRED_MAX_STRING_LENGTH
    }                                                           // CRED_MAX_VALUE_SIZE 
    size += Cred_StrASizeW(credW->TargetAlias); // CRED_MAX_STRING_LENGTH
    size += Cred_StrASizeW(credW->UserName);    // CRED_MAX_USERNAME_LENGTH

    CREDENTIALA* credA = LocalAlloc(LMEM_FIXED, size);

    char* ptr = ((char*)credW) + sizeof(CREDENTIALW);

    credA->Flags = credW->Flags;
    credA->Type = credW->Type;
    credA->TargetName = Cred_CopyW2A(&ptr, credW->TargetName);
    credA->Comment = Cred_CopyW2A(&ptr, credW->Comment);
    credA->LastWritten = credW->LastWritten;
    credA->CredentialBlobSize = credW->CredentialBlobSize;
    credA->CredentialBlob = credW->CredentialBlob;              // CRED_MAX_CREDENTIAL_BLOB_SIZE 
    credA->Persist = credW->Persist;
    credA->AttributeCount = credW->AttributeCount;
    credA->Attributes = (PCREDENTIAL_ATTRIBUTEA)ptr;
    ptr += sizeof(PCREDENTIAL_ATTRIBUTEA) * credW->AttributeCount;
    for (DWORD i = 0; i < credW->AttributeCount; i++) {
        credA->Attributes[i].Keyword = Cred_CopyW2A(&ptr, credW->Attributes[i].Keyword);
        credA->Attributes[i].Flags = credW->Attributes[i].Flags;
        credA->Attributes[i].ValueSize = credW->Attributes[i].ValueSize;
        credA->Attributes[i].Value = credW->Attributes[i].Value;
    }   
    credA->TargetAlias = Cred_CopyW2A(&ptr, credW->TargetAlias);
    credA->UserName = Cred_CopyW2A(&ptr, credW->UserName);

    return credA;
}


//---------------------------------------------------------------------------
// Cred_CREDENTIAL_TARGET_INFORMATIONA2W
//---------------------------------------------------------------------------


_FX CREDENTIAL_TARGET_INFORMATIONW* Cred_CREDENTIAL_TARGET_INFORMATIONA2W(CREDENTIAL_TARGET_INFORMATIONA* TargetInfoA)
{
    ULONG size = sizeof(CREDENTIAL_TARGET_INFORMATIONW);
    size += Cred_StrWSizeA(TargetInfoA->TargetName);
    size += Cred_StrWSizeA(TargetInfoA->NetbiosServerName);
    size += Cred_StrWSizeA(TargetInfoA->DnsServerName);
    size += Cred_StrWSizeA(TargetInfoA->NetbiosDomainName);
    size += Cred_StrWSizeA(TargetInfoA->DnsDomainName);
    size += Cred_StrWSizeA(TargetInfoA->DnsTreeName);
    size += Cred_StrWSizeA(TargetInfoA->PackageName);

    CREDENTIAL_TARGET_INFORMATIONW* TargetInfoW = LocalAlloc(LMEM_FIXED, size);

    WCHAR* ptr = (WCHAR*)(((char*)TargetInfoW) + sizeof(CREDENTIAL_TARGET_INFORMATIONW));

    TargetInfoW->TargetName = Cred_CopyA2W(&ptr, TargetInfoA->TargetName);
    TargetInfoW->NetbiosServerName = Cred_CopyA2W(&ptr, TargetInfoA->NetbiosServerName);
    TargetInfoW->DnsServerName = Cred_CopyA2W(&ptr, TargetInfoA->DnsServerName);
    TargetInfoW->NetbiosDomainName = Cred_CopyA2W(&ptr, TargetInfoA->NetbiosDomainName);
    TargetInfoW->DnsDomainName = Cred_CopyA2W(&ptr, TargetInfoA->DnsDomainName);
    TargetInfoW->DnsTreeName = Cred_CopyA2W(&ptr, TargetInfoA->DnsTreeName);
    TargetInfoW->PackageName = Cred_CopyA2W(&ptr, TargetInfoA->PackageName);
    TargetInfoW->Flags = TargetInfoA->Flags;
    TargetInfoW->CredTypeCount = TargetInfoA->CredTypeCount;
    TargetInfoW->CredTypes = TargetInfoA->CredTypes;
    
    return TargetInfoW;
}


//---------------------------------------------------------------------------
// Cred_CredRenameW
//---------------------------------------------------------------------------


_FX BOOL Cred_CredRenameW(
    void *OldTargetName, void *NewTargetName, ULONG Type, ULONG Flags)
{
    SbieApi_Log(2205, L"CredRenameW");
    SetLastError(ERROR_NO_SUCH_LOGON_SESSION);
    return FALSE;
}


//---------------------------------------------------------------------------
// Cred_CredWriteA
//---------------------------------------------------------------------------


_FX BOOL Cred_CredWriteA(void *pCredential, ULONG Flags)
{
    CREDENTIALA *credA = (CREDENTIALA *)pCredential;

    CREDENTIALW *credW = Cred_CREDENTIALA2W(credA);

    BOOL ret = Cred_CredWriteW(credW, Flags);

    if (credW) LocalFree(credW);

    return ret;
}


//---------------------------------------------------------------------------
// Cred_CredWriteDomainCredentialsW
//---------------------------------------------------------------------------


_FX BOOL Cred_CredWriteDomainCredentialsA(
    void *pTargetInfo, void *pCredential, ULONG Flags)
{
    CREDENTIAL_TARGET_INFORMATIONA *TargetInfoA =
        (CREDENTIAL_TARGET_INFORMATIONA *)pTargetInfo;
    CREDENTIALA *credA = (CREDENTIALA *)pCredential;

    CREDENTIAL_TARGET_INFORMATIONW *TargetInfoW = Cred_CREDENTIAL_TARGET_INFORMATIONA2W(TargetInfoA);
    CREDENTIALW *credW = Cred_CREDENTIALA2W(credA);

    BOOL ret = Cred_CredWriteDomainCredentialsW(TargetInfoW, credW, Flags);

    if (TargetInfoW) LocalFree(TargetInfoW);
    if (credW) LocalFree(credW);

    return ret;
}


//---------------------------------------------------------------------------
// Cred_CredGetTargetInfoW
//---------------------------------------------------------------------------


/*_FX BOOL Cred_CredGetTargetInfoW(
    void *pTargetName, ULONG Flags, void **pTargetInfo)
{
    SbieApi_Log(2205, L"CredGetTargetInfoW");
    return __sys_CredGetTargetInfoW(pTargetName, Flags, pTargetInfo);
}*/


//---------------------------------------------------------------------------
// Cred_CredGetTargetInfoA
//---------------------------------------------------------------------------


/*_FX BOOL Cred_CredGetTargetInfoA(
    void *pTargetName, ULONG Flags, void **pTargetInfo)
{
    SbieApi_Log(2205, L"CredGetTargetInfoA");
    return __sys_CredGetTargetInfoA(pTargetName, Flags, pTargetInfo);
}*/


//---------------------------------------------------------------------------
// Cred_CredRenameA
//---------------------------------------------------------------------------


_FX BOOL Cred_CredRenameA(
    void *OldTargetName, void *NewTargetName, ULONG Type, ULONG Flags)
{
    SbieApi_Log(2205, L"CredRenameA");
    SetLastError(ERROR_NO_SUCH_LOGON_SESSION);
    return FALSE;
}


//---------------------------------------------------------------------------
// Cred_CredDeleteA
//---------------------------------------------------------------------------


_FX BOOL Cred_CredDeleteA(const char *TargetName, ULONG Type, ULONG Flags)
{
    const WCHAR* TargetNameW = Cred_STRA2W(TargetName);
    
    BOOL ret = Cred_CredDeleteW(TargetNameW, Type, Flags);

    if (TargetNameW) LocalFree((WCHAR*)TargetNameW);

    return ret;
}


//---------------------------------------------------------------------------
// Cred_CredReadA
//---------------------------------------------------------------------------


_FX BOOL Cred_CredReadA(
    const char *TargetName, ULONG Type, ULONG Flags, void **ppCredential)
{
    const WCHAR* TargetNameW = Cred_STRA2W(TargetName);
    
    void* pCredentialW = NULL;
    BOOL ret = Cred_CredReadW(TargetNameW, Type, Flags, &pCredentialW);

    if (pCredentialW) {
        *ppCredential = Cred_CREDENTIALW2A(pCredentialW);
        LocalFree(pCredentialW);
    }
    if (TargetNameW) LocalFree((WCHAR*)TargetNameW);

    return ret;
}


//---------------------------------------------------------------------------
// Cred_CredReadDomainCredentialsA
//---------------------------------------------------------------------------


_FX BOOL Cred_CredReadDomainCredentialsA(
    void *pTargetInfo, ULONG Flags, ULONG *pCount, void ***ppCredentials)
{
    // todo
    SbieApi_Log(2205, L"CredReadDomainCredentialsA");
    return __sys_CredReadDomainCredentialsA(
                                pTargetInfo, Flags, pCount, ppCredentials);
}


//---------------------------------------------------------------------------
// Cred_CredEnumerateA
//---------------------------------------------------------------------------


_FX BOOL Cred_CredEnumerateA(
    void *pFilter, ULONG Flags, ULONG *pCount, void ***ppCredentials)
{
    // todo
    //SbieApi_Log(2205, L"CredEnumerateA");
    return __sys_CredEnumerateA(pFilter, Flags, pCount, ppCredentials);
}
