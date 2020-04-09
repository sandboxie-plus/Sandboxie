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
// Files in Box
//---------------------------------------------------------------------------


#ifndef _MY_BOX_FILE_H
#define _MY_BOX_FILE_H


class CBoxFile
{
    const CString &m_name;

    CString m_FilePath;
    CString m_FilePathDos;
    void *m_root;
    void *m_curdir;

    BOOL m_IncludeDeleted;

    static int m_RefreshCounter;

    //
    //
    //

    void CreateSkeletonTree();
    void CreateQuickRecoveryFolders();
    bool GetAbsolutePathForRecoveryFolder(WCHAR *buf, ULONG buf_len);

    void DeleteFolder(void *voidfolder, BOOL DeleteSelf);
    void *AddFolder(const WCHAR *name, void *parent);
    BOOL ReadFolder(void *voidfolder);

    //
    //
    //

public:

    CBoxFile(const CString &name);
    ~CBoxFile();

    const CString &GetPathNt() const;
    const CString &GetPathDos() const;

    void RebuildSkeletonTree();
    void RebuildQuickRecoveryFolders();

    CString GetCopyPathForTruePath(const WCHAR *TruePath) const;
    CString GetTruePathForCopyPath(const WCHAR *CopyPath) const;
    CString GetDriveNameForTruePath(const WCHAR *TruePath) const;
    void TranslateNtToDosPath(CString &InOutPath) const;

    int ChangeFolder(const CString &path, BOOL ForceRead = FALSE);

    BOOL IsFolderExpandedView() const;
    void SetFolderExpandedView(BOOL view);
    BOOL IsQuickRecoverFolder() const;
    BOOL IsPhysicalFolder() const;
    void GetFolderPaths(CString &TruePath, CString &CopyPath) const;
    void GetFolderCreationTime(FILETIME *out_time) const;

    int GetFolderNumChildren() const;
    POSITION GetFolderHeadPosition() const;
    const WCHAR *GetNextFolder(POSITION &pos, BOOL &IsExpandedView) const;
    const WCHAR *GetNextFile(POSITION &pos, ULONG64 &size) const;

    void SetIncludeDeleted(BOOL include);

    BOOL IsEmpty();

    BOOL GetBoxCreationTime(FILETIME *out_time);
    int GetBoxRefreshCounter() const;

    BOOL QueryFileAttributes(const WCHAR *path, ULONG *attrs, ULONG64 *size);

};


#endif // _MY_BOX_FILE_H
