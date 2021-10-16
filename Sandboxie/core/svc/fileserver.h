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


#ifndef _MY_FILESERVER_H
#define _MY_FILESERVER_H


#include "PipeServer.h"


class FileServer
{

public:

    FileServer(PipeServer *pipeServer);

protected:

    static MSG_HEADER *Handler(void *_this, MSG_HEADER *msg);

    void *MyAlloc(ULONG len);
    void MyFree(void *ptr);

    MSG_HEADER *SetAttributes(MSG_HEADER *msg, HANDLE idProcess);

    MSG_HEADER *SetShortName(MSG_HEADER *msg, HANDLE idProcess);

    MSG_HEADER *LoadKey(MSG_HEADER *msg, HANDLE idProcess);

    MSG_HEADER *SetReparsePoint(MSG_HEADER *msg, HANDLE idProcess);

    MSG_HEADER *OpenWow64Key(MSG_HEADER *msg, HANDLE idProcess);

    MSG_HEADER *CheckKeyExists(MSG_HEADER *msg, HANDLE idProcess);

    MSG_HEADER *GetAllHandles(HANDLE idProcess);

    bool CopySingleFile(
        const WCHAR *src_path, const WCHAR *dst_path, HANDLE idProcess);

    NTSTATUS MyFindFirstFile(
        const WCHAR *path, HANDLE *handle,
        FILE_DIRECTORY_INFORMATION **info,
        FILE_DIRECTORY_INFORMATION **info_ptr);

    NTSTATUS MyFindNextFile(
        HANDLE handle,
        FILE_DIRECTORY_INFORMATION *info,
        FILE_DIRECTORY_INFORMATION **info_ptr);

    NTSTATUS OpenBoxFile(
        HANDLE idProcess, WCHAR *request_path,
        ACCESS_MASK desired_access, ULONG create_options,
        HANDLE *out_handle);

    NTSTATUS CheckBoxFilePath(
        HANDLE idProcess, WCHAR *request_path, const WCHAR *extra_path);

    NTSTATUS CheckBoxKeyPath(
        HANDLE idProcess, WCHAR *request_path, const WCHAR *extra_path);

protected:

    WCHAR *m_windows;
    WCHAR *m_winsxs;
    PSECURITY_DESCRIPTOR m_PublicSd;
    HANDLE m_heap;

};


#endif /* _MY_FILESERVER_H */
