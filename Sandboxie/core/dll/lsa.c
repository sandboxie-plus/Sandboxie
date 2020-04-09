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
// Local Security Authority
//---------------------------------------------------------------------------


#include "dll.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN Lsa_Init_Common(const WCHAR *DllName);

static NTSTATUS Lsa_LsaRegisterLogonProcess(
    void *LogonProcessName, HANDLE *LsaHandle, void *SecurityMode);


//---------------------------------------------------------------------------


typedef ULONG (*P_LsaConnectUntrusted)(HANDLE *LsaHandle);

typedef NTSTATUS (*P_LsaRegisterLogonProcess)(
    void *LogonProcessName, HANDLE *LsaHandle, void *SecurityMode);

typedef NTSTATUS (*P_LsaOpenPolicy)(
    void *SystemName, void *ObjectAttributes, ACCESS_MASK DesiredAccess,
    HANDLE *PolicyHandle);


//---------------------------------------------------------------------------


static P_LsaConnectUntrusted        __sys_LsaConnectUntrusted       = NULL;
static P_LsaRegisterLogonProcess    __sys_LsaRegisterLogonProcess   = NULL;


//---------------------------------------------------------------------------
// Lsa_Init_Common
//---------------------------------------------------------------------------


_FX BOOLEAN Lsa_Init_Common(const WCHAR *DllName)
{
    void *LsaRegisterLogonProcess;

    //
    // intercept SECUR32/SSPICLI entry points
    //

    __sys_LsaConnectUntrusted = (P_LsaConnectUntrusted)
        Ldr_GetProcAddrNew(DllName, L"LsaConnectUntrusted","LsaConnectUntrusted");
    LsaRegisterLogonProcess = (P_LsaRegisterLogonProcess)
        Ldr_GetProcAddrNew(DllName, L"LsaRegisterLogonProcess","LsaRegisterLogonProcess");

    SBIEDLL_HOOK(Lsa_,LsaRegisterLogonProcess);

    return TRUE;
}


//---------------------------------------------------------------------------
// Lsa_LsaRegisterLogonProcess
//---------------------------------------------------------------------------


static NTSTATUS Lsa_LsaRegisterLogonProcess(
    void *LogonProcessName, HANDLE *LsaHandle, void *SecurityMode)
{
    NTSTATUS status = __sys_LsaRegisterLogonProcess(
        LogonProcessName, LsaHandle, SecurityMode);
    if (! NT_SUCCESS(status))
        status = __sys_LsaConnectUntrusted(LsaHandle);
    return status;
}


//---------------------------------------------------------------------------
// Lsa_Init_Secur32
//---------------------------------------------------------------------------


_FX BOOLEAN Lsa_Init_Secur32(HMODULE module)
{
    if (Dll_KernelBase) {

        //
        // On Windows 7, LsaRegisterLogonProcess and LsaConnectUntrusted
        // are implemented by SspiCli.dll
        //

        return TRUE;
    }

    return Lsa_Init_Common(DllName_secur32);
}


//---------------------------------------------------------------------------
// Lsa_Init_SspiCli
//---------------------------------------------------------------------------


_FX BOOLEAN Lsa_Init_SspiCli(HMODULE module)
{
    if (! Dll_KernelBase) {

        //
        // On Windows earlier than 7, LsaRegisterLogonProcess and
        // LsaConnectUntrusted are implemented by Secur32.dll
        //

        return TRUE;
    }

    return Lsa_Init_Common(DllName_sspicli);
}
