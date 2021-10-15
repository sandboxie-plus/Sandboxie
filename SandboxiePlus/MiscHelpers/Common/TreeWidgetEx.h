#pragma once

#include "../mischelpers_global.h"
#include "TreeViewEx.h"

class MISCHELPERS_EXPORT QTreeWidgetEx: public QTreeWidget
{
	Q_OBJECT
public:
	QTreeWidgetEx(QWidget *parent = 0) : QTreeWidget(parent) 
	{
		setUniformRowHeights(true);

		m_AutoFitMax = 0;
		m_ColumnReset = 1;

		header()->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(header(), SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(OnMenuRequested(const QPoint &)));

		m_pMenu = new QMenu(this);

        // Important: if something is shown/hidden we need a new size
		connect(this->model(), SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(OnExpandCollapsed()));
		connect(this->model(), SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(OnExpandCollapsed()));
		//connect(this->header(), SIGNAL(geometriesChanged()), this, SLOT(OnExpandCollapsed())); // fixme
        connect(this, SIGNAL(expanded(const QModelIndex &)), this, SLOT(OnExpandCollapsed()));
        connect(this, SIGNAL(collapsed(const QModelIndex &)), this, SLOT(OnExpandCollapsed()));
	}

	static void AddSubItem(QTreeWidgetItem* pRoot, const QString& Key, const QString& Value)
	{
		QTreeWidgetItem* pItem = new QTreeWidgetItem(QStringList(Key));
		pItem->setText(1, Value);
		pRoot->addChild(pItem);
	}

	// Use this function to make the widget autosize to show all items up to this value
	void setAutoFitMax(int AutoFitMax)
	{
		m_AutoFitMax = AutoFitMax;
	}

	void setColumnReset(int iMode)
	{
		m_ColumnReset = iMode;
	}

    QSize sizeHint() const {return m_AutoFitMax ? MySize() : QTreeWidget::sizeHint(); };
    QSize minimumSizeHint() const { return m_AutoFitMax ? MySize() : QTreeWidget::sizeHint(); };

signals:
	void ResetColumns();

public slots:
	void OnResetColumns()
	{
		for (int i = 0; i < columnCount(); i++) {
			setColumnHidden(i, false);
			QTreeView::resizeColumnToContents(i);
		}
	}

private slots:
	void OnMenuRequested(const QPoint &point)
	{
		if(m_Columns.isEmpty())
		{
			QTreeWidgetItem* pHeader = headerItem();
			for(int i=0; i < columnCount(); i++)
			{
				QAction* pAction = new QAction(pHeader->text(i), m_pMenu);
				pAction->setCheckable(true);
				connect(pAction, SIGNAL(triggered()), this, SLOT(OnMenu()));
				m_pMenu->addAction(pAction);
				m_Columns[pAction] = i;
			}

			if (m_ColumnReset)
			{
				m_pMenu->addSeparator();
				QAction* pAction = m_pMenu->addAction(QTreeViewEx::m_ResetColumns);
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
		setColumnHidden(Column, !pAction->isChecked());
		QTimer::singleShot(10, this, SLOT(OnExpandCollapsed()));
	}

	void OnExpandCollapsed() 
	{
		if (m_AutoFitMax) 
			updateGeometry();
	}

protected:
    QSize MySize() const
    {   //QSize tst(sizeHintForColumn(0) + 2 * frameWidth(), sizeHintForRow(0) + 2 * frameWidth());

        int neededHight= 2 * frameWidth()+ this->header()->height();

        QAbstractItemModel* m = this->model();

		QModelIndex root = this->rootIndex();
        //if(this->rootIsDecorated())
        neededHight += recursiveHeightHint(root,m);
        
		if (this->horizontalScrollBar()->isVisible())
			neededHight += this->horizontalScrollBar()->height();

		if (neededHight > m_AutoFitMax)
			neededHight = m_AutoFitMax;

        QSize temp = QTreeView::sizeHint();
        temp.setHeight(neededHight);
        return QSize(1,neededHight);
    }

    // we need the size of all visible items -> isExpanded
    // the root item is usually shown as a non-Valid index -> !i.isValid()
    int recursiveHeightHint(QModelIndex i,QAbstractItemModel* m) const
    {
        int temp=sizeHintForIndex(i).height() + 1;
        if(this->isExpanded(i) || !i.isValid())
        {
			if(m->hasChildren(i))
			{
				int numRows = m->rowCount(i);
				for(int count =0;count<numRows;count++)
					temp+=recursiveHeightHint(m->index(count,0,i),m);
			}
        }
        return temp;
    }

	QMenu*				m_pMenu;
	QMap<QAction*, int>	m_Columns;
	int					m_AutoFitMax;
	int					m_ColumnReset;
};
