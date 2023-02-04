#include "stdafx.h"
#include "TraceModel.h"
#include "../MiscHelpers/Common/Common.h"
#include "../SbiePlusAPI.h"

#define FIRST_COLUMN 0

#define PROCESS_MARK	0x10000000
#define THREAD_MARK		0x20000000

CTraceModel::CTraceModel(QObject* parent)
	:QAbstractItemModelEx(parent)
{
	m_bTree = false;

	m_Root = MkNode(0);

	m_LastCount = 0;
}

CTraceModel::~CTraceModel()
{
	FreeNode(m_Root);
	m_Root = NULL;
}

QList<QModelIndex> CTraceModel::Sync(const QVector<CTraceEntryPtr>& EntryList)
{
	QList<QModelIndex> NewBranches;

	// Note: since this is a log and we ever always only add entries we save cpu time by always skipping the already know portion of the list

	int i = 0;
	if (EntryList.count() >= m_LastCount && m_LastCount > 0)
	{
		i = m_LastCount - 1;
		if (m_LastID == EntryList.at(i)->GetUID())
			i++;
		else
			i = 0;
	}

	emit layoutAboutToBeChanged();

	for (; i < EntryList.count(); i++)
	{
		const CTraceEntryPtr& pEntry = EntryList.at(i);

		quint64 ID = pEntry->GetUID();

		STreeNode* pNode = MkNode(ID);
		pNode->pEntry = pEntry;
		if (m_bTree)
		{
			quint64 Path = PROCESS_MARK | pEntry->GetProcessId();
			Path |= quint64(THREAD_MARK | pEntry->GetThreadId()) << 32;

			pNode->Parent = FindParentNode(m_Root, Path, 0, &NewBranches);
		}
		else
			pNode->Parent = m_Root;

		//pNode->Row = pNode->Parent->Children.size();
		pNode->Parent->Children.append(pNode);
	}

	emit layoutChanged();

	m_LastCount = EntryList.count();
	if(m_LastCount)
		m_LastID = EntryList.last()->GetUID();

	return NewBranches;
}

//__inline uint qHash(const CTraceModel::STracePath& var)
//{
//    unsigned int hash = 5381;
//    for (quint32* ptr = var.path; ptr < var.path + var.count; ptr++)
//        hash = ((hash << 15) + hash) ^ *ptr;
//    return hash;
//}
//
//bool operator == (const CTraceModel::STracePath& l, const CTraceModel::STracePath& r)
//{
//	if (l.count != r.count)
//		return false;
//	return memcmp(l.path, r.path, l.count) == 0;
//}

CTraceModel::STreeNode* CTraceModel::FindParentNode(STreeNode* pParent, quint64 Path, int PathsIndex, QList<QModelIndex>* pNewBranches)
{
	if (2 <= PathsIndex)
		return pParent;
	
	quint64 CurPath = PathsIndex == 0 ? Path & 0x00000000FFFFFFFF : Path;
	STreeNode* &pNode = m_Branches[CurPath];
	if(!pNode)
	{
		pNode = MkNode(PathsIndex == 0 ? Path & 0x00000000FFFFFFFF : (Path >> 32));
		pNode->Parent = pParent;
		pNewBranches->append(createIndex(pParent->Children.size(), FIRST_COLUMN, pNode));

		//pNode->Row = pParent->Children.size();
		pParent->Children.append(pNode);
	}
	return FindParentNode(pNode, Path, PathsIndex + 1, pNewBranches);
}

void CTraceModel::Clear(bool bMem)
{
	m_LastCount = 0;
	m_LastID.clear();

	beginResetModel();

	m_Branches.clear();
	FreeNode(m_Root);

	if (bMem)
		m_NodeAllocator.dispose();

	m_Root = MkNode(0);

	endResetModel();
}

PoolAllocator<sizeof(CTraceModel::STreeNode)> CTraceModel::m_NodeAllocator;

CTraceModel::STreeNode*	CTraceModel::MkNode(quint64 Id) 
{ 
	STreeNode* pNode = (STreeNode*)m_NodeAllocator.allocate(sizeof(STreeNode));
	new (pNode) STreeNode(Id);
	return pNode;
	//return new STreeNode(Id);
}

void CTraceModel::FreeNode(STreeNode* pNode) 
{ 
	foreach(STreeNode* pSubNode, pNode->Children)
		FreeNode(pSubNode);
	pNode->~STreeNode();
	m_NodeAllocator.free(pNode);
	//delete pNode; 
}

bool CTraceModel::TestHighLight(STreeNode* pNode) const
{
	if (m_HighLightExp.isEmpty())
		return false;
	for (int i = 0; i < eCount; i++) {
		if (NodeData(pNode, Qt::DisplayRole, i).toString().contains(m_HighLightExp, Qt::CaseInsensitive))
			return true;
	}
	return false;
}

QVariant CTraceModel::NodeData(STreeNode* pNode, int role, int section) const
{
	const CTraceEntryPtr& pEntry = pNode->pEntry;
	if(!pEntry.constData())
	{
		if (section != FIRST_COLUMN || (role != Qt::DisplayRole && role != Qt::EditRole))
			return QVariant();

		quint32 id = pNode->ID;
		if (id & PROCESS_MARK) {
			CTraceEntryPtr pProcEntry; // pick first log entry of first thread to query the process name
			if (!pNode->Children.isEmpty()) {
				STreeNode* pSubNode = pNode->Children.first();
				if (!pSubNode->Children.isEmpty())
					pProcEntry = pSubNode->Children.first()->pEntry;
			}
			if (pProcEntry && !pProcEntry->GetProcessName().isEmpty())
				return tr("%1 (%2)").arg(pProcEntry->GetProcessName()).arg(pProcEntry->GetProcessId());
			return tr("Process %1").arg(id & 0x0FFFFFFF);
		}
		else if (id & THREAD_MARK)
			return tr("Thread %1").arg(id & 0x0FFFFFFF);
		else
			return QString::number(id, 16).rightJustified(8, '0');
	}
	
	switch(role)
	{
		case Qt::DisplayRole:
		case Qt::EditRole: // sort role
		{
			switch (section)
			{
			//case eProcess:		return pEntry->GetProcessId(); 
			//case eTimeStamp:		return pEntry->GetUID();
			case eProcess:		{
									if(!m_bTree) {
										QString Name = pEntry->GetProcessName();
										return QString("%1 (%2, %3) - %4").arg(Name.isEmpty() ? tr("Unknown") : Name)
											.arg(pEntry->GetProcessId()).arg(pEntry->GetThreadId())
											.arg(QDateTime::fromMSecsSinceEpoch(pEntry->GetTimeStamp()).toString("hh:mm:ss.zzz"));
									} else 
										return QDateTime::fromMSecsSinceEpoch(pEntry->GetTimeStamp()).toString("hh:mm:ss.zzz");
								}
			case eType:				return pEntry->GetTypeStr();
			case eStatus:			return pEntry->GetStautsStr();
			case eValue:		{			
									QString sValue = pEntry->GetName();
									if (!sValue.isEmpty() && !pEntry->GetMessage().isEmpty())
										sValue += " ";
									sValue += pEntry->GetMessage();
									return sValue;
								}
			}
		}
		case Qt::BackgroundRole:
		{
			if(!CTreeItemModel::GetDarkMode())
				return TestHighLight(pNode) ? QColor(Qt::yellow) : QVariant();
			break;
		}
		case Qt::ForegroundRole:
		{
			if(CTreeItemModel::GetDarkMode())
				return TestHighLight(pNode) ? QColor(Qt::yellow) : QVariant();
			break;
		}
	}

	return QVariant();
}

CTraceEntryPtr CTraceModel::GetEntry(const QModelIndex& index) const
{
	if (!index.isValid())
		return CTraceEntryPtr();

	STreeNode* pNode = static_cast<STreeNode*>(index.internalPointer());
	ASSERT(pNode);

	return pNode->pEntry;
}

QVariant CTraceModel::data(const QModelIndex &index, int role) const
{
    return Data(index, role, index.column());
}

QVariant CTraceModel::GetItemID(const QModelIndex& index) const
{
	if (!index.isValid())
		return QVariant();

	STreeNode* pNode = static_cast<STreeNode*>(index.internalPointer());

	return pNode->ID;
}

QVariant CTraceModel::Data(const QModelIndex &index, int role, int section) const
{
	if (!index.isValid())
		return QVariant();

	STreeNode* pNode = static_cast<STreeNode*>(index.internalPointer());
	ASSERT(pNode);

	return NodeData(pNode, role, section);
}

Qt::ItemFlags CTraceModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
	if(index.column() == 0)
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex CTraceModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    STreeNode* pParent;
    if (!parent.isValid())
        pParent = m_Root;
    else
        pParent = static_cast<STreeNode*>(parent.internalPointer());

	if(STreeNode* pNode = pParent->Children.count() > row ? pParent->Children[row] : NULL)
        return createIndex(row, column, pNode);
    return QModelIndex();
}

QModelIndex CTraceModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    STreeNode* pNode = static_cast<STreeNode*>(index.internalPointer());
	ASSERT(pNode->Parent);
	STreeNode* pParent = pNode->Parent;
    if (pParent == m_Root)
        return QModelIndex();

	int row = 0;
	if(pParent->Parent)
		row = pParent->Parent->Children.indexOf(pParent);
    return createIndex(row, 0, pParent);
}

int CTraceModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

	STreeNode* pNode;
    if (!parent.isValid())
        pNode = m_Root;
    else
        pNode = static_cast<STreeNode*>(parent.internalPointer());
	return pNode->Children.count();
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