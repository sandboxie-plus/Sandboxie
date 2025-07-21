/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2022 David Xanatos, xanasoft.com
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
// File (Misc)
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOL File_MoveFileWithProgressA(
    const UCHAR *OldPath, const UCHAR *NewPath,
    void *CallPtr, void *CallData, ULONG dwFlags);

static BOOL File_MoveFileWithProgressW(
    const WCHAR *OldPath, const WCHAR *NewPath,
    void *CallPtr, void *CallData, ULONG dwFlags);

static BOOL File_ReplaceFileW(
    const WCHAR *lpReplacedFileName, const WCHAR *lpReplacementFileName,
    const WCHAR *lpBackupFileName, ULONG dwReplaceFlags,
    void *lpExclude, void *lpReserved);

static BOOL File_ReplaceFileW_2(
    const WCHAR *SourcePath, const WCHAR *TargetPath,
    const WCHAR *BackupPath);

static void File_ReplaceFileW_3(
    HANDLE hFile, WCHAR **FilePath, ULONG *FileFlags,
    const FILE_DRIVE **FileDrive, const FILE_LINK **FileLink);

static BOOL File_DefineDosDeviceW(
    ULONG Flags, void *DevName, void *TargetPath);

static BOOL File_GetVolumeInformationW(
    const WCHAR *lpRootPathName,
    WCHAR *lpVolumeNameBuffer, ULONG nVolumeNameSize,
    ULONG *lpVolumeSerialNumber, ULONG *lpMaximumComponentLength,
    ULONG *lpFileSystemFlags,
    WCHAR *lpFileSystemNameBuffer, ULONG nFileSystemNameSize);

static ULONG File_GetTempPathW(ULONG nBufferLength, WCHAR *lpBuffer);

static BOOL File_WriteProcessMemory(
    HANDLE hProcess,
    LPVOID lpBaseAddress,
    LPCVOID lpBuffer,
    SIZE_T nSize,
    SIZE_T * lpNumberOfBytesWritten);

//---------------------------------------------------------------------------


static P_MoveFileWithProgress       __sys_MoveFileWithProgressW     = NULL;
static P_MoveFileWithProgress       __sys_MoveFileWithProgressA     = NULL;
static P_ReplaceFile                __sys_ReplaceFileW              = NULL;
static P_DefineDosDevice            __sys_DefineDosDeviceW          = NULL;

static P_GetVolumeInformation       __sys_GetVolumeInformationW     = NULL;
static P_WriteProcessMemory         __sys_WriteProcessMemory        = NULL;

static P_GetTempPath                __sys_GetTempPathW              = NULL;


//---------------------------------------------------------------------------
// File_MoveFileWithProgressA
//---------------------------------------------------------------------------


_FX BOOL File_MoveFileWithProgressA(
    const UCHAR *OldPath, const UCHAR *NewPath,
    void *CallPtr, void *CallData, ULONG dwFlags)
{
    dwFlags &= ~MOVEFILE_DELAY_UNTIL_REBOOT;
    dwFlags |= MOVEFILE_COPY_ALLOWED;
    return __sys_MoveFileWithProgressA(
                        OldPath, NewPath, CallPtr, CallData, dwFlags);
}


//---------------------------------------------------------------------------
// File_MoveFileWithProgressW
//---------------------------------------------------------------------------


_FX BOOL File_MoveFileWithProgressW(
    const WCHAR *OldPath, const WCHAR *NewPath,
    void *CallPtr, void *CallData, ULONG dwFlags)
{
    dwFlags &= ~MOVEFILE_DELAY_UNTIL_REBOOT;
    dwFlags |= MOVEFILE_COPY_ALLOWED;
    return __sys_MoveFileWithProgressW(
                        OldPath, NewPath, CallPtr, CallData, dwFlags);
}


//---------------------------------------------------------------------------
// File_ReplaceFileW
//---------------------------------------------------------------------------


_FX BOOL File_ReplaceFileW(
    const WCHAR *lpReplacedFileName, const WCHAR *lpReplacementFileName,
    const WCHAR *lpBackupFileName, ULONG dwReplaceFlags,
    void *lpExclude, void *lpReserved)
{
    BOOL b;

    if (File_ReplaceFileW_2(
            lpReplacementFileName, lpReplacedFileName, lpBackupFileName)) {

        if (lpBackupFileName) {
            b = MoveFileEx(lpReplacedFileName, lpBackupFileName,
                    MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);
            if (! b)
                SetLastError(ERROR_UNABLE_TO_REMOVE_REPLACED);
        } else
            b = TRUE;

        if (b) {
            b = MoveFileEx(lpReplacementFileName, lpReplacedFileName,
                    MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);
            if (! b)
                SetLastError(ERROR_UNABLE_TO_MOVE_REPLACEMENT);
        }

    } else {

        b = __sys_ReplaceFileW(
            lpReplacedFileName, lpReplacementFileName, lpBackupFileName,
            dwReplaceFlags, lpExclude, lpReserved);
    }

    return b;
}


//---------------------------------------------------------------------------
// File_ReplaceFileW_2
//---------------------------------------------------------------------------


_FX BOOL File_ReplaceFileW_2(
    const WCHAR *SourcePath, const WCHAR *TargetPath,
    const WCHAR *BackupPath)
{
    HANDLE hSource, hTarget, hBackup;
    const FILE_DRIVE *SourceDrive, *TargetDrive, *BackupDrive;
    const FILE_LINK  *SourceLink,  *TargetLink,  *BackupLink;
    WCHAR *SourcePath2, *TargetPath2, *BackupPath2;
    ULONG  SourceFlags, TargetFlags, BackupFlags;
    BOOLEAN TargetCreated, BackupCreated;
    BOOL ReturnValue;

    hSource = hTarget = hBackup = NULL;
    SourcePath2 = TargetPath2 = BackupPath2 = NULL;
    TargetCreated = BackupCreated = FALSE;
    ReturnValue = FALSE;

    //
    // open the source path
    //

    hSource = CreateFile(SourcePath, GENERIC_WRITE, FILE_SHARE_VALID_FLAGS,
                         NULL, OPEN_EXISTING, 0, NULL);

    if ((! hSource) || (hSource == INVALID_HANDLE_VALUE))
        goto finish;

    //
    // open the target path
    //

    hTarget = CreateFile(TargetPath, GENERIC_WRITE, FILE_SHARE_VALID_FLAGS,
                         NULL, OPEN_ALWAYS, 0, NULL);

    if ((! hTarget) || (hTarget == INVALID_HANDLE_VALUE))
        goto finish;

    if (GetLastError() != ERROR_ALREADY_EXISTS)
        TargetCreated = TRUE;

    //
    // open the backup path
    //

    if (BackupPath) {

        hBackup = CreateFile(BackupPath, GENERIC_WRITE,
                             FILE_SHARE_VALID_FLAGS, NULL, OPEN_ALWAYS,
                             0, NULL);

        if ((! hBackup) || (hBackup == INVALID_HANDLE_VALUE))
            goto finish;

        if (GetLastError() != ERROR_ALREADY_EXISTS)
            BackupCreated = TRUE;
    }

    //
    // get the paths and flags for each of the files
    // and check if all of the files are inside the box
    //        or if all of the files are outside the box (open paths)
    // and check if all of the files are on the same volume
    //

    File_ReplaceFileW_3(hSource,
        &SourcePath2, &SourceFlags, &SourceDrive, &SourceLink);

    File_ReplaceFileW_3(hTarget,
        &TargetPath2, &TargetFlags, &TargetDrive, &TargetLink);

    if (hBackup) {
        File_ReplaceFileW_3(hBackup,
            &BackupPath2, &BackupFlags, &BackupDrive, &BackupLink);
    } else {
        BackupPath2 = TargetPath2;
        BackupFlags = TargetFlags;
        BackupDrive = TargetDrive;
        BackupLink = TargetLink;
    }

    if (    (SourceFlags && TargetFlags && BackupFlags)
        ||  ((! SourceFlags) && (! TargetFlags) && (! BackupFlags))) {

        //
        // if all files are either inside or outside the sandbox
        // then we can let the original ReplaceFile do the work
        //

        goto finish;
    }

    if (    TargetDrive == SourceDrive && TargetLink == SourceLink
        &&  BackupDrive == SourceDrive && BackupLink == SourceLink) {

        //
        // if all the files are on the same volume
        // then we can let the original ReplaceFile do the work
        //

        goto finish;
    }

    //
    // if some of the files are open and some are sandboxed, and
    // they are not all on the same volume, then we have to do
    // our own implementation of ReplaceFile that does not rely
    // on NtSetInformationFile(FileRenameInformation)
    //

    ReturnValue = TRUE;

    //
    // finish
    //

finish:

    if (BackupPath2 && BackupPath2 != TargetPath2)
        Dll_Free(BackupPath2);
    if (TargetPath2)
        Dll_Free(TargetPath2);
    if (SourcePath2)
        Dll_Free(SourcePath2);

    if (hBackup && hBackup != INVALID_HANDLE_VALUE)
        CloseHandle(hBackup);
    if (hTarget && hTarget != INVALID_HANDLE_VALUE)
        CloseHandle(hTarget);
    if (hSource && hSource != INVALID_HANDLE_VALUE)
        CloseHandle(hSource);

    if (BackupCreated)
        DeleteFile(BackupPath);
    if (TargetCreated)
        DeleteFile(TargetPath);

    return ReturnValue;
}


//---------------------------------------------------------------------------
// File_ReplaceFileW_3
//---------------------------------------------------------------------------


_FX void File_ReplaceFileW_3(
    HANDLE hFile, WCHAR **FilePath, ULONG *FileFlags,
    const FILE_DRIVE **FileDrive, const FILE_LINK **FileLink)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    NTSTATUS status;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    WCHAR *path;
    ULONG len;

    Dll_PushTlsNameBuffer(TlsData);

    RtlInitUnicodeString(&objname, L"");

    __try {

    status = File_GetName(hFile, &objname, &TruePath, &CopyPath, FileFlags);

    if (NT_SUCCESS(status)) {

        (*FileFlags) &= FGN_IS_BOXED_PATH;
        if (*FileFlags) {

            if (File_Snapshot != NULL) {
                WCHAR* TmplName = File_FindSnapshotPath(CopyPath);
                if (TmplName) CopyPath = TmplName;
            }

            len = (wcslen(CopyPath) + 1) * sizeof(WCHAR);
            path = Dll_AllocTemp(len);
            memcpy(path, CopyPath, len);

        } else {

            len = (wcslen(TruePath) + 1) * sizeof(WCHAR);
            path = Dll_AllocTemp(len);
            memcpy(path, TruePath, len);
        }

        *FilePath = path;

        *FileDrive =
            File_GetDriveAndLinkForPath(path, wcslen(path), FileLink);
        LeaveCriticalSection(File_DrivesAndLinks_CritSec);
    }

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    Dll_PopTlsNameBuffer(TlsData);

    if (! NT_SUCCESS(status)) {
        *FilePath = NULL;
        *FileFlags = 0;
        *FileDrive = NULL;
        *FileLink = NULL;
    }
}


//---------------------------------------------------------------------------
// File_DefineDosDeviceW
//---------------------------------------------------------------------------


_FX BOOL File_DefineDosDeviceW(ULONG Flags, void *DevName, void *TargetPath)
{
    SetLastError(ERROR_ACCESS_DENIED);
    return FALSE;
}


//---------------------------------------------------------------------------
// File_CreateBoxedPath
//---------------------------------------------------------------------------


_FX NTSTATUS File_CreateBoxedPath(const WCHAR *PathToCreate)
{
    static const WCHAR *_X = L"\\X";

    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    NTSTATUS status;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    ULONG FileFlags, mp_flags;

    Dll_PushTlsNameBuffer(TlsData);

    RtlInitUnicodeString(&objname, PathToCreate);

    __try {

    //
    // get the file name we're trying to open
    //

    status = File_GetName(NULL, &objname, &TruePath, &CopyPath, &FileFlags);

    if (! NT_SUCCESS(status))
        __leave;

    //
    // check if this is an open or closed path
    //

    mp_flags = File_MatchPath(TruePath, &FileFlags);

    if (PATH_IS_OPEN(mp_flags))
        __leave;

    if (PATH_IS_CLOSED(mp_flags)) {
        status = STATUS_ACCESS_DENIED;
        __leave;
    }

    //
    // create the path.  note that File_CreatePath does not create
    // the last path component, so add a dummy suffix
    //

    wcscat(TruePath, _X);
    wcscat(CopyPath, _X);

    status = File_CreatePath(TruePath, CopyPath);

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
// File_GetVolumeInformationW
//---------------------------------------------------------------------------


//_FX BOOL File_GetVolumeInformationW(
//    const WCHAR *lpRootPathName,
//    WCHAR *lpVolumeNameBuffer, ULONG nVolumeNameSize,
//    ULONG *lpVolumeSerialNumber, ULONG *lpMaximumComponentLength,
//    ULONG *lpFileSystemFlags,
//    WCHAR *lpFileSystemNameBuffer, ULONG nFileSystemNameSize)
//{
//    //
//    // the flash plugin process of Google Chrome issues a special form
//    // of GetVolumeInformationW with all-NULL parameters.  this fails
//    // with an access denied error.  to work around this, we install
//    // this hook, and automatically return TRUE in this special case.
//    //
//
//    // $Workaround$ - 3rd party fix
//    if (Dll_ChromeSandbox &&
//        lpVolumeNameBuffer == NULL && nVolumeNameSize == 0 &&
//        lpVolumeSerialNumber == NULL && lpMaximumComponentLength == NULL &&
//        lpFileSystemFlags == NULL &&
//        lpFileSystemNameBuffer == NULL && nFileSystemNameSize == 0) {
//
//        SetLastError(ERROR_SUCCESS);
//        return TRUE;
//
//    }
//
//    return __sys_GetVolumeInformationW(
//        lpRootPathName, lpVolumeNameBuffer, nVolumeNameSize,
//        lpVolumeSerialNumber, lpMaximumComponentLength,
//        lpFileSystemFlags, lpFileSystemNameBuffer, nFileSystemNameSize);
//}


//---------------------------------------------------------------------------
// File_GetTempPathW
//---------------------------------------------------------------------------


_FX ULONG File_GetTempPathW(ULONG nBufferLength, WCHAR *lpBuffer)
{
    //
    // the temporary files directory may not exist outside the sandbox
    // for a system process, so make sure we create it.  this causes a
    // problem in the MSI Server process on Windows 8
    //

    ULONG rc = __sys_GetTempPathW(nBufferLength, lpBuffer);
    if (rc && rc < nBufferLength) {
        ULONG err = GetLastError();
        if (GetFileAttributes(lpBuffer) == INVALID_FILE_ATTRIBUTES)
            CreateDirectory(lpBuffer, NULL);
        SetLastError(err);
    }
    return rc;
}

//BOOLEAN RpcRt_TestCallingModule(ULONG_PTR pRetAddr, ULONG_PTR hModule);

BOOL File_WriteProcessMemory(
    HANDLE hProcess,
    LPVOID lpBaseAddress,
    LPCVOID lpBuffer,
    SIZE_T nSize,
    SIZE_T * lpNumberOfBytesWritten)
{
    if (!Dll_CompartmentMode) {
    
        // $Workaround$ - 3rd party fix
        if ((Dll_ImageType == DLL_IMAGE_MOZILLA_FIREFOX || Dll_ImageType == DLL_IMAGE_MOZILLA_THUNDERBIRD) &&
            lpBaseAddress && lpBaseAddress == GetProcAddress(Dll_Ntdll, "NtSetInformationThread"))
        //if (RpcRt_TestCallingModule((ULONG_PTR)lpBaseAddress, (ULONG_PTR)Dll_Ntdll))
        {
            if (lpNumberOfBytesWritten)
            {
                *lpNumberOfBytesWritten = nSize;
            }
            return TRUE; // ignore
        }
    }

    extern BOOLEAN Dll_HookTrace;
    if (Dll_HookTrace) {

        WCHAR* pModule = NULL;
        char* pExport = NULL;
        LPVOID pAddress = NULL;
        if (Trace_FindExportByAddress(lpBaseAddress, &pModule, &pExport, &pAddress))
        {
            if (_wcsicmp(Dll_ImageName, pModule) != 0) // ignore self
            {
                WCHAR dbg[1024];
                WCHAR* dbg_ptr = dbg;
                size_t dbg_size = ARRAYSIZE(dbg);
                int len = Sbie_snwprintf(dbg_ptr, dbg_size, L"Application Hooking: %s!%S+0x%Ix [", pModule, pExport, ((UINT_PTR)lpBaseAddress - (UINT_PTR)pAddress));
                dbg_ptr += len;
                dbg_size -= len;
                BufferToHexW(lpBuffer, nSize, dbg_ptr, dbg_size - 1);
                wcscat(dbg_ptr, L"]");
                SbieApi_MonitorPutMsg(MONITOR_HOOK, dbg);
            }
        }
    }

    return __sys_WriteProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten);
}