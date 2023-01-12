#pragma once

#include "../mischelpers_global.h"
#include <QSortFilterProxyModel>
#include <QTreeView>
#include "Finder.h"

class MISCHELPERS_EXPORT CSortFilterProxyModel: public QSortFilterProxyModel
{
	Q_OBJECT

public:
	CSortFilterProxyModel(QObject* parent = 0) : QSortFilterProxyModel(parent) 
	{
		m_bHighLight = false;
		m_iColumn = 0;

		this->setSortCaseSensitivity(Qt::CaseInsensitive);
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

		// default behaviour
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
	void SetFilter(const QString& Exp, int iOptions, int Col = -1) // -1 = any
	{
		QString ExpStr = ((iOptions & CFinder::eRegExp) == 0) ? Exp : (".*" + QRegularExpression::escape(Exp) + ".*");
		QRegularExpression RegExp(ExpStr, (iOptions & CFinder::eCaseSens) != 0 ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);

		QModelIndex idx;
		m_iColumn = Col;
		m_bHighLight = (iOptions & CFinder::eHighLight) != 0;
		setFilterKeyColumn(Col); 
		setFilterRegularExpression(RegExp);
		if (m_bHighLight)
			emit layoutChanged();
	}

protected:
	bool		m_bHighLight;
	int			m_iColumn;
};

