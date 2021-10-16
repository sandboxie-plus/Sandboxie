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
// Registry:  32-bit Code for Windows XP (Parse Procedure Hook)
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static void Key_Check_KB979683(const WCHAR *KbName);

static BOOLEAN Key_Init_XpHook(void);

static void Key_Unload_XpHook(void);

static NTSTATUS Key_MyParseProc(OBJ_PARSE_PROC_ARGS);

static void Key_HookWaitForSingleObject(void);


//---------------------------------------------------------------------------


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, Key_Check_KB979683)
#pragma alloc_text (INIT, Key_Init_XpHook)
#endif // ALLOC_PRAGMA


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static const WCHAR *Key_ObjectName = L"Key";
static OB_PARSE_METHOD Key_NtParseProc = NULL;
static ULONG_PTR Key_JumpStub = 0;
static BOOLEAN Key_ParseHooked = FALSE;

static ULONG *Key_WaitForSingleObject_Pid = NULL;


//---------------------------------------------------------------------------
// Key_Check_KB979683
//---------------------------------------------------------------------------


_FX void Key_Check_KB979683(const WCHAR *KbName)
{
    //
    // on Windows 2003 and later, both NtOpenKey and NtCreateKey
    // always send a ParseContext, with the first byte indicating
    // if this is NtOpenKey or NtCreateKey.
    //
    // on Windows 2000 and XP until hotfix KB979683, NtOpenKey never
    // sends a ParseContext, and NtCreateKey always sends a ParseContext.
    //
    // with hotfix KB979683 installed, Windows 2000/XP always send a
    // ParseContext, like Windows 2003, however the byte at offset 0x21
    // indicates if this is NtOpenKey or NtCreateKey.
    //

    if (Driver_OsVersion <= DRIVER_WINDOWS_XP) {

        NTSTATUS status;
        WCHAR path[128];
        UNICODE_STRING objname;
        OBJECT_ATTRIBUTES objattrs;
        IO_STATUS_BLOCK MyIoStatusBlock;
        HANDLE handle;

        wcscpy(path, Key_Registry_Machine);
        wcscat(path, L"\\SOFTWARE\\Microsoft\\Updates\\Windows ");
        if (Driver_OsVersion == DRIVER_WINDOWS_XP)
            wcscat(path, L"XP\\SP4");
        else
            wcscat(path, L"2000\\SP5");
        wcscat(path, KbName);

        RtlInitUnicodeString(&objname, path);

        InitializeObjectAttributes(
            &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

        status = ZwOpenKey(&handle, KEY_READ, &objattrs);
        if (! NT_SUCCESS(status)) {

            //
            // some computers might have a damaged registry that did not
            // record the registry key for the hotfix, in this case look
            // for the catalog file related to the hotfix
            //

            wcscpy(path, L"\\SystemRoot\\System32\\CatRoot"
                         L"\\{F750E6C3-38EE-11D1-85E5-00C04FC295EE}");
            wcscat(path, KbName);
            wcscat(path, L".cat");

            RtlInitUnicodeString(&objname, path);

            status = ZwCreateFile(
                &handle, FILE_GENERIC_READ, &objattrs, &MyIoStatusBlock,
                NULL, 0, FILE_SHARE_ANY, FILE_OPEN,
                FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                NULL, 0);
        }

        if (NT_SUCCESS(status)) {

            ZwClose(handle);

            Key_Have_KB979683 = TRUE;
        }
    }
}


//---------------------------------------------------------------------------
// Key_Init_XpHook
//---------------------------------------------------------------------------


_FX BOOLEAN Key_Init_XpHook(void)
{
    Key_Check_KB979683(L"\\KB979683");
    if (! Key_Have_KB979683) {
        // KB981852 installs a modified kernel just like KB979683, handle
        // the case where KB979683 is not there but KB981852 is there.
        Key_Check_KB979683(L"\\KB981852");
        if (! Key_Have_KB979683) {
            // same for KB2393802
            Key_Check_KB979683(L"\\KB2393802");
            if (! Key_Have_KB979683) {
                // same for KB2633171
                Key_Check_KB979683(L"\\KB2633171");
                if (! Key_Have_KB979683) {
                    // same for KB2676562
                    Key_Check_KB979683(L"\\KB2676562");
                    if (! Key_Have_KB979683) {
                        // same for KB2799494
                        Key_Check_KB979683(L"\\KB2799494");
                    }
                }
            }
        }
    }

    //
    // Registry notifications on Windows XP and Server 2003 are inadequate
    // because they do not pass any DesiredAccess fields, so we have to
    // use parse procedure hooks instead
    //

    if (! Obj_HookParseProc(Key_ObjectName,
                            Key_MyParseProc,
                            &Key_NtParseProc,
                            &Key_JumpStub))
        return FALSE;
    Key_ParseHooked = TRUE;

    if (Driver_CheckThirdParty(L"klif", SERVICE_KERNEL_DRIVER))
        Key_HookWaitForSingleObject();

    if (Driver_CheckThirdParty(L"SAVOnAccessControl",
                                            SERVICE_FILE_SYSTEM_DRIVER))
        Key_NeverUnmountHives = TRUE;

    return TRUE;
}


//---------------------------------------------------------------------------
// Key_Unload_XpHook
//---------------------------------------------------------------------------


_FX void Key_Unload_XpHook(void)
{
    if (Key_ParseHooked) {
        Process_DisableHookEntry(Key_JumpStub);
        Key_ParseHooked = FALSE;
    }

    if (Key_WaitForSingleObject_Pid)
        *Key_WaitForSingleObject_Pid = -1;
}


//---------------------------------------------------------------------------
// Key_MyParseProc
//---------------------------------------------------------------------------


_FX NTSTATUS Key_MyParseProc(OBJ_PARSE_PROC_ARGS)
{
    NTSTATUS status;
    PROCESS *proc = Process_GetCurrent();
    if (AccessMode != KernelMode) {
        if (proc != PROCESS_TERMINATED)
            status = CALL_PARSE_PROC_2(Key_MyParseProc_2);
        else
            status = STATUS_PROCESS_IS_TERMINATING;
    } else
        status = STATUS_SUCCESS;
    OBJ_CALL_SYSTEM_PARSE_PROC(Key_NtParseProc);
}


//---------------------------------------------------------------------------
// Key_HookWaitForSingleObject
//---------------------------------------------------------------------------


_FX void Key_HookWaitForSingleObject(void)
{
    static UCHAR HookBytes[] = {
        // 0xE8, xx,xx,xx,xx                // call  PsGetCurrentProcessId
        // 0xB9, xx,xx,xx,xx                // mov   ecx,Pid
        0x3B, 0x01,                         // cmp   eax,[ecx]
        0x75, 0x20,                         // jne   j1
        0x8B, 0x44, 0x24, 0x14,             // mov   eax,dword ptr [esp+5*4]
        0x85, 0xC0,                         // test  eax,eax
        0x74, 0x18,                         // jz    j1
        0x81, 0x38, 0x00, 0x40, 0x96, 0xD5, // cmp   [eax],0D5964000h
        0x75, 0x10,                         // jne   j1
        0x81, 0x78, 0x04, 0x36, 0xFF, 0xFF, 0xFF, // cmp [eax+4],0FFFFFF36h
        0x75, 0x07,                         // jne   j1
        0x83, 0xC1, 0x04,                   // add   ecx,4
        0x89, 0x4C, 0x24, 0x14,             // mov   [esp+5*4],ecx
        0xE9  // xx,xx,xx,xx                // j1:   jmp   xxxxxxxx
    };

    UCHAR *ptr, *tramp;

    //
    // on Windows XP with ZoneAlarm installed, we have to hook
    // KeWaitForSingleObject to intercept waits for 86400 seconds
    // occurring within the SbieSvc process
    //

    if (Driver_OsVersion != DRIVER_WINDOWS_XP)
        return;

    //
    // make sure KeWaitForSingleObject can be hot-patchable, and also
    // check if an earlier instance of this driver already hooked it
    //

    ptr = (void *)KeWaitForSingleObject;

    tramp = NULL;

    if (ptr[0] != 0x8B || ptr[1] != 0xFF ||
        (ptr[-5] != 0xCC && ptr[-5] != 0x90) ||
        (ptr[-4] != 0xCC && ptr[-4] != 0x90) ||
        (ptr[-3] != 0xCC && ptr[-3] != 0x90) ||
        (ptr[-2] != 0xCC && ptr[-2] != 0x90) ||
        (ptr[-1] != 0xCC && ptr[-1] != 0x90)) {

        if (*(USHORT *)ptr == 0xF9EB && ptr[-5] == 0xE9) {

            //
            // if KeWaitForSingleObject is hotpatched and jumps to
            // what looks like our hook (from a previously-loaded instance
            // of our driver), then it's ok to use it
            //

            tramp = ptr + *(ULONG *)(ptr - 4);
            if (*tramp != 0xE8 ||
                    memcmp(tramp + 10, HookBytes, sizeof(HookBytes)) != 0)
                tramp = NULL;
        }

        if (! tramp)
            return;
    }

    //
    // build our KeWaitForSingleObject replacement hook, which
    // is there to monitor for ZoneAlarm evil delays
    //

    if (! tramp) {
        tramp = Hook_BuildTramp(NULL, NULL, FALSE, FALSE);
        if (! tramp)
            return;
    }

    Key_WaitForSingleObject_Pid =
        (ULONG *)(&(HOOK_TRAMP_CODE_TO_TRAMP_HEAD(tramp))->size);
    Key_WaitForSingleObject_Pid[0] = -1;
    Key_WaitForSingleObject_Pid[1] = -5000000;    //     0.5 second
    Key_WaitForSingleObject_Pid[2] = 0xFFFFFFFF;  // (in 100 nano units)

    ptr = tramp;

    *ptr = 0xE8;                    // call  PsGetCurrentProcessId
    *(ULONG *)(ptr + 1) =
        (ULONG)((UCHAR *)PsGetCurrentProcessId - (ptr + 5));
    ptr += 5;

    *ptr = 0xB9;                    // mov   ecx,Pid
    *(ULONG *)(ptr + 1) = (ULONG)Key_WaitForSingleObject_Pid;
    ptr += 5;

    memcpy(ptr, HookBytes, sizeof(HookBytes));
    ptr += sizeof(HookBytes);
    *(ULONG *)ptr =                 // jmp   original_routine
        (ULONG)((UCHAR *)KeWaitForSingleObject + 2 - (ptr + 4));

    //
    // and finally hot-patch KeWaitForSingleObject
    //

    DisableWriteProtect();

    ptr = (void *)KeWaitForSingleObject;
    ptr[-5] = 0xE9;
    *(ULONG *)(ptr - 4) = (ULONG)(tramp - ptr);

    *(USHORT *)ptr = 0xF9EB;                // jmp short -5

    EnableWriteProtect();
}
