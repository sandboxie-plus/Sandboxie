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
 // Inter-Process Communication
 //---------------------------------------------------------------------------


#include "ipc.h"
#include "obj.h"
#include "api.h"
#include "util.h"
#include "session.h"



//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _LSA_MESSAGE_XP {

    PORT_MESSAGE port_msg;
    ULONG api_code;
    ULONG status;
    ULONG auth_pkg_code;
    ULONG* buf;
    ULONG buf_len;

} LSA_MESSAGE_XP;


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


#ifndef _WIN64

static ULONG Ipc_MSV10_AuthPackageNumber = 0;

#endif


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

BOOLEAN Ipc_Filter_Lsa_Ep_Msg(PROCESS* proc, UCHAR uMsg);


//---------------------------------------------------------------------------
// Ipc_CheckPortRequest_Lsa
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_CheckPortRequest_Lsa(
    PROCESS* proc, OBJECT_NAME_INFORMATION* Name, PORT_MESSAGE* msg)
{
    NTSTATUS status;

    if (!proc->ipc_block_password)
        return STATUS_BAD_INITIAL_PC;

    //
    // check that it is \LsaAuthenticationPort
    // or that it is \RPC Control\lsasspirpc (Windows 7 variant)
    //

    if (Name->Name.Length == 22 * sizeof(WCHAR)) {

        if (_wcsicmp(Name->Name.Buffer, L"\\LsaAuthenticationPort") != 0)
            return STATUS_BAD_INITIAL_PC;

    }
    else if (Name->Name.Length == 23 * sizeof(WCHAR)) {

        if (_wcsicmp(Name->Name.Buffer, L"\\RPC Control\\lsasspirpc") != 0)
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

        if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA) {

            //
            // in Windows Vista and Windows 7, a password change request
            // includes the WCHAR string Negotiate immediately followed
            // by a non-zero WCHAR
            //

            WCHAR* ptr = (WCHAR*)((UCHAR*)msg + sizeof(PORT_MESSAGE));
            ULONG  len = msg->u1.s1.DataLength;

            ProbeForRead(ptr, len, sizeof(WCHAR));
            len /= sizeof(WCHAR);

            while (len > 9 + 1) {

                if (ptr[0] == L'N' && ptr[9] != 0
                    && wmemcmp(ptr, L"Negotiate", 9) == 0) {

                    status = STATUS_ACCESS_DENIED;
                    break;
                }

                ++ptr;
                --len;
            }

        }
#ifndef _WIN64
        else { // xp support

         //
         // prior to Windows Vista, we have a 'call package' api
         // call (value 2), which identifies the MSV10 auth package,
         // and a change password sub code (value 5)
         //

            if (msg->u1.s1.TotalLength >= sizeof(LSA_MESSAGE_XP)) {

                LSA_MESSAGE_XP* msg2 = (LSA_MESSAGE_XP*)msg;

                ProbeForRead(
                    msg2, sizeof(LSA_MESSAGE_XP), sizeof(ULONG_PTR));

                if (msg2->api_code == 2 &&  // LsaCallAuthenticationPackage
                    msg2->auth_pkg_code == Ipc_MSV10_AuthPackageNumber &&
                    msg2->buf_len >= sizeof(ULONG)) {

                    ULONG* buf = msg2->buf;
                    ProbeForRead(buf, sizeof(ULONG), sizeof(ULONG));

                    if (*buf == 5) {            // change password

                        status = STATUS_ACCESS_DENIED;
                    }
                }
            }
        }
#endif

    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    if (status == STATUS_ACCESS_DENIED)
        Log_Msg_Process(MSG_PASSWORD_CHANGE_DENIED, NULL, NULL, -1, proc->pid);

    return status;
}


//---------------------------------------------------------------------------
// Ipc_CheckPortRequest_LsaEP
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_CheckPortRequest_LsaEP(
    PROCESS* proc, OBJECT_NAME_INFORMATION* Name, PORT_MESSAGE* msg)
{
    NTSTATUS status;

    if (proc->ipc_open_lsa_endpoint)
        return STATUS_BAD_INITIAL_PC;

    if (Name->Name.Length == 28 * sizeof(WCHAR)) {

        if (_wcsicmp(Name->Name.Buffer, L"\\RPC Control\\LSARPC_ENDPOINT") != 0)
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

            if (Ipc_Filter_Lsa_Ep_Msg(proc, ptr[20]))
                status = STATUS_ACCESS_DENIED;

            //DbgPrint("\\RPC Control\\LSARPC_ENDPOINT message ID: %d\n", (int)ptr[20]);
        }

    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    return status;
}


//---------------------------------------------------------------------------
// Ipc_Filter_Lsa_Ep_Msg
//---------------------------------------------------------------------------


_FX BOOLEAN Ipc_Filter_Lsa_Ep_Msg(PROCESS* proc, UCHAR uMsg)
{
    BOOLEAN filter = FALSE;

    switch (uMsg)
    {
    //case 0x00:	//LsaClose_0

    //case 0x02:	//LsaEnumeratePrivileges
    //case 0x03:	//LsaQuerySecurityObject
    case 0x04:		//LsaSetSecurityObject


    //case 0x07:	//LsaQueryInformationPolicy
    case 0x08:		//LsaSetInformationPolicy
    case 0x09:		//LsaClearAuditLog
    case 0x0A:		//LsaCreateAccount
    //case 0x0B:	//LsaEnumerateAccounts
    case 0x0C:		//LsaCreateTrustedDomain
    //case 0x0D:	//LsaEnumerateTrustedDomains
    //case 0x0E:	//LsaICLookupNames
    //case 0x0F:	//LsaICLookupSids_0
    //case 0x10:	//LsaCreateSecret
    //case 0x11:	//LsaOpenAccount
    //case 0x12:	//LsaEnumeratePrivilegesOfAccount
    case 0x13:		//LsaAddPrivilegesToAccount
    case 0x14:		//LsaRemovePrivilegesFromAccount
    //case 0x15:	//LsaGetQuotasForAccount
    case 0x16:		//LsaSetQuotasForAccount
    //case 0x17:	//LsaGetSystemAccessAccount
    case 0x18:		//LsaSetSystemAccessAccount
    //case 0x19:	//LsaOpenTrustedDomain
    //case 0x1A:	//LsaQueryInfoTrustedDomain
    case 0x1B:		//LsaSetInformationTrustedDomain
    //case 0x1C:	//LsaOpenSecret
    //case 0x1D:	//LsaSetSecret_Old
    //case 0x1E:	//LsaQuerySecret_Old
    //case 0x1F:	//LsaLookupPrivilegeValue
    //case 0x20:	//LsaLookupPrivilegeName
    //case 0x21:	//LsaLookupPrivilegeDisplayName
    //case 0x22:	//LsaDelete
    //case 0x23:	//LsaEnumerateAccountsWithUserRight
    //case 0x24:	//LsaEnumerateAccountRights
    case 0x25:		//LsaAddAccountRights
    case 0x26:		//LsaRemoveAccountRights
    //case 0x27:	//LsaQueryTrustedDomainInfo
    case 0x28:		//LsaSetTrustedDomainInformation
    case 0x29:		//LsaDeleteTrustedDomain
    case 0x2A:		//LsaStorePrivateData_Old
    case 0x2B:		//LsaRetrievePrivateData_Old
    //case 0x2C:	//LsaOpenPolicy_0
    //case 0x2D:	//LsaGetUserName
    //case 0x2E:	//LsaQueryInformationPolicy

    //case 0x30:	//LsaQueryTrustedDomainInfoByName
    case 0x31:		//LsaSetTrustedDomainInfoByName
    //case 0x32:	//LsaEnumerateTrustedDomainsEx


    //case 0x35:	//LsaQueryDomainInformationPolicy
    case 0x36:		//LsaSetDomainInformationPolicy
    //case 0x37:	//LsaOpenTrustedDomainByName

    //case 0x39:	//LsaICLookupSids_0
    //case 0x3A:	//LsaICLookupNames
    case 0x3B:		//LsaCreateTrustedDomainEx


    //case 0x44:	//LsaICLookupNames

    //case 0x49:	//LsaQueryForestTrustInformation
    case 0x4A:		//LsaSetForestTrustInformation

    //case 0x4C:	//LsaICLookupSidsWithCreds
    //case 0x4D:	//LsaICLookupNamesWithCreds
    //case 0x4E:	//LsaOpenPolicySce
    case 0x5F:		//LsaManageSidNameMapping


    case 0x67:		//AuditpSetGlobalSacl
    //case 0x68:	//AuditpQueryGlobalSacl


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
            Log_Debug_Msg(mon_type, L"\\RPC Control\\LSARPC_ENDPOINT", msg_str);
        }
    }

    return filter;
}

#ifndef _WIN64


//---------------------------------------------------------------------------
// Ipc_Api_SetLsaAuthPkg
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_Api_SetLsaAuthPkg(PROCESS* proc, ULONG64* parms) // xp support
{
    //
    // caller must be our service process
    //

    if (proc || (PsGetCurrentProcessId() != Api_ServiceProcessId))
        return STATUS_ACCESS_DENIED;

    //
    // collect msv10 auth package number
    //

    if (Ipc_MSV10_AuthPackageNumber)
        return STATUS_ACCESS_DENIED;

    Ipc_MSV10_AuthPackageNumber = (ULONG)parms[1];
    return STATUS_SUCCESS;
}

#endif