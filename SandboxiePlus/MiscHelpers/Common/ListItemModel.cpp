#include "stdafx.h"
#include "ListItemModel.h"

#define FIRST_COLUMN 0

bool CListItemModel::m_DarkMode = false;

CListItemModel::CListItemModel(QObject *parent)
: QAbstractItemModelEx(parent)
{
	m_bUseIcons = false;
}

CListItemModel::~CListItemModel()
{
	foreach(SListNode* pNode, m_List)
		delete pNode;
}

int CListItemModel::GetRow(SListNode* pNode) const
{
	int Index = m_RevList.value(pNode, -1);
	ASSERT(Index != -1);
	ASSERT(m_List[Index] == pNode); 
	return Index;
}

void CSimpleListModel::Sync(QList<QVariantMap> List)
{
	QList<SListNode*> New;
	QHash<QVariant, SListNode*> Old = m_Map;

	foreach (const QVariantMap& Cur, List)
	{
		QVariant ID = Cur["ID"];

		int Row = -1;
		SListNode* pNode = static_cast<SListNode*>(Old[ID]);
		if(!pNode)
		{
			pNode = static_cast<SListNode*>(MkNode(ID));
			pNode->Values.resize(columnCount());
			pNode->IsBold = Cur["IsBold"].toBool();
			pNode->Icon = Cur["Icon"];
			New.append(pNode);
		}
		else
		{
			Old[ID] = NULL;
			Row = GetRow(pNode);
		}

		int Col = 0;
		bool State = false;
		bool Changed = false;

		QVariantMap Values = Cur["Values"].toMap();
		for(int section = FIRST_COLUMN; section < columnCount(); section++)
		{
			if (!IsColumnEnabled(section))
				continue; // ignore columns which are hidden

			QVariant Value = Values[QString::number(section)];

			SListNode::SValue& ColValue = pNode->Values[section];

			if (ColValue.Raw != Value)
			{
				Changed = true;
				ColValue.Raw = Value;

				//ColValue.Formatted = 
			}

			if(State != Changed)
			{
				if(State && Row != -1)
					emit dataChanged(createIndex(Row, Col), createIndex(Row, section-1));
				State = Changed;
				Col = section;
			}
			Changed = false;
		}
		if(State && Row != -1)
			emit dataChanged(createIndex(Row, Col, pNode), createIndex(Row, columnCount()-1, pNode));
	}

	CListItemModel::Sync(New, Old);
}

void CListItemModel::Sync(QList<SListNode*>& New, QHash<QVariant, SListNode*>& Old)
{
	int Removed = 0;

	int Begin = -1;
	int End = -1;
	for(int i = m_List.count()-1; i >= -1; i--) 
	{
		QVariant ID = i >= 0 ? m_List[i]->ID : QVariant();
		if(!ID.isNull() && (Old.value(ID) != NULL)) // remove it
		{
			m_Map.remove(ID);
			if(End == -1)
				End = i;
		}
		else if(End != -1) // keep it and remove whatis to be removed at once
		{
			Begin = i + 1;

			beginRemoveRows(QModelIndex(), Begin, End);
			for (int j = End; j >= Begin; j--)
			{
				Removed++;

				SListNode* pNode = m_List.takeAt(j);
				m_RevList.remove(pNode);
				delete pNode;
			}
			endRemoveRows();

			End = -1;
			Begin = -1;
		}
    }

	// if something was removed we need to update the row cache
	if (Removed > 0)
	{
		ASSERT(m_RevList.size() == m_List.size());
		for (int i = 0; i < m_List.count(); i++)
			m_RevList[m_List[i]] = i;
	}

	Begin = m_List.count();
	for(QList<SListNode*>::iterator I = New.begin(); I != New.end(); I++)
	{
		SListNode* pNode = *I;
		m_Map.insert(pNode->ID, pNode);
		
		int Index = m_List.size();
		m_List.append(pNode);
		m_RevList.insert(pNode, Index);
	}
	End = m_List.count();
	if(Begin < End)
	{
		beginInsertRows(QModelIndex(), Begin, End-1);
		endInsertRows();
	}
}

QModelIndex CListItemModel::FindIndex(const QVariant& ID)
{
	if(SListNode* pNode = m_Map.value(ID))
	{
		int row = m_List.indexOf(pNode);
		ASSERT(row != -1);
		return createIndex(row, FIRST_COLUMN, pNode);
	}
	return QModelIndex();
}

void CListItemModel::Clear()
{
	//beginResetModel();
	if (rowCount() == 0)
		return;
	beginRemoveRows(QModelIndex(), 0, rowCount()-1);
	foreach(SListNode* pNode, m_List)
		delete pNode;
	m_List.clear();
	m_RevList.clear();
	m_Map.clear();
	endRemoveRows();
	//endResetModel();
}

QVariant CListItemModel::data(const QModelIndex &index, int role) const
{
    return Data(index, role, index.column());
}

QVariant CListItemModel::Data(const QModelIndex &index, int role, int section) const
{
	if (!index.isValid())
        return QVariant();

    //if(role == Qt::SizeHintRole )
    //    return QSize(64,16); // for fixing height

	SListNode* pNode = static_cast<SListNode*>(index.internalPointer());

	switch(role)
	{
		case Qt::DisplayRole:
		{
			SListNode::SValue& Value = pNode->Values[section];
			return Value.Formatted.isValid() ? Value.Formatted : Value.Raw;
		}
		case Qt::EditRole: // sort role
		{
			return pNode->Values[section].Raw;
		}
		case Qt::ToolTipRole:
		{
			QString ToolTip;
			emit ToolTipCallback(pNode->ID, ToolTip);
			if(!ToolTip.isNull())
				return ToolTip;
			break;
		}
		case Qt::DecorationRole:
		{
			if (m_bUseIcons && section == FIRST_COLUMN)
				return pNode->Icon.isValid() ? pNode->Icon : GetDefaultIcon();
			break;
		}
		case Qt::FontRole:
		{
			if (section == FIRST_COLUMN && pNode->IsBold)
			{
				QFont fnt;
				fnt.setBold(true);
				return fnt;
			}
			break;
		}
		case Qt::BackgroundRole:
		{
			if(!m_DarkMode)
				return pNode->Color.isValid() ? pNode->Color : QVariant();
			break;
		}
		case Qt::ForegroundRole:
		{
			if(m_DarkMode)
				return pNode->Color.isValid() ? pNode->Color : QVariant();
			else if (pNode->IsGray)
			{
				QColor Color = Qt::darkGray;
				return QBrush(Color);
			}
			break;
		}

		case Qt::UserRole:
		{
			switch(section)
			{
				case FIRST_COLUMN:			return pNode->ID;
			}
			break;
		}
	}
	return QVariant();
}

Qt::ItemFlags CListItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex CListItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if (parent.isValid()) 
        return QModelIndex();
	if(m_List.count() > row)
		return createIndex(row, column, m_List[row]);
	return QModelIndex();
}

QModelIndex CListItemModel::parent(const QModelIndex &index) const
{
	return QModelIndex();
}

int CListItemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    if (parent.isValid())
        return 0;
	return m_List.count();
}

int CSimpleListModel::columnCount(const QModelIndex &parent) const
{
	return m_Headers.count();
}

QVariant CSimpleListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		if (section < m_Headers.size())
			return m_Headers.at(section);
	}
    return QVariant();
}