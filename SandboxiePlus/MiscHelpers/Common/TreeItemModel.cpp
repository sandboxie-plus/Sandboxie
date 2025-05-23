#include "stdafx.h"
#include "TreeItemModel.h"

QString QTreeViewEx::m_ResetColumns = "Reset Columns";

#define FIRST_COLUMN 0

bool CTreeItemModel::m_DarkMode = false;

CTreeItemModel::CTreeItemModel(QObject *parent)
: QAbstractItemModelEx(parent)
{
	m_bUseIcons = false;
	m_Root = NULL;
}

CTreeItemModel::~CTreeItemModel()
{
	Q_ASSERT(m_Root == NULL);
}

CSimpleTreeModel::CSimpleTreeModel(QObject *parent) 
 : CTreeItemModel(parent) 
{
	m_bTree = true;
	m_Root = MkNode(QVariant());
}

CSimpleTreeModel::~CSimpleTreeModel()
{
	FreeNode(m_Root);
	m_Root = NULL;
}

QList<QVariant> CSimpleTreeModel::MakePath(const QVariantMap& Cur, const QMap<QVariant, QVariantMap>& List)
{
	QVariant ParentID = Cur["ParentID"];
	QVariantMap Parent = List.value(ParentID);

	QList<QVariant> Path;
	if (!Parent.isEmpty() && ParentID != Cur["ID"])
	{
		Path = MakePath(Parent, List);
		Path.append(ParentID);
	}
	return Path;
}

bool CSimpleTreeModel::TestPath(const QList<QVariant>& Path, const QVariantMap& Cur, const QMap<QVariant, QVariantMap>& List, int Index)
{
	QVariant ParentID = Cur["ParentID"];
	QVariantMap Parent = List.value(ParentID);

	if (!Parent.isEmpty())
	{
		if(Index >= Path.size() || Path[Path.size() - Index - 1] != ParentID)
			return false;

		return TestPath(Path, Parent, List, Index + 1);
	}

	return Path.size() == Index;
}

void CSimpleTreeModel::Sync(const QMap<QVariant, QVariantMap>& List)
{
	QMap<QList<QVariant>, QList<STreeNode*> > New;
	QHash<QVariant, STreeNode*> Old = m_Map;

	foreach (const QVariantMap& Cur, List)
	{
		QVariant ID = Cur["ID"];

		QModelIndex Index;
		
		STreeNode* pNode = static_cast<STreeNode*>(Old.value(ID));
		if(!pNode || (m_bTree ? !TestPath(pNode->Path, Cur, List) : !pNode->Path.isEmpty()))
		{
			pNode = static_cast<STreeNode*>(MkNode(ID));
			pNode->Values.resize(columnCount());
			if(m_bTree)
				pNode->Path = MakePath(Cur, List);
			pNode->Icon = Cur["Icon"];
			New[pNode->Path].append(pNode);
		}
		else
		{
			Old[ID] = NULL;
			Index = Find(m_Root, pNode);
		}

		//if(Index.isValid()) // this is to slow, be more precise
		//	emit dataChanged(createIndex(Index.row(), 0, pNode), createIndex(Index.row(), columnCount()-1, pNode));

		int Col = 0;
		bool State = false;
		bool Changed = false;

		if (pNode->IsBold != Cur["IsBold"].toBool()) {
			pNode->IsBold = Cur["IsBold"].toBool();
			Changed = true;
		}

		QVariantMap Values = Cur["Values"].toMap();
		for(int section = FIRST_COLUMN; section < columnCount(); section++)
		{
			if (!IsColumnEnabled(section))
				continue; // ignore columns which are hidden

			QVariant Value = Cur[m_ColumnKeys.at(section).second];

			STreeNode::SValue& ColValue = pNode->Values[section];

			if (ColValue.Raw != Value)
			{
				Changed = true;
				ColValue.Raw = Value;

				//ColValue.Formatted = 
			}

			if(State != Changed)
			{
				if(State && Index.isValid())
					emit dataChanged(createIndex(Index.row(), Col, pNode), createIndex(Index.row(), section-1, pNode));
				State = Changed;
				Col = section;
			}
			Changed = false;
		}
		if(State && Index.isValid())
			emit dataChanged(createIndex(Index.row(), Col, pNode), createIndex(Index.row(), columnCount()-1, pNode));

	}

	CTreeItemModel::Sync(New, Old);
}

void CTreeItemModel::Sync(QMap<QList<QVariant>, QList<STreeNode*> >& New, QHash<QVariant, STreeNode*>& Old, QList<QModelIndex>* pNewBranches)
{
	if(!Old.isEmpty())
		Purge(m_Root, QModelIndex(), Old);

	if(!New.isEmpty())
	{
		emit layoutAboutToBeChanged();

		//foreach(const QString& Path, New.keys())
		for(QMap<QList<QVariant>, QList<STreeNode*> >::const_iterator I = New.begin(); I != New.end(); I++)
			Fill(m_Root, /*QModelIndex(),*/ I.key(), 0, I.value(), pNewBranches);

		emit layoutChanged();
	}

	emit Updated();
}

/*void CTreeItemModel::CountItems()
{
	CountItems(m_Root);
}

int CTreeItemModel::CountItems(STreeNode* pRoot)
{
	if(pRoot->Children.isEmpty())
		return 1;
	
	int Counter = 0;
	foreach(STreeNode* pChild, pRoot->Children)
		Counter += CountItems(pChild);
	//pRoot->AllChildren = Counter;
	return Counter;
}*/

CTreeItemModel::STreeNode* CTreeItemModel::MkVirtualNode(const QVariant& Id, STreeNode* pParent)
{
	STreeNode* pNode = MkNode(Id);
	pNode->Parent = pParent;
	pNode->Virtual = true;
	pNode->Values.resize(columnCount());
	return pNode;
}

void CTreeItemModel::Purge(STreeNode* pParent, const QModelIndex &parent, QHash<QVariant, STreeNode*> &Old)
{
	int Removed = 0;

	int Begin = -1;
	int End = -1;
	for(int i = pParent->Children.count()-1; i >= -1; i--) 
	{
		STreeNode* pNode = i >= 0 ? pNode = pParent->Children[i] : NULL;
		if(pNode)
			Purge(pNode, index(i, 0, parent), Old);

		bool bRemove = false;
		if(pNode && (pNode->Virtual || pNode->ID.isNull() || (bRemove = Old.value(pNode->ID) != NULL)) && pNode->Children.isEmpty()) // remove it
		{
			//m_Map.remove(pNode->ID, pNode);
			m_Map.remove(pNode->ID);
			if(End == -1)
				End = i;
		}
		else // keep it
		{
			if(bRemove)
			{
				ASSERT(!pNode->Children.isEmpty()); // we wanted to remove it but we have to keep it
				//m_Map.remove(pNode->ID, pNode);
				m_Map.remove(pNode->ID);
				pNode->ID = QVariant();
				pNode->Icon.clear();
			}

			if(End != -1) // remove what's to be removed at once
			{
				Begin = i + 1;

				beginRemoveRows(parent, Begin, End);
				//ASSERT(pParent->Children.count() > End);
				for(int j = End; j >= Begin; j--)
				{
					pNode = pParent->Children.takeAt(j);
					FreeNode(pNode);
					Removed++;
				}
				endRemoveRows();

				End = -1;
				Begin = -1;
			}
		}
    }

	if(Removed > 0)
	{
		//pParent->Aux.clear();
		for (int i = pParent->Children.count() - 1; i >= 0; i--)
		{
			pParent->Children[i]->Row = i;
			//pParent->Aux.insert(pParent->Children[i]->ID, i);
		}
	}
}

void CTreeItemModel::Fill(STreeNode* pParent, /*const QModelIndex &parent,*/ const QList<QVariant>& Paths, int PathsIndex, const QList<STreeNode*>& New, QList<QModelIndex>* pNewBranches)
{
	if(Paths.size() > PathsIndex)
	{
		QVariant CurPath = Paths.at(PathsIndex);
		//STreeNode* pNode;
		//int i = pParent->Aux.value(CurPath, -1);
		//if(i != -1)
		//	pNode = pParent->Children[i];
		//else
		STreeNode* &pNode = m_Map[CurPath];
		if(!pNode)
		{
			//i = 0;
			pNode = MkVirtualNode(CurPath, pParent);

			//if (pNewNode) pNewNode->append(createIndex(pParent->Children.size(), FIRST_COLUMN, pNode));
			if (pNewBranches && pParent->Children.size() == 0 && pParent != m_Root) pNewBranches->append(createIndex(pParent->Row, FIRST_COLUMN, pParent));

			//int Count = pParent->Children.count();
			//beginInsertRows(parent, Count, Count);
			//pParent->Aux.insert(pNode->ID, pParent->Children.size());
			pNode->Row = pParent->Children.size();
			pParent->Children.append(pNode);
			//endInsertRows();
		}
		Fill(pNode, /*index(i, 0, parent),*/ Paths, PathsIndex + 1, New, pNewBranches);
	}
	else
	{
		for(QList<STreeNode*>::const_iterator I = New.begin(); I != New.end(); I++)
		{
			STreeNode* pNode = *I;
			ASSERT(pNode);
			//ASSERT(!m_Map.contains(pNode->ID));
			m_Map.insert(pNode->ID, pNode);
			pNode->Parent = pParent;

			//if (pNewNode) pNewNode->append(createIndex(pParent->Children.size(), FIRST_COLUMN, pNode));
			if (pNewBranches && pParent->Children.size() == 0 && pParent != m_Root) pNewBranches->append(createIndex(pParent->Row, FIRST_COLUMN, pParent));

			//int Count = pParent->Children.count();
			//beginInsertRows(parent, Count, Count);
			//if(!m_LeafsOnly) // when all non virtual entries are always leafs, don't fill the aux map
			//	pParent->Aux.insert(pNode->ID, pParent->Children.size());
			pNode->Row = pParent->Children.size();
			pParent->Children.append(pNode);
			//endInsertRows();
		}
	}
}

QModelIndex CTreeItemModel::FindIndex(const QVariant& ID)
{
	if(STreeNode* pNode = m_Map.value(ID))
		return Find(m_Root, pNode);
	return QModelIndex();
}

QModelIndex CTreeItemModel::Find(STreeNode* pParent, STreeNode* pNode) const
{
	// ''find''
	ASSERT(pNode->Parent->Children[pNode->Row] == pNode);
	return createIndex(pNode->Row, FIRST_COLUMN, pNode);
	/*
	int count = pParent->Children.count();
	for(int i=0; i < count; i++)
	{
		if (pParent->Children[i] == pNode)
		{
			ASSERT(i == pNode->Row);
			return createIndex(i, FIRST_COLUMN, pNode);
		}

		QModelIndex Index = Find(pParent->Children[i], pNode);
		if(Index.isValid())
			return Index;
	}
	return QModelIndex();
	*/
}

void CTreeItemModel::Clear()
{
	QHash<QVariant, STreeNode*> Old = m_Map;
	//beginResetModel();
	Purge(m_Root, QModelIndex(), Old);
	//endResetModel();
	ASSERT(m_Map.isEmpty());
}

void CTreeItemModel::RemoveIndex(const QModelIndex &index)
{
	if (!index.isValid())
        return;

	STreeNode* pNode = static_cast<STreeNode*>(index.internalPointer());
	if (!m_Nodes.contains(pNode))
		return;

	QHash<QVariant, STreeNode*> Old;
	Old[pNode->ID] = pNode;

	Purge(m_Root, QModelIndex(), Old);
}

QVariant CTreeItemModel::data(const QModelIndex &index, int role) const
{
    return Data(index, role, index.column());
}

bool CTreeItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if(index.column() == FIRST_COLUMN && role == Qt::CheckStateRole)
	{
		STreeNode* pNode = static_cast<STreeNode*>(index.internalPointer());
		if (!m_Nodes.contains(pNode))
			return false;
		emit CheckChanged(pNode->ID, value.toInt() != Qt::Unchecked);
		return true;
	}
	return false;
}

QVariant CTreeItemModel::GetItemID(const QModelIndex& index) const
{
	if (!index.isValid())
		return QVariant();

	STreeNode* pNode = static_cast<STreeNode*>(index.internalPointer());
	if (!m_Nodes.contains(pNode))
		return QVariant();
	return pNode->ID;
}

QVariant CTreeItemModel::Data(const QModelIndex &index, int role, int section) const
{
	if (!index.isValid())
		return QVariant();

	//if(role == Qt::SizeHintRole)
	//    return QSize(64,16); // for fixing height

	STreeNode* pNode = static_cast<STreeNode*>(index.internalPointer());
	if (!m_Nodes.contains(pNode))
		return QVariant();

	return NodeData(pNode, role, section);
}

QVariant CTreeItemModel::NodeData(STreeNode* pNode, int role, int section) const
{
	if (pNode->Values.size() <= section)
		return QVariant();

    switch(role)
	{
		case Qt::DisplayRole:
		{
			STreeNode::SValue& Value = pNode->Values[section];
			return Value.Formatted.isValid() ? Value.Formatted : Value.Raw;
		}
		case Qt::EditRole: // sort role
		{
			STreeNode::SValue& value = pNode->Values[section];
			return value.SortKey.isValid() ? value.SortKey : value.Raw;
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
		case Qt::CheckStateRole:
		{
			/*if(section == eModule)
			{
				if(pNode->...)
					return Qt::Unchecked;
				else
					return Qt::Checked;
			}*/
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

Qt::ItemFlags CTreeItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
	if(index.column() == 0)
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex CTreeItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    STreeNode* pParent;
    if (!parent.isValid())
        pParent = m_Root;
	else {
		pParent = static_cast<STreeNode*>(parent.internalPointer());
		if (!m_Nodes.contains(pParent))
			return QModelIndex();
	}

	if(STreeNode* pNode = pParent->Children.count() > row ? pParent->Children[row] : NULL)
        return createIndex(row, column, pNode);
    return QModelIndex();
}

QModelIndex CTreeItemModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    STreeNode* pNode = static_cast<STreeNode*>(index.internalPointer());
	if (!m_Nodes.contains(pNode))
		return QModelIndex();
	ASSERT(pNode->Parent);
	STreeNode* pParent = pNode->Parent;
    if (pParent == m_Root)
        return QModelIndex();

	int row = 0;
	if(pParent->Parent)
		row = pParent->Parent->Children.indexOf(pParent);
    return createIndex(row, 0, pParent);
}

int CTreeItemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

	STreeNode* pNode;
    if (!parent.isValid())
        pNode = m_Root;
	else {
		pNode = static_cast<STreeNode*>(parent.internalPointer());
		if (!m_Nodes.contains(pNode))
			return 0;
	}
	return pNode->Children.count();
}

int CSimpleTreeModel::columnCount(const QModelIndex &parent) const
{
	return m_ColumnKeys.count();
}

QVariant CSimpleTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		if (section < m_ColumnKeys.size())
			return m_ColumnKeys.at(section).first;
	}
    return QVariant();
}
