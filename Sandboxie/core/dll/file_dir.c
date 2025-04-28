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
// File (Dir)
//---------------------------------------------------------------------------


#include "common/pool.h"
#include "common/map.h"
#include "common/pattern.h"

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
    FILE_SNAPSHOT* snapshot;
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

static NTSTATUS File_MergeDummy(
    WCHAR *TruePath, FILE_MERGE_FILE *qfile, UNICODE_STRING *FileMask);

static void File_MergeFree(FILE_MERGE *merge);

static NTSTATUS File_GetMergedInformation(
    FILE_MERGE *merge, WCHAR *TruePath, WCHAR *CopyPath,
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

NTSTATUS File_NtCloseImpl(HANDLE FileHandle);

VOID File_NtCloseDir(HANDLE FileHandle, void* CloseParams);

//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static CRITICAL_SECTION File_CurDir_CritSec;
static WCHAR *File_CurDir_LastInput = NULL;
static WCHAR *File_CurDir_LastOutput = NULL;

static LIST File_DirHandles;
static CRITICAL_SECTION File_DirHandles_CritSec;




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

        BOOLEAN use_rule_specificity = (Dll_ProcessFlags & SBIE_FLAG_RULE_SPECIFICITY) != 0;

        if (PATH_IS_CLOSED(mp_flags))
            status = STATUS_ACCESS_DENIED;

        else if (PATH_IS_WRITE(mp_flags) && !use_rule_specificity) 
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
        merge, TruePath, CopyPath, IoStatusBlock,
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

                Handle_UnRegisterHandler(merge->handle, File_NtCloseDir, NULL);
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

        List_Insert_After(&File_DirHandles, NULL, merge);
        Handle_RegisterHandler(merge->handle, File_NtCloseDir, NULL, FALSE);
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
    WCHAR* OriginalPath = NULL;
    ULONG TruePathFlags = 0;
	BOOLEAN NoCopyPath = FALSE;

    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    Dll_PushTlsNameBuffer(TlsData);

	InitializeObjectAttributes(
		&objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

	//
	// open the copy file
	//

    if (File_Delete_v2) {

        //
        // test if the path is deleted and find the oldest snapshot with a relocation
        //

        WCHAR* OldTruePath = File_ResolveTruePath(TruePath, NULL, &TruePathFlags);
        if (FILE_PATH_DELETED(TruePathFlags) && !FILE_PATH_RELOCATED(TruePathFlags))
            TruePathDeleted = TRUE;
        else if (OldTruePath) {

            OriginalPath = TruePath;

            if (File_Snapshot != NULL) {

                //
                // note: File_ResolveTruePath returns a buffer from the TMPL_NAME_BUFFER slot, 
                // which is reused byFile_MakeSnapshotPath, so we need to make non reusable copy
                // 

        	    TruePath = Dll_GetTlsNameBuffer(TlsData, MISC_NAME_BUFFER, (wcslen(OldTruePath) + 1) * sizeof(WCHAR));
                wcscpy(TruePath, OldTruePath);
            }
            else
                TruePath = OldTruePath;
        }
    }
    else {
        if (File_CheckDeletedParent(CopyPath)) {
            status = STATUS_OBJECT_PATH_NOT_FOUND;
            goto finish;
        }
    }

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

            // if (!File_Delete_v2 &&
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
			goto finish;
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
			goto finish;
	}

	//
	// Now open the parent snapshots if present, and it's aprent and so on....
	//

	if (File_Snapshot != NULL)
	{
        WCHAR* TempPath = TruePath;

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

				status = __sys_NtQueryInformationFile(
					merge->files[merge->files_count].handle, &IoStatusBlock, &info,
					sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);

				if (NT_SUCCESS(status)) {

                    // if (!File_Delete_v2 &&
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
					break; // don't look any further
				}

                merge->files[merge->files_count].snapshot = Cur_Snapshot;

                //
				// copy file passed all checks;  indicate it is ready for use
				//

				merge->files[merge->files_count].more_files = TRUE;
				merge->files[merge->files_count].RestartScan = TRUE;
				merge->files[merge->files_count].scram_key = Cur_Snapshot->ScramKey;
				merge->files_count++;

			}
            // else: ignore the error, proceed to next snapshot

            //
            // check if we have a relocation and update CopyPath for the next snapshot accordingly
            // since we don't need opypath anyware anymore we can alter it
            //

            if (File_Delete_v2) {

                WCHAR* Relocation = NULL;
			    ULONG Flags = File_GetPathFlags_internal(&Cur_Snapshot->PathRoot, TempPath, &Relocation, TRUE);
                if (FILE_PATH_DELETED(Flags))
                    break;

			    if (Relocation) {

				    if (!Cur_Snapshot->Parent) 
					    break; // take a shortcut

				    TempPath = Dll_GetTlsNameBuffer(TlsData, TRUE_NAME_BUFFER, (wcslen(Relocation) + 1) * sizeof(WCHAR));
				    wcscpy(TempPath, Relocation);

				    //
				    // update the copy file name
				    //

				    Dll_PushTlsNameBuffer(TlsData);

				    WCHAR* TruePath2, *CopyPath2;
				    RtlInitUnicodeString(&objname, Relocation);
				    File_GetName(NULL, &objname, &TruePath2, &CopyPath2, NULL);

				    Dll_PopTlsNameBuffer(TlsData);

				    // note: pop leaves TruePath2 valid we can still use it

				    CopyPath = Dll_GetTlsNameBuffer(TlsData, COPY_NAME_BUFFER, (wcslen(CopyPath2) + 1) * sizeof(WCHAR));
				    wcscpy(CopyPath, CopyPath2);
			    }
		    }
		}
	}

	//
	// if there is no copy file, we don't need to merge anything,
	// and can let the system work directly on the true file
	//

    if ((TruePathFlags & FILE_CHILDREN_DELETED_FLAG) == 0) // we need to do full merge if children ar marked deleted
    if (merge->files_count == 0) {

		status = STATUS_BAD_INITIAL_PC;

		goto finish;
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

    // RtlInitUnicodeString(&objname, );
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

    if (status == STATUS_OBJECT_NAME_NOT_FOUND ||
        status == STATUS_OBJECT_PATH_NOT_FOUND ||
        status == STATUS_ACCESS_DENIED) {

        BOOLEAN use_rule_specificity = (Dll_ProcessFlags & SBIE_FLAG_RULE_SPECIFICITY) != 0;

        //
        // if rule specificity is enabled we may not have access to this true path
        // but still have access to some sub paths, in this case instead of listing the
        // true directory we parse the rule list and construct a cached dummy directory
        //

        if (use_rule_specificity) {

            if (File_MergeDummy(TruePath, merge->true_ptr, &merge->file_mask) == STATUS_SUCCESS) {

                merge->true_ptr->handle = NULL;
                status = STATUS_SUCCESS;
            }
        }
    }

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

			goto finish;
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

        //
        // true dir may be actually a dummy dir 
        //

        if (merge->true_ptr->handle) {

            status = File_MergeCache(
                merge->true_ptr, &merge->file_mask, ForceCache);
        }
        
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

finish:
    Dll_PopTlsNameBuffer(TlsData);

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
// File_MergeDummy
//---------------------------------------------------------------------------


_FX NTSTATUS File_MergeDummy(
    WCHAR *TruePath, FILE_MERGE_FILE *qfile, UNICODE_STRING *FileMask)
{
    NTSTATUS status;
    FILE_ID_BOTH_DIR_INFORMATION *info_area;
    FILE_ID_BOTH_DIR_INFORMATION *info_ptr;
    LIST *cache_list;
    FILE_MERGE_CACHE_FILE *cache_file;
    FILE_MERGE_CACHE_FILE *ins_point;
    ULONG len;
    const ULONG INFO_AREA_LEN = 0x10000;  // the size used by cmd.exe


    if (qfile->cache_pool) {
        Pool_Delete(qfile->cache_pool);
        qfile->cache_pool = NULL;
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
    // create a dummy directory, build a sorted files list
    //

    PATTERN* mask = NULL;
    if (FileMask->Buffer) 
        mask = Pattern_Create(qfile->cache_pool, FileMask->Buffer, TRUE, 0);
    WCHAR* test_buf = Pool_Alloc(qfile->cache_pool, 0x1000);

    LIST* lists[4];
    SbieDll_GetReadablePaths(L'f', lists);

    ULONG TruePathLen = wcslen(TruePath);
    if (TruePathLen > 1 && TruePath[TruePathLen - 1] == L'\\')
        TruePathLen--; // never take last \ into account

    ULONG* PrevEntry = NULL;
    info_ptr = info_area;
    for (int i=0; lists[i] != NULL; i++) {

        PATTERN* pat = List_Head(lists[i]);
        while (pat) {

            const WCHAR* patstr = Pattern_Source(pat);

            if (_wcsnicmp(TruePath, patstr, TruePathLen) == 0 && patstr[TruePathLen] == L'\\') {

                const WCHAR* ptr = &patstr[TruePathLen + 1];
                WCHAR* end = wcschr(ptr, L'\\');
                if(end == NULL) end = wcschr(ptr, L'*');
                if(end == NULL) end = wcschr(ptr, L'\0');
                ULONG name_len = (ULONG)(end - ptr);

                if (mask) {
                    
                    memcpy(test_buf, ptr, (name_len + 1) * sizeof(WCHAR));
                    _wcslwr(test_buf);

                    if (!Pattern_Match(mask, test_buf, name_len))
                        goto next;
                }

                //
                // check if the true path exists
                //

                WCHAR* FakePath = Dll_AllocTemp(TruePathLen * sizeof(WCHAR) + 1 + name_len * sizeof(WCHAR) + 10);

                wmemcpy(FakePath, TruePath, TruePathLen);
                FakePath[TruePathLen] = L'\\';
                end = &FakePath[TruePathLen + 1];
                *end = L'\0';
                wmemcpy(end, ptr, name_len);
                end[name_len] = L'\0';

                FILE_NETWORK_OPEN_INFORMATION info;
                status = File_QueryFullAttributesDirectoryFile(FakePath, &info);

                Dll_Free(FakePath);

                //
                // add directory entry
                //

                if (NT_SUCCESS(status)) {

                    info_ptr->FileNameLength = name_len * sizeof(WCHAR);
                    memcpy(info_ptr->FileName, ptr, info_ptr->FileNameLength);
                    info_ptr->FileName[info_ptr->FileNameLength] = L'\0';

                    info_ptr->CreationTime = info.CreationTime;
                    info_ptr->LastAccessTime = info.LastAccessTime;
                    info_ptr->LastWriteTime = info.LastWriteTime;
                    info_ptr->ChangeTime = info.ChangeTime;
                    info_ptr->AllocationSize = info.AllocationSize;
                    info_ptr->EndOfFile = info.EndOfFile;
                    info_ptr->FileAttributes = info.FileAttributes;

                    
    //ULONG           NextEntryOffset;
    //ULONG           FileIndex;
    //ULONG           EaInformationLength;
    //CCHAR           ShortNameLength;
    //WCHAR           ShortName[12];
    //LARGE_INTEGER   FileId;
                    

                    info_ptr->FileId.QuadPart = -1;

                    PrevEntry = &info_ptr->NextEntryOffset;

                    info_ptr->NextEntryOffset = sizeof(FILE_ID_BOTH_DIR_INFORMATION) + info_ptr->FileNameLength + sizeof(WCHAR) + 16; // +16 some buffer space

                    ULONG tmp = (info_ptr->NextEntryOffset & 0x07);
                    if (tmp != 0) // fix alignment when needed
                        info_ptr->NextEntryOffset += 0x8 - tmp;

                    info_ptr = (FILE_ID_BOTH_DIR_INFORMATION*)
                        ((UCHAR*)info_ptr + info_ptr->NextEntryOffset);

                    // todo: fix-me possible info_area buffer overflow!!!!
                }
            }

        next:
            pat = List_Next(pat);
        }
    }

    Pool_Free(test_buf, 0x1000);

    status = STATUS_SUCCESS;

    SbieDll_ReleaseFilePathLock();

    if(mask)
        Pattern_Free(mask);

    if (PrevEntry == NULL) {
        
        // no dummys created

        Pool_Delete(qfile->cache_pool);
        qfile->cache_pool = NULL;

        return STATUS_NO_MORE_ENTRIES;
    }
    *PrevEntry = 0;

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

        if (cmp != 0) { // skip duplicates

            if (ins_point)
                List_Insert_Before(cache_list, ins_point, cache_file);
            else
                List_Insert_After(cache_list, NULL, cache_file);
        }

        if (info_ptr->NextEntryOffset == 0)
            break;
        info_ptr = (FILE_ID_BOTH_DIR_INFORMATION *)
            ((UCHAR *)info_ptr + info_ptr->NextEntryOffset);
    }

    Pool_Free(info_area, INFO_AREA_LEN);

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
    FILE_MERGE *merge, WCHAR *TruePath, WCHAR *CopyPath,
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
    ULONG TruePathLen = wcslen(TruePath);
    ULONG CopyPathLen = wcslen(CopyPath);

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

    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    Dll_PushTlsNameBuffer(TlsData);

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

				if (cmp == 0) // equal - same file in both, use newer (best)
					cur->have_entry = FALSE;
				else if (cmp > 0)
					best = cur;
			}
		}

		if (best->have_entry) {
            ptr_info = best->info;
			best->have_entry = FALSE;
        }

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

        //if (ptr_info->FileId.QuadPart == -1) {
        //    WCHAR msg[1024];
        //    Sbie_snwprintf(msg, 1024, L"File_MergeDummy simulate %s", ptr_info->FileName);
        //    SbieApi_MonitorPutMsg(MONITOR_OTHER | MONITOR_TRACE, msg);
        //}

        if (File_Delete_v2) {

            if ((merge->true_ptr && ptr_info == merge->true_ptr->info) // is in true path
                || ptr_info != merge->files[0].info) { // is in template

                WCHAR* TruePath2 = Dll_GetTlsNameBuffer(TlsData, TRUE_NAME_BUFFER, ((TruePathLen + 1) * sizeof(WCHAR) + ptr_info->FileNameLength + sizeof(WCHAR)));
                WCHAR* ptr = TruePath2;
                wmemcpy(ptr, TruePath, TruePathLen);
                ptr += TruePathLen;
                if (ptr[-1] != L'\\') *ptr++ = L'\\';
                wmemcpy(ptr, ptr_info->FileName, ptr_info->FileNameLength / sizeof(WCHAR));
                ptr += ptr_info->FileNameLength / sizeof(WCHAR);
                *ptr = L'\0';

                WCHAR* CopyPath2 = Dll_GetTlsNameBuffer(TlsData, COPY_NAME_BUFFER, ((CopyPathLen + 1) * sizeof(WCHAR) + ptr_info->FileNameLength + sizeof(WCHAR)));
                ptr = CopyPath2;
                wmemcpy(ptr, CopyPath, CopyPathLen);
                ptr += CopyPathLen;
                if (ptr[-1] != L'\\') *ptr++ = L'\\';
                wmemcpy(ptr, ptr_info->FileName, ptr_info->FileNameLength / sizeof(WCHAR));
                ptr += ptr_info->FileNameLength / sizeof(WCHAR);
                *ptr = L'\0';

                //
                // check if the file is listed as deleted
                //

                if (File_IsDeletedEx(TruePath2, CopyPath2, best->snapshot))
                    continue;

            } //else // is in copy path - nothing to do
        }
        else {

		    // if the entry found was in the copy directory, then the file
		    // may be marked deleted (see Filesys_Mark_File_Deleted for
		    // details).  if it is marked so, we pretend this entry does
		    // not exist by fetching the following one

            if ((!merge->true_ptr || ptr_info != merge->true_ptr->info) &&
                IS_DELETE_MARK(&ptr_info->CreationTime))
                continue;
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
                status = STATUS_BUFFER_OVERFLOW;
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

        // This structure must be aligned on a LONGLONG (8-byte) 
        // boundary. If a buffer contains two or more of these 
        // structures, the NextEntryOffset value in each entry, 
        // except the last, falls on an 8-byte boundary.
        
        ULONG tmp = (*(ULONG*)next_entry & 0x07);
        if (tmp != 0) // fix alignment when needed
            *(ULONG*)next_entry += 0x8 - tmp;

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

    Dll_PopTlsNameBuffer(TlsData);

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
    BOOLEAN DeleteOnClose = FALSE;
    UNICODE_STRING uni;
    WCHAR *DeletePath = NULL;

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
    // check the handle map and execute the close handlers if there is are any
    // and prepare the DeleteOnClose if its set
    //

    Handle_ExecuteCloseHandler(FileHandle, &DeleteOnClose);

    //
    // prepare delete disposition if set
    //

    if (DeleteOnClose) {

        Dll_PushTlsNameBuffer(TlsData);

        __try {

            WCHAR *TruePath, *CopyPath;
            ULONG FileFlags;

            RtlInitUnicodeString(&uni, L"");
            status = File_GetName(
                FileHandle, &uni, &TruePath, &CopyPath, &FileFlags);

            ULONG len = wcslen(TruePath);
            DeletePath = Dll_AllocTemp((len + 8) * sizeof(WCHAR));
            wmemcpy(DeletePath, TruePath, len + 1);

            if (Dll_ChromeSandbox) {

                //
                // if this is a Chrome sandbox process, we have
                // to pass a DOS path to NtDeleteFile rather
                // than a file handle
                //

                if (SbieDll_TranslateNtToDosPath(DeletePath)) {
                    len = wcslen(DeletePath);
                    wmemmove(DeletePath + 4, DeletePath, len + 1);
                    wmemcpy(DeletePath, File_BQQB, 4);
                }
            }

        } __except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
        }

        Dll_PopTlsNameBuffer(TlsData);
    }

    //
    // close the handle
    //

    status = pSysNtClose ? pSysNtClose(FileHandle) : NtClose(FileHandle);

    //
    // finish
    //

    TlsData->file_NtClose_lock = FALSE;

    //
    // execute pending delete disposition
    //

    if (DeletePath) {
            
        OBJECT_ATTRIBUTES objattrs;
        RtlInitUnicodeString(&uni, DeletePath);
        InitializeObjectAttributes(
            &objattrs, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);
            
        File_NtDeleteFileImpl(&objattrs);
    
        Dll_Free(DeletePath);
    }

    SetLastError(LastError);
    return status;
}


//---------------------------------------------------------------------------
// File_NtCloseDir
//---------------------------------------------------------------------------


_FX VOID File_NtCloseDir(HANDLE FileHandle, void* CloseParams)
{
    FILE_MERGE *merge;

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

    handle = FileHandle;

    status = SbieDll_GetHandlePath(FileHandle, NULL, &IsBoxedPath);
    if (IsBoxedPath && (
            NT_SUCCESS(status) || (status == STATUS_BAD_INITIAL_PC))) {
        
        path = Dll_AllocTemp(8192);
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

        Dll_Free(path);
    }


    status = __sys_NtQueryVolumeInformationFile(
        handle, IoStatusBlock, FsInformation, Length, FsInformationClass);

    if (handle != FileHandle)
        __sys_NtClose(handle);

    return status;
}


//---------------------------------------------------------------------------
// File_CanonizePath
//---------------------------------------------------------------------------


WCHAR* File_CanonizePath(const wchar_t* absolute_path, ULONG abs_path_len, const wchar_t* relative_path, ULONG rel_path_len) 
{
    ULONG i, j;
    
    while(absolute_path[abs_path_len-1] == L'\\')
        abs_path_len--;

    WCHAR* result = (WCHAR*)Dll_Alloc((abs_path_len + rel_path_len + 1) * sizeof(wchar_t));
    if (!result) return NULL;
    wcsncpy(result, absolute_path, abs_path_len);
    result[abs_path_len] = 0;

    for (i = 0; i < rel_path_len; ) {

        if (relative_path[i] == L'.' && relative_path[i + 1] == L'.' && (relative_path[i + 2] == L'\\' || relative_path[i + 2] == L'\0')) {

            for (j = abs_path_len - 1; j >= 0 && result[j] != L'\\'; --j)
                result[j] = L'\0';
            if (j >= 0)
                result[j] = L'\0';

            abs_path_len = j;

            i += 3;

        } else if (relative_path[i] == L'.' && (relative_path[i + 1] == L'\\' || relative_path[i + 1] == L'\0')) {

            i += 2;

        } else {

            for (j = i; j < rel_path_len && relative_path[j] != L'\\' && relative_path[j] != L'\0'; ++j)
                ;

            if (abs_path_len > 0 && result[abs_path_len - 1] != L'\\') {
                result[abs_path_len] = L'\\';
                abs_path_len++;
            }
            
            wcsncpy(result + abs_path_len, &relative_path[i], j - i);
            result[abs_path_len + j - i] = L'\0';

            abs_path_len += j - i;

            i = j + (relative_path[j] == L'\\' ? 1 : 0);
        }
    }

    return result;
}


//---------------------------------------------------------------------------
// File_SetReparsePoint
//---------------------------------------------------------------------------


_FX NTSTATUS File_SetReparsePoint(
    HANDLE FileHandle, PREPARSE_DATA_BUFFER Data, ULONG DataLen)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    NTSTATUS status;
    UNICODE_STRING objname;
    OBJECT_ATTRIBUTES objattrs;
    WCHAR *TruePath, *CopyPath;
    //WCHAR *SourcePath = NULL, *TargetPath = NULL;
    WCHAR* AbsolutePath = NULL;
    ULONG FileFlags, mp_flags;
    PREPARSE_DATA_BUFFER NewData = NULL;
    ULONG NewDataLen;
    IO_STATUS_BLOCK MyIoStatusBlock;
    BOOLEAN MigrateTarget = FALSE;

    if (! Data)
        return STATUS_BAD_INITIAL_PC;

    //
    // get paths to source and target directories
    //

    Dll_PushTlsNameBuffer(TlsData);

    __try {
        USHORT SubstituteNameLength;
        WCHAR* SubstituteNameBuffer;
        USHORT PrintNameLength;
        WCHAR* PrintNameBuffer;

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

        //
        // get the absolute reparse target path
        //

        if (Data->ReparseTag == IO_REPARSE_TAG_SYMLINK)
        {
            SubstituteNameLength = Data->SymbolicLinkReparseBuffer.SubstituteNameLength;
            SubstituteNameBuffer = &Data->SymbolicLinkReparseBuffer.PathBuffer[Data->SymbolicLinkReparseBuffer.SubstituteNameOffset/sizeof(WCHAR)];
            PrintNameLength = Data->SymbolicLinkReparseBuffer.PrintNameLength;
            PrintNameBuffer = &Data->SymbolicLinkReparseBuffer.PathBuffer[Data->SymbolicLinkReparseBuffer.PrintNameOffset/sizeof(WCHAR)];
            if (Data->SymbolicLinkReparseBuffer.Flags & SYMLINK_FLAG_RELATIVE) {

                WCHAR* LinkName = wcsrchr(TruePath, L'\\');
                AbsolutePath = File_CanonizePath(TruePath, (ULONG)(LinkName - TruePath), SubstituteNameBuffer, SubstituteNameLength / sizeof(wchar_t));
            }

            NewDataLen = (UFIELD_OFFSET(REPARSE_DATA_BUFFER, SymbolicLinkReparseBuffer.PathBuffer) - UFIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer));
        }
        else if (Data->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT)
        {
            SubstituteNameLength = Data->MountPointReparseBuffer.SubstituteNameLength;
            SubstituteNameBuffer = &Data->MountPointReparseBuffer.PathBuffer[Data->MountPointReparseBuffer.SubstituteNameOffset/sizeof(WCHAR)];
            PrintNameLength = Data->MountPointReparseBuffer.PrintNameLength;
            PrintNameBuffer = &Data->MountPointReparseBuffer.PathBuffer[Data->MountPointReparseBuffer.PrintNameOffset/sizeof(WCHAR)]; 

            NewDataLen = (UFIELD_OFFSET(REPARSE_DATA_BUFFER, MountPointReparseBuffer.PathBuffer) - UFIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer));
        }
        else {
            status = STATUS_BAD_INITIAL_PC;
            __leave;
        }

		//if (File_Snapshot != NULL){
        // WCHAR* TmplName = File_FindSnapshotPath(CopyPath);
        // if (TmplName) CopyPath = TmplName;
		//}
        
        //SourcePath = Dll_Alloc((wcslen(CopyPath) + 4) * sizeof(WCHAR));
        //wcscpy(SourcePath, CopyPath);

        //
        // get copy path of reparse target
        //

        if (AbsolutePath) {
            objname.Length = wcslen(AbsolutePath) * sizeof(wchar_t);
            objname.Buffer = AbsolutePath;
        } else {
            objname.Length = SubstituteNameLength;
            objname.Buffer = SubstituteNameBuffer;
        }
        objname.MaximumLength = objname.Length;

        status = File_GetName(NULL, &objname, &TruePath, &CopyPath, NULL);
        if (! NT_SUCCESS(status))
            __leave;

        if (AbsolutePath) {

            //
            // We can allow for a relative path in the box but must ensure the hatget gets migrated
            //
                
            MigrateTarget = TRUE;
            status = STATUS_BAD_INITIAL_PC;
            __leave;
        }

        //TargetPath = Dll_Alloc((wcslen(CopyPath) + 4) * sizeof(WCHAR));
        //wcscpy(TargetPath, CopyPath);

        WCHAR* NewSubstituteNameBuffer = CopyPath;
        WCHAR* OldPrintNameBuffer = PrintNameBuffer; // we don't need to change the display name
        
        if (Data->ReparseTag == IO_REPARSE_TAG_SYMLINK) {

            SbieDll_TranslateNtToDosPath(NewSubstituteNameBuffer);
            memmove(NewSubstituteNameBuffer + 4, NewSubstituteNameBuffer, (wcslen(NewSubstituteNameBuffer) + 1) * sizeof(wchar_t));
            wcsncpy(NewSubstituteNameBuffer, L"\\??\\", 4);
        }

        SubstituteNameLength = wcslen(NewSubstituteNameBuffer) * sizeof(WCHAR);

        NewDataLen += SubstituteNameLength + sizeof(WCHAR) + PrintNameLength + sizeof(WCHAR) + 8;
        NewData = Dll_Alloc(NewDataLen);
        memzero(NewData, sizeof(REPARSE_DATA_BUFFER));

        NewData->ReparseTag = Data->ReparseTag;
        NewData->Reserved = 0; //Data->Reserved;
        NewData->ReparseDataLength = (USHORT)NewDataLen - 8;

        if (NewData->ReparseTag == IO_REPARSE_TAG_SYMLINK)
        {
            NewData->SymbolicLinkReparseBuffer.SubstituteNameOffset = 0;
            NewData->SymbolicLinkReparseBuffer.SubstituteNameLength = SubstituteNameLength;
            SubstituteNameBuffer = &NewData->SymbolicLinkReparseBuffer.PathBuffer[NewData->SymbolicLinkReparseBuffer.SubstituteNameOffset/sizeof(WCHAR)];
            NewData->SymbolicLinkReparseBuffer.PrintNameLength = PrintNameLength;
            NewData->SymbolicLinkReparseBuffer.PrintNameOffset = SubstituteNameLength + sizeof(WCHAR);
            PrintNameBuffer = &NewData->SymbolicLinkReparseBuffer.PathBuffer[NewData->SymbolicLinkReparseBuffer.PrintNameOffset/sizeof(WCHAR)];
            
        }
        else if (NewData->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT)
        {
            NewData->MountPointReparseBuffer.SubstituteNameOffset = 0;
            NewData->MountPointReparseBuffer.SubstituteNameLength = SubstituteNameLength;
            SubstituteNameBuffer = &NewData->MountPointReparseBuffer.PathBuffer[NewData->MountPointReparseBuffer.SubstituteNameOffset/sizeof(WCHAR)];
            NewData->MountPointReparseBuffer.PrintNameLength = PrintNameLength;
            NewData->MountPointReparseBuffer.PrintNameOffset = SubstituteNameLength + sizeof(WCHAR);
            PrintNameBuffer = &NewData->MountPointReparseBuffer.PathBuffer[NewData->MountPointReparseBuffer.PrintNameOffset/sizeof(WCHAR)]; 
        }

        memcpy(SubstituteNameBuffer, NewSubstituteNameBuffer, SubstituteNameLength + sizeof(WCHAR));
        memcpy(PrintNameBuffer, OldPrintNameBuffer, PrintNameLength + sizeof(WCHAR));

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    //
    // since Curt's code in the driver handles reparsing and the driver is no longer blocking this operation,
    // we can do it directly without the need to ask our service
    //

    if (NT_SUCCESS(status)) {

        status = __sys_NtFsControlFile(
            FileHandle, NULL, NULL, NULL,
            &MyIoStatusBlock, FSCTL_SET_REPARSE_POINT,
            NewData, NewDataLen,
            NULL, 0);

        MigrateTarget = NT_SUCCESS(status);
    }

    /*
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
        Dll_Free(TargetPath);*/

    if (MigrateTarget) {

        //
        // We must migrate the file or directory into the sandbox as the path reparsing by NtCreateFile
        // is done by the kernel and we do not "manually" reparse the path before invoking it,
        // hence there must be the expected file at the path we are linking to.
        //

        HANDLE SourceHandle;
        if (NT_SUCCESS(File_OpenForRenameFile(&SourceHandle, TruePath)))
            NtClose(SourceHandle);
    }

    if (AbsolutePath)
        Dll_Free(AbsolutePath);

    if (NewData)
        Dll_Free(NewData);

    Dll_PopTlsNameBuffer(TlsData);

    return status;
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
// File_CreateBaseFolders
//---------------------------------------------------------------------------

//#include <Knownfolders.h>

_FX void File_CreateBaseFolders()
{
    NTSTATUS status;
    WCHAR conf_buf[2048];

    //
    // in privacy mode we need to pre create some folders or else programs may fail
    //

    //File_CreateBoxedPath(File_SysVolume);
    // 
    //if (SbieApi_QueryConfBool(NULL, L"SeparateUserFolders", TRUE)) {
    //    File_CreateBoxedPath(File_AllUsers);
    //    File_CreateBoxedPath(File_CurrentUser);
    //}

    for (ULONG index = 0; ; ++index) {

        status = SbieApi_QueryConf(
            L"TemplateDefaultFolders", L"DefaultFolder", index | CONF_GET_NO_GLOBAL, conf_buf, sizeof(conf_buf) - 16 * sizeof(WCHAR));
        if (!NT_SUCCESS(status))
            break;

        File_CreateBoxedPath(conf_buf);
    }

    for (ULONG index = 0; ; ++index) {

        status = SbieApi_QueryConf(
            NULL, L"DefaultFolder", index | CONF_GET_NO_EXPAND, conf_buf, sizeof(conf_buf) - 16 * sizeof(WCHAR));
        if (!NT_SUCCESS(status))
            break;

        WCHAR expanded[MAX_PATH];
        DWORD len = ExpandEnvironmentStringsW(conf_buf, expanded, MAX_PATH);
        if (len == 0 || len > MAX_PATH || wcschr(expanded, L'%'))
            continue;

        WCHAR* pathNT = File_TranslateDosToNtPath(expanded);
        if (pathNT) {
            File_CreateBoxedPath(pathNT);
            Dll_Free(pathNT);
        }
    }
}
