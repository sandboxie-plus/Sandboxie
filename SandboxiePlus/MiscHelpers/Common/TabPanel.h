#pragma once
#include <qwidget.h>
#include "Settings.h"

#ifdef USE_QEXTWIDGETS
#include "../../qextwidgets/qtabwidgetex.h"
#endif

#include "../mischelpers_global.h"

class MISCHELPERS_EXPORT CTabPanel : public QWidget
{
	Q_OBJECT
public:
	CTabPanel(QWidget* parent = 0);
	virtual ~CTabPanel();

	virtual int			GetTabCount() { return m_AllTabs.size(); }
	virtual QString		GetTabLabel(int Index) { if (Index >= m_AllTabs.size()) return ""; return m_AllTabs[Index].Name; }
	virtual void		ShowTab(int Index, bool bShow);
	virtual bool		IsTabVisible(int Index) { if (Index >= m_AllTabs.size()) return false; return m_AllTabs[Index].bVisible; }

protected:
	virtual void		InitializeTabs() = 0;
	virtual int			AddTab(QWidget* pWidget, const QString& Name);
	virtual void		RebuildTabs(const int ActiveTab, const QStringList& VisibleTabs);
	virtual void		SaveTabs(int& ActiveTab, QStringList& VisibleTabs);

	QVBoxLayout*		m_pMainLayout;

#ifdef USE_QEXTWIDGETS
	QTabWidgetEx*		m_pTabs;
#else
	QTabWidget*			m_pTabs;
#endif
	struct STab
	{
		QString	Name;
		QWidget* pWidget;
		bool bVisible;
	};
	QVector<STab>		m_AllTabs;
};

