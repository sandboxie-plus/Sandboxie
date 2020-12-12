#pragma once

#include "../mischelpers_global.h"

class MISCHELPERS_EXPORT CProgressDialog : public QMainWindow
{
	Q_OBJECT

public:
	CProgressDialog(const QString& Prompt, QWidget* parent = 0)
	 : QMainWindow(parent)
	{
		setWindowFlags(Qt::Tool);

		m_pMainWidget = new QWidget();
		m_pMainLayout = new QGridLayout(this);
		m_pMainWidget->setLayout(m_pMainLayout);
		this->setCentralWidget(m_pMainWidget);
 
		m_pMessageLabel = new QLabel(Prompt);
		m_pMessageLabel->setMinimumWidth(300);
		m_pMessageLabel->setWordWrap(true);
		m_pMainLayout->addWidget(m_pMessageLabel, 0, 0, 1, 1);

		m_pProgressBar = new QProgressBar();
		m_pProgressBar->setTextVisible(false);
		m_pProgressBar->setMaximum(0);
		m_pProgressBar->setMinimum(0);
		m_pMainLayout->addWidget(m_pProgressBar, 1, 0, 1, 1);

		m_pButtonBox = new QDialogButtonBox();
		m_pButtonBox->setOrientation(Qt::Horizontal);
		m_pButtonBox->setStandardButtons(QDialogButtonBox::Cancel);
		m_pMainLayout->addWidget(m_pButtonBox, 2, 0, 1, 1);
 
		setFixedSize(sizeHint());

		connect(m_pButtonBox,SIGNAL(rejected()),this,SIGNAL(Cancel()));

		m_TimerId = startTimer(1000);
		m_CountDown = 0;
	}
	~CProgressDialog()
	{
		killTimer(m_TimerId);
	}

signals:
	void		Cancel();

public slots:
	void		OnProgressMessage(const QString& Message, int Progress = -1)
	{
		if(!Message.isEmpty())
			m_pMessageLabel->setText(Message);

		if (Progress == -1)
		{
			if (m_pProgressBar->maximum() != 0)
				m_pProgressBar->setMaximum(0);
		}
		else
		{
			if (m_pProgressBar->maximum() != 100)
				m_pProgressBar->setMaximum(100);

			m_pProgressBar->setValue(Progress);
		}
	}

	void		OnStatusMessage(const QString& Message, int Code = 0)
	{
		//if(Code == 0)
			m_pMessageLabel->setText(Message);
		//else // note: parent can't be set as this window may auto close
		//	QMessageBox::warning(NULL, this->windowTitle(), Message); 
	}

	void		OnFinished()
	{
		m_pButtonBox->button(QDialogButtonBox::Cancel)->setEnabled(false);
		m_CountDown = 3;
	}

protected:
	void timerEvent(QTimerEvent *e)
	{
		if (e->timerId() != m_TimerId) 
		{
			QMainWindow::timerEvent(e);
			return;
		}
		
		if(m_CountDown != 0)
		{
			m_CountDown--;
			m_pButtonBox->button(QDialogButtonBox::Cancel)->setText(tr("Close (%1)").arg(m_CountDown));
			if (m_CountDown == 0)
				close();
		}
	}

	void closeEvent(QCloseEvent *e)
	{
		emit Cancel();
	}

	int					m_TimerId;
	int					m_CountDown;

	QWidget*			m_pMainWidget;
	QGridLayout*		m_pMainLayout;
	QLabel*				m_pMessageLabel;
	QProgressBar*		m_pProgressBar;
	QDialogButtonBox*	m_pButtonBox;
};
