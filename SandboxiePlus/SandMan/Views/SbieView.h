#pragma once

#include "../../MiscHelpers/Common/PanelView.h"
#include "../../MiscHelpers/Common/TreeviewEx.h"
#include "../Models/SbieModel.h"

class CSbieView : public CPanelView
{
	Q_OBJECT
public:
	CSbieView(QWidget* parent = 0);
	virtual ~CSbieView();

	virtual QTreeViewEx*		GetTree() { return m_pSbieTree; }

	virtual QList<CSandBoxPtr>	GetSelectedBoxes();
	virtual QList<CBoxedProcessPtr>	GetSelectedProcesses();

	//virtual void				UpdateRunMenu();

	virtual QString				AddNewBox();
	virtual void				SelectBox(const QString& Name);

signals:
	void						RecoveryRequested(const QString& BoxName);

public slots:
	void						Clear();
	void						Refresh();
	void						StoreGroups();
	void						ReloadGroups();

private slots:
	void						OnToolTipCallback(const QVariant& ID, QString& ToolTip);

	void						OnDoubleClicked(const QModelIndex& index);
	void						ProcessSelection(const QItemSelection& selected, const QItemSelection& deselected);

	void						OnGroupAction();
	void						OnSandBoxAction();
	void						OnSandBoxAction(QAction* pAction);
	void						OnProcessAction();

protected:
	virtual void				OnMenu(const QPoint& Point);
	virtual QTreeView*			GetView() { return m_pSbieTree; }
	virtual QAbstractItemModel* GetModel() { return m_pSortProxy; }

	virtual void				UpdateRunMenu(const CSandBoxPtr& pBox);

	QMap<QString, QStringList>	m_Groups;

private:

	void					UpdateGroupMenu();

	QString					FindParent(const QString& Name);
	bool					IsParentOf(const QString& Name, const QString& Group);

	QVBoxLayout*			m_pMainLayout;

	QTreeViewEx*			m_pSbieTree;
	CSbieModel*				m_pSbieModel;
	QSortFilterProxyModel*	m_pSortProxy;

	QAction*				m_pNewBox;
	QAction*				m_pAddGroupe;
	QAction*				m_pDelGroupe;
	int						m_iMenuTop;
	QMenu*					m_pMenuRun;
	QAction*				m_pMenuRunAny;
	QAction*				m_pMenuRunMenu;
	QAction*				m_pMenuRunBrowser;
	QAction*				m_pMenuRunMailer;
	QAction*				m_pMenuRunExplorer;
	QAction*				m_pMenuRunCmd;
	QAction*				m_pMenuRunCmdAdmin;
	QAction*				m_pMenuMkLink;
	QMenu*					m_pMenuPresets;
	QActionGroup*			m_pMenuPresetsAdmin;
	QAction*				m_pMenuPresetsShowUAC;
	QAction*				m_pMenuPresetsNoAdmin;
	QAction*				m_pMenuPresetsFakeAdmin;
	QAction*				m_pMenuPresetsINet;
	QAction*				m_pMenuPresetsShares;
	QAction*				m_pMenuOptions;
	QAction*				m_pMenuSnapshots;
	QAction*				m_pMenuEmptyBox;
	QAction*				m_pMenuExplore;
	QAction*				m_pMenuRecover;
	QAction*				m_pMenuCleanUp;
	QAction*				m_pMenuRemove;
	QMenu*					m_pMenuMoveTo;
	int						m_iMoveTo;
	QAction*				m_pMenuRename;
	int						m_iMenuBox;

	QAction*				m_pMenuTerminate;
	QAction*				m_pMenuLinkTo;
	QMenu*					m_pMenuPreset;
	QAction*				m_pMenuBlackList;
	QAction*				m_pMenuAllowInternet;
	QAction*				m_pMenuMarkForced;
	QAction*				m_pMenuMarkLinger;
	QAction*				m_pMenuMarkLeader;
	QAction*				m_pMenuPinToRun;
	//QAction*				m_pMenuSuspend;
	//QAction*				m_pMenuResume;
	int						m_iMenuProc;

	int						m_iMenuRun;
};