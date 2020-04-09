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
// File Server -- using PipeServer
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "fileserver.h"
#include "FileWire.h"
#include "misc.h"
#include "common/my_version.h"
#include "common/win32_ntddk.h"
#include "common/defines.h"
#include "core/dll/sbiedll.h"
#include "msgids.h"
#include <stdio.h>
#include <stdlib.h>


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define MyAlloc_WCHAR(n) (WCHAR *)MyAlloc((n) * sizeof(WCHAR))


//---------------------------------------------------------------------------
// Constants
//---------------------------------------------------------------------------


const ULONG DIR_INFO_LENGTH = 10240;


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


FileServer::FileServer(PipeServer *pipeServer)
{
    m_heap = HeapCreate(0, 0, 0);
    if (! m_heap)
        m_heap = GetProcessHeap();

    m_windows = NULL;
    m_winsxs = NULL;
    m_PublicSd = NULL;

    //
    // enable SeRestorePrivilege so SetShortName() and LoadKey()
    // can do their job
    //

    WCHAR priv_space[64];
    TOKEN_PRIVILEGES *privs = (TOKEN_PRIVILEGES *)priv_space;

    BOOL b = LookupPrivilegeValue(
                L"", SE_RESTORE_NAME, &privs->Privileges[0].Luid);
    if (b) {

        privs->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        privs->PrivilegeCount = 1;

        HANDLE hToken;
        b = OpenProcessToken(
                GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken);
        if (b) {

            b = AdjustTokenPrivileges(hToken, FALSE, privs, 0, NULL, NULL);
            CloseHandle(hToken);
        }
    }

    if (! b)
        LogEvent(MSG_9234, 0x9208, GetLastError());

    //
    // convert x:\Windows\WinSxS to sandboxed form \drive\x\windows\winsxs\ .
    //

    m_windows = MyAlloc_WCHAR(MAX_PATH + 8);

    if (! m_windows) {
        LogEvent(MSG_9234, 0x9209, STATUS_INSUFFICIENT_RESOURCES);
        return;
    }

    GetSystemWindowsDirectory(m_windows, MAX_PATH);
    if (! m_windows[0])
        wcscpy(m_windows, L"C:\\WINDOWS");
    else if (m_windows[wcslen(m_windows) - 1] == L'\\')
             m_windows[wcslen(m_windows) - 1] = L'\0';

    m_winsxs = MyAlloc_WCHAR(wcslen(m_windows) + 64);
    if (! m_winsxs) {
        LogEvent(MSG_9234, 0x9210, STATUS_INSUFFICIENT_RESOURCES);
        return;
    }
    wsprintf(m_winsxs,
             L"\\drive\\%c%s\\winsxs\\", m_windows[0], &m_windows[2]);

    //
    // prepare Security Descriptor for creating files in WinSxS
    //

    m_PublicSd = MyAlloc(64);
    if (! m_PublicSd) {
        LogEvent(MSG_9234, 0x9211, STATUS_INSUFFICIENT_RESOURCES);
        return;
    }
    RtlCreateSecurityDescriptor(m_PublicSd, SECURITY_DESCRIPTOR_REVISION);
    RtlSetDaclSecurityDescriptor(m_PublicSd, TRUE, NULL, FALSE);

    //
    // subscribe as a target of the pipe server
    //

    pipeServer->Register(MSGID_FILE, this, Handler);
}


//---------------------------------------------------------------------------
// Handler
//---------------------------------------------------------------------------


MSG_HEADER *FileServer::Handler(void *_this, MSG_HEADER *msg)
{
    FileServer *pThis = (FileServer *)_this;

    HANDLE idProcess = (HANDLE)(ULONG_PTR)PipeServer::GetCallerProcessId();

    if (0 != SbieApi_QueryProcess(idProcess, NULL, NULL, NULL, NULL))
        return SHORT_REPLY(STATUS_ACCESS_DENIED);

    //
    // requests where we don't impersonate the caller
    //

    if (msg->msgid == MSGID_FILE_SET_ATTRIBUTES)
        return pThis->SetAttributes(msg, idProcess);

    if (msg->msgid == MSGID_FILE_SET_SHORT_NAME)
        return pThis->SetShortName(msg, idProcess);

    if (msg->msgid == MSGID_FILE_LOAD_KEY)
        return pThis->LoadKey(msg, idProcess);

    if (msg->msgid == MSGID_FILE_GET_ALL_HANDLES)
        return pThis->GetAllHandles(idProcess);

    //
    // requests where we impersonate the caller
    //

    if (PipeServer::ImpersonateCaller(&msg) != 0)
        return msg;

    if (msg->msgid == MSGID_FILE_SET_REPARSE_POINT)
        return pThis->SetReparsePoint(msg, idProcess);

    if (msg->msgid == MSGID_FILE_OPEN_WOW64_KEY)
        return pThis->OpenWow64Key(msg, idProcess);

    if (msg->msgid == MSGID_FILE_CHECK_KEY_EXISTS)
        return pThis->CheckKeyExists(msg, idProcess);

    return NULL;
}


//---------------------------------------------------------------------------
// MyAlloc
//---------------------------------------------------------------------------


NOINLINE void *FileServer::MyAlloc(ULONG len)
{
    ULONG len2 = len + sizeof(ULONG_PTR) * 2;
    char *buf = (char *)HeapAlloc(m_heap, HEAP_ZERO_MEMORY, len2);
    if (buf) {
        *(ULONG_PTR *)buf = len;
        buf += sizeof(ULONG_PTR);
        *(ULONG_PTR *)(buf + len) = tzuk;
    }
    return buf;
}


//---------------------------------------------------------------------------
// MyFree
//---------------------------------------------------------------------------


NOINLINE void FileServer::MyFree(void *ptr)
{
    ULONG_PTR *p_len = (ULONG_PTR *)((char *)ptr - sizeof(ULONG_PTR));
    ULONG_PTR *p_tzuk = (ULONG_PTR *)((char *)ptr + *p_len);
    if (*p_tzuk != tzuk)
        SbieApi_Log(2316, NULL);
    *p_tzuk = 0;
    HeapFree(m_heap, 0, p_len);
}


//---------------------------------------------------------------------------
// SetAttributes
//---------------------------------------------------------------------------


MSG_HEADER *FileServer::SetAttributes(MSG_HEADER *msg, HANDLE idProcess)
{
    //
    // validate request structure
    //

    FILE_SET_ATTRIBUTES_REQ *req = (FILE_SET_ATTRIBUTES_REQ *)msg;
    if (req->h.length < sizeof(FILE_SET_ATTRIBUTES_REQ))
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);
    if (req->path_len > PIPE_MAX_DATA_LEN)
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);
    ULONG offset = FIELD_OFFSET(FILE_SET_ATTRIBUTES_REQ, path);
    if (offset + req->path_len > req->h.length)
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);

    //
    // open the caller-specified sandboxed file
    //

    HANDLE handle;
    NTSTATUS status = OpenBoxFile(
        idProcess, req->path,
        FILE_WRITE_ATTRIBUTES | SYNCHRONIZE, FILE_SYNCHRONOUS_IO_NONALERT,
        &handle);

    if (NT_SUCCESS(status)) {

        //
        // set attributes
        //

        IO_STATUS_BLOCK IoStatusBlock;
        status = NtSetInformationFile(
            handle, &IoStatusBlock,
            &req->info, sizeof(req->info), FileBasicInformation);

        NtClose(handle);
    }

    return SHORT_REPLY(status);
}


//---------------------------------------------------------------------------
// SetShortName
//---------------------------------------------------------------------------


MSG_HEADER *FileServer::SetShortName(MSG_HEADER *msg, HANDLE idProcess)
{
    //
    // validate request structure
    //

    FILE_SET_SHORT_NAME_REQ *req = (FILE_SET_SHORT_NAME_REQ *)msg;
    if (req->h.length < sizeof(FILE_SET_SHORT_NAME_REQ))
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);
    if (req->info.FileNameLength > sizeof(req->info.FileName))
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);
    if (req->path_len > PIPE_MAX_DATA_LEN)
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);
    ULONG offset = FIELD_OFFSET(FILE_SET_SHORT_NAME_REQ, path);
    if (offset + req->path_len > req->h.length)
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);

    //
    // open the caller-specified sandboxed file
    //

    HANDLE handle;
    NTSTATUS status = OpenBoxFile(
        idProcess, req->path,
        FILE_GENERIC_WRITE | DELETE,
        FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT,
        &handle);

    if (NT_SUCCESS(status)) {

        //
        // set short name
        //

        IO_STATUS_BLOCK IoStatusBlock;
        status = NtSetInformationFile(
            handle, &IoStatusBlock,
            &req->info, sizeof(req->info), FileShortNameInformation);

        NtClose(handle);
    }

    return SHORT_REPLY(status);
}


//---------------------------------------------------------------------------
// OpenBoxFile
//---------------------------------------------------------------------------


NTSTATUS FileServer::OpenBoxFile(
    HANDLE idProcess, WCHAR *request_path,
    ACCESS_MASK desired_access, ULONG create_options,
    HANDLE *out_handle)
{
    NTSTATUS status = CheckBoxFilePath(idProcess, request_path, L"\\");
    if (! NT_SUCCESS(status))
        SHORT_REPLY(status);

    UNICODE_STRING objname;
    RtlInitUnicodeString(&objname, request_path);

    OBJECT_ATTRIBUTES objattrs;
    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    IO_STATUS_BLOCK IoStatusBlock;
    status = NtCreateFile(
        out_handle, desired_access, &objattrs, &IoStatusBlock, NULL, 0,
        FILE_SHARE_VALID_FLAGS, FILE_OPEN, create_options, NULL, 0);

    return status;
}


//---------------------------------------------------------------------------
// LoadKey
//---------------------------------------------------------------------------


MSG_HEADER *FileServer::LoadKey(MSG_HEADER *msg, HANDLE idProcess)
{
    //
    // validate request structure
    //

    FILE_LOAD_KEY_REQ *req = (FILE_LOAD_KEY_REQ *)msg;
    if (req->h.length < sizeof(FILE_LOAD_KEY_REQ))
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);

    //
    // validate request from sandboxed process
    //

    NTSTATUS status = SbieApi_QueryProcess(
        idProcess, NULL, NULL, NULL, NULL);
    if (! NT_SUCCESS(status))
        return SHORT_REPLY(STATUS_ACCESS_DENIED);;

    //
    // this request is needed for Windows 7 TrustedInstaller trying to:
    //
    // mount the file %SystemRoot%\config\COMPONENTS
    // into the registry key HKEY_LOCAL_MACHINE\COMPONENTS
    //
    // mount the file %SystemRoot%\SMI\Store\Machine\SCHEMA.DAT
    // into the registry key HKEY_LOCAL_MACHINE\SCHEMA
    //

    BOOLEAN ok = FALSE;
    req->KeyPath[127] = L'\0';

    if (_wcsicmp(req->KeyPath, L"\\REGISTRY\\MACHINE\\COMPONENTS") == 0)
        ok = TRUE;
    else if (_wcsicmp(req->KeyPath, L"\\REGISTRY\\MACHINE\\SCHEMA") == 0)
        ok = TRUE;
    if (! ok)
        return SHORT_REPLY(STATUS_ACCESS_DENIED);

    WCHAR *MyFilePath = MyAlloc_WCHAR(wcslen(m_windows) + 128);
    if (! MyFilePath)
        return SHORT_REPLY(STATUS_INSUFFICIENT_RESOURCES);
    wcscpy(MyFilePath, L"\\??\\");
    WCHAR *MyFilePath1 = MyFilePath + 4;
    wcscpy(MyFilePath1, m_windows);
    wcscat(MyFilePath1, L"\\System32\\");
    WCHAR *MyFilePath2 = MyFilePath + wcslen(MyFilePath);

    ok = FALSE;
    req->FilePath[127] = L'\0';

    wcscpy(MyFilePath2, L"config\\COMPONENTS");
    if (_wcsicmp(req->FilePath, MyFilePath1) == 0)
        ok = TRUE;
    else {
        wcscpy(MyFilePath2, L"SMI\\Store\\Machine\\SCHEMA.DAT");
        if (_wcsicmp(req->FilePath, MyFilePath1) == 0)
            ok = TRUE;
    }

    if (! ok) {
        MyFree(MyFilePath);
        return SHORT_REPLY(STATUS_ACCESS_DENIED);
    }

    //
    // mount registry hive
    //

    UNICODE_STRING KeyPath_uni;
    UNICODE_STRING FilePath_uni;
    OBJECT_ATTRIBUTES source;
    OBJECT_ATTRIBUTES target;

    RtlInitUnicodeString(&KeyPath_uni, MyFilePath);
    InitializeObjectAttributes(&source,
        &KeyPath_uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

    RtlInitUnicodeString(&FilePath_uni, req->KeyPath);
    InitializeObjectAttributes(&target,
        &FilePath_uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = NtLoadKey(&target, &source);

    MyFree(MyFilePath);

    return SHORT_REPLY(status);
}


//---------------------------------------------------------------------------
// GetAllHandles
//---------------------------------------------------------------------------


MSG_HEADER *FileServer::GetAllHandles(HANDLE idProcess)
{
    NTSTATUS status;
    ULONG info_len = 64, len, i;

    struct MY_SYSTEM_HANDLE_INFORMATION {
        ULONG Count;
        struct HANDLE_INFO {
            ULONG       ProcessId;
            UCHAR       ObjectTypeNumber;
            UCHAR       Flags;
            USHORT      Handle;
            PVOID       Object;
            ACCESS_MASK GrantedAccess;
        } HandleInfo[1];
    };
    struct MY_SYSTEM_HANDLE_INFORMATION *info = NULL;

    //
    // get list of open handles in the system
    //

    for (i = 0; i < 5; ++i) {

        info = (struct MY_SYSTEM_HANDLE_INFORMATION *)
                                    HeapAlloc(m_heap, 0, info_len);
        if (! info)
            status = STATUS_INSUFFICIENT_RESOURCES;
        else {

            status = NtQuerySystemInformation(
                SystemHandleInformation, info, info_len, &len);
        }

        if (NT_SUCCESS(status))
            break;

        HeapFree(m_heap, 0, info);
        info_len = len + 64;

        if (status == STATUS_BUFFER_OVERFLOW ||
            status == STATUS_INFO_LENGTH_MISMATCH ||
            status == STATUS_BUFFER_TOO_SMALL) {

            continue;
        }

        break;
    }

    if (NT_SUCCESS(status)) {

        ULONG rpl_len = sizeof(FILE_GET_ALL_HANDLES_RPL)
                      + info->Count * sizeof(ULONG64);

        FILE_GET_ALL_HANDLES_RPL *rpl =
                    (FILE_GET_ALL_HANDLES_RPL *)LONG_REPLY(rpl_len);

        if (! rpl)
            status = STATUS_INSUFFICIENT_RESOURCES;
        else {

            for (i = 0; i < info->Count; ++i) {

                if (info->HandleInfo[i].ProcessId == (ULONG_PTR)idProcess) {

                    ULONG handle =
                        (ULONG)(ULONG_PTR)info->HandleInfo[i].Handle;

                    ULONG objtype =
                        (ULONG)info->HandleInfo[i].ObjectTypeNumber;

                    rpl->handles[rpl->num_handles] =
                        (handle & 0x00FFFFFFU) | (objtype << 24);

                    ++rpl->num_handles;
                }
            }

            HeapFree(m_heap, 0, info);

            return (MSG_HEADER *)rpl;
        }
    }

    if (info)
        HeapFree(m_heap, 0, info);

    return SHORT_REPLY(status);
}


//---------------------------------------------------------------------------
// SetReparsePoint
//---------------------------------------------------------------------------


MSG_HEADER *FileServer::SetReparsePoint(MSG_HEADER *msg, HANDLE idProcess)
{
    NTSTATUS status;

    //
    // validate request structure
    //

    FILE_SET_REPARSE_POINT_REQ *req = (FILE_SET_REPARSE_POINT_REQ *)msg;
    if (req->h.length < sizeof(FILE_SET_REPARSE_POINT_REQ))
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);
    if (req->src_path_len > PIPE_MAX_DATA_LEN)
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);
    if (req->dst_path_len > PIPE_MAX_DATA_LEN)
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);
    ULONG offset = FIELD_OFFSET(FILE_SET_REPARSE_POINT_REQ, src_path);
    if (offset + req->src_path_len > req->h.length)
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);
    if (req->dst_path_ofs > req->h.length)
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);
    if (req->dst_path_ofs + req->dst_path_len > req->h.length)
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);

    //
    // check that both paths are inside the sandbox, and that the
    // destination path is inside the sandboxed WinSxS directory
    //

    WCHAR *src_path = req->src_path;
    WCHAR *dst_path = (WCHAR *)((UCHAR *)req + req->dst_path_ofs);

    status = CheckBoxFilePath(idProcess, src_path, L"\\");
    if (! NT_SUCCESS(status))
        return SHORT_REPLY(STATUS_UNSUCCESSFUL);
    status = CheckBoxFilePath(idProcess, dst_path, m_winsxs);
    if (! NT_SUCCESS(status))
        return SHORT_REPLY(STATUS_UNSUCCESSFUL);

    //
    // prepare paths for source and destination files
    //

    WCHAR *src_file = MyAlloc_WCHAR(wcslen(src_path) + 512);
    if (! src_file)
        return SHORT_REPLY(STATUS_INSUFFICIENT_RESOURCES);

    WCHAR *dst_file = MyAlloc_WCHAR(wcslen(dst_path) + 512);
    if (! dst_file) {
        MyFree(src_file);
        return SHORT_REPLY(STATUS_INSUFFICIENT_RESOURCES);
    }

    //
    // emulate the creation of a reparse point from source to
    // destination, by copying any files in the destination
    // directory to the source directory
    //

    HANDLE handle;
    FILE_DIRECTORY_INFORMATION *info, *info_ptr;

    status = MyFindFirstFile(dst_path, &handle, &info, &info_ptr);

    while (NT_SUCCESS(status)) {

        if (! (info_ptr->FileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

            wcscpy(src_file, src_path);
            wcscat(src_file, L"\\");
            wcscat(src_file, info_ptr->FileName);

            wcscpy(dst_file, dst_path);
            wcscat(dst_file, L"\\");
            wcscat(dst_file, info_ptr->FileName);

            CopySingleFile(dst_file, src_file, idProcess);
        }

        status = MyFindNextFile(handle, info, &info_ptr);
    }

    if (status == STATUS_NO_MORE_FILES)
        status = STATUS_SUCCESS;

    //
    // finish
    //

    if (handle)
        NtClose(handle);
    if (info)
        MyFree(info);
    MyFree(src_file);
    MyFree(dst_file);

    return SHORT_REPLY(status);
}


//---------------------------------------------------------------------------
// CopySingleFile
//---------------------------------------------------------------------------


bool FileServer::CopySingleFile(
    const WCHAR *src_path, const WCHAR *dst_path, HANDLE idProcess)
{
    NTSTATUS status;
    UNICODE_STRING objname;
    OBJECT_ATTRIBUTES objattrs;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE src_handle = NULL;
    HANDLE dst_handle = NULL;
    FILE_NETWORK_OPEN_INFORMATION open_info;
    void *buffer = NULL;

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, m_PublicSd);

    do {

        //
        // open source file
        //

        RtlInitUnicodeString(&objname, src_path);

        status = NtCreateFile(
            &src_handle, FILE_GENERIC_READ, &objattrs, &IoStatusBlock,
            NULL, 0, FILE_SHARE_VALID_FLAGS, FILE_OPEN,
            FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE,
            NULL, 0);

        if (! NT_SUCCESS(status))
            break;

        //
        // create parent directory for destination file
        //

        ULONG session_id = PipeServer::GetCallerSessionId();

        WCHAR *LastBackslashPtr = (WCHAR*)wcsrchr(dst_path, L'\\');
        *LastBackslashPtr = L'\0';

        WCHAR *AssemblyNamePtr = (WCHAR*)wcsrchr(dst_path, L'\\');
        if (AssemblyNamePtr)
            ++AssemblyNamePtr;
        else
            AssemblyNamePtr = L"?";

        RtlInitUnicodeString(&objname, dst_path);

        status = NtCreateFile(
            &dst_handle, FILE_GENERIC_WRITE, &objattrs, &IoStatusBlock,
            NULL, 0, FILE_SHARE_VALID_FLAGS, FILE_OPEN,
            FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE,
            NULL, 0);

        if (status == STATUS_SHARING_VIOLATION) {
            // typically occurs when called from SetReparsePoint
            status = STATUS_SUCCESS;
        }

        if (status == STATUS_OBJECT_NAME_NOT_FOUND ||
            status == STATUS_OBJECT_PATH_NOT_FOUND) {

            // SbieApi_LogEx(session_id, 2107, AssemblyNamePtr);

            status = NtCreateFile(
                &dst_handle, FILE_GENERIC_WRITE, &objattrs, &IoStatusBlock,
                NULL, 0, FILE_SHARE_VALID_FLAGS, FILE_OPEN_IF,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE,
                NULL, 0);
        }

        if (! NT_SUCCESS(status)) {

            SbieApi_LogEx(
                session_id, 2216, L"[%08X] %S", status, AssemblyNamePtr);
        }

        *LastBackslashPtr = L'\\';

        if (! NT_SUCCESS(status))
            break;

        NtClose(dst_handle);
        dst_handle = NULL;

        //
        // create destination file
        //

        RtlInitUnicodeString(&objname, dst_path);

        status = NtCreateFile(
            &dst_handle, FILE_GENERIC_WRITE, &objattrs, &IoStatusBlock,
            NULL, 0, FILE_SHARE_VALID_FLAGS, FILE_OVERWRITE_IF,
            FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE,
            NULL, 0);

        //
        // copy source to destination
        //

        if (NT_SUCCESS(status)) {

            status = NtQueryInformationFile(
                src_handle, &IoStatusBlock, &open_info,
                sizeof(FILE_NETWORK_OPEN_INFORMATION),
                FileNetworkOpenInformation);
        }

        if (NT_SUCCESS(status)) {

            buffer = MyAlloc(4100);
            if (! buffer)
                status = STATUS_INSUFFICIENT_RESOURCES;
        }

        while (NT_SUCCESS(status) && open_info.EndOfFile.QuadPart) {

            ULONG buffer_size = 4096;
            if (open_info.EndOfFile.QuadPart < buffer_size)
                buffer_size = open_info.EndOfFile.LowPart;

            status = NtReadFile(
                src_handle, NULL, NULL, NULL, &IoStatusBlock,
                buffer, buffer_size, NULL, NULL);

            if (NT_SUCCESS(status)) {

                buffer_size = (ULONG)IoStatusBlock.Information;
                open_info.EndOfFile.QuadPart -= buffer_size;

                status = NtWriteFile(
                    dst_handle, NULL, NULL, NULL, &IoStatusBlock,
                    buffer, buffer_size, NULL, NULL);
            }
        }

        //
        // report any errors
        //

        if (! NT_SUCCESS(status)) {

            SbieApi_LogEx(
                session_id, 2216, L"[%08X] %S", status, AssemblyNamePtr);
        }

    } while (0);

    if (buffer)
        MyFree(buffer);
    if (dst_handle)
        NtClose(dst_handle);
    if (src_handle)
        NtClose(src_handle);

    return (NT_SUCCESS(status) ? true : false);
}


//---------------------------------------------------------------------------
// MyFindFirstFile
//---------------------------------------------------------------------------


NTSTATUS FileServer::MyFindFirstFile(
    const WCHAR *path, HANDLE *handle,
    FILE_DIRECTORY_INFORMATION **info,
    FILE_DIRECTORY_INFORMATION **info_ptr)
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    IO_STATUS_BLOCK IoStatusBlock;

    *handle = NULL;
    *info = NULL;
    *info_ptr = NULL;

    //
    // open directory
    //

    RtlInitUnicodeString(&objname, path);

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = NtCreateFile(
        handle, FILE_GENERIC_READ, &objattrs, &IoStatusBlock,
        NULL, 0, FILE_SHARE_VALID_FLAGS, FILE_OPEN,
        FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE,
        NULL, 0);

    if (! NT_SUCCESS(status))
        return status;

    //
    // allocate work buffer
    //

    FILE_DIRECTORY_INFORMATION *entry =
        (FILE_DIRECTORY_INFORMATION *)MyAlloc(DIR_INFO_LENGTH);
    if (! entry)
        return STATUS_INSUFFICIENT_RESOURCES;
    entry->NextEntryOffset = 0;
    *info = entry;
    *info_ptr = entry;

    return MyFindNextFile(*handle, *info, info_ptr);
}


//---------------------------------------------------------------------------
// MyFindNextFile
//---------------------------------------------------------------------------


NTSTATUS FileServer::MyFindNextFile(
    HANDLE handle,
    FILE_DIRECTORY_INFORMATION *info,
    FILE_DIRECTORY_INFORMATION **info_ptr)
{
    //
    // on entry, info_ptr points at the entry we returned last time,
    // so advance the pointer, or read a new buffer if we can't advance
    //

    FILE_DIRECTORY_INFORMATION *entry =
        (FILE_DIRECTORY_INFORMATION *)(*info_ptr);

    if (entry->NextEntryOffset) {

        ULONG SavedNextEntryOffset = entry->FileIndex;

        entry = (FILE_DIRECTORY_INFORMATION *)
                            ((UCHAR *)entry + entry->NextEntryOffset);

        entry->NextEntryOffset = SavedNextEntryOffset;

    } else {

        NTSTATUS status;
        IO_STATUS_BLOCK IoStatusBlock;

        status = NtQueryDirectoryFile(
                    handle, NULL, NULL, NULL, &IoStatusBlock,
                    info, DIR_INFO_LENGTH, FileDirectoryInformation,
                    FALSE, NULL, FALSE);

        if (status != STATUS_SUCCESS && status != STATUS_BUFFER_OVERFLOW)
            return status;

        entry = info;
    }

    //
    // adjust and return the new current entry.  note that putting
    // a zero past FileName overwrites the NextEntryOffset field of
    // the entry after this one, so save it
    //

    if (entry->NextEntryOffset) {

        FILE_DIRECTORY_INFORMATION *next_entry =
            (FILE_DIRECTORY_INFORMATION *)
                ((UCHAR *)entry + entry->NextEntryOffset);

        entry->FileIndex = next_entry->NextEntryOffset;
    }

    entry->FileName[entry->FileNameLength / sizeof(WCHAR)] = L'\0';

    *info_ptr = entry;

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// OpenWow64Key
//---------------------------------------------------------------------------


MSG_HEADER *FileServer::OpenWow64Key(MSG_HEADER *msg, HANDLE idProcess)
{
#ifdef _WIN64

    //
    // validate request structure
    //

    FILE_OPEN_WOW64_KEY_REQ *req = (FILE_OPEN_WOW64_KEY_REQ *)msg;
    if (req->h.length < sizeof(FILE_OPEN_WOW64_KEY_REQ))
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);
    if (req->KeyPath_len > PIPE_MAX_DATA_LEN)
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);
    ULONG offset = FIELD_OFFSET(FILE_OPEN_WOW64_KEY_REQ, KeyPath);
    if (offset + req->KeyPath_len > req->h.length)
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);
    if (req->Wow64DesiredAccess != KEY_WOW64_32KEY)
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);

    //
    // convert registry key path from NT form to Win32 form
    //

    BOOLEAN ShouldCloseRootKey;
    HKEY hRootKey;
    const WCHAR *lpSubKey;

    if (req->KeyPath[10] == L'M' || req->KeyPath[10] == L'm') {
        // MACHINE
        ShouldCloseRootKey = FALSE;
        hRootKey = HKEY_LOCAL_MACHINE;
        lpSubKey = req->KeyPath + 10 + 7;
    } else if (req->KeyPath[10] == L'U' || req->KeyPath[10] == L'u') {
        // USER
        ShouldCloseRootKey = TRUE;
        hRootKey = HKEY_CURRENT_USER;
        lpSubKey = req->KeyPath + 10 + 4;
        if (*lpSubKey != L'\\')
            SHORT_REPLY(STATUS_INVALID_PARAMETER);
        do {
            if (*lpSubKey == L'_') {
                if (_wcsnicmp(lpSubKey + 1, L"classes", 7) == 0)
                    hRootKey = HKEY_CLASSES_ROOT;
            }
            ++lpSubKey;
        } while (*lpSubKey != L'\0' && *lpSubKey != L'\\');
    } else
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);
    if (*lpSubKey == L'\\')
        ++lpSubKey;

    //
    // open the current user or current class registry hive
    //

    BOOL ok = TRUE;

    if (hRootKey == HKEY_CURRENT_USER) {

        if (RegOpenCurrentUser(KEY_READ, &hRootKey) != 0)
            ok = FALSE;

    } else if (hRootKey == HKEY_CLASSES_ROOT) {

        HANDLE hToken;
        ok = OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken);
        if (ok) {
            if (RegOpenUserClassesRoot(hToken, 0, KEY_READ, &hRootKey) != 0)
                ok = FALSE;
        }
        CloseHandle(hToken);
    }

    if (! ok)
        return SHORT_REPLY(STATUS_OBJECT_NAME_NOT_FOUND);

    //
    // try to open the key, chopping off the last path component
    //

    HKEY hKey = NULL;
    WCHAR *BackslashPtr = NULL;
    LONG rc;

    while (1) {

        rc = RegOpenKeyEx(hRootKey, lpSubKey, 0,
                          KEY_READ | KEY_WOW64_32KEY, &hKey);
        if (rc == 0)
            break;
        if (rc != ERROR_FILE_NOT_FOUND && rc != ERROR_PATH_NOT_FOUND)
            break;

        WCHAR *NewPtr = (WCHAR*)wcsrchr(lpSubKey, L'\\');
        if (BackslashPtr)
            *BackslashPtr = L'\\';
        BackslashPtr = NewPtr;

        if (! BackslashPtr)
            break;
        *BackslashPtr = L'\0';
    }

    if (ShouldCloseRootKey)
        RegCloseKey(hRootKey);

    if (rc != 0)
        return SHORT_REPLY(STATUS_OBJECT_NAME_NOT_FOUND);

    //
    // query the full path to the key we opened
    //

    ULONG len = 256;
    KEY_NAME_INFORMATION *name = (KEY_NAME_INFORMATION *)MyAlloc(len);
    if (name) {
        rc = NtQueryKey(hKey, KeyNameInformation, name, len, &len);
        if (rc == STATUS_BUFFER_OVERFLOW) {
            MyFree(name);
            len += 64;
            name = (KEY_NAME_INFORMATION *)MyAlloc(len);
            if (name)
                rc = NtQueryKey(hKey, KeyNameInformation, name, len, &len);
        }
    }

    CloseHandle(hKey);

    if (rc != STATUS_SUCCESS) {
        if (name)
            MyFree(name);
        return SHORT_REPLY(rc);
    }

    //
    // prepare the reply
    //

    len = name->NameLength / sizeof(WCHAR);
    ULONG rpl_len = sizeof(FILE_OPEN_WOW64_KEY_RPL) +
                  (len + 2) * sizeof(WCHAR);
    if (BackslashPtr) {
        *BackslashPtr = L'\\';
        rpl_len += wcslen(BackslashPtr) * sizeof(WCHAR);
    }

    FILE_OPEN_WOW64_KEY_RPL *rpl =
        (FILE_OPEN_WOW64_KEY_RPL *)LONG_REPLY(rpl_len);
    if (rpl) {

        wmemcpy(rpl->KeyPath, name->Name, len);
        if (BackslashPtr)
            wcscpy(rpl->KeyPath + len, BackslashPtr);
        else
            rpl->KeyPath[len] = L'\0';
    }

    MyFree(name);

    return (MSG_HEADER *)rpl;

#else ! _WIN64

    return SHORT_REPLY(STATUS_NOT_SUPPORTED);

#endif _WIN64
}


//---------------------------------------------------------------------------
// CheckKeyExists
//---------------------------------------------------------------------------


MSG_HEADER *FileServer::CheckKeyExists(MSG_HEADER *msg, HANDLE idProcess)
{
    //
    // validate request structure
    //

    FILE_CHECK_KEY_EXISTS_REQ *req = (FILE_CHECK_KEY_EXISTS_REQ *)msg;
    if (req->h.length < sizeof(FILE_CHECK_KEY_EXISTS_REQ))
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);
    if (req->KeyPath_len > PIPE_MAX_DATA_LEN)
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);
    ULONG offset = FIELD_OFFSET(FILE_CHECK_KEY_EXISTS_REQ, KeyPath);
    if (offset + req->KeyPath_len > req->h.length)
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);

    //
    // open the caller-specified sandboxed file
    //

    NTSTATUS status = CheckBoxKeyPath(idProcess, req->KeyPath, L"\\");
    if (NT_SUCCESS(status)) {

        UNICODE_STRING objname;
        OBJECT_ATTRIBUTES objattrs;
        HANDLE handle;

        RtlInitUnicodeString(&objname, req->KeyPath);
        InitializeObjectAttributes(
            &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

        status = NtOpenKey(&handle, KEY_READ, &objattrs);

        if (NT_SUCCESS(status))
            NtClose(handle);
    }

    return SHORT_REPLY(status);
}


//---------------------------------------------------------------------------
// CheckBoxFilePath
//---------------------------------------------------------------------------


NTSTATUS FileServer::CheckBoxFilePath(
    HANDLE idProcess, WCHAR *request_path, const WCHAR *extra_path)
{
    //
    // get the box file path for the calling process
    //

    ULONG len = 0;
    NTSTATUS status = SbieApi_QueryProcessPath(
        idProcess, NULL, NULL, NULL, &len, NULL, NULL);
    if (! NT_SUCCESS(status))
        return status;

    WCHAR *sandbox_path = (WCHAR *)MyAlloc(
        len + 8 + wcslen(extra_path) * sizeof(WCHAR));
    if (! sandbox_path)
        return STATUS_INSUFFICIENT_RESOURCES;

    status = SbieApi_QueryProcessPath(
        idProcess, sandbox_path, NULL, NULL, &len, NULL, NULL);
    if (! NT_SUCCESS(status)) {
        MyFree(sandbox_path);
        return status;
    }

    wcscat(sandbox_path, extra_path);

    //
    // make sure the caller specified a path inside the sandbox
    //

    ULONG sandbox_path_len = wcslen(sandbox_path);
    ULONG request_path_len = wcslen(request_path);
    if (request_path_len <= sandbox_path_len ||
            _wcsnicmp(sandbox_path, request_path, sandbox_path_len) != 0) {
        MyFree(sandbox_path);
        return STATUS_ACCESS_DENIED;
    }

    //
    // all is well
    //

    MyFree(sandbox_path);
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// CheckBoxKeyPath
//---------------------------------------------------------------------------


NTSTATUS FileServer::CheckBoxKeyPath(
    HANDLE idProcess, WCHAR *request_path, const WCHAR *extra_path)
{
    //
    // get the box key path for the calling process
    //

    ULONG len = 0;
    NTSTATUS status = SbieApi_QueryProcessPath(
        idProcess, NULL, NULL, NULL, NULL, &len, NULL);
    if (! NT_SUCCESS(status))
        return status;

    WCHAR *sandbox_path = (WCHAR *)MyAlloc(
        len + 8 + wcslen(extra_path) * sizeof(WCHAR));
    if (! sandbox_path)
        return STATUS_INSUFFICIENT_RESOURCES;

    status = SbieApi_QueryProcessPath(
        idProcess, NULL, sandbox_path, NULL, NULL, &len, NULL);
    if (! NT_SUCCESS(status)) {
        MyFree(sandbox_path);
        return status;
    }

    wcscat(sandbox_path, extra_path);

    //
    // make sure the caller specified a path inside the sandbox
    //

    ULONG sandbox_path_len = wcslen(sandbox_path);
    ULONG request_path_len = wcslen(request_path);
    if (request_path_len <= sandbox_path_len ||
            _wcsnicmp(sandbox_path, request_path, sandbox_path_len) != 0) {
        MyFree(sandbox_path);
        return STATUS_ACCESS_DENIED;
    }

    //
    // all is well
    //

    MyFree(sandbox_path);
    return STATUS_SUCCESS;
}
