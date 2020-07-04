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
// Driver Assistant, log messages
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct WORK_ITEM {
    ULONG type;
    ULONG data[1];
} WORK_ITEM;


//---------------------------------------------------------------------------
// LogMessage
//---------------------------------------------------------------------------


void DriverAssist::LogMessage()
{
    EnterCriticalSection(&m_LogMessage_CritSec);

    ULONG m_workItemLen = 4096;
    void *m_workItemBuf = NULL;

    while (1) {

        m_workItemBuf = HeapAlloc(GetProcessHeap(), 0, m_workItemLen);
        if (! m_workItemBuf)
            break;

        ULONG len = m_workItemLen;
		ULONG message_number = m_last_message_number;
		ULONG code = -1;
		ULONG pid = 0;
		ULONG status = SbieApi_GetMessage(&message_number, -1, &code, &pid, (wchar_t*)m_workItemBuf, len);

        if (status == STATUS_BUFFER_TOO_SMALL) {
            HeapFree(GetProcessHeap(), 0, m_workItemBuf);
            m_workItemBuf = NULL;
            m_workItemLen += 4096;
            continue;
        }

        if (status != 0)
            break; // error or no more entries

		//if (message_number != m_last_message_number + 1)
		//	we missed something
		m_last_message_number = message_number;

		if (code == 0)
			break; // empty dummy

		LogMessage_Single(code, (wchar_t*)m_workItemBuf);
    }

    if (m_workItemBuf)
        HeapFree(GetProcessHeap(), 0, m_workItemBuf);

    LeaveCriticalSection(&m_LogMessage_CritSec);
}


//---------------------------------------------------------------------------
// LogMessage_Single
//---------------------------------------------------------------------------


void DriverAssist::LogMessage_Single(ULONG code, wchar_t* data)
{
    //
    // check if logging is enabled
    //

    union {
        KEY_VALUE_PARTIAL_INFORMATION info;
        WCHAR space[MAX_PATH + 8];
    } u;

    if (! SbieDll_GetServiceRegistryValue(L"LogFile", &u.info, sizeof(u)))
        return;
    if (u.info.Type != REG_SZ || u.info.DataLength >= sizeof(u))
        return;

    WCHAR *path = (WCHAR *)u.info.Data;
    WCHAR LogVer = *path;
    if (LogVer != L'1' && LogVer != L'2')
        return;
    ++path;
    if (*path != L';')
        return;
    ++path;

    //
    // get log message
    //

    if (code == MSG_2199)
        return;

    WCHAR *str1 = data;
    ULONG str1_len = wcslen(str1);
    WCHAR *str2 = str1 + str1_len + 1;
    ULONG str2_len = wcslen(str2);

    WCHAR *text = SbieDll_FormatMessage2(code, str1, str2);
    if (! text)
        return;

    //
    // log version 2, add timestamp
    //

    if (LogVer == L'2') {

        WCHAR *text2 = (WCHAR *)LocalAlloc(
            LMEM_FIXED, (wcslen(text) + 64) * sizeof(WCHAR));
        if (! text2) {
            LocalFree(text);
            return;
        }

        SYSTEMTIME st;
        GetLocalTime(&st);

        wsprintf(text2, L"%04d-%02d-%02d %02d:%02d:%02d %s",
            st.wYear, st.wMonth, st.wDay,
            st.wHour, st.wMinute, st.wSecond,
            text);

        LocalFree(text);
        text = text2;
    }

    //
    // write message to main log file and secondary log files
    //

    LogMessage_Write(path, text);

    LogMessage_Multi(code, path, text);

    LocalFree(text);
}


//---------------------------------------------------------------------------
// LogMessage_Multi
//---------------------------------------------------------------------------


void DriverAssist::LogMessage_Multi(
    ULONG msgid, const WCHAR *path, const WCHAR *text)
{
    union {
        KEY_VALUE_PARTIAL_INFORMATION info;
        WCHAR space[256];
    } u;

    if (! SbieDll_GetServiceRegistryValue(L"MultiLog", &u.info, sizeof(u)))
        return;
    if (u.info.Type != REG_SZ || u.info.DataLength >= sizeof(u))
        return;

    WCHAR *ptr = (WCHAR *)u.info.Data;
    while (*ptr) {
        if (_wtoi(ptr) == (msgid & 0xFFFF))
            break;
        while (*ptr && *ptr != L',' && *ptr != L';')
            ++ptr;
        if (! (*ptr))
            return;
        ++ptr;
    }

    WCHAR *ptr2 = (WCHAR*)wcsrchr(text, L']');
    if (! ptr2)
        return;
    ptr = ptr2;
    while (ptr > text && *ptr != L'[')
        --ptr;
    if ((ptr == text) || (ptr2 - ptr <= 1) || (ptr2 - ptr > 34))
        return;
    WCHAR boxname[40];
    wmemcpy(boxname, ptr + 1, ptr2 - ptr - 1);
    boxname[ptr2 - ptr - 1] = L'\0';

    LONG rc = SbieApi_IsBoxEnabled(boxname);
    if (rc != STATUS_SUCCESS && rc != STATUS_ACCOUNT_RESTRICTION)
        return;

    ptr = wcsrchr((WCHAR*)path, L'.');
    if (! ptr)
        return;
    ULONG len = wcslen(path) + 128;
    WCHAR *path2 = (WCHAR *)HeapAlloc(
                                GetProcessHeap(), 0, len * sizeof(WCHAR));
    if (! path2)
        return;
    wmemcpy(path2, path, ptr - path);
    path2[ptr - path] = L'_';
    wcscpy(&path2[ptr - path + 1], boxname);
    wcscat(path2, ptr);

    LogMessage_Write(path2, text);

    HeapFree(GetProcessHeap(), 0, path2);
}


//---------------------------------------------------------------------------
// LogMessage_Write
//---------------------------------------------------------------------------


void DriverAssist::LogMessage_Write(const WCHAR *path, const WCHAR *text)
{
    HANDLE hFile = CreateFile(
        path, FILE_GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
        return;

    SetFilePointer(hFile, 0, NULL, FILE_END);

    ULONG bytes;
    static const WCHAR *crlf = L"\r\n";
    WriteFile(hFile, text, wcslen(text) * sizeof(WCHAR), &bytes, NULL);
    WriteFile(hFile, crlf, wcslen(crlf) * sizeof(WCHAR), &bytes, NULL);

    CloseHandle(hFile);
}
