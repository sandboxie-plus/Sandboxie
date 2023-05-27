#include "stdafx.h"
#include "SbiePlusAPI.h"
#include "SbieProcess.h"
#include "SandMan.h"
#include "..\MiscHelpers\Common\Common.h"
#include <windows.h>
#include <Shlobj_core.h>
#include "BoxMonitor.h"
#include "..\MiscHelpers\Common\OtherFunctions.h"
#include "../QSbieAPI/SbieUtils.h"
#include "../MiscHelpers/Archive/Archive.h"
#include <QtConcurrent>
#include "Helpers/WinHelper.h"

CSbiePlusAPI::CSbiePlusAPI(QObject* parent) : CSbieAPI(parent)
{
	m_BoxMonitor = new CBoxMonitor();

	m_JobCount = 0;
}

CSbiePlusAPI::~CSbiePlusAPI()
{
	delete m_BoxMonitor;
}

CSandBox* CSbiePlusAPI::NewSandBox(const QString& BoxName, class CSbieAPI* pAPI)
{
	return new CSandBoxPlus(BoxName, pAPI);
}

CBoxedProcess* CSbiePlusAPI::NewBoxedProcess(quint32 ProcessId, class CSandBox* pBox)
{
	return new CSbieProcess(ProcessId, pBox);
}

CBoxedProcessPtr CSbiePlusAPI::OnProcessBoxed(quint32 ProcessId, const QString& Path, const QString& Box, quint32 ParentId, const QString& CmdLine)
{
	CBoxedProcessPtr pProcess = CSbieAPI::OnProcessBoxed(ProcessId, Path, Box, ParentId, CmdLine);
	if (!pProcess.isNull() && pProcess->GetFileName().indexOf(theAPI->GetSbiePath(), 0, Qt::CaseInsensitive) != 0) {
		CSandBoxPlus* pBox = pProcess.objectCast<CSbieProcess>()->GetBox();
		pBox->m_RecentPrograms.insert(pProcess->GetProcessName());
	}
	return pProcess;
}

BOOL CALLBACK CSbiePlusAPI__WindowEnum(HWND hwnd, LPARAM lParam)
{
	if (GetParent(hwnd) || GetWindow(hwnd, GW_OWNER))
		return TRUE;
	ULONG style = GetWindowLong(hwnd, GWL_STYLE);
	if ((style & (WS_CAPTION | WS_SYSMENU)) != (WS_CAPTION | WS_SYSMENU))
		return TRUE;
	if (!IsWindowVisible(hwnd))
		return TRUE;
	/*
	if ((style & WS_OVERLAPPEDWINDOW) != WS_OVERLAPPEDWINDOW &&
		(style & WS_POPUPWINDOW)      != WS_POPUPWINDOW)
		return TRUE;
	*/

	ULONG pid;
	GetWindowThreadProcessId(hwnd, &pid);

	QMultiMap<quint32, QString>& m_WindowMap = *((QMultiMap<quint32, QString>*)(lParam));

	WCHAR title[256];
	GetWindowTextW(hwnd, title, 256);

	m_WindowMap.insert(pid, QString::fromWCharArray(title));

	return TRUE;
}

void CSbiePlusAPI::UpdateWindowMap()
{
	m_WindowMap.clear();
	EnumWindows(CSbiePlusAPI__WindowEnum, (LPARAM)&m_WindowMap);
}

bool CSbiePlusAPI::IsRunningAsAdmin()
{
	if (m_UserSid.left(9) != "S-1-5-21-")
		return false;
	if (m_UserSid.right(4) != "-500")
		return false;
	return true;
}

void CSbiePlusAPI::StopMonitor()
{
	m_BoxMonitor->Stop();
}

SB_STATUS CSbiePlusAPI::RunStart(const QString& BoxName, const QString& Command, bool Elevated, const QString& WorkingDir, QProcess* pProcess)
{
	if (!pProcess)
		pProcess = new QProcess(this);
	SB_STATUS Status = CSbieAPI::RunStart(BoxName, Command, Elevated, WorkingDir, pProcess);
	if (pProcess->parent() == this) {
		if (!Status.IsError()) {
			connect(pProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(OnStartFinished()));
			m_PendingStarts.insert(pProcess->processId());
			return SB_OK;
		}
		delete pProcess;
	}
	return Status;
}

void CSbiePlusAPI::OnStartFinished()
{
	QProcess* pProcess = (QProcess*)sender();
	m_PendingStarts.remove(pProcess->processId());
	pProcess->deleteLater();
}

///////////////////////////////////////////////////////////////////////////////
// CSandBoxPlus
//

CSandBoxPlus::CSandBoxPlus(const QString& BoxName, class CSbieAPI* pAPI) : CSandBox(BoxName, pAPI)
{
	m_bLogApiFound = false;
	m_bINetBlocked = false;
	m_bINetExceptions = false;
	m_bSharesAllowed = false;
	m_bDropRights = false;
	

	m_bSecurityEnhanced = false;
	m_bPrivacyEnhanced = false;
	m_bApplicationCompartment = false;
	m_iUnsecureDebugging = 0;
	m_bRootAccessOpen = false;

	m_TotalSize = theConf->GetValue("SizeCache/" + m_Name, -1).toLongLong();

	m_SuspendRecovery = false;
	m_IsEmpty = false;

	m_pOptionsWnd = NULL;
	m_pRecoveryWnd = NULL;

	m_BoxType = eDefault;
	m_BoxDel = false;
	m_NoForce = false;
	m_BoxColor = QColor(Qt::yellow).rgb();
}

CSandBoxPlus::~CSandBoxPlus()
{
}

class QFileX : public QFile {
public:
	QFileX(const QString& path, const CSbieProgressPtr& pProgress, CArchive* pArchive) : QFile(path) 
	{
		m_pProgress = pProgress;
		m_pArchive = pArchive;
	}

	bool open(OpenMode flags) override
	{
		if (m_pProgress->IsCanceled())
			return false;
		m_pProgress->ShowMessage(Split2(fileName(), "/", true).second);
		m_pProgress->SetProgress((int)(m_pArchive->GetProgress() * 100.0));
		return QFile::open(flags);
	}

	qint64 size() const override
	{
		qint64 Size = QFile::size();
		if (QFileInfo(fileName()).isShortcut())
		{
			QFile File(fileName());
			if (File.open(QFile::ReadOnly))
				Size = File.size();
			File.close();
		}
		return Size;
	}

protected:
	CSbieProgressPtr m_pProgress;
	CArchive* m_pArchive;
};

void CSandBoxPlus::ExportBoxAsync(const CSbieProgressPtr& pProgress, const QString& ExportPath, const QString& RootPath, const QString& Section)
{
	//CArchive Archive(ExportPath + ".tmp");
	CArchive Archive(ExportPath);

	QMap<int, QIODevice*> Files;

	//QTemporaryFile* pConfigFile = new QTemporaryFile;
	//pConfigFile->open();
	//pConfigFile->write(Section.toUtf8());
	//pConfigFile->close();
	//
	//int ArcIndex = Archive.AddFile("BoxConfig.ini");
	//Files.insert(ArcIndex, pConfigFile);

	QFile File(RootPath + "\\" + "BoxConfig.ini");
	if (File.open(QFile::WriteOnly)) {
		File.write(Section.toUtf8());
		File.close();
	}

	QStringList FileList = ListDir(RootPath + "\\");
	foreach(const QString& File, FileList)
	{
		StrPair RootName = Split2(File, "\\", true);
		if(RootName.second.isEmpty())
		{
			RootName.second = RootName.first;
			RootName.first = "";
		}
		int ArcIndex = Archive.AddFile(RootName.second);
		if(ArcIndex != -1)
		{
			QString FileName = (RootName.first.isEmpty() ?  RootPath + "\\" : RootName.first) + RootName.second;
			Files.insert(ArcIndex, new QFileX(FileName, pProgress, &Archive));
		}
		//else
			// this file is already present in the archive, this should not happen !!!
	}

	SB_STATUS Status = SB_OK;
	if (!Archive.Update(&Files)) 
		Status = SB_ERR((ESbieMsgCodes)SBX_7zCreateFailed);
	
	//if(!Status.IsError() && !pProgress->IsCanceled())
	//	QFile::rename(ExportPath + ".tmp", ExportPath);
	//else
	//	QFile::remove(ExportPath + ".tmp");

	File.remove();

	pProgress->Finish(Status);
}

SB_PROGRESS CSandBoxPlus::ExportBox(const QString& FileName)
{
	if (!CArchive::IsInit())
		return SB_ERR((ESbieMsgCodes)SBX_7zNotReady);

	if (theAPI->HasProcesses(m_Name))
		return SB_ERR(SB_SnapIsRunning); // todo

	if (!IsInitialized())
		return SB_ERR(SB_SnapIsEmpty); // todo

	QString Section = theAPI->SbieIniGetEx(m_Name, "");

	CSbieProgressPtr pProgress = CSbieProgressPtr(new CSbieProgress());
	QtConcurrent::run(CSandBoxPlus::ExportBoxAsync, pProgress, FileName, m_FilePath, Section);
	return SB_PROGRESS(OP_ASYNC, pProgress);
}

void CSandBoxPlus::ImportBoxAsync(const CSbieProgressPtr& pProgress, const QString& ImportPath, const QString& RootPath, const QString& BoxName)
{
	CArchive Archive(ImportPath);

	if (Archive.Open() != 1) {
		pProgress->Finish(SB_ERR((ESbieMsgCodes)SBX_7zOpenFailed));
		return;
	}

	bool IsBoxArchive = false;

	QMap<int, QIODevice*> Files;

	for (int i = 0; i < Archive.FileCount(); i++) {
		int ArcIndex = Archive.FindByIndex(i);
		if(Archive.FileProperty(ArcIndex, "IsDir").toBool())
			continue;
		QString File = Archive.FileProperty(ArcIndex, "Path").toString();
		if (File == "BoxConfig.ini")
			IsBoxArchive = true;
		Files.insert(ArcIndex, new QFileX(CArchive::PrepareExtraction(File, RootPath + "\\"), pProgress, &Archive));
	}

	if(!IsBoxArchive) {
		pProgress->Finish(SB_ERR((ESbieMsgCodes)SBX_NotBoxArchive));
		return;
	}

	SB_STATUS Status = SB_OK;
	if (!Archive.Extract(&Files))
		Status = SB_ERR((ESbieMsgCodes)SBX_7zExtractFailed);

	if (!Status.IsError() && !pProgress->IsCanceled())
	{
		QFile File(RootPath + "\\" + "BoxConfig.ini");
		if (File.open(QFile::ReadOnly)) {

			QMetaObject::invokeMethod(theAPI, "SbieIniSetSection", Qt::BlockingQueuedConnection, // run this in the main thread
				Q_ARG(QString, BoxName),
				Q_ARG(QString, QString::fromUtf8(File.readAll()))
			);

			File.close();
		}
		File.remove();
	}

	pProgress->Finish(Status);
}

SB_PROGRESS CSandBoxPlus::ImportBox(const QString& FileName)
{
	if (!CArchive::IsInit())
		return SB_ERR((ESbieMsgCodes)SBX_7zNotReady);

	CSbieProgressPtr pProgress = CSbieProgressPtr(new CSbieProgress());
	QtConcurrent::run(CSandBoxPlus::ImportBoxAsync, pProgress, FileName, m_FilePath, m_Name);
	return SB_PROGRESS(OP_ASYNC, pProgress);
}

void CSandBoxPlus::UpdateDetails()
{
	//m_bLogApiFound = GetTextList("OpenPipePath", false).contains("\\Device\\NamedPipe\\LogAPI");
	m_bLogApiFound = false;
	QStringList InjectDlls = GetTextList("InjectDll", false);
	foreach(const QString & InjectDll, InjectDlls) {
		if (InjectDll.contains("logapi", Qt::CaseInsensitive)) {
			m_bLogApiFound = true;
			break;
		}
	}

	m_bINetBlocked = false;
	foreach(const QString& Entry, GetTextList("ClosedFilePath", false)) {
		if (Entry == "!<InternetAccess>,InternetAccessDevices") {
			m_bINetBlocked = true;
			break;
		}
	}
	if (theGUI->IsWFPEnabled()) {
		foreach(const QString & Entry, GetTextList("AllowNetworkAccess", false)) {
			if (Entry == "!<InternetAccess>,n") {
				m_bINetBlocked = true;
				break;
			}
		}
	}
	if (m_bINetBlocked) {
		foreach(const QString& Entry, GetTextList("ProcessGroup", true)) {
			StrPair NameList = Split2(Entry, ",");
			if (NameList.first == "<InternetAccess>" && !NameList.second.isEmpty()) {
				m_bINetExceptions = true;
				break;
			}
		}
	}

	m_bSharesAllowed = GetBool("BlockNetworkFiles", true) == false;

	m_bDropRights = GetBool("DropAdminRights", false);

	m_bRootAccessOpen = false;
	foreach(const QString& Setting, QString("OpenFilePath|OpenKeyPath|OpenPipePath|OpenConfPath").split("|")) {
		foreach(const QString& Entry, GetTextList(Setting, false)) {
			if (Entry == "*" || Entry == "\\") {
				m_bRootAccessOpen = true;
				break;
			}
		}
	}

	if (CheckUnsecureConfig())
		m_iUnsecureDebugging = 1;
	else if(GetBool("ExposeBoxedSystem", false) || GetBool("UnrestrictedSCM", false) /*|| GetBool("RunServicesAsSystem", false)*/)
		m_iUnsecureDebugging = 2;
	else
		m_iUnsecureDebugging = 0;

	//GetBool("SandboxieLogon", false)

	m_bSecurityEnhanced = m_iUnsecureDebugging == 0 && (GetBool("UseSecurityMode", false));
	m_bApplicationCompartment = GetBool("NoSecurityIsolation", false);
	m_bPrivacyEnhanced = !m_bRootAccessOpen && (m_iUnsecureDebugging != 1 || m_bApplicationCompartment) && (GetBool("UsePrivacyMode", false)); // app compartments are inhenrently insecure

	CSandBox::UpdateDetails();

	m_BoxType = GetTypeImpl();

	m_BoxDel = GetBool("AutoDelete", false);
	m_NoForce = GetBool("DisableForceRules", false);

	QStringList BorderCfg = GetText("BorderColor").split(",");
	m_BoxColor = QColor("#" + BorderCfg[0].mid(5, 2) + BorderCfg[0].mid(3, 2) + BorderCfg[0].mid(1, 2)).rgb();
}

bool CSandBoxPlus::IsBoxexPath(const QString& Path)
{
	return Path.left(m_FilePath.length()).compare(m_FilePath, Qt::CaseInsensitive) == 0;
}

bool CSandBoxPlus::IsFileDeleted(const QString& RealPath, const QString& Snapshot, const QStringList& SnapshotList, const QMap<QString, QList<QString>>& DeletedPaths)
{
	int NextIndex = SnapshotList.indexOf(Snapshot) - 1;
	if (NextIndex < 0) return false; // no newer snapshot

	QString NewerSnapshot = SnapshotList.at(NextIndex);

	foreach(const QString &DeletedPath, DeletedPaths[NewerSnapshot]) {
		int len = DeletedPath.length();
		if (RealPath.length() >= len && RealPath.left(len).compare(DeletedPath, Qt::CaseInsensitive) == 0) {
			if (RealPath.length() == len || RealPath[len] == '\\')
				return true;
		}
	}

	return IsFileDeleted(RealPath, NewerSnapshot, SnapshotList, DeletedPaths);
}

void CSandBoxPlus::ScanStartMenu()
{
	QStringList SnapshotList;
	SnapshotList.append("");

	QString CurSnapshot;
	QMap<QString, SBoxSnapshot> Snapshots = GetSnapshots(&CurSnapshot);
	while(!CurSnapshot.isEmpty()) {
		SnapshotList.append(CurSnapshot);
		CurSnapshot = Snapshots[CurSnapshot].Parent;
	}

	QMap<QString, QList<QString>> DeletedPaths;
	foreach (const QString& Snapshot, SnapshotList)
	{
		QString PathsFile = GetFileRoot();
		if (!Snapshot.isEmpty())
			PathsFile += "\\snapshot-" + Snapshot;
		PathsFile += "\\FilePaths.dat";

		QFile File(PathsFile);
		if (File.open(QFile::ReadOnly)) {
			QByteArray Data = File.readAll();
			File.close();
			QString Text = QString::fromWCharArray((const wchar_t*)Data.constData(), Data.size()/sizeof(wchar_t));

			QList<QString> Deleted;

			QTextStream in(&Text);
			while (!in.atEnd()) {
				QStringList Line = in.readLine().split("|");
				if (Line.length() < 2 || Line[1] != "1")
					continue; // not a delete entry
				Deleted.append(theAPI->Nt2DosPath(Line[0]));
			}

			DeletedPaths[Snapshot] = Deleted;
		}
	}

	QMap<QString, SFoundLink> FoundLinks;

	int csidls[] = { CSIDL_DESKTOPDIRECTORY, CSIDL_COMMON_DESKTOPDIRECTORY, CSIDL_STARTMENU, CSIDL_COMMON_STARTMENU };
	for (int i = 0; i < ARRAYSIZE(csidls); i++)
	{
		WCHAR path[2048];
		if (SHGetFolderPath(NULL, csidls[i], NULL, SHGFP_TYPE_CURRENT, path) != S_OK)
			continue;

		foreach (const QString& Snapshot, SnapshotList)
		{
			QString BoxPath = theAPI->GetBoxedPath(this, QString::fromWCharArray(path), Snapshot);
			QStringList	Files = ListDir(BoxPath, QStringList() << "*.lnk" << "*.url" << "*.pif", i >= 2); // no subdir scan for desktop as people like to put junk there
			foreach(QString File, Files)
			{
				QString RealPath = theAPI->GetRealPath(this, BoxPath + "\\" + QString(File).replace("/", "\\"));

				// we scan the snapshots newest to oldest
				if (FoundLinks.contains(RealPath.toLower())) 
					continue;
				
				// check if the shortcut file is marked deleted
				if (IsFileDeleted(RealPath, Snapshot, SnapshotList, DeletedPaths))
					continue;

				SFoundLink FoundLink;
				FoundLink.Snapshot = Snapshot;
				FoundLink.LinkPath = BoxPath + "\\" + QString(File).replace("/", "\\");
				FoundLink.RealPath = RealPath;
				FoundLink.SubPath = (i >= 2 ? "" : "Desktop/") + File;
				FoundLinks.insert(FoundLink.RealPath.toLower(), FoundLink);
			}
		}
	}


	bool bChanged = false;
	auto OldStartMenu = ListToSet(m_StartMenu.keys());

	foreach (const SFoundLink& FoundLink, FoundLinks) {

		if (!OldStartMenu.remove(FoundLink.SubPath.toLower()))
			bChanged = true;

		StrPair PathName = Split2(FoundLink.SubPath, "/", true);
		StrPair NameExt = Split2(PathName.second, ".", true);
		if (NameExt.second.toLower() != "lnk")
			continue; // todo url

		QVariantMap Link = ResolveShortcut(FoundLink.LinkPath);
		if (!Link.contains("Path"))
			continue;

		SLink* pLink = &m_StartMenu[FoundLink.SubPath.toLower()];
		pLink->Folder = PathName.first;
		pLink->Name = NameExt.first;
		if(!pLink->Target.isEmpty() && pLink->Target != Link["Path"].toString())
			bChanged = true;
		pLink->Target = Link["Path"].toString();
		pLink->Icon = Link["IconPath"].toString();
		pLink->IconIndex = Link["IconIndex"].toInt();
		pLink->WorkDir = Link["WorkingDir"].toString();

		if (!pLink->Target.isEmpty() && !QFile::exists(pLink->Target) && !IsBoxexPath(pLink->Target))
			pLink->Target = theAPI->GetBoxedPath(this, pLink->Target, FoundLink.Snapshot);
		if (!pLink->WorkDir.isEmpty() && !QFile::exists(pLink->WorkDir) && !IsBoxexPath(pLink->WorkDir))
			pLink->WorkDir = theAPI->GetBoxedPath(this, pLink->WorkDir, FoundLink.Snapshot);
		if (!pLink->Icon.isEmpty() && !QFile::exists(pLink->Icon) && !IsBoxexPath(pLink->Icon))
			pLink->Icon = theAPI->GetBoxedPath(this, pLink->Icon, FoundLink.Snapshot);
	}

	foreach(const QString &Path, OldStartMenu)
		m_StartMenu.remove(Path);
	
	if (bChanged || !OldStartMenu.isEmpty())
		emit StartMenuChanged();
}

void CSandBoxPlus::SetBoxPaths(const QString& FilePath, const QString& RegPath, const QString& IpcPath)
{
	bool bPathChanged = (FilePath != m_FilePath);

	if (bPathChanged && !m_FilePath.isEmpty())
		((CSbiePlusAPI*)theAPI)->m_BoxMonitor->RemoveBox(this);

	CSandBox::SetBoxPaths(FilePath, RegPath, IpcPath);

	if (m_FilePath.isEmpty()) {
		m_IsEmpty = true;
		return;
	}

	m_IsEmpty = IsEmpty();

	if (bPathChanged && theConf->GetBool("Options/WatchBoxSize", false) && m_TotalSize == -1)
		((CSbiePlusAPI*)theAPI)->m_BoxMonitor->ScanBox(this);

	if (theConf->GetBool("Options/ScanStartMenu", true))
		ScanStartMenu();
}

void CSandBoxPlus::UpdateSize()
{
	m_TotalSize = -1;
	if(theConf->GetBool("Options/WatchBoxSize", false))
		((CSbiePlusAPI*)theAPI)->m_BoxMonitor->ScanBox(this);

	m_IsEmpty = IsEmpty();
}

void CSandBoxPlus::SetSize(quint64 Size)
{ 
	m_TotalSize = Size; 
	theConf->SetValue("SizeCache/" + m_Name, Size);
}

bool CSandBoxPlus::IsSizePending() const
{
	return ((CSbiePlusAPI*)theAPI)->m_BoxMonitor->IsScanPending(this);
}

void CSandBoxPlus::OpenBox()
{
	CSandBox::OpenBox();

	m_IsEmpty = false;
	
	if (theConf->GetBool("Options/WatchBoxSize", false))
		((CSbiePlusAPI*)theAPI)->m_BoxMonitor->WatchBox(this);
}

void CSandBoxPlus::CloseBox()
{
	CSandBox::CloseBox();

	m_SuspendRecovery = false;

	((CSbiePlusAPI*)theAPI)->m_BoxMonitor->CloseBox(this);

	if (theConf->GetBool("Options/ScanStartMenu", true))
		ScanStartMenu();
}

void CSandBoxPlus::ConnectEndSlot(const SB_PROGRESS& Status)
{
	CSbieProgressPtr pProgress = Status.GetValue();
	if (!pProgress.isNull())
		connect(pProgress.data(), SIGNAL(Finished()), this, SLOT(EndModifyingBox()));
	else
		EndModifyingBox();
}

void CSandBoxPlus::BeginModifyingBox()
{
	//
	// before altering the box structure or deleting it
	// we need to ensure that sandman will not hold any open handles on the files/folders inside
	//

	((CSbiePlusAPI*)theAPI)->m_BoxMonitor->RemoveBox(this);
	
	emit AboutToBeModified();
}

void CSandBoxPlus::EndModifyingBox()
{
	UpdateSize();
	if (theConf->GetBool("Options/ScanStartMenu", true))
		ScanStartMenu();
}

bool CSandBoxPlus::CheckUnsecureConfig() const
{
	//if (GetBool("UnsafeTemplate", false, true, true)) return true;
	if (GetBool("OriginalToken", false, true, true)) return true;
	if (GetBool("OpenToken", false, true, true)) return true;
		if(GetBool("UnrestrictedToken", false, true, true)) return true;
			if (GetBool("KeepTokenIntegrity", false, true, true)) return true;
			if (GetBool("UnstrippedToken", false, true, true)) return true;
				if (GetBool("KeepUserGroup", false, true, true)) return true;
			if (!GetBool("AnonymousLogon", true, true, true)) return true;
		if(GetBool("UnfilteredToken", false, true, true)) return true;
	if (GetBool("DisableFileFilter", false, true, true)) return true;
	if (GetBool("DisableKeyFilter", false, true, true)) return true;
	if (GetBool("DisableObjectFilter", false, true, true)) return true;
	if (GetBool("StripSystemPrivileges", false, true, true)) return true;
	return false;
}

QString CSandBoxPlus::GetStatusStr() const
{
	if (!m_IsEnabled)
		return tr("Disabled");

	if (!m_StatusStr.isEmpty())
		return m_StatusStr;

	QStringList Status;

	//if (m_IsEmpty)
	//	Status.append(tr("Empty"));

	if (m_bRootAccessOpen)
		Status.append(tr("OPEN Root Access"));
	else if (m_bApplicationCompartment)
		Status.append(tr("Application Compartment"));
	else if (m_iUnsecureDebugging == 1)
		Status.append(tr("NOT SECURE"));
	else if (m_iUnsecureDebugging == 2)
		Status.append(tr("Reduced Isolation"));
	else if(m_bSecurityEnhanced)
		Status.append(tr("Enhanced Isolation"));
	
	if(m_bPrivacyEnhanced)
		Status.append(tr("Privacy Enhanced"));

	if (m_bLogApiFound)
		Status.append(tr("API Log"));
	if (m_bINetBlocked) {
		if(m_bINetExceptions)
			Status.append(tr("No INet (with Exceptions)"));
		else
			Status.append(tr("No INet"));
	}
	if (m_bSharesAllowed)
		Status.append(tr("Net Share"));
	if (m_bDropRights && !m_bSecurityEnhanced)
		Status.append(tr("No Admin"));

	if(m_BoxDel)
		Status.append(tr("Auto Delete"));

	if (Status.isEmpty())
		return tr("Normal");
	return Status.join(", ");
}

CSandBoxPlus::EBoxTypes CSandBoxPlus::GetTypeImpl() const
{
	if (m_bRootAccessOpen)
		return eOpen;

	if (m_bApplicationCompartment && m_bPrivacyEnhanced)
		return eAppBoxPlus;
	if (m_bApplicationCompartment)
		return eAppBox;

	if (m_iUnsecureDebugging != 0)
		return eInsecure;

	if (m_bSecurityEnhanced && m_bPrivacyEnhanced)
		return eHardenedPlus;
	if (m_bSecurityEnhanced)
		return eHardened;

	if (m_bPrivacyEnhanced)
		return eDefaultPlus;
	return eDefault;
}

void CSandBoxPlus::SetLogApi(bool bEnable)
{
	if (bEnable)
	{
		//InsertText("OpenPipePath", "\\Device\\NamedPipe\\LogAPI");
		InsertText("InjectDll", "\\LogAPI\\logapi32.dll");
		InsertText("InjectDll64", "\\LogAPI\\logapi64.dll");
#ifdef _M_ARM64
		InsertText("InjectDllARM64", "\\LogAPI\\logapi64a.dll");
#endif
	}
	else
	{
		//DelValue("OpenPipePath", "\\Device\\NamedPipe\\LogAPI");
		DelValue("InjectDll", "\\LogAPI\\logapi32.dll");
		DelValue("InjectDll64", "\\LogAPI\\logapi64.dll");
#ifdef _M_ARM64
		DelValue("InjectDllARM64", "\\LogAPI\\logapi64a.dll");
#endif
	}
	m_bLogApiFound = bEnable;
}

void CSandBoxPlus::SetINetBlock(bool bEnable)
{
	if (bEnable) {
		if(theGUI->IsWFPEnabled())
			InsertText("AllowNetworkAccess", "!<InternetAccess>,n");
		else
			InsertText("ClosedFilePath", "!<InternetAccess>,InternetAccessDevices");
	}
	else
	{
		foreach(const QString& Entry, GetTextList("ClosedFilePath", false))
		{
			if (Entry.contains("InternetAccessDevices"))
				DelValue("ClosedFilePath", Entry);
		}
		foreach(const QString& Entry, GetTextList("AllowNetworkAccess", false))
		{
			if (Entry.contains("!<InternetAccess>,n"))
				DelValue("AllowNetworkAccess", Entry);
		}
	}
}

void CSandBoxPlus::SetAllowShares(bool bEnable)
{
	SetBoolSafe("BlockNetworkFiles", !bEnable);
}

void CSandBoxPlus::SetDropRights(bool bEnable)
{
	SetBoolSafe("DropAdminRights", bEnable);
}

QStringList::iterator FindInStrList(QStringList& list, const QString& str)
{
	QStringList::iterator J = list.begin();
	for (; J != list.end(); ++J)
	{
		if (J->compare(str, Qt::CaseInsensitive) == 0)
			break;
	}
	return J;
}

void SetInStrList(QStringList& list, const QString& str, bool bSet)
{
	if (bSet)
		list.append(str);
	else
	{
		for (QStringList::iterator J = list.begin(); J != list.end();)
		{
			if (J->compare(str, Qt::CaseInsensitive) == 0) {
				J = list.erase(J);
				continue;
			}
			++J;
		}
	}
}

bool CSandBoxPlus::TestProgramGroup(const QString& Group, const QString& ProgName)
{
	QStringList ProcessGroups = GetTextList("ProcessGroup", false);
	foreach(const QString & ProcessGroup, ProcessGroups)
	{
		StrPair GroupPaths = Split2(ProcessGroup, ",");
		if (GroupPaths.first.compare(Group, Qt::CaseInsensitive) == 0)
		{
			QStringList Programs = SplitStr(GroupPaths.second, ",");
			return FindInStrList(Programs, ProgName) != Programs.end();
		}
	}
	return false;
}

void CSandBoxPlus::EditProgramGroup(const QString& Group, const QString& ProgName, bool bSet)
{
	QStringList ProcessGroups = GetTextList("ProcessGroup", false);

	QStringList Programs;
	QStringList::iterator I = ProcessGroups.begin();
	for (; I != ProcessGroups.end(); ++I)
	{
		StrPair GroupPaths = Split2(*I, ",");
		if (GroupPaths.first.compare(Group, Qt::CaseInsensitive) == 0)
		{
			Programs = SplitStr(GroupPaths.second, ",");
			break;
		}
	}
	if (I == ProcessGroups.end())
		I = ProcessGroups.insert(I, "");

	SetInStrList(Programs, ProgName, bSet);

	*I = Group + "," + Programs.join(",");

	UpdateTextList("ProcessGroup", ProcessGroups, false);
}

void CSandBoxPlus::BlockProgram(const QString& ProgName)
{
	bool WhiteList = false;
	bool BlackList = false;
	foreach(const QString& Entry, GetTextList("ClosedIpcPath", false))
	{
		StrPair ProgPath = Split2(Entry, ",");
		if (ProgPath.second != "*")
			continue;
		if (ProgPath.first.compare("!<StartRunAccess>", Qt::CaseInsensitive) == 0)
			WhiteList = true;
		else if (ProgPath.first.compare("<StartRunAccess>", Qt::CaseInsensitive) == 0)
			BlackList = true;
		else
			continue;
		break;
	}

	if (!WhiteList && !BlackList)
	{
		BlackList = true;
		InsertText("ClosedIpcPath", "<StartRunAccess>,*");
	}

	EditProgramGroup("<StartRunAccess>", ProgName, !WhiteList);
}

void CSandBoxPlus::SetInternetAccess(const QString& ProgName, bool bSet)
{
	EditProgramGroup("<InternetAccess>", ProgName, bSet);
}

bool CSandBoxPlus::HasInternetAccess(const QString& ProgName)
{
	return TestProgramGroup("<InternetAccess>", ProgName);
}

void CSandBoxPlus::SetForcedProgram(const QString& ProgName, bool bSet)
{
	QStringList Programs = GetTextList("ForceProcess", false);
	SetInStrList(Programs, ProgName, bSet);
	UpdateTextList("ForceProcess", Programs, false);
}

bool CSandBoxPlus::IsForcedProgram(const QString& ProgName)
{
	QStringList Programs = GetTextList("ForceProcess", false);
	return FindInStrList(Programs, ProgName) != Programs.end();
}

void CSandBoxPlus::SetLingeringProgram(const QString& ProgName, bool bSet)
{
	QStringList Programs = GetTextList("LingerProcess", false);
	SetInStrList(Programs, ProgName, bSet);
	UpdateTextList("LingerProcess", Programs, false);
}

int CSandBoxPlus::IsLingeringProgram(const QString& ProgName)
{
	QStringList Programs = GetTextList("LingerProcess", false);
	if (FindInStrList(Programs, ProgName) != Programs.end())
		return 1;
	Programs = GetTextList("LingerProcess", true);
	if (FindInStrList(Programs, ProgName) != Programs.end())
		return 2;
	return 0;
}

void CSandBoxPlus::SetLeaderProgram(const QString& ProgName, bool bSet)
{
	QStringList Programs = GetTextList("LeaderProcess", false);
	SetInStrList(Programs, ProgName, bSet);
	UpdateTextList("LeaderProcess", Programs, false);
}

int	CSandBoxPlus::IsLeaderProgram(const QString& ProgName)
{
	QStringList Programs = GetTextList("LeaderProcess", false);
	return FindInStrList(Programs, ProgName) != Programs.end() ? 1 : 0; 
}

SB_STATUS CSandBoxPlus::DeleteContentAsync(bool DeleteSnapshots, bool bOnAutoDelete)
{
	if (GetBool("NeverDelete", false))
		return SB_ERR(SB_DeleteProtect);

	SB_STATUS Status = TerminateAll();
	if (Status.IsError())
		return Status;
	m_ActiveProcessCount = 0; // to ensure CleanBox will be happy

	foreach(const QString& Command, GetTextList("OnBoxDelete", true, false, true)) {
		CBoxJob* pJob = new COnDeleteJob(this, Expand(Command));
		AddJobToQueue(pJob);
	}

	CBoxJob* pJob = new CCleanUpJob(this, DeleteSnapshots, bOnAutoDelete);
	AddJobToQueue(pJob);

	return SB_OK;
}

void CSandBoxPlus::AddJobToQueue(CBoxJob* pJob)
{
	theAPI->m_JobCount++;
	m_JobQueue.append(QSharedPointer<CBoxJob>(pJob));
	if (m_JobQueue.count() == 1)
		StartNextJob();
}

void CSandBoxPlus::StartNextJob()
{
next:
	Q_ASSERT(m_JobQueue.count() > 0);
	Q_ASSERT(m_JobQueue.first()->GetProgress().isNull());

	QSharedPointer<CBoxJob> pJob = m_JobQueue.first();
	SB_PROGRESS Status = pJob->Start();
	if (Status.GetStatus() == OP_ASYNC) 
	{
		m_StatusStr = pJob->GetDescription();

		CSbieProgressPtr pProgress = Status.GetValue();
		connect(pProgress.data(), SIGNAL(Message(const QString&)), this, SLOT(OnAsyncMessage(const QString&)));
		connect(pProgress.data(), SIGNAL(Progress(int)), this, SLOT(OnAsyncProgress(int)));
		connect(pProgress.data(), SIGNAL(Finished()), this, SLOT(OnAsyncFinished()));
	}
	else
	{
		m_JobQueue.removeFirst();
		theAPI->m_JobCount--;
		if (Status.IsError()) {
			theAPI->m_JobCount -= m_JobQueue.count();
			m_JobQueue.clear();
			theGUI->CheckResults(QList<SB_STATUS>() << Status);
			return;
		}
		if (!m_JobQueue.isEmpty())
			goto next;
	}
}

void CSandBoxPlus::OnAsyncFinished()
{
	Q_ASSERT(m_JobQueue.count() > 0);
	Q_ASSERT(!m_JobQueue.first()->GetProgress().isNull());

	m_StatusStr.clear();

	QSharedPointer<CBoxJob> pJob = m_JobQueue.takeFirst();
	theAPI->m_JobCount--;
	CSbieProgressPtr pProgress = pJob->GetProgress();
	if (pProgress->IsCanceled()) {
		theAPI->m_JobCount -= m_JobQueue.count();
		m_JobQueue.clear();
		return;
	}

	SB_STATUS Status = pProgress->GetStatus();
	if (Status.IsError()) {
		theAPI->m_JobCount -= m_JobQueue.count();
		m_JobQueue.clear();
		theGUI->CheckResults(QList<SB_STATUS>() << Status, true);
		return;
	}
	else
		pJob->Finished();

	if (!m_JobQueue.isEmpty())
		StartNextJob();
}

void CSandBoxPlus::OnAsyncMessage(const QString& Text)
{
	m_StatusStr = Text;
}

void CSandBoxPlus::OnAsyncProgress(int Progress)
{
}

void CSandBoxPlus::OnCancelAsync()
{
	if (m_JobQueue.isEmpty())
		return;
	Q_ASSERT(!m_JobQueue.first()->GetProgress().isNull());

	QSharedPointer<CBoxJob> pJob = m_JobQueue.first();
	CSbieProgressPtr pProgress = pJob->GetProgress();
	pProgress->Cancel();
}

QString CSandBoxPlus::MakeBoxCommand(const QString& FileName)
{
	QString BoxFileName = FileName;
	//if (BoxFileName.indexOf(m_FilePath, Qt::CaseInsensitive) == 0) {
	//	BoxFileName.remove(0, m_FilePath.length());
	//	if (BoxFileName.at(0) != '\\')
	//		BoxFileName.prepend('\\');
	//}
	return BoxFileName.replace(m_FilePath, "%BoxRoot%", Qt::CaseInsensitive);
}

QString CSandBoxPlus::GetCommandFile(const QString& Command)
{
	QString Path = Command;
	if (Path.left(1) == "\"") {
		int End = Path.indexOf("\"", 1);
		if (End != -1) Path = Path.mid(1, End - 1);
	}
	else {
		int End = Path.indexOf(" ");
		if (End != -1) Path.truncate(End);
	}
	//if (Path.left(1) == "\\")
	//	Path.prepend(m_FilePath);
	return Path.replace("%BoxRoot%", m_FilePath, Qt::CaseInsensitive);
}

QString CSandBoxPlus::GetFullCommand(const QString& Command)
{
	QString FullCmd = Command;
	//if(FullCmd.left(1) == "\\")
	//	FullCmd.prepend(m_FilePath);
	//else if(FullCmd.left(2) == "\"\\")
	//	FullCmd.insert(1, m_FilePath);
	return FullCmd.replace("%BoxRoot%", m_FilePath, Qt::CaseInsensitive);
}