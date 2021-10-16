#include "stdafx.h"
#include "SmartGridWidget.h"
#include <math.h>

CSmartGridWidget::CSmartGridWidget(QWidget* parent)
	: QWidget(parent)
{
	m_pMainLayout = new QGridLayout();
	this->setLayout(m_pMainLayout);

	m_pMainLayout->setMargin(1);
	m_pMainLayout->setSpacing(2);

	m_bReArangePending = false;
}

void CSmartGridWidget::SetBackground(const QColor& BackColor)
{
	QPalette pal = palette();
	pal.setColor(QPalette::Background, BackColor);
	this->setAutoFillBackground(true);
	this->setPalette(pal);
}

void CSmartGridWidget::AddWidget(QWidget* pWidget)
{
	m_Widgets.append(pWidget);
	if(!pWidget->parent())
		pWidget->setParent(this);

	if (!m_bReArangePending)
	{
		m_bReArangePending = true;
		QTimer::singleShot(0, this, SLOT(ReArange()));
	}
}

void CSmartGridWidget::ReArange()
{
	m_bReArangePending = false;

	int count = 0;
	while (count < m_Widgets.size())
	{
		if (m_Widgets[count] == NULL)
			m_Widgets.removeAt(count);
		else
			count++;
	}

	float columns = ceil(sqrt((float)count));
	float rows = columns > 0 ? ceil(count / columns) : 0;

	for (int row = 0; row < rows; row++)
	{
		for (int column = 0; column < columns; column++)
		{
			int index = row * columns + column;
			if (index >= count)
				break;

			m_pMainLayout->addWidget(m_Widgets.at(index), row, column);
		}
	}
}