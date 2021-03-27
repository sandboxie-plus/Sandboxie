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
// Syscall Management, 32-bit code
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// System Service Descriptor Table
//---------------------------------------------------------------------------


__declspec(dllimport) void *KeServiceDescriptorTable;
__declspec(dllimport) KeAddSystemServiceTable(PVOID, PVOID, PVOID, PVOID, PVOID);


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static void *Syscall_HandleOADriver_2(void *ntos_func);

ULONG findFlagOffset(UCHAR *ptr)
{
    ULONG offset = 0;
    unsigned char c = 0;
    unsigned char i = 0;
    unsigned instrSize = 0;

    if (!ptr) {
        return 0;
    }

    for (i = 4; i <= 10; i++) {
        if (*(ULONG *)&ptr[i] == 0x0124a164 && *(USHORT*)&ptr[i + 4] == 0 && ptr[i + 6] == 0x8b) {
            c = i + 7;
            break;
        }
    }

    if (c) {
        if (ptr[c] < 0xc0 && ptr[c] >= 0x80) {
            instrSize = 5;
        }
        else if (ptr[c] < 0x80 && ptr[c] > 0x20) {
            instrSize = 2;
        }

        if (instrSize) {
            c = c + instrSize;
            if (ptr[c] == 0xf7) {   //test [reg + flag_offset], 4 byte mask
                offset = *(ULONG *)&ptr[c + 2];
            }
            else if (*(USHORT *)&ptr[c] == 0xba0f) {    //bt [reg + flag_offset],1 byte value
                offset = *(ULONG *)&ptr[c + 3];
            }
        }
    }
    return offset;
}

ULONG Syscall_GetProcessFlagsOffset()
{
    ULONG offset = 0;
    UNICODE_STRING uni;
    UCHAR * MmMapViewInSessionSpaceEx = NULL;

    RtlInitUnicodeString(&uni, L"MmMapViewInSessionSpaceEx");
    MmMapViewInSessionSpaceEx = (UCHAR *)MmGetSystemRoutineAddress(&uni);

    if (!MmMapViewInSessionSpaceEx) {
        RtlInitUnicodeString(&uni, L"MmMapViewInSessionSpace");
        MmMapViewInSessionSpaceEx = (UCHAR *)MmGetSystemRoutineAddress(&uni);
    }
    if (MmMapViewInSessionSpaceEx) {
        offset = findFlagOffset(MmMapViewInSessionSpaceEx);
    }

    return offset;
}

ULONG Syscall_GetProcessFlags3Offset()
{
    UNICODE_STRING uni;
    UCHAR* PtrPsIsProcessCommitRelinquished;
    ULONG offset = 0;

    RtlInitUnicodeString(&uni, L"PsIsProcessCommitRelinquished");
    PtrPsIsProcessCommitRelinquished = (UCHAR *)MmGetSystemRoutineAddress(&uni);

    if (PtrPsIsProcessCommitRelinquished) {
        if (*((ULONGLONG *)PtrPsIsProcessCommitRelinquished) == 0x08458BEC8B55FF8B) {
            offset = *((ULONG*)(PtrPsIsProcessCommitRelinquished + 0xa));
        }
    }

    return offset;
}


// The function KeAddSystemServiceTable contains references to both the KeServiceDescriptorTable & KeServiceDescriptorTableShadow
// Since both of those tables have the exact same first entry, we search KeAddSystemServiceTable, byte by byte, looking for a pointer to a SYSTEM_SERVICE_TABLE
// that is not KeServiceDescriptorTable, but has the same contents as KeServiceDescriptorTable[0].
// That can only be the KeServiceDescriptorTableShadow.

typedef struct _SYSTEM_SERVICE_TABLE
{
    PULONG ServiceTable;
    PULONG CounterTable;
    ULONG NumberOfServices;
    PUCHAR ParamTable;
} SYSTEM_SERVICE_TABLE, *PSYSTEM_SERVICE_TABLE;

PSYSTEM_SERVICE_TABLE GetShadowTableAddress()
{
    PUCHAR pCheckByte = (PUCHAR)KeAddSystemServiceTable;
    PSYSTEM_SERVICE_TABLE pTable = NULL;
    int i;

    for (i = 0; i < 1024; i++)
    {
        pTable = *(PSYSTEM_SERVICE_TABLE*)pCheckByte;
        if (!MmIsAddressValid(pTable) ||
            ((PVOID)pTable == (PVOID)&KeServiceDescriptorTable) ||
            (memcmp(pTable, &KeServiceDescriptorTable, sizeof(SYSTEM_SERVICE_TABLE))))
        {
            pCheckByte++;
            pTable = NULL;
        }
        if (pTable)
            break;
    }
    return pTable;
}

//---------------------------------------------------------------------------
// Syscall_GetServiceTable
//---------------------------------------------------------------------------


_FX void *Syscall_GetServiceTable(void)
{
    static ULONG *ShadowTable = NULL;
    ULONG *MasterTable;

    if (ShadowTable)
        return ShadowTable;

    // Hard Offset Dependency

    //Set flags2 offset
    if (Driver_OsBuild > 21286) { // 21H2
        
        //
        // on windows 21337 MSFT changed MmMapViewInSessionSpaceEx 
        // so lets go with a known good value from 21286
        // Flags2_Offset=248 MitigationFlags_Offset=1168 SignatureLevel_Offset=932 
        //

        Process_Flags2 = 0xF8;
    }
    else if (Driver_OsVersion > DRIVER_WINDOWS_XP) {
        Process_Flags1 = Syscall_GetProcessFlagsOffset();
        if (Process_Flags1 > 0x80 && Process_Flags1 < 0x500) {
            Process_Flags2 = Process_Flags1 - 4;
        }
        else {
            Process_Flags1 = 0;
            Process_Flags2 = 0;
            Log_Msg1(MSG_1113, L"FLAGS_2");
            return NULL;
        }
    }

    //
    // Master table is always known, we need to find the Shadow table
    //
    MasterTable = (ULONG_PTR *)&KeServiceDescriptorTable;

    if (    Driver_OsVersion >= DRIVER_WINDOWS_XP
         && Driver_OsVersion <= DRIVER_WINDOWS_2003) {

        // Shadow table precedes Master table by 0x40 bytes on
        // Windows XP and Windows 2003 pre SP 1

        ShadowTable = (ULONG_PTR *)((ULONG_PTR)MasterTable - 0x40);

        if (ShadowTable[0] != MasterTable[0] &&
            Driver_OsVersion == DRIVER_WINDOWS_2003) {

            // Shadow table precedes Master table by 0x20 bytes on
            // Windows 2003 post SP 1

            ShadowTable = (ULONG_PTR *)((ULONG_PTR)MasterTable - 0x20);
        }

    } else if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA &&
               Driver_OsVersion <= DRIVER_WINDOWS_7) {

        // Shadow table follows Master table by 0x40 bytes on
        // Windows Vista and Windows 7

        ShadowTable = (ULONG_PTR *)((ULONG_PTR)MasterTable + 0x40);

    } else if (Driver_OsVersion >= DRIVER_WINDOWS_8 &&
               Driver_OsVersion <= DRIVER_WINDOWS_10) {

        // Shadow table precedes Master table by 0x40 bytes on
        // Windows 8, Windows 8.1

        if (Driver_OsBuild >= 14965) {
            Process_Flags3 = Syscall_GetProcessFlags3Offset();
            if (!Process_Flags3) {
                Log_Msg1(MSG_1113, L"FLAGS_3");
                return NULL;
            }
        }
        ShadowTable = (ULONG_PTR *)((ULONG_PTR)MasterTable - 0x40);
    }

    if ((!ShadowTable) || (ShadowTable[0] != MasterTable[0])) {

        ShadowTable = (ULONG*)GetShadowTableAddress();
        if (ShadowTable == NULL)
        {
            // error T - can't locate shadow table
            Log_Msg1(MSG_1113, L"TABLE");
        }
    }

    return ShadowTable;
}


//---------------------------------------------------------------------------
// Syscall_GetIndexFromNtdll
//---------------------------------------------------------------------------


_FX ULONG Syscall_GetIndexFromNtdll(
    UCHAR *code, const UCHAR *name, ULONG name_len)
{
    ULONG index = -1;

    if (*code == 0xB8)                      // mov eax, syscall number
        index = *(ULONG *)(code + 1);

    if (index == -1)
        Log_Msg1(MSG_1113, L"INDEX");

    return index;
}


//---------------------------------------------------------------------------
// Syscall_GetKernelAddr
//---------------------------------------------------------------------------


_FX BOOLEAN Syscall_GetKernelAddr(
    ULONG index, void **pKernelAddr, ULONG *pParamCount)
{
    ULONG *ServiceTable = Syscall_GetServiceTable();
    if (ServiceTable) {

        USHORT MaxSyscallIndexPlusOne = (USHORT)ServiceTable[2];
        if ((index < 0x1000) &&
                        ((index & 0xFFF) < MaxSyscallIndexPlusOne)) {

            ULONG *ProcTable = (ULONG *)(ServiceTable[0]);
            UCHAR *ParmTable = (UCHAR *)(ServiceTable[3]);

            *pKernelAddr = (void *)ProcTable[index];
            *pParamCount = ((ULONG)ParmTable[index]) / 4;
            return TRUE;
        }

        Log_Msg1(MSG_1113, L"ADDRESS");
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// Syscall_HandleOADriver
//---------------------------------------------------------------------------


_FX void Syscall_HandleOADriver(void)
{
    //
    // conflict with Online Armor firewall software and workaround:
    //
    // OADriver kernel driver replaces service descriptor table pointers
    // (i.e. hooks NT syscalls), and some of the replacement routines
    // incorrectly enter an infinite wait, in the context of a process
    // in the sandbox under Sandboxie v4.
    //
    // to work around this, we Syscall_HandleOADriver_2 try to extract
    // the address of the saved descriptor table pointer, so our
    // Syscall_Invoke function can bypass the problematic OA routines
    //
    // this workaround is not required on 64-bit Windows.
    //

    if (Driver_CheckThirdParty(L"OADevice", SERVICE_FILE_SYSTEM_DRIVER)) {

        /*UCHAR *OADriverApisToFix[] = {
            "AllocateVirtualMemory", "CreateFile", "CreateKey", "CreatePort",
            "CreateProcess", "CreateProcessEx", "CreateSection",
            "CreateThread", "DebugActiveProcess", "DuplicateObject",
            "LoadDriver", "OpenFile", "OpenProcess", "OpenSection",
            "OpenThread", "ConnectPort", "SecureConnectPort",
            NULL
        };*/
        UCHAR *OADriverApisToFix[] = {
            "OpenSection", "ConnectPort", "SecureConnectPort",
            NULL
        };
        ULONG i;

        for (i = 0; OADriverApisToFix[i]; ++i) {

            SYSCALL_ENTRY *entry = Syscall_GetByName(OADriverApisToFix[i]);
            if (entry) {

                void *new_ntos_func =
                            Syscall_HandleOADriver_2(entry->ntos_func);
                if (new_ntos_func) {

                    // DbgPrint("Name = %s Old = %08X New = %08X\n", OADriverApisToFix[i], entry->ntos_func, new_ntos_func);

                    entry->ntos_func = new_ntos_func;
                }
            }
        }
    }
}


//---------------------------------------------------------------------------
// Syscall_HandleOADriver_2
//---------------------------------------------------------------------------


_FX void *Syscall_HandleOADriver_2(void *ntos_func)
{
    UCHAR *code = ntos_func;
    ULONG i;
    ULONG jump_target_1 = 0;
    ULONG jump_target_2 = 0;

    //
    // the function begins with two comparisons and two conditional jumps
    // which go to the same address
    //

    for (i = 0; i < 48; ++i) {

        USHORT *code2 = (USHORT *)(code + i);
        UCHAR *jump_inst;
        if (*code2 == 0x3539)       // cmp dword ptr [xx], esi
            jump_inst = code + i + 6;
        else if (*code2 == 0x1D39)  // cmp dword ptr [xx], ebx
            jump_inst = code + i + 6;
        else if (*code2 == 0x3D83)  // cmp dword ptr [xx], nnn
            jump_inst = code + i + 7;
        else
            jump_inst = NULL;
        if (jump_inst && *(USHORT *)jump_inst == 0x840F) {   // je

            //
            // calculate the target address of the jump instructions
            //

            ULONG jump_target = *(ULONG *)(jump_inst + 2)
                              + (ULONG)jump_inst + 6;
            if (! jump_target_1)
                jump_target_1 = jump_target;
            else {
                if (! jump_target_2)
                    jump_target_2 = jump_target;
                break;
            }
        }
    }

    //
    // the two jump instructions must go to the same address
    //

    if ((! jump_target_1) || (jump_target_1 != jump_target_2))
        return NULL;
    code = (UCHAR *)jump_target_1;

    //
    // extract the address for the original SSDT routine
    //

    for (i = 0; i < 48; ++i) {

        USHORT *call_1 = (USHORT *)(code + i);
        if (*call_1 == 0x15FF) {            // call dword ptr [xx]
            UCHAR *call2 = code + i + 6;
            if (call2[0] == 0xE8 && call2[5] == 0xC2) { // call xx ; ret xx

                void ***orig_syscall_ptr = (void ***)(call_1 + 1);
                return **orig_syscall_ptr;
            }
        }
    }

    return NULL;
}
