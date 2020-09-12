/*
 *
 * Copyright (c) 2020, David Xanatos
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "stdafx.h"
#include <QtConcurrent>
#include "SandBox.h"
#include "../SbieAPI.h"

#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

#include <windows.h>
#include "..\..\Sandboxie\common\win32_ntddk.h"

//struct SSandBox
//{
//};

CSandBox::CSandBox(const QString& BoxName, class CSbieAPI* pAPI) : CSbieIni(BoxName, pAPI)
{
	//m = new SSandBox;

	m_ActiveProcessCount = 0;

	// when loading a sandbox that is not initialized, initialize it
	int cfglvl = GetNum("ConfigLevel");
	if (cfglvl >= 7)
		return;
	SetNum("ConfigLevel", 7);

	if (cfglvl == 6) {
		//SetDefaultTemplates7(*this);
	}
	else if (cfglvl >= 1) {
		//UpdateTemplates(*this);
	}
	else
	{
		SetBool("AutoRecover", false);
		SetBool("BlockNetworkFiles", true);

		//SetDefaultTemplates6(*this); // why 6?

		InsertText("RecoverFolder", "%Desktop%");
		//InsertText("RecoverFolder", "%Favorites%"); // obsolete
		InsertText("RecoverFolder", "%Personal%");
		InsertText("RecoverFolder", "%{374DE290-123F-4565-9164-39C4925E467B}%"); // %USERPROFILE%\Downloads

		SetText("BorderColor", "#00FFFF,ttl"); // "#00FFFF,off"
	}
}

CSandBox::~CSandBox()
{
	//delete m;
}

void CSandBox::UpdateDetails()
{
}

SB_STATUS CSandBox::RunStart(const QString& Command)
{
	return m_pAPI->RunStart(m_Name, Command);
}

SB_STATUS CSandBox::RunCommand(const QString& Command)
{
	return m_pAPI->RunSandboxed(m_Name, Command);
}

SB_STATUS CSandBox::TerminateAll()
{
	return m_pAPI->TerminateAll(m_Name);
}

SB_PROGRESS CSandBox::CleanBox()
{
	if (GetBool("NeverDelete", false))
		return SB_ERR(tr("Delete protection is enabled for the sandbox"));

	SB_STATUS Status = TerminateAll();
	if (Status.IsError())
		return Status;

	QString TempPath;
	Status = RenameForDelete(m_FilePath, TempPath);
	if (Status.IsError()) {
		if (Status.GetStatus() == STATUS_OBJECT_NAME_NOT_FOUND || Status.GetStatus() == STATUS_OBJECT_PATH_NOT_FOUND)
			return SB_OK; // no sandbox folder, nothing to do
		return Status;
	}

	return CleanBoxFolders(QStringList(TempPath));
}

SB_PROGRESS CSandBox::CleanBoxFolders(const QStringList& BoxFolders)
{
	// cache the DeleteCommand as GetText is not thread safe
	QString DeleteCommand = GetText("DeleteCommand", "%SystemRoot%\\System32\\cmd.exe /c rmdir /s /q \"%SANDBOX%\"");

	// do the actual delete asynchroniusly in a reandom worker thread...
	//QFutureWatcher<bool>* pFuture = new QFutureWatcher<bool>();
	//connect(pFuture, SIGNAL(finished()), pFuture, SLOT(deleteLater()));
	//pFuture->setFuture(QtConcurrent::run(CSandBox::CleanBoxAsync, pProgress, BoxFolders, DeleteCommand));

	CSbieProgressPtr pProgress = CSbieProgressPtr(new CSbieProgress());
	QtConcurrent::run(CSandBox::CleanBoxAsync, pProgress, BoxFolders, DeleteCommand);
	return SB_PROGRESS(OP_ASYNC, pProgress);
}

SB_STATUS CSandBox::RenameBox(const QString& NewName)
{
	if (QDir(m_FilePath).exists())
		return SB_ERR(tr("A sandbox must be emptied before it can be renamed."));
	if(NewName.length() > 32)
		return SB_ERR(tr("The sandbox name can not be longer than 32 charakters."));
	
	return RenameSection(QString(NewName).replace(" ", "_"));
}

SB_STATUS CSandBox::RemoveBox()
{
	if (QDir(m_FilePath).exists())
		return SB_ERR(tr("A sandbox must be emptied before it can be deleted."));

	return RemoveSection();
}

QList<SBoxSnapshot> CSandBox::GetSnapshots(QString* pCurrent) const
{
	QSettings ini(m_FilePath + "\\Snapshots.ini", QSettings::IniFormat);

	QList<SBoxSnapshot> Snapshots;

	foreach(const QString& Snapshot, ini.childGroups())
	{
		if (Snapshot.indexOf("Snapshot_") != 0)
			continue;

		SBoxSnapshot BoxSnapshot;
		BoxSnapshot.ID = Snapshot.mid(9);
		BoxSnapshot.Parent = ini.value(Snapshot + "/Parent").toString();

		BoxSnapshot.NameStr = ini.value(Snapshot + "/Name").toString();
		BoxSnapshot.InfoStr = ini.value(Snapshot + "/Description").toString();
		BoxSnapshot.SnapDate = ini.value(Snapshot + "/SnapshotDate").toDateTime();

		Snapshots.append(BoxSnapshot);
	}

	if(pCurrent)
		*pCurrent = ini.value("Current/Snapshot").toString();

	return Snapshots;
}

SB_STATUS CSandBox__OsRename(const wstring& SrcPath, const wstring& DestDir, const wstring& DestName, int Mode);

SB_PROGRESS CSandBox::TakeSnapshot(const QString& Name)
{
	QSettings ini(m_FilePath + "\\Snapshots.ini", QSettings::IniFormat);

	if (m_pAPI->HasProcesses(m_Name))
		return SB_ERR(tr("Can't take a snapshot while processes are running in the box."), OP_CONFIRM);

	QStringList Snapshots = ini.childGroups();

	QString ID;
	for (int i = 1; ; i++)
	{
		ID = QString::number(i);
		if(!Snapshots.contains("Snapshot_" + ID))
			break;
	}

	if (!QDir().mkdir(m_FilePath + "\\snapshot-" + ID))
		return SB_ERR(tr("Failed to create directory for new snapshot"));
	if (!QFile::copy(m_FilePath + "\\RegHive", m_FilePath + "\\snapshot-" + ID + "\\RegHive"))
		return SB_ERR(tr("Failed to copy RegHive to snapshot"));

	ini.setValue("Snapshot_" + ID + "/Name", Name);
	ini.setValue("Snapshot_" + ID + "/SnapshotDate", QDateTime::currentDateTime());
	QString Current = ini.value("Current/Snapshot").toString();
	if(!Current.isEmpty())
		ini.setValue("Snapshot_" + ID + "/Parent", Current);

	ini.setValue("Current/Snapshot", ID);
	ini.sync();

	wstring dest_dir = (m_FilePath + "\\snapshot-" + ID).toStdWString();
	SB_STATUS Status = CSandBox__OsRename((m_FilePath + "\\drive").toStdWString(), dest_dir, L"drive", 1);
	if (Status.IsError() && Status.GetStatus() != STATUS_OBJECT_NAME_NOT_FOUND && Status.GetStatus() != STATUS_OBJECT_PATH_NOT_FOUND)
		return Status;
	Status = CSandBox__OsRename((m_FilePath + "\\share").toStdWString(), dest_dir, L"share", 1);
	if (Status.IsError() && Status.GetStatus() != STATUS_OBJECT_NAME_NOT_FOUND && Status.GetStatus() != STATUS_OBJECT_PATH_NOT_FOUND)
		return Status;
	Status = CSandBox__OsRename((m_FilePath + "\\user").toStdWString(), dest_dir, L"user", 1);
	if (Status.IsError() && Status.GetStatus() != STATUS_OBJECT_NAME_NOT_FOUND && Status.GetStatus() != STATUS_OBJECT_PATH_NOT_FOUND)
		return Status;

	return SB_OK;
}

SB_PROGRESS CSandBox::RemoveSnapshot(const QString& ID)
{
	QSettings ini(m_FilePath + "\\Snapshots.ini", QSettings::IniFormat);

	if (!ini.childGroups().contains("Snapshot_" + ID))
		return SB_ERR(tr("Snapshot not found"));

	QString Current = ini.value("Current/Snapshot").toString();
	if(Current == ID)
		return SB_ERR(tr("Can't remove a currently used snapshot"));
	
	foreach(const QString& Snapshot, ini.childGroups())
	{
		if (Snapshot.indexOf("Snapshot_") != 0)
			continue;

		if(ini.value(Snapshot + "/Parent").toString() == ID)
			return SB_ERR(tr("Can't remove a snapshots that is used by an other snapshot"));
	}

	// ToDo: allow removel of intermediate snapshots by merging the folders

	if (m_pAPI->HasProcesses(m_Name))
		return SB_ERR(tr("Can't remove a snapshots while processes are running in the box."), OP_CONFIRM);

	ini.remove("Snapshot_" + ID);
	ini.sync();

	return CleanBoxFolders(QStringList(m_FilePath + "\\snapshot-" + ID));
}

SB_PROGRESS CSandBox::SelectSnapshot(const QString& ID)
{
	QSettings ini(m_FilePath + "\\Snapshots.ini", QSettings::IniFormat);

	if (!ini.childGroups().contains("Snapshot_" + ID))
		return SB_ERR(tr("Snapshot not found"));

	if (m_pAPI->HasProcesses(m_Name))
		return SB_ERR(tr("Can't switch snapshots while processes are running in the box."), OP_CONFIRM);

	ini.setValue("Current/Snapshot", ID);
	ini.sync();

	if (!QFile::remove(m_FilePath + "\\RegHive"))
		return SB_ERR(tr("Failed to remove old RegHive"));
	if (!QFile::copy(m_FilePath + "\\snapshot-" + ID + "\\RegHive", m_FilePath + "\\RegHive"))
		return SB_ERR(tr("Failed to copy RegHive from snapshot"));

	QStringList BoxFolders;
	BoxFolders.append(m_FilePath + "\\drive");
	BoxFolders.append(m_FilePath + "\\share");
	BoxFolders.append(m_FilePath + "\\user");
	return CleanBoxFolders(BoxFolders);
}

SB_STATUS CSandBox::SetSnapshotInfo(const QString& ID, const QString& Name, const QString& Description)
{
	QSettings ini(m_FilePath + "\\Snapshots.ini", QSettings::IniFormat);

	if (!ini.childGroups().contains("Snapshot_" + ID))
		return SB_ERR(tr("Snapshot not found"));

	if (!Name.isNull())
		ini.setValue("Snapshot_" + ID + "/Name", Name);
	if (!Description.isNull())
		ini.setValue("Snapshot_" + ID + "/Description", Description);

	return SB_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// SBox Deletion
//

SB_STATUS CSandBox__OsRename(const wstring& SrcPath, const wstring& DestDir, const wstring& DestName, int Mode)
{
	NTSTATUS status;
	IO_STATUS_BLOCK IoStatusBlock;

	wstring src_path = L"\\??\\" + SrcPath;
	UNICODE_STRING uni;
	RtlInitUnicodeString(&uni, src_path.c_str());

	OBJECT_ATTRIBUTES objattrs;
	InitializeObjectAttributes(&objattrs, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

	HANDLE src_handle = NULL; // open source file/folder
	status = NtCreateFile(&src_handle, DELETE | SYNCHRONIZE, &objattrs, &IoStatusBlock, NULL, 
		NULL,
		Mode == 1 ? (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE) : 0,			// mode 1 = folder, 0 = file
		FILE_OPEN,
		FILE_SYNCHRONOUS_IO_NONALERT | (Mode == 1 ? (FILE_DIRECTORY_FILE) : 0),				// mode 1 = folder, 0 = file
		NULL, NULL);

	if (!NT_SUCCESS(status)) {
		return SB_ERR(CSandBox::tr("Can't open source path"), status);
	}

	wstring dst_path = L"\\??\\" + DestDir;
	RtlInitUnicodeString(&uni, dst_path.c_str());
	InitializeObjectAttributes(&objattrs, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

	HANDLE dst_handle = NULL; // open destination fodler
	status = NtCreateFile(&dst_handle, FILE_GENERIC_READ, &objattrs, &IoStatusBlock, NULL, 
		Mode == 1 ? 0 : FILE_ATTRIBUTE_NORMAL,												// mode 1 = folder, 0 = file
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 
		FILE_OPEN, 
		FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, 
		NULL, NULL);

	if (!NT_SUCCESS(status)) {
		NtClose(src_handle);
		return SB_ERR(CSandBox::tr("Can't open destination folder"), status);
	}

	// do the rename and retry if needed
	union {
		FILE_RENAME_INFORMATION info;
		WCHAR space[128];
	} u;
	u.info.ReplaceIfExists = FALSE;
	u.info.RootDirectory = dst_handle;
	u.info.FileNameLength = DestName.length() * sizeof(WCHAR);
	wcscpy(u.info.FileName, DestName.c_str());

	for (int retries = 0; retries < 20; retries++)
	{
		status = NtSetInformationFile(src_handle, &IoStatusBlock, &u.info, sizeof(u), FileRenameInformation);
		/*if (status == STATUS_ACCESS_DENIED || status == STATUS_SHARING_VIOLATION)
		{
			// Please terminate programs running in the sandbox before deleting its contents - 3221
		}*/
		if (status != STATUS_SHARING_VIOLATION)
			break;

		Sleep(300);
	}

	NtClose(dst_handle);
	NtClose(src_handle);

	if (!NT_SUCCESS(status)) {
		//SetLastError(RtlNtStatusToDosError(status));
		return SB_ERR(CSandBox::tr("Rename operation failed"), status);
	}
	return SB_OK;
}

SB_STATUS CSandBox::RenameForDelete(const QString& BoxPath, QString& TempPath)
{
	wstring box_path = BoxPath.toStdWString();

	size_t pos = box_path.find_last_of(L'\\');
	wstring box_name = box_path.substr(pos + 1);
	wstring box_root = box_path.substr(0, pos);

	WCHAR temp_name[64];
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	wsprintf(temp_name, L"__Delete_%s_%08X%08X", box_name.c_str(), ft.dwHighDateTime, ft.dwLowDateTime); // maintain a compatible naming convention

	TempPath = QString::fromStdWString(box_root + L"\\" + temp_name);

	SB_STATUS Status = CSandBox__OsRename(box_path, box_root, temp_name, 1);
	if (Status.IsError())
		return SB_ERR(tr("Could not move the sandbox folder out of the way %1").arg(Status.GetText()), Status.GetStatus());
	return SB_OK;
}

bool CSandBox__WaitForFolder(const wstring& folder, int seconds = 10)
{
	NTSTATUS status = STATUS_SUCCESS;
	IO_STATUS_BLOCK IoStatusBlock;

	wstring path = L"\\??\\" + folder;
	UNICODE_STRING uni;
	RtlInitUnicodeString(&uni, path.c_str());

	OBJECT_ATTRIBUTES objattrs;
	InitializeObjectAttributes(&objattrs, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

	for (int retries = 0; retries < seconds * 2; retries++)
	{
		HANDLE handle = NULL;
		status = NtCreateFile(&handle, DELETE | SYNCHRONIZE, &objattrs, &IoStatusBlock, NULL, FILE_ATTRIBUTE_NORMAL,
			0, FILE_OPEN, FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
		if (NT_SUCCESS(status)) {
			NtClose(handle);
			return true;
		}

		Sleep(500);
	}

	return false;
}

bool CSandBox__RemoveFileAttributes(const WCHAR *parent, const WCHAR *child)
{
	IO_STATUS_BLOCK IoStatusBlock;
	NTSTATUS status;

	wstring path = L"\\??\\" + wstring(parent) + L"\\" + wstring(child);
	UNICODE_STRING uni;
	RtlInitUnicodeString(&uni, path.c_str());

	OBJECT_ATTRIBUTES objattrs;
	InitializeObjectAttributes(&objattrs, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

	HANDLE handle = NULL;
	status = NtCreateFile(&handle, FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
		&objattrs, &IoStatusBlock, NULL, 0, 0, FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	if (NT_SUCCESS(status)) 
	{
		union {
			FILE_BASIC_INFORMATION info;
			WCHAR space[128];
		} u;

		status = NtQueryInformationFile(handle, &IoStatusBlock, &u.info, sizeof(u), FileBasicInformation);
		if (NT_SUCCESS(status)) 
		{
			u.info.FileAttributes &= ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
			if (u.info.FileAttributes == 0 || u.info.FileAttributes == FILE_ATTRIBUTE_DIRECTORY)
				u.info.FileAttributes |= FILE_ATTRIBUTE_NORMAL;

			status = NtSetInformationFile(handle, &IoStatusBlock, &u.info, sizeof(u), FileBasicInformation);
		}
	}

	if (handle)
		NtClose(handle);

	//SetLastError(RtlNtStatusToDosError(status));
	return (status == STATUS_SUCCESS);
}

SB_STATUS CSandBox__PrepareForDelete(const wstring& BoxFolder)
{
	static const WCHAR *deviceNames[] = {
		L"aux", L"clock$", L"con", L"nul", L"prn",
		L"com1", L"com2", L"com3", L"com4", L"com5",
		L"com6", L"com7", L"com8", L"com9",
		L"lpt1", L"lpt2", L"lpt3", L"lpt4", L"lpt5",
		L"lpt6", L"lpt7", L"lpt8", L"lpt9",
		NULL
	};

	static const UCHAR valid_chars[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
		"0123456789 ^&@{}[],$=!-#()%.+~_";

	//
	// prepare the removel of the folder, remove all file attributes and junction points.
	// if any path is longer than MAX_PATH shorten it by copying the offending entry to the box folder root
	//

	list<wstring> folders;
	folders.push_back(BoxFolder);

	while (!folders.empty())
	{
		wstring folder = folders.front();
		folders.pop_front();

		HANDLE hFind = NULL;
		
		for (;;)
		{
			WIN32_FIND_DATA data;
			if (hFind == NULL)
			{
				hFind = FindFirstFile((folder + L"\\*").c_str(), &data);
				if (hFind == INVALID_HANDLE_VALUE)
					break;
			}
			else if (!FindNextFile(hFind, &data))
				break;

			WCHAR *name = data.cFileName;
			if (wcscmp(name, L".") == 0 || wcscmp(name, L"..") == 0)
				continue;
			ULONG name_len = wcslen(name);

			// clear problrmativ attributes
			if (data.dwFileAttributes & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM))
				CSandBox__RemoveFileAttributes(folder.c_str(), name);

			// Check if the path is too long
			bool needRename = ((folder.length() + name_len) > 220);
			// Check if the file name matches a DOS device name
			if ((!needRename) && (name_len <= 8)) {
				for (ULONG devNum = 0; deviceNames[devNum]; ++devNum) {
					const WCHAR *devName = deviceNames[devNum];
					ULONG devNameLen = wcslen(devName);
					if (_wcsnicmp(name, devName, devNameLen) == 0) {
						needRename = true;
						break;
					}
				}
			}
			// Check if the file name contains non-ASCII or invalid ASCII characters
			if (!needRename) {
				for (const WCHAR *nameptr = name; *nameptr; ++nameptr) {
					const UCHAR *charptr;
					if (*nameptr >= 0x80) {
						needRename = TRUE;
						break;
					}
					for (charptr = valid_chars; *charptr; ++charptr) {
						if ((UCHAR)*nameptr == *charptr)
							break;
					}
					if (!*charptr) {
						needRename = TRUE;
						break;
					}
				}
			}
			// Check if the file name ends with a dot or a space
			if (!needRename) {
				if (name_len > 1 && (name[name_len - 1] == L'.' || name[name_len - 1] == L' '))
					needRename = TRUE;
			}

			// rename and move the offending file to the box folder root
			if (needRename) 
			{
				FILETIME now;
				GetSystemTimeAsFileTime(&now);
				static ULONG counter = 0;
				WCHAR temp_name[64];
				wsprintf(temp_name, L"%08X-%08X-%08X", now.dwHighDateTime, now.dwLowDateTime, ++counter);

				SB_STATUS Status = CSandBox__OsRename(folder + L"\\" + name, BoxFolder, temp_name, 0);
				if (Status.IsError())
					return Status;
				
				folder = BoxFolder;
				wcscpy(name, temp_name);
				//name_len = wcslen(name);
			}
			
			// If the filesystem item is a folder add it to the processing queue, ...
			if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
			{
				// ... although if the directory is a reparse point delete it right away.
				if (data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) 
				{
					wstring full_path = folder + L"\\" + name;
					if (RemoveDirectory(full_path.c_str()))
						return SB_ERR(CSandBox::tr("Failed to remove a reparse point!"));
				}
				else
					folders.push_front(folder + L"\\" + name);
			}
		}

		if (hFind != INVALID_HANDLE_VALUE)
			FindClose(hFind);
	}

	CSandBox__RemoveFileAttributes(BoxFolder.c_str(), L"");

	return SB_OK;
}

bool CSandBox::CleanBoxAsync(const CSbieProgressPtr& pProgress, const QStringList& BoxFolders, const QString& DeleteCommand)
{
	SB_STATUS Status;

	foreach(const QString& CurFolder, BoxFolders)
	{
		if (!QDir().exists(CurFolder))
			continue;

		wstring BoxFolder = CurFolder.toStdWString();

		pProgress->ShowMessage(tr("Waiting for folder"));

		// Prepare the folder to be deleted
		CSandBox__WaitForFolder(BoxFolder);

		if (pProgress->IsCancel())
			break;

		pProgress->ShowMessage(tr("Preparing for deletion"));
		Status = CSandBox__PrepareForDelete(BoxFolder);
		if (Status.IsError()) {
			Status = SB_ERR(CSandBox::tr("Error renaming one of the long file names in the sandbox: %1. The contents of the sandbox will not be deleted.").arg(Status.GetText()), Status.GetStatus());
			break;
		}
		
		// Prepare and issue the delete command
		CSandBox__WaitForFolder(BoxFolder);

		if (pProgress->IsCancel())
			break;

		pProgress->ShowMessage(tr("Running delete command"));

		QString Cmd = DeleteCommand;
		Cmd.replace("%SANDBOX%", CurFolder); // expand %SANDBOX%

		// Expand other environment variables
		foreach(const QString& key, QProcessEnvironment::systemEnvironment().keys())
			Cmd.replace("%" + key + "%", QProcessEnvironment::systemEnvironment().value(key));

		QProcess Proc;
		Proc.execute(Cmd);
		Proc.waitForFinished();
		if (Proc.exitCode() != 0){
			Status = SB_ERR(CSandBox::tr("Error sandbox delete command returned %1. The contents of the sandbox will not be deleted.").arg(Proc.exitCode()));
			break;
		}
	}

	pProgress->Finish(Status);

	return !Status.IsError();
}
