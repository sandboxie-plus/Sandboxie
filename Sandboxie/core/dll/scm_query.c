/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
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
// Service Control Manager
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct QUERY_SERVICE_CONFIG_64 {

    ULONG dwServiceType;
    ULONG dwStartType;
    ULONG dwErrorControl;
    ULONG pad1;
    ULONG64 lpBinaryPathName;
    ULONG64 lpLoadOrderGroup;
    ULONG dwTagId;
    ULONG pad2;
    ULONG64 lpDependencies;
    ULONG64 lpServiceStartName;
    ULONG64 lpDisplayName;

} QUERY_SERVICE_CONFIG_64;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static WCHAR *Scm_GetServiceConfigString(SERVICE_QUERY_RPL *rpl, UCHAR type);

static void *Scm_QueryBoxedServiceByName(
    const WCHAR *ServiceNm,
    ULONG with_service_status, ULONG with_service_config);

void *Scm_QueryServiceByName(
    const WCHAR *ServiceNm,
    ULONG with_service_status, ULONG with_service_config);

static void *Scm_QueryServiceByHandle(
    HANDLE hService, ULONG with_service_status, ULONG with_service_config);

static BOOL Scm_QueryServiceStatusEx(
    SC_HANDLE hService, SC_STATUS_TYPE InfoLevel,
    LPBYTE lpBuffer, DWORD cbBufSize, LPDWORD pcbBytesNeeded);

static BOOL Scm_QueryServiceStatus(
    SC_HANDLE hService, SERVICE_STATUS *lpServiceStatus);

static BOOL Scm_QueryServiceConfigW(
    SC_HANDLE hService, void *lpServiceConfig,
    DWORD cbBufSize, LPDWORD pcbBytesNeeded);

static BOOL Scm_QueryServiceConfigA(
    SC_HANDLE hService, void *lpServiceConfig,
    DWORD cbBufSize, LPDWORD pcbBytesNeeded);

static BOOL Scm_QueryServiceConfig2W(
    SC_HANDLE hService, DWORD InfoLevel, LPBYTE lpBuffer,
    DWORD cbBufSize, LPDWORD pcbBytesNeeded);

static BOOL Scm_QueryServiceConfig2A(
    SC_HANDLE hService, DWORD InfoLevel, LPBYTE lpBuffer,
    DWORD cbBufSize, LPDWORD pcbBytesNeeded);

static BOOL Scm_EnumServicesStatusX(
    SC_HANDLE hSCManager, DWORD dwServiceType, DWORD dwServiceState,
    LPENUM_SERVICE_STATUS lpServices,
    DWORD cbBufSize, LPDWORD pcbBytesNeeded, LPDWORD lpServicesReturned,
    LPDWORD lpResumeHandle, BOOL IsUnicode, BOOL IsExtended);

static BOOL Scm_EnumServicesStatusW(
    SC_HANDLE hSCManager, DWORD dwServiceType, DWORD dwServiceState,
    void *lpServices, DWORD cbBufSize, LPDWORD pcbBytesNeeded,
    LPDWORD lpServicesReturned, LPDWORD lpResumeHandle);

static BOOL Scm_EnumServicesStatusA(
    SC_HANDLE hSCManager, DWORD dwServiceType, DWORD dwServiceState,
    void *lpServices, DWORD cbBufSize, LPDWORD pcbBytesNeeded,
    LPDWORD lpServicesReturned, LPDWORD lpResumeHandle);

static BOOL Scm_EnumServicesStatusExW(
    SC_HANDLE hSCManager, SC_ENUM_TYPE InfoLevel,
    DWORD dwServiceType, DWORD dwServiceState, void *lpServices,
    DWORD cbBufSize, LPDWORD pcbBytesNeeded,
    LPDWORD lpServicesReturned, LPDWORD lpResumeHandle, void *GroupName);

static BOOL Scm_EnumServicesStatusExA(
    SC_HANDLE hSCManager, SC_ENUM_TYPE InfoLevel,
    DWORD dwServiceType, DWORD dwServiceState, void *lpServices,
    DWORD cbBufSize, LPDWORD pcbBytesNeeded,
    LPDWORD lpServicesReturned, LPDWORD lpResumeHandle, void *GroupName);

static BOOL Scm_QueryServiceLockStatusW(
    SC_HANDLE hService, void *lpServiceStatus,
    DWORD cbBufSize, LPDWORD pcbBytesNeeded);

static BOOL Scm_QueryServiceLockStatusA(
    SC_HANDLE hService, void *lpServiceStatus,
    DWORD cbBufSize, LPDWORD pcbBytesNeeded);

static BOOL Scm_GetServiceDisplayNameW(
    SC_HANDLE hSCManager, WCHAR *lpServiceName,
    WCHAR *lpDisplayName, LPDWORD lpcchBuffer);

static BOOL Scm_GetServiceDisplayNameA(
    SC_HANDLE hSCManager, UCHAR *lpServiceName,
    UCHAR *lpDisplayName, LPDWORD lpcchBuffer);

static BOOL Scm_GetServiceKeyNameW(
    SC_HANDLE hSCManager, WCHAR *lpDisplayName,
    WCHAR *lpServiceName, LPDWORD lpcchBuffer);

static BOOL Scm_GetServiceKeyNameA(
    SC_HANDLE hSCManager, UCHAR *lpDisplayName,
    UCHAR *lpServiceName, LPDWORD lpcchBuffer);

static BOOL Scm_EnumDependentServicesW(
    SC_HANDLE hService, DWORD dwServiceState,
    LPENUM_SERVICE_STATUS lpServices, DWORD cbBufSize,
    LPDWORD pcbBytesNeeded, LPDWORD lpServicesReturned);

static BOOL Scm_EnumDependentServicesA(
    SC_HANDLE hService, DWORD dwServiceState,
    LPENUM_SERVICE_STATUS lpServices, DWORD cbBufSize,
    LPDWORD pcbBytesNeeded, LPDWORD lpServicesReturned);

static BOOL Scm_QueryServiceObjectSecurity(
    SC_HANDLE hService,
    SECURITY_INFORMATION dwSecurityInformation,
    PSECURITY_DESCRIPTOR lpSecurityDescriptor,
    DWORD cbBufSize, LPDWORD pcbBytesNeeded);

static BOOL Scm_SetServiceObjectSecurity(
    SC_HANDLE hService,
    SECURITY_INFORMATION dwSecurityInformation,
    PSECURITY_DESCRIPTOR lpSecurityDescriptor);


//---------------------------------------------------------------------------
// Scm_GetServiceConfigString
//---------------------------------------------------------------------------


_FX WCHAR *Scm_GetServiceConfigString(SERVICE_QUERY_RPL *rpl, UCHAR type)
{
    ULONG_PTR ptr;

#ifndef _WIN64
    if (Dll_IsWow64) {

        //
        // 32-bit caller but SERVICE_QUERY_RPL returned from 64-bit SbieSvc:
        // rpl contains a QUERY_SERVICE_CONFIG_64 structure
        //

        QUERY_SERVICE_CONFIG_64 *cfg =
            (QUERY_SERVICE_CONFIG_64 *)&rpl->service_config;

        if (type == 'D')
            ptr = (ULONG_PTR)cfg->lpDisplayName;
        else if (type == 'P')
            ptr = (ULONG_PTR)cfg->lpBinaryPathName;

    } else 
#endif
    {

        //
        // if not Wow64 then caller bitness matches bitness of SbieSvc
        // (both 32-bit or both 64-bit)
        //

        QUERY_SERVICE_CONFIG *cfg =
            (QUERY_SERVICE_CONFIG *)&rpl->service_config;

        if (type == 'D')
            ptr = (ULONG_PTR)cfg->lpDisplayName;
        else if (type == 'P')
            ptr = (ULONG_PTR)cfg->lpBinaryPathName;
    }

    ptr += (ULONG_PTR)&rpl->service_config;
    return (WCHAR *)ptr;
}


//---------------------------------------------------------------------------
// Scm_QueryBoxedServiceByName
//---------------------------------------------------------------------------


_FX void *Scm_QueryBoxedServiceByName(
    const WCHAR *ServiceNm,
    ULONG with_service_status, ULONG with_service_config)
{
    NTSTATUS status;
    ULONG len, service_config_len;
    SERVICE_QUERY_RPL *rpl;
    HANDLE hkey;
    UNICODE_STRING uni;
    union {
        KEY_VALUE_PARTIAL_INFORMATION info;
        WCHAR info_space[256];
    } u;
    WCHAR *DisplayName, *ImagePath, *ObjectName;
    ULONG ServiceState;

    //
    // open the key for the service
    //

    Scm_DiscardKeyCache(ServiceNm);

    hkey = Scm_OpenKeyForService(ServiceNm, FALSE);
    if (! hkey) {
        SetLastError(ERROR_INVALID_HANDLE);
        return NULL;
    }

    //
    // determine the length (and contents) for the service config
    // portion of the response buffer
    //

    DisplayName = ImagePath = ObjectName = NULL;

    if (with_service_config == 0) {

        service_config_len = 0;

    } else if (with_service_config == -1) {

        service_config_len = sizeof(QUERY_SERVICE_CONFIG)
            // plus an empty MULTI_SZ string for
            // lpLoadOrderGroup and lpDependencies
                           + sizeof(WCHAR) * 2;

#ifndef _WIN64
        if (Dll_IsWow64) {

            //
            // if this is Wow64, then SbieSvc is 64-bit, which returns
            // QUERY_SERVICE_CONFIG with 64-bit pointers rather than
            // 32-bit pointers, so we have to fake the same thing,
            // and set up extra space for the five pointers:
            // lpBinaryPathName lpLoadOrderGroup lpDependencies
            // lpServiceStartName lpDisplayName
            //

            service_config_len += 5 * sizeof(ULONG);
        }
#endif

        //
        // DisplayName
        //

        RtlInitUnicodeString(&uni, L"DisplayName");
        status = NtQueryValueKey(
            hkey, &uni, KeyValuePartialInformation,
            &u.info, sizeof(u), &len);

        if (NT_SUCCESS(status) &&
                    u.info.Type == REG_SZ && u.info.DataLength != 0)
            u.info_space[250] = L'\0';
        else
            *(WCHAR *)u.info.Data = L'\0';

        len = (wcslen((WCHAR *)u.info.Data) + 1) * sizeof(WCHAR);
        DisplayName = Dll_AllocTemp(len);
        memcpy(DisplayName, (WCHAR *)u.info.Data, len);
        service_config_len += len;

        //
        // ImagePath
        //

        RtlInitUnicodeString(&uni, L"ImagePath");
        status = NtQueryValueKey(
            hkey, &uni, KeyValuePartialInformation,
            &u.info, sizeof(u), &len);

        if (NT_SUCCESS(status) && u.info.DataLength != 0 &&
                (u.info.Type == REG_SZ || u.info.Type == REG_EXPAND_SZ))
            u.info_space[250] = L'\0';
        else
            *(WCHAR *)u.info.Data = L'\0';

        ImagePath = Dll_AllocTemp(9 * sizeof(WCHAR));
        len = ExpandEnvironmentStrings((WCHAR *)u.info.Data, ImagePath, 9);
        if (len > 9) {
            Dll_Free(ImagePath);
            ImagePath = Dll_AllocTemp((len + 8) * sizeof(WCHAR));
            len = ExpandEnvironmentStrings(
                                (WCHAR *)u.info.Data, ImagePath, len + 8);
        }

        service_config_len += len * sizeof(WCHAR);

        //
        // ObjectName
        //

        RtlInitUnicodeString(&uni, L"ObjectName");
        status = NtQueryValueKey(
            hkey, &uni, KeyValuePartialInformation,
            &u.info, sizeof(u), &len);

        if (NT_SUCCESS(status) &&
                    u.info.Type == REG_SZ && u.info.DataLength != 0)
            u.info_space[250] = L'\0';
        else
            *(WCHAR *)u.info.Data = L'\0';

        len = (wcslen((WCHAR *)u.info.Data) + 1) * sizeof(WCHAR);
        ObjectName = Dll_AllocTemp(len);
        memcpy(ObjectName, (WCHAR *)u.info.Data, len);
        service_config_len += len;

    } else if (with_service_config == SERVICE_CONFIG_DESCRIPTION) {

        service_config_len = sizeof(SERVICE_DESCRIPTION)
                           + (wcslen(ServiceNm) + 1) * sizeof(WCHAR);

#ifndef _WIN64
        if (Dll_IsWow64) {
            // set up extra space for lpDescription
            service_config_len += sizeof(ULONG);
        }
#endif

    } else if (with_service_config == SERVICE_CONFIG_FAILURE_ACTIONS) {

        service_config_len = sizeof(SERVICE_FAILURE_ACTIONS);

#ifndef _WIN64
        if (Dll_IsWow64)  {
            // set up extra space for lpRebootMsg, lpCommand and lpsaActions
            service_config_len += 3 * sizeof(ULONG);
        }
#endif

    } else if (with_service_config == 3 || with_service_config == 4) {

        service_config_len = sizeof(BOOL);      // Windows Vista config level

    } else if (with_service_config == 7) {

        service_config_len = sizeof(DWORD);     // Windows Vista config level

    } else {
        NtClose(hkey);
        SetLastError(ERROR_INVALID_LEVEL);
        return NULL;
    }

    //
    // allocate the response buffer
    //

    len = sizeof(SERVICE_QUERY_RPL) + service_config_len;
    rpl = Dll_Alloc(len);
    rpl->h.length = len;
    rpl->h.status = 0;
    rpl->service_config_len = service_config_len;

    memzero(&rpl->service_status, sizeof(SERVICE_STATUS_PROCESS));
    memzero(&rpl->service_config, service_config_len);

    rpl->service_status.dwCurrentState = SERVICE_STOPPED;

    //
    // collect service type.  if this isn't a WIN32_OWN_PROCESS service,
    // don't try to collect any of the dynamic info.
    //

    RtlInitUnicodeString(&uni, L"Type");
    status = NtQueryValueKey(
        hkey, &uni, KeyValuePartialInformation, &u.info, sizeof(u), &len);
    if (NT_SUCCESS(status) &&
                u.info.Type == REG_DWORD && u.info.DataLength == 4)
        rpl->service_status.dwServiceType = *(ULONG *)u.info.Data;

    if (! (rpl->service_status.dwServiceType & SERVICE_WIN32))
        goto after_service_status;

    if (! with_service_status)
        goto after_service_status;

    //
    // collect service current state
    //

    ServiceState = SERVICE_STOPPED;

    RtlInitUnicodeString(&uni, SBIE L"_CurrentState");
    status = NtQueryValueKey(
        hkey, &uni, KeyValuePartialInformation, &u.info, sizeof(u), &len);
    if (NT_SUCCESS(status) &&
                u.info.Type == REG_DWORD && u.info.DataLength == 4)
        ServiceState = *(ULONG *)u.info.Data;

    //
    // if the service is listed as start pending, but more than 8 seconds
    // have passed since it entered start pending, then set stopped status
    //

    if (ServiceState == SERVICE_START_PENDING) {

        RtlInitUnicodeString(&uni, SBIE L"_StartTicks");
        status = NtQueryValueKey(
            hkey, &uni, KeyValuePartialInformation,
            &u.info, sizeof(u), &len);
        if (NT_SUCCESS(status) &&
                u.info.Type == REG_DWORD && u.info.DataLength == 4 &&
                (GetTickCount() - *(ULONG *)u.info.Data <= 8 * 1000)) {

            RtlInitUnicodeString(&uni, SBIE L"_ProcessId");
            status = NtQueryValueKey(
                hkey, &uni, KeyValuePartialInformation,
                &u.info, sizeof(u), &len);

        } else
            status = STATUS_BAD_INITIAL_PC;

    //
    // if the service is listed as running, check if the service listed
    // its ProcessId, and if that process is running, and its image name
    // matches the image name for the service
    //

    } else if (ServiceState != SERVICE_STOPPED) {

        RtlInitUnicodeString(&uni, SBIE L"_ProcessId");
        status = NtQueryValueKey(
            hkey, &uni, KeyValuePartialInformation,
            &u.info, sizeof(u), &len);

    } else
        status = STATUS_BAD_INITIAL_PC;

    if (NT_SUCCESS(status) &&
                u.info.Type == REG_DWORD && u.info.DataLength == 4) {

        //
        // check if the process with that ProcessId exists, and if
        // the process name is the same as the service image name
        //

        ULONG pid;
        WCHAR boxname[BOXNAME_COUNT];
        WCHAR imagename[96];
        ULONG session_id;
        WCHAR *ptr, *ptr2;

        pid = *(ULONG *)u.info.Data;
        status = SbieApi_QueryProcess(
            (HANDLE)(ULONG_PTR)pid, boxname, imagename, NULL, &session_id);

        if (NT_SUCCESS(status) &&
                session_id == Dll_SessionId &&
                _wcsicmp(boxname, Dll_BoxName) == 0) {

            ptr2 = wcsrchr(imagename, L'.');
            if (ptr2)
                *ptr2 = L'\0';

            RtlInitUnicodeString(&uni, L"ImagePath");
            status = NtQueryValueKey(
                hkey, &uni, KeyValuePartialInformation,
                &u.info, sizeof(u), &len);

            if (NT_SUCCESS(status) && u.info.DataLength != 0 &&
                (u.info.Type == REG_SZ || u.info.Type == REG_EXPAND_SZ)) {

                u.info_space[250] = L'\0';

                ptr = wcsrchr((WCHAR *)u.info.Data, L'\\');
                if (! ptr)
                    ptr = wcsrchr((WCHAR *)u.info.Data, L'\"');
                if (ptr)
                    ++ptr;
                else
                    ptr = (WCHAR *)u.info.Data;

                ptr2 = wcsrchr(ptr, L'\"');
                if (ptr2)
                    *ptr2 = L'\0';
                else {
                    ptr2 = wcsrchr(ptr, L' ');
                    if (ptr2)
                        *ptr2 = L'\0';
                }
                ptr2 = wcsrchr(ptr, L'.');
                if (ptr2)
                    *ptr2 = L'\0';

                //
                // special cases for unsadboxed system services running
                // as sandboxed services, where the sandboxed process
                // name is SandboxieXxx.exe and does not match the
                // service image name as specified in the registry
                //

                if (_wcsicmp(ptr, imagename) == 0) {

                    rpl->service_status.dwProcessId = pid;

                } else {

                    const WCHAR **MySvc, *MySvcs[] = {
                        _bits,          SandboxieBITS,
                        Scm_CryptSvc,   SandboxieCrypto,
                        _wuauserv,      SandboxieWUAU,
                        // _eventsystem,   SandboxieEventSys,
                        NULL };

                    for (MySvc = MySvcs; *MySvc; MySvc += 2) {

                        if (_wcsicmp(ServiceNm, MySvc[0]) != 0)
                            continue;

                        len = (ULONG)(ULONG_PTR)
                                        (wcsrchr(MySvc[1], L'.') - MySvc[1]);
                        if (_wcsnicmp(imagename, MySvc[1], len) != 0)
                            continue;

                        rpl->service_status.dwProcessId = pid;
                        break;
                    }
                }
            }
        }

        if (rpl->service_status.dwProcessId)
            rpl->service_status.dwCurrentState = ServiceState;
    }

    //
    // collect service exit code values which might be useful
    // even if the service is not running, i.e. if it just stopped
    //

    RtlInitUnicodeString(&uni, SBIE L"_Win32ExitCode");
    status = NtQueryValueKey(
        hkey, &uni, KeyValuePartialInformation, &u.info, sizeof(u), &len);
    if (NT_SUCCESS(status) &&
                u.info.Type == REG_DWORD && u.info.DataLength == 4)
        rpl->service_status.dwWin32ExitCode = *(ULONG *)u.info.Data;

    RtlInitUnicodeString(&uni, SBIE L"_ServiceSpecificExitCode");
    status = NtQueryValueKey(
        hkey, &uni, KeyValuePartialInformation, &u.info, sizeof(u), &len);
    if (NT_SUCCESS(status) &&
                u.info.Type == REG_DWORD && u.info.DataLength == 4)
        rpl->service_status.dwServiceSpecificExitCode =
                                                    *(ULONG *)u.info.Data;

    //
    // collect information that is only useful if the service is running
    //

    if (rpl->service_status.dwCurrentState == SERVICE_STOPPED)
        goto after_service_status;

    RtlInitUnicodeString(&uni, SBIE L"_ControlsAccepted");
    status = NtQueryValueKey(
        hkey, &uni, KeyValuePartialInformation, &u.info, sizeof(u), &len);
    if (NT_SUCCESS(status) &&
                u.info.Type == REG_DWORD && u.info.DataLength == 4)
        rpl->service_status.dwControlsAccepted = *(ULONG *)u.info.Data;

    RtlInitUnicodeString(&uni, SBIE L"_CheckPoint");
    status = NtQueryValueKey(
        hkey, &uni, KeyValuePartialInformation, &u.info, sizeof(u), &len);
    if (NT_SUCCESS(status) &&
                u.info.Type == REG_DWORD && u.info.DataLength == 4)
        rpl->service_status.dwCheckPoint = *(ULONG *)u.info.Data;

    RtlInitUnicodeString(&uni, SBIE L"_WaitHint");
    status = NtQueryValueKey(
        hkey, &uni, KeyValuePartialInformation, &u.info, sizeof(u), &len);
    if (NT_SUCCESS(status) &&
                u.info.Type == REG_DWORD && u.info.DataLength == 4)
        rpl->service_status.dwWaitHint = *(ULONG *)u.info.Data;

after_service_status:

    //
    // service config
    //

    memzero(&rpl->service_config, service_config_len);

    if (with_service_config == -1) {

        ULONG_PTR *p_lpBinaryPathName   = 0;
        ULONG_PTR *p_lpLoadOrderGroup   = 0;
        ULONG_PTR *p_lpDependencies     = 0;
        ULONG_PTR *p_lpServiceStartName = 0;
        ULONG_PTR *p_lpDisplayName      = 0;

        ULONG_PTR base = (ULONG_PTR)&rpl->service_config;
        WCHAR *next;

        //
        // if this is Wow64, then SbieSvc is 64-bit, which returns
        // QUERY_SERVICE_CONFIG with 64-bit pointers rather than
        // 32-bit pointers, so we have to get the right offsets
        // for the pointer fields
        //

#ifndef _WIN64
        if (Dll_IsWow64) {

            p_lpBinaryPathName = (ULONG_PTR *)(base +
                FIELD_OFFSET(QUERY_SERVICE_CONFIG_64, lpBinaryPathName));
            p_lpLoadOrderGroup = (ULONG_PTR *)(base +
                FIELD_OFFSET(QUERY_SERVICE_CONFIG_64, lpLoadOrderGroup));
            p_lpDependencies = (ULONG_PTR *)(base +
                FIELD_OFFSET(QUERY_SERVICE_CONFIG_64, lpDependencies));
            p_lpServiceStartName = (ULONG_PTR *)(base +
                FIELD_OFFSET(QUERY_SERVICE_CONFIG_64, lpServiceStartName));
            p_lpDisplayName = (ULONG_PTR *)(base +
                FIELD_OFFSET(QUERY_SERVICE_CONFIG_64, lpDisplayName));

            next = (WCHAR *)(base + sizeof(QUERY_SERVICE_CONFIG_64));

        } else 
#endif
        {

            p_lpBinaryPathName = (ULONG_PTR *)(base +
                FIELD_OFFSET(QUERY_SERVICE_CONFIG, lpBinaryPathName));
            p_lpLoadOrderGroup = (ULONG_PTR *)(base +
                FIELD_OFFSET(QUERY_SERVICE_CONFIG, lpLoadOrderGroup));
            p_lpDependencies = (ULONG_PTR *)(base +
                FIELD_OFFSET(QUERY_SERVICE_CONFIG, lpDependencies));
            p_lpServiceStartName = (ULONG_PTR *)(base +
                FIELD_OFFSET(QUERY_SERVICE_CONFIG, lpServiceStartName));
            p_lpDisplayName = (ULONG_PTR *)(base +
                FIELD_OFFSET(QUERY_SERVICE_CONFIG, lpDisplayName));

            next = (WCHAR *)(base + sizeof(QUERY_SERVICE_CONFIG));
        }

        *p_lpLoadOrderGroup = *p_lpDependencies = (ULONG_PTR)next - base;

        *next = L'\0';
        ++next;
        *next = L'\0';
        ++next;

        if (DisplayName) {
            wcscpy(next, DisplayName);
            *p_lpDisplayName = (ULONG_PTR)next - base;
            next += wcslen(next) + 1;
            Dll_Free(DisplayName);
        }

        if (ImagePath) {
            wcscpy(next, ImagePath);
            *p_lpBinaryPathName = (ULONG_PTR)next - base;
            next += wcslen(next) + 1;
            Dll_Free(ImagePath);
        }

        if (ObjectName) {
            wcscpy(next, ObjectName);
            *p_lpServiceStartName = (ULONG_PTR)next - base;
            next += wcslen(next) + 1;
            Dll_Free(ObjectName);
        }

        //
        // prepare first three DWORD fields which are
        // not affected by 32/64 bit issues
        //

        rpl->service_config.dwServiceType =
            rpl->service_status.dwServiceType;

        RtlInitUnicodeString(&uni, L"Start");
        status = NtQueryValueKey(
            hkey, &uni, KeyValuePartialInformation,
            &u.info, sizeof(u), &len);
        if (NT_SUCCESS(status) &&
                    u.info.Type == REG_DWORD && u.info.DataLength == 4)
            rpl->service_config.dwStartType = *(ULONG *)u.info.Data;

        RtlInitUnicodeString(&uni, L"ErrorControl");
        status = NtQueryValueKey(
            hkey, &uni, KeyValuePartialInformation,
            &u.info, sizeof(u), &len);
        if (NT_SUCCESS(status) &&
                    u.info.Type == REG_DWORD && u.info.DataLength == 4)
            rpl->service_config.dwErrorControl = *(ULONG *)u.info.Data;

    } else if (with_service_config == SERVICE_CONFIG_DESCRIPTION) {

        //
        // create a simple SERVICE_DESCRIPTOR structure
        //

        ULONG_PTR base = (ULONG_PTR)&rpl->service_config;
        ULONG offset = sizeof(ULONG_PTR);
#ifndef _WIN64
        if (Dll_IsWow64)
            offset += sizeof(ULONG);
#endif
        *(ULONG_PTR *)base = offset;
        wcscpy((WCHAR *)(base + offset), ServiceNm);
    }

    NtClose(hkey);

    return rpl;
}


//---------------------------------------------------------------------------
// Scm_QueryServiceByName
//---------------------------------------------------------------------------


_FX void *Scm_QueryServiceByName(
    const WCHAR *ServiceNm,
    ULONG with_service_status, ULONG with_service_config)
{
    union {
        SERVICE_QUERY_REQ req;
        WCHAR req_space[384];
    } u;
    SERVICE_QUERY_RPL *rpl;
    ULONG error;

    //
    // if the name identifies a sandboxed service, let someone else
    // create the output structure
    //

    if (Scm_IsBoxedService(ServiceNm)) {
        return Scm_QueryBoxedServiceByName(
            ServiceNm, with_service_status, with_service_config);
    }

    //
    // this is a real service, so query through SbieSvc
    //

    u.req.name_len = wcslen(ServiceNm);
    wcscpy(u.req.name, ServiceNm);
    u.req.with_service_status = (USHORT)with_service_status;
    u.req.with_service_config = (USHORT)with_service_config;

    u.req.h.length = sizeof(SERVICE_QUERY_REQ)
                   + (u.req.name_len + 1) * sizeof(WCHAR);
    u.req.h.msgid = MSGID_SERVICE_QUERY;

    rpl = (SERVICE_QUERY_RPL *)SbieDll_CallServer(&u.req.h);

    if (rpl) {
        error = rpl->h.status;
        if (error) {
            Dll_Free(rpl);
            rpl = NULL;
        }
    } else
        error = RPC_S_SERVER_UNAVAILABLE;

    SetLastError(error);
    return rpl;
}


//---------------------------------------------------------------------------
// Scm_QueryServiceByHandle
//---------------------------------------------------------------------------


_FX void *Scm_QueryServiceByHandle(
    HANDLE hService, ULONG with_service_status, ULONG with_service_config)
{
    WCHAR *ServiceNm = Scm_GetHandleName(hService);
    if (! ServiceNm)
        return NULL;
    return Scm_QueryServiceByName(
        ServiceNm, with_service_status, with_service_config);
}


//---------------------------------------------------------------------------
// Scm_QueryServiceStatusEx
//---------------------------------------------------------------------------


_FX BOOL Scm_QueryServiceStatusEx(
    SC_HANDLE hService, SC_STATUS_TYPE InfoLevel,
    LPBYTE lpBuffer, DWORD cbBufSize, LPDWORD pcbBytesNeeded)
{
    SERVICE_QUERY_RPL *rpl;

    if (InfoLevel != SC_STATUS_PROCESS_INFO) {
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }
    if (cbBufSize < sizeof(SERVICE_STATUS_PROCESS)) {
        *pcbBytesNeeded = sizeof(SERVICE_STATUS_PROCESS);
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    rpl =
        (SERVICE_QUERY_RPL *)Scm_QueryServiceByHandle(hService, TRUE, 0);
    if (! rpl)
        return FALSE;

    memcpy(lpBuffer, &rpl->service_status, sizeof(rpl->service_status));
    Dll_Free(rpl);
    SetLastError(0);
    return TRUE;
}


//---------------------------------------------------------------------------
// Scm_QueryServiceStatus
//---------------------------------------------------------------------------


_FX BOOL Scm_QueryServiceStatus(
    SC_HANDLE hService, SERVICE_STATUS *lpServiceStatus)
{
    SERVICE_STATUS_PROCESS status;
    ULONG len = sizeof(SERVICE_STATUS_PROCESS);
    BOOL ok = Scm_QueryServiceStatusEx(
        hService, SC_STATUS_PROCESS_INFO, (BYTE *)&status, len, &len);
    if (ok)
        memcpy(lpServiceStatus, &status, sizeof(SERVICE_STATUS));
    return ok;
}


//---------------------------------------------------------------------------
// Scm_QueryServiceConfigW
//---------------------------------------------------------------------------


_FX BOOL Scm_QueryServiceConfigW(
    SC_HANDLE hService, void *lpServiceConfig,
    DWORD cbBufSize, LPDWORD pcbBytesNeeded)
{
    SERVICE_QUERY_RPL *rpl;

    rpl = (SERVICE_QUERY_RPL *)Scm_QueryServiceByHandle(hService, FALSE, -1);
    if (! rpl)
        return FALSE;

    if (pcbBytesNeeded)
        *pcbBytesNeeded = rpl->service_config_len;
    if (rpl->service_config_len > cbBufSize) {
        Dll_Free(rpl);
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

#ifndef _WIN64
    if (Dll_IsWow64) {

        //
        // if this is a 32-bit process in a 64-bit machine, then SbieSvc
        // returns a 64-bit QUERY_SERVICE_CONFIGW while our caller has
        // provided a 32-bit QUERY_SERVICE_CONFIGW
        //

        QUERY_SERVICE_CONFIG_64 *cfg =
            (QUERY_SERVICE_CONFIG_64 *)&rpl->service_config;
        QUERY_SERVICE_CONFIG *out = (QUERY_SERVICE_CONFIG *)lpServiceConfig;
        WCHAR *optr, *iptr;
        ULONG len;

        out->dwServiceType      =           cfg->dwServiceType;
        out->dwStartType        =           cfg->dwStartType;
        out->dwErrorControl     =           cfg->dwErrorControl;
        out->lpBinaryPathName   = NULL;
        out->lpLoadOrderGroup   = NULL;
        out->dwTagId            =           cfg->dwTagId;
        out->lpDependencies     = NULL;
        out->lpServiceStartName = NULL;
        out->lpDisplayName      = NULL;

        optr = (WCHAR *)(out + 1);

#define COPY_FIELD_64_TO_32(field)                                  \
        if (cfg->field) {                                           \
            out->field = optr;                                      \
            iptr = (WCHAR *)((UCHAR *)cfg + (ULONG)cfg->field);     \
            len = wcslen(iptr) + 1;                                 \
            wmemcpy(optr, iptr, len);                               \
            optr += len;                                            \
        }

        COPY_FIELD_64_TO_32(lpBinaryPathName);
        COPY_FIELD_64_TO_32(lpLoadOrderGroup);
        COPY_FIELD_64_TO_32(lpServiceStartName);
        COPY_FIELD_64_TO_32(lpDisplayName);

#undef COPY_FIELD_64_TO_32

        if (cfg->lpDependencies) {
            out->lpDependencies = optr;
            iptr = (WCHAR *)((UCHAR *)cfg + (ULONG)cfg->lpDependencies);
            if (! *iptr) {
                *optr = L'\0';
                ++optr;
            }
            while (*iptr) {
                len = wcslen(iptr) + 1;
                wmemcpy(optr, iptr, len);
                iptr += len;
                optr += len;
            }
            *optr = L'\0';
        }

    } else 
#endif
    {

        //
        // copy 32-bit QUERY_SERVICE_CONFIGW structure to caller
        // then convert offsets into pointers
        //

        QUERY_SERVICE_CONFIG *cfg =
            (QUERY_SERVICE_CONFIG *)&rpl->service_config;
        QUERY_SERVICE_CONFIGA *out =
            (QUERY_SERVICE_CONFIGA *)lpServiceConfig;
        memcpy(out, cfg, rpl->service_config_len);

        if (out->lpBinaryPathName)
            out->lpBinaryPathName      += (ULONG_PTR)out;
        if (out->lpLoadOrderGroup)
            out->lpLoadOrderGroup      += (ULONG_PTR)out;
        if (out->lpDependencies)
            out->lpDependencies        += (ULONG_PTR)out;
        if (out->lpServiceStartName)
            out->lpServiceStartName    += (ULONG_PTR)out;
        if (out->lpDisplayName)
            out->lpDisplayName         += (ULONG_PTR)out;
    }

    Dll_Free(rpl);

    SetLastError(0);
    return TRUE;
}


//---------------------------------------------------------------------------
// Scm_QueryServiceConfigA
//---------------------------------------------------------------------------


_FX BOOL Scm_QueryServiceConfigA(
    SC_HANDLE hService, void *lpServiceConfig,
    DWORD cbBufSize, LPDWORD pcbBytesNeeded)
{
    QUERY_SERVICE_CONFIGW *cfg;
    QUERY_SERVICE_CONFIGA *out;
    BOOL ok;
    ULONG last_error;
    UNICODE_STRING uni;
    ANSI_STRING ansi;

    cfg = (QUERY_SERVICE_CONFIGW *)Dll_Alloc(10240);

    ok = Scm_QueryServiceConfigW(hService, cfg, 10000, pcbBytesNeeded);
    last_error = GetLastError();
    if (*pcbBytesNeeded > cbBufSize) {
        ok = FALSE;
        last_error = ERROR_INSUFFICIENT_BUFFER;
    }
    if (! ok) {
        Dll_Free(cfg);
        SetLastError(last_error);
        return FALSE;
    }

    memcpy(lpServiceConfig, cfg, *pcbBytesNeeded);
    out = (QUERY_SERVICE_CONFIGA *)lpServiceConfig;

    if (out->lpBinaryPathName) {
        RtlInitUnicodeString(&uni, cfg->lpBinaryPathName);
        ansi.Length = 0;
        ansi.MaximumLength = 8192;
        ansi.Buffer = out->lpBinaryPathName;
        RtlUnicodeStringToAnsiString(&ansi, &uni, FALSE);
    }

    if (out->lpLoadOrderGroup) {
        RtlInitUnicodeString(&uni, cfg->lpLoadOrderGroup);
        ansi.Length = 0;
        ansi.MaximumLength = 8192;
        ansi.Buffer = out->lpLoadOrderGroup;
        RtlUnicodeStringToAnsiString(&ansi, &uni, FALSE);
    }

    if (out->lpDependencies) {
        RtlInitUnicodeString(&uni, cfg->lpDependencies);
        ansi.Length = 0;
        ansi.MaximumLength = 8192;
        ansi.Buffer = out->lpDependencies;
        RtlUnicodeStringToAnsiString(&ansi, &uni, FALSE);
    }

    if (out->lpServiceStartName) {
        RtlInitUnicodeString(&uni, cfg->lpServiceStartName);
        ansi.Length = 0;
        ansi.MaximumLength = 8192;
        ansi.Buffer = out->lpServiceStartName;
        RtlUnicodeStringToAnsiString(&ansi, &uni, FALSE);
    }

    if (out->lpDisplayName) {
        RtlInitUnicodeString(&uni, cfg->lpDisplayName);
        ansi.Length = 0;
        ansi.MaximumLength = 8192;
        ansi.Buffer = out->lpDisplayName;
        RtlUnicodeStringToAnsiString(&ansi, &uni, FALSE);
    }

    Dll_Free(cfg);

    SetLastError(0);
    return TRUE;
}


//---------------------------------------------------------------------------
// Scm_QueryServiceConfig2W
//---------------------------------------------------------------------------


_FX BOOL Scm_QueryServiceConfig2W(
    SC_HANDLE hService, DWORD InfoLevel, LPBYTE lpBuffer,
    DWORD cbBufSize, LPDWORD pcbBytesNeeded)
{
    SERVICE_QUERY_RPL *rpl;

    rpl = (SERVICE_QUERY_RPL *)
                    Scm_QueryServiceByHandle(hService, FALSE, InfoLevel);
    if (! rpl)
        return FALSE;

    if (pcbBytesNeeded)
        *pcbBytesNeeded = rpl->service_config_len;
    if (rpl->service_config_len > cbBufSize) {
        Dll_Free(rpl);
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    memcpy(lpBuffer, &rpl->service_config, rpl->service_config_len);
    Dll_Free(rpl);

    if (InfoLevel == SERVICE_CONFIG_DESCRIPTION) {

        SERVICE_DESCRIPTIONA *cfga = (SERVICE_DESCRIPTIONA *)lpBuffer;
        if (cfga->lpDescription)
            cfga->lpDescription     += (ULONG_PTR)cfga;

    } else if (InfoLevel == SERVICE_CONFIG_FAILURE_ACTIONS) {

        SERVICE_FAILURE_ACTIONSA *cfga =
                                    (SERVICE_FAILURE_ACTIONSA *)lpBuffer;
        if (cfga->lpRebootMsg)
            cfga->lpRebootMsg       += (ULONG_PTR)cfga;
        if (cfga->lpCommand)
            cfga->lpCommand         += (ULONG_PTR)cfga;
        if (cfga->lpsaActions) {
            UCHAR *lpsaActions = (UCHAR *)cfga->lpsaActions
                               + (ULONG_PTR)cfga;
            cfga->lpsaActions  = (SC_ACTION *)lpsaActions;
        }
    }

    SetLastError(0);
    return TRUE;
}


//---------------------------------------------------------------------------
// Scm_QueryServiceConfig2A
//---------------------------------------------------------------------------


_FX BOOL Scm_QueryServiceConfig2A(
    SC_HANDLE hService, DWORD InfoLevel, LPBYTE lpBuffer,
    DWORD cbBufSize, LPDWORD pcbBytesNeeded)
{
    UNICODE_STRING uni;
    ANSI_STRING ansi;
    ULONG len;
    WCHAR *wstr;

    BOOL ok = Scm_QueryServiceConfig2W(
                hService, InfoLevel, lpBuffer, cbBufSize, pcbBytesNeeded);
    if (! ok)
        return FALSE;

#define MyUnicodeToAnsiString2A(m)                          \
    if (info->m) {                                          \
        len  = (wcslen(info->m) + 1) * sizeof(WCHAR);       \
        wstr = Dll_AllocTemp(len);                          \
        memcpy(wstr, info->m, len);                         \
        RtlInitUnicodeString(&uni, wstr);                   \
        ansi.Length = 0;                                    \
        ansi.MaximumLength = (USHORT)len;                   \
        ansi.Buffer = (UCHAR *)info->m;                     \
        RtlUnicodeStringToAnsiString(&ansi, &uni, FALSE);   \
        Dll_Free(wstr);                                     \
    }

    if (InfoLevel == SERVICE_CONFIG_DESCRIPTION) {

        SERVICE_DESCRIPTIONW *info =
            (SERVICE_DESCRIPTIONW *)lpBuffer;

        MyUnicodeToAnsiString2A(lpDescription);

    } else if (InfoLevel == SERVICE_CONFIG_FAILURE_ACTIONS) {

        SERVICE_FAILURE_ACTIONSW *info =
            (SERVICE_FAILURE_ACTIONSW *)lpBuffer;

        MyUnicodeToAnsiString2A(lpRebootMsg);
        MyUnicodeToAnsiString2A(lpCommand);
    }

    SetLastError(0);
    return TRUE;
}


//---------------------------------------------------------------------------
// Scm_EnumServicesStatusX
//---------------------------------------------------------------------------


_FX BOOL Scm_EnumServicesStatusX(
    SC_HANDLE hSCManager, DWORD dwServiceType, DWORD dwServiceState,
    LPENUM_SERVICE_STATUS lpServices,
    DWORD cbBufSize, LPDWORD pcbBytesNeeded, LPDWORD lpServicesReturned,
    LPDWORD lpResumeHandle, BOOL IsUnicode, BOOL IsExtended)
{
    ULONG ServicesReturned = 0;
    ULONG BytesNeeded = 0;
    UCHAR *hptr, *lptr;
    WCHAR *ServiceName, *DisplayName;
    ULONG ServiceNameLen, DisplayNameLen;
    ULONG enumlen, svclen;
    ENUM_SERVICE_STATUS_PROCESS *ess;
    SERVICE_STATUS_PROCESS ss, *ssp;
    WCHAR *names_base, *names;
    QUERY_SERVICE_CONFIG *cfg;
    SC_HANDLE hService;
    ULONG idx;

    if (hSCManager != HANDLE_SERVICE_MANAGER) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    if ((dwServiceType & SERVICE_TYPE_ALL) == 0 ||
        (dwServiceState & SERVICE_STATE_ALL) == 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (IsExtended) {
        if (IsUnicode)
            enumlen = sizeof(ENUM_SERVICE_STATUS_PROCESSW);
        else
            enumlen = sizeof(ENUM_SERVICE_STATUS_PROCESSA);
    } else {
        if (IsUnicode)
            enumlen = sizeof(ENUM_SERVICE_STATUSW);
        else
            enumlen = sizeof(ENUM_SERVICE_STATUSA);
    }

    cfg = Dll_Alloc(8192);
    hService = NULL;

    lptr = (UCHAR *)lpServices;
    hptr = (UCHAR *)lpServices + cbBufSize - 1;

    names_base = names = Scm_GetAllServices();
    if (lpResumeHandle) {
        for (idx = 0; idx < *lpResumeHandle; ++idx) {
            if (! *names)
                break;
            names += wcslen(names) + 1;
        }
        idx = *lpResumeHandle;
    } else
        idx = 0;

    while (*names) {

        if (hService)
            Scm_CloseServiceHandle(hService);

        ServiceName = names;
        ServiceNameLen = wcslen(ServiceName) + 1;
        names += ServiceNameLen;
        ++idx;

        hService = Scm_OpenServiceW(
                        hSCManager, ServiceName, SERVICE_QUERY_STATUS);
        if (! hService)
            continue;

        if (! Scm_QueryServiceStatusEx(
                hService, SC_STATUS_PROCESS_INFO,
                (BYTE *)&ss, sizeof(SERVICE_STATUS_PROCESS), &svclen))
            continue;

        // service type and requested type must match
        if ((ss.dwServiceType & dwServiceType) == 0)
            continue;
        if (ss.dwCurrentState == SERVICE_STOPPED) {
            // if service state is SERVICE_STOPPED,
            // requested state must include SERVICE_INACTIVE
            if (! (dwServiceState & SERVICE_INACTIVE))
                continue;
        } else {
            // if service state is anything but SERVICE_STOPPED,
            // requested state must include SERVICE_ACTIVE
            if (! (dwServiceState & SERVICE_ACTIVE))
                continue;
        }

        if (Scm_QueryServiceConfigW(hService, cfg, 8192, NULL))
            DisplayName = cfg->lpDisplayName;
        else
            DisplayName = NULL;
        if (DisplayName)
            DisplayNameLen = wcslen(DisplayName) + 1;
        else
            DisplayNameLen = 0;
        if (IsUnicode) {
            ServiceNameLen *= sizeof(WCHAR);
            DisplayNameLen *= sizeof(WCHAR);
        }

        svclen = enumlen + ServiceNameLen + DisplayNameLen;

        if (BytesNeeded > 0 || cbBufSize < svclen) {
            if (BytesNeeded == 0 && lpResumeHandle)
                *lpResumeHandle = idx - 1;
            BytesNeeded += svclen;
            continue;
        }

        cbBufSize -= svclen;
        ++ServicesReturned;

        ess = (ENUM_SERVICE_STATUS_PROCESS *)lptr;

        if (IsUnicode) {
            hptr -= ServiceNameLen;
            memcpy(hptr, ServiceName, ServiceNameLen);
            ess->lpServiceName = (WCHAR *)hptr;
            if (DisplayName) {
                hptr -= DisplayNameLen;
                memcpy(hptr, DisplayName, DisplayNameLen);
                ess->lpDisplayName = (WCHAR *)hptr;
            } else
                ess->lpDisplayName = NULL;

        } else {
            UNICODE_STRING uni;
            ANSI_STRING ansi;

            hptr -= ServiceNameLen;
            RtlInitUnicodeString(&uni, ServiceName);
            ansi.Length = 0;
            ansi.MaximumLength = (USHORT)ServiceNameLen;
            ansi.Buffer = hptr;
            RtlUnicodeStringToAnsiString(&ansi, &uni, FALSE);
            ess->lpServiceName = (WCHAR *)hptr;

            if (DisplayName) {
                hptr -= DisplayNameLen;
                RtlInitUnicodeString(&uni, DisplayName);
                ansi.Length = 0;
                ansi.MaximumLength = (USHORT)DisplayNameLen;
                ansi.Buffer = hptr;
                RtlUnicodeStringToAnsiString(&ansi, &uni, FALSE);
                ess->lpDisplayName = (WCHAR *)hptr;
            } else
                ess->lpDisplayName = NULL;
        }

        ssp = &ess->ServiceStatusProcess;
        ssp->dwServiceType = ss.dwServiceType;
        ssp->dwCurrentState = ss.dwCurrentState;
        ssp->dwControlsAccepted = ss.dwControlsAccepted;
        ssp->dwWin32ExitCode = 0;
        ssp->dwServiceSpecificExitCode = 0;
        ssp->dwCheckPoint = 0;
        ssp->dwWaitHint = 0;
        if (IsExtended) {
            ssp->dwProcessId = ss.dwProcessId;
            ssp->dwServiceFlags = ss.dwServiceFlags;
        }
        lptr += enumlen;
    }

    if (hService)
        Scm_CloseServiceHandle(hService);

    Dll_Free(cfg);

    Dll_Free(names_base);

    *pcbBytesNeeded = BytesNeeded;
    *lpServicesReturned = ServicesReturned;
    if (BytesNeeded)
        SetLastError(ERROR_MORE_DATA);
    else
        SetLastError(0);
    return TRUE;
}


//---------------------------------------------------------------------------
// Scm_EnumServicesStatusW
//---------------------------------------------------------------------------


_FX BOOL Scm_EnumServicesStatusW(
    SC_HANDLE hSCManager, DWORD dwServiceType, DWORD dwServiceState,
    void *lpServices, DWORD cbBufSize, LPDWORD pcbBytesNeeded,
    LPDWORD lpServicesReturned, LPDWORD lpResumeHandle)
{
    return Scm_EnumServicesStatusX(
        hSCManager, dwServiceType, dwServiceState, lpServices, cbBufSize,
        pcbBytesNeeded, lpServicesReturned, lpResumeHandle, TRUE, FALSE);
}


//---------------------------------------------------------------------------
// Scm_EnumServicesStatusA
//---------------------------------------------------------------------------


_FX BOOL Scm_EnumServicesStatusA(
    SC_HANDLE hSCManager, DWORD dwServiceType, DWORD dwServiceState,
    void *lpServices, DWORD cbBufSize, LPDWORD pcbBytesNeeded,
    LPDWORD lpServicesReturned, LPDWORD lpResumeHandle)
{
    return Scm_EnumServicesStatusX(
        hSCManager, dwServiceType, dwServiceState, lpServices, cbBufSize,
        pcbBytesNeeded, lpServicesReturned, lpResumeHandle, FALSE, FALSE);
}


//---------------------------------------------------------------------------
// Scm_EnumServicesStatusExW
//---------------------------------------------------------------------------


_FX BOOL Scm_EnumServicesStatusExW(
    SC_HANDLE hSCManager, SC_ENUM_TYPE InfoLevel,
    DWORD dwServiceType, DWORD dwServiceState, void *lpServices,
    DWORD cbBufSize, LPDWORD pcbBytesNeeded,
    LPDWORD lpServicesReturned, LPDWORD lpResumeHandle, void *GroupName)
{
    if (InfoLevel != SC_ENUM_PROCESS_INFO) {
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }
    return Scm_EnumServicesStatusX(
        hSCManager, dwServiceType, dwServiceState, lpServices, cbBufSize,
        pcbBytesNeeded, lpServicesReturned, lpResumeHandle, TRUE, TRUE);
}


//---------------------------------------------------------------------------
// Scm_EnumServicesStatusExA
//---------------------------------------------------------------------------


_FX BOOL Scm_EnumServicesStatusExA(
    SC_HANDLE hSCManager, SC_ENUM_TYPE InfoLevel,
    DWORD dwServiceType, DWORD dwServiceState, void *lpServices,
    DWORD cbBufSize, LPDWORD pcbBytesNeeded,
    LPDWORD lpServicesReturned, LPDWORD lpResumeHandle, void *GroupName)
{
    if (InfoLevel != SC_ENUM_PROCESS_INFO) {
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }
    return Scm_EnumServicesStatusX(
        hSCManager, dwServiceType, dwServiceState, lpServices, cbBufSize,
        pcbBytesNeeded, lpServicesReturned, lpResumeHandle, FALSE, TRUE);
}


//---------------------------------------------------------------------------
// Scm_QueryServiceLockStatusW
//---------------------------------------------------------------------------


_FX BOOL Scm_QueryServiceLockStatusW(
    SC_HANDLE hService, void *lpServiceStatus,
    DWORD cbBufSize, LPDWORD pcbBytesNeeded)
{
    *pcbBytesNeeded = sizeof(QUERY_SERVICE_LOCK_STATUSW)
                    + sizeof(WCHAR);
    if (cbBufSize < *pcbBytesNeeded) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }
    memzero(lpServiceStatus, *pcbBytesNeeded);
    ((QUERY_SERVICE_LOCK_STATUSW *)lpServiceStatus)->lpLockOwner = (WCHAR *)
        ((UCHAR *)lpServiceStatus + sizeof(QUERY_SERVICE_LOCK_STATUSW));
    SetLastError(ERROR_SUCCESS);
    return TRUE;
}


//---------------------------------------------------------------------------
// Scm_QueryServiceLockStatusA
//---------------------------------------------------------------------------


_FX BOOL Scm_QueryServiceLockStatusA(
    SC_HANDLE hService, void *lpServiceStatus,
    DWORD cbBufSize, LPDWORD pcbBytesNeeded)
{
    *pcbBytesNeeded = sizeof(QUERY_SERVICE_LOCK_STATUSA)
                    + sizeof(WCHAR);    // +2 bytes even in ASCII version
    if (cbBufSize < *pcbBytesNeeded) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }
    memzero(lpServiceStatus, *pcbBytesNeeded);
    ((QUERY_SERVICE_LOCK_STATUSA *)lpServiceStatus)->lpLockOwner =
        (UCHAR *)lpServiceStatus + sizeof(QUERY_SERVICE_LOCK_STATUSA);
    SetLastError(ERROR_SUCCESS);
    return TRUE;
}


//---------------------------------------------------------------------------
// Scm_GetServiceDisplayNameW
//---------------------------------------------------------------------------


_FX BOOL Scm_GetServiceDisplayNameW(
    SC_HANDLE hSCManager, WCHAR *lpServiceName,
    WCHAR *lpDisplayName, LPDWORD lpcchBuffer)
{
    SERVICE_QUERY_RPL *rpl;
    ULONG need_len;
    WCHAR *display;

    if (lpDisplayName)
        *lpDisplayName = L'\0';

    rpl = Scm_QueryServiceByName(lpServiceName, FALSE, -1);
    if (! rpl)
        return FALSE;

    display = Scm_GetServiceConfigString(rpl, 'D');
    need_len = wcslen(display);

    if (need_len >= *lpcchBuffer) {
        *lpcchBuffer = need_len;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    wmemcpy(lpDisplayName, display, need_len + 1);
    *lpcchBuffer = need_len;

    SetLastError(0);
    return TRUE;
}


//---------------------------------------------------------------------------
// Scm_GetServiceDisplayNameA
//---------------------------------------------------------------------------


_FX BOOL Scm_GetServiceDisplayNameA(
    SC_HANDLE hSCManager, UCHAR *lpServiceName,
    UCHAR *lpDisplayName, LPDWORD lpcchBuffer)
{
    ANSI_STRING ansi;
    UNICODE_STRING uni;
    SERVICE_QUERY_RPL *rpl;
    ULONG need_len, i;
    WCHAR *display;

    if (lpDisplayName)
        *lpDisplayName = '\0';

    RtlInitString(&ansi, lpServiceName);
    RtlAnsiStringToUnicodeString(&uni, &ansi, TRUE);

    rpl = Scm_QueryServiceByName(uni.Buffer, FALSE, -1);

    RtlFreeUnicodeString(&uni);

    if (! rpl)
        return FALSE;

    display = Scm_GetServiceConfigString(rpl, 'D');
    need_len = wcslen(display);

    if (need_len >= *lpcchBuffer) {
        *lpcchBuffer = need_len;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    for (i = 0; i <= need_len; ++i)
        lpDisplayName[i] = (UCHAR)display[i];
    *lpcchBuffer = need_len;

    SetLastError(0);
    return TRUE;
}


//---------------------------------------------------------------------------
// Scm_GetServiceKeyNameW
//---------------------------------------------------------------------------


_FX BOOL Scm_GetServiceKeyNameW(
    SC_HANDLE hSCManager, WCHAR *lpDisplayName,
    WCHAR *lpServiceName, LPDWORD lpcchBuffer)
{
    WCHAR *names_base, *names, *display;
    SERVICE_QUERY_RPL *rpl;
    ULONG need_len;

    ULONG error = ERROR_SERVICE_DOES_NOT_EXIST;
    BOOLEAN ok = FALSE;

    if (lpServiceName)
        *lpServiceName = L'\0';

    rpl = NULL;
    names_base = names = Scm_GetAllServices();
    while (*names) {

        rpl = Scm_QueryServiceByName(names, FALSE, -1);
        if (! rpl) {
            error = GetLastError();
            break;
        }

        display = Scm_GetServiceConfigString(rpl, 'D');
        if (_wcsicmp(display, lpDisplayName) == 0)
            break;

        Dll_Free(rpl);
        rpl = NULL;

        names += wcslen(names) + 1;
    }

    if (! rpl)
        goto finish;

    need_len = wcslen(names);

    if (need_len >= *lpcchBuffer) {
        *lpcchBuffer = need_len;
        error = ERROR_INSUFFICIENT_BUFFER;
        goto finish;
    }

    wmemcpy(lpServiceName, names, need_len + 1);
    *lpcchBuffer = need_len;

    error = 0;
    ok = TRUE;

finish:

    if (rpl)
        Dll_Free(rpl);

    if (names_base)
        Dll_Free(names_base);

    SetLastError(error);
    return ok;
}


//---------------------------------------------------------------------------
// Scm_GetServiceKeyNameA
//---------------------------------------------------------------------------


_FX BOOL Scm_GetServiceKeyNameA(
    SC_HANDLE hSCManager, UCHAR *lpDisplayName,
    UCHAR *lpServiceName, LPDWORD lpcchBuffer)
{
    ANSI_STRING ansi;
    UNICODE_STRING uni;
    WCHAR *names_base, *names, *display;
    SERVICE_QUERY_RPL *rpl;
    ULONG need_len, i;

    ULONG error = ERROR_SERVICE_DOES_NOT_EXIST;
    BOOLEAN ok = FALSE;

    if (lpServiceName)
        *lpServiceName = '\0';

    RtlInitString(&ansi, lpDisplayName);
    RtlAnsiStringToUnicodeString(&uni, &ansi, TRUE);

    names_base = names = Scm_GetAllServices();
    while (*names) {

        rpl = Scm_QueryServiceByName(names, FALSE, -1);
        if (! rpl) {
            error = GetLastError();
            break;
        }

        display = Scm_GetServiceConfigString(rpl, 'D');
        if (_wcsicmp(display, uni.Buffer) == 0)
            break;

        Dll_Free(rpl);
        rpl = NULL;

        names += wcslen(names) + 1;
    }

    if (! rpl)
        goto finish;

    need_len = wcslen(names);

    if (need_len >= *lpcchBuffer) {
        *lpcchBuffer = need_len;
        error = ERROR_INSUFFICIENT_BUFFER;
        goto finish;
    }

    for (i = 0; i <= need_len; ++i)
        lpServiceName[i] = (UCHAR)names[i];
    *lpcchBuffer = need_len;

    error = 0;
    ok = TRUE;

finish:

    if (rpl)
        Dll_Free(rpl);

    if (names_base)
        Dll_Free(names_base);

    RtlFreeUnicodeString(&uni);

    SetLastError(error);
    return ok;
}


//---------------------------------------------------------------------------
// Scm_EnumDependentServicesW
//---------------------------------------------------------------------------


_FX BOOL Scm_EnumDependentServicesW(
    SC_HANDLE hService, DWORD dwServiceState,
    LPENUM_SERVICE_STATUS lpServices, DWORD cbBufSize,
    LPDWORD pcbBytesNeeded, LPDWORD lpServicesReturned)
{
    *lpServicesReturned = 0;
    SetLastError(0);
    return TRUE;
}


//---------------------------------------------------------------------------
// Scm_EnumDependentServicesA
//---------------------------------------------------------------------------


_FX BOOL Scm_EnumDependentServicesA(
    SC_HANDLE hService, DWORD dwServiceState,
    LPENUM_SERVICE_STATUS lpServices, DWORD cbBufSize,
    LPDWORD pcbBytesNeeded, LPDWORD lpServicesReturned)
{
    *lpServicesReturned = 0;
    SetLastError(0);
    return TRUE;
}


//---------------------------------------------------------------------------
// Scm_QueryServiceObjectSecurity
//---------------------------------------------------------------------------


_FX BOOL Scm_QueryServiceObjectSecurity(
    SC_HANDLE hService,
    SECURITY_INFORMATION dwSecurityInformation,
    PSECURITY_DESCRIPTOR lpSecurityDescriptor,
    DWORD cbBufSize, LPDWORD pcbBytesNeeded)
{
    static const UCHAR sid[12] = {
        1,                                      // Revision
        1,                                      // SubAuthorityCount
        0,0,0,0,0,5, // SECURITY_NT_AUTHORITY   // IdentifierAuthority
        SECURITY_LOCAL_SYSTEM_RID               // SubAuthority
    };
    static const UCHAR dacl[96] = {
        2,                                      // AclRevision
        0,                                      // Sbz1
        96, 0,                                  // AclSize
        4, 0,                                   // AceCount
        0, 0,                                   // Sbz2
        // ACEs follow
        0x00, 0x00, 0x14, 0x00, 0xFD, 0x01, 0x02, 0x00,
        0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05,
        0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00,
        0xFF, 0x01, 0x0F, 0x00, 0x01, 0x02, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x05, 0x20, 0x00, 0x00, 0x00,
        0x20, 0x02, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00,
        0x8D, 0x01, 0x02, 0x00, 0x01, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x05, 0x0B, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x18, 0x00, 0xFD, 0x01, 0x02, 0x00,
        0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05,
        0x20, 0x00, 0x00, 0x00, 0x23, 0x02, 0x00, 0x00
    };

    SECURITY_DESCRIPTOR_RELATIVE *sd;
    ULONG need_len;
    ULONG offset;

    if (! Scm_GetHandleName(hService))
        return FALSE;

    //
    // compute minimum length needed
    //

    if ((! dwSecurityInformation) ||
        (dwSecurityInformation & ~(
            DACL_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION |
            OWNER_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION))) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    need_len = sizeof(SECURITY_DESCRIPTOR_RELATIVE);
    if (dwSecurityInformation & DACL_SECURITY_INFORMATION)
        need_len += sizeof(dacl);
    if (dwSecurityInformation & OWNER_SECURITY_INFORMATION)
        need_len += sizeof(sid);
    if (dwSecurityInformation & GROUP_SECURITY_INFORMATION)
        need_len += sizeof(sid);

    *pcbBytesNeeded = need_len;
    if (need_len > cbBufSize) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    //
    // fill output buffer
    //

    sd = (SECURITY_DESCRIPTOR_RELATIVE *)lpSecurityDescriptor;
    sd->Revision = 1;
    sd->Sbz1 = 0;
    sd->Control = SE_SELF_RELATIVE;
    sd->Owner = 0;
    sd->Group = 0;
    sd->Sacl = 0;
    sd->Dacl = 0;

    offset = sizeof(SECURITY_DESCRIPTOR_RELATIVE);

    if (dwSecurityInformation & DACL_SECURITY_INFORMATION) {
        sd->Control |= SE_DACL_PRESENT;
        sd->Dacl = offset;
        memcpy(((UCHAR *)sd) + offset, dacl, sizeof(dacl));
        offset += sizeof(dacl);
    }

    if (dwSecurityInformation & OWNER_SECURITY_INFORMATION) {
        sd->Owner = offset;
        memcpy(((UCHAR *)sd) + offset, sid, sizeof(sid));
        offset += sizeof(sid);
    }

    if (dwSecurityInformation & GROUP_SECURITY_INFORMATION) {
        sd->Group = offset;
        memcpy(((UCHAR *)sd) + offset, sid, sizeof(sid));
        offset += sizeof(sid);
    }

    SetLastError(0);
    return TRUE;
}


//---------------------------------------------------------------------------
// Scm_SetServiceObjectSecurity
//---------------------------------------------------------------------------


_FX BOOL Scm_SetServiceObjectSecurity(
    SC_HANDLE hService,
    SECURITY_INFORMATION dwSecurityInformation,
    PSECURITY_DESCRIPTOR lpSecurityDescriptor)
{
    if (! Scm_GetHandleName(hService))
        return FALSE;
    SetLastError(0);
    return TRUE;
}
