#include "stdafx.h"
#include "Finder.h"

bool CFinder::m_DarkMode = false;

QString CFinder::m_CaseInsensitive = "Case Sensitive";
QString CFinder::m_RegExpStr = "RegExp";
QString CFinder::m_Highlight = "Highlight";
QString CFinder::m_CloseStr = "Close";
QString CFinder::m_FindStr = "&Find ...";
QString CFinder::m_AllColumns = "All columns";

QWidget* CFinder::AddFinder(QTreeView* pTree, QObject* pFilterTarget, int iOptions, CFinder** ppFinder)
{
	QWidget* pWidget = new QWidget();
	QVBoxLayout* pLayout = new QVBoxLayout();
	pLayout->setContentsMargins(0,0,0,0);
	pWidget->setLayout(pLayout);

	pLayout->addWidget(pTree);
	CFinder* pFinder = new CFinder(pFilterTarget, pWidget, iOptions);
	pFinder->SetTree(pTree);
	pLayout->addWidget(pFinder);

	if (ppFinder)
		*ppFinder = pFinder;
	return pWidget;
}

CFinder::CFinder(QObject* pFilterTarget, QWidget *parent, int iOptions)
:QWidget(parent)
{
	m_pSearchLayout = new QHBoxLayout();
	m_pSearchLayout->setContentsMargins(0,0,0,0);
	m_pSearchLayout->setSpacing(3);
	m_pSearchLayout->setAlignment(Qt::AlignLeft);

	m_pSearch = new QLineEdit();
	m_pSearch->setMinimumWidth(150);
	m_pSearch->setMaximumWidth(350);
	m_pSearchLayout->addWidget(m_pSearch);
	QObject::connect(m_pSearch, SIGNAL(textChanged(QString)), this, SLOT(OnText()));
    QObject::connect(m_pSearch, SIGNAL(returnPressed()), this, SLOT(OnReturn()));

	if ((iOptions & eCaseSens) != 0)
	{
		m_pCaseSensitive = new QCheckBox(m_CaseInsensitive);
		m_pSearchLayout->addWidget(m_pCaseSensitive);
		connect(m_pCaseSensitive, SIGNAL(stateChanged(int)), this, SLOT(OnUpdate()));
	}
	else
		m_pCaseSensitive = NULL;

	if ((iOptions & eRegExp) != 0)
	{
		m_pRegExp = new QCheckBox(m_RegExpStr);
		m_pSearchLayout->addWidget(m_pRegExp);
		connect(m_pRegExp, SIGNAL(stateChanged(int)), this, SLOT(OnUpdate()));
	}
	else
		m_pRegExp = NULL;

	m_pTree = NULL;
	m_pModel = qobject_cast<QAbstractItemModel*>(pFilterTarget);

	if (m_pModel) {
		m_pColumn = new QComboBox();
		m_pSearchLayout->addWidget(m_pColumn);
		connect(m_pColumn, SIGNAL(currentIndexChanged(int)), this, SLOT(OnUpdate()));
		m_pColumn->setVisible(false);
	}
	else
		m_pColumn = NULL;

	if ((iOptions & eHighLight) != 0)
	{
		m_pHighLight = new QCheckBox(m_Highlight);
		if ((iOptions & eHighLightDefault) == eHighLightDefault)
			m_pHighLight->setChecked(true);
		m_pSearchLayout->addWidget(m_pHighLight);
		connect(m_pHighLight, SIGNAL(stateChanged(int)), this, SLOT(OnUpdate()));
	}
	else
		m_pHighLight = NULL;

	QToolButton* pClose = new QToolButton(this);
    pClose->setIcon(QIcon(":/close.png"));
    pClose->setAutoRaise(true);
    pClose->setText(m_CloseStr);
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
		QAction* pFind = new QAction(m_FindStr, parent);
		pFind->setShortcut(QKeySequence::Find);
		pFind->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		parent->addAction(pFind);
		QObject::connect(pFind, SIGNAL(triggered()), this, SLOT(Open()));
	}

	if (pFilterTarget) {
		QObject::connect(this, SIGNAL(SetFilter(const QString&, int, int)), pFilterTarget, SLOT(SetFilter(const QString&, int, int)));
		//QObject::connect(this, SIGNAL(SelectNext()), pFilterTarget, SLOT(SelectNext()));
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

void CFinder::SetTree(QTreeView* pTree) 
{ 
	m_pTree = pTree; 
	QObject::connect(this, SIGNAL(SelectNext()), this, SLOT(OnSelectNext()));
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
	if (m_pColumn && m_pColumn->count() == 0)
	{
		m_pColumn->addItem(m_AllColumns, -1);
		for (int i = 0; i < m_pModel->columnCount(); i++)
			m_pColumn->addItem(m_pModel->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString(), i);
		m_pColumn->setVisible(true);
	}

	show();
	m_pSearch->setFocus(Qt::OtherFocusReason);
	m_pSearch->selectAll();
	OnUpdate();
}

void CFinder::OnUpdate()
{
	m_pTimer->stop();
	if (!isVisible() || m_pSearch->text().isEmpty())
		SetFilter(QString(), 0, GetColumn());
	int iOptions = 0;
	if (GetRegExp())
		iOptions |= eRegExp;
	if (GetCaseSensitive())
		iOptions |= eCaseSens;
	if (GetHighLight())
		iOptions |= eHighLight;
	QString Exp = m_pSearch->text();

	QString ExpStr = ((iOptions & CFinder::eRegExp) != 0) ? Exp : (".*" + QRegularExpression::escape(Exp).replace("\\","\\\\") + ".*");
	m_RegExp = QRegularExpression(ExpStr, (iOptions & CFinder::eCaseSens) != 0 ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);

	SetFilter(Exp, iOptions, GetColumn());
}

void CFinder::OnText()
{
	m_pTimer->stop();
	m_pTimer->start();
}

void CFinder::OnReturn()
{
	OnUpdate();
	if (!m_pHighLight || m_pHighLight->isChecked())
		emit SelectNext();
}

void CFinder::Close()
{
	emit SetFilter(QString());
	hide();
}

////////////////////////////////////////////////////////////////
//

bool CFinder::MatchString(const QString& value)
{
	return value.contains(m_RegExp);
}

bool CFinder::MatchCell(QModelIndex idx, int column)
{
	QModelIndex tmp = idx.sibling(idx.row(), column);

	QString str = m_pModel->data(tmp, Qt::DisplayRole).toString();
	return MatchString(str);
}

bool CFinder::MatchRow(QModelIndex idx)
{
	int iColumn = GetColumn();
	if (iColumn != -1)
		return MatchCell(idx, iColumn);

	for(int col = 0; col < m_pModel->columnCount(idx); col++) {
		if (MatchCell(idx, col))
			return true;
	}
	return false;
}

QModelIndex	CFinder::FindRow(QModelIndex par, int start, bool reverse)
{
	int numRows = m_pModel->rowCount(par);
	for (int row = start; row < numRows && row >= 0; row += (reverse ? -1 : 1)) {
		QModelIndex cur = m_pModel->index(row, 0, par);
		if (MatchRow(cur))
			return cur;
		if (m_pModel->hasChildren(cur)) {
			QModelIndex child = FindRow(cur, reverse ? m_pModel->rowCount(cur) - 1 : 0, reverse);
			if (child.isValid())
				return child;
		}
	}

	return QModelIndex();
}

QModelIndex	CFinder::FindRow(bool reverse)
{
	bool next = true;
	QModelIndex idx = m_pTree->currentIndex();
	if (!(next = idx.isValid()))
		idx = m_pModel->index(0, 0);

next_sibling:
	if (idx.isValid() && m_pModel->hasChildren(idx)) {
		QModelIndex child = FindRow(idx, reverse ? m_pModel->rowCount(idx) - 1 : 0, reverse);
		if (child.isValid())
			return child;
	}

	QModelIndex cur = FindRow(m_pModel->parent(idx), idx.row() + (next ? (reverse ? -1 : 1) : 0), reverse);
	if (cur.isValid())
		return cur;

next_parent:
	QModelIndex parent = m_pModel->parent(idx);
	if (parent.isValid()) {
		QModelIndex sibling = parent.siblingAtRow(parent.row() + (reverse ? -1 : 1));
		if (sibling.isValid()) {
			idx = sibling;
			next = false;
			goto next_sibling;
		} else {
			idx = parent;
			goto next_parent;
		}
	}

	return QModelIndex();
}

void CFinder::OnSelectNext() 
{
	if (!m_pModel)
		return;

	QModelIndex idx = FindRow(QApplication::keyboardModifiers() & Qt::ShiftModifier);

	if (idx.isValid())
		m_pTree->setCurrentIndex(idx);
	else
		QApplication::beep();
}