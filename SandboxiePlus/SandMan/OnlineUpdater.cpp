#include "stdafx.h"
#include "OnlineUpdater.h"
#include "../MiscHelpers/Common/Common.h"
#include "../MiscHelpers/Common/OtherFunctions.h"
#include "SandMan.h"
#include "Windows/SettingsWindow.h"
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include "../MiscHelpers/Common/CheckableMessageBox.h"
#include <QMessageBox>
#include "../../SandboxieTools/UpdUtil/UpdUtil.h"
#include <QCryptographicHash>
#include "Helpers/WinAdmin.h"
#include <windows.h>
#include <QRandomGenerator>

#ifdef QT_NO_SSL
#error Qt requires Open SSL support for the updater to work
#endif

#ifdef _DEBUG

// mess with a dummy installation when debugging

/*#undef VERSION_MJR
#define VERSION_MJR		1
#undef VERSION_MIN
#define VERSION_MIN 	11
#undef VERSION_REV
#define VERSION_REV 	4
#undef VERSION_UPD
#define VERSION_UPD 	0*/

#define DUMMY_PATH "C:\\Projects\\Sandboxie\\SandboxieTools\\x64\\Debug\\Test"
#endif

DWORD GetIdleTime() // in seconds
{
	LASTINPUTINFO lastInPut;
	GetLastInputInfo(&lastInPut);
    return (GetTickCount() - lastInPut.dwTime) / 1000;
}

COnlineUpdater::COnlineUpdater(QObject* parent) : QObject(parent)
{
	m_IgnoredUpdates = theConf->GetStringList("Options/IgnoredUpdates");

	m_RequestManager = NULL;
	m_pUpdaterUtil = NULL;

	LoadState();
}

void COnlineUpdater::StartJob(CUpdatesJob* pJob, const QUrl& Url)
{
	if (m_RequestManager == NULL) 
		m_RequestManager = new CNetworkAccessManager(30 * 1000, this);

	QNetworkRequest Request = QNetworkRequest(Url);
	//Request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
	Request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
	//Request.setRawHeader("Accept-Encoding", "gzip");
	QNetworkReply* pReply = m_RequestManager->get(Request);
	connect(pReply, SIGNAL(finished()), this, SLOT(OnRequestFinished()));
	connect(pReply, SIGNAL(downloadProgress(qint64, qint64)), pJob, SLOT(OnDownloadProgress(qint64, qint64)));

	connect(pJob->m_pProgress.data(), &CSbieProgress::Canceled, pReply, &QNetworkReply::abort);
	m_JobQueue.insert(pReply, pJob);
}

void COnlineUpdater::OnRequestFinished()
{
	QNetworkReply* pReply = qobject_cast<QNetworkReply*>(sender());
	CUpdatesJob* pJob = m_JobQueue.take(pReply);
	if (pJob) {
		pJob->Finish(pReply);
		pJob->deleteLater();
	}
	pReply->deleteLater();
}

quint64 COnlineUpdater::GetRandID()
{
	quint64 RandID = 0;
	theAPI->GetSecureParam("RandID", &RandID, sizeof(RandID));
	if (!RandID) {
		RandID = QRandomGenerator64::global()->generate();
		theAPI->SetSecureParam("RandID", &RandID, sizeof(RandID));
	}
	return RandID;
}

SB_PROGRESS COnlineUpdater::GetUpdates(QObject* receiver, const char* member, const QVariantMap& Params)
{
	QUrlQuery Query;
	Query.addQueryItem("action", "update");
	Query.addQueryItem("software", "sandboxie-plus");
	//QString Branch = theConf->GetString("Options/ReleaseBranch");
	//if (!Branch.isEmpty())
	//	Query.addQueryItem("branch", Branch);
	//Query.addQueryItem("version", theGUI->GetVersion());
	//Query.addQueryItem("version", QString::number(VERSION_MJR) + "." + QString::number(VERSION_MIN) + "." + QString::number(VERSION_REV) + "." + QString::number(VERSION_UPD));
#ifdef INSIDER_BUILD
	Query.addQueryItem("version", QString(__DATE__));
#else
	Query.addQueryItem("version", QString::number(VERSION_MJR) + "." + QString::number(VERSION_MIN) + "." + QString::number(VERSION_REV));
#endif
	Query.addQueryItem("system", "windows-" + QSysInfo::kernelVersion() + "-" + QSysInfo::currentCpuArchitecture());
	Query.addQueryItem("language", QLocale::system().name());
#ifdef _DEBUG
	Query.addQueryItem("debug", "1");
#endif

	QString UpdateKey = GetArguments(g_Certificate, L'\n', L':').value("UPDATEKEY");
	//if (UpdateKey.isEmpty())
	//	UpdateKey = theAPI->GetGlobalSettings()->GetText("UpdateKey"); // theConf->GetString("Options/UpdateKey");
	//if (UpdateKey.isEmpty())
	//	UpdateKey = "00000000000000000000000000000000";
	Query.addQueryItem("update_key", UpdateKey);
	
	quint64 RandID = COnlineUpdater::GetRandID();
	quint32 Hash = theAPI->GetUserSettings()->GetName().mid(13).toInt(NULL, 16);
	QString HashKey = QString::number(Hash, 16).rightJustified(8, '0').toUpper() + "-" + QString::number(RandID, 16).rightJustified(16, '0').toUpper();
	Query.addQueryItem("hash_key", HashKey);

	if (Params.contains("channel")) 
		Query.addQueryItem("channel", Params["channel"].toString());
	else {
		QString ReleaseChannel = theConf->GetString("Options/ReleaseChannel", "stable");
		Query.addQueryItem("channel", ReleaseChannel);
	}

	Query.addQueryItem("auto", Params["manual"].toBool() ? "0" : "1");

	if (!Params["manual"].toBool()) {
		int UpdateInterval = theConf->GetInt("Options/UpdateInterval", UPDATE_INTERVAL); // in seconds
		Query.addQueryItem("interval", QString::number(UpdateInterval));
	}

#ifdef _DEBUG
	QString Test = Query.toString();
#endif

	QUrl Url("https://sandboxie-plus.com/update.php");
	Url.setQuery(Query);

	CUpdatesJob* pJob = new CGetUpdatesJob(Params, this);
	StartJob(pJob, Url);
	QObject::connect(pJob, SIGNAL(UpdateData(const QVariantMap&, const QVariantMap&)), receiver, member, Qt::QueuedConnection);
	return SB_PROGRESS(OP_ASYNC, pJob->m_pProgress);
}

void CGetUpdatesJob::Finish(QNetworkReply* pReply)
{
	QVariantMap Data;

	auto err = pReply->error();
	if (err != QNetworkReply::NoError) 
	{
		//m_pProgress->Finish(SB_ERR(SB_OtherError, QVariantList() << tr("Updater Error: %1").arg(err), err));
		Data["error"] = true;
		Data["errorMsg"] = tr("%1").arg(err);
	}
	else
	{
		QByteArray Reply = pReply->readAll();

		Data = QJsonDocument::fromJson(Reply).toVariant().toMap();

		if (Data.contains("cbl"))
		{
			QVariantMap CertBL = Data["cbl"].toMap();
			QByteArray BlockList0 = CertBL["list"].toByteArray();
			QByteArray BlockListSig0 = QByteArray::fromHex(CertBL["sig"].toByteArray());

			if (theAPI->TestSignature(BlockList0, BlockListSig0))
			{
				std::string BlockList;
				BlockList.resize(qMax(0x10000, BlockList0.size()), 0); // 64 kb should be enough
				static quint32 BlockListLen = 0;
				if (BlockListLen == 0) {
					SB_STATUS Status = theAPI->GetSecureParam("CertBlockList", (void*)BlockList.c_str(), BlockList.size(), &BlockListLen, true);
					//BlockList.resize(BlockListLen);
					if (Status.IsError()) // error
						BlockListLen = 0;
				}

				if (BlockListLen < BlockList0.size())
				{
					theAPI->SetSecureParam("CertBlockList", BlockList0, BlockList0.size());
					theAPI->SetSecureParam("CertBlockListSig", BlockListSig0, BlockListSig0.size());
					BlockListLen = BlockList0.size();
					//BlockList = BlockList0;

					theGUI->ReloadCert();
				}
			}
			else
			{
				Q_ASSERT(0);
			}
		}

		time_t CurrentDate = QDateTime::currentDateTimeUtc().toSecsSinceEpoch();
		theAPI->SetSecureParam("LastUpdate", &CurrentDate, sizeof(CurrentDate));

		QString LabelMsg = Data["labelMsg"].toString();
		theConf->SetValue("Updater/LabelMessage", LabelMsg);
	}

	m_pProgress->Finish(SB_OK);

	emit UpdateData(Data, m_Params);
}

QDateTime COnlineUpdater::GetLastUpdateDate()
{
	time_t UpdateDate = 0;
	theAPI->GetSecureParam("LastUpdate", &UpdateDate, sizeof(UpdateDate));

	time_t CurrentDate = QDateTime::currentDateTimeUtc().toSecsSinceEpoch();
	if (UpdateDate > CurrentDate) { // can't be in the future
		UpdateDate = 0;
		theAPI->SetSecureParam("LastUpdate", &UpdateDate, sizeof(UpdateDate));
	}

	return QDateTime::fromSecsSinceEpoch(UpdateDate);
}

SB_PROGRESS COnlineUpdater::DownloadFile(const QString& Url, QObject* receiver, const char* member, const QVariantMap& Params)
{
	CUpdatesJob* pJob = new CGetFileJob(Params, this);
	StartJob(pJob, Url);
	QObject::connect(pJob, SIGNAL(Download(const QString&, const QVariantMap&)), receiver, member, Qt::QueuedConnection);
	return SB_PROGRESS(OP_ASYNC, pJob->m_pProgress);
}

void CGetFileJob::Finish(QNetworkReply* pReply)
{
	quint64 Size = pReply->bytesAvailable();

	m_pProgress->SetProgress(-1);

	QString FilePath = m_Params["path"].toString();
	if (FilePath.isEmpty()) {
		QString Name = pReply->request().url().fileName();
		if (Name.isEmpty())
			Name = "unnamed_download.tmp";
		FilePath = ((COnlineUpdater*)parent())->GetUpdateDir(true) + "/" + Name;
	}

	QFile File(FilePath);
	if (File.open(QFile::WriteOnly)) {
		while (pReply->bytesAvailable() > 0)
			File.write(pReply->read(4096));
		File.flush();
		QDateTime Date = m_Params["setDate"].toDateTime();
		if(Date.isValid())
			File.setFileTime(Date, QFileDevice::FileModificationTime);
		File.close();
	}

	m_pProgress->Finish(SB_OK);

	if (File.size() != Size) {
		QMessageBox::critical(theGUI, "Sandboxie-Plus", tr("Failed to download file from: %1").arg(pReply->request().url().toString()));
		return;
	}

	emit Download(FilePath, m_Params);
}

SB_PROGRESS COnlineUpdater::GetSupportCert(const QString& Serial, QObject* receiver, const char* member, const QVariantMap& Params)
{
	QString UpdateKey = Params["key"].toString();

	QUrlQuery Query;

	bool bHwId = false;
	if (!Serial.isEmpty()) {
		Query.addQueryItem("SN", Serial);
		if (Serial.length() > 5 && Serial.at(4).toUpper() == 'N')
			bHwId = true;
	}

	if(!UpdateKey.isEmpty())
		Query.addQueryItem("UpdateKey", UpdateKey);

	quint64 RandID = COnlineUpdater::GetRandID();
	quint32 Hash = theAPI->GetUserSettings()->GetName().mid(13).toInt(NULL, 16);
	QString HashKey = QString::number(Hash, 16).rightJustified(8, '0').toUpper() + "-" + QString::number(RandID, 16).rightJustified(16, '0').toUpper();
	Query.addQueryItem("HashKey", HashKey);

	if (Serial.isEmpty() && Params.contains("Name")) { // Request eval Key
		Query.addQueryItem("Name", Params["Name"].toString()); // for cert
		Query.addQueryItem("eMail", Params["eMail"].toString());
		bHwId = true;
	}

	if (IsLockRequired()) {
		Query.addQueryItem("LR", "1");
		bHwId = true;
	}

	if (bHwId) {
		wchar_t uuid_str[40];
		theAPI->GetDriverInfo(-2, uuid_str, sizeof(uuid_str));
		Query.addQueryItem("HwId", QString::fromWCharArray(uuid_str));
	}

#ifdef _DEBUG
	QString Test = Query.toString();
#endif

	QUrl Url("https://sandboxie-plus.com/get_cert.php?");
	Url.setQuery(Query);

	CUpdatesJob* pJob = new CGetCertJob(Params, this);
	StartJob(pJob, Url);
	QObject::connect(pJob, SIGNAL(Certificate(const QByteArray&, const QVariantMap&)), receiver, member, Qt::QueuedConnection);
	return SB_PROGRESS(OP_ASYNC, pJob->m_pProgress);
}

extern "C" NTSTATUS NTAPI NtQueryInstallUILanguage(LANGID* LanguageId);

bool COnlineUpdater::IsLockRequired()
{
	if (theConf->GetBool("Debug/LockedRegion", false))
		return true;

	if (g_CertInfo.lock_req)
		return true;

	LANGID LangID = 0;
	if ((NtQueryInstallUILanguage(&LangID) == 0) && (LangID == 0x0804))
		return true;

	if (theGUI->m_LanguageId == 0x0804)
		return true;

	return false;
}

void CGetCertJob::Finish(QNetworkReply* pReply)
{
	QByteArray Reply = pReply->readAll();

	m_pProgress->Finish(SB_OK);

	if (Reply.left(1) == "{") { // error

		QVariantMap Data = QJsonDocument::fromJson(Reply).toVariant().toMap();
		Reply.clear();

		m_Params["error"] = Data["errorMsg"].toString();
	}
	
	emit Certificate(Reply, m_Params);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Update Handling
//

void COnlineUpdater::LoadState()
{
	m_CheckMode = eInit;

	int iUpdate = 0;
	QString UpdateStr = ParseVersionStr(theConf->GetString("Updater/PendingUpdate"), &iUpdate);
	if (!IsVersionNewer(UpdateStr) && (UpdateStr != GetCurrentVersion() || iUpdate <= GetCurrentUpdate()))
		theConf->SetValue("Updater/PendingUpdate", ""); // it seems update has been applied

	bool bIsInstallerReady = false;
	QString FilePath = theConf->GetString("Updater/InstallerPath");
	if (!FilePath.isEmpty() && QFile::exists(FilePath)) {
		QString ReleaseStr = ParseVersionStr(theConf->GetString("Updater/InstallerVersion"));
		if (IsVersionNewer(ReleaseStr)) {
			bIsInstallerReady = true;
		}
	}
	QString OnNewRelease = GetOnNewReleaseOption();
	bool bCanRunInstaller = OnNewRelease == "install";

	bool bIsUpdateReady = false;
	QVariantMap Update = QJsonDocument::fromJson(ReadFileAsString(GetUpdateDir() + "/" UPDATE_FILE).toUtf8()).toVariant().toMap();
	if (!Update.isEmpty()) {
		int iUpdate = 0;
		QString UpdateStr = ParseVersionStr(theConf->GetString("Updater/UpdateVersion"), &iUpdate);
		if (IsVersionNewer(UpdateStr) || (UpdateStr == GetCurrentVersion() && iUpdate > GetCurrentUpdate())) {
			if (ScanUpdateFiles(Update) == eNone) // check if this update has already been applied
				theConf->SetValue("Updater/CurrentUpdate", MakeVersionStr(Update)); // cache result
			else
				bIsUpdateReady = true;
		}
	}
	QString OnNewUpdate = GetOnNewUpdateOption();
	bool bCanApplyUpdate = OnNewUpdate == "install";

	if (bIsInstallerReady && bCanRunInstaller)
		m_CheckMode = ePendingInstall;
	else if (bIsUpdateReady && bCanApplyUpdate)
		m_CheckMode = ePendingUpdate;
}

QString COnlineUpdater::GetOnNewUpdateOption() const
{
	QString OnNewUpdate = theConf->GetString("Options/OnNewUpdate", "ignore");

	QString ReleaseChannel = theConf->GetString("Options/ReleaseChannel", "stable");
	if (ReleaseChannel != "preview" && (!g_CertInfo.active || g_CertInfo.expired)) // without active cert, allow revisions for preview channel
		return "ignore"; // this service requires a valid certificate

	return OnNewUpdate;
}

QString COnlineUpdater::GetOnNewReleaseOption() const
{
	QString OnNewRelease = theConf->GetString("Options/OnNewRelease", "download");

	if (OnNewRelease == "install" || OnNewRelease == "download") {
		QString ReleaseChannel = theConf->GetString("Options/ReleaseChannel", "stable");
		if (ReleaseChannel != "preview" && (!g_CertInfo.active || g_CertInfo.expired)) // without active cert, allow automated updates only for preview channel
			return "notify"; // this service requires a valid certificate
	}

	//if ((g_CertInfo.active && g_CertInfo.expired) && OnNewRelease == "install")
	//	return "download"; // disable auto update on an active but expired personal certificate
	return OnNewRelease;
}

bool COnlineUpdater::ShowCertWarningIfNeeded()
{
	//
	// This function checks if this installation uses a expired personal
	// certificate which is active for the current build
	// in which case it shows a warning that updating to the latest build 
	// will deactivate the certificate
	//

	if (!(g_CertInfo.active && g_CertInfo.expired))
		return true;

	QString Message = tr("Your Sandboxie-Plus supporter certificate is expired, however for the current build you are using it remains active, when you update to a newer build exclusive supporter features will be disabled.\n\n"
		"Do you still want to update?");
	int Ret = QMessageBox("Sandboxie-Plus", Message, QMessageBox::Warning, QMessageBox::Yes, QMessageBox::No | QMessageBox::Escape | QMessageBox::Default, QMessageBox::Cancel, theGUI).exec();
	if (Ret == QMessageBox::Cancel) {
		QTimer::singleShot(10, this, [=] {
			theConf->DelValue("Updater/InstallerPath");
			theConf->DelValue("Updater/UpdateVersion");
			theGUI->UpdateLabel();
		});
	}
	return Ret == QMessageBox::Yes;
}

void COnlineUpdater::Process() 
{
	int UpdateInterval = theConf->GetInt("Options/UpdateInterval", UPDATE_INTERVAL); // in seconds
	QDateTime CurretnDate = QDateTime::currentDateTime();
	time_t NextUpdateCheck = theConf->GetUInt64("Options/NextCheckForUpdates", 0);
	if (NextUpdateCheck == 0 || NextUpdateCheck > CurretnDate.addDays(31).toSecsSinceEpoch()) { // no check made yet or invalid value
		NextUpdateCheck = CurretnDate.addSecs(UpdateInterval).toSecsSinceEpoch();
		theConf->SetValue("Options/NextCheckForUpdates", NextUpdateCheck);
	}

	int iCheckUpdates = theConf->GetInt("Options/CheckForUpdates", 2);
	if (iCheckUpdates != 0)
	{
		if(CurretnDate.toSecsSinceEpoch() >= NextUpdateCheck)
		{
			if (iCheckUpdates == 2)
			{
				bool bCheck = false;
				iCheckUpdates = CCheckableMessageBox::question(theGUI, "Sandboxie-Plus", tr("Do you want to check if there is a new version of Sandboxie-Plus?")
					, tr("Don't show this message again."), &bCheck, QDialogButtonBox::Yes | QDialogButtonBox::No, QDialogButtonBox::Yes, QMessageBox::Information) == QDialogButtonBox::Ok ? 1 : 0;

				if (bCheck)
					theConf->SetValue("Options/CheckForUpdates", iCheckUpdates);
			}

			if (iCheckUpdates == 0) // no clicked on prompt
				theConf->SetValue("Options/NextCheckForUpdates", CurretnDate.addSecs(UpdateInterval).toSecsSinceEpoch());
			else
			{
				// schedule next check in 12 h in case this one fails
				theConf->SetValue("Options/NextCheckForUpdates", CurretnDate.addSecs(12 * 60 * 60).toSecsSinceEpoch());
				
				CheckForUpdates(false);
			}
		}
	}
	else if (g_CertInfo.active)
	{
		QDateTime LastUpdateDate = COnlineUpdater::GetLastUpdateDate();
		int DaysSinceUpdate = LastUpdateDate.daysTo(CurretnDate);
		if (DaysSinceUpdate > 90 && CurretnDate.toSecsSinceEpoch() >= NextUpdateCheck && m_JobQueue.isEmpty())
		{
			bool bCheck = true;
			if (theConf->GetInt("Options/AutoUpdateTemplates", -1) != 1)
			{
				bool State = false;
				if (CCheckableMessageBox::question(theGUI, "Sandboxie-Plus", tr("To ensure optimal compatibility with your software, Sandboxie needs to update its compatibility templates. Do you want to proceed?")
					, tr("Enable auto template updates"), &State, QDialogButtonBox::Yes | QDialogButtonBox::No, QDialogButtonBox::Yes, QMessageBox::Information) == QDialogButtonBox::No)
					bCheck = false;
				if (State)
					theConf->SetValue("Options/AutoUpdateTemplates", 1);
			}
			theConf->SetValue("Options/NextCheckForUpdates", CurretnDate.addSecs(UpdateInterval).toSecsSinceEpoch());
			if (bCheck)
				UpdateTemplates();
		}
	}

	if (m_CheckMode == ePendingUpdate || m_CheckMode == ePendingInstall)
	{
		// When auto install/apply is active wait for the user to be idle
#ifndef _DEBUG
		if(GetIdleTime() > theConf->GetInt("Options/UpdateIdleTime", 30*60)) // default 30 minutes
#endif
		// and wait for no processes running in the boxes
		if (theAPI->IsConnected() && theAPI->GetAllProcesses().isEmpty())
		{
			if (m_CheckMode == ePendingUpdate)
				ApplyUpdate(eFull, true);
			else if (m_CheckMode == ePendingInstall)
				RunInstaller(true);
			m_CheckMode = eInit;
		}
	}
}

void COnlineUpdater::CheckForUpdates(bool bManual)
{
	if (m_CheckMode == eManual || m_CheckMode == eAuto)
		return; // already in progress

#ifdef _DEBUG
	if (QApplication::keyboardModifiers() & Qt::ControlModifier)
		bManual = false;
#endif

	// clean up old check result
	m_UpdateData.clear();

	m_CheckMode = bManual ? eManual : eAuto;

	QVariantMap Params;
    SB_PROGRESS Status = GetUpdates(this, SLOT(OnUpdateData(const QVariantMap&, const QVariantMap&)), Params);
	if (bManual && Status.GetStatus() == OP_ASYNC) {
		theGUI->AddAsyncOp(Status.GetValue());
		Status.GetValue()->ShowMessage(tr("Checking for updates..."));
	}
}

void COnlineUpdater::OnUpdateData(const QVariantMap& Data, const QVariantMap& Params)
{
	if (Data.isEmpty() || Data["error"].toBool()) {
		QString Error = Data.isEmpty() ? tr("server not reachable") : Data["errorMsg"].toString();
		theGUI->OnLogMessage(tr("Failed to check for updates, error: %1").arg(Error), m_CheckMode != eManual);
		if (m_CheckMode == eManual)
			QMessageBox::critical(theGUI, "Sandboxie-Plus", tr("Failed to check for updates, error: %1").arg(Error));
		m_CheckMode = eInit;
		return;
	}

	bool bNothing = true;
	bool bAuto = m_CheckMode != eManual;

	if (HandleUserMessage(Data))
		bNothing = false;

	m_UpdateData = Data;
	m_LastUpdate = QDateTime::currentDateTime();
	
	bool PendingUpdate = HandleUpdate();
	theGUI->UpdateLabel();

	if (PendingUpdate) {
		bNothing = false;
	}

	if (bAuto) {
		int UpdateInterval = theConf->GetInt("Options/UpdateInterval", UPDATE_INTERVAL); // in seconds
		theConf->SetValue("Options/NextCheckForUpdates", QDateTime::currentDateTime().addSecs(UpdateInterval).toSecsSinceEpoch());
#ifdef _DEBUG
		theGUI->AddLogMessage(tr("Update Check completed, no new updates"));
#endif
	}
	else if (bNothing)  {
		QMessageBox::information(theGUI, "Sandboxie-Plus", tr("No new updates found, your Sandboxie-Plus is up-to-date.\n"
			"\nNote: The update check is often behind the latest GitHub release to ensure that only tested updates are offered."));
	}
}

bool COnlineUpdater::HandleUpdate()
{
	QString PendingUpdate;

	QString OnNewRelease = GetOnNewReleaseOption();
	bool bNewRelease = false;
	QVariantMap Release = m_UpdateData["release"].toMap();
	QString ReleaseStr = Release["version"].toString();
	if (IsVersionNewer(ReleaseStr)) {
		if (m_CheckMode == eManual || !m_IgnoredUpdates.contains(ReleaseStr)) {
			PendingUpdate = ReleaseStr;
			bNewRelease = true;
		}
	}

	QString OnNewUpdate = GetOnNewUpdateOption();
	bool bNewUpdate = false;
	QVariantMap Update = m_UpdateData["update"].toMap();
	QString UpdateStr = Update["version"].toString();
	bool bNewer;
	if ((bNewer = IsVersionNewer(UpdateStr)) || UpdateStr == GetCurrentVersion()) {
		int iUpdate = Update["update"].toInt();
		if (iUpdate) UpdateStr += QChar('a' + (iUpdate - 1));
		if (bNewer || iUpdate > GetCurrentUpdate()) {
			if (ScanUpdateFiles(Update) == eNone) // check if this update has already been applied
				theConf->SetValue("Updater/CurrentUpdate", MakeVersionStr(Update)); // cache result
			else if (OnNewUpdate != "ignore")
			{
				if(PendingUpdate.isEmpty())
					PendingUpdate = UpdateStr;
				if (m_CheckMode == eManual || !m_IgnoredUpdates.contains(UpdateStr)) {
					bNewUpdate = true;
				}
			}
		}
	}

	theConf->SetValue("Updater/PendingUpdate", PendingUpdate);

	//
	// special case: updates allowed be to installed, but releases only allowed to be downloaded
	// solution: apply updates silently, then prompt to install new release, else prioritize installing new releases over updating the existing one
	//

	bool bAllowAuto = g_CertInfo.active && !g_CertInfo.expired; // To use automatic updates a valid certificate is required

	bool bCanRunInstaller = (m_CheckMode == eAuto && OnNewRelease == "install");
	bool bIsInstallerReady = false;
	if (bNewRelease) 
	{
		if (theConf->GetString("Updater/InstallerVersion") == MakeVersionStr(Release))
		{
			QString FilePath = theConf->GetString("Updater/InstallerPath");
			bIsInstallerReady = (!FilePath.isEmpty() && QFile::exists(FilePath));
		}

		if (!bIsInstallerReady)
		{
			// clear when not up to date
			theConf->DelValue("Updater/InstallerVersion");

			if ((bCanRunInstaller || (m_CheckMode == eAuto && OnNewRelease == "download")) || AskDownload(Release, bAllowAuto))
			{
				if (DownloadInstaller(Release, m_CheckMode == eManual))
					return true;
			}
		}
	}

	bool bCanApplyUpdate = (m_CheckMode == eAuto && OnNewUpdate == "install");
	if (bNewUpdate)
	{
		if ((!bNewRelease || (bCanApplyUpdate && !bCanRunInstaller)))
		{
			bool bIsUpdateReady = false;
			if (theConf->GetString("Updater/UpdateVersion") == MakeVersionStr(Update))
				bIsUpdateReady = QFile::exists(GetUpdateDir() + "/" UPDATE_FILE);

			if (!bIsUpdateReady)
			{
				// clear when not up to date
				theConf->DelValue("Updater/UpdateVersion");

				if ((bCanApplyUpdate || (m_CheckMode == eAuto && OnNewUpdate == "download")) || AskDownload(Update, true))
				{
					if (DownloadUpdate(Update, eFull, m_CheckMode == eManual))
						return true;
				}
			}
			else if (m_CheckMode == eManual) {
				if (ApplyUpdate(eFull, false))
					return true;
			}
			else if (bCanApplyUpdate)
				m_CheckMode = ePendingUpdate;
		}
	}

	if (bIsInstallerReady)
	{
		if (m_CheckMode == eManual) {
			if (RunInstaller(false))
				return true;
		}
		else if(bCanRunInstaller)
			m_CheckMode = ePendingInstall;
	}

	if (m_CheckMode != ePendingUpdate && m_CheckMode != ePendingInstall)
		m_CheckMode = eInit;

	return bNewRelease || bNewUpdate;
}

bool COnlineUpdater::AskDownload(const QVariantMap& Data, bool bAuto)
{
	QString VersionStr = MakeVersionStr(Data);

	QString UpdateMsg = Data["infoMsg"].toString();
	QString UpdateUrl = Data["infoUrl"].toString();
	
	QString FullMessage = !UpdateMsg.isEmpty() ? UpdateMsg : 
		tr("<p>There is a new version of Sandboxie-Plus available.<br /><font color='red'><b>New version:</b></font> <b>%1</b></p>").arg(VersionStr);

	QVariantMap Installer = Data["installer"].toMap();
	QString DownloadUrl = Installer["downloadUrl"].toString();

	enum EAction
	{
		eNone = 0,
		eDownload,
		eNotify,
	} Action = eNone;

	if (bAuto && !DownloadUrl.isEmpty()) {
		Action = eDownload;
		FullMessage += tr("<p>Do you want to download the installer?</p>");
	}
	else if (bAuto && Data.contains("files")) {
		Action = eDownload;
		FullMessage += tr("<p>Do you want to download the updates?</p>");
	}
	else if (!UpdateUrl.isEmpty()) {
		Action = eNotify;
		FullMessage += tr("<p>Do you want to go to the <a href=\"%1\">download page</a>?</p>").arg(UpdateUrl);
	}

	CCheckableMessageBox mb(theGUI);
	mb.setWindowTitle("Sandboxie-Plus");
	QIcon ico(QLatin1String(":/SandMan.png"));
	mb.setIconPixmap(ico.pixmap(64, 64));
	//mb.setTextFormat(Qt::RichText);
	mb.setText(FullMessage);
	mb.setCheckBoxText(tr("Don't show this update anymore."));
	mb.setCheckBoxVisible(m_CheckMode != eManual);

	if (Action != eNone) {
		mb.setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No | QDialogButtonBox::Cancel);
		mb.setDefaultButton(QDialogButtonBox::Yes);
	} else
		mb.setStandardButtons(QDialogButtonBox::Ok);

	mb.exec();

	if (mb.clickedStandardButton() == QDialogButtonBox::Yes)
	{
		if (Action == eDownload) 
		{
			m_CheckMode = eManual;
			return true;
		}
		else
			QDesktopServices::openUrl(UpdateUrl);
	}
	else 
	{
		if (mb.clickedStandardButton() == QDialogButtonBox::Cancel) 
		{
			theConf->SetValue("Updater/PendingUpdate", ""); 
			theGUI->UpdateLabel();
		}

		if (mb.isChecked())
			theConf->SetValue("Options/IgnoredUpdates", m_IgnoredUpdates << VersionStr);
	}
	return false;
}

COnlineUpdater::EUpdateScope COnlineUpdater::GetFileScope(const QString& Path)
{
	static const WCHAR CoreFiles[] = SCOPE_CORE_FILES;
	static const WCHAR LangFiles[] = SCOPE_LANG_FILES;
	static const WCHAR TmplFiles[] = SCOPE_TMPL_FILES;

	auto WildMatch = [Path](const WCHAR* pFiles) {
		for (const WCHAR* pFile = pFiles; *pFile; pFile += wcslen(pFile) + 1) {
			QString WC = QRegularExpression::wildcardToRegularExpression(QString::fromWCharArray(pFile));
			QRegularExpression RegExp(WC, QRegularExpression::CaseInsensitiveOption);
			if (RegExp.match(Path).hasMatch())
				return true;
		}
		return false;
	};

	if (WildMatch(CoreFiles))
		return eCore;
	if (WildMatch(TmplFiles) || WildMatch(LangFiles))
		return eMeta;
	// unknown files are handled the same as known Plus files
	return eFull;
}

COnlineUpdater::EUpdateScope COnlineUpdater::ScanUpdateFiles(const QVariantMap& Update)
{
	QString AppDir = QApplication::applicationDirPath();
#ifdef DUMMY_PATH
	AppDir = DUMMY_PATH;
#endif

	EUpdateScope Scope = eNone;

	foreach(const QVariant vFile, Update["files"].toList()) {
		QVariantMap File = vFile.toMap();
		QCryptographicHash qHash(QCryptographicHash::Sha256);
		QFile qFile(AppDir + "\\" + File["path"].toString());
		if (qFile.open(QFile::ReadOnly)) {
			qHash.addData(&qFile);
			qFile.close();
		}
		if (qHash.result() == QByteArray::fromHex(File["hash"].toByteArray()))
			continue; // file did not change
		
		EUpdateScope CurScope = GetFileScope(File["path"].toString());
		if (Scope < CurScope)
			Scope = CurScope;
	}

	return Scope;
}

bool COnlineUpdater::DownloadUpdate(const QVariantMap& Update, EUpdateScope Scope, bool bAndApply)
{
	QJsonDocument doc(QJsonValue::fromVariant(Update).toObject());			
	WriteStringToFile(GetUpdateDir(true) + "/" UPDATE_FILE, doc.toJson());
	
	theConf->DelValue("Updater/UpdateVersion");

	QStringList Params;
	Params.append("update");
	Params.append("sandboxie-plus");
	Params.append("/step:prepare");
	if(Scope == eTmpl)
		Params.append("/scope:tmpl");
	else if(Scope == eMeta)
		Params.append("/scope:meta");
	Params.append("/embedded");
	Params.append("/temp:" + GetUpdateDir().replace("/", "\\"));
#ifdef DUMMY_PATH
	Params.append("/path:" DUMMY_PATH);
#endif

	m_pUpdaterUtil = new QProcess(this);
	m_pUpdaterUtil->setProperty("apply", bAndApply);
	m_pUpdaterUtil->setProperty("tmpl", Scope == eTmpl);
	m_pUpdaterUtil->setProperty("version", MakeVersionStr(Update));
	m_pUpdaterUtil->setProgram(QApplication::applicationDirPath() + "/UpdUtil.exe");
	m_pUpdaterUtil->setArguments(Params);
	connect(m_pUpdaterUtil, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(OnPrepareFinished(int, QProcess::ExitStatus)));
	connect(m_pUpdaterUtil, SIGNAL(readyReadStandardOutput()), this, SLOT(OnPrepareOutput()));
	connect(m_pUpdaterUtil, SIGNAL(readyReadStandardError()), this, SLOT(OnPrepareError()));
	m_pUpdaterUtil->start();

	if (m_pUpdaterUtil->state() != QProcess::Running)
		return false;

	m_pUpdateProgress = CSbieProgressPtr(new CSbieProgress());
	connect(m_pUpdateProgress.data(), &CSbieProgress::Canceled, this, [&]() {
		if (m_pUpdaterUtil && m_pUpdaterUtil->state() == QProcess::Running)
				m_pUpdaterUtil->terminate();
	});
	theGUI->AddAsyncOp(m_pUpdateProgress);
	m_pUpdateProgress->ShowMessage(tr("Downloading updates..."));

	return true;
}

void COnlineUpdater::OnPrepareOutput()
{
	QProcess* pProcess = (QProcess*)sender();
	QByteArray Text = pProcess->readAllStandardOutput();
	qDebug() << "UPD-OUT:\t" << Text;

	if (!m_pUpdateProgress.isNull())
		m_pUpdateProgress->ShowMessage(Text.trimmed());
}

void COnlineUpdater::OnPrepareError()
{
	QProcess* pProcess = (QProcess*)sender();
	QByteArray Text = pProcess->readAllStandardOutput();
	qDebug() << "UPD-ERR:\t" << Text;
}

QString GetUpdErrorStr(int exitCode)
{
	switch (exitCode)
	{
	case ERROR_INVALID: return COnlineUpdater::tr("invalid parameter");
	case ERROR_GET: return COnlineUpdater::tr("failed to download updated information");
	case ERROR_LOAD: return COnlineUpdater::tr("failed to load updated json file");
	case ERROR_DOWNLOAD: return COnlineUpdater::tr("failed to download a particular file");
	case ERROR_SCAN: return COnlineUpdater::tr("failed to scan existing installation");
	case ERROR_SIGN: return COnlineUpdater::tr("updated signature is invalid !!!");
	case ERROR_HASH: return COnlineUpdater::tr("downloaded file is corrupted");
	case ERROR_INTERNAL: return COnlineUpdater::tr("internal error");
	default: return COnlineUpdater::tr("unknown error");
	}
}

void COnlineUpdater::OnPrepareFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	QProcess* pProcess = (QProcess*)sender();
	if (pProcess != m_pUpdaterUtil) {
		pProcess->deleteLater();
		return;
	}
	bool bAndApply = pProcess->property("apply").toBool();
	bool bTmplOnly = pProcess->property("tmpl").toBool();
	QString VersionStr = pProcess->property("version").toString();

	m_pUpdaterUtil->deleteLater();
	m_pUpdaterUtil = NULL;

	if (m_pUpdateProgress.isNull())
		return; // canceled

	m_pUpdateProgress->Finish(SB_OK);
	m_pUpdateProgress.clear();

	if (exitCode < 0) {
		QMessageBox::critical(theGUI, "Sandboxie-Plus", tr("Failed to download updates from server, error %1").arg(GetUpdErrorStr(exitCode)));
		return; // failed
	}

	theConf->SetValue("Updater/UpdateVersion", VersionStr);

	if (bAndApply)
		ApplyUpdate(bTmplOnly ? eTmpl : eFull, false);
	else 
	{
		HandleUpdate();
		theGUI->UpdateLabel();
	}
}

bool COnlineUpdater::ApplyUpdate(EUpdateScope Scope, bool bSilent)
{
	if (Scope != eTmpl)
	{
		if (!ShowCertWarningIfNeeded())
			return false;

		if (!bSilent)
		{
			QString Message = tr("<p>Updates for Sandboxie-Plus have been downloaded.</p><p>Do you want to apply these updates? If any programs are running sandboxed, they will be terminated.</p>");
			int Ret = QMessageBox("Sandboxie-Plus", Message, QMessageBox::Information, QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape, QMessageBox::Cancel, theGUI).exec();
			if (Ret == QMessageBox::Cancel) {
				theConf->DelValue("Updater/UpdateVersion");
				theGUI->UpdateLabel();
			}
			if (Ret != QMessageBox::Yes)
				return false;
		}

		QVariantMap Update = QJsonDocument::fromJson(ReadFileAsString(GetUpdateDir() + "/" UPDATE_FILE).toUtf8()).toVariant().toMap();
		Scope = ScanUpdateFiles(Update);
		if (Scope == eNone)
			return true; // nothing to do
	}

	if(Scope != eMeta)
		theAPI->TerminateAll();

	QStringList Params;
	Params.append("update");
	Params.append("sandboxie-plus");
	Params.append("/step:apply");
	if(Scope == eTmpl)
		Params.append("/scope:tmpl");
	else if(Scope == eMeta)
		Params.append("/scope:meta");
	else
		Params.append("/restart");
#ifndef _DEBUG
	Params.append("/embedded");
#else
	Params.append("/pause");
#endif
	Params.append("/temp:" + GetUpdateDir().replace("/", "\\"));
#ifdef DUMMY_PATH
	Params.append("/path:" DUMMY_PATH);
#endif
	if (Scope == eFull)
		Params.append("/open:sandman.exe");

	SB_RESULT(int) status = RunUpdater(Params, true, Scope != eFull);
	if (!status.IsError()) {
		if(bSilent)
			theConf->DelValue("Updater/UpdateVersion");
		if (Scope == eTmpl || Scope == eMeta)
			theAPI->ReloadConfig();
		else if (Scope == eFull)
			QApplication::quit();
		else
			theGUI->ConnectSbie();
		return true;
	}
	return false;
}

SB_RESULT(int) COnlineUpdater::RunUpdater(const QStringList& Params, bool bSilent, bool Wait)
{
	if (bSilent) {
		SB_RESULT(int) Result = theAPI->RunUpdateUtility(Params, 2, Wait);
		if (!Result.IsError())
			return Result;
		// else fallback to ShellExecuteEx
		if (theConf->GetBool("Options/UpdateNoFallback", false))
			return Result;
	}

	std::wstring wFile = QString(QApplication::applicationDirPath() + "/UpdUtil.exe").replace("/", "\\").toStdWString();
	std::wstring wParams;
	foreach(const QString & Param, Params) {
		if (!wParams.empty()) wParams.push_back(L' ');
		wParams += L"\"" + Param.toStdWString() + L"\"";
	}

	int ExitCode = RunElevated(wFile, wParams, Wait ? INFINITE : 0);
	if (ExitCode == STATUS_PENDING && !Wait)
		ExitCode = 0;
	return CSbieResult<int>(ExitCode);
}

bool COnlineUpdater::DownloadInstaller(const QVariantMap& Release, bool bAndRun)
{
	if (m_RequestManager == NULL) 
		m_RequestManager = new CNetworkAccessManager(30 * 1000, this);

	QVariantMap Installer = Release["installer"].toMap();
	QString DownloadUrl = Installer["downloadUrl"].toString();
	if (DownloadUrl.isEmpty())
		return false;

	// clean up old installer if present
	QString FilePath = theConf->GetString("Updater/InstallerPath");
	if (!FilePath.isEmpty()) {
		QFile::remove(FilePath);
		QFile::remove(FilePath + ".sig");
		theConf->DelValue("Updater/InstallerPath");
	}

	QVariantMap Params;
	Params["run"] = bAndRun;
	Params["version"] = MakeVersionStr(Release);
	Params["signature"] = Installer["signature"];
    SB_PROGRESS Status = DownloadFile(DownloadUrl, this, SLOT(OnInstallerDownload(const QString&, const QVariantMap&)), Params);
	if (Status.GetStatus() == OP_ASYNC) {
		theGUI->AddAsyncOp(Status.GetValue());
		Status.GetValue()->ShowMessage(tr("Downloading installer..."));
	}

	return true;
}

void COnlineUpdater::OnInstallerDownload(const QString& Path, const QVariantMap& Params)
{
	bool bAndRun = Params["run"].toBool();
	QString VersionStr = Params["version"].toString();
	QByteArray Signature = Params["signature"].toByteArray();

	QFile SigFile(Path + ".sig");
	if (SigFile.open(QFile::WriteOnly)) {
		SigFile.write(QByteArray::fromBase64(Signature));
		SigFile.close();
	}

	theConf->SetValue("Updater/InstallerVersion", VersionStr);
	theConf->SetValue("Updater/InstallerPath", Path);

	if (bAndRun)
		RunInstaller(false);
	else 
	{
		HandleUpdate();
		theGUI->UpdateLabel();
	}
}

bool COnlineUpdater::RunInstaller(bool bSilent)
{
	if (!ShowCertWarningIfNeeded())
		return false;

	QString FilePath = theConf->GetString("Updater/InstallerPath");
	if (FilePath.isEmpty() || !QFile::exists(FilePath)) {
		theConf->DelValue("Updater/InstallerPath");
		theConf->DelValue("Updater/InstallerVersion");
		return false;
	}

	if (!bSilent) {
		QString Message = tr("<p>A new Sandboxie-Plus installer has been downloaded to the following location:</p><p><a href=\"%2\">%1</a></p><p>Do you want to begin the installation? If any programs are running sandboxed, they will be terminated.</p>")
			.arg(FilePath).arg("File:///" + Split2(FilePath, "/", true).first);
		int Ret = QMessageBox("Sandboxie-Plus", Message, QMessageBox::Information, QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape, QMessageBox::Cancel, theGUI).exec();
		if (Ret == QMessageBox::Cancel) {
			QFile::remove(FilePath);
			QFile::remove(FilePath + ".sig");
			theConf->DelValue("Updater/InstallerPath");
			theGUI->UpdateLabel();
		}
		if (Ret != QMessageBox::Yes)
			return false;
	}

	theAPI->TerminateAll();

	if (RunInstaller2(FilePath, true)) {
		if (bSilent)
			theConf->DelValue("Updater/InstallerVersion");
		QApplication::quit();
		return true;
	}
	return false;
}

void COnlineUpdater::UpdateTemplates()
{
	QVariantMap Params;
    SB_PROGRESS Status = GetUpdates(this, SLOT(OnUpdateDataTmpl(const QVariantMap&, const QVariantMap&)), Params);
	//if (Status.GetStatus() == OP_ASYNC) {
	//	theGUI->AddAsyncOp(Status.GetValue());
	//	Status.GetValue()->ShowMessage(tr("Checking for updates..."));
	//}
}

void COnlineUpdater::OnUpdateDataTmpl(const QVariantMap& Data, const QVariantMap& Params)
{
	QVariantMap Release = Data["release"].toMap();

	QByteArray TemplatesHash;
	foreach(const QVariant vFile, Release["files"].toList()) {
		QVariantMap File = vFile.toMap();
		if (File["path"].toString() == "Templates.ini")
			TemplatesHash = QByteArray::fromHex(File["hash"].toByteArray());
	}
	if (TemplatesHash.isEmpty())
		return; // fine not found

	QString AppDir = QApplication::applicationDirPath();
#ifdef DUMMY_PATH
	AppDir = DUMMY_PATH;
#endif

	QCryptographicHash qHash(QCryptographicHash::Sha256);
	QFile qFile(QApplication::applicationDirPath() + "\\Templates.ini");
	if (qFile.open(QFile::ReadOnly)) {
		qHash.addData(&qFile);
		qFile.close();
	}
	if (qHash.result() == TemplatesHash)
		return; // no update

	if (QMessageBox::question(theGUI, "Sandboxie-Plus", tr("There is a new Templates.ini available, do you want to download it?"), QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes)
		return;

	DownloadUpdate(Release, eTmpl, true);
}

bool COnlineUpdater::RunInstaller2(const QString& FilePath, bool bSilent)
{
	if (bSilent && !theGUI->IsFullyPortable()) 
	{
		QStringList Params;
		Params.append("run_setup");
		Params.append(QString(FilePath).replace("/", "\\"));
#ifndef _DEBUG_
		Params.append("/embedded");
#else
		Params.append("/pause");
#endif

		SB_RESULT(int) Result = theAPI->RunUpdateUtility(Params, 1);
		if (!Result.IsError())
			return true;
		// else fallback to ShellExecuteEx
		if (theConf->GetBool("Options/UpdateNoFallback", false))
			return false;
	}

	std::wstring wFile = QString(FilePath).replace("/", "\\").toStdWString();
	std::wstring wParams;
	if(theGUI->IsFullyPortable())
		wParams = L"/PORTABLE=1";
#ifndef _DEBUG
	else
		wParams = L"/SILENT";
#endif

	return RunElevated(wFile, wParams) == 0;
}

bool COnlineUpdater::HandleUserMessage(const QVariantMap& Data)
{
	QString UserMsg = Data["userMsg"].toString();
	if (!UserMsg.isEmpty())
	{
		QString MsgHash = QCryptographicHash::hash(Data["userMsg"].toByteArray(), QCryptographicHash::Md5).toHex().left(8);
		if (!m_IgnoredUpdates.contains(MsgHash))
		{
			QString FullMessage = UserMsg;
			QString InfoUrl = Data["infoUrl"].toString();
			if (!InfoUrl.isEmpty())
				FullMessage += tr("<p>Do you want to go to the <a href=\"%1\">info page</a>?</p>").arg(InfoUrl);

			CCheckableMessageBox mb(theGUI);
			mb.setWindowTitle("Sandboxie-Plus");
			
			QByteArray MsgIcon = QByteArray::fromBase64(Data["msgIcon"].toByteArray());
			if (!MsgIcon.isEmpty())
			{
				QPixmap pixmap;
				pixmap.loadFromData(MsgIcon, "PNG");
				mb.setIconPixmap(pixmap);
			}
			else
			{
				QIcon ico(QLatin1String(":/SandMan.png"));
				mb.setIconPixmap(ico.pixmap(64, 64));
			}
			
			//mb.setTextFormat(Qt::RichText);
			mb.setText(UserMsg);
			mb.setCheckBoxText(tr("Don't show this announcement in the future."));
			
			if (!InfoUrl.isEmpty()) {
				mb.setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No);
				mb.setDefaultButton(QDialogButtonBox::Yes);
			}
			else
				mb.setStandardButtons(QDialogButtonBox::Ok);

			mb.exec();

			if (mb.isChecked())
				theConf->SetValue("Options/IgnoredUpdates", m_IgnoredUpdates << MsgHash);

			if (mb.clickedStandardButton() == QDialogButtonBox::Yes)
			{
				QDesktopServices::openUrl(InfoUrl);
			}

			return true;
		}
	}
	return false;
}

QString COnlineUpdater::GetUpdateDir(bool bCreate)
{
	QString TempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
	if (TempDir.right(1) != "/")
		TempDir += "/";
	TempDir += "sandboxie-updater";
	// Note: must not end with a /
	if(bCreate)
		QDir().mkpath(TempDir);
	return TempDir;
}

QString COnlineUpdater::MakeVersionStr(const QVariantMap& Data)
{
	QString Str = Data["version"].toString();
	int iUpdate = Data["update"].toInt();
	if (iUpdate) Str += QChar('a' + (iUpdate - 1));
	return Str;
}

QString COnlineUpdater::ParseVersionStr(const QString& Str, int* pUpdate)
{
	int pos = Str.indexOf(QRegularExpression("[a-zA-Z]"));
	if (pos == -1)
		return Str;
	QString Ver = Str.left(pos);
	if (pUpdate) {
		QString Tmp = Str.mid(pos);
		*pUpdate = Tmp[0].toLatin1() - 'a' + 1;
	}
	return Ver;
}

QString COnlineUpdater::GetCurrentVersion()
{
	return QString::number(VERSION_MJR) + "." + QString::number(VERSION_MIN) + "." + QString::number(VERSION_REV);
}

int COnlineUpdater::GetCurrentUpdate()
{
	int iUpdate = 0;
	QString Version = ParseVersionStr(theConf->GetString("Updater/CurrentUpdate", 0), &iUpdate);
	if(Version != GetCurrentVersion() || iUpdate < VERSION_UPD)
		iUpdate = VERSION_UPD;
	return iUpdate;
}

quint32 COnlineUpdater::CurrentVersion()
{
	//quint8 myVersion[4] = { VERSION_UPD, VERSION_REV, VERSION_MIN, VERSION_MJR }; // ntohl
	quint8 myVersion[4] = { 0, VERSION_REV, VERSION_MIN, VERSION_MJR }; // ntohl
	quint32 MyVersion = *(quint32*)&myVersion;
	return MyVersion;
}

quint32 COnlineUpdater::VersionToInt(const QString& VersionStr)
{
	quint32 Version = 0;
	QStringList Nums = VersionStr.split(".");
	for (int i = 0, Bits = 24; i < Nums.count() && Bits >= 0; i++, Bits -= 8)
		Version |= (Nums[i].toInt() & 0xFF) << Bits;
	return Version;
}

bool COnlineUpdater::IsVersionNewer(const QString& VersionStr)
{
	if (VersionStr.isEmpty())
		return false;

#ifdef INSIDER_BUILD
	QString sVersion = VersionStr;
	if (sVersion[4] == ' ') sVersion[4] = '0';
	QDateTime VersionDate = QDateTime::fromString(sVersion, "MMM dd yyyy");
	
	sVersion = QString(__DATE__);
	if (sVersion[4] == ' ') sVersion[4] = '0';
	QDateTime BuildDate = QDateTime::fromString(sVersion, "MMM dd yyyy");

	return (VersionDate > BuildDate);
#else
	return VersionToInt(VersionStr) > CurrentVersion();
#endif
}
