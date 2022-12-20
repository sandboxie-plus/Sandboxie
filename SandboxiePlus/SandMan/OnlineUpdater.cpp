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
#include "../../SandboxieLive/UpdUtil/UpdUtil.h"
#include <QCryptographicHash>
#include "Helpers/WinAdmin.h"
#include <windows.h>

#ifdef _DEBUG

// mess with a dummy installation when debugging

#undef VERSION_MJR
#define VERSION_MJR		1
#undef VERSION_MIN
#define VERSION_MIN 	5
#undef VERSION_REV
#define VERSION_REV 	3
#undef VERSION_UPD
#define VERSION_UPD 	0

#define DUMMY_PATH "C:\\Projects\\Sandboxie\\SandboxieLive\\x64\\Debug\\Test"
#endif

DWORD GetIdleTime() // in seconds
{
	LASTINPUTINFO lastInPut;
	GetLastInputInfo(&lastInPut);
    return (GetTickCount() - lastInPut.dwTime) / 1000;
}

COnlineUpdater::COnlineUpdater(QObject *parent) : QObject(parent) 
{
	m_IgnoredUpdates = theConf->GetStringList("Options/IgnoredUpdates");

	m_RequestManager = NULL;
	m_pUpdaterUtil = NULL;

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
	QString OnNewRelease = theConf->GetString("Options/OnNewRelease", "download");
	bool bCanRunInstaller = OnNewRelease == "install";

	bool bIsUpdateReady = false;
	QVariantMap Update = QJsonDocument::fromJson(ReadFileAsString(GetUpdateDir(true) + "/" UPDATE_FILE).toUtf8()).toVariant().toMap();
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
	QString OnNewUpdate = theConf->GetString("Options/OnNewUpdate", "ignore");
	bool bCanApplyUpdate = OnNewUpdate == "install";

	if (bIsInstallerReady && bCanRunInstaller)
		m_CheckMode = ePendingInstall;
	else if (bIsUpdateReady && bCanApplyUpdate)
		m_CheckMode = ePendingUpdate;
}

void COnlineUpdater::Process() 
{
	int iCheckUpdates = theConf->GetInt("Options/CheckForUpdates", 2);
	if (iCheckUpdates != 0)
	{
		time_t NextUpdateCheck = theConf->GetUInt64("Options/NextCheckForUpdates", 0);
		if (NextUpdateCheck == 0)
			theConf->SetValue("Options/NextCheckForUpdates", QDateTime::currentDateTime().addDays(7).toSecsSinceEpoch());
		else if(QDateTime::currentDateTime().toSecsSinceEpoch() >= NextUpdateCheck)
		{
			if (iCheckUpdates == 2)
			{
				bool bCheck = false;
				iCheckUpdates = CCheckableMessageBox::question(theGUI, "Sandboxie-Plus", tr("Do you want to check if there is a new version of Sandboxie-Plus?")
					, tr("Don't show this message again."), &bCheck, QDialogButtonBox::Yes | QDialogButtonBox::No, QDialogButtonBox::Yes, QMessageBox::Information) == QDialogButtonBox::Ok ? 1 : 0;

				if (bCheck)
					theConf->SetValue("Options/CheckForUpdates", iCheckUpdates);
			}

			if (iCheckUpdates == 0)
				theConf->SetValue("Options/NextCheckForUpdates", QDateTime::currentDateTime().addDays(7).toSecsSinceEpoch());
			else
			{
				theConf->SetValue("Options/NextCheckForUpdates", QDateTime::currentDateTime().addDays(1).toSecsSinceEpoch());
				
				CheckForUpdates(false);
			}
		}
	}

	if (!m_pUpdateProgress.isNull() && m_RequestManager != NULL) {
		if (m_pUpdateProgress->IsCanceled()) {
			m_pUpdateProgress->Finish(SB_OK);
			m_pUpdateProgress.clear();

			m_RequestManager->AbortAll();

			if (m_pUpdaterUtil && m_pUpdaterUtil->state() == QProcess::Running)
				m_pUpdaterUtil->terminate();
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
				ApplyUpdate(true);
			else if (m_CheckMode == ePendingInstall)
				RunInstaller(true);
			m_CheckMode = eInit;
		}
	}
}

void COnlineUpdater::GetUpdates(QObject* receiver, const char* member, const QVariantMap& Params)
{
	CGetUpdatesJob* pJob = new CGetUpdatesJob(Params, this);
	QObject::connect(pJob, SIGNAL(UpdateData(const QVariantMap&, const QVariantMap&)), receiver, member, Qt::QueuedConnection);

	if (m_RequestManager == NULL) 
		m_RequestManager = new CNetworkAccessManager(30 * 1000, this);

	QUrlQuery Query;
	Query.addQueryItem("action", "update");
	Query.addQueryItem("software", "sandboxie-plus");
	//QString Branche = theConf->GetString("Options/ReleaseBranche");
	//if (!Branche.isEmpty())
	//	Query.addQueryItem("branche", Branche);
	//Query.addQueryItem("version", theGUI->GetVersion());
	//Query.addQueryItem("version", QString::number(VERSION_MJR) + "." + QString::number(VERSION_MIN) + "." + QString::number(VERSION_REV) + "." + QString::number(VERSION_UPD));
	Query.addQueryItem("version", QString::number(VERSION_MJR) + "." + QString::number(VERSION_MIN) + "." + QString::number(VERSION_REV));
	Query.addQueryItem("system", "windows-" + QSysInfo::kernelVersion() + "-" + QSysInfo::currentCpuArchitecture());
	Query.addQueryItem("language", QString::number(theGUI->m_LanguageId));

	QString UpdateKey = GetArguments(g_Certificate, L'\n', L':').value("UPDATEKEY");
	if (UpdateKey.isEmpty())
		UpdateKey = theAPI->GetGlobalSettings()->GetText("UpdateKey"); // theConf->GetString("Options/UpdateKey");
	if (!UpdateKey.isEmpty())
		Query.addQueryItem("update_key", UpdateKey);

	if (Params.contains("channel")) 
		Query.addQueryItem("channel", Params["channel"].toString());
	else {
		QString ReleaseChannel = theConf->GetString("Options/ReleaseChannel", "stable");
		Query.addQueryItem("channel", ReleaseChannel);
	}

	if(Params.contains("manual")) Query.addQueryItem("auto", Params["manual"].toBool() ? "0" : "1");

	//QString Test = Query.toString();

	QUrl Url("https://sandboxie-plus.com/update.php");
	Url.setQuery(Query);

	QNetworkRequest Request = QNetworkRequest(Url);
	//Request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
	Request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
	//Request.setRawHeader("Accept-Encoding", "gzip");
	QNetworkReply* pReply = m_RequestManager->get(Request);
	connect(pReply, SIGNAL(finished()), this, SLOT(OnUpdateCheck()));

	m_JobQueue.insert(pReply, pJob);
}

void COnlineUpdater::CheckForUpdates(bool bManual)
{
	if (!m_pUpdateProgress.isNull())
		return;

#ifdef _DEBUG
	if (QApplication::keyboardModifiers() & Qt::ControlModifier)
		bManual = false;
#endif

	m_pUpdateProgress = CSbieProgressPtr(new CSbieProgress());
	theGUI->AddAsyncOp(m_pUpdateProgress);
	m_pUpdateProgress->ShowMessage(tr("Checking for updates..."));

	// clean up old check result
	m_UpdateData.clear();

	m_CheckMode = bManual ? eManual : eAuto;
	GetUpdates(this, SLOT(OnUpdateData(const QVariantMap&, const QVariantMap&)));
}

void COnlineUpdater::OnUpdateCheck()
{
	QNetworkReply* pReply = qobject_cast<QNetworkReply*>(sender());
	QByteArray Reply = pReply->readAll();
	pReply->deleteLater();

	CGetUpdatesJob* pJob = m_JobQueue.take(pReply);
	if (!pJob)
		return;

	QVariantMap Data = QJsonDocument::fromJson(Reply).toVariant().toMap();

	emit pJob->UpdateData(Data, pJob->m_Params);
	pJob->deleteLater();
}

void COnlineUpdater::OnDownloadProgress(qint64 bytes, qint64 bytesTotal)
{
	if (bytesTotal != 0 && !m_pUpdateProgress.isNull())
		m_pUpdateProgress->Progress(100 * bytes / bytesTotal);
}

void COnlineUpdater::OnUpdateData(const QVariantMap& Data, const QVariantMap& Params)
{
	if (!m_pUpdateProgress.isNull()) {
		m_pUpdateProgress->Finish(SB_OK);
		m_pUpdateProgress.clear();
	}

	if (Data.isEmpty() || Data["error"].toBool()) {
		QString Error = Data.isEmpty() ? tr("server not reachable") : Data["errorMsg"].toString();
		theGUI->OnLogMessage(tr("Failed to check for updates, error: %1").arg(Error), m_CheckMode != eManual);
		if (m_CheckMode == eManual)
			QMessageBox::critical(theGUI, "Sandboxie-Plus", tr("Failed to check for updates, error: %1").arg(Error));
		return;
	}

	bool bNothing = true;

	if (HandleUserMessage(Data))
		bNothing = false;

	m_UpdateData = Data;
	
	bool PendingUpdate = HandleUpdate();
	theGUI->UpdateLabel();

	if (PendingUpdate) {
		bNothing = false;
	}

	if (bNothing)  {

		//theConf->SetValue("Options/NextCheckForUpdates", QDateTime::currentDateTime().addDays(7).toSecsSinceEpoch());

		if (m_CheckMode == eManual) {
			QMessageBox::information(theGUI, "Sandboxie-Plus", tr("No new updates found, your Sandboxie-Plus is up-to-date.\n"
				"\nNote: The update check is often behind the latest GitHub release to ensure that only tested updates are offered."));
		}
	}
}

bool COnlineUpdater::HandleUpdate()
{
	QString PendingUpdate;

	bool bNewRelease = false;
	QVariantMap Release = m_UpdateData["release"].toMap();
	QString ReleaseStr = Release["version"].toString();
	if (IsVersionNewer(ReleaseStr)) {
		if (m_CheckMode == eManual || !m_IgnoredUpdates.contains(ReleaseStr)) {
			PendingUpdate = ReleaseStr;
			bNewRelease = true;
		}
	}

	QString OnNewUpdate = theConf->GetString("Options/OnNewUpdate", "ignore");

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

	QString OnNewRelease = theConf->GetString("Options/OnNewRelease", "download");
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

			if ((bCanRunInstaller || (m_CheckMode == eAuto && OnNewRelease == "download")) || AskDownload(Release))
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
				bIsUpdateReady = QFile::exists(GetUpdateDir(true) + "/" UPDATE_FILE);

			if (!bIsUpdateReady)
			{
				// clear when not up to date
				theConf->DelValue("Updater/UpdateVersion");

				if ((bCanApplyUpdate || (m_CheckMode == eAuto && OnNewUpdate == "download")) || AskDownload(Update))
				{
					if (DownloadUpdate(Update, m_CheckMode == eManual))
						return true;
				}
			}
			else if (m_CheckMode == eManual) {
				if (ApplyUpdate(false))
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

	return bNewRelease || bNewUpdate;
}

bool COnlineUpdater::AskDownload(const QVariantMap& Data)
{
	QString VersionStr = MakeVersionStr(Data);

	QString UpdateMsg = Data["infoMsg"].toString();
	QString UpdateUrl = Data["infoUrl"].toString();
	
	QString FullMessage = !UpdateMsg.isEmpty() ? UpdateMsg : 
		tr("<p>There is a new version of Sandboxie-Plus available.<br /><font color='red'><b>New version:</b></font> <b>%1</b></p>").arg(VersionStr);

	QVariantMap Installer = Data["installer"].toMap();
	QString DownloadUrl = Installer["downloadUrl"].toString();

	if (!DownloadUrl.isEmpty())
		FullMessage += tr("<p>Do you want to download the installer?</p>");
	else if(Data.contains("files"))
		FullMessage += tr("<p>Do you want to download the updates?</p>");
	else if (!UpdateUrl.isEmpty())
		FullMessage += tr("<p>Do you want to go to the <a href=\"%1\">update page</a>?</p>").arg(UpdateUrl);

	CCheckableMessageBox mb(theGUI);
	mb.setWindowTitle("Sandboxie-Plus");
	QIcon ico(QLatin1String(":/SandMan.png"));
	mb.setIconPixmap(ico.pixmap(64, 64));
	//mb.setTextFormat(Qt::RichText);
	mb.setText(FullMessage);
	mb.setCheckBoxText(tr("Don't show this update anymore."));
	mb.setCheckBoxVisible(m_CheckMode != eManual);

	if (!UpdateUrl.isEmpty() || !DownloadUrl.isEmpty() || Data.contains("files")) {
		mb.setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No);
		mb.setDefaultButton(QDialogButtonBox::Yes);
	}
	else
		mb.setStandardButtons(QDialogButtonBox::Ok);

	mb.exec();

	if (mb.clickedStandardButton() == QDialogButtonBox::Yes)
	{
		if (!DownloadUrl.isEmpty() || Data.contains("files")) {
			m_CheckMode = eManual;
			return true;
		}
		else
			QDesktopServices::openUrl(UpdateUrl);
	}
	else if (mb.isChecked())
		theConf->SetValue("Options/IgnoredUpdates", m_IgnoredUpdates << VersionStr);
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

bool COnlineUpdater::DownloadUpdate(const QVariantMap& Update, bool bAndApply)
{
	QJsonDocument doc(QJsonValue::fromVariant(Update).toObject());			
	WriteStringToFile(GetUpdateDir(true) + "/" UPDATE_FILE, doc.toJson());
	
	theConf->DelValue("Updater/UpdateVersion");

	QStringList Params;
	Params.append("update");
	Params.append("sandboxie-plus");
	Params.append("/step:prepare");
	Params.append("/embedded");
	Params.append("/temp:" + GetUpdateDir().replace("/", "\\"));
#ifdef DUMMY_PATH
	Params.append("/path:" DUMMY_PATH);
#endif

	m_pUpdaterUtil = new QProcess(this);
	m_pUpdaterUtil->setProperty("apply", bAndApply);
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
		ApplyUpdate(false);
	else 
	{
		HandleUpdate();
		theGUI->UpdateLabel();
	}
}

bool COnlineUpdater::ApplyUpdate(bool bSilent)
{
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

	QVariantMap Update = QJsonDocument::fromJson(ReadFileAsString(GetUpdateDir(true) + "/" UPDATE_FILE).toUtf8()).toVariant().toMap();
	EUpdateScope Scope =  ScanUpdateFiles(Update);
	if (Scope == eNone)
		return true; // nothing to do

	if(Scope != eMeta)
		theAPI->TerminateAll();

	QStringList Params;
	Params.append("update");
	Params.append("sandboxie-plus");
	Params.append("/step:apply");
	if(Scope == eMeta)
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

	if (RunUpdater(Params, bSilent, Scope != eFull)) {
		if(bSilent)
			theConf->DelValue("Updater/UpdateVersion");
		if (Scope == eMeta)
			theAPI->ReloadConfig();
		else if (Scope == eFull)
			QApplication::quit();
		else
			theGUI->ConnectSbie();
		return true;
	}
	return false;
}

bool COnlineUpdater::RunUpdater(const QStringList& Params, bool bSilent, bool Wait)
{
	if (bSilent) {
		SB_RESULT(int) Result = theAPI->RunUpdateUtility(Params, 2, Wait);
		if (!Result.IsError())
			return true;
		// else fallback to ShellExecuteEx
		if (theConf->GetBool("Options/UpdateNoFallback", false))
			return false;
	}

	std::wstring wFile = QString(QApplication::applicationDirPath() + "/UpdUtil.exe").replace("/", "\\").toStdWString();
	std::wstring wParams;
	foreach(const QString & Param, Params) {
		if (!wParams.empty()) wParams.push_back(L' ');
		wParams += L"\"" + Param.toStdWString() + L"\"";
	}

	return RunElevated(wFile, wParams, Wait ? INFINITE : 0) == 0;
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

	QNetworkRequest Request = QNetworkRequest(DownloadUrl);
	//Request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
	Request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
	//Request.setRawHeader("Accept-Encoding", "gzip");
	QNetworkReply* pReply = m_RequestManager->get(Request);
	pReply->setProperty("run", bAndRun);
	pReply->setProperty("version", MakeVersionStr(Release));
	pReply->setProperty("signature", Installer["signature"]);
	connect(pReply, SIGNAL(finished()), this, SLOT(OnInstallerDownload()));
	connect(pReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(OnDownloadProgress(qint64, qint64)));

	m_pUpdateProgress = CSbieProgressPtr(new CSbieProgress());
	theGUI->AddAsyncOp(m_pUpdateProgress);
	m_pUpdateProgress->ShowMessage(tr("Downloading installer..."));

	return true;
}

void COnlineUpdater::OnInstallerDownload()
{
	if (m_pUpdateProgress.isNull())
		return;

	m_pUpdateProgress->Progress(-1);

	QNetworkReply* pReply = qobject_cast<QNetworkReply*>(sender());
	bool bAndRun = pReply->property("run").toBool();
	QString VersionStr = pReply->property("version").toString();
	QByteArray Signature = pReply->property("signature").toByteArray();
	quint64 Size = pReply->bytesAvailable();
	QString Name = pReply->request().url().fileName();
	if (Name.isEmpty() || Name.right(4).compare(".exe", Qt::CaseInsensitive) != 0)
		Name = "Sandboxie-Plus-Install.exe";

	QString FilePath = GetUpdateDir() + "/" + Name;

	QFile File(FilePath);
	if (File.open(QFile::WriteOnly)) {
		while (pReply->bytesAvailable() > 0)
			File.write(pReply->read(4096));
		File.close();
	}

	QFile SigFile(FilePath + ".sig");
	if (SigFile.open(QFile::WriteOnly)) {
		SigFile.write(QByteArray::fromBase64(Signature));
		SigFile.close();
	}

	pReply->deleteLater();

	m_pUpdateProgress->Finish(SB_OK);
	m_pUpdateProgress.clear();

	if (File.size() != Size) {
		QMessageBox::critical(theGUI, "Sandboxie-Plus", tr("Failed to download installer from: %1").arg(pReply->request().url().toString()));
		return;
	}

	theConf->SetValue("Updater/InstallerVersion", VersionStr);
	theConf->SetValue("Updater/InstallerPath", FilePath);

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

	if (RunInstaller2(FilePath, bSilent)) {
		if (bSilent)
			theConf->DelValue("Updater/InstallerVersion");
		QApplication::quit();
		return true;
	}
	return false;
}

bool COnlineUpdater::RunInstaller2(const QString& FilePath, bool bSilent)
{
	if (bSilent) 
	{
		QStringList Params;
		Params.append("run_setup");
		Params.append(QString(FilePath).replace("/", "\\"));
#ifndef _DEBUG
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
#ifndef _DEBUG
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
			QIcon ico(QLatin1String(":/SandMan.png"));
			mb.setIconPixmap(ico.pixmap(64, 64));
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

bool COnlineUpdater::IsVersionNewer(const QString& VersionStr)
{
	if (VersionStr.isEmpty())
		return false;

	//quint8 myVersion[4] = { VERSION_UPD, VERSION_REV, VERSION_MIN, VERSION_MJR }; // ntohl
	quint8 myVersion[4] = { 0, VERSION_REV, VERSION_MIN, VERSION_MJR }; // ntohl
	quint32 MyVersion = *(quint32*)&myVersion;

	quint32 Version = 0;
	QStringList Nums = VersionStr.split(".");
	for (int i = 0, Bits = 24; i < Nums.count() && Bits >= 0; i++, Bits -= 8)
		Version |= (Nums[i].toInt() & 0xFF) << Bits;

	return (Version > MyVersion);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// cert stuf

void COnlineUpdater::UpdateCert()
{
	QString UpdateKey; // for now only patreons can update the cert automatically
	TArguments args = GetArguments(g_Certificate, L'\n', L':');
	if(args.value("TYPE").indexOf("PATREON") == 0)
		UpdateKey = args.value("UPDATEKEY");
	if (UpdateKey.isEmpty()) {
		theGUI->OpenUrl("https://sandboxie-plus.com/go.php?to=sbie-get-cert");
		return;
	}

	if (!m_pUpdateProgress.isNull())
		return;

	m_pUpdateProgress = CSbieProgressPtr(new CSbieProgress());
	theGUI->AddAsyncOp(m_pUpdateProgress);
	m_pUpdateProgress->ShowMessage(tr("Checking for certificate..."));

	if (m_RequestManager == NULL) 
		m_RequestManager = new CNetworkAccessManager(30 * 1000, this);


	QUrlQuery Query;
	Query.addQueryItem("UpdateKey", UpdateKey);

	QUrl Url("https://sandboxie-plus.com/get_cert.php");
	Url.setQuery(Query);

	QNetworkRequest Request = QNetworkRequest(Url);
	//Request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
	Request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
	//Request.setRawHeader("Accept-Encoding", "gzip");
	QNetworkReply* pReply = m_RequestManager->get(Request);
	connect(pReply, SIGNAL(finished()), this, SLOT(OnCertCheck()));
}

void COnlineUpdater::OnCertCheck()
{
	if (m_pUpdateProgress.isNull())
		return;

	QNetworkReply* pReply = qobject_cast<QNetworkReply*>(sender());
	QByteArray Reply = pReply->readAll();
	int Code = pReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	pReply->deleteLater();

	m_pUpdateProgress->Finish(SB_OK);
	m_pUpdateProgress.clear();

	if (Code > 299 || Code < 200) {
		QMessageBox::critical(theGUI, "Sandboxie-Plus", tr("No certificate found on server!"));
		return;
	}

	if (Reply.replace("\r\n","\n").compare(g_Certificate.replace("\r\n","\n"), Qt::CaseInsensitive) == 0){
		QMessageBox::information(theGUI, "Sandboxie-Plus", tr("There is no updated certificate available."));
		return;
	}

	QString CertPath = theAPI->GetSbiePath() + "\\Certificate.dat";
	QString TempPath = QDir::tempPath() + "/Sbie+Certificate.dat";
	QFile CertFile(TempPath);
	if (CertFile.open(QFile::WriteOnly)) {
		CertFile.write(Reply);
		CertFile.close();
	}

	WindowsMoveFile(TempPath.replace("/", "\\"), CertPath.replace("/", "\\"));

	if (!theAPI->ReloadCert().IsError()) {
		CSettingsWindow::LoadCertificate();
		theGUI->UpdateCertState();
	}
	else { // this should not happen
		g_Certificate.clear();
		g_CertInfo.State = 0;
	}
}
