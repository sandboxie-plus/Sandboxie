/*
 * Copyright 2023 David Xanatos, xanasoft.com
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
// Mount Manager
//---------------------------------------------------------------------------


#ifndef _MY_MOUNTMANAGER_H
#define _MY_MOUNTMANAGER_H


#include <windows.h>
#include "common/win32_ntddk.h"
#include "common/list.h"
#include "common/map.h"
#include "common/pool.h"

#include "PipeServer.h"

#include <string>
#include <map>
#include <list>
#include <vector>
#include <memory>

struct BOX_MOUNT;
struct BOX_ROOT;

class MountManager
{

public:

    MountManager(PipeServer *pipeServer);

    static MountManager *GetInstance();

    static void Shutdown();

    bool AcquireBoxRoot(const WCHAR* boxname, const WCHAR* reg_root, const WCHAR* file_root, ULONG session_id);
    void LockBoxRoot(const WCHAR* reg_root, ULONG session_id);
    void ReleaseBoxRoot(const WCHAR* reg_root, bool force, ULONG session_id);

protected:

    static MSG_HEADER *Handler(void *_this, MSG_HEADER *msg);
    MSG_HEADER *CreateHandler(MSG_HEADER *msg);
    MSG_HEADER *MountHandler(MSG_HEADER *msg);
    MSG_HEADER *UnmountHandler(MSG_HEADER *msg);
    MSG_HEADER *EnumHandler(MSG_HEADER *msg);
    MSG_HEADER *QueryHandler(MSG_HEADER *msg);
    MSG_HEADER *UpdateHandler(MSG_HEADER *msg);

    //static DWORD CleanUp(LPVOID lpThreadParameter);
    
    void UnmountAll();

    std::shared_ptr<BOX_ROOT> GetBoxRootLocked(const WCHAR* reg_root, bool bCanAdd, ULONG session_id = -1);
    std::wstring GetImageFileName(const WCHAR* file_root);
    std::wstring GetProxyName(const std::wstring& ImageFile);

    HANDLE OpenOrCreateNtFolder(const WCHAR* NtPath);

    int CreateJunction(const std::wstring& TargetNtPath, const std::wstring& FileRootPath, ULONG session_id);
    bool RemoveJunction(const std::wstring& FileRootPath, ULONG session_id);

    std::shared_ptr<BOX_MOUNT> FindImDisk(const std::wstring& ImageFile, ULONG session_id);
    std::shared_ptr<BOX_MOUNT> MountImDisk(const std::wstring& ImageFile, const wchar_t* pPassword, ULONG64 sizeKb, ULONG session_id, const wchar_t* drvLetter = NULL);
    static bool TryUnmountImDisk(const std::wstring& Device, HANDLE hProcess, int iMode = 0);
    static bool UnmountImDisk(const std::wstring& Device, HANDLE hProcess);
    bool UnmountImDiskLocked(const std::shared_ptr<BOX_MOUNT>& pToUnMount, ULONG session_id);

    std::shared_ptr<BOX_MOUNT> GetRamDisk(ULONG session_id);

    CRITICAL_SECTION m_CritSec;
    std::shared_ptr<BOX_MOUNT> m_RamDisk;
    std::map<std::wstring, std::shared_ptr<BOX_ROOT> > m_RootMap;
    //HANDLE m_hCleanUpThread;

    static MountManager* m_instance;
};


#endif /* _MY_MOUNTMANAGER_H */
