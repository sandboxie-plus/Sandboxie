/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2024 David Xanatos, xanasoft.com
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

#include "advapi.h"


//---------------------------------------------------------------------------
// Scm_DllHack
//---------------------------------------------------------------------------


_FX BOOLEAN Scm_DllHack(HMODULE module, const WCHAR *svcname)
{
    ULONG state;
    SC_HANDLE hService;
    SERVICE_QUERY_RPL *rpl;

    //
    // hack:  make sure the given service is running
    //

    if (! module)
        return TRUE;

    if (Scm_IsBoxedService(svcname))
        return TRUE;

    rpl = (SERVICE_QUERY_RPL *)Scm_QueryServiceByName(svcname, TRUE, 0);
    if (! rpl)
        return TRUE;

    state = rpl->service_status.dwCurrentState;
    Dll_Free(rpl);
    if (state != SERVICE_STOPPED)
        return TRUE;

    hService = Scm_OpenServiceWImpl(
                    HANDLE_SERVICE_MANAGER, svcname, SERVICE_START);
    if (hService) {

        if (Scm_StartServiceWImpl(hService, 0, NULL))
            Sleep(500);

        Scm_CloseServiceHandleImpl(hService);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Scm_OsppcDll
//---------------------------------------------------------------------------


_FX BOOLEAN Scm_OsppcDll(HMODULE module)
{
    Custom_OsppcDll(module);

    //
    // hack for Office 2010 osppc.dll:  make sure osppsvc service is running
    //

    return Scm_DllHack(module, L"osppsvc");
}


//---------------------------------------------------------------------------
// Scm_DWriteDll
//---------------------------------------------------------------------------


_FX BOOLEAN Scm_DWriteDll(HMODULE module)
{
    //
    // hack for IE 9 DWrite.dll:  make sure FontCache service is running
    //

    return Scm_DllHack(module, L"FontCache");
}


//---------------------------------------------------------------------------
// Scm_Start_Sppsvc
//---------------------------------------------------------------------------


_FX int Scm_Start_Sppsvc()
{
    SC_HANDLE handle1 = Scm_OpenSCManagerW(NULL, NULL, GENERIC_READ);
    SC_HANDLE handle2 = NULL;
    int rc = 0;

    if (handle1) {
        SC_HANDLE handle2 = Scm_OpenServiceWImpl(handle1, L"sppsvc", SERVICE_START);
        if (handle2) {
            SERVICE_STATUS lpServiceStatus;
            int count = 0;
            lpServiceStatus.dwCurrentState = 0;
            Scm_StartServiceWImpl(handle2, 0, NULL);

            while (lpServiceStatus.dwCurrentState != SERVICE_RUNNING && count++ < 10) {
                Sleep(50);
                Scm_QueryServiceStatusImpl(handle2, &lpServiceStatus);
                if (lpServiceStatus.dwCurrentState == 4) {
                    rc = 1;
                }
            }
        }
    }

    if (handle1)
        Scm_CloseServiceHandleImpl(handle1);
    if (handle2)
        Scm_CloseServiceHandleImpl(handle2);
    return rc;
}
