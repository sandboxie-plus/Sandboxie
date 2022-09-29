#pragma once

#include <QStyledItemDelegate>

__inline uint qHash( const QVariant & var )
{
    if ( !var.isValid() /*|| var.isNull()*/ )
        //return -1;
        Q_ASSERT(0);

    switch ( var.type() )
    {
        case QVariant::Int:
                return qHash( var.toInt() );
            break;
        case QVariant::UInt:
                return qHash( var.toUInt() );
            break;
        case QVariant::Bool:
                return qHash( var.toUInt() );
            break;
        case QVariant::Double:
                return qHash( var.toUInt() );
            break;
        case QVariant::LongLong:
                return qHash( var.toLongLong() );
            break;
        case QVariant::ULongLong:
                return qHash( var.toULongLong() );
            break;
        case QVariant::String:
                return qHash( var.toString() );
            break;
        case QVariant::Char:
                return qHash( var.toChar() );
            break;
        case QVariant::StringList:
                return qHash( var.toString() );
            break;
        case QVariant::ByteArray:
                return qHash( var.toByteArray() );
            break;
        case QVariant::Date:
        case QVariant::Time:
        case QVariant::DateTime:
        case QVariant::Url:
        case QVariant::Locale:
        //case QVariant::RegExp:
        //        return qHash( var.toString() );
            break;
        case QVariant::Map:
        case QVariant::List:
        case QVariant::BitArray:
        case QVariant::Size:
        case QVariant::SizeF:
        case QVariant::Rect:
        case QVariant::LineF:
        case QVariant::Line:
        case QVariant::RectF:
        case QVariant::Point:
        case QVariant::PointF:
            // not supported yet
            break;
        case QVariant::UserType:
        case QVariant::Invalid:
        default:
            return -1;
    }

    // could not generate a hash for the given variant
    return -1;
}

#include "../mischelpers_global.h"

class MISCHELPERS_EXPORT QAbstractItemModelEx : public QAbstractItemModel
{
    Q_OBJECT

public:
	QAbstractItemModelEx(QObject *parent = 0) : QAbstractItemModel(parent) {}
	virtual ~QAbstractItemModelEx() {}

	bool IsColumnEnabled(int column)
	{
		return m_Columns.contains(column);
	}

	void SetColumnEnabled(int column, bool set)
	{
		if (!set)
			m_Columns.remove(column);
		else
			m_Columns.insert(column);
	}

protected:

	QSet<int>				m_Columns;
};

class MISCHELPERS_EXPORT QTreeViewEx: public QTreeView
{
	Q_OBJECT
public:
	QTreeViewEx(QWidget *parent = 0) : QTreeView(parent) 
	{
		setUniformRowHeights(true);

		m_ColumnReset = 1;

		header()->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(header(), SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(OnMenuRequested(const QPoint &)));

		m_pMenu = new QMenu(this);
	}

	void setColumnReset(int iMode)
	{
		m_ColumnReset = iMode;
	}

	void setColumnFixed(int column, bool fixed) 
	{
		if (fixed)
			m_FixedColumns.insert(column);
		else
			m_FixedColumns.remove(column);
	}

	bool isColumnFixed(int column) const
	{
		return m_FixedColumns.contains(column);
	}

	QModelIndexList selectedRows() const
	{
		int Column = 0;
		QAbstractItemModel* pModel = model();
		for (int i = 0; i < pModel->columnCount(); i++)
		{
			if (!isColumnHidden(i))
			{
				Column = i;
				break;
			}
		}

		QModelIndexList IndexList;
		foreach(const QModelIndex& Index, selectedIndexes())
		{
			if (Index.column() == Column)
				IndexList.append(Index);
		}
		return IndexList;
	}

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
				setIndexWidget(I.value().second, NULL);
		}
	}

	bool restoreState(const QByteArray &state)
	{
		bool bRet = header()->restoreState(state);
		
		SyncColumnsWithModel();

		return bRet;
	}

	QByteArray saveState() const
	{
		return header()->saveState();
	}

	QAbstractItemModelEx* modelEx() const
	{
		QAbstractItemModelEx* pModel = qobject_cast<QAbstractItemModelEx*>(model());
		if (!pModel)
		{
			QSortFilterProxyModel* pProxyModel = qobject_cast<QSortFilterProxyModel*>(model());
			if(pProxyModel)
				pModel = qobject_cast<QAbstractItemModelEx*>(pProxyModel->sourceModel());
		}
		return pModel;
	}

	void SetColumnHidden(int column, bool hide, bool fixed = false)
	{
		if (!fixed && isColumnFixed(column))
			return; // can not change fixed columns

		setColumnHidden(column, hide);

		if(QAbstractItemModelEx* pModel = modelEx())
			pModel->SetColumnEnabled(column, !hide);

		if (fixed)
			setColumnFixed(column, true);

		emit ColumnChanged(column, !hide);
	}

	static QString m_ResetColumns;

signals:
	void ColumnChanged(int column, bool visible);
	void ResetColumns();

public slots:
	void SyncColumnsWithModel()
	{
		if(QAbstractItemModelEx* pModel = modelEx())
		{
			for (int i = 0; i < pModel->columnCount(); i++)
				pModel->SetColumnEnabled(i, !isColumnHidden(i));
		}
	}

	void OnResetColumns()
	{
		QAbstractItemModel* pModel = model();
		for (int i = 0; i < pModel->columnCount(); i++) {
			SetColumnHidden(i, false);
			QTreeView::resizeColumnToContents(i);
		}
	}

private slots:
	void OnMenuRequested(const QPoint &point)
	{
		QAbstractItemModel* pModel = model();

		if(m_Columns.isEmpty())
		{
			for(int i=0; i < pModel->columnCount(); i++)
			{
				QString Label = pModel->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();
				if(Label.isEmpty() || m_FixedColumns.contains(i))
					continue;
				QAction* pAction = new QAction(Label, m_pMenu);
				pAction->setCheckable(true);
				pAction->setChecked(!isColumnHidden(i));
				connect(pAction, SIGNAL(triggered()), this, SLOT(OnMenu()));
				m_pMenu->addAction(pAction);
				m_Columns[pAction] = i;
			}

			if (m_ColumnReset)
			{
				m_pMenu->addSeparator();
				QAction* pAction = m_pMenu->addAction(m_ResetColumns);
				if(m_ColumnReset == 1)
					connect(pAction, SIGNAL(triggered()), this, SLOT(OnResetColumns()));
				else
					connect(pAction, SIGNAL(triggered()), this, SIGNAL(ResetColumns()));
			}
		}

		for(QMap<QAction*, int>::iterator I = m_Columns.begin(); I != m_Columns.end(); I++)
			I.key()->setChecked(!isColumnHidden(I.value()));

		m_pMenu->popup(QCursor::pos());	
	}

	void OnMenu()
	{
		QAction* pAction = (QAction*)sender();
		int Column = m_Columns.value(pAction, -1);
		SetColumnHidden(Column, !pAction->isChecked());
	}

protected:
	QMenu*				m_pMenu;
	QMap<QAction*, int>	m_Columns;
	QSet<int>			m_FixedColumns;
	int					m_ColumnReset;
};

class MISCHELPERS_EXPORT QStyledItemDelegateMaxH : public QStyledItemDelegate
{
    Q_OBJECT
public:
    QStyledItemDelegateMaxH(int MaxHeight, QObject *parent = 0)
        : QStyledItemDelegate(parent) {m_MaxHeight = MaxHeight;}
	
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		QSize size = QStyledItemDelegate::sizeHint(option, index);
		size.setHeight(m_MaxHeight);
		return size;
	}

	int m_MaxHeight;
};

class MISCHELPERS_EXPORT CStyledGridItemDelegate : public QStyledItemDelegateMaxH
{
public:
	explicit CStyledGridItemDelegate(int MaxHeight, QObject * parent = 0) : CStyledGridItemDelegate(MaxHeight, false, parent) { }
	explicit CStyledGridItemDelegate(int MaxHeight, QColor Color, QObject * parent = 0) : CStyledGridItemDelegate(MaxHeight, Color, false, parent) { }
	explicit CStyledGridItemDelegate(int MaxHeight, bool Tree, QObject * parent = 0) : CStyledGridItemDelegate(MaxHeight, QColor(Qt::darkGray), false, parent) { }
	explicit CStyledGridItemDelegate(int MaxHeight, QColor Color, bool Tree, QObject * parent = 0) : QStyledItemDelegateMaxH(MaxHeight, parent) { 
		m_Color = Color;  
		m_Tree = Tree; 
		m_Grid = true;
	}
 
	void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
    {
		QStyledItemDelegate::paint(painter, option, index);

		if (m_Grid)
		{
			painter->save();
			painter->setPen(m_Color);
			//painter->drawRect(option.rect);
			//painter->drawLine(option.rect.left(), option.rect.top(), option.rect.right(), option.rect.top());
			painter->drawLine(option.rect.right(), option.rect.top(), option.rect.right(), option.rect.bottom());
			painter->drawLine(option.rect.left() + (m_Tree && index.column() == 0 ? 24 : 0), option.rect.bottom(), option.rect.right(), option.rect.bottom());
			painter->restore();
		}
    }

	bool m_Grid;
	bool m_Tree;
	QColor m_Color;
};