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
//
// Immediate Recovery for Files
//
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static void File_InitRecoverFolders(void);

static void File_InitRecoverList(
    const WCHAR *setting, LIST *list, BOOLEAN MustBeValidPath,
    WCHAR *buf, ULONG buf_len);

static BOOLEAN File_IsRecoverable(const WCHAR *TruePath);

static void File_DoAutoRecover_2(BOOLEAN force, ULONG ticks);

static ULONG File_DoAutoRecover_3(
    const WCHAR *PathToFind, WCHAR *PathBuf1024,
    SYSTEM_HANDLE_INFORMATION *info, FILE_GET_ALL_HANDLES_RPL *rpl,
    UCHAR *FileObjectTypeNumber);

static ULONG File_DoAutoRecover_4(
    const WCHAR *PathToFind, WCHAR *PathBuf1024,
    HANDLE FileHandle, UCHAR ObjectTypeNumber, UCHAR *FileObjectTypeNumber);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static LIST File_RecoverFolders;
static LIST File_RecoverIgnores;

static LIST File_RecPaths;
static CRITICAL_SECTION File_RecHandles_CritSec;

static BOOLEAN File_MsoDllLoaded = FALSE;

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
    BOOLEAN IsRecoverable;

    IsRecoverable = File_IsRecoverable(TruePath);

    //
    // in a Chrome sandbox process, handles are opened by the broker,
    // so skip checking against the list of recorded file handles
    //
    
    // Note: this does not seam to be required anymore 
    
    //if ((! IsRecoverable) && Dll_ChromeSandbox) {
    //
    //    FILE_ACCESS_INFORMATION info;
    //
    //    status = __sys_NtQueryInformationFile(
    //        FileHandle, &IoStatusBlock, &info,
    //        sizeof(FILE_ACCESS_INFORMATION), FileAccessInformation);
    //
    //    if (NT_SUCCESS(status) && (info.AccessFlags & FILE_WRITE_DATA))
    //        IsRecoverable = 2; //TRUE; // we still want to return false when called from File_NtCreateFileImpl
    //    else
    //        IsRecoverable = FALSE;
    //}

    if (IsRecoverable != FALSE)
        Handle_RegisterCloseHandler(FileHandle, File_NotifyRecover);

    return IsRecoverable == TRUE;
}


//---------------------------------------------------------------------------
// File_NotifyRecover
//---------------------------------------------------------------------------


_FX void File_NotifyRecover(HANDLE FileHandle)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    NTSTATUS status;
    union {
        FILE_NETWORK_OPEN_INFORMATION open;
        ULONG space[16];
    } info;
    ULONG length;
    ULONG FileFlags;
    UNICODE_STRING uni;
    WCHAR *TruePath, *CopyPath;
    IO_STATUS_BLOCK IoStatusBlock;
   
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

        if (!File_IsRecoverable(TruePath))
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
        
    } while (0);

    Dll_PopTlsNameBuffer(TlsData);
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

    if (!Dll_CompartmentMode) // NoServiceAssist // don't try that in app mode, we had a proepr token
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
			if (!colon) {

                UNICODE_STRING uni;
                WCHAR *TruePath, *CopyPath;
                RtlInitUnicodeString(&uni, rec->path);
                status = File_GetName(NULL, &uni, &TruePath, &CopyPath, NULL);

				const WCHAR* strings[] = { Dll_BoxName, rec->path, CopyPath, NULL };
				SbieApi_LogMsgExt(2199, strings);
			}
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

    status = File_GetFileName(FileHandle, 1000, PathBuf1024);
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