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
// Delete Contents of Sandbox
//---------------------------------------------------------------------------

#include "stdafx.h"

#include <shellapi.h>
#include "common/win32_ntddk.h"
#include "common/defines.h"
#include "core/dll/sbiedll.h"
#include "msgs/msgs.h"
#include "common/my_version.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


extern void Show_Error(WCHAR *Descr);

extern void DeleteSandbox(
    const WCHAR *BoxName, BOOL bLogoff, BOOL bSilent, int phase);

static void RenameSandbox(void);
static void LaunchPhase2(void);
static void DeleteFiles(void);
static void DeleteFilesInBox(const WCHAR *boxname);
static void TranslateCommand(
    WCHAR *cmd1, WCHAR *cmd2, const WCHAR *BoxFolder);
static void WaitForFolder(const WCHAR *folder, ULONG seconds);
static void ProcessFiles(const WCHAR *BoxPath);

static WCHAR *GetBoxFilePath(const WCHAR *boxname, ULONG extra);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static const WCHAR *g_BoxName;
static bool g_Logoff;
static bool g_Silent;
static HANDLE g_event_handle = NULL;


//---------------------------------------------------------------------------
// Error
//---------------------------------------------------------------------------


void Error(const WCHAR *Descr, NTSTATUS Status)
{
    if (g_event_handle) {
        CloseHandle(g_event_handle);
        g_event_handle = NULL;
    }

    if (! g_Silent) {

        WCHAR text[512];
        wcscpy(text, SbieDll_FormatMessage1(MSG_3214, g_BoxName));
        wcscat(text, Descr);
        if (Status) {
            if (Status == STATUS_ACCESS_DENIED ||
                Status == STATUS_SHARING_VIOLATION)
            {
                wcscat(text, L"\n\n");
                wcscat(text,
                    SbieDll_FormatMessage0(MSG_3215));
            }
            SetLastError(RtlNtStatusToDosError(Status));
        }
        Show_Error(text);
    }

    if (g_Logoff)
        ExitWindowsEx(EWX_LOGOFF, 0);

    ExitProcess(1);
}


//---------------------------------------------------------------------------
// DeleteSandbox
//---------------------------------------------------------------------------


void DeleteSandbox(
    const WCHAR *BoxName, BOOL bLogoff, BOOL bSilent, int phase)
{
    g_BoxName = BoxName;
    g_Logoff = bLogoff ? true : false;
    g_Silent = bSilent ? true : false;

    if (SbieApi_QueryProcess(NULL, NULL, NULL, NULL, NULL) == 0) {
        SetLastError(0);
        Error(SbieDll_FormatMessage0(MSG_3221), 0);
    }

    if ((phase <= 1) &&
            SbieApi_QueryConfBool(g_BoxName, L"NeverDelete", FALSE)) {

        SetLastError(0);
        Error(SbieDll_FormatMessage0(MSG_3051), 0);

    } else {

        WCHAR evname[128];
        ULONG session_id = 0;
        if (! ProcessIdToSessionId(GetCurrentProcessId(), &session_id))
            session_id = 0;
        wsprintf(evname,
                 SANDBOXIE L"_Delete_Sandbox_Session_%d", session_id);
        g_event_handle = CreateEvent(NULL, FALSE, FALSE, evname);

        if (phase == 2) {
            DeleteFiles();
            if (g_Logoff)
                ExitWindowsEx(EWX_LOGOFF, 0);
        } else if (phase <= 1) {
            RenameSandbox();
            if (phase == 0)
                LaunchPhase2();
        }
    }

    ExitProcess(0);
}


//---------------------------------------------------------------------------
//
// DELETE FILES
//
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// RenameSandbox
//---------------------------------------------------------------------------


void RenameSandbox(void)
{
    NTSTATUS status;
    union {
        FILE_RENAME_INFORMATION info;
        WCHAR space[128];
    } u;
    UNICODE_STRING uni;
    OBJECT_ATTRIBUTES objattrs;
    IO_STATUS_BLOCK MyIoStatusBlock;
    HANDLE hSandbox, hTopLevel;

    //
    // open sandbox folder: root\Sandbox\user\BoxName
    //

    WCHAR *boxpath = GetBoxFilePath(g_BoxName, 128);
    if (! boxpath)
        Error(SbieDll_FormatMessage0(MSG_3216), 0);

    RtlMoveMemory(
        boxpath + 4, boxpath, (wcslen(boxpath) + 1) * sizeof(WCHAR));
    RtlCopyMemory(boxpath, L"\\??\\", 4 * sizeof(WCHAR));

    RtlInitUnicodeString(&uni, boxpath);
    InitializeObjectAttributes(
        &objattrs, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = NtCreateFile(
        &hSandbox, DELETE | SYNCHRONIZE, &objattrs,
        &MyIoStatusBlock, NULL, 0,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN, FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
        NULL, NULL);

    if (status != STATUS_SUCCESS) {

        if (status == STATUS_OBJECT_NAME_NOT_FOUND ||
            status == STATUS_OBJECT_PATH_NOT_FOUND)
        {
            // no sandbox folder -- nothing to do
            return;
        }

        Error(SbieDll_FormatMessage0(MSG_3217), status);
    }

    //
    // open top level folder: root\Sandbox\user
    //

    WCHAR *backslash = wcsrchr(boxpath, L'\\');
    if (backslash - boxpath == 6)
        ++backslash;        // "\??\X:" --> "\??\X:\" .
    *backslash = L'\0';
    RtlInitUnicodeString(&uni, boxpath);

    status = NtCreateFile(
        &hTopLevel, FILE_GENERIC_READ, &objattrs,
        &MyIoStatusBlock, NULL, 0,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN, FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
        NULL, NULL);

    if (status != STATUS_SUCCESS)
        Error(SbieDll_FormatMessage0(MSG_3218), status);

    // rename sandbox using Nt file operations

    u.info.ReplaceIfExists = FALSE;
    u.info.RootDirectory = hTopLevel;

    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    wsprintf(u.info.FileName, L"__Delete_%s_%08X%08X",
             g_BoxName, ft.dwHighDateTime, ft.dwLowDateTime);
    u.info.FileNameLength = wcslen(u.info.FileName) * sizeof(WCHAR);

    ULONG retries = 0;
    while (1) {

        status = NtSetInformationFile(
            hSandbox, &MyIoStatusBlock,
            &u.info, sizeof(u), FileRenameInformation);

        if (NT_SUCCESS(status))
            break;

        if (status == STATUS_ACCESS_DENIED ||
            status == STATUS_SHARING_VIOLATION) {

            ULONG pids[512];
            SbieApi_EnumProcess(g_BoxName, pids);
            if (pids[0]) {

                SetLastError(0);
                Error(SbieDll_FormatMessage0(MSG_3221), 0);
            }
        }

        if (retries == 30)
            Error(SbieDll_FormatMessage0(MSG_3219), status);

        ++retries;
        Sleep(200);
    }
}


//---------------------------------------------------------------------------
// LaunchProgram
//---------------------------------------------------------------------------


void LaunchProgram(WCHAR *cmdSrc, bool bWait)
{
    WCHAR cmd[768];
    ExpandEnvironmentStrings(cmdSrc, cmd, 760);

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_FORCEOFFFEEDBACK | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

    BOOL ok = CreateProcess(
        NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    if (ok && bWait) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        ULONG ExitCode = 0;
        if (GetExitCodeProcess(pi.hProcess, &ExitCode) && ExitCode != 0) {
            CloseHandle(pi.hProcess);
            SetLastError(ExitCode);
            ok = FALSE;
        } else
            CloseHandle(pi.hProcess);
    }

    if (! ok) {
        WCHAR txt[1024];
        wcscpy(txt, SbieDll_FormatMessage0(MSG_3222));
        wcscat(txt, cmd);
        Error(txt, 0);
    }
}


//---------------------------------------------------------------------------
// LaunchPhase2
//---------------------------------------------------------------------------


NOINLINE void LaunchPhase2(void)
{
    WCHAR cmd[512];

    cmd[0] = L'\"';
    GetModuleFileName(NULL, &cmd[1], 500);
    wcscat(cmd, L"\" delete_sandbox");
    if (g_Logoff)
        wcscat(cmd, L"_logoff");
    if (g_Silent)
        wcscat(cmd, L"_silent");
    wcscat(cmd, L"_phase2");

    LaunchProgram(cmd, FALSE);
}


//---------------------------------------------------------------------------
// DeleteFiles
//---------------------------------------------------------------------------


NOINLINE void DeleteFiles(void)
{
    WCHAR boxname[48];
    int index = -1;
    while (1) {
        index = SbieApi_EnumBoxes(index, boxname);
        if (index == -1)
            break;
        g_BoxName = boxname;
        DeleteFilesInBox(boxname);
    }
}


//---------------------------------------------------------------------------
// DeleteFilesInBox
//---------------------------------------------------------------------------


void DeleteFilesInBox(const WCHAR *boxname)
{
    static const WCHAR *_DeleteCommand = L"DeleteCommand";
    static const WCHAR *_DefaultDeleteCommand =
        L"%SystemRoot%\\System32\\cmd.exe /c rmdir /s /q \"%SANDBOX%\"";

    //
    // open parent folder of sandbox
    //

    WCHAR *boxpath = GetBoxFilePath(boxname, 128);
    if (! boxpath) {
        // Error(SbieDll_FormatMessage0(MSG_3216), 0);
        return;
    }
    WCHAR *backslash = wcsrchr(boxpath, L'\\');
    if (backslash)
        *backslash = L'\0';
    wcscat(boxpath, L"\\*");

    //
    // get delete command
    //

    ULONG len = 2048;
    WCHAR *cmd = (WCHAR *)HeapAlloc(
                        GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, len);
    memzero(cmd, len);
    len -= 8;

    SbieApi_QueryConfAsIs(boxname, _DeleteCommand, 0, cmd, len);

    if (! cmd[0])
        wcscpy(cmd, _DefaultDeleteCommand);

    //
    // find all folders named __Delete_something
    //

    WCHAR *tmpl = (WCHAR *)HeapAlloc(
                        GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, 256);
    wsprintf(tmpl, L"__Delete_%s_", boxname);

    WCHAR *BoxFolder = (WCHAR *)
        HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, 2048);

    WIN32_FIND_DATA data;
    HANDLE hFind = FindFirstFile(boxpath, &data);
    if (hFind == INVALID_HANDLE_VALUE) {
        HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, boxpath);
        return;
    }

    while (1) {
        if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            wcslen(data.cFileName) == wcslen(tmpl) + 16 &&
            _wcsnicmp(data.cFileName, tmpl, wcslen(tmpl)) == 0)
        {
            wcscpy(BoxFolder, boxpath);
            wcscpy(BoxFolder + wcslen(BoxFolder) - 1, data.cFileName);

            //
            // prepare sandbox for deletion by taking care of read-only
            // files, junction points, and paths that are too long
            //

            WaitForFolder(BoxFolder, 10);

            ProcessFiles(BoxFolder);

            WaitForFolder(BoxFolder, 10);

            //
            // translate %SANDBOX% placeholders in DeleteCommand,
            // then invoke DeleteCommand
            //

            WCHAR cmd2[1536];
            TranslateCommand(cmd, cmd2, BoxFolder);
            LaunchProgram(cmd2, TRUE);
        }

        if (! FindNextFile(hFind, &data))
            break;
    }

    HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, boxpath);
}


//---------------------------------------------------------------------------
// TranslateCommand
//---------------------------------------------------------------------------


_FX void TranslateCommand(WCHAR *cmd1, WCHAR *cmd2, const WCHAR *BoxFolder)
{
    static const WCHAR *_sandbox = L"%SANDBOX%";
    static const WCHAR *_sandbox_bs = L"%SANDBOX\\\\%";

    WCHAR *ptr1 = cmd1, *ptr2 = cmd2;
    while (*ptr1) {
        if (*ptr1 == L'%') {

            if (_wcsnicmp(ptr1, _sandbox, wcslen(_sandbox)) == 0) {
                wcscpy(ptr2, BoxFolder);
                ptr2 += wcslen(ptr2);
                ptr1 += wcslen(_sandbox);
                continue;
            }

            if (_wcsnicmp(ptr1, _sandbox_bs, wcslen(_sandbox_bs)) == 0) {
                const WCHAR *boxptr = BoxFolder;
                while (*boxptr) {
                    if (*boxptr == L'\\') {
                        *ptr2 = *boxptr;
                        ++ptr2;
                    }
                    *ptr2 = *boxptr;
                    ++ptr2;
                    ++boxptr;
                }
                ptr1 += wcslen(_sandbox_bs);
                continue;
            }
        }

        *ptr2 = *ptr1;
        ++ptr2;
        ++ptr1;
    }
    *ptr2 = L'\0';
}


//---------------------------------------------------------------------------
// WaitForFolder
//---------------------------------------------------------------------------


NOINLINE void WaitForFolder(const WCHAR *folder, ULONG seconds)
{
    NTSTATUS status;
    UNICODE_STRING uni;
    OBJECT_ATTRIBUTES objattrs;
    IO_STATUS_BLOCK MyIoStatusBlock;
    HANDLE handle;
    ULONG retries;

    ULONG len = (wcslen(folder) + 8) * sizeof(WCHAR);
    WCHAR *path = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, len);
    if (! path)
        return;
    wsprintf(path, L"\\??\\%s", folder);

    RtlInitUnicodeString(&uni, path);
    InitializeObjectAttributes(
        &objattrs, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

    for (retries = 0; retries < seconds * 2; ++retries) {

        status = NtCreateFile(
            &handle, DELETE | SYNCHRONIZE, &objattrs,
            &MyIoStatusBlock, NULL, FILE_ATTRIBUTE_NORMAL,
            0, FILE_OPEN, FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
            NULL, 0);

        if (NT_SUCCESS(status)) {
            NtClose(handle);
            break;
        }

        Sleep(500);
    }

    HeapFree(GetProcessHeap(), 0, path);
}


//---------------------------------------------------------------------------
// RemoveFileAttributes
//---------------------------------------------------------------------------


bool RemoveFileAttributes(
    HANDLE heap, const WCHAR *parent, const WCHAR *child)
{
    static ULONG counter = 0;
    UNICODE_STRING uni;
    OBJECT_ATTRIBUTES objattrs;
    IO_STATUS_BLOCK MyIoStatusBlock;
    NTSTATUS status = STATUS_SUCCESS;
    HANDLE handle = NULL;

    // prepare path name

    ULONG len = (wcslen(parent) + wcslen(child) + 8) * sizeof(WCHAR);
    WCHAR *path = (WCHAR *)HeapAlloc(heap, 0, len);
    wsprintf(path, L"\\??\\%s\\%s", parent, child);

    // open file

    if (NT_SUCCESS(status)) {

        RtlInitUnicodeString(&uni, path);
        InitializeObjectAttributes(
            &objattrs, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

        status = NtCreateFile(
            &handle,
            FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
            &objattrs, &MyIoStatusBlock, NULL, 0, 0,
            FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
    }

    if (NT_SUCCESS(status)) {

        union {
            FILE_BASIC_INFORMATION info;
            WCHAR space[128];
        } u;

        status = NtQueryInformationFile(
            handle, &MyIoStatusBlock,
            &u.info, sizeof(u), FileBasicInformation);

        if (NT_SUCCESS(status)) {

            u.info.FileAttributes &= ~(FILE_ATTRIBUTE_READONLY |
                FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
            if (u.info.FileAttributes == 0 ||
                u.info.FileAttributes == FILE_ATTRIBUTE_DIRECTORY)
                    u.info.FileAttributes |= FILE_ATTRIBUTE_NORMAL;

            status = NtSetInformationFile(
                handle, &MyIoStatusBlock,
                &u.info, sizeof(u), FileBasicInformation);
        }
    }

    // close and free everything

    if (handle)
        NtClose(handle);

    HeapFree(heap, 0, path);

    SetLastError(RtlNtStatusToDosError(status));
    return (status == STATUS_SUCCESS);
}


//---------------------------------------------------------------------------
// RenameSingleFile
//---------------------------------------------------------------------------


bool RenameSingleFile(
    HANDLE heap,
    const WCHAR *parent, const WCHAR *child, const WCHAR *BoxPath)
{
    static ULONG counter = 0;
    UNICODE_STRING uni;
    OBJECT_ATTRIBUTES objattrs;
    IO_STATUS_BLOCK MyIoStatusBlock;
    NTSTATUS status = STATUS_SUCCESS;
    HANDLE src_handle = NULL, dst_handle = NULL;
    ULONG retries;

    // prepare source and destination path names

    ULONG len = (wcslen(parent) + wcslen(child) + 8) * sizeof(WCHAR);
    WCHAR *src_path = (WCHAR *)HeapAlloc(heap, 0, len);
    wsprintf(src_path, L"\\??\\%s\\%s", parent, child);

    len = (wcslen(BoxPath) + 64) * sizeof(WCHAR);
    FILETIME now;
    GetSystemTimeAsFileTime(&now);
    ++counter;
    WCHAR *dst_path = (WCHAR *)HeapAlloc(heap, 0, len);
    wsprintf(dst_path, L"\\??\\%s\\%08X-%08X-%08X",
                BoxPath, now.dwHighDateTime, now.dwLowDateTime, counter);
    WCHAR *dst_name = wcsrchr(dst_path, L'\\');
    *dst_name = L'\0';
    ++dst_name;

    // open files and directories

    if (NT_SUCCESS(status)) {

        RtlInitUnicodeString(&uni, src_path);
        InitializeObjectAttributes(
            &objattrs, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

        status = NtCreateFile(
            &src_handle, DELETE | SYNCHRONIZE,
            &objattrs, &MyIoStatusBlock, NULL, 0, 0,
            FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
    }

    if (NT_SUCCESS(status)) {

        RtlInitUnicodeString(&uni, dst_path);
        InitializeObjectAttributes(
            &objattrs, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

        status = NtCreateFile(
            &dst_handle, FILE_GENERIC_READ, &objattrs,
            &MyIoStatusBlock, NULL, FILE_ATTRIBUTE_NORMAL,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            FILE_OPEN, FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
            NULL, 0);
    }

    if (NT_SUCCESS(status)) {

        union {
            FILE_RENAME_INFORMATION info;
            WCHAR space[128];
        } u;

        u.info.ReplaceIfExists = FALSE;
        u.info.RootDirectory = dst_handle;
        u.info.FileNameLength = wcslen(dst_name) * sizeof(WCHAR);
        wcscpy(u.info.FileName, dst_name);

        for (retries = 0; retries < 20; ++retries) {

            status = NtSetInformationFile(
                src_handle, &MyIoStatusBlock,
                &u.info, sizeof(u), FileRenameInformation);

            if (status != STATUS_SHARING_VIOLATION)
                break;

            Sleep(300);
        }
    }

    // close and free everything

    if (dst_handle)
        NtClose(dst_handle);
    if (src_handle)
        NtClose(src_handle);

    HeapFree(heap, 0, dst_path);
    HeapFree(heap, 0, src_path);

    if (status == STATUS_OBJECT_NAME_NOT_FOUND ||
        status == STATUS_OBJECT_PATH_NOT_FOUND) {

        status = STATUS_SUCCESS;
    }

    SetLastError(RtlNtStatusToDosError(status));
    return (status == STATUS_SUCCESS);
}


//---------------------------------------------------------------------------
// ProcessFiles
//---------------------------------------------------------------------------


typedef struct _PATHELEM {
    struct _PATHELEM *next;
    WCHAR path[1];
} PATHELEM;


PATHELEM *AllocPathElem(HANDLE heap, const WCHAR *parent, const WCHAR *child)
{
    ULONG len = sizeof(PATHELEM)
              + (wcslen(parent) + wcslen(child) + 4) * sizeof(WCHAR);
    PATHELEM *elem = (PATHELEM *)HeapAlloc(heap, 0, len);
    elem->next = NULL;
    len = wcslen(parent);
    CopyMemory(elem->path, parent, len * sizeof(WCHAR));
    if (*child) {
        elem->path[len] = L'\\';
        wcscpy(elem->path + len + 1, child);
    } else
        elem->path[len] = L'\0';
    return elem;
}


void ProcessFiles(const WCHAR *BoxPath)
{
    static const UCHAR valid_chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
        "0123456789 ^&@{}[],$=!-#()%.+~_";
    bool anyRenames;
    HANDLE heap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 0, 0);

mainloop:

    anyRenames = false;

    //
    // process every file in the sandbox folder. remove file attributes
    // and junction points where we find them.  also rename any files
    // or directories in the sandbox that have a path which is too long
    // for normal Win32 processing
    //

    PATHELEM *elem_next = AllocPathElem(heap, BoxPath, L"");

    while (1) {

        PATHELEM *elem = elem_next;
        if (! elem)
            break;
        elem_next = elem->next;

        WCHAR *search_path = AllocPathElem(heap, elem->path, L"*")->path;
        WIN32_FIND_DATA data;
        HANDLE hFind = FindFirstFile(search_path, &data);
        if (hFind == INVALID_HANDLE_VALUE)
            continue;
        bool firstTime = true;

        while (1) {
            if (firstTime)
                firstTime = false;
            else {
                if (! FindNextFile(hFind, &data))
                    break;
            }

            const WCHAR *name = data.cFileName;
            if (wcscmp(name, L".") == 0 || wcscmp(name, L"..") == 0)
                continue;
            ULONG name_len = wcslen(name);

            if (data.dwFileAttributes & (FILE_ATTRIBUTE_READONLY |
                    FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) {
                RemoveFileAttributes(heap, elem->path, name);
            }

            //
            // check if the path is too long,
            // or if the file name matches a DOS device name,
            // or if the file name contains non-ASCII characters
            // or if the file name contains invalid ASCII characters
            // or if the file name ends with a dot or a space
            //

            bool needRename = ((wcslen(elem->path) + name_len) > 220);

            if ((! needRename) && (name_len <= 8)) {
                if(SbieDll_IsReservedFileName(name))
                    needRename = true;
            }

            if (! needRename) {
                for (const WCHAR *nameptr = name; *nameptr; ++nameptr) {
                    const UCHAR *charptr;
                    if (*nameptr >= 0x80) {     // non-ASCII
                        needRename = TRUE;
                        break;
                    }
                    for (charptr = valid_chars; *charptr; ++charptr) {
                        if ((UCHAR)*nameptr == *charptr)
                            break;
                    }
                    if (! *charptr) {           // invalid ASCII character
                        needRename = TRUE;
                        break;
                    }
                }
            }

            if (! needRename) {
                if (name_len > 1 && (name[name_len - 1] == L'.' ||
                                     name[name_len - 1] == L' ')) {
                    needRename = TRUE;          // file ends with a dot/space
                }
            }

            //
            // if necessary, move the file to a simple name at sandbox root
            //

            if (needRename) {

                bool ok = RenameSingleFile(heap, elem->path, name, BoxPath);
                if (! ok) {
                    Error(SbieDll_FormatMessage0(MSG_3220), 0);
                    return;
                }
                anyRenames = true;

            //
            // otherwise delete the directory if it is a reparse point
            //

            } else if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

                bool removed = false;
                if (data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
                    ULONG len = wcslen(elem->path) + wcslen(name) + 4;
                    WCHAR *fullpath =
                            (WCHAR *)HeapAlloc(heap, 0, len * sizeof(WCHAR));
                    if (fullpath) {
                        wsprintf(fullpath, L"%s\\%s", elem->path, name);
                        if (RemoveDirectory(fullpath))
                            removed = true;
                        HeapFree(heap, 0, fullpath);
                    }
                }

                if (! removed) {
                    PATHELEM *elem_new =
                                    AllocPathElem(heap, elem->path, name);
                    elem_new->next = elem_next;
                    elem_next = elem_new;
                }
            }
        }

        FindClose(hFind);
    }

    //
    // in case we detected a file path that was too long for normal Win32
    // processing, and renamed it to a shorter name, we must now repeat
    // the process, because that file may have been a directory which
    // could itself go deep enough to cause a long path again
    //

    if (anyRenames)
        goto mainloop;

    RemoveFileAttributes(heap, BoxPath, L"");

    HeapDestroy(heap);
}


//---------------------------------------------------------------------------
// GetBoxPath
//---------------------------------------------------------------------------


ALIGNED WCHAR *GetBoxFilePath(const WCHAR *boxname, ULONG extra)
{
    LONG status;
    ULONG len;
    WCHAR *path;

    len = 0;

    status = SbieApi_QueryBoxPath(
        boxname, NULL, NULL, NULL, &len, NULL, NULL);
    if (status != 0)
        return NULL;

    path = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, len + extra);
    if (path) {

        status = SbieApi_QueryBoxPath(
            boxname, path, NULL, NULL, &len, NULL, NULL);

        if (status == 0) {
            BOOLEAN ok = SbieDll_TranslateNtToDosPath(path);
            if (! ok)
                status = 1;
        }

        if (status != 0) {
            HeapFree(GetProcessHeap(), 0, path);
            path = NULL;
        }
    }

    return path;
}
