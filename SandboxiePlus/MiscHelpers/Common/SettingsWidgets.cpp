#include "stdafx.h"
#include "SettingsWidgets.h"
#include "CheckableMessageBox.h"
#include "NeonEffect.h"

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

//////////////////////////////////////////////////////////////////////////
// CConfigDialog

CConfigDialog::CConfigDialog(QWidget* parent) 
	: QDialog(parent) 
{
	m_pStack = NULL;
	m_pSearch = NULL;
	m_pTree = NULL;
}

QWidget* CConfigDialog::ConvertToTree(QTabWidget* pTabWidget)
{
	QWidget* pAltView = new QWidget(this);
	QGridLayout* pLayout = new QGridLayout(pAltView);
	pLayout->setContentsMargins(0, 0, 0, 0);
	m_pTree = new QTreeWidget();
	m_pTree->setHeaderHidden(true);
	m_pTree->setMinimumWidth(200);
	QStyle* pStyle = QStyleFactory::create("windows"); // show lines
	m_pTree->setStyle(pStyle);
	connect(m_pTree, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(OnItemClicked(QTreeWidgetItem*, int)));
	m_pSearch = new QLineEdit();
	m_pSearch->setPlaceholderText(tr("Search Option"));
	QObject::connect(m_pSearch, SIGNAL(returnPressed()), this, SLOT(OnSearchOption()));
	m_SearchI = m_SearchJ = m_SearchP = 0;
	m_LastFound = NULL;
	pLayout->addWidget(m_pSearch, 0, 0);
	pLayout->addWidget(m_pTree, 1, 0);
	m_pStack = new QStackedLayout();
	m_pStack->setContentsMargins(0, 0, 0, 0);
	pLayout->addLayout(m_pStack, 0, 1, 2, 1);

	for (int i = 0, k = 0; i < pTabWidget->count(); i++, k++) {
		QTreeWidgetItem* pItem = new QTreeWidgetItem(QStringList() << pTabWidget->tabText(i));
		m_pTree->addTopLevelItem(pItem);
		pItem->setData(1, Qt::UserRole, k);
		pItem->setIcon(0, pTabWidget->tabIcon(i));
		QGridLayout* pGrid = qobject_cast<QGridLayout*>(pTabWidget->widget(i)->layout());
		QTabWidget* pTabs = pGrid ? qobject_cast<QTabWidget*>(pGrid->itemAt(0)->widget()) : NULL;
		if (!pTabs) {
			pItem->setData(0, Qt::UserRole, m_pStack->count());
			m_pStack->addWidget(pTabWidget->widget(i--));
		}
		else {
			//pItem->setData(0, Qt::UserRole, -1);
			//pItem->setFlags(pItem->flags() & ~Qt::ItemIsSelectable);
			pItem->setData(0, Qt::UserRole, m_pStack->count()); // take the first tab for the parent entry
			for (int j = 0; j < pTabs->count(); j++) {
				QTreeWidgetItem* pSubItem = new QTreeWidgetItem(QStringList() << pTabs->tabText(j));
				pItem->addChild(pSubItem);
				pSubItem->setData(0, Qt::UserRole, m_pStack->count());
				m_pStack->addWidget(pTabs->widget(j--));
			}
		}
	}

	m_pTree->expandAll();

	pTabWidget->clear();
	return pAltView;
}

template <class T>
bool CConfigDialog__CompareText(T pWidget, const QString& Text)  {
	QString Str = pWidget->text();
	if (!Str.toLower().contains(Text))
		return false;
	qDebug() << Str;
	return true;
}

QWidget* CConfigDialog__SearchWidget(QWidget* pParent, const QString& Text, int& Pos)
{
	QList<QWidget*> Widgets = pParent->findChildren<QWidget*>();
	for (Pos; Pos < Widgets.count(); Pos++) {
		QWidget* pWidget = Widgets[Pos];
		if (!pWidget->isHidden()) {
			if (QCheckBox* pCheck = qobject_cast<QCheckBox*>(pWidget)) {
				if (CConfigDialog__CompareText(pCheck, Text))
					return pCheck;
			}
			else if (QLabel* pLabel = qobject_cast<QLabel*>(pWidget)) {
				if (CConfigDialog__CompareText(pLabel, Text))
					return pLabel;
			}
		}
	}
	Pos = 0;
	return NULL;
}

QWidget* CConfigDialog__SearchOption(QTreeWidget* pTree, QStackedLayout* pStack, const QString& Text, int& I, int& J, int& Pos)
{
	for (; I < pTree->topLevelItemCount(); I++) {
		QTreeWidgetItem* pItem = pTree->topLevelItem(I);
		if (pItem->childCount() == 0) {
			int Index = pItem->data(0, Qt::UserRole).toInt();
			if (QWidget* pWidget = CConfigDialog__SearchWidget(pStack->widget(Index), Text, Pos))
				return pWidget;
		}
		else {
			for (; J < pItem->childCount(); J++) {
				QTreeWidgetItem* pSubItem = pItem->child(J);
				int Index = pSubItem->data(0, Qt::UserRole).toInt();
				if (QWidget* pWidget = CConfigDialog__SearchWidget(pStack->widget(Index), Text, Pos))
					return pWidget;
			}
			J = 0;
		}
	}
	I = 0;
	return NULL;
}

void CConfigDialog::OnSearchOption()
{
	QString Text = m_pSearch->text().toLower();

	if (m_LastFound) {
		//m_LastFound->setPalette(QApplication::palette());
		//m_LastFound->setAutoFillBackground(false);
		m_LastFound->setGraphicsEffect(NULL);
	}

	QWidget* pWidget = CConfigDialog__SearchOption(m_pTree, m_pStack, Text, m_SearchI, m_SearchJ, m_SearchP);
	if (!pWidget) {
		QApplication::beep();
		return;
	}
	m_SearchP++; // move index to the next for the next search

	QTreeWidgetItem* pItem = m_pTree->topLevelItem(m_SearchI);
	if (pItem && pItem->childCount() > 0)
		pItem = pItem->child(m_SearchJ);
	if (!pItem) return;
	//m_pTree->setItemSelected(pItem, true);
	m_pTree->setCurrentItem(pItem, 0);
	OnItemClicked(pItem, 0);

	m_LastFound = pWidget;
	//QPalette palette = m_LastFound->palette();
	//palette.setColor(QPalette::Button, Qt::red);
	//palette.setColor(QPalette::Base, Qt::red);
	//palette.setColor(QPalette::Window, Qt::red);
	//m_LastFound->setAutoFillBackground(true);
	//m_LastFound->setPalette(palette);
	auto neon = new CNeonEffect(5, 4);
	neon->setGlowColor(Qt::red);
	//neon->setColor(Qt::black);
	m_LastFound->setGraphicsEffect(neon);
}

void CConfigDialog::OnItemClicked(QTreeWidgetItem* pItem, int Column)
{
	int Index = pItem->data(0, Qt::UserRole).toInt();
	if (Index != -1)
		m_pStack->setCurrentIndex(Index);

	QTreeWidgetItem* pRootItem = pItem;
	while (pRootItem->parent()) pRootItem = pRootItem->parent();
	int RootIndex = pRootItem->data(1, Qt::UserRole).toInt();
	if (m_iCurrentTab != RootIndex)
		OnTab(RootIndex);
}
