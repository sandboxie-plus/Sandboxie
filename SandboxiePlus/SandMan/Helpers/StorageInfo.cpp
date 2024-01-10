#include "stdafx.h"

#include "StorageInfo.h"

#include <windows.h>

#include <winternl.h>
#include <winioctl.h>

typedef double DOUBLE;
typedef GUID* PGUID;

#include <stdio.h>
#include <cfgmgr32.h>
#include <devguid.h>
#include <initguid.h>
#include <devpkey.h>

#pragma comment(lib, "cfgmgr32.lib")



extern "C" {
typedef struct _RTLP_CURDIR_REF
{
    LONG ReferenceCount;
    HANDLE DirectoryHandle;
} RTLP_CURDIR_REF, * PRTLP_CURDIR_REF;

typedef struct _RTL_RELATIVE_NAME_U
{
    UNICODE_STRING RelativeName;
    HANDLE ContainingDirectory;
    PRTLP_CURDIR_REF CurDirRef;
} RTL_RELATIVE_NAME_U, * PRTL_RELATIVE_NAME_U;


NTSYSAPI
NTSTATUS
NTAPI
RtlDosPathNameToNtPathName_U_WithStatus(
    PCWSTR                  DosFileName,
    PUNICODE_STRING         NtFileName,
    PWSTR*                  FilePart,
    PRTL_RELATIVE_NAME_U    RelativeName
);

NTSTATUS WINAPI NtQuerySymbolicLinkObject(
    HANDLE          LinkHandle,
    PUNICODE_STRING LinkTarget,
    PULONG          ReturnedLength
);

NTSTATUS WINAPI NtOpenSymbolicLinkObject(
    PHANDLE            LinkHandle,
    ACCESS_MASK        DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes
);

typedef enum _FSINFOCLASS {
    FileFsVolumeInformation       = 1,
    FileFsLabelInformation,      // 2
    FileFsSizeInformation,       // 3
    FileFsDeviceInformation,     // 4
    FileFsAttributeInformation,  // 5
    FileFsControlInformation,    // 6
    FileFsFullSizeInformation,   // 7
    FileFsObjectIdInformation,   // 8
    FileFsDriverPathInformation, // 9
    FileFsMaximumInformation
} FS_INFORMATION_CLASS, *PFS_INFORMATION_CLASS;

__declspec(dllimport) NTSTATUS __stdcall
NtQueryVolumeInformationFile(
    IN  HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FsInformation,
    IN  ULONG Length,
    IN  ULONG FsInformationClass
);

typedef struct _FILE_FS_VOLUME_INFORMATION {
  LARGE_INTEGER VolumeCreationTime;
  ULONG         VolumeSerialNumber;
  ULONG         VolumeLabelLength;
  BOOLEAN       SupportsObjects;
  WCHAR         VolumeLabel[1];
} FILE_FS_VOLUME_INFORMATION, *PFILE_FS_VOLUME_INFORMATION;
}

#define SYMBOLIC_LINK_QUERY 0x0001

std::wstring QueryLinkTarget(LPCWSTR linkName)
{
    std::wstring        target;

    UNICODE_STRING ObjectName;
    if (NT_SUCCESS(RtlDosPathNameToNtPathName_U_WithStatus(linkName, &ObjectName, NULL, NULL)))
    {
        OBJECT_ATTRIBUTES   Obja;
        InitializeObjectAttributes(&Obja, &ObjectName, OBJ_CASE_INSENSITIVE, NULL, NULL);

        HANDLE hLink;
        if (NT_SUCCESS(NtOpenSymbolicLinkObject(&hLink, SYMBOLIC_LINK_QUERY, &Obja))) 
        {
            UNICODE_STRING		InfoString;

            target.resize(0x1000);
            InfoString.Buffer = (WCHAR*)target.c_str();
            InfoString.Length = (USHORT)(target.size()-1) * sizeof(WCHAR);
            InfoString.MaximumLength = InfoString.Length + sizeof(UNICODE_NULL);

            if (NT_SUCCESS(NtQuerySymbolicLinkObject(hLink, &InfoString, NULL)))
                InfoString.Buffer[InfoString.Length / sizeof(WCHAR)] = 0;
            else
                InfoString.Buffer[0] = 0;

            NtClose(hLink);
        }

        RtlFreeUnicodeString(&ObjectName);
    }

	return target.c_str(); // truncate
}

//#define DSK_BASIC       0
//#define DSK_DYN_SIMPLE  1
//#define DSK_DYN_SPANNED 2

bool GetVolumeInfo(wchar_t* w32_name, SVolumeInfo* info)
{
    bool                    bRet = true;
    HANDLE                  hVolume;
    STORAGE_DEVICE_NUMBER   dnum;
    unsigned char           buff[4096];
    PVOLUME_DISK_EXTENTS    ext = (PVOLUME_DISK_EXTENTS)buff;
    DWORD                   dwBytes;

    hVolume = CreateFile(w32_name, SYNCHRONIZE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hVolume == INVALID_HANDLE_VALUE)
        return false;

    if (DeviceIoControl(hVolume, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &dnum, sizeof(dnum), &dwBytes, NULL)) {
        
        SVolumeInfo::SDisk disk;
        if(dnum.PartitionNumber == 0)
            disk.deviceName = L"\\Device\\Floppy" + std::to_wstring(dnum.DeviceNumber); 
        else if(dnum.PartitionNumber == -1)
            disk.deviceName = L"\\Device\\CdRom" + std::to_wstring(dnum.DeviceNumber); 
        else
            disk.deviceName = L"\\Device\\Harddisk" + std::to_wstring(dnum.DeviceNumber);
        disk.dskNumber = dnum.DeviceNumber;
        disk.dskType = dnum.DeviceType;
        //disk.prtStart = ext->Extents[i].StartingOffset.QuadPart;
        //disk.prtSize = ext->Extents[i].ExtentLength.QuadPart;
        info->disks.push_back(disk);

        //info->parNumb = dnum.PartitionNumber;

        //info->dskType = DSK_BASIC;
    }
    else if (DeviceIoControl(hVolume, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, 0, ext, sizeof(buff), &dwBytes, NULL))
    {
        for (int i = 0; i < ext->NumberOfDiskExtents; i++) {
            SVolumeInfo::SDisk disk;
            disk.deviceName = L"\\Device\\Harddisk" + std::to_wstring(ext->Extents[i].DiskNumber);
            disk.dskNumber = ext->Extents[i].DiskNumber;
            disk.dskType = FILE_DEVICE_DISK;
            //disk.prtStart = ext->Extents[i].StartingOffset.QuadPart;
            //disk.prtSize = ext->Extents[i].ExtentLength.QuadPart;
            info->disks.push_back(disk);
        }

        //info->dskType = ext->NumberOfDiskExtents == 1 ? DSK_DYN_SIMPLE : DSK_DYN_SPANNED;
    }
    else
        bRet = false;

    CloseHandle(hVolume);
    return bRet;
}

std::list<SVolumeInfo> ListAllVolumes()
{
	std::list<SVolumeInfo> volumes;

    wchar_t volumeName[MAX_PATH + 1];
    HANDLE hFindVolume = FindFirstVolume(volumeName, MAX_PATH + 1);
    if (hFindVolume == INVALID_HANDLE_VALUE)
        return volumes;

    do
    {
        SVolumeInfo info;
        info.volumeName = volumeName;

        DWORD dwRetLen = 0;
        wchar_t mountPoints[0x1000];
        //QueryDosDevice(&volumeName[4], driveLetter, MAX_PATH + 1);
        if (GetVolumePathNamesForVolumeName(volumeName, mountPoints, ARRAYSIZE(mountPoints), &dwRetLen)) {
            for (wchar_t* mountPoint = mountPoints; *mountPoint; mountPoint += wcslen(mountPoint) + 1)
                info.mountPoints.push_back(mountPoint);
        }

        volumeName[wcslen(volumeName) - 1] = 0; // strip trailing L'\\'

        info.deviceName = QueryLinkTarget(volumeName);

        if (!GetVolumeInfo(volumeName, &info))
            continue;

        volumes.push_back(info);

    } while (FindNextVolume(hFindVolume, volumeName, MAX_PATH + 1));

    FindVolumeClose(hFindVolume);

	return volumes;
}

std::wstring QueryDiskDeviceInterfaceString(PWSTR DeviceInterface, CONST DEVPROPKEY *PropertyKey)
{
    CONFIGRET result;
    ULONG bufferSize;
    DEVPROPTYPE devicePropertyType;
    DEVINST deviceInstanceHandle;
    ULONG deviceInstanceIdLength = MAX_DEVICE_ID_LEN;
    WCHAR deviceInstanceId[MAX_DEVICE_ID_LEN + 1] = L"";

    if (CM_Get_Device_Interface_Property(DeviceInterface, &DEVPKEY_Device_InstanceId, &devicePropertyType, (PBYTE)deviceInstanceId, &deviceInstanceIdLength, 0 ) != CR_SUCCESS)
        return L"";

    if (CM_Locate_DevNode(&deviceInstanceHandle, deviceInstanceId, CM_LOCATE_DEVNODE_PHANTOM ) != CR_SUCCESS)
        return L"";

    bufferSize = 0x40;
    std::wstring deviceDescription;
    deviceDescription.resize(bufferSize);

    if ((result = CM_Get_DevNode_Property(deviceInstanceHandle, PropertyKey, &devicePropertyType, (PBYTE)deviceDescription.c_str(), &bufferSize, 0 )) != CR_SUCCESS) {
        deviceDescription.resize(bufferSize);
        result = CM_Get_DevNode_Property(deviceInstanceHandle, PropertyKey, &devicePropertyType, (PBYTE)deviceDescription.c_str(), &bufferSize, 0 );
    }

    return deviceDescription.c_str(); // truncate
}

std::map<std::wstring, SDriveInfo> ListAllDrives()
{
    std::map<std::wstring, SDriveInfo> drives;

    struct SDevice {
        LPGUID guid;
        const wchar_t* prefix;
    };

    SDevice devices[] = { 
        (PGUID)&GUID_DEVINTERFACE_DISK, L"\\Device\\Harddisk", 
        (PGUID)&GUID_DEVINTERFACE_CDROM, L"\\Device\\CdRom", 
        (PGUID)&GUID_DEVINTERFACE_FLOPPY, L"\\Device\\Floppy", 
        NULL, NULL };

    for (SDevice* pdevice = devices; pdevice->guid != NULL; pdevice++)
    {
        PWSTR deviceInterfaceList;
        ULONG deviceInterfaceListLength = 0;
        PWSTR deviceInterface;

        if (CM_Get_Device_Interface_List_Size(&deviceInterfaceListLength, pdevice->guid, NULL, CM_GET_DEVICE_INTERFACE_LIST_PRESENT) != CR_SUCCESS) {
            return drives;
        }

        deviceInterfaceList = (PWSTR)malloc(deviceInterfaceListLength * sizeof(WCHAR));
        memset(deviceInterfaceList, 0, deviceInterfaceListLength * sizeof(WCHAR));
        if (CM_Get_Device_Interface_List(pdevice->guid, NULL, deviceInterfaceList, deviceInterfaceListLength, CM_GET_DEVICE_INTERFACE_LIST_PRESENT) != CR_SUCCESS) {
            free(deviceInterfaceList);
            return drives;
        }

        for (deviceInterface = deviceInterfaceList; *deviceInterface; deviceInterface += wcslen(deviceInterface) + 1)
        {
            
            HANDLE deviceHandle = CreateFile(deviceInterface, FILE_READ_ATTRIBUTES | SYNCHRONIZE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
            if (deviceHandle != INVALID_HANDLE_VALUE)
            {
                STORAGE_DEVICE_NUMBER result;
                memset(&result, 0, sizeof(STORAGE_DEVICE_NUMBER));
                DWORD dwRet;
                if (DeviceIoControl(deviceHandle, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &result, sizeof(result), &dwRet, NULL))
                {
                    std::wstring name = QueryDiskDeviceInterfaceString(deviceInterface, &DEVPKEY_Device_FriendlyName);
                    if (!name.empty()) {
                        std::wstring enumerator = QueryDiskDeviceInterfaceString(deviceInterface, &DEVPKEY_Device_EnumeratorName);

                        drives[pdevice->prefix + std::to_wstring(result.DeviceNumber)] = SDriveInfo{ name , deviceInterface , enumerator };
                    }
                }
                // else // not connected

                NtClose(deviceHandle);
            }
        }
    }

    return drives;
}