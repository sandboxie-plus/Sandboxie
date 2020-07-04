#include "stdafx.h"
#include "ApiMonModel.h"
#include "../MiscHelpers/Common/Common.h"

CApiMonModel::CApiMonModel(QObject *parent)
:CListItemModel(parent)
{
}

CApiMonModel::~CApiMonModel()
{
}

void CApiMonModel::Sync(const QList<CApiLogEntryPtr>& List, QSet<quint64> PIDs)
{
	QList<SListNode*> New;
	QHash<QVariant, SListNode*> Old = m_Map;

	foreach (const CApiLogEntryPtr& pEntry, List)
	{
		QVariant ID = pEntry->GetUID();

		if (!PIDs.isEmpty() && !PIDs.contains(pEntry->GetProcessId()))
			continue;

		int Row = -1;
		QHash<QVariant, SListNode*>::iterator I = Old.find(ID);
		SApiLogNode* pNode = I != Old.end() ? static_cast<SApiLogNode*>(I.value()) : NULL;
		if(!pNode)
		{
			pNode = static_cast<SApiLogNode*>(MkNode(ID));
			pNode->Values.resize(columnCount());
			pNode->pEntry = pEntry;
			New.append(pNode);
		}
		else
		{
			I.value() = NULL;
			Row = GetRow(pNode);
		}

		int Col = 0;
		bool State = false;
		int Changed = 0;

		/*int RowColor = CTaskExplorer::eNone;
		if (pGDI->IsMarkedForRemoval() && CTaskExplorer::UseListColor(CTaskExplorer::eToBeRemoved))		RowColor = CTaskExplorer::eToBeRemoved;
		else if (pGDI->IsNewlyCreated() && CTaskExplorer::UseListColor(CTaskExplorer::eAdded))			RowColor = CTaskExplorer::eAdded;

		if (pNode->iColor != RowColor) {
			pNode->iColor = RowColor;
			pNode->Color = CTaskExplorer::GetListColor(RowColor);
			Changed = 2;
		}*/

		for(int section = 0; section < columnCount(); section++)
		{
			if (!m_Columns.contains(section))
				continue; // ignore columns which are hidden

			QVariant Value;
			switch(section)
			{
				case eProcess:			Value = pEntry->GetProcessId(); break;
				case eTimeStamp:		Value = pEntry->GetTimeStamp(); break;
				case eMessage:			Value = pEntry->GetMessage(); break;
			}

			SApiLogNode::SValue& ColValue = pNode->Values[section];

			if (ColValue.Raw != Value)
			{
				if(Changed == 0)
					Changed = 1;
				ColValue.Raw = Value;

				switch (section)
				{
					case eProcess:			ColValue.Formated = QString::number(pEntry->GetProcessId()); break;
					case eTimeStamp:		ColValue.Formated = pEntry->GetTimeStamp().toString("hh:mm:ss.zzz"); break;
					//case eType:			ColValue.Formated = ; break;
					//case eValue:			ColValue.Formated = ; break;
				}
			}

			if(State != (Changed != 0))
			{
				if(State && Row != -1)
					emit dataChanged(createIndex(Row, Col), createIndex(Row, section-1));
				State = (Changed != 0);
				Col = section;
			}
			if(Changed == 1)
				Changed = 0;
		}
		if(State && Row != -1)
			emit dataChanged(createIndex(Row, Col, pNode), createIndex(Row, columnCount()-1, pNode));

	}

	CListItemModel::Sync(New, Old);
}

CApiLogEntryPtr CApiMonModel::GetEntry(const QModelIndex &index) const
{
	if (!index.isValid())
        return CApiLogEntryPtr();

	SApiLogNode* pNode = static_cast<SApiLogNode*>(index.internalPointer());
	return pNode->pEntry;
}

int CApiMonModel::columnCount(const QModelIndex &parent) const
{
	return eCount;
}

QVariant CApiMonModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch(section)
		{
			case eProcess:			return tr("Process");
			case eTimeStamp:		return tr("Time Stamp");
			case eMessage:			return tr("Message");
		}
	}
    return QVariant();
}
