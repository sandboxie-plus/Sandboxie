
void CSandMan::CheckForUpdates(bool bManual)
{
	if (!m_pUpdateProgress.isNull())
		return;

	m_pUpdateProgress = CSbieProgressPtr(new CSbieProgress());
	AddAsyncOp(m_pUpdateProgress);
	m_pUpdateProgress->ShowMessage(tr("Checking for updates..."));

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
	Query.addQueryItem("language", QString::number(m_LanguageId));

	QString UpdateKey = GetArguments(g_Certificate, L'\n', L':').value("UPDATEKEY");
	if (UpdateKey.isEmpty())
		UpdateKey = theAPI->GetGlobalSettings()->GetText("UpdateKey"); // theConf->GetString("Options/UpdateKey");
	if (!UpdateKey.isEmpty())
		Query.addQueryItem("update_key", UpdateKey);
	Query.addQueryItem("auto", bManual ? "0" : "1");

	QUrl Url("https://sandboxie-plus.com/update.php");
	Url.setQuery(Query);

	QNetworkRequest Request = QNetworkRequest(Url);
	Request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
	//Request.setRawHeader("Accept-Encoding", "gzip");
	QNetworkReply* pReply = m_RequestManager->get(Request);
	pReply->setProperty("manual", bManual);
	connect(pReply, SIGNAL(finished()), this, SLOT(OnUpdateCheck()));
}

void CSandMan::OnUpdateCheck()
{
	if (m_pUpdateProgress.isNull())
		return;

	QNetworkReply* pReply = qobject_cast<QNetworkReply*>(sender());
	bool bManual = pReply->property("manual").toBool();
	QByteArray Reply = pReply->readAll();
	pReply->deleteLater();

	m_pUpdateProgress->Finish(SB_OK);
	m_pUpdateProgress.clear();

	QVariantMap Data = QJsonDocument::fromJson(Reply).toVariant().toMap();
	if (Data.isEmpty() || Data["error"].toBool())
	{
		QString Error = Data.isEmpty() ? tr("server not reachable") : Data["errorMsg"].toString();
		OnLogMessage(tr("Failed to check for updates, error: %1").arg(Error), !bManual);
		if (bManual)
			QMessageBox::critical(this, "Sandboxie-Plus", tr("Failed to check for updates, error: %1").arg(Error));
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

			CCheckableMessageBox mb(this);
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

	QString VersionStr = Data["version"].toString();
	if (!VersionStr.isEmpty()) //&& VersionStr != GetVersion())
	{
		UCHAR myVersion[4] = { VERSION_UPD, VERSION_REV, VERSION_MIN, VERSION_MJR }; // ntohl
		ULONG MyVersion = *(ULONG*)&myVersion;

		ULONG Version = 0;
		QStringList Nums = VersionStr.split(".");
		for (int i = 0, Bits = 24; i < Nums.count() && Bits >= 0; i++, Bits -= 8)
			Version |= (Nums[i].toInt() & 0xFF) << Bits;

		if (Version > MyVersion)
		if (bManual || !IgnoredUpdates.contains(VersionStr)) // when checked manually always show result
		{
			bNothing = false;
			//QDateTime Updated = QDateTime::fromTime_t(Data["updated"].toULongLong());

			QString DownloadUrl = Data["downloadUrl"].toString();
			//	'sha256'
			//	'signature'

			if (!DownloadUrl.isEmpty() && theConf->GetInt("Options/DownloadUpdates", 0) == 1)
				DownloadUpdates(DownloadUrl, bManual);
			else
			{
				QString UpdateMsg = Data["updateMsg"].toString();
				QString UpdateUrl = Data["updateUrl"].toString();

				QString FullMessage = UpdateMsg.isEmpty() ? tr("<p>There is a new version of Sandboxie-Plus available.<br /><font color='red'>New version:</font> <b>%1</b></p>").arg(VersionStr) : UpdateMsg;
				if (!DownloadUrl.isEmpty())
					FullMessage += tr("<p>Do you want to download the latest version?</p>");
				else if (!UpdateUrl.isEmpty())
					FullMessage += tr("<p>Do you want to go to the <a href=\"%1\">download page</a>?</p>").arg(UpdateUrl);

				CCheckableMessageBox mb(this);
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

				if (mb.isChecked())
					theConf->SetValue("Options/IgnoredUpdates", IgnoredUpdates << VersionStr);

				if (mb.clickedStandardButton() == QDialogButtonBox::Yes)
				{
					if (!DownloadUrl.isEmpty())
						DownloadUpdates(DownloadUrl, bManual);
					else
						QDesktopServices::openUrl(UpdateUrl);
				}
			}
		}
	}

	if (bNothing) 
	{
		theConf->SetValue("Options/NextCheckForUpdates", QDateTime::currentDateTime().addDays(7).toTime_t());

		if (bManual) {
			QMessageBox::information(this, "Sandboxie-Plus", tr("No new updates found, your Sandboxie-Plus is up-to-date.\n"
				"\nNote: The update check is often behind the latest GitHub release to ensure that only tested updates are offered."));
		}
	}
}

void CSandMan::DownloadUpdates(const QString& DownloadUrl, bool bManual)
{
	QNetworkRequest Request = QNetworkRequest(DownloadUrl);
	Request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
	//Request.setRawHeader("Accept-Encoding", "gzip");
	QNetworkReply* pReply = m_RequestManager->get(Request);
	pReply->setProperty("manual", bManual);
	connect(pReply, SIGNAL(finished()), this, SLOT(OnUpdateDownload()));
	connect(pReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(OnUpdateProgress(qint64, qint64)));

	m_pUpdateProgress = CSbieProgressPtr(new CSbieProgress());
	AddAsyncOp(m_pUpdateProgress);
	m_pUpdateProgress->ShowMessage(tr("Downloading new version..."));
}

void CSandMan::OnUpdateProgress(qint64 bytes, qint64 bytesTotal)
{
	if (bytesTotal != 0 && !m_pUpdateProgress.isNull())
		m_pUpdateProgress->Progress(100 * bytes / bytesTotal);
}

void CSandMan::OnUpdateDownload()
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
		QMessageBox::critical(this, "Sandboxie-Plus", tr("Failed to download update from: %1").arg(pReply->request().url().toString()));
		return;
	}

	theConf->SetValue("Options/PendingUpdatePackage", FilePath);
	UpdateLabel();

	if (bManual)
		InstallUpdate();
}

void CSandMan::InstallUpdate()
{
	QString FilePath = theConf->GetString("Options/PendingUpdatePackage");
	if (FilePath.isEmpty())
		return;

	QString Message = tr("<p>A Sandboxie-Plus update has been downloaded to the following location:</p><p><a href=\"%2\">%1</a></p><p>Do you want to begin the installation? If any programs are running sandboxed, they will be terminated.</p>")
		.arg(FilePath).arg("File:///" + Split2(FilePath, "/", true).first);
	int Ret = QMessageBox("Sandboxie-Plus", Message, QMessageBox::Information, QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape, QMessageBox::Cancel, this).exec();
	if (Ret == QMessageBox::Cancel) {
		theConf->DelValue("Options/PendingUpdatePackage");
		UpdateLabel();
	}
	if (Ret != QMessageBox::Yes)
		return;
	
	theAPI->TerminateAll();

	wstring wFile = FilePath.toStdWString();

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

void CSandMan::OnHelp()
{
	if (sender() == m_pSupport)
		QDesktopServices::openUrl(QUrl("https://sandboxie-plus.com/go.php?to=donate"));
	else if (sender() == m_pForum)
		QDesktopServices::openUrl(QUrl("https://sandboxie-plus.com/go.php?to=sbie-forum"));
	else if (sender() == m_pManual)
		QDesktopServices::openUrl(QUrl("https://sandboxie-plus.com/go.php?to=sbie-docs"));
	else
		QDesktopServices::openUrl(QUrl("https://sandboxie-plus.com/go.php?to=patreon"));
}

void CSandMan::OnAbout()
{
	if (sender() == m_pAbout)
	{
		QString AboutCaption = tr(
			"<h3>About Sandboxie-Plus</h3>"
			"<p>Version %1</p>"
			"<p>Copyright (c) 2020-2022 by DavidXanatos</p>"
		).arg(GetVersion());

		QString CertInfo;
		if (!g_Certificate.isEmpty()) {
			CertInfo = tr("This copy of Sandboxie+ is certified for: %1").arg(GetArguments(g_Certificate, L'\n', L':').value("NAME"));
		} else {
			CertInfo = tr("Sandboxie+ is free for personal and non-commercial use.");
		}

		QString AboutText = tr(
			"Sandboxie-Plus is an open source continuation of Sandboxie.<br />"
			"Visit <a href=\"https://sandboxie-plus.com\">sandboxie-plus.com</a> for more information.<br />"
			"<br />"
			"%3<br />"
			"<br />"
			"Driver version: %1<br />"
			"Features: %2<br />"
			"<br />"
			"Icons from <a href=\"https://icons8.com\">icons8.com</a>"
		).arg(theAPI->GetVersion()).arg(theAPI->GetFeatureStr()).arg(CertInfo);

		QMessageBox *msgBox = new QMessageBox(this);
		msgBox->setAttribute(Qt::WA_DeleteOnClose);
		msgBox->setWindowTitle(tr("About Sandboxie-Plus"));
		msgBox->setText(AboutCaption);
		msgBox->setInformativeText(AboutText);

		QIcon ico(QLatin1String(":/SandMan.png"));
		msgBox->setIconPixmap(ico.pixmap(128, 128));

		SafeExec(msgBox);
	}
	else if (sender() == m_pAboutQt)
		QMessageBox::aboutQt(this);
}

void CSandMan::UpdateCert()
{
	QString UpdateKey; // for now only patreons can update the cert automatically
	if(GetArguments(g_Certificate, L'\n', L':').value("type").indexOf("PATREON") == 0)
		UpdateKey = GetArguments(g_Certificate, L'\n', L':').value("UPDATEKEY");
	if (UpdateKey.isEmpty()) {
		OpenUrl("https://sandboxie-plus.com/go.php?to=sbie-get-cert");
		return;
	}

	if (!m_pUpdateProgress.isNull())
		return;

	m_pUpdateProgress = CSbieProgressPtr(new CSbieProgress());
	AddAsyncOp(m_pUpdateProgress);
	m_pUpdateProgress->ShowMessage(tr("Checking for certificate..."));

	if (m_RequestManager == NULL) 
		m_RequestManager = new CNetworkAccessManager(30 * 1000, this);


	QUrlQuery Query;
	Query.addQueryItem("UpdateKey", UpdateKey);

	QUrl Url("https://sandboxie-plus.com/get_cert.php");
	Url.setQuery(Query);

	QNetworkRequest Request = QNetworkRequest(Url);
	Request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
	//Request.setRawHeader("Accept-Encoding", "gzip");
	QNetworkReply* pReply = m_RequestManager->get(Request);
	connect(pReply, SIGNAL(finished()), this, SLOT(OnCertCheck()));
}

void CSandMan::OnCertCheck()
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
		QMessageBox::critical(this, "Sandboxie-Plus", tr("No certificate found on server!"));
		return;
	}

	if (Reply.replace("\r\n","\n").compare(g_Certificate.replace("\r\n","\n"), Qt::CaseInsensitive) == 0){
		QMessageBox::information(this, "Sandboxie-Plus", tr("There is no updated certificate available."));
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
		UpdateCertState();
	}
	else { // this should not happen
		g_Certificate.clear();
		g_CertInfo.State = 0;
	}
}