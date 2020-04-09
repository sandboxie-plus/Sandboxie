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
// Handles PDH (Deny access to performance counters
//---------------------------------------------------------------------------

#include "dll.h"

static _FX UINT Pdh_PdhConnectMachineW(LPCWSTR lpwsMachine);

static _FX UINT Pdh_PdhLookupPerfNameByIndexW(
    LPCWSTR szMachineName,
    DWORD dwNameIndex,
    LPTSTR szNameBuffer,
    LPDWORD pcchNameBufferSize
);

typedef UINT(*P_PdhConnectMachineW)(
    LPCWSTR lpwsMachine);

typedef UINT(*P_PdhLookupPerfNameByIndexW)(
    LPCWSTR szMachineName,
    DWORD dwNameIndex,
    LPTSTR szNameBuffer,
    LPDWORD pcchNameBufferSize);

static P_PdhConnectMachineW           __sys_PdhConnectMachineW = NULL;
static P_PdhLookupPerfNameByIndexW    __sys_PdhLookupPerfNameByIndexW = NULL;


//---------------------------------------------------------------------------
// Pdh_Init
//---------------------------------------------------------------------------

_FX BOOLEAN Pdh_Init(HMODULE hDll)
{
    void * PdhConnectMachineW;
    void * PdhLookupPerfNameByIndexW;

    PdhConnectMachineW = (void*)GetProcAddress(hDll, "PdhConnectMachineW");
    if (PdhConnectMachineW == NULL)
        return FALSE;

    PdhLookupPerfNameByIndexW = (void*)GetProcAddress(hDll, "PdhLookupPerfNameByIndexW");
    if (PdhLookupPerfNameByIndexW == NULL)
        return FALSE;

    SBIEDLL_HOOK(Pdh_, PdhConnectMachineW);
    SBIEDLL_HOOK(Pdh_, PdhLookupPerfNameByIndexW);
    return TRUE;
}

//---------------------------------------------------------------------------
// Pdh_PdhConnectMachineW
//---------------------------------------------------------------------------

static _FX UINT Pdh_PdhConnectMachineW(LPCWSTR lpwsMachine)
{
    return ERROR_ACCESS_DENIED;
}

//---------------------------------------------------------------------------
// Pdh_PdhLookupPerfNameByIndexW
//---------------------------------------------------------------------------

static _FX UINT Pdh_PdhLookupPerfNameByIndexW(
    LPCWSTR szMachineName,
    DWORD dwNameIndex,
    LPTSTR szNameBuffer,
    LPDWORD pcchNameBufferSize
)
{
    return ERROR_ACCESS_DENIED;
}
