/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020 David Xanatos, xanasoft.com
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
// Files in Box
//---------------------------------------------------------------------------

#include "stdafx.h"
#include "MyApp.h"
#include "BoxFile.h"
#include "Box.h"
#include "common/win32_ntddk.h"


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


struct FileEntry {

    FileEntry *parent;
    CPtrList *children;
    BOOL analyzed;
    BOOL expanded;
    ULONG64 size;
    FILETIME time;
    CString *path;
    WCHAR name[1];
};


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


int CBoxFile::m_RefreshCounter = 0;


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CBoxFile::CBoxFile(const CString &name)
    : m_name(name)
{
    m_root      = NULL;
    m_curdir    = NULL;
    m_IncludeDeleted = FALSE;

    RebuildSkeletonTree();
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CBoxFile::~CBoxFile()
{
    if (m_root)
        DeleteFolder(m_root, TRUE);
}


//---------------------------------------------------------------------------
// GetPathNt
//---------------------------------------------------------------------------


const CString &CBoxFile::GetPathNt() const
{
    return m_FilePath;
}


//---------------------------------------------------------------------------
// GetPathDos
//---------------------------------------------------------------------------


const CString &CBoxFile::GetPathDos() const
{
    return m_FilePathDos;
}


//---------------------------------------------------------------------------
// RebuildSkeletonTree
//---------------------------------------------------------------------------


void CBoxFile::RebuildSkeletonTree()
{
    ++m_RefreshCounter;

    if (m_root)
        DeleteFolder(m_root, TRUE);

    ULONG buf_len = 4096 * sizeof(WCHAR);
    WCHAR *buf = malloc_WCHAR(buf_len);

    SbieApi_QueryBoxPath(m_name, buf, NULL, NULL, &buf_len, NULL, NULL);
    if (buf[0]) {

        m_FilePath = buf;
        m_FilePathDos = buf;
        TranslateNtToDosPath(m_FilePathDos);

        CreateSkeletonTree();
    }

    free(buf);
}


//---------------------------------------------------------------------------
// CreateSkeletonTree
//---------------------------------------------------------------------------


void CBoxFile::CreateSkeletonTree()
{
    ULONG len;
    FileEntry *folder;

    len = sizeof(FileEntry) + 2 * sizeof(WCHAR);
    folder = (FileEntry *)malloc(len);
    memzero(folder, len);
    folder->children = new CPtrList();

    m_root = folder;
    m_curdir = m_root;

    FileEntry *qr = (FileEntry *)AddFolder(CMyMsg(MSG_3520), m_root);
    qr->time.dwLowDateTime = 0xFFFF;

    FileEntry *all = (FileEntry *)AddFolder(CMyMsg(MSG_3521), m_root);
    all->path = new CString(m_FilePath);
}


//---------------------------------------------------------------------------
// CreateQuickRecoveryFolders
//---------------------------------------------------------------------------


void CBoxFile::CreateQuickRecoveryFolders()
{
    FileEntry *folder = (FileEntry *)m_curdir;
    if (folder->children->GetCount())
        return;

    //
    // collect RecoverFolder settings
    //

    CStringList qrFolders;

    ULONG buf_len = 4096 * sizeof(WCHAR);
    WCHAR *buf = malloc_WCHAR(buf_len);

    int index = 0;
    while (1) {

        SbieApi_QueryConf(
            m_name, CBox::_RecoverFolder, index,
            buf, buf_len - 16 * sizeof(WCHAR));
        if (! buf[0])
            break;
        ++index;

        while (1) {
            int len = wcslen(buf);
            if (len > 1 && buf[len - 1] == L'\\')
                buf[len - 1] = L'\0';
            else
                break;
        }

        qrFolders.AddTail(buf);

        if (GetAbsolutePathForRecoveryFolder(buf, buf_len)) {

            while (1) {
                int len = wcslen(buf);
                if (len > 1 && buf[len - 1] == L'\\')
                    buf[len - 1] = L'\0';
                else
                    break;
            }

            qrFolders.AddTail(buf);
        }
    }

    free(buf);

    //
    // process each Quick Recovery folder
    //

    int qrFolderCount = (int)qrFolders.GetCount();
    int qrIndex0, qrIndex1;

    for (qrIndex0 = 0; qrIndex0 < qrFolderCount; ++qrIndex0) {

        const CString &qrFolder0 =
            qrFolders.GetAt(qrFolders.FindIndex(qrIndex0));

        int qrBackslash0 = qrFolder0.ReverseFind(L'\\');
        if (qrBackslash0 == -1)
            qrBackslash0 = 0;
        else if (qrBackslash0 < qrFolder0.GetLength())
            ++qrBackslash0;
        CString qrFolder0Name = qrFolder0.Mid(qrBackslash0);

        CString DriveName = GetDriveNameForTruePath(qrFolder0);
        if (! DriveName.IsEmpty())
            qrFolder0Name = DriveName;

        bool qrPathDup = false;
        bool qrNameDup = false;
        for (qrIndex1 = 0; qrIndex1 < qrFolderCount; ++qrIndex1) {

            const CString &qrFolder1 =
                qrFolders.GetAt(qrFolders.FindIndex(qrIndex1));

            if (qrIndex1 > qrIndex0) {

                if (qrFolder1.CompareNoCase(qrFolder0) == 0)
                    qrPathDup = true;
            }

            if (qrIndex1 != qrIndex0) {

                int qrBackslash1 = qrFolder1.ReverseFind(L'\\');
                if (qrBackslash1 == -1)
                    qrBackslash1 = 0;
                else if (qrBackslash1 < qrFolder1.GetLength())
                    ++qrBackslash1;
                CString qrFolder1Name = qrFolder1.Mid(qrBackslash1);

                if (qrFolder1Name.CompareNoCase(qrFolder0Name) == 0)
                    qrNameDup = true;
            }
        }

        //
        //
        //

        if (qrPathDup)
            continue;

        CString ChildCopyPath = GetCopyPathForTruePath(qrFolder0);
        if (ChildCopyPath.IsEmpty())
            continue;

        if (qrNameDup) {

            CString qrFolder0Dos = qrFolder0;
            TranslateNtToDosPath(qrFolder0Dos);
            qrFolder0Name += L" <" + qrFolder0Dos + L">";
        }

        FileEntry *child = (FileEntry *)AddFolder(qrFolder0Name, folder);
        child->path = new CString(ChildCopyPath);

        ReadFolder(child);
    }
}


//---------------------------------------------------------------------------
// GetAbsolutePathForRecoveryFolder
//---------------------------------------------------------------------------


bool CBoxFile::GetAbsolutePathForRecoveryFolder(WCHAR *buf, ULONG buf_len)
{
    CString SavePath(buf);
    bool converted = false;

    NTSTATUS status;
    UNICODE_STRING uni;
    OBJECT_ATTRIBUTES objattrs;
    IO_STATUS_BLOCK MyIoStatusBlock;
    HANDLE hFile;

    RtlInitUnicodeString(&uni, buf);

    InitializeObjectAttributes(
        &objattrs, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = NtCreateFile(
        &hFile, FILE_READ_ATTRIBUTES | SYNCHRONIZE, &objattrs,
        &MyIoStatusBlock, NULL, 0,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN, FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
        NULL, 0);

    if (NT_SUCCESS(status)) {

        buf_len -= 4;
        status = SbieApi_GetFileName(hFile, buf, &buf_len, NULL);
        if (status == 0 && SavePath.CompareNoCase(buf) != 0)
            converted = true;

        CloseHandle(hFile);
    }

    return converted;
}


//---------------------------------------------------------------------------
// RebuildQuickRecoveryFolders
//---------------------------------------------------------------------------


void CBoxFile::RebuildQuickRecoveryFolders()
{
    if (m_root) {
        void *qrFolder = ((FileEntry *)m_root)->children->GetHead();
        if (qrFolder) {
            CString path = CString(L"\\") + ((FileEntry *)qrFolder)->name;
            DeleteFolder(qrFolder, FALSE);
            ++m_RefreshCounter;
            ChangeFolder(path, TRUE);
        }
    }
}


//---------------------------------------------------------------------------
// GetCopyPathForTruePath
//---------------------------------------------------------------------------


CString CBoxFile::GetCopyPathForTruePath(const WCHAR *TruePath) const
{
    ULONG TruePath_len = wcslen(TruePath);

	BOOLEAN SeparateUserFolders = SbieApi_QueryConfBool(m_name, L"SeparateUserFolders", TRUE);

    for (int index = SeparateUserFolders ? -4 : -1; index < 26; ++index) {

        const WCHAR *prefix = NULL;
        if (index == -4)
			prefix = SbieDll_GetUserPathEx(L'p');
        else if (index == -3)
            prefix = SbieDll_GetUserPathEx(L'a');
        else if (index == -2)
            prefix = SbieDll_GetUserPathEx(L'c');
        else if (index == -1)
			prefix = L"\\device\\mup"; 
        else
            prefix = SbieDll_GetDrivePath(index);
        if (! prefix)
            continue;

        ULONG prefix_len = wcslen(prefix);
        if (TruePath_len >= prefix_len &&
            (TruePath[prefix_len] == L'\\' || TruePath[prefix_len] == L'\0')
            && _wcsnicmp(TruePath, prefix, prefix_len) == 0) {

            CString CopyPath(m_FilePath);
            if (index == -4)
				CopyPath += L"\\user\\public";
            else if (index == -3)
                CopyPath += L"\\user\\all";
            else if (index == -2)
                CopyPath += L"\\user\\current";
            else if (index == -1)
				CopyPath += L"\\share"; 
            else {
                WCHAR letter = L'a' + index;
                CopyPath += L"\\drive\\";
                CopyPath += letter;
            }
            CopyPath += &TruePath[prefix_len];

            return CopyPath;
        }
    }

    return CString();
}


//---------------------------------------------------------------------------
// GetTruePathForCopyPath
//---------------------------------------------------------------------------


CString CBoxFile::GetTruePathForCopyPath(const WCHAR *CopyPath) const
{
    int CopyPath_len = wcslen(CopyPath);

    if (CopyPath_len >= m_FilePath.GetLength() &&
        _wcsnicmp(CopyPath, m_FilePath, m_FilePath.GetLength()) == 0 &&
        (CopyPath[m_FilePath.GetLength()] == L'\\' ||
         CopyPath[m_FilePath.GetLength()] == L'\0')) {
        CopyPath += m_FilePath.GetLength();
        CopyPath_len -= m_FilePath.GetLength();
    }

    if (CopyPath_len >= m_FilePathDos.GetLength() &&
        _wcsnicmp(CopyPath, m_FilePathDos, m_FilePathDos.GetLength()) == 0 &&
        (CopyPath[m_FilePathDos.GetLength()] == L'\\' ||
         CopyPath[m_FilePathDos.GetLength()] == L'\0')) {
        CopyPath += m_FilePathDos.GetLength();
        CopyPath_len -= m_FilePathDos.GetLength();
    }

    for (int index = -4; index < 26; ++index) {

        WCHAR drive_space[32];
        const WCHAR *prefix = NULL;
        if (index == -4)
            prefix = L"\\share";
        else if (index == -3)
            prefix = L"\\user\\all";
        else if (index == -2)
            prefix = L"\\user\\current";
        else if (index == -1)
            prefix = L"\\user\\public";
        else {
            wcscpy(drive_space, L"\\drive\\x");
            drive_space[wcslen(drive_space) - 1] = (L'a' + index);
            prefix = drive_space;
        }
        if (! prefix)
            continue;

        int prefix_len = wcslen(prefix);
        if (CopyPath_len >= prefix_len &&
            (CopyPath[prefix_len] == L'\\' || CopyPath[prefix_len] == L'\0')
            && _wcsnicmp(CopyPath, prefix, prefix_len) == 0) {

            if (index == -4)
                prefix = L"\\";
            else if (index == -3)
                prefix = SbieDll_GetUserPathEx(L'a');
            else if (index == -2)
                prefix = SbieDll_GetUserPathEx(L'c');
            else if (index == -1)
                prefix = SbieDll_GetUserPathEx(L'p');
            else
                prefix = SbieDll_GetDrivePath(index);
            CString TruePath =
                CString(prefix) + CString(&CopyPath[prefix_len]);
            return TruePath;
        }
    }

    return CString();
}


//---------------------------------------------------------------------------
// GetDriveNameForTruePath
//---------------------------------------------------------------------------


CString CBoxFile::GetDriveNameForTruePath(const WCHAR *TruePath) const
{
    ULONG TruePath_len = wcslen(TruePath);

    for (int index = 0; index < 26; ++index) {

        const WCHAR *prefix = SbieDll_GetDrivePath(index);
        if (! prefix)
            continue;

        ULONG prefix_len = wcslen(prefix);
        if (TruePath_len == prefix_len
                && _wcsnicmp(TruePath, prefix, prefix_len) == 0) {

            CString DriveName;
            DriveName.Format(L"%c:", (L'A' + index));
            return DriveName;
        }
    }

    return CString();
}


//---------------------------------------------------------------------------
// TranslateNtToDosPath
//---------------------------------------------------------------------------


void CBoxFile::TranslateNtToDosPath(CString &InOutPath) const
{
    WCHAR *path;
    path = malloc_WCHAR((InOutPath.GetLength() + 8) * sizeof(WCHAR));
    wcscpy(path, InOutPath);
    SbieDll_TranslateNtToDosPath(path);
    InOutPath = path;
    free(path);
    if (InOutPath.GetLength() == 2 && InOutPath.Mid(1, 1) == L':')
        InOutPath += L'\\';
}


//---------------------------------------------------------------------------
// DeleteFolder
//---------------------------------------------------------------------------


void CBoxFile::DeleteFolder(void *voidfolder, BOOL DeleteSelf)
{
    FileEntry *folder = (FileEntry *)voidfolder;

    if (folder->children) {
        while (! folder->children->IsEmpty()) {
            FileEntry *child = (FileEntry *)folder->children->RemoveHead();
            DeleteFolder(child, TRUE);
        }
    }

    if (DeleteSelf) {

        if (folder->children)
            delete folder->children;
        if (folder->path)
            delete folder->path;
        free(folder);

    } else
        folder->analyzed = FALSE;
}


//---------------------------------------------------------------------------
// AddFolder
//---------------------------------------------------------------------------


void *CBoxFile::AddFolder(const WCHAR *name, void *parent)
{
    ULONG len = sizeof(FileEntry) + (wcslen(name) + 1) * sizeof(WCHAR);
    FileEntry *folder = (FileEntry *)malloc(len);
    memzero(folder, len);
    folder->parent = (FileEntry *)parent;
    folder->children = new CPtrList();
    wcscpy(folder->name, name);
    ((FileEntry *)parent)->children->AddTail(folder);
    return folder;
}


//---------------------------------------------------------------------------
// ReadFolder
//---------------------------------------------------------------------------


BOOL CBoxFile::ReadFolder(void *voidfolder)
{
    FileEntry *folder = (FileEntry *)voidfolder;

    //
    //
    //

    NTSTATUS status;
    UNICODE_STRING uni;
    OBJECT_ATTRIBUTES objattrs;
    IO_STATUS_BLOCK MyIoStatusBlock;
    HANDLE hFile;

    CString &folder_path = *folder->path;
    uni.Buffer = (WCHAR *)(const WCHAR *)folder_path;
    uni.Length = folder_path.GetLength() * sizeof(WCHAR);
    uni.MaximumLength = uni.Length + sizeof(WCHAR);

    InitializeObjectAttributes(
        &objattrs, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = NtCreateFile(
        &hFile, FILE_GENERIC_READ, &objattrs, &MyIoStatusBlock, NULL,
        0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN, FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
        NULL, NULL);

    if (! NT_SUCCESS(status))
        return FALSE;

    //
    // compare timestamps in order to avoid unnecessary refresh
    //

    FILETIME time;
    BOOL ok = GetFileTime(hFile, NULL, NULL, &time);
    if (! ok) {
        CloseHandle(hFile);
        return FALSE;
    }

    if (time.dwLowDateTime == folder->time.dwLowDateTime &&
        time.dwHighDateTime == folder->time.dwHighDateTime) {

        CloseHandle(hFile);
        return TRUE;
    }

    //
    // refresh timestamp and delete existing items
    //

    DeleteFolder(folder, FALSE);

    folder->time.dwLowDateTime  = time.dwLowDateTime;
    folder->time.dwHighDateTime = time.dwHighDateTime;

    //
    //
    //

    const ULONG DIR_INFO_LENGTH = 10240;

    void *dir_info = malloc(DIR_INFO_LENGTH);
    status = STATUS_SUCCESS;

    while (status == STATUS_SUCCESS) {

        status = NtQueryDirectoryFile(
            hFile, NULL, NULL, NULL, &MyIoStatusBlock,
            dir_info, DIR_INFO_LENGTH, FileDirectoryInformation,
            FALSE, NULL, FALSE);

        if (status != STATUS_SUCCESS &&
            status != STATUS_BUFFER_OVERFLOW)
            break;

        FILE_DIRECTORY_INFORMATION *info =
            (FILE_DIRECTORY_INFORMATION *)dir_info;

        while (1) {

            if (m_IncludeDeleted || (
                    info->CreationTime.HighPart != 0x01B01234 &&
                    info->CreationTime.LowPart  != 0xDEAD44A0)) {

                CString name(
                    info->FileName, info->FileNameLength / sizeof(WCHAR));

                if (info->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

                    if (name.Compare(L".") != 0 &&
                        name.Compare(L"..") != 0) {

                        CString *name2 = &name;
                        FileEntry *parent = folder->parent;
                        if (parent == m_root) {
                            if (_wcsicmp(name, L"drive") == 0)
                                name2 = new CMyMsg(MSG_3522);
                            else if (_wcsicmp(name, L"share") == 0)
                                name2 = new CMyMsg(MSG_3523);
                            else if (_wcsicmp(name, L"user") == 0)
                                name2 = new CMyMsg(MSG_3524);
                        } else if (parent && parent->parent == m_root) {
                            if (_wcsicmp(name, L"all") == 0)
                                name2 = new CMyMsg(MSG_3526);
                            else if (_wcsicmp(name, L"current") == 0)
                                name2 = new CMyMsg(MSG_3525);
                            else if (_wcsicmp(name, L"public") == 0)
                                name2 = new CMyMsg(MSG_3541);
                        }

                        FileEntry *child =
                            (FileEntry *)AddFolder(*name2, folder);
                        child->path =
                            new CString(folder_path + L'\\' + name);

                        if (name2 != &name)
                            delete name2;
                    }

                } else {

                    ULONG len = sizeof(FileEntry)
                              + (name.GetLength() + 1) * sizeof(WCHAR);
                    FileEntry *file = (FileEntry *)malloc(len);
                    memzero(file, len);
                    file->parent = folder;
                    wcscpy(file->name, name);
                    file->size = info->EndOfFile.QuadPart;

                    folder->children->AddTail(file);
                }
            }

            if (folder->children->GetCount() > 100)
                break;
            if (info->NextEntryOffset == 0)
                break;
            char *next_entry = ((char *)info) + info->NextEntryOffset;
            info = (FILE_DIRECTORY_INFORMATION *)next_entry;
        }
    }

    free(dir_info);

    //
    //
    //

    CloseHandle(hFile);

    if (status != STATUS_SUCCESS && status != STATUS_NO_MORE_FILES)
        return FALSE;

    return TRUE;
}


//---------------------------------------------------------------------------
// ChangeFolder
//---------------------------------------------------------------------------


int CBoxFile::ChangeFolder(const CString &path, BOOL ForceRead)
{
    FileEntry *old_curdir = (FileEntry *)m_curdir;

    //
    // follow the specified path to change the current folder
    //

    FileEntry *folder;
    int index = 0;

    if (path.GetLength() > 0 && path.GetAt(0) == L'\\') {
        if (! m_root)
            return -1;
        m_curdir = m_root;
        index = 1;
    }

    int AngleBracketIndex = path.Find(L'<');
    if (AngleBracketIndex == -1)
        AngleBracketIndex = 99999;

    while (index < path.GetLength()) {

        folder = (FileEntry *)m_curdir;

        int backslash = path.Find(L'\\', index);
        if (backslash > AngleBracketIndex) {
            int AngleBracketIndex2 = path.Find(L'>', index);
            if (backslash < AngleBracketIndex2)
                backslash = path.Find(L'\\', AngleBracketIndex2);
        }
        if (backslash == -1)
            backslash = path.GetLength();

        CString component = path.Mid(index, backslash - index);

        if (component == L"..")
            m_curdir = folder->parent;
        else {

            BOOL found = FALSE;
            if (folder->children) {

                POSITION pos = folder->children->GetHeadPosition();
                while (pos) {
                    FileEntry *child =
                        (FileEntry *)folder->children->GetNext(pos);
                    if (child->children && component == child->name) {
                        m_curdir = child;
                        found = TRUE;
                        break;
                    }
                }
            }

            if (! found) {
                m_curdir = old_curdir;
                return -1;
            }
        }

        index = backslash + 1;
    }

    //
    // refresh current folder if necessary
    //

    int retval = 0;

    folder = (FileEntry *)m_curdir;
    if (ForceRead)
        folder->analyzed = FALSE;

    if (! folder->analyzed) {

        if (folder->parent == m_root &&
                folder == ((FileEntry *)m_root)->children->GetHead()) {

            if (folder->time.dwLowDateTime != (m_RefreshCounter & 0xFFFF)) {
                DeleteFolder(folder, FALSE);
                folder->time.dwLowDateTime = (m_RefreshCounter & 0xFFFF);
            }
            CreateQuickRecoveryFolders();

        } else if (folder->path) {

            BOOL ok = ReadFolder(folder);
            if (! ok) {
                if (folder->parent == m_root || IsQuickRecoverFolder())
                    ok = TRUE;
            }
            if (! ok) {
                m_curdir = old_curdir;
                return -1;
            }
        }

        folder->analyzed = TRUE;
        return 1;
    }

    return 0;
}


//---------------------------------------------------------------------------
// IsFolderExpandedView
//---------------------------------------------------------------------------


BOOL CBoxFile::IsFolderExpandedView() const
{
    FileEntry *folder = ((FileEntry *)m_curdir);
    return folder->expanded;
}


//---------------------------------------------------------------------------
// SetFolderExpandedView
//---------------------------------------------------------------------------


void CBoxFile::SetFolderExpandedView(BOOL view)
{
    FileEntry *folder = ((FileEntry *)m_curdir);
    folder->expanded = view;
}


//---------------------------------------------------------------------------
// IsQuickRecoverFolder
//---------------------------------------------------------------------------


BOOL CBoxFile::IsQuickRecoverFolder() const
{
    FileEntry *folder = ((FileEntry *)m_curdir);
    FileEntry *qr = (FileEntry *)((FileEntry *)m_root)->children->GetHead();
    while (folder != m_root && folder != qr) {
        if (folder->parent == qr)
            return TRUE;
        folder = folder->parent;
    }
    return FALSE;
}


//---------------------------------------------------------------------------
// IsPhysicalFolder
//---------------------------------------------------------------------------


BOOL CBoxFile::IsPhysicalFolder() const
{
    FileEntry *folder = ((FileEntry *)m_curdir);
    if (folder->time.dwHighDateTime == 0
                        && folder->time.dwLowDateTime <= 0xFFFF)
        return FALSE;
    return TRUE;
}


//---------------------------------------------------------------------------
// GetFolderPaths
//---------------------------------------------------------------------------


void CBoxFile::GetFolderPaths(CString &TruePath, CString &CopyPath) const
{
    FileEntry *folder = ((FileEntry *)m_curdir);
    if (folder->path && folder->children)
        CopyPath = *folder->path;
    else if (folder->parent && folder->parent->path) {
        CopyPath = CString(*folder->parent->path)
                 + CString(L"\\")
                 + CString(folder->name);
    } else
        CopyPath = CString();

    TruePath = GetTruePathForCopyPath(CopyPath);

    TranslateNtToDosPath(TruePath);
    TranslateNtToDosPath(CopyPath);
}


//---------------------------------------------------------------------------
// GetFolderNumChildren
//---------------------------------------------------------------------------


int CBoxFile::GetFolderNumChildren() const
{
    CPtrList *children = ((FileEntry *)m_curdir)->children;
    if (children)
        return (int)children->GetCount();
    else
        return -1;
}


//---------------------------------------------------------------------------
// GetFolderHeadPosition
//---------------------------------------------------------------------------


POSITION CBoxFile::GetFolderHeadPosition() const
{
    return ((FileEntry *)m_curdir)->children->GetHeadPosition();
}


//---------------------------------------------------------------------------
// GetNextFolder
//---------------------------------------------------------------------------


const WCHAR *CBoxFile::GetNextFolder(
    POSITION &pos, BOOL &IsExpandedView) const
{
    while (pos) {
        FileEntry *folder = (FileEntry *)
            ((FileEntry *)m_curdir)->children->GetNext(pos);
        if (folder->children) {
            IsExpandedView = folder->expanded;
            return folder->name;
        }
    }
    return NULL;
}


//---------------------------------------------------------------------------
// GetNextFile
//---------------------------------------------------------------------------


const WCHAR *CBoxFile::GetNextFile(POSITION &pos, ULONG64 &size) const
{
    while (pos) {
        FileEntry *file = (FileEntry *)
            ((FileEntry *)m_curdir)->children->GetNext(pos);
        if (! file->children) {
            size = file->size;
            return file->name;
        }
    }
    return NULL;
}


//---------------------------------------------------------------------------
// SetIncludeDeleted
//---------------------------------------------------------------------------


void CBoxFile::SetIncludeDeleted(BOOL include)
{
    m_IncludeDeleted = include;
}


//---------------------------------------------------------------------------
// IsEmpty
//---------------------------------------------------------------------------


BOOL CBoxFile::IsEmpty()
{
    NTSTATUS status;
    UNICODE_STRING uni;
    OBJECT_ATTRIBUTES objattrs;
    IO_STATUS_BLOCK MyIoStatusBlock;
    HANDLE hFile;

    CString &folder_path = m_FilePath;
    uni.Buffer = (WCHAR *)(const WCHAR *)folder_path;
    uni.Length = folder_path.GetLength() * sizeof(WCHAR);
    uni.MaximumLength = uni.Length + sizeof(WCHAR);

    InitializeObjectAttributes(
        &objattrs, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = NtCreateFile(
        &hFile, FILE_GENERIC_READ, &objattrs, &MyIoStatusBlock, NULL,
        0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN, FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
        NULL, NULL);

    if (status == STATUS_OBJECT_NAME_NOT_FOUND ||
        status == STATUS_OBJECT_PATH_NOT_FOUND ||
        status == STATUS_OBJECT_PATH_SYNTAX_BAD ||
        status == STATUS_NO_MEDIA_IN_DEVICE)
        return TRUE;

    if (! NT_SUCCESS(status))
        return FALSE;

    //
    //
    //

    BOOL empty = TRUE;

    const ULONG DIR_INFO_LENGTH = 10240;

    void *dir_info = malloc(DIR_INFO_LENGTH);
    status = STATUS_SUCCESS;

    while (status == STATUS_SUCCESS && empty) {

        status = NtQueryDirectoryFile(
            hFile, NULL, NULL, NULL, &MyIoStatusBlock,
            dir_info, DIR_INFO_LENGTH, FileDirectoryInformation,
            FALSE, NULL, FALSE);

        if (status != STATUS_SUCCESS &&
            status != STATUS_BUFFER_OVERFLOW)
            break;

        FILE_DIRECTORY_INFORMATION *info =
            (FILE_DIRECTORY_INFORMATION *)dir_info;

        while (empty) {
            CString name(
                info->FileName, info->FileNameLength / sizeof(WCHAR));
            if (name.Compare(L".") != 0 && name.Compare(L"..") != 0)
                empty = FALSE;
            else {
                if (info->NextEntryOffset == 0)
                    break;
                char *next_entry = ((char *)info) + info->NextEntryOffset;
                info = (FILE_DIRECTORY_INFORMATION *)next_entry;
            }
        }
    }

    free(dir_info);

    CloseHandle(hFile);

    return empty;
}


//---------------------------------------------------------------------------
// GetBoxRefreshCounter
//---------------------------------------------------------------------------


int CBoxFile::GetBoxRefreshCounter() const
{
    return m_RefreshCounter;
}


//---------------------------------------------------------------------------
// GetBoxCreationTime
//---------------------------------------------------------------------------


BOOL CBoxFile::GetBoxCreationTime(FILETIME *out_time)
{
    NTSTATUS status;
    UNICODE_STRING uni;
    OBJECT_ATTRIBUTES objattrs;
    IO_STATUS_BLOCK MyIoStatusBlock;
    HANDLE hFile;
    BOOL ok;

    CString &folder_path = m_FilePath;
    uni.Buffer = (WCHAR *)(const WCHAR *)folder_path;
    uni.Length = folder_path.GetLength() * sizeof(WCHAR);
    uni.MaximumLength = uni.Length + sizeof(WCHAR);

    InitializeObjectAttributes(
        &objattrs, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = NtCreateFile(
        &hFile, FILE_GENERIC_READ, &objattrs, &MyIoStatusBlock, NULL,
        0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN, FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
        NULL, NULL);

    if (status == STATUS_OBJECT_NAME_NOT_FOUND ||
        status == STATUS_OBJECT_PATH_NOT_FOUND ||
        status == STATUS_OBJECT_PATH_SYNTAX_BAD)
        return FALSE;

    if (! NT_SUCCESS(status))
        return FALSE;

    //
    //
    //

    ok = GetFileTime(hFile, out_time, NULL, NULL);

    CloseHandle(hFile);

    return ok;
}


//---------------------------------------------------------------------------
// QueryFileAttributes
//---------------------------------------------------------------------------


BOOL CBoxFile::QueryFileAttributes(
    const WCHAR *path, ULONG *attrs, ULONG64 *size)
{
    CString prefixed_path = CString(L"\\??\\") + path;
    return SbieDll_QueryFileAttributes((const WCHAR*)prefixed_path, size, NULL, attrs);
}
