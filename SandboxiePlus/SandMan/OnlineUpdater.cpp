#include "stdafx.h"
#include "OnlineUpdater.h"
#include "../MiscHelpers/Common/Common.h"
#include "SandMan.h"
#include "Windows/SettingsWindow.h"
#include <QUrlQuery>
#include <QJsonDocument>
#include "../MiscHelpers/Common/CheckableMessageBox.h"
#include <QMessageBox>

#include <windows.h>

COnlineUpdater* g_pUpdater = NULL;

COnlineUpdater::COnlineUpdater(QObject *parent) : QObject(parent) 
{
	m_RequestManager = NULL;
}

COnlineUpdater* COnlineUpdater::Instance()
{
	if (!g_pUpdater)
		g_pUpdater = new COnlineUpdater(theGUI);
	return g_pUpdater;
}

void COnlineUpdater::Process() 
{
	if (g_pUpdater && !g_pUpdater->m_pUpdateProgress.isNull() && g_pUpdater->m_RequestManager != NULL) {
		if (g_pUpdater->m_pUpdateProgress->IsCanceled()) {
			g_pUpdater->m_pUpdateProgress->Finish(SB_OK);
			g_pUpdater->m_pUpdateProgress.clear();

			g_pUpdater->m_RequestManager->AbortAll();
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
	Query.addQueryItem("software", "sandboxie-plus");
	//QString Branche = theConf->GetString("Options/ReleaseBranche");
	//if (!Branche.isEmpty())
	//	Query.addQueryItem("branche", Branche);
	//Query.addQueryItem("version", GetVersion());
	Query.addQueryItem("version", QString::number(VERSION_MJR) + "." + QString::number(VERSION_MIN) + "." + QString::number(VERSION_REV) + "." + QString::number(VERSION_UPD));
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
		int UpdateChannel = theConf->GetInt("Options/UpdateChannel", 0);
		Query.addQueryItem("channel", QString::number(UpdateChannel));
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
	//pReply->setProperty("manual", bManual);
	connect(pReply, SIGNAL(finished()), this, SLOT(OnUpdateCheck()));

	m_JobQueue.insert(pReply, pJob);
}

void COnlineUpdater::CheckForUpdates(bool bManual, bool bDownload)
{
	if (!m_pUpdateProgress.isNull())
		return;

	m_pUpdateProgress = CSbieProgressPtr(new CSbieProgress());
	theGUI->AddAsyncOp(m_pUpdateProgress);
	m_pUpdateProgress->ShowMessage(tr("Checking for updates..."));

	QVariantMap Params;
	Params["manual"] = bManual;
	Params["download"] = bDownload;
	GetUpdates(this, SLOT(OnUpdateData(const QVariantMap&, const QVariantMap&)), Params);
}

void COnlineUpdater::OnUpdateCheck()
{
	QNetworkReply* pReply = qobject_cast<QNetworkReply*>(sender());
	//bool bManual = pReply->property("manual").toBool();
	QByteArray Reply = pReply->readAll();
	pReply->deleteLater();

	CGetUpdatesJob* pJob = m_JobQueue.take(pReply);
	if (!pJob)
		return;

	QVariantMap Data = QJsonDocument::fromJson(Reply).toVariant().toMap();

	emit pJob->UpdateData(Data, pJob->m_Params);
	pJob->deleteLater();
}

void COnlineUpdater::OnUpdateData(const QVariantMap& Data, const QVariantMap& Params)
{
	bool bManual = Params["manual"].toBool();
	bool bDownload = Params["download"].toBool();

	if (!m_pUpdateProgress.isNull()) {
		m_pUpdateProgress->Finish(SB_OK);
		m_pUpdateProgress.clear();
	}

	if (Data.isEmpty() || Data["error"].toBool())
	{
		QString Error = Data.isEmpty() ? tr("server not reachable") : Data["errorMsg"].toString();
		theGUI->OnLogMessage(tr("Failed to check for updates, error: %1").arg(Error), !bManual);
		if (bManual)
			QMessageBox::critical(theGUI, "Sandboxie-Plus", tr("Failed to check for updates, error: %1").arg(Error));
		return;
	}

	bool bNothing = true;

	QStringList IgnoredUpdates = theConf->GetStringList("Options/IgnoredUpdates");

	QString UserMsg = Data["userMsg"].toString();
	if (!UserMsg.isEmpty())
	{
		QString MsgHash = QCryptographicHash::hash(Data["userMsg"].toByteArray(), QCryptographicHash::Md5).toHex().left(8);
		if (!IgnoredUpdates.contains(MsgHash))
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
				theConf->SetValue("Options/IgnoredUpdates", IgnoredUpdates << MsgHash);

			if (mb.clickedStandardButton() == QDialogButtonBox::Yes)
			{
				QDesktopServices::openUrl(InfoUrl);
			}

			bNothing = false;
		}
	}

	//int TemplateVersion = CSbieIni("DefaultTemplates", theAPI, this).GetNum("TemplateVersion", 0, false, true);

	QString VersionStr = Data["version"].toString();
	if (!VersionStr.isEmpty()) //&& VersionStr != GetVersion())
	{
		if (IsVersionNewer(VersionStr))
		if (bManual || !IgnoredUpdates.contains(VersionStr)) // when checked manually always show result
		{
			bNothing = false;
			//QDateTime Updated = QDateTime::fromSecsSinceEpoch(Data["updated"].toULongLong());

			QString DownloadUrl = Data["downloadUrl"].toString();
			//	'sha256'
			//	'signature'

			if (!DownloadUrl.isEmpty() && (bDownload || theConf->GetInt("Options/DownloadUpdates", 0) == 1)) 
			{
				theConf->SetValue("Options/PendingUpdateVersion", VersionStr);
				DownloadUpdates(DownloadUrl, bManual);
			}
			else
			{
				QString UpdateMsg = Data["updateMsg"].toString();
				QString UpdateUrl = Data["updateUrl"].toString();

				QString FullMessage = UpdateMsg.isEmpty() ? tr("<p>There is a new version of Sandboxie-Plus available.<br /><font color='red'><b>New version:</b></font> <b>%1</b></p>").arg(VersionStr) : UpdateMsg;
				if (!DownloadUrl.isEmpty())
					FullMessage += tr("<p>Do you want to download the latest version?</p>");
				else if (!UpdateUrl.isEmpty())
					FullMessage += tr("<p>Do you want to go to the <a href=\"%1\">download page</a>?</p>").arg(UpdateUrl);

				CCheckableMessageBox mb(theGUI);
				mb.setWindowTitle("Sandboxie-Plus");
				QIcon ico(QLatin1String(":/SandMan.png"));
				mb.setIconPixmap(ico.pixmap(64, 64));
				//mb.setTextFormat(Qt::RichText);
				mb.setText(FullMessage);
				mb.setCheckBoxText(tr("Don't show this message anymore."));
				mb.setCheckBoxVisible(!bManual);

				if (!UpdateUrl.isEmpty() || !DownloadUrl.isEmpty()) {
					mb.setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No);
					mb.setDefaultButton(QDialogButtonBox::Yes);
				}
				else
					mb.setStandardButtons(QDialogButtonBox::Ok);

				mb.exec();

				if (mb.isChecked()) {
					theConf->DelValue("Options/PendingUpdateVersion");
					theGUI->UpdateLabel();
					theConf->SetValue("Options/IgnoredUpdates", IgnoredUpdates << VersionStr);
				}

				if (mb.clickedStandardButton() == QDialogButtonBox::Yes)
				{
					if (!DownloadUrl.isEmpty()) 
					{
						theConf->SetValue("Options/PendingUpdateVersion", VersionStr);
						DownloadUpdates(DownloadUrl, bManual);
					} 
					else
						QDesktopServices::openUrl(UpdateUrl);
				}
				else if (!mb.isChecked())
				{
					theConf->SetValue("Options/PendingUpdateVersion", VersionStr);
					theGUI->UpdateLabel();
				}
			}
		}
	}

	if (bNothing) 
	{
		theConf->DelValue("Options/PendingUpdateVersion");
		theGUI->UpdateLabel();
		theConf->SetValue("Options/NextCheckForUpdates", QDateTime::currentDateTime().addDays(7).toSecsSinceEpoch());

		if (bManual) {
			QMessageBox::information(theGUI, "Sandboxie-Plus", tr("No new updates found, your Sandboxie-Plus is up-to-date.\n"
				"\nNote: The update check is often behind the latest GitHub release to ensure that only tested updates are offered."));
		}
	}
}

void COnlineUpdater::DownloadUpdates(const QString& DownloadUrl, bool bManual)
{
	QNetworkRequest Request = QNetworkRequest(DownloadUrl);
	//Request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
	Request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
	//Request.setRawHeader("Accept-Encoding", "gzip");
	QNetworkReply* pReply = m_RequestManager->get(Request);
	pReply->setProperty("manual", bManual);
	connect(pReply, SIGNAL(finished()), this, SLOT(OnUpdateDownload()));
	connect(pReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(OnUpdateProgress(qint64, qint64)));

	m_pUpdateProgress = CSbieProgressPtr(new CSbieProgress());
	theGUI->AddAsyncOp(m_pUpdateProgress);
	m_pUpdateProgress->ShowMessage(tr("Downloading new version..."));
}

void COnlineUpdater::OnUpdateProgress(qint64 bytes, qint64 bytesTotal)
{
	if (bytesTotal != 0 && !m_pUpdateProgress.isNull())
		m_pUpdateProgress->Progress(100 * bytes / bytesTotal);
}

void COnlineUpdater::OnUpdateDownload()
{
	if (m_pUpdateProgress.isNull())
		return;

	QString TempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
	if (TempDir.right(1) != "/")
		TempDir += "/";

	m_pUpdateProgress->Progress(-1);

	QNetworkReply* pReply = qobject_cast<QNetworkReply*>(sender());
	bool bManual = pReply->property("manual").toBool();
	quint64 Size = pReply->bytesAvailable();
	QString Name = pReply->request().url().fileName();
	if (Name.isEmpty() || Name.right(4).compare(".exe", Qt::CaseInsensitive) != 0)
		Name = "Sandboxie-Plus-Install.exe";

	QString FilePath = TempDir + Name;

	QFile File(FilePath);
	if (File.open(QFile::WriteOnly)) {
		while (pReply->bytesAvailable() > 0)
			File.write(pReply->read(4096));
		File.close();
	}

	pReply->deleteLater();

	m_pUpdateProgress->Finish(SB_OK);
	m_pUpdateProgress.clear();

	if (File.size() != Size) {
		QMessageBox::critical(theGUI, "Sandboxie-Plus", tr("Failed to download update from: %1").arg(pReply->request().url().toString()));
		return;
	}

	theConf->SetValue("Options/PendingUpdatePackage", FilePath);
	theGUI->UpdateLabel();

	if (bManual)
		InstallUpdate();
}

bool COnlineUpdater::IsVersionNewer(const QString& VersionStr)
{
#ifdef _DEBUG
	if (QApplication::keyboardModifiers() & Qt::ControlModifier)
		return true;
#endif

	quint8 myVersion[4] = { VERSION_UPD, VERSION_REV, VERSION_MIN, VERSION_MJR }; // ntohl
	quint32 MyVersion = *(quint32*)&myVersion;

	quint32 Version = 0;
	QStringList Nums = VersionStr.split(".");
	for (int i = 0, Bits = 24; i < Nums.count() && Bits >= 0; i++, Bits -= 8)
		Version |= (Nums[i].toInt() & 0xFF) << Bits;

	return (Version > MyVersion);
}

void COnlineUpdater::CheckPendingUpdate()
{
	QString VersionStr = theConf->GetString("Options/PendingUpdateVersion");
	bool bIsNewVersion = IsVersionNewer(VersionStr);
	if (!bIsNewVersion)
		theConf->DelValue("Options/PendingUpdateVersion");
	QString FilePath = theConf->GetString("Options/PendingUpdatePackage");
	if (!bIsNewVersion || !QFile::exists(FilePath))
		theConf->DelValue("Options/PendingUpdatePackage");
}

void COnlineUpdater::DownloadUpdate()
{
	CheckForUpdates(true, true);
}

void COnlineUpdater::InstallUpdate()
{
	QString FilePath = theConf->GetString("Options/PendingUpdatePackage");
	if (FilePath.isEmpty())
		return;

	QString Message = tr("<p>A Sandboxie-Plus update has been downloaded to the following location:</p><p><a href=\"%2\">%1</a></p><p>Do you want to begin the installation? If any programs are running sandboxed, they will be terminated.</p>")
		.arg(FilePath).arg("File:///" + Split2(FilePath, "/", true).first);
	int Ret = QMessageBox("Sandboxie-Plus", Message, QMessageBox::Information, QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape, QMessageBox::Cancel, theGUI).exec();
	if (Ret == QMessageBox::Cancel) {
		theConf->DelValue("Options/PendingUpdatePackage");
		theGUI->UpdateLabel();
	}
	if (Ret != QMessageBox::Yes)
		return;
	
	theAPI->TerminateAll();

	std::wstring wFile = FilePath.toStdWString();

	SHELLEXECUTEINFO si = { 0 };
	si.cbSize = sizeof(SHELLEXECUTEINFO);
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.hwnd = NULL;
	si.lpVerb = L"runas";
	si.lpFile = wFile.c_str();
	si.lpParameters = L"/SILENT";
	si.lpDirectory = NULL;
	si.nShow = SW_SHOW;
	si.hInstApp = NULL;

	if (ShellExecuteEx(&si)) {
		theConf->DelValue("Options/PendingUpdatePackage");
		QApplication::quit();
	}
}

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