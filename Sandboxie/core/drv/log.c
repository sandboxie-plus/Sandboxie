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
// Message log
//---------------------------------------------------------------------------


#include "driver.h"
#include "log.h"
#include "api.h"
#include "util.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static void Log_Event_Msg(
    NTSTATUS error_code,
    const WCHAR *string1,
    const WCHAR *string2);

static void Log_Popup_Msg_2(
    NTSTATUS error_code,
    const WCHAR *string1, ULONG string1_len,
    const WCHAR *string2, ULONG string2_len,
    ULONG session_id);


//---------------------------------------------------------------------------
// Log_Event_Msg
//---------------------------------------------------------------------------


_FX void Log_Event_Msg(
    NTSTATUS error_code,
    const WCHAR *string1,
    const WCHAR *string2)
{
    int entry_size;
    int string1_len = 0;
    int string2_len = 0;

    if (string1)
        string1_len = (wcslen(string1) + 1) * sizeof(WCHAR);
    if (string2)
        string2_len = (wcslen(string2) + 1) * sizeof(WCHAR);
    entry_size = sizeof(IO_ERROR_LOG_PACKET) + string1_len + string2_len;
    if (entry_size <= ERROR_LOG_MAXIMUM_SIZE) {

        IO_ERROR_LOG_PACKET *entry =
            (IO_ERROR_LOG_PACKET *)IoAllocateErrorLogEntry(
                Driver_Object, (UCHAR)entry_size);

        if (entry) {
            UCHAR *strings;

            entry->RetryCount = 0;
            entry->DumpDataSize = 0;

            entry->NumberOfStrings = 0;
            strings = ((UCHAR *)entry) + sizeof(IO_ERROR_LOG_PACKET);
            if (string1_len > 0) {
                ++entry->NumberOfStrings;
                memcpy(strings, string1, string1_len);
                strings += string1_len;
            }
            if (string2_len > 0) {
                ++entry->NumberOfStrings;
                memcpy(strings, string2, string2_len);
                strings += string2_len;
            }
            entry->StringOffset = sizeof(IO_ERROR_LOG_PACKET);

            entry->ErrorCode = error_code;
            entry->FinalStatus = STATUS_SUCCESS;

            IoWriteErrorLogEntry(entry);
        }
    }
}


//---------------------------------------------------------------------------
// Log_Popup_Msg
//---------------------------------------------------------------------------


_FX void Log_Popup_Msg(
    NTSTATUS error_code,
    const WCHAR *string1,
    const WCHAR *string2,
    ULONG session_id)
{
    ULONG string1_len, string2_len;

    if (string1)
        string1_len = wcslen(string1);
    else
        string1_len = 0;

    if (string2)
        string2_len = wcslen(string2);
    else
        string2_len = 0;

    //
    // log message to target session
    //

    if (session_id == -1) {
        NTSTATUS status = MyGetSessionId(&session_id);
        if (! NT_SUCCESS(status))
            session_id = 0;
    }

    if ((Driver_OsVersion >= DRIVER_WINDOWS_VISTA) && (session_id == 0))
        session_id = 1;

    Log_Popup_Msg_2(
        error_code, string1, string1_len, string2, string2_len, session_id);

    //
    // log message to SbieSvc and trigger SbieSvc to wake up and collect it
    //

    Log_Popup_Msg_2(
        error_code, string1, string1_len, string2, string2_len, -1);

    string1_len = 0;
    Api_SendServiceMessage(SVC_LOG_MESSAGE, sizeof(ULONG), &string1_len);

    // DbgPrint("POPUP %04d %S %S\n", error_code & 0xFFFF, string1, string2);
}


//---------------------------------------------------------------------------
// Log_Popup_Msg_2
//---------------------------------------------------------------------------


_FX void Log_Popup_Msg_2(
    NTSTATUS error_code,
    const WCHAR *string1, ULONG string1_len,
    const WCHAR *string2, ULONG string2_len,
    ULONG session_id)
{
    API_WORK_ITEM *work_item;
    ULONG length;
    WCHAR *ptr;

    length = sizeof(API_WORK_ITEM)
           + sizeof(ULONG)      // msgid
           + (string1_len + 1) * sizeof(WCHAR)
           + (string2_len + 1) * sizeof(WCHAR);

    //
    // prepare work item
    //

    work_item = Mem_Alloc(Driver_Pool, length);
    if (work_item) {

        work_item->length = length;

        work_item->session_id = session_id;

        work_item->type = API_LOG_MESSAGE;

        work_item->data[0] = error_code;

        ptr = (WCHAR *)&work_item->data[1];

        if (string1_len) {
            wmemcpy(ptr, string1, string1_len);
            ptr += string1_len;
        }
        *ptr = L'\0';
        ++ptr;

        if (string2_len) {
            wmemcpy(ptr, string2, string2_len);
            ptr += string2_len;
        }
        *ptr = L'\0';

        Api_AddWork(work_item);
    }
}


//---------------------------------------------------------------------------
// Log_Msg
//---------------------------------------------------------------------------


_FX void Log_Msg(
    NTSTATUS error_code,
    const WCHAR *string1,
    const WCHAR *string2)
{
    Log_Msg_Session(error_code, string1, string2, -1);
}


//---------------------------------------------------------------------------
// Log_Msg_Session
//---------------------------------------------------------------------------


_FX void Log_Msg_Session(
    NTSTATUS error_code,
    const WCHAR *string1,
    const WCHAR *string2,
    ULONG session_id)
{
    ULONG facility = (error_code >> 16) & 0x0F;
    if (facility & MSG_FACILITY_EVENT)
        Log_Event_Msg(error_code, string1, string2);
    if (facility & MSG_FACILITY_POPUP)
        Log_Popup_Msg(error_code, string1, string2, session_id);
}


//---------------------------------------------------------------------------
// Log_Status_Ex
//---------------------------------------------------------------------------


_FX void Log_Status_Ex(
    NTSTATUS error_code,
    ULONG error_subcode,
    NTSTATUS nt_status,
    const WCHAR *string2)
{
    Log_Status_Ex_Session(error_code, error_subcode, nt_status, string2, -1);
}


//---------------------------------------------------------------------------
// Log_Status_Ex_Session
//---------------------------------------------------------------------------


_FX void Log_Status_Ex_Session(
    NTSTATUS error_code,
    ULONG error_subcode,
    NTSTATUS nt_status,
    const WCHAR *string2,
    ULONG session_id)
{
    WCHAR str[100];

    if (error_subcode)
        swprintf(str, L"[%08X / %02X]", nt_status, error_subcode);
    else
        swprintf(str, L"[%08X]", nt_status);

    Log_Msg_Session(error_code, str, string2, session_id);
}


//---------------------------------------------------------------------------
// Log_Debug_Msg
//---------------------------------------------------------------------------


_FX void Log_Debug_Msg(const WCHAR *string1, const WCHAR *string2)
{
    DbgPrint("(%06d) SBIE %S %S\n",
        PsGetCurrentProcessId(), string1, string2);
}
