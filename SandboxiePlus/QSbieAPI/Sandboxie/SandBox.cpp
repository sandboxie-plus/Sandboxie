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

#include "Helpers/NtIO.h"

//struct SSandBox
//{
//};

CSandBox::CSandBox(const QString& BoxName, class CSbieAPI* pAPI) : CSbieIni(BoxName, pAPI)
{
	//m = new SSandBox;

	m_IsEnabled = true;

	m_ActiveProcessCount = 0;

	// when loading a sandbox that is not initialized, initialize it
	int cfglvl = GetNum("ConfigLevel");
	if (cfglvl >= 9)
		return;

	if (cfglvl == 0)
	{
		SetBool("AutoRecover", false);
		SetBool("BlockNetworkFiles", true);

		// recovery
		InsertText("RecoverFolder", "%Desktop%");
		//InsertText("RecoverFolder", "%Favorites%"); // obsolete
		InsertText("RecoverFolder", "%Personal%");
		InsertText("RecoverFolder", "%{374DE290-123F-4565-9164-39C4925E467B}%"); // %USERPROFILE%\Downloads

		SetText("BorderColor", "#00FFFF,ttl"); // "#00FFFF,off"
	}

	if (cfglvl < 6)
	{
		// templates L6
		InsertText("Template", "AutoRecoverIgnore");
		InsertText("Template", "Firefox_Phishing_DirectAccess");
		InsertText("Template", "Chrome_Phishing_DirectAccess");
		InsertText("Template", "LingerPrograms");
	}

	if (cfglvl < 7)
	{
		// templates L7
		InsertText("Template", "BlockPorts");
		//InsertText("Template", "WindowsFontCache"); // since 5.46.3 open by driver
		InsertText("Template", "qWave");
	}

	if (cfglvl < 8)
	{
		// templates L8
		InsertText("Template", "FileCopy");
		InsertText("Template", "SkipHook");
	}
	
	if (cfglvl < 9)
	{
		// fix the unfortunate typo
		if (GetTextList("Template", false).contains("FileCppy"))
		{
			InsertText("Template", "FileCopy");
			DelValue("Template", "FileCppy");
		}

		DelValue("Template", "WindowsFontCache");

		// templates L9
		if (GetBool("DropAdminRights", false) == false) 
		{
			// enable those templates only for non hardened boxes
			InsertText("Template", "OpenBluetooth");
		}
	}

	SetNum("ConfigLevel", 9);
}

CSandBox::~CSandBox()
{
	//delete m;
}

void CSandBox::UpdateDetails()
{
}

SB_STATUS CSandBox::RunStart(const QString& Command, bool Elevated)
{
	return m_pAPI->RunStart(m_Name, Command, NULL, Elevated);
}

SB_STATUS CSandBox::RunSandboxed(const QString& Command)
{
	return m_pAPI->RunSandboxed(m_Name, Command);
}

SB_STATUS CSandBox::TerminateAll()
{
	return m_pAPI->TerminateAll(m_Name);
}

bool CSandBox::IsEmpty() const
{
	return !QDir(m_FilePath).exists();
}

SB_PROGRESS CSandBox::CleanBox()
{
	if (GetBool("NeverDelete", false))
		return SB_ERR(SB_DeleteProtect);

	SB_STATUS Status = TerminateAll();
	if (Status.IsError())
		return Status;

	return CleanBoxFolders(QStringList(m_FilePath));
}

SB_PROGRESS CSandBox::CleanBoxFolders(const QStringList& BoxFolders)
{
	CSbieProgressPtr pProgress = CSbieProgressPtr(new CSbieProgress());
	QtConcurrent::run(CSandBox::CleanBoxAsync, pProgress, BoxFolders);
	return SB_PROGRESS(OP_ASYNC, pProgress);
}

SB_STATUS CSandBox__DeleteFolder(const CSbieProgressPtr& pProgress, const QString& Folder)
{
	if (!QDir().exists(Folder))
		return SB_OK;

	pProgress->ShowMessage(CSandBox::tr("Waiting for folder: %1").arg(Folder));

	SNtObject ntObject(L"\\??\\" + Folder.toStdWString());

	NtIo_WaitForFolder(&ntObject.attr);

	if (pProgress->IsCanceled())
		return STATUS_REQUEST_ABORTED; // or STATUS_TRANSACTION_ABORTED ?

	pProgress->ShowMessage(CSandBox::tr("Deleting folder: %1").arg(Folder));

	NTSTATUS status = NtIo_DeleteFolderRecursively(&ntObject.attr);
	if (!NT_SUCCESS(status))
		return SB_ERR(SB_DeleteError, QVariantList() << Folder, status);
	return SB_OK;
}

void CSandBox::CleanBoxAsync(const CSbieProgressPtr& pProgress, const QStringList& BoxFolders)
{
	SB_STATUS Status;

	foreach(const QString& Folder, BoxFolders)
	{
		Status = CSandBox__DeleteFolder(pProgress, Folder);
		if (Status.IsError())
			break;
	}

	pProgress->Finish(Status);
}

SB_STATUS CSandBox::RenameBox(const QString& NewName)
{
	if (!IsEmpty())
		return SB_ERR(SB_RemNotEmpty);

	SB_STATUS Status = CSbieAPI::ValidateName(NewName);
	if (Status.IsError())
		return Status;
	
	return RenameSection(NewName);
}

SB_STATUS CSandBox::RemoveBox()
{
	if (!IsEmpty())
		return SB_ERR(SB_DelNotEmpty);

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
		BoxSnapshot.SnapDate = QDateTime::fromTime_t(ini.value(Snapshot + "/SnapshotDate").toULongLong());

		Snapshots.append(BoxSnapshot);
	}

	if(pCurrent)
		*pCurrent = ini.value("Current/Snapshot").toString();

	return Snapshots;
}

QStringList CSandBox__BoxSubFolders = QStringList() << "drive" << "user" << "share";

SB_STATUS CSandBox__MoveFolder(const QString& SourcePath, const QString& ParentFolder, const QString& TargetName)
{
	SNtObject src_dir(L"\\??\\" + SourcePath.toStdWString());
	SNtObject dest_dir(L"\\??\\" + ParentFolder.toStdWString());
	NTSTATUS status = NtIo_RenameFolder(&src_dir.attr, &dest_dir.attr, TargetName.toStdWString().c_str());
	if (!NT_SUCCESS(status) && status != STATUS_OBJECT_NAME_NOT_FOUND && status != STATUS_OBJECT_PATH_NOT_FOUND)
		return SB_ERR(SB_FailedMoveDir, QVariantList() <<SourcePath << (ParentFolder + "\\" + TargetName), status);
	return SB_OK;
}

SB_PROGRESS CSandBox::TakeSnapshot(const QString& Name)
{
	QSettings ini(m_FilePath + "\\Snapshots.ini", QSettings::IniFormat);

	if (m_pAPI->HasProcesses(m_Name))
		return SB_ERR(SB_SnapIsRunning, OP_CONFIRM);

	if (IsEmpty())
		return SB_ERR(SB_SnapIsEmpty);

	QStringList Snapshots = ini.childGroups();

	QString ID;
	for (int i = 1; ; i++)
	{
		ID = QString::number(i);
		if(!Snapshots.contains("Snapshot_" + ID))
			break;
	}

	if (!QDir().mkdir(m_FilePath + "\\snapshot-" + ID))
		return SB_ERR(SB_SnapMkDirFail);
	if (!QFile::copy(m_FilePath + "\\RegHive", m_FilePath + "\\snapshot-" + ID + "\\RegHive"))
		return SB_ERR(SB_SnapCopyRegFail);

	ini.setValue("Snapshot_" + ID + "/Name", Name);
	ini.setValue("Snapshot_" + ID + "/SnapshotDate", QDateTime::currentDateTime().toTime_t());
	QString Current = ini.value("Current/Snapshot").toString();
	if(!Current.isEmpty())
		ini.setValue("Snapshot_" + ID + "/Parent", Current);

	ini.setValue("Current/Snapshot", ID);
	ini.sync();

	foreach(const QString& BoxSubFolder, CSandBox__BoxSubFolders) 
	{
		SB_STATUS Status = CSandBox__MoveFolder(m_FilePath + "\\" + BoxSubFolder, m_FilePath + "\\snapshot-" + ID, BoxSubFolder);
		if (Status.IsError())
			return Status;
	}
	return SB_OK;
}

SB_PROGRESS CSandBox::RemoveSnapshot(const QString& ID)
{
	QSettings ini(m_FilePath + "\\Snapshots.ini", QSettings::IniFormat);

	if (!ini.childGroups().contains("Snapshot_" + ID))
		return SB_ERR(SB_SnapNotFound);

	if (m_pAPI->HasProcesses(m_Name))
		return SB_ERR(SB_SnapIsRunning, OP_CONFIRM);
	
	QStringList ChildIDs;
	foreach(const QString& Snapshot, ini.childGroups())
	{
		if (Snapshot.indexOf("Snapshot_") != 0)
			continue;

		if (ini.value(Snapshot + "/Parent").toString() == ID)
			ChildIDs.append(Snapshot.mid(9));
	}

	QString Current = ini.value("Current/Snapshot").toString();
	bool IsCurrent = Current == ID;

	if (ChildIDs.count() >= 2 || (ChildIDs.count() == 1 && IsCurrent))
		return SB_ERR(SB_SnapIsShared);

	CSbieProgressPtr pProgress = CSbieProgressPtr(new CSbieProgress());
	if (ChildIDs.count() == 1 || IsCurrent)
		QtConcurrent::run(CSandBox::MergeSnapshotAsync, pProgress, m_FilePath, ID, IsCurrent ? QString() : ChildIDs.first());
	else
		QtConcurrent::run(CSandBox::DeleteSnapshotAsync, pProgress, m_FilePath, ID);
	return SB_PROGRESS(OP_ASYNC, pProgress);
}

void CSandBox::DeleteSnapshotAsync(const CSbieProgressPtr& pProgress, const QString& BoxPath, const QString& ID)
{
	SB_STATUS Status = CSandBox__DeleteFolder(pProgress, BoxPath + "\\snapshot-" + ID);

	if (!Status.IsError())
	{
		QSettings ini(BoxPath + "\\Snapshots.ini", QSettings::IniFormat);

		ini.remove("Snapshot_" + ID);
		ini.sync();
	}

	pProgress->Finish(Status);
}

SB_STATUS CSandBox__MergeFolders(const CSbieProgressPtr& pProgress, const QString& TargetFolder, const QString& SourceFolder)
{
	if (!QDir().exists(SourceFolder))
		return SB_OK; // nothing to do

	pProgress->ShowMessage(CSandBox::tr("Waiting for folder: %1").arg(SourceFolder));

	SNtObject ntSource(L"\\??\\" + SourceFolder.toStdWString());

	NtIo_WaitForFolder(&ntSource.attr);

	if (!QDir().exists(TargetFolder))
		QDir().mkpath(TargetFolder); // just make it
	
	pProgress->ShowMessage(CSandBox::tr("Waiting for folder: %1").arg(TargetFolder));

	SNtObject ntTarget(L"\\??\\" + TargetFolder.toStdWString());

	NtIo_WaitForFolder(&ntTarget.attr);

	if (pProgress->IsCanceled())
		return STATUS_REQUEST_ABORTED; // or STATUS_TRANSACTION_ABORTED ?

	pProgress->ShowMessage(CSandBox::tr("Merging folders: %1 >> %2").arg(SourceFolder).arg(TargetFolder));

	NTSTATUS status = NtIo_MergeFolder(&ntSource.attr, &ntTarget.attr);
	if (!NT_SUCCESS(status))
		return SB_ERR(SB_SnapMergeFail, QVariantList() << TargetFolder << SourceFolder, status);
	return SB_OK;
}

SB_STATUS CSandBox__CleanupSnapshot(const QString& Folder)
{
	SNtObject ntHiveFile(L"\\??\\" + (Folder + "\\RegHive").toStdWString());
	SB_STATUS status = NtDeleteFile(&ntHiveFile.attr);
	if (NT_SUCCESS(status)) {
		SNtObject ntSnapshotFile(L"\\??\\" + Folder.toStdWString());
		status = NtDeleteFile(&ntSnapshotFile.attr);
	}
	if (!NT_SUCCESS(status))
		return SB_ERR(SB_SnapRmDirFail, QVariantList() << Folder, status);
	return SB_OK;
}

void CSandBox::MergeSnapshotAsync(const CSbieProgressPtr& pProgress, const QString& BoxPath, const QString& TargetID, const QString& SourceID)
{
	//
	// Targe is to be removed;
	// Source is the child snpshot that has to remain
	// we merge target with source by overwrite target with source
	// than we rename target to source
	// finally we adapt the ini
	//

	bool IsCurrent = SourceID.isEmpty();
	QString SourceFolder = IsCurrent ? BoxPath : (BoxPath + "\\snapshot-" + SourceID);
	QString TargetFolder = BoxPath + "\\snapshot-" + TargetID;

	SB_STATUS Status = SB_OK;
	
	foreach(const QString& BoxSubFolder, CSandBox__BoxSubFolders) 
	{
		Status = CSandBox__MergeFolders(pProgress, TargetFolder + "\\" + BoxSubFolder, SourceFolder + "\\" + BoxSubFolder);
		if (Status.IsError())
			break;
	}

	pProgress->ShowMessage(CSandBox::tr("Finishing Snapshot Merge..."));

	if(!Status.IsError())
	{
		if (IsCurrent)
		{
			// move all folders out of the snapshot to root
			foreach(const QString& BoxSubFolder, CSandBox__BoxSubFolders) 
			{
				Status = CSandBox__MoveFolder(TargetFolder + "\\" + BoxSubFolder, SourceFolder, BoxSubFolder);
				if (Status.IsError())
					break;
			}

			// delete snapshot rest
			if (!Status.IsError())
				Status = CSandBox__CleanupSnapshot(TargetFolder);
		}
		else
		{
			// delete rest of source snpshot
			Status = CSandBox__CleanupSnapshot(SourceFolder);

			// rename target snapshot o source snapshot
			if (!Status.IsError())
				Status = CSandBox__MoveFolder(TargetFolder, BoxPath, "snapshot-" + SourceID);
		}
	}

	// save changes to the ini
	if (!Status.IsError())
	{
		QSettings ini(BoxPath + "\\Snapshots.ini", QSettings::IniFormat);

		QString TargetParent = ini.value("Snapshot_" + TargetID + "/Parent").toString();
		if (IsCurrent)
			ini.setValue("Current/Snapshot", TargetParent);
		else
			ini.setValue("Snapshot_" + SourceID + "/Parent", TargetParent);

		ini.remove("Snapshot_" + TargetID);
		ini.sync();
	}

	pProgress->Finish(Status);
}

SB_PROGRESS CSandBox::SelectSnapshot(const QString& ID)
{
	QSettings ini(m_FilePath + "\\Snapshots.ini", QSettings::IniFormat);

	if (!ini.childGroups().contains("Snapshot_" + ID))
		return SB_ERR(SB_SnapNotFound);

	if (m_pAPI->HasProcesses(m_Name))
		return SB_ERR(SB_SnapIsRunning, OP_CONFIRM);

	if (!QFile::remove(m_FilePath + "\\RegHive"))
		return SB_ERR(SB_SnapDelRegFail);
	if (!QFile::copy(m_FilePath + "\\snapshot-" + ID + "\\RegHive", m_FilePath + "\\RegHive"))
		return SB_ERR(SB_SnapCopyRegFail);

	ini.setValue("Current/Snapshot", ID);
	ini.sync();

	QStringList BoxFolders;
	foreach(const QString& BoxSubFolder, CSandBox__BoxSubFolders)
		BoxFolders.append(m_FilePath + "\\" + BoxSubFolder);
	return CleanBoxFolders(BoxFolders);
}

SB_STATUS CSandBox::SetSnapshotInfo(const QString& ID, const QString& Name, const QString& Description)
{
	QSettings ini(m_FilePath + "\\Snapshots.ini", QSettings::IniFormat);

	if (!ini.childGroups().contains("Snapshot_" + ID))
		return SB_ERR(SB_SnapNotFound);

	if (!Name.isNull())
		ini.setValue("Snapshot_" + ID + "/Name", Name);
	if (!Description.isNull())
		ini.setValue("Snapshot_" + ID + "/Description", Description);

	return SB_OK;
}
