/*
* Copyright 2023 David Xanatos, xanasoft.com
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

#include "stdafx.h"

#include <stdio.h>
#include <Windows.h>
#include <AclAPI.h>
#include <Sddl.h>
#include "common/defines.h"
#include "common/my_version.h"
#include "core/dll/sbieapi.h"
#include <string>
#include <map>

extern "C" void Display_Error(PWSTR SubFuncName, DWORD LastError);

struct SDaclEntry
{
    SDaclEntry() : pSid(NULL), AllowMask(0),DenyMask(0)  {}
    BYTE bSid[68];
    PSID pSid;
    ACCESS_MASK AllowMask;
    ACCESS_MASK DenyMask;
};

std::map<std::wstring, SDaclEntry> ListFolderDACLs(const wchar_t* folderPath)
{
    std::map<std::wstring, SDaclEntry> map;

    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    DWORD result = GetNamedSecurityInfoW(folderPath, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, NULL, NULL, &pSecurityDescriptor);
    if (result != ERROR_SUCCESS) {
        Display_Error(L"GetNamedSecurityInfoW", result);
        return map;
    }

    BOOL ok;
    PACL pDacl = NULL;
    BOOL bDaclPresent = FALSE;
    BOOL bDaclDefaulted = FALSE;
    ok = GetSecurityDescriptorDacl(pSecurityDescriptor, &bDaclPresent, &pDacl, &bDaclDefaulted);
    if (!ok) {
        Display_Error(L"GetSecurityDescriptorDacl", 0);
        return map;
    }
    if (!bDaclPresent)
        return map; // empty not an error

    for (DWORD i = 0; i < pDacl->AceCount; ++i) {

        PACE_HEADER pAceHeader = NULL;
        if (!GetAce(pDacl, i, (LPVOID*)&pAceHeader)) {
            //Display_Error(L"GetAce", 0);
            continue;
        }

        PSID pSid = NULL;
        ACCESS_MASK AllowMask = 0;
        ACCESS_MASK DenyMask = 0;
        switch (pAceHeader->AceType) {
            case ACCESS_ALLOWED_ACE_TYPE: {
                PACCESS_ALLOWED_ACE pAce = (PACCESS_ALLOWED_ACE)pAceHeader;
                pSid = (PSID)&pAce->SidStart;
                AllowMask = pAce->Mask;
                break;
            }
            case ACCESS_DENIED_ACE_TYPE: {
                PACCESS_DENIED_ACE pAce = (PACCESS_DENIED_ACE)pAceHeader;
                pSid = (PSID)&pAce->SidStart;
                DenyMask = pAce->Mask;
                break;
            }
        }

        if (pSid) {
            LPWSTR pSidString = NULL;
            if (ConvertSidToStringSidW(pSid, &pSidString)) {
                SDaclEntry& entry = map[pSidString];
                if (!entry.pSid) {
                    CopySid(sizeof(entry.bSid), entry.bSid, pSid);
                    entry.pSid = entry.bSid;
                }
                entry.AllowMask |= AllowMask;
                entry.DenyMask |= DenyMask;
                LocalFree(pSidString);
            }
        }
    }

    LocalFree(pSecurityDescriptor);

    return map;
}

BOOL UpdateFolderDACLs(const wchar_t* folderPath, EXPLICIT_ACCESS *ea)
{
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    DWORD result = GetNamedSecurityInfoW(folderPath, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, NULL, NULL, &pSecurityDescriptor);
    if (result != ERROR_SUCCESS) {
        Display_Error(L"GetNamedSecurityInfoW", result);
        return FALSE;
    }

    BOOL ok;
    PACL pDacl = NULL;
    BOOL bDaclPresent = FALSE;
    BOOL bDaclDefaulted = FALSE;
    ok = GetSecurityDescriptorDacl(pSecurityDescriptor, &bDaclPresent, &pDacl, &bDaclDefaulted);
    if (!ok) {
        Display_Error(L"GetSecurityDescriptorDacl", 0);
        return FALSE;
    }
    if (!bDaclPresent) {
        Display_Error(L"GetSecurityDescriptorDacl", ERROR_INVALID_ACCESS);
        return FALSE;
    }

    result = SetEntriesInAclW(1, ea, pDacl, &pDacl);
    if (result != ERROR_SUCCESS)
        Display_Error(L"SetEntriesInAclW", result);
    else {
        result = SetNamedSecurityInfoW((LPWSTR)folderPath, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, pDacl, NULL);
        if (result != ERROR_SUCCESS)
            Display_Error(L"SetNamedSecurityInfoW", result);
    }

    LocalFree(pSecurityDescriptor);
    return result == ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// Kmd_FixDacls
//---------------------------------------------------------------------------

extern "C" BOOL Kmd_FixDacls()
{
    WCHAR HomePath[MAX_PATH];
    SbieApi_GetHomePath(NULL, 0, HomePath, MAX_PATH);
    if (!*HomePath) // sbie not installed or not running
        return FALSE;

    //
    // remove problematic permissions created when the
    // win 11 shell extension was registered 
    // for a folder not being under program files
    //

    std::map<std::wstring, SDaclEntry> map = ListFolderDACLs(HomePath);

    for (auto I = map.begin(); I != map.end(); ++I) {

        if (I->first.length() > 44 && (
            (I->first.substr(0, 13) == L"S-1-15-3-1024")
         || (I->first.substr(0, 8) == L"S-1-15-2") )) {

            EXPLICIT_ACCESS ea_clear =
            {
                GENERIC_ALL,
                REVOKE_ACCESS,
                SUB_CONTAINERS_AND_OBJECTS_INHERIT,
                {
                    NULL,
                    NO_MULTIPLE_TRUSTEE,
                    TRUSTEE_IS_SID,
                    TRUSTEE_IS_GROUP,
                    reinterpret_cast<LPTSTR>(I->second.pSid)
                }
            };

            UpdateFolderDACLs(HomePath, &ea_clear);
        }
    }

    //
    // add read access for ALL_APP_PACKAGES
    //

    PSID pSid = NULL; // ALL_APP_PACKAGES
    ConvertStringSidToSidW(L"S-1-15-2-1", &pSid);

    EXPLICIT_ACCESS ea_set =
    {
        GENERIC_READ | GENERIC_EXECUTE,
        SET_ACCESS,
        SUB_CONTAINERS_AND_OBJECTS_INHERIT,
        {
            NULL, 
            NO_MULTIPLE_TRUSTEE,
            TRUSTEE_IS_SID,
            TRUSTEE_IS_GROUP,
            reinterpret_cast<LPTSTR>(pSid)
        }
    };

    UpdateFolderDACLs(HomePath, &ea_set);

    LocalFree(pSid);

    return TRUE;
}
