/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2024 David Xanatos, xanasoft.com
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
// File (Init)
//---------------------------------------------------------------------------


#include "common/my_version.h"
#include "core/svc/ProcessWire.h"
#include <tchar.h>


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define MOUNTMGRCONTROLTYPE  ((ULONG) 'm')

#define IOCTL_MOUNTMGR_QUERY_POINTS \
    CTL_CODE(MOUNTMGRCONTROLTYPE, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_MOUNTMGR_QUERY_DOS_VOLUME_PATHS \
    CTL_CODE(MOUNTMGRCONTROLTYPE, 13, METHOD_BUFFERED, FILE_ANY_ACCESS)


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _MOUNTMGR_MOUNT_POINT {
    ULONG   SymbolicLinkNameOffset;
    USHORT  SymbolicLinkNameLength;
    ULONG   UniqueIdOffset;
    USHORT  UniqueIdLength;
    ULONG   DeviceNameOffset;
    USHORT  DeviceNameLength;
} MOUNTMGR_MOUNT_POINT, *PMOUNTMGR_MOUNT_POINT;


typedef struct _MOUNTMGR_MOUNT_POINTS {
    ULONG                   Size;
    ULONG                   NumberOfMountPoints;
    MOUNTMGR_MOUNT_POINT    MountPoints[1];
} MOUNTMGR_MOUNT_POINTS, *PMOUNTMGR_MOUNT_POINTS;


typedef struct _MOUNTDEV_NAME {
    USHORT  NameLength;
    WCHAR   Name[1];
} MOUNTDEV_NAME, *PMOUNTDEV_NAME;


typedef struct _MOUNTMGR_VOLUME_PATHS {
    ULONG   MultiSzLength;
    WCHAR   MultiSz[1];
} MOUNTMGR_VOLUME_PATHS, *PMOUNTMGR_VOLUME_PATHS;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


//static void File_InitPathList(void);

static BOOLEAN File_InitDrives(ULONG DriveMask);

static void File_InitLinks(THREAD_DATA *TlsData);

#ifdef WOW64_FS_REDIR
static void File_InitWow64(void);
#endif WOW64_FS_REDIR

static BOOLEAN File_InitUsers(void);

static NTSTATUS File_InitUsers_2(
    HANDLE hProfileKey, WCHAR *buf1, WCHAR *buf2, const ULONG buf_len,
    ULONG *errlvl);

static NTSTATUS File_InitUsers_3(
    HANDLE hkey, UNICODE_STRING *uni, WCHAR *buf, ULONG buf_len);

static BOOLEAN File_InitUsersFromEnvironment(WCHAR *buf, ULONG buf_len);

static WCHAR *File_AllocAndInitEnvironment_2(
    ULONG len, WCHAR **p_optr, BOOLEAN add_dev_map);

static void File_AdjustDrives(
    ULONG path_drive_index, BOOLEAN subst, const WCHAR *path);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static const WCHAR *File_AllUsers_EnvVar    = ENV_VAR_PFX L"ALL_USERS";
static const WCHAR *File_CurrentUser_EnvVar = ENV_VAR_PFX L"CURRENT_USER";
static const WCHAR *File_PublicUser_EnvVar  = ENV_VAR_PFX L"PUBLIC_USER";
static const WCHAR *File_DeviceMap_EnvVar   = ENV_VAR_PFX L"DEVICE_MAP";

#undef ENV_VAR_PFX


//---------------------------------------------------------------------------
// File_Init
//---------------------------------------------------------------------------


_FX BOOLEAN File_Init(void)
{
    HMODULE module = Dll_Ntdll;

    void *RtlGetFullPathName_UEx;
    void *GetTempPathW;
    void *NtQueryDirectoryFileEx = NULL;
    void *NtQueryInformationByName = NULL;
    InitializeCriticalSection(&File_CurDir_CritSec);

    InitializeCriticalSection(&File_DirHandles_CritSec);
    List_Init(&File_DirHandles);

    File_ProxyPipes = Dll_Alloc(sizeof(ULONG) * 256);
    memzero(File_ProxyPipes, sizeof(ULONG) * 256);

    SbieDll_MatchPath(L'f', (const WCHAR *)-1); //File_InitPathList();

    File_DriveAddSN = SbieApi_QueryConfBool(NULL, L"UseVolumeSerialNumbers", FALSE);

    File_UseVolumeGuid = SbieApi_QueryConfBool(NULL, L"UseVolumeGuidWhenNoLetter", FALSE);

    if (! File_InitDrives(0xFFFFFFFF))
        return FALSE;

    File_Delete_v2 = SbieApi_QueryConfBool(NULL, L"UseFileDeleteV2", FALSE);
    if (File_Delete_v2)
        File_InitDelete_v2();

    // this is here as it requirers file stuff to be set up
    extern BOOLEAN Key_Delete_v2;
    BOOLEAN Key_InitDelete_v2();
    Key_Delete_v2 = SbieApi_QueryConfBool(NULL, L"UseRegDeleteV2", FALSE);
    if (Key_Delete_v2)
        Key_InitDelete_v2();

    // this requirers key stuff to be set up
	if (SbieApi_QueryConfBool(NULL, L"SeparateUserFolders", TRUE)) {
		if (!File_InitUsers())
			return FALSE;
	}

    if (Dll_OsBuild >= 6000) {

        void *GetFinalPathNameByHandleW =
            GetProcAddress(Dll_KernelBase ? Dll_KernelBase : Dll_Kernel32,
                "GetFinalPathNameByHandleW");
        if (GetFinalPathNameByHandleW) {
            SBIEDLL_HOOK(File_,GetFinalPathNameByHandleW);
        }
    }

    
    Dll_BoxFileDosPath = Dll_Alloc((Dll_BoxFilePathLen + 1) * sizeof(WCHAR));
    wcscpy((WCHAR *)Dll_BoxFileDosPath, Dll_BoxFilePath);
    if (!SbieDll_TranslateNtToDosPath((WCHAR *)Dll_BoxFileDosPath) /*|| _wcsnicmp(Dll_BoxFileDosPath, L"\\\\.\\", 4) == 0*/)
    {
        Dll_Free((WCHAR *)Dll_BoxFileDosPath);
        Dll_BoxFileDosPath = NULL;

        //
        // the root is redirected with a reparse point and the target device does not have a drvie letter
        // implement workaround, see SbieDll_TranslateNtToDosPath
        //

        ULONG BoxFileRawPathLen;
        if (NT_SUCCESS(SbieApi_QueryProcessInfoStr(0, 'root', NULL, &BoxFileRawPathLen))) 
        {
            Dll_BoxFileRawPath = Dll_AllocTemp(BoxFileRawPathLen);
            if (NT_SUCCESS(SbieApi_QueryProcessInfoStr(0, 'root', (WCHAR*)Dll_BoxFileRawPath, &BoxFileRawPathLen))) 
            {
                Dll_BoxFileRawPathLen = wcslen(Dll_BoxFileRawPath);

                Dll_BoxFileDosPath = Dll_Alloc(BoxFileRawPathLen);
                wcscpy((WCHAR*)Dll_BoxFileDosPath, Dll_BoxFileRawPath);
                if (!SbieDll_TranslateNtToDosPath((WCHAR*)Dll_BoxFileDosPath)) {
                    Dll_Free((WCHAR *)Dll_BoxFileDosPath);
                    Dll_BoxFileDosPath = NULL;
                }
            }
        }
    }
    if(Dll_BoxFileDosPath)
        Dll_BoxFileDosPathLen = wcslen(Dll_BoxFileDosPath);

	File_InitSnapshots();

    File_InitRecoverFolders();

    File_InitFileMigration();

    //
    // intercept NTDLL entry points
    //
    NtQueryDirectoryFileEx = GetProcAddress(Dll_Ntdll, "NtQueryDirectoryFileEx");
    if (NtQueryDirectoryFileEx) {

        SBIEDLL_HOOK(File_, NtQueryDirectoryFileEx);
    }

    SBIEDLL_HOOK(File_,NtCreateFile);
    SBIEDLL_HOOK(File_,NtOpenFile);
    SBIEDLL_HOOK(File_,NtQueryAttributesFile);
    SBIEDLL_HOOK(File_,NtQueryFullAttributesFile);

    NtQueryInformationByName = GetProcAddress(Dll_Ntdll, "NtQueryInformationByName");
    if (NtQueryInformationByName) {

        SBIEDLL_HOOK(File_, NtQueryInformationByName);
    }

    SBIEDLL_HOOK(File_,NtQueryInformationFile);
    SBIEDLL_HOOK(File_,NtQueryDirectoryFile);
    SBIEDLL_HOOK(File_,NtSetInformationFile);
    SBIEDLL_HOOK(File_,NtDeleteFile);
    SBIEDLL_HOOK(File_,NtClose);
    SBIEDLL_HOOK(File_,NtCreateNamedPipeFile);
    SBIEDLL_HOOK(File_,NtCreateMailslotFile);
    SBIEDLL_HOOK(File_,NtReadFile);
    SBIEDLL_HOOK(File_,NtWriteFile);
    SBIEDLL_HOOK(File_,NtFsControlFile);

    if (!Dll_CompartmentMode) // else ping does not work
    if (File_IsBlockedNetParam(NULL)) {
        SBIEDLL_HOOK(File_,NtDeviceIoControlFile);
    }

    RtlGetFullPathName_UEx =
        GetProcAddress(Dll_Ntdll, "RtlGetFullPathName_UEx");
    if (RtlGetFullPathName_UEx) {
        SBIEDLL_HOOK(File_,RtlGetFullPathName_UEx);
    } else {
        SBIEDLL_HOOK(File_,RtlGetFullPathName_U);
    }

    SBIEDLL_HOOK(File_,RtlGetCurrentDirectory_U);
    SBIEDLL_HOOK(File_,RtlSetCurrentDirectory_U);

    SBIEDLL_HOOK(File_,NtQueryVolumeInformationFile);

    //
    // intercept KERNEL32 entry points
    //

    SBIEDLL_HOOK(File_,MoveFileWithProgressW);
    SBIEDLL_HOOK(File_,ReplaceFileW);

    if (1) {

        void *DefineDosDeviceW =
            GetProcAddress(Dll_KernelBase ? Dll_KernelBase : Dll_Kernel32,
                "DefineDosDeviceW");
        if (DefineDosDeviceW) {
            SBIEDLL_HOOK(File_,DefineDosDeviceW);
        }
    }

    if (Dll_OsBuild >= 8400 && Dll_IsSystemSid) {
        // see File_GetTempPathW in file file_misc.c
        GetTempPathW = GetProcAddress(Dll_KernelBase, "GetTempPathW");
        if (GetTempPathW) {
            SBIEDLL_HOOK(File_,GetTempPathW);
        }
    }

    // $Workaround$ - 3rd party fix
    //
    // support for Google Chrome flash plugin process
    //
    // $Workaround$ - 3rd party fix
    //void *GetVolumeInformationW =
    //    GetProcAddress(Dll_KernelBase ? Dll_KernelBase : Dll_Kernel32,
    //        "GetVolumeInformationW");
    //SBIEDLL_HOOK(File_,GetVolumeInformationW);

    void* WriteProcessMemory =
        GetProcAddress(Dll_KernelBase ? Dll_KernelBase : Dll_Kernel32,
            "WriteProcessMemory");
    SBIEDLL_HOOK(File_, WriteProcessMemory);

    return TRUE;
}


//---------------------------------------------------------------------------
// File_IsBlockedNetParam
//---------------------------------------------------------------------------


_FX BOOLEAN File_IsBlockedNetParam(const WCHAR *BoxName)
{
    return SbieApi_QueryConfBool(BoxName, L"BlockNetParam", TRUE);
}


//---------------------------------------------------------------------------
// File_InitPathList
//---------------------------------------------------------------------------


//_FX void File_InitPathList(void)
//{
//    OBJECT_ATTRIBUTES objattrs;
//    UNICODE_STRING objname;
//    IO_STATUS_BLOCK MyIoStatusBlock;
//    HANDLE handle;
//    WCHAR *buf, *ptr;
//
//    RtlInitUnicodeString(&objname, L"\\SystemRoot");
//    InitializeObjectAttributes(
//        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);
//    handle = 0;
//    NtOpenFile(&handle, FILE_READ_DATA, &objattrs,
//               &MyIoStatusBlock, FILE_SHARE_VALID_FLAGS, 0);
//
//    //
//
//    const ULONG PATH_BUF_LEN = 1024;
//    buf = Dll_AllocTemp(PATH_BUF_LEN);
//
//    if (NT_SUCCESS(File_GetFileName(handle, PATH_BUF_LEN, buf)) && (ptr = wcsrchr(buf, L'\\')) != NULL) 
//        ptr[1] = L'\0'; // strip the folder name
//    else // fallback
//        wcscpy(buf, L"\\??\\C:\\");
//
//    File_SysVolumeLen = wcslen(buf);
//    File_SysVolume =
//        Dll_Alloc((File_SysVolumeLen + 1) * sizeof(WCHAR));
//    wcscpy(File_SysVolume, buf);
//
//    Dll_Free(buf);
//
//    //
//
//    if (handle)
//        NtClose(handle);
//
//    SbieDll_MatchPath(L'f', (const WCHAR *)-1);
//}


//---------------------------------------------------------------------------
// File_GetVolumeSN
//---------------------------------------------------------------------------


_FX ULONG File_GetVolumeSN(const FILE_DRIVE *drive)
{
    ULONG sn = 0;
    HANDLE handle;
    IO_STATUS_BLOCK iosb;

    UNICODE_STRING objname;
    objname.Buffer = Dll_Alloc((drive->len + 4) * sizeof(WCHAR));
    wmemcpy(objname.Buffer, drive->path, drive->len);
    objname.Buffer[drive->len    ] = L'\\';
    objname.Buffer[drive->len + 1] = L'\0';
    
    objname.Length = (USHORT)(drive->len + 1) * sizeof(WCHAR);
    objname.MaximumLength = objname.Length + sizeof(WCHAR);

    OBJECT_ATTRIBUTES objattrs;
    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);
    
    ULONG OldMode;
    RtlSetThreadErrorMode(0x10u, &OldMode);
    NTSTATUS status = NtCreateFile(
        &handle, GENERIC_READ | SYNCHRONIZE, &objattrs,
        &iosb, NULL, 0, FILE_SHARE_VALID_FLAGS,
        FILE_OPEN,
        FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
        NULL, 0);
    RtlSetThreadErrorMode(OldMode, 0i64);

    Dll_Free(objname.Buffer);

    if (NT_SUCCESS(status))
    {
        union {
            FILE_FS_VOLUME_INFORMATION volumeInfo;
            BYTE volumeInfoBuff[64];
        } u;
        if (NT_SUCCESS(NtQueryVolumeInformationFile(handle, &iosb, &u.volumeInfo, sizeof(u), FileFsVolumeInformation)))
            sn = u.volumeInfo.VolumeSerialNumber;

        NtClose(handle);
    }

    return sn;
}


//---------------------------------------------------------------------------
// File_InitDrives
//---------------------------------------------------------------------------


_FX BOOLEAN File_InitDrives(ULONG DriveMask)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    HANDLE handle;
    FILE_DRIVE *file_drive;
    ULONG file_drive_len;
    ULONG drive;
    ULONG path_len;
    //ULONG drive_count;
    WCHAR *path;
    WCHAR error_str[16];
    BOOLEAN CallInitLinks;

    //
    // lock the drives and links structures
    //

    if (! File_DrivesAndLinks_CritSec) {

        File_DrivesAndLinks_CritSec = Dll_Alloc(sizeof(CRITICAL_SECTION));
        InitializeCriticalSectionAndSpinCount(
            File_DrivesAndLinks_CritSec, 1000);
    }

    EnterCriticalSection(File_DrivesAndLinks_CritSec);

    //
    // initialize list of reparse points for mounted volumes and junctions
    //

    if (! File_PermLinks) {

        File_PermLinks = Dll_Alloc(sizeof(LIST));
        List_Init(File_PermLinks);

        File_TempLinks = Dll_Alloc(sizeof(LIST));
        List_Init(File_TempLinks);

        File_GuidLinks = Dll_Alloc(sizeof(LIST));
        List_Init(File_GuidLinks);

        CallInitLinks = TRUE;

    } else
        CallInitLinks = FALSE;

    //
    // create an array to hold the mappings of the 26 possible
    // drive letters to their full NT pathnames
    //

    Dll_PushTlsNameBuffer(TlsData);

    if (! File_Drives) {
        File_Drives = Dll_Alloc(26 * sizeof(FILE_DRIVE *));
        memzero(File_Drives, 26 * sizeof(FILE_DRIVE *));
    }

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    //drive_count = 0;

    for (drive = 0; drive < 26; ++drive) {

        BOOLEAN HaveSymlinkTarget = FALSE;

        FILE_DRIVE *old_drive = NULL;

        //
        // process this drive if (1) it was specified in the mask,
        // or (2) if it is currently an incomplete drive in the form
        // \??\X:\ and drive X was specified in the mask
        //

        if (! (DriveMask & (1 << drive))) {

            file_drive = File_Drives[drive];
            if (file_drive) {
                path = file_drive->path;
                if (path && wmemcmp(path, File_BQQB, 4) == 0 &&
                        path[4] && path[5] == L':' && path[6] == L'\\') {

                    WCHAR other_drive = path[4];
                    if (other_drive >= L'A' && other_drive <= L'Z')
                        other_drive = other_drive - L'A' + L'a';
                    if (other_drive >= L'a' && other_drive <= L'z') {
                        other_drive -= L'a';
                        if (DriveMask & (1 << other_drive)) {

                            DriveMask |= (1 << drive);
                        }
                    }
                }
            }

            if (! (DriveMask & (1 << drive)))
                continue;
        }

        if (File_Drives[drive]) {

            old_drive = File_Drives[drive];
            File_Drives[drive] = NULL;

            File_RemovePermLinks(old_drive->path);
        }

        //
        // translate the DosDevices symbolic link "\??\x:" into its
        // device object.  note that sometimes the DosDevices name
        // translate to symbolic link itself.
        //

        path_len = 16;
        path = Dll_Alloc(path_len);
        Sbie_snwprintf(path, 8, L"\\??\\%c:", L'A' + drive);

        RtlInitUnicodeString(&objname, path);

        status = NtOpenSymbolicLinkObject(
            &handle, SYMBOLIC_LINK_QUERY, &objattrs);

        if (!NT_SUCCESS(status)) {

            //
            // if the object is a valid symbolic link but we don't have
            // access rights to open the symbolic link then we ask the
            // driver to query the link for us
            //

            WCHAR *path2 = Dll_AllocTemp(1024 * sizeof(WCHAR));
            wcscpy(path2, path);

            NTSTATUS status2 = SbieApi_QuerySymbolicLink(path2, 1024 * sizeof(WCHAR));
            if (NT_SUCCESS(status2)) {

                Dll_Free(path);
                path = path2;
                status = status2;

                objname.Length = (USHORT)(wcslen(path) * sizeof(WCHAR));
                objname.MaximumLength = objname.Length + sizeof(WCHAR);
                objname.Buffer = path;

                handle = NULL;
                HaveSymlinkTarget = TRUE;

            } else {

                Dll_Free(path2);
            }
        }

        //
        // errors indicate the drive does not exist, is no longer valid
        // or was never valid.  we can also get access denied, if the
        // drive appears as ClosedFilePath.  in any case, silently ignore
        //

        if (! NT_SUCCESS(status)) {

            Dll_Free(path);

            if (status != STATUS_OBJECT_NAME_NOT_FOUND &&
                status != STATUS_OBJECT_PATH_NOT_FOUND &&
                status != STATUS_OBJECT_TYPE_MISMATCH &&
                status != STATUS_ACCESS_DENIED) {

                Sbie_snwprintf(error_str, 16, L"%c [%08X]", L'A' + drive, status);
                SbieApi_Log(2307, error_str);
            }

            if (old_drive) {
                File_AdjustDrives(drive, old_drive->subst, old_drive->path);
                Dll_Free(old_drive);
            }

            continue;
        }

        //
        // get the target of the symbolic link
        //

        if (handle) {

            memzero(&objname, sizeof(UNICODE_STRING));
            status = NtQuerySymbolicLinkObject(handle, &objname, &path_len);

            if (status != STATUS_BUFFER_TOO_SMALL) {
                if (NT_SUCCESS(status))
                    status = STATUS_UNSUCCESSFUL;

            } else {

                Dll_Free(path);
                path_len += 32;
                path = Dll_Alloc(path_len);

                objname.MaximumLength = (USHORT)(path_len - 8);
                objname.Length        = objname.MaximumLength - sizeof(WCHAR);
                objname.Buffer        = path;
                status = NtQuerySymbolicLinkObject(handle, &objname, NULL);

                if (NT_SUCCESS(status)) {

                    HaveSymlinkTarget = TRUE;
                }
            }

            NtClose(handle);
        }

        //
        // add a new drive entry
        //

        if (HaveSymlinkTarget) {

            BOOLEAN subst = FALSE;
            BOOLEAN translated = FALSE;
            WCHAR *save_path = path;
            path[objname.Length / sizeof(WCHAR)] = L'\0';

            if (wmemcmp(path, File_BQQB, 4) == 0
                                        && path[4] && path[5] == L':') {
                // SUBST drives translate to \??\X:...
                subst = TRUE;
            }

            path = File_GetName_TranslateSymlinks(
                        TlsData, path, objname.Length, &translated);
            if (path) {

                // (returned path points into NameBuffer
                // so it need not be explicitly freed)

                path_len = wcslen(path);
                file_drive_len = sizeof(FILE_DRIVE)
                               + (path_len + 1) * sizeof(WCHAR);
                file_drive = Dll_Alloc(file_drive_len);
                file_drive->letter = (WCHAR)(drive + L'A');
                file_drive->subst = subst;
                file_drive->len = path_len;
                wcscpy(file_drive->path, path);
                *file_drive->sn = 0;
                if (File_DriveAddSN) {
                    ULONG sn = File_GetVolumeSN(file_drive);
                    if(sn != 0)
                        Sbie_snwprintf(file_drive->sn, 10, L"%04X-%04X", HIWORD(sn), LOWORD(sn));
                }

                File_Drives[drive] = file_drive;
                //drive_count++;

                SbieApi_MonitorPut(MONITOR_DRIVE, path);
            }

            Dll_Free(save_path);
        }

        //
        // discard old drive entry
        //

        if (old_drive)
            Dll_Free(old_drive);

        if (! NT_SUCCESS(status)) {

            Sbie_snwprintf(error_str, 16, L"%c [%08X]", L'A' + drive, status);
            SbieApi_Log(2307, error_str);
        }
    }

    //if (drive_count == 0) {
    //    Sbie_snwprintf(error_str, 16, L"No Drives Found");
    //    SbieApi_Log(2307, error_str);
    //}

    //
    // initialize list of volumes mounted to directories rather than drives
    //

    if (CallInitLinks) {

        File_InitLinks(TlsData);

#ifdef WOW64_FS_REDIR
        File_InitWow64();
#endif WOW64_FS_REDIR
    }

    Dll_PopTlsNameBuffer(TlsData);

    LeaveCriticalSection(File_DrivesAndLinks_CritSec);

    return TRUE;
}


//---------------------------------------------------------------------------
// File_InitLinks
//---------------------------------------------------------------------------


_FX void File_InitLinks(THREAD_DATA *TlsData)
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE hMountMgr;
    MOUNTMGR_MOUNT_POINT Input1;
    MOUNTMGR_MOUNT_POINTS *Output1;
    MOUNTDEV_NAME *Input2;
    MOUNTMGR_VOLUME_PATHS *Output2;
    ULONG index1;
    WCHAR save_char;
    FILE_GUID* guid;
    ULONG alloc_len;
    WCHAR text[256];

    //
    // cleanup old guid entries
    //

    EnterCriticalSection(File_DrivesAndLinks_CritSec);
    guid = List_Head(File_GuidLinks);
    while (guid) {
        FILE_LINK *next_guid = List_Next(guid);
        List_Remove(File_GuidLinks, guid);
        Dll_Free(guid);
        guid = next_guid;
    }
    LeaveCriticalSection(File_DrivesAndLinks_CritSec);

    //
    // open mount point manager device
    //

    RtlInitUnicodeString(&objname, L"\\Device\\MountPointManager");
    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = NtCreateFile(
        &hMountMgr, FILE_GENERIC_EXECUTE, &objattrs, &IoStatusBlock,
        NULL, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_VALID_FLAGS,
        FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE,
        NULL, 0);

    if (! NT_SUCCESS(status))
        return;

    //
    // query all mount points
    //

    memset(&Input1, 0, sizeof(MOUNTMGR_MOUNT_POINT));
    Output1 = Dll_Alloc(8192);

    status = NtDeviceIoControlFile(
        hMountMgr, NULL, NULL, NULL, &IoStatusBlock,
        IOCTL_MOUNTMGR_QUERY_POINTS,
        &Input1, sizeof(MOUNTMGR_MOUNT_POINT),
        Output1, 8192);

    if (! NT_SUCCESS(status)) {
        NtClose(hMountMgr);
        return;
    }

    Input2 = Dll_Alloc(128);
    Output2 = Dll_Alloc(8192);

    //
    // process each primary mount point of each volume
    //

    for (index1 = 0; index1 < Output1->NumberOfMountPoints; ++index1) {

        MOUNTMGR_MOUNT_POINT *MountPoint = &Output1->MountPoints[index1];
        WCHAR *DeviceName =
            (WCHAR *)((UCHAR *)Output1 + MountPoint->DeviceNameOffset);
        ULONG DeviceNameLen =
            MountPoint->DeviceNameLength / sizeof(WCHAR);
        WCHAR *VolumeName =
            (WCHAR *)((UCHAR *)Output1 + MountPoint->SymbolicLinkNameOffset);
        ULONG VolumeNameLen =
            MountPoint->SymbolicLinkNameLength / sizeof(WCHAR);

        Sbie_snwprintf(text, 256, L"Found Mountpoint: %.*s <-> %.*s", VolumeNameLen, VolumeName, DeviceNameLen, DeviceName);
        SbieApi_MonitorPut2(MONITOR_DRIVE | MONITOR_TRACE, text, FALSE);

        if (VolumeNameLen != 48 && VolumeNameLen != 49)
            continue;
        if (_wcsnicmp(VolumeName, L"\\??\\Volume{", 11) != 0)
            continue;

        //
        // store guid to nt device association
        //

        alloc_len = sizeof(FILE_GUID)
              + (VolumeNameLen + 1) * sizeof(WCHAR);
        guid = Dll_Alloc(alloc_len);
        wmemcpy(guid->guid, &VolumeName[10], 38);
        guid->guid[38] = 0;
        guid->len = DeviceNameLen;
        wmemcpy(guid->path, DeviceName, DeviceNameLen);
        guid->path[DeviceNameLen] = 0;
        EnterCriticalSection(File_DrivesAndLinks_CritSec);
        List_Insert_Before(File_GuidLinks, NULL, guid);
        LeaveCriticalSection(File_DrivesAndLinks_CritSec);

        //
        // get all the DOS paths where the volume is mounted
        //

        Input2->NameLength = 48 * sizeof(WCHAR);
        wmemcpy(Input2->Name, VolumeName, 48);
        Input2->Name[48] = L'\0';

        status = NtDeviceIoControlFile(
            hMountMgr, NULL, NULL, NULL, &IoStatusBlock,
            IOCTL_MOUNTMGR_QUERY_DOS_VOLUME_PATHS,
            Input2, 98,
            Output2, 8192);

        if (! NT_SUCCESS(status))
            continue;

        //
        // process DOS paths
        //

        save_char = DeviceName[DeviceNameLen];
        DeviceName[DeviceNameLen] = L'\0';

        if (Output2->MultiSzLength && *Output2->MultiSz) {

            WCHAR *DosPath = Output2->MultiSz;
            ULONG DosPathLen = wcslen(DosPath);
            if (DosPathLen <= 3) {

                //
                // handle the case where the volume is also mounted as a
                // drive letter:
                // 1.  ignore the first mounted path (the drive letter)
                // 2.  add a reparse point from any other mounted path to
                //     the device name
                //

                DosPath += DosPathLen + 1;
                while (*DosPath) {
                    Sbie_snwprintf(text, 256, L"Mountpoint AddLink: %s <-> %s", DosPath, DeviceName);
                    SbieApi_MonitorPut2(MONITOR_DRIVE | MONITOR_TRACE, text, FALSE);
                    File_AddLink(TRUE, DosPath, DeviceName);
                    DosPath += wcslen(DosPath) + 1;
                }

			} else if (File_UseVolumeGuid) {

                // handle the case where the volume is not mounted as a
                // drive letter:
                //     add reparse points for all mounted directories

                //
                // This behaviour creates \[BoxRoot]\drive\{guid} folders
                // instead of using the first mount point on a volume with a letter
                //

                WCHAR *FirstDosPath = DosPath;
                Sbie_snwprintf(text, 256, L"Mountpoint AddLink: %s <-> %s", FirstDosPath, DeviceName);
                SbieApi_MonitorPut2(MONITOR_DRIVE | MONITOR_TRACE, text, FALSE);
                File_AddLink(TRUE, FirstDosPath, DeviceName);
                DosPath += DosPathLen + 1;
                while (*DosPath) {
                    Sbie_snwprintf(text, 256, L"Mountpoint AddLink: %s <-> %s", DosPath, DeviceName);
                    SbieApi_MonitorPut2(MONITOR_DRIVE | MONITOR_TRACE, text, FALSE);
                    File_AddLink(TRUE, DosPath, DeviceName);
                    DosPath += wcslen(DosPath) + 1;
                }
				
            } else {

                //
                // handle the case where the volume is not mounted as a
                // drive letter:
                // 1.  add a reparse point from the device name to the first
                //     mounted directory
                // 2.  add a reparse point from any other mounted directory
                //     also to the first mounted directory
                //

                //
                // Note: this behaviour makes the first mounted directory
                // the location in the box where all files for that volume will be located
                // other mount points will be redirected to this folder
                //

                WCHAR *FirstDosPath = DosPath;
                Sbie_snwprintf(text, 256, L"Mountpoint AddLink: %s <-> %s", DeviceName, FirstDosPath);
                SbieApi_MonitorPut2(MONITOR_DRIVE | MONITOR_TRACE, text, FALSE);
                File_AddLink(TRUE, DeviceName, FirstDosPath);
                DosPath += DosPathLen + 1;
                while (*DosPath) {
                    Sbie_snwprintf(text, 256, L"Mountpoint AddLink: %s <-> %s", DosPath, FirstDosPath);
                    SbieApi_MonitorPut2(MONITOR_DRIVE | MONITOR_TRACE, text, FALSE);
                    File_AddLink(TRUE, DosPath, FirstDosPath);
                    DosPath += wcslen(DosPath) + 1;
                }
            }
        }

        DeviceName[DeviceNameLen] = save_char;
    }

    Dll_Free(Output2);
    Dll_Free(Input2);
    Dll_Free(Output1);
    NtClose(hMountMgr);

    //
    // the sandbox path may be specified on a directory mount point
    // C:\MOUNT\SANDBOX, in which case Dll_BoxFilePath will be set to
    // the target device \Device\HarddiskVolume2, but we also need to
    // keep the mount point location \Device\HarddiskVolume1\MOUNT\SANDBOX
    // which is used in File_GetName
    //

    if (Dll_BoxName) {

        WCHAR *TruePath = Dll_GetTlsNameBuffer(TlsData, TRUE_NAME_BUFFER,
                                (Dll_BoxFilePathLen + 1) * sizeof(WCHAR));
        wmemcpy(TruePath, Dll_BoxFilePath, Dll_BoxFilePathLen + 1);

        if (TruePath) {

            BOOLEAN converted =
                File_GetName_ConvertLinks(TlsData, &TruePath, FALSE);
            if (converted) {

                ULONG len = wcslen(TruePath);
                File_AltBoxPath = Dll_Alloc((len + 1) * sizeof(WCHAR));
                wmemcpy(File_AltBoxPath, TruePath, len + 1);
                File_AltBoxPathLen = len;
            }
        }
    }
}


//---------------------------------------------------------------------------
// File_AdjustDrives
//---------------------------------------------------------------------------


_FX void File_AdjustDrives(
    ULONG path_drive_index, BOOLEAN subst, const WCHAR *path)
{
    FILE_DRIVE *file_drive;
    WCHAR *file_drive_path;
    ULONG path_len;
    ULONG drive_index;
    BOOLEAN file_drive_subst;

    //
    // if the path of any other drives references the path being removed,
    // then convert the path of the other drive to the form \\X:\...
    //
    // if the drive being removed is a SUBST drive, then we only look at
    // other SUBST drives here, to prevent the removal of the SUBST drive
    // from having an effect on drives that are not related to the SUBST
    //

    path_len = wcslen(path);

    for (drive_index = 0; drive_index < 26; ++drive_index) {

        file_drive = File_Drives[drive_index];
        if (! file_drive)
            continue;
        file_drive_subst = file_drive->subst;
        if (subst && (! file_drive_subst))
            continue;
        file_drive_path = file_drive->path;

        if (wcslen(file_drive_path) >= path_len
                &&  (file_drive_path[path_len] == L'\0' ||
                     file_drive_path[path_len] == L'\\')
                &&  _wcsnicmp(file_drive_path, path, path_len) == 0) {

            WCHAR *new_path;
            ULONG alloc_len = sizeof(FILE_DRIVE)
                            + (path_len + 8) * sizeof(WCHAR);
            file_drive = Dll_Alloc(alloc_len);
            file_drive->letter = (WCHAR)(drive_index + L'A');
            file_drive->subst = file_drive_subst;

            new_path = file_drive->path;
            wmemcpy(new_path, File_BQQB, 4);
            new_path[4] = (WCHAR)(path_drive_index + L'A');
            new_path[5] = L':';
            wcscpy(new_path + 6, file_drive_path + path_len);

            file_drive->len = wcslen(new_path);

            Dll_Free(File_Drives[drive_index]);
            File_Drives[drive_index] = file_drive;

            SbieApi_MonitorPut(MONITOR_DRIVE, new_path);
        }
    }
}


//---------------------------------------------------------------------------
// File_InitWow64
//---------------------------------------------------------------------------


#ifdef WOW64_FS_REDIR
_FX void File_InitWow64(void)
{
    typedef (*P_GetSystemWow64Directory)(WCHAR *buf, UINT len);
    P_GetSystemWow64Directory pGetSystemWow64Directory;
    WCHAR *path, *path32, *path64;

    if (! Dll_IsWow64)
        return;

    //
    // add a file link to translate System32 to SysWow64
    //

    pGetSystemWow64Directory = (P_GetSystemWow64Directory)
        GetProcAddress(Dll_Kernel32, "GetSystemWow64DirectoryW");
    if (! pGetSystemWow64Directory)
        return;

    path = Dll_AllocTemp((MAX_PATH + 64) * sizeof(WCHAR));

    *path = L'\0';
    GetSystemWindowsDirectory(path, MAX_PATH);
    if (*path) {
        if (path[wcslen(path) - 1] == L'\\')
            path[wcslen(path) - 1] = L'\0';
        wcscat(path, L"\\System32");
    }

    path32 = Dll_Alloc((7 + wcslen(path) + 1) * sizeof(WCHAR));

    wcscpy(path32, L"\\drive\\");
    path32[7] = path[0]; // drive letter
    wcscpy(&path32[8], &path[2]); // skip :

    File_Wow64System32 = path32;
    File_Wow64System32Len = wcslen(path32);

    path32 = File_TranslateDosToNtPath(path);
    if (path32) {

        //
        // get the path for the SysNative virtual folder
        //

        wcscpy(path + wcslen(path) - 5, L"Native");
        path64 = File_TranslateDosToNtPath(path);
        if (path64) {

            File_Wow64SysNative = path64;
            File_Wow64SysNativeLen = wcslen(path64);
        }

        //
        // add a file link to translate System32 to SysWow64
        //

        *path = L'\0';
        pGetSystemWow64Directory(path, MAX_PATH);

        path64 = File_TranslateDosToNtPath(path);
        if (path64) {

            if (File_AddLink(TRUE, path32, path64))
                File_Wow64FileLink = List_Tail(File_PermLinks);

            Dll_Free(path64);
        }

        //
        //
        //

        Dll_Free(path32);
    }

    Dll_Free(path);

    //
    // get Wow64 filesystem redirection APIs
    //

    if (File_Wow64FileLink) {

        File_Wow64DisableWow64FsRedirection =
            GetProcAddress(Dll_Kernel32, "Wow64DisableWow64FsRedirection");
        File_Wow64RevertWow64FsRedirection =
            GetProcAddress(Dll_Kernel32, "Wow64RevertWow64FsRedirection");
    }
}
#endif WOW64_FS_REDIR


//---------------------------------------------------------------------------
// File_InitUsers
//---------------------------------------------------------------------------


_FX BOOLEAN File_InitUsers(void)
{
    static const WCHAR *_ProfileList =
        L"\\Registry\\Machine\\Software"
            L"\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList";
    NTSTATUS status;
    UNICODE_STRING objname;
    OBJECT_ATTRIBUTES objattrs;
    const ULONG buf_len = 1024;
    WCHAR *buf1, *buf2;
    ULONG errlvl;
    HANDLE hProfileKey;

    //
    // check if File_AllUsers / File_CurrentUser / File_PublicUser
    // were passed from the parent process through the environment
    //

    buf1 = Dll_Alloc(buf_len);

    if (File_InitUsersFromEnvironment(buf1, buf_len)) {

        Dll_Free(buf1);
        return TRUE;
    }

    //
    // open ProfileList key
    //

    RtlInitUnicodeString(&objname, _ProfileList);

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = NtOpenKey(&hProfileKey, KEY_READ, &objattrs);
    if (! NT_SUCCESS(status))
        errlvl = 0x11;
    else {

        //
        // more processing in File_InitUsers_2
        //

        buf2 = Dll_Alloc(buf_len);

        errlvl = 0;
        status = File_InitUsers_2(hProfileKey, buf1, buf2, buf_len, &errlvl);

        Dll_Free(buf2);

        NtClose(hProfileKey);
    }

    //
    // finish
    //

    Dll_Free(buf1);

    if (errlvl) {
        WCHAR error_str[16];
        Sbie_snwprintf(error_str, 16, L"[%08X / %02X]", status, errlvl);
        SbieApi_Log(2306, error_str);
        return FALSE;
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// File_InitUsers_2
//---------------------------------------------------------------------------


_FX NTSTATUS File_InitUsers_2(
    HANDLE hProfileKey, WCHAR *buf1, WCHAR *buf2, const ULONG buf_len,
    ULONG *errlvl)
{
    NTSTATUS status;
    FILE_DRIVE *drive;
    ULONG len1, len2;
    HANDLE hSidKey;
    UNICODE_STRING uni;
    OBJECT_ATTRIBUTES objattrs;

    //
    // query value ProfilesDirectory in the ProfileList key
    //

    RtlInitUnicodeString(&uni, L"ProfilesDirectory");

    status = File_InitUsers_3(hProfileKey, &uni, buf1, buf_len);

    if (! NT_SUCCESS(status)) {
        *errlvl = 0x22;
        return status;
    }

    len1 = wcslen(buf1);

    //
    // query value AllUsersProfile in the ProfileList key.
    // on Windows Vista, there is no AllUsersProfile, instead there
    // is a ProgramData value
    //

    RtlInitUnicodeString(&uni, L"AllUsersProfile");

    status = File_InitUsers_3(hProfileKey, &uni, buf2, buf_len);

    if (status == STATUS_OBJECT_NAME_NOT_FOUND) {

        RtlInitUnicodeString(&uni, L"ProgramData");

        status = File_InitUsers_3(hProfileKey, &uni, buf2, buf_len);
    }

    if (! NT_SUCCESS(status)) {
        *errlvl = 0x33;
        return status;
    }

    len2 = wcslen(buf2);

    //
    // combine ProfilesDirectory\AllUsersProfile into File_AllUsers
    //

    if (uni.Buffer[0] == L'A') {            // AllUsersProfile

        if (buf1[1] == L':')
            drive = File_GetDriveForLetter(buf1[0]);
        else
            drive = NULL;
        if (! drive) {
            *errlvl = 0x22;
            return STATUS_OBJECT_PATH_SYNTAX_BAD;
        }

        File_AllUsersLen = drive->len + (len1 - 2) + 1 + len2;
        File_AllUsers = Dll_Alloc((File_AllUsersLen + 1) * sizeof(WCHAR));
        wcscpy(File_AllUsers, drive->path);
        wcscat(File_AllUsers, &buf1[2]);
        wcscat(File_AllUsers, L"\\");
        wcscat(File_AllUsers, buf2);

        LeaveCriticalSection(File_DrivesAndLinks_CritSec);

    } else if (uni.Buffer[0] == 'P') {      // ProgramData

        if (buf2[1] == L':')
            drive = File_GetDriveForLetter(buf2[0]);
        else
            drive = NULL;
        if (! drive) {
            *errlvl = 0x33;
            return STATUS_OBJECT_PATH_SYNTAX_BAD;
        }

        File_AllUsersLen = drive->len + (len2 - 2);
        File_AllUsers = Dll_Alloc((File_AllUsersLen + 1) * sizeof(WCHAR));
        wcscpy(File_AllUsers, drive->path);
        wcscat(File_AllUsers, &buf2[2]);

        LeaveCriticalSection(File_DrivesAndLinks_CritSec);

    } else {

        *errlvl = 0xEE;                     // shouldn't happen
        return STATUS_INVALID_PARAMETER;
    }

    //
    // open ProfileList\SID key
    //

    RtlInitUnicodeString(&uni, Dll_SidString);

    InitializeObjectAttributes(
        &objattrs, &uni, OBJ_CASE_INSENSITIVE, hProfileKey, NULL);

    status = NtOpenKey(&hSidKey, KEY_READ, &objattrs);

    if (! NT_SUCCESS(status)) {
        *errlvl = 0x44;
        return status;
    }

    //
    // query value ProfileImagePath in the ProfileList\SID key
    //

    RtlInitUnicodeString(&uni, L"ProfileImagePath");

    status = File_InitUsers_3(hSidKey, &uni, buf1, buf_len);

    NtClose(hSidKey);

    if (! NT_SUCCESS(status)) {
        *errlvl = 0x44;
        return status;
    }

    len1 = wcslen(buf1);

    if (buf1[1] == L':')
        drive = File_GetDriveForLetter(buf1[0]);
    else
        drive = NULL;
    if (! drive) {
        *errlvl = 0x44;
        return STATUS_OBJECT_PATH_SYNTAX_BAD;
    }

    File_CurrentUserLen = drive->len + (len1 - 2);
    File_CurrentUser = Dll_Alloc((File_CurrentUserLen + 1) * sizeof(WCHAR));
    wcscpy(File_CurrentUser, drive->path);
    wcscat(File_CurrentUser, &buf1[2]);

    LeaveCriticalSection(File_DrivesAndLinks_CritSec);

    //
    // query value Public in the ProfileList key (Windows Vista/7)
    //

    RtlInitUnicodeString(&uni, L"Public");

    status = File_InitUsers_3(hProfileKey, &uni, buf2, buf_len);

    if (NT_SUCCESS(status)) {

        len2 = wcslen(buf2);

        if (buf2[1] == L':')
            drive = File_GetDriveForLetter(buf2[0]);
        else
            drive = NULL;
        if (! drive) {
            *errlvl = 0x55;
            return STATUS_OBJECT_PATH_SYNTAX_BAD;
        }

        File_PublicUserLen = drive->len + (len2 - 2);
        File_PublicUser =
            Dll_Alloc((File_PublicUserLen + 1) * sizeof(WCHAR));
        wcscpy(File_PublicUser, drive->path);
        wcscat(File_PublicUser, &buf2[2]);

        LeaveCriticalSection(File_DrivesAndLinks_CritSec);

    } else if (status != STATUS_OBJECT_NAME_NOT_FOUND) {
        *errlvl = 0x55;
        return status;
    }

    //
    // finish
    //

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// File_InitUsers_3
//---------------------------------------------------------------------------


_FX NTSTATUS File_InitUsers_3(
    HANDLE hkey, UNICODE_STRING *uni, WCHAR *buf, ULONG buf_len)
{
    const ULONG nSize = (buf_len / sizeof(WCHAR)) - 8;

    ULONG len;
    WCHAR *str;

    KEY_VALUE_PARTIAL_INFORMATION *info =
        (KEY_VALUE_PARTIAL_INFORMATION *)buf;

    NTSTATUS status = NtQueryValueKey(
        hkey, uni, KeyValuePartialInformation, info, buf_len - 16, &len);

    if (NT_SUCCESS(status)) {

        len = info->DataLength / sizeof(WCHAR);
        str = Dll_AllocTemp((len + 1) * sizeof(WCHAR));
        wmemcpy(str, (WCHAR*)info->Data, len);
        str[len] = L'\0';

        len = ExpandEnvironmentStrings(str, buf, nSize);

        Dll_Free(str);

        if (len == 0 || len >= nSize)
            status = STATUS_NAME_TOO_LONG;
    }

    return status;
}


//---------------------------------------------------------------------------
// File_InitUsersFromEnvironment
//---------------------------------------------------------------------------


_FX BOOLEAN File_InitUsersFromEnvironment(WCHAR *buf, ULONG buf_len)
{
    const ULONG nSize = (buf_len / sizeof(WCHAR)) - 8;

    memzero(buf, 16);
    GetEnvironmentVariable(File_AllUsers_EnvVar, buf, nSize);
    if (buf[0]) {

        File_AllUsersLen = wcslen(buf);
        File_AllUsers =
            Dll_Alloc((File_AllUsersLen + 1) * sizeof(WCHAR));
        wcscpy(File_AllUsers, buf);
    }

    memzero(buf, 16);
    GetEnvironmentVariable(File_CurrentUser_EnvVar, buf, nSize);
    if (buf[0]) {

        File_CurrentUserLen = wcslen(buf);
        File_CurrentUser =
            Dll_Alloc((File_CurrentUserLen + 1) * sizeof(WCHAR));
        wcscpy(File_CurrentUser, buf);
    }

    memzero(buf, 16);
    GetEnvironmentVariable(File_PublicUser_EnvVar, buf, nSize);
    if (buf[0]) {

        File_PublicUserLen = wcslen(buf);
        File_PublicUser =
            Dll_Alloc((File_PublicUserLen + 1) * sizeof(WCHAR));
        wcscpy(File_PublicUser, buf);
    }

    memzero(buf, 16);
    GetEnvironmentVariable(File_DeviceMap_EnvVar, buf, nSize);
    if (buf[0]) {

        buf[95] = L'\0';
        File_GetSetDeviceMap(buf);
    }

    SetEnvironmentVariable(File_AllUsers_EnvVar, NULL);
    SetEnvironmentVariable(File_CurrentUser_EnvVar, NULL);
    SetEnvironmentVariable(File_PublicUser_EnvVar, NULL);
    SetEnvironmentVariable(File_DeviceMap_EnvVar, NULL);

    if (File_AllUsersLen && File_CurrentUserLen)
        return TRUE;    // note that File_PublicUserLen is not checked
    else
        return FALSE;
}


//---------------------------------------------------------------------------
// Proc_AllocAndInitEnvironment
//---------------------------------------------------------------------------


_FX void *File_AllocAndInitEnvironment(
    void *Environment, BOOLEAN IsUnicode, BOOLEAN AddDeviceMap,
    ULONG *OutLengthInBytes)
{
    ULONG len, len1, i;
    WCHAR *env, *optr, *iptrW;
    UCHAR *iptrA;

    //
    // get the default environment strings if necessary
    //

    if (! Environment) {

        Environment = GetEnvironmentStrings();
        IsUnicode = TRUE;
    }

    //
    // scan input environment and compute its total length
    //

    len = 0;

    if (IsUnicode) {

        iptrW = (WCHAR *)Environment;
        while (*iptrW) {
            len1 = wcslen(iptrW) + 1;
            iptrW += len1;
            len += len1;
        }

    } else {

        iptrA = (UCHAR *)Environment;
        while (*iptrA) {
            len1 = strlen(iptrA) + 1;
            iptrA += len1;
            len += len1;
        }
    }

    len = (len + 8) * sizeof(WCHAR);

    //
    // create the output environment, and add special environment
    // variables which will be queried and removed by File_InitUsers
    //

    env = File_AllocAndInitEnvironment_2(len, &optr, AddDeviceMap);

    //
    // copy the rest of the environment
    //

    if (IsUnicode) {

        iptrW = (WCHAR *)Environment;
        while (*iptrW) {

            len1 = wcslen(iptrW) + 1;
            wmemcpy(optr, iptrW, len1);
            optr += len1;
            iptrW += len1;
        }

    } else {

        iptrA = (UCHAR *)Environment;
        while (*iptrA) {

            len1 = strlen(iptrA) + 1;
            for (i = 0; i < len1; ++i) {
                *optr = (WCHAR)*iptrA;
                ++optr;
                ++iptrA;
            }
        }
    }

    *optr = L'\0';

    if (OutLengthInBytes)
        *OutLengthInBytes = (ULONG)((ULONG_PTR)optr - (ULONG_PTR)env);

    return env;
}


//---------------------------------------------------------------------------
// File_AllocAndInitEnvironment_2
//---------------------------------------------------------------------------


_FX WCHAR *File_AllocAndInitEnvironment_2(
    ULONG len, WCHAR **p_optr, BOOLEAN add_dev_map)
{
    WCHAR *env, *optr;
    WCHAR devmap[96];

    len += (    wcslen(File_AllUsers_EnvVar) + 1
              + File_AllUsersLen + 1
              + wcslen(File_CurrentUser_EnvVar) + 1
              + File_CurrentUserLen + 1
           )  * sizeof(WCHAR);

    if (File_PublicUserLen) {

        len += (    wcslen(File_PublicUser_EnvVar) + 1
                  + File_PublicUserLen + 1
               )  * sizeof(WCHAR);
    }

    if (add_dev_map) {

        devmap[0] = L'\0';
        File_GetSetDeviceMap(devmap);

        len += (    wcslen(File_DeviceMap_EnvVar) + 1
                  + wcslen(devmap) + 1
               )  * sizeof(WCHAR);
    }

    env = Dll_Alloc(len);
    optr = env;

    if (File_AllUsersLen) {

        wcscpy(optr, File_AllUsers_EnvVar);
        optr += wcslen(optr);
        *optr = L'=';
        wcscpy(optr + 1, File_AllUsers);
        optr += wcslen(optr) + 1;
    }

    if (File_CurrentUserLen) {

        wcscpy(optr, File_CurrentUser_EnvVar);
        optr += wcslen(optr);
        *optr = L'=';
        wcscpy(optr + 1, File_CurrentUser);
        optr += wcslen(optr) + 1;
    }

    if (File_PublicUserLen) {

        wcscpy(optr, File_PublicUser_EnvVar);
        optr += wcslen(optr);
        *optr = L'=';
        wcscpy(optr + 1, File_PublicUser);
        optr += wcslen(optr) + 1;
    }

    if (add_dev_map) {

        wcscpy(optr, File_DeviceMap_EnvVar);
        optr += wcslen(optr);
        *optr = L'=';
        wcscpy(optr + 1, devmap);
        optr += wcslen(optr) + 1;
    }

    *p_optr = optr;
    return env;
}


//---------------------------------------------------------------------------
// File_ConcatPath2
//---------------------------------------------------------------------------


_FX WCHAR *File_ConcatPath2(const WCHAR *Path1, ULONG Path1Len, const WCHAR *Path2, ULONG Path2Len)
{
    ULONG Length = Path1Len + Path2Len;
    WCHAR* Path = Dll_Alloc((Length + 1) * sizeof(WCHAR));
    wmemcpy(Path, Path1, Path1Len);
    wmemcpy(Path + Path1Len, Path2, Path2Len);
    Path[Length] = L'\0';
    return Path;
}


//---------------------------------------------------------------------------
// File_TranslateDosToNtPath2
//---------------------------------------------------------------------------


_FX WCHAR *File_TranslateDosToNtPath2(const WCHAR *DosPath, ULONG DosPathLen)
{
    WCHAR *NtPath = NULL;

    if (DosPath && DosPath[0] && DosPath[1]) {

        if (DosPath[0] == L'\\' && DosPath[1] == L'\\') {

            //
            // network path
            //

            NtPath = File_ConcatPath2(File_Mup, File_MupLen, DosPath + 2, DosPathLen - 2);

        } else if (DosPath[1] == L':' &&
                        (DosPath[2] == L'\\' || DosPath[2] == L'\0')) {

            //
            // drive-letter path
            //

            FILE_DRIVE *drive = File_GetDriveForLetter(DosPath[0]);
            if (drive) {

                NtPath = File_ConcatPath2(drive->path, drive->len, DosPath + 2, DosPathLen - 2);

                LeaveCriticalSection(File_DrivesAndLinks_CritSec);
            }
        }
    }

    return NtPath;
}


//---------------------------------------------------------------------------
// File_TranslateDosToNtPath
//---------------------------------------------------------------------------


_FX WCHAR *File_TranslateDosToNtPath(const WCHAR *DosPath)
{
    return File_TranslateDosToNtPath2(DosPath, DosPath ? wcslen(DosPath) : 0);
}


//---------------------------------------------------------------------------
// File_GetSetDeviceMap
//---------------------------------------------------------------------------


_FX void File_GetSetDeviceMap(WCHAR *DeviceMap96)
{
    static const WCHAR *_fmt = L"[%08X / %02X]";
    NTSTATUS status;
    UNICODE_STRING objname;
    OBJECT_ATTRIBUTES objattrs;
    MSG_HEADER *rpl;

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    //
    // a sandboxed process starting under the SYSTEM account does not
    // have the local DosDevices directory that it parent, Start.exe,
    // had.  but SbieDll may have recorded the directory, and we
    // can now restore it
    //
    // note:  the new processes initially inherits the device map of
    // the parent process, but on Windows XP and later, when the parent
    // replaces the primary token of the child, the device map is
    // dereferenced.  the deref does not occur on Windows 2000, but
    // we do it there as well, just in case
    //

    if (*DeviceMap96) {

        //
        // DeviceMap96 points to a device map to set in this process
        //

        PROCESS_DEVICEMAP_INFORMATION info;

        RtlInitUnicodeString(&objname, DeviceMap96);

        status = NtOpenDirectoryObject(
            &info.Set.DirectoryHandle, DIRECTORY_TRAVERSE, &objattrs);

        if (status == STATUS_ACCESS_DENIED) {

            //
            // the process may not be able to open its own device map,
            // for example if the device map has a DACL that specifies
            // only Administrators, and Drop Rights removed that group.
            // work around that by getting SbieSvc to open the device map
            //

            PROCESS_OPEN_DEVICE_MAP_REQ req;
            req.h.length = sizeof(PROCESS_OPEN_DEVICE_MAP_REQ);
            req.h.msgid = MSGID_PROCESS_OPEN_DEVICE_MAP;
            req.DirectoryHandlePtr = (ULONG_PTR)&info.Set.DirectoryHandle;
            wcscpy(req.DirectoryName, DeviceMap96);
            rpl = SbieDll_CallServer(&req.h);
            if (rpl) {
                status = rpl->status;
                Dll_Free(rpl);
            }
        }

        if (! NT_SUCCESS(status)) {

            SbieApi_Log(2321, _fmt, status, 0x88);

        } else {

            status = NtSetInformationProcess(
                NtCurrentProcess(), ProcessDeviceMap,
                &info, sizeof(info.Set));

#ifndef _WIN64
            if (status == STATUS_INFO_LENGTH_MISMATCH && Dll_IsWow64) {

                //
                // 32-bit process on 64-bit Windows can't set its device map
                // due to an error in the wow64 api layer, so ask SbieSvc
                // to do it for us.  see also core/svc/ProcessServer.cpp
                //

                PROCESS_SET_DEVICE_MAP_REQ req;
                req.h.length = sizeof(PROCESS_SET_DEVICE_MAP_REQ);
                req.h.msgid = MSGID_PROCESS_SET_DEVICE_MAP;
                req.DirectoryHandle =
                            (ULONG64)(ULONG_PTR)info.Set.DirectoryHandle;
                rpl = SbieDll_CallServer(&req.h);
                if (! rpl)
                    status = STATUS_SERVER_DISABLED;
                else {
                    status = rpl->status;
                    Dll_Free(rpl);
                }
            }
#endif

            NtClose(info.Set.DirectoryHandle);

            if (! NT_SUCCESS(status)) {

                SbieApi_Log(2321, _fmt, status, 0x77);
            }
        }

    } else {

        //
        // Query the device map of the current process into DeviceMap64
        //

        HANDLE handle;

        RtlInitUnicodeString(&objname, L"\\??");

        status = NtOpenDirectoryObject(&handle, DIRECTORY_QUERY, &objattrs);

        if (status == STATUS_ACCESS_DENIED) {

            //
            // on Windows with User Account Control, it is possible for
            // the directory object to be accessible only to Administrators,
            // so ask the driver to open it for us
            //

            status = SbieApi_OpenDeviceMap(&handle);
        }

        if (! NT_SUCCESS(status)) {

            SbieApi_Log(2321, _fmt, status, 0x11);

        } else {

            WCHAR dirname[128];
            ULONG length = sizeof(dirname);
            status = NtQueryObject(
                handle, ObjectNameInformation, dirname, length, &length);

            NtClose(handle);

            if (! NT_SUCCESS(status)) {

                SbieApi_Log(2321, _fmt, status, 0x22);

            } else {

                UNICODE_STRING *uni =
                    &((OBJECT_NAME_INFORMATION *)dirname)->Name;
                length = uni->Length / sizeof(WCHAR);
                if (length > 95)
                    length = 95;
                wmemcpy(DeviceMap96, uni->Buffer, length);
                DeviceMap96[length] = L'\0';
            }
        }
    }
}
