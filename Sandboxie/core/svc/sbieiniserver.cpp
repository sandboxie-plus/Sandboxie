/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2025 David Xanatos, xanasoft.com
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
#define RC4_HEADER_ONLY
#include "common/rc4.c"
#include "core/drv/api_defs.h"
#include "DriverAssist.h"

#ifdef NEW_INI_MODE
extern "C" {
    #include "common/stream.h"
}
#include "common/ini.h"

SbieIniServer* SbieIniServer::m_instance = NULL;
#endif


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


SbieIniServer::SbieIniServer(PipeServer *pipeServer)
{
    InitializeCriticalSection(&m_critsec);

#ifdef NEW_INI_MODE
    m_instance = this;

    m_pConfigIni = NULL;
#else
    m_text = NULL;
#endif

    m_hLockFile = INVALID_HANDLE_VALUE;
    LockConf(NULL);

    pipeServer->Register(MSGID_SBIE_INI, this, Handler);
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


SbieIniServer::~SbieIniServer()
{
    m_instance = this; // fix-me: potential race condition, but this does not matter as we don't use the destructor anyways

    EnterCriticalSection(&m_instance->m_critsec);
    
    delete m_pConfigIni;

    LeaveCriticalSection(&m_instance->m_critsec);

	// cleanup CS
	DeleteCriticalSection(&m_instance->m_critsec);
}


//---------------------------------------------------------------------------
// Handler
//---------------------------------------------------------------------------


MSG_HEADER *SbieIniServer::Handler(void *_this, MSG_HEADER *msg)
{
    SbieIniServer *pThis = (SbieIniServer *)_this;

    EnterCriticalSection(&pThis->m_critsec);

#ifndef NEW_INI_MODE
    pThis->m_text = NULL;
    pThis->m_text_base = NULL;
    pThis->m_text_max_len = 0;
    //pThis->m_insertbom = FALSE;
#endif

    MSG_HEADER *rpl = pThis->Handler2(msg);

#ifndef NEW_INI_MODE
    if (pThis->m_text)
        HeapFree(GetProcessHeap(), 0, pThis->m_text_base);

    pThis->m_text = NULL;
    pThis->m_text_base = NULL;
    pThis->m_text_max_len = 0;
#endif

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

        return RunSbieCtrl(msg, idProcess, NT_SUCCESS(status));
    }

    if (msg->msgid == MSGID_SBIE_INI_RC4_CRYPT) {

        return RC4Crypt(msg, idProcess, NT_SUCCESS(status));
    }

    if (NT_SUCCESS(status))     // if sandboxed
        return SHORT_REPLY(STATUS_NOT_SUPPORTED);

    //
    // Get/Set *.dat files in sandboxie's home directory
    //

    if (msg->msgid == MSGID_SBIE_INI_SET_DAT) {

        return SetDatFile(msg, idProcess);
    }
    //if (msg->msgid == MSGID_SBIE_INI_GET_DAT) {
    //
    //    return GetDatFile(msg, idProcess);
    //}

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
    // the below opcodes require the ini to be cached
    //

    if (m_pConfigIni == NULL) {

        if (!NT_SUCCESS(status = CacheConfig()))
            return SHORT_REPLY(status);
    }

    //
    // handle a MSGID_SBIE_INI_GET_SETTING request
    //

    if (msg->msgid == MSGID_SBIE_INI_GET_SETTING) {

        return GetSetting(msg);
    }

    //
    // handle a SBIE_INI_TEMPLATE_REQ request
    //

    if (msg->msgid == MSGID_SBIE_INI_TEMPLATE) {

        status = SetTemplate(msg);

        if (NT_SUCCESS(status))
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

        if (NT_SUCCESS(status))
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
    
    SBIE_INI_SETTING_REQ *req = (SBIE_INI_SETTING_REQ *)msg;
    if (msg->msgid == MSGID_SBIE_INI_SET_SETTING)
        status = SetSetting(msg);
    else if (msg->msgid == MSGID_SBIE_INI_ADD_SETTING)
        status = AddSetting(msg, false);
    else if (msg->msgid == MSGID_SBIE_INI_INS_SETTING)
        status = AddSetting(msg, true);
    else if (msg->msgid == MSGID_SBIE_INI_DEL_SETTING)
        status = DelSetting(msg);

    if (NT_SUCCESS(status) && req->refresh)
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
    wsprintf(ver_str, L"%S", MY_VERSION_STRING);

    ULONG ver_len = wcslen(ver_str);
    ULONG rpl_len = sizeof(SBIE_INI_GET_USER_RPL)
                  + (ver_len + 1) * sizeof(WCHAR);
    SBIE_INI_GET_VERSION_RPL *rpl =
        (SBIE_INI_GET_VERSION_RPL *)LONG_REPLY(rpl_len);
    if (! rpl)
        return SHORT_REPLY(STATUS_INSUFFICIENT_RESOURCES);

    wcscpy(rpl->version, ver_str);
    rpl->abi_ver = MY_ABI_VERSION;

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
    WCHAR *IniPath;
    BOOLEAN IsHomePath = FALSE;

    RevertToSelf();

    if (GetIniPath(&IniPath, &IsHomePath)) {

        ULONG rpl_len = sizeof(SBIE_INI_GET_PATH_RPL)
                      + (wcslen(IniPath) + 1) * sizeof(WCHAR);
        SBIE_INI_GET_PATH_RPL *rpl =
            (SBIE_INI_GET_PATH_RPL *)LONG_REPLY(rpl_len);

        if (rpl) {
            rpl->is_home_path = IsHomePath;
            wcscpy(rpl->path, IniPath);
        }

        HeapFree(GetProcessHeap(), 0, IniPath);

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

        ULONG status = IsCallerAuthorized(hToken, req->password, req->section);
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
    //WCHAR domain[256];
    //ULONG domain_len = sizeof(domain) / sizeof(WCHAR) - 4;
    //SID_NAME_USE use;

    m_username[0] = L'\0';

    //ok = LookupAccountSid(NULL, info.user.User.Sid,
    //        m_username, &username_len, domain, &domain_len, &use);
    ok = DriverAssist::LookupSidCached(info.user.User.Sid, m_username, &username_len);
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


ULONG SbieIniServer::IsCallerAuthorized(HANDLE hToken, const WCHAR *Password, const WCHAR *Section)
{
    WCHAR buf[42], buf2[42];

    //
    // check for Administrator-only access
    //

    if (SbieApi_QueryConfBool(Section, L"EditAdminOnly", FALSE)) {

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


#ifdef NEW_INI_MODE
//---------------------------------------------------------------------------
// NotifyConfigReloaded
//---------------------------------------------------------------------------


void SbieIniServer::NotifyConfigReloaded()
{
    if (!m_instance)
        return;

    EnterCriticalSection(&m_instance->m_critsec);
    
    // purge config cache
    if (m_instance->m_pConfigIni != NULL) {
        delete m_instance->m_pConfigIni;
        m_instance->m_pConfigIni = NULL;
    }

    LeaveCriticalSection(&m_instance->m_critsec);
}


//---------------------------------------------------------------------------
// CacheConfig
//---------------------------------------------------------------------------


ULONG SbieIniServer::CacheConfig()
{
    if (m_pConfigIni != NULL) {
        delete m_pConfigIni;
        m_pConfigIni = NULL;
    }

    WCHAR *IniPath;
    BOOLEAN IsUTF8 = FALSE;
    if (! GetIniPath(&IniPath, NULL, &IsUTF8))
        return STATUS_INSUFFICIENT_RESOURCES;

    WCHAR* iniData = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    ULONG attrs = GetFileAttributes(IniPath);
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        ULONG LastError = GetLastError();
        attrs = 0;
        if (LastError != ERROR_FILE_NOT_FOUND) {
            SbieApi_LogEx(m_session_id, 2322, L"[21 / %d]", LastError);
            goto finish;
        }
    }

    const ULONG CONFLICTING_ATTRS = (   FILE_ATTRIBUTE_DEVICE |
                                        FILE_ATTRIBUTE_DIRECTORY |
                                        FILE_ATTRIBUTE_REPARSE_POINT |
                                        FILE_ATTRIBUTE_ENCRYPTED |
                                        FILE_ATTRIBUTE_OFFLINE);

    if (attrs & CONFLICTING_ATTRS) {
        SbieApi_LogEx(m_session_id, 2322, L"[22 / %d]", GetLastError());
        attrs = 0;
        goto finish;
    }

    //
    // open Sandboxie.ini
    //

    UnlockConf();

    hFile = CreateFile(
        IniPath, FILE_GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        if (err == ERROR_FILE_NOT_FOUND) 
            status = STATUS_SUCCESS; // the file does not exist that's ok
        else
            SbieApi_LogEx(m_session_id, 2322, L"[23 / %d]", err);
        goto finish;
    }

    // 
    // read Sandboxie.ini
    //

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize) || fileSize.QuadPart >= CONF_LINE_LEN * 2 * CONF_MAX_LINES) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        SbieApi_LogEx(m_session_id, 2322, L"[24 / %d]", status);
        goto finish;
    }

    if (fileSize.QuadPart == 0) {
        status = STATUS_SUCCESS;
        goto finish; // nothing to do
    }

    iniData = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, (SIZE_T)(fileSize.QuadPart + 128));
    if (!iniData) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto finish;
    }

    DWORD bytesRead;
    if (!ReadFile(hFile, iniData, (DWORD)fileSize.QuadPart, &bytesRead, NULL) || bytesRead != (DWORD)fileSize.QuadPart) {
        status = STATUS_NOT_READ_FROM_COPY;
        SbieApi_LogEx(m_session_id, 2322, L"[25 / %d]", status);
        goto finish;
    }

    WCHAR* iniDataPtr = iniData;

    // Decode the BOM if present and andance the iniDataPtr accordingly
    ULONG encoding = Read_BOM((UCHAR**)&iniDataPtr, &bytesRead);

    if (encoding == 1) { // UTF-8 Signature
        // decode utf8
        int ByteSize = MultiByteToWideChar(CP_UTF8, 0, (char*)iniDataPtr, bytesRead, NULL, 0) + 1;
        WCHAR* tmpData = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, ByteSize * sizeof(wchar_t));
        if (!tmpData) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto finish;
        }
        bytesRead = MultiByteToWideChar(CP_UTF8, 0, (char*)iniDataPtr, bytesRead, tmpData, ByteSize);
        // swap buffers
        HeapFree(GetProcessHeap(), 0, iniData);
        iniDataPtr = iniData = tmpData;
    }
    else {
        if (encoding == 2) { //Unicode (UTF-16 BE) BOM
            // swap all bytes
            UCHAR* tmpData = (UCHAR*)iniDataPtr;
            for (DWORD i = 0; i < bytesRead - 1; i += 2) {
                UCHAR tmp = tmpData[i + 1];
                tmpData[i + 1] = tmpData[i];
                tmpData[i] = tmp;
            }
        }
        //else if (encoding == 0) //Unicode (UTF-16 LE) BOM
        bytesRead /= sizeof(wchar_t);
    }

    iniDataPtr[bytesRead] = L'\0';

    m_pConfigIni = new SConfigIni;
    m_pConfigIni->Encoding = encoding;

    m_pConfigIni->Sections.push_back(SIniSection{});
    SIniSection* pSection = &m_pConfigIni->Sections.back();
    while(*iniDataPtr != L'\0' && pSection != NULL)
    {
        Ini_Read_ConfigSection(iniDataPtr, pSection->Entries);
        if (*iniDataPtr == L'\0')
            break;

        pSection = Ini_Read_SectionHeader(iniDataPtr, m_pConfigIni);
    }

    status = STATUS_SUCCESS;

finish:
    if(iniData != NULL)
        HeapFree(GetProcessHeap(), 0, iniData);

    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);

    LockConf(IniPath);

    HeapFree(GetProcessHeap(), 0, IniPath);

    if (NT_SUCCESS(status) && m_pConfigIni == NULL) {

        m_pConfigIni = new SConfigIni;
        m_pConfigIni->Encoding = 0;

        // set a ini header with a descriptive comment
        m_pConfigIni->Sections.push_back(SIniSection{ L"" });
        m_pConfigIni->Sections.back().Entries.push_back(SIniEntry{ L"", L"#" });
        m_pConfigIni->Sections.back().Entries.push_back(SIniEntry{ L"", L"# Sandboxie configuration file" });
        m_pConfigIni->Sections.back().Entries.push_back(SIniEntry{ L"", L"#" });

        m_pConfigIni->Sections.push_back(SIniSection{ L"GlobalSettings" });
    }

    return status;
}


//---------------------------------------------------------------------------
// GetIniSection
//---------------------------------------------------------------------------


SIniSection* SbieIniServer::GetIniSection(const WCHAR* section, bool bCanAdd)
{
    SIniSection* pSection = NULL;
    for (auto I = m_pConfigIni->Sections.begin(); I != m_pConfigIni->Sections.end(); ++I)
    {
        if (_wcsicmp(I->Name.c_str(), section) == 0) {
            pSection = &(*I);
            break;
        }
    }

    if (!pSection && bCanAdd) {
        m_pConfigIni->Sections.push_back(SIniSection{section});
        pSection = &m_pConfigIni->Sections.back();
    }
    return pSection;
}


//---------------------------------------------------------------------------
// GetSetting
//---------------------------------------------------------------------------


MSG_HEADER *SbieIniServer::GetSetting(MSG_HEADER *msg)
{
    SBIE_INI_SETTING_REQ *req = (SBIE_INI_SETTING_REQ *)msg;

    RevertToSelf();

    //
    // Get the relevant ini section object
    //

    SIniSection* pSection = GetIniSection(req->section, false);
    if (!pSection)
        return SHORT_REPLY(STATUS_OBJECT_NAME_NOT_FOUND);


    //
    // get the values
    //

    std::wstring iniData;

    for (auto I = pSection->Entries.begin(); I != pSection->Entries.end(); ++I)
    {
        if (*req->setting == L'\0') { // get section
            if(I->Name.size() > 0)
                iniData += I->Name + L"=";
            iniData += I->Value + L"\r\n";
        }
        else if (_wcsicmp(I->Name.c_str(), req->setting) == 0) {
            if(!iniData.empty()) // string list
                //iniData.push_back(L'\0');
                iniData.push_back(L'\n');
            iniData += I->Value;
        }
    }

    //
    // prepare the reply
    //

    ULONG rpl_len = sizeof(SBIE_INI_SETTING_RPL) + (iniData.size() + 1) * sizeof(WCHAR);
    SBIE_INI_SETTING_RPL *rpl = (SBIE_INI_SETTING_RPL *)LONG_REPLY(rpl_len);
    if (!rpl) 
        return SHORT_REPLY(STATUS_INSUFFICIENT_RESOURCES);

    rpl->value_len = iniData.size() + 1;
    wcscpy(rpl->value, iniData.c_str());

    return (MSG_HEADER*)rpl;
}


//---------------------------------------------------------------------------
// SetSetting
//---------------------------------------------------------------------------


ULONG SbieIniServer::SetSetting(MSG_HEADER* msg)
{
    SBIE_INI_SETTING_REQ *req = (SBIE_INI_SETTING_REQ *)msg;
    BOOLEAN have_value = (req->value_len != 0);

    if (wcslen(req->setting) == 0 && !have_value)
        return STATUS_SUCCESS; // dummy request to trigger a refresh

    //
    // check if this is a delete section request and if so execute it
    //

    if (wcscmp(req->setting, L"*") == 0 && !have_value) 
    {
        for (auto I = m_pConfigIni->Sections.begin(); I != m_pConfigIni->Sections.end(); ++I)
        {
            if (_wcsicmp(I->Name.c_str(), req->section) == 0) {
                m_pConfigIni->Sections.erase(I);
                break;
            }
        }
        return STATUS_SUCCESS;
    }

    //
    // Get the relevant ini section object
    //

    SIniSection* pSection = GetIniSection(req->section, true);

    //
    // Check if this is a replace section request and if so execute it
    //

    if (wcslen(req->setting) == 0 && have_value) 
    {
        std::list<SIniEntry> entries;

        WCHAR* iniDataPtr = req->value;
        Ini_Read_ConfigSection(iniDataPtr, entries);
        if (*iniDataPtr != L'\0') // there must be no sections inside another section
            return STATUS_INVALID_PARAMETER;

        pSection->Entries = entries;
        return STATUS_SUCCESS;
    }

    //
    // remove old values and set the new once
    //

    std::list<SIniEntry>::iterator pos = pSection->Entries.end();
    for (auto I = pSection->Entries.begin(); I != pSection->Entries.end();)
    {
        if (_wcsicmp(I->Name.c_str(), req->setting) == 0) {
            I = pSection->Entries.erase(I);
            pos = I;
        }
        else
            ++I;
    }

    //
    // set the value(s) if present
    //

    if (have_value) 
    {
        /*for (WCHAR* value = req->value; req->value_len > 0 && *value != L'\0';) 
        {
            pSection->Entries.insert(pos, SIniEntry{ req->setting, value });

            ULONG len = wcslen(value);
            req->value_len -= len;
            if (req->value_len > 0) {
                req->value_len -= 1;
                value += len + 1;
            }
        }*/

        //
        // Note: SbieCtrl passes a \n separated list to replace all values in a string list
        //

        for (WCHAR* value = req->value; *value != L'\0'; ) 
        {
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

            pSection->Entries.insert(pos, SIniEntry{ req->setting, std::wstring(value, cpylen) });

            value += skiplen;
        }
    }

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// AddSetting
//---------------------------------------------------------------------------


ULONG SbieIniServer::AddSetting(MSG_HEADER* msg, bool insert)
{
    //
    // if a value was not specified, do nothing
    //

    SBIE_INI_SETTING_REQ *req = (SBIE_INI_SETTING_REQ *)msg;
    if (! req->value_len)
        return STATUS_SUCCESS;

    bool seen_value = false;

    //
    // Get the relevant ini section object
    //

    SIniSection* pSection = GetIniSection(req->section, true);

    //
    // Find the right place to add the value
    //

    std::list<SIniEntry>::iterator pos = pSection->Entries.end();
    for (auto I = pSection->Entries.begin(); I != pSection->Entries.end();++I)
    {
        if (_wcsicmp(I->Name.c_str(), req->setting) == 0) {
            // !insert -> append -> find last entry
            if (!insert || pos == pSection->Entries.end()) {
                pos = I;
                if (!insert) pos++;
            }
            //if (_wcsicmp(I->Value.c_str(), req->value) == 0) {
            //    // this value is already present, so let's abort right here
            //    return STATUS_SUCCESS;
            //}
        }
    }

    //
    // add the value to the string list
    //

    pSection->Entries.insert(pos, SIniEntry{ req->setting, req->value });

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// DelSetting
//---------------------------------------------------------------------------


ULONG SbieIniServer::DelSetting(MSG_HEADER* msg)
{
    //
    // if a value was not specified, go directly to SetSetting,
    // which will discard all instances of the setting
    //

    SBIE_INI_SETTING_REQ *req = (SBIE_INI_SETTING_REQ *)msg;
    if (! req->value_len)
        return SetSetting(msg);

    //
    // Get the relevant ini section object
    //

    SIniSection* pSection = GetIniSection(req->section, false);
    if (!pSection)
        return STATUS_SUCCESS;

    //
    // discard setting with the matching the value
    //

    for (auto I = pSection->Entries.begin(); I != pSection->Entries.end();)
    {
        if (_wcsicmp(I->Name.c_str(), req->setting) == 0 && _wcsicmp(I->Value.c_str(), req->value) == 0) {
            I = pSection->Entries.erase(I);
            // Note: we could break here, but let's finish in case there is a duplicate
        }
        else
            ++I;
    }

    return STATUS_SUCCESS;
}
#else
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
#endif

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

#ifndef NEW_INI_MODE
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
#endif


//---------------------------------------------------------------------------
// RefreshConf
//---------------------------------------------------------------------------


ULONG SbieIniServer::RefreshConf()
{
#ifdef NEW_INI_MODE
    if (!m_pConfigIni)
        return STATUS_UNSUCCESSFUL;

    std::wstring iniData;
#else
    if (!m_text)
        return STATUS_UNSUCCESSFUL;
#endif

    WCHAR *IniPath;
    BOOLEAN IsUTF8 = FALSE;
    if (! GetIniPath(&IniPath, NULL, &IsUTF8))
        return STATUS_INSUFFICIENT_RESOURCES;

    WCHAR *TmpPath = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, 2048);
    if (! TmpPath) {
        HeapFree(GetProcessHeap(), 0, IniPath);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    wcscpy(TmpPath, IniPath);
    WCHAR *ptr = wcsrchr(TmpPath, L'.');
    wsprintf(ptr, L"-%d.tmp", GetTickCount());


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

    int retryCnt = 0;
retry:
    hFile = CreateFile(
        IniPath, FILE_GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_SHARING_VIOLATION && retryCnt++ < 10) {
            Sleep(100);
            goto retry;
        }
        SbieApi_LogEx(m_session_id, 2322, L"[15 / %d]", GetLastError());
        goto finish;
    }

#ifdef NEW_INI_MODE

    //
    // rebuild the ini from the cache with new values, if present, 
    // and keeping comments and most of the formatting
    //

    for (auto I = m_pConfigIni->Sections.begin(); I != m_pConfigIni->Sections.end(); ++I)
    {
        if (I->Name.size() > 0)
            iniData += L"[" + I->Name + L"]\r\n";

        for (auto J = I->Entries.begin(); J != I->Entries.end(); ++J)
        {
            if(J->Name.size() > 0)
                iniData += J->Name + L"=";
            iniData += J->Value + L"\r\n";
        }
        iniData += L"\r\n";
    }

    const WCHAR* m_text_base = iniData.c_str();
    IsUTF8 = m_pConfigIni->Encoding == 1;
#endif

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


bool SbieIniServer::GetIniPath(WCHAR **IniPath,
                               BOOLEAN *IsHomePath, BOOLEAN* IsUTF8)
{
    static const WCHAR *_ini = SANDBOXIE_INI;
    WCHAR *path = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, 2048);
    if (! path)
        return false;

    // the Sandbox driver tells us if the ini file comes from the
    // program home directory or from the Windows directory, and
    // we use that information to select the output path

    if (IsUTF8 != NULL) {
        LONG rc = SbieApi_QueryConfAsIs(NULL, L"IniEncoding", 0, path, 8);
        if (rc == 0 && *path == L'8')
            *IsUTF8 = TRUE;
    }

    LONG rc = SbieApi_QueryConfAsIs(NULL, L"IniLocation", 0, path, 260 * sizeof(WCHAR));
    if (rc == 0 && *path == L'\\') {

        if (wcsnicmp(path, L"\\??\\", 4) == 0)
            wmemmove(path, path + 4, wcslen(path)+1 - 4);
    }
    else if (rc == 0 && *path == L'H') {

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

        return false;
    }

    *IniPath = path;

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

    bool FreePaths = false;
    if (! IniPath) {
        if (! GetIniPath(&IniPath))
            return;
        FreePaths = TRUE;
    }

    m_hLockFile = CreateFile(
        IniPath, FILE_GENERIC_READ, FILE_SHARE_READ,
        NULL, OPEN_EXISTING, 0, NULL);

    if (FreePaths) {
        HeapFree(GetProcessHeap(), 0, IniPath);
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


MSG_HEADER *SbieIniServer::RunSbieCtrl(MSG_HEADER *msg, HANDLE idProcess, bool isSandboxed)
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    HANDLE hToken = NULL;
    BOOL ok = TRUE;
    WCHAR ctrlCmd[128] = { 0 };

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
        SbieApi_SessionLeader(m_session_id, &SbieCtrlProcessId);
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
        WCHAR buf[10], ch = 0;
        bool ok2 = SetUserSettingsSectionName(hToken);
        if (ok2) {
            SbieApi_QueryConfAsIs(
                m_sectionname, _Setting, 0, buf, sizeof(buf) - 2);
            ch = towlower(buf[0]);
        }
        if (! ch) {
            wcscpy(m_sectionname + 13, L"Default");   // UserSettings_Default
            SbieApi_QueryConfAsIs(
                m_sectionname, _Setting, 0, buf, 8 * sizeof(WCHAR));
            ch = towlower(buf[0]);
        }

        if (ch == L'n') {
            status = STATUS_LOGON_NOT_GRANTED;
            ok = FALSE;
        }
    }

    //
    // get the agent binary name
    //

    if (isSandboxed) {

        const WCHAR* _Setting2 = SBIECTRL_ L"AutoStartAgent";
        bool ok2 = SetUserSettingsSectionName(hToken);
        if (ok2) {
            SbieApi_QueryConfAsIs(
                m_sectionname, _Setting2, 0, ctrlCmd, sizeof(ctrlCmd) - 2);
        }
        else {
            wcscpy(m_sectionname + 13, L"Default");   // UserSettings_Default
            SbieApi_QueryConfAsIs(
                m_sectionname, _Setting2, 0, ctrlCmd, sizeof(ctrlCmd) - 2);
        }

    } else if (msg->length > sizeof(MSG_HEADER)) {

        ULONG len = (ULONG)(msg->length - sizeof(MSG_HEADER));
        memcpy(ctrlCmd, (UCHAR*)msg + sizeof(MSG_HEADER), len);
        ctrlCmd[len / sizeof(WCHAR)] = L'\0';
    }

    //
    // run Sandboxie Control in caller session
    //

    if (ok) {

        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        WCHAR *args = NULL;

        //
        // split the agent executable name from the arguments
        //

        WCHAR* end = (WCHAR*)SbieDll_FindArgumentEnd(ctrlCmd);
        if (*end) {
            *end++ = 0;
            args = end;
        }

        //
        // run the agent executable from the sbie home directory,
        // when none was specified fallback to SBIECTRL_EXE
        //

        if (SbieDll_RunFromHome(*ctrlCmd ? ctrlCmd : SBIECTRL_EXE, args, &si, NULL)) {

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


//---------------------------------------------------------------------------
// SetDatFile
//---------------------------------------------------------------------------


MSG_HEADER *SbieIniServer::SetDatFile(MSG_HEADER *msg, HANDLE idProcess)
{
    HANDLE SessionLeaderPid;
    SbieApi_SessionLeader(m_session_id, &SessionLeaderPid);
    if (SessionLeaderPid != idProcess)
        return SHORT_REPLY(STATUS_ACCESS_DENIED);

    SBIE_INI_SETTING_REQ *req = (SBIE_INI_SETTING_REQ *)msg;
    if (req->h.length < sizeof(SBIE_INI_SETTING_REQ))
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);

    wchar_t* ext = wcsrchr(req->setting, L'.');
    if (!ext || (_wcsicmp(ext, L".dat") != 0) || wcsstr(req->setting, L"..") != NULL)
        return SHORT_REPLY(STATUS_INVALID_FILE_FOR_SECTION);

    WCHAR path[768];
    NTSTATUS status = SbieApi_GetHomePath(path, 768, NULL, 0);
    if (!NT_SUCCESS(status))
        return SHORT_REPLY(status);
    wcscat(path, L"\\");
    wcscat(path, req->setting);

    UNICODE_STRING objname;
    RtlInitUnicodeString(&objname, path);

    OBJECT_ATTRIBUTES objattrs;
    InitializeObjectAttributes(&objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    if (req->value_len == 0) {

        NtDeleteFile(&objattrs);

        return SHORT_REPLY(STATUS_SUCCESS);
    }

    HANDLE handle = INVALID_HANDLE_VALUE;
    IO_STATUS_BLOCK IoStatusBlock;
    status = NtCreateFile(&handle, FILE_GENERIC_WRITE, &objattrs, &IoStatusBlock,NULL, 0, FILE_SHARE_VALID_FLAGS, FILE_OVERWRITE_IF, FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE, NULL, 0);
    if (NT_SUCCESS(status)) {

        status = NtWriteFile(handle, NULL, NULL, NULL, &IoStatusBlock, req->value, req->value_len, NULL, NULL);

        NtClose(handle);
    }

    return SHORT_REPLY(status);
}


//---------------------------------------------------------------------------
// GetDatFile
//---------------------------------------------------------------------------

//
//MSG_HEADER *SbieIniServer::GetDatFile(MSG_HEADER *msg, HANDLE idProcess)
//{
//    // ToDo
//}


//---------------------------------------------------------------------------
// RC4Crypt
//---------------------------------------------------------------------------


MSG_HEADER *SbieIniServer::RC4Crypt(MSG_HEADER *msg, HANDLE idProcess, bool isSandboxed)
{
    //
    // The purpose of this function is to provide a simple machine bound obfuscation
    // for example to store passwords which are required in plain text.
    // To this end we use a Random 64 bit key which is generated once and stored in the registry
    // as well as the rc4 algorithm for the encryption, applying the same transformation twice 
    // yealds the original plaintext, hence only one function is sufficient.
    // 
    // Please note that neither the mechanism nor the use of the rc4 algorithm can be considered 
    // cryptographically secure by any means.
    // This mechanism is only good for simple obfuscation of non critical data.
    //

    SBIE_INI_RC4_CRYPT_REQ *req = (SBIE_INI_RC4_CRYPT_REQ *)msg;
    if (req->h.length < sizeof(SBIE_INI_RC4_CRYPT_REQ))
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);

    ULONG rpl_len = sizeof(SBIE_INI_RC4_CRYPT_RPL) + req->value_len;
    SBIE_INI_RC4_CRYPT_RPL *rpl = (SBIE_INI_RC4_CRYPT_RPL *)LONG_REPLY(rpl_len);
    if (!rpl) 
        return SHORT_REPLY(STATUS_INSUFFICIENT_RESOURCES);

    rpl->value_len = req->value_len;
    memcpy(rpl->value, req->value, req->value_len);

    ULONG64 RandID = 0;
    SbieApi_Call(API_GET_SECURE_PARAM, 3, L"RandID", (ULONG_PTR)&RandID, sizeof(RandID));
    if (RandID == 0) {
		srand(GetTickCount());
        RandID = ULONG64(rand() & 0xFFFF) | (ULONG64(rand() & 0xFFFF) << 16) | (ULONG64(rand() & 0xFFFF) << 32) | (ULONG64(rand() & 0xFFFF) << 48);
        SbieApi_Call(API_SET_SECURE_PARAM, 3, L"RandID", (ULONG_PTR)&RandID, sizeof(RandID));
    }

    rc4_crypt((BYTE*)&RandID, sizeof(RandID), 0x1000, rpl->value, rpl->value_len);

    return (MSG_HEADER*)rpl;
}