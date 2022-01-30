/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020 David Xanatos, xanasoft.com
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
// Hook Management
//---------------------------------------------------------------------------


#ifndef _MY_HOOK_H
//#define _MY_HOOK_H // defined by  "../dll/hook.h"


#include "driver.h"

#include "../dll/hook.h"

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


// Hook_GetServiceIndex analyzes the NTDLL or USER32 user mode stub,
// specified by DllProc, and computes the system service number.

LONG Hook_GetServiceIndex(void *DllProc, LONG *SkipIndexes);

// Hook_GetService analyzes the NTDLL or USER32 user mode stub, specified
// by DllProc, and computes the system service number, and uses this number
// to return the NTOSKRNL procedure that would actually service the request.
// This returns the both real routine within NTOSKRNL (NtXxx) and also the
// kernel-mode Zw dispatcher stub (ZwXxx).

BOOLEAN Hook_GetService(
	void *DllProc, LONG *SkipIndexes, ULONG ParamCount,
	void **NtService, void **ZwService);


#ifdef HOOK_WITH_PRIVATE_PARTS

// Returns the address of the NTOS kernel service identified by the
// specified service index.  It must take exactly as many parameters
// as indicated.  This routine is implemented differently for 32-bit
// and 64-bit mode.

void *Hook_GetNtServiceInternal(ULONG ServiceIndex, ULONG ParamCount);
void *Hook_GetZwServiceInternal(ULONG ServiceIndex);


#endif // HOOK_WITH_PRIVATE_PARTS


//---------------------------------------------------------------------------


NTSTATUS Hook_Api_Tramp(PROCESS *proc, ULONG64 *parms);


//---------------------------------------------------------------------------


#endif // _MY_HOOK_H
