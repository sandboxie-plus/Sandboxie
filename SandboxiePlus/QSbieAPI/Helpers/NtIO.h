#pragma once

#include "../qsbieapi_global.h"

struct QSBIEAPI_EXPORT SNtObject
{
	SNtObject(const std::wstring& Name, HANDLE parent = NULL)
	{
		name = Name;
		RtlInitUnicodeString(&uni, name.c_str());
		InitializeObjectAttributes(&attr, &uni, OBJ_CASE_INSENSITIVE, parent, 0);
	}

	POBJECT_ATTRIBUTES Get() { return &attr; }

	std::wstring name;
	UNICODE_STRING uni;
	OBJECT_ATTRIBUTES attr;

private:
	SNtObject(const SNtObject&) {}
	SNtObject& operator=(const SNtObject&) { return *this; }
};


QSBIEAPI_EXPORT bool NtIo_WaitForFolder(POBJECT_ATTRIBUTES objattrs, int seconds = 10, bool (*cb)(const WCHAR* info, void* param) = NULL, void* param = NULL);

QSBIEAPI_EXPORT BOOLEAN NtIo_FileExists(POBJECT_ATTRIBUTES objattrs);

QSBIEAPI_EXPORT NTSTATUS NtIo_RemoveProblematicAttributes(POBJECT_ATTRIBUTES objattrs);

QSBIEAPI_EXPORT NTSTATUS NtIo_RemoveJunction(POBJECT_ATTRIBUTES objattrs);

QSBIEAPI_EXPORT NTSTATUS NtIo_DeleteFile(SNtObject& ntObject, bool (*cb)(const WCHAR* info, void* param) = NULL, void* param = NULL);
QSBIEAPI_EXPORT NTSTATUS NtIo_DeleteFolderRecursively(POBJECT_ATTRIBUTES objattrs, bool (*cb)(const WCHAR* info, void* param) = NULL, void* param = NULL);

QSBIEAPI_EXPORT NTSTATUS NtIo_RenameFile(POBJECT_ATTRIBUTES src_objattrs, POBJECT_ATTRIBUTES dest_objattrs, const WCHAR* DestName);
QSBIEAPI_EXPORT NTSTATUS NtIo_RenameFolder(POBJECT_ATTRIBUTES src_objattrs, POBJECT_ATTRIBUTES dest_objattrs, const WCHAR* DestName);
QSBIEAPI_EXPORT NTSTATUS NtIo_RenameJunction(POBJECT_ATTRIBUTES src_objattrs, POBJECT_ATTRIBUTES dest_objattrs, const WCHAR* DestName);

QSBIEAPI_EXPORT NTSTATUS NtIo_MergeFolder(POBJECT_ATTRIBUTES src_objattrs, POBJECT_ATTRIBUTES dest_objattrs, bool (*cb)(const WCHAR* info, void* param) = NULL, void* param = NULL);

QSBIEAPI_EXPORT NTSTATUS NtIo_CopyFolder(POBJECT_ATTRIBUTES src_objattrs, POBJECT_ATTRIBUTES dest_objattrs, bool (*cb)(const WCHAR* info, void* param), void* param);