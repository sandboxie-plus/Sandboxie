#pragma once

#include "../mischelpers_global.h"

class MISCHELPERS_EXPORT CExitDialog : public QDialog
{
public:
	CExitDialog(const QString& Prompt, QWidget* parent = 0)
	 : QDialog(parent)
	{
		m_pMainLayout = new QGridLayout(this);
 
		QLabel* pLabel = new QLabel(Prompt);
		m_pMainLayout->addWidget(pLabel, 0, 0, 1, 1);

		m_pButtonBox = new QDialogButtonBox();
		m_pButtonBox->setOrientation(Qt::Horizontal);
		m_pButtonBox->setStandardButtons(QDialogButtonBox::Yes|QDialogButtonBox::No);
		m_pMainLayout->addWidget(m_pButtonBox, 2, 0, 1, 1);
 
		connect(m_pButtonBox,SIGNAL(accepted()),this,SLOT(accept()));
		connect(m_pButtonBox,SIGNAL(rejected()),this,SLOT(reject()));

		m_TimerId = startTimer(1000);
		m_CountDown = 15;
	}
	~CExitDialog()
	{
		killTimer(m_TimerId);
	}

protected:
	void timerEvent(QTimerEvent *e)
	{
		if (e->timerId() != m_TimerId) 
		{
			QDialog::timerEvent(e);
			return;
		}

		if(m_CountDown != 0)
		{
			m_CountDown--;
			m_pButtonBox->button(QDialogButtonBox::Yes)->setText(tr("Yes (%1)").arg(m_CountDown));
			if(m_CountDown == 0)
				accept();
		}
	}

	void reject()
	{
		hide();
	}

	void closeEvent(QCloseEvent *e)
	{
		hide();
		e->ignore();
	}

	int					m_TimerId;
	int					m_CountDown;

	QGridLayout*		m_pMainLayout;
	QDialogButtonBox *	m_pButtonBox;
};
