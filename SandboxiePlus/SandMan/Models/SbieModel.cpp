#include "stdafx.h"
#include "SbieModel.h"
#include "../../MiscHelpers/Common/Common.h"
#include "../../MiscHelpers/Common/IconExtreactor.h"


CSbieModel::CSbieModel(QObject *parent)
:CTreeItemModel(parent)
{
	for (int i = 0; i < eMaxColor; i++)
		m_BoxIcons[(EBoxColors)i] = qMakePair(QIcon(QString(":/Boxes/Empty%1").arg(i)), QIcon(QString(":/Boxes/Full%1").arg(i)));

	//m_BoxEmpty = QIcon(":/BoxEmpty");
	//m_BoxInUse = QIcon(":/BoxInUse");
	m_ExeIcon = QIcon(":/exeIcon32");

	m_Root = MkNode(QVariant());
}

CSbieModel::~CSbieModel()
{
}

QList<QVariant> CSbieModel::MakeProcPath(const QString& BoxName, const CBoxedProcessPtr& pProcess, const QMap<quint64, CBoxedProcessPtr>& ProcessList)
{
	QList<QVariant> Path = MakeProcPath(pProcess, ProcessList);
	Path.prepend(BoxName);
	return Path;
}

QList<QVariant> CSbieModel::MakeProcPath(const CBoxedProcessPtr& pProcess, const QMap<quint64, CBoxedProcessPtr>& ProcessList)
{
	quint64 ParentID = pProcess->GetParendPID();
	CBoxedProcessPtr pParent = ProcessList.value(ParentID);

	QList<QVariant> Path;
	if (!pParent.isNull() && ParentID != pProcess->GetProcessId())
	{
		Path = MakeProcPath(pParent, ProcessList);
		Path.append(ParentID);
	}
	return Path;
}

bool CSbieModel::TestProcPath(const QList<QVariant>& Path, const QString& BoxName, const CBoxedProcessPtr& pProcess, const QMap<quint64, CBoxedProcessPtr>& ProcessList, int Index)
{
	if (Index == 0)
	{
		if (Path.isEmpty() || BoxName != Path[0])
			return false;

		return TestProcPath(Path, BoxName, pProcess, ProcessList, 1);
	}

	quint64 ParentID = pProcess->GetParendPID();
	CBoxedProcessPtr pParent = ProcessList.value(ParentID);

	if (!pParent.isNull() && ParentID != pProcess->GetProcessId())
	{
		if(Index >= Path.size() || Path[Path.size() - Index] != ParentID)
			return false;

		return TestProcPath(Path, BoxName, pParent, ProcessList, Index + 1);
	}

	return Path.size() == Index;
}

QList<QVariant> CSbieModel::Sync(const QMap<QString, CSandBoxPtr>& BoxList)
{
	QList<QVariant> Added;
	QMap<QList<QVariant>, QList<STreeNode*> > New;
	QHash<QVariant, STreeNode*> Old = m_Map;

	foreach (const CSandBoxPtr& pBox, BoxList)
	{
		QVariant ID = pBox->GetName();

		QModelIndex Index;
		
		QHash<QVariant, STreeNode*>::iterator I = Old.find(ID);
		SSandBoxNode* pNode = I != Old.end() ? static_cast<SSandBoxNode*>(I.value()) : NULL;
		if(!pNode)
		{
			pNode = static_cast<SSandBoxNode*>(MkNode(ID));
			pNode->Values.resize(columnCount());
			pNode->pBox = pBox;
			New[pNode->Path].append(pNode);
			Added.append(ID);
		}
		else
		{
			I.value() = NULL;
			Index = Find(m_Root, pNode);
		}

		CSandBoxPlus* pBoxEx = qobject_cast<CSandBoxPlus*>(pBox.data());

		int Col = 0;
		bool State = false;
		int Changed = 0;

		QMap<quint64, CBoxedProcessPtr> ProcessList = pBox->GetProcessList();

		bool HasActive = Sync(pBox, ProcessList, New, Old, Added);
		int inUse = (HasActive ? 1 : 0);
		int boxType = eYelow;
		if(pBoxEx->HasLogApi())
			boxType = eRed;
		if (pBoxEx->IsUnsecureDebugging())
			boxType = eMagenta;
		else if (pBoxEx->IsSecurityRestricted())
			boxType = eOrang;

		if (pNode->inUse != inUse || pNode->boxType != boxType)
		{
			pNode->inUse = inUse;
			pNode->boxType = boxType;
			//pNode->Icon = pNode->inUse ? m_BoxInUse : m_BoxEmpty;
			pNode->Icon = pNode->inUse ? m_BoxIcons[(EBoxColors)boxType].second : m_BoxIcons[(EBoxColors)boxType].first;
			Changed = 1; // set change for first column
		}

		for(int section = 0; section < columnCount(); section++)
		{
			if (!m_Columns.contains(section))
				continue; // ignore columns which are hidden

			QVariant Value;
			switch(section)
			{
				case eName:				Value = pBox->GetName(); break;
				case eStatus:			Value = pBox.objectCast<CSandBoxPlus>()->GetStatusStr(); break;
				case ePath:				Value = pBox->GetFileRoot(); break;
			}

			SSandBoxNode::SValue& ColValue = pNode->Values[section];

			if (ColValue.Raw != Value)
			{
				if(Changed == 0)
					Changed = 1;
				ColValue.Raw = Value;

				switch (section)
				{
				case eName:				ColValue.Formated = Value.toString().replace("_", " "); break;
				}
			}

			if(State != (Changed != 0))
			{
				if(State && Index.isValid())
					emit dataChanged(createIndex(Index.row(), Col, pNode), createIndex(Index.row(), section-1, pNode));
				State = (Changed != 0);
				Col = section;
			}
			if(Changed == 1)
				Changed = 0;
		}
		if(State && Index.isValid())
			emit dataChanged(createIndex(Index.row(), Col, pNode), createIndex(Index.row(), columnCount()-1, pNode));
	}

	CTreeItemModel::Sync(New, Old);
	return Added;
}

bool CSbieModel::Sync(const CSandBoxPtr& pBox, const QMap<quint64, CBoxedProcessPtr>& ProcessList, QMap<QList<QVariant>, QList<STreeNode*> >& New, QHash<QVariant, STreeNode*>& Old, QList<QVariant>& Added)
{
	QString BoxName = pBox->GetName();

	int ActiveCount = 0;

	foreach(const CBoxedProcessPtr& pProcess, ProcessList)
	{
		QVariant ID = pProcess->GetProcessId();

		QModelIndex Index;

		QHash<QVariant, STreeNode*>::iterator I = Old.find(ID);
		SSandBoxNode* pNode = I != Old.end() ? static_cast<SSandBoxNode*>(I.value()) : NULL;
		if (!pNode || (m_bTree ? !TestProcPath(pNode->Path, BoxName, pProcess, ProcessList) : !pNode->Path.isEmpty()))
		{
			pNode = static_cast<SSandBoxNode*>(MkNode(ID));
			pNode->Values.resize(columnCount());
			if(m_bTree)
				pNode->Path = MakeProcPath(BoxName, pProcess, ProcessList);
			pNode->pBox = pBox;
			pNode->pProcess = pProcess;
			New[pNode->Path].append(pNode);
			Added.append(ID);
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

		bool bIsTerminated = pProcess->IsTerminated();
		if (pNode->IsGray != bIsTerminated)
		{
			pNode->IsGray = bIsTerminated;
			Changed = 2; // update all columns for this item
		}

		if (!bIsTerminated)
			ActiveCount++;

		if (pNode->Icon.isNull())
		{
			PixmapEntryList icons = extractIcons(pProcess->GetFileName(), false);
			if (icons.isEmpty())
				pNode->Icon = m_ExeIcon;
			else
				pNode->Icon = icons.first().pixmap;
		}

		for (int section = 0; section < columnCount(); section++)
		{
			if (!m_Columns.contains(section))
				continue; // ignore columns which are hidden

			QVariant Value;
			switch (section)
			{
			case eName:				Value = pProcess->GetProcessName(); break;
			case eProcessId:		Value = pProcess->GetProcessId(); break;
			case eStatus:			Value = pProcess->GetStatusStr(); break;
			//case eTitle:			break; // todo
			//case eLogCount:			break; // todo Value = pProcess->GetResourceLog().count(); break;
			case eTimeStamp:		Value = pProcess->GetTimeStamp(); break;
			case ePath:				Value = pProcess->GetFileName(); break;
			}

			SSandBoxNode::SValue& ColValue = pNode->Values[section];

			if (ColValue.Raw != Value)
			{
				if (Changed == 0)
					Changed = 1;
				ColValue.Raw = Value;

				switch (section)
				{
					case eProcessId:		ColValue.Formated = QString::number(pProcess->GetProcessId()); break;
					//case eLogCount:			ColValue.Formated = QString::number(Value.toInt()); break;
					case eTimeStamp:		ColValue.Formated = pProcess->GetTimeStamp().toString("hh:mm:ss"); break;
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

	return ActiveCount != 0;
}

CSandBoxPtr CSbieModel::GetSandBox(const QModelIndex &index) const
{
	if (!index.isValid())
        return CSandBoxPtr();

	SSandBoxNode* pNode = static_cast<SSandBoxNode*>(index.internalPointer());
	ASSERT(pNode);

	return pNode->pBox;
}

CBoxedProcessPtr CSbieModel::GetProcess(const QModelIndex &index) const
{
	if (!index.isValid())
		return CBoxedProcessPtr();

	SSandBoxNode* pNode = static_cast<SSandBoxNode*>(index.internalPointer());
	ASSERT(pNode);

	return pNode->pProcess;
}

int CSbieModel::columnCount(const QModelIndex &parent) const
{
	return eCount;
}

QVariant CSbieModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch(section)
		{
			case eName:				return tr("Name");
			case eProcessId:		return tr("Process ID");
			case eStatus:			return tr("Status");
			//case eTitle:			return tr("Title");
			//case eLogCount:			return tr("Log Count");
			case eTimeStamp:		return tr("Start Time");
			case ePath:				return tr("Path");
		}
	}
    return QVariant();
}

/*QVariant CSbieModel::GetDefaultIcon() const 
{ 
	return g_ExeIcon;
}*/
