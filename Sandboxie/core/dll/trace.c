/*
 * Copyright 2020 DavidXanatos, xanasoft.com
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
// Trace Helper
//---------------------------------------------------------------------------


#include "trace.h"
#include "dll.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static void Trace_RtlSetLastWin32Error(ULONG err);

static void Trace_OutputDebugStringW(const WCHAR *str);

static void Trace_OutputDebugStringA(const UCHAR *str);


//---------------------------------------------------------------------------


typedef void (*P_RtlSetLastWin32Error)(ULONG err);
typedef void (*P_OutputDebugString)(const void *str);


static P_RtlSetLastWin32Error       __sys_RtlSetLastWin32Error      = NULL;
static P_OutputDebugString          __sys_OutputDebugStringW        = NULL;
static P_OutputDebugString          __sys_OutputDebugStringA        = NULL;


//---------------------------------------------------------------------------
// Trace_Init
//---------------------------------------------------------------------------


_FX int Trace_Init(void)
{
	P_RtlSetLastWin32Error RtlSetLastWin32Error;
    P_OutputDebugString OutputDebugStringW;
    P_OutputDebugString OutputDebugStringA;

    //
    // intercept NTDLL entry points
    //

    RtlSetLastWin32Error = (P_RtlSetLastWin32Error)
        GetProcAddress(Dll_Ntdll, "RtlSetLastWin32Error");
    SBIEDLL_HOOK(Trace_,RtlSetLastWin32Error);

    //
    // intercept KERNEL32 entry points
    //

    OutputDebugStringW = (P_OutputDebugString)
        GetProcAddress(Dll_Kernel32, "OutputDebugStringW");
	SBIEDLL_HOOK(Trace_, OutputDebugStringW);

    OutputDebugStringA = (P_OutputDebugString)
        GetProcAddress(Dll_Kernel32, "OutputDebugStringA");
    SBIEDLL_HOOK(Trace_,OutputDebugStringA);

    return TRUE;
}


//---------------------------------------------------------------------------
// Trace_RtlSetLastWin32Error
//---------------------------------------------------------------------------


ALIGNED void Trace_RtlSetLastWin32Error(ULONG err)
{
    if (err) {
		WCHAR strW[64];
        Sbie_snwprintf(strW, 64, L"SetError: %d\n", err);
		SbieApi_MonitorPut2(MONITOR_OTHER | MONITOR_TRACE, strW, FALSE);
    }
    __sys_RtlSetLastWin32Error(err);
}


//---------------------------------------------------------------------------
// Trace_OutputDebugStringW
//---------------------------------------------------------------------------


ALIGNED void Trace_OutputDebugStringW(const WCHAR *strW)
{
	SbieApi_MonitorPut2(MONITOR_OTHER | MONITOR_TRACE, strW, FALSE);

    __sys_OutputDebugStringW(strW);
}


//---------------------------------------------------------------------------
// Trace_OutputDebugStringA
//---------------------------------------------------------------------------


ALIGNED void Trace_OutputDebugStringA(const UCHAR *strA)
{
	WCHAR strW[256 + 1];
	Sbie_snwprintf(strW, 256 + 1, L"%S", strA); // convert to WCHAR
	SbieApi_MonitorPut2(MONITOR_OTHER | MONITOR_TRACE, strW, FALSE);

    __sys_OutputDebugStringA(strA);
}
