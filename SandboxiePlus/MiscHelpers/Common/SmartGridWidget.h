#pragma once
#include <qwidget.h>

#include "../mischelpers_global.h"

class MISCHELPERS_EXPORT CSmartGridWidget : public QWidget
{
	Q_OBJECT

public:
	CSmartGridWidget(QWidget* parent = NULL);
	virtual ~CSmartGridWidget() {}

	virtual void			SetBackground(const QColor& BackColor);

	virtual void			AddWidget(QWidget* pWidget);

	virtual int				GetCount()				{ return m_Widgets.count(); }
	virtual QWidget*		GetWidget(int Index)	{ return m_Widgets.at(Index); }

public slots:
	virtual void			ReArange();

protected:
	QGridLayout*			m_pMainLayout;

	QList<QPointer<QWidget>>m_Widgets;

	bool					m_bReArangePending;
};
