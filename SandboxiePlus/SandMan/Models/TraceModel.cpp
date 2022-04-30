#include "stdafx.h"
#include "TraceModel.h"
#include "../MiscHelpers/Common/Common.h"
#include "../SbiePlusAPI.h"



CTraceModel::CTraceModel(QObject* parent)
	:CTreeItemModel(parent)
{
	m_Root = MkNode(QVariant());

	m_LastCount = 0;
}

CTraceModel::~CTraceModel()
{
}

/*QList<QVariant> CTraceModel::MakePath(const CTraceEntryPtr& pEntry, const QList<CTraceEntryPtr>& EntryList)
{
	quint64 ParentID = pEntry->GetParentWnd();
	CTraceEntryPtr pParent = EntryList.value(ParentID);

	QList<QVariant> Path;
	if (!pParent.isNull() && ParentID != pEntry->GetHWnd())
	{
		Path = MakeWndPath(pParent, EntryList);
		Path.append(ParentID);
	}
	return Path;
}

bool CTraceModel::TestPath(const QList<QVariant>& Path, const CTraceEntryPtr& pEntry, const QList<CTraceEntryPtr>& EntryList, int Index)
{
	quint64 ParentID = pEntry->GetParentWnd();
	CTraceEntryPtr pParent = EntryList.value(ParentID);

	if (!pParent.isNull() && ParentID != pEntry->GetHWnd())
	{
		if (Index >= Path.size() || Path[Path.size() - Index - 1] != ParentID)
			return false;

		return TestWndPath(Path, pParent, EntryList, Index + 1);
	}

	return Path.size() == Index;
}*/

QList<QVariant>	CTraceModel::Sync(const QVector<CTraceEntryPtr>& EntryList, int (*Filter)(const CTraceEntryPtr&, void*), void* params)
{
	QList<QVariant>	Added;
	QMap<QList<QVariant>, QList<STreeNode*> > New;
	QHash<QVariant, STreeNode*> Old = m_Map;

	// Note: since this is a log and we ever always only add entries we save cpu time by always skipping the already know portion of the list

	int i = 0;
	if (EntryList.count() >= m_LastCount && m_LastCount > 0)
	{
		i = m_LastCount - 1;
		if (m_LastID == EntryList.at(i)->GetUID())
		{
			i++;
			Old.clear();
		}
		else
			i = 0;
	}

	for (; i < EntryList.count(); i++)
	{
		CTraceEntryPtr pEntry = EntryList.at(i);

		int iFilter = Filter(pEntry, params);
		if (!iFilter)
			continue;

		quint64 ID = pEntry->GetUID();

		QModelIndex Index;

		QHash<QVariant, STreeNode*>::iterator I = Old.find(ID);
		STraceNode* pNode = I != Old.end() ? static_cast<STraceNode*>(I.value()) : NULL;
		if (!pNode /*|| (m_bTree ? !TestPath(pNode->Path, pEntry, EntryList) : !pNode->Path.isEmpty())*/)
		{
			pNode = static_cast<STraceNode*>(MkNode(ID));
			pNode->Values.resize(columnCount());
			if (m_bTree) {
				//pNode->Path.append(QString("pid_%1").arg(pEntry->GetProcessId()));
				//pNode->Path.append(QString("tid_%1").arg(pEntry->GetThreadId()));
				if (pEntry->GetProcessName().isEmpty())
					pNode->Path.append(tr("Process %1").arg(pEntry->GetProcessId()));
				else
					pNode->Path.append(tr("%1 (%2)").arg(pEntry->GetProcessName()).arg(pEntry->GetProcessId()));
				pNode->Path.append(QString("Thread %1").arg(pEntry->GetThreadId()));
				//pNode->Path = MakePath(pEntry, EntryList);
			}
			pNode->pEntry = pEntry;
			New[pNode->Path].append(pNode);
			//SetProcessName(pEntry->GetProcessName(), pEntry->GetProcessId(), pEntry->GetThreadId());
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

		if (pNode->bHighLight != (iFilter == 2)) {
			pNode->bHighLight = (iFilter == 2);
			pNode->Color = pNode->bHighLight ? Qt::yellow : QColor();
			Changed = -1;
		}

		for (int section = 0; section < columnCount(); section++)
		{
			if (!m_Columns.contains(section))
				continue; // ignore columns which are hidden

			QVariant Value;
			switch (section)
			{
			//case eProcess:			Value = pEntry->GetProcessId(); break;
			//case eTimeStamp:		Value = pEntry->GetUID(); break;
			case eProcess:			Value = pEntry->GetUID(); break;
			case eType:				Value = pEntry->GetTypeStr(); break;
			case eStatus:			Value = pEntry->GetStautsStr(); break;
			case eValue:		{			
									QString sValue = pEntry->GetName();
									if (!sValue.isEmpty() && !pEntry->GetMessage().isEmpty())
										sValue += " ";
									sValue += pEntry->GetMessage();
									Value = sValue;
									break;
								}
			}

			STraceNode::SValue& ColValue = pNode->Values[section];

			if (ColValue.Raw != Value)
			{
				if (Changed == 0)
					Changed = 1;
				ColValue.Raw = Value;

				switch (section)
				{
					/*case eProcess:
					{
						CBoxedProcessPtr pProcess = theAPI->GetProcessById(pEntry->GetProcessId());
						ColValue.Formated = QString("%1 (%2, %3)").arg(pProcess.isNull() ? tr("Unknown") : pProcess->GetProcessName()).arg(pEntry->GetProcessId()).arg(pEntry->GetThreadId());
						break;
					}
					case eTimeStamp:		ColValue.Formated = pEntry->GetTimeStamp().toString("hh:mm:ss.zzz"); break;*/
					case eProcess:			
						if(!m_bTree) {
							QString Name = pEntry->GetProcessName();
							ColValue.Formated = QString("%1 (%2, %3) - %4").arg(Name.isEmpty() ? tr("Unknown") : Name)
								.arg(pEntry->GetProcessId()).arg(pEntry->GetThreadId()).arg(pEntry->GetTimeStamp().toString("hh:mm:ss.zzz"));
						} else 
							ColValue.Formated = pEntry->GetTimeStamp().toString("hh:mm:ss.zzz");
						break;
						//case eType:			ColValue.Formated = ; break;
						//case eValue:			ColValue.Formated = ; break;
				}
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

	m_LastCount = EntryList.count();
	if(m_LastCount)
		m_LastID = EntryList.last()->GetUID();

	CTreeItemModel::Sync(New, Old, &Added);

	return Added;
}

void CTraceModel::Clear()
{
	m_LastCount = 0;
	m_LastID.clear();

	/*foreach(quint32 pid, m_PidMap.uniqueKeys()) {
		SProgInfo& Info = m_PidMap[pid];
		Info.Dirty = true;
		Info.Threads.clear();
	}
	m_PidMap.clear();*/
	CTreeItemModel::Clear();
}

/*
void CTraceModel::SetProcessName(const QString& Name, quint32 pid, quint32 tid)
{
	SProgInfo& Info = m_PidMap[pid];
	Info.Name = Name;
	if (!Info.Threads.contains(tid)) {
		Info.Threads.insert(tid);
		Info.Dirty = true;
	}
	if (Info.Dirty) {
		Info.Dirty = false;
		emit NewBranche();
	}
}

QString CTraceModel::GetProcessName(quint32 pid)
{
	SProgInfo& Info = m_PidMap[pid];
	return Info.Name;
}

void CTraceModel::LogThreadId(quint32 pid, quint32 tid)
{
	SProgInfo& Info = m_PidMap[pid];
	if (!Info.Threads.contains(tid)) {
		Info.Threads.insert(tid);
		emit NewBranche();
	}
}
*/

CTraceModel::STreeNode* CTraceModel::MkVirtualNode(const QVariant& Id, STreeNode* pParent)
{ 
	STreeNode* pNode = CTreeItemModel::MkVirtualNode(Id, pParent);

	/*StrPair typeId = Split2(Id.toString(), "_");
	if (typeId.first == "pid")
	{
		quint32 pid = typeId.second.toUInt();
		QString Name = GetProcessName(pid);
		pNode->Values[0].Raw = pid;
		if(!Name.isEmpty())
			pNode->Values[0].Formated = tr("%1 (%2)").arg(Name).arg(pid); 
		else
			pNode->Values[0].Formated = tr("Process %1").arg(pid);
	}
	else if (typeId.first == "tid")
	{
		quint32 tid = typeId.second.toUInt();
		quint32 pid = Split2(pParent->ID.toString(), "_").second.toUInt();
		LogThreadId(pid, tid);
		pNode->Values[0].Raw = tid;
		pNode->Values[0].Formated = tr("Thread %1").arg(tid);
	}
	else*/
		pNode->Values[0].Raw = pNode->Values[0].Formated = Id;

	return pNode;
}

CTraceEntryPtr CTraceModel::GetEntry(const QModelIndex& index) const
{
	if (!index.isValid())
		return CTraceEntryPtr();

	STraceNode* pNode = static_cast<STraceNode*>(index.internalPointer());
	ASSERT(pNode);

	return pNode->pEntry;
}

int CTraceModel::columnCount(const QModelIndex& parent) const
{
	return eCount;
}

QVariant CTraceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch (section)
		{
		case eProcess:			return tr("Process");
		//case eTimeStamp:		return tr("Time Stamp");
		case eType:				return tr("Type");
		case eStatus:			return tr("Status");
		case eValue:			return tr("Value");
		}
	}
	return QVariant();
}