/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2021-2023 David Xanatos, xanasoft.com
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
// Protected Storage
//---------------------------------------------------------------------------

extern "C" {
#include "dll.h"
}

#include <windows.h>
#include "ipstore_impl.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


typedef HRESULT (*P_PStoreCreateInstance)(
    IPStore **ppProvider, PST_PROVIDERID *pProviderID,
    void *pReserved, DWORD dwFlags);

static P_PStoreCreateInstance __sys_PStoreCreateInstance = NULL;

static void *__sys_CoTaskMemAlloc = NULL;


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static const char *Pst_PStoreCreateInstanceProcName = "PStoreCreateInstance";


//---------------------------------------------------------------------------
// PStoreCreateInstance
//---------------------------------------------------------------------------


_FX HRESULT Pst_PStoreCreateInstance(
    IPStore **ppProvider,
    PST_PROVIDERID *pProviderID,
    void *pReserved,
    DWORD dwFlags)
{
    if (! __sys_CoTaskMemAlloc) {
        HMODULE ole32 = GetModuleHandle(DllName_ole32_or_combase);
        __sys_CoTaskMemAlloc = GetProcAddress(ole32, "CoTaskMemAlloc");
        if (! __sys_CoTaskMemAlloc)
            return E_FAIL;
    }
    *ppProvider = new IPStoreImpl(__sys_CoTaskMemAlloc);
    return S_OK;
}


//---------------------------------------------------------------------------
// Pst_Init
//---------------------------------------------------------------------------


extern "C" _FX BOOLEAN Pst_Init(HMODULE module)
{
    P_PStoreCreateInstance PStoreCreateInstance;

    //
    // if OpenProtectedStorage is specified, don't hook anything
    //

    ULONG mp_flags = SbieDll_MatchPath(L'i', L"\\RPC Control\\protected_storage");
    if (PATH_IS_OPEN(mp_flags))
        return TRUE;

    //
    // in app mode we don't need these hooks as we have a full token
    //

    if (Dll_CompartmentMode)
        return TRUE;

    //
    // hook Protected Storage entry point
    //

    PStoreCreateInstance = (P_PStoreCreateInstance)
        GetProcAddress(module, Pst_PStoreCreateInstanceProcName);

    SBIEDLL_HOOK(Pst_,PStoreCreateInstance)

    return TRUE;
}


//---------------------------------------------------------------------------
// SbieDll_InitPStore
//---------------------------------------------------------------------------


extern "C" void *SbieDll_InitPStore(void)
{
    static const WCHAR *_pstorec = L"pstorec.dll";
    void *pst = NULL;

    HMODULE module = GetModuleHandle(_pstorec);
    if (! module)
        module = LoadLibrary(_pstorec);
    if (module) {

        P_PStoreCreateInstance pPStoreCreateInstance =
            (P_PStoreCreateInstance)
                GetProcAddress(module, Pst_PStoreCreateInstanceProcName);
        if (pPStoreCreateInstance) {

            HRESULT hr = pPStoreCreateInstance(
                                (IPStore **)&pst, NULL, NULL, 0);
            if (hr != S_OK)
                pst = NULL;
        }
    }

    return pst;
}
