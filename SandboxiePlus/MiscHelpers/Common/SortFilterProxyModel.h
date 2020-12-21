#pragma once

#include "../mischelpers_global.h"

class MISCHELPERS_EXPORT CSortFilterProxyModel: public QSortFilterProxyModel
{
	Q_OBJECT

public:
	CSortFilterProxyModel(bool bAlternate, QObject* parrent = 0) : QSortFilterProxyModel(parrent) 
	{
		m_bAlternate = bAlternate;
		m_bHighLight = false;
	}

	bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const
	{
		if (m_bHighLight)
			return true;

		// allow the item to pass if any of the child items pass
		if(!filterRegExp().isEmpty())
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
		if (role == Qt::BackgroundRole)
		{
			if (m_bHighLight)
			{
				if (!filterRegExp().isEmpty())
				{
					QString Key = QSortFilterProxyModel::data(index, filterRole()).toString();
					if (Key.contains(filterRegExp()))
						return QColor(Qt::yellow);
				}
				return QColor(Qt::white);
			}

			if (m_bAlternate && !Data.isValid())
			{
				if (0 == index.row() % 2)
					return QColor(226, 237, 253);
				else
					return QColor(Qt::white);
			}
		}
		return Data;
	}

public slots:
	void SetFilter(const QRegExp& Exp, bool bHighLight = false, int Col = -1) // -1 = any
	{
		m_bHighLight = bHighLight;
		setFilterKeyColumn(Col); 
		setFilterRegExp(Exp);
	}

protected:
	bool		m_bAlternate;
	bool		m_bHighLight;
};