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
#include "../../SandboxieTools/UpdUtil/UpdUtil.h"


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
	else if (!m_KnownAddons.isEmpty()) {
		QFileInfo info(theConf->GetConfigDir() + "/" ADDONS_FILE);
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
	QFile::remove(theConf->GetConfigDir() + "/" ADDONS_FILE);
	WriteStringToFile(theConf->GetConfigDir() + "/" ADDONS_FILE, doc.toJson());

	LoadAddons();
	emit DataUpdated();
}

QList<CAddonInfoPtr> CAddonManager::GetAddons() 
{
	if (m_KnownAddons.isEmpty()) {
		if (!LoadAddons())
			UpdateAddons();
	}

	QMap<QString, CAddonInfoPtr> Addons;

	m_Installed.clear();
	QDir Dir(theAPI->GetSbiePath() + ADDONS_PATH);
	foreach(const QFileInfo & Info, Dir.entryInfoList(QStringList() << "*.json", QDir::Files | QDir::Hidden | QDir::System)) {
		QString AddonPath = theAPI->GetSbiePath() + ADDONS_PATH + Info.fileName();
		QString AddonStr = ReadFileAsString(AddonPath);
		QVariantMap Data = QJsonDocument::fromJson(AddonStr.toUtf8()).toVariant().toMap();
		m_Installed.append(CAddonPtr(new CAddon(Data)));
		Addons.insert(Data["id"].toString().toLower(), CAddonInfoPtr(new CAddonInfo(Data, true)));
	}

	foreach(const CAddonPtr& pAddon, m_KnownAddons) {
		CAddonInfoPtr& pInfo = Addons[pAddon->Id.toLower()];
		if (pInfo.isNull()) {
			bool Installed = false;
			
			QString Key = pAddon->GetSpecificEntry("uninstallKey").toString();
			if (!Key.isEmpty()) {
				QSettings settings(Key, QSettings::NativeFormat);
				QString Uninstall = settings.value("UninstallString").toString();
				if (!Uninstall.isEmpty()) {
					Installed = true;
					m_Installed.append(CAddonPtr(new CAddon(pAddon->Data)));
				}
			}

			pInfo = CAddonInfoPtr(new CAddonInfo(pAddon->Data, Installed));
		}
		else if (pInfo->Data["version"] != pAddon->Data["version"])
			pInfo->UpdateVersion = pAddon->Data["version"].toString();
	}

	return Addons.values(); 
}

bool CAddonManager::LoadAddons()
{
	m_KnownAddons.clear();

	QString AddonPath = theConf->GetConfigDir() + "/" ADDONS_FILE;
	QString AddonStr = ReadFileAsString(AddonPath);
	QVariantMap Data = QJsonDocument::fromJson(AddonStr.toUtf8()).toVariant().toMap();
	foreach(const QVariant vAddon, Data["list"].toList())
		m_KnownAddons.append(CAddonPtr(new CAddon(vAddon.toMap())));

	return !m_KnownAddons.isEmpty();
}

CAddonPtr CAddonManager::GetAddon(const QString& Id, EState State)
{
	if (State != eNotINstalled) 
	{
		foreach(const CAddonPtr & pAddon, m_Installed) {
			if (pAddon->Id.compare(Id, Qt::CaseInsensitive) == 0)
				return pAddon;
		}
	}

	if (State != eInstalled)
	{
		if (m_KnownAddons.isEmpty())
			LoadAddons();

		foreach(const CAddonPtr & pAddon, m_KnownAddons) {
			if (pAddon->Id.compare(Id, Qt::CaseInsensitive) == 0)
				return pAddon;
		}
	}
	return CAddonPtr();
}

/*bool CAddonManager::CheckAddon(const CAddonPtr& pAddon)
{
	QString Key = pAddon->GetSpecificEntry("uninstallKey").toString();
	if (!Key.isEmpty()) {
		QSettings settings(Key, QSettings::NativeFormat);
		QString Uninstall = settings.value("UninstallString").toString();
		return !Uninstall.isEmpty();
	}
	
	/ *QStringList Files = pAddon->GetSpecificEntry("files").toStringList();
	foreach(const QString & File, Files) {
		if (theGUI->GetCompat()->CheckFile(ExpandPath(File)))
			return true;
	}
	return false;* /

	QString AddonFile = theAPI->GetSbiePath() + ADDONS_PATH + pAddon->Id + ".json";
	return QFile::exists(AddonFile);
}*/

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

SB_PROGRESS CAddonManager::InstallAddon(const QString& Id)
{
	CAddonPtr pAddon = GetAddon(Id, eNotINstalled);
	if (!pAddon)
		return SB_ERR(SB_OtherError, QVariantList() << tr("Add-on not found, please try updating the add-on list in the global settings!"));

	QFile::remove(theGUI->m_pUpdater->GetUpdateDir(true) + "/" ADDONS_FILE);
	QFile::copy(theConf->GetConfigDir() + "/" ADDONS_FILE, theGUI->m_pUpdater->GetUpdateDir(true) + "/" ADDONS_FILE);

	QStringList Params;
	Params.append("modify");
	Params.append("add:" + pAddon->Id);
	Params.append("/agent_arch:" + GetAppArch());
	Params.append("/framework:" + GetFramework());
	Params.append("/step:apply");
	Params.append("/embedded");
	Params.append("/temp:" + theGUI->m_pUpdater->GetUpdateDir().replace("/", "\\"));

	pAddon->pProgress = CSbieProgressPtr(new CSbieProgress());
	QtConcurrent::run(CAddonManager::RunUpdaterAsync, pAddon, Params);
	//QTimer::singleShot(10, this, [=]() { CAddonManager::RunUpdaterAsync(pAddon, Params); });
	return SB_PROGRESS(OP_ASYNC, pAddon->pProgress);
}

SB_PROGRESS CAddonManager::RemoveAddon(const QString& Id)
{
	CAddonPtr pAddon = GetAddon(Id, eInstalled);
	if (!pAddon)
		return SB_ERR(SB_OtherError, QVariantList() << tr("Add-on not found!"));

	QStringList Params;
	Params.append("modify");
	Params.append("remove:" + pAddon->Id);
	Params.append("/step:apply");
	Params.append("/embedded");

	pAddon->pProgress = CSbieProgressPtr(new CSbieProgress());
	QtConcurrent::run(CAddonManager::RunUpdaterAsync, pAddon, Params);
	//QTimer::singleShot(10, this, [=]() { CAddonManager::RunUpdaterAsync(pAddon, Params); });
	return SB_PROGRESS(OP_ASYNC, pAddon->pProgress);
}

QString GetUpdErrorStr(int exitCode);

QString GetUpdErrorStr2(int exitCode)
{
	switch (exitCode)
	{
	case ERROR_NO_ADDON: return CAddonManager::tr("Add-on Not Found");
	case ERROR_NO_ADDON2: return CAddonManager::tr("Add-on is not available for this platform");
	case ERROR_BAD_ADDON: return CAddonManager::tr("Missing installation instructions");
	case ERROR_BAD_ADDON2: return CAddonManager::tr("Executing add-on setup failed");
	case ERROR_DELETE: return CAddonManager::tr("Failed to delete a file during add-on removal");
	default: return GetUpdErrorStr(exitCode);
	}
}

void CAddonManager::RunUpdaterAsync(CAddonPtr pAddon, const QStringList& Params)
{
#ifdef _DEBUG
	CSbieResult<int> Status = COnlineUpdater::RunUpdater(Params, false, true);
#else
	CSbieResult<int> Status = COnlineUpdater::RunUpdater(Params, true, true);
#endif

	if(Status.IsError())
		pAddon->pProgress->Finish(SB_ERR(SB_OtherError, QVariantList() << tr("Updater failed to perform add-on operation")));
	else if(Status.GetValue() < 0)
		pAddon->pProgress->Finish(SB_ERR(SB_OtherError, QVariantList() << tr("Updater failed to perform add-on operation, error: %1").arg(GetUpdErrorStr2(Status.GetValue()))));
	else
		pAddon->pProgress->Finish(SB_OK);
	pAddon->pProgress.clear();
}

QString CAddonManager::GetAppArch()
{
#ifdef _M_ARM64
	return "a64";
#elif _WIN64
	return "x64";
#else
	return "x86";
#endif
}

QString CAddonManager::GetSysArch()
{
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    switch (systemInfo.wProcessorArchitecture)
    {
	case PROCESSOR_ARCHITECTURE_INTEL: return "x86";
	case PROCESSOR_ARCHITECTURE_AMD64: return "x64";
	case PROCESSOR_ARCHITECTURE_ARM64: return "a64";
    }
	return GetAppArch(); // fallback
}

QString CAddonManager::GetFramework()
{
	QString qt = QString("qt%1.%2.%3").arg(QT_VERSION_MAJOR).arg(QT_VERSION_MINOR).arg(QT_VERSION_PATCH);
#ifdef  _DEBUG
	qt.append("d");
#endif //  _DEBUG
	qt.append("_" + GetAppArch());
	return qt;
}

QVariant CAddon::GetSpecificEntry(const QString& Name, QString* pName)
{
	//
	// First we check the Qt specific entry for our version of Qt and platform
	//

	QString qt = CAddonManager::GetFramework();
#ifdef  _DEBUG
	qt.append("d");
#endif //  _DEBUG
	if (Data.contains(Name + "-" + qt)) {
		if (pName) *pName = Name + "-" + qt;
		return Data[Name + "-" + qt];
	}

	//
	// Second we check the actual architecture
	//

	QString match = Data["matchArch"].toString();
	QString arch = match != "agent" ? CAddonManager::GetSysArch() : CAddonManager::GetAppArch();
	if (Data.contains(Name + "-" + arch)) {
		if (pName) *pName = Name + "-" + arch;
		return Data[Name + "-" + arch];
	}

	//
	// last we try the unspecific entry
	//
	
	if (Data.contains(Name)) {
		if (pName) *pName = Name;
		return Data[Name];
	}

	return QString();
}

QString CAddonInfo::GetLocalizedEntry(const QString& Name)
{
	if (Data.contains(Name + "_" + theGUI->m_Language))
		return Data[Name + "_" + theGUI->m_Language].toString();

	QString LangAux = theGUI->m_Language; // Short version as fallback
	LangAux.truncate(LangAux.lastIndexOf('_'));
	if (Data.contains(Name + "_" + LangAux))
		return Data[Name + "_" + LangAux].toString();

	return Data[Name].toString();
}
