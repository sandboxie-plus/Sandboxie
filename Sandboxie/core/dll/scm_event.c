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
// Functions
//---------------------------------------------------------------------------


static HANDLE Scm_RegisterEventSourceW(WCHAR *ServerName, WCHAR *SourceName);

static HANDLE Scm_RegisterEventSourceA(UCHAR *ServerName, UCHAR *SourceName);

static BOOL Scm_DeregisterEventSource(HANDLE hEventLog);

static BOOL Scm_ReportEventW(
    HANDLE hEventLog, WORD wType, WORD wCategory, DWORD dwEventID,
    PSID lpUserSid, WORD wNumStrings, DWORD dwDataSize,
    WCHAR *Strings, void *RawData);

static BOOL Scm_ReportEventA(
    HANDLE hEventLog, WORD wType, WORD wCategory, DWORD dwEventID,
    PSID lpUserSid, WORD wNumStrings, DWORD dwDataSize,
    UCHAR *Strings, void *RawData);

static BOOL Scm_CloseEventLog(
    HANDLE hEventLog);

//---------------------------------------------------------------------------
// Scm_RegisterEventSourceW
//---------------------------------------------------------------------------


_FX HANDLE Scm_RegisterEventSourceW(WCHAR *ServerName, WCHAR *SourceName)
{
    SetLastError(0);
    return (HANDLE)HANDLE_EVENT_LOG;
}

/*
    NTSTATUS status;
    WCHAR keyname[128];
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    HANDLE handle;
    ULONG retries;
    union {
        KEY_VALUE_PARTIAL_INFORMATION info;
        WCHAR info_space[256];
    } u;
    ULONG len;
    WCHAR *name = NULL;

    for (retries = 0; retries < 2; ++retries) {

        wcscpy(keyname,
            L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Services"
            L"\\EventLog\\");
        if (retries == 0)
            wcscat(keyname, L"Application\\");
        else if (retries == 1)
            wcscat(keyname, L"System\\");

        wcscat(keyname, SourceName);
        RtlInitUnicodeString(&objname, keyname);

        InitializeObjectAttributes(
            &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

        status = NtOpenKey(&handle, KEY_QUERY_VALUE, &objattrs);
        if (! NT_SUCCESS(status))
            continue;

        RtlInitUnicodeString(&objname, L"EventMessageFile");
        status = NtQueryValueKey(
            handle, &objname, KeyValuePartialInformation,
            &u.info, sizeof(u), &len);

        NtClose(handle);

        if (! NT_SUCCESS(status) || u.info.DataLength == 0 ||
                (u.info.Type != REG_SZ && u.info.Type != REG_EXPAND_SZ)) {

            SetLastError(ERROR_INVALID_HANDLE);
            return NULL;
        }

        u.info_space[250] = L'\0';
        name = Dll_Alloc(1028 * sizeof(WCHAR));
        ExpandEnvironmentStrings((WCHAR *)u.info.Data, name, 1024);
        SetLastError(0);
        return name;
    }

    name = Dll_Alloc(sizeof(WCHAR));
    name[0] = L'\0';
    SetLastError(0);
    return name;
}
*/


//---------------------------------------------------------------------------
// Scm_RegisterEventSourceA
//---------------------------------------------------------------------------


_FX HANDLE Scm_RegisterEventSourceA(UCHAR *ServerName, UCHAR *SourceName)
{
    ANSI_STRING ansi;
    UNICODE_STRING uni;
    HANDLE handle;
    ULONG error;

    RtlInitString(&ansi, SourceName);
    RtlAnsiStringToUnicodeString(&uni, &ansi, TRUE);

    handle = Scm_RegisterEventSourceW(NULL, uni.Buffer);
    error = GetLastError();

    RtlFreeUnicodeString(&uni);

    SetLastError(error);
    return handle;
}


//---------------------------------------------------------------------------
// Scm_DeregisterEventSource
//---------------------------------------------------------------------------


_FX BOOL Scm_DeregisterEventSource(HANDLE hEventLog)
{
    SetLastError(0);
    return TRUE;
}


//---------------------------------------------------------------------------
// Scm_ReportEventW
//---------------------------------------------------------------------------


_FX BOOL Scm_ReportEventW(
    HANDLE hEventLog, WORD wType, WORD wCategory, DWORD dwEventID,
    PSID lpUserSid, WORD wNumStrings, DWORD dwDataSize,
    WCHAR *Strings, void *RawData)
{
    SetLastError(0);
    return TRUE;
}


//---------------------------------------------------------------------------
// Scm_ReportEventA
//---------------------------------------------------------------------------


_FX BOOL Scm_ReportEventA(
    HANDLE hEventLog, WORD wType, WORD wCategory, DWORD dwEventID,
    PSID lpUserSid, WORD wNumStrings, DWORD dwDataSize,
    UCHAR *Strings, void *RawData)
{
    SetLastError(0);
    return TRUE;
}


//---------------------------------------------------------------------------
// Scm_CloseEventLog
//---------------------------------------------------------------------------


_FX BOOL Scm_CloseEventLog(
    HANDLE hEventLog)
{
    if (hEventLog == (HANDLE)HANDLE_EVENT_LOG)
    {
        SetLastError(0);
        return TRUE;
    }
    return __sys_CloseEventLog(hEventLog);
}
