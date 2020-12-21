#include "stdafx.h"
#include "TabPanel.h"

CTabPanel::CTabPanel(QWidget* parent)
	: QWidget(parent)
{
	m_pMainLayout = new QVBoxLayout();
	m_pMainLayout->setMargin(0);
	this->setLayout(m_pMainLayout);

#ifdef USE_QEXTWIDGETS
	m_pTabs = new QTabWidgetEx();
	m_pTabs->setMultiRow(true);
#else
	m_pTabs = new QTabWidget();
#endif
	m_pMainLayout->addWidget(m_pTabs);
}

CTabPanel::~CTabPanel()
{
}

void CTabPanel::ShowTab(int Index, bool bShow)
{
	if (Index >= m_AllTabs.size())
		return;
	
	m_AllTabs[Index].bVisible = bShow;

	int ActiveTab = 0;
	QStringList VisibleTabs;

	SaveTabs(ActiveTab, VisibleTabs);

	RebuildTabs(ActiveTab, VisibleTabs);
}

void CTabPanel::AddTab(QWidget* pWidget, const QString& Name)
{
	STab Tab{Name, pWidget, true};
	m_AllTabs.append(Tab);
	m_pTabs->addTab(Tab.pWidget, Tab.Name);
}

void CTabPanel::SaveTabs(int& ActiveTab, QStringList& VisibleTabs)
{
	ActiveTab = 0;
	VisibleTabs.clear();
	for(int i=0; i < m_AllTabs.size(); i++)
	{
		STab& Tab = m_AllTabs[i];

		VisibleTabs.append(QString::number(Tab.bVisible));
		if (m_pTabs->currentWidget() == Tab.pWidget)
			ActiveTab = i;
	}
}

void CTabPanel::RebuildTabs(const int ActiveTab, const QStringList& VisibleTabs)
{
	m_pTabs->clear();
	for(int i=0; i < m_AllTabs.size(); i++)
	{
		STab& Tab = m_AllTabs[i];

		if (VisibleTabs.size() <= i || VisibleTabs[i].toInt() != 0)
		{
			Tab.bVisible = true;

			m_pTabs->addTab(Tab.pWidget, Tab.Name);
			if (i == ActiveTab)
				m_pTabs->setCurrentWidget(Tab.pWidget);
		}
	}
}
