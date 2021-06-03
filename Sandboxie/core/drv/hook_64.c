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


#ifdef _WIN64


#define HOOK_WITH_PRIVATE_PARTS
#include "hook.h"
#include "syscall.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static void *Hook_GetShadowTable(void);

static ULONG Hook_Find_ZwRoutine(ULONG service_index, void **out_routine);


//---------------------------------------------------------------------------


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, Hook_Find_ZwRoutine)
#endif // ALLOC_PRAGMA


//---------------------------------------------------------------------------
// System Service Descriptor Table
//---------------------------------------------------------------------------


#pragma pack(push)
#pragma pack(1)


typedef struct _SERVICE_DESCRIPTOR {

    LONG *Addrs;
    ULONG *DontCare1;   // always zero?
    ULONG Limit;
    LONG DontCare2;     // always zero?
    UCHAR *DontCare3;

} SERVICE_DESCRIPTOR;


#pragma pack(pop)



//---------------------------------------------------------------------------
// Hook_GetNtServiceInternal
//---------------------------------------------------------------------------


_FX void *Hook_GetNtServiceInternal(ULONG ServiceIndex, ULONG ParamCount)
{
    SERVICE_DESCRIPTOR *Descr;
    ULONG TableCount;
    LONG_PTR offset;
    void *value;
    WCHAR err[2];

    SERVICE_DESCRIPTOR *ShadowTable = Syscall_GetServiceTable();
    if (! ShadowTable)
        return NULL;

    value = NULL;
    err[0] = err[1] = 0;

    if (ServiceIndex >= 0x2000 || (ServiceIndex & 0xFFF) >= 0x600) {
        err[0] = L'1';      // invalid service number
        goto finish;
    }

    if (ServiceIndex & 0x1000) {
        Descr = &ShadowTable[1];
        ServiceIndex &= ~0x1000;
    } else
        Descr = &ShadowTable[0];

    //
    // the 64-bit service table only counts the number of parameters
    // above four, because the first four are passed in registers
    //

    TableCount = (ULONG)(Descr->Addrs[ServiceIndex] & 0x0F);
    if ((ParamCount <= 4 && TableCount != 0) ||
        (ParamCount  > 4 && TableCount != ParamCount - 4)) {

        err[0] = L'2';      // parameter count mismatch
        goto finish;
    }

    offset = (LONG_PTR)Descr->Addrs[ServiceIndex];
    offset >>= 4;

    value = (UCHAR *)Descr->Addrs + offset;

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
    subcode = Hook_Find_ZwRoutine(ServiceIndex, &routine);
    if (subcode != 0) {
        WCHAR err[8];
        RtlStringCbPrintfW(err, sizeof(err), L"%d", subcode);
        Log_Msg1(MSG_HOOK_ZW_SERVICE, err);
        routine = NULL;
    }
    return routine;
}


//---------------------------------------------------------------------------
// Hook_Find_ZwRoutine
//---------------------------------------------------------------------------


_FX ULONG Hook_Find_ZwRoutine(ULONG ServiceNum, void **out_routine)
{
    UCHAR *addr;
    BOOLEAN found = FALSE;
    int i;

    // on 64-bit Windows, ZwXxx functions seem to be ordered arbitrarily

    UCHAR *first_routine = (UCHAR *)ZwWaitForSingleObject;
    UCHAR *last_routine  = (UCHAR *)ZwUnloadKey + 0x140;

    // scan the ZwXxx functions in NTOSKRNL, to find the one that invokes
    // the system service with the index we found

    i = 0;
    addr = first_routine;
    while (addr != last_routine) {

        UCHAR *save_addr = addr;

        // DbgPrint("Address %X Byte %X\n", addr, *addr);

        if (*(USHORT *)addr == 0xC033 && addr[2] == 0xC3) {
            // HAL7600 activation tool overwrites ZwLockProductActivationKeys
            // with 33 C0 C3 (xor eax,eax ; ret), but leaves the original
            // "xchg ax,ax" at the end of the original code, so we try to
            // locate this and advance to the next redirector
            for (i = 3; i < 36; ++i) {
                if (*(USHORT *)(addr + i) == 0x9066)
                    break;
            }
            if (i < 36) {
                addr += i + 2;
                continue;
            }
        }

        // a ZwXxx system service redirector looks like this in XP 64-bit :-
        // 0x48 8B C4 FA 48 83 EC xx 50 9C 6A xx 48 8D xx xx xx xx xx 50 ...
        //  ... B8 xx xx xx xx E9 xx xx xx xx 66 90

        if (addr[0] != 0x48 || addr[1] != 0x8B)
            break;
        addr += 4;
        if (addr[0] != 0x48 || addr[1] != 0x83)
            break;
        addr += 8;
        if (addr[0] != 0x48 || addr[1] != 0x8D)
            break;
        addr += 8;

        if (addr[0] != 0xB8)
            break;
        if (*(ULONG *)(addr + 1) == ServiceNum) {
            addr = save_addr;
            found = TRUE;
            break;
        }
        addr += 5;

        if (addr[0] != 0xE9)
            break;
        addr += 5;

        if (addr[0] != 0x66 || addr[1] != 0x90)
            break;
        addr += 2;
    }

    if ((! found) && (addr != last_routine)) {
        return 0x24;
    }

    if (found) {
        *out_routine = addr;
        return 0;
    } else
        return 0x29;
}


//---------------------------------------------------------------------------


#endif // _WIN64
