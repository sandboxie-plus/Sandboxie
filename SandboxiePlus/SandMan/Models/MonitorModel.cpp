#include "stdafx.h"
#include "MonitorModel.h"
#include "../MiscHelpers/Common/Common.h"
#include "../SbiePlusAPI.h"

CMonitorModel::CMonitorModel(QObject* parent)
	:CTreeItemModel(parent)
{
	m_Root = MkNode(QVariant());
}

CMonitorModel::~CMonitorModel()
{
}

QList<QVariant>	CMonitorModel::Sync(const QMap<QString, CMonitorEntryPtr>& EntryMap, void* params)
{
	QList<QVariant>	Added;
	QMap<QList<QVariant>, QList<STreeNode*> > New;
	QHash<QVariant, STreeNode*> Old = m_Map;

	foreach(const CMonitorEntryPtr pEntry, EntryMap)
	{
		quint64 ID = (quint64)pEntry.data();

		QModelIndex Index;

		QHash<QVariant, STreeNode*>::iterator I = Old.find(ID);
		STraceNode* pNode = I != Old.end() ? static_cast<STraceNode*>(I.value()) : NULL;
		if (!pNode)
		{
			pNode = static_cast<STraceNode*>(MkNode(ID));
			pNode->Values.resize(columnCount());
			pNode->pEntry = pEntry;
			New[pNode->Path].append(pNode);
		}
		else
		{
			I.value() = NULL;
			Index = Find(m_Root, pNode);
		}

		//if(Index.isValid()) // this is to slow, be more precise
		//	emit dataChanged(createIndex(Index.row(), 0, pNode), createIndex(Index.row(), columnCount()-1, pNode));

		int Col = 0;
		bool State = false;
		int Changed = 0;

		for (int section = 0; section < columnCount(); section++)
		{
			if (!m_Columns.contains(section))
				continue; // ignore columns which are hidden

			QVariant Value;
			switch (section)
			{
			case eType:				Value = pEntry->GetTypeStr(); break;
			case eStatus:			Value = pEntry->GetStautsStr(); break;
			case eValue:			Value = pEntry->GetName(); break;
			case eCounter:			Value = pEntry->GetCount(); break;
			}

			STraceNode::SValue& ColValue = pNode->Values[section];

			if (ColValue.Raw != Value)
			{
				if (Changed == 0)
					Changed = 1;
				ColValue.Raw = Value;

				/*switch (section)
				{
						//case eType:			ColValue.Formatted = ; break;
						//case eValue:			ColValue.Formatted = ; break;
				}*/
			}

			if (State != (Changed != 0))
			{
				if (State && Index.isValid())
					emit dataChanged(createIndex(Index.row(), Col, pNode), createIndex(Index.row(), section - 1, pNode));
				State = (Changed != 0);
				Col = section;
			}
			if (Changed == 1)
				Changed = 0;
		}
		if (State && Index.isValid())
			emit dataChanged(createIndex(Index.row(), Col, pNode), createIndex(Index.row(), columnCount() - 1, pNode));

	}

	CTreeItemModel::Sync(New, Old, &Added);

	return Added;
}

void CMonitorModel::Clear()
{
	CTreeItemModel::Clear();
}

int CMonitorModel::columnCount(const QModelIndex& parent) const
{
	return eCount;
}

QVariant CMonitorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch (section)
		{
		case eType:				return tr("Type");
		case eStatus:			return tr("Status");
		case eValue:			return tr("Value");
		case eCounter:			return tr("Count");
		}
	}
	return QVariant();
}