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
// Sandboxie Ini Server -- using PipeServer
//---------------------------------------------------------------------------

#include "stdafx.h"

#include <wincrypt.h>
#include <wtsapi32.h>
#include <userenv.h>
#include "sbieiniserver.h"
#include "sbieiniwire.h"
#include "misc.h"
#include "core/dll/sbiedll.h"
#include "common/defines.h"
#include "common/my_version.h"
#define CRC_HEADER_ONLY
#include "common/crc.c"


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


SbieIniServer::SbieIniServer(PipeServer *pipeServer)
{
    InitializeCriticalSection(&m_critsec);
    m_text = NULL;

    m_hLockFile = INVALID_HANDLE_VALUE;
    LockConf(NULL);

    pipeServer->Register(MSGID_SBIE_INI, this, Handler);
}


//---------------------------------------------------------------------------
// Handler
//---------------------------------------------------------------------------


MSG_HEADER *SbieIniServer::Handler(void *_this, MSG_HEADER *msg)
{
    SbieIniServer *pThis = (SbieIniServer *)_this;

    EnterCriticalSection(&pThis->m_critsec);

    pThis->m_text = NULL;
    pThis->m_text_base = NULL;
    pThis->m_text_max_len = 0;
    //pThis->m_insertbom = FALSE;

    MSG_HEADER *rpl = pThis->Handler2(msg);

    if (pThis->m_text)
        HeapFree(GetProcessHeap(), 0, pThis->m_text_base);

    pThis->m_text = NULL;
    pThis->m_text_base = NULL;
    pThis->m_text_max_len = 0;

    LeaveCriticalSection(&pThis->m_critsec);

    return rpl;
}


//---------------------------------------------------------------------------
// Handler
//---------------------------------------------------------------------------


MSG_HEADER *SbieIniServer::Handler2(MSG_HEADER *msg)
{
    //
    // handle run sbie ctrl request from any process, otherwise
    // caller must not be sandboxed, and we impersonate caller
    //

    HANDLE idProcess = (HANDLE)(ULONG_PTR)PipeServer::GetCallerProcessId();

    m_session_id = PipeServer::GetCallerSessionId();

    NTSTATUS status =
                SbieApi_QueryProcess(idProcess, NULL, NULL, NULL, NULL);

    if (msg->msgid == MSGID_SBIE_INI_RUN_SBIE_CTRL) {

        return RunSbieCtrl(idProcess, NT_SUCCESS(status));
    }

    if (NT_SUCCESS(status))     // if sandboxed
        return SHORT_REPLY(STATUS_NOT_SUPPORTED);

    if (PipeServer::ImpersonateCaller(&msg) != 0)
        return msg;

    //
    // handle get version request
    //

    if (msg->msgid == MSGID_SBIE_INI_GET_VERSION) {

        return GetVersion(msg);
    }

    //
    // handle get wait handle request
    //

    if (msg->msgid == MSGID_SBIE_INI_GET_WAIT_HANDLE) {

        return GetWaitHandle(idProcess);
    }

    //
    // handle get user request
    //

    if (msg->msgid == MSGID_SBIE_INI_GET_USER) {

        return GetUser(msg);
    }

    //
    // handle get path request
    //

    if (msg->msgid == MSGID_SBIE_INI_GET_PATH) {

        return GetPath(msg);
    }

    //
    // handle a SBIE_INI_TEMPLATE_REQ request
    //

    if (msg->msgid == MSGID_SBIE_INI_TEMPLATE) {

        status = SetTemplate(msg);

        if (m_text && NT_SUCCESS(status))
            status = RefreshConf();

        if (status == STATUS_INSUFFICIENT_RESOURCES)
            SbieApi_LogEx(m_session_id, 2305, NULL);

        return SHORT_REPLY(status);
    }

    //
    // handle a SBIE_INI_PASSWORD_REQ request
    //

    if (msg->msgid == MSGID_SBIE_INI_SET_PASSWORD ||
        msg->msgid == MSGID_SBIE_INI_TEST_PASSWORD) {

        status = SetOrTestPassword(msg);

        if (m_text && NT_SUCCESS(status))
            status = RefreshConf();

        if (status == STATUS_INSUFFICIENT_RESOURCES)
            SbieApi_LogEx(m_session_id, 2305, NULL);

        return SHORT_REPLY(status);
    }

    //
    // make sure this is a SBIE_INI_SETTING_REQ request, and then
    // validate the parameters
    //

    if (msg->msgid == MSGID_SBIE_INI_SET_SETTING ||
        msg->msgid == MSGID_SBIE_INI_ADD_SETTING ||
        msg->msgid == MSGID_SBIE_INI_INS_SETTING ||
        msg->msgid == MSGID_SBIE_INI_DEL_SETTING) {

        status = CheckRequest(msg);
    } else
        status = STATUS_INVALID_SYSTEM_SERVICE;

    if (! NT_SUCCESS(status))
        return SHORT_REPLY(status);

    RevertToSelf();

    //
    // handle request to alter configuration
    //

    if (msg->msgid == MSGID_SBIE_INI_SET_SETTING)
        status = SetSetting(msg);
    else if (msg->msgid == MSGID_SBIE_INI_ADD_SETTING)
        status = AddSetting(msg, false);
    else if (msg->msgid == MSGID_SBIE_INI_INS_SETTING)
        status = AddSetting(msg, true);
    else if (msg->msgid == MSGID_SBIE_INI_DEL_SETTING)
        status = DelSetting(msg);

    if (m_text && NT_SUCCESS(status))
        status = RefreshConf();

    if (status == STATUS_INSUFFICIENT_RESOURCES)
        SbieApi_LogEx(m_session_id, 2305, NULL);

    return SHORT_REPLY(status);
}


//---------------------------------------------------------------------------
// GetVersion
//---------------------------------------------------------------------------


MSG_HEADER *SbieIniServer::GetVersion(MSG_HEADER *msg)
{
    WCHAR ver_str[16];
    wsprintf(ver_str, L"%S", MY_VERSION_COMPAT);

    ULONG ver_len = wcslen(ver_str);
    ULONG rpl_len = sizeof(SBIE_INI_GET_USER_RPL)
                  + (ver_len + 1) * sizeof(WCHAR);
    SBIE_INI_GET_VERSION_RPL *rpl =
        (SBIE_INI_GET_VERSION_RPL *)LONG_REPLY(rpl_len);
    if (! rpl)
        return SHORT_REPLY(STATUS_INSUFFICIENT_RESOURCES);

    wcscpy(rpl->version, ver_str);
    rpl->version_len = ver_len;

    return &rpl->h;
}


//---------------------------------------------------------------------------
// GetWaitHandle
//---------------------------------------------------------------------------


MSG_HEADER *SbieIniServer::GetWaitHandle(HANDLE idProcess)
{
    ULONG LastError;
    HANDLE hCallerProcess =
        OpenProcess(PROCESS_DUP_HANDLE, FALSE, (ULONG)(ULONG_PTR)idProcess);
    if (! hCallerProcess)
        LastError = GetLastError();
    else {
        HANDLE pHandle;
        BOOL ok = DuplicateHandle(GetCurrentProcess(), GetCurrentProcess(),
                          hCallerProcess, &pHandle, SYNCHRONIZE, FALSE, 0);
        if (! ok)
            LastError = GetLastError();
        CloseHandle(hCallerProcess);
        if (ok) {
            ULONG rpl_len = sizeof(SBIE_INI_GET_WAIT_HANDLE_RPL);
            SBIE_INI_GET_WAIT_HANDLE_RPL *rpl =
                (SBIE_INI_GET_WAIT_HANDLE_RPL *)LONG_REPLY(rpl_len);
            if (! rpl)
                LastError = GetLastError();
            else {
                rpl->hProcess = pHandle;
                return &rpl->h;
            }
        }
    }
    return SHORT_REPLY(LastError);
}


//---------------------------------------------------------------------------
// GetUser
//---------------------------------------------------------------------------


MSG_HEADER *SbieIniServer::GetUser(MSG_HEADER *msg)
{
    HANDLE hToken;
    BOOL ok1 = OpenThreadToken(
                        GetCurrentThread(), TOKEN_QUERY, FALSE, &hToken);
    if (! ok1)
        return SHORT_REPLY(STATUS_NO_TOKEN);

    bool ok2 = SetUserSettingsSectionName(hToken);

    BOOLEAN admin = FALSE;
    if (ok2 && TokenIsAdmin(hToken))
        admin = TRUE;

    CloseHandle(hToken);

    if (! ok2)
        return SHORT_REPLY(STATUS_NO_TOKEN);

    ULONG name_len = wcslen(m_username);
    ULONG rpl_len  = sizeof(SBIE_INI_GET_USER_RPL)
                   + (name_len + 1) * sizeof(WCHAR);
    SBIE_INI_GET_USER_RPL *rpl =
        (SBIE_INI_GET_USER_RPL *)LONG_REPLY(rpl_len);
    if (! rpl)
        return SHORT_REPLY(STATUS_INSUFFICIENT_RESOURCES);

    rpl->admin = admin;
    wcscpy(rpl->section, m_sectionname);
    wcscpy(rpl->name,    m_username);
    rpl->name_len = name_len;

    return &rpl->h;
}


//---------------------------------------------------------------------------
// GetPath
//---------------------------------------------------------------------------


MSG_HEADER *SbieIniServer::GetPath(MSG_HEADER *msg)
{
    WCHAR *IniPath, *TmpPath;
    BOOLEAN IsHomePath = FALSE;

    RevertToSelf();

    if (GetIniPath(&IniPath, &TmpPath, &IsHomePath)) {

        ULONG rpl_len = sizeof(SBIE_INI_GET_PATH_RPL)
                      + (wcslen(IniPath) + 1) * sizeof(WCHAR);
        SBIE_INI_GET_PATH_RPL *rpl =
            (SBIE_INI_GET_PATH_RPL *)LONG_REPLY(rpl_len);

        if (rpl) {
            rpl->is_home_path = IsHomePath;
            wcscpy(rpl->path, IniPath);
        }

        HeapFree(GetProcessHeap(), 0, IniPath);
        HeapFree(GetProcessHeap(), 0, TmpPath);

        if (rpl)
            return (MSG_HEADER *)rpl;
    }

    return SHORT_REPLY(STATUS_INSUFFICIENT_RESOURCES);
}


//---------------------------------------------------------------------------
// CheckRequest
//---------------------------------------------------------------------------


ULONG SbieIniServer::CheckRequest(MSG_HEADER *msg)
{
    SBIE_INI_SETTING_REQ *req = (SBIE_INI_SETTING_REQ *)msg;
    if (req->h.length < sizeof(SBIE_INI_SETTING_REQ))
        return STATUS_INVALID_PARAMETER;

    ULONG value_len = req->value_len * sizeof(WCHAR);
    if (value_len > PIPE_MAX_DATA_LEN)
        return STATUS_INVALID_PARAMETER;
    ULONG offset = FIELD_OFFSET(SBIE_INI_SETTING_REQ, value);
    if (offset + value_len > req->h.length)
        return STATUS_INVALID_PARAMETER;
    if (req->value[req->value_len])
        return STATUS_INVALID_PARAMETER;

    if (! req->section[0])
        wcscpy(req->section, L"GlobalSettings");

    HANDLE hToken;
    BOOL ok = OpenThreadToken(
                        GetCurrentThread(), TOKEN_QUERY, FALSE, &hToken);
    if (! ok)
        return STATUS_NO_TOKEN;

    if (_wcsnicmp(req->section, L"UserSettings_", 13) == 0) {

        if (! SetUserSettingsSectionName(hToken)) {
            CloseHandle(hToken);
            return STATUS_NO_TOKEN;
        }
        wcscpy(req->section, m_sectionname);

    } else {

        ULONG status = IsCallerAuthorized(hToken, req->password);
        if (status != 0)
            return status;
    }

    CloseHandle(hToken);

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// SetUserSettingsSectionName
//---------------------------------------------------------------------------


bool SbieIniServer::SetUserSettingsSectionName(HANDLE hToken)
{
    union {
        TOKEN_USER user;
        UCHAR space[128];
        WCHAR value[4];
    } info;

    m_admin = FALSE;

    //
    // if the UserSettings_Portable section exists, use that
    //

    const WCHAR *_portable = L"UserSettings_Portable";

    NTSTATUS status = SbieApi_QueryConfAsIs(
                        _portable, NULL, 0, info.value, sizeof(info.value));
    if (status == STATUS_SUCCESS || status == STATUS_BUFFER_TOO_SMALL) {

        wcscpy(m_sectionname, _portable);
        wcscpy(m_username,    _portable + 13);  // L"Portable"

        return true;
    }

    //
    // get user name of caller
    //

    ULONG info_len = sizeof(info);
    BOOL ok = GetTokenInformation(
                            hToken, TokenUser, &info, info_len, &info_len);
    if (! ok)
        return false;

    ULONG username_len = sizeof(m_username) / sizeof(WCHAR) - 4;
    WCHAR domain[256];
    ULONG domain_len = sizeof(domain) / sizeof(WCHAR) - 4;
    SID_NAME_USE use;

    m_username[0] = L'\0';

    ok = LookupAccountSid(NULL, info.user.User.Sid,
            m_username, &username_len, domain, &domain_len, &use);
    if (! ok || ! m_username[0])
        return false;

    m_username[sizeof(m_username) / sizeof(WCHAR) - 4] = L'\0';
    _wcslwr(m_username);

    //
    // compute crc
    //

    ULONG crc = CRC_Adler32(
        (UCHAR *)m_username, wcslen(m_username) * sizeof(WCHAR));
    wsprintf(m_sectionname, L"UserSettings_%08X", crc);

    //
    // translate spaces and backslashes in username to underscores
    //

    while (1) {
        WCHAR *ptr = wcschr(m_username, L'\\');
        if (! ptr)
            ptr = wcschr(m_username, L' ');
        if (! ptr)
            break;
        *ptr = L'_';
    }

    return true;
}


//---------------------------------------------------------------------------
// TokenIsAdmin
//---------------------------------------------------------------------------


bool SbieIniServer::TokenIsAdmin(HANDLE hToken, bool OnlyFull)
{
    //
    // check if token is member of the Administrators group
    //

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    BOOL b = AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS,
                0, 0, 0, 0, 0, 0,
                &AdministratorsGroup);
    if (b) {
        if (! CheckTokenMembership(NULL, AdministratorsGroup, &b))
            b = FALSE;
        FreeSid(AdministratorsGroup);

        //
        // on Windows Vista, check for UAC split token
        //

        if (! b || OnlyFull) {
            OSVERSIONINFO osvi;
            osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
            if (GetVersionEx(&osvi) && osvi.dwMajorVersion >= 6) {
                ULONG elevationType, len;
                b = GetTokenInformation(
                        hToken, (TOKEN_INFORMATION_CLASS)TokenElevationType,
                        &elevationType, sizeof(elevationType), &len);
                if (b && (elevationType != TokenElevationTypeFull &&
                          (OnlyFull || elevationType != TokenElevationTypeLimited)))
                    b = FALSE;
            }
        }
    }

    return b ? true : false;
}


//---------------------------------------------------------------------------
// HashPassword
//---------------------------------------------------------------------------


bool SbieIniServer::HashPassword(const WCHAR *Password, WCHAR *Hash41)
{
    HCRYPTPROV hCryptProv;
    HCRYPTHASH hCryptHash;
    UCHAR data[20];
    ULONG data_len;
    BOOL ok;
    ULONG ErrorLevel;
    ULONG i;
    WCHAR *HashPtr;

    hCryptProv = NULL;
    hCryptHash = NULL;

    ok = CryptAcquireContext(
                &hCryptProv, NULL, MS_DEF_PROV, PROV_RSA_FULL,
                CRYPT_VERIFYCONTEXT | CRYPT_SILENT);
    if (! ok) {
        ErrorLevel = 0x11;
        goto finish;
    }

    ok = CryptCreateHash(hCryptProv, CALG_SHA1, 0, 0, &hCryptHash);
    if (! ok) {
        ErrorLevel = 0x22;
        goto finish;
    }

    data_len = wcslen(Password);
    if (data_len > 64)
        data_len = 64;
    data_len *= sizeof(WCHAR);

    ok = CryptHashData(hCryptHash, (BYTE *)Password, data_len, 0);
    if (! ok) {
        ErrorLevel = 0x33;
        goto finish;
    }

    memzero(data, sizeof(data));
    data_len = sizeof(data);
    ok = CryptGetHashParam(hCryptHash, HP_HASHVAL, data, &data_len, 0);
    if (! ok) {
        ErrorLevel = 0x44;
        goto finish;
    }

    //
    // make hash printable as a sequence of hex digits
    //

    HashPtr = Hash41;

    for (i = 0; i < 20; ++i) {

        UCHAR NibbleH = (data[i] & 0xF0) >> 8;
        UCHAR NibbleL = (data[i] & 0x0F);

        if (NibbleH >= 10)
            *HashPtr = L'A' + NibbleH - 10;
        else
            *HashPtr = L'0' + NibbleH;
        ++HashPtr;

        if (NibbleL >= 10)
            *HashPtr = L'A' + NibbleL - 10;
        else
            *HashPtr = L'0' + NibbleL;
        ++HashPtr;
    }

    *HashPtr = L'\0';

    //
    // finish
    //

    ok = TRUE;
    ErrorLevel = 0;

finish:

    ULONG LastError = GetLastError();

    if (hCryptHash)
        CryptDestroyHash(hCryptHash);

    if (hCryptProv)
        CryptReleaseContext(hCryptProv, 0);

    if (! ok) {
        SbieApi_LogEx(
            m_session_id, 2323, L"[%d / %08X]", ErrorLevel, LastError);
    }

    return (ok ? true : false);
}


//---------------------------------------------------------------------------
// IsCallerAuthorized
//---------------------------------------------------------------------------


ULONG SbieIniServer::IsCallerAuthorized(HANDLE hToken, const WCHAR *Password)
{
    WCHAR buf[42], buf2[42];

    //
    // check for Administrator-only access
    //

    if (SbieApi_QueryConfBool(NULL, L"EditAdminOnly", FALSE)) {

        if (! TokenIsAdmin(hToken)) {
            CloseHandle(hToken);
            return STATUS_LOGON_NOT_GRANTED;
        }
    }

    //
    // check for password protection
    //

    buf[41] = L'\0';
    SbieApi_QueryConfAsIs(
        NULL, L"EditPassword", 0, buf, 41 * sizeof(WCHAR));
    if (! buf[0])
        return STATUS_SUCCESS;

    bool access_granted = false;
    if (wcslen(buf) == 40)
        if (HashPassword(Password, buf2))
            if (wmemcmp(buf, buf2, 40) == 0)
                access_granted = true;

    SecureZeroMemory(buf2, sizeof(buf2));
    SecureZeroMemory(buf, sizeof(buf));

    if (! access_granted) {
        CloseHandle(hToken);
        return STATUS_WRONG_PASSWORD;
    }

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// SetSetting
//---------------------------------------------------------------------------


ULONG SbieIniServer::SetSetting(MSG_HEADER *msg)
{
    SBIE_INI_SETTING_REQ *req = (SBIE_INI_SETTING_REQ *)msg;
    BOOLEAN have_value = (req->value_len != 0);

    //
    // for each section ...
    //

    WCHAR bracked_section_name[78];

    NTSTATUS status;
    WCHAR *setting = m_line;
    ULONG setting_idx;
    ULONG value_idx;
    bool setting_match = false;
    bool any_section_match = false;

    bool delete_section_request =
        (*req->setting == L'*' && (! have_value)) ? true : false;

    WCHAR section_name[70];
    ULONG section_idx = -1;

    while (1) {

        if (section_idx == -1) {

            wcscpy(section_name, L"GlobalSettings");

        } else {

            status = SbieApi_QueryConfAsIs(
                NULL, NULL, section_idx | CONF_GET_NO_TEMPLS,
                section_name, sizeof(WCHAR) * 66);

            if (! NT_SUCCESS(status)) {
                if (status == STATUS_RESOURCE_NAME_NOT_FOUND)
                    status = STATUS_SUCCESS;
                if (! NT_SUCCESS(status))
                    return status;
                break;
            }
        }

        ++section_idx;

        //
        // write the section header
        //

        bool section_match =
            (_wcsicmp(section_name, req->section) == 0);

        if (section_match)
            any_section_match = true;

        if (section_match && delete_section_request)
            continue;

        wcscpy(bracked_section_name, L"[");
        wcscat(bracked_section_name, section_name);
        wcscat(bracked_section_name, L"]");

        if (! AddText(L""))
            return STATUS_INSUFFICIENT_RESOURCES;
        if (! AddText(bracked_section_name))
            return STATUS_INSUFFICIENT_RESOURCES;
        if (! AddText(L""))
            return STATUS_INSUFFICIENT_RESOURCES;

        //
        // for each setting in each section ...
        //

        setting_idx = 0;

        while (1) {

            status = SbieApi_QueryConfAsIs(
                section_name, NULL, setting_idx | CONF_GET_NO_TEMPLS,
                setting, sizeof(WCHAR) * 66);

            if (! NT_SUCCESS(status)) {
                if (status != STATUS_RESOURCE_NAME_NOT_FOUND)
                    return status;
                break;
            }

            ++setting_idx;

            //
            // replace setting if matches request parameters
            //

            if (section_match && _wcsicmp(setting, req->setting) == 0) {

                if (have_value && (! setting_match)) {
                    if (! AddCallerText(setting, req->value))
                        return STATUS_INSUFFICIENT_RESOURCES;
                }

                setting_match = true;
                continue;
            }

            //
            // else for each index of each setting in each section ...
            //

            WCHAR *ptr = setting + wcslen(setting);

            value_idx = 0;

            while (1) {

                ULONG remaining_buf_len = sizeof(m_line) - sizeof(WCHAR) * 4
                                        - (ptr + 1 - setting) * sizeof(WCHAR);

                status = SbieApi_QueryConfAsIs(
                    section_name, setting,
                    value_idx | CONF_GET_NO_GLOBAL | CONF_GET_NO_TEMPLS,
                    ptr + 1, remaining_buf_len);

                if (! NT_SUCCESS(status)) {
                    if (status != STATUS_RESOURCE_NAME_NOT_FOUND)
                        return status;
                    break;
                }

                if (NT_SUCCESS(status)) {

                    *ptr = L'=';

                    if (! AddText(setting))
                        return STATUS_INSUFFICIENT_RESOURCES;

                    *ptr = L'\0';
                }

                ++value_idx;
            }
        }

        //
        // if setting was not replaced, it must be added
        //

        if (section_match && (! setting_match) && have_value) {
            wcscpy(setting, req->setting);
            if (! AddCallerText(setting, req->value))
                return STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    //
    // if section was not found, add it, and then add the setting
    //

    if ((! any_section_match) && delete_section_request)
        any_section_match = true;

    if ((! any_section_match) && req->section[0]) {

        wcscpy(bracked_section_name, L"[");
        wcscat(bracked_section_name, req->section);
        wcscat(bracked_section_name, L"]");

        if (! AddText(L""))
            return STATUS_INSUFFICIENT_RESOURCES;
        if (! AddText(bracked_section_name))
            return STATUS_INSUFFICIENT_RESOURCES;
        if (! AddText(L""))
            return STATUS_INSUFFICIENT_RESOURCES;

        if (have_value) {

            wcscpy(setting, req->setting);
            if (! AddCallerText(setting, req->value))
                return STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// AddSetting
//---------------------------------------------------------------------------


ULONG SbieIniServer::AddSetting(MSG_HEADER *msg, bool insert)
{
    //
    // if a value was not specified, do nothing
    //

    SBIE_INI_SETTING_REQ *req = (SBIE_INI_SETTING_REQ *)msg;
    if (! req->value_len)
        return STATUS_SUCCESS;

    //
    // add the new setting and collect others
    //

    NTSTATUS status;
    WCHAR *setting = m_line;
    ULONG setting_len;
    WCHAR *text = NULL;
    ULONG text_len = 0;
    ULONG index = (insert ? -1 : 0);
    ULONG seen_value = 0;

    while (1) {

        if (index == -1) {

            wcscpy(setting, req->value);
            index = 0;

        } else {

            status = SbieApi_QueryConfAsIs(
                req->section, req->setting,
                index | CONF_GET_NO_GLOBAL | CONF_GET_NO_TEMPLS,
                setting, sizeof(m_line) - sizeof(WCHAR) * 4);

            if (! NT_SUCCESS(status)) {

                if (insert)
                    break;

                wcscpy(setting, req->value);
                index = -1;

            } else
                ++index;
        }

        setting_len = wcslen(setting);

        if (setting_len >= req->value_len) {
            if (_wcsicmp(setting, req->value) == 0)
                ++seen_value;
        }

        text_len += (setting_len + 2) * sizeof(WCHAR);
        if (text) {

            WCHAR *text2 =
                (WCHAR *)HeapReAlloc(GetProcessHeap(), 0, text, text_len);
            if (! text2) {
                HeapFree(GetProcessHeap(), 0, text);
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            text = text2;
            wcscat(text, L"\n");
            wcscat(text, setting);

        } else {

            text = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, text_len);
            if (! text)
                return STATUS_INSUFFICIENT_RESOURCES;
            wcscpy(text, setting);
        }

        if (index == -1)
            break;
    }

    //
    // if the value already existed, don't bother, otherwise apply setting
    //

    if (seen_value > 1) {
        if (text)
            HeapFree(GetProcessHeap(), 0, text);
        return STATUS_SUCCESS;
    }

    return CallSetSetting(text, msg);
}


//---------------------------------------------------------------------------
// DelSetting
//---------------------------------------------------------------------------


ULONG SbieIniServer::DelSetting(MSG_HEADER *msg)
{
    //
    // if a value was not specified, go directly to SetSetting,
    // which will discard all instances of the setting
    //

    SBIE_INI_SETTING_REQ *req = (SBIE_INI_SETTING_REQ *)msg;
    if (! req->value_len)
        return SetSetting(msg);

    //
    // collect the setting and discard the instances matching the value
    //

    NTSTATUS status;
    WCHAR *setting = m_line;
    ULONG setting_len;
    WCHAR *text = NULL;
    ULONG text_len = 0;
    ULONG index = 0;
    BOOLEAN seen_value = FALSE;

    while (1) {

        status = SbieApi_QueryConfAsIs(
            req->section, req->setting,
            index | CONF_GET_NO_GLOBAL | CONF_GET_NO_TEMPLS,
            setting, sizeof(m_line) - sizeof(WCHAR) * 4);
        if (! NT_SUCCESS(status))
            break;
        ++index;

        setting_len = wcslen(setting);
        if (setting_len == req->value_len) {
            if (_wcsicmp(setting, req->value) == 0) {
                seen_value = TRUE;
                continue;
            }
        }

        text_len += (setting_len + 2) * sizeof(WCHAR);
        if (text) {

            WCHAR *text2 =
                (WCHAR *)HeapReAlloc(GetProcessHeap(), 0, text, text_len);
            if (! text2) {
                HeapFree(GetProcessHeap(), 0, text);
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            text = text2;
            wcscat(text, L"\n");
            wcscat(text, setting);

        } else {

            text = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, text_len);
            if (! text)
                return STATUS_INSUFFICIENT_RESOURCES;
            wcscpy(text, setting);
        }
    }

    //
    // if the value wasn't there, don't bother, otherwise apply setting
    //

    if (! seen_value) {
        if (text)
            HeapFree(GetProcessHeap(), 0, text);
        return STATUS_SUCCESS;
    }

    return CallSetSetting(text, msg);
}


//---------------------------------------------------------------------------
// SetTemplate
//---------------------------------------------------------------------------


ULONG SbieIniServer::SetTemplate(MSG_HEADER *msg)
{
    SBIE_INI_TEMPLATE_REQ *req = (SBIE_INI_TEMPLATE_REQ *)msg;
    if (req->h.length < sizeof(SBIE_INI_TEMPLATE_REQ))
        return STATUS_INVALID_PARAMETER;

    ULONG value_len = req->value_len * sizeof(WCHAR);
    if (value_len > PIPE_MAX_DATA_LEN)
        return STATUS_INVALID_PARAMETER;
    ULONG offset = FIELD_OFFSET(SBIE_INI_TEMPLATE_REQ, value);
    if (offset + value_len > req->h.length)
        return STATUS_INVALID_PARAMETER;
    if (req->value[req->value_len])
        return STATUS_INVALID_PARAMETER;

    HANDLE hToken;
    BOOL ok = OpenThreadToken(
                        GetCurrentThread(), TOKEN_QUERY, FALSE, &hToken);
    if (! ok)
        return STATUS_NO_TOKEN;

    if (req->user) {

        if (! SetUserSettingsSectionName(hToken)) {
            CloseHandle(hToken);
            return STATUS_NO_TOKEN;
        }

        if (wcslen(req->varname) + 1 + wcslen(m_username) > 62) {
            CloseHandle(hToken);
            return STATUS_NAME_TOO_LONG;
        }

    } else {

        ULONG status = IsCallerAuthorized(hToken, req->password);
        if (status != 0)
            return status;
    }

    CloseHandle(hToken);

    RevertToSelf();

    //
    //
    //

    NTSTATUS status;
    ULONG req2_len = sizeof(SBIE_INI_SETTING_REQ)
                   + value_len * sizeof(WCHAR);
    SBIE_INI_SETTING_REQ *req2 =
        (SBIE_INI_SETTING_REQ *)HeapAlloc(GetProcessHeap(), 0, req2_len);
    if (! req2)
        status = STATUS_INSUFFICIENT_RESOURCES;
    else {

        req2->h.msgid = MSGID_SBIE_INI_SET_SETTING;
        req2->h.length = req2_len;

        wcscpy(req2->section, L"TemplateSettings");

        wcscpy(req2->setting, req->varname);
        if (req->user) {
            WCHAR *ptr = req2->setting + wcslen(req2->setting);
            *ptr = L'.';
            wcscpy(ptr + 1, m_username);
        }

        req2->value_len = req->value_len;
        wcscpy(req2->value, req->value);

        status = SetSetting(&req2->h);

        HeapFree(GetProcessHeap(), 0, req2);
    }

    return status;
}


//---------------------------------------------------------------------------
// SetOrTestPassword
//---------------------------------------------------------------------------


ULONG SbieIniServer::SetOrTestPassword(MSG_HEADER *msg)
{
    SBIE_INI_PASSWORD_REQ *req = (SBIE_INI_PASSWORD_REQ *)msg;
    if (req->h.length < sizeof(SBIE_INI_PASSWORD_REQ))
        return STATUS_INVALID_PARAMETER;

    HANDLE hToken;
    BOOL ok = OpenThreadToken(
                        GetCurrentThread(), TOKEN_QUERY, FALSE, &hToken);
    if (! ok)
        return STATUS_NO_TOKEN;

    ULONG status = IsCallerAuthorized(hToken, req->old_password);

    if (status == 0)
        CloseHandle(hToken);

    RevertToSelf();

    //
    // handle a set password request
    // (we already did everything for a test password request)
    //

    if (status == 0 && msg->msgid == MSGID_SBIE_INI_SET_PASSWORD) {

        ULONG req2_len = sizeof(SBIE_INI_SETTING_REQ)
                       + 128  * sizeof(WCHAR);
        SBIE_INI_SETTING_REQ *req2 =
            (SBIE_INI_SETTING_REQ *)HeapAlloc(GetProcessHeap(), 0, req2_len);
        if (! req2)
            status = STATUS_INSUFFICIENT_RESOURCES;
        else {

            req2->h.msgid = MSGID_SBIE_INI_SET_SETTING;
            req2->h.length = req2_len;

            wcscpy(req2->section, L"GlobalSettings");

            wcscpy(req2->setting, L"EditPassword");

            req2->value_len = 40;
            if (HashPassword(req->new_password, req2->value)) {

                if (! *req->new_password) {
                    req2->value_len = 0;
                    req2->value[0] = L'\0';
                }

                status = SetSetting(&req2->h);

            } else
                status = STATUS_ACCESS_DENIED;

            HeapFree(GetProcessHeap(), 0, req2);
        }
    }

    return status;
}


//---------------------------------------------------------------------------
// CallSetSetting
//---------------------------------------------------------------------------


ULONG SbieIniServer::CallSetSetting(WCHAR *text, MSG_HEADER *msg)
{
    SBIE_INI_SETTING_REQ *req = (SBIE_INI_SETTING_REQ *)msg;

    //
    // build a new request and call SetSetting
    //

    ULONG status;
    ULONG text_len = 0;
    if (text)
        text_len = (wcslen(text) + 1) * sizeof(WCHAR);
    ULONG req2_len = sizeof(SBIE_INI_SETTING_REQ) + text_len;

    SBIE_INI_SETTING_REQ *req2 =
        (SBIE_INI_SETTING_REQ *)HeapAlloc(GetProcessHeap(), 0, req2_len);
    if (! req2)
        status = STATUS_INSUFFICIENT_RESOURCES;
    else {

        req2->h.msgid = MSGID_SBIE_INI_SET_SETTING;
        req2->h.length = req2_len;

        wcscpy(req2->section, req->section);
        wcscpy(req2->setting, req->setting);

        if (text) {
            wcscpy(req2->value,   text);
            req2->value_len = wcslen(text);
        } else {
            req2->value[0] = L'\0';
            req2->value_len = 0;
        }

        status = SetSetting(&req2->h);

        HeapFree(GetProcessHeap(), 0, req2);
    }

    if (text)
        HeapFree(GetProcessHeap(), 0, text);

    return status;
}


//---------------------------------------------------------------------------
// AddText
//---------------------------------------------------------------------------


bool SbieIniServer::AddText(const WCHAR *line)
{
    //static const WCHAR *_ByteOrderMark = L"ByteOrderMark=";
    //static ULONG _ByteOrderMarkLen = 0;

    ULONG line_len = wcslen(line);

    if ((! m_text_base) ||
        (m_text - m_text_base + line_len + 32 > m_text_max_len)) {

        ULONG text_max_len = m_text_max_len + line_len + 1024;
        WCHAR *text_base = (WCHAR *)HeapAlloc(
            GetProcessHeap(), 0, text_max_len * sizeof(WCHAR));
        if (! text_base)
            return false;

        if (m_text_base) {
            wmemcpy(text_base, m_text_base, m_text - m_text_base);
            HeapFree(GetProcessHeap(), 0, m_text_base);
        }

        m_text = (m_text - m_text_base) + text_base;
        m_text_base = text_base;
        m_text_max_len = text_max_len;
    }

    wmemcpy(m_text, line, line_len);
    m_text += line_len;
    m_text[0] = L'\r';
    m_text[1] = L'\n';
    m_text[2] = L'\0';
    m_text += 2;

    //if (! _ByteOrderMarkLen)
    //    _ByteOrderMarkLen = wcslen(_ByteOrderMark);
    //if (_wcsnicmp(line, _ByteOrderMark, _ByteOrderMarkLen) == 0) {
    //    const WCHAR ch = line[_ByteOrderMarkLen];
    //    if (ch == L'y' || ch == L'Y')
    //        m_insertbom = TRUE;
    //}

    return true;
}


//---------------------------------------------------------------------------
// AddCallerText
//---------------------------------------------------------------------------


bool SbieIniServer::AddCallerText(WCHAR *setting, WCHAR *value)
{
    WCHAR *ptr = setting + wcslen(setting);
    *ptr = L'=';
    ++ptr;

    while (*value) {

        ULONG cpylen, skiplen;
        WCHAR *cr = wcschr(value, L'\n');
        if (cr) {
            cpylen = (ULONG)(cr - value);
            skiplen = cpylen + 1;
        } else {
            cpylen = wcslen(value);
            skiplen = cpylen;
        }
        if (cpylen > 1900)  // see also CONF_LINE_LEN (2000) in SbieDrv
            cpylen = 1900;
        wmemcpy(ptr, value, cpylen);
        ptr[cpylen] = L'\0';

        if (! AddText(setting))
            return false;

        value += skiplen;
    }

    return true;
}


//---------------------------------------------------------------------------
// RefreshConf
//---------------------------------------------------------------------------


ULONG SbieIniServer::RefreshConf()
{
    WCHAR *IniPath, *TmpPath;
    BOOLEAN IsUTF8 = FALSE;
    if (! GetIniPath(&IniPath, &TmpPath, NULL, &IsUTF8))
        return STATUS_INSUFFICIENT_RESOURCES;

    HANDLE hFile = INVALID_HANDLE_VALUE;
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    bool RestoreOnError = false;

    //
    // check that attributes are valid, and remove any system attributes
    //

    ULONG attrs = GetFileAttributes(IniPath);
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        ULONG LastError = GetLastError();
        attrs = 0;
        if (LastError != ERROR_FILE_NOT_FOUND) {
            SbieApi_LogEx(m_session_id, 2322, L"[11 / %d]", LastError);
            goto finish;
        }
    }

    const ULONG CONFLICTING_ATTRS = (   FILE_ATTRIBUTE_DEVICE |
                                        FILE_ATTRIBUTE_DIRECTORY |
                                        FILE_ATTRIBUTE_REPARSE_POINT |
                                        FILE_ATTRIBUTE_ENCRYPTED |
                                        FILE_ATTRIBUTE_OFFLINE);

    if (attrs & CONFLICTING_ATTRS) {
        SbieApi_LogEx(m_session_id, 2322, L"[12 / %d]", GetLastError());
        attrs = 0;
        goto finish;
    }

    const ULONG REMOVABLE_ATTRS = (     FILE_ATTRIBUTE_READONLY |
                                        FILE_ATTRIBUTE_HIDDEN |
                                        FILE_ATTRIBUTE_SYSTEM);

    if (attrs & REMOVABLE_ATTRS) {
        if (! SetFileAttributes(IniPath, attrs & ~REMOVABLE_ATTRS)) {
            SbieApi_LogEx(m_session_id, 2322, L"[13 / %d]", GetLastError());
            goto finish;
        }
    }

    //
    // create a backup copy
    //

    if (CopyFile(IniPath, TmpPath, FALSE))
        RestoreOnError = true;
    else {
        if (GetLastError() != ERROR_FILE_NOT_FOUND) {
            SbieApi_LogEx(m_session_id, 2322, L"[14 / %d]", GetLastError());
            goto finish;
        }
    }

    //
    // overwrite Sandboxie.ini
    //

    UnlockConf();

    hFile = CreateFile(
        IniPath, FILE_GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        SbieApi_LogEx(m_session_id, 2322, L"[15 / %d]", GetLastError());
        goto finish;
    }

    if (IsUTF8)
    {
        // UTF-8 Signature
        static const UCHAR bom[3] = { 0xEF, 0xBB, 0xBF };
        ULONG lenDummy;
        WriteFile(hFile, bom, sizeof(bom), &lenDummy, NULL);
    }
    else
    //if (m_insertbom) 
    {
        // UNICODE Byte Order Mark (little endian)
        static const UCHAR bom[2] = { 0xFF, 0xFE };
        ULONG lenDummy;
        WriteFile(hFile, bom, sizeof(bom), &lenDummy, NULL);
    }

    ULONG lenToWrite = wcslen(m_text_base) * sizeof(WCHAR);
    
    char* text_utf8 = NULL;
    if (IsUTF8)
    {
        ULONG utf8_len = WideCharToMultiByte(CP_UTF8, 0, m_text_base, lenToWrite / sizeof(WCHAR), NULL, 0, NULL, NULL);
        text_utf8 = (char*)HeapAlloc(GetProcessHeap(), 0, utf8_len);
        lenToWrite = WideCharToMultiByte(CP_UTF8, 0, m_text_base, lenToWrite / sizeof(WCHAR), text_utf8, utf8_len, NULL, NULL);
    }

    ULONG lenWritten = 0;
    if (! WriteFile(hFile, text_utf8 ? (void*)text_utf8 : (void*)m_text_base, lenToWrite, &lenWritten, NULL))
        lenWritten = -1;

    if(text_utf8)
        HeapFree(GetProcessHeap(), 0, text_utf8);

    if (lenWritten != lenToWrite)
        SbieApi_LogEx(m_session_id, 2322, L"[16 / %d]", GetLastError());
    else {
        if (! SetEndOfFile(hFile)) {
            lenWritten = -1;
            SbieApi_LogEx(m_session_id, 2322, L"[17 / %d]", GetLastError());
        }
    }
    CloseHandle(hFile);
    hFile = INVALID_HANDLE_VALUE;
    if (lenWritten != lenToWrite)
        goto finish;

    //
    // apply new Sandboxie.ini into the driver
    //

    status = SbieApi_ReloadConf(m_session_id, 0);

finish:

    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);

    if ((! NT_SUCCESS(status)) && RestoreOnError) {
        if (! CopyFile(TmpPath, IniPath, FALSE))
            SbieApi_LogEx(m_session_id, 2322, L"[18 / %d]", GetLastError());
    }

    if (attrs & REMOVABLE_ATTRS)
        SetFileAttributes(IniPath, attrs);

    DeleteFile(TmpPath);

    LockConf(IniPath);

    HeapFree(GetProcessHeap(), 0, IniPath);
    HeapFree(GetProcessHeap(), 0, TmpPath);

    return status;
}


//---------------------------------------------------------------------------
// GetIniPath
//---------------------------------------------------------------------------


bool SbieIniServer::GetIniPath(WCHAR **IniPath, WCHAR **TmpPath,
                               BOOLEAN *IsHomePath, BOOLEAN* IsUTF8)
{
    static const WCHAR *_ini = SANDBOXIE_INI;
    WCHAR *path = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, 2048);
    if (! path)
        return false;
    WCHAR *tmp = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, 2048);
    if (! tmp) {
        HeapFree(GetProcessHeap(), 0, path);
        return false;
    }

    // the Sandbox driver tells us if the ini file comes from the
    // program home directory or from the Windows directory, and
    // we use that information to select the output path

    if (IsUTF8 != NULL) {
        LONG rc = SbieApi_QueryConfAsIs(NULL, L"IniEncoding", 0, path, 8);
        if (rc == 0 && *path == L'8')
            *IsUTF8 = TRUE;
    }

    LONG rc = SbieApi_QueryConfAsIs(NULL, L"IniLocation", 0, path, 8);
    if (rc == 0 && *path == L'H') {

        //
        // Sandboxie.ini was last read from Sandboxie home directory
        //

        STARTUPINFOW si;
        if (SbieDll_RunFromHome(_ini, NULL, &si, NULL)) {
            WCHAR *path2 = (WCHAR *)si.lpReserved;
            wcscpy(path, path2);
            HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, path2);

            if (IsHomePath)
                *IsHomePath = TRUE;
        }
    }
    else if (rc == 0 && *path == L'W') {
        if (! GetWindowsDirectory(path, MAX_PATH))
            wcscpy(path, L"C:\\WINDOWS");
        if (path[wcslen(path) - 1] != L'\\')
            wcscat(path, L"\\");
        wcscat(path, _ini);
    }
    else
    {
        HeapFree(GetProcessHeap(), 0, path);
        HeapFree(GetProcessHeap(), 0, tmp);

        return false;
    }

    //
    // generate the temp file name
    //

    wcscpy(tmp, path);
    WCHAR *ptr = wcsrchr(tmp, L'.');
    wsprintf(ptr, L".tmp-%d", GetTickCount());

    *IniPath = path;
    *TmpPath = tmp;

    return true;
}


//---------------------------------------------------------------------------
// LockConf
//---------------------------------------------------------------------------


void SbieIniServer::LockConf(WCHAR *IniPath)
{
    WCHAR buf[80];
    LONG rc;

    if (m_hLockFile != INVALID_HANDLE_VALUE)
        return;

    rc = SbieApi_QueryConfAsIs(NULL, L"EditPassword", 0, buf, sizeof(buf));
    if (rc != 0) {
        m_hLockFile = INVALID_HANDLE_VALUE;
        return;
    }

    WCHAR *TmpPath;
    bool FreePaths = false;
    if (! IniPath) {
        if (! GetIniPath(&IniPath, &TmpPath))
            return;
        FreePaths = TRUE;
    }

    m_hLockFile = CreateFile(
        IniPath, FILE_GENERIC_READ, FILE_SHARE_READ,
        NULL, OPEN_EXISTING, 0, NULL);

    if (FreePaths) {
        HeapFree(GetProcessHeap(), 0, IniPath);
        HeapFree(GetProcessHeap(), 0, TmpPath);
    }
}


//---------------------------------------------------------------------------
// UnlockConf
//---------------------------------------------------------------------------


void SbieIniServer::UnlockConf()
{
    if (m_hLockFile != INVALID_HANDLE_VALUE) {
        CloseHandle(m_hLockFile);
        m_hLockFile = INVALID_HANDLE_VALUE;
    }
}


//---------------------------------------------------------------------------
// RunSbieCtrl
//---------------------------------------------------------------------------


MSG_HEADER *SbieIniServer::RunSbieCtrl(HANDLE idProcess, bool isSandboxed)
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    HANDLE hToken = NULL;
    BOOL ok = TRUE;
    WCHAR ctrlName[64] = { 0 };

    //
    // get token from caller session or caller process.  note that on
    // Windows Vista, we don't accept a request from session 0
    //

    OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (GetVersionEx(&osvi) && osvi.dwMajorVersion >= 6) {

        if (! m_session_id)
            ok = FALSE;
    }

    if (ok) {

        ok = WTSQueryUserToken(m_session_id, &hToken);
    }

    if (! ok)
        hToken = NULL;

    //
    // abort if Sandboxie Control is already running in the target session
    //

    if (ok) {

        HANDLE SbieCtrlProcessId;
        SbieApi_SessionLeader(hToken, &SbieCtrlProcessId);
        if (SbieCtrlProcessId) {
            status = STATUS_IMAGE_ALREADY_LOADED;
            ok = FALSE;
        }
    }

    //
    // abort if Sandboxie Control should not auto start
    //

    if (ok && isSandboxed) {

        const WCHAR *_Setting = SBIECTRL_ L"EnableAutoStart";
        const WCHAR* _Setting2 = SBIECTRL_ L"AutoStartAgent";
        WCHAR buf[10], ch = 0;
        bool ok2 = SetUserSettingsSectionName(hToken);
        if (ok2) {
            SbieApi_QueryConfAsIs(
                m_sectionname, _Setting, 0, buf, sizeof(buf) - 2);
            ch = towlower(buf[0]);

            SbieApi_QueryConfAsIs(
                m_sectionname, _Setting2, 0, ctrlName, sizeof(ctrlName) - 2);
        }
        if (! ch) {
            wcscpy(m_sectionname + 13, L"Default");   // UserSettings_Default
            SbieApi_QueryConfAsIs(
                m_sectionname, _Setting, 0, buf, 8 * sizeof(WCHAR));
            ch = towlower(buf[0]);

            SbieApi_QueryConfAsIs(
                m_sectionname, _Setting2, 0, ctrlName, sizeof(ctrlName) - 2);
        }

        if (ch == L'n') {
            status = STATUS_LOGON_NOT_GRANTED;
            ok = FALSE;
        }
    }

    //
    // run Sandboxie Control in caller session
    //

    if (ok) {

        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        WCHAR *args;
        if (isSandboxed)
            args = NULL;
        else if (*ctrlName)
            args = L" -autorun";
        else
            args = L" /open /sync";

        if (SbieDll_RunFromHome(*ctrlName ? ctrlName : SBIECTRL_EXE, args, &si, NULL)) {

            WCHAR *CmdLine = (WCHAR *)si.lpReserved;

            void *Environ;
            if (! CreateEnvironmentBlock(&Environ, hToken, FALSE))
                Environ = NULL;

            memzero(&si, sizeof(STARTUPINFO));
            si.cb = sizeof(STARTUPINFO);
            si.dwFlags = STARTF_FORCEOFFFEEDBACK;

            ok = CreateProcessAsUser(
                    hToken, NULL, CmdLine, NULL, NULL, FALSE,
                    (Environ ? CREATE_UNICODE_ENVIRONMENT : 0),
                    Environ, NULL, &si, &pi);

            if (Environ)
                DestroyEnvironmentBlock(Environ);

            HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, CmdLine);

            if (ok) {

                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);

                status = STATUS_SUCCESS;
            }
        }
    }

    //
    // finish
    //

    if (hToken)
        CloseHandle(hToken);

    return SHORT_REPLY(status);
}
