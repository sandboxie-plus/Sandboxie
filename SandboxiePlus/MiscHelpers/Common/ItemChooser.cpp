#include "stdafx.h"
#include "ItemChooser.h"

CItemChooser::CItemChooser(const QString& Prompt, QWidget *parent)
	: QDialog(parent)
{
	this->setWindowTitle(tr("Item Chooser"));

    m_pMainLayout = new QVBoxLayout();
	this->setLayout(m_pMainLayout);

    m_pLabel = new QLabel(Prompt.isNull() ? tr("Select items that will be used.") : Prompt, this);
    m_pLabel->setWordWrap(true);
    m_pMainLayout->addWidget(m_pLabel);

    m_pCenterLayout = new QHBoxLayout();
	m_pMainLayout->addLayout(m_pCenterLayout);

    m_pListAll = new QListWidget(this);
    m_pCenterLayout->addWidget(m_pListAll);
	m_pListAll->setSelectionMode(QAbstractItemView::ExtendedSelection);
	connect(m_pListAll, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(OnAdd()));

    m_pMidleLayout = new QVBoxLayout();
	m_pCenterLayout->addLayout(m_pMidleLayout);

    m_pMidleLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

    m_pBtnAdd = new QPushButton(tr("Add >"), this);
	connect(m_pBtnAdd, SIGNAL(clicked(bool)), this, SLOT(OnAdd()));
    m_pMidleLayout->addWidget(m_pBtnAdd);

    m_pBtnRemove = new QPushButton(tr("< Remove"), this);
	connect(m_pBtnRemove, SIGNAL(clicked(bool)), this, SLOT(OnRemove()));
    m_pMidleLayout->addWidget(m_pBtnRemove);

    m_pMidleLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

    m_pListChoosen = new QListWidget(this);
    m_pCenterLayout->addWidget(m_pListChoosen);
	m_pListChoosen->setSelectionMode(QAbstractItemView::ExtendedSelection);
	connect(m_pListChoosen, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(OnRemove()));

    m_pRightLayout = new QVBoxLayout();
    m_pCenterLayout->addLayout(m_pRightLayout);

    m_pBtnUp = new QPushButton(tr("Move Up"), this);
	connect(m_pBtnUp, SIGNAL(clicked(bool)), this, SLOT(OnUp()));
    m_pRightLayout->addWidget(m_pBtnUp);

    m_pBtnDown = new QPushButton(tr("Move Down"), this);
	connect(m_pBtnDown, SIGNAL(clicked(bool)), this, SLOT(OnDown()));
    m_pRightLayout->addWidget(m_pBtnDown);

    m_pRightLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

	m_InsertIndex = m_pMainLayout->count();

    m_pButtonBox = new QDialogButtonBox(this);
	m_pMainLayout->addWidget(m_pButtonBox);
    m_pButtonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
	connect(m_pButtonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(m_pButtonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void CItemChooser::closeEvent(QCloseEvent *e)
{
	this->deleteLater();
}

void CItemChooser::SetPrompt(const QString& Text)
{
	m_pLabel->setText(Text);
}

void CItemChooser::AddWidgets(QList<QWidget*> Widgets)
{
	foreach(QWidget* pWidget, Widgets)
		AddWidget(pWidget);
}

void CItemChooser::AddWidget(QWidget* pWidget)
{
	m_pMainLayout->insertWidget(m_InsertIndex++, pWidget);
}

void CItemChooser::AddItem(const QString& Label, const QVariant& Data)
{
	QListWidgetItem* pItem = new QListWidgetItem(Label);
	pItem->setData(Qt::UserRole, Data);
	m_pListAll->addItem(pItem);
}

void CItemChooser::OnAdd()
{
	foreach(QListWidgetItem* pItem, m_pListAll->selectedItems())
		AddItem(pItem);
}

void CItemChooser::OnRemove()
{
	foreach(QListWidgetItem* pItem, m_pListChoosen->selectedItems())
		RemoveItem(pItem);
}

void CItemChooser::AddItem(QListWidgetItem* pItem)
{
	pItem->setFlags(pItem->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
	if (m_pListChoosen->findItems(pItem->text(), Qt::MatchExactly).count() > 0)
		return; // already added

	QListWidgetItem* pChoosenItem = new QListWidgetItem(pItem->text());
	pChoosenItem->setData(Qt::UserRole, pItem->data(Qt::UserRole));
	m_pListChoosen->addItem(pChoosenItem);
}

void CItemChooser::RemoveItem(QListWidgetItem* pChoosenItem)
{
	foreach(QListWidgetItem* pItem, m_pListAll->findItems(pChoosenItem->text(), Qt::MatchExactly))
		pItem->setFlags(pItem->flags() | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	delete pChoosenItem;
}

void CItemChooser::OnUp()
{
	for (int i = 0; i < m_pListChoosen->count(); i++)
	{
		QListWidgetItem* pItem = m_pListChoosen->item(i);
		if (!pItem->isSelected())
			continue;
		if (!MoveItem(pItem, -1))
			break;
	}
}

void CItemChooser::OnDown()
{
	for (int i = m_pListChoosen->count()-1; i >= 0 ; i--)
	{
		QListWidgetItem* pItem = m_pListChoosen->item(i);
		if (!pItem->isSelected())
			continue;
		if (!MoveItem(pItem, 1))
			break;
	}
}

bool CItemChooser::MoveItem(QListWidgetItem* pItem, int Pos)
{
    int currIndex = m_pListChoosen->row(pItem);
 
    QListWidgetItem *pPrev = m_pListChoosen->item(m_pListChoosen->row(pItem) + Pos);
	if (!pPrev)
		return false;
    int prevIndex = m_pListChoosen->row(pPrev);
 
    QListWidgetItem *pTemp = m_pListChoosen->takeItem(prevIndex);
    m_pListChoosen->insertItem(prevIndex, pItem);
    m_pListChoosen->insertItem(currIndex, pTemp);
	return true;
}

QVariantList CItemChooser::GetChoosenItems()
{
	QVariantList ChoosenItems;
	for(int i=0; i < m_pListChoosen->count(); i++)
		ChoosenItems.append(m_pListChoosen->item(i)->data(Qt::UserRole));
	return ChoosenItems;
}

void CItemChooser::ChooseItems(const QVariantList& ChoosenItems)
{
	foreach(const QVariant& Data, ChoosenItems)
		ChooseItem(Data);
}

void CItemChooser::ChooseItem(const QVariant& Data)
{
	QListWidgetItem* pItem = NULL;
	for (int i = 0; i < m_pListAll->count(); i++)
	{
		if (m_pListAll->item(i)->data(Qt::UserRole) == Data)
		{
			pItem = m_pListAll->item(i);
			break;
		}
	}

	if (pItem)
		AddItem(pItem);
}