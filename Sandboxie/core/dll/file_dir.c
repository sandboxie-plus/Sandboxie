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
// File (Dir)
//---------------------------------------------------------------------------


#include "common/pool.h"


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _FILE_MERGE_CACHE_FILE {

    LIST_ELEM list_elem;
    ULONG info_len;
    UNICODE_STRING name_uni;
    FILE_ID_BOTH_DIR_INFORMATION info;
    // ... space for filename immediately following

} FILE_MERGE_CACHE_FILE;


typedef struct _FILE_MERGE_FILE {

    HANDLE handle;
    FILE_ID_BOTH_DIR_INFORMATION *info;
    ULONG info_len;
    WCHAR *name;
    ULONG name_max_len;
    UNICODE_STRING name_uni;
    BOOLEAN have_entry;
	BOOLEAN saved_have_entry;
    BOOLEAN more_files;
    BOOLEAN RestartScan;
    BOOLEAN no_file_ids;
    POOL *cache_pool;
    LIST cache_list;
	ULONG scram_key;

} FILE_MERGE_FILE;


typedef struct _FILE_MERGE {

    LIST_ELEM list_elem;

    HANDLE handle;
    BOOLEAN cant_merge;
    BOOLEAN first_request;

    UNICODE_STRING file_mask;
	FILE_MERGE_FILE* files; // copy file, snapshot_1 file, snapshot_2 file, ..., true file
	ULONG files_count;
	FILE_MERGE_FILE* true_ptr;

    ULONG name_len;     // in bytes, excluding NULL
    WCHAR name[0];

} FILE_MERGE;


typedef struct _FILE_APC {

    PIO_APC_ROUTINE routine;
    void *context;
    IO_STATUS_BLOCK *IoStatusBlock;

} FILE_APC;


typedef struct _FILE_FS_DEVICE_INFORMATION {
    DEVICE_TYPE DeviceType;
    ULONG Characteristics;
} FILE_FS_DEVICE_INFORMATION, *PFILE_FS_DEVICE_INFORMATION;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static void File_InitRecoverFolders(void);

static NTSTATUS File_NtQueryDirectoryFile(
    HANDLE FileHandle,
    HANDLE Event,
    PIO_APC_ROUTINE ApcRoutine,
    void *ApcContext,
    IO_STATUS_BLOCK *IoStatusBlock,
    void *FileInformation,
    ULONG Length,
    FILE_INFORMATION_CLASS FileInformationClass,
    BOOLEAN ReturnSingleEntry,
    UNICODE_STRING *FileMask,
    BOOLEAN RestartScan);

static NTSTATUS File_NtQueryDirectoryFileEx(
    HANDLE FileHandle,
    HANDLE Event,
    PIO_APC_ROUTINE ApcRoutine,
    void *ApcContext,
    IO_STATUS_BLOCK *IoStatusBlock,
    void *FileInformation,
    ULONG Length,
    FILE_INFORMATION_CLASS FileInformationClass,
    BOOLEAN ReturnSingleEntry,
    UNICODE_STRING *FileMask);

static void File_ApcStub(ULONG_PTR context);

static NTSTATUS File_Merge(
    HANDLE FileHandle, WCHAR *TruePath, WCHAR *CopyPath,
    BOOLEAN RestartScan, WCHAR **FileMask, FILE_MERGE **out_merge);

static NTSTATUS File_OpenForMerge(
    FILE_MERGE *merge, WCHAR *TruePath, WCHAR *CopyPath);

static NTSTATUS File_MergeCache(
    FILE_MERGE_FILE *qfile, UNICODE_STRING *FileMask, BOOLEAN ForceCache);

static NTSTATUS File_MergeCacheWin2000(
    FILE_MERGE_FILE *qfile, UNICODE_STRING *FileMask,
    FILE_ID_BOTH_DIR_INFORMATION *info_area, ULONG info_area_len);

static void File_MergeFree(FILE_MERGE *merge);

static NTSTATUS File_GetMergedInformation(
    FILE_MERGE *merge,
    IO_STATUS_BLOCK *IoStatusBlock,
    void *FileInformation,
    ULONG Length,
    FILE_INFORMATION_CLASS FileInformationClass,
    BOOLEAN ReturnSingleEntry);

static NTSTATUS File_GetFullInformation(
    FILE_MERGE_FILE *qfile, UNICODE_STRING *FileMask, BOOLEAN XorFileId);

static WCHAR *File_CopyFixedInformation(
    FILE_ID_BOTH_DIR_INFORMATION *source, void *target,
    FILE_INFORMATION_CLASS FileInformationClass);

static NTSTATUS File_NtClose(HANDLE FileHandle);

static NTSTATUS File_DeleteDirectory(
        const WCHAR *FilePath, BOOLEAN JustCheck);

static NTSTATUS File_MarkChildrenDeleted(const WCHAR *ParentTruePath);

static void File_InitRecoverList(
    const WCHAR *setting, LIST *list, BOOLEAN MustBeValidPath,
    WCHAR *buf, ULONG buf_len);

static void File_NotifyRecover(HANDLE FileHandle, MSG_HEADER **out_req);

static BOOLEAN File_IsRecoverable(const WCHAR *TruePath);

static ULONG File_RtlGetCurrentDirectory_U(ULONG buf_len, WCHAR *buf_ptr);

static NTSTATUS File_RtlSetCurrentDirectory_U(UNICODE_STRING *PathName);

static ULONG File_CallRtlGetFullPathName(
    WCHAR *src, ULONG buf_len, WCHAR *buf_ptr, WCHAR **file_part_ptr);

static ULONG File_RtlGetFullPathName_U(
    WCHAR *src, ULONG buf_len, WCHAR *buf_ptr, WCHAR **file_part_ptr);

static NTSTATUS File_RtlGetFullPathName_UEx(
    WCHAR *src, ULONG buf_len, WCHAR *buf_ptr, WCHAR **file_part_ptr,
    ULONG *ret_len_ptr);

static NTSTATUS File_NtQueryVolumeInformationFile(
    HANDLE FileHandle,
    IO_STATUS_BLOCK *IoStatusBlock,
    PVOID FsInformation,
    ULONG Length,
    ULONG FsInformationClass);

static void File_DoAutoRecover_2(BOOLEAN force, ULONG ticks);

static ULONG File_DoAutoRecover_3(
    const WCHAR *PathToFind, WCHAR *PathBuf1024,
    SYSTEM_HANDLE_INFORMATION *info, FILE_GET_ALL_HANDLES_RPL *rpl,
    UCHAR *FileObjectTypeNumber);

static ULONG File_DoAutoRecover_4(
    const WCHAR *PathToFind, WCHAR *PathBuf1024,
    HANDLE FileHandle, UCHAR ObjectTypeNumber, UCHAR *FileObjectTypeNumber);

NTSTATUS File_NtCloseImpl(HANDLE FileHandle);

//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static LIST File_RecoverFolders;
static LIST File_RecoverIgnores;

static HANDLE *File_RecHandles = NULL;
static LIST File_RecPaths;
static CRITICAL_SECTION File_RecHandles_CritSec;

static CRITICAL_SECTION File_CurDir_CritSec;
static WCHAR *File_CurDir_LastInput = NULL;
static WCHAR *File_CurDir_LastOutput = NULL;

static LIST File_DirHandles;
static CRITICAL_SECTION File_DirHandles_CritSec;

static BOOLEAN File_MsoDllLoaded = FALSE;


//---------------------------------------------------------------------------
// File_NtQueryDirectoryFileEx
// Function added to Windows 10 RS4 build 17035
// Takes one less argument than the non Ex version:  The last argument "RestartScan" has been removed.
// At the dispatch level "RestartScan" is a "don't care" so calling the non-Ex hook is safe.
//---------------------------------------------------------------------------

_FX NTSTATUS File_NtQueryDirectoryFileEx(
    HANDLE FileHandle,
    HANDLE Event,
    PIO_APC_ROUTINE ApcRoutine,
    void *ApcContext,
    IO_STATUS_BLOCK *IoStatusBlock,
    void *FileInformation,
    ULONG Length,
    FILE_INFORMATION_CLASS FileInformationClass,
    BOOLEAN ReturnSingleEntry,
    UNICODE_STRING *FileMask)
{
    return File_NtQueryDirectoryFile(FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, FileInformation, Length, FileInformationClass, ReturnSingleEntry, FileMask, 0);
}

//---------------------------------------------------------------------------
// File_NtQueryDirectoryFile
//---------------------------------------------------------------------------


_FX NTSTATUS File_NtQueryDirectoryFile(
    HANDLE FileHandle,
    HANDLE Event,
    PIO_APC_ROUTINE ApcRoutine,
    void *ApcContext,
    IO_STATUS_BLOCK *IoStatusBlock,
    void *FileInformation,
    ULONG Length,
    FILE_INFORMATION_CLASS FileInformationClass,
    BOOLEAN ReturnSingleEntry,
    UNICODE_STRING *FileMask,
    BOOLEAN RestartScan)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    UNICODE_STRING objname;
    FILE_MERGE *merge;
    WCHAR *file_mask;
    ULONG FileFlags;
    BOOLEAN merge_lock;

    //
    // not a recursive invocation, handle the call here
    //

    merge_lock = FALSE;
    file_mask = NULL;

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    //
    // get the paths for this FileHandle, and try to find the merge
    // entry for them.  if there is only one part of the true/copy pair,
    // then we will get STATUS_BAD_INITIAL_PC, and pass the request
    // to the system
    //

    RtlInitUnicodeString(&objname, L"");

    status = File_GetName(
                    FileHandle, &objname, &TruePath, &CopyPath, &FileFlags);

    //
    // if the caller is trying to query the root directory of a device,
    // we get an error return value, and have to add the trailing backslash
    //

    if (status == STATUS_BAD_INITIAL_PC && CopyPath) {

        WCHAR *ptr = TruePath + wcslen(TruePath);
        ptr[0] = L'\\';
        ptr[1] = L'\0';

        status = STATUS_SUCCESS;
    }

    //
    // check if the path is open or closed
    //

    if (NT_SUCCESS(status)) {

        ULONG mp_flags = File_MatchPath(TruePath, &FileFlags);

        if (PATH_IS_CLOSED(mp_flags))
            status = STATUS_ACCESS_DENIED;

        else if (PATH_IS_WRITE(mp_flags))
            status = STATUS_BAD_INITIAL_PC;

        else if (PATH_IS_OPEN(mp_flags))
            status = STATUS_BAD_INITIAL_PC;
    }

    //
    // if we have STATUS_BAD_INITIAL_PC at this point, the access is either
    // to some non-disk device, or to an open path, so the OS must handle it.
    // any other non-zero status is simply returned as error.
    //

    if (! NT_SUCCESS(status)) {

        if (status == STATUS_BAD_INITIAL_PC) {

            status = __sys_NtQueryDirectoryFile(
                FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock,
                FileInformation, Length, FileInformationClass,
                ReturnSingleEntry, FileMask, RestartScan);
        }

        __leave;
    }

    //
    // capture FileMask
    //

    if (FileMask && FileMask->Length && FileMask->Buffer) {

        ULONG FileMask_len;

        FileMask_len = FileMask->Length & ~1;
        file_mask = Dll_Alloc(FileMask_len + sizeof(WCHAR));
        memcpy(file_mask, FileMask->Buffer, FileMask_len);
        file_mask[FileMask_len / sizeof(WCHAR)] = L'\0';
    }

    //
    // create (or find) a FILE_MERGE structure to serve the request
    //

    if (! ReturnSingleEntry)
        RestartScan = FALSE;

    status = File_Merge(
        FileHandle, TruePath, CopyPath, RestartScan, &file_mask, &merge);

    if (! NT_SUCCESS(status)) {

        if (status == STATUS_BAD_INITIAL_PC) {

            status = __sys_NtQueryDirectoryFile(
                FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock,
                FileInformation, Length, FileInformationClass,
                ReturnSingleEntry, FileMask, RestartScan);
        }

        __leave;
    }

    //
    // we're handling the request here, first check the minimum length
    //

    merge_lock = TRUE;

    status = File_GetMergedInformation(
        merge, IoStatusBlock,
        FileInformation, Length, FileInformationClass, ReturnSingleEntry);

    if (merge->first_request) {

        if (status == STATUS_NO_MORE_FILES)
            status = STATUS_NO_SUCH_FILE;

        merge->first_request = FALSE;
    }

    LeaveCriticalSection(&File_DirHandles_CritSec);
    merge_lock = FALSE;

    if (Event)
        SetEvent(Event);

    if (ApcRoutine) {

        FILE_APC *apc = Dll_Alloc(sizeof(FILE_APC));
        apc->routine = ApcRoutine;
        apc->context = ApcContext;
        apc->IoStatusBlock = IoStatusBlock;
        QueueUserAPC(File_ApcStub, GetCurrentThread(), (ULONG_PTR)apc);
    }

    //
    // finish
    //

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    if (merge_lock)
        LeaveCriticalSection(&File_DirHandles_CritSec);

    if (file_mask)
        Dll_Free(file_mask);

    Dll_PopTlsNameBuffer(TlsData);
    SetLastError(LastError);
    return status;
}


//---------------------------------------------------------------------------
// File_ApcStub
//---------------------------------------------------------------------------


_FX void File_ApcStub(ULONG_PTR context)
{
    FILE_APC *apc = (FILE_APC *)context;
    apc->routine(apc->context, apc->IoStatusBlock, 0);
    Dll_Free(apc);
}


//---------------------------------------------------------------------------
// File_Merge
//---------------------------------------------------------------------------


_FX NTSTATUS File_Merge(
    HANDLE FileHandle, WCHAR *TruePath, WCHAR *CopyPath,
    BOOLEAN RestartScan, WCHAR **FileMask, FILE_MERGE **out_merge)
{
    static const WCHAR *_tsclient = L"\\device\\mup\\tsclient\\";
    NTSTATUS status;
    ULONG TruePath_len;
    FILE_MERGE *merge;

    //
    // if we have information cached for this handle, return it
    //

    TruePath_len = wcslen(TruePath) * sizeof(WCHAR);

    EnterCriticalSection(&File_DirHandles_CritSec);

    merge = List_Head(&File_DirHandles);
    while (merge) {

        FILE_MERGE *next = List_Next(merge);

        if (merge->handle == FileHandle) {

            if ((! RestartScan) &&
                merge->name_len == TruePath_len &&
                _wcsicmp(merge->name, TruePath) == 0) {

                //
                // we found a cached entry for the same handle, and
                // the same file path, so we are going to use it.
                //

                break;

            } else {

                List_Remove(&File_DirHandles, merge);
                File_MergeFree(merge);
            }
        }

        merge = next;
    }

    //
    // if we don't have a merge entry, create one.  it is inserted
    // at the end of the list, so the loop above always gets to
    // look at all existing merge entries and discard stale ones
    //

    if (! merge) {

        merge = Dll_Alloc(sizeof(FILE_MERGE) + TruePath_len + sizeof(WCHAR));
        memzero(merge, sizeof(FILE_MERGE));

		merge->files = Dll_Alloc(sizeof(FILE_MERGE_FILE) * (2 + File_Snapshot_Count));
		memzero(merge->files, sizeof(FILE_MERGE_FILE) * (2 + File_Snapshot_Count));

        merge->handle = FileHandle;
        merge->cant_merge = FALSE;
        merge->first_request = TRUE;

        if (*FileMask) {
            RtlInitUnicodeString(&merge->file_mask, *FileMask);
            *FileMask = NULL;
        }

        merge->name_len = TruePath_len;
        memcpy(merge->name, TruePath, TruePath_len + sizeof(WCHAR));

        if (TruePath_len >= wcslen(_tsclient) * sizeof(WCHAR) &&
                _wcsnicmp(TruePath, _tsclient, wcslen(_tsclient)) == 0) {
            //
            // shares provided by Remote Desktop can't provide file IDs
            //
			merge->files[0].no_file_ids = TRUE;
        }

        if (File_Windows2000) {
            //
            // Windows 2000 SP 4 seems to include support for info class
            // FileIdBothDirectoryInformation, although according to
            // documentation it is only supported on Windows XP and later
            //
			for(ULONG i = 0; i < 2 + File_Snapshot_Count; i++)
				merge->files[i].no_file_ids = TRUE;
        }

        List_Insert_After(&File_DirHandles, NULL, merge);
    }

    //
    // open the directory for the true path
    //

    if (merge->cant_merge) {

        //
        // if cant_merge is set, then we already know that either TruePath
        // or CopyPath exist, but not both, so return special status
        //

        status = STATUS_BAD_INITIAL_PC;

    } else if (!merge->files[0].handle) {

        //
        // open the true and copy directories, if we haven't already.
        // we don't check for merge->true_file.handle, because it is
        // a possible scenario that only merge->copy_file.handle exists
        //

        status = File_OpenForMerge(merge, TruePath, CopyPath);

        if (status == STATUS_BAD_INITIAL_PC)
            merge->cant_merge = TRUE;

    } else

        status = STATUS_SUCCESS;

    //
    // finish
    //

    if (! NT_SUCCESS(status))
        LeaveCriticalSection(&File_DirHandles_CritSec);

    *out_merge = merge;
    return status;
}


//---------------------------------------------------------------------------
// File_OpenForMerge
//---------------------------------------------------------------------------


_FX NTSTATUS File_OpenForMerge(
	FILE_MERGE *merge, WCHAR *TruePath, WCHAR *CopyPath)
{
	NTSTATUS status;
	OBJECT_ATTRIBUTES objattrs;
	UNICODE_STRING objname;
	IO_STATUS_BLOCK IoStatusBlock;
	union {
		FILE_BASIC_INFORMATION basic;
	} info;
	ULONG len;
	WCHAR *ptr;
	// BOOLEAN TruePathIsRoot;
	BOOLEAN TruePathDeleted = FALSE; // indicates that one of the parent snapshots deleted the true directory
	BOOLEAN NoCopyPath = FALSE;

	InitializeObjectAttributes(
		&objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

	//
	// open the copy file
	//

	if (File_CheckDeletedParent(CopyPath))
		return STATUS_OBJECT_PATH_NOT_FOUND;

	RtlInitUnicodeString(&objname, CopyPath);

	status = __sys_NtCreateFile(
		&merge->files[0].handle,
		FILE_GENERIC_READ,              // DesiredAccess
		&objattrs,
		&IoStatusBlock,
		NULL,                           // AllocationSize
		0,                              // FileAttributes
		FILE_SHARE_VALID_FLAGS,         // ShareAccess
		FILE_OPEN,                      // CreateDisposition
		FILE_SYNCHRONOUS_IO_NONALERT,   // CreateOptions
		NULL,                           // EaBuffer
		0);                             // EaLength

	if (NT_SUCCESS(status)) {

		//
		// if the copy file exists, check if it is marked as deleted,
		// and if so, close it and pretend it doesn't exist;  otherwise
		// make sure it is a directory file
		//

		status = __sys_NtQueryInformationFile(
			merge->files[0].handle, &IoStatusBlock, &info,
			sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);

		if (NT_SUCCESS(status)) {

			if (IS_DELETE_MARK(&info.basic.CreationTime)) {

				status = STATUS_OBJECT_NAME_NOT_FOUND;

			}
			else if ((info.basic.FileAttributes &
				FILE_ATTRIBUTE_DIRECTORY) == 0) {

				status = STATUS_INVALID_PARAMETER;
			}
		}

		if (!NT_SUCCESS(status)) {

			__sys_NtClose(merge->files[0].handle);
			merge->files[0].handle = NULL;
			return status;
		}

		//
		// copy file passed all checks;  indicate it is ready for use
		//

		merge->files[0].more_files = TRUE;
		merge->files[0].RestartScan = TRUE;
		merge->files_count++;
	}
	else {

		//
		// if there is no copy file, we don't need to merge anything,
		// and can let the system work directly on the true file
		//

		if (status == STATUS_OBJECT_NAME_NOT_FOUND ||
			status == STATUS_OBJECT_PATH_NOT_FOUND ||
			status == STATUS_ACCESS_DENIED) {

			NoCopyPath = TRUE;
		}
		else
			return status;
	}

	//
	// Now open the parent snapshots if present, and it's aprent and so on....
	//

	if (File_Snapshot != NULL)
	{
		for (FILE_SNAPSHOT* Cur_Snapshot = File_Snapshot; Cur_Snapshot != NULL; Cur_Snapshot = Cur_Snapshot->Parent)
		{
			WCHAR* TmplName = File_MakeSnapshotPath(Cur_Snapshot, CopyPath);
			if (!TmplName)
				break;

			RtlInitUnicodeString(&objname, TmplName);

			status = __sys_NtCreateFile(
				&merge->files[merge->files_count].handle,
				FILE_GENERIC_READ,              // DesiredAccess
				&objattrs,
				&IoStatusBlock,
				NULL,                           // AllocationSize
				0,                              // FileAttributes
				FILE_SHARE_VALID_FLAGS,         // ShareAccess
				FILE_OPEN,                      // CreateDisposition
				FILE_SYNCHRONOUS_IO_NONALERT,   // CreateOptions
				NULL,                           // EaBuffer
				0);                             // EaLength

			if (NT_SUCCESS(status)) {

				//
				// if the copy file exists, check if it is marked as deleted,
				// and if so, close it and pretend it doesn't exist;  otherwise
				// make sure it is a directory file
				//

				// todo reduce redundant code, combine with the code for the copy_file

				status = __sys_NtQueryInformationFile(
					merge->files[merge->files_count].handle, &IoStatusBlock, &info,
					sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);

				if (NT_SUCCESS(status)) {

					if (IS_DELETE_MARK(&info.basic.CreationTime)) {

						status = STATUS_OBJECT_NAME_NOT_FOUND;

					}
					else if ((info.basic.FileAttributes &
						FILE_ATTRIBUTE_DIRECTORY) == 0) {

						status = STATUS_INVALID_PARAMETER;
					}
				}

				if (!NT_SUCCESS(status)) {

					__sys_NtClose(merge->files[merge->files_count].handle);
					merge->files[merge->files_count].handle = NULL;

					TruePathDeleted = TRUE;
					break; // dont look any further
				}

				//
				// copy file passed all checks;  indicate it is ready for use
				//

				merge->files[merge->files_count].more_files = TRUE;
				merge->files[merge->files_count].RestartScan = TRUE;
				merge->files[merge->files_count].scram_key = Cur_Snapshot->ScramKey;
				merge->files_count++;

			}
			else {

				//
				// Ignroe errors here for now // todo
				//

			}
		}
	}

	//
	// if there is no copy file, we don't need to merge anything,
	// and can let the system work directly on the true file
	//

	if (merge->files_count == 0) {

		status = STATUS_BAD_INITIAL_PC;

		return status;
	}

	if (TruePathDeleted)
		goto skip_true_file;

	//
	// true path must end with a backslash, so that we are able to
	// open the root directory of the volume device
	//

	// TruePathIsRoot = FALSE;

	len = wcslen(TruePath) * sizeof(WCHAR);
	if (len > sizeof(WCHAR)) {
		ptr = &TruePath[len / sizeof(WCHAR) - 1];
		if (*ptr != L'\\') {
			ptr[1] = L'\\';
			ptr[2] = L'\0';
			len += sizeof(WCHAR);
		}
		else {
			ptr = NULL;
			// TruePathIsRoot = TRUE;
		}
	}
	else
		ptr = NULL;

	objname.Length = (USHORT)len;
	objname.MaximumLength = objname.Length + sizeof(WCHAR);
	objname.Buffer = TruePath;

	//
	// open the true file
	//

	merge->true_ptr = &merge->files[merge->files_count];

	status = __sys_NtCreateFile(
		&merge->true_ptr->handle,
		FILE_GENERIC_READ,              // DesiredAccess
		&objattrs,
		&IoStatusBlock,
		NULL,                           // AllocationSize
		0,                              // FileAttributes
		FILE_SHARE_VALID_FLAGS,         // ShareAccess
		FILE_OPEN,                      // CreateDisposition
		FILE_SYNCHRONOUS_IO_NONALERT |  // CreateOptions
		FILE_DIRECTORY_FILE,
		NULL,                           // EaBuffer
		0);                             // EaLength

	if (ptr)
		ptr[1] = L'\0';

	//
	// even if the true directory could not be opened because it isn't
	// there, or because it is a file rather than a directory, we still
	// go ahead, and will use only the copy path for the "merge".
	// for any other error opening the true directory, we abort.
	//

	if (!NT_SUCCESS(status)) {

		merge->true_ptr->handle = NULL;
		merge->true_ptr = NULL;

		if (status != STATUS_NOT_A_DIRECTORY &&
			status != STATUS_OBJECT_NAME_NOT_FOUND &&
			status != STATUS_OBJECT_PATH_NOT_FOUND) {

			for (ULONG i = 0; i < merge->files_count; i++) {
				__sys_NtClose(merge->files[i].handle);
				merge->files[i].handle = NULL;
			}

			if (status == STATUS_ACCESS_DENIED)
				status = STATUS_BAD_INITIAL_PC;

			return status;
		}

		status = STATUS_SUCCESS;

	}
	else {

		//
		// true file passed all checks;  indicate it is ready for use
		//

		merge->true_ptr->more_files = TRUE;
		merge->true_ptr->RestartScan = TRUE;
		merge->files_count++;
	}

skip_true_file:

	//
	// now that both copy and true directories were opened, we will need to
	// merge them.  for this to work, we need a sorted directory listing.
	// NTFS is always sorted, but FAT isn't, so cache the listing if needed.
	//
	// note that if we don't have a true handle, we won't merge anything,
	// so do not have to cache in advance.  on the other hand, if the
	// true path is cached, we also have to cache the copy path, to make
	// sure the files will be ordered in the same sequence.  and vice
	// versa: if the copy path is cached, make sure the true path is cached
	//

	if (merge->true_ptr) {

        BOOLEAN ForceCache = FALSE;
        if (merge->name_len >= File_MupLen * sizeof(WCHAR)
                && _wcsnicmp(merge->name, File_Mup, File_MupLen) == 0) {
            //
            // remote shares have all kinds of quirks, for example:
            // returning STATUS_BUFFER_OVERFLOW from NtQueryDirectoryFile
            // used in File_GetFullInformation, but then returning
            // STATUS_NO_MORE_FILES when called with a larger buffer.
            // we work around this by caching everything in advance
            //
            ForceCache = TRUE;
        }

        status = File_MergeCache(
                    merge->true_ptr, &merge->file_mask, ForceCache);

        if (NT_SUCCESS(status)) {

            BOOLEAN HaveTrueCache = (merge->true_ptr->cache_pool != NULL);
			BOOLEAN HaveCopyCache = FALSE;

			for (ULONG i = 0; i < merge->files_count - 1; i++) {

				status = File_MergeCache(
					&merge->files[i], &merge->file_mask, HaveTrueCache);

				if (NT_SUCCESS(status) && merge->files[i].cache_pool != NULL)
					HaveCopyCache = TRUE;
			}

            if (!HaveTrueCache && HaveCopyCache) {

                status = File_MergeCache(
                            merge->true_ptr, &merge->file_mask, TRUE);
            }
        }

        if (! NT_SUCCESS(status)) {

			for (ULONG i = 0; i < merge->files_count; i++) {
				if (merge->files[i].handle) {
					__sys_NtClose(merge->files[i].handle);
					merge->files[i].handle = NULL;
				}
			}
        }
    }

	return status;
}


//---------------------------------------------------------------------------
// File_MergeCache
//---------------------------------------------------------------------------


_FX NTSTATUS File_MergeCache(
    FILE_MERGE_FILE *qfile, UNICODE_STRING *FileMask, BOOLEAN ForceCache)
{
    NTSTATUS status;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_ATTRIBUTE_TAG_INFORMATION taginfo;
    FILE_ID_BOTH_DIR_INFORMATION *info_area;
    FILE_ID_BOTH_DIR_INFORMATION *info_ptr;
    LIST *cache_list;
    FILE_MERGE_CACHE_FILE *cache_file;
    FILE_MERGE_CACHE_FILE *ins_point;
    ULONG len;
    const ULONG INFO_AREA_LEN = 0x10000;  // the size used by cmd.exe

    //
    // ZwQueryDirectoryFile for NTFS returns the entries sorted by ascending
    // order of filename.  for FAT there is no guarantee of such a sort
    // order.  but we need a sorted directory listing for the directory
    // merging to work.  on FAT, the following ZwQueryInformationFile will
    // fail, and this is how we can tell NTFS apart from FAT
    //

    if (qfile->cache_pool) {
        Pool_Delete(qfile->cache_pool);
        qfile->cache_pool = NULL;
    }

    if (! ForceCache) {

        status = __sys_NtQueryInformationFile(
            qfile->handle, &IoStatusBlock,
            &taginfo, sizeof(taginfo), FileAttributeTagInformation);

        if (status != STATUS_INVALID_PARAMETER &&
                status != STATUS_NOT_IMPLEMENTED)
            return status;
    }

    //
    // prepare the cache pool
    //

    qfile->cache_pool = Pool_Create();
    if (! qfile->cache_pool)
        return STATUS_INSUFFICIENT_RESOURCES;

    cache_list = &qfile->cache_list;
    List_Init(cache_list);

    info_area = Pool_Alloc(qfile->cache_pool, INFO_AREA_LEN);
    if (! info_area)
        return STATUS_INSUFFICIENT_RESOURCES;

    //
    // read entire directory, build a sorted files list
    //

    while (1) {

        info_area->NextEntryOffset = tzuk;

        status = __sys_NtQueryDirectoryFile(
            qfile->handle,
            NULL, NULL, NULL,   // Event, ApcRoutine, ApcContext
            &IoStatusBlock,
            info_area, INFO_AREA_LEN,
            FileIdBothDirectoryInformation,
            FALSE,              // ReturnSingleEntry
            FileMask,
            qfile->RestartScan);

        if (status == STATUS_BUFFER_OVERFLOW &&
                info_area->NextEntryOffset != tzuk) {
            // we got STATUS_BUFFER_OVERFLOW but buffer was filled
            status = STATUS_SUCCESS;
        }

        if (! NT_SUCCESS(status)) {

            if (status == STATUS_INVALID_PARAMETER  ||
                status == STATUS_INVALID_INFO_CLASS ||
                status == STATUS_INVALID_LEVEL ) {

                //
                // can't get file ids, try secondary approach
                // NetApp drive returns STATUS_INVALID_LEVEL error
                //

                status = File_MergeCacheWin2000(qfile, FileMask,
                                                info_area, INFO_AREA_LEN);
            }

            break;
        }

        qfile->RestartScan = FALSE;

        info_ptr = info_area;
        while (1) {
            int cmp;

            len = sizeof(FILE_MERGE_CACHE_FILE)
                + info_ptr->FileNameLength;

            cache_file = Pool_Alloc(qfile->cache_pool, len);
            if (! cache_file) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            len = sizeof(FILE_ID_BOTH_DIR_INFORMATION)
                - sizeof(WCHAR)     // the [1] from FileName[1]
                + info_ptr->FileNameLength;
            memcpy(&cache_file->info, info_ptr, len);
            cache_file->info.NextEntryOffset = 0;
            cache_file->info_len = len;

            cache_file->name_uni.Length = (USHORT)info_ptr->FileNameLength;
            cache_file->name_uni.MaximumLength = cache_file->name_uni.Length;
            cache_file->name_uni.Buffer = cache_file->info.FileName;

            // insert file into the ordered list

            ins_point = List_Head(cache_list);
            cmp = -1;
            while (ins_point) {
                cmp = RtlCompareUnicodeString(
                    &ins_point->name_uni, &cache_file->name_uni,
                    TRUE);                      // CaseInSensitive
                if ( (cmp > 0) || (cmp == 0) )
                    break;
                ins_point = List_Next(ins_point);
            }
            // There is a bug with Isilon drives.  NtQueryDirectoryFile does not return STATUS_NO_MORE_FILES but always returns STATUS_SUCCESS with the same file name.
            // This causes an infinite loop in this code.  So, if the name_uni we just received is the same as what we just added to the list, assume it is the Isilon bug
            // and break out of this loop.
            if (cmp == 0)
            {
                status = STATUS_NO_MORE_FILES;
                break;
            }

            if (ins_point)
                List_Insert_Before(cache_list, ins_point, cache_file);
            else
                List_Insert_After(cache_list, NULL, cache_file);

            // process next file

            if (info_ptr->NextEntryOffset == 0)
                break;
            info_ptr = (FILE_ID_BOTH_DIR_INFORMATION *)
                ((UCHAR *)info_ptr + info_ptr->NextEntryOffset);
        }

        if (! NT_SUCCESS(status))
            break;
    }

    if (status == STATUS_NO_MORE_FILES || status == STATUS_NO_SUCH_FILE)
        status = STATUS_SUCCESS;

    Pool_Free(info_area, INFO_AREA_LEN);

    return status;
}


//---------------------------------------------------------------------------
// File_MergeCacheWin2000
//---------------------------------------------------------------------------


_FX NTSTATUS File_MergeCacheWin2000(
    FILE_MERGE_FILE *qfile, UNICODE_STRING *FileMask,
    FILE_ID_BOTH_DIR_INFORMATION *info_area, ULONG info_area_len)
{
    NTSTATUS status;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_BOTH_DIRECTORY_INFORMATION *info_ptr;
    LIST *cache_list;
    FILE_MERGE_CACHE_FILE *cache_file;
    FILE_MERGE_CACHE_FILE *ins_point;
    ULONG len;

    //
    // on Windows 2000, the query for FileIdBothDirectoryInformation in
    // File_MergeCache may not work, here use an alternative info class
    // that doesn't include the FileId field
    //
    // note that this function is useful also in more recent editions of
    // Windows, as some filesystems or filesystem filters (like CafeAgent)
    // will fail calls using FileIdBothDirectoryInformation information,
    // see File_GetFullInformation
    //

    cache_list = &qfile->cache_list;

    //
    // read entire directory, build a sorted files list
    //

    while (1) {

        info_area->NextEntryOffset = tzuk;

        status = __sys_NtQueryDirectoryFile(
            qfile->handle,
            NULL, NULL, NULL,   // Event, ApcRoutine, ApcContext
            &IoStatusBlock,
            info_area, info_area_len,
            FileBothDirectoryInformation,   // no FileId
            FALSE,              // ReturnSingleEntry
            FileMask,
            qfile->RestartScan);

        if (status == STATUS_BUFFER_OVERFLOW &&
                info_area->NextEntryOffset != tzuk) {
            // we got STATUS_BUFFER_OVERFLOW but buffer was filled
            status = STATUS_SUCCESS;
        }

        if (! NT_SUCCESS(status))
            break;

        qfile->RestartScan = FALSE;

        info_ptr = (FILE_BOTH_DIRECTORY_INFORMATION *)info_area;
        while (1) {

            len = sizeof(FILE_MERGE_CACHE_FILE)
                + info_ptr->FileNameLength;

            cache_file = Pool_Alloc(qfile->cache_pool, len);
            if (! cache_file) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            //
            // FILE_ID_BOTH_DIR ... and FILE_BOTH_DIRECTORY ... begin with
            // the same layout, but differ towards the end, so we can copy
            // the common part and then add the final FileName
            //

            memcpy(&cache_file->info, info_ptr,
                   FIELD_OFFSET(FILE_ID_BOTH_DIR_INFORMATION, FileId));

            cache_file->info.FileId.QuadPart = 0;

            memcpy(&cache_file->info.FileName,
                   info_ptr->FileName, info_ptr->FileNameLength);

            len = sizeof(FILE_ID_BOTH_DIR_INFORMATION)
                - sizeof(WCHAR)     // the [1] from FileName[1]
                + info_ptr->FileNameLength;

            cache_file->info.NextEntryOffset = 0;
            cache_file->info_len = len;

            cache_file->name_uni.Length = (USHORT)info_ptr->FileNameLength;
            cache_file->name_uni.MaximumLength = cache_file->name_uni.Length;
            cache_file->name_uni.Buffer = cache_file->info.FileName;

            // insert file into the ordered list

            ins_point = List_Head(cache_list);
            while (ins_point) {
                int cmp = RtlCompareUnicodeString(
                    &ins_point->name_uni, &cache_file->name_uni,
                    TRUE);                      // CaseInSensitive
                if (cmp > 0)
                    break;
                ins_point = List_Next(ins_point);
            }

            if (ins_point)
                List_Insert_Before(cache_list, ins_point, cache_file);
            else
                List_Insert_After(cache_list, NULL, cache_file);

            // process next file

            if (info_ptr->NextEntryOffset == 0)
                break;
            info_ptr = (FILE_BOTH_DIRECTORY_INFORMATION *)
                ((UCHAR *)info_ptr + info_ptr->NextEntryOffset);
        }

        if (! NT_SUCCESS(status))
            break;
    }

    return status;
}


//---------------------------------------------------------------------------
// File_MergeFree
//---------------------------------------------------------------------------


_FX void File_MergeFree(FILE_MERGE *merge)
{
	if (merge->files)
	{
		for (ULONG i = 0; i < merge->files_count; i++)
		{
			if (merge->files[i].handle)
				__sys_NtClose(merge->files[i].handle);
			if (merge->files[i].info)
				Dll_Free(merge->files[i].info);
			if (merge->files[i].name)
				Dll_Free(merge->files[i].name);
			if (merge->files[i].cache_pool)
				Pool_Delete(merge->files[i].cache_pool);
		}

		Dll_Free(merge->files);
	}

    if (merge->file_mask.Buffer)
        Dll_Free(merge->file_mask.Buffer);
    Dll_Free(merge);
}


//---------------------------------------------------------------------------
// File_GetMergedInformation
//---------------------------------------------------------------------------


_FX NTSTATUS File_GetMergedInformation(
    FILE_MERGE *merge,
    IO_STATUS_BLOCK *IoStatusBlock,
    void *FileInformation,
    ULONG Length,
    FILE_INFORMATION_CLASS FileInformationClass,
    BOOLEAN ReturnSingleEntry)
{
	NTSTATUS status = STATUS_SUCCESS;
    ULONG info_entry_length;
    FILE_ID_BOTH_DIR_INFORMATION *ptr_info;
    PVOID prev_entry;
    PVOID next_entry;
    WCHAR *name_ptr;
    ULONG len;

    info_entry_length = 0;
    if (FileInformationClass == FileDirectoryInformation)
        info_entry_length = sizeof(FILE_DIRECTORY_INFORMATION);
    else if (FileInformationClass == FileFullDirectoryInformation)
        info_entry_length = sizeof(FILE_FULL_DIRECTORY_INFORMATION);
    else if (FileInformationClass == FileBothDirectoryInformation)
        info_entry_length = sizeof(FILE_BOTH_DIRECTORY_INFORMATION);
    else if (FileInformationClass == FileNamesInformation)
        info_entry_length = sizeof(FILE_NAMES_INFORMATION);
    else if (FileInformationClass == FileIdBothDirectoryInformation)
        info_entry_length = sizeof(FILE_ID_BOTH_DIR_INFORMATION);
    else if (FileInformationClass == FileIdFullDirectoryInformation)
        info_entry_length = sizeof(FILE_ID_FULL_DIR_INFORMATION);
    else
        return STATUS_INVALID_INFO_CLASS;

    if (info_entry_length > Length)
        return STATUS_INFO_LENGTH_MISMATCH;

    prev_entry = FileInformation;
    next_entry = FileInformation;

    IoStatusBlock->Information = 0;     // reset count of bytes written

    while (1) {

        // get directory entries from both directories

		for (ULONG i = 0; i < merge->files_count && NT_SUCCESS(status); i++)
		{
			status = File_GetFullInformation(
				&merge->files[i], &merge->file_mask, TRUE);
		}
        if (! NT_SUCCESS(status))
            break;

        // find where we need to copy the next directory entry from:
        // merge the directories in a sorted order, but prefer to
        // take info from the copy directory if a file exists in both

        ptr_info = NULL;
		for (ULONG i = 0; i < merge->files_count; i++)
			merge->files[i].saved_have_entry = merge->files[i].have_entry;

        /*if (merge->files[0].have_entry &&      // both directories
			merge->true_ptr && merge->true_ptr->have_entry) {      // have an entry

            int cmp = RtlCompareUnicodeString(
                      &merge->true_ptr->name_uni,
                      &merge->files[0].name_uni,
                      TRUE);                    // CaseInSensitive

            if (cmp < 0) {  // true name sorts before copy name
                ptr_info = merge->true_ptr->info;
                merge->true_ptr->have_entry = FALSE;
            } else {        // true name equal to or after copy name
                ptr_info = merge->files[0].info;
                merge->files[0].have_entry = FALSE;
                if (cmp == 0)   // equal
                    merge->true_ptr->have_entry = FALSE;
            }

        } else if (merge->files[0].have_entry) {   // only copy
            merge->files[0].have_entry = FALSE;
            ptr_info = merge->files[0].info;

        } else if (merge->true_ptr && merge->true_ptr->have_entry) {   // only true
            ptr_info = merge->true_ptr->info;
			merge->true_ptr->have_entry = FALSE;
        }*/

		FILE_MERGE_FILE* best = &merge->files[0];

		for (ULONG i = 1; i < merge->files_count; i++) {

			FILE_MERGE_FILE* cur = &merge->files[i];

			if (!best->have_entry) {
				best = cur;
			}
			else if (cur->have_entry) {

				int cmp = RtlCompareUnicodeString(&best->name_uni, &cur->name_uni, TRUE); // CaseInSensitive

				if (cmp == 0) // equal - same file in booth, use newer (best)
					cur->have_entry = FALSE;
				else if (cmp > 0)
					best = cur;
			}
		}

		if (best->have_entry) {
            ptr_info = best->info;
			best->have_entry = FALSE;
        }

		// if the entry found was in the copy directory, then the file
		// may be marked deleted (see Filesys_Mark_File_Deleted for
		// details).  if it is marked so, we pretend this entry does
		// not exist by fetching the following one

		if (ptr_info && (!merge->true_ptr || ptr_info != merge->true_ptr->info) &&
			IS_DELETE_MARK(&ptr_info->CreationTime))
			continue;

        // if both directories are exhausted, reset the
        // NextEntryOffset field of FILE_*_INFORMATION to
        // indicate no more entries, and return status

        if (! ptr_info) {
            if (next_entry == FileInformation)
                status = STATUS_NO_MORE_FILES;
            else
                status = STATUS_SUCCESS;
            *(ULONG *)prev_entry = 0;   // reset NextEntryOffset
            break;
        }

        //
        // make sure caller has enough room in output buffer for the
        // entry.  for the first entry in the buffer, it is ok to have
        // room for only the entry header, excluding filename, but
        // less than that and we will return BUFFER_OVERFLOW.  for
        // later entries in a large buffer (second and on), we must
        // have room for the entry including filename, or we stop here
        // and let the entry be copied the next time around
        //

        len = info_entry_length
            - sizeof(WCHAR);        // the [1] from FileName[1]
        if (next_entry != FileInformation)
            len += ptr_info->FileNameLength;

        if ((UCHAR *)next_entry - (UCHAR *)FileInformation + len > Length) {

            // current entries have not been used yet,
            // reset flags so they are used again next time
			for (ULONG i = 0; i < merge->files_count; i++)
				merge->files[i].have_entry = merge->files[i].saved_have_entry;

            *(ULONG *)prev_entry = 0;   // reset NextEntryOffset

            if (next_entry == FileInformation)
                return STATUS_BUFFER_OVERFLOW;
            else
                break;
        }

        //
        // by now we've selected which source buffer to use (from true
        // file or from copy file), and verified the buffer is large
        // enough to contain at least the fixed portion of the data.
        // now copy the fields from the source buffer and place them
        // in the caller's output buffer
        //

        // source=ptr_info
        // target=next_entry
        //

        name_ptr = File_CopyFixedInformation(
            ptr_info, next_entry, FileInformationClass);

        // copy as much of the filename as there is room available
        // in the caller's buffer.  note that for the second and
        // later entries in a large buffer we already checked that
        // there is enough room for whole entry including filename

        len = (UCHAR *)next_entry - (UCHAR *)FileInformation
            + info_entry_length
            - sizeof(WCHAR)     // the [1] from FileName[1]
            + ptr_info->FileNameLength;
        if (len > Length)
            len = ptr_info->FileNameLength - (len - Length);
        else
            len = ptr_info->FileNameLength;

        memcpy(name_ptr, ptr_info->FileName, len);

        // the following condition can only be met for the first
        // entry in the buffer, so if we can't complete even
        // the first one, we want to say BUFFER_OVERFLOW

        if (len < ptr_info->FileNameLength) {

            // current entries have not gotten used yet,
            // reset flags so they are used again next time
			for (ULONG i = 0; i < merge->files_count; i++)
				merge->files[i].have_entry = merge->files[i].saved_have_entry;

            *(ULONG *)prev_entry = 0;   // reset NextEntryOffset

            status = STATUS_BUFFER_OVERFLOW;
            break;
        }

        prev_entry = next_entry;
        (UCHAR *)next_entry += *(ULONG *)next_entry;    // NextEntryOffset

        if (ReturnSingleEntry ||
            (UCHAR *)next_entry + info_entry_length >
                (UCHAR *)FileInformation + Length) {
            *(ULONG *)prev_entry = 0;   // reset NextEntryOffset
            break;
        }
    }

    IoStatusBlock->Status = status;
    IoStatusBlock->Information =        // number of bytes written
        (UCHAR *)next_entry - (UCHAR *)FileInformation;

    return status;
}


//---------------------------------------------------------------------------
// File_GetFullInformation
//---------------------------------------------------------------------------


_FX NTSTATUS File_GetFullInformation(
    FILE_MERGE_FILE *qfile, UNICODE_STRING *FileMask, BOOLEAN XorFileId)
{
    NTSTATUS status;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_MERGE_CACHE_FILE *cache_file;

    // if we don't currently have a directory entry, get one

    if ((! qfile->have_entry) && qfile->more_files) {

        while (1) {

            if (! qfile->info) {
                qfile->info_len += 256;
                qfile->info = Dll_Alloc(qfile->info_len);
            }

            // if we have pre-cached the entire directory listing,
            // return a cache entry;  otherwise use ZwQueryDirectoryFile

            if (qfile->cache_pool) {

                cache_file = List_Head(&qfile->cache_list);
                if (cache_file) {
                    if (qfile->info_len >= cache_file->info_len) {
                        memcpy(qfile->info, &cache_file->info,
                               cache_file->info_len);
                        List_Remove(&qfile->cache_list, cache_file);
                        status = STATUS_SUCCESS;
                    } else
                        status = STATUS_BUFFER_OVERFLOW;
                } else
                    status = STATUS_NO_MORE_FILES;

            } else {

                ULONG info_class = FileIdBothDirectoryInformation;
                if (qfile->no_file_ids)
                    info_class = FileBothDirectoryInformation;

                status = __sys_NtQueryDirectoryFile(
                    qfile->handle,
                    NULL, NULL, NULL,   // Event, ApcRoutine, ApcContext
                    &IoStatusBlock,
                    qfile->info, qfile->info_len,
                    info_class,         // File(Id)BothDirectoryInformation
                    TRUE,               // ReturnSingleEntry
                    FileMask,
                    qfile->RestartScan);

                //
                // support for shares that don't support the info class
                // FileIdBothDirectoryInformation, and support scrambling
                // the FileID for files in the sandbox
                //

                if (NT_SUCCESS(status)) {

                    FILE_ID_BOTH_DIR_INFORMATION *dst = qfile->info;

                    if (qfile->no_file_ids) {

                        FILE_BOTH_DIRECTORY_INFORMATION *src =
                            (FILE_BOTH_DIRECTORY_INFORMATION *)qfile->info;
                        memmove(dst->FileName,
                                src->FileName, src->FileNameLength);
                        dst->FileId.QuadPart = 0;
                    }

                    //
                    // see File_NtQueryInformationFile for a discussion
                    // about why we have to scramble the sandbox FileId
                    //

                    if (XorFileId && dst->FileId.QuadPart) {

                        dst->FileId.LowPart  ^= 0xFFFFFFFF;
                        dst->FileId.HighPart ^= 0xFFFFFFFF;
                    }

                //
                // if we failed to get information using file ids, then
                // try an alternative approach without file ids, see also
                // File_MergeCache and File_MergeCacheWin2000
                // NetApp drive returns STATUS_INVALID_LEVEL error
                //

                } else if ((status == STATUS_INVALID_PARAMETER ||
                            status == STATUS_INVALID_LEVEL     ||
                            status == STATUS_INVALID_INFO_CLASS)
                    && (info_class == FileIdBothDirectoryInformation)) {

                    NTSTATUS status2;
                    qfile->no_file_ids = TRUE;
                    status2 = File_MergeCache(qfile, FileMask, TRUE);
                    if (NT_SUCCESS(status2))
                        continue;
                }
            }

			//
			// Scramble the short file name to ensure each snapshot has unique short names
			//

			if (NT_SUCCESS(status) && qfile->scram_key && qfile->info->ShortNameLength > 0)
				File_ScrambleShortName(qfile->info->ShortName, &qfile->info->ShortNameLength, qfile->scram_key);

            if (status == STATUS_BUFFER_OVERFLOW) {

                Dll_Free(qfile->info);
                qfile->info = NULL;
                continue;

            } else {
                qfile->RestartScan = FALSE;
                break;
            }
        }

        if (NT_SUCCESS(status)) {

            // we got an entry, now copy it into a null-terminated
            // unicode-string pointed to by the name member of the
            // FILESYS_DIR_QUERY_FILE structure.  make sure that
            // buffer is large enough to hold the name.

            if (qfile->name_max_len <
                qfile->info->FileNameLength + sizeof(WCHAR)) {
                if (qfile->name)
                    Dll_Free(qfile->name);
                qfile->name_max_len =
                    qfile->info->FileNameLength + sizeof(WCHAR);
                qfile->name = Dll_Alloc(qfile->name_max_len);
            }

            if (qfile->name) {
                memcpy(qfile->name, qfile->info->FileName,
                       qfile->info->FileNameLength);
                qfile->name[qfile->info->FileNameLength / sizeof(WCHAR)] = 0;

                qfile->name_uni.Length =
                    (USHORT)qfile->info->FileNameLength;
                qfile->name_uni.MaximumLength =
                    (USHORT)(qfile->name_uni.Length + sizeof(WCHAR));
                qfile->name_uni.Buffer = qfile->name;

                qfile->have_entry = TRUE;
            }

        } else {

            // we did not get an entry from ZwQueryDirectoryFile.
            // if it was because the file listing is really finished,
            // we don't consider that an error, but just mark this
            // fact in the FILE_MERGE_FILE structure.

            qfile->more_files = FALSE;

            if (status == STATUS_NO_MORE_FILES ||
                status == STATUS_NO_SUCH_FILE)
                status = STATUS_SUCCESS;
        }

    } else  // no entry and no more files
        status = STATUS_SUCCESS;

    return status;
}


//---------------------------------------------------------------------------
// File_CopyFixedInformation
//---------------------------------------------------------------------------


_FX WCHAR *File_CopyFixedInformation(
    FILE_ID_BOTH_DIR_INFORMATION *source, void *target,
    FILE_INFORMATION_CLASS FileInformationClass)
{

#define COPY_ULONG(y)          tmp_info->y = source->y;
#define COPY_LARGE_INTEGER(y)  tmp_info->y.QuadPart = source->y.QuadPart
#define COPY_BYTES(y,n)        memcpy(tmp_info->y, source->y, n)
#define COPY_ARRAY(y)          COPY_BYTES(y,sizeof(tmp_info->y))

    //
    // FileDirectoryInformation
    //

    if (FileInformationClass == FileDirectoryInformation) {

        FILE_DIRECTORY_INFORMATION *tmp_info =
            (FILE_DIRECTORY_INFORMATION *)target;

        tmp_info->NextEntryOffset =
            FIELD_OFFSET(FILE_DIRECTORY_INFORMATION, FileName)
            + source->FileNameLength;

        COPY_ULONG(FileIndex);
        COPY_LARGE_INTEGER(CreationTime);
        COPY_LARGE_INTEGER(LastAccessTime);
        COPY_LARGE_INTEGER(LastWriteTime);
        COPY_LARGE_INTEGER(ChangeTime);
        COPY_LARGE_INTEGER(EndOfFile);
        COPY_LARGE_INTEGER(AllocationSize);
        COPY_ULONG(FileAttributes);
        COPY_ULONG(FileNameLength);

        return tmp_info->FileName;

    //
    // FileFullDirectoryInformation
    //

    } else if (FileInformationClass == FileFullDirectoryInformation) {

        FILE_FULL_DIRECTORY_INFORMATION *tmp_info =
            (FILE_FULL_DIRECTORY_INFORMATION *)target;

        tmp_info->NextEntryOffset =
            FIELD_OFFSET(FILE_FULL_DIRECTORY_INFORMATION, FileName)
            + source->FileNameLength;

        COPY_ULONG(FileIndex);
        COPY_LARGE_INTEGER(CreationTime);
        COPY_LARGE_INTEGER(LastAccessTime);
        COPY_LARGE_INTEGER(LastWriteTime);
        COPY_LARGE_INTEGER(ChangeTime);
        COPY_LARGE_INTEGER(EndOfFile);
        COPY_LARGE_INTEGER(AllocationSize);
        COPY_ULONG(FileAttributes);
        COPY_ULONG(FileNameLength);
        COPY_ULONG(EaInformationLength);

        return tmp_info->FileName;

    //
    // FileBothDirectoryInformation
    //

    } else if (FileInformationClass == FileBothDirectoryInformation) {

        FILE_BOTH_DIRECTORY_INFORMATION *tmp_info =
            (FILE_BOTH_DIRECTORY_INFORMATION *)target;

        tmp_info->NextEntryOffset =
            FIELD_OFFSET(FILE_BOTH_DIRECTORY_INFORMATION, FileName)
            + source->FileNameLength;

        COPY_ULONG(FileIndex);
        COPY_LARGE_INTEGER(CreationTime);
        COPY_LARGE_INTEGER(LastAccessTime);
        COPY_LARGE_INTEGER(LastWriteTime);
        COPY_LARGE_INTEGER(ChangeTime);
        COPY_LARGE_INTEGER(EndOfFile);
        COPY_LARGE_INTEGER(AllocationSize);
        COPY_ULONG(FileAttributes);
        COPY_ULONG(FileNameLength);
        COPY_ULONG(EaInformationLength);
        COPY_ULONG(ShortNameLength);
        COPY_ARRAY(ShortName);

        return tmp_info->FileName;

    //
    // FileNamesInformation
    //

    } else if (FileInformationClass == FileNamesInformation) {

        FILE_NAMES_INFORMATION *tmp_info =
            (FILE_NAMES_INFORMATION *)target;

        tmp_info->NextEntryOffset =
            FIELD_OFFSET(FILE_NAMES_INFORMATION, FileName)
            + source->FileNameLength;

        COPY_ULONG(FileIndex);
        COPY_ULONG(FileNameLength);

        return tmp_info->FileName;

    //
    // FileIdBothDirectoryInformation
    //

    } else if (FileInformationClass == FileIdBothDirectoryInformation) {

        FILE_ID_BOTH_DIR_INFORMATION *tmp_info =
            (FILE_ID_BOTH_DIR_INFORMATION *)target;

        tmp_info->NextEntryOffset =
            FIELD_OFFSET(FILE_ID_BOTH_DIR_INFORMATION, FileName)
            + source->FileNameLength;

        COPY_ULONG(FileIndex);
        COPY_LARGE_INTEGER(CreationTime);
        COPY_LARGE_INTEGER(LastAccessTime);
        COPY_LARGE_INTEGER(LastWriteTime);
        COPY_LARGE_INTEGER(ChangeTime);
        COPY_LARGE_INTEGER(EndOfFile);
        COPY_LARGE_INTEGER(AllocationSize);
        COPY_ULONG(FileAttributes);
        COPY_ULONG(FileNameLength);
        COPY_ULONG(EaInformationLength);
        COPY_ULONG(ShortNameLength);
        COPY_ARRAY(ShortName);
        COPY_LARGE_INTEGER(FileId);

        return tmp_info->FileName;

    //
    // FileIdFullDirectoryInformation
    //

    } else if (FileInformationClass == FileIdFullDirectoryInformation) {

        FILE_ID_FULL_DIR_INFORMATION *tmp_info =
            (FILE_ID_FULL_DIR_INFORMATION *)target;

        tmp_info->NextEntryOffset =
            FIELD_OFFSET(FILE_ID_FULL_DIR_INFORMATION, FileName)
            + source->FileNameLength;

        COPY_ULONG(FileIndex);
        COPY_LARGE_INTEGER(CreationTime);
        COPY_LARGE_INTEGER(LastAccessTime);
        COPY_LARGE_INTEGER(LastWriteTime);
        COPY_LARGE_INTEGER(ChangeTime);
        COPY_LARGE_INTEGER(EndOfFile);
        COPY_LARGE_INTEGER(AllocationSize);
        COPY_ULONG(FileAttributes);
        COPY_ULONG(FileNameLength);
        COPY_ULONG(EaInformationLength);
        COPY_LARGE_INTEGER(FileId);

        return tmp_info->FileName;
    }

    return NULL;

#undef COPY_ULONG
#undef COPY_LARGE_INTEGER
#undef COPY_BYTES
#undef COPY_ARRAY
}


//---------------------------------------------------------------------------
// File_NtClose
//---------------------------------------------------------------------------


_FX NTSTATUS File_NtClose(HANDLE FileHandle)
{
    NTSTATUS status;

    __try {
        status = File_NtCloseImpl(FileHandle);
    }
    __except (GetExceptionCode() == EXCEPTION_INVALID_HANDLE ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        return STATUS_INVALID_HANDLE;
    }

    status = StopTailCallOptimization(status);

    return status;
}


//---------------------------------------------------------------------------
// File_NtCloseImpl
//---------------------------------------------------------------------------


_FX NTSTATUS File_NtCloseImpl(HANDLE FileHandle)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status;
    ULONG type;
    FILE_MERGE *merge;
    MSG_HEADER *req;

    P_NtClose pSysNtClose = __sys_NtClose;

    //
    // handle a recursive invocation of NtClose,
    // and requests to close a psuedo-handle
    //

    if (TlsData->file_NtClose_lock ||
        FileHandle == NtCurrentProcess() ||
        FileHandle == NtCurrentThread()) {

        return pSysNtClose ? pSysNtClose(FileHandle) : NtClose(FileHandle);
    }

    TlsData->file_NtClose_lock = TRUE;

    //
    // close for a proxy pipe handle
    //

    if (((ULONG_PTR)FileHandle & PROXY_PIPE_MASK) == PROXY_PIPE_MASK) {

        File_CloseProxyPipe(FileHandle);

        TlsData->file_NtClose_lock = FALSE;

        SetLastError(LastError);
        return STATUS_SUCCESS;
    }

    //
    // determine the type of handle we are closing.
    // if we are closing a key handle, call Key_NtClose
    //

    type = Obj_GetObjectType(FileHandle);

    if (type == OBJ_TYPE_KEY) {

        Key_NtClose(FileHandle);
    }

    //
    // if not closing a file handle, stop here
    //

    if (type != OBJ_TYPE_FILE) {

        TlsData->file_NtClose_lock = FALSE;

        SetLastError(LastError);

        return pSysNtClose ? pSysNtClose(FileHandle) : NtClose(FileHandle);
    }

    //
    // close for a real handle
    //

    req = NULL;

    EnterCriticalSection(&File_DirHandles_CritSec);

    merge = List_Head(&File_DirHandles);
    while (merge) {
        FILE_MERGE *next = List_Next(merge);
        if (merge->handle == FileHandle) {
            List_Remove(&File_DirHandles, merge);
            File_MergeFree(merge);
        }
        merge = next;
    }

    LeaveCriticalSection(&File_DirHandles_CritSec);

    //
    // close and recover file
    //

    File_NotifyRecover(FileHandle, &req);

    status = pSysNtClose ? pSysNtClose(FileHandle) : NtClose(FileHandle);

    if (req) {
        MSG_HEADER *rpl = SbieDll_CallServer(req);
        Dll_Free(req);
        if (rpl)
            Dll_Free(rpl);
    }

    TlsData->file_NtClose_lock = FALSE;

    SetLastError(LastError);
    return status;
}


//---------------------------------------------------------------------------
// File_DeleteDirectory
//---------------------------------------------------------------------------


_FX NTSTATUS File_DeleteDirectory(const WCHAR *FilePath, BOOLEAN JustCheck)
{
    NTSTATUS status;
    ULONG FilePath_len;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    HANDLE handle;
    IO_STATUS_BLOCK IoStatusBlock;
    ULONG info_len;
    FILE_DIRECTORY_INFORMATION *info;
    BOOLEAN is_dot1, is_dot2;
    BOOLEAN RestartScan;

    //
    // open the directory
    //

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    FilePath_len = wcslen(FilePath) * sizeof(WCHAR);

    objname.Length        = (USHORT)FilePath_len;
    objname.MaximumLength = (USHORT)(objname.Length + sizeof(WCHAR));
    objname.Buffer        = (WCHAR *)FilePath;

    status = __sys_NtCreateFile(
        &handle, FILE_GENERIC_READ, &objattrs, &IoStatusBlock, NULL, 0,
        FILE_SHARE_VALID_FLAGS, FILE_OPEN,
        FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

    if (! NT_SUCCESS(status))
        return status;

    //
    // make sure no child files exist, other than "." and "..".
    // we use our NtQueryDirectoryFile here, so it merges from
    // both directories, and discards deleted entries
    //

    info_len = sizeof(FILE_DIRECTORY_INFORMATION)
             + 300 * sizeof(WCHAR);
    info = (FILE_DIRECTORY_INFORMATION *)Dll_Alloc(info_len);

    RestartScan = TRUE;

    while (NT_SUCCESS(status)) {

        status = NtQueryDirectoryFile(
            handle, NULL, NULL, NULL, &IoStatusBlock,
            info, info_len, FileDirectoryInformation,
            TRUE, NULL, RestartScan);

        RestartScan = FALSE;

        if (NT_SUCCESS(status)) {

            is_dot1 = (
                info->FileNameLength == sizeof(WCHAR) &&
                info->FileName[0] == L'.');

            is_dot2 = (
                info->FileNameLength == sizeof(WCHAR) * 2 &&
                info->FileName[0] == L'.' &&
                info->FileName[1] == L'.');

            if (! (is_dot1 || is_dot2))
                status = STATUS_DIRECTORY_NOT_EMPTY;
        }
    }

    if (status == STATUS_NO_MORE_FILES || status == STATUS_NO_SUCH_FILE)
        status = STATUS_SUCCESS;

    if ((! NT_SUCCESS(status)) || JustCheck) {
        Dll_Free(info);
        NtClose(handle);
        return status;
    }

    //
    // now delete any files that physically exist within the copy
    // directory, even if marked deleted.  for this, we query the
    // copy directory by directly calling the system
    // NtQueryDirectoryFile with the copy FileHandle.
    //

    RestartScan = TRUE;
    status = STATUS_SUCCESS;

    while (NT_SUCCESS(status)) {

        status = __sys_NtQueryDirectoryFile(
            handle, NULL, NULL, NULL, &IoStatusBlock,
            info, info_len, FileDirectoryInformation,
            TRUE, NULL, RestartScan);

        RestartScan = FALSE;

        if (NT_SUCCESS(status)) {

            is_dot1 = (
                info->FileNameLength == sizeof(WCHAR) &&
                info->FileName[0] == L'.');

            is_dot2 = (
                info->FileNameLength == sizeof(WCHAR) * 2 &&
                info->FileName[0] == L'.' &&
                info->FileName[1] == L'.');

            if (! (is_dot1 || is_dot2)) {

                ULONG TempPath_len = FilePath_len + info->FileNameLength
                                   + 2 * sizeof(WCHAR);
                WCHAR *TempPath = Dll_AllocTemp(TempPath_len);
                WCHAR *TempPtr = TempPath;
                memcpy(TempPtr, FilePath, FilePath_len);
                TempPtr += FilePath_len / sizeof(WCHAR);
                *TempPtr = L'\\';
                ++TempPtr;
                memcpy(TempPtr, info->FileName, info->FileNameLength);
                TempPtr += info->FileNameLength / sizeof(WCHAR);
                *TempPtr = L'\0';

                objname.Length      = (USHORT)(TempPath_len - sizeof(WCHAR));
                objname.MaximumLength = (USHORT)TempPath_len;
                objname.Buffer      = TempPath;

                status = __sys_NtDeleteFile(&objattrs);

                Dll_Free(TempPath);

                if (status == STATUS_OBJECT_NAME_NOT_FOUND)
                    status = STATUS_SUCCESS;
            }
        }
    }

    //
    // finish
    //

    Dll_Free(info);
    NtClose(handle);

    if (status == STATUS_NO_MORE_FILES || status == STATUS_NO_SUCH_FILE)
        status = STATUS_SUCCESS;

    return status;
}


//---------------------------------------------------------------------------
// File_MarkChildrenDeleted
//---------------------------------------------------------------------------


_FX NTSTATUS File_MarkChildrenDeleted(const WCHAR *ParentTruePath)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    HANDLE handle;
    IO_STATUS_BLOCK IoStatusBlock;
    ULONG info_len;
    FILE_DIRECTORY_INFORMATION *info;
    BOOLEAN is_dot1, is_dot2;
    BOOLEAN RestartScan;
    WCHAR *TruePath, *CopyPath;

    //
    // open the TruePath directory
    //

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    RtlInitUnicodeString(&objname, ParentTruePath);

    status = __sys_NtCreateFile(
        &handle, FILE_GENERIC_READ, &objattrs, &IoStatusBlock, NULL, 0,
        FILE_SHARE_VALID_FLAGS, FILE_OPEN,
        FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

    if (! NT_SUCCESS(status))
        return status;

    //
    // enumerate the children in the TruePath directory.  we can
    // use the system NtQueryDirectoryFile because we know there is
    // nothing to merge (CopyPath was just created and is empty)
    //

    info_len = sizeof(FILE_DIRECTORY_INFORMATION)
             + 300 * sizeof(WCHAR);
    info = (FILE_DIRECTORY_INFORMATION *)Dll_Alloc(info_len);

    RestartScan = TRUE;

    while (NT_SUCCESS(status)) {

        //status = __sys_NtQueryDirectoryFile(
		status = NtQueryDirectoryFile(
            handle, NULL, NULL, NULL, &IoStatusBlock,
            info, info_len, FileDirectoryInformation,
            TRUE, NULL, RestartScan);

        RestartScan = FALSE;

        if (NT_SUCCESS(status)) {

            is_dot1 = (
                info->FileNameLength == sizeof(WCHAR) &&
                info->FileName[0] == L'.');

            is_dot2 = (
                info->FileNameLength == sizeof(WCHAR) * 2 &&
                info->FileName[0] == L'.' &&
                info->FileName[1] == L'.');

            if (! (is_dot1 || is_dot2)) {

                //
                // check if the file is an OpenFilePath
                //

                ULONG FileFlags, mp_flags;

                Dll_PushTlsNameBuffer(TlsData);

                objname.Length = (USHORT)info->FileNameLength;
                objname.MaximumLength = objname.Length;
                objname.Buffer = info->FileName;

                status = File_GetName(
                    handle, &objname, &TruePath, &CopyPath, &FileFlags);

                if (NT_SUCCESS(status))
                    mp_flags = File_MatchPath(TruePath, &FileFlags);
                else
                    mp_flags = PATH_CLOSED_FLAG;

                if (! mp_flags) {

                    //
                    // mark the child deleted in the copy directory
                    //

                    HANDLE handle2;
                    FILE_BASIC_INFORMATION info;

                    RtlInitUnicodeString(&objname, CopyPath);

                    status = __sys_NtCreateFile(
                        &handle2, FILE_GENERIC_WRITE, &objattrs,
                        &IoStatusBlock, NULL, 0, FILE_SHARE_VALID_FLAGS,
                        FILE_SUPERSEDE, FILE_NON_DIRECTORY_FILE |
                            FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

                    if (NT_SUCCESS(status)) {

                        memzero(&info, sizeof(FILE_BASIC_INFORMATION));

                        info.CreationTime.HighPart = DELETE_MARK_HIGH;
                        info.CreationTime.LowPart  = DELETE_MARK_LOW;

                        status = __sys_NtSetInformationFile(
                            handle2, &IoStatusBlock,
                            &info, sizeof(FILE_BASIC_INFORMATION),
                            FileBasicInformation);

                        NtClose(handle2);
                    }
                }

                Dll_PopTlsNameBuffer(TlsData);

                status = STATUS_SUCCESS;
            }
        }
    }

    Dll_Free(info);
    NtClose(handle);
    return status;
}


//---------------------------------------------------------------------------
//
// Immediate Recovery for Files
//
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _FILE_RECOVER_FOLDER {

    LIST_ELEM list_elem;
    ULONG ticks;            // for File_RecPaths
    ULONG path_len;
    WCHAR path[1];

} FILE_RECOVER_FOLDER;


//---------------------------------------------------------------------------
// File_InitRecoverFolders
//---------------------------------------------------------------------------


_FX void File_InitRecoverFolders(void)
{
    //
    // init list of recoverable file handles
    //

    List_Init(&File_RecoverFolders);
    List_Init(&File_RecoverIgnores);

    InitializeCriticalSectionAndSpinCount(&File_RecHandles_CritSec, 1000);

    List_Init(&File_RecPaths);

    File_RecHandles = Dll_Alloc(sizeof(HANDLE) * 128);
    memzero(File_RecHandles, sizeof(HANDLE) * 128);
    File_RecHandles[127] = (HANDLE)-1;

    //
    // init list of recover folders
    //

    if (SbieApi_QueryConfBool(NULL, L"AutoRecover", FALSE)) {

        ULONG buf_len = 4096 * sizeof(WCHAR);
        WCHAR *buf = Dll_AllocTemp(buf_len);

        File_InitRecoverList(
            L"RecoverFolder", &File_RecoverFolders, TRUE, buf, buf_len);

        File_InitRecoverList(
            L"AutoRecoverIgnore", &File_RecoverIgnores, FALSE, buf, buf_len);

        Dll_Free(buf);
    }
}


//---------------------------------------------------------------------------
// File_InitRecoverList
//---------------------------------------------------------------------------


_FX void File_InitRecoverList(
    const WCHAR *setting, LIST *list, BOOLEAN MustBeValidPath,
    WCHAR *buf, ULONG buf_len)
{
    UNICODE_STRING uni;
    WCHAR *TruePath, *CopyPath, *ReparsedPath;
    FILE_RECOVER_FOLDER *fold;

    ULONG index = 0;
    while (1) {

        NTSTATUS status = SbieApi_QueryConf(
            NULL, setting, index, buf, buf_len - 16 * sizeof(WCHAR));
        if (! NT_SUCCESS(status))
            break;
        ++index;

        RtlInitUnicodeString(&uni, buf);
        status = File_GetName(NULL, &uni, &TruePath, &CopyPath, NULL);

        ReparsedPath = NULL;

        if (NT_SUCCESS(status) && MustBeValidPath) {

            ReparsedPath = File_TranslateTempLinks(TruePath, FALSE);
            if (ReparsedPath)
                TruePath = ReparsedPath;

        } else if ((! NT_SUCCESS(status)) && (! MustBeValidPath)) {

            TruePath = buf;
            status = STATUS_SUCCESS;
        }

        if (NT_SUCCESS(status)) {

            ULONG len = wcslen(TruePath);
            if (len && TruePath[len - 1] == L'\\') {
                TruePath[len - 1] = L'\0';
                --len;
            }
            len = sizeof(FILE_RECOVER_FOLDER)
                + (len + 1) * sizeof(WCHAR);
            fold = Dll_Alloc(len);

            fold->ticks = 0;    // not used

            wcscpy(fold->path, TruePath);
            fold->path_len = wcslen(fold->path);

            List_Insert_After(list, NULL, fold);
        }

        if (ReparsedPath)
            Dll_Free(ReparsedPath);
    }
}


//---------------------------------------------------------------------------
// File_IsRecoverable
//---------------------------------------------------------------------------


_FX BOOLEAN File_IsRecoverable(
    const WCHAR *TruePath)
{
    const WCHAR *save_TruePath;
    FILE_RECOVER_FOLDER *fold;
    const WCHAR *ptr;
    ULONG TruePath_len;
    ULONG PrefixLen;
    BOOLEAN ok;

    //
    // if we have a path that looks like
    // \Device\LanmanRedirector\;Q:000000000000b09f\server\share\f1.txt
    // \Device\Mup\;LanmanRedirector\;Q:000000000000b09f\server\share\f1.txt
    // then translate to
    // \Device\Mup\server\share\f1.txt
    // and test again.  We do this because the SbieDrv records paths
    // in the \Device\Mup format.  See SbieDrv::File_TranslateShares.
    //

    save_TruePath = TruePath;

    if (_wcsnicmp(TruePath, File_Redirector, File_RedirectorLen) == 0)
        PrefixLen = File_RedirectorLen;
    else if (_wcsnicmp(TruePath, File_DfsClientRedir, File_DfsClientRedirLen) == 0)
        PrefixLen = File_DfsClientRedirLen;
    else if (_wcsnicmp(TruePath, File_HgfsRedir, File_HgfsRedirLen) == 0)
        PrefixLen = File_HgfsRedirLen;
    else if (_wcsnicmp(TruePath, File_MupRedir, File_MupRedirLen) == 0)
        PrefixLen = File_MupRedirLen;
    else
        PrefixLen = 0;

    if (PrefixLen && TruePath[PrefixLen] == L';') {

        WCHAR *ptr = wcschr(TruePath + PrefixLen, L'\\');
        if (ptr && ptr[0] && ptr[1]) {

            ULONG len1   = wcslen(ptr + 1);
            ULONG len2   = (File_MupLen + len1 + 8) * sizeof(WCHAR);
            WCHAR *path2 = Dll_Alloc(len2);
            wmemcpy(path2, File_Mup, File_MupLen);
            wmemcpy(path2 + File_MupLen, ptr + 1, len1 + 1);

            TruePath = (const WCHAR *)path2;
        }
    }

    //
    // look for the TruePath in the list of RecoverFolder settings
    //

    ok = FALSE;

    fold = List_Head(&File_RecoverFolders);
    while (fold) {

        if (_wcsnicmp(fold->path, TruePath, fold->path_len) == 0) {
            ptr = TruePath + fold->path_len;
            if (*ptr == L'\\' || *ptr == L'\0') {
                ok = TRUE;
                break;
            }
        }

        fold = List_Next(fold);
    }

    if (! ok)
        goto finish;

    //
    // ignore files that begin with ~$ (Microsoft Office temp files)
    // or that don't have a file type extension (probably temp files)
    //

    if (File_MsoDllLoaded) {

        ptr = wcsrchr(TruePath, L'\\');
        if (ptr) {
            if (ptr[1] == L'~' && ptr[2] == L'$')
                ok = FALSE;
            else {
                ptr = wcschr(ptr, L'.');
                if (! ptr)
                    ok = FALSE;
            }
            if (! ok)
                goto finish;
        }
    }

    //
    // look for TruePath in the list of AutoRecoverIgnore settings
    //

    TruePath_len = wcslen(TruePath);

    fold = List_Head(&File_RecoverIgnores);
    while (fold) {

        if (_wcsnicmp(fold->path, TruePath, fold->path_len) == 0) {
            ptr = TruePath + fold->path_len;
            if (*ptr == L'\\' || *ptr == L'\0') {
                ok = FALSE;
                break;
            }
        }

        if (TruePath_len >= fold->path_len) {
            ptr = TruePath + TruePath_len - fold->path_len;
            if (_wcsicmp(fold->path, ptr) == 0) {
                ok = FALSE;
                break;
            }
        }

        fold = List_Next(fold);
    }

    //
    // finish
    //

finish:

    if (TruePath != save_TruePath)
        Dll_Free((WCHAR *)TruePath);
    return ok;
}


//---------------------------------------------------------------------------
// File_RecordRecover
//---------------------------------------------------------------------------


_FX BOOLEAN File_RecordRecover(HANDLE FileHandle, const WCHAR *TruePath)
{
    ULONG i;

    if (! File_RecHandles)
        return FALSE;

    if (! File_IsRecoverable(TruePath))
        return FALSE;

    EnterCriticalSection(&File_RecHandles_CritSec);

    if (FileHandle) {
        for (i = 0; File_RecHandles[i] != (HANDLE)-1; ++i)
            if (File_RecHandles[i] == FileHandle) {
                FileHandle = NULL;
                break;
            }
    }

    if (FileHandle) {
        for (i = 0; File_RecHandles[i] != (HANDLE)-1; ++i)
            if (! File_RecHandles[i]) {
                File_RecHandles[i] = FileHandle;
                FileHandle = NULL;
                break;
            }
    }

    LeaveCriticalSection(&File_RecHandles_CritSec);

    return TRUE;
}


//---------------------------------------------------------------------------
// File_DuplicateRecover
//---------------------------------------------------------------------------


_FX void File_DuplicateRecover(
    HANDLE OldFileHandle, HANDLE NewFileHandle)
{
    ULONG i;
    BOOLEAN dup;

    if (! File_RecHandles)
        return;

    //
    // called from NtDuplicateObject to duplicate the "recoverability"
    // of the old handle to the new handle.  needed in particular for
    // SHFileOperation to recover correctly on Windows Vista
    //

    dup = FALSE;

    EnterCriticalSection(&File_RecHandles_CritSec);

    for (i = 0; File_RecHandles[i] != (HANDLE)-1; ++i)
        if (File_RecHandles[i] == OldFileHandle) {
            dup = TRUE;
            break;
        }

    if (dup && NewFileHandle) {
        for (i = 0; File_RecHandles[i] != (HANDLE)-1; ++i)
            if (File_RecHandles[i] == NewFileHandle) {
                NewFileHandle = NULL;
                break;
            }
    }

    if (dup && NewFileHandle) {
        for (i = 0; File_RecHandles[i] != (HANDLE)-1; ++i)
            if (! File_RecHandles[i]) {
                File_RecHandles[i] = NewFileHandle;
                NewFileHandle = NULL;
                break;
            }
    }

    LeaveCriticalSection(&File_RecHandles_CritSec);
}


//---------------------------------------------------------------------------
// File_NotifyRecover
//---------------------------------------------------------------------------


_FX void File_NotifyRecover(
    HANDLE FileHandle, MSG_HEADER **out_req)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    NTSTATUS status;
    union {
        FILE_NETWORK_OPEN_INFORMATION open;
        ULONG space[16];
    } info;
    ULONG length;
    ULONG i;
    ULONG FileFlags;
    UNICODE_STRING uni;
    WCHAR *TruePath, *CopyPath;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN IsRecoverable;

    //
    // check input handle against list of recorded handles
    //

    if (! File_RecHandles)
        return;

    IsRecoverable = FALSE;
    EnterCriticalSection(&File_RecHandles_CritSec);

    for (i = 0; File_RecHandles[i] != (HANDLE)-1; ++i) {
        if (File_RecHandles[i] == FileHandle)
        {
            File_RecHandles[i] = NULL;
            IsRecoverable = TRUE;
            break;
        }
    }

    LeaveCriticalSection(&File_RecHandles_CritSec);

    //
    // in a Chrome sandbox process, handles are opened by the broker,
    // so skip checking against the list of recorded file handles
    //

    if ((! IsRecoverable) && Dll_ChromeSandbox) {

        FILE_ACCESS_INFORMATION info;

        status = __sys_NtQueryInformationFile(
            FileHandle, &IoStatusBlock, &info,
            sizeof(FILE_ACCESS_INFORMATION), FileAccessInformation);

        if (NT_SUCCESS(status) && (info.AccessFlags & FILE_WRITE_DATA))
            IsRecoverable = TRUE;
        else
            IsRecoverable = FALSE;
    }

    if (! IsRecoverable)
        return;

    //
    // send request to SbieCtrl (if recoverable file)
    //

    Dll_PushTlsNameBuffer(TlsData);

    do {

        RtlInitUnicodeString(&uni, L"");
        status = File_GetName(
            FileHandle, &uni, &TruePath, &CopyPath, &FileFlags);
        if (! NT_SUCCESS(status))
            break;

        if (! (FileFlags & FGN_IS_BOXED_PATH))
            break;

        //
        // Immediate Recovery
        //

        IsRecoverable = File_IsRecoverable(TruePath);
        if (! IsRecoverable)
            break;

        status = __sys_NtQueryInformationFile(
            FileHandle, &IoStatusBlock, &info,
            sizeof(FILE_NETWORK_OPEN_INFORMATION),
            FileNetworkOpenInformation);

        if (! NT_SUCCESS(status))
            break;
        if (info.open.EndOfFile.QuadPart == 0)
            break;

        //
        // queue immediate recovery elements for later processing
        //

        if (IsRecoverable) {

            FILE_RECOVER_FOLDER *rec;
            ULONG TruePath_len;

            EnterCriticalSection(&File_RecHandles_CritSec);

            TruePath_len = wcslen(TruePath);

            rec = List_Head(&File_RecPaths);
            while (rec) {
                if (rec->path_len == TruePath_len)
                    if (_wcsicmp(rec->path, TruePath) == 0)
                        break;
                rec = List_Next(rec);
            }

            if (! rec) {

                length = sizeof(FILE_RECOVER_FOLDER)
                       + (TruePath_len + 1) * sizeof(WCHAR);
                rec = Dll_Alloc(length);

                rec->ticks = GetTickCount();

                wcscpy(rec->path, TruePath);
                rec->path_len = TruePath_len;

                List_Insert_After(&File_RecPaths, NULL, rec);
            }

            LeaveCriticalSection(&File_RecHandles_CritSec);
            if (rec)
                File_DoAutoRecover(TRUE);
        }

    } while (0);

    Dll_PopTlsNameBuffer(TlsData);
}


//---------------------------------------------------------------------------
// File_RtlGetCurrentDirectory_U
//---------------------------------------------------------------------------


_FX ULONG File_RtlGetCurrentDirectory_U(ULONG buf_len, WCHAR *buf_ptr)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    WCHAR *TruePath;
    WCHAR *path;
    ULONG len, is_root;

    //
    // handle a recursive invocation of RtlGetCurrentDirectory_U
    //

    if (TlsData->file_GetCurDir_lock)
        return __sys_RtlGetCurrentDirectory_U(buf_len, buf_ptr);

    //
    // get the current directory from the system
    //

    path = Dll_AllocTemp(8192);
    len = __sys_RtlGetCurrentDirectory_U(8190, path);
    if (len > 8190) {
        Dll_Free(path);
        SetLastError(LastError);
        return __sys_RtlGetCurrentDirectory_U(buf_len, buf_ptr);
    }

    TlsData->file_GetCurDir_lock = TRUE;

    //
    // check if we already handled this directory
    //

    EnterCriticalSection(&File_CurDir_CritSec);

    if (File_CurDir_LastInput && _wcsicmp(path, File_CurDir_LastInput) == 0)
        goto already_handled;

    //
    // not handled yet, do handling now
    //

    if (File_CurDir_LastOutput)
        Dll_Free(File_CurDir_LastOutput);
    File_CurDir_LastOutput = NULL;

    if (File_CurDir_LastInput)
        Dll_Free(File_CurDir_LastInput);
    len = wcslen(path) + 1;
    File_CurDir_LastInput = Dll_Alloc(len * sizeof(WCHAR));
    wmemcpy(File_CurDir_LastInput, path, len);

    is_root = 0;
    if (len >= 2 && path[len - 2] == L'\\')
        is_root = 1;

    TruePath = File_GetTruePathForBoxedPath(path, TRUE);

    if (TruePath) {

        Dll_Free(path);
        path = TruePath;

        len = wcslen(path) + 1;
        if (len >= 2 && path[len - 2] == L'\\')
            is_root = 0;

        File_CurDir_LastOutput =
            Dll_Alloc((len + is_root) * sizeof(WCHAR));
        wmemcpy(File_CurDir_LastOutput, path, len);
        if (is_root)
            wmemcpy(File_CurDir_LastOutput + len - 1, L"\\\0", 2);
    }

    //
    // return the handled directory
    //

already_handled:

    if (File_CurDir_LastOutput) {

        len = (wcslen(File_CurDir_LastOutput) + 1) * sizeof(WCHAR);
        if (buf_len >= len) {
            memcpy(buf_ptr, File_CurDir_LastOutput, len);
            len -= sizeof(WCHAR);
        }

        LeaveCriticalSection(&File_CurDir_CritSec);

    } else {

        LeaveCriticalSection(&File_CurDir_CritSec);
        len = __sys_RtlGetCurrentDirectory_U(buf_len, buf_ptr);
    }

    Dll_Free(path);

    TlsData->file_GetCurDir_lock = FALSE;
    SetLastError(LastError);

    return len;
}


//---------------------------------------------------------------------------
// File_RtlSetCurrentDirectory_U
//---------------------------------------------------------------------------


_FX NTSTATUS File_RtlSetCurrentDirectory_U(UNICODE_STRING *PathName)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status = __sys_RtlSetCurrentDirectory_U(PathName);
    if (NT_SUCCESS(status)) {

        //
        // RtlGetFullPathName_U gets the directory directly from the
        // PEB rather than through RtlGetCurrentDirectory_U, so
        // we could have a discrepancy between what the PEB contains
        // and what our RtlGetFullPathName_U would return.  fix it
        // by calling RtlSetCurrentDirectory_U again
        //

        WCHAR *path = Dll_AllocTemp(8192);

        ULONG len = File_RtlGetCurrentDirectory_U(8190, path);
        if (len <= 8190) {

            UNICODE_STRING uni;
            path[8190 / sizeof(WCHAR)] = L'\0';
            RtlInitUnicodeString(&uni, path);

            status = __sys_RtlSetCurrentDirectory_U(&uni);
        }

        Dll_Free(path);
    }

    SetLastError(LastError);
    return status;
}


//---------------------------------------------------------------------------
// File_CallRtlGetFullPathName
//---------------------------------------------------------------------------


_FX ULONG File_CallRtlGetFullPathName(
    WCHAR *src, ULONG buf_len, WCHAR *buf_ptr, WCHAR **file_part_ptr)
{
    ULONG ret_len;

    if (__sys_RtlGetFullPathName_UEx) {

        __sys_RtlGetFullPathName_UEx(
            src, buf_len, buf_ptr, file_part_ptr, &ret_len);

    } else {

        ret_len = __sys_RtlGetFullPathName_U(
            src, buf_len, buf_ptr, file_part_ptr);
    }

    return ret_len;
}


//---------------------------------------------------------------------------
// File_RtlGetFullPathName_U
//---------------------------------------------------------------------------


_FX ULONG File_RtlGetFullPathName_U(
    WCHAR *src, ULONG buf_len, WCHAR *buf_ptr, WCHAR **file_part_ptr)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    WCHAR *temp_buf;
    WCHAR *temp_ptr;
    ULONG ret_len;

    if (src && File_CurDir_LastOutput && src[0] == *File_CurDir_LastOutput
        && src[1] == L':' && src[2] == L'.' && src[3] == L'\0') {

        //
        // if invoked for "X:." where X is the result of the translation
        // done in File_RtlGetCurrentDirectory_U, then we need to override
        // C:. with the result from File_RtlGetCurrentDirectory_U, to
        // make sure the correct current directory is returned
        //

        return File_CallRtlGetFullPathName(
                    File_CurDir_LastOutput, buf_len, buf_ptr, file_part_ptr);
    }

    //
    // remove sandbox prefix, except during process creation.  the child
    // process must have a real directory path, even if sandboxed
    //

    if (TlsData->proc_create_process) {

        return File_CallRtlGetFullPathName(
                    src, buf_len, buf_ptr, file_part_ptr);
    }

    //
    // get the path into an intermediate buffer that should be large
    // enough for any possible path.  but reset caller's buffers just
    // as RtlGetFullPathName_U would do if it worked on those buffers
    //

    if (buf_ptr && buf_len >= sizeof(WCHAR))
        *buf_ptr = L'\0';

    if (file_part_ptr)
        *file_part_ptr = NULL;

    temp_buf = Dll_AllocTemp(8192);
    temp_ptr = NULL;

    ret_len = File_CallRtlGetFullPathName(src, 8192, temp_buf, &temp_ptr);

    if (ret_len && ret_len <= 8192) {

        //
        // if the path we got is inside the sandbox, change it to
        // the corresponding path outside the sandbox
        //

        BOOLEAN TrailingBackslash =
                        (temp_buf[ret_len / sizeof(WCHAR) - 1] == L'\\');

        WCHAR *TruePath = File_GetTruePathForBoxedPath(temp_buf, TRUE);

        if (TruePath) {

            WCHAR *iptr = TruePath;
            WCHAR *optr = temp_buf;
            temp_ptr = NULL;

            while (*iptr) {
                *optr = *iptr;
                ++optr;
                if (*iptr == L'\\')
                    temp_ptr = optr;
                ++iptr;
            }

            if (TrailingBackslash) {
                *optr = L'\\';
                ++optr;
                temp_ptr = optr;
            }

            *optr = L'\0';
            ret_len = (optr - temp_buf) * sizeof(WCHAR);

            Dll_Free(TruePath);
        }

        //
        // copy the contents of the intermediate buffer to the caller
        //

        if (ret_len + sizeof(WCHAR) <= buf_len) {

            memcpy(buf_ptr, temp_buf, ret_len);
            buf_ptr[ret_len / sizeof(WCHAR)] = L'\0';

            if (file_part_ptr && temp_ptr && *temp_ptr)
                *file_part_ptr = temp_ptr - temp_buf + buf_ptr;

        } else
            ret_len += sizeof(WCHAR);

    } else
        ret_len = 0;

    //
    // finish
    //

    Dll_Free(temp_buf);
    SetLastError(LastError);
    return ret_len;
}


//---------------------------------------------------------------------------
// File_RtlGetFullPathName_UEx
//---------------------------------------------------------------------------


_FX NTSTATUS File_RtlGetFullPathName_UEx(
    WCHAR *src, ULONG buf_len, WCHAR *buf_ptr, WCHAR **file_part_ptr,
    ULONG *ret_len_ptr)
{
    //
    // On Windows 7, we have to hook RtlGetFullPathName_UEx instead of the
    // normal version, but we can still use the normal version internally
    //

    ULONG ret_len =
            File_RtlGetFullPathName_U(src, buf_len, buf_ptr, file_part_ptr);
    if (ret_len_ptr)
        *ret_len_ptr = ret_len;
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// File_NtQueryVolumeInformationFile
//---------------------------------------------------------------------------


_FX NTSTATUS File_NtQueryVolumeInformationFile(
    HANDLE FileHandle,
    IO_STATUS_BLOCK *IoStatusBlock,
    PVOID FsInformation,
    ULONG Length,
    ULONG FsInformationClass)
{
    NTSTATUS status;
    HANDLE handle;
    BOOLEAN IsBoxedPath;
    WCHAR *path;

    // NtQueryObject on a named pipe handle can hang when it is in pending read/write. See P.71 NT/2000 Native API Reference
    // If the caller only asks about FileFsDeviceInformation and it is a named pipe, hook can return right away without the 
    // need to wait on NtQueryObject called by SbieDll_GetHandlePath
    if (FsInformationClass == FileFsDeviceInformation && Length >= sizeof(FILE_FS_DEVICE_INFORMATION))
    {
        FILE_FS_DEVICE_INFORMATION devInfo = { 0 };
        IO_STATUS_BLOCK ioStatusBlock = { 0 };

        status = __sys_NtQueryVolumeInformationFile(FileHandle, &ioStatusBlock, &devInfo, sizeof(devInfo), FileFsDeviceInformation);

        if (NT_SUCCESS(status) && devInfo.DeviceType == FILE_DEVICE_NAMED_PIPE)
        {
            if (IoStatusBlock)
                *IoStatusBlock = ioStatusBlock;

            memcpy(FsInformation, &devInfo, sizeof(devInfo));
            return status;
        }
    }

    //
    // if caller is querying volume info for \Sandbox\...\drive\X,
    // then open the real drive X to get the correct result
    //

    path = Dll_AllocTemp(8192);

    handle = FileHandle;

    status = SbieDll_GetHandlePath(FileHandle, path, &IsBoxedPath);
    if (IsBoxedPath && (
            NT_SUCCESS(status) || (status == STATUS_BAD_INITIAL_PC))) {

        status = SbieDll_GetHandlePath(FileHandle, path, NULL);
        if (NT_SUCCESS(status)) {

            const FILE_DRIVE *drive =
                File_GetDriveForPath(path, wcslen(path));
            if (drive) {

                //
                // append a suffix backslash to open the drive root
                //

                UNICODE_STRING objname;
                OBJECT_ATTRIBUTES objattrs;

                objname.Buffer = Dll_Alloc((drive->len + 4) * sizeof(WCHAR));
                wmemcpy(objname.Buffer, drive->path, drive->len);
                objname.Buffer[drive->len    ] = L'\\';
                objname.Buffer[drive->len + 1] = L'\0';

                objname.Length = (USHORT)(drive->len + 1) * sizeof(WCHAR);
                objname.MaximumLength = objname.Length + sizeof(WCHAR);

                InitializeObjectAttributes(
                    &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

                if (FsInformationClass == FileFsSizeInformation ||
                    FsInformationClass == FileFsFullSizeInformation) {

                    //
                    // for these info classes we can use a simpler
                    // and faster open request, the same way the
                    // GetDiskFreeSpace API does it
                    //

                    status = __sys_NtOpenFile(
                        &handle, FILE_LIST_DIRECTORY | SYNCHRONIZE, &objattrs,
                        IoStatusBlock, FILE_SHARE_READ | FILE_SHARE_WRITE,
                            FILE_SYNCHRONOUS_IO_NONALERT |
                            FILE_DIRECTORY_FILE |
                            FILE_OPEN_FOR_FREE_SPACE_QUERY);

                } else {

                    status = __sys_NtCreateFile(
                        &handle, GENERIC_READ | SYNCHRONIZE, &objattrs,
                        IoStatusBlock, NULL, 0, FILE_SHARE_VALID_FLAGS,
                        FILE_OPEN,
                        FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                        NULL, 0);
                }

                Dll_Free(objname.Buffer);

                if (! NT_SUCCESS(status))
                    handle = FileHandle;

                LeaveCriticalSection(File_DrivesAndLinks_CritSec);
            }
        }
    }

    Dll_Free(path);

    status = __sys_NtQueryVolumeInformationFile(
        handle, IoStatusBlock, FsInformation, Length, FsInformationClass);

    if (handle != FileHandle)
        __sys_NtClose(handle);

    return status;
}


//---------------------------------------------------------------------------
// File_SetReparsePoint
//---------------------------------------------------------------------------


_FX NTSTATUS File_SetReparsePoint(
    HANDLE FileHandle, UCHAR *Data, ULONG DataLen)
{
    THREAD_DATA *TlsData;
    NTSTATUS status;
    UNICODE_STRING objname;
    OBJECT_ATTRIBUTES objattrs;
    WCHAR *TruePath, *CopyPath;
    WCHAR *SourcePath, *TargetPath;
    USHORT NameOffset, NameLength;
    ULONG FileFlags, mp_flags;

    if (! Data)
        return STATUS_BAD_INITIAL_PC;

    if (*(ULONG *)Data == IO_REPARSE_TAG_SYMLINK)
        return STATUS_INVALID_DEVICE_REQUEST;

    if (*(ULONG *)Data != IO_REPARSE_TAG_MOUNT_POINT)
        return STATUS_BAD_INITIAL_PC;

    NameOffset = *(USHORT *)(Data + 8);
    NameLength = *(USHORT *)(Data + 10);

    SourcePath = NULL;
    TargetPath = NULL;

    //
    // get paths to source and target directories
    //

    TlsData = Dll_GetTlsData(NULL);

    Dll_PushTlsNameBuffer(TlsData);

    __try {

        //
        // get copy path of reparse source
        //

        RtlInitUnicodeString(&objname, L"");
        InitializeObjectAttributes(
            &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

        status = File_GetName(
                    FileHandle, &objname, &TruePath, &CopyPath, &FileFlags);
        if (! NT_SUCCESS(status))
            __leave;

        //
        // check if this is an open or closed path
        //

        mp_flags = File_MatchPath(TruePath, &FileFlags);

        if (PATH_IS_OPEN(mp_flags)) {
            status = STATUS_BAD_INITIAL_PC;
            __leave;
        }

        if (PATH_IS_CLOSED(mp_flags)) {
            status = STATUS_ACCESS_DENIED;
            __leave;
        }

		if (File_Snapshot != NULL)
			File_FindSnapshotPath(&CopyPath);

        SourcePath = Dll_Alloc((wcslen(CopyPath) + 4) * sizeof(WCHAR));
        wcscpy(SourcePath, CopyPath);

        //
        // get copy path of reparse target
        //

        objname.Length = NameLength;
        objname.MaximumLength = objname.Length;
        objname.Buffer = (WCHAR *)(Data + 0x10 + NameOffset);;

        status = File_GetName(NULL, &objname, &TruePath, &CopyPath, NULL);
        if (! NT_SUCCESS(status))
            __leave;

        TargetPath = Dll_Alloc((wcslen(CopyPath) + 4) * sizeof(WCHAR));
        wcscpy(TargetPath, CopyPath);

    //
    // finish
    //

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    //
    // send request to SbieSvc
    //

    if (SourcePath && TargetPath) {

        ULONG req_len;
        ULONG dst_ofs;
        WCHAR *dst_path;
        FILE_SET_REPARSE_POINT_REQ *req;
        MSG_HEADER *rpl;

        req_len = sizeof(FILE_SET_REPARSE_POINT_REQ)
                + (wcslen(SourcePath) + 4) * sizeof(WCHAR);
        dst_ofs = req_len;
        req_len += (wcslen(TargetPath) + 4) * sizeof(WCHAR);

        req = (FILE_SET_REPARSE_POINT_REQ *)Dll_Alloc(req_len);
        req->h.length = req_len;
        req->h.msgid = MSGID_FILE_SET_REPARSE_POINT;

        wcscpy(req->src_path, SourcePath);
        req->src_path_len = wcslen(req->src_path) * sizeof(WCHAR);

        dst_path = (WCHAR *)((UCHAR *)req + dst_ofs);
        wcscpy(dst_path, TargetPath);
        req->dst_path_ofs = dst_ofs;
        req->dst_path_len = wcslen(dst_path) * sizeof(WCHAR);

        rpl = SbieDll_CallServer(&req->h);

        Dll_Free(req);

        if (rpl) {
            status = rpl->status;
            Dll_Free(rpl);
        } else
            status = STATUS_ACCESS_DENIED;
    }

    if (SourcePath)
        Dll_Free(SourcePath);
    if (TargetPath)
        Dll_Free(TargetPath);

    Dll_PopTlsNameBuffer(TlsData);

    return status;
}


//---------------------------------------------------------------------------
// File_DoAutoRecover
//---------------------------------------------------------------------------


_FX void File_DoAutoRecover(BOOLEAN force)
{
    static ULONG last_ticks = 0;

    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    ULONG ticks = GetTickCount();
    if (force || (ticks - last_ticks > 400)) {

        last_ticks = ticks;

        if (TryEnterCriticalSection(&File_RecHandles_CritSec)) {

            if (List_Head(&File_RecPaths)) {

                Dll_PushTlsNameBuffer(TlsData);

                File_DoAutoRecover_2(force, ticks);

                Dll_PopTlsNameBuffer(TlsData);
            }

            LeaveCriticalSection(&File_RecHandles_CritSec);
        }
    }

    SetLastError(LastError);
}


//---------------------------------------------------------------------------
// File_DoAutoRecover_2
//---------------------------------------------------------------------------


_FX void File_DoAutoRecover_2(BOOLEAN force, ULONG ticks)
{
    NTSTATUS status;
    SYSTEM_HANDLE_INFORMATION *info = NULL;
    FILE_GET_ALL_HANDLES_RPL *rpl = NULL;
    ULONG info_len = 64, len, i;
    WCHAR *pathbuf;
    ULONG UseCount = 0;
    FILE_RECOVER_FOLDER *rec;
    UCHAR FileObjectTypeNumber = 0;

    //
    // get list of open handles in the system
    //

    for (i = 0; i < 5; ++i) {

        info = Dll_AllocTemp(info_len);

        status = NtQuerySystemInformation(
            SystemHandleInformation, info, info_len, &len);

        if (NT_SUCCESS(status))
            break;

        Dll_Free(info);
        info_len = len + 64;

        if (status == STATUS_BUFFER_OVERFLOW ||
            status == STATUS_INFO_LENGTH_MISMATCH ||
            status == STATUS_BUFFER_TOO_SMALL) {

            continue;
        }

        break;
    }

    if (status == STATUS_ACCESS_DENIED) {

        //
        // on Windows 8.1, NtQuerySystemInformation fails, probably because
        // we are running without any privileges, so go through SbieSvc
        //

        MSG_HEADER req;
        req.length = sizeof(req);
        req.msgid = MSGID_FILE_GET_ALL_HANDLES;
        rpl = (FILE_GET_ALL_HANDLES_RPL *)SbieDll_CallServer(&req);

        if (rpl) {
            info = NULL;
            status = STATUS_SUCCESS;
        }
    }

    if (! NT_SUCCESS(status))
        return;

    pathbuf = Dll_AllocTemp(1024);

    //
    // scan list of queued recovery files
    //

    rec = List_Head(&File_RecPaths);
    while (rec) {

        FILE_RECOVER_FOLDER *rec_next = List_Next(rec);
        BOOLEAN send2199 = FALSE;

        if (force)
            send2199 = TRUE;
        else {
            if (ticks - rec->ticks >= 1000) {
                ULONG UseCount = File_DoAutoRecover_3(
                    rec->path, pathbuf, info, rpl, &FileObjectTypeNumber);
                if (UseCount == 0)
                    send2199 = TRUE;
            }
        }

        if (send2199) {
            WCHAR *colon = wcschr(rec->path, L':');
            if (! colon)
                SbieApi_Log2199(rec->path);
            List_Remove(&File_RecPaths, rec);
        }

        rec = rec_next;
    }

    //
    // finish
    //

    Dll_Free(pathbuf);
    if (info)
        Dll_Free(info);
    if (rpl)
        Dll_Free(rpl);
}


//---------------------------------------------------------------------------
// File_DoAutoRecover_3
//---------------------------------------------------------------------------


_FX ULONG File_DoAutoRecover_3(
    const WCHAR *PathToFind, WCHAR *PathBuf1024,
    SYSTEM_HANDLE_INFORMATION *info, FILE_GET_ALL_HANDLES_RPL *rpl,
    UCHAR *FileObjectTypeNumber)
{
    HANDLE FileHandle;
    ULONG UseCount, i;

    //
    // scan handles for current process
    //

    UseCount = 0;

    if (info) {

        for (i = 0; i < info->Count; ++i) {

            HANDLE_INFO *hi = &info->HandleInfo[i];

            if (hi->ProcessId != Dll_ProcessId)
                continue;

            FileHandle = (HANDLE)(ULONG_PTR)hi->Handle;

            UseCount += File_DoAutoRecover_4(
                    PathToFind, PathBuf1024,
                    FileHandle, hi->ObjectTypeNumber, FileObjectTypeNumber);
        }

    } else if (rpl) {

        for (i = 0; i < rpl->num_handles; ++i) {

            UCHAR objtype = (UCHAR)(rpl->handles[i] >> 24);

            FileHandle = (HANDLE)(ULONG_PTR)(rpl->handles[i] & 0x00FFFFFFU);

            UseCount += File_DoAutoRecover_4(
                    PathToFind, PathBuf1024,
                    FileHandle, objtype, FileObjectTypeNumber);
        }
    }

    return UseCount;
}


//---------------------------------------------------------------------------
// File_DoAutoRecover_4
//---------------------------------------------------------------------------


_FX ULONG File_DoAutoRecover_4(
    const WCHAR *PathToFind, WCHAR *PathBuf1024,
    HANDLE FileHandle, UCHAR ObjectTypeNumber, UCHAR *FileObjectTypeNumber)
{
    UNICODE_STRING uni;
    WCHAR *TruePath, *CopyPath;
    NTSTATUS status;

    //
    // make sure the handle is to a file
    //

    if (*FileObjectTypeNumber) {

        if (ObjectTypeNumber != *FileObjectTypeNumber)
            return 0;

    } else {

        if (Obj_GetObjectType(FileHandle) == OBJ_TYPE_FILE) {

            *FileObjectTypeNumber = ObjectTypeNumber;

        } else
            return 0;
    }

    //
    // get file name
    //

    status = SbieApi_GetFileName(FileHandle, 1000, PathBuf1024);
    if (! NT_SUCCESS(status))
        return 0;

    RtlInitUnicodeString(&uni, PathBuf1024);
    status = File_GetName(NULL, &uni, &TruePath, &CopyPath, NULL);
    if (! NT_SUCCESS(status))
        return 0;

    if (_wcsicmp(PathToFind, TruePath) == 0)
        return 1;

    return 0;
}


//---------------------------------------------------------------------------
// File_MyQueryDirectoryFile
//---------------------------------------------------------------------------


_FX NTSTATUS File_MyQueryDirectoryFile(
    HANDLE FileHandle,
    void *FileInformation,
    ULONG Length,
    FILE_INFORMATION_CLASS FileInformationClass,
    BOOLEAN ReturnSingleEntry,
    UNICODE_STRING *FileMask,
    BOOLEAN RestartScan)
{
    IO_STATUS_BLOCK MyIoStatusBlock;

    if (! __sys_NtQueryDirectoryFile)
        return STATUS_NOT_SUPPORTED;

    return __sys_NtQueryDirectoryFile(
        FileHandle, NULL, NULL, NULL, &MyIoStatusBlock,
        FileInformation, Length, FileInformationClass,
        ReturnSingleEntry, FileMask, RestartScan);
}


//---------------------------------------------------------------------------
// File_MsoDll
//---------------------------------------------------------------------------


_FX BOOLEAN File_MsoDll(HMODULE module)
{
    //
    // hack for File_IsRecoverable
    //

    File_MsoDllLoaded = TRUE;
    return TRUE;
}


//---------------------------------------------------------------------------
// File_Scramble_Char
//---------------------------------------------------------------------------


_FX WCHAR File_Scramble_Char(WCHAR wValue, int Key, BOOLEAN scram)
{
	//
	// This function allows to scramble file name charakters properly, 
	// i.e. no invalid cahacters can result fron this operation.
	// It does not scramble invalid charakters like: " * / : < > ? \ |
	// And it does not scramble ~
	// The entropy of the scrambler is 25,5bit (i.e. 52 million values)
	//

	char reserved_ch[] = { '\"', '*', '/', ':', '<', '>', '?', '\\', '|' };
	const int reserved_count = 9;
	const int max_ch = 0x7E - reserved_count - 0x20;

	int uValue = (wValue & 0x7F);
	if (uValue < 0x20 || uValue >= 0x7E) // < space || >= ~
		return wValue;
	for (int i = 0; i < reserved_count; i++)
		if (uValue == reserved_ch[i]) return wValue;

	Key &= 0x7f;
	while (Key >= max_ch)
		Key -= max_ch;
	if (!scram)
		Key = -Key;

	for (int i = 1; i <= reserved_count; i++)
		if (uValue > reserved_ch[reserved_count - i])	uValue -= 1;
	uValue -= 0x20;

	uValue += Key;

	if (uValue >= max_ch)
		uValue -= max_ch;
	else if (uValue < 0)
		uValue += max_ch;

	uValue += 0x20;
	for (int i = 0; i < reserved_count; i++)
		if (uValue >= reserved_ch[i])	uValue += 1;

	return uValue;
}


//---------------------------------------------------------------------------
// File_ScrambleShortName
//---------------------------------------------------------------------------


_FX void File_ScrambleShortName(WCHAR* ShortName, CCHAR* ShortNameBytes, ULONG ScramKey)
{
	CCHAR ShortNameLength = *ShortNameBytes / sizeof(WCHAR);

	CCHAR dot_pos;
	WCHAR *dot = wcsrchr(ShortName, L'.');
	if (dot == NULL) {
		dot_pos = ShortNameLength;
		if (ShortNameLength >= 12)
			return; // this should never not happen!
		ShortName[ShortNameLength++] = L'.';
	}
	else
		dot_pos = (CCHAR)(dot - ShortName);

	while (ShortNameLength - dot_pos < 4)
	{
		if (ShortNameLength >= 12)
			return; // this should never not happen!
		ShortName[ShortNameLength++] = L' ';
	}

	*ShortNameBytes = ShortNameLength * sizeof(WCHAR);

	if (dot_pos > 0)
		ShortName[dot_pos - 1] = File_Scramble_Char(ShortName[dot_pos - 1], ((char*)&ScramKey)[0], TRUE);
	for (int i = 1; i <= 3; i++)
		ShortName[dot_pos + i] = File_Scramble_Char(ShortName[dot_pos + i], ((char*)&ScramKey)[i], TRUE);
}


//---------------------------------------------------------------------------
// File_UnScrambleShortName
//---------------------------------------------------------------------------


_FX void File_UnScrambleShortName(WCHAR* ShortName, ULONG ScramKey)
{
	CCHAR ShortNameLength = (CCHAR)wcslen(ShortName);

	WCHAR *dot = wcsrchr(ShortName, L'.');
	if (dot == NULL)
		return; // not a scrambled short name.
	CCHAR dot_pos = (CCHAR)(dot - ShortName);

	if (dot_pos > 0)
		ShortName[dot_pos - 1] = File_Scramble_Char(ShortName[dot_pos - 1], ((char*)&ScramKey)[0], FALSE);
	for (int i = 1; i <= 3; i++)
		ShortName[dot_pos + i] = File_Scramble_Char(ShortName[dot_pos + i], ((char*)&ScramKey)[i], FALSE);

	while (ShortName[ShortNameLength - 1] == L' ')
		ShortName[ShortNameLength-- - 1] = 0;
	if (ShortName[ShortNameLength - 1] == L'.')
		ShortName[ShortNameLength-- - 1] = 0;
}
