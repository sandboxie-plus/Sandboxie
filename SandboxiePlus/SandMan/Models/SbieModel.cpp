#include "stdafx.h"
#include "SbieModel.h"
#include "../../MiscHelpers/Common/Common.h"
#include "../SandMan.h"
#include "../Helpers/WinHelper.h"

CSbieModel::CSbieModel(QObject *parent)
: CTreeItemModel(parent)
{
	m_bTree = true;
	m_LargeIcons = false;

	//m_BoxEmpty = QIcon(":/BoxEmpty");
	//m_BoxInUse = QIcon(":/BoxInUse");
	m_ExeIcon = QIcon(":/exeIcon32");

	m_SbieModelMimeType = "application/x-sbie-data";

	m_Root = MkNode(QVariant());
}

CSbieModel::~CSbieModel()
{
	FreeNode(m_Root);
	m_Root = NULL;
}

QList<QVariant> CSbieModel::MakeProcPath(const QString& BoxName, const CBoxedProcessPtr& pProcess, const QMap<quint32, CBoxedProcessPtr>& ProcessList)
{
	QList<QVariant> Path;
	MakeProcPath(pProcess, ProcessList, Path);
	Path.prepend(BoxName);
	return Path;
}

void CSbieModel::MakeProcPath(const CBoxedProcessPtr& pProcess, const QMap<quint32, CBoxedProcessPtr>& ProcessList, QList<QVariant>& Path)
{
	quint32 ParentID = pProcess->GetParendPID();
	CBoxedProcessPtr pParent = ProcessList.value(ParentID);

	if (!pParent.isNull() && ParentID != pProcess->GetProcessId() && !Path.contains(ParentID))
	{
		Path.prepend(ParentID);
		MakeProcPath(pParent, ProcessList, Path);
	}
}

bool CSbieModel::TestProcPath(const QList<QVariant>& Path, const QString& BoxName, const CBoxedProcessPtr& pProcess, const QMap<quint32, CBoxedProcessPtr>& ProcessList, int Index)
{
	if (Index == 0)
	{
		if (Path.isEmpty() || BoxName != Path[0])
			return false;

		return TestProcPath(Path, BoxName, pProcess, ProcessList, 1);
	}

	quint32 ParentID = pProcess->GetParendPID();
	CBoxedProcessPtr pParent = ProcessList.value(ParentID);

	if (!pParent.isNull() && ParentID != pProcess->GetProcessId())
	{
		if(Index >= Path.size() || Path[Path.size() - Index] != ParentID)
			return false;

		return TestProcPath(Path, BoxName, pParent, ProcessList, Index + 1);
	}

	return Path.size() == Index;
}

QString CSbieModel__AddGroupMark(const QString& Name)
{
	return Name.isEmpty() ? "" : ("!" + Name);
}

bool CSbieModel__HasGroupMark(const QString& Name)
{
	return Name.left(1) == "!";
}

QString CSbieModel__RemoveGroupMark(const QString& Name)
{
	return Name.left(1) == "!" ? Name.mid(1) : Name;
}

QString CSbieModel::FindParent(const QVariant& Name, const QMap<QString, QStringList>& Groups)
{
	for(auto I = Groups.begin(); I != Groups.end(); ++I)
	{
		if (I.value().contains(CSbieModel__RemoveGroupMark(Name.toString()), Qt::CaseInsensitive))
			return CSbieModel__AddGroupMark(I.key());
	}
	return QString();
}

void CSbieModel::MakeBoxPath(const QVariant& Name, const QMap<QString, QStringList>& Groups, QList<QVariant>& Path)
{
	QString ParentID = FindParent(Name, Groups);

	if (!ParentID.isEmpty() && ParentID != Name && !Path.contains(ParentID))
	{
		Path.prepend(ParentID);
		MakeBoxPath(ParentID, Groups, Path);
	}
}

QList<QVariant>	CSbieModel::MakeBoxPath(const QVariant& Name, const QMap<QString, QStringList>& Groups)
{
	QList<QVariant> Path;
	MakeBoxPath(Name, Groups, Path);
	return Path;
}

QList<QVariant> CSbieModel::Sync(const QMap<QString, CSandBoxPtr>& BoxList, const QMap<QString, QStringList>& Groups, bool ShowHidden)
{
	QList<QVariant> Added;
	QMap<QList<QVariant>, QList<STreeNode*> > New;
	QHash<QVariant, STreeNode*> Old = m_Map;

	bool bGroupsFirst = theConf->GetBool("Options/SortGroupsFirst", false);
	bool bWatchSize = theConf->GetBool("Options/WatchBoxSize", false);
	bool ColorIcons = theConf->GetBool("Options/ColorBoxIcons", false);
	bool OverlayIcons = theConf->GetBool("Options/UseOverlayIcons", true);
	bool bPlus = (theAPI->GetFeatureFlags() & CSbieAPI::eSbieFeatureCert) != 0;
	bool bVintage = theConf->GetInt("Options/ViewMode", 1) == 2;
	if (bVintage)
		bPlus = false;
	bool bHideCore = theConf->GetBool("Options/HideSbieProcesses", false);

	foreach(const QString& Group, Groups.keys())
	{
		if (Group.isEmpty())
			continue;
		QVariant ID = CSbieModel__AddGroupMark(Group);
		
		QModelIndex Index;
		
		QHash<QVariant, STreeNode*>::iterator I = Old.find(ID);
		SSandBoxNode* pNode = I != Old.end() ? static_cast<SSandBoxNode*>(I.value()) : NULL;
		if (!pNode)
		{
			pNode = static_cast<SSandBoxNode*>(MkNode(ID));
			pNode->Values.resize(columnCount());
			if (m_bTree) 
				pNode->Path = MakeBoxPath(ID, Groups); 
			pNode->pBox = NULL;
			New[pNode->Path].append(pNode);
			Added.append(ID);

			QIcon Icon = QIcon(bPlus ? ":/Boxes/Group2" : ":/Boxes/Group"); // theGUI->GetBoxIcon(CSandBoxPlus::eDefault, false);
			if (m_LargeIcons) // but not for boxes
				Icon = QIcon(Icon.pixmap(QSize(32,32)).scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
			pNode->Icon = Icon;
			pNode->IsBold = true;

			pNode->Values[eName].Raw = Group;
			if(bGroupsFirst) 
				pNode->Values[eName].SortKey = ID;
			pNode->Values[eStatus].Raw = tr("Box Group");
		}
		else
		{
			I.value() = NULL;
			Index = Find(m_Root, pNode);
		}

		int Changed = 0;

		QString ParentGroup = pNode->Path.isEmpty() ? "" : CSbieModel__RemoveGroupMark(pNode->Path.last().toString());
		int OrderNumber = Groups[ParentGroup].indexOf(Group);
		if (pNode->OrderNumber != OrderNumber) {
			pNode->OrderNumber = (OrderNumber == -1) ? Groups[ParentGroup].size() : OrderNumber;
			Changed = 1;
		}

		if (Changed && Index.isValid())
			emit dataChanged(createIndex(Index.row(), 0, pNode), createIndex(Index.row(), columnCount()-1, pNode));
	}

	foreach (const CSandBoxPtr& pBox, BoxList)
	{
		if (!ShowHidden && (!pBox->IsEnabled() /*|| pBox->GetBool("IsShadow")*/))
			continue;

		QVariant ID = pBox->GetName();

		QModelIndex Index;
		
		QHash<QVariant, STreeNode*>::iterator I = Old.find(ID);
		SSandBoxNode* pNode = I != Old.end() ? static_cast<SSandBoxNode*>(I.value()) : NULL;
		if(!pNode)
		{
			pNode = static_cast<SSandBoxNode*>(MkNode(ID));
			pNode->Values.resize(columnCount());
			if (m_bTree)
				pNode->Path = MakeBoxPath(ID, Groups);
			pNode->pBox = pBox;
			New[pNode->Path].append(pNode);
			Added.append(ID);
		}
		else
		{
			I.value() = NULL;
			Index = Find(m_Root, pNode);
		}

		auto pBoxEx = pBox.objectCast<CSandBoxPlus>();

		int Col = 0;
		bool State = false;
		int Changed = 0;

		QString Group = pNode->Path.isEmpty() ? "" : CSbieModel__RemoveGroupMark(pNode->Path.last().toString());
		int OrderNumber = Groups[Group].indexOf(pBox->GetName());
		if (pNode->OrderNumber != OrderNumber) {
			pNode->OrderNumber = (OrderNumber == -1) ? Groups[Group].size() : OrderNumber;
			Changed = 1;
		}

		QMap<quint32, CBoxedProcessPtr> ProcessList = pBox->GetProcessList();

		if (bHideCore) {
			for (auto I = ProcessList.begin(); I != ProcessList.end();) {
				if (I.value()->GetFileName().indexOf(theAPI->GetSbiePath() + "\\Sandboxie", Qt::CaseInsensitive) == 0)
					I = ProcessList.erase(I);
				else 
					++I;
			}
		}

		bool inUse = Sync(pBox, pNode->Path, ProcessList, New, Old, Added);
		bool Busy = pBoxEx->IsBoxBusy();
		int boxType = pBoxEx->GetType();
		bool boxDel = pBoxEx->IsAutoDelete();
		bool boxNoForce = pBoxEx->IsForceDisabled();
		int boxColor = pBoxEx->GetColor();
		SSandBoxNode::EMountState mountState = SSandBoxNode::eNone;
		if (pBoxEx->UseRamDisk()) 
			mountState = SSandBoxNode::eRamDisk;
		else if (pBoxEx->UseImageFile()) {
			if(pBoxEx->GetMountRoot().isEmpty())
				mountState = SSandBoxNode::eUnmounted;
			else
				mountState = SSandBoxNode::eMounted;
		}
		
		QIcon Icon;
		QString BoxIcon = pBox->GetText("BoxIcon");
		if (!BoxIcon.isEmpty())
		{
			if (pNode->BoxIcon != BoxIcon || (pNode->busyState || Busy) || pNode->boxDel != boxDel || pNode->boxNoForce != boxNoForce) 
			{
				StrPair PathIndex = Split2(BoxIcon, ",");
				if (!PathIndex.second.isEmpty() && !PathIndex.second.contains("."))
					Icon = QIcon(LoadWindowsIcon(PathIndex.first, PathIndex.second.toInt()));
				else
					Icon = QIcon(QPixmap(BoxIcon));
				pNode->BoxIcon = BoxIcon;
			}
		}
		else if (pNode->inUse != inUse || 
			(pNode->busyState || Busy) || 
			pNode->boxType != boxType || 
			pNode->boxColor != boxColor || 
			pNode->boxDel != boxDel || 
			pNode->boxNoForce != boxNoForce || 
			!pNode->BoxIcon.isEmpty() ||
			pNode->MountState != mountState
			)
		{
			pNode->inUse = inUse;
			pNode->boxType = boxType;
			pNode->boxColor = boxColor;
			pNode->boxDel = boxDel;
			pNode->boxNoForce = boxNoForce;
			//pNode->Icon = pNode->inUse ? m_BoxInUse : m_BoxEmpty;
			if(ColorIcons)
				Icon = theGUI->GetColorIcon(boxColor, inUse);
			else
				Icon = theGUI->GetBoxIcon(boxType, inUse);
			pNode->BoxIcon.clear();
			pNode->MountState = mountState;
		}

		if (!Icon.isNull()) 
		{
			if (Busy)	
				Icon = theGUI->MakeIconBusy(Icon, pNode->busyState++);
			else {
				pNode->busyState = 0;

				if (OverlayIcons) 
				{
					if(boxNoForce)
						Icon = theGUI->IconAddOverlay(Icon, ":/IconDFP");
					else if(mountState == SSandBoxNode::eRamDisk)
						Icon = theGUI->IconAddOverlay(Icon, ":/Actions/RamDisk.png");
					else if(mountState == SSandBoxNode::eMounted)
						Icon = theGUI->IconAddOverlay(Icon, ":/Actions/LockOpen.png");
					else if(mountState == SSandBoxNode::eUnmounted)
						Icon = theGUI->IconAddOverlay(Icon, ":/Actions/LockClosed.png");
					else if (boxDel && !bVintage)
						Icon = theGUI->IconAddOverlay(Icon, ":/Boxes/AutoDel");
				}
			}
			
			if (m_LargeIcons) // but not for boxes
				Icon = QIcon(Icon.pixmap(QSize(32,32)).scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

			pNode->Icon = Icon;
			Changed = 1; // set change for first column
		}


		if (pNode->IsGray != !pBoxEx->IsEnabled())
		{
			pNode->IsGray = !pBoxEx->IsEnabled();
			Changed = 2; // set change for all columns
		}

		for(int section = 0; section < columnCount(); section++)
		{
			if (!IsColumnEnabled(section))
				continue; // ignore columns which are hidden

			QVariant Value;
			switch(section)
			{
				case eName:				Value = pBoxEx->GetDisplayName(); break;
				//case eName:				Value = pBox->GetName(); break;
				case eStatus:			Value = pBox.objectCast<CSandBoxPlus>()->GetStatusStr(); break;
				case eTitle:			break;
				case eInfo:				Value = pBox.objectCast<CSandBoxPlus>()->IsEmptyCached() ? -2 : (bWatchSize ? pBox.objectCast<CSandBoxPlus>()->GetSize() : 0); break;
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
				//case eName:				ColValue.Formatted = Value.toString().replace("_", " "); break;
				case eInfo:				ColValue.Formatted = Value.toULongLong() == -2 ? tr("Empty") : (Value.toULongLong() > 0 ? FormatSize(Value.toULongLong()) : ""); break;
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

bool CSbieModel::Sync(const CSandBoxPtr& pBox, const QList<QVariant>& Path, const QMap<quint32, CBoxedProcessPtr>& ProcessList, QMap<QList<QVariant>, QList<STreeNode*> >& New, QHash<QVariant, STreeNode*>& Old, QList<QVariant>& Added)
{
	QString BoxName = pBox->GetName();

	int ActiveCount = 0;
	bool OverlayIcons = theConf->GetBool("Options/UseOverlayIcons", true);

	foreach(const CBoxedProcessPtr& pProc, ProcessList)
	{
		QSharedPointer<CSbieProcess> pProcess = pProc.objectCast<CSbieProcess>();
		QVariant ID = pProcess->GetProcessId();

		QModelIndex Index;

		QHash<QVariant, STreeNode*>::iterator I = Old.find(ID);
		SSandBoxNode* pNode = I != Old.end() ? static_cast<SSandBoxNode*>(I.value()) : NULL;
		if (!pNode || (m_bTree ? !TestProcPath(pNode->Path.mid(Path.length()), BoxName, pProcess, ProcessList) : !pNode->Path.isEmpty())) // todo: improve that
		{
			pNode = static_cast<SSandBoxNode*>(MkNode(ID));
			pNode->Values.resize(columnCount());
			if (m_bTree)
				pNode->Path = Path + MakeProcPath(BoxName, pProcess, ProcessList);
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

		if (pNode->Icon.isNull() && !pProcess->GetFileName().isEmpty())
		{
			//PixmapEntryList icons = extractIcons(pProcess->GetFileName(), false);
			//if (icons.isEmpty())
			//	pNode->Icon = m_ExeIcon;
			//else
			//	pNode->Icon = icons.first().pixmap;

			QIcon Icon = m_IconProvider.icon(QFileInfo(pProcess->GetFileName()));
			if (Icon.isNull())
				Icon = m_ExeIcon;

			if (OverlayIcons) {
				if (pProcess->HasSystemToken())
					Icon = theGUI->IconAddOverlay(Icon, ":/Actions/SystemShield.png");
				else if (pProcess->HasElevatedToken())
					Icon = theGUI->IconAddOverlay(Icon, ":/Actions/AdminShield.png");
				else if (pProcess->HasAppContainerToken())
					Icon = theGUI->IconAddOverlay(Icon, ":/Actions/AppContainer.png"); // AppContainer is also Restricted
				else if (pProcess->HasRestrictedToken())
					Icon = theGUI->IconAddOverlay(Icon, ":/Actions/Restricted.png");
			}

			pNode->Icon = Icon;
			Changed = 1;
		}

		for (int section = 0; section < columnCount(); section++)
		{
			if (!IsColumnEnabled(section))
				continue; // ignore columns which are hidden

			QVariant Value;
			switch (section)
			{
			case eName: {				
							QString Name = pProcess->GetProcessName(); 
							if (pProcess->IsWoW64())
								Name += " *32";
							Value = Name;
							break;
						}
			case eProcessId:		Value = pProcess->GetProcessId(); break;
			case eStatus:			Value = pProcess->GetStatusStr(); break;
			case eTitle:			Value = theAPI->GetProcessTitle(pProcess->GetProcessId()); break;
			//case eLogCount:			break; // todo Value = pProcess->GetResourceLog().count(); break;
			case eInfo:				Value = pProcess->GetTimeStamp(); break;
			//case ePath:				Value = pProcess->GetFileName(); break;
			case ePath: {
									QString CmdLine = pProcess->GetCommandLine(); 
									Value = CmdLine.isEmpty() ? pProcess->GetFileName() : CmdLine;
									break;
						}
			}

			SSandBoxNode::SValue& ColValue = pNode->Values[section];

			if (ColValue.Raw != Value)
			{
				if (Changed == 0)
					Changed = 1;
				ColValue.Raw = Value;

				switch (section)
				{
					case eProcessId:		ColValue.Formatted = QString::number(pProcess->GetProcessId()); break;
					//case eLogCount:			ColValue.Formatted = QString::number(Value.toInt()); break;
					case eInfo:				ColValue.Formatted = pProcess->GetTimeStamp().toString("hh:mm:ss"); break;
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

QVariant CSbieModel::NodeData(STreeNode* pNode, int role, int section) const
{
	if (section == 0 && role == Qt::InitialSortOrderRole) {
		return ((SSandBoxNode*)pNode)->OrderNumber;
	}
	return CTreeItemModel::NodeData(pNode, role, section);
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

QString	CSbieModel::GetGroup(const QModelIndex& index) const
{
	if (!index.isValid())
		return QString();

	SSandBoxNode* pNode = static_cast<SSandBoxNode*>(index.internalPointer());
	ASSERT(pNode);
	
	if(!CSbieModel__HasGroupMark(pNode->ID.toString()))
		return QString();

	return pNode->ID.toString();
}

QVariant CSbieModel::GetID(const QModelIndex &index) const
{
	if (!index.isValid())
		return QVariant();

	SSandBoxNode* pNode = static_cast<SSandBoxNode*>(index.internalPointer());
	ASSERT(pNode);

	if (!pNode->pProcess && !pNode->pBox)
		return CSbieModel__RemoveGroupMark(pNode->ID.toString());

	return pNode->ID;
}

QModelIndex CSbieModel::FindGroupIndex(const QString& Name) const
{
	QVariant ID = CSbieModel__AddGroupMark(Name);
	QHash<QVariant, STreeNode*>::const_iterator I = m_Map.find(ID);
	if (I == m_Map.end())
		return QModelIndex();

	return Find(m_Root, I.value());
}

CSbieModel::ETypes CSbieModel::GetType(const QModelIndex &index) const
{
	if (!index.isValid())
		return eNone;

	SSandBoxNode* pNode = static_cast<SSandBoxNode*>(index.internalPointer());
	ASSERT(pNode);

	if (pNode->pProcess)
		return eProcess;
	if (pNode->pBox)
		return eBox;
	return eGroup;
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
			case eTitle:			return tr("Title");
			case eInfo:				return tr("Info");
			//case eSize:				return tr("Size");
			//case eLogCount:			return tr("Log Count");
			//case eTimeStamp:		return tr("Start Time");
			case ePath:				return tr("Path / Command Line");
		}
	}
    return QVariant();
}

/*QVariant CSbieModel::GetDefaultIcon() const 
{ 
	return g_ExeIcon;
}*/

QVariant CSbieModel::data(const QModelIndex &index, int role) const
{
	//if(m_LargeIcons && role == Qt::SizeHintRole)
	//    return QSize(32,32);

	return CTreeItemModel::data(index, role);
}

Qt::ItemFlags CSbieModel::flags(const QModelIndex& index) const 
{
	Qt::ItemFlags Flags = CTreeItemModel::flags(index);

	Flags |= Qt::ItemIsDragEnabled;

	SSandBoxNode* pNode = static_cast<SSandBoxNode*>(index.internalPointer());
	if (!pNode || (pNode && CSbieModel__HasGroupMark(pNode->ID.toString())) || pNode == m_Root)
		Flags |= Qt::ItemIsDropEnabled;

	return Flags;
}

QMimeData* CSbieModel::mimeData(const QModelIndexList &indexes) const
{
	QStringList Boxes;
	for (int i = 0; i < indexes.count(); i++) {
		if (indexes[i].column() != 0)
			continue;
		SSandBoxNode* pNode = static_cast<SSandBoxNode*>(indexes[i].internalPointer());
		Boxes.append(pNode->ID.toString());
	}

	QMimeData *data = new QMimeData();
	data->setData(m_SbieModelMimeType, Boxes.join(",").toLatin1());
	return data;
}

bool CSbieModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {

	QStringList Boxes = QString::fromLatin1(data->data(m_SbieModelMimeType)).split(",");
	QString To = ""; // root

	SSandBoxNode* pNode = static_cast<SSandBoxNode*>(parent.internalPointer());
	if (pNode)
		To = CSbieModel__RemoveGroupMark(pNode->ID.toString());

	foreach(const QString & Name, Boxes) {
		if(CSbieModel__HasGroupMark(Name))
			MoveGroup(CSbieModel__RemoveGroupMark(Name), To, row);
		else
			MoveBox(Name, To, row);
	}

	return true;
}
