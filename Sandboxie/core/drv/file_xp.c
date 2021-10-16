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
// File System:  32-bit Code for Windows XP (Parse Procedure Hook)
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _OPEN_PACKET {

    CSHORT Type;
    CSHORT Size;
    PFILE_OBJECT FileObject;
    NTSTATUS FinalStatus;
    ULONG_PTR Information;
    ULONG ParseCheck;
    PFILE_OBJECT RelatedFileObject;
    LARGE_INTEGER AllocationSize;
    ULONG CreateOptions;
    USHORT FileAttributes;
    USHORT ShareAccess;
    PVOID EaBuffer;
    ULONG EaLength;
    ULONG Options;
    ULONG CreateDisposition;

} OPEN_PACKET;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN File_Init_XpHook(void);

static void File_Unload_XpHook(void);

static BOOLEAN File_CreateMyContext(
    PACCESS_STATE AccessState,
    KPROCESSOR_MODE AccessMode,
    PVOID Context,
    MY_CONTEXT *MyContext);

static NTSTATUS File_File_MyParseProc(OBJ_PARSE_PROC_ARGS);

static NTSTATUS File_Device_MyParseProc(OBJ_PARSE_PROC_ARGS);


//---------------------------------------------------------------------------


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, File_Init_XpHook)
#endif // ALLOC_PRAGMA


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static const WCHAR *File_File_ObjectName = L"File";
static OB_PARSE_METHOD File_File_NtParseProc = NULL;
static ULONG_PTR File_File_JumpStub = 0;
static BOOLEAN File_File_ParseHooked = FALSE;

static const WCHAR *File_Device_ObjectName = L"Device";
static OB_PARSE_METHOD File_Device_NtParseProc = NULL;
static ULONG_PTR File_Device_JumpStub = 0;
static BOOLEAN File_Device_ParseHooked = FALSE;


//---------------------------------------------------------------------------
// File_Init_XpHook
//---------------------------------------------------------------------------


_FX BOOLEAN File_Init_XpHook(void)
{
    //
    // hook parse procedures
    //

    if (! Obj_HookParseProc(File_File_ObjectName,
                            File_File_MyParseProc,
                            &File_File_NtParseProc,
                            &File_File_JumpStub)) {

        return FALSE;
    }

    File_File_ParseHooked = TRUE;

    if (! Obj_HookParseProc(File_Device_ObjectName,
                            File_Device_MyParseProc,
                            &File_Device_NtParseProc,
                            &File_Device_JumpStub)) {

        return FALSE;
    }

    File_Device_ParseHooked = TRUE;

    //
    // successful initialization
    //

    return TRUE;
}


//---------------------------------------------------------------------------
// File_Unload_XpHook
//---------------------------------------------------------------------------


_FX void File_Unload_XpHook(void)
{
    if (File_File_ParseHooked) {
        Process_DisableHookEntry(File_File_JumpStub);
        File_File_ParseHooked = FALSE;
    }

    if (File_Device_ParseHooked) {
        Process_DisableHookEntry(File_Device_JumpStub);
        File_Device_ParseHooked = FALSE;
    }
}


//---------------------------------------------------------------------------
// File_CreateMyContext
//---------------------------------------------------------------------------


_FX BOOLEAN File_CreateMyContext(
    PACCESS_STATE AccessState,
    KPROCESSOR_MODE AccessMode,
    PVOID Context,
    MY_CONTEXT *MyContext)
{
    //
    // first, prepare the MyContext structure
    //

    if (Context && ((OPEN_PACKET *)Context)->Type == IO_TYPE_OPEN_PACKET) {

        //
        // in Windows Vista, there are extra eight bytes in the Context
        // structure which cause both CreateOptions and CreateDisposition
        // fields to be offset by eight bytes
        //
        // the code tries to access offsets 0x20, 0x30 and 0x34 for XP, but
        // the eight byte offset makes this 0x28, 0x38 and 0x3C which works
        // for 32-bit Windows Vista, Windows 7, and Windows 8
        //

        MyContext->HaveContext = TRUE;

        /*if (Driver_OsVersion >= DRIVER_WINDOWS_VISTA)
            ((UCHAR *)Context) += 8;*/

        MyContext->CreateDisposition =
                                ((OPEN_PACKET *)Context)->CreateDisposition;

        MyContext->CreateOptions = ((OPEN_PACKET *)Context)->CreateOptions;

        MyContext->Options = ((OPEN_PACKET *)Context)->Options;

    } else {

        MyContext->HaveContext = FALSE;

        MyContext->CreateDisposition = 0;
        MyContext->CreateOptions = 0;
        MyContext->Options = 0;
    }

    MyContext->AccessMode = AccessMode;

    MyContext->OriginalDesiredAccess = AccessState->OriginalDesiredAccess;

    //
    // Return TRUE if we should intercept the request, FALSE otherwise:
    //
    // Normally MyParseProc does not intercept calls coming from kernel
    // mode.  The exception is during a file rename operation:
    // NtSetInformationFile will call IopOpenLinkOrRenameTarget which
    // will open the target directory, and we need to sandbox this access.
    // Luckily, IopOpenLinkOrRenameTarget seems to be the only bit of
    // kernel mode code that sets the open packet flag
    // IO_OPEN_TARGET_DIRECTORY, making it easy to identify this call.
    //

    if (MyContext->AccessMode != KernelMode)
        return TRUE;

    else if (MyContext->HaveContext &&
             MyContext->Options & IO_OPEN_TARGET_DIRECTORY)
        return TRUE;

    else
        return FALSE;
}


//---------------------------------------------------------------------------
// File_File_MyParseProc
//---------------------------------------------------------------------------


_FX NTSTATUS File_File_MyParseProc(OBJ_PARSE_PROC_ARGS)
{
    NTSTATUS status;
    MY_CONTEXT MyContext;
    PROCESS *proc = Process_GetCurrent();
    if (File_CreateMyContext(AccessState, AccessMode, Context, &MyContext)) {
        if (proc != PROCESS_TERMINATED) {
            FILE_OBJECT *file_object = (FILE_OBJECT *)ParseObject;
            status = File_Generic_MyParseProc(
                proc, ParseObject, file_object->DeviceObject->DeviceType,
                RemainingName, &MyContext, FALSE);
        } else
            status = STATUS_PROCESS_IS_TERMINATING;
    } else
        status = STATUS_SUCCESS;
    OBJ_CALL_SYSTEM_PARSE_PROC(File_File_NtParseProc);
}


//---------------------------------------------------------------------------
// File_Device_MyParseProc
//---------------------------------------------------------------------------


_FX NTSTATUS File_Device_MyParseProc(OBJ_PARSE_PROC_ARGS)
{
    NTSTATUS status;
    MY_CONTEXT MyContext;
    PROCESS *proc = Process_GetCurrent();
    if (File_CreateMyContext(AccessState, AccessMode, Context, &MyContext)) {
        if (proc != PROCESS_TERMINATED) {
            DEVICE_OBJECT *device_object = (DEVICE_OBJECT *)ParseObject;
            status = File_Generic_MyParseProc(
                proc, ParseObject, device_object->DeviceType,
                RemainingName, &MyContext, TRUE);
        } else
            status = STATUS_PROCESS_IS_TERMINATING;
    } else {
        status = STATUS_SUCCESS;
        if (AccessMode == KernelMode) {
            BOOLEAN bSetDirty = FALSE;
            DEVICE_OBJECT *device_object = (DEVICE_OBJECT *)ParseObject;
            File_ReplaceTokenIfFontRequest(
                AccessState, ParseObject, RemainingName, &bSetDirty);
        }
    }
    OBJ_CALL_SYSTEM_PARSE_PROC(File_Device_NtParseProc);
}
