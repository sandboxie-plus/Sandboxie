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
// Configuration (User Management)
//---------------------------------------------------------------------------


#include "conf.h"
#include "api.h"
#include "process.h"


//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------


typedef struct _CONF_USER {

    LIST_ELEM list_elem;
    ULONG len;
    ULONG sid_len;
    ULONG name_len;
    WCHAR *sid;
    WCHAR *name;
    WCHAR space[1];

} CONF_USER;


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static LIST Conf_Users;
static PERESOURCE Conf_Users_Lock = NULL;
static KEVENT *Conf_Users_Event = NULL;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN Conf_GetUserNameForSid(
    const WCHAR *SidString, ULONG SessionId, WCHAR *varvalue);

static BOOLEAN Conf_GetGroupsForSid(WCHAR *PageSizeBuffer, ULONG SessionId);

static BOOLEAN Conf_FindUserName(const WCHAR *needle, const WCHAR *haystack);


//---------------------------------------------------------------------------


static NTSTATUS Conf_Api_SetUserName(PROCESS *proc, ULONG64 *parms);

static NTSTATUS Conf_Api_IsBoxEnabled(PROCESS *proc, ULONG64 *parms);


//---------------------------------------------------------------------------


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, Conf_Init_User)
#endif // ALLOC_PRAGMA


//---------------------------------------------------------------------------
// Conf_Init_User
//---------------------------------------------------------------------------


_FX BOOLEAN Conf_Init_User(void)
{
    List_Init(&Conf_Users);

    Conf_Users_Event = ExAllocatePoolWithTag(NonPagedPool, sizeof(KEVENT), tzuk);
    if (! Conf_Users_Event) {
        Log_Msg0(MSG_1104);
        return FALSE;
    }
    KeInitializeEvent(Conf_Users_Event, SynchronizationEvent, FALSE);

    if (! Mem_GetLockResource(&Conf_Users_Lock, TRUE))
        return FALSE;

    Api_SetFunction(API_SET_USER_NAME,  Conf_Api_SetUserName);
    Api_SetFunction(API_IS_BOX_ENABLED, Conf_Api_IsBoxEnabled);

    return TRUE;
}


//---------------------------------------------------------------------------
// Conf_Unload_User
//---------------------------------------------------------------------------


_FX void Conf_Unload_User(void)
{
    if (Conf_Users_Event) {
        ExFreePoolWithTag(Conf_Users_Event, tzuk);
        Conf_Users_Event = NULL;
    }

    Mem_FreeLockResource(&Conf_Users_Lock);
}


//---------------------------------------------------------------------------
// Conf_GetUserNameForSid
//---------------------------------------------------------------------------


_FX BOOLEAN Conf_GetUserNameForSid(
    const WCHAR *SidString, ULONG SessionId, WCHAR *varvalue)
{
    static const WCHAR *_unknown = L"unknown";
    ULONG sid_len;
    ULONG retries;
    BOOLEAN message_sent;

    sid_len = wcslen(SidString);
    message_sent = FALSE;

    for (retries = 0; (retries < 40) && (! Driver_Unloading); ++retries) {

        //
        // look for our sid in the list of known Conf_Users
        //

        CONF_USER *user1;
        LARGE_INTEGER time;
        KIRQL irql;

        KeRaiseIrql(APC_LEVEL, &irql);
        ExAcquireResourceExclusiveLite(Conf_Users_Lock, TRUE);

        user1 = List_Head(&Conf_Users);
        while (user1) {
            if (user1->sid_len == sid_len &&
                    _wcsicmp(user1->sid, SidString) == 0)
                break;
            user1 = List_Next(user1);
        }

        //
        // if we found our SID, translate the user name
        //

        if (user1 && varvalue)
            wcscpy(varvalue, user1->name);

        ExReleaseResourceLite(Conf_Users_Lock);
        KeLowerIrql(irql);

        if (user1)
            return TRUE;

        //
        // we couldn't find the SID, so ask SbieSvc to translate
        //

        if (! message_sent) {

            ULONG msg_len = sizeof(SVC_LOOKUP_SID_MSG)
                          + (sid_len + 4) * sizeof(WCHAR);

            SVC_LOOKUP_SID_MSG *msg = Mem_Alloc(Driver_Pool, msg_len);
            if (! msg)
                break;

            msg->session_id = SessionId;
            wcscpy(msg->sid_string, SidString);

            message_sent =
                Api_SendServiceMessage(SVC_LOOKUP_SID, msg_len, msg);

            Mem_Free(msg, msg_len);

            if (! message_sent)
                break;
        }

        //
        // now wait for SbieSvc to report back
        //

        /*Conf_AdjustUseCount(TRUE);
        time.QuadPart = -(SECONDS(1) * 7);
        KeDelayExecutionThread(KernelMode, FALSE, &time);
        Conf_AdjustUseCount(FALSE);*/

        time.QuadPart = -(SECONDS(1) / 4);
        KeWaitForSingleObject(
            Conf_Users_Event, Executive, KernelMode, FALSE, &time);
    }

    Log_Msg1(MSG_CONF_USER_NAME, SidString);
    if (varvalue)
        wcscpy(varvalue, _unknown);
    return FALSE;
}


//---------------------------------------------------------------------------
// Conf_Expand_UserName
//---------------------------------------------------------------------------


_FX BOOLEAN Conf_Expand_UserName(CONF_EXPAND_ARGS *args, WCHAR *varvalue)
{
    return Conf_GetUserNameForSid(args->sid, *args->session, varvalue);
}


//---------------------------------------------------------------------------
// Conf_GetGroupsForSid
//---------------------------------------------------------------------------


_FX BOOLEAN Conf_GetGroupsForSid(WCHAR *PageSizeBuffer, ULONG SessionId)
{
    UNICODE_STRING uni;
    PACCESS_TOKEN pAccessToken;
    TOKEN_GROUPS *groups;
    WCHAR *bufptr;
    NTSTATUS status;
    ULONG i;

    bufptr = PageSizeBuffer;

    pAccessToken = PsReferencePrimaryToken(PsGetCurrentProcess());
    status = SeQueryInformationToken(pAccessToken, TokenGroups, &groups);

    if (NT_SUCCESS(status)) {

        for (i = 0; i < groups->GroupCount; ++i) {

            ULONG attrs = groups->Groups[i].Attributes;
            if (! (attrs & SE_GROUP_ENABLED))
                continue;
            if (attrs & (SE_GROUP_USE_FOR_DENY_ONLY | SE_GROUP_LOGON_ID |
                         SE_GROUP_INTEGRITY | SE_GROUP_INTEGRITY_ENABLED))
                continue;

            uni.Length = 0;
            uni.MaximumLength = 512;
            uni.Buffer = bufptr;

            status = RtlConvertSidToUnicodeString(
                            &uni, groups->Groups[i].Sid, FALSE);

            if (NT_SUCCESS(status)) {

                if (! Conf_GetUserNameForSid(bufptr, SessionId, bufptr))
                    status = STATUS_OBJECT_NAME_NOT_FOUND;
            }

            if (! NT_SUCCESS(status)) {
                bufptr[0] = L'?';
                bufptr[1] = L'\0';
            }

            bufptr += wcslen(bufptr) + 1;
            if (bufptr - PageSizeBuffer > 1024)
                break;
        }

        //
        // on Windows Vista, check for UAC split token
        //

        if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA &&
                                        (bufptr - PageSizeBuffer < 1024)) {

            ULONG *pElevationType;
            NTSTATUS status = SeQueryInformationToken(
                pAccessToken, TokenElevationType, &pElevationType);
            if (NT_SUCCESS(status)) {
                if (*pElevationType == TokenElevationTypeFull ||
                    *pElevationType == TokenElevationTypeLimited) {

                    wcscpy(bufptr, L"Administrators");
                    bufptr += wcslen(bufptr) + 1;
                }
                ExFreePool(pElevationType);
            }
        }
    }

    *bufptr = L'\0';

    PsDereferencePrimaryToken(pAccessToken);

    return (NT_SUCCESS(status) ? TRUE : FALSE);
}


//---------------------------------------------------------------------------
// Conf_FindUserName
//---------------------------------------------------------------------------


_FX BOOLEAN Conf_FindUserName(const WCHAR *needle, const WCHAR *haystack)
{
    const WCHAR *needle_ptr;

    if (! (*needle))
        return FALSE;

    while (1) {

        while (*haystack == L' ' || *haystack == L'\t' || *haystack == L',')
            ++haystack;
        if (! (*haystack))
            break;

        for (needle_ptr = needle; *needle_ptr; ++needle_ptr) {
            WCHAR x = *haystack;
            ++haystack;
            if (x == L',')
                break;
            x = (x == L' ') ? L'_' : towlower(x);
            if (x != towlower(*needle_ptr))
                break;
        }

        if (! (*needle_ptr)) {
            while (*haystack == L' ' || *haystack == L'\t')
                ++haystack;
            if (*haystack == L',' || (! (*haystack)))
                return TRUE;
        }
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// Conf_IsBoxEnabled
//---------------------------------------------------------------------------


_FX BOOLEAN Conf_IsBoxEnabled(
    const WCHAR *BoxName, const WCHAR *SidString, ULONG SessionId)
{
    const WCHAR *value;
    WCHAR *buffer;
    BOOLEAN enabled;

    //
    // expect setting  Enabled=y,
    // and potentially Enabled=y,user1,user2,...
    //

    enabled = FALSE;

    Conf_AdjustUseCount(TRUE);

    value = Conf_Get(BoxName, L"Enabled", CONF_GET_NO_GLOBAL);
    if ((! value) || (*value != L'y' && *value != L'Y'))
        goto release_and_return;

    value = wcschr(value, L',');
    if (! value) {
        enabled = TRUE;
        goto release_and_return;
    }

    //
    // check if the username or any of the groupnames
    // appears in the Enabled setting
    //

    buffer = ExAllocatePoolWithTag(PagedPool, PAGE_SIZE, tzuk);
    if (buffer) {

        if (Conf_GetUserNameForSid(SidString, SessionId, buffer)) {

            if (Conf_FindUserName(buffer, value))
                enabled = TRUE;

            else if (Conf_GetGroupsForSid(buffer, SessionId)) {

                WCHAR *group = buffer;
                while (*group) {

                    if (Conf_FindUserName(group, value)) {
                        enabled = TRUE;
                        break;
                    }

                    group += wcslen(group) + 1;
                }
            }
        }

        ExFreePoolWithTag(buffer, tzuk);
    }

release_and_return:

    Conf_AdjustUseCount(FALSE);

    return enabled;
}


//---------------------------------------------------------------------------
// Conf_Api_SetUserName
//---------------------------------------------------------------------------


NTSTATUS Conf_Api_SetUserName(PROCESS *proc, ULONG64 *parms)
{
    API_SET_USER_NAME_ARGS *args = (API_SET_USER_NAME_ARGS *)parms;
    NTSTATUS status;
    UNICODE_STRING64 *user_uni;
    WCHAR *user_sid, *user_name;
    ULONG user_sid_len, user_name_len;
    CONF_USER *user, *user1;
    ULONG user_len;
    KIRQL irql;

    //
    // this API must be invoked by the Sandboxie service
    //

    if (proc || (PsGetCurrentProcessId() != Api_ServiceProcessId)) {

        return STATUS_ACCESS_DENIED;
    }

    //
    // probe user target path parameter (sidstring and username)
    //

    user_uni = args->sidstring.val;
    if (! user_uni)
        return STATUS_INVALID_PARAMETER;
    ProbeForRead(user_uni, sizeof(UNICODE_STRING64), sizeof(ULONG64));

    user_sid = (WCHAR *)user_uni->Buffer;
    user_sid_len = user_uni->Length & ~1;
    if ((! user_sid) || (! user_sid_len) || (user_sid_len > 1024))
        return STATUS_INVALID_PARAMETER;
    ProbeForRead(user_sid, user_sid_len, sizeof(WCHAR));

    user_uni = args->username.val;
    if (! user_uni)
        return STATUS_INVALID_PARAMETER;
    ProbeForRead(user_uni, sizeof(UNICODE_STRING64), sizeof(ULONG64));

    user_name = (WCHAR *)user_uni->Buffer;
    user_name_len = user_uni->Length & ~1;
    if ((! user_name) || (! user_name_len) || (user_name_len > 1024))
        return STATUS_INVALID_PARAMETER;
    ProbeForRead(user_name, user_name_len, sizeof(WCHAR));

    //
    // create a CONF_USER element
    //

    user_len = sizeof(CONF_USER) + user_sid_len + user_name_len + 8;
    user = Mem_Alloc(Driver_Pool, user_len);
    if (! user)
        return STATUS_INSUFFICIENT_RESOURCES;

    __try {

        user->len = user_len;

        user->sid = &user->space[0];
        memcpy(user->sid, user_sid, user_sid_len);
        user->sid[user_sid_len / sizeof(WCHAR)] = L'\0';
        user->sid_len = wcslen(user->sid);

        user->name = user->sid + user->sid_len + 1;
        memcpy(user->name, user_name, user_name_len);
        user->name[user_name_len / sizeof(WCHAR)] = L'\0';
        user->name_len = wcslen(user->name);

        while (1) {
            WCHAR *ptr = wcschr(user->name, L'\\');
            if (! ptr)
                ptr = wcschr(user->name, L' ');
            if (! ptr)
                break;
            *ptr = L'_';
        }

        status = STATUS_SUCCESS;

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    if (! NT_SUCCESS(status)) {
        Mem_Free(user, user_len);
        return status;
    }

    //
    // remove an existing CONF_USER element if we find one matching
    // the new CONF_USER element.  then add the new entry
    //

    KeRaiseIrql(APC_LEVEL, &irql);
    ExAcquireResourceExclusiveLite(Conf_Users_Lock, TRUE);

    user1 = List_Head(&Conf_Users);
    while (user1) {

        if (user1->sid_len == user->sid_len &&
                _wcsicmp(user1->sid, user->sid) == 0) {

            List_Remove(&Conf_Users, user1);
            Mem_Free(user1, user1->len);
            break;
        }

        user1 = List_Next(user1);
    }

    List_Insert_After(&Conf_Users, NULL, user);

    ExReleaseResourceLite(Conf_Users_Lock);
    KeLowerIrql(irql);

    KeSetEvent(Conf_Users_Event, 0, FALSE);

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Conf_Api_IsBoxEnabled
//---------------------------------------------------------------------------


_FX NTSTATUS Conf_Api_IsBoxEnabled(PROCESS *proc, ULONG64 *parms)
{
    API_IS_BOX_ENABLED_ARGS *args = (API_IS_BOX_ENABLED_ARGS *)parms;
    NTSTATUS status;
    ULONG SessionId;
    UNICODE_STRING SidString;
    WCHAR boxname[34];

    if (! Api_CopyBoxNameFromUser(boxname, (WCHAR *)args->box_name.val))
        return STATUS_INVALID_PARAMETER;

    status = Process_GetSidStringAndSessionId(
                        NtCurrentProcess(), NULL, &SidString, &SessionId);
    if (NT_SUCCESS(status)) {

        status = Conf_IsValidBox(boxname);
        if (NT_SUCCESS(status)) {

            if (! Conf_IsBoxEnabled(boxname, SidString.Buffer, SessionId))
                status = STATUS_ACCOUNT_RESTRICTION;
        }

        RtlFreeUnicodeString(&SidString);
    }

    return status;
}
