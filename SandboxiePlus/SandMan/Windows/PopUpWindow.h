#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_PopUpWindow.h"
#include "../SbiePlusAPI.h"

class CPopUpEntry: public QWidget
{
	Q_OBJECT
public:
	CPopUpEntry(const QString& Message, QWidget* parent = 0) : QWidget(parent)
	{
		m_Message = Message;

		m_pMainLayout = new QGridLayout(this);
		m_pMainLayout->setContentsMargins(6, 0, 6, 0);
		this->setLayout(m_pMainLayout);
	}
	virtual ~CPopUpEntry() {}

	virtual QString		GetMessageText() { return m_Message; }
	
protected:

	QGridLayout*		m_pMainLayout;
	QString				m_Message;
};

class CPopUpMessage : public CPopUpEntry
{
	Q_OBJECT
public:
	CPopUpMessage(const QString& Message, quint32 MsgCode, const QStringList& MsgData, QWidget* parent = 0) : CPopUpEntry(Message, parent)
	{
		m_MsgCode = MsgCode;
		m_MsgData = MsgData;

		QLabel* pLabel = new QLabel(Message);
		pLabel->setToolTip(Message);
		pLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Maximum);
		m_pMainLayout->addWidget(pLabel, 0, 0);

		if (MsgCode != 0)
		{
			QToolButton* pHelp = new QToolButton();
			pHelp->setText(tr("?"));
			pHelp->setToolTip(tr("Visit %1 for a detailed explanation.").arg(QString("https://sandboxie-plus.com/go.php?to=sbie-sbie%1/").arg(GetMsgId())));
			pHelp->setMaximumWidth(16);
			QObject::connect(pHelp, SIGNAL(clicked(bool)), this, SLOT(OnHelp()));
			m_pMainLayout->addWidget(pHelp, 0, 1);


			QToolButton* pDismiss = new QToolButton();
			pDismiss->setText(tr("Dismiss"));
			pDismiss->setToolTip(tr("Remove this message from the list"));
			pDismiss->setPopupMode(QToolButton::MenuButtonPopup);
			QMenu* pMenu = new QMenu();
			pMenu->addAction(tr("Hide all such messages"), this, SIGNAL(Hide()));
			pDismiss->setMenu(pMenu);
			//QObject::connect(pDismiss, SIGNAL(triggered(QAction*)), , SLOT());
			QObject::connect(pDismiss, SIGNAL(clicked(bool)), this, SIGNAL(Dismiss()));
			m_pMainLayout->addWidget(pDismiss, 0, 2);
		}
	}

	quint32				GetMsgCode() { return m_MsgCode; }
	quint32				GetMsgId()   { return m_MsgCode & 0xFFFF; }
	QStringList			GetMsgData() { return m_MsgData; }
	QString				GetMsgData(int Index) { return m_MsgData.size() <= Index ? QString() : m_MsgData[Index]; }

signals:
	void				Dismiss();
	void				Hide();

private slots:
	void				OnHelp() { QDesktopServices::openUrl(QUrl(QString("https://sandboxie-plus.com/go.php?to=sbie-sbie%1/").arg(GetMsgId()))); }

protected:
	quint32				m_MsgCode;
	QStringList			m_MsgData;
};

class CPopUpPrompt : public CPopUpEntry
{
	Q_OBJECT
public:
	CPopUpPrompt(const QString& Message, quint32 RequestId, QVariantMap Result, const CBoxedProcessPtr& pProcess, QWidget* parent = 0) : CPopUpEntry(Message, parent)
	{
		m_RequestId = RequestId;
		m_Result = Result;
		m_pProcess = pProcess;
		m_bAddToList = false;

		m_pLabel = new QLabel(Message);
		m_pLabel->setToolTip(Message);
		m_pLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Maximum);
		m_pMainLayout->addWidget(m_pLabel, 0, 0, 1, 5);

		m_pRemember = new QCheckBox(tr("Remember for this process"));
		m_pRemember->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
		m_pMainLayout->addWidget(m_pRemember, 1, 0);

		m_pTimeOut = new QLabel();
		//m_pTimeOut->setStyleSheet("background-color:red;");
		m_pMainLayout->addWidget(m_pTimeOut, 1, 1);


		m_pYes = new QToolButton();
		m_pYes->setText(tr("Yes"));
		connect(m_pYes, SIGNAL(clicked(bool)), this, SLOT(OnAccepted()));
		m_pMainLayout->addWidget(m_pYes, 1, 2);

		m_pNo = new QToolButton();
		m_pNo->setText(tr("No"));
		connect(m_pNo, SIGNAL(clicked(bool)), this, SLOT(OnRejected()));
		m_pMainLayout->addWidget(m_pNo, 1, 3);

		m_pTerminate = new QToolButton();
		m_pTerminate->setText(tr("Terminate"));
		connect(m_pTerminate, SIGNAL(clicked(bool)), this, SLOT(OnTerminate()));
		m_pMainLayout->addWidget(m_pTerminate, 1, 4);


		m_iTimeOutSec = 60; // todo: communicate the actual value from dll or have a setting for that

		if (RequestId == 0)
			m_uTimerID = -1;
		else
			m_uTimerID = startTimer(1000);
	}
	~CPopUpPrompt() {
		if(m_uTimerID != -1)
			killTimer(m_uTimerID);
	}

	void AddAddToList()
	{
		m_pYes->setPopupMode(QToolButton::MenuButtonPopup);
		QMenu* pMenu = new QMenu();
		pMenu->addAction(tr("Yes and add to allowed programs"), this, SLOT(OnAcceptedAlways()));
		m_pYes->setMenu(pMenu);
	}

signals:
	void		PromptResult(int retval);

private slots:
	void		OnAcceptedAlways() { m_bAddToList = true; emit PromptResult(1); }
	void		OnAccepted() { emit PromptResult(1); }
	void		OnRejected() { emit PromptResult(0); }
	void		OnTerminate() { emit PromptResult(-1); }

protected:
	friend class CPopUpWindow;

	void		timerEvent(QTimerEvent* pEvent) 
	{
		if (pEvent->timerId() != m_uTimerID)
			return;
		if (m_iTimeOutSec > 0)
		{
			m_iTimeOutSec--;

			extern bool CPopUpWindow__DarkMode;
			if (CPopUpWindow__DarkMode) {
				QPalette palette = m_pLabel->palette();
				palette.setColor(QPalette::Text, Qt::red);
				m_pLabel->setPalette(palette);
			}
			else
				repaint();
		}

		if (m_pProcess->IsTerminated()) {
			m_pTimeOut->setText(tr("Requesting process terminated"));
			m_pYes->setEnabled(false);
			m_pTerminate->setEnabled(false);
		}
		else if (m_iTimeOutSec > 0)
			m_pTimeOut->setText(tr("Request will time out in %1 sec").arg(m_iTimeOutSec));
		else {
			m_pTimeOut->setText(tr("Request timed out").arg(m_iTimeOutSec));
			m_pYes->setEnabled(false);
		}
	}
	int					m_uTimerID;

	virtual void		paintEvent(QPaintEvent *event)
	{
		QWidget::paintEvent(event);

		extern bool CPopUpWindow__DarkMode;
		if (!CPopUpWindow__DarkMode && (m_iTimeOutSec % 2) != 0) 
		{
			QPainter p(this);
			p.fillRect(2, 1, width() - 4, height() - 2, QColor(0xFF, 0xCC, 0xCC));
		}
	}

	quint32				m_RequestId;
	QVariantMap			m_Result;
	CBoxedProcessPtr	m_pProcess;
	bool				m_bAddToList;

	QLabel*				m_pLabel;
	QCheckBox*			m_pRemember;
	qint32				m_iTimeOutSec;
	QLabel*				m_pTimeOut;
	QToolButton*		m_pYes;
	QToolButton*		m_pNo;
	QToolButton*		m_pTerminate;
};

class CPopUpRecovery : public CPopUpEntry
{
	Q_OBJECT
public:
	CPopUpRecovery(const QString& Message, const QString& FilePath, const QString& BoxName, QWidget* parent = 0) : CPopUpEntry(Message, parent)
	{
		m_BoxName = BoxName;
		m_FilePath = FilePath;

		QLabel* pLabel = new QLabel(Message);
		pLabel->setToolTip(Message);
		pLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Maximum);
		//pLabel->setWordWrap(true);
		extern bool CPopUpWindow__DarkMode;
		if(CPopUpWindow__DarkMode) {
			QPalette palette = pLabel->palette();
			palette.setColor(QPalette::Text, Qt::green);
			pLabel->setPalette(palette);
		}
		m_pMainLayout->addWidget(pLabel, 0, 0, 2, 4);

		m_pMainLayout->addWidget(new QLabel(tr("Recover to:")), 2, 0);
		m_pTarget = new QComboBox();

		m_LastTargetIndex = 0;
		m_pTarget->addItem(m_FilePath.left(m_FilePath.lastIndexOf("\\")));
		m_pTarget->addItem(tr("Browse"), 1);
		m_pTarget->addItem(tr("Clear folder list"), -1);
		m_ListCleared = false;
		connect(m_pTarget, SIGNAL(currentIndexChanged(int)), this, SLOT(OnTargetChanged()));

		m_pTarget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		m_pMainLayout->addWidget(m_pTarget, 2, 1);

		QToolButton* pRecover = new QToolButton();
		pRecover->setText(tr("Recover"));
		pRecover->setToolTip(tr("Recover the file to original location"));
		pRecover->setPopupMode(QToolButton::MenuButtonPopup);
		QMenu* pRecMenu = new QMenu();
		pRecMenu->addAction(tr("Recover && Explore"), this, SLOT(OnRecoverExp()));
		pRecMenu->addAction(tr("Recover && Open/Run"), this, SLOT(OnRecoverRun()));
		pRecMenu->addSeparator();
		pRecMenu->addAction(tr("Open file recovery for this box"), this, SIGNAL(OpenRecovery()));
		pRecover->setMenu(pRecMenu);
		//QObject::connect(pRecover, SIGNAL(triggered(QAction*)), , SLOT());
		QObject::connect(pRecover, SIGNAL(clicked(bool)), this, SLOT(OnRecover()));
		m_pMainLayout->addWidget(pRecover, 2, 2);


		QToolButton* pDismiss = new QToolButton();
		pDismiss->setText(tr("Dismiss"));
		pDismiss->setToolTip(tr("Don't recover this file right now"));
		pDismiss->setPopupMode(QToolButton::MenuButtonPopup);
		QMenu* pMenu = new QMenu();
		pMenu->addAction(tr("Dismiss all from this box"), this, SLOT(OnDismissAll()));
		pMenu->addAction(tr("Disable quick recovery until the box restarts"), this, SLOT(OnDisable()));
		pDismiss->setMenu(pMenu);
		//QObject::connect(pDismiss, SIGNAL(triggered(QAction*)), , SLOT());
		QObject::connect(pDismiss, SIGNAL(clicked(bool)), this, SIGNAL(Dismiss()));
		m_pMainLayout->addWidget(pDismiss, 2, 3);
	}

signals:
	void		Dismiss(int iFlag = 0);
	void		RecoverFile(int Action = 0);
	void		OpenRecovery();

private slots:
	void		OnDisable()		{ emit Dismiss(0x03); }
	void		OnDismissAll()	{ emit Dismiss(0x01); }
	void		OnRecover()		{ emit RecoverFile(); }
	void		OnRecoverRun()	{ emit RecoverFile(1); }
	void		OnRecoverExp()  { emit RecoverFile(2); }

	void		OnTargetChanged()
	{
		int op = m_pTarget->currentData().toInt();
		if (op == 1)
		{
			QString Folder = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");
			if (Folder.isEmpty()) {
				m_pTarget->setCurrentIndex(m_LastTargetIndex);
				return;
			}
			m_LastTargetIndex = m_pTarget->count() - 1;
			m_pTarget->insertItem(m_LastTargetIndex, Folder);
			m_pTarget->setCurrentIndex(m_LastTargetIndex);
		}
		else if (op == -1)
		{
			while (m_pTarget->count() > 3)
				m_pTarget->removeItem(2);
			m_pTarget->setCurrentIndex(0);
			m_ListCleared = true;
		}
	}

protected:
	friend class CPopUpWindow;

	virtual void		paintEvent(QPaintEvent *event)
	{
		QWidget::paintEvent(event);

		extern bool CPopUpWindow__DarkMode;
		if (!CPopUpWindow__DarkMode) {
			QPainter p(this);
			p.fillRect(2, 1, width() - 4, height() - 2, QColor(0xCC, 0xFF, 0xCC));
		}
	}

	QString				m_FilePath;
	QString				m_BoxName;
	QComboBox*			m_pTarget;
	int					m_LastTargetIndex;
	bool				m_ListCleared;
};

class CPopUpProgress : public CPopUpEntry
{
	Q_OBJECT
public:
	CPopUpProgress(const QString& Message, const QString& ID, quint64 Total, QWidget* parent = 0) : CPopUpEntry(Message, parent)
	{
		m_ID = ID;
		m_Total = Total;

		m_pLabel = new QLabel(Message);
		m_pLabel->setToolTip(Message);
		m_pLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Maximum);
		m_pMainLayout->addWidget(m_pLabel, 0, 0, 1, 3);

		m_pProgressBar = new QProgressBar();
		m_pProgressBar->setTextVisible(false);
		m_pProgressBar->setMaximum(m_Total != 0? 100 : 0);
		m_pProgressBar->setMinimum(0);
		m_pProgressBar->setFixedHeight(16);

		m_pMainLayout->addWidget(m_pProgressBar, 1, 0, 1, 2);

		QToolButton* pDismiss = new QToolButton();
		pDismiss->setText(tr("Dismiss"));
		pDismiss->setToolTip(tr("Remove this progress indicator from the list"));
		//pDismiss->setPopupMode(QToolButton::MenuButtonPopup);
		//QMenu* pMenu = new QMenu();
		//pMenu->addAction(tr("Hide this progress for this process"), this, SIGNAL(Hide()));
		//pDismiss->setMenu(pMenu);
		//QObject::connect(pDismiss, SIGNAL(triggered(QAction*)), , SLOT());
		QObject::connect(pDismiss, SIGNAL(clicked(bool)), this, SIGNAL(Dismiss()));
		m_pMainLayout->addWidget(pDismiss, 1, 2);

		m_iTimeOutSec = 5;

		m_uTimerID = startTimer(1000);
	}
	~CPopUpProgress() {
		if (m_uTimerID != -1)
			killTimer(m_uTimerID);
	}

	void				UpdateProgress(const QString& Message, quint64 Value)
	{
		if(m_Total != 0)
			m_pProgressBar->setValue(100 - (100 * Value / m_Total));
		m_pLabel->setText(Message);
		m_iTimeOutSec = 5;
	}

signals:
	void				Dismiss(bool bHide = true);
	//void				Hide();

protected:
	friend class CPopUpWindow;

	void				timerEvent(QTimerEvent* pEvent)
	{
		if (pEvent->timerId() != m_uTimerID)
			return;
		if (m_iTimeOutSec > 0)
		{
			if (--m_iTimeOutSec == 0)
				emit Dismiss(false);
		}
	}
	int					m_uTimerID;

	QString				m_ID;
	quint64				m_Total;

	QLabel*				m_pLabel;
	QProgressBar*		m_pProgressBar;
	qint32				m_iTimeOutSec;
};


class CPopUpWindow : public QMainWindow
{
	Q_OBJECT
public:
	CPopUpWindow(QWidget* parent = 0);
	~CPopUpWindow();

	virtual void		AddLogMessage(const QString& Message, quint32 MsgCode, const QStringList& MsgData, quint32 ProcessId);
	virtual void		AddUserPrompt(quint32 RequestId, const QVariantMap& Data, quint32 ProcessId);
	virtual void		AddFileToRecover(const QString& FilePath, const QString& BoxName, quint32 ProcessId);
	virtual void		ShowProgress(quint32 MsgCode, const QStringList& MsgData, quint32 ProcessId);

	static void			SetDarkMode(bool bDark) { extern bool CPopUpWindow__DarkMode;  CPopUpWindow__DarkMode = bDark; }

signals:
	void				RecoveryRequested(const QString& BoxName);

public slots:
	virtual void		Show();

	virtual void		ReloadHiddenMessages();
private slots:
	virtual void		OnDismissMessage();
	virtual void		OnHideMessage();

	virtual void		OnPromptResult(int retval);

	virtual void		OnDismiss(int iFlag);
	virtual void		OnRecoverFile(int Action);
	virtual void		OnOpenRecovery();

	virtual void		OnDismissProgress(bool bHide);

	virtual void		OnCopy();

protected:
	virtual void		closeEvent(QCloseEvent *e);

	virtual void		AddEntry(CPopUpEntry* pEntry);
	virtual void		RemoveEntry(CPopUpEntry* pEntry);

	virtual bool		IsMessageHidden(quint32 MsgCode, const QStringList& MsgData);

	virtual void		SendPromptResult(CPopUpPrompt* pEntry, int retval);

	QMultiMap<quint32, QString> m_HiddenMessages;

private:
	bool				m_ResetPosition;
	QAction*			m_pActionCopy;
	Ui::PopUpWindow ui;
};
