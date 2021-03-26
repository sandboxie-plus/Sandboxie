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
// Service Control Manager
//---------------------------------------------------------------------------

//#define ErrorMessageBox(txt)
//
//#define HOOK_WIN32(func) {                                                  \
//    const char *FuncName = #func;                                           \
//    void *SourceFunc = Ldr_GetProcAddrNew(DllName_advapi32, L#func, #func); \
//    if (!SourceFunc && Dll_KernelBase) {                                    \
//        SourceFunc = GetProcAddress(Dll_KernelBase, FuncName);              \
//    }                                                                       \
//    __sys_##func =                                                          \
//        (ULONG_PTR)SbieDll_Hook(FuncName, SourceFunc, my_##func);           \
//    if (! __sys_##func)                                                     \
//        hook_success = FALSE;                                               \
//    }
//
//
//#include "../../apps/com/privs.h"

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

static HANDLE Scm_CreateWaitableTimerW(
    LPSECURITY_ATTRIBUTES lpTimerAttributes,
    BOOL bManualReset, LPCWSTR lpTimerName);

//---------------------------------------------------------------------------
// Prototypes
//---------------------------------------------------------------------------


typedef HANDLE (*P_CreateWaitableTimerW)(
    LPSECURITY_ATTRIBUTES lpTimerAttributes,
    BOOL bManualReset, LPCWSTR lpTimerName);


//---------------------------------------------------------------------------
// Pointers
//---------------------------------------------------------------------------


static P_CreateWaitableTimerW       __sys_CreateWaitableTimerW = NULL;


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------

static volatile BOOLEAN Scm_IsMsiServer = FALSE;

static const WCHAR *_MsiServerInUseEventName = SBIE L"_WindowsInstallerInUse";

static HANDLE   Msi_ServerInUseEvent = NULL;


//---------------------------------------------------------------------------
// Scm_SetupMsiHooks
//---------------------------------------------------------------------------


_FX BOOLEAN Scm_SetupMsiHooks()
{

    //while (!IsDebuggerPresent())
    //    Sleep(500);
    //__debugbreak();

    P_CreateWaitableTimerW CreateWaitableTimerW = (P_CreateWaitableTimerW)GetProcAddress(Dll_Kernel32, "CreateWaitableTimerW");
    
    SBIEDLL_HOOK(Scm_, CreateWaitableTimerW);

    //// hook privilege-related functions
    //if (!Hook_Privilege())
    //    return FALSE;

    return TRUE;
}


//---------------------------------------------------------------------------
// Scm_CreateWaitableTimerW
//---------------------------------------------------------------------------


_FX HANDLE Scm_CreateWaitableTimerW(
    LPSECURITY_ATTRIBUTES lpTimerAttributes,
    BOOL bManualReset, LPCWSTR lpTimerName)
{

    //
    // When MsiServer is not started as system (missing the admin groupe) this call fails
    //

    lpTimerAttributes = NULL;

    return __sys_CreateWaitableTimerW(lpTimerAttributes, bManualReset, lpTimerName);
}


//---------------------------------------------------------------------------
// Scm_SetupMsiWaiter
//---------------------------------------------------------------------------


_FX void Scm_SetupMsiWaiter()
{
    //
    // release our hold on the MSI Server in-use flag
    //

    if (Msi_ServerInUseEvent) {
        CloseHandle(Msi_ServerInUseEvent);
        Msi_ServerInUseEvent = NULL;
    }

    //
    // end the service as soon as the in-use flag is gone
    //

    while (1) {

        HANDLE hEvent = OpenEvent(
            EVENT_MODIFY_STATE, FALSE, _MsiServerInUseEventName);

        if (!hEvent) {
            ExitProcess(0);
            break;
        }

        CloseHandle(hEvent);
        Sleep(2000);
    }
}


//---------------------------------------------------------------------------
// Scm_MsiDll
//---------------------------------------------------------------------------


_FX BOOLEAN Scm_MsiDll(HMODULE module)
{
    //
    // MSI library unloading
    //      XXX - Ldr module no longer does unload notifications
    //            so we might rely on MsiCloseHandle instead
    //

    /* if (! module) {

        if (Msi_ServerInUseEvent) {
            CloseHandle(Msi_ServerInUseEvent);
            Msi_ServerInUseEvent = FALSE;
        }

        return TRUE;
    }*/

    //
    // indicate we are one more process that is using the MSI Server
    //

    if (Dll_ImageType != DLL_IMAGE_SANDBOXIE_RPCSS &&
        Dll_ImageType != DLL_IMAGE_SANDBOXIE_DCOMLAUNCH) {

        if ((!Msi_ServerInUseEvent) && (!Scm_IsMsiServer)) {
            Msi_ServerInUseEvent = CreateEvent(
                NULL, TRUE, FALSE, _MsiServerInUseEventName);
        }
    }

    return TRUE;
}
