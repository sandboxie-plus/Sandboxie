#pragma once

#include <QWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QMap>
#include <QLabel>
#include <QLineEdit>
#include <QToolBar>
#include <QAction>
#include <QMenu>

class CResourceView : public QWidget
{
	Q_OBJECT
public:
	CResourceView(QWidget* parent = 0);
	~CResourceView();

public slots:
	void		Refresh();
	void		Clear();

protected:
	void		timerEvent(QTimerEvent* pEvent);
	int			m_uTimerID;

private slots:
	void		OnTerminateProcess();
	void		OnTerminateBox();
	void		OnExpandAll();
	void		OnCollapseAll();
	void		OnFilterChanged(const QString& text);
	void		OnContextMenu(const QPoint& pos);

private:

	struct SProcessStats
	{
		quint32 ProcessId;
		QString ProcessName;
		QString BoxName;
		quint64 WorkingSetSize;		// bytes
		quint64 PrivateBytes;		// bytes
		quint64 KernelTime;			// 100-ns intervals
		quint64 UserTime;			// 100-ns intervals
		double  CpuUsage;			// 0..100
	};

	struct SBoxStats
	{
		QString BoxName;
		int		ProcessCount;
		quint64 TotalWorkingSet;
		quint64 TotalPrivateBytes;
		double	TotalCpuUsage;
		quint64 DiskSize;
		QList<SProcessStats> Processes;
	};

	void		UpdateProcessStats(SProcessStats& Stats);
	double		CalcCpuUsage(quint32 pid, quint64 kernelTime, quint64 userTime);
	bool		MatchesFilter(const QString& boxName, const QList<SProcessStats>& processes) const;

	QMap<quint32, QPair<quint64, quint64>> m_LastTimes; // pid -> (kernel+user, timestamp)

	// Toolbar
	QToolBar*		m_pToolBar;
	QAction*		m_pRefreshAction;
	QAction*		m_pTerminateAction;
	QAction*		m_pTerminateBoxAction;
	QAction*		m_pExpandAllAction;
	QAction*		m_pCollapseAllAction;

	// Filter
	QLineEdit*		m_pFilterEdit;

	// Summary bar
	QLabel*			m_pSummaryLabel;

	// Context menu
	QMenu*			m_pContextMenu;

	QVBoxLayout*	m_pMainLayout;
	QTreeWidget*	m_pTree;
};
