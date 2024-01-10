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
// Mount Manager Helpers
//---------------------------------------------------------------------------

#include "stdafx.h"
#include "common/win32_ntddk.h"
#include <winioctl.h>

#include <string>
#include <map>
#include <list>
#include <vector>
#include <memory>

std::wstring GetVolumeLabel(const std::wstring &NtPath)
{
    std::wstring Label;

    HANDLE handle;
    IO_STATUS_BLOCK iosb;

    UNICODE_STRING objname;
    RtlInitUnicodeString(&objname, NtPath.c_str());

    OBJECT_ATTRIBUTES objattrs;
    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);
    
    ULONG OldMode;
    RtlSetThreadErrorMode(0x10u, &OldMode);
    NTSTATUS status = NtCreateFile(
        &handle, GENERIC_READ | SYNCHRONIZE, &objattrs,
        &iosb, NULL, 0, FILE_SHARE_VALID_FLAGS,
        FILE_OPEN,
        FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
        NULL, 0);
    RtlSetThreadErrorMode(OldMode, 0i64);

    if (NT_SUCCESS(status))
    {
        union {
            FILE_FS_VOLUME_INFORMATION volumeInfo;
            BYTE volumeInfoBuff[64];
        } u;
        if (NT_SUCCESS(NtQueryVolumeInformationFile(handle, &iosb, &u.volumeInfo, sizeof(u), FileFsVolumeInformation)))
            Label = std::wstring(u.volumeInfo.VolumeLabel, u.volumeInfo.VolumeLabelLength / sizeof(WCHAR));

        NtClose(handle);
    }

    return Label;
}

extern "C" {


// ImDisk

// Base names for device objects created in \Device
#define IMDISK_DEVICE_DIR_NAME              L"\\Device"
#define IMDISK_DEVICE_BASE_NAME             IMDISK_DEVICE_DIR_NAME  L"\\ImDisk"
#define IMDISK_CTL_DEVICE_NAME              IMDISK_DEVICE_BASE_NAME L"Ctl"

#define IMDISK_DRIVER_VERSION               0x0103

// Base value for the IOCTL's.
#define FILE_DEVICE_IMDISK		            0x8372

#define IOCTL_IMDISK_QUERY_VERSION	        ((ULONG) CTL_CODE(FILE_DEVICE_IMDISK, 0x800, METHOD_BUFFERED, 0))
#define IOCTL_IMDISK_CREATE_DEVICE	        ((ULONG) CTL_CODE(FILE_DEVICE_IMDISK, 0x801, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS))
#define IOCTL_IMDISK_QUERY_DEVICE	        ((ULONG) CTL_CODE(FILE_DEVICE_IMDISK, 0x802, METHOD_BUFFERED, 0))
#define IOCTL_IMDISK_QUERY_DRIVER           ((ULONG) CTL_CODE(FILE_DEVICE_IMDISK, 0x803, METHOD_BUFFERED, 0))
#define IOCTL_IMDISK_REFERENCE_HANDLE       ((ULONG) CTL_CODE(FILE_DEVICE_IMDISK, 0x804, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS))
#define IOCTL_IMDISK_SET_DEVICE_FLAGS       ((ULONG) CTL_CODE(FILE_DEVICE_IMDISK, 0x805, METHOD_BUFFERED, 0))
#define IOCTL_IMDISK_REMOVE_DEVICE          ((ULONG) CTL_CODE(FILE_DEVICE_IMDISK, 0x806, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS))
#define IOCTL_IMDISK_IOCTL_PASS_THROUGH	    ((ULONG) CTL_CODE(FILE_DEVICE_IMDISK, 0x807, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS))
#define IOCTL_IMDISK_FSCTL_PASS_THROUGH	    ((ULONG) CTL_CODE(FILE_DEVICE_IMDISK, 0x808, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS))
#define IOCTL_IMDISK_GET_REFERENCED_HANDLE  ((ULONG) CTL_CODE(FILE_DEVICE_IMDISK, 0x809, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS))


typedef struct _IMDISK_CREATE_DATA
{
    /// On create this can be set to IMDISK_AUTO_DEVICE_NUMBER
    ULONG           DeviceNumber;
    /// Total size in bytes (in the Cylinders field) and virtual geometry.
    DISK_GEOMETRY   DiskGeometry;
    /// The byte offset in the image file where the virtual disk begins.
    LARGE_INTEGER   ImageOffset;
    /// Creation flags. Type of device and type of connection.
    ULONG           Flags;
    /// Drive letter (if used, otherwise zero).
    WCHAR           DriveLetter;
    /// Length in bytes of the FileName member.
    USHORT          FileNameLength;
    /// Dynamically-sized member that specifies the image file name.
    WCHAR           FileName[1];
} IMDISK_CREATE_DATA, *PIMDISK_CREATE_DATA;

/// Virtual disk is backed by image file
#define IMDISK_TYPE_FILE                0x00000100
/// Virtual disk is backed by virtual memory
#define IMDISK_TYPE_VM                  0x00000200
/// Virtual disk is backed by proxy connection
#define IMDISK_TYPE_PROXY               0x00000300

/// Extracts the IMDISK_TYPE_xxx from flags
#define IMDISK_TYPE(x)                  ((ULONG)(x) & 0x00000F00)


HANDLE WINAPI ImDiskOpenDeviceByName(PUNICODE_STRING FileName, DWORD AccessMode)
{
    NTSTATUS status;
    HANDLE handle;
    OBJECT_ATTRIBUTES object_attrib;
    IO_STATUS_BLOCK io_status;

    InitializeObjectAttributes(&object_attrib,
        FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    status = NtOpenFile(&handle,
        SYNCHRONIZE | AccessMode,
        &object_attrib,
        &io_status,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);

    if (!NT_SUCCESS(status))
    {
        SetLastError(RtlNtStatusToDosError(status));
        return INVALID_HANDLE_VALUE;
    }

    return handle;
}

//HANDLE WINAPI ImDiskOpenDeviceByNumber(DWORD DeviceNumber, DWORD AccessMode)
//{
//    WCHAR device_path[_countof(IMDISK_DEVICE_BASE_NAME) + 16];
//
//    UNICODE_STRING file_name;
//
//    // Build device path, e.g. \Device\ImDisk2
//    _snwprintf_s(device_path, ARRAYSIZE(device_path), _countof(device_path),
//        IMDISK_DEVICE_BASE_NAME L"%u", DeviceNumber);
//    device_path[_countof(device_path) - 1] = 0;
//
//    RtlInitUnicodeString(&file_name, device_path);
//
//    return ImDiskOpenDeviceByName(&file_name, AccessMode);
//}

HANDLE WINAPI ImDiskOpenDeviceByMountPoint(LPCWSTR MountPoint, DWORD AccessMode)
{
    UNICODE_STRING DeviceName;
    WCHAR DriveLetterPath[] = L"\\DosDevices\\ :";
    PREPARSE_DATA_BUFFER ReparseData = NULL;
    HANDLE h;

    if ((MountPoint[0] != 0) &&
        ((wcscmp(MountPoint + 1, L":") == 0) ||
        (wcscmp(MountPoint + 1, L":\\") == 0)))
    {
        DriveLetterPath[12] = MountPoint[0];

        RtlInitUnicodeString(&DeviceName, DriveLetterPath);
    }
    else if (((wcsncmp(MountPoint, L"\\\\?\\", 4) == 0) ||
        (wcsncmp(MountPoint, L"\\\\.\\", 4) == 0)) &&
        (wcschr(MountPoint + 4, L'\\') == NULL))
    {
        return CreateFile(MountPoint, AccessMode,
            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    }
    else
    {
        HANDLE hDir;
        DWORD dw;
        DWORD buffer_size =
            FIELD_OFFSET(REPARSE_DATA_BUFFER, MountPointReparseBuffer) +
            MAXIMUM_REPARSE_DATA_BUFFER_SIZE;

        hDir = CreateFile(MountPoint, GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS |
            FILE_FLAG_OPEN_REPARSE_POINT, NULL);

        if (hDir == INVALID_HANDLE_VALUE)
            return INVALID_HANDLE_VALUE;

        ReparseData = (PREPARSE_DATA_BUFFER)HeapAlloc(GetProcessHeap(),
            HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY,
            buffer_size);

        if (!DeviceIoControl(hDir, FSCTL_GET_REPARSE_POINT,
            NULL, 0,
            ReparseData, buffer_size,
            &dw, NULL))
        {
            DWORD last_error = GetLastError();
            CloseHandle(hDir);
            HeapFree(GetProcessHeap(), 0, ReparseData);
            SetLastError(last_error);
            return INVALID_HANDLE_VALUE;
        }

        CloseHandle(hDir);

        if (ReparseData->ReparseTag != IO_REPARSE_TAG_MOUNT_POINT)
        {
            HeapFree(GetProcessHeap(), 0, ReparseData);
            SetLastError(ERROR_NOT_A_REPARSE_POINT);
            return INVALID_HANDLE_VALUE;
        }

        DeviceName.Length =
            ReparseData->MountPointReparseBuffer.SubstituteNameLength;
        
        DeviceName.Buffer = (PWSTR)
            ((PUCHAR)ReparseData->MountPointReparseBuffer.PathBuffer +
            ReparseData->MountPointReparseBuffer.SubstituteNameOffset);
        
        DeviceName.MaximumLength = DeviceName.Length;
    }
    
    if (DeviceName.Buffer[(DeviceName.Length >> 1) - 1] == L'\\')
    {
        DeviceName.Buffer[(DeviceName.Length >> 1) - 1] = 0;
        DeviceName.Length -= 2;
    }

    h = ImDiskOpenDeviceByName(&DeviceName, AccessMode);

    if (ReparseData != NULL)
        HeapFree(GetProcessHeap(), 0, ReparseData);

    return h;
}

BOOL WINAPI IsImDiskDriverReady()
{
    BOOL bRet = FALSE;
    
    UNICODE_STRING objname;
    RtlInitUnicodeString(&objname, IMDISK_CTL_DEVICE_NAME);

    HANDLE Device = ImDiskOpenDeviceByName(&objname, GENERIC_READ | GENERIC_WRITE);

    if (Device != INVALID_HANDLE_VALUE)
    {
        DWORD VersionCheck;
        DWORD BytesReturned;
        if (DeviceIoControl(Device, IOCTL_IMDISK_QUERY_VERSION, NULL, 0, &VersionCheck, sizeof VersionCheck, &BytesReturned, NULL)) 
        {
            if (BytesReturned >= sizeof(VersionCheck))
                bRet = (VersionCheck == IMDISK_DRIVER_VERSION);
        }

        CloseHandle(Device);
    }

    return bRet;
}


BOOL WINAPI ImDiskGetDeviceListEx(IN ULONG ListLength, OUT ULONG *DeviceList)
{
    UNICODE_STRING file_name;
    HANDLE driver;
    ULONG dw;

    RtlInitUnicodeString(&file_name, IMDISK_CTL_DEVICE_NAME);

    driver = ImDiskOpenDeviceByName(&file_name, GENERIC_READ);
    if (driver == INVALID_HANDLE_VALUE)
        return 0;

    if (!DeviceIoControl(driver,
        IOCTL_IMDISK_QUERY_DRIVER,
        NULL, 0,
        DeviceList, ListLength << 2,
        &dw, NULL))
    {
        DWORD dwLastError = GetLastError();
        NtClose(driver);
        SetLastError(dwLastError);
        return FALSE;
    }

    NtClose(driver);

    if ((dw == sizeof(ULONG)) &
        (*DeviceList > 0))
    {
        SetLastError(ERROR_MORE_DATA);
        return FALSE;
    }

    SetLastError(NO_ERROR);
    return TRUE;
}


WCHAR WINAPI ImDiskFindFreeDriveLetter()
{
    DWORD logical_drives = GetLogicalDrives();
    WCHAR search;

    for (search = L'Z'; search >= L'I'; search--)
    {
        if ((logical_drives & (1 << (search - L'A'))) == 0)
        {
            return search;
        }
    }

    return 0;
}

}

std::wstring ImDiskQueryDeviceProxy(const std::wstring& FileName)
{
    std::wstring proxy;

    UNICODE_STRING file_name;
    RtlInitUnicodeString(&file_name, (WCHAR*)FileName.c_str());
    HANDLE device = ImDiskOpenDeviceByName(&file_name, FILE_READ_ATTRIBUTES);
    if (device != INVALID_HANDLE_VALUE)
    {
        union {
            IMDISK_CREATE_DATA create_data;
            BYTE buffer[sizeof(IMDISK_CREATE_DATA) + MAX_PATH * 4];
        }u;

        DWORD dw;
        if (DeviceIoControl(device, IOCTL_IMDISK_QUERY_DEVICE, NULL, 0, &u.create_data, sizeof(u.buffer), &dw, NULL))
        {
            if (IMDISK_TYPE(u.create_data.Flags) == IMDISK_TYPE_PROXY)
                proxy = std::wstring(u.create_data.FileName, u.create_data.FileNameLength / sizeof(wchar_t));
        }

        NtClose(device);
    }

    return proxy;
}

ULONGLONG ImDiskQueryDeviceSize(const std::wstring& FileName)
{
    ULONGLONG size;

    UNICODE_STRING file_name;
    RtlInitUnicodeString(&file_name, (WCHAR*)FileName.c_str());
    HANDLE device = ImDiskOpenDeviceByName(&file_name, FILE_READ_ATTRIBUTES);
    if (device != INVALID_HANDLE_VALUE)
    {
        union {
            IMDISK_CREATE_DATA create_data;
            BYTE buffer[sizeof(IMDISK_CREATE_DATA) + MAX_PATH * 4];
        }u;

        DWORD dw;
        if (DeviceIoControl(device, IOCTL_IMDISK_QUERY_DEVICE, NULL, 0, &u.create_data, sizeof(u.buffer), &dw, NULL))
        {
            if (IMDISK_TYPE(u.create_data.Flags) == IMDISK_TYPE_PROXY)
                size = u.create_data.DiskGeometry.Cylinders.QuadPart;
        }

        NtClose(device);
    }

    return size;
}