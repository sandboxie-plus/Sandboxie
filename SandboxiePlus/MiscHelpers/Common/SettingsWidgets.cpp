#include "stdafx.h"
#include "SettingsWidgets.h"
#include "CheckableMessageBox.h"

///////////////////////////////////////////////////
// CPathEdit

CPathEdit::CPathEdit(bool bDirs, QWidget *parent)
 : CTxtEdit(parent) 
{
	m_bDirs = bDirs;

	QHBoxLayout* pLayout = new QHBoxLayout(this);
	pLayout->setMargin(0);
	m_pEdit = new QLineEdit(this);
	connect(m_pEdit, SIGNAL(textChanged(const QString &)), this, SIGNAL(textChanged(const QString &)));
	pLayout->addWidget(m_pEdit);
	QPushButton* pButton = new QPushButton("...");
	pButton->setMaximumWidth(25);
	connect(pButton, SIGNAL(clicked(bool)), this, SLOT(Browse()));
	pLayout->addWidget(pButton);
}

void CPathEdit::Browse()
{
	QString FilePath = m_bDirs
		? QFileDialog::getExistingDirectory(this, tr("Select Directory"))
		: QFileDialog::getOpenFileName(0, tr("Browse"), "", QString("Any File (*.*)"));
	if (FilePath.isEmpty())
		return;
	if (m_bWinPath)
		FilePath.replace("/", "\\");
	SetText(FilePath);
}

///////////////////////////////////////////////////
// CProxyEdit

CProxyEdit::CProxyEdit(QWidget *parent)
 : CTxtEdit(parent) 
{
	QHBoxLayout* pLayout = new QHBoxLayout(this);
	pLayout->setMargin(0);

	m_pType = new QComboBox();
	m_pType->addItem(QString("No"));
	m_pType->addItem(QString("http"));
	m_pType->addItem(QString("socks5"));
	connect(m_pType, SIGNAL(activated(int)), this, SLOT(OnType(int)));
	pLayout->addWidget(m_pType);

	m_pEdit = new QLineEdit(this);
	connect(m_pEdit, SIGNAL(textChanged(const QString &)), this, SIGNAL(textChanged(const QString &)));
	pLayout->addWidget(m_pEdit);
}

void CProxyEdit::SetText(const QString& Text)
{
	QUrl Url(Text);
	m_pType->setCurrentText(Url.scheme());
	m_pEdit->setText(Text);
}

void CProxyEdit::OnType(int Index)
{
	if(Index == 0)
		m_pEdit->setEnabled(false);
	else
	{
		m_pEdit->setEnabled(true);
		m_pEdit->setText(m_pType->currentText() + "://");
	}
}
