#include "stdafx.h"
#include "PopUpWindow.h"
#include <windows.h>
#include <QWindow>
#include "SandMan.h"
#include "../MiscHelpers/Common/Common.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../SbiePlusAPI.h"

bool CPopUpWindow__DarkMode = false;

CPopUpWindow::CPopUpWindow(QWidget* parent) : QMainWindow(parent)
{
	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::CustomizeWindowHint;
	//flags &= ~Qt::WindowContextHelpButtonHint;
	//flags &= ~Qt::WindowSystemMenuHint;
	//flags &= ~Qt::WindowMinMaxButtonsHint;
	flags &= ~Qt::WindowMaximizeButtonHint;
	//flags &= ~Qt::WindowCloseButtonHint;
	setWindowFlags(flags);

	this->setWindowTitle(tr("Sandboxie-Plus Notifications"));

	QWidget* centralWidget = new QWidget();
	ui.setupUi(centralWidget);
	this->setCentralWidget(centralWidget);

	//setWindowFlags(Qt::Tool);

	ui.table->verticalHeader()->hide();
	ui.table->horizontalHeader()->hide();
	ui.table->setColumnCount(1);
	ui.table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

	m_pActionCopy = new QAction();
	connect(m_pActionCopy, SIGNAL(triggered(bool)), this, SLOT(OnCopy()));
	m_pActionCopy->setShortcut(QKeySequence::Copy);
	m_pActionCopy->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	this->addAction(m_pActionCopy);

	m_iTopMost = 0;
	SetWindowPos((HWND)this->winId(), 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	
	m_uTimerID = startTimer(1000);

	m_ResetPosition = !restoreGeometry(theConf->GetBlob("PopUpWindow/Window_Geometry"));
}

CPopUpWindow::~CPopUpWindow()
{
	killTimer(m_uTimerID);

	theConf->SetBlob("PopUpWindow/Window_Geometry", saveGeometry());
}

void CPopUpWindow::AddEntry(CPopUpEntry* pEntry)
{
	int RowCounter = ui.table->rowCount();
	ui.table->insertRow(RowCounter);
	ui.table->setCellWidget(RowCounter, 0, pEntry);
	ui.table->verticalHeader()->setSectionResizeMode(RowCounter, QHeaderView::ResizeToContents);

	ui.table->scrollToBottom();
	
	Show();
}

void CPopUpWindow::RemoveEntry(CPopUpEntry* pEntry)
{
	for (int i = 0; i < ui.table->rowCount(); i++)
	{
		CPopUpEntry* pCurEntry = qobject_cast<CPopUpEntry*>(ui.table->cellWidget(i, 0));
		if (pCurEntry && pCurEntry == pEntry) {
			ui.table->removeRow(i--);
			break;
		}
	}

	if (ui.table->rowCount() == 0)
		this->hide();
}

void CPopUpWindow::Show()
{
	Poke();

	QScreen *screen = this->windowHandle()->screen();
	QRect scrRect = screen->availableGeometry();

	if (!m_ResetPosition)
	{
		QRect wndRect = this->frameGeometry();
		if (wndRect.bottom() > scrRect.height() || wndRect.right() > scrRect.width() || wndRect.top() < 0 || wndRect.left() < 0)
			m_ResetPosition = true;
	}

	if (m_ResetPosition)
	{
		m_ResetPosition = false;

		this->resize(600, 200);
		this->move(scrRect.width() - 600 - 20, scrRect.height() - 200 - 50);
	}

	SafeShow(this);
}

void CPopUpWindow::Poke()
{
	if (!this->isVisible() || m_iTopMost <= -5) {
		SetWindowPos((HWND)this->winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		m_iTopMost = 5;
	}
}

void CPopUpWindow::closeEvent(QCloseEvent *e)
{
	for (int i = 0; i < ui.table->rowCount(); i++)
	{
		CPopUpPrompt* pEntry = qobject_cast<CPopUpPrompt*>(ui.table->cellWidget(i, 0));
		if (pEntry)
			SendPromptResult(pEntry, 0);
	}

	ui.table->clear();
	ui.table->setRowCount(0);

	e->ignore();

	this->hide();
}

void CPopUpWindow::timerEvent(QTimerEvent* pEvent)
{
	if (pEvent->timerId() != m_uTimerID)
		return;

	if (m_iTopMost > -5 && (--m_iTopMost == 0)) {
		SetWindowPos((HWND)this->winId(), HWND_NOTOPMOST , 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
}

void CPopUpWindow::AddLogMessage(const QString& Message, quint32 MsgCode, const QStringList& MsgData, quint32 ProcessId)
{
	if (IsMessageHidden(MsgCode, MsgData))
		return;

	CPopUpMessage* pEntry = new CPopUpMessage(Message, MsgCode, MsgData, this);
	QObject::connect(pEntry, SIGNAL(Dismiss()), this, SLOT(OnDismissMessage()));
	QObject::connect(pEntry, SIGNAL(Hide()), this, SLOT(OnHideMessage()));
	AddEntry(pEntry);

	if ((MsgCode & 0xFFFF) == 1319) // Blocked spooler print to file
	{
		CBoxedProcessPtr pProcess = theAPI->GetProcessById(ProcessId);
		if (pProcess.isNull() || pProcess->IsTerminated())
			return;

		QString Message2 = tr("Do you want to allow the print spooler to write outside the sandbox for %1 (%2)?").arg(pProcess->GetProcessName()).arg(pProcess->GetProcessId());

		QVariantMap Result;
		Result["id"] = (int)CSbieAPI::ePrintSpooler;

		CPopUpPrompt* pEntry = new CPopUpPrompt(Message2, 0, Result, pProcess, this);
		pEntry->m_pRemember->setVisible(false);
		connect(pEntry, SIGNAL(PromptResult(int)), this, SLOT(OnPromptResult(int)));
		AddEntry(pEntry);
	}

	// 2219 // elevation is disabled

	// 1307 // internet denied

	// 1308 // start/run denied
}

void CPopUpWindow::ReloadHiddenMessages()
{
	m_HiddenMessages.clear();

	if (theAPI->GetUserSettings() == NULL)
		return;

	QStringList HiddenMessages = theAPI->GetUserSettings()->GetTextList("SbieCtrl_HideMessage", true);
	foreach(const QString& HiddenMessage, HiddenMessages)
	{
		StrPair CodeDetail = Split2(HiddenMessage, ",");
		m_HiddenMessages.insert(CodeDetail.first.toInt(), CodeDetail.second);
	}
}

void CPopUpWindow::OnDismissMessage()
{
	CPopUpMessage* pEntry = qobject_cast<CPopUpMessage*>(sender());
	RemoveEntry(pEntry);
}

void CPopUpWindow::OnHideMessage()
{
	CPopUpMessage* pEntry = qobject_cast<CPopUpMessage*>(sender());
	
	if (QMessageBox("Sandboxie-Plus", theAPI->GetSbieMsgStr(3647, theGUI->m_LanguageId).arg(pEntry->GetMsgId()).arg("")
		, QMessageBox::Question, QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape, QMessageBox::NoButton, this).exec() != QMessageBox::Yes)
		return;

	m_HiddenMessages.insert(pEntry->GetMsgId(), pEntry->GetMsgData(1));
	if (theAPI->GetUserSettings() != NULL)
		theAPI->GetUserSettings()->AppendText("SbieCtrl_HideMessage", QString("%1, %2").arg(pEntry->GetMsgId()).arg(pEntry->GetMsgData(1)));

	for (int i = 0; i < ui.table->rowCount(); i++)
	{
		CPopUpMessage* pEntry = qobject_cast<CPopUpMessage*>(ui.table->cellWidget(i, 0));
		if (pEntry && IsMessageHidden(pEntry->GetMsgCode(), pEntry->GetMsgData()))
			ui.table->removeRow(i--);
	}

	if(ui.table->rowCount() == 0)
		this->hide();
}

bool CPopUpWindow::IsMessageHidden(quint32 MsgCode, const QStringList& MsgData)
{
	foreach(const QString& Details, m_HiddenMessages.values(MsgCode & 0xFFFF))
	{
		if(Details.isEmpty() || (MsgData.size() >= 2 && Details.compare(MsgData[1]) == 0))
			return true;
	}
	return false;
}

void CPopUpWindow::AddUserPrompt(quint32 RequestId, const QVariantMap& Data, quint32 ProcessId)
{
	CBoxedProcessPtr pProcess = theAPI->GetProcessById(ProcessId);
	if (pProcess.isNull() || pProcess->IsTerminated())
		return;

	QVariantMap Result;
	Result["id"] = Data["id"];

	int retval = pProcess.objectCast<CSbieProcess>()->GetRememberedAction(Result["id"].toInt());
	if (retval != -1)
	{
		Result["retval"] = retval;
		theAPI->SendReplyData(RequestId, Result);
		return;
	}

	QString Message;
	switch (Data["id"].toInt())
	{
	case CSbieAPI::eFileMigration:
		Message = tr("Do you want to allow %4 (%5) to copy a %1 large file into sandbox: %2?\nFile name: %3")
			.arg(FormatSize(Data["fileSize"].toULongLong())).arg(pProcess->GetBoxName())
			.arg(Data["fileName"].toString())
			.arg(pProcess->GetProcessName()).arg(pProcess->GetProcessId());
		break;
	case CSbieAPI::eInetBlockade:
		Message = tr("Do you want to allow %1 (%2) access to the internet?\nFull path: %3")
			.arg(pProcess->GetProcessName()).arg(pProcess->GetProcessId())
			.arg(pProcess->GetFileName());
		break;
	}

	CPopUpPrompt* pEntry = new CPopUpPrompt(Message, RequestId, Result, pProcess, this);
	switch (pEntry->m_Result["id"].toInt())
	{
		case CSbieAPI::eInetBlockade: 
			pEntry->AddAddToList();
			pEntry->m_pRemember->setChecked(true); 
			break;
	}
	connect(pEntry, SIGNAL(PromptResult(int)), this, SLOT(OnPromptResult(int)));
	AddEntry(pEntry);
}

void CPopUpWindow::OnPromptResult(int retval)
{
	CPopUpPrompt* pEntry = qobject_cast<CPopUpPrompt*>(sender());
	if (retval == -1)
		pEntry->m_pProcess->Terminate();
	else
		SendPromptResult(pEntry, retval);
	RemoveEntry(pEntry);
}

void CPopUpWindow::SendPromptResult(CPopUpPrompt* pEntry, int retval)
{
	if (retval == 1) 
	{
		switch (pEntry->m_Result["id"].toInt())
		{
			case CSbieAPI::ePrintSpooler: 
				theAPI->SetProcessExemption(pEntry->m_pProcess->GetProcessId(), 'splr', true); 
				break;
			case CSbieAPI::eInetBlockade: 
				if (pEntry->m_bAddToList)
					pEntry->m_pProcess.objectCast<CSbieProcess>()->SetInternetAccess(true);
				theAPI->SetProcessExemption(pEntry->m_pProcess->GetProcessId(), 'inet', true);
				break;
		}
	}

	if (pEntry->m_RequestId == 0)
		return;

	pEntry->m_Result["retval"] = retval;
	theAPI->SendReplyData(pEntry->m_RequestId, pEntry->m_Result);

	if (pEntry->m_pRemember->isChecked())
		pEntry->m_pProcess.objectCast<CSbieProcess>()->SetRememberedAction(pEntry->m_Result["id"].toInt(), retval);
}

void CPopUpWindow::AddFileToRecover(const QString& FilePath, QString BoxPath, const CSandBoxPtr& pBox, quint32 ProcessId)
{
	CBoxedProcessPtr pProcess = theAPI->GetProcessById(ProcessId);

	QString Message = tr("%1 is eligible for quick recovery from %2.\nThe file was written by: %3")
		.arg(FilePath.mid(FilePath.lastIndexOf("\\") + 1)).arg(QString(pBox->GetName()).replace("_", " "))
		.arg(pProcess.isNull() ? tr("an UNKNOWN process.") : tr("%1 (%2)").arg(pProcess->GetProcessName()).arg(pProcess->GetProcessId()));

	if (BoxPath.isEmpty()) // legacy case, no BoxName, no support for driver serial numbers
		BoxPath = theAPI->GetBoxedPath(pBox->GetName(), FilePath);

	CPopUpRecovery* pEntry = new CPopUpRecovery(Message, FilePath, BoxPath, pBox->GetName(), this);

	QStringList RecoverTargets = theAPI->GetUserSettings()->GetTextList("SbieCtrl_RecoverTarget", true);
	pEntry->m_pTarget->insertItems(pEntry->m_pTarget->count()-1, RecoverTargets);

	connect(pEntry, SIGNAL(Dismiss(int)), this, SLOT(OnDismiss(int)));
	connect(pEntry, SIGNAL(RecoverFile(int)), this, SLOT(OnRecoverFile(int)));
	connect(pEntry, SIGNAL(OpenRecovery()), this, SLOT(OnOpenRecovery()));
	AddEntry(pEntry);
}

void CPopUpWindow::OnDismiss(int iFlag)
{
	CPopUpRecovery* pEntry = qobject_cast<CPopUpRecovery*>(sender());

	if (iFlag == 0)
		RemoveEntry(pEntry);

	if ((iFlag & 0x02) != 0) // disable for this  box
	{
		CSandBoxPtr pBox = theAPI->GetBoxByName(pEntry->m_BoxName);
		if (!pBox.isNull())
			pBox.objectCast<CSandBoxPlus>()->SetSuspendRecovery();
	}
	
	if ((iFlag & 0x01) != 0) // dismiss all from this box
	{
		for (int i = 0; i < ui.table->rowCount(); i++)
		{
			CPopUpRecovery* pCurEntry = qobject_cast<CPopUpRecovery*>(ui.table->cellWidget(i, 0));
			if (pCurEntry && pCurEntry->m_BoxName == pEntry->m_BoxName)
				ui.table->removeRow(i--);
		}

		if (ui.table->rowCount() == 0)
			this->hide();
	}
}

void CPopUpWindow::OnRecoverFile(int Action)
{
	CPopUpRecovery* pEntry = qobject_cast<CPopUpRecovery*>(sender());

	QString RecoveryFolder = pEntry->m_pTarget->currentText();

	if (pEntry->m_pTarget->currentIndex() != 0 || pEntry->m_ListCleared) {
		QStringList RecoverTargets;
		for (int i = 2; i < pEntry->m_pTarget->count() - 1; i++)
			RecoverTargets.append(pEntry->m_pTarget->itemText(i));
		theAPI->GetUserSettings()->UpdateTextList("SbieCtrl_RecoverTarget", RecoverTargets, true);
	}

	QString FileName = pEntry->m_FilePath.mid(pEntry->m_FilePath.lastIndexOf("\\") + 1);
	//QString BoxedFilePath = theAPI->GetBoxedPath(pEntry->m_BoxName, pEntry->m_FilePath); // pEntry->m_BoxPath

	QList<QPair<QString, QString>> FileList;
	FileList.append(qMakePair(pEntry->m_BoxPath, RecoveryFolder + "\\" + FileName));

	SB_PROGRESS Status = theGUI->RecoverFiles(pEntry->m_BoxName, FileList, Action);
	if (Status.GetStatus() == OP_ASYNC)
		theGUI->AddAsyncOp(Status.GetValue());
		
	RemoveEntry(pEntry);
}

void CPopUpWindow::OnOpenRecovery()
{
	CPopUpRecovery* pEntry = qobject_cast<CPopUpRecovery*>(sender());

	CSandBoxPtr pBox = theAPI->GetBoxByName(pEntry->m_BoxName);
	if (pBox)
		theGUI->ShowRecovery(pBox);

	// since we opened the recovery dialog, we can dismiss all the notifications for this box
	OnDismiss(0x01);
}

void CPopUpWindow::ShowProgress(quint32 MsgCode, const QStringList& MsgData, quint32 ProcessId)
{
	QString BoxName = MsgData.size() >= 2 ? MsgData[1] : tr("UNKNOWN");
	QString FilePath = MsgData.size() >= 3 ? theAPI->Nt2DosPath(MsgData[2]) : tr("UNKNOWN");
	quint64 SizeLeft = MsgData.size() >= 4 ? MsgData[3].toULongLong() : 0;

	if (m_HiddenMessages.contains(0, FilePath))
		return;

	QString Message = tr("Migrating a large file %1 into the sandbox %2, %3 left.\nFull path: %4")
		.arg(FilePath.mid(FilePath.lastIndexOf("\\") + 1)).arg(BoxName).arg(FormatSize(SizeLeft))
		.arg(FilePath);

	CPopUpProgress* pEntry = NULL;
	for (int i = 0; i < ui.table->rowCount(); i++)
	{
		CPopUpProgress* pCurEntry = qobject_cast<CPopUpProgress*>(ui.table->cellWidget(i, 0));
		if (pCurEntry && pCurEntry->m_ID == FilePath) {
			pEntry = pCurEntry;
			break;
		}
	}

	if (!pEntry)
	{
		pEntry = new CPopUpProgress(Message, FilePath, SizeLeft, this);
		QObject::connect(pEntry, SIGNAL(Dismiss(bool)), this, SLOT(OnDismissProgress(bool)));
		AddEntry(pEntry);
	}
	else
		pEntry->UpdateProgress(Message, SizeLeft);
}

void CPopUpWindow::OnDismissProgress(bool bHide)
{
	CPopUpProgress* pEntry = qobject_cast<CPopUpProgress*>(sender());

	if (bHide)
		m_HiddenMessages.insert(0, pEntry->m_ID);

	RemoveEntry(pEntry);
}

void CPopUpWindow::OnCopy()
{
	QStringList Messages;
	foreach(const QModelIndex& Index, ui.table->selectionModel()->selectedIndexes())
	{
		CPopUpEntry* pCurEntry = qobject_cast<CPopUpEntry*>(ui.table->cellWidget(Index.row(), 0));
		Messages.append(pCurEntry->GetMessageText());
	}
	QApplication::clipboard()->setText(Messages.join("\n"));
}
