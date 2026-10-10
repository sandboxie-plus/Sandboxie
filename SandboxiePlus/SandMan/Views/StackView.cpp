#include "stdafx.h"
#include "..\SandMan.h"
#include "StackView.h"
#include "..\..\MiscHelpers\Common\Common.h"
#include "..\Helpers\RedactedStackExport.h"
#include "..\Windows\StackExportDialog.h"

CStackView::CStackView(QWidget *parent)
	: CPanelView(parent)
{
	m_pMainLayout = new QVBoxLayout();
	m_pMainLayout->setContentsMargins(0, 0, 0, 0);
	this->setLayout(m_pMainLayout);

	// Stack List
	m_pStackList = new QTreeWidgetEx();
	m_pStackList->setItemDelegate(new CTreeItemDelegate());
	//m_pStackList->setHeaderLabels(tr("#|Symbol|Stack address|Frame address|Control address|Return address|Stack parameters|File info").split("|"));
	m_pStackList->setHeaderLabels(tr("#|Symbol").split("|"));
	m_pStackList->setMinimumHeight(50);

	m_pStackList->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_pStackList->setSortingEnabled(false);

	m_pStackList->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pStackList, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(OnMenu(const QPoint &)));

	m_pMainLayout->addWidget(CFinder::AddFinder(m_pStackList, this, true, &m_pFinder));
	// 

	m_bIsInvalid = false;

	//m_pMenu = new QMenu();
	AddPanelItemsToMenu();

	m_pStackList->header()->restoreState(theConf->GetBlob("MainWindow/StackView_Columns"));

	m_pMenu->addSeparator();
	m_pMenu->addAction(tr("Export Redacted..."), this, SLOT(OnExportRedacted()));
}

CStackView::~CStackView()
{
	theConf->SetBlob("MainWindow/StackView_Columns", m_pStackList->header()->saveState());
}

void CStackView::Invalidate()
{
	m_bIsInvalid = true;

	for (int i = 0; i < m_pStackList->topLevelItemCount(); i++)
	{
		for(int j=0; j < m_pStackList->columnCount(); j++)
			m_pStackList->topLevelItem(i)->setForeground(j, Qt::lightGray);
	}
}

void CStackView::ShowStack(const QVector<quint64>& Stack, const CBoxedProcessPtr& pProcess)
{
	if (m_pCurrentProcess != pProcess)
	{
		if (!m_pCurrentProcess.isNull())
			disconnect(m_pCurrentProcess.data(), SIGNAL(SymbolChanged(quint64)),
				this, SLOT(OnSymbolChanged(quint64)));

		m_pCurrentProcess = pProcess;
		if (!m_pCurrentProcess.isNull())
			connect(m_pCurrentProcess.data(), SIGNAL(SymbolChanged(quint64)),
				this, SLOT(OnSymbolChanged(quint64)), Qt::UniqueConnection);
	}

	m_CurrentStack = Stack;
	pProcess->ResolveSymbols(Stack);

	int i = 0;
	for (; i < Stack.count(); i++)
	{
		quint64 Address = Stack[i];
		
		QTreeWidgetItem* pItem;
		if (i >= m_pStackList->topLevelItemCount())
		{
			pItem = new QTreeWidgetItem();
			pItem->setData(eStack, Qt::UserRole, i);
			pItem->setText(eStack, QString::number(i));
			m_pStackList->addTopLevelItem(pItem);
		}
		else
			pItem = m_pStackList->topLevelItem(i);

		if (m_bIsInvalid)
		{
			for (int j = 0; j < m_pStackList->columnCount(); j++)
				pItem->setForeground(j, Qt::black);
		}

		QString Symbol = pProcess->GetSymbol(Address);
		if (Symbol.isNull()) Symbol = FormatAddress(Address);
		pItem->setText(eSymbol, Symbol);
	}

	for (; i < m_pStackList->topLevelItemCount(); )
		delete m_pStackList->topLevelItem(i);

	CPanelWidgetEx::ApplyFilter(m_pStackList, m_pFinder->isVisible() ? &m_pFinder->GetSearchExp() : NULL);

	m_bIsInvalid = false;
}

void CStackView::OnSymbolChanged(quint64 Address)
{
	if (m_pCurrentProcess.isNull())
		return;

	QString Symbol = m_pCurrentProcess->GetSymbol(Address);
	for (int i = 0; i < m_CurrentStack.count(); i++) {
		if (m_CurrentStack[i] == Address &&
				i < m_pStackList->topLevelItemCount())
			m_pStackList->topLevelItem(i)->setText(eSymbol, Symbol);
	}
}

void CStackView::SetFilter(const QRegularExpression& Exp, int iOptions, int Col)
{
	CPanelWidgetEx::ApplyFilter(m_pStackList, &m_pFinder->GetSearchExp());
}

void CStackView::OnExportRedacted()
{
	SStackCapture Capture;
	Capture.state = SStackCapture::eComplete;
	Capture.frames.reserve(m_CurrentStack.size());
	for (int i = 0; i < m_CurrentStack.size(); ++i) {
		quint64 Address = m_CurrentStack[i];
		SStackFrame Frame;
		Frame.address = Address;
		if (!m_pCurrentProcess.isNull())
			Frame.symbol = m_pCurrentProcess->GetSymbol(Address);
		// Module grouping: use the resolver's module short name because the
		// live diagnostic path does not expose a base address here. Two
		// modules that share a base name inside one report collide by
		// design; the dialog and the docs state so.
		int bang = Frame.symbol.indexOf(QLatin1Char('!'));
		int plus = Frame.symbol.indexOf(QLatin1Char('+'));
		int split = (bang >= 0 && (plus < 0 || bang < plus)) ? bang : plus;
		if (split > 0)
			Frame.moduleKey = Frame.symbol.left(split);
		Capture.frames.append(Frame);
	}
	CStackExportDialog dlg(Capture, this);
	dlg.exec();
}