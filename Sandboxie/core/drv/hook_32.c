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
// Function Hooking and Instrumentation, 32-bit version
//---------------------------------------------------------------------------


#ifndef _WIN64


#define HOOK_WITH_PRIVATE_PARTS
#include "hook.h"
#include "syscall.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static ULONG Hook_Find_ZwRoutine_1(ULONG service_index, void **out_routine);
static ULONG Hook_Find_ZwRoutine_2(ULONG service_index, void **out_routine);

__declspec(dllimport) void __cdecl wcstombs(void);


//---------------------------------------------------------------------------


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, Hook_Find_ZwRoutine_1)
#pragma alloc_text (INIT, Hook_Find_ZwRoutine_2)
#endif // ALLOC_PRAGMA


//---------------------------------------------------------------------------
// Hook_GetNtServiceInternal
//---------------------------------------------------------------------------


_FX void *Hook_GetNtServiceInternal(ULONG ServiceIndex, ULONG ParamCount)
{
    ULONG *ProcTable;
    UCHAR *ParmTable;
    void *value;
    WCHAR err[2];

    ULONG *ShadowTable = Syscall_GetServiceTable();
    if (! ShadowTable)
        return NULL;

    value = NULL;
    err[0] = err[1] = 0;

    if (ServiceIndex >= 0x2000 || (ServiceIndex & 0xFFF) >= 0x600) {
        err[0] = L'1';      // invalid service number
        goto finish;
    }

    if (ServiceIndex & 0x1000) {
        ProcTable = (ULONG *)(ShadowTable[4]);
        ParmTable = (UCHAR *)(ShadowTable[4 + 3]);
        ServiceIndex &= ~0x1000;
    } else {
        ProcTable = (ULONG *)(ShadowTable[0]);
        ParmTable = (UCHAR *)(ShadowTable[0 + 3]);
    }

    if (ParmTable[ServiceIndex] != ParamCount * 4) {
        err[0] = L'2';      // parameter count mismatch
        goto finish;
    }

    value = (void *)ProcTable[ServiceIndex];

finish:
    if (err[0])
        Log_Msg1(MSG_HOOK_NT_SERVICE, err);

    return value;
}


//---------------------------------------------------------------------------
// Hook_GetZwServiceInternal
//---------------------------------------------------------------------------


_FX void *Hook_GetZwServiceInternal(ULONG ServiceIndex)
{
    ULONG subcode;
    void *routine;

    routine = NULL;
    subcode = Hook_Find_ZwRoutine_1(ServiceIndex, &routine);
    if (subcode == 0x99)
        subcode = Hook_Find_ZwRoutine_2(ServiceIndex, &routine);
    if (subcode != 0) {
        WCHAR err[8];
        RtlStringCbPrintfW(err, sizeof(err), L"0x%X", subcode);
        Log_Msg1(MSG_HOOK_ZW_SERVICE, err);
        routine = NULL;
    }
    return routine;
}


//---------------------------------------------------------------------------
// Hook_Find_ZwRoutine_1
//---------------------------------------------------------------------------


_FX ULONG Hook_Find_ZwRoutine_1(ULONG ServiceNum, void **out_routine)
{
    UCHAR *addr;
    BOOLEAN found = FALSE;
    int i;

    //
    // ZwAccessCheckAndAuditAlarm is the first ZwXxx routine in 32-bit
    // kernels, but if driver verifier is enabled, then it will be
    // redirected to VfZwAccessCheckAndAuditAlarm, in that case we
    // use ZwAlertThread which is still one of the first routines
    //

    UCHAR *first_routine = (UCHAR *)ZwAccessCheckAndAuditAlarm;
    UCHAR *last_routine  = (UCHAR *)ZwYieldExecution;

    if (Driver_OsVersion < DRIVER_WINDOWS_8) {

        if ((UCHAR *)ZwAlertThread < first_routine)
            first_routine = (UCHAR *)ZwAlertThread;

    } else {

        //
        // on Windows 8, ZwXxx routines appear in the reverse order,
        // with ZwZxx first, and ZwAxx last
        //

        UCHAR *swap_routine = first_routine;
        first_routine = last_routine;
        last_routine  = swap_routine;
    }

    // DbgPrint("First Routine %X Last Routine %X\n", first_routine, last_routine);

    // scan the ZwXxx functions in NTOSKRNL, starting with a relatively
    // early one in the alphabet (ZwA...), to find the one that invokes
    // the system service with the index we found

    addr = first_routine;
    while (addr != last_routine) {

        // DbgPrint("    Address %X Byte %X\n", addr, *addr);

        if (*addr == 0xB8 && *(ULONG *)(addr + 1) == ServiceNum) {
            found = TRUE;
            break;
        }

        if (*(ULONG *)addr == 0x08C2C033 && addr[4] == 0x00) {
            // HAL7600 activation tool overwrites ZwLockProductActivationKeys
            // with 33 C0 C2 08 00 (xor eax,eax ; ret 8), but leaves the
            // original "ret 8" at the end of the original code, so we try to
            // locate this and advance to the next redirector
            for (i = 5; i < 20; ++i) {
                if (addr[i + 0] == 0xC2 &&
                                addr[i + 1] == 0x08 && addr[i + 2] == 0x00)
                    break;
            }
            if (i < 20) {
                addr += i + 3;
                goto skip_padding_bytes;
            }
        }

        // DbgPrint("Address %X Byte %X\n", addr, *addr);

        // a ZwXxx system service redirector looks like any of these :-
        //      for Windows 2000
        // 0xB8 xx xx xx xx 0x8D xx xx xx CD 2E C2 xx xx 8B FF
        // 0xB8 xx xx xx xx 0x8D xx xx xx CD 2E C3
        //      for Windows XP
        // 0xB8 xx xx xx xx 0x8D xx xx xx 9C 6A 08 E8 xx xx xx xx C2 xx xx
        // 0xB8 xx xx xx xx 0x8D xx xx xx 9C 6A 08 E8 xx xx xx xx C3
        // and may be followed by a number of 90s and 8B FFs

        if (*addr != 0xB8)              // 0xB8 xx xx xx xx
            break;
        addr += 5;

        if (*addr != 0x8D)              // 0x8D xx xx xx
            break;
        addr += 4;

        if (*addr == 0xCD)
            addr += 2;
        else if (*addr == 0x9C)
            addr += 8;
        else
            break;

skip_padding_bytes:

        for (i = 0; i < 8; ++i) {
            // DbgPrint("    Address %X Byte %X\n", addr, *addr);
            switch (*addr) {
                case 0xC2:  addr += 3;  break;          // RET nnnn
                case 0xC3:  addr += 1;  break;          // RET
                case 0x90:  addr += 1;  break;          // NOP
                case 0x8B:  addr += 2;  break;          // MOV EDI, EDI
                default:                break;          // unknown
            }
        }
    }

    if ((! found) && (addr != last_routine)) {
        // DbgPrint("Stopped at address %X\n", addr);
        return 0x25;
    }

    if (found) {
        *out_routine = addr;
        return 0;
    } else
        return 0x99;    // indicate we should try Hook_Find_ZwRoutine_2
}


//---------------------------------------------------------------------------
// Hook_Find_ZwRoutine_2
//---------------------------------------------------------------------------


_FX ULONG Hook_Find_ZwRoutine_2(ULONG ServiceNum, void **out_routine)
{
    UCHAR *addr = NULL;
    BOOLEAN found = FALSE;
    ULONG subcode = 0;
    int i;

    static const UCHAR ZwXxx_Code[] = {
        0x8D, 0x54, 0x24, 0x04,                 // lea     edx,[esp+0x4]
        0x9C,                                   // pushfd
        0x6A, 0x08,                             // push    0x8
        0xE8                                    // call    ...
    };

    __try {

        // starting with XP SP1a, the unexported ZwXxx routines are placed
        // in a different area in NTOSKRNL.EXE.  In XP SP1a, they start at
        // wcstomb+0x4FD9 with the ZwXxx for service index 1. We search
        // for them

        for (addr = (UCHAR *)wcstombs + 0x4000;
             addr < (UCHAR *)wcstombs + 0x6000;
             ++addr) {

            if (*addr == 0xB8 &&
                sizeof(ZwXxx_Code) == RtlCompareMemory(
                        addr + 5, ZwXxx_Code, sizeof(ZwXxx_Code))) {
                found = TRUE;
                break;
            }
        }

        if (! found)
            subcode = 0x26;
        else {

            found = FALSE;

            while (*addr == 0xB8) {
                if (*(ULONG *)(addr + 1) == ServiceNum) {
                    found = TRUE;
                    break;
                }
                addr += 0x11;
                // skip up to six bytes to find the next 0xB8
                for (i = 0; i < 6 && *addr != 0xB8; ++i, ++addr)
                    ;
            }

            if (! found)
                subcode = 0x27;
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        subcode = 0x28;
    }

    if (subcode == 0)
        *out_routine = addr;

    return subcode;
}


//---------------------------------------------------------------------------


#endif _WIN64
