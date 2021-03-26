#pragma once
#include <qwidget.h>

#include "TreeViewEx.h"

#include "../mischelpers_global.h"

class MISCHELPERS_EXPORT COneColumnModel : public QIdentityProxyModel
{
	Q_OBJECT
public:
	COneColumnModel( QObject* parrent = 0) : QIdentityProxyModel(parrent) {}

	int	columnCount(const QModelIndex &parent = QModelIndex()) const { return 1; }
};

class MISCHELPERS_EXPORT CSplitTreeView : public QWidget
{
	Q_OBJECT
public:
	CSplitTreeView(QAbstractItemModel* pModel, QWidget *parent = 0);
	virtual ~CSplitTreeView();

	QTreeView*			GetView() { return m_pList; }
	QTreeView*			GetTree() { return m_pTree; }

	void				SetTree(bool bSet);
	bool				IsTree() const { return m_bTreeEnabled; }

	void				SetTreeWidth(int Width);
	int					GetTreeWidth() const;

	QModelIndex			currentIndex() const { return m_pList->currentIndex(); }
	QModelIndexList		selectedRows() const;

	QByteArray			saveState() const;
	bool				restoreState(const QByteArray &state);

	template<class T>
	void StartUpdatingWidgets(T& OldMap, T& Map)
	{
		for(typename T::iterator I = Map.begin(); I != Map.end();)
		{
			if(I.value().first == NULL)
				I = Map.erase(I);
			else
			{
				OldMap.insert(I.key(), I.value());
				I++;
			}
		}
	}

	template<class T>
	void EndUpdatingWidgets(T& OldMap, T& Map)
	{
		for(typename T::iterator I = OldMap.begin(); I != OldMap.end(); I++)
		{
			Map.remove(I.key());
			if(I.value().second.isValid())
				m_pList->setIndexWidget(I.value().second, NULL);
		}
	}

signals:
	void				TreeEnabled(bool bEnabled);

	void				MenuRequested(const QPoint &);

	void				clicked(const QModelIndex& Index);
	void				doubleClicked(const QModelIndex& Index);
	void				currentChanged(const QModelIndex &current, const QModelIndex &previous);
	void				selectionChanged(const QItemSelection& selected, const QItemSelection& seselected);

public slots:
	void				hideColumn(int column) { m_pList->hideColumn(column); }
    void				showColumn(int column) { m_pList->showColumn(column); }
	void				expand(const QModelIndex &index);
	void				collapse(const QModelIndex &index);
    void				resizeColumnToContents(int column) { m_pList->resizeColumnToContents(column); }
    void				sortByColumn(int column) { m_pList->sortByColumn(column); }
    void				expandAll() { m_pTree->expandAll(); }
    void				collapseAll() { m_pTree->collapseAll(); }
    void				expandToDepth(int depth) { m_pTree->expandToDepth(depth); }

private slots:
	void				OnSplitterMoved(int pos, int index);

	void				OnClickedTree(const QModelIndex& Index);
	void				OnDoubleClickedTree(const QModelIndex& Index);

	void				OnExpandTree(const QModelIndex& index);
	void				OnCollapseTree(const QModelIndex& index);

	void				OnTreeSelectionChanged(const QItemSelection& Selected, const QItemSelection& Deselected);
	void				OnListSelectionChanged(const QItemSelection& Selected, const QItemSelection& Deselected);

	void				OnTreeCurrentChanged(const QModelIndex &current, const QModelIndex &previous);
	void				OnListCurrentChanged(const QModelIndex &current, const QModelIndex &previous);

	void				OnTreeCustomSortByColumn(int column);
	void				OnListCustomSortByColumn(int column);

private:

	QHBoxLayout*			m_pMainLayout;

	bool					m_bTreeEnabled;
	QSplitter*				m_pSplitter;
	QTreeView*				m_pTree;
	QTreeView*				m_pList;
	QAbstractItemModel*		m_pModel;
	COneColumnModel*		m_pOneModel;

	int						m_LockSellection;
};
