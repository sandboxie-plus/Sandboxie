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
// Defines
//---------------------------------------------------------------------------

//
// \RPC Control\samss lpc is an LRPC (ncalrpc) endpoint of lsass.exe, and
// lsass serves more interfaces than SAMR on it, e.g. CNG Key Isolation.
// a request carries the binding id lsass assigned to its interface in the
// reply to the bind message, and the procedure number.  the layouts below
// are only relied upon on Windows 10 and later
//

#define LRPC_MSG_REQUEST                0
#define LRPC_MSG_BIND                   1

#define LRPC_REQUEST_BINDING_ID_OFS     16  // ULONG
#define LRPC_REQUEST_MIN_LEN            24  // ProcNum is the ULONG at 20

#define LRPC_BIND_STATUS_OFS            8   // ULONG, RPC status in the reply
#define LRPC_BIND_INTERFACE_OFS         12  // RPC_SYNTAX_IDENTIFIER
#define LRPC_BIND_SYNTAX_SET_OFS        32  // ULONG, 1 DCE NDR, 2 NDR64, 4 fake NDR64
#define LRPC_BIND_SYNTAX_IDS_OFS        36  // USHORT[3], binding id for each of them
#define LRPC_BIND_MIN_LEN               42


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


// {12345778-1234-ABCD-EF00-0123456789AC}
static const GUID Ipc_Sam_SamrInterface = {
    0x12345778, 0x1234, 0xABCD, { 0xEF, 0x00, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAC } };


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

_FX BOOLEAN Ipc_Filter_Sam_Msg(PROCESS* proc, UCHAR uMsg, ULONG BindingId);

static BOOLEAN Ipc_Sam_IsOtherBinding(
    PROCESS* proc, void* PortObject, ULONG BindingId);

//---------------------------------------------------------------------------
// Ipc_CheckPortRequest_Sam
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_CheckPortRequest_Sam(
    PROCESS* proc, OBJECT_NAME_INFORMATION* Name, PORT_MESSAGE* msg,
    void* PortObject, void** OtherBindPort)
{
    NTSTATUS status;
    ULONG msg_type = (ULONG)-1;
    ULONG binding_id = (ULONG)-1;
    BOOLEAN other_bind = FALSE;
    UCHAR uMsg = 0;

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

            ProbeForRead(ptr, len, sizeof(WCHAR));

            uMsg = ptr[20];

            if (Driver_OsVersion >= DRIVER_WINDOWS_10 && len >= LRPC_REQUEST_MIN_LEN) {

                msg_type = *(ULONG*)ptr;

                if (msg_type == LRPC_MSG_REQUEST)
                    binding_id = *(ULONG*)(ptr + LRPC_REQUEST_BINDING_ID_OFS);

                else if (msg_type == LRPC_MSG_BIND && len >= LRPC_BIND_MIN_LEN) {

                    other_bind = memcmp(ptr + LRPC_BIND_INTERFACE_OFS,
                        &Ipc_Sam_SamrInterface, sizeof(GUID)) != 0;
                }
            }

            //DbgPrint("\\RPC Control\\samss lpc message ID: %d\n", (int)ptr[20]);
        }

    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    if (NT_SUCCESS(status) && Driver_OsVersion >= DRIVER_WINDOWS_7) {

        //
        // a bind of another interface than SAMR carries no SAM operation,
        // and the caller records the binding ids from the reply, so that
        // the procedure numbers of that interface are not mistaken for SAM
        // operations, e.g. CNG Key Isolation's 2 and 10 (issue #5727).
        // requests on any other binding are filtered as before
        //

        if (other_bind) {

            if (OtherBindPort) {
                ObReferenceObject(PortObject);
                *OtherBindPort = PortObject;
            }

        } else if (msg_type == LRPC_MSG_REQUEST
                        && Ipc_Sam_IsOtherBinding(proc, PortObject, binding_id)) {

            ; // not a SAM operation

        } else if (Ipc_Filter_Sam_Msg(proc, uMsg, binding_id))
            status = STATUS_ACCESS_DENIED;
    }

    return status;
}


//---------------------------------------------------------------------------
// Ipc_Sam_IsOtherBinding
//---------------------------------------------------------------------------


_FX BOOLEAN Ipc_Sam_IsOtherBinding(
    PROCESS* proc, void* PortObject, ULONG BindingId)
{
    BOOLEAN found = FALSE;
    KIRQL irql;
    ULONG i;

    if (BindingId > 0xFFFF)
        return FALSE;

    KeRaiseIrql(APC_LEVEL, &irql);
    ExAcquireResourceSharedLite(proc->ipc_lock, TRUE);

    for (i = 0; i < IPC_SAM_BINDINGS_MAX && proc->ipc_sam_port[i]; i++) {

        if (proc->ipc_sam_port[i] == PortObject
                && proc->ipc_sam_binding[i] == (USHORT)BindingId) {
            found = TRUE;
            break;
        }
    }

    ExReleaseResourceLite(proc->ipc_lock);
    KeLowerIrql(irql);

    if (found && Session_MonitorCount && (proc->ipc_trace & TRACE_ALLOW)) {

        WCHAR msg_str[48];
        RtlStringCbPrintfW(msg_str, sizeof(msg_str), L"Binding: %u, not SAMR", BindingId);
        Log_Debug_Msg(MONITOR_IPC | MONITOR_OPEN, msg_str, L"\\RPC Control\\samss lpc");
    }

    return found;
}


//---------------------------------------------------------------------------
// Ipc_Sam_RecordBindReply
//---------------------------------------------------------------------------


_FX void Ipc_Sam_RecordBindReply(
    PROCESS* proc, void* PortObject, PORT_MESSAGE* ReplyMsg)
{
    ULONG rpc_status = (ULONG)-1;
    ULONG syntax_set = 0;
    USHORT ids[3] = { 0, 0, 0 };
    ULONG count = 0;
    ULONG len;
    UCHAR* ptr;
    ULONG i, j;
    KIRQL irql;

    //
    // read the binding ids lsass assigned to the interface from its reply
    // to the bind message, see Ipc_AlpcSendWaitReceivePort
    //

    __try {

        ProbeForRead(ReplyMsg, sizeof(PORT_MESSAGE), sizeof(ULONG_PTR));

        len = ReplyMsg->u1.s1.DataLength;
        ptr = (UCHAR*)((UCHAR*)ReplyMsg + sizeof(PORT_MESSAGE));

        if (len >= LRPC_BIND_MIN_LEN) {

            ProbeForRead(ptr, LRPC_BIND_MIN_LEN, sizeof(USHORT));

            if (*(ULONG*)ptr == LRPC_MSG_BIND
                    && memcmp(ptr + LRPC_BIND_INTERFACE_OFS,
                            &Ipc_Sam_SamrInterface, sizeof(GUID)) != 0) {

                rpc_status = *(ULONG*)(ptr + LRPC_BIND_STATUS_OFS);
                syntax_set = *(ULONG*)(ptr + LRPC_BIND_SYNTAX_SET_OFS);

                for (i = 0; i < 3; i++) {
                    if (rpc_status == 0 && (syntax_set & (1 << i)))
                        ids[count++] = *(USHORT*)(ptr + LRPC_BIND_SYNTAX_IDS_OFS + i * sizeof(USHORT));
                }
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        count = 0;
    }

    if (count) {

        KeRaiseIrql(APC_LEVEL, &irql);
        ExAcquireResourceExclusiveLite(proc->ipc_lock, TRUE);

        for (j = 0; j < count; j++) {

            for (i = 0; i < IPC_SAM_BINDINGS_MAX && proc->ipc_sam_port[i]; i++) {
                if (proc->ipc_sam_port[i] == PortObject
                        && proc->ipc_sam_binding[i] == ids[j])
                    break;
            }

            //
            // when the table is full the binding is not added, and its
            // requests stay filtered as SAM requests
            //

            if (i < IPC_SAM_BINDINGS_MAX && ! proc->ipc_sam_port[i]) {
                ObReferenceObject(PortObject);
                proc->ipc_sam_binding[i] = ids[j];
                proc->ipc_sam_port[i] = PortObject;
            }
        }

        ExReleaseResourceLite(proc->ipc_lock);
        KeLowerIrql(irql);
    }

    if (Session_MonitorCount && (proc->ipc_trace & TRACE_ALLOW)) {

        WCHAR msg_str[96];
        RtlStringCbPrintfW(msg_str, sizeof(msg_str),
            L"Bind reply: status %X, syntax %X, ids %u %u %u, recorded %u",
            rpc_status, syntax_set, ids[0], ids[1], ids[2], count);
        Log_Debug_Msg(MONITOR_IPC | MONITOR_OPEN, msg_str, L"\\RPC Control\\samss lpc");
    }
}


//---------------------------------------------------------------------------
// Ipc_Sam_ReleaseProcess
//---------------------------------------------------------------------------


_FX void Ipc_Sam_ReleaseProcess(PROCESS* proc)
{
    ULONG i;

    for (i = 0; i < IPC_SAM_BINDINGS_MAX && proc->ipc_sam_port[i]; i++) {
        ObDereferenceObject(proc->ipc_sam_port[i]);
        proc->ipc_sam_port[i] = NULL;
    }
}


//---------------------------------------------------------------------------
// Ipc_Filter_Sam_Msg
//---------------------------------------------------------------------------


_FX BOOLEAN Ipc_Filter_Sam_Msg(PROCESS* proc, UCHAR uMsg, ULONG BindingId)
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
            WCHAR msg_str[48];
            if (BindingId != (ULONG)-1)
                RtlStringCbPrintfW(msg_str, sizeof(msg_str), L"Msg: %02X, Binding: %u", (ULONG)uMsg, BindingId);
            else
                RtlStringCbPrintfW(msg_str, sizeof(msg_str), L"Msg: %02X", (ULONG)uMsg);
            Log_Debug_Msg(mon_type, msg_str, L"\\RPC Control\\samss lpc");
        }
    }

    return filter;
}
