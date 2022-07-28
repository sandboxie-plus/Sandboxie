/*
 * Copyright 2022 David Xanatos, xanasoft.com
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
// Driver Assistant
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "DriverAssist.h"
#include <ntsecapi.h>
#include "common/lsalookupi_ddk.h"
#include "common/my_Version.h"

extern "C" {

    NTSTATUS WINAPI LsaManageSidNameMapping(
        _In_  LSA_SID_NAME_MAPPING_OPERATION_TYPE    OpType,
        _In_  PLSA_SID_NAME_MAPPING_OPERATION_INPUT  OpInput,
        _Out_ PLSA_SID_NAME_MAPPING_OPERATION_OUTPUT* OpOutput
    );

    NTSTATUS NTAPI RtlCreateVirtualAccountSid(
        _In_ PUNICODE_STRING  	Name,
        _In_ ULONG  	        BaseSubAuthority,
        _Out_ PSID  	        Sid,
        _Inout_ PULONG  	    SidLength
    );

    __declspec(dllimport) NTSTATUS __stdcall RtlInitUnicodeString(
        PUNICODE_STRING DestinationString,
        const WCHAR* SourceString
    );

    #define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
}

NTSTATUS AddSidName(PSID pSID, const WCHAR* domain, const WCHAR* user)
{
    NTSTATUS status;
    LSA_SID_NAME_MAPPING_OPERATION_ADD_INPUT add_input;
    memset(&add_input, 0, sizeof(LSA_SID_NAME_MAPPING_OPERATION_ADD_INPUT));
    PLSA_SID_NAME_MAPPING_OPERATION_INPUT input = (PLSA_SID_NAME_MAPPING_OPERATION_INPUT)&add_input;
    PLSA_SID_NAME_MAPPING_OPERATION_OUTPUT output = NULL;

    add_input.Sid = pSID;
    add_input.Flags = 0;
    RtlInitUnicodeString(&add_input.DomainName, domain);
    if(user != NULL)
        RtlInitUnicodeString(&add_input.AccountName, user);

    status = LsaManageSidNameMapping(LsaSidNameMappingOperation_Add, input, &output);

    if(output) 
        LsaFreeMemory(output);

    return status;
}

NTSTATUS RemoveSidName(const WCHAR* domain, const WCHAR* user)
{
    NTSTATUS status;
    _LSA_SID_NAME_MAPPING_OPERATION_REMOVE_INPUT add_input;
    memset(&add_input, 0, sizeof(_LSA_SID_NAME_MAPPING_OPERATION_REMOVE_INPUT));
    PLSA_SID_NAME_MAPPING_OPERATION_INPUT input = (PLSA_SID_NAME_MAPPING_OPERATION_INPUT)&add_input;
    PLSA_SID_NAME_MAPPING_OPERATION_OUTPUT output = NULL;

    RtlInitUnicodeString(&add_input.DomainName, domain);
    if(user != NULL)
        RtlInitUnicodeString(&add_input.AccountName, user);

    status = LsaManageSidNameMapping(LsaSidNameMappingOperation_Remove, input, &output);

    if(output) 
        LsaFreeMemory(output);

    return status;
}

#define SBIE_RID 100 // must be between 80 and 111 inclusive

UCHAR SandboxieSid[12] = { // S-1-5-100-
    1,                                      // Revision
    1,                                      // SubAuthorityCount
    0,0,0,0,0,5, // SECURITY_NT_AUTHORITY   // IdentifierAuthority
    SBIE_RID,0,0,0                          // SubAuthority
};


//---------------------------------------------------------------------------
// GetSandboxieSID
//---------------------------------------------------------------------------


bool DriverAssist::GetSandboxieSID(const WCHAR* boxname, UCHAR* pSID, DWORD dwSidSize)
{
    if (!SbieApi_QueryConfBool(boxname, L"SandboxieLogon", FALSE))
        return false;

	WCHAR szUserName[256], szDomainName[256];
	DWORD dwDomainSize = ARRAYSIZE(szDomainName);
    SID_NAME_USE snu = SidTypeInvalid;

    wcscpy(szUserName, SANDBOXIE L"\\");
    wcscat(szUserName, boxname);

    if (LookupAccountName(NULL, szUserName, pSID, &dwSidSize, szDomainName, &dwDomainSize, &snu))
        return true;

    //
    // add Sandboxie domain "Sandboxie"
    //

    static bool SbieAdded = false;
    if (!SbieAdded) {
        AddSidName(SandboxieSid, SANDBOXIE, NULL);
        SbieAdded = true;
    }

    //
    // add Sandboxie box user "Sandboxie\\DefaultBox"
    //

    UNICODE_STRING Name;
    RtlInitUnicodeString(&Name, boxname);
    RtlCreateVirtualAccountSid(&Name, SBIE_RID, pSID, &dwSidSize);

    return NT_SUCCESS(AddSidName(pSID, SANDBOXIE, boxname));
}


//---------------------------------------------------------------------------
// CleanUpSIDs
//---------------------------------------------------------------------------


void DriverAssist::CleanUpSIDs()
{
    RemoveSidName(SANDBOXIE, NULL);
}