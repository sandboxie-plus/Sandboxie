/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2021 David Xanatos, xanasoft.com
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
// File System
//---------------------------------------------------------------------------


#include "file.h"
#include "obj.h"
#include "api.h"
#include "conf.h"
#include "util.h"
#include "session.h"
#include "syscall.h"
#include "common/pattern.h"
#include "common/my_version.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define FILE_DENIED_ACCESS ~(                                           \
    STANDARD_RIGHTS_READ | GENERIC_READ | SYNCHRONIZE | READ_CONTROL |  \
    FILE_READ_DATA | FILE_READ_EA | FILE_READ_ATTRIBUTES | FILE_EXECUTE)

#define DIRECTORY_JUNCTION_ACCESS (                                     \
    GENERIC_ALL | GENERIC_WRITE | MAXIMUM_ALLOWED |                     \
    FILE_APPEND_DATA | FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES)


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _MY_CONTEXT {

    KPROCESSOR_MODE AccessMode;
    BOOLEAN HaveContext;
    ULONG CreateDisposition;
    ULONG CreateOptions;
    ULONG Options;
    ULONG OriginalDesiredAccess;

} MY_CONTEXT;


typedef struct _BLOCKED_DLL {

    LIST_ELEM list_elem;
    ULONG path_len;
    WCHAR path[4];      // padding bytes

} BLOCKED_DLL;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


extern void File_InitReparsePoints(BOOLEAN init);

static void File_CreateBoxPath_2(HANDLE FileHandle);

static void File_AdjustBoxFilePath(PROCESS *proc, HANDLE handle);

static BOOLEAN File_InitPaths(PROCESS *proc,
    LIST *open_file_paths, LIST *closed_file_paths,
    LIST *read_file_paths, LIST *write_file_paths);

static BOOLEAN File_BlockInternetAccess(PROCESS *proc);

static BOOLEAN File_BlockInternetAccess2(
    PROCESS *proc, const WCHAR *name, int modifier);

static NTSTATUS File_Generic_MyParseProc(
    PROCESS *proc, PVOID ParseObject, ULONG device_type,
    PUNICODE_STRING RemainingName, MY_CONTEXT *MyContext, BOOLEAN msg1313);

static NTSTATUS File_CreatePagingFile(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args);

static void File_ReplaceTokenIfFontRequest(
    ACCESS_STATE *AccessState,
    PDEVICE_OBJECT DeviceObject, UNICODE_STRING *FileName, BOOLEAN* pbSetDirty);

static NTSTATUS File_Api_SetShortName2(
    PROCESS *proc, OBJECT_ATTRIBUTES *objattrs,
    FILE_NAME_INFORMATION *name, ULONG name_len);


//---------------------------------------------------------------------------


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, File_Init)
#endif // ALLOC_PRAGMA


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


// Windows 2000 and Windows XP name for the LanmanRedirector device
       const WCHAR *File_Redirector = L"\\Device\\LanmanRedirector";
       const ULONG  File_RedirectorLen = 24;

// Windows Vista name for the LanmanRedirector device
       const WCHAR *File_MupRedir = L"\\Device\\Mup\\;LanmanRedirector";
       const ULONG  File_MupRedirLen = 29;

       const WCHAR *File_DfsClientRedir = L"\\Device\\Mup\\DfsClient";
       const ULONG  File_DfsClientRedirLen = 21;

       const WCHAR *File_HgfsRedir = L"\\Device\\Mup\\;hgfs";       // VMWare Host Guest File System
       const ULONG  File_HgfsRedirLen = 17;

       const WCHAR *File_Mup = L"\\Device\\Mup";
       const ULONG  File_MupLen = 11;

       const WCHAR *File_NamedPipe = L"\\Device\\NamedPipe";
       const ULONG  File_NamedPipeLen = 17;

static char *File_DesktopIniText = NULL;


static const WCHAR *File_RawIp = L"rawip6";
static const WCHAR *File_Http  = L"http\\";
static const WCHAR *File_Tcp   = L"tcp6";
static const WCHAR *File_Udp   = L"udp6";
static const WCHAR *File_Ip    = L"ip6";
static const WCHAR *File_Afd   = L"afd";
static const WCHAR *File_Nsi   = L"nsi";


//---------------------------------------------------------------------------
// Include code for minifilter
//---------------------------------------------------------------------------


#include "file_flt.c"


//---------------------------------------------------------------------------
// Include code for 32-bit Windows XP
//---------------------------------------------------------------------------


#ifndef _WIN64
#include "file_xp.c"
#endif _WIN64


//---------------------------------------------------------------------------
// File_Init
//---------------------------------------------------------------------------


_FX BOOLEAN File_Init(void)
{
    //
    // install parse procedure hook on Windows XP/2003
    // register as a filesystem minifilter on Vista and later
    //

    typedef BOOLEAN (*P_File_Init_2)(void);

    P_File_Init_2 p_File_Init_2 = File_Init_Filter;

#ifndef _WIN64

    if (Driver_OsVersion < DRIVER_WINDOWS_VISTA) {

        p_File_Init_2 = File_Init_XpHook;
    }

#endif ! _WIN64

    if (! p_File_Init_2())
        return FALSE;

    //
    // init reparse points
    //

    File_InitReparsePoints(TRUE);

    //
    // set syscalls handlers that are applicable in XP as well as Vista
    //

    if (! Syscall_Set1("CreatePagingFile", File_CreatePagingFile))
        return FALSE;

    //
    // set API functions
    //

    Api_SetFunction(API_RENAME_FILE,            File_Api_Rename);
    Api_SetFunction(API_GET_FILE_NAME,          File_Api_GetName);
    Api_SetFunction(API_REFRESH_FILE_PATH_LIST, File_Api_RefreshPathList);
    Api_SetFunction(API_OPEN_FILE,              File_Api_Open);
    Api_SetFunction(API_CHECK_INTERNET_ACCESS,  File_Api_CheckInternetAccess);
    Api_SetFunction(API_GET_BLOCKED_DLL,        File_Api_GetBlockedDll);

    return TRUE;
}


//---------------------------------------------------------------------------
// File_Unload
//---------------------------------------------------------------------------


_FX void File_Unload(void)
{
    //
    // uninstall parse procedure hook on Windows XP
    // deregister as a filesystem minifilter on Vista and later
    //

    typedef void (*P_File_Unload_2)(void);

    P_File_Unload_2 p_File_Unload_2 = File_Unload_Filter;

#ifndef _WIN64

    if (Driver_OsVersion < DRIVER_WINDOWS_VISTA) {

        p_File_Unload_2 = File_Unload_XpHook;
    }

#endif ! _WIn64

    p_File_Unload_2();

    //
    // unload reparse points
    //

    File_InitReparsePoints(FALSE);
}


//---------------------------------------------------------------------------
// File_CreateBoxPath
//---------------------------------------------------------------------------


_FX BOOLEAN File_CreateBoxPath(PROCESS *proc)
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE handle;
    WCHAR *ptr;
    ULONG retries;

    //
    // we should only have to check the path for the first process in
    // any hierarchy of processes, any child processes should benefit
    // from cloning the parent box with the adjusted path
    //

    if (proc->parent_was_sandboxed)
        return TRUE;

    InitializeObjectAttributes(&objattrs,
        &objname, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL, Driver_PublicSd);

    //
    // main loop:  try to create sandbox path, removing last path component
    // each time we get STATUS_OBJECT_PATH_NOT_FOUND
    //

    ptr = NULL;

    retries = 0;
    while (retries < 64) {

        ++retries;
        RtlInitUnicodeString(&objname, proc->box->file_path);

        status = ZwCreateFile(
            &handle,
            FILE_GENERIC_READ | FILE_WRITE_ATTRIBUTES,
            &objattrs,
            &IoStatusBlock,
            NULL,                   // AllocationSize
            0,                      // FileAttributes
            FILE_SHARE_VALID_FLAGS, // ShareAccess
            FILE_OPEN_IF,           // CreateDisposition
            FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
            NULL, 0);               // EaBuffer, EaLength

        if (status == STATUS_OBJECT_PATH_NOT_FOUND) {

            //
            // if parent directories are missing, terminate the
            // path at the last backslash and retry the loop
            //

            WCHAR *ptr2 = wcsrchr(proc->box->file_path, L'\\');
            if (ptr)
                *ptr = L'\\';
            ptr = ptr2;
            if (ptr)
                *ptr = L'\0';

        } else {

            //
            // if we could create the path, we check if any backslashes
            // were zeroed out.  if so, we restart the loop from the top
            // using the full path.  otherwise, we're done
            //

            if (NT_SUCCESS(status)) {

                if (IoStatusBlock.Information == FILE_CREATED)
                    File_CreateBoxPath_2(handle);

                if (! ptr) {
                    // after we opened the full sandbox path, we adjust
                    // the box path to account for any reparse points
                    File_AdjustBoxFilePath(proc, handle);
                }

                ZwClose(handle);
            }

            if (ptr) {

                *ptr = L'\\';
                ptr = NULL;

            } else
                break;
        }
    }

    if (retries >= 64)
        status = STATUS_UNSUCCESSFUL;

    if (! NT_SUCCESS(status)) {
		Log_Status_Ex_Process(MSG_FILE_CREATE_BOX_PATH, 0, status, proc->box->file_path, -1, proc->pid);
    }

    return (NT_SUCCESS(status));
}


//---------------------------------------------------------------------------
// File_CreateBoxPath_2
//---------------------------------------------------------------------------


_FX void File_CreateBoxPath_2(HANDLE FileHandle)
{
    static char *_Tip =
        "This folder is a work area created by the program "
        MY_PRODUCT_NAME_STRING ".\r\n";

    static char *_DesktopIni =
        "IconIndex=8\r\n"
        "InfoTip=";

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE handle;
    FILE_BASIC_INFORMATION info;

    //
    // prepare desktop.ini file contents
    //

    if (! File_DesktopIniText) {

        File_DesktopIniText = Mem_Alloc(Driver_Pool, 768);
        if (File_DesktopIniText) {

            RtlStringCbPrintfA(File_DesktopIniText, 768, 
                        "[.ShellClassInfo]\r\n"
                        "IconFile=%S\\%S\r\n"
                        "IconIndex=9\r\n"
                        "IconResource=%S\\%S,9\r\n"
                        "InfoTip=%s",
                        Driver_HomePathDos + 4, SBIECTRL_EXE,
                        Driver_HomePathDos + 4, SBIECTRL_EXE,
                        _Tip);
        }
    }

    InitializeObjectAttributes(&objattrs,
        &objname, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        FileHandle, Driver_PublicSd);

    //
    // hide the newly created sandbox directory
    //

    status = ZwQueryInformationFile(
        FileHandle, &IoStatusBlock,
        &info, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);

    if (NT_SUCCESS(status)) {

        info.FileAttributes &= ~(FILE_ATTRIBUTE_NORMAL |
                                 FILE_ATTRIBUTE_HIDDEN |
                                 FILE_ATTRIBUTE_SYSTEM);
        info.FileAttributes |= FILE_ATTRIBUTE_READONLY;
        status = ZwSetInformationFile(
            FileHandle, &IoStatusBlock,
            &info, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);
    }

    //
    // create the desktop.ini file
    //

    if (File_DesktopIniText) {

        RtlInitUnicodeString(&objname, L"desktop.ini");

        status = ZwCreateFile(
            &handle,
            FILE_GENERIC_READ | FILE_GENERIC_WRITE,
            &objattrs,
            &IoStatusBlock,
            NULL,                   // AllocationSize
            FILE_ATTRIBUTE_HIDDEN | // FileAttributes
            FILE_ATTRIBUTE_SYSTEM,
            FILE_SHARE_VALID_FLAGS, // ShareAccess
            FILE_OPEN_IF,           // CreateDisposition
            FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
            NULL, 0);               // EaBuffer, EaLength

        if (NT_SUCCESS(status) &&
                IoStatusBlock.Information == FILE_CREATED) {

            status = ZwWriteFile(
                        handle, NULL, NULL, NULL, &IoStatusBlock,
                        File_DesktopIniText, strlen(File_DesktopIniText),
                        NULL, NULL);

            ZwClose(handle);
        }
    }

    //
    // create a warning text file
    //

    RtlInitUnicodeString(&objname, L"DONT-USE.TXT");

    status = ZwCreateFile(
        &handle,
        FILE_GENERIC_READ | FILE_GENERIC_WRITE,
        &objattrs,
        &IoStatusBlock,
        NULL,                   // AllocationSize
        0,                      // FileAttributes
        FILE_SHARE_VALID_FLAGS, // ShareAccess
        FILE_OPEN_IF,           // CreateDisposition
        FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
        NULL, 0);               // EaBuffer, EaLength

    if (NT_SUCCESS(status) && IoStatusBlock.Information == FILE_CREATED) {

        status = ZwWriteFile(handle, NULL, NULL, NULL, &IoStatusBlock,
                             _Tip, strlen(_Tip), NULL, NULL);

        ZwClose(handle);
    }
}


//---------------------------------------------------------------------------
// File_AdjustBoxFilePath
//---------------------------------------------------------------------------


_FX void File_AdjustBoxFilePath(PROCESS *proc, HANDLE handle)
{
    NTSTATUS status;
    FILE_OBJECT *object;

    status = ObReferenceObjectByHandle(
        handle, 0, *IoFileObjectType, KernelMode, &object, NULL);

    if (NT_SUCCESS(status)) {

        OBJECT_NAME_INFORMATION *Name = NULL;
        ULONG NameLength;

        Obj_GetName(proc->pool, object, &Name, &NameLength);

        if (Name && (Name != &Obj_Unnamed)) {

            BOX *box = proc->box;
            WCHAR *path     = Name->Name.Buffer;
            ULONG  path_len = wcslen(path);

            if (path_len != box->file_path_len
                    || _wcsicmp(path, box->file_path) != 0) {

                //
                // semi-atomically replace the file_path field
                //

                path_len = (path_len + 1) * sizeof(WCHAR);

                InterlockedExchange(&box->file_path_len, 0);
                InterlockedExchangePointer(&box->file_path, path);
                InterlockedExchange(&box->file_path_len, path_len);
            }
        }

        ObDereferenceObject(object);
    }
}


//---------------------------------------------------------------------------
// File_InitPaths
//---------------------------------------------------------------------------


_FX BOOLEAN File_InitPaths(PROCESS *proc,
    LIST *open_file_paths, LIST *closed_file_paths,
    LIST *read_file_paths, LIST *write_file_paths)
{
    static const WCHAR *_PstPipe = L"\\Device\\NamedPipe\\protected_storage";
    static const WCHAR *_OpenFile = L"OpenFilePath";
    static const WCHAR *_OpenPipe = L"OpenPipePath";
    static const WCHAR *_ClosedPath = L"ClosedFilePath";
    static const WCHAR *_ReadPath = L"ReadFilePath";
    static const WCHAR *_WritePath = L"WriteFilePath";
    static const WCHAR *openpipes[] = {
        L"\\Device\\NamedPipe\\",               // named pipe root
        L"\\Device\\MailSlot\\",                // mail slot root
        //
        // network
        //
        L"\\Device\\NamedPipe\\ROUTER",
        L"\\Device\\NamedPipe\\ShimViewer",
        L"\\Device\\Afd",
        L"\\Device\\Afd\\Endpoint",
        L"\\Device\\Afd\\AsyncConnectHlp",
        L"\\Device\\Afd\\AsyncSelectHlp",
        L"\\Device\\Afd\\ROUTER",
        L"\\Device\\WS2IFSL",
        L"\\Device\\WS2IFSL\\NifsPvd",
        L"\\Device\\WS2IFSL\\NifsSct",
        L"\\Device\\Tcp",
        L"\\Device\\Tcp6",
        L"\\Device\\Ip",
        L"\\Device\\Ip6",
        L"\\Device\\Udp",
        L"\\Device\\Udp6",
        L"\\Device\\RawIp",
        L"\\Device\\RawIp6",
        L"\\Device\\NetBT_Tcpip_*",
        L"\\Device\\Http\\*",
        L"\\Device\\Nsi",                           // Windows 7
        //
        // Windows 7 fault-tolerant heap
        //
        L"\\Device\\NamedPipe\\ProtectedPrefix\\LocalService\\FTHPIPE",
        //
        // printer
        //
        L"\\Device\\NamedPipe\\spoolss",
        L"\\Device\\NamedPipe\\spooler*",
        L"%DefaultSpoolDirectory%\\*",
        L"%DefaultSpoolDirectory2%\\*",
        L"\\Device\\NamedPipe\\*_doPDF*",           // doPDF
        //
        // multimedia
        //
        L"\\Device\\NamedPipe\\AudioSrv",
        //
        // third-party software
        //
        L"\\Device\\NamedPipe\\Adobe LM Service*",
        L"\\Device\\NamedPipe\\XTIERRPCPIPE",       // Novell NetIdentity
        NULL
    };
    static const WCHAR *strWinRMFiles[] = {
        // Windows Remote Management (WinRM) is a large security hole.  A sandboxed app running in an elevated cmd shell can send any admin command to the host.
        // Block the WinRS.exe and the automation dlls to make it very difficult for someone to use.
        // See ICD-10136 "Sandboxie security hole allows guest to run any command in host as admin"
        //
        L"%SystemRoot%\\System32\\wsmsvc.dll",
        L"%SystemRoot%\\System32\\wsmauto.dll",
        L"%SystemRoot%\\System32\\winrs.exe",
        // Don't forget the WoW64 files
        L"%SystemRoot%\\SysWoW64\\wsmsvc.dll",
        L"%SystemRoot%\\SysWoW64\\wsmauto.dll",
        L"%SystemRoot%\\SysWoW64\\winrs.exe",
        // Note: This is not a proper fix its just a cheap mitidation!!! 
        NULL
    };

    BOOLEAN ok;
    ULONG i;

    //
    // open paths
    //

    ok = Process_GetPaths(proc, open_file_paths, _OpenPipe, TRUE);
    if (! ok) {
        Log_MsgP1(MSG_INIT_PATHS, _OpenPipe, proc->pid);
        return FALSE;
    }

    if (! proc->image_from_box) {

        ok = Process_GetPaths(proc, open_file_paths, _OpenFile, TRUE);

        if (! ok) {
            Log_MsgP1(MSG_INIT_PATHS, _OpenFile, proc->pid);
            return FALSE;
        }
    }

    if (ok && Conf_Get_Boolean(
                proc->box->name, Driver_OpenProtectedStorage, 0, FALSE)) {
        ok = Process_AddPath(
                proc, open_file_paths, NULL, TRUE, _PstPipe, FALSE);
    }

    for (i = 0; openpipes[i] && ok; ++i) {
        ok = Process_AddPath(
            proc, open_file_paths, NULL, TRUE, openpipes[i], FALSE);
    }

    if (! ok) {
        Log_MsgP1(MSG_INIT_PATHS, _OpenPipe, proc->pid);
        return FALSE;
    }

    //
    // closed paths
    //

    ok = Process_GetPaths(proc, closed_file_paths, _ClosedPath, TRUE);
    if (ok) {
        // the LanmanRedirector/Mup devices (when accessed without extra paths)
        // is a security attack, and must be closed
        ok = Process_AddPath(proc, closed_file_paths, NULL,
                             TRUE, File_Redirector, FALSE);
        if (ok) {
            ok = Process_AddPath(proc, closed_file_paths, NULL,
                                 TRUE, File_Mup, FALSE);
        }
    }

    if(Conf_Get_Boolean(proc->box->name, L"BlockWinRM", 0, TRUE))
    for (i = 0; strWinRMFiles[i] && ok; ++i) {
        ok = Process_AddPath(proc, closed_file_paths, _ClosedPath, TRUE, strWinRMFiles[i], FALSE);
    }

    if (! ok) {
        Log_MsgP1(MSG_INIT_PATHS, _ClosedPath, proc->pid);
        return FALSE;
    }

    //
    // read-only paths (stored also as open paths)
    //

    ok = Process_GetPaths(proc, open_file_paths, _ReadPath, TRUE);
    if (ok)
        ok = Process_GetPaths(proc, read_file_paths, _ReadPath, TRUE);
    if (! ok) {
        Log_MsgP1(MSG_INIT_PATHS, _ReadPath, proc->pid);
        return FALSE;
    }

    //
    // write-only paths (stored also as closed paths)
    //

    ok = Process_GetPaths2(
            proc, write_file_paths, closed_file_paths,
            _WritePath, TRUE);
    if (ok) {
        ok = Process_GetPaths(
                proc, closed_file_paths, _WritePath, TRUE);
    }
    if (! ok) {
        Log_MsgP1(MSG_INIT_PATHS, _WritePath, proc->pid);
        return FALSE;
    }

    //
    // if this is a Sandboxie program (like SandboxieRpcSs), don't allow
    // the Windows compatibility (shim) DLLs to be loaded
    //

    if (proc->image_sbie) {

        static const WCHAR *_ShimEng =
                                L"%SystemRoot%\\System32\\shimeng.dll";
        static const WCHAR *_AppHelp =
                                L"%SystemRoot%\\System32\\apphelp.dll";

        ok = Process_AddPath(proc, closed_file_paths, _ClosedPath,
                             TRUE, _ShimEng, FALSE);
        if (ok) {
            ok = Process_AddPath(proc, closed_file_paths, _ClosedPath,
                                 TRUE, _AppHelp, FALSE);
        }
    }

    //
    // finish
    //

    return TRUE;
}


//---------------------------------------------------------------------------
// File_BlockInternetAccess
//---------------------------------------------------------------------------


_FX BOOLEAN File_BlockInternetAccess(PROCESS *proc)
{
    BOOLEAN is_open, is_closed;
    BOOLEAN ok;

    //
    // is this process excempted from the blocade
    //

	if (proc->AllowInternetAccess)
		return TRUE;

    //
    // should we warn on access to internet resources
    //

    proc->file_warn_internet = Conf_Get_Boolean(
        proc->box->name, L"NotifyInternetAccessDenied", 0, TRUE);

    if (proc->image_sbie)
        proc->file_warn_internet = FALSE;

    //
    // add Internet devices if ClosedFilePath=InternetAccessDevices
    //

    Process_MatchPath(
        proc->pool, L"InternetAccessDevices", 21,
        NULL, &proc->closed_file_paths,
        &is_open, &is_closed);

    ok = TRUE;

    if (is_closed) {

        //
        // if the configuration specifies to block the pseudo Internet
        // device, add real Internet devices.  keep in sync with the
        // list of devices in File_Api_CheckInternetAccess
        //

        if (ok)         // \Device\RawIp6
            ok = File_BlockInternetAccess2(proc, File_RawIp, 0);

        if (ok)         // \Device\RawIp4
            ok = File_BlockInternetAccess2(proc, File_RawIp, -1);

        if (ok)         // \Device\Http\*
            ok = File_BlockInternetAccess2(proc, File_Http, +1);

        if (ok)         // \Device\Tcp6
            ok = File_BlockInternetAccess2(proc, File_Tcp, 0);

        if (ok)         // \Device\Tcp4
            ok = File_BlockInternetAccess2(proc, File_Tcp, -1);

        if (ok)         // \Device\Udp6
            ok = File_BlockInternetAccess2(proc, File_Udp, 0);

        if (ok)         // \Device\Udp4
            ok = File_BlockInternetAccess2(proc, File_Udp, -1);

        if (ok)         // \Device\Ip6
            ok = File_BlockInternetAccess2(proc, File_Ip, 0);

        if (ok)         // \Device\Ip4
            ok = File_BlockInternetAccess2(proc, File_Ip, -1);

        if (ok)         // \Device\Afd*
            ok = File_BlockInternetAccess2(proc, File_Afd, +1);

        if (ok)         // \Device\Nsi
            ok = File_BlockInternetAccess2(proc, File_Nsi, 0);
    }

    return ok;
}


//---------------------------------------------------------------------------
// File_BlockInternetAccess2
//---------------------------------------------------------------------------


_FX BOOLEAN File_BlockInternetAccess2(
    PROCESS *proc, const WCHAR *name, int modifier)
{
    WCHAR device_name[32];
    BOOLEAN add_star;

    wmemcpy(device_name,    File_Mup, 8);   // \Device\ prefix
    wcscpy(device_name + 8, name);

    if (modifier == +1)
        add_star = TRUE;
    else {
        add_star = FALSE;

        if (modifier == -1) {
            WCHAR *ptr = device_name + wcslen(device_name) - 1;
            *ptr = L'\0';
        }
    }

    return Process_AddPath(
        proc, &proc->closed_file_paths, NULL, TRUE, device_name, add_star);
}


//---------------------------------------------------------------------------
// File_InitProcess
//---------------------------------------------------------------------------


_FX BOOLEAN File_InitProcess(PROCESS *proc)
{
    BOOLEAN ok = File_InitPaths(proc,   &proc->open_file_paths,
                                        &proc->closed_file_paths,
                                        &proc->read_file_paths,
                                        &proc->write_file_paths);
    if (ok)
        ok = File_BlockInternetAccess(proc);

    if (ok) {

        //
        // check if should we warn on direct access to disk devices
        //

        proc->file_warn_direct_access = Conf_Get_Boolean(
                    proc->box->name, L"NotifyDirectDiskAccess", 0, FALSE);
    }

    proc->file_open_devapi_cmapi = Conf_Get_Boolean(proc->box->name, L"OpenDevCMApi", 0, FALSE);

    if (ok && proc->image_path && (! proc->image_sbie)) {

        //
        // make sure the image path does not match a ClosedFilePath setting
        //

        BOOLEAN is_open, is_closed;
        Process_MatchPath(
            proc->pool, proc->image_path, wcslen(proc->image_path),
            NULL, &proc->closed_file_paths,
            &is_open, &is_closed);

        if (is_closed)
            ok = FALSE;
    }

    return ok;
}


//---------------------------------------------------------------------------
// File_IsDelayLoadDll
//---------------------------------------------------------------------------


_FX BOOLEAN File_IsDelayLoadDll(PROCESS *proc, const WCHAR *DllName)
{
    BOOLEAN retval = FALSE;
    ULONG idx = 0;

    Conf_AdjustUseCount(TRUE);

    while (1) {
        const WCHAR *value = Conf_Get(proc->box->name, L"DelayLoadDll", idx);
        if (! value)
            break;
        //DbgPrint("Comparing <%S> vs <%S>\n", DllName, value);
        if (_wcsicmp(value, DllName) == 0) {
            retval = TRUE;
            break;
        }
        ++idx;
    }

    Conf_AdjustUseCount(FALSE);

    return retval;
}


//---------------------------------------------------------------------------
// File_Generic_MyParseProc
//---------------------------------------------------------------------------


_FX NTSTATUS File_Generic_MyParseProc(
    PROCESS *proc, PVOID ParseObject, ULONG device_type,
    PUNICODE_STRING RemainingName, MY_CONTEXT *MyContext, BOOLEAN msg1313)
{
    static const WCHAR *_Device         = L"\\Device\\";
    static const ULONG  _DeviceLen      = 8;
    static const WCHAR *_NamedPipe      = L"NamedPipe";
    static const ULONG  _NamedPipeLen   = 9;
    static const WCHAR *_MailSlot       = L"MailSlot";
    static const ULONG  _MailSlotLen    = 8;
    NTSTATUS status;
    OBJECT_NAME_INFORMATION *Name;
    ULONG NameLength;
    WCHAR *PipeName;
    BOOLEAN IsPipeDevice;
    BOOLEAN IsBoxedPath;
    BOOLEAN ShouldMonitorAccess;
    BOOLEAN write_access;
    ULONG DesiredAccess;
    ULONG CreateDisposition;
    ULONG CreateOptions;
    ULONG file_trace;
    ULONG MonitorPrefixLen;
    WCHAR *MonitorSuffixPtr;

    //
    // skip requests dealing with devices we don't care about
    //

    if (device_type != FILE_DEVICE_DISK &&
        device_type != FILE_DEVICE_NAMED_PIPE &&
        device_type != FILE_DEVICE_MAILSLOT &&
        device_type != FILE_DEVICE_NETWORK &&
        device_type != FILE_DEVICE_MULTI_UNC_PROVIDER &&
        device_type != FILE_DEVICE_NETWORK_FILE_SYSTEM &&
        device_type != FILE_DEVICE_DFS)
    {
        if ((proc->file_trace & TRACE_IGNORE) || Session_MonitorCount) {

            ULONG ignore_str_len;
            WCHAR *ignore_str;
            WCHAR *device_name_ptr;

            status = Obj_GetParseName(
                proc->pool, ParseObject, RemainingName, &Name, &NameLength);
            if (NT_SUCCESS(status))
                device_name_ptr = Name->Name.Buffer;
            else
                device_name_ptr = Obj_Unnamed.Name.Buffer;

            ignore_str_len = (wcslen(device_name_ptr) + 24) * sizeof(WCHAR);
            ignore_str = Mem_Alloc(proc->pool, ignore_str_len);
            if (ignore_str) {

                RtlStringCbPrintfW(ignore_str, ignore_str_len,
                    L"(FI) %08X %s", device_type, device_name_ptr);

                if (proc->file_trace & TRACE_IGNORE)
                    Log_Debug_Msg(MONITOR_IGNORE, ignore_str, Driver_Empty);

                else if (Session_MonitorCount &&
                        device_type != FILE_DEVICE_PHYSICAL_NETCARD)
                    Session_MonitorPut(MONITOR_IGNORE, ignore_str + 4, proc->pid);

                Mem_Free(ignore_str, ignore_str_len);
            }

            if (Name && Name != &Obj_Unnamed)
                Mem_Free(Name, NameLength);
        }

        return STATUS_SUCCESS;
    }

    //
    // figure out the complete path.  deny access to unnamed objects.
    //

    status = Obj_GetParseName(
        proc->pool, ParseObject, RemainingName, &Name, &NameLength);
    if (Name == &Obj_Unnamed)
        status = STATUS_ACCESS_DENIED;

    if (! NT_SUCCESS(status)) {

        //
        // special case:  unnamed object which cannot be parsed due
        // to a STATUS_OBJECT_PATH_INVALID error is most likely an
        // anonymous pipe, so pass it through to the system
        //

        if (status == STATUS_OBJECT_PATH_INVALID &&
                ((! RemainingName) || RemainingName->Length == 0)) {

            status = STATUS_SUCCESS;
        }

        return status;
    }

    //
    // check if the specified path leads inside the box.  and if this
    // is an unsandboxed NamedPipe or MailSlot, we need to remember
    // the name of the specific pipe
    //

    IsPipeDevice = FALSE;
    IsBoxedPath  = FALSE;
    PipeName = NULL;

    ShouldMonitorAccess = FALSE;

    MonitorPrefixLen = 0;
    MonitorSuffixPtr = NULL;

    if (device_type == FILE_DEVICE_NAMED_PIPE ||
        device_type == FILE_DEVICE_MAILSLOT) {

        IsPipeDevice = TRUE;

        //
        // make sure path begins with \Device\ prefix
        //

        if (_wcsnicmp(Name->Name.Buffer, _Device, _DeviceLen) == 0) {

            UNICODE_STRING uni;
            WCHAR *path2 = Name->Name.Buffer + _DeviceLen;

            //
            // if path continues with NamedPipe\ or MailSlot\ prefix,
            // we check if the path is sandboxed
            //

            if (_wcsnicmp(path2, _NamedPipe, _NamedPipeLen) == 0)
                PipeName = path2 + _NamedPipeLen;
            else if (_wcsnicmp(path2, _MailSlot, _MailSlotLen) == 0)
                PipeName = path2 + _MailSlotLen;

            if (PipeName) {

                //
                // if the path has the \Device\NamedPipe\ or
                // \Device\MailSlot\ prefix (with a backslashes following
                // the device name), then check if the path is sandboxed.
                // if so, we don't care about PipeName, reset it to NULL
                //

                if (*PipeName == L'\\') {

                    ++PipeName;
                    RtlInitUnicodeString(&uni, PipeName);
                    if (Box_IsBoxedPath(proc->box, pipe, &uni)) {
                        MonitorPrefixLen =
                            (ULONG)(ULONG_PTR)(PipeName - Name->Name.Buffer);
                        MonitorSuffixPtr = PipeName +
                            proc->box->pipe_path_len / sizeof(WCHAR);
                        IsBoxedPath = TRUE;
                        PipeName = NULL;
                    }

                //
                // if the path is just \Device\NamedPipe or
                // \Device\MailSlot (no backslash), then PipeName will
                // point to the null character, which is good (see below)
                //
                // Otherwise, we this is not the NamedPipe or MailSlot
                // device at all, so reset PipeName to NULL
                //

                } else if (*PipeName != L'\0')
                    PipeName = NULL;
            }
        }

    } else if (Box_IsBoxedPath(proc->box, file, &Name->Name))
        IsBoxedPath = TRUE;

    //
    // check the desired access and creation flags in the open packet.
    // named pipes are always considered opened for write access, because
    // they can be used to drive server programs outside the sandbox
    //

    write_access = FALSE;
    DesiredAccess = MyContext->OriginalDesiredAccess;

    if (IsPipeDevice) {

        write_access = TRUE;

        //
        // if this is a read access to the NamedPipe directory itself
        // (as used by Win32 WaitNamedPipe API), then allow
        //

        if (PipeName && (*PipeName == L'\0') &&
                (DesiredAccess & FILE_DENIED_ACCESS) == 0) {

            write_access = FALSE;
        }

    } else {

        if (DesiredAccess & FILE_DENIED_ACCESS)
            write_access = TRUE;
    }

    if (MyContext->HaveContext) {

        CreateDisposition = MyContext->CreateDisposition;
        if (CreateDisposition != FILE_OPEN)
            write_access = TRUE;

        CreateOptions = MyContext->CreateOptions;
        if (CreateOptions & FILE_DELETE_ON_CLOSE)
            write_access = TRUE;

    } else {

        //
        // this usually happens when the parse procedure is invoked by
        // NtOpenSymbolicLinkObject
        //

        CreateDisposition = -1;
        CreateOptions = -1;
    }

    //
    // allow/deny access to files
    //

    status = STATUS_SUCCESS;

    if (! IsBoxedPath) {

        //
        // allow/deny access to paths outside the sandbox
        //

        LIST *open_file_paths;
        BOOLEAN is_open, is_closed;

        WCHAR *path = Name->Name.Buffer;
        ULONG path_len = Name->Name.Length / sizeof(WCHAR);

        WCHAR *temp_path, *temp_ptr;

        KIRQL irql;

        //
        // if the path contains a colon that indicates an NTFS
        // alternate data stream, create a temporary path without the colon
        //

        temp_path = NULL;

        temp_ptr = wcsrchr(path, L'\\');
        if (temp_ptr) {
            temp_ptr = wcschr(temp_ptr, L':');
            if (temp_ptr) {

                temp_path = Mem_Alloc(proc->pool, Name->Name.Length);
                if (temp_path) {
                    memcpy(temp_path, path, Name->Name.Length);

                    path_len = (ULONG)(ULONG_PTR)(temp_ptr - path);
                    path = temp_path;
                    path[path_len] = L'\0';
                }
            }
        }

        //
        // give read-only access to Sandboxie home folder,
        // disregarding any settings that might affect it
        //

        if (path_len >= Driver_HomePathNt_Len
                && (path[Driver_HomePathNt_Len] == L'\\' ||
                    path[Driver_HomePathNt_Len] == L'\0')
                && 0 == Box_NlsStrCmp(
                    path, Driver_HomePathNt, Driver_HomePathNt_Len)) {

            if (write_access)
                status = STATUS_ACCESS_DENIED;
            else
                status = STATUS_SUCCESS;

            goto skip_due_to_home_folder;
        }

        //
        // take lock
        //

        KeRaiseIrql(APC_LEVEL, &irql);
        ExAcquireResourceSharedLite(proc->file_lock, TRUE);

        //
        // deny access in two cases:
        // - if unsandboxed path matches a closed path
        // - if unsandboxed path does not match an open path,
        //   and this is a write access
        //

        if (write_access)
            open_file_paths = &proc->open_file_paths;
        else
            open_file_paths = NULL;

        Process_MatchPath(
            proc->pool, path, path_len,
            open_file_paths, &proc->closed_file_paths,
            &is_open, &is_closed);

        if ((! is_open) && (! is_closed)) {

    //
    // if we have a path that looks like any of these
    // \Device\LanmanRedirector\server\shr\f1.txt
    // \Device\LanmanRedirector\;Q:000000000000b09f\server\shr\f1.txt
    // \Device\Mup\;LanmanRedirector\server\share\f1.txt
    // \Device\Mup\;LanmanRedirector\;Q:000000000000b09f\server\share\f1.txt
    // then translate to
    // \Device\Mup\server\shr\f1.txt
    // and test again.  We do this because open/closed paths are
    // recorded in the \Device\Mup format.  See File_TranslateShares.
    //

            ULONG PrefixLen;
            if (_wcsnicmp(path, File_Redirector, File_RedirectorLen) == 0)
                PrefixLen = File_RedirectorLen;
            else if (_wcsnicmp(path, File_MupRedir, File_MupRedirLen) == 0)
                PrefixLen = File_MupRedirLen;
            else if (_wcsnicmp(path, File_DfsClientRedir, File_DfsClientRedirLen) == 0)
                PrefixLen = File_DfsClientRedirLen;
            else if (_wcsnicmp(path, File_HgfsRedir, File_HgfsRedirLen) == 0)
                PrefixLen = File_HgfsRedirLen;
            else if (_wcsnicmp(path, File_Mup, File_MupLen) == 0)
                PrefixLen = File_MupLen;
            else
                PrefixLen = 0;

            if (PrefixLen && path[PrefixLen]     == L'\\' &&
                             path[PrefixLen + 1] != L'\0') {

                const WCHAR *ptr = path + PrefixLen;
                if (ptr[1] == L';')
                    ptr = wcschr(ptr + 2, L'\\');

                if (ptr && ptr[0] && ptr[1]) {

                    ULONG len1   = wcslen(ptr + 1);
                    ULONG len2   = (File_MupLen + len1 + 8) * sizeof(WCHAR);
                    WCHAR *path2 = Mem_Alloc(proc->pool, len2);
                    if (! path2)
                        status = STATUS_ACCESS_DENIED;
                    else {

                        wmemcpy(path2, File_Mup, File_MupLen);
                        path2[File_MupLen] = L'\\';
                        wmemcpy(path2 + File_MupLen + 1, ptr + 1, len1 + 1);
                        len1 += File_MupLen + 1;

                        Process_MatchPath(
                            proc->pool, path2, len1,
                            open_file_paths, &proc->closed_file_paths,
                            &is_open, &is_closed);

                        if (write_access && is_open) {

                            //
                            // read-only paths are also listed as open
                            // paths, so if we granted write access to an
                            // open path,  we need to check that this
                            // isn't also a read-only path
                            //

                            Process_MatchPath(
                                proc->pool, path2, len1,
                                &proc->read_file_paths, NULL,
                                &is_open, &is_closed);

                            if (is_open) {  // match on OpenFilePath
                                            // and also ReadFilePath
                                is_open = FALSE;
                                is_closed = TRUE;

                            } else {        // match on OpenFilePath
                                            // but not ReadFilePath
                                is_open = TRUE;
                            }
                        }

                        //DbgPrint("MyParseProc %S, desired = %x, type = %x\n", path2, MyContext->OriginalDesiredAccess, device_type);
                        if ((MyContext->OriginalDesiredAccess != FILE_READ_ATTRIBUTES) &&
                            (MyContext->OriginalDesiredAccess != SYNCHRONIZE) &&
                            !is_open && Conf_Get_Boolean(proc->box->name, L"BlockNetworkFiles", 0, FALSE))
                        {
                            if (!write_access)
                            {
                                // see if path is open for reading
                                Process_MatchPath(
                                    proc->pool, path2, len1,
                                    &proc->open_file_paths, NULL,
                                    &is_open, &is_closed);
                            }
                            if (!is_open)
                                is_closed = TRUE;
                        }

                        Mem_Free(path2, len2);
                    }
                }
            }
        }

        if (is_closed || (write_access && (! is_open))) {

            status = STATUS_ACCESS_DENIED;

            if (is_closed)
                ShouldMonitorAccess = TRUE;
        }

        //
        // read-only paths are also listed as open paths, so if we granted
        // write access to an open path, we need to check that this isn't
        // also a read-only path
        //

        if (write_access && is_open) {

            Process_MatchPath(
                proc->pool, path, path_len,
                &proc->read_file_paths, NULL,
                &is_open, &is_closed);

            if (is_open) {                  // match on OpenFilePath
                                            // and also ReadFilePath
                status = STATUS_ACCESS_DENIED;
                ShouldMonitorAccess = TRUE;
            }
        }

        //
        // release lock
        //

        ExReleaseResourceLite(proc->file_lock);
        KeLowerIrql(irql);

        //
        // if process is trying to load a special DLL before SbieDll
        // has been initialized, then pretend the file does not exist,
        // and add the DLL path so it can be loaded by SbieDll
        //

        if ((! proc->sbiedll_loaded) && status == STATUS_SUCCESS
                && (CreateOptions & FILE_DIRECTORY_FILE) == 0) {

            WCHAR *backslash = wcsrchr(path, L'\\');
            if (backslash && File_IsDelayLoadDll(proc, backslash + 1)) {

                ULONG len = sizeof(BLOCKED_DLL) + path_len * sizeof(WCHAR);
                BLOCKED_DLL *blk = Mem_Alloc(proc->pool, len);
                if (blk) {

                    blk->path_len = path_len;
                    wmemcpy(blk->path, path, path_len + 1);

                    KeRaiseIrql(APC_LEVEL, &irql);
                    ExAcquireResourceExclusiveLite(proc->file_lock, TRUE);

                    List_Insert_After(&proc->blocked_dlls, NULL, blk);

                    ExReleaseResourceLite(proc->file_lock);
                    KeLowerIrql(irql);
                }

                status = STATUS_OBJECT_NAME_NOT_FOUND;
            }
        }

        //
        // release temporary path
        //

skip_due_to_home_folder:

        if (temp_path)
            Mem_Free(temp_path, Name->Name.Length);

    } else {

        //
        // allow/deny access to paths inside the sandbox
        //

        /*
        // This code breaks Win 10 1903 (see my comments in file_flt.c in File_PreOperation). 1903 now sets the FILE_WRITE_DATA bit
        // (which is part of DIRECTORY_JUNCTION_ACCESS) on a rename, which causes this code to set status = STATUS_ACCESS_DENIED.
        // Since we check for links outside the sandbox in File_PreOperation, I don't believe this code is necessary so I am commenting it out for now.

        if (! IsPipeDevice) {

            //
            // we can't allow write access on sandbox directories as this
            // can be used to create a directory reparse point (junction) in
            // a call to NtFsControlFile.  instead, SbieDll will use the
            // SbieSvc FileServer proxy to adjust attributes on directories.
            //
            // note that the the sandbox cannot be circumvented by this, but
            // deleting the sandbox (recursively) would delete the contents
            // of the target of the junction.
            //
            // exception:  if this is a rename request coming through
            // IopOpenLinkOrRenameTarget (see above), allow write access
            // to the sandboxed target directory
            //

            if (! (MyContext->Options & IO_OPEN_TARGET_DIRECTORY)) {

                if ((DesiredAccess & DIRECTORY_JUNCTION_ACCESS) && (
                         (CreateOptions & FILE_NON_DIRECTORY_FILE) == 0 ||
                         (CreateOptions & FILE_DIRECTORY_FILE) != 0)) {

                    status = STATUS_ACCESS_DENIED;
                }
            }
        }
        */
    }

    //
    // trace the request if so desired
    //

    if (IsPipeDevice)
        file_trace = proc->pipe_trace;
    else
        file_trace = proc->file_trace;

    if (file_trace & (TRACE_ALLOW | TRACE_DENY)) {

        WCHAR access_str[48];
        WCHAR letter;

        if ((! NT_SUCCESS(status)) && (file_trace & TRACE_DENY))
            letter = L'D';
        else if (NT_SUCCESS(status) && (file_trace & TRACE_ALLOW))
            letter = L'A';
        else
            letter = 0;

        if (letter) {

            ULONG mon_type = IsPipeDevice ? MONITOR_PIPE : MONITOR_FILE;
            if (!IsBoxedPath) {
                if (ShouldMonitorAccess == TRUE)
                    mon_type |= MONITOR_DENY;
                else
                    mon_type |= MONITOR_OPEN;
            }
            if(!IsPipeDevice && !ShouldMonitorAccess)
                mon_type |= MONITOR_TRACE;

            RtlStringCbPrintfW(access_str, sizeof(access_str), L"(F%c) %08X.%02X.%08X",
                letter, DesiredAccess,
                CreateDisposition & 0x0F, CreateOptions);
            Log_Debug_Msg(mon_type, access_str, Name->Name.Buffer);
        }
    }

    else if (IsPipeDevice && Session_MonitorCount) {

        ULONG mon_type = MONITOR_PIPE;
        WCHAR *mon_name = Name->Name.Buffer;

        if (MonitorPrefixLen && MonitorSuffixPtr) {
            WCHAR *ptr = mon_name + MonitorPrefixLen;
            wmemmove(ptr, MonitorSuffixPtr, wcslen(MonitorSuffixPtr) + 1);
        } else if (NT_SUCCESS(status))
            mon_type |= MONITOR_OPEN;
        else
            mon_type |= MONITOR_DENY;
        Session_MonitorPut(mon_type, mon_name, proc->pid);

    } else if (ShouldMonitorAccess) {

        Session_MonitorPut(MONITOR_FILE | MONITOR_DENY, Name->Name.Buffer, proc->pid);

    } 
    
    if (!ShouldMonitorAccess && msg1313 
                       && status == STATUS_ACCESS_DENIED
                       && device_type == FILE_DEVICE_DISK
                       && RemainingName && RemainingName->Length == 0) {

        // this block can only be reached when (! ShouldMonitorAccess)
        // which means we never issue SBIE1313 for paths that match
        // ClosedFilePath or ReadFilePath

        if (proc->file_warn_direct_access) {

            //Log_MsgP1(MSG_BLOCKED_DIRECT_DISK_ACCESS, proc->image_name, proc->pid);
            Process_LogMessage(proc, MSG_BLOCKED_DIRECT_DISK_ACCESS);
        }
    }

    Mem_Free(Name, NameLength);

    return status;
}


//---------------------------------------------------------------------------
// File_CreatePagingFile
//---------------------------------------------------------------------------


_FX NTSTATUS File_CreatePagingFile(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args)
{
    return STATUS_PRIVILEGE_NOT_HELD;
}


//---------------------------------------------------------------------------
// File_ReplaceTokenIfFontRequest
//---------------------------------------------------------------------------


_FX void File_ReplaceTokenIfFontRequest(
    ACCESS_STATE *AccessState,
    PDEVICE_OBJECT DeviceObject, UNICODE_STRING *FileName, BOOLEAN* pbSetDirty)
{
    //
    // CreateFontIndirectExW (and family of functions) seems to only create
    // a font descriptor but font data is not loaded until it is actually
    // used, for example by GetTextMetrics.  at time of use, control will
    // reach win32k!bCreateSection which will try to open the font file,
    // and will fail due to the highly restricted primary process token.
    //
    // the above is also true for getting a font through GetStockObject,
    // and there also seems to be a scenario where win32k will occasionally
    // release data for a previously loaded font, and will load it at some
    // later point when the font is used again, which means that even if
    // we use font creation helper hooks, win32k!bCreateSection can still
    // execute at any random time.
    //
    // to work around this, we check if a kernel mode caller is running
    // in the context of a non-impersonated thread, which belongs to a
    // process in the sandbox, and if this is read-only access to a font
    // file.  if true, we replace the PrimaryToken in the ACCESS_STATE
    // structure for the call, to make sure the request is successful.
    //
    // (note that win32k!bCreateSection uses DesiredAccess = 0x001200A9)
    //

    const ULONG _DesiredAccess = SYNCHRONIZE | READ_CONTROL
                               | FILE_READ_ATTRIBUTES | FILE_EXECUTE
                               | FILE_READ_EA | FILE_READ_DATA;
    PROCESS *proc;
    WCHAR *ptr;
    ULONG len, i;

    if (AccessState->SubjectSecurityContext.ClientToken)
        return;                                 // if active impersonation

    if (AccessState->OriginalDesiredAccess & (~_DesiredAccess))
        return;                                 // if not specific rights

    proc = Process_Find(PsGetCurrentProcessId(), NULL);
    if (! proc)
        return;                                 // if not sandboxed
    if (! proc->primary_token)
        return;                                 // if not restricted token

    //
    // check if the path references the Fonts folder
    //

    if (! FileName)
        return;
    if (! FileName->Buffer)
        return;

    ptr = FileName->Buffer;
    len = FileName->Length / sizeof(WCHAR);
    for (i = 0; i < len; ++i) {
        if (ptr[i] != L'\\')
            continue;
        if (i + 7 < len && _wcsnicmp(&ptr[i], L"\\FONTS\\", 7) == 0)
            break;
    }

    if (i == len) {

        //
        // a CreateScalableResource call doesn't access the Fonts folder,
        // but the Gdi_CreateScalableFontResourceW hook in core/dll/gdi.c
        // sends us a path inside the sandbox
        //

        BOOLEAN IsBoxedPath = FALSE;

        if (DeviceObject) {

            OBJECT_NAME_INFORMATION *Name;
            ULONG NameLength;

            NTSTATUS status = Obj_GetParseName(
                proc->pool, DeviceObject, FileName, &Name, &NameLength);
            if (NT_SUCCESS(status)) {

                if (Box_IsBoxedPath(proc->box, file, &Name->Name))
                    IsBoxedPath = TRUE;

                if (Name && Name != &Obj_Unnamed)
                    Mem_Free(Name, NameLength);
            }

            //DbgPrint("%d - BoxPath <%S> - File <%S>\n", IsBoxedPath, proc->box->file_path, FileName->Buffer);
        }

        if (! IsBoxedPath)
            return;
    }

    //
    // Using impersonation token in ClientToken if it is available
    // Replacing the primary token caused BSOD with Digital Guardian when dereferencing the token
    //

    ObReferenceObject(proc->primary_token);

    if (!AccessState->SubjectSecurityContext.ClientToken)
    {
        AccessState->SubjectSecurityContext.ClientToken = proc->primary_token;

        if (AccessState->SubjectSecurityContext.ImpersonationLevel < SecurityImpersonation)
        {
            AccessState->SubjectSecurityContext.ImpersonationLevel = SecurityImpersonation;
        }
    }
    else // fall back to old way
    {
        AccessState->SubjectSecurityContext.PrimaryToken = proc->primary_token;
    }

    *pbSetDirty = TRUE;
}


//---------------------------------------------------------------------------
// File_Api_Rename
//---------------------------------------------------------------------------


_FX NTSTATUS File_Api_Rename(PROCESS *proc, ULONG64 *parms)
{
    API_RENAME_FILE_ARGS *args = (API_RENAME_FILE_ARGS *)parms;
    NTSTATUS status;
    UNICODE_STRING64 *user_uni;
    WCHAR *user_dir, *user_name;
    ULONG user_dir_len, user_name_len;
    WCHAR *path, *name;
    FILE_RENAME_INFORMATION *info;
    ULONG path_len, name_len, info_len;
    WCHAR save_char;
    HANDLE dir_handle;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN is_open, is_closed;
    KIRQL irql;

    //
    // this API must be invoked by a sandboxed process
    //

    if (! proc)
        return STATUS_NOT_IMPLEMENTED;

    //
    // probe user target path parameter (target dir and target name)
    //

    user_uni = args->target_dir.val;
    if (! user_uni)
        return STATUS_INVALID_PARAMETER;
    ProbeForRead(user_uni, sizeof(UNICODE_STRING64), sizeof(ULONG64));

    user_dir = (WCHAR *)user_uni->Buffer;
    user_dir_len = user_uni->Length & ~1;
    if ((! user_dir) || (! user_dir_len) || (user_dir_len > 32000))
        return STATUS_INVALID_PARAMETER;
    ProbeForRead(user_dir, user_dir_len, sizeof(WCHAR));

    user_uni = args->target_name.val;
    if (! user_uni)
        return STATUS_INVALID_PARAMETER;
    ProbeForRead(user_uni, sizeof(UNICODE_STRING64), sizeof(ULONG64));

    user_name = (WCHAR *)user_uni->Buffer;
    user_name_len = user_uni->Length & ~1;
    if ((! user_name) || (! user_name_len) || (user_name_len > 32000))
        return STATUS_INVALID_PARAMETER;
    ProbeForRead(user_name, user_name_len, sizeof(WCHAR));

    //
    // copy user parameters into consolidated buffer:  dir"\"name
    //

    path_len = user_dir_len + user_name_len + sizeof(WCHAR) * 8;
    path = Mem_Alloc(proc->pool, path_len);
    if (! path)
        return STATUS_INSUFFICIENT_RESOURCES;
    memzero(path, path_len);

    memcpy(path, user_dir, user_dir_len);
    name = path + wcslen(path);
    *name = L'\\';
    memcpy(&name[1], user_name, user_name_len);

    if (wcschr(&name[1], L'\\'))
        return STATUS_INVALID_PARAMETER;

    //
    // check if the full target path is an open path, and stop if not
    //

    KeRaiseIrql(APC_LEVEL, &irql);
    ExAcquireResourceSharedLite(proc->file_lock, TRUE);

    Process_MatchPath(
        proc->pool, path, wcslen(path),
        &proc->open_file_paths, &proc->closed_file_paths,
        &is_open, &is_closed);

    if ((! is_open) || is_closed) {

        ExReleaseResourceLite(proc->file_lock);
        KeLowerIrql(irql);

        Mem_Free(path, path_len);
        return STATUS_BAD_INITIAL_PC;
    }

    //
    // check if the target directory is an open path, and stop if it is
    //

    *name = L'\0';

    Process_MatchPath(
        proc->pool, path, wcslen(path),
        &proc->open_file_paths, &proc->closed_file_paths,
        &is_open, &is_closed);

    ExReleaseResourceLite(proc->file_lock);
    KeLowerIrql(irql);

    if (is_open || is_closed) {

        Mem_Free(path, path_len);
        return STATUS_BAD_INITIAL_PC;
    }

    //
    // now we have established that the full target path name is an
    // open path, but the parent directory in that path isn't open.
    // therefore we will open the parent directory for write access
    // from kernel mode, and do the rename here
    //
    // we put a the trailing backslash on the path, so that we can open
    // the parent directory even when the parent is the root directory
    //

    save_char = name[1];
    name[0] = L'\\';
    name[1] = L'\0';

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    RtlInitUnicodeString(&objname, path);

    status = ZwCreateFile(
        &dir_handle, FILE_GENERIC_WRITE, &objattrs,
        &IoStatusBlock, NULL, 0, FILE_SHARE_VALID_FLAGS,
        FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

    if (! NT_SUCCESS(status)) {
        Mem_Free(path, path_len);
        return status;
    }

    //
    // allocate an information buffer, and issue rename request
    //

    ++name;
    *name = save_char;
    name_len = wcslen(name) * sizeof(WCHAR);

    info_len = sizeof(FILE_RENAME_INFORMATION) + name_len + 8;
    info = Mem_Alloc(proc->pool, info_len);
    if (! info)
        status = STATUS_INSUFFICIENT_RESOURCES;
    else {

        memzero(info, info_len);
        info->ReplaceIfExists = args->replace_if_exists.val;
        info->RootDirectory = dir_handle;
        info->FileNameLength = name_len;
        memcpy(info->FileName, name, name_len);


		FILE_OBJECT *object;
		status = ObReferenceObjectByHandle(args->file_handle.val, 0L, *IoFileObjectType, UserMode, (PVOID)&object, NULL);

		if (NT_SUCCESS(status)) {

			HANDLE handle;
			status = ObOpenObjectByPointer((PVOID)object, OBJ_FORCE_ACCESS_CHECK |
				OBJ_KERNEL_HANDLE, NULL, GENERIC_ALL, *IoFileObjectType, KernelMode, &handle);

			if (NT_SUCCESS(status)) {

				status = ZwSetInformationFile(
					handle, &IoStatusBlock, //args->file_handle.val, &IoStatusBlock,
					info, info_len, FileRenameInformation);

				ZwClose(handle);
			}

			ObDereferenceObject(object);
		}

        // FIXME, we may get STATUS_NOT_SAME_DEVICE, however, in most cases,
        // this API call is used to rename a file inside a folder, rather
        // than move files across folders, so that isn't a problem

        Mem_Free(info, info_len);
    }

    NtClose(dir_handle);
    Mem_Free(path, path_len);
    return status;
}


//---------------------------------------------------------------------------
// File_Api_GetName
//---------------------------------------------------------------------------


_FX NTSTATUS File_Api_GetName(PROCESS *proc, ULONG64 *parms)
{
    API_GET_FILE_NAME_ARGS *args = (API_GET_FILE_NAME_ARGS *)parms;
    FILE_OBJECT *object;
    NTSTATUS status;

    status = ObReferenceObjectByHandle(
        args->handle.val, 0, *IoFileObjectType, UserMode, &object, NULL);

    if (NT_SUCCESS(status)) {

        /*DbgPrint("Handle %08X DeviceObject %08X (Type %08X) ObjectName=<%*.*S>\n",
            args->handle.val, object->DeviceObject,
            object->DeviceObject ? object->DeviceObject->DeviceType : -1,
            object->FileName.Length / sizeof(WCHAR), object->FileName.Length / sizeof(WCHAR), object->FileName.Buffer);*/

        if (    (! object->DeviceObject)
            ||  (! object->FileName.Length)
            ||  (! object->FileName.Buffer)) {

            ObDereferenceObject(object);
            status = STATUS_BAD_DEVICE_TYPE;
        }
    }

    if (NT_SUCCESS(status)) {

        POOL *pool = proc ? proc->pool : Driver_Pool;
        OBJECT_NAME_INFORMATION *Name = NULL;
        ULONG NameLength, len;
        WCHAR *user;

        __try {

            if (object->DeviceObject->DeviceType == FILE_DEVICE_DISK) {

                //
                // normal case for a file on a disk
                //

                status = Obj_GetName(pool, object, &Name, &NameLength);
                if (NT_SUCCESS(status) && (Name == &Obj_Unnamed)) {
                    Name = NULL;
                    status = STATUS_OBJECT_PATH_NOT_FOUND;
                }

                if (NT_SUCCESS(status)) {

                    len = Name->Name.Length
                        + sizeof(WCHAR) * 2;  // null padding
                    if (len > args->name_len.val)
                        status = STATUS_BUFFER_TOO_SMALL;
                    else {

                        user = args->name_buf.val;
                        ProbeForWrite(user, len, sizeof(WCHAR));

                        memcpy(user, Name->Name.Buffer, Name->Name.Length);
                        user += Name->Name.Length / sizeof(WCHAR);
                        *user = L'\0';

                        status = STATUS_SUCCESS;

                        //DbgPrint("Result DISK:  %S\n", args->name_buf.val);
                    }
                }

            } else {

                //
                // not file or not on disk
                //

                status = Obj_GetName(
                    pool, object->DeviceObject, &Name, &NameLength);
                if (NT_SUCCESS(status) && (Name == &Obj_Unnamed)) {
                    Name = NULL;
                    status = STATUS_OBJECT_PATH_NOT_FOUND;
                }

                if (NT_SUCCESS(status)) {

                    len = Name->Name.Length
                        + object->FileName.Length
                        + sizeof(WCHAR) * 2;  // null padding
                    if (len > args->name_len.val)
                        status = STATUS_BUFFER_TOO_SMALL;
                    else {

                        user = args->name_buf.val;
                        ProbeForWrite(user, len, sizeof(WCHAR));

                        memcpy(user, Name->Name.Buffer, Name->Name.Length);
                        user += Name->Name.Length / sizeof(WCHAR);
                        memcpy(user, object->FileName.Buffer,
                                     object->FileName.Length);
                        user += object->FileName.Length / sizeof(WCHAR);
                        *user = L'\0';

                        status = STATUS_SUCCESS;

                        //DbgPrint("Result MISC:  %S\n", args->name_buf.val);
                    }
                }
            }

        } __except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
        }

        if (Name)
            Mem_Free(Name, NameLength);

        ObDereferenceObject(object);
    }

    return status;
}


//---------------------------------------------------------------------------
// File_Api_RefreshPathList
//---------------------------------------------------------------------------


_FX NTSTATUS File_Api_RefreshPathList(PROCESS *proc, ULONG64 *parms)
{
    NTSTATUS status;
    LIST open_paths,    *p_open_paths;
    LIST closed_paths,  *p_closed_paths;
    LIST read_paths,    *p_read_paths;
    LIST write_paths,   *p_write_paths;
    PATTERN *pat;
    BOOLEAN ok;
    KIRQL irql;

    //
    // this API must be invoked by a sandboxed process
    //

    if (! proc)
        return STATUS_NOT_IMPLEMENTED;

    //
    // build a new path list in a temporary space
    //

    List_Init(&open_paths);
    List_Init(&closed_paths);
    List_Init(&read_paths);
    List_Init(&write_paths);

    ok = File_InitPaths(proc,
            &open_paths, &closed_paths, &read_paths, &write_paths);

    //
    // select which set of path lists to delete, the currently active set
    // in case of success, or the partially created new set on failure
    //

    KeRaiseIrql(APC_LEVEL, &irql);
    ExAcquireResourceExclusiveLite(proc->file_lock, TRUE);

    if (ok) {

        p_open_paths    = &proc->open_file_paths;
        p_closed_paths  = &proc->closed_file_paths;
        p_read_paths    = &proc->read_file_paths;
        p_write_paths   = &proc->write_file_paths;

    } else {

        p_open_paths    = &open_paths;
        p_closed_paths  = &closed_paths;
        p_read_paths    = &read_paths;
        p_write_paths   = &write_paths;

    }

    //
    // delete the selected path lists
    //

    while (1) {
        pat = List_Head(p_open_paths);
        if (! pat)
            break;
        List_Remove(p_open_paths, pat);
        Pattern_Free(pat);
    }

    while (1) {
        pat = List_Head(p_closed_paths);
        if (! pat)
            break;
        List_Remove(p_closed_paths, pat);
        Pattern_Free(pat);
    }

    while (1) {
        pat = List_Head(p_read_paths);
        if (! pat)
            break;
        List_Remove(p_read_paths, pat);
        Pattern_Free(pat);
    }

    while (1) {
        pat = List_Head(p_write_paths);
        if (! pat)
            break;
        List_Remove(p_write_paths, pat);
        Pattern_Free(pat);
    }

    //
    // restore old path lists, in case of failure
    //

    if (ok) {

        memcpy(&proc->open_file_paths,    &open_paths,      sizeof(LIST));
        memcpy(&proc->closed_file_paths,  &closed_paths,    sizeof(LIST));
        memcpy(&proc->read_file_paths,    &read_paths,      sizeof(LIST));
        memcpy(&proc->write_file_paths,   &write_paths,     sizeof(LIST));
	}

	//
	// now we need to re block the internet access
	//

	if (ok)
		ok = File_BlockInternetAccess(proc);

	if (ok) {

        status = STATUS_SUCCESS;

    } else {

        status = STATUS_UNSUCCESSFUL;
    }

    ExReleaseResourceLite(proc->file_lock);
    KeLowerIrql(irql);

    return status;
}


//---------------------------------------------------------------------------
// File_Api_Open
//---------------------------------------------------------------------------


_FX NTSTATUS File_Api_Open(PROCESS *proc, ULONG64 *parms)
{
    API_OPEN_FILE_ARGS *args = (API_OPEN_FILE_ARGS *)parms;
    ULONG path_len;
    WCHAR *path;
    HANDLE *user_handle;
    NTSTATUS status;
    HANDLE handle;
    UNICODE_STRING objname;
    OBJECT_ATTRIBUTES objattrs;
    IO_STATUS_BLOCK IoStatusBlock;
    ULONG DesiredAccess;
    ULONG CreateOptions;
    ULONG AccessCheckOptions;
    KIRQL irql;
    BOOLEAN is_open, is_closed;

    //
    // this API must be invoked by a sandboxed process on Windows XP
    //

    if (! proc)
        return STATUS_NOT_IMPLEMENTED;

    //
    // check and capture parameters
    //

    user_handle = args->file_handle.val;
    if (! user_handle)
        return STATUS_INVALID_PARAMETER;

    ProbeForWrite(user_handle, sizeof(HANDLE), sizeof(HANDLE));

    path_len = args->path_len.val & ~1;
    if ((! path_len) || (path_len > 512 * sizeof(WCHAR)))
        return STATUS_INVALID_PARAMETER;

    ProbeForRead(args->path_str.val, path_len, sizeof(WCHAR));

    path = Mem_Alloc(proc->pool, path_len + 8);
    memcpy(path, args->path_str.val, path_len);
    path[path_len / sizeof(WCHAR)] = L'\0';

    //
    // if the path matches a ClosedFilePath setting, we only allow opening
    // directory files, and only for query attribute access;  this is used
    // by File_NtQueryFullAttributesFile in SbieDll
    //

    DesiredAccess = FILE_GENERIC_READ;
    CreateOptions = FILE_SYNCHRONOUS_IO_NONALERT;
    AccessCheckOptions =
        IO_FORCE_ACCESS_CHECK | IO_IGNORE_SHARE_ACCESS_CHECK;

    KeRaiseIrql(APC_LEVEL, &irql);
    ExAcquireResourceSharedLite(proc->file_lock, TRUE);

    Process_MatchPath(
        proc->pool, path, wcslen(path),
        NULL, &proc->closed_file_paths,
        &is_open, &is_closed);

    ExReleaseResourceLite(proc->file_lock);
    KeLowerIrql(irql);

    if (is_closed) {

        DesiredAccess  = FILE_READ_ATTRIBUTES | SYNCHRONIZE;
        CreateOptions |= FILE_DIRECTORY_FILE;
    }

    if (proc->file_trace & (TRACE_ALLOW | TRACE_DENY)) {

        WCHAR access_str[48];
        WCHAR letter;

        if (is_closed && (proc->file_trace & TRACE_DENY))
            letter = L'D';
        else if (proc->file_trace & TRACE_ALLOW)
            letter = L'A';
        else
            letter = 0;

        if (letter) {

            ULONG mon_type = MONITOR_FILE;
            mon_type |= MONITOR_TRACE;

            RtlStringCbPrintfW(access_str, sizeof(access_str), L"(F%c) %08X.%02X.%08X",
                letter, DesiredAccess,
                0 & 0x0F, CreateOptions);
            Log_Debug_Msg(mon_type, access_str, path);
        }
    }
    else if (is_closed) {

        Session_MonitorPut(MONITOR_FILE | MONITOR_DENY, path, proc->pid);
    }


    //
    // for a named pipe in the sandbox, use other parameters for the
    // call to IoCreateFileSpecifyDeviceObjectHint.  this is necessary
    // for some esoteric cases of the Chrome restricted sandbox process
    //

    /*if (path_len > File_NamedPipeLen + 1 &&
        _wcsnicmp(path, File_NamedPipe, File_NamedPipeLen) == 0 &&
        path[File_NamedPipeLen] == L'\\' && (! is_closed)) {

        RtlInitUnicodeString(&objname, path + File_NamedPipeLen + 1);
        if (Box_IsBoxedPath(proc->box, pipe, &objname)) {

            DesiredAccess = GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE
                          | FILE_READ_ATTRIBUTES;
            CreateOptions = FILE_OPEN_NO_RECALL | FILE_NON_DIRECTORY_FILE;
            AccessCheckOptions = 0;
        }
    }*/

    //
    // issue open request and finish
    //

    RtlInitUnicodeString(&objname, path);

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = IoCreateFileSpecifyDeviceObjectHint(
            &handle, DesiredAccess, &objattrs, &IoStatusBlock,
            NULL, 0, FILE_SHARE_VALID_FLAGS, FILE_OPEN, CreateOptions,
            NULL, 0, CreateFileTypeNone, NULL, AccessCheckOptions, NULL);

    Mem_Free(path, path_len + 8);

    if (NT_SUCCESS(status))
        *user_handle = handle;

    return status;
}


//---------------------------------------------------------------------------
// File_Api_CheckInternetAccess
//---------------------------------------------------------------------------


_FX NTSTATUS File_Api_CheckInternetAccess(PROCESS *proc, ULONG64 *parms)
{
    API_CHECK_INTERNET_ACCESS_ARGS *args =
        (API_CHECK_INTERNET_ACCESS_ARGS *)parms;
    WCHAR *user_devname;
    WCHAR device_name[42];
    WCHAR *ptr, *ptr2;
    ULONG len;
    HANDLE ProcessId;
    NTSTATUS status;
    KIRQL irql;
    BOOLEAN chk;

    //
    // get the device name specified in the request
    //

    user_devname = args->device_name.val;
    if (! user_devname)
        return STATUS_INVALID_PARAMETER;
    ProbeForRead(user_devname, sizeof(WCHAR) * 32, sizeof(WCHAR));
    wmemcpy(device_name,        File_Mup,     8);   // \Device\ prefix
    wmemcpy(device_name + 8,    user_devname, 32);

    //
    // convert the device name to lowercase, stop at the first backslash
    //

    ptr = device_name + 8;
    ptr[32] = L'\0';
    len = 0;
    for (ptr2 = ptr; *ptr2; ++ptr2) {
        if (*ptr2 == L'\\') {
            *ptr2 = L'\0';
            break;
        }
        if (*ptr2 >= L'A' && *ptr2 <= L'Z')
            *ptr2 = (*ptr2 - L'A') + L'a';
        ++len;
    }

    //
    // check if this is an internet device
    //

    chk = FALSE;
    if (len == 6) {

        if (wmemcmp(ptr, File_RawIp, 6) == 0)
            chk = TRUE;

    } else if (len == 5) {

        if (wmemcmp(ptr, File_RawIp, 5) == 0)
            chk = TRUE;

    } else if (len == 4) {

        if (wmemcmp(ptr, File_Http, 4) == 0)
            chk = TRUE;
        if (wmemcmp(ptr, File_Tcp, 4) == 0)
            chk = TRUE;
        else if (wmemcmp(ptr, File_Udp, 4) == 0)
            chk = TRUE;

    } else if (len == 3) {

        if (wmemcmp(ptr, File_Tcp, 3) == 0)
            chk = TRUE;
        else if (wmemcmp(ptr, File_Udp, 3) == 0)
            chk = TRUE;
        else if (wmemcmp(ptr, File_Ip, 3) == 0)
            chk = TRUE;
        else if (wmemcmp(ptr, File_Afd, 3) == 0)
            chk = TRUE;
        else if (wmemcmp(ptr, File_Nsi, 3) == 0)
            chk = TRUE;

    } else if (len == 2) {

        if (wmemcmp(ptr, File_Ip, 2) == 0)
            chk = TRUE;
    }

    if (! chk)
        return STATUS_OBJECT_NAME_INVALID;

    //
    // if a ProcessId was specified, then locate and lock the matching
    // process. ProcessId must be specified if the caller is not sandboxed
    //

    ProcessId = args->process_id.val;
    if (proc) {
        if (ProcessId == proc->pid || IS_ARG_CURRENT_PROCESS(ProcessId))
            ProcessId = 0;  // don't have to search for the current pid
    } else {
        if ((! ProcessId) || IS_ARG_CURRENT_PROCESS(ProcessId))
            return STATUS_INVALID_CID;
    }

    if (ProcessId) {

        proc = Process_Find(ProcessId, &irql);
        if ((! proc) || proc->terminated) {
            ExReleaseResourceLite(Process_ListLock);
            KeLowerIrql(irql);
            return STATUS_INVALID_CID;
        }
    }

    //
    // check file access restrictions
    //

    if (1) {

        BOOLEAN is_open, is_closed;
        KIRQL irql2;

        KeRaiseIrql(APC_LEVEL, &irql2);
        ExAcquireResourceSharedLite(proc->file_lock, TRUE);

        Process_MatchPath(
            proc->pool, device_name, wcslen(device_name),
            NULL, &proc->closed_file_paths,
            &is_open, &is_closed);

        ExReleaseResourceLite(proc->file_lock);
        KeLowerIrql(irql2);

        if (is_closed) {

            status = STATUS_ACCESS_DENIED;

            if (proc->file_warn_internet && args->issue_message.val) {

                //
                // issue message SBIE1307 when Internet restrictions apply
                //

                Process_LogMessage(proc, MSG_INTERNET_ACCESS_DENIED);
                proc->file_warn_internet = FALSE;
            }

        } else {

            status = STATUS_SUCCESS;
        }
    }

    //
    // finish
    //

    if (ProcessId) {

        ExReleaseResourceLite(Process_ListLock);
        KeLowerIrql(irql);
    }

    return status;
}


//---------------------------------------------------------------------------
// File_Api_GetBlockedDll
//---------------------------------------------------------------------------


_FX NTSTATUS File_Api_GetBlockedDll(PROCESS *proc, ULONG64 *parms)
{
    API_GET_BLOCKED_DLL_ARGS *args = (API_GET_BLOCKED_DLL_ARGS *)parms;
    WCHAR *user_buf;
    ULONG user_len;
    ULONG len;
    NTSTATUS status;
    BLOCKED_DLL *blk;
    KIRQL irql;

    //
    // this API must be invoked by a sandboxed process
    //

    if (! proc)
        return STATUS_NOT_IMPLEMENTED;

    //
    // check input buffers
    //

    user_buf = args->dll_name_buf.val;
    user_len = args->dll_name_len.val / sizeof(WCHAR);
    if ((! user_buf) || (! user_len))
        return STATUS_INVALID_PARAMETER;

    //
    // return first blocked dll in the list
    //

    KeRaiseIrql(APC_LEVEL, &irql);
    ExAcquireResourceExclusiveLite(proc->file_lock, TRUE);

    blk = List_Head(&proc->blocked_dlls);
    if (! blk)
        status = STATUS_END_OF_FILE;
    else {

        __try {

            len = blk->path_len;
            if (len >= user_len)
                len = user_len - 1;

            ProbeForWrite(
                user_buf, sizeof(WCHAR) * (len + 1), sizeof(WCHAR));
            wmemcpy(user_buf, blk->path, len);
            user_buf[len] = L'\0';

            status = STATUS_SUCCESS;

        } __except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
        }

        List_Remove(&proc->blocked_dlls, blk);

        len = sizeof(BLOCKED_DLL) + blk->path_len * sizeof(WCHAR);
        Mem_Free(blk, len);
    }

    ExReleaseResourceLite(proc->file_lock);
    KeLowerIrql(irql);

    return status;
}
