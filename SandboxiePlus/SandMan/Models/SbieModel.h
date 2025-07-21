#pragma once
#include <qwidget.h>
#include "../SbiePlusAPI.h"
#include "../SbieProcess.h"
#include "../../MiscHelpers/Common/TreeItemModel.h"
#include <QMimeData>
#include <QFileIconProvider>

class CSbieModel : public CTreeItemModel
{
    Q_OBJECT

public:
    CSbieModel(QObject *parent = 0);
	~CSbieModel();

	QList<QVariant>	Sync(const QMap<QString, CSandBoxPtr>& BoxList, const QMap<QString, QStringList>& Groups = QMap<QString, QStringList>(), bool ShowHidden = false);

	void			SetTree(bool bTree)				{ m_bTree = bTree; }
	bool			IsTree() const					{ return m_bTree; }
	void			SetLargeIcons(bool bSet = true) { m_LargeIcons = bSet; }

	CSandBoxPtr		GetSandBox(const QModelIndex &index) const;
	CBoxedProcessPtr GetProcess(const QModelIndex &index) const;
	QString			GetGroup(const QModelIndex &index) const;
	QVariant		GetID(const QModelIndex &index) const;
	QModelIndex		FindGroupIndex(const QString& Name) const;

	enum ETypes
	{
		eNone = 0,
		eGroup,
		eBox,
		eProcess
	}				GetType(const QModelIndex &index) const;

	Qt::DropActions supportedDropActions() const { return Qt::MoveAction; }
	QVariant data(const QModelIndex &index, int role) const;
	Qt::ItemFlags flags(const QModelIndex& index) const;
	QStringList mimeTypes() { return QStringList() << m_SbieModelMimeType; }
	QMimeData* mimeData(const QModelIndexList& indexes) const;
	bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const { return true; }
	bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);

	int				columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant		headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	enum EColumns
	{
		eName = 0,
		eProcessId,
		eTitle,
		eStatus,
		eInfo,
		//eSize,
		//eLogCount,
		//eTimeStamp,
		ePath,
		eCount
	};

signals:
	void			MoveBox(const QString& Name, const QString& To, int row);
	void			MoveGroup(const QString& Name, const QString& To, int row);

protected:
	bool			Sync(const CSandBoxPtr& pBox, const QList<QVariant>& Path, const QMap<quint32, CBoxedProcessPtr>& ProcessList, QMap<QList<QVariant>, QList<STreeNode*> >& New, QHash<QVariant, STreeNode*>& Old, QList<QVariant>& Added);

	struct SSandBoxNode: STreeNode
	{
		SSandBoxNode(CTreeItemModel* pModel, const QVariant& Id) : STreeNode(pModel, Id) { 
			inUse = false;  
			busyState = 0; 
			boxType = -1; 
			boxDel = false; 
			boxNoForce = false; 
			boxColor = 0; 
			OrderNumber = 0; 
			MountState = eNone;
		}

		CSandBoxPtr	pBox;
		bool		inUse;
		int			busyState;
		int			boxType;
		bool		boxDel;
		bool		boxNoForce;
		int			boxColor;
		int			OrderNumber;
		QString		BoxIcon;
		enum EMountState{
			eNone = 0,
			eMounted,
			eUnmounted,
			eRamDisk
		}			MountState;

		CBoxedProcessPtr pProcess;
	};

	virtual QVariant		NodeData(STreeNode* pNode, int role, int section) const;

	virtual STreeNode*		MkNode(const QVariant& Id) { return new SSandBoxNode(this, Id); }

	QList<QVariant>			MakeProcPath(const QString& BoxName, const CBoxedProcessPtr& pProcess, const QMap<quint32, CBoxedProcessPtr>& ProcessList);
	void					MakeProcPath(const CBoxedProcessPtr& pProcess, const QMap<quint32, CBoxedProcessPtr>& ProcessList, QList<QVariant>& Path);
	bool					TestProcPath(const QList<QVariant>& Path, const QString& BoxName, const CBoxedProcessPtr& pProcess, const QMap<quint32, CBoxedProcessPtr>& ProcessList, int Index = 0);

	QString					FindParent(const QVariant& Name, const QMap<QString, QStringList>& Groups);
	QList<QVariant>			MakeBoxPath(const QVariant& Name, const QMap<QString, QStringList>& Groups);
	void					MakeBoxPath(const QVariant& Name, const QMap<QString, QStringList>& Groups, QList<QVariant>& Path);

	//virtual QVariant		GetDefaultIcon() const;

private:

	bool								m_bTree;
	bool m_LargeIcons;
	//QIcon m_BoxEmpty;
	//QIcon m_BoxInUse;
	QIcon m_ExeIcon;

	QString m_SbieModelMimeType;
	QFileIconProvider m_IconProvider;
};