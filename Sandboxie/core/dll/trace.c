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

#include "dll.h"
#include "trace.h"


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

    if (SbieApi_QueryConfBool(NULL, L"ErrorTrace", FALSE)) {
        RtlSetLastWin32Error = (P_RtlSetLastWin32Error)
            GetProcAddress(Dll_Ntdll, "RtlSetLastWin32Error");
        SBIEDLL_HOOK(Trace_, RtlSetLastWin32Error);
    }

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


//---------------------------------------------------------------------------
// Trace_FindModuleByAddress
//---------------------------------------------------------------------------

#include "../../common/my_xeb.h"

WCHAR* Trace_FindModuleByAddress(void* address)
{
    WCHAR* found = NULL;

    PLIST_ENTRY Head, Next;
    PLDR_DATA_TABLE_ENTRY Entry;
    PPEB peb = (PPEB)NtCurrentPeb();

    EnterCriticalSection((PRTL_CRITICAL_SECTION)peb->LoaderLock);

    Head = &peb->Ldr->InLoadOrderModuleList;
    Next = Head->Flink;

    while (Next != Head)
    {
        Entry = CONTAINING_RECORD(Next, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

        if (Entry->DllBase < address && (UINT_PTR)Entry->DllBase + Entry->SizeOfImage > (UINT_PTR)address)
        {
            found = Entry->BaseDllName.Buffer;
            break;
        }
        Next = Next->Flink;
    }

    LeaveCriticalSection((PRTL_CRITICAL_SECTION)peb->LoaderLock);

    return found;
}