/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC
 * Copyright 2020 David Xanatos, xanasoft.com
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
// Ipc_Filter_Lsa_Ep_Msg
//---------------------------------------------------------------------------


_FX BOOLEAN Ipc_Filter_Lsa_Ep_Msg(UCHAR uMsg)
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

    if (Session_MonitorCount) {

        WCHAR access_str[24];
        swprintf(access_str, L" Msg: %02X", (ULONG)uMsg);
        const WCHAR* strings[3] = { L"\\RPC Control\\LSARPC_ENDPOINT", access_str, NULL };
        Session_MonitorPutEx(MONITOR_IPC | (filter ? MONITOR_DENY : MONITOR_OPEN), strings, NULL, PsGetCurrentProcessId());
    }

    return filter;
}

