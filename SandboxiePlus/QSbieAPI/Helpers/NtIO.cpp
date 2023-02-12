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

		if (FileAttributes & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM))
			NtIo_RemoveProblematicAttributes(&ntFoundObject.attr);

		if (FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
			status = NtIo_RemoveJunction(&ntFoundObject.attr);
		else if (FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			status = NtIo_DeleteFolderRecursivelyImpl(&ntFoundObject.attr, cb, param);
		
		if (NT_SUCCESS(status))
			status = NtDeleteFile(&ntFoundObject.attr);
	}

	NtClose(Handle);
	
	return status;
}

NTSTATUS NtIo_DeleteFolderRecursively(POBJECT_ATTRIBUTES objattrs, bool (*cb)(const WCHAR* info, void* param), void* param)
{
	NtIo_RemoveProblematicAttributes(objattrs);

	NTSTATUS status = NtIo_DeleteFolderRecursivelyImpl(objattrs, cb, param);

	if (NT_SUCCESS(status))
		status = NtDeleteFile(objattrs);

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
	status = NtCreateFile(&handle, SYNCHRONIZE, objattrs, &Iosb, NULL, 0, 0, FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
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

		if (FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
				status = NtIo_RenameJunction(&ntSrcObject.attr, dest_objattrs, FileName.c_str());
			else if (TargetExists)
				status = NtIo_MergeFolder(&ntSrcObject.attr, &ntDestObject.attr, cb, param);
			else
				status = NtIo_RenameFolder(&ntSrcObject.attr, dest_objattrs, FileName.c_str());
		}
		else
		{
			if (TargetExists)
				status = NtDeleteFile(&ntDestObject.attr);
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