/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2023 David Xanatos, xanasoft.com
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
// File
//---------------------------------------------------------------------------

#define NOGDI
#include "dll.h"
#include "obj.h"
#include "handle.h"
#include <stdio.h>
#include <dbt.h>
#include "core/svc/FileWire.h"
#include "core/svc/InteractiveWire.h"
#include "core/svc/UserWire.h"
#include "debug.h"
#include "trace.h"

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define FILE_DENIED_ACCESS ~(                                           \
    STANDARD_RIGHTS_READ | GENERIC_READ | SYNCHRONIZE | READ_CONTROL |  \
    FILE_READ_DATA | FILE_READ_EA | FILE_READ_ATTRIBUTES |              \
    GENERIC_EXECUTE | FILE_EXECUTE)

#define DIRECTORY_JUNCTION_ACCESS (                                     \
    GENERIC_ALL | GENERIC_WRITE | MAXIMUM_ALLOWED |                     \
    FILE_APPEND_DATA | FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES)


#define DELETE_MARK_LOW   0xDEAD44A0
#define DELETE_MARK_HIGH  0x01B01234

#define IS_DELETE_MARK(p_large_integer)                 \
    ((p_large_integer)->HighPart == DELETE_MARK_HIGH && \
     (p_large_integer)->LowPart  == DELETE_MARK_LOW)


#define TYPE_DIRECTORY      FILE_DIRECTORY_FILE
#define TYPE_FILE           FILE_NON_DIRECTORY_FILE
#define TYPE_DELETED        FILE_DELETE_ON_CLOSE
#define TYPE_READ_ONLY      FILE_RESERVE_OPFILTER
#define TYPE_SYSTEM         FILE_OPEN_FOR_FREE_SPACE_QUERY
#define TYPE_REPARSE_POINT  FILE_OPEN_REPARSE_POINT
#define TYPE_EFS            FILE_ATTRIBUTE_ENCRYPTED


#define OBJECT_ATTRIBUTES_ATTRIBUTES                            \
    (ObjectAttributes                                           \
        ? ObjectAttributes->Attributes | OBJ_CASE_INSENSITIVE   \
        : 0)


#ifdef _WIN64
#define PROXY_PIPE_MASK     0xFFFFFFFFFFFFFF00
#else
#define PROXY_PIPE_MASK     0xFFFFFF00
#endif


#define FGN_IS_BOXED_PATH           0x0001
#define FGN_TRAILING_BACKSLASH      0x0002
#define FGN_NETWORK_SHARE           0x0004
#define FGN_REPARSED_OPEN_PATH      0x0100
#define FGN_REPARSED_CLOSED_PATH    0x0200
#define FGN_REPARSED_WRITE_PATH     0x0400

#define NO_RELOCATION               ((PUNICODE_STRING)-1)

#ifndef  _WIN64
#define WOW64_FS_REDIR
#endif ! _WIN64


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


struct _FILE_DRIVE;
struct _FILE_LINK;
struct _FILE_GUID;
typedef struct _FILE_LINK FILE_LINK;
typedef struct _FILE_DRIVE FILE_DRIVE;
typedef struct _FILE_GUID FILE_GUID;



//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

static ULONG File_FindBoxPrefix(const WCHAR* Path);

NTSTATUS File_GetCopyPath(WCHAR *TruePath, WCHAR **OutCopyPath);

NTSTATUS File_GetTruePath(WCHAR *CopyPath, WCHAR **OutTruePath);

WCHAR* File_FindSnapshotPath(WCHAR* CopyPath);

SBIEDLL_EXPORT NTSTATUS File_GetName(
    HANDLE RootDirectory, UNICODE_STRING *ObjectName,
    WCHAR **OutTruePath, WCHAR **OutCopyPath, ULONG *OutFlags);

static WCHAR *File_TranslateDosToNtPath2(
    const WCHAR *DosPath, ULONG DosPathLen);

static WCHAR *File_GetName_TranslateSymlinks(
    THREAD_DATA *TlsData, const WCHAR *objname_buf, ULONG objname_len,
    BOOLEAN *translated);

static WCHAR *File_GetName_ExpandShortNames(
    THREAD_DATA *TlsData, WCHAR *Path);

static BOOLEAN File_GetName_ConvertLinks(
    THREAD_DATA *TlsData, WCHAR **OutTruePath, BOOLEAN ConvertWow64Link);

static void File_GetName_FixTruePrefix(
    THREAD_DATA *TlsData,
    WCHAR **OutTruePath, ULONG *InOutLength,
    ULONG old_prefix_len,
    const WCHAR *new_prefix, ULONG new_prefix_len);

#ifdef WOW64_FS_REDIR
static ULONG File_GetName_SkipWow64Link(const WCHAR *name);
#endif WOW64_FS_REDIR

static NTSTATUS File_GetName_FromFileId(
    OBJECT_ATTRIBUTES *ObjectAttributes,
    WCHAR **OutTruePath, WCHAR **OutCopyPath);

static ULONG File_MatchPath(const WCHAR *path, ULONG *FileFlags);

static ULONG File_MatchPath2(const WCHAR *path, ULONG *FileFlags, BOOLEAN bCheckObjectExists, BOOLEAN bMonitorLog);

static NTSTATUS File_AddCurrentUserToSD(PSECURITY_DESCRIPTOR *pSD);

static NTSTATUS File_NtOpenFile(
    HANDLE *FileHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    IO_STATUS_BLOCK *IoStatusBlock,
    ULONG ShareAccess,
    ULONG OpenOptions);

static NTSTATUS File_NtCreateFile(
    HANDLE *FileHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    IO_STATUS_BLOCK *IoStatusBlock,
    LARGE_INTEGER *AllocationSize,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    void *EaBuffer,
    ULONG EaLength);

static NTSTATUS File_NtCreateFileImpl(
    HANDLE *FileHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    IO_STATUS_BLOCK *IoStatusBlock,
    LARGE_INTEGER *AllocationSize,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    void *EaBuffer,
    ULONG EaLength);

static NTSTATUS File_NtCreateTrueFile(
    HANDLE *FileHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    IO_STATUS_BLOCK *IoStatusBlock,
    LARGE_INTEGER *AllocationSize,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    void *EaBuffer,
    ULONG EaLength);

static NTSTATUS File_NtCreateCopyFile(
    PHANDLE FileHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PIO_STATUS_BLOCK IoStatusBlock,
    PLARGE_INTEGER AllocationSize,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    PVOID EaBuffer,
    ULONG EaLength);

static NTSTATUS File_NtCreateFileProxy(
    HANDLE *FileHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    IO_STATUS_BLOCK *IoStatusBlock,
    LARGE_INTEGER *AllocationSize,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    void *EaBuffer,
    ULONG EaLength);

static NTSTATUS File_CheckCreateParameters(
    ACCESS_MASK DesiredAccess, ULONG CreateDisposition,
    ULONG CreateOptions, ULONG FileType);

static NTSTATUS File_GetFileType(
    OBJECT_ATTRIBUTES *ObjectAttributes, BOOLEAN IsWritePath,
    ULONG *FileType, BOOLEAN *IsEmpty);

static BOOLEAN File_CheckDeletedParent(WCHAR *CopyPath);

static NTSTATUS File_CreatePath(WCHAR *TruePath, WCHAR *CopyPath);

static NTSTATUS File_MigrateFile(
    const WCHAR *TruePath, const WCHAR *CopyPath,
    BOOLEAN IsWritePath, BOOLEAN WithContents);

static NTSTATUS File_MigrateJunction(
    const WCHAR *TruePath, const WCHAR *CopyPath,
    BOOLEAN IsWritePath);

static NTSTATUS File_CopyShortName(
    const WCHAR *TruePath, const WCHAR *CopyPath);

static BOOLEAN File_AdjustShortName(
    const WCHAR *TruePath, const WCHAR *CopyPath, HANDLE FileHandle);

static NTSTATUS File_SetCreateTime(HANDLE FileHandle, const WCHAR *CopyPath);

static NTSTATUS File_MarkDeleted(HANDLE FileHandle, const WCHAR *CopyPath);

static NTSTATUS File_QueryFullAttributesDirectoryFile(
    const WCHAR *TruePath, FILE_NETWORK_OPEN_INFORMATION *FileInformation);

static ULONG File_CheckDepthForIsWritePath(const WCHAR *TruePath);

static NTSTATUS File_NtQueryAttributesFile(
    OBJECT_ATTRIBUTES *ObjectAttributes,
    FILE_BASIC_INFORMATION *FileInformation);

static NTSTATUS File_NtQueryFullAttributesFile(
    OBJECT_ATTRIBUTES *ObjectAttributes,
    FILE_NETWORK_OPEN_INFORMATION *FileInformation);

static NTSTATUS File_NtQueryFullAttributesFileImpl(
    OBJECT_ATTRIBUTES *ObjectAttributes,
    FILE_NETWORK_OPEN_INFORMATION *FileInformation);

static NTSTATUS File_NtQueryInformationFile(
    HANDLE FileHandle,
    IO_STATUS_BLOCK *IoStatusBlock,
    void *FileInformation,
    ULONG Length,
    FILE_INFORMATION_CLASS FileInformationClass);

static NTSTATUS File_NtQueryInformationByName(
    POBJECT_ATTRIBUTES ObjectAttributes,
    PIO_STATUS_BLOCK IoStatusBlock,
    PVOID FileInformation,
    ULONG Length,
    FILE_INFORMATION_CLASS FileInformationClass
);

static ULONG File_GetFinalPathNameByHandleW(
    HANDLE hFile, WCHAR *lpszFilePath, ULONG cchFilePath, ULONG dwFlags);

static WCHAR *File_GetFinalPathNameByHandleW_2(
    WCHAR *TruePath, ULONG dwFlags);

static WCHAR *File_GetFinalPathNameByHandleW_3(
    WCHAR *TruePath, ULONG TruePath_len);

static NTSTATUS File_NtSetInformationFile(
    HANDLE FileHandle,
    IO_STATUS_BLOCK *IoStatusBlock,
    void *FileInformation,
    ULONG Length,
    FILE_INFORMATION_CLASS FileInformationClass);

static NTSTATUS File_SetAttributes(
    HANDLE FileHandle, const WCHAR *CopyPath,
    FILE_BASIC_INFORMATION *Information);

NTSTATUS File_SetDisposition(
    HANDLE FileHandle, IO_STATUS_BLOCK *IoStatusBlock,
    void *FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass);

static NTSTATUS File_NtDeleteFile(OBJECT_ATTRIBUTES *ObjectAttributes);

static NTSTATUS File_NtDeleteFileImpl(OBJECT_ATTRIBUTES *ObjectAttributes);

static WCHAR *File_ConcatPath2(
    const WCHAR *Path1, ULONG Path1Len, const WCHAR *Path2, ULONG Path2Len);

static WCHAR* File_CanonizePath(
    const wchar_t* absolute_path, ULONG abs_path_len, const wchar_t* relative_path, ULONG rel_path_len);

static NTSTATUS File_OpenForRenameFile(
    HANDLE* pSourceHandle, const WCHAR *TruePath);

static NTSTATUS File_RenameFile(
    HANDLE FileHandle, void *info, BOOLEAN LinkOp);

static BOOLEAN File_RecordRecover(HANDLE FileHandle, const WCHAR *TruePath);

static NTSTATUS File_SetReparsePoint(
    HANDLE FileHandle, PREPARSE_DATA_BUFFER Data, ULONG DataLen);

NTSTATUS File_GetFileName(HANDLE FileHandle, ULONG NameLen, WCHAR *NameBuf);

//---------------------------------------------------------------------------


static P_NtOpenFile                 __sys_NtOpenFile                = NULL;
       P_NtCreateFile               __sys_NtCreateFile              = NULL;
static P_NtQueryAttributesFile      __sys_NtQueryAttributesFile     = NULL;
static P_NtQueryFullAttributesFile  __sys_NtQueryFullAttributesFile = NULL;
static P_NtQueryInformationFile     __sys_NtQueryInformationFile    = NULL;
static P_NtQueryInformationByName   __sys_NtQueryInformationByName  = NULL;
       P_GetFinalPathNameByHandle   __sys_GetFinalPathNameByHandleW = NULL;
       P_NtQueryDirectoryFile       __sys_NtQueryDirectoryFile      = NULL;
static P_NtQueryDirectoryFileEx     __sys_NtQueryDirectoryFileEx    = NULL;
static P_NtSetInformationFile       __sys_NtSetInformationFile      = NULL;
static P_NtDeleteFile               __sys_NtDeleteFile              = NULL;
       P_NtClose                    __sys_NtClose                   = NULL;
static P_NtCreateNamedPipeFile      __sys_NtCreateNamedPipeFile     = NULL;
static P_NtCreateMailslotFile       __sys_NtCreateMailslotFile      = NULL;
static P_NtReadFile                 __sys_NtReadFile                = NULL;
static P_NtWriteFile                __sys_NtWriteFile               = NULL;
static P_NtFsControlFile            __sys_NtFsControlFile           = NULL;
       P_NtDeviceIoControlFile      __sys_NtDeviceIoControlFile     = NULL;
static P_RtlGetCurrentDirectory_U   __sys_RtlGetCurrentDirectory_U  = NULL;
static P_RtlSetCurrentDirectory_U   __sys_RtlSetCurrentDirectory_U  = NULL;
static P_RtlGetFullPathName_U       __sys_RtlGetFullPathName_U      = NULL;
static P_RtlGetFullPathName_UEx     __sys_RtlGetFullPathName_UEx    = NULL;
static P_NtQueryVolumeInformationFile
                                    __sys_NtQueryVolumeInformationFile
                                                                    = NULL;


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


// Windows 2000 and Windows XP name for the LanmanRedirector device
static const WCHAR *File_Redirector = L"\\device\\lanmanredirector\\";
static const ULONG File_RedirectorLen = 25;

// Windows Vista name for the LanmanRedirector device
static const WCHAR *File_MupRedir = L"\\device\\mup\\;lanmanredirector\\";
static const ULONG File_MupRedirLen = 30;

static const WCHAR *File_DfsClientRedir = L"\\device\\mup\\dfsclient\\";
static const ULONG  File_DfsClientRedirLen = 22;

static const WCHAR *File_HgfsRedir = L"\\device\\mup\\;hgfs\\";
static const ULONG  File_HgfsRedirLen = 18;

       const WCHAR *File_Mup = L"\\device\\mup\\";
static const ULONG File_MupLen = 12;

       const WCHAR *File_BQQB = L"\\??\\";

static const ULONG _DeviceLen = 8;
static const WCHAR *_Share = L"\\share\\";
static const ULONG _ShareLen = 7;
static const WCHAR *_Drive = L"\\drive\\";
static const ULONG _DriveLen = 7;

static const WCHAR *_User = L"\\user";
static const ULONG _UserLen = 5;
static const WCHAR *_UserAll = L"\\user\\all";
static const ULONG _UserAllLen = 9;
static const WCHAR *_UserCurrent = L"\\user\\current";
static const ULONG _UserCurrentLen = 13;
static const WCHAR *_UserPublic = L"\\user\\public";
static const ULONG _UserPublicLen = 12;

#ifdef WOW64_FS_REDIR
static WCHAR *File_Wow64System32 = NULL;
static ULONG  File_Wow64System32Len = 0;
static WCHAR *File_Wow64SysNative = NULL;
static ULONG  File_Wow64SysNativeLen = 0;
static FILE_LINK *File_Wow64FileLink = NULL;
static void *File_Wow64DisableWow64FsRedirection = NULL;
static void *File_Wow64RevertWow64FsRedirection = NULL;
#endif WOW64_FS_REDIR

//static WCHAR *File_SysVolume = NULL;
//static ULONG File_SysVolumeLen = 0;

static WCHAR *File_AllUsers = NULL;
static ULONG File_AllUsersLen = 0;

static WCHAR *File_CurrentUser = NULL;
static ULONG File_CurrentUserLen = 0;

static WCHAR *File_PublicUser = NULL;
static ULONG File_PublicUserLen = 0;

static BOOLEAN File_DriveAddSN = FALSE;
static BOOLEAN File_UseVolumeGuid = FALSE;

BOOLEAN File_Delete_v2 = FALSE;

static WCHAR *File_AltBoxPath = NULL;
static ULONG File_AltBoxPathLen = 0;



//---------------------------------------------------------------------------
// File (other modules)
//---------------------------------------------------------------------------


#include <winioctl.h>
#include "file_link.c"
#include "file_pipe.c"
#include "file_del.c"
#include "file_snapshots.c"
#include "file_dir.c"
#include "file_recovery.c"
#include "file_misc.c"
#include "file_copy.c"
#include "file_init.c"


//---------------------------------------------------------------------------
// File_FindBoxPrefix
//---------------------------------------------------------------------------


_FX ULONG File_FindBoxPrefix(const WCHAR* Path)
{
    ULONG length = wcslen(Path);

    struct {
        const WCHAR* Path;
        ULONG PathLen;
    } BoxFilePaths[3] = {
        Dll_BoxFilePath, Dll_BoxFilePathLen,
        Dll_BoxFileRawPath, Dll_BoxFileRawPathLen,
        File_AltBoxPath, File_AltBoxPathLen // ToDo: deprecated, remove - raw path is more reliable and covers all cases
    }, *Dll_BoxFile;

    for (int i = 0; i < ARRAYSIZE(BoxFilePaths); i++) {

        Dll_BoxFile = &BoxFilePaths[i];
        if (!Dll_BoxFile->Path)
            continue;
    
        if (length >= Dll_BoxFile->PathLen &&
                0 == Dll_NlsStrCmp(
                    Path, Dll_BoxFile->Path, Dll_BoxFile->PathLen))
        {
            //if (Prefix) *Prefix = Dll_BoxFile->Path;
            return Dll_BoxFile->PathLen;
        }
    }

	return 0;
}


//---------------------------------------------------------------------------
// File_GetCopyPathImpl
//---------------------------------------------------------------------------


_FX NTSTATUS File_GetCopyPathImpl(WCHAR* TruePath, WCHAR **OutCopyPath, ULONG *OutFlags, WCHAR* snapshot_id, BOOLEAN have_trailing_backslash, BOOLEAN* p_add_trailing_backslash)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    ULONG length;
    WCHAR* name;
    const FILE_DRIVE *drive;
    const FILE_GUID* guid;
    ULONG PrefixLength;
    
    length = wcslen(TruePath);
    name = Dll_GetTlsNameBuffer(
                TlsData, COPY_NAME_BUFFER, Dll_BoxFilePathLen + length);

    *OutCopyPath = name;

    wmemcpy(name, Dll_BoxFilePath, Dll_BoxFilePathLen);
    name += Dll_BoxFilePathLen;

    //
    // if we requested real paths, re add the snapshot prefix
    //

    if (snapshot_id && *snapshot_id) {

        *name++ = L'\\';
        wmemcpy(name, File_Snapshot_Prefix, File_Snapshot_PrefixLen);
        name += File_Snapshot_PrefixLen;
        ULONG len = wcslen(snapshot_id);
        wmemcpy(name, snapshot_id, len);
        name += len;
    }


    //
    // if the true path points to a remote share or mapped drive,
    // convert that to box the portable form "\share\computer\folder"
    //

    PrefixLength = 0;
    if (length >= File_RedirectorLen && _wcsnicmp(TruePath, File_Redirector, File_RedirectorLen) == 0)
        PrefixLength = File_RedirectorLen;
    else if (length >= File_DfsClientRedirLen && _wcsnicmp(TruePath, File_DfsClientRedir, File_DfsClientRedirLen) == 0)
        PrefixLength = File_DfsClientRedirLen;
    else if (length >= File_HgfsRedirLen && _wcsnicmp(TruePath, File_HgfsRedir, File_HgfsRedirLen) == 0)
        PrefixLength = File_HgfsRedirLen;
    else if (length >= File_MupRedirLen && _wcsnicmp(TruePath, File_MupRedir, File_MupRedirLen) == 0)
        PrefixLength = File_MupRedirLen;

    if (PrefixLength) {

        WCHAR *ptr = TruePath + PrefixLength;
        if (*ptr == L';') {
            ptr = wcschr(ptr, L'\\');
            if (! ptr)
                return STATUS_BAD_INITIAL_PC;
            ++ptr;
        }

        wmemcpy(name, _Share, _ShareLen);
        name += _ShareLen;

        length = wcslen(ptr);
        wmemcpy(name, ptr, length);

        if (OutFlags)
            *OutFlags |= FGN_NETWORK_SHARE;

    // does this next section really need to be different than above?
    } else if (length >= File_MupLen &&
        _wcsnicmp(TruePath, File_Mup, File_MupLen) == 0) {

        WCHAR *ptr = TruePath + File_MupLen;
        if (*ptr == L';')   // like \Device\Mup\;RdpDr;:2\...
            return STATUS_BAD_INITIAL_PC;
        ptr = wcschr(ptr, L'\\');
        if (File_IsPipeSuffix(ptr))
            return STATUS_BAD_INITIAL_PC;

        wmemcpy(name, _Share, _ShareLen);
        name += _ShareLen;

        length -= File_MupLen;
        wmemcpy(name, TruePath + File_MupLen, length);

        if (OutFlags)
            *OutFlags |= FGN_NETWORK_SHARE;
    }

    //
    // if the true path begins with the full path to the home folder
    // for the AllUsers or for the current user, then we translate
    // the copy path to the box portable form "\user\all" or
    // "\user\current", respectively
    //

    else if (File_AllUsersLen && length >= File_AllUsersLen &&
                0 == Dll_NlsStrCmp(
                        TruePath, File_AllUsers, File_AllUsersLen))
    {
        wmemcpy(name, _UserAll, _UserAllLen);
        name += _UserAllLen;

        length -= File_AllUsersLen;
        wmemcpy(name, TruePath + File_AllUsersLen, length);

    }

    else if (File_CurrentUserLen && length >= File_CurrentUserLen &&
                0 == Dll_NlsStrCmp(
                        TruePath, File_CurrentUser, File_CurrentUserLen))
    {
        wmemcpy(name, _UserCurrent, _UserCurrentLen);
        name += _UserCurrentLen;

        length -= File_CurrentUserLen;
        wmemcpy(name, TruePath + File_CurrentUserLen, length);

    }

    else if (File_PublicUserLen && length >= File_PublicUserLen &&
                0 == Dll_NlsStrCmp(
                        TruePath, File_PublicUser, File_PublicUserLen))
    {
        wmemcpy(name, _UserPublic, _UserPublicLen);
        name += _UserPublicLen;

        length -= File_PublicUserLen;
        wmemcpy(name, TruePath + File_PublicUserLen, length);
    }

    //
    // otherwise, if the true path begins with the NT path for one of
    // the known DosDevices drives, then translate to the box portable
    // form "\drive\x"
    //

    else {

        ULONG drive_len;

        guid = NULL;
        drive = File_GetDriveForPath(TruePath, length);
        if (drive)
            drive_len = drive->len;
        else
            drive = File_GetDriveForUncPath(TruePath, length, &drive_len);
        if (!drive && File_UseVolumeGuid)
            guid = File_GetGuidForPath(TruePath, length);

        if (drive || guid) {

            WCHAR drive_letter = 0;
            WCHAR sn[10] = { 0 };
            WCHAR drive_guid[38 + 1];

            if (drive) {
                drive_letter = drive->letter;
                wcscpy(sn, drive->sn);
            }
            else { // if guid
                wcscpy(drive_guid, guid->guid);
                drive_len = guid->len;
            }

            LeaveCriticalSection(File_DrivesAndLinks_CritSec);

            wmemcpy(name, _Drive, _DriveLen);
            name += _DriveLen;
            if (drive_letter) {

                *name = drive_letter;
                ++name;
                
                if (File_DriveAddSN && *sn) {

                    *name = L'~';
                    ++name;
                    wcscpy(name, sn);
                    name += 9;
                }

                *name = L'\0';
            }
            else { // if guid

                wcscpy(name, drive_guid);
                name += wcslen(drive_guid); // = 38
            }

            if (length == drive_len) {

                //
                // in the special case of a request to open the
                // volume device itself, rather than any file within
                // the device, we return a special status code
                //

                if (! have_trailing_backslash)
                    return STATUS_BAD_INITIAL_PC;

                //
                // otherwise, caller must want to open the root
                // directory of the device, so remember to add the
                // trailing backslash before we're done
                //

                if (p_add_trailing_backslash) *p_add_trailing_backslash = TRUE;
            }

            length -= drive_len;
            wmemcpy(name, TruePath + drive_len, length);

        } else {

            //
            // if we couldn't find any matching logical drive, then
            // we return STATUS_BAD_INITIAL_PC so this DLL does not
            // try any further sandboxing.  (But the driver will still
            // block any attempt to access disk devices.)
            //

            return STATUS_BAD_INITIAL_PC;
        }
    }

    //
    // null-terminate the copy path, and add the missing trailing
    // backslash to the true path, if there was one
    //

    name += length;
    *name = L'\0';

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// File_GetCopyPath
//---------------------------------------------------------------------------


_FX NTSTATUS File_GetCopyPath(WCHAR* TruePath, WCHAR **OutCopyPath)
{
    return File_GetCopyPathImpl(TruePath, OutCopyPath, NULL, NULL, FALSE, NULL);
}


//---------------------------------------------------------------------------
// File_GetTruePathImpl
//---------------------------------------------------------------------------


_FX NTSTATUS File_GetTruePathImpl(ULONG length, WCHAR **OutTruePath, ULONG *OutFlags, BOOLEAN* p_is_boxed_path, BOOLEAN no_relocation, WCHAR* snapshot_id, BOOLEAN* p_convert_links_again)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    ULONG prefixLen = 0;
    WCHAR* name;
    const FILE_DRIVE *drive;
    const FILE_GUID* guid;

check_sandbox_prefix:

    prefixLen = File_FindBoxPrefix(*OutTruePath);
    if (prefixLen) {

        *OutTruePath += prefixLen;
        length -= prefixLen;

        if (! length) {
            //
            // caller specified just the sandbox prefix
            //
            *OutTruePath = NULL;
            return STATUS_BAD_INITIAL_PC;
        }

        if (OutFlags)
            *OutFlags |= FGN_IS_BOXED_PATH;
        *p_is_boxed_path = TRUE;
    }
     
	//
	// If its a sandboxed file, check if its in the current image or in a snapshot
	// If its in a snapshot remove the snapshot prefix
	//

	if (p_is_boxed_path) {
		if (length >= 10 && 0 == Dll_NlsStrCmp(*OutTruePath + 1, File_Snapshot_Prefix, File_Snapshot_PrefixLen))
		{
			WCHAR* path = wcschr(*OutTruePath + 1 + File_Snapshot_PrefixLen, L'\\');
			if (path == NULL) {
				//
				// caller specified just the sandbox snapshot prefix, or the path is to long
				//
                *OutTruePath = NULL;
				return STATUS_BAD_INITIAL_PC;
			}

            if (no_relocation) {
                ULONG len = (ULONG)(path - (*OutTruePath + 1 + File_Snapshot_PrefixLen));
                if (len < FILE_MAX_SNAPSHOT_ID) {
                    wmemcpy(snapshot_id, *OutTruePath + 1 + File_Snapshot_PrefixLen, len);
                    snapshot_id[len] = L'\0';
                }
            }

			length -= (ULONG)(path - *OutTruePath);
			*OutTruePath = path;
		}
	}

    //
    // the true path may now begin with "\drive\x", for instance,
    // if the process specified a RootDirectory handle that leads
    // inside the box.  we have to change this box convention to
    // full NT path of the drive letter.  a later section of code
    // will change it back to \drive\x for the copy path.
    //
    // note that we temporarily use the COPY_NAME_BUFFER here, but
    // that's ok because it hasn't been initialized yet
    //

    if (length >= (_DriveLen - 1) &&
        _wcsnicmp(*OutTruePath, _Drive, _DriveLen - 1) == 0)
    {
        name = (*OutTruePath);

        drive = NULL;
        guid = NULL;
        if (name[_DriveLen - 1] == L'\\') {
            if (name[_DriveLen] == L'{' && File_UseVolumeGuid)
                guid = File_GetLinkForGuid(&name[_DriveLen]);
            else
                drive = File_GetDriveForLetter(name[_DriveLen]);
        }
            

        if (drive) {

            ULONG len = _DriveLen + 1; /* drive letter */

            // skip any suffix after the drive letter
            if (File_DriveAddSN) {
                WCHAR* ptr = wcschr(*OutTruePath + _DriveLen + 1, L'\\');
                if (!ptr) ptr = wcschr(*OutTruePath + _DriveLen + 1, L'\0');
                len = (ULONG)(ptr - *OutTruePath);
            }

            File_GetName_FixTruePrefix(TlsData,
                OutTruePath, &length, len,
                drive->path, drive->len);
        }
        else if (guid) {

            ULONG len = _DriveLen + 38; /* drive guid*/

            File_GetName_FixTruePrefix(TlsData,
                OutTruePath, &length, len,
                guid->path, guid->len);
        }
        else {

            //
            // caller specified invalid path for \sandbox\drive\x
            //
            *OutTruePath = NULL;
            return STATUS_BAD_INITIAL_PC;
        }

        if (p_convert_links_again) *p_convert_links_again = TRUE;

        LeaveCriticalSection(File_DrivesAndLinks_CritSec);

        goto check_sandbox_prefix;
    }

    //
    // alternatively, the true path may begin with "\user\all" which,
    // is a box convention for the AllUsers home folder.  or, with
    // "\user\current", which is a box convention for the home folder
    // of the current user.  both cases must be translated similarly
    // to the "\drive\x" case above.
    //
    // note that we temporarily use the COPY_NAME_BUFFER here, but
    // that's ok because it hasn't been initialized yet
    //

    else if (length >= _UserLen &&
                _wcsnicmp(*OutTruePath, _User, _UserLen) == 0) {

        if (File_AllUsersLen && length >= _UserAllLen &&
               _wcsnicmp(*OutTruePath, _UserAll, _UserAllLen) == 0) {

            File_GetName_FixTruePrefix(TlsData,
                OutTruePath, &length, _UserAllLen,
                File_AllUsers, File_AllUsersLen);

        } else if (File_CurrentUserLen &&
                    length >= _UserCurrentLen && _wcsnicmp(
                        *OutTruePath, _UserCurrent, _UserCurrentLen) == 0) {

            File_GetName_FixTruePrefix(TlsData,
                OutTruePath, &length, _UserCurrentLen,
                File_CurrentUser, File_CurrentUserLen);

        } else if (File_PublicUserLen &&
                    length >= _UserPublicLen && _wcsnicmp(
                        *OutTruePath, _UserPublic, _UserPublicLen) == 0) {

            File_GetName_FixTruePrefix(TlsData,
                OutTruePath, &length, _UserPublicLen,
                File_PublicUser, File_PublicUserLen);

        } else {

            //
            // the path is "...\box\user..." but not for user\current or
            // user\all, so restore the sandbox prefix and return special
            // status for read-only access
            //

            name = Dll_GetTlsNameBuffer(
                        TlsData, TRUE_NAME_BUFFER,
                        (Dll_BoxFilePathLen + length + 1) * sizeof(WCHAR));

            wmemmove(name + Dll_BoxFilePathLen, *OutTruePath, length + 1);
            wmemcpy(name, Dll_BoxFilePath, Dll_BoxFilePathLen);

            *OutTruePath = name;

            return STATUS_BAD_INITIAL_PC;
        }

        if (p_convert_links_again) *p_convert_links_again = TRUE;
    }

    //
    // alternatively, the true path may begin with "\share\..." which,
    // is a box convention for remote shares.  in this case it has to
    // be translated similarly to the "\drive\x" case above.
    //

    else if (length >= _ShareLen  &&
             _wcsnicmp(*OutTruePath, _Share, _ShareLen) == 0) {

        File_GetName_FixTruePrefix(TlsData,
            OutTruePath, &length, _ShareLen,
            File_Mup, File_MupLen);

        if (p_convert_links_again) *p_convert_links_again = TRUE;
    }

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// File_GetTruePath
//---------------------------------------------------------------------------


_FX NTSTATUS File_GetTruePath(WCHAR *CopyPath, WCHAR **OutTruePath)
{
    ULONG length = wcslen(CopyPath);
    BOOLEAN is_boxed_path = FALSE;
    *OutTruePath = CopyPath;
    NTSTATUS status = File_GetTruePathImpl(length, OutTruePath, NULL, &is_boxed_path, FALSE, NULL, NULL);
    if (NT_SUCCESS(status) && !is_boxed_path)
        return STATUS_BAD_INITIAL_STACK; // indicate with this error code that the path provided was already the true path
    return status;
}


//---------------------------------------------------------------------------
// File_GetName
//---------------------------------------------------------------------------


_FX NTSTATUS File_GetName(
    HANDLE RootDirectory, UNICODE_STRING *ObjectName,
    WCHAR **OutTruePath, WCHAR **OutCopyPath, ULONG *OutFlags)
{	
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    NTSTATUS status;
    ULONG length;
    WCHAR *name, *TruePath;
    ULONG objname_len;
    WCHAR *objname_buf;
    const FILE_DRIVE *drive;
    const FILE_GUID* guid;
    BOOLEAN have_trailing_backslash, add_trailing_backslash;
    BOOLEAN have_tilde;
    BOOLEAN convert_links_again;
    BOOLEAN is_boxed_path;

#ifdef WOW64_FS_REDIR
    BOOLEAN convert_wow64_link = (File_Wow64FileLink) ? TRUE : FALSE;
#else
    const BOOLEAN convert_wow64_link = FALSE;
#endif WOW64_FS_REDIR
    BOOLEAN no_relocation = FALSE;
    WCHAR snapshot_id[FILE_MAX_SNAPSHOT_ID];
    snapshot_id[0] = L'\0';

    *OutTruePath = NULL;
    *OutCopyPath = NULL;
    if (OutFlags)
        *OutFlags = 0;

    if (ObjectName == NO_RELOCATION) {
        no_relocation = TRUE;
        ObjectName = NULL;
    }

    if (ObjectName) {
        objname_len = ObjectName->Length & ~1;
        objname_buf = ObjectName->Buffer;
    } else {
        objname_len = 0;
        objname_buf = NULL;
    }

    drive = NULL;
    guid = NULL;

    //
    // if a root handle is specified, we query the full name of the
    // root file, and append the ObjectName
    //

    if (RootDirectory) {

        UNICODE_STRING *uni = NULL;

        length = 256;
        name = Dll_GetTlsNameBuffer(
                        TlsData, TRUE_NAME_BUFFER, length + objname_len);

        status = Obj_GetObjectName(RootDirectory, name, &length);

        if (status == STATUS_OBJECT_PATH_INVALID && objname_len == 0) {

            //
            // special case:  if STATUS_OBJECT_PATH_INVALID is returned,
            // and the root directory turns out to be a File object,
            // and there is no object name, then:
            // this is most likely an anonynymous pipe, so return special
            // status STATUS_BAD_INITIAL_PC
            //

            if (Obj_GetObjectType(RootDirectory) == OBJ_TYPE_FILE) {

                name[0] = L'\0';
                name[1] = L'\0';
                *OutTruePath = name;
                return STATUS_BAD_INITIAL_PC;
            }
        }

        if (status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL || status == STATUS_INFO_LENGTH_MISMATCH) {

            name = Dll_GetTlsNameBuffer(
                        TlsData, TRUE_NAME_BUFFER, length + objname_len);

            status = Obj_GetObjectName(RootDirectory, name, &length);
        }

        if (! NT_SUCCESS(status))
            return status;

        uni = &((OBJECT_NAME_INFORMATION *)name)->Name;

#ifdef WOW64_FS_REDIR
        //
        // if the root directory handle references System32 in a WOW64
        // process, for example as a result of opening SysNative, then
        // we should not convert that System32 back to SysWow64
        //

        if (uni->Buffer && convert_wow64_link) {

            const ULONG sys32len = File_Wow64FileLink->src_len;

            name = uni->Buffer;
            length = uni->Length & ~1;
            name[length] = L'\0';

            if (length >= sys32len
                && _wcsnicmp(name, File_Wow64FileLink->src, sys32len) == 0
                && (name[sys32len] == L'\\' || name[sys32len] == L'\0')) {

                convert_wow64_link = FALSE;
            }

            else {

                //
                // if the file/directory is located in the sandbox, we still need to check the path
                //

                ULONG prefixLen = File_FindBoxPrefix(name);
                if (prefixLen != 0) {

                    name += prefixLen;
                    length -= prefixLen;

		            if (length >= 10 && 0 == Dll_NlsStrCmp(name + 1, File_Snapshot_Prefix, File_Snapshot_PrefixLen)) {
			            WCHAR* ptr = wcschr(name + 1 + File_Snapshot_PrefixLen, L'\\');
                        if (ptr) {
                            length -= (ULONG)(ptr - name);
                            name = ptr;
                        }
		            }

                    if(length >= File_Wow64System32Len
                        && _wcsnicmp(name, File_Wow64System32, File_Wow64System32Len) == 0
                        && (name[File_Wow64System32Len] == L'\\' || name[File_Wow64System32Len] == L'\0')) {

                        convert_wow64_link = FALSE;
                    }
                }
            }
        }
#endif WOW64_FS_REDIR

        if (uni->Buffer) {
            *OutTruePath = uni->Buffer;
            name = uni->Buffer + uni->Length / sizeof(WCHAR);
        } else
            *OutTruePath = name;

        if (objname_len) {

            if (*objname_buf != L':') {
                *name = L'\\';
                ++name;
            }

            memcpy(name, objname_buf, objname_len);
            name += objname_len / sizeof(WCHAR);
        }

        *name = L'\0';

        File_GetName_ConvertLinks(TlsData, OutTruePath, convert_wow64_link);

    //
    // if no root handle, then we only have the object name to
    // work with.  it may begin with a DosDevices name "\??\x:"
    // which we have to convert to a full path to the device
    //

    } else if (objname_len) {

        if (objname_len >= 6 * sizeof(WCHAR)) {
            if (objname_buf[0] == L'\\' && objname_buf[1] == L'?' &&
                objname_buf[2] == L'?'  && objname_buf[3] == L'\\' &&
                objname_buf[5] == L':')
            {
                drive = File_GetDriveForLetter(objname_buf[4]);
                if (! drive)
                    return STATUS_OBJECT_PATH_NOT_FOUND;
                objname_buf += 6;
                objname_len -= 6 * sizeof(WCHAR);
            }
        }

        if (drive) {

            //
            // convert a DosDevices name into a full NT path to
            // the device represented by the drive letter
            //

            name = Dll_GetTlsNameBuffer(
                        TlsData, TRUE_NAME_BUFFER,
                        objname_len + (drive->len + 1) * sizeof(WCHAR));

            *OutTruePath = name;

            wmemcpy(name, drive->path, drive->len);
            name += drive->len;

            memcpy(name, objname_buf, objname_len);
            name += objname_len / sizeof(WCHAR);
            *name = L'\0';

        } else {

            //
            // otherwise check if we were already given a full NT path
            // to a disk device.  if we find a drive here, it also prevents
            // the next section of code from trying to translate symlinks
            //

            guid = NULL;
            drive = File_GetDriveForPath(
                                objname_buf, objname_len / sizeof(WCHAR));
            if(!drive && File_UseVolumeGuid)
                guid = File_GetGuidForPath(objname_buf, objname_len / sizeof(WCHAR));

            if (drive || guid) {

                name = Dll_GetTlsNameBuffer(
                        TlsData, TRUE_NAME_BUFFER,
                        objname_len + sizeof(WCHAR));

                *OutTruePath = name;

                memcpy(name, objname_buf, objname_len);
                name += objname_len / sizeof(WCHAR);
                *name = L'\0';
            }
        }

        if (drive || guid) {

            File_GetName_ConvertLinks(
                TlsData, OutTruePath, convert_wow64_link);

            LeaveCriticalSection(File_DrivesAndLinks_CritSec);

        } else {

            BOOLEAN translated = FALSE;

            *OutTruePath = File_GetName_TranslateSymlinks(
                            TlsData, objname_buf, objname_len, &translated);

            if (! *OutTruePath)
                return STATUS_OBJECT_PATH_SYNTAX_BAD;

            translated |= File_GetName_ConvertLinks(
                                TlsData, OutTruePath, convert_wow64_link);

            if (! translated) {
                // remote shares prefixed by \Device\Mup are ok even
                // if they were not translated.  anything else means
                // a device that probably isn't a filesystem, although
                // it may also be a volume that is not mounted to a drive
                // letter, so check for a reparse point before failing
                if (_wcsnicmp(*OutTruePath, File_Mup, File_MupLen) != 0) {
                    if (! translated)
                        return STATUS_BAD_INITIAL_PC;
                }
            }
        }

    //
    // if no root handle, and no object name, then abort
    //

    } else
        return STATUS_OBJECT_PATH_SYNTAX_BAD;

    //
    // remove duplicate backslashes
    //

    name = *OutTruePath;
    length = wcslen(name);

    while (name[0]) {
        if (name[0] == L'\\' && name[1] == L'\\') {

            ULONG move_len = length - (ULONG)(name - *OutTruePath) + 1;
            wmemmove(name, name + 1, move_len);
            --length;

        } else
            ++name;
    }

    //
    // remove the trailing backslash.  only if the caller is trying to
    // open the root directory, we will be putting that backslash
    // back onto the true path, before returning
    //

    name = *OutTruePath;
    if (length && name[length - 1] == L'\\') {

        --length;
        name[length] = L'\0';
        have_trailing_backslash = TRUE;

        if (OutFlags)
            *OutFlags |= FGN_TRAILING_BACKSLASH;

    } else
        have_trailing_backslash = FALSE;

    add_trailing_backslash = FALSE;

    //
    // make sure the true path begins with the "\device\" prefix.
    // note that Windows returns more informative status codes here,
    // like STATUS_OBJECT_NAME_NOT_FOUND, STATUS_OBJECT_PATH_NOT_FOUND
    // and STATUS_OBJECT_TYPE_MISMATCH.  but we take the easy way out
    //

    if (length < _DeviceLen ||
            _wcsnicmp(*OutTruePath, File_Mup, _DeviceLen) != 0) {

        return STATUS_OBJECT_PATH_SYNTAX_BAD;
    }

    //
    // if this is a named pipe or mail slot, return special status
    //

    if (!drive && !guid && File_IsNamedPipe(*OutTruePath, NULL)) {

        return STATUS_BAD_INITIAL_PC;
    }

    //
    // expand short names in the true path, but only if contains a tilde
    //

    if (wcschr(*OutTruePath, L'~')) {

        have_tilde = TRUE;

        name = File_GetName_ExpandShortNames(TlsData, *OutTruePath);
        length = wcslen(name);
        *OutTruePath = name;

    } else
        have_tilde = FALSE;

    //
    // if the path leads inside the sandbox, we advance the pointer.
    //

    convert_links_again = FALSE;
    is_boxed_path = FALSE;

    TruePath = *OutTruePath;    // save pointer in case we need to restore

check_sandbox_prefix:

    status = File_GetTruePathImpl(length, OutTruePath, OutFlags, &is_boxed_path, no_relocation, snapshot_id, &convert_links_again);
    if (!NT_SUCCESS(status)) {
        if(*OutTruePath == NULL)
            *OutTruePath = TruePath;
        return status;
    }

    //
    // if we had a tilde, try short name expansion again, now that we
    // have translated prefixes like \user\current back into their
    // real path names
    //

    if (have_tilde) {
        name = File_GetName_ExpandShortNames(TlsData, *OutTruePath);
        *OutTruePath = name;
        length = wcslen(name);
    }

    //
    // final conversion of any links (volume reparse points), but only
    // in case TruePath was inside the sandbox and we adjusted it
    //

    if (convert_links_again) {
        File_GetName_ConvertLinks(TlsData, OutTruePath, convert_wow64_link);
        length = wcslen(*OutTruePath);
    }

    //
    // convert \Windows\SysNative to \Windows\System32
    //

#ifdef WOW64_FS_REDIR
    if (convert_wow64_link) {

        name = *OutTruePath;
        length = wcslen(name);

        if (length >= File_Wow64SysNativeLen
                && 0 == _wcsnicmp(
                        name, File_Wow64SysNative, File_Wow64SysNativeLen)
                && (name[File_Wow64SysNativeLen] == L'\\' ||
                        name[File_Wow64SysNativeLen] == L'\0')) {

            name = *OutTruePath;

            File_GetName_FixTruePrefix(
                TlsData, &name, &length,
                File_Wow64SysNativeLen,
                File_Wow64FileLink->src, File_Wow64FileLink->src_len);

            *OutTruePath = name;
        }
    }
#endif WOW64_FS_REDIR

    //
    // translate reparse points and use the resulting string
    // as the base for creating CopyPath
    //

    TruePath = File_TranslateTempLinks(*OutTruePath, TRUE);

    if (TruePath) {

        length = wcslen(TruePath);

        //
        // if the reparsed path now begins with the sandbox prefix then
        // we need to go back
        //

        if(File_FindBoxPrefix(TruePath))
            is_boxed_path = TRUE;
        
        name = Dll_GetTlsNameBuffer(
                TlsData, TRUE_NAME_BUFFER, (length + 1) * sizeof(WCHAR));
        wmemcpy(name, TruePath, length + 1);

        Dll_Free(TruePath);

        TruePath = name;
        *OutTruePath = TruePath;

        if (is_boxed_path) {
            convert_links_again = FALSE;

            goto check_sandbox_prefix;
        }

        //
        // otherwise test the reparsed path for open/closed paths and
        // then continue to create the copy path
        //

        if (OutFlags) {
            ULONG mp_flags = File_MatchPath(TruePath, OutFlags);
            if (PATH_IS_OPEN(mp_flags))
                *OutFlags |= FGN_REPARSED_OPEN_PATH;
            if (PATH_IS_CLOSED(mp_flags))
                *OutFlags |= FGN_REPARSED_CLOSED_PATH;
            if (PATH_IS_WRITE(mp_flags))
                *OutFlags |= FGN_REPARSED_WRITE_PATH;
        }

    } else
        TruePath = *OutTruePath;

    //
    // if this is a unboxed path, and we opened it by object,
    // check path relocation and update true path accordingly.
    //

    if (!is_boxed_path && RootDirectory && !no_relocation) {
      
        name = Handle_GetRelocationPath(RootDirectory, objname_len);
        if (name) {

            *OutTruePath = name;
            TruePath = *OutTruePath;

            name = (*OutTruePath) + wcslen(*OutTruePath);

            if (objname_len) {

                *name = L'\\';
                ++name;
                memcpy(name, objname_buf, objname_len);

                name += objname_len / sizeof(WCHAR);
            }

            *name = L'\0';
        }
    }

    //
    // now create the copy path, which is the box prefix prepended
    // to the true path that we have.  note that the copy path will
    // still be missing its null terminator.
    //

    status = File_GetCopyPathImpl(TruePath, OutCopyPath, OutFlags, snapshot_id, have_trailing_backslash, &add_trailing_backslash);
    if (!NT_SUCCESS(status))
        return status;

    if (add_trailing_backslash) {
        name = *OutTruePath;
        name += wcslen(name);
        name[0] = L'\\';
        name[1] = L'\0';
    }

    //
    // debugging helper
    //

    /*if (IsDebuggerPresent()) {
        OutputDebugString(*OutTruePath);
        OutputDebugString(L"\n");
    }*/

    /*if (_wcsicmp(Dll_ImageName, L"acrord32.exe") == 0) {
        ULONG len = wcslen(*OutTruePath) + 1;
        WCHAR *path = Dll_AllocTemp(len * sizeof(WCHAR));
        wmemcpy(path, *OutTruePath, len);
        _wcslwr(path);
        if (wcsstr(path, L"products.txt")) {
            while (! IsDebuggerPresent()) { OutputDebugString(L"BREAK\n"); Sleep(500); }
            __debugbreak();
        }
    }*/

    //
    // finish
    //

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// File_GetName_TranslateSymlinks
//---------------------------------------------------------------------------


_FX WCHAR *File_GetName_TranslateSymlinks(
    THREAD_DATA *TlsData, const WCHAR *objname_buf, ULONG objname_len,
    BOOLEAN *translated)
{
    NTSTATUS status;
    HANDLE handle;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR *name;
    ULONG path_len;
    const WCHAR *suffix;
    ULONG suffix_len;

    // This prevents an additional \device\pipename prefix being added by File_GetBoxedPipeName (thanks Chrome).
    if (objname_len > 26 && !_wcsnicmp(objname_buf, L"\\??\\pipe", 8)) {
        if (!_wcsnicmp(objname_buf + 8, File_NamedPipe, 17)) {
            objname_buf += 8;
        }
    }

    if (objname_len >= 18 && File_IsNamedPipe(objname_buf, NULL)) {
        handle = NULL;
        goto not_link;
    }

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    objname.Length = (USHORT)objname_len;
    objname.Buffer = (WCHAR *)objname_buf;

    //
    // try to open the longest symbolic link we find.  for instance,
    // if the object name is \??\PIPE\MyPipe, we will open the link
    // "\??\PIPE" even though "\??\" itself is also a link
    //

    while (1) {

        objname.MaximumLength = objname.Length;

        status = NtOpenSymbolicLinkObject(
            &handle, SYMBOLIC_LINK_QUERY, &objattrs);

        if (NT_SUCCESS(status))
            break;

        if (status == STATUS_ACCESS_DENIED &&
                                objname.Length <= 1020 * sizeof(WCHAR)) {

            //
            // if the object is a valid symbolic link but we don't have
            // access rights to open the symbolic link then we ask the
            // driver to query the link for us
            //

            WCHAR *path = Dll_AllocTemp(1024 * sizeof(WCHAR));
            memcpy(path, objname.Buffer, objname.Length);
            path[objname.Length / sizeof(WCHAR)] = L'\0';

            status = SbieApi_QuerySymbolicLink(path, 1024 * sizeof(WCHAR));
            if (NT_SUCCESS(status)) {

                path_len = wcslen(path) * sizeof(WCHAR);

                suffix = objname_buf + objname.Length / sizeof(WCHAR);
                suffix_len = objname_len - objname.Length;

                name = Dll_GetTlsNameBuffer(
                                TlsData, TRUE_NAME_BUFFER,
                                path_len + suffix_len + sizeof(WCHAR));

                memcpy(name, path, path_len);

                Dll_Free(path);
                goto copy_suffix;

            } else {

                Dll_Free(path);
                status = STATUS_ACCESS_DENIED;
            }
        }

        //
        // path was not a symbolic link, so chop off the
        // last path component before checking the path again
        //

        handle = NULL;

        if (objname.Length <= sizeof(WCHAR))
            break;

        do {
            objname.Length -= sizeof(WCHAR);
        } while (   objname.Length &&
                    objname_buf[objname.Length / sizeof(WCHAR)] != L'\\');

        if (objname.Length <= sizeof(WCHAR))
            break;
    }

    //
    // if we couldn't locate a symbolic link then we're done
    //

not_link:

    if (! handle) {

        name = Dll_GetTlsNameBuffer(
                    TlsData, TRUE_NAME_BUFFER,
                    objname_len + sizeof(WCHAR));

        memcpy(name, objname_buf, objname_len);
        *(name + objname_len / sizeof(WCHAR)) = L'\0';

        return name;
    }

    //
    // otherwise query the symbolic link into the true name buffer
    //

    suffix = objname_buf + objname.Length / sizeof(WCHAR);
    suffix_len = objname_len - objname.Length;

    memzero(&objname, sizeof(UNICODE_STRING));
    path_len = 0;
    status = NtQuerySymbolicLinkObject(handle, &objname, &path_len);
    if (status != STATUS_BUFFER_TOO_SMALL) {
        NtClose(handle);
        return NULL;
    }

    name = Dll_GetTlsNameBuffer(
                    TlsData, TRUE_NAME_BUFFER,
                    path_len + suffix_len + sizeof(WCHAR));

    objname.Length = (USHORT)(path_len - sizeof(WCHAR));
    objname.MaximumLength = (USHORT)path_len;
    objname.Buffer = name;
    status = NtQuerySymbolicLinkObject(handle, &objname, NULL);

    path_len = objname.Length;

    NtClose(handle);

    //
    // the true name buffer contains the expansion of the symbolic link,
    // copy the rest of it (the suffix) and invoke ourselves recursively
    //

copy_suffix:

    if (! NT_SUCCESS(status))
        name = NULL;

    else {

        WCHAR *name2 = name + path_len / sizeof(WCHAR);

        memcpy(name2, suffix, suffix_len);
        name2 += suffix_len / sizeof(WCHAR);
        *name2 = L'\0';

        //
        // copy the result to the copy name buffer, for the recursive
        // invocation of this function.  if the recursive invocation
        // doesn't find a symbolic link to translate, it will simply
        // put the result in the true name buffer, and return
        //

        name2 = Dll_GetTlsNameBuffer(
                    TlsData, COPY_NAME_BUFFER,
                    path_len + suffix_len + sizeof(WCHAR));

        memcpy(name2, name, path_len + suffix_len + sizeof(WCHAR));

        name = File_GetName_TranslateSymlinks(
                    TlsData, name2, path_len + suffix_len, translated);
        if (name)
            *translated = TRUE;
    }

    return name;
}


//---------------------------------------------------------------------------
// File_GetName_ExpandShortNames2
//---------------------------------------------------------------------------


_FX NTSTATUS File_GetName_ExpandShortNames2(
	WCHAR *Path, ULONG index, ULONG backslash_index, PFILE_BOTH_DIRECTORY_INFORMATION info, const ULONG info_size, FILE_SNAPSHOT* Cur_Snapshot)
{
	NTSTATUS status;

	UNICODE_STRING uni;
	OBJECT_ATTRIBUTES ObjAttrs;
	HANDLE handle;
	IO_STATUS_BLOCK IoStatusBlock;

	WCHAR* TmplName;

	WCHAR save_char;

	save_char = Path[backslash_index + 1];
	Path[backslash_index + 1] = L'\0';

	TmplName = File_MakeSnapshotPath(Cur_Snapshot, Path);
	if(TmplName != NULL)
		uni.Buffer = TmplName;
	else
		uni.Buffer = Path;
	uni.Length = wcslen(uni.Buffer) * sizeof(WCHAR);
	uni.MaximumLength = uni.Length + sizeof(WCHAR);

	InitializeObjectAttributes(
		&ObjAttrs, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

	status = __sys_NtCreateFile(
		&handle,
		GENERIC_READ | SYNCHRONIZE,     // DesiredAccess
		&ObjAttrs,
		&IoStatusBlock,
		NULL,                           // AllocationSize
		0,                              // FileAttributes
		FILE_SHARE_VALID_FLAGS,         // ShareAccess
		FILE_OPEN,                      // CreateDisposition
		FILE_DIRECTORY_FILE |           // CreateOptions
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,                           // EaBuffer
		0);                             // EaLength

	//
	// restore original path
	//

	Path[backslash_index + 1] = save_char;

	if (!NT_SUCCESS(status)) 
		return status;


	// query long name for short name.  if the short name is not
	// found with a status of NO_SUCH_FILE, then possibly it was
	// already deleted or does not even exist yet.  in this case
	// we leave the short name as is instead of failing.

	save_char = Path[index];
	Path[index] = L'\0';

	WCHAR ShortName[12 + 1];
	if (Cur_Snapshot && Cur_Snapshot->ScramKey && wcslen(&Path[backslash_index + 1]) <= 12)
	{
		//
		// If we are checking in a snapshot we need to unscramble the short name
		//

		wcscpy(ShortName, &Path[backslash_index + 1]);
		File_UnScrambleShortName(ShortName, Cur_Snapshot->ScramKey);
		uni.Buffer = ShortName;
	}
	else
		uni.Buffer = &Path[backslash_index + 1];
	uni.Length = wcslen(uni.Buffer) * sizeof(WCHAR);
	uni.MaximumLength = uni.Length + sizeof(WCHAR);

	status = __sys_NtQueryDirectoryFile(
		handle,
		NULL, NULL, NULL,   // Event, ApcRoutine, ApcContext
		&IoStatusBlock,
		info, info_size, FileBothDirectoryInformation,
		TRUE, &uni, FALSE);

	NtClose(handle);

	Path[index] = save_char;        // restore original path

	return status;
}


//---------------------------------------------------------------------------
// File_GetName_ExpandShortNames
//---------------------------------------------------------------------------


_FX WCHAR *File_GetName_ExpandShortNames(
    THREAD_DATA *TlsData, WCHAR *Path)
{
    NTSTATUS status;
    PFILE_BOTH_DIRECTORY_INFORMATION info = NULL;
    const ULONG info_size = 1024;
    ULONG index;

    //
    // this function scans the input path for any tilde (~) characters
    // that may hint to short names, and expands them to the long names.
    // it can only translate short names to long names outside the box.
    //

	info = Dll_AllocTemp(info_size);
    status = STATUS_SUCCESS;

    for (index = 0; Path[index] != 0; ) {

        // scan path string until a tilde (~) is found, but also keep
        // the position of the last backslash character before the tilde.

        ULONG backslash_index;
        ULONG dot_count;
        ULONG len;
        WCHAR *copy;

        for (; Path[index] != L'\0' && Path[index] != L'~'; ++index)
            if (Path[index] == L'\\')
                backslash_index = index;

        if (Path[index] == L'\0')       // end of path, no tilde
            break;

        // a tilde was found, find the first backslash following it,
        // and count how many dot (.) characters we see along the way.
        // if the tilde is the first character in the component,
        // we don't treat the component as a short name

        if (index == backslash_index + 1)       // begins with a tilde?
            dot_count = 99;                     // probably not a short name
        else
            dot_count = 0;

        for (; Path[index] != L'\0' && Path[index] != L'\\'; ++index)
            if (Path[index] == L'.')
                ++dot_count;

        // if more than one dot found, or path component is longer than
        // 12 characters (for the 8.3 format), it's not a short name

        if (dot_count > 1 || (index - backslash_index - 1) > 12)
            continue;

        // otherwise open the directory containing the short name component

		status = File_GetName_ExpandShortNames2(Path, index, backslash_index, info, info_size, NULL);

		if (!NT_SUCCESS(status) && File_Snapshot != NULL)
		{
			for (FILE_SNAPSHOT* Cur_Snapshot = File_Snapshot; Cur_Snapshot != NULL; Cur_Snapshot = Cur_Snapshot->Parent)
			{
				status = File_GetName_ExpandShortNames2(Path, index, backslash_index, info, info_size, Cur_Snapshot);
				if (NT_SUCCESS(status))
					break;
			}
		}

		/*
		// stop if we can't open the directory, but file-not-found
		// or file-not-a-directory errors may occur because the caller is
		// trying to access a directory that exists only in the copy system,
		// while we're looking at the true system.   so we shouldn't fail.

		if (!NT_SUCCESS(status)) {

			if (status == STATUS_OBJECT_NAME_NOT_FOUND ||
				status == STATUS_OBJECT_PATH_NOT_FOUND ||
				status == STATUS_NOT_A_DIRECTORY) {

				status = STATUS_SUCCESS;
			}

			break;
		}

        if (status == STATUS_NO_SUCH_FILE) {    // short name not found,
            status = STATUS_SUCCESS;            // so don't replace it
            continue;
        }

        if (! NT_SUCCESS(status))       // could not query long name?
            break;
		*/

		if (!NT_SUCCESS(status))
			continue;

        //
        // expand the path with the short name into the copy name buffer,
        // then copy it back into the true name buffer
        //

        len = (wcslen(Path) + 1) * sizeof(WCHAR) + info->FileNameLength;
        copy = Dll_GetTlsNameBuffer(TlsData, COPY_NAME_BUFFER, len);

        wmemcpy(copy, Path, backslash_index + 1);
        len = (backslash_index + 1);
        memcpy(copy + len, info->FileName, info->FileNameLength);
        len += info->FileNameLength / sizeof(WCHAR);
        wcscpy(copy + len, Path + index);

        len = (wcslen(copy) + 1) * sizeof(WCHAR);
        Path = Dll_GetTlsNameBuffer(TlsData, TRUE_NAME_BUFFER, len);
        memcpy(Path, copy, len);

        index = backslash_index + info->FileNameLength / sizeof(WCHAR) + 1;

        status = STATUS_SUCCESS;
    }

    // free memory allocated for FILE_BOTH_DIRECTORY_INFORMATION, and return

    if (info)
        Dll_Free(info);

    return Path;
}


//---------------------------------------------------------------------------
// File_GetName_ConvertLinks
//---------------------------------------------------------------------------


_FX BOOLEAN File_GetName_ConvertLinks(
    THREAD_DATA *TlsData, WCHAR **OutTruePath, BOOLEAN ConvertWow64Link)
{
    WCHAR *name;
    ULONG name_len;
    FILE_LINK *link;
    ULONG retries = 0;
    BOOLEAN converted = FALSE;

    name = *OutTruePath;
    name_len = wcslen(name);

    EnterCriticalSection(File_DrivesAndLinks_CritSec);

    link = List_Head(File_PermLinks);
    while (link) {

        const ULONG src_len = link->src_len;
        if (name_len >= src_len &&
                (name[src_len] == L'\\' || name[src_len] == L'\0') &&
                _wcsnicmp(name, link->src, src_len) == 0) {

#ifdef WOW64_FS_REDIR
            if (link == File_Wow64FileLink) {
                ULONG skip = (! ConvertWow64Link) ? 1
                           : File_GetName_SkipWow64Link(name + src_len);
                if (skip) {
                    link = List_Next(link);
                    continue;
                }
            }
#endif WOW64_FS_REDIR

            File_GetName_FixTruePrefix(
                TlsData, &name, &name_len,
                src_len, link->dst, link->dst_len);

            *OutTruePath = name;
            converted = TRUE;
            link = List_Head(File_PermLinks);

            ++retries;
            if (retries == 16)
                break;

        } else
            link = List_Next(link);
    }

    LeaveCriticalSection(File_DrivesAndLinks_CritSec);

    return converted;
}


//---------------------------------------------------------------------------
// File_GetName_FixTruePrefix
//---------------------------------------------------------------------------


_FX void File_GetName_FixTruePrefix(
    THREAD_DATA *TlsData,
    WCHAR **OutTruePath, ULONG *InOutLength,
    ULONG old_prefix_len,
    const WCHAR *new_prefix, ULONG new_prefix_len)
{
    //
    // input:    *OutTruePath points to the TruePath, which is *InOutLength
    //           characters long, and begins with some prefix that is
    //           old_prefix_len characters long
    //
    // process:  append the suffix of TruePath to the specifieid new_prefix,
    //           which is new_prefix_len chacters long
    //
    // output:   *OutTruePath and *InOutLength are adjusted.
    //

    ULONG suffix_len = *InOutLength - old_prefix_len + /* NULL */ 1;
    ULONG len = new_prefix_len + suffix_len;
    WCHAR *nm = Dll_GetTlsNameBuffer(
                    TlsData, COPY_NAME_BUFFER, len * sizeof(WCHAR));
    wmemcpy(nm, new_prefix, new_prefix_len);
    wmemcpy(nm + new_prefix_len, *OutTruePath + old_prefix_len, suffix_len);

    *OutTruePath = Dll_GetTlsNameBuffer(
                    TlsData, TRUE_NAME_BUFFER, len * sizeof(WCHAR));
    wmemcpy(*OutTruePath, nm, len);
    *InOutLength = len - /* NULL */ 1;
}


//---------------------------------------------------------------------------
// File_GetName_SkipWow64Link
//---------------------------------------------------------------------------


#ifdef WOW64_FS_REDIR
_FX ULONG File_GetName_SkipWow64Link(const WCHAR *name)
{
    typedef (*pfs)(ULONG_PTR x);
    ULONG x;

    //
    // before translating System32 to SysWow64, make sure
    // filesystem redirection was not disabled.
    //
    // note that some programs (e.g. DriverGenius) make the mistake
    // of calling Wow64RevertWow64FsRedirection(&x)
    // instead of Wow64RevertWow64FsRedirection(x)
    // and then our call to the same function sets x to some pointer
    // values instead of zero.  to counter this, we only consider
    // Wow64 redirection disabled if x is exactly 1
    //

    ((pfs)File_Wow64DisableWow64FsRedirection)((ULONG_PTR)&x);
    ((pfs)File_Wow64RevertWow64FsRedirection)(x);

    if (x != 1)
        x = 0;

    if ((! x) && *name) {

        //
        // some folders should never be redirected.
        //

        static const WCHAR *_specialcases[] = {
            L"driverstore",            // only on Windows 7
            L"catroot", L"catroot2",
            L"drivers\\etc",
            L"logfiles",
            L"spool",
            NULL
        };
        ULONG i, n;

        ++name; // past backslash

        for (i = 0; _specialcases[i]; ++i) {

            if (i == 0 && Dll_OsBuild < 7600)
                continue;

            n = wcslen(_specialcases[i]);
            if (_wcsnicmp(name, _specialcases[i], n) == 0 &&
                    (name[n] == L'\\' || name[n] == L'\0')) {

                x = 1;
                break;
            }
        }
    }

    return x;
}
#endif WOW64_FS_REDIR


//---------------------------------------------------------------------------
// File_Wow64FixProcImage
//---------------------------------------------------------------------------


#ifdef WOW64_FS_REDIR
_FX VOID File_Wow64FixProcImage(WCHAR* proc_image_path)
{
    if (!proc_image_path)
        return;

    if (File_Wow64FileLink) {

        const ULONG sys32len = File_Wow64FileLink->src_len;

        WCHAR* name = File_TranslateDosToNtPath(proc_image_path);
        ULONG length = wcslen(name);

        if (length >= sys32len
            && _wcsnicmp(name, File_Wow64FileLink->src, sys32len) == 0
            && (name[sys32len] == L'\\' || name[sys32len] == L'\0')) {

            wmemcpy(proc_image_path, File_Wow64SysNative, File_Wow64SysNativeLen);
            wmemcpy(proc_image_path + File_Wow64SysNativeLen, name + sys32len, length - sys32len + 1);

            SbieDll_TranslateNtToDosPath(proc_image_path);
        }

        Dll_Free(name);
    }

}
#endif WOW64_FS_REDIR


//---------------------------------------------------------------------------
// File_GetName_FromFileId
//---------------------------------------------------------------------------


_FX NTSTATUS File_GetName_FromFileId(
    OBJECT_ATTRIBUTES *ObjectAttributes,
    WCHAR **OutTruePath, WCHAR **OutCopyPath)
{
    NTSTATUS status;
    HANDLE FileHandle = NULL;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;

    //
    // open by FileId requires a parent directory handle
    // and an object name with an even length up to 8 bytes
    //

    if (! ObjectAttributes->ObjectName)
        return STATUS_OBJECT_NAME_INVALID;
    if (ObjectAttributes->ObjectName->Length > 8)
        return STATUS_OBJECT_NAME_INVALID;
    if (ObjectAttributes->ObjectName->Length & 1)
        return STATUS_OBJECT_NAME_INVALID;
    if (! ObjectAttributes->RootDirectory)
        return STATUS_OBJECT_PATH_SYNTAX_BAD;

    //
    // assuming the requested file exists outside the sandbox:
    //
    // if caller is trying to open by FileId using a parent directory,
    // the parent directory may be D:\sandbox\drive\C rather than the
    // real C: and this would cause a problem if both the C: and D:
    // drives have a file with the same FileId.  to workaround this,
    // we always prefer to use the real C: as parent directory
    //

    if (1) {

        BOOLEAN IsBoxedPath;
        status = SbieDll_GetHandlePath(
                    ObjectAttributes->RootDirectory, NULL, &IsBoxedPath);
        if (IsBoxedPath && (
                NT_SUCCESS(status) || (status == STATUS_BAD_INITIAL_PC))) {

            WCHAR *path = Dll_AllocTemp(8192);
            status = SbieDll_GetHandlePath(
                ObjectAttributes->RootDirectory, path, NULL);
            if (NT_SUCCESS(status)) {

                HANDLE hTrueRoot;

                InitializeObjectAttributes(&objattrs,
                    &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);
                RtlInitUnicodeString(&objname, path);

                status = __sys_NtCreateFile(
                    &hTrueRoot, SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                    &objattrs, &IoStatusBlock, NULL, 0,
                    FILE_SHARE_VALID_FLAGS,
                    FILE_OPEN,
                    FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL, 0);

                if (NT_SUCCESS(status)) {

                    objattrs.RootDirectory = hTrueRoot;
                    objattrs.ObjectName = ObjectAttributes->ObjectName;

                    status = __sys_NtCreateFile(
                        &FileHandle, SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                        &objattrs, &IoStatusBlock, NULL, 0,
                        FILE_SHARE_VALID_FLAGS,
                        FILE_OPEN,
                        FILE_OPEN_BY_FILE_ID | FILE_SYNCHRONOUS_IO_NONALERT,
                        NULL, 0);

                    if (! NT_SUCCESS(status))
                        FileHandle = NULL;

                    NtClose(hTrueRoot);
                }
            }

            Dll_Free(path);
        }
    }

    //
    // if we don't have a handle yet, two options are possible:
    //
    // 1.  the specified parent directory was not inside the sandbox,
    //     i.e. caller specified real C: and not D:\sandbox\drive\C
    //
    // 2.  the file exists in the sandbox but the FileId specified by
    //     the caller was scrambled/invalid by NtQueryDirectoryFile
    //     or NtQueryInformationFile
    //
    // for both cases, try to open the file using the parent directory
    // specified by the caller
    //

    if (! FileHandle) {

        status = __sys_NtCreateFile(
            &FileHandle, SYNCHRONIZE | FILE_READ_ATTRIBUTES,
            ObjectAttributes, &IoStatusBlock, NULL, 0,
            FILE_SHARE_VALID_FLAGS, FILE_OPEN,
            FILE_OPEN_BY_FILE_ID | FILE_SYNCHRONOUS_IO_NONALERT,
            NULL, 0);

        if (! NT_SUCCESS(status)) {

            //
            // if the file exists in the sandbox, and its FileId was
            // queried using NtQueryDirectoryFile/NtQueryInformationFile
            // then we returned a scrambled FileId, so un-scramble it
            //

            LARGE_INTEGER FileId;
            memzero(&FileId, sizeof(FileId));
            memcpy(&FileId, ObjectAttributes->ObjectName->Buffer,
                            ObjectAttributes->ObjectName->Length);
            FileId.LowPart  ^= 0xFFFFFFFF;
            FileId.HighPart ^= 0xFFFFFFFF;

            objname.Length = sizeof(FileId);
            objname.MaximumLength = objname.Length;
            objname.Buffer = (WCHAR *)&FileId;

            InitializeObjectAttributes(&objattrs,
                &objname, OBJ_CASE_INSENSITIVE,
                ObjectAttributes->RootDirectory, NULL);

            status = __sys_NtCreateFile(
                &FileHandle, SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                &objattrs, &IoStatusBlock, NULL, 0,
                FILE_SHARE_VALID_FLAGS, FILE_OPEN,
                FILE_OPEN_BY_FILE_ID | FILE_SYNCHRONOUS_IO_NONALERT,
                NULL, 0);

            if (! NT_SUCCESS(status))
                FileHandle = NULL;
        }
    }

    //
    // if we got a handle for the FileId, query the real path
    //

    if (FileHandle) {

        status =
            File_GetName(FileHandle, NULL, OutTruePath, OutCopyPath, NULL);

        NtClose(FileHandle);

    } else
        status = STATUS_INVALID_PARAMETER;

    return status;
}


//---------------------------------------------------------------------------
// File_MatchPath
//---------------------------------------------------------------------------


_FX ULONG File_MatchPath(const WCHAR *path, ULONG *FileFlags)
{
    return File_MatchPath2(path, FileFlags, TRUE, TRUE);
}


//---------------------------------------------------------------------------
// File_MatchPath2
//---------------------------------------------------------------------------


_FX ULONG File_MatchPath2(const WCHAR *path, ULONG *FileFlags, BOOLEAN bCheckObjectExists, BOOLEAN bMonitorLog)
{
    WCHAR *temp_path;
    const WCHAR *ptr;
    ULONG PrefixLen;
    ULONG mp_flags;

    //
    // if the path contains a colon that indicates an NTFS
    // alternate data stream, create a temporary path without the colon
    //

    temp_path = NULL;

    ptr = wcsrchr(path, L'\\');
    if (ptr) {
        ptr = wcschr(ptr, L':');
        if (ptr) {

            ULONG len = (wcslen(path) + 1) * sizeof(WCHAR);
            temp_path = Dll_AllocTemp(len);
            memcpy(temp_path, path, len);

            temp_path[ptr - path] = L'\0';
            path = (const WCHAR *)temp_path;
        }
    }

    //
    // give read-only access to Sandboxie home folder,
    // disregarding any settings that might affect it
    //

    if (Dll_HomeNtPathLen) {
        ULONG path_len = wcslen(path);
        if (path_len >= Dll_HomeNtPathLen
                && (path[Dll_HomeNtPathLen] == L'\\' ||
                    path[Dll_HomeNtPathLen] == L'\0')
                && 0 == Dll_NlsStrCmp(
                            path, Dll_HomeNtPath, Dll_HomeNtPathLen)) {

            mp_flags = PATH_OPEN_FLAG;
            goto finish;
        }
    }

    //
    // if the File_GetName already ran File_MatchPath on a reparsed
    // TruePath then we're done
    //

    if (FileFlags) {

        if ((*FileFlags) & FGN_REPARSED_OPEN_PATH) {
            mp_flags = PATH_OPEN_FLAG;
            goto finish;
        }
        if ((*FileFlags) & FGN_REPARSED_CLOSED_PATH) {
            mp_flags = PATH_CLOSED_FLAG;
            goto finish;
        }
        if ((*FileFlags) & FGN_REPARSED_WRITE_PATH) {
            mp_flags = PATH_WRITE_FLAG;
            goto finish;
        }
    }

    //
    // check for network paths
    //

    if (_wcsnicmp(path, File_Redirector, File_RedirectorLen) == 0)
        PrefixLen = File_RedirectorLen;
    else if (_wcsnicmp(path, File_DfsClientRedir, File_DfsClientRedirLen) == 0)
        PrefixLen = File_DfsClientRedirLen;
    else if (_wcsnicmp(path, File_HgfsRedir, File_HgfsRedirLen) == 0)
        PrefixLen = File_HgfsRedirLen;
    else if (_wcsnicmp(path, File_MupRedir, File_MupRedirLen) == 0)
        PrefixLen = File_MupRedirLen;
    else
        PrefixLen = 0;

    //
    // if we have a path that looks like
    // \Device\LanmanRedirector\;Q:000000000000b09f\server\share\f1.txt
    // \Device\Mup\;LanmanRedirector\;Q:000000000000b09f\server\share\f1.txt
    // then translate to
    // \Device\Mup\server\share\f1.txt
    // and test again.  We do this because the SbieDrv records paths
    // in the \Device\Mup format.  See SbieDrv::File_TranslateShares.
    //

    if (PrefixLen) {

        ptr = path + PrefixLen;
        if (*ptr == L';')
            ptr = wcschr(ptr, L'\\');
        else
            --ptr;
        if (ptr && ptr[0] && ptr[1]) {

            ULONG len1 = wcslen(ptr + 1);
            ULONG len2 = (File_MupLen + len1 + 8) * sizeof(WCHAR);
            WCHAR* path2 = Dll_AllocTemp(len2);
            wmemcpy(path2, File_Mup, File_MupLen);
            wmemcpy(path2 + File_MupLen, ptr + 1, len1 + 1);

            mp_flags = SbieDll_MatchPath2(L'f', path2, bCheckObjectExists, bMonitorLog);

            Dll_Free(path2);
            goto finish;
        }
    }

    //
    // match path
    //

    mp_flags = SbieDll_MatchPath2((FileFlags ? L'f' : L'p'), path, FALSE, TRUE);

    if (mp_flags)
        goto finish;

    //
    // if path references a mount point, we see it as the mount location
    // \Device\HarddiskVolume1\MOUNT but the driver sees it as the target
    // location \Device\HarddiskVolume2, so check for this case
    //

    if (FileFlags) {

        WCHAR *path2 = File_FixPermLinksForMatchPath(path);
        if (path2) {
            mp_flags = SbieDll_MatchPath2(L'f', path2, bCheckObjectExists, bMonitorLog);
            Dll_Free(path2);

            if (PATH_IS_WRITE(mp_flags)) {
                (*FileFlags) |= FGN_REPARSED_WRITE_PATH;
                goto finish;
            }
            if (PATH_IS_CLOSED(mp_flags)) {
                (*FileFlags) |= FGN_REPARSED_CLOSED_PATH;
                goto finish;
            }
            if (PATH_IS_OPEN(mp_flags)) {
                (*FileFlags) |= FGN_REPARSED_OPEN_PATH;
                goto finish;
            }
        }
    }

    //
    // finish
    //

finish:

    if (temp_path)
        Dll_Free(temp_path);

    return mp_flags;
}


//---------------------------------------------------------------------------
// File_NtOpenFile
//---------------------------------------------------------------------------


_FX NTSTATUS File_NtOpenFile(
    HANDLE *FileHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    IO_STATUS_BLOCK *IoStatusBlock,
    ULONG ShareAccess,
    ULONG OpenOptions)
{
    NTSTATUS status;

#ifdef _M_ARM64EC

    //
	// TODO: Fix-Me:
    // In ARM64EC xtajit64.dll calls NtOpenFile and when this happens __chkstk_arm64ec
	// crashes causing a stack overflow. To avoid this we call NtOpenFile directly.
    //

    extern UINT_PTR Dll_xtajit64;
    ULONG_PTR pRetAddr = (ULONG_PTR)_ReturnAddress();

    if (pRetAddr > Dll_xtajit64 && pRetAddr < Dll_xtajit64 + 0x180000) {

        //SbieApi_Log(2301, L"NtOpenFile bypass on ARM64EC for %S", 
        // ObjectAttributes && ObjectAttributes->ObjectName && ObjectAttributes->ObjectName->Buffer ? ObjectAttributes->ObjectName->Buffer : L"[UNNAMED]");

        status = __sys_NtOpenFile(
            FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock,
            ShareAccess, OpenOptions);
    } else
#endif

    status = File_NtCreateFileImpl(
        FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock,
        NULL, 0, ShareAccess, FILE_OPEN, OpenOptions, NULL, 0);

    status = StopTailCallOptimization(status);

    return status;
}


//---------------------------------------------------------------------------
// File_NtCreateFile
//---------------------------------------------------------------------------


_FX NTSTATUS File_NtCreateFile(
    HANDLE *FileHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    IO_STATUS_BLOCK *IoStatusBlock,
    LARGE_INTEGER *AllocationSize,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    void *EaBuffer,
    ULONG EaLength)
{
    NTSTATUS status = File_NtCreateFileImpl(
        FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock,
        AllocationSize, FileAttributes, ShareAccess, CreateDisposition,
        CreateOptions, EaBuffer, EaLength);

    status = StopTailCallOptimization(status);

    return status;
}


//---------------------------------------------------------------------------
// File_DuplicateSecurityDescriptor
//---------------------------------------------------------------------------


PSECURITY_DESCRIPTOR File_DuplicateSecurityDescriptor(PSECURITY_DESCRIPTOR pOriginalSD)
{
    if (pOriginalSD == NULL || !RtlValidSecurityDescriptor(pOriginalSD))
        return NULL;

    SECURITY_DESCRIPTOR_CONTROL control;
    ULONG revision;
    if (!NT_SUCCESS(RtlGetControlSecurityDescriptor(pOriginalSD, &control, &revision)))
        return NULL;

    BOOL isSelfRelative = (control & SE_SELF_RELATIVE) != 0;

    if (!isSelfRelative) 
    {
        ULONG sdSize = 0;
        NTSTATUS status = RtlMakeSelfRelativeSD(pOriginalSD, NULL, &sdSize);
        if (status != STATUS_BUFFER_TOO_SMALL)
            return NULL;

        PSECURITY_DESCRIPTOR pSelfRelativeSD = (PSECURITY_DESCRIPTOR)Dll_AllocTemp(sdSize);
        if (pSelfRelativeSD == NULL)
            return NULL;

        status = RtlMakeSelfRelativeSD(pOriginalSD, pSelfRelativeSD, &sdSize);
        if (!NT_SUCCESS(status)) {
            LocalFree(pSelfRelativeSD);
            return NULL;
        }

        return pSelfRelativeSD; 
    }
    else
    {
        ULONG sdSize = RtlLengthSecurityDescriptor(pOriginalSD);

        PSECURITY_DESCRIPTOR pNewSD = (PSECURITY_DESCRIPTOR)Dll_AllocTemp(sdSize);
        if (pNewSD == NULL)
            return NULL;

        memcpy(pNewSD, pOriginalSD, sdSize);

        return pNewSD;
    }
}


//---------------------------------------------------------------------------
// File_AddCurrentUserToSD
//---------------------------------------------------------------------------


NTSTATUS File_AddCurrentUserToSD(PSECURITY_DESCRIPTOR *pSD)
{
    PACL pOldDACL = NULL;
    PACL pNewDACL = NULL;
    PSECURITY_DESCRIPTOR pAbsoluteSD = NULL;
    ULONG daclLength = 0;
    NTSTATUS status;
    BOOLEAN daclPresent = FALSE, daclDefaulted = FALSE;
    ULONG aceCount = 0;
    ULONG absoluteSDSize = 0, daclSize = 0, saclSize = 0, ownerSize = 0, groupSize = 0;
    PSID ownerSid = NULL, groupSid = NULL;
    PACL sacl = NULL;

    if (!Dll_SidString)
        return STATUS_UNSUCCESSFUL;
    PSID pSid = Dll_SidStringToSid(Dll_SidString);
    if (!pSid)
        return STATUS_UNSUCCESSFUL;

    status = RtlSelfRelativeToAbsoluteSD(*pSD, NULL, &absoluteSDSize, NULL, &daclSize, NULL, &saclSize, NULL, &ownerSize, NULL, &groupSize);
    if (status != STATUS_BUFFER_TOO_SMALL)
        return status;

    pAbsoluteSD = (PSECURITY_DESCRIPTOR)Dll_AllocTemp(absoluteSDSize);
    pOldDACL = (PACL)Dll_AllocTemp(daclSize);
    sacl = (PACL)Dll_AllocTemp(saclSize);
    ownerSid = (PSID)Dll_AllocTemp(ownerSize);
    groupSid = (PSID)Dll_AllocTemp(groupSize);

    if (!pAbsoluteSD || !pOldDACL || !sacl || !ownerSid || !groupSid) {
        status = STATUS_NO_MEMORY;
        goto cleanup;
    }

    status = RtlSelfRelativeToAbsoluteSD(*pSD, pAbsoluteSD, &absoluteSDSize, pOldDACL, &daclSize, sacl, &saclSize, ownerSid, &ownerSize, groupSid, &groupSize);
    if (!NT_SUCCESS(status))
        goto cleanup;

    status = RtlGetDaclSecurityDescriptor(pAbsoluteSD, &daclPresent, &pOldDACL, &daclDefaulted);
    if (!NT_SUCCESS(status) || !daclPresent || !pOldDACL)
        goto cleanup;

    daclLength = pOldDACL->AclSize + sizeof(ACCESS_ALLOWED_ACE) + RtlLengthSid(pSid) - sizeof(DWORD);

    pNewDACL = (PACL)Dll_AllocTemp(daclLength);
    if (!pNewDACL) {
        status = STATUS_NO_MEMORY;
        goto cleanup;
    }

    status = RtlCreateAcl(pNewDACL, daclLength, pOldDACL->AclRevision);
    if (!NT_SUCCESS(status))
        goto cleanup;

    for (aceCount = 0; aceCount < pOldDACL->AceCount; aceCount++) {
        PVOID pAce;
        if (NT_SUCCESS(RtlGetAce(pOldDACL, aceCount, &pAce))) {
            status = RtlAddAce(pNewDACL, pOldDACL->AclRevision, -1, pAce, ((PACE_HEADER)pAce)->AceSize);
            if (!NT_SUCCESS(status))
                goto cleanup;
        }
    }

    status = RtlAddAccessAllowedAceEx(pNewDACL, pNewDACL->AclRevision, CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE | INHERITED_ACE, GENERIC_ALL, pSid );
    if (!NT_SUCCESS(status))
        goto cleanup;

    status = RtlSetDaclSecurityDescriptor(pAbsoluteSD, TRUE, pNewDACL, FALSE);
    if (!NT_SUCCESS(status))
        goto cleanup;

    ULONG selfRelativeSDSize = 0;
    status = RtlMakeSelfRelativeSD(pAbsoluteSD, NULL, &selfRelativeSDSize);
    if (status != STATUS_BUFFER_TOO_SMALL)
        goto cleanup;

    Dll_Free(*pSD);
    *pSD = (PSECURITY_DESCRIPTOR)Dll_AllocTemp(selfRelativeSDSize);
    if (!*pSD) {
        status = STATUS_NO_MEMORY;
        goto cleanup;
    }

    status = RtlMakeSelfRelativeSD(pAbsoluteSD, *pSD, &selfRelativeSDSize);
    if (!NT_SUCCESS(status))
        goto cleanup;

cleanup:
    if (pAbsoluteSD) Dll_Free(pAbsoluteSD);
    if (pNewDACL) Dll_Free(pNewDACL);
    if (ownerSid) Dll_Free(ownerSid);
    if (groupSid) Dll_Free(groupSid);
    if (sacl) Dll_Free(sacl);

    Dll_Free(pSid);

    return status;
}


//---------------------------------------------------------------------------
// File_NtCreateFileImpl
//---------------------------------------------------------------------------


#ifdef WITH_DEBUG_
static P_NtCreateFile               __sys_NtCreateFile_ = NULL;

_FX NTSTATUS File_MyCreateFile(
    HANDLE* FileHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES* ObjectAttributes,
    IO_STATUS_BLOCK* IoStatusBlock,
    LARGE_INTEGER* AllocationSize,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    void* EaBuffer,
    ULONG EaLength)
{
    NTSTATUS status = __sys_NtCreateFile_(
        FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock,
        AllocationSize, FileAttributes, ShareAccess, CreateDisposition,
        CreateOptions, EaBuffer, EaLength);

    if (ObjectAttributes && ObjectAttributes->ObjectName && ObjectAttributes->ObjectName->Buffer
        && wcsstr(ObjectAttributes->ObjectName->Buffer, L"ext-ms-win-branding-winbrand-l1-1-0.dll") != 0)
    {
        return status;
    }

    //if (NT_SUCCESS(status)) DbgPrint("%p: %p\r\n", _ReturnAddress(), *FileHandle);

    status = StopTailCallOptimization(status);

    return status;
}
#endif

_FX NTSTATUS File_NtCreateFileImpl(
    HANDLE *FileHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    IO_STATUS_BLOCK *IoStatusBlock,
    LARGE_INTEGER *AllocationSize,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    void *EaBuffer,
    ULONG EaLength)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    WCHAR *TruePathColon;
    WCHAR *CopyPathColon;
    UNICODE_STRING objname;
    OBJECT_ATTRIBUTES objattrs;
    ULONG FileFlags, FileType, mp_flags;
    BOOLEAN HaveCopyParent, HaveCopyFile, HaveTrueParent;
    BOOLEAN DeleteOnClose, DeleteChildren;
    BOOLEAN IsEmptyCopyFile;
    BOOLEAN AlreadyReparsed;
    UCHAR HaveTrueFile;
	BOOLEAN HaveSnapshotParent;
    ULONG TruePathFlags;
    WCHAR* OriginalPath;
    BOOLEAN TrueOpened;
    //char *pPtr = NULL;
    BOOLEAN SkipOriginalTry;
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;

    //if (wcsstr(Dll_ImageName, L"chrome.exe") != 0) {
    //  *pPtr = 34;
    //  //while (! IsDebuggerPresent()) { OutputDebugString(L"BREAK\n"); Sleep(500); }
    //  //   __debugbreak();
    //}

#ifdef WITH_DEBUG_
    if (__sys_NtCreateFile_ == NULL)
    {
        __sys_NtCreateFile_ = __sys_NtCreateFile;
        __sys_NtCreateFile = File_MyCreateFile;
    }
#endif

    /*if (ObjectAttributes && ObjectAttributes->ObjectName && ObjectAttributes->ObjectName->Buffer
        && _wcsicmp(ObjectAttributes->ObjectName->Buffer, L"\\??\\PhysicalDrive0") == 0)
    {
        return __sys_NtCreateFile(
            FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock,
            AllocationSize, FileAttributes, ShareAccess, CreateDisposition,
            CreateOptions, EaBuffer, EaLength);
    }*/

    /*if (ObjectAttributes && ObjectAttributes->ObjectName && ObjectAttributes->ObjectName->Buffer
        && wcsstr(ObjectAttributes->ObjectName->Buffer, L"socket_") != NULL ) {
        while (! IsDebuggerPresent()) { OutputDebugString(L"BREAK\n"); Sleep(500); }
           __debugbreak();
    }*/

    //
    // if this is a recursive invocation of NtCreateFile,
    // then pass it as-is down the chain
    //

    if (TlsData->file_NtCreateFile_lock) {

        return __sys_NtCreateFile(
            FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock,
            AllocationSize, FileAttributes, ShareAccess, CreateDisposition,
            CreateOptions, EaBuffer, EaLength);
    }

    //
    // not a recursive invocation, handle the call here
    //

    TlsData->file_NtCreateFile_lock = TRUE;

    Dll_PushTlsNameBuffer(TlsData);

    AlreadyReparsed = FALSE;

    if (Dll_OsBuild >= 8400 && Dll_ImageType == DLL_IMAGE_TRUSTED_INSTALLER)
        DesiredAccess &= ~ACCESS_SYSTEM_SECURITY;   // for TiWorker.exe (W8)

    // MSIServer without system
    extern BOOLEAN Scm_MsiServer_Systemless;
    if ((DesiredAccess & ACCESS_SYSTEM_SECURITY) != 0 && Dll_ImageType == DLL_IMAGE_MSI_INSTALLER && Scm_MsiServer_Systemless
        && ObjectAttributes && ObjectAttributes->ObjectName && ObjectAttributes->ObjectName->Buffer
        && _wcsicmp(ObjectAttributes->ObjectName->Buffer + (ObjectAttributes->ObjectName->Length / sizeof(WCHAR)) - 4, L".msi") == 0
        ){

        //
        // MSIServer when accessing \??\C:\WINDOWS\Installer\???????.msi files will get a PRIVILEGE_NOT_HELD error when requesting ACCESS_SYSTEM_SECURITY
        // However, if we broadly clear this flag we will get Warning 1946 Property 'System.AppUserModel.ID' could not be set on *.lnk files
        //

        DesiredAccess &= ~ACCESS_SYSTEM_SECURITY;
    }

    OriginalPath = NULL;
    TrueOpened = FALSE;
    SkipOriginalTry = FALSE;

    __try {

    IoStatusBlock->Information = FILE_DOES_NOT_EXIST;
    IoStatusBlock->Status = 0;

    if (Secure_CopyACLs) {

        pSecurityDescriptor = File_DuplicateSecurityDescriptor(ObjectAttributes->SecurityDescriptor);
        if (pSecurityDescriptor)
            File_AddCurrentUserToSD(&pSecurityDescriptor);

        InitializeObjectAttributes(&objattrs,
            &objname, OBJECT_ATTRIBUTES_ATTRIBUTES, NULL, pSecurityDescriptor);
    }
    else {
        InitializeObjectAttributes(&objattrs,
            &objname, OBJECT_ATTRIBUTES_ATTRIBUTES, NULL, Secure_NormalSD);
        /*objattrs.SecurityQualityOfService =
            ObjectAttributes->SecurityQualityOfService;*/
    }

    //
    // remove creation options that can't be honored because the
    // SbieDrv has removed privileges
    //

    if (!Dll_CompartmentMode)
        CreateOptions &= ~FILE_OPEN_FOR_BACKUP_INTENT;

    //
    // get the full paths for the true and copy files.
    //

    if (CreateOptions & FILE_OPEN_BY_FILE_ID) {

        status = File_GetName_FromFileId(
                            ObjectAttributes, &TruePath, &CopyPath);
        FileFlags = 0;

        CreateOptions &= ~FILE_OPEN_BY_FILE_ID;

    } else {

        status = File_GetName(
            ObjectAttributes->RootDirectory, ObjectAttributes->ObjectName,
            &TruePath, &CopyPath, &FileFlags);

        //
        // this is some sort of device access
        //

        if (status == STATUS_OBJECT_PATH_SYNTAX_BAD) {

            //
            // the driver usually blocks this anyways so try only in app mode
            //

            if (Dll_CompartmentMode){

                SbieApi_MonitorPut2(MONITOR_PIPE, TruePath, FALSE);

                Dll_PopTlsNameBuffer(TlsData);

                TlsData->file_NtCreateFile_lock = FALSE;

                if(pSecurityDescriptor)
                    Dll_Free(pSecurityDescriptor);

                return __sys_NtCreateFile(
                    FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock,
                    AllocationSize, FileAttributes, ShareAccess, CreateDisposition,
                    CreateOptions, EaBuffer, EaLength);

            } else {
                SbieApi_MonitorPut2(MONITOR_PIPE | MONITOR_DENY, TruePath, FALSE);
            }
        }
    }

    if (Dll_ApiTrace || Dll_FileTrace) {
        WCHAR trace_str[2048];
        ULONG len = Sbie_snwprintf(trace_str, 2048, L"File_NtCreateFileImpl %s DesiredAccess=0x%08X CreateDisposition=0x%08X CreateOptions=0x%08X", TruePath, DesiredAccess, CreateDisposition, CreateOptions);
        SbieApi_MonitorPut2Ex(MONITOR_APICALL | MONITOR_TRACE, len, trace_str, FALSE, FALSE);
    }

    SkipOriginalTry = (status == STATUS_BAD_INITIAL_PC);

    //if ( (wcsstr(TruePath, L"Harddisk0\\DR0") != 0) || wcsstr(TruePath, L"HarddiskVolume3") != 0) {
    //  while (! IsDebuggerPresent()) { OutputDebugString(L"BREAK\n"); Sleep(500); }
    //     __debugbreak();
    //}

ReparseLoop:

    if (! NT_SUCCESS(status)) {

        //
        // we may get STATUS_BAD_INITIAL_PC if the caller is trying
        // to open something that isn't a file object.  this could be
        // the volume device rather than any file in the device, or a
        // named pipe or mail slot devices, or any other device that
        // can't be translated to a drive letter
        //

        if (status == STATUS_BAD_INITIAL_PC) {

            WCHAR *pipe_server = NULL;
            ULONG pipe_type = File_IsNamedPipe(TruePath, &pipe_server);

            if (pipe_type) {

                status = File_NtCreateFilePipe(
                    FileHandle, DesiredAccess, &objattrs,
                    ObjectAttributes->SecurityDescriptor,
                    ObjectAttributes->SecurityQualityOfService,
                    IoStatusBlock, ShareAccess,
                    CreateDisposition, CreateOptions,
                    TruePath, pipe_type, pipe_server);

            } else {

                //
                // the path is neither a disk nor a pipe so we generally
                // allow access unless it explicitly matches a closed path
                //

                mp_flags = SbieDll_MatchPath(L'p', TruePath);

                if (PATH_IS_CLOSED(mp_flags)) {
                    status = STATUS_ACCESS_DENIED;
                    __leave;
                }

                //
                // initially try with the caller-specified desired access,
                // which will be allowed if our driver is ignoring this
                // particular device
                //

                status = __sys_NtCreateFile(
                    FileHandle, DesiredAccess, ObjectAttributes,
                    IoStatusBlock, AllocationSize, FileAttributes,
                    ShareAccess, CreateDisposition, CreateOptions,
                    EaBuffer, EaLength);

                if (status == STATUS_ACCESS_DENIED) {

                    CreateDisposition = FILE_OPEN;
                    CreateOptions &= ~FILE_DELETE_ON_CLOSE;
                    DesiredAccess &= ~FILE_DENIED_ACCESS;

                    //
                    // If this is an access on a raw disk device, adapt the requested permissions to what the drivers permits
                    //

                    if (ObjectAttributes->ObjectName && &ObjectAttributes->ObjectName->Buffer != NULL && ObjectAttributes->ObjectName->Length > (4 * sizeof(WCHAR))
                        && wcsncmp(ObjectAttributes->ObjectName->Buffer, L"\\??\\", 4) == 0
                        && (DesiredAccess & ~(SYNCHRONIZE | READ_CONTROL | FILE_READ_EA | FILE_READ_ATTRIBUTES)) != 0)
                    {
                        if (!SbieApi_QueryConfBool(NULL, L"AllowRawDiskRead", FALSE))
                        if ((ObjectAttributes->ObjectName->Length == (6 * sizeof(WCHAR)) && ObjectAttributes->ObjectName->Buffer[5] == L':') // \??\C:
                            || wcsncmp(&ObjectAttributes->ObjectName->Buffer[4], L"PhysicalDrive", 13) == 0 // \??\PhysicalDrive1
                            || wcsncmp(&ObjectAttributes->ObjectName->Buffer[4], L"Volume", 6) == 0) // \??\Volume{2b985816-4b6f-11ea-bd33-48a4725d5bbe}
                        {
                            DesiredAccess &= (SYNCHRONIZE | READ_CONTROL | FILE_READ_EA | FILE_READ_ATTRIBUTES);
                        }
                    }

                    status = __sys_NtCreateFile(
                        FileHandle, DesiredAccess, ObjectAttributes,
                        IoStatusBlock, AllocationSize, FileAttributes,
                        ShareAccess, CreateDisposition, CreateOptions,
                        EaBuffer, EaLength);
                }
            }
        }

        __leave;
    }

    //
    // allow SXS module to intercept open requests during CreateProcess
    //

    if (TlsData->proc_create_process &&
                                Sxs_FileCallback(TruePath, FileHandle))
        __leave;

    //
    // check if this is an open or closed path
    //

    mp_flags = File_MatchPath(TruePath, &FileFlags);

    if (PATH_IS_CLOSED(mp_flags)) {
        status = STATUS_ACCESS_DENIED;
        __leave;
    }

    if (PATH_IS_OPEN(mp_flags)) {

        WCHAR *ReparsedPath = NULL;

        RtlInitUnicodeString(&objname, TruePath);
        objattrs.SecurityDescriptor = ObjectAttributes->SecurityDescriptor;
        /*
        objattrs.SecurityQualityOfService =
            ObjectAttributes->SecurityQualityOfService;
        */

        //status = __sys_NtCreateFile(
        status = File_NtCreateTrueFile(
            FileHandle, DesiredAccess, &objattrs,
            IoStatusBlock, AllocationSize, FileAttributes,
            ShareAccess, CreateDisposition, CreateOptions,
            EaBuffer, EaLength);

        if ((status == STATUS_ACCESS_DENIED || status == STATUS_OBJECT_NAME_NOT_FOUND) &&
                (FileFlags & FGN_REPARSED_OPEN_PATH)) {

            //
            // the request may fail if the path contains a reparse point,
            // in this case try again with the absolute path
            //

            ReparsedPath = File_TranslateTempLinks(TruePath, FALSE);
            if (ReparsedPath) {
                WCHAR *ReparsedPath2 =
                    File_FixPermLinksForMatchPath(ReparsedPath);
                if (ReparsedPath2) {
                    Dll_Free(ReparsedPath);
                    ReparsedPath = ReparsedPath2;
                }
            } else
                ReparsedPath = File_FixPermLinksForMatchPath(TruePath);

            if (ReparsedPath) {
                RtlInitUnicodeString(&objname, ReparsedPath);

                //status = __sys_NtCreateFile(
                status = File_NtCreateTrueFile(
                    FileHandle, DesiredAccess, &objattrs,
                    IoStatusBlock, AllocationSize, FileAttributes,
                    ShareAccess, CreateDisposition, CreateOptions,
                    EaBuffer, EaLength);
            }
        }

        if (status == STATUS_ACCESS_DENIED &&
                DesiredAccess == MAXIMUM_ALLOWED) {

            //
            // if we can't get maximum access, try read-only access
            //

            //status = __sys_NtCreateFile(
            status = File_NtCreateTrueFile(
                FileHandle, FILE_GENERIC_READ, &objattrs,
                IoStatusBlock, AllocationSize, FileAttributes,
                ShareAccess, CreateDisposition, CreateOptions,
                EaBuffer, EaLength);
        }

        if (ReparsedPath)
            Dll_Free(ReparsedPath);

        if (NT_SUCCESS(status)) TrueOpened = TRUE;

        //
        // if we got STATUS_OBJECT_PATH_NOT_FOUND on an open path, meaning
        // that parent directories are missing outside the sandbox, then
        // treat the path as a normal path (not open)
        //

        // (By Curt) This code makes no sense to me at all. If a path is open, then we should be done here.
        // An Sbie user pointed out that currently folders in an open path are being created in the sandbox.

        //if (status != STATUS_OBJECT_PATH_NOT_FOUND)
            __leave;

        //else
            //mp_flags &= ~PATH_OPEN_FLAG;
    }

    //
    // if Microsoft Outlook 2010 is writing an OICE_ file used to communicate
    // with an embedded previewer that is running with a Restricted Token then
    // use the Everyone security descriptor
    //

    // $Workaround$ - 3rd party fix
    if (Dll_ImageType == DLL_IMAGE_OFFICE_OUTLOOK &&
            wcsstr(TruePath, L"\\OICE_")) {

        objattrs.SecurityDescriptor = Secure_EveryoneSD;
    }

    //
    // otherwise we have to do the work, so first check parameters.
    //

    status = File_CheckCreateParameters(
        DesiredAccess, CreateDisposition, CreateOptions, -1);
    if (! NT_SUCCESS(status))
        __leave;

    //
    // if TruePath and CopyPath contain colons that indicate an NTFS
    // alternate data stream, we remove these for now
    //

    TruePathColon = wcsrchr(TruePath, L'\\');
    if (TruePathColon) {
        TruePathColon = wcschr(TruePathColon, L':');
        if (TruePathColon)
            *TruePathColon = L'\0';
    }

    CopyPathColon = wcsrchr(CopyPath, L'\\');
    if (CopyPathColon) {
        CopyPathColon = wcschr(CopyPathColon, L':');
        if (CopyPathColon)
            *CopyPathColon = L'\0';
    }

    //
    // abort early if the parent of CopyPath exists but marked deleted
    //

    if (!File_Delete_v2)
    if (File_CheckDeletedParent(CopyPath)) {
        status = STATUS_OBJECT_PATH_NOT_FOUND;
        __leave;
    }

    //
    // if the caller is trying to open the image file for write access,
    // then deny the request.  note that at this point we don't make
    // any distinction between true and copy paths
    //

    if (Ldr_ImageTruePath) {

        const ULONG DesiredAccess2 = (DesiredAccess & FILE_DENIED_ACCESS)
                                   & ~(FILE_WRITE_ATTRIBUTES | DELETE);
        if (DesiredAccess2 || (CreateDisposition != FILE_OPEN &&
                               CreateDisposition != FILE_OPEN_IF)) {

            if (_wcsicmp(TruePath, Ldr_ImageTruePath) == 0) {

                status = STATUS_SHARING_VIOLATION;
                __leave;
            }
        }
    }

    //
    // get the type of the file
    //

    IsEmptyCopyFile = FALSE;

    HaveTrueFile = '?';

    RtlInitUnicodeString(&objname, CopyPath);
    status = File_GetFileType(&objattrs, FALSE, &FileType, &IsEmptyCopyFile);


    HaveSnapshotParent = FALSE;

    //
    // Check true path relocation
    //

    WCHAR* OldTruePath = File_ResolveTruePath(TruePath, CopyPath, &TruePathFlags);
    if (OldTruePath) {
        OriginalPath = TruePath;
        TruePath = OldTruePath;
    }


    if (NT_SUCCESS(status)) {

        ULONG TrueFileType;

        //
        // we got the file type of an existing CopyPath file.
        // note that a CopyPath that is marked deleted, is still
        // considered to exist
        //

        HaveCopyParent = TRUE;
        HaveCopyFile = TRUE;

        if (CreateOptions & FILE_DELETE_ON_CLOSE) {

            // $Workaround$ - 3rd party fix
            if (Dll_DigitalGuardian && (PATH_IS_WRITE(mp_flags) || PATH_IS_CLOSED(mp_flags)))
            {
                HaveTrueFile = 'N';
                status = STATUS_SUCCESS;
            }
            else
            {
                //
                // special case:  for FILE_DELETE_ON_CLOSE handling, we need
                // to know if a TrueFile exists, to decide if we are going
                // to really delete CopyPath, or just mark it deleted
                //

                RtlInitUnicodeString(&objname, TruePath);
                status = File_GetFileType(&objattrs, FALSE, &TrueFileType, NULL);
                if (NT_SUCCESS(status))
                    HaveTrueFile = 'Y';
                else {
                    HaveTrueFile = 'N';
                    status = STATUS_SUCCESS;
                }
            }
        }

    }
    else if (status == STATUS_OBJECT_NAME_NOT_FOUND || status == STATUS_OBJECT_PATH_NOT_FOUND) {

        //
        // the CopyPath file does not exist, but its parent path may exist
        //

        HaveCopyFile = FALSE;

        if (status == STATUS_OBJECT_NAME_NOT_FOUND)
            HaveCopyParent = TRUE;
        else
            HaveCopyParent = FALSE;

        //
        // check if the parent folder exists in a snapshot
        //

        if (! HaveCopyParent) {

            WCHAR* ptr1 = wcsrchr(CopyPath, L'\\');
            *ptr1 = L'\0';
            //WCHAR* ptr2 = wcsrchr(TruePath, L'\\');
            //*ptr2 = L'\0';

            Dll_PushTlsNameBuffer(TlsData);

            WCHAR* TmplName = File_FindSnapshotPath(CopyPath);
            if (TmplName != NULL) 
                HaveSnapshotParent = TRUE;

            Dll_PopTlsNameBuffer(TlsData);

            //*ptr2 = L'\\';
            *ptr1 = L'\\';
        }

        //
        // we need to check if the true path exists
        //

        RtlInitUnicodeString(&objname, TruePath);

        if (PATH_IS_WRITE(mp_flags)) {

            BOOLEAN use_rule_specificity = (Dll_ProcessFlags & SBIE_FLAG_RULE_SPECIFICITY) != 0;

            if (use_rule_specificity && SbieDll_HasReadableSubPath(L'f', OriginalPath ? OriginalPath : TruePath)){

                //
                // When using Rule specificity we need to create some dummy directories 
                //

                File_CreateBoxedPath(OriginalPath ? OriginalPath : TruePath);
            }
            else if (OriginalPath) {

                status = File_GetFileType(&objattrs, FALSE, &FileType, NULL);
                if (status == STATUS_NOT_A_DIRECTORY)
                    status = STATUS_ACCESS_DENIED;
            }
            else {

                //
                // for a write-only path, the directory must be the
                // first (or: highest level) directory which matches
                // the write-only setting.  note that File_GetFileType
                // will need to use SbieApi_OpenFile in this case
                //
                // if the request is for a path below the highest level,
                // we pretend the path does not exist
                //

                int depth = File_CheckDepthForIsWritePath(TruePath);
                if (depth == 0) {
                    FileType = 0;
                    status = STATUS_SUCCESS;
                    //status = File_GetFileType(&objattrs, TRUE, &FileType, NULL);
                    //if (status == STATUS_NOT_A_DIRECTORY)
                    //    status = STATUS_ACCESS_DENIED;
                } else {
                    FileType = 0;
                    if (depth == 1 || HaveCopyParent || HaveSnapshotParent)
                        status = STATUS_OBJECT_NAME_NOT_FOUND;
                    else
                        status = STATUS_OBJECT_PATH_NOT_FOUND;
                }
            }
        } else {

            //
            // otherwise not write-only, so do normal File_GetFileType
            //

            status = STATUS_SUCCESS;
            FileType = 0;

            if (TruePathFlags) {
                if (FILE_PARENT_DELETED(TruePathFlags))
                    status = STATUS_OBJECT_PATH_NOT_FOUND;
                else if (FILE_IS_DELETED(TruePathFlags))
                    status = STATUS_OBJECT_NAME_NOT_FOUND;
            }

            if (NT_SUCCESS(status)) {
                status = File_GetFileType(&objattrs, FALSE, &FileType, NULL);
            }
        }

        if (!Dll_CompartmentMode)
        if ((FileType & TYPE_EFS) != 0) {
            SbieApi_Log(2225, TruePath);
        }

		//
		// If the "true" file is in an snapshot it can be a deleted one, 
		// check for this and act acrodingly.
		//

		if (TruePathFlags & FILE_INSNAPSHOT_FLAG) {

			if (FileType & TYPE_DELETED) {

				status = STATUS_OBJECT_NAME_NOT_FOUND;
			}
		}

        if ((FileType & TYPE_REPARSE_POINT)
                && (CreateOptions & FILE_OPEN_REPARSE_POINT) == 0
                && (! AlreadyReparsed)) {

            //
            // the directory we are accessing might be a reparse point
            //

            WCHAR *ReparsedPath = File_TranslateTempLinks(TruePath, FALSE);
            AlreadyReparsed = TRUE;
            if (ReparsedPath) {
                RtlInitUnicodeString(&objname, ReparsedPath);
                status = File_GetName(
                    NULL, &objname, &TruePath, &CopyPath, &FileFlags);
                Dll_Free(ReparsedPath);
                goto ReparseLoop;
            }
        }

        if (NT_SUCCESS(status) ||
            status == STATUS_OBJECT_NAME_NOT_FOUND ||
            status == STATUS_OBJECT_PATH_NOT_FOUND)
        {

            if (CreateOptions & FILE_DELETE_ON_CLOSE) {
                if (NT_SUCCESS(status))
                    HaveTrueFile = 'Y';
                else
                    HaveTrueFile = 'N';
            } else if (NT_SUCCESS(status))
                    HaveTrueFile = 'y';

            if (status == STATUS_OBJECT_PATH_NOT_FOUND)
                HaveTrueParent = FALSE;
            else
                HaveTrueParent = TRUE;

            //
            // when opening a directory that matches write-only path,
            // we force the creation of the corresponding directory
            // in the copy system, and remove any read-only attributes
            // to make sure File_CheckCreateParameters won't fail
            //

            if (PATH_IS_WRITE(mp_flags) && NT_SUCCESS(status) && !OriginalPath) {
                DesiredAccess |= FILE_GENERIC_WRITE;
                FileType &= ~(TYPE_READ_ONLY | TYPE_SYSTEM);
            }

            status = STATUS_SUCCESS;
        }

        //
        // we don't have CopyPath, but if we did find TruePath, and this
        // is a read-only operation, then let the system handle it
        // (on the TruePath)
        //

        if (FileType && (CreateDisposition == FILE_OPEN ||
                         CreateDisposition == FILE_OPEN_IF)) {

            //
            // exception:  executable images are often accessed as a very
            // specific set of parameters that includes access to write
            // attributes.  in this case we remove the write access.
            //

            if ((DesiredAccess & FILE_DENIED_ACCESS)
                                                == FILE_WRITE_ATTRIBUTES) {

                //
                // don't apply the exception is when the call is coming
                // from File_SetAttributes or File_RenameFile
                //

                if (TlsData->file_dont_strip_write_access == 0)
                    DesiredAccess &= ~FILE_WRITE_ATTRIBUTES;
            }

            //
            // exception:  if the only write-access flag is DELETE, and
            // FILE_DELETE_ON_CLOSE is not requested, then we can drop
            // the DELETE flag.  (this combination is commonly requested
            // by the Win32 DeleteFile API, and it later calls
            // NtSetInformationFile on the returned handle.)
            //

            if (((DesiredAccess & FILE_DENIED_ACCESS) == DELETE) &&
                ((CreateOptions & FILE_DELETE_ON_CLOSE) == 0)) {

                DesiredAccess &= ~DELETE;
            }

            //
            // firefox starting with version 106 opens plugin exe's with GENERIC_WRITE
            // to mitigate this issue we strip this flag when we detect that it tries to 
            // do that with an exe that exists outside the sandbox
            //

            // $Workaround$ - 3rd party fix
            if (Dll_ImageType == DLL_IMAGE_MOZILLA_FIREFOX && (DesiredAccess & GENERIC_WRITE)) {
                const WCHAR *dot = wcsrchr(TruePath, L'.');
                if (dot && _wcsicmp(dot, L".exe") == 0)
                    DesiredAccess &= ~GENERIC_WRITE;
            }

            //
            // having processed the exceptions we can decide if we are
            // going to work on the copy file, or if we are going to
            // let the system work on the true file
            //

            if ((DesiredAccess & FILE_DENIED_ACCESS) == 0) {

                if (TruePathColon)
                    *TruePathColon = L':';
                RtlInitUnicodeString(&objname, TruePath);

                //
                // reduce the access, and call the system
                //

                if (CreateDisposition == FILE_OPEN_IF)
                    CreateDisposition = FILE_OPEN;

                //status = __sys_NtCreateFile(
                status = File_NtCreateTrueFile(
                    FileHandle, DesiredAccess, &objattrs,
                    IoStatusBlock, AllocationSize, FileAttributes,
                    ShareAccess, CreateDisposition, CreateOptions,
                    EaBuffer, EaLength);

                if (NT_SUCCESS(status)) TrueOpened = TRUE;

                //if (status == STATUS_ACCESS_DENIED)
                //{
                //  while(!IsDebuggerPresent()) Sleep(50); __debugbreak();
                //}

                // MSIServer without system
                if (status == STATUS_ACCESS_DENIED && Dll_ImageType == DLL_IMAGE_MSI_INSTALLER //&& Scm_MsiServer_Systemless
                    && ObjectAttributes->ObjectName->Buffer && ObjectAttributes->ObjectName->Length >= 34
                    && _wcsicmp(ObjectAttributes->ObjectName->Buffer + (ObjectAttributes->ObjectName->Length / sizeof(WCHAR)) - 11, L"\\Config.Msi") == 0
                    ) {
                    
                    //
                    // MSI must not fail accessing \??\C:\WINDOWS\Installer\Config.msi but this folder is readable only for system,
                    // so we create a boxed copy instead and open it
                    //
        
                    RtlInitUnicodeString(&objname, CopyPath);
                    //status = __sys_NtCreateFile(
                    status = File_NtCreateCopyFile(
                        FileHandle, DesiredAccess, &objattrs,
                        IoStatusBlock, AllocationSize, FileAttributes,
                        ShareAccess, FILE_OPEN_IF, FILE_DIRECTORY_FILE,
                        EaBuffer, EaLength);
                }

                //
                // special case for SandboxieCrypto on Windows Vista,
                // which tries to open catdb that are locked by
                // the real CryptSvc process.  convert read-only access
                // to write access so the files can be migrated
                //
                // similarly for Windows Update and its DataStore.edb.
                //
                // similarly for the Windows 8 WebCache dllhost and
                // its WebCache v01/v24 data files
                //
                // otherwise, return results directly to caller
                //

                if (Dll_ImageType != DLL_IMAGE_SANDBOXIE_CRYPTO &&
                    Dll_ImageType != DLL_IMAGE_SANDBOXIE_WUAU &&
                    Dll_ImageType != DLL_IMAGE_DLLHOST_WININET_CACHE)
                    __leave;

                if (status != STATUS_SHARING_VIOLATION)
                    __leave;

                DesiredAccess |= FILE_GENERIC_WRITE;
                status = STATUS_SUCCESS;
            }
        }

    }

    //
    // abort early if the parent of CopyPath exists but marked deleted
    //

    if (FILE_PATH_DELETED(TruePathFlags)) { // actual file or its parent
        if (!HaveCopyFile && (HaveTrueFile == 'Y' || HaveTrueFile == 'y')) { // if this is set status will be success

            FileType = 0;
            status = STATUS_OBJECT_PATH_NOT_FOUND;

            //
            // if this is a create operation check if the parent folder is ok and if so clear the error
            //

            if (CreateDisposition != FILE_OPEN && CreateDisposition != FILE_OVERWRITE) {
                if (!FILE_PARENT_DELETED(TruePathFlags)) { // parent not deleted
                    status = STATUS_SUCCESS;
                }
            }
        }
    }

    if (! NT_SUCCESS(status))
        __leave;

    //
    // check creation parameters again, now that we know the file type
    //

    status = File_CheckCreateParameters(
        DesiredAccess, CreateDisposition, CreateOptions, FileType);

    if (status == STATUS_OBJECT_NAME_NOT_FOUND &&
            (! HaveCopyParent) && (! HaveSnapshotParent) && (! HaveTrueParent)) {

        //
        // special case:  File_CheckCreateParameters returns
        // "file not found" but both true and copy parent directory
        // is missing, so convert error to "path not found"
        //

        status = STATUS_OBJECT_PATH_NOT_FOUND;
    }

    if (status == STATUS_FILE_IS_A_DIRECTORY &&
            TruePathColon && CopyPathColon &&
            (CreateOptions & FILE_NON_DIRECTORY_FILE) &&
            (FileType & TYPE_DIRECTORY)) {

        //
        // special case:  accessing an alternate data stream in a
        // directory file.  File_CheckCreateParameters doesn't know
        // about the ADS part, but sees a FILE_NON_DIRECTORY_FILE
        // access to a TYPE_DIRECTORY file, and returns error
        //

        status = STATUS_SUCCESS;
    }

    if (! NT_SUCCESS(status))
        __leave;

    //
    // if caller wants to delete the file, we have to make sure the
    // file (or directory) are deletable (and empty).  we don't support
    // deletion of alternate data streams
    //

    if (CreateOptions & FILE_DELETE_ON_CLOSE) {

        if (TruePathColon || CopyPathColon) {

            status = STATUS_ACCESS_DENIED;
            __leave;
        }

        CreateOptions &= ~FILE_DELETE_ON_CLOSE;
        DeleteOnClose = TRUE;

    } else
        DeleteOnClose = FALSE;

    //
    // for Windows Explorer, any write access on an existing TruePath
    // file that has no corresponding CopyPath file (and disposition
    // is FILE_OPEN), is converted to read-only access
    // in particular this stops Windows Vista Explorer from creating
    // sandboxed files when right-clicking Properties on a file
    //

    if (Dll_ImageType == DLL_IMAGE_SHELL_EXPLORER &&
            (TlsData->file_dont_strip_write_access == 0)) {

        if ((! DeleteOnClose) && (! HaveCopyFile) && FileType
                && (CreateDisposition == FILE_OPEN)
                && PATH_NOT_WRITE(mp_flags)) {

            RtlInitUnicodeString(&objname, TruePath);

            //status = __sys_NtCreateFile(
            status = File_NtCreateTrueFile(
                FileHandle, FILE_GENERIC_READ, &objattrs,
                IoStatusBlock, AllocationSize, FileAttributes,
                ShareAccess, CreateDisposition, CreateOptions,
                EaBuffer, EaLength);

            if (NT_SUCCESS(status)) TrueOpened = TRUE;

            __leave;
        }
    }

    //
    // if we got here, we will have to work on the CopyPath, so we
    // have to create the parent directories, if they're not there
    //
    // we don't do this for write-only paths because we are
    // pretending there is nothing outside the copy system
    //

    if (! HaveCopyParent) {

        if (!HaveTrueParent && Dll_ImageType == DLL_IMAGE_MSI_INSTALLER && Scm_MsiServer_Systemless
            && wcsstr(CopyPath, L"\\system32\\config\\systemprofile\\") != NULL) {

            //
            // MSI must not fail accessing \??\C:\WINDOWS\system32\config\systemprofile\AppData\Local\Temp\ 
            // but this folder is readable only for system, so we create a boxed copy instead and open it
            //

            HaveTrueParent = TRUE;
        }

        if (HaveTrueParent || HaveSnapshotParent) {

            status = File_CreatePath(OriginalPath ? OriginalPath : TruePath, CopyPath);

        } else
            status = STATUS_OBJECT_PATH_NOT_FOUND;

        if (! NT_SUCCESS(status))
            __leave;
    }

    //
    // if TruePath exists while CopyPath does not (note that a file marked
    // deleted is considered to exist), then:
    //

    if ((! HaveCopyFile) && FileType) {

        BOOLEAN IsWritePath = FALSE;
        if ((FileType & TYPE_DIRECTORY) && PATH_IS_WRITE(mp_flags))
            IsWritePath = TRUE;

        //
        // if the operation is to open the existing file non-destructively;
        // or if the operation is destructive but only on one of the streams
        // within the file;  then we migrate TruePath into CopyPath.
        //
        // (note that at this point in the program flow, a non-destructive
        // open, for a CopyPath that does not exist, must also include
        // write access, or else it would have been handled earlier already)
        //

        if (FileType & TYPE_REPARSE_POINT) {

            status = File_MigrateJunction(
                            TruePath, CopyPath, IsWritePath);

        } else if (CreateDisposition == FILE_OPEN ||
            CreateDisposition == FILE_OPEN_IF ||
            TruePathColon) {

            BOOLEAN WithContents = TRUE;

            if (FileType & TYPE_FILE) {

                //
                // we don't actually copy the contents, if the file is
                // opened for the intent of deleting it, which is when
                // both DELETE and FILE_DELETE_ON_CLOSE are specified.
                //

                if (DeleteOnClose && (
                        (DesiredAccess & FILE_DENIED_ACCESS) == DELETE)) {

                    WithContents = FALSE;

                } else if (Dll_ImageType == DLL_IMAGE_WINDOWS_MEDIA_PLAYER) {

                    //
                    // Windows Media Player CurrentDatabase_xxx.wmdb file
                    //

                    WCHAR *dot = wcsrchr(TruePath, L'.');
                    if (dot && _wcsicmp(dot + 1, L"wmdb") == 0) {

                        WithContents = FALSE;
                    }
                }

            } else {

                WithContents = FALSE;
            }

            status = File_MigrateFile(
                            TruePath, CopyPath, IsWritePath, WithContents);

        //
        // if the file is to be overwritten, as opposed to superseded,
        // we don't care about contents but we must copy its attributes
        // from TruePath
        //

        } else if ( CreateDisposition == FILE_OVERWRITE ||
                    CreateDisposition == FILE_OVERWRITE_IF) {

            status = File_MigrateFile(
                            TruePath, CopyPath, IsWritePath, FALSE);
        }

        //
        // if migration reports the file is too big, then ask for
        // read-only access to the TruePath
        //

        if (! NT_SUCCESS(status)) {

            if (status == STATUS_BAD_INITIAL_PC) {

                if (TruePathColon)
                    *TruePathColon = L':';
                RtlInitUnicodeString(&objname, TruePath);

                DesiredAccess &= ~FILE_DENIED_ACCESS;
                CreateOptions &= ~FILE_DELETE_ON_CLOSE;

                //status = __sys_NtCreateFile(
                status = File_NtCreateTrueFile(
                    FileHandle, DesiredAccess, &objattrs, IoStatusBlock,
                    AllocationSize, FileAttributes, ShareAccess,
                    CreateDisposition, CreateOptions, EaBuffer, EaLength);

                if (NT_SUCCESS(status)) TrueOpened = TRUE;
            }

            __leave;
        }
    }

    //
    // an alternative case is if the caller is asking to create CopyPath
    // through a destructive operation, and CopyPath is marked deleted.
    // in this case we physically delete the stale CopyPath.
    //

    if (!File_Delete_v2) {

        DeleteChildren = FALSE;

        if (HaveCopyFile && (FileType & TYPE_DELETED) &&
                (CreateDisposition != FILE_OPEN)) {

            RtlInitUnicodeString(&objname, CopyPath);
            status = __sys_NtDeleteFile(&objattrs);

            if (! NT_SUCCESS(status))
                __leave;

            FileType = 0;

            if ((CreateOptions & FILE_DIRECTORY_FILE) && (! CopyPathColon)) {

                //
                // if the caller is re-creating a directory that was already
                // deleted (and marked so) in the sandbox, then we should mark
                // everything in it as deleted, after it has been re-created
                //

                DeleteChildren = TRUE;
            }
        }
    }

    //
    // Note: This is disabled in the driver since Win 10 1903 (see my comments in file.c in File_Generic_MyParseProc).
    // if the caller specifies write attributes, this is only permitted
    // on non-directory files, so we must be sure to tell the driver
    //

    /*if (DesiredAccess & DIRECTORY_JUNCTION_ACCESS) {

        if ((CreateOptions & FILE_DIRECTORY_FILE) ||
                (FileType & TYPE_DIRECTORY) &&
                (! TruePathColon) && (! CopyPathColon)) {

            DesiredAccess &= ~DIRECTORY_JUNCTION_ACCESS;
            DesiredAccess |= FILE_GENERIC_READ;

        } else {

            CreateOptions |= FILE_NON_DIRECTORY_FILE;
        }
    }*/

    //
    // finally we are ready to execute the caller's request on CopyPath.
    //
    // if FILE_DELETE_ON_CLOSE was specified on a copy file that has
    // no matching true file, then try to really delete the file
    //

    if (CopyPathColon)
        *CopyPathColon = L':';
    RtlInitUnicodeString(&objname, CopyPath);

    if (DeleteOnClose && (File_Delete_v2 || HaveTrueFile == 'N')) {

        CreateOptions |= FILE_DELETE_ON_CLOSE;
        DesiredAccess |= DELETE;
    }

    //status = __sys_NtCreateFile(
    status = File_NtCreateCopyFile(
        FileHandle, DesiredAccess | FILE_READ_ATTRIBUTES,
        &objattrs, IoStatusBlock, AllocationSize, FileAttributes,
        ShareAccess, CreateDisposition, CreateOptions, EaBuffer, EaLength);

    if (!NT_SUCCESS(status) && DeleteOnClose && !File_Delete_v2 && HaveTrueFile == 'N') {

        CreateOptions &= ~FILE_DELETE_ON_CLOSE;
        DesiredAccess &= ~DELETE;

        //status = __sys_NtCreateFile(
        status = File_NtCreateCopyFile(
            FileHandle, DesiredAccess | FILE_READ_ATTRIBUTES,
            &objattrs, IoStatusBlock, AllocationSize, FileAttributes,
            ShareAccess, CreateDisposition, CreateOptions,
            EaBuffer, EaLength);
    }

    //
    // if the file was opened with FILE_DELETE_ON_CLOSE, then invoke
    // File_MarkDeleted to mark it deleted.  otherwise make sure
    // the creation timestamp is not marked deleted.
    //

    if (NT_SUCCESS(status)) {

        if (DeleteOnClose) {

            //
            // if we have a corresponding true file, then mark the copy
            // file deleted, unless it's an alternate data stream
            //

            if (HaveTrueFile == 'Y') {

                if (CopyPathColon)
                    status = STATUS_ACCESS_DENIED;
                else if (File_Delete_v2)
                    status = File_MarkDeleted_v2(OriginalPath ? OriginalPath : TruePath);
                else
                    status = File_MarkDeleted(*FileHandle, CopyPath);
            }

        } else {

            BOOLEAN IsRecover = FALSE;

            //
            // file was not opened for deletion, but NTFS file systems
            // may sometimes persist an (out of date) creation time
            //
            if (!File_Delete_v2)
                status = File_SetCreateTime(*FileHandle, CopyPath);

            if (NT_SUCCESS(status)) {

                if (CreateOptions & FILE_DIRECTORY_FILE) {

                    //
                    // if a directory has been created over a deleted
                    // directory, then mark all its children deleted
                    //

                    if (!File_Delete_v2) {
                        if (DeleteChildren) {

                            TlsData->file_NtCreateFile_lock = FALSE;

                            File_MarkChildrenDeleted(TruePath);
                        }
                    }

                } else {

                    if (FileType == 0 || IsEmptyCopyFile ||
                            (FileType & TYPE_DELETED)) {

                        //
                        // if a non-directory file has been created where one
                        // did not exist before (even outside the sandbox),
                        // or was deleted, then record it for recovery
                        //

                        IsRecover = File_RecordRecover(*FileHandle, TruePath);
                    }
                }

                //
                // if a file or directory has been created where one did
                // not exist before (even outside the sandbox), or was
                // deleted, then check the short name for duplicates
                //

                if ((! IsRecover) &&
                        (FileType == 0 || (FileType & TYPE_DELETED))) {

                    BOOLEAN adjusted = File_AdjustShortName(
                                            TruePath, CopyPath, *FileHandle);
                    if (adjusted) {

                        //
                        // file handle was closed in File_AdjustShortName
                        //

                        //status = __sys_NtCreateFile(
                        status = File_NtCreateCopyFile(
                            FileHandle, DesiredAccess | FILE_READ_ATTRIBUTES,
                            &objattrs, IoStatusBlock,
                            AllocationSize, FileAttributes,
                            ShareAccess, FILE_OPEN, CreateOptions,
                            EaBuffer, EaLength);

                        if (! NT_SUCCESS(status))
                            *FileHandle = NULL;
                    }
                }
            }
        }

        if (! NT_SUCCESS(status)) {
            if (*FileHandle) {
                NtClose(*FileHandle);
                *FileHandle = NULL;
            }
            IoStatusBlock->Information = 0;
        }
    }

    //
    // finish
    //

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    //
    // we can get access denied in a restricted Chrome sandbox process:
    // perhaps we tried to access a file/device in the box which isn't
    // accessible to a restricted token, but the real file might be
    // accessible, so try to access the real file
    //

    if (Dll_RestrictedToken && status == STATUS_ACCESS_DENIED && !SkipOriginalTry) {

        status = __sys_NtCreateFile(
            FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock,
            AllocationSize, FileAttributes, ShareAccess, CreateDisposition,
            CreateOptions, EaBuffer, EaLength);

        if (NT_SUCCESS(status)) TrueOpened = TRUE; // is that right?

        if (! NT_SUCCESS(status))
            status = STATUS_ACCESS_DENIED;
    }

    //
    // Relocation, if we opened a relocated location we need to 
    // store the original true path for the File_GetName function
    //

    if (TrueOpened && OriginalPath) {

        Handle_SetRelocationPath(*FileHandle, OriginalPath);
    }

    //
    // finish
    //

    Dll_PopTlsNameBuffer(TlsData);

    TlsData->file_NtCreateFile_lock = FALSE;

    __try {

        if (! NT_SUCCESS(status))
            IoStatusBlock->Status = status;

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    if (Dll_ApiTrace || Dll_FileTrace) {
        WCHAR trace_str[2048];
        ULONG len = Sbie_snwprintf(trace_str, 2048, L"File_NtCreateFileImpl status = 0x%08X", status);
        SbieApi_MonitorPut2Ex(MONITOR_APICALL | MONITOR_TRACE, len, trace_str, FALSE, FALSE);
    }

    if(pSecurityDescriptor)
        Dll_Free(pSecurityDescriptor);

    SetLastError(LastError);
    return status;
}


//---------------------------------------------------------------------------
// File_NtCreateTrueFile
//---------------------------------------------------------------------------


_FX NTSTATUS File_NtCreateTrueFile(
    HANDLE *FileHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    IO_STATUS_BLOCK *IoStatusBlock,
    LARGE_INTEGER *AllocationSize,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    void *EaBuffer,
    ULONG EaLength)
{
    NTSTATUS status = __sys_NtCreateFile(
        FileHandle, DesiredAccess, ObjectAttributes,
        IoStatusBlock, AllocationSize, FileAttributes,
        ShareAccess, CreateDisposition, CreateOptions,
        EaBuffer, EaLength);

    if (!Dll_CompartmentMode)
    if (status == STATUS_ACCESS_DENIED
      && SbieApi_QueryConfBool(NULL, L"EnableEFS", FALSE)) {

        WCHAR* TruePath = ObjectAttributes->ObjectName->Buffer;

        //
        // check if we are handling a EFS file or folder
        //

        ULONG FileType;
        status = File_GetFileType(ObjectAttributes, FALSE, &FileType, NULL);

        if (status == STATUS_OBJECT_NAME_NOT_FOUND && CreateDisposition != 0) {

            //
            // check status of parent directory
            //

            WCHAR* ptr1 = wcsrchr(TruePath, L'\\');
            *ptr1 = L'\0';
            RtlInitUnicodeString(ObjectAttributes->ObjectName, TruePath);

            status = File_GetFileType(ObjectAttributes, FALSE, &FileType, NULL);

            *ptr1 = L'\\';
            RtlInitUnicodeString(ObjectAttributes->ObjectName, TruePath);
        }

        if (NT_SUCCESS(status) && (FileType & TYPE_EFS) != 0) {

            //
            // invoke NtCreateFile Proxy
            //

            status = File_NtCreateFileProxy(
                FileHandle, DesiredAccess, ObjectAttributes,
                IoStatusBlock, AllocationSize, FileAttributes,
                ShareAccess, CreateDisposition, CreateOptions,
                EaBuffer, EaLength);

            if(!NT_SUCCESS(status))
                SbieApi_Log(2225, TruePath);
        }
    }

    return status;
}


//---------------------------------------------------------------------------
// File_NtCreateCopyFile
//---------------------------------------------------------------------------


static NTSTATUS File_NtCreateCopyFile(
    PHANDLE FileHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PIO_STATUS_BLOCK IoStatusBlock,
    PLARGE_INTEGER AllocationSize,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    PVOID EaBuffer,
    ULONG EaLength)
{
    NTSTATUS status = __sys_NtCreateFile(
        FileHandle, DesiredAccess, ObjectAttributes,
        IoStatusBlock, AllocationSize, FileAttributes,
        ShareAccess, CreateDisposition, CreateOptions,
        EaBuffer, EaLength);

    return status;
}


//---------------------------------------------------------------------------
// File_NtCreateFileProxy
//---------------------------------------------------------------------------


NTSTATUS File_NtCreateFileProxy(
    HANDLE *FileHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    IO_STATUS_BLOCK *IoStatusBlock,
    LARGE_INTEGER *AllocationSize,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    void *EaBuffer,
    ULONG EaLength)
{
    NTSTATUS status;
    static WCHAR* _QueueName = NULL;

    if (!_QueueName) {
        _QueueName = Dll_Alloc(32 * sizeof(WCHAR));
        Sbie_snwprintf(_QueueName, 32, L"*USERPROXY_%08X", Dll_SessionId);
    }

    if (ObjectAttributes->RootDirectory != NULL || ObjectAttributes->ObjectName == NULL) {

        SbieApi_Log(2205, L"NtCreateFile (EFS)");
        return STATUS_ACCESS_DENIED;
    }

    ULONG path_len = ObjectAttributes->ObjectName->Length + sizeof(WCHAR);
    ULONG req_len = sizeof(USER_OPEN_FILE_REQ) + path_len + EaLength;
    ULONG path_pos = sizeof(USER_OPEN_FILE_REQ);
    ULONG ea_pos = path_pos + path_len;

    USER_OPEN_FILE_REQ *req = (USER_OPEN_FILE_REQ *)Dll_AllocTemp(req_len);

    WCHAR* path_buff = ((UCHAR*)req) + path_pos;
    memcpy(path_buff, ObjectAttributes->ObjectName->Buffer, path_len);

    if (EaBuffer && EaLength > 0) {
        void* ea_buff = ((UCHAR*)req) + ea_pos;
        memcpy(ea_buff, EaBuffer, EaLength);
    }

    req->msgid = USER_OPEN_FILE;

    req->DesiredAccess = DesiredAccess;
    req->FileNameOffset = path_pos;
    //req->FileNameSize = path_len;
    req->AllocationSize = AllocationSize ? AllocationSize->QuadPart : 0;
    req->FileAttributes = FileAttributes;
    req->ShareAccess = ShareAccess;
    req->CreateDisposition = CreateDisposition;
    req->CreateOptions = CreateOptions;
    req->EaBufferOffset = EaBuffer ? ea_pos : 0;
    req->EaLength = EaLength;

    USER_OPEN_FILE_RPL *rpl = SbieDll_CallProxySvr(_QueueName, req, req_len, sizeof(*rpl), 100);
    if (!rpl) {
        status = STATUS_INTERNAL_ERROR;
        goto finish;
    }

    if (NT_SUCCESS(rpl->status)) {
        status = rpl->error;
        *FileHandle = (HANDLE)rpl->FileHandle;
        IoStatusBlock->Status = rpl->Status;
        IoStatusBlock->Information = (ULONG_PTR)rpl->Information;
    }

    Dll_Free(rpl);
finish:
    Dll_Free(req);
    return status;
}


//---------------------------------------------------------------------------
// File_CheckCreateParameters
//---------------------------------------------------------------------------


_FX NTSTATUS File_CheckCreateParameters(
    ACCESS_MASK DesiredAccess, ULONG CreateDisposition,
    ULONG CreateOptions, ULONG FileType)
{
    BOOLEAN FileExists;
    BOOLEAN FileReadOnly, FileSystem;

    //
    // if the caller is asking for synchronous operation, then
    // DesiredAccess must include SYNCHRONIZE
    //

    if (    (CreateOptions & (
                FILE_SYNCHRONOUS_IO_ALERT | FILE_SYNCHRONOUS_IO_NONALERT)) &&
            ((DesiredAccess & SYNCHRONIZE) == 0)) {

        return STATUS_INVALID_PARAMETER;
    }

    //
    // the caller cannot specify both directory and non-directory
    //

    if ((CreateOptions &
            (FILE_DIRECTORY_FILE | FILE_NON_DIRECTORY_FILE))
            == (FILE_DIRECTORY_FILE | FILE_NON_DIRECTORY_FILE)) {

        return STATUS_INVALID_PARAMETER;
    }

    //
    // if the caller specifies the directory file bit in CreateOptions,
    // then CreateDisposition must not be FILE_SUPERSEDE,
    // FILE_OVERWRITE or FILE_OVERWRITE_IF
    //

    if (CreateOptions & FILE_DIRECTORY_FILE)
    {
        if (CreateDisposition == FILE_SUPERSEDE ||
            CreateDisposition == FILE_OVERWRITE ||
            CreateDisposition == FILE_OVERWRITE_IF) {

            return STATUS_INVALID_PARAMETER;
        }
    }

    //
    // if CreateOptions specifies FILE_DELETE_ON_CLOSE, then
    // DesiredAccess must specify DELETE
    //

    if (CreateOptions & FILE_DELETE_ON_CLOSE &&
            (! (DesiredAccess & DELETE))) {

        return STATUS_INVALID_PARAMETER;
    }

    //
    // split off the FILE_DELETE_ON_CLOSE bit of FileType into FileExists,
    // and reset that bit in FileType, for easier comparisons
    //

    if (FileType == -1)
        return STATUS_SUCCESS;

    FileExists = FALSE;
    FileReadOnly = FALSE;
    FileSystem = FALSE;

    if (FileType & TYPE_DELETED) {

        FileType = 0;

    } else if (FileType != 0) {

        FileExists = TRUE;

        if (FileType & TYPE_READ_ONLY)
            FileReadOnly = TRUE;

        if (FileType & TYPE_SYSTEM)
            FileSystem = TRUE;

        FileType &= TYPE_DIRECTORY | TYPE_FILE;
    }

    //
    // file must already exist (and not be marked as deleted),
    // for FILE_OPEN or FILE_OVERWRITE requests
    //

    if ((! FileExists) && (
            CreateDisposition == FILE_OPEN ||
            CreateDisposition == FILE_OVERWRITE)) {

        return STATUS_OBJECT_NAME_NOT_FOUND;
    }

    //
    // file must not be a directory, for FILE_SUPERSEDE, FILE_OVERWRITE
    // and FILE_OVERWRITE_IF requests.  the status code differentiates
    // between trying to overwrite a directory when FILE_NON_DIRECTORY_FILE
    // bit is specified, and when that bit isn't specified.
    //

    if ((FileType & TYPE_DIRECTORY) && (
            CreateDisposition == FILE_SUPERSEDE ||
            CreateDisposition == FILE_OVERWRITE ||
            CreateDisposition == FILE_OVERWRITE_IF)) {

        if (CreateOptions & FILE_NON_DIRECTORY_FILE)
            return STATUS_FILE_IS_A_DIRECTORY;
        else
            return STATUS_OBJECT_NAME_COLLISION;
    }

    //
    // for FILE_CREATE, the file must not exist.  if it does exist,
    // generally the error code is STATUS_OBJECT_NAME_COLLISION,
    // unless explicitly requesting to create a regular file while
    // the existing file is a directory, in which case the error
    // code is STATUS_FILE_IS_A_DIRECTORY
    //

    if (FileExists && (CreateDisposition == FILE_CREATE)) {

        if ((FileType & TYPE_DIRECTORY) &&
                (CreateOptions & FILE_NON_DIRECTORY_FILE))
            return STATUS_FILE_IS_A_DIRECTORY;
        else
            return STATUS_OBJECT_NAME_COLLISION;
    }

    //
    // for FILE_OPEN and FILE_OPEN_IF with an explicit specification
    // of a file type, verify that the file type is indeed as expected
    //

    if (CreateDisposition == FILE_OPEN ||
        CreateDisposition == FILE_OPEN_IF) {

        if ((CreateOptions & FILE_DIRECTORY_FILE) &&
                        (FileType & TYPE_FILE))
            return STATUS_NOT_A_DIRECTORY;

        else if ((CreateOptions & FILE_NON_DIRECTORY_FILE) &&
                        (FileType & TYPE_DIRECTORY))
            return STATUS_FILE_IS_A_DIRECTORY;
    }

    //
    // for most write access requests (explicitly through DesiredAccess,
    // or if CreateDisposition is any of the destructive operations), the
    // file must not be read-only.  access to write attributes and to
    // the SACL is allowed even on a read-only file.
    //
    // additionally, system files (but not hidden files) cannot be the target
    // of a destructive operation, regardless of the write access
    //

    if (FileSystem && (
            CreateDisposition == FILE_SUPERSEDE ||
            CreateDisposition == FILE_OVERWRITE ||
            CreateDisposition == FILE_OVERWRITE_IF)) {

        FileReadOnly = TRUE;
    }

    if (FileReadOnly && (CreateOptions & FILE_DELETE_ON_CLOSE))
        return STATUS_CANNOT_DELETE;

    if (FileReadOnly && (
            CreateDisposition == FILE_OPEN ||
            CreateDisposition == FILE_OPEN_IF)) {

        //
        // if the access is only write attributes or access SACL or DACL
        // then pretend the file is not read-only so the access is allowed
        //

        const ACCESS_MASK DeniedAccess = DesiredAccess & FILE_DENIED_ACCESS;
        const ACCESS_MASK AllowedAccess = FILE_WRITE_ATTRIBUTES | DELETE
                                        | ACCESS_SYSTEM_SECURITY
                                        | WRITE_OWNER | WRITE_DAC;
        if (DeniedAccess & AllowedAccess)
            if ((DeniedAccess & ~(AllowedAccess)) == 0)
                FileReadOnly = FALSE;
    }

    if (FileReadOnly && (
            (DesiredAccess & FILE_DENIED_ACCESS) || (
                CreateDisposition == FILE_SUPERSEDE ||
                CreateDisposition == FILE_OVERWRITE ||
                CreateDisposition == FILE_OVERWRITE_IF))) {

        if (CreateOptions & FILE_DELETE_ON_CLOSE)
            return STATUS_CANNOT_DELETE;
        else
            return STATUS_ACCESS_DENIED;
    }

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// File_GetFileType
//---------------------------------------------------------------------------


_FX NTSTATUS File_GetFileType(
    OBJECT_ATTRIBUTES *ObjectAttributes, BOOLEAN IsWritePath,
    ULONG *FileType, BOOLEAN *IsEmpty)
{
    NTSTATUS status;
    FILE_NETWORK_OPEN_INFORMATION info;
    ULONG type;

    *FileType = 0;

    P_NtQueryFullAttributesFile pNtQueryFullAttributesFile = __sys_NtQueryFullAttributesFile;
    // special case for File_InitRecoverFolders as it's called before we hook those functions
    if (!pNtQueryFullAttributesFile)
        pNtQueryFullAttributesFile = NtQueryFullAttributesFile;

    if (IsWritePath) {
        status = File_QueryFullAttributesDirectoryFile(
                            ObjectAttributes->ObjectName->Buffer, &info);
    } else {
        status = pNtQueryFullAttributesFile(ObjectAttributes, &info);
    }

    if (! NT_SUCCESS(status)) {
        if (status == STATUS_NO_SUCH_FILE)
            status = STATUS_OBJECT_NAME_NOT_FOUND;

        return status;      // with *FileType = 0
    }

    //
    // the type returned is TYPE_DIRECTORY or TYPE_FILE.
    // for files marked deleted, we set the TYPE_DELETED bit.
    // for read-only files, and hidden/system files, we set other bits.
    //

    if (info.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        type = TYPE_DIRECTORY;
        if (info.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
            type |= TYPE_REPARSE_POINT;
    } else {
        type = TYPE_FILE;
        if (IsEmpty && info.EndOfFile.QuadPart == 0)
            *IsEmpty = TRUE;
    }

    if (info.FileAttributes & FILE_ATTRIBUTE_READONLY)
        type |= TYPE_READ_ONLY;
    if (info.FileAttributes & FILE_ATTRIBUTE_SYSTEM)
        type |= TYPE_SYSTEM;

    if (info.FileAttributes & FILE_ATTRIBUTE_ENCRYPTED)
        type |= TYPE_EFS;

    if (!File_Delete_v2) {
        if (IS_DELETE_MARK(&info.CreationTime))
            type |= TYPE_DELETED;
    }

    *FileType = type;

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// File_CheckDeletedParent
//---------------------------------------------------------------------------


_FX BOOLEAN File_CheckDeletedParent(WCHAR *CopyPath)
{
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    ULONG FileType;
    WCHAR *ptr = NULL;
	NTSTATUS status;

    //
    // remove the last path component so we can open the parent directory
    //

    while (1) {

        WCHAR *ptr_old = ptr;
        ptr = wcsrchr(CopyPath, L'\\');
        if (ptr_old)
            *ptr_old = L'\\';

        if ((! ptr) || ptr == CopyPath)
            return FALSE;

        *ptr = L'\0';

        InitializeObjectAttributes(
            &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

        RtlInitUnicodeString(&objname, CopyPath);

        if (objname.Length <= Dll_BoxFilePathLen * sizeof(WCHAR)) {
            *ptr = L'\\';
            return FALSE;
        }

		status = File_GetFileType(&objattrs, FALSE, &FileType, NULL);
		if (status == STATUS_OBJECT_NAME_NOT_FOUND || status == STATUS_OBJECT_PATH_NOT_FOUND)
			continue;

        if (FileType & TYPE_DELETED) {
            *ptr = L'\\';
            return TRUE;
        }

		//
		// If we have snapshots check their status, if we have a entry in the most recent snapshot
		// than older delete markings are not relevant
		//

		for (FILE_SNAPSHOT* Cur_Snapshot = File_Snapshot; Cur_Snapshot != NULL; Cur_Snapshot = Cur_Snapshot->Parent)
		{
			WCHAR* TmplName = File_MakeSnapshotPath(Cur_Snapshot, CopyPath);
			if (!TmplName)
				break;

			RtlInitUnicodeString(&objname, TmplName);
			status = File_GetFileType(&objattrs, FALSE, &FileType, NULL);
			if (status == STATUS_OBJECT_NAME_NOT_FOUND || status == STATUS_OBJECT_PATH_NOT_FOUND)
				continue;

			if (FileType & TYPE_DELETED) {
				*ptr = L'\\';
				return TRUE;
			}

			if (NT_SUCCESS(status))
				break;
		}
    }
}


//---------------------------------------------------------------------------
// File_CreatePath
//---------------------------------------------------------------------------


_FX NTSTATUS File_CreatePath(WCHAR *TruePath, WCHAR *CopyPath)
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    HANDLE handle;
    WCHAR *path;
    WCHAR *sep, *sep2;
    WCHAR savechar, savechar2;
    USHORT savelength;
    USHORT savemaximumlength;
    ULONG TruePath_len, CopyPath_len;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_BASIC_INFORMATION basic_info;
    BOOLEAN IsDeleted = FALSE;
    OBJECT_ATTRIBUTES objattrs2;
    UNICODE_STRING objname2;
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;

    //
    // first we traverse backward along the path, removing the last
    // path component each time, and trying to create the path that
    // we have left.  we stop when we succeed, ie, when we reach
    // the end of the existing directory tree.
    //

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, Secure_NormalSD);

    RtlInitUnicodeString(&objname, CopyPath);

    InitializeObjectAttributes(
        &objattrs2, &objname2, OBJ_CASE_INSENSITIVE, NULL, Secure_NormalSD);

    RtlInitUnicodeString(&objname2, TruePath);

    TruePath_len = wcslen(TruePath);
    CopyPath_len = objname.Length / sizeof(WCHAR);

    path = objname.Buffer;
    sep = path + CopyPath_len;

    while (1) {

        --sep;
        while ((sep > path) && (*sep != L'\\'))
            --sep;

        if (sep <= path) {
            //
            // we went back all the way to the first character.  this
            // shouldn't happen in practice, because we certainly have
            // stopped at existing directories before getting here.
            //
            return STATUS_OBJECT_PATH_INVALID;
        }

        //
        // chop off the last component of the path, and try to open
        // or create it.  if we succeed, break out of this loop.
        // (unless the directory was already there, and marked deleted.)
        //

        savechar = *sep;
        *sep = L'\0';

        sep2 = TruePath + TruePath_len - (CopyPath_len - (sep - CopyPath));
        savechar2 = *sep2;
        *sep2 = L'\0';

        if (Secure_CopyACLs) {
            
            if (pSecurityDescriptor) {
                Dll_Free(pSecurityDescriptor);
                pSecurityDescriptor = NULL;
            }

            savelength = objname2.Length;
            savemaximumlength = objname2.MaximumLength;
            objname2.Length = (sep2 - TruePath) * sizeof(WCHAR);
            objname2.MaximumLength = objname2.Length + sizeof(WCHAR);

            status = __sys_NtCreateFile(
                &handle, FILE_READ_ATTRIBUTES, &objattrs2,
                &IoStatusBlock, NULL,
                FILE_ATTRIBUTE_NORMAL, FILE_SHARE_VALID_FLAGS,
                FILE_OPEN, FILE_DIRECTORY_FILE, NULL, 0);

            if (NT_SUCCESS(status)) {

                ULONG lengthNeeded = 0;
                status = NtQuerySecurityObject(handle, DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION | /*OWNER_SECURITY_INFORMATION |*/ GROUP_SECURITY_INFORMATION, NULL, 0, &lengthNeeded);
                if (status == STATUS_BUFFER_TOO_SMALL) {
                    pSecurityDescriptor = (PSECURITY_DESCRIPTOR)Dll_AllocTemp(lengthNeeded);
                    status = NtQuerySecurityObject(handle, DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION | /*OWNER_SECURITY_INFORMATION |*/ GROUP_SECURITY_INFORMATION, pSecurityDescriptor, lengthNeeded, &lengthNeeded);
                    if (NT_SUCCESS(status)) 
                        File_AddCurrentUserToSD(&pSecurityDescriptor);
                    else {
                        Dll_Free(pSecurityDescriptor);
                        pSecurityDescriptor = NULL;
                    }
                }

                NtClose(handle);
            }

            InitializeObjectAttributes(
                &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, pSecurityDescriptor ? pSecurityDescriptor : Secure_NormalSD);
        }

        savelength = objname.Length;
        savemaximumlength = objname.MaximumLength;
        objname.Length = (sep - path) * sizeof(WCHAR);
        objname.MaximumLength = objname.Length + sizeof(WCHAR);

        status = __sys_NtCreateFile(
            &handle, FILE_READ_ATTRIBUTES | DELETE, &objattrs,
            &IoStatusBlock, NULL, 0, FILE_SHARE_VALID_FLAGS,
            FILE_OPEN_IF, FILE_DIRECTORY_FILE, NULL, 0);

        if (File_Delete_v2) {
            if (!NT_SUCCESS(status)) {
                IsDeleted = FILE_IS_DELETED(File_IsDeletedEx(TruePath, CopyPath, NULL));
            }
        }

        objname.Length = savelength;
        objname.MaximumLength = savemaximumlength;
        *sep = savechar;
        *sep2 = savechar2;

        if (NT_SUCCESS(status)) {

            if (!File_Delete_v2) {

                status = __sys_NtQueryInformationFile(
                    handle, &IoStatusBlock, &basic_info,
                    sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);

                if (NT_SUCCESS(status)) {
                    IsDeleted = IS_DELETE_MARK(&basic_info.CreationTime);
                }
            }

            NtClose(handle);

            if (IsDeleted) {

                return STATUS_OBJECT_PATH_NOT_FOUND;
            }

            break;
        }

        if (status != STATUS_OBJECT_NAME_NOT_FOUND &&
            status != STATUS_OBJECT_PATH_NOT_FOUND)
        {
            return status;
        }
    }

    //
    // copy the short file name, for the directory we just created
    //

    savechar = *sep;
    *sep = L'\0';

    sep2 = TruePath + TruePath_len - (CopyPath_len - (sep - CopyPath));
    savechar2 = *sep2;
    *sep2 = L'\0';

    if (_wcsicmp(sep2 + 1, sep + 1) == 0)
        File_CopyShortName(TruePath, CopyPath);

    *sep = savechar;
    *sep2 = savechar2;

    //
    // now traverse forward, creating all the missing directories
    // in the hierarchy.
    //

    while (1) {

        ++sep;
        ++sep2;
        while (*sep && *sep != L'\\') {
            ++sep;
            ++sep2;
        }

        if (! *sep)
            break;

        savechar = *sep;
        *sep = L'\0';

        if (Secure_CopyACLs) {
            
            if (pSecurityDescriptor) {
                Dll_Free(pSecurityDescriptor);
                pSecurityDescriptor = NULL;
            }

            savechar2 = *sep2;
            *sep2 = L'\0';

            savelength = objname2.Length;
            savemaximumlength = objname2.MaximumLength;
            objname2.Length = (sep2 - TruePath) * sizeof(WCHAR);
            objname2.MaximumLength = objname2.Length + sizeof(WCHAR);

            status = __sys_NtCreateFile(
                &handle, FILE_READ_ATTRIBUTES, &objattrs2,
                &IoStatusBlock, NULL,
                FILE_ATTRIBUTE_NORMAL, FILE_SHARE_VALID_FLAGS,
                FILE_OPEN, FILE_DIRECTORY_FILE, NULL, 0);

            if (NT_SUCCESS(status)) {

                ULONG lengthNeeded = 0;
                status = NtQuerySecurityObject(handle, DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION | /*OWNER_SECURITY_INFORMATION |*/ GROUP_SECURITY_INFORMATION, NULL, 0, &lengthNeeded);
                if (status == STATUS_BUFFER_TOO_SMALL) {
                    pSecurityDescriptor = (PSECURITY_DESCRIPTOR)Dll_AllocTemp(lengthNeeded);
                    status = NtQuerySecurityObject(handle, DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION | /*OWNER_SECURITY_INFORMATION |*/ GROUP_SECURITY_INFORMATION, pSecurityDescriptor, lengthNeeded, &lengthNeeded);
                    if (NT_SUCCESS(status)) 
                        File_AddCurrentUserToSD(&pSecurityDescriptor);
                    else {
                        Dll_Free(pSecurityDescriptor);
                        pSecurityDescriptor = NULL;
                    }
                }

                NtClose(handle);
            }

            InitializeObjectAttributes(
                &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, pSecurityDescriptor ? pSecurityDescriptor : Secure_NormalSD);

            *sep2 = savechar2;
        }

        savelength = objname.Length;
        savemaximumlength = objname.MaximumLength;
        objname.Length = (sep - path) * sizeof(WCHAR);
        objname.MaximumLength = objname.Length + sizeof(WCHAR);

        status = __sys_NtCreateFile(
            &handle, FILE_READ_ATTRIBUTES | DELETE, &objattrs,
            &IoStatusBlock, NULL,
            FILE_ATTRIBUTE_NORMAL, FILE_SHARE_VALID_FLAGS,
            FILE_OPEN_IF, FILE_DIRECTORY_FILE, NULL, 0);

        if (NT_SUCCESS(status)) {

            NtClose(handle);

            savechar2 = *sep2;
            *sep2 = L'\0';

            if (_wcsicmp(sep2 + 1, sep + 1) == 0)
                File_CopyShortName(TruePath, CopyPath);

            *sep2 = savechar2;
        }

        objname.Length = savelength;
        objname.MaximumLength = savemaximumlength;
        *sep = savechar;

        if (! NT_SUCCESS(status))
            break;
    }
    
    if(pSecurityDescriptor)
        Dll_Free(pSecurityDescriptor);

    return status;
}


//---------------------------------------------------------------------------
// File_CopyShortName
//---------------------------------------------------------------------------


_FX NTSTATUS File_CopyShortName(
    const WCHAR *TruePath, const WCHAR *CopyPath)
{
    NTSTATUS status;
    HANDLE handle;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    IO_STATUS_BLOCK IoStatusBlock;
    union {
        FILE_BOTH_DIRECTORY_INFORMATION dir;
        UCHAR space[128];
    } info;
    WCHAR save_char;
    WCHAR *backslash;

    //
    // open the directory containing the last element of TruePath.
    // keep the trailing backslash, in case we are opening the
    // root directory of the volume
    //

    backslash = wcsrchr(TruePath, L'\\');
    if ((! backslash) || (! backslash[1]))
        return STATUS_SUCCESS;

    save_char = backslash[1];
    backslash[1] = L'\0';

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    RtlInitUnicodeString(&objname, TruePath);

    status = __sys_NtCreateFile(
        &handle, FILE_GENERIC_READ, &objattrs, &IoStatusBlock, NULL,
        0, FILE_SHARE_VALID_FLAGS, FILE_OPEN,
        FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

    backslash[1] = save_char;

    if (! NT_SUCCESS(status))
        return status;

    //
    // query the short name
    //

    RtlInitUnicodeString(&objname, backslash + 1);

    info.dir.NextEntryOffset = tzuk;

    status = __sys_NtQueryDirectoryFile(
        handle, NULL, NULL, NULL, &IoStatusBlock,
        &info.dir, sizeof(info), FileBothDirectoryInformation,
        TRUE, &objname, FALSE);

    if (status == STATUS_BUFFER_OVERFLOW) {
        status = STATUS_SUCCESS;

        //
        // although STATUS_BUFFER_OVERFLOW should fill as much of the buffer
        // as we gave it, some faulty drivers (e.g. avast! pro) may return
        // this status without touching the buffer at all, in this case retry
        // with a larger buffer
        //

        if (info.dir.NextEntryOffset == tzuk) {

            FILE_BOTH_DIRECTORY_INFORMATION *dir2 = Dll_AllocTemp(1024);

            status = __sys_NtQueryDirectoryFile(
                handle, NULL, NULL, NULL, &IoStatusBlock,
                dir2, 1024, FileBothDirectoryInformation,
                TRUE, &objname, FALSE);

            memcpy(&info.dir, dir2, sizeof(info));

            Dll_Free(dir2);

            if (status != STATUS_SUCCESS)
                return FALSE;
        }
    }

    NtClose(handle);

    //
    // set the short file name.  if we get STATUS_OBJECT_NAME_COLLISION,
    // it means the particular short name is already in use in that
    // directory.  but we're not going to fail the request over this
    //

    if (NT_SUCCESS(status) &&
            info.dir.ShortNameLength &&
            info.dir.ShortNameLength <= 12 * sizeof(WCHAR)) {

        FILE_SET_SHORT_NAME_REQ *req;
        MSG_HEADER *rpl;

        ULONG CopyPath_len = (wcslen(CopyPath) + 1) * sizeof(WCHAR);
        ULONG req_len = sizeof(FILE_SET_SHORT_NAME_REQ) + CopyPath_len;

        req = (FILE_SET_SHORT_NAME_REQ *)Dll_AllocTemp(req_len);
        if (req) {

            req->h.length = req_len;
            req->h.msgid = MSGID_FILE_SET_SHORT_NAME;

            memzero(&req->info, sizeof(req->info));
            req->info.FileNameLength = info.dir.ShortNameLength;
            memcpy(req->info.FileName, info.dir.ShortName,
                   req->info.FileNameLength);

            req->path_len = CopyPath_len;
            wcscpy(req->path, CopyPath);

            rpl = SbieDll_CallServer(&req->h);
            if (rpl)
                Dll_Free(rpl);

            Dll_Free(req);
        }
    }

    //
    // for a directory file, also copy files times
    //

    if (NT_SUCCESS(status) &&
            info.dir.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

        FILE_SET_ATTRIBUTES_REQ *req;
        MSG_HEADER *rpl;

        ULONG CopyPath_len = (wcslen(CopyPath) + 1) * sizeof(WCHAR);
        ULONG req_len = sizeof(FILE_SET_ATTRIBUTES_REQ) + CopyPath_len;

        req = (FILE_SET_ATTRIBUTES_REQ *)Dll_AllocTemp(req_len);
        if (req) {

            req->h.length = req_len;
            req->h.msgid = MSGID_FILE_SET_ATTRIBUTES;

            memzero(&req->info, sizeof(req->info));
            req->info.CreationTime.QuadPart   = info.dir.CreationTime.QuadPart;
            req->info.LastAccessTime.QuadPart = info.dir.LastAccessTime.QuadPart;
            req->info.LastWriteTime.QuadPart  = info.dir.LastWriteTime.QuadPart;
            req->info.ChangeTime.QuadPart  = info.dir.ChangeTime.QuadPart;

            req->path_len = CopyPath_len;
            wcscpy(req->path, CopyPath);

            rpl = SbieDll_CallServer(&req->h);
            if (rpl)
                Dll_Free(rpl);

            Dll_Free(req);
        }
    }

    return status;
}


//---------------------------------------------------------------------------
// File_AdjustShortName
//---------------------------------------------------------------------------


_FX BOOLEAN File_AdjustShortName(
    const WCHAR *TruePath, const WCHAR *CopyPath, HANDLE FileHandle)
{
    NTSTATUS status;
    HANDLE handle;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    IO_STATUS_BLOCK IoStatusBlock;
    union {
        FILE_BOTH_DIRECTORY_INFORMATION dir;
        UCHAR space[128];
    } info;
    WCHAR save_char;
    WCHAR *backslash;

    //
    // open the directory containing the last element of CopyPath.
    // keep the trailing backslash, in case we are opening the
    // root directory of the volume
    //

    backslash = wcsrchr(CopyPath, L'\\');
    if ((! backslash) || (! backslash[1]))
        return FALSE;

    save_char = backslash[1];
    backslash[1] = L'\0';

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    RtlInitUnicodeString(&objname, CopyPath);

    status = __sys_NtCreateFile(
        &handle, FILE_GENERIC_READ, &objattrs, &IoStatusBlock, NULL,
        0, FILE_SHARE_VALID_FLAGS, FILE_OPEN,
        FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

    if (status == STATUS_NOT_A_DIRECTORY) {

        status = __sys_NtCreateFile(
            &handle, FILE_GENERIC_READ, &objattrs, &IoStatusBlock, NULL,
            0, FILE_SHARE_VALID_FLAGS, FILE_OPEN,
            FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
    }

    backslash[1] = save_char;

    if (! NT_SUCCESS(status))
        return FALSE;

    //
    // query the short name
    //

    RtlInitUnicodeString(&objname, backslash + 1);

    info.dir.NextEntryOffset = tzuk;

    status = __sys_NtQueryDirectoryFile(
        handle, NULL, NULL, NULL, &IoStatusBlock,
        &info.dir, sizeof(info), FileBothDirectoryInformation,
        TRUE, &objname, FALSE);

    if (status == STATUS_BUFFER_OVERFLOW) {
        status = STATUS_SUCCESS;

        //
        // although STATUS_BUFFER_OVERFLOW should fill as much of the buffer
        // as we gave it, some faulty drivers (e.g. avast! pro) may return
        // this status without touching the buffer at all, in this case retry
        // with a larger buffer
        //

        if (info.dir.NextEntryOffset == tzuk) {

            FILE_BOTH_DIRECTORY_INFORMATION *dir2 = Dll_AllocTemp(1024);

            status = __sys_NtQueryDirectoryFile(
                handle, NULL, NULL, NULL, &IoStatusBlock,
                dir2, 1024, FileBothDirectoryInformation,
                TRUE, &objname, FALSE);

            memcpy(&info.dir, dir2, sizeof(info));

            Dll_Free(dir2);

            if (status != STATUS_SUCCESS)
                return FALSE;
        }
    }

    NtClose(handle);

    if (! NT_SUCCESS(status))
        return FALSE;

    if (! info.dir.ShortNameLength)
        return FALSE;

    //
    // now remove the last element of TruePath and append the short path
    //

    backslash = wcsrchr(TruePath, L'\\');
    if ((! backslash) || (! backslash[1]))
        return STATUS_SUCCESS;

    ++backslash;

    memcpy(backslash, info.dir.ShortName, info.dir.ShortNameLength);
    backslash[info.dir.ShortNameLength / sizeof(WCHAR)] = L'\0';

    //
    // try to open this new combined TruePath
    //

    RtlInitUnicodeString(&objname, TruePath);

    status = __sys_NtCreateFile(
        &handle, FILE_GENERIC_READ, &objattrs, &IoStatusBlock, NULL,
        0, FILE_SHARE_VALID_FLAGS, FILE_OPEN,
        FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

    //
    // if we could open a file then it means the newly created copy file
    // has the same short name as some true file in the same directory.
    // we must generate a random short name instead
    //

    if (NT_SUCCESS(status)) {

        ULONG ticks = GetTickCount();

        FILE_SET_SHORT_NAME_REQ *req;
        MSG_HEADER *rpl;

        ULONG CopyPath_len = (wcslen(CopyPath) + 1) * sizeof(WCHAR);
        ULONG req_len = sizeof(FILE_SET_SHORT_NAME_REQ) + CopyPath_len;

        NtClose(handle);
        NtClose(FileHandle);

        req = (FILE_SET_SHORT_NAME_REQ *)Dll_AllocTemp(req_len);
        if (req) {

            req->h.length = req_len;
            req->h.msgid = MSGID_FILE_SET_SHORT_NAME;

            memzero(&req->info, sizeof(req->info));
            Sbie_snwprintf(req->info.FileName, 12,
                     L"SB~%05X.%03X", ticks >> 12, ticks & 0xFFF);
            req->info.FileNameLength = (8 + 1 + 3) * sizeof(WCHAR);

            req->path_len = CopyPath_len;
            wcscpy(req->path, CopyPath);

            rpl = SbieDll_CallServer(&req->h);
            if (rpl)
                Dll_Free(rpl);

            Dll_Free(req);
        }

        return TRUE;
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// File_SetCreateTime
//---------------------------------------------------------------------------


_FX NTSTATUS File_SetCreateTime(HANDLE FileHandle, const WCHAR *CopyPath)
{
    NTSTATUS status;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_BASIC_INFORMATION info;

    status = __sys_NtQueryInformationFile(
        FileHandle, &IoStatusBlock, &info,
        sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);

    if (NT_SUCCESS(status)) {

        if (IS_DELETE_MARK(&info.CreationTime)) {

            FILETIME ft;
            GetSystemTimeAsFileTime(&ft);

            info.CreationTime.HighPart = ft.dwHighDateTime;
            info.CreationTime.LowPart  = ft.dwLowDateTime;

            status = File_SetAttributes(FileHandle, CopyPath, &info);
        }
    }

    return status;
}


//---------------------------------------------------------------------------
// File_MarkDeleted
//---------------------------------------------------------------------------


_FX NTSTATUS File_MarkDeleted(HANDLE FileHandle, const WCHAR *CopyPath)
{
    NTSTATUS status;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_BASIC_INFORMATION info;

    //
    // get the type of the file
    //

    status = __sys_NtQueryInformationFile(
        FileHandle, &IoStatusBlock,
        &info, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);

    if (! NT_SUCCESS(status))
        return status;

    //
    // if we are trying to mark a directory as deleted, it must not
    // have any children.  we use our NtQueryDirectoryFile to merge
    // true/copy directories and hide files already marked deleted.
    //
    // we pass FilePath rather than FileHandle, because the handle
    // may have been opened for asynchronous access.
    //

    if (info.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

        status = File_DeleteDirectory(CopyPath, FALSE);

        if (status == STATUS_DELETE_PENDING)
            status = STATUS_SUCCESS;

        if (! NT_SUCCESS(status))
            return status;
    }

    //
    // mark the file deleted
    //

    info.CreationTime.HighPart = DELETE_MARK_HIGH;
    info.CreationTime.LowPart  = DELETE_MARK_LOW;

    status = File_SetAttributes(FileHandle, CopyPath, &info);

    if (status == STATUS_DELETE_PENDING)
        status = STATUS_SUCCESS;

    return status;
}


//---------------------------------------------------------------------------
// File_QueryFullAttributesDirectoryFile
//---------------------------------------------------------------------------


_FX NTSTATUS File_QueryFullAttributesDirectoryFile(
    const WCHAR *TruePath, FILE_NETWORK_OPEN_INFORMATION *FileInformation)
{
    IO_STATUS_BLOCK MyIoStatusBlock;
    HANDLE FileHandle;
    NTSTATUS status;

    //
    // try to use SbieApi_OpenFile which will open the file, bypassing
    // ClosedFilePath settings, but only if it a directory file.  it
    // returns a handle that can only be used to query file attributes
    //

    status = SbieApi_OpenFile(&FileHandle, TruePath);
    if (NT_SUCCESS(status)) {

        status = __sys_NtQueryInformationFile(
            FileHandle, &MyIoStatusBlock,
            FileInformation, sizeof(FILE_BASIC_INFORMATION),
            FileBasicInformation);

        NtClose(FileHandle);

        if (NT_SUCCESS(status)) {

            //
            // convert FILE_BASIC_INFORMATION
            // into FILE_NETWORK_OPEN_INFORMATION
            //

            const FILE_BASIC_INFORMATION *BasicInfo =
                (FILE_BASIC_INFORMATION *)FileInformation;
            ULONG FileAttrs = BasicInfo->FileAttributes;

            FileInformation->AllocationSize.QuadPart = 0;
            FileInformation->EndOfFile.QuadPart = 0;
            FileInformation->FileAttributes = FileAttrs;
        }
    }

    return status;
}


//---------------------------------------------------------------------------
// File_CheckDepthForIsWritePath
//---------------------------------------------------------------------------


_FX ULONG File_CheckDepthForIsWritePath(const WCHAR *TruePath)
{
    ULONG FileFlags, mp_flags, len;
    WCHAR *copy, *ptr;

    //
    // given a path that matches a write-only setting, this function
    // removes the last path component in each iteration to find out
    // how deep the input path is relative to the write-only setting
    //

    len = wcslen(TruePath);
    if (! len)
        return 0;
    copy = Dll_AllocTemp((len + 1) * sizeof(WCHAR));
    wmemcpy(copy, TruePath, len + 1);

    while (copy[len - 1] == L'\\') {
        --len;
        copy[len] = L'\0';
    }

    FileFlags = 0;
    len = 0;

    while (1) {

        ptr = wcsrchr(copy, L'\\');
        if (! ptr)
            break;
        *ptr = L'\0';

        mp_flags = File_MatchPath(copy, &FileFlags);
        if (PATH_NOT_WRITE(mp_flags))
            break;

        ++len;
    }

    Dll_Free(copy);

    return len;
}


//---------------------------------------------------------------------------
// File_NtQueryAttributesFile
//---------------------------------------------------------------------------


_FX NTSTATUS File_NtQueryAttributesFile(
    OBJECT_ATTRIBUTES *ObjectAttributes,
    FILE_BASIC_INFORMATION *FileInformation)
{
    NTSTATUS status;
    FILE_NETWORK_OPEN_INFORMATION info;

    //
    // NtQueryAttributesFile doesn't always return the correct CreationTime
    // timestamp, so use NtQueryFullAttributesFile.  it also saves the
    // trouble of having to duplicate the logic used in that other function
    //

    status = File_NtQueryFullAttributesFileImpl(ObjectAttributes, &info);

    if (NT_SUCCESS(status)) {

        FILE_NETWORK_OPEN_INFORMATION *src = &info;
        FILE_BASIC_INFORMATION        *dst = FileInformation;

        dst->CreationTime.QuadPart      = src->CreationTime.QuadPart;
        dst->LastAccessTime.QuadPart    = src->LastAccessTime.QuadPart;
        dst->LastWriteTime.QuadPart     = src->LastWriteTime.QuadPart;
        dst->ChangeTime.QuadPart        = src->ChangeTime.QuadPart;
        dst->FileAttributes             = src->FileAttributes;
    }

    status = StopTailCallOptimization(status);

    return status;
}


//---------------------------------------------------------------------------
// File_NtQueryFullAttributesFile
//---------------------------------------------------------------------------


_FX NTSTATUS File_NtQueryFullAttributesFile(
    OBJECT_ATTRIBUTES *ObjectAttributes,
    FILE_NETWORK_OPEN_INFORMATION *FileInformation)
{
    NTSTATUS status = File_NtQueryFullAttributesFileImpl(ObjectAttributes, FileInformation);

    if (status == STATUS_OBJECT_NAME_NOT_FOUND && Dll_ImageType == DLL_IMAGE_MSI_INSTALLER
        && ObjectAttributes != NULL && ObjectAttributes->ObjectName != NULL 
        // ObjectAttributes->ObjectName == "\\??\\C:\\Config.Msi" // or any other system drive
        && ObjectAttributes->ObjectName->Buffer && ObjectAttributes->ObjectName->Length == 34
        && _wcsicmp(ObjectAttributes->ObjectName->Buffer + 6, L"\\Config.Msi") == 0
        ) {

        //
        // MSI bug: this must not fail, hence we create the directory and retry
        //

        CreateDirectory(ObjectAttributes->ObjectName->Buffer, NULL);

        status = File_NtQueryFullAttributesFileImpl(ObjectAttributes, FileInformation);
    }

    status = StopTailCallOptimization(status);

    return status;
}


//---------------------------------------------------------------------------
// File_NtQueryFullAttributesFileImpl
//---------------------------------------------------------------------------


_FX NTSTATUS File_NtQueryFullAttributesFileImpl(
    OBJECT_ATTRIBUTES *ObjectAttributes,
    FILE_NETWORK_OPEN_INFORMATION *FileInformation)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status, status2;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    ULONG FileFlags, FileAttrs, mp_flags;
    ULONG TruePathFlags;
    WCHAR* OriginalPath;

    //
    // special case:  when it starts, the Windows Explorer process looks
    // for Autorun.inf files on all drives (including removable) and that's
    // just too slow
    //

    if (Dll_ImageType == DLL_IMAGE_SHELL_EXPLORER) {

        if (ObjectAttributes && ObjectAttributes->ObjectName &&
            ObjectAttributes->ObjectName->Length == 36 &&
            ObjectAttributes->ObjectName->Buffer &&
            _wcsicmp(ObjectAttributes->ObjectName->Buffer + 7,
                                                    L"autorun.inf") == 0) {

            return STATUS_OBJECT_NAME_NOT_FOUND;
        }
    }

    //
    // otherwise continue with normal processing
    //

    FileAttrs = -1;

    Dll_PushTlsNameBuffer(TlsData);

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    __try {

    //
    // get the file name we're trying to open
    //

    status = File_GetName(
        ObjectAttributes->RootDirectory, ObjectAttributes->ObjectName,
        &TruePath, &CopyPath, &FileFlags);

    if (Dll_ApiTrace || Dll_FileTrace) {
        WCHAR trace_str[2048];
        ULONG len = Sbie_snwprintf(trace_str, 2048, L"File_NtQueryFullAttributesFileImpl %s", TruePath);
        SbieApi_MonitorPut2Ex(MONITOR_APICALL | MONITOR_TRACE, len, trace_str, FALSE, FALSE);
    }

    if (! NT_SUCCESS(status)) {

        if (status == STATUS_BAD_INITIAL_PC) {

            //
            // if we get STATUS_BAD_INITIAL_PC here, this is most likely
            // an attempt to query the attributes of the root directory,
            // so we can do it on the true path
            //

            wcscat(TruePath, L"\\");
            RtlInitUnicodeString(&objname, TruePath);

            status = __sys_NtQueryFullAttributesFile(
                &objattrs, FileInformation);
        }

        __leave;
    }

    //
    // check if this is a closed path
    //

    mp_flags = File_MatchPath(TruePath, &FileFlags);

    if (PATH_IS_CLOSED(mp_flags)) {

        //
        // query the attributes of a directory file, bypassing
        // ClosedFilePath (see File_QueryFullAttributesDirectoryFile)
        //
        // for non-directory files, abort with status access denied
        //

        status = File_QueryFullAttributesDirectoryFile(
                                            TruePath, FileInformation);

        if (! NT_SUCCESS(status))
            status = STATUS_ACCESS_DENIED;

        __leave;
    }

    //
    // check if this is an open path
    //

    if (PATH_IS_OPEN(mp_flags)) {

        RtlInitUnicodeString(&objname, TruePath);

        status = __sys_NtQueryFullAttributesFile(&objattrs, FileInformation);

        if (NT_SUCCESS(status))
            FileAttrs = FileInformation->FileAttributes;

        __leave;
    }

    //
    // try NtQueryFullAttributesFile on the CopyPath first
    //

    if (!File_Delete_v2)
    if (File_CheckDeletedParent(CopyPath)) {
        status = STATUS_OBJECT_PATH_NOT_FOUND;
        __leave;
    }

    RtlInitUnicodeString(&objname, CopyPath);

    status = __sys_NtQueryFullAttributesFile(&objattrs, FileInformation);

    if (NT_SUCCESS(status) || (
            status != STATUS_OBJECT_NAME_NOT_FOUND &&
            status != STATUS_OBJECT_PATH_NOT_FOUND)) {

        if (!File_Delete_v2) {

            if (NT_SUCCESS(status) &&
                IS_DELETE_MARK(&FileInformation->CreationTime))
                status = STATUS_OBJECT_NAME_NOT_FOUND;
        }

        if (NT_SUCCESS(status))
            FileAttrs = FileInformation->FileAttributes;

        __leave;
    }

    //
    // Check true path relocation
    //

    OriginalPath = NULL;
    WCHAR* OldTruePath = File_ResolveTruePath(TruePath, CopyPath, &TruePathFlags);
    if (OldTruePath) {
        OriginalPath = TruePath;
        TruePath = OldTruePath;
    }

    //
    // check if this is a write-only path.  if the path is not
    // the highest level match on the write-only setting, we
    // pretend the path does not exist; see also NtCreateFile
    //

    if (PATH_IS_WRITE(mp_flags)) {

        BOOLEAN use_rule_specificity = (Dll_ProcessFlags & SBIE_FLAG_RULE_SPECIFICITY) != 0;

        if (use_rule_specificity && SbieDll_HasReadableSubPath(L'f', OriginalPath ? OriginalPath : TruePath)){

            //
            // When using Rule specificity we need to create some dummy directories 
            //

            File_CreateBoxedPath(OriginalPath ? OriginalPath : TruePath);
        }
        else if (OriginalPath) {
            ; // try TruePath which points by now to the snapshot location
        }
        else {

            int depth = File_CheckDepthForIsWritePath(TruePath);
            if (depth == 0) {
                status = File_QueryFullAttributesDirectoryFile(
                    TruePath, FileInformation);
                if (status == STATUS_NOT_A_DIRECTORY)
                    status = STATUS_OBJECT_NAME_NOT_FOUND;
            }
            else if (depth == 1)
                status = STATUS_OBJECT_NAME_NOT_FOUND;
            else {
                // if depth > 1 we leave the status from querying
                // the copy path, which would be
                // - STATUS_OBJECT_NAME_NOT_FOUND if copy parent exists
                // - STATUS_OBJECT_PATH_NOT_FOUND if it does not exist
                //
            }

            if (NT_SUCCESS(status))
                FileAttrs = FileInformation->FileAttributes;

            __leave;
        }
    }

    //
    // if we couldn't find CopyPath, or if it's an open path,
    // then try on the TruePath
    //

    RtlInitUnicodeString(&objname, TruePath);

    status2 = __sys_NtQueryFullAttributesFile(&objattrs, FileInformation);

    if (TruePathFlags && NT_SUCCESS(status2)) {

        //
        // if we found only the true file check if its listed as deleted
        //

        if (FILE_PARENT_DELETED(TruePathFlags)) { // parent deleted
            status = STATUS_OBJECT_PATH_NOT_FOUND;
            __leave;
        } else if (FILE_IS_DELETED(TruePathFlags)) { // path deleted
            status = STATUS_OBJECT_NAME_NOT_FOUND;
            __leave;
        }
    }

    if (status2 != STATUS_OBJECT_PATH_NOT_FOUND) {

        status = status2;

        if (NT_SUCCESS(status))
            FileAttrs = FileInformation->FileAttributes;
    }

    //
    // finish
    //

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    if (FileAttrs != -1                                 // if successful
        && (FileAttrs & FILE_ATTRIBUTE_DIRECTORY) == 0  // and not directory
        && (FileFlags & FGN_TRAILING_BACKSLASH)) {      // but trailing b.s

        status = STATUS_OBJECT_NAME_INVALID;
    }

    if (Dll_ApiTrace || Dll_FileTrace) {
        WCHAR trace_str[2048];
        ULONG len = Sbie_snwprintf(trace_str, 2048, L"File_NtQueryFullAttributesFileImpl status = 0x%08X", status);
        SbieApi_MonitorPut2Ex(MONITOR_APICALL | MONITOR_TRACE, len, trace_str, FALSE, FALSE);
    }

    Dll_PopTlsNameBuffer(TlsData);
    SetLastError(LastError);
    return status;
}


//---------------------------------------------------------------------------
// File_NtQueryInformationFile
//---------------------------------------------------------------------------


_FX NTSTATUS File_NtQueryInformationFile(
    HANDLE FileHandle,
    IO_STATUS_BLOCK *IoStatusBlock,
    void *FileInformation,
    ULONG Length,
    FILE_INFORMATION_CLASS FileInformationClass)
{
    NTSTATUS status;
    ULONG LastError;
    THREAD_DATA *TlsData;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    ULONG TruePathLen;
    FILE_LINK *file_link;

    if (FileInformationClass == FileNetworkPhysicalNameInformation) {

        // To support DFS
        File_GetName(FileHandle, NULL, &TruePath, &CopyPath, NULL);
        if (TruePath)
        {
            NTSTATUS status2;
            HANDLE FileHandleTrue = 0;
            OBJECT_ATTRIBUTES objattrs;
            UNICODE_STRING objname;
            IO_STATUS_BLOCK IoStatusBlock2;
            InitializeObjectAttributes(
                &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

            RtlInitUnicodeString(&objname, TruePath);

            status2 = __sys_NtCreateFile(
                &FileHandleTrue, FILE_READ_ATTRIBUTES | SYNCHRONIZE, &objattrs,
                &IoStatusBlock2, NULL, 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 
                FILE_OPEN, FILE_OPEN_FOR_BACKUP_INTENT|FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

            if(NT_SUCCESS(status2))
            {
                status = __sys_NtQueryInformationFile(
                    FileHandleTrue, IoStatusBlock, FileInformation,
                    Length, FileInformationClass);
                NtClose(FileHandleTrue);

                return status;
            }
        }
    }

    //
    // we only handle FileNameInformation here
    //

    else if (FileInformationClass != FileNameInformation) {

        LARGE_INTEGER *FileId = NULL;

        status = __sys_NtQueryInformationFile(
            FileHandle, IoStatusBlock, FileInformation,
            Length, FileInformationClass);

        //
        // if caller queried the FileId, and the file is in the
        // sandbox, then scramble the FileId to make it less likely
        // that the file can be opened by it without unscrambling
        // (see also File_GetName_FromFileId)
        //
        // the reason for this is the possibly of files on both C:
        // and D: drives having the same FileId.  the program may
        // wish to open use a handle on drive C: to open using the
        // FileId by might end up using a sandbox handle like
        // D:\sandbox\drive\C which is actually on drive D:.  this
        // makes it impossible to figure out if the program wants
        // the file on C: or the sandboxed file on D:.  to make
        // this less likely to be a problem, we scrambe the FileId
        // for files in the sandbox.  see also NtQueryDirectoryFile
        // and File_GetFullInformation
        //

        if (FileInformationClass == FileInternalInformation &&
                NT_SUCCESS(status)) {

            FileId = &((FILE_INTERNAL_INFORMATION *)FileInformation)
                            ->IndexNumber;

        } else if (FileInformationClass == FileAllInformation &&
            (NT_SUCCESS(status) || status == STATUS_BUFFER_OVERFLOW)) {

            FileId = &(((FILE_ALL_INFORMATION *)FileInformation)->
                            InternalInformation.IndexNumber);
        }

        if (FileId && FileId->QuadPart) {

            BOOLEAN IsBoxedPath;
            NTSTATUS status2 =
                SbieDll_GetHandlePath(FileHandle, NULL, &IsBoxedPath);
            if (IsBoxedPath && (NT_SUCCESS(status2)
                                    || (status2 == STATUS_BAD_INITIAL_PC))) {

                FileId->LowPart  ^= 0xFFFFFFFF;
                FileId->HighPart ^= 0xFFFFFFFF;
            }
        }

        return status;
    }

    //
    // validate buffer length
    //

    if (Length < sizeof(ULONG) * 4)
        return STATUS_INFO_LENGTH_MISMATCH;

    //
    // get the true path for the handle passed, and copy into buffer
    //

    TlsData = Dll_GetTlsData(&LastError);
    Dll_PushTlsNameBuffer(TlsData);

    __try {

    status = File_GetName(FileHandle, NULL, &TruePath, &CopyPath, NULL);
    if (status == STATUS_BAD_INITIAL_PC)
        status = STATUS_SUCCESS;
    if (! NT_SUCCESS(status))
        __leave;

    file_link = File_FindPermLinksForMatchPath(TruePath, wcslen(TruePath));
    if (file_link) {

        //
        // File_GetName may translate a path to a volume that is mounted
        // without a drive letter, for example \Device\HarddiskVolume2\XXX
        // translates to \Device\HarddiskVolume1\MOUNT\XXX, and we want
        // to make sure that we return \XXX rather than \MOUNT\XXX
        //

        TruePath += file_link->dst_len;
        TruePathLen = wcslen(TruePath);

        LeaveCriticalSection(File_DrivesAndLinks_CritSec);

    } else if (_wcsnicmp(TruePath, File_Mup, File_MupLen) == 0) {

        //
        // the TruePath may refer to a network device
        //

        TruePath += File_MupLen - 1;
        TruePathLen = wcslen(TruePath);

    } else {

        //
        // secondary check for a network drive
        //

        WCHAR *NetworkPath = NULL;
        ULONG dummy_len;
        const FILE_DRIVE *file_drive =
            File_GetDriveForUncPath(TruePath, wcslen(TruePath), &dummy_len);

        if (file_drive) {

            LeaveCriticalSection(File_DrivesAndLinks_CritSec);

            NetworkPath = wcschr(TruePath + 8, L'\\');
            if (NetworkPath) {

                TruePathLen = wcslen(NetworkPath);
                wmemmove(TruePath, NetworkPath, TruePathLen + 1);
            }
        }

        if (! NetworkPath) {

            //
            // otherwise we do normal drive letter processing
            //

            if (SbieDll_TranslateNtToDosPath(TruePath)) {
                TruePathLen = wcslen(TruePath);
                if (TruePathLen >= 2 && TruePath[1] == L':') {
                    if (TruePathLen == 2)
                        TruePathLen = 0;
                    else {
                        TruePath += 2;
                        TruePathLen -= 2;
                    }
                }
            }
            else { // todo: fix-me this is not elegant
                TruePathLen = wcslen(TruePath);
                const FILE_GUID* guid = File_GetGuidForPath(TruePath, TruePathLen);
                if (guid) {
                    TruePath += guid->len;
                    TruePathLen -= guid->len;
                    LeaveCriticalSection(File_DrivesAndLinks_CritSec);
                }
            }
        }
    }

    if (TruePathLen == 0) {
        TruePath[0] = L'\\';
        TruePath[1] = L'\0';
        TruePathLen = 1;
    }

    TruePathLen *= sizeof(WCHAR);
    *(ULONG *)FileInformation = TruePathLen;
    Length -= sizeof(ULONG);
    if (Length > TruePathLen) {
        Length = TruePathLen;
        status = STATUS_SUCCESS;
    } else
        status = STATUS_BUFFER_OVERFLOW;
    memcpy((ULONG *)FileInformation + 1, TruePath, Length);

    IoStatusBlock->Status = status;
    IoStatusBlock->Information = sizeof(ULONG) + Length;

    //
    // finish
    //

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    Dll_PopTlsNameBuffer(TlsData);
    SetLastError(LastError);
    return status;
}


//---------------------------------------------------------------------------
// File_NtQueryInformationByName
//---------------------------------------------------------------------------


_FX NTSTATUS File_NtQueryInformationByName(
    POBJECT_ATTRIBUTES ObjectAttributes,
    PIO_STATUS_BLOCK IoStatusBlock,
    PVOID FileInformation,
    ULONG Length,
    FILE_INFORMATION_CLASS FileInformationClass
)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status, status2;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    ULONG FileFlags, mp_flags;
    ULONG TruePathFlags;
    WCHAR* OriginalPath;

    Dll_PushTlsNameBuffer(TlsData);

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    __try {

        //
        // get the file name we're trying to open
        //

        status = File_GetName(
            ObjectAttributes->RootDirectory, ObjectAttributes->ObjectName,
            &TruePath, &CopyPath, &FileFlags);

        if (Dll_ApiTrace || Dll_FileTrace) {
            WCHAR trace_str[2048];
            ULONG len = Sbie_snwprintf(trace_str, 2048, L"File_NtQueryInformationByName %s", TruePath);
            SbieApi_MonitorPut2Ex(MONITOR_APICALL | MONITOR_TRACE, len, trace_str, FALSE, FALSE);
        }

        if (! NT_SUCCESS(status)) {

            if (status == STATUS_BAD_INITIAL_PC) {

                //
                // if we get STATUS_BAD_INITIAL_PC here, this is most likely
                // an attempt to query the attributes of the root directory,
                // so we can do it on the true path
                //

                wcscat(TruePath, L"\\");
                RtlInitUnicodeString(&objname, TruePath);

                status = __sys_NtQueryInformationByName(
                    &objattrs, IoStatusBlock, FileInformation,
                    Length, FileInformationClass);
            }

            __leave;
        }

        //
        // check if this is a closed path
        //

        mp_flags = File_MatchPath(TruePath, &FileFlags);

        if (PATH_IS_CLOSED(mp_flags)) {

            status = STATUS_ACCESS_DENIED;

            __leave;
        }

        //
        // check if this is an open path
        //

        if (PATH_IS_OPEN(mp_flags)) {

            RtlInitUnicodeString(&objname, TruePath);

            status = __sys_NtQueryInformationByName(
                &objattrs, IoStatusBlock, FileInformation,
                Length, FileInformationClass);

            __leave;
        }

        //
        // try NtQueryInformationByName on the CopyPath first
        //

        if (!File_Delete_v2)
            if (File_CheckDeletedParent(CopyPath)) {
                status = STATUS_OBJECT_PATH_NOT_FOUND;
                __leave;
            }

        RtlInitUnicodeString(&objname, CopyPath);

        status = __sys_NtQueryInformationByName(
            &objattrs, IoStatusBlock, FileInformation,
            Length, FileInformationClass);

        if (NT_SUCCESS(status) || (
            status != STATUS_OBJECT_NAME_NOT_FOUND &&
            status != STATUS_OBJECT_PATH_NOT_FOUND)) {

            // todo
            /*if (!File_Delete_v2) {

                if (NT_SUCCESS(status) &&
                    IS_DELETE_MARK(&FileInformation->CreationTime))
                    status = STATUS_OBJECT_NAME_NOT_FOUND;
            }*/

            __leave;
        }

        //
        // Check true path relocation
        //

        OriginalPath = NULL;
        WCHAR* OldTruePath = File_ResolveTruePath(TruePath, CopyPath, &TruePathFlags);
        if (OldTruePath) {
            OriginalPath = TruePath;
            TruePath = OldTruePath;
        }

        //
        // check if this is a write-only path.  if the path is not
        // the highest level match on the write-only setting, we
        // pretend the path does not exist; see also NtCreateFile
        //

        if (PATH_IS_WRITE(mp_flags)) {

            BOOLEAN use_rule_specificity = (Dll_ProcessFlags & SBIE_FLAG_RULE_SPECIFICITY) != 0;

            if (use_rule_specificity && SbieDll_HasReadableSubPath(L'f', OriginalPath ? OriginalPath : TruePath)){

                //
                // When using Rule specificity we need to create some dummy directories 
                //

                File_CreateBoxedPath(OriginalPath ? OriginalPath : TruePath);
            }
            else if (OriginalPath) {
                ; // try TruePath which points by now to the snapshot location
            }
            else {

                int depth = File_CheckDepthForIsWritePath(TruePath);
                if (depth == 0) {

                    RtlInitUnicodeString(&objname, TruePath);

                    status = __sys_NtQueryInformationByName(
                        &objattrs, IoStatusBlock, FileInformation,
                        Length, FileInformationClass);
                }
                else if (depth == 1)
                    status = STATUS_OBJECT_NAME_NOT_FOUND;
                else {
                    // if depth > 1 we leave the status from querying
                    // the copy path, which would be
                    // - STATUS_OBJECT_NAME_NOT_FOUND if copy parent exists
                    // - STATUS_OBJECT_PATH_NOT_FOUND if it does not exist
                    //
                }

                __leave;
            }
        }

        //
        // if we couldn't find CopyPath, or if it's an open path,
        // then try on the TruePath
        //

        RtlInitUnicodeString(&objname, TruePath);

        status2 = __sys_NtQueryInformationByName(
            &objattrs, IoStatusBlock, FileInformation,
            Length, FileInformationClass);

        if (TruePathFlags && NT_SUCCESS(status2)) {

            //
            // if we found only the true file check if its listed as deleted
            //

            if (FILE_PARENT_DELETED(TruePathFlags)) { // parent deleted
                status = STATUS_OBJECT_PATH_NOT_FOUND;
                __leave;
            } else if (FILE_IS_DELETED(TruePathFlags)) { // path deleted
                status = STATUS_OBJECT_NAME_NOT_FOUND;
                __leave;
            }
        }

        if (status2 != STATUS_OBJECT_PATH_NOT_FOUND) {

            status = status2;
        }

        //
        // finish
        //

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    if (Dll_ApiTrace || Dll_FileTrace) {
        WCHAR trace_str[2048];
        ULONG len = Sbie_snwprintf(trace_str, 2048, L"File_NtQueryInformationByName status = 0x%08X", status);
        SbieApi_MonitorPut2Ex(MONITOR_APICALL | MONITOR_TRACE, len, trace_str, FALSE, FALSE);
    }

    Dll_PopTlsNameBuffer(TlsData);
    SetLastError(LastError);
    return status;

    return status;
}


//---------------------------------------------------------------------------
// File_GetFinalPathNameByHandleW
//---------------------------------------------------------------------------


_FX ULONG File_GetFinalPathNameByHandleW(
    HANDLE hFile, WCHAR *lpszFilePath, ULONG cchFilePath, ULONG dwFlags)
{
    NTSTATUS status;
    ULONG rc;
    ULONG err;
    WCHAR *path, *result;
    BOOLEAN IsBoxedPath;

    status = SbieDll_GetHandlePath(hFile, NULL, &IsBoxedPath);
    if (IsBoxedPath &&
            (NT_SUCCESS(status) || (status == STATUS_BAD_INITIAL_PC))) {

        //
        // the specified file is inside the sandbox, so handle the request
        //
        
        path = Dll_AllocTemp(8192);
        status = SbieDll_GetHandlePath(hFile, path, NULL);
        if (! NT_SUCCESS(status)) {

            rc = 0;
            err = ERROR_PATH_NOT_FOUND;

        } else {

            result = File_GetFinalPathNameByHandleW_2(path, dwFlags);
            if (! result) {

                rc = 0;
                err = GetLastError();

            } else {

                ULONG len = wcslen(result);
                if (len >= cchFilePath) {

                    rc = len + 1;
                    err = ERROR_NOT_ENOUGH_MEMORY;

                } else {

                    wmemcpy(lpszFilePath, result, len + 1);
                    rc = len;
                    err = 0;
                }

                if (result != path)
                    Dll_Free(result);
            }
        }

        Dll_Free(path);
    }

    //
    // the file is outside the sandbox, so the system can handle it
    //

    if (! IsBoxedPath) {

        rc = __sys_GetFinalPathNameByHandleW(
                    hFile, lpszFilePath, cchFilePath, dwFlags);
        err = GetLastError();
    }

    if (Dll_ApiTrace || Dll_FileTrace) {
        WCHAR trace_str[2048];
        ULONG len = Sbie_snwprintf(trace_str, 2048, L"File_GetFinalPathNameByHandleW %s", lpszFilePath);
        SbieApi_MonitorPut2Ex(MONITOR_APICALL | MONITOR_TRACE, len, trace_str, FALSE, FALSE);
    }

    SetLastError(err);
    return rc;
}


//---------------------------------------------------------------------------
// File_GetFinalPathNameByHandleW_2
//---------------------------------------------------------------------------


_FX WCHAR *File_GetFinalPathNameByHandleW_2(WCHAR *TruePath, ULONG dwFlags)
{
    static const WCHAR *_DosPrefix = L"\\\\?\\UNC\\";
    const FILE_DRIVE *file_drive;
    const FILE_LINK *file_link;
    const WCHAR *suffix, *suffix2;
    WCHAR *path;
    WCHAR *ReparsedPath;
    ULONG TruePath_len;
    ULONG suffix_len, suffix2_len;
    WCHAR drive_letter;
    BOOLEAN AddBackslash;

    //
    // validate input flags
    //

    dwFlags &= VOLUME_NAME_GUID | VOLUME_NAME_NT | VOLUME_NAME_NONE;

    if (dwFlags & VOLUME_NAME_GUID) {

        if (dwFlags & (VOLUME_NAME_NT | VOLUME_NAME_NONE)) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return NULL;
        }

    } else if (dwFlags & VOLUME_NAME_NT) {

        if (dwFlags & (VOLUME_NAME_GUID | VOLUME_NAME_NONE)) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return NULL;
        }
    }

    TruePath_len = wcslen(TruePath);

    //
    // handle a network path
    //

    if (_wcsnicmp(TruePath, File_Mup, File_MupLen) == 0) {

        if (dwFlags & VOLUME_NAME_GUID) {
            //
            // VOLUME_NAME_GUID not supported for network shares
            //
            SetLastError(ERROR_PATH_NOT_FOUND);
            return NULL;
        }

        AddBackslash = FALSE;
        path = wcschr(TruePath + File_MupLen, L'\\');
        if (path) {
            path = wcschr(path + 1, L'\\');
            if (! path) {
                // if the path has just one backslash after the
                // \device\mup\ prefix then it is the root of the share,
                // for example \device\mup\server\share
                AddBackslash = TRUE;
            }
        }

        if (dwFlags & VOLUME_NAME_NT) {

            path = Dll_AllocTemp((TruePath_len + 4) * sizeof(WCHAR));
            wmemcpy(path, TruePath, TruePath_len + 1);
            path[1] = L'D';     // \Device\Mup rather than \device\mup
            path[8] = L'M';

        } else {

            suffix = TruePath + File_MupLen - 1;
            suffix_len = wcslen(suffix);

            path = Dll_AllocTemp((suffix_len + 12) * sizeof(WCHAR));

            if (dwFlags & VOLUME_NAME_NONE)
                wmemcpy(path, suffix, suffix_len + 1);
            else {
                wmemcpy(path, _DosPrefix, 8);
                wmemcpy(path + 8, suffix, suffix_len + 1);
            }
        }

        if (AddBackslash)
            wcscat(path, _DosPrefix + 7);

        return path;
    }

    //
    // handle the GUID case
    //

    if (dwFlags & VOLUME_NAME_GUID) {

        return File_GetFinalPathNameByHandleW_3(TruePath, TruePath_len);
    }

    //
    // analyse the path
    //

    ReparsedPath = NULL;
    AddBackslash = FALSE;
    drive_letter = 0;
    suffix2 = NULL;

    file_link = File_FindPermLinksForMatchPath(TruePath, TruePath_len);
    if (file_link) {

        //
        // if the volume is mounted on a directory then the TruePath here
        // will specify the NT path to the location of the mount, i.e.
        // \Device\HarddiskVolume1\MOUNT\XXX  instead of
        // \Device\HarddiskVolume2\XXX
        //
        // for non-DOS return values we need to convert the path back
        // to the form \Device\HarddiskVolume2\XXX.  for DOS return
        // values we use the drive letter of the mounted location
        //

        if (file_link->dst_len == TruePath_len)
            AddBackslash = TRUE;

        if (dwFlags != VOLUME_NAME_DOS) {

            //
            // for VOLUME_NAME_NT and VOLUME_NAME_NONE
            // we want the real device name even if it was mounted
            // on a directory and doesn't have a drive letter
            //

            ReparsedPath = File_FixPermLinksForMatchPath(TruePath);
            if (ReparsedPath) {

                TruePath = ReparsedPath;
                TruePath_len = wcslen(TruePath);

                suffix = TruePath + file_link->src_len;

            } else {
                // release lock by File_FindPermLinksForMatchPath
                LeaveCriticalSection(File_DrivesAndLinks_CritSec);
                SetLastError(ERROR_PATH_NOT_FOUND);
                return NULL;
            }

        } else {

            //
            // for VOLUME_NAME_DOS we want a path with a drive letter
            //

            file_drive = File_GetDriveForPath(TruePath, TruePath_len);
            if (! file_drive) {

                file_drive = File_GetDriveForPath(file_link->src, file_link->src_len);
                if (!file_drive) {

                    // release lock by File_FindPermLinksForMatchPath
                    LeaveCriticalSection(File_DrivesAndLinks_CritSec);
                    SetLastError(ERROR_PATH_NOT_FOUND);
                    return NULL;
                }
                else
                {
                    drive_letter = file_drive->letter;
                    suffix = file_link->src + file_drive->len;
                    suffix2 = TruePath + file_link->dst_len;

                    // release lock by File_GetDriveForPath
                    LeaveCriticalSection(File_DrivesAndLinks_CritSec);
                }
            }
            else
            {
                drive_letter = file_drive->letter;
                suffix = TruePath + file_drive->len;

                // release lock by File_GetDriveForPath
                LeaveCriticalSection(File_DrivesAndLinks_CritSec);
            }
        }

        // release lock by File_FindPermLinksForMatchPath
        LeaveCriticalSection(File_DrivesAndLinks_CritSec);

    } else {

        //
        // in the normal case, the volume is mounted on a drive letter
        //

        file_drive = File_GetDriveForPath(TruePath, TruePath_len);
        if (! file_drive) {
            SetLastError(ERROR_PATH_NOT_FOUND);
            return NULL;
        }

        if (file_drive->len == TruePath_len)
            AddBackslash = TRUE;

        drive_letter = file_drive->letter;
        suffix = TruePath + file_drive->len;

        // release lock by File_GetDriveForPath
        LeaveCriticalSection(File_DrivesAndLinks_CritSec);
    }

    //
    // build return path
    //

    if (dwFlags & VOLUME_NAME_NT) {

        path = Dll_AllocTemp((TruePath_len + 4) * sizeof(WCHAR));
        wmemcpy(path, TruePath, TruePath_len + 1);

    } else if (dwFlags & VOLUME_NAME_NONE) {

        suffix_len = wcslen(suffix);
        path = Dll_AllocTemp((suffix_len + 4) * sizeof(WCHAR));
        wmemcpy(path, suffix, suffix_len + 1);

    } else { // VOLUME_NAME_DOS

        suffix_len = wcslen(suffix);
        suffix2_len = suffix2 ? wcslen(suffix2) : 0;
        path = Dll_AllocTemp((suffix_len + suffix2_len + 16) * sizeof(WCHAR));
        wmemcpy(path, _DosPrefix, 4);
        path[4] = drive_letter;
        path[5] = L':';
        wmemcpy(path + 6, suffix, suffix_len + 1);
        if (suffix2)
            wcscat(path, suffix2);

    }

    if (AddBackslash)
        wcscat(path, _DosPrefix + 7);

    if (ReparsedPath)
        Dll_Free(ReparsedPath);

    return path;
}


//---------------------------------------------------------------------------
// File_GetFinalPathNameByHandleW_3
//---------------------------------------------------------------------------


_FX WCHAR *File_GetFinalPathNameByHandleW_3(
    WCHAR *TruePath, ULONG TruePath_len)
{
    const FILE_DRIVE *file_drive;
    const FILE_LINK *file_link;
    WCHAR *path;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE hFile;
    NTSTATUS status;

    file_link = File_FindPermLinksForMatchPath(TruePath, TruePath_len);
    if (file_link) {

        path = Dll_AllocTemp((file_link->src_len + 4) * sizeof(WCHAR));
        wmemcpy(path, file_link->src, file_link->src_len + 1);

        TruePath += file_link->dst_len + 1;

        // release lock by File_FindPermLinksForMatchPath
        LeaveCriticalSection(File_DrivesAndLinks_CritSec);

    } else {

        file_drive = File_GetDriveForPath(TruePath, TruePath_len);
        if (! file_drive) {
            SetLastError(ERROR_PATH_NOT_FOUND);
            return NULL;
        }

        path = Dll_AllocTemp((file_drive->len + 4) * sizeof(WCHAR));
        wmemcpy(path, file_drive->path, file_drive->len + 1);

        TruePath += file_drive->len + 1;

        // release lock by File_GetDriveForPath
        LeaveCriticalSection(File_DrivesAndLinks_CritSec);
    }

    //
    // open the root directory on the device and use the real
    // GetFinalPathNameByHandleW to get the \\?\Volume{...} path
    // and finally append any remaining suffix
    //

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    wcscat(path, L"\\");
    RtlInitUnicodeString(&objname, path);

    status = __sys_NtCreateFile(
        &hFile, FILE_READ_ATTRIBUTES | SYNCHRONIZE,
        &objattrs, &IoStatusBlock, NULL, 0, FILE_SHARE_VALID_FLAGS,
        FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

    Dll_Free(path);

    if (! NT_SUCCESS(status)) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return NULL;
    }

    path = Dll_AllocTemp((MAX_PATH + TruePath_len + 8) * sizeof(WCHAR));
    status = __sys_GetFinalPathNameByHandleW(
                    hFile, path, MAX_PATH, 1 /* VOLUME_NAME_GUID */);

    NtClose(hFile);

    if (status == 0 || status > MAX_PATH - 1) {
        status = GetLastError();
        Dll_Free(path);
        SetLastError(status);
        return NULL;
    }

    wcscat(path, TruePath);
    return path;
}


//---------------------------------------------------------------------------
// File_NtQueryObjectName
//---------------------------------------------------------------------------


_FX ULONG File_NtQueryObjectName(UNICODE_STRING *ObjectName, ULONG MaxLen)
{
    //
    // adjust the path returned by NtQueryObject(ObjectNameInformation)
    // to not include a sandbox prefix
    //
    // this is important because the Google Chrome broker uses NtQueryObject
    // to confirm the opened handle matches the requested path
    //

    ULONG Len = ObjectName->Length;
    WCHAR *Buf = ObjectName->Buffer;

    if (Len >= Dll_BoxFilePathLen * sizeof(WCHAR) &&
            0 == Dll_NlsStrCmp(Buf, Dll_BoxFilePath, Dll_BoxFilePathLen)) {

        ULONG NewPathLen;
        WCHAR *NewPath;

        NewPath = File_GetTruePathForBoxedPath(Buf, FALSE);
        if (! NewPath) {
            // this can happen when ObjectName is exactly
            // the sandbox prefix, e.g. X:\SANDBOX\USER\BOXNAME
            return 0;
        }

        NewPathLen = (wcslen(NewPath) + 1) * sizeof(WCHAR);
        if (MaxLen < sizeof(UNICODE_STRING) + NewPathLen)
            return NewPathLen;

        memcpy(Buf, NewPath, NewPathLen);

        ObjectName->Length = (USHORT)(wcslen(Buf) * sizeof(WCHAR));
        ObjectName->MaximumLength = ObjectName->Length + sizeof(WCHAR);

        return ObjectName->MaximumLength;
    }

    return 0;
}


//---------------------------------------------------------------------------
// File_NtSetInformationFile
//---------------------------------------------------------------------------


_FX NTSTATUS File_NtSetInformationFile(
    HANDLE FileHandle,
    IO_STATUS_BLOCK *IoStatusBlock,
    void *FileInformation,
    ULONG Length,
    FILE_INFORMATION_CLASS FileInformationClass)
{
    ULONG LastError = GetLastError();

    NTSTATUS status;
    BOOLEAN FillIoStatusBlock;

    FillIoStatusBlock = TRUE;

    //
    // set attributes request
    //

    if (FileInformationClass == FileBasicInformation) {

        if (Length < sizeof(FILE_BASIC_INFORMATION))
            status = STATUS_INFO_LENGTH_MISMATCH;
        else
            status = File_SetAttributes(FileHandle, NULL, FileInformation);

    //
    // delete request
    //

    } else if (FileInformationClass == FileDispositionInformation ||
                FileInformationClass == FileDispositionInformationEx) {

        if (Length < sizeof(FILE_DISPOSITION_INFORMATION))
            status = STATUS_INFO_LENGTH_MISMATCH;
        else
            status = File_SetDisposition(
                FileHandle, IoStatusBlock, FileInformation, Length, FileInformationClass);

    //
    // rename request
    //

    } else if ( FileInformationClass == FileRenameInformation ||
                FileInformationClass == FileRenameInformationEx ) {

        status = File_RenameFile(FileHandle, FileInformation, FALSE);

    //
    // pipe state request on a proxy pipe
    //

    } else if (
        (   FileInformationClass == FilePipeInformation ||
            FileInformationClass == FileCompletionInformation ||
            FileInformationClass == FileIoCompletionNotificationInformation)
        && ((ULONG_PTR)FileHandle & PROXY_PIPE_MASK) == PROXY_PIPE_MASK) {

        FillIoStatusBlock = FALSE;

        status = File_SetProxyPipe(
            FileHandle, IoStatusBlock,
            FileInformation, Length, FileInformationClass);
    //
    // link request
    //

    } else if ( FileInformationClass == FileLinkInformation ||
                FileInformationClass == FileLinkInformationEx || 
                FileInformationClass == FileHardLinkInformation ||
                FileInformationClass == FileHardLinkFullIdInformation) {

        if (FileInformationClass == FileLinkInformation || 
            FileInformationClass == FileLinkInformationEx) {

            status = File_RenameFile(FileHandle, FileInformation, TRUE);

        }
        else // todo
        {
            FillIoStatusBlock = FALSE;

            status = __sys_NtSetInformationFile(
                FileHandle, IoStatusBlock,
                FileInformation, Length, FileInformationClass);
        }

        if (!NT_SUCCESS(status)) {
            //
            // we don't support hard links in the sandbox, but return
            // STATUS_INVALID_DEVICE_REQUEST and hopefully the caller will
            // invoke CopyFile instead.  dfsvc.exe (ClickOnce) does that.
            //

            status = STATUS_INVALID_DEVICE_REQUEST;

            FillIoStatusBlock = TRUE;
        }

    //
    // any other request
    //

    } else {

        FillIoStatusBlock = FALSE;

        status = __sys_NtSetInformationFile(
            FileHandle, IoStatusBlock,
            FileInformation, Length, FileInformationClass);
    }

    if (FillIoStatusBlock) {

        __try {

            IoStatusBlock->Status = status;
            IoStatusBlock->Information = 0;

        } __except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
        }
    }

    SetLastError(LastError);
    return status;
}


//---------------------------------------------------------------------------
// File_SetAttributes
//---------------------------------------------------------------------------


_FX NTSTATUS File_SetAttributes(
    HANDLE FileHandle, const WCHAR *CopyPath,
    FILE_BASIC_INFORMATION *Information)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    NTSTATUS status;
    IO_STATUS_BLOCK IoStatusBlock;

    //
    // first call the system to handle the request
    //

    status = __sys_NtSetInformationFile(
        FileHandle, &IoStatusBlock,
        Information, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);

    if (status != STATUS_ACCESS_DENIED)
        return status;

    //
    // special optimization exception:  if caller asked to update
    // FILE_BASIC_INFORMATION and provided only the LastAccessTime
    // field, we ignore the request, as there is little point to
    // migrate a file into the sandbox merely to change its last
    // access time.  (most likely through Win32 SetFileTime)
    //

    if (Information->CreationTime.QuadPart      == 0 &&
        Information->LastAccessTime.QuadPart    != 0 &&
        Information->LastWriteTime.QuadPart     == 0 &&
        Information->ChangeTime.QuadPart        == 0 &&
        Information->FileAttributes             == 0)
    {
        return STATUS_SUCCESS;
    }

    //
    // special optimization exception:  if caller asked to update
    // FILE_BASIC_INFORMATION and provided -1 for all time fields
    // and zero for FileAttributes, then we do nothing
    // (this is used by Windows Explorer for some reason)
    //

    if (Information->CreationTime.QuadPart      == -1 &&
        Information->LastAccessTime.QuadPart    == -1 &&
        Information->LastWriteTime.QuadPart     == -1 &&
        Information->ChangeTime.QuadPart        == -1 &&
        Information->FileAttributes             == 0)
    {
        return STATUS_SUCCESS;
    }

    //
    // handle the request
    //

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    //
    // if we weren't given a CopyPath, then we are called from
    // NtSetInformationFile and may not actually have a copy file to
    // work with -- if NtCreateFile stripped write attributes.
    // in this case, we have to migrate the file.
    //

    if (! CopyPath) {

        WCHAR *TruePath;
        ULONG FileFlags;
        OBJECT_ATTRIBUTES objattrs;
        UNICODE_STRING objname;

        //
        // get the path of the file for which attributes have to be set
        //

        RtlInitUnicodeString(&objname, L"");

        status = File_GetName(
            FileHandle, &objname, &TruePath, (WCHAR **)&CopyPath, &FileFlags);

        if (! NT_SUCCESS(status))
            __leave;

        if (FileFlags & FGN_IS_BOXED_PATH)
            goto has_copy_path;

        //
        // migrate the file into the sandbox.  because access mask of
        // just FILE_WRITE_ATTRIBUTES | SYNCHRONIZE gets the exception
        // for executable images (see NtCreateFile), we set a special flag
        //

        InitializeObjectAttributes(
            &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

        RtlInitUnicodeString(&objname, TruePath);

        ++TlsData->file_dont_strip_write_access;

        status = NtCreateFile(
            &FileHandle,
            FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
            &objattrs, &IoStatusBlock, NULL, 0, FILE_SHARE_VALID_FLAGS,
            FILE_OPEN_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

        --TlsData->file_dont_strip_write_access;

        if (! NT_SUCCESS(status))
            __leave;

        //
        // now try to set the attributes here.  we may still get
        // STATUS_ACCESS_DENIED if the file is really a directory,
        // in this case we fall through to use the FileServer proxy.
        // in this case we also query again the full path, to account
        // for any reparse points which may have been referenced
        //

        status = __sys_NtSetInformationFile(
            FileHandle, &IoStatusBlock,
            Information, sizeof(FILE_BASIC_INFORMATION),
            FileBasicInformation);

        if (status == STATUS_ACCESS_DENIED) {

            NTSTATUS status2;
            WCHAR *CopyPath2;

            RtlInitUnicodeString(&objname, L"");

            status2 = File_GetName(
                FileHandle, &objname, &TruePath, &CopyPath2, NULL);

            if (NT_SUCCESS(status2))
                *((WCHAR **)&CopyPath) = CopyPath2;
        }

        NtClose(FileHandle);

        if (status != STATUS_ACCESS_DENIED)
            __leave;
    }

has_copy_path:

    //
    // ask the FileServer proxy in SbieSvc to do the job.  this is needed
    // because the driver always blocks FILE_WRITE_ATTRIBUTE access on
    // directory files, even in the sandbox, to block junction points
    //

    if (CopyPath) {

        MSG_HEADER *rpl;
        FILE_SET_ATTRIBUTES_REQ *req;

        ULONG CopyPath_len = (wcslen(CopyPath) + 1) * sizeof(WCHAR);
        ULONG req_len = sizeof(FILE_SET_ATTRIBUTES_REQ) + CopyPath_len;

        req = (FILE_SET_ATTRIBUTES_REQ *)Dll_AllocTemp(req_len);
        if (req) {

            req->h.length = req_len;
            req->h.msgid = MSGID_FILE_SET_ATTRIBUTES;

            memcpy(&req->info, Information, sizeof(FILE_BASIC_INFORMATION));

            req->path_len = CopyPath_len;
            wcscpy(req->path, CopyPath);

            rpl = SbieDll_CallServer(&req->h);
            if (rpl) {
                status = rpl->status;
                Dll_Free(rpl);
            } else
                status = STATUS_ACCESS_DENIED;
        }

        Dll_Free(req);
    }

    //
    // finish
    //

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    Dll_PopTlsNameBuffer(TlsData);
    return status;
}


//---------------------------------------------------------------------------
// File_SetDisposition
//---------------------------------------------------------------------------


_FX NTSTATUS File_SetDisposition(
    HANDLE FileHandle, IO_STATUS_BLOCK *IoStatusBlock,
    void *FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    UNICODE_STRING uni;
    //WCHAR *DosPath;
    NTSTATUS status;
    ULONG FileFlags;
    ULONG mp_flags;
    FILE_ATTRIBUTE_TAG_INFORMATION taginfo;

    //
    // check if the specified path is an open or closed path
    //

    RtlInitUnicodeString(&uni, L"");
    
    mp_flags = 0;
    //DosPath = NULL;

    Dll_PushTlsNameBuffer(TlsData);

    __try {

        WCHAR *TruePath, *CopyPath;

        status = File_GetName(
                    FileHandle, &uni, &TruePath, &CopyPath, &FileFlags);

        if (NT_SUCCESS(status)) {

            mp_flags = File_MatchPath(TruePath, &FileFlags);

            if (PATH_IS_CLOSED(mp_flags))
                status = STATUS_ACCESS_DENIED;

            else if (PATH_NOT_OPEN(mp_flags)) {

                status = __sys_NtQueryInformationFile(
                    FileHandle, IoStatusBlock,
                    &taginfo, sizeof(taginfo), FileAttributeTagInformation);

                if (NT_SUCCESS(status) && (taginfo.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0)
                    __leave;

                status = File_DeleteDirectory(CopyPath, TRUE);

                if (status != STATUS_DIRECTORY_NOT_EMPTY)
                    status = STATUS_SUCCESS;

                /*
                if (NT_SUCCESS(status) && Dll_ChromeSandbox) {

                    //
                    // if this is a Chrome sandbox process, we have
                    // to pass a DOS path to NtDeleteFile rather
                    // than a file handle
                    //

                    ULONG len = wcslen(TruePath);
                    DosPath = Dll_AllocTemp((len + 8) * sizeof(WCHAR));
                    wmemcpy(DosPath, TruePath, len + 1);
                    if (SbieDll_TranslateNtToDosPath(DosPath)) {
                        len = wcslen(DosPath);
                        wmemmove(DosPath + 4, DosPath, len + 1);
                        wmemcpy(DosPath, File_BQQB, 4);
                    } else {
                        Dll_Free(DosPath);
                        DosPath = NULL;
                    }
                }
                */
            }
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    Dll_PopTlsNameBuffer(TlsData);

    //
    // handle the request appropriately
    //
  
    if (PATH_IS_OPEN(mp_flags) /*|| ((FileFlags & FGN_IS_BOXED_PATH) != 0)*/) { // boxed path fails for directories

        status = __sys_NtSetInformationFile(
            FileHandle, IoStatusBlock,
            FileInformation, Length, FileInformationClass);

    } else if (NT_SUCCESS(status)) {

        BOOLEAN DeleteOnClose = FALSE;

        if (FileInformationClass == FileDispositionInformation) {

            DeleteOnClose = ((FILE_DISPOSITION_INFORMATION*)FileInformation)->DeleteFileOnClose;

        } else if (FileInformationClass == FileDispositionInformationEx) { // Win 10 RS1 and later

            ULONG Flags = ((FILE_DISPOSITION_INFORMATION_EX*)FileInformation)->Flags;

            if ((Flags & FILE_DISPOSITION_DELETE) != 0)
                DeleteOnClose = TRUE;
            else if((Flags & FILE_DISPOSITION_ON_CLOSE) != 0) // FILE_DISPOSITION_ON_CLOSE with no FILE_DISPOSITION_DELETE means clear flag
                DeleteOnClose = FALSE;
        }

        OBJECT_ATTRIBUTES objattrs;

        InitializeObjectAttributes(
            &objattrs, &uni, OBJ_CASE_INSENSITIVE, FileHandle, NULL);

        //
        // check if the call to File_NtDeleteFileImpl from the delete handler is expected to fail 
        // and return the appropriate error
        //

        FILE_NETWORK_OPEN_INFORMATION info;
        if (NT_SUCCESS(__sys_NtQueryFullAttributesFile(&objattrs, &info)) && ((info.FileAttributes & FILE_ATTRIBUTE_READONLY) != 0)) {

            status = STATUS_CANNOT_DELETE;
        } else {

            Handle_SetDeleteOnClose(FileHandle, DeleteOnClose);
        }

	    /*
        if (DosPath) {
            objattrs.RootDirectory = NULL;
            RtlInitUnicodeString(&uni, DosPath);
        }

        status = File_NtDeleteFileImpl(&objattrs);
	    */

        IoStatusBlock->Status = 0;
        IoStatusBlock->Information = 8;
    }

    //if (DosPath)
    //    Dll_Free(DosPath);

    SetLastError(LastError);
    return status;
}


//---------------------------------------------------------------------------
// File_NtDeleteFile
//---------------------------------------------------------------------------


_FX NTSTATUS File_NtDeleteFile(OBJECT_ATTRIBUTES *ObjectAttributes)
{
    NTSTATUS status = File_NtDeleteFileImpl(ObjectAttributes);

    status = StopTailCallOptimization(status);

    return status;
}

//---------------------------------------------------------------------------
// File_NtDeleteFileImpl
//---------------------------------------------------------------------------


_FX NTSTATUS File_NtDeleteFileImpl(OBJECT_ATTRIBUTES *ObjectAttributes)
{
    NTSTATUS status;
    HANDLE handle;
    IO_STATUS_BLOCK IoStatusBlock;

    status = File_NtCreateFileImpl(
        &handle, DELETE, ObjectAttributes, &IoStatusBlock, NULL, 0,
        FILE_SHARE_VALID_FLAGS, FILE_OPEN, FILE_DELETE_ON_CLOSE | FILE_OPEN_REPARSE_POINT, NULL, 0);

    if (NT_SUCCESS(status))
        NtClose(handle);

    return status;
}


//---------------------------------------------------------------------------
// File_RenameOpenFile
//---------------------------------------------------------------------------


_FX LONG File_RenameOpenFile(
    HANDLE file_handle,
    const WCHAR* user_dir, const WCHAR* user_name,
    BOOLEAN replace_if_exists)
{
    //
    // in compartment mode we don't need driver assistance we can do things ourselves
    // this code is a port of the same routine in the driver
    //

    NTSTATUS status;
    ULONG user_dir_len = (wcslen(user_dir) + 1) * sizeof(WCHAR);
    ULONG user_name_len = (wcslen(user_name) + 1) * sizeof(WCHAR);
    WCHAR *path, *name;
    FILE_RENAME_INFORMATION *info;
    ULONG path_len, name_len, info_len;
    WCHAR save_char;
    HANDLE dir_handle;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    IO_STATUS_BLOCK IoStatusBlock;
    ULONG mp_flags;

    //
    // copy user parameters into consolidated buffer:  dir"\"name
    //

    path_len = user_dir_len + user_name_len + sizeof(WCHAR) * 8;
    path = Dll_AllocTemp(path_len);
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

    mp_flags = File_MatchPath(path, NULL);
        
    if (!PATH_IS_OPEN(mp_flags) || PATH_IS_CLOSED(mp_flags)) {
        Dll_Free(path);
        return STATUS_BAD_INITIAL_PC;
    }

    //
    // check if the target directory is an open path, and stop if it is
    //

    *name = L'\0';

    mp_flags = File_MatchPath(path, NULL);

    if (PATH_IS_OPEN(mp_flags) || PATH_IS_CLOSED(mp_flags)) {    
        Dll_Free(path);
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

    status = __sys_NtCreateFile(
        &dir_handle, FILE_GENERIC_WRITE, &objattrs,
        &IoStatusBlock, NULL, 0, FILE_SHARE_VALID_FLAGS,
        FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

    if (! NT_SUCCESS(status)) {
        Dll_Free(path);
        return status;
    }

    //
    // allocate an information buffer, and issue rename request
    //

    ++name;
    *name = save_char;
    name_len = wcslen(name) * sizeof(WCHAR);

    info_len = sizeof(FILE_RENAME_INFORMATION) + name_len + 8;
    info = Dll_AllocTemp(info_len);
    if (! info)
        status = STATUS_INSUFFICIENT_RESOURCES;
    else {

        memzero(info, info_len);
        info->ReplaceIfExists = replace_if_exists;
        info->RootDirectory = dir_handle;
        info->FileNameLength = name_len;
        memcpy(info->FileName, name, name_len);

		if (NT_SUCCESS(status)) {

			status = __sys_NtSetInformationFile(
				file_handle, &IoStatusBlock, //args->file_handle.val, &IoStatusBlock,
				info, info_len, FileRenameInformation);
		}

        // FIXME, we may get STATUS_NOT_SAME_DEVICE, however, in most cases,
        // this API call is used to rename a file inside a folder, rather
        // than move files across folders, so that isn't a problem

        Dll_Free(info);
    }

    NtClose(dir_handle);
    Dll_Free(path);
    return status;
}


//---------------------------------------------------------------------------
// File_OpenForRenameFile
//---------------------------------------------------------------------------


_FX NTSTATUS File_OpenForRenameFile(
    HANDLE* pSourceHandle, const WCHAR *TruePath)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    IO_STATUS_BLOCK IoStatusBlock;

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, Secure_NormalSD);

    //
    // open the file for write access.  this should cause the file
    // to be migrated into the sandbox, including its parent directories
    //

    RtlInitUnicodeString(&objname, TruePath);

    ++TlsData->file_dont_strip_write_access;

    status = NtCreateFile(
        pSourceHandle, FILE_GENERIC_WRITE | DELETE, &objattrs,
        &IoStatusBlock, NULL, 0, FILE_SHARE_VALID_FLAGS,
        FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

    if (status == STATUS_SHARING_VIOLATION ||
        status == STATUS_ACCESS_DENIED) {

        //
        // Windows Mail opens *.eml files with a combination of
        // FILE_SHARE_READ | FILE_SHARE_DELETE, but not FILE_SHARE_WRITE,
        // which means we can't open them with FILE_GENERIC_WRITE
        // during rename processing here
        //
        // also, for read-only files, we get an error when we open them
        // for FILE_GENERIC_WRITE, but just DELETE should also work
        //

        status = NtCreateFile(
            pSourceHandle, SYNCHRONIZE | DELETE, &objattrs,
            &IoStatusBlock, NULL, 0, FILE_SHARE_VALID_FLAGS,
            FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
    }

    --TlsData->file_dont_strip_write_access;

    return status;
}


//---------------------------------------------------------------------------
// File_RenameFile
//---------------------------------------------------------------------------


_FX NTSTATUS File_RenameFile(
    HANDLE FileHandle, void *info, BOOLEAN LinkOp)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    IO_STATUS_BLOCK IoStatusBlock;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    HANDLE SourceHandle, TargetHandle;
    WCHAR *SourceTruePath;
    WCHAR *SourceCopyPath;
    WCHAR *TargetTruePath;
    WCHAR *TargetCopyPath;
    WCHAR *TargetFileName;
    WCHAR *ReparsedPath;
    WCHAR save_char;
    ULONG info2_len;
    void *info2;
    FILE_NETWORK_OPEN_INFORMATION open_info;
    ULONG SourceFlags;
    ULONG TargetFlags;
    ULONG len;
    BOOLEAN ReplaceIfExists;

    SourceHandle = NULL;
    TargetHandle = NULL;
    SourceTruePath = NULL;
    SourceCopyPath = NULL;
    TargetTruePath = NULL;
    TargetCopyPath = NULL;
    info2 = NULL;

    Dll_PushTlsNameBuffer(TlsData);

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, Secure_NormalSD);

    __try {

    //
    // get the name of the file to be renamed
    //

    RtlInitUnicodeString(&objname, L"");

    status = File_GetName(FileHandle, &objname, &TruePath, &CopyPath, NULL);

    if (! NT_SUCCESS(status))
        __leave;

    //
    // migrate into the sandbox, including its parent directories
    //

    status = File_OpenForRenameFile(&SourceHandle, TruePath);

    //
    // if we still get STATUS_SHARING_VIOLATION, give up on trying
    // to make sure the file is migrated into the sandbox, and hope
    // that the input FileHandle is suitable for a rename operation
    //

    if (status == STATUS_SHARING_VIOLATION) {

        SourceHandle = FileHandle;
        status = STATUS_SUCCESS;
    }

    if (! NT_SUCCESS(status))
        __leave;

    //
    // get the name of the opened handle, again.  if it is still
    // outside the box, it must be an open path
    //

    RtlInitUnicodeString(&objname, L"");

    status = File_GetName(
        SourceHandle, &objname, &TruePath, &CopyPath, &SourceFlags);

    if (! NT_SUCCESS(status))
        __leave;

    //
    // save a copy of the source path:
    //
    // 1.  if this isn't an open path, we will need to re-create
    //     the source path as empty file, marked deleted before
    //     we finish
    //
    // 2.  if ReplaceIfExists is not specified, we have to check
    //     if the source and target are the same path
    //

    len = (wcslen(TruePath) + 1) * sizeof(WCHAR);
    SourceTruePath = Dll_AllocTemp(len);
    memcpy(SourceTruePath, TruePath, len);

    len = (wcslen(CopyPath) + 1) * sizeof(WCHAR);
    SourceCopyPath = Dll_AllocTemp(len);
    memcpy(SourceCopyPath, CopyPath, len);

    //
    // get the target name requested by the caller, and keep
    // duplicated strings, because any call to NtCreateFile may
    // overwrite the shared name buffers
    //

    if (LinkOp) {

        FILE_LINK_INFORMATION *infoL = info;

        objname.Length = (USHORT)infoL->FileNameLength;
        objname.MaximumLength = objname.Length;
        objname.Buffer = infoL->FileName;

        status = File_GetName(
            infoL->RootDirectory, &objname, &TruePath, &CopyPath, &TargetFlags);

        ReplaceIfExists = infoL->ReplaceIfExists;

    } else {

        FILE_RENAME_INFORMATION *infoR = info;

        objname.Length = (USHORT)infoR->FileNameLength;
        objname.MaximumLength = objname.Length;
        objname.Buffer = infoR->FileName;

        status = File_GetName(
            infoR->RootDirectory, &objname, &TruePath, &CopyPath, &TargetFlags);

        ReplaceIfExists = infoR->ReplaceIfExists;
    }

    if (! NT_SUCCESS(status))
        __leave;

    ReparsedPath = File_TranslateTempLinks(TruePath, FALSE);
    if (ReparsedPath) {

        len = (wcslen(ReparsedPath) + 1) * sizeof(WCHAR);
        TargetTruePath = Dll_AllocTemp(len);
        memcpy(TargetTruePath, ReparsedPath, len);
        Dll_Free(ReparsedPath);

    } else {

        len = (wcslen(TruePath) + 1) * sizeof(WCHAR);
        TargetTruePath = Dll_AllocTemp(len);
        memcpy(TargetTruePath, TruePath, len);
    }

    len = (wcslen(CopyPath) + 1) * sizeof(WCHAR);
    TargetCopyPath = Dll_AllocTemp(len);
    memcpy(TargetCopyPath, CopyPath, len);

    //
    // separate the true path into directory and filename portions
    //

    TargetFileName = wcsrchr(TargetTruePath, L'\\');

    if (wcschr(TargetFileName, L':')) {
        status = STATUS_INVALID_PARAMETER;
        __leave;
    }

    ++TargetFileName;

    if(!LinkOp) {

        //
        // if the full path name for the target is an open path, we want
        // to be able to rename outside the sandbox.  however, the parent
        // directory in that full path may not be an open path itself.
        // invoke the driver to do such a rename on our behalf
        //

        TargetFileName[-1] = L'\0';

        ReparsedPath = File_FixPermLinksForMatchPath(TargetTruePath);
        if (! ReparsedPath)
            ReparsedPath = TargetTruePath;

        status = SbieApi_RenameFile(SourceHandle, ReparsedPath, TargetFileName, ReplaceIfExists);

        if (ReparsedPath != TargetTruePath)
            Dll_Free(ReparsedPath);

        TargetFileName[-1] = L'\\';

        if (status != STATUS_BAD_INITIAL_PC) {

            if (NT_SUCCESS(status))
                goto after_rename;
            __leave;
        }

    }

    //
    // open the parent directory of the target path name of the rename.
    // this should cause the directory to be created in the sandbox,
    // or return a handle to the true directory, if it is an open path
    //
    // we keep the trailing backslash on the path, so that we can open
    // the parent directory even when the parent is the root directory
    //

    save_char = *TargetFileName;
    *TargetFileName = L'\0';

    RtlInitUnicodeString(&objname, TargetTruePath);

    ++TlsData->file_dont_strip_write_access;

    status = NtCreateFile(
        &TargetHandle, FILE_GENERIC_WRITE, &objattrs,
        &IoStatusBlock, NULL, 0, FILE_SHARE_VALID_FLAGS, FILE_OPEN,
        FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE, NULL, 0);

    if (status == STATUS_OBJECT_NAME_NOT_FOUND) {

        //
        // we got "file not found" when opening the parent directory,
        // so we really need to return a "path not found" error code
        //

        status = STATUS_OBJECT_PATH_NOT_FOUND;

    } else if (status == STATUS_ACCESS_DENIED) {

        //
        // for hidden/system/read-only directories we can get access
        // denied, so try to explicitly create the copy directory.
        // if that also fails, then open the directory read-only
        //

        WCHAR *TargetCopyPtr = wcsrchr(TargetCopyPath, L'\\') + 1;
        WCHAR save_char_copy = *TargetCopyPtr;
        *TargetCopyPtr = L'\0';

        File_CreatePath(TargetTruePath, TargetCopyPath);

        *TargetCopyPtr = save_char_copy;

        status = NtCreateFile(
            &TargetHandle, FILE_GENERIC_WRITE, &objattrs,
            &IoStatusBlock, NULL, 0, FILE_SHARE_VALID_FLAGS, FILE_OPEN,
            FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE, NULL, 0);

        if (status == STATUS_ACCESS_DENIED) {

            status = NtCreateFile(
                &TargetHandle, FILE_GENERIC_READ, &objattrs,
                &IoStatusBlock, NULL, 0, FILE_SHARE_VALID_FLAGS, FILE_OPEN,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE, NULL, 0);
        }
    }

    --TlsData->file_dont_strip_write_access;

    *TargetFileName = save_char;

    if (! NT_SUCCESS(status))
        __leave;

    //
    // allocate a new information buffer
    //

    if (LinkOp) {

        FILE_LINK_INFORMATION *infoL = info;
        FILE_LINK_INFORMATION *info2L;

        info2_len = sizeof(FILE_LINK_INFORMATION)
                  + wcslen(TargetFileName) * sizeof(WCHAR);
        info2 = Dll_AllocTemp(info2_len);

        info2L = info2;
        info2L->ReplaceIfExists = infoL->ReplaceIfExists;
        info2L->RootDirectory = TargetHandle;
        info2L->FileNameLength = wcslen(TargetFileName) * sizeof(WCHAR);
        memcpy(info2L->FileName, TargetFileName, info2L->FileNameLength);

    } else {

        FILE_RENAME_INFORMATION *infoR = info;
        FILE_RENAME_INFORMATION *info2R;

        info2_len = sizeof(FILE_RENAME_INFORMATION)
                  + wcslen(TargetFileName) * sizeof(WCHAR);
        info2 = Dll_AllocTemp(info2_len);

        info2R = info2;
        info2R->ReplaceIfExists = infoR->ReplaceIfExists;
        info2R->RootDirectory = TargetHandle;
        info2R->FileNameLength = wcslen(TargetFileName) * sizeof(WCHAR);
        memcpy(info2R->FileName, TargetFileName, info2R->FileNameLength);

    }

    //
    // if the source and target paths are the same (in a case
    // insensitive compare), then skip the following check which
    // might result in the possible deletion of the source path
    //

    if (_wcsicmp(SourceTruePath, TargetTruePath) == 0) {

        goto issue_rename;
    }

    //
    // if the caller requested to replace the destination file,
    // then physically delete it first.  we expect this may fail:
    // if the file does not actually exist, or if it exists only
    // outside the sandbox in a directory that isn't open.
    //

    RtlInitUnicodeString(&objname, TargetCopyPath);

    if (! ReplaceIfExists) {

        //
        // if caller did not explicitly ask to replace, but the
        // destination path name is marked deleted, then we also
        // physically delete the destination
        //

        status = __sys_NtQueryFullAttributesFile(&objattrs, &open_info);

        if (NT_SUCCESS(status)) {

            if (IS_DELETE_MARK(&open_info.CreationTime)) { // !File_Delete_v2 &&

				ReplaceIfExists = TRUE;
                if (LinkOp) ((FILE_LINK_INFORMATION*)info2)->ReplaceIfExists = TRUE;
                else        ((FILE_RENAME_INFORMATION*)info2)->ReplaceIfExists = TRUE;

            } else {
                status = STATUS_OBJECT_NAME_COLLISION;
                __leave;
            }

        } else {

            WCHAR* TargetTruePath2 = TargetTruePath;

            ULONG TargetTruePathFlags = 0;
            WCHAR* OldTruePath = File_ResolveTruePath(TargetTruePath, TargetCopyPath, &TargetTruePathFlags);
            if (OldTruePath)
                TargetTruePath2 = OldTruePath;

            RtlInitUnicodeString(&objname, TargetTruePath2);

            if (FILE_PATH_DELETED(TargetTruePathFlags)) // File_Delete_v2 &&
            {
                status = STATUS_OBJECT_NAME_NOT_FOUND;
            }
            // $Workaround$ - 3rd party fix
            else if (!Dll_DigitalGuardian)
            {
                status = __sys_NtQueryFullAttributesFile(&objattrs, &open_info);
            }
            else
            {
                ULONG mp_flags = File_MatchPath(TargetTruePath2, &TargetFlags);

                if (PATH_IS_OPEN(mp_flags) || !mp_flags)
                {
                    // check true path exist
                    status = __sys_NtQueryFullAttributesFile(&objattrs, &open_info);
                }
                else
                {
                    // check copy path exist
                    RtlInitUnicodeString(&objname, TargetCopyPath);
                    status = __sys_NtQueryFullAttributesFile(&objattrs, &open_info);
                }
            }

            if (NT_SUCCESS(status)) {
                status = STATUS_OBJECT_NAME_COLLISION;
                __leave;
            }

            RtlInitUnicodeString(&objname, TargetCopyPath);
        }
    }

    if (ReplaceIfExists) {

        __sys_NtDeleteFile(&objattrs);
    }

    //
    // issue the rename request
    //

issue_rename:

    status = __sys_NtSetInformationFile(
        SourceHandle, &IoStatusBlock,
        info2, info2_len, LinkOp ? FileLinkInformation : FileRenameInformation);

    if (status == STATUS_SHARING_VIOLATION && SourceHandle != FileHandle) {

        //
        // in case opening that the second SourceHandle prevents the
        // rename from succeeding, try again with FileHandle (assuming
        // it was opened with DELETE access)
        //

        NtClose(SourceHandle);
        SourceHandle = FileHandle;

        status = __sys_NtSetInformationFile(
            SourceHandle, &IoStatusBlock,
            info2, info2_len, LinkOp ? FileLinkInformation : FileRenameInformation);
    }

    if (! NT_SUCCESS(status)) {

        // FIXME, we may get STATUS_NOT_SAME_DEVICE here, if the rename
        // involves an OpenFilePath, however, in most cases, this call
        // is coming from kernel32!MoveFileXxx, which is smart enough
        // to copy a file when it can't be renamed (MOVEFILE_COPY_ALLOWED)

        if (! NT_SUCCESS(status))
            __leave;
    }

    NtClose(TargetHandle);
    TargetHandle = NULL;

    //
    // in the case the target file was marked deleted in the sandbox,
    // make sure the delete mark is overwritten.  we're working on the
    // target copy path, so we expect the NtCreateFile may fail if this
    // is an open path
    //

    if (!File_Delete_v2) {

        RtlInitUnicodeString(&objname, TargetCopyPath);

        status = __sys_NtCreateFile(
            &TargetHandle, FILE_READ_ATTRIBUTES | SYNCHRONIZE,
            &objattrs, &IoStatusBlock, NULL, 0, FILE_SHARE_VALID_FLAGS,
            FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

        if (NT_SUCCESS(status)) {

            File_SetCreateTime(TargetHandle, TargetCopyPath);

            NtClose(TargetHandle);
            TargetHandle = NULL;

        }
        else
            status = STATUS_SUCCESS;
    }

    //
    // record for recovery
    //

    File_RecordRecover(FileHandle, TargetTruePath);

    //
    // if the source file exists in the sandbox, we need to create an
    // empty file, marked deleted, in place of the old name
    //

after_rename:

    if (SourceFlags & FGN_IS_BOXED_PATH) {

        NTSTATUS status2;
        WCHAR* SourceTruePath2 = SourceTruePath;

        WCHAR* OldTruePath = File_ResolveTruePath(SourceTruePath, SourceCopyPath, NULL);
        if (OldTruePath)
            SourceTruePath2 = OldTruePath;

        RtlInitUnicodeString(&objname, SourceTruePath2);
        status2 = __sys_NtQueryFullAttributesFile(&objattrs, &open_info);

        if (File_Delete_v2) {

            BOOLEAN TrueExists = FALSE;
            BOOLEAN IsDirectroy;

            if (NT_SUCCESS(status2)) {

                //
                // if this file exist in the true path mark it as deleted,
                // directories are handled by File_SetRelocation
                //

                TrueExists = TRUE;
                IsDirectroy = (open_info.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

                if (!IsDirectroy)
                    File_MarkDeleted_v2(SourceTruePath);
            }
            else {

                //
                // if it does not exist check if it was a directory, it may be a boxed directory 
                // which is a relocation target in which case we will need to update the relocation data
                //

                IO_STATUS_BLOCK IoStatusBlock;
                FILE_BASIC_INFORMATION info3;

                status2 = __sys_NtQueryInformationFile(
                    FileHandle, &IoStatusBlock, &info3,
                    sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);

                IsDirectroy = (info3.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
            }

            //
            // if this is a directory and if so update/create the appropriate remapping
            //

            if (TrueExists && IsDirectroy) {

                File_SetRelocation(SourceTruePath, TargetTruePath);
            }
        }
        else
        if (NT_SUCCESS(status2)) {

            HANDLE handle2;

            //
            // mark deleted only if there is a corresponding file
            // outside the sandbox
            //

            RtlInitUnicodeString(&objname, SourceCopyPath);

            status2 = __sys_NtCreateFile(
                &handle2, FILE_GENERIC_READ, &objattrs,
                &IoStatusBlock, NULL, 0, FILE_SHARE_VALID_FLAGS,
                FILE_CREATE,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE,
                NULL, 0);

            if (NT_SUCCESS(status2)) {

                File_MarkDeleted(handle2, SourceCopyPath);

                NtClose(handle2);
            }
        }
    }

    //
    // finish
    //

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    Dll_PopTlsNameBuffer(TlsData);

    if (SourceHandle && SourceHandle != FileHandle)
        NtClose(SourceHandle);
    if (TargetHandle)
        NtClose(TargetHandle);
    if (SourceTruePath)
        Dll_Free(SourceTruePath);
    if (SourceCopyPath)
        Dll_Free(SourceCopyPath);
    if (TargetTruePath)
        Dll_Free(TargetTruePath);
    if (TargetCopyPath)
        Dll_Free(TargetCopyPath);
    if (info2)
        Dll_Free(info2);

    return status;
}


//---------------------------------------------------------------------------
// File_GetTrueHandle
//---------------------------------------------------------------------------


_FX HANDLE File_GetTrueHandle(HANDLE FileHandle, BOOLEAN *pIsOpenPath)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE handle = NULL;
    ULONG FileFlags;

    Dll_PushTlsNameBuffer(TlsData);

    //
    // get file path for the handle
    //

    if (pIsOpenPath)
        *pIsOpenPath = FALSE;

    RtlInitUnicodeString(&objname, L"");
    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = File_GetName(
                    FileHandle, &objname, &TruePath, &CopyPath, &FileFlags);
    if (NT_SUCCESS(status)) {

        //
        // check if this is an open or closed path
        //

        ULONG mp_flags = File_MatchPath(TruePath, &FileFlags);

        if (PATH_IS_OPEN(mp_flags) && pIsOpenPath)
            *pIsOpenPath = TRUE;

        if (! mp_flags) {

            //
            // open file
            //

            RtlInitUnicodeString(&objname, TruePath);

            status = __sys_NtCreateFile(
                &handle, FILE_GENERIC_READ, &objattrs,
                &IoStatusBlock, NULL, 0, FILE_SHARE_VALID_FLAGS,
                FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

            if (! NT_SUCCESS(status))
                handle = NULL;
        }
    }

    Dll_PopTlsNameBuffer(TlsData);
    SetLastError(LastError);
    return handle;
}


//---------------------------------------------------------------------------
// SbieDll_GetHandlePath
//---------------------------------------------------------------------------


_FX ULONG SbieDll_GetHandlePath(
    HANDLE FileHandle, WCHAR *OutWchar8192, BOOLEAN *IsBoxedPath)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    NTSTATUS status;
    ULONG FileFlags;
    WCHAR *TruePath;
    WCHAR *CopyPath;

    SbieDll_GetDrivePath(0);            // initialize drives

    Dll_PushTlsNameBuffer(TlsData);

    //
    // This function returns actual paths as they exist in the real filesystem
    // copy paths may point to the snapshot if the file is there
    // and true paths will point to the original location if they're redirected
    // therefore calling hooked file functions on these paths may run into file not found 
    // when the original location is marked as deleted
    //

    status = File_GetName(
        FileHandle, NO_RELOCATION, &TruePath, &CopyPath, &FileFlags);

    if (IsBoxedPath) {

        if (FileFlags & FGN_IS_BOXED_PATH)
            *IsBoxedPath = TRUE;
        else
            *IsBoxedPath = FALSE;

    } else if (status == STATUS_BAD_INITIAL_PC)
        status = STATUS_SUCCESS;

    if (NT_SUCCESS(status) && OutWchar8192) {

        ULONG len;
        WCHAR *src = TruePath;
        if (Dll_BoxName &&              // sandboxed process
                IsBoxedPath && *IsBoxedPath) {

            src = CopyPath;
        }

        len = wcslen(src);
        if (len > 8192 / sizeof(WCHAR) - 4)
            len = 8192 / sizeof(WCHAR) - 4;
        wmemcpy(OutWchar8192, src, len);
        OutWchar8192[len] = L'\0';
    }

    Dll_PopTlsNameBuffer(TlsData);

    return status;
}


//---------------------------------------------------------------------------
// SbieDll_GetDrivePath
//---------------------------------------------------------------------------


_FX const WCHAR *SbieDll_GetDrivePath(ULONG DriveIndex)
{
    if ((! File_Drives) || (DriveIndex == -1)) {
        File_InitDrives(0xFFFFFFFF);
        if (DriveIndex == -1)
            return NULL;
    }

    if (File_Drives && (DriveIndex < 26) && File_Drives[DriveIndex])
        return File_Drives[DriveIndex]->path;

    return NULL;
}


//---------------------------------------------------------------------------
// SbieDll_GetUserPathEx
//---------------------------------------------------------------------------


_FX const WCHAR *SbieDll_GetUserPathEx(WCHAR which)
{
    if (! Dll_SidString) {

        UNICODE_STRING SidString;
        NTSTATUS status = Dll_GetCurrentSidString(&SidString);
        if (NT_SUCCESS(status))
            Dll_SidString = SidString.Buffer;

        if (! Dll_SidString)
            return NULL;
    }

    if (! File_CurrentUser) {

        SbieDll_GetDrivePath(0);            // initialize drives
        File_InitUsers();
    }

    if (which == L'a')
        return File_AllUsers;
    else if (which == L'c')
        return File_CurrentUser;
    else if (which == L'p')
        return File_PublicUser;

    return NULL;
}


//---------------------------------------------------------------------------
// SbieDll_TranslateNtToDosPath
//---------------------------------------------------------------------------


_FX BOOLEAN SbieDll_TranslateNtToDosPath(WCHAR *path)
{
    const FILE_DRIVE *drive;
    ULONG path_len, prefix_len;

    // 
    // sometimes we get a DOS path with the \??\ prefix
    // in such cases we just quickly strip it and are done
    // 

    if (_wcsnicmp(path, L"\\??\\", 4) == 0) {
    
        wmemmove(path, path + 4, wcslen(path) - 4 + 1);
    
        return TRUE;
    }

    if (! File_DrivesAndLinks_CritSec) {    // if not sandboxed

        File_DrivesAndLinks_CritSec = Dll_Alloc(sizeof(CRITICAL_SECTION));
        InitializeCriticalSectionAndSpinCount(
            File_DrivesAndLinks_CritSec, 1000);
        SbieDll_GetDrivePath(0);            // initialize drives
    }

    if (_wcsnicmp(path, File_Mup, File_MupLen) == 0) {

        WCHAR *ptr = path + File_MupLen - 1;
        wmemmove(path + 1, ptr, wcslen(ptr) + 1);

        return TRUE;
    }

    path_len = wcslen(path);
    
    //
    // workaround for hidden box root
    //

    if (Dll_BoxFileDosPathLen && Dll_BoxFilePathLen <= path_len && _wcsnicmp(path, Dll_BoxFilePath, Dll_BoxFilePathLen) == 0)
    {
        wmemmove(path + Dll_BoxFileDosPathLen, path + Dll_BoxFilePathLen, wcslen(path + Dll_BoxFilePathLen) + 1);
        wmemcpy(path, Dll_BoxFileDosPath, Dll_BoxFileDosPathLen);
        return TRUE;
    }

    //
    // Find Dos Drive Letter
    //

    drive = File_GetDriveForPath(path, path_len);
    if (drive)
        prefix_len = drive->len;
    else
        drive = File_GetDriveForUncPath(path, path_len, &prefix_len);

    if (drive) {

        WCHAR drive_letter = drive->letter;
        WCHAR *ptr = path + prefix_len;

        LeaveCriticalSection(File_DrivesAndLinks_CritSec);

        if (*ptr == L'\\' || *ptr == L'\0') {
            wmemmove(path + 2, ptr, wcslen(ptr) + 1);
            path[0] = drive_letter;
            path[1] = L':';
        }

        return TRUE;
    }

    // 
    // sometimes we have to use a path which has no drive letter
    // to make this work we use the \\.\ prefix which replaces \Device\
    // and is accepted by regular non NT Win32 APIs
    // 
    // Note: fix me this makes chrome crash handler hang
    // 

    /*if (_wcsnicmp(path, L"\\Device\\", 8) == 0) {

        wcscpy(path, L"\\\\.\\");
        wmemmove(path + 4, path + 8, wcslen(path + 8) + 1);

        return TRUE;
    }*/

    return FALSE;
}


//---------------------------------------------------------------------------
// File_GetTruePathForBoxedPath
//---------------------------------------------------------------------------


_FX WCHAR *File_GetTruePathForBoxedPath(const WCHAR *Path, BOOLEAN IsDosPath)
{
    WCHAR *NtPath;
    WCHAR *TruePathResult = NULL;

    if (IsDosPath)
        NtPath = File_TranslateDosToNtPath(Path);
    else
        NtPath = (WCHAR *)Path;

    if (NtPath) {

        if (_wcsnicmp(NtPath, Dll_BoxFilePath, Dll_BoxFilePathLen) == 0 || (Dll_BoxFileRawPath && _wcsnicmp(NtPath, Dll_BoxFileRawPath, Dll_BoxFileRawPathLen) == 0)) {

            NTSTATUS status;
            UNICODE_STRING uni;
            WCHAR *TruePath, *CopyPath;

            THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

            RtlInitUnicodeString(&uni, NtPath);

            Dll_PushTlsNameBuffer(TlsData);

            status = File_GetName(NULL, &uni, &TruePath, &CopyPath, NULL);

            if (status == STATUS_BAD_INITIAL_PC) {

                //
                // add suffix backslash to c:\sandbox\...\drive\x so
                // File_GetName doesn't think we try to open a device
                //

                ULONG len = wcslen(NtPath);
                WCHAR *BackslashPath =
                                Dll_AllocTemp((len + 2) * sizeof(WCHAR));
                wmemcpy(BackslashPath, NtPath, len);
                BackslashPath[len] = L'\\';
                BackslashPath[len + 1] = L'\0';

                RtlInitUnicodeString(&uni, BackslashPath);

                status =
                    File_GetName(NULL, &uni, &TruePath, &CopyPath, NULL);

                Dll_Free(BackslashPath);
            }

            if (NT_SUCCESS(status)) {
                if (IsDosPath) {
                    if (! SbieDll_TranslateNtToDosPath(TruePath))
                        TruePath = NULL;
                }
            } else
                TruePath = NULL;

            if (TruePath) {

                ULONG len = (wcslen(TruePath) + 1) * sizeof(WCHAR);
                TruePathResult = Dll_Alloc(len);
                memcpy(TruePathResult, TruePath, len);
            }

            Dll_PopTlsNameBuffer(TlsData);
        }

        if (NtPath != Path)
            Dll_Free(NtPath);
    }

    return TruePathResult;
}


//---------------------------------------------------------------------------
// SbieDll_DeviceChange
//---------------------------------------------------------------------------


_FX void SbieDll_DeviceChange(WPARAM wParam, LPARAM lParam)
{
    static ULONG LastTickCount = 0;
    static ULONG LastWParam    = 0;
    static ULONG LastDriveMask = 0;

    if (wParam == DBT_DEVICEARRIVAL || wParam == DBT_DEVICEREMOVECOMPLETE) {

        DEV_BROADCAST_HDR *hdr = (DEV_BROADCAST_HDR *)lParam;
        if (hdr->dbch_devicetype == DBT_DEVTYP_VOLUME) {
            DEV_BROADCAST_VOLUME *vol = (DEV_BROADCAST_VOLUME *)hdr;
            if (! (vol->dbcv_flags & DBTF_MEDIA)) {

                ULONG ThisTickCount = GetTickCount();
                ULONG ThisDriveMask = vol->dbcv_unitmask;

                if ((wParam != LastWParam) ||
                        (ThisDriveMask != LastDriveMask) ||
                        ((ThisTickCount - LastTickCount) > 500)) {

                    File_InitDrives(ThisDriveMask);
                    Dll_RefreshPathList();
                }

                LastTickCount = ThisTickCount;
                LastWParam    = (ULONG)wParam;
                LastDriveMask = ThisDriveMask;
            }
        }

    } else if ((wParam & 0xFF80) == 0xAA00 && lParam == tzuk) { // see NetApi_NetUseAdd

        UCHAR drive_number = (UCHAR)(wParam & 0x1F);
        if (drive_number < 26) {
            File_InitDrives(1 << drive_number);
            Dll_RefreshPathList();
        }

    } else if (wParam == 'sb' && lParam == 0) {

        Dll_RefreshPathList();
    }
}


//---------------------------------------------------------------------------
// SbieDll_QueryFileAttributes
//---------------------------------------------------------------------------


BOOL SbieDll_QueryFileAttributes(const WCHAR *NtPath, ULONG64 *size, ULONG64 *date, ULONG *attrs)
{
    NTSTATUS status;
    UNICODE_STRING uni;
    OBJECT_ATTRIBUTES objattrs;
    FILE_NETWORK_OPEN_INFORMATION info;

    uni.Buffer = (WCHAR *)NtPath;
    uni.Length = wcslen(NtPath) * sizeof(WCHAR);
    uni.MaximumLength = uni.Length + sizeof(WCHAR);

    InitializeObjectAttributes(
        &objattrs, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

    if(__sys_NtQueryFullAttributesFile)
        status = __sys_NtQueryFullAttributesFile(&objattrs, &info);
    else
        status = NtQueryFullAttributesFile(&objattrs, &info);

    if (! NT_SUCCESS(status))
        return FALSE;

    if(size) *size = info.EndOfFile.QuadPart;
    if(date) *date = info.LastWriteTime.QuadPart;
    if(attrs) *attrs = info.FileAttributes;
    return TRUE;
}


// We don't want calls to StopTailCallOptimization to be optimized away
#pragma optimize("", off)

_FX NTSTATUS StopTailCallOptimization(NTSTATUS status)
{
    return status;
}

// $Workaround$ - 3rd party fix
_FX BOOLEAN DigitalGuardian_Init(HMODULE hModule)
{
    Dll_DigitalGuardian = hModule;

    return TRUE;
}
