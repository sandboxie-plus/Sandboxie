
void CSandMan::OnFileToRecover(const QString& BoxName, const QString& FilePath, const QString& BoxPath, quint32 ProcessId)
{
	CSandBoxPtr pBox = theAPI->GetBoxByName(BoxName);
	if (pBox.isNull() || pBox.objectCast<CSandBoxPlus>()->IsRecoverySuspended() || IsDisableRecovery())
		return;

	if (theConf->GetBool("Options/InstantRecovery", true))
	{
		auto pBoxEx = pBox.objectCast<CSandBoxPlus>();

		if (!pBoxEx->m_pRecoveryWnd) {
			pBoxEx->m_pRecoveryWnd = new CRecoveryWindow(pBox, true, this);
			connect(this, SIGNAL(Closed()), pBoxEx->m_pRecoveryWnd, SLOT(close()));
			connect(pBoxEx->m_pRecoveryWnd, &CRecoveryWindow::Closed, [pBoxEx]() {
				pBoxEx->m_pRecoveryWnd = NULL;
				});

			pBoxEx->m_pRecoveryWnd->AddFile(FilePath, BoxPath);

			pBoxEx->m_pRecoveryWnd->setModal(true);
			pBoxEx->m_pRecoveryWnd->show();
		}
		else
		{
			//if (!theGUI->IsAlwaysOnTop()) {
			//	SetWindowPos((HWND)pWnd->winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			//	QTimer::singleShot(100, this, [pWnd]() {
			//		SetWindowPos((HWND)pWnd->winId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			//		});
			//}

			pBoxEx->m_pRecoveryWnd->AddFile(FilePath, BoxPath);
		}

	}
	else
		m_pPopUpWindow->AddFileToRecover(FilePath, BoxPath, pBox, ProcessId);
}

bool CSandMan::OpenRecovery(const CSandBoxPtr& pBox, bool& DeleteSnapshots, bool bCloseEmpty)
{
	auto pBoxEx = pBox.objectCast<CSandBoxPlus>();
	if (!pBoxEx) return false;
	if (pBoxEx->m_pRecoveryWnd != NULL) {
		if (pBoxEx->m_pRecoveryWnd->IsDeleteDialog())
			return false;
		pBoxEx->m_pRecoveryWnd->close();
	}

	CRecoveryWindow* pRecoveryWnd = pBoxEx->m_pRecoveryWnd = new CRecoveryWindow(pBox, false, this);
	connect(this, SIGNAL(Closed()), pBoxEx->m_pRecoveryWnd, SLOT(close()));
	if (pBoxEx->m_pRecoveryWnd->FindFiles() == 0 && bCloseEmpty) {
		delete pBoxEx->m_pRecoveryWnd;
		pBoxEx->m_pRecoveryWnd = NULL;
		return true;
	}
	else {
		connect(pBoxEx->m_pRecoveryWnd, &CRecoveryWindow::Closed, [pBoxEx]() {
			pBoxEx->m_pRecoveryWnd = NULL;
		});
		if (pBoxEx->m_pRecoveryWnd->exec() != 1)
			return false;
	}
	DeleteSnapshots = pRecoveryWnd->IsDeleteSnapshots();
	return true;
}

CRecoveryWindow* CSandMan::ShowRecovery(const CSandBoxPtr& pBox)
{
	auto pBoxEx = pBox.objectCast<CSandBoxPlus>();
	if (!pBoxEx) return NULL;
	if (pBoxEx->m_pRecoveryWnd == NULL) {
		pBoxEx->m_pRecoveryWnd = new CRecoveryWindow(pBox, false);
		connect(this, SIGNAL(Closed()), pBoxEx->m_pRecoveryWnd, SLOT(close()));
		connect(pBoxEx->m_pRecoveryWnd, &CRecoveryWindow::Closed, [pBoxEx]() {
			pBoxEx->m_pRecoveryWnd = NULL;
		});
		pBoxEx->m_pRecoveryWnd->show();
	}
	else { // We don't want to force window in front on instant recovery 
		pBoxEx->m_pRecoveryWnd->setWindowState((pBoxEx->m_pRecoveryWnd->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
		SetForegroundWindow((HWND)pBoxEx->m_pRecoveryWnd->winId());
	}
	pBoxEx->m_pRecoveryWnd->FindFiles();
	return pBoxEx->m_pRecoveryWnd;
}

QStringList CSandMan::GetFileCheckers(const CSandBoxPtr& pBox)
{
	QStringList Checkers;

	if (!theGUI->GetAddonManager()->GetAddon("FileChecker", CAddonManager::eInstalled).isNull())
		Checkers.append(pBox->Expand("powershell -exec bypass -nop -File \"%SbieHome%\\addons\\FileChecker\\CheckFile.ps1\" -bin"));
	
	if (!pBox.isNull()) {
		foreach(const QString & Value, pBox->GetTextList("OnFileRecovery", true, false, true)) {
			Checkers.append(pBox->Expand(Value));
		}
	}

	return Checkers;
}

SB_PROGRESS CSandMan::CheckFiles(const QString& BoxName, const QStringList& Files)
{
	CSbieProgressPtr pProgress = CSbieProgressPtr(new CSbieProgress());
	CSandBoxPtr pBox = theAPI->GetBoxByName(BoxName);
	QtConcurrent::run(CSandMan::CheckFilesAsync, pProgress, BoxName, Files, GetFileCheckers(pBox));
	return SB_PROGRESS(OP_ASYNC, pProgress);
}

void CSandMan::CheckFilesAsync(const CSbieProgressPtr& pProgress, const QString& BoxName, const QStringList& Files, const QStringList& Checkers)
{
	int FailCount = 0;
	for (QStringList::const_iterator I = Files.begin(); I != Files.end(); ++I) 
	{
		if (pProgress->IsCanceled()) break;

		QString BoxPath = *I;
		QString FileName = BoxPath.mid(BoxPath.lastIndexOf("\\") + 1);
		
		pProgress->ShowMessage(tr("Checking file %1").arg(FileName));

		foreach(const QString & Value, Checkers) {
			QString Output;
			int ret = CSbieUtils::ExecCommandEx(Value + " \"" + BoxPath + "\"", &Output, 15000); // 15 sec timeout
			if (ret != 0) {
				FailCount++;
				QMetaObject::invokeMethod(theGUI, "ShowMessage", Qt::BlockingQueuedConnection, // show this message using the GUI thread
					Q_ARG(QString, tr("The file %1 failed a security check!\n\n%2").arg(BoxPath).arg(Output)),
					Q_ARG(int, QMessageBox::Warning)
				);
			}
		}
	}
	if (FailCount == 0) {
		QMetaObject::invokeMethod(theGUI, "ShowMessage", Qt::BlockingQueuedConnection, // show this message using the GUI thread
			Q_ARG(QString, tr("All files passed the checks")),
			Q_ARG(int, QMessageBox::Information)
		);
	}

	pProgress->Finish(SB_OK);
}

SB_PROGRESS CSandMan::RecoverFiles(const QString& BoxName, const QList<QPair<QString, QString>>& FileList, QWidget* pParent, int Action)
{
	CSbieProgressPtr pProgress = CSbieProgressPtr(new CSbieProgress());
	CSandBoxPtr pBox = theAPI->GetBoxByName(BoxName);
	QtConcurrent::run(CSandMan::RecoverFilesAsync, qMakePair(pProgress, pParent), BoxName, FileList, GetFileCheckers(pBox), Action);
	return SB_PROGRESS(OP_ASYNC, pProgress);
}

void CSandMan::RecoverFilesAsync(QPair<const CSbieProgressPtr&,QWidget*> pParam, const QString& BoxName, const QList<QPair<QString, QString>>& FileList, const QStringList& Checkers, int Action)
{
	const CSbieProgressPtr& pProgress = pParam.first;
	QWidget* pParent = pParam.second;

	SB_STATUS Status = SB_OK;

	int OverwriteOnExist = -1;
	int RecoverCheckFailed = -1;

	QStringList Unrecovered;
	for (QList<QPair<QString, QString>>::const_iterator I = FileList.begin(); I != FileList.end(); ++I)
	{
		if (pProgress->IsCanceled()) break;

		QString BoxPath = I->first;
		QString RecoveryPath = I->second;
		QString FileName = BoxPath.mid(BoxPath.lastIndexOf("\\") + 1);
		QString RecoveryFolder = RecoveryPath.left(RecoveryPath.lastIndexOf("\\") + 1);

		if (!Checkers.isEmpty()) {

			pProgress->ShowMessage(tr("Checking file %1").arg(FileName));

			//bool bNoGui = true;
			//if (GetKeyState(VK_CONTROL) & 0x8000)
			//	bNoGui = false;

			int ret = 0;
			foreach(const QString & Value, Checkers) {
				QString Output;
				ret = CSbieUtils::ExecCommandEx(Value + " \"" + BoxPath + "\"", &Output, 15000); // 15 sec timeout
				if (ret != 0) {

					int Recover = RecoverCheckFailed;
					if (Recover == -1)
					{
						bool forAll = false;
						int retVal = 0;
						QMetaObject::invokeMethod(theGUI, "ShowQuestion", Qt::BlockingQueuedConnection, // show this question using the GUI thread
							Q_RETURN_ARG(int, retVal),
							Q_ARG(QString, tr("The file %1 failed a security check, do you want to recover it anyway?\n\n%2").arg(BoxPath).arg(Output)),
							Q_ARG(QString, tr("Do this for all files!")),
							Q_ARG(bool*, &forAll),
							Q_ARG(int, QDialogButtonBox::Yes | QDialogButtonBox::No),
							Q_ARG(int, QDialogButtonBox::No),
							Q_ARG(int, QMessageBox::Warning),
							Q_ARG(QWidget*, pParent)
						);

						Recover = retVal == QDialogButtonBox::Yes ? 1 : 0;
						if (forAll)
							RecoverCheckFailed = Recover;
					}

					if (Recover == 1)
						ret = 0;
					else
						break;
				}
			}
			if (ret != 0)
				continue; // Do not recover this file
		}

		pProgress->ShowMessage(tr("Recovering file %1 to %2").arg(FileName).arg(RecoveryFolder));

		QDir().mkpath(RecoveryFolder);
		if (QFile::exists(RecoveryPath)) 
		{
			int Overwrite = OverwriteOnExist;
			if (Overwrite == -1)
			{
				bool forAll = false;
				int retVal = 0;
				QMetaObject::invokeMethod(theGUI, "ShowQuestion", Qt::BlockingQueuedConnection, // show this question using the GUI thread
					Q_RETURN_ARG(int, retVal),
					Q_ARG(QString, tr("The file %1 already exists, do you want to overwrite it?").arg(RecoveryPath)),
					Q_ARG(QString, tr("Do this for all files!")),
					Q_ARG(bool*, &forAll),
					Q_ARG(int, QDialogButtonBox::Yes | QDialogButtonBox::No),
					Q_ARG(int, QDialogButtonBox::No),
					Q_ARG(int, QMessageBox::Question),
					Q_ARG(QWidget*, pParent)
				);

				Overwrite = retVal == QDialogButtonBox::Yes ? 1 : 0;
				if (forAll)
					OverwriteOnExist = Overwrite;
			}
			if (Overwrite == 1)
				QFile::remove(RecoveryPath);
		}

		if (!QFile::rename(BoxPath, RecoveryPath))
			Unrecovered.append(BoxPath);
		else {
			QMetaObject::invokeMethod(theGUI, "OnFileRecovered", Qt::BlockingQueuedConnection, // show this question using the GUI thread
				Q_ARG(QString, BoxName),
				Q_ARG(QString, RecoveryPath),
				Q_ARG(QString, BoxPath)
			);
		}
	}

	if (!Unrecovered.isEmpty())
		Status = SB_ERR(SB_Message, QVariantList () << (tr("Failed to recover some files: \n") + Unrecovered.join("\n")));
	else if(FileList.count() == 1 && Action != 0)
	{
		std::wstring path = FileList.first().second.toStdWString();
		switch (Action)
		{
		case 1: // open
			ShellExecuteW(NULL, NULL, path.c_str(), NULL, NULL, SW_SHOWNORMAL);
			break;
		case 2: // explore
			ShellExecuteW(NULL, NULL, L"explorer.exe", (L"/select,\"" + path + L"\"").c_str(), NULL, SW_SHOWNORMAL);
			break;
		}
	}


	pProgress->Finish(Status);
}

void CSandMan::AddFileRecovered(const QString& BoxName, const QString& FilePath)
{
	CPanelWidgetEx* pRecoveryLog = m_pRecoveryLog;
	if (pRecoveryLog == NULL) {
		pRecoveryLog = m_pRecoveryLogWnd->m_pRecoveryLog;
		if (!pRecoveryLog) return;
	}

	QTreeWidgetItem* pItem = new QTreeWidgetItem(); // Time|Box|FilePath
	pItem->setText(0, QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
	pItem->setText(1, BoxName);
	pItem->setText(2, FilePath);
	pRecoveryLog->GetTree()->addTopLevelItem(pItem);

	pRecoveryLog->GetView()->verticalScrollBar()->setValue(pRecoveryLog->GetView()->verticalScrollBar()->maximum());
}

void CSandMan::OnFileRecovered(const QString& BoxName, const QString& FilePath, const QString& BoxPath)
{
	AddFileRecovered(BoxName, FilePath);

	CSandBoxPtr pBox = theAPI->GetBoxByName(BoxName);
	if (pBox)
		pBox.objectCast<CSandBoxPlus>()->UpdateSize();
}


////////////////////////////////////////////////////////////////////////////////////////
// CRecoveryLogWnd

CRecoveryLogWnd::CRecoveryLogWnd(QWidget *parent)
	: QDialog(parent)
{
	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::CustomizeWindowHint;
	//flags &= ~Qt::WindowContextHelpButtonHint;
	//flags &= ~Qt::WindowSystemMenuHint;
	//flags &= ~Qt::WindowMinMaxButtonsHint;
	//flags |= Qt::WindowMinimizeButtonHint;
	//flags &= ~Qt::WindowCloseButtonHint;
	flags &= ~Qt::WindowContextHelpButtonHint;
	//flags &= ~Qt::WindowSystemMenuHint;
	setWindowFlags(flags);

	this->setWindowTitle(tr("Sandboxie-Plus - Recovery Log"));

	QGridLayout* pLayout = new QGridLayout();
	//pLayout->setContentsMargins(3,3,3,3);
	
	m_pRecoveryLog = new CPanelWidgetEx();
	m_pRecoveryLog->GetTree()->setItemDelegate(new CTreeItemDelegate());

	m_pRecoveryLog->GetTree()->setAlternatingRowColors(theConf->GetBool("Options/AltRowColors", false));

	//m_pRecoveryLog->GetView()->setItemDelegate(theGUI->GetItemDelegate());
	((QTreeWidgetEx*)m_pRecoveryLog->GetView())->setHeaderLabels(tr("Time|Box Name|File Path").split("|"));

	QAction* pAction = new QAction(tr("Cleanup Recovery Log"));
	connect(pAction, SIGNAL(triggered()), m_pRecoveryLog->GetTree(), SLOT(clear()));
	m_pRecoveryLog->GetMenu()->insertAction(m_pRecoveryLog->GetMenu()->actions()[0], pAction);
	m_pRecoveryLog->GetMenu()->insertSeparator(m_pRecoveryLog->GetMenu()->actions()[0]);

	m_pRecoveryLog->GetView()->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_pRecoveryLog->GetView()->setSortingEnabled(false);

	connect(m_pRecoveryLog->GetTree(), SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnDblClick(QTreeWidgetItem*)));

	pLayout->addWidget(new QLabel(tr("The following files were recently recovered and moved out of a sandbox.")), 0, 0);
	pLayout->addWidget(m_pRecoveryLog, 1, 0);
	this->setLayout(pLayout);

	restoreGeometry(theConf->GetBlob("RecoveryLogWindow/Window_Geometry"));
}

CRecoveryLogWnd::~CRecoveryLogWnd()
{
	theConf->SetBlob("RecoveryLogWindow/Window_Geometry", saveGeometry());
}

void CRecoveryLogWnd::closeEvent(QCloseEvent *e)
{
	emit Closed();
	//this->deleteLater();
}

void CRecoveryLogWnd::OnDblClick(QTreeWidgetItem* pItem)
{
	ShellExecuteW(NULL, NULL, L"explorer.exe", (L"/select,\"" + pItem->text(2).toStdWString() + L"\"").c_str(), NULL, SW_SHOWNORMAL);
}

void CSandMan::OnRecoveryLog()
{
	if (!m_pRecoveryLogWnd->isVisible()) {
		m_pRecoveryLogWnd->setWindowFlag(Qt::WindowStaysOnTopHint, theGUI->IsAlwaysOnTop());
		CSandMan::SafeShow(m_pRecoveryLogWnd);
	}
}
