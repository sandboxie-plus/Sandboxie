#include "stdafx.h"
#include "Finder.h"

bool CFinder::m_DarkMode = false;

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
	m_pSearchLayout->setMargin(0);
	m_pSearchLayout->setSpacing(3);
	m_pSearchLayout->setAlignment(Qt::AlignLeft);

	m_pSearch = new QLineEdit();
	m_pSearch->setMinimumWidth(150);
	m_pSearch->setMaximumWidth(350);
	m_pSearchLayout->addWidget(m_pSearch);
	QObject::connect(m_pSearch, SIGNAL(textChanged(QString)), this, SLOT(OnText()));
    QObject::connect(m_pSearch, SIGNAL(returnPressed()), this, SLOT(OnReturn()));

	m_pCaseSensitive = new QCheckBox(tr("Case Sensitive"));
	m_pSearchLayout->addWidget(m_pCaseSensitive);
	connect(m_pCaseSensitive, SIGNAL(stateChanged(int)), this, SLOT(OnUpdate()));

	m_pRegExp = new QCheckBox(tr("RegExp"));
	m_pSearchLayout->addWidget(m_pRegExp);
	connect(m_pRegExp, SIGNAL(stateChanged(int)), this, SLOT(OnUpdate()));

	m_pSortProxy = qobject_cast<QSortFilterProxyModel*>(pFilterTarget);

	if (m_pSortProxy) {
		m_pColumn = new QComboBox();
		m_pSearchLayout->addWidget(m_pColumn);
		connect(m_pColumn, SIGNAL(currentIndexChanged(int)), this, SLOT(OnUpdate()));
		m_pColumn->setVisible(false);
	}
	else
		m_pColumn = NULL;

	if (HighLightOption)
	{
		m_pHighLight = new QCheckBox(tr("Highlight"));
		//m_pHighLight->setChecked(true);
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

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	//setMaximumHeight(30);

	hide();

	if (parent)
	{
		QAction* pFind = new QAction(tr("&Find ..."), parent);
		pFind->setShortcut(QKeySequence::Find);
		pFind->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		parent->addAction(pFind);
		QObject::connect(pFind, SIGNAL(triggered()), this, SLOT(Open()));
	}

	if (pFilterTarget) {
		QObject::connect(this, SIGNAL(SetFilter(const QRegExp&, bool, int)), pFilterTarget, SLOT(SetFilter(const QRegExp&, bool, int)));
		QObject::connect(this, SIGNAL(SelectNext()), pFilterTarget, SLOT(SelectNext()));
	}

	m_pTimer = new QTimer(this);
	m_pTimer->setSingleShot(true);
	m_pTimer->setInterval(500);
	connect(m_pTimer, SIGNAL(timeout()), SLOT(OnUpdate()));

	this->installEventFilter(this);
}

CFinder::~CFinder()
{
}

bool CFinder::eventFilter(QObject* source, QEvent* event)
{
	if (event->type() == QEvent::KeyPress && ((QKeyEvent*)event)->key() == Qt::Key_Escape
		&& ((QKeyEvent*)event)->modifiers() == Qt::NoModifier)
	{
		Close();
		return true; // cancel event
	}

	return QWidget::eventFilter(source, event);
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
	if (!isVisible() || m_pSearch->text().isEmpty())
		return QRegExp();
	return QRegExp(m_pSearch->text(), m_pCaseSensitive->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive, m_pRegExp->isChecked() ? QRegExp::RegExp : QRegExp::FixedString);
}

void CFinder::OnUpdate()
{
	m_pTimer->stop();
	emit SetFilter(GetRegExp(), GetHighLight(), GetColumn());
}

void CFinder::OnText()
{
	m_pTimer->stop();
	m_pTimer->start();
}

void CFinder::OnReturn()
{
	OnUpdate();
	if (m_pHighLight->isChecked())
		emit SelectNext();
}

void CFinder::Close()
{
	emit SetFilter(QRegExp());
	hide();
}