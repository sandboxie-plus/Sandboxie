#include "stdafx.h"
#include "Finder.h"

QWidget* CFinder::AddFinder(QWidget* pList, QObject* pFilterTarget, bool HighLightOption, CFinder** ppFinder)
{
	QWidget* pWidget = new QWidget();
	QVBoxLayout* pLayout = new QVBoxLayout();
	pLayout->setMargin(0);
	pWidget->setLayout(pLayout);

	pLayout->addWidget(pList);
	CFinder* pFinder = new CFinder(pFilterTarget, pWidget, HighLightOption);
	pLayout->addWidget(pFinder);

	if (ppFinder)
		*ppFinder = pFinder;
	return pWidget;
}

CFinder::CFinder(QObject* pFilterTarget, QWidget *parent, bool HighLightOption)
:QWidget(parent)
{
	m_pSearchLayout = new QHBoxLayout();
	m_pSearchLayout->setMargin(3);
	m_pSearchLayout->setSpacing(3);
	m_pSearchLayout->setAlignment(Qt::AlignLeft);

	m_pSearch = new QLineEdit();
	m_pSearch->setMinimumWidth(150);
	m_pSearch->setMaximumWidth(350);
	m_pSearchLayout->addWidget(m_pSearch);
	QObject::connect(m_pSearch, SIGNAL(textChanged(QString)), this, SLOT(OnUpdate()));
    //QObject::connect(m_pSearch, SIGNAL(returnPressed()), this, SLOT(_q_next()));

	m_pCaseSensitive = new QCheckBox(tr("Case Sensitive"));
	m_pSearchLayout->addWidget(m_pCaseSensitive);
	connect(m_pCaseSensitive, SIGNAL(stateChanged(int)), this, SLOT(OnUpdate()));

	m_pRegExp = new QCheckBox(tr("RegExp"));
	m_pSearchLayout->addWidget(m_pRegExp);
	connect(m_pRegExp, SIGNAL(stateChanged(int)), this, SLOT(OnUpdate()));

	m_pColumn = new QComboBox();
	m_pSearchLayout->addWidget(m_pColumn);
	connect(m_pColumn, SIGNAL(currentIndexChanged(int)), this, SLOT(OnUpdate()));
	m_pColumn->setVisible(false);

	if (HighLightOption)
	{
		m_pHighLight = new QCheckBox(tr("Highlight"));
		m_pSearchLayout->addWidget(m_pHighLight);
		connect(m_pHighLight, SIGNAL(stateChanged(int)), this, SLOT(OnUpdate()));
	}
	else
		m_pHighLight = NULL;

	QToolButton* pClose = new QToolButton(this);
    pClose->setIcon(QIcon(":/close.png"));
    pClose->setAutoRaise(true);
    pClose->setText(tr("Close"));
    m_pSearchLayout->addWidget(pClose);
	QObject::connect(pClose, SIGNAL(clicked()), this, SLOT(Close()));

	QWidget* pSpacer = new QWidget();
	pSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_pSearchLayout->addWidget(pSpacer);

	setLayout(m_pSearchLayout);

	setMaximumHeight(30);

	hide();

	if (parent)
	{
		QAction* pFind = new QAction(tr("&Find ..."), parent);
		pFind->setShortcut(QKeySequence::Find);
		pFind->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		parent->addAction(pFind);
		QObject::connect(pFind, SIGNAL(triggered()), this, SLOT(Open()));
	}

	m_pSortProxy = qobject_cast<QSortFilterProxyModel*>(pFilterTarget);
	if (pFilterTarget)
		QObject::connect(this, SIGNAL(SetFilter(const QRegExp&, bool, int)), pFilterTarget, SLOT(SetFilter(const QRegExp&, bool, int)));
}

CFinder::~CFinder()
{
}


void CFinder::Open()
{
	if (m_pSortProxy && m_pColumn->count() == 0)
	{
		m_pColumn->addItem(tr("All columns"), -1);
		for (int i = 0; i < m_pSortProxy->columnCount(); i++)
			m_pColumn->addItem(m_pSortProxy->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString(), i);
		m_pColumn->setVisible(true);
	}

	show();
	m_pSearch->setFocus(Qt::OtherFocusReason);
	m_pSearch->selectAll();
	OnUpdate();
}

QRegExp CFinder::GetRegExp() const
{
	if (!isVisible())
		return QRegExp();
	return QRegExp(m_pSearch->text(), m_pCaseSensitive->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive, m_pRegExp->isChecked() ? QRegExp::RegExp : QRegExp::FixedString);
}

void CFinder::OnUpdate()
{
	emit SetFilter(GetRegExp(), GetHighLight(), GetColumn());
}

void CFinder::Close()
{
	emit SetFilter(QRegExp());
	hide();
}