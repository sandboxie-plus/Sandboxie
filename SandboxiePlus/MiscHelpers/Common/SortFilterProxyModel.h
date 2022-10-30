#pragma once

#include "../mischelpers_global.h"
#include <QSortFilterProxyModel>
#include <QTreeView>
#include "Finder.h"

class MISCHELPERS_EXPORT CSortFilterProxyModel: public QSortFilterProxyModel
{
	Q_OBJECT

public:
	CSortFilterProxyModel(QObject* parrent = 0) : QSortFilterProxyModel(parrent) 
	{
		m_bHighLight = false;
		m_iColumn = 0;
		m_pView = NULL;

		this->setSortCaseSensitivity(Qt::CaseInsensitive);
	}

	void setView(QTreeView* pView)
	{
		m_pView = pView;
	}

	bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const
	{
		if (m_bHighLight)
			return true;

		// allow the item to pass if any of the child items pass
		if(filterRegularExpression().isValid())
		{
			// get source-model index for current row
			QModelIndex source_index = sourceModel()->index(source_row, 0, source_parent);
			if(source_index.isValid())
			{
				// if any of children matches the filter, then current index matches the filter as well
				int nb = sourceModel()->rowCount(source_index);
				for(int i = 0; i < nb; i++)
				{
					if(filterAcceptsRow(i, source_index))
						return true;
				}
				// check current index itself
				return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
			}
		}

		// default behavioure
		return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
	}

	QVariant data(const QModelIndex &index, int role) const
	{
		QVariant Data = QSortFilterProxyModel::data(index, role);
		if (m_bHighLight && role == (CFinder::GetDarkMode() ? Qt::ForegroundRole : Qt::BackgroundRole))
		{
			if (filterRegularExpression().isValid())
			{
				QString Key = QSortFilterProxyModel::data(index, filterRole()).toString();
				if (Key.contains(filterRegularExpression()))
					return QColor(Qt::yellow);
			}
			//return QColor(Qt::white);
		}

		//if (role == Qt::BackgroundRole)
		//{
		//	if (m_bAlternate && !Data.isValid())
		//	{
		//		if (0 == index.row() % 2)
		//			return QColor(226, 237, 253);
		//		else
		//			return QColor(Qt::white);
		//	}
		//}
		return Data;
	}

public slots:
	void SetFilter(const QRegularExpression& Exp, bool bHighLight = false, int Col = -1) // -1 = any
	{
		QModelIndex idx;
		//if (m_pView) idx = m_pView->currentIndex();
		m_iColumn = Col;
		m_bHighLight = bHighLight;
		setFilterKeyColumn(Col); 
		setFilterRegularExpression(Exp);
		//if (m_pView) m_pView->setCurrentIndex(idx);
		if (m_bHighLight)
			emit layoutChanged();
	}

	void SelectNext()
	{
		if (!m_pView)
			return;

		bool next = true;
		QModelIndex idx = m_pView->currentIndex();
		if (!(next = idx.isValid()))
			idx = index(0, 0);

		//if (QApplication::keyboardModifiers() & Qt::ControlModifier)
		if (QApplication::keyboardModifiers() & Qt::ShiftModifier)
			idx = FindPrev(idx, next);
		else
			idx = FindNext(idx, next);

		if (idx.isValid())
			m_pView->setCurrentIndex(idx);
		else
			QApplication::beep();
	}

protected:
	bool		m_bHighLight;
	int			m_iColumn;
	QTreeView*	m_pView;

	bool		MatchCell(QModelIndex idx, int column)
	{
		QModelIndex tmp = idx.sibling(idx.row(), column);

		QString str = data(tmp, filterRole()).toString();
		if (str.contains(filterRegularExpression()))
			return true;
		return false;
	}

	bool		MatchRow(QModelIndex idx)
	{
		if (m_iColumn != -1)
			return MatchCell(idx, m_iColumn);

		for(int col = 0; col < columnCount(idx); col++) {
			if (MatchCell(idx, col))
				return true;
		}
		return false;
	}

	QModelIndex	FindNext(QModelIndex idx, bool next = false)
	{
		if (MatchRow(idx) && !next)
			return idx;

		if (hasChildren(idx))
		{
			int numRows = rowCount(idx);
			for (int count = 0; count < numRows; count++) {
				QModelIndex tmp = FindNext(index(count, 0, idx));
				if (tmp.isValid())
					return tmp;
			}
		}

		do {
			QModelIndex par = parent(idx);

			int numRows = rowCount(par);
			for (int count = idx.row() + 1; count < numRows; count++) {
				QModelIndex tmp = FindNext(index(count, 0, par));
				if (tmp.isValid())
					return tmp;
			}

			idx = par;
		} while (idx.isValid());

		return QModelIndex();
	}

	QModelIndex	FindPrev(QModelIndex idx, bool next = false)
	{
		if (MatchRow(idx) && !next)
			return idx;

		if (hasChildren(idx))
		{
			int numRows = rowCount(idx);
			for (int count = numRows-1; count >= 0; count++) {
				QModelIndex tmp = FindNext(index(count, 0, idx));
				if (tmp.isValid())
					return tmp;
			}
		}

		do {
			QModelIndex par = parent(idx);

			int numRows = rowCount(par);
			for (int count = idx.row() - 1; count >= 0; count--) {
				QModelIndex tmp = FindNext(index(count, 0, par));
				if (tmp.isValid())
					return tmp;
			}

			idx = par;
		} while (idx.isValid());

		return QModelIndex();
	}
};