#include "stdafx.h"
#include "AddonManager.h"
#include "SandMan.h"
#include "OnlineUpdater.h"
#include "../MiscHelpers/Common/Common.h"
#include "../MiscHelpers/Common/OtherFunctions.h"
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include "../QSbieAPI/Sandboxie/SbieTemplates.h"
#include <QtConcurrent>
#include "../MiscHelpers/Archive/Archive.h"


#include <Windows.h>

CAddonManager::CAddonManager(QObject* parent)
 : QObject(parent)
{
	
}

void CAddonManager::UpdateAddonsWhenNotCached()
{
	QVariantMap Data = theGUI->m_pUpdater->GetUpdateData();
	if (!Data.isEmpty() && Data.contains("addons") && theGUI->m_pUpdater->GetLastUpdateTime() > QDateTime::currentDateTime().addDays(-1)) {
		OnUpdateData(Data, QVariantMap());
		return;
	}
	else if (!m_Addons.isEmpty()) {
		QFileInfo info(theConf->GetConfigDir() + "/addons.json");
		if (info.birthTime() > QDateTime::currentDateTime().addDays(-1))
			return;
	}

	UpdateAddons();
}

void CAddonManager::UpdateAddons()
{
	theGUI->m_pUpdater->GetUpdates(this, SLOT(OnUpdateData(const QVariantMap&, const QVariantMap&)));
}

void CAddonManager::OnUpdateData(const QVariantMap& Data, const QVariantMap& Params)
{
    if (Data.isEmpty() || Data["error"].toBool())
        return;

    QVariantMap Addons = Data["addons"].toMap();

	QJsonDocument doc(QJsonValue::fromVariant(Addons).toObject());			
	QFile::remove(theConf->GetConfigDir() + "/addons.json");
	WriteStringToFile(theConf->GetConfigDir() + "/addons.json", doc.toJson());

	LoadAddons();
	emit DataUpdated();
}

QList<CAddonPtr> CAddonManager::GetAddons() 
{
	if (m_Addons.isEmpty()) {
		if (!LoadAddons())
			UpdateAddons();
	}
	else {
		foreach(const CAddonPtr& pAddon, m_Addons)
			pAddon->Installed = CheckAddon(pAddon);
	}
	return m_Addons; 
}

bool CAddonManager::LoadAddons()
{
	m_Addons.clear();

	QString AddonPath = theConf->GetConfigDir() + "/addons.json";
	QVariantMap Data = QJsonDocument::fromJson(ReadFileAsString(AddonPath).toUtf8()).toVariant().toMap();
	foreach(const QVariant vAddon, Data["list"].toList()) {
		CAddonPtr pAddon = CAddonPtr(new CAddon(vAddon.toMap()));
		pAddon->Installed = CheckAddon(pAddon);
		m_Addons.append(pAddon);
	}

	return !m_Addons.isEmpty();
}

CAddonPtr CAddonManager::GetAddon(const QString& Id)
{
	if (m_Addons.isEmpty())
		LoadAddons();

	foreach(const CAddonPtr& pAddon, m_Addons) {
		if (pAddon->Id.compare(Id, Qt::CaseInsensitive) == 0) {
			pAddon->Installed = CheckAddon(pAddon);
			return pAddon;
		}
	}
	return CAddonPtr();
}

bool CAddonManager::HasAddon(const QString& Id)
{
	CAddonPtr pAddon = GetAddon(Id);
	return pAddon && pAddon->Installed;
}

bool CAddonManager::CheckAddon(const CAddonPtr& pAddon)
{
	QString Key = pAddon->GetSpecificEntry("uninstall_key").toString();
	if (!Key.isEmpty()) {
		QSettings settings(Key, QSettings::NativeFormat);
		QString Uninstall = settings.value("UninstallString").toString();
		return !Uninstall.isEmpty();
	}
	
	QStringList Files = pAddon->GetSpecificEntry("files").toStringList();
	foreach(const QString & File, Files) {
		if (theGUI->GetCompat()->CheckFile(ExpandPath(File)))
			return true;
	}
	return false;
}

SB_PROGRESS CAddonManager::TryInstallAddon(const QString& Id, QWidget* pParent, const QString& Prompt)
{
	if (QMessageBox("Sandboxie-Plus", Prompt.isEmpty() ? tr("Do you want to download and install %1?").arg(Id) : Prompt,
	  QMessageBox::Question, QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape, QMessageBox::NoButton, pParent ? pParent : theGUI).exec() != QMessageBox::Yes)
		return SB_ERR(SB_OtherError);

	SB_PROGRESS Status = InstallAddon(Id);
	if (Status.GetStatus() == OP_ASYNC)
		theGUI->AddAsyncOp(Status.GetValue(), false, tr("Installing: %1").arg(Id), pParent);
	else
		theGUI->CheckResults(QList<SB_STATUS>() << Status, pParent);
	return Status;
}

SB_PROGRESS CAddonManager::InstallAddon(const QString& Id)
{
	CAddonPtr pAddon = GetAddon(Id);
	if (!pAddon)
		return SB_ERR(SB_OtherError, QVariantList() << tr("Addon not found, please try updating the addon list in the global settings!"));
	if (pAddon->Installed)
		return SB_ERR(SB_OtherError, QVariantList() << tr("Addon already installed!"));

	QString Entry;
	QString Url = pAddon->GetSpecificEntry("download", &Entry).toString();
	if (Url.isEmpty()) 
		return SB_ERR(SB_OtherError, QVariantList() << tr("Addon has no download url, addon may not be available for your platform."));

	QVariantMap Params;
	Params["name"] = Id;
	Params["path"] = theGUI->m_pUpdater->GetUpdateDir(true) + "/" + QUrl(Url).fileName();
	Params["signature"] = pAddon->Data.value(Entry + "_sig");
	theGUI->m_pUpdater->DownloadFile(Url, this, SLOT(OnAddonDownloaded(const QString&, const QVariantMap&)), Params);

	pAddon->pProgress = CSbieProgressPtr(new CSbieProgress());
	connect(pAddon->pProgress.data(), SIGNAL(Finished()), this, SIGNAL(AddonInstalled()));
	pAddon->pProgress->ShowMessage(tr("Downloading Addon %1").arg(pAddon->Id));
	return SB_PROGRESS(OP_ASYNC, pAddon->pProgress);
}

extern "C" NTSTATUS VerifyFileSignatureImpl(const wchar_t* FilePath, PVOID Signature, ULONG SignatureSize);

void CAddonManager::OnAddonDownloaded(const QString& Path, const QVariantMap& Params)
{
	CAddonPtr pAddon = GetAddon(Params["name"].toString());

	QByteArray Signature = QByteArray::fromBase64(Params["signature"].toByteArray());
	
	if (VerifyFileSignatureImpl(QString(Path).replace("/","\\").toStdWString().c_str(), Signature.data(), Signature.size()) < 0) { // !NT_SUCCESS
		pAddon->pProgress->Finish(SB_ERR(SB_OtherError, QVariantList() << tr("Download signature is not valid!")));
		pAddon->pProgress.create();
		return;
	}

	pAddon->pProgress->ShowMessage(tr("Installing Addon %1").arg(pAddon->Id));

	QtConcurrent::run(CAddonManager::InstallAddonAsync, Path, pAddon);
}

void CAddonManager::InstallAddonAsync(const QString& FilePath, CAddonPtr pAddon)
{
	SB_STATUS Status = SB_OK;

	CArchive Archive(FilePath);

	if (Archive.Open() == 1) 
	{
		QString FileDir = Split2(FilePath, ".", true).first.replace("/", "\\");
		if (Archive.Extract(FileDir)) {

			QString Cmd = pAddon->GetSpecificEntry("installer").toString();
			QString Path = ExpandPath(pAddon->GetSpecificEntry("install_path").toString());
			if (!Cmd.isEmpty() && QFile::exists(FileDir + Cmd)) 
			{
				pAddon->pProgress->ShowMessage(tr("Running Installer for %1").arg(pAddon->Id));

				std::wstring sbiehome = theAPI->GetSbiePath().toStdWString();
				std::wstring plusdata = theConf->GetConfigDir().toStdWString();

				LPWCH environmentStrings = GetEnvironmentStrings();

				DWORD environmentLen = 0;
				for (LPWCH current = environmentStrings; *current; current += wcslen(current) + 1)
					environmentLen += wcslen(current) + 1;

				LPWCH modifiedEnvironment = (LPWCH)LocalAlloc(0, (environmentLen + sbiehome.length() + 1 + plusdata.length() + 1 + 1) * sizeof(wchar_t));
				memcpy(modifiedEnvironment, environmentStrings, (environmentLen + 1) * sizeof(wchar_t));
				
				FreeEnvironmentStrings(environmentStrings);

				LPWCH modifiedEnvironmentEnd = modifiedEnvironment + environmentLen;

				wcscpy(modifiedEnvironmentEnd, L"SBIEHOME=");
				wcscat(modifiedEnvironmentEnd, sbiehome.c_str());
				modifiedEnvironmentEnd += wcslen(modifiedEnvironmentEnd) + 1;

				wcscpy(modifiedEnvironmentEnd, L"PLUSDATA=");
				wcscat(modifiedEnvironmentEnd, plusdata.c_str());
				modifiedEnvironmentEnd += wcslen(modifiedEnvironmentEnd) + 1;

				*modifiedEnvironmentEnd = 0;

				STARTUPINFO si = { sizeof(si), 0 };
				PROCESS_INFORMATION pi = { 0 };
				if (CreateProcessW(NULL, (wchar_t*)(FileDir + Cmd).toStdWString().c_str(), NULL, NULL, FALSE, CREATE_UNICODE_ENVIRONMENT, modifiedEnvironment, NULL, &si, &pi))
				{
					while (WaitForSingleObject(pi.hProcess, 1000) == WAIT_TIMEOUT && !pAddon->pProgress->IsCanceled());
					CloseHandle(pi.hProcess);
					CloseHandle(pi.hThread);
				}
				else
					Status = SB_ERR(SB_OtherError, QVariantList() << tr("Failed to start installer (%1)!").arg(GetLastError()));

				LocalFree(modifiedEnvironment);
			}
			else if (!Path.isEmpty()) 
			{
				pAddon->pProgress->ShowMessage(tr("Copying Files for %1").arg(pAddon->Id));

				std::wstring from;
				foreach(const QString & file, ListDir(FileDir)) {
					QString File = QString(file).replace("/", "\\");
					from.append((FileDir + "\\" + File).toStdWString());
					from.append(L"\0", 1);
				}
				from.append(L"\0", 1);

				std::wstring to;
				to.append(Path.toStdWString());
				to.append(L"\0", 1);

				SHFILEOPSTRUCT SHFileOp;
				memset(&SHFileOp, 0, sizeof(SHFILEOPSTRUCT));
				SHFileOp.hwnd = NULL;
				SHFileOp.wFunc = FO_MOVE;
				SHFileOp.pFrom = from.c_str();
				SHFileOp.pTo = to.c_str();
				SHFileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;

				SHFileOperation(&SHFileOp);
			}

			QDir(FileDir).removeRecursively();
		}
		else
			Status = SB_ERR(SB_OtherError, QVariantList() << tr("Failed to unpack addon!"));
		Archive.Close();
	}

	QFile::remove(FilePath);

	if (!Status.IsError()) {
		pAddon->Installed = CheckAddon(pAddon);
		if (!pAddon->Installed)
			Status = SB_ERR(SB_OtherError, QVariantList() << tr("Addon Installation Failed!"));
	}
	pAddon->pProgress->Finish(Status);
	pAddon->pProgress.create();
}

SB_PROGRESS CAddonManager::TryRemoveAddon(const QString& Id, QWidget* pParent)
{
	if (QMessageBox("Sandboxie-Plus", tr("Do you want to remove %1?").arg(Id),
	  QMessageBox::Question, QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape, QMessageBox::NoButton, pParent ? pParent : theGUI).exec() != QMessageBox::Yes)
		return SB_ERR(SB_OtherError);

	SB_PROGRESS Status = RemoveAddon(Id);
	if (Status.GetStatus() == OP_ASYNC)
		theGUI->AddAsyncOp(Status.GetValue(), false, tr("Removing: %1").arg(Id), pParent);
	else
		theGUI->CheckResults(QList<SB_STATUS>() << Status, pParent);
	return Status;
}

SB_PROGRESS CAddonManager::RemoveAddon(const QString& Id)
{
	CAddonPtr pAddon = GetAddon(Id);
	if (!pAddon)
		return SB_ERR(SB_OtherError, QVariantList() << tr("Addon not found!"));

	pAddon->pProgress = CSbieProgressPtr(new CSbieProgress());
	QtConcurrent::run(CAddonManager::RemoveAddonAsync, pAddon);
	return SB_PROGRESS(OP_ASYNC, pAddon->pProgress);
}

void CAddonManager::CleanupPath(const QString& Path)
{
	StrPair PathName = Split2(Path, "\\", true);
	if (ListDir(PathName.first).isEmpty())
	{
		QDir().rmdir(PathName.first);
		//qDebug() << "delete dir" << PathName.first;
		CleanupPath(PathName.first);
	}
}

void CAddonManager::RemoveAddonAsync(CAddonPtr pAddon)
{
	SB_STATUS Status = SB_OK;

	QString Key = pAddon->GetSpecificEntry("uninstall_key").toString();
	if (!Key.isEmpty())
	{
		QSettings settings(Key, QSettings::NativeFormat);
		QString Cmd = settings.value("UninstallString").toString();
		
		pAddon->pProgress->ShowMessage(tr("Running Uninstaller for %1").arg(pAddon->Id));

		STARTUPINFO si = { sizeof(si), 0 };
		PROCESS_INFORMATION pi = { 0 };
		if (CreateProcessW(NULL, (wchar_t*)Cmd.toStdWString().c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		{
			while (WaitForSingleObject(pi.hProcess, 1000) == WAIT_TIMEOUT && !pAddon->pProgress->IsCanceled());
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
		else
			Status = SB_ERR(SB_OtherError, QVariantList() << tr("Failed to start uninstaller!"));
	}
	else
	{
		QStringList Files = pAddon->GetSpecificEntry("files").toStringList();
		//foreach(const QString & File, Files) {
		//	pAddon->pProgress->ShowMessage(tr("Removing %1").arg(File));
		//	QString FilePath = ExpandPath(File);
		//	QFile::remove(FilePath);
		//	CleanupPath(FilePath);
		//}

		std::wstring from;
		foreach(const QString & File, Files) {
			QString FilePath = ExpandPath(File);
			if (QFile::exists(FilePath)) {
				from.append(FilePath.toStdWString());
				from.append(L"\0", 1);
			}
		}
		from.append(L"\0", 1);

		SHFILEOPSTRUCT SHFileOp;
		memset(&SHFileOp, 0, sizeof(SHFILEOPSTRUCT));
		SHFileOp.hwnd = NULL;
		SHFileOp.wFunc = FO_DELETE;
		SHFileOp.pFrom = from.c_str();
		SHFileOp.pTo = NULL;
		SHFileOp.fFlags = FOF_NOCONFIRMATION;    

		SHFileOperation(&SHFileOp);
	}

	if (!Status.IsError()) {
		pAddon->Installed = CheckAddon(pAddon);
		if (pAddon->Installed)
			Status = SB_ERR(SB_OtherError, QVariantList() << tr("Addon Removal Failed!"));
	}
	pAddon->pProgress->Finish(Status);
	pAddon->pProgress.create();
}

QString CAddonManager::ExpandPath(QString Path)
{
	Path.replace("%SbieHome%", theAPI->GetSbiePath(), Qt::CaseInsensitive);
	Path.replace("%PlusData%", theConf->GetConfigDir(), Qt::CaseInsensitive);

	return theGUI->GetCompat()->ExpandPath(Path);
}

QString GetArch()
{
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    switch (systemInfo.wProcessorArchitecture)
    {
	case PROCESSOR_ARCHITECTURE_INTEL: return "x86";
	case PROCESSOR_ARCHITECTURE_AMD64: return "x64";
	case PROCESSOR_ARCHITECTURE_ARM64: return "a64";
    }
	return "???";
}

QVariant CAddon::GetSpecificEntry(const QString& Name, QString* pName)
{
#ifdef _M_ARM64
	QString arch = "a64";
#elif _WIN64
	QString arch = "x64";
#else
	QString arch = "x86";
#endif

	//
	// First we check the qt cpecific entry for our version of qt and platform
	//

	QString qt = QString("qt%1_%2_%3_%4").arg(QT_VERSION_MAJOR).arg(QT_VERSION_MINOR).arg(QT_VERSION_PATCH).arg(arch);
#ifdef  _DEBUG
	qt.append("d");
#endif //  _DEBUG
	if (Data.contains(Name + "_" + qt)) {
		if (pName) *pName = Name + "_" + qt;
		return Data[Name + "_" + qt];
	}

	//
	// Second we check the actual architecture
	//

	QString match = Data["match_arch"].toString();
	if (match != "agent")
		arch = GetArch();
	if (Data.contains(Name + "_" + arch)) {
		if (pName) *pName = Name + "_" + arch;
		return Data[Name + "_" + arch];
	}

	//
	// last we try the unsoecific entry
	//
	
	if (Data.contains(Name)) {
		if (pName) *pName = Name;
		return Data[Name];
	}

	return QString();
}

QString CAddon::GetLocalizedEntry(const QString& Name)
{
	if (Data.contains(Name + "_" + theGUI->m_Language))
		return Data[Name + "_" + theGUI->m_Language].toString();

	QString LangAux = theGUI->m_Language; // Short version as fallback
	LangAux.truncate(LangAux.lastIndexOf('_'));
	if (Data.contains(Name + "_" + LangAux))
		return Data[Name + "_" + LangAux].toString();

	return Data[Name].toString();
}
