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
// Syscall Management, 64-bit code
//---------------------------------------------------------------------------


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
// Functions
//---------------------------------------------------------------------------


static void *Syscall_GetMasterServiceTable(void);


//---------------------------------------------------------------------------

#define MAX_SERVICE_TABLE_SEARCH_SIZE 0xa0
#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, Syscall_GetMasterServiceTable)
#endif // ALLOC_PRAGMA


//---------------------------------------------------------------------------
// Syscall_GetMasterServiceTable
//---------------------------------------------------------------------------


_FX void *Syscall_GetMasterServiceTable(void)
{
    NTSTATUS status;
    void *MasterTable;

    UNICODE_STRING uni;
    UCHAR *ptr;
    ULONG i;
    ULONG_PTR nt, nt_from_code;
    ULONG ofs32a;
    ULONG ofs32b;

    //
    // if NTOSKRNL exports KeServiceDescriptorTable, use that
    //

    RtlInitUnicodeString(&uni, L"KeServiceDescriptorTable");
    MasterTable = MmGetSystemRoutineAddress(&uni);

    if (MasterTable)
        return MasterTable;

    // else we find it ourselves.  first find where the NT kernel was loaded.
    // this may be variable on Vista
    //

    ptr = ExAllocatePoolWithTag(PagedPool, PAGE_SIZE, tzuk);
    if (! ptr) {

        Log_Msg0(MSG_1104);
        return NULL;
    }

    i = 0;
    status = ZwQuerySystemInformation(
        SystemModuleInformation, ptr, PAGE_SIZE, &i);

    if (status != STATUS_SUCCESS &&
        status != STATUS_INFO_LENGTH_MISMATCH) {

        ExFreePoolWithTag(ptr, tzuk);
        return NULL;
    }

    nt = ((SYSTEM_MODULE_INFORMATION *)ptr)->ModuleInfo[0].ImageBaseAddress;

    ExFreePoolWithTag(ptr, tzuk);

    //MasterTable lookup for windows 10
    if (Driver_OsBuild >= 10041) {

        ULONG_PTR kernel_base = nt;
        RtlInitUnicodeString(&uni, L"KeAddSystemServiceTable");
        ptr = (UCHAR *)MmGetSystemRoutineAddress(&uni);

        DbgPrint("Trying KiInitSystem = %p, OS = %d\n", MasterTable, Driver_OsBuild);
        if (ptr) {
            nt = 0;
            //Look for the following instruction pattern
            //LEA <register>, target address       : [0x48 || 0x4c][0x8d][<0x20][4 byte offset to target]
            //MOV qword[target address],<register> : [0x48 || 0x4c][0x89][<0x20][4 byte offset to target]

            //The target address discovered in this case is not the system service table but instead points
            //to the middle of a structure that contains both the master and shadow table addresses:
            //the shadow table is at <target address> + 0x20
            //the master table is at <target address> - 0x20

            //The following heuristic matches a 64 bit LEA (memory to register) instruction to an associated 64 bit MOV
            //(register to memory) both instructions need to point to the same relative target address. 
            //The target address range needs to be above (less than) the current address (reference address is the address of the following instruction) 
            //of up to -0x200000. If a non matching 64 bit MOV follows the LEA instruction the heuristic continues to search for 
            //a matching pair up to MAX_SERVICE_TABLE_SEARCH_SIZE bytes.

            for (i = 10, ptr = ptr + 10; i < MAX_SERVICE_TABLE_SEARCH_SIZE; i++, ptr++) {
                LONG delta;
                //filter on full 64 bit only opcodes.
                if (*ptr == 0x48 || *ptr == 0x4c || *ptr == 0x44) { //parse rex prefix
                    //switch(operand 1)
                    switch (ptr[1]) {
                        unsigned long testDelta;
                    case 0x8d:  //LEA instruction is first:  sets a target address
                                //if another LEA in a instruction is discovered before an associated
                                //MOV instruction is found.  A new target address is set based on the
                                //the new LEA instruction if the target is in the target memory 
                                //range.
                        delta = *(LONG *)(ptr + 3);
                        testDelta = delta * -1;
                        if (ptr[0] == 0x44 && ptr[2] == 0x90 && delta > 0 && delta < 0x400000) {
                            nt = kernel_base + delta;
                            i += 6;
                            ptr += 6;
                        }
                        //test operand 2 is in register range
                        else if (ptr[2] < 0x20) {
                            nt = (LONG_PTR)(ptr + 7) + delta;
                            i += 6;
                            ptr += 6;
                        }
                        break;
                    case 0x89:  //MOV instruction is second: verifies the discovered target address in the
                                //previous LEA instruction.
                        delta = *(LONG *)(ptr + 3);
                        //test operand 2 is in register range
                        if (ptr[2] < 0x20 && nt && nt == (LONG_PTR)(ptr + 7) + delta) {
                            if (Driver_OsBuild >= 17682) {
                                nt -= 0x20;
                            }
                            else {
                                nt += 0x20;
                            }
                            return (void *)nt;
                        }
                        break;
                    }
                }
            }
            return 0; //target address not found
        }
    }

    //
    // next, analyze KeAddSystemServiceTable to learn where shadow table is
    //

    RtlInitUnicodeString(&uni, L"KeAddSystemServiceTable");
    ptr = MmGetSystemRoutineAddress(&uni);
    if (! ptr)
        return NULL;

    //
    // within the first 32 bytes, we expect the instruction "lea r11,nt"
    // or any variation thereof, which looks like this:
    // address:  4C xx xx ofs32
    // such that (address + sizeof(instruction) + ofs32) == nt
    //
    // on Windows 8:  expect "lea rbx,nt" instead, which is 48 xx xx ofs32
    //

    nt_from_code = 0;

    for (i = 0; i < 32; ++i) {
        if (*ptr == 0x4C || *ptr == 0x48) {
            ofs32a = *(ULONG *)&ptr[3];
            nt_from_code = (ULONG_PTR)(ptr + 7 + (LONG_PTR)(LONG)ofs32a);
            if (nt_from_code == nt) {
                ptr += 7;
                break;
            }
            nt_from_code = 0;
        }
        ++ptr;
    }

    if (nt != nt_from_code)
        return NULL;

    //
    // within the next 32 bytes we expect two "cmp [xx+ofs32],0" instructions
    // such that either ofs32 is less than 0x400000 bytes away from nt and
    // are exactly 0x40 bytes apart (on Vista SP2:  0xC0 bytes apart)
    //

    ofs32a = -1;
    ofs32b = -1;

    for (i = 0; i < 32; ++i) {
        if ((ptr[0] == 0x4A || ptr[0] == 0x4B) &&
                ptr[1] == 0x83 && ptr[8] == 0x00)
        {
            if (ofs32a == -1)
                ofs32a = *(ULONG *)&ptr[4];
            else if (ofs32b == -1) {
                ofs32b = *(ULONG *)&ptr[4];
                break;
            }
        }
        ++ptr;
    }

    if (ofs32a >= 0x400000 || ofs32b >= 0x400000) {

        //
        // alternatively on Windows 8: expect "cmp [rax+rbx+ofs32],0"
        // followed by "lea rcx,[rbx+ofs32]
        //

        ptr -= i;

        for (i = 0; i < 32; ++i) {
            if (ptr[0] == 0x48) {
                if (ptr[1] == 0x83 && ptr[2] == 0xBC && ptr[3] == 0x18 &&
                        ptr[8] == 0x00 && ofs32a == -1)
                    ofs32a = *(ULONG *)&ptr[4];
                else if (ptr[1] == 0x8D && ptr[2] == 0x8B && ofs32b == -1)
                    ofs32b = *(ULONG *)&ptr[3];

            }
            ++ptr;
        }
    }

    //
    // the service descriptor tables should be less than 0x400000
    // bytes from the start of the 'nt' module, and should be exactly
    // 0x40 or 0xC0 bytes apart
    //

    if (ofs32a >= 0x400000 || ofs32b >= 0x400000)
        return NULL;

    // This code block is broken by KB4056892 (for Win 7-64 so far). This is the Intel Meltdown bug
    //if (        (ofs32a - ofs32b != 0x40 && ofs32b - ofs32a != 0x40)
    //         && (ofs32a - ofs32b != 0xC0 && ofs32b - ofs32a != 0xC0))
    //    return NULL;

    MasterTable = (void *)(nt + ofs32a);

    return MasterTable;
}


//---------------------------------------------------------------------------
// Syscall_GetServiceTable
//---------------------------------------------------------------------------


_FX void *Syscall_GetServiceTable(void)
{
    static SERVICE_DESCRIPTOR *ShadowTable = NULL;
    SERVICE_DESCRIPTOR *MasterTable;

    if (ShadowTable)
        return ShadowTable;

    // Hard Offset Dependency

    //
    // the shadow table should be 0x40 bytes before the master table,
    // on Windows XP (64-bit),
    // and 0x40 bytes after the master table, on Windows Vista (64-bit)
    //     0xC0 bytes after the master table, on    Vista SP 2 (64-bit)
    //

    MasterTable = (SERVICE_DESCRIPTOR *)Syscall_GetMasterServiceTable();

    if (!MasterTable) {
        Log_Msg1(MSG_1113, L"MASTER TABLE");
        return NULL;
    }
    if (Driver_OsBuild >= 17682) {
        //In windows 10 the ShadowTable and MasterTable point the same memory
        //they are equal.
        ShadowTable = (SERVICE_DESCRIPTOR *)MasterTable;
    }
    else if (Driver_OsBuild >= 9800) {
        ShadowTable = (SERVICE_DESCRIPTOR *)((ULONG_PTR)MasterTable - 0x40);
    }
    else {
        ShadowTable = (SERVICE_DESCRIPTOR *)((ULONG_PTR)MasterTable + 0x40);
        if (ShadowTable->Addrs != MasterTable->Addrs) {
            ShadowTable = (SERVICE_DESCRIPTOR *)((ULONG_PTR)MasterTable + 0xC0);
        }
        if (ShadowTable->Addrs != MasterTable->Addrs) {
            ShadowTable = (SERVICE_DESCRIPTOR *)((ULONG_PTR)MasterTable + 0x2C0);
        }
    }
    if (ShadowTable->Addrs != MasterTable->Addrs) {

        Log_Msg1(MSG_1113, L"SHADOW TABLE");
        ShadowTable = NULL;
    }

    // DbgPrint("MASTER %p SHADOW %p DIFF %p\n", MasterTable, ShadowTable, (ULONG)((ULONG_PTR)ShadowTable - (ULONG_PTR)MasterTable));

    return ShadowTable;
}


//---------------------------------------------------------------------------
// Syscall_GetIndexFromNtdll
//---------------------------------------------------------------------------


_FX ULONG Syscall_GetIndexFromNtdll(
    UCHAR *code, const UCHAR *name, ULONG name_len)
{
    ULONG index = -1;

    if (code[0] == 0x4C && code[1] == 0x8B && code[2] == 0xD1)
        code += 3;                          // skip mov r10, rcx

    if (*code == 0xB8)                      // mov eax, syscall number
        index = *(ULONG *)(code + 1);

    if (index == -1) {

        //
        // on 64-bit Windows, some syscalls are fake, and should be skipped
        //

        if (*code == 0xE9 && IS_PROC_NAME(15, "QuerySystemTime")) {

            index = -2;
        }
    }

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
    SERVICE_DESCRIPTOR *ShadowTable =
                            (SERVICE_DESCRIPTOR *)Syscall_GetServiceTable();
    if (ShadowTable) {

        ULONG MaxSyscallIndexPlusOne = ShadowTable->Limit;
        if ((index < 0x1000) &&
                        ((index & 0xFFF) < MaxSyscallIndexPlusOne)) {

            LONG_PTR EntryValue = (LONG_PTR)ShadowTable->Addrs[index];

            *pKernelAddr = (UCHAR *)ShadowTable->Addrs + (EntryValue >> 4);
            *pParamCount = (ULONG)(EntryValue & 0x0F) + 4;
            return TRUE;
        }

        Log_Msg1(MSG_1113, L"ADDRESS");
    }

    return FALSE;
}
