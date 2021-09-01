/*
 * Copyright 2021 David Xanatos, xanasoft.com
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
 // Inter-Process Communication
 //---------------------------------------------------------------------------


#include "ipc.h"
#include "obj.h"
#include "api.h"
#include "util.h"
#include "session.h"

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

_FX BOOLEAN Ipc_Filter_Sam_Msg(PROCESS* proc, UCHAR uMsg);

//---------------------------------------------------------------------------
// Ipc_CheckPortRequest_Sam
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_CheckPortRequest_Sam(
    PROCESS* proc, OBJECT_NAME_INFORMATION* Name, PORT_MESSAGE* msg)
{
    NTSTATUS status;

    if (proc->ipc_open_sam_endpoint)
        return STATUS_BAD_INITIAL_PC;

    if (Name->Name.Length == 22 * sizeof(WCHAR)) {

        if (_wcsicmp(Name->Name.Buffer, L"\\RPC Control\\samss lpc") != 0)
            return STATUS_BAD_INITIAL_PC;

    }
    else
        return STATUS_BAD_INITIAL_PC;

    //
    // examine message
    //

    status = STATUS_SUCCESS;

    __try {

        ProbeForRead(msg, sizeof(PORT_MESSAGE), sizeof(ULONG_PTR));

        if (Driver_OsVersion >= DRIVER_WINDOWS_7) {

            ULONG  len = msg->u1.s1.DataLength;
            UCHAR* ptr = (UCHAR*)((UCHAR*)msg + sizeof(PORT_MESSAGE));
            int i = 0;
            int rc = -2;

            ProbeForRead(ptr, len, sizeof(WCHAR));

            if (Ipc_Filter_Sam_Msg(proc, ptr[20]))
                status = STATUS_ACCESS_DENIED;

            //DbgPrint("\\RPC Control\\samss lpc message ID: %d\n", (int)ptr[20]);
        }

    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    return status;
}


//---------------------------------------------------------------------------
// Ipc_Filter_Sam_Msg
//---------------------------------------------------------------------------


_FX BOOLEAN Ipc_Filter_Sam_Msg(PROCESS* proc, UCHAR uMsg)
{
    BOOLEAN filter = FALSE;

    switch (uMsg)
    {
    //case 0x00: //SamConnect
    //case 0x01: //SamCloseHandle
    case 0x02: //SamSetSecurityObject // fixme: SandboxieCrypto.exe needs this some times #740 //if(proc->image_sbie) break;
    //case 0x03: //SamQuerySecurityObject
    //case 0x05: //SamLookupDomainInSamServer
    //case 0x06: //SamEnumerateDomainsInSamServer
    //case 0x07: //SamOpenDomain
    //case 0x08: //SamQueryInformationDomain
    case 0x09: //SamSetInformationDomain
    case 0x0A: //SamCreateGroupInDomain
    //case 0x0B: //SamEnumerateGroupsInDomain
    case 0x0C: //SamCreateUserInDomain
    //case 0x0D: //SamiEnumerateUsersInDomain
    case 0x0E: //SamCreateAliasInDomain
    //case 0x0F: //SamEnumerateAliasesInDomain
    //case 0x10: //SamGetAliasMembership
    //case 0x11: //SamiLookupNamesInDomain
    //case 0x12: //SampLookupIdsInDomain
    //case 0x13: //SamOpenGroup
    //case 0x14: //SamQueryInformationGroup
    case 0x15: //SamSetInformationGroup
    case 0x17: //SamDeleteGroup
    case 0x18: //SamRemoveMemberFromGroup
    //case 0x19: //SamGetMembersInGroup
    //case 0x1A: //SamSetMemberAttributesOfGroup
    //case 0x1B: //SamOpenAlias
    //case 0x1C: //SamQueryInformationAlias
    case 0x1D: //SamSetInformationAlias
    case 0x1E: //SamDeleteAlias
    case 0x1F: //SamAddMemberToAlias
    case 0x20: //SamRemoveMemberFromAlias
    //case 0x21: //SamGetMembersInAlias
    //case 0x22: //SamOpenUser
    case 0x23: //SamDeleteUser
    //case 0x24: //SamQueryInformationUser
    case 0x25: //SamSetInformationUser
    case 0x26: //SamiLmChangePasswordUser, SamiLmChangePasswordUser
    //case 0x27: //SamGetGroupsForUser
    //case 0x28: //SamQueryDisplayInformation
    //case 0x29: //SamGetDisplayEnumerationIndex
    //case 0x2C: //SampCheckPasswordRestrictions
    case 0x2D: //SamRemoveMemberFromForeignDomain
    //case 0x2E: //SamQueryInformationDomain
    //case 0x30: //SamQueryDisplayInformation
    //case 0x31: //SamGetDisplayEnumerationIndex
    case 0x32: //SamCreateUser2InDomain
    //case 0x33: //SamQueryDisplayInformation
    case 0x35: //SamRemoveMultipleMembersFromAlias
    case 0x37: //SamiChangePasswordUser2
    case 0x38: //SamiChangePasswordUser2
    //case 0x39: //SamConnect
    case 0x3A: //SampSetInfoUserUseOldInfoClass
    case 0x3B: //SamiSetBootKeyInformation
    //case 0x3E: //SamConnect
    //case 0x40: //SamConnect
    //case 0x41: //SamRidToSid
    case 0x42: //SampSetDSRMPassword
    //case 0x43: //SamValidatePassword
    //case 0x44: //SamQueryLocalizableAccountsInDomain
    //case 0x45: //SamPerformGenericOperation
    case 0x46: //SamiSyncDSRMPasswordFromAccount
    //case 0x47: //SamiLookupNamesInDomain
    //case 0x48: //SamiEnumerateUsersInDomain

    //case 0xEF:    //Unknown
        filter = TRUE;
    }

    if (Session_MonitorCount && (proc->ipc_trace & (TRACE_ALLOW | TRACE_DENY))) {

        ULONG mon_type = MONITOR_IPC;

        if (filter && (proc->ipc_trace & TRACE_DENY))
            mon_type |= MONITOR_DENY;
        else if (!filter && (proc->ipc_trace & TRACE_ALLOW))
            mon_type |= MONITOR_OPEN;
        else
            mon_type = 0;

        if (mon_type) {
            WCHAR msg_str[24];
            RtlStringCbPrintfW(msg_str, sizeof(msg_str), L"Msg: %02X", (ULONG)uMsg);
            Log_Debug_Msg(mon_type, L"\\RPC Control\\samss lpc", msg_str);
        }
    }

    return filter;
}
