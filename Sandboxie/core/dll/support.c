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
// Support Functions for Programs Using SbieDll
//---------------------------------------------------------------------------


#ifndef SBIEDLL_FORMATMESSAGE_ONLY


#include <stdio.h>
#include "dll.h"
#include "obj.h"
#include "advapi.h"
#include "core/svc/msgids.h"
#include "core/svc/SbieIniWire.h"
#include "core/svc/ProcessWire.h"
#include "common/my_version.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static void SbieDll_SetStartError(ULONG Level);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static WCHAR *SbieDll_StartError = NULL;

const WCHAR *Support_SbieSvcKeyPath =
    L"\\registry\\machine\\system\\currentcontrolset\\services\\" SBIESVC;



//---------------------------------------------------------------------------
// SbieDll_SetStartError
//---------------------------------------------------------------------------


_FX void SbieDll_SetStartError(ULONG Level)
{
    ULONG ErrorCode = GetLastError();

    DWORD FormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS;
    WCHAR *ErrorText;

	size_t len;

    if (SbieDll_StartError) {
        Dll_Free(SbieDll_StartError);
        SbieDll_StartError = NULL;
    }

    FormatMessage(FormatFlags, NULL, ErrorCode,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR)&ErrorText, 0, NULL);

	len = (wcslen(ErrorText) + 32);
    SbieDll_StartError = Dll_Alloc(len * sizeof(WCHAR));

    Sbie_snwprintf(SbieDll_StartError, len,
             L"[%02X / %d] %s", Level, ErrorCode, ErrorText);

    LocalFree(ErrorText);
}


//---------------------------------------------------------------------------
// SbieDll_GetStartError
//---------------------------------------------------------------------------


_FX const WCHAR *SbieDll_GetStartError(void)
{
    return SbieDll_StartError;
}


//---------------------------------------------------------------------------
// SbieDll_StartSbieSvc
//---------------------------------------------------------------------------


_FX BOOLEAN SbieDll_StartSbieSvc(BOOLEAN retry)
{
    typedef void *(*P_OpenSCManager)(void *p1, void *p2, ULONG acc);
    typedef void *(*P_OpenService)(void *hSCM, void *name, ULONG acc);
    typedef BOOL (*P_StartService)(void *hSvc, ULONG p2, void *p3);
    typedef void *(*P_CloseServiceHandle)(void *h);
    static P_OpenSCManager pOpenSCManagerW = NULL;
    static P_OpenService pOpenServiceW = NULL;
    static P_StartService pStartServiceW = NULL;
    static P_CloseServiceHandle pCloseServiceHandle = NULL;
    ULONG retries;

    if (! pOpenSCManagerW) {
        HMODULE mod = LoadLibrary(DllName_advapi32);
        if (mod) {
            pOpenSCManagerW = (P_OpenSCManager)
                                GetProcAddress(mod, "OpenSCManagerW");
            pOpenServiceW = (P_OpenService)
                                GetProcAddress(mod, "OpenServiceW");
            pStartServiceW = (P_StartService)
                                GetProcAddress(mod, "StartServiceW");
            pCloseServiceHandle = (P_CloseServiceHandle)
                                GetProcAddress(mod, "CloseServiceHandle");
        }
    }

    for (retries = 0; retries < 3; ++retries) {

        SBIE_INI_GET_VERSION_REQ req;
        SBIE_INI_GET_VERSION_RPL *rpl;

        req.h.length = sizeof(SBIE_INI_GET_VERSION_REQ);
        req.h.msgid = MSGID_SBIE_INI_GET_VERSION;
        rpl = (SBIE_INI_GET_VERSION_RPL *)SbieDll_CallServer(&req.h);
        if (rpl) {
            Dll_Free(rpl);
            return TRUE;
        }

        if (retries == 0) {

            void *hScm = NULL;
            if (pOpenSCManagerW) {

                hScm = pOpenSCManagerW(NULL, NULL, GENERIC_READ);
                if (! hScm)
                    SbieDll_SetStartError(0x11);
            }

            if (hScm) {

                void *hSvc = NULL;
                if (pOpenServiceW) {

                    hSvc = pOpenServiceW(hScm, SBIESVC, SERVICE_START);
                    if (! hSvc)
                        SbieDll_SetStartError(0x22);

                } if (hSvc) {

                    BOOL ok = FALSE;
                    if (pStartServiceW) {

                        ok = pStartServiceW(hSvc, 0, NULL);

                        /*if ((! ok) && GetLastError() ==
                                        ERROR_SERVICE_ALREADY_RUNNING)
                            ok = TRUE;*/

                        if (! ok)
                            SbieDll_SetStartError(0x33);
                    }

                    pCloseServiceHandle(hSvc);
                }

                pCloseServiceHandle(hScm);
            }

        } else if (! retry)
            return FALSE;

        Sleep(200);
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// Dll_NlsStrCmp
//---------------------------------------------------------------------------


_FX int Dll_NlsStrCmp(const WCHAR *s1, const WCHAR *s2, ULONG len)
{
    UNICODE_STRING u1, u2;

    u1.Length = u1.MaximumLength = u2.Length = u2.MaximumLength =
        (USHORT)(len * sizeof(WCHAR));
    u1.Buffer = (WCHAR *)s1;
    u2.Buffer = (WCHAR *)s2;

    return RtlCompareUnicodeString(&u1, &u2, TRUE);
}


//---------------------------------------------------------------------------
// SbieDll_GetServiceRegistryValue
//---------------------------------------------------------------------------


_FX BOOLEAN SbieDll_GetServiceRegistryValue(
    const WCHAR *name, void *kvpi, ULONG sizeof_kvpi)
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    HANDLE handle;
    ULONG len;

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    RtlInitUnicodeString(&objname, Support_SbieSvcKeyPath);

    status = NtOpenKey(&handle, KEY_READ, &objattrs);
    if (! NT_SUCCESS(status))
        return FALSE;

    RtlInitUnicodeString(&objname, name);

    status = NtQueryValueKey(
        handle, &objname, KeyValuePartialInformation,
        kvpi, sizeof_kvpi, &len);

    NtClose(handle);

    if (! NT_SUCCESS(status))
        return FALSE;

    return TRUE;
}


//---------------------------------------------------------------------------
// SbieDll_GetLanguage
//---------------------------------------------------------------------------


_FX ULONG SbieDll_GetLanguage(BOOLEAN *rtl)
{
    static ULONG lang = 0;
    union {
        KEY_VALUE_PARTIAL_INFORMATION info;
        WCHAR space[32];
    } u;

    if (! lang) {

        lang = 1033;                            // default English

        if (SbieDll_GetServiceRegistryValue(
                L"Language", &u.info, sizeof(u))) {

            if (u.info.Type == REG_DWORD &&
                u.info.DataLength == sizeof(ULONG)) {

                lang = *(ULONG *)u.info.Data;
                if (lang != 1025 &&     /* Arabic */
                    lang != 1026 &&     /* Bulgarian */
                    lang != 1050 &&     /* Croatian */
                    lang != 1029 &&     /* Czech */
                    lang != 1030 &&     /* Danish */
                    lang != 1031 &&     /* German */
                    lang != 1032 &&     /* Greek */
                    lang != 1034 &&     /* Spanish */
                    lang != 1035 &&     /* Finnish */
                    lang != 1036 &&     /* French */
                    lang != 1037 &&     /* Hebrew */
                    lang != 1038 &&     /* Hungarian */
                    lang != 1057 &&     /* Indonesian */
                    lang != 1040 &&     /* Italian */
                    lang != 1041 &&     /* Japanese */
                    lang != 1042 &&     /* Korean */
                    lang != 1043 &&     /* Dutch */
                    lang != 1045 &&     /* Polish */
                    lang != 1046 &&     /* Portuguese (Brasil) */
                    lang != 2070 &&     /* Portuguese (Portugal) */
                    lang != 1049 &&     /* Russian */
                    lang != 1051 &&     /* Slovak */
                    lang != 1052 &&     /* Albanian */
                    lang != 1053 &&     /* Swedish */
                    lang != 1055 &&     /* Turkish */
                    lang != 1058 &&     /* Ukrainian */
                    lang != 1061 &&     /* Estonian */
                    lang != 1065 &&     /* Farsi */
                    lang != 1071 &&     /* Macedonian */
                    lang != 1028 &&     /* Chinese (Traditional) */
                    lang != 2052) {     /* Chinese (Simplified) */

                    lang = 1033;        /* English - default */
                }
            }
        }
    }

    if (rtl) {
        if (lang == 1037)               /* Hebrew */
            *rtl = TRUE;
        else
            *rtl = FALSE;
    }

    return lang;
}


//---------------------------------------------------------------------------
// SbieDll_KillOne
//---------------------------------------------------------------------------


_FX BOOLEAN SbieDll_KillOne(ULONG ProcessId)
{
    PROCESS_KILL_ONE_REQ req;
    MSG_HEADER *rpl;
    BOOLEAN ok = FALSE;

    req.h.length = sizeof(PROCESS_KILL_ONE_REQ);
    req.h.msgid = MSGID_PROCESS_KILL_ONE;
    req.pid = ProcessId;

    rpl = SbieDll_CallServer(&req.h);

    if (rpl) {
        if (rpl->status == 0)
            ok = TRUE;
        Dll_Free(rpl);
    }

    return ok;
}


//---------------------------------------------------------------------------
// SbieDll_KillAll
//---------------------------------------------------------------------------


_FX BOOLEAN SbieDll_KillAll(ULONG SessionId, const WCHAR *BoxName)
{
    PROCESS_KILL_ALL_REQ req;
    MSG_HEADER *rpl;
    BOOLEAN ok = FALSE;

    req.h.length = sizeof(PROCESS_KILL_ALL_REQ);
    req.h.msgid = MSGID_PROCESS_KILL_ALL;
    req.session_id = SessionId;
    wcscpy(req.boxname, BoxName ? BoxName : Dll_BoxName);

    rpl = SbieDll_CallServer(&req.h);

    if (rpl) {
        if (rpl->status == 0)
            ok = TRUE;
        Dll_Free(rpl);
    }

    return ok;
}


//---------------------------------------------------------------------------
// Dll_SidStringToSid
//---------------------------------------------------------------------------


_FX void *Dll_SidStringToSid(const WCHAR *SidString)
{
    ULONG AuthorityCount;
    ULONG Authority[12];
    WCHAR *SidSpace;

    const WCHAR *ptr = SidString;           // expect S-r-i-s-s

    // expect S- prefix
    if (ptr[0] != L'S' || ptr[1] != L'-')
        return NULL;
    ptr += 2;

    // expect revision 1, as in S-1-
    if (ptr[0] != L'1' || ptr[1] != L'-')
        return NULL;
    ptr += 2;

    // collect identifier authority and then sub authorities
    AuthorityCount = 0;
    while (*ptr) {
        const WCHAR *ptr2 = ptr;
        while (*ptr2 != L'-') {
            if (! iswdigit(*ptr2)) {
                if (! *ptr2)
                    break;
                return NULL;
            }
            ++ptr2;
        }
        if (ptr2 == ptr)
            return NULL;
        Authority[AuthorityCount] = _wtoi(ptr);
        ++AuthorityCount;
        if (AuthorityCount >= sizeof(Authority) / sizeof(ULONG))
            return NULL;
        ptr = ptr2;
        if (*ptr)
            ++ptr;
    }

    // expect identifier authority and at least one sub authority
    if (AuthorityCount < 2)
        return NULL;

    SidSpace = Dll_Alloc(8 + (AuthorityCount - 1) * sizeof(ULONG));
    if (SidSpace) {

        UCHAR *sid = (UCHAR *)SidSpace;
        ULONG i;
        ULONG *subauth;

        sid[0] = 1;                             // Revision
        sid[1] = (UCHAR)(AuthorityCount - 1);
        *(USHORT *)(sid + 2) = 0;               // IdentifierAuthority
        *(ULONG *)(sid + 4) = 0;
        sid[7] = (UCHAR)Authority[0];

        subauth = (ULONG *)(sid + 8);
        for (i = 1; i < AuthorityCount; ++i)
            subauth[i - 1] = Authority[i];
    }

    return SidSpace;
}


//---------------------------------------------------------------------------
// Dll_GetCurrentSidString
//---------------------------------------------------------------------------


_FX NTSTATUS Dll_GetCurrentSidString(UNICODE_STRING *SidString)
{
    HANDLE token;
    union {
        WCHAR info_space[64];
        TOKEN_USER token;
    } info;
    ULONG len;

    NTSTATUS status = NtOpenProcessToken(
        NtCurrentProcess(), TOKEN_QUERY, &token);
    if (NT_SUCCESS(status)) {

        status = NtQueryInformationToken(
            token, TokenUser, &info, sizeof(info), &len);
        if (NT_SUCCESS(status)) {

            status = RtlConvertSidToUnicodeString(
                SidString, info.token.User.Sid, TRUE);
        }

        NtClose(token);
    }

    return status;
}


//---------------------------------------------------------------------------
// SbieDll_GetTokenElevationType
//---------------------------------------------------------------------------


_FX ULONG SbieDll_GetTokenElevationType(void)
{
    static BOOLEAN AlreadyChecked = FALSE;
    static ULONG   CachedResult   = TokenElevationTypeNone;

    if (! AlreadyChecked) {

        HANDLE TokenHandle = NULL;
        TOKEN_GROUPS *Groups = NULL;
        HANDLE KeyHandle = NULL;

        do {

            NTSTATUS status;
            ULONG elevationType, len;
            OSVERSIONINFO osvi;
            OBJECT_ATTRIBUTES objattrs;
            UNICODE_STRING objname;
            ULONG kvpi_space[8];
            KEY_VALUE_PARTIAL_INFORMATION *kvpi;

            osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
            if (GetVersionEx(&osvi) && osvi.dwMajorVersion == 5) {

                //
                // earlier than Windows Vista, return TokenElevationTypeNone
                //

                break;
            }

            status = NtOpenProcessToken(NtCurrentProcess(), TOKEN_QUERY,
                                        &TokenHandle);
            if (! NT_SUCCESS(status))
                break;

            status = NtQueryInformationToken(
                TokenHandle, (TOKEN_INFORMATION_CLASS)TokenElevationType,
                &elevationType, sizeof(elevationType), &len);
            if (! NT_SUCCESS(status))
                break;

            //
            // TokenElevationTypeFull or TokenElevationTypeLimited
            // indicate UAC is active.  otherwise TokenElevationTypeDefault
            // is the only other valid value
            //

            if (elevationType == TokenElevationTypeFull ||
                elevationType == TokenElevationTypeLimited) {

                CachedResult = elevationType;
                break;
            }

            if (elevationType != TokenElevationTypeDefault)
                break;

            //
            // check if token is a member of the Administrators group
            //

            Groups = Dll_AllocTemp(8192);

            status = NtQueryInformationToken(
                TokenHandle, (TOKEN_INFORMATION_CLASS)TokenGroups,
                Groups, 8192, &len);

            if (NT_SUCCESS(status)) {

static UCHAR Support_BuiltinDomainRid[12] = {
    1,                                      // Revision
    2,                                      // SubAuthorityCount
    0,0,0,0,0,5, // SECURITY_NT_AUTHORITY   // IdentifierAuthority
    SECURITY_BUILTIN_DOMAIN_RID             // SubAuthority
};

                for (len = 0; len < Groups->GroupCount; ++len) {
                    PSID_AND_ATTRIBUTES group = &Groups->Groups[len];
                    PISID sid = group->Sid;
                    if (0 == memcmp(sid, Support_BuiltinDomainRid, 12)
                        && sid->SubAuthority[1] == DOMAIN_ALIAS_RID_ADMINS
                        && group->Attributes & SE_GROUP_ENABLED)
                    {
                        elevationType = TokenElevationTypeFull;
                        break;
                    }
                }
            }

            if (elevationType == TokenElevationTypeFull) {

                CachedResult = elevationType;
                break;
            }

            //
            // check registry to see if UAC is enabled or disabled
            //

            InitializeObjectAttributes(
                &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

            RtlInitUnicodeString(&objname,
                L"\\registry\\machine\\software\\microsoft\\"
                L"\\windows\\currentversion\\policies\\system");

            status = NtOpenKey(&KeyHandle, KEY_READ, &objattrs);
            if (! NT_SUCCESS(status))
                break;

            RtlInitUnicodeString(&objname, L"EnableLUA");

            kvpi = (KEY_VALUE_PARTIAL_INFORMATION *)kvpi_space;

            status = NtQueryValueKey(
                KeyHandle, &objname, KeyValuePartialInformation,
                kvpi, sizeof(kvpi_space), &len);

            if (NT_SUCCESS(status) &&
                    kvpi->DataLength == sizeof(ULONG) &&
                    *(ULONG *)&kvpi->Data != 0) {

                //
                // non-Admin user but UAC elevation is possible
                //

                CachedResult = elevationType;
                break;
            }

        } while (0);

        if (KeyHandle)
            CloseHandle(KeyHandle);
        if (Groups)
            Dll_Free(Groups);
        if (TokenHandle)
            CloseHandle(TokenHandle);

        AlreadyChecked = TRUE;
    }

    return CachedResult;
}


//---------------------------------------------------------------------------


#endif  /* SBIEDLL_FORMATMESSAGE_ONLY */

#if 1

//---------------------------------------------------------------------------
// SbieDll_FormatMessage_2
//---------------------------------------------------------------------------

extern int __CRTDECL Sbie_snwprintf(wchar_t *_Buffer, size_t Count, const wchar_t * const _Format, ...);

_FX ULONG SbieDll_FormatMessage_2(WCHAR **text_ptr, const WCHAR **ins)
{
    //
    // for right-to-left language text files (Hebrew and Arabic),
    // the percent signs can make it difficult to edit the text files
    // in Notepad.  as a workaround, also support for .N. in addition
    // to %N as a parameter for FormatMessage.  this workaround is
    // used only by the Hebrew and Arabic text files.
    //

    const ULONG FormatFlags     = FORMAT_MESSAGE_FROM_STRING |
                                  FORMAT_MESSAGE_ARGUMENT_ARRAY |
                                  FORMAT_MESSAGE_ALLOCATE_BUFFER;
    const WCHAR *_x2 = L".2.";
    const WCHAR *_x3 = L".3.";
    const WCHAR *_x4 = L".4.";
    WCHAR *oldtxt, *newtxt, *ptr;
    ULONG rc;

    oldtxt = *text_ptr;
    ptr = wcsstr(oldtxt, _x2);
    if (! ptr)
        return 0;

    //
    // if the .2. is in the insert rather than the actual message,
    // then just quit.  this is not ideal, but not frequent either.
    // (fixed in version 4.05 build 012)
    //
    if (ins[1] && wcsstr(ins[1], _x2))
        return 0;
    if (ins[2] && wcsstr(ins[2], _x2))
        return 0;

    newtxt = LocalAlloc(LMEM_FIXED, (wcslen(oldtxt) + 1) * sizeof(WCHAR));
    if (! newtxt)
        return 0;
    wcscpy(newtxt, oldtxt);
    ptr = newtxt + (ptr - oldtxt);

    while (ptr) {
        ptr[0] = L'%';
        wmemmove(ptr + 2, ptr + 3, wcslen(ptr + 3) + 1);
        ptr = wcsstr(newtxt, _x2);
        if (! ptr)
            ptr = wcsstr(newtxt, _x3);
        if (! ptr)
            ptr = wcsstr(newtxt, _x4);
    }

    rc = FormatMessage(FormatFlags, newtxt, 0, 0,
                       (LPWSTR)&ptr, 4, (va_list *)ins);
    if (rc != 0) {
        *text_ptr = ptr;
        LocalFree(oldtxt);
    }

    LocalFree(newtxt);
    return rc;
}


//---------------------------------------------------------------------------
// SbieDll_FormatMessage
//---------------------------------------------------------------------------


_FX WCHAR *SbieDll_FormatMessage(ULONG code, const WCHAR **ins)
{
    static HMODULE SbieMsgDll   = NULL;
    const ULONG FormatFlags     = FORMAT_MESSAGE_FROM_HMODULE |
                                  FORMAT_MESSAGE_ARGUMENT_ARRAY |
                                  FORMAT_MESSAGE_ALLOCATE_BUFFER;
    WCHAR *out;
    ULONG rc;
    ULONG err = GetLastError();

    //
    // get the handle for SbieMsg.dll
    //

    if (! SbieMsgDll) {

#ifndef SBIEDLL_FORMATMESSAGE_ONLY

        STARTUPINFOW si;
        if (SbieDll_RunFromHome(SBIEMSG_DLL, NULL, &si, NULL)) {
            WCHAR *path2 = (WCHAR *)si.lpReserved;
            SbieMsgDll =
                LoadLibraryEx(path2, NULL, LOAD_LIBRARY_AS_DATAFILE);
            HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, path2);
        }

        if (! SbieMsgDll) {
            WCHAR *path = LocalAlloc(
                                LMEM_FIXED, (MAX_PATH + 64) * sizeof(WCHAR));
            if (path) {
                if (SbieApi_GetHomePath(NULL, 0, path, MAX_PATH) == 0) {
                    wcscat(path, L"\\" SBIEMSG_DLL);
                    SbieMsgDll =
                        LoadLibraryEx(path, NULL, LOAD_LIBRARY_AS_DATAFILE);
                }
                LocalFree(path);
            }
        }

#else

        SBIEDLL_GET_SBIE_MSG_DLL

#endif  /* SBIEDLL_FORMATMESSAGE_ONLY */
    }

    //
    // try to translate the string
    //

    rc = 0;
    out = NULL;

    if (SbieMsgDll) {

        rc = FormatMessage(FormatFlags, SbieMsgDll, code,
                           SbieDll_GetLanguage(NULL),
                           (LPWSTR)&out, 4, (va_list *)ins);
        if (rc != 0) {
            ULONG xrc = SbieDll_FormatMessage_2(&out, ins);
            if (xrc)
                rc = xrc;
        }
    }

    if (rc == 0) {

        out = LocalAlloc(LMEM_FIXED, 128 * sizeof(WCHAR));
        if (out) {
            static const WCHAR *_empty = L"";
            Sbie_snwprintf(out, 128, L"err=%08X ... str1=%40.40s ... str2=%40.40s",
                     code,
                     ins && ins[0] ? ins[0] : _empty,
                     ins && ins[1] ? ins[1] : _empty);
            rc = wcslen(out);
        }
    }

    if (out) {

        if (out[rc - 1] == L'\r' || out[rc - 1] == L'\n') {
            out[rc - 1] = L'\0';
            --rc;
        }
        if (out[rc - 1] == L'\r' || out[rc - 1] == L'\n') {
            out[rc - 1] = L'\0';
            --rc;
        }
    }

    SetLastError(err);
    return out;
}


//---------------------------------------------------------------------------
// SbieDll_FormatMessage0
//---------------------------------------------------------------------------


_FX WCHAR *SbieDll_FormatMessage0(ULONG code)
{
    return SbieDll_FormatMessage(code, NULL);
}


//---------------------------------------------------------------------------
// SbieDll_FormatMessage1
//---------------------------------------------------------------------------


_FX WCHAR *SbieDll_FormatMessage1(ULONG code, const WCHAR *ins1)
{
    const WCHAR *ins[6];
    memzero((WCHAR *)ins, sizeof(ins));
    ins[1] = (WCHAR *)ins1;
    return SbieDll_FormatMessage(code, ins);
}


//---------------------------------------------------------------------------
// SbieDll_FormatMessage2
//---------------------------------------------------------------------------


_FX WCHAR *SbieDll_FormatMessage2(
    ULONG code, const WCHAR *ins1, const WCHAR *ins2)
{
    const WCHAR *ins[6];
    memzero((WCHAR *)ins, sizeof(ins));
    ins[1] = ins1;
    ins[2] = ins2;
    return SbieDll_FormatMessage(code, ins);
}

#endif

//---------------------------------------------------------------------------
// SbieDll_IsReservedFileName
//---------------------------------------------------------------------------

_FX BOOLEAN SbieDll_IsReservedFileName(const WCHAR *name)
{
    static const WCHAR* deviceNames[] = {
        L"aux", L"clock$", L"con", L"nul", L"prn",
        L"com1", L"com2", L"com3", L"com4", L"com5",
        L"com6", L"com7", L"com8", L"com9",
        L"lpt1", L"lpt2", L"lpt3", L"lpt4", L"lpt5",
        L"lpt6", L"lpt7", L"lpt8", L"lpt9",
        NULL
    };

    for (ULONG devNum = 0; deviceNames[devNum]; ++devNum) {
        const WCHAR* devName = deviceNames[devNum];
        //ULONG devNameLen = wcslen(devName);
        //if (_wcsnicmp(name, devName, devNameLen) == 0) {
        if (_wcsicmp(name, devName) == 0) {
            return TRUE;
        }
    }

    return FALSE;
}