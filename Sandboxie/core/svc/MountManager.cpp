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

#include "stdafx.h"

#include "MountManager.h"
#include "ProcessServer.h"
#include "core/drv/api_defs.h"
#include "MountManagerWire.h"
#include "common/win32_ntddk.h"
#include <winioctl.h>
#include <psapi.h>
#include "common/my_version.h"
#include <algorithm>
#include "../../../SandboxieTools/ImBox/ImBox.h"
#include "../dll/sbiedll.h"
#include "core/drv/verify.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define REPARSE_MOUNTPOINT_HEADER_SIZE 8

#define IMDISK_DEVICE_LEN 14
#define IMDISK_DEVICE L"\\Device\\ImDisk"
#define SBIEDISK_LABEL L"Sandbox"
#define RAMDISK_IMAGE L"RamDisk"

#define IMBOX_PROXY L"ImBox_Proxy"
#define IMBOX_EVENT L"ImBox_Event"
#define IMBOX_SECTION L"ImBox_Section"


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef int (*P_SHCreateDirectoryExW)(HWND hwnd, LPCWSTR pszPath, const SECURITY_ATTRIBUTES* psa);

// from winfile
typedef BOOLEAN(*FMIFS_CALLBACK)(ULONG PacketType, ULONG PacketLength, PVOID PacketData);
typedef void (WINAPI* PFORMAT) (PWSTR DriveName, ULONG MediaType, PWSTR FileSystemName, PWSTR Label, BOOLEAN Quick, FMIFS_CALLBACK Callback);

struct BOX_MOUNT
{
    std::wstring NtPath;
    std::wstring ImageFile;
    bool Protected = false;
    bool Unmounting = false;
    //int RefCount = 0;
    HANDLE ProcessHandle = NULL;
};

struct BOX_ROOT
{
    std::shared_ptr<BOX_MOUNT> Mount;
    ULONG session_id;
    bool InUse = false;
    bool AutoUnmount = false;
    std::wstring Path;
};


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


extern "C" {

    HANDLE WINAPI ImDiskOpenDeviceByMountPoint(LPCWSTR MountPoint, DWORD AccessMode);
    BOOL WINAPI IsImDiskDriverReady();
    BOOL WINAPI ImDiskGetDeviceListEx(IN ULONG ListLength, OUT ULONG* DeviceList);
    WCHAR WINAPI ImDiskFindFreeDriveLetter();
}

std::wstring GetVolumeLabel(const std::wstring& NtPath);
std::wstring ImDiskQueryDeviceProxy(const std::wstring& FileName);
ULONGLONG ImDiskQueryDeviceSize(const std::wstring& FileName);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


P_SHCreateDirectoryExW __sys_SHCreateDirectoryExW = NULL;

MountManager* MountManager::m_instance = NULL;


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


MountManager::MountManager(PipeServer *pipeServer)
{
    InitializeCriticalSection(&m_CritSec);

    HMODULE shell32 = LoadLibrary(L"shell32.dll");
    __sys_SHCreateDirectoryExW = (P_SHCreateDirectoryExW)GetProcAddress(shell32, "SHCreateDirectoryExW");

    //m_hCleanUpThread = INVALID_HANDLE_VALUE;

    pipeServer->Register(MSGID_IMBOX, this, Handler);

    // todo: find mounted disks

    m_instance = this;
}


//---------------------------------------------------------------------------
// MountManager
//---------------------------------------------------------------------------


MountManager *MountManager::GetInstance()
{
    return m_instance;
}


//---------------------------------------------------------------------------
// Shutdown
//---------------------------------------------------------------------------


void MountManager::Shutdown()
{
    if (m_instance) {

        m_instance->UnmountAll();

        //for (int i = 0; i < 10 * 10 && m_instance->m_hCleanUpThread != INVALID_HANDLE_VALUE; i++)
        //    Sleep(100);

        delete m_instance;
        m_instance = NULL;
    }
}


//---------------------------------------------------------------------------
// Handler
//---------------------------------------------------------------------------


MSG_HEADER *MountManager::Handler(void *_this, MSG_HEADER *msg)
{
    MountManager *pThis = (MountManager *)_this;

    HANDLE idProcess = (HANDLE)(ULONG_PTR)PipeServer::GetCallerProcessId();

    if (0 == SbieApi_QueryProcess(idProcess, NULL, NULL, NULL, NULL))
        return SHORT_REPLY(STATUS_ACCESS_DENIED);

    if (msg->msgid == MSGID_IMBOX_CREATE) {
        return pThis->CreateHandler(msg);
    }

    if (msg->msgid == MSGID_IMBOX_MOUNT) {
        return pThis->MountHandler(msg);
    }

    if (msg->msgid == MSGID_IMBOX_UNMOUNT) {
        return pThis->UnmountHandler(msg);
    }

    if (msg->msgid == MSGID_IMBOX_ENUM) {
        return pThis->EnumHandler(msg);
    }

    if (msg->msgid == MSGID_IMBOX_QUERY) {
        return pThis->QueryHandler(msg);
    }

    if (msg->msgid == MSGID_IMBOX_UPDATE) {
        return pThis->UpdateHandler(msg);
    }

    return NULL;
}


//---------------------------------------------------------------------------
// CreateHandler
//---------------------------------------------------------------------------


MSG_HEADER *MountManager::CreateHandler(MSG_HEADER *msg)
{
    //
    // parse request packet
    //

    IMBOX_CREATE_REQ *req = (IMBOX_CREATE_REQ *)msg;
    if (req->h.length < sizeof(IMBOX_CREATE_REQ))
        return SHORT_REPLY(ERROR_INVALID_PARAMETER);

    ULONG session_id = PipeServer::GetCallerSessionId();

    std::wstring ImageFile = GetImageFileName(req->file_root);

    std::wstring RootPath(req->file_root, wcsrchr(req->file_root, L'\\'));
    HANDLE handle = OpenOrCreateNtFolder(RootPath.c_str());
    if (!handle)
        return SHORT_REPLY(ERROR_PATH_NOT_FOUND);
    CloseHandle(handle);

    std::shared_ptr<BOX_MOUNT> pMount = MountImDisk(ImageFile, req->password, req->image_size, session_id);
    if(!pMount)
        return SHORT_REPLY(ERROR_FUNCTION_FAILED);

    UnmountImDisk(pMount->NtPath, pMount->ProcessHandle);
    return SHORT_REPLY(ERROR_SUCCESS);
}


//---------------------------------------------------------------------------
// MountHandler
//---------------------------------------------------------------------------


MSG_HEADER *MountManager::MountHandler(MSG_HEADER *msg)
{
    //
    // parse request packet
    //

    IMBOX_MOUNT_REQ *req = (IMBOX_MOUNT_REQ *)msg;
    if (req->h.length < sizeof(IMBOX_MOUNT_REQ))
        return SHORT_REPLY(ERROR_INVALID_PARAMETER);

    ULONG session_id = PipeServer::GetCallerSessionId();

    std::wstring ImageFile = GetImageFileName(req->file_root);

    std::shared_ptr<BOX_MOUNT> pMount = FindImDisk(ImageFile, session_id);
    if (!pMount) {
        pMount = MountImDisk(ImageFile, req->password, 0, session_id);
        if (!pMount)
            return SHORT_REPLY(ERROR_FUNCTION_FAILED);
    }

    ULONG errlvl = 0;
    std::wstring TargetNtPath = pMount->NtPath;

    if (req->protect_root) {
        if (NT_SUCCESS(SbieApi_Call(API_PROTECT_ROOT, 2, req->reg_root, TargetNtPath.c_str())))
            pMount->Protected = true;
    }


    std::shared_ptr<BOX_ROOT> pRoot = GetBoxRootLocked(req->reg_root, true, session_id);

    pRoot->AutoUnmount = req->auto_unmount;
    pRoot->Mount = pMount;
    //pRoot->Mount->RefCount++;
    pRoot->Path = req->file_root;

    LeaveCriticalSection(&m_CritSec);


    if (TargetNtPath.back() != L'\\') TargetNtPath.push_back(L'\\');
    TargetNtPath += SBIEDISK_LABEL;
    HANDLE handle = OpenOrCreateNtFolder(TargetNtPath.c_str());
    if (!handle)
        errlvl = 0x12;
    else 
    {
        CloseHandle(handle);

        errlvl = CreateJunction(TargetNtPath, req->file_root, session_id);
    }

    if (errlvl != 0) {

        //ULONG err = GetLastError();
        
        ReleaseBoxRoot(req->reg_root, true, session_id);

        return SHORT_REPLY(ERROR_FUNCTION_FAILED);
    }
    return SHORT_REPLY(ERROR_SUCCESS);
}


//---------------------------------------------------------------------------
// UnmountHandler
//---------------------------------------------------------------------------


MSG_HEADER *MountManager::UnmountHandler(MSG_HEADER *msg)
{
    //
    // parse request packet
    //

    IMBOX_UNMOUNT_REQ *req = (IMBOX_UNMOUNT_REQ *)msg;
    if (req->h.length < sizeof(IMBOX_UNMOUNT_REQ))
        return SHORT_REPLY(ERROR_INVALID_PARAMETER);

    ULONG session_id = PipeServer::GetCallerSessionId();

    std::shared_ptr<BOX_ROOT> pRoot = GetBoxRootLocked(req->reg_root, false, session_id);
    if(!pRoot)
        return SHORT_REPLY(ERROR_SUCCESS);

    bool ok = false;

    if (!pRoot->InUse) {

        RemoveJunction(pRoot->Path, session_id);

        if (pRoot->Mount->Protected)
            SbieApi_Call(API_UNPROTECT_ROOT, 1, req->reg_root);

        ok = UnmountImDiskLocked(pRoot->Mount, session_id);
    }

    LeaveCriticalSection(&m_CritSec);

    if(!ok)
        return SHORT_REPLY(ERROR_FUNCTION_FAILED);
    return SHORT_REPLY(ERROR_SUCCESS);
}


//---------------------------------------------------------------------------
// EnumHandler
//---------------------------------------------------------------------------


MSG_HEADER *MountManager::EnumHandler(MSG_HEADER *msg)
{
    //
    // parse request packet
    //

    IMBOX_ENUM_REQ *req = (IMBOX_ENUM_REQ *)msg;
    if (req->h.length < sizeof(IMBOX_ENUM_REQ))
        return SHORT_REPLY(ERROR_INVALID_PARAMETER);

    if(!IsImDiskDriverReady())
        return SHORT_REPLY(ERROR_DEVICE_NOT_AVAILABLE);

    EnterCriticalSection(&m_CritSec);

    ULONG rpl_len = sizeof(IMBOX_ENUM_RPL) + sizeof(WCHAR);

    rpl_len += (m_RootMap.size() * MAX_REG_ROOT_LEN) * sizeof(WCHAR);

    IMBOX_ENUM_RPL *rpl = (IMBOX_ENUM_RPL *)LONG_REPLY(rpl_len);
    if (rpl) {

        rpl->h.status = ERROR_SUCCESS;

        WCHAR *reg_roots = rpl->reg_roots;
        for (auto I = m_RootMap.begin(); I != m_RootMap.end(); I++) {
            wcscpy(reg_roots, I->first.c_str());
            reg_roots += wcslen(reg_roots) + 1;
        }
        *reg_roots = L'\0';
    }

    LeaveCriticalSection(&m_CritSec);
    
    if (! rpl)
        return SHORT_REPLY(ERROR_NOT_ENOUGH_MEMORY);

    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// QueryHandler
//---------------------------------------------------------------------------


MSG_HEADER *MountManager::QueryHandler(MSG_HEADER *msg)
{
    //
    // parse request packet
    //

    IMBOX_QUERY_REQ *req = (IMBOX_QUERY_REQ *)msg;
    if (req->h.length < sizeof(IMBOX_QUERY_REQ))
        return SHORT_REPLY(ERROR_INVALID_PARAMETER);

    if(!IsImDiskDriverReady())
        return SHORT_REPLY(ERROR_DEVICE_NOT_AVAILABLE);

    std::shared_ptr<BOX_MOUNT> pMount;
    if (*req->reg_root) { 
        std::shared_ptr<BOX_ROOT> pRoot = GetBoxRootLocked(req->reg_root, false);
        if (pRoot)
            pMount = pRoot->Mount;
    }
    else if (m_RamDisk != NULL) { // empty root to query ramdisk
        EnterCriticalSection(&m_CritSec);
        pMount = m_RamDisk;
    }

    IMBOX_QUERY_RPL* rpl = NULL;
    if (pMount) {

        ULONG rpl_len = sizeof(IMBOX_QUERY_RPL);
        rpl_len += pMount->NtPath.size() * sizeof(WCHAR);

        rpl = (IMBOX_QUERY_RPL*)LONG_REPLY(rpl_len);
        if (rpl) {

            rpl->h.status = ERROR_SUCCESS;

            //scscpy(rpl->boxname, pRoot

            rpl->disk_size = ImDiskQueryDeviceSize(pMount->NtPath);
            if (pMount->ImageFile.empty()) { // ram disk - memory used by ImBox process
                PROCESS_MEMORY_COUNTERS_EX pmc;
                if (GetProcessMemoryInfo(pMount->ProcessHandle, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc)))
                    rpl->used_size = pmc.PrivateUsage;
            }
            else { // sparse image file size
                LARGE_INTEGER liSparseFileCompressedSize;
                liSparseFileCompressedSize.LowPart = GetCompressedFileSize(pMount->ImageFile.c_str(), (LPDWORD)&liSparseFileCompressedSize.HighPart);
                rpl->used_size = liSparseFileCompressedSize.QuadPart;
            }
            wcscpy(rpl->disk_root, pMount->NtPath.c_str());
        }

        LeaveCriticalSection(&m_CritSec);
    }
    else
        return SHORT_REPLY(ERROR_NOT_FOUND);
    if (! rpl)
        return SHORT_REPLY(ERROR_NOT_ENOUGH_MEMORY);
    return (MSG_HEADER *)rpl;
}


//---------------------------------------------------------------------------
// UpdateHandler
//---------------------------------------------------------------------------


MSG_HEADER *MountManager::UpdateHandler(MSG_HEADER *msg)
{
    //
    // parse request packet
    //

    IMBOX_UPDATE_REQ *req = (IMBOX_UPDATE_REQ *)msg;
    if (req->h.length < sizeof(IMBOX_UPDATE_REQ))
        return SHORT_REPLY(ERROR_INVALID_PARAMETER);

    // imdisk -e -s 1g -u 1

    return SHORT_REPLY(ERROR_CALL_NOT_IMPLEMENTED); // todo
}


//---------------------------------------------------------------------------
// OpenOrCreateNtFolder
//---------------------------------------------------------------------------


HANDLE MountManager::OpenOrCreateNtFolder(const WCHAR* NtPath)
{
    UNICODE_STRING objname;
    RtlInitUnicodeString(&objname, NtPath);

    OBJECT_ATTRIBUTES objattrs;
    InitializeObjectAttributes(&objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, SbieDll_GetPublicSD());

    HANDLE handle = NULL;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS status;

    status = NtCreateFile(&handle, GENERIC_READ | GENERIC_WRITE, &objattrs, &IoStatusBlock, NULL, 0, FILE_SHARE_VALID_FLAGS, FILE_OPEN_IF, FILE_DIRECTORY_FILE | FILE_OPEN_REPARSE_POINT, NULL, 0);
    if (status == STATUS_OBJECT_PATH_NOT_FOUND) {

        std::wstring DosPath = NtPath;
        if (!SbieDll_TranslateNtToDosPath((WCHAR*)DosPath.c_str()))
            return NULL;

        WCHAR* dosPath = (WCHAR*)DosPath.c_str();
        *wcsrchr(dosPath, L'\\') = L'\0'; // truncate path as we want the last folder to be created with SbieDll_GetPublicSD
        if (__sys_SHCreateDirectoryExW(NULL, dosPath, NULL) != ERROR_SUCCESS)
            return NULL;

        status = NtCreateFile(&handle, GENERIC_READ | GENERIC_WRITE, &objattrs, &IoStatusBlock, NULL, 0, FILE_SHARE_VALID_FLAGS, FILE_OPEN_IF, FILE_DIRECTORY_FILE | FILE_OPEN_REPARSE_POINT, NULL, 0);
    }
    if (!NT_SUCCESS(status))
        return NULL;

    return handle;
}


//---------------------------------------------------------------------------
// CreateJunction
//---------------------------------------------------------------------------


int MountManager::CreateJunction(const std::wstring& TargetNtPath, const std::wstring& FileRootPath, ULONG session_id)
{
    ULONG errlvl = 0;
    HANDLE handle;

    //
    // open root folder, if not exist, create it
    //

    handle = OpenOrCreateNtFolder(FileRootPath.c_str());
    if(!handle)
        errlvl = 0x33;

    //
    // get the junction target
    //

    BYTE buf[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];  // We need a large buffer
    REPARSE_DATA_BUFFER& ReparseBuffer = (REPARSE_DATA_BUFFER&)buf;
    DWORD dwRet;

    std::wstring JunctionTarget;
    if (errlvl == 0) {
        if (::DeviceIoControl(handle, FSCTL_GET_REPARSE_POINT, NULL, 0, &ReparseBuffer, MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &dwRet, NULL)) {
            JunctionTarget = ReparseBuffer.MountPointReparseBuffer.PathBuffer;
        }
        // else not a junction
    }

    //
    // check if the junction target is right, if not remove it
    //

    if (errlvl == 0 && !JunctionTarget.empty()) {
        if (_wcsicmp(JunctionTarget.c_str(), TargetNtPath.c_str()) != 0) {
            SbieApi_LogEx(session_id, 2231, L"%S != %S", JunctionTarget.c_str(), TargetNtPath.c_str());

            memset(buf, 0, REPARSE_MOUNTPOINT_HEADER_SIZE);
	        ReparseBuffer.ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;
            if (!::DeviceIoControl(handle, FSCTL_DELETE_REPARSE_POINT, &ReparseBuffer, REPARSE_MOUNTPOINT_HEADER_SIZE, NULL, 0, &dwRet, NULL))
                errlvl = 0x55; // failed to remove invalid junction target

            JunctionTarget.clear();
        }
    }

    //
    // set junction target, if needed
    //

    if (errlvl == 0 && JunctionTarget.empty()) {
                
        memset(&ReparseBuffer, 0, sizeof(buf));
        ReparseBuffer.ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;
        ReparseBuffer.ReparseDataLength = 4 * sizeof(USHORT);

        ReparseBuffer.MountPointReparseBuffer.SubstituteNameOffset = 0;
        ReparseBuffer.MountPointReparseBuffer.SubstituteNameLength = TargetNtPath.length() * sizeof(WCHAR);
        USHORT SubstituteNameSize = ReparseBuffer.MountPointReparseBuffer.SubstituteNameLength + sizeof(WCHAR);
        memcpy(ReparseBuffer.MountPointReparseBuffer.PathBuffer, TargetNtPath.c_str(), SubstituteNameSize);
        ReparseBuffer.ReparseDataLength += SubstituteNameSize;

        ReparseBuffer.MountPointReparseBuffer.PrintNameOffset = SubstituteNameSize;
	    ReparseBuffer.MountPointReparseBuffer.PrintNameLength = TargetNtPath.length() * sizeof(WCHAR);
	    USHORT PrintNameSize = ReparseBuffer.MountPointReparseBuffer.PrintNameLength + sizeof(WCHAR);
        memcpy(ReparseBuffer.MountPointReparseBuffer.PathBuffer + SubstituteNameSize/sizeof(WCHAR), TargetNtPath.c_str(), PrintNameSize);
        ReparseBuffer.ReparseDataLength += PrintNameSize;

        if (!::DeviceIoControl(handle, FSCTL_SET_REPARSE_POINT, &ReparseBuffer, ReparseBuffer.ReparseDataLength + REPARSE_MOUNTPOINT_HEADER_SIZE, NULL, 0, &dwRet, NULL))
            errlvl = 0x44;
    }

    if (handle)
        CloseHandle(handle);

    return errlvl;
}


//---------------------------------------------------------------------------
// RemoveJunction
//---------------------------------------------------------------------------


bool MountManager::RemoveJunction(const std::wstring& FileRootPath, ULONG session_id)
{
    bool ok = false;

    UNICODE_STRING objname;
    RtlInitUnicodeString(&objname, FileRootPath.c_str());

    OBJECT_ATTRIBUTES objattrs;
    InitializeObjectAttributes(&objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    HANDLE handle = NULL;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS status;

    status = NtCreateFile(&handle, GENERIC_READ | GENERIC_WRITE, &objattrs, &IoStatusBlock, NULL, 0, FILE_SHARE_VALID_FLAGS, FILE_OPEN, FILE_DIRECTORY_FILE | FILE_OPEN_REPARSE_POINT, NULL, 0);
    if (NT_SUCCESS(status)) {

        BYTE buf[REPARSE_MOUNTPOINT_HEADER_SIZE];
        REPARSE_DATA_BUFFER& ReparseBuffer = (REPARSE_DATA_BUFFER&)buf;
        DWORD dwRet;

        //
        // remove junction
        //

        memset(&ReparseBuffer, 0, REPARSE_MOUNTPOINT_HEADER_SIZE);
        ReparseBuffer.ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;
        ok = !!::DeviceIoControl(handle, FSCTL_DELETE_REPARSE_POINT, &ReparseBuffer, REPARSE_MOUNTPOINT_HEADER_SIZE, NULL, 0, &dwRet, NULL);

        CloseHandle(handle);

        NtDeleteFile(&objattrs);
    }

    return ok;
}


//---------------------------------------------------------------------------
// GetProxyName
//---------------------------------------------------------------------------


std::wstring MountManager::GetProxyName(const std::wstring& ImageFile)
{
    std::wstring ProxyName;
    if (ImageFile.empty())
        ProxyName = RAMDISK_IMAGE;
    else {
        ProxyName = ImageFile;
        std::replace(ProxyName.begin(), ProxyName.end(), L'\\', L'/');
    }
    return ProxyName;
}


//---------------------------------------------------------------------------
// FindImDisk
//---------------------------------------------------------------------------


std::shared_ptr<BOX_MOUNT> MountManager::FindImDisk(const std::wstring& ImageFile, ULONG session_id)
{
    std::wstring TargetNtPath;

    std::wstring ProxyName = GetProxyName(ImageFile);

    //
    // Find an already mounted RamDisk,
    // we inspect the volume label to determine if its ours
    // 

    std::vector<ULONG> DeviceList;
    DeviceList.resize(3);

retry:
    if (!ImDiskGetDeviceListEx(DeviceList.size(), &DeviceList.front())) {
        switch (GetLastError())
        {
        case ERROR_FILE_NOT_FOUND:
            SbieApi_LogEx(session_id, 2232, L"");
            return NULL;

        case ERROR_MORE_DATA:
            DeviceList.resize(DeviceList[0] + 1);
            goto retry;

        default:
            SbieApi_LogEx(session_id, 2233, L"%d", GetLastError());
            return NULL;
        }
    }

    for (ULONG counter = 1; counter <= DeviceList[0]; counter++) {

        std::wstring proxy = ImDiskQueryDeviceProxy(IMDISK_DEVICE + std::to_wstring(DeviceList[counter]));
        if (_wcsnicmp(proxy.c_str(), L"\\BaseNamedObjects\\Global\\" IMBOX_PROXY, 25 + 11) != 0)
            continue;
        std::size_t pos = proxy.find_first_of(L'!');
        if (pos == std::wstring::npos || _wcsicmp(proxy.c_str() + (pos + 1), ProxyName.c_str()) != 0)
            continue;

        //if (GetVolumeLabel(IMDISK_DEVICE + std::to_wstring(DeviceList[counter]) + L"\\") != SBIEDISK_LABEL) 
        //  continue;

        TargetNtPath = IMDISK_DEVICE + std::to_wstring(DeviceList[counter]);
        break;
    }

    std::shared_ptr<BOX_MOUNT> pMount;
    if (!TargetNtPath.empty()) {
        pMount = std::make_shared<BOX_MOUNT>();
        pMount->NtPath = TargetNtPath;
        pMount->ImageFile = ImageFile;
        //pMount->Protected = todo: query driver
    }
    return pMount;
}


//---------------------------------------------------------------------------
// MountImDisk
//---------------------------------------------------------------------------


std::shared_ptr<BOX_MOUNT> MountManager::MountImDisk(const std::wstring& ImageFile, const wchar_t* pPassword, ULONG64 sizeKb, ULONG session_id, const wchar_t* drvLetter)
{
    bool ok = false;

    std::shared_ptr<BOX_MOUNT> pMount = std::make_shared<BOX_MOUNT>();

    std::wstring ProxyName = GetProxyName(ImageFile);

    //
    // mount a new disk
    // we need to use a temporary drive letter in order to format the volume using the fmifs.dll API
    //

    // todo allow mounting without mount

    WCHAR Drive[4] = L"\0:";
    if (drvLetter) {
        WCHAR letter = towupper(drvLetter[0]);
        if (letter >= L'A' && letter <= L'Z' && drvLetter[1] == L':') {
            DWORD logical_drives = GetLogicalDrives();
            if((logical_drives & (1 << (letter - L'A'))) == 0)
                Drive[0] = letter;
        }
    }
    else
        Drive[0] = ImDiskFindFreeDriveLetter();
    if (Drive[0] == 0) {
        SbieApi_LogEx(session_id, 2234, L"");
        return NULL;
    }

    std::wstring cmd;
    if (ImageFile.empty()) cmd = L"ImBox type=ram";
    else cmd = L"ImBox type=img image=\"" + ImageFile + L"\"";
    if (pPassword && *pPassword) cmd += L" cipher=AES";
    //cmd += L" size=" + std::to_wstring(sizeKb * 1024ull) + L" mount=" + std::wstring(Drive) + L" format=ntfs:" SBIEDISK_LABEL;
    cmd += L" size=" + std::to_wstring(sizeKb * 1024ull) + L" mount=" + std::wstring(Drive) + L" format=ntfs";

#ifdef _M_ARM64
	ULONG64 ctr = _ReadStatusReg(ARM64_CNTVCT);
#else
	ULONG64 ctr = __rdtsc();
#endif

    WCHAR sName[32];
    wsprintf(sName, L"_%08X_%08X%08X", GetCurrentProcessId(), (ULONG)(ctr >> 32), (ULONG)ctr);

    cmd += L" proxy=" IMBOX_PROXY + std::wstring(sName) + L"!" + ProxyName;

    HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, (IMBOX_EVENT + std::wstring(sName)).c_str());
    cmd += L" event=" IMBOX_EVENT + std::wstring(sName);

    WCHAR* pSection = NULL;
    HANDLE hMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 0x1000, (IMBOX_SECTION + std::wstring(sName)).c_str());
    if (hMapping) {
        pSection = (WCHAR*)MapViewOfFile(hMapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0x1000);
        memzero(pSection, 0x1000);
    }
    cmd += L" section=" IMBOX_SECTION + std::wstring(sName);

    if (pPassword && *pPassword)
        wmemcpy(pSection, pPassword, wcslen(pPassword) + 1);


    WCHAR app[768];
    if (!NT_SUCCESS(SbieApi_GetHomePath(NULL, 0, app, 768)))
        return NULL;
    wcscat(app, L"\\ImBox.exe");
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi = { 0 };
    if (CreateProcess(app, (WCHAR*)cmd.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {

        //
        // Wait for ImDisk to be mounted
        //

        HANDLE hEvents[] = { hEvent, pi.hProcess };
        DWORD dwEvent = WaitForMultipleObjects(ARRAYSIZE(hEvents), hEvents, FALSE, 40 * 1000);
        if (dwEvent != WAIT_OBJECT_0) {
            DWORD ret;
            GetExitCodeProcess(pi.hProcess, &ret);
            if (ret == STILL_ACTIVE) {
                SbieApi_LogEx(session_id, 2240, L"%S", Drive);
                //TerminateProcess(pi.hProcess, -1);
            }
            else {
                switch (ret) {
                case ERR_FILE_NOT_OPENED:   SbieApi_LogEx(session_id, 2241, L"%S", ImageFile.c_str()); break;
                case ERR_UNKNOWN_CIPHER:    SbieApi_LogEx(session_id, 2242, L""); break;
                case ERR_WRONG_PASSWORD:    SbieApi_LogEx(session_id, 2243, L""); break;
                case ERR_KEY_REQUIRED:      SbieApi_LogEx(session_id, 2244, L""); break;
                case ERR_IMDISK_FAILED:     SbieApi_LogEx(session_id, 2236, L"ImDisk"); break;
                case ERR_IMDISK_TIMEOUT:    SbieApi_LogEx(session_id, 2240, L"%S", Drive); break;
                default:                    SbieApi_LogEx(session_id, 2246, L"%d", ret); break;
                }
            }
        }
        else {

            if(_wcsnicmp(pSection, IMDISK_DEVICE, IMDISK_DEVICE_LEN) == 0)
                pMount->NtPath = std::wstring(pSection);
            else { // fallback
                HANDLE handle = ImDiskOpenDeviceByMountPoint(Drive, 0);
                if (handle != INVALID_HANDLE_VALUE) {
                    BYTE buffer[MAX_PATH];
                    DWORD length = sizeof(buffer);
                    if (NT_SUCCESS(NtQueryObject(handle, ObjectNameInformation, buffer, length, &length))) {
                        UNICODE_STRING* uni = &((OBJECT_NAME_INFORMATION*)buffer)->Name;
                        length = uni->Length / sizeof(WCHAR);
                        if (uni->Buffer) {
                            uni->Buffer[length] = 0;
                            pMount->NtPath = uni->Buffer;
                        }
                    }
                    CloseHandle(handle);
                }
            }

            if (!pMount->NtPath.empty()) {
                    
                ok = true;

                if (!drvLetter) {
                    if (!DefineDosDevice(DDD_REMOVE_DEFINITION | DDD_EXACT_MATCH_ON_REMOVE | DDD_RAW_TARGET_PATH, Drive, pMount->NtPath.c_str())) {
                        SbieApi_LogEx(session_id, 2235, L"%S", Drive);
                    }
                }
            }
        }

        if(ok)
            pMount->ProcessHandle = pi.hProcess;
        else
            CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else
        SbieApi_LogEx(session_id, 2236, L"ImBox");

    if (pSection) {
        memzero(pSection, 0x1000);
        UnmapViewOfFile(pSection);
    }
    if(hMapping) 
        CloseHandle(hMapping);

    if(hEvent) 
        CloseHandle(hEvent);

    if (!ok)
        return NULL;
    pMount->ImageFile = ImageFile;
    return pMount;
}


//---------------------------------------------------------------------------
// GetBoxRootLocked
//---------------------------------------------------------------------------


std::shared_ptr<BOX_ROOT> MountManager::GetBoxRootLocked(const WCHAR* reg_root, bool bCanAdd, ULONG session_id)
{
    std::shared_ptr<BOX_ROOT> pRoot;

    int mayRetry = 20 * 10;
retry:
    EnterCriticalSection(&m_CritSec);

    auto I = m_RootMap.find(reg_root);

    if (I != m_RootMap.end())
        pRoot = I->second;
    else if (!bCanAdd) {
        LeaveCriticalSection(&m_CritSec);
        return NULL;
    }
    else {
        pRoot = std::make_shared<BOX_ROOT>();
        pRoot->session_id = session_id;
        m_RootMap.insert(std::make_pair(reg_root, pRoot));
    }
   
    // is im process of being unmounted, wait to finish
//    if (pRoot->Mount && pRoot->Mount->RefCount == -1) {
    if (pRoot->Mount && pRoot->Mount->Unmounting) {
        LeaveCriticalSection(&m_CritSec);
        Sleep(100);
        if(--mayRetry > 0)
            goto retry;
    }

    return pRoot;
}


//---------------------------------------------------------------------------
// GetImageFileName
//---------------------------------------------------------------------------


std::wstring MountManager::GetImageFileName(const WCHAR* file_root)
{
    std::wstring ImageFile = file_root;
    SbieDll_TranslateNtToDosPath((WCHAR*)ImageFile.c_str());
    ImageFile.resize(wcslen(ImageFile.c_str()));
    ImageFile += L".box";
    return ImageFile;
}


//---------------------------------------------------------------------------
// AcquireBoxRoot
//---------------------------------------------------------------------------


bool MountManager::AcquireBoxRoot(const WCHAR* boxname, const WCHAR* reg_root, const WCHAR* file_root, ULONG session_id)
{
    BOOLEAN UseRamDisk = SbieApi_QueryConfBool(boxname, L"UseRamDisk", FALSE);
    BOOLEAN UseFileImage = SbieApi_QueryConfBool(boxname, L"UseFileImage", FALSE);

    //
    // We use the [KeyRootPath] to uniquely identify a sandbox, the driver requires
    // both [KeyRootPath] as well as the hive file location [FileRootPath]\RegHive to match,
    // hence either is a good unique identifier, in case of a conflict the second sandbox start attempt fails.
    // As SbieApi_GetUnmountHive provides only [KeyRootPath] and no file path it is expedient to use it.
    //

    ULONG errlvl = 0;

    std::shared_ptr<BOX_ROOT> pRoot = GetBoxRootLocked(reg_root, UseRamDisk || UseFileImage, session_id);
    if (!pRoot) // when NULL is returned m_CritSec is not locked
        return true; 
    if (!UseRamDisk && !UseFileImage) { // when a box root was found but is not needed, release it
        LeaveCriticalSection(&m_CritSec);
        ReleaseBoxRoot(reg_root, true, session_id);
        return true;
    }

    //
    // Check if the mount is still up an if the ImDisk device is the right one
    // 
    // WARNING: when we enter here and an image is already mounted, file_root
    //              will point to the ImDisk device and not the real path
    //

    if (!pRoot->InUse) {
        std::shared_ptr<BOX_MOUNT>& pMount = UseRamDisk ? m_RamDisk : pRoot->Mount;
        if (pMount && !pMount->NtPath.empty()) {
            std::wstring proxy = ImDiskQueryDeviceProxy(pMount->NtPath);
            if (_wcsnicmp(proxy.c_str(), L"\\BaseNamedObjects\\Global\\" IMBOX_PROXY, 25 + 11) != 0)
                pMount->NtPath.clear();
        }
    }

    //
    // Find or create a new mount when needed
    //

    std::wstring TargetNtPath;

    SCertInfo CertInfo = { 0 };
    if ((UseFileImage || UseRamDisk) && (!NT_SUCCESS(SbieApi_QueryDrvInfo(-1, &CertInfo, sizeof(CertInfo))) || !(CertInfo.active && (UseFileImage ? CertInfo.opt_enc : CertInfo.opt_sec)))) {
        const WCHAR* strings[] = { boxname, UseFileImage ? L"UseFileImage" : L"UseRamDisk" , NULL };
        SbieApi_LogMsgExt(session_id, UseFileImage ? 6009 : 6008, strings);
        errlvl = 0x66;
    } else

    if(!pRoot->Mount || pRoot->Mount->NtPath.empty()) {
//        SbieApi_LogEx(session_id, 2201, L"AcquireBoxRoot %S", boxname);
        
        if (UseRamDisk) {

            if(!m_RamDisk || m_RamDisk->NtPath.empty())
                m_RamDisk = FindImDisk(L"", session_id);
            if (!m_RamDisk) {
                ULONG sizeKb = SbieApi_QueryConfNumber(NULL, L"RamDiskSizeKb", 0);
				if (sizeKb < 100*1024) // we want at lesat 100MB
					SbieApi_LogEx(session_id, 2238, L"");
                else {
                    WCHAR drvLetter[32] = { 0 };
                    SbieApi_QueryConf(NULL, L"RamDiskLetter", 0, drvLetter, ARRAYSIZE(drvLetter));
                    m_RamDisk = MountImDisk(L"", NULL, sizeKb, session_id, *drvLetter ? drvLetter : NULL);
                }
            }
            pRoot->Mount = m_RamDisk;
        }
        else if (UseFileImage) {

            std::wstring ImageFile = GetImageFileName(file_root);

            //WCHAR Password[128 + 1];
            //SbieApi_QueryConfAsIs(boxname, L"BoxPassword", 0, Password, sizeof(Password)); // todo
            pRoot->Mount = FindImDisk(ImageFile, session_id);
            if (!pRoot->Mount) {
                //ULONG sizeKb = SbieApi_QueryConfNumber(NULL, L"BoxImageSizeKb", 4194304);
                pRoot->Mount = MountImDisk(ImageFile, NULL, 0, session_id);
            }

            //pRoot->AutoUnmount = SbieApi_QueryConfBool(boxname, L"AutoUnmount", FALSE);
        }
            
        if (!pRoot->Mount || pRoot->Mount->NtPath.empty())
            errlvl = 0x11;
        else
            pRoot->Path = file_root;
        //    pRoot->Mount->RefCount++;
    }

    if (errlvl == 0 && !pRoot->InUse) {

        //
        // Append box name and try to create
        //

        TargetNtPath = pRoot->Mount->NtPath + L"\\";
        if (UseRamDisk) // ram disk is shared so individualize the folder names
            TargetNtPath += boxname;
        else
            TargetNtPath += SBIEDISK_LABEL;

        HANDLE handle = OpenOrCreateNtFolder(TargetNtPath.c_str());
        if (!handle)
            errlvl = 0x12;
        else
        {
            CloseHandle(handle);

            //
            // Setup box root folder junction
            //

            errlvl = CreateJunction(TargetNtPath, file_root, session_id);
        }
    }

    LeaveCriticalSection(&m_CritSec);

    if (errlvl) {

        ULONG err = GetLastError();
        if (err == ERROR_DIR_NOT_EMPTY)
            SbieApi_LogEx(session_id, 2239, L"%S", boxname);
        else
            SbieApi_LogEx(session_id, 2230, L"%S [%02X / %d]", boxname, errlvl, err);

        //
        // Clean up on error
        //

        ReleaseBoxRoot(reg_root, true, session_id);

        return false;
    }
    return true;
}


//---------------------------------------------------------------------------
// LockBoxRoot
//---------------------------------------------------------------------------


void MountManager::LockBoxRoot(const WCHAR* reg_root, ULONG session_id)
{
    EnterCriticalSection(&m_CritSec);

    auto I = m_RootMap.find(reg_root);
    if (I != m_RootMap.end()) {
//        SbieApi_LogEx(session_id, 2201, L"LockBoxRoot %S", reg_root);
        I->second->InUse = true;
    }
    
    LeaveCriticalSection(&m_CritSec);
}


//---------------------------------------------------------------------------
// ReleaseBoxRoot
//---------------------------------------------------------------------------


void MountManager::ReleaseBoxRoot(const WCHAR* reg_root, bool force, ULONG session_id)
{
    EnterCriticalSection(&m_CritSec);

    auto I = m_RootMap.find(reg_root);
    if (I != m_RootMap.end()) {
//        SbieApi_LogEx(session_id, 2201, L"ReleaseBoxRoot %S", reg_root);
        I->second->InUse = false;
		if (!I->second->Mount)
			m_RootMap.erase(I);
		else if (force || I->second->AutoUnmount) {
			RemoveJunction(I->second->Path.c_str(), session_id);
            if (I->second->Mount == m_RamDisk) {
				//I->second->Mount->RefCount--;
				m_RootMap.erase(I);
			}
			//else if (m_hCleanUpThread == INVALID_HANDLE_VALUE)
			//	m_hCleanUpThread = CreateThread(NULL, 0, CleanUp, this, 0, NULL);
            else {

                if (I->second->Mount->Protected)
                    SbieApi_Call(API_UNPROTECT_ROOT, 1, I->first.c_str());

                UnmountImDiskLocked(I->second->Mount, session_id);
            }
		}
    }
    
    LeaveCriticalSection(&m_CritSec);
}


//---------------------------------------------------------------------------
// CleanUp
//---------------------------------------------------------------------------


//DWORD WINAPI MountManager::CleanUp(LPVOID lpThreadParameter)
//{
//    MountManager* This = (MountManager*)lpThreadParameter;
//
//    EnterCriticalSection(&This->m_CritSec);
//    for (;;) {
//        
//        ULONG session_id = 0;
//        std::shared_ptr<BOX_MOUNT> pToUnMount;
//
//        for (auto I = This->m_RootMap.begin(); I != This->m_RootMap.end(); ) {
//            if (!I->second->InUse && I->second->AutoUnmount) {
//                if (I->second->Mount->RefCount > 1) {
//                    I->second->Mount->RefCount--;
//                    I = This->m_RootMap.erase(I);
//                    continue;
//                }
//                else if (I->second->Mount->RefCount == 1) { // || I->second->Mount->RefCount == -1
//                    pToUnMount = I->second->Mount;
//                    session_id = I->second->session_id;
//                    break;
//                }
//            }
//            ++I;
//        }
//
//        if (!pToUnMount)
//            break; // nothing to do
//
//        std::wstring Device = pToUnMount->NtPath;
//        HANDLE hProcess = pToUnMount->ProcessHandle;
//        pToUnMount->RefCount = -1; // 1 -> -1
//
//        LeaveCriticalSection(&This->m_CritSec);
//    
//        bool ok = UnMountImDisk(Device, hProcess);
//
//        if(!ok)
//            SbieApi_LogEx(session_id, 2237, L"%S", Device.c_str());
//
//        EnterCriticalSection(&This->m_CritSec);
//
//        if (ok) {
//            for (auto I = This->m_RootMap.begin(); I != This->m_RootMap.end(); ++I) {
//                if (I->second->Mount == pToUnMount) {
//                    if(I->second->Mount->Protected)
//                        SbieApi_Call(API_UNPROTECT_ROOT, 1, I->first.c_str());
//                    This->m_RootMap.erase(I);
//                    break;
//                }
//            }
//        }
//    }
//
//    CloseHandle(This->m_hCleanUpThread);
//    This->m_hCleanUpThread = INVALID_HANDLE_VALUE;
//    LeaveCriticalSection(&This->m_CritSec);
//
//    return 0;
//}


//---------------------------------------------------------------------------
// TryUnMountImDisk
//---------------------------------------------------------------------------


bool MountManager::TryUnmountImDisk(const std::wstring& Device, HANDLE hProcess, int iMode)
{
    bool ok = false;

    if (Device.size() <= IMDISK_DEVICE_LEN)
        return false; // not an imdisk path
    std::wstring cmd;
    switch (iMode)
    {
    case 0: cmd = L"imdisk -d -u "; break;  // graceful
    case 1: cmd = L"imdisk -D -u "; break;  // forced
    case 2: cmd = L"imdisk -R -u "; break;  // emergency
    }
    cmd += Device.substr(IMDISK_DEVICE_LEN);

    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi = { 0 };
    if (CreateProcess(NULL, (WCHAR*)cmd.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        if (WaitForSingleObject(pi.hProcess, 10 * 1000) == WAIT_OBJECT_0) {
            DWORD ret = 0;
            GetExitCodeProcess(pi.hProcess, &ret);
            ok = (ret == 0 || ret == 1); // 0 - ok // 1 - device not found (already unmounted?)
        }
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);

        if (ok && hProcess) {
            WaitForSingleObject(hProcess, 10 * 1000);
            CloseHandle(hProcess);
        }
    }

    return ok;
}


//---------------------------------------------------------------------------
// UnMountImDisk
//---------------------------------------------------------------------------


bool MountManager::UnmountImDisk(const std::wstring& Device, HANDLE hProcess) 
{
    for (int i = 0; i < 7; i++) { // 5 attempt normal and 2 forced
        if (TryUnmountImDisk(Device, hProcess, i > 4 ? 1 : 0))
            return true;
        Sleep(1000);
    }
    // last emergency attempt
    TryUnmountImDisk(Device, hProcess, 2);
    return false; // report an error when emergency unmoutn was needed
}


//---------------------------------------------------------------------------
// UnmountImDiskLocked
//---------------------------------------------------------------------------


bool MountManager::UnmountImDiskLocked(const std::shared_ptr<BOX_MOUNT>& pToUnMount, ULONG session_id)
{
    std::wstring Device = pToUnMount->NtPath;
    HANDLE hProcess = pToUnMount->ProcessHandle;
    pToUnMount->Unmounting = true;
    //pToUnMount->RefCount = -1; // 1 -> -1

    LeaveCriticalSection(&m_CritSec);
    
    bool ok = UnmountImDisk(Device, hProcess);

    if(!ok)
        SbieApi_LogEx(session_id, 2237, L"%S", Device.c_str());

    EnterCriticalSection(&m_CritSec);

    for (auto I = m_RootMap.begin(); I != m_RootMap.end(); ++I) {
        if (I->second->Mount == pToUnMount) {
            if (ok)
                m_RootMap.erase(I);
            else
                pToUnMount->Unmounting = false;
            break;
        }
    }

    return ok;
}


//---------------------------------------------------------------------------
// UnmountAll
//---------------------------------------------------------------------------


void MountManager::UnmountAll()
{
    EnterCriticalSection(&m_CritSec);

    for (auto I = m_RootMap.begin(); I != m_RootMap.end(); I++) {
        RemoveJunction(I->second->Path.c_str(), -1);
        if (I->second->Mount && I->second->Mount != m_RamDisk)
            UnmountImDisk(I->second->Mount->NtPath, I->second->Mount->ProcessHandle);
    }
    if(m_RamDisk)
        UnmountImDisk(m_RamDisk->NtPath, m_RamDisk->ProcessHandle);

    LeaveCriticalSection(&m_CritSec);
}

