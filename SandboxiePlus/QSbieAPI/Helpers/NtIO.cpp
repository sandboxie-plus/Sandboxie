#include "stdafx.h"

#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

#include <windows.h>
#include "..\..\..\Sandboxie\common\win32_ntddk.h"

#include "NtIO.h"

bool NtIo_WaitForFolder(POBJECT_ATTRIBUTES objattrs, int seconds, bool (*cb)(const WCHAR* info, void* param), void* param)
{
	NTSTATUS status = STATUS_SUCCESS;
	IO_STATUS_BLOCK IoStatusBlock;

	for (int retries = 0; retries < seconds * 2; retries++)
	{
		if (cb && !cb(objattrs->ObjectName->Buffer, param))
			return false;

		HANDLE handle = NULL;
		status = NtCreateFile(&handle, DELETE | SYNCHRONIZE, objattrs, &IoStatusBlock, NULL, FILE_ATTRIBUTE_NORMAL,
			0, FILE_OPEN, FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
		if (NT_SUCCESS(status)) {
			NtClose(handle);
			return true;
		}

		Sleep(500);
	}

	return false;
}

NTSTATUS NtIo_RemoveProblematicAttributes(POBJECT_ATTRIBUTES objattrs)
{
	IO_STATUS_BLOCK IoStatusBlock;
	NTSTATUS status;

	HANDLE handle = NULL;
	status = NtCreateFile(&handle, FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
		objattrs, &IoStatusBlock, NULL, 0, 0, FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	if (NT_SUCCESS(status))
	{
		FILE_BASIC_INFORMATION info;

		status = NtQueryInformationFile(handle, &IoStatusBlock, &info, sizeof(info), FileBasicInformation);
		if (NT_SUCCESS(status))
		{
			info.FileAttributes &= ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
			if (info.FileAttributes == 0 || info.FileAttributes == FILE_ATTRIBUTE_DIRECTORY)
				info.FileAttributes |= FILE_ATTRIBUTE_NORMAL;

			status = NtSetInformationFile(handle, &IoStatusBlock, &info, sizeof(info), FileBasicInformation);
		}

		NtClose(handle);
	}

	return status;
}

typedef struct _REPARSE_DATA_MOUNT_POINT
{
	DWORD ReparseTag;
	WORD ReparseDataLength;
	WORD Reserved;
	WORD NameOffset;
	WORD NameLength;
	WORD DisplayNameOffset;
	WORD DisplayNameLength;
	BYTE Data[65536];
} REPARSE_DATA_MOUNT_POINT, *PREPARSE_DATA_MOUNT_POINT;

NTSTATUS NtIo_RemoveJunction(POBJECT_ATTRIBUTES objattrs)
{
	NTSTATUS status = STATUS_SUCCESS;
	IO_STATUS_BLOCK Iosb;

	HANDLE Handle;
	status = NtCreateFile(&Handle, GENERIC_WRITE | DELETE, objattrs, &Iosb, 0, 0, FILE_SHARE_READ, FILE_OPEN, FILE_FLAG_OPEN_REPARSE_POINT, 0, 0); // 0x40100080, , , , , 0x00204020
	if (NT_SUCCESS(status))
	{
		REPARSE_DATA_MOUNT_POINT ReparseBuffer = { 0 };
		status = NtFsControlFile(Handle, NULL, NULL, NULL, &Iosb, FSCTL_GET_REPARSE_POINT, NULL, 0, &ReparseBuffer, sizeof(ReparseBuffer));
		if (NT_SUCCESS(status))
		{
			REPARSE_GUID_DATA_BUFFER ReparseData = { 0 };
			ReparseData.ReparseTag = ReparseBuffer.ReparseTag;
			ReparseData.ReparseDataLength = 0;
			status = NtFsControlFile(Handle, NULL, NULL, NULL, &Iosb, FSCTL_DELETE_REPARSE_POINT, &ReparseData, REPARSE_GUID_DATA_BUFFER_HEADER_SIZE, NULL, 0);
		}

		NtClose(Handle);
	}

	return status;
}

NTSTATUS NtIo_DeleteFolderRecursivelyImpl(POBJECT_ATTRIBUTES objattrs, bool (*cb)(const WCHAR* info, void* param), void* param);

NTSTATUS NtIo_DeleteFile(ULONG FileAttributes, OBJECT_ATTRIBUTES* attr, bool (*cb)(const WCHAR* info, void* param), void* param)
{
	NTSTATUS status = STATUS_SUCCESS;

	if (FileAttributes & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM))
		NtIo_RemoveProblematicAttributes(attr);

	if (FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
		status = NtIo_RemoveJunction(attr);
	else if (FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		status = NtIo_DeleteFolderRecursivelyImpl(attr, cb, param);
		
	if (NT_SUCCESS(status))
		status = NtDeleteFile(attr);

	if (status == STATUS_OBJECT_NAME_NOT_FOUND || status == STATUS_OBJECT_PATH_NOT_FOUND)
		status = STATUS_SUCCESS; // we wanted it gone and its not here, success

	return status;
}

NTSTATUS NtIo_DeleteFile(SNtObject& ntObject, bool (*cb)(const WCHAR* info, void* param), void* param)
{
	FILE_BASIC_INFORMATION info = { 0 };
	NtQueryAttributesFile(&ntObject.attr, &info);

	return NtIo_DeleteFile(info.FileAttributes, &ntObject.attr, cb, param);
}

NTSTATUS NtIo_DeleteFolderRecursivelyImpl(POBJECT_ATTRIBUTES objattrs, bool (*cb)(const WCHAR* info, void* param), void* param)
{
	NTSTATUS status = STATUS_SUCCESS;
	IO_STATUS_BLOCK Iosb;

	if (cb && !cb(objattrs->ObjectName->Buffer, param))
		return STATUS_CANCELLED;

	HANDLE Handle;
	status = NtCreateFile(&Handle, GENERIC_READ, objattrs, &Iosb,
		0, FILE_ATTRIBUTE_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, FILE_OPEN, FILE_DIRECTORY_FILE, 0, 0);
	if (!NT_SUCCESS(status))
		return status;

	for ( ; status == STATUS_SUCCESS; )
	{
		std::wstring FileName;
		ULONG FileAttributes;

		PFILE_BOTH_DIRECTORY_INFORMATION Info = (PFILE_BOTH_DIRECTORY_INFORMATION)malloc(PAGE_SIZE);

		HANDLE Event;
		NtCreateEvent(&Event, GENERIC_ALL, 0, NotificationEvent, FALSE);
		status = NtQueryDirectoryFile(Handle, Event, 0, 0, &Iosb, Info, PAGE_SIZE, FileBothDirectoryInformation, TRUE, NULL, FALSE);
		if (status == STATUS_PENDING){
			NtWaitForSingleObject(Event, TRUE, 0);
			status = Iosb.Status;
		}
		NtClose(Event);

		if (NT_SUCCESS(status))
		{
			FileName.assign(Info->FileName, Info->FileNameLength / sizeof(wchar_t));
			FileAttributes = Info->FileAttributes;
		}

		free(Info);

		if (!NT_SUCCESS(status)) {
			if(status == STATUS_NO_MORE_FILES)
				status = STATUS_SUCCESS;
			break;
		}

		if (wcscmp(FileName.c_str(), L".") == 0 || wcscmp(FileName.c_str(), L"..") == 0)
			continue;

		SNtObject ntFoundObject(FileName, Handle);

		status = NtIo_DeleteFile(FileAttributes, &ntFoundObject.attr, cb, param);
	}

	NtClose(Handle);
	
	return status;
}

NTSTATUS NtIo_DeleteFolderRecursively(POBJECT_ATTRIBUTES objattrs, bool (*cb)(const WCHAR* info, void* param), void* param)
{
	NtIo_RemoveProblematicAttributes(objattrs);

	NTSTATUS status = NtIo_DeleteFolderRecursivelyImpl(objattrs, cb, param);

	if (NT_SUCCESS(status)) {
		NtIo_RemoveJunction(objattrs);
		status = NtDeleteFile(objattrs);
	}

	if (status == STATUS_OBJECT_NAME_NOT_FOUND || status == STATUS_OBJECT_PATH_NOT_FOUND)
		status = STATUS_SUCCESS; // we wanted it gone and its not here, success

	return status;
}

NTSTATUS NtIo_RenameFileOrFolder(POBJECT_ATTRIBUTES src_objattrs, POBJECT_ATTRIBUTES dest_objattrs, const WCHAR* DestName, ULONG Share, ULONG Create)
{
	NTSTATUS status;
	IO_STATUS_BLOCK IoStatusBlock;

	HANDLE src_handle = NULL; // open source file/folder
	status = NtCreateFile(&src_handle, DELETE | SYNCHRONIZE, src_objattrs, &IoStatusBlock, NULL,
		0,
		Share,
		FILE_OPEN,
		Create | FILE_SYNCHRONOUS_IO_NONALERT,
		NULL, NULL);

	if (!NT_SUCCESS(status))
		return status;

	HANDLE dst_handle = NULL; // open destination folder
	status = NtCreateFile(&dst_handle, FILE_GENERIC_READ, dest_objattrs, &IoStatusBlock, NULL,
		0, // for dir? FILE_ATTRIBUTE_NORMAL
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_OPEN,
		FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
		NULL, NULL);

	if (!NT_SUCCESS(status))
		return status;

	// do the rename and retry if needed
	ULONG InfoSize = sizeof(FILE_RENAME_INFORMATION) + wcslen(DestName) * sizeof(WCHAR) + 16;
	PFILE_RENAME_INFORMATION pInfo = (PFILE_RENAME_INFORMATION)malloc(InfoSize);
	pInfo->ReplaceIfExists = FALSE;
	pInfo->RootDirectory = dst_handle;
	pInfo->FileNameLength = wcslen(DestName) * sizeof(WCHAR);
	wcscpy(pInfo->FileName, DestName);

	for (int retries = 0; retries < 20; retries++)
	{
		status = NtSetInformationFile(src_handle, &IoStatusBlock, pInfo, InfoSize, FileRenameInformation);
		/*if (status == STATUS_ACCESS_DENIED || status == STATUS_SHARING_VIOLATION)
		{
			// Please terminate programs running in the sandbox before deleting its contents - 3221
		}*/
		if (status != STATUS_SHARING_VIOLATION)
			break;

		Sleep(300);
	}

	free(pInfo);

	NtClose(dst_handle);
	NtClose(src_handle);

	return status;
}

NTSTATUS NtIo_RenameFile(POBJECT_ATTRIBUTES src_objattrs, POBJECT_ATTRIBUTES dest_objattrs, const WCHAR* DestName)
{
	return NtIo_RenameFileOrFolder(src_objattrs, dest_objattrs, DestName, 0, 0);
}

NTSTATUS NtIo_RenameFolder(POBJECT_ATTRIBUTES src_objattrs, POBJECT_ATTRIBUTES dest_objattrs, const WCHAR* DestName)
{
	return NtIo_RenameFileOrFolder(src_objattrs, dest_objattrs, DestName, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, FILE_DIRECTORY_FILE);
}

NTSTATUS NtIo_RenameJunction(POBJECT_ATTRIBUTES src_objattrs, POBJECT_ATTRIBUTES dest_objattrs, const WCHAR* DestName)
{
	return NtIo_RenameFileOrFolder(src_objattrs, dest_objattrs, DestName, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, FILE_FLAG_OPEN_REPARSE_POINT);
}

BOOLEAN NtIo_FileExists(POBJECT_ATTRIBUTES objattrs)
{
	NTSTATUS status = STATUS_SUCCESS;
	IO_STATUS_BLOCK Iosb;

	HANDLE handle;
	status = NtCreateFile(&handle, SYNCHRONIZE, objattrs, &Iosb, NULL, 0, 0, FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_REPARSE_POINT, NULL, 0);
	if (NT_SUCCESS(status))
	{
		// STATUS_OBJECT_NAME_NOT_FOUND // STATUS_OBJECT_PATH_NOT_FOUND
		NtClose(handle);
		return TRUE;
	}
	return FALSE;
}

NTSTATUS NtIo_MergeFolder(POBJECT_ATTRIBUTES src_objattrs, POBJECT_ATTRIBUTES dest_objattrs, bool (*cb)(const WCHAR* info, void* param), void* param)
{
	NTSTATUS status = STATUS_SUCCESS;
	IO_STATUS_BLOCK Iosb;

	if (cb && !cb(src_objattrs->ObjectName->Buffer, param))
		return STATUS_CANCELLED;

	HANDLE ScrHandle;
	status = NtCreateFile(&ScrHandle, GENERIC_READ, src_objattrs, &Iosb,
		0, FILE_ATTRIBUTE_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, FILE_OPEN, FILE_DIRECTORY_FILE, 0, 0);
	if (!NT_SUCCESS(status))
		return status;

	HANDLE DestHandle;
	status = NtCreateFile(&DestHandle, GENERIC_READ, dest_objattrs, &Iosb,
		0, FILE_ATTRIBUTE_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, FILE_OPEN, FILE_DIRECTORY_FILE, 0, 0);
	if (!NT_SUCCESS(status))
		return status;

	for (; status == STATUS_SUCCESS; )
	{
		std::wstring FileName;
		ULONG FileAttributes;

		PFILE_BOTH_DIRECTORY_INFORMATION Info = (PFILE_BOTH_DIRECTORY_INFORMATION)malloc(PAGE_SIZE);

		HANDLE Event;
		NtCreateEvent(&Event, GENERIC_ALL, 0, NotificationEvent, FALSE);
		status = NtQueryDirectoryFile(ScrHandle, Event, 0, 0, &Iosb, Info, PAGE_SIZE, FileBothDirectoryInformation, TRUE, NULL, FALSE);
		if (status == STATUS_PENDING){
			NtWaitForSingleObject(Event, TRUE, 0);
			status = Iosb.Status;
		}
		NtClose(Event);

		if (NT_SUCCESS(status))
		{
			FileName.assign(Info->FileName, Info->FileNameLength / sizeof(wchar_t));
			FileAttributes = Info->FileAttributes;
		}

		free(Info);

		if (!NT_SUCCESS(status)) {
			if (status == STATUS_NO_MORE_FILES)
				status = STATUS_SUCCESS;
			break;
		}

		if (wcscmp(FileName.c_str(), L".") == 0 || wcscmp(FileName.c_str(), L"..") == 0)
			continue;

		SNtObject ntSrcObject(FileName, ScrHandle);
		SNtObject ntDestObject(FileName, DestHandle);

		BOOLEAN TargetExists = NtIo_FileExists(&ntDestObject.attr);
		//if (FileAttributes & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM))
		//	NtIo_RemoveProblematicAttributes(&ntFoundObject.attr);

		if (FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
			if (TargetExists)
				status = NtIo_DeleteFile(ntDestObject, cb, param);
			if (NT_SUCCESS(status))
				status = NtIo_RenameJunction(&ntSrcObject.attr, dest_objattrs, FileName.c_str());
		} 
		else if (FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (TargetExists)
				status = NtIo_MergeFolder(&ntSrcObject.attr, &ntDestObject.attr, cb, param);
			else
				status = NtIo_RenameFolder(&ntSrcObject.attr, dest_objattrs, FileName.c_str());
		}
		else
		{
			if (TargetExists)
				status = NtIo_DeleteFile(ntDestObject, cb, param);
			if (NT_SUCCESS(status))
				status = NtIo_RenameFile(&ntSrcObject.attr, dest_objattrs, FileName.c_str());
		}
	}

	NtClose(ScrHandle);

	NtClose(DestHandle);

	if (NT_SUCCESS(status))
		status = NtDeleteFile(src_objattrs);

	return status;
}

// copy

std::wstring NtIo_ResolveObjectPath(const std::wstring& fullPath) 
{
    SNtObject obj(fullPath);
    IO_STATUS_BLOCK iosb;
    HANDLE handle = NULL;
    NTSTATUS status = NtOpenFile(&handle, FILE_READ_ATTRIBUTES | SYNCHRONIZE, &obj.attr, &iosb,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                 FILE_OPEN_REPARSE_POINT | FILE_SYNCHRONOUS_IO_NONALERT);
    if (!NT_SUCCESS(status)) return L"";

    BYTE buffer[4096];
    auto nameInfo = reinterpret_cast<POBJECT_NAME_INFORMATION>(buffer);
    status = NtQueryObject(handle, ObjectNameInformation, nameInfo, sizeof(buffer), NULL);
    NtClose(handle);

    if (!NT_SUCCESS(status)) return L"";
    return std::wstring(nameInfo->Name.Buffer, nameInfo->Name.Length / sizeof(WCHAR));
}

#define SYMLINK_FLAG_DIRECTORY 0x1

NTSTATUS NtIo_CopyReparsePoint(POBJECT_ATTRIBUTES src_objattrs, POBJECT_ATTRIBUTES dest_objattrs,
                                 const std::wstring& srcBase, const std::wstring& destBase,
                                 bool (*cb)(const WCHAR* info, void* param), void* param)
{
    NTSTATUS status;
    IO_STATUS_BLOCK IoStatusBlock;

    if (cb && !cb(src_objattrs->ObjectName->Buffer, param))
        return STATUS_CANCELLED;

    HANDLE src_handle = NULL;
    status = NtCreateFile(&src_handle, GENERIC_READ | READ_CONTROL | SYNCHRONIZE, src_objattrs, &IoStatusBlock, NULL, 0,
                           FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                           FILE_OPEN, FILE_OPEN_REPARSE_POINT | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
    if (!NT_SUCCESS(status))
        return status;

    BYTE reparseBuffer[65536] = {0};
    ULONG reparseBufferSize = sizeof(reparseBuffer);
    status = NtFsControlFile(src_handle, NULL, NULL, NULL, &IoStatusBlock,
                             FSCTL_GET_REPARSE_POINT, NULL, 0, reparseBuffer, reparseBufferSize);
    if (status == STATUS_PENDING) {
        NtWaitForSingleObject(src_handle, TRUE, 0);
        status = IoStatusBlock.Status;
    }
	reparseBufferSize = IoStatusBlock.Information;
    if (!NT_SUCCESS(status)) {
        NtClose(src_handle);
        return status;
    }
    NtClose(src_handle);

	auto* pReparseData = (REPARSE_DATA_BUFFER*)reparseBuffer;

    if (pReparseData->ReparseTag == IO_REPARSE_TAG_SYMLINK) {

        USHORT substOffset = pReparseData->SymbolicLinkReparseBuffer.SubstituteNameOffset;
        USHORT substLength = pReparseData->SymbolicLinkReparseBuffer.SubstituteNameLength;
        std::wstring target(reinterpret_cast<wchar_t*>(pReparseData->SymbolicLinkReparseBuffer.PathBuffer + (substOffset / sizeof(WCHAR))), substLength / sizeof(WCHAR));

		USHORT printOffset = pReparseData->SymbolicLinkReparseBuffer.PrintNameOffset;
        USHORT printLength = pReparseData->SymbolicLinkReparseBuffer.PrintNameLength;
		std::wstring display(reinterpret_cast<wchar_t*>(pReparseData->SymbolicLinkReparseBuffer.PathBuffer + (printOffset / sizeof(WCHAR))), printLength / sizeof(WCHAR));

        target = NtIo_ResolveObjectPath(target);

		if (target.compare(0, srcBase.size(), srcBase) == 0) {

			std::wstring newTarget = destBase + target.substr(srcBase.size());

			const std::wstring& printName = display;
			const std::wstring& substName = newTarget;

			const size_t substLen = substName.size() * sizeof(WCHAR);
			const size_t printLen = printName.size() * sizeof(WCHAR);
			const size_t totalLen = FIELD_OFFSET(REPARSE_DATA_BUFFER, SymbolicLinkReparseBuffer.PathBuffer) + substLen + printLen;

			std::vector<BYTE> newReparseBuffer(totalLen);
			REPARSE_DATA_BUFFER* pBuf = (REPARSE_DATA_BUFFER*)newReparseBuffer.data();
			pBuf->ReparseTag = IO_REPARSE_TAG_SYMLINK;
			pBuf->ReparseDataLength = (USHORT)(substLen + printLen + 12);
			pBuf->Reserved = 0;
			pBuf->SymbolicLinkReparseBuffer.SubstituteNameOffset = 0;
			pBuf->SymbolicLinkReparseBuffer.SubstituteNameLength = (USHORT)substLen;
			pBuf->SymbolicLinkReparseBuffer.PrintNameOffset = (USHORT)substLen;
			pBuf->SymbolicLinkReparseBuffer.PrintNameLength = (USHORT)printLen;
			pBuf->SymbolicLinkReparseBuffer.Flags = pReparseData->SymbolicLinkReparseBuffer.Flags; 

			memcpy(pBuf->SymbolicLinkReparseBuffer.PathBuffer, substName.c_str(), substLen);
			memcpy((BYTE*)pBuf->SymbolicLinkReparseBuffer.PathBuffer + substLen, printName.c_str(), printLen);

			HANDLE dest_handle = NULL;
			ULONG symlinkOptions = FILE_OPEN_REPARSE_POINT | FILE_SYNCHRONOUS_IO_NONALERT;
            if (pReparseData->SymbolicLinkReparseBuffer.Flags & SYMLINK_FLAG_DIRECTORY)
                symlinkOptions |= FILE_DIRECTORY_FILE;

            status = NtCreateFile(&dest_handle, GENERIC_WRITE | WRITE_DAC | WRITE_OWNER | SYNCHRONIZE, dest_objattrs, &IoStatusBlock, NULL, FILE_ATTRIBUTE_REPARSE_POINT,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_CREATE, symlinkOptions, NULL, 0);
			if (!NT_SUCCESS(status)) 
				return status;

			status = NtFsControlFile(dest_handle, NULL, NULL, NULL, &IoStatusBlock,
				FSCTL_SET_REPARSE_POINT, newReparseBuffer.data(), (ULONG)totalLen, NULL, 0);
			if (status == STATUS_PENDING) {
				NtWaitForSingleObject(dest_handle, TRUE, 0);
				status = IoStatusBlock.Status;
			}
			NtClose(dest_handle);
			return status;
		}
    }

	if (pReparseData->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT) {

        USHORT substOffset = pReparseData->MountPointReparseBuffer.SubstituteNameOffset;
        USHORT substLength = pReparseData->MountPointReparseBuffer.SubstituteNameLength;
        std::wstring target(reinterpret_cast<wchar_t*>(pReparseData->MountPointReparseBuffer.PathBuffer + (substOffset / sizeof(WCHAR))), substLength / sizeof(WCHAR));

		USHORT printOffset = pReparseData->MountPointReparseBuffer.PrintNameOffset;
        USHORT printLength = pReparseData->MountPointReparseBuffer.PrintNameLength;
		std::wstring display(reinterpret_cast<wchar_t*>(pReparseData->MountPointReparseBuffer.PathBuffer + (printOffset / sizeof(WCHAR))), printLength / sizeof(WCHAR));

		target = NtIo_ResolveObjectPath(target);

		if (target.compare(0, srcBase.size(), srcBase) == 0) {

			std::wstring newTarget = destBase + target.substr(srcBase.size());

			const std::wstring& printName = display;
			const std::wstring& substName = newTarget;

			const size_t substLen = substName.size() * sizeof(WCHAR);
			const size_t printLen = printName.size() * sizeof(WCHAR);
			const size_t totalLen = FIELD_OFFSET(REPARSE_DATA_BUFFER, MountPointReparseBuffer.PathBuffer) + substLen + printLen + sizeof(WCHAR) * 2;

			std::vector<BYTE> newReparseBuffer(totalLen);
			REPARSE_DATA_BUFFER* pBuf = (REPARSE_DATA_BUFFER*)newReparseBuffer.data();
			pBuf->ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;
			pBuf->ReparseDataLength = (USHORT)(substLen + printLen + 12);
			pBuf->Reserved = 0;
			pBuf->MountPointReparseBuffer.SubstituteNameOffset = 0;
			pBuf->MountPointReparseBuffer.SubstituteNameLength = (USHORT)substLen;
			pBuf->MountPointReparseBuffer.PrintNameOffset = (USHORT)(substLen + sizeof(WCHAR));
			pBuf->MountPointReparseBuffer.PrintNameLength = (USHORT)printLen;

			memcpy(pBuf->MountPointReparseBuffer.PathBuffer, substName.c_str(), substLen);
			memcpy((BYTE*)pBuf->MountPointReparseBuffer.PathBuffer + substLen + sizeof(WCHAR), printName.c_str(), printLen);

			HANDLE dest_handle = NULL;
			status = NtCreateFile(&dest_handle, GENERIC_WRITE | WRITE_DAC | WRITE_OWNER | SYNCHRONIZE, dest_objattrs, &IoStatusBlock, NULL, FILE_ATTRIBUTE_REPARSE_POINT,
				FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
				FILE_CREATE, FILE_DIRECTORY_FILE | FILE_OPEN_REPARSE_POINT | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
			if (!NT_SUCCESS(status))
				return status;

			status = NtFsControlFile(dest_handle, NULL, NULL, NULL, &IoStatusBlock,
				FSCTL_SET_REPARSE_POINT, newReparseBuffer.data(), (ULONG)totalLen, NULL, 0);
			if (status == STATUS_PENDING) {
				NtWaitForSingleObject(dest_handle, TRUE, 0);
				status = IoStatusBlock.Status;
			}
			NtClose(dest_handle);
			return status;
		}
	}
	
    HANDLE dest_handle = NULL;
    ULONG genericOptions = FILE_OPEN_REPARSE_POINT | FILE_SYNCHRONOUS_IO_NONALERT;
    if (pReparseData->ReparseTag == IO_REPARSE_TAG_SYMLINK && (pReparseData->SymbolicLinkReparseBuffer.Flags & SYMLINK_FLAG_DIRECTORY))
        genericOptions |= FILE_DIRECTORY_FILE;
    else if (pReparseData->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT)
        genericOptions |= FILE_DIRECTORY_FILE;

    status = NtCreateFile(&dest_handle, GENERIC_WRITE | WRITE_DAC | WRITE_OWNER | SYNCHRONIZE, dest_objattrs, &IoStatusBlock, NULL, FILE_ATTRIBUTE_REPARSE_POINT,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_CREATE, genericOptions, NULL, 0);
    if (!NT_SUCCESS(status))
        return status;

    status = NtFsControlFile(dest_handle, NULL, NULL, NULL, &IoStatusBlock,
            FSCTL_SET_REPARSE_POINT, reparseBuffer, reparseBufferSize, NULL, 0);
    if (status == STATUS_PENDING) {
        NtWaitForSingleObject(dest_handle, TRUE, 0);
        status = IoStatusBlock.Status;
    }
    NtClose(dest_handle);
    return status;
}

NTSTATUS NtIo_CopyBasicInfo(HANDLE src, HANDLE dst)
{
    FILE_BASIC_INFORMATION basicInfo = {};
    IO_STATUS_BLOCK iosb;
    NTSTATUS status = NtQueryInformationFile(src, &iosb, &basicInfo, sizeof(basicInfo), FileBasicInformation);
    if (!NT_SUCCESS(status)) return status;
    return NtSetInformationFile(dst, &iosb, &basicInfo, sizeof(basicInfo), FileBasicInformation);
}

NTSTATUS NtIo_CopySecurity(HANDLE src, HANDLE dst)
{
    PSECURITY_DESCRIPTOR sd = NULL;
    NTSTATUS status = NtQuerySecurityObject(src, OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION |
                                                  DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION,
                                           NULL, 0, (PULONG)&sd);
    if (status != STATUS_BUFFER_TOO_SMALL) return status;

    ULONG needed = 0;
    NtQuerySecurityObject(src, OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION |
                               DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION,
                          NULL, 0, &needed);

    sd = (PSECURITY_DESCRIPTOR)malloc(needed);
    status = NtQuerySecurityObject(src, OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION |
                                        DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION,
                                   sd, needed, &needed);
    if (!NT_SUCCESS(status)) {
        free(sd);
        return status;
    }

    status = NtSetSecurityObject(dst, OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION |
                                       DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION, sd);
    free(sd);
    return status;
}

extern "C" {

NTSTATUS NTAPI NtQueryEaFile(
    HANDLE FileHandle,
    PIO_STATUS_BLOCK IoStatusBlock,
    PVOID Buffer,
    ULONG Length,
    BOOLEAN ReturnSingleEntry,
    PVOID EaList OPTIONAL,
    ULONG EaListLength,
    PULONG EaIndex OPTIONAL,
    BOOLEAN RestartScan
);

NTSTATUS NTAPI NtSetEaFile(
    HANDLE FileHandle,
    PIO_STATUS_BLOCK IoStatusBlock,
    PVOID EaBuffer,
    ULONG EaBufferSize
);

}

NTSTATUS NtIo_CopyFile(
	POBJECT_ATTRIBUTES src_objattrs,
	POBJECT_ATTRIBUTES dest_objattrs,
	bool (*cb)(const WCHAR* info, void* param),
	void* param);

void NtIo_CopyMetadata(HANDLE srcHandle, HANDLE dstHandle,
                                          POBJECT_ATTRIBUTES src_objattrs, POBJECT_ATTRIBUTES dst_objattrs) {
    IO_STATUS_BLOCK IoStatusBlock;

    // Copy EA
    void* eaBuffer = malloc(64 * 1024);
    ULONG eaLength = 0;
    NTSTATUS status = NtQueryEaFile(srcHandle, &IoStatusBlock, eaBuffer, 64 * 1024, FALSE, NULL, 0, NULL, TRUE);
    if (status == STATUS_BUFFER_OVERFLOW || status == STATUS_SUCCESS) {
        eaLength = (ULONG)IoStatusBlock.Information;
        if (eaLength > 0)
            NtSetEaFile(dstHandle, &IoStatusBlock, eaBuffer, eaLength);
    }
    free(eaBuffer);

    // Copy ADS
    FILE_STREAM_INFORMATION* streamInfo = (FILE_STREAM_INFORMATION*)malloc(PAGE_SIZE);
    status = NtQueryInformationFile(srcHandle, &IoStatusBlock, streamInfo, PAGE_SIZE, FileStreamInformation);
    if (NT_SUCCESS(status) && IoStatusBlock.Information >= sizeof(FILE_STREAM_INFORMATION)) {
        FILE_STREAM_INFORMATION* entry = streamInfo;
        while (true) {
            std::wstring streamName(entry->StreamName, entry->StreamNameLength / sizeof(WCHAR));
            if (!streamName.empty() && streamName.find(L":") != std::wstring::npos && streamName != L"::$DATA") {
                std::wstring srcStreamPath = std::wstring(src_objattrs->ObjectName->Buffer) + streamName;
                std::wstring dstStreamPath = std::wstring(dst_objattrs->ObjectName->Buffer) + streamName;

                SNtObject srcStreamObj(srcStreamPath);
                SNtObject dstStreamObj(dstStreamPath);

                NtIo_CopyFile(&srcStreamObj.attr, &dstStreamObj.attr, nullptr, nullptr);
            }
            if (entry->NextEntryOffset == 0) break;
            entry = (FILE_STREAM_INFORMATION*)((BYTE*)entry + entry->NextEntryOffset);
        }
    }
    free(streamInfo);
}

NTSTATUS NtIo_CopyFile(
    POBJECT_ATTRIBUTES src_objattrs,
    POBJECT_ATTRIBUTES dest_objattrs,
    bool (*cb)(const WCHAR* info, void* param),
    void* param)
{
    NTSTATUS status;
    IO_STATUS_BLOCK IoStatusBlock;

    if (cb && !cb(src_objattrs->ObjectName->Buffer, param))
        return STATUS_CANCELLED;

    HANDLE src_handle = NULL;
    status = NtCreateFile(&src_handle, GENERIC_READ | READ_CONTROL | SYNCHRONIZE, src_objattrs, &IoStatusBlock, NULL,
                          0, FILE_SHARE_READ, FILE_OPEN,
                          FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
    if (!NT_SUCCESS(status)) return status;

    HANDLE dst_handle = NULL;
    status = NtCreateFile(&dst_handle, GENERIC_WRITE | WRITE_DAC | WRITE_OWNER | SYNCHRONIZE, dest_objattrs, &IoStatusBlock, NULL,
                          FILE_ATTRIBUTE_NORMAL, 0, FILE_OVERWRITE_IF,
                          FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
    if (!NT_SUCCESS(status)) {
        NtClose(src_handle);
        return status;
    }

    BYTE buffer[8192];
    ULONG bytesRead = 0;
    ULONGLONG lastCbTime = 0;
    ULONGLONG totalWritten = 0;
	ULONGLONG fileSize = 0;

    FILE_STANDARD_INFORMATION stdInfo = {};
    status = NtQueryInformationFile(src_handle, &IoStatusBlock, &stdInfo, sizeof(stdInfo), FileStandardInformation);
    if (NT_SUCCESS(status)) 
		fileSize = stdInfo.EndOfFile.QuadPart;

	while (NT_SUCCESS(status)) {
		if (cb) {
			ULONGLONG now = GetTickCount64();
			if (now - lastCbTime >= 250) {
				int percent = (fileSize > 0) ? (int)((totalWritten * 100) / fileSize) : 0;
				std::wstring displayName = src_objattrs->ObjectName->Buffer;
				displayName += L" [" + std::to_wstring(percent) + L"%]";
				if (!cb(displayName.c_str(), param)) {
					status = STATUS_CANCELLED;
					break;
				}
				lastCbTime = now;
			}
		}

		status = NtReadFile(src_handle, NULL, NULL, NULL, &IoStatusBlock, buffer, sizeof(buffer), NULL, NULL);
		if (status == STATUS_PENDING) {
			NtWaitForSingleObject(src_handle, TRUE, NULL);
			status = IoStatusBlock.Status;
		}

		if (status == STATUS_END_OF_FILE || IoStatusBlock.Information == 0) {
			status = STATUS_SUCCESS;
			break;
		}

		if (!NT_SUCCESS(status))
			break;

		totalWritten += IoStatusBlock.Information;
		ULONG bytesToWrite = (ULONG)IoStatusBlock.Information;
		status = NtWriteFile(dst_handle, NULL, NULL, NULL, &IoStatusBlock, buffer, bytesToWrite, NULL, NULL);
		if (status == STATUS_PENDING) {
			NtWaitForSingleObject(dst_handle, TRUE, NULL);
			status = IoStatusBlock.Status;
		}
	}

	const WCHAR* fullName = src_objattrs->ObjectName->Buffer;
    const WCHAR* lastSlash = wcsrchr(fullName, L'\\');
    const WCHAR* colon = wcschr(fullName, L':');
    bool isStream = colon && (!lastSlash || colon > lastSlash);

	if (NT_SUCCESS(status) && !isStream) {
		NtIo_CopyBasicInfo(src_handle, dst_handle);
		NtIo_CopySecurity(src_handle, dst_handle);
        NtIo_CopyMetadata(src_handle, dst_handle, src_objattrs, dest_objattrs);
	}

    NtClose(src_handle);
    NtClose(dst_handle);

	if (status == STATUS_CANCELLED)
		NtDeleteFile(dest_objattrs);

    return status;
}

NTSTATUS NtIo_CopyFolder(POBJECT_ATTRIBUTES src_objattrs, POBJECT_ATTRIBUTES dest_objattrs,
                         bool (*cb)(const WCHAR* info, void* param), void* param)
{
    NTSTATUS status = STATUS_SUCCESS;
    IO_STATUS_BLOCK IoStatusBlock;

    if (cb && !cb(src_objattrs->ObjectName->Buffer, param))
        return STATUS_CANCELLED;

    HANDLE destFolderHandle = NULL;
    status = NtCreateFile(&destFolderHandle, FILE_GENERIC_WRITE | READ_CONTROL | WRITE_DAC | WRITE_OWNER | SYNCHRONIZE, dest_objattrs, &IoStatusBlock, NULL,
                          FILE_ATTRIBUTE_DIRECTORY,
                          FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                          FILE_OPEN_IF, FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
    if (!NT_SUCCESS(status)) return status;

    HANDLE srcHandle = NULL;
    status = NtCreateFile(&srcHandle, FILE_LIST_DIRECTORY | FILE_READ_ATTRIBUTES | READ_CONTROL | SYNCHRONIZE, src_objattrs, &IoStatusBlock, NULL,
                          FILE_ATTRIBUTE_DIRECTORY,
                          FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                          FILE_OPEN, FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
    if (!NT_SUCCESS(status)) {
        NtClose(destFolderHandle);
        return status;
    }

    NtIo_CopyBasicInfo(srcHandle, destFolderHandle);
    NtIo_CopySecurity(srcHandle, destFolderHandle);
	NtIo_CopyMetadata(srcHandle, destFolderHandle, src_objattrs, dest_objattrs);

    std::wstring srcBase = src_objattrs->ObjectName->Buffer;
    std::wstring destBase = dest_objattrs->ObjectName->Buffer;

	if (srcBase.compare(0, 4, L"\\??\\") == 0)
		srcBase = NtIo_ResolveObjectPath(srcBase);
	if (destBase.compare(0, 4, L"\\??\\") == 0)
		destBase = NtIo_ResolveObjectPath(destBase);

    for (;;) {
        std::wstring FileName;
        ULONG FileAttributes = 0;

        PFILE_BOTH_DIRECTORY_INFORMATION Info = (PFILE_BOTH_DIRECTORY_INFORMATION)malloc(PAGE_SIZE);
        HANDLE event;
        NtCreateEvent(&event, GENERIC_ALL, NULL, NotificationEvent, FALSE);

        status = NtQueryDirectoryFile(srcHandle, event, NULL, NULL, &IoStatusBlock,
                                      Info, PAGE_SIZE, FileBothDirectoryInformation, TRUE, NULL, FALSE);
        if (status == STATUS_PENDING) {
            NtWaitForSingleObject(event, TRUE, NULL);
            status = IoStatusBlock.Status;
        }
        NtClose(event);

        if (!NT_SUCCESS(status)) {
            free(Info);
            if (status == STATUS_NO_MORE_FILES) status = STATUS_SUCCESS;
            break;
        }

        FileName.assign(Info->FileName, Info->FileNameLength / sizeof(WCHAR));
        FileAttributes = Info->FileAttributes;
        free(Info);

        if (FileName == L"." || FileName == L"..") continue;

        SNtObject ntSrcObject(srcBase + L"\\" + FileName);
        SNtObject ntDestObject(destBase + L"\\" + FileName);

        if (FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
            status = NtIo_CopyReparsePoint(&ntSrcObject.attr, &ntDestObject.attr, srcBase, destBase, cb, param);
        } else if (FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            status = NtIo_CopyFolder(&ntSrcObject.attr, &ntDestObject.attr, cb, param);
        } else {
            status = NtIo_CopyFile(&ntSrcObject.attr, &ntDestObject.attr, cb, param);
        }

        if (!NT_SUCCESS(status)) break;
    }

    NtClose(srcHandle);
    NtClose(destFolderHandle);
    return status;
}
