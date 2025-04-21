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

#include "../Helpers/NtIO.h"

//struct SSandBox
//{
//};

CSandBox::CSandBox(const QString& BoxName, class CSbieAPI* pAPI) : CSbieIni(BoxName, pAPI)
{
	//m = new SSandBox;

	m_IsEnabled = true;

	m_ActiveProcessCount = 0;
	m_ActiveProcessDirty = false;

	// when loading a sandbox that is not initialized, initialize it
	int cfglvl = GetNum("ConfigLevel");
	if (cfglvl >= 10)
		return;

	if (cfglvl == 0)
	{
		SetBool("BlockNetworkFiles", true);

		// recovery
		AppendText("RecoverFolder", "%Desktop%");
		//AppendText("RecoverFolder", "%Favorites%"); // obsolete
		AppendText("RecoverFolder", "%Personal%");
		AppendText("RecoverFolder", "%{374DE290-123F-4565-9164-39C4925E467B}%"); // %USERPROFILE%\Downloads

		SetText("BorderColor", "#00FFFF,ttl"); // "#00FFFF,off"
	}

	if (cfglvl < 6)
	{
		// templates L6
		AppendText("Template", "AutoRecoverIgnore");
		//AppendText("Template", "Firefox_Phishing_DirectAccess");
		//AppendText("Template", "Chrome_Phishing_DirectAccess");
		AppendText("Template", "LingerPrograms");
	}

	if (cfglvl < 7)
	{
		// templates L7
		AppendText("Template", "BlockPorts");
		//AppendText("Template", "WindowsFontCache"); // since 5.46.3 open by driver
		AppendText("Template", "qWave");
	}

	if (cfglvl < 8)
	{
		// templates L8
		AppendText("Template", "FileCopy");
		AppendText("Template", "SkipHook");
	}
	
	if (cfglvl < 9)
	{
		// fix the unfortunate typo
		if (GetTextList("Template", false).contains("FileCppy"))
		{
			AppendText("Template", "FileCopy");
			DelValue("Template", "FileCppy");
		}

		DelValue("Template", "WindowsFontCache");

		// templates L9
		if (GetBool("DropAdminRights", false) == false) 
		{
			// enable those templates only for non hardened boxes
			AppendText("Template", "OpenBluetooth");
		}
	}

	if (cfglvl < 10)
	{
		// starting with 5.62.3 OpenProtectedStorage is a template
		if (GetBool("OpenProtectedStorage")) {
			DelValue("OpenProtectedStorage");
			AppendText("Template", "OpenProtectedStorage");
		}
	}

	SetNum("ConfigLevel", 10);
}

CSandBox::~CSandBox()
{
	//delete m;
}

void CSandBox::UpdateDetails()
{
	if (GetBool("UseRamDisk") || GetBool("UseFileImage"))
	{
		auto res = m_pAPI->ImBoxQuery(m_RegPath);
		if (res.IsError()) {
			m_Mount.clear();
			return;
		}
		QVariantMap Info = res.GetValue();
		m_Mount = Info["DiskRoot"].toString();
	}
	else if(!m_Mount.isEmpty())
		m_Mount.clear();
}

void CSandBox::SetBoxPaths(const QString& FilePath, const QString& RegPath, const QString& IpcPath)
{
	m_FilePath = FilePath;
	m_RegPath = RegPath;
	m_IpcPath = IpcPath;
}

void CSandBox::SetFileRoot(const QString& FilePath)
{
	SetText("FileRootPath", FilePath);
	m_pAPI->UpdateBoxPaths(this);
}

SB_STATUS CSandBox::RunStart(const QString& Command, bool Elevated)
{
/*#ifdef _DEBUG
	if ((QGuiApplication::queryKeyboardModifiers() & Qt::ControlModifier) != 0)
		return RunSandboxed(Command);
#endif*/
	return m_pAPI->RunStart(m_Name, Command, Elevated ? CSbieAPI::eStartElevated : CSbieAPI::eStartDefault);
}

SB_STATUS CSandBox::RunSandboxed(const QString& Command)
{
	return m_pAPI->RunSandboxed(m_Name, Command);
}

SB_STATUS CSandBox::TerminateAll()
{
	return m_pAPI->TerminateAll(m_Name);
}

SB_STATUS CSandBox::SetSuspendedAll(bool bSuspended)
{
	return m_pAPI->SetSuspendedAll(m_Name, bSuspended);
}

void CSandBox::OpenBox()
{
	HANDLE hFile = CreateFileW((LPCWSTR)m_FilePath.utf16(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (hFile != INVALID_HANDLE_VALUE) 
	{
		WCHAR targetPath[MAX_PATH];
		if(GetFinalPathNameByHandleW(hFile, targetPath, MAX_PATH, FILE_NAME_NORMALIZED | VOLUME_NAME_NT))
			m_FileRePath = QString::fromWCharArray(targetPath);
		CloseHandle(hFile);
	}
}

void CSandBox::CloseBox() 
{
	m_FileRePath.clear();
}

bool CSandBox::IsEmpty() const
{
	return !QFile::exists(m_FilePath);
}

SB_PROGRESS CSandBox::CleanBox()
{
	if (GetBool("NeverDelete", false))
		return SB_ERR(SB_DeleteProtect);

	if (GetActiveProcessCount() > 0)
		return SB_ERR(SB_DeleteNotEmpty);

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

	NtIo_WaitForFolder(&ntObject.attr, 10, [](const WCHAR* info, void* param) {
		return !((CSbieProgress*)param)->IsCanceled(); 
	}, pProgress.data());

	if (pProgress->IsCanceled())
		return SB_ERR(SB_DeleteError, QVariantList() << Folder, STATUS_CANCELLED);

	pProgress->ShowMessage(CSandBox::tr("Deleting folder: %1").arg(Folder));

	NTSTATUS status = NtIo_DeleteFolderRecursively(&ntObject.attr, [](const WCHAR* info, void* param) {
		CSbieProgress* pProgress = (CSbieProgress*)param;
		pProgress->ShowMessage(CSandBox::tr("Deleting folder: %1").arg(QString::fromWCharArray(info)));
		return !pProgress->IsCanceled(); 
	}, pProgress.data());

	if (!NT_SUCCESS(status))
		return SB_ERR(SB_DeleteError, QVariantList() << Folder, status);
	return SB_OK;
}

void CSandBox::CleanBoxAsync(const CSbieProgressPtr& pProgress, const QStringList& BoxFolders)
{
	SB_STATUS Status;

	foreach(const QString& Folder, BoxFolders)
	{
		for (int i = 0; i < 10; i++) {
			Status = CSandBox__DeleteFolder(pProgress, Folder);
			if (!Status.IsError() || Status.GetStatus() == STATUS_CANCELLED)
				break;
			
			QThread::sleep(1); // wait a second and retry
		}

		if (Status.IsError())
			break;
	}

	pProgress->Finish(Status);
}

SB_STATUS CSandBox__MoveFolder(const QString& SourcePath, const QString& ParentFolder, const QString& TargetName);

SB_STATUS CSandBox::RenameBox(const QString& NewName)
{
	if (GetActiveProcessCount() > 0)
		return SB_ERR(SB_RemNotStopped);

	if (NewName.compare(m_Name, Qt::CaseInsensitive) == 0)
		return SB_OK;

	SB_STATUS Status = CSbieAPI::ValidateName(NewName);
	if (Status.IsError())
		return Status;

	if (QDir(m_FilePath).exists()) 
	{	
		QStringList FilePath = m_FilePath.split("\\");
		if (FilePath.last().isEmpty()) FilePath.removeLast();
		QString Name = FilePath.takeLast();
		if (Name.compare(m_Name, Qt::CaseInsensitive) == 0) 
		{
			//Status = CSandBox__MoveFolder(m_FilePath, FilePath.join("\\"), NewName);
			//if (Status.IsError())
			//	return Status;
			if(!QDir().rename(m_FilePath, FilePath.join("\\") + "\\" + NewName))
				return SB_ERR(SB_FailedMoveDir, QVariantList() << m_FilePath << (FilePath.join("\\") + "\\" + NewName), 0xC0000001 /*STATUS_UNSUCCESSFUL*/);

			QString FileRootPath = GetText("FileRootPath");
			if (!FileRootPath.isEmpty())
				SetText("FileRootPath", FilePath.join("\\") + "\\%SANDBOX%");
		}
	}

	QString OldName = m_Name;
	Status = RenameSection(NewName);
	if (!Status.IsError()) {
		CSandBoxPtr pBox = m_pAPI->m_SandBoxes.take(OldName.toLower());
		if (pBox)m_pAPI->m_SandBoxes.insert(NewName.toLower(), pBox);
	}
	return Status;
}

SB_STATUS CSandBox::RemoveBox()
{
	if (!IsEmpty())
		return SB_ERR(SB_DelNotEmpty);

	return RemoveSection();
}

QString CSandBox::Expand(const QString& Value)
{
	QString Value2 = Value;

	QRegularExpression rx("%([\\{\\}\\-a-zA-Z0-9 ]+)%");
	for (int pos = 0; ; ) {
		auto result = rx.match(Value, pos);
		if (!result.hasMatch())
			break;
		pos = result.capturedStart();
		QString var = result.captured(1);
		QString val;
		if (var.compare("SbieHome", Qt::CaseInsensitive) == 0)
			val = this->m_pAPI->GetSbiePath();
		else if (var.compare("BoxPath", Qt::CaseInsensitive) == 0)
			val = this->GetFileRoot();
		else if (var.compare("BoxName", Qt::CaseInsensitive) == 0)
			val = this->GetName();
		else
			val = m_pAPI->SbieIniGet(this->GetName(), "%" + var + "%", 0x80000000); // CONF_JUST_EXPAND
		Value2.replace("%" + var + "%", val);
		pos += result.capturedLength();
	}

	return Value2;
}

QMap<QString, SBoxSnapshot> CSandBox::GetSnapshots(QString* pCurrent, QString* pDefault) const
{
	QSettings ini(m_FilePath + "\\Snapshots.ini", QSettings::IniFormat);

	QMap<QString, SBoxSnapshot> Snapshots;

	foreach(const QString& Snapshot, ini.childGroups())
	{
		if (Snapshot.indexOf("Snapshot_") != 0)
			continue;

		SBoxSnapshot BoxSnapshot;
		BoxSnapshot.ID = Snapshot.mid(9);
		BoxSnapshot.Parent = ini.value(Snapshot + "/Parent").toString();

		BoxSnapshot.NameStr = ini.value(Snapshot + "/Name").toString();
		BoxSnapshot.InfoStr = ini.value(Snapshot + "/Description").toString();
		BoxSnapshot.SnapDate = QDateTime::fromSecsSinceEpoch(ini.value(Snapshot + "/SnapshotDate").toULongLong());

		Snapshots.insert(BoxSnapshot.ID, BoxSnapshot);
	}

	if(pCurrent)
		*pCurrent = ini.value("Current/Snapshot").toString();
	if(pDefault)
		*pDefault = ini.value("Current/Default").toString();

	return Snapshots;
}

void CSandBox::SetDefaultSnapshot(QString Default)
{
	QSettings ini(m_FilePath + "\\Snapshots.ini", QSettings::IniFormat);

	ini.setValue("Current/Default", Default);
	ini.sync();
}

QString CSandBox::GetDefaultSnapshot(QString* pCurrent) const
{
	QSettings ini(m_FilePath + "\\Snapshots.ini", QSettings::IniFormat);

	if(pCurrent)
		*pCurrent = ini.value("Current/Snapshot").toString();

	return ini.value("Current/Default").toString();
}

QStringList CSandBox__BoxSubFolders = QStringList() << "drive" << "user" << "share";

struct SBoxDataFile
{
	SBoxDataFile(QString name, bool required, bool recursive) : Name(name), Required(required), Recursive(recursive) {}
	QString Name;
	bool Required; // fail on fail
	bool Recursive;
};

QList<SBoxDataFile> CSandBox__BoxDataFiles = QList<SBoxDataFile>() 
	<< SBoxDataFile("RegHive", true, false) 
	<< SBoxDataFile("RegPaths.dat", false, false) 
	<< SBoxDataFile("FilePaths.dat", false, true)
;

bool CSandBox::IsInitialized() const
{
	if (IsEmpty())
		return false;

	foreach(const QString & BoxSubFolder, CSandBox__BoxSubFolders) {
		if (QDir(m_FilePath + "\\" + BoxSubFolder).exists())
			return true;
	}
	foreach(const SBoxDataFile& BoxDataFile, CSandBox__BoxDataFiles) {
		if (BoxDataFile.Required && QFile::exists(m_FilePath + "\\" + BoxDataFile.Name))
			return true;
	}
	return false;
}

bool CSandBox::HasSnapshots() const
{
	return QFile::exists(m_FilePath + "\\Snapshots.ini");
}

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
		return SB_ERR(SB_SnapIsRunning);

	if (!IsInitialized())
		return SB_ERR(SB_SnapIsEmpty);

	QStringList Snapshots = ini.childGroups();

	QString ID;
	for (int i = 1; ; i++)
	{
		ID = QString::number(i);
		if(!Snapshots.contains("Snapshot_" + ID))
			break;
	}

	if (!QDir().mkpath(m_FilePath + "\\snapshot-" + ID))
		return SB_ERR(SB_SnapMkDirFail);

	foreach(const SBoxDataFile& BoxDataFile, CSandBox__BoxDataFiles) 
	{
		if (!QFile::copy(m_FilePath + "\\" + BoxDataFile.Name, m_FilePath + "\\snapshot-" + ID + "\\" + BoxDataFile.Name)) {
			if (BoxDataFile.Required)
				return SB_ERR(SB_SnapCopyDatFail);
		}
		else if (BoxDataFile.Recursive) // this one is incremental, hence delete it from the copy root, after it was copied to the snapshot
			QFile::remove(m_FilePath + "\\" + BoxDataFile.Name);
	}

	ini.setValue("Snapshot_" + ID + "/Name", Name);
	ini.setValue("Snapshot_" + ID + "/SnapshotDate", QDateTime::currentDateTime().toSecsSinceEpoch());
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
		return SB_ERR(SB_SnapIsRunning);
	
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
		QtConcurrent::run(CSandBox::MergeSnapshotAsync, pProgress, m_FilePath, ID, IsCurrent ? QString() : ChildIDs.first(), qMakePair(m_Name, m_pAPI));
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

	NtIo_WaitForFolder(&ntSource.attr, 10, [](const WCHAR* info, void* param) {
		return !((CSbieProgress*)param)->IsCanceled(); 
	}, pProgress.data());

	if (!QDir().exists(TargetFolder))
		QDir().mkpath(TargetFolder); // just make it
	
	pProgress->ShowMessage(CSandBox::tr("Waiting for folder: %1").arg(TargetFolder));

	SNtObject ntTarget(L"\\??\\" + TargetFolder.toStdWString());

	NtIo_WaitForFolder(&ntTarget.attr, 10, [](const WCHAR* info, void* param) {
		return !((CSbieProgress*)param)->IsCanceled(); 
	}, pProgress.data());

	if (pProgress->IsCanceled())
		return SB_ERR(SB_SnapMergeFail, QVariantList() << TargetFolder << SourceFolder, STATUS_CANCELLED);

	pProgress->ShowMessage(CSandBox::tr("Merging folders: %1 >> %2").arg(SourceFolder).arg(TargetFolder));

	NTSTATUS status = NtIo_MergeFolder(&ntSource.attr, &ntTarget.attr, [](const WCHAR* info, void* param) {
		CSbieProgress* pProgress = (CSbieProgress*)param;
		pProgress->ShowMessage(CSandBox::tr("Merging folder: %1").arg(QString::fromWCharArray(info)));
		return !pProgress->IsCanceled(); 
	}, pProgress.data());

	if (!NT_SUCCESS(status))
		return SB_ERR(SB_SnapMergeFail, QVariantList() << TargetFolder << SourceFolder, status);
	return SB_OK;
}

SB_STATUS CSandBox__CleanupSnapshot(const QString& Folder)
{
	// remove files which may be in the snapshot
	foreach(const SBoxDataFile& BoxDataFile, CSandBox__BoxDataFiles) 
		QFile::remove(Folder + "\\" + BoxDataFile.Name);

	// delete snapshot folder, at this stage it should be empty
	// when its not empty delete will fail
	SNtObject ntSnapshotFile(L"\\??\\" + Folder.toStdWString());
	NTSTATUS status = NtDeleteFile(&ntSnapshotFile.attr);
	if (!NT_SUCCESS(status))
		return SB_ERR(SB_SnapRmDirFail, QVariantList() << Folder, status);
	return SB_OK;
}

void CSandBox__MoveDataFilesSafe(const QString& SourceFolder, const QString& TargetFolder)
{
	foreach(const SBoxDataFile& BoxDataFile, CSandBox__BoxDataFiles)
	{
		if (!QFile::exists(SourceFolder + "\\" + BoxDataFile.Name))
			continue;

		QFile::remove(TargetFolder + "\\" + BoxDataFile.Name);
		QFile::rename(SourceFolder + "\\" + BoxDataFile.Name, TargetFolder + "\\" + BoxDataFile.Name);
	}
}

// path flags, saved to file
#define FILE_DELETED_FLAG       0x0001
#define FILE_RELOCATION_FLAG    0x0002

void CSandBox::MergeSnapshotAsync(const CSbieProgressPtr& pProgress, const QString& BoxPath, const QString& TargetID, const QString& SourceID, const QPair<const QString, class CSbieAPI*>& params)
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

	auto GetBoxedPath = [BoxPath, params](const QString& Path, const QString& TargetFolder) {
		QString SubPath = params.second->GetBoxedPath(params.first, Path).mid(BoxPath.length());
		return TargetFolder + SubPath;
	};

	SB_STATUS Status = SB_OK;

	// apply source FilePaths.dat on the targetfolder
	if (QFile::exists(SourceFolder + "\\FilePaths.dat")) 
	{
		QFile datSource(SourceFolder + "\\FilePaths.dat");
		if (datSource.open(QFile::ReadOnly)) 
		{
			QByteArray datBin = datSource.readAll();

			QStringList datData = QString::fromWCharArray((wchar_t*)datBin.data(), datBin.size() / sizeof(wchar_t)).split("\n");

			// process relocations
			foreach (const QString& Line, datData) {
				QStringList Data = Line.trimmed().split("|");

				QString Path = Data[0];
				if (Path.isEmpty()) continue;
				Path = GetBoxedPath(Path, TargetFolder);
				int Flags = Data.size() >= 2 ? Data[1].toInt() : 0;

				if (Flags & FILE_RELOCATION_FLAG)
				{
					QString Relocation = Data.size() >= 3 ? GetBoxedPath(Data[2], TargetFolder) : QString();
					
					SNtObject ntSrc(L"\\??\\" + Relocation.toStdWString());

					if (NtIo_FileExists(&ntSrc.attr)) {

						SNtObject ntOld(L"\\??\\" + Path.toStdWString());

						NTSTATUS status = NtIo_DeleteFolderRecursively(&ntOld.attr, [](const WCHAR* info, void* param) {
							CSbieProgress* pProgress = (CSbieProgress*)param;
							pProgress->ShowMessage(CSandBox::tr("Deleting folder: %1").arg(QString::fromWCharArray(info)));
							return !pProgress->IsCanceled();
						}, pProgress.data());

						if (NT_SUCCESS(status))
						{
							QStringList PathX = Path.split("\\");
							QString Name = PathX.takeLast();
							SNtObject ntDest(L"\\??\\" + PathX.join("\\").toStdWString());

							status = NtIo_RenameFolder(&ntSrc.attr, &ntDest.attr, Name.toStdWString().c_str());
						}
					}
				}
			}

			// process deletions
			foreach (const QString& Line, datData) {
				QStringList Data = Line.trimmed().split("|");

				QString Path = Data[0];
				if (Path.isEmpty()) continue;
				Path = GetBoxedPath(Path, TargetFolder);
				int Flags = Data.size() >= 2 ? Data[1].toInt() : 0;

				if (Flags & FILE_DELETED_FLAG)
				{
					SNtObject ntPath(L"\\??\\" + Path.toStdWString());

					NTSTATUS status = NtIo_DeleteFile(ntPath, [](const WCHAR* info, void* param) {
						CSbieProgress* pProgress = (CSbieProgress*)param;
						pProgress->ShowMessage(CSandBox::tr("Deleting: %1").arg(QString::fromWCharArray(info)));
						return !pProgress->IsCanceled(); 
					}, pProgress.data());
				}
			}

			// merge DeleteV2 file entries to the Target FilePaths.dat
			QFile datTarget(TargetFolder + "\\FilePaths.dat");
			if (datTarget.open(QFile::ReadWrite)) {
				// merge with target
				datTarget.seek(datTarget.size());
				datTarget.write(datBin);
				datTarget.close();
			}

			// remove source FilePaths.dat
			datSource.close();
			datSource.remove();
		}
	}

	// merge source folders to the target snapshot
	foreach(const QString& BoxSubFolder, CSandBox__BoxSubFolders) 
	{
		Status = CSandBox__MergeFolders(pProgress, TargetFolder + "\\" + BoxSubFolder, SourceFolder + "\\" + BoxSubFolder);
		if (Status.IsError())
			break;
	}

	pProgress->ShowMessage(CSandBox::tr("Finishing Snapshot Merge..."));

	if(!Status.IsError())
	{
		// copy other data files from source to target
		CSandBox__MoveDataFilesSafe(SourceFolder, TargetFolder);

		if (IsCurrent)
		{
			// move all folders out of the snapshot to root
			foreach(const QString& BoxSubFolder, CSandBox__BoxSubFolders) 
			{
				Status = CSandBox__MoveFolder(TargetFolder + "\\" + BoxSubFolder, SourceFolder, BoxSubFolder);
				if (Status.IsError())
					break;
			}

			// move all data files out of the snapshot to root
			CSandBox__MoveDataFilesSafe(TargetFolder, SourceFolder);
			
			// delete snapshot rest
			if (!Status.IsError())
				Status = CSandBox__CleanupSnapshot(TargetFolder);
		}
		else
		{
			// delete rest of source snpshot
			Status = CSandBox__CleanupSnapshot(SourceFolder);

			// rename target snapshot to source snapshot
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

	if (!ID.isEmpty() && !ini.childGroups().contains("Snapshot_" + ID))
		return SB_ERR(SB_SnapNotFound);

	if (m_pAPI->HasProcesses(m_Name))
		return SB_ERR(SB_SnapIsRunning);

	foreach(const SBoxDataFile& BoxDataFile, CSandBox__BoxDataFiles)
	{
		QFile::remove(m_FilePath + "\\" + BoxDataFile.Name);

		if (ID.isEmpty() || BoxDataFile.Recursive)
			continue; // this one is incremental, don't restore it

		if (!QFile::copy(m_FilePath + "\\snapshot-" + ID + "\\" + BoxDataFile.Name, m_FilePath + "\\" + BoxDataFile.Name)) {
			if (BoxDataFile.Required)
				return SB_ERR(SB_SnapCopyDatFail);
		}
	}

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

SB_STATUS CSandBox::ImBoxCreate(quint64 uSizeKb, const QString& Password)
{
	return m_pAPI->ImBoxCreate(this, uSizeKb, Password);
}

SB_STATUS CSandBox::ImBoxMount(const QString& Password, bool bProtect, bool bAutoUnmount)
{
	return m_pAPI->ImBoxMount(this, Password, bProtect, bAutoUnmount);
}

SB_STATUS CSandBox::ImBoxUnmount()
{
	return m_pAPI->ImBoxUnmount(this);
}
