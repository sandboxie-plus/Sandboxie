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

#include "dll.h"
#include "common/my_version.h"
#include <stdio.h>

void Ldr_LoadInjectDlls(BOOLEAN bHostInject);
extern BOOLEAN g_bHostInject;

typedef DWORD (__stdcall *P__CorExeMain)();

P__CorExeMain __sys__CorExeMain = NULL;

_FX DWORD MsCorEE__CorExeMain()
{
    static BOOL bFirstCall = TRUE;
    DWORD ret = 0;

    // Load inject dlls just once
    if (bFirstCall)
    {
        // if we caused PEB.ReadImageFileExecOptions to be non-zero then restore
        // the zero value here
        if (Dll_OsBuild < 8400) {

            UCHAR *ReadImageFileExecOptions = (UCHAR *)(NtCurrentPeb() + 1);
            if (*ReadImageFileExecOptions == '*')
                *ReadImageFileExecOptions = 0;
        }

        //
        // do some post-LDR initialization
        //
        Ldr_LoadInjectDlls(g_bHostInject);

        bFirstCall = FALSE;
    }

    ret = __sys__CorExeMain();

    return ret;
}


// Load inject dlls in .Net process' entry (_CorExeMain).
_FX BOOLEAN MsCorEE_Init(HMODULE hmodule)
{

    // Use the code from AdvApi_Init
#define GETPROC(x,s) __sys_##x##s = (P_##x) Ldr_GetProcAddrNew(DllName_mscoree, L#x L#s,#x #s);
    
    P__CorExeMain _CorExeMain = NULL;

    GETPROC(_CorExeMain,);

#undef GETPROC

    _CorExeMain = __sys__CorExeMain;

    SBIEDLL_HOOK(MsCorEE_,_CorExeMain);
    
    return TRUE;
}


